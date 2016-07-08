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
 * FILENAME: $Workfile: $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC component tasks functions.
 *
 * $Revision: $
 * $Date: $
 *
 ****************************************************************************************/
#ifndef BBRF4CEZRCTASKS_H
#define BBRF4CEZRCTASKS_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysTaskScheduler.h"
#include "bbSysQueue.h"
#include "bbRF4CEZRC.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC Task ID enumeration.
  */
typedef enum _RF4CE_ZRC_TaskID_t
{
    RF4CE_ZRC1_TARGET_BIND_TIMEOUT_HANDLER = 0,
    RF4CE_ZRC_COMMON_GROUP_HANDLER,
    RF4CE_ZRC_GETSET_GROUP_HANDLER,
    RF4CE_ZRC_CC_GROUP_HANDLER,
    RF4CE_ZRC_ALL_GROUP_HANDLER,
    RF4CE_ZRC_PRIVATE_GROUP_HANDLER,
    RF4CE_ZRC2_BIND_TOTAL_TIMEOUT_HANDLER,
    RF4CE_GDP2_HEARTBEAT_GROUP_HANDLER,
    RF4CE_GDP2_CLIENT_NOTIFICATION_GROUP_HANDLER,
    RF4CE_ZRC2_BIND_BLACKOUT_TIMER_HANDLER,
    RF4CE_ZRC2_BIND_VALIDATION_TIMER_HANDLER,
    RF4CE_ZRC2_BR_PUSHBUTTON_TIMEOUT_HANDLER,

    // TODO FOR DEBUG ONLY
    RF4CE_ZRC2_ORIGNATOR_CONFIGURATION_TIMEOUT_HANDLER,
    // TODO FOR DEBUG ONLY - END

#ifdef RF4CE_TARGET
    RF4CE_ZRC2_BIND_RECIPIENT_TIMER_HANDLER,
#endif /* #ifdef RF4CE_TARGET */

    RF4CE_NUMBER_OF_HANDLERS,
} RF4CE_ZRC_TaskID_t;

/**//**
 * \brief RF4CE ZRC Handler ID enumeration.
  */
typedef enum _RF4CE_ZRC_HandlerID_t
{
    /* Common Group */
    RF4CE_ZRC_START_HANDLER = 0,
    RF4CE_ZRC_RESET_HANDLER,
    RF4CE_ZRC1_CONTROLLER_BIND_HANDLER,
    RF4CE_ZRC1_TARGET_BIND_HANDLER,
    RF4CE_ZRC2_ENABLE_BIND_HANDLER,
    RF4CE_ZRC2_DISABLE_BIND_HANDLER,
    RF4CE_ZRC2_SET_PUSH_BUTTON_FLAG_HANDLER,
    RF4CE_ZRC2_CLEAR_PUSH_BUTTON_FLAG_HANDLER,
    RF4CE_ZRC2_BIND_HANDLER,
    RF4CE_ZRC2_PROXY_BIND_HANDLER,

    /* Get/Set Group */
    RF4CE_ZRC1_GET_ATTRIBUTE_HANDLER,
    RF4CE_ZRC1_SET_ATTRIBUTE_HANDLER,
    RF4CE_ZRC2_GET_ATTRIBUTE_HANDLER,
    RF4CE_ZRC2_SET_ATTRIBUTE_HANDLER,
    RF4CE_ZRC2_PULL_ATTRIBUTE_HANDLER,
    RF4CE_ZRC2_PUSH_ATTRIBUTE_HANDLER,

    /* Control Command Group */
    RF4CE_ZRC1_CONTROL_COMMAND_PRESSED_HANDLER,
    RF4CE_ZRC1_CONTROL_COMMAND_RELEASED_HANDLER,
    RF4CE_ZRC2_CONTROL_COMMAND_PRESSED_HANDLER,
    RF4CE_ZRC2_CONTROL_COMMAND_RELEASED_HANDLER,
    RF4CE_ZRC2_CONTROL_COMMAND_MAP_HANDLER,
    RF4CE_ZRC2_CONTROL_COMMAND_TX_HANDLER,
    RF4CE_ZRC2_ACTION_MAPPING_HANDLER,

    /* All Group */
    RF4CE_ZRC1_COMMAND_DISCOVERY_HANDLER,
    RF4CE_ZRC1_VENDOR_SPECIFIC_HANDLER,

    /* All Group. KEY EXCHANGE. Client */
    RF4CE_ZRC2_KEY_EXCHANGE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_HANDLER_SEND,
    RF4CE_ZRC2_KEY_EXCHANGE_CHALLENGE_RESPONSE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_SEND_RESPONSE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_CONFIRM_HANDLER,

    /* All Group. KEY EXCHANGE. Server */
    RF4CE_ZRC2_KEY_EXCHANGE_CHALLENGE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_CHALLENGE_SEND_RESPONSE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_RESPONSE_HANDLER,
    RF4CE_ZRC2_KEY_EXCHANGE_SEND_CONFIRM_HANDLER,

    /* Polling group. */
    RF4CE_GDP2_HEARTBEAT_HANDLER,
    RF4CE_GDP2_CLIENT_NOTIFICATION_HANDLER,
    RF4CE_GDP2_GENERIC_RESPONSE_HANDLER,

    RF4CE_ZRC_NUMBER_OF_REQUEST_HANDLERS
} RF4CE_ZRC_HandlerID_t;

/**//**
 * \brief RF4CE ZRC Tasks filters.
  */
