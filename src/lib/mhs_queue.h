#ifndef __MHS_QUEUE_H__
#define __MHS_QUEUE_H__

#include "util.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct _TMhsQueueItem TMhsQueueItem;
typedef struct _TMhsQueue TMhsQueue;

typedef void(*TMhsQueueCB)(TMhsQueue *queue, void *user_data);

#pragma pack(push, 1)
struct _TMhsQueueItem
  {
  TMhsQueueItem *Next;
  uint32_t Size;
  uint8_t Data[1];
  };

struct TMhsQueueHeader
  {
  TMhsQueueItem *Next;
  uint32_t Size;
  };
#pragma pack(pop)

struct _TMhsQueue
  {
  TMhsQueueItem *First;
  TMhsQueueItem *Last;
  uint32_t MemLimit;
  uint32_t CurrentMem;
  TMhsQueueCB Proc;
  void *UserData;
#ifdef __WIN32__
// ****** Windows
  CRITICAL_SECTION EventLock;
#else
// ****** Linux
  pthread_mutex_t Mutex;
#endif
  };


#ifdef __WIN32__
// ****** Windows
#define mhs_queue_enter_critical(queue) do {if (queue) EnterCriticalSection(&(queue)->EventLock);} while(0)
#define mhs_queue_leave_critical(queue) do {if (queue) LeaveCriticalSection(&(queue)->EventLock);} while(0)
#else
// ****** Linux
#define mhs_queue_enter_critical(queue) do {if (queue) pthread_mutex_lock(&(queue)->Mutex);} while(0)
#define mhs_queue_leave_critical(queue) do {if (queue) pthread_mutex_unlock(&(queue)->Mutex);} while(0)
#endif

TMhsQueue *mhs_queue_create(uint32_t mem_limit, uint8_t async, TMhsQueueCB proc, void *user_data);
void mhs_queue_destroy(TMhsQueue **queue);
int mhs_queue_push(TMhsQueue *queue, const void *data, uint32_t size);
void *mhs_queue_pop(TMhsQueue *queue);
void mhs_queue_clear(TMhsQueue *queue);

void mhs_queue_data_free(void *data);
uint32_t mhs_queue_get_data_size(void *data);

#ifdef __cplusplus
  }
#endif

#endif
