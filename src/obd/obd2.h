#ifndef __OBD2_H__
#define __OBD2_H__

#include "obd_types.h"
#include "obd_pids.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define OBD2_ERR_PID_NOT_FOUND      -4
#define OBD2_ERR_PID_NOT_SUPPORTED  -3
#define OBD2_ERR_PID_DISABLED       -2

#define OBD2_ERR_VALUE_INVALID      -1
#define OBD2_VALUE_OK                0
#define OBD2_VALUE_UPDATE            1


// Commandos
#define OBD2_START_CMD     0x0001
#define OBD2_DTC_READ_CMD  0x0002

// Event Callbacks
#define OBD2_EVENT_CONNECT          1
#define OBD2_EVENT_DTC_READ_FINISH  2
#define OBD2_EVENT_DTC_READ_ERROR   3


TObd2 *Obd2Create(struct TIsotp *iso_tp, TObd2EventCB proc, void *user_data);
void Obd2Destroy(TObd2 **obd);
void Obd2Start(TObd2 *obd);
void Obd2Stop(TObd2 *obd);
void Obd2ReadDtc(TObd2 *obd);

int32_t Obd2DataGetAndClearStatus(TOBD2Data *current_data);
TOBD2Data *Obd2GetByPid(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status);
uint32_t Obd2ValueGetAsU32(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status);
double Obd2ValueGetAsReal(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status);
char *Obd2ValueGetAsString(TObd2 *obd, uint8_t mode, uint8_t pid, int32_t *status);
uint16_t *Obd2ValueGetDTCs(TObd2 *obd, int32_t *status, uint32_t *size);

#ifdef __cplusplus
  }
#endif

#endif
