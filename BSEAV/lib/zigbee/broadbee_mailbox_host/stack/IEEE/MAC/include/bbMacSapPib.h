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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC-PIB for MAC-SAP definitions.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_PIB_H
#define _BB_MAC_SAP_PIB_H

/************************* INCLUDES ***********************************************************************************/
#include "bbMacSapDefs.h"
#include "bbMacSapAddress.h"
#include "bbMacSapSecurity.h"
#include "bbMacSapService.h"
#include "bbHalRandom.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Enumeration of identifiers of private MAC-PIB attributes.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.4.2, 7.6.1, tables 86, 88.
 */
typedef enum _MacPibAttributeId_t {

#if defined(_HAL_USE_PRNG_)
    MAC_ATTRIBUTES_ID_BEGIN             = 0x3B,     /*!< First ID of MAC-PIB attributes. PRNG is used. */
#else
    MAC_ATTRIBUTES_ID_BEGIN             = 0x3D,     /*!< First ID of MAC-PIB attributes. TRNG is used. */
#endif


    /* Additional MAC-PIB attributes for Pseudo-Random Number Generator (PRNG). */

    MAC_PRNG_SEED                       = 0x3B,     /*!< Seed value of the PRNG. Read to obtain the value with which the
                                                        PRNG was initialized. Write to assign new seed value an
                                                        initialize the PRNG. Write the same value to just reinitialize
                                                        the PRNG.*/

    MAC_PRNG_COUNTER                    = 0x3C,     /*!< Counter value of the PRNG. Read to obtain the current value of
                                                        the PRNG Counter. Write to forward or rewind the PRNG to the
                                                        specified Counter value. Write zero to reinitialize the PRNG
                                                        with its Seed value. */


    /* Additional MAC-PIB attributes for dual context runtime switching on/off. */

    MAC_CONTEXT_ENABLED                 = 0x3D,     /*!< Indicates if the specified MAC context is enabled. The default
                                                        value is FALSE. The only allowed value in MLME-SET.request is
                                                        TRUE (i.e., the context may be enabled after the hardware reset,
                                                        but may not be disabled). This attribute is not reset to the
                                                        default value during MLME-RESET.request (i.e., the context may
                                                        not be disabled other then to reset the hardware). */


    /* Additional MAC-PIB attributes for address filtering hardware support. */

    MAC_PAN_COORDINATOR                 = 0x3E,     /*!< Indicates if the MAC sublayer belongs to the PAN coordinator
                                                        NWK device and shall accept valid frames without destination
                                                        address in the MHR (i.e., the address mode equals to 0). If this
                                                        attribute is set to FALSE the MAC will reject frames without
                                                        destination address. The default value is TRUE to be consistent
                                                        with the frames filtering and rejection rules defined by
                                                        IEEE 802.15.4-2006. This attribute is also assigned by
                                                        MLME-START.request. */

    MAC_EXTENDED_ADDRESS                = 0x3F,     /*!< The 64-bit (IEEE) address assigned to the device. The default
                                                        value is 0. Allowed values are 0 - 0xFFFF'FFFF'FFFF'FFFF. This
                                                        attribute is not changed or reset to the default value during
                                                        MLME-RESET.request, so there is no need to restore its value
                                                        after MAC reset. This attribute is allowed to be reassigned an
                                                        unlimited number of times after hardware reset. MAC does not
                                                        restrict any activities even if this attribute is not
                                                        initialized to a legal value by the application. For the dual
                                                        context MAC the 64-bit addresses of different contexts are
                                                        allowed to be equal. This attribute value is accessible also as
                                                        the MAC constant \e aExtendedAddress.*/


    /* Standard MAC-PIB attributes according to IEEE 802.15.4-2006 except MAC Security attributes. */

    MAC_ACK_WAIT_DURATION               = 0x40,     /*!< The maximum number of symbols to wait for an acknowledgment
                                                        frame to arrive following a transmitted data frame. */

    MAC_ASSOCIATION_PERMIT              = 0x41,     /*!< Indication of whether a coordinator is currently allowing
                                                        association. */

    MAC_AUTO_REQUEST                    = 0x42,     /*!< Indication of whether a device automatically sends a data
                                                        request command if its address is listed in the beacon frame. */

    MAC_BATT_LIFE_EXT                   = 0x43,     /*!< Indication of whether battery life extension, by reduction of
                                                        coordinator receiver operation time during the CAP,
                                                        is enabled. */

    MAC_BATT_LIFE_EXT_PERIODS           = 0x44,     /*!< The number of backoff periods during which the receiver is
                                                        enabled following a beacon in battery life extension mode. */

    MAC_BEACON_PAYLOAD                  = 0x45,     /*!< The contents of the beacon payload. */

    MAC_BEACON_PAYLOAD_LENGTH           = 0x46,     /*!< The length, in octets, of the beacon payload. */

    MAC_BEACON_ORDER                    = 0x47,     /*!< Specification of how often the coordinator transmits
                                                        a beacon. */

    MAC_BEACON_TX_TIME                  = 0x48,     /*!< The time that the device transmitted its last beacon frame,
                                                        in symbol periods. */

    MAC_BSN                             = 0x49,     /*!< The sequence number added to the transmitted beacon frame. */

    MAC_COORD_EXTENDED_ADDRESS          = 0x4A,     /*!< The 64 bit address of the coordinator with which the device is
                                                        associated. */

    MAC_COORD_SHORT_ADDRESS             = 0x4B,     /*!< The 16 bit short address assigned to the coordinator with which
                                                        the device is associated. */

    MAC_DSN                             = 0x4C,     /*!< The sequence number added to the transmitted data or MAC
                                                        command frame. */

    MAC_GTS_PERMIT                      = 0x4D,     /*!< TRUE if the PAN coordinator is to accept GTS requests.
                                                        FALSE otherwise. */

    MAC_MAX_CSMA_BACKOFFS               = 0x4E,     /*!< The maximum number of backoffs the CSMA-CA algorithm will
                                                        attempt before declaring a channel access failure. */

    MAC_MIN_BE                          = 0x4F,     /*!< The minimum value of the backoff exponent in the CSMA-CA
                                                        algorithm. */

    MAC_PAN_ID                          = 0x50,     /*!< The 16 bit identifier of the PAN on which the device
                                                        is operating. */

    MAC_PROMISCUOUS_MODE                = 0x51,     /*!< This indicates whether the MAC sublayer is in a promiscuous
                                                        (receive all) mode. */

    MAC_RX_ON_WHEN_IDLE                 = 0x52,     /*!< This indicates whether the MAC sublayer is to enable its
                                                        receiver during idle periods. */

    MAC_SHORT_ADDRESS                   = 0x53,     /*!< The 16 bit address that the device uses to communicate
                                                        in the PAN. */

    MAC_SUPERFRAME_ORDER                = 0x54,     /*!< This specifies the length of the active portion of the
                                                        superframe, including the beacon frame. */

    MAC_TRANSACTION_PERSISTENCE_TIME    = 0x55,     /*!< The maximum time (in superframe periods) that a transaction  is
                                                        stored by a coordinator and indicated in its beacon. */

    MAC_ASSOCIATED_PAN_COORD            = 0x56,     /*!< Indication of whether the device is associated to the PAN
                                                        through the PAN coordinator. */

    MAC_MAX_BE                          = 0x57,     /*!< The maximum value of the backoff exponent, BE, in the
                                                        CSMA-CA algorithm. */

    MAC_MAX_FRAME_TOTAL_WAIT_TIME       = 0x58,     /*!< The maximum number of CAP symbols in a beacon-enabled PAN, or
                                                        symbols in a nonbeacon-enabled PAN, to wait either for a frame
                                                        intended as a response to a data request frame or for a
                                                        broadcast frame following a beacon with the Frame Pending
                                                        subfield set to one. */

    MAC_MAX_FRAME_RETRIES               = 0x59,     /*!< The maximum number of retries allowed after a transmission
                                                        failure. */

    MAC_RESPONSE_WAIT_TIME              = 0x5A,     /*!< The maximum time, in multiples of aBaseSuperframeDuration, a
                                                        device shall wait for a response command frame to be available
                                                        following a request command frame. */

    MAC_SYNC_SYMBOL_OFFSET              = 0x5B,     /*!< The offset, measured in symbols, between the symbol boundary at
                                                        which the MLME captures the timestamp of each transmitted or
                                                        received frame, and the onset of the first symbol past the SFD,
                                                        namely, the first symbol of the Length field. */

    MAC_TIMESTAMP_SUPPORTED             = 0x5C,     /*!< Indication of whether the MAC sublayer supports the optional
                                                        timestamping feature for incoming and outgoing data frames. */

    MAC_SECURITY_ENABLED                = 0x5D,     /*!< Indication of whether the MAC sublayer has security enabled. */

    MAC_MIN_LIFS_PERIOD                 = 0x5E,     /*!< The minimum number of symbols forming a LIFS period.
                                                        NOTE: This attribute ID is not defined by the standard.
                                                        It has been defined by Broadcom on technical support request.
                                                        https://support.broadcom.com/IMS/Main.aspx?IssueID=718250 */

    MAC_MIN_SIFS_PERIOD                 = 0x5F,     /*!< The minimum number of symbols forming a SIFS period.
                                                        NOTE: This attribute ID is not defined by the standard.
                                                        It has been defined by Broadcom on technical support request.
                                                        https://support.broadcom.com/IMS/Main.aspx?IssueID=718250 */

    MAC_ATTRIBUTES_ID_END               = 0x5F,     /*!< Last ID of MAC-PIB private attributes, excluding security. */


#if defined(_MAC_CONTEXT_ZBPRO_)

    /* Additional MAC-PIB attributes for need of Thread mode. */

    MAC_SECURITY_ATTRIBUTES_ID_BEGIN    = 0x70,     /*!< First ID of MAC-PIB Security attributes. */

    MAC_THREAD_MODE                     = 0x70,     /*!< Mode of MAC beaconing. If TRUE, Thread mode of beaconing is
                                                        turned on, MAC emits beacon from the extended source address
                                                        irrespectively of the directive to use the short address (i.e.,
                                                        even if its short address is in the range 0x0000..0xFFFD). If
                                                        FALSE, ZigBee (standard) mode of beaconing is selected, MAC
                                                        behaves according to the standard. */


    /* Standard MAC-PIB Security attributes according to IEEE 802.15.4-2006. */

    MAC_KEY_TABLE                       = 0x71,     /*!< A table of KeyDescriptor entries, each containing keys and
                                                        related information required for secured communications. */

    MAC_KEY_TABLE_ENTRIES               = 0x72,     /*!< The number of entries in macKeyTable. */

    MAC_DEVICE_TABLE                    = 0x73,     /*!< A table of DeviceDescriptor entries, each indicating a remote
                                                        device with which this device securely communicates. */

    MAC_DEVICE_TABLE_ENTRIES            = 0x74,     /*!< The number of entries in macDeviceTable. */

    MAC_SECURITY_LEVEL_TABLE            = 0x75,     /*!< A table of SecurityLevelDescriptor entries, each with
                                                        information about the minimum security level expected depending
                                                        on incoming frame type and subtype. */

    MAC_SECURITY_LEVEL_TABLE_ENTRIES    = 0x76,     /*!< The number of entries in macSecurityLevelTable. */

    MAC_FRAME_COUNTER                   = 0x77,     /*!< The outgoing frame counter for this device. */

    MAC_AUTO_REQUEST_SECURITY_LEVEL     = 0x78,     /*!< The security level used for automatic data requests. */

    MAC_AUTO_REQUEST_KEY_ID_MODE        = 0x79,     /*!< The key identifier mode used for automatic data requests. This
                                                        attribute is invalid if the macAutoRequestSecurityLevel
                                                        attribute is set to 0x00. */

    MAC_AUTO_REQUEST_KEY_SOURCE         = 0x7A,     /*!< The originator of the key used for automatic data requests.
                                                        This attribute is invalid if the macAutoRequestKeyIdMode element
                                                        is invalid or set to 0x00. */

    MAC_AUTO_REQUEST_KEY_INDEX          = 0x7B,     /*!< The index of the key used for automatic data requests. This
                                                        attribute is invalid if the macAutoRequestKeyIdMode attribute is
                                                        invalid or set to 0x00. */

    MAC_DEFAULT_KEY_SOURCE              = 0x7C,     /*!< The originator of the default key used for key identifier mode
                                                        0x01. */

    MAC_PAN_COORD_EXTENDED_ADDRESS      = 0x7D,     /*!< The 64-bit address of the PAN coordinator. */

    MAC_PAN_COORD_SHORT_ADDRESS         = 0x7E,     /*!< The 16-bit short address assigned to the PAN coordinator. A
                                                        value of 0xFFFE indicates that the PAN coordinator is only using
                                                        its 64-bit extended address. A value of 0xFFFF indicates that
                                                        this value is unknown. */

    MAC_SECURITY_ATTRIBUTES_ID_END      = 0x7E,     /*!< Last ID of MAC-PIB Security attributes. */

#endif /* _MAC_CONTEXT_ZBPRO_ */


    /* Mirrored PHY attributes for needs of dual-context MAC. */

    MAC_CURRENT_CHANNEL                 = PHY_CURRENT_CHANNEL,      /*!< The RF channel to use for all following
                                                                        transmissions and receptions. */

    MAC_CURRENT_PAGE                    = PHY_CURRENT_PAGE,         /*!< The current PHY channel page. */

} MacPibAttributeId_t;

