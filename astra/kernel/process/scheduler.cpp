/***************************************************************************
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
 ***************************************************************************/

#include "scheduler.h"
#include "tztask.h"
#include "tzioc.h"
#include "hwtimer.h"
#include "gic.h"
#include "utils/vector.h"
#include "tracelog.h"

TzTask *currentTask[MAX_NUM_CPUS];

SpinLock Scheduler::schedLock;

PerCPU<bool> Scheduler::newTimeSlice;
PerCPU<Scheduler::Class> Scheduler::scheduleClass;

uint32_t Scheduler::TimeSliceDuration;
uint32_t Scheduler::TimeSliceDelay;
uint32_t Scheduler::TimeSliceStagger;
uint32_t Scheduler::TimeSliceEDFLimit;
PerCPU<uint32_t> Scheduler::TimeSliceEDF;
PerCPU<uint64_t> Scheduler::pendingEDFTimeSlice;

PerCPU<unsigned int> Scheduler::sumRunnablePriorities;
PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::runQueue;
PerCPU<Timer> Scheduler::preemptionTimer;

PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::edfQueue;
PerCPU<Timer> Scheduler::edfScheduleTimer;
PerCPU<Timer> Scheduler::edfPreemptionTimer;

PerCPU<uint64_t> Scheduler::cfsGlobalSlot;
PerCPU<uint64_t> Scheduler::edfGlobalSlot;

uint32_t Scheduler::cpuFreq;
PerCPU<uint32_t> Scheduler::cpuLoad;

#ifdef TASK_LOG
PerCPU<uint64_t> Scheduler::worldRunTimeStart;
PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::idleQueue;
Timer dumpTimer;
#endif

void Scheduler::preemptionTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("preemptionTimerFired 0x%lx\n", TzHwCounter::timeNow());
}

void Scheduler::edfScheduleTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("edfScheduleTimerFired 0x%lx\n", TzHwCounter::timeNow());
    newTimeSlice.cpuLocal() = true;
    scheduleClass.cpuLocal() = EDF_Class;
}

void Scheduler::edfPreemptionTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("edfPreemptionTimerFired 0x%lx\n", TzHwCounter::timeNow());
    scheduleClass.cpuLocal() = CFS_Class;
}

#ifdef TASK_LOG
void Scheduler::dumpTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);
    TraceLog::stop();
    TraceLog::inval();
    TraceLog::start();
}
#endif

void Scheduler::init() {
    spinLockInit(&schedLock);

    TimeSliceDuration = (uint64_t)TzHwCounter::frequency() * TIME_SLICE_DURATION_MS / 1000;
    TimeSliceDelay    = (uint64_t)TzHwCounter::frequency() * TIME_SLICE_DELAY_MS / 1000;
    TimeSliceStagger  = (uint64_t)TimeSliceDuration * TIME_SLICE_STAGGER_PERCENT / 100;
    TimeSliceEDFLimit = (uint64_t)TimeSliceDuration * TIME_SLICE_EDF_LIMIT_PERCENT / 100;

    runQueue.cpuLocal().init();
    preemptionTimer.cpuLocal() = INVALID_TIMER;

    newTimeSlice.cpuLocal() = false;
    pendingEDFTimeSlice.cpuLocal() = 0;
    scheduleClass.cpuLocal() = CFS_Class;

    sumRunnablePriorities.cpuLocal() = 0;
    cfsGlobalSlot.cpuLocal() = 0;
    edfGlobalSlot.cpuLocal() = 0;

    edfQueue.cpuLocal().init();

    if (ARMV8_BOOT_MODE == boot_mode) {
        /* Entire EDF period is CPU load, calculated with init CPU frequency */
        ARCH_SPECIFIC_CPU_UPDATE(0, cpuFreq);
        cpuLoad.cpuLocal() = cpuFreq * TIME_SLICE_EDF_PERCENT / 100;
    }
    else {
        /* Fake CPU frequency as 100, CPU load is the percentage */
        cpuFreq = 100;
        cpuLoad.cpuLocal() = TIME_SLICE_EDF_PERCENT;
    }
    //printf("Scheduler cpu update %d %d\n", cpuFreq, cpuLoad.cpuLocal());

    uint64_t timeNow = TzHwCounter::timeNow();
    uint64_t timeAlign = timeNow - timeNow % TimeSliceDuration + TimeSliceDuration;

    uint64_t timeSchedule = timeAlign + TimeSliceDelay + TimeSliceStagger * arm::smpCpuNum();
    edfScheduleTimer.cpuLocal() = TzTimers::create(timeSchedule, TimeSliceDuration, edfScheduleTimerFired, nullptr);
    //printf("Scheduler time slice timer 0x%lx 0x%lx\n", timeNow, timeSchedule);

#ifdef TASK_LOG
    worldRunTimeStart.cpuLocal() = timeNow;
    idleQueue.cpuLocal().init();

    uint64_t DumpInterval = TzHwCounter::frequency() * 5;
    uint64_t timeDump = timeSchedule + DumpInterval;
    dumpTimer = TzTimers::create(timeDump, DumpInterval, dumpTimerFired, nullptr);
#endif
    printf("Scheduler init done\n");
}

