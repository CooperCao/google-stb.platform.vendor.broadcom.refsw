/***************************************************************************
 *     Copyright (c) 2004-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description: bscheduler.cpp: Provides Single Threaded Access to 
 *                     Live Media Library.
 *
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "Media.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "blive_ext.h"
#include "bstd.h"

#define SS_DEBUG 1
#include "bscheduler.h"

BDBG_MODULE(blive_ext);

static StepableTaskScheduler* lm_scheduler = NULL;
static bool lm_scheduler_thread_stop = false;
static UsageEnvironment* lm_env = NULL;
static pthread_t lm_scheduler_thread = 0;
static unsigned lm_refCnt = 0;

//
// This class extends the Basic Scheduler provided by Live Media. The main idea
// is to allow other threads to registers functions that can be run in 
// the Live Media's Thread context. This is needed becaused the Live Media
// library is *NOT* Thread Safe.
//
StepableTaskScheduler::StepableTaskScheduler() 
    : delayedTaskQueueCount(0), totalTasksQueued(0), totalTasksScheduled(0), totalTasksRun(0)
{
        // initialize the mutex to synchronize access between application & LM Scheduler Thread
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
    if (pthread_mutex_init(&mutex, &mutexAttr) !=0) 
        {
        BDBG_ERR(("mutex init failed"));
                BDBG_ASSERT(0);
    }

        // reset the task queue
    for(int i=0;i<B_LM_TASK_QUEUE_SIZE;i++) 
        { 
        delayedTaskQueue[i].schedule = false;
    }
}

StepableTaskScheduler::~StepableTaskScheduler() 
{

    pthread_mutexattr_destroy(&mutexAttr);
    pthread_mutex_destroy(&mutex);
}

// Function to schedule delayed tasks and execute them one by one
void StepableTaskScheduler::doSingleStep(void) 
{
    doScheduleDelayedTasks();
    SingleStep(B_LM_SINGLE_STEP_USEC);
}

#ifdef SS_DEBUG
typedef struct 
{
  void *appCtx;
  TaskFunc *func;
} taskHandlerCtx_t;

void StepableTaskScheduler::printStats()
{
        printf("Live Media Scheduler Stats ------------------------------------------------------->\n");
        printf("%10d totalTasksQueued\n", totalTasksQueued);
        printf("%10d totalTasksScheduled\n", totalTasksScheduled);
        printf("%10d totalTasksRun\n", totalTasksRun);
        printf("%10d tasksCurrentlyQueued\n", delayedTaskQueueCount);
}

//
// Function catch all the callback function calls. Main purpose to update
// the debug stats "totalTasksRun"
//
void doDelayedTaskHandler(void *ctx)
{
        taskHandlerCtx_t *handlerCtx;

        handlerCtx = (taskHandlerCtx_t *)ctx;
        BDBG_ASSERT((handlerCtx));
        lm_scheduler->incrTotalTasksRun();

        // now invoke the actual task
        (*handlerCtx->func)(handlerCtx->appCtx);

        // and free the handler ctx
        free(handlerCtx);
}

//
// Function to schedule the delayed tasks w/ the Live Media so that they can be run
// part of the SingleStep() call in a Single Threaded manner.
//
void StepableTaskScheduler::doScheduleDelayedTasks(void) 
{
        taskHandlerCtx_t *handlerCtx;
    lock();

    for(int i=0;i<B_LM_TASK_QUEUE_SIZE;i++) 
        {
        if(delayedTaskQueue[i].schedule) 
                {
                        // get space to save the taskHandlerCtx
                        if ( (handlerCtx = (taskHandlerCtx_t *)malloc(sizeof(taskHandlerCtx_t))) == NULL )
                        {
                          BDBG_ASSERT((handlerCtx != NULL));
                        }
                        handlerCtx->func = delayedTaskQueue[i].delayedTask;
                        handlerCtx->appCtx = delayedTaskQueue[i].context;
                                                                                          
                        // schedule the task w/ Live Media Library, it gets to run 
                        // when delayInUsec timeout expires.
                        BasicTaskScheduler:: scheduleDelayedTask(delayedTaskQueue[i].delayInUsec,
                                                                (TaskFunc *)doDelayedTaskHandler,
                                                                handlerCtx);
            BDBG_MSG(("@@@ scheduled delayed task: delayInUsec: %d, context: 0x%x, func: 0x%x\n", 
                   (unsigned)delayedTaskQueue[i].delayInUsec,
                   (unsigned long)&delayedTaskQueue[i].context,
                   (unsigned long)delayedTaskQueue[i].delayedTask));
                        // mark the queue entry as free as it has been scheduled to run
            delayedTaskQueue[i].schedule = false;
                        totalTasksScheduled++;
        }
    }

        // all tasks are now scheduled to run, so current delayed task queue is empty.
    delayedTaskQueueCount = 0;

    unlock();
}
#else

//
// Function to schedule the delayed tasks w/ the Live Media so that they can be run
// part of the SingleStep() call in a Single Threaded manner.
//
void StepableTaskScheduler::doScheduleDelayedTasks(void) 
{
    lock();

    for(int i=0;i<B_LM_TASK_QUEUE_SIZE;i++) 
        {
        if(delayedTaskQueue[i].schedule) 
                {
                        // schedule the task w/ Live Media Library, it gets to run 
                        // when delayInUsec timeout expires.
                        BasicTaskScheduler:: scheduleDelayedTask(delayedTaskQueue[i].delayInUsec,
                                                                delayedTaskQueue[i].delayedTask,
                                                                delayedTaskQueue[i].context);
            BDBG_MSG(("@@@ scheduled delayed task: delayInUsec: %d, context: 0x%x, func: 0x%x\n", 
                   (unsigned)delayedTaskQueue[i].delayInUsec,
                   (unsigned)&delayedTaskQueue[i].context,
                   (unsigned)delayedTaskQueue[i].delayedTask));
                        // mark the queue entry as free as it has been scheduled to run
            delayedTaskQueue[i].schedule = false;
        }
    }

        // all tasks are now scheduled to run, so current delayed task queue is empty.
    delayedTaskQueueCount = 0;

    unlock();
}
#endif
void StepableTaskScheduler::queueDelayedTask(int64_t microseconds, TaskFunc* func, void* context)
{

        // synchronize with the LM Scheduler Thread
    lock();

        // check if we have exceeded the task queue size
    if(delayedTaskQueueCount>=B_LM_TASK_QUEUE_SIZE) {
        BDBG_ERR(("queueDelayedTask(): max task queue count exceeded"));
                BDBG_ASSERT(0);
    }

        // store the task related parameters
    delayedTaskQueue[delayedTaskQueueCount].delayInUsec = microseconds;
    delayedTaskQueue[delayedTaskQueueCount].delayedTask = func;
    delayedTaskQueue[delayedTaskQueueCount].context = context;

        // enable the task entry
    delayedTaskQueue[delayedTaskQueueCount].schedule = true;

        // increment the currently queued tasks that need to get scheduled
    delayedTaskQueueCount++;
        totalTasksQueued++;

        // release the mutex
    unlock();
}

//
// This is the main function that runs the Live Media Scheduler Thread.
//
static void *schedulerFunc(void *data) 
{
        //
        // this is the Live Media Scheduler thread function that is constantly running the 
        // queued tasks and also monitoring & invoking callbacks on sockets w/ rd/wr events.
        //
        printf("Live Media Scheduler Thread is running (Scheduler PID = %d)\n", getpid());
    while(lm_scheduler_thread_stop == false) 
        {
                StepableTaskScheduler* scheduler = (StepableTaskScheduler*)data;
        scheduler->doSingleStep();
    }

        // here lm_scheduler_thread_stop == true 
        // which means Live Media Scheduler Thread is being stopped
    delete lm_scheduler; 
        lm_scheduler = NULL;

        // note: also need to free lm_env
        lm_env = NULL;

        return NULL;
}

/* ***********************************************************************************************/
/* ********************************* LM SCHEDULER APIs *******************************************/
/* ***********************************************************************************************/

