/*******************************************************************************
                           isotp.c  -  description
                             -------------------
    begin             : 25.01.2019
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
#include <string.h>
#include "util.h"
#include "mhs_thread.h"
#include "mhs_queue.h"
#include "can_device.h"
#include "isotp.h"


static int32_t IsotpSendFc(struct TIsotp *tp, uint8_t flowstatus)
{
uint8_t idx;
struct TCanMsg msg;

// **** Message Format
msg.Flags.Long = 0L;  // RTR = 0, EFF = 0
if (tp->Flags & CAN_ISOTP_29BIT_ID)
  {
  msg.Flags.Flag.EFF = 1;
  msg.Id = ((tp->RxId & 0x000000FF ) << 8) | 0x18DA00F1;
  }
else 
  msg.Id = tp->RxId - 8;

idx = 0;
if (tp->Flags & CAN_ISOTP_TX_PADDING)
  {
  memset(&msg.MsgData[0], tp->TxPadContent, 8);
  msg.MsgLen = 8;
  }
else
  {
  if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
    msg.MsgLen = 4;
  else
    msg.MsgLen = 3;
  } 
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  msg.MsgData[idx++] = tp->ExtAddress;
msg.MsgData[idx++] = N_PCI_FC | flowstatus;
msg.MsgData[idx++] = tp->Rx.FcBs;
msg.MsgData[idx++] = tp->Rx.FcStmin;
if (CanDevTxMessage(tp->CanDevice, &msg, 1) < 0)
  return(-1);
/* reset blocksize counter */
tp->Rx.BsCounter = 0;
return(0);
}


static void IsotpRxPkt(struct TIsotp *tp, const uint8_t *data, uint32_t size)
{
(void)mhs_queue_push(tp->RxQueue, data, size);
mhs_event_set(tp->Event, ISOTP_RX_EVENT);
}


static void IsotpTxSet(struct TIsotp *tp, uint32_t events, uint32_t wait, uint8_t state)
{
tp->Tx.State = state;
tp->Tx.Timestamp = get_tick();
tp->TimeoutSet = wait;
if (events)
  mhs_event_set((TMhsEvent *)tp->Thread, events);
}


