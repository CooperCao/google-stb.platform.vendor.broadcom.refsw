/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include <cstdint>
#include "tzerrno.h"
#include "tzfutex.h"

#include "system.h"
#include "svcutils.h"
#include "tztask.h"
#include "scheduler.h"
#include "futex.h"
#include "tzclone.h"
#include "fs/ramfs.h"


void SysCalls::doSetThreadArea(TzTask *currTask) {

    unsigned long userArg = currTask->userReg(TzTask::UserRegs::r0);
    struct user_desc *argUser = (struct user_desc *)userArg;
    struct user_desc userDesc;

    bool rc = copyFromUser(argUser, &userDesc);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    currTask->SetThreadInfo((uintptr_t) argUser);
    int rv = currTask->setThreadArea(&userDesc);
    if (rv != 0) {
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    copyToUser(argUser, &userDesc);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doGetThreadArea(TzTask *currTask) {

    unsigned long userArg = currTask->userReg(TzTask::UserRegs::r0);
    struct user_desc *argUser = (struct user_desc *)userArg;
    struct user_desc userDesc;

    bool rc = copyFromUser(argUser, &userDesc);
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    //printf("%s: entry_num %d\n", __FUNCTION__, userDesc.entry_number);
    int rv = currTask->getThreadArea(&userDesc);
    if (rv != 0) {
        currTask->writeUserReg(TzTask::UserRegs::r0, rv);
        return;
    }

    copyToUser(argUser, &userDesc);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doSetTidAddress(TzTask *currTask) {
    int *tidPtr = (int *)currTask->userReg(TzTask::UserRegs::r0);

    if ((tidPtr == nullptr) || (!validateUserMemAccess(tidPtr, sizeof(void *)))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    //printf("%s: tidptr %p\n", __FUNCTION__, tidPtr);
    int rv = currTask->setTidAddress(tidPtr);
    //printf("%s: return tid %d\n", __FUNCTION__, rv);

    currTask->writeUserReg(TzTask::UserRegs::r0, rv);
}

void SysCalls::doGetTid(TzTask *currTask) {
    currTask->writeUserReg(TzTask::UserRegs::r0, currTask->id());
}

void SysCalls::doFutex(TzTask *currTask) {

    // Extract the system call parameters
    int *uaddr = (int *)currTask->userReg(TzTask::UserRegs::r0);
    int op = (int)currTask->userReg(TzTask::UserRegs::r1);
    int val = (int)currTask->userReg(TzTask::UserRegs::r2);
    timespec *timeout = (timespec *)currTask->userReg(TzTask::UserRegs::r3);
    int *uaddr2 = (int *)currTask->userReg(TzTask::UserRegs::r4);
    int val3 = (int)currTask->userReg(TzTask::UserRegs::r5);

    if ((uaddr == nullptr) || (!validateUserMemAccess(uaddr, sizeof(int *)))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    op = op & ~FUTEX_PRIVATE_FLAG;

    // printf("---->%s task %d: uaddr %p op %d val %d\n", __FUNCTION__, currTask->id(), uaddr, op, val);

    switch (op) {
    case FUTEX_WAIT: {
        int *paddr = (int *)userMemPhysAddr(uaddr);

        uint64_t sleepUntil = -1;
        if (timeout != nullptr) {
            timespec kTimeout;

            if (!copyFromUser(timeout, &kTimeout)) {
                currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
                return;
            }

            //printf("\ttimeout %ld.%ld\n", kTimeout.tv_sec, kTimeout.tv_nsec);
            uint64_t us = ((uint64_t )kTimeout.tv_sec * 1000000ULL) + (uint64_t)kTimeout.tv_nsec/1000;
            uint64_t ticks = TzHwCounter::usToTicks(us);
            //printf("\tticks 0x%x%x\n", (int)(ticks >> 32), (int)(ticks & 0xffffffff));
            sleepUntil = TzHwCounter::timeNow() + ticks;
        }

        Futex *futex = FutexStore::lookUp(paddr);
        if (futex == nullptr) {
            futex = new Futex(paddr);
            FutexStore::add(futex);

        }

        int rc = futex->wait(val, sleepUntil);

        if (futex->numWaitingTasks() == 0) {
            FutexStore::remove(futex);
            delete futex;

        }

        currTask->writeUserReg(TzTask::UserRegs::r0, rc);
        return;
    }

    case FUTEX_WAKE: {
        int *paddr = (int *)userMemPhysAddr(uaddr);

        Futex *futex = FutexStore::lookUp(paddr);
        if (futex == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, 0);
            return;
        }

        int rc = futex->wake(val);

        currTask->writeUserReg(TzTask::UserRegs::r0, rc);
        return;
    }

    case FUTEX_REQUEUE: {
        if ((uaddr2 == nullptr) || (!validateUserMemAccess(uaddr2, sizeof(int *)))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        int *paddr = (int *)userMemPhysAddr(uaddr);
        int *paddr2 = (int *)userMemPhysAddr(uaddr2);

        Futex *futex = FutexStore::lookUp(paddr);
        if (futex == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, 0);
            return;
        }

        Futex *futex2 = FutexStore::lookUp(paddr2);
        if (futex2 == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        int rc = futex->requeue(val, futex2);

        currTask->writeUserReg(TzTask::UserRegs::r0, rc);
        return;
    }

    case FUTEX_CMP_REQUEUE: {
        if ((uaddr2 == nullptr) || (!validateUserMemAccess(uaddr2, sizeof(int *)))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        int *paddr = (int *)userMemPhysAddr(uaddr);
        int *paddr2 = (int *)userMemPhysAddr(uaddr2);

        Futex *futex = FutexStore::lookUp(paddr);
        if (futex == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, 0);
            return;
        }

        Futex *futex2 = FutexStore::lookUp(paddr2);
        if (futex2 == nullptr) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
            return;
        }

        int rc = futex->cmpRequeue(val3, val, futex2);

        currTask->writeUserReg(TzTask::UserRegs::r0, rc);
        return;
    }

    default:
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOSYS);
        return;
    }

}

void SysCalls::doClone(TzTask *currTask) {
    unsigned long flags = (unsigned long)currTask->userReg(TzTask::UserRegs::r0);
    if(flags==SIGCHLD) {
        return doFork(currTask);
    }
    void *childStack = (void *)currTask->userReg(TzTask::UserRegs::r1);
    void *ptid = (void *)currTask->userReg(TzTask::UserRegs::r2);
    void *ctid = (void *)currTask->userReg(TzTask::UserRegs::r4);
    // void *ptRegs = (void *)currTask->userReg(TzTask::UserRegs::r4);
    // void *entryPoint = (void *)currTask->userReg(TzTask::UserRegs::r5);
    uint8_t *tls = (uint8_t *)currTask->userReg(TzTask::UserRegs::r3);
    //tls = tls - 8 + 192; // Look at TP_ADJ macro in MUSL pthread_arch.h. sizeof(struct pthread) = 192.

    //printf("---->%s: ptid %p ctid %p tls %p childStack %p flags %lx\n", __FUNCTION__, ptid, ctid, tls, childStack, flags);

    if ((ptid != nullptr) && (flags & CLONE_PARENT_SETTID)) {
        if (!validateUserMemAccess(ptid, sizeof(void *))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
    }

    if ((ctid != nullptr) && (flags & CLONE_CHILD_SETTID)) {
        if (!validateUserMemAccess(ctid, sizeof(void *))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            return;
        }
    }

    if (childStack == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }
    if (!validateUserMemAccess(childStack, sizeof(void *))) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    TzTask *cloned = new TzTask(*currTask, flags, childStack, ptid, ctid, tls);
    if (cloned == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }
    if (cloned->status() != TzTask::State::Ready) {
        delete cloned;
        currTask->writeUserReg(TzTask::UserRegs::r0, -ENOMEM);
        return;
    }

    Scheduler::addTask(cloned);

    currTask->writeUserReg(TzTask::UserRegs::r0, cloned->id());
}

struct sched_param {
    int sched_priority;     /* Scheduling priority */
};

void SysCalls::doSetSchedulerParam(TzTask *currTask) {

    //int pid = currTask->userReg(TzTask::UserRegs::r0);
    void *priority_t = (void *)currTask->userReg(TzTask::UserRegs::r1);
    int priority;

    if (priority_t != nullptr) {
        if (!validateUserMemAccess(priority_t, sizeof(priority))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            printf("Error param \n");
            return;
        }
    }

    //printf("doSetSchedulerParam %d \n",pid);
    bool rc = fromUser(priority_t, &priority, sizeof(priority));
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    Scheduler::removeTask(currTask);
    int oldPriority;
    int oldPolicy;
    currTask->getScheduler(&oldPolicy, &oldPriority);
    //printf("Get Task %d %d\n",oldPolicy, oldPriority);
    currTask->setScheduler(oldPolicy, priority);
    //printf("Set Task %d %d\n",oldPolicy, priority);

    Scheduler::addTask(currTask);

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);

}
void SysCalls::doGetSchedulerParam(TzTask *currTask) {

    //int pid = currTask->userReg(TzTask::UserRegs::r0);
    void *param_t = (void *)currTask->userReg(TzTask::UserRegs::r1);

    //printf("doGetSchedulerParam %d \n",pid);

    if (param_t != nullptr) {
        if (!validateUserMemAccess(param_t, sizeof(sched_param))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            printf("Error param \n");
            return;
        }
    }


    Scheduler::removeTask(currTask);
    int oldPriority;
    int oldPolicy;
    currTask->getScheduler(&oldPolicy, &oldPriority);
    //printf("Get Task %d %d\n",oldPolicy, oldPriority);

    sched_param param;
    param.sched_priority = oldPriority;
    bool rc = toUser(param_t, &param, sizeof(param));
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }

    Scheduler::addTask(currTask);

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);

}
void SysCalls::doGetScheduler(TzTask *currTask) {

    int pid = (int)currTask->userReg(TzTask::UserRegs::r0);

    //printf("doGetScheduler %d \n",pid);

    //Scheduler::removeTask(currTask);
    TzTask *task;
    if(currTask->id() == pid)
        task = currTask;
    else
        task = TzTask::taskFromId(pid);
    if(task == NULL) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        printf("Error : Invalid Task PID \n");
        return;
    }
    int oldPriority;
    int oldPolicy;
    task->getScheduler(&oldPolicy, &oldPriority);
    //printf("doGetScheduler : Get Task %d %d\n",oldPolicy, oldPriority);
    //Scheduler::addTask(currTask);

    currTask->writeUserReg(TzTask::UserRegs::r0, oldPolicy);

}

