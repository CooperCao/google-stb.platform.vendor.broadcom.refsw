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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkCfgFsm.h $
*
* DESCRIPTION:
*   NWK layer Finite State Machines integral description.
*
* $Revision: 3955 $
* $Date: 2014-10-08 12:45:05Z $
*
*****************************************************************************************/


#ifndef _BB_ZB_PRO_NWK_CFG_FSM_H
#define _BB_ZB_PRO_NWK_CFG_FSM_H


/************************* INCLUDES *****************************************************/
#include "bbSysFsm.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief FSMs states enumeration.
 */
enum
{
    /* Common states. */
    S_IDLE,
    S_INVALID,
    S_WAITING,
    S_SEND_A_CONFIRM,
    S_HANDLING_STARTED,

    /* TX FSM States */
    S_ROUTE_CHECK,
    S_NWK_HEADER_COMPOSING,
    S_TX_PREPARATION,
    S_ROUTE_RECORD_TRANSMISSION,
    S_ENCRYPTING,
    S_TX_START,
    S_FRAME_SENT,
    S_TX_FINISHED,
    S_FAILURE_REPORTING,
    S_WAITING_FAILURE_CONFIRM,
    S_DECRYPTING,
    S_CONFIRMING,
    S_INITIALIZING,
    S_ROUTE_DISCOVERING,
    S_TX_INDIRECT_BROADCAST,

    /* Reset handler states */
    S_MAC_RESETTING,

    /* Join service states */
    S_VERIFY,
    S_CHANNEL_CHANGE,
    S_DISCOVERY,
    S_DISCOVERY_DELAY,
    S_ASSOCIATE_REQ,
    S_PREPARE_REJOIN,
    S_REJOIN_REQ,
    S_REJOIN_RESP,
    S_COMMISSIONING,
    S_CONFIGURE,
    S_JOIN_REQ,
    S_FINISH,

    /* Leave service states */
    S_REMOVING_NEIGHBOR,
    S_LEAVING_ITSELF,
    S_UPDATING_TABLE,
    S_MAC_RESET,
    S_LEAVE_REQUEST_HANDLING,
    S_LEAVE_NOTIFICATION_HANDLING,

    /* Permit Joining */
    S_PERMITION_IN_PROGRESS,
    S_PROHIBITION_IN_PROGRESS,
    S_WAITING_FOR_CONFIRM,

    /* Discovery service */
    S_SCANING,

    /* Network formation service states */
    S_ED_SCAN,
    S_ACTIVE_SCAN,
    S_STARTING,

    /* PanId conflict module */
    S_CONFLICT_REPORTING,
    S_CONFLICT_MANAGING,
    S_CONFLICT_RESOLVING,

    STATES_NUMBER,      /*!< The total number of all states.
                            This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs states.
 */
/* TODO: Validate test condition. */
#if !(STATES_NUMBER <= SYS_FSM_STATE_FLAG)
# error The allowed number of FSM states is exceeded.
#endif


/**//**
 * \brief FSMs events enumeration.
 */
enum
{
    E_NEW_REQUEST,
    E_EXECUTE,
    E_CONFIRM,
    E_DONE,
    E_TIMEOUT,
    E_SCROLL,

    /* TX FSM events */
    E_START_TX,
    E_ROUTING_DONE,
    E_STATUS_OK,
    E_STATUS_ERR,

    /* Join service events */
    E_DISCOVERY,
    E_ASSOCIATE_OK,
    E_ASSOCIATE_ERR,

    E_REJOIN_REQ_OK,
    E_REJOIN_REQ_ERR,
    E_REJOIN_RESP_OK,
    E_REJOIN_RESP_ERR,
    E_NWK_SET_OK,
    E_NWK_SET_ERR,

    E_RESPONSE,
    E_NO_POSTPONED,

    /* Leave service events */
    E_NEW_INDICATION,

    /* Discovery service events */
    E_BEACON,
    E_SCAN_DONE,

    /* PanId conflict events */
    E_REPORT_CMD,
    E_PANID_LIST_COMPILED,
    E_SEND_REPORT_CMD_DONE,
    E_UPDATE_CMD,
    E_SEND_UPDATE_CMD_DONE,
    E_SET_DONE,

    EVENTS_NUMBER,      /*!< The total number of all signals.
                            This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs events.
 */
/* TODO: Validate test condition. */
#if !(EVENTS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM events is exceeded.
#endif


/**//**
 * \brief FSMs guards enumeration.
 */
enum
{
    NWK_G_QUEUE_IS_EMPTY,
    NWK_G_QUEUE_IS_NOT_EMPTY,
    NWK_G_IS_SERVICE_DISABLED,

