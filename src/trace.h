#ifndef __TRACE_H__
#define __TRACE_H__

#include <glib.h>
#include "can_device.h"

#ifdef __cplusplus
  extern "C" {
#endif

GtkWidget *CreateTraceWdg(struct TCanDevice *can_device);
void DestroyTraceWdg(void);

#ifdef __cplusplus
  }
#endif

#endif
