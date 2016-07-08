/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <asm-generic/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/tcp.h>
#include <netinet/ip.h>
#include <pthread.h>
#include "bip_server.h"
#include "tcp_connx_migration.h"
#include "bip_server_priv.h"
#include "nexus_asp.h"
#define BSTD_UNUSED(x) ((void )x)

int BIP_Server_Stop(BipServerHandle serverCtx)
{
    int rc;
    int socketFd;

    myPrint("Press any key to proceed with Connection Migration back to Linux !!!");
    printLinuxConnxState();
    getchar();
    /* stop the ASP channel: this ensures that ASP has stopped streaming and synchronized the TCP state */
    /* this means that all ACKs for any streamed out data has been received */
    /* also, this call returns the current TCP session state */
    rc = NEXUS_Asp_Stop(&serverCtx->socketState);
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_Stop failed...", rc, error);

    /* now migrate the connection back to the host */
    rc = setTcpStateAndMigrateToLinux(&serverCtx->socketState, &socketFd);
    CHECK_ERR_NZ_GOTO("setTcpStateAndMigrateToLinux Failed ...", rc, error);

    /* stop filtering of packets */
    stopAspManager();

    myPrint("Press any key to proceed with Connection tear-down !!!");
    getchar();
    /* now close the socket: Linux should send out a FIN to the client */
    close(socketFd);
    myPrint("Linux socket status: server side TCP connection should now be in the TIME_WAIT state !!!!!");
    printLinuxConnxState();

    printf("%s: Done\n", __FUNCTION__);
    return 0;

error:
    return -1;
}

int BIP_Server_SendHttpResponse(BipServerHandle serverCtx, char *initialData, size_t initialDataLen)
{
    serverCtx->respBuffer = malloc(initialDataLen+1);
    CHECK_PTR_GOTO("BIPServer RespBuffer alloc Failed...", serverCtx->respBuffer, error);
    /* cache a copy of response buffer */
    /* it will be sent by ASP via the BIP_Server_Start() or if app doesn't call that (in error http response case) */
    /* it will sent by the connection thread callback */
    memcpy(serverCtx->respBuffer, initialData, initialDataLen);
    serverCtx->respBuffer[initialDataLen] = '\0';
    serverCtx->respBufferSize = initialDataLen;
    printf("%s: cached the HTTP response\n", __FUNCTION__);
    return 0;
error:
    return -1;
}

int BIP_Server_Start(BipServerHandle serverCtx, char *fileName)
{
    int rc;
    int socketFd = serverCtx->socketFd;
    BSTD_UNUSED(fileName);

    /* we determine here if we can use ASP for this streaming session (true for most streaming out sessions) */
    /* for now, assume that we can and proceed below w/ ASP setup */
    myPrint("BIP_Server_Start(): Press any key to start TCP Connection Offload to ASP simulator....");
    getchar();

    /* setup Asp Manager: it handles all runtime packets that ASP h/w can't directly receive or process */
    /* e.g. fragmented packets, ICMP PATH MPU pkts, etc. */
    startAspManager();

    /* gather TCP state info from Linux and Freeze the TCP socket */
    rc = getTcpStateAndMigrateFromLinux(socketFd, &serverCtx->socketState);
    CHECK_ERR_NZ_GOTO("getTcpStateAndMigrateFromLinux Failed ...", rc, error);

    /* now close the socket */
    /* NOTE!!: since we have frozen the socket, it doesn't cause Linux to send out TCP FIN to the client */
    /* And thus the client doesn't know about this migration to ASP */
    close(socketFd);
    myPrint("Linux socket status: server side TCP connection should disappear at this point!!!!!");
    printLinuxConnxState();

#if 0 //Commented for now
    /* pass TCP state to ASP so that it is setup */
    NEXUS_Asp_GetDefaultetSettings();
    NEXUS_Asp_SetSettings();
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_SetSettings failed...", rc, error);
#endif

    /* start the ASP channel: currently passing TCP state here */
    rc = NEXUS_Asp_Start(&serverCtx->socketState, serverCtx->respBuffer, serverCtx->respBufferSize);
    CHECK_ERR_NZ_GOTO("NEXUS_Asp_Start failed...", rc, error);
    serverCtx->respBufferSent = 1;

    /* TODO: now program the switch to redirect all packets to ASP instead of host */

    printf("%s: Done\n", __FUNCTION__);
    return 0;

error:
    return -1;
}

