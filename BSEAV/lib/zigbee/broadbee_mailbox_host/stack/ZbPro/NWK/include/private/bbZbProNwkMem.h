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
*       Contains zigbee network descriptor definition and interface for it.
*
*******************************************************************************/

#ifndef _ZBPRO_NWK_MEM_H
#define _ZBPRO_NWK_MEM_H

/************************* INCLUDES ****************************************************/
#include "private/bbZbProNwkNeighborTable.h"
#include "private/bbZbProNwkAddressMap.h"
#include "private/bbZbProNwkRoutingTable.h"
#include "private/bbZbProNwkSrcRouteTable.h"
#include "private/bbZbProNwkBroadcastTable.h"
#include "private/bbZbProNwkRouteDiscoveryTable.h"
#include "private/bbZbProNwkNibPrivate.h"

#include "private/bbZbProNwkSecurity.h"
#include "private/bbZbProNwkBufferManager.h"
#include "private/bbZbProNwkDispatcher.h"
#include "private/bbZbProNwkTxFsm.h"
#include "private/bbZbProNwkRxHandler.h"

#include "private/bbZbProNwkPanIdConflict.h"
#include "private/bbZbProNwkAddrConflict.h"
#include "private/bbZbProNwkLinkStatus.h"
#include "private/bbZbProNwkStatusService.h"
#include "private/bbZbProNwkJoinCommon.h"
#include "private/bbZbProNwkRejoinReq.h"
#include "private/bbZbProNwkRejoinResp.h"
#include "private/bbZbProNwkLeave.h"
#include "private/bbZbProNwkRouteRecord.h"
#include "private/bbZbProNwkUpdate.h"
#include "private/bbZbProNwkReport.h"

#include "private/bbZbProNwkReset.h"
#include "private/bbZbProNwkData.h"
#include "private/bbZbProNwkRouteDiscovery.h"
#include "private/bbZbProNwkManyToOneDiscovery.h"
#include "private/bbZbProNwkGetSet.h"
#include "private/bbZbProNwkDiscovery.h"
#include "private/bbZbProNwkPermitJoining.h"
#include "private/bbZbProNwkStartRouter.h"
#include "private/bbZbProNwkNetworkFormation.h"
#include "private/bbZbProNwkState.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief NWK layer descriptor type.
 */
typedef struct _ZbProNwkDescriptor_t
{
    ZbProNwkStateDescr_t                state;
    ZbProNwkNib_t                       nib;
    ZbProNwkGetSetDescr_t               getSetService;

    /* TODO: Change a visibility of the Neighbor Table type from public to private. */
    ZbProNwkNeighborTableDescr_t        neighborTable;
    ZbProNwkAddressMapDescriptor_t      addressMap;
    ZbProNwkRoutingTable_t              routingTable;
    ZbProNwkSrcRouteTable_t             srcRouteTable;
    ZbProNwkBTT_t                       btt;
    ZbProNwkRouteDiscoveryTable_t       discoveryTable;
    ZbProNwkSecurityMaterialStorage_t   securityMaterial;

    /* Memory descriptor for internal services. */
    ZbProNwkPacketManagerDescr_t        packetManager;
    ZbProNwkDispatcherDescr_t           dispatcher;
    ZbProTxFsmDescr_t                   txFsm;
    ZbProRxHandlerDescr_t               rxHandler;

    ZbProNwkPanIdConflictServiceDescr_t panIdConflictService;
    ZbProNwkAddrConflictServiceDescr_t  addrConflictService;
    ZbProNwkLinkStatusServiceDescr_t    linkStatusService;
    ZbProNwkStatusServiceDescr_t        networkStatusService;
    ZbProNwkJoinServiceDescriptor_t     commonJoinService;
    ZbProNwkRejoinRequestServiceDescr_t rejoinReqService;
    ZbProNwkRejoinResponseServiceDescr_t rejoinRespService;
    ZbProNwkLeaveServiceDescr_t         leaveService;
    ZbProNwkDataServiceDescr_t          dataService;
    ZbProNwkRouteRecordDescr_t          routeRecord;
    ZbProNwkReportRequestServiceDescr_t networkReport;
    ZbProNwkUpdateRequestServiceDescr_t networkUpdate;
    ZbProNwkRouteDiscoveryDescr_t       routeDiscovery;
    ZbProNwkManyToOneDiscoveryServiceDescr_t mtoDiscovery;

    ZbProNwkPermitJoiningDescr_t        permitJoiningService;
    ZbProNwkDiscoveryServiceDescr_t     discoveryService;
    ZbProNwkResetHandlerDescr_t         resetHandler;
    ZbProNwkPassiveAckDescr_t           passiveAck;
    ZbProNwkStartRouterHandlerDescr_t   startRouterHandler;
    ZbProNwkNetworkFormationHandlerDescr_t networkFormationHandler;
} ZbProNwkDescriptor_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Returns a pointer to the common NWK layer descriptor.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkDescriptor_t nwkDescriptor;

