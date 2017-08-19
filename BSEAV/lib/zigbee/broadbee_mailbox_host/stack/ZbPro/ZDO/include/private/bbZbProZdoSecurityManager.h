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
 *      This header describes types and API for the ZDO Security Manager component.
 *
*******************************************************************************/

#ifndef _ZBPRO_ZDO_SECURITY_MANAGER_H
#define _ZBPRO_ZDO_SECURITY_MANAGER_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"
#include "bbZbProNwkSap.h"
#include "bbZbProApsSapSecurityServices.h"
#include "bbZbProZdoCommon.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief ZDO Security Manager possible device status enumeration.
 */
typedef enum _ZbProZdoDeviceStatus_t
{
    ZBPRO_ZDO_AUTHENTICATION_SUCCESS_STATUS,
    ZBPRO_ZDO_REMOVE_DEVICE_STATUS
} ZbProZdoDeviceStatus_t;

/**//**
 * \brief ZDO Security Manager lazy part state enumeration.
 */
typedef enum _ZbProZdoSecurityManagerLazyState_t
{
    IS_IDLE = 0,
    IS_BUSY = 1,
} ZbProZdoSecurityManagerLazyState_t;

/**//**
 * \brief ZDO Security Manager internal descriptor.
 */
typedef struct _ZbProZdoSecurityManagerDescr_t
{
    SYS_FSM_StateId_t                   state;
    SYS_TimeoutTask_t                   timer;
    ZbProSspNwkKeySeqNum_t              keySeqNumber;
    //ZBPRO_NWK_Neighbor_t                *neighbor; // TODO: not used and can be deleted.
    ZBPRO_APS_UpdateDeviceReqDescr_t    updateDevReq;

    ZbProZdoSecurityManagerLazyState_t  lazyPartState;
    ZBPRO_ZDO_Status_t                  postponedStatus;
} ZbProZdoSecurityManagerDescr_t;

/**//**
 * \brief ZDO Security manager commissioning information type.
 */
typedef struct _ZBPRO_ZDO_CommissioningInfo_t
{
    ZBPRO_NWK_RejoinMethod_t    joinMethod;
} ZBPRO_ZDO_CommissioningInfo_t;

/**//**
 * \brief Type of ZDO Security Manager starting confirmation primitive.
 * \param[in] status - result status of security authentication procedure.
 */
typedef void (ZbProZdoSecurityManagerStartConfirm_t)(const ZBPRO_ZDO_Status_t status);

/************************* FUNCTION PROTOTYPES *****************************************/
/*************************************************************************************//**
    \brief Resets component internal memory.
*****************************************************************************************/
void zbProZdoSecurityManagerReset(void);

/*************************************************************************************//**
    \brief Starts ZDO Security Manager authentication procedure.
    \param[in] info - additional parameters for commissioning procedure.
*****************************************************************************************/
void zbProZdoSecurityManagerStart(ZBPRO_ZDO_CommissioningInfo_t *const info);

/*************************************************************************************//**
    \brief Internal ZDO layer callback for security authentication procedure.
*****************************************************************************************/
ZDO_PRIVATE ZbProZdoSecurityManagerStartConfirm_t zbProZdoSecurityManagerStartConfirm;

/*************************************************************************************//**
    \brief ZDO Security Manager handler needed to raise lazy start confirmation.
*****************************************************************************************/
void zbProZdoSecurityManagerLazyStatusHandler(SYS_SchedulerTaskDescriptor_t *const taskDescr);

/*************************************************************************************//**
    \brief ZDO Security Manager timer event handler.
*****************************************************************************************/
void zbProZdoSecurityManagerHandler(SYS_SchedulerTaskDescriptor_t *const taskDescr);

/*************************************************************************************//**
    \brief Initiates sending of Update Device to the Trust Center.
*****************************************************************************************/
void zbProZdoSecurityManagerSendUpdateDevice(const ZBPRO_NWK_ExtAddr_t *const extAddr, const ZBPRO_NWK_NwkAddr_t nwkAddr,
        const ZBPRO_APS_UpdateDeviceStatus_t status);

/*************************************************************************************//**
    \brief Notifies Network Manager component about new device status.
    \param[in] deviceAddress - extended address of the device.
    \param[in] status - new status.
*****************************************************************************************/
void zbProZdoSecuritykManagerAuthStatusInd(ZBPRO_NWK_ExtAddr_t const deviceAddress, const ZbProZdoDeviceStatus_t status);

#endif /* _ZBPRO_ZDO_SECURITY_MANAGER_H */

/* eof bbZbProZdoSecurityManager.h */