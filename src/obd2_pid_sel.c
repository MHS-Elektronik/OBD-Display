/*******************************************************************************
                        obd2_pid_sel.c  -  description
                             -------------------
    begin             : 28.01.2019
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
#include "main.h"
#include "obd2.h"
#include "obd2_pid_sel.h"


enum
  {
  PID_SEL_COL_ENABLE = 0,
  PID_SEL_COL_SENSITIVE,
  PID_SEL_COL_TEXT,
  PID_SEL_COL_DB,
  PID_SEL_COLUMN_NUMBER
  };


struct TPidSelWin
  {
  GtkWidget *PidListView;
  GtkWidget *PidEnableToggleWdg;
  gulong PidEnableToggleCbId;
  TObd2 *Obd;
  };


static void PidEnabledCB(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
TOBD2Data *current_data;
GtkTreeModel *model;
GtkTreeIter  iter;
GtkTreePath *path;
gboolean aktiv;

model = (GtkTreeModel *)data;
path = gtk_tree_path_new_from_string(path_str);
gtk_tree_model_get_iter(model, &iter, path);
gtk_tree_model_get(model, &iter, PID_SEL_COL_DB, &current_data, -1);
g_assert(current_data != NULL);
aktiv = !gtk_cell_renderer_toggle_get_active(cell);
current_data->Enabled = aktiv;
gtk_list_store_set(GTK_LIST_STORE(model), &iter, PID_SEL_COL_ENABLE, aktiv, -1);
gtk_tree_path_free(path);
}


gboolean view_selection_func (GtkTreeSelection *selection, GtkTreeModel *model,
           GtkTreePath *path, gboolean path_currently_selected, gpointer userdata)
{
GtkTreeIter iter;
gboolean sensitive;

sensitive = TRUE;
if (gtk_tree_model_get_iter(model, &iter, path))
  gtk_tree_model_get(model, &iter, PID_SEL_COL_SENSITIVE, &sensitive, -1);
return(sensitive);
}


static void PidSelWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TPidSelWin *pid_sel_win;
(void)widget;

pid_sel_win = (struct TPidSelWin *)data;
g_free(pid_sel_win);
}


GtkWidget *PidSelWinCreate(TObd2 *obd)
{
struct TPidSelWin *pid_sel_win;
GtkWidget *pid_list, *widget, *box;
GtkListStore *store;
GtkCellRenderer *cell;
GtkTreeSelection *sel;
GtkTreeViewColumn *col;

if (!obd)
  return(NULL);
if (!(pid_sel_win = (struct TPidSelWin *)g_malloc0(sizeof(struct TPidSelWin))))
  return(NULL);
pid_sel_win->Obd = obd;

box = gtk_vbox_new(FALSE, 3);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);
g_object_set_data(G_OBJECT(box), "pid_sel_win", pid_sel_win);
g_signal_connect(G_OBJECT(box), "destroy", G_CALLBACK(PidSelWinDestroyCB), pid_sel_win);

widget = gtk_label_new (NULL);
gtk_label_set_markup(GTK_LABEL(widget), "<span size=\"x-large\">PIDs auswählen</span>");
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

store = gtk_list_store_new(PID_SEL_COLUMN_NUMBER, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
pid_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
g_object_unref(store);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pid_list), FALSE);
//gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(pid_list), TRUE);

col = gtk_tree_view_column_new();
// 1. Spalte -> Enable
cell = gtk_cell_renderer_toggle_new ();
gtk_tree_view_column_pack_start(col, cell, FALSE);
pid_sel_win->PidEnableToggleWdg = cell;
pid_sel_win->PidEnableToggleCbId = g_signal_connect(cell, "toggled", G_CALLBACK(PidEnabledCB), GTK_TREE_MODEL(store));
gtk_tree_view_column_set_attributes(col, cell,
							    "active", PID_SEL_COL_ENABLE,
                   "activatable", PID_SEL_COL_SENSITIVE, NULL);
// 2. Spalte -> Text
cell = gtk_cell_renderer_text_new();
gtk_tree_view_column_pack_start(col, cell, TRUE);
gtk_tree_view_column_set_attributes(col, cell, "markup", PID_SEL_COL_TEXT,
                         "sensitive", PID_SEL_COL_SENSITIVE, NULL);

gtk_tree_view_append_column(GTK_TREE_VIEW(pid_list), col);

sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(pid_list));
gtk_tree_selection_set_select_function(sel, view_selection_func, GTK_TREE_MODEL(store), NULL);
gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
pid_sel_win->PidListView = pid_list;

// **** Scroll Window
widget = gtk_scrolled_window_new(NULL,NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(widget), GTK_SHADOW_IN);
gtk_container_add(GTK_CONTAINER(widget), pid_list);

gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

gtk_widget_show_all(box);
return(box);
}


void PidSelWinEvent(GtkWidget *widget, uint32_t event)
{
GtkWidget *pid_list;
struct TPidSelWin *pid_sel_win;
GtkTreeIter iter;
GtkListStore *store;
gboolean aktiv, sensitive;
int idx;
TObd2 *obd;
TOBD2Data *current_data;

if (event != APP_EVENT_OBD2_CONNECT)
  return;
if (!(pid_sel_win = (struct TPidSelWin *)g_object_get_data(G_OBJECT(widget), "pid_sel_win")))
  return;
pid_list = pid_sel_win->PidListView;
obd = pid_sel_win->Obd;
store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pid_list)));
g_signal_handler_block(pid_sel_win->PidEnableToggleWdg, pid_sel_win->PidEnableToggleCbId);
gtk_list_store_clear(store);
for (idx = 0; idx < obd->CurrentDataSize; idx++)
  {
  current_data = &obd->CurrentData[idx];
  if (current_data->Supported)
    {
    if (current_data->EnableLock)
      sensitive = FALSE;
    else
      sensitive = TRUE;
    if (current_data->Enabled)
      aktiv = TRUE;
    else
      aktiv = FALSE;
    }
  else
    {
    sensitive = FALSE;
    if (current_data->Enabled) //<*>
      aktiv = TRUE;
    else
      aktiv = FALSE;
    //aktiv = FALSE;
    }
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
  PID_SEL_COL_ENABLE, aktiv,
  PID_SEL_COL_SENSITIVE, sensitive,
  PID_SEL_COL_TEXT, current_data->Cmd->Name,
  PID_SEL_COL_DB, current_data, -1);
  }
g_signal_handler_unblock(pid_sel_win->PidEnableToggleWdg, pid_sel_win->PidEnableToggleCbId);
}
