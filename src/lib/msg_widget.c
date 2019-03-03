/*******************************************************************************
                        msg_widget.c  -  description
                             -------------------
    begin             : 09.09.2017
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
#include <gtk/gtk.h>
#include "gtk-ex-frame.h"
#include "obd_disp_icons.h"
#include "iconcache.h"
#include "msg_widget.h"

static GdkColor DefFgColor = { 0, 0xFFFF, 0xFFFF, 0xFFFF };  // weiß

struct TMsgWidget
  {
  GtkWidget *VBox;
  GtkWidget *Image;
  GtkWidget *Label;
  GtkWidget *CustomWidget;
  };


static void MsgWidgetDestroy(GtkWidget *widget, gpointer data)
{
struct TMsgWidget *msg_widget;
(void)widget;

msg_widget = (struct TMsgWidget *)data;
g_free(msg_widget);
}


GtkWidget *CreateMsgWidget(guint status, const gchar *text)
{
struct TMsgWidget *msg_widget;
GtkWidget *vbox, *box, *widget, *w;

if (!(msg_widget = (struct TMsgWidget *)g_malloc0(sizeof(struct TMsgWidget))))
  return(NULL);
widget = gtk_ex_frame_new();
gtk_widget_modify_bg(GTK_WIDGET(widget), GTK_STATE_NORMAL, &DefFgColor);
g_object_set_data(G_OBJECT(widget), "msg_widget", msg_widget);
g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(MsgWidgetDestroy), msg_widget);

vbox = gtk_vbox_new(FALSE, 5);
gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
gtk_container_add (GTK_CONTAINER(widget), vbox);

box = gtk_hbox_new(FALSE, 5);
gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 0);
msg_widget->VBox = vbox;
// **** Icon
switch (status)
  {
  case MSG_WDG_STATUS_BUSY  : {
                              w = mhs_get_image(MSG_WDG_BUSY_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_OK    : {
                              w = mhs_get_image(MSG_WDG_OK_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_WARN  : {
                              w = mhs_get_image(MSG_WDG_WARN_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_ERROR : {
                              w = mhs_get_image(MSG_WDG_ERROR_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_STOP  : {
                              w = mhs_get_image(MSG_WDG_STOP_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_QUESTION :
                              {
                              w = mhs_get_image(MSG_WDG_QUESTION_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_WAIT  : {
                              w = mhs_get_image(MSG_WDG_USER_AKTION_ICONE);
                              break;
                              }
  default                   : {
                              w = NULL;
                              }
  }
gtk_box_pack_start(GTK_BOX(box), w, FALSE, FALSE, 0);
msg_widget->Image = w;
// **** Text
w = gtk_label_new(NULL);
gtk_misc_set_alignment(GTK_MISC(w), 0.0, 0.5);
gtk_label_set_markup(GTK_LABEL(w), text);
gtk_box_pack_start(GTK_BOX(box), w, TRUE, TRUE, 0);
msg_widget->Label = w;
gtk_widget_show_all(widget);
return(widget);
}


void MsgWidgetAddCustomWidget(GtkWidget *msg_widget, GtkWidget *widget)
{
struct TMsgWidget *msg_widget_data;

if (!(msg_widget_data = (struct TMsgWidget *)g_object_get_data(G_OBJECT(msg_widget), "msg_widget")))
  return;
msg_widget_data->CustomWidget = widget;
gtk_box_pack_start(GTK_BOX(msg_widget_data->VBox), widget, FALSE, FALSE, 0);
gtk_widget_show_all(widget);
}


void MsgWidgetDestroyCustomWidget(GtkWidget *msg_widget)
{
struct TMsgWidget *msg_widget_data;

if (!(msg_widget_data = (struct TMsgWidget *)g_object_get_data(G_OBJECT(msg_widget), "msg_widget")))
  return;
if (msg_widget_data->CustomWidget)
  {
  gtk_widget_destroy(msg_widget_data->CustomWidget);
  msg_widget_data->CustomWidget = NULL;
  }
}


void UpdateMsgWidget(GtkWidget *widget, guint status, const gchar *text)
{
struct TMsgWidget *msg_widget;

if (!(msg_widget = (struct TMsgWidget *)g_object_get_data(G_OBJECT(widget), "msg_widget")))
  return;
switch (status)
  {
  case MSG_WDG_STATUS_BUSY  : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_BUSY_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_OK    : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_OK_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_WARN  : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_WARN_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_ERROR : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_ERROR_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_STOP  : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_STOP_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_QUESTION :
                              {
                              mhs_set_image(msg_widget->Image, MSG_WDG_QUESTION_ICONE);
                              break;
                              }
  case MSG_WDG_STATUS_WAIT  : {
                              mhs_set_image(msg_widget->Image, MSG_WDG_USER_AKTION_ICONE);
                              break;
                              }
  }
gtk_label_set_markup(GTK_LABEL(msg_widget->Label), text);
}
