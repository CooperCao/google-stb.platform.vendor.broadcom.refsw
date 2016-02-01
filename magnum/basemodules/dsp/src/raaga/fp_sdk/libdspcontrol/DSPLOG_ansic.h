/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
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

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdarg.h>
#  define BFPSDK_DSPLOG_INLINE  inline
#else
#  include "bstd_defs.h"
#  define BFPSDK_DSPLOG_INLINE
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
static BFPSDK_DSPLOG_INLINE __attribute__((unused, noreturn))
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
static BFPSDK_DSPLOG_INLINE __attribute__((unused)) \
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
static BFPSDK_DSPLOG_INLINE __attribute__((unused))
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
static BFPSDK_DSPLOG_INLINE __attribute__((unused, noreturn))
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
static BFPSDK_DSPLOG_INLINE __attribute__((unused, noreturn))
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

#undef BFPSDK_DSPLOG_INLINE


#endif  /* _DSPLOG_ANSIC_H_ */
