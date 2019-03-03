/*******************************************************************************
                         lcd_gauge_win.c  -  description
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
#include "gauge.h"
#include "gtk-lcd.h"
#include "gtk-digital-show.h"
#include "gtk-ex-frame.h"
#include "lcd_gauge_win.h"
#include "main.h"


struct TObd2DataWin
  {
  GtkWidget *SpeedGauge;
  GtkWidget *RpmGauge;
  GtkWidget *EngineLoadGauge;
  GtkWidget *MAP;
  GtkWidget *WaterTemp;
  GtkWidget *FuelLevel;
  GtkWidget *AirTemp;
  GtkWidget *Voltage;

  TObd2 *Obd;
  };

//static struct TObdDataWin ObdDataWin;

//static GdkColor LcdGreenColor = {0, 0x0000, 0xFFFF, 0x0000};

static GdkColor WarnBgColor = { 0, 0xFFFF, 0xFFFF, 0x6600 };  // hell-gelb
static GdkColor WarnFgColor = { 0, 0xFFFF, 0xFFFF, 0      };  // gelb

static GdkColor LimitBgColor = { 0, 0xFFFF, 0x80FF, 0x80FF };  // hell-rot
static GdkColor LimitFgColor = { 0, 0xFFFF, 0,      0x0000 };  // rot


static void Obd2DataWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TObd2DataWin *obd_data_win;
(void)widget;

obd_data_win = (struct TObd2DataWin *)data;
g_free(obd_data_win);
}


GtkWidget *Obd2DataWinCreate(TObd2 *obd)
{
struct TObd2DataWin *obd_data_win;
GtkWidget *frame, *widget, *table, *event_box;
gchar *filename;

if (!obd)
  return(NULL);
if (!(obd_data_win = (struct TObd2DataWin *)g_malloc0(sizeof(struct TObd2DataWin))))
  return(NULL);
obd_data_win->Obd = obd;
event_box = gtk_event_box_new();
g_object_set_data(G_OBJECT(event_box), "obd_data_win", obd_data_win);
g_signal_connect(G_OBJECT(event_box), "destroy", G_CALLBACK(Obd2DataWinDestroyCB), obd_data_win);

filename = g_build_filename(BaseDir, "background.jpg", NULL);
SetBackgroundImage(event_box, filename);
safe_free(filename);
                      // 4 Zeilen, 3 Spalten
table = gtk_table_new(4, 3, FALSE);
gtk_container_add(GTK_CONTAINER(event_box), table);
gtk_container_set_border_width(GTK_CONTAINER(table), 5);
gtk_table_set_col_spacings(GTK_TABLE(table), 5);
gtk_table_set_row_spacings(GTK_TABLE(table), 5);

// Zeile 1 - 3, Spalte 1
obd_data_win->SpeedGauge = mtx_gauge_face_new();
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(obd_data_win->SpeedGauge), "kmh_gauge.xml");
frame = gtk_ex_frame_new();
widget = gtk_alignment_new (0.5, 0.5, 1, 1);
//gtk_widget_show (widget); <*>
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 0, 0);
gtk_container_add(GTK_CONTAINER(widget), obd_data_win->SpeedGauge);
gtk_table_attach(GTK_TABLE(table), frame, 0, 1, 0, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 4, Spalte 1
obd_data_win->FuelLevel = gtk_digital_show_new_with_config("<b> Tank </b>", "<b>%</b>", 0, TRUE, 0.0, 100.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), obd_data_win->FuelLevel, 0, 1, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

// Zeile 1 - 2, Spalte 2
obd_data_win->RpmGauge = mtx_gauge_face_new();
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(obd_data_win->RpmGauge), "rpm_gauge.xml");
frame = gtk_ex_frame_new();
widget = gtk_alignment_new (0.5, 0.5, 1, 1);
//gtk_widget_show (widget); <*>
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 0, 0);
gtk_container_add(GTK_CONTAINER(widget), obd_data_win->RpmGauge);
gtk_table_attach(GTK_TABLE(table), frame, 1, 2, 0, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 3, Spalte 2
obd_data_win->MAP = gtk_digital_show_new_with_config("<b> MAP </b>", "<b>kPa</b>", 0, TRUE, 0.0, 255.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), obd_data_win->MAP, 1, 2, 2, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 4, Spalte 2
obd_data_win->AirTemp = gtk_digital_show_new_with_config("<b> Ladelufttemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 100.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(obd_data_win->AirTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 0.0, NULL);
g_object_set(G_OBJECT(obd_data_win->AirTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 60.0, "max_limit", 80.0, NULL);
gtk_table_attach(GTK_TABLE(table), obd_data_win->AirTemp, 1, 2, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

// Zeile 1 - 2, Spalte 3
obd_data_win->EngineLoadGauge = mtx_gauge_face_new();
mtx_gauge_face_import_xml(MTX_GAUGE_FACE(obd_data_win->EngineLoadGauge), "load_gauge.xml");
frame = gtk_ex_frame_new();
widget = gtk_alignment_new (0.5, 0.5, 1, 1);
//gtk_widget_show (widget); <*>
gtk_container_add (GTK_CONTAINER (frame), widget);
gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 0, 0);
gtk_container_add(GTK_CONTAINER(widget), obd_data_win->EngineLoadGauge);
gtk_table_attach(GTK_TABLE(table), frame, 2, 3, 0, 2, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 3, Spalte 3
obd_data_win->WaterTemp = gtk_digital_show_new_with_config("<b> Wassertemperatur </b>", "<b>°C</b>", 0, TRUE, -40.0, 100.0, 0.0, NULL, NULL);
g_object_set(G_OBJECT(obd_data_win->WaterTemp), "min_warn_fg_color", &WarnFgColor, "min_warn_bg_color", &WarnBgColor,
             "min_limit_fg_color", &LimitFgColor, "min_limit_bg_color", &LimitBgColor, "min_limit", -20.0, "min_warn", 20.0, NULL);
g_object_set(G_OBJECT(obd_data_win->WaterTemp), "max_warn_fg_color", &WarnFgColor, "max_warn_bg_color", &WarnBgColor,
             "max_limit_fg_color", &LimitFgColor, "max_limit_bg_color", &LimitBgColor, "max_warn", 100.0, "max_limit", 110.0, NULL);
gtk_table_attach(GTK_TABLE(table), obd_data_win->WaterTemp, 2, 3, 2, 3, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);
// Zeile 4, Spalte 3
obd_data_win->Voltage = gtk_digital_show_new_with_config("<b> Spannung </b>", "<b>V</b>", 1, TRUE, 0.0, 65.0, 0.0, NULL, NULL);
gtk_table_attach(GTK_TABLE(table), obd_data_win->Voltage, 2, 3, 3, 4, GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0, 0);

return(event_box);
}


void Obd2DataWinEvent(GtkWidget *widget, uint32_t event)
{
/*struct TObd2DataWin *obd_data_win;

if (!(obd_data_win = (struct TObd2DataWin *)g_object_get_data(G_OBJECT(widget), "obd_data_win")))
  return;*/
}


