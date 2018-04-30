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
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "hwtimer.h"
#include "utils/priorityqueue.h"
#include "cpulocal.h"
#include "tzsched.h"

extern "C" void schedule();

#define TIME_SLICE_DURATION_MS          20
#define TIME_SLICE_DELAY_MS             1000
#define TIME_SLICE_STAGGER_PERCENT      1
#define TIME_SLICE_EDF_PERCENT          60
#define TIME_SLICE_EDF_LIMIT_PERCENT    80

class TzTask;
extern "C" TzTask *currentTask[MAX_NUM_CPUS];

class Scheduler {
public:

    enum Class {
        CFS_Class,
        EDF_Class = SCHED_DEADLINE
    };

    static void init();
    static void initSecondaryCpu();

    static void addTask(TzTask *);
    static void removeTask(TzTask *);
    static void currTaskStopped();
    static void updateTimeSlice();

    static TzTask *cfsSchedule();
    static TzTask *edfSchedule();

    static void scheduleClassType(Class scheduleClass);
    static Class scheduleClassType();

#ifdef TASK_LOG // To print the CPU %
    static PerCPU<uint64_t> worldRunTimeStart;
    static PerCPU<tzutils::PriorityQueue<TzTask>> idleQueue;
    static void updateTaskLog(uint64_t worldRunTime);
    static void dumpTimerFired(Timer t, void *ctx);
#endif

private:
    static SpinLock schedLock;

    static PerCPU<bool> newTimeSlice;
    static PerCPU<Class> scheduleClass;

    static uint32_t TimeSliceDuration;
    static uint32_t TimeSliceDelay;
    static uint32_t TimeSliceStagger;
    static uint32_t TimeSliceEDFLimit;
    static PerCPU<uint32_t> TimeSliceEDF;

    static PerCPU<unsigned int> sumRunnablePriorities;
    static PerCPU<tzutils::PriorityQueue<TzTask>> runQueue;
    static PerCPU<Timer> preemptionTimer;

    static PerCPU<tzutils::PriorityQueue<TzTask>> edfQueue;
    static PerCPU<Timer> edfScheduleTimer;
    static PerCPU<Timer> edfPreemptionTimer;

    static PerCPU<uint64_t> cfsGlobalSlot;
    static PerCPU<uint64_t> edfGlobalSlot;

    static uint32_t cpuFreq;
    static PerCPU<uint32_t> cpuLoad;

    static void startNewTimeSlice();

    static void preemptionTimerFired(Timer t, void *ctx);
    static void edfScheduleTimerFired(Timer t, void *ctx);
    static void edfPreemptionTimerFired(Timer t, void *ctx);
};

#endif /* SCHEDULER_H_ */
