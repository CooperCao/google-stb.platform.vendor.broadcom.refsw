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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/private/bbRF4CEMSOTasks.h $
 *
 * DESCRIPTION:
 *   This is the header file for the MSO RF4CE Profile
 *   Tasks manager.
 *
 * $Revision: 2999 $
 * $Date: 2014-07-21 13:30:43Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_TASKS_H
#define _RF4CE_MSO_TASKS_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO tasks enumeration.
 */
typedef enum _RF4CE_MSO_Tasks_t
{
    RF4CE_MSO_START_RESET_BIND_TASK = 0,
    RF4CE_MSO_BIND_PAIR_HANDLER,
    RF4CE_MSO_BIND_BLACKOUT_HANDLER,
    RF4CE_MSO_GET_SET_PA_TASK,
    RF4CE_MSO_GET_SET_RIB_TASK,
    RF4CE_MSO_BIND_CHECK_VALIDATION_HANDLER,
    RF4CE_MSO_USER_CONTROL_HANDLER
} RF4CE_MSO_Tasks_t;

/**//**
 * \brief RF4CE MSO tasks handlers enumeration.
 */
typedef enum _RF4CE_MSO_HandlersId_t
{
    RF4CE_MSO_START_HANDLER = 0,
    RF4CE_MSO_RESET_HANDLER,
    RF4CE_MSO_BIND_HANDLER,

    RF4CE_MSO_PA_GET_HANDLER,
    RF4CE_MSO_PA_SET_HANDLER,

    RF4CE_MSO_RIB_GET_HANDLER,
    RF4CE_MSO_RIB_SET_HANDLER
} RF4CE_MSO_HandlersId_t;

/**//**
 * \brief RF4CE MSO tasks filters.
 */
#define RF4CE_MSO_START_RESET_BIND_BUSY_FLAG 1
#define IS_RF4CE_MSO_START_RESET_BIND_BUSY() (0 != (mso->tasksBusy & RF4CE_MSO_START_RESET_BIND_BUSY_FLAG))
#define SET_RF4CE_MSO_START_RESET_BIND_BUSY() mso->tasksBusy |= RF4CE_MSO_START_RESET_BIND_BUSY_FLAG
#define CLEAR_RF4CE_MSO_START_RESET_BIND_BUSY() mso->tasksBusy &= ~RF4CE_MSO_START_RESET_BIND_BUSY_FLAG
#define RF4CE_MSO_PA_GETSET_BUSY_FLAG 2
#define IS_RF4CE_MSO_PA_GETSET_BUSY() (0 != (mso->tasksBusy & RF4CE_MSO_PA_GETSET_BUSY_FLAG))
#define SET_RF4CE_MSO_PA_GETSET_BUSY() mso->tasksBusy |= RF4CE_MSO_PA_GETSET_BUSY_FLAG
#define CLEAR_RF4CE_MSO_PA_GETSET_BUSY() mso->tasksBusy &= ~RF4CE_MSO_PA_GETSET_BUSY_FLAG
#define RF4CE_MSO_RIB_GETSET_BUSY_FLAG 4
#define IS_RF4CE_MSO_RIB_GETSET_BUSY() (0 != (mso->tasksBusy & RF4CE_MSO_RIB_GETSET_BUSY_FLAG))
#define SET_RF4CE_MSO_RIB_GETSET_BUSY() mso->tasksBusy |= RF4CE_MSO_RIB_GETSET_BUSY_FLAG
#define CLEAR_RF4CE_MSO_RIB_GETSET_BUSY() mso->tasksBusy &= ~RF4CE_MSO_RIB_GETSET_BUSY_FLAG
#define RF4CE_MSO_UC_BUSY_FLAG 8
#define IS_RF4CE_MSO_UC_BUSY() (0 != (mso->tasksBusy & RF4CE_MSO_UC_BUSY_FLAG))
#define SET_RF4CE_MSO_UC_BUSY() mso->tasksBusy |= RF4CE_MSO_UC_BUSY_FLAG
#define CLEAR_RF4CE_MSO_UC_BUSY() mso->tasksBusy &= ~RF4CE_MSO_UC_BUSY_FLAG

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Processes the Start Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_StartHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Reset Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_ResetHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Get Profile Attribute Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_PA_GetHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Set Profile Attribute Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_PA_SetHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Get RIB Attribute Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_GetHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Set RIB Attribute Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_SetHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Processes the Bind Request.

 \param[in] queueElement - pointer to the parameter queue element structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Bind Request Pair handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindPairHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Bind Request Blackout handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindBlackoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Validation handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_ValidateHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);


/************************************************************************************//**
 \brief Bind Request Check Validation handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindCheckValidationHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Bind Request Timeout handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Processes the User Control Request.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UserControlHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief MSO Bind Validation Timeout task handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_TargetBindTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);


/************************************************************************************//**
 \brief MSO Bind Blackout request task handler.

 \param[in] taskDescriptor - pointer to the task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_TargetBindBlackoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

#endif /* _RF4CE_MSO_TASKS_H */