void SysCalls::doSetScheduler(TzTask *currTask) {

    int pid = currTask->userReg(TzTask::UserRegs::r0);
    int policy = currTask->userReg(TzTask::UserRegs::r1);
    void *param_t = (void *)currTask->userReg(TzTask::UserRegs::r2);

    //printf("doSetScheduler %d\n",pid);
    TzTask *task;
    if(currTask->id() == pid)
        task = currTask;
    else
        task = TzTask::taskFromId(pid);

    if(task == NULL) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        printf("Error : Invalid Task PID \n");
        return;
    }

    if (param_t != nullptr) {
        if (!validateUserMemAccess(param_t, sizeof(sched_param))) {
            currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
            printf("Error param \n");
            return;
        }
    }

    sched_param param;
    bool rc = fromUser(param_t, &param, sizeof(param));
    if (!rc) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EFAULT);
        return;
    }


    Scheduler::removeTask(task);
    int oldPriority;
    int oldPolicy;
    task->getScheduler(&oldPolicy, &oldPriority);
    //printf("Get Task %d %d\n",oldPolicy, oldPriority);

    task->setScheduler(policy, param.sched_priority);
    //printf("Set Task %d %d\n",policy, param.sched_priority);

    Scheduler::addTask(task);
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);

}
void SysCalls::doSchedRunTask(TzTask *currTask) {
    int pid = currTask->userReg(TzTask::UserRegs::r0);
    TzTask *runTask =  TzTask::taskFromId(pid);
    if (runTask == nullptr) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }

    Scheduler::removeTask(currTask);
    currTask->keepAtQueueHead = true;
    Scheduler::updateTimeSlice();
    Scheduler::addTask(currTask);
    currentTask[arm::smpCpuNum()] = runTask;

    int priority,policy;
    runTask->getScheduler(&policy, &priority);
    /*if(policy == SCHED_FIFO)
        Scheduler::setWHPreemptionTimer(runTask);*/

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
    resumeCurrentTask();
}

void SysCalls::doCriticalSecEnter(TzTask *currTask) {
    UNUSED(currTask);
#if 0
    unsigned int timeInNanoSec= currTask->userReg(TzTask::UserRegs::r0);
    if (timeInNanoSec > TzTask::MAX_CRITICAL_SECTION_TIME) {
        currTask->writeUserReg(TzTask::UserRegs::r0, -EINVAL);
        return;
    }
    currTask->enterCriticalSection(timeInNanoSec);
#endif

    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}

void SysCalls::doCriticalSecExit(TzTask *currTask) {
    UNUSED(currTask);
    //int rv = currTask->exitCriticalSection();
    currTask->writeUserReg(TzTask::UserRegs::r0, 0);
}
