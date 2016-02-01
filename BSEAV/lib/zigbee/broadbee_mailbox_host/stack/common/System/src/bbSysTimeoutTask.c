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
* FILENAME: $Workfile: trunk/stack/common/System/src/bbSysTimeoutTask.c $
*
* DESCRIPTION:
*   Timeout task handler implementation.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

/************************* INCLUDES ****************************************************/
#include "bbSysTimeoutTask.h"
#include "bbHalSystemTimer.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Converts time from milliseconds to timeout task ticks. The value is rounded up.
 */
#define TIMEOUT_TASK_MS_TO_TICKS(time_ms)       CEIL(time_ms, HAL_TIMER_TASK_PERIOD_MS)
#define TIMEOUT_TASK_TICKS_TO_MS(time_ticks)    ((time_ticks) * HAL_TIMER_TASK_PERIOD_MS)

/**//**
 * \brief Timeout special value.
 */
#define SYS_TIMEOUT_TASK_EXPIRED         0

/**//**
 * \brief Timeout task flags definition.
 */
#define SYS_TIMEOUT_TASK_FLAG_CANCEL     0x01
#define SYS_TIMEOUT_TASK_FLAG_FIRED      0x02
#define SYS_TIMEOUT_TASK_FLAG_REPEAT     0x04
#define SYS_TIMEOUT_TASK_FLAG_IS_TASK    0x08
#define SYS_TIMEOUT_TASK_FLAG_IS_SIGNAL  0x10

/* TODO: Move to the memory manager */
static SYS_QueueDescriptor_t MM_TimeoutTasksQueue =
{
    .nextElement = NULL
};

SYS_QueueDescriptor_t *MM_GET_TIMEOUT_TASKS_QUEUE(void)
{
    return &MM_TimeoutTasksQueue;
}

/************************* INLINE FUNCTIONS ********************************************/
/************************************************************************************//**
  \brief Returns a pointer to the service field by the specified pointer to queue element.
****************************************************************************************/
INLINE SYS_TimeoutTaskServiceField_t *getServiceField(SYS_QueueElement_t *queueElement)
{
    return (NULL != queueElement) ?
           GET_PARENT_BY_FIELD(SYS_TimeoutTaskServiceField_t, queueElement, queueElement) :
           NULL;
}

/************************************************************************************//**
  \brief Returns a pointer to the service field by the specified pointer to queue element.
****************************************************************************************/
INLINE uint32_t getTimeout(SYS_TimeoutTaskServiceField_t *serviceField)
{
    return (serviceField->flags & SYS_TIMEOUT_TASK_FLAG_IS_TASK) ?
           GET_PARENT_BY_FIELD(SYS_TimeoutTask_t, service, serviceField)->timeout :
           GET_PARENT_BY_FIELD(SYS_TimeoutSignal_t, service, serviceField)->timeout;
}

/************************* IMPLEMENTATION **********************************************/

/************************************************************************************//**
  \brief Reset routine for Timeout Task module.
****************************************************************************************/
void SYS_TimeoutTaskHandlerReset(void)
{
    SYS_QueueResetQueue(MM_GET_TIMEOUT_TASKS_QUEUE());
}

/************************************************************************************//**
  \brief Must be called by HAL from the special task handler
         each HAL_TIMEOUT_TASK_TIMER_GRANULARITY ms.
****************************************************************************************/
void SYS_TimeoutTaskHandlerTick(void)
{
    SYS_QueueDescriptor_t *timeoutTasksQueue = MM_GET_TIMEOUT_TASKS_QUEUE();
    SYS_TimeoutTaskServiceField_t *serviceField = getServiceField(SYS_QueueGetQueueHead(timeoutTasksQueue));

    while (NULL != serviceField)
    {
        SYS_DbgAssert(0 != serviceField->counter, SYSTIMEOUTTASK_HANDLERTICK_DA0);

        if (SYS_TIMEOUT_TASK_EXPIRED == --serviceField->counter)
        {
            serviceField->flags |= SYS_TIMEOUT_TASK_FLAG_FIRED;

            if (!(serviceField->flags & SYS_TIMEOUT_TASK_FLAG_REPEAT))
                SYS_QueueRemoveQueueElement(timeoutTasksQueue, &serviceField->queueElement);
            else
            {
                serviceField->counter = TIMEOUT_TASK_MS_TO_TICKS(getTimeout(serviceField));
                if (0 == serviceField->counter)
                    serviceField->counter++;
            }
        }

        if (serviceField->flags & SYS_TIMEOUT_TASK_FLAG_FIRED)
        {
            serviceField->flags &= ~SYS_TIMEOUT_TASK_FLAG_FIRED;

            if (serviceField->flags & SYS_TIMEOUT_TASK_FLAG_IS_TASK)
            {
                SYS_TimeoutTask_t *timeoutTask = GET_PARENT_BY_FIELD(SYS_TimeoutTask_t, service, serviceField);
                SYS_SchedulerPostTask(timeoutTask->taskDescriptor, timeoutTask->handlerId);
            }
            else if (serviceField->flags & SYS_TIMEOUT_TASK_FLAG_IS_SIGNAL)
            {
                SYS_TimeoutSignal_t *const timeoutSignal =
                    GET_PARENT_BY_FIELD(SYS_TimeoutSignal_t, service, serviceField);
                SYS_DbgAssert(NULL != timeoutSignal->callback, SYSTIMEOUTTASK_HANDLERTICK_DA1);
                timeoutSignal->callback(&timeoutSignal->service);
            }
            else
                /* At least one of the flags shall be set. */
                SYS_DbgAssert(false, SYSTIMEOUTTASK_HANDLERTICK_DA2);
        }

        serviceField = getServiceField(SYS_QueueGetNextQueueElement(&serviceField->queueElement));
    }
}

