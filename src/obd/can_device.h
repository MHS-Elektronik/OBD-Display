#ifndef __CAN_DEVICE_H__
#define __CAN_DEVICE_H__

#include "can_drv.h"
#include "mhs_thread.h"

#ifdef __cplusplus
  extern "C" {
#endif


/**************************************************************************/
/*                     D E F I N E - M A C R O S                          */
/**************************************************************************/
#define RX_TEMP_BUFFER_SIZE 1024

#define CAN_CORE_INIT         0
#define CAN_CORE_DRV_LOAD     1

#define DEV_STATUS_CLOSE       0
#define DEV_STATUS_READY       1

#define CAN_CORE_DRIVER_ERROR  -1
#define DEV_CAN_OPEN_ERROR     -2

#define CAN_DEV_CAN_NOT_START  0x00000001

/**************************************************************************/
/*                       D A T E N - T Y P E N                            */
/**************************************************************************/
typedef struct TCanCore TCanCore;

struct TCanCore
  {
  char *DriverFileName;
  char *DriverInfoStr;
  char *LastErrorString;
  int32_t Status;
  };

typedef struct _TCanDevRxHandler TCanDevRxHandler;

struct TCanDevice
  {
  TCanCore *CanCore;
  uint32_t DeviceIndex;
  int32_t Status;
  char *LastErrorString;
  struct TCanDeviceInfo DeviceInfo;
  //char *TinyCanInfoStr; <*>
  TMhsThread *Thread;
  TCanDevRxHandler *CanDevRxHandler;
  uint32_t EventTimeout;
  TMhsEvent *Event;
  struct TCanMsg RxTempBuffer[RX_TEMP_BUFFER_SIZE];
  };


typedef void(*TCanDevRxCB)(struct TCanDevice *can_device, struct TCanMsg *msg, void *user_data);


struct _TCanDevRxHandler
  {
  TCanDevRxHandler *Next;
  TCanDevRxCB Proc;
  void *UserData;
  };


//extern TCanCore *CanCore;
extern uint32_t TimeNow;

/**************************************************************************/
/*                        F U N K T I O N E N                             */
/**************************************************************************/
struct TCanDevice *CanDevCreate(uint32_t timeout);
void CanDevDestroy(struct TCanDevice **can_device);
int32_t CanDevOpen(struct TCanDevice *can_device, uint32_t flags, const char *snr, uint16_t speed);
void CanDevClose(struct TCanDevice *can_device);



int32_t CanDevTxMessage(struct TCanDevice *can_device, struct TCanMsg *msgs, int32_t count);

int CanDevRxEventConnect(struct TCanDevice *can_device, TCanDevRxCB proc, void *user_data);
void CanDevRxEventDisconnect(struct TCanDevice *can_device, TCanDevRxCB proc);


#ifdef __cplusplus
  }
#endif

#endif
