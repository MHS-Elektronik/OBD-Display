/*******************************************************************************
                        can_device.c  -  description
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
#include "util.h"
#include "mhs_thread.h"
#include "can_drv.h"
#include "can_device.h"


#ifdef __WIN32__
  #deinfe DRIVER_FILE "mhstcan.dll"
  #define FULL_DRIVER_FILE NULL
#else
  #define DRIVER_FILE "libmhstcan.so"
  #define FULL_DRIVER_FILE "/opt/tiny_can/can_api/libmhstcan.so"
#endif

#define TREIBER_INIT "CanCallThread=0"
#define CREATE_DEVICE_OPTIONS "CanRxDFifoSize=16384;CanTxDFifoSize=16384"

#define RX_EVENT     0x00000001
#define STATUS_EVENT 0x00000002


static volatile gint CanDevicesCount;

uint32_t TimeNow;
TCanCore *CanCore = NULL;


static void CanDevHandlerDestroy(struct TCanDevice *can_device);
static void CommThreadExecute(TMhsThread *thread);



struct TCanDevice *CanDevCreate(uint32_t timeout)
{
int32_t err;
char *str;
struct TCanDevice *can_device;

if (!CanCore)
  {
  if (!(CanCore = (struct TCanCore *)g_malloc0(sizeof(struct TCanCore))))
    return(NULL);
  }
if (!(can_device = (struct TCanDevice *)g_malloc0(sizeof(struct TCanDevice))))
  return(NULL);
can_device->CanCore = CanCore;  
can_device->EventTimeout = timeout;
safe_free(can_device->LastErrorString);
err = 0;
if (!CanDevicesCount)
  {
  safe_free(CanCore->DriverFileName);
  CanCore->DriverFileName = g_strdup(DRIVER_FILE);
  safe_free(CanCore->DriverInfoStr);
  safe_free(CanCore->LastErrorString);
  // **** Treiber DLL laden
  if ((err = LoadDriver(FULL_DRIVER_FILE)) < 0)
    CanCore->LastErrorString = g_strdup_printf("LoadDriver Error-Code:%d", err);
  // **** Treiber DLL initialisieren
  else if ((err = CanExInitDriver(TREIBER_INIT)) < 0)
    CanCore->LastErrorString = g_strdup_printf("CanInitDrv Error-Code:%d", err);
  if (err < 0)
    CanCore->Status = CAN_CORE_DRIVER_ERROR;
  else
    {
    if ((str = CanDrvInfo()))
      CanCore->DriverInfoStr = g_strdup(str);
    CanCore->Status = CAN_CORE_DRV_LOAD;
    CanDevicesCount++;
    }
  }
if (!err)
// **** Device u. Empfangs-FIFO für das Device erzeugen
  {
  if ((err = CanExCreateDevice(&can_device->DeviceIndex, CREATE_DEVICE_OPTIONS)) < 0)
    can_device->LastErrorString = g_strdup_printf("CanExCreateDevice Error-Code:%d", err);
  }

if (!err)
  {
  if (!(can_device->Event = CanExCreateEvent()))   // Event Objekt erstellen
    err = -1;
  else
    {
    // Events mit API Ereignissen verknüfpen
    CanExSetObjEvent(can_device->DeviceIndex, MHS_EVS_STATUS, can_device->Event, STATUS_EVENT);
    CanExSetObjEvent(can_device->DeviceIndex, MHS_EVS_OBJECT, can_device->Event, RX_EVENT);

    if (!(can_device->Thread = mhs_create_thread(CommThreadExecute, can_device, MHS_THREAD_PRIORITY_HIGHEST, 0)))
      err = -1;
    else
      {
      mhs_event_set_event_mask((TMhsEvent *)can_device->Thread, MHS_ALL_EVENTS);
      can_device->Status = DEV_STATUS_CLOSE;
      }
    }
  }
if (err < 0)
  CanDevDestroy(&can_device);
return(can_device);
}


void CanDevDestroy(struct TCanDevice **can_device)
{
struct TCanDevice *c;

if (can_device)
  {
  if (!(c = *can_device))
    return;
  if (CanDevicesCount)
    CanDevicesCount--;
  if (c->Event)
    CanExSetEvent(c->Event, 0x10);
  if (c->Thread)
    {
    (void)mhs_join_thread(c->Thread, 1000);
    mhs_destroy_thread(&c->Thread, 0);
    }
  CanDevHandlerDestroy(c);
  // Device löschen
  (void)CanExDestroyDevice(&c->DeviceIndex);
  safe_free(c->LastErrorString);
  g_free(c);
  *can_device = NULL;
  }
if (!CanDevicesCount)
  {
  safe_free(CanCore->DriverFileName);
  safe_free(CanCore->DriverInfoStr);
  safe_free(CanCore->LastErrorString);
  CanCore->Status = CAN_CORE_INIT;
  CanDownDriver();
  // **** DLL entladen
  UnloadDriver();
  }
}


int32_t CanDevOpen(struct TCanDevice *can_device, uint32_t flags, const char *snr, uint16_t speed)
{
int32_t err;
char *str;

err = 0;
if ((!can_device) || (!CanCore))
  return(-1); // <*>
safe_free(can_device->LastErrorString);
str = NULL;
if (CanCore->Status < 0)
  {
  can_device->Status = CanCore->Status;
  can_device->LastErrorString = g_strdup_printf("API Init Error"); // <*>
  return(-1);
  }
// **** Schnittstelle PC <-> Tiny-CAN öffnen
if (snr)
  str = g_strdup_printf("Snr=%s", snr);
if ((err = CanDeviceOpen(can_device->DeviceIndex, str) < 0))
  can_device->LastErrorString = g_strdup_printf("CanDeviceOpen Error-Code:%d", err);
safe_free(str);
if (!err)
  {
  if ((err = (gint)CanExGetDeviceInfo(can_device->DeviceIndex, &can_device->DeviceInfo, NULL, NULL)) < 0)
    can_device->LastErrorString = g_strdup_printf("CanExGetDeviceInfo Error-Code:%d", err);
  }
/*if ((str = CanDrvHwInfo(can_device->DeviceIndex))) <*>
  {
  safe_free(can_device->TinyCanInfoStr);
  can_device->TinyCanInfoStr = g_strdup(str);
  } */