/************************************************************************************//**
  \brief Posts timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutTaskPost(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode)
{
    SYS_DbgAssert(NULL != timeoutTask, SYSTIMEOUTTASK_POSTTIMEOUTTASK_DA0);

    timeoutTask->service.flags  = SYS_TIMEOUT_TASK_FLAG_IS_TASK;
    timeoutTask->service.flags |= (TIMEOUT_TASK_REPEAT_MODE == mode) ?
                                  SYS_TIMEOUT_TASK_FLAG_REPEAT : 0;
    timeoutTask->service.counter = TIMEOUT_TASK_MS_TO_TICKS(timeoutTask->timeout);
    if (0 == timeoutTask->service.counter)
        timeoutTask->service.counter++;
    SYS_QueuePutQueueElementToTail(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutTask->service.queueElement);
}

/************************************************************************************//**
  \brief Re-posts timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutTaskRePost(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode)
{
    SYS_DbgAssert(NULL != timeoutTask, SYSTIMEOUTTASK_REPOSTTIMEOUTTASK_DA0);

    if(timeoutTask->service.flags & SYS_TIMEOUT_TASK_FLAG_CANCEL)
        return;
    timeoutTask->service.flags  = SYS_TIMEOUT_TASK_FLAG_IS_TASK;
    timeoutTask->service.flags |= (TIMEOUT_TASK_REPEAT_MODE == mode) ?
                                  SYS_TIMEOUT_TASK_FLAG_REPEAT : 0;
    timeoutTask->service.counter = TIMEOUT_TASK_MS_TO_TICKS(timeoutTask->timeout);
    if (0 == timeoutTask->service.counter)
        timeoutTask->service.counter++;
    SYS_QueuePutQueueElementToTail(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutTask->service.queueElement);
}

/************************************************************************************//**
  \brief Remove timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
****************************************************************************************/
void SYS_TimeoutTaskRemove(SYS_TimeoutTask_t *timeoutTask)
{
    SYS_DbgAssert(NULL != timeoutTask, SYSTIMEOUTTASK_REMOVETIMEOUTTASK_DA0);

    SYS_QueueRemoveQueueElement(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutTask->service.queueElement);
    timeoutTask->service.flags |=  SYS_TIMEOUT_TASK_FLAG_CANCEL;
}

/************************************************************************************//**
  \brief Starts timeout signal.
  \param[in] timeoutSignal - pointer to target timeout signal structure.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutSignalStart(SYS_TimeoutSignal_t *timeoutSignal, SYS_TimeoutTaskMode_t mode)
{
    SYS_DbgAssert(NULL != timeoutSignal, SYSTIMEOUTTASK_STARTTIMEOUTSIGNAL_DA0);

    timeoutSignal->service.flags  = SYS_TIMEOUT_TASK_FLAG_IS_SIGNAL;
    timeoutSignal->service.flags |= (TIMEOUT_TASK_REPEAT_MODE == mode) ?
                                    SYS_TIMEOUT_TASK_FLAG_REPEAT : 0;
    timeoutSignal->service.counter = TIMEOUT_TASK_MS_TO_TICKS(timeoutSignal->timeout);
    if (0 == timeoutSignal->service.counter)
        timeoutSignal->service.counter++;
    SYS_QueuePutQueueElementToTail(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutSignal->service.queueElement);
}

/************************************************************************************//**
  \brief Stops timeout signal.
  \param[in] timeoutSignal - pointer to target timeout signal structure.
****************************************************************************************/
void SYS_TimeoutSignalStop(SYS_TimeoutSignal_t *timeoutSignal)
{
    SYS_DbgAssert(NULL != timeoutSignal, SYSTIMEOUTTASK_STOPTTIMEOUTSIGNAL_DA0);

    SYS_QueueRemoveQueueElement(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutSignal->service.queueElement);
}

/************************************************************************************//**
  \brief Returns remaining time in ms to fire
  \param[in] timeoutService - pointer to timeout service structure.

  \return remaining time or 0, if the timer is not active
****************************************************************************************/
SYS_Time_t SYS_TimeoutRemain(SYS_TimeoutTaskServiceField_t *timeoutService)
{
    if (0 == timeoutService->counter
            || NULL == SYS_QueueFindParentElement(MM_GET_TIMEOUT_TASKS_QUEUE(), &timeoutService->queueElement))
        return 0;

    return TIMEOUT_TASK_TICKS_TO_MS(timeoutService->counter);
}