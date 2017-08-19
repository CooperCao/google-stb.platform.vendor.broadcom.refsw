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
******************************************************************************/

#ifndef _ZIGBEE_RPC_SERVER_PRIV_H_
#define _ZIGBEE_RPC_SERVER_PRIV_H_

typedef enum socket_state {
    SOCKET_INACTIVE=0,
#ifdef SIM
    SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_PairInd,
    SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_CheckValidationInd,
    SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_ControlCommandInd,
#endif
#ifdef TEST
    SOCKET_NEED_TO_SEND_RPC_S2C_ServerLoopbackInd,
    SOCKET_NEED_TO_WAIT_FOR_RPC_S2C_ServerLoopbackInd_callback,
#endif
    SOCKET_IDLE
} socket_state;

typedef struct socket_cb_t {
    int state;
    int message_rx[MAX_MSG_SIZE_IN_WORDS];
    int message_id;
    int message_rx_payload_size_in_words;
} socket_cb_t;

typedef struct zigbee_api_cb_RF4CE_StartReq_t {
    unsigned int server_request;
    unsigned int client_request;
    RF4CE_StartReqDescr_t client_request_data;
} zigbee_api_cb_RF4CE_StartReq_t;

typedef struct zigbee_api_cb_Req_t {
    unsigned int server_request;
    unsigned int client_request;
    char client_request_data[0];
} zigbee_api_cb_Req_t;


typedef struct zigbee_api_cb_HA_EnterNetworkReq_t {
    int socket;
    unsigned int server_request;
    /* TODO */
#if 0
    HA_EnterNetworkReq_t *client_request;
    HA_EnterNetworkReq_t client_request_data;
#endif
} zigbee_api_cb_HA_EnterNetworkReq_t;

typedef struct zigbee_api_cb_RF4CE_ZRC1_ControlCommandInd_t {
    RF4CE_ZRC1_ControlCommandIndParams_t indication;
} zigbee_api_cb_RF4CE_ZRC1_ControlCommandInd_t;

typedef struct zigbee_api_cb_RF4CE_ZRC2_ControlCommandInd_t {
    RF4CE_ZRC2_ControlCommandIndParams_t indication;
} zigbee_api_cb_RF4CE_ZRC2_ControlCommandInd_t;

typedef struct zigbee_api_cb_ClientLoopbackReq_t {
    int socket;
    unsigned int tx_buffer;
    unsigned int num_of_words;
    unsigned int callback;
    unsigned int rx_buffer;
} zigbee_api_cb_ClientLoopbackReq_t;

typedef struct zigbee_api_cb_ClientCoreLoopbackReq_t {
    int socket;
    unsigned int num_of_words;
    unsigned int callback;
    unsigned int rx_buffer;
} zigbee_api_cb_ClientCoreLoopbackReq_t;

typedef struct zigbee_api_cb_t {
    zigbee_api_cb_HA_EnterNetworkReq_t HA_EnterNetworkReq;
    zigbee_api_cb_ClientLoopbackReq_t ClientLoopbackReq;
    zigbee_api_cb_ClientCoreLoopbackReq_t ClientCoreLoopbackReq;
} zigbee_api_cb_t;

typedef struct zigbee_api_rf4ce_cb_t {
    zigbee_api_cb_RF4CE_StartReq_t RF4CE_StartReq;
    zigbee_api_cb_RF4CE_ZRC1_ControlCommandInd_t RF4CE_ZRC1_ControlCommandInd;
    zigbee_api_cb_RF4CE_ZRC2_ControlCommandInd_t RF4CE_ZRC2_ControlCommandInd;
} zigbee_api_rf4ce_cb_t;

extern socket_cb_t socket_cb[];

#endif /*_ZIGBEE_RPC_SERVER_PRIV_H_*/

/* eof zigbee_rpc_server_priv.h */