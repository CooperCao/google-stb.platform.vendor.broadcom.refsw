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
 *      MAC-SAP common definitions.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_DEFS_H
#define _BB_MAC_SAP_DEFS_H

/************************* INCLUDES ***********************************************************************************/
#include "bbMacBasics.h"
#include "bbPhySapForMac.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   MAC constants.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.4.1, table 85.
 */
/**@{*/
#define MAC_aBaseSlotDuration               (60)    /*!< The number of symbols forming a superframe slot when the
                                                        superframe order is equal to 0. */

#define MAC_aGTSDescPersistenceTime         (4)     /*!< The number of superframes in which a GTS descriptor exists in
                                                        the beacon frame of the PAN coordinator. */

#define MAC_aMaxBeaconOverhead              (75)    /*!< The maximum number of octets added by the MAC sublayer to the
                                                        MAC payload of a beacon frame. */

#define MAC_aMaxLostBeacons                 (4)     /*!< The number of consecutive lost beacons that will cause the MAC
                                                        sublayer of a receiving device to declare a loss of
                                                        synchronization. */

#define MAC_aMaxMPDUUnsecuredOverhead       (25)    /*!< The maximum number of octets added by the MAC sublayer to the
                                                        PSDU without security. */

#define MAC_aMaxSIFSFrameSize               (18)    /*!< The maximum size of an MPDU, in octets, that can be followed by
                                                        a SIFS period. */

#define MAC_aMinCAPLength                   (440)   /*!< The minimum number of symbols forming the CAP. This ensures
                                                        that MAC commands can still be transferred to devices when GTSs
                                                        are being used. An exception to this minimum shall be allowed
                                                        for the accommodation of the temporary increase in the beacon
                                                        frame length needed to perform GTS maintenance. */

#define MAC_aMinMPDUOverhead                (9)     /*!< The minimum number of octets added by the MAC sublayer
                                                        to the PSDU. */

#define MAC_aNumSuperframeSlots             (16)    /*!< The number of slots contained in any superframe. */

#define MAC_aUnitBackoffPeriod              (20)    /*!< The number of symbols forming the basic time period used by the
                                                        CSMA-CA algorithm. */

#define MAC_aBaseSuperframeDuration         (MAC_aBaseSlotDuration * MAC_aNumSuperframeSlots)
                                                    /*!< The number of symbols forming a superframe when the superframe
                                                        order is equal to 0. */

#define MAC_aMaxBeaconPayloadLength         (PHY_aMaxPHYPacketSize - MAC_aMaxBeaconOverhead)
                                                    /*!< The maximum size, in octets, of a beacon payload. */

#define MAC_aMaxMACSafePayloadSize          (PHY_aMaxPHYPacketSize - MAC_aMaxMPDUUnsecuredOverhead)
                                                    /*!< The maximum number of octets that can be transmitted in the MAC
                                                        Payload field of an unsecured MAC frame that will be guaranteed
                                                        not to exceed aMaxPHYPacketSize. */

#define MAC_aMaxMACPayloadSize              (PHY_aMaxPHYPacketSize - MAC_aMinMPDUOverhead)
                                                    /*!< The maximum number of octets that can be transmitted in the MAC
                                                        Payload field. */

#if defined(_MAC_CONTEXT_ZBPRO_)
/**//**
 * \brief   The 64-bit address assigned to this device in ZigBee PRO context.
 */
# define ZBPRO_MAC_aExtendedAddress         (MAC_GetAddr64bit(MAC_ZBPRO_CONTEXT_ID))
#endif /* _MAC_CONTEXT_ZBPRO_ */

#if defined(_MAC_CONTEXT_RF4CE_)
/**//**
 * \brief   The 64-bit address assigned to this device in RF4CE context.
 */
# define RF4CE_MAC_aExtendedAddress         (MAC_GetAddr64bit(MAC_RF4CE_CONTEXT_ID))
#endif /* _MAC_CONTEXT_RF4CE_ */
/**@}*/

/**//**
 * \brief   Consolidated enumerations for the MAC-SAP.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.17, table 78.
 */
