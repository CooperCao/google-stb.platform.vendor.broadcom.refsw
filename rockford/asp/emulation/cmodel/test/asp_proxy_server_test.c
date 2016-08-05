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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "../asp_proxy_server_api.h"


typedef struct ASP_Mgr
{
    int fdPayload;
    int fdRawFrame;
    int fdEvents;
} ASP_Mgr;
typedef ASP_Mgr* ASP_MgrHandle;


static int
connectToProxyServer(
    const char *pServerPort,
    const char *pServerIp
    )
{
    int sockFd;
    struct addrinfo hints;
    struct addrinfo *addrInfo = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    assert(getaddrinfo(pServerIp, pServerPort, &hints, &addrInfo) == 0);

    sockFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    assert(sockFd > 0);

    assert(connect(sockFd, addrInfo->ai_addr, addrInfo->ai_addrlen) == 0);

    fprintf(stdout, "%s: connected to server:port=%s:%s, sockFd=%d\n", __FUNCTION__, pServerIp, pServerPort, sockFd);
    return (sockFd);
}

static void
ASP_Mgr_Destory(
    ASP_MgrHandle hAspMgr
    )
{
    /* Cleanup */

    close(hAspMgr->fdPayload);
    close(hAspMgr->fdEvents);
    close(hAspMgr->fdRawFrame);
    free(hAspMgr);
}

static ASP_MgrHandle
ASP_Mgr_Create(
    const char *pProxyServerIp
    )
{
    ASP_MgrHandle hAspMgr;

    /* Malloc ASP_Mgr */
    hAspMgr = calloc(1, sizeof(ASP_Mgr));
    assert(hAspMgr);

    hAspMgr->fdPayload = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_PAYLOAD, pProxyServerIp);
    assert(hAspMgr->fdPayload >= 0);

    hAspMgr->fdRawFrame = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_RAW_FRAMES, pProxyServerIp);
    assert(hAspMgr->fdRawFrame >= 0);

    hAspMgr->fdEvents = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_EVENTS, pProxyServerIp);
    assert(hAspMgr->fdEvents >= 0);

    return (hAspMgr);
}

static int sendOffloadConnxToAspMsg(
    int sockFd
    )
{

    /* Send the Offload Request message. */
    ASP_ProxyServerMsgOffloadConnxToAsp offloadConnxToAspMsg;

    /* TODO: Obtain TCP state associated with the sockFd and send a message to the Proxy Server. */
    /* For now, it is filled w/ test values. */
    offloadConnxToAspMsg.socketState.tcpState.seq = 1000;
    offloadConnxToAspMsg.socketState.tcpState.ack = 2000;

    ASP_ProxyServerMsg_Send(sockFd, ASP_ProxyServerMsg_eOffloadConnxToAsp, sizeof(offloadConnxToAspMsg), &offloadConnxToAspMsg);
    return (0);
}

static int recvOffloadConnxToAspMsgResp(
    int sockFd
    )
{
    /* Recv the Resp message. */
    ASP_ProxyServerMsgOffloadConnxToAspResp offloadConnxToAspMsgResp;
    ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;

    assert( ASP_ProxyServerMsg_RecvHeader(sockFd, &aspProxyServerMsgHeader) == true );
    assert( aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eOffloadConnxToAspResp );

    assert( ASP_ProxyServerMsg_RecvPayload(sockFd, sizeof(offloadConnxToAspMsgResp), &offloadConnxToAspMsgResp) == true );
    assert( offloadConnxToAspMsgResp.offloadToAspSuccessful == true );

    fprintf(stdout, "%s: Offload to ASP is successful!\n", __FUNCTION__);
    return (0);
}

static int sendOffloadConnxFromAspMsg(
    int sockFd
    )
{
    /* Send the Offload From ASP Request message. */
    ASP_ProxyServerMsgOffloadConnxFromAsp offloadConnxFromAspMsg;

    ASP_ProxyServerMsg_Send(sockFd, ASP_ProxyServerMsg_eOffloadConnxFromAsp, sizeof(offloadConnxFromAspMsg), &offloadConnxFromAspMsg);
    return (0);
}

static int recvOffloadConnxFromAspMsgResp(
    int sockFd
    )
{
    /* Recv the Resp message. */
    ASP_ProxyServerMsgOffloadConnxFromAspResp offloadConnxFromAspMsgResp;
    ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;

    assert( ASP_ProxyServerMsg_RecvHeader(sockFd, &aspProxyServerMsgHeader) == true );
    assert( aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eOffloadConnxFromAspResp );

    assert( ASP_ProxyServerMsg_RecvPayload(sockFd, sizeof(offloadConnxFromAspMsgResp), &offloadConnxFromAspMsgResp) == true );
    assert( offloadConnxFromAspMsgResp.socketState.tcpState.seq == 1000 );
    assert( offloadConnxFromAspMsgResp.socketState.tcpState.ack == 2000 );

    fprintf(stdout, "%s: Offload from ASP is successful!\n", __FUNCTION__);
    return (0);
}

int main()
{
    ASP_ProxyServerHandle hProxyServer;
    ASP_ProxyServerCreateSettings settings;
    int rc;
    int sockFd;
    char *pProxyServerIp = "127.0.0.1";
    ASP_MgrHandle hAspMgr;

    /* Change the default port & see if that works. */
    {
        ASP_ProxyServer_GetDefaultCreateSettings(&settings);
#define TEST_PORT "20000"
        settings.pPayloadListenerPort = TEST_PORT;
        hProxyServer = ASP_ProxyServer_Create(&settings);
        assert(hProxyServer);

        sockFd = connectToProxyServer(TEST_PORT, pProxyServerIp);
        assert(sockFd >= 0);
        close(sockFd);
        ASP_ProxyServer_Destory(hProxyServer);
    }

    /* Create Server w/ default settings & ensure all connections can be made. */
    {
        ASP_ProxyServer_GetDefaultCreateSettings(&settings);
        hProxyServer = ASP_ProxyServer_Create(&settings);
        assert(hProxyServer);

        hAspMgr = ASP_Mgr_Create(pProxyServerIp);
        assert(hAspMgr);

        /* rcv connection requests. */
        rc = ASP_ProxyServer_ProcessIo(hProxyServer);
        assert(rc == SUCCESS) ;

        /* simulate offload. */
        rc = sendOffloadConnxToAspMsg(hAspMgr->fdPayload);
        assert(rc == 0);

        /* rcv offload request. */
        rc = ASP_ProxyServer_ProcessIo(hProxyServer);
        assert(rc == SUCCESS) ;

        /* now recv response back from the proxy server. */
        recvOffloadConnxToAspMsgResp(hAspMgr->fdPayload);
        assert(rc == 0);

        /* tell ASP to offload the connx back & recv offload state. */
        rc = sendOffloadConnxFromAspMsg(hAspMgr->fdPayload);
        assert(rc == 0);

        /* run ProxyServer to receive the offload request & send us the response back. */
        rc = ASP_ProxyServer_ProcessIo(hProxyServer);
        assert(rc == SUCCESS) ;

        /* now recv response back from the proxy server. */
        rc = recvOffloadConnxFromAspMsgResp(hAspMgr->fdPayload);
        assert(rc == 0);

        ASP_Mgr_Destory(hAspMgr);
        ASP_ProxyServer_Destory(hProxyServer);
    }

    return 0;
}
