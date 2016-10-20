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
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "../../../../nexus/lib/bip/include/bip.h"

#include "asp_manager_api.h"
#include "asp_connx_migration_api.h"
#include "../cmodel/asp_proxy_server_message_api.h"

/* Global ASP Mgr Context. */
typedef struct ASP_Mgr
{
    char *pAspProxyServerIp;
} ASP_Mgr;

static ASP_Mgr g_aspMgr;
typedef struct ASP_Mgr* ASP_MgrHandle;

typedef struct ASP_ChannelMgr
{
    int fdPayload;
    int fdRawFrame;
    int fdEvents;
    int fdMigratedConnx;    /* sockFd of migrated socket. */

    BIP_IoCheckerHandle hRawFrameFdIoChecker;

    struct sockaddr_in remoteIpAddr;    /* TODO: This is no more required, depricated with actual raw frame send.*/
    char *pInterfaceName;               /* This is needed to send the raw frame to network.*/
    unsigned char      remoteHostMacAddr[8];/* This is needed to send the raw frame to network.*/
    ASP_MgrHandle hAspMgr;  /* Parent handle. */
} ASP_ChannelMgr;

/*
 * ASP_Mgr_Uninit()
 */
void
ASP_Mgr_Uninit(void)
{

    BIP_Uninit();

    NEXUS_Platform_Uninit();

    if (g_aspMgr.pAspProxyServerIp)
    {
        free(g_aspMgr.pAspProxyServerIp);
        g_aspMgr.pAspProxyServerIp = NULL;
    }
}

/*
 * ASP_Mgr_Init()
 */
int
ASP_Mgr_Init(
    const char *pAspProxyServerIp      /* If non-null, communicate w/ ASP using this proxy server. */
    )
{
    int rc;
    /* TODO:Later if we bring in BIP_Uninit() then this will be removed.*/
    static NEXUS_PlatformSettings   platformSettings;    /* Make "static" to reduce stack usage for Coverity. */

    memset(&g_aspMgr, 0, sizeof(g_aspMgr));

    if (pAspProxyServerIp)
    {
        g_aspMgr.pAspProxyServerIp = strndup(pAspProxyServerIp, strlen(pAspProxyServerIp));
        assert(g_aspMgr.pAspProxyServerIp);
    }

    /* Entry Point: */
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    /* TODO:Later if we bring in BIP_Init() then this will be removed.*/
    rc = BIP_Init(NULL);
    assert(rc == 0);

    return 0;
}

/*
 * ASP_ChannelMgr_Close()
 */
void
ASP_ChannelMgr_Close(
ASP_ChannelMgrHandle hAspChannelMgr
    )
{
    /** Now we can free memory for getRemoteMacAddress **/
    if(hAspChannelMgr->pInterfaceName)
    {
        free(hAspChannelMgr->pInterfaceName);
        hAspChannelMgr->pInterfaceName = NULL;
    }

    assert(hAspChannelMgr);
    free(hAspChannelMgr);
}

/*
 * ASP_ChannelMgr_Open()
 */
ASP_ChannelMgrHandle
ASP_ChannelMgr_Open(
    void
    )
{
    ASP_ChannelMgrHandle hAspChannelMgr;

    hAspChannelMgr = calloc(1, sizeof(ASP_ChannelMgr));
    assert(hAspChannelMgr);

    hAspChannelMgr->hAspMgr = &g_aspMgr;
    return (hAspChannelMgr);
}

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

    assert(connect(sockFd, (struct sockaddr *)addrInfo->ai_addr, addrInfo->ai_addrlen) == 0);

    fprintf(stdout, "%s: connected to server:port=%s:%s, sockFd=%d\n", __FUNCTION__, pServerIp, pServerPort, sockFd);
    return (sockFd);
}

