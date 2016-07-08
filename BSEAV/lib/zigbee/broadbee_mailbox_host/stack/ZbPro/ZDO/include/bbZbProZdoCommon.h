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
* FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoCommon.h $
*
* DESCRIPTION:
*   ZDO common definitions and ZDO initialization function interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_COMMON_H
#define _BB_ZBPRO_ZDO_COMMON_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoAttributes.h"
#include "bbZbProApsCommon.h"
#include "bbRpc.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for the service field of Local Client ZDO Request object.
 * \details
 *  This structure shall be embedded as a service field into each particular Local Client
 *  ZDO Request structure.
 */
typedef RpcLocalRequest_t  ZbProZdoLocalRequest_t;


/**//**
 * \brief   Enumeration of ZDO/ZDP Statuses.
 * \par     Documentation
 *  See ZigBee Document 053474r20, table 2.138.
 */
typedef enum _ZBPRO_ZDO_Status_t
{
    ZBPRO_ZDO_SUCCESS                       = 0x00,     /*!< The requested operation or transmission was completed
                                                            successfully. */

    /*                               0x01 ... 0x7F         < reserved. */

    ZBPRO_ZDO_AUTHENTICATION_NOT_SUPPORTED  = 0x70,     /*!< Custom status AUTHENTICATION_NOT_SUPPORTED. */

    ZBPRO_ZDO_AUTHENTICATION_FAIL           = 0x71,     /*!< Custom status AUTHENTICATION_FAIL. */

    ZBPRO_ZDO_AUTHENTICATION_TIMEOUT        = 0x72,     /*!< Custom status AUTHENTICATION_TIMEOUT. */

    ZBPRO_ZDO_INVALID_REQUEST_TYPE          = 0x80,     /*!< The supplied request type was invalid. */

    ZBPRO_ZDO_DEVICE_NOT_FOUND              = 0x81,     /*!< The requested device did not exist on a device following a
                                                            child descriptor request to a parent. */

    ZBPRO_ZDO_INVALID_END_POINT             = 0x82,     /*!< The supplied endpoint was equal to 0x00 or 0xff. */

    ZBPRO_ZDO_NOT_ACTIVE                    = 0x83,     /*!< The requested endpoint is not described by a simple
                                                            descriptor. */

    ZBPRO_ZDO_NOT_SUPPORTED                 = 0x84,     /*!< The requested optional feature is not supported on the
                                                            target device. */

    ZBPRO_ZDO_TIMEOUT                       = 0x85,     /*!< A timeout has occurred with the requested operation. */

    ZBPRO_ZDO_NO_MATCH                      = 0x86,     /*!< The end device bind request was unsuccessful due to a
                                                            failure to match any suitable clusters. */

    /*                                        0x87         < reserved. */

    ZBPRO_ZDO_INTERNAL_REJECT               = 0x87,     /*!< This status value is used by ZDO FSM internally. A ZDO
                                                            service can sets up the buffer status to point out that the
                                                            ZDO dispatcher FSM has to do not send response. */

    ZBPRO_ZDO_NO_ENTRY                      = 0x88,     /*!< The unbind request was unsuccessful due to the coordinator
                                                            or source device not having an entry in its binding table to
                                                            unbind. */

    ZBPRO_ZDO_NO_DESCRIPTOR                 = 0x89,     /*!< A child descriptor was not available following a discovery
                                                            request to a parent. */

    ZBPRO_ZDO_INSUFFICIENT_SPACE            = 0x8A,     /*!< The device does not have storage space to support the
                                                            requested operation. */

    ZBPRO_ZDO_NOT_PERMITTED                 = 0x8B,     /*!< The device is not in the proper state to support the
                                                            requested operation. */

    ZBPRO_ZDO_TABLE_FULL                    = 0x8C,     /*!< The device does not have table space to support the
                                                            operation. */

    ZBPRO_ZDO_NOT_AUTHORIZED                = 0x8D,     /*!< The permissions configuration table on the target indicates
                                                            that the request is not authorized from this device. */

    ZBPRO_ZDO_DEVICE_BINDING_TABLE_FULL     = 0x8E,     /*!< The device does not have binding table space to support the
                                                            operation. */

    /*                               0x8F ... 0xFF         < reserved. */

    ZBPRO_ZDO_CHANNEL_SWITCHED              = 0x8F,     /*!< Channel was switched within processing of Mgmt_NWK_Update
                                                            command. */
} ZBPRO_ZDO_Status_t;