    /* TX FSM Guards */
    NWK_G_TOO_LONG_PACKET,
    NWK_G_IS_LOOPBACK,
    NWK_G_ROUTE_EXIST,
    NWK_G_ROUTING_IS_ENABLED,
    NWK_G_ROUTE_RECORD_IS_NEEDED,
    NWK_G_DELAY_IS_NEEDED,
    NWK_G_ENCRYPTION_IS_NEEDED,
    NWK_G_INDIRECT_BROADCAST,
    NWK_G_DECRYPTION_IS_NEEDED,
    NWK_G_TX_FINISHED,
    NWK_G_ROUTE_FAILURE,
    NWK_G_IS_SUCCESSFUL_REPORT,
    NWK_G_NEXT_NEIGHBOR_FOUND,
    NWK_G_IS_MANY_TO_ONE_ROUTE,
    NWK_G_IS_SOURCE_ROUTE,
    NWK_G_IS_ANY_KEY_ACTIVATED,
    NWK_G_ROUTING_IS_SUCCESSFUL,
    NWK_G_IS_BROADCAST_LB_NEEDED,

    /* Join service guards */
    NWK_G_INIT_AND_VALIDATE,
    NWK_G_ASSOCIATION_REQ,
    NWK_G_ORPHANING_REQ,
    NWK_G_REJOIN_REQ,
    NWK_G_CHANNEL_CHANGE_REQ,
    NWK_G_COMMISSIONING_REQ,

    NWK_G_IS_REQUESTED_EXT_PANID,
    NWK_G_NO_SUITABLE_PARENT,
    NWK_G_IS_DISCOVERY_NEEDED,
    NWK_G_ISNT_ENOUGH_TIMES,
    NWK_G_CHANNEL_IS_DIFFERENT,
    NWK_G_IS_ACTUAL_MAC_CHANNEL,
    NWK_G_PANID_IS_DIFFERENT,
    NWK_G_IS_ACTUAL_MAC_PANID,
    NWK_G_IS_INVALID_SHORT_ADDR,

    NWK_G_EXT_PANID_IS_DIFFERENT,
    NWK_G_UPDATEID_IS_DIFFERENT,

    NWK_G_INVALID_SUITABLE_PARENT,
    NWK_G_VALID_PARENT,
    NWK_STOP_RESP_TIMER,

    /* Leave service Guards */
    NWK_G_IS_BUSY,
    NWK_G_IS_LEAVE_DISABLED,
    NWK_G_IS_CORRECT_DEVICE_ADDR,
    NWK_G_IS_NOT_NEIGHBOR,
    NWK_G_IS_AUTHENTICATED_CHILD,
    NWK_G_IS_UNAUTHENTICATED_CHILD,
    NWK_G_IS_REQUEST_SUBFIELD_SET,
    NWK_G_IS_FROM_PARENT,
    NWK_G_IS_REMOVE_CHILDREN_SUBFIELD_SET,

    /* Start Router service guards. */
    NWK_G_IS_START_ROUTER_ENABLED,
    NWK_G_IS_SUCCESSFUL_STATUS,

    /* Discovery service guards */
    NWK_G_IS_VALID_REQUEST,
    NWK_G_IS_NETWORK_DISCOVERY,
    NWK_G_IS_ED_SCAN,
    NWK_G_APPLY_EXT_FILTER,

    /* Permit Joining */
    NWK_G_IS_PERMIT_JOINING_ENABLED,
    NWK_G_IS_PERMANENT,
    NWK_G_IS_SET_BUSY,

    /* Network Formation service guards. */
    NWK_G_SINGLE_CHANNEL,
    NWK_G_MAC_REQ_SUCCESSFUL,
    NWK_G_PARAMETERS_ARE_CORRECT,

    /* PanId conflict module */
    NWK_G_ISNT_ACTIVE_NETWORK,
    NWK_G_IS_NON_CONFLICTED_BEACON,
    NWK_G_IS_LIST_MEM_ALLOCATED,
    NWK_G_IS_VALID_UPDATE_ID,
    NWK_G_IS_UPDATE_PENDING,

    GUARDS_NUMBER,                  /*!< The total number of all guards.
                                        This item must be the last one in the list. */
};


/*
 * Validate the total number of the FSMs guards.
 */
/* TODO: Validate test condition. */
#if !(GUARDS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM guards is exceeded.
#endif


/*************************************************************************************//**
  \brief
    All Project FSMs actions enumeration.
*****************************************************************************************/
enum
{
    NWK_A_POST_TASK,
    NWK_A_START_PROCESSING,
    NWK_A_POSTPONE_REQ,
    NWK_A_POSTPONE_IND,
    NWK_A_SPLIT_UNSECURED_FRAME,
    NWK_A_SEND_CONFIRM,
    NWK_A_PERFORM_DELAY,
    NWK_A_ABORT_WAITING,
    NWK_A_START_TIMER,
    NWK_A_POSTPONE_REQ_AND_POST_TASK,

    /* TX FSM Actions */
    NWK_A_PREPARE_HEADER,
    NWK_A_ROUTE_DISCOVERY,
    NWK_A_SEND_ROUTE_RECORD,
    NWK_A_ENCRYPT,
    NWK_A_SEND_INDIRECT_BROADCAST,
    NWK_A_SEND_MAC_DATA_REQ,
    NWK_A_SEND_MANY_TO_ONE_FAILURE,
    NWK_A_SEND_SOURCE_ROUTE_FAILURE,
    NWK_A_DECRYPT,
    NWK_A_FIND_ROUTE,
    NWK_A_DELETE_ROUTE,
    NWK_A_INIT_BUFFER,
    NWK_A_PERFORM_LOOPBACK,
    NWK_A_RAISE_RECEIVED_STATUS,
    NWK_A_COMPOSE_UNSECURED_FRAME,
    NWK_A_FINISH_INDIRECT_BROADCAST,

