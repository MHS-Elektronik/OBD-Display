#ifndef __MHS_THREAD_H__
#define __MHS_THREAD_H__

#include "util.h"
#include "mhs_event.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define MHS_THREAD_PRIORITY_NORMAL         0
#define MHS_THREAD_PRIORITY_ABOVE_NORMAL   1
#define MHS_THREAD_PRIORITY_HIGHEST        2
#define MHS_THREAD_PRIORITY_TIME_CRITICAL  3
#define MHS_THREAD_PRIORITY_REALTIME       4

typedef struct _TMhsThread TMhsThread;

typedef void (*MhsThreadFunc)(TMhsThread *);

struct _TMhsThread
  {
  TMhsEvent Event;
#ifdef __WIN32__
// ****** Windows
  HANDLE Thread;
#else
// ****** Linux
  pthread_t Thread;
#endif
  volatile int32_t Run;
  int32_t Priority;
  MhsThreadFunc Func;
  void *Data;
  };



TMhsThread *mhs_create_thread(MhsThreadFunc func, void *data, int32_t priority, int32_t run);
int32_t mhs_destroy_thread(TMhsThread **mhs_thread, uint32_t timeout);

int32_t mhs_run_thread(TMhsThread *mhs_thread);
int32_t mhs_run_thread_ex(TMhsThread *mhs_thread, void *data);
int32_t mhs_join_thread(TMhsThread *mhs_thread, uint32_t timeout);
int32_t mhs_thread_join_status(TMhsThread *mhs_thread);
int32_t mhs_thread_set_priority(TMhsThread *mhs_thread, int32_t priority);

void mhs_exit_thread(TMhsThread *mhs_thread);

#ifdef __cplusplus
  }
#endif

#endif
