/*******************************************************************************
                            main.c  -  description
                             -------------------
    begin             : 28.01.2019
    last modify       : 08.02.2019
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
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "obd_disp_icons.h"
#include "can_device.h"
#include "isotp.h"
#include "obd2.h"
#include "util.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "gtk_util.h"
#include "iconcache.h"
#include "setup.h"
#include "lcd_gauge_win.h"
#include "trace.h"
#include "start_win.h"
#include "obd2_pid_sel.h"
#include "obd2_list_win.h"
#include "dtc_db.h"
#include "obd2_dtc_win.h"
#include "vin_info_win.h"
#include "vin_db.h"
#include "main.h"

#define OBD2_TX_ID 0x7DF
#define OBD2_RX_ID 0x7E8

#define START_PAGE_INDEX           0
#define DEFAULT_VALUES_PAGE_INDEX  1
#define OBD_LIST_PAGE_INDEX        2
#define OBD_DTC_PAGE_INDEX         3
#define VIN_INFO_PAGE_INDEX        4
#define OBD_SETUP_PAGE_INDEX       5
#define CAN_RAW_VIEW_PAGE_INDEX    6
#define MENU_PAGE_INDEX            7

static struct TMainWin MainWin;
guint EventTimerId;
gchar *BaseDir;
static guint64 AppStatusTimeStamp;
static gint AppStatus = APP_INIT;
static gint PrevPage = -1;


static void MenuCB(GtkButton *button, gpointer user_data);


static void PathsInit(gchar *prog)
{
BaseDir = g_path_get_dirname(prog);
g_chdir(BaseDir);
}


void SendEventToWindows(uint32_t event)
{
Obd2DataWinEvent(MainWin.Obd2DataWin, event);
Obd2ListWinEvent(MainWin.Obd2ListWin, event);
Obd2DtcWinEvent(MainWin.Obd2DtcWin, event);
PidSelWinEvent(MainWin.PidSelWin, event);
}


void SetBackgroundImage(GtkWidget *window, const gchar *filename)
{
GdkPixmap *background;
GdkPixbuf *pixbuf;
GtkStyle *style;
GError *error = NULL;

pixbuf = gdk_pixbuf_new_from_file (filename, &error);
if (error != NULL)
  {
  if (error->domain == GDK_PIXBUF_ERROR)
    g_print ("Pixbuf Related Error:\n");
  if (error->domain == G_FILE_ERROR)
    g_print ("File Error: Check file permissions and state:\n");
  g_printerr ("%s\n", error[0].message);
  }
gdk_pixbuf_render_pixmap_and_mask (pixbuf, &background, NULL, 0);

style = gtk_style_new();
style->bg_pixmap[0] = background;
gtk_widget_set_style(GTK_WIDGET(window), GTK_STYLE(style));
}


static void GuiSetPage(guint index)
{
char *str, *vin, *vin_data;
uint32_t saved_dtcs_num;


if (index != PrevPage)
  {
  // Hide Page
  switch (PrevPage)
    {
    case START_PAGE_INDEX :
            {
            break;
            }
    case DEFAULT_VALUES_PAGE_INDEX :
            {
            break;
            }
    case OBD_LIST_PAGE_INDEX :
            {
            break;
            }
    case OBD_DTC_PAGE_INDEX :
            {
            break;
            }
    case VIN_INFO_PAGE_INDEX :
            {
            break;
            }
    case OBD_SETUP_PAGE_INDEX :
            {
            SendEventToWindows(APP_EVENT_PID_SELECT_CHANGE);
            break;
            }
    case CAN_RAW_VIEW_PAGE_INDEX :
            {
            break;
            }
    case MENU_PAGE_INDEX :
            {
            break;
            }
    }
  // Show Page
  switch (index)
    {
    case START_PAGE_INDEX :
            {
            break;
            }
    case DEFAULT_VALUES_PAGE_INDEX :
            {
            break;
            }
    case OBD_LIST_PAGE_INDEX :
            {
            break;
            }
    case OBD_DTC_PAGE_INDEX :
            {
            Obd2DtcWinEvent(MainWin.Obd2DtcWin, APP_EVENT_SHOW);
            break;
            }
    case VIN_INFO_PAGE_INDEX :
            {
            VinInfoWinEvent(MainWin.VinInfoWin, APP_EVENT_SHOW);
            break;
            }
    case OBD_SETUP_PAGE_INDEX :
            {
            break;
            }
    case CAN_RAW_VIEW_PAGE_INDEX :
            {
            break;
            }
    case MENU_PAGE_INDEX :
            {
            break;
            }
    }
  PrevPage = index;
  }

if ((index == MENU_PAGE_INDEX) || (index == START_PAGE_INDEX))
  gtk_widget_hide(MainWin.StatusBar);
else
  {
  gtk_widget_show(MainWin.StatusBar);
  vin_data = Obd2ValueGetAsString(MainWin.Obd2, 9, PID_GET_VIN, NULL);
  vin = &vin_data[1];
  saved_dtcs_num = Obd2ValueGetAsU32(MainWin.Obd2, 1, PID_MONITOR_STATUS_DTCS, NULL);
  saved_dtcs_num = (saved_dtcs_num >> 24) & 0x7F;
  if (saved_dtcs_num)
    {
    gtk_widget_show(MainWin.StatusEventBox);
    str = g_markup_printf_escaped("<span foreground=\"red\"><b>%u</b> Fehler gespeichert</span>", saved_dtcs_num);
    gtk_label_set_markup(GTK_LABEL(MainWin.StatusWarnText), str);
    safe_free(str);
    }
  else
    gtk_widget_hide(MainWin.StatusEventBox);  
  str = g_markup_printf_escaped("<span foreground=\"blue\">Verbunden mit: <b>%s</b></span>", vin);
  gtk_label_set_markup(GTK_LABEL(MainWin.StatusInfoText), str);
  safe_free(str);
  safe_free(vin_data);
  }
gtk_notebook_set_current_page(GTK_NOTEBOOK(MainWin.Notebook), index);
}


static void SetupFullscreen(void)
{
if (Setup.ShowFullscreen)
  gtk_window_fullscreen(GTK_WINDOW(MainWin.Main));
else
  gtk_window_unfullscreen(GTK_WINDOW(MainWin.Main));
}


static void FullScreenCB(GtkToggleButton *togglebutton, gpointer user_data)
{
if (gtk_toggle_button_get_active(togglebutton))
  Setup.ShowFullscreen = 1;
else
  Setup.ShowFullscreen = 0;
SetupFullscreen();
}


/******************************************************************************/
/*                                 Menü Seite                                 */
/******************************************************************************/
static GtkWidget *MainMenu(void)
{
GtkWidget *widget, *box, *btn_bar, *image;

//box = gtk_vbox_new(FALSE, 10); <*>
box = gtk_table_new(4, 2, FALSE);
gtk_container_set_border_width(GTK_CONTAINER(box), 5);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">Default Werte anzeigen</span>", "Standert Werte anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)DEFAULT_VALUES_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 0, 1, 0, 1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">OBD Daten anzeigen</span>", "Alle aktuellen OBD Daten anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)OBD_LIST_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 0, 1, 1, 2, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">Fehlercodes</span>", "Fehlercodes auslesen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)OBD_DTC_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 0, 1, 2, 3, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">VIM Info</span>", "Informationen zum Fahrzeug anzeigen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)VIN_INFO_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 1, 2, 0, 1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">OBD Setup</span>", "PIDs auswählen");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)OBD_SETUP_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 1, 2, 1, 2, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

