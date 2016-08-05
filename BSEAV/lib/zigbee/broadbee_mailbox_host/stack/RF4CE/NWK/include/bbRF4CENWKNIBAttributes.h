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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKNIBAttributes.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer component NIB Attributes handlers.
 *
 * $Revision: 12956 $
 * $Date: 2016-07-14 01:20:40Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_NIB_ATTRIBUTES_H
#define _RF4CE_NWK_NIB_ATTRIBUTES_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKRequestService.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief NWK NIB Attributes IDs.
  */
typedef enum _RF4CE_NWK_NIB_Ids_t
{
    RF4CE_NWK_ACTIVE_PERIOD = 0x60,            /*!< The active period of a device in MAC symbols. */
    RF4CE_NWK_BASE_CHANNEL,                    /*!< The logical channel that was chosen during device initialization. */
    RF4CE_NWK_DISCOVERY_LQI_THRESHOLD,         /*!< The LQI threshold below which discovery requests will be rejected. */
    RF4CE_NWK_DISCOVERY_REPETITION_INTERVAL,   /*!< The interval, in MAC symbols, at which discovery attempts are made on
                                                    all channels. */
    RF4CE_NWK_DUTY_CYCLE,                      /*!< The duty cycle of a device in MAC symbols. A value of 0x000000
                                                    indicates the device is not using RF4CE power saving and the
                                                    application must provide this functionality directly. */
    RF4CE_NWK_FRAME_COUNTER,                   /*!< The frame counter added to the transmitted NPDU. */
    RF4CE_NWK_INDICATE_DISCOVERY_REQUESTS,     /*!< Indicates whether the NLME indicates the reception of discovery
                                                    request command frames to the application. TRUE indicates that the
                                                    NLME notifies the application. */
    RF4CE_NWK_IN_POWER_SAVE,                   /*!< The power save mode of the node. TRUE indicates that the device is
                                                    operating in power save mode. */
    RF4CE_NWK_PAIRING_TABLE,                   /*!< The pairing table managed by the device. */
    RF4CE_NWK_MAX_DISCOVERY_REPETITIONS,       /*!< The maximum number of discovery attempts made at the
                                                    nwkDiscovery-RepetitionInterval rate. Note that when auto discovery
                                                    mode is being used on a device that wants to be discovered, this
                                                    value should be >= 2 on the device initiating the discovery process. */
    RF4CE_NWK_MAX_FIRST_ATTEMPT_CSMA_BACKOFFS, /*!< The maximum number of backoffs the MAC CSMA-CA algorithm will
                                                    attempt before declaring a channel access failure for the first
                                                    transmission attempt. */
    RF4CE_NWK_MAX_FIRST_ATTEMPT_FRAME_RETRIES, /*!< The maximum number of MAC retries allowed after a transmission
                                                    failure for the first transmission attempt. */
    RF4CE_NWK_MAX_REPORTED_NODE_DESCRIPTORS,   /*!< The maximum number of node descriptors that can be obtained before
                                                    reporting to the application. */
    RF4CE_NWK_RESPONSE_WAIT_TIME,              /*!< The maximum time in MAC symbols, a device shall wait for a response
                                                    command frame following a request command frame. */
    RF4CE_NWK_SCAN_DURATION,                   /*!< A measure of the duration of a scanning operation. */
    RF4CE_NWK_USER_STRING,                     /*!< The user defined character string used to identify this node. */
    /* Custom member(s) */
    RF4CE_NWK_MAX_PAIRING_TABLE_ENTRIES,       /*!< Maximum amount of pairing table entries available. */
    RF4CE_NWK_PAN_ID,                          /*!< Pan ID. */
    RF4CE_NWK_CHANNEL_NORMALIZATION_CAPABLE,   /*!< Node capabilities: If the node is channel normalization capable. */
    RF4CE_NWK_SECURITY_CAPABLE,                /*!< Node capabilities: If the node is security capable. */
    RF4CE_NWK_POWER_SOURCE,                    /*!< Node capabilities: If the node is powered from mains. */
    RF4CE_NWK_FA_SCAN_THRESHOLD,
    RF4CE_NWK_NUM_SUPPORTED_PROFILES,
    RF4CE_NWK_ANTENNA_AVAILABLE,
    RF4CE_NWK_SUPPORTED_PROFILES,
    RF4CE_NWK_TX_POWER_KEY_EXCHANGE,           /*!< Tx Power for the key seed tranmission */
    RF4CE_NWK_FA_COUNT_THRESHOLD,              /*!<  Frequency Agility Count Threshold */
    RF4CE_NWK_FA_DECREMENT,                    /*!<  Frequency Agility Decrement */
    RF4CE_NWK_ATTRIBUTE_MAXIMUM
} RF4CE_NWK_NIB_Ids_t;

