#ifndef __C_BUFFER_H__
#define __C_BUFFER_H__

#include <glib.h>
#include "can_types.h"
#include "mhs_thread.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TCanBuffer TCanBuffer;

typedef gint(*CBufGEventCb)(TCanBuffer *cbuf, guint events, gpointer user_data);

struct _TCanBuffer
  {
  struct TCanMsg *Data;
  guint Len;
  guint Pos;
  gboolean Override;

  guint EventTimeout;
  guint EventDelay;
  guint EventLimit;

  gboolean Looped;
  volatile guint Events;
  guint EventCount;
  gint64 EventTime;
  gpointer UserData;
  CBufGEventCb Callback;
  TMhsThread *Thread;

  guint EventId;
  };


TCanBuffer *CBufCreate(guint reserved_size, gboolean override, guint event_delay,
     guint event_timeout, guint event_limit, CBufGEventCb callback, gpointer user_data);
void CBufDestroy(TCanBuffer **cbuf);
gint CBufAddMsgs(TCanBuffer *cbuf, struct TCanMsg *msgs, gint size);
gint CBufGetMsgByIndex(TCanBuffer *cbuf, struct TCanMsg *dest, guint index);
int CBufDataClear(TCanBuffer *cbuf);
gint CBufGetSize(TCanBuffer *cbuf);
void CBufSetEvents(TCanBuffer *cbuf, guint events);

#ifdef __cplusplus
  }
#endif

#endif