// ******* Daten Empfangen
static void IsotpRxDataHandler(struct TCanDevice *can_device, struct TCanMsg *msg, void *user_data)
{
struct TIsotp *tp;
uint8_t pci, len, idx;
uint32_t size;

if (!(tp = (struct TIsotp *)user_data))
  return;
// Check Rx Timeout
if (tp->Rx.State == ISOTP_WAIT_DATA)
  {
  if (mhs_diff_time(TimeNow, tp->Rx.Timestamp) > 1100)
    tp->Rx.State = ISOTP_IDLE;
  }
if (!msg)  // Timeout Call
  return;
if (msg->Id != tp->RxId)
  return;
if (tp->Flags & CAN_ISOTP_29BIT_ID)
  {
  if (!msg->Flags.Flag.EFF)
    return;
  }
else
  {
  if (msg->Flags.Flag.EFF)
    return;
  }
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  {
  if ((len = msg->MsgLen) < 2)
    return;  // <*> Fehler
  // ISO-TP Extended Adresse prüfen
  if (msg->MsgData[0] != tp->ExtRxAddress)
    return;
  idx = 1;
  len -= 2;
  }
else
  {
  if ((len = msg->MsgLen) < 1)
    return;  // <*> Fehler
  idx = 0;
  len--;
  }

pci = msg->MsgData[idx++];
switch (pci & 0xF0)
  {
// ***************** flow control frame
  case N_PCI_FC :
      {
      if ((tp->Tx.State != ISOTP_WAIT_FC) &&
          (tp->Tx.State != ISOTP_WAIT_FIRST_FC))
        break;
      if (len < 3)
        {
        IsotpTxSet(tp, ISOTP_TX_CANCEL, 0, ISOTP_IDLE);
        //tp->Tx.State = ISOTP_IDLE;
        // <*> Fehler
        break;
        }

      // 1. FC Frame empfangen ?
      if (tp->Tx.State == ISOTP_WAIT_FIRST_FC)
        {
        tp->Tx.FcBs = msg->MsgData[idx++];
        tp->Tx.FcStmin = msg->MsgData[idx];

        // fix wrong STmin values according spec
        if ((tp->Tx.FcStmin > 0x7F) &&
           ((tp->Tx.FcStmin < 0xF1) ||
            (tp->Tx.FcStmin > 0xF9)))
          tp->Tx.FcStmin = 0x7F;

        // <*> frame_txtime ??
        /*if (so->opt.flags & CAN_ISOTP_FORCE_TXSTMIN)
          force_tx_stmin);
        else if (so->txfc.stmin < 0x80)
          so->txfc.stmin * 1000000  // ns
        else
          (so->txfc.stmin - 0xF0) * 100000; */
        tp->Tx.State = ISOTP_WAIT_FC;
        }
      switch (pci & 0x0F)
        {
        case ISOTP_FC_CTS :
            {
            tp->Tx.BsCounter = 0;
            IsotpTxSet(tp, ISOTP_TX_TIMEOUT_SET, tp->Tx.FcStmin, ISOTP_SENDING);  // Wartezeit ?
            //tp->Tx.State = ISOTP_SENDING;
            /* start cyclic timer for sending CF frame */
            break;
            }
        case ISOTP_FC_WT :
            {
            /* start timer to wait for next FC frame */
            IsotpTxSet(tp, ISOTP_TX_TIMEOUT_SET, 1000, ISOTP_WAIT_FC);
            // <*>
            break;
            }
        case ISOTP_FC_OVFLW :
            {
            // <*> Fehlerbehandlung
            //tp->Tx.State = ISOTP_IDLE;
            IsotpTxSet(tp, ISOTP_TX_CANCEL, 0, ISOTP_IDLE);
            break;
            }
        default :
            //tp->Tx.State = ISOTP_IDLE;
            IsotpTxSet(tp, ISOTP_TX_CANCEL, 0, ISOTP_IDLE);
        }
      break;
      }

// *********** single frame
  case N_PCI_SF:
      {
      tp->Rx.State = ISOTP_IDLE;
      if ((!len) || ((pci & 0x0F) > len))
        break;
      size = pci & 0x0F;
      IsotpRxPkt(tp, &msg->MsgData[idx], size);
      break;
      }
// ******************* first frame
  case N_PCI_FF:
      {
      tp->Rx.State = ISOTP_IDLE;
      if (!len)
        break;   // <*> Frame Format Fehler
      // Paket größe ermitteln
      len--;
      size = ((pci & 0x0F) << 8) | msg->MsgData[idx++];
      if (!size) // 32 bit PDU length
        {
        if (len < 4)
          break; // <*> Frame Format Fehler
        len -= 4;
        size =  msg->MsgData[idx++] << 24;
        size += msg->MsgData[idx++] << 16;
        size += msg->MsgData[idx++] << 8;
        size += msg->MsgData[idx++];
        }
      tp->Rx.Size = size;
      if (size > MAX_MSG_LENGTH)
        {
        if (tp->Flags & CAN_ISOTP_LISTEN_MODE)
          IsotpSendFc(tp, ISOTP_FC_OVFLW);
        break;
        }
      if (len > size)
        len = size;
      if (len)
        memcpy(tp->Rx.Buf, &msg->MsgData[idx], len);
      tp->Rx.Count = len;
      tp->Rx.Ptr = tp->Rx.Buf + len;

      // initial setup for this pdu receiption
      tp->Rx.PktIdx = 1;
      tp->Rx.State = ISOTP_WAIT_DATA;
      tp->Rx.Timestamp = TimeNow;

      if (!(tp->Flags & CAN_ISOTP_LISTEN_MODE))
        // send our first FC frame
        (void)IsotpSendFc(tp, ISOTP_FC_CTS);
      break;
      }
// ************************* consecutive frame
  case N_PCI_CF:
      {
      if (tp->Rx.State != ISOTP_WAIT_DATA)
        break;
      tp->Rx.Timestamp = TimeNow;

      if ((pci & 0x0F) != tp->Rx.PktIdx)
        {
        tp->Rx.State = ISOTP_IDLE;  // <*> Fehlerbehandlung
        break;
        }
      tp->Rx.PktIdx = (tp->Rx.PktIdx + 1) & 0x0F;

      size = tp->Rx.Size;
      if (tp->Rx.Count + len > size)
        len = size - tp->Rx.Count;
      if (len)
        {
        memcpy(tp->Rx.Ptr, &msg->MsgData[idx], len);
        tp->Rx.Count += len;
        tp->Rx.Ptr += len;
        }
      if (tp->Rx.Count >= size)
        {  // Alle Pakete empfangen
        tp->Rx.State = ISOTP_IDLE;
        IsotpRxPkt(tp, tp->Rx.Buf, size);
        break;
        }

      if (tp->Flags & CAN_ISOTP_LISTEN_MODE)
        break;

      /* perform blocksize handling, if enabled */
      if (!tp->Rx.FcBs || ++tp->Rx.BsCounter < tp->Rx.FcBs)
        {
        /* start rx timeout watchdog */
        break;
        }

      /* we reached the specified blocksize so->rxfc.bs */
      (void)IsotpSendFc(tp, ISOTP_FC_CTS);

      break;
      }
  }
}


