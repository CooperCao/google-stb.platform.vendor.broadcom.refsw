/***************************************************************************
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
 ***************************************************************************/

#include "scheduler.h"
#include "tztask.h"
#include "tzioc.h"
#include "hwtimer.h"
#include "gic.h"
#include "utils/vector.h"
#include "tracelog.h"

TzTask *currentTask[MAX_NUM_CPUS];

unsigned long Scheduler::TimeSliceDuration;
unsigned long Scheduler::TimeSliceDelay;
unsigned long Scheduler::TimeSliceStagger;
unsigned long Scheduler::TimeSliceEDF;

PerCPU<unsigned int> Scheduler::sumRunnablePriorities;
PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::runQueue;
SpinLock Scheduler::schedLock;

PerCPU<Timer> Scheduler::preemptionTimer;
PerCPU<uint64_t> Scheduler::startWorldRunTime;
PerCPU<uint64_t> Scheduler::cfsGlobalSlot;

PerCPU<uint64_t> Scheduler::edfGlobalSlot;
PerCPU<Timer> Scheduler::edfPreemptionTimer;
PerCPU<Timer> Scheduler::edfScheduleTimer;
PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::edfQueue;
PerCPU<Scheduler::Class> Scheduler::scheduleClass;

#ifdef TASK_LOG
PerCPU<tzutils::PriorityQueue<TzTask>> Scheduler::idleQueue;
Timer dumpTimer;
#endif

extern "C" void preemptionTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("preemptionTimerFired 0x%x%x  \n", (int)(TzHwCounter::timeNow() >> 32), (int)(TzHwCounter::timeNow() & 0xffffffff));

}

extern "C" void edfScheduleTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("\t edfScheduleTimerFired 0x%x%x  \n", (int)(TzHwCounter::timeNow() >> 32), (int)(TzHwCounter::timeNow() & 0xffffffff));
    Scheduler::scheduleClassType(Scheduler::Class::EDF_Class);
}

extern "C" void edfPreemptionTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);

    //printf("\t edfPreemptionTimerFired 0x%x%x  \n", (int)(TzHwCounter::timeNow() >> 32), (int)(TzHwCounter::timeNow() & 0xffffffff));
    Scheduler::scheduleClassType(Scheduler::Class::CFS_Class);
}

#ifdef TASK_LOG
extern "C" void dumpTimerFired(Timer t, void *ctx) {
    UNUSED(t);
    UNUSED(ctx);
    TraceLog::stop();
    TraceLog::inval();
    TraceLog::start();
}
#endif


void Scheduler::init() {
    spinLockInit(&schedLock);

    TimeSliceDuration = (TzHwCounter::frequency() / 1000 ) * TIME_SLICE_DURATION_MS;
    TimeSliceDelay    = (TzHwCounter::frequency() / 1000 ) * TIME_SLICE_DELAY_MS;
    TimeSliceStagger  = (TimeSliceDuration / 100) * TIME_SLICE_STAGGER_PERCENT;
    TimeSliceEDF      = (TimeSliceDuration / 100) * TIME_SLICE_EDF_PERCENT;
    //printf("Scheduler Init %ld %ld \n", TimeSliceDuration, TzHwCounter::frequency());

    runQueue.cpuLocal().init();
    preemptionTimer.cpuLocal() = INVALID_TIMER;
    scheduleClass.cpuLocal() = Class::CFS_Class;

    sumRunnablePriorities.cpuLocal() = 0;
    startWorldRunTime.cpuLocal() = 0;
    cfsGlobalSlot.cpuLocal() = 0;
    edfGlobalSlot.cpuLocal() = 0;

    edfQueue.cpuLocal().init();

    uint64_t timeNow = TzHwCounter::timeNow();
    uint64_t timeAlign = timeNow - timeNow % TimeSliceDuration + TimeSliceDuration;

    uint64_t timeSchedule = timeAlign + TimeSliceDelay + TimeSliceStagger * arm::smpCpuNum();
    edfScheduleTimer.cpuLocal() = TzTimers::create(timeSchedule, (uint64_t)TimeSliceDuration, edfScheduleTimerFired, nullptr);

    uint64_t timeStop = timeSchedule + TimeSliceEDF;
    edfPreemptionTimer.cpuLocal() = TzTimers::create(timeStop, (uint64_t)TimeSliceDuration, edfPreemptionTimerFired, nullptr);

#ifdef TASK_LOG
    idleQueue.cpuLocal().init();
    dumpTimer = TzTimers::create((TzHwCounter::timeNow()+(TzHwCounter::frequency()*5)), (uint64_t) TzHwCounter::frequency()*5, dumpTimerFired, nullptr);
#endif
    printf("Scheduler init done \n");
}

