/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef _C_UTILS_INTERNAL_H_
#define _C_UTILS_INTERNAL_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#ifndef ASMCPP

#include "fp_sdk_config.h"


/**
 * Useful macros that we don't want to redistribute.
 */



#define STRUCT_MEMBER_SIZE(struct_type, member)     sizeof(((struct_type *)0)->member)
#define STRUCT_MEMBER_TYPE(struct_type, member)     typeof(((struct_type *)0)->member)


/* Compiler version as a number */
#define __GCC_VERSION__(version, minor, patchlevel)  ((version    & 0xff) << 16 | \
                                                      (minor      & 0xff) <<  8 | \
                                                      (patchlevel & 0xff))
#define __FP_GCC_VERSION__  __GCC_VERSION__(__FP_GCC__, __FP_GCC_MINOR__, __FP_GCC_PATCHLEVEL__)
#define __GNUC_VERSION__    __GCC_VERSION__(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)


/* Arithmetic */
/** Returns true if x (nonnegative) is a power of 2; 0 is considered power of 2. */
#define IS_POWER_OF_2(x)                    (((x) & ((x) - 1)) == 0)
/** Returns true if x is multiple of y. Requires x >= 0, y > 0. */
#define IS_MULTIPLE_OF(x, y)                (((x) % (y)) == 0)
/** Returns true if x is multiple of y. Requires x >= 0, y > 0 and a power of 2. */
#define IS_MULTIPLE_OF_POW2(x, y)           (((x) & ((y) - 1)) == 0)
/** Rounds down x to the nearest multiple of y. Requires x >= 0, y > 0. */
#define FLOOR_TO_MULTIPLE_OF(x, y)          ((x) - ((x) % (y)))
/** Rounds down x to the nearest multiple of y. Requires x >= 0, y > 0 and a power of 2. */
#define FLOOR_TO_MULTIPLE_OF_POW2(x, y)     ((x) & ~((y) - 1))
/** Rounds up x to the nearest multiple of y. Requires x >= 0, y > 0. */
#define CEIL_TO_MULTIPLE_OF(x, y)           (((x) + ((y) - 1)) - (((x) + ((y) - 1)) % (y)))
/** Rounds up x to the nearest multiple of y. Requires x >= 0, y > 0 and a power of 2.
 *  Beware that x=0 is a multiple of any y. */
#define CEIL_TO_MULTIPLE_OF_POW2(x, y)      (((x) + ((y) - 1)) & ~((y) - 1))
/** Divides and rounds down the result to the nearest integer value. */
#define FLOOR_DIV(n, d)                     ((n) / (d))
/** Divides and rounds up the result to the nearest integer value.
 *  Requires that (n + d - 1) doesn't overflow the used data type. */
#define CEIL_DIV(n, d)                      (((n) + (d) - 1) / (d))
/** Calculates n % d. Requires x >= 0, y > 0 and a power of 2. */
#define MOD_POW2(n, d)                      ((n) & ((d) - 1))
/** Gets the min or max of two values.
 *  Unsafe as it doesn't cope well with arguments with side effects. */
#define UNSAFE_MIN(a, b)                    ((a) < (b) ? (a) : (b))
#define UNSAFE_MAX(a, b)                    ((a) > (b) ? (a) : (b))


/* Bit twiddling */
/** Create a bit mask with only bit n set */
#define BIT(n)                              (1 << (n))
/** Create a bit mask with bits from b(ottom) to t(op) (inclusive) set */
#define BITMASK(t, b)                       (BIT(1 + (t)) - BIT(b))


/* Measurement units */
#define KHz(hz)     (1000 *               (hz))
#define MHz(hz)     (1000 * 1000 *        (hz))
#define GHz(hz)     (1000 * 1000 * 1000 * (hz))
#define KB(bytes)   (1000 *               (bytes))
#define MB(bytes)   (1000 * 1000 *        (bytes))
#define GB(bytes)   (1000 * 1000 * 1000 * (bytes))
#define KiB(bytes)  (1024 *               (bytes))
#define MiB(bytes)  (1024 * 1024 *        (bytes))
#define GiB(bytes)  (1024 * 1024 * 1024 * (bytes))


#if !defined(__LINKER_SCRIPT__)     /* limit typed bitmasks to C sources */

