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
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacCfgFsm.h $
*
* DESCRIPTION:
*   MAC layer FSMs integral description.
*
* $Revision: 10910 $
* $Date: 2016-04-06 14:25:51Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_CFG_FSM_H
#define _BB_MAC_CFG_FSM_H


/************************* INCLUDES *****************************************************/
#include "bbSysFsm.h"               /* FSM engine interface. */
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   FSM common states.
 */
enum
{
    /* Valid state IDs are 00-7F. Bit #7 is used by FSM engine as BUSY flag. */

    S_IDLE = 0,  ST_MFE_IDLE = 0,       /*!< IDLE state. */
};


/**//**
 * \brief   MAC-FE sublayer FSM states enumeration.
 */
enum
{
    /* States relating to frame transmission via the MAC-LE. */

    S_WAIT_MAC_LE_TX_CONF = 40,                         /*!< Wait for confirmation from the MAC-LE
                                                            on request to transmit a MAC frame. */

    S_WAIT_MAC_LE_TX_CONF_DIRECT,                       /*!< Wait for confirmation from the MAC-LE
                                                            on request to transmit a MAC frame
                                                            for the case of direct transmission. */

    S_WAIT_MAC_LE_TX_CONF_PENDING,                      /*!< Wait for confirmation from the MAC-LE
                                                            on request to transmit a pending MAC frame
                                                            for the case of indirect transmission. */

    S_WAIT_MAC_LE_TX_ASSOC_REQ_CONF,                    /*!< Wait for confirmation from the MAC-LE on request
                                                            to transmit an Association Request MAC Command frame. */

    S_WAIT_MAC_LE_TX_BEACON_REQ_CONF,                   /*!< Wait for confirmation from the MAC-LE on request
                                                            to transmit a Beacon Request MAC Command frame. */

    S_WAIT_MAC_LE_TX_DATA_REQ_CONF,                     /*!< Wait for confirmation from the MAC-LE on request
                                                            to transmit a Data Request MAC Command frame. */

    S_WAIT_MAC_LE_TX_POLL_DATA_IND,                     /*!< Wait for Data indication during the MLME-POLL.request
                                                             executing. */

    /* States relating to channel switching via the MAC-LE. */

    S_WAIT_MAC_LE_SET_CHANNEL_CONF,                     /*!< Wait for confirmation from the MAC-LE on request
                                                            to set new PHY channel and channel page. */

    S_WAIT_MAC_LE_SET_NEXT_CHANNEL_CONF_ED,             /*!< Wait for confirmation from the MAC-LE on request
                                                            to set new PHY channel and channel page for the case
                                                            of next channel assignment during the ED scan. */

    S_WAIT_MAC_LE_SET_NEXT_CHANNEL_CONF_ACTIVE,         /*!< Wait for confirmation from the MAC-LE on request
                                                            to set new PHY channel and channel page for the case
                                                            of next channel assignment during the Active scan. */

    S_WAIT_MAC_LE_SET_ORIG_CHANNEL_CONF_ED,             /*!< Wait for confirmation from the MAC-LE on request
                                                            to set new PHY channel and channel page for the case
                                                            of original channel assignment during the ED scan. */

    S_WAIT_MAC_LE_SET_ORIG_CHANNEL_CONF_ACTIVE,         /*!< Wait for confirmation from the MAC-LE on request
                                                            to set new PHY channel and channel page for the case
                                                            of original channel assignment during the Active scan. */

    /* States relating to Energy Detection scan via the MAC-LE. */

    S_WAIT_MAC_LE_CONTINUOUS_PHY_ED_CONF,               /*!< Wait for confirmation from the MAC-LE on request
                                                            to perform continuous PHY Energy Detection scan. */

    /* States relating to prolonged procedures. */

    S_DELAY_MAC_RESPONSE_WAIT_TIME,                     /*!< Delay MAC-FE for macResponseWaitTime period. */

    S_DELAY_MAC_SCAN_DURATION,                          /*!< Delay MAC-FE for ScanDuration period. */

