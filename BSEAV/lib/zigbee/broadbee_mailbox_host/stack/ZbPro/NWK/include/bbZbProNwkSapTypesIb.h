/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkSapTypesIb.h $
*
* DESCRIPTION:
*   NWK-NIB Attributes definition.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
*****************************************************************************************/


#ifndef _ZBPRO_NWK_SAP_TYPES_IB_H
#define _ZBPRO_NWK_SAP_TYPES_IB_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"
#include "bbZbProNwkCommon.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief NWK-NIB attributes identifiers enumeration (see ZigBee spec r20, Table 3.44).
 */
typedef enum _ZbProNwkNibAttributeId_t
{
    ZBPRO_NWK_NIB_PAN_ID                              = 0x80,    /*!<  This NIB attribute should, at all times, have the same
                                                             value as MAC_PAN_ID. */
    ZBPRO_NWK_NIB_SEQUENCE_NUMBER                     = 0x81,    /*!<  A sequence number used to identify outgoing frames
                                                             (see ZigBee spec r20, sub-clause 3.6.2). */
    ZBPRO_NWK_NIB_PASSIVE_ACK_TIMEOUT_OCT             = 0x82,    /*!<  The maximum time duration in OctetDurations allowed for
                                                             the parent and all child devices to retransmit a broadcast
                                                             message (passive acknowledgment timeout). */
    ZBPRO_NWK_NIB_MAX_BROADCAST_RETRIES               = 0x83,    /*!<  The maximum number of retries allowed after a broadcast
                                                             transmission failure. */
    ZBPRO_NWK_NIB_MAX_CHILDREN                        = 0x84,    /*!<  The number of children a device is allowed to have on its
                                                             current network Note that when ZBPRO_NWK_NIB_ADDR_ALLOC has a value
                                                             of 0x02, indicating stochastic addressing, the value of
                                                             this attribute is implementation dependent. */
    ZBPRO_NWK_NIB_MAX_DEPTH                           = 0x85,    /*!<  The depth a device can have. */
    ZBPRO_NWK_NIB_MAX_ROUTERS                         = 0x86,    /*!<  The number of routers any one device is allowed to have
                                                             as children. This value is determined by the ZigBee
                                                             coordinator for all devices in the network.
                                                             If ZBPRO_NWK_NIB_ADDR_ALLOC is 0x02 this value not used. */
    ZBPRO_NWK_NIB_NEIGHBOR_TABLE                      = 0x87,    /*!<  The current set of neighbor table entries in the device
                                                             (see ZigBee spec r20, Table 3.48). */
    ZBPRO_NWK_NIB_NETWORK_BROADCAST_DELIVERY_TIME_OCT = 0x88,    /*!<  Time duration in OctetDurations that a broadcast message
                                                             needs to encompass the entire network. This is a calculated
                                                             quantity based on other NIB attributes.
                                                             The formula is given in subclause 3.5.2.1. */
    ZBPRO_NWK_NIB_REPORT_CONSTANT_COST                = 0x89,    /*!<  If this is set to 0, the NWK layer shall calculate link
                                                             cost from all neighbor nodes using the LQI values reported
                                                             by the MAC layer;
                                                             otherwise, it shall report a constant value. */
    ZBPRO_NWK_NIB_RESERVED_0X8A                       = 0x8A,
    ZBPRO_NWK_NIB_ROUTE_TABLE                         = 0x8B,    /*!<  The current set of routing table entries in the device
                                                             (see ZigBee spec r20, Table 3.51). */
    ZBPRO_NWK_NIB_TIME_STAMP                          = 0x8C,    /*!<  A flag that determines if a time stamp indication
                                                             is provided on incoming and outgoing packets.
                                                             TRUE= time indication provided.
                                                             FALSE = no time indication provided. */
    ZBPRO_NWK_NIB_TX_TOTAL                            = 0x8D,    /*!<  A count of unicast transmissions made by the NWK layer on
                                                             this device. Each time the NWK layer transmits a unicast
                                                             frame, by invoking the MCPSDATA. request primitive of the
                                                             MAC sub-layer, it shall increment this counter. When either
                                                             the NHL performs an NLMESET.request on this attribute or if
                                                             the value of ZBPRO_NWK_NIB_TX_TOTAL rolls over past 0xffff the
                                                             NWK layer shall reset to 0x00 each Transmit Failure field
                                                             contained in the neighbor table. */
    ZBPRO_NWK_NIB_SYM_LINK                            = 0x8E,    /*!<  The current route symmetry setting: TRUE means that routes
                                                             are considered to be comprised of symmetric links. Backward
                                                             and forward routes are created during one-route discovery
                                                             and they are identical. FALSE indicates that routes are not
                                                             consider to be comprised of symmetric links.
                                                             Only the forward route is stored during route discovery. */
    ZBPRO_NWK_NIB_CAPABILITY_INFORMATION              = 0x8F,    /*!<  This field shall contain the device capability information
                                                             established at network joining time
                                                             (see ZigBee spec r20, Table 3.47). */
    ZBPRO_NWK_NIB_ADDR_ALLOC                          = 0x90,    /*!<  A value that determines the method used to assign addresses:
                                                             0x00 = use distributed address allocation
                                                             0x01 = reserved
                                                             0x02 = use stochastic address allocation. */
    ZBPRO_NWK_NIB_USE_TREE_ROUTING                    = 0x91,    /*!<  A flag that determines whether the NWK layer should assume
                                                             the ability to use hierarchical routing:
                                                             TRUE = assume the ability to use hierarchical routing.
                                                             FALSE = never use hierarchical routing. */
    ZBPRO_NWK_NIB_MANAGER_ADDR                        = 0x92,    /*!<  The address of the designated network channel manager
                                                             function. */
    ZBPRO_NWK_NIB_MAX_SOURCE_ROUTE                    = 0x93,    /*!<  The maximum number of hops in a source route. */
    ZBPRO_NWK_NIB_UPDATE_ID                           = 0x94,    /*!<  The value identifying a snapshot of the network settings
                                                             with which this node is operating with. */
    ZBPRO_NWK_NIB_TRANSACTION_PERSISTENCE_TIME        = 0x95,    /*!<  The maximum time (in superframe periods) that a transaction
                                                             is stored by a coordinator and indicated in its beacon.
                                                             This attribute reflects the value of the MAC PIB attribute
                                                             MAC_TRANSACTION_PERSISTENCE_TIME and any changes made by
                                                             the higher layer will be reflected in the MAC PIB
                                                             attribute value as well. */
    ZBPRO_NWK_NIB_NETWORK_ADDRESS                     = 0x96,    /*!<  The 16-bit address that the device uses to communicate with
                                                             the PAN. This attribute reflects the value of the MAC PIB
                                                             attribute MAC_SHORT_ADDRESS and any changes made by
                                                             the higher layer will be reflected in the MAC PIB attribute
                                                             value as well. */
    ZBPRO_NWK_NIB_STACK_PROFILE                       = 0x97,    /*!<  The identifier of the ZigBee stack profile in use for
                                                             this device. */
    ZBPRO_NWK_NIB_BROADCAST_TRANSACTION_TABLE         = 0x98,    /*!<  The current set of broadcast transaction table entries in
                                                             the device (see ZigBee spec r20, Table 3.55). */
    ZBPRO_NWK_NIB_GROUP_ID_TABLE                      = 0x99,    /*!<  The set of group identifiers, in the range 0x0000 - 0xffff,
                                                             for groups of which this device is a member. */
    ZBPRO_NWK_NIB_EXTENDED_PAN_ID                     = 0x9A,    /*!<  The Extended PAN Identifier for the PAN of which the device
                                                             is a member. The value 0x0000000000000000 means
                                                             the Extended PAN Identifier is unknown. */
    ZBPRO_NWK_NIB_USE_MULTICAST                       = 0x9B,    /*!<  Flag determining the layer where multicast messaging occurs
                                                             TRUE = multicast occurs at the network layer.
                                                             FALSE= multicast occurs at the APS layer and using
                                                             the APS header. */
    ZBPRO_NWK_NIB_ROUTE_RECORD_TABLE                  = 0x9C,    /*!<  The route record table
                                                             (see ZigBee spec r20, Table 3.45). */
    ZBPRO_NWK_NIB_IS_CONCENTRATOR                     = 0x9D,    /*!<  A flag determining if this device is a concentrator.
                                                             TRUE = Device is a concentrator.
                                                             FALSE = Device is not a concentrator. */
    ZBPRO_NWK_NIB_CONCENTRATOR_RADIUS                 = 0x9E,    /*!<  The hop count radius for concentrator route discoveries. */
    ZBPRO_NWK_NIB_CONCENTRATOR_DISCOVERY_TIME_SEC     = 0x9F,    /*!<  The time in seconds between concentrator route discoveries.
                                                             If set to 0x0000, the discoveries are done at start up and
                                                             by the next higher layer only. */
    ZBPRO_NWK_NIB_SECURITY_LEVEL                      = 0xA0,    /*!<  Security. The security level for outgoing and incoming
                                                             NWK frames; the allowable security level identifiers
                                                             are presented in Table 4.40. */
    ZBPRO_NWK_NIB_SECURITY_MATERIAL_SET               = 0xA1,    /*!<  Security. Set of network security material descriptors
                                                             capable of maintaining an active and alternate
                                                             network key. */
    ZBPRO_NWK_NIB_ACTIVE_KEY_SEQ_NUMBER               = 0xA2,    /*!<  Security. The sequence number of the active network key
                                                             in nwkSecurityMaterialSet. */
    ZBPRO_NWK_NIB_ALL_FRESH                           = 0xA3,    /*!<  Security. Indicates whether incoming NWK frames must be all
                                                             checked for freshness when the memory for incoming frame
                                                             counts is exceeded. */
    ZBPRO_NWK_NIB_RESERVED_0XA4                       = 0xA4,
    ZBPRO_NWK_NIB_SECURE_ALL_FRAMES                   = 0xA5,    /*!<  Security. Indicates whether security shall be applied to
                                                             incoming and outgoing NWK data frames.
                                                             If set to 0x01, security processing shall be applied to all
                                                             incoming and outgoing frames except data frames destined
                                                             for the current device that have the security sub-field of
                                                             the frame control field set to 0. If this attribute has a
                                                             value of 0x01, the NWK layer shall not relay frames that
                                                             have the security sub-field of the frame control field set
                                                             to 0. The SecurityEnable parameter of the NLDEDATA.request
                                                             primitive shall override the setting of this attribute. */
    ZBPRO_NWK_NIB_LINK_STATUS_PERIOD_SEC              = 0xA6,    /*!<  The time in seconds between link status command frames. */
    ZBPRO_NWK_NIB_ROUTER_AGE_LIMIT                    = 0xA7,    /*!<  The number of missed link status command frames before
                                                             resetting the link costs to zero. */
    ZBPRO_NWK_NIB_UNIQUE_ADDR                         = 0xA8,    /*!<  A flag that determines whether the NWK layer should detect
                                                             and correct conflicting addresses:
                                                             TRUE = assume addresses are unique.
                                                             FALSE = addresses may not be unique. */
    ZBPRO_NWK_NIB_ADDRESS_MAP                         = 0xA9,    /*!<  The current set of 64-bit IEEE to 16-bit network address
                                                             map (see ZigBee spec r20, Table 3.46). */
    ZBPRO_NWK_NIB_LEAVE_REQUEST_ALLOWED               = 0xAA,    /*!<  This policy determines whether or not a remote NWK leave
                                                             request command frame received by the local device
                                                             is accepted. */

    /* Extended attributes which are not described in ZigBee spec r20, Table 3.44. */
    ZBPRO_NWK_NIB_ROUTE_REQUEST_ID                    = 0xB0,    /*!<  A sequence number for a route request command frame that is
                                                             incremented each time a device initiates route request. */
    ZBPRO_NWK_NIB_MAX_LINK_ROUTE_COST                 = 0xB1,    /*!<  Select this route to the hop only when
                                                             the cost is less than given threshold. */
    ZBPRO_NWK_NIB_NETWORK_CHANNEL                     = 0xB3,    /*!<  Network Channel. Reflected PHY_CURRENT_CHANNEL. */
    ZBPRO_NWK_NIB_DEVICE_TYPE                         = 0xB4,    /*!<  Device Type. */

    ZBPRO_NWK_NIB_SCAN_ATTEMPTS                       = 0xB5,
    ZBPRO_NWK_NIB_TIME_BTWN_SCANS                     = 0xB6,
    ZBPRO_NWK_NIB_AUTHORIZED_FLAG                     = 0xB7,
    ZBPRO_NWK_NIB_CONCENTRATOR_NO_ROUTE_CACHE         = 0xB8,

    ZBPRO_NWK_NIB_FRAME_COUNTER         = 0xB9,
    /* Water mark. */
    ZBPRO_NWK_NIB_ATTRIBUTES_ID_END

} ZbProNwkNibAttributeId_t;

