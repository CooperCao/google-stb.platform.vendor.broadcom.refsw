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

#include "hwtimer.h"
#include "tztask.h"
#include "scheduler.h"
#include "waitqueue.h"

static void timeOutHandler(Timer t, void *ctx) {
    UNUSED(t);
    WaitQueue::WQState *wqState = (WaitQueue::WQState *)ctx;
    TzTask *task = wqState->task;
    WaitQueue *wq = wqState->wq;

    if ((task != nullptr) && (wq != nullptr))
        wq->timedOut(task);
}

void WaitQueue::init() {
    head = nullptr;
    spinLockInit(&lock);
}

void WaitQueue::addWaiter(TzTask *task, uint64_t timeout) {
    spinLockAcquire(&lock);

    task->wqState.wq = this;
    task->wqState.task = task;
    task->wqState.next = head;
    task->wqState.prev = &head;

    if (head != nullptr)
        head->wqState.prev = &(task->wqState.next);

    if (timeout != -1) {
        task->wqState.timer = TzTimers::create(timeout, timeOutHandler, &(task->wqState));
    }
    else {
        task->wqState.timer = INVALID_TIMER;
    }
    head = task;
    spinLockRelease(&lock);
}

void WaitQueue::wait(TzTask *task, uint64_t timeout) {

    addWaiter(task, timeout);

    task->yield();

    PageTable::kernelPageTable()->activate();
    Scheduler::addTask(task);
    //printf("task %d done waiting\n", task->id());
}

void WaitQueue::unlockAndWait(SpinLock *toUnlock, TzTask *task, uint64_t timeout) {

    addWaiter(task, timeout);

    spinLockRelease(toUnlock);

    task->yield();

    PageTable::kernelPageTable()->activate();
    Scheduler::addTask(task);
    //printf("task %d done waiting2\n", task->id());
}

int WaitQueue::signal() {

    spinLockAcquire(&lock);

    int count = 0;
    while (head != nullptr) {
        TzTask *curr = head;
        TzTask *next = curr->wqState.next;

        //printf("%s: wake up task %p\n", __PRETTY_FUNCTION__, curr);
        if (curr->wqState.timer != INVALID_TIMER) {
            TzTimers::destroy(curr->wqState.timer);
            curr->wqState.timer = INVALID_TIMER;
        }

        curr->awaken();

        curr->wqState.wq = nullptr;
        curr->wqState.prev = nullptr;
        curr->wqState.next = nullptr;
        if (next != nullptr)
            next->wqState.prev = &head;

        head = next;
        count++;
    }

    spinLockRelease(&lock);

    return count;
}

int WaitQueue::signalOne() {

    spinLockAcquire(&lock);

    if (head == nullptr) {
        spinLockRelease(&lock);
        return 0;
    }

    head->awaken();

    TzTask *curr = head;
    TzTask *next = curr->wqState.next;

    if (curr->wqState.timer != INVALID_TIMER) {
        TzTimers::destroy(curr->wqState.timer);
        curr->wqState.timer = INVALID_TIMER;
    }
    curr->wqState.wq = nullptr;

    if (next != nullptr)
        next->wqState.prev = &head;
    curr->wqState.next = nullptr;
    curr->wqState.prev = nullptr;

    head = next;

    spinLockRelease(&lock);

    return 1;
}

int WaitQueue::signalSome(int n) {

    spinLockAcquire(&lock);

    int count = 0;
    while ((count < n) && (head != nullptr)) {
        TzTask *curr = head;
        TzTask *next = curr->wqState.next;

        //printf("%s: wake up task %p\n", __PRETTY_FUNCTION__, curr);
        if (curr->wqState.timer != INVALID_TIMER) {
            TzTimers::destroy(curr->wqState.timer);
            curr->wqState.timer = INVALID_TIMER;
        }
        curr->wqState.wq = nullptr;

        curr->awaken();

        curr->wqState.prev = nullptr;
        curr->wqState.next = nullptr;
        if (next != nullptr)
            next->wqState.prev = &head;

        head = next;

        count++;
    }

    spinLockRelease(&lock);

    return count;
}

void WaitQueue::timedOut(TzTask *task) {
    spinLockAcquire(&lock);

    TzTask *curr = task;
    TzTask *next = curr->wqState.next;
    TzTask **prev = curr->wqState.prev;

    if (next != nullptr) {
        next->wqState.prev = prev;
    }

    *prev = next;

    curr->wqState.wq = nullptr;
    curr->wqState.next = nullptr;
    curr->wqState.prev = nullptr;
    curr->wqState.timer = INVALID_TIMER;

    curr->awaken();

    spinLockRelease(&lock);
}

void WaitQueue::migrateTasks(WaitQueue *other) {
    spinLockAcquire(&lock);
    spinLockAcquire(&other->lock);

    while (head != nullptr) {
        TzTask *curr = head;
        TzTask *next = curr->wqState.next;

        curr->wqState.wq = nullptr;
        curr->wqState.prev = nullptr;
        curr->wqState.next = nullptr;
        if (next != nullptr)
            next->wqState.prev = &head;


        curr->wqState.wq = other;
        curr->wqState.prev = &other->head;
        curr->wqState.next = other->head;
        if (other->head != nullptr)
            other->head->wqState.prev = &curr->wqState.next;
        other->head = curr;

        head = next;
    }

    spinLockRelease(&other->lock);
    spinLockRelease(&lock);

}

int WaitQueue::numWaitingTasks() {
    TzTask *curr = head;
    int count = 0;
    while (curr != nullptr) {
        count++;
        curr = curr->wqState.next;
    }

    return count;
}