    /* Transient states. */

    S_CONTINUE_MAC_SCAN_ED,                             /*!< Transient state to start or continue the ED scan. */

    S_CONTINUE_MAC_SCAN_ACTIVE,                         /*!< Transient state to start or continue the Active scan. */

    S_CONTINUE_MAC_ISSUE_CONF,                          /*!< Transient state to issue confirmation. */

    S_PROCEED_ISSUE_ASSOC_CONF,                         /*!< Transient state to issue confirmation for Association. */
};


/**//**
 * \brief   FSM common events.
 */
enum
{
    E_CONFIRM = 0,  EV_COMPLETE = 0,                /*!< Confirms execution of a request to MAC-LE from MAC-LE. */
    E_INDICATION,   EV_RECEIVE = E_INDICATION,      /*!< Indicates reception of response frame from MAC-LE to MAC-FE. */
    E_TIMEOUT,      EV_TIMEOUT = E_TIMEOUT,         /*!< Signals timeout from the Symbol Timer. */
    E_START,        EV_START = E_START,             /*!< Starts the new request/response processing. */

    E_PROCEED,      EV_PROCEED = E_PROCEED,         /*!< Proceeds with the transient FSM state. */
};


/**//**
 * \brief   MAC-FE sublayer FSM guards enumeration.
 */
enum
{
    /* Guards relating to requests parameters validation. */

    MAC_IF_INVALID_PARAMETER,               /*!< If request/response parameter(s) is(are) invalid. */

    MAC_IF_INVALID_ADDRESS,                 /*!< If Src. and Dst. address modes of MCPS-DATA.request are invalid. */

    MAC_IF_INVALID_HANDLE,                  /*!< If an MSDU handle was not found in the transaction table. */

    MAC_IF_UNSUPPORTED_ATTRIBUTE,           /*!< If an attribute specified with MLME-GET/SET.request is not supported
                                                neither by the MAC-PIB nor by the PHY-PIB. */

    MAC_IF_READ_ONLY,                       /*!< If an attribute specified with MLME-SET.request is read-only. */

    MAC_IF_UNSUPPORTED_SECURITY,            /*!< If a secured transmission is requested. */

    MAC_IF_INVALID_GTS,                     /*!< If a transmission during GTS is requested. */

    MAC_IF_TRANSACTION_OVERFLOW,            /*!< If an indirect transmission is requested for RF4CE context. */

    /* Guards relating to conditional requests processing. */

    MAC_IF_SET_DEFAULT_PIB,                 /*!< If MLME-RESET.request is commenced with Set Default PIB parameter. */

    MAC_IF_SET_PHY_CHANNEL,                 /*!< If MLME-SET.request is commenced for macCurrentChannel attribute. */

    MAC_IF_SET_PHY_PAGE,                    /*!< If MLME-SET.request is commenced for macCurrentPage attribute. */

    MAC_IF_SCAN_TYPE_ED,                    /*!< If MLME-SCAN.request is commenced for Energy Detection scan. */

    MAC_IF_SCAN_TYPE_ACTIVE,                /*!< If MLME-SCAN.request is commenced for Active scan. */

    MAC_IF_HAVE_ANOTHER_CHANNEL,            /*!< If there is another channel to scan for the MLME-SCAN.request. */

    MAC_IF_NOT_ORIGINAL_CHANNEL,            /*!< If after completion of MLME-SCAN.request the current channel or page
                                                differ from the original channel and page. */

    MAC_IF_NOT_AUTO_REQUEST,                /*!< If MAC-PIB attribute macAutoRequest equals to FALSE. */

    MAC_IF_NOT_ALLOC_ED_RESULT,             /*!< If during MLME-SCAN.request processing there is a lack of the dynamic
                                                memory to allocate a chunk for storing the Energy Detect List. */

    MAC_IF_NOT_ALLOC_PAN_DESCR,             /*!< If during MLME-SCAN.request processing there is a lack of the dynamic
                                                memory to allocate a chunk for storing the next PAN Descriptor. */

