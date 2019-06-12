#ifndef __CAN_AUTOBAUD_H__
#define __CAN_AUTOBAUD_H__

#include <glib.h>
#include "can_device.h"
#include "mhs_g_messages.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define AUTOBAUD_10K_EN   0x00000001
#define AUTOBAUD_20K_EN   0x00000002
#define AUTOBAUD_50K_EN   0x00000004
#define AUTOBAUD_100K_EN  0x00000008
#define AUTOBAUD_125K_EN  0x00000010
#define AUTOBAUD_250K_EN  0x00000020
#define AUTOBAUD_500K_EN  0x00000040
#define AUTOBAUD_800K_EN  0x00000080
#define AUTOBAUD_1M_EN    0x00000100
#define AUTOBAUD_ALL_EN   0xFFFFFFFF

#define AUTOBAUD_TEMP_BUFFER_SIZE 255

#define AUTOBAUD_FRAMES_NONE  0
#define AUTOBAUD_FRAMES_STD   1
#define AUTOBAUD_FRAMES_EXT   2
#define AUTOBAUD_FRAMES_MIXED 3


//#define AUTOBAUD_CLOSE_ON_FINISH  0x0001
#define AUTOBAUD_NO_SILENT_MODE     0x0002
#define AUTOBAUD_AKTIV_SCAN         0x0004


struct TCanAutobaud
  {
  struct TCanDevice *CanDevice;
  uint32_t Flags;
  uint32_t EnableSpeeds;
  struct TCanMsg TxMsg;
  
  uint16_t CanSpeed;
  uint8_t FrameFormatType;
  
  uint32_t CanSpeedsCount;
  uint32_t AktivIndex;
  uint32_t AktivTabIndex;
  TMhsGScheduler *Scheduler;
  GThread *Thread;
  TMhsEvent *Event;
  struct TCanMsg RxTempBuffer[AUTOBAUD_TEMP_BUFFER_SIZE];
  };
  

struct TCanAutobaud *CreateCanAutobaud(TMhsGScheduler *scheduler, struct TCanDevice *can_device, uint32_t flags);
void DestroyCanAutobaud(struct TCanAutobaud **autobaud_ref);
void SetupAutobaud(struct TCanAutobaud *autobaud, uint32_t enable_speeds, const struct TCanMsg *tx_msg, uint32_t flags);
void StartCanAutobaud(struct TCanAutobaud *autobaud, uint16_t start_can_speed);
void StopCanAutobaud(struct TCanAutobaud *autobaud);

#ifdef __cplusplus
  }
#endif

#endif

