/***************************************************************************
                        main_event.c  -  description
                             -------------------
    begin             : 04.02.2012
    copyright         : (C) 2008 - 2012 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "util.h"
#include "mhs_event.h"


/***************************************************************/
/* time in ms                                                  */
/***************************************************************/
void mhs_calc_abs_timeout(struct timespec *timeout, uint32_t time)
{
struct timespec now;
#ifdef __APPLE__
struct timeval tv;
#endif
uint32_t rem;

#ifdef __APPLE__
if (gettimeofday(&tv, NULL) < 0)
  return;
TIMEVAL_TO_TIMESPEC(&tv, &now);
#else
clock_gettime(CLOCK_REALTIME, &now);
#endif
timeout->tv_sec = now.tv_sec + (time / 1000);
rem = ((time % 1000) * 1000000);
if ((now.tv_nsec + rem) >= 1000000000)
  {
  timeout->tv_sec++; /* carry bit stored in tv_sec */
  timeout->tv_nsec = (now.tv_nsec + rem) - 1000000000;
  }
else
  timeout->tv_nsec = now.tv_nsec + rem;
}


TMhsEvent *mhs_event_create(void)
{
return(mhs_event_create_ex(sizeof(TMhsEvent)));
}


TMhsEvent *mhs_event_create_ex(int32_t struct_size)
{
TMhsEvent *event;

event = (TMhsEvent *)mhs_calloc(1, struct_size);
if (!event)
  return(NULL);
event->Waiting = MHS_EVENT_STATUS_INIT;
if ((pthread_mutex_init(&event->Mutex, NULL)) == 0)
  {
  if (pthread_cond_init(&event->Cond, NULL) == 0)
    return(event);
  else
    pthread_mutex_destroy(&event->Mutex);
  }
mhs_free(event);
return(NULL);
}


void mhs_event_destroy(TMhsEvent **mhs_event)
{
TMhsEvent *event;

if (!mhs_event)
  return;
if (!(event = *mhs_event))
  return;
pthread_mutex_destroy(&event->Mutex);
pthread_cond_destroy(&event->Cond);
mhs_free(event);
*mhs_event = NULL;
}


uint32_t mhs_event_get_event_mask(TMhsEvent *event)
{
if (!event)
  return(0);
return(event->EventsMask);
}


void mhs_event_set_event_mask(TMhsEvent *event, uint32_t mask)
{
if (!event)
  return;
event->EventsMask = mask;
}


void mhs_event_set_unlocked(TMhsEvent *event, uint32_t events)
{
if ((!event) || (!events))
  return;

if (events & MHS_TERMINATE)
  {
  event->Events = MHS_TERMINATE;
  event->Waiting |= MHS_EVENT_STATUS_TERM_PEND;
  pthread_cond_signal(&event->Cond);
  }
else if (event->EventsMask & events)
  {
  event->Events |= events;
  event->Waiting |= MHS_EVENT_STATUS_PEND;
  pthread_cond_signal(&event->Cond);
  }
}


void mhs_event_set(TMhsEvent *event, uint32_t events)
{
if ((!event) || (!events))
  return;
pthread_mutex_lock(&event->Mutex);
if (events & MHS_TERMINATE)
  {
  event->Events = MHS_TERMINATE;
  event->Waiting |= MHS_EVENT_STATUS_TERM_PEND;
  pthread_cond_signal(&event->Cond);
  }
else if (event->EventsMask & events)
  {
  event->Events |= events;
  event->Waiting |= MHS_EVENT_STATUS_PEND;
  pthread_cond_signal(&event->Cond);
  }
pthread_mutex_unlock(&event->Mutex);
}


void mhs_event_clear_unlocked(TMhsEvent *event, uint32_t events)
{
if ((!event) || (!events))
  return;
if (!(event->Events = ((~events) & event->Events)))
  event->Waiting &= MHS_EVENT_STATUS_PEND_CLEAR;
}


void mhs_event_clear(TMhsEvent *event, uint32_t events)
{
if ((!event) || (!events))
  return;
pthread_mutex_lock(&event->Mutex);
if (!(event->Events = ((~events) & event->Events)))
  event->Waiting &= MHS_EVENT_STATUS_PEND_CLEAR;
pthread_mutex_unlock(&event->Mutex);
}


uint32_t mhs_event_get_unlocked(TMhsEvent *event, int32_t clear)
{
uint32_t events;

if (!event)
  return(0);
if (!event->Events)
  return(0);
events = event->Events;
if (events & MHS_TERMINATE)
  events = MHS_TERMINATE;
else if (events)
  {
  if (clear)
    event->Events = 0;  // Alle Events löschen
  }
else
  event->Waiting &= MHS_EVENT_STATUS_PEND_CLEAR;
return(events);
}


uint32_t mhs_event_get(TMhsEvent *event, int32_t clear)
{
uint32_t events;

if (!event)
  return(0);
if (!event->Events)
  return(0);
pthread_mutex_lock(&event->Mutex);
events = event->Events;
if (events & MHS_TERMINATE)
  events = MHS_TERMINATE;
else if (events)
  {
  if (clear)
    event->Events = 0;  // Alle Events löschen
  }
else
  event->Waiting &= MHS_EVENT_STATUS_PEND_CLEAR;
pthread_mutex_unlock(&event->Mutex);
return(events);
}


uint32_t mhs_wait_for_event(TMhsEvent *event, uint32_t timeout)
{
uint32_t events, event_hit;
struct timespec tabs;

if (!event)
  return(0);
pthread_mutex_lock(&event->Mutex);
events = event->Events;
if (events & MHS_TERMINATE)
  {
  event->Events = MHS_TERMINATE;
  event->Waiting |= MHS_EVENT_STATUS_TERM_PEND;
  }
else if (events)
  event->Events = 0;
else
  event->Waiting &= MHS_EVENT_STATUS_PEND_CLEAR;

if (!events)
  {
  event_hit = 1;
  if (timeout)
    {
    mhs_calc_abs_timeout(&tabs, timeout);
    if (pthread_cond_timedwait(&event->Cond, &event->Mutex, &tabs) == ETIMEDOUT)
      event_hit = 0;
    }
  else
    pthread_cond_wait(&event->Cond, &event->Mutex);

  if (event_hit)
    {
    events = event->Events;
    if (events & MHS_TERMINATE)
      {
      events = MHS_TERMINATE;
      event->Waiting |= MHS_EVENT_STATUS_TERM_PEND;
      pthread_cond_signal(&event->Cond);
      }
    else
      event->Events = 0;
    }
  else
    events = 0;
  }
pthread_mutex_unlock(&event->Mutex);
return(events);
}


int32_t mhs_wait_status(TMhsEvent *event)
{
if (!event)
  return(MHS_EVENT_STATUS_INVALID);
pthread_mutex_lock(&event->Mutex);
pthread_mutex_unlock(&event->Mutex);
return(event->Waiting);
}


/***************************************************************/
/*                                         */
/***************************************************************/
uint32_t mhs_sleep_ex(TMhsEvent *event, uint32_t time)
{
uint32_t events;
struct timespec tabs;

if (!time)
  {
  mhs_sleep(0);
  return(0);
  }
if (!event)
  {
  mhs_sleep(time);
  return(0);
  }
pthread_mutex_lock(&event->Mutex);
mhs_calc_abs_timeout(&tabs, time);
if (pthread_cond_timedwait(&event->Cond, &event->Mutex, &tabs) == ETIMEDOUT)
  events = 0;
else
  {
  if ((events = event->Events))
    pthread_cond_signal(&event->Cond);
  }
pthread_mutex_unlock(&event->Mutex);
return(events);
}
