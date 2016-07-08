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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkNibPrivate.h $
*
* DESCRIPTION:
*   NWK-NIB private definitions.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _ZBPRO_NWK_NIB_PRIVATE_H
#define _ZBPRO_NWK_NIB_PRIVATE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProNwkSapTypesIb.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief NWK-NIB attributes storage type.
 */
typedef struct _ZbProNwkNib_t
{
    /* 64-bit data. */
    ZBPRO_NWK_NIB_ExtendedPanId_t                 nwkExtendedPanId;                 /*!< nwkExtendedPanId value. */
    ZBPRO_NWK_NIB_ExtAddr_t                       nwkMirroredIeeeAddr;

    /* 32-bit data. */
    ZBPRO_NWK_NIB_PassiveAckTimeoutOct_t             nwkPassiveAckTimeoutOct;             /*!< nwkPassiveAckTimeoutOct value. */
    ZBPRO_NWK_NIB_NetworkBroadcastDeliveryTimeOct_t  nwkNetworkBroadcastDeliveryTimeOct;  /*!< nwkNetworkBroadcastDeliveryTimeOct value. */

    /* 16-bit data. */
    ZBPRO_NWK_NIB_PanId_t                         nwkPanId;                         /*!< nwkPanId value. */
    ZBPRO_NWK_NIB_TxTotal_t                       nwkTxTotal;                       /*!< nwkTxTotal value. */
    ZBPRO_NWK_NIB_ManagerAddr_t                   nwkManagerAddr;                   /*!< nwkManagerAddr value. */
    ZBPRO_NWK_NIB_TransactionPersistenceTime_t    nwkTransactionPersistenceTime;    /*!< nwkTransactionPersistenceTime value. */
    ZBPRO_NWK_NIB_NetworkAddress_t                nwkNetworkAddress;                /*!< nwkNetworkAddress value. */
    ZBPRO_NWK_NIB_TimeBtwnScans_t                 nwkTimeBtwnScans;

    /* 8-bit data. */
    ZBPRO_NWK_NIB_SequenceNumber_t                nwkSequenceNumber;                /*!< nwkSequenceNumber value. */
    ZBPRO_NWK_NIB_MaxBroadcastRetries_t           nwkMaxBroadcastRetries;           /*!< nwkMaxBroadcastRetries value. */
    ZBPRO_NWK_NIB_MaxChildren_t                   nwkMaxChildren;                   /*!< nwkMaxChildren value. */
    ZBPRO_NWK_NIB_MaxDepth_t                      nwkMaxDepth;                      /*!< nwkMaxDepth value. */
    ZBPRO_NWK_NIB_MaxRouters_t                    nwkMaxRouters;                    /*!< nwkMaxRouters value. */
    ZBPRO_NWK_NIB_ReportConstantCost_t            nwkReportConstantCost;            /*!< nwkReportConstantCost value. */
    ZBPRO_NWK_NIB_AddrAlloc_t                     nwkAddrAlloc;                     /*!< nwkAddrAlloc value. */
    ZBPRO_NWK_NIB_MaxSourceRoute_t                nwkMaxSourceRoute;                /*!< nwkMaxSourceRoute value. */
    ZBPRO_NWK_NIB_UpdateId_t                      nwkUpdateId;                      /*!< nwkUpdateId value. */
    ZBPRO_NWK_NIB_StackProfile_t                  nwkStackProfile;                  /*!< nwkStackProfile value. */
    ZBPRO_NWK_NIB_ConcentratorRadius_t            nwkConcentratorRadius;            /*!< nwkConcentratorRadius value. */
    ZBPRO_NWK_NIB_ConcentratorDiscoveryTimeSec_t  nwkConcentratorDiscoveryTimeSec;  /*!< nwkConcentratorDiscoveryTimeSec value. */
    ZBPRO_NWK_NIB_SecurityLevel_t                 nwkSecurityLevel;                 /*!< nwkSecurityLevel value. */
    ZBPRO_NWK_NIB_ActiveKeySeqNumber_t            nwkActiveKeySeqNumber;            /*!< nwkActiveKeySeqNumber value. */
    ZBPRO_NWK_NIB_LinkStatusPeriodSec_t           nwkLinkStatusPeriodSec;           /*!< nwkLinkStatusPeriodSec value. */
    ZBPRO_NWK_NIB_RouterAgeLimit_t                nwkRouterAgeLimit;                /*!< nwkRouterAgeLimit value. */
    ZBPRO_NWK_NIB_CapabilityInformation_t         nwkCapabilityInformation;         /*!< nwkCapabilityInformation value. */

    /* 8-bit data (extended attributes). */
    ZBPRO_NWK_NIB_RouteRequestId_t                nwkRouteRequestId;                /*!< nwkRouteRequestId value. */
    ZBPRO_NWK_NIB_MaxLinkRouteCost_t              nwkMaxLinkRouteCost;              /*!< nwkMaxLinkRouteCost value. */
    PHY_Channel_t                                 nwkNetworkChannel;                /*!< nwkNetworkChannel value. */
    PHY_Page_t                                    nwkNetworkPage;                   /*!< nwkNetworkPage value. */
    ZBPRO_NWK_NIB_DeviceType_t                    nwkDeviceType;                    /*!< nwkDeviceType value. */
    ZBPRO_NWK_NIB_MaxDepth_t                      nwkDepth;
    ZBPRO_NWK_NIB_ScanAttempts_t                  nwkScanAttempts;

    /* 1-bit data. */
    ZBPRO_NWK_NIB_TimeStamp_t                     nwkTimeStamp;                     /*!< nwkTimeStamp value. */
    ZBPRO_NWK_NIB_SymLink_t                       nwkSymLink;                       /*!< nwkSymLink value. */
    ZBPRO_NWK_NIB_UseTreeRouting_t                nwkUseTreeRouting;                /*!< nwkUseTreeRouting value. */
    ZBPRO_NWK_NIB_UseMulticast_t                  nwkUseMulticast;                  /*!< nwkUseMulticast value. */
    ZBPRO_NWK_NIB_IsConcentrator_t                nwkIsConcentrator;                /*!< nwkIsConcentrator value. */
    ZBPRO_NWK_NIB_AllFresh_t                      nwkAllFresh;                      /*!< nwkAllFresh value. */
    ZBPRO_NWK_NIB_SecureAllFrames_t               nwkSecureAllFrames;               /*!< nwkSecureAllFrames value. */
    ZBPRO_NWK_NIB_UniqueAddr_t                    nwkUniqueAddr;                    /*!< nwkUniqueAddr value. */
    ZBPRO_NWK_NIB_LeaveRequestAllowed_t           nwkLeaveRequestAllowed;           /*!< nwkLeaveRequestAllowed value. */
    Bool8_t                                       nwkAuthorizedFlag;
    Bool8_t                                       nwkConcentratorNoRouteCache;

    uint8_t                                       nwkReservedAlignment0;
} ZbProNwkNib_t;