//
// API to start the LM Scheduler Thread. It returns a pointer to the UsageEnvironment object
// that caller can use to access the Live Media Functions/Classes/Objects.
// Note: subsequent calls to this API simply return the pointer the lm_env object.
//
#define DEFAULT_STACK_SIZE (64*1024*8)

UsageEnvironment* blive_scheduler_open(void) 
{
    int ret;
    pthread_attr_t threadAttr;

    lm_scheduler_thread_stop = false;

    if(lm_scheduler == NULL) 
        { 
                // create the LM Scheduler object
        lm_scheduler = StepableTaskScheduler::createNew();
                BDBG_ASSERT( lm_scheduler != NULL );

                // create the LM UsageEnvironment object
                lm_env = BasicUsageEnvironment::createNew(*lm_scheduler); 
                BDBG_ASSERT( lm_env != NULL );

                // create the Live Media Scheduler Thread
                pthread_attr_init(&threadAttr);
                pthread_attr_setstacksize(&threadAttr, DEFAULT_STACK_SIZE);

        		ret = pthread_create(&lm_scheduler_thread, &threadAttr, schedulerFunc, lm_scheduler);
#ifdef B_REFSW_DEBUG
                BDBG_ASSERT( ret == 0 );
#else
                BSTD_UNUSED ( ret );
#endif
    }

        // synchronize with the LM Scheduler Thread
    lm_scheduler->lock();

        // increment the ref count as this API can be called multiple times
        lm_refCnt++;

        // release the mutex
    lm_scheduler->unlock();

        // return the UsageEnvironment object pointer, caller uses it to access the 
        // Live Media functions/classess/objects.
        return lm_env;
}

