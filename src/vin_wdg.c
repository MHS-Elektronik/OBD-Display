/*******************************************************************************
                          vin_wdg.c  -  description
                             -------------------
    begin             : 28.02.2017
    last modify       : 08.02.2019
    copyright         : (C) 2017 - 2019 by MHS-Elektronik GmbH & Co. KG, Germany
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
#include <glib.h>
#include <gtk/gtk.h>
#include "util.h"
#include "gtk_util.h"
#include "obd2.h"
#include "vin_db.h"
#include "vin_wdg.h"


static const gchar *VinWdgTableStr[] = {
  "Hersteller",
  "Region / Land",
  "Baureihe, Motortyp & Ausstattung",
  "Herstellerwerk",
  "Modelljahr",
  "Seriennummer",
  "ECU Name",
  NULL};


static gchar *GetVinInfoValue(guint idx, gpointer *data)
{
TObd2 *obd;
const char *region, *country;
char *ret_str, *vin_data, *vin, *ecu_name;

obd = (TObd2 *)data;
if (!(vin_data = Obd2ValueGetAsString(obd, 9, PID_GET_VIN, NULL)))
  return(g_strdup("?"));
vin = &vin_data[1]; // <*>
ecu_name = Obd2ValueGetAsString(obd, 9, PID_GET_ECU_NAME, NULL);
ret_str = NULL;
switch (idx)
  {
  case 0 : {  // Hersteller
           ret_str = g_strdup(VinGetManufacturer(vin));
           break;
           }
  case 1 : {  // Region / Land
           region = VinGetRegion(vin);
           country = VinGetCountry(vin);
           ret_str = g_strdup_printf("%s / %s", region, country);
           break;
           }
  case 2 : {  // Baureihe, Motortyp && Ausstattung
           ret_str = VinGetVds(vin);
           break;
           }
  case 3 : {  // Herstellerwerk
           ret_str = g_strdup_printf("%c", VinGetAssemblyPlant(vin));
           break;
           }
  case 4 : {  // Modelljahr
           ret_str = g_strdup(VinGetYear(vin));
           break;
           }
  case 5 : {  // Seriennummer
           ret_str = g_strdup(VinGetSerialNo(vin));
           break;
           }
  case 6 : {
           if (ecu_name)
             ret_str = ecu_name;
           else
             ret_str = g_strdup("Unknown");
           break;
           }
  }
safe_free(vin_data);
return(ret_str);
}



GtkWidget *VinWdgCreate(TObd2 *obd)
{
GtkWidget *table;

table = CreateInfoTable(VinWdgTableStr, GetVinInfoValue, (gpointer)obd);
return(table);
}


void VinWdgUpdate(GtkWidget *widget)
{
InfoTableUpdate(widget);
}



