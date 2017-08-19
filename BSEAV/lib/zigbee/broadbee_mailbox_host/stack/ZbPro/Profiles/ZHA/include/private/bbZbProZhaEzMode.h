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
*       ZHA layer EZ-mode procedure interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZHA_EZ_MODE_H_
#define _BB_ZBPRO_ZHA_EZ_MODE_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapGetSet.h"
#include "bbZbProZdoSapTypesGetSimpleDescHandler.h"
#include "bbZbProZdoSapTypesNetworkManager.h"
#include "bbZbProZdoSapTypesNodeManager.h"
#include "bbZbProZdoSapTypesDiscoveryManager.h"

#include "bbZbProApsSapBindUnbind.h"
#include "bbZbProZdoSapTypesBindingManager.h"

#include "bbZbProZclSapClusterIdentify.h"

#include "bbZbProZhaCommon.h"

typedef struct
{
    ZBPRO_APS_ShortAddr_t   shortAddr;
    ZBPRO_APS_EndpointId_t  endpoint;
} ZbProZhaEzModeAddr_t;

typedef struct
{
    SYS_QueueDescriptor_t                   queue;
    SYS_FSM_Descriptor_t                    fsm;
    SYS_TimeoutSignal_t                     timer;

    uint8_t                                 roundsLeft;
    ZBPRO_APS_SimpleDescriptor_t            selfSd;
    ZBPRO_APS_SimpleDescriptor_t            remoteSd;
    ZbProZhaEzModeAddr_t                    peerList[ZBPRO_ZHA_EZ_FIND_TABLE_SIZE];
    uint8_t                                 seekEp;
    uint8_t                                 seekCl;
    ZBPRO_APS_ClusterId_t                   clusterToBind;
    ZBPRO_APS_EndpointId_t                  endpointUnderResolving;

    union
    {
        ZBPRO_APS_SetReqDescr_t                         aspSet;
        ZBPRO_ZDO_StartNetworkReqDescr_t                zdoStart;
        ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t           zdpPermJoin;
        ZBPRO_ZDO_AddrResolvingReqDescr_t               zdpIeee;
        ZBPRO_ZCL_IdentifyCmdIdentifyReqDescr_t         zclIdentify;
        ZBPRO_ZCL_IdentifyCmdIdentifyQueryReqDescr_t    zclIdentifyQuery;
    };

    union
    {
        ZBPRO_APS_BindUnbindReqDescr_t      apsBind;
        ZBPRO_ZDO_BindUnbindReqDescr_t      zdpBind;
        ZBPRO_ZDO_SimpleDescReqDescr_t      zdpSimple;
    };


} zbProZhaEzModeDescriptor_t;

/************************* PROTOTYPES ***************************************************/
void zbProZhaEzModeHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
void zbProZhaEzModeReset(void);

#endif /* _BB_ZBPRO_ZHA_EZ_MODE_H_ */

/* eof bbZbProZhaEzMode.h */