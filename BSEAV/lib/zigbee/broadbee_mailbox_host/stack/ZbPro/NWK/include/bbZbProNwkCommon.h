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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkCommon.h $
*
* DESCRIPTION:
*   Network Layer Common declarations
*
* $Revision: 3417 $
* $Date: 2014-08-27 16:19:14Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_COMMON_H
#define _ZBPRO_NWK_COMMON_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbSysQueue.h"
#include "bbSysFsm.h"
#include "bbSysTable.h"
#include "bbPhyBasics.h"
#include "bbMacSapForZBPRO.h"
#include "bbMacSapAddress.h"
#include "bbZbProSsp.h"
#include "bbZbProNwkAttributes.h"
#include "bbZbProNwkConstants.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief Broadcast addresses. See ZigBee spec r20, Table 3.54, page 412.
 */
#define ZBPRO_NWK_ALL_DEVICE_ADDR               (0xFFFFu)
/*      RESERVED                                (0xFFFEu) */
#define ZBPRO_NWK_ALL_RX_ON_WHEN_IDLE_DEVICE_ADDR (0xFFFDu)
#define ZBPRO_NWK_ALL_ROUTER_ADDR               (0xFFFCu)
#define ZBPRO_NWK_ALL_LOW_POWER_ROUTER_ADDR     (0xFFFBu)
/*      RESERVED                                (0xFFFAu)
        RESERVED                                (...)
        RESERVED                                (0xFFF8u) */
#define ZBPRO_NWK_INVALID_SHORT_ADDR            MAC_DONT_USE_SHORT_ADDRESS
/**//**
 * \brief Valid broadcast addresses. See ZigBee spec r20, Table 3.54, page 412.
 */
#define ZBPRO_NWK_IS_BROADCAST_ADDR(addr)       ((addr) >= 0xFFF8u)

/**//**
 * \brief Valid unicast addresses. See ZigBee spec r20, Table 3.44, page 343.
 */
#define ZBPRO_NWK_IS_ADDR_UNICAST(addr)         ((addr) < 0xFFF8u)

/**//**
 * \brief Not valid unicast address. Auxiliary value.
 */
#define ZBPRO_NWK_NOT_VALID_UNICAST_ADDR        (0xFFFFu)

/**//**
 * \brief Network coordinator address.
 */
#define ZBPRO_NWK_COORDINATOR_ADDR                 0x0000U
/**//**
 * \brief True if extended address is valid.
 */
#define ZBPRO_NWK_IS_VALID_EXTENDED_ADDR(extAddr)   (ZBPRO_NWK_NOT_VALID_EXTENDED_ADDR != (extAddr))

/**//**
 * \brief The value of the extendedPanId attribute in NIB has
 *        a value of 0x0000000000000000 indicating that the device is not (or wasn't)
 *        currently joined to a network.
 */
#define ZBPRO_NWK_UNASSIGNED_EXTENDED_PANID     0ULL

#define ZBPRO_NWK_NOT_VALID_EXTENDED_ADDR   (0x0000000000000000ULL)

/**//**
 * \brief Describes maximum network layer data unit size, shall be appropriate ZBPRO_NWK_MaxDuSize_t.
 */
#define ZBPRO_NWK_MAX_DU_SIZE                 50U /* CRITICAL TODO: calculate me */

/**//**
 * \brief Macro for time conversion.
 */
#define ZBPRO_NWK_OCTETS_TO_MILLISECONDS(octets) (((uint32_t)(octets) * 2 * 16) / 1000)
#define ZBPRO_NWK_SYMBOLS_TO_MILLISECONDS(symbols) ZBPRO_NWK_OCTETS_TO_MILLISECONDS((symbols) / 2)
/**//**
 * \brief Maximum network depth allowed by the size of the Device Depth field of the Beacon payload format
 * Refer to ZigBee Spec r20 page 359 line 9
 */
#define ZBPRO_NWK_MAX_DEPTH                 0x0Fu

/************************* TYPES *******************************************************/

/**//**
 * \brief NWK Layer Status Values
 */
