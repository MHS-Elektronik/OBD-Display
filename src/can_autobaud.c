/*******************************************************************************
                        can_autobaud.c  -  description
                             -------------------
    begin             : 27.01.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include <gtk/gtk.h>
#include "can_drv.h"
#include "can_device.h"
#include "util.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "can_autobaud.h"

#define INDEX_AUTOBAUD_FIFO 0x80000001 // <*> verschieben

struct TCanSpeedsTab
  {
  uint16_t CanSpeed;
  const gchar *CanSpeedStr;
  };


#define TestWaitTime 3000  // Wartezeit auf CAN Nachrichten in ms
#define ERROR_CNT_LIMIT 10
#define HIT_CNT_LIMIT   10

#define UsedCanSpeedsTabSize 9
// Tabelle mit CAN Übertragungsgeschwindigkeiten
static const struct TCanSpeedsTab UsedCanSpeedsTab[UsedCanSpeedsTabSize] = {
    {CAN_10K_BIT,  "10 kBit/s"},    // 10 kBit/s
    {CAN_20K_BIT,  "20 kBit/s"},    // 20 kBit/s
    {CAN_50K_BIT,  "50 kBit/s"},    // 50 kBit/s
    {CAN_100K_BIT, "100 kBit/s"},   // 100 kBit/s
    {CAN_125K_BIT, "125 kBit/s"},   // 125 kBit/s
    {CAN_250K_BIT, "250 kBit/s"},   // 250 kBit/s
    {CAN_500K_BIT, "500 kBit/s"},   // 500 kBit/s
    {CAN_800K_BIT, "800 kBit/s"},   // 800 kBit/s
    {CAN_1M_BIT,   "1 MBit/s"}};    // 1 MBit/s


#define RX_EVENT     0x00000001
#define STATUS_EVENT 0x00000002


static gpointer ThreadExecute(gpointer data);
static int32_t CountBits(uint32_t value);



struct TCanAutobaud *CreateCanAutobaud(TMhsGScheduler *scheduler, struct TCanDevice *can_device, uint32_t flags)
{
struct TCanAutobaud *autobaud;

if (!(autobaud = (struct TCanAutobaud *)g_malloc0(sizeof(struct TCanAutobaud))))
  return(NULL);
autobaud->CanDevice = can_device;
autobaud->Flags = flags;
autobaud->EnableSpeeds = AUTOBAUD_ALL_EN;
(void)CanExSetAsUByte(can_device->DeviceIndex, "CanErrorMsgsEnable", 1);
// Empfangs FIFO  erzeugen
(void)CanExCreateFifo(INDEX_AUTOBAUD_FIFO, 1000, NULL, 0, 0xFFFFFFFF);
autobaud->Event = CanExCreateEvent();   // Event Objekt erstellen
// Events mit API Ereignissen verknüfpen
CanExSetObjEvent(can_device->DeviceIndex, MHS_EVS_STATUS, autobaud->Event, STATUS_EVENT);
CanExSetObjEvent(INDEX_AUTOBAUD_FIFO, MHS_EVS_OBJECT, autobaud->Event, RX_EVENT);
autobaud->Scheduler = scheduler;
return(autobaud);
}


void DestroyCanAutobaud(struct TCanAutobaud **autobaud_ref)
{
struct TCanAutobaud *autobaud;

if (!(autobaud = *autobaud_ref))
  return;
StopCanAutobaud(autobaud);
g_free(autobaud);
*autobaud_ref = NULL;
}


void SetupAutobaud(struct TCanAutobaud *autobaud, uint32_t enable_speeds)
{
if (!autobaud)
  return;
autobaud->EnableSpeeds = enable_speeds;
autobaud->CanSpeedsCount = CountBits(enable_speeds);
}


void StartCanAutobaud(struct TCanAutobaud *autobaud, uint16_t start_can_speed)
{
TMhsGMessage *g_msg;

StopCanAutobaud(autobaud);
autobaud->AktivTabIndex = 0;
autobaud->CanSpeed = start_can_speed;
if (!(autobaud->CanSpeedsCount = CountBits(autobaud->EnableSpeeds)))
  {
  g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_ERROR, "CAN Speed nicht definiert"); // <*> Text noch ändern
  mhs_g_message_post(autobaud->Scheduler, g_msg);
  return;
  }
// Terminate Event setzen
CanExResetEvent(autobaud->Event, 0x10); //MHS_TERMINATE);
// Thread erzeugen und starten
autobaud->Thread = g_thread_new(NULL, ThreadExecute, autobaud);
}


void StopCanAutobaud(struct TCanAutobaud *autobaud)
{
if (autobaud->Thread)
  {
  // Terminate Event setzen
  CanExSetEvent(autobaud->Event, 0x10); //MHS_TERMINATE);
  g_thread_join(autobaud->Thread);
  autobaud->Thread = NULL;
  }
}


/****************/
/* Event Thread */
/****************/
static int32_t CountBits(uint32_t value)
{
uint32_t i;
int32_t cnt;

cnt = 0;
for (i = 1; i; i <<= 1)
  {
  if (i & value)
    cnt++;
  }
return(cnt);
}


