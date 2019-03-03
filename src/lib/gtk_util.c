/*******************************************************************************
                          gtk_util.c  -  description
                             -------------------
    begin             : 28.02.2017
    last modify       : 08.02.2019
    copyright         : (C) 2017 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <string.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include "util.h"
#include "gtk_util.h"

struct TInfoTableWdg
  {
  TInfoTableGetValueFunc GetValueFunc;
  gpointer GetValueFuncData;
  const gchar **DescriptionList;
  GtkWidget **ValueLabels;
  };


static void InfoTableDestroyCB(GtkWidget *widget, gpointer data)
{
struct TInfoTableWdg *info_table;
(void)widget;

info_table = (struct TInfoTableWdg *)data;
g_free(info_table);
}


GtkWidget *CreateInfoTable(const gchar **description_list, TInfoTableGetValueFunc get_value_func, gpointer get_value_func_data)
{
struct TInfoTableWdg *info_table;
GtkWidget *table, *widget;
gchar *desc, *str, *label_str;
guint i, size;

if (!description_list)
  return(NULL);
if (!(info_table = (struct TInfoTableWdg *)g_malloc0(sizeof(struct TInfoTableWdg))))
  return(NULL);
for (size = 0; (gchar *)description_list[size]; size++);
info_table->ValueLabels = (GtkWidget **)g_malloc0(size * sizeof(GtkWidget *));
info_table->GetValueFunc = get_value_func;
info_table->GetValueFuncData = get_value_func_data;
info_table->DescriptionList = description_list;
// **** Tabelle erzeugen
table = gtk_table_new(4, 2, FALSE);
g_object_set_data(G_OBJECT(table), "info_table", info_table);
g_signal_connect(G_OBJECT(table), "destroy", G_CALLBACK(InfoTableDestroyCB), info_table);
gtk_table_set_row_spacings(GTK_TABLE(table), 4);
gtk_table_set_col_spacings(GTK_TABLE(table), 3);

for (i = 0; (desc = (gchar *)description_list[i]); i++)
  {
  // **** Bezeichner Label erzeugen
  label_str = g_markup_printf_escaped("<span foreground=\"blue\"><big>%s</big></span>:", desc);
  widget = gtk_label_new(label_str);
  safe_free(label_str);
  gtk_table_attach(GTK_TABLE(table), widget, 0, 1, i, i+1,
                  (GtkAttachOptions)(GTK_FILL),
                  (GtkAttachOptions)(0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 1, 0);

  // **** Value Label erzeugen
  str = NULL;
  label_str = NULL;
  if (get_value_func)
    {
    str =(get_value_func)(i, get_value_func_data);
    label_str = g_markup_printf_escaped("<span foreground=\"red\"><big>%s</big></span>", str);
    }
  widget = gtk_label_new(label_str);
  info_table->ValueLabels[i] = widget;
  gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
  gtk_widget_set_size_request(widget, 500, -1);  // <*> ?
  safe_free(str);
  safe_free(label_str);
  gtk_table_attach(GTK_TABLE(table), widget, 1, 2, i, i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(0), 0, 0);
  gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  }
return(table);
}


void InfoTableUpdate(GtkWidget *widget)
{
struct TInfoTableWdg *info_table;
TInfoTableGetValueFunc get_value_func;
gpointer get_value_func_data;
GtkWidget *label_wdg;
gchar *str, *label_str;
guint i;

if (!(info_table = (struct TInfoTableWdg *)g_object_get_data(G_OBJECT(widget), "info_table")))
  return;
get_value_func = info_table->GetValueFunc;
get_value_func_data = info_table->GetValueFuncData;
for (i = 0; (gchar *)info_table->DescriptionList[i]; i++)
  {
  // **** Value Label erzeugen
  if (get_value_func)
    {
    str =(get_value_func)(i, get_value_func_data);
    label_str = g_markup_printf_escaped("<span foreground=\"red\"><big>%s</big></span>", str);
    label_wdg = info_table->ValueLabels[i];
    gtk_label_set_markup(GTK_LABEL(label_wdg), label_str);
    safe_free(str);
    safe_free(label_str);
    }
  }
}




void UpdateGtk(void)
{
while (gtk_events_pending())
  gtk_main_iteration();
}


GtkWidget *create_menue_button(const gchar *stock, const gchar *text, const gchar *secondary)
{
GtkWidget *button, *align, *image, *hbox, *label, *vbox;

button = gtk_button_new ();
label = gtk_label_new (NULL);
gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), text);
gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);

vbox = gtk_vbox_new (FALSE, 2);
if (stock)
  {
  image = gtk_image_new_from_stock (stock, GTK_ICON_SIZE_DIALOG);
  hbox = gtk_hbox_new (FALSE, 20);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  }

align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

label = gtk_label_new (NULL);
gtk_label_set_text (GTK_LABEL (label), secondary);
gtk_box_pack_end (GTK_BOX (vbox), label, FALSE, FALSE, 0);

gtk_container_add (GTK_CONTAINER (button), align);
if (stock)
  gtk_container_add (GTK_CONTAINER (align), hbox);
else
  gtk_container_add (GTK_CONTAINER (align), vbox);
gtk_widget_show_all (align);

return(button);
}

