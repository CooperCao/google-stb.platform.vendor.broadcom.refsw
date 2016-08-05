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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../asp_manager_api.h"

static int
createListener(
    const char *pListenerPort
    )
{
    int sockFd;
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(atoi(pListenerPort));
    localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(sockFd > 0);

    assert(bind(sockFd, (struct sockaddr *)&localAddr, localAddrLen) == 0);

#if 0
    {
        int nonblock = 1;
        /* Commenting out as this simple example currently can be blocking. */
        /* Make the socket non-blocking. */
        assert (ioctl(sockFd, FIONBIO, &nonblock) == 0);
    }
#endif

    /* Start listening. */
    assert (listen(sockFd, 32) == 0);

    fprintf(stdout, "%s: listener to port=%s is ready, sockFd=%d\n", __FUNCTION__, pListenerPort, sockFd);
    return (sockFd);
}

static int
acceptConnection(
    int listenerSockFd
    )
{
    int nonblock = 1;
    int sockFd;
    struct sockaddr_in remoteAddr;
    socklen_t remoteAddrLen = sizeof(remoteAddr);

    sockFd = accept(listenerSockFd, (struct sockaddr *)&remoteAddr, &remoteAddrLen);
    if (sockFd < 0)
        return 0;

    /* Make the socket non-blocking. */
    assert (ioctl(sockFd, FIONBIO, &nonblock) == 0);

    fprintf(stdout, "%s: accepted new connx: listenerSockFd=%d, new sockFd=%d\n", __FUNCTION__, listenerSockFd, sockFd);
    return (sockFd);
}

int main(int argc, char *argv[])
{
    int rc;
    int listenerFd;
    int sockFd;
    char *pAspProxyServerIp;
    ASP_ChannelMgrHandle hAspChannelMgr;

    if (argc > 1) pAspProxyServerIp = argv[1];
    else pAspProxyServerIp = "127.0.0.1";

    rc = ASP_Mgr_Init(pAspProxyServerIp);
    assert(rc ==0);

    listenerFd = createListener("5000");
    assert(listenerFd > 0);

    /* Wait for client connection. */
    sockFd = acceptConnection(listenerFd);
    assert(sockFd);

    /* Open a ASP Channel Mananger. */
    hAspChannelMgr = ASP_ChannelMgr_Open();
    assert(hAspChannelMgr);

    /* Offload Connection to a ASP Channel Manager. */
    fprintf(stdout, "%s: Offload Connection: Host --> ASP...\n", __FUNCTION__);
    rc = ASP_ChannelMgr_Start(hAspChannelMgr, sockFd);
    assert(rc == 0);

#define PAYLOAD_SIZE 16
    /* Send Test Payload. */
    {
        int i;
        uint8_t payload[PAYLOAD_SIZE];

        for (i=0; i < PAYLOAD_SIZE; i++) payload[i] = i;
        fprintf(stdout, "%s: Sending Payload of length=%d\n", __FUNCTION__, PAYLOAD_SIZE);
        rc = ASP_ChannelMgr_SendPayload(hAspChannelMgr, payload, PAYLOAD_SIZE);
        assert(rc == 0);
    }

    /* Recv Test Payload. */
    {
        int i=0;
        uint8_t payload[PAYLOAD_SIZE];
        ssize_t payloadLength;

        fprintf(stdout, "%s: Receiving Payload of size=%d\n", __FUNCTION__, PAYLOAD_SIZE);
        payloadLength = ASP_ChannelMgr_RecvPayload(hAspChannelMgr, payload, PAYLOAD_SIZE);
        assert(payloadLength >= 0);
        i=0;
        while (i<(int)payloadLength)
        {
            fprintf(stdout, "%02u ", payload[i++]);
            if (i%8 == 0) fprintf(stdout, "\n");
        }
    }

    fprintf(stdout, "%s: Offload Connection: ASP --> Host...\n", __FUNCTION__);
    rc = ASP_ChannelMgr_Stop(hAspChannelMgr, &sockFd);
    assert(rc == 0);

    close(sockFd);

    return 0;
}
