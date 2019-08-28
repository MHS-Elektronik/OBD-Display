/***************************************************************************
                           setup.c  -  description
                             -------------------
    begin             : 07.10.2017
    copyright         : (C) 2017 by MHS-Elektronik GmbH & Co. KG, Germany
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
#include <gtk/gtk.h>
#include "main.h"
#include "util.h"
#include "configfile.h"
#include "obd2.h"
#include "setup.h"


static const gchar CONFIG_FILE_NAME[] = "ObdDisplay.cfg";

struct TSetup Setup;


gint LoadConfigFile(TObd2 *obd)
{
ConfigFile *cfgfile;
gchar *filename;
GList *list;
TOBD2Data *obd_data;
gchar *value_str, *pid_str, *endptr;
guint value; 
uint8_t pid;

if (!(filename = g_build_filename(BaseDir, CONFIG_FILE_NAME, NULL)))
  return(-1);
if (!(cfgfile = cfg_open_file(filename)))
  {
  g_free(filename);
  return(-1);
  }
// Sektion GLOBAL
(void)cfg_read_int(cfgfile, "GLOBAL", "ShowFullscreen", &Setup.ShowFullscreen);
// Sektion ENABLE_PIDS
list = cfg_open_section(cfgfile, "ENABLE_PIDS");
while ((pid_str = cfg_read_section_key_value(&list, &value_str)))
  {
  value = (guint)strtoul(value_str, &endptr, 0);  
  safe_free(value_str);
  if (!value)
    continue; 
  pid = (uint8_t)strtoul(pid_str, &endptr, 0);
  if ((obd_data = Obd2GetByPid(obd, 1, pid, NULL)))    
    obd_data->Enabled = 1;  
  }
// Sektion XMLDATABASE
(void)cfg_read_int(cfgfile, "XMLDATABASE", "Enable", &Setup.EnableXMLDatabase);
safe_free(Setup.XMLDbPath);
(void)cfg_read_string(cfgfile, "XMLDATABASE", "DbPath", &Setup.XMLDbPath);
cfg_free(cfgfile);
g_free(filename);
return(0);
}


void ConfigDestroy(void)
{
safe_free(Setup.XMLDbPath);
}





