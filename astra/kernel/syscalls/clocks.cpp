/******************************************************************************
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
 *****************************************************************************/

/*
 * clocks.cpp
 *
 *  Created on: Feb 9, 2015
 *      Author: gambhire
 */

#include "tzerrno.h"
#include "tztask.h"
#include "clock.h"
#include "svcutils.h"


uint64_t System::cpuBootedAt[MAX_NUM_CPUS];

uint64_t TzClock::RealTime::timeBase;
uint64_t TzClock::RealTime::currAdjustment;
uint64_t TzClock::RealTime::adjustmentStep;
uint64_t TzClock::RealTime::lastAdjustedAt;
int TzClock::RealTime::numAdjSteps;

uint64_t TzClock::Monotonic::currAdjustment;
uint64_t TzClock::Monotonic::adjustmentStep;
uint64_t TzClock::Monotonic::lastAdjustedAt;
int TzClock::Monotonic::numAdjSteps;

void TzClock::init() {
    TzClock::RealTime::init();
    TzClock::Monotonic::init();
}

TzClock::RealTime::RealTime() {
}

TzClock::RealTime::~RealTime() {
}

void TzClock::RealTime::init() {
    timeBase = 0;
    currAdjustment = 0;
    adjustmentStep = 0;
    lastAdjustedAt = 0;
    numAdjSteps = 0;
}

void TzClock::RealTime::resolution(timespec *ts) {
    unsigned long hz = TzHwCounter::frequency();

    const long nsPerTick = (long)1000000000L/hz;

    ts->tv_sec = 0;
    ts->tv_nsec = nsPerTick;

}

void TzClock::RealTime::time(timespec *ts) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    // Translate raw ticks to number of ticks from epoch start.
    now += timeBase;

    // Check and apply any pending adjustments
    if (numAdjSteps > 0) {
        uint64_t delta = lastAdjustedAt - now;
        if (delta >= adjustmentStep) {
            now += delta;
            lastAdjustedAt = now;
            numAdjSteps--;
        }
    }

    ts->tv_sec = now/hz;
    ts->tv_nsec = (now - (ts->tv_sec * hz)) * nsPerTick;
}

void TzClock::RealTime::setTime(timespec *ts) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    uint64_t newTicks = (ts->tv_sec*hz) + (ts->tv_nsec/nsPerTick);
    timeBase = newTicks - now;
}

void TzClock::RealTime::adjustTime(timespec *delta, timespec *oldDelta) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    numAdjSteps = delta->tv_sec + 1;
    const uint64_t ticksInDelta = (delta->tv_sec * hz) + (delta->tv_nsec/nsPerTick);
    adjustmentStep = ticksInDelta/numAdjSteps;
    lastAdjustedAt = now;

    oldDelta->tv_sec = currAdjustment/hz;
    oldDelta->tv_nsec = (currAdjustment - (oldDelta->tv_sec * hz)) * nsPerTick;

    currAdjustment = ticksInDelta;
}

TzClock::Monotonic::Monotonic() {
}

TzClock::Monotonic::~Monotonic() {

}

void TzClock::Monotonic::init() {
    currAdjustment = 0;
    adjustmentStep = 0;
    lastAdjustedAt = 0;
    numAdjSteps = 0;
}

void TzClock::Monotonic::resolution(timespec *ts) {
    TzClock::RealTime::resolution(ts);
}

void TzClock::Monotonic::time(timespec *ts) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    // Check and apply any pending adjustments
    if (numAdjSteps > 0) {
        uint64_t delta = lastAdjustedAt - now;
        if (delta >= adjustmentStep) {
            now += delta;
            lastAdjustedAt = now;
            numAdjSteps--;
        }
    }

    ts->tv_sec = now/hz;
    ts->tv_nsec = (now - (ts->tv_sec * hz)) * nsPerTick;
}

void TzClock::Monotonic::rawTime(timespec *ts) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    ts->tv_sec = now/hz;
    ts->tv_nsec = (now - (ts->tv_sec * hz)) * nsPerTick;
}

bool TzClock::Monotonic::adjustTime(timespec *delta, timespec *oldDelta) {
    uint64_t now = TzHwCounter::timeNow();
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    oldDelta->tv_sec = currAdjustment/hz;
    oldDelta->tv_nsec = (currAdjustment - (oldDelta->tv_sec * hz)) * nsPerTick;

    const uint64_t ticksInDelta = (delta->tv_sec * hz) + (delta->tv_nsec/nsPerTick);
    if ((ticksInDelta + now) < now) {
        // The adjustment will take time backwards.
        // Cannot let that happen on a monotonic clock.
        return false;
    }

    numAdjSteps = delta->tv_sec + 1;
    adjustmentStep = ticksInDelta/numAdjSteps;
    lastAdjustedAt = now;

    currAdjustment = ticksInDelta;

    return true;
}

TzClock::TaskClock::TaskClock(const TzTask *t) : task(t) {
    timeBase = 0;
}

TzClock::TaskClock::~TaskClock() {

}

void TzClock::TaskClock::resolution(timespec *ts) {
    TzClock::RealTime::resolution(ts);
}

void TzClock::TaskClock::time(timespec *ts) {
    uint64_t now = task->cumRunTime;
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    // Translate raw ticks to number of ticks from epoch start.
    now += timeBase;

    ts->tv_sec = now/hz;
    ts->tv_nsec = (now - (ts->tv_sec * hz)) * nsPerTick;
}