#if RF4CE_NWK_ATTRIBUTE_MAXIMUM > 0x7f
    #error ('Too many RF4CE NWK attributes')
#endif

/**//**
 * \brief NWK NIB Pairing entry helper macros.
  */
#define SET_RF4CE_PAIRING_ENTRY_USED(pairingEntry) \
    (pairingEntry)->dstLogicalChannel |= RF4CE_PAIRING_BUSY
#define SET_RF4CE_PAIRING_ENTRY_UNUSED(pairingEntry) \
    (pairingEntry)->dstLogicalChannel &= ~RF4CE_PAIRING_BUSY
#define IS_RF4CE_PAIRING_ENTRY_USED(pairingEntry) \
    (0 != ((pairingEntry)->dstLogicalChannel & RF4CE_PAIRING_BUSY))
#define GET_RF4CE_PAIRING_ENTRY_CHANNEL(pairingEntry) \
    ((pairingEntry)->dstLogicalChannel & ~RF4CE_PAIRING_BUSY)


/************************* TYPES *******************************************************/
/**//**
 * \brief 128-bit security key type.
  */
typedef uint8_t SecurityKey_t[RF4CE_SECURITY_KEY_LENGTH];

/**//**
 * \brief RF4CE Pairing Table entry format.
 */
typedef struct _RF4CE_PairingTableEntry_t
{
    SecurityKey_t key;          /*!< The link key to be used to secure this pairing link. */
    uint64_t dstIeeeAddr;       /*!< The IEEE address of the destination device. */
    uint32_t rcpFrameCounter;   /*!< The frame counter last received from the recipient node. */
    uint16_t srcNetAddr;        /*!< The network address to be assumed by the source device. */
    uint16_t dstPanId;          /*!< The PAN identifier of the destination device. */
    uint16_t dstNetAddr;        /*!< The network address of the destination device. */
    uint8_t dstLogicalChannel;  /*!< The expected channel of the destination device. */
    uint8_t rcpCapabilities;    /*!< The node capabilities of the recipient node. */
} RF4CE_PairingTableEntry_t;

/**//**
 * \brief Internal NIB attributes for the RF4CE interface data structure.
 */