/************************************************************************************//**
 \brief Returns a pointer to the NWK Information base.
 ****************************************************************************************/
INLINE ZbProNwkNib_t *zbProNwkNib(void)
{
    return &nwkDescriptor.nib;
}

/************************************************************************************//**
  \brief Returns a pointer to NLME-GET, NLME-SET Service Descriptor.
****************************************************************************************/
INLINE ZbProNwkGetSetDescr_t *zbProNwkGetSetDescr(void)
{
    return &nwkDescriptor.getSetService;
}

/************************************************************************************//**
  \brief Returns a pointer to Neighbor Table.
****************************************************************************************/
INLINE ZbProNwkNeighborTableDescr_t *zbProNwkNeighborTableDescr(void)
{
    return &nwkDescriptor.neighborTable;
}

/************************************************************************************//**
  \brief First entry of the neighbor table.
****************************************************************************************/
INLINE ZBPRO_NWK_Neighbor_t *zbProNwkNeighborTableBegin(void)
{
    return zbProNwkNeighborTableDescr()->table;
}

/************************************************************************************//**
  \brief Returns a pointer to memory area after last entry of the neighbor table.
****************************************************************************************/
INLINE ZBPRO_NWK_Neighbor_t *zbProNwkNeighborTableEnd(void)
{
    return zbProNwkNeighborTableDescr()->table + ZBPRO_NWK_NEIGHBOR_TABLE_SIZE;
}

/************************************************************************************//**
  \brief Returns a pointer to Address map.
****************************************************************************************/
INLINE ZbProNwkAddressMapDescriptor_t *zbProNwkAddressMap(void)
{
    return &nwkDescriptor.addressMap;
}

/************************************************************************************//**
  \brief Returns a pointer to Routing Table.
****************************************************************************************/
INLINE ZbProNwkRoutingTable_t *zbProNwkRoutingTable(void)
{
    return &nwkDescriptor.routingTable;
}

/************************************************************************************//**
  \brief Returns a pointer to source route table.
****************************************************************************************/
INLINE ZbProNwkSrcRouteTable_t *zbProNwkSrcRouteTable(void)
{
    return &nwkDescriptor.srcRouteTable;
}

/************************************************************************************//**
  \brief Returns a pointer to Broadcast Transaction Table (BTT).
****************************************************************************************/
INLINE ZbProNwkBTT_t *zbProNwkBtt(void)
{
    return &nwkDescriptor.btt;
}

/************************************************************************************//**
  \brief Returns a pointer to the Route Discovery Table.
****************************************************************************************/
INLINE ZbProNwkRouteDiscoveryTable_t *zbProNwkRouteDiscoveryTable(void)
{
    return &nwkDescriptor.discoveryTable;
}

/************************************************************************************//**
  \brief Returns a pointer to the Route Discovery Table.
****************************************************************************************/
INLINE ZbProNwkManyToOneDiscoveryServiceDescr_t *zbProNwkManyToOneDiscoveryDescr(void)
{
    return &nwkDescriptor.mtoDiscovery;
}

/************************************************************************************//**
  \brief Returns a pointer to the Network Security Material Storage.
****************************************************************************************/
INLINE ZbProNwkSecurityMaterialStorage_t *zbProNwkSecurityMaterialStorage(void)
{
    return &nwkDescriptor.securityMaterial;
}

/************************************************************************************//**
 \brief Returns a pointer to the Packet Manager memory.
 ****************************************************************************************/
INLINE ZbProNwkPacketManagerDescr_t *zbProNwkBufferManagerDescr(void)
{
    return &nwkDescriptor.packetManager;
}

/************************************************************************************//**
 \brief Returns a pointer to the Dispatcher memory.
 ****************************************************************************************/
INLINE ZbProNwkDispatcherDescr_t *zbProNwkDispatcherDescr(void)
{
    return &nwkDescriptor.dispatcher;
}

/************************************************************************************//**
 \brief Returns a pointer to the NWK Tx Fsm memory.
 ****************************************************************************************/
INLINE ZbProTxFsmDescr_t *zbProNwkTxFsmDescr(void)
{
    return &nwkDescriptor.txFsm;
}

/************************************************************************************//**
 \brief Returns a pointer to the NWK Rx Handler memory.
 ****************************************************************************************/
INLINE ZbProRxHandlerDescr_t *zbProNwkRxHandlerDescr(void)
{
    return &nwkDescriptor.rxHandler;
}

/************************************************************************************//**
 \brief Returns a pointer to the panId conflict service memory.
 ****************************************************************************************/
INLINE ZbProNwkPanIdConflictServiceDescr_t *zbProNwkPanIdConflictServiceDescr(void)
{
    return &nwkDescriptor.panIdConflictService;
}

/************************************************************************************//**
 \brief Returns a pointer to the address conflict service memory.
 ****************************************************************************************/
INLINE ZbProNwkAddrConflictServiceDescr_t *zbProNwkAddrConflictServiceDescr(void)
{
    return &nwkDescriptor.addrConflictService;
}