void Obd2DataWinPaintValues(GtkWidget *widget)
{
double value;
struct TObd2DataWin *obd_data_win;
TObd2 *obd;

if (!(obd_data_win = (struct TObd2DataWin *)g_object_get_data(G_OBJECT(widget), "obd_data_win")))
  return;
obd = obd_data_win->Obd;

value = Obd2ValueGetAsReal(obd, 1, PID_SPEED, NULL);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(obd_data_win->SpeedGauge), value);
value = Obd2ValueGetAsReal(obd, 1, PID_RPM, NULL);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(obd_data_win->RpmGauge), value);
value = Obd2ValueGetAsReal(obd, 1, PID_ENGINE_LOAD, NULL);
mtx_gauge_face_set_value(MTX_GAUGE_FACE(obd_data_win->EngineLoadGauge), value);

value = Obd2ValueGetAsReal(obd, 1, PID_INTAKE_MAP, NULL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(obd_data_win->MAP), value);
value = Obd2ValueGetAsReal(obd, 1, PID_COOLANT_TEMP, NULL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(obd_data_win->WaterTemp), value);
value = Obd2ValueGetAsReal(obd, 1, PID_FUEL_LEVEL, NULL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(obd_data_win->FuelLevel), value);
value = Obd2ValueGetAsReal(obd, 1, PID_INTAKE_TEMP, NULL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(obd_data_win->AirTemp), value);
value = Obd2ValueGetAsReal(obd, 1, PID_CONTROL_MODULE_VOLTAGE, NULL);
gtk_digital_show_set_value(GTK_DIGITAL_SHOW(obd_data_win->Voltage), value);
}
