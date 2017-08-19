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
 *      This is the header file for the RF4CE MSO Profile Attributes component.
 *
*******************************************************************************/

#ifndef _RF4CE_MSO_PROFILE_ATTRIBUTES_H
#define _RF4CE_MSO_PROFILE_ATTRIBUTES_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CENWKConstants.h"
#include "bbSysQueue.h"
#include "bbRF4CENWKRequestService.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO profile Attributes enumeration.
 * \ingroup RF4CE_MSO_ProfileAttributeMisc
 */
typedef enum _RF4CE_MSO_ProfileAttributeID_t
{
    RF4CE_MSO_APL_KEY_REPEAT_INTERVAL = 0,      /*!< Controller. The interval in ms at which user command repeat frames
                                                     will be transmitted for repeatable keys. */
    RF4CE_MSO_APL_RESPONSE_WAIT_TIME,           /*!< Controller. The maximum time in symbols that a device SHALL wait
                                                     (after the aplcResponseIdleWaitTime expired) to receive a response
                                                     command frame following a request command frame. */
    RF4CE_MSO_APL_MAX_PAIRING_CANDIDATES,       /*!< Controller. The maximum number of pairing candidates selected from
                                                     the NLME-DISCOVERY.response node descriptor list. */
    RF4CE_MSO_APL_LINK_LOST_WAIT_TIME,          /*!< Controller. The maximum time in ms that a device can stay in the
                                                     validation procedure without receiving the responses corresponding
                                                     to its requests. [Can be updated by RIB procedure at the start of
                                                     the validation procedure.] */
    RF4CE_MSO_APL_AUTO_CHECK_VALIDATION_PERIOD, /*!< Controller. The time period in ms between the regular check
                                                     validation requests that a controller transmits in the validation
                                                     procedure. [Can be updated by RIB procedure at the start of the
                                                     validation.] */
    RF4CE_MSO_APL_KEY_EXCHANGE_TRANSFER_COUNT,  /*!< Controller. The value of the KeyExTransferCount parameter passed to
                                                     the pair request primitive during the temporary pairing procedure. */

    FR4CE_MSO_MAX_CONTROLLER_ATTRIBUTE,

    RF4CE_MSO_APL_KEY_REPEAT_WAIT_TIME = 0x10,  /*!< Target. The duration in ms that a recipient of a user control
                                                     repeated command frame waits before terminating a repeated
                                                     operation. */
    RF4CE_MSO_APL_VALIDATION_WAIT_TIME,         /*!< Target. The maximum time in ms that a device can stay in the
                                                     validation procedure. */
    RF4CE_MSO_APL_VALIDATION_INITIAL_WATCHDOG_TIME, /*!< Target. The maximum time in ms that a device can stay in the
                                                         validation procedure, without receiving a first validation
                                                         watchdog kick. */

    RF4CE_MSO_APL_SHORT_RF_PERIOD,

    FR4CE_MSO_MAX_TARGET_ATTRIBUTE,

    RF4CE_MSO_APL_USER_STRING = 0x20            /*!< Target and Controller. The user-defined character
                                                     string to carry application-related information. */
} RF4CE_MSO_ProfileAttributeID_t;

/**//**
 * \brief RF4CE MSO profile Attributes GET/SET status enumeration.
 * \ingroup RF4CE_MSO_ProfileAttributeMisc
 */
typedef enum _RF4CE_MSO_ProfileAttributeStatus_t
{
    RF4CE_MSO_PA_SUCCESS = 0,
    RF4CE_MSO_PA_UNSUPPORTED_ID,
    RF4CE_MSO_PA_INVALID
} RF4CE_MSO_ProfileAttributeStatus_t;

/**//**
 * \brief RF4CE MSO profile Attributes initialization MACRO.
 */
#ifdef RF4CE_CONTROLLER
#define RF4CE_MSO_INIT_PROFILE_ATTRIBUTES \
    .aplKeyRepeatInterval = MSO_APLC_MAX_KEY_REPEAT_INTERVAL >> 1, \
    .aplResponseWaitTime = 0x186A, \
    .aplMaxPairingCandidates = RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES, \
    .aplLinkLostWaitTime = 0x2710, \
    .aplAutoCheckValidationPeriod = 0x01F4, \
    .aplKeyExchangeTransferCount = 0x04, \
    .aplUserString = RF4CE_MSO_CLIENT_USER_STRING,