    /* Reset handler actions */
    NWK_A_RESET_MAC,
    NWK_A_RESET_NETWORK_LAYER,

    /* Join service actions */
    NWK_A_PERFORM_CHANNEL_CHANGE,
    NWK_A_ORPHANING_PROCEDURE,
    NWK_A_PERFORM_DISCOVERY,
    NWK_A_START_DISCOVERY_DELAY_TIMER,
    NWK_A_SEND_ASSOCIATE_REQ,
    NWK_A_SEND_REJOIN_REQ,
    NWK_A_START_REJOIN_TIMER,

    NWK_A_PICK_UP_RANDOM_ADDR,
    NWK_A_SET_NEW_EXT_PANID,
    NWK_A_SET_NEW_ADDR,
    NWK_A_MAC_UPDATE_ADDR,
    NWK_A_SET_PARENT_PANID,
    NWK_A_MAC_UPDATE_PANID,
    NWK_A_SET_PARENT_CHANNEL,
    NWK_A_MAC_UPDATE_CHANNEL,
    NWK_A_SET_OTHER_ATTR,
    NWK_A_CHANGE_GLOBAL_STATE,

    NWK_A_RAISE_LOG_ID,
    NWK_A_RAISE_INVALID_REQ,
    NWK_A_RAISE_NOT_PERMITED,
    NWK_A_RAISE_UNSUP_ATTR,
    NWK_A_RAISE_ERR_STATUS,
    NWK_A_RAISE_SUCCESS,

    /* Leave service actions */
    NWK_A_PREPARE_FOR_REMOVING_NEIGHBOR,
    NWK_A_LEAVING_ITSELF,
    NWK_A_SEND_LEAVE_COMMAND,
    NWK_A_UPDATE_NEIGHBOR_TABLE_AND_RAISE_CONFIRM,
    NWK_A_CLEAN_EXT_PANID_AND_RAISE_CONFIRM,
    NWK_A_RAISE_INDICATION,
    NWK_A_RAISE_INDICATION_AND_UPDATE_TABLES,
    NWK_A_RAISE_LEAVE_ITSELF_INDICATION,
    NWK_A_PARSE_FRAME,
    NWK_A_PARSE_FRAME_FROM_QUEUE,

    /* Start Router actions */
    NWK_A_START_MAC,
    NWK_A_FINISH_AND_CONFIRM,
    NWK_A_ASSERT,

    /* Discovery service actions */
    NWK_A_START_SCANING,
    NWK_A_CATCH_BEACON,
    NWK_A_RAISE_DISCOVERY_CONF,
    NWK_A_RAISE_ED_SCAN_CONF,

    /* Permit Joining */
    NWK_A_PERMIT_ASSOCIATION,
    NWK_A_FORBID_ASSOCIATION,
    NWK_A_START_TIMER_AND_CONFIRM,
    NWK_A_STOP_TIMER_AND_CONFIRM,

    /* Network Formation actions */
    NWK_A_SELECT_CHANNEL,
    NWK_A_SET_CHANNEL,
    NWK_A_PERFORM_ED_SCAN,
    NWK_A_PERFORM_ACTIVE_SCAN,
    NWK_A_SELECT_PANID,
    NWK_A_START_ROUTING,
    NWK_A_SEND_CONFIRM_WITH_MAC_STATUS,
    NWK_A_FINALIZE_FORMATION,

    NWK_A_RAISE_INVALID_REQUEST_STATUS,
    NWK_A_RAISE_UNKNOWN_DEVICE_STATUS,
    NWK_A_RAISE_ROUTE_ERROR_STATUS,
    NWK_A_RAISE_STURTUP_FAILURE_STATUS,
    NWK_A_RAISE_FRAME_TOO_LONG_STATUS,
    NWK_A_RAISE_NO_KEY_STATUS,

    /* PanId conflict module */
    NWK_A_PREPARE_PANID_LIST,
    NWK_A_SEND_REPORT_CMD,
    NWK_A_POSTPONE_UPDATE_CMD,
    NWK_A_START_POSTPONED_UPDATE,
    NWK_A_START_UPDATE_TIMER,
    NWK_A_SEND_UPDATE_CMD,
    NWK_A_CHANGE_PARAMS,
    NWK_A_RAISE_STATUS_IND,

    ACTIONS_NUMBER,                 /*!< The total number of all actions.
                                         This item must be the last one in the list. */
};

/*
 * Validate the total number of the FSMs actions.
 */
/* TODO: Validate test condition. */
#if !(ACTIONS_NUMBER <= SYS_FSM_ILLEGAL_VALUE)
# error The allowed number of FSM actions is exceeded.
#endif

#endif /* _BB_ZB_PRO_NWK_CFG_FSM_H */