typedef enum
{
    /* see ZigBee Specification r20, 3.7 */
    ZBPRO_NWK_SUCCESS                             = 0x00,
    ZBPRO_NWK_INVALID_PARAMETER                   = 0xc1,
    ZBPRO_NWK_INVALID_REQUEST                     = 0xc2,
    ZBPRO_NWK_NOT_PERMITTED                       = 0xc3,
    ZBPRO_NWK_STARTUP_FAILURE                     = 0xc4,
    ZBPRO_NWK_ALREADY_PRESENT                     = 0xc5,
    ZBPRO_NWK_SYNC_FAILURE                        = 0xc6,
    ZBPRO_NWK_NEIGHBOR_TABLE_FULL                 = 0xc7,
    ZBPRO_NWK_UNKNOWN_DEVICE                      = 0xc8,
    ZBPRO_NWK_UNSUPPORTED_ATTRIBUTE               = 0xc9,
    ZBPRO_NWK_NO_NETWORKS                         = 0xca,
    ZBPRO_NWK_MAX_FRM_COUNTER                     = 0xcc,
    ZBPRO_NWK_NO_KEY                              = 0xcd,
    ZBPRO_NWK_BAD_CCM_OUTPUT                      = 0xce,
    ZBPRO_NWK_ROUTE_DISCOVERY_FAILED              = 0xd0,
    ZBPRO_NWK_ROUTE_ERROR                         = 0xd1,
    ZBPRO_NWK_BT_TABLE_FULL                       = 0xd2,
    ZBPRO_NWK_FRAME_NOT_BUFFERED                  = 0xd3,

    /**//**
     * \brief MAC enumeration, see IEEE 802.15.4-2006, 7.1.17
     */
    ZBPRO_NWK_MAC_SUCCESS                         = 0x00,
    ZBPRO_NWK_MAC_BEACON_LOSS                     = 0xe0,
    ZBPRO_NWK_MAC_CHANNEL_ACCESS_FAILURE          = 0xe1,
    ZBPRO_NWK_MAC_COUNTER_ERROR                   = 0xdb,
    ZBPRO_NWK_MAC_DENIED                          = 0xe2,
    ZBPRO_NWK_MAC_DISABLE_TRX_FAILURE             = 0xe3,
    ZBPRO_NWK_MAC_FRAME_TOO_LONG                  = 0xe5,
    ZBPRO_NWK_MAC_IMPROPER_KEY_TYPE               = 0xdc,
    ZBPRO_NWK_MAC_IMPROPER_SECURITY_LEVEL         = 0xdd,
    ZBPRO_NWK_MAC_INVALID_ADDRESS                 = 0xf5,
    ZBPRO_NWK_MAC_INVALID_GTS                     = 0xe6,
    ZBPRO_NWK_MAC_INVALID_HANDLE                  = 0xe7,
    ZBPRO_NWK_MAC_INVALID_INDEX                   = 0xf9,
    ZBPRO_NWK_MAC_INVALID_PARAMETER               = 0xe8,
    ZBPRO_NWK_MAC_LIMIT_REACHED                   = 0xfa,
    ZBPRO_NWK_MAC_NO_ACK                          = 0xe9,
    ZBPRO_NWK_MAC_NO_BEACON                       = 0xea,
    ZBPRO_NWK_MAC_NO_DATA                         = 0xeb,
    ZBPRO_NWK_MAC_NO_SHORT_ADDRESS                = 0xec,
    ZBPRO_NWK_MAC_ON_TIME_TOO_LONG                = 0xf6,
    ZBPRO_NWK_MAC_OUT_OF_CAP                      = 0xed,
    ZBPRO_NWK_MAC_PAN_ID_CONFLICT                 = 0xee,
    ZBPRO_NWK_MAC_PAST_TIME                       = 0xf7,
    ZBPRO_NWK_MAC_READ_ONLY                       = 0xfb,
    ZBPRO_NWK_MAC_REALIGNMENT                     = 0xef,
    ZBPRO_NWK_MAC_SCAN_IN_PROGRESS                = 0xfc,
    ZBPRO_NWK_MAC_SECURITY_ERROR                  = 0xe4,
    ZBPRO_NWK_MAC_SUPERFRAME_OVERLAP              = 0xfd,
    ZBPRO_NWK_MAC_TRACKING_OFF                    = 0xf8,
    ZBPRO_NWK_MAC_TRANSACTION_EXPIRED             = 0xf0,
    ZBPRO_NWK_MAC_TRANSACTION_OVERFLOW            = 0xf1,
    ZBPRO_NWK_MAC_TX_ACTIVE                       = 0xf2,
    ZBPRO_NWK_MAC_UNAVAILABLE_KEY                 = 0xf3,
    ZBPRO_NWK_MAC_UNSUPPORTED_ATTRIBUTE           = 0xf4,
    ZBPRO_NWK_MAC_UNSUPPORTED_LEGACY              = 0xde,
    ZBPRO_NWK_MAC_UNSUPPORTED_SECURITY            = 0xdf
} ZBPRO_NWK_Status_t;