void Scheduler::initSecondaryCpu() {
    runQueue.cpuLocal().init();
    preemptionTimer.cpuLocal() = INVALID_TIMER;
    scheduleClass.cpuLocal() = Class::CFS_Class;

    sumRunnablePriorities.cpuLocal() = 0;
    startWorldRunTime.cpuLocal() = 0;
    cfsGlobalSlot.cpuLocal() = 0;
    edfGlobalSlot.cpuLocal() = 0;

    edfQueue.cpuLocal().init();

    uint64_t timeNow = TzHwCounter::timeNow();
    uint64_t timeAlign = timeNow - timeNow % TimeSliceDuration + TimeSliceDuration;

    uint64_t timeSchedule = timeAlign + TimeSliceDelay + TimeSliceStagger * arm::smpCpuNum();
    edfScheduleTimer.cpuLocal() = TzTimers::create(timeSchedule, (uint64_t)TimeSliceDuration, edfScheduleTimerFired, nullptr);

    uint64_t timeStop = timeSchedule + TimeSliceEDF;
    edfPreemptionTimer.cpuLocal() = TzTimers::create(timeStop, (uint64_t)TimeSliceDuration, edfPreemptionTimerFired, nullptr);

#ifdef TASK_LOG
    idleQueue.cpuLocal().init();
#endif
    printf("Secondary scheduler init done \n");
}

void Scheduler::addTask(TzTask *task) {
    SpinLocker locker(&schedLock);
    if (task->type == TzTask::Type::EDF_Task) {
        edfQueue.cpuLocal().remove(task->id());
        edfQueue.cpuLocal().enqueue(task);
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
        if ( T != nullptr)
            sumRunnablePriorities.cpuLocal() -= task->priority;
    }

#ifdef TASK_LOG
    if ( T != nullptr)
        idleQueue.cpuLocal().enqueue(task);
#endif
}

void Scheduler::currTaskStopped() {

    Timer timer = preemptionTimer.cpuLocal();
    if(timer!=INVALID_TIMER) {
        //printf("to destroy timer: 0x%x%x\n", (int)(timer >> 32), (int)(timer & 0xffffffff));
        TzTimers::destroy(timer);
    }
    preemptionTimer.cpuLocal() = INVALID_TIMER;
}

void Scheduler::scheduleClassType(Scheduler::Class classType) {

    scheduleClass.cpuLocal() = classType;
}

Scheduler::Class Scheduler::scheduleClassType() {

    return scheduleClass.cpuLocal();
}

#ifdef TASK_LOG
void Scheduler::updateTaskLog(uint64_t worldRunTime){
    int cpuNum = arm::smpCpuNum();
    edfQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
    runQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
    idleQueue.cpuLocal().populateTaskInfo(worldRunTime,cpuNum);
}
#endif