/**//**
 * \brief First ID of NWK-NIB attributes.
 */
#define ZBPRO_NWK_NIB_ATTRIBUTES_ID_BEGIN             (ZBPRO_NWK_NIB_PAN_ID)

#define ZBPRO_NWK_NIB_RESERVED_ID_START               (ZBPRO_NWK_NIB_LEAVE_REQUEST_ALLOWED)
#define ZBPRO_NWK_NIB_RESERVED_ID_END                 (ZBPRO_NWK_NIB_ROUTE_REQUEST_ID)
/**//**
 * \brief Check is the value in the valid NWK-NIB attributes range.
 */
#define ZBPRO_NWK_NIB_IS_ATTRIBUTE_VALID(attrId)                    \
    ((attrId >= ZBPRO_NWK_NIB_ATTRIBUTES_ID_BEGIN)                  \
     && (attrId < ZBPRO_NWK_NIB_ATTRIBUTES_ID_END)                  \
     && (attrId != ZBPRO_NWK_NIB_RESERVED_0X8A)                     \
     && (attrId != ZBPRO_NWK_NIB_RESERVED_0XA4)                     \
     && (attrId != ZBPRO_NWK_NIB_NEIGHBOR_TABLE)                    \
     && (attrId != ZBPRO_NWK_NIB_ROUTE_TABLE)                       \
     && (attrId != ZBPRO_NWK_NIB_BROADCAST_TRANSACTION_TABLE)       \
     && (attrId != ZBPRO_NWK_NIB_GROUP_ID_TABLE)                    \
     && (attrId != ZBPRO_NWK_NIB_ROUTE_RECORD_TABLE)                \
     && (attrId != ZBPRO_NWK_NIB_SECURITY_MATERIAL_SET)             \
     && (attrId != ZBPRO_NWK_NIB_ADDRESS_MAP)                       \
     && ((attrId <= ZBPRO_NWK_NIB_RESERVED_ID_START)                \
        || (attrId >= ZBPRO_NWK_NIB_RESERVED_ID_END)))