void Scheduler::initSecondaryCpu() {
    runQueue.cpuLocal().init();
    preemptionTimer.cpuLocal() = INVALID_TIMER;

    newTimeSlice.cpuLocal() = false;
    pendingEDFTimeSlice.cpuLocal() = 0;
    scheduleClass.cpuLocal() = CFS_Class;

    sumRunnablePriorities.cpuLocal() = 0;
    cfsGlobalSlot.cpuLocal() = 0;
    edfGlobalSlot.cpuLocal() = 0;

    edfQueue.cpuLocal().init();

    /* No tasks on secondary CPU */
    cpuLoad.cpuLocal() = 0;

    uint64_t timeNow = TzHwCounter::timeNow();
    uint64_t timeAlign = timeNow - timeNow % TimeSliceDuration + TimeSliceDuration;

    uint64_t timeSchedule = timeAlign + TimeSliceDelay + TimeSliceStagger * arm::smpCpuNum();
    edfScheduleTimer.cpuLocal() = TzTimers::create(timeSchedule, (uint64_t)TimeSliceDuration, edfScheduleTimerFired, nullptr);
    //printf("Scheduler time slice timer 0x%lx 0x%lx\n", timeNow, timeSchedule);

#ifdef TASK_LOG
    worldRunTimeStart.cpuLocal() = timeNow;
    idleQueue.cpuLocal().init();
#endif
    printf("Secondary scheduler init done\n");
}

void  Scheduler::evaluateSwitch(TzTask *task) {
    if(scheduleClassType() == CFS_Class)
    {
        //If task gets ready in same timeslice i.e. task slot is equal to global slot and totalRunTime is lesser than quantaRunTime
        if((task->slotTimeSlice <= edfGlobalSlot.cpuLocal()) && (pendingEDFTimeSlice.cpuLocal() > 0))
        {
            scheduleClass.cpuLocal() = EDF_Class;
        }
    }
}

void Scheduler::addTask(TzTask *task) {
    SpinLocker locker(&schedLock);
    if (task->type == TzTask::Type::EDF_Task) {
        edfQueue.cpuLocal().remove(task->id());
        edfQueue.cpuLocal().enqueue(task);
        evaluateSwitch(task);
    }
    else {
        sumRunnablePriorities.cpuLocal() += task->priority;
        runQueue.cpuLocal().remove(task->id());
        runQueue.cpuLocal().enqueue(task);
    }
#ifdef TASK_LOG
    idleQueue.cpuLocal().remove(task->id());
#endif

}

void Scheduler::removeTask(TzTask *task) {
    SpinLocker locker(&schedLock);

    TzTask *T = nullptr;
    if (task->type == TzTask::Type::EDF_Task) {
        T = edfQueue.cpuLocal().remove(task->id());
    }
    else {
        T = runQueue.cpuLocal().remove(task->id());
        if (T != nullptr)
            sumRunnablePriorities.cpuLocal() -= task->priority;
    }

#ifdef TASK_LOG
    if (T != nullptr)
        idleQueue.cpuLocal().enqueue(task);
#endif
}

void Scheduler::currTaskStopped() {

    Timer timer = preemptionTimer.cpuLocal();
    if (timer!=INVALID_TIMER) {
        //printf("to destroy timer: 0x%lx\n", timer);
        TzTimers::destroy(timer);
    }
    preemptionTimer.cpuLocal() = INVALID_TIMER;
}

void Scheduler::disbaleEDFPremeptionTimer() {
    Timer timer = edfPreemptionTimer.cpuLocal();
    pendingEDFTimeSlice.cpuLocal() -= TzHwCounter::timeNow();
    if (timer!=INVALID_TIMER) {
        TzTimers::destroy(timer);
    }
    edfPreemptionTimer.cpuLocal() = INVALID_TIMER;
}

