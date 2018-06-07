/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

/*
 * Original full-featured DSPLOG header, included when a non-limited runtime
 * environment and a C GNU99 compliant compiler are available.
 */

#ifndef _DSPLOG_GNU99_H_
#define _DSPLOG_GNU99_H_



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
#  define DSPLOG_impl_va(filename, line, log_level, fmt, va_args)
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
__attribute__((format(printf, 4, 5)))
void DSPLOG_impl(const char *filename,
                 const int line,
                 const int log_level,
                 const char *fmt,
                 ...);
#else
#  define DSPLOG_impl(filename, line, log_level, fmt, ...)
#endif


/**
 * Macro to log at FAILURE LEVEL
 * @param ...  printf-style format string and arguments.
 */
#define DSPLOG_FAILURE(...) DSPLOG_impl(__FILE__, __LINE__, DSPLOG_FAILURE_LEVEL, __VA_ARGS__)
/** Macro to log at ERROR LEVEL @see #DSPLOG_FAILURE() */
#define DSPLOG_ERROR(...)   DSPLOG_impl(__FILE__, __LINE__, DSPLOG_ERROR_LEVEL,   __VA_ARGS__)
/** Macro to log at INFO LEVEL @see #DSPLOG_FAILURE() */
#define DSPLOG_INFO(...)    DSPLOG_impl(__FILE__, __LINE__, DSPLOG_INFO_LEVEL,    __VA_ARGS__)
/** Macro to log at DETAIL LEVEL @see #DSPLOG_FAILURE() */
#define DSPLOG_DETAIL(...)  DSPLOG_impl(__FILE__, __LINE__, DSPLOG_DETAIL_LEVEL,  __VA_ARGS__)
/** Macro to log at DEBUG LEVEL @see #DSPLOG_FAILURE() */
#define DSPLOG_DEBUG(...)   DSPLOG_impl(__FILE__, __LINE__, DSPLOG_DEBUG_LEVEL,   __VA_ARGS__)
/** Macro to log at JUNK LEVEL @see #DSPLOG_FAILURE() */
#define DSPLOG_JUNK(...)    DSPLOG_impl(__FILE__, __LINE__, DSPLOG_JUNK_LEVEL,    __VA_ARGS__)

/**
 * Variable parameters macro for logging, permits to specify log level.
 * @param log_level log message level
 * @param ...       printf-style format string and arguments.
 */
#define DSPLOG_LOG(log_level, ...)      DSPLOG_impl(__FILE__, __LINE__, log_level, __VA_ARGS__)


/**
 * Log a failure and then exit with code 1.
 *
 * @param ...  printf-style format string and arguments.
 */
#define FATAL_ERROR(...)                        \
    do {                                        \
        DSPLOG_FAILURE(__VA_ARGS__);            \
        DSPLOG_EXIT_FUNC(1);                    \
    } while (0)


/**
 * Log a failure and then exit with specified code
 *
 * @param code  the process return code
 * @param ...   printf-style format string and arguments.
 */
#define FATAL_ERROR_CODE(code, ...)             \
    do {                                        \
        DSPLOG_FAILURE(__VA_ARGS__);            \
        DSPLOG_EXIT_FUNC(code);                 \
    } while (0)


#ifdef __cplusplus
}
#endif


#endif  /* _DSPLOG_GNU99_H_ */