static int32_t IsotpSendSf(struct TIsotp *tp)
{
uint8_t idx, space, d_len;
uint8_t *p;
struct TCanMsg msg;

idx = 0;
// **** Message Format
msg.Flags.Long = 0L;  // RTR = 0, EFF = 0
if (tp->Flags & CAN_ISOTP_29BIT_ID)
  msg.Flags.Flag.EFF = 1;
msg.Id = tp->TxId;
d_len = tp->Tx.Size;
space = 7;
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  {
  msg.MsgData[idx++] = tp->ExtAddress;
  space--;
  }
msg.MsgData[idx++] = N_PCI_SF | d_len;
space -= d_len;
for (p = tp->Tx.Buf; d_len; d_len--)
  msg.MsgData[idx++] = *p++;
// Padding
if (tp->Flags & CAN_ISOTP_TX_PADDING)
  {
  for (; space; space--)
    msg.MsgData[idx++] = tp->TxPadContent;
  }
msg.MsgLen = idx;
if (CanDevTxMessage(tp->CanDevice, &msg, 1) < 0)
  return(-1);
return(0);
}


static int32_t IsotpSendFf(struct TIsotp *tp)
{
uint8_t idx, len;
uint32_t size;
struct TCanMsg msg;

idx = 0;
// **** Message Format
msg.Flags.Long = 0L;  // RTR = 0, EFF = 0
if (tp->Flags & CAN_ISOTP_29BIT_ID)
  msg.Flags.Flag.EFF = 1;
msg.Id = tp->TxId;
msg.MsgLen = 8;
len = 8;
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  {
  msg.MsgData[idx++] = tp->ExtAddress;
  len--;
  }
size = tp->Tx.Size;
if (size > 4095)
  {
  // use 32 bit FF_DL notation
  msg.MsgData[idx++] = N_PCI_FF;
  msg.MsgData[idx++] = 0;
  msg.MsgData[idx++] = (uint8_t)(size >> 24) & 0xFF;
  msg.MsgData[idx++] = (uint8_t)(size >> 16) & 0xFF;
  msg.MsgData[idx++] = (uint8_t)(size >> 8) & 0xFF;
  msg.MsgData[idx++] = (uint8_t)size & 0xFF;
  len -= 6;
  }
else
  {
  // use 12 bit FF_DL notation
  msg.MsgData[idx++] = (uint8_t)(size >> 8) | N_PCI_FF;
  msg.MsgData[idx++] = (uint8_t)size & 0xFF;
  len -= 2;
  }
tp->Tx.Count = len;
tp->Tx.Ptr = tp->Tx.Buf;
for (; len; len--)
  msg.MsgData[idx++] = *tp->Tx.Ptr++;

tp->Tx.PktIdx = 1;
tp->Tx.State = ISOTP_WAIT_FIRST_FC;
if (CanDevTxMessage(tp->CanDevice, &msg, 1) < 0)
  return(-1);
return(0);
}