/**//**
 * \brief NWK-NIB public attributes identifiers enumeration.
 */
typedef ZbProNwkNibAttributeId_t  ZBPRO_NWK_NibAttributeId_t;

/**//**
 * \brief NWK-NIB attributes data types.
 */
typedef ZBPRO_NWK_PanId_t   ZBPRO_NWK_NIB_PanId_t;                             /*!<  Data type for nwkPanId. */
typedef uint8_t             ZBPRO_NWK_NIB_SequenceNumber_t;                    /*!<  Data type for nwkSequenceNumber. */
typedef uint32_t            ZBPRO_NWK_NIB_PassiveAckTimeoutOct_t;              /*!<  Data type for nwkPassiveAckTimeoutOct. */
typedef uint8_t             ZBPRO_NWK_NIB_MaxBroadcastRetries_t;               /*!<  Data type for nwkMaxBroadcastRetries. */
typedef uint8_t             ZBPRO_NWK_NIB_MaxChildren_t;                       /*!<  Data type for nwkMaxChildren. */
typedef ZBPRO_NWK_Depth_t   ZBPRO_NWK_NIB_MaxDepth_t;                          /*!<  Data type for nwkMaxDepth. */
typedef uint8_t             ZBPRO_NWK_NIB_MaxRouters_t;                        /*!<  Data type for nwkMaxRouters. */
typedef uint32_t            ZBPRO_NWK_NIB_NetworkBroadcastDeliveryTimeOct_t;   /*!<  Data type for nwkNetworkBroadcastDeliveryTimeOct. */
typedef Bool8_t             ZBPRO_NWK_NIB_ReportConstantCost_t;                /*!<  Data type for nwkReportConstantCost. */
typedef Bool8_t             ZBPRO_NWK_NIB_TimeStamp_t;                         /*!<  Data type for nwkTimeStamp. */
typedef uint16_t            ZBPRO_NWK_NIB_TxTotal_t;                           /*!<  Data type for nwkTxTotal. */
typedef Bool8_t             ZBPRO_NWK_NIB_SymLink_t;                           /*!<  Data type for nwkSymLink. */
typedef ZBPRO_NWK_Capability_t  ZBPRO_NWK_NIB_CapabilityInformation_t;         /*!<  Data type for nwkCapabilityInformation. */
typedef uint8_t             ZBPRO_NWK_NIB_AddrAlloc_t;                         /*!<  Data type for nwkAddrAlloc. */
typedef Bool8_t             ZBPRO_NWK_NIB_UseTreeRouting_t;                    /*!<  Data type for nwkUseTreeRouting. */
typedef uint16_t            ZBPRO_NWK_NIB_ManagerAddr_t;                       /*!<  Data type for nwkManagerAddr. */
typedef uint8_t             ZBPRO_NWK_NIB_MaxSourceRoute_t;                    /*!<  Data type for nwkMaxSourceRoute. */
typedef uint8_t             ZBPRO_NWK_NIB_UpdateId_t;                          /*!<  Data type for nwkUpdateId. */
typedef MAC_TransactionPersistenceTime_t ZBPRO_NWK_NIB_TransactionPersistenceTime_t; /*!<  Data type for nwkTransactionPersistenceTime. */
typedef ZBPRO_NWK_NwkAddr_t ZBPRO_NWK_NIB_NetworkAddress_t;                    /*!<  Data type for nwkNetworkAddress. */
typedef uint8_t             ZBPRO_NWK_NIB_StackProfile_t;                      /*!<  Data type for nwkStackProfile. */
typedef ZBPRO_NWK_ExtPanId_t ZBPRO_NWK_NIB_ExtendedPanId_t;                    /*!<  Data type for nwkExtendedPanId. */
typedef Bool8_t             ZBPRO_NWK_NIB_UseMulticast_t;                      /*!<  Data type for nwkUseMulticast. */
typedef Bool8_t             ZBPRO_NWK_NIB_IsConcentrator_t;                    /*!<  Data type for nwkIsConcentrator. */
typedef uint8_t             ZBPRO_NWK_NIB_ConcentratorRadius_t;                /*!<  Data type for nwkConcentratorRadius. */
typedef uint8_t             ZBPRO_NWK_NIB_ConcentratorDiscoveryTimeSec_t;      /*!<  Data type for nwkConcentratorDiscoveryTimeSec. */
typedef uint8_t             ZBPRO_NWK_NIB_SecurityLevel_t;                     /*!<  Data type for nwkSecurityLevel. */
typedef ZbProSspNwkKeySeqNum_t ZBPRO_NWK_NIB_ActiveKeySeqNumber_t;             /*!<  Data type for nwkActiveKeySeqNumber. */
typedef Bool8_t             ZBPRO_NWK_NIB_AllFresh_t;                          /*!<  Data type for nwkAllFresh. */
typedef Bool8_t             ZBPRO_NWK_NIB_SecureAllFrames_t;                   /*!<  Data type for nwkSecureAllFrames. */
typedef uint8_t             ZBPRO_NWK_NIB_LinkStatusPeriodSec_t;               /*!<  Data type for nwkLinkStatusPeriodSec. */
typedef uint8_t             ZBPRO_NWK_NIB_RouterAgeLimit_t;                    /*!<  Data type for nwkRouterAgeLimit. */
typedef Bool8_t             ZBPRO_NWK_NIB_UniqueAddr_t;                        /*!<  Data type for nwkUniqueAddr. */
typedef Bool8_t             ZBPRO_NWK_NIB_LeaveRequestAllowed_t;               /*!<  Data type for nwkLeaveRequestAllowed. */

