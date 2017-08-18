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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the RF4CE Network Layer component discovery handler.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_DISCOVERY_H
#define _RF4CE_NWK_DISCOVERY_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbSysTaskScheduler.h"
#include "bbSysQueue.h"
#include "bbMacSapForRF4CE.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE NWK Application Capabilities GET/SET fields macros.
 */
#define RF4CE_NWK_APP_CAPS_GET_STRING_SPECIFIED(data) (((uint8_t)(data)) & 1)
#define RF4CE_NWK_APP_CAPS_SET_STRING_SPECIFIED(data, value) ((((uint8_t)(data)) & 0xFE) | ((value) != 0 ? 1 : 0))
#define RF4CE_NWK_APP_CAPS_GET_SUPPORTED_DEVICES_TYPES(data) ((((uint8_t)(data)) >> 1) & 0x03)
#define RF4CE_NWK_APP_CAPS_SET_SUPPORTED_DEVICES_TYPES(data, value) ((((uint8_t)(data)) & 0xF9) | (((value) << 1) & 0x06))
#define RF4CE_NWK_APP_CAPS_GET_SUPPORTED_PROFILES(data) ((((uint8_t)(data)) >> 4) & 0x07)
#define RF4CE_NWK_APP_CAPS_SET_SUPPORTED_PROFILES(data, value) ((((uint8_t)(data)) & 0x8F) | (((value) << 4) & 0x70))

/**//**
 * \brief RF4CE NWK Address Mode.
 */
typedef enum _RF4CE_NWK_AddrMode_t
{
    RF4CE_NWK_16BIT = 0,
    RF4CE_NWK_64BIT = 1
} RF4CE_NWK_AddrMode_t;


/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK Application Capabilities field format.
 */
typedef uint8_t RF4CE_NWK_ApplicationCapabilities_t;

/**//**
 * \brief RF4CE NWK The capabilities of the responding node field format.
 */
typedef uint8_t RF4CE_NWK_NodeCapabilities_t;

/**//**
 * \brief RF4CE NWK Elements of the NodeDesc type.
 */
typedef struct _RF4CE_NWK_NodeDesc_t
{
    uint8_t status;                                                 /*!< The status of the discovery request as reported
                                                                         by the responding device. */
    uint8_t logicalChannel;                                         /*!< The logical channel of the responding device. */
    uint16_t panId;                                                 /*!< The PAN identifier of the responding device. */
    uint64_t ieeeAddr;                                              /*!< The IEEE address of the responding device. */
    RF4CE_NWK_NodeCapabilities_t nodeCapabilities;                  /*!< The capabilities of the responding node. */
    uint16_t vendorId;                                              /*!< The vendor identifier of the responding node. */
    uint8_t vendorString[RF4CE_NWK_VENDOR_STRING_LENGTH];           /*!< The vendor string of the responding node. */
    RF4CE_NWK_ApplicationCapabilities_t appCapabilities;            /*!< The application capabilities of the responding
                                                                         node. */
    uint8_t discReqLQI;                                             /*!< The LQI of the discovery request command frame
                                                                         reported by the responding device. */
    uint8_t discRespLQI;                                            /*!< The LQI of the discovery response command frame
                                                                         reported by the responding device. */
    uint8_t *userString;                                            /*!< User string. Variable part of the entry */
    uint8_t *devices;                                               /*!< List of the supported devices. Variable part
                                                                         of the entry */
    uint8_t *profiles;                                              /*!< List of the supported profiles. Variable part
                                                                         of the entry */
} RF4CE_NWK_NodeDesc_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.confirm parameters type.
 * \ingroup RF4CE_NWK_DiscoveryConf
 */
typedef struct _RF4CE_NWK_DiscoveryConfParams_t
{
    uint8_t status;                     /*!< The status of the network discovery attempt. */
    uint8_t numNodes;                   /*!< The number of discovered nodes in the nodeDescList parameter. */
    SYS_DataPointer_t payload;          /*!< The list of node descriptors discovered. */
} RF4CE_NWK_DiscoveryConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.request parameters type.
 * \ingroup RF4CE_NWK_DiscoveryReq
 */