/**//**
 * \brief Forbidden status value.
 *        Shall have unique value.
 *        Is used to determine that the status was raised
 *        uninitialized.
 */
#define ZBPRO_NWK_FORBIDDEN_STATUS_VALUE        0xFE

/**//**
 * \brief The device type enumeration.
 */
typedef enum _ZBPRO_NWK_DeviceType_t
{
    ZBPRO_DEVICE_TYPE_COORDINATOR   = 0x00U,
    ZBPRO_DEVICE_TYPE_ROUTER        = 0x01U,
    ZBPRO_DEVICE_TYPE_END_DEVICE    = 0x02U,
} ZBPRO_NWK_DeviceType_t;

/**//**
 * \brief NWK Timestamp type definition
 */
typedef uint32_t ZBPRO_NWK_Timestamp_t;

/**//**
 * \brief NWK Destination Address mode
 */
typedef enum
{
    ZBPRO_NWK_NOADDR        = 0x00,
    ZBPRO_NWK_MULTICAST     = 0x01,
    ZBPRO_NWK_UNIBROADCAST  = 0x02,
    ZBPRO_NWK_UNICAST       = 0x02
} ZBPRO_NWK_AddrMode_t;

/**//**
 * \brief NWK Allocation Address mode
 */

typedef enum
{
    ZBPRO_DISTRIBUTED_ADDRESS_ALLOCATION    = 0x00,
    // reserved by spec                     = 0x01,
    ZBPRO_STOCHASTIC_ADDRESS_ALLOCATION     = 0x02,
    ZBPRO_LINEAR_ADDRESS_ALLOCATION         = 0x03 /*!< Addition address allocation mode for test proposal. */
} ZBPRO_NWK_AddrAllocationMode_t;

/**//**
 * \brief IEEE 64-bit address
 */
typedef MAC_Addr64bit_t         ZBPRO_NWK_ExtAddr_t;

/**//**
 * \brief 16-bit network address
 */
typedef MAC_Addr16bit_t         ZBPRO_NWK_NwkAddr_t;

/**//**
 * \brief 16-bit Pan Id
 */
typedef MAC_PanId_t             ZBPRO_NWK_PanId_t;

/**//**
 * \brief Type of the 64-bit PAN identifier of the network.
 */
typedef uint64_t                ZBPRO_NWK_ExtPanId_t;

/**//**
 * \brief Type of MAC scan duration type.
 * TODO: use mac type instead uint8_t
 */
typedef uint8_t                 ZBPRO_NWK_ScanDuration_t;

/**//**
 * \brief Type of update identifier.
 * The value identifying a snapshot of the network settings with which this node is operating with.
 */
typedef uint8_t                 ZBPRO_NWK_UpdateId_t;

/**//**
 * \brief Type of transmit failure counter in relation to particular neighbor.
 * A value indicating if previous transmissions to the device were successful or not.
 * Higher values indicate more failures.
 */
typedef uint8_t                 ZBPRO_NWK_TransmitFailure_t;

/**//**
 * \brief Maximum value of the transmit failure counter.
 */
#define ZBPRO_NWK_TRANSMIT_FAILURE_LIMIT    UINT8_MAX

/**//**
 * \brief Type of permit join duration.
 */
typedef uint8_t                 ZBPRO_NWK_PermitJoinDuration_t;

/**//**
 * \brief Type of link cost. ZigBee spec r20, 3.4.8.3.2.
 *  \note
 *  The structure of link cost is described below in C
 *  language style:
 *  \code
 *  typedef struct {
 *      uint8_t         incomingCost : 3,
 *      uint8_t         reserved1    : 2,
 *      uint8_t         outgoingCost : 3,
 *      uint8_t         reserved2    : 2
 *  };
 *  \note
 *  To access link cost fields the special macros shall be used.
 */
typedef uint8_t ZBPRO_NWK_LinkCost_t;

/**//**
 * \brief Macros to work with complex link cost structure.
 */
#define ZBPRO_NWK_GET_LINK_COST_INCOMING(complexLinkCost)            GET_BITFIELD_VALUE(complexLinkCost, 0, 3)
#define ZBPRO_NWK_GET_LINK_COST_RESERVED0(complexLinkCost)           GET_BITFIELD_VALUE(complexLinkCost, 3, 1)
#define ZBPRO_NWK_GET_LINK_COST_OUTGOING(complexLinkCost)            GET_BITFIELD_VALUE(complexLinkCost, 4, 3)
#define ZBPRO_NWK_GET_LINK_COST_RESERVED1(complexLinkCost)           GET_BITFIELD_VALUE(complexLinkCost, 7, 1)