void TzClock::TaskClock::setTime(timespec *ts) {
    uint64_t now = task->cumRunTime;
    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;

    uint64_t newTicks = (ts->tv_sec*hz) + (ts->tv_nsec/nsPerTick);
    timeBase = newTicks - now;
}

void SysCalls::doClockGetRes(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    timespec kernelTs;
    timespec *userTs = (timespec *)arg1;

    clockid_t clockId = arg0;
    TzTask *targetTask = currTask;

    if (clockId < 0) { // find the target task
        // http://man7.org/linux/man-pages/man3/clock_getcpuclockid.3.html

        pid_t pid = -(((clockId - 2) / 8) + 1);

        if (0 == pid) pid = currTask->id();

        clockId = CLOCK_PROCESS_CPUTIME_ID;
        targetTask = TzTask::taskFromId(pid);

        if (nullptr == targetTask) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ESRCH);
            return;
        }
        // check permission to access target clock
        if (currTask->pgid != targetTask->pgid) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EPERM);
            return;
        }
    }

    bool rc = copyFromUser(userTs, &kernelTs);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    switch (clockId) {
    case CLOCK_REALTIME:
    case CLOCK_REALTIME_COARSE:
        TzClock::RealTime::resolution(&kernelTs);
        break;

    case CLOCK_MONOTONIC:
    case CLOCK_MONOTONIC_RAW:
    case CLOCK_MONOTONIC_COARSE:
    case CLOCK_BOOTTIME:
        TzClock::Monotonic::resolution(&kernelTs);
        break;

    case CLOCK_PROCESS_CPUTIME_ID:
    case CLOCK_THREAD_CPUTIME_ID: {
        TzClock::TaskClock *tclk = targetTask->getTaskClock();
        tclk->resolution(&kernelTs);
        break;
    }

    default:
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    // printf("Copying kernel struct %p to user %p\n", &kernelTs, userTs);
    copyToUser(userTs, &kernelTs);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doClockGetTime(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    timespec kernelTs;
    timespec *userTs = (timespec *)arg1;

    clockid_t clockId = arg0;
    TzTask *targetTask = currTask;

    if (clockId < 0) { // find the target task
        // http://man7.org/linux/man-pages/man3/clock_getcpuclockid.3.html

        pid_t pid = -(((clockId - 2) / 8) + 1);

        if (0 == pid) pid = currTask->id();

        clockId = CLOCK_PROCESS_CPUTIME_ID;
        targetTask = TzTask::taskFromId(pid);

        if (nullptr == targetTask) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -ESRCH);
            return;
        }
        // check permission to access target clock
        if (currTask->pgid != targetTask->pgid) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EPERM);
            return;
        }
    }

    bool rc = copyFromUser(userTs, &kernelTs);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
         return;
    }

    switch (clockId) {
    case CLOCK_REALTIME:
    case CLOCK_REALTIME_COARSE:
        TzClock::RealTime::time(&kernelTs);
        break;

    case CLOCK_MONOTONIC:
    case CLOCK_MONOTONIC_COARSE:
        TzClock::Monotonic::time(&kernelTs);
        break;

    case CLOCK_MONOTONIC_RAW:
    case CLOCK_BOOTTIME:
        TzClock::Monotonic::rawTime(&kernelTs);
        break;

    case CLOCK_PROCESS_CPUTIME_ID:
    case CLOCK_THREAD_CPUTIME_ID: {
        TzClock::TaskClock *tclk = targetTask->getTaskClock();
        tclk->time(&kernelTs);
        break;
    }

    default:
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    copyToUser(userTs, &kernelTs);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doClockSetTime(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    timespec kernelTs;
    timespec *userTs = (timespec *)arg1;
    bool rc = copyFromUser(userTs, &kernelTs);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    switch (arg0) {
    case CLOCK_REALTIME:
    case CLOCK_REALTIME_COARSE:
        TzClock::RealTime::setTime(&kernelTs);
        break;

    case CLOCK_PROCESS_CPUTIME_ID:
    case CLOCK_THREAD_CPUTIME_ID: {
        TzClock::TaskClock *tclk = currTask->getTaskClock();
        tclk->setTime(&kernelTs);
        break;
    }

    default:
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    copyToUser(userTs, &kernelTs);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doNanoSleep(TzTask *currTask) {
    unsigned long arg0 = currTask->userReg(TzTask::UserRegs::r0);
    unsigned long arg1 = currTask->userReg(TzTask::UserRegs::r1);

    timespec durationKernel, remKernel;
    timespec *durationUser, *remUser;

    durationUser = (timespec *)arg0;
    remUser = (timespec *)arg1;
    bool rc = copyFromUser(durationUser, &durationKernel);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    const unsigned long hz = TzHwCounter::frequency();
    const unsigned long nsPerTick = 1000000000UL/hz;
    unsigned long ticks = (durationKernel.tv_sec * hz) + (durationKernel.tv_nsec/nsPerTick);
    unsigned long remain;

    currTask->nanosleep(ticks, &remain);

    remKernel.tv_sec = remain/hz;
    remKernel.tv_nsec = (remain - (remKernel.tv_sec * hz)) * nsPerTick;

    rc = copyToUser(remUser, &remKernel);
    if (!rc)
        if (remain > 0)
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINTR);
        else
            currTask->writeUserReg(TzTask::UserRegs::r0, 0);
    else
        currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}