    MAC_IF_LIMIT_EXCEEDED,                  /*!< If during MLME-SCAN.request processing the maximum allowed number
                                                (limit) of stored beacons is exceeded. */

    MAC_IF_LIMIT_REACHED,                   /*!< If during MLME-SCAN.request processing there is a lack of the dynamic
                                                memory, or the number of gathered beacons overruns the limit. */

    MAC_IF_NO_BEACON,                       /*!< If during MLME-SCAN.request processing for the case of Active scan
                                                no beacons were received. */

    MAC_IF_NO_SHORT_ADDRESS,                /*!< If MLME-START.request is commenced when MAC-PIB attribute
                                                macShortAddress is equal to 0xFFFF. */

    MAC_IF_DIRECT_REQUEST,                  /*!< If MCPS-DATA.request is commenced for Direct data transmission. */

    MAC_IF_NEW_REQUEST,                     /*!< If MCPS-DATA.request being processed is new one, not pending. */

    MAC_IF_NEW_RESPONSE,                    /*!< If MLME-ASSOCIATE.response being processed is new one, not pending. */

    MAC_IF_TRANSACTION_EXPIRED,             /*!< If pending transaction has expired. */

    /* Guards relating to MAC-LE confirmations processing. */

    MAC_IF_SUCCESS,                         /*!< If successful confirmation is received from the MAC-LE. */

    MAC_IF_NOT_SUCCESS,                     /*!< If unsuccessful confirmation is received from the MAC-LE. */

    MAC_IF_CHANNEL_ACCESS_FAILURE,          /*!< If MAC-LE confirmed 'Channel access failure' on attempt to transmit. */

    MAC_IF_NO_ACK,                          /*!< If MAC-LE confirmed 'No acknowledgment' on attempt to transmit. */

    MAC_IF_NO_FRAME_PENDING,                /*!< If ACK frame received on transmitted Data Request MAC Command frame
                                                has the 'Frame Pending' subfield clear. */

    MAC_IF_POLL_ZERO_LENGTH,                /*!< If Data Frame with zero length payload was recieved. */

    MAC_IF_POLL_INCORRECT_IND,              /*!< If the recieved Data Indication is a correct for the MLME-POLL.request. */

    /* Guards relating to MAC-LE indications processing. */

    MAC_IF_BEACON_DIFF_CHANNEL,             /*!< If received Beacon frame is from another PHY channel. */

    MAC_IF_BEACON_NOT_UNIQUE,               /*!< If received Beacon frame is not unique. */

    MAC_IF_DATA_REQ_NOT_CONFIRMED,          /*!< If Data Request was not confirmed yet. */

    MAC_IF_ASSOC_RESP_NOT_RECEIVED,         /*!< If no Association Response was received yet that coincides with
                                                MLME-ASSOCIATION.request being processed. */

    MAC_IF_ASSOC_PAN_AT_CAPACITY,           /*!< If received Association Response MAC Command frame has
                                                association status 'PAN at capacity'. */

    MAC_IF_ASSOC_PAN_ACCESS_DENIED,         /*!< If received Association Response MAC Command frame has
                                                association status 'PAN access denied'. */

    MAC_IF_FRAME_TOO_LONG,                  /*!< If the frame is too long for sending. */

    MAC_IF_COUNTER_ERROR,                   /*!< If there is a COUNTER ERROR fail during processing. */

    MAC_IF_UNAVAILABLE_KEY,                 /*!< If the security procedure returned UNAVAILABLE_KEY status. */

    MAC_IF_NO_MEMORY,                       /*!< If it is not enough memory to proceed. */
};


/**//**
 * \brief   MAC-FE sublayer FSM actions enumeration.
 */
enum
{
    /* Actions for issuing confirmations with different statuses with codes from 0x00 to 0x02. */

    MAC_DO_SAP_CONF_SUCCESS           = MAC_SUCCESS,                    /*!< Issue successful confirmation. */

