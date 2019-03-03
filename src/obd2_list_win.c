/*******************************************************************************
                         obd_list_win.c  -  description
                             -------------------
    begin             : 19.01.2019
    last modify       : 03.02.2019    
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
#include "obd2.h"
#include "obd2_list_win.h"

  enum
  {
  OBD2_LIST_NAME = 0,
  OBD2_LIST_VALUE,
  OBD2_LIST_DB,
  OBD2_LIST_COLUMN_NUMBER
  };


struct TObd2ListWin
  {
  GtkWidget *ListView;
  TObd2 *Obd;
  };


static void Obd2ListWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TObd2ListWin *obd_list_win;
(void)widget;

obd_list_win = (struct TObd2ListWin *)data;
g_free(obd_list_win);
}


static void StrPrintValue(TOBD2Data *current_data, gchar *str)
{
const struct TObdShow *obd_show;
gchar value1_str[20];
gchar value2_str[20];
guint size, n;

obd_show = current_data->Cmd->Show;
str[0] = '\0';
size = 200;
switch (obd_show->Mode1)
  {
  case OBD_SHOW_NONE : return;
  case OBD_SHOW_RAW  : {
                       g_snprintf(value1_str, 20, obd_show->FmtStr1, current_data->ObdU32Value);
                       break;
                       }
  case OBD_SHOW_REAL : {
                       g_snprintf(value1_str, 20, obd_show->FmtStr1, current_data->ObdValue1);
                       break;
                       }
  }
if (obd_show->Units1)
  n = g_snprintf(str, size, "<span foreground=\"red\" font_desc=\"ds-digital 32\">%s %s", value1_str, obd_show->Units1);
else
  n = g_snprintf(str, size, "<span foreground=\"red\" font_desc=\"ds-digital 32\">%s", value1_str);
str += n;
size -= n;
if (obd_show->Mode2 == OBD_SHOW_REAL)
  {
  g_snprintf(value2_str, 20, obd_show->FmtStr2, current_data->ObdValue2);
  if (obd_show->Units2)
    n = g_snprintf(str, size, "\n%s %s", value2_str, obd_show->Units2);
  else
    n = g_snprintf(str, size, "\n%s", value2_str);
  str += n;
  size -= n;
  }
g_snprintf(str, size, "</span>");
}


GtkWidget *Obd2ListWinCreate(TObd2 *obd)
{
struct TObd2ListWin *obd_list_win;
GtkWidget *list_view, *box, *widget;
GtkListStore *store;
GtkCellRenderer *renderer;
GtkTreeSelection *sel;

if (!obd)
  return(NULL);
if (!(obd_list_win = (struct TObd2ListWin *)g_malloc0(sizeof(struct TObd2ListWin))))
  return(NULL);
obd_list_win->Obd = obd;

box = gtk_vbox_new(FALSE, 3);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);
g_object_set_data(G_OBJECT(box), "obd_list_win", obd_list_win);
g_signal_connect(G_OBJECT(box), "destroy", G_CALLBACK(Obd2ListWinDestroyCB), obd_list_win);

widget = gtk_label_new (NULL);
gtk_label_set_markup(GTK_LABEL(widget), "<span size=\"x-large\">OBD Anzeige</span>");
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

store = gtk_list_store_new(OBD2_LIST_COLUMN_NUMBER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
g_object_unref(store);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list_view), FALSE);
// 1. Spalte Icon
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(list_view), -1,
    "Name", renderer, "markup", OBD2_LIST_NAME, NULL);
// 2. Spalte Text
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(list_view), -1,
    "Wert", renderer, "markup", OBD2_LIST_VALUE, NULL);

obd_list_win->ListView = list_view;
// **** Scroll Window
widget = gtk_scrolled_window_new(NULL,NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(widget), GTK_SHADOW_IN);
gtk_container_add(GTK_CONTAINER(widget), list_view);
gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(list_view));
gtk_tree_selection_set_mode(sel, GTK_SELECTION_NONE);
return(box);
}


void Obd2ListWinEvent(GtkWidget *widget, uint32_t event)
{
struct TObd2ListWin *obd_list_win;
GtkWidget *list_view;
GtkTreeIter iter;
GtkListStore *store;
TObd2 *obd;
int idx;
gchar *str;
TOBD2Data *current_data;

if (!(obd_list_win = (struct TObd2ListWin *)g_object_get_data(G_OBJECT(widget), "obd_list_win")))
  return;
list_view = obd_list_win->ListView;
obd = obd_list_win->Obd;
store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list_view)));
gtk_list_store_clear(store);
for (idx = 0; idx < obd->CurrentDataSize; idx++)
  {
  current_data = &obd->CurrentData[idx];
  if ((current_data->Supported) && (current_data->Enabled))
    {
    str = g_strdup_printf("<span foreground=\"blue\"size=\"x-large\"><b>%s</b></span>", current_data->Cmd->Name);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
    OBD2_LIST_NAME, str,
    OBD2_LIST_DB, current_data, -1);
    safe_free(str);
    }
  }
}


void Obd2ListWinPaintValues(GtkWidget *widget)
{
struct TObd2ListWin *obd_list_win;
GtkWidget *list_view;
GtkTreeIter iter;
GtkListStore *store;
gboolean valid;
TOBD2Data *current_data;
gchar val_str[200];

if (!(obd_list_win = (struct TObd2ListWin *)g_object_get_data(G_OBJECT(widget), "obd_list_win")))
  return;
list_view = obd_list_win->ListView;
store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list_view)));

valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
while (valid)
  {
  gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, OBD2_LIST_DB, &current_data, -1);
  StrPrintValue(current_data, val_str);
  gtk_list_store_set(store, &iter, OBD2_LIST_VALUE, val_str, -1);
  valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
  }
}
