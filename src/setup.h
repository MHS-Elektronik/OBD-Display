#ifndef __SETUP_H__
#define __SETUP_H__

#include <glib.h>
#include "obd2.h"

#ifdef __cplusplus
  extern "C" {
#endif

struct TSetup
  {
  int ShowFullscreen;
  int EnableXMLDatabase;
  gchar *XMLDbPath;
  };
  
extern struct TSetup Setup;
  
gint LoadConfigFile(TObd2 *obd);
void ConfigDestroy(void);

#ifdef __cplusplus
  }
#endif

#endif