/**//**
 * \brief   Enumeration of identifiers of public MAC-PIB attributes.
 * \details MAC-PIB public attributes set includes MAC-PIB private attributes subset and PHY-PIB public attributes set.
 *  MAC private and PHY public attributes identifiers ranges do not intersect with each other.
 * \note Enumeration \c PHY_PibAttributeId_t has the same data size (8-bit) as \c MacPibAttributeId_t and consequently
 *  \c MAC_PibAttributeId_t.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.4.2, 7.6.1, tables 86, 88.
 */
typedef MacPibAttributeId_t  MAC_PibAttributeId_t;
SYS_DbgAssertStatic(sizeof(MAC_PibAttributeId_t) == sizeof(PHY_PibAttributeId_t));

/**//**
 * \name    Additional data types for private MAC-PIB attributes.
 */
/**@{*/
#if defined(_HAL_USE_PRNG_)
typedef HAL_PrngSeed_t          MAC_PrngSeed_t;                     /*!< Data type for macPrngSeed. */
typedef HAL_PrngCounter_t       MAC_PrngCounter_t;                  /*!< Data type for macPrngCounter. */
#endif
typedef Bool8_t                 MAC_ContextEnabled_t;               /*!< Data type for macContextEnabled. */
typedef Bool8_t                 MAC_PanCoordinator_t;               /*!< Data type for macPanCoordinator. */
typedef MAC_Addr64bit_t         MAC_ExtendedAddress_t;              /*!< Data type for macExtendedAddress. */
typedef uint8_t                 MAC_AckWaitDuration_t;              /*!< Data type for macAckWaitDuration. */
typedef Bool8_t                 MAC_AssociationPermit_t;            /*!< Data type for macAssociationPermit. */
typedef Bool8_t                 MAC_AutoRequest_t;                  /*!< Data type for macAutoRequest. */
typedef Bool8_t                 MAC_BattLifeExt_t;                  /*!< Data type for macBattLifeExt. */
typedef uint8_t                 MAC_BattLifeExtPeriods_t;           /*!< Data type for macBattLifeExtPeriods. */
typedef SYS_DataPointer_t       MAC_BeaconPayload_t;                /*!< Data type for macBeaconPayload. */
typedef uint8_t                 MAC_BeaconPayloadLength_t;          /*!< Data type for macBeaconPayloadLength. */
typedef uint8_t                 MAC_BeaconOrder_t;                  /*!< Data type for macBeaconOrder. */
typedef HAL_Symbol__Tstamp_t    MAC_BeaconTxTime_t;                 /*!< Data type for macBeaconTxTime. */
/* See bbMacSapService.h        MAC_Bsn_t;                             < Data type for macBSN. */
typedef MAC_Addr64bit_t         MAC_CoordExtendedAddress_t;         /*!< Data type for macCoordExtendedAddress. */
typedef MAC_Addr16bit_t         MAC_CoordShortAddress_t;            /*!< Data type for macCoordShortAddress. */
/* See bbMacSapService.h        MAC_Dsn_t;                             < Data type for macDSN. */
typedef Bool8_t                 MAC_GTSPermit_t;                    /*!< Data type for macGTSPermit. */
typedef uint8_t                 MAC_MaxCSMABackoffs_t;              /*!< Data type for macMaxCSMABackoffs. */
typedef uint8_t                 MAC_MinBE_t;                        /*!< Data type for macMinBE. */
/* See bbMacSapAddress.h        MAC_PanId_t;                           < Data type for macPANId. */
typedef BitField8_t             MAC_PromiscuousMode_t;              /*!< Data type for macPromiscuousMode. */
typedef Bool8_t                 MAC_RxOnWhenIdle_t;                 /*!< Data type for macRxOnWhenIdle. */
typedef MAC_Addr16bit_t         MAC_ShortAddress_t;                 /*!< Data type for macShortAddress. */
typedef uint8_t                 MAC_SuperframeOrder_t;              /*!< Data type for macSuperframeOrder. */
typedef uint16_t                MAC_TransactionPersistenceTime_t;   /*!< Data type for macTransactionPersistenceTime. */
typedef Bool8_t                 MAC_AssociatedPANCoord_t;           /*!< Data type for macAssociatedPANCoord. */
typedef uint8_t                 MAC_MaxBE_t;                        /*!< Data type for macMaxBE. */
typedef uint16_t                MAC_MaxFrameTotalWaitTime_t;        /*!< Data type for macMaxFrameTotalWaitTime. */
typedef uint8_t                 MAC_MaxFrameRetries_t;              /*!< Data type for macMaxFrameRetries. */
typedef uint8_t                 MAC_ResponseWaitTime_t;             /*!< Data type for macResponseWaitTime. */
typedef uint16_t                MAC_SyncSymbolOffset_t;             /*!< Data type for macSyncSymbolOffset. */
typedef Bool8_t                 MAC_TimestampSupported_t;           /*!< Data type for macTimestampSupported. */
typedef Bool8_t                 MAC_SecurityEnabled_t;              /*!< Data type for macSecurityEnabled. */
typedef uint8_t                 MAC_MinLIFSPeriod_t;                /*!< Data type for macMinLIFSPeriod. */
typedef uint8_t                 MAC_MinSIFSPeriod_t;                /*!< Data type for macMinSIFSPeriod. */
typedef Bool8_t                 MAC_ThreadMode_t;                   /*!< Data type for macThreadMode. */
/**@}*/