typedef enum _MAC_Enumerations_t {
    MAC_SUCCESS                     = 0x00,     /*!< The requested operation was completed successfully. For a
                                                    transmission request, this value indicates a successful
                                                    transmission. */

    MAC_ASSOCIATION_SUCCESSFUL      = 0x00,     /*!< Association response status: Association successful. */

    MAC_PAN_AT_CAPACITY             = 0x01,     /*!< Association response status: PAN at capacity. */

    MAC_PAN_ACCESS_DENIED           = 0x02,     /*!< Association response status: PAN access denied. */

    MAC_ASSOCIATION_STATUS_MAX      = MAC_PAN_ACCESS_DENIED,    /*!< Maximum allowed value for
                                                                    association response status. */
    MAC_NO_MEMORY                   = 0x7D,     /*!< Not enough dynamic memory to proceed some request. */

    MAC_PURGED                      = 0x7E,     /*!< The MCPS-DATA.request processing was terminated due to
                                                    MCPS-PURGE.request. */

    MAC_RESET                       = 0x7F,     /*!< The request processing was terminated due to MAC reset. */

    MAC_COUNTER_ERROR               = 0xDB,     /*!< The frame counter purportedly applied by the originator of the
                                                    received frame is invalid. */

    MAC_IMPROPER_KEY_TYPE           = 0xDC,     /*!< The key purportedly applied by the originator of the received frame
                                                    is not allowed to be used with that frame type according to the key
                                                    usage policy of the recipient. */

    MAC_IMPROPER_SECURITY_LEVEL     = 0xDD,     /*!< The security level purportedly applied by the originator of the
                                                    received frame does not meet the minimum security level
                                                    required/expected by the recipient for that frame type. */

    MAC_UNSUPPORTED_LEGACY          = 0xDE,     /*!< The received frame was purportedly secured using security based on
                                                    IEEE Std 802.15.4-2003, and such security is not supported by
                                                    IEEE Std 802.15.4-2006. */

    MAC_UNSUPPORTED_SECURITY        = 0xDF,     /*!< The security purportedly applied by the originator of the received
                                                    frame is not supported. */

    MAC_BEACON_LOSS                 = 0xE0,     /*!< The beacon was lost following a synchronization request. */

    MAC_CHANNEL_ACCESS_FAILURE      = 0xE1,     /*!< A transmission could not take place due to activity on the channel,
                                                    i.e., the CSMA-CA mechanism has failed. */

    MAC_DENIED                      = 0xE2,     /*!< The GTS request has been denied by the PAN coordinator. */

    MAC_DISABLE_TRX_FAILURE         = 0xE3,     /*!< The attempt to disable the transceiver has failed. */

    MAC_FAILED_SECURITY_CHECK       = 0xE4,     /*!< The received frame induces a failed security check according to the
                                                    security suite (only for IEEE Std 802.15.4-2003). */

    MAC_SECURITY_ERROR              = 0xE4,     /*!< Cryptographic processing of the received secured frame failed
                                                    (only for IEEE Std 802.15.4-2006). */

    MAC_FRAME_TOO_LONG              = 0xE5,     /*!< Either a frame resulting from processing has a length that is
                                                    greater than aMaxPHYPacketSize or a requested transaction is too
                                                    large to fit in the CAP or GTS. */

    MAC_INVALID_GTS                 = 0xE6,     /*!< The requested GTS transmission failed because the specified GTS
                                                    either did not have a transmit GTS direction or was not defined. */

    MAC_INVALID_HANDLE              = 0xE7,     /*!< A request to purge an MSDU from the transaction queue was made
                                                    using an MSDU handle that was not found in the transaction table. */

    MAC_INVALID_PARAMETER           = 0xE8,     /*!< A parameter in the primitive is either not supported or is out of
                                                    the valid range. */

    MAC_NO_ACK                      = 0xE9,     /*!< No acknowledgment was received after macMaxFrameRetries. */

    MAC_NO_BEACON                   = 0xEA,     /*!< A scan operation failed to find any network beacons. */

    MAC_NO_DATA                     = 0xEB,     /*!< No response data were available following a request. */

    MAC_NO_SHORT_ADDRESS            = 0xEC,     /*!< The operation failed because a 16-bit short address was not
                                                    allocated. */

    MAC_OUT_OF_CAP                  = 0xED,     /*!< A receiver enable request was unsuccessful because it could not be
                                                    completed within the CAP (only for IEEE Std 802.15.4-2003). */

    MAC_PAN_ID_CONFLICT             = 0xEE,     /*!< A PAN identifier conflict has been detected and communicated to the
                                                    PAN coordinator. */

    MAC_REALIGNMENT                 = 0xEF,     /*!< A coordinator realignment command has been received. */

    MAC_TRANSACTION_EXPIRED         = 0xF0,     /*!< The transaction has expired and its information was discarded. */

    MAC_TRANSACTION_OVERFLOW        = 0xF1,     /*!< There is no capacity to store the transaction. */

    MAC_TX_ACTIVE                   = 0xF2,     /*!< The transceiver was in the transmitter enabled state
                                                    when the receiver was requested to be enabled
                                                    (only for IEEE Std 802.15.4-2003). */

    MAC_UNAVAILABLE_KEY             = 0xF3,     /*!< The key purportedly used by the originator of the received frame is
                                                    not available or, if available, the originating device is not known
                                                    or is blacklisted with that particular key. */

    MAC_UNSUPPORTED_ATTRIBUTE       = 0xF4,     /*!< A SET/GET request was issued with the identifier of a PIB attribute
                                                    that is not supported. */

    MAC_INVALID_ADDRESS             = 0xF5,     /*!< A request to send data was unsuccessful because neither the source
                                                    address parameters nor the destination address parameters
                                                    were present. */

    MAC_ON_TIME_TOO_LONG            = 0xF6,     /*!< A receiver enable request was unsuccessful because it specified
                                                    a number of symbols that was longer than the beacon interval. */

    MAC_PAST_TIME                   = 0xF7,     /*!< A receiver enable request was unsuccessful because it could not be
                                                    completed within the current superframe and was not permitted to be
                                                    deferred until the next superframe. */

    MAC_TRACKING_OFF                = 0xF8,     /*!< The device was instructed to start sending beacons based on the
                                                    timing of the beacon transmissions of its coordinator, but the
                                                    device is not currently tracking the beacon of its coordinator. */

    MAC_INVALID_INDEX               = 0xF9,     /*!< An attempt to write to a MAC PIB attribute that is in a table
                                                    failed because the specified table index was out of range. */

    MAC_LIMIT_REACHED               = 0xFA,     /*!< A scan operation terminated prematurely because the number of PAN
                                                    descriptors stored reached an implementationspecified maximum. */

    MAC_READ_ONLY                   = 0xFB,     /*!< A SET/GET request was issued with the identifier of an attribute
                                                    that is read only. */

    MAC_SCAN_IN_PROGRESS            = 0xFC,     /*!< A request to perform a scan operation failed because the MLME was
                                                    in the process of performing a previously initiated scan
                                                    operation. */

    MAC_SUPERFRAME_OVERLAP          = 0xFD,     /*!< The device was instructed to start sending beacons based on the
                                                    timing of the beacon transmissions of its coordinator, but the
                                                    instructed start time overlapped the transmission time of the beacon
                                                    of its coordinator. */
} MAC_Enumerations_t;

/**//**
 * \brief   Data type for the \c status parameter returned by the MAC-SAP confirmation primitives.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.2.1, 7.1.3.4.1, 7.1.6.2.1, 7.1.9.2.1, 7.1.10.2.1, 7.1.11.2.1, 7.1.13.2.1,
 *  7.1.14.2.1, tables 42, 50, 57, 64, 66, 68, 71, 73.
 */
typedef MAC_Enumerations_t  MAC_Status_t;

#endif /* _BB_MAC_SAP_DEFS_H */

/* eof bbMacSapDefs.h */