//
// API to close the LM Scheduler Thread, it only frees the context on the last call.
//
void blive_scheduler_close(void) 
{
    if(lm_scheduler == NULL) 
                return;

        // synchronize with the LM Scheduler Thread
    lm_scheduler->lock();

        //only close when all users are gone 
        lm_refCnt--;

        if (lm_refCnt > 0)
        {
                // release the mutex
            lm_scheduler->unlock();
                return;
        }

        BDBG_WRN(("Live Media Scheduler Thread is being shutdown\n"));
    lm_scheduler_thread_stop = true;

        // release the mutex
    lm_scheduler->unlock();
}

//
// API to queue a task for delayed execution in the LM Scheduler Thread.
//
void blive_scheduler_queue_delayed_task(int64_t microseconds, TaskFunc* func, void *context) 
{
        //BDBG_MSG(("scheduling delayed task: func ptr 0x%x, timeout %lld, context 0x%x\n", 
        //      func, microseconds, context));

    lm_scheduler->queueDelayedTask(microseconds, func, context);
}

//
// API to wait for completion of a queued task. Caller is required to define a 
// blive_scheduler_task_wait_ctx_t variable & pass a pointer to it. This 
// varible is used by the callback function to wake up the waiting app thread.
//
void blive_scheduler_wait(blive_scheduler_task_wait_ctx_t* wait_ctx)
{
        // initialize the pthread mutex (needed to avoid a race described below)
        pthread_mutexattr_init(&wait_ctx->mutex_attr);
        pthread_mutexattr_settype(&wait_ctx->mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    if (pthread_mutex_init(&wait_ctx->mutex, &wait_ctx->mutex_attr) !=0) 
        {
        BDBG_ERR(("blive_scheduler_wait(): mutex init failed"));
                BDBG_ASSERT(0);
    }

        // initialize the pthread condition variable
    if (pthread_cond_init(&wait_ctx->cond, NULL) !=0) 
        {
        BDBG_ERR(("blive_scheduler_wait(): cond init failed"));
                BDBG_ASSERT(0);
    }

        //
        // now wait until the callback function gets to run
        //

        // first get a mutex, this is to avoid a race where
        // other thread can signal the condition completion
        // before this thread gets to wait on the condition
        pthread_mutex_lock(&wait_ctx->mutex);

        //printf("before waiting in wait, PID = %d\n", getpid());
        // now wait for the condition completion
        pthread_cond_wait(&wait_ctx->cond, &wait_ctx->mutex);

        //printf("after waking up in app thread, PID = %d\n", getpid());
        // and then unlock the mutex
        pthread_mutex_unlock(&wait_ctx->mutex);

        // we now destroy the mutex & its attribute as they get reinitialized.
    pthread_mutexattr_destroy(&wait_ctx->mutex_attr);
    pthread_mutex_destroy(&wait_ctx->mutex);
}

//
// API to wake up an application to indicate the completion of a queued task.
//
void blive_scheduler_signal(blive_scheduler_task_wait_ctx_t* wait_ctx)
{
        // get the mutex first (to avoid the above described race)
        pthread_mutex_lock(&wait_ctx->mutex);

        // indicate to application thread that callback function processing is over
        pthread_cond_signal(&wait_ctx->cond);

        //printf("before unlock in wakeup, PID = %d\n", getpid());
        //now release the mutex
        pthread_mutex_unlock(&wait_ctx->mutex);
}

//
// API to print scheduler stats
//
void blive_scheduler_print_stats()
{
  lm_scheduler->printStats();
}

