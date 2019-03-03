#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "can_device.h"
#include "isotp.h"
#include "obd2.h"
#include "dtc_db.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define APP_INIT         0
#define APP_START        1
#define APP_START_FINISH 2
#define APP_RUN          3

#define APP_EVENT_OBD2_CONNECT          1
#define APP_EVENT_PID_SELECT_CHANGE     2
#define APP_EVENT_SHOW                  3
#define APP_EVENT_OBD2_DTC_READ_FINISH  4
#define APP_EVENT_OBD2_DTC_READ_ERROR   5 

struct TMainWin
  {
  GtkWidget *Main;  
  GtkWidget *StatusBar;
  GtkWidget *StatusEventBox;
  GtkWidget *StatusWarnImage;
  GtkWidget *StatusWarnText;
  GtkWidget *StatusInfoText;
  GtkWidget *EventBox;
  GtkWidget *Notebook;
  GtkWidget *StartWin;

  GtkWidget *Obd2DataWin;
  GtkWidget *Obd2ListWin;
  GtkWidget *Obd2DtcWin;
  GtkWidget *VinInfoWin;
  GtkWidget *PidSelWin;

  struct TCanDevice *CanDevice;
  struct TIsotp *Isotp;
  TObd2 *Obd2;
  TDtcListItem *DtcDb;
  };


extern gchar *BaseDir;

void SendEventToWindows(uint32_t event);
void SetBackgroundImage(GtkWidget *window, const gchar *filename);
void SetAppStatus(gint app_status);

#ifdef __cplusplus
  }
#endif

#endif
