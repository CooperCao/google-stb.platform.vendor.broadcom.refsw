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
VideoCore OS Abstraction Layer - basic types
=============================================================================*/

#ifndef VCOS_TYPES_H
#define VCOS_TYPES_H

#define VCOS_VERSION   1

#include <assert.h>
#include <stddef.h>
#include "vcos_platform_types.h"
#include "vcos_attr.h"

#ifdef __cplusplus
#include <stdexcept>
#endif

/** Entry function for a lowlevel thread.
  *
  * Returns void for consistency with Nucleus/ThreadX.
  */
typedef void (*VCOS_LLTHREAD_ENTRY_FN_T)(void *);

/** Thread entry point. Returns a void* for consistency
  * with pthreads.
  */
typedef void *(*VCOS_THREAD_ENTRY_FN_T)(void*);

// inline is a keyword in C99 and C++, although MSVC supports a
// good portion of C99, somehow the inline keyword missed the list
#if defined(_MSC_VER) && !defined(inline) && !defined(__cplusplus)
#define inline __inline
#endif

/* Error return codes - chosen to be similar to errno values */
typedef enum
{
   VCOS_SUCCESS,
   VCOS_EAGAIN,
   VCOS_ENOENT,
   VCOS_ENOSPC,
   VCOS_EINVAL,
   VCOS_EACCESS,
   VCOS_ENOMEM,
   VCOS_ENOSYS,
   VCOS_EEXIST,
   VCOS_ENXIO,
   VCOS_EINTR,
   VCOS_ETIMEDOUT
} VCOS_STATUS_T;

static inline const char *vcos_desc_status(VCOS_STATUS_T status)
{
   switch (status)
   {
   case VCOS_SUCCESS:   return "SUCCESS";
   case VCOS_EAGAIN:    return "EAGAIN";
   case VCOS_ENOENT:    return "ENOENT";
   case VCOS_ENOSPC:    return "ENOSPC";
   case VCOS_EINVAL:    return "EINVAL";
   case VCOS_EACCESS:   return "EACCESS";
   case VCOS_ENOMEM:    return "ENOMEM";
   case VCOS_ENOSYS:    return "ENOSYS";
   case VCOS_EEXIST:    return "EEXIST";
   case VCOS_ENXIO:     return "ENXIO";
   case VCOS_EINTR:     return "EINTR";
   case VCOS_ETIMEDOUT: return "ETIMEDOUT";
   default:             assert(0); return NULL;
   }
}

#ifdef __cplusplus

namespace vcos
{

class error : public std::runtime_error
{
public:
   error(VCOS_STATUS_T status) : std::runtime_error(vcos_desc_status(status)) {}
};

// Somewhere, this is included from within an extern "C" block. We have to negate that
// to avoid build errors about throw on Windows.
extern "C++"
{

inline void throw_if_error(VCOS_STATUS_T status)
{
   if (status != VCOS_SUCCESS)
      throw error(status);
}

} // extern "C++"

}

#endif

#if defined(__HIGHC__) || defined(__HIGHC_ANSI__) || defined(__VECTORC__)
#define _VCOS_METAWARE
#endif

/** It seems that __FUNCTION__ isn't standard!
  */
#if !defined(__STDC__) || (__STDC_VERSION__ < 199901L)
# if (defined(__GNUC__) && (__GNUC__ >= 2)) || defined(__VIDEOCORE__) || defined(_MSC_VER)
#  define VCOS_FUNCTION __FUNCTION__
# else
#  define VCOS_FUNCTION "<unknown>"
# endif
#else
# define VCOS_FUNCTION __func__
#endif

/*
 * Branch prediction hint for compiler
 */
#ifdef 	CC_UNLIKELY
#define VCOS_UNLIKELY CC_UNLIKELY
#else
#ifdef G_UNLIKELY
#define VCOS_UNLIKELY G_UNLIKELY
#endif
#endif

#ifndef VCOS_UNLIKELY
#define VCOS_UNLIKELY(a) (a)
#endif

#define _VCOS_MS_PER_TICK (1000/VCOS_TICKS_PER_SECOND)
#define _VCOS_US_PER_TICK (1000000/VCOS_TICKS_PER_SECOND)

/* Convert a number of milliseconds to a tick count. Internal use only - fails to
 * convert VCOS_SUSPEND correctly.
 */
#define _VCOS_MS_TO_TICKS(ms) (((ms)+_VCOS_MS_PER_TICK-1)/_VCOS_MS_PER_TICK)

#define VCOS_TICKS_TO_MS(ticks) ((ticks) * _VCOS_MS_PER_TICK)

/* Convert a number of microseconds to a tick count. Internal use only - fails to
 * convert VCOS_SUSPEND correctly.
 */
#define _VCOS_US_TO_TICKS(us) (((us)+_VCOS_US_PER_TICK-1)/_VCOS_US_PER_TICK)

#define VCOS_TICKS_TO_US(ticks) ((ticks) * _VCOS_US_PER_TICK)

/** VCOS version of DATESTR, from pcdisk.h. Used by the hostreq service.
 */
typedef struct vcos_datestr
{
   uint8_t       cmsec;              /**< Centesimal mili second */
   uint16_t      date;               /**< Date */
   uint16_t      time;               /**< Time */

} VCOS_DATESTR;

#define vcos_min(x,y) ((x) < (y) ? (x) : (y))
#define vcos_max(x,y) ((x) > (y) ? (x) : (y))

/** Return the count of an array. FIXME: under gcc we could make
 * this report an error for pointers using __builtin_types_compatible().
 */
#define vcos_countof(x) (sizeof((x)) / sizeof((x)[0]))

/* for backward compatibility */
#define countof(x) (sizeof((x)) / sizeof((x)[0]))

#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))

#ifdef _MSC_VER
   #define vcos_alignof(T) __alignof(T)
#elif defined(__GNUC__)
   #define vcos_alignof(T) __alignof__(T)
#else
   #define vcos_alignof(T) (sizeof(struct { T t; char ch; }) - sizeof(T))
#endif

/** bool_t is not a POSIX type so cannot rely on it. Define it here.
  * It's not even defined in stdbool.h.
  */
typedef int32_t vcos_bool_t;
typedef int32_t vcos_fourcc_t;

#define VCOS_FALSE   0
#define VCOS_TRUE    (!VCOS_FALSE)

/** Mark unused arguments/locals to keep compilers quiet */
#define vcos_unused(x) (void)(x)
#ifdef NDEBUG
#define vcos_unused_in_release(x) vcos_unused(x)
#else
#define vcos_unused_in_release(x)
#endif

#ifdef NDEBUG
#define IS_DEBUG 0
#else
#define IS_DEBUG 1
#endif

#ifdef NDEBUG
#define debug_only(x)
#else
#define debug_only(x) x
#endif

/** For backward compatibility */
typedef vcos_fourcc_t fourcc_t;
typedef vcos_fourcc_t FOURCC_T;

/**
 * Construct a fourcc value.
 * VCOS_FOURCC('a','b','c','d') will correspond to the bytes "abcd"
 * on little-endian platforms.
 */
#define VCOS_FOURCC(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#ifdef __cplusplus
#define VCOS_EXTERN_C_BEGIN extern "C" {
#define VCOS_EXTERN_C_END }
#else
#define VCOS_EXTERN_C_BEGIN
#define VCOS_EXTERN_C_END
#endif

#ifdef _MSC_VER
#define PRIuSIZE "Iu"
#define PRIxSIZE "Ix"
#else
#define PRIuSIZE "zu"
#define PRIxSIZE "zx"
#endif

#endif
