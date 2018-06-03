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

#include "libdspcontrol/CHIP.h"

#if !(FEATURE_IS(SW_HOST, LINUX)          || \
      FEATURE_IS(SW_HOST, RAAGA_ROCKFORD) || \
      IS_TARGET(RaagaFP4015_si_magnum_permissive) || \
      IS_HOST(BM)                         || \
      IS_HOST(DSP_LESS))
#  error "This module is only for Linux and Raaga Rockford (and you are not building for BM or workstation either)"
#endif
#if FEATURE_IS(FILE_IO, UNAVAILABLE)
#  error "File I/O not available for this SoC"
#endif


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libdspcontrol/OS.h"



int OS_open(const char *path, int oflag)
{
    int res;

    do
    {
        res = open(path, oflag, 0644);
    } while((res < 0) && ((errno == EINTR) || (errno == EAGAIN)));

    return res;
}


ssize_t OS_read(int fd, void *buf, size_t nbyte)
{
    ssize_t res;

    do
    {
        res = read(fd, buf, nbyte);
    } while((res < 0) && ((errno == EINTR) || (errno == EAGAIN)));

    return res;
}



ssize_t OS_write(int fd, const void *buf, size_t nbyte)
{
    ssize_t res;

    do
    {
        res = write(fd, buf, nbyte);
    } while((res < 0) && ((errno == EINTR) || (errno == EAGAIN)));

    return res;
}


/* not all systems have OS_sleep */
#if FEATURE_IS(SW_HOST, LINUX)    || \
    FEATURE_IS(SW_HOST, FREERTOS) || \
    FEATURE_IS(SW_HOST, RAAGA_ROCKFORD)
#  define IF_OS_WRITEALL_CAN_SLEEP(...)     __VA_ARGS__
#else
#  define IF_OS_WRITEALL_CAN_SLEEP(...)
#endif


size_t OS_writeAll(int fd, const void *buff, size_t nbytes, unsigned max_attempts,
                   unsigned ms_before_retry __attribute__((unused)), ssize_t *last_ret_value)
{
    size_t written_bytes = 0;
    char *cur_buf_ptr = (char *) buff;
    while(nbytes > 0)
    {
        ssize_t ret_value = OS_write(fd, cur_buf_ptr, nbytes);
        if(last_ret_value != NULL)
            *last_ret_value = ret_value;

        if(ret_value > 0)
        {
            written_bytes += ret_value;
            nbytes -= ret_value;
            cur_buf_ptr += ret_value;
            if(nbytes == 0)
                break;      /* done, no more attempts required */
        }
        else    /* error or no written bytes */
        {
            if((max_attempts--) <= 1)
                break;  /* out of attempts */

IF_OS_WRITEALL_CAN_SLEEP(
            if(ms_before_retry > 0)
                OS_sleep(ms_before_retry);)
        }
    }

    return written_bytes;
}


int OS_close(int fd)
{
    int res;

    do
    {
        res = close(fd);
    } while((res < 0) && ((errno == EINTR) || (errno == EAGAIN)));

    return res;
}


off_t OS_lseek(int fd, off_t offset)
{
    return lseek(fd, offset, SEEK_SET);
}
