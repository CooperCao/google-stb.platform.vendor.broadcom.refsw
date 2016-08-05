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

#if 0
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>


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

#include "asp_proxy_server_api.h"
#include "nexus_aspsim_api.h"
#include "asp_xpt_api.h"
#include "asp_nw_sw_api.h"


typedef enum ASP_ProxyServerSocketState
{
    ASP_ProxyServerSocketState_eUninit,
    ASP_ProxyServerSocketState_eWaitingForRawFrameSocket,
    ASP_ProxyServerSocketState_eWaitingForEventSocket,
    ASP_ProxyServerSocketState_eWaitingForMessages,
    ASP_ProxyServerSocketState_eWaitingForStartResponseFromAsp,
    ASP_ProxyServerSocketState_eWaitingForStopResponseFromAsp,
    ASP_ProxyServerSocketState_eStopped,
    ASP_ProxyServerSocketState_eMax
} ASP_ProxyServerSocketState;

typedef struct ASP_ProxyServerSocket
{
    ASP_ProxyServerSocketMsgInfo    payloadSocketInfo;
    ASP_ProxyServerSocketMsgInfo    rawFrameSocketInfo;
    ASP_ProxyServerSocketMsgInfo    eventSocketInfo;
    ASP_ProxyServerSocketState      state;
    ASP_ProxyServerHandle           hProxyServer;
    SocketState                     socketState;
    void                            *pPayload; /* This is used like scratch buffer to collect payload. Allocated when needed and as soon as done freed the memory.*/
    NEXUS_AspSim_ChannelHandle      hNexusAspSimChannel;
    ASP_XptMcpbChannelHandle        hXptMcpbChannel;
    ASP_NwSwFlowHandle              hNwSwFlow;

    /* TODO: listNext for serverSocket list. */
} ASP_ProxyServerSocket;
typedef ASP_ProxyServerSocket* ASP_ProxyServerSocketHandle;

typedef struct ASP_ProxyServer
{
    ASP_ProxyServerCreateSettings createSettings;
    int fdListenerPayload;
    int fdListenerRawFrame;
    int fdListenerEvent;
    /* TODO: make it a listHead for serverSocket lists. */
    ASP_ProxyServerSocketHandle hProxyServerSocket; /* this should be removed when listHead is added. */
} ASP_ProxyServer;

/*
 * ASP_ProxyServer_Destroy()
 *  API to Destroy the Proxy Server related resources & Context.
 */
void
ASP_ProxyServer_Destory(
    ASP_ProxyServerHandle hProxyServer
    )
{
    /* Cleanup */

    close(hProxyServer->fdListenerPayload);
    close(hProxyServer->fdListenerEvent);
    close(hProxyServer->fdListenerRawFrame);
    free(hProxyServer);
}

static int
createListener(
    const char *pListenerPort
    )
{
    int nonblock = 1;
    int sockFd;
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(atoi(pListenerPort));
    localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(sockFd > 0);

    assert(bind(sockFd, (struct sockaddr *)&localAddr, localAddrLen) == 0);

    /* Make the socket non-blocking. */
    assert (ioctl(sockFd, FIONBIO, &nonblock) == 0);

    /* Start listening. */
    assert (listen(sockFd, 32) == 0);

    fprintf(stdout, "%s: listener to port=%s is ready, sockFd=%d\n", __FUNCTION__, pListenerPort, sockFd);
    return (sockFd);
}

/*
 * ASP_ProxyServer_Create: called one time during the initialization phase
 */
