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
 * ipcops.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: gambhire
 */

#include "tzfcntl.h"
#include "arm/arm.h"
#include "arm/spinlock.h"

#include "system.h"
#include "tztask.h"
#include "tzmemory.h"
#include "tzmman.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"

#include "lib_printf.h"

int TzTask::mqOpen(const char *mqName, int oflag, int mode, mq_attr *attr) {

    // Make sure there's a free file descriptor slot
    int fd = 0;
    while ((fd < MAX_FD) && (files[fd].data != nullptr))
        fd++;
    if (fd == MAX_FD)
        return -ENFILE;

    MsgQueue *mq = MsgQueue::lookUp(mqName);
    if (mq == nullptr) {
        if (oflag & O_CREAT) {
            mq = MsgQueue::open(mqName, mode, attr, uid, gid);
        }
    }
    else {
        if ((oflag & O_EXCL) && (oflag & O_CREAT))
            return -EEXIST;
    }

    uint16_t owner = mq->uid();
    uint16_t group = mq->gid();
    int perms = mq->access();

    int rc = 0;
    if ((owner == uid) || (uid == System::UID)) {
        if ((mode & R_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(OWNER_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }
    else if (group == gid) {
        if ((mode & R_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(GROUP_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }
    else {
        if ((mode & R_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_READ_BIT)))
            rc = -EACCES;
        if ((mode & W_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_WRITE_BIT)))
            rc = -EACCES;
        if ((mode & X_OK) && (!(OTHERS_ACCESS_PERMS(perms) & PERMS_EXECUTE_BIT)))
            rc = -EACCES;
    }
    if (rc != 0)
        return rc;

    files[fd].type = MQueue;
    files[fd].queue = mq;
    files[fd].read = ((oflag & O_RW_MASK) == O_RDONLY) || (oflag & O_RDWR);
    files[fd].write = (oflag & O_WRONLY) || (oflag & O_RDWR);
    files[fd].closeOnExec = (oflag & O_CLOEXEC);

    return fd;
}

int TzTask::mqSend(int fd, const char *msg, size_t msgSize, int prio, uint64_t timeout) {

    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    MsgQueue *queue = files[fd].queue;
    if (queue == nullptr)
        return -ENOENT;

    if (!files[fd].write)
        return -EBADF;

    UNUSED(timeout);

    return queue->send(msg, msgSize, prio);
}

int TzTask::mqRecv(int fd, char *msg, size_t msgSize, int *prio, uint64_t timeout) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    MsgQueue *queue = files[fd].queue;
    if (queue == nullptr)
        return -ENOENT;

    if (!files[fd].read)
        return -EBADF;

    return queue->recv(msg, msgSize, (unsigned int *)prio, timeout);
}

int TzTask::mqNotify(int fd, struct sigevent *ev) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    MsgQueue *queue = files[fd].queue;
    if (queue == nullptr)
        return -ENOENT;

    if (!files[fd].read)
        return -EBADF;

    switch (ev->sigev_notify) {
    case SIGEV_NONE:
        queue->removeListener();
        return 0;

    case SIGEV_SIGNAL:
        if (!queue->addListener(this, ev->sigev_signo, ev->sigev_value))
            return -EBUSY;
        return 0;

    case SIGEV_THREAD:
        return -ENOTSUP;

    default:
        return -EINVAL;
    }
}

int TzTask::mqAttrChange(int fd, mq_attr *newAttr, mq_attr *oldAttr) {
    if ((fd >= MAX_FD) || (fd < 0))
        return -ENOENT;

    MsgQueue *queue = files[fd].queue;
    if (queue == nullptr)
        return -ENOENT;

    if (!files[fd].read)
        return -EBADF;

    if ((newAttr->mq_flags != 0) && (newAttr->mq_flags != O_NONBLOCK))
        return -EINVAL;

    queue->changeAttr(newAttr, oldAttr);
    return 0;
}
