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
 *      MAC-PIB internals definitions.
 *
*******************************************************************************/

#ifndef _BB_MAC_PIB_DEFS_H
#define _BB_MAC_PIB_DEFS_H

/************************* INCLUDES ***********************************************************************************/
#include "bbMacSapPib.h"
#if defined(_MAC_CONTEXT_ZBPRO_)
# include "private/bbMacSecurityDefs.h"
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Structure for storing permanent MAC-PIB attributes.
 * \details These nonstandard MAC-PIB attributes are implemented by each MAC Context; their values are preserved during
 *  the MLME-RESET.request.
 */
typedef struct _MacPibPermanent_t {
    /* 64-bit data. */
    MAC_ExtendedAddress_t   macExtendedAddress;     /*!< Value of macExtendedAddress. */
    /* 1-bit data. */
    MAC_ContextEnabled_t    macContextEnabled;      /*!< Value of macContextEnabled. */
} MacPibPermanent_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Structure for storing essential MAC-PIB attributes.
 * \details These MAC-PIB attributes are implemented by each MAC Context.
 */
typedef struct _MacPibEssential_t {
    /* 16-bit data. */
    MAC_PanId_t             macPANId;                   /*!< Value of macPANId. */
    MAC_ShortAddress_t      macShortAddress;            /*!< Value of macShortAddress. */

    /* 8-bit data. */
    PHY_PageChannel_t       macCurrentChannelOnPage;    /*!< Values of macCurrentPage and macCurrentChannel. */
    MAC_Dsn_t               macDSN;                     /*!< Value of macDSN. */
    MAC_MaxCSMABackoffs_t   macMaxCSMABackoffs;         /*!< Value of macMaxCSMABackoffs. */
    MAC_MinBE_t             macMinBE;                   /*!< Value of macMinBE. */
    MAC_MaxBE_t             macMaxBE;                   /*!< Value of macMaxBE. */
    MAC_MaxFrameRetries_t   macMaxFrameRetries;         /*!< Value of macMaxFrameRetries. */

    /* 1-bit data. */
    MAC_RxOnWhenIdle_t      macRxOnWhenIdle;            /*!< Value of macRxOnWhenIdle. */
} MacPibEssential_t;

/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   Structure for storing MAC-PIB attributes for Beacons support.
 * \details These MAC-PIB attributes are implemented by each MAC Context except the single-context MAC configuration for
 *  RF4CE-Controller.
 * \note    The macBeaconPayload attribute is stored outside this structure.
 */
typedef struct _MacPibBeaconing_t {
    /* 8-bit data. */
    MAC_BeaconPayloadLength_t   macBeaconPayloadLength;     /*!< Value of macBeaconPayloadLength. */
    MAC_Bsn_t                   macBSN;                     /*!< Value of macBSN. */

    /* 1-bit data. */
    MAC_AutoRequest_t           macAutoRequest;             /*!< Value of macAutoRequest. */
} MacPibBeaconing_t;
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Structure for storing ZigBee PRO specific MAC-PIB attributes.
 * \details These MAC-PIB attributes are implemented only by MAC context for ZigBee PRO stack.
 * \note    The MAC Context for RF4CE does not use retransmissions, CSMA-CA back-offs and auto-request.
 */
typedef struct _MacPibZBPRO_t {
    /* 64-bit data. */
    MAC_CoordExtendedAddress_t          macCoordExtendedAddress;        /*!< Value of macCoordExtendedAddress. */

    /* 16-bit data. */
    MAC_CoordShortAddress_t             macCoordShortAddress;           /*!< Value of macCoordShortAddress. */
    MAC_TransactionPersistenceTime_t    macTransactionPersistenceTime;  /*!< Value of macTransactionPersistenceTime. */
    MAC_MaxFrameTotalWaitTime_t         macMaxFrameTotalWaitTime;       /*!< Value of macMaxFrameTotalWaitTime. */

    /* 8-bit data. */
    MAC_ResponseWaitTime_t              macResponseWaitTime;            /*!< Value of macResponseWaitTime. */

    /* 1-bit data. */
    MAC_PanCoordinator_t                macPanCoordinator;              /*!< Value of macPanCoordinator. */
    MAC_AssociationPermit_t             macAssociationPermit;           /*!< Value of macAssociationPermit. */
    MAC_SecurityEnabled_t               macSecurityEnabled;             /*!< Value of macSecurityEnabled. */
} MacPibZBPRO_t;
#endif /* _MAC_CONTEXT_ZBPRO_ */

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   Structure for the MAC Security attributes.
 * \details These MAC-PIB attributes are implemented only by MAC context for ZigBee PRO stack.
 */
