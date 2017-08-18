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
 *      This is the general header file for the ZigBee PRO NWK component.
 *
*******************************************************************************/

#ifndef _ZBPRO_NWK_H
#define _ZBPRO_NWK_H

/************************* INCLUDES ****************************************************/
#include "bbSysTimeoutTask.h"
#include "bbSysTaskScheduler.h"
#include "bbZbProNwkAttributes.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Identifiers of the NWK layer handlers.
 */
typedef enum _ZBPRO_NWK_HandlerId_t
{
    ZBPRO_NWK_RESET_HANDLER_HANDLER_ID              = 0x00,
    ZBPRO_NWK_RX_HANDLER_ID                         = 0x01,
    ZBPRO_NWK_ADDRESS_CONFLICT_HANDLER_ID           = 0x02,
    ZBPRO_NWK_TX_TIMEOUT_HANDLER_ID                 = 0x03,
    ZBPRO_NWK_DISPATCHER_HANDLER_ID                 = 0x04,
    ZBPRO_NWK_LEAVE_HANDLER_ID                      = 0x05,
    ZBPRO_NWK_ROUTE_DISCOVERY_HANDLER_ID            = 0x06,
    ZBPRO_NWK_ROUTE_DISCOVERY_TABLE_HANDLER_ID      = 0x07,
    ZBPRO_NWK_START_ROUTER_HANDLER_ID               = 0x08,
    ZBPRO_NWK_PERMIT_JOINING_HANDLER_ID             = 0x09,
    ZBPRO_NWK_DISCOVERY_HANDLER_ID                  = 0x0A,
    ZBPRO_NWK_NETWORK_FORMATION_HANDLER_ID          = 0x0B,
    ZBPRO_NWK_JOIN_COMMON_HANDLER_ID                = 0x0C,
    ZBPRO_NWK_ROUTE_RECORD_HANDLER_ID               = 0x0D,
    ZBPRO_NWK_DATA_REQUEST_HANDLER_ID               = 0x0E,
    ZBPRO_NWK_NEIGHBOR_TABLE_HANDLER_ID             = 0x0F,
    ZBPRO_NWK_GET_SET_HANDLER_ID                    = 0x10,
    ZBPRO_NWK_LINK_STATUS_HANDLER_ID                = 0x11,
    ZBPRO_NWK_HANDLERS_AMOUNT,
} ZBPRO_NWK_HandlerId_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Posts handling task within ZB PRO NWK layer task.
 \param[in] handlerId - identifier of the NWK layer handler to be called.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkPostTask(ZBPRO_NWK_HandlerId_t handlerId);

/************************************************************************************//**
  \brief Recalls handling task within ZB PRO NWK layer task.
  \param[in] handlerId - identifier of the NWK layer handler to be recalled.
****************************************************************************************/
NWK_PRIVATE void zbProNwkRecallTask(ZBPRO_NWK_HandlerId_t handlerId);

/************************************************************************************//**
  \brief Posts timeout task within ZB PRO NWK layer task.
  \param[in] timeoutTask - pointer to the timeout task.
  \param[in] mode - required timer mode.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkPostTimeoutTask(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode);

/************************* INLINE FUNCTION PROTOTYPES **********************************/
/************************************************************************************//**
  \brief Remove timeout task.
  \param[in] timeoutTask - pointer to target timeout task.
 ****************************************************************************************/
INLINE void zbProNwkRemoveTimeoutTask(SYS_TimeoutTask_t *timeoutTask)
{
    SYS_TimeoutTaskRemove(timeoutTask);
}

#endif /* _ZBPRO_NWK_H */

/* eof bbZbProNwk.h */
