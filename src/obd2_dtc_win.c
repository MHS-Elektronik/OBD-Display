/*******************************************************************************
                        obd2_dtc_win.c  -  description
                             -------------------
    begin             : 06.02.2019
    last modify       : 06.02.2019
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
#include "dtc_db.h"
#include "obd2.h"
#include "obd2_dtc_win.h"

  enum
  {
  OBD2_DTC_CODE = 0,
  OBD2_DTC_DESCRIPTION,
  OBD2_DTC_DB_ITEM,
  OBD2_DTC_COLUMN_NUMBER
  };


struct TObd2DtcWin
  {
  GtkWidget *ListView;
  TObd2 *Obd;
  TDtcListItem *DtcDb;
  };


const char NoErrorDescription[] = {"Keine Beschreibung in der Datenbank"};

static char *DtcToString(uint16_t dtc)
{
char c;

switch (dtc & 0xC000)
  {
  case 0x0000 : {
                c = 'P'; // Powertrain
                break;
                }
  case 0x4000 : {
                c = 'C'; // Chassis
                break;
                }
  case 0x8000 : {
                c = 'B'; // Body
                break;
                }
  case 0xC000 : {
                c = 'U'; // Network
                break;
                }
  }
return(g_strdup_printf("%c%04X", c, (dtc & 0x3FFF)));
}


static void Obd2DtcWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TObd2DtcWin *obd_dtc_win;
(void)widget;

obd_dtc_win = (struct TObd2DtcWin *)data;
g_free(obd_dtc_win);
}


GtkWidget *Obd2DtcWinCreate(TObd2 *obd, TDtcListItem *dtc_db)
{
struct TObd2DtcWin *obd_dtc_win;
GtkWidget *list_view, *box, *widget;
GtkListStore *store;
GtkCellRenderer *renderer;
GtkTreeSelection *sel;

if (!obd)
  return(NULL);
if (!(obd_dtc_win = (struct TObd2DtcWin *)g_malloc0(sizeof(struct TObd2DtcWin))))
  return(NULL);
obd_dtc_win->Obd = obd;
obd_dtc_win->DtcDb = dtc_db;

box = gtk_vbox_new(FALSE, 3);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);
g_object_set_data(G_OBJECT(box), "obd_dtc_win", obd_dtc_win);
g_signal_connect(G_OBJECT(box), "destroy", G_CALLBACK(Obd2DtcWinDestroyCB), obd_dtc_win);

widget = gtk_label_new (NULL);
gtk_label_set_markup(GTK_LABEL(widget), "<span size=\"x-large\">Fehlerspeicher</span>");
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

store = gtk_list_store_new(OBD2_DTC_COLUMN_NUMBER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
g_object_unref(store);
gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list_view), TRUE);
// 1. Spalte: DTC Code
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(list_view), -1,
    "Code", renderer, "markup", OBD2_DTC_CODE, NULL);
// 2. Spalte: Beschreibung
renderer = gtk_cell_renderer_text_new();
gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(list_view), -1,
    "Beschreibung", renderer, "markup", OBD2_DTC_DESCRIPTION, NULL);

obd_dtc_win->ListView = list_view;
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


void Obd2DtcWinEvent(GtkWidget *widget, uint32_t event)
{
struct TObd2DtcWin *obd_dtc_win;
TDtcListItem *db;
GtkWidget *list_view;
GtkTreeIter iter;
GtkListStore *store;
TObd2 *obd;
char *dtc_str, *description;
uint16_t *dtcs, *dtc;
uint32_t dtc_size;

if (!(obd_dtc_win = (struct TObd2DtcWin *)g_object_get_data(G_OBJECT(widget), "obd_dtc_win")))
   return;
list_view = obd_dtc_win->ListView;
obd = obd_dtc_win->Obd;
if (event == APP_EVENT_SHOW)
  Obd2ReadDtc(obd);
else if ((event == APP_EVENT_OBD2_DTC_READ_FINISH) ||
         (event == APP_EVENT_OBD2_DTC_READ_ERROR))
  {
  store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list_view)));
  gtk_list_store_clear(store);

  dtcs = Obd2ValueGetDTCs(obd, NULL, &dtc_size);
  for (dtc = dtcs; dtc_size; dtc_size--)
    {
    dtc_str = DtcToString(*dtc++);
    if ((db = DtcDbGetItem(obd_dtc_win->DtcDb, dtc_str)))
      description = db->Description;
    else
      description = (char *)NoErrorDescription;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
        OBD2_DTC_CODE, dtc_str,
        OBD2_DTC_DESCRIPTION, description,
        OBD2_DTC_DB_ITEM, db, -1);
    safe_free(dtc_str);
    }
  safe_free(dtcs);
  }
}

/*for (db = obd_dtc_win->DtcDb; db; db = db->Next)
  {
  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
  OBD2_DTC_CODE, db->DtcNo,
  OBD2_DTC_DESCRIPTION, db->Description,
  OBD2_DTC_DB_ITEM, NULL, -1);
  }*/

void Obd2DtcWinPaintValues(GtkWidget *widget)
{
}
