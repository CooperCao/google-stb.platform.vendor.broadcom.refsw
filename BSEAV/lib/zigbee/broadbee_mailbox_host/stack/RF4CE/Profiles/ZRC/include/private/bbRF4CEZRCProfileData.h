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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/private/bbRF4CEZRCProfileData.h $
 *
 * DESCRIPTION:
 *   This is the private header file for the RF4CE ZRC profile data handler.
 *
 * $Revision: 3515 $
 * $Date: 2014-09-10 11:15:21Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_PROFILE_DATA_H
#define _RF4CE_ZRC_PROFILE_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysBasics.h"
#include "bbRF4CEZRC1CommandDiscovery.h"
#include "bbRF4CEZRCControlCommand.h"
#include "bbRF4CEZRC1VendorSpecific.h"
#include "bbRF4CEZRCAttributes.h"
#include "bbRF4CEZRCKeyExchange.h"
#include "bbRF4CEZRCExternalIndications.h"
#include "bbSysNvmManager.h"
#include "private/bbRF4CEZRCPrivateActionMap.h"
#include "private/bbRF4CEZRCPrivateControlCommand.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC Control Command Flags values.
 */
#define RF4CE_ZRC2_FLAG 0
#define RF4CE_ZRC1_FLAG 1
#define RF4CE_ZRC_PRESSED_FLAG 0
#define RF4CE_ZRC_RELEASED_FLAG 2
#define RF4CE_ZRC1_RELEASED_NO_CALLBACK_FLAG 4
#define RF4CE_ZRC_DATA_RECEIVED_FLAG 8
#define RF4CE_ZRC_IN_CC_REQUEST_FLAG 0x10
#define RF4CE_ZRC_USED_GETSET_FLAG 0x20
#define RF4CE_ZRC_LEAVE_RECEIVER_ON_FLAG 0x40
#define RF4CE_ZRC_READ_WRITE_USED_FLAG 0x80
#define RF4CE_ZRC_IN_BLACKOUT_FLAG 0x0100
#define RF4CE_ZRC_GETTING_DATA_FLAG 0x0200
#define RF4CE_ZRC_IN_BLACKOUT_GDP_TO_ZRC_FLAG 0x0400
#define RF4CE_ZRC_IN_TIMEOUT_CONFIG_COMPLETE_FLAG 0x0800
#define RF4CE_ZRC_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG 0x1000
#define RF4CE_ZRC_PROXY_BIND_FLAG 0x2000
#define RF4CE_ZRC_IN_BLACKOUT_VALIDATION_FLAG 0x4000
#define RF4CE_ZRC_IN_VALIDATION_REP_FLAG 0x8000
#define RF4CE_ZRC_LOST_LINK_FLAG 0x010000
#define RF4CE_ZRC_IN_VALIDATION_FLAG 0x020000
#define RF4CE_ZRC_RELEASE_FAIL_FLAG 0x040000

