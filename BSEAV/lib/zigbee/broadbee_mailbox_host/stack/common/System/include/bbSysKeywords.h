/******************************************************************************
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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      SYS macro keywords definition.
 *
*******************************************************************************/

#ifndef _BB_SYS_KEYWORDS_H
#define _BB_SYS_KEYWORDS_H


/************************* DEFINITIONS **************************************************/
/*
 * SYS public and private domains visibility attributes.
 */
#if defined(_MAKE_CC_SYS_)      /* Single SYS CC-file build target. */
# define SYS_PUBLIC             extern
# define SYS_FRIEND             extern
# define SYS_PRIVATE            static
# define SYS_MEMDECL(vardecl)
# define SYS_MEMDEF             static
# define SYS_STATIC             static
#
#else                           /* All files are compiled separately. */
# define SYS_PUBLIC             extern
# define SYS_FRIEND             extern
# define SYS_PRIVATE            extern
# define SYS_MEMDECL(vardecl)   extern vardecl
# define SYS_MEMDEF
# define SYS_STATIC             static
#
#endif


/*
 * HAL public and private domains visibility attributes.
 */
#if defined(_MAKE_CC_HAL_)      /* Single HAL CC-file build target. */
# define HAL_PUBLIC  extern
# define HAL_PRIVATE static
# define HAL_STATIC  static
#
#else                           /* All files are compiled separately. */
# define HAL_PUBLIC  extern
# define HAL_PRIVATE extern
# define HAL_STATIC  static
#
#endif


/*
 * Cross-platform macros to support packed data structures.
 */
#if defined(__arc__)
# define PACKED                 __attribute__ ((packed))
# define UNALIGNED              _Unaligned
#
#elif defined(__arm__) || defined(__i386__)
# undef  PACKED
# define PACKED                 __attribute__ ((packed, gcc_struct))
# ifndef UNALIGNED
#  define UNALIGNED
# endif
#
#endif

/* Cast a const value to a non-const value.  i.e. cheat when we want
   initialized variables to be placed in iCCM and also to be changeable */
#define UNCONSTU32(x) *(uint32_t *)(x)
#define UNCONSTU16(x) *(uint16_t *)(x)
#define UNCONSTU8(x) *(uint8_t *)(x)

#define UNCONSTI32(x) *(int32_t *)(x)
#define UNCONSTI16(x) *(int16_t *)(x)
#define UNCONSTI8(x) *(int8_t *)(x)


/*
 * Cross-platform auxiliary macros.
 */
#if defined(__arc__)
/* Use static inline for -Os to achieve additional optimization. */
//# define inline                 static _Inline
//# define INLINE                 static _Inline
/* Use inline for -O0, otherwise .text section will grow. */
# define inline                 _Inline
# define INLINE                 _Inline
# define NOINIT                 __attribute__ ((section (".noinit")))
# define WEAK                   __attribute__ ((weak))
#
#elif defined(__arm__) || defined(__i386__)
# define INLINE                 static inline
# define NOINIT
# define WEAK                   /* define if required */
#
#endif


#endif /* _BB_SYS_KEYWORDS_H */

/* eof bbSysKeywords.h */