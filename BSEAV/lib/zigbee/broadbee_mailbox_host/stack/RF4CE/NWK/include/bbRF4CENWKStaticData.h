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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKStaticData.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer component NIB Attributes static
 *   data declaration.
 *
 * $Revision: 3456 $
 * $Date: 2014-09-04 11:56:47Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_STATIC_DATA_H
#define _RF4CE_NWK_STATIC_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"

#ifdef USE_RF4CE_NWK

#include "bbRF4CENWKNIBAttributes.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKConstants.h"
#include "bbSysTaskScheduler.h"
#include "bbSysTimeoutTask.h"
#include "bbSysQueue.h"
#include "bbRF4CENWKData.h"
#include "bbRF4CENWKStartReset.h"
#include "bbRF4CENWKDiscovery.h"
#include "bbRF4CENWKPair.h"
#include "bbRF4CENWKRX.h"
#include "bbRF4CEFrame.h"
#include "bbMacSapForRF4CE.h"
#include "bbSecurity.h"
#include "bbRF4CENWKUpdateKey.h"
#ifdef RF4CE_NWK_PRINT_KEY
#include "bbSysPrint.h"
#endif

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK incoming data indications queue.
 */
typedef struct _RF4CE_NWK_MACIndication_t
{
    SYS_QueueElement_t queueElement;
    MAC_DataIndParams_t indication;
} RF4CE_NWK_MACIndication_t;

/**//**
 * \brief RF4CE NWK static data structure.
 */