ASP_ProxyServerHandle
ASP_ProxyServer_Create(
    ASP_ProxyServerCreateSettings *pSettings
    )
{
    ASP_ProxyServerHandle hProxyServer;
    ASP_ProxyServerCreateSettings settings;

    /* Malloc ASP_ProxyServer */
    hProxyServer = calloc(1, sizeof(ASP_ProxyServer));
    assert(hProxyServer);

    if (!pSettings)
    {
        ASP_ProxyServer_GetDefaultCreateSettings(&settings);
        pSettings = &settings;
    }

    /* Create listener sockets for sending & receiving payload, rawFrame, and listenerEvents from the ASP DUT. */
    hProxyServer->fdListenerPayload = createListener(pSettings->pPayloadListenerPort);
    assert(hProxyServer->fdListenerPayload > 0);

    hProxyServer->fdListenerRawFrame = createListener(pSettings->pRawFrameListenerPort);
    assert(hProxyServer->fdListenerRawFrame > 0);

    hProxyServer->fdListenerEvent = createListener(pSettings->pEventListenerPort);
    assert(hProxyServer->fdListenerEvent > 0);

    return (hProxyServer);
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

int sendRawFrameToNw(void *context, int param, void *rawFramebuffer, size_t frameSize)
{
    int rc ;
    ASP_ProxyServerSocketHandle hProxyServerSocket = (ASP_ProxyServerSocketHandle)context;
    rc = ASP_ProxyServerMsg_Send(hProxyServerSocket->rawFrameSocketInfo.sockFd, ASP_ProxyServerMsg_eSendRawFrameToNw, frameSize, rawFramebuffer);
    assert(rc);
    return rc;
}

static void
processOffloadConnxToAspMessage(
    ASP_ProxyServerSocketHandle hProxyServerSocket,
    ASP_ProxyServerMsgOffloadConnxToAsp *pOffloadConnxToAspMsg
    )
{
    int rc;
    fprintf(stdout, "%s: hProxyServerSocket=%p: seq=%u ack=%u\n",
            __FUNCTION__, hProxyServerSocket, pOffloadConnxToAspMsg->socketState.tcpState.seq, pOffloadConnxToAspMsg->socketState.tcpState.ack);
    hProxyServerSocket->socketState = pOffloadConnxToAspMsg->socketState;

    /* Open an nexus asp channel and then invoke the start api.*/
    {
        NEXUS_AspSim_ChannelOpenSettings sNexusAspChannelOpenSettings;
        NEXUS_AspSim_ChannelStartSettings sNexusAspChannelStartSettings;
        ASP_SocketInfo  sAspSocketInfo;
        ASP_FeedRawFrameCallbackDesc feedRawFrameDesc;
        int32_t flowId;

        /* Setup Mcpb Channel.*/
        hProxyServerSocket->hXptMcpbChannel = ASP_XptMcpbChannel_Open();
        rc = ASP_XptMcpbChannel_Start(hProxyServerSocket->hXptMcpbChannel);
        assert(rc==0);

        /* Setup nw sw module.*/
        /* TODO: populate sAspSocketInfo structure from the pOffloadConnxToAspMsg*/
        feedRawFrameDesc.callback = sendRawFrameToNw;
        feedRawFrameDesc.pContext = hProxyServerSocket;
        hProxyServerSocket->hNwSwFlow = ASP_NwSwFlow_Open(&sAspSocketInfo, &feedRawFrameDesc, &flowId);

        NEXUS_AspSim_Channel_GetDefaultOpenSettings( &sNexusAspChannelOpenSettings );
        hProxyServerSocket->hNexusAspSimChannel = NEXUS_AspSim_Channel_Open(&sNexusAspChannelOpenSettings);
        assert( hProxyServerSocket->hNexusAspSimChannel );

        NEXUS_AspSim_Channel_GetDefaultStartSettings( &sNexusAspChannelStartSettings );
        sNexusAspChannelStartSettings.socketState = pOffloadConnxToAspMsg->socketState;

        /* TODO:call ASP_XptMcpbChannel_GetBuffer since for streamout case we have to pass this as a part of start message.
           Later this will be replaced with raveBufferInfo as defined in actual StreamOutStart message.*/
        sNexusAspChannelStartSettings.pXptMcpbBuffer = ASP_XptMcpbChannel_GetBuffer(hProxyServerSocket->hXptMcpbChannel);
        assert(sNexusAspChannelStartSettings.pXptMcpbBuffer);

        /*TODO: Later set other start specific messages, like sw cfg , xpt info etc..*/
        rc = NEXUS_AspSim_Channel_Start(hProxyServerSocket->hNexusAspSimChannel, &sNexusAspChannelStartSettings);
        assert(rc==0);

        rc = ASP_NwSwFlow_Start(hProxyServerSocket->hNwSwFlow);
        assert(rc==0);

        hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForStartResponseFromAsp;
    }

}

static void
processOffloadConnxFromAspMessage(
    ASP_ProxyServerSocketHandle hProxyServerSocket,
    ASP_ProxyServerMsgOffloadConnxFromAsp *pOffloadConnxFromAspMsg
    )
{
    int rc;

    rc = ASP_NwSwFlow_Stop( hProxyServerSocket->hNwSwFlow );
    assert(rc == 0);

    rc = NEXUS_AspSim_Channel_Stop( hProxyServerSocket->hNexusAspSimChannel );
    assert(rc == 0);

    rc = ASP_XptMcpbChannel_Stop( hProxyServerSocket->hXptMcpbChannel );
    assert(rc == 0);

    hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForStopResponseFromAsp;

}

static void
processSendPayloadToAspMessage(
    ASP_ProxyServerSocketHandle hProxyServerSocket,
    size_t payloadLength,
    void *pPayload
    )
{

#if 0
    /* TODO: Add Call to ASP_McpbChannel_Send to send the payload. */
    {
        /* For now, just do a test print of payload. */
        int i=0;
        uint8_t *pBuf = pPayload;
        fprintf(stdout, "%s: payloadLength=%zu: payload:\n", __FUNCTION__, payloadLength);
        while (i<(int)payloadLength)
        {
            fprintf(stdout, "%02u ", pBuf[i++]);
            if (i%8 == 0) fprintf(stdout, "\n");
        }
    }
#endif

    /* First check whether the corresponding nexus_aspChannel is started or not.
       if started then call ASP_XptMcpbChannel_FeedPayload.*/
    if(NEXUS_AspSim_Channel_IsStarted(hProxyServerSocket->hNexusAspSimChannel))
    {
        ASP_XptMcpbChannel_FeedPayload( hProxyServerSocket->hXptMcpbChannel, pPayload, payloadLength);

        /* as soon as this function is done we assume that the payload has been consumed by asp simulator.*/
        {
            /* This means payload just consumed so send the ASP_ProxyServerMsg_eSendPayloadToAspResp .*/
            int rc;

            ASP_ProxyServerMsgSendPayloadToAspResp sendPayloadToAspResp;
            sendPayloadToAspResp.payloadGivenToAspSuccess = true;
            rc = ASP_ProxyServerMsg_Send(hProxyServerSocket->payloadSocketInfo.sockFd, ASP_ProxyServerMsg_eSendPayloadToAspResp, sizeof(sendPayloadToAspResp), &sendPayloadToAspResp);
            assert(rc);
        }
    }

}

static void
processRecvPayloadFromAspMessage(
    ASP_ProxyServerSocketHandle hProxyServerSocket,
    ASP_ProxyServerMsgRecvPayloadFromAsp *pRecvPayloadFromAsp
    )
{
    /* TODO: Add Call to ASP_M2mDma2mDmaChannel_Recv to recv the payload. */
    fprintf(stdout, "%s: DUT is asking for %zu bytes to recv from ASP\n", __FUNCTION__, pRecvPayloadFromAsp->payloadSize);

    /* Once we have read payload from M2M DMA, send it over to the DUT. */
    {
        int rc;
#define PAYLOAD_SIZE 16
        int i;
        uint8_t payload[PAYLOAD_SIZE];

        for (i=0; i < PAYLOAD_SIZE; i++) payload[i] = i*2;
        fprintf(stdout, "%s: Send RecvPayload Resp\n", __FUNCTION__ );
        rc = ASP_ProxyServerMsg_Send(hProxyServerSocket->payloadSocketInfo.sockFd, ASP_ProxyServerMsg_eRecvPayloadFromAspResp, PAYLOAD_SIZE, payload);
        assert(rc);
    }
}

static int
processMessage(
    ASP_ProxyServerSocketHandle hProxyServerSocket,
    ASP_ProxyServerSocketMsgInfoHandle hSocketMsgInfo
    )
{
    bool msgRcvd = false;

    /* If we haven't currently read a msgHeader, check if msgHeader is available */
    if (!hSocketMsgInfo->msgHeaderValid)
    {
        hSocketMsgInfo->msgHeaderValid = ASP_ProxyServerMsg_RecvHeader(hSocketMsgInfo->sockFd, &hSocketMsgInfo->msgHeader);
    }

    /* If we have a valid msgHeader, then check if its correspoding msgPayload is available. */
    if (hSocketMsgInfo->msgHeaderValid)
    {
        /* socket contains the msg header, so read the actual message and then process it. */
        if (hSocketMsgInfo->msgHeader.type == ASP_ProxyServerMsg_eOffloadConnxToAsp)
        {
            ASP_ProxyServerMsgOffloadConnxToAsp offloadConnxToAspMsg;

            msgRcvd = ASP_ProxyServerMsg_RecvPayload(hSocketMsgInfo->sockFd, hSocketMsgInfo->msgHeader.length, &offloadConnxToAspMsg);
            fprintf(stdout,"\n%s: ASP_ProxyServerMsg_eOffloadConnxToAsp received-----------------------------\n", __FUNCTION__);
            if (msgRcvd)
            {
                processOffloadConnxToAspMessage(hProxyServerSocket, &offloadConnxToAspMsg);
                hSocketMsgInfo->msgHeaderValid = false; /* set to false as we have now processed this msg. */
                fprintf(stdout,"\n%s: processOffloadConnxToAspMessage Done-----------------------------\n", __FUNCTION__);
            }
            else
            {
                fprintf(stdout,"full msg is not yet available, returning ....\n");
            }
        }
        else if (hSocketMsgInfo->msgHeader.type == ASP_ProxyServerMsg_eOffloadConnxFromAsp)
        {
            ASP_ProxyServerMsgOffloadConnxFromAsp offloadConnxFromAspMsg;

            fprintf(stdout,"\n%s: ASP_ProxyServerMsg_eOffloadConnxFromAsp received-----------------------------\n", __FUNCTION__);

            msgRcvd = ASP_ProxyServerMsg_RecvPayload(hSocketMsgInfo->sockFd, hSocketMsgInfo->msgHeader.length, &offloadConnxFromAspMsg);
            if (msgRcvd)
            {
                processOffloadConnxFromAspMessage(hProxyServerSocket, &offloadConnxFromAspMsg);
                hSocketMsgInfo->msgHeaderValid = false; /* set to false as we have now processed this msg. */
                fprintf(stdout,"\n%s: processOffloadConnxFromAspMessage Done-----------------------------\n", __FUNCTION__);
            }
            else
            {
               fprintf(stdout,"full msg is not yet available, returning ....\n");
            }
        }
        else if (hSocketMsgInfo->msgHeader.type == ASP_ProxyServerMsg_eSendPayloadToAsp)
        {
            size_t payloadLength = hSocketMsgInfo->msgHeader.length;

            if (hProxyServerSocket->pPayload == NULL)
            {
                hProxyServerSocket->pPayload = calloc(1, payloadLength);
                assert(hProxyServerSocket->pPayload);
            }

            fprintf(stdout,"\n%s: ASP_ProxyServerMsg_eSendPayloadToAsp received-----------------------------\n", __FUNCTION__);
            msgRcvd = ASP_ProxyServerMsg_RecvPayload(hSocketMsgInfo->sockFd, payloadLength, hProxyServerSocket->pPayload);
            if (msgRcvd)
            {
                processSendPayloadToAspMessage(hProxyServerSocket, payloadLength, hProxyServerSocket->pPayload);
                hSocketMsgInfo->msgHeaderValid = false; /* set to false as we have now processed this msg. */
                free(hProxyServerSocket->pPayload);
                hProxyServerSocket->pPayload = NULL;
                fprintf(stdout,"\n%s: processSendPayloadToAspMessage Done-----------------------------\n", __FUNCTION__);
            }
            else
            {
                fprintf(stdout,"full msg is not yet available, returning ....\n");
            }
        }
        else if (hSocketMsgInfo->msgHeader.type == ASP_ProxyServerMsg_eRecvPayloadFromAsp)
        {
            ASP_ProxyServerMsgRecvPayloadFromAsp recvPayloadFromAsp;

            assert(hSocketMsgInfo->msgHeader.length == sizeof(recvPayloadFromAsp));
            fprintf(stdout,"\n%s: ASP_ProxyServerMsg_eRecvPayloadFromAsp received-----------------------------\n", __FUNCTION__);
            msgRcvd = ASP_ProxyServerMsg_RecvPayload(hSocketMsgInfo->sockFd, hSocketMsgInfo->msgHeader.length, &recvPayloadFromAsp);
            if (msgRcvd)
            {
                processRecvPayloadFromAspMessage(hProxyServerSocket, &recvPayloadFromAsp);
                hSocketMsgInfo->msgHeaderValid = false; /* set to false as we have now processed this msg. */
            }
            else
            {
                fprintf(stdout,"full msg is not yet available, returning ....\n");
            }
        }
    }

    return 0;
}

int
ASP_ProxyServer_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    )
{
    int newSockFd;
    ASP_ProxyServerSocketHandle hProxyServerSocket;

    /*
     * For Each ServerSocket, do following:
     */
    if ((hProxyServerSocket = hProxyServer->hProxyServerSocket) != NULL)     /* TODO: iterate thru the link list. */
    {
        if (hProxyServerSocket->state == ASP_ProxyServerSocketState_eWaitingForRawFrameSocket)
        {
            newSockFd = acceptConnection(hProxyServer->fdListenerRawFrame);
            if (newSockFd > 0)
            {
                hProxyServerSocket->rawFrameSocketInfo.sockFd = newSockFd;
                hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForEventSocket;
            }

        }
        if (hProxyServerSocket->state == ASP_ProxyServerSocketState_eWaitingForEventSocket)
        {
            newSockFd = acceptConnection(hProxyServer->fdListenerEvent);
            if (newSockFd > 0)
            {
                hProxyServerSocket->eventSocketInfo.sockFd = newSockFd;
                hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForMessages;
            }
        }
        if (hProxyServerSocket->state == ASP_ProxyServerSocketState_eWaitingForMessages)
        {
            /* Read Message from each type socket and call corresponding API to process them. */
            processMessage(hProxyServerSocket, &hProxyServerSocket->payloadSocketInfo);
            processMessage(hProxyServerSocket, &hProxyServerSocket->rawFrameSocketInfo);
            processMessage(hProxyServerSocket, &hProxyServerSocket->eventSocketInfo);
        }
    }

    /* Check if there is a new connection that should be accepted. */
    if (hProxyServer->hProxyServerSocket == NULL) /* TODO: for now, we only accept one client connection at a time. */
    {
        newSockFd = acceptConnection(hProxyServer->fdListenerPayload);
        if (newSockFd > 0)
        {
            /* Yes, we have a new connection from DUT side, so create a new ServerSocket object to hold the state. */
            hProxyServerSocket = calloc(1, sizeof(ASP_ProxyServerSocket));
            assert(hProxyServerSocket);
            hProxyServerSocket->payloadSocketInfo.sockFd = newSockFd;
            hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForRawFrameSocket;
            hProxyServerSocket->hProxyServer = hProxyServer;

            /* TODO: instead of this, insert this ServerSocket to the list. */
            hProxyServer->hProxyServerSocket = hProxyServerSocket;

        }
    }
    return 0;
}

