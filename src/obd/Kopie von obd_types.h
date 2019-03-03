#ifndef __OBD_TYPES_H__
#define __OBD_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif


typedef enum OBD2DataType
  {
	OBD2TypeBitfield,
	OBD2TypeNumeric,
	OBD2TypeString,
	OBD2TypeOther
  } OBD2DataType;


#define ObdU32Value Value.U32
#define ObdValue1   Value.Vals[0]
#define ObdValue2   Value.Vals[1]
#define ObdString   Value.String


typedef union _TObdValue TObdValue;

union _TObdValue
  {
  uint32_t U32;
  double Vals[2];
  char *String;
  };

typedef struct _TOBD2Data TOBD2Data;

typedef void (*OBD2ResponseDecoder)(TOBD2Data *obd_data, uint8_t *data, uint8_t len);

typedef struct _TOBD2Cmd TOBD2Cmd;

struct _TOBD2Cmd
  {
	const char *Name;          // A human-readable description of the command.
  const char *Units;
  const char *FmtStr;
	uint8_t Payload[2];        // The raw request payload, which for all commands is simply a (mode, PID) tuple.
	OBD2DataType ResponseType; // The type of data contained in the response for this command.
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
  uint8_t Status;
  const TOBD2Cmd *Cmd;
  TObdValue Value;
  };


typedef struct _TObd2 TObd2;

struct _TObd2
  {
  struct TIsotp *Isotp;
  int CurrentDataSize;
  TOBD2Data *CurrentData;
  };


#ifdef __cplusplus
  }
#endif

#endif
