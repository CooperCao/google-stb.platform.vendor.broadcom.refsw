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
 *      RF4CE NWK Pair Originator Finite State Machines description.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_PAIR_FSM_H
#define _RF4CE_NWK_PAIR_FSM_H

/************************* INCLUDES *****************************************************/
#include "bbSysFsm.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Pairing Originator Side FSMs states enumeration.
 */
typedef enum _RF4CE_NWK_PairOriginatorStates_t
{
    RF4CE_ORG_S_IDLE = 0,
    RF4CE_ORG_S_PREPARATIONS,
    RF4CE_ORG_S_PAIR_WAIT_CONF_AND_RESP,
    RF4CE_ORG_S_WAIT_KEY_SEEDS,
    RF4CE_ORG_S_PING_WAIT_CONF_AND_RESP,
    RF4CE_ORG_S_FINISH,
    RF4CE_ORG_S_CONFIRMING,
    RF4CE_ORG_S_PING,
} RF4CE_NWK_PairOriginatorStates_t;

/**//**
 * \brief Pairing Originator FSMs events enumeration.
*/
typedef enum _RF4CE_NWK_PairOriginatorEvents_t
{
    RF4CE_ORG_E_IN_STATE = 0,
    RF4CE_ORG_E_START,
    RF4CE_ORG_E_RX_ENABLE_CONF,
    RF4CE_ORG_E_SET_PANID_CONF,
    RF4CE_ORG_E_SET_SHORT_CONF,
    RF4CE_ORG_E_SET_RETRIES_CONF,
    RF4CE_ORG_E_SET_BACKOFF_CONF,
    RF4CE_ORG_E_SET_CHANNEL_CONF,
    RF4CE_ORG_E_SEND_PAIR_FAIL,
    RF4CE_ORG_E_SEND_PAIR_SUCCESS,
    RF4CE_ORG_E_PAIR_RESP,
    RF4CE_ORG_E_PING_SUCCESS,
    RF4CE_ORG_E_PING_FAIL,
    RF4CE_ORG_E_RECOVER_CHANNEL_CONF,
    RF4CE_ORG_E_NVM_CONF,
    RF4CE_ORG_E_FAIL,
    RF4CE_ORG_E_KEY_SEED_IND,
    RF4CE_ORG_E_LAST_KEY_SEED,
    RF4CE_ORG_E_PING_REQ_READY,
    RF4CE_ORG_E_PING_RESP,
    RF4CE_ORG_E_TIMEOUT,
} RF4CE_NWK_PairOriginatorEvents_t;

/**//**
 * \brief Pairing Originator FSMs guards enumeration.
*/

typedef enum _RF4CE_NWK_PairOriginatorGuards_t
{
    RF4CE_ORG_G_INVALID_PARAMETER   = 0,
    RF4CE_ORG_G_NO_ORG_CAPACITY,
    RF4CE_ORG_G_BUILD_RESPONSE_FAIL,
    RF4CE_ORG_G_DIFF_CHANNEL,
    RF4CE_ORG_G_HAS_RESPONSE,
    RF4CE_ORG_G_LAST_CHANNEL,
    RF4CE_ORG_G_NEED_WAIT_RESPONSE,
    RF4CE_ORG_G_PAIR_RESP_FAIL,
    RF4CE_ORG_G_KEY_SEEDS_NOT_EXPECTED,
    RF4CE_ORG_G_CHANNEL_CHANGED,
    RF4CE_ORG_G_KEY_SEED_ERROR,
    RF4CE_ORG_G_LAST_KEY_SEED,
    RF4CE_ORG_G_SUCCESS,
    RF4CE_ORG_G_RECOVER_CHANNEL,
    RF4CE_ORG_G_BUILD_PING_FAIL,
    RF4CE_ORG_G_PING_RSP_FAIL,
    RF4CE_ORG_G_NEED_WAIT_CONF,
} RF4CE_NWK_PairOriginatorGuards_t;

/**//**
  \brief Pairing Originator FSMs actions enumeration.
*/
typedef enum _RF4CE_NWK_PairOriginatorActions_s
{
    RF4CE_ORG_A_RX_ENABLE_REQ = 0,
    RF4CE_ORG_A_SET_PANID_REQ,
    RF4CE_ORG_A_SET_SHORT_REQ,
    RF4CE_ORG_A_RX_DISABLE_REQ,
    RF4CE_ORG_A_SET_RETRIES_REQ,
    RF4CE_ORG_A_SET_BACKOFF_REQ,
    RF4CE_ORG_A_SET_CHANNEL_REQ,
    RF4CE_ORG_A_SEND_PAIR_REQ,
    RF4CE_ORG_A_PAIR_STATUS_FROM_MAC,
    RF4CE_ORG_A_SET_NEXT_CHANNEL,
    RF4CE_ORG_A_START_RESP_TIMEOUT,
    RF4CE_ORG_A_RESEND_PAIR_REQ,
    RF4CE_ORG_A_SET_NO_RESPONSE,
    RF4CE_ORG_A_SET_CORRECT_CHANNEL,
    RF4CE_ORG_A_START_KEY_SEED_TIMEOUT,
    RF4CE_ORG_A_STOP_TIMEOUT,
    RF4CE_ORG_A_SET_SECURITY_TIMEOUT,
    RF4CE_ORG_A_PING_STATUS_FROM_MAC,
    RF4CE_ORG_A_FINISH,
    RF4CE_ORG_A_STORE_TO_NVM,
    RF4CE_ORG_A_START_RECOVER_CHANNEL_TIMEOUT,
    RF4CE_ORG_A_RECOVER_CHANNEL,
    RF4CE_ORG_A_SEND_CONFIRM,
    RF4CE_ORG_A_SEND_PING_REQ,
    RF4CE_ORG_A_RAISE_NOT_PERMITTED,
} RF4CE_NWK_PairOriginatorActions_s;