#define ZBPRO_NWK_SET_LINK_COST_INCOMING(complexLinkCost, value)     SET_BITFIELD_VALUE(complexLinkCost, 0, 3, value)
#define ZBPRO_NWK_SET_LINK_COST_RESERVED0(complexLinkCost, value)    SET_BITFIELD_VALUE(complexLinkCost, 3, 1, value)
#define ZBPRO_NWK_SET_LINK_COST_OUTGOING(complexLinkCost, value)     SET_BITFIELD_VALUE(complexLinkCost, 4, 3, value)
#define ZBPRO_NWK_SET_LINK_COST_RESERVED1(complexLinkCost, value)    SET_BITFIELD_VALUE(complexLinkCost, 7, 1, value)

/**//**
 * \brief Type of route path cost. ZigBee spec r20, 3.4.1.3.4.
 */
typedef uint8_t ZBPRO_NWK_PathCost_t;
#define ZBPRO_NWK_FORBIDDEN_COST_VALUE 0xFF

/**//**
 * \brief Type of network connection cost is calculated on the base of LQI.
 */
typedef uint8_t ZBPRO_NWK_Cost_t;

/**//**
 * \brief Type of network depth attribute.
 */
typedef uint8_t ZBPRO_NWK_Depth_t;

/**//**
 * \brief Declaration of the Network Layer service structure.
 *        Is included to the every request descriptor.
 */
typedef struct _ZbProNwkServiceField_t
{
    SYS_QueueElement_t   queueElement;
} ZbProNwkServiceField_t;

/**//**
 * \brief Capability Information Bit-Field structure,
 * see ZigBee Specification r20, Table 3.47,
 * see IEEE 802.15.4-2006, 7.3.1.2
 */
typedef MAC_CapabilityInfo_t ZBPRO_NWK_Capability_t;

#define ZBPRO_NWK_GET_CAPABILITY_INFO_ALTERNATE_PAN_COORDINATOR(cInfo)          GET_BITFIELD_VALUE(cInfo, 0, 1)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_DEVICE_TYPE(cInfo)                        GET_BITFIELD_VALUE(cInfo, 1, 1)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_POWER_SOURCE(cInfo)                       GET_BITFIELD_VALUE(cInfo, 2, 1)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_RX_ON_WHEN_IDLE(cInfo)                    GET_BITFIELD_VALUE(cInfo, 3, 1)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_RESERVED(cInfo)                           GET_BITFIELD_VALUE(cInfo, 4, 2)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_SECURITY_CAPABILITY(cInfo)                GET_BITFIELD_VALUE(cInfo, 6, 1)
#define ZBPRO_NWK_GET_CAPABILITY_INFO_ALLOCATE_ADDRESS(cInfo)                   GET_BITFIELD_VALUE(cInfo, 7, 1)

#define ZBPRO_NWK_SET_CAPABILITY_INFO_ALTERNATE_PAN_COORDINATOR(cInfo, value)   SET_BITFIELD_VALUE(cInfo, 0, 1, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_DEVICE_TYPE(cInfo, value)                 SET_BITFIELD_VALUE(cInfo, 1, 1, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_POWER_SOURCE(cInfo, value)                SET_BITFIELD_VALUE(cInfo, 2, 1, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_RX_ON_WHEN_IDLE(cInfo, value)             SET_BITFIELD_VALUE(cInfo, 3, 1, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_RESERVED(cInfo, value)                    SET_BITFIELD_VALUE(cInfo, 4, 2, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_SECURITY_CAPABILITY(cInfo, value)         SET_BITFIELD_VALUE(cInfo, 6, 1, value)
#define ZBPRO_NWK_SET_CAPABILITY_INFO_ALLOCATE_ADDRESS(cInfo, value)            SET_BITFIELD_VALUE(cInfo, 7, 1, value)


/**//**
 * \brief This parameter controls the method of joining the network.
 */
typedef enum _ZBPRO_NWK_RejoinMethod_t
{
    /* If the device is requesting to join a network through association. */
    ZBPRO_NWK_JOIN_VIA_ASSOCIATION  = 0x00U,
    /* if the device is joining directly or rejoining the network using
            the orphaning procedure. */
    ZBPRO_NWK_JOIN_VIA_ORPHANING    = 0x01U,
    /* if the device is joining the network using the NWK rejoining procedure. */
    ZBPRO_NWK_JOIN_VIA_REJOIN       = 0x02U,
    /* if the device is to change the operational network channel to that
            identified in the ScanChannels parameter. */
    ZBPRO_NWK_JOIN_CHANNEL_CHANGE   = 0x03U,

    /* Service methods do not use them from application. */

    /* if the device is joining the network without any join procedure, just actualizes it attributes. */
    ZBPRO_NWK_JOIN_VIA_COMMISSIONING = 0x04U,
    ZBPRO_NWK_JOIN_VIA_FORMATION    = 0x05U,
    ZBPRO_NWK_JOIN_INVALID_TYPE,
} ZBPRO_NWK_RejoinMethod_t;

