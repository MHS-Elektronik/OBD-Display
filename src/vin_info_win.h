#ifndef __VIN_INFO_WIN_H__
#define __VIN_INFO_WIN_H__

#include <gtk/gtk.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *VinInfoWinCreate(TObd2 *obd);
void VinInfoWinEvent(GtkWidget *widget, uint32_t event);
void VinInfoWinPaintValues(GtkWidget *widget);


#ifdef __cplusplus
  }
#endif

#endif
