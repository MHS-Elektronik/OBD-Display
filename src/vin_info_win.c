/*******************************************************************************
                        vin_info_win.c  -  description
                             -------------------
    begin             : 10.02.2019
    last modify       : 10.02.2019
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
#include "gtk_util.h"
#include "main.h"
#include "obd2.h"
#include "vin_wdg.h"
#include "vin_info_win.h"


struct TVinInfoWin
  {
  GtkWidget *VinWdg;
  };


static void VinInfoWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TVinInfoWin *vin_info_win;
(void)widget;

vin_info_win = (struct TVinInfoWin *)data;
g_free(vin_info_win);
}


GtkWidget *VinInfoWinCreate(TObd2 *obd)
{
struct TVinInfoWin *vin_info_win;
GtkWidget *box, *widget;

if (!obd)
  return(NULL);
if (!(vin_info_win = (struct TVinInfoWin *)g_malloc0(sizeof(struct TVinInfoWin))))
  return(NULL);
box = gtk_vbox_new(FALSE, 3);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);
g_object_set_data(G_OBJECT(box), "vin_info_win", vin_info_win);
g_signal_connect(G_OBJECT(box), "destroy", G_CALLBACK(VinInfoWinDestroyCB), vin_info_win);

widget = gtk_label_new (NULL);
gtk_label_set_markup(GTK_LABEL(widget), "<span size=\"x-large\">Fharzeug Informationen:</span>");
gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

widget = VinWdgCreate(obd);
vin_info_win->VinWdg = widget;
gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);

return(box);
}


void VinInfoWinEvent(GtkWidget *widget, uint32_t event)
{
struct TVinInfoWin *vin_info_win;

if (event != APP_EVENT_SHOW)
  return;
if (!(vin_info_win = (struct TVinInfoWin *)g_object_get_data(G_OBJECT(widget), "vin_info_win")))
  return;
VinWdgUpdate(vin_info_win->VinWdg);
}



void VinInfoWinPaintValues(GtkWidget *widget)
{
}
