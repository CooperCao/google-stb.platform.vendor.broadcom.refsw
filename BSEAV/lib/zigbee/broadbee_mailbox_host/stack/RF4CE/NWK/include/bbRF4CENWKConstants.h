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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKConstants.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer component constants declarations.
 *
 * $Revision: 3484 $
 * $Date: 2014-09-08 08:06:50Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_CONSTANTS_H
#define _RF4CE_NWK_CONSTANTS_H

/************************* CONSTANTS ***************************************************/
/* The amount that needs to be added to the frame counter if a device is reset. */
#define RF4CE_NWKC_FRAME_COUNTER_WINDOW              1024
/* The length, in octets, of the MAC beacon payload field, as used by the ZigBee RF4CE protocol. */
#define RF4CE_NWKC_MAC_BEACON_PAYLOAD_LENGTH         2
/* The maximum duty cycle in MAC symbols, permitted for a power saving device. */
#define RF4CE_NWKC_MAX_DUTY_CYCLE                    62500
/* The maximum timeout for short retry period while sneaking among channels. */
#define RF4CE_NWKC_REPEAT_SHORT_PERIOD               6250
/* The maximum time, in MAC symbols, to wait for each security link key seed exchange. */
#define RF4CE_NWKC_MAX_KEY_SEED_WAIT_TIME            3750
/* The maximum number of entries supported in the pairing table. */
/* For a controller this value should be 1. For a target this value can be overwritten
 * with the SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES=xx compiler macro */
#ifdef RF4CE_CONTROLLER
#    ifdef SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES
#        if (SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0)
#            define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES
#        else /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0 */
#            define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES 1
#        endif /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0 */
#    else /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES */
#        define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES 1
#    endif /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES */
#else /* RF4CE_CONTROLLER */
#    ifdef SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES
#        if (SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0)
#            define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES
#        else /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0 */
#            define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES 6
#        endif /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES > 0 */
#    else /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES */
#        define RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES 6
#    endif /* SET_RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES */
#endif /* RF4CE_CONTROLLER */
/* The maximum acceptable power, in dBm, at which key seed command frames should be sent. */
#define RF4CE_NWKC_MAX_SEC_CMD_TX_POWER              -15
/* The minimum receiver on time, in MAC symbols, permitted for a power saving device. */
#define RF4CE_NWKC_MIN_ACTIVE_PERIOD                 1050
/* The minimum number of pairing table entries that a controller device shall support. */
#define RF4CE_NWKC_MIN_CONTROLLER_PAIRING_TABLE_SIZE 1
/* The minimum number of entries that must be supported in the node descriptor list generated through the discovery
   process. */
#define RF4CE_NWKC_MIN_NODE_DESC_LIST_SIZE           3
/* The maximum number of entries supported in the node descriptors list generated through the discovery process. */
#define RF4CE_NWKC_MAX_NODE_DESC_LIST_SIZE           0xff   /* Implementation-specific but >= nwkcMinNodeDescListSize */
/* The minimum overhead the NWK layer adds to an application payload. */
#define RF4CE_NWKC_MIN_NWK_HEADER_OVERHEAD           5
/* The minimum number of pairing table entries that a target device shall support. */
#define RF4CE_NWKC_MIN_TARGET_PAIRING_TABLE_SIZE     5
/* The identifier of the NWK layer protocol being used by this device. */
#define RF4CE_NWKC_PROTOCOL_IDENTIFIER               0xce
/* The version of the ZigBee RF4CE protocol implemented on this device. */
#define RF4CE_NWKC_PROTOCOL_VERSION                  1          /* 0b01 */

/**//**
 * \brief RF4CE NWK Node Capabilities flags definitions.
 */
#define RF4CE_NWK_NODE_TARGET                           0x01
#define RF4CE_NWK_NODE_CONTROLLER                       0x00
#define RF4CE_NWK_NODE_POWER_MAINS                      0x02
#define RF4CE_NWK_NODE_POWER_BATTERY                    0x00
#define RF4CE_NWK_NODE_SECURITY_ENABLED                 0x04
#define RF4CE_NWK_NODE_SECURITY_DISABLED                0x00
#define RF4CE_NWK_NODE_CHANNEL_NORMALIZATION_CAPABLE    0x08
#define RF4CE_NWK_NODE_CHANNEL_NORMALIZATION_INCAPABLE  0x00

