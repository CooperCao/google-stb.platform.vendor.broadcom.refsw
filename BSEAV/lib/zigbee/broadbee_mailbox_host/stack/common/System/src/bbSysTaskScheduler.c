/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/src/bbSysTaskScheduler.c $
*
* DESCRIPTION:
*   Scheduler implementation.
*
* $Revision: 2869 $
* $Date: 2014-07-10 08:15:06Z $
*
****************************************************************************************/

/************************* INCLUDES ****************************************************/
#include "bbSysTaskScheduler.h"     /* Task Scheduler interface. */

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Is used to set the "Task is posted" flag.
  */
#define SYS_SCHEDULER_TASK_WAS_POSTED         0x80

/**//**
 * \brief The set of macros to work with the "Task is posted" flag.
  */
#define GET_PRIORITY(task) (task->priority & ~SYS_SCHEDULER_TASK_WAS_POSTED)
#define IS_TASK_POSTED(task) (task->priority & SYS_SCHEDULER_TASK_WAS_POSTED)
#define MARK_TASK_AS_POSTED(task) {task->priority |= SYS_SCHEDULER_TASK_WAS_POSTED;}
#define MARK_TASK_AS_NOT_POSTED(task) {task->priority &= ~SYS_SCHEDULER_TASK_WAS_POSTED;}

#define GET_TASK_DESCR(queueElement) (NULL != queueElement) ? \
                                      GET_PARENT_BY_FIELD(SYS_SchedulerTaskDescriptor_t, qElem, queueElement) : \
                                      NULL;


/* TODO: Move to the memory manager */
static SYS_QueueDescriptor_t MM_SchedulerActiveTasksQueue;
SYS_QueueDescriptor_t *MM_GetSchedulerActiveTasksQueue(void)
{
    return &MM_SchedulerActiveTasksQueue;
}

/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
  \brief Initializes scheduler.
****************************************************************************************/
void SYS_SchedulerInit(void)
{
    ATOMIC_SECTION_ENTER(SYS_SCHEDULER_INIT_0)
    SYS_QUEUE_ITERATION(MM_GetSchedulerActiveTasksQueue(), iterator)
    {
         SYS_SchedulerTaskDescriptor_t *task = GET_TASK_DESCR(iterator);
         MARK_TASK_AS_NOT_POSTED(task);
    }

    SYS_QueueResetQueue(MM_GetSchedulerActiveTasksQueue());
    ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_INIT_0)
}

