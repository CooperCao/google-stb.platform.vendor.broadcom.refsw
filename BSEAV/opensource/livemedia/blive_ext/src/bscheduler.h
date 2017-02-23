/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 **************************************************************************/

/**
Summary:
	blive_ext library header file.
        is a C wrapper around the LiveMedia library.
Description:
	It provides prototype definitions for the C APIs provided by the
        blive_ext library. This library provides extensions on top of the
        Live Media Library.

        Current externsions are in the following areas:
          1) APIs to enable Session Announcement Protocol Support (blive_sap_*)
          2) APIs to provide single threaded access to the Live Media Library (blive_scheduler_*)

**/
#ifndef BSCHEDULER_H__
#define BSCHEDULER_H__

#define B_LM_TASK_QUEUE_SIZE		32	        // max concurrent delayed tasks
#define	B_LM_SINGLE_STEP_USEC		500000		// max block on RTCP socket
#define BLIVE_MAX_PARAM_SIZE            128             // max parameter size

// structure to hold the per callback function (i.e. task) information.
typedef struct delayedTaskItem
{
	TaskFunc *delayedTask;  //function to run in the LM Scheduler Thread
	int64_t delayInUsec;    //how long to delay before running this function
	void *context;          //function specific context, passed in as argument
	bool schedule;          //does this task entry need scheduling?
} delayedTaskItem;

//
// This class extends the Basic Scheduler provided by Live Media. The main idea
// is to allow other threads to registers functions that can be run in
// the Live Media's Thread context. This is needed becaused the Live Media
// library is *NOT* Thread Safe.
//
class StepableTaskScheduler: public BasicTaskScheduler
{
public:
	static StepableTaskScheduler* createNew()
                { return new StepableTaskScheduler(); };
	virtual ~StepableTaskScheduler();

	void doSingleStep(void);
	void doScheduleDelayedTasks(void);
#ifdef SS_DEBUG
	void printStats();
	void incrTotalTasksRun()
                { totalTasksRun++; };
#endif
        // queue to hold tasks until they get scheduled w/ LM thread
	void queueDelayedTask(int64_t microseconds, TaskFunc* func, void *context);
        void lock()
	        { pthread_mutex_lock(&mutex); }
        void unlock()
	        { pthread_mutex_unlock(&mutex); }
protected:
	StepableTaskScheduler();
private:
	delayedTaskItem delayedTaskQueue[B_LM_TASK_QUEUE_SIZE];
	unsigned delayedTaskQueueCount;
	pthread_mutex_t mutex;
        pthread_mutexattr_t mutexAttr;
#ifdef SS_DEBUG
	unsigned totalTasksQueued;
	unsigned totalTasksScheduled;
	unsigned totalTasksRun;
#endif
};

#endif /* BSCHEDULER_H__ */