void Scheduler::updateTimeSlice() {

    const int cpuNum = arm::smpCpuNum();

    TzTask *currTask = currentTask[cpuNum];

#ifdef TASK_LOG // To print the CPU %
    uint64_t timeNow = TzHwCounter::timeNow();
    uint64_t worldRunTime = timeNow - worldRunTimeStart.cpuLocal();

    /* Dump every 100ms */
    if ((worldRunTime >= (TzHwCounter::frequency()/10))) {
        updateTaskLog(worldRunTime);
        worldRunTimeStart.cpuLocal() = timeNow;
    }
#endif

    /* Update for all Tasks even though it might not be in runnable state anymore */
    if (currTask != nullptr) {
        uint64_t taskRunTime = TzHwCounter::timeNow() - currTask->lastScheduledAt;
        currTask->totalRunTime += taskRunTime;
        currTask->cumRunTime += taskRunTime;
        currTask->taskRunTime += taskRunTime;

        /* Reset the lastScheduledAt so that we don't add taskRunTime twice if it were to come here again in the same schedule run */
        /* This happens when EDF task is not present */
        currTask->lastScheduledAt = TzHwCounter::timeNow();

        if ((currTask->totalRunTime >= currTask->quantaRunTime)) {
            currTask->totalRunTime = 0;
            /* Usually slotTime is equal to OR 1 more than globalSlot */
            /* If its LESS then this task went to sleep and lost track of slots so update its slotTime*/
            if (currTask->type == TzTask::Type::CFS_Task) {
                if (currTask->slotTimeSlice < cfsGlobalSlot.cpuLocal()) {
                    currTask->slotTimeSlice = cfsGlobalSlot.cpuLocal();
                }
            }
            else if (currTask->type == TzTask::Type::EDF_Task) {
                if (currTask->slotTimeSlice < edfGlobalSlot.cpuLocal()) {
                    currTask->slotTimeSlice = edfGlobalSlot.cpuLocal();
                }
            }
        }

        /* Remove & Enqueue task to make sure it bubbles up/down according to the schedule slot that was changed above */
        /* Enqueue task only if remove succeeded: Task might not be in runQueue (runnable state) */
        if (currTask->type == TzTask::Type::CFS_Task) {
            if (runQueue.cpuLocal().remove(currTask->id()) != nullptr)
                runQueue.cpuLocal().enqueue(currTask);
        }
        else if (currTask->type == TzTask::Type::EDF_Task) {
            if (edfQueue.cpuLocal().remove(currTask->id()) != nullptr)
                edfQueue.cpuLocal().enqueue(currTask);
        }
        //printf("-->CurrTask %d Type %d Slot (%ld / %ld) 0x%lx / 0x%lx\n", currTask->id(), currTask->type,
        //currTask->slotTimeSlice, cfsGlobalSlot.cpuLocal(), currTask->pqValue(), currTask->quantaRunTime);
    }
}

void Scheduler::startNewTimeSlice() {
    if (ARMV8_BOOT_MODE == boot_mode) {
        /* CPU update */
        ARCH_SPECIFIC_CPU_UPDATE(cpuLoad.cpuLocal(), cpuFreq);
    }

    if (ARMV7_BOOT_MODE == boot_mode) {
        /* Call tzioc processing */
        if (arm::smpCpuNum() == 0)
            TzIoc::proc();
    }

    if (cpuLoad.cpuLocal()) {
        /* Scale EDF preemption timer according to CPU frequency */
        TimeSliceEDF.cpuLocal() = (uint64_t)TimeSliceDuration * cpuLoad.cpuLocal() / cpuFreq;
        if (TimeSliceEDF.cpuLocal() > TimeSliceEDFLimit)
            TimeSliceEDF.cpuLocal() = TimeSliceEDFLimit;

        uint64_t timeNow = TzHwCounter::timeNow();
        pendingEDFTimeSlice.cpuLocal() = timeNow + TimeSliceEDF.cpuLocal();
        edfPreemptionTimer.cpuLocal() = TzTimers::create(pendingEDFTimeSlice.cpuLocal(), edfPreemptionTimerFired, nullptr);
    }
    edfGlobalSlot.cpuLocal() += 1;
    cfsGlobalSlot.cpuLocal() += 1;
}

void Scheduler::scheduleClassType(Scheduler::Class classType) {

    scheduleClass.cpuLocal() = classType;
}

Scheduler::Class Scheduler::scheduleClassType() {

    return scheduleClass.cpuLocal();
}

#ifdef TASK_LOG
void Scheduler::updateTaskLog(uint64_t worldRunTime) {
    int cpuNum = arm::smpCpuNum();
    edfQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
    runQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
    idleQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
}
#endif

