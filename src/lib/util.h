#ifndef __UTIL_H__
#define __UTIL_H__

//#include <glib.h>
//#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define GetByte(x) *((uint8_t *)(x))
#define GetWord(x) *((uint16_t *)(x))
#define GetLong(x) *((uint32_t *)(x))

#define SetByte(x, v) *((uint8_t *)x) = (v)
#define SetWord(x, v) *((uint16_t *)x) = (v)
#define SetLong(x, v) *((uint32_t *)x) = (v)

#define lo(x)  (unsigned char)(x & 0xFF)
#define hi(x)  (unsigned char)((x >> 8) & 0xFF)

#define l_lo(x) (unsigned char)(x)
#define l_m1(x) (unsigned char)((x) >> 8)
#define l_m2(x) (unsigned char)((x) >> 16)
#define l_hi(x) (unsigned char)((x) >> 24)

#define safe_free(d) do { \
  if ((d)) \
    { \
    free((d)); \
    (d) = NULL; \
    } \
  } while(0)


#define mhs_malloc  malloc
#define mhs_calloc  calloc
#define mhs_malloc0(s)  calloc(1,s)
#define mhs_free    free

#define mhs_strdup g_strdup

#ifdef __WIN32__
// ****** Windows
#include <windows.h>
#define CALLBACK_TYPE CALLBACK
#define mhs_sleep(x) Sleep(x)
#define get_tick() GetTickCount()
#else
// ****** Linux>
#include <unistd.h>
#define CALLBACK_TYPE
#define mhs_sleep(x) usleep((x) * 1000)
uint32_t get_tick(void);
#endif

uint32_t mhs_diff_time(uint32_t now, uint32_t stamp);
char *get_item_as_string(char **str, char *trenner, int *result);

#ifdef __cplusplus
  }
#endif

#endif
