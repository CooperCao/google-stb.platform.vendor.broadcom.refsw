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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCBind.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC profile Binding functionality.
 *
 * $Revision: 2512 $
 * $Date: 2014-05-26 11:58:18Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_BIND_H
#define _RF4CE_ZRC_BIND_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind class extractor.
 */
#define RF4CE_ZRC2_GET_CLASS(descr) ((descr) & 0x0F)
#define RF4CE_ZRC2_SET_CLASS(descr, class) (((class) & 0x0F) | ((descr) & 0xF0))
#define RF4CE_ZRC2_GET_DUPLICATE_CLASS_ID(descr) (((descr) >> 4) & 0x03)
#define RF4CE_ZRC2_SET_DUPLICATE_CLASS_ID(descr, dupClassNum) ((((dupClassNum) << 4) & 0x30) | ((descr) & 0xCF))

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind duplicate class IDs.
 */
typedef enum _RF4CE_ZRC2_BindClassId_t
{
    RF4CE_ZRC2_BIND_CLASS_PRECOMMOSSIONED = 0x00,
    RF4CE_ZRC2_BIND_CLASS_BUTTON_PRESS_INDICATION = 0x01,
    RF4CE_ZRC2_BIND_CLASS_RESERVED1 = 0x02,
    RF4CE_ZRC2_BIND_CLASS_RESERVED2 = 0x03,
    RF4CE_ZRC2_BIND_CLASS_DISCOVERABLE_ONLY = 0x0F,
    RF4CE_ZRC2_BIND_CLASS_MAX_VALUE = RF4CE_ZRC2_BIND_CLASS_DISCOVERABLE_ONLY,
} RF4CE_ZRC2_BindClassId_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind duplicate class IDs.
 */
typedef enum _RF4CE_ZRC2_BindDuplicateClassNumberHandling_t
{
    RF4CE_ZRC2_USE_NODE_DESCRIPTOR_AS_IS = 0,
    RF4CE_ZRC2_RECLASSIFY_NODE_DESCRIPTOR = 1,
    RF4CE_ZRC2_ABORT_BINDING = 2,
    RF4CE_ZRC2_RESERVED = 3
} RF4CE_ZRC2_BindDuplicateClassNumberHandling_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind class numbers.
 */
#define RF4CE_ZRC2_MIN_CLASS 2
#define RF4CE_ZRC2_MAX_CLASS 0xE
#define RF4CE_ZRC2_DISCOVERABLE_ONLY_CLASS 0xF

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_BindConfParams_t
{
    uint8_t status;     /*!< The status of binding. See RF4CE_ZRC2_BindStatus_t. */
    uint8_t pairingRef; /*!< The pairing reference on successful binding. */
    uint8_t profileId;  /*!< Actual profile the node is bound to: RF4CE_ZRC_GDP1_COMPLIANT or RF4CE_ZRC_GDP2_COMPLIANT */
} RF4CE_ZRC2_BindConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind notification parameters.
 */
typedef struct _RF4CE_ZRC2_BindingFinishedNtfyIndParams_t
{
    uint8_t status;     /*!< The status of binding. See RF4CE_ZRC2_BindStatus_t. */
    uint8_t pairingRef; /*!< The pairing reference on successful binding. */
    uint8_t profileId;  /*!< Actual profile the node is bound to: RF4CE_ZRC_GDP1_COMPLIANT_PROTOCOL_ID or RF4CE_ZRC_GDP2_COMPLIANT_PROTOCOL_ID */
} RF4CE_ZRC2_BindingFinishedNtfyIndParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind request declaration.
 */
typedef struct _RF4CE_ZRC2_BindReqDescr_t RF4CE_ZRC2_BindReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind callback.
 */
typedef void (*RF4CE_ZRC2_BindCallback_t)(RF4CE_ZRC2_BindReqDescr_t *req, RF4CE_ZRC2_BindConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind request.
 */
typedef struct _RF4CE_ZRC2_BindReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;  /*!< Service field */
#endif /* _HOST_ */
    RF4CE_ZRC2_BindCallback_t callback; /*!< Callback on request completion. */
} RF4CE_ZRC2_BindReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Proxy Bind request parameters.
 */
typedef struct _RF4CE_ZRC2_ProxyBindReqParams_t
{
    uint64_t address;   /*!< The known remote host's address. */
} RF4CE_ZRC2_ProxyBindReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Proxy Bind request declaration.
 */
