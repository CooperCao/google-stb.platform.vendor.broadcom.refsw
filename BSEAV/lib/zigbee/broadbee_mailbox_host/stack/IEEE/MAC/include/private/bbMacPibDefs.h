/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacPibDefs.h $
*
* DESCRIPTION:
*   MAC-PIB internals definitions.
*
* $Revision: 3547 $
* $Date: 2014-09-11 13:46:56Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_PIB_DEFS_H
#define _BB_MAC_PIB_DEFS_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for storing permanent MAC-PIB attributes.
 * \details These nonstandard MAC-PIB attributes are implemented by each MAC Context;
 *  their values are preserved during the MLME-RESET.request.
 */
typedef struct _MacPibPermanent_t
{
    /* 64-bit data. */
    MAC_ExtendedAddress_t  macExtendedAddress;      /*!< Value of macExtendedAddress. */

    /* 1-bit data. */
    MAC_ContextEnabled_t   macContextEnabled;       /*!< Value of macContextEnabled. */

} MacPibPermanent_t;


/**//**
 * \brief   Structure for storing essential MAC-PIB attributes.
 * \details These MAC-PIB attributes are implemented by each MAC Context.
 */
typedef struct _MacPibEssential_t
{
    /* Structured / 16-bit data. */
    PHY_ChannelOnPage_t    macCurrentChannelOnPage;     /*!< Values of macCurrentPage and macCurrentChannel. */

    /* 16-bit data. */
    MAC_PanId_t            macPanId;                    /*!< Value of macPanId. */

    MAC_ShortAddress_t     macShortAddress;             /*!< Value of macShortAddress. */

    /* 8-bit data. */
    MAC_Dsn_t              macDsn;                      /*!< Value of macDsn. */

    MAC_MaxCsmaBackoffs_t  macMaxCsmaBackoffs;          /*!< Value of macMaxCsmaBackoffs. */

    MAC_MinBe_t            macMinBe;                    /*!< Value of macMinBe. */

    MAC_MaxBe_t            macMaxBe;                    /*!< Value of macMaxBe. */

    MAC_MaxFrameRetries_t  macMaxFrameRetries;          /*!< Value of macMaxFrameRetries. */

    /* 1-bit data. */
    MAC_RxOnWhenIdle_t     macRxOnWhenIdle;             /*!< Value of macRxOnWhenIdle. */

} MacPibEssential_t;


/**//**
 * \brief   Structure for storing MAC-PIB attributes for Beacons support.
 * \details These MAC-PIB attributes are implemented by each MAC Context except the
 *  single-context MAC configuration for RF4CE-Controller.
 * \note    The macBeaconPayload attribute is stored outside this structure.
 */
typedef struct _MacPibBeaconing_t
{
    /* 8-bit data. */
    MAC_BeaconPayloadLength_t  macBeaconPayloadLength;      /*!< Value of macBeaconPayloadLength. */

    MAC_Bsn_t                  macBsn;                      /*!< Value of macBsn. */

    /* 1-bit data. */
    MAC_AutoRequest_t          macAutoRequest;              /*!< Value of macAutoRequest. */

} MacPibBeaconing_t;


/**//**
 * \brief   Structure for storing ZigBee PRO specific MAC-PIB attributes.
 * \details These MAC-PIB attributes are implemented only by MAC context for ZigBee PRO
 *  stack.
 * \note    The MAC Context for RF4CE does not use retransmissions, CSMA-CA back-offs and
 *  auto-request.
 */
