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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/private/bbRF4CEZRCPrivateBind.h $
 *
 * DESCRIPTION:
 *   This is the private header file for the RF4CE ZRC profile Binding functionality.
 *
 * $Revision: 2512 $
 * $Date: 2014-05-26 11:58:18Z $
 *
 ****************************************************************************************/
#ifndef BBRF4CEZRCPRIVATEBIND_H
#define BBRF4CEZRCPRIVATEBIND_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysBasics.h"
#include "bbSysQueue.h"
#include "bbRF4CEZRCBind.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief The MIN LQI filter for discovery user string
 */
#define RF4CE_GDP_MIN_LQI_FILTER_DISCOVERY 12

/**//**
 * \brief The Vendor Id filter for discovery user string
 */
#define RF4CE_GDP_BO_USER_STRING_DISCOVERY_VENDOR_ID_FILTER_INDX 9
#define RF4CE_GDP_BO_USER_STRING_DISCOVERY_VENDOR_ID_FILTER_WILDCARD 0xFFFF

/**//**
 * \brief The Min/Max class filter for discovery user string
 */
#define RF4CE_GDP_BO_USER_STRING_DISCOVERY_MIN_MAX_CLASS_FILTER_INDX 11

/**//**
 * \brief The Min LQI filter for discovery user string
 */
#define RF4CE_GDP_BO_USER_STRING_DISCOVERY_MIN_LQI_FILTER_INDX 12

/**//**
 * \brief The LQI filter for discovery user string
 */
#define RF4CE_GDP_BR_USER_STRING_DISCOVERY_LQI_THRESHOLD_INDX 14

/**//**
 * \brief The tertiary class descriptor for discovery user string
 */
#define RF4CE_GDP_BR_USER_STRING_TERTIARY_CLASS_DESCR_INDX 11

/**//**
 * \brief The secondary class descriptor for discovery user string
 */
#define RF4CE_GDP_BR_USER_STRING_SECONDARY_CLASS_DESCR_INDX 12

/**//**
 * \brief The primary class descriptor for discovery user string
 */
#define RF4CE_GDP_BR_USER_STRING_PRIMARY_CLASS_DESCR_INDX 13

/**//**
 * \brief Amount of binding class descriptor in Binding Recipient user string (primary, secondary, tertiary).
 */
#define RF4CE_ZRC2_NUMBER_OF_CLASS_DESCRIPTORS  (3)
#define RF4CE_ZRC2_PRIMARY_CLASS_DESCRIPTOR     (2)
#define RF4CE_ZRC2_SECONDARY_CLASS_DESCRIPTOR   (1)
#define RF4CE_ZRC2_TERTIARY_CLASS_DESCRIPTOR    (0)

#define RF4CE_ZRC2_CLASS_DESCR_ARRAY_ITERATION(iterator) \
        for (uint8_t i = 0, iterator = RF4CE_ZRC2_PRIMARY_CLASS_DESCRIPTOR; i < RF4CE_ZRC2_NUMBER_OF_CLASS_DESCRIPTORS; i++, iterator--)

/**//**
 * \brief Memory depth for the previous binding candidates.
 */
#define RF4CE_ZRC2_BINDING_CANDIDATES_MEMORY_DEPTH  3

/************************* TYPES *******************************************************/
/**//**
 * \brief ZRC 2.0 pairing candidate entry.
 */
typedef struct _RF4CE_ZRC2_PairingCandidate_t
{
    SYS_QueueElement_t queueElement;                                    /*!< Service field */
    uint64_t address;                                                   /*!< Node's IEEE address */
    uint16_t panId;                                                     /*!< Node's pan ID */
    uint8_t logicalChannel;                                             /*!< Node's logical channel */
    uint8_t classDescriptors[RF4CE_ZRC2_NUMBER_OF_CLASS_DESCRIPTORS];   /*!< Primary node's class */
    uint8_t provisionalProfile;                                         /*!< Node's profile ID */
    uint8_t discRespLQI;                                                /*!< Discovery response LQI */
} RF4CE_ZRC2_PairingCandidate_t;

/**//**
 * \brief ZRC 2.0 previous binding candidate parameters.
 */
typedef struct _RF4CE_ZRC2_PreviousPairingCandidate_t
{
    uint64_t address;                   /*!< Node's IEEE address */
    uint8_t primaryClass;               /*!< Primary node's class */
} RF4CE_ZRC2_PreviousPairingCandidate_t;

/**//**
 * \brief ZRC 2.0 binding state.
 */
