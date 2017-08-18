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
*       Common declarations of a joining procedure.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_JOIN_COMMON_H
#define _ZBPRO_NWK_JOIN_COMMON_H

/************************* INCLUDES ****************************************************/
#include "bbSysFsm.h"
#include "bbZbProNwkSapTypesDiscovery.h"
#include "bbZbProNwkNeighbor.h" // TODO: should be deleted!
#include "bbZbProNwkSapTypesGetSet.h"
#include "private/bbZbProNwk.h"
#include "private/bbZbProNwkNeighborTable.h"
#include "private/bbZbProNwkRejoinReq.h"
#include "private/bbZbProNwkRejoinResp.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Join service buffers type.
 */
typedef struct _ZbProNwkJoinBuffer_t
{
    union
    {
        MAC_AssociateRespDescr_t            associateResp;
        MAC_OrphanRespDescr_t               orphanResp;
        ZbProNwkRejoinResponseDescriptor_t  rejoinResp;
    };
    ZBPRO_NWK_Neighbor_t               *child;
    Bool8_t                             isBusy;
} ZbProNwkJoinBuffer_t;

/**//**
 * \brief Join service descriptor type.
 */
typedef struct _ZbProNwkJoinServiceDescriptor_t
{
    ZbProNwkJoinBuffer_t    buffer[ZBPRO_NWK_JOIN_BUFFERS_AMOUNT];
    SYS_FSM_Descriptor_t    fsm;
    SYS_QueueDescriptor_t   queue;
    ZBPRO_NWK_Neighbor_t   *suitableParent;
    ZBPRO_NWK_NIB_ScanAttempts_t scanAttempts;
    ZBPRO_NWK_NwkAddr_t     newNwkAddr;
    union
    {
        MAC_AssociateReqDescr_t                 asscciateReq;
        ZBPRO_NWK_SetReqDescr_t                 nwkSetReq;
        ZbProNwkDiscoveryInternalReqDescr_t     discoveryReq;
        ZbProNwkRejoinReqDescriptor_t           rejoinReq;
        SYS_TimeoutSignal_t                     timer;
    };
} ZbProNwkJoinServiceDescriptor_t;
/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Resets join service.
****************************************************************************************/
NWK_PRIVATE void zbProNwkJoinReset(void);

/************************************************************************************//**
  \brief Join service task handler.
****************************************************************************************/
NWK_PRIVATE void zbProNwkJoinCommonHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/*************************************************************************************//**
  \brief Raises rejoin response indication events.
 ****************************************************************************************/
NWK_PRIVATE void zbProNwkJoinRejoinRespInd(ZbProNwkRejoinRespIndParams_t *indParams);

/************************************************************************************//**
  \brief Allocate free buffer.
  \return NULL if pool has no free buffer and pointer to the buffer otherwise.
****************************************************************************************/
NWK_PRIVATE ZbProNwkJoinBuffer_t *zbProNwkJoinAllocBuffer(void);

/************************************************************************************//**
  \brief Frees given buffer.
****************************************************************************************/
NWK_PRIVATE void zbProNwkJoinFreeBuffer(ZbProNwkJoinBuffer_t *buffer);

/************************************************************************************//**
  \brief Helper function. Raises NLME-JOIN indication primitive.
****************************************************************************************/
INLINE void zbProNwkJoinRaiseInd(ZbProNwkJoinBuffer_t *const buffer)
{
    ZBPRO_NWK_JoinIndParams_t indParams;

    indParams.capabilityInformation     = buffer->child->capability;
    indParams.extendedAddress           = buffer->child->extAddr;
    indParams.networkAddress            = buffer->child->networkAddr;
    indParams.rejoinNetwork             = buffer->child->joinMethod;
    indParams.secureRejoin              = buffer->child->isSecureJoin;

    ZBPRO_NWK_JoinInd(&indParams);
}

#endif /* _ZBPRO_NWK_JOIN_COMMON_H */

/* eof bbZbProNwkJoinCommon.h */