/************************************************************************************//**
 \brief Returns a pointer to the link status service memory.
 ****************************************************************************************/
INLINE ZbProNwkLinkStatusServiceDescr_t *zbProNwkLinkStatusServiceDescr(void)
{
    return &nwkDescriptor.linkStatusService;
}

/************************************************************************************//**
 \brief Returns a pointer to the network status service memory.
 ****************************************************************************************/
INLINE ZbProNwkStatusServiceDescr_t *zbProNwkStatusServiceDescr(void)
{
    return &nwkDescriptor.networkStatusService;
}

/************************************************************************************//**
 \brief Returns a pointer to the common join service memory.
 ****************************************************************************************/
INLINE ZbProNwkJoinServiceDescriptor_t *zbProNwkCommonJoinServiceDescr(void)
{
    return &nwkDescriptor.commonJoinService;
}

/************************************************************************************//**
 \brief Returns a pointer to the rejoin client service memory.
 ****************************************************************************************/
INLINE ZbProNwkRejoinRequestServiceDescr_t *zbProNwkRejoinReqServiceDescr(void)
{
    return &nwkDescriptor.rejoinReqService;
}

/************************************************************************************//**
 \brief Returns a pointer to the rejoin server service memory.
 ****************************************************************************************/
INLINE ZbProNwkRejoinResponseServiceDescr_t *zbProNwkRejoinRespServiceDescr(void)
{
    return &nwkDescriptor.rejoinRespService;
}

/************************************************************************************//**
 \brief Returns a pointer to the leave server service memory.
 ****************************************************************************************/
INLINE ZbProNwkLeaveServiceDescr_t *zbProNwkLeaveServiceDescr(void)
{
    return &nwkDescriptor.leaveService;
}

/************************************************************************************//**
 \brief Returns a pointer to the Data Service memory.
 ****************************************************************************************/
INLINE ZbProNwkDataServiceDescr_t *zbProNwkDataServiceDescr(void)
{
    return &nwkDescriptor.dataService;
}

/************************************************************************************//**
  \brief Returns a pointer to Permit Joining Descriptor.
****************************************************************************************/
INLINE ZbProNwkPermitJoiningDescr_t *zbProNwkPermitJoiningDescr(void)
{
    return &nwkDescriptor.permitJoiningService;
}

/************************************************************************************//**
  \brief Returns a pointer to discovery service descriptor.
****************************************************************************************/
INLINE ZbProNwkDiscoveryServiceDescr_t *zbProNwkdiscoveryServiceDescr(void)
{
    return &nwkDescriptor.discoveryService;
}

/************************************************************************************//**
  \brief Returns a pointer to Route Record Descriptor.
****************************************************************************************/
INLINE ZbProNwkRouteRecordDescr_t *zbProNwkRouteRecordDescr(void)
{
    return &nwkDescriptor.routeRecord;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Report command handler Descriptor.
****************************************************************************************/
INLINE ZbProNwkReportRequestServiceDescr_t *zbProNwkReportDescr(void)
{
    return &nwkDescriptor.networkReport;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Update command handler Descriptor.
****************************************************************************************/
INLINE ZbProNwkUpdateRequestServiceDescr_t *zbProNwkUpdateDescr(void)
{
    return &nwkDescriptor.networkUpdate;
}

/************************************************************************************//**
  \brief Returns a pointer to Route Discovery service Descriptor.
****************************************************************************************/
INLINE ZbProNwkRouteDiscoveryDescr_t *zbProNwkRouteDiscoveryDescr(void)
{
    return &nwkDescriptor.routeDiscovery;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Reset handler Descriptor.
****************************************************************************************/
INLINE ZbProNwkResetHandlerDescr_t *zbProNwkResetHandlerDescr(void)
{
    return &nwkDescriptor.resetHandler;
}

/************************************************************************************//**
  \brief Returns a pointer to Passive Ack module descriptor.
****************************************************************************************/
INLINE ZbProNwkPassiveAckDescr_t *zbProNwkPassiveAckDescr(void)
{
    return &nwkDescriptor.passiveAck;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Start Router handler Descriptor.
****************************************************************************************/
INLINE ZbProNwkStartRouterHandlerDescr_t *zbProNwkStartRouterHandlerDescr(void)
{
    return &nwkDescriptor.startRouterHandler;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Formation handler Descriptor.
****************************************************************************************/
INLINE ZbProNwkNetworkFormationHandlerDescr_t *zbProNwkNetworkFormationHandlerDescr(void)
{
    return &nwkDescriptor.networkFormationHandler;
}

/************************************************************************************//**
  \brief Returns a pointer to Network Global State.
****************************************************************************************/
INLINE ZbProNwkStateDescr_t *zbProNwkState(void)
{
    return &nwkDescriptor.state;
}

#endif /* _ZBPRO_NWK_MEM_H */

/* eof bbZbProNwkMem.h */
