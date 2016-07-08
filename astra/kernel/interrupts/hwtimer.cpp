/***************************************************************************
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
 ***************************************************************************/

#include "arm/arm.h"
#include "hwtimer.h"
#include "clock.h"
#include "interrupt.h"

#include "utils/priorityqueue.h"

PerCPU<TzTimers::Timers> TzTimers::timers;
static uint64_t nextTimerId;

typedef ObjCacheAllocator<TzTimers::TimerEntry> TimerAllocator;
static PerCPU<TimerAllocator> allocator;

spinlock_t TzTimers::lock;

ISRStatus tzTimerISR(int intrNum) {
    UNUSED(intrNum);

    // disable timer interrupts CNTP_CTL.
    register uint32_t enable = 0;
    asm volatile("mcr p15, 0, %[rt], c14, c2, 1" : : [rt] "r" (enable) :);

    TzTimers::hwTimerFired();
    return IntrDone;
}

void TzTimers::init() {
    nextTimerId = 0;
    Interrupt::isrRegister(CORTEX_A15_SECURE_TIMER_INTERRUPT, tzTimerISR);
    spinlock_init("timers.lock", &lock);

    TimerEntry::init();
    timers.cpuLocal().init();
}

void TzTimers::secondaryCpuInit() {
    Interrupt::isrRegister(CORTEX_A15_SECURE_TIMER_INTERRUPT, tzTimerISR);

    TimerEntry::init();
    timers.cpuLocal().init();
}

void TzTimers::TimerEntry::init() {
    allocator.cpuLocal().init();
}

void *TzTimers::TimerEntry::operator new (size_t sz) {
    UNUSED(sz);
    TzTimers::TimerEntry *rv = allocator.cpuLocal().alloc();
    return rv;
}

void TzTimers::TimerEntry::operator delete(void *te) {
    allocator.cpuLocal().free((TimerEntry *)te);
}

void TzTimers::restartHwTimer() {
    // Read CNTP_CTL to check if the timer is enabled
    register unsigned long cntpctl;
    asm volatile("mrc p15, 0, %[rt], c14, c2, 1" : [rt] "=r" (cntpctl) : :);
    bool hwTimerEnabled = (cntpctl & 1) && ((cntpctl & 2) == 0);

    uint64_t hwTimerFiresAt = 0xffffffffffffffff;
    if (hwTimerEnabled) {
        // Get the current CNTP_CVAL
        register uint32_t timeHigh, timeLow;
        asm volatile("MRRC p15, 2, %[low], %[high], c14" : [low] "=r" (timeLow), [high] "=r" (timeHigh) : : );
        hwTimerFiresAt = ((uint64_t) timeHigh << 32) | timeLow;
    }

    //Check the timer value at the head of the queue
    TimerEntry *headEntry = timers.cpuLocal().head();
    if (headEntry == nullptr)
        return;

    uint64_t headTime = headEntry->fireAt;
    if ((headEntry != nullptr) && (headTime < hwTimerFiresAt)) {

        // Set the hardware timer to fire at headTime
        register uint32_t fireAtLow = (uint32_t)(headTime & 0xffffffff);
        register uint32_t fireAtHigh = (uint32_t)(headTime >> 32);
        asm volatile("MCRR p15, 2, %[low], %[high], c14" : :[low] "r" (fireAtLow), [high] "r" (fireAtHigh):);

        register uint32_t enable = 0x1;
        asm volatile("mcr p15, 0, %[rt], c14, c2, 1" : : [rt] "r" (enable):);

        // printf("%s: fires at 0x%lx%lx\n", __FUNCTION__, fireAtLow, fireAtHigh);
    }
}

Timer TzTimers::create(unsigned long delta, OnExpiry handler, void *ctx) {
    SpinLocker locker(&lock);

    TzTimers::TimerEntry *te = new TzTimers::TimerEntry();
    te->fireAt = TzHwCounter::timeNow() + delta;
    te->callback = handler;
    te->ctx = ctx;
    te->handle = ++nextTimerId;
    te->period = 0;
    te->periodicTimer = false;

    timers.cpuLocal().enqueue(te);
    restartHwTimer();

    return te->id();
}

Timer TzTimers::create(uint64_t fireAt, OnExpiry handler, void *ctx) {
    SpinLocker locker(&lock);

    TzTimers::TimerEntry *te = new TzTimers::TimerEntry();
    te->fireAt = fireAt;
    te->callback = handler;
    te->ctx = ctx;
    te->handle = ++nextTimerId;
    te->period = 0;
    te->periodicTimer = false;

    timers.cpuLocal().enqueue(te);
    restartHwTimer();

    return te->id();
}

Timer TzTimers::create(uint64_t fireAt, uint64_t period, OnExpiry handler, void *ctx) {
    SpinLocker locker(&lock);

    TzTimers::TimerEntry *te = new TzTimers::TimerEntry();
    te->fireAt = fireAt;
    te->callback = handler;
    te->ctx = ctx;
    te->handle = ++nextTimerId;
    te->period = period;
    te->periodicTimer = true;

    timers.cpuLocal().enqueue(te);
    restartHwTimer();

    return te->id();
}


void TzTimers::destroy(Timer th) {
    SpinLocker locker(&lock);

    TzTimers::TimerEntry *te = timers.cpuLocal().remove(th);
    if (te != nullptr) {
        delete te;
    }

    restartHwTimer();
}

void TzTimers::hwTimerFired() {
    SpinLocker locker(&lock);

    // Get the current time ( CNTPCT ).
    register uint32_t timeHigh, timeLow;
    asm volatile("MRRC p15, 0, %[low], %[high], c14" : [low] "=r" (timeLow), [high] "=r" (timeHigh) : : );
    uint64_t timeNow = ((uint64_t) timeHigh << 32) | timeLow;

    // Trigger the timer callback for all entries smaller than the current time.
    TimerEntry *nextEntry = timers.cpuLocal().head();
    while ((nextEntry != nullptr) && (nextEntry->fireAt <= timeNow)) {

        nextEntry->callback(nextEntry->id(), nextEntry->ctx);
        timers.cpuLocal().dequeue();

        if(nextEntry->isPeriodic()){
            /* Do not delete the Timer entry. Setup next fireAt value */
            nextEntry->fireAt += nextEntry->period;
            timers.cpuLocal().enqueue(nextEntry);
        }else{
            delete nextEntry;
        }
        nextEntry = timers.cpuLocal().head();
    }

    restartHwTimer();
}