typedef struct _RF4CE_NWK_DiscoveryReqParams_t
{
    uint16_t dstPanId;              /*!< The PAN identifier of the destination device for the discovery. This value can
                                         be set to 0xffff to indicate a wildcard. */
    uint16_t dstNetAddr;            /*!< The address of the destination device for the discovery. This value can be set
                                         to 0xffff to indicate a wildcard. */
    RF4CE_NWK_ApplicationCapabilities_t orgAppCapabilities; /*!< The application capabilities of the node issuing this request. */
    uint8_t searchDevType;          /*!< The device type to discover. This value can be set to 0xff to indicate a
                                         wildcard. */
    uint8_t discProfileIdListSize;  /*!< The number of profile identifiers contained in the discProfileIdList
                                         parameter. */
    uint32_t discDuration;          /*!< The maximum number of MAC symbols to wait for discovery responses to be sent
                                         back from potential target nodes on each channel. This value must be less than
                                         or equal to one third of nwkDiscoveryRepetitionInterval. */
    SYS_DataPointer_t payload;      /*!< The list of device types supported by the node issuing this request. */
                                    /*!< The list of profile identifiers disclosed as supported by the node issuing this
                                         request. */
                                    /*!< The list of profile identifiers against which profile identifiers contained in
                                         received discovery response command frames will be matched for acceptance. */
} RF4CE_NWK_DiscoveryReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.request type declaration.
 * \ingroup RF4CE_NWK_DiscoveryReq
 */
typedef struct _RF4CE_NWK_DiscoveryReqDescr_t RF4CE_NWK_DiscoveryReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY confirmation function type.
 * \ingroup RF4CE_NWK_DiscoveryConf
 */
