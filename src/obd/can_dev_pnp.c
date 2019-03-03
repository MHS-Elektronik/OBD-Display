/*******************************************************************************
                        can_dev_pnp.c  -  description
                             -------------------
    begin             : 27.01.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include <gtk/gtk.h>
#include "util.h"
#include "can_drv.h"
#include "mhs_g_messages.h"
#include "mhs_msg_types.h"
#include "can_dev_pnp.h"

#define PNP_EVENT    0x00000001
#define SCAN_EVENT   0x00000002

/********************/
/* PnP Event Thread */
/********************/
static gpointer PnPThreadExecute(gpointer data)
{
uint32_t event;
int32_t num_devs;
struct TCanDevPnP *can_pnp;
TMhsGMessage *g_msg;

can_pnp = (struct TCanDevPnP *)data;
do
  {
  event = CanExWaitForEvent(can_pnp->PnPEvent, 0);
  if (event & 0x80000000)        // Beenden Event, Thread Schleife verlassen
    break;
  else if (event & (PNP_EVENT | SCAN_EVENT))    // Pluy &  Play Event
    {
    can_pnp->DevicesListCount = 0;
    CanExDataFree((void **)&can_pnp->DevicesList);
    if ((num_devs = CanExGetDeviceList(&can_pnp->DevicesList, 0)) > 0)
      can_pnp->DevicesListCount = num_devs;
    if ((!(event & PNP_EVENT)) || (num_devs != can_pnp->LastNumDevs))
      {
      can_pnp->LastNumDevs = num_devs;
      g_msg = mhs_g_new_message(MHS_MSG_PNP_EVENT, &num_devs, sizeof(int32_t));
      mhs_g_message_post(can_pnp->Scheduler, g_msg);
      }
    }
  }
while (1);
return(NULL);
}


struct TCanDevPnP *CanDevPnPCreate(TMhsGScheduler *scheduler)
{
struct TCanDevPnP *can_pnp;

if (!scheduler)
  return(NULL);
if (!(can_pnp = (struct TCanDevPnP *)g_malloc0(sizeof(struct TCanDevPnP))))
  return(NULL);
can_pnp->LastNumDevs = -1;
can_pnp->Scheduler = scheduler;
can_pnp->PnPEvent = CanExCreateEvent();   // Event Objekt erstellen
CanExSetObjEvent(INDEX_INVALID, MHS_EVS_PNP, can_pnp->PnPEvent, PNP_EVENT);
// Thread erzeugen und starten
can_pnp->PnPThread = g_thread_new(NULL, PnPThreadExecute, can_pnp);
return(can_pnp);
}


void CanDevPnPDestroy(struct TCanDevPnP **can_pnp)
{
struct TCanDevPnP *pnp;

if (!can_pnp)
  return;
if (!(pnp = *can_pnp))
  return;
if (pnp->PnPThread)
  {
  // Terminate Event setzen
  CanExSetEvent(pnp->PnPEvent, MHS_TERMINATE);
  g_thread_join(pnp->PnPThread);
  pnp->PnPThread = NULL;
  }
g_free(pnp);
*can_pnp = NULL;
}


void CanDevPnPScan(struct TCanDevPnP *can_pnp)
{
CanExSetEvent(can_pnp->PnPEvent, SCAN_EVENT);
}
