/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSTD_DEFS_H__
#define BSTD_DEFS_H__

#include <iso646.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>


#define BSTD_UNUSED(x) ((void)x)


/* can't include stdint */
/* #include <stdint.h> */

typedef   signed char  int8_t;
typedef unsigned char uint8_t;

typedef   signed short  int16_t;
typedef unsigned short uint16_t;

typedef   signed long  int32_t;
typedef unsigned long uint32_t;

typedef unsigned long long uint64_t;
typedef signed long long int64_t;

typedef  int8_t  int_least8_t;
typedef uint8_t uint_least8_t;

typedef  int16_t  int_least16_t;
typedef uint16_t uint_least16_t;

typedef  int32_t  int_least32_t;
typedef uint32_t uint_least32_t;

/* 7.18.4.1 Macros for minimum-width integer constants */

#define  INT8_C(value)  ((int_least8_t)(value))
#define UINT8_C(value) ((uint_least8_t)(value))

#define  INT16_C(value)  ((int_least16_t)(value))
#define UINT16_C(value) ((uint_least16_t)(value))

#define  INT32_C(value)  ((int_least32_t)(value ## L))
#define UINT32_C(value) ((uint_least32_t)(value ## UL))

/* 7.18.4.2 Macros for greatest-width integer constants */

#define  INTMAX_C(value)  ((intmax_t)(value ## L))
#define UINTMAX_C(value) ((uintmax_t)(value ## UL))


#endif /* BSTD_DEFS_H__ */

