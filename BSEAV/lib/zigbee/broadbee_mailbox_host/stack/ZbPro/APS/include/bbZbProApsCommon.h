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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsCommon.h $
 *
 * DESCRIPTION:
 *   Contains general types declarations for the ZigBee PRO APS component.
 *
 * $Revision: 6992 $
 * $Date: 2015-06-05 14:04:50Z $
 *
 ****************************************************************************************/


#ifndef _ZBPRO_APS_COMMON_H
#define _ZBPRO_APS_COMMON_H


/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbSysQueue.h"
#include "bbMacSapDefs.h"
#include "bbZbProNwkCommon.h"
#include "bbZbProApsConfig.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief NWK broadcast addresses
 */
#define ZBPRO_APS_SHORT_ADDRESS_ALL                     ZBPRO_NWK_ALL_DEVICE_ADDR
#define ZBPRO_APS_SHORT_ADDRESS_ALL_RX_ON_WHEN_IDLE     ZBPRO_NWK_ALL_RX_ON_WHEN_IDLE_DEVICE_ADDR
#define ZBPRO_APS_SHORT_ADDRESS_ALL_ROUTERS             ZBPRO_NWK_ALL_ROUTER_ADDR
#define ZBPRO_APS_SHORT_ADDRESS_ALL_LOW_POWER_ROUTERS   ZBPRO_NWK_ALL_LOW_POWER_ROUTER_ADDR

/**//**
 * \brief Special Extended Addresses which match broadcast short addresses
 */
#define ZBPRO_APS_EXT_ADDRESS_ALL                       0xFFFFFFFFFFFFFFFFul
#define ZBPRO_APS_EXT_ADDRESS_ALL_RX_ON_WHEN_IDLE       0xFFFFFFFFFFFFFFFDul
#define ZBPRO_APS_EXT_ADDRESS_ALL_ROUTERS               0xFFFFFFFFFFFFFFFCul
#define ZBPRO_APS_EXT_ADDRESS_ALL_LOW_POWER_ROUTERS     0xFFFFFFFFFFFFFFFBul

#define ZBPRO_APS_IS_EXT_ADDRESS_UNICAST(addr)          ((addr) < ZBPRO_APS_EXT_ADDRESS_ALL_LOW_POWER_ROUTERS)

/**//**
 * \brief Checks a address information contained in ZBPRO_APS_Address_t and returns true if it's a unicast address.
 */
#define ZBPRO_APS_IS_ADDRESS_DESCRIPTION_UNICAST(pApsAddrStruct) \
    ((APS_INDIRECT_MODE          == (pApsAddrStruct)->addrMode) ? false : \
     (APS_GROUP_ADDRESS_MODE     == (pApsAddrStruct)->addrMode) ? false : \
     (APS_SHORT_ADDRESS_MODE     == (pApsAddrStruct)->addrMode) ? ZBPRO_NWK_IS_ADDR_UNICAST((pApsAddrStruct)->shortAddr) : \
     (APS_EXTENDED_ADDRESS_MODE  == (pApsAddrStruct)->addrMode) ? ZBPRO_APS_IS_EXT_ADDRESS_UNICAST((pApsAddrStruct)->extAddr) : \
     false)

/**//**
 * \brief Invalid Extended Address
 */
#define ZBPRO_APS_EXT_ADDRESS_INVALID    0ull

/**//**
 * \brief Check if an Extended Address is valid
 */
#define ZBPRO_APS_EXT_ADDRESS_IS_VALID(addr)    (ZBPRO_APS_EXT_ADDRESS_INVALID != addr)

/**//**
 * \brief Is the specified extended address a broadcast address?
 */
#define ZBPRO_APS_EXT_ADDRESS_IS_BROADCAST(addr)    ((addr) >= ZBPRO_APS_EXT_ADDRESS_ALL_LOW_POWER_ROUTERS)

/**//**
 * \brief Describes the maximum application support layer data unit size (may be fragmented),
 *      shall be appropriate ZBPRO_APS_MaxDuSize_t.
 */
