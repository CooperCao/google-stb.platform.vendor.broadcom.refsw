/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - public header file
=============================================================================*/

#ifndef VCOS_STRING_H
#define VCOS_STRING_H

#if __VECTORC__
#undef _GNU_SOURCE
#endif

/**
  * \file
  *
  * String functions.
  *
  */

#include "vcos_types.h"
#include "vcos_platform.h"

#include <string.h>
#include <stdarg.h>
#ifdef _GNU_SOURCE
#include <stdio.h> /* For asprintf/vasprintf */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Case insensitive string comparison.
  *
  */

static inline int vcos_strcasecmp(const char *s1, const char *s2);

static inline int vcos_strncasecmp(const char *s1, const char *s2, size_t n);

/** Just like http://linux.die.net/man/3/asprintf */
#ifdef _GNU_SOURCE
#define vcos_vasprintf vasprintf
#define vcos_asprintf asprintf
#else
int vcos_vasprintf(char **strp, const char *fmt, va_list ap);
int vcos_asprintf(char **strp, const char *fmt, ...) VCOS_FORMAT_ATTR_(printf, 2, 3);
#endif

/** Like vsnprintf, except it places the output at the specified offset.
  * Output is truncated to fit in buflen bytes, and is guaranteed to be NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_vsprintf(char *buf, size_t buflen, size_t offset, const char *fmt, va_list ap);

#define VCOS_SAFE_VSPRINTF(buf, offset, fmt, ap) \
   vcos_safe_vsprintf(buf, sizeof(buf) + ((char (*)[sizeof(buf)])buf - &(buf)), offset, fmt, ap)

/** Like snprintf, except it places the output at the specified offset.
  * Output is truncated to fit in buflen bytes, and is guaranteed to be NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_sprintf(char *buf, size_t buflen, size_t offset, const char *fmt, ...) VCOS_FORMAT_ATTR_(printf, 4, 5);

/* The Metaware compiler currently has a bug in its variadic macro handling which
   causes it to append a spurious command to the end of its __VA_ARGS__ data.
   Do not use until this has been fixed (and this comment has been deleted). */

#define VCOS_SAFE_SPRINTF(buf, offset, ...) \
   vcos_safe_sprintf(buf, sizeof(buf) + ((char (*)[sizeof(buf)])buf - &(buf)), offset, __VA_ARGS__)

/** Copies string src to dst at the specified offset.
  * Output is truncated to fit in dstlen bytes, i.e. the string is at most
  * (buflen - 1) characters long. Unlike strncpy, exactly one NUL is written
  * to dst, which is always NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_strcpy(char *dst, const char *src, size_t dstlen, size_t offset);

#define VCOS_SAFE_STRCPY(dst, src, offset) \
   vcos_safe_strcpy(dst, src, sizeof(dst) + ((char (*)[sizeof(dst)])dst - &(dst)), offset)

/* Use like:
 * VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(str, 1024, vcos_safe_sprintf, "%d %f", 1, 2.0);
 * printf("%s", str); */
#define VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, BUF_SIZE, FUNC, ...)      \
   char BUF_NAME[BUF_SIZE];                                                \
   do                                                                      \
   {                                                                       \
      size_t offset_ = (FUNC)(BUF_NAME, sizeof(BUF_NAME), 0, __VA_ARGS__); \
      assert(offset_ < sizeof(BUF_NAME)); /* Or we truncated it */    \
   } while (0)

#ifdef __cplusplus
}
#endif
#endif