typedef void (*RF4CE_NWK_DiscoveryConfCallback_t)(RF4CE_NWK_DiscoveryReqDescr_t *req, RF4CE_NWK_DiscoveryConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.request type.
 * \ingroup RF4CE_NWK_DiscoveryReq
 */
typedef struct _RF4CE_NWK_DiscoveryReqDescr_t
{
#ifndef _HOST_
    SYS_QueueElement_t queueElement;            /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_DiscoveryReqParams_t params;   /*!< The filled request. */
    RF4CE_NWK_DiscoveryConfCallback_t callback; /*!< Callback to be called upon operation complete. */
} RF4CE_NWK_DiscoveryReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.indication parameters type.
 * \ingroup RF4CE_NWK_DiscoveryInd
 */
typedef struct _RF4CE_NWK_DiscoveryIndParams_t
{
    uint8_t status;                     /*!< The status of the pairing table. */
    uint64_t srcIeeeAddr;               /*!< The IEEE address of the device requesting the discovery. */
    RF4CE_NWK_NodeCapabilities_t orgNodeCapabilities; /*!< The capabilities of the originator of the discovery request. */
    uint16_t orgVendorId;               /*!< The vendor identifier of the originator of the discovery request. */
    uint8_t orgVendorString[RF4CE_NWK_VENDOR_STRING_LENGTH];    /*!< The vendor string of the originator of the
                                                                     discovery request. */
    RF4CE_NWK_ApplicationCapabilities_t orgAppCapabilities; /*!< The application capabilities of the originator
                                                                          of the discovery request. */
    uint8_t searchDevType;              /*!< The device type being discovered. If this is 0xff, any type is being
                                             requested. */
    uint8_t rxLinkQuality;              /*!< LQI value, as passed via the MAC sub-layer, of the discovery request
                                             command frame. */
    SYS_DataPointer_t payload;          /*!< The user defined identification string of the originator of the discovery
                                             request. */
                                        /*!< The list of device types supported by the originator of the discovery
                                             request. */
                                        /*!< The list of profile identifiers supported by the originator of the
                                             discovery request. */
} RF4CE_NWK_DiscoveryIndParams_t;

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.indication callback function type.
 * \ingroup RF4CE_NWK_DiscoveryInd
 */
typedef void (RF4CE_NWK_DiscoveryIndCallback_t)(RF4CE_NWK_DiscoveryIndParams_t *indication);

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.response parameters type.
 * \ingroup RF4CE_NWK_DiscoveryResp
 */
typedef struct _RF4CE_NWK_DiscoveryRespParams_t
{
    uint8_t status;                     /*!< The status of the discovery request. */
    uint64_t dstIeeeAddr;               /*!< The IEEE address of the device requesting discovery. */
    RF4CE_NWK_ApplicationCapabilities_t recAppCapabilities; /*!< The application capabilities of the node
                                                                          issuing this response. */
    uint8_t discReqLQI;                 /*!< The LQI value from the associated NLME-DISCOVERY.indication request. */
    SYS_DataPointer_t payload;          /*!< The list of device types supported by the node issuing this response. */
                                        /*!< The list of profile identifiers supported by the node issuing this response. */
} RF4CE_NWK_DiscoveryRespParams_t;

/**//**
 * \brief RF4CE NWK NLME-COMM.STATUS.indication parameters type.
 * \ingroup RF4CE_NWK_CommStatusInd
 */
typedef struct _RF4CE_NWK_COMMStatusIndParams_t
{
    uint8_t status;                     /*!< The status of the transmission. */
    uint8_t pairingRef;                 /*!< Reference into the pairing table indicating the recipient node. A value of 0xff
                                             indicates that a discovery response command frame was sent. */
    uint16_t dstPanId;                  /*!< The PAN identifier of the destination device. */
    uint8_t dstAddrMode;                /*!< The address mode of the destination device. */
    MAC_Address_t dstAddr;              /*!< The address of the destination device. */
} RF4CE_NWK_COMMStatusIndParams_t;

/**//**
 * \brief RF4CE NWK NLME-COMM.STATUS.indication callback function type.
 * \ingroup RF4CE_NWK_CommStatusInd
 */
typedef void (RF4CE_NWK_COMMStatusIndCallback_t)(RF4CE_NWK_COMMStatusIndParams_t *indication);

/**//**
 * \brief RF4CE NWK NLME-DISCOVERY.response type.
 * \ingroup RF4CE_NWK_DiscoveryResp
 */
typedef struct _RF4CE_NWK_DiscoveryRespDescr_t
{
    RF4CE_NWK_DiscoveryRespParams_t params;              /*!< The filled response. */
} RF4CE_NWK_DiscoveryRespDescr_t;

/**//**
 * \brief RF4CE NWK NLME-AUTODISCOVERY.confirm parameters type.
 * \ingroup RF4CE_NWK_AutoDiscoveryConf
 */
typedef struct _RF4CE_NWK_AutoDiscoveryConfParams_t
{
    uint8_t status;                     /*!< The status of the auto discovery response mode. */
    uint64_t srcIeeeAddr;               /*!< The IEEE address from which the discovery request was received. */
} RF4CE_NWK_AutoDiscoveryConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-AUTODISCOVERY.request parameters type.
 * \ingroup RF4CE_NWK_AutoDiscoveryReq
 */
typedef struct _RF4CE_NWK_AutoDiscoveryReqParams_t
{
    RF4CE_NWK_ApplicationCapabilities_t recAppCapabilities; /*!< The application capabilities of the node issuing this primitive. */
    uint32_t autoDiscDuration;       /*!< The maximum number of MAC symbols NLME will be in auto discovery response mode. */
    SYS_DataPointer_t payload;       /*!< The list of device types supported by the node issuing this primitive. */
                                     /*!< The list of profile identifiers supported by the node issuing this primitive. */
} RF4CE_NWK_AutoDiscoveryReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-AUTODISCOVERY.request type declaration.
 * \ingroup RF4CE_NWK_AutoDiscoveryReq
 */
typedef struct _RF4CE_NWK_AutoDiscoveryReqDescr_t RF4CE_NWK_AutoDiscoveryReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-AUTODISCOVERY confirmation function type.
 * \ingroup RF4CE_NWK_AutoDiscoveryConf
 */
typedef void (*RF4CE_NWK_AutoDiscoveryConfCallback_t)(RF4CE_NWK_AutoDiscoveryReqDescr_t *req, RF4CE_NWK_AutoDiscoveryConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-AUTODISCOVERY.request type.
 * \ingroup RF4CE_NWK_AutoDiscoveryReq
 */
typedef struct _RF4CE_NWK_AutoDiscoveryReqDescr_t
{
#ifndef _HOST_
    SYS_QueueElement_t queueElement;            /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_AutoDiscoveryReqParams_t params;   /*!< The filled request. */
    RF4CE_NWK_AutoDiscoveryConfCallback_t callback; /*!< Callback to be called upon operation complete. */
} RF4CE_NWK_AutoDiscoveryReqDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to discover neighbor targets.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DiscoveryReq(RF4CE_NWK_DiscoveryReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to respond to the received discovery request data.
 \ingroup RF4CE_NWK_Functions

 \param[in] response - pointer to the structure that contains a pointer to the response structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_DiscoveryResp(RF4CE_NWK_DiscoveryRespDescr_t *response);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to enable automatic discovery requests responding.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_AutoDiscoveryReq(RF4CE_NWK_AutoDiscoveryReqDescr_t *request);

/************************************************************************************//**
 \brief RF4CE NWK NLME-DISCOVERY.indication prototype. For more information see RF4CE_NWK_DiscoveryIndCallback_t type.
 \ingroup RF4CE_NWK_Functions
 ****************************************************************************************/
extern RF4CE_NWK_DiscoveryIndCallback_t RF4CE_NWK_DiscoveryInd;

/************************************************************************************//**
 \brief RF4CE NWK NLME-COM.STATUS.indication prototype. For more information see RF4CE_NWK_COMMStatusIndCallback_t type.
 \ingroup RF4CE_NWK_Functions
 ****************************************************************************************/
extern RF4CE_NWK_COMMStatusIndCallback_t RF4CE_NWK_COMMStatusInd;

#endif /* _RF4CE_NWK_DISCOVERY_H */

/* eof bbRF4CENWKDiscovery.h */