void Scheduler::updateTimeSlice() {

    const int cpuNum = arm::smpCpuNum();

    TzTask *currTask = currentTask[cpuNum];

#ifdef TASK_LOG // To print the CPU %
        uint64_t nowTime = TzHwCounter::timeNow();
        uint64_t worldRunTime = nowTime - Scheduler::startWorldRunTime.cpuLocal();
    /* Dump every 200ms */
    if((worldRunTime >= (TzHwCounter::frequency()/5))){
        updateTaskLog(worldRunTime);
            Scheduler::startWorldRunTime.cpuLocal() = nowTime;
        }
#endif

    /* Update for all Tasks even though it might not be in runnable state anymore */
    if(currTask != nullptr)
    {
        uint64_t taskRunTime = TzHwCounter::timeNow() - currTask->lastScheduledAt;
        currTask->totalRunTime += taskRunTime;
        currTask->cumRunTime += taskRunTime;
        currTask->taskRunTime += taskRunTime;

        /* Reset the lastScheduledAt so that we don't add taskRunTime twice if it were to come here again in the same schedule run */
        /* This happens when EDF task is not present */
        currTask->lastScheduledAt = TzHwCounter::timeNow();

        if((currTask->totalRunTime >= currTask->quantaRunTime))
        {
            currTask->totalRunTime = 0;
            /* Increment the slotTime */
            currTask->slotTimeSlice += 1;

            /* Usually slotTime is equal to OR 1 more than globalSlot */
            /* If its LESS then this task went to sleep and lost track of slots so update its slotTime*/

            if(currTask->type == TzTask::Type::CFS_Task){
                if(currTask->slotTimeSlice < cfsGlobalSlot.cpuLocal()){
                    //printf("Old Task Update Slot\n");
                    currTask->slotTimeSlice = cfsGlobalSlot.cpuLocal();
                }
            }else if(currTask->type == TzTask::Type::EDF_Task){
                if(currTask->slotTimeSlice < edfGlobalSlot.cpuLocal()){
                    //printf("Old Task Update Slot\n");
                    currTask->slotTimeSlice = edfGlobalSlot.cpuLocal();
                }
            }

        }

        /* Remove & Enqueue task to make sure it bubbles up/down according to the schedule slot that was changed above */
        /* Enqueue task only if remove succeeded: Task might not be in runQueue (runnable state) */
        if(currTask->type == TzTask::Type::CFS_Task){
            if(runQueue.cpuLocal().remove(currTask->id()) != nullptr)
                runQueue.cpuLocal().enqueue(currTask);
        }else if(currTask->type == TzTask::Type::EDF_Task){
            if(edfQueue.cpuLocal().remove(currTask->id()) != nullptr)
                edfQueue.cpuLocal().enqueue(currTask);
        }
        //printf("-->CurrTask %d Type %d Slot (%d / %d) 0x%x%x / 0x%x%x \n",currTask->id(), currTask->type, (int)(currTask->slotTimeSlice & 0xffffffff),
        //(int)(cfsGlobalSlot.cpuLocal() & 0xffffffff), (int)(currTask->pqValue() >> 32), (int)(currTask->pqValue() & 0xffffffff),
        //(int)(currTask->quantaRunTime  >> 32), (int)(currTask->quantaRunTime  & 0xffffffff) );

    }
}

TzTask *Scheduler::cfsSchedule() {
    SpinLocker locker(&schedLock);

    const int cpuNum = arm::smpCpuNum();
    currTaskStopped();
    updateTimeSlice();

    TzTask *nextTask = runQueue.cpuLocal().dequeue();
    if (nextTask == nullptr)
        return nullptr;

    nextTask->lastScheduledAt = TzHwCounter::timeNow();

    /* If the nextTask has slot higher than global slot then all the tasks         */
    /* in the queue have slot higher than global slot. So increment the global slot. */
    if(nextTask->slotTimeSlice > cfsGlobalSlot.cpuLocal()){

        cfsGlobalSlot.cpuLocal() = nextTask->slotTimeSlice;

    }
    if (runQueue.cpuLocal().head() != nullptr)
    {
        nextTask->quantaRunTime = (nextTask->priority * TimeSliceDuration)/sumRunnablePriorities.cpuLocal();
        uint64_t remainingRunTime = (uint64_t)(nextTask->quantaRunTime  - nextTask->pqValue());
        uint64_t timerRunTime = nextTask->lastScheduledAt + remainingRunTime;

        //printf("Quanta %d %d %d %d \n",nextTask->id(),nextTask->priority,(unsigned int)TimeSliceDuration,(unsigned int) sumRunnablePriorities.cpuLocal());
        preemptionTimer.cpuLocal() = TzTimers::create(timerRunTime, preemptionTimerFired, nullptr);

    }
    else {
        // There is no other runnable task. Hence no need for pre-emption.
        //printf("There is no other runnable task. Hence no need for pre-emption. \n");
    }

    //printf("<--NextTask %d Type %d Slot (%d / %d) 0x%x%x / 0x%x%x \n",nextTask->id(), nextTask->type, (int)(nextTask->slotTimeSlice & 0xffffffff),
    //(int)(cfsGlobalSlot.cpuLocal() & 0xffffffff), (int)(nextTask->pqValue() >> 32), (int)(nextTask->pqValue() & 0xffffffff),
    //(int)(nextTask->quantaRunTime  >> 32), (int)(nextTask->quantaRunTime  & 0xffffffff) );

    currentTask[cpuNum] = nextTask;

    sumRunnablePriorities.cpuLocal() -= nextTask->priority;

    if ((nextTask == TzTask::nwProxy())&&(cpuNum == 0)) {
        // Notify tzioc in normal world for processing.
        // This code can only be reached on the boot CPU.
        TzIoc::notify();
    }

    return nextTask;
}


