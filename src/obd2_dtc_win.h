#ifndef __OBD2_DTC_WIN_H__
#define __OBD2_DTC_WIN_H__

#include <gtk/gtk.h>
#include "obd2.h"
#include "dtc_db.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *Obd2DtcWinCreate(TObd2 *obd, TDtcListItem *dtc_db);
void Obd2DtcWinEvent(GtkWidget *widget, uint32_t event);
void Obd2DtcWinPaintValues(GtkWidget *widget);


#ifdef __cplusplus
  }
#endif

#endif