const struct TCanSpeedsTab *AutobaudFirstSpeed(struct TCanAutobaud *autobaud)
{
int32_t idx, hit_idx, idx_count;
uint32_t aktiv_index;

if (!autobaud)
  return(NULL);

hit_idx = -1;
aktiv_index = 0;
idx_count = 0;
for (idx = 0; idx < UsedCanSpeedsTabSize; idx++)
  {
  if (autobaud->EnableSpeeds & (0x0001 << idx))
    {
    if (autobaud->CanSpeed == UsedCanSpeedsTab[idx].CanSpeed)
      {
      hit_idx = idx;
      aktiv_index = idx_count;
      break;
      }
    if (hit_idx < 0)
      {
      hit_idx = idx;
      aktiv_index = idx_count;
      }
    idx_count++;
    }
  }
if (hit_idx < 0)
  {
  autobaud->AktivTabIndex = 0;
  autobaud->AktivIndex = 0;
  return(NULL);
  }
else
  {
  autobaud->AktivTabIndex = (uint32_t)hit_idx;
  autobaud->AktivIndex = aktiv_index;
  return(&UsedCanSpeedsTab[hit_idx]);
  }
}


const struct TCanSpeedsTab *AutobaudNextSpeed(struct TCanAutobaud *autobaud)
{
uint32_t cnt, idx;

if (!autobaud)
  return(NULL);
cnt = 0;
if (++autobaud->AktivIndex >= autobaud->CanSpeedsCount)
  autobaud->AktivIndex = 0;
idx = autobaud->AktivTabIndex;
for (idx++; idx < UsedCanSpeedsTabSize; idx++)
  {
  cnt++;
  if (autobaud->EnableSpeeds & (0x0001 << idx))
    {
    autobaud->AktivTabIndex = idx;
    return(&UsedCanSpeedsTab[idx]);
    }
  }
for (idx = 0; ++cnt <= UsedCanSpeedsTabSize; idx++)
  {
  if (autobaud->EnableSpeeds & (0x0001 << idx))
    {
    autobaud->AktivTabIndex = idx;
    return(&UsedCanSpeedsTab[idx]);
    }
  }
autobaud->AktivTabIndex = 0;
return(NULL);
}