static int sendRawFrameToNw(
    ASP_ChannelMgrHandle    hAspChannelMgr,
    char                    *pRawFrameBuffer,
    uint32_t                rawFrameSize
    )
{
#if 0
    int rc  = 0;
    int socketFd = -1;
    int one = 1;
    const int *val = &one;

    socketFd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if(socketFd < 0)
    {
        fprintf(stdout, "%s: Failed to create raw socket:\n", __FUNCTION__);
        goto error;
    }

    rc = setsockopt(socketFd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));
    if(socketFd < 0)
    {
        fprintf(stdout, "%s: setsocketopt failed to set IP_HDRINCL ...\n", __FUNCTION__);
        goto error;
    }

    fprintf(stdout,"\n %s: =======================> sending to %s:%d\n",__FUNCTION__, inet_ntoa(hAspChannelMgr->remoteIpAddr.sin_addr), ntohs(hAspChannelMgr->remoteIpAddr.sin_port));
    rc = sendto(socketFd, pRawFrameBuffer, rawFrameSize, 0, (struct sockaddr *)&hAspChannelMgr->remoteIpAddr, sizeof(hAspChannelMgr->remoteIpAddr));
    if(rc < 0)
    {
        fprintf(stdout, "%s:sendto failed ...\n", __FUNCTION__);
        goto error;
    }
