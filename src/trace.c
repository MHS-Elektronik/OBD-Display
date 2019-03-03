/*******************************************************************************
                           trace.c  -  description
                             -------------------
    begin             : 28.01.2019
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
#include "util.h"
#include "can_device.h"
#include "cbuf.h"
#include "etable.h"
#include "trace.h"


#define CAN_BUFFER_SIZE 1000000

#define TRACE_SHOW_ALL           0
#define TRACE_SHOW_ONLY_KNOWN    1
#define TRACE_SHOW_ONLY_UNKNOWN  2

#define RX_EVENT     0x00000001

struct TTraceWin
  {
  struct TCanDevice *CanDevice;
  GtkWidget *VBox;
  GtkWidget *Toolbar;
  GtkToolItem *NeuButton;
  GtkToolItem *CanStartStopButton;
  //GtkWidget *FilterSelectComboBox; <*>
  GtkWidget *ETable;
  TCanBuffer *Buffer;
  guint CanStartStopClickId;
  guint Run;
  //guint TraceFilter;
  };


static struct TTraceWin TraceWin;


static struct TETableDesc TableDescription[] = {
  // Colum - Name   | Colum - Template              |  Show, Color ()
  {"Id",             "XXXXXXXX ",                         1, {0, 0, 0, 0}},
  {"DLC",            "XX ",                               1, {0, 0, 0, 0}},
  {"Data (Hex)",     "XX XX XX XX XX XX XX XX ",          1, {0, 0, 0, 0}},
  {"Data (ASCII)",   "XXXXXXXX ",                         1, {0, 0, 0, 0}},
  {NULL, NULL, 0, {0, 0, 0, 0}}};


static gint GetLine(guint index, gpointer user_data, gchar *line, GdkColor *color, guint *flags)
{
struct TTraceWin *trace_win;
guint32 msg_len, i;
unsigned char ch, hex;
struct TCanMsg msg;
(void)color;
(void)flags;

trace_win = (struct TTraceWin *)user_data;
if (CBufGetMsgByIndex(trace_win->Buffer, &msg, index))
  return(-1);
msg_len = msg.MsgLen;
// Id |
if (msg.MsgEFF)
  line += g_snprintf(line, 13, "%.8X\t", msg.Id);
else
  line += g_snprintf(line, 13, "     %.3X\t", msg.Id);
// Dlc |
line += g_snprintf(line, 4, "%u\t", msg_len);
// Bei RTR msg_len auf 0 setztzen, keine Daten
if (msg.MsgRTR)
  msg_len = 0;
// Hex |
for (i = 0; i < msg_len; i++)
  {
  ch = msg.MsgData[i];
  hex = ch >> 4;
  if (hex > 9)
    *line++ = 55 + hex;
  else
    *line++ = '0' + hex;
  hex = ch & 0x0F;
  if (hex > 9)
    *line++ = 55 + hex;
  else
    *line++ = '0' + hex;
  *line++ = ' ';
  }
*line++ = '\t';
// ASCII |
for (i = 0; i < msg_len; i++)
  {
  ch = msg.MsgData[i];
  if ((ch <= 32) || (ch >= 126))
    *line++ = '.';
  else
    *line++ = (char)ch;
  }
*line++ = '\t';
*line = '\0';  // Fehler vermeiden wenn keine Daten
return(0);
}


// ******* Daten Empfangen
static void TraceRxDataHandler(struct TCanDevice *can_device, struct TCanMsg *msg, void *user_data)
{
struct TTraceWin *trace_win;
(void)can_device;

trace_win = (struct TTraceWin *)user_data;
if ((!msg) || (!trace_win->Run))
  return;
CBufAddMsgs(trace_win->Buffer, msg, 1);
}


static gint EventProc(TCanBuffer *cbuf, guint events, gpointer user_data)
{
struct TTraceWin *trace_win;
(void)events;

trace_win = (struct TTraceWin *)user_data;
etable_set_row_size(ETABLE(trace_win->ETable), CBufGetSize(cbuf));
return(TRUE);
}


static void SetStartStop(struct TTraceWin *trace_win)
{
GtkWidget *image;
gint iconsize;

g_signal_handler_block(G_OBJECT(TraceWin.CanStartStopButton), TraceWin.CanStartStopClickId);
// **** Menue Items updaten
iconsize = gtk_toolbar_get_icon_size(GTK_TOOLBAR(TraceWin.Toolbar));
if (trace_win->Run)
  {
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(TraceWin.CanStartStopButton), TRUE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, iconsize);
  gtk_widget_show(image);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Stop");
  }
else
  {
  // Button
  gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(TraceWin.CanStartStopButton), FALSE);
  image = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, iconsize);
  gtk_widget_show(image);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
  gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Start");
  }
g_signal_handler_unblock(G_OBJECT(TraceWin.CanStartStopButton), TraceWin.CanStartStopClickId);
}


static void NeuButtonClickCB(GtkButton *button, gpointer user_data)
{
struct TTraceWin *trace_win;
(void)button;

trace_win = (struct TTraceWin *)user_data;
etable_set_row_size(ETABLE(trace_win->ETable), 0);
CBufDataClear(trace_win->Buffer);
}


static void CanStartStopButtonClickCB(GtkButton *button, gpointer user_data)
{
struct TTraceWin *trace_win;
(void)button;

trace_win = (struct TTraceWin *)user_data;
if (trace_win->Run)
  trace_win->Run = 0;
else
  trace_win->Run = 1;
SetStartStop(trace_win);
}

/* <*>
static void FilterSelectComboBoxCB(GtkWidget *combo, gpointer user_data)
{
struct TTraceWin *trace_win;

trace_win = (struct TTraceWin *)user_data;
trace_win->TraceFilter = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
} */