/**//**
 * \name    Redefinition of different data types for ZDO layer.
 */
/**@{*/
typedef ZBPRO_NWK_Capability_t  ZBPRO_ZDO_Capability_t;     /*!< Node capabilities. */

typedef ZBPRO_APS_ExtAddr_t     ZBPRO_ZDO_ExtAddr_t;        /*!< Extended 64-bit MAC (IEEE) address. */

typedef ZBPRO_NWK_NwkAddr_t     ZBPRO_ZDO_NwkAddr_t;        /*!< Short 16-bit NWK address. */

typedef ZBPRO_NWK_ExtPanId_t    ZBPRO_ZDO_ExtPanId_t;       /*!< Extended 64-bit NWK PAN identifier. */

typedef ZBPRO_NWK_PanId_t       ZBPRO_ZDO_PanId_t;          /*!< Short 16-bit PAN identifier. */

typedef ZBPRO_APS_Address_t     ZBPRO_ZDO_Address_t;        /*!< Structure for storing either Short, or Extended node
                                                                address, or Group address. */

typedef ZBPRO_APS_ProfileId_t   ZBPRO_ZDO_ProfileId_t;      /*!< Profile identifier. */

typedef ZBPRO_APS_EndpointId_t  ZBPRO_ZDO_Endpoint_t;       /*!< Endpoint identifier. */

typedef ZBPRO_APS_ClusterId_t   ZBPRO_ZDO_ClusterId_t;      /*!< Cluster identifier. */

typedef ZBPRO_NWK_DeviceType_t  ZBPRO_ZDO_DeviceType_t;     /*!< Device Type. */

/**//**
 * \brief Server mask type, the node descriptor part. For more detailed please see spec R20 chapter 2.3.2.3.10 p.85
 */
typedef union _ZBPRO_ZDO_ServerMask_t
{
    BitField16_t     plain;
    struct
    {
        BitField16_t    primaryTrustCenter : 1;
        BitField16_t    backupTrustCenter : 1;
        BitField16_t    primaryBindingTableCache : 1;
        BitField16_t    backupBindingTableCache : 1;
        BitField16_t    primaryDiscoveryCache: 1;
        BitField16_t    backupDiscoveryCache: 1;
        BitField16_t    networkManager: 1;
        BitField16_t    reserved : 8;
    };
} ZBPRO_ZDO_ServerMask_t;
/**@}*/


/**//**
 * \brief   Numeric value of ZDP profile identifier.
 * \note
 *  ZDP profile identifier is not specified by the official ZigBee PRO Specification, but
 *  it is assumed to be 0x0000 by ZigBee Pro Compliant Platform Test Specification.
 * \par     Documentation
 *  See ZigBee Document 07-5035-06, subclause 10.69.3.
 */
#define ZBPRO_ZDP_PROFILE_ID    0x0000


/**//**
 * \brief   Enumeration of ZDP Services.
 * \details
 *  Actual sets of ZDP Client Services and ZDP Server Services are merged by corresponding
 *  request-response clusters' pairs of client and server. For each pair of client and
 *  server services only single ZDP Service is presented in this enumeration. Identifier
 *  numeric value of such a ZDP Service is set to the corresponding ZDP Client Service
 *  cluster identifier.
 * \note
 *  Identifier of the corresponding ZDP Service Response cluster is equal to ZDP Service
 *  identifier (or ZDP Client Service cluster identifier) plus 0x8000.
 * \note
 *  These 16-bit ZDP Services' identifiers are numerically equal to their corresponding
 *  RPC Service identifiers in 32-bits format and may be used instead of RPC Service
 *  identifiers directly because ZDP Profile identifier (that is presented in the higher
 *  16 bits of an RPC Service identifier) equals to 0x0000.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclauses 2.4.3, 2.4.4.
 */