widget = create_menue_button(NULL, "<span weight=\"bold\" size=\"x-large\">CAN-Trace</span>", "Anzeige der CAN Rohdaten");
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)CAN_RAW_VIEW_PAGE_INDEX);
//gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), widget, 1, 2, 2, 3, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(0), 0, 0);

btn_bar = gtk_hbox_new(FALSE, 10);
//gtk_box_pack_end(GTK_BOX(box), btn_bar, FALSE, FALSE, 0); <*>
gtk_table_attach(GTK_TABLE(box), btn_bar, 0, 2, 3, 4, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(GTK_FILL), 0, 0);

widget = gtk_toggle_button_new_with_label("Vollbild");
g_signal_connect((gpointer)widget, "toggled", G_CALLBACK(FullScreenCB), NULL);
gtk_box_pack_start(GTK_BOX(btn_bar), widget, FALSE, FALSE, 0);

widget = gtk_button_new();
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)1000);
image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
gtk_container_add(GTK_CONTAINER(widget), image);
gtk_box_pack_end(GTK_BOX(btn_bar), widget, FALSE, FALSE, 0);

return(box);
}


static gint EventTimerProc(gpointer data)
{
gint page;
(void)data;

page = gtk_notebook_get_current_page(GTK_NOTEBOOK(MainWin.Notebook));
TimeNow = g_get_monotonic_time();
// *** Fehler wieder zur Startseite zurückspringen
if ((page) && (MainWin.CanDevice->Status < DEV_STATUS_READY))
  {
  AppStatus = APP_INIT;
  GuiSetPage(START_PAGE_INDEX);
  }
if (!page)
  {
  switch (AppStatus)
    {
    case APP_INIT  : {
                     StartWinRun(MainWin.StartWin);
                     AppStatusTimeStamp = 0;
                     AppStatus = APP_START;
                     break;
                     }
    case APP_START : break;
    case APP_START_FINISH :
                     {
                     if (!AppStatusTimeStamp)
                       AppStatusTimeStamp = TimeNow;
                     else
                       {
                       if ((TimeNow - AppStatusTimeStamp) >= (2000 * 1000))
                         {
                         StartWinStop(MainWin.StartWin);
                         GuiSetPage(DEFAULT_VALUES_PAGE_INDEX);
                         AppStatus = APP_RUN;
                         break;
                         }
                       }
                     }
    }
  }
else
  {
  switch (page)
    {
    case START_PAGE_INDEX :
            {
            break;
            }
    case DEFAULT_VALUES_PAGE_INDEX :
            {
            Obd2DataWinPaintValues(MainWin.Obd2DataWin);
            break;
            }
    case OBD_LIST_PAGE_INDEX :
            {
            Obd2ListWinPaintValues(MainWin.Obd2ListWin);
            break;
            }
    case OBD_DTC_PAGE_INDEX :
            {
            break;
            }
    case VIN_INFO_PAGE_INDEX :
            {
            break;
            }
    case OBD_SETUP_PAGE_INDEX :
            {
            break;
            }
    case CAN_RAW_VIEW_PAGE_INDEX :
            {
            break;
            }
    case MENU_PAGE_INDEX :
            {
            break;
            }
    }
  }
return(TRUE);
}