#define ZBPRO_APS_MAX_DU_SIZE                 50U /* CRITICAL TODO: calculate me */

/************************* TYPES *******************************************************/

/**//**
 * \brief All possible APS layer result codes.
 */
typedef enum _ZBPRO_APS_Status_t
{
    /** A request has been executed successfully. */
    ZBPRO_APS_SUCCESS_STATUS                 = 0x00,
    /** A transmit request has failed since the ASDU is too large and fragmentation is not
    supported. */
    ZBPRO_APS_ASDU_TOO_LONG_STATUS           = 0xa0,
    /** A received fragmented frame cannot be defragmented currently. */
    ZBPRO_APS_DEFRAG_DEFERRED_STATUS         = 0xa1,
    /** A received fragmented frame cannot be defragmented, because the device does not support
    fragmentation. */
    ZBPRO_APS_DEFRAG_UNSUPPORTED_STATUS      = 0xa2,
    /** APS is in a state when request's execution is impossible. */
    ZBPRO_APS_ILLEGAL_REQUEST_STATUS         = 0xa3,
    /** An APSME-UNBIND request has failed, because the requested binding link does not exist in
    the binding table. */
    ZBPRO_APS_INVALID_BINDING_STATUS         = 0xa4,
    /** An APSME-REMOVE-GROUP request has been issued with a group identifier that is absent in
    the group table. */
    ZBPRO_APS_INVALID_GROUP_STATUS           = 0xa5,
    /** A parameter's value is invalid or out of range. */
    ZBPRO_APS_INVALID_PARAMETER_STATUS       = 0xa6,
    /** An APSDE-DATA request requesting acknowledged transmission has failed because of no
    acknowledgement being received. */
    ZBPRO_APS_NO_ACK_STATUS                  = 0xa7,
    /** An APSDE-DATA request with the destination addressing mode set to 0x00 has failed, because
    no devices are bound to this device. */
    ZBPRO_APS_NO_BOUND_DEVICE_STATUS         = 0xa8,
    /** An APSDE-DATA request with the destination addressing mode set to 0x03 has failed, because
    the corresponding short address has not been found in the address map table. */
    ZBPRO_APS_NO_SHORT_ADDRESS_STATUS        = 0xa9,
    /** An APSDE-DATA request with the destination addressing mode set to 0x00 has failed, because
    the binding table is not supported on the device. */
    ZBPRO_APS_NOT_SUPPORTED_STATUS           = 0xaa,
    /** An ASDU secured using a link key has been received . */
    ZBPRO_APS_SECURED_LINK_KEY_STATUS        = 0xab,
    /** An ASDU secured using a network key has been received. */
    ZBPRO_APS_SECURED_NWK_KEY_STATUS         = 0xac,
    /** An APSDE-DATA request requesting security has resulted in an error during the corresponding
    security processing. */
    ZBPRO_APS_SECURITY_FAIL_STATUS           = 0xad,
    /** An APSME-BIND request or APSME-ADD-GROUP request have been issued while the binding table
    or the group table, respectively, is full. */
    ZBPRO_APS_TABLE_FULL_STATUS              = 0xae,
    /** An unsecured ASDU has been received. */
    ZBPRO_APS_UNSECURED_STATUS               = 0xaf,
    /** An APSME-GET request or APSME-SET request has been issued with an unknown attribute's
    identifier. */
    ZBPRO_APS_UNSUPPORTED_ATTRIBUTE_STATUS = 0xb0,
} ZBPRO_APS_Status_t;

/**//**
 * \brief Type is used to store the device manufacturer code.
 * \details
 *  Zero value is not allowed for manufacturer identifier. It may be used in particular
 *  case as a flag that object belongs to standardized domain but not to manufacturer
 *  specific domain.
 */
typedef uint16_t                ZBPRO_APS_ManufacturerCode_t;

/**//**
 * \brief Type is used to store short (network) address.
 */
