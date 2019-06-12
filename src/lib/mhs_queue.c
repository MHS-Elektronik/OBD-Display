/*******************************************************************************
                          mhs_queue.c  -  description
                             -------------------
    begin             : 25.01.2019
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
#include <string.h>
#ifdef __WIN32__
// ****** Windows
#else
// ****** Linux
  #include <pthread.h>
#endif
#include "util.h"
#include "mhs_queue.h"



TMhsQueue *mhs_queue_create(uint32_t mem_limit, uint8_t async, TMhsQueueCB proc, void *user_data)
{
TMhsQueue *queue;

if (!(queue = (TMhsQueue *)mhs_malloc0(sizeof(TMhsQueue))))
  return(NULL);
if (async)
  {
#ifdef __WIN32__
// ****** Windows
  InitializeCriticalSection(&queue->EventLock);
#else
// ****** Linux
  if ((pthread_mutex_init(&queue->Mutex, NULL)))
    {
    mhs_free(queue);
    return(NULL);
    }
#endif
  }
queue->MemLimit = mem_limit;
queue->Proc = proc;
queue->UserData = user_data;
return(queue);
}


void mhs_queue_destroy(TMhsQueue **queue)
{
TMhsQueue *q;

if (!queue)
  return;
if (!(q = *queue))
  return;
#ifdef __WIN32__
// ****** Windows
DeleteCriticalSection(&q->EventLock);
#else
// ****** Linux
pthread_mutex_destroy(&q->Mutex);
#endif
mhs_free(q);
*queue = NULL;
}


int mhs_queue_push(TMhsQueue *queue, const void *data, uint32_t size)
{
int res;
TMhsQueueItem *item;

res = 0;
if (!queue)
  return(-1);
mhs_queue_enter_critical(queue);
if (queue->MemLimit)
  {
  if ((queue->CurrentMem + size) > queue->MemLimit)
    res = -1;
  }
if (!res)
  {
  if (!(item = mhs_malloc(sizeof(TMhsQueueItem) + size)))
    res = -1;
  else
    {
    item->Next = NULL;
    memcpy(item->Data, data, size);
    item->Size = size;
    queue->CurrentMem += size;
    if (!queue->First)
      {
      queue->First = item;
      queue->Last = item;
      }
    else
      {
      queue->Last->Next = item;
      queue->Last = item;
      }
    }
  }
mhs_queue_leave_critical(queue);
if ((!res) && (queue->Proc))
  (queue->Proc)(queue, queue->UserData);
return(res);
}


void *mhs_queue_pop(TMhsQueue *queue)
{
TMhsQueueItem *item;

if (!queue)
  return(NULL);
if (!queue->First)
  return(NULL);
mhs_queue_enter_critical(queue);
if ((item = queue->First))
  {
  if (queue->CurrentMem >= item->Size)
    queue->CurrentMem -= item->Size;
  if (queue->First == queue->Last)
    queue->First = NULL;
  else
    queue->First = item->Next;
  }
mhs_queue_leave_critical(queue);
return(item->Data);
}


void mhs_queue_clear(TMhsQueue *queue)
{
void *data;

while ((data = mhs_queue_pop(queue)))
  mhs_queue_data_free(data);
}


void mhs_queue_data_free(void *data)
{
struct TMhsQueueHeader *header;

if (data)
  {
  header = (struct TMhsQueueHeader *)data;
  header--;
  free(header);
  }
}


uint32_t mhs_queue_get_data_size(void *data)
{
struct TMhsQueueHeader *header;

if (data)
  {
  header = (struct TMhsQueueHeader *)data;
  header--;
  return(header->Size);
  }
return(0);
}