#endif

    int socketFd;
    int rc  = 0;
    struct sockaddr_ll socket_address;
    struct ifreq if_idx;

    /* Open RAW socket to send on */
    if ((socketFd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        fprintf(stdout, "%s: Failed to create raw socket:\n", __FUNCTION__);
        goto error;
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, hAspChannelMgr->pInterfaceName, IFNAMSIZ-1);
    if (ioctl(socketFd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");


    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = hAspChannelMgr->remoteHostMacAddr[0];
    socket_address.sll_addr[1] = hAspChannelMgr->remoteHostMacAddr[1];
    socket_address.sll_addr[2] = hAspChannelMgr->remoteHostMacAddr[2];
    socket_address.sll_addr[3] = hAspChannelMgr->remoteHostMacAddr[3];
    socket_address.sll_addr[4] = hAspChannelMgr->remoteHostMacAddr[4];
    socket_address.sll_addr[5] = hAspChannelMgr->remoteHostMacAddr[5];

    /* Send packet */
    if (sendto(socketFd, pRawFrameBuffer, rawFrameSize, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
        printf("Send failed\n");

    fprintf(stdout, "%s: ASP_ProxyServerMsg_eSendRawFrameToNw   rawFrameSize==============%d\n", __FUNCTION__, rawFrameSize);

    close(socketFd);

    return rc;
error:
    if(socketFd != -1)
    {
        close(socketFd);
    }
    return rc;
}

static void sendrawFrameFromRecvIoCheckerCallback(
    void *pContext,
    int param,
    BIP_IoCheckerEvent eventMask
    )
{
    ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;
    char *pRawFrameBuffer = NULL;
    ASP_ChannelMgrHandle    hAspChannelMgr = (ASP_ChannelMgrHandle)pContext;
    int brc;

    fprintf(stdout, "%s: Recv message of type ASP_ProxyServerMsg_eSendRawFrameToNw\n", __FUNCTION__);
    brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdRawFrame, &aspProxyServerMsgHeader);
    assert(brc == true);
    assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eSendRawFrameToNw);


    /* Allocate memory for raw frame. */
    pRawFrameBuffer = calloc(1, (aspProxyServerMsgHeader.length*4));

    brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdRawFrame, aspProxyServerMsgHeader.length, pRawFrameBuffer);
    assert(brc == true);

    fprintf(stdout, "%s: message payload size = %d----------------------\n", __FUNCTION__, aspProxyServerMsgHeader.length);
    brc = sendRawFrameToNw( hAspChannelMgr, pRawFrameBuffer, aspProxyServerMsgHeader.length);
    assert(brc != -1);

    if(pRawFrameBuffer)
    {
        free(pRawFrameBuffer);
    }
}

/*
 * ASP_ChannelMgr_Start()
 */
int
ASP_ChannelMgr_Start(
    ASP_ChannelMgrHandle    hAspChannelMgr,
    int                     sockFdToOffload     /* in: fd of socket to be offloaded from host to ASP. */
    )
{
    int rc;
    bool brc;
    ASP_ProxyServerMsgOffloadConnxToAsp offloadConnxToAspMsg;
    SocketState socketState;

    /* Obtain TCP state associated with the sockFdToOffload and send a message to the Proxy Server. */
    {
        rc = ASP_ConnxMigration_GetTcpStateFromLinux(sockFdToOffload,
                 &socketState,
                 &hAspChannelMgr->pInterfaceName,  /* This is requuired to be preserved since it will be rewuired at the time of sendRawFrameToNw.*/
                 hAspChannelMgr->remoteHostMacAddr /* This is requuired to be preserved since it will be rewuired at the time of sendRawFrameToNw.*/
                 );
        assert(rc == 0);
        hAspChannelMgr->remoteIpAddr = socketState.remoteIpAddr;
    }

    /* Setup connection w/ the ASP Proxy Server. */
    {
        BIP_IoCheckerCreateSetting ioCreateSettings;

        printf("\n%s: connectint to connectToProxyServer for ASP_PROXY_SERVER_PORT_FOR_PAYLOAD\n",__FUNCTION__);
        hAspChannelMgr->fdPayload = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_PAYLOAD, hAspChannelMgr->hAspMgr->pAspProxyServerIp);
        assert(hAspChannelMgr->fdPayload >= 0);

        hAspChannelMgr->fdRawFrame = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_RAW_FRAMES, hAspChannelMgr->hAspMgr->pAspProxyServerIp);
        assert(hAspChannelMgr->fdRawFrame >= 0);

        /* Once rawFrame is connected, create a ioChecker to observe it for any incoming raw Frames from asp c-model.*/
        BIP_IoChecker_GetDefaultCreateSettings( &ioCreateSettings );
        ioCreateSettings.fd =  hAspChannelMgr->fdRawFrame;
        ioCreateSettings.settings.callBackFunction = sendrawFrameFromRecvIoCheckerCallback;
        ioCreateSettings.settings.callBackContext  =  hAspChannelMgr;
        /* TODO: Later we will need to add mac address in param*/
        hAspChannelMgr->hRawFrameFdIoChecker = BIP_IoChecker_Create( &ioCreateSettings );
        assert(hAspChannelMgr->hRawFrameFdIoChecker);
        fprintf(stdout, "%s: Created hRawFrameFdIoChecker for rfdRawFrame=%d\n", __FUNCTION__, hAspChannelMgr->fdRawFrame);
        BIP_IoChecker_Enable( hAspChannelMgr->hRawFrameFdIoChecker, BIP_IoCheckerEvent_ePollIn);

        hAspChannelMgr->fdEvents = connectToProxyServer(ASP_PROXY_SERVER_PORT_FOR_EVENTS, hAspChannelMgr->hAspMgr->pAspProxyServerIp);
        assert(hAspChannelMgr->fdEvents >= 0);
    }

    /* Prepare OffloadConnxToAsp message & send it to ASP Proxy Server. */
    {
        offloadConnxToAspMsg.socketState = socketState;
        offloadConnxToAspMsg.streamOut = true;

        /* And send the message. */
        fprintf(stdout, "%s: Sending ASP_ProxyServerMsgOffloadConnxToAsp on fdPayload=%d\n", __FUNCTION__, hAspChannelMgr->fdPayload);
        brc = ASP_ProxyServerMsg_Send( hAspChannelMgr->fdPayload, ASP_ProxyServerMsg_eOffloadConnxToAsp, sizeof(offloadConnxToAspMsg), &offloadConnxToAspMsg);
        assert(brc == true);
    }

    /* Recv the status of ASP Offload operation back to the DUT. */
    {
        ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;
        ASP_ProxyServerMsgOffloadConnxToAspResp offloadConnxToAspMsgResp;

        sleep(2);

        fprintf(stdout, "%s: Recv Response of type ASP_ProxyServerMsg_eOffloadConnxToAspResp.\n", __FUNCTION__);
        brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdPayload, &aspProxyServerMsgHeader);
        assert(brc == true);
        assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eOffloadConnxToAspResp);

        brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdPayload, sizeof(offloadConnxToAspMsgResp), &offloadConnxToAspMsgResp);
        assert(brc == true);
        fprintf(stdout, "%s: Connx Offload Response status: offloadToAspSuccessful=%s\n", __FUNCTION__, offloadConnxToAspMsgResp.offloadToAspSuccessful?"Y":"N");
    }

    /*
     * Note: we no longer close the sockFdToOffload after offloading it to ASP.
     * This makes Linux to reserve the local & remote port combination and
     * thus not allocate it to another connection between the same peers.
     * We will close this migrated fd right before offloading connx back
     * (in the ASP_ChannelMgr_Stop().
     */
    hAspChannelMgr->fdMigratedConnx = sockFdToOffload;
    fprintf(stdout, "%s: Offloaded connection From Host to ASP: offloadedSockFd=%d\n", __FUNCTION__, sockFdToOffload);
    return (0);
}