typedef struct _MacPibSecurity_t {
    /* 64-bit data. */
    MAC_KeySource_t                     macAutoRequestKeySource;        /*!< Value of macAutoRequestKeySource. */
    MAC_KeySource_t                     macDefaultKeySource;            /*!< Value of macDefaultKeySource. */
    MAC_ExtendedAddress_t               macPANCoordExtendedAddress;     /*!< Value of macPANCoordExtendedAddress. */

    /* 32-bit data. */
    MAC_FrameCounter_t                  macFrameCounter;                /*!< Value of macFrameCounter. */

    /* 16-bit data. */
    MM_ChunkId_t                        macKeyTableLink;                /*!< Link to macKeyTable. */
    MM_ChunkId_t                        macDeviceTableLink;             /*!< Link to macDeviceTable. */
    MM_ChunkId_t                        macSecurityLevelTableLink;      /*!< Link to macSecurityLevelTable. */
    MAC_ShortAddress_t                  macPANCoordShortAddress;        /*!< Value of macPANCoordShortAddress. */

    /* 16- or 8-bit data. */
    MAC_KeyTableEntries_t               macKeyTableEntries;             /*!< Value of macKeyTableEntries. */
    MAC_DeviceTableEntries_t            macDeviceTableEntries;          /*!< Value of macDeviceTableEntries. */

    /* 8-bit data. */
    MAC_SecurityLevelTableEntries_t     macSecurityLevelTableEntries;   /*!< Value of macSecurityLevelTableEntries. */
    MAC_SecurityLevel_t                 macAutoRequestSecurityLevel;    /*!< Value of macAutoRequestSecurityLevel. */
    MAC_KeyIdMode_t                     macAutoRequestIdMode;           /*!< Value of macAutoRequestIdMode. */
    MAC_KeyIndex_t                      macAutoRequestKeyIndex;         /*!< Value of macAutoRequestKeyIndex. */

    /* 1-bit data. */
    MAC_ThreadMode_t                    macThreadMode;                  /*!< Value of macThreadMode. */
} MacPibSecurity_t;
#endif /* _MAC_CONTEXT_ZBPRO_ */

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    MAC-PIB attributes default values.
 */
