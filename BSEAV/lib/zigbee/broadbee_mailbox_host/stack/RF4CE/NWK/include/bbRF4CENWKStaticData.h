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
 *      This is the header file for the RF4CE Network Layer component NIB Attributes static
 *      data declaration.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_STATIC_DATA_H
#define _RF4CE_NWK_STATIC_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"

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

#include "private/bbRF4CENWKPairUnpairCommon.h"
#include "private/bbRF4CENWKIndications.h"
#include "private/bbRF4CENWKSecurity.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK static data structure.
 */
typedef struct _RF4CE_NWK_StaticData_t
{
    RF4CE_NIB_Attributes_t NIB;                         /*!< Network Information Base */
    RF4CE_NWKState_t nwkState;                          /*!< Current Network State */
    MAC_Addr16bit_t shortAddress;                       /*!< Current network address */
    MAC_PanId_t panId;                                  /*!< Current network Pan ID */
    SYS_SchedulerTaskDescriptor_t taskDescriptor;       /*!< Tasks for network */
    RF4CE_NWK_SetReqDescr_t *setRequest;                /*!< NWK setRequest field */
    uint8_t lastResetType;                              /*!< Last reset type field */
    uint8_t keySeedFrameSequenceNumber;                 /*!< Key seed frame sequence number field */
    uint8_t safeChannelNumber;                          /*!< Safe channel number field */
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
    } macRequests;                                       /*!< MAC requests container */
    union
    {
        RF4CE_NWK_StartReqDescr_t *start;                /*!< Currently running START request */
        RF4CE_NWK_ResetReqDescr_t *reset;                /*!< Currently running RESET request */
    } nwkStartResetRequest;                              /*!< START/RESET request container */
    MAC_DataReqDescr_t *macUnpairRequest;                /*!< MAC Unpair request pointer */
    SYS_TimeoutTask_t timeoutTask;                       /*!< The timeout measuring task */
    SYS_TimeoutTask_t timeoutTaskData;                   /*!< The timeout task for DATA requests */
    SYS_TimeoutTask_t powerSavingTask;                   /*!< The timeout measuring for Power Saving task */
    RF4CE_NWK_RXEnableReqDescr_t psRXEnableReq;          /*!< Power Saving RX enable request */
    RF4CE_NWK_RXEnableReqDescr_t gpRXEnableReq;          /*!< General Purpose RX enable request */
    Bool8_t isDiscoveryRequest;                          /*!< Service field */
    SYS_DataPointer_t internalFrame;                     /*!< Internal data frame for MAC DATA requests */
    RF4CE_NWK_SetReqDescr_t *setNIBReq;                  /*!< Internal set NIB request */
    RF4CE_PairingTableEntry_t *temporaryPairing;         /*!< Pointer to the current pairing entry for the
                                                              currently pairing sequence */
    RF4CE_NWK_UnpairReqDescr_t *unpairRequest;           /*!< Currently running UNPAIR request */
    RF4CE_NWK_UnpairIndParams_t indicationUnpair;        /*!< Currently running UNPAIR request data indication */
    RF4CE_NWK_RXEnableReqDescr_t *rxEnableRequest;       /*!< Currently running RX-ENABLE request */
    RF4CE_NWK_OutgoingPacket_t *pOutgoingPacket;         /*!< Current Outgoing packet */
    MAC_RxEnableReqDescr_t macRXEnableRequest;           /*!< The MAC RX Enable request descriptor */
    uint8_t numDiscoveryRepetitions;                     /*!< The counter of the passed Discovery Request repetitions */
    SYS_DataPointer_t discoveryNodes;                    /*!< Discovered nodes collector for Discovery Confirmation */
    uint8_t numDiscoveryNodes;                           /*!< Number of nodes collected for Discovery Confirmation */
    union
    {
        RF4CE_NWK_DiscoveryReqDescr_t *discoveryRequest; /*!< Currently running DISCOVERY request */
        RF4CE_NWK_PairReqDescr_t *pairRequest;           /*!< Currently running PAIR request */
    } internalRequest;                                   /*!< Internal requests container */

    uint8_t broadcastTXChannel;                          /*!< Broadcast TX Channel */
    uint8_t dataTXResult;                                /*!< Data TX result */
    uint8_t discoveryStatus;                             /*!< Currently set discovery request status */
    uint8_t currentChannel;                              /*!< Currently set channel */
    uint8_t currentDataStatus;                           /*!< Currently set data request status */