TzTask *Scheduler::cfsSchedule() {
    SpinLocker locker(&schedLock);

    const int cpuNum = arm::smpCpuNum();
    currTaskStopped();
    updateTimeSlice();

    TzTask *nextTask = runQueue.cpuLocal().dequeue();
    if (nextTask == nullptr)
        return nullptr;

    nextTask->lastScheduledAt = TzHwCounter::timeNow();

    if (runQueue.cpuLocal().head() != nullptr) {
        nextTask->quantaRunTime = (uint64_t)TimeSliceDuration * nextTask->priority / sumRunnablePriorities.cpuLocal();
        uint64_t remainingRunTime = (uint64_t)(nextTask->quantaRunTime  - nextTask->pqValue());
        uint64_t timerRunTime = nextTask->lastScheduledAt + remainingRunTime;

        //printf("Quanta %d %d %d %d\n",nextTask->id(), nextTask->priority, (unsigned int)TimeSliceDuration, (unsigned int) sumRunnablePriorities.cpuLocal());
        preemptionTimer.cpuLocal() = TzTimers::create(timerRunTime, preemptionTimerFired, nullptr);

    }
    else {
        // There is no other runnable task. Hence no need for pre-emption.
        //printf("There is no other runnable task. Hence no need for pre-emption.\n");
    }
    //printf("<--NextTask %d Type %d Slot (%ld / %ld) 0x%lx / 0x%lx\n", nextTask->id(), nextTask->type,
    //nextTask->slotTimeSlice, cfsGlobalSlot.cpuLocal(), nextTask->pqValue(), nextTask->quantaRunTime);

    currentTask[cpuNum] = nextTask;

    sumRunnablePriorities.cpuLocal() -= nextTask->priority;

    return nextTask;
}


/*=====================================================================*/


TzTask *Scheduler::edfSchedule() {
    SpinLocker locker(&schedLock);

    const int cpuNum = arm::smpCpuNum();
    currTaskStopped();
    updateTimeSlice();

    if (newTimeSlice.cpuLocal()) {
        startNewTimeSlice();
        newTimeSlice.cpuLocal() = false;
    }

    TzTask *nextTask = edfQueue.cpuLocal().dequeue();
    if (nextTask == nullptr) {
        scheduleClass.cpuLocal() = CFS_Class;
        disbaleEDFPremeptionTimer();
        return nullptr;
    }
    //Start EDF Preemption Timer if not running
    if(edfPreemptionTimer.cpuLocal() == INVALID_TIMER) {
        pendingEDFTimeSlice.cpuLocal() += TzHwCounter::timeNow();
        edfPreemptionTimer.cpuLocal() = TzTimers::create(pendingEDFTimeSlice.cpuLocal(), edfPreemptionTimerFired, nullptr);
    }

    nextTask->lastScheduledAt = TzHwCounter::timeNow();

    //printf("<--NextTask %d Type %d Slot (%ld / %ld) 0x%lx / 0x%lx\n", nextTask->id(), nextTask->type,
    //nextTask->slotTimeSlice, cfsGlobalSlot.cpuLocal(), nextTask->pqValue(), nextTask->quantaRunTime);

    currentTask[cpuNum] = nextTask;

    return nextTask;
}


/*=====================================================================*/
void schedule() {

    TzTask *next;
    bool taskTerminated;

    do {
        //printf("Schedule Class %d 0x%lx\n", Scheduler::scheduleClassType(), TzHwCounter::timeNow());

        if (Scheduler::scheduleClassType() == Scheduler::EDF_Class) {
            do {
                next = Scheduler::edfSchedule();
                taskTerminated = false;

                if (next == nullptr) {
                    Scheduler::scheduleClassType(Scheduler::CFS_Class);
                    break;
                }

                if ((next->userReg(TzTask::UserRegs::spsr) & 0x1f) == Mode_USR)
                    taskTerminated = next->signalDispatch();

            } while (taskTerminated);
        }
        else { // if (Scheduler::scheduleClassType() == Scheduler::CFS_Class) {
            do {
                next = Scheduler::cfsSchedule();
                taskTerminated = false;

                if (next == nullptr) {
                    err_msg("No runnable task - missing the normal world proxy task ?\n ");
                    kernelHalt("No runnable task");
                }

                if ((next->userReg(TzTask::UserRegs::spsr) & 0x1f) == Mode_USR)
                    taskTerminated = next->signalDispatch();

            } while (taskTerminated);
        }
    } while (next == nullptr);

    next->run();
}
