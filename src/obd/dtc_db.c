/*******************************************************************************
                           dtc_db.c  -  description
                             -------------------
    begin             : 06.02.2019
    last modify       : 06.02.2019
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
#include "dtc_db.h"

#define MAX_LINE_SIZE 512


/*
******************** ExtractString ********************
*/
static char *ExtractItemString(char *instr, char *outstr, int max, int *result)
{
int len;

*result = -1;
len = 0;
// Führende Leerzeichen löschen
while (*instr == ' ')
  instr++;
// " Zeichen löschen
if (*instr == '"')
  {
  instr++;
  while ((*instr != '"') && (*instr))
    {
    if (++len >= max)
      break;
    *outstr++ = *instr++;
    }
  if (*instr == '"')
    instr++;
  while ((*instr != '|') && (*instr))
    instr++;
  *result = len;
  }
else
  {
  while ((*instr != '|') && (*instr))
    {
    if (++len >= max)
      break;
    *outstr++ = *instr++;
    }
  if (len)
    *result = len;
  }
if (*instr == '|')
  instr++;
*outstr = 0;
return(instr);
}


static TDtcListItem *DtcDbAdd(TDtcListItem **list, const char *dtc_str, const char *description)
{
TDtcListItem *dtc_item, *l;

if (!list)
  return(NULL);
if (!(dtc_item = (TDtcListItem *)mhs_malloc0(sizeof(TDtcListItem))))
  return(NULL);
strncpy(dtc_item->DtcNo, dtc_str, 6);
dtc_item->Description = g_strdup(description);
l = *list;
if (!l)
  *list = dtc_item;
else
  {
  while (l->Next)
    l = l->Next;
  l->Next = dtc_item;
  }
return(dtc_item);
}


TDtcListItem *DtcDbLoad(const char *file_name)
{
TDtcListItem *list;
int res;
FILE *file;
char *str;
char dtc[6];
char line[MAX_LINE_SIZE];
char description[MAX_LINE_SIZE];

list = NULL;
file = fopen(file_name, "r");
if (!file)
  return(NULL);  // Fehler beim öffnen der Datei

while (fgets(line, MAX_LINE_SIZE-1, file))
  {
  str = line;
  // Führende Leerzeichen löschen
  while (*str == ' ')
    str++;
  // Kommentar Zeile / Leerzeile
  if (*str == ';' || *str == '#' || *str == 0)
    continue;
  // DTC
  str = ExtractItemString(str, dtc, 6, &res);
  if (res <= 0)
    continue;
  str = ExtractItemString(str, description, MAX_LINE_SIZE, &res);
  if (res <= 0)
    continue;
  if (!DtcDbAdd(&list, dtc, description))
    return(NULL);
  }
fclose(file);
return(list);
}


void DtcDbFree(TDtcListItem **dtc_db)
{
TDtcListItem *list, *tmp;

if (!dtc_db)
  return;
list = *dtc_db;
while (list)
  {
  tmp = list->Next;
  mhs_free(list);
  safe_free(list->Description);
  list = tmp;
  }
*dtc_db = NULL;
}


TDtcListItem *DtcDbGetItem(TDtcListItem *dtc_db, const char *dtc_str)
{
for (; dtc_db; dtc_db = dtc_db->Next)
  {
  if (!strcmp(dtc_str, dtc_db->DtcNo))
    break;
  }
return(dtc_db);
}


