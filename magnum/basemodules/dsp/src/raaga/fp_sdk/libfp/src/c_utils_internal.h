/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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


/* Debugging */
/** Compile time assertion, to be used at function scope */
/* technique borrowed from http://www.jaggersoft.com/pubs/CVu11_3.html */
#define COMPILE_TIME_ASSERT(pred)       switch(0){case 0:case pred:;}


#endif /* !defined(ASMCPP) */

#endif /* _C_UTILS_INTERNAL_H_ */
