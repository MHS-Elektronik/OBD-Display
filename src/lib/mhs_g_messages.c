/***************************************************************************
                          mhs_g_event.c  -  description
                             -------------------
    begin             : 09.06.2012
    last modify       : 07.06.2015
    copyright         : (C) 2009 - 2015 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/
#include <glib.h>
#include <string.h>
#include "mhs_g_messages.h"


static gboolean mhs_g_message_source_prepare(GSource *gsource, gint *timeout);
static gboolean mhs_g_message_source_check(GSource *gsource);
static gboolean mhs_g_message_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static void mhs_g_message_source_finalize (GSource *gsource);

static GSourceFuncs source_funcs = {
	mhs_g_message_source_prepare,
	mhs_g_message_source_check,
	mhs_g_message_source_dispatch,
	mhs_g_message_source_finalize,
  NULL,
  NULL
};



TMhsGMessage *mhs_g_new_message(guint msg_type, gpointer data, guint size)
{
TMhsGMessage *msg;

if (!(msg = (TMhsGMessage *)g_malloc((sizeof(TMhsGMessage) - 1) + size)))
  return(NULL);
msg->MessageType = msg_type;
msg->Size = size;
if (data)
  memcpy(msg->Data, data, size);
return(msg);
}


TMhsGMessage *mhs_g_new_message_from_string(guint msg_type, const gchar *str)
{
guint size;

size = 0;
if (str)
  size = strlen(str) + 1;
return(mhs_g_new_message(msg_type, (gpointer)str, size));
}


void mhs_g_message_post(TMhsGScheduler *scheduler, TMhsGMessage *msg)
{
if ((!scheduler) || (!msg))
  return;
g_async_queue_push(scheduler->Queue, (gpointer)msg);
g_main_context_wakeup(NULL);
}


TMhsGScheduler *mhs_g_message_scheduler_create(MhsGMessageCb callback, gpointer user_data, gboolean auto_free_message)
{
GSource *gsource;
TMhsGScheduler *scheduler;

gsource = g_source_new(&source_funcs, sizeof(TMhsGScheduler));
//g_source_set_priority (gsource, G_PRIORITY_DEFAULT_IDLE);
g_source_set_callback(gsource, (GSourceFunc)callback, user_data, NULL);

scheduler = (TMhsGScheduler *)gsource;
scheduler->Queue = g_async_queue_new_full(g_free);
scheduler->AutoFreeMessage = auto_free_message;
scheduler->SourceId = g_source_attach(gsource, NULL);
g_source_unref (gsource);
return (scheduler);
}


void mhs_g_message_scheduler_destroy(TMhsGScheduler *scheduler)
{
if (scheduler)
  {
  g_async_queue_unref(scheduler->Queue);
  g_source_remove(scheduler->SourceId);
  }
}


static gboolean mhs_g_message_source_prepare(GSource *gsource, gint *timeout)
{
TMhsGScheduler *scheduler;

scheduler = (TMhsGScheduler *)gsource;
*timeout = -1;
return(g_async_queue_length(scheduler->Queue) > 0);
}


static gboolean mhs_g_message_source_check(GSource *gsource)
{
TMhsGScheduler *scheduler;

scheduler = (TMhsGScheduler *)gsource;
return(g_async_queue_length(scheduler->Queue) > 0);
}


static gboolean mhs_g_message_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
TMhsGScheduler *scheduler;
TMhsGMessage *msg;
MhsGMessageCb func;
gint i;

if (!callback)
  return(TRUE);
scheduler = (TMhsGScheduler *)source;
func = (MhsGMessageCb)callback;
for (i = 100; i; i--)
  {
  if (!(msg = (TMhsGMessage *)g_async_queue_try_pop(scheduler->Queue)))
    break;
  (func)(msg, user_data);
  if (scheduler->AutoFreeMessage)
    g_free(msg);
  }
return(TRUE);
}


static void mhs_g_message_source_finalize(GSource *gsource)
{
TMhsGScheduler *scheduler;

scheduler = (TMhsGScheduler *)gsource;
g_async_queue_unref(scheduler->Queue);
}