#define RF4CE_ZRC_SET_ZRC1_CC_FLAG(v) ((v) |= RF4CE_ZRC1_FLAG)
#define RF4CE_ZRC_SET_ZRC2_CC_FLAG(v) ((v) &= ~RF4CE_ZRC1_FLAG)
#define RF4CE_ZRC_SET_PRESSED_CC_FLAG(v) ((v) &= ~RF4CE_ZRC_RELEASED_FLAG)
#define RF4CE_ZRC_SET_RELEASED_CC_FLAG(v) ((v) |= RF4CE_ZRC_RELEASED_FLAG)
#define RF4CE_ZRC_SET_RELEASED_NO_CALLBACK_CC_FLAG(v) ((v) |= RF4CE_ZRC1_RELEASED_NO_CALLBACK_FLAG)
#define RF4CE_ZRC_CLEAR_RELEASED_NO_CALLBACK_CC_FLAG(v) ((v) &= ~RF4CE_ZRC1_RELEASED_NO_CALLBACK_FLAG)
#define RF4CE_ZRC_SET_DATA_RECEIVED_CC_FLAG(v) ((v) |= RF4CE_ZRC_DATA_RECEIVED_FLAG)
#define RF4CE_ZRC_CLEAR_DATA_RECEIVED_CC_FLAG(v) ((v) &= ~RF4CE_ZRC_DATA_RECEIVED_FLAG)
#define RF4CE_ZRC_SET_IN_REQUEST_CC_FLAG(v) ((v) |= RF4CE_ZRC_IN_CC_REQUEST_FLAG)
#define RF4CE_ZRC_CLEAR_IN_REQUEST_CC_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_CC_REQUEST_FLAG)
#define RF4CE_ZRC_SET_USED_GETSET_FLAG(v) ((v) |= RF4CE_ZRC_USED_GETSET_FLAG)
#define RF4CE_ZRC_CLEAR_USED_GETSET_FLAG(v) ((v) &= ~RF4CE_ZRC_USED_GETSET_FLAG)
#define RF4CE_ZRC_SET_LEAVE_RECEIVER_ON_FLAG(v) ((v) |= RF4CE_ZRC_LEAVE_RECEIVER_ON_FLAG)
#define RF4CE_ZRC_CLEAR_LEAVE_RECEIVER_ON_FLAG(v) ((v) &= ~RF4CE_ZRC_LEAVE_RECEIVER_ON_FLAG)
#define RF4CE_ZRC_SET_READ_WRITE_USED_FLAG(v) ((v) |= RF4CE_ZRC_READ_WRITE_USED_FLAG)
#define RF4CE_ZRC_CLEAR_READ_WRITE_USED_FLAG(v) ((v) &= ~RF4CE_ZRC_READ_WRITE_USED_FLAG)
#define RF4CE_ZRC_SET_IN_BLACKOUT_FLAG(v) ((v) |= RF4CE_ZRC_IN_BLACKOUT_FLAG)
#define RF4CE_ZRC_CLEAR_IN_BLACKOUT_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_BLACKOUT_FLAG)
#define RF4CE_ZRC_SET_GETTING_DATA_FLAG(v) ((v) |= RF4CE_ZRC_GETTING_DATA_FLAG)
#define RF4CE_ZRC_CLEAR_GETTING_DATA_FLAG(v) ((v) &= ~RF4CE_ZRC_GETTING_DATA_FLAG)
#define RF4CE_ZRC_SET_IN_BLACKOUT_GDP_TO_ZRC_FLAG(v) ((v) |= RF4CE_ZRC_IN_BLACKOUT_GDP_TO_ZRC_FLAG)
#define RF4CE_ZRC_CLEAR_IN_BLACKOUT_GDP_TO_ZRC_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_BLACKOUT_GDP_TO_ZRC_FLAG)
#define RF4CE_ZRC_SET_IN_TIMEOUT_CONFIG_COMPLETE_FLAG(v) ((v) |= RF4CE_ZRC_IN_TIMEOUT_CONFIG_COMPLETE_FLAG)
#define RF4CE_ZRC_CLEAR_IN_TIMEOUT_CONFIG_COMPLETE_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_TIMEOUT_CONFIG_COMPLETE_FLAG)
#define RF4CE_ZRC_SET_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG(v) ((v) |= RF4CE_ZRC_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG)
#define RF4CE_ZRC_CLEAR_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG)
#define RF4CE_ZRC_SET_PROXY_BIND_FLAG(v) ((v) |= RF4CE_ZRC_PROXY_BIND_FLAG)
#define RF4CE_ZRC_CLEAR_PROXY_BIND_FLAG(v) ((v) &= ~RF4CE_ZRC_PROXY_BIND_FLAG)
#define RF4CE_ZRC_SET_IN_VALIDATION_BLACKOUT_FLAG(v) ((v) |= RF4CE_ZRC_IN_BLACKOUT_VALIDATION_FLAG)
#define RF4CE_ZRC_CLEAR_IN_VALIDATION_BLACKOUT_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_BLACKOUT_VALIDATION_FLAG)
#define RF4CE_ZRC_SET_IN_VALIDATION_REP_FLAG(v) ((v) |= RF4CE_ZRC_IN_VALIDATION_REP_FLAG)
#define RF4CE_ZRC_CLEAR_IN_VALIDATION_REP_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_VALIDATION_REP_FLAG)
#define RF4CE_ZRC_SET_LOST_LINK_FLAG(v) ((v) |= RF4CE_ZRC_LOST_LINK_FLAG)
#define RF4CE_ZRC_CLEAR_LOST_LINK_FLAG(v) ((v) &= ~RF4CE_ZRC_LOST_LINK_FLAG)
#define RF4CE_ZRC_SET_RELEASE_FAIL_FLAG(v) ((v) |= RF4CE_ZRC_RELEASE_FAIL_FLAG)
#define RF4CE_ZRC_CLEAR_RELEASE_FAIL_FLAG(v) ((v) &= ~RF4CE_ZRC_RELEASE_FAIL_FLAG)

