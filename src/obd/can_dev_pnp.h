#ifndef __CAN_DEV_PNP_H__
#define __CAN_DEV_PNP_H__

#include <gtk/gtk.h>
#include "can_drv.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"

#ifdef __cplusplus
  extern "C" {
#endif


/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
struct TCanDevPnP
  {
  //TCanDevice *CanDevice; <*>
  // PnP
  TMhsGScheduler *Scheduler;
  GThread *PnPThread;
  TMhsEvent *PnPEvent;
  int32_t LastNumDevs;
  int32_t DevicesListCount;
  struct TCanDevicesList *DevicesList;
  };
    
/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
struct TCanDevPnP *CanDevPnPCreate(TMhsGScheduler *scheduler);
void CanDevPnPDestroy(struct TCanDevPnP **can_pnp);
void CanDevPnPScan(struct TCanDevPnP *can_pnp);

#ifdef __cplusplus
  }
#endif

#endif