/*
 * ASP_ChannelMgr_Stop()
 */
int
ASP_ChannelMgr_Stop(
    ASP_ChannelMgrHandle    hAspChannelMgr,
    int                     *pSockFd        /* out: fd of socket offloaded back from ASP to host. */
    )
{
    int rc;
    bool brc;
    ASP_ProxyServerMsgOffloadConnxFromAsp offloadConnxFromAspMsg;
    SocketState socketState;

    /* Prepare OffloadConnxFromAsp message & send it to ASP Proxy Server. */
    {
        /* Fill-in msg fields. */
        offloadConnxFromAspMsg.unused = 1;

        /* And send the message. */
        fprintf(stdout, "%s: Sending ASP_ProxyServerMsgOffloadConnxFromAsp on fdPayload=%d\n", __FUNCTION__, hAspChannelMgr->fdPayload);
        brc = ASP_ProxyServerMsg_Send( hAspChannelMgr->fdPayload, ASP_ProxyServerMsg_eOffloadConnxFromAsp, sizeof(offloadConnxFromAspMsg), &offloadConnxFromAspMsg);
        assert(brc == true);
    }

    /* Recv the state of the offloaded connection back from ASP. */
    {
        ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;
        ASP_ProxyServerMsgOffloadConnxFromAspResp offloadConnxFromAspMsgResp;

        fprintf(stdout, "%s: Recv Response of type ASP_ProxyServerMsgOffloadConnxFromAsp.\n", __FUNCTION__);
        brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdPayload, &aspProxyServerMsgHeader);
        assert(brc == true);
        assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eOffloadConnxFromAspResp);

        brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdPayload, sizeof(offloadConnxFromAspMsgResp), &offloadConnxFromAspMsgResp);
        assert(brc == true);
        fprintf(stdout, "%s: Connx Offload back to host: TCP state: seq=%u ack=%u\n", __FUNCTION__, offloadConnxFromAspMsgResp.socketState.tcpState.seq, offloadConnxFromAspMsgResp.socketState.tcpState.ack);
        socketState = offloadConnxFromAspMsgResp.socketState;
    }

    /* Close connections to ASP Proxy Server. */
    {
        if(hAspChannelMgr->hRawFrameFdIoChecker)
        {
            BIP_IoChecker_Disable( hAspChannelMgr->hRawFrameFdIoChecker, BIP_IoCheckerEvent_ePollIn);

            BIP_IoChecker_Destroy(hAspChannelMgr->hRawFrameFdIoChecker);
            fprintf(stdout, "%s: Destroyed hRawFrameFdIoChecker\n", __FUNCTION__);
        }

        close(hAspChannelMgr->fdPayload);
        close(hAspChannelMgr->fdEvents);
        close(hAspChannelMgr->fdRawFrame);
    }

    /* Offload connection back to Linux using the latest TCP state received from the ASP. */
    {
        rc = ASP_ConnxMigration_SetTcpStateToLinux(&socketState, hAspChannelMgr->fdMigratedConnx, pSockFd );
        assert(rc == 0);
    }

    fprintf(stdout, "%s: Offloaded connection from ASP to Host: sockFd=%d\n", __FUNCTION__, *pSockFd);
    return (0);
}