/**//**
 * \brief NWK-NIB attributes data types which are not implemented.
 */
typedef uint8_t       *ZBPRO_NWK_NIB_NeighborTable_t;                     /*!<  Data type for nwkNeighborTable. */
typedef uint8_t       *ZBPRO_NWK_NIB_RouteTable_t;                        /*!<  Data type for nwkRouteTable. */
typedef uint8_t       *ZBPRO_NWK_NIB_BroadcastTransactionTable_t;         /*!<  Data type for nwkBroadcastTransactionTable. */
typedef uint8_t       *ZBPRO_NWK_NIB_GroupIdTable_t;                      /*!<  Data type for nwkGroupIdTable. */
typedef uint8_t       *ZBPRO_NWK_NIB_RouteRecordTable_t;                  /*!<  Data type for nwkRouteRecordTable. */
typedef uint8_t       *ZBPRO_NWK_NIB_SecurityMaterialSet_t;               /*!<  Data type for nwkSecurityMaterialSet. */
typedef uint8_t       *ZBPRO_NWK_NIB_AddressMap_t;                        /*!<  Data type for nwkAddressMap. */

/**//**
 * \brief Extended NWK-NIB attributes data types.
 */
typedef uint8_t        ZBPRO_NWK_NIB_RouteRequestId_t;                    /*!<  Data type for nwkRouteRequestId. */
typedef uint8_t        ZBPRO_NWK_NIB_MaxLinkRouteCost_t;                  /*!<  Data type for nwkMaxLinkRouteCost. */
typedef PHY_CurrentChannel_t ZBPRO_NWK_NIB_NetworkChannel_t;              /*!<  Data type for nwkNetworkChannel. */
typedef PHY_CurrentPage_t ZBPRO_NWK_NIB_NetworkPage_t;                    /*!<  Data type for nwkNetworkPage. */
typedef ZBPRO_NWK_DeviceType_t ZBPRO_NWK_NIB_DeviceType_t;                /*!<  Data type for nwkDeviceType. */
typedef uint8_t        ZBPRO_NWK_NIB_ScanAttempts_t;                      /* required by ZDO */
typedef uint16_t       ZBPRO_NWK_NIB_TimeBtwnScans_t;                     /* required by ZDO */
typedef ZBPRO_NWK_ExtAddr_t ZBPRO_NWK_NIB_ExtAddr_t;                      /* required by MAC */
/**//**
 * \brief NWK-NIB private attributes data types list macro.
 */