static void Obd2EventCB(TObd2 *obd, guint event, void *user_data)
{
TMhsGMessage *g_msg;

g_msg = mhs_g_new_message(MHS_MSG_OBD2_EVENT, &event, sizeof(guint));
mhs_g_message_post(MainScheduler, g_msg);
}


static void MenuCB(GtkButton *button, gpointer user_data)
{
guint index;

index = (uintptr_t)user_data;
if (index == 1000)
  gtk_main_quit();
GuiSetPage(index);
}


static void ShowMenuClickedCB(GtkButton *button, gpointer user_data)
{
if (AppStatus >= APP_RUN)
  GuiSetPage(MENU_PAGE_INDEX);
}


static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
if (AppStatus >= APP_RUN)
  GuiSetPage(MENU_PAGE_INDEX);
return(FALSE);
}


static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
if (AppStatus >= APP_RUN)
  GuiSetPage(MENU_PAGE_INDEX);
return(FALSE);
}


void SetAppStatus(gint app_status)
{
AppStatus = app_status;
if (app_status == APP_START_FINISH)
  SendEventToWindows(APP_EVENT_OBD2_CONNECT);
}


int main(int argc, char *argv[])
{
GtkWidget *win, *vbox, *widget, *notebook, *hbox, *widget2;
gchar *filename;

/* Initialize GTK+ */
gtk_init (&argc, &argv);

PathsInit(argv[0]);
(void)LoadConfigFile();
/* Create the main window */
win = gtk_window_new (GTK_WINDOW_TOPLEVEL);

MainWin.Main = win;
//gtk_container_set_border_width (GTK_CONTAINER (win), 8);
gtk_window_set_title(GTK_WINDOW (win), "OBD Display");
gtk_window_set_position(GTK_WINDOW (win), GTK_WIN_POS_CENTER);
gtk_window_set_default_size(GTK_WINDOW(win), 800, 480);
//gtk_window_set_default_size(GTK_WINDOW(win), 1606, 966);

win = gtk_event_box_new();
filename = g_build_filename(BaseDir, "background.jpg", NULL);
SetBackgroundImage(win, filename);
safe_free(filename);
gtk_container_add(GTK_CONTAINER(MainWin.Main), win);
MainWin.EventBox = win;

g_signal_connect(MainWin.Main, "destroy", gtk_main_quit, NULL);
gtk_widget_set_events(win, GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK);
g_signal_connect(win, "key_press_event", G_CALLBACK(on_key_press), NULL);
g_signal_connect(win, "button_press_event", G_CALLBACK(button_press_event), NULL);
gtk_widget_realize(win);

filename = g_build_filename(BaseDir, "dtc.db", NULL);
MainWin.DtcDb = DtcDbLoad(filename);
safe_free(filename);
filename = g_build_filename(BaseDir, "wmi.db", NULL);
(void)VinWmiDbLoad(filename);
safe_free(filename);

if ((MainWin.CanDevice = CanDevCreate(5000)))
  {
  if ((MainWin.Isotp = IsotpCreate(MainWin.CanDevice, 0)))
    {
    IsotpIdSetup(MainWin.Isotp, OBD2_TX_ID, OBD2_RX_ID, 0);
    if ((MainWin.Obd2 = Obd2Create(MainWin.Isotp, Obd2EventCB, &MainWin)))
      EventTimerId = g_timeout_add(300, (GtkFunction)EventTimerProc, (gpointer)&MainWin);
    }
  }
vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER (win), vbox);

