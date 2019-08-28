/*******************************************************************************
                          sock_lib.c  -  description
                             ------------------
    begin             : 02.08.2019
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
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <glib.h>
#include "util.h"
#include "mhs_event.h"  // mhs_calc_abs_timeout
#include "mhs_file_event.h"
#include "sock_lib.h"

#define NB_CONNECTION  16

#define max(a, b) ((a)>(b)) ? (a) : (b)


#define SockLibLock(l) g_mutex_lock(&(l)->Mutex)
#define SockLibUnlock(l) g_mutex_unlock(&(l)->Mutex)


/***************************************************************/
/* time in ms                                                  */
/***************************************************************/
/*static void mhs_calc_abs_timeout(struct timespec *timeout, uint32_t time)
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
  timeout->tv_sec++; // carry bit stored in tv_sec
  timeout->tv_nsec = (now.tv_nsec + rem) - 1000000000;
  }
else
  timeout->tv_nsec = now.tv_nsec + rem;
} */


/***************************************************************************/
/*                       Ein Object löschen                                */
/***************************************************************************/
TSockLibClient *CreateClient(TSockLib *sock_lib)
{
TSockLibClient *list, *new;

if (!(new = (TSockLibClient *)g_malloc0(sizeof(TSockLibClient))))
  return(NULL);
new->Fd = -1;
list = sock_lib->Clients;
if (!list)
  sock_lib->Clients = new;
else
  {
  while (list->Next)
    list = list->Next;
  list->Next = new;
  }
return(new);
}


static void DestroyClient(TSockLib *sock_lib, TSockLibClient *client)
{
TSockLibClient *list, *prev;

prev = NULL;
// Liste nach "obj" durchsuchen
for (list = sock_lib->Clients; list; list = list->Next)
  {
  if (list == client)
    {
    if (prev)
      prev->Next = list->Next;
    else
      sock_lib->Clients = list->Next;
    g_free(list);
    break;
    }
  prev = list;
  }
}


TSockLibClient *GetClientByFd(TSockLib *sock_lib, int fd)
{
TSockLibClient *list;

for (list = sock_lib->Clients; list; list = list->Next)
  {
  if (list->Fd == fd)
    return(list);
  }
return(NULL);
}


static int OpenNewConn(TSockLib *sock_lib)
{
socklen_t addrlen;
TSockLibClient *client;
int res, option;

if (!(client = CreateClient(sock_lib)))
  return(-1);
res = 0;
//addrlen = sizeof(struct sockaddr_in); <*>
addrlen = sizeof(struct sockaddr_storage);
client->Fd = accept(sock_lib->ServerSocket, (struct sockaddr *)&client->ClientAddr, &addrlen);
if (client->Fd == -1)
  res = -1;
else
  {
  // Set the TCP no delay flag
  option = 1;
  res = setsockopt(client->Fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&option, sizeof(int));
  if (res >= 0)
    (sock_lib->NewClientCB)(sock_lib, client, sock_lib->UserData);
  }
if (res < 0)
  DestroyClient(sock_lib, client);
return(res);
}


