#ifndef __OBD2_PID_SEL_H__
#define __OBD2_PID_SEL_H__

#include <gtk/gtk.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *PidSelWinCreate(TObd2 *obd);
void PidSelWinEvent(GtkWidget *widget, uint32_t event);

#ifdef __cplusplus
  }
#endif

#endif
