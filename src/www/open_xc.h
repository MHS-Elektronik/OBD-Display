#ifndef __OPEN_XC_H__
#define __OPEN_XC_H__

#include <stdint.h>
#include <glib.h>
#include "obd2.h"
#include "sock_lib.h"
#include "mhs_thread.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define OPEN_XC_MESSAGE_BUFFER_SIZE 32768

typedef struct _TOpenXc TOpenXc;

struct _TOpenXc
  {
  TObd2 *Obd;
  TSockLib *SockLib;
  char MsgBuffer[OPEN_XC_MESSAGE_BUFFER_SIZE];
  TMhsThread *Thread;
  };


TOpenXc *OpenXcCreate(TObd2 *obd, const char *ip, int port);
void OpenXcDestroy(TOpenXc **open_xc);

#ifdef __cplusplus
  }
#endif

#endif