/* **************** connection setup related functions **************** */
void waitForNetworkEvent(int socketFd)
{
    fd_set rfds;
    struct timeval tv;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(socketFd, &rfds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        if ( select(socketFd +1, &rfds, NULL, NULL, &tv) < 0 ) {
            perror("ERROR: select(): exiting...");
            break;
        }

        if (!FD_ISSET(socketFd, &rfds))
            /* No request from Client yet, go back to select loop */
            continue;
        break;
    }

    return;
}

/* waits for incoming HTTP requests, reads it and store it in a buffer */
static int getInitialRequestData(int socketFd, char *reqBuf, size_t reqBufSize, size_t *bytesRead)
{
    waitForNetworkEvent(socketFd);

    /* Read HTTP request */
    *bytesRead = read(socketFd, reqBuf, reqBufSize-1);
    CHECK_ERR_LEZ_GOTO("Failed to read initial request: ", strerror(errno), *bytesRead, error);
    DBG3("reqBuf %p, reqBufSize %d, bytesRead %d\n", reqBuf, reqBufSize, *bytesRead);

    reqBuf[*bytesRead] = '\0'; /* null terminate to make it into a string */
    DBG3("%s: Initial Req (of %d bytes)[\n%s]\n", __FUNCTION__, *bytesRead, reqBuf);
    return 0;

error:
    return -1;
}

int BIP_Server_RecvHttpRequest(BipServerHandle serverCtx, char **initialReqData, size_t *initialReqDataLen)
{
    int rc;

    if (!serverCtx->respBufferSent) {
        /* previous HTTP Response was not yet sent, so send it out */
        /* we can come here for when app meets an HTTP error for a previous request during persistent HTTP sessions */
        if (write(serverCtx->socketFd, serverCtx->respBuffer, serverCtx->respBufferSize) != serverCtx->respBufferSize) {
            printf("%s: Failed to write the HTTP response, errno %d: %s", __FUNCTION__, errno, strerror(errno));
        }
        serverCtx->respBufferSent = 1;
        if (serverCtx->respBuffer)
            free(serverCtx->respBuffer);
        serverCtx->respBuffer = NULL;
    }

    *initialReqData = serverCtx->reqBuffer;
    printf("%s: Waiting for initial data from client ....\n", __FUNCTION__);
    rc = getInitialRequestData(serverCtx->socketFd, *initialReqData, serverCtx->reqBufferSize, initialReqDataLen);
    CHECK_ERR_NZ_GOTO("Failed to Receive Initial Data", rc, error);

    return 0;

error:
    return -1;
}

/* ****************** initialization related functions ****************** */

/* Open the listener service */
void BIP_Server_DestroyTcpListener(ListenerStateHandle listenerState)
{
    int i;
    for (i=0; i<MAX_NUM_LISTENERS; i++) {
        if (listenerState->port == gServerGlobalCtx.listenerState[i].port) {
            gServerGlobalCtx.listenerState[i].refCnt--;
            if (gServerGlobalCtx.listenerState[i].refCnt == 0) {
                listenerState->stopListenerThread = 1;
                sleep(1); /* TODO: BKNI_WaitOnEvent() */
                shutdown(listenerState->socketFd, SHUT_RDWR);
                sleep(1); /* TODO: BKNI_WaitOnEvent() */
                close(listenerState->socketFd);
                gServerGlobalCtx.listenerState[i].port = 0;
            }
        }
    }
    /* TODO: Destroy the main Listener thread */
}

void BIP_Server_GetDefaultListenerSettings(BIP_ServerListenerSettings *settings)
{
    memset(settings, 0, sizeof(BIP_ServerListenerSettings));
}

