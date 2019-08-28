/*******************************************************************************
                         xml_database.c  -  description
                             -------------------
    begin             : 31.07.2019
    copyright         : (C) 2019 by MHS-Elektronik GmbH & Co. KG, Germany
                               http://www.mhs-elektronik.de
    autho             : Klaus Demlehner, klaus@mhs-elektronik.de
 *******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the MIT License <LICENSE.TXT or                 *
 *   http://opensource.org/licenses/MIT>                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "util.h"
#include "setup.h"
#include "main.h" 
#include "xml_database.h"

static const guint XMLFilesRefreshTime = 1000;  // 1 Sek. 

static const gchar DashboardXMLFilename[] = {"dashboard.xml"};
static const gchar StatusXMLFilename[] = {"status.xml"};


static const gchar DashboardXML[] = {
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  "<dashboard>\n"
  "  <Speed>%3.0f</Speed>\n"
	"  <Rpm>%5.0f</Rpm>\n"
  "  <EngineLoad>%3.0f</EngineLoad>\n"
  "  <MAP>%3.0f</MAP>\n"
  "  <WaterTemp>%3.0f</WaterTemp>\n"
  "  <FuelLevel>%3.0f</FuelLevel>\n"
  "</dashboard>"};

static const gchar StatusXML[] = {
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  "<status>\n"
  "  <Speed>%3.0f</Speed>\n"
	"  <Rpm>%5.0f</Rpm>\n"
  "  <EngineLoad>%3.0f</EngineLoad>\n"
  "  <MAP>%3.0f</MAP>\n"
  "  <WaterTemp>%3.0f</WaterTemp>\n"
  "  <FuelLevel>%3.0f</FuelLevel>\n"
  "</status>"};


struct TXmlDb
  {
  guint64 LastEventTime;
  };

struct TXmlDb *XmlDb = NULL;


gint XMLDatabaseCreate(void)
{
gint res;
int size;
FILE *file;
gchar *str, *filename;

res = 0;
if (!Setup.EnableXMLDatabase)
  return(0);
// *** Prüfen ob XML Datei erzeugt werden kann  
str = g_strdup_printf(StatusXML, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
filename = g_build_filename(Setup.XMLDbPath, StatusXMLFilename, NULL);
if ((file = fopen(filename, "wb")))
	{
  size = strlen(str);
  (void)fwrite(str, 1, size, file);
  fclose(file);
  }
else
  res = -1;  
safe_free(filename);  
safe_free(str);
if (!res)
  {  
  if (!(XmlDb = (struct TXmlDb *)g_malloc0(sizeof(struct TXmlDb))))
    res = -1;
  }  
return(res);
}


void XMLDatabaseDestroy(void)
{
safe_free(XmlDb);
}


void XMLDatabaseUpdate(TObd2 *obd)
{
FILE *file;
gchar *str, *s, *filename;
gchar c;
int size;
double speed, rpm, engine_load, map, water_temp, fuel_level;  

if (!XmlDb)
  return;
str = NULL;
if ((XmlDb->LastEventTime > TimeNow) ||
   ((TimeNow - XmlDb->LastEventTime) >= (XMLFilesRefreshTime * 1000)))
  {
  XmlDb->LastEventTime = TimeNow;
  speed       = Obd2ValueGetAsReal(obd, 1, PID_SPEED, NULL);
  rpm         = Obd2ValueGetAsReal(obd, 1, PID_RPM, NULL);
  engine_load = Obd2ValueGetAsReal(obd, 1, PID_ENGINE_LOAD, NULL);
  map         = Obd2ValueGetAsReal(obd, 1, PID_INTAKE_MAP, NULL);
  water_temp  = Obd2ValueGetAsReal(obd, 1, PID_COOLANT_TEMP, NULL);
  fuel_level  = Obd2ValueGetAsReal(obd, 1, PID_FUEL_LEVEL, NULL);
  // Status
  str = g_strdup_printf(StatusXML, speed, rpm, engine_load, map, water_temp, fuel_level);
  filename = g_build_filename(Setup.XMLDbPath, StatusXMLFilename, NULL);
  if ((file = fopen(filename, "wb")))
	  {
    size = strlen(str);
    (void)fwrite(str, 1, size, file);
    fclose(file);
    }
  safe_free(filename);  
  safe_free(str);
  // Dashboard
  str = g_strdup_printf(DashboardXML, speed, rpm, engine_load, map, water_temp, fuel_level);                        
  for (s = str; (c = *s); s++)
    {
    if (c == ',')
      *s = '.';
    }
  filename = g_build_filename(Setup.XMLDbPath, DashboardXMLFilename, NULL);
  if ((file = fopen(filename, "wb")))
	  {
    size = strlen(str);
    (void)fwrite(str, 1, size, file);
    fclose(file);
    }
  safe_free(filename);  
  safe_free(str);  
  }
}