int ASP_ChannelMgr_SendPayload(
    ASP_ChannelMgrHandle    hAspChannelMgr,
    void                    *pPayload,
    size_t                  payloadLength
    )
{
    bool brc;
    ASP_ProxyServerMsgSendPayloadToAspResp sendPayloadToAspResp;

    /* Send Paylod to ASP via the ASP ProxyServer. */
    {
        fprintf(stdout, "%s: Sending %zu bytes of payload to ASP on fdPayload=%d\n", __FUNCTION__, payloadLength, hAspChannelMgr->fdPayload);
        brc = ASP_ProxyServerMsg_Send( hAspChannelMgr->fdPayload, ASP_ProxyServerMsg_eSendPayloadToAsp, payloadLength, pPayload);
        assert(brc == true);
    }

    /* Recv the status of SendPayloadToAsp message. */
    {
        ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;

        fprintf(stdout, "%s: Recv Response of type ASP_ProxyServerMsg_eSendPayloadToAspResp\n", __FUNCTION__);
        brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdPayload, &aspProxyServerMsgHeader);
        assert(brc == true);
#if 0
        fprintf(stdout, "%s: Header type is %d expected type is %d, header length is %d\n", __FUNCTION__, aspProxyServerMsgHeader.type, ASP_ProxyServerMsg_eSendPayloadToAspResp, aspProxyServerMsgHeader.length );
#endif
        assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eSendPayloadToAspResp);

        brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdPayload, sizeof(sendPayloadToAspResp), &sendPayloadToAspResp);
        assert(brc == true);
        fprintf(stdout, "%s: sendPayloadToAsp status: payloadGivenToAspSuccess=%s\n", __FUNCTION__, sendPayloadToAspResp.payloadGivenToAspSuccess?"Y":"N");
    }

#if 0
    /* TODO: This will be removed once asp driver is ready.*/
    /* Now recv the ASP_ProxyServerMsg_eSendRawFrameToNw */
    {
        ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;
        char *pRawFrameBuffer = NULL;

        fprintf(stdout, "%s: Recv message of type ASP_ProxyServerMsg_eSendRawFrameToNw\n", __FUNCTION__);
        brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdRawFrame, &aspProxyServerMsgHeader);
        assert(brc == true);
        assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eSendRawFrameToNw);


        /* Allocate memory for raw frame. */
        pRawFrameBuffer = calloc(1, (aspProxyServerMsgHeader.length*4));

        brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdRawFrame, aspProxyServerMsgHeader.length, pRawFrameBuffer);
        assert(brc == true);

        fprintf(stdout, "%s: message payload size = %d----------------------\n", __FUNCTION__, aspProxyServerMsgHeader.length);
        brc = sendRawFrameToNw( hAspChannelMgr, pRawFrameBuffer, aspProxyServerMsgHeader.length);
        assert(brc != -1);

        if(pRawFrameBuffer)
        {
            free(pRawFrameBuffer);
        }
    }
#endif

    return (0);
}


/* API to recv payload via on offloaded connection. */
ssize_t ASP_ChannelMgr_RecvPayload(
    ASP_ChannelMgrHandle    hAspChannelMgr,
    void                    *pPayload,
    size_t                  payloadSize
    )
{
    bool                    brc;
    ssize_t                 payloadLength;
    ASP_ProxyServerMsgRecvPayloadFromAsp recvPayloadFromAsp;

    /* Send RecvPayload Message to ASP via ASP ProxyServer. */
    {
        fprintf(stdout, "%s: Sending message to recv payload %zu bytes to ASP on fdPayload=%d\n", __FUNCTION__, payloadSize, hAspChannelMgr->fdPayload);
        recvPayloadFromAsp.payloadSize = payloadSize;
        brc = ASP_ProxyServerMsg_Send( hAspChannelMgr->fdPayload, ASP_ProxyServerMsg_eRecvPayloadFromAsp, sizeof(recvPayloadFromAsp), &recvPayloadFromAsp);
        assert(brc == true);
    }

    /* Recv the Payload back. */
    {
        ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;

        fprintf(stdout, "%s: Recv Response of type ASP_ProxyServerMsg_eRecvPayloadFromAspResp\n", __FUNCTION__);
        brc = ASP_ProxyServerMsg_RecvHeader(hAspChannelMgr->fdPayload, &aspProxyServerMsgHeader);
        assert(brc == true);
        assert(aspProxyServerMsgHeader.type == ASP_ProxyServerMsg_eRecvPayloadFromAspResp);

        payloadLength = aspProxyServerMsgHeader.length;
        assert( aspProxyServerMsgHeader.length <= payloadSize );
        brc = ASP_ProxyServerMsg_RecvPayload(hAspChannelMgr->fdPayload, payloadLength, pPayload);
        assert(brc == true);
        fprintf(stdout, "%s: RecvPayload status: rcvd=%zd bytes from ASP\n", __FUNCTION__, payloadLength);
    }

    return (payloadLength);
}
