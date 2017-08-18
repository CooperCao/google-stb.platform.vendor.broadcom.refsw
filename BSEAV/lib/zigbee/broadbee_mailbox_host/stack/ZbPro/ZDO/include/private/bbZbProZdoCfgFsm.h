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

/******************************************************************************
*
* DESCRIPTION:
*       ZDO layer Finite State Machines integral description.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDO_CFG_FSM_H
#define _BB_ZBPRO_ZDO_CFG_FSM_H


/************************* INCLUDES *****************************************************/
#include "bbSysFsm.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of states of ZDO Dispatcher FSM and all ZDP Services FSMs.
 */
enum
{
    /* Common-use states. */
    S_IDLE,
    S_LEAVING,

    /* End-Device Bind states. */
    S_IDLE_WAITING_FIRST_END_DEVICE_BIND_REQ,       /*!< End Device Binding Manager is idle and waiting infinitely for
                                                        the first remote End_Device_Bind request. Note: this and the
                                                        following two states must have the lowest indexes in the list
                                                        of all states belonging to the End Device Bind Manager FSM. */

    S_IDLE_WAITING_SECOND_END_DEVICE_BIND_REQ,      /*!< End Device Binding Manager is still idle but engaged with the
                                                        first remote End_Device_Bind request received, waiting for the
                                                        timeout period for the second remote End_Device_Bind request. */

    S_WAITING_TEST_UNBIND_RSP,                      /*!< Waiting for the Unbind_rsp on the issued test Unbind_req. */

    S_WAITING_MATCHED_CLUSTER_BIND_RSP,             /*!< Waiting for the Bind_rsp on the Bind_req issued for a matched
                                                        cluster. */

    S_WAITING_MATCHED_CLUSTER_UNBIND_RSP,           /*!< Waiting for the Unbind_rsp on the Unbind_req issued for a
                                                        matched cluster. */

    /* Security Manager states. */
    S_DEFAULT_TC_LINK_KEY,
    S_CHECK_CONFIGURATION,
    S_WAIT_FOR_DUMMY_KEY,
    S_WAIT_FOR_NETWORK_KEY,
    S_AUTHENTICATED,
    S_SWITCHING_KEY,
    S_AUTHENTICATING_CHILD,

    /* Network Manager states. */
    S_PREPARE,
    S_JOINING,
    S_AUTHENTICATION,
    S_AUTHENTICATION_LEAVING,
    S_SETTINIG_PERMIT_JOINING_WINDOW,
    S_STARTING_ROUTER,
    S_STARTING_APS,
    S_STORING_ATTR,
    S_SENDING_DEVICE_ANNCE,
    S_STOPING_APS,

    /* Channel Manager */
    S_SCANING,
    S_CHANGING,

    ZDO_TRANSIENT_STATES,                           /*!< Start of the section of transient states. All the transient
                                                        states' identifiers must follow this identifier. */

    /* End-Device Bind states. */
    T_END_DEVICE_BIND_START_WAITING_TIMEOUT,        /*!< Start timeout timer for period of waiting for the second remote
                                                        End_Device_Bind request. */

    T_END_DEVICE_BIND_RECALL_TIMEOUT_TIMER,         /*!< Recall the timeout timer of waiting for the second remote
                                                        End_Device_Bind request. */

    T_END_DEVICE_BIND_MATCH_REQUESTS,               /*!< Match two End_Device_Bind requests. */

    T_END_DEVICE_BIND_PROCESS_MATCHED_STATUS,       /*!< Process the matched status of two requests. */

    T_END_DEVICE_BIND_ISSUE_TEST_UNBIND_REQ,        /*!< Compose and issue the test Unbind_req command for one of
                                                        matched clusters in order to discover if to bind or unbind all
                                                        other clusters. */

    T_END_DEVICE_BINDING_PROCEED,                   /*!< Start or proceed the binding procedure. Issue ZDO Bind request
                                                        for the next matched cluster. */

    T_END_DEVICE_BINDING_IS_CONTINUE,               /*!< Discover if there are more matched clusters that are not bound
                                                        yet, and if have to continue the binding procedure. */

    T_END_DEVICE_UNBINDING_PROCEED,                 /*!< Start or proceed the unbinding procedure. Issue ZDO Unbind
                                                        request for the next matched cluster. */

    T_END_DEVICE_UNBINDING_IS_CONTINUE,             /*!< Discover if there are more matched clusters that are not
                                                        unbound yet, and if have to continue the unbinding procedure. */

