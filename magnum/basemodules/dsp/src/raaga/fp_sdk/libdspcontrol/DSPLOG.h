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
 *****************************************************************************/

/**
 * @file
 * @ingroup libdspcontrol
 * @brief Logging module.
 *
 * Allow programs to easily output log messages.
 * The default destination of messages is stderr, but it can be changed using #DSPLOG_setSink.
 * Each message has an associated log level; if it is smaller or equal to the level set
 * through #DSPLOG_setLevel it is written to destination, otherwise it is discarded.
 * Some service information can be prepended to each actual message. It's up to the user to
 * decide, using the #DSPLOG_setFormat, if to show
 * - the target system name
 * - the date and time the message was generated
 * - the source file name and the line where the message originates
 * .
 * The underlying logging function is #DSPLOG_impl; however, users are advised to use the provided
 * #DSPLOG_FAILURE(), #DSPLOG_ERROR(), #DSPLOG_INFO(), #DSPLOG_DETAIL(), #DSPLOG_DEBUG(), #DSPLOG_JUNK()
 * or the generic #DSPLOG_LOG() macros.
 * The #FATAL_ERROR macro should be used when the program is supposed to output an error message and then exit.
 *
 * #DSPLOG_memdump is an utility function that logs a hexadecimal dump of a byte array on a single line.
 */

#ifndef _DSPLOG_H_
#define _DSPLOG_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdarg.h>
#  include <stdbool.h>
#  include <stdint.h>
#  include <stdio.h>
#else
#  include "bstd.h"
#endif