typedef enum _ZbProZdpServiceId_t
{
    ZDP_SERVICE_ID_NWK_ADDR                 = 0x0000,       /*!< Inquire as to the 16-bit address of the Remote Device
                                                                based on its known IEEE address. */

    ZDP_SERVICE_ID_IEEE_ADDR                = 0x0001,       /*!< Inquire as to the 64-bit IEEE address of the Remote
                                                                Device based on their known 16-bit address. */

    ZDP_SERVICE_ID_NODE_DESC                = 0x0002,       /*!< Inquire as to the node descriptor of a remote
                                                                device. */

    ZDP_SERVICE_ID_POWER_DESC               = 0x0003,       /*!< Inquire as to the power descriptor of a remote
                                                                device. */

    ZDP_SERVICE_ID_SIMPLE_DESC              = 0x0004,       /*!< Inquire as to the simple descriptor of a remote
                                                                device on a specified endpoint. */

    ZDP_SERVICE_ID_ACTIVE_EP                = 0x0005,       /*!< Acquire the list of endpoints on a remote device with
                                                                simple descriptors. */

    ZDP_SERVICE_ID_MATCH_DESC               = 0x0006,       /*!< Find remote devices supporting a specific simple
                                                                descriptor match criterion. */

    ZDP_SERVICE_ID_COMPLEX_DESC             = 0x0010,       /*!< Inquire as to the complex descriptor of a remote
                                                                device. */

    ZDP_SERVICE_ID_USER_DESC                = 0x0011,       /*!< Inquire as to the user descriptor of a remote
                                                                device. */

    ZDP_SERVICE_ID_DISCOVERY_CACHE          = 0x0012,       /*!< Locate a Primary Discovery Cache device on the
                                                                network. */

    ZDP_SERVICE_ID_DEVICE_ANNCE             = 0x0013,       /*!< Notify other ZigBee devices that the device has joined
                                                                or re-joined the network. */

    ZDP_SERVICE_ID_USER_DESC_SET            = 0x0014,       /*!< Configure the user descriptor on a remote device. */

    ZDP_SERVICE_ID_SYSTEM_SERVER_DISCOVERY  = 0x0015,       /*!< Discover the location of a particular system server or
                                                                servers. */

    ZDP_SERVICE_ID_DISCOVERY_STORE          = 0x0016,       /*!< Request storage for discovery cache information on a
                                                                Primary Discovery Cache device. */

    ZDP_SERVICE_ID_NODE_DESC_STORE          = 0x0017,       /*!< Request storage for Node Descriptor on a Primary
                                                                Discovery Cache device. */

    ZDP_SERVICE_ID_POWER_DESC_STORE         = 0x0018,       /*!< Request storage for Power Descriptor on a Primary
                                                                Discovery Cache device. */

    ZDP_SERVICE_ID_ACTIVE_EP_STORE          = 0x0019,       /*!< Request storage for the list of Active Endpoints on a
                                                                Primary Discovery Cache device. */

    ZDP_SERVICE_ID_SIMPLE_DESC_STORE        = 0x001A,       /*!< Request storage for the list of Simple Descriptors on a
                                                                Primary Discovery Cache device. */

    ZDP_SERVICE_ID_REMOVE_NODE_CACHE        = 0x001B,       /*!< Request removal of discovery cache information for a
                                                                specified ZigBee end device from a Primary Discovery
                                                                Cache device. */

    ZDP_SERVICE_ID_FIND_NODE_CACHE          = 0x001C,       /*!< Broadcast to all devices for which macRxOnWhenIdle
                                                                equals to TRUE a request to find a device on the network
                                                                that holds discovery information for the device of
                                                                interest. */

    ZDP_SERVICE_ID_EXTENDED_SIMPLE_DESC     = 0x001D,       /*!< Inquire as to the simple descriptor of a remote
                                                                device on a specified endpoint. */

    ZDP_SERVICE_ID_EXTENDED_ACTIVE_EP       = 0x001E,       /*!< Acquire the list of endpoints on a remote device with
                                                                simple descriptors. */

    ZDP_SERVICE_ID_END_DEVICE_BIND          = 0x0020,       /*!< Perform End Device Bind with a Remote Device. */

    ZDP_SERVICE_ID_BIND                     = 0x0021,       /*!< Create a Binding Table entry for the source and
                                                                destination addresses contained as parameters. */

    ZDP_SERVICE_ID_UNBIND                   = 0x0022,       /*!< Remove a Binding Table entry for the source and
                                                                destination addresses contained as parameters. */

    ZDP_SERVICE_ID_BIND_REGISTER            = 0x0023,       /*!< Register that the requesting local device wishes to
                                                                hold its own binding table entries. */

    ZDP_SERVICE_ID_REPLACE_DEVICE           = 0x0024,       /*!< Requests the primary binding table cache device to
                                                                change all binding table entries which match OldAddress
                                                                and OldEndpoint as specified. */

    ZDP_SERVICE_ID_STORE_BKUP_BIND_ENTRY    = 0x0025,       /*!< Requests the remote backup binding table cache device
                                                                to backup storage of the entry. */

    ZDP_SERVICE_ID_REMOVE_BKUP_BIND_ENTRY   = 0x0026,       /*!< Requests the remote backup binding table cache device
                                                                to remove the entry from backup storage. */

    ZDP_SERVICE_ID_BACKUP_BIND_TABLE        = 0x0027,       /*!< Requests the remote backup binding table cache device
                                                                to backup storage of its entire binding table. */

    ZDP_SERVICE_ID_RECOVER_BIND_TABLE       = 0x0028,       /*!< Requests the remote backup binding table cache device
                                                                to completely restore the binding table. */

    ZDP_SERVICE_ID_BACKUP_SOURCE_BIND       = 0x0029,       /*!< Requests the remote backup binding table cache device
                                                                to backup storage of its entire source table. */

    ZDP_SERVICE_ID_RECOVER_SOURCE_BIND      = 0x002A,       /*!< Requests the remote backup binding table cache device
                                                                to completely restore the source binding table. */

    ZDP_SERVICE_ID_MGMT_NWK_DISC            = 0x0030,       /*!< Request the Remote Device execute a Scan to report back
                                                                networks in the vicinity of the Local Device. */

    ZDP_SERVICE_ID_MGMT_LQI                 = 0x0031,       /*!< Obtain a neighbor list for the Remote Device along with
                                                                associated LQI values to each neighbor. */

    ZDP_SERVICE_ID_MGMT_RTG                 = 0x0032,       /*!< Retrieve the contents of the Routing Table from the
                                                                Remote Device. */

    ZDP_SERVICE_ID_MGMT_BIND                = 0x0033,       /*!< Retrieve the contents of the Binding Table from the
                                                                Remote Device. */

    ZDP_SERVICE_ID_MGMT_LEAVE               = 0x0034,       /*!< Request the Remote Device or another device to leave
                                                                the network. */

    ZDP_SERVICE_ID_MGMT_DIRECT_JOIN         = 0x0035,       /*!< Request the Remote Device to permit a device designated
                                                                by DeviceAddress to join the network directly. */

    ZDP_SERVICE_ID_MGMT_PERMIT_JOINING      = 0x0036,       /*!< Request a remote device or devices to allow or disallow
                                                                association. */

    ZDP_SERVICE_ID_MGMT_CACHE               = 0x0037,       /*!< Retrieve a list of ZigBee End Devices registered with a
                                                                Primary Discovery Cache device. */

    ZDP_SERVICE_ID_MGMT_NWK_UPDATE          = 0x0038,       /*!< Update network configuration parameters or request
                                                                information from devices on network conditions in the
                                                                local operating environment. */

    ZDP_SERVICE_ID_MAX                      = 0x7FFF,       /*!< Maximum value of ZDP Service identifier. This value
                                                                also makes the enumeration 16-bit integer type. */
} ZbProZdpServiceId_t;

/*
 * Validate ZDP Service Id enumeration data type size.
 */
SYS_DbgAssertStatic(2 == sizeof(ZbProZdpServiceId_t));

typedef struct _ZBPRO_ZDO_ReadyIndParams_t
{
    ZBPRO_ZDO_Status_t status;
} ZBPRO_ZDO_ReadyIndParams_t;

/************************* FUNCTIONS PROTOTYPES *****************************************/
/**//**
 * \brief   Initializes ZDO/ZDP, ZigBee PRO APS and NWK layers.
 * \details
 *  Call this function ones at application startup.
 */
void ZBPRO_ZDO_Initialization(void);

/**//**
 * \brief Notifies upper layer when the ZDO finish initialization procedure
 */
void ZBPRO_ZDO_ReadyInd(ZBPRO_ZDO_ReadyIndParams_t *ind);

#endif /* _BB_ZBPRO_ZDO_COMMON_H */
