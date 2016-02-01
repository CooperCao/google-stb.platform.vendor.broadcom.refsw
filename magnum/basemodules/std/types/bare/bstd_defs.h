/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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
/* TODO: not found #include <limits.h> */
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#define BSTD_BARE_OS    1
#define BSTD_UNUSED(x) ((void)x)


/* can't include stdint */
/* #include <stdint.h> */

typedef   signed char  int8_t;
typedef unsigned char uint8_t;

typedef   signed short  int16_t;
typedef unsigned short uint16_t;

typedef   signed int int32_t;
typedef unsigned int uint32_t;

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

/* 7.18.2.1 Limits of exact-width integer types */

#define  INT8_MIN (-128)
#define  INT8_MAX ( 127)
#define UINT8_MAX ( 255)

#define  INT16_MIN (-32768)
#define  INT16_MAX ( 32767)
#define UINT16_MAX ( 65535)

#define  INT32_MIN (-2147483648L)
#define  INT32_MAX ( 2147483647L)
#define UINT32_MAX ( 4294967295UL)

/* 7.18.2.2 Limits of minimum-width integer types */

#define  INT_LEAST8_MIN  INT8_MIN
#define  INT_LEAST8_MAX  INT8_MAX
#define UINT_LEAST8_MAX UINT8_MAX

#define  INT_LEAST16_MIN  INT16_MIN
#define  INT_LEAST16_MAX  INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX

#define  INT_LEAST32_MIN  INT32_MIN
#define  INT_LEAST32_MAX  INT32_MAX
#define UINT_LEAST32_MAX UINT32_MAX

/* 7.18.2.3 Limits of fastest minimum-width integer types */

#define  INT_FAST8_MIN  INT8_MIN
#define  INT_FAST8_MAX  INT8_MAX
#define UINT_FAST8_MAX UINT8_MAX

#define  INT_FAST16_MIN  INT16_MIN
#define  INT_FAST16_MAX  INT16_MAX
#define UINT_FAST16_MAX UINT16_MAX

#define  INT_FAST32_MIN  INT32_MIN
#define  INT_FAST32_MAX  INT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

/* 7.18.2.4 Limits of integer types capable of holding object pointers */

#define  INT_PTR_MIN  INT32_MIN
#define  INT_PTR_MAX  INT32_MAX
#define UINT_PTR_MAX UINT32_MAX

/* 7.18.2.5 Limits of greatest-width integer types */

#define  INTMAX_MIN  INT32_MIN
#define  INTMAX_MAX  INT32_MAX
#define UINTMAX_MAX UINT32_MAX

/* min signed ptrdiff_t range: -65535 .. 65535 */
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

/* min   signed sig_atomic_t range: -127 .. 127 */
/* min unsigned sig_atomic_t range:    0 .. 255 */
#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

/* min   signed wchar_t range: -127 .. 127 */
/* min unsigned wchar_t range:    0 .. 255 */
#define WCHAR_MIN UINT16_MIN
#define WCHAR_MAX UINT16_MAX

/* min   signed wint_t range: -32767 .. 32767 */
/* min unsigned wint_t range:      0 .. 65535 */
#define WINT_MIN UINT16_MIN
#define WINT_MAX UINT16_MAX


#define __int8_t_defined
#define __int16_t_defined
#define __int32_t_defined
#define __int64_t_defined

#endif /* BSTD_DEFS_H__ */