#define ZBPRO_NWK_NIB_PRIVATE_VARIANT                                                           \
    union                                                                                       \
    {                                                                                           \
        ZBPRO_NWK_NIB_PanId_t                             nwkPanId;                             \
        ZBPRO_NWK_NIB_SequenceNumber_t                    nwkSequenceNumber;                    \
        ZBPRO_NWK_NIB_PassiveAckTimeoutOct_t              nwkPassiveAckTimeoutOct;              \
        ZBPRO_NWK_NIB_MaxBroadcastRetries_t               nwkMaxBroadcastRetries;               \
        ZBPRO_NWK_NIB_MaxChildren_t                       nwkMaxChildren;                       \
        ZBPRO_NWK_NIB_MaxDepth_t                          nwkMaxDepth;                          \
        ZBPRO_NWK_NIB_MaxRouters_t                        nwkMaxRouters;                        \
        ZBPRO_NWK_NIB_NetworkBroadcastDeliveryTimeOct_t   nwkNetworkBroadcastDeliveryTimeOct;   \
        ZBPRO_NWK_NIB_ReportConstantCost_t                nwkReportConstantCost;                \
        ZBPRO_NWK_NIB_TimeStamp_t                         nwkTimeStamp;                         \
        ZBPRO_NWK_NIB_TxTotal_t                           nwkTxTotal;                           \
        ZBPRO_NWK_NIB_SymLink_t                           nwkSymLink;                           \
        ZBPRO_NWK_NIB_CapabilityInformation_t             nwkCapabilityInformation;             \
        ZBPRO_NWK_NIB_AddrAlloc_t                         nwkAddrAlloc;                         \
        ZBPRO_NWK_NIB_UseTreeRouting_t                    nwkUseTreeRouting;                    \
        ZBPRO_NWK_NIB_ManagerAddr_t                       nwkManagerAddr;                       \
        ZBPRO_NWK_NIB_MaxSourceRoute_t                    nwkMaxSourceRoute;                    \
        ZBPRO_NWK_NIB_UpdateId_t                          nwkUpdateId;                          \
        ZBPRO_NWK_NIB_TransactionPersistenceTime_t        nwkTransactionPersistenceTime;        \
        ZBPRO_NWK_NIB_NetworkAddress_t                    nwkNetworkAddress;                    \
        ZBPRO_NWK_NIB_StackProfile_t                      nwkStackProfile;                      \
        ZBPRO_NWK_NIB_ExtendedPanId_t                     nwkExtendedPanId;                     \
        ZBPRO_NWK_NIB_UseMulticast_t                      nwkUseMulticast;                      \
        ZBPRO_NWK_NIB_IsConcentrator_t                    nwkIsConcentrator;                    \
        ZBPRO_NWK_NIB_ConcentratorRadius_t                nwkConcentratorRadius;                \
        ZBPRO_NWK_NIB_ConcentratorDiscoveryTimeSec_t      nwkConcentratorDiscoveryTimeSec;      \
        ZBPRO_NWK_NIB_SecurityLevel_t                     nwkSecurityLevel;                     \
        ZBPRO_NWK_NIB_ActiveKeySeqNumber_t                nwkActiveKeySeqNumber;                \
        ZBPRO_NWK_NIB_AllFresh_t                          nwkAllFresh;                          \
        ZBPRO_NWK_NIB_SecureAllFrames_t                   nwkSecureAllFrames;                   \
        ZBPRO_NWK_NIB_LinkStatusPeriodSec_t               nwkLinkStatusPeriodSec;               \
        ZBPRO_NWK_NIB_RouterAgeLimit_t                    nwkRouterAgeLimit;                    \
        ZBPRO_NWK_NIB_UniqueAddr_t                        nwkUniqueAddr;                        \
        ZBPRO_NWK_NIB_LeaveRequestAllowed_t               nwkLeaveRequestAllowed;               \
        ZBPRO_NWK_NIB_RouteRequestId_t                    nwkRouteRequestId;                    \
        ZBPRO_NWK_NIB_MaxLinkRouteCost_t                  nwkMaxLinkRouteCost;                  \
        ZBPRO_NWK_NIB_NetworkChannel_t                    nwkNetworkChannel;                    \
        ZBPRO_NWK_NIB_NetworkPage_t                       nwkNetworkPage;                       \
        ZBPRO_NWK_NIB_DeviceType_t                        nwkDeviceType;                        \
        ZBPRO_NWK_NIB_MaxDepth_t                          nwkDepth;                             \
        ZBPRO_NWK_NIB_ScanAttempts_t                      nwkScanAttempts;                      \
        ZBPRO_NWK_NIB_TimeBtwnScans_t                     nwkTimeBtwnScans;                     \
        Bool8_t                                           nwkAuthorizedFlag;                    \
        Bool8_t                                           nwkConcentratorNoRouteCache;          \
        ZbProSspFrameCnt_t                                nwkFrameCounter;                      \
    }


