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
 *      This is the header file for the RF4CE ZRC Profile component:
 *      the list of indications that MUST be implemented on the HOST for either Controller or Target
 *      depending on the settings.
 *
*******************************************************************************/

#ifndef _RF4CE_ZRC_EXTERNAL_INDICATIONS_H
#define _RF4CE_ZRC_EXTERNAL_INDICATIONS_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"
#include "bbRF4CENWK.h"
#include "bbRF4CEPM.h"
#include "bbRF4CEZRCControlCommand.h"
#include "bbRF4CEZRC1VendorSpecific.h"
#include "bbRF4CEZRCPollService.h"
#include "bbRF4CEZRCHeartbeat.h"
#include "bbRF4CEZRCClientNotification.h"
#include "bbRF4CEZRCIdentify.h"
#include "bbRF4CEZRCValidation.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Control Command Indication parameters structure.
 * \ingroup RF4CE_ZRC1_ControlCommandInd
 */
typedef struct _RF4CE_ZRC1_ControlCommandIndParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference. */
    uint8_t flags;             /*!< RF4CE_ZRC1_USER_CONTROL_PRESSED or RF4CE_ZRC1_USER_CONTROL_REPEATED or RF4CE_ZRC1_USER_CONTROL_RELEASED. */
    uint8_t commandCode;       /*!< Command code */
    SYS_DataPointer_t payload; /*!< Possible accompanying payload */
} RF4CE_ZRC1_ControlCommandIndParams_t;

#ifndef _PHY_TEST_HOST_INTERFACE_
/**//**
 * \brief RF4CE ZRC 1.1 Vendor Specific Indication parameters structure.
 * \ingroup RF4CE_ZRC1_VendorSpecificInd
 */
typedef RF4CE_ZRC1_VendorSpecificReqParams_t RF4CE_ZRC1_VendorSpecificIndParams_t;

#endif
/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication parameters.
 * \ingroup RF4CE_ZRC2_GetSharedSecretInd
 */
typedef struct _RF4CE_ZRC2_GetSharedSecretIndParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference */
    uint8_t profileId;         /*!< Profile Id */
    uint8_t flags;             /*!< First byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
    uint8_t vendorSpecific;    /*!< Second byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
} RF4CE_ZRC2_GetSharedSecretIndParams_t;

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret response parameters.
 * \ingroup RF4CE_ZRC2_GetSharedSecretResp
 */
typedef struct _RF4CE_ZRC2_GetSharedSecretRespParams_t
{
    Bool8_t status;                                  /*!< True on success, false otherwize */
    uint8_t flags;                                   /*!< First byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
    uint8_t vendorSpecific;                          /*!< Second byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
    uint8_t sharedSecret[RF4CE_SECURITY_KEY_LENGTH]; /*!< Shared Key */
} RF4CE_ZRC2_GetSharedSecretRespParams_t;

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication descriptor declaration.
 * \ingroup RF4CE_ZRC2_GetSharedSecretInd
 */
typedef struct _RF4CE_ZRC2_GetSharedSecretIndDescr_t RF4CE_ZRC2_GetSharedSecretIndDescr_t;

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication callback.
 * \ingroup RF4CE_ZRC2_GetSharedSecretResp
 */
typedef void (*RF4CE_ZRC2_GetSharedSecretIndCallback_t)(RF4CE_ZRC2_GetSharedSecretIndDescr_t *req, RF4CE_ZRC2_GetSharedSecretRespParams_t *conf);

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication descriptor.
 * \ingroup RF4CE_ZRC2_GetSharedSecretInd
 */
struct _RF4CE_ZRC2_GetSharedSecretIndDescr_t
{
    RF4CE_ZRC2_GetSharedSecretIndParams_t params;     /*!< Indication parameters */
    RF4CE_ZRC2_GetSharedSecretIndCallback_t callback; /*!< Indication callback */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Indication to the HOST on ZRC 1.1 Control Command.
 \ingroup RF4CE_ZRC_Functions

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *indication);

/************************************************************************************//**
 \brief Indication to the HOST on ZRC 1.1 Vendor Specific.
 \ingroup RF4CE_ZRC_Functions

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Control Command indication.
 \ingroup RF4CE_ZRC_Functions

 \param[in] indication - pointer to the ZRC 2.0 Control Command indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Check Validation indication.
 \ingroup RF4CE_ZRC_Functions

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Validation begin indication.
 \ingroup RF4CE_ZRC_Functions

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_StartValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC to host Get Shared Secret request.
 \ingroup RF4CE_ZRC_Functions

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_GetSharedSecretInd(RF4CE_ZRC2_GetSharedSecretIndDescr_t *request);

#endif /* _RF4CE_ZRC_EXTERNAL_INDICATIONS_H */

/* EOF bbRF4CEZRCExternalIndications.h */