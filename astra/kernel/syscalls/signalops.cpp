/******************************************************************************
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
 *****************************************************************************/

/*
 * signalops.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: gambhire
 */

#include <cstdint>
#include "tzerrno.h"

#include "system.h"
#include "svcutils.h"
#include "tztask.h"
#include "scheduler.h"

#include "fs/ramfs.h"

void SysCalls::doKill(TzTask *currTask) {

    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    int pid = (unsigned int)arg0;
    int rv = 0;

    if (pid > 0) { // signal to a specific task pointed by pid
        TzTask *target = TzTask::taskFromId((int)arg0);
        if (target == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        if (arg1 >= TzTask::NumSignals) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        rv = target->sigQueue((int)arg1);
    }
    else { // signal to targeted process group whose pgid is -pid
        int pgid = 0;
        if (!pid) pgid = currTask->processGroup();// zero pid refers to current process group
        else pgid = -pid;

        if (arg1 >= TzTask::NumSignals) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }
        rv = ProcessGroup::signalGroup(pgid, arg1);
    }
    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}


void SysCalls::doRtSigQueueInfo(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    TzTask *target = TzTask::taskFromId((int)arg0);
    if (target == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    uint16_t currUid = currTask->owner();
    uint16_t currGid = currTask->group();
    if ((currUid != System::UID) && (currUid != target->owner())
            && (currGid != target->group())) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EPERM);
        return;
    }

    if (arg1 >= TzTask::NumSignals) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    siginfo_t kSigInfo;
    siginfo_t *uSigInfo = (siginfo_t *)arg2;
    bool rc = copyFromUser(uSigInfo, &kSigInfo);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    int rv = target->sigQueue((int)arg1, &kSigInfo);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doSigAction(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);
    unsigned long arg2 = currTask->userReg(TzTask::UserRegs::r2);

    int sigNum = (int)arg0;
    if ((sigNum <= 0) || (sigNum >= SIGRTMAX)) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    struct k_sigaction *uAction = (struct k_sigaction *)arg1;
    struct k_sigaction kAction;
    if (uAction != nullptr) {
        bool rc = copyFromUser(uAction, &kAction);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
    }

    struct k_sigaction *uOldAction = (struct k_sigaction *)arg2;
    struct k_sigaction kOldAction;
    if (uOldAction != nullptr) {
        bool rc = copyFromUser(uOldAction, &kOldAction);
        if (!rc) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
    }

    int rv = currTask->sigAction(sigNum, (uAction == nullptr) ? nullptr:&kAction, &kOldAction);

    if ((uOldAction != nullptr) && (rv == 0)) {
        copyToUser(uOldAction, &kOldAction);
    }

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doSigReturn(TzTask *currTask) {
    currTask->sigReturn();

    // sigReturn never returns
    // currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}