typedef struct _RF4CE_NIB_Attributes_t
{
    struct
    {
        uint32_t nwkFrameCounter;                        /*!< The frame counter added to the transmitted NPDU. */
    } frameCounter;

    struct
    {
        uint8_t nwkBaseChannel;                          /*!< The logical channel that was chosen during device
                                                              initialization. */
        uint8_t nwkInPowerSave;                          /*!< The power save mode of the node. TRUE indicates that the
                                                              device is operating in power save mode. */
    } autoNIB;

    struct
    {
        uint32_t nwkDiscoveryRepetitionInterval;         /*!< The interval, in MAC symbols, at which discovery attempts are
                                                              made on all channels. */
        uint8_t nwkMaxDiscoveryRepetitions;              /*!< The maximum number of discovery attempts made at the
                                                              nwkDiscovery-RepetitionInterval rate. Note that when auto
                                                              discovery mode is being used on a device that wants to be
                                                              discovered, this value should be ≥ 2 on the device initiating
                                                              the discovery process. */
        uint8_t nwkMaxReportedNodeDescriptors;           /*!< The maximum number of node descriptors that can be obtained
                                                              before reporting to the application. */
        int8_t  nwkFaScanThreshold;
        uint8_t  nwkFaCountThreshold;
        int8_t  nwkFaDecrement;
    } nonStorable;

    struct
    {
        RF4CE_PairingTableEntry_t nwkPairingTable[RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES + 1]; /*!< The pairing table managed by
                                                                                                  the device. */
        uint8_t nwkUserString[RF4CE_NWK_USER_STRING_LENGTH];/*!< The user defined character string used to identify this
                                                                 node. */
        uint32_t nwkActivePeriod;                        /*!< The active period of a device in MAC symbols. */
        uint32_t nwkDutyCycle;                           /*!< The duty cycle of a device in MAC symbols. A value of 0x000000
                                                              indicates the device is not using RF4CE power saving and the
                                                              application must provide this functionality directly. */
        uint32_t nwkResponseWaitTime;                    /*!< The maximum time in MAC symbols, a device shall wait for a
                                                              response command frame following a request command frame. */
        uint8_t nwkIndicateDiscoveryRequests;            /*!< Indicates whether the NLME indicates the reception of
                                                              discovery request command frames to the application. TRUE
                                                              indicates that the NLME notifies the application. */
        uint8_t nwkScanDuration;                         /*!< A measure of the duration of a scanning operation. */
        uint8_t nwkMaxFirstAttemptFrameRetries;          /*!< The maximum number of MAC retries allowed after a transmission
                                                              failure for the first transmission attempt. */
        uint8_t nwkMaxFirstAttemptCSMABackoffs;          /*!< The maximum number of backoffs the MAC CSMA-CA algorithm will
                                                              attempt before declaring a channel access failure for the
                                                              first transmission attempt. */
        uint8_t nwkNodeCapabilities;                     /*!< Current node capabilities */
        uint8_t nwkDiscoveryLQIThreshold;                /*!< The LQI threshold below which discovery requests will be
                                                              rejected. */
        int8_t  nwkTxPowerKeyExchange;                   /*!< Tx Power during Key Exchange. */
    } storable;
} RF4CE_NIB_Attributes_t;

/**//**
 * \brief NIB attribute for the RF4CE interface data union.
 */
typedef union _RF4CE_NIB_AttributesAll_t
{
    uint32_t nwkActivePeriod;                        /*!< The active period of a device in MAC symbols. */
    uint32_t nwkDiscoveryRepetitionInterval;         /*!< The interval, in MAC symbols, at which discovery attempts are
                                                          made on all channels. */
    uint32_t nwkDutyCycle;                           /*!< The duty cycle of a device in MAC symbols. A value of 0x000000
                                                          indicates the device is not using RF4CE power saving and the
                                                          application must provide this functionality directly. */
    uint32_t nwkFrameCounter;                        /*!< The frame counter added to the transmitted NPDU. */
    uint32_t nwkResponseWaitTime;                    /*!< The maximum time in MAC symbols, a device shall wait for a
                                                          response command frame following a request command frame. */
    uint8_t nwkMaxFirstAttemptFrameRetries;          /*!< The maximum number of MAC retries allowed after a transmission
                                                          failure for the first transmission attempt. */
    uint8_t nwkBaseChannel;                          /*!< The logical channel that was chosen during device
                                                          initialization. */
    uint8_t nwkIndicateDiscoveryRequests;            /*!< Indicates whether the NLME indicates the reception of
                                                          discovery request command frames to the application. TRUE
                                                          indicates that the NLME notifies the application. */
    uint8_t nwkInPowerSave;                          /*!< The power save mode of the node. TRUE indicates that the
                                                          device is operating in power save mode. */
    uint8_t nwkMaxFirstAttemptCSMABackoffs;          /*!< The maximum number of backoffs the MAC CSMA-CA algorithm will
                                                          attempt before declaring a channel access failure for the
                                                          first transmission attempt. */
    uint8_t nwkScanDuration;                         /*!< A measure of the duration of a scanning operation. */
    uint8_t nwkDiscoveryLQIThreshold;                /*!< The LQI threshold below which discovery requests will be
                                                          rejected. */
    RF4CE_PairingTableEntry_t nwkPairingTableEntry;  /*!< The pairing table entry managed by the device. */
    uint8_t nwkMaxDiscoveryRepetitions;              /*!< The maximum number of discovery attempts made at the
                                                          nwkDiscovery-RepetitionInterval rate. Note that when auto
                                                          discovery mode is being used on a device that wants to be
                                                          discovered, this value should be ≥ 2 on the device initiating
                                                          the discovery process. */
    uint8_t nwkMaxReportedNodeDescriptors;           /*!< The maximum number of node descriptors that can be obtained
                                                          before reporting to the application. */
    uint8_t nwkUserString[RF4CE_NWK_USER_STRING_LENGTH];/*!< The user defined character string used to identify this
                                                          node. */
    uint8_t nwkMaxPairingTableEntries;               /*!< The maximum number of pairing table entries. Read only. */
    uint16_t nwkPanId;                               /*!< Pan ID */
    uint8_t nwkChannelNormalization;                 /*!< Node capabilities: If the node is channel normalization capable. */
    uint8_t nwkSecurityCapable;                      /*!< Node capabilities: If the node is security capable. */
    uint8_t nwkPowerSource;                          /*!< Node capabilities: If the node is powered from mains. */
    int8_t nwkFaScanThreshold;                      /*!< Extension of constant RF4CE_NWKC_FA_SCAN_THRESHOLD */
    uint8_t nwkNumSupportedProfiles;
    uint8_t antennaAvailable;
    uint8_t nwkSupportedProfiles[RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH];
    int8_t  nwkTxPowerKeyExchange;                   /*!< Tx Power during Key Exchange. */
    uint8_t nwkFaCountThreshold;                     /*!< Self-defined attribute for RF4CE_NWKC_FA_COUNT */
    int8_t nwkFaDecrement;                           /*!< Self-defined attribute for RF4CE_NWKC_FA_DECREMENT */
 } RF4CE_NIB_AttributesAll_t;