#define ZRC_COMMON_GROUP_BUSY_FLAG 1
#define IS_ZRC_COMMON_GROUP_BUSY() (0 != (zrc->flagsBusy & ZRC_COMMON_GROUP_BUSY_FLAG))
#define SET_ZRC_COMMON_GROUP_BUSY() zrc->flagsBusy |= ZRC_COMMON_GROUP_BUSY_FLAG
#define CLEAR_ZRC_COMMON_GROUP_BUSY() zrc->flagsBusy &= ~ZRC_COMMON_GROUP_BUSY_FLAG
#define ZRC_GETSET_GROUP_BUSY_FLAG 2
#define IS_ZRC_GETSET_GROUP_BUSY() (0 != (zrc->flagsBusy & ZRC_GETSET_GROUP_BUSY_FLAG))
#define SET_ZRC_GETSET_GROUP_BUSY() zrc->flagsBusy |= ZRC_GETSET_GROUP_BUSY_FLAG
#define CLEAR_ZRC_GETSET_GROUP_BUSY() zrc->flagsBusy &= ~ZRC_GETSET_GROUP_BUSY_FLAG
#define ZRC_CC_GROUP_BUSY_FLAG 4
#define IS_ZRC_CC_GROUP_BUSY() (0 != (zrc->flagsBusy & ZRC_CC_GROUP_BUSY_FLAG))
#define SET_ZRC_CC_GROUP_BUSY() zrc->flagsBusy |= ZRC_CC_GROUP_BUSY_FLAG
#define CLEAR_ZRC_CC_GROUP_BUSY() zrc->flagsBusy &= ~ZRC_CC_GROUP_BUSY_FLAG
#define ZRC_ALL_GROUP_BUSY_FLAG 8
#define IS_ZRC_ALL_GROUP_BUSY() (0 != (zrc->flagsBusy & ZRC_ALL_GROUP_BUSY_FLAG))
#define SET_ZRC_ALL_GROUP_BUSY() zrc->flagsBusy |= ZRC_ALL_GROUP_BUSY_FLAG
#define CLEAR_ZRC_ALL_GROUP_BUSY() zrc->flagsBusy &= ~ZRC_ALL_GROUP_BUSY_FLAG

/**//**
 * \brief RF4CE ZRC1 Task array.
  */
extern const SYS_SchedulerTaskHandler_t rf4cezrcTaskHandlers[];

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Posts requests for handling.
  \param[in] serviceField - pointer to the service field of the request.
****************************************************************************************/
void rf4cezrcPostRequest(RF4CE_NWK_RequestService_t *const serviceField);

/************************************************************************************//**
 \brief ZRC 1.1 Get/Set Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1GetSetAttributeHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Get Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2GetAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Private Get Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PrivateGetAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Set Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2SetAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Private Set Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PrivateSetAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Push Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PushAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Pull Attributes Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2PullAttributesHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Enable/Disable Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2EnableDisableBindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Push Button Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2ButtonBindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2BindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 2.0 Proxy Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
// void rf4cezrc2ProxyBindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 1.1 Controller Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1CBindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC 1.1 Target Bind Request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1TBindHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
  \brief ZRC1 Target Bind Timeout task handler.
  \param[in] taskDescriptor - pointer to the task descriptor.
****************************************************************************************/
void rf4cezrc1TBindTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief RF4CE ZRC 1.1 Command Discovery request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1CommandDiscoveryHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief ZRC1 Control Command Pressed/Released task handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
****************************************************************************************/
void rf4cezrc1ControlCommandHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief Vendor Specific sending task.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc1VendorSpecificHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief RF4CE ZRC Start request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrcStartHandler(SYS_QueueElement_t *queueElement);

/************************************************************************************//**
 \brief RF4CE ZRC Reset request handler.

 \param[in] queueElement - pointer to the queue element structure.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrcResetHandler(SYS_QueueElement_t *queueElement);

#if defined(USE_RF4CE_PROFILE_ZRC2)
/************************************************************************************//**
 \brief RF4CE ZRC2 Key Exchange task handlers

 \param[in] queueElement - pointer to the queue element structure.
 ****************************************************************************************/
void rf4cezrc2KeyExchangeHandler(SYS_QueueElement_t *queueElement);
void rf4ceZrc2KeyExchangeHandlerSend(SYS_QueueElement_t *queueElement);
void rf4cezrc2KeyExchangeChallengeHandler(SYS_QueueElement_t *queueElement);
void rf4cezrc2KeyExchangeChallengeSendResponseHandler(SYS_QueueElement_t *queueElement);
void rf4ceZrc2KeyExchangeChallengeResponseHandler(SYS_QueueElement_t *queueElement);
void rf4ceZrc2KeyExchangeSendResponseHandler(SYS_QueueElement_t *queueElement);
void rf4cezrc2KeyExchangeResponseHandler(SYS_QueueElement_t *queueElement);
void rf4ceZrc2KeyExchangeSendConfirmHandler(SYS_QueueElement_t *queueElement);
void rf4ceZrc2KeyExchangeConfirmHandler(SYS_QueueElement_t *queueElement);

#endif /* defined(USE_RF4CE_PROFILE_ZRC2) */

/************************************************************************************//**
  \brief Starts timer with specified parameters.
****************************************************************************************/
void rf4cePostTimeoutTask(SYS_TimeoutTask_t *timeoutTask, SYS_TimeoutTaskMode_t mode);

// TODO FOR DEBUG ONLY
void rf4cezrc2OriginatorConfigurationTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
// TODO FOR DEBUG ONLY - END

#endif // BBRF4CEZRCTASKS_H
