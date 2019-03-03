#ifndef __MHS_G_MESSAGES_H__
#define __MHS_G_MESSAGES_H__

//#include <stdint.h>
#include <glib.h>
#include "util.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMhsGMessage TMhsGMessage;

struct _TMhsGMessage
  {
  guint MessageType;
  guint Size;
  guchar Data[1];
  };


typedef struct _TMhsGScheduler TMhsGScheduler;

struct _TMhsGScheduler
  {
  GSource Source;
  GAsyncQueue *Queue;
  guint SourceId;
  gboolean AutoFreeMessage;
  };

typedef void(*MhsGMessageCb)(TMhsGMessage *msg, gpointer user_data);

#define mhs_g_message_free(msg)  safe_free(msg)

TMhsGMessage *mhs_g_new_message(guint msg_type, gpointer data, guint size);
TMhsGMessage *mhs_g_new_message_from_string(guint msg_type, const gchar *str);

void mhs_g_message_post(TMhsGScheduler *scheduler, TMhsGMessage *msg);
TMhsGScheduler *mhs_g_message_scheduler_create(MhsGMessageCb callback, gpointer user_data, gboolean auto_free_message);
void mhs_g_message_scheduler_destroy(TMhsGScheduler *scheduler);

#ifdef __cplusplus
  }
#endif

#endif

