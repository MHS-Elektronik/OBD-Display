#ifndef __MHS_EVENT_H__
#define __MHS_EVENT_H__

#include <stdint.h>
#include "can_drv.h"

#ifdef __cplusplus
  extern "C" {
#endif

#define MHS_TERMINATE 0x80000000

#define MHS_ALL_EVENTS 0xFFFFFFFF


#define MHS_EVENT_STATUS_INVALID         0x00000000
#define MHS_EVENT_STATUS_INIT            0x00000100
#define MHS_EVENT_STATUS_TERMINATE       0x00000200

#define MHS_EVENT_STATUS_PEND            0x00000010
#define MHS_EVENT_STATUS_TERM_PEND       0x00000020

#define MHS_EVENT_STATUS_PEND_TERM_PEND  0x00000030

#define MHS_EVENT_STATUS_PEND_MASK       0x000000F0
#define MHS_EVENT_STATUS_PEND_CLEAR      0xFFFFFF0F

/*
typedef struct _TMhsEvent TMhsEvent;

#pragma pack(push, 1)
struct _TMhsEvent
  {
  volatile uint32_t Events;
  volatile uint32_t EventsMask;
  volatile int32_t Waiting;
#ifdef __WIN32__
// ****** Windows
  uint32_t WinEventCount;
  HANDLE WinEvent[3];
  CRITICAL_SECTION EventLock;
#else
// ****** Linux
  pthread_cond_t Cond;
  pthread_mutex_t Mutex;
#endif
  };
#pragma pack(pop)
*/

#ifdef __WIN32__
// ****** Windows
#define mhs_enter_critical(event) do {if (event) EnterCriticalSection(&((TMhsEvent *)event)->EventLock);} while(0)
#define mhs_leave_critical(event) do {if (event) LeaveCriticalSection(&((TMhsEvent *)event)->EventLock);} while(0)
#else
// ****** Linux
#define mhs_enter_critical(event) do {if (event) pthread_mutex_lock(&((TMhsEvent *)event)->Mutex);} while(0)
#define mhs_leave_critical(event) do {if (event) pthread_mutex_unlock(&((TMhsEvent *)event)->Mutex);} while(0)
#endif

void mhs_calc_abs_timeout(struct timespec *timeout, uint32_t time);

TMhsEvent *mhs_event_create(void);
TMhsEvent *mhs_event_create_ex(int32_t struct_size);
void mhs_event_destroy(TMhsEvent **mhs_event);
uint32_t mhs_event_get_event_mask(TMhsEvent *event);
void mhs_event_set_event_mask(TMhsEvent *event, uint32_t mask);
void mhs_event_set_unlocked(TMhsEvent *event, uint32_t events);
void mhs_event_set(TMhsEvent *event, uint32_t events) ;
void mhs_event_clear_unlocked(TMhsEvent *event, uint32_t events);
void mhs_event_clear(TMhsEvent *event, uint32_t events);
uint32_t mhs_event_get_unlocked(TMhsEvent *event, int32_t clear);
uint32_t mhs_event_get(TMhsEvent *event, int32_t clear);
uint32_t mhs_wait_for_event(TMhsEvent *event, uint32_t timeout);
int32_t mhs_wait_status(TMhsEvent *event);
uint32_t mhs_sleep_ex(TMhsEvent *event, uint32_t time);

#ifdef __cplusplus
  }
#endif

#endif