/***********************************************************/
/*              S O C K E T - T H R E A D                  */
/***********************************************************/
static void *thread_execute(void *data)
{
struct timeval timeout;
uint8_t event;
int rc, fd, event_fd, maxfd;
TSockLib *sock_lib;
TSockLibClient *client;

// **** Thread Initialisieren
(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
(void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

sock_lib = (TSockLib *)data;
event_fd = file_event_get_fd(sock_lib->FileEvent);

while (sock_lib->Run)
  {
  // **** fd Array initialisieren
  FD_ZERO(&sock_lib->RxFdSet);
  FD_ZERO(&sock_lib->ErrFdSet);
  // **** fd Array setzen
  FD_SET(event_fd, &sock_lib->RxFdSet); // Event Pipe hinzufügen
  FD_SET(event_fd, &sock_lib->ErrFdSet); // Event Pipe hinzufügen
  FD_SET(sock_lib->ServerSocket, &sock_lib->RxFdSet); // Den Socket der verbindungen annimmt hinzufügen
  FD_SET(sock_lib->ServerSocket, &sock_lib->ErrFdSet); // Den Socket der verbindungen annimmt hinzufügen
  maxfd = max(event_fd, sock_lib->ServerSocket);
  //SockLibLock(sock_lib);<*>
  for (client = sock_lib->Clients; client; client = client->Next)
    {
    if ((fd = client->Fd) < 0)
      continue;
    maxfd = max(maxfd, fd);
    FD_SET(fd,&sock_lib->RxFdSet);
    FD_SET(fd,&sock_lib->ErrFdSet);
    }
  //SockLibUnlock(sock_lib); <*>
  // **** Select aufrufen
  if ((sock_lib->SockTimeout.tv_sec) || (sock_lib->SockTimeout.tv_usec))
    {
    timeout.tv_sec = sock_lib->SockTimeout.tv_sec;
    timeout.tv_usec = sock_lib->SockTimeout.tv_usec;
    rc = select(maxfd + 1, &sock_lib->RxFdSet, NULL, &sock_lib->ErrFdSet, &timeout);
    }
  else
    rc = select(maxfd + 1, &sock_lib->RxFdSet, NULL, &sock_lib->ErrFdSet, NULL);
  if (rc == -1)
    {
    if (errno == EINTR)
      continue;
    else
      break;
    }
  if (!sock_lib->Run)
    break;
  if (rc == 0)  // Timeout
    {
    (sock_lib->EventCB)(sock_lib, SOCK_LIB_TIMEOUT, NULL, NULL, 0, sock_lib->UserData);
    continue;
    }

  if (FD_ISSET(event_fd, &sock_lib->RxFdSet))
    {
    if ((event = get_file_event(sock_lib->FileEvent)))
      (sock_lib->EventCB)(sock_lib, SOCK_LIB_EVENT + event, 0, NULL, 0, sock_lib->UserData);
    }

  // **** Neue Verbindung ?
  if (FD_ISSET(sock_lib->ServerSocket, &sock_lib->RxFdSet))
    (void)OpenNewConn(sock_lib);

  for (client = sock_lib->Clients; client; client = client->Next)
    {
    if ((fd = client->Fd) < 0)
      continue;
    /*if (FD_ISSET(fd, &sock_lib->ErrFdSet))
      {
      (sock_lib->EventCB)(sock_lib, SOCK_LIB_CLOSE, client, NULL, 0, sock_lib->UserData);
      }
    else*/ if (FD_ISSET(fd, &sock_lib->RxFdSet))
      {
      do
        rc = recv(fd, sock_lib->SockRxDPuffer, SOCK_RXD_PUFFER_SIZE-1, MSG_DONTWAIT);
      while ((rc == -1) && (errno == EINTR));
      if (rc < 0)
        continue;
      if (rc == 0)
        {
        close(client->Fd); // <*> notwendig ?
        (sock_lib->EventCB)(sock_lib, SOCK_LIB_CLOSE, client, NULL, 0, sock_lib->UserData);
        DestroyClient(sock_lib, client);
        }
      else
        {
        sock_lib->SockRxDPuffer[rc]='\0';
        (sock_lib->EventCB)(sock_lib, SOCK_LIB_READ, client, sock_lib->SockRxDPuffer, rc, sock_lib->UserData);
        }
      }
    }
  }
return(NULL);
}


/*
******************** SockWrite ********************
*/
int32_t SockLibWrite(TSockLibClient *client, const char *data, int32_t size)
{
return(send(client->Fd, data, size, MSG_NOSIGNAL));
}


/*
******************** SockWriteAll ********************
*/
int32_t SockLibWriteAll(TSockLib *sock_lib, const char *data, int32_t size)
{
int32_t err;
TSockLibClient *client;

err = 0;
for (client = sock_lib->Clients; client; client = client->Next)
  {
  if (send(client->Fd, data, size, MSG_NOSIGNAL) < 0)
    err = -1;
  }
return(err);
}


/*
******************** SockMakeEvent ********************
*/
void SockMakeEvent(TSockLib *sock_lib, uint8_t event)
{
(void)set_file_event(sock_lib->FileEvent, event);
}


TSockLib *SockLibCreate(const char *ip, int port, uint32_t timeout,
   TSockLibNewClientCB new_client_cb, TSockLibEventCB event_cb, gpointer user_data)
{
gint res;
TSockLib *sock_lib;
struct sockaddr_in addr;
int flags;

if ((!new_client_cb) || (!event_cb))
  return(NULL);
if (!(sock_lib = (TSockLib *)g_malloc0(sizeof(TSockLib))))
  return(NULL);
sock_lib->NewClientCB = new_client_cb;
sock_lib->EventCB = event_cb;
sock_lib->UserData = user_data;
g_mutex_init(&sock_lib->Mutex);
res = 0;
sock_lib->ServerSocket = -1;
sock_lib->Thread = (pthread_t)-1;
// **** Timeout setzen
if (timeout)
  {
  sock_lib->SockTimeout.tv_sec = timeout / 1000;
  sock_lib->SockTimeout.tv_usec = (timeout % 1000) * 1000;
  }
else
  {
  sock_lib->SockTimeout.tv_sec = 0L;
  sock_lib->SockTimeout.tv_usec = 0L;
  }

flags = SOCK_STREAM;
#ifdef SOCK_CLOEXEC // <*> noch prüfen
    flags |= SOCK_CLOEXEC;
#endif
if ((sock_lib->ServerSocket = socket(PF_INET, flags, IPPROTO_TCP)) == -1)
  res = -1;
else
  {
  flags = 1;
  if (setsockopt(sock_lib->ServerSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags)) == -1)
    res = -1;
  }
if (!res)
  {
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (!ip)
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen any addresses
  else
    addr.sin_addr.s_addr = inet_addr(ip); // Listen only specified IP address
  if (bind(sock_lib->ServerSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    res = -1;
  else
    {
    if (listen(sock_lib->ServerSocket, NB_CONNECTION) == -1)
      res = -1;
    }
  }
if (!res)
  {
  if (!(sock_lib->FileEvent = create_file_event()))
    res = -1;
  }
if (!res)
  {
  sock_lib->Run = 1;
  if (pthread_create(&sock_lib->Thread, NULL, thread_execute, (void *)sock_lib))
    {
    sock_lib->Thread = (pthread_t)-1;
    sock_lib->Run = 0;
    res = -1;
    }
  }
if (res)
  SockLibDestroy(&sock_lib);
return(sock_lib);
}


void SockLibDestroy(TSockLib **sock_lib)
{
struct timespec tabs;
TSockLibClient *client, *next;
TSockLib *lib;

if (!sock_lib)
  return;
if (!(lib = *sock_lib))
  return;
if (lib->Thread != -1)
  {
  lib->Run = 0;
  (void)set_file_event(lib->FileEvent, 0xFF);
  mhs_calc_abs_timeout(&tabs, 1000);
  if (pthread_timedjoin_np(lib->Thread, NULL, &tabs))
    {
    if (pthread_cancel(lib->Thread) != ESRCH)
      (void)pthread_join(lib->Thread, NULL);
    }
  destroy_file_event(&lib->FileEvent);
  lib->Thread = (pthread_t)-1;
  }
for (client = lib->Clients; client; client = client->Next)
  {
  shutdown(client->Fd, SHUT_RDWR);
  close(client->Fd);
  }

client = lib->Clients;
while (client)
  {
  next = client->Next;
  g_free(client);
  client = next;
  }
g_mutex_clear(&lib->Mutex);
if (lib->ServerSocket != -1)
  {
  shutdown(lib->ServerSocket, SHUT_RDWR);
  close(lib->ServerSocket);
  }
safe_free(lib);

*sock_lib = NULL;
}



