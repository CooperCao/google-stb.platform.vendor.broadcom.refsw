/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/**
 * @file
 * @ingroup libdspcontrol
 * @brief OS abstraction layer.
 *
 * The OS module offers a common interface towards base operating system APIs,
 * wrapping common system calls and introducing some utility function.
 */

#ifndef _OS_H_
#define _OS_H_

#include "libdspcontrol/CHIP.h"



#ifdef __cplusplus
extern "C"
{
#endif


#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM) && !IS_TARGET(RaagaFP4015_si_magnum_permissive)
#  error "This module is unsuitable for Raaga Magnum"
#endif


#if FEATURE_IS(SW_HOST, LINUX)    || \
    FEATURE_IS(SW_HOST, FREERTOS) || \
    IS_TARGET(RaagaFP4015_si_magnum_permissive) || \
    FEATURE_IS(SW_HOST, RAAGA_ROCKFORD)

/**
 * Sleep for a short period of time.
 *
 * @param time_ms number of milliseconds to sleep.
 */
void OS_sleep(unsigned int time_ms);


/**
 * Convert current wall clock time to a string using strftime.
 * Where strftime is not available, the @a format parameter is ignored and a generic
 * measure of the elapsed time is used.
 *
 * @param dst     buffer to hold the returned string
 * @param n       length of the @a dst buffer
 * @param format  string for strftime, ignored if not supported
 * @return the number of bytes placed in @a dst, not including the terminating null character
 */
size_t OS_timeAsString(char *dst, size_t n, char *format);

#endif  /* FEATURE_IS(SW_HOST, LINUX) || FEATURE_IS(SW_HOST, FREERTOS) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD) || IS_TARGET(RaagaFP4015_si_magnum_permissive) ||*/



#if FEATURE_IS(SW_HOST, LINUX)          || \
    FEATURE_IS(SW_HOST, RAAGA_ROCKFORD) || \
    IS_TARGET(RaagaFP4015_si_magnum_permissive) || \
    IS_HOST(BM)                         || \
    IS_HOST(DSP_LESS)

#include <stddef.h>
#include <sys/types.h>



#if FEATURE_IS(FILE_IO, AVAILABLE)

/**
 * Open a file, retrying if the result is EINTR or EAGAIN.
 * Wrapper around the open Posix function, matching arguments and return value.
 */
int OS_open(const char *path, int oflag);


/**
 * Read from file, retrying if the result is EINTR or EAGAIN.
 * Wrapper around the read Posix function, matching arguments and return value.
 */
ssize_t OS_read(int fd, void *buf, size_t nbyte);


/**
 * Write to file, retrying if the result is EINTR or EAGAIN.
 * Wrapper around the write Posix function, matching arguments and return value.
 */
ssize_t OS_write(int fd, const void *buf, size_t nbyte);


/**
 * Utility function around the OS_write API, tries to write the whole buffer
 * to the given file descriptor. Gives up after the maximum number of retries.
 * If ms_before_retry is not zero, waits before each retry.
 *
 * @param[in]  fd               file descriptor to write to
 * @param[in]  buff             buffer containing the data to write
 * @param[in]  nbytes           number of bytes to write from buff
 * @param[in]  max_attempts     give up after this number of writes
 * @param[in]  ms_before_retry  if not 0, sleep this number of ms before retrying
 * @param[out] last_ret_value   if not NULL, the last OS_write return value is saved here
 * @return                      the number of successfully written bytes
 */
size_t OS_writeAll(int fd, const void *buf, size_t nbytes, unsigned max_attempts,
                   unsigned ms_before_retry, ssize_t *last_ret_value);


/**
 * Close a file, retrying if the result is EINTR or EAGAIN.
 * Wrapper around the close Posix function, matching arguments and return value.
 */
int OS_close(int fd);


/**
 * Seek to a given offset in the file.
 * Wrapper around the lseek Posix function, matching arguments and return value.
 */
off_t OS_lseek(int fd, off_t offset);

#endif  /* FEATURE_IS(FILE_IO, AVAILABLE) */


/**
 * Search for a file using the PATH environment variable.
 *
 * Before checking $PATH, the function tries to open the given
 * filename directly, so that absolute pathnames or files in the
 * current working directory will be resolved.
 *
 * If that check fails, the file name is prepended with each element
 * of $PATH in turn until a match is found.  If there is no match, an
 * empty string is returned.
 *
 * @param file          name of the file to search for
 * @param result        buffer to hold result pathname
 * @param maxResultLen  length of @a result buffer
 */
void OS_pathSearch(const char *file, char *result, int maxResultLen);


/**
 * Spawn a new process and create a pipe to its standard input.
 *
 * @param path       file path of the program to spawn
 * @param argv       arguments to program (0th arg is name, terminate with NULL)
 * @param child_pid  spawned process PID; pass NULL if not interested
 * @return file descriptor of pipe to new program's stdin
 */
int OS_spawn(const char *path, char * const argv[], pid_t *child_pid);


/**
 * Get command line used to launch process with specified PID.
 *
 * @param pid process to retrieve command line
 * @param result buffer to hold the command line
 * @param maxResultLen length of @a result buffer
 * @return the number of bytes required to contain the whole command line, -1 on error
 */
long OS_getCmdLine(pid_t pid, char *result, int maxResultLen);


/**
 * Get PID of the parent process of pid.
 *
 * @param pid  the "child" process PID
 * @return PID of the parent of pid, -1 on error
 */
pid_t OS_getParentPid(pid_t pid);

#endif  /* FEATURE_IS(SW_HOST, LINUX) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD) || IS_HOST(BM) || IS_HOST(DSP_LESS) || IS_TARGET(RaagaFP4015_si_magnum_permissive) || */


#ifdef __cplusplus
}
#endif


#endif  /* _OS_H_ */