static int32_t IsotpSendCf(struct TIsotp *tp)
{
int32_t res;
uint8_t idx, space, d_len;
uint32_t size;
struct TCanMsg msg;

res = 0;
// **** Message Format
msg.Flags.Long = 0L;  // RTR = 0, EFF = 0
if (tp->Flags & CAN_ISOTP_29BIT_ID)
  msg.Flags.Flag.EFF = 1;
msg.Id = tp->TxId;

size = tp->Tx.Size;
idx = 0;
space = 7;
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  {
  msg.MsgData[idx++] = tp->ExtAddress;
  space--;
  }
if (size <= space)
  {
  res = 1;
  d_len = size;
  }
else
  d_len = space;
// Header - PCI
msg.MsgData[idx++] = N_PCI_CF | tp->Tx.PktIdx;
tp->Tx.PktIdx = (tp->Tx.PktIdx + 1) & 0x0F;
// Daten
tp->Tx.Count += d_len;
space -= d_len;
for (; d_len; d_len--)
  msg.MsgData[idx++] = *tp->Tx.Ptr++;
// Padding
if (tp->Flags & CAN_ISOTP_TX_PADDING)
  {
  for (; space; space--)
    msg.MsgData[idx++] = tp->TxPadContent;
  }
msg.MsgLen = idx;
tp->Tx.BsCounter++;
if (CanDevTxMessage(tp->CanDevice, &msg, 1) < 0)
  return(-1);
return(res);
}


static void TxThreadExecute(TMhsThread *thread)
{
struct TIsotp *tp;
uint32_t timeout, events; //, time_now;
uint8_t st_min, bs;
int32_t res;

if (!(tp = (struct TIsotp *)thread->Data))
  return;
timeout = 0;
while (thread->Run)
  {
  events = mhs_wait_for_event((TMhsEvent *)tp->Thread, timeout);
  timeout = 0;
  if (events & MHS_TERMINATE)
    break;
  if (events & ISOTP_TX_CANCEL)
    continue;
  //time_now = GetTickCount(); <*>
  if (events & ISOTP_TX_TIMEOUT_SET)
    {
    timeout = tp->TimeoutSet;
    continue;
    }
  if (tp->Tx.State == ISOTP_SENDING)// (events & ISOTP_TX_FC_EVENT)
    {
    st_min = tp->Tx.FcStmin;
    bs = tp->Tx.FcBs;

    while ((res = IsotpSendCf(tp)) == 0)
      {
      if ((bs) && (tp->Tx.BsCounter >= bs))
        {
        tp->Tx.State = ISOTP_WAIT_FC;
        timeout = 1000; // FC Timeout 1 Sek.
        break;
        }
      if (st_min)
        {
        if (mhs_sleep_ex((TMhsEvent *)tp->Thread, (uint32_t)st_min))
          break;
        }
      }
    if (res > 0)
      {
      // <*> Tx Ok
      tp->Tx.State = ISOTP_IDLE;
      }
    else if (res < 0)
      {
      // <*> Tx Error
      tp->Tx.State = ISOTP_IDLE;
      }
    }
  else if ((tp->Tx.State == ISOTP_WAIT_FC) ||
	        (tp->Tx.State == ISOTP_WAIT_FIRST_FC))
    {
    // <*> Timeout Error
    tp->Tx.State = ISOTP_IDLE;
    }
  }
}



struct TIsotp *IsotpCreate(struct TCanDevice *can_device, uint32_t flags)
{
int32_t err;
struct TIsotp *tp;

if (!can_device)
  return(NULL);
if (!(tp = (struct TIsotp *)mhs_malloc0(sizeof(struct TIsotp))))
  return(NULL);
tp->Flags = flags;
tp->CanDevice = can_device;
tp->RxQueue = mhs_queue_create(0, 1, NULL, NULL);
tp->Event = mhs_event_create();
mhs_event_set_event_mask(tp->Event, MHS_ALL_EVENTS);
CanDevRxEventConnect(can_device, IsotpRxDataHandler, tp);

if (!(tp->Thread = mhs_create_thread(TxThreadExecute, tp, MHS_THREAD_PRIORITY_NORMAL, 0)))
  err = -1;
else
  {
  mhs_event_set_event_mask((TMhsEvent *)tp->Thread, MHS_ALL_EVENTS);
  mhs_run_thread(tp->Thread);
  }
if (err < 0)
  IsotpDestroy(&tp);
return(tp);
}


