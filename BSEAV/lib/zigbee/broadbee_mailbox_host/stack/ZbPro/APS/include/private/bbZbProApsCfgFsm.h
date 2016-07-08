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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsCfgFsm.h $
*
* DESCRIPTION:
*   APS layer Finite State Machines description.
*
* $Revision: 9364 $
* $Date: 2016-01-04 18:21:51Z $
*
*****************************************************************************************/


#ifndef _BB_ZB_PRO_APS_CFG_FSM_H
#define _BB_ZB_PRO_APS_CFG_FSM_H


/************************* INCLUDES *****************************************************/
#include "bbSysFsm.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief FSMs states enumeration.
 */
enum
{
    S_START,
    APS_S_DECRYPTING,
    APS_S_UPDATING_SECURITY,
    APS_S_WAIT_EXTADDR,
    APS_S_WAIT_NWKADDR,
    APS_S_ENCRYPTING,
    APS_S_UNSECURE_SENDING,
    APS_S_UNSEC_TX_CONFIRMED,
    APS_S_UNSEC_TX_ACKED,
    APS_S_SECURE_SENDING,
    APS_S_SEC_TX_CONFIRMED,
    APS_S_SEC_TX_ACKED,
    APS_S_CONFIRMING,
    APS_S_SEARCHING_FOR_KEY,
    S_END,
    STATES_NUMBER,      /*!< The total number of all states.
                          This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs states.
 */
#if !(STATES_NUMBER <= SYS_FSM_STATE_FLAG)
# error The allowed number of FSM states is exceeded.
#endif


/**//**
 * \brief FSMs events enumeration.
 */
enum
{
    E_IN_STATE,
    E_ACK_GOT,
    E_TIMEOUT,
    E_ENC_CONFIRM,
    E_DEC_CONFIRM,
    E_CONFIRM,
    E_NWK_CONFIRM   = E_CONFIRM,
    E_ZDO_CONFIRM   = E_CONFIRM,
    E_ZDP_RESP,
    E_PROCEED,
    E_DELAY_COMPL,
    EVENTS_NUMBER,      /*!< The total number of all signals.
                          This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs events.
 */
#if !(EVENTS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM events is exceeded.
#endif


/**//**
 * \brief FSMs guards enumeration.
 */
enum
{
    APS_G_ACK_NOT_NEEDED,
    APS_G_ACK_REQUESTED,
    APS_G_CMD_PAIR_ENC_FAIL,
    APS_G_DUPLICATE,
    APS_G_FRAGMENTED,
    APS_G_LAST_ATTEMPT,
    APS_G_NO_EXTADDR_FOR_SEC,
    APS_G_NO_NWKADDR,
    APS_G_OUT_OF_NWK,
    APS_G_SECURED,
    APS_G_SECURITY_NEEDED,
    APS_G_STATUS_BAD,
    APS_G_TOO_LONG,
    APS_G_TUNNEL_REQUESTED,
    APS_G_DUPLICATE_REQUESTED,
    APS_G_NO_KEY,
    GUARDS_NUMBER,                  /*!< The total number of all guards.
                                      This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs guards.
 */
#if !(GUARDS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM guards is exceeded.
#endif


/*************************************************************************************//**
  \brief
    All Project FSMs actions enumeration.
*****************************************************************************************/
enum
{
    APS_A_CHANGE_KEY_TYPE,
    APS_A_CONFIRM,
    APS_A_SPLIT_HDR_AND_CONFIRM,
    APS_A_DECRYPT,
    APS_A_RAISE_LOG_ID,
    APS_A_DECRYPT_TO_CONFIRM,
    APS_A_DECRYPT_TO_RESEND,
    APS_A_HANDLE_DUPLICATE,
    APS_A_ENCRYPT,
    APS_A_INDICATE,
    APS_A_RESOLVE_EXTADDR,
    APS_A_RESOLVE_NWKADDR,
    APS_A_SECURE_SEND,
    APS_A_PUT_IN_ACK_WAIT_QUEUE,
    APS_A_ACK_WAIT_START,
    APS_A_POST_TO_PROCEED,
    APS_A_UNSECURE_RESEND,
    APS_A_UNSECURE_SEND,
    APS_A_UPDATE_SECURITY,
    APS_A_PREPARE_FOR_DECRIPTION,
    APS_A_DUPLICATE,
    APS_A_FIND_KEY,
    ACTIONS_NUMBER,                 /*!< The total number of all actions.
                                      This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSMs actions.
 */
#if !(ACTIONS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM actions is exceeded.
#endif

#endif /* _BB_ZB_PRO_APS_CFG_FSM_H */
