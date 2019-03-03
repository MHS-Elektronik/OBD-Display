#ifndef __VIN_WDG_H__
#define __VIN_WDG_H__

#include <glib.h>
#include <gtk/gtk.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *VinWdgCreate(TObd2 *obd);
void VinWdgUpdate(GtkWidget *widget);

#ifdef __cplusplus
  }
#endif

#endif
