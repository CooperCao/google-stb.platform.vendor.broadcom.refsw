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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysOptions.h $
*
* DESCRIPTION:
*   Compiler and SYS options setup.
*
* $Revision: 3858 $
* $Date: 2014-10-03 09:33:34Z $
*
*****************************************************************************************/


#ifndef _BB_SYS_OPTIONS_H
#define _BB_SYS_OPTIONS_H


/************************* AUTO CONFIGURATION *******************************************/
/*
 * Support 64-bit GCC.
 */
#if defined(__x86_64__) && !defined(__i386__)
# define __i386__
#endif


/************************* VALIDATIONS **************************************************/
/*
 * Validate the current build target platform.
 */
#if defined(__arc__)
#
# if defined(__i386__)
#  error Platform signature conflict between ARC and i386 has been detected. Build is terminated.
# elif (_ARCVER != 0x25)
#  error Only ARC601 chip is supported for the ARC family target. Build is terminated.
# endif
#
# /* ARC601 is supported. */
#
#elif defined(__i386__)
#
# /* i386 is supported. */
#
#else
# error Platform is not recognized and is not supported. Build is terminated.
#endif


/*
 * Validate Unit-tests environment usage.
 */
#if defined(_UNIT_TEST_) && defined(__arc__)
# error Unit-tests environment is not allowed for ARC platform.
#endif


/*
 * Validate the compiler type.
 */
#if defined(__cplusplus) && defined(__arc__)
# error G++ compiler is not allowed for ARC platform.
#endif


/*
 * Validate the target platform for application profiling.
 */
#if defined(_PROFILE_) && !defined(__arc__)
# error Profilig is implemented only for ARC platform.
#endif


/************************* AUTO CONFIGURATION *******************************************/
/*
 * Customize debugging and profiling for Unit-Test build.
 */
#if defined(_UNIT_TEST_)
#
# undef  _DEBUG_
# define _DEBUG_                    /* Enable Regular Debugging. */
#
# undef  _DEBUG_COMPLEX_
# define _DEBUG_COMPLEX_            /* Enable Debugging of Complex expressions. */
#
# undef  _DEBUG_LOG_
# define _DEBUG_LOG_                /* Enable Debug Logging. */
#
# undef  _DEBUG_HARNESSLOG_
# define _DEBUG_HARNESSLOG_  2      /* Deliver Errors and Warnings to the Test Teardown interface. */
#
# undef  _DEBUG_CONSOLELOG_         /* Disable custom Debug Logging to Console; use the Test-Harness functionality. */
#
# undef  _DEBUG_STDOUTLOG_
# undef  _DEBUG_STDERRLOG_
# undef  _DEBUG_STDUARTLOG_         /* Use the default Test-Harness console for the Text Debug Logging. */
#
# undef  _DEBUG_FILELINE_
# define _DEBUG_FILELINE_           /* Enable File-Line Debugging information. */
#
# undef  _PROFILE_
# define _PROFILE_                  /* Enable Regular Profiling. */
#
#endif /* _UNIT_TEST_ */


/************************* DEFINITIONS **************************************************/
/*
 * Set up MetaWare C compiler for ARC target platform.
 */
#if defined(__arc__)
# pragma On(Microsoft)                  /* Enable anonymous substructures.       */
# pragma Off(Long_enums)                /* Implement short enums.                */
# pragma Off(Volatile_cache_bypass)     /* Cache is not implemented in hardware. */
# pragma On(Packed_means_unaligned)     /* Make all packed structures unaligned. */
# pragma On(Unaligned_means_packed)     /* Make all unaligned structures packed. */
# pragma Off(Behaved)                   /* The program can freely use pointers.  */
#endif


/*
 * Set up GNU C compiler for i386 target platform.
 */
