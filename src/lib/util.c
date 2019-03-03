/*******************************************************************************
                            util.c  -  description
                             -------------------
    begin             : 26.01.2019
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
#include <sys/time.h>
#include <string.h>
#include <ctype.h>
#include "util.h"



/*
******************** get_tick ********************
*/
#ifndef __WIN32__
uint32_t get_tick(void)
{
struct timeval now;

gettimeofday(&now, NULL);
return((now.tv_sec * 1000) + (now.tv_usec / 1000));
}
#endif


/*
******************** mhs_diff_time ********************
Funktion  : Differenzeit zwischen now und stamp
            berechnen unter berücksichtigung eines
            Überlaufs

Eingaben  : now => Aktuelle Zeit
            stamp => Zeitstempel in der Vergangenheit

Ausgaben  : result => Differenz-Zeit x = now - stamp

Call's    : keine
*/
uint32_t mhs_diff_time(uint32_t now, uint32_t stamp)
{
if (stamp > now)
  return((0xFFFFFFFF - stamp) + now + 1);
else
  return(now - stamp);
}


#define SET_RESULT(result, value) if ((result)) *(result) = (value)

char *get_item_as_string(char **str, char *trenner, int *result)
{
int hit, cnt, l;
char *s, *t, *start, *end, *item;

if ((!str) || (!trenner))
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
s = *str;
if (!s)
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
// Führende Leerzeichen überspringen
if (*s == '\0')
  {
  SET_RESULT(result, -1);
  return(NULL);
  }
SET_RESULT(result, 0);
end = s;
item = s;
start = s;

hit = 0;
for (s = end; *s; s++)
  {
  cnt = 0;
  for (t = trenner; *t; t++)
    {
    cnt++;
    if (*s == *t)
      {
      *s++ = '\0';
      end = s;
      SET_RESULT(result, cnt);
      hit = 1;
      break;
      }
    }
  if (hit)
    break;
  }
// Abschliesende Leerzeichen löschen
if ((l = strlen(item)))
  {
  s = item + (l-1);
  while ((l) && ((isspace((int)*s)) || (*s == '\n') || (*s == '\r')))
  {
    l--;
    s--;
  }
  if (l)
    s++;
  *s = 0;
  }
if (end == start)
  *str = start + strlen(end);
else
  *str = end;
return(item);
}
