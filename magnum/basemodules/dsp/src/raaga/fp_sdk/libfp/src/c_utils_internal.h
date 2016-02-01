/******************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                     *
 *                                                                            *
 *      This material is the confidential trade secret and proprietary        *
 *      information of Broadcom Corporation. It may not be reproduced,        *
 *      used, sold or transferred to any third party without the prior        *
 *      written consent of Broadcom Corporation. All rights reserved.         *
 *                                                                            *
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