typedef struct _RF4CE_NWK_StaticData_t
{
    /*!< Network Information Base */
    RF4CE_NIB_Attributes_t NIB;
    /*!< Current Network State */
    RF4CE_NWKState_t nwkState;
    /*!< Current network address */
    MAC_Addr16bit_t shortAddress;
    /*!< Current network Pan ID */
    MAC_PanId_t panId;
    /*!< Tasks for network */
    SYS_SchedulerTaskDescriptor_t taskDescriptor;\
    RF4CE_NWK_SetReqDescr_t *setRequest;
    uint8_t lastResetType;
    uint8_t keySeedFrameSequenceNumber;
    uint8_t safeChannelNumber;
#ifdef RF4CE_NWK_PRINT_KEY
    SYS_PrintIndParams_t indKey;
    SYS_PrintIndParams_t indNonce;
    SYS_PrintIndParams_t indA;
    SYS_PrintIndParams_t indData;
    SYS_PrintIndParams_t indEData;
    struct
    {
        uint8_t customNonce[SECURITY_CCM_NONCE_SIZE];
        uint8_t customA[SECURITY_CCM_NONCE_SIZE];
        uint8_t customKey[16];
        uint8_t customData[16];
        uint8_t customDataSize;
        uint8_t customEData[16];
        uint8_t customEDataSize;
    };
#endif
#ifdef SPIKE_FOR_MAC_RX_ENABLE
    Bool8_t rxOn;
    SYS_TimeoutTask_t rxEnableTimeout;
#endif /* SPIKE_FOR_MAC_RX_ENABLE */
    struct
    {
        MAC_GetReqDescr_t get;                           /*!< MAC GET request */
        MAC_SetReqDescr_t set;                           /*!< MAC SET request */
        MAC_DataReqDescr_t data;                         /*!< MAC DATA request */
        MAC_ResetReqDescr_t reset;                       /*!< MAC RESET request - for Target only */
#ifdef RF4CE_TARGET
        MAC_ScanReqDescr_t scan;                         /*!< MAC SCAN request - for Target only */
        MAC_StartReqDescr_t start;                       /*!< MAC START request - for Target only */
        MAC_SetReqDescr_t setFA;                         /*!< MAC SET request for frequency agility - for Target only */
#endif /* RF4CE_TARGET */
    } macRequests;
    union
    {
        RF4CE_NWK_StartReqDescr_t *start;                /*!< Currently running START request */
        RF4CE_NWK_ResetReqDescr_t *reset;                /*!< Currently running RESET request */
    } nwkStartResetRequest;
    MAC_DataReqDescr_t *macUnpairRequest;

#ifdef RF4CE_ENABLE_MAC_STATS
    uint8_t inMacRequest;
    uint8_t macRequestId;
    uint8_t macRequestAttributeId;
    union
    {
        MAC_DataConfCallback_t *data;
        MAC_GetConfCallback_t *get;
        MAC_ResetConfCallback_t *reset;
#ifdef RF4CE_TARGET
        MAC_ScanConfCallback_t *scan;
#endif
        MAC_SetConfCallback_t *set_;
        MAC_StartConfCallback_t *start;
    } macRequestCallback;
#endif

    /*!< The timeout measuring task */
    SYS_TimeoutTask_t timeoutTask;
    /*!< The timeout task for DATA requests */
    SYS_TimeoutTask_t timeoutTaskData;
    /*!< The timeout measuring for Power Saving task */
    SYS_TimeoutTask_t powerSavingTask;
    /*!< The timeout task for dereffed RX Enable task */
    SYS_TimeoutTask_t rxEnableDeferredTask;
    /*!< Power Saving RX enable request */
    RF4CE_NWK_RXEnableReqDescr_t psRXEnableReq;
    /*!< General Purpose RX enable request */
    RF4CE_NWK_RXEnableReqDescr_t gpRXEnableReq;
    Bool8_t isDiscoveryRequest;
    /*!< The pairing reference for the currently pairing sequence */
    uint8_t temporaryPairingRef;
    /*!< The internal data frame for MAC DATA requests */
    SYS_DataPointer_t internalFrame;
    RF4CE_NWK_SetReqDescr_t *setNIBReq;
    /*!< Pointer to the current pairing entry for the currently pairing sequence */
    RF4CE_PairingTableEntry_t *temporaryPairing;
    /*!< Currently running UNPAIR request */
    RF4CE_NWK_UnpairReqDescr_t *unpairRequest;
    /*!< Currently running UNAPIR request data indication */
    RF4CE_NWK_UnpairIndParams_t indicationUnpair;
    /*!< Currently running RX-ENABLE request */
    RF4CE_NWK_RXEnableReqDescr_t *rxEnableRequest;
    /*!< Current Outgoing packet */
    RF4CE_NWK_OutgoingPacket_t *pOutgoingPacket;
    /*!< The MAC RX Enable request descriptor */
    MAC_RxEnableReqDescr_t macRXEnableRequest;
    /*!< The counter of the passed Discovery Request repetitions */
    uint8_t numDiscoveryRepetitions;
    /*!< The discovered nodes collector for Discovery Confirmation */
    SYS_DataPointer_t discoveryNodes;
    /*!< The number of discovered nodes collected so far for Discovery Confirmation */
    uint8_t numDiscoveryNodes;
    union
    {
        RF4CE_NWK_DiscoveryReqDescr_t *discoveryRequest; /*!< Currently running DISCOVERY request */
        RF4CE_NWK_PairReqDescr_t *pairRequest;           /*!< Currently running PAIR request */
    } internalRequest;
    /*!< The used pair response header for pairing sequence */
    PairResponseFrameHeader_t headerPair;
    /*!< The used pair response variable part for pairing sequence */
    uint8_t pairVarPart[RF4CE_NWK_MAX_TOTAL_VARIABLE_LENGTH];
    SecurityKey_t oldKey;
    /*!< The generated pairing ping request data */
    uint32_t pairingData;
    uint8_t broadcastTXChannel;
    uint8_t dataTXResult;
    /*!< Currently set pair request status - for Target only */
    RF4CE_NLDE_DATA_Status_t pairStatus;
    /*!< Currently set pair confirmation - for Target only */
    RF4CE_NWK_PairConfParams_t confirmPair;
    RF4CE_NWK_PairConfParams_t *pConfirmPair;
    /*!< Currently set discovery request status */
    uint8_t discoveryStatus;
    /*!< Currently set channel */
    uint8_t currentChannel;
    /*!< Currently set data request status */
    uint8_t currentDataStatus;
#ifdef RF4CE_TARGET
    /*!< Currently running AUTO Discovery request - for Target only */
    RF4CE_NWK_AutoDiscoveryReqDescr_t *autoDiscoveryRequest;
    uint8_t nwkIndicateDiscoveryParameterBackup;
    SYS_QueueDescriptor_t autoDiscoveryQueue;
    /*!< Currently running AUTO Discovery response descriptor - for Target only */
    RF4CE_NWK_DiscoveryRespDescr_t *discoveryResponse;
    uint64_t discoveryIeeeAddr;
    /*!< MAC Data rquest for Discovery Response - for Target only */
    MAC_DataReqDescr_t dataDiscoveryResponse;
    /*!< Currently running AUTO Discovery request confirmation structure - for Target only */
    RF4CE_NWK_AutoDiscoveryConfParams_t autoDiscoveryConfirm;
    /*!< Currently running Discovery Request footer to be stored for compare - for Target only */
    uint8_t requestFooter;
    /*!< Used for KEY SEED request counter - for Target only */
    uint8_t keySeedNeeded;
    /*!< Used for KEY SEED request security key storage - for Target only */
    SecurityKey_t securityKeySeed;
    /*!< Current pairing status - for Target only */
    RF4CE_NLDE_DATA_Status_t pairingStatus;
    /*!< The timeout measuring for Frequency Agility task */
    SYS_TimeoutTask_t frequencyAgilityTask;
    /*!< Indicates if Frequency Agility task is started */
    Bool8_t isFrequencyAgilityOn;
    uint8_t energyLevels[3];
    uint8_t faCnt;
    /*!< This variable will handle the profile ID according to supplied Pair Response which is necessary for
         proper COMMStatusIndication handling in the Profile Manager */
    uint8_t currentPairProfileId[RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH];
    uint8_t currentPairProfileIdLength;
#ifdef RF4CE_NWK_GU_DISCOVERY
    uint8_t isSpecialAutoDiscoveryOn;
    uint8_t lenSpecialAutoDiscoveryDevices;
    uint8_t specialAutoDiscoveryDevices[3];
    uint8_t lenSpecialAutoDiscoveryProfiles;
    uint8_t specialAutoDiscoveryProfiles[7];
#endif /* RF4CE_NWK_GU_DISCOVERY */
#endif /* RF4CE_TARGET */
    /*!< Current request's dataHandle (being incremented upon successful MAC DATA request) */
    uint8_t dataHandle;
    /*!< The NWK.GET NWK.SET requests queue */
    SYS_QueueDescriptor_t setGetQueue;
    /*!< The NWK.START NWK.RESET requests queue */
    SYS_QueueDescriptor_t startResetQueue;
    /*!< The NWK.RX-Enable requests queue */
    SYS_QueueDescriptor_t rxEnableQueue;
    /*!< The NWK.RX-Enable deferred requests queue */
    SYS_QueueDescriptor_t rxEnableDeferredQueue;
    /*!< The preserved space for incoming data packets */
    RF4CE_NWK_IncomingPacket_t incomingPackets[RF4CE_NWK_MAX_INCOMING_PACKETS];
    /*!< The only one existing outgoing packet */
    RF4CE_NWK_OutgoingPacket_t outgoingPacket;
    /*!< The MAC data indication queue */
    SYS_QueueDescriptor_t dataIndicationQueue;
    /*!< The MAC data indication packeted queue */
    SYS_QueueDescriptor_t dataIndicationPacketsQueue;
    /*!< The Discovery Requests queue */
    SYS_QueueDescriptor_t discoveryQueue;
    /*!< The Pair Requests/Responses queue */
    SYS_QueueDescriptor_t pairingQueue;
    /*!< The Update Key Requests queue */
    SYS_QueueDescriptor_t updateKeyQueue;
    /*!< The DATA Requests queue */
    SYS_QueueDescriptor_t dataRequestQueue;
    /*!< The request for Encryption/Decryption of the current data request */
    Security_CCMReq_t securityRequest;
    /*!< The flag "BUSY" for Encryption/Decryption of the current data request */
    bool inEncryption;
    /*!< The FREE MAC data indications queue */
    SYS_QueueDescriptor_t freeIndication;
    /*!< The USED MAC data indications queue */
    SYS_QueueDescriptor_t usedIndication;
    /*!< The array of MAC data indications to be used for incoming data processing */
    RF4CE_NWK_MACIndication_t queueIndication[RF4CE_NWK_MAX_INCOMING_PACKETS];
    /*!< The MAC data indication to be used for incoming packet decryption purposes */
    MAC_DataIndParams_t encryptionIndication;
    /*!< True if MAC soft start is performed */
    Bool8_t isMACSoftStart;
    Bool8_t isNWKStartUp;
    /*!< Previous value for the PHY phyTransmitPower. Need to be restored after the Key Seed sending. */
    PHY_TransmitPower_t previousTransmitPower;
#ifdef RF4CE_NO_NVM_DELAY_DISABLED
    Bool8_t inNVMRequest;
#endif /* RF4CE_NO_NVM_DELAY_DISABLED */
#ifdef ENABLE_RF4CE_LOGGER
    uint8_t logCounter;
    uint8_t logger[16];
#endif /* ENABLE_RF4CE_LOGGER */
} RF4CE_NWK_StaticData_t;

