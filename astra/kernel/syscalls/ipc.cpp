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
 * ipc.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: gambhire
 */

#include <cstdint>
#include "tzerrno.h"

#include "tzfcntl.h"
#include "tzmman.h"
#include "fs/ramfs.h"
#include "svcutils.h"
#include "tztask.h"
#include "tzmqueue.h"
#include "tzsignal.h"

static char mqNameStr[256];

void SysCalls::doMqOpen(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);

    const char *uName = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uName, mqNameStr, 256, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 4096;
    attr.mq_curmsgs = 0;

    if (arg3 != 0) {
        bool rc = copyFromUser((mq_attr *)arg3, &attr);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }

        if (attr.mq_maxmsg * attr.mq_msgsize >
            MsgQueue::MaxNumMsgs * MsgQueue::MaxMsgSize) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
            return;
        }
    }

    int rv = currTask->mqOpen(mqNameStr, (int)arg1, (int)arg2, &attr);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMqUnlink(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    const char *uName = (const char *)arg0;
    bool truncated = false;
    bool rc = strFromUser(uName, mqNameStr, 256, &truncated);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (truncated) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENAMETOOLONG);
        return;
    }

    uint16_t uid = currTask->owner();
    uint16_t gid = currTask->group();

    MsgQueue *mq = MsgQueue::lookUp(mqNameStr);
    if (mq == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOENT);
        return;
    }

    uint16_t mqOwner = mq->uid();
    uint16_t mqGroup = mq->gid();
    int accessMode = mq->access();

    if ((uid != mqOwner) && (gid != mqGroup)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EACCES);
        return;
    }
    if ((uid != mqOwner) && (!(GROUP_ACCESS_PERMS(accessMode) & PERMS_WRITE_BIT))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EACCES);
        return;
    }

    rc = MsgQueue::unlink(mqNameStr);
    if (!rc)
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOENT);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doMqTimedSend(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);

    int fd = (int)arg0;
    size_t size = (size_t)arg2;

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg1);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, size, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;
    const char *msgPtr = (const char *)buffer;

    int prio = (int)arg3;
    int rv = currTask->mqSend(fd, msgPtr, size, prio, -1);

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMqTimedRecv(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);
    unsigned long arg3 = currTask->userReg(TzTask::UserRegs::r3);
    unsigned long arg4 = currTask->userReg(TzTask::UserRegs::r4);

    int fd = (int)arg0;
    size_t size = (size_t)arg2;

    int numPages = 0;
    TzMem::VirtAddr userBuffer = (TzMem::VirtAddr)(arg1);
    TzMem::VirtAddr kva = mapToKernel(userBuffer, size, &numPages);
    if (kva == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    int pageOffset = (uint8_t *)userBuffer - (uint8_t *)PAGE_START_4K(userBuffer);
    uint8_t *buffer = (uint8_t *)kva + pageOffset;
    char *msgPtr = (char *)buffer;

    int prio = 0;

    struct timespec ts;
    uint64_t timeout;
    bool rc = copyFromUser((struct timespec *)arg4, &ts);
    if (!rc) {
        timeout = -1;
    }
    else {
        const unsigned long hz = TzHwCounter::frequency();
        const unsigned long nsPerTick = 1000000000UL/hz;
        timeout = (ts.tv_sec * hz) + (ts.tv_nsec/nsPerTick);
    }

    int rv = currTask->mqRecv(fd, msgPtr, size, &prio, timeout);
    if (rv > 0) {
        copyToUser((int *)arg3, &prio);
    }

    unmap(kva, numPages);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMqNotify(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    int fd = (int)arg0;
    struct sigevent *uSigEvent = (struct sigevent *)arg1;

    struct sigevent kSigEvent;
    bool rc = copyFromUser(uSigEvent, &kSigEvent);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    if ((kSigEvent.sigev_signo < 0) || (kSigEvent.sigev_signo > SIGRTMAX)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    int rv = currTask->mqNotify(fd, &kSigEvent);
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doMqGetSetAttr(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    mq_attr kNewAttr, kOldAttr;
    mq_attr *uNewAttr, *uOldAttr;

    int fd = (int)arg0;
    uNewAttr = (mq_attr *)arg1;
    uOldAttr = (mq_attr *)arg2;

    bool rc = copyFromUser(uNewAttr, &kNewAttr);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    int rv = currTask->mqAttrChange(fd, &kNewAttr, &kOldAttr);
    if (rv == 0) {
        copyToUser(uOldAttr, &kOldAttr);
    }
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}
