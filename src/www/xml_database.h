#ifndef __XML_DATABASE_H__
#define __XML_DATABASE_H__

#include <glib.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

gint XMLDatabaseCreate(void);
void XMLDatabaseDestroy(void);
void XMLDatabaseUpdate(TObd2 *obd);

#ifdef __cplusplus
  }
#endif

#endif
