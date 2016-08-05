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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRC1Attributes.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC 1.1 profile attributes handler.
 *
 * $Revision: 3368 $
 * $Date: 2014-08-21 16:02:35Z $
 *
 ****************************************************************************************/

#ifndef BBRF4CEZRC1ATTRIBUTES_H
#define BBRF4CEZRC1ATTRIBUTES_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEZRCAttributes.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Attribute identifier.
 */
typedef enum _RF4CE_ZRC1_AttributesID_t
{
    RF4CE_ZRC1_APL_KEY_REPEAT_INTERVAL            = 0x80, /*!< ZRC 1. The interval in ms at which user command repeat
                                                               frames will be transmitted. */
    RF4CE_ZRC1_APL_KEY_REPEAT_WAIT_TIME           = 0x81, /*!< ZRC 1. The duration that a recipient of a user control
                                                               repeated command frame waits before terminating a
                                                               repeated operation. */
    RF4CE_ZRC1_APL_KEY_EXCHANGE_TRANSFER_COUNT    = 0x82, /*!< ZRC 1. The value of the KeyExTransferCount parameter
                                                               passed to the pair request primitive during the push
                                                               button pairing procedure. */
    RF4CE_ZRC1_APL_COMMAND_DISCOVERY              = 0x83, /*!< ZRC 1. Command discovery value. */
                                                          /* Overlapped with GDP2.0 POWER_STATUS -wzz 2016.01.27*/
    RF4CE_ZRC1_APL_AUTODISCOVERY_PAIR_TIMEOUT     = 0x84, /*!< Custom attribute. */
                                                          /* Overlapped with RF4CE_ZRC2_POLL_CONSTRAINTS -wzz 2016.01.27*/
//#ifdef _PHY_TEST_HOST_INTERFACE_
    /* A self-defined attribute is created here for customer requirement.
        Max auto-discovery duration is a constant in ZRC1.1 spec.
        ZRC2.0 has aplcBindWindowDuration(RF4CE_GDP_APLC_BIND_WINDOW_DURATION).
        The self-defined attribute GDP_AUTO_DISC_DURATION may represent both of them.
        Besides, we may consider to merge common attributes currently in ZRC1 and ZRC2 systematically to GDP module.
        -wzz 2016.01.27
    */
    GDP_AUTO_DISC_DURATION                        = 0x8C
//#endif
} RF4CE_ZRC1_AttributesID_t;

/**//**
 * \brief RF4CE ZRC Attributes operation status code.
 */
typedef enum _RF4CE_ZRC_AttributeStatus_t
{
    RF4CE_ZRC_ATTR_STATUS_SUCCESS = 0,               /*!< Operation completed successfully. */
    RF4CE_ZRC_ATTR_STATUS_UNSUPPORTED_ATTRIBUTE,     /*!< The supplied attribute ID is invalid. */
    RF4CE_ZRC_ATTR_STATUS_ILLEGAL_REQUEST,           /*!< The request is invalid for that attribute ID. */
    RF4CE_ZRC_ATTR_STATUS_INVALID_ENTRY              /*!< Attribute's index is out of range. For arrays only. */
} RF4CE_ZRC_AttributeStatus_t;

#define RF4CE_ZRC_DEFAULT_HDMI_BANK \
    {0x1f, 0x06, 0, 0xe0, 0xff, 0x03, 0x13, 0, \
    0x0f, 0, 0, 0, 0, 0, 0x1e, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0}

/**//**
 * \brief RF4CE ZRC on board attributes initialization.
 */
#define INIT_ZRC1_ATTRIBUTES \
{ \
    .aplZRC1KeyRepeatInterval = RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL >> 1, \
    .aplZRC1KeyRepeatWaitTime = RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL << 1, \
    .aplZRC1KeyExchangeTransferCount = RF4CE_ZRC1_APLC_MIN_KEY_EXCHANGE_TRANSFER_COUNT, \
    .aplZRC1CommandDiscovery = RF4CE_ZRC_DEFAULT_HDMI_BANK, \
    .aplZRC1AutodiscoveryPairTimeout = RF4CE_ZRC1_MAX_PAIR_INDICATION_WAIT_TIME, \
    .aplGDPAutoDiscoveryDuration = RF4CE_ZRC1_AUTO_DISCOVERY_DURATION, \
}

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Attribute DATA structure.
 */