/**//**
 * \brief   Enumeration of values of MAC-PIB attribute macPromiscuousMode.
 * \details The macPromiscuousMode attribute is not implemented for the case of ordinary build. This attribute returns
 *  zero if read, and attempt to assign it is confirmed with READ_ONLY status.
 * \details Attribute macPromiscuousMode is implemented only for the case of MAC Tester build. This attribute has a
 *  custom bitmap format and regulates different aspects of MAC behavior:
 *  - bit #0 - ignored, reserved for purposes of the IEEE-compliant promiscuous mode, shall be assigned with zero.
 *  - bit #1 - disables responding with ACK frame on received unicast frame having Ack.Request flag set; disables also
 *      responding with pending frame on received Data Request command.
 *  - bit #2 - disables responding with Beacon frame on received Beacon Request command.
 *  - bit #3 - disables Dst. Address filtering on reception.
 *  - bit #4 - disables filtering DSN values on reception.
 *  - bit #5 - disables Hardware CRC and Dst. Address filtering on reception.
 *  - bits #6..7 - ignored, reserved for further purposes, shall be assigned with zero.
 */
typedef enum _MAC_PromiscuousModeCodes_t {
    MAC_PROMISCUOUS_MODE_DISABLED           = 0,            /*!< Promiscuous mode is disabled. */
    MAC_PROMISCUOUS_MODE_IEEE_COMPLIANT     = (1 << 0),     /*!< Enable IEEE-compliant promiscuous mode. Ignored. */
    MAC_PROMISCUOUS_MODE_DISABLE_ACK        = (1 << 1),     /*!< Disable responding with ACK and pending frames. */
    MAC_PROMISCUOUS_MODE_DISABLE_BEACON     = (1 << 2),     /*!< Disable responding with Beacon frame. */
    MAC_PROMISCUOUS_MODE_DISABLE_DST_FILTER = (1 << 3),     /*!< Disable Dst. Address filtering on reception. */
    MAC_PROMISCUOUS_MODE_DISABLE_DSN_FILTER = (1 << 4),     /*!< Disable filtering DSN values on reception. */
    MAC_PROMISCUOUS_MODE_DISABLE_HW_FILTER  = (1 << 5),     /*!< Disable Hardware CRC and Dst. Address filtering. */
} MAC_PromiscuousModeCodes_t;
SYS_DbgAssertStatic(MAC_PROMISCUOUS_MODE_DISABLED == FALSE && MAC_PROMISCUOUS_MODE_IEEE_COMPLIANT == TRUE);