typedef enum _Rf4ceZrcBindingState_t
{
    RF4CE_ZRC_BINDING_STATE_IDLE,
    RF4CE_ZRC_BINDING_STATE_DISCOVERY,
    RF4CE_ZRC_BINDING_STATE_PAIRING,
    RF4CE_ZRC_BINDING_STATE_PAIRING_BLACKOUT,
    RF4CE_ZRC_BINDING_STATE_GDP_CONFIGURATION,
    RF4CE_ZRC_BINDING_STATE_GDP_CONFIGURATION_BLACKOUT,
    RF4CE_ZRC_BINDING_STATE_ZRC_CONFIGURATION,
    RF4CE_ZRC_BINDING_STATE_ZRC_CONFIGURATION_BLACKOUT,
    RF4CE_ZRC_BINDING_STATE_VALIDATION,
    RF4CE_ZRC_BINDING_STATE_EXTENDED_VALIDATION,
    RF4CE_ZRC_BINDING_STATE_ERROR,
    RF4CE_ZRC_BINDING_STATE_BOUND,

    RF4CE_ZRC_BINDING_NUMBER_OF_STATES
} Rf4ceZrcBindingState_t;

/**//**
 * \brief ZRC 2.0 binding timer state.
 */
typedef enum _Rf4ceZrcBindingTotalTimerState_t
{
    RF4CE_ZRC_BINDING_TIMER_STATE_RUN,
    RF4CE_ZRC_BINDING_TIMER_STATE_TIMEOUT_NOT_CHECKED,
    RF4CE_ZRC_BINDING_TIMER_STATE_TIMEOUT_CHECKED,
} Rf4ceZrcBindingTotalTimerState_t;

/**//**
 * \brief ZRC 2.0 binding role.
 */
typedef enum _Rf4ceZrcBindingRole_t
{
    RF4CE_ZRC_BINDING_ORIGINATOR,
    RF4CE_ZRC_BINDING_RECIPIENT
} Rf4ceZrcBindingRole_t;

/**//**
 * \brief ZRC 2.0 binding validation state.
 */
typedef enum _Rf4ceZrcBindingValidationState_t
{
    RF4CE_ZRC_VALIDATION_STATE_IDLE,
    RF4CE_ZRC_VALIDATION_STATE_WAIT_FOR_VALIDATION_RESPONSE,
    RF4CE_ZRC_VALIDATION_STATE_DISCOVERING,
} Rf4ceZrcBindingValidationState_t;

/**//**
 * \brief RF4CE ZRC GDP Discovery Complete callback.
 */
typedef void (*RF4CE_ZRC_DiscoveryCompleteCB_t)(RF4CE_ZRC2GDP2_Status_t status, RF4CE_ZRC2_PairingCandidate_t *candidate);

/**//**
 * \brief RF4CE ZRC GDP Configuration Complete callback.
 */
typedef void (*RF4CE_ZRC_ConfigurationCompleteCB_t)(RF4CE_ZRC2GDP2_Status_t status);

/**//**
 * \brief ZRC 2.0 binding related static data.
 */
typedef struct _RF4CE_ZRC2_BindingData_t
{
    uint8_t prevCandidatesPosition;
    RF4CE_ZRC2_PreviousPairingCandidate_t prevCandidates[RF4CE_ZRC2_BINDING_CANDIDATES_MEMORY_DEPTH];
    RF4CE_ZRC2_PairingCandidate_t candidate;

    SYS_TimeoutTask_t zrc2BindTotalTimeout;
    Rf4ceZrcBindingTotalTimerState_t totalTimerState;

    SYS_TimeoutTask_t zrc2BindBlackoutTimer;
    SYS_TimeoutTask_t zrc2ValidationTimer;

    SYS_TimeoutTask_t zrc2RecipientBindTimer;

    uint16_t zrc2ValidationTotalTimeout;
    uint16_t zrc2AutoCheckValidationTimer;
    uint16_t zrc2LinkLostTimer;
    uint16_t zrc2ExtendedValidationDiscoveryTimer;
    uint8_t  logicalChannel;

    Rf4ceZrcBindingRole_t  bindingRole;
    Rf4ceZrcBindingState_t state;
    uint16_t index;
    uint8_t validationResult;
    Rf4ceZrcBindingValidationState_t validationState;
    Bool8_t pushButtonStimulusReceived;
    uint8_t temporaryPairingRef;

    struct {
        uint64_t srcIeeeAddr;
        uint8_t userString[RF4CE_NWK_USER_STRING_LENGTH];
    } prevDiscReqParams;

    RF4CE_ZRC_DiscoveryCompleteCB_t discoveryCb;

    RF4CE_ZRC2_BindConfParams_t bindConfirm;
} RF4CE_ZRC2_BindingData_t;

#define RF4CE_ZRC2_BINDING_DATA()      &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->bindingData

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
  \brief ZRC2 Bind Total Timeout task handler.
  \param[in] taskDescriptor - pointer to the task descriptor.
****************************************************************************************/
void rf4cezrc2BindTotalTimeoutHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Starts discovery procedure.
    \param[in] discoveryCb - callback to be called after the procedure is finished.
    \return true if procedure has been started successfully.
 ****************************************************************************************/
bool rf4cezrc2StartDiscoveryProcedure(RF4CE_ZRC_DiscoveryCompleteCB_t discoveryCb);

/************************************************************************************//**
    \brief Tries to build a pair request and issue a NWK Pair Request to the pairing
           candidate specified.
    \param[in] candidate - pointer to the pairing candidate data.
    \return true on success.
 ****************************************************************************************/
