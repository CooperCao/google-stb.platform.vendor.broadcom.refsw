/***************************************************************************
 *     (c)2003-2012 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
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

/******************************************************************************

This header file implements the environment described below, with the
exception of <float.h>, for the following compilation environments:

- C90 (as defined by ISO/IEC 9899:1990(E))
- C95 (as defined by ISO/IEC 9899:1995(E) Amendment 1 to C90)
- C99 (as defined by ISO/IEC 9899:1999(E))
- C++ (as defined by ISO/IEC 14882:1998(E))

As stated in ISO/IEC 9899:1999(E) clause 4 paragraph 6:

"A conforming freestanding implementation shall accept any strictly
conforming program that does not use complex types and in which the
use of the features specified in the library clause (clause 7) is
confined to the contents of the standard headers <float.h>,
<iso646.h>, <limits.h>, <stdarg.h>, <stdbool.h>, <stddef.h>, and
<stdint.h>."

Our intention is to standardize on a set of types that is based on C99.

******************************************************************************/

#define BSTD_UNUSED(x)  { volatile void *bstd_unused; bstd_unused = (void *)&(x); }

#define STDC_1995 199409L
#define STDC_1999 199901L

#if defined __STDC_VERSION__ && __STDC_VERSION__ != STDC_1995 && __STDC_VERSION__ != STDC_1999
	#pragma message( "Unanticipated value: predefined macro __STDC_VERSION__" )
#endif

/*****************************************************************************/

#define STDCPP_1998 199711L

#if defined __cplusplus && __cplusplus && __cplusplus != 1 && __cplusplus != STDCPP_1998
	#pragma message( "Unanticipated value: predefined macro __cplusplus" )
#endif

/*****************************************************************************/

#if __STDC_VERSION__ < STDC_1999
	#define restrict
#endif

/*****************************************************************************/
/*                                                                           */
/* 17.9 Alternative spellings                                                */
/*                                                                           */
/*****************************************************************************/

#if (defined __cplusplus || defined __STDC_VERSION__) && !defined UNDER_CE
	#include <iso646.h>
#else
	#define and    &&
	#define and_eq &=
	#define bitand &
	#define bitor  |
	#define compl  ~
	#define not    !
	#define not_eq !=
	#define or     ||
	#define or_eq  |=
	#define xor    ^
	#define xor_eq ^=
#endif

/*****************************************************************************/
/*                                                                           */
/* 17.10 Sizes of integer types <limits.h>                                   */
/*                                                                           */
/*****************************************************************************/

#include <limits.h>

/*****************************************************************************/
/*                                                                           */
/* 17.15 Variable arguments <stdarg.h>                                       */
/*                                                                           */
/*****************************************************************************/

#include <stdarg.h>

/*****************************************************************************/
/*                                                                           */
/* 17.16 Boolean type and values <stdbool.h>                                 */
/*                                                                           */
/*****************************************************************************/

#if __STDC_VERSION__ >= STDC_1999
	#include <stdbool.h>
#else
	#if !defined __cplusplus
		#if defined BRCM_BOOL
			#define bool BRCM_BOOL
		#else
         #if defined(_MSC_VER)
          #if _MSC_VER > 1300
		   #define bool unsigned char       
          #else /* MSC_VER > 1300 */
		   #define bool unsigned int
          #endif /* _MSC_VER > 1300 */
         #else /* defined(_MSC_VER) */
		  #define bool unsigned int        
         #endif        
		#endif
		#define true 1
		#define false 0
	#endif
	#define __bool_true_false_are_defined 1
#endif

/*****************************************************************************/
/*                                                                           */
/* 17.17 Common definitions <stddef.h>                                       */
/*                                                                           */
/*****************************************************************************/

#include <stddef.h>

/*****************************************************************************/
/*                                                                           */
/* 17.18 Integer types <stdint.h>                                            */
/*                                                                           */
/*****************************************************************************/

#if __STDC_VERSION__ >= STDC_1999
	#include <stdint.h>
#else

/*************************/
/* 17.18.1 Integer types */
/*************************/

#if defined __sun__
	#include <sys/int_types.h>
#else

/* 17.18.1.1 Exact-width integer types */

#if defined LINUX
	#if defined __KERNEL__
		#include <linux/types.h>
	#else
		#include <sys/types.h>
		typedef u_int8_t  uint8_t;
		typedef u_int16_t uint16_t;
		typedef u_int32_t uint32_t;
	#endif
#elif defined VXWORKS
	#include <types/vxTypes.h>
#else
	typedef   signed char  int8_t;
	typedef unsigned char uint8_t;

	typedef   signed short  int16_t;
	typedef unsigned short uint16_t;

	typedef   signed long  int32_t;
	typedef unsigned long uint32_t;
