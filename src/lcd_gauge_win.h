#ifndef __LCD_GAUGE_WIN_H__
#define __LCD_GAUGE_WIN_H__

#include <gtk/gtk.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *Obd2DataWinCreate(TObd2 *obd);
void Obd2DataWinEvent(GtkWidget *widget, uint32_t event);
void Obd2DataWinPaintValues(GtkWidget *widget);

#ifdef __cplusplus
  }
#endif

#endif
