/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/*
 * taskfileops.cpp
 *
 *  Created on: Jan 28, 2015
 *      Author: gambhire
 */

#include <hwtimer.h>
#include <ioctl.h>

#include "arm/spinlock.h"

#include "tztask.h"
#include "tzmemory.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"
#include "eventqueue.h"

#include "lib_printf.h"

int TzTask::poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    EventQueue eq;
    for (nfds_t i=0; i<nfds; i++) {
        fds[i].revents = 0;

        int fd = fds[i].fd;
        if (files[fd].type == File) {
            IFile *file = files[fd].file;
            if (file == nullptr) {
                fds[i].revents = POLLNVAL;
                continue;
            }
            file->addWatcher(fds[i].events, &fds[i].revents, &eq);
        }
        else if (files[fd].type == MQueue) {
            MsgQueue *queue = files[fd].queue;
            if (queue == nullptr) {
                fds[i].revents = POLLNVAL;
                continue;
            }
        }
        else {
            fds[i].revents = POLLNVAL;
            continue;
        }
    }

    if (timeout != 0)
        eq.wait(this, timeout);

    int rv = 0;
    for (nfds_t i=0; i<nfds; i++) {
        if (fds[i].revents != 0)
            rv++;
    }

    return rv;
}

ssize_t TzTask::writev(int fd, const iovec *iov, int iovcnt) {
    if (fd >= MAX_FD)
        return -EBADF;

    IFile *file = files[fd].file;
    if (file == nullptr)
        return -EBADF;

    ssize_t rv = file->writev(iov, iovcnt, files[fd].offset);
    if (rv > 0)
        files[fd].offset += rv;

    return rv;
}

ssize_t TzTask::readv(int fd, const iovec *iov, int iovcnt) {
    if (fd >= MAX_FD)
        return -EBADF;

    IFile *file = files[fd].file;
    if (file == nullptr)
        return -EBADF;

    ssize_t rv = file->readv(iov, iovcnt, files[fd].offset);
    if (rv > 0)
        files[fd].offset += rv;

    return rv;
}