#ifdef RF4CE_TARGET
    RF4CE_NWK_AutoDiscoveryReqDescr_t *autoDiscoveryRequest; /*!< Currently running AUTO Discovery request
                                                                  - for Target only */
    uint8_t nwkIndicateDiscoveryParameterBackup;         /*!< NWK indicate discovery backup */
    SYS_QueueDescriptor_t autoDiscoveryQueue;            /*!< Auto Discovery queue */
    RF4CE_NWK_DiscoveryRespDescr_t *discoveryResponse;   /*!< Currently running AUTO Discovery response descriptor
                                                              - for Target only */
    uint64_t discoveryIeeeAddr;                          /*!< Discovery IEEE address */
    MAC_DataReqDescr_t dataDiscoveryResponse;            /*!< MAC Data request for Discovery Response
                                                              - for Target only */
    RF4CE_NWK_AutoDiscoveryConfParams_t autoDiscoveryConfirm; /*!< Currently running AUTO Discovery request
                                                                   confirmation structure - for Target only */
    uint8_t requestFooter;                               /*!< Currently running Discovery Request footer to be stored
                                                              for compare - for Target only */
    uint8_t keySeedNeeded;                               /*!< Used for KEY SEED request counter - for Target only */
    SecurityKey_t securityKeySeed;                       /*!< Used for KEY SEED request security key storage
                                                              - for Target only */
    RF4CE_NLDE_DATA_Status_t pairingStatus;              /*!< Current pairing status - for Target only */
    SYS_TimeoutTask_t frequencyAgilityTask;              /*!< The timeout measuring for Frequency Agility task */
    Bool8_t isFrequencyAgilityOn;                        /*!< Indicates if Frequency Agility task is started */


    uint8_t energyLevels[3];                             /*!< Energy level array */
    uint8_t faCnt;                                       /*!< Frequency Agility counter */
    uint8_t currentPairProfileId[RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH]; /*!< This variable will handle the profile ID
                                                              according to supplied Pair Response which is necessary
                                                              for proper COMMStatusIndication handling in the
                                                              Profile Manager */
    uint8_t currentPairProfileIdLength;                  /*!< Pair profile ID length */
#ifdef RF4CE_NWK_GU_DISCOVERY
    uint8_t isSpecialAutoDiscoveryOn;
    uint8_t lenSpecialAutoDiscoveryDevices;
    uint8_t specialAutoDiscoveryDevices[3];
    uint8_t lenSpecialAutoDiscoveryProfiles;
    uint8_t specialAutoDiscoveryProfiles[7];
#endif /* RF4CE_NWK_GU_DISCOVERY */
#endif /* RF4CE_TARGET */
    uint8_t dataHandle;                                  /*!< Current request's dataHandle (being incremented
                                                              upon successful MAC DATA request) */
    SYS_QueueDescriptor_t setGetQueue;                   /*!< The NWK.GET NWK.SET requests queue */
    SYS_QueueDescriptor_t startResetQueue;               /*!< The NWK.START NWK.RESET requests queue */
    SYS_QueueDescriptor_t rxEnableQueue;                 /*!< The NWK.RX-Enable requests queue */
    RF4CE_NWK_OutgoingPacket_t outgoingPacket;           /*!< The only one existing outgoing packet */
    SYS_QueueDescriptor_t discoveryQueue;                /*!< The Discovery Requests queue */
    SYS_QueueDescriptor_t pairingQueue;                  /*!< The Pair/Unpair Requests queue */
    SYS_QueueDescriptor_t updateKeyQueue;                /*!< The Update Key Requests queue */
    SYS_QueueDescriptor_t dataRequestQueue;              /*!< The DATA Requests queue */
    Security_CCMReq_t securityRequest;                   /*!< The request for Encryption of the
                                                              current data request */
    bool inEncryption;                                   /*!< The flag "BUSY" for Encryption of the
                                                              current data request */

    RF4CE_RxServiceMem_t         rxMemDescr;             /*!< The momory for rf4ce rx service */
    RF4CE_DecryptionServiceMem_t decryptionMemory;       /*!< The momory for rf4ce decryption service */

    Bool8_t isMACSoftStart;                              /*!< True if MAC soft start is performed */
    Bool8_t isNWKStartUp;                                /*!< True if NWK is started up */
    PHY_TransmitPower_t previousTransmitPower;           /*!< Previous value for the PHY phyTransmitPower.
                                                              Need to be restored after the Key Seed sending. */
#ifdef RF4CE_NO_NVM_DELAY_DISABLED
    Bool8_t inNVMRequest;
#endif /* RF4CE_NO_NVM_DELAY_DISABLED */
#ifdef ENABLE_RF4CE_LOGGER
    uint8_t logCounter;
    uint8_t logger[16];
#endif /* ENABLE_RF4CE_LOGGER */

    RF4CE_NWK_PairingOriginatorMem_t pairingOriginatorMemDescr; /*!< Pairing originator mem descriptor */
    RF4CE_PairingTableEntry_t backupPairEntryOriginator; /*!< Backup Pairing reference */
    bool PairRefSavedOriginator;                         /*!< True if backup is valid */
#ifdef RF4CE_TARGET
    RF4CE_NWK_PairingRecipientMem_t pairingRecipientMemDescr;
    RF4CE_PairingTableEntry_t backupPairEntryRecipient;  /*!< Backup Pairing reference */
    bool PairRefSavedRecipient;                          /*!< True if backup is valid */
#endif
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
    .startResetQueue = \
    { \
        .nextElement = NULL \
    }, \
    .rxEnableQueue = \
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

#endif /* _RF4CE_NWK_STATIC_DATA_H */

/* eof bbRF4CENWKStaticData.h */
