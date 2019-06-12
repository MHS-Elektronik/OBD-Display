/*******************************************************************************
                        start_win.c  -  description
                             -------------------
    begin             : 27.01.2019
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
#include <gdk/gdk.h>
#include "util.h"
#include "gtk_util.h"
#include "main.h"
#include "can_dev_pnp.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "can_autobaud.h"
#include "msg_widget.h"
#include "obd2.h"
#include "vin_wdg.h"
#include "start_win.h"


struct TStartWin
  {
  struct TCanAutobaud *CanAutobaud;
  struct TCanDevPnP *CanDevPnP;
  struct TMainWin *MainWin;
  GtkWidget *BaseWdg;
  GtkWidget *DriverWdg;
  GtkWidget *HardwareWdg;
  GtkWidget *AutoBaudWdg;
  GtkWidget *ObdWdg;
  };

struct TCanDriverInfo
  {
  gchar *FileName;
  gchar *Description;
  gchar *Version;
  };


TMhsGScheduler *MainScheduler = NULL;


static const gchar *DriverInfoTableStr[] = {
  "Dateiname",
  "Beschreibung",
  "Treiber Version",
  NULL};

static const gchar *DeviceInfoTableStr[] = {
  "Hardware",
  "Seriennummer",
  "Firmware Version",
  NULL};

const GdkColor BgColor = { 0, 0, 0, 0.5 };

static const struct TCanMsg AutobaudTxMsg = {
  .Id = OBD2_TX_ID,
  .MsgFlags = 8,
  .MsgData = {0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  .Time = {0, 0}};


static void CanDriverInfoFree(struct TCanDriverInfo *driver_info)
{
safe_free(driver_info->FileName);
safe_free(driver_info->Description);
safe_free(driver_info->Version);
}


static void ExtractDriverInfo(const char *str, struct TCanDriverInfo *driver_info)
{
int match;
char *tmpstr, *s, *key, *val;

tmpstr = g_strdup(str);
s = tmpstr;
do
  {
  // Bezeichner auslesen
  key = get_item_as_string(&s, ":=", &match);
  if (match <= 0)
    break;
  // Value auslesen
  val = get_item_as_string(&s, ";", &match);
  if (match < 0)
    break;
  if (!g_ascii_strcasecmp(key, "Description"))
    {
    driver_info->Description = g_strdup(val);
    continue;
    }
  if (!g_ascii_strcasecmp(key, "Version"))
    {
    driver_info->Version = g_strdup(val);
    continue;
    }
  }
while(1);
safe_free(tmpstr);
}


static gchar *GetDriverInfoValue(guint idx, gpointer *data)
{
struct TCanDriverInfo *driver_info;

driver_info = (struct TCanDriverInfo *)data;
switch (idx)
  {
  case 0 : { return(g_strdup(driver_info->FileName)); }
  case 1 : { return(g_strdup(driver_info->Description)); }
  case 2 : { return(g_strdup(driver_info->Version)); }
  default : return(NULL);
  }
}


static GtkWidget *CreateDriverInfoTable(struct TCanDevice *can_device)
{
struct TCanDriverInfo driver_info;
struct TCanCore *can_core;
GtkWidget *table;

if (!can_device)
  return(NULL);
can_core = can_device->CanCore;
driver_info.FileName = NULL;
driver_info.Description = NULL;
driver_info.Version = NULL;
ExtractDriverInfo(can_core->DriverInfoStr, &driver_info);
driver_info.FileName = g_strdup(can_core->DriverFileName);
table = CreateInfoTable(DriverInfoTableStr, GetDriverInfoValue, (gpointer)&driver_info);
CanDriverInfoFree(&driver_info);
return(table);
}


static gchar *GetDeviceInfoValue(guint idx, gpointer *data)
{
struct TCanDeviceInfo *info;
guint ver, ver2;

info = (struct TCanDeviceInfo *)data;
switch (idx)
  {
  case 0 : { return(g_strdup(info->Description)); }
  case 1 : { return(g_strdup(info->SerialNumber)); }
  case 2 : {
           ver = info->FirmwareVersion / 1000;
           ver2 = info->FirmwareVersion % 1000;
           return(g_strdup_printf("%u.%02u", ver, ver2));
           }
  default : return(NULL);
  }
}


static GtkWidget *CreateDeviceInfoTable(struct TCanDevice *can_device)
{
GtkWidget *table;

if (!can_device)
  return(NULL);
if (can_device->Status < DEV_STATUS_READY)
  return(NULL);
table = CreateInfoTable(DeviceInfoTableStr, GetDeviceInfoValue, (gpointer)&can_device->DeviceInfo);
return(table);
}


static void SchedulerMsgsCB(TMhsGMessage *msg, gpointer user_data)
{
GtkWidget *widget;
int32_t num_devs;
gchar *str, *msg_str;
guint status;
gint err;
TObd2 *obd2;
struct TStartWin *start_win;
guint sub_msg;

start_win = (struct TStartWin *)user_data;
if (!start_win->MainWin)
  return;
err = 0;
// ******* Tiny-CAN Status ********
if (msg->MessageType == MHS_MSG_PNP_EVENT)
  {
  num_devs = *((int32_t *)msg->Data);
  MsgWidgetDestroyCustomWidget(start_win->HardwareWdg);
  MsgWidgetDestroyCustomWidget(start_win->ObdWdg);
  if (num_devs <= 0)
    {
    err = 1;
    UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_WAIT, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                                 "<b>Tiny-CAN nicht verbunden</b>");
    }
  else
    {
    UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                               "<b>Initialisierung, Status unbekannt</b>");
    UpdateGtk();
    if (CanDevOpen(start_win->CanAutobaud->CanDevice, CAN_DEV_CAN_NOT_START, NULL, 0) < 0)  // <*> start_win->CanAutobaud->CanSpeed
      {
      err = 1;
      str = g_strdup_printf("<span size=\"x-large\">Tiny-CAN Status</span>\n"
                            "<b>Fehler beim Öffnen des CAN Devices</b>\n"
                            "%s", start_win->CanAutobaud->CanDevice->LastErrorString);
      UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_ERROR, str);
      safe_free(str);
      }
    else
      {
      UpdateMsgWidget(start_win->HardwareWdg, MSG_WDG_STATUS_OK, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                               "<b>Verbindung zum Tiny-CAN hergestellt</b>");
      if ((widget = CreateDeviceInfoTable(start_win->CanAutobaud->CanDevice)))
        MsgWidgetAddCustomWidget(start_win->HardwareWdg, widget);
      }
    }

  if (err)
    {
    // ******* Automatische Baudratenerkennung *******
    UpdateMsgWidget(start_win->AutoBaudWdg, MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                             "<b>CAN Schnittstelle nicht geöffnet</b>");
    // ******* OBD *******
    UpdateMsgWidget(start_win->ObdWdg, MSG_WDG_STATUS_STOP, "<span size=\"x-large\">OBD</span>\n"
                                             "<b>CAN Schnittstelle nicht geöffnet</b>");
    }
  else
    {
    // ******* Automatische Baudratenerkennung *******
    UpdateMsgWidget(start_win->AutoBaudWdg, MSG_WDG_STATUS_WAIT, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                             "<b>Baudratenerkennung wird gestartet ...</b>");
    // ******* OBD *******
    UpdateMsgWidget(start_win->ObdWdg, MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">OBD</span>\n"
                                             "<b>Warte auf Baudraten erkennung</b>");
    }

  //UpdateGtk(); <*> raus
  if (err)
    {
    Obd2Stop(start_win->MainWin->Obd2);
    CanDevClose(start_win->CanAutobaud->CanDevice);
    StopCanAutobaud(start_win->CanAutobaud);
    }
  else
    StartCanAutobaud(start_win->CanAutobaud, start_win->CanAutobaud->CanSpeed);
  }
else if (msg->MessageType == MHS_MSG_OBD2_EVENT)
  {
  sub_msg = *((guint *)msg->Data);
  if (sub_msg == OBD2_EVENT_DTC_READ_FINISH)
    SendEventToWindows(APP_EVENT_OBD2_DTC_READ_FINISH);
  else if (sub_msg == OBD2_EVENT_DTC_READ_ERROR)
    SendEventToWindows(APP_EVENT_OBD2_DTC_READ_ERROR);
  else if (sub_msg == OBD2_EVENT_CONNECT)
    {
    UpdateMsgWidget(start_win->ObdWdg, MSG_WDG_STATUS_OK, "<span size=\"x-large\">OBD</span>\n"
                                              "<b>Verbindung hergestellt</b>");
    if ((obd2 = start_win->MainWin->Obd2))
      {
      widget = VinWdgCreate(obd2);
      MsgWidgetAddCustomWidget(start_win->ObdWdg, widget);
      }
    SetAppStatus(APP_START_FINISH);
    }
  }
else
  {
  msg_str = (gchar *)msg->Data;
  MsgWidgetDestroyCustomWidget(start_win->ObdWdg);  // <*>
  str = g_strdup_printf("<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                  "<b>%s</b>", msg_str);
  switch (msg->MessageType)
    {
    case MHS_MSG_AUTOBAUD_START  : {
                                   status = MSG_WDG_STATUS_BUSY;
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_RUN    : {
                                   status = MSG_WDG_STATUS_BUSY;
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_OK     : {
                                   status = MSG_WDG_STATUS_OK;
                                   Obd2Start(start_win->MainWin->Obd2);
                                   //SetAppStatus(APP_START_FINISH); <*>
                                   break;
                                   }
    case MHS_MSG_AUTOBAUD_ERROR  : {
                                   status = MSG_WDG_STATUS_STOP;
                                   break;
                                   }
    default                      : status = MSG_WDG_STATUS_STOP;
    }
  UpdateMsgWidget(start_win->AutoBaudWdg, status, str);
  safe_free(str);
  }

}


static void StartWinDestroyCB(GtkWidget *widget, gpointer data)
{
struct TStartWin *start_win;
(void)widget;

start_win = (struct TStartWin *)data;
CanDevPnPDestroy(&start_win->CanDevPnP);
DestroyCanAutobaud(&start_win->CanAutobaud);
g_free(start_win);
}


GtkWidget *CreateStartWin(struct TMainWin *main_win)
{
gchar *str;
GtkWidget *box, *widget, *scrolledwindow;
struct TStartWin *start_win;
gint err;
struct TCanCore *can_core;

if (!main_win)
  return(NULL);
can_core = main_win->CanDevice->CanCore;
if (!(start_win = (struct TStartWin *)g_malloc0(sizeof(struct TStartWin))))
  return(NULL);
start_win->MainWin = main_win;
box = gtk_vbox_new(FALSE, 3);
gtk_container_set_border_width(GTK_CONTAINER(box), 2);

scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_SHADOW_NONE);
gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow), box);//event_box);
//gtk_widget_modify_bg(gtk_bin_get_child(GTK_BIN(scrolledwindow)), GTK_STATE_NORMAL, &BgColor);
//gtk_widget_set_opacity(gtk_bin_get_child(GTK_BIN(scrolledwindow)), 0.5);


g_object_set_data(G_OBJECT(scrolledwindow), "start_win", start_win);
g_signal_connect(G_OBJECT(scrolledwindow), "destroy", G_CALLBACK(StartWinDestroyCB), start_win);

err = 0;
// ******* System *******
if (can_core->Status == CAN_CORE_DRIVER_ERROR)
  {
  err = 1;
  str = g_strdup_printf("<span size=\"x-large\">System</span>\n"
                        "<b>Laden und Initialisieren des Tiny-CAN API Treibers fehlgeschlagen</b>\n"
                        "%s", can_core->LastErrorString);
  start_win->DriverWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, str);
  safe_free(str);
  }
else
  {
  start_win->DriverWdg = CreateMsgWidget(MSG_WDG_STATUS_OK, "<span size=\"x-large\">System</span>\n"
                                         "<b>Tiny-CAN API Treiber geladen und Initialisiert</b>");
  if (can_core->DriverInfoStr)
    {
    widget = CreateDriverInfoTable(main_win->CanDevice);
    MsgWidgetAddCustomWidget(start_win->DriverWdg, widget);
    }
  }
gtk_box_pack_start(GTK_BOX(box), start_win->DriverWdg, FALSE, FALSE, 0);
// ******* Tiny-CAN Status ********
if (err)
  {
  start_win->HardwareWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                           "<b>Fataler Fehler: Treiber API nicht geladen</b>");
  }
else
  {
  start_win->HardwareWdg = CreateMsgWidget(MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Tiny-CAN Status</span>\n"
                                           "<b>Initialisierung, Status unbekannt</b>");
  }
gtk_box_pack_start(GTK_BOX(box), start_win->HardwareWdg, FALSE, FALSE, 0);
// ******* Automatische Baudratenerkennung *******
if (err)
  {
  // ******* Automatische Baudratenerkennung *******
  start_win->AutoBaudWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                          "<b>Fataler Fehler: Treiber API nicht geladen</b>");
  // ******* Obd *******
  start_win->ObdWdg = CreateMsgWidget(MSG_WDG_STATUS_STOP, "<span size=\"x-large\">OBD</span>\n"
                                           "<b>Fataler Fehler: Treiber API nicht geladen</b>");
  }
else
  {
  // ******* Automatische Baudratenerkennung *******
  start_win->AutoBaudWdg = CreateMsgWidget(MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">Automatische Baudratenerkennung</span>\n"
                                           "<b>Tiny-CAN Hardware noch nicht geöffnet</b>");
  // ******* Obd *******
  start_win->ObdWdg = CreateMsgWidget(MSG_WDG_STATUS_BUSY, "<span size=\"x-large\">OBD</span>\n"
                                           "<b>Tiny-CAN Hardware noch nicht geöffnet</b>");
  }
gtk_box_pack_start(GTK_BOX(box), start_win->AutoBaudWdg, FALSE, FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), start_win->ObdWdg, FALSE, FALSE, 0);

start_win->BaseWdg = box;
if (!err)
  {
  MainScheduler = mhs_g_message_scheduler_create(SchedulerMsgsCB, (gpointer)start_win, TRUE);
  start_win->CanAutobaud = CreateCanAutobaud(MainScheduler, main_win->CanDevice, 0);
  SetupAutobaud(start_win->CanAutobaud, AUTOBAUD_250K_EN | AUTOBAUD_500K_EN, &AutobaudTxMsg, AUTOBAUD_NO_SILENT_MODE | AUTOBAUD_AKTIV_SCAN);
  }
gtk_widget_show_all(scrolledwindow);
return(scrolledwindow);
}


void StartWinRun(GtkWidget *widget)
{
struct TStartWin *start_win;

if (!(start_win = (struct TStartWin *)g_object_get_data(G_OBJECT(widget), "start_win")))
  return;
if (MainScheduler)
  {
  start_win->CanDevPnP = CanDevPnPCreate(MainScheduler);
  CanDevPnPScan(start_win->CanDevPnP);
  }
}


void StartWinStop(GtkWidget *widget)
{
struct TStartWin *start_win;

if (!(start_win = (struct TStartWin *)g_object_get_data(G_OBJECT(widget), "start_win")))
  return;
StopCanAutobaud(start_win->CanAutobaud);
}