typedef struct _RF4CE_ZRC1_Attributes_t
{
    uint32_t aplZRC1KeyRepeatInterval;            /*!< ZRC 1. The interval in ms at which user command repeat
                                                       frames will be transmitted. */
    uint32_t aplZRC1KeyRepeatWaitTime;            /*!< ZRC 1. The duration that a recipient of a user control
                                                       repeated command frame waits before terminating a
                                                       repeated operation. */
    uint8_t     aplZRC1KeyExchangeTransferCount;    /*!< ZRC 1. The value of the KeyExTransferCount parameter
                                                       passed to the pair request primitive during the push
                                                       button pairing procedure. */
    uint8_t     aplZRC1CommandDiscovery[32];        /*!< ZRC 1. Command discovery bitmap. */
    uint16_t    aplZRC1AutodiscoveryPairTimeout;
//#ifdef _PHY_TEST_HOST_INTERFACE_
    uint32_t    aplGDPAutoDiscoveryDuration;        /*!< Self-defined attribute for total Binding Timeout. */
//#endif
} RF4CE_ZRC1_Attributes_t;

/**//**
 * \brief RF4CE ZRC 1.1 Attribute DATA union.
 */
typedef union _RF4CE_ZRC1_Attribute_t
{
    uint32_t aplZRC1KeyRepeatInterval;            /*!< ZRC 1. The interval in ms at which user command repeat
                                                       frames will be transmitted. */
    uint32_t aplZRC1KeyRepeatWaitTime;            /*!< ZRC 1. The duration that a recipient of a user control
                                                       repeated command frame waits before terminating a
                                                       repeated operation. */
    uint8_t     aplZRC1KeyExchangeTransferCount;    /*!< ZRC 1. The value of the KeyExTransferCount parameter
                                                       passed to the pair request primitive during the push
                                                       button pairing procedure. */
    uint8_t     aplZRC1CommandDiscovery[32];        /*!< ZRC 1. Command discovery bitmap. */
    uint16_t    aplZRC1AutodiscoveryPairTimeout;
#ifdef _PHY_TEST_HOST_INTERFACE_
    uint32_t    aplGDPAutoDiscoveryDuration;        /*!< Self-defined attribute for total Binding Timeout. */
#endif
} RF4CE_ZRC1_Attribute_t;

/**//**
 * \brief RF4CE ZRC 1.1 Get Attribute Request parameters.
 */
typedef struct _RF4CE_ZRC1_GetAttributeReqParams_t
{
    uint8_t attributeId;
} RF4CE_ZRC1_GetAttributeReqParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Get Attribute Request confirm.
 */
typedef struct _RF4CE_ZRC1_GetAttributeConfParams_t
{
    uint8_t status;
    RF4CE_ZRC1_Attribute_t data;
} RF4CE_ZRC1_GetAttributeConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Set Attribute Request parameters.
 */
typedef struct _RF4CE_ZRC1_SetAttributeReqParams_t
{
    uint8_t attributeId;
    RF4CE_ZRC1_Attribute_t data;
} RF4CE_ZRC1_SetAttributeReqParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Set Attribute Request confirm.
 */
typedef struct _RF4CE_ZRC1_SetAttributeConfParams_t
{
    uint8_t status;
} RF4CE_ZRC1_SetAttributeConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Get request descriptor declaration.
 */
typedef struct _RF4CE_ZRC1_GetAttributeDescr_t RF4CE_ZRC1_GetAttributeDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Get request callback.
 */
typedef void (*RF4CE_ZRC1_GetAttributeCallback_t)(RF4CE_ZRC1_GetAttributeDescr_t *req, RF4CE_ZRC1_GetAttributeConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Set request descriptor declaration.
 */
typedef struct _RF4CE_ZRC1_SetAttributeDescr_t RF4CE_ZRC1_SetAttributeDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Set request callback.
 */
typedef void (*RF4CE_ZRC1_SetAttributeCallback_t)(RF4CE_ZRC1_SetAttributeDescr_t *req, RF4CE_ZRC1_SetAttributeConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Get request descriptor.
 */
struct _RF4CE_ZRC1_GetAttributeDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ZRC1_GetAttributeReqParams_t params;
    RF4CE_ZRC1_GetAttributeCallback_t callback;
};

/**//**
 * \brief RF4CE ZRC 1.1 Attribute Set request descriptor.
 */
struct _RF4CE_ZRC1_SetAttributeDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;
#else
	void *context;
#endif /* _HOST_ */
    RF4CE_ZRC1_SetAttributeReqParams_t params;
    RF4CE_ZRC1_SetAttributeCallback_t callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts asynchronous ZRC 1.1 Get Attributes Request.

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_GetAttributesReq(RF4CE_ZRC1_GetAttributeDescr_t *request);

/************************************************************************************//**
 \brief Starts asynchronous ZRC 1.1 Set Attributes Request.

 \param[in] request - pointer to the request descriptor.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_SetAttributesReq(RF4CE_ZRC1_SetAttributeDescr_t *request);

#endif // BBRF4CEZRC1ATTRIBUTES_H