/* Raaga Magnum is allergic to limits.h (sigh...) so we need a bit of acrobatics here */
#  if defined(__FIREPATH__)
#    include <limits.h>
#  else
#    include "libdspcontrol/CHIP.h"
#    if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#      ifdef __CHAR_BIT__
#        define CHAR_BIT    __CHAR_BIT__
#      else
#        define CHAR_BIT    8
#      endif
#    else
#      include <limits.h>
#    endif
#  endif

#define NBITS(type)                         (CHAR_BIT * sizeof(type))
#define BIT_T(type, n)                      ((n) < NBITS(type) ? ((type) 1) << ((n) % NBITS(type)) : ((type) 0))
#define BITMASK_T(type, t, b)               ((type)(BIT_T(type, 1 + (t)) - BIT_T(type, b)))

/** Create a 32 bit mask with only bit n set */
#define BIT_32(n)                           BIT_T(uint32_t, n)
/** Create a 32 bit mask with bits from b(ottom) to t(op) (inclusive) set */
#define BITMASK_32(t, b)                    BITMASK_T(uint32_t, t, b)
/** Create a 64 bit mask with only bit n set */
#define BIT_64(n)                           BIT_T(uint64_t, n)
/** Create a 64 bit mask with bits from b(ottom) to t(op) (inclusive) set */
#define BITMASK_64(t, b)                    BITMASK_T(uint64_t, t, b)

#endif  /* !defined(__LINKER_SCRIPT__) */


/* https://lists.freebsd.org/pipermail/freebsd-current/2007-February/069093.html */
#define _BITS_FLOOD_2(x)        (              (x) | (              (x) >>  1))
#define _BITS_FLOOD_4(x)        ( _BITS_FLOOD_2(x) | ( _BITS_FLOOD_2(x) >>  2))
#define _BITS_FLOOD_8(x)        ( _BITS_FLOOD_4(x) | ( _BITS_FLOOD_4(x) >>  4))
#define _BITS_FLOOD_16(x)       ( _BITS_FLOOD_8(x) | ( _BITS_FLOOD_8(x) >>  8))
#define _BITS_FLOOD_32(x)       (_BITS_FLOOD_16(x) | (_BITS_FLOOD_16(x) >> 16))
#define _BITS_FLOOD_64(x)       (_BITS_FLOOD_32(x) | (_BITS_FLOOD_32(x) >> 32))
/** Return the next power-of-2 value. Requires x > 0 and x < 2**31. */
#define CEIL_TO_POWER_OF_2(x)   (_BITS_FLOOD_32(x - 1) + 1)


/* Debugging */
/** Compile time assertion, to be used at function scope */
/* technique borrowed from http://www.jaggersoft.com/pubs/CVu11_3.html */
#define COMPILE_TIME_ASSERT(pred)           switch(0){case 0:case pred:;}


/** Annotate code as needed to be placed low in memory.
 * This is defined only for internal use. */
#if CHIP_CLASS_DSL
#  if defined(ASMCPP) || defined(__RALL2__)
#    define __low(section)                  STRINGIFY_EXPANDED(PASTE(.low, section))
#  else
#    define __low                           __attribute__((section_prefix(".low")))
#  endif
#else
#  if defined(ASMCPP) || defined(__RALL2__)
#    define __low(section)                  STRINGIFY(section)
#  else
#    define __low
#  endif
#endif

/** Explicitly mark a switch-case fallthrough on compilers that support/require that. */
#if __FP_GCC__ > 2 || (__FP_GCC__ == 2 && __FP_GCC_MINOR__ >= 8)
#  define __fallthrough         __attribute__((fallthrough))
#else
#  define __fallthrough
#endif

#endif /* !defined(ASMCPP) */


/* Preprocessor */
#define MACRO_EXPAND(x)             x
#define STRINGIFY(x)                #x
#define STRINGIFY_EXPANDED(x)       STRINGIFY(x)
#define PASTE(x, y)                 x ## y
#define PASTE_EXPANDED(x, y)        PASTE(x, y)
#define PASTE3(x, y, z)             x ## y ## z
#define PASTE3_EXPANDED(x, y, z)    PASTE3(x, y, z)


#endif /* _C_UTILS_INTERNAL_H_ */