#else /* RF4CE_CONTROLLER */
#define RF4CE_MSO_INIT_PROFILE_ATTRIBUTES \
    .aplResponseWaitTime = 0x186A, \
    .aplKeyRepeatWaitTime = MSO_APLC_MAX_KEY_REPEAT_INTERVAL, \
    .aplValidationWaitTime = 0x0, \
    .aplValidationInitialWatchdogTime = 0x1F40, \
    .aplMaxPairingCandidates = RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES, \
    .aplLinkLostWaitTime = 0x2710, \
    .aplAutoCheckValidationPeriod = 0x01F4, \
    .aplKeyExchangeTransferCount = 0x04, \
    .aplUserString = RF4CE_MSO_TARGET_USER_STRING, \
    .aplShortRFPeriod = 100000
#endif /* RF4CE_CONTROLLER */

/************************* TYPES *******************************************************/
/**//**
 * \brief MSO Profile Attributes.
 * \ingroup RF4CE_MSO_ProfileAttributeMisc
 */
typedef struct _RF4CE_MSO_ProfileAttributes_t
{
    uint32_t aplResponseWaitTime;               /*!< Controller. The maximum time in symbols that a device SHALL wait
                                                     (after the aplcResponseIdleWaitTime expired) to receive a response
                                                     command frame following a request command frame. */
    uint8_t aplMaxPairingCandidates;            /*!< Controller. The maximum number of pairing candidates selected from
                                                     the NLME-DISCOVERY.response node descriptor list. */
    uint16_t aplLinkLostWaitTime;               /*!< Controller. The maximum time in ms that a device can stay in the
                                                     validation procedure without receiving the responses corresponding
                                                     to its requests. [Can be updated by RIB procedure at the start of
                                                     the validation procedure.] */
    uint16_t aplAutoCheckValidationPeriod;      /*!< Controller. The time period in ms between the regular check
                                                     validation requests that a controller transmits in the validation
                                                     procedure. [Can be updated by RIB procedure at the start of the
                                                     validation.] */
    uint8_t aplKeyExchangeTransferCount;        /*!< Controller. The value of the KeyExTransferCount parameter passed to
                                                     the pair request primitive during the temporary pairing procedure. */
    uint16_t aplKeyRepeatInterval;              /*!< Controller. The interval in ms at which user command repeat frames
                                                     will be transmitted for repeatable keys. */
#ifdef RF4CE_TARGET
    uint16_t aplKeyRepeatWaitTime;              /*!< Target. The duration in ms that a recipient of a user control
                                                     repeated command frame waits before terminating a repeated
                                                     operation. */
    uint16_t aplValidationWaitTime;             /*!< Target. The maximum time in ms that a device can stay in the
                                                     validation procedure. */
    uint16_t aplValidationInitialWatchdogTime;  /*!< Target. The maximum time in ms that a device can stay in the
                                                     validation procedure, without receiving a first validation
                                                     watchdog kick. */
    uint32_t aplShortRFPeriod;
#endif /* RF4CE_TARGET */
    uint8_t aplUserString[RF4CE_NWK_USER_STRING_LENGTH]; /*!< Target and Controller. The user-defined character
                                                              string to carry application-related information. */
} RF4CE_MSO_ProfileAttributes_t;

/**//**
 * \brief MSO Profile Attributes union for all of the attributes.
 * \ingroup RF4CE_MSO_ProfileAttributeMisc
 */
typedef union _RF4CE_MSO_ProfileAttributesUnion_t
{
    uint16_t aplKeyRepeatWaitTime;              /*!< Value is KeyRepeatWaitTime */
    uint16_t aplValidationWaitTime;             /*!< Value is ValidationWaitTime */
    uint16_t aplValidationInitialWatchdogTime;  /*!< Value is ValidationInitialWatchdogTime */
    uint16_t aplKeyRepeatInterval;              /*!< Value is KeyRepeatInterval */
    uint32_t aplResponseWaitTime;               /*!< Value is ResponseWaitTime */
    uint8_t aplMaxPairingCandidates;            /*!< Value is MaxPairingCandidates */
    uint16_t aplLinkLostWaitTime;               /*!< Value is LinkLostWaitTime */
    uint16_t aplAutoCheckValidationPeriod;      /*!< Value is AutoCheckValidationPeriod */
    uint8_t aplKeyExchangeTransferCount;        /*!< Value is KeyExchangeTransferCount */
    uint8_t aplUserString[RF4CE_NWK_USER_STRING_LENGTH]; /*!< Value is UserString */
    uint32_t aplShortRFPeriod;                  /*!< Value is ShortRFPeriod */
} RF4CE_MSO_ProfileAttributesUnion_t;

/**//**
 * \brief MSO Profile Attribute GET request parameters.
 * \ingroup RF4CE_MSO_GetProfileAttributeReq
 */
typedef struct _RF4CE_MSO_GetProfileAttributeReqParams_t
{
    uint8_t id;        /*!< One of the RF4CE_MSO_ProfileAttributeID_t values */
} RF4CE_MSO_GetProfileAttributeReqParams_t;

