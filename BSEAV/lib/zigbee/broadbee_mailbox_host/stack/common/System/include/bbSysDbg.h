/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysDbg.h $
*
* DESCRIPTION:
*   Debug asserts toolset interface.
*
* $Revision: 3943 $
* $Date: 2014-10-07 20:55:38Z $
*
*****************************************************************************************/


#ifndef _BB_SYS_DBG_H
#define _BB_SYS_DBG_H


/************************* INCLUDES *****************************************************/
#include "bbSysUtils.h"             /* Auxiliary utilities toolset interface. */
#include "bbSysDbgUids.h"           /* Debug asserts UIDs enumeration. */

#ifdef __cplusplus
extern "C"{
#endif

/************************* AUTO CONFIGURATION *******************************************/
/*
 * Switch-on regular debugging for complex debugging and/or debug logging.
 */
#if defined(_DEBUG_COMPLEX_) && !defined(_DEBUG_)
# define _DEBUG_
#endif

#if defined(_DEBUG_LOG_) && !defined(_DEBUG_)
# define _DEBUG_
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Set of macros for screening the asserted expressions.
 * \param[in]   guard       The asserted expression that is validated to be TRUE under the
 *  debug build. This parameter may be omitted.
 * \return  The guard expression that evaluates to boolean value.
 * \details Under the release build the \p guard is substituted with TRUE which means that
 *  the asserted expression is considered to be satisfied unconditionally. Under the debug
 *  build the \p guard, if specified, is returned as-is; and if it's empty the FALSE is
 *  returned.
 */
/**@{*/
#define DBG_ASSERT_WRAPPER_ENABLED(guard)       (guard+FALSE)
#define DBG_ASSERT_WRAPPER_1(guard)             (guard+FALSE)
#define DBG_ASSERT_WRAPPER_(guard)              (guard+FALSE)
#define DBG_ASSERT_WRAPPER_DISABLED(guard)      (TRUE)
#define DBG_ASSERT_WRAPPER_0(guard)             (TRUE)
/**@}*/


/**//**
 * \brief   Wrapper macro for the guarded expression of particular severity level.
 * \param[in]   status      Enables or disables the asserted expression validation.
 * \param[in]   guard       The asserted expression. This parameter may be omitted.
 * \return  The guard expression that evaluates to boolean value.
 * \details If the \p status evaluates to empty identifier, or 1, or ENABLED, the \p guard
 *  expression is asserted. If \p status evaluates to 0 or DISABLED, the \p guard is
 *  screened and evaluates to TRUE unconditionally.
 * \details The \p status may be a different macro. To resolve it properly the nested
 *  helper macro is used.
 */
/**@{*/
#define DBG_ASSERT_WRAPPER(status, guard)       __DBG_ASSERT_WRAPPER(status, guard)
#define __DBG_ASSERT_WRAPPER(status, guard)     DBG_ASSERT_WRAPPER_##status(guard)
/**@}*/


/**//**
 * \brief   Set of constants defining severity levels of asserted expressions.
 * \details These constants may be joined with the bitwise OR. No ordering is established
 *  between levels - i.e., each level may be enabled or disabled separately.
 * \details These macros establishes the minimal set of supported severity levels.
 *  Additional key macros may be defined within particular units to provide more tunable
 *  debug. All the private keys must have values of power of two (in order to support
 *  bitwise OR joining) and must be in the range from 0x100 to 0x80000000.
 * \details When the set of levels macro is defined externally with the -D compiler
 *  directive, the meaning is the following:
 *  - -Dmyunit_debug=DBG_FAIL           -- enables only the Failure level,
 *  - -Dmyunit_debug=DBG_FAIL|DBG_WARN  -- enables Failure and Warning levels, disables
 *      Halt and Information levels. Parentheses around the expression are not necessary,
 *  - -Dmyunit_debug=DBG_NONE           -- disables all debug levels,
 *  - -Dmyunit_debug=                   -- evaluates to empty value for the myunit_debug
 *      macro. This syntax should not be used with -D directive because one may think that
 *      it would disable all debug levels for the myunit, but it may conflict with the
 *      meaning of the "#define myunit_debug" directive which may be thought as,
 *      inversely, to enable all debug levels for the myunit,
 *  - -Dmyunit_debug                    -- evaluates to one and enables all debug levels,
 *  - if omitted                        -- evaluates to zero and disables all debug
 *      levels.
 *
 * \details When the set of levels macro is defined internally with the #define
 *  preprocessor directive, the meaning is the following:
 *  - #define myunit_debug DBG_FAIL             -- enables only the Failure level,
 *  - #define myunit_debug DBG_FAIL|DBG_WARN    -- enables Failure and Warning levels,
 *      disables Halt and Information levels. Parentheses around the expression are not
 *      necessary,
 *  - #define myunit_debug DBG_NONE             -- disables all debug levels,
 *  - #define myunit_debug                      -- evaluates to the empty value and should
 *      be considered as the directive to enable all debug levels,
 *  - if omitted                                -- evaluates to zero and disables all debug
 *      levels.
 *
 * \note    Value 1 is chosen for the DBG_ALL because of the case when the set of levels
 *  macro is defined externally with the -D compiler option without assignment - i.e.,
 *  like this: "-Dmyunit_debug". In this case the myunit_debug macro evaluates to 1.
 */
/**@{*/
#define DBG_NONE    (0)         /*!< All levels are disabled. */
#define DBG_INFO    (0x10)      /*!< Severity level Information. */
#define DBG_WARN    (0x20)      /*!< Severity level Warning. */
#define DBG_HALT    (0x40)      /*!< Severity level Halt (stands for Error). */
#define DBG_FAIL    (0x80)      /*!< Severity level Failure. */
#define DBG_ALL     (1)         /*!< All levels are enabled. */
/**@}*/


/**//**
 * \brief   Set of macros testing if the specific severity level is enabled.
 * \param[in]   levels      Set of enabled severity levels joined with the bitwise OR.
 * \return  TRUE if the particular severity level is included into the set \p levels;
 *  FALSE otherwise.
 */
/**@{*/
#define DBG_IS_INFO_ENABLED(levels)     (((levels) == 1) || (0 != ((levels) & (DBG_INFO))))
#define DBG_IS_WARN_ENABLED(levels)     (((levels) == 1) || (0 != ((levels) & (DBG_WARN))))
#define DBG_IS_HALT_ENABLED(levels)     (((levels) == 1) || (0 != ((levels) & (DBG_HALT))))
#define DBG_IS_FAIL_ENABLED(levels)     (((levels) == 1) || (0 != ((levels) & (DBG_FAIL))))
/**@}*/


/**//**
 * \brief   Validates asserted boolean expression for equity to TRUE at compile-time.
 * \param[in]   boolExpr    Boolean expression to be validated.
 * \details This macro produces compilation error "Division by zero attempted" which stops
 *  the make if the asserted expression \p boolExpr does not hold.
 * \details This macro may be used inside or outside function body, in source files or in
 *  header files as well, arbitrary number of times per single file or code block.
 * \details The asserted expression must be computable at compile-time.
 */
#ifdef _DEBUG_IGNORE_STATIC_ASSERTS_
#define SYS_DbgAssertStatic(boolExpr)
#else
#define SYS_DbgAssertStatic(boolExpr)\
        enum {\
            __sysDbgUID = 1 / ( !!(boolExpr) )\
        };

/*
 * Auxiliary macros for SYS_DbgAssertStatic.
 */
#define __sysDbgUID                 __sysDbgMakeUID(__COUNTER__)
#define __sysDbgMakeUID(counter)    __sysDbgMakeUID_(counter)
#define __sysDbgMakeUID_(counter)   __uid##counter
#endif /* _DEBUG_IGNORE_STATIC_ASSERTS_ */

/**//**
 * \brief   Validates asserted boolean expression for equity to TRUE at run-time, halts
 *  the application and rises error if the expression does not hold.
 * \param[in]   boolExpr    Boolean expression to be validated.
 * \param[in]   errorUid    Globally unique numeric identifier of the error message.
 * \details In general, for the Debug build, this macro validates the asserted
 *  boolean expression \p boolExpr; and if it does not hold, it halts the application and
 *  raises the error message with the code \p errorUid.
 * \details On particular platforms with different options this macro may have different
 *  behavior on:
 *  - how and if the application is halted: put into the infinite loop or restarted, etc.;
 *  - how and if the error message and some auxiliary information (the file name, the
 *    number of line in the file) is raised: printed into the stdout or stderr, or
 *    transmitted via the UART, etc.
 *
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build the \p boolExpr is evaluated only in the part that may
 *  have a side-effect; and if the whole \p boolExpr has no side-effects this macro is
 *  completely excluded (optimized) by the compiler from the binary output.
 * \note    The \p boolExpr may be considered by the compiler as having side-effects at
 *  least in the following cases:
 *  - if the result of expression evaluation (or one of its parts) is stored or may be
 *    stored in global or static or volatile variable or used somewhere else;
 *  - if a function is called or may be called from the expression;
 *  - if a global or static or volatile variable is used or may be used for expression
 *    evaluation.
 *
 * \details Use SYS_DbgAssertComplex to completely exclude the asserted expression from
 *  the binary output for the Release build (even for expressions having side-effects).
 * \details Use SYS_DbgAssertLog if a warning message shall be raised without application
 *  halting.
 * \details Use SYS_DbgHalt for unconditional halting of the application.
 */
#if defined(_DEBUG_)
#
# if defined(_DEBUG_FILELINE_)
#  define SYS_DbgAssert(boolExpr, errorUid)\
        do {\
            while (!(boolExpr)) {\
                sysDbgHalt((errorUid), __FILE__, __LINE__);\
                break;\
            }\
        } while (0)
#
# else /* ! _DEBUG_FILELINE_ */
#  define SYS_DbgAssert(boolExpr, errorUid)\
        do {\
            while (!(boolExpr)) {\
                sysDbgHalt(errorUid);\
                break;\
            }\
        } while (0)
#
# endif /* ! _DEBUG_FILELINE_ */
#
#else /* ! _DEBUG_ */
# define SYS_DbgAssert(boolExpr, errorUid)          while(boolExpr) break
#endif


/**//**
 * \brief   Validates asserted boolean expression for equity to TRUE at run-time, halts
 *  the application and rises error if the expression does not hold.
 * \param[in]   boolExpr    Boolean expression to be validated.
 * \param[in]   errorUid    Globally unique numeric identifier of the error message.
 * \details In general, for the Debug build, this macro validates the asserted boolean
 *  expression \p boolExpr; and if it does not hold, it halts the application and raises
 *  the error message with the code \p errorUid.
 * \details On particular platforms with different options this macro may have different
 *  behavior on:
 *  - how and if the application is halted: put into the infinite loop or restarted, etc.;
 *  - how and if the error message and some auxiliary information (the file name, the
 *    number of line in the file) is raised: printed into the stdout or stderr, or
 *    transmitted via the UART, etc.
 *
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build, unlike the SYS_DbgAssert macro, this macro is
 *  completely excluded (optimized) by the compiler from the output, even if the asserted
 *  expression has some side-effects.
 * \details Use SYS_DbgAssert if the asserted expression is need to be evaluated also for
 *  the Release build.
 * \details Use SYS_DbgAssertLog if a warning message shall be raised without application
 *  halting.
 * \details Use SYS_DbgHalt for unconditional halting of the application.
 */
#if defined(_DEBUG_COMPLEX_)
# define SYS_DbgAssertComplex(boolExpr, errorUid)   SYS_DbgAssert(boolExpr, errorUid)
#else
# define SYS_DbgAssertComplex(boolExpr, errorUid)   while(0)
#endif


/**//**
 * \brief   Validates asserted boolean expression for equity to TRUE at run-time, logs
 *  warning if the expression does not hold and continues the application execution.
 * \param[in]   boolExpr        Boolean expression to be validated.
 * \param[in]   warningUid      Globally unique numeric identifier of the warning message.
 * \details In general, for the Debug build, this macro validates the asserted boolean
 *  expression \p boolExpr; and if it does not hold, it logs the warning message with the
 *  code \p warningUid and continues the application execution.
 * \details On particular platforms with different options this macro may have different
 *  behavior on how and if the warning message and some auxiliary information (the file
 *  name, the number of line in the file) is logged: printed into the stdout or stderr, or
 *  transmitted via the UART, etc.
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build, just like the SYS_DbgAssertComplex macro, this macro is
 *  completely excluded (optimized) by the compiler from the output, even if the asserted
 *  expression has some side-effects.
 * \details Use SYS_DbgAssertComplex to halt the application in the case when the asserted
 *  expression is violated.
 * \details Use SYS_DbgLogId for unconditional warning signaling.
 */
#if defined(_DEBUG_LOG_)
#
# if defined(_DEBUG_FILELINE_)
#  define SYS_DbgAssertLog(boolExpr, warningUid)\
        do {\
            while (!(boolExpr)) {\
                sysDbgLogId((warningUid), __FILE__, __LINE__);\
                break;\
            }\
        } while (0)
#
# else /* ! _DEBUG_FILELINE_ */
#  define SYS_DbgAssertLog(boolExpr, warningUid)\
        do {\
            while (!(boolExpr)) {\
                sysDbgLogId(warningUid);\
                break;\
            }\
        } while (0)
#
# endif /* ! _DEBUG_FILELINE_ */
#
#else /* ! _DEBUG_LOG_ */
# define SYS_DbgAssertLog(boolExpr, warningUid)     while(0)
#endif


/**//**
 * \brief   Halts the application and rises error.
 * \param[in]   errorUid    Globally unique numeric identifier of the error message.
 * \details In general, for the Debug build, this macro unconditionally halts the
 *  application and raises the error message with the code \p errorUid.
 * \details On particular platforms with different options this macro may have different
 *  behavior on:
 *  - how and if the application is halted: put into the infinite loop or restarted, etc.;
 *  - how and if the error message and some auxiliary information (the file name, the
 *    number of line in the file) is raised: printed into the stdout or stderr, or
 *    transmitted via the UART, etc.
 *
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build this macro is completely excluded (optimized) by the
 *  compiler from the binary output.
 * \details Use SYS_DbgAssert or SYS_DbgAssertComplex to halt the application
 *  conditionally on specific asserted expression.
 */
#define SYS_DbgHalt(errorUid)       SYS_DbgAssert(FALSE, errorUid)


/**//**
 * \brief   Logs warning and continues the application execution.
 * \param[in]   warningUid      Globally unique numeric identifier of the warning message.
 * \details In general, for the Debug build, this macro unconditionally logs the warning
 *  message with the code \p warningUid and continues the application execution.
 * \details On particular platforms with different options this macro may have different
 *  behavior on how and if the warning message and some auxiliary information (the file
 *  name, the number of line in the file) is logged: printed into the stdout or stderr, or
 *  transmitted via the UART, etc.
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build this macro is completely excluded (optimized) by the
 *  compiler from the binary output.
 * \details Use SYS_DbgAssertLog to signal warning conditionally on specific asserted
 *  expression.
 * \details Use SYS_DbgLogStr to log custom formatted warning message.
 */
#define SYS_DbgLogId(warningUid)    SYS_DbgAssertLog(FALSE, warningUid)


/**//**
 * \brief   Logs custom formatted debugging string with auxiliary parameters.
 * \param[in]   va_args     The variable length comma-separated list of parameters forming
 *  the custom warning message according to the printf arguments list specification.
 * \details In general, for the Debug build, this macro unconditionally logs the custom
 *  warning message and continues the application execution.
 * \details On particular platforms with different options this macro may have different
 *  behavior on how and if the warning message is logged: printed into the stdout or
 *  stderr, or transmitted via the UART, etc.
 * \details The trailing End-of-Line symbol is appended automatically.
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build this macro is completely excluded (optimized) by the
 *  compiler from the binary output.
 * \details Use SYS_DbgLogId to log one of the registered warning messages with the
 *  specified globally unique numeric identifier.
 * \details Use SYS_DbgLogTrace to log tracing message with custom part for the application
 *  execution flow.
 */
#if (defined(_DEBUG_LOG_) && (defined(_DEBUG_STDOUTLOG_) || defined(_DEBUG_STDERRLOG_))) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
# define SYS_DbgLogStr(...)         sysDbgLogStr(__VA_ARGS__)
#else
# define SYS_DbgLogStr(...)         while(0)
#endif


/**//**
 * \brief   Logs custom tracing message and continues the application execution.
 * \param[in]   format      Pointer to the constant string specifying the format of the
 *  logged message.
 * \param[in]   va_args     The variable length comma-separated list of parameters forming
 *  the custom tracing message according to the printf arguments list specification.
 * \details In general, for the Debug build, this macro unconditionally logs the custom
 *  tracing message and continues the application execution.
 * \details On particular platforms with different options this macro may have different
 *  behavior on how and if the tracing message is logged: printed into the stdout or
 *  stderr, or transmitted via the UART, etc.
 * \details The trailing End-of-Line symbol is appended automatically.
 * \details This macro may be used just as a function call in the if-else and the same
 *  constructions.
 * \details For the Release build this macro is completely excluded (optimized) by the
 *  compiler from the binary output.
 * \details Use SYS_DbgLogStr to log custom formatted warning message.
 */
#if defined(_DEBUG_LOG_)
# define SYS_DbgLogTrace(...)\
        do {\
            sysDbgLogStr("%s: ", __FUNCTION__);\
            sysDbgLogStr(__VA_ARGS__);\
        } while(0)
#else
# define SYS_DbgLogTrace(...)       while(0)
#endif


/************************* PROTOTYPES ***************************************************/
#if defined(_DEBUG_)
# if defined(_DEBUG_FILELINE_)
/*************************************************************************************//**
 * \brief   Logs error and halts program execution.
 * \param[in]   errorUid    Error identifier to be logged.
 * \param[in]   fileName    Pointer to the constant string with the name of the file
 *  containing triggered asserted expression.
 * \param[in]   fileLine    Number of the line in the file at which the asserted
 *  expression was validated.
*****************************************************************************************/
SYS_PUBLIC void sysDbgHalt(const uint32_t errorUid, const char *const fileName, const uint32_t fileLine);


# else /* ! _DEBUG_FILELINE_ */
/*************************************************************************************//**
 * \brief   Logs error and halts program execution.
 * \param[in]   errorUid    Error identifier to be logged.
*****************************************************************************************/
SYS_PUBLIC void sysDbgHalt(const uint32_t errorUid);
# endif /* ! _DEBUG_FILELINE_ */
#endif /* _DEBUG_ */


#if defined(_DEBUG_LOG_)
# if defined(_DEBUG_FILELINE_)
/*************************************************************************************//**
 * \brief   Logs warning and proceeds with program execution.
 * \param[in]   warningUid      Warning identifier to be logged.
 * \param[in]   fileName        Pointer to the constant string with the name of the file
 *  containing triggered asserted expression.
 * \param[in]   fileLine        Number of the line in the file at which the asserted
 *  expression was validated.
*****************************************************************************************/
SYS_PUBLIC void sysDbgLogId(const uint32_t warningUid, const char *const fileName, const uint32_t fileLine);


# else /* ! _DEBUG_FILELINE_ */
/*************************************************************************************//**
 * \brief   Logs warning and proceeds with program execution.
 * \param[in]   warningUid      Warning identifier to be logged.
*****************************************************************************************/
SYS_PUBLIC void sysDbgLogId(const uint32_t warningUid);
# endif /* ! _DEBUG_FILELINE_ */
#endif /* _DEBUG_LOG_ */


#if defined(_DEBUG_LOG_)
/*************************************************************************************//**
 * \brief   Logs custom formatted debugging string with auxiliary parameters.
 * \param[in]   format      Pointer to the constant string specifying the format of the
 *  logged message.
 * \param[in]   va_args     The variable length comma-separated list of parameters forming
 *  the custom warning message according to the printf arguments list specification.
 * \details
 *  The trailing End-of-Line symbol is appended automatically.
*****************************************************************************************/
SYS_PUBLIC void sysDbgLogStr(const char *const format, ...);
#endif /* _DEBUG_LOG_ */

#ifdef __cplusplus
}
#endif

#endif /* _BB_SYS_DBG_H */