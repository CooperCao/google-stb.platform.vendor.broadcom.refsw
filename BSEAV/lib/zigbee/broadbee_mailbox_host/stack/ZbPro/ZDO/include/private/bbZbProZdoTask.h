/******************************************************************************
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
 *****************************************************************************/

/******************************************************************************
*
* DESCRIPTION:
*       ZDO layer task handlers interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDO_H
#define _BB_ZBPRO_ZDO_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of ZDO layer task handlers.
 * \note
 *  Do not change the relative order of identifiers. It may significantly affect behavior
 *  of ZDO.
 */
typedef enum _ZbProZdoHandlerId_t
{
    ZBPRO_ZDO_SECURITY_MANAGER_LAZY_HANDLER_ID,

    ZBPRO_ZDO_SECURITY_MANAGER_HANDLER_ID,

    ZBPRO_ZDO_NETWORK_MANAGER_LEAVE_ID,

    ZBPRO_ZDO_NETWORK_MANAGER_START_ID,

    ZBPRO_ZDO_NETWORK_MANAGER_JOIN_IND_ID,

    ZBPRO_ZDO_NETWORK_MANAGER_LEAVE_RESET_ID,

    ZBPRO_ZDO_CHANNEL_MANAGER_START_ID,

    ZBPRO_ZDO_HANDLERS_AMOUNT,                          /*!< Number of ZDO layer task handlers. Must be the last line in
                                                            the list. */
} ZbProZdoHandlerId_t;


/**//**
 * \brief   Data type for ZDO layer task descriptor.
 */
typedef SYS_SchedulerTaskDescriptor_t  ZbProZdoTaskDescr_t;


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Resets all modules of the ZDO layer.
 */
ZDO_PRIVATE void bbZbProZdoReset(void);


/**//**
 * \brief   Schedules ZDO layer task.
 * \param[in]   handlerId       Identifier of a ZDO task handler.
 */
ZDO_PRIVATE void zbProZdoPostTask(const ZbProZdoHandlerId_t  handlerId);


/**//**
 * \brief   Recalls previously scheduled task handler processing of ZDO layer.
 * \param[in]   handlerId       Identifier of the ZDO layer task handler to be recalled.
 */
ZDO_PRIVATE void zbProZdoRecallTask(const ZbProZdoHandlerId_t  handlerId);


/**//**
 * \brief   Engages the system timer to schedule ZDO layer timeout task after the
 *  specified timeout.
 * \param[in]   timeoutTask     Pointer to the timeout task descriptor.
 * \param[in]   timerMode       Desired timer mode.
 */
ZDO_PRIVATE void zbProZdoPostTimeoutTask(
                SYS_TimeoutTask_t    *const  timeoutTask,
                const SYS_TimeoutTaskMode_t  timerMode);


/**//**
 * \brief   Recalls the previously engaged timeout timer and its timeout task.
 * \param[in]   timeoutTask     Pointer to timeout task descriptor.
 * \note
 *  If the specified timer has already triggered and the timeout task was scheduled by it
 *  but was not yet activated by the Task Scheduler, such a timeout task is also recalled
 *  on this function call.
 */
ZDO_PRIVATE void zbProZdoRecallTimeoutTask(SYS_TimeoutTask_t *const  timeoutTask);


#endif /* _BB_ZBPRO_ZDO_H */

/* eof bbZbProZdoTask.h */