/* The capabilities of this node. [Separately Target and Controller] */
#define RF4CE_NWKC_NODE_CAPABILITIES_TARGET          (RF4CE_NWK_NODE_TARGET | RF4CE_NWK_NODE_POWER_MAINS | \
    RF4CE_NWK_NODE_SECURITY_ENABLED | RF4CE_NWK_NODE_CHANNEL_NORMALIZATION_INCAPABLE)
#define RF4CE_NWKC_NODE_CAPABILITIES_CONTROLLER      (RF4CE_NWK_NODE_CONTROLLER | RF4CE_NWK_NODE_POWER_BATTERY | \
    RF4CE_NWK_NODE_SECURITY_ENABLED | RF4CE_NWK_NODE_CHANNEL_NORMALIZATION_INCAPABLE)
/* The capabilities of this node. According to the external definition. */

#ifndef RF4CE_CUSTOM_COMPILE_RULES

#ifdef RF4CE_TARGET
#define RF4CE_NWKC_NODE_CAPABILITIES RF4CE_NWKC_NODE_CAPABILITIES_TARGET
#else /* RF4CE_TARGET */
#define RF4CE_NWKC_NODE_CAPABILITIES RF4CE_NWKC_NODE_CAPABILITIES_CONTROLLER
#endif /* RF4CE_TARGET */
/* The manufacturer-specific vendor identifier for this node. */
#define RF4CE_NWKC_VENDOR_IDENTIFIER                 0x1234
/* The manufacturer-specific identification string for this node. */
#define RF4CE_NWKC_VENDOR_STRING                     "BRDBEE"

#endif /* RF4CE_CUSTOM_COMPILE_RULES */

/* Vendor string length */
#define RF4CE_NWK_VENDOR_STRING_LENGTH               7
/* User string length */
#define RF4CE_NWK_USER_STRING_LENGTH                 15
/* ZRC2 Application specific User string length */
#define RF4CE_NWK_USER_STRING_APL_LENGTH             8
/* Device type list maximum length (2-bit field) */
#define RF4CE_NWK_MAX_DEVICE_TYPE_LIST_LENGTH        3
/* Profile ID list maximum length (3-bit field) */
#define RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH         7
/* Total Device Node maximum size */
#define RF4CE_NWK_MAX_TOTAL_VARIABLE_LENGTH          (RF4CE_NWK_USER_STRING_LENGTH + \
    RF4CE_NWK_MAX_DEVICE_TYPE_LIST_LENGTH + RF4CE_NWK_MAX_PROFILE_ID_LIST_LENGTH)
/* Security Key Length */
#define RF4CE_SECURITY_KEY_LENGTH                    (128 >> 3)
/* Maximum incoming packets for NWK */
#define RF4CE_NWK_MAX_INCOMING_PACKETS               5
/* Maximum outgoing packets for NWK */
#define RF4CE_NWK_MAX_OUTGOING_PACKETS               1
/* Pairing table entry is busy. Available on the dstLogicalChannel entry. */
#define RF4CE_PAIRING_BUSY                           0x80

/* The address mask */
#define RF4CE_ADDRESS_MASK                           0xffff
/* The start pairing address */
#define RF4CE_START_PAIR_ADDRESS                     0xfffe
/* The extended address mask */
#define RF4CE_EXTADDRESS_MASK                        0xffffffffffffffffULL
/* The invalid address */
#define RF4CE_INVALID_ADDRESS                        RF4CE_ADDRESS_MASK
/* The invalid extended address */
#define RF4CE_INVALID_EXT_ADDRESS                    RF4CE_EXTADDRESS_MASK
/* The Channel Page (IEEE Std 802.15.4-2006 Table 2) */
#define RF4CE_NWKC_CHANNEL_PAGE                      0
/* The index for MAC Scan ED for channel 15 */
#define RF4CE_NWK_INDEX_15                           0
/* The index for MAC Scan ED for channel 20 */
#define RF4CE_NWK_INDEX_20                           1
/* The index for MAC Scan ED for channel 25 */
#define RF4CE_NWK_INDEX_25                           2

