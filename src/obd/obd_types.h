#ifndef __OBD_TYPES_H__
#define __OBD_TYPES_H__

#include <stdint.h>
#include "mhs_thread.h"
#include "isotp.h"

#ifdef __cplusplus
  extern "C" {
#endif


typedef enum OBD2DataType
  {
	OBD2TypeBitfield,
	OBD2TypeNumeric,
	OBD2TypeString,
  OBD2TypeDTC,
	OBD2TypeOther
  } OBD2DataType;


#define ObdU32Value Value.U32
#define ObdValue1   Value.Vals[0]
#define ObdValue2   Value.Vals[1]
#define ObdString   Value.String
#define ObdDTCs     Value.DTCs

typedef struct _TDTCs TDTCs;

struct _TDTCs
  {
  uint32_t Size;
  uint16_t *DtcNo;
  };

typedef union _TObdValue TObdValue;

union _TObdValue
  {
  uint32_t U32;
  double Vals[2];
  char *String;
  TDTCs DTCs;
  };

typedef struct _TOBD2Data TOBD2Data;

typedef void (*OBD2ResponseDecoder)(TOBD2Data *obd_data, uint8_t *data, uint8_t len);

#define OBD_SHOW_NONE  0
#define OBD_SHOW_RAW   1
#define OBD_SHOW_REAL  2

struct TObdShow
  {
  uint32_t Mode1;
  const char *FmtStr1;
  const char *Units1;
  uint32_t Mode2;
  const char *FmtStr2;
  const char *Units2;
  };

typedef struct _TOBD2Cmd TOBD2Cmd;

struct _TOBD2Cmd
  {
  const char *Name;          // A human-readable description of the command.
  const struct TObdShow *Show;
	uint8_t Payload[2];        // The raw request payload, which for all commands is simply a (mode, PID) tuple.
	OBD2DataType ResponseType; // The type of data contained in the response for this command.
  uint8_t Flags;
  uint8_t DataLength;
  double Scale;
  double Offset;
	OBD2ResponseDecoder ResponseDecoder; // A pointer to a function that can decode a raw response payload for this command.
  };

/** Helper macros that return the mode/PID for a given command */
#define OBD2CmdGetMode(cmd) (cmd)->Payload[0]
#define OBD2CmdGetPID(cmd) (cmd)->Payload[1]

struct _TOBD2Data
  {
  uint8_t Supported;
  uint8_t Enabled;
  uint8_t EnableLock;
  uint8_t Status;
  uint8_t Update;
  const TOBD2Cmd *Cmd;
  TObdValue Value;
  };


#define OBD2_INIT                  0
#define OBD2_SPPORTED_PID_SCAN     1
#define OBD2_START_PID_SCAN        2
#define OBD2_PID_SCAN              3


typedef struct _TObd2 TObd2;

typedef void(*TObd2EventCB)(TObd2 *obd, uint32_t event, void *user_data);

struct _TObd2
  {
  struct TIsotp *Isotp;
  uint32_t Status;
  uint32_t AktivPidCount;
  TObd2EventCB Proc;
  void *UserData;
  int AktivIndex;
  // Mode 1 - Current Data
  int CurrentDataSize;
  TOBD2Data *CurrentData;
  // Mode 9 - Vehicle Information
  int VehicleInformationSize;
  TOBD2Data *VehicleInformation;
  // Mode 3 - DTC (Dignostic Trouble Codes)
  TOBD2Data Dtc;
  TMhsThread *Thread;
  };


#ifdef __cplusplus
  }
#endif

#endif