/*=====================================================================*/


TzTask *Scheduler::edfSchedule() {
    SpinLocker locker(&schedLock);

    const int cpuNum = arm::smpCpuNum();
    currTaskStopped();
    updateTimeSlice();

    TzTask *nextTask = edfQueue.cpuLocal().dequeue();
    if (nextTask == nullptr){
        Scheduler::scheduleClassType(Scheduler::Class::CFS_Class);
        return nullptr;
    }

    nextTask->lastScheduledAt = TzHwCounter::timeNow();

    /* If the nextTask has slot higher than global slot then all the tasks         */
    /* in the queue have slot higher than global slot. So increment the global slot. */
    if(nextTask->slotTimeSlice > edfGlobalSlot.cpuLocal()){

        edfGlobalSlot.cpuLocal() = nextTask->slotTimeSlice;
    }

    if (runQueue.cpuLocal().head() != nullptr)
    {

        nextTask->quantaRunTime = (TimeSliceDuration / 100) * nextTask->priority;
        uint64_t remainingRunTime = (uint64_t)(nextTask->quantaRunTime  - nextTask->pqValue());
        uint64_t timerRunTime = nextTask->lastScheduledAt + remainingRunTime;
        preemptionTimer.cpuLocal() = TzTimers::create(timerRunTime, preemptionTimerFired, nullptr);

    }
    else {
        // There is no other runnable task. Hence no need for pre-emption.
        //printf("There is no other runnable task. Hence no need for pre-emption. \n");
    }
    //printf("<--NextTask %d Type %d Slot (%d / %d) 0x%x%x / 0x%x%x \n",nextTask->id(), nextTask->type, (int)(nextTask->slotTimeSlice & 0xffffffff),
    //(int)(edfGlobalSlot.cpuLocal() & 0xffffffff), (int)(nextTask->pqValue() >> 32), (int)(nextTask->pqValue() & 0xffffffff),
    //(int)(nextTask->quantaRunTime  >> 32), (int)(nextTask->quantaRunTime  & 0xffffffff) );

    currentTask[cpuNum] = nextTask;

    if ((nextTask == TzTask::nwProxy())&&(cpuNum == 0)) {
        // Notify tzioc in normal world for processing.
        // This code can only be reached on the boot CPU.
        TzIoc::notify();
    }
    return nextTask;

}


/*=====================================================================*/
void schedule() {

    TzTask *next;
    bool taskTerminated;

    if (Scheduler::startWorldRunTime.cpuLocal() == 0)
        Scheduler::startWorldRunTime.cpuLocal() = TzHwCounter::timeNow();

    do {
        //printf("Schedule Class %d 0x%x%x \n", Scheduler::scheduleClassType(), (int)(TzHwCounter::timeNow() >> 32), (int)(TzHwCounter::timeNow() & 0xffffffff));

        if (Scheduler::scheduleClassType() == Scheduler::Class::EDF_Class) {
            do{
                next = Scheduler::edfSchedule();
                taskTerminated = false;

                if (next == nullptr) {
                    Scheduler::scheduleClassType(Scheduler::Class::CFS_Class);
                    break;
                }

                if ((next->userReg(TzTask::UserRegs::spsr) & 0x1f) == Mode_USR)
                    taskTerminated = next->signalDispatch();

            }while (taskTerminated);
        }
        else { // if(Scheduler::scheduleClassType() == Scheduler::Class::CFS_Class) {
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