if (!err)
  {    
  /*****************************************/
  /*  CAN Speed einstellen & Bus starten   */
  /*****************************************/
  // **** Übertragungsgeschwindigkeit einstellen
  CanSetSpeed(can_device->DeviceIndex, speed);
  if (flags & CAN_DEV_CAN_NOT_START)
  // **** CAN Bus Start, alle FIFOs, Filter, Puffer und Fehler löschen
    CanSetMode(can_device->DeviceIndex, OP_CAN_STOP, CAN_CMD_ALL_CLEAR);
  else
    CanSetMode(can_device->DeviceIndex, OP_CAN_START, CAN_CMD_ALL_CLEAR);
  CanExResetEvent(can_device->Event, 0x10);
  mhs_run_thread(can_device->Thread);
  can_device->Status = DEV_STATUS_READY;
  }
else
  can_device->Status = DEV_CAN_OPEN_ERROR;
return(0);
}


void CanDevClose(struct TCanDevice *can_device)
{
CanExSetEvent(can_device->Event, 0x10);
// Device schließen
(void)CanDeviceClose(can_device->DeviceIndex);
(void)mhs_join_thread(can_device->Thread, 5000);

safe_free(can_device->LastErrorString);
can_device->Status = DEV_STATUS_CLOSE;
}


int32_t CanDevTxMessage(struct TCanDevice *can_device, struct TCanMsg *msgs, int32_t count)
{
if (!can_device)
  return(-1);
return(CanTransmit(can_device->DeviceIndex, msgs, count));
}