/**//**
 * \brief NWK-NIB private attributes variant data type.
 */
typedef ZBPRO_NWK_NIB_PRIVATE_VARIANT  ZbProNwkNibAttributeValue_t;

/**//**
 * \brief NWK-NIB public attributes data types list macro.
 */
#define ZBPRO_NWK_NIB_PUBLIC_VARIANT    \
    union                               \
    {                                   \
        MAC_PIB_PUBLIC_VARIANT;         \
        ZBPRO_NWK_NIB_PRIVATE_VARIANT;  \
    }

/**//**
 * \brief NWK-NIB public attributes variant data type.
 */
typedef ZBPRO_NWK_NIB_PUBLIC_VARIANT  ZBPRO_NWK_NibAttributeValue_t;

/**//**
 * \brief NWK-NIB attributes default values.
 */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PAN_ID                             (0)     /* reflected from mac */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SEQUENCE_NUMBER                    (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PASSIVE_ACK_TIMEOUT_OCT            (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_BROADCAST_RETRIES              (0x03U)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_CHILDREN                       (ZBPRO_NWK_NEIGHBOR_TABLE_SIZE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_DEPTH                          ZBPRO_NWK_MAX_DEPTH
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_ROUTERS                        (ZBPRO_NWK_NEIGHBOR_TABLE_SIZE / 2)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_BROADCAST_DELIVERY_TIME \
    (ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PASSIVE_ACK_TIMEOUT * ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_BROADCAST_RETRIES)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_REPORT_CONSTANT_COST               (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TIME_STAMP                         (FALSE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TX_TOTAL                           (0x0000)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SYM_LINK                           (FALSE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CAPABILITY_INFORMATION             (0)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ADDR_ALLOC                         (0x02) // ZBPRO_STOCHASTIC_ADDRESS_ALLOCATION
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_USE_TREE_ROUTING                   (FALSE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MANAGER_ADDR                       (0x0000)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_SOURCE_ROUTE                   (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_UPDATE_ID                          (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TRANSACTION_PERSISTENCE_TIME       (0) /* reflected from mac */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_NETWORK_ADDRESS                    (0) /* reflected from mac */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_STACK_PROFILE                      (0x02)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_EXTENDED_PAN_ID                    (ZBPRO_NWK_UNASSIGNED_EXTENDED_PANID)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_USE_MULTICAST                      (FALSE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_IS_CONCENTRATOR                    (FALSE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_RADIUS                (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_DISCOVERY_TIME_SEC    (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SECURITY_LEVEL                     (0x05)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ACTIVE_KEY_SEQ_NUMBER              (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ALL_FRESH                          (TRUE)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SECURE_ALL_FRAMES                  (TRUE) // TODO: change to true after key initialization.
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_LINK_STATUS_PERIOD_SEC             (0x0f)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ROUTER_AGE_LIMIT                   (0x03)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_UNIQUE_ADDR                        (FALSE) // TODO: check this attribute in address conflict module
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_LEAVE_REQUEST_ALLOWED              (TRUE)  // TODO: use it in leave module
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_PASSIVE_ACK_TIMEOUT                (0x3d09) /* octets duration ~= 500 ms. in 2.4Ghz */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_ROUTE_REQUEST_ID                   (0x00)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MAX_LINK_ROUTE_COST                (0x08) /* Valid range: 0 - 8
                                                                                      0 - ignore information in the neighbor table (always
                                                                                          start route discovery)
                                                                                      8 - always send data directly to a neighbor */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_NETWORK_CHANNEL                    (0) /* reflected from mac */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_NETWORK_PAGE                       (0) /* reflected from mac */
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_DEVICE_TYPE                        (ZBPRO_DEVICE_TYPE_COORDINATOR)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_SCAN_ATTEMPTS                      (5)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_TIME_BTWN_SCANS                    (100)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_MIRRORED_IEEE_ADDR                 (0)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_AUTHORIZED_FLAG                    (0)
#define ZBPRO_NWK_NIB_ATTR_DEFAULT_VALUE_CONCENTRATOR_NO_ROUTE_CACHE        (!ZBPRO_NWK_SRC_ROUTING_TABLE_SIZE_IS_ENOUGH_FOR_CACHE)

/**//**
 * \brief NWK-NIB attributes values constraints.
 */
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_PAN_ID                                (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_PAN_ID                                (0xffff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_SEQUENCE_NUMBER                       (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_SEQUENCE_NUMBER                       (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_PASSIVE_ACK_TIMEOUT_OCT               (0x000000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_PASSIVE_ACK_TIMEOUT_OCT               (0xffffff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_BROADCAST_RETRIES                 (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_BROADCAST_RETRIES                 (0x05)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_CHILDREN                          (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_CHILDREN                          (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_DEPTH                             (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_DEPTH                             (ZBPRO_NWK_MAX_DEPTH)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_ROUTERS                           (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_ROUTERS                           (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_NETWORK_BROADCAST_DELIVERY_TIME_OCT   (0x00000000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_NETWORK_BROADCAST_DELIVERY_TIME_OCT   (0xffffffff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_REPORT_CONSTANT_COST                  (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_REPORT_CONSTANT_COST                  (0x01)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_TX_TOTAL                              (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_TX_TOTAL                              (0xffff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_CAPABILITY_INFORMATION                (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_CAPABILITY_INFORMATION                (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ADDR_ALLOC                            (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ADDR_ALLOC                            (0x03)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MANAGER_ADDR                          (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MANAGER_ADDR                          (0xfff7)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_MAX_SOURCE_ROUTE                      (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_MAX_SOURCE_ROUTE                      (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_UPDATE_ID                             (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_UPDATE_ID                             (0xFF)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_TRANSACTION_PERSISTENCE_TIME          (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_TRANSACTION_PERSISTENCE_TIME          (0xffff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_NETWORK_ADDRESS                       (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_NETWORK_ADDRESS                       (0xfff7)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_STACK_PROFILE                         (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_STACK_PROFILE                         (0x0f)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_EXTENDED_PAN_ID                       (0x0000000000000000ull)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_EXTENDED_PAN_ID                       (0xfffffffffffffffeull)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_CONCENTRATOR_RADIUS                   (0x01)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_CONCENTRATOR_RADIUS                   (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_CONCENTRATOR_DISCOVERY_TIME_SEC       (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_CONCENTRATOR_DISCOVERY_TIME_SEC       (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_SECURITY_LEVEL                        (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_SECURITY_LEVEL                        (0x07)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ACTIVE_KEY_SEQ_NUMBER                 (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ACTIVE_KEY_SEQ_NUMBER                 (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ALL_FRESH                             (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ALL_FRESH                             (0x01)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_SECURE_ALL_FRAMES                     (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_SECURE_ALL_FRAMES                     (0x01)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_LINK_STATUS_PERIOD_SEC                (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_LINK_STATUS_PERIOD_SEC                (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ROUTER_AGE_LIMIT                      (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ROUTER_AGE_LIMIT                      (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ROUTER_SCAN_ATTEMPTS                  (0x00)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ROUTER_SCAN_ATTEMPTS                  (0xff)
#define ZBPRO_NWK_NIB_ATTR_MINALLOWED_ROUTER_TIME_BTWN_SCANS                (0x0000)
#define ZBPRO_NWK_NIB_ATTR_MAXALLOWED_ROUTER_TIME_BTWN_SCANS                (0xffff)

#endif /* _ZBPRO_NWK_SAP_TYPES_IB_H */