    MAC_DO_SAP_CONF_NOT_SUCCESS,                                        /*!< Issue confirmation with come fail status. */

    MAC_DO_SAP_CONF_PAN_AT_CAPACITY   = MAC_PAN_AT_CAPACITY,            /*!< Issue confirmation with error status
                                                                            'PAN at capacity'. */

    MAC_DO_SAP_CONF_PAN_ACCESS_DENIED = MAC_PAN_ACCESS_DENIED,          /*!< Issue confirmation with error status
                                                                            'PAN access denied'. */

    /* Actions to be performed by the MAC-FE. */

    MAC_DO_FE_COMMENCE_SCAN,                    /*!< Preserve MAC state and configure MAC for scanning. */

    MAC_DO_FE_SET_LIMIT_REACHED,                /*!< Set Limit Reached flag and clear the set Scan Channels
                                                    in the MLME-SCAN.request FSM extended memory. */

    MAC_DO_FE_SET_HAVE_BEACON,                  /*!< Set Have Beacon flag in the scan FSM extended memory. */

    MAC_DO_FE_SAVE_NEW_BEACON,                  /*!< Save newly received Beacon frame in the scan results list. */

    MAC_DO_FE_SAVE_RESULT_SCAN_ED,              /*!< Finally save the Energy Detection scan results. */

    MAC_DO_FE_SAVE_RESULT_SCAN_ACTIVE,          /*!< Finally save the Active scan results. */

    MAC_DO_FE_CREATE_TRANSACTION,               /*!< Create new transaction for indirect transmission. */

    MAC_DO_FE_RETURN_TRANSACTION,               /*!< Return activated transaction back to the queue. */

    MAC_DO_FE_PURGE_TRANSACTIONS,               /*!< Purge transactions by handle value. */

    MAC_DO_FE_DELAY_FOR_RESPONSE_WAIT_TIME,     /*!< Perform delay for macResponseWaitTime symbols. */

    MAC_DO_FE_WAIT_FOR_RX_ASSOC_RESP_CMD,       /*!< Save fact that Data Request was confirmed and start timed waiting
                                                    for Association Response MAC Command frame. */

    MAC_DO_FE_SAVE_ASSOC_RESP_PARAMS,           /*!< Save parameters of the received Association Response. */

    MAC_DO_FE_DELAY_FOR_SCAN_DURATION,          /*!< Perform delay for ScanDuration period. */

    MAC_DO_FE_RESET_DISPATCHER_AND_PIB,         /*!< Reset MAC-FE Requests Queues and MAC PIB. */

    MAC_DO_FE_RESET_DISPATCHER_ONLY,            /*!< Reset MAC-FE Requests Queues, but preserve MAC PIB. */

    /* Actions to be performed by the MAC-LE. */

    MAC_DO_LE_TX_BEACON,                    /*!< Commence transmission of Beacon frame. */

    MAC_DO_LE_TX_DATA_DIRECT,               /*!< Commence transmission of direct Data frame. */

    MAC_DO_LE_TX_DATA_PENDING,              /*!< Commence transmission of pending Data frame. */

    MAC_DO_LE_TX_ASSOC_REQ_CMD,             /*!< Commence transmission of Association Request MAC Command frame. */

    MAC_DO_LE_TX_ASSOC_RESP_CMD,            /*!< Commence transmission of Association Response MAC Command frame. */

    MAC_DO_LE_TX_BEACON_REQ_CMD,            /*!< Commence transmission of Beacon Request MAC Command frame. */

    MAC_DO_LE_TX_DATA_REQ_CMD,              /*!< Commence transmission of Data Request MAC Command frame. */

    MAC_DO_LE_TX_COORD_REALIGNMENT_CMD,     /*!< Commence transmission of Coordinator Realignment MAC Command frame. */

    MAC_DO_LE_SET_PHY_CHANNEL_REQ,          /*!< Set PHY channel and channel page. */

