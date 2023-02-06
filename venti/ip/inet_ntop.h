/**
 * @file inet_ntop.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022
 *
 */

// From MUSL Project

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <cstdint>
#include <cstdio>

#define _AF_INET 2
#define _AF_INET6 10

namespace mocoder::venti {

inline const char *inet_ntop(int af, const void * a0, char * s,
                      int l) {
  const unsigned char *a = (const unsigned char*)a0;
  int i, j, max, best;
  char buf[100];

  switch (af) {
    case _AF_INET:
      if (snprintf(s, l, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]) < l) return s;
      break;
    case _AF_INET6:
      if (memcmp(a, "\0\0\0\0\0\0\0\0\0\0\377\377", 12))
        snprintf(buf, sizeof buf, "%x:%x:%x:%x:%x:%x:%x:%x", 256 * a[0] + a[1],
                 256 * a[2] + a[3], 256 * a[4] + a[5], 256 * a[6] + a[7],
                 256 * a[8] + a[9], 256 * a[10] + a[11], 256 * a[12] + a[13],
                 256 * a[14] + a[15]);
      else
        snprintf(buf, sizeof buf, "%x:%x:%x:%x:%x:%x:%d.%d.%d.%d",
                 256 * a[0] + a[1], 256 * a[2] + a[3], 256 * a[4] + a[5],
                 256 * a[6] + a[7], 256 * a[8] + a[9], 256 * a[10] + a[11],
                 a[12], a[13], a[14], a[15]);
      /* Replace longest /(^0|:)[:0]{2,}/ with "::" */
      for (i = best = 0, max = 2; buf[i]; i++) {
        if (i && buf[i] != ':') continue;
        j = strspn(buf + i, ":0");
        if (j > max) best = i, max = j;
      }
      if (max > 2) {
        buf[best] = buf[best + 1] = ':';
        memmove(buf + best + 2, buf + best + max, i - best - max + 1);
      }
      if (strlen(buf) < l) {
        strcpy(s, buf);
        return s;
      }
      break;
    default:
      errno = EAFNOSUPPORT;
      return 0;
  }
  errno = ENOSPC;
  return 0;
}

}  // namespace mocoder::venti