/* Define the conventional RF4CE channels set if channels are not assigned from the make configuration. */
#if (defined(RF4CE_NWK_LOGICALCHANNEL_15) || defined(RF4CE_NWK_LOGICALCHANNEL_20) || defined(RF4CE_NWK_LOGICALCHANNEL_25))
# if (!defined(RF4CE_NWK_LOGICALCHANNEL_15) || !defined(RF4CE_NWK_LOGICALCHANNEL_20) || !defined(RF4CE_NWK_LOGICALCHANNEL_25))
#  error Either all or none of three RF4CE channels must be assigned in make configuration.
# endif
#else /* else: if at least one of channels is defined externally, then ... */
# define RF4CE_NWK_LOGICALCHANNEL_15                 15
# define RF4CE_NWK_LOGICALCHANNEL_20                 20
# define RF4CE_NWK_LOGICALCHANNEL_25                 25
#endif /* (defined(RF4CE_NWK_LOGICALCHANNEL_15) || defined(RF4CE_NWK_LOGICALCHANNEL_20) || defined(RF4CE_NWK_LOGICALCHANNEL_25)) */

/* The channel mask to use in all scanning operations. */
#define RF4CE_NWKC_CHANNEL_MASK_15                   BIT(RF4CE_NWK_LOGICALCHANNEL_15) //0x0008000
#define RF4CE_NWKC_CHANNEL_MASK_20                   BIT(RF4CE_NWK_LOGICALCHANNEL_20) //0x0100000
#define RF4CE_NWKC_CHANNEL_MASK_25                   BIT(RF4CE_NWK_LOGICALCHANNEL_25) //0x2000000
/* 010 0001 0000 1000 0000 0000 0000 */
#define RF4CE_NWKC_CHANNEL_MASK (RF4CE_NWKC_CHANNEL_MASK_15 | RF4CE_NWKC_CHANNEL_MASK_20 | RF4CE_NWKC_CHANNEL_MASK_25)

/* Maximum repetition interval value */
#define RF4CE_NWK_MAX_REPETITION_INTERVAL            0xffffff
/* TRUE value */
#define RF4CE_NWK_TRUE                               1
/* FALSE value */
#define RF4CE_NWK_FALSE                              0
/* Maximum first attempt CSMA Backoffs value */
#define RF4CE_NWK_MAX__FIRST_ATTEMPT_CSMA_BACKOFFS   5
/* Maximum first attempt frame retries value */
#define RF4CE_NWK_MAX__FIRST_ATTEMPT_FRAME_RETRIES   7
/* Maximum response wait time value */
#define RF4CE_NWK_MAX_RESPONSE_WAIT_TIME             0xffffff
/* RX enable forever */
#define RF4CE_NWK_RX_ON_FOREVER                      0xffffff
/* RX disable */
#define RF4CE_NWK_RX_OFF                             0
/* Maximum scan duration value */
#define RF4CE_NWK_MAX_SCAN_DURATION                  14

/* Discovery device mask */
#define RF4CE_NWK_DEVICE_MASK                        0xff

/* Invalid frame counter value */
#define RF4CE_NWK_INVALID_FRAME_COUNTER              0xFFFFFFFF

/* Invalid pairing reference value */
#define RF4CE_NWK_INVALID_PAIRING_REF                0xff


/* Frequency Agility Counter */
#define RF4CE_NWK_FREQUENCY_AGILITY_COUTER           10

/* Frequency Agility Timeout */
#define RF4CE_NWK_FREQUENCY_AGILITY_TIMEOUT          100    /* 300 */

/* Frequency Agility Scan Threshold */
#ifdef __SoC__
# define RF4CE_NWKC_FA_SCAN_THRESHOLD                -40    /* Value is in dBm. Assigned according to Broadcom CSP #932372. */
#else /* __SoC__ */
# define RF4CE_NWKC_FA_SCAN_THRESHOLD                -81    /* -70 */
#endif /* __SoC__ */

/* Frequency Agility Scan Duration */
#define RF4CE_NWKC_FA_SCAN_DURATION                  0xFA

#endif /* _RF4CE_NWK_CONSTANTS_H */