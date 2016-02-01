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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCConstants.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC profile constants.
 *
 * $Revision: 3272 $
 * $Date: 2014-08-15 09:13:13Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_CONSTANTS_H
#define _RF4CE_ZRC_CONSTANTS_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief ZRC profile version.
 */
#define RF4CE_ZRC_VERSION                                 0x0200
#define RF4CE_GDP_VERSION                                 0x0200

/**//**
 * \brief ZRC profile actions banks version.
 */
#define RF4CE_ZRC_ACTION_BANKS_VERSION                    0x0100

/**//**
 * \brief RF4CE ZRC Protocol IDs.
 */
#define RF4CE_ZRC_GDP1_COMPLIANT_PROTOCOL_ID              0x01
#define RF4CE_ZRC_GDP2_COMPLIANT_PROTOCOL_ID              0x03

/**//**
 * \brief RF4CE DGP/ZRC frame control field flags.
 */
#define RF4CE_GDP_FC_DATA_PENDING                         0x80
#define RF4CE_GDP_FC_GDP_COMMAND                          0x40
#define RF4CE_GDP_FC_IS_DATA_PENDING(v)                   (0 != ((v) & RF4CE_GDP_FC_DATA_PENDING))
#define RF4CE_GDP_FC_IS_GDP_COMMAND(v)                    (0 != ((v) & RF4CE_GDP_FC_GDP_COMMAND))
#define RF4CE_GDP_FC_GET_COMMAND_CODE(v)                  ((v) & 0x0F)
#define RF4CE_GDP_FC_SET_DATA_PENDING(v, isDataPending)   ((isDataPending) ? ((v) | RF4CE_GDP_FC_DATA_PENDING) : ((v) & ~RF4CE_GDP_FC_DATA_PENDING))
#define RF4CE_GDP_FC_SET_GDP_COMMAND(v, isGDPCommand)     ((isGDPCommand) ? ((v) | RF4CE_GDP_FC_GDP_COMMAND) : ((v) & ~RF4CE_GDP_FC_GDP_COMMAND))
#define RF4CE_GDP_FC_SET_COMMAND_CODE(v, commandCode)     (((v) & 0xF0) | ((commandCode) & 0x0F))
#define RF4CE_GDP_FC_MAKE(commandCode, isGDPCommand, isDataPending) \
    (((commandCode) & 0x0F) | ((isGDPCommand) ? RF4CE_GDP_FC_GDP_COMMAND : 0) | ((isDataPending) ? RF4CE_GDP_FC_DATA_PENDING : 0))

/**//**
 * \brief RF4CE GDP blackout timeout value.
 */
#define RF4CE_GDP_APLC_CONFIG_BLACKOUT_TIME               108 /* ms */

/**//**
 * \brief RF4CE GDP bind window duration.
 */
#define RF4CE_GDP_APLC_BIND_WINDOW_DURATION               30000 /* ms */

/**//**
 * \brief RF4CE GDP maximum allowed value for aplMaxAutoCheckValidationPeriod attribute.
 */
#define RF4CE_GDP_APLC_MAX_AUTO_CHECK_VALIDATION_PERIOD   10000 /* ms */

/**//**
 * \brief RF4CE GDP The maximum time the Binding Recipient shall wait to receive a
 *        command frame from a Binding Originator during its configuration phase.
 */
#define RF4CE_GDP_APLC_MAX_CONFIG_WAIT_TIME               100 /* ms */

/**//**
 * \brief RF4CE GDP The maximal time the validation can take in extended validation mode.
 */
#define RF4CE_GDP_APLC_MAX_EXTENDED_VALIDATION_DURATION         65000 /* ms */
#define RF4CE_GDP_APLC_EXTENDED_VALIDATION_DISCOVERY_PERIOD     5000 /* ms */

/**//**
 * \brief RF4CE GDP The maximal time the validation can take in normal validation mode.
 */
#define RF4CE_GDP_APLC_MAX_EXTENDED_VALIDATION_DURATION_FOR_RECIPIENT  50000 /* ms */