void BIP_Server_Destroy(BipServerHandle serverCtx)
{
    if (serverCtx) {
        if (serverCtx->reqBuffer)
            free(serverCtx->reqBuffer);
        if (serverCtx->respBuffer)
            free(serverCtx->respBuffer);
        if (serverCtx->connectionThread)
            pthread_join(serverCtx->connectionThread, NULL);
        free(serverCtx);
    }
}

void *BIP_Server_ConnectionThread(void *data)
{
    BipServerHandle serverCtx = (BipServerHandle)data;
    printf("%s: Accepted Connection from %s:%d to ", __FUNCTION__, inet_ntoa(serverCtx->socketState.remoteIpAddr.sin_addr), ntohs(serverCtx->socketState.remoteIpAddr.sin_port));
    printf("local address %s:%d on socketFd %d\n", inet_ntoa(serverCtx->socketState.localIpAddr.sin_addr), ntohs(serverCtx->socketState.localIpAddr.sin_port), serverCtx->socketFd);

    /* invoke the callback and pass return serverCtx; */
    serverCtx->listenerState->settings.newConnectionCallback(serverCtx);

    /* control only comes here when we return from the newConnectionCallback which is at the end of streaming session */

    /* check if app returned because of an error, in that case, it doesn't call BIP_Start() and simply returns to this callback */
    if (!serverCtx->respBufferSent) {
        if (write(serverCtx->socketFd, serverCtx->respBuffer, serverCtx->respBufferSize) != serverCtx->respBufferSize) {
            printf("%s: Failed to write the HTTP response, errno %d: %s", __FUNCTION__, errno, strerror(errno));
        }
    }
    serverCtx->done = 1;
    myPrint("Connection Thread done");
    printLinuxConnxState();
    return NULL;
}

/* This thread listens on all incoming requests for a give listening socket and starts a new child thread to process the new request */
void *BIP_Server_TcpListenerThread(void *data)
{
    int rc;
    int newSocketFd;
    struct sockaddr_in remoteIpAddr;
    int addrLen = sizeof(remoteIpAddr);
    ListenerStateHandle listenerState = (ListenerStateHandle)data;
    BipServerHandle serverCtx = NULL;

    while (!listenerState->stopListenerThread) {
        DBG("%s: Waiting for Next Client Connection Request...\n", __FUNCTION__);

        /* TODO: need to modify waitForNetworkEvent to timeout every 1 sec even if there is no new connection */
        /* this will allow us to periodically release resources for previously allocated serverCtx which are now done */
        waitForNetworkEvent(listenerState->socketFd);

#if 0
        For each entry in the serverCtxList, if serverCtx is in done state, then release its resources */
        /* TODO: keep a list of all the server connection contexts. */
        if (serverCtx->done) {
            BIP_Server_Destroy(serverCtx);
        }
#endif
        /* accept connection */
        newSocketFd = accept(listenerState->socketFd, (struct sockaddr *)&remoteIpAddr, (socklen_t *)&addrLen);
        CHECK_ERR_LZ_GOTO("Failed to accept new connection request: ", strerror(errno), newSocketFd, error);
        DBG2("%s: accepted new connection, new socketFd %d\n", __FUNCTION__, newSocketFd);
        printLinuxConnxState();

        /* now allocate state for this connection & spawn a connection thread to process this incoming connection */
        serverCtx = malloc(sizeof(BIP_ServerCtx));
        CHECK_PTR_GOTO("BIPServer Ctx alloc Failed...", serverCtx, error);
        memset(serverCtx, 0, sizeof(BIP_ServerCtx));
        serverCtx->reqBuffer = malloc(MAX_REQ_BUFFER_SIZE);
        CHECK_PTR_GOTO("BIPServer ReqBuffer alloc Failed...", serverCtx->reqBuffer, error);
        serverCtx->listenerState = listenerState;
        serverCtx->socketFd = newSocketFd;
        serverCtx->reqBufferSize = MAX_REQ_BUFFER_SIZE;
        serverCtx->socketState.remoteIpAddr = remoteIpAddr;
        rc = getsockname(newSocketFd, (struct sockaddr *)&serverCtx->socketState.localIpAddr, (socklen_t *)&addrLen);
        CHECK_ERR_NZ_GOTO("failed to get local address info..", rc, error);

#if 0
        /* TODO: keep a list of all the server connection contexts. Add this serverCtx to this global list */
#endif

        /* now create a thread to carry out the processing related to this new connection */
        if (pthread_create(&serverCtx->connectionThread, NULL, BIP_Server_ConnectionThread, serverCtx)) {
            assert(NULL);
        }
        /* go back and wait on new client connection */
    }

    printf("%s: listener (%p) on port %d is done\n", __FUNCTION__, (void *)listenerState, listenerState->port);
    return NULL;

error:
    /* TODO: more clean up here, free all server ctx belonging to this listener */
    if (serverCtx)
        BIP_Server_Destroy(serverCtx);
    return NULL;
}

