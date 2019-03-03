#ifndef __OBD2_LIST_WIN_H__
#define __OBD2_LIST_WIN_H__

#include <gtk/gtk.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *Obd2ListWinCreate(TObd2 *obd);
void Obd2ListWinEvent(GtkWidget *widget, uint32_t event);
void Obd2ListWinPaintValues(GtkWidget *widget);


#ifdef __cplusplus
  }
#endif

#endif