/* later this function can be moved to another file.*/
int
ASP_Nexus_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    )
{
    int rc = 0;
    ASP_ProxyServerSocketHandle hProxyServerSocket = NULL;
    SocketState sSocketState;

    if ((hProxyServerSocket = hProxyServer->hProxyServerSocket) != NULL)     /* TODO: iterate thru the link list. */
    {
        if(hProxyServerSocket->hNexusAspSimChannel != NULL) {
             Nexus_AspSim_ProcessIo(hProxyServerSocket->hNexusAspSimChannel);

             if(NEXUS_AspSim_Channel_IsStarted(hProxyServerSocket->hNexusAspSimChannel)
                && ASP_ProxyServerSocketState_eWaitingForStartResponseFromAsp == hProxyServerSocket->state )
             {

                 /* Send the status of ASP Offload operation back to the DUT. */
                 {
                     ASP_ProxyServerMsgOffloadConnxToAspResp offloadConnxToAspMsgResp;
                     offloadConnxToAspMsgResp.offloadToAspSuccessful = true;
                     rc = ASP_ProxyServerMsg_Send(hProxyServerSocket->payloadSocketInfo.sockFd, ASP_ProxyServerMsg_eOffloadConnxToAspResp, sizeof(offloadConnxToAspMsgResp), &offloadConnxToAspMsgResp);
                     assert(rc);
                 }

                 /*TODO: Once StartResponse is send back to asp manager we will set the state to ASP_ProxyServerSocketState_eWaitingForMessages.
                   Check if this has any side effect. It may not work if we allow Asp manager to send another command before they
                   recive a response for the last command.*/
                 hProxyServerSocket->state = ASP_ProxyServerSocketState_eWaitingForMessages;
             }
             else if(NEXUS_AspSim_Channel_IsStopped(hProxyServerSocket->hNexusAspSimChannel)
                && ASP_ProxyServerSocketState_eWaitingForStopResponseFromAsp == hProxyServerSocket->state )
             {
                 printf("\n%s: NEXUS_AspSim_Channel_IsStopped --------------------------------------------->\n",__FUNCTION__);
                 /* Now at the end check if  AspChannel is stopped,
                   then first get the latestSocket state from aspChannel object,
                   then call NEXUS_AspSim_Channel_Close.
                   send the ASP_ProxyServerMsg_eOffloadConnxFromAspResp.
                   */

                 /*TODO: get the unique channel number along with channel socketstate to migrate the connection back.*/
                 rc = NEXUS_AspSim_Channel_GetLatestSocketState( hProxyServerSocket->hNexusAspSimChannel, &sSocketState );

                 /* Now we can close the Nexus_AspSim_channel object.*/
                 ASP_NwSwFlow_Close( hProxyServerSocket->hNwSwFlow );
                 NEXUS_AspSim_Channel_Close( hProxyServerSocket->hNexusAspSimChannel );
                 ASP_XptMcpbChannel_Close( hProxyServerSocket->hXptMcpbChannel );

                 /* Send ASP_ProxyServerMsg_eOffloadConnxFromAspResp */
                 /* Stop response message will have the latest socket state for connection migration back to linux .
                    Stop response has received or not will be checked from ASP_CModel_ProcessIo function.*/
                {
                    ASP_ProxyServerMsgOffloadConnxFromAspResp offloadConnxFromAspMsgResp;
                    offloadConnxFromAspMsgResp.socketState = sSocketState;
                    rc = ASP_ProxyServerMsg_Send(hProxyServerSocket->payloadSocketInfo.sockFd, ASP_ProxyServerMsg_eOffloadConnxFromAspResp, sizeof(offloadConnxFromAspMsgResp), &offloadConnxFromAspMsgResp);
                    assert(rc);
                }

                hProxyServerSocket->state = ASP_ProxyServerSocketState_eStopped;

                /* Now free hProxyServerSocket.*/
                /* TODO: Later need to check whether to call a destroy and create for this. Also once we create list, remove from the list.*/
                if(hProxyServerSocket)
                {
                    /* close all the sockets */
                    close(hProxyServerSocket->eventSocketInfo.sockFd);
                    close(hProxyServerSocket->payloadSocketInfo.sockFd);
                    close(hProxyServerSocket->rawFrameSocketInfo.sockFd);
                    free(hProxyServerSocket);
                }
                hProxyServer->hProxyServerSocket = NULL;

             }
        }

         /* Now we are done with the ServerSocket,destroy the serverSocket.*/
    }
    return 0;
}

/* later this function can be moved to another file.*/
int Asp_Simulator_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    )
{
    int rc = 0;
    Asp_Simulator_ProcessMessages();

    Asp_Simulator_ProcessPayloadData();

    return (0);
}
