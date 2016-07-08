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
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkNibApi.h $
*
* DESCRIPTION:
*   NWK-NIB API interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _ZBPRO_NWK_NIB_API_H
#define _ZBPRO_NWK_NIB_API_H


/************************* INCLUDES *****************************************************/
#include "bbSysEvent.h"
#include "bbZbProNwkSapTypesIb.h"
#include "private/bbZbProNwkMem.h"
#include "private/bbZbProNwkNibApi.h"
#include "private/bbZbProNwkBeacon.h"
#include "friend/bbZbProNwkNibApiFriend.h"
#include "friend/bbZbProNwkSecurityMaterialFriend.h"

#define ZBPRO_NWK_BEACON_PAYLOAD_UPDATE_ID_OFFSET 14

/************************* INLINES ******************************************************/
#pragma Offwarn(428)
INLINE void zbProNwkNibCheckState(void)
{
    SYS_DbgAssertComplex(zbProNwkStateIsOn(ZBPRO_NWK_GET_SET_SERVICE_STATE), NWKNIBAPI_CHECK_GLOBAL_STATE);
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkPanId value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_PanId_t zbProNwkNibApiGetPanId(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkPanId;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkSequenceNumber value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_SequenceNumber_t zbProNwkNibApiGetSequenceNumber(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkSequenceNumber;
}

/*************************************************************************************//**
  \brief Increment and return value of NWK-NIB attribute nwkSequenceNumber.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_SequenceNumber_t zbProNwkNibApiGetAndIncSequenceNumber(void)
{
    zbProNwkNibCheckState();
    return ++zbProNwkNib()->nwkSequenceNumber;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkPassiveAckTimeoutOct value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_PassiveAckTimeoutOct_t zbProNwkNibApiGetPassiveAckTimeoutOct(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkPassiveAckTimeoutOct;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxBroadcastRetries value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxBroadcastRetries_t zbProNwkNibApiGetMaxBroadcastRetries(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxBroadcastRetries;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxChildren value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxChildren_t zbProNwkNibApiGetMaxChildren(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxChildren;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxDepth value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxDepth_t zbProNwkNibApiGetMaxDepth(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxDepth;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxRouters value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxRouters_t zbProNwkNibApiGetMaxRouters(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxRouters;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkNetworkBroadcastDeliveryTimeOct value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_NetworkBroadcastDeliveryTimeOct_t zbProNwkNibApiGetNetworkBroadcastDeliveryTimeOct(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkNetworkBroadcastDeliveryTimeOct;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkReportConstantCost value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ReportConstantCost_t zbProNwkNibApiGetReportConstantCost(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkReportConstantCost;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkTimeStamp value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_TimeStamp_t zbProNwkNibApiGetTimeStamp(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkTimeStamp;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkTxTotal value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_TxTotal_t zbProNwkNibApiGetTxTotal(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkTxTotal;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkSymLink value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_SymLink_t zbProNwkNibApiGetSymLink(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkSymLink;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkCapabilityInformation value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_CapabilityInformation_t zbProNwkNibApiGetCapabilityInformation(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkCapabilityInformation;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkAddrAlloc value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_AddrAlloc_t zbProNwkNibApiGetAddrAlloc(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkAddrAlloc;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkUseTreeRouting value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_UseTreeRouting_t zbProNwkNibApiGetUseTreeRouting(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkUseTreeRouting;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkManagerAddr value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ManagerAddr_t zbProNwkNibApiGetManagerAddr(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkManagerAddr;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxSourceRoute value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxSourceRoute_t zbProNwkNibApiGetMaxSourceRoute(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxSourceRoute;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkUpdateId value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_UpdateId_t zbProNwkNibApiGetUpdateId(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkUpdateId;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkTransactionPersistenceTime value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_TransactionPersistenceTime_t zbProNwkNibApiGetTransactionPersistenceTime(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkTransactionPersistenceTime;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkNetworkAddress value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_NetworkAddress_t zbProNwkNibApiGetNetworkAddress(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkNetworkAddress;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkStackProfile value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_StackProfile_t zbProNwkNibApiGetStackProfile(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkStackProfile;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkExtendedPanId value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ExtendedPanId_t zbProNwkNibApiGetExtendedPanId(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkExtendedPanId;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkUseMulticast value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_UseMulticast_t zbProNwkNibApiGetUseMulticast(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkUseMulticast;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkIsConcentrator value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_IsConcentrator_t zbProNwkNibApiGetIsConcentrator(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkIsConcentrator;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkConcentratorRadius value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ConcentratorRadius_t zbProNwkNibApiGetConcentratorRadius(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkConcentratorRadius;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkConcentratorDiscoveryTimeSec value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ConcentratorDiscoveryTimeSec_t zbProNwkNibApiGetConcentratorDiscoveryTimeSec(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkConcentratorDiscoveryTimeSec;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkSecurityLevel value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_SecurityLevel_t zbProNwkNibApiGetSecurityLevel(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkSecurityLevel;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkActiveKeySeqNumber value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_ActiveKeySeqNumber_t zbProNwkNibApiGetActiveKeySeqNumber(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkActiveKeySeqNumber;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkAllFresh value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_AllFresh_t zbProNwkNibApiGetAllFresh(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkAllFresh;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkSecureAllFrames value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_SecureAllFrames_t zbProNwkNibApiGetSecureAllFrames(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkSecureAllFrames;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkLinkStatusPeriodSec value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_LinkStatusPeriodSec_t zbProNwkNibApiGetLinkStatusPeriodSec(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkLinkStatusPeriodSec;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkRouterAgeLimit value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_RouterAgeLimit_t zbProNwkNibApiGetRouterAgeLimit(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkRouterAgeLimit;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkUniqueAddr value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_UniqueAddr_t zbProNwkNibApiGetUniqueAddr(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkUniqueAddr;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkLeaveRequestAllowed value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_LeaveRequestAllowed_t zbProNwkNibApiGetLeaveRequestAllowed(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkLeaveRequestAllowed;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkRouteRequestId value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_RouteRequestId_t zbProNwkNibApiGetRouteRequestId(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkRouteRequestId;
}

/*************************************************************************************//**
  \brief Increment and return value of NWK-NIB attribute nwkRouteRequestId.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_RouteRequestId_t zbProNwkNibApiGetAndIncRouteRequestId(void)
{
    return ++zbProNwkNib()->nwkRouteRequestId;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkMaxLinkRouteCost value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxLinkRouteCost_t zbProNwkNibApiGetMaxLinkRouteCost(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMaxLinkRouteCost;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkNetworkChannel value.
*****************************************************************************************/
INLINE PHY_Channel_t zbProNwkNibApiGetNetworkChannel(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkNetworkChannel;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkNetworkPage value.
*****************************************************************************************/
INLINE PHY_Page_t zbProNwkNibApiGetNetworkPage(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkNetworkPage;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkDeviceType value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_DeviceType_t zbProNwkNibApiGetDeviceType(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkDeviceType;
}

/*************************************************************************************//**
  \brief Returns NWK-NIB attribute nwkDeviceType value.
*****************************************************************************************/
INLINE ZBPRO_NWK_NIB_MaxDepth_t zbProNwkNibApiGetDepth(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkDepth;
}

INLINE ZBPRO_NWK_NIB_ScanAttempts_t zbProNwkNibApiGetScanAttempts(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkScanAttempts;
}

INLINE ZBPRO_NWK_NIB_TimeBtwnScans_t zbProNwkNibApiGetTimeBtwnScans(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkTimeBtwnScans;
}

INLINE ZBPRO_NWK_ExtAddr_t zbProNwkNibApiGetMirroredIeeeAddr(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkMirroredIeeeAddr;
}

INLINE bool zbProNwkNibApiGetAuthorizedFlag(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkAuthorizedFlag;
}

INLINE bool zbProNwkNibApiGetConcentratorNoRouteCache(void)
{
    zbProNwkNibCheckState();
    return zbProNwkNib()->nwkConcentratorNoRouteCache;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkPanId.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetPanId(ZBPRO_NWK_NIB_PanId_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_PAN_ID <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_PAN_ID >= newValue,
                  NWKNIBAPI_SETPAN_ID_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkPanId = newValue;
    {
        MAC_PibAttributeValue_t attr;
        attr.macPANId = newValue;
        macPibApiSet(MAC_FOR_ZBPRO_CONTEXT((MAC_PibAttributeId_t)MAC_PAN_ID, &attr, NULL));
    }
    return &zbProNwkNib()->nwkPanId;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkSequenceNumber.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetSequenceNumber(ZBPRO_NWK_NIB_SequenceNumber_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_SEQUENCE_NUMBER <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_SEQUENCE_NUMBER >= newValue,
                  NWKNIBAPI_SETSEQUENCE_NUMBER_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkSequenceNumber = newValue;
    return &zbProNwkNib()->nwkSequenceNumber;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkPassiveAckTimeoutOct.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetPassiveAckTimeoutOct(ZBPRO_NWK_NIB_PassiveAckTimeoutOct_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_PASSIVE_ACK_TIMEOUT_OCT <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_PASSIVE_ACK_TIMEOUT_OCT >= newValue,
                  NWKNIBAPI_SETPASSIVE_ACK_TIMEOUT_OCT_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkPassiveAckTimeoutOct = newValue;
    return &zbProNwkNib()->nwkPassiveAckTimeoutOct;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxBroadcastRetries.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxBroadcastRetries(ZBPRO_NWK_NIB_MaxBroadcastRetries_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_BROADCAST_RETRIES <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_BROADCAST_RETRIES >= newValue,
                  NWKNIBAPI_SETMAX_BROADCAST_RETRIES_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxBroadcastRetries = newValue;
    return &zbProNwkNib()->nwkMaxBroadcastRetries;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxChildren.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxChildren(ZBPRO_NWK_NIB_MaxChildren_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_CHILDREN <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_CHILDREN >= newValue,
                  NWKNIBAPI_SETMAX_CHILDREN_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxChildren = newValue;
    zbProNwkBeaconUpdateChildCapacity(true, ZBPRO_DEVICE_TYPE_ROUTER /* any value */);
    return &zbProNwkNib()->nwkMaxChildren;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxDepth.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxDepth(ZBPRO_NWK_NIB_MaxDepth_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_DEPTH <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_DEPTH >= newValue,
                  NWKNIBAPI_SETMAX_DEPTH_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxDepth = newValue;
    return &zbProNwkNib()->nwkMaxDepth;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxRouters.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxRouters(ZBPRO_NWK_NIB_MaxRouters_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_ROUTERS <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_ROUTERS >= newValue,
                  NWKNIBAPI_SETMAX_ROUTERS_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxRouters = newValue;
    zbProNwkBeaconUpdateChildCapacity(true, ZBPRO_DEVICE_TYPE_ROUTER /* any value */);
    return &zbProNwkNib()->nwkMaxRouters;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkNetworkBroadcastDeliveryTimeOct.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetNetworkBroadcastDeliveryTimeOct(ZBPRO_NWK_NIB_NetworkBroadcastDeliveryTimeOct_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_NETWORK_BROADCAST_DELIVERY_TIME_OCT <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_NETWORK_BROADCAST_DELIVERY_TIME_OCT >= newValue,
                  NWKNIBAPI_SETNETWORK_BROADCAST_DELIVERY_TIME_OCT_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkNetworkBroadcastDeliveryTimeOct = newValue;
    return &zbProNwkNib()->nwkNetworkBroadcastDeliveryTimeOct;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkReportConstantCost.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetReportConstantCost(ZBPRO_NWK_NIB_ReportConstantCost_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_REPORT_CONSTANT_COST <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_REPORT_CONSTANT_COST >= newValue,
                  NWKNIBAPI_SETREPORT_CONSTANT_COST_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkReportConstantCost = newValue;
    return &zbProNwkNib()->nwkReportConstantCost;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkTimeStamp.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetTimeStamp(ZBPRO_NWK_NIB_TimeStamp_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkTimeStamp = newValue;
    return &zbProNwkNib()->nwkTimeStamp;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkTxTotal.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetTxTotal(ZBPRO_NWK_NIB_TxTotal_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_TX_TOTAL <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_TX_TOTAL >= newValue,
                  NWKNIBAPI_SETTX_TOTAL_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkTxTotal = newValue;

    /* Reset transmission failure counters in the nwkNeighborTable when nwkTxTotal is assigned;
     * see ZigBee Document 053474r20, table 3.44, attribute nwkTxTotal. */
    zbProNwkNTResetTransmissionFailures();

    return &zbProNwkNib()->nwkTxTotal;
}

/*************************************************************************************//**
  \brief Increments value of the NWK-NIB attribute nwkTxTotal.
*****************************************************************************************/
INLINE void *zbProNwkNibApiIncTxTotal(void)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkTxTotal++;

    /* Reset transmission failure counters on nwkTxTotal roll over 0xFFFF;
     * see ZigBee Document 053474r20, table 3.44, attribute nwkTxTotal. */
    if (0x0000 == zbProNwkNib()->nwkTxTotal)
        zbProNwkNTResetTransmissionFailures();

    return &zbProNwkNib()->nwkTxTotal;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkSymLink.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetSymLink(ZBPRO_NWK_NIB_SymLink_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkSymLink = newValue;
    return &zbProNwkNib()->nwkSymLink;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkCapabilityInformation.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetCapabilityInformation(ZBPRO_NWK_NIB_CapabilityInformation_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkCapabilityInformation = newValue;
    return &zbProNwkNib()->nwkCapabilityInformation;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkAddrAlloc.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetAddrAlloc(ZBPRO_NWK_NIB_AddrAlloc_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_ADDR_ALLOC <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ADDR_ALLOC >= newValue,
                  NWKNIBAPI_SETADDR_ALLOC_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkAddrAlloc = newValue;
    return &zbProNwkNib()->nwkAddrAlloc;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkUseTreeRouting.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetUseTreeRouting(ZBPRO_NWK_NIB_UseTreeRouting_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkUseTreeRouting = newValue;
    return &zbProNwkNib()->nwkUseTreeRouting;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkManagerAddr.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetManagerAddr(ZBPRO_NWK_NIB_ManagerAddr_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MANAGER_ADDR <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MANAGER_ADDR >= newValue,
                  NWKNIBAPI_SETMANAGER_ADDR_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkManagerAddr = newValue;
    return &zbProNwkNib()->nwkManagerAddr;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxSourceRoute.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxSourceRoute(ZBPRO_NWK_NIB_MaxSourceRoute_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_SOURCE_ROUTE <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_SOURCE_ROUTE >= newValue,
                  NWKNIBAPI_SETMAX_SOURCE_ROUTE_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxSourceRoute = newValue;
    return &zbProNwkNib()->nwkMaxSourceRoute;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkUpdateId.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetUpdateId(ZBPRO_NWK_NIB_UpdateId_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_UPDATE_ID <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_UPDATE_ID >= newValue,
                  NWKNIBAPI_SETUPDATE_ID_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkUpdateId = newValue;

    /* Reset the nwkTxTotal attribute (and all the transmission failure counters in the Neighbor Table);
     * see ZigBee Document 053474r20, annex E. */
    zbProNwkNibApiSetTxTotal(0x0000);

    /* Update MAC Beacon payload. */
    SYS_DataPointer_t beaconPayload;
    zbProNwkNibApiGet(MAC_BEACON_PAYLOAD, NULL, &beaconPayload);
    if (SYS_CheckPayload(&beaconPayload))
        SYS_CopyToPayload(&beaconPayload, ZBPRO_NWK_BEACON_PAYLOAD_UPDATE_ID_OFFSET, &newValue, sizeof(newValue));

    SYS_RAISE_EVENT(ZBPRO_NWK_HOT_UPDATE);
    return &zbProNwkNib()->nwkUpdateId;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkTransactionPersistenceTime.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetTransactionPersistenceTime(ZBPRO_NWK_NIB_TransactionPersistenceTime_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_TRANSACTION_PERSISTENCE_TIME <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_TRANSACTION_PERSISTENCE_TIME >= newValue,
                  NWKNIBAPI_SETTRANSACTION_PERSISTENCE_TIME_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkTransactionPersistenceTime = newValue;
    {
        MAC_PibAttributeValue_t attr;
        attr.macTransactionPersistenceTime = newValue;
        macPibApiSet(MAC_FOR_ZBPRO_CONTEXT((MAC_PibAttributeId_t)MAC_TRANSACTION_PERSISTENCE_TIME, &attr, NULL));
    }
    return &zbProNwkNib()->nwkTransactionPersistenceTime;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkNetworkAddress.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetNetworkAddress(ZBPRO_NWK_NIB_NetworkAddress_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_NETWORK_ADDRESS <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_NETWORK_ADDRESS >= newValue,
                  NWKNIBAPI_SETNETWORK_ADDRESS_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkNetworkAddress = newValue;
    {
        MAC_PibAttributeValue_t attr;
        attr.macShortAddress = newValue;
        macPibApiSet(MAC_FOR_ZBPRO_CONTEXT((MAC_PibAttributeId_t)MAC_SHORT_ADDRESS, &attr, NULL));
    }
    return &zbProNwkNib()->nwkNetworkAddress;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkStackProfile.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetStackProfile(ZBPRO_NWK_NIB_StackProfile_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_STACK_PROFILE <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_STACK_PROFILE >= newValue,
                  NWKNIBAPI_SETSTACK_PROFILE_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkStackProfile = newValue;
    return &zbProNwkNib()->nwkStackProfile;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkExtendedPanId.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetExtendedPanId(ZBPRO_NWK_NIB_ExtendedPanId_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_EXTENDED_PAN_ID <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_EXTENDED_PAN_ID >= newValue,
                  NWKNIBAPI_SETEXTENDED_PAN_ID_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkExtendedPanId = newValue;
    return &zbProNwkNib()->nwkExtendedPanId;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkUseMulticast.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetUseMulticast(ZBPRO_NWK_NIB_UseMulticast_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkUseMulticast = newValue;
    return &zbProNwkNib()->nwkUseMulticast;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkIsConcentrator.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetIsConcentrator(ZBPRO_NWK_NIB_IsConcentrator_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkIsConcentrator = newValue;
    zbProNwkMtoDiscovery();
    return &zbProNwkNib()->nwkIsConcentrator;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkConcentratorRadius.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetConcentratorRadius(ZBPRO_NWK_NIB_ConcentratorRadius_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_CONCENTRATOR_RADIUS <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_CONCENTRATOR_RADIUS >= newValue,
                  NWKNIBAPI_SETCONCENTRATOR_RADIUS_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkConcentratorRadius = newValue;
    zbProNwkMtoDiscovery();
    return &zbProNwkNib()->nwkConcentratorRadius;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkConcentratorDiscoveryTimeSec.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetConcentratorDiscoveryTimeSec(ZBPRO_NWK_NIB_ConcentratorDiscoveryTimeSec_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_CONCENTRATOR_DISCOVERY_TIME_SEC <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_CONCENTRATOR_DISCOVERY_TIME_SEC >= newValue,
                  NWKNIBAPI_SETCONCENTRATOR_DISCOVERY_TIME_SEC_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkConcentratorDiscoveryTimeSec = newValue;
    zbProNwkMtoDiscovery();
    return &zbProNwkNib()->nwkConcentratorDiscoveryTimeSec;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkSecurityLevel.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetSecurityLevel(ZBPRO_NWK_NIB_SecurityLevel_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_SECURITY_LEVEL <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_SECURITY_LEVEL >= newValue,
                  NWKNIBAPI_SETSECURITY_LEVEL_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkSecurityLevel = newValue;
    return &zbProNwkNib()->nwkSecurityLevel;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkActiveKeySeqNumber.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetActiveKeySeqNumber(ZBPRO_NWK_NIB_ActiveKeySeqNumber_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_ACTIVE_KEY_SEQ_NUMBER <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ACTIVE_KEY_SEQ_NUMBER >= newValue,
                  NWKNIBAPI_SETACTIVE_KEY_SEQ_NUMBER_DA0);

    zbProNwkNibCheckState();
    const ZbProSspKey_t *potentialKey = zbProNwkNibApiGetKey(newValue);
    if (potentialKey)
    {
        zbProNwkNib()->nwkActiveKeySeqNumber = newValue;
        SYS_RAISE_EVENT(ZBPRO_NWK_HOT_UPDATE);
        return &zbProNwkNib()->nwkActiveKeySeqNumber;
    }

    SYS_DbgLogId(ZBPRO_NIB_TRIES_ACTIVATE_INVALID_KEY);
    return NULL;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkAllFresh.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetAllFresh(ZBPRO_NWK_NIB_AllFresh_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkAllFresh = newValue;
    return &zbProNwkNib()->nwkAllFresh;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkSecureAllFrames.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetSecureAllFrames(ZBPRO_NWK_NIB_SecureAllFrames_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkSecureAllFrames = newValue;
    return &zbProNwkNib()->nwkSecureAllFrames;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkLinkStatusPeriodSec.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetLinkStatusPeriodSec(ZBPRO_NWK_NIB_LinkStatusPeriodSec_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_LINK_STATUS_PERIOD_SEC <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_LINK_STATUS_PERIOD_SEC >= newValue,
                  NWKNIBAPI_SETLINK_STATUS_PERIOD_SEC_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkLinkStatusPeriodSec = newValue;
    return &zbProNwkNib()->nwkLinkStatusPeriodSec;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkRouterAgeLimit.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetRouterAgeLimit(ZBPRO_NWK_NIB_RouterAgeLimit_t newValue)
{
    SYS_DbgAssertComplex(ZBPRO_NWK_NIB_ATTR_MINALLOWED_ROUTER_AGE_LIMIT <= newValue &&
                  ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ROUTER_AGE_LIMIT >= newValue,
                  NWKNIBAPI_SETROUTER_AGE_LIMIT_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkRouterAgeLimit = newValue;
    return &zbProNwkNib()->nwkRouterAgeLimit;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkUniqueAddr.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetUniqueAddr(ZBPRO_NWK_NIB_UniqueAddr_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkUniqueAddr = newValue;
    return &zbProNwkNib()->nwkUniqueAddr;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkLeaveRequestAllowed.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetLeaveRequestAllowed(ZBPRO_NWK_NIB_LeaveRequestAllowed_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkLeaveRequestAllowed = newValue;
    return &zbProNwkNib()->nwkLeaveRequestAllowed;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkRouteRequestId.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetRouteRequestId(ZBPRO_NWK_NIB_RouteRequestId_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkRouteRequestId = newValue;
    return &zbProNwkNib()->nwkRouteRequestId;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkMaxLinkRouteCost.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetMaxLinkRouteCost(ZBPRO_NWK_NIB_MaxLinkRouteCost_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMaxLinkRouteCost = newValue;
    return &zbProNwkNib()->nwkMaxLinkRouteCost;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkNetworkChannel.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetNetworkChannel(PHY_Channel_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkNetworkChannel = newValue;
    return &zbProNwkNib()->nwkNetworkChannel;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkDeviceType.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetDeviceType(ZBPRO_NWK_NIB_DeviceType_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkDeviceType = newValue;
    return &zbProNwkNib()->nwkDeviceType;
}

/*************************************************************************************//**
  \brief Sets new value to NWK-NIB attribute nwkDepth.
  \param[in] newValue - New value of the attribute.
*****************************************************************************************/
INLINE void *zbProNwkNibApiSetDepth(ZBPRO_NWK_NIB_MaxDepth_t newValue)
{
    SYS_DbgAssertComplex(zbProNwkNib()->nwkMaxDepth >= newValue,
                  NWKNIBAPI_SETDEPTH_DA0);

    zbProNwkNibCheckState();
    zbProNwkNib()->nwkDepth = newValue;
    return &zbProNwkNib()->nwkDepth;
}

INLINE void *zbProNwkNibApiSetScanAttempts(ZBPRO_NWK_NIB_MaxDepth_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkScanAttempts = newValue;
    return &zbProNwkNib()->nwkScanAttempts;
}

INLINE void *zbProNwkNibApiSetTimeBtwnScans(ZBPRO_NWK_NIB_MaxDepth_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkTimeBtwnScans = newValue;
    return &zbProNwkNib()->nwkTimeBtwnScans;
}

INLINE void *zbProNwkNibApiSetMirroredIeeeAddr(ZBPRO_NWK_ExtAddr_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkMirroredIeeeAddr = newValue;
    return &zbProNwkNib()->nwkMirroredIeeeAddr;
}

INLINE void *zbProNwkNibApiSetAuthorizedFlag(Bool8_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkAuthorizedFlag = newValue;
    return &zbProNwkNib()->nwkAuthorizedFlag;
}

INLINE void *zbProNwkNibApiSetConcentratorNoRouteCache(Bool8_t newValue)
{
    zbProNwkNibCheckState();
    zbProNwkNib()->nwkConcentratorNoRouteCache = newValue;
    return &zbProNwkNib()->nwkConcentratorNoRouteCache;
}

/*************************************************************************************//**
  \brief Resets NWK-NIB attributes to their default values.
*****************************************************************************************/
INLINE void zbProNwkNibApiReset(void)
{
    *zbProNwkNib() = (const ZbProNwkNib_t)ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUES;

    ZBPRO_NWK_GET_ATTR(MAC_PAN_ID, &zbProNwkNib()->nwkPanId);
    ZBPRO_NWK_GET_ATTR(MAC_TRANSACTION_PERSISTENCE_TIME, &zbProNwkNib()->nwkTransactionPersistenceTime);
    ZBPRO_NWK_GET_ATTR(MAC_SHORT_ADDRESS, &zbProNwkNib()->nwkNetworkAddress);
    ZBPRO_NWK_GET_ATTR(PHY_CURRENT_CHANNEL, &zbProNwkNib()->nwkNetworkChannel);
    ZBPRO_NWK_GET_ATTR(PHY_CURRENT_PAGE, &zbProNwkNib()->nwkNetworkPage);
}

#pragma Onwarn(428)
#endif /* _ZBPRO_NWK_NIB_API_H */