void IsotpDestroy(struct TIsotp **tp)
{
struct TIsotp *isotp;

if (!tp)
  return;
if (!(isotp = *tp))
  return;
if (isotp->Thread)
  {
  (void)mhs_join_thread(isotp->Thread, 1000);
  mhs_destroy_thread(&isotp->Thread, 0);
  }
CanDevRxEventDisconnect(isotp->CanDevice, IsotpRxDataHandler);
mhs_event_destroy(&isotp->Event);
mhs_queue_destroy(&isotp->RxQueue);
mhs_free(isotp);
*tp = NULL;
}


void IsotpSetup(struct TIsotp *tp, uint32_t flags, uint8_t bs, uint8_t st_min, uint8_t tx_pad_content)
{
if (!tp)
  return;
tp->Flags = flags;
tp->Rx.FcBs = bs;
tp->Rx.FcStmin = st_min;
tp->TxPadContent = tx_pad_content;
}


void IsotpIdSetup(struct TIsotp *tp, uint32_t tx_id, uint32_t rx_id, uint8_t eff)
{
if (!tp)
  return;
if (eff)
  tp->Flags |= CAN_ISOTP_29BIT_ID;
else
  tp->Flags &= (~CAN_ISOTP_29BIT_ID);
tp->TxId = tx_id;
tp->RxId = rx_id;
}


void IsotpExtAdrSetup(struct TIsotp *tp, uint8_t tx_ext_adr, uint8_t rx_ext_adr)
{
if (!tp)
  return;
tp->Flags |= CAN_ISOTP_EXTEND_ADDR;
tp->ExtAddress = tx_ext_adr;
tp->ExtRxAddress = rx_ext_adr;
}


int32_t IsotpSend(struct TIsotp *tp, const uint8_t *data, uint32_t size)
{
uint32_t res, len;

if (tp->Tx.State != ISOTP_IDLE)
  return(-1);
if (!size || size > MAX_MSG_LENGTH)
  return(-1);

memcpy(tp->Tx.Buf, data, size);
tp->Tx.Size = size;
//tp->Tx.State = ISOTP_SENDING; <*>

len = 7;
if (tp->Flags & CAN_ISOTP_EXTEND_ADDR)
  len--;
// check for single frame transmission depending on TX_DL
if (size <= len)
  {
  res = IsotpSendSf(tp);
  //tp->Tx.State = ISOTP_IDLE; <*>
  }
else
  {
  /* send first frame and wait for FC */
  res = IsotpSendFf(tp);
  /* start timeout for FC */
  if (!res)
    IsotpTxSet(tp, ISOTP_TX_TIMEOUT_SET, 1000, ISOTP_WAIT_FIRST_FC);
  }
return(res);
}


int32_t IsotpSendReceive(struct TIsotp *tp, const uint8_t *tx_data, uint32_t tx_size,
        uint8_t **rx_data, uint32_t *rx_size, uint32_t timeout)
{
int32_t err;
uint32_t events;
uint8_t *rx;

if ((!tp) || (!rx_data) || (!rx_size))
  return(-1);
*rx_size = 0;
*rx_data = NULL;
mhs_event_clear(tp->Event, 0xFF); // <*> nur zur Sicherheit
mhs_queue_clear(tp->RxQueue);
if ((err = IsotpSend(tp, tx_data, tx_size)))
  return(err);
events = mhs_wait_for_event(tp->Event, timeout);
if (events & ISOTP_RX_EVENT)
  {
  rx = mhs_queue_pop(tp->RxQueue);
  *rx_size = mhs_queue_get_data_size(rx);
  *rx_data = rx;
  err = 0;
  }
else
  err = -1;
return(err);
}



