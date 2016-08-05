/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef __ASP_PROXY_SERVER_MESSAGE_API_H__
#define __ASP_PROXY_SERVER_MESSAGE_API_H__

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <netinet/in.h>

#include "asp_utils.h"

/* Proxy Server Ports */
#define ASP_PROXY_SERVER_PORT_FOR_PAYLOAD       "9999"
#define ASP_PROXY_SERVER_PORT_FOR_RAW_FRAMES    "9998"
#define ASP_PROXY_SERVER_PORT_FOR_EVENTS        "9997"

/* Messages exchanged between DUT & ASP ProxyServer */
typedef enum
{
    ASP_ProxyServerMsg_eOffloadConnxToAsp,     /* Msg containing initial TCP/UDP state for Offloading connection from Host to ASP. ASP will setup a Channel for this connection. */
    ASP_ProxyServerMsg_eOffloadConnxToAspResp, /* Resp Msg containing initial TCP/UDP state for Offloading connection from Host to ASP. ASP will setup a Channel for this connection. */
    ASP_ProxyServerMsg_eOffloadConnxFromAsp,   /* Msg containing final TCP/UDP state for Offloading connection from ASP back to Host. */
    ASP_ProxyServerMsg_eOffloadConnxFromAspResp,   /* Msg containing final TCP/UDP state for Offloading connection from ASP back to Host. */
    ASP_ProxyServerMsg_eSendPayloadToAsp,      /* Msg containing Payload to send to Network Peer. ASP Segments it, prepares Ethernet Frames and then sends to the client via _eSendRawFramesToNw msg. */
    ASP_ProxyServerMsg_eSendPayloadToAspResp,  /* Msg containing Payload to send to Network Peer. ASP Segments it, prepares Ethernet Frames and then sends to the client via _eSendRawFramesToNw msg. */
    ASP_ProxyServerMsg_eSendRawFrameToAsp,     /* Msg containing Raw Frame (either TCP ACK/RST or TCP/UDP Payload) from the Network Peer. ASP will process the Ethernet/IP/TCP or UDP headers. */
    ASP_ProxyServerMsg_eSendRawFrameToAspResp, /* Msg containing Raw Frame (either TCP ACK/RST or TCP/UDP Payload) from the Network Peer. ASP will process the Ethernet/IP/TCP or UDP headers. */
    ASP_ProxyServerMsg_eRecvPayloadFromAsp,    /* Msg containing Payload received from Network Peer. ASP has completed the TCP/UDP processing on a previous Raw Frame & has a payload ready for App. */
    ASP_ProxyServerMsg_eRecvPayloadFromAspResp,/* Msg containing Payload received from Network Peer. ASP has completed the TCP/UDP processing on a previous Raw Frame & has a payload ready for App. */
    ASP_ProxyServerMsg_eSendRawFrameToNw,      /* Msg containing a Raw Frame (either TCP ACK for previous TCP Payload from Peer or TCP/UDP Payload). */
    ASP_ProxyServerMsg_eSendCloseEventToAsp,   /* Msg containing Close Event from App. This indicates to ASP to Stop the current Channel. */
    ASP_ProxyServerMsg_eSendCloseEventToApp,   /* Msg containing Close Event (FIN) from Network Peer. */
    ASP_ProxyServerMsg_eSendAbortEventToApp,   /* Msg containing Abort Event (RST) from Network Peer. */

    ASP_ProxyServerMsg_eMax
} ASP_ProxyServerMsg;

/* Message format: <message_type: 4 bytes> < message_length: 4 bytes> < message payload > */
typedef struct
{
    ASP_ProxyServerMsg  type;
    uint32_t            length;    /* length of the message payload following the ASP_ProxyServerMsgHeader (doesn't include the ASP_ProxyServerMsgHeader length). */
} ASP_ProxyServerMsgHeader;

/* Structure to hold socket & last read MsgHeader on it. */
typedef struct
{
    int                 sockFd;
    bool                msgHeaderValid;
    ASP_ProxyServerMsgHeader msgHeader;
} ASP_ProxyServerSocketMsgInfo;
typedef ASP_ProxyServerSocketMsgInfo* ASP_ProxyServerSocketMsgInfoHandle;

/* APIs to Receive the Message Request Header & Payload. */

/* Returns true when complete message header has been read into the pAspProxyServerSocketMsgHeader. Doesn't return partial message header. */
bool ASP_ProxyServerMsg_RecvHeader(
    int sockFd,
    ASP_ProxyServerMsgHeader *pAspProxyServerSocketMsgHeader
    );

/* Returns true when msgPayloadLength have been read into the pMsgPayload. Doesn't return partial message payload. */
bool ASP_ProxyServerMsg_RecvPayload(
    int sockFd,
    size_t msgPayloadLength,
    void *msgPayload
    );

/* Builds message header using type & length and sends it along w/ the msg payload. */
bool ASP_ProxyServerMsg_Send(
    int sockFd,
    ASP_ProxyServerMsg msgType,
    size_t msgPayloadLength,
    void *pMsgPayload
    );

/********** ASP_ProxyServerMsg_eOffloadConnxToAsp Message Type Format ***********/
typedef struct tcpState
{
    uint32_t            seq;
    uint32_t            ack;
    uint32_t            maxSegSize;
    struct tcp_info     tcpInfo;
} TcpState;

typedef struct SocketState
{
    TcpState            tcpState;
    struct sockaddr_in  remoteIpAddr;
    struct sockaddr_in  localIpAddr;
} SocketState;
typedef struct SocketState *hSocketState;

typedef struct ASP_ProxyServerMsgOffloadConnxToAsp
{
    SocketState         socketState;
    bool                streamOut;
} ASP_ProxyServerMsgOffloadConnxToAsp;


/********** ASP_ProxyServerMsg_eOffloadConnxToAspResp Message Type Format ***********/
typedef struct ASP_ProxyServerMsgOffloadConnxToAspResp
{
    bool                offloadToAspSuccessful;
} ASP_ProxyServerMsgOffloadConnxToAspResp;


/********** ASP_ProxyServerMsg_eOffloadConnxFromAsp Message Type Format ***********/
typedef struct ASP_ProxyServerMsgOffloadConnxFromAsp
{
    int                 unused;
} ASP_ProxyServerMsgOffloadConnxFromAsp;


/********** ASP_ProxyServerMsg_eOffloadConnxFromAspResp Message Type Format ***********/
typedef struct ASP_ProxyServerMsgOffloadConnxFromAspResp
{
    SocketState         socketState;
} ASP_ProxyServerMsgOffloadConnxFromAspResp;


/********** ASP_ProxyServerMsg_eSendPayloadToAspResp Message Type Format ***********/
typedef struct ASP_ProxyServerMsgSendPayloadToAspResp
{
    bool                payloadGivenToAspSuccess;
} ASP_ProxyServerMsgSendPayloadToAspResp;



/********** ASP_ProxyServerMsg_eRecvPayloadFromAsp Message Type Format ***********/
typedef struct ASP_ProxyServerMsgRecvPayloadFromAsp
{
    size_t              payloadSize;
} ASP_ProxyServerMsgRecvPayloadFromAsp;

#endif /* __ASP_PROXY_SERVER_MESSAGE_API_H__ */