/**//**
 * \brief   Conventional MAC-PIB attributes.
 * \note    Attribute macBeaconPayload is not included into the union because it is transferred as payload but not by
 *  value.
 */
#define MAC_PIB_ATTRIBUTES\
        MAC_ContextEnabled_t                macContextEnabled;\
        MAC_PanCoordinator_t                macPanCoordinator;\
        MAC_ExtendedAddress_t               macExtendedAddress;\
        MAC_AckWaitDuration_t               macAckWaitDuration;\
        MAC_AssociationPermit_t             macAssociationPermit;\
        MAC_AutoRequest_t                   macAutoRequest;\
        MAC_BattLifeExt_t                   macBattLifeExt;\
        MAC_BattLifeExtPeriods_t            macBattLifeExtPeriods;\
        MAC_BeaconPayloadLength_t           macBeaconPayloadLength;\
        MAC_BeaconOrder_t                   macBeaconOrder;\
        MAC_BeaconTxTime_t                  macBeaconTxTime;\
        MAC_Bsn_t                           macBSN;\
        MAC_CoordExtendedAddress_t          macCoordExtendedAddress;\
        MAC_CoordShortAddress_t             macCoordShortAddress;\
        MAC_Dsn_t                           macDSN;\
        MAC_GTSPermit_t                     macGTSPermit;\
        MAC_MaxCSMABackoffs_t               macMaxCSMABackoffs;\
        MAC_MinBE_t                         macMinBE;\
        MAC_PanId_t                         macPANId;\
        MAC_PromiscuousMode_t               macPromiscuousMode;\
        MAC_RxOnWhenIdle_t                  macRxOnWhenIdle;\
        MAC_ShortAddress_t                  macShortAddress;\
        MAC_SuperframeOrder_t               macSuperframeOrder;\
        MAC_TransactionPersistenceTime_t    macTransactionPersistenceTime;\
        MAC_AssociatedPANCoord_t            macAssociatedPANCoord;\
        MAC_MaxBE_t                         macMaxBE;\
        MAC_MaxFrameTotalWaitTime_t         macMaxFrameTotalWaitTime;\
        MAC_MaxFrameRetries_t               macMaxFrameRetries;\
        MAC_ResponseWaitTime_t              macResponseWaitTime;\
        MAC_SyncSymbolOffset_t              macSyncSymbolOffset;\
        MAC_TimestampSupported_t            macTimestampSupported;\
        MAC_SecurityEnabled_t               macSecurityEnabled;\
        MAC_MinLIFSPeriod_t                 macMinLIFSPeriod;\
        MAC_MinSIFSPeriod_t                 macMinSIFSPeriod;\
        PHY_Channel_t                       macCurrentChannel;\
        PHY_Page_t                          macCurrentPage;

