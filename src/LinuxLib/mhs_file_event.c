/***************************************************************************
                       msh_file_event.c  -  description
                             -------------------
    begin             : 13.03.2012
    copyright         : (C) 2011 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/
//#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "mhs_file_event.h"



struct TFileEvent *create_file_event(void)
{
struct TFileEvent *file_event;
int32_t fcntl_flags;

file_event = (struct TFileEvent *)g_malloc0(sizeof(struct TFileEvent));
if (!file_event)
  return(NULL);
if (pipe(file_event->EventPipe) == 0)
  {
  fcntl_flags = fcntl(file_event->EventPipe[0], F_GETFL);
  fcntl_flags |= (O_NONBLOCK | FD_CLOEXEC);
  fcntl (file_event->EventPipe[0], F_SETFL, fcntl_flags);

  fcntl_flags = fcntl(file_event->EventPipe[1], F_GETFL);
  fcntl_flags |= (O_NONBLOCK | FD_CLOEXEC);
  fcntl (file_event->EventPipe[1], F_SETFL, fcntl_flags);
  return(file_event);
  }
else
  {
  g_free(file_event);
  return(NULL);
  }
}


void destroy_file_event(struct TFileEvent **file_event)
{
struct TFileEvent *fev;

if (!file_event)
  return;
if ((fev = *file_event))
  {
  if (fev->EventPipe[0] != -1)
    {
    close(fev->EventPipe[0]);
    fev->EventPipe[0] = -1;
    }
  if (fev->EventPipe[1] != -1)
    {
    close(fev->EventPipe[1]);
    fev->EventPipe[1] = -1;
    }
  g_free(fev);
  }
*file_event = NULL;
}


int32_t set_file_event(struct TFileEvent *file_event, unsigned char event)
{
int32_t c;

if (!event)
  return(0);
if (!file_event)
  return(-1);
if (file_event->EventPipe[1] == -1)
  return(-1);
do
  c = write (file_event->EventPipe[1], &event, 1);
while ((c == -1) && (errno == EINTR));
if (c < 0)
  return(-1);
return(0);
}


unsigned char get_file_event(struct TFileEvent *file_event)
{
unsigned char ch;
int32_t c;

if (!file_event)
  return(0);
if (file_event->EventPipe[0] == -1)
  return(0);
do
  c = read (file_event->EventPipe[0], &ch, 1);
while ((c == -1) && (errno == EINTR));
if (c <= 0)
  return(0);
return(ch);
}


int32_t file_event_get_fd(struct TFileEvent *file_event)
{
if (!file_event)
  return(-1);
return(file_event->EventPipe[0]);
}
