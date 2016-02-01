/*
 * Copyright (c) 2010-2011 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

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
