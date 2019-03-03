#ifndef __MSG_WIDGET_H__
#define __MSG_WIDGET_H__

#include <glib.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define MSG_WDG_STATUS_BUSY     0
#define MSG_WDG_STATUS_OK       1
#define MSG_WDG_STATUS_WARN     2
#define MSG_WDG_STATUS_ERROR    3
#define MSG_WDG_STATUS_STOP     4
#define MSG_WDG_STATUS_QUESTION 5
#define MSG_WDG_STATUS_WAIT     6

GtkWidget *CreateMsgWidget(guint status, const gchar *text);
void MsgWidgetAddCustomWidget(GtkWidget *msg_widget, GtkWidget *widget);
void MsgWidgetDestroyCustomWidget(GtkWidget *msg_widget);
void UpdateMsgWidget(GtkWidget *widget, guint status, const gchar *text);

#ifdef __cplusplus
  }
#endif

#endif