    STATES_NUMBER,      /*!< The total number of all states. This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSM states.
 */
#if !(STATES_NUMBER <= SYS_FSM_STATE_FLAG)
# error The allowed number of FSM states is exceeded.
#endif


/**//**
 * \brief   Enumeration of events passed to ZDO Dispatcher FSM and all ZDP Services FSMs.
 */
enum
{
    /* Common-use events. */
    E_TIMEOUT,                              /*!< Signals the timeout event. */

    E_PROCEED,                              /*!< Performs next iteration of FSM to jump from one of transient states. */

    /* End-Device Bind events. */
    E_END_DEVICE_BIND_REQ_RECEIVED,         /*!< Signals reception of new remote End_Device_Bind request. */

    E_TIMEOUT_WAITING_SECOND_REQ,           /*!< Signals timeout of waiting for the second End_Device_Bind request. */

    E_BIND_UNBIND_RSP_RECEIVED,             /*!< Signals reception of (Un)Bind_rsp on previously issued (Un)Bind_req. */

    /* Security Manager events. */
    E_START,
    E_SCROLL,
    E_TRANSPORT_KEY_IND,
    E_SWITCH_KEY_IND,
    E_REMOVE_DEVICE_IND,
    E_AUTHENTICATE_DEVICE,
    E_UPDATE_DEVICE_SENT,
    E_TUNNELED_CMD_SENT,

    /* Network Manager events. */
    E_EXECUTE,
    E_NEW_REQUEST,
    E_JOINED,
    E_AUTH,
    E_AUTH_LEAVING,
    E_PERMIT_JOIN,
    E_START_ROUTER,
    E_APS_START,
    E_NVM_DONE,
    E_ANNCE,
    E_ANNCE_REQ,
    E_LEAVE_IND,
    E_LEAVE,
    E_APS_STOP,
    E_NO_POSTPONED,

    /* Channel Manager */
    E_UPDATE,
    E_SCAN_DONE,
    E_CHANGE_OK,
    E_CHANGE_ERR,
    E_TIMER,

    EVENTS_NUMBER,      /*!< The total number of all signals. This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSM events.
 */
#if !(EVENTS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM events is exceeded.
#endif


/**//**
 * \brief   Enumeration of guards of ZDO Dispatcher FSM and all ZDP Services FSMs.
 */
enum
{
    /* End-Device Bind guards. */
    ZDO_G_IS_NO_MATCH,                              /*!< Is two received End_Device_Bind requests has no single match
                                                        between their output and input clusters. */

    ZDO_G_IS_NO_ENTRY,                              /*!< Is test Unbind_rsp returned with NO_ENTRY status. In this case
                                                        have to perform the binding procedure. Otherwise have to perform
                                                        the unbinding procedure. */

    ZDO_G_IS_FAILURE,                               /*!< Is Bind_rsp or Unbind_rsp returned with one of failure
                                                        statuses. For the case of binding procedure, it will be
                                                        terminated on the first failure status received. For the case of
                                                        unbinding procedureá status returned in the mentioned responses
                                                        is ignored. */

    ZDO_G_IS_CONTINUE,                              /*!< Is there are not processed but matched output clusters in the
                                                        compound list. In this case shall proceed with the current
                                                        binding or unbinding procedure. */

    /* Security Manager guards. */
    ZDO_G_IS_SECURITY_DISABLED,
    ZDO_G_IS_AUTHENTICATION_REQUIRED,
    ZDO_G_PRECONFIGURED_NETWORK_KEY,
    ZDO_G_PRECONFIGURED_TRUST_CENTER_LINK_KEY,
    ZDO_G_PRECONFIGURED_TRUST_CENTER_MASTER_KEY,
    ZDO_G_NOT_PRECONFIGURED_KEY,
    ZDO_G_IS_DUMMY_NWK_KEY,
    ZDO_G_IS_VALID_STD_NWK_KEY,
    ZDO_G_IS_VALID_APP_LINK_KEY,
    ZDO_G_IS_VALID_TC_LINK_KEY,
    ZDO_G_IS_TRUSTED_SOURCE,
    ZDO_G_IS_LEAVE_ITSELF,
    ZDO_G_IS_TRUST_CENTER,
    ZDO_G_IS_TC_ADDRESS_INVALID,
    ZDO_G_IS_SECURED_REJOIN,
    ZDO_G_IS_TRANSPORT_KEY_CMD,
    ZDO_G_IS_DEVICE_LEFT,

