/****************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BFPSDK_LIBDSPCONTROL_FEATURE_ENDIANESS
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
