/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

/**
 * Utility function/macros and replacements for sometime missing non-standard
 * functions.
 * Note: endianess-related functions are handled in ENDIANESS.{h,c} .
 */


#ifndef _UTIL_H_
#define _UTIL_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stddef.h>
#  include <stdint.h>
#else
#  include "bstd_defs.h"
#endif


/**
 * The UTIL_strnlen() function returns the number of bytes in the string
 * pointed to by s, excluding the terminating null byte ('\0'), but at
 * most maxlen.  In doing this, UTIL_strnlen() looks only at the first maxlen
 * bytes at s and never beyond s+maxlen.
 */
size_t UTIL_strnlen(const char *s, size_t maxlen);


/**
 * The UTIL_stpncpy() function copies at most n characters from the string
 * pointed to by src, including the terminating null byte ('\0'), to the array
 * pointed to by dest.
 * Exactly n characters are written at dest. If the length strlen(src) is smaller
 * than n, the remaining characters in the array pointed to by dest are filled
 * with null bytes ('\0'), If the length strlen(src) is greater than or equal to
 * n, the string pointed to by dest will not be null-terminated.
 *
 * UTIL_stpncpy() returns a pointer to the terminating null byte in dest, or,
 * if dest is not null-terminated, dest+n.
 */
char *UTIL_stpncpy(char *dest, const char *src, size_t n);


#if IS_TARGET(BCM93160A0_OFDMA_Rx_si) || \
    FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
/* The MIPS-SDE libc and Raaga Magnum toolchains lack the strnlen and stpncpy
 * functions, use UTIL_strnlen and UTIL_stpncpy as replacements. */
#  define strnlen UTIL_strnlen
#  define stpncpy UTIL_stpncpy
#endif


/**
 * Converts an integer value to a null-terminated string using the specified base
 * and stores the result in a static char array. Returns a pointer to the result
 * converted string.
 */
char* UTIL_itoa(uint32_t val, unsigned base);


#endif  /* _UTIL_H_ */