#endif

	typedef uint32_t off_t;

/* 64 bit integer types */
#if defined(INT64_MAX)
/* 64 bit integers shall be already defined there */
# elif defined(VXWORKS)
/* Use VxWorks version of int64_t and uint64_t */
# elif defined(_MSC_VER)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
# elif defined(__GNUC__)
typedef long long int64_t;
typedef unsigned long long uint64_t;
# elif defined(_WIN32_WCE)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
# else
#  define BSTD_INT64_NOT_DEFINED
#endif

/* signed size type */
typedef long ssize_t;

/* 17.18.1.2 Minimun-width integer types */

typedef  int8_t  int_least8_t;
typedef uint8_t uint_least8_t;

typedef  int16_t  int_least16_t;
typedef uint16_t uint_least16_t;

typedef  int32_t  int_least32_t;
typedef uint32_t uint_least32_t;

/* 17.18.1.3 Fastest minimum-width integer types */

typedef  int8_t  int_fast8_t;
typedef uint8_t uint_fast8_t;

typedef  int16_t  int_fast16_t;
typedef uint16_t uint_fast16_t;

typedef  int32_t  int_fast32_t;
typedef uint32_t uint_fast32_t;

/* 7.18.1.4 Integer types capable of holding object pointers */

#if defined LINUX && !defined __KERNEL__
#include <unistd.h>
#elif !defined WIN32 || _MSC_VER < 1300 || (defined UNDER_CE && UNDER_CE < 600)
typedef  int32_t  intptr_t;
#endif
#if !defined WIN32 || _MSC_VER < 1300 || (defined UNDER_CE && UNDER_CE < 600)
typedef uint32_t uintptr_t;
#endif

/* 7.18.1.5 Greatest-width integer types */

typedef  int32_t  intmax_t;
typedef uint32_t uintmax_t;

#endif /* __sun__ */

/***************************************************/
/* 7.18.2  Limits of specified-width integer types */
/***************************************************/

#if defined __sun__
	#include <sys/int_limits.h>
#else

#if !defined __cplusplus || defined __STDC_LIMIT_MACROS /* 217 */

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

#endif /* 217 */

#endif /* __sun__ */

/****************************************/
/* 7.18.3 Limits of other integer types */
/****************************************/

#if !defined __cplusplus || defined __STDC_LIMIT_MACROS /* 218 */

/* min signed ptrdiff_t range: -65535 .. 65535 */
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

/* min   signed sig_atomic_t range: -127 .. 127 */
/* min unsigned sig_atomic_t range:    0 .. 255 */
#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

/* min unsigned size_t range: 0 .. 65535 */
// #if !defined __sun__
#if !defined SIZE_MAX
	#define SIZE_MAX UINT32_MIN
#endif /* __sun__ */

/* min   signed wchar_t range: -127 .. 127 */
/* min unsigned wchar_t range:    0 .. 255 */
#define WCHAR_MIN UINT16_MIN
#define WCHAR_MAX UINT16_MAX

/* min   signed wint_t range: -32767 .. 32767 */
/* min unsigned wint_t range:      0 .. 65535 */
#define WINT_MIN UINT16_MIN
#define WINT_MAX UINT16_MAX

#endif /* 218 */

/***************************************/
/* 7.18.4 Macros for integer constants */
/***************************************/

#if !defined __cplusplus || defined __STDC_CONSTANT_MACROS /* 220 */

/* 7.18.4.1 Macros for minimum-width integer constants */

#define  INT8_C(value)  ((int_least8_t)(value))
#define UINT8_C(value) ((uint_least8_t)(value))

#define  INT16_C(value)  ((int_least16_t)(value))
#define UINT16_C(value) ((uint_least16_t)(value))

#define  INT32_C(value)  ((int_least32_t)(value ## L))
#if 0 //RRL
#define UINT32_C(value) ((uint_least32_t)(value ## UL))
#else
#define UINT32_C(value) ((uint_least32_t)(value))
#endif
/* 7.18.4.2 Macros for greatest-width integer constants */

#define  INTMAX_C(value)  ((intmax_t)(value ## L))
#define UINTMAX_C(value) ((uintmax_t)(value ## UL))

#endif /* 220 */

#endif /* !C99 */

struct tm {
int tm_mon;
int tm_mday;
int tm_year;
int tm_hour;
int tm_min;
};

/* TBD other platforms define custom bstd_cfg.h for nexus */
#define BSTD_INLINE extern __inline

/*****************************************************************************/

#endif /* BRCM_T_H__ */

/* end of file */
