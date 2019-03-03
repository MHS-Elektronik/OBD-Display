/*******************************************************************************
                            cpuf.c  -  description
                             -------------------
    begin             : 28.02.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include "can_types.h"
#include "util.h"
#include "mhs_thread.h"
#include "mhs_g_event.h"
#include "cbuf.h"

/*
#define CBufLockI(l)   mhs_enter_critical((TMhsEvent *)l->Thread)
#define CBufUnlockI(l) mhs_leave_critical((TMhsEvent *)l->Thread)
*/
#define CBufLock(l) do {if ((l)->Callback) mhs_enter_critical((TMhsEvent *)l->Thread);} while(0)
#define CBufUnlock(l) do {if ((l)->Callback) mhs_leave_critical((TMhsEvent *)l->Thread);} while(0)


static void ThreadExecute(TMhsThread *thread);


static gint EventProc(guint events, gpointer user_data)
{
TCanBuffer *cbuf;

cbuf = (TCanBuffer *)user_data;
if (cbuf->Callback)
  return((cbuf->Callback)(cbuf, events, cbuf->UserData));
else
  return(FALSE);
}

/*
******************** CBufCreate ********************
*/
TCanBuffer *CBufCreate(guint reserved_size, gboolean override, guint event_delay,
     guint event_timeout, guint event_limit, CBufGEventCb callback, gpointer user_data)
{
TCanBuffer *cbuf;

if (!reserved_size)
  return(NULL);
if ((cbuf = (TCanBuffer *)g_malloc0(sizeof(TCanBuffer))))
  {
  if (!(cbuf->Data = g_malloc(sizeof(struct TCanMsg) * reserved_size)))
    {
    safe_free(cbuf);
    return(NULL);
    }
  cbuf->Pos = 0;
  cbuf->Len = reserved_size;
  cbuf->Override = override;
  cbuf->EventDelay = event_delay;
  cbuf->EventTimeout = event_timeout;
  cbuf->EventLimit = event_limit;
  cbuf->EventCount = 0;
  cbuf->Events = 0;
  cbuf->UserData = user_data;
  cbuf->Callback = callback;
  if (callback)
    {
    // Thread erzeugen
    cbuf->Thread = mhs_create_thread(ThreadExecute, cbuf, 1, 0);
    mhs_event_set_event_mask((TMhsEvent *)cbuf->Thread, MHS_ALL_EVENTS);
    //cbuf->EventId = g_event_add_full(cbuf, G_PRIORITY_DEFAULT, callback, user_data, NULL); <*>
    cbuf->EventId = mhs_g_event_add((GSourceFunc)EventProc, (gpointer)cbuf);
    mhs_run_thread(cbuf->Thread);
    }
  }
return(cbuf);
}


/*
******************** CBufDestroy ********************
*/
void CBufDestroy(TCanBuffer **cbuf)
{
TCanBuffer *buffer;

if (!cbuf)
  return;
buffer = *cbuf;
if (buffer)
  {
  if (buffer->Callback)
    {
    mhs_destroy_thread(&buffer->Thread, 1000);
    if (buffer->EventId)
      g_source_remove(buffer->EventId);
    }
  safe_free(buffer->Data);
  g_free(buffer);
  }
*cbuf = NULL;
}


gint CBufAddMsgs(TCanBuffer *cbuf, struct TCanMsg *msgs, gint size)
{
gint f;
guint len, pos;

if ((!cbuf) || (!size) || (!msgs))
  return(-1);
CBufLock(cbuf);
len = cbuf->Len;
pos = cbuf->Pos;
if (cbuf->Override)
  {
  for (; size; size--)
    {
    memcpy(&cbuf->Data[pos], msgs++, sizeof(struct TCanMsg) * size);
    if (++pos >= len)
      {
      pos = 0;
      cbuf->Looped = TRUE;
      }
    }
  cbuf->Pos = pos;
  }
else
  {
  f = len - pos;
  if (size > f)
    size = f;
  if (size)
    {
    memcpy(&cbuf->Data[pos], msgs, sizeof(struct TCanMsg) * size);
    cbuf->Pos += size;
    }
  }
CBufUnlock(cbuf);
if (size > 0)
  {
  cbuf->EventCount += size;
  if (cbuf->Callback)
    mhs_event_set((TMhsEvent *)cbuf->Thread, 0x00000001);
  }
return(size);
}


gint CBufGetMsgByIndex(TCanBuffer *cbuf, struct TCanMsg *dest, guint index)
{
gint res;
guint len, pos;

if ((!cbuf) || (!dest))
  return(-1);
CBufLock(cbuf);
res = 0;
len = cbuf->Len;
pos = cbuf->Pos;
if (cbuf->Override)
  {
  if (cbuf->Looped == FALSE)
    {
    if (index >= pos)
      res = -1;
    }
  else
    {
    if (index >= len)
      res = -1;
    else
      {
      index += pos;
      if (index >= len)
        index -= len;
      }
    }
  }
else
  {
  if (index >= pos)
    res = -1;
  }
if (res >= 0)
  memcpy(dest, &cbuf->Data[index], sizeof(struct TCanMsg));
CBufUnlock(cbuf);
return(res);
}


/*
******************** CBufDataClear ********************
*/
gint CBufDataClear(TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
CBufLock(cbuf);
cbuf->Looped = FALSE;
cbuf->Pos = 0;
CBufUnlock(cbuf);
return(0);
}


/*
******************** CBufGetSize ********************
Funktion  : Maximale Puffergröße abfragen
*/
gint CBufGetSize(TCanBuffer *cbuf)
{
if (!cbuf)
  return(-1);
if (!cbuf->Data)
  return(-1);
if (cbuf->Override)
  {
  if (cbuf->Looped == FALSE)
    return((int)cbuf->Pos);
  else
    return((int)cbuf->Len);
  }
else
  return((int)cbuf->Pos);
}


void CBufSetEvents(TCanBuffer *cbuf, guint events)
{
if (!cbuf)
  return;
mhs_event_set((TMhsEvent *)cbuf->Thread, events);
}


/****************/
/* Event Thread */
/****************/
static void ThreadExecute(TMhsThread *thread)
{
TCanBuffer *cbuf;
gint hit;
guint event_limit;
uint32_t sleep_time, event_delay, events;
gint64 time, event_timeout;

cbuf = (TCanBuffer *)thread->Data;
event_timeout = (gint64)cbuf->EventTimeout * 1000;
event_delay = cbuf->EventDelay;
event_limit = cbuf->EventLimit;
sleep_time = 0;
while (thread->Run)
  {
  events = mhs_wait_for_event((TMhsEvent *)thread, sleep_time);
  if (events & MHS_TERMINATE)
    break;
  sleep_time = 0;
  hit = 0;
  time = g_get_monotonic_time();
  if ((event_limit) && (cbuf->EventCount >= event_limit))
    hit = 1;
  else if ((cbuf->EventCount) && (time >= (cbuf->EventTime + event_timeout)))
    hit = 1;
  else if (events & 0x00000001)
    {
    if (event_delay)
      sleep_time = event_delay;
    else
      hit = 1;
    }
  else if (cbuf->EventCount)
    hit = 1;
  if (hit)
    {
    cbuf->EventCount = 0;
    cbuf->EventTime = time;
    // Event Callback triggern
    if (mhs_g_set_events(cbuf->EventId, 1))
        break;
    }
  }
}