typedef ZBPRO_NWK_NwkAddr_t     ZBPRO_APS_ShortAddr_t;

/**//**
 * \brief Type is used to store extended (IEEE) address.
 */
typedef ZBPRO_NWK_ExtAddr_t     ZBPRO_APS_ExtAddr_t;

/**//**
 * \brief Type of the 64-bit PAN identifier of the network.
 */
typedef ZBPRO_NWK_ExtPanId_t    ZBPRO_APS_ExtPanId_t;

/**//**
 * \brief Type is intended to store group identifier.
 */
typedef uint16_t                ZBPRO_APS_GroupId_t;

/**//**
 * \brief   Type is intended to store endpoint identifier.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.1.2.
 */
typedef uint8_t                 ZBPRO_APS_EndpointId_t;

/**//**
 * \brief Network Radius
 */
typedef uint8_t                 ZBPRO_APS_NwkRadius_t;

/**//**
 * \brief Delay between two chunks of the fragmented message.
 */
typedef uint8_t                 ZBPRO_APS_InterframeDelay_t;

/**//**
 * \brief Energy level on channel.
 */
typedef PHY_Ed_t                ZBPRO_APS_ChannelEnergyLevel_t;

/**//**
 * \brief Percentage of transmission failures.
 */
typedef uint8_t                 ZBPRO_APS_FailureRate_t;

/**//**
 * \brief Countdown timer (in hours) to the next channel change.
 */
typedef uint8_t                 ZBPRO_APS_ChannelTimer_t;

/**//**
 * \brief The period of time a device will wait for the next expected security protocol frame.(in milliseconds)
 */
typedef uint16_t                ZBPRO_APS_SucurityTimeOutPeriod_t;

/**//**
 * \brief   ZDO endpoint ID.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.1.2.
 */
#define ZBPRO_APS_ENDPOINT_ID_ZDO           0x00u

/**//**
 * \brief   Broadcast endpoint ID.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.1.2.
 */
#define ZBPRO_APS_ENDPOINT_ID_BROADCAST     0xFFu

/**//**
 * \brief   Type is intended to store profile identifier.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.2.4.1.1.1, table 2.2.
 */
typedef enum _ZBPRO_APS_ProfileId_t
{
    ZBPRO_APS_PROFILE_ID_ZHA                = 0x0104,   /* ZigBee Home Automation Profile Id */
    ZBPRO_APS_PROFILE_ID_TEST               = 0x7F01,   /* Test Profile Id */
    ZBPRO_APS_PROFILE_STANDARD_RANGE_MAX    = 0x7FFF,   /* Standard Profile range */
    ZBPRO_APS_PROFILE_ID_WILDCARD           = 0xFFFFu,  /* WildCard Profile Id (ZigBee 053474r20, 2.3.3.1.)*/

    ZBPRO_APS_PROFILE_ID_FORCE_TO_16BIT     = 0xFFFFu   /* to force enum occupy 16 bits */
} ZBPRO_APS_ProfileId_t;
//SYS_DbgAssertStatic(sizeof(ZBPRO_APS_ProfileId_t) == 2);    /* check of ZBPRO_APS_ProfileId_t actual size */ /* deleted by huajun for short-enum */

/**//**
 * \brief Type is intended to store device identifier. (shall be obtained from the ZigBee Alliance.)
 */
typedef uint16_t                ZBPRO_APS_DeviceId_t;

/**//**
 * \brief Type is intended to store device version.
 */
typedef uint8_t                 ZBPRO_APS_DeviceVersion_t;

/**//**
 * \brief   Default numeric value of device version.
 * \detsils
 *  This default value must be used in the case when profile doesn't define version for
 *  its device.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.3.2.5.4, table 2.40.
 */
#define ZBPRO_APS_DEVICE_VERSION_DEFAULT    0x0

/**//**
 * \brief   Maximum numeric value of device version.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.3.2.5.4, table 2.40.
 */
#define ZBPRO_APS_DEVICE_VERSION_MAX        0xF