/************************************************************************************//**
  \brief Runs pending task with the highest priority.
****************************************************************************************/
bool SYS_SchedulerRunTask(void)
{
    SYS_QueueDescriptor_t *activeTasksQueue = NULL;
    SYS_SchedulerTaskDescriptor_t *currentTask = NULL;
    SYS_SchedulerTaskDescriptor_t *highestPriorityTask = NULL;

    ATOMIC_SECTION_ENTER(SYS_SCHEDULER_RUN_TASK_0)
    activeTasksQueue = MM_GetSchedulerActiveTasksQueue();
    highestPriorityTask = GET_TASK_DESCR(SYS_QueueGetQueueHead(activeTasksQueue));

    if (NULL != highestPriorityTask)
        currentTask = GET_TASK_DESCR(SYS_QueueGetNextQueueElement(&highestPriorityTask->qElem));

    ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_RUN_TASK_0)

    while (currentTask)
    {
        ATOMIC_SECTION_ENTER(SYS_SCHEDULER_RUN_TASK_1)
        if (GET_PRIORITY(currentTask) <= GET_PRIORITY(highestPriorityTask))
            highestPriorityTask = currentTask;

        currentTask = GET_TASK_DESCR(SYS_QueueGetNextQueueElement(&currentTask->qElem));
        ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_RUN_TASK_1)
    }

    if (highestPriorityTask)
    {
        const SYS_SchedulerTaskHandler_t *handler = highestPriorityTask->handlers;
        uint32_t scanMask = 1UL; /* Set the bit in the mask corresponding to the very first handler. */

        SYS_DbgAssert(NULL != handler, HALT_SYS_SchedulerRunTask_NullHandlers);
        SYS_DbgAssert(highestPriorityTask->handlersMask, SYSSCHEDULER_RUNTASK_0);

        ATOMIC_SECTION_ENTER(SYS_SCHEDULER_RUN_TASK_2)
        while (NULL != *handler)
        {
            if (highestPriorityTask->handlersMask & scanMask)
            {
                highestPriorityTask->handlersMask &= ~scanMask;
                SYS_QueueRemoveQueueElement(activeTasksQueue, &highestPriorityTask->qElem);

                if (0UL == highestPriorityTask->handlersMask)
                    /* Task doesn't have active handlers. Mark it as "Unposted" and remove from queue. */
                    MARK_TASK_AS_NOT_POSTED(highestPriorityTask)
                else
                    /* Task have an active handler(s). Put it to the head of the queue. */
                    SYS_QueuePutQueueElementToHead(activeTasksQueue, &highestPriorityTask->qElem);

                break;
            }

            handler++;
            scanMask <<= 1; /* Shift scan mask to check the next handler. */
        }
        ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_RUN_TASK_2)

        if (NULL != *handler)
        {
            (*handler)(highestPriorityTask);
            return true;
        }
        else
            /* If the mask is not empty but the handler is not found than the task handlers array is incorrect. */
            SYS_DbgAssert(0 == highestPriorityTask->handlersMask, SYSSCHEDULER_RUNTASK_1);

    }

    return false;
}

/************************************************************************************//**
  \brief Posts task.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be called.
****************************************************************************************/
void SYS_SchedulerPostTask(SYS_SchedulerTaskDescriptor_t *task,
                           SYS_SchedulerTaskHandlerId_t handlerId)
{
    ATOMIC_SECTION_ENTER(SYS_SCHEDULER_POST_TASK_0)
    task->handlersMask |= (1UL << handlerId);

    if (!IS_TASK_POSTED(task))
    {
        MARK_TASK_AS_POSTED(task)
        SYS_QueuePutQueueElementToHead(MM_GetSchedulerActiveTasksQueue(), &task->qElem);
    }
    ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_POST_TASK_0)
}

/************************************************************************************//**
  \brief Recalls task.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be recalled.
****************************************************************************************/
void SYS_SchedulerRecallTask(SYS_SchedulerTaskDescriptor_t *task,
                             SYS_SchedulerTaskHandlerId_t handlerId)
{
    ATOMIC_SECTION_ENTER(SYS_SCHEDULER_RECALL_TASK_0)
    task->handlersMask &= ~(1UL << handlerId);

    if (IS_TASK_POSTED(task) && (0 == task->handlersMask))
    {
        MARK_TASK_AS_NOT_POSTED(task)
        SYS_QueueRemoveQueueElement(MM_GetSchedulerActiveTasksQueue(), &task->qElem);
    }
    ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_RECALL_TASK_0)
}

/************************************************************************************//**
  \brief Checks if the specified task and its handler are presented in the queue.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be called.
****************************************************************************************/
bool SYS_SchedulerIsTaskScheduled(SYS_SchedulerTaskDescriptor_t *task,
                                  SYS_SchedulerTaskHandlerId_t   handlerId)
{
    bool result = false;

    /* TODO: SYS_DbgAssert(NULL != task, 666); */
    /* TODO: SYS_DbgAssert(handlerId < SYS_SCHEDULER_HANDLERS_MAX_NUMBER, 666); */
    ATOMIC_SECTION_ENTER(SYS_SCHEDULER_IS_TASK_SCHEDULED_0)
    if (IS_TASK_POSTED(task))
        result = ( 0 != ( task->handlersMask & (1 << handlerId) ) );
    ATOMIC_SECTION_LEAVE(SYS_SCHEDULER_IS_TASK_SCHEDULED_0)

    return result;
}