/**//**
 * \brief NWK-NIB attributes default values.
 */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUES                                                                               \
        {                                                                                                               \
            .nwkPanId                           = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PAN_ID,                              \
            .nwkSequenceNumber                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SEQUENCE_NUMBER,                     \
            .nwkMaxBroadcastRetries             = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_BROADCAST_RETRIES,               \
            .nwkMaxChildren                     = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_CHILDREN,                        \
            .nwkMaxDepth                        = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_DEPTH,                           \
            .nwkMaxRouters                      = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_ROUTERS,                         \
            .nwkReportConstantCost              = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_REPORT_CONSTANT_COST,                \
            .nwkTimeStamp                       = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TIME_STAMP,                          \
            .nwkTxTotal                         = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TX_TOTAL,                            \
            .nwkSymLink                         = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SYM_LINK,                            \
            .nwkCapabilityInformation           = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CAPABILITY_INFORMATION,              \
            .nwkAddrAlloc                       = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ADDR_ALLOC,                          \
            .nwkUseTreeRouting                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_USE_TREE_ROUTING,                    \
            .nwkManagerAddr                     = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MANAGER_ADDR,                        \
            .nwkMaxSourceRoute                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_SOURCE_ROUTE,                    \
            .nwkUpdateId                        = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_UPDATE_ID,                           \
            .nwkStackProfile                    = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_STACK_PROFILE,                       \
            .nwkExtendedPanId                   = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_EXTENDED_PAN_ID,                     \
            .nwkUseMulticast                    = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_USE_MULTICAST,                       \
            .nwkIsConcentrator                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_IS_CONCENTRATOR,                     \
            .nwkConcentratorRadius              = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_RADIUS,                 \
            .nwkConcentratorDiscoveryTimeSec    = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_DISCOVERY_TIME_SEC,     \
            .nwkSecurityLevel                   = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SECURITY_LEVEL,                      \
            .nwkActiveKeySeqNumber              = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ACTIVE_KEY_SEQ_NUMBER,               \
            .nwkAllFresh                        = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ALL_FRESH,                           \
            .nwkSecureAllFrames                 = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SECURE_ALL_FRAMES,                   \
            .nwkLinkStatusPeriodSec             = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_LINK_STATUS_PERIOD_SEC,              \
            .nwkRouterAgeLimit                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ROUTER_AGE_LIMIT,                    \
            .nwkUniqueAddr                      = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_UNIQUE_ADDR,                         \
            .nwkLeaveRequestAllowed             = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_LEAVE_REQUEST_ALLOWED,               \
            .nwkPassiveAckTimeoutOct            = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PASSIVE_ACK_TIMEOUT,                 \
            .nwkNetworkBroadcastDeliveryTimeOct = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_BROADCAST_DELIVERY_TIME,             \
            .nwkRouteRequestId                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ROUTE_REQUEST_ID,                    \
            .nwkMaxLinkRouteCost                = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_LINK_ROUTE_COST,                 \
            .nwkDeviceType                      = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_DEVICE_TYPE,                         \
            .nwkScanAttempts                    = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SCAN_ATTEMPTS,                       \
            .nwkTimeBtwnScans                   = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TIME_BTWN_SCANS,                     \
            .nwkMirroredIeeeAddr                = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MIRRORED_IEEE_ADDR,                  \
            .nwkAuthorizedFlag                  = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_AUTHORIZED_FLAG,                     \
            .nwkConcentratorNoRouteCache        = ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_NO_ROUTE_CACHE          \
        }
#endif /* _ZBPRO_NWK_NIB_PRIVATE_H */