/**//**
 * \brief   Type is intended to store cluster identifier.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.2.4.1.1.1, table 2.2.
 */
typedef uint16_t                ZBPRO_APS_ClusterId_t;

/**//**
 * \brief Type is intended to store cluster amount.
 */
typedef uint8_t                 ZBPRO_APS_ClusterAmount_t;

/**//**
 * \brief Type describes addressing mode. Refer to ZigBee Spec r20 table 2.2.
 */
typedef enum
{
    /** Indirect addressing (using binding) */
    APS_INDIRECT_MODE = 0x00,
    /** Group addressing mode */
    APS_GROUP_ADDRESS_MODE = 0x01,
    /** Unicast or broadcast addressing mode, with a 16-bit network (short) address */
    APS_SHORT_ADDRESS_MODE = 0x02,
    /** Unicast addressing mode, with a 64-bit IEEE (extended) address. (not supported for transmission)*/
    APS_EXTENDED_ADDRESS_MODE = 0x03
} ZBPRO_APS_AddressMode_t;

/**//**
 * \brief Type contains addressing information which could be used to store
 * either destination or source addressing.
 */
typedef struct _ZBPRO_APS_Address_t
{
    ZBPRO_APS_AddressMode_t addrMode;
    union
    {
        ZBPRO_APS_ExtAddr_t   extAddr;
        ZBPRO_APS_ShortAddr_t shortAddr;
        ZBPRO_APS_GroupId_t   groupAddr;
    };
} ZBPRO_APS_Address_t;

/**//**
 * \brief APS Timestamp type definition
 */
typedef uint32_t ZBPRO_APS_Timestamp_t;

/**//**
 * \brief ZigBeePRO APS Command Ids which corresponds with
 * Transceiver requester Ids
 */
typedef enum _ZbProApsCommandId_t
{
    ZBPRO_APS_DATA_ID                              = 0x00,  /* used internally */
    ZBPRO_APS_ACK_ID                               = 0x01,  /* used internally */
    ZBPRO_APS_LAST_NONCMD_ID                       = ZBPRO_APS_ACK_ID,

    ZBPRO_APS_CMD_TRANSPORT_KEY_ID                 = 0x05,
    ZBPRO_APS_FIRST_CMD_ID                         = ZBPRO_APS_CMD_TRANSPORT_KEY_ID,
    /* Commands which comply with common Security Policy */
    ZBPRO_APS_CMD_UPDATE_DEVICE_ID                 = 0x06,
    ZBPRO_APS_CMD_SECURITYPOLICY_FIRST_ID          = ZBPRO_APS_CMD_UPDATE_DEVICE_ID,
    ZBPRO_APS_CMD_REMOVE_DEVICE_ID                 = 0x07,
    ZBPRO_APS_CMD_REQUEST_KEY_ID                   = 0x08,
    ZBPRO_APS_CMD_SWITCH_KEY_ID                    = 0x09,
    ZBPRO_APS_CMD_SECURITYPOLICY_LAST_ID           = ZBPRO_APS_CMD_SWITCH_KEY_ID,
    ZBPRO_APS_LAST_CONSECUTIVE_CMD_ID              = ZBPRO_APS_CMD_SWITCH_KEY_ID,

    ZBPRO_APS_CMD_TUNNEL_ID                        = 0x0E,  /* special command to embed Transport key */

    ZBPRO_APS_ID_AMOUNT
} ZbProApsCommandId_t;

/**//**
 * \brief Address and endpoint aggregate type
 */
typedef struct _ZbProApsPeer_t
{
    ZBPRO_APS_Address_t     addr;
    ZBPRO_APS_EndpointId_t  endpoint;
} ZbProApsPeer_t;

/**//**
 * \brief Aps counter type.
 */
typedef uint8_t ZbProApsCounter_t;

/**//**
 * \brief Maximum data unit size. Can not be more then 0x7fff.
 */
typedef uint16_t ZBPRO_APS_MaxDuSize_t;

#endif /* _ZBPRO_APS_COMMON_H */
