/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

#ifndef _ENDIANESS_H_
#define _ENDIANESS_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdint.h>
#else
#  include "bstd_defs.h"
#endif


#if !HAS_FEATURE(ENDIANESS)
#  error "No endianess indication found, please configure the ENDIANESS feature in CHIP.h"
#endif


/* Check if GCC bswap builtins are available or not. If no builtins,
 * BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS will be defined. */
#ifdef __GNUC__
#  define BFPSDK_ENDIANESS_GCC_VERSION (__GNUC__ * 0x10000 + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
#  if BFPSDK_ENDIANESS_GCC_VERSION < 0x040300    /* a GCC old enough to lack __builtin_bswap32 */
#    define BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS
#  endif
#  undef BFPSDK_ENDIANESS_GCC_VERSION
#else
#  define BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS
#endif


#ifdef BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS
uint16_t ENDIANESS_bswap16(uint16_t x);
uint32_t ENDIANESS_bswap32(uint32_t x);
uint64_t ENDIANESS_bswap64(uint64_t x);
#else
#  define ENDIANESS_bswap16     __builtin_bswap16
#  define ENDIANESS_bswap32     __builtin_bswap32
#  define ENDIANESS_bswap64     __builtin_bswap64
#endif


/* Decide if data from a FP subsystem requires
 * endianess swapping on this host. */
# if FEATURE_IS(ENDIANESS, BIG)

/* Big Endian Host - swap */
#  define ENDIANESS_fptoh16(i)  ENDIANESS_bswap16(i)
#  define ENDIANESS_htofp16(i)  ENDIANESS_bswap16(i)
#  define ENDIANESS_fptoh32(i)  ENDIANESS_bswap32(i)
#  define ENDIANESS_htofp32(i)  ENDIANESS_bswap32(i)
#  define ENDIANESS_fptoh64(i)  ENDIANESS_bswap64(i)
#  define ENDIANESS_htofp64(i)  ENDIANESS_bswap64(i)

# else

/* Little Endian Host - do nothing */
#  define ENDIANESS_fptoh16(i)  (i)
#  define ENDIANESS_htofp16(i)  (i)
#  define ENDIANESS_fptoh32(i)  (i)
#  define ENDIANESS_htofp32(i)  (i)
#  define ENDIANESS_fptoh64(i)  (i)
#  define ENDIANESS_htofp64(i)  (i)

# endif /* FEATURE_IS(ENDIANESS, BIG) */

#endif  /* _ENDIANESS_H_ */
