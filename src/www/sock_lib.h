#ifndef __SOCK_LIB_H__
#define __SOCK_LIB_H__

#include <stdint.h>
#include <netinet/tcp.h>
#include <glib.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define SOCK_RXD_PUFFER_SIZE 32768

#define SOCK_LIB_TIMEOUT  0x0000
#define SOCK_LIB_EVENT    0x1000
#define SOCK_LIB_CLOSE    0x2000
#define SOCK_LIB_READ     0x4000

typedef struct _TSockLibClient TSockLibClient;
typedef struct _TSockLib TSockLib;

typedef int32_t(*TSockLibNewClientCB)(TSockLib *sock_lib, TSockLibClient *client, gpointer user_data);
typedef int32_t(*TSockLibEventCB)(TSockLib *sock_lib, uint32_t event, TSockLibClient *client, char *data, int32_t size, gpointer user_data);


struct _TSockLibClient
  {
  TSockLibClient *Next;
  gpointer UserData;
  //struct sockaddr_in ClientAddr; <*>
  struct sockaddr_storage ClientAddr;
  int Fd;
  };


struct _TSockLib
  {
  struct timeval SockTimeout;
  TSockLibNewClientCB NewClientCB;
  TSockLibEventCB EventCB;
  gpointer UserData;
  char SockRxDPuffer[SOCK_RXD_PUFFER_SIZE];
  int ServerSocket;
  fd_set RxFdSet;           // Inhalt leeren
  fd_set ErrFdSet;
  int FdMax;
  pthread_t Thread;
  GMutex Mutex;
  volatile int32_t Run;
  struct TFileEvent *FileEvent;
  TSockLibClient *Clients;
  };

int32_t SockLibWrite(TSockLibClient *client, const char *data, int32_t size);
int32_t SockLibWriteAll(TSockLib *sock_lib, const char *data, int32_t size);
void SockMakeEvent(TSockLib *sock_lib, uint8_t event);
TSockLib *SockLibCreate(const char *ip, int port, uint32_t timeout, 
   TSockLibNewClientCB new_client_cb, TSockLibEventCB event_cb, gpointer user_data);
void SockLibDestroy(TSockLib **sock_lib);

#ifdef __cplusplus
  }
#endif

#endif