/**//**
 * \brief Maximum data unit size. Can not be more then 0x7f.
 */
typedef uint8_t ZBPRO_NWK_MaxDuSize_t;

/**//**
 * \brief Type of beacon payload bit field.
 *  \note
 *  The structure of link cost is described below in C
 *  language style:
 *  \code
 *  typedef struct {
 *      uint8_t   stackProfile       :4,
 *      uint8_t   nwkProtocolVersion :4,
 *      uint8_t   reserved           :2,
 *      uint8_t   routerCapacity     :1,
 *      uint8_t   deviceDepth        :4,
 *      uint8_t   endDeviceCapacity  :1
 *  };
 *  \note
 *  To access link cost fields the special macros shall be used.
 */
typedef uint16_t ZbProNwkBeaconPayloadBitfield_t;

/**//**
 * \brief Macros to work with beacon payload bit field.
 */
#define NWK_GET_BEACON_STACK_PROFILE(beaconBitField)            GET_BITFIELD_VALUE(beaconBitField, 0, 4)
#define NWK_GET_BEACON_PROTOCOL_VERSION(beaconBitField)         GET_BITFIELD_VALUE(beaconBitField, 4, 4)
#define NWK_GET_BEACON_ROUTER_CAPACITY(beaconBitField)          GET_BITFIELD_VALUE(beaconBitField, 10, 1)
#define NWK_GET_BEACON_DEVICE_DEPTH(beaconBitField)             GET_BITFIELD_VALUE(beaconBitField, 11, 4)
#define NWK_GET_BEACON_END_DEVICE_CAPACITY(beaconBitField)      GET_BITFIELD_VALUE(beaconBitField, 15, 1)

#define NWK_SET_BEACON_STACK_PROFILE(beaconBitField, value)         SET_BITFIELD_VALUE(beaconBitField, 0, 4, value)
#define NWK_SET_BEACON_PROTOCOL_VERSION(beaconBitField, value)      SET_BITFIELD_VALUE(beaconBitField, 4, 4, value)
#define NWK_SET_BEACON_ROUTER_CAPACITY(beaconBitField, value)       SET_BITFIELD_VALUE(beaconBitField, 10, 1, value)
#define NWK_SET_BEACON_DEVICE_DEPTH(beaconBitField, value)          SET_BITFIELD_VALUE(beaconBitField, 11, 4, value)
#define NWK_SET_BEACON_END_DEVICE_CAPACITY(beaconBitField, value)   SET_BITFIELD_VALUE(beaconBitField, 15, 4, value)

/**//**
 * \brief The beacon payload shall contain the information shown in ZigBee spec r20,
 *        Table 3.56. This enables the NWK layer to provide additional information
 *        to new devices that are performing network discovery and allows these new
 *        devices to more efficiently select a network and a particular neighbor
 *        to join.
 */
typedef struct _ZbProNwkParsedBeaconPayload_t
{
    uint8_t protocolId;                         /*!< This field identifies the network layer protocols in use and,
                                                     for purposes of this specification, shall always be set to 0,
                                                     indicating the ZigBee protocols. */

    ZbProNwkBeaconPayloadBitfield_t options;    /*!< This field contains bitfields (bits 8-23) from
                                                     beacon payload structure. */


    ZBPRO_NWK_ExtPanId_t nwkExtendedPanid;      /*!< The globally unique ID for the PAN of which the beaconing
                                                   device is a member. */

    uint8_t txOffset[3];                        /*!< This value indicates the difference in time, measured in
                                                   symbols, between the beacon transmission time of the device
                                                   and the beacon transmission time of its parent. */

    ZBPRO_NWK_UpdateId_t updateId;              /*!< This field reflects the value of nwkUpdateId from the NIB. */
} ZbProNwkParsedBeaconPayload_t;

/**//**
 * \brief Beacon notify type.
 */
typedef struct _ZbProNwkParsedBeaconNotify_t
{
    MAC_BeaconNotifyIndParams_t indParams;
    ZbProNwkParsedBeaconPayload_t     beaconPayload;
} ZbProNwkParsedBeaconNotify_t;

#endif /* _ZBPRO_NWK_COMMON_H */