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
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysTimeoutTask.h $
*
* DESCRIPTION:
*   Timeout task handler.
*
* $Revision: 2186 $
* $Date: 2014-04-14 10:55:48Z $
*
****************************************************************************************/

#ifndef _SYS_TIMEOUTTASK_H
#define _SYS_TIMEOUTTASK_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysTaskScheduler.h"     /* Task Scheduler interface.     */
#include "bbSysTime.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Converts time from seconds to milliseconds.
 *        TODO: Move to the appropriate place.
 */
#define SYS_CONVERT_S_TO_MS(time_s) ((time_s) * 1000UL)

/************************* TYPES *******************************************************/

/**//**
 * \brief Enumeration of Timeout Task modes.
  */
typedef enum _SYS_TimeoutTaskMode_t
{
    TIMEOUT_TASK_ONE_SHOT_MODE,
    TIMEOUT_TASK_REPEAT_MODE
} SYS_TimeoutTaskMode_t;

/**//**
 * \brief Service field of the Timeout Task.
  */
typedef struct _SYS_TimeoutTaskServiceField_t
{
    SYS_QueueElement_t queueElement;         /*!< Service field for putting the task to the queue. */
    uint8_t flags;                           /*!< Flags to manipulate the posted task behavior. */
    uint32_t counter;                        /*!< Countdown counter in tics. . */
} SYS_TimeoutTaskServiceField_t;

/**//**
 * \brief Type describes the structure of a timeout task.
  */
typedef struct _SYS_TimeoutTask_t
{
    SYS_TimeoutTaskServiceField_t service;   /*!< Service field. */

    uint32_t timeout;                        /*!< Timeout in milliseconds. */
    SYS_SchedulerTaskDescriptor_t *taskDescriptor; /*!< Pointer to the task descriptor to be launched in timeout milliseconds. */
    uint8_t handlerId;                       /*!< Identifier of the task to be launched in timeout milliseconds. */
} SYS_TimeoutTask_t;

/**//**
 * \brief Type describes a pointer to callback function of Timeout Signal.
  */
typedef void (*SYS_TimeoutSignalCalback_t)(SYS_TimeoutTaskServiceField_t *const timeoutService);

/**//**
 * \brief Type describes the structure of a timeout task.
  */
typedef struct _SYS_TimeoutSignal_t
{
    SYS_TimeoutTaskServiceField_t service;   /*!< Service field. */

    uint32_t timeout;                        /*!< Timeout in milliseconds. */
    SYS_TimeoutSignalCalback_t  callback;    /*!< Callback which should be called when timeout is expired. */
} SYS_TimeoutSignal_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Reset routine for Timeout Task module.
****************************************************************************************/
void SYS_TimeoutTaskHandlerReset(void);

/************************************************************************************//**
  \brief Must be called by HAL each HAL_TIMEOUT_TASK_TIMER_GRANULARITY milliseconds.
****************************************************************************************/
void SYS_TimeoutTaskHandlerTick(void);

/************************************************************************************//**
  \brief Posts timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutTaskPost(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode);

/************************************************************************************//**
  \brief Re-posts timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutTaskRePost(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode);

/************************************************************************************//**
  \brief Remove timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
****************************************************************************************/
void SYS_TimeoutTaskRemove(SYS_TimeoutTask_t *timeoutTask);

/************************************************************************************//**
  \brief Starts timeout signal.
  \param[in] timeoutSignal - pointer to target timeout signal structure.
  \param[in] mode - required timer mode.
****************************************************************************************/
void SYS_TimeoutSignalStart(SYS_TimeoutSignal_t *timeoutSignal, SYS_TimeoutTaskMode_t mode);

/************************************************************************************//**
  \brief Stops timeout signal.
  \param[in] timeoutSignal - pointer to target timeout signal structure.
****************************************************************************************/
void SYS_TimeoutSignalStop(SYS_TimeoutSignal_t *timeoutSignal);

/************************************************************************************//**
  \brief Returns remaining time in ms to fire
  \param[in] timeoutService - pointer to timeout service structure.
****************************************************************************************/
SYS_Time_t SYS_TimeoutRemain(SYS_TimeoutTaskServiceField_t *timeoutService);

#endif /* _SYS_TIMEOUTTASK_H */
/* eof bbSysTimeoutTask.h */