#define RF4CE_ZRC_SET_IN_VALIDATION_FLAG(v) ((v) |= RF4CE_ZRC_IN_VALIDATION_FLAG)
#define RF4CE_ZRC_CLEAR_IN_VALIDATION_FLAG(v) ((v) &= ~RF4CE_ZRC_IN_VALIDATION_FLAG)

#define RF4CE_ZRC_IS_ZRC1_CC(v) (((v) & RF4CE_ZRC1_FLAG) != 0)
#define RF4CE_ZRC_IS_ZRC2_CC(v) (((v) & RF4CE_ZRC1_FLAG) == 0)
#define RF4CE_ZRC_IS_CC_PRESSED(v) (((v) & RF4CE_ZRC_RELEASED_FLAG) == 0)
#define RF4CE_ZRC_IS_CC_RELEASED(v) (((v) & RF4CE_ZRC_RELEASED_FLAG) != 0)
#define RF4CE_ZRC_IS_CC_RELEASED_NO_CALLBACK(v) (((v) & RF4CE_ZRC1_RELEASED_NO_CALLBACK_FLAG) != 0)
#define RF4CE_ZRC_IS_CC_DATA_RECEIVED(v) (((v) & RF4CE_ZRC_DATA_RECEIVED_FLAG) != 0)
#define RF4CE_ZRC_IS_CC_IN_REQUEST(v) (((v) & RF4CE_ZRC_IN_CC_REQUEST_FLAG) != 0)
#define RF4CE_ZRC_IS_GETSET_USED(v) (((v) & RF4CE_ZRC_USED_GETSET_FLAG) != 0)
#define RF4CE_ZRC_IS_LEAVE_RECEIVER_ON(v) (((v) & RF4CE_ZRC_LEAVE_RECEIVER_ON_FLAG) != 0)
#define RF4CE_ZRC_IS_READ_WRITE_USED(v) (((v) & RF4CE_ZRC_READ_WRITE_USED_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_BLACKOUT(v) (((v) & RF4CE_ZRC_IN_BLACKOUT_FLAG) != 0)
#define RF4CE_ZRC_IS_GETTING_DATA(v) (((v) & RF4CE_ZRC_GETTING_DATA_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_BLACKOUT_GDP_TO_ZRC(v) (((v) & RF4CE_ZRC_IN_BLACKOUT_GDP_TO_ZRC_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_TIMEOUT_CONFIG_COMPLETE(v) (((v) & RF4CE_ZRC_IN_TIMEOUT_CONFIG_COMPLETE_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_WAITING_CALLBACK_CONFIG_COMPLETE(v) (((v) & RF4CE_ZRC_IN_WAITING_CALLBACK_CONFIG_COMPLETE_FLAG) != 0)
#define RF4CE_ZRC_IS_PROXY_BIND(v) (((v) & RF4CE_ZRC_PROXY_BIND_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_VALIDATION_BLACKOUT(v) (((v) & RF4CE_ZRC_IN_BLACKOUT_VALIDATION_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_VALIDATION_REP(v) (((v) & RF4CE_ZRC_IN_VALIDATION_REP_FLAG) != 0)
#define RF4CE_ZRC_IS_LOST_LINK(v) (((v) & RF4CE_ZRC_LOST_LINK_FLAG) != 0)
#define RF4CE_ZRC_IS_IN_VALIDATION(v) (((v) & RF4CE_ZRC_IN_VALIDATION_FLAG) != 0)
#define RF4CE_ZRC_IS_RELEASE_FAIL(v) (((v) & RF4CE_ZRC_RELEASE_FAIL_FLAG) != 0)

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC Storable Profile Data type.
 */
typedef struct _RF4CE_ZRC_StorableProfileData_t
{
    uint8_t aplZRC1CommandDiscovery[32];
} RF4CE_ZRC_StorableProfileData_t;

/**//**
 * \brief RF4CE ZRC Profile Data type.
 */