/**//**
 * \brief   Additional private MAC-PIB attributes for PRNG support.
 */
#if defined(_HAL_USE_PRNG_)
# define MAC_PIB_PRNG_ATTRIBUTES\
        MAC_PrngSeed_t                      macPrngSeed;\
        MAC_PrngCounter_t                   macPrngCounter;
#else
# define MAC_PIB_PRNG_ATTRIBUTES
#endif

/**//**
 * \brief   Additional private MAC-PIB security attributes.
 * \note    Attributes macKeyTable, macDeviceTable, macSecurityLevelTable are not included into the union because they
 *  are transferred as payload but not by value.
 */
#if defined(_MAC_CONTEXT_ZBPRO_)
# define MAC_PIB_SECURITY_ATTRIBUTES\
        MAC_ThreadMode_t                    macThreadMode;\
        MAC_KeyTableEntries_t               macKeyTableEntries;\
        MAC_DeviceTableEntries_t            macDeviceTableEntries;\
        MAC_SecurityLevelTableEntries_t     macSecurityLevelTableEntries;\
        MAC_FrameCounter_t                  macFrameCounter;\
        MAC_SecurityLevel_t                 macAutoRequestSecurityLevel;\
        MAC_KeyIdMode_t                     macAutoRequestKeyIdMode;\
        MAC_KeySource_t                     macAutoRequestKeySource;\
        MAC_KeyIndex_t                      macAutoRequestKeyIndex;\
        MAC_KeySource_t                     macDefaultKeySource;\
        MAC_ExtendedAddress_t               macPANCoordExtendedAddress;\
        MAC_ShortAddress_t                  macPANCoordShortAddress;
