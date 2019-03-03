/***************************************************************************
                          mhs_g_event.c  -  description
                             -------------------
    begin             : 09.06.2012
    last modify       : 07.06.2015
    copyright         : (C) 2009 - 2015 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include <glib.h>
#include "mhs_g_event.h"

typedef struct _MhsGEventSource MhsGEventSource;

struct _MhsGEventSource
  {
  GSource source;
  volatile guint events;
  volatile guint events_mask;
  };

//static CRITICAL_SECTION EventLock;

static gboolean mhs_g_event_prepare(GSource *source, gint *timeout);
static gboolean mhs_g_event_check(GSource *source);
static gboolean mhs_g_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);


#ifndef GTK3
/*
static guint g_atomic_int_and(volatile guint *atomic, guint val)
{
guint oldval;

EnterCriticalSection(&EventLock);
oldval = *atomic;
*atomic = oldval & val;
LeaveCriticalSection(&EventLock);
return(oldval);
}

static guint g_atomic_int_or(volatile guint *atomic, guint val)
{
guint oldval;

EnterCriticalSection(&EventLock);
oldval = *atomic;
*atomic = oldval | val;
LeaveCriticalSection(&EventLock);
return(oldval);
}*/

#endif

GSourceFuncs mhs_g_event_funcs =
  {
  mhs_g_event_prepare,
  mhs_g_event_check,
  mhs_g_event_dispatch,
  NULL,
  NULL,
  NULL
  };

/* Timeouts */
static gboolean mhs_g_event_prepare(GSource *source, gint *timeout)
{
MhsGEventSource *event_source;

event_source = (MhsGEventSource *)source;
*timeout = -1;
if (event_source->events & event_source->events_mask)
  return(TRUE);
else
  return(FALSE);
}


static gboolean mhs_g_event_check(GSource *source)
{
MhsGEventSource *event_source;

event_source = (MhsGEventSource *)source;
if (event_source->events & event_source->events_mask)
  return(TRUE);
else
  return(FALSE);
}


static gboolean mhs_g_event_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
MhsGEventSource *event_source;
MhsGEventCb func;
guint events, mask;

event_source = (MhsGEventSource *)source;
func = (MhsGEventCb)callback;
if (!func)
  {
  g_warning ("Event source dispatched without callback\n"
             "You must call g_source_set_callback().");
  return(FALSE);
  }
mask = event_source->events_mask;
if ((events = g_atomic_int_and(&event_source->events, ~mask)))
  {
  events &= mask;
  if (events)
    return((*func)(events, user_data));
  else
    return(TRUE);
  }
else
  return(TRUE);
}



GSource *mhs_g_event_source_new(void)
{
GSource *source;
MhsGEventSource *event_source;

source = g_source_new(&mhs_g_event_funcs, sizeof(MhsGEventSource));
event_source = (MhsGEventSource *)source;
event_source->events = 0;
event_source->events_mask = 0xFFFFFFFF;
return(source);
}


guint mhs_g_event_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify notify)
{
GSource *source;
guint id;

g_return_val_if_fail (function != NULL, 0);

source = mhs_g_event_source_new();

if (priority != G_PRIORITY_DEFAULT)
  g_source_set_priority (source, priority);

g_source_set_callback(source, function, data, notify);
id = g_source_attach(source, NULL);
g_source_unref(source);

return(id);
}


guint mhs_g_event_add(GSourceFunc function, gpointer data)
{
return(mhs_g_event_add_full(G_PRIORITY_DEFAULT, function, data, NULL));
}


gint mhs_g_event_set_mask(guint id, guint mask)
{
MhsGEventSource *event_source;

if (!id)
  return(-1);
event_source = (MhsGEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
event_source->events_mask = mask;
return(0);
}


gint mhs_g_reset_events(guint id, guint events)
{
MhsGEventSource *event_source;

if (!id)
  return(-1);
event_source = (MhsGEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
(void)g_atomic_int_and(&event_source->events, ~events);
return(0);
}


gint mhs_g_set_events(guint id, guint events)
{
MhsGEventSource *event_source;

if (!id)
  return(-1);
event_source = (MhsGEventSource *)g_main_context_find_source_by_id(NULL, id);
if (!event_source)
  return(-1);
(void)g_atomic_int_or(&event_source->events, events);
g_main_context_wakeup(NULL);
return(0);
}


void mhs_g_init_events(void)
{
//InitializeCriticalSection(&EventLock);
}


void mhs_g_destroy_events(void)
{
//DeleteCriticalSection(&EventLock);
}
