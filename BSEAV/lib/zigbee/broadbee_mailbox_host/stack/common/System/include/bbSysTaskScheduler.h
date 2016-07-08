/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysTaskScheduler.h $
*
* DESCRIPTION:
*   Task Scheduler interface.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _SYS_TASKSCHEDULER_H
#define _SYS_TASKSCHEDULER_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysQueue.h"             /* Queues engine interface.      */

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief   List of task priorities to be used.
 * \note    The most significant bit of the priority field is reserved for the Task
 *  Scheduler internal use. So, the maximum allowed value of the priority level is 0x7F.
 * \note    The value 0x00 is reserved as the start-up value for the noninitialized
 *   tasks descriptor priority field. So, the minimum allowed value of the priority level
 *   is 0x01. This value is used as a marker of noninitialized tasks descriptor for the
 *   case when it is initialized dynamically.
 */
typedef enum _SYS_SchedulerTaskPriorityLevel_t
{
    SYS_SCHEDULER_NONASSIGNED_PRIORITY   = 0x00,        /*!< This value is just for initialization of the priority field
                                                            of tasks descriptor on power-on reset. It shall not be used
                                                            by any unit as its tasks priority level. */

    SYS_SCHEDULER_MAC_PRIORITY           = 0x01,        /*!< The MAC tasks priority level. */
    SYS_SCHEDULER_HAL_PRIORITY           = 0x02,        /*!< The HAL tasks priority level. */
    SYS_SCHEDULER_MAILBOX_PRIORITY       = 0x03,        /*!< The MAILBOX tasks priority level. */

    SYS_SCHEDULER_ZBPRO_NWK_PRIORITY     = 0x10,
    SYS_SCHEDULER_ZBPRO_APS_PRIORITY     = 0x11,
    SYS_SCHEDULER_ZBPRO_ZDO_PRIORITY     = 0x12,
    SYS_SCHEDULER_ZBPRO_TC_PRIORITY      = 0x13,
    SYS_SCHEDULER_ZBPRO_HA_PRIORITY      = 0x14,

    SYS_SCHEDULER_RF4CE_NWK_PRIORITY     = 0x10,
    SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY = 0x14,
    SYS_SCHEDULER_RF4CE_GDP_PRIORITY     = SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY,
    SYS_SCHEDULER_RF4CE_ZRC_PRIORITY     = SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY,
    SYS_SCHEDULER_RF4CE_MSO_PRIORITY     = SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY,
    SYS_SCHEDULER_RF4CE_PRIORITY         = SYS_SCHEDULER_RF4CE_PROFILE_PRIORITY,

    SYS_SCHEDULER_NVM_PRIORITY           = 0x20,

    SYS_SCHEDULER_TERM_TASK_PRIORITY     = 0x7F,    /*!< Priority of the User Terminal task. */
    SYS_SCHEDULER_TEST_TASK_PRIORITY     = 0x7F,    /*!< Priority of the Unit Test task. */

} SYS_SchedulerTaskPriorityLevel_t;

/**//**
 * \brief The maximum allowed number of handlers for a task.
 */
#define SYS_SCHEDULER_HANDLERS_MAX_NUMBER     32

/************************* TYPES *******************************************************/
/**//**
 * \brief Type for task handler identifier.
 */
typedef uint8_t SYS_SchedulerTaskHandlerId_t;

/**//**
 * \brief Prototype of the task descriptor type.
 */
typedef struct _SYS_SchedulerTaskDescriptor_t SYS_SchedulerTaskDescriptor_t;

/**//**
 * \brief Type of task's handler function.
 */
typedef void (*SYS_SchedulerTaskHandler_t)(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief Type for storing a task priority.
 */
typedef uint8_t SYS_SchedulerPriority_t;

/**//**
 * \brief Type describes the structure of task descriptor.
 */
struct _SYS_SchedulerTaskDescriptor_t
{
    SYS_QueueElement_t                qElem;            /*!< Service field for putting the task to the queue. */

    SYS_SchedulerPriority_t           priority;         /*!< Task priority. */

    const SYS_SchedulerTaskHandler_t *handlers;         /*!< Array of task handlers (32 max). */

    uint32_t                          handlersMask;     /*!< Each bit reflects whether the corresponding handler
                                                            shall be called or not. */
};

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Initializes scheduler.
****************************************************************************************/
void SYS_SchedulerInit(void);

/************************************************************************************//**
  \brief Runs pending task with the highest priority.
  \return True if any handler was called, false otherwise.
****************************************************************************************/
bool SYS_SchedulerRunTask(void);

/************************************************************************************//**
  \brief Posts task.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be called.
****************************************************************************************/
void SYS_SchedulerPostTask(SYS_SchedulerTaskDescriptor_t *task,
                           SYS_SchedulerTaskHandlerId_t handlerId);

/************************************************************************************//**
  \brief Recalls task.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be recalled.
****************************************************************************************/
void SYS_SchedulerRecallTask(SYS_SchedulerTaskDescriptor_t *task,
                             SYS_SchedulerTaskHandlerId_t handlerId);

/************************************************************************************//**
  \brief Checks if the specified task and its handler are presented in the queue.
  \param[in] task - pointer to target task.
  \param[in] handlerId - identifier of the handler to be called.
****************************************************************************************/
bool SYS_SchedulerIsTaskScheduled(SYS_SchedulerTaskDescriptor_t *task,
                                  SYS_SchedulerTaskHandlerId_t handlerId);

#endif /* _SYS_TASKSCHEDULER_H */
/* eof bbSysScheduler.h */