static gpointer ThreadExecute(gpointer data)
{
struct TCanAutobaud *autobaud;
struct TCanDevice *can_device;
const struct TCanSpeedsTab *speed_tab_item;
struct TCanMsg *msg;
TMhsGMessage *g_msg;
struct TDeviceStatus status;
uint32_t event;
int32_t size, hit_cnt, eff_cnt, std_cnt, error_cnt, first, setup, ok;
gchar *str;

if (!(autobaud = (struct TCanAutobaud *)data))
  return(NULL);
can_device = autobaud->CanDevice;
first = 1;
setup = 1;
ok = 0;
str = NULL;
if (!(speed_tab_item = AutobaudFirstSpeed(autobaud)))
  return(NULL);
do
  {
  if (setup)
    {
    setup = 0;
    // **** Übertragungsgeschwindigkeit einstellen
    CanSetSpeed(can_device->DeviceIndex, speed_tab_item->CanSpeed);
    // **** CAN Bus Start
    if (first)
      {
      first = 0;
      CanSetMode(can_device->DeviceIndex, OP_CAN_START_LOM, CAN_CMD_ALL_CLEAR);
      }
    else
      CanSetMode(can_device->DeviceIndex, OP_CAN_NO_CHANGE, CAN_CMD_ALL_CLEAR);
    CanReceiveClear(INDEX_AUTOBAUD_FIFO);
    hit_cnt = 0;
    eff_cnt = 0;
    std_cnt = 0;
    error_cnt = 0;
    autobaud->FrameFormatType = 0;
    if (autobaud->CanSpeedsCount == 1)
      {
      str = g_strdup_printf("Baudrate %s testen, warte auf CAN Nachrichten ...",
           speed_tab_item->CanSpeedStr);
      }
    else
      {
      str = g_strdup_printf("[%u/%u] Baudrate %s testen, warte auf CAN Nachrichten ...",
           autobaud->AktivIndex + 1, autobaud->CanSpeedsCount, speed_tab_item->CanSpeedStr);
      }
    g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_START, str);
    mhs_g_message_post(autobaud->Scheduler, g_msg);
    safe_free(str);
    }

  event = CanExWaitForEvent(autobaud->Event, TestWaitTime);
  if (event & 0x10) //0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & STATUS_EVENT)
    {
    /*********************************/
    /*  Status abfragen & auswerten  */
    /*********************************/
    CanGetDeviceStatus(can_device->DeviceIndex, &status);

    if (status.DrvStatus >= DRV_STATUS_CAN_OPEN)
      {
      if (status.CanStatus == CAN_STATUS_BUS_OFF)
        CanSetMode(can_device->DeviceIndex, OP_CAN_RESET, CAN_CMD_NONE);
      }
    else
      {
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_ERROR, "CAN Device nicht geöffnet");
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      break;  // Thread beenden
      }
    }
  else if (event & RX_EVENT)     // CAN Rx Event
    {
    while ((size = CanReceive(INDEX_AUTOBAUD_FIFO, autobaud->RxTempBuffer, AUTOBAUD_TEMP_BUFFER_SIZE)) > 0)
      {
      for (msg = autobaud->RxTempBuffer; size; size--)
        {
        if (msg->MsgErr)
          error_cnt++;
        else
          {
          if (msg->MsgEFF)
            eff_cnt++;
          else
            std_cnt++;
          hit_cnt++;
          }
        msg++;
        }
      if (error_cnt >= ERROR_CNT_LIMIT)
        {
        setup = 1;
        break;
        }
      else if (hit_cnt >= HIT_CNT_LIMIT)
        {
        ok = 1;
        break;
        }
      }
    if (ok)
      {
      if (autobaud->CanSpeedsCount == 1)
        {
        str = g_strdup_printf("Baudrate %s erfolgreich dedektiert, %d CAN Nachrichten erfolgreich empfangen",
           speed_tab_item->CanSpeedStr, hit_cnt);
        }
      else
        {
        str = g_strdup_printf("[%u/%u] Baudrate %s erfolgreich dedektiert, %d CAN Nachrichten erfolgreich empfangen",
           autobaud->AktivIndex + 1, autobaud->CanSpeedsCount, speed_tab_item->CanSpeedStr, hit_cnt);
        }
      if (std_cnt)
        autobaud->FrameFormatType |= AUTOBAUD_FRAMES_STD;
      if (eff_cnt)
        autobaud->FrameFormatType |= AUTOBAUD_FRAMES_EXT;
      autobaud->CanSpeed = speed_tab_item->CanSpeed;
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_OK, str);
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      safe_free(str);
      break;   // Thread beenden
      }
    else
      {
      // <*> noch ändern nur das Tiny-CAN IV-XL kann Busfehler erkennen
      if (autobaud->CanSpeedsCount == 1)
        {
        str = g_strdup_printf("Baudrate %s testen, %d CAN Nachrichten erfolgreich empfangen, %d Busfehler erkannt",
           speed_tab_item->CanSpeedStr, hit_cnt, error_cnt);
        }
      else
        {
        str = g_strdup_printf("[%u/%u] Baudrate %s testen, %d CAN Nachrichten erfolgreich empfangen, %d Busfehler erkannt",
           autobaud->AktivIndex + 1, autobaud->CanSpeedsCount, speed_tab_item->CanSpeedStr, hit_cnt, error_cnt);
        }
      g_msg = mhs_g_new_message_from_string(MHS_MSG_AUTOBAUD_RUN, str);
      mhs_g_message_post(autobaud->Scheduler, g_msg);
      safe_free(str);
      }
    }
  else if (!event) // Timeout
    setup = 1;

  if (setup)
    {
    speed_tab_item = AutobaudNextSpeed(autobaud);
    /*if (++autobaud->AktivTabIndex >= UsedCanSpeedsTabSize)
      autobaud->AktivTabIndex = 0;*/
    }
  }
while (1);
//if (autobaud->Flags & AUTOBAUD_CLOSE_ON_FINISH)
//  CanSetMode(can_device->DeviceIndex, OP_CAN_STOP, CAN_CMD_ALL_CLEAR); <*>
CanSetMode(can_device->DeviceIndex, OP_CAN_START, CAN_CMD_ALL_CLEAR);
return(NULL);
}