/************************* DEFINITIONS *************************************************/
#define RF4CE_NWK_STATIC_DATA_VAR_NAME rf4ceNWKStaticData

/**//**
 * \brief Common Stack Static Structure member.
 */
#define RF4CE_NWK_STATIC_DATA_FIELD()               RF4CE_NWK_StaticData_t RF4CE_NWK_StaticDataDataField;

/**//**
 * \brief Common Stack Static Structure member access.
 */
#define GET_RF4CE_NWK_STATIC_DATA_FIELD()   (&RF4CE_NWK_STATIC_DATA_VAR_NAME.RF4CE_NWK_StaticDataDataField)

/**//**
 * \brief Common Stack Static Structure member initialization.
 */
extern const SYS_SchedulerTaskHandler_t rf4ceNWKTaskHandlers[];
#ifdef RF4CE_NO_NVM_DELAY_DISABLED
#define RF4CE_NO_NVM_DELAY_INIT .inNVMRequest = false,
#else /* RF4CE_NO_NVM_DELAY_DISABLED */
#define RF4CE_NO_NVM_DELAY_INIT
#endif /* RF4CE_NO_NVM_DELAY_DISABLED */
#define INIT_RF4CE_NWK_STATIC_DATA_FIELD() \
.RF4CE_NWK_StaticDataDataField = \
{ \
    .NIB = \
    { \
        RF4CE_DEFAULT_NIB_VALUES() \
    }, \
    .nwkState = RF4CE_NWK_STATE_DORMANT, \
    .taskDescriptor = \
    { \
        .qElem = \
        { \
            .nextElement = NULL \
        }, \
        .priority = SYS_SCHEDULER_RF4CE_NWK_PRIORITY, \
        .handlers = rf4ceNWKTaskHandlers, \
        .handlersMask = 0, \
    }, \
    .setGetQueue = \
    { \
        .nextElement = NULL \
    }, \
    .dataRequestQueue = \
    { \
        .nextElement = NULL \
    }, \
    .dataIndicationQueue = \
    { \
        .nextElement = NULL \
    }, \
    .dataIndicationPacketsQueue = \
    { \
        .nextElement = NULL \
    }, \
    .startResetQueue = \
    { \
        .nextElement = NULL \
    }, \
    .rxEnableQueue = \
    { \
        .nextElement = NULL \
    }, \
    .rxEnableDeferredQueue = \
    { \
        .nextElement = NULL \
    }, \
    .discoveryQueue = \
    { \
        .nextElement = NULL \
    }, \
    .updateKeyQueue = \
    { \
       .nextElement = NULL \
    }, \
    .rxEnableRequest = NULL, \
    .setNIBReq = NULL, \
    .dataHandle = 0, \
    .securityRequest = \
    { \
        .a = \
        { \
            .block = MM_BAD_BLOCK_ID, \
        }, \
        .m = \
        { \
            .block = MM_BAD_BLOCK_ID, \
        }, \
        .nonce = \
        { \
            .block = MM_BAD_BLOCK_ID, \
        }, \
        .mic = \
        { \
            .block = MM_BAD_BLOCK_ID, \
        }, \
    }, \
    .inEncryption = false, \
    .pOutgoingPacket = NULL, \
    .freeIndication = \
    { \
        .nextElement = NULL, \
    }, \
    .usedIndication = \
    { \
        .nextElement = NULL, \
    }, \
    .isMACSoftStart = false, \
    .panId = RF4CE_ADDRESS_MASK, \
    .shortAddress = RF4CE_ADDRESS_MASK, \
    .setRequest = NULL, \
    .isNWKStartUp = true, \
    RF4CE_NO_NVM_DELAY_INIT \
    .isDiscoveryRequest = false, \
},

typedef struct _INT_RF4CE_NWK_StaticData_t
{
    RF4CE_NWK_STATIC_DATA_FIELD()
} INT_RF4CE_NWK_StaticData_t;

extern INT_RF4CE_NWK_StaticData_t RF4CE_NWK_STATIC_DATA_VAR_NAME;

#else /* USE_RF4CE_NWK */

#define RF4CE_NWK_STATIC_DATA_FIELD()
#define GET_RF4CE_NWK_STATIC_DATA_FIELD() NULL
#define INIT_RF4CE_NWK_STATIC_DATA_FIELD()

#endif /* USE_RF4CE_NWK */

#endif /* _RF4CE_NWK_STATIC_DATA_H */