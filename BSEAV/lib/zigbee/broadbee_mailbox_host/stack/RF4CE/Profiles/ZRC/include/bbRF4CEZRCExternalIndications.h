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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCExternalIndications.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC Profile component:
 *   the list of indications that MUST be implemented on the HOST for either Controller or Target
 *   depending on the settings.
 *
 * $Revision: 3515 $
 * $Date: 2014-09-10 11:15:21Z $
 *
 ****************************************************************************************/
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
 */
typedef RF4CE_ZRC1_VendorSpecificReqParams_t RF4CE_ZRC1_VendorSpecificIndParams_t;

#endif
/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication parameters.
 */
typedef struct _RF4CE_ZRC2_GetSharedSecretIndParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference */
    uint8_t profileId;         /*!< Profile Id */
    uint8_t flags;             /*!< First byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
    uint8_t vendorSpecific;    /*!< Second byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
} RF4CE_ZRC2_GetSharedSecretIndParams_t;

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret confirmation parameters.
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
 */
typedef struct _RF4CE_ZRC2_GetSharedSecretIndDescr_t RF4CE_ZRC2_GetSharedSecretIndDescr_t;

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication callback.
 */
typedef void (*RF4CE_ZRC2_GetSharedSecretIndCallback_t)(RF4CE_ZRC2_GetSharedSecretIndDescr_t *req, RF4CE_ZRC2_GetSharedSecretRespParams_t *conf);

/**//**
 * \brief RF4CE ZRC2 Get Shared Secret Indication descriptor.
 */
struct _RF4CE_ZRC2_GetSharedSecretIndDescr_t
{
    RF4CE_ZRC2_GetSharedSecretIndParams_t params;
    RF4CE_ZRC2_GetSharedSecretIndCallback_t callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Indication to the HOST on ZRC 1.1 Control Command.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *indication);

/************************************************************************************//**
 \brief Indication to the HOST on ZRC 1.1 Vendor Specific.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_VendorSpecificInd(RF4CE_ZRC1_VendorSpecificIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Control Command indication.

 \param[in] indication - pointer to the ZRC 2.0 Control Command indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *indication);

// TODO: Remove. This function is moved into the bbRF4CEZRCClientNotification.h
///************************************************************************************//**
// \brief ZRC 2.0 Client notification request indication.
//
// \param[in] indication - pointer to the indication structure.
// \return Nothing.
// ****************************************************************************************/
//void RF4CE_ZRC2_ClientNotificationInd(RF4CE_ZRC2_ClientNotificationIndParams_t *indication);

// TODO: Decide if need to keep or delete.
/************************************************************************************//**
 \brief ZRC 2.0 Heartbeat request indication.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
// void RF4CE_ZRC2_HeartbeatInd(RF4CE_ZRC2_HeartbeatIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Check Validation indication.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_CheckValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC 2.0 Validation begin indication.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_StartValidationInd(RF4CE_ZRC2_CheckValidationIndParams_t *indication);

/************************************************************************************//**
 \brief ZRC to host Get Shared Secret request.

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_GetSharedSecretInd(RF4CE_ZRC2_GetSharedSecretIndDescr_t *request);

#endif /* RF4CE_TARGET */