typedef struct _MacPibZBPRO_t
{
    /* 64-bit data. */
    MAC_CoordExtendedAddress_t        macCoordExtendedAddress;          /*!< Value of macCoordExtendedAddress. */

    /* 16-bit data. */
    MAC_CoordShortAddress_t           macCoordShortAddress;             /*!< Value of macCoordShortAddress. */

    MAC_TransactionPersistenceTime_t  macTransactionPersistenceTime;    /*!< Value of macTransactionPersistenceTime. */

    MAC_MaxFrameTotalWaitTime_t       macMaxFrameTotalWaitTime;         /*!< Value of macMaxFrameTotalWaitTime. */

    /* 8-bit data. */
    MAC_ResponseWaitTime_t            macResponseWaitTime;              /*!< Value of macResponseWaitTime. */

    /* 1-bit data. */
    MAC_PanCoordinator_t              macPanCoordinator;                /*!< Value of macPanCoordinator. */

    MAC_AssociationPermit_t           macAssociationPermit;             /*!< Value of macAssociationPermit. */

} MacPibZBPRO_t;


/*
 * MAC-PIB attributes default values.
 */
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
#define MAC_ATTR_DEFAULT_VALUE_GTS_PERMIT                       FALSE   /* Nonbeacon-enabled PAN does not provide GTS. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_CSMA_BACKOFFS                4
#define MAC_ATTR_DEFAULT_VALUE_MIN_BE                           3
#define MAC_ATTR_DEFAULT_VALUE_PAN_ID                           MAC_UNASSIGNED_PAN_ID
#define MAC_ATTR_DEFAULT_VALUE_PROMISCUOUS_MODE                 FALSE
#define MAC_ATTR_DEFAULT_VALUE_RX_ON_WHEN_IDLE                  FALSE
#define MAC_ATTR_DEFAULT_VALUE_SHORT_ADDRESS                    MAC_UNASSIGNED_SHORT_ADDRESS
#define MAC_ATTR_DEFAULT_VALUE_SUPERFRAME_ORDER                 15
#define MAC_ATTR_DEFAULT_VALUE_TRANSACTION_PERSISTENCE_TIME     0x01F4
#define MAC_ATTR_DEFAULT_VALUE_ASSOCIATED_PAN_COORD             FALSE
#define MAC_ATTR_DEFAULT_VALUE_MAX_BE                           5
#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_TOTAL_WAIT_TIME        1220    // TODO: Default value according to the MAC Cert. Test Spec., test DATA-04.
//#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_TOTAL_WAIT_TIME        1986    /* TODO: Replace with expression according to default PHY; see the comment below. */
#define MAC_ATTR_DEFAULT_VALUE_MAX_FRAME_RETRIES                3
#define MAC_ATTR_DEFAULT_VALUE_RESPONSE_WAIT_TIME               32
#define MAC_ATTR_DEFAULT_VALUE_SYNC_SYMBOL_OFFSET               0
#define MAC_ATTR_DEFAULT_VALUE_TIMESTAMP_SUPPORTED              TRUE
#define MAC_ATTR_DEFAULT_VALUE_SECURITY_ENABLED                 FALSE
#define MAC_ATTR_DEFAULT_VALUE_MIN_LIFS_PERIOD                  40
#define MAC_ATTR_DEFAULT_VALUE_MIN_SIFS_PERIOD                  12


/* TODO: Implement as a macro-formula from other default values.
 *
 * Probably better to implement a common function for the def-macro and for macPibApiGetDefaultMaxFrameTotalWaitTime()
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


/*
 * MAC-PIB attributes values constraints.
 */
#define MAC_ATTR_ONLYALLOWED_VALUE_CONTEXT_ENABLED              TRUE
#define MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_ZBPRO   16
#define MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_RF4CE   4
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_CSMA_BACKOFFS             5
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_BE                        8
#define MAC_ATTR_MINALLOWED_VALUE_MAX_BE                        3
#define MAC_ATTR_MAXALLOWED_VALUE_MAX_FRAME_RETRIES             7
#define MAC_ATTR_MAXALLOWED_VALUE_RESPONSE_WAIT_TIME            64
#define MAC_ATTR_MINALLOWED_VALUE_RESPONSE_WAIT_TIME            2


#endif /* _BB_MAC_PIB_DEFS_H */