/**//**
 * \brief MSO Profile Attribute GET request confirmation parameters.
 * \ingroup RF4CE_MSO_GetProfileAttributeConf
 */
typedef struct _RF4CE_MSO_GetProfileAttributeConfParams_t
{
    uint8_t status;    /*!< One of the RF4CE_MSO_ProfileAttributeStatus_t values */
    uint8_t id;        /*!< One of the RF4CE_MSO_ProfileAttributeID_t values */
    RF4CE_MSO_ProfileAttributesUnion_t attribute; /*!< The resulting value */
} RF4CE_MSO_GetProfileAttributeConfParams_t;

/**//**
 * \brief MSO Profile Attribute GET request descriptor declaration.
 * \ingroup RF4CE_MSO_GetProfileAttributeReq
 */
typedef struct _RF4CE_MSO_GetProfileAttributeReqDescr_t RF4CE_MSO_GetProfileAttributeReqDescr_t;

/**//**
 * \brief MSO Profile Attribute GET request callback declaration.
 * \ingroup RF4CE_MSO_GetProfileAttributeConf
 */
typedef void (*RF4CE_MSO_GetProfileAttributeCallback_t)(RF4CE_MSO_GetProfileAttributeReqDescr_t *req, RF4CE_MSO_GetProfileAttributeConfParams_t *conf);

/**//**
 * \brief MSO Profile Attribute GET request descriptor.
 * \ingroup RF4CE_MSO_GetProfileAttributeReq
 */
struct _RF4CE_MSO_GetProfileAttributeReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;               /*!< Service field */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_MSO_GetProfileAttributeReqParams_t params;  /*!< Request parameters */
    RF4CE_MSO_GetProfileAttributeCallback_t callback; /*!< Request callback */
};

/**//**
 * \brief MSO Profile Attribute SET request parameters.
 * \ingroup RF4CE_MSO_SetProfileAttributeReq
 */
typedef struct _RF4CE_MSO_SetProfileAttributeReqParams_t
{
    uint8_t id;        /*!< One of the RF4CE_MSO_ProfileAttributeID_t values */
    RF4CE_MSO_ProfileAttributesUnion_t attribute; /*!< The value for that attribute */
} RF4CE_MSO_SetProfileAttributeReqParams_t;

/**//**
 * \brief MSO Profile Attribute SET request confirmation parameters.
 * \ingroup RF4CE_MSO_SetProfileAttributeConf
 */
typedef struct _RF4CE_MSO_SetProfileAttributeConfParams_t
{
    uint8_t status;    /*!< One of the RF4CE_MSO_ProfileAttributeStatus_t values */
    uint8_t id;        /*!< One of the RF4CE_MSO_ProfileAttributeID_t values */
} RF4CE_MSO_SetProfileAttributeConfParams_t;

/**//**
 * \brief MSO Profile Attribute SET request descriptor declaration.
 * \ingroup RF4CE_MSO_SetProfileAttributeReq
 */
typedef struct _RF4CE_MSO_SetProfileAttributeReqDescr_t RF4CE_MSO_SetProfileAttributeReqDescr_t;

/**//**
 * \brief MSO Profile Attribute SET request callback declaration.
 * \ingroup RF4CE_MSO_SetProfileAttributeConf
 */
typedef void (*RF4CE_MSO_SetProfileAttributeCallback_t)(RF4CE_MSO_SetProfileAttributeReqDescr_t *req, RF4CE_MSO_SetProfileAttributeConfParams_t *conf);

/**//**
 * \brief MSO Profile Attribute SET request descriptor.
 * \ingroup RF4CE_MSO_SetProfileAttributeReq
 */
struct _RF4CE_MSO_SetProfileAttributeReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;               /*!< Service field */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_MSO_SetProfileAttributeReqParams_t params;  /*!< Request parameters */
    RF4CE_MSO_SetProfileAttributeCallback_t callback; /*!< Request callback */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to GET the specified Profile Attribute.
 \ingroup RF4CE_MSO_Functions

 \param[in] request - pointer to the structure that contains the request structure.
 \return Nothing
 ****************************************************************************************/
void RF4CE_MSO_GetProfileAttributeReq(RF4CE_MSO_GetProfileAttributeReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to SET the specified Profile Attribute.
 \ingroup RF4CE_MSO_Functions

 \param[in] request - pointer to the structure that contains the request structure.
 \return Nothing
 ****************************************************************************************/
void RF4CE_MSO_SetProfileAttributeReq(RF4CE_MSO_SetProfileAttributeReqDescr_t *request);

#endif /* _RF4CE_MSO_PROFILE_ATTRIBUTES_H */

/* eof bbRF4CEMSOProfileAttributes.h */