bool rf4cezrc2StartPairingProcedure(RF4CE_ZRC2_PairingCandidate_t *candidate);

/************************************************************************************//**
    \brief Function is called after the pairing procedure is finished.
    \param[in] status - result status of pairing procedure.
    \param[in] pairingRef - pairing reference of the remote device.
    \param[in] protocolId - protocol identifier supported by other side.
 ****************************************************************************************/
void rf4cezrc2PairingProcedureFinished(RF4CE_NLDE_DATA_Status_t status, uint8_t pairingRef, uint8_t protocolId);

/************************************************************************************//**
    \brief Initiates unpairing procedure.
 ****************************************************************************************/
void rf4cezrc2StartUnpairingProcedure(void);

/************************************************************************************//**
 \brief Starts the ZRC 2.0 GDP Configuration phase.
 ****************************************************************************************/
void rf4cezrc2GdpConfigurationStart(void);

/************************************************************************************//**
 \brief Starts the ZRC 2.0 ZRC Configuration phase.
 ****************************************************************************************/
void rf4cezrc2ZrcConfigurationStart(void);

/************************************************************************************//**
 \brief Sends the ZRC 2.0 GDP 2.0 Configuration Complete request.
 \param[in] callback - pointer to the callback function.
 ***************************************************************************************/
void rf4cezrc2SendConfigurationComplete(void);

/************************************************************************************//**
 \brief Configuration Complete ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2ConfigurationCompleteInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

/************************************************************************************//**
    \brief Function is called after the GDP Configuration procedure is finished.
    \param[in] status - result status of GDP configuration procedure.
 ***************************************************************************************/
void rf4cezrc2ConfigurationFinished(RF4CE_ZRC2GDP2_Status_t status);

/************************************************************************************//**
  \brief ZRC2 Bind Blackout Timer task handler.
  \param[in] taskDescriptor - pointer to the task descriptor.
****************************************************************************************/
void rf4cezrc2BindBlackoutTimerHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief Starts the ZRC 2.0 ZRC Validation phase entry.
 ***************************************************************************************/
void rf4cezrc2ZrcValidationStart(void);

/************************************************************************************//**
  \brief ZRC2 Bind Validation Timer task handler.
  \param[in] taskDescriptor - pointer to the task descriptor.
****************************************************************************************/
void rf4cezrc2BindValidationTimerHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Terminates binding procedure with current candidate and starts it with
           new one.
    \param[in] newCandidate - pointer to the best found candidate.
 ****************************************************************************************/
void rf4cezrc2ChangeCandidate(RF4CE_ZRC2_PairingCandidate_t *newCandidate);

/************************************************************************************//**
    \brief Function is called after the Validation procedure is finished.
    \param[in] status - result status of configuration procedure.
 ****************************************************************************************/
void rf4cezrc2ValidationFinished(RF4CE_ZRC2GDP2_Status_t status);

/************************************************************************************//**
    \brief Compiles ZRC 2.0 Binding Originator User String. See GDP 2.0 Spec r29 7.2.3.1.
    \param[in, out] buffer - pointer to the memory allocated for User String.
 ****************************************************************************************/
void rf4cezrc2CompileOriginatorUserString(uint8_t *buffer);

#ifdef RF4CE_TARGET
/************************************************************************************//**
    \brief Compiles ZRC 2.0 Binding Recipient User String. See GDP 2.0 Spec r29 7.2.3.1.
    \param[in, out] buffer - pointer to the memory allocated for User String.
 ****************************************************************************************/
void rf4cezrc2CompileRecipientUserString(uint8_t *buffer);

/************************************************************************************//**
    \brief Responses on the the received discovery request.
    \param[in] indication - pointer to the discovery request parameters.
    \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_DiscoveryResponse(RF4CE_NWK_DiscoveryIndParams_t *indication);

// TODO
// make a comments
void rf4cezrc2StartRecipientBindTimer(void);
// TODO
// make a comments
void rf4cezrc2RenewRecipientBindTimer(void);
// TODO
// make a comments
void rf4cezrc2StopRecipientBindTimer(void);
// TODO
// make a comments
void rf4cezrc2BindRecipientTimerHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
// TODO
// make a comments
#endif

/************************************************************************************//**
    \brief Checks if the received discovery request shall be answered with
           discovery response.
    \param[in] indication - pointer to the discovery request parameters.
    \return True if discovery response shall be issued, false otherwise.
 ****************************************************************************************/
bool RF4CE_ZRC2_IsDiscoveryResponseNeeded(RF4CE_NWK_DiscoveryIndParams_t *indication);

/************************************************************************************//**
    \brief Returns true if the device is not binding
 ****************************************************************************************/
bool rf4cezrc2BindIsStatusIdle(void);

/************************************************************************************//**
    \brief Clears Push Button Stimulus flag on the Target.
 ****************************************************************************************/
void rf4cezrc2ClearPushButtonStimulus(void);

#endif // BBRF4CEZRCPRIVATEBIND_H