typedef struct _RF4CE_ZRC2_ProxyBindReqDescr_t RF4CE_ZRC2_ProxyBindReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Proxy Bind callback.
 */
typedef void (*RF4CE_ZRC2_ProxyBindCallback_t)(RF4CE_ZRC2_ProxyBindReqDescr_t *req, RF4CE_ZRC2_BindConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Bind request.
 */
struct _RF4CE_ZRC2_ProxyBindReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;      /*!< Service field */
#endif /* _HOST_ */
    RF4CE_ZRC2_ProxyBindReqParams_t params;  /*!< Request parameters. */
    RF4CE_ZRC2_ProxyBindCallback_t callback; /*!< Callback on request completion. */
};

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Binding Commmon confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_BindingConfParams_t
{
    uint8_t status; /*!< One of the RF4CE_ZRC2GDP2_Status_t values */
} RF4CE_ZRC2_BindingConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Button Enabled Binding request parameters.
 */
typedef struct _RF4CE_ZRC2_ButtonBindingReqParams_t
{
    uint32_t autoDiscDuration;       /*!< The maximum number of MAC symbols NLME will be in auto discovery response mode. */
} RF4CE_ZRC2_ButtonBindingReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Button Enabled Binding request descriptor declaration.
 */
typedef struct _RF4CE_ZRC2_ButtonBindingReqDescr_t RF4CE_ZRC2_ButtonBindingReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Button Enabled Binding callback.
 */
typedef void (*RF4CE_ZRC2_ButtonBindingCallback_t)(RF4CE_ZRC2_ButtonBindingReqDescr_t *req, RF4CE_ZRC2_BindingConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Button Enabled Binding request descriptor.
 */
struct _RF4CE_ZRC2_ButtonBindingReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;          /*!< Service field */
#endif /* _HOST_ */
    RF4CE_ZRC2_ButtonBindingReqParams_t params;  /*!< Request parameters. */
    RF4CE_ZRC2_ButtonBindingCallback_t callback; /*!< Pointer to the callback  */
};

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Binding request descriptor declaration.
 */
typedef struct _RF4CE_ZRC2_BindingReqDescr_t RF4CE_ZRC2_BindingReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Binding callback.
 */
typedef void (*RF4CE_ZRC2_BindingCallback_t)(RF4CE_ZRC2_BindingReqDescr_t *req, RF4CE_ZRC2_BindingConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Binding request descriptor.
 */
struct _RF4CE_ZRC2_BindingReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;      /*!< Service field */
#endif /* _HOST_ */
    RF4CE_ZRC2_BindingCallback_t callback;   /*!< Pointer to the callback  */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts asynchronous Binding Procedure.

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_BindReq(RF4CE_ZRC2_BindReqDescr_t *request);

/************************************************************************************//**
 \brief Starts asynchronous Proxy Binding Procedure.

 \param[in] request - pointer to the request.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_ProxyBindReq(RF4CE_ZRC2_ProxyBindReqDescr_t *request);

/************************************************************************************//**
 \brief Enables Binding Procedure on the Target.
 ****************************************************************************************/
void RF4CE_ZRC2_EnableBindingReq(RF4CE_ZRC2_BindingReqDescr_t *request);

/************************************************************************************//**
 \brief Disables Binding Procedure on the Target.
 ****************************************************************************************/
void RF4CE_ZRC2_DisableBindingReq(RF4CE_ZRC2_BindingReqDescr_t *request);

/************************************************************************************//**
    \brief Sets Push Button Stimulus flag on the Target.
    \param[in] request - pointer to the request.
 ****************************************************************************************/
void RF4CE_ZRC2_SetPushButtonStimulusReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request);

/************************************************************************************//**
    \brief Clears Push Button Stimulus flag on the Target.
    \param[in] request - pointer to the request.
 ****************************************************************************************/
void RF4CE_ZRC2_ClearPushButtonStimulusReq(RF4CE_ZRC2_ButtonBindingReqDescr_t *request);

/************************************************************************************//**
    \brief Pair notification callback.
    \param[in] indication - pairing parameters.
 ****************************************************************************************/
void RF4CE_ZRC2_PairNtfyInd(RF4CE_PairingIndParams_t *indication);

/************************************************************************************//**
    \brief Binding notification callback.
    \param[in] indication - binding procedure parameters.
 ****************************************************************************************/
void RF4CE_ZRC2_BindingFinishedNtfyInd(RF4CE_ZRC2_BindingFinishedNtfyIndParams_t *indication);

#endif /* _RF4CE_ZRC_BIND_H */