    /* Network Manager guards. */
    ZDO_G_IS_COMMISSIONING,
    ZDO_G_IS_COORDINATOR,
    ZDO_G_IS_ROUTER,
    ZDO_G_IS_END_DEVICE,
    ZDO_G_IS_START_SERVICE_DISABLED,
    ZDO_G_IS_ANNCE_SERVICE_DISABLED,
    ZDO_G_IS_LEAVE_SERVICE_DISABLED,
    ZDO_G_IS_SELF_IEEE_INVALID,
    ZDO_G_COMPOUSE_DESCRIPTORS,
    ZDO_G_IS_INVALID_STATUS,
    ZDO_G_NEED_TO_ENABLE_ROUTING,
    ZDO_G_IS_NVM_ENABLED,
    ZDO_G_IS_NEEDED_ANNCE,
    ZDO_G_IS_ITSELF_LEAVE,
    ZDO_G_IS_CHILD_LEAVE,
    ZDO_G_IS_FINISH_PART_PENDING,

    /* Channel Manager */
    ZDO_G_IS_ONE_CHANNEL_ABLE,
    ZDO_G_HAS_CANDIDATE,
    ZDO_G_IS_ENOUGH_TIMES,
    ZDO_G_IS_ENOUGH_HISTORY,
    ZDO_G_CHECK_TIMER,
    ZDO_G_START_TIMER,
    ZDO_G_IS_SAME_CHANNEL,

    GUARDS_NUMBER,      /*!< The total number of all guards. This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSM guards.
 */
#if !(GUARDS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM guards is exceeded.
#endif


/**//**
 * \brief All Project FSMs actions enumeration.
 */
enum
{
    /* Common-use actions. */
    ZDO_A_SEND_UPDATE_DEVICE,

    /* End-Device Bind actions. */
    ZDO_A_SAVE_FIRST_END_DEVICE_BIND_REQ,           /*!< Save the first End_Device_Bind request received. */

    ZDO_A_SAVE_SECOND_END_DEVICE_BIND_REQ,          /*!< Save the second End_Device_Bind request received. Note: this
                                                        identifier must follow directly the first one to have the
                                                        numeric identifier for one unit greater than the first. */

    ZDO_A_START_WAITING_TIMEOUT,                    /*!< Start timeout timer for period of waiting for the second remote
                                                        End_Device_Bind request. */

    ZDO_A_RESPOND_FAILURE_TIMEOUT,                  /*!< Issue response to the End Device Bind Service Handler with
                                                        failure TIMEOUT status for the first End_Device_Bind request ZDP
                                                        Server Transaction. */

    ZDO_A_RECALL_TIMEOUT_TIMER,                     /*!< Recall the timeout timer of waiting for the second remote
                                                        End_Device_Bind request. */

    ZDO_A_MATCH_BIND_REQUESTS,                      /*!< Match two End_Device_Bind requests. */

    ZDO_A_RESPOND_FAILURE_NO_MATCH,                 /*!< Issue response to the End Device Bind Service Handler with the
                                                        NO_MATCH failure status for the first End_Device_Bind request
                                                        ZDP Server Transaction; and then assign the NO_MATCH failure
                                                        status to the currently processed (the second) End_Device_Bind
                                                        request ZDP Server Transaction and finish it with directive to
                                                        the End Device Bind Service Handler not to hold this transaction
                                                        but continue its processing and transmit response on it. */

    ZDO_A_FIND_FIRST_MATCHED_CLUSTER,               /*!< Searches for the first matched output cluster within two
                                                        received requests. This cluster will be used for the test Unbind
                                                        request to discover if to bind or unbind all other clusters. */

    ZDO_A_ISSUE_BIND_REQ,                           /*!< Request ZDO Bind Service to issue ZDP Bind_req command for one
                                                        of matched output clusters. */

    ZDO_A_ISSUE_UNBIND_REQ,                         /*!< Request ZDO Unbind Service to issue ZDP Unbind_req command for
                                                        one of matched output clusters. */

    ZDO_A_FIND_NEXT_MATCHED_CLUSTER,                /*!< Searches for the next matched output cluster within two
                                                        received requests. If there are no matched clusters in the rest
                                                        of the compound output cluster list, set the cursor to 2N, where
                                                        N is the maximum allowed amount of clusters in a single list. */

