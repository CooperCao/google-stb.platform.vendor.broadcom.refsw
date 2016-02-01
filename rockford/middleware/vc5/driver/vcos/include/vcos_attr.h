/*
 * Copyright (c) 2010-2013 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

/*=============================================================================
VideoCore OS Abstraction Layer - compiler-specific attributes
=============================================================================*/

#ifndef VCOS_ATTR_H
#define VCOS_ATTR_H

/**
 * Type attribute indicating the enum should be stored in as few bytes as
 * possible. MetaWare does this by default, so the attribute is useful when
 * structs need to be portable to GCC too.
 *
 * MSVC doesn't support VCOS_ENUM_PACKED, so code that needs to be portable
 * across all platforms but wants the type-safety and debug-info benefits
 * of enum types when possible, should do:
 *
 *    typedef enum VCOS_ENUM_PACKED { a = 0, b = 0xffff } EXAMPLE_T;
 *    struct foo {
 *       int bar;
 *       #if VCOS_HAS_ENUM_PACKED
 *       EXAMPLE_T baz;
 *       #else
 *       uint16_t baz;
 *       #endif
 *    };
 */
#if defined(__GNUC__)
# define VCOS_ENUM_PACKED  __attribute__ ((packed))
# define VCOS_HAS_ENUM_PACKED 1
#elif defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ENUM_PACKED /* packed enums are default on Metaware */
#ifndef VCOS_HAS_ENUM_PACKED /* To allow override when using hcl386 */
# define VCOS_HAS_ENUM_PACKED 1
#endif
#else
# define VCOS_ENUM_PACKED
# define VCOS_HAS_ENUM_PACKED 0
#endif

#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
#define VCOS_ATTR_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define VCOS_ATTR_NORETURN __declspec(noreturn)
#else
#define VCOS_ATTR_NORETURN
#endif

/** Variable attribute indicating the variable must be emitted even if it appears unused. */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_USED  __attribute__ ((used))
#else
# define VCOS_ATTR_USED
#endif

/** Variable attribute indicating the compiler should not warn if the variable is unused. */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_POSSIBLY_UNUSED  __attribute__ ((unused))
#else
# define VCOS_ATTR_POSSIBLY_UNUSED
#endif

/** Variable attribute requiring specific alignment.
 *
 * Use as:
 *   int VCOS_ATTR_ALIGNED(256) n;
 * or:
 *   VCOS_ATTR_ALIGNED(256) int n;
 * or if you don't want to support MSVC:
 *   int n VCOS_ATTR_ALIGNED(256);
 */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_ALIGNED(n)  __attribute__ ((aligned(n)))
#elif defined(_MSC_VER)
# define VCOS_ATTR_ALIGNED(n)  __declspec(align(n))
#else
/* Force a syntax error if this is used when the compiler doesn't support it,
 * instead of silently misaligning */
# define VCOS_ATTR_ALIGNED(n) VCOS_ATTR_ALIGNED_NOT_SUPPORTED_ON_THIS_COMPILER
#endif

/** Max alignment reported by vcos_alignof() for any type, ie the alignment
 * that malloc() guarantees */
#define VCOS_MAX_ALIGN 8 /* Just assume this for now */

/** Variable attribute requiring specific ELF section.
 *
 * Use as:
 *   int n VCOS_ATTR_SECTION(".foo") = 1;
 *
 * A pointer like &n will have type "VCOS_ATTR_SECTION_QUALIFIER int *".
 */
#if defined(__GNUC__)
# define VCOS_ATTR_SECTION(s)  __attribute__ ((section(s)))
# define VCOS_ATTR_SECTION_QUALIFIER
#elif defined(__HIGHC__) || defined(__VECTORC__)
/* hcvc requires 'far' else it'll put small objects in .sdata/.rsdata/.sbss */
# define VCOS_ATTR_SECTION(s)  __attribute__ ((far, section(s)))
# define VCOS_ATTR_SECTION_QUALIFIER _Far
#else
/* Force a syntax error if this is used when the compiler doesn't support it */
# define VCOS_ATTR_SECTION(s) VCOS_ATTR_SECTION_NOT_SUPPORTED_ON_THIS_COMPILER
# define VCOS_ATTR_SECTION_QUALIFIER
#endif

/** Define a function as a weak alias to another function.
 * @param ret_type     Function return type.
 * @param alias_name   Name of the alias.
 * @param param_list   Function parameter list, including the parentheses.
 * @param target_name  Target function (bare function name, not a string).
 */
#if defined(__GNUC__) || defined(__HIGHC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS(ret_type, alias_name, param_list, target_name) \
   __attribute__ ((weak, alias(#target_name))) ret_type alias_name param_list
#elif defined(__VECTORC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS(ret_type, alias_name, param_list, target_name) \
   pragma weak(alias_name, target_name)
#else
# define VCOS_WEAK_ALIAS(ret_type, alias, params, target)  static_assrt(0)
#endif

/** Define a function as a weak alias to another function, specified as a string.
 * @param ret_type     Function return type.
 * @param alias_name   Name of the alias.
 * @param param_list   Function parameter list, including the parentheses.
 * @param target_name  Target function name as a string.
 * @note Prefer the use of VCOS_WEAK_ALIAS - it is likely to be more portable.
 *       Only use VCOS_WEAK_ALIAS_STR if you need to do pre-processor mangling of the target
 *       symbol.
 */
#if defined(__GNUC__) || defined(__HIGHC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS_STR(ret_type, alias_name, param_list, target_name) \
   __attribute__ ((weak, alias(target_name))) ret_type alias_name param_list
#elif defined(__VECTORC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS_STR(ret_type, alias_name, param_list, target_name) \
   pragma weak(alias_name, target_name)
#else
# define VCOS_WEAK_ALIAS_STR(ret_type, alias, params, target)  static_assrt(0)
#endif

/** Define a weak symbol
 *
 * e.g.
 *
 * VCOS_WEAK_SYM(int, foo);
 *
 */
#if defined(__GNUC__)
# define VCOS_WEAK_SYM(sym_type, sym_name) sym_type sym_name __attribute__((weak))
#elif defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_WEAK_SYM(sym_type, sym_name) \
   sym_type sym_name; \
   pragma weak(sym_name)
#else
# define VCOS_WEAK_SYM(sym_type, sym_name) static_assrt(0)
#endif

#endif /* VCOS_ATTR_H */