int CanDevRxEventConnect(struct TCanDevice *can_device, TCanDevRxCB proc, void *user_data)
{
TCanDevRxHandler *handler, *list;

if (!can_device)
  return(-1);
mhs_enter_critical((TMhsEvent *)can_device->Thread);
for (handler = can_device->CanDevRxHandler; handler; handler = handler->Next)
  {
  if (handler->Proc == proc)
    break;
  }
if (!handler)
  {
  if (!(handler = (TCanDevRxHandler *)g_malloc0(sizeof(TCanDevRxHandler))))
    return(-1);
  list = can_device->CanDevRxHandler;
  if (!list)
    can_device->CanDevRxHandler = handler;
  else
    {
    while (list->Next)
      list = list->Next;
    list->Next = handler;
    }
  handler->Proc = proc;
  handler->UserData = user_data;
  }
mhs_leave_critical((TMhsEvent *)can_device->Thread);
return(0);
}


void CanDevRxEventDisconnect(struct TCanDevice *can_device, TCanDevRxCB proc)
{
TCanDevRxHandler *list, *prev;

if (!can_device)
  return;
mhs_enter_critical((TMhsEvent *)can_device->Thread);
// Liste nach "handler" durchsuchen
prev = NULL;
for (list = can_device->CanDevRxHandler; list; list = list->Next)
  {
  if (list->Proc == proc)
    {
    if (prev)
      prev->Next = list->Next;
    else
      can_device->CanDevRxHandler = list->Next;
    g_free(list);
    break;
    }
  prev = list;
  }
mhs_leave_critical((TMhsEvent *)can_device->Thread);
}


/***************************************************************************/
/*                       Alle Event Handler löschen                        */
/***************************************************************************/
static void CanDevHandlerDestroy(struct TCanDevice *can_device)
{
TCanDevRxHandler *tmp, *list;

if (!can_device)
  return;
list = can_device->CanDevRxHandler;
while (list)
  {
  tmp = list->Next;
  g_free(list);
  list = tmp;
  }
can_device->CanDevRxHandler = NULL;
}


static void CommThreadExecute(TMhsThread *thread)
{
uint32_t events;
struct TDeviceStatus status;
struct TCanDevice *can_device;
struct TCanMsg *msg;
TCanDevRxHandler *handler;
int32_t size;

if (!(can_device = (struct TCanDevice *)thread->Data))
  return;
while (thread->Run)
  {
  events = CanExWaitForEvent(can_device->Event, can_device->EventTimeout);
  if (events & 0x10)         // Beenden Event, Thread Schleife verlassen
    break;
  TimeNow = get_tick();
  if (events & STATUS_EVENT)
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
      can_device->Status = DEV_STATUS_CLOSE;
    }
  if (events & RX_EVENT)     // CAN Rx Event
    {
    while ((size = CanReceive(can_device->DeviceIndex, can_device->RxTempBuffer, RX_TEMP_BUFFER_SIZE)) > 0)
      {
      if (!thread->Run)
        break;
      for (msg = can_device->RxTempBuffer; size; size--)
        {
        mhs_enter_critical((TMhsEvent *)can_device->Thread);
        for (handler = can_device->CanDevRxHandler; handler; handler = handler->Next)
          {
          if (handler->Proc)
            (handler->Proc)(can_device, msg, handler->UserData);
          }
        mhs_leave_critical((TMhsEvent *)can_device->Thread);
        msg++;
        }
      }
    }
  if ((!events) && (can_device->EventTimeout))  // Timeout
    {
    mhs_enter_critical((TMhsEvent *)can_device->Thread);
    for (handler = can_device->CanDevRxHandler; handler; handler = handler->Next)
      {
      if (handler->Proc)
        (handler->Proc)(can_device, NULL, handler->UserData);
      }
    mhs_leave_critical((TMhsEvent *)can_device->Thread);
    }
  }
}