    ZDO_A_RESPOND_FAILURE,                          /*!< Issue two responses to the End Device Bind Service Handler for
                                                        both two End_Device_Bind requests with the failure status
                                                        returned by Bind or Unbind Service. */

    ZDO_A_RESPOND_SUCCESS,                          /*!< Issue two responses to the End Device Bind Service Handler for
                                                        both two End_Device_Bind requests with the SUCCESS status. */

    /* Security Manager actions. */
    ZDO_A_SEND_LAZY_SUCCESSFUL_STATUS,
    ZDO_A_SEND_LAZY_AUTH_FAIL_STATUS,
    ZDO_A_SET_DEFAULT_TC_LINK_KEY,
    ZDO_A_SEND_SUCCESSFUL_STATUS,
    ZDO_A_PREPARE_FOR_AUTHENTICATION,
    ZDO_A_START_TRUST_CENTER,
    ZDO_A_RESET_KEY_COUNTERS,
    ZDO_A_SEND_NOT_SUPPORTED_STATUS,
    ZDO_A_SEND_FAIL_STATUS,
    ZDO_A_START_TIMEOUT_TIMER,
    ZDO_A_HANDLE_DUMMY_KEY_IND,
    ZDO_A_HANDLE_FIRST_STD_NWK_KEY_IND,
    ZDO_A_HANDLE_STD_NWK_KEY_IND,
    ZDO_A_HANDLE_APP_KEY_IND,
    ZDO_A_HANDLE_TC_KEY_IND,
    ZDO_A_SEND_TIMEOUT_STATUS,
    ZDO_A_HANDLE_SWITCH_KEY_IND,
    ZDO_A_DELAY_BEFORE_KEY_SWITCH,
    ZDO_A_SWITCH_NETWORK_KEY,
    ZDO_A_INITIATE_LEAVE_ITSELF,
    ZDO_A_INITIATE_LEAVE_CHILD,
    ZDO_A_CANCEL_CHILD_AUTHENTICATION,
    ZDO_A_CHILD_WITHOUT_SEC,
    ZDO_A_CHILD_AUTHENTICATED,
    ZDO_A_GENERATE_CHILD_REMOVED_EVENT,

    /* Network Manager actions. */
    ZDO_A_START_PROCESSING,
    ZDO_A_POSTPONE_REQ,
    ZDO_A_RAISE_ERR_STATUS,
    ZDO_A_RAISE_CALLBACK,
    ZDO_A_SEND_NWK_FORMATION,
    ZDO_A_SEND_NWK_JOIN,
    ZDO_A_PERFORM_AUTHENTICATION,
    ZDO_A_SET_PERMIT_JOINING_TIME,
    ZDO_A_PERFORM_START_ROUTER,
    ZDO_A_PERFORM_START_APS,
    ZDO_A_STORE_NWK_ATTRIBUTES,
    ZDO_A_SEND_DEVICE_ANNCE,
    ZDO_A_RAISE_INVALID_REQ,
    ZDO_A_RAISE_LOG_ID,
    ZDO_A_RAISE_ANNCE_LOG_ID,
    ZDO_A_POSTPONE_LEAVE,
    ZDO_A_POSTPONE_FINISH_PART,
    ZDO_A_REPORT_LEAVE,
    ZDO_A_GET_NEXT_DEVICE,
    ZDO_A_PERFORM_LEAVE,
    ZDO_A_TAKE_PARAMS_AND_APS_STOP,
    ZDO_A_STORE_OR_STOP,
    ZDO_A_PERFORM_APS_STOP,
    ZDO_A_FINISH_ITSELF_LEAVE,
    ZDO_A_RAISE_CHILD_LEFT_NTFY,
    ZDO_A_AUTHENTIFICATION_FAILED,
    ZDO_A_LEAVING_CALLBACK,

    /* Channel Manager */
    ZDO_A_NEXT_TRY,
    ZDO_A_CONTINUE,
    ZDO_A_ASK_ENERGY,
    ZDO_A_CHANCHE_CHANNEL,
    ZDO_A_CHANNEL_QUALITY_NOTIFY,

    ACTIONS_NUMBER,     /*!< The total number of all actions. This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSMs actions.
 */
#if !(ACTIONS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM actions is exceeded.
#endif


#endif /* _BB_ZBPRO_ZDO_CFG_FSM_H */

/* eof bbZbProZdoCfgFsm.h */