typedef struct _RF4CE_ZRC_ProfileData_t
{
    uint32_t commandDiscoveryTimeout;
    SYS_DataPointer_t zrcControlCommandFrame;
    uint32_t zrcFlags;
    uint32_t zrcControlCommandTimeout;
    uint32_t zrc1CCProc;
    RF4CE_NWK_RXEnableReqDescr_t nwkRXEnable;
    RF4CE_ZRC1_CommandDiscoveryResponseFrame_t cmdDiscoveryResponse;
    RF4CE_ZRC1_CommandDiscoveryStatus_t cmdDiscoveryResponseStatus;

#if defined(USE_RF4CE_PROFILE_ZRC2)
    union
    {
        RF4CE_ZRC2_GetAttributesReqDescr_t      get;
        RF4CE_ZRC2_SetAttributesReqDescr_t      set;

        /* Key Exchange aux requests */
        RF4CE_ZRC2_GetSharedSecretIndDescr_t    getSharedSecretRequest;
        RF4CE_NWK_UpdateKeyReqDescr_t           updateKeyRequest;

        /* action command aux request */
        rf4ceZrc2ActionMapReqDescr_t            mapReq;
    } requests;
    uint32_t zrcGetSetTimeout;

    /* Key Exchange Staff */
    RF4CE_NWK_RequestService_t              serverTask;             /* task to process a server indication */
    uint8_t newKey[RF4CE_SECURITY_KEY_LENGTH];
    uint8_t linkKey[RF4CE_SECURITY_KEY_LENGTH];
    uint8_t sharedSecret[RF4CE_SECURITY_KEY_LENGTH];
    uint32_t newKeyTagA;
    uint32_t newKeyTagB;
    uint32_t keyExchangeTimeout;
    uint16_t newKeyFlags;
    uint8_t inKeyExchange;
    uint8_t keyExchangeResult;
    uint32_t linkLostBlackoutTimeout;
    uint32_t allValidationTimeout;
    uint32_t validationRepeatTimeout;
    uint32_t checkValidationTimeout;

    /* Action command */
    SYS_DataPointer_t               actionCommandRepeatable;
    uint32_t                        actionCommandRepeatTimeout;
    uint32_t                        actionCommandWaitTimeout;
    rf4ceZrc2ControlCommandState_t  actionCommandState;
    uint8_t                         actionCommandOffset;
    Bool8_t                         actionCommandKeyCanceled;

    uint32_t gdp2HeartbeatTimeout;

    /* Private Get/Set variables */
    SYS_DataPointer_t payloadGet;
    uint16_t reqPayloadOffset;
    Bool8_t storeSimpleAttributesInNVM;
#endif /* defined(USE_RF4CE_PROFILE_ZRC2) */
} RF4CE_ZRC_ProfileData_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Processes the ZRC timeout task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC_TimerProc(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief Processes the ZRC cleanup task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \param[in] _zrcs_ - pointer to the RF4CE_ZRC_StorableProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC_ResetProfileData(RF4CE_ZRC_ProfileData_t *_zrc_, RF4CE_ZRC_StorableProfileData_t *_zrcs_);

/************************************************************************************//**
 \brief Processes the ZRC Command Discovery Timeout task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1CommandDiscoveryTimeoutHandler(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief Processes the ZRC Control Command Timeout task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrcControlCommandTimeoutHandler(RF4CE_ZRC_ProfileData_t *_zrc_);
/************************************************************************************//**
 \brief Processes the ZRC Control Command Repost task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1ControlCommandRepost(RF4CE_ZRC_ProfileData_t *_zrc_);

#if defined(USE_RF4CE_PROFILE_ZRC2)

/************************************************************************************//**
 \brief ZRC 2.0 Get/Set/Push/Pull attributes timeout task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetSetPushPullTimeout(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief ZRC 2.0 Get/Set/Push/Pull attributes additional warn timeout task.

 \param[in] _zrc_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetSetPushPullWarnTimeout(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief Processes the ZRC2 key exchange timeout task.

 \param[in] _zrc2_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 ****************************************************************************************/
void rf4ceZrc2KeyExchangeTimeoutHandler(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief Processes the ZRC2 aplActionRepeatTriggerInterval timeout task.

 \param[in] _zrc2_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 ****************************************************************************************/
void rf4ceZrc2ControlCommandRepeatTimeoutHandler(RF4CE_ZRC_ProfileData_t *_zrc_);

/************************************************************************************//**
 \brief Processes the ZRC2 aplActionRepeatWaitTime timeout task.

 \param[in] _zrc2_ - pointer to the RF4CE_ZRC_ProfileData_t structure.
 ****************************************************************************************/
void rf4ceZrc2ControlCommandWaitTimeoutHandler(RF4CE_ZRC_ProfileData_t *_zrc_);

#endif /* defined(USE_RF4CE_PROFILE_ZRC2) */

#endif /* _RF4CE_ZRC_PROFILE_DATA_H */
