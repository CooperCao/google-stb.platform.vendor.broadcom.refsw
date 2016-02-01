/***************************************************************************
 *     Copyright (c) 2004-2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

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

