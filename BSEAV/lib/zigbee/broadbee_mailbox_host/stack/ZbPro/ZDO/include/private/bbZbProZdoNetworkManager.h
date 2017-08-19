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
 *      This header describes private types and API for the ZDO Network Manager component.
 *
*******************************************************************************/

#ifndef _ZBPRO_ZDO_NETWORK_MANAGER_H
#define _ZBPRO_ZDO_NETWORK_MANAGER_H

/************************* INCLUDES *****************************************************/
#include "bbSysEvent.h"
#include "bbZbProNwkSapTypesNetworkFormation.h"
#include "bbZbProNwkSapTypesJoin.h"
#include "bbZbProNwkSapTypesStartRouter.h"
#include "bbZbProNwkSapTypesPermitJoining.h"
#include "bbZbProNwkSapTypesLeave.h"

#include "bbZbProAps.h"

#include "bbZbProZdoSapTypesNetworkManager.h"
#include "bbZbProZdoSapTypesDeviceAnnce.h"

#include "private/bbZbProZdoSecurityManager.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief ZDO Network Manager internal descriptor.
 */
typedef struct _ZbProZdoNetworkManagerDescr_t
{
    union
    {
        /* start device part */
        ZBPRO_NWK_NetworkFormationReqDescr_t    formationReq;
        ZBPRO_NWK_JoinReqDescr_t                joinReq;
        ZBPRO_NWK_PermitJoiningReqDescr_t       permitJoinReq;
        ZBPRO_NWK_StartRouterReqDescr_t         startRouterReq;
        ZBPRO_APS_StartStopReqDescr_t           apsStartReq;
        ZBPRO_ZDO_DeviceAnnceReqDescr_t         deviceAnnceReq;

        /* leave part */
        ZBPRO_NWK_LeaveReqDescr_t               leaveReq;
        ZBPRO_APS_StartStopReqDescr_t           apsStopReq;
    };

    SYS_EventHandlerParams_t        eventHandler;
    SYS_FSM_Descriptor_t            startFsm;
    SYS_FSM_Descriptor_t            leaveFsm;
    SYS_QueueDescriptor_t           startQueue;
    ZBPRO_ZDO_CommissioningInfo_t   commissionInfo;
    ZBPRO_NWK_NwkAddr_t             childNwkAddress;
    bool                            isPendingFinishPart     : 1;
    bool                            isPendingItselfLeave    : 1;
    bool                            rejoinAfterItselfLeave  : 1;
    bool                            removeItselfChilds      : 1;
    ZBPRO_ZDO_Status_t              authentificationStatus;
} ZbProZdoNetworkManagerDescr_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/*************************************************************************************//**
    \brief ZDO Network Manager start network handler.
*****************************************************************************************/
void zbProZdoNetworkManagerStartHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*************************************************************************************//**
    \brief ZDO Network Manager leave request/indication handler.
*****************************************************************************************/
void zbProZdoNetworkManagerLeaveHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*************************************************************************************//**
    \brief ZDO Network Manager join indication handler.
*****************************************************************************************/
void zbProZdoNetworkManagerJoinHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*************************************************************************************//**
    \brief ZDO Async Reset on Leave task handler
*****************************************************************************************/
void zbProZdoNetworkManagerLeaveResetHandler(SYS_SchedulerTaskDescriptor_t *const taskDescr);

/*************************************************************************************//**
  \brief Initiates the leaving procedure.
  \param[in] reqDescr - pointer to the request structure.
  \param[in] deviceAddress  - extended device address.
  \param[in] rejoin         - true if device should start the rejoin procedure after leave.
  \param[in] removeChildren - true if children should be removed.
*****************************************************************************************/
void zbProZdoNetworkManagerLeave(const ZBPRO_NWK_ExtAddr_t deviceAddress, const bool rejoin, const bool removeChildren);

/*************************************************************************************//**
  \brief Resets ZDO network manager start routine.
*****************************************************************************************/
void zbProZdoNetworkMangerStartReset(void);

/*************************************************************************************//**
  \brief Resets ZDO network manager leave routine.
*****************************************************************************************/
void zbProZdoNetworkMangerLeaveReset(void);

/*************************************************************************************//**
  \brief Resets all routines of the ZDO network manager.
*****************************************************************************************/
INLINE void zbProZdoNetworkManagerReset(void)
{
    zbProZdoNetworkMangerStartReset();
    zbProZdoNetworkMangerLeaveReset();
}

#endif /* _ZBPRO_ZDO_NETWORK_MANAGER_H */

/* eof bbZbProZdoNetworkManager.h */