#else
# define MAC_PIB_SECURITY_ATTRIBUTES
#endif

/**//**
 * \brief   Union of all private MAC-PIB attributes.
 */
#define MAC_PIB_PRIVATE_VARIANT\
    union {\
        MAC_PIB_ATTRIBUTES\
        MAC_PIB_PRNG_ATTRIBUTES\
        MAC_PIB_SECURITY_ATTRIBUTES\
        uint64_t                            macDummyFieldToComeWith64bitWidth;\
    }

/**//**
 * \brief   Variant data type for private MAC-PIB attributes.
 * \details This data type may be projected on the 64-bit integer.
 */
typedef MAC_PIB_PRIVATE_VARIANT  MacPibAttributeValue_t;
SYS_DbgAssertStatic(sizeof(MacPibAttributeValue_t) == sizeof(uint64_t));

/**//**
 * \brief   Union of all public MAC-PIB attributes.
 * \details MAC-PIB public attributes set includes MAC-PIB private attributes subset and PHY-PIB public attributes set.
 * \details Use this macro to define one-step-higher-layer public variant data type.
 * \par     Example of usage
 * \code
 *  #define ZBPRO_NWK_NIB_PUBLIC_VARIANT\
 *      union {\
 *          MAC_PIB_PUBLIC_VARIANT;\
 *          ZBPRO_NWK_NIB_PRIVATE_VARIANT;\
 *      }
 * \endcode
 */
#define MAC_PIB_PUBLIC_VARIANT\
    union {\
        PHY_PIB_PUBLIC_VARIANT;\
        MAC_PIB_PRIVATE_VARIANT;\
    }

/**//**
 * \brief   Variant data type for public MAC-PIB attributes.
 * \details MAC-PIB public attributes set includes MAC-PIB private attributes subset and PHY-PIB public attributes set.
 * \details This data type may be projected on the 64-bit integer.
 */
typedef MAC_PIB_PUBLIC_VARIANT  MAC_PibAttributeValue_t;
SYS_DbgAssertStatic(sizeof(MAC_PibAttributeValue_t) == sizeof(uint64_t));

#endif /* _BB_MAC_SAP_PIB_H */

/* eof bbMacSapPib.h */