GtkWidget *CreateTraceWdg(struct TCanDevice *can_device)
{
GtkWidget *widget, *image;
gint icon_size;

if (!can_device)
  return(NULL);
if (!(TraceWin.Buffer = CBufCreate(CAN_BUFFER_SIZE, FALSE, 100, 100, 0, EventProc, (gpointer)&TraceWin)))
  return(NULL);
TraceWin.CanDevice = can_device;
CanDevRxEventConnect(can_device, TraceRxDataHandler, &TraceWin);
// **** VBox
TraceWin.VBox = gtk_vbox_new (FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(TraceWin.VBox), 0);
// **** Toolbar erzeugen
TraceWin.Toolbar = gtk_toolbar_new();
gtk_box_pack_start(GTK_BOX(TraceWin.VBox), TraceWin.Toolbar, FALSE, FALSE, 0);
gtk_toolbar_set_style(GTK_TOOLBAR(TraceWin.Toolbar), GTK_TOOLBAR_BOTH_HORIZ); //GTK_TOOLBAR_BOTH); <*>
icon_size = gtk_toolbar_get_icon_size(GTK_TOOLBAR(TraceWin.Toolbar));
// **** Button "Neu"
image = gtk_image_new_from_stock("gtk-clear", icon_size);
TraceWin.NeuButton = gtk_tool_button_new(image, "New");
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(TraceWin.NeuButton));
(void)g_signal_connect(G_OBJECT(TraceWin.NeuButton), "clicked", G_CALLBACK(NeuButtonClickCB), &TraceWin);
// **** Button "Start/Stop"
TraceWin.CanStartStopButton = gtk_toggle_tool_button_new();
gtk_tool_button_set_label(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), "Start");
image = gtk_image_new_from_stock ("gtk-media-play", icon_size);
gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(TraceWin.CanStartStopButton), image);
gtk_widget_show(GTK_WIDGET(TraceWin.CanStartStopButton));
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(TraceWin.CanStartStopButton));
TraceWin.CanStartStopClickId = g_signal_connect(G_OBJECT(TraceWin.CanStartStopButton), "clicked", G_CALLBACK(CanStartStopButtonClickCB), &TraceWin);

/* <*>
// **** Seperator
widget = (GtkWidget*) gtk_separator_tool_item_new ();
gtk_container_add (GTK_CONTAINER (TraceWin.Toolbar), widget);
// **** Test
widget = gtk_label_new("Anzeigen: ");
item = GTK_WIDGET(gtk_tool_item_new());
gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(widget));
gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(item));
// **** ComboBox
TraceWin.FilterSelectComboBox = gtk_combo_box_new_text();
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Alle");
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Nur bekannte");
gtk_combo_box_append_text(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), "Nur Unbekannte");
gtk_combo_box_set_active(GTK_COMBO_BOX(TraceWin.FilterSelectComboBox), 0);
(void)g_signal_connect(G_OBJECT(TraceWin.FilterSelectComboBox), "changed", G_CALLBACK(FilterSelectComboBoxCB), &TraceWin);
item = GTK_WIDGET(gtk_tool_item_new());
gtk_container_add(GTK_CONTAINER(item), GTK_WIDGET(TraceWin.FilterSelectComboBox));
*/

//gtk_container_add(GTK_CONTAINER(TraceWin.Toolbar), GTK_WIDGET(item)); <*> raus
// **** Tabelle
widget = etable_new(GetLine, &TraceWin, TableDescription);
g_object_set(widget, "auto_scroll", TRUE, NULL);
gtk_box_pack_start(GTK_BOX(TraceWin.VBox), widget, TRUE, TRUE, 0);
TraceWin.ETable = widget;

return(TraceWin.VBox);
}


void DestroyTraceWdg(void)
{
CanDevRxEventDisconnect(TraceWin.CanDevice, TraceRxDataHandler);
CBufDestroy(&TraceWin.Buffer);
}