/**//**
 * \brief RF4CE GDP The maximal time the validation can take in normal validation mode.
 */
#define RF4CE_GDP_APLC_MAX_NORMAL_VALIDATION_DURATION     25000 /* ms */

/**//**
 * \brief RF4CE GDP The maximal time the validation can take in normal validation mode.
 */
#define RF4CE_GDP_APLC_MAX_NORMAL_VALIDATION_DURATION_FOR_RECIPIENT  15000 /* ms */

/**//**
 * \brief RF4CE GDP The maximum allowed value to configure the polling timeout in the
 *        aplPollConfiguration attribute.
 */
#define RF4CE_GDP_APLC_MAX_POLLING_TIMEOUT                100 /* ms */

/**//**
 * \brief RF4CE GDP The maximum time a node shall leave its receiver on in order to receive data indicated
 *        via the data pending subfield of the frame control field of an incoming frame.
 */
#define RF4CE_GDP_APLC_MAX_RXON_WAIT_TIME                 100 /* ms */

/**//**
 * \brief RF4CE GDP The maximum time a node shall wait for a response command frame following a request
 *        command frame.
 */
#define RF4CE_GDP_APLC_MAX_RESPONSE_WAIT_TIME             100 /* ms */
#define RF4CE_GDP_APLC_MAX_RESPONSE_WAIT_WARN_TIME        60  /* ms. More strict constraints to warn NVM issue */

/**//**
 * \brief RF4CE GDP The minimum value of the KeyExchangeTransferCount parameter passed to the pair request
 *        primitive during the validation based binding procedure.
 */
#define RF4CE_GDP_APLC_MIN_KEY_EXCHANGE_TRANSFER_COUNT    3

/**//**
 * \brief ZRC maximum time the Binding Recipient shall wait to receive a command frame from a Binding
 *        Initiator during its configuration phase.
 */
#define RF4CE_ZRC_APLC_MAX_CONFIG_WAIT_TIME               100 /* ms */

/**//**
 * \brief ZRC maximum time between consecutive actions command frame transmissions indicating a repeated action.
 */
#define RF4CE_ZRC_APLC_MAX_ACTION_REPEAT_TRIGGER_INTERVAL 200

/**//**
 * \brief The time that an action control record should be repeated.
 */
#define RF4CE_ZRC_APLC_SHORT_RETRY_DURATION               100 /* ms */

/**//**
 * \brief The ZRC 1.1 constants.
 */
#define RF4CE_ZRC1_APLC_MAX_CMD_DISC_RX_ON_DURATION     200 /* ms */
#define RF4CE_ZRC1_APLC_MAX_KEY_REPEAT_INTERVAL         100 /* ms */
#define RF4CE_ZRC1_APLC_MAX_PAIR_INDICATION_WAIT_TIME   1000 /* ms */
#define RF4CE_ZRC1_APLC_MAX_RESPONSE_WAIT_TIME          200 /* ms */
#define RF4CE_ZRC1_APLC_MIN_KEY_EXCHANGE_TRANSFER_COUNT 3
#define RF4CE_ZRC1_APLC_MIN_TARGET_BLACKOUT_PERIOD      500 /* ms */

#define RF4CE_ZRC1_DISCOVERY_REPETITION_INTERVAL        0x00f424
#define RF4CE_ZRC1_MAX_DISCOVERY_REPETITIONS            0x1e
#define RF4CE_ZRC1_MAX_REPORTED_NODE_DESCRIPTORS        1
#define RF4CE_ZRC1_DISCOVERY_DURATION                   0x00186a
#define RF4CE_ZRC1_AUTO_DISCOVERY_DURATION              0x1c9c38 /* (30 s) */
#define RF4CE_ZRC1_MAX_PAIR_INDICATION_WAIT_TIME        2000 /* (1 s but necessary to increase as we'll receive the indication ONLY after the pairing took place) */

#endif /* _RF4CE_ZRC_CONSTANTS_H */