/**//**
 * \brief Pairing Recipient FSMs states enumeration.
 */
typedef enum _RF4CE_NWK_PairRecipientStates_t
{
    RF4CE_REC_S_IDLE                = 0,
    RF4CE_REC_S_COMM_STATUS,
    RF4CE_REC_S_KEY_SEEDS_PREPARE,
    RF4CE_REC_S_KEY_SEEDS,
    RF4CE_REC_S_KEY_SEEDS_CYCLE,
    RF4CE_REC_S_PING_WAITING,
    RF4CE_REC_S_KEY_SEEDS_FINISH,
    RF4CE_REC_S_PING_REQUEST,
} RF4CE_NWK_PairRecipientStates_t;

/**//**
 * \brief Pairing Recipient FSMs events enumeration.
 */
typedef enum _RF4CE_NWK_PairRecipientEvents_t
{
    RF4CE_REC_E_IN_STATE = 0,
    RF4CE_REC_E_PAIR_REQ,
    RF4CE_REC_E_SEND_PAIR_RESP,
    RF4CE_REC_E_SET_RETRIES_CONF,
    RF4CE_REC_E_SET_BACKOFF_CONF,
    RF4CE_REC_E_PAIR_RESP_SUCCESS,
    RF4CE_REC_E_GET_TX_CONF,
    RF4CE_REC_E_SET_TX_POWER_CONF,
    RF4CE_REC_E_KEY_SEED_CONF,
    RF4CE_REC_E_RESTORE_TX_POWER_CONF,
    RF4CE_REC_E_RESTORE_RETRIES_CONF,
    RF4CE_REC_E_PING_REQUEST_IND,
    RF4CE_REC_E_PING_RESP_READY,
    RF4CE_REC_E_PING_RESP_CONF,
    RF4CE_REC_E_FAIL,
    RF4CE_REC_E_TIMEOUT,
} RF4CE_NWK_PairRecipientEvents_t;

/**//**
 * \brief Pairing Recipient FSMs guards enumeration.
 */
typedef enum _RF4CE_NWK_PairRecipientGuards_t
{
    RF4CE_REC_G_PAIR_REQ_FAIL = 0,
    RF4CE_REC_G_IS_AUTORESPONSE,
    RF4CE_REC_G_BUILD_RESPONSE_FAIL,
    RF4CE_REC_G_NEED_KEY_SEEDS,
    RF4CE_REC_G_BUILD_KEY_SEED_FAIL,
    RF4CE_REC_G_LAST_KEY_SEED,
    RF4CE_REC_G_PING_REQ_FAIL,
    RF4CE_REC_G_BUILD_PING_RESP_FAIL,
    RF4CE_REC_G_PING_RECEIVED,
    RF4CE_REC_G_KEY_SEED_FAILED,
    RF4CE_REC_G_NEED_PING_RESP,
    RF4CE_REC_G_KEY_SEED_NO_MEMORY,
} RF4CE_NWK_PairRecipientGuards_t;

/**//**
 * \brief Pairing Recipient FSMs actions enumeration.
 */
typedef enum _RF4CE_NWK_PairRecipientActions_t
{
    RF4CE_REC_A_PREPARE_RESPONSE        = 0,
    RF4CE_REC_A_RAISE_PAIR_REQ_IND,
    RF4CE_REC_A_SET_RETRIES_REQ,
    RF4CE_REC_A_SET_BACKOFF_REQ,
    RF4CE_REC_A_SEND_PAIR_RESP,
    RF4CE_REC_A_GET_TX_POWER_REQ,
    RF4CE_REC_A_SET_TX_POWER_REQ,
    RF4CE_REC_A_SET_RETRIES_ZERO_REQ,
    RF4CE_REC_A_START_KEY_SEEDING,
    RF4CE_REC_A_SEND_KEY_SEED_REQ,
    RF4CE_REC_A_KEY_SEEDS_SUCCESS,
    RF4CE_REC_A_PING_RECEIVED,
    RF4CE_REC_A_SET_SECURITY_TIMEOUT,
    RF4CE_REC_A_SET_SECURITY_FAILURE,
    RF4CE_REC_A_SEND_PING_RESP,
    RF4CE_REC_A_RESTORE_TX_POWER_REQ,
    RF4CE_REC_A_RESTORE_RETRIES_REQ,
    RF4CE_REC_A_RISE_COMM_STATUS_IND,
    RF4CE_REC_A_KEY_SEEDS_DECREMENT,
    RF4CE_REC_A_RECOVER_KEY_SEED,
} RF4CE_NWK_PairRecipientActions_t;


#endif /* _RF4CE_NWK_PAIR_FSM_H */

/* eof bbRF4CENWKPairFsm.h */