#if defined (__i386__) && !defined (__clang__)
# pragma GCC optimize "short-enums"     /* Implement short enums. */
# pragma GCC diagnostic ignored "-Wattributes"
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Macro to be used in the #if preprocessor instruction to test the given
 *  macro-identifier how it is defined.
 * \param[in]   macro       Identifier of a macro to be tested.
 * \return  TRUE if \p macro is defined as 1 (one) or defined without a value; FALSE if
 *  \p macro is defined with different value then 1(one), for example 0 (zero) or another
 *  numeric value, or if \p macro is not defined.
 */
#define DEFINED_OR_ONE(macro)       (__DEFINED_OR_ONE(macro) == 10) || (__DEFINED_OR_ONE(macro) == 110)

/**//**
 * \brief   Helper macro to be used by DEFINED_OR_ONE() macro.
 * \param[in]   macro       Identifier of a macro to be processed.
 * \return  Different numeric values depending on how \p macro is defined. If \p macro is
 *  defined as 1 (one), then 110 is returned. If \p macro is defined without a value,
 *  then 10 is returned. If \p macro is defined as 0 (zero), then 010 is returned which
 *  is the octal form of 8 (eight).
 */
#define __DEFINED_OR_ONE(macro)     macro##10


/**//**
 * \brief   Macro tests if the supplied \p macro is defined empty.
 * \param[in]   macro       Identifier of a macro to be tested.
 * \return  TRUE if \p macro is defined without a value; FALSE if \p macro is defined with
 *  some numeric value or numeric expression, or not defined at all.
 * \details Consider the first part of the resolving expression. When \p macro is defined
 *  empty (i.e., for example <tt>#define MY_EMPTY_MACRO</tt>), the first expression
 *  <tt>(macro+1 == -macro-1)</tt> is resolved by preprocessor to <tt>(+1 == - -1)</tt>
 *  and consequently evaluates to TRUE. And the same with the second part of expression.\n
 *  If \p macro is not defined at all, the preprocessor considers it equal to zero when
 *  it's used in a preprocessor equation. So, the first expression is resolved to
 *  <tt>(0+1 == -0-1)</tt> and consequently evaluates to FALSE. The same with the second
 *  part of expression.\n
 *  And finally, if \p macro is defined with a numeric value N (or even a complex numeric
 *  expression), then the first expression is resolved to <tt>(N+1 == -N-1)</tt> and
 *  obviously evaluates to FALSE with tiny exception. The same with the second part of
 *  expression.
 * \details The only exception is when \p macro is defined, for example, as -1. In this
 *  case the first part of expression is resolved to <tt>(-1+1 == - -1-1)</tt> and
 *  undesirably evaluates to TRUE. To screen this effect the second part is introduced; it
 *  is resolved to <tt>(-2+1 == - -2-1)</tt> and evaluates to FALSE.
 * \details It may be proved that no other numeric expression assigned to \p macro lead
 *  this expression to become TRUE. Let \p macro be in the form <tt>x+y</tt>. Then the
 *  test expression is resolved to <tt>(n+m+1 == -n+m-1 && n+m+2 == -n+m-2)</tt> which is
 *  equivalent to <tt>(n+1 == -n-1 && n+2 == -n-2)</tt>. So, the above complex form of
 *  \p macro (when the negation sign is applied only to <tt>x</tt> but not to <tt>y</tt>)
 *  is redundant and does not give any new solution above the simple form of \p macro.\n
 *  And arbitrary simple form of \p macro (when the negation sign is applied to the whole
 *  \p macro) is not able to resolve <tt>(n+1 == -n-1 && n+2 == -n-2)</tt> to TRUE,
 *  because <tt>n+n == -1-1</tt> and <tt>n+n == -2-2</tt> may not hold simultaneously.
 * \details Parentheses around compared expressions are necessary to make the equality
 *  operator the highest priority over all the operators inside the \p macro. Without
 *  parentheses the expression evaluates improperly, for example, if \p macro is the
 *  bitwise OR operator without enclosing parentheses (like this: "0x04|0x08").
 */
#define DEFINED_EMPTY(macro)        (((macro+1) == (-macro-1)) && ((macro+2) == (-macro-2)))


#endif /* _BB_SYS_OPTIONS_H */