/**//**
 * \brief NLME-SET identification structure declaration.
 */
typedef struct _RF4CE_NWK_AttributeID_t
{
    uint8_t attrId;                     /*!< The identifier of the NIB attribute that was written. */
    uint8_t attrIndex;                  /*!< The index within the table or array of the specified NIB attribute that was
                                             written. This parameter is valid only for NIB attributes that are tables or
                                             arrays. */
} RF4CE_NWK_AttributeID_t;

/**//**
 * \brief NLME-SET confirm primitive's parameters structure declaration.
 */
typedef struct _RF4CE_NWK_SetConfParams_t
{
    uint8_t status;                     /*!< The result of the request to write the NIB attribute. */
    RF4CE_NWK_AttributeID_t attrId;     /*!< NIB attribute identification. */
} RF4CE_NWK_SetConfParams_t;

/**//**
 * \brief NLME-SET request primitive's parameters structure declaration.
 */
typedef struct _RF4CE_NWK_SetReqParams_t
{
    RF4CE_NWK_AttributeID_t attrId;     /*!< NIB attribute identification. */
    RF4CE_NIB_AttributesAll_t data;     /*!< New attribute's value. */
} RF4CE_NWK_SetReqParams_t;

/**//**
 * \brief NLME-SET request structure data type declaration.
 */
typedef struct _RF4CE_NWK_SetReqDescr_t RF4CE_NWK_SetReqDescr_t;

/**//**
 * \brief NLME-SET request callback declaration.
 */
typedef void (*RF4CE_NWK_SetConfCallback_t)(RF4CE_NWK_SetReqDescr_t *req, RF4CE_NWK_SetConfParams_t *conf);

/**//**
 * \brief NLME-SET request structure declaration.
 */
typedef struct _RF4CE_NWK_SetReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;   /*!< Service field */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_SetReqParams_t params;   /*!< Request containing structure */
    RF4CE_NWK_SetConfCallback_t callback; /*!< Callback for confirmation. */
} RF4CE_NWK_SetReqDescr_t;

/**//**
 * \brief NLME-GET confirm primitive's parameters structure declaration.
 */
typedef struct _RF4CE_NWK_GetConfParams_t
{
    uint8_t status;                     /*!< The result of the request to read the NIB attribute. */
    RF4CE_NWK_AttributeID_t attrId;     /*!< NIB attribute identification. */
    RF4CE_NIB_AttributesAll_t data;     /*!< The requested data. */
} RF4CE_NWK_GetConfParams_t;