/* Open the listener service */
ListenerStateHandle BIP_Server_CreateTcpListener(BIP_ServerListenerSettings *settings)
{
    int i;
    struct sockaddr_in localIpAddr;
    int socketFd;
    int reuse_flag = 1;
    int rc;

    /* Lower the timewait interval as different tests stop & restart the server which causes listener socket to at times be stuck in the TIME_WAIT state. */
    system("echo 1 > /proc/sys/net/ipv4/tcp_fin_timeout");

    /* lookup if we are already listening on a port. If so, increment the refCnt. Otherwise listen on it */
    for (i=0; i<MAX_NUM_LISTENERS; i++) {
        if (gServerGlobalCtx.listenerState[i].refCnt && settings->port == gServerGlobalCtx.listenerState[i].port) {
            gServerGlobalCtx.listenerState[i].refCnt++;
            printf("listener ctx already found in the global list..\n");
            return &gServerGlobalCtx.listenerState[i];
        }
    }

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_ERR_LZ_GOTO("Failed to create new socket: ", strerror(errno), socketFd, error);

    rc = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag));
    CHECK_ERR_LZ_GOTO("Failed to set socket option: ", strerror(errno), rc, error);

    localIpAddr.sin_family = AF_INET;
    localIpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localIpAddr.sin_port = htons(settings->port);
    rc = bind(socketFd, (struct sockaddr *) &localIpAddr, sizeof(localIpAddr));
    CHECK_ERR_LZ_GOTO("Failed to bind to socket: ", strerror(errno), rc, error);

    rc = listen(socketFd, 48);
    CHECK_ERR_LZ_GOTO("Failed to listen on socket: ", strerror(errno), rc, error);

    /* now insert this into the global listener list */
    for (i=0; i<MAX_NUM_LISTENERS; i++) {
        if (gServerGlobalCtx.listenerState[i].refCnt == 0) {
            gServerGlobalCtx.listenerState[i].refCnt = 1;
            gServerGlobalCtx.listenerState[i].socketFd = socketFd;
            gServerGlobalCtx.listenerState[i].port = settings->port;
            gServerGlobalCtx.listenerState[i].settings = *settings;
            break;
        }
    }
    if (i == MAX_NUM_LISTENERS) {
        printf("All listeners are currently being used (%d), need to increase MAX_NUM_LISTENERS from %d and re-compile\n", i, MAX_NUM_LISTENERS);
        return NULL;
    }

    /* start the listening thread here to monitor the listening sockets for new connection requests and accept them */
    printf("server listening on port %d, socketFd %d\n", settings->port, socketFd);
    if (pthread_create(&gServerGlobalCtx.listenerState[i].listenerThread, NULL, BIP_Server_TcpListenerThread, &gServerGlobalCtx.listenerState[i])) {
        assert(NULL);
    }

    return &gServerGlobalCtx.listenerState[i];

error:
    printf("%s: Failed", __FUNCTION__);
    return NULL;
}