#ifdef __cplusplus
extern "C"
{
#endif


/*
 * Log levels
 */
/** Placeholder for 'not set' variables / 'unknown' values. */
#define DSPLOG_UNKNOWN_LEVEL    (-1)
/** Nothing gets logged, the DSPLOG module functions are silent. */
#define DSPLOG_NOTHING_LEVEL    0
/** Serious errors that impede program continuation. */
#define DSPLOG_FAILURE_LEVEL    1
/** Recoverable errors. */
#define DSPLOG_ERROR_LEVEL      2
/** Messages giving a general overview of the program execution. */
#define DSPLOG_INFO_LEVEL       3
/** Messages giving a detailed insight of the program execution. */
#define DSPLOG_DETAIL_LEVEL     4
/** Detailed technical information, used for debug purposes. */
#define DSPLOG_DEBUG_LEVEL      5
/** Extremely verbose information about program evolution, used for debug purposes. */
#define DSPLOG_JUNK_LEVEL       6
/** Minimum valid value for log level */
#define DSPLOG_MIN_LEVEL        DSPLOG_NOTHING_LEVEL
/** Maximum valid value for log level */
#define DSPLOG_MAX_LEVEL        DSPLOG_JUNK_LEVEL


/*
 * Defaults
 */
/** Log level default value at startup. */
#define DSPLOG_DEFAULT_LEVEL            DSPLOG_INFO_LEVEL
/** Default message destination at startup. */
#define DSPLOG_DEFAULT_SINK             stderr
/** Format flag default value at startup. */
#define DSPLOG_DEFAULT_SHOWTARGET       false
/** Format flag default value at startup. */
#define DSPLOG_DEFAULT_SHOWTIME         false
/** Format flag default value at startup. */
#define DSPLOG_DEFAULT_SHOWERRORTYPE    true
/** Format flag default value at startup. */
#define DSPLOG_DEFAULT_SHOWFILENAME     true


/* Documentation for wrapper macros (defined elsewhere) */
/**
 * @def DSPLOG_FAILURE(...)
 * Macro to log at DSPLOG_FAILURE_LEVEL
 * @param ...  printf-style format string and arguments.
 */
/**
 * @def DSPLOG_ERROR(...)
 * Macro to log at DSPLOG_ERROR_LEVEL
 * @param ...  printf-style format string and arguments.
 */
/**
 * @def DSPLOG_INFO(...)
 * Macro to log at DSPLOG_INFO_LEVEL
 * @param ...  printf-style format string and arguments.
 */
/**
 * @def DSPLOG_DETAIL(...)
 * Macro to log at DSPLOG_DETAIL_LEVEL
 * @param ...  printf-style format string and arguments.
 */
/**
 * @def DSPLOG_DEBUG(...)
 * Macro to log at DSPLOG_DEBUG_LEVEL
 * @param ...  printf-style format string and arguments.
 */
/**
 * @def DSPLOG_JUNK(...)
 * Macro to log at DSPLOG_JUNK_LEVEL
 * @param ...  printf-style format string and arguments.
 */


/* Define a cross-platform exit macro */
#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)

#  include <bkni.h>
#  define DSPLOG_EXIT_FUNC(code)    do { BSTD_UNUSED(code); BKNI_Fail(); } while (0)

#elif FEATURE_IS(LIBC, FREESTANDING)

/* This is not ideal, but there are cases where we can't really exit anywhere
 * and branching to NULL (by default) makes enough noise for somebody to notice */
typedef void (*DSPLOG_EXIT_FUNC_T)(int status);
extern DSPLOG_EXIT_FUNC_T DSPLOG_EXIT_FUNC;
void DSPLOG_setExitFunction(DSPLOG_EXIT_FUNC_T new_func);

#elif FEATURE_IS(LIBC, HOSTED)

#  include <stdlib.h>
#  define DSPLOG_EXIT_FUNC(code)    exit(code)

#else
#  error "Unable to define the DSPLOG_EXIT_FUNC macro"
#endif


/**
 * Allow user to configure log level.
 *
 * @param new_level  the new minimum level to be logged
 */
void DSPLOG_setLevel(int new_level);


#if FEATURE_IS(DSPLOG_SINK, STDIO)
/**
 * Configure log messages destination stream.
 *
 * @param new_sink  the new target for log messages, NULL to rollback to the default
 */
void DSPLOG_setFileSink(FILE *new_sink);
#endif


#if FEATURE_IS(DSPLOG_SINK, MEMORY)
/**
 * Configure log messages destination buffer.
 *
 * NOTE: the logging on memory functionality is still very basic.
 *       Once the buffer is full no more prints will be generated.
 *       Traversing the buffer is up to the user.
 *
 * @param new_buffer   buffer where to write '\x0'-separated log messages
 * @param buffer_size  size of new_buffer in bytes
 */
void DSPLOG_setMemorySink(uint8_t *new_buffer, size_t buffer_size);
#endif


/**
 * Allow user to configure log messages prefixes.
 *
 * @param new_show_target    whether to show a leading target name in log messages
 * @param new_show_time      whether to show time in log messages
 * @param new_show_filename  whether to show originating filename in log messages
 */
void DSPLOG_setFormat(bool new_show_target, bool new_show_time, bool new_show_filename);


/**
 * Dumps an array of space-separated bytes in hexadecimal on a single text line.
 *
 * @param log_level  level used for the message
 * @param prefix     the line prefix
 * @param bytes      pointer to the data to be dumped
 * @param len        number of bytes to dump
 */
void DSPLOG_memdump(int log_level, const char *prefix, const uint8_t *bytes, unsigned len);


/**
 * Dumps an array of space-separated bytes in hexadecimal on a single text line.
 * Continues on a new line every bytes_per_line.
 *
 * @param log_level       level used for the message
 * @param prefix          the line prefix
 * @param bytes           pointer to the data to be dumped
 * @param len             number of bytes to dump
 * @param bytes_per_line  maximum number of bytes per line
 */
void DSPLOG_memdump_wrap(int log_level, const char *prefix, const uint8_t *bytes, unsigned len, unsigned bytes_per_line);


#ifdef __cplusplus
}
#endif



#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include "DSPLOG_ansic.h"
#else
#  include "DSPLOG_gnu99.h"
#endif


#endif  /* _DSPLOG_H_ */