/**@{*/
#define MAC_ATTR_DEFAULT_VALUE_CONTEXT_ENABLED                  FALSE
#define MAC_ATTR_DEFAULT_VALUE_PAN_COORDINATOR                  FALSE
#define MAC_ATTR_DEFAULT_VALUE_EXTENDED_ADDRESS                 MAC_UNASSIGNED_EXTENDED_ADDRESS
#define MAC_ATTR_DEFAULT_VALUE_ASSOCIATION_PERMIT               FALSE
#define MAC_ATTR_DEFAULT_VALUE_AUTO_REQUEST                     TRUE
#define MAC_ATTR_DEFAULT_VALUE_BATT_LIFE_EXT                    FALSE
#define MAC_ATTR_DEFAULT_VALUE_BEACON_PAYLOAD_LENGTH            0
#define MAC_ATTR_DEFAULT_VALUE_BEACON_ORDER                     15
#define MAC_ATTR_DEFAULT_VALUE_BEACON_TX_TIME                   0x000000
#define MAC_ATTR_DEFAULT_VALUE_BSN                              0x00
#define MAC_ATTR_DEFAULT_VALUE_COORD_EXTENDED_ADDRESS           MAC_UNASSIGNED_EXTENDED_ADDRESS
#define MAC_ATTR_DEFAULT_VALUE_COORD_SHORT_ADDRESS              MAC_UNASSIGNED_SHORT_ADDRESS
#define MAC_ATTR_DEFAULT_VALUE_DSN                              0x00
#define MAC_ATTR_DEFAULT_VALUE_GTS_PERMIT                       FALSE   /* Nonbeacon-enabled PAN doesn't provide GTS. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_CSMA_BACKOFFS                4
#define MAC_ATTR_DEFAULT_VALUE_MIN_BE                           3
#define MAC_ATTR_DEFAULT_VALUE_MIN_BE_ZBPRO                     5       /* See ZigBee PRO r20, Annex D.6. */
#define MAC_ATTR_DEFAULT_VALUE_PAN_ID                           MAC_UNASSIGNED_PAN_ID
#define MAC_ATTR_DEFAULT_VALUE_PROMISCUOUS_MODE                 MAC_PROMISCUOUS_MODE_DISABLED
#define MAC_ATTR_DEFAULT_VALUE_RX_ON_WHEN_IDLE                  FALSE
#define MAC_ATTR_DEFAULT_VALUE_SHORT_ADDRESS                    MAC_UNASSIGNED_SHORT_ADDRESS
#define MAC_ATTR_DEFAULT_VALUE_SUPERFRAME_ORDER                 15
#define MAC_ATTR_DEFAULT_VALUE_TRANSACTION_PERSISTENCE_TIME     0x01F4
#define MAC_ATTR_DEFAULT_VALUE_ASSOCIATED_PAN_COORD             FALSE
#define MAC_ATTR_DEFAULT_VALUE_MAX_BE                           5
#define MAC_ATTR_DEFAULT_VALUE_MAX_BE_ZBPRO                     8       /* See ZigBee PRO r20, Annex D.6. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_TOTAL_WAIT_TIME        (1220 + 266)    /* According to DATA-04 cert. test. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_TOTAL_WAIT_TIME_ZBPRO  \
        ((31 + 63 + 127 + 255 * (4 - 3 + 1)) * 20 + 8 * (4 + 1) + 12 + 266)     /* For macMinBE = 5 and macMaxBE = 8. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_RETRIES                3
#define MAC_ATTR_DEFAULT_VALUE_RESPONSE_WAIT_TIME               32
#define MAC_ATTR_DEFAULT_VALUE_SYNC_SYMBOL_OFFSET               0
#define MAC_ATTR_DEFAULT_VALUE_TIMESTAMP_SUPPORTED              TRUE
#define MAC_ATTR_DEFAULT_VALUE_SECURITY_ENABLED                 FALSE
#define MAC_ATTR_DEFAULT_VALUE_MIN_LIFS_PERIOD                  40
#define MAC_ATTR_DEFAULT_VALUE_MIN_SIFS_PERIOD                  12

#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_TYPE                   3
#define MAC_ATTR_DEFAULT_VALUE_MIN_COMMAND_FRAME_IDENTIFIER     1
#define MAC_ATTR_DEFAULT_VALUE_MAX_COMMAND_FRAME_IDENTIFIER     9
#define MAC_ATTR_DEFAULT_VALUE_MAX_SECURITY_MINIMUM             7
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Probably better to implement a common function for the def-macro and for macPibApiGetDefaultMaxFrameTotalWaitTime()
 *
 * Default value for macMaxFrameTotalWaitTime @ 2.45 MHz band:
 *
 *  macMaxFrameTotalWaitTime = (2^macMinBE) * (2^m - 1) * aUnitBackoffPeriod +
 *          (2^macMaxBE - 1) * (macMaxCSMABackoffs - m) * aUnitBackoffPeriod +
 *          phyMaxFrameDuration,
 *  where
 *  m = MIN(macMaxBE-macMinBE, macMaxCSMABackoffs).
 *
 *  macMaxBE = 5
 *  macMinBE = 3
 *  macMaxCSMABackoffs = 4
 *  m = MIN(macMaxBE-macMinBE, macMaxCSMABackoffs) = MIN(5-3, 4) = MIN(2, 4) = 2
 *  2^macMinBE = 2^3 = 8
 *  2^m - 1 = 2^2 - 1 = 4-1 = 3
 *  2^macMaxBE - 1 = 2^5 - 1 = 32 - 1 = 31
 *  macMaxCSMABackoffs - m = 4 - 2 = 2
 *  macMaxFrameTotalWaitTime = (8*3+31*2)*20+266 = 1986
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    MAC-PIB attributes values constraints.
 */
/**@{*/
#define MAC_ATTR_ONLYALLOWED_VALUE_CONTEXT_ENABLED              TRUE
#define MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_ZBPRO   16
#define MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_RF4CE   4
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_CSMA_BACKOFFS             5
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_BE                        8
#define MAC_ATTR_MINALLOWED_VALUE_MAX_BE                        3
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_FRAME_RETRIES             7
#define MAC_ATTR_MAXALLOWED_VALUE_RESPONSE_WAIT_TIME            64
#define MAC_ATTR_MINALLOWED_VALUE_RESPONSE_WAIT_TIME            2
/**@}*/

#endif /* _BB_MAC_PIB_DEFS_H */

/* eof bbMacPibDefs.h */