/**//**
 * \brief NLME-GET request primitive's parameters structure declaration.
 */
typedef struct _RF4CE_NWK_GetReqParams_t
{
    RF4CE_NWK_AttributeID_t attrId;     /*!< NIB attribute identification. */
} RF4CE_NWK_GetReqParams_t;

/**//**
 * \brief NLME-GET request structure data type declaration.
 */
typedef struct _RF4CE_NWK_GetReqDescr_t RF4CE_NWK_GetReqDescr_t;

/**//**
 * \brief NLME-GET request callback declaration.
 */
typedef void (*RF4CE_NWK_GetConfCallback_t)(RF4CE_NWK_GetReqDescr_t *req, RF4CE_NWK_GetConfParams_t *conf);

/**//**
 * \brief NLME-GET request primitive's parameters structure declaration.
 */
typedef struct _RF4CE_NWK_GetReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;   /*!< Service field */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_GetReqParams_t params;   /*!< Request containing structure */
    RF4CE_NWK_GetConfCallback_t callback; /*!< Callback for confirmation. */
} RF4CE_NWK_GetReqDescr_t;

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Default NIB values assignment macro.
 */
#ifdef RF4CE_TARGET
#    define RF4CE_DEFAULT_NODE_CAPS \
        .nwkNodeCapabilities = RF4CE_NWKC_NODE_CAPABILITIES | RF4CE_NWK_NODE_TARGET
#else /* RF4CE_TARGET */
#    define RF4CE_DEFAULT_NODE_CAPS \
        .nwkNodeCapabilities = RF4CE_NWKC_NODE_CAPABILITIES & (~RF4CE_NWK_NODE_TARGET)
#endif /* RF4CE_TARGET */

#define RF4CE_DEFAULT_NONSTORABLE_NIB_VALUES() \
    .nwkDiscoveryRepetitionInterval = 0x0030d4, \
    .nwkMaxDiscoveryRepetitions = 1, \
    .nwkMaxReportedNodeDescriptors = 3,\
    .nwkFaScanThreshold = RF4CE_NWKC_FA_SCAN_THRESHOLD, \
    .nwkFaCountThreshold = RF4CE_NWK_FREQUENCY_AGILITY_COUTER, \
    .nwkFaDecrement = RF4CE_NWK_FREQUENCY_AGILITY_DECREMENT,

#define RF4CE_DEFAULT_STORABLE_NIB_VALUES() \
    .nwkActivePeriod = RF4CE_NWKC_MIN_ACTIVE_PERIOD, \
    .nwkDutyCycle = 0, \
    .nwkResponseWaitTime = 0x00186a, \
    .nwkMaxFirstAttemptFrameRetries = 3, \
    .nwkIndicateDiscoveryRequests = 0, \
    .nwkMaxFirstAttemptCSMABackoffs = 4, \
    .nwkScanDuration = 6, \
    .nwkDiscoveryLQIThreshold = 0xff, \
    .nwkTxPowerKeyExchange = RF4CE_NWKC_SEC_CMD_TX_POWER, \
    RF4CE_DEFAULT_NODE_CAPS,

#define RF4CE_DEFAULT_AUTO_NIB_VALUES() \
    .nwkBaseChannel = 15, \
    .nwkInPowerSave = 1,

#define RF4CE_DEFAULT_NIB_VALUES() \
    .frameCounter = \
    { \
        .nwkFrameCounter = 0, \
    }, \
    .nonStorable = \
    { \
        RF4CE_DEFAULT_NONSTORABLE_NIB_VALUES() \
    }, \
    .autoNIB = \
    { \
        RF4CE_DEFAULT_AUTO_NIB_VALUES() \
    }, \
    .storable = \
    { \
        RF4CE_DEFAULT_STORABLE_NIB_VALUES() \
    },

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to set appropriate NIB attribute.

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_SetReq(RF4CE_NWK_SetReqDescr_t *request);

/************************************************************************************//**
 \brief Initiates asynchronous procedure to get appropriate NIB attribute.

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_GetReq(RF4CE_NWK_GetReqDescr_t *request);

#endif /* _RF4CE_NWK_NIB_ATTRIBUTES_H */