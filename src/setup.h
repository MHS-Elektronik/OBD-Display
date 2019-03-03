#ifndef __SETUP_H__
#define __SETUP_H__

struct TSetup
  {
  int ShowFullscreen;
  };
  
extern struct TSetup Setup;
  
gint LoadConfigFile(void);
void ConfigDestroy(void);

#endif