notebook = gtk_notebook_new();
MainWin.Notebook = notebook;
gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

// 1. Seite -> S T A R T .. .
widget = CreateStartWin(&MainWin);
MainWin.StartWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 2. Seite -> Default Werte anzeigen
widget = Obd2DataWinCreate(MainWin.Obd2);
MainWin.Obd2DataWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 3. Seite -> OBD Daten anzeigen
widget = Obd2ListWinCreate(MainWin.Obd2);
MainWin.Obd2ListWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 4. Seite -> Fehlespeicher auslesen
widget = Obd2DtcWinCreate(MainWin.Obd2, MainWin.DtcDb);
MainWin.Obd2DtcWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 5. Seite -> Fehlespeicher auslesen
widget = VinInfoWinCreate(MainWin.Obd2);
MainWin.VinInfoWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 6. Seite -> OBD Setup
widget = PidSelWinCreate(MainWin.Obd2);
MainWin.PidSelWin = widget;
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 7. Seite -> Trace
widget = CreateTraceWdg(MainWin.CanDevice);
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
// 8. Seite -> Menü
widget = MainMenu();
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

hbox = gtk_hbox_new(FALSE, 5);

MainWin.StatusBar = hbox;

widget = gtk_button_new();
gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
widget2 = gtk_hbox_new(FALSE, 0);
MainWin.StatusWarnImage = mhs_get_image(LIST_WDG_WARN_ICONE);
gtk_box_pack_start(GTK_BOX(widget2), MainWin.StatusWarnImage, FALSE, FALSE, 0);
MainWin.StatusWarnText = gtk_label_new(NULL);
gtk_box_pack_start(GTK_BOX(widget2), MainWin.StatusWarnText, FALSE, FALSE, 0);
gtk_container_add(GTK_CONTAINER(widget), widget2);
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(MenuCB), (gpointer)OBD_DTC_PAGE_INDEX);
MainWin.StatusEventBox = widget;
gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
/*
widget = gtk_hbox_new(FALSE, 0); <*>
MainWin.StatusEventBox = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(MainWin.StatusEventBox), widget);
g_signal_connect(MainWin.StatusEventBox, "button-release-event", G_CALLBACK(MenuCB), (gpointer)OBD_DTC_PAGE_INDEX);
MainWin.StatusWarnImage = mhs_get_image(LIST_WDG_WARN_ICONE);
gtk_box_pack_start(GTK_BOX(widget), MainWin.StatusWarnImage, FALSE, FALSE, 0);
MainWin.StatusWarnText = gtk_label_new(NULL);
gtk_box_pack_start(GTK_BOX(widget), MainWin.StatusWarnText, FALSE, FALSE, 0);

gtk_box_pack_start(GTK_BOX(hbox), MainWin.StatusEventBox, FALSE, FALSE, 0);*/

MainWin.StatusInfoText = gtk_label_new(NULL);
gtk_box_pack_start(GTK_BOX(hbox), MainWin.StatusInfoText, TRUE, TRUE, 0);

widget = gtk_button_new();
gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
widget2 = gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_LARGE_TOOLBAR);
gtk_container_add(GTK_CONTAINER(widget), widget2);
g_signal_connect((gpointer)widget, "clicked", G_CALLBACK(ShowMenuClickedCB), NULL);
gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);

gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

gtk_widget_show_all(MainWin.Main);

GuiSetPage(START_PAGE_INDEX);

SetupFullscreen();

gtk_main();

if (EventTimerId)
  g_source_remove(EventTimerId);
DestroyTraceWdg();
Obd2Destroy(&MainWin.Obd2);
IsotpDestroy(&MainWin.Isotp);
CanDevDestroy(&MainWin.CanDevice);
DtcDbFree(&MainWin.DtcDb);
VinWmiDbFree();
ConfigDestroy();
safe_free(BaseDir);
return 0;
}

