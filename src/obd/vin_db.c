/*******************************************************************************
                           vin_db.c  -  description
                             -------------------
    begin             : 07.02.2019
    last modify       : 07.02.2019
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
#include <glib.h> // <*> raus
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "vin_db.h"

#define MAX_LINE_SIZE 512


struct TVinRegions
  {
  char Begin;
  char End;
  char *Region;
  };

struct TVinCountries
  {
  char CountryKey;
  char Begin;
  char End;
  char *Country;
  };

struct TVinYears
  {
  char YearKey;
  char *Year;
  };

typedef struct _TVinWmiListItem TVinWmiListItem;

struct _TVinWmiListItem
  {
  TVinWmiListItem *Next;
  char Wmi[4];
  char *Manufacturer;
  };


static const char VinChars[] = {"ABCDEFGHIJKLMNOPRSTUVWXYZ1234567890"};

static const struct TVinRegions VinRegions[] = {
       {'A', 'H', "Africa"},
       {'J', 'R', "Asia"},
       {'S', 'Z', "Europe"},
       {'1', '5', "North America"},
       {'6', '7', "Oceania"},
       {'8', '9', "South America"},
       {'\0',  '\0',  NULL}};

#define UNKNOWN_REGION "Unknown"

static const struct TVinCountries VinCountries[] = {
       {'A', 'A', 'H', "South Africa"},
       {'A', 'J', 'N', "Ivory Coast"},
       {'B', 'A', 'E', "Angola"},
       {'B', 'F', 'K', "Kenya"},
       {'B', 'L', 'R', "Tanzania"},
       {'C', 'A', 'E', "Benin"},
       {'C', 'F', 'K', "Madagascar"},
       {'C', 'L', 'R', "Tunisia"},
       {'D', 'A', 'E', "Egypt"},
       {'D', 'F', 'K', "Morocco"},
       {'D', 'L', 'R', "Zambia"},
       {'E', 'A', 'E', "Ethiopia"},
       {'E', 'F', 'K', "Mozambique"},
       {'F', 'A', 'E', "Ghana"},
       {'F', 'F', 'K', "Nigeria"},
       {'J', 'A', '0', "Japan"},
       {'K', 'A', 'E', "Sri Lanka"},
       {'K', 'F', 'K', "Israel"},
       {'K', 'L', 'R', "Korea {South}"},
       {'K', 'S', '0', "Kazakhstan"},
       {'L', 'A', '0', "China"},
       {'M', 'A', 'E', "India"},
       {'M', 'F', 'K', "Indonesia"},
       {'M', 'L', 'R', "Thailand"},
       {'N', 'A', 'E', "Iran"},
       {'N', 'F', 'K', "Pakistan"},
       {'N', 'L', 'R', "Turkey"},
       {'P', 'A', 'E', "Philippines"},
       {'P', 'F', 'K', "Singapore"},
       {'P', 'L', 'R', "Malaysia"},
       {'R', 'A', 'E', "United Arab Emirates"},
       {'R', 'F', 'K', "Taiwan"},
       {'R', 'L', 'R', "Vietnam"},
       {'R', 'S', '0', "Saudi Arabia"},
       {'S', 'A', 'M', "United Kingdom"},
       {'S', 'N', 'T', "Germany"},
       {'S', 'U', 'Z', "Poland"},
       {'S', '1', '4', "Latvia"},
       {'T', 'A', 'H', "Switzerland"},
       {'T', 'J', 'P', "Czech Republic"},
       {'T', 'R', 'V', "Hungary"},
       {'T', 'W', '1', "Portugal"},
       {'U', 'H', 'M', "Denmark"},
       {'U', 'N', 'T', "Ireland"},
       {'U', 'U', 'Z', "Romania"},
       {'U', '5', '7', "Slovakia"},
       {'V', 'A', 'E', "Austria"},
       {'V', 'F', 'R', "France"},
       {'V', 'S', 'W', "Spain"},
       {'V', 'X', '2', "Serbia"},
       {'V', '3', '5', "Croatia"},
       {'V', '6', '0', "Estonia"},
       {'W', 'A', '0', "Germany"},
       {'X', 'A', 'E', "Bulgaria"},
       {'X', 'F', 'K', "Greece"},
       {'X', 'L', 'R', "Netherlands"},
       {'X', 'S', 'W', "Russia"},
       {'X', 'X', '2', "Luxembourg"},
       {'X', '3', '0', "Russia"},
       {'Y', 'A', 'E', "Belgium"},
       {'Y', 'F', 'K', "Finland"},
       {'Y', 'L', 'R', "Malta"},
       {'Y', 'S', 'W', "Sweden"},
       {'Y', 'X', '2', "Norway"},
       {'Y', '3', '5', "Belarus"},
       {'Y', '6', '0', "Ukraine"},
       {'Z', 'A', 'R', "Italy"},
       {'Z', 'X', '2', "Slovenia"},
       {'Z', '3', '5', "Lithuania"},
       {'1', 'A', '0', "United States"},
       {'2', 'A', '0', "Canada"},
       {'3', 'A', '7', "Mexico"},
       {'3', '8', '0', "Cayman Islands"},
       {'4', 'A', '0', "United States"},
       {'5', 'A', '0', "United States"},
       {'6', 'A', 'W', "Australia"},
       {'7', 'A', 'E', "New Zealand"},
       {'8', 'A', 'E', "Argentina"},
       {'8', 'F', 'K', "Chile"},
       {'8', 'L', 'R', "Ecuador"},
       {'8', 'S', 'W', "Peru"},
       {'8', 'X', '2', "Venezuela"},
       {'9', 'A', 'E', "Brazil"},
       {'9', 'F', 'K', "Colombia"},
       {'9', 'L', 'R', "Paraguay"},
       {'9', 'S', 'W', "Uruguay"},
       {'9', 'X', '2', "Trinidad & Tobago"},
       {'9', '3', '9', "Brazil"},
       {'\0', '\0','\0', NULL}};

#define UNKNOWN_COUNTRY "Not assigned"

static const struct TVinYears VinYears[] = {
    {'A', "2010"},
    {'B', "2011"},
    {'C', "2012"},
    {'D', "2013"},
    {'E', "2014"},
    {'F', "2015"},
    {'G', "2016"},
    {'H', "2017"},
    {'J', "2018"},
    {'K', "2019"},
    {'L', "2020"},
    {'M', "2021"},
    {'N', "1992"},
    {'P', "1993"},
    {'R', "1994"},
    {'S', "1995"},
    {'T', "1996"},
    {'V', "1997"},
    {'W', "1998"},
    {'X', "1999"},
    {'Y', "2000"},
    {'1', "2001"},
    {'2', "2002"},
    {'3', "2003"},
    {'4', "2004"},
    {'5', "2005"},
    {'6', "2006"},
    {'7', "2007"},
    {'8', "2008"},
    {'9', "2009"},
    {'\0',  NULL}};

static const char NoVinErrorStr[] = {"VIN Error"};

static TVinWmiListItem *VinWmiDb = NULL;


static const uint8_t VinGetCharIdx(const char ch)
{
char c;
uint8_t idx;

for (idx = 0; (c = VinChars[idx]); idx++)
  {
  if (c == ch)
    return(idx);
  }
return(0); // <*> Fehler auswertung ?
}


const char *VinGetCountry(const char *vin)
{
int i;
char ch0, contry_key;
uint8_t ch1_idx;

if (!vin)
  return(NoVinErrorStr);
ch0 = vin[0];
ch1_idx = VinGetCharIdx(vin[1]);
for (i = 0; (contry_key = VinCountries[i].CountryKey); i++)
  {
  if (contry_key == ch0)
    {

    if ((ch1_idx >= VinGetCharIdx(VinCountries[i].Begin)) &&
        (ch1_idx <= VinGetCharIdx(VinCountries[i].End)))
      return(VinCountries[i].Country);
    }
  }
return(UNKNOWN_COUNTRY);
}


const char *VinGetRegion(const char *vin)
{
int i;
char begin;
uint8_t region_idx;

if (!vin)
  return(NoVinErrorStr);
region_idx = VinGetCharIdx(vin[0]);
for (i = 0; (begin = VinRegions[i].Begin); i++)
  {
  if ((region_idx >= VinGetCharIdx(begin)) && (region_idx <= VinGetCharIdx(VinRegions[i].End)))
    return(VinRegions[i].Region);
  }
return(UNKNOWN_REGION);
}

// result must be free!
char *VinGetVds(const char *vin)
{
char *str;

if (!vin)
  return(g_strdup(NoVinErrorStr));  // <*> g raus
if (!(str = malloc(7 * sizeof(char))))
  return(NULL);
memcpy(str, &vin[3], 6);
str[6] = '\0';
return(str);
}

const char *VinGetYear(const char *vin)
{
int i;
char ch0, key;

if (!vin)
  return(NoVinErrorStr);
ch0 = vin[9];
for (i = 0; (key = VinYears[i].YearKey); i++)
  {
  if (key == ch0)
    return(VinYears[i].Year);
  }
return("Unknown");
}


const char VinGetAssemblyPlant(const char *vin)
{
if (!vin)
  return('-');
return(vin[10]);
}


const char *VinGetSerialNo(const char *vin)
{
if (!vin)
  return(NoVinErrorStr);
return(&vin[11]);
}


const char *VinGetManufacturer(const char *vin)
{
char wmi[4];
TVinWmiListItem *db;

if (!vin)
  return(NoVinErrorStr);
memcpy(wmi, vin, 3);
wmi[3] = '\0';
for (db = VinWmiDb; db; db = db->Next)
  {
  if (!strcmp(wmi, db->Wmi))
    return(db->Manufacturer);
  }
return(NULL);
}


static TVinWmiListItem *VinWmiDbAdd(TVinWmiListItem **list, const char *wmi, const char *manufacturer)
{
TVinWmiListItem *item, *l;

if (!list)
  return(NULL);
if (!(item = (TVinWmiListItem *)mhs_malloc0(sizeof(TVinWmiListItem))))
  return(NULL);
strncpy(item->Wmi, wmi, 4);
item->Manufacturer = g_strdup(manufacturer);
l = *list;
if (!l)
  *list = item;
else
  {
  while (l->Next)
    l = l->Next;
  l->Next = item;
  }
return(item);
}


int VinWmiDbLoad(const char *file_name)
{
TVinWmiListItem *list;
int res, len;
FILE *file;
char *str, *wmi;
char line[MAX_LINE_SIZE];

list = NULL;
if (!(file = fopen(file_name, "rt")))
  return(-1);  // Fehler beim öffnen der Datei

while (fgets(line, MAX_LINE_SIZE-1, file))
  {
  str = line;
  wmi = get_item_as_string(&str, "|", &res);
  if (res <= 0)
    continue;
  len = strlen(str) - 1;      
  if (str[len] == '\n')
    str[len] = '\0';  
  if (!VinWmiDbAdd(&list, wmi, str))
    return(-1);
  }
fclose(file);
VinWmiDb = list;
return(0);
}


void VinWmiDbFree(void)
{
TVinWmiListItem *tmp;

while (VinWmiDb)
  {
  tmp = VinWmiDb->Next;
  safe_free(VinWmiDb->Manufacturer);
  mhs_free(VinWmiDb);
  VinWmiDb = tmp;
  }
}

/*void VinWmiDbFree(TVinWmiListItem **db)
{
TVinWmiListItem *list, *tmp;

if (!db)
  return;
list = *db;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  safe_free(list->Manufacturer);
  list = tmp;
  }
*db = NULL;
} */

