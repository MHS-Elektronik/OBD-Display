/***************************************************************************
                        mhs_thread.c  -  description
                             -------------------
    begin             : 08.03.2012
    copyright         : (C) 2012 by MHS-Elektronik GmbH & Co. KG, Germany
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
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "util.h"
#include "mhs_event.h"
#include "mhs_thread.h"


#ifdef __APPLE__

struct args
  {
  int32_t joined;
  pthread_t td;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  void **res;
  };


static void *waiter(void *ap)
{
struct args *args = ap;

pthread_join(args->td, args->res);
pthread_mutex_lock(&args->mtx);
pthread_mutex_unlock(&args->mtx);
args->joined = 1;
pthread_cond_signal(&args->cond);
return(0);
}


static int32_t pthread_timedjoin_np(pthread_t td, void **res, struct timespec *ts)
{
pthread_t tmp;
int32_t ret;
struct args args = { .td = td, .res = res };

pthread_mutex_init(&args.mtx, 0);
pthread_cond_init(&args.cond, 0);
pthread_mutex_lock(&args.mtx);

if (!(ret = pthread_create(&tmp, 0, waiter, &args)))
  {
  do
    ret = pthread_cond_timedwait(&args.cond, &args.mtx, ts);
  while (!args.joined && ret != ETIMEDOUT);

  pthread_cancel(tmp);
  pthread_join(tmp, 0);
  }
pthread_cond_destroy(&args.cond);
pthread_mutex_destroy(&args.mtx);

return(args.joined ? 0 : ret);
}

#endif

static void *thread_execute(void *data)
{
TMhsThread *self = (TMhsThread*)data;

// **** Thread Initialisieren
(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
(void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
// **** Thread Funktion
self->Func(self);
// **** Thread beendet
((TMhsEvent *)self)->Waiting |= MHS_EVENT_STATUS_TERMINATE;

return(NULL);
}


TMhsThread *mhs_create_thread(MhsThreadFunc func, void *data, int32_t priority, int32_t run)
{
TMhsThread *mhs_thread;

mhs_thread = (TMhsThread *)mhs_event_create_ex(sizeof(TMhsThread));
if (!mhs_thread)
  return(NULL);
mhs_thread->Run = 0;
mhs_thread->Thread = (pthread_t)-1;
mhs_thread->Data = data;
mhs_thread->Func = func;
mhs_thread->Priority = priority;
((TMhsEvent *)mhs_thread)->Waiting &= ~MHS_EVENT_STATUS_TERMINATE;
// Main Thread erzeugen und starten, wenn "run" 1
if (run)
  {
  if (mhs_run_thread(mhs_thread) < 0)
    {
    mhs_event_destroy((TMhsEvent **)mhs_thread);
    return(NULL);
    }
  }
return(mhs_thread);
}


int32_t mhs_run_thread(TMhsThread *mhs_thread)
{
int32_t err, policy;
pthread_attr_t thread_attr;
struct sched_param param;

if (!mhs_thread)
  return(-1);
if (mhs_thread->Run)
  return(-1);
err = 0;
((TMhsEvent *)mhs_thread)->Waiting &= ~MHS_EVENT_STATUS_TERMINATE;
mhs_thread->Run = 1;
switch (mhs_thread->Priority)
  {
  case 0 :   // Normal
  case 1 : {
           policy = SCHED_OTHER;
           break;
           }
  case 2 : {
           policy = SCHED_RR;
           break;
           }
  case 3 : { // Am höchsten
           policy = SCHED_FIFO;
           break;
           }
  default : policy = SCHED_OTHER;
  }
pthread_attr_init(&thread_attr);
pthread_attr_setschedpolicy(&thread_attr, policy);
pthread_attr_getschedparam(&thread_attr, &param);
param.sched_priority = sched_get_priority_max(policy);
pthread_attr_setschedparam(&thread_attr, &param);

if (pthread_create(&mhs_thread->Thread, &thread_attr, thread_execute, mhs_thread))
  {
  mhs_thread->Thread = (pthread_t)-1;
  mhs_thread->Run = 0;
  err = -1;
  }
pthread_attr_destroy(&thread_attr);
return(err);
}


int32_t mhs_run_thread_ex(TMhsThread *mhs_thread, void *data)
{
if (!mhs_thread)
  return(-1);
mhs_thread->Data = data;
return(mhs_run_thread(mhs_thread));
}


int32_t mhs_destroy_thread(TMhsThread **mhs_thread, uint32_t timeout)
{
TMhsThread *thread;

if (!mhs_thread)
  return(0);
thread = *mhs_thread;
if (!thread)
  return(0);
(void)mhs_join_thread(thread, timeout);
mhs_event_destroy((TMhsEvent **)mhs_thread);
return(0);
}


int32_t mhs_join_thread(TMhsThread *mhs_thread, uint32_t timeout)
{
int32_t res;
struct timespec tabs;

res = 0;
if (!mhs_thread)
  return(-1);
if (mhs_thread->Thread == (pthread_t)-1)
  return(-1);
mhs_thread->Run = 0;
mhs_event_set((TMhsEvent *)mhs_thread, MHS_TERMINATE);
if (timeout)
  {
  mhs_calc_abs_timeout(&tabs, timeout);
  if (pthread_timedjoin_np(mhs_thread->Thread, NULL, &tabs))
    {
    if (pthread_cancel(mhs_thread->Thread) != ESRCH)
      (void)pthread_join(mhs_thread->Thread, NULL);
      //(void)pthread_detach(mhs_thread->Thread);
    res = -2;
    }
  }
else
  pthread_join(mhs_thread->Thread, NULL);
mhs_thread->Thread = (pthread_t)-1;
mhs_event_clear((TMhsEvent *)mhs_thread, MHS_ALL_EVENTS);
return(res);
}


int32_t mhs_thread_join_status(TMhsThread *mhs_thread)
{
if (!mhs_thread)
  return(0);
if (mhs_thread->Thread == (pthread_t)-1)
  return(0);
return(1);
}


int32_t mhs_thread_set_priority(TMhsThread *mhs_thread, int32_t priority)
{
struct sched_param sched;
int32_t policy;

if (!mhs_thread)
  return(0);
mhs_thread->Priority = priority;
if (mhs_thread->Thread != (pthread_t)-1)
  {
  switch (mhs_thread->Priority)
    {
    case 0 :   // Normal
    case 1 : {
             priority = SCHED_OTHER;
             break;
             }
    case 2 : {
             priority = SCHED_RR;
             break;
             }
    case 3 : { // Am höchsten
             priority = SCHED_FIFO;
             break;
             }
    default : priority = SCHED_OTHER;
    }
  pthread_getschedparam(mhs_thread->Thread, &policy, &sched);
  sched.sched_priority = sched_get_priority_max(priority);
  pthread_setschedparam(mhs_thread->Thread, policy, &sched);
  }
return(0);
}


void mhs_exit_thread(TMhsThread *mhs_thread)
{
if (!mhs_thread)
  return;
mhs_thread->Run = 0;
mhs_event_set((TMhsEvent *)mhs_thread, MHS_TERMINATE);
}
