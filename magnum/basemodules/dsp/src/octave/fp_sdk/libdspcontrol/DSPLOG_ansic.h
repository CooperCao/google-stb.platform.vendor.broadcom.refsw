/****************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/*
 * This is a mostly API-compatible replacement for the DSPLOG.h header file
 * to be used when strict ANSI C (C89-C90) compliance is required.
 *
 * The current implementation if far from optimal.
 */

#ifndef _DSPLOG_ANSIC_H_
#define _DSPLOG_ANSIC_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/UTIL_c.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdarg.h>
#  define BFPSDK_DSPLOG_ATTRS   BFPSDK_STATIC_ALWAYS_INLINE
#else
#  include "bstd_defs.h"
/* Some STB refsw toolchains complain that
 * "sorry, unimplemented: function 'FOO' can never be inlined because it uses variable argument lists"
 * so remove the functions inlining. */
#  define BFPSDK_DSPLOG_ATTRS   static
#endif



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Underlying log function. Don't use this directly,
 * call the provided macros instead.
 *
 * @param filename  name of the file originating this log message
 * @param line      line in the file originating this log message
 * @param log_level level of the message
 * @param fmt       format string, printf-like
 * @param va_args   printf-like arguments
 */
#if !FEATURE_IS(DSPLOG_SINK, NULL)
void DSPLOG_impl_va(const char *filename,
                    const int line,
                    const int log_level,
                    const char *fmt,
                    va_list va_args);
#else
#  define DSPLOG_impl_va(filename, line, log_level, fmt, va_args)   { (void) filename; (void) line; (void) log_level; (void) fmt; (void) va_args; }
#endif


/**
 * Underlying log function. Don't use this directly,
 * call the provided macros instead.
 *
 * @param filename  name of the file originating this log message
 * @param line      line in the file originating this log message
 * @param log_level level of the message
 * @param fmt       format string, printf-like
 */
#if !FEATURE_IS(DSPLOG_SINK, NULL)
void DSPLOG_impl(const char *filename,
                 const int line,
                 const int log_level,
                 const char *fmt,
                 ...);
#else
BFPSDK_DSPLOG_ATTRS __attribute__((unused, noreturn))
void DSPLOG_impl(const char *filename,
                 const int line,
                 const int log_level,
                 const char *fmt,
                 ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    DSPLOG_impl_va(filename, line, log_level, fmt, va_args);
    va_end(va_args);
}
#endif


#define DECLARE_DSPLOG_MACRO(loglevel)              \
BFPSDK_DSPLOG_ATTRS __attribute__((unused)) \
void DSPLOG_##loglevel(const char *fmt, ...)        \
{                                                   \
    va_list va_args;                                \
    va_start(va_args, fmt);                         \
    DSPLOG_impl_va("???", 0,                        \
                   DSPLOG_##loglevel##_LEVEL,       \
                   fmt, va_args);                   \
    va_end(va_args);                                \
}

/**
 * Macro to log at FAILURE LEVEL
 * @param ...  printf-style format string and arguments.
 */
DECLARE_DSPLOG_MACRO(FAILURE)
/** Macro to log at ERROR LEVEL @see #DSPLOG_FAILURE() */
DECLARE_DSPLOG_MACRO(ERROR)
/** Macro to log at INFO LEVEL @see #DSPLOG_FAILURE() */
DECLARE_DSPLOG_MACRO(INFO)
/** Macro to log at DETAIL LEVEL @see #DSPLOG_FAILURE() */
DECLARE_DSPLOG_MACRO(DETAIL)
/** Macro to log at DEBUG LEVEL @see #DSPLOG_FAILURE() */
DECLARE_DSPLOG_MACRO(DEBUG)
/** Macro to log at JUNK LEVEL @see #DSPLOG_FAILURE() */
DECLARE_DSPLOG_MACRO(JUNK)

/**
 * Variable parameters macro for logging, permits to specify log level.
 * @param log_level log message level
 * @param fmt       printf-style format string
 * @param ...       format string arguments
 */
BFPSDK_DSPLOG_ATTRS __attribute__((unused))
void DSPLOG_LOG(int log_level, const char *fmt, ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    DSPLOG_impl_va("???", 0, log_level, fmt, va_args);
    va_end(va_args);
}


/**
 * Log a failure and then exit with code 1.
 *
 * @param fmt  printf-style format string
 * @param ...  format string arguments
 */
BFPSDK_DSPLOG_ATTRS __attribute__((unused, noreturn))
void FATAL_ERROR(const char *fmt, ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    do
    {
        DSPLOG_impl_va("???", 0, DSPLOG_FAILURE_LEVEL, fmt, va_args);
        DSPLOG_EXIT_FUNC(1);
    } while(0);
    va_end(va_args);
}


/**
 * Log a failure and then exit with code 1.
 *
 * @param code the process return code
 * @param fmt  printf-style format string
 * @param ...  format string arguments
 */
BFPSDK_DSPLOG_ATTRS __attribute__((unused, noreturn))
void FATAL_ERROR_CODE(int code, const char *fmt, ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    do
    {
        DSPLOG_impl_va("???", 0, DSPLOG_FAILURE_LEVEL, fmt, va_args);
        DSPLOG_EXIT_FUNC(code);
    } while(0);
    va_end(va_args);
}


#ifdef __cplusplus
}
#endif

#undef BFPSDK_DSPLOG_ATTRS


#endif  /* _DSPLOG_ANSIC_H_ */
