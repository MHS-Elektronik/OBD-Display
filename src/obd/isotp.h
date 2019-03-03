#ifndef __ISOTP_H__
#define __ISOTP_H__

#include "util.h"
#include "mhs_thread.h"
#include "mhs_queue.h"
#include "can_device.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define MAX_MSG_LENGTH 8200

// N_PCI type values in bits 7-4 of N_PCI bytes
#define N_PCI_SF  0x00   // single frame
#define N_PCI_FF  0x10   // first frame
#define N_PCI_CF  0x20   // consecutive frame
#define N_PCI_FC  0x30   // flow control

// Flow Status given in FC frame
#define ISOTP_FC_CTS    0    // clear to send
#define ISOTP_FC_WT     1    // wait
#define ISOTP_FC_OVFLW  2    // overflow

#define ISOTP_IDLE           0
#define ISOTP_WAIT_FIRST_FC  1
#define ISOTP_WAIT_FC        2
#define ISOTP_WAIT_DATA      3
#define ISOTP_SENDING        4


// Events
#define ISOTP_RX_EVENT       0x01

#define ISOTP_TX_TIMEOUT_SET 0x01
#define ISOTP_TX_CANCEL      0x02

// Flags
#define CAN_ISOTP_29BIT_ID      1
#define CAN_ISOTP_EXTEND_ADDR   2
#define CAN_ISOTP_TX_PADDING    4
#define CAN_ISOTP_LISTEN_MODE   8

struct TpCon
  {
  uint8_t State;
  uint32_t Size;
  uint32_t Count;
  uint8_t PktIdx;
  uint8_t FcBs;
  uint8_t FcStmin;
  uint8_t BsCounter;
  uint32_t Timestamp;

  uint8_t *Ptr;
  uint8_t  Buf[MAX_MSG_LENGTH+1];
  };


struct TIsotp
  {
  struct TCanDevice *CanDevice;
  TMhsQueue *RxQueue;
  TMhsEvent *Event;
  TMhsThread *Thread;
  uint32_t TimeoutSet;
  uint32_t Flags;
  uint32_t TxId;
  uint32_t RxId;
  uint8_t ExtAddress;
  uint8_t ExtRxAddress;
  uint8_t TxPadContent;

  struct TpCon Rx;
  struct TpCon Tx;
  };


struct TIsotp *IsotpCreate(struct TCanDevice *can_device, uint32_t flags);
void IsotpDestroy(struct TIsotp **tp);
void IsotpSetup(struct TIsotp *tp, uint32_t flags, uint8_t bs, uint8_t st_min, uint8_t tx_pad_content);
void IsotpIdSetup(struct TIsotp *tp, uint32_t tx_id, uint32_t rx_id, uint8_t eff);
void IsotpExtAdrSetup(struct TIsotp *tp, uint8_t tx_ext_adr, uint8_t rx_ext_adr);
int32_t IsotpSend(struct TIsotp *tp, const uint8_t *data, uint32_t size);
int32_t IsotpSendReceive(struct TIsotp *tp, const uint8_t *tx_data, uint32_t tx_size,
        uint8_t **rx_data, uint32_t *rx_size, uint32_t timeout);

#ifdef __cplusplus
  }
#endif

#endif