    MAC_DO_LE_SET_PHY_PAGE_REQ,             /*!< Set PHY channel page and the first channel on it. */

    MAC_DO_LE_SET_NEXT_PHY_CHANNEL_REQ,     /*!< Set next PHY channel and channel page to scan. */

    MAC_DO_LE_SET_ORIG_PHY_CHANNEL_REQ,     /*!< Set original PHY channel and channel page that were prior to scan. */

    MAC_DO_LE_CONTINUOUS_PHY_ED_REQ,        /*!< Commence continuous PHY Energy Detection measurement. */

    /* Actions for issuing confirmations with different statuses with codes from 0xDB to 0xFF. */

    MAC_DO_SAP_CONF_UNSUPPORTED_SECURITY   = MAC_UNSUPPORTED_SECURITY,      /*!< Issue confirmation with error status
                                                                                UNSUPPORTED_SECURITY (0xDF). */

    MAC_DO_SAP_CONF_CHANNEL_ACCESS_FAILURE = MAC_CHANNEL_ACCESS_FAILURE,    /*!< Issue confirmation with error status
                                                                                CHANNEL_ACCESS_FAILURE (0xE1). */

    MAC_DO_SAP_CONF_INVALID_GTS            = MAC_INVALID_GTS,               /*!< Issue confirmation with error status
                                                                                INVALID_GTS (0xE6). */

    MAC_DO_SAP_CONF_INVALID_HANDLE         = MAC_INVALID_HANDLE,            /*!< Issue confirmation with error status
                                                                                INVALID_HANDLE (0xE7). */

    MAC_DO_SAP_CONF_INVALID_PARAMETER      = MAC_INVALID_PARAMETER,         /*!< Issue confirmation with error status
                                                                                INVALID_PARAMETER (0xE8). */

    MAC_DO_SAP_CONF_NO_ACK                 = MAC_NO_ACK,                    /*!< Issue confirmation with error status
                                                                                NO_ACK (0xE9). */

    MAC_DO_SAP_CONF_NO_BEACON              = MAC_NO_BEACON,                 /*!< Issue confirmation with error status
                                                                                NO_BEACON (0xEA). */

    MAC_DO_SAP_CONF_NO_DATA                = MAC_NO_DATA,                   /*!< Issue confirmation with error status
                                                                                NO_DATA (0xEB). */

    MAC_DO_SAP_CONF_NO_SHORT_ADDRESS       = MAC_NO_SHORT_ADDRESS,          /*!< Issue confirmation with error status
                                                                                NO_SHORT_ADDRESS (0xEC). */

    MAC_DO_SAP_CONF_TRANSACTION_EXPIRED    = MAC_TRANSACTION_EXPIRED,       /*!< Issue confirmation with error status
                                                                                TRANSACTION_EXPIRED (0xF0). */

    MAC_DO_SAP_CONF_TRANSACTION_OVERFLOW   = MAC_TRANSACTION_OVERFLOW,      /*!< Issue confirmation with error status
                                                                                TRANSACTION_OVERFLOW (0xF1). */

    MAC_DO_SAP_CONF_UNSUPPORTED_ATTRIBUTE  = MAC_UNSUPPORTED_ATTRIBUTE,     /*!< Issue confirmation with error status
                                                                                UNSUPPORTED_ATTRIBUTE (0xF4). */

    MAC_DO_SAP_CONF_INVALID_ADDRESS        = MAC_INVALID_ADDRESS,           /*!< Issue confirmation with error status
                                                                                INVALID_ADDRESS (0xF5). */

    MAC_DO_SAP_CONF_LIMIT_REACHED          = MAC_LIMIT_REACHED,             /*!< Issue confirmation with error status
                                                                                LIMIT_REACHED (0xFA). */

    MAC_DO_SAP_CONF_READ_ONLY              = MAC_READ_ONLY,                 /*!< Issue confirmation with error status
                                                                                READ_ONLY (0xFB). */
};


#endif /* _BB_MAC_CFG_FSM_H */
