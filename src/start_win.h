#ifndef __START_WIN_H__
#define __START_WIN_H__

#include <gtk/gtk.h>
//#include "can_device.h"
#include "main.h"

#ifdef __cplusplus
  extern "C" {
#endif

TMhsGScheduler *MainScheduler;

GtkWidget *CreateStartWin(struct TMainWin *main_win); //struct TCanDevice *can_device); <*>
void StartWinRun(GtkWidget *widget);
void StartWinStop(GtkWidget *widget);

#ifdef __cplusplus
  }
#endif

#endif
