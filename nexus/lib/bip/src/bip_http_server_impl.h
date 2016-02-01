/******************************************************************************
 * (c) 2015 Broadcom Corporation
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
 *
 *****************************************************************************/

#ifndef BIP_HTTP_SERVER_IMPL_H
#define BIP_HTTP_SERVER_IMPL_H

#include "bip_priv.h"

/*
 * States for HttpServer Object (main server object),
 *  states reflecting HttpServer APIs visiable to Apps: Create, Start, Stop of Server
 */
typedef enum BIP_HttpServerStartState
{
    BIP_HttpServerStartState_eUninitialized = 0,     /* Initial state: object is just created but fully initialzed */
    BIP_HttpServerStartState_eIdle,                  /* Idle state: Server object is just created, but it hasn't yet started listening for Requests. */
    BIP_HttpServerStartState_eReadyToStart,          /* Server object has the needed Settings (e.g. port #) to start listening. */
    BIP_HttpServerStartState_eStarted,               /* Server is started where it has started Listener and is ready to accept incoming connections & HTTP Requests. */
    BIP_HttpServerStartState_eStopped =              /* Server is stopped and is ready to be started again. */
        BIP_HttpServerStartState_eReadyToStart,
    BIP_HttpServerStartState_eMax
} BIP_HttpServerStartState;

/*
 * State for HttpListener
 *  sub-state of HttpServer that manages BIP_Listener object.
 */
typedef enum BIP_HttpServerListenerState
{
    BIP_HttpServerListenerState_eIdle,              /* Idle state: not enabled connectedCB or calling any of BIP_Listern's APIs. Not expecting any new connx. */
    BIP_HttpServerListenerState_eNew,               /* Transitional state in between eIdle & eListening: does one time tasks for Listening state: e.g. start timer. */
    BIP_HttpServerListenerState_eListening,         /* Waiting for connectedCB from BIP_Listener object: either app enabled reqRcvCB or called _RecvRequest() API. */
    BIP_HttpServerListenerState_eRequestQueueFull,  /* We just accpted a connx and are setting up the new ServerSocket object associated w/ this new connx. */
                                                    /* We are not going to accept any more until we complete receiving 1st Request. */
                                                    /* Avoids DOS attacks, kernel still buffers the incoming connections & requests. */
    BIP_HttpServerListenerState_eMax
} BIP_HttpServerListenerState;

/**
 * BIP_HttpSocket Callback States: has two states:
 *  disabled    no callback specified by caller || we have received one msg/payload and now waiting for app to _Recv it.
 *  enabled     callback specified by caller || a blocking _Recv call is waiting for completion.
 **/
typedef enum BIP_HttpSocketCallbackState
{
    BIP_HttpSocketCallbackState_eDisabled,
    BIP_HttpSocketCallbackState_eEnabled
} BIP_HttpSocketCallbackState;

typedef enum BIP_ListenerCallbackState
{
    BIP_ListenerCallbackState_eDisabled,
    BIP_ListenerCallbackState_eEnabled
} BIP_ListenerCallbackState;

/*
 * State associated with each ServerSocket instance.
 */
typedef enum BIP_HttpServerSocketState
{
    BIP_HttpServerSocketState_eUninitialized = 0,
    BIP_HttpServerSocketState_eIdle,
    BIP_HttpServerSocketState_eWaitingForRequestArrival,    /* Waiting for a New complete HTTP Request on the associated HttpSocket. */
    BIP_HttpServerSocketState_eWaitingForRecvRequestApi,    /* Received a New Request, requestReceivedCallback to App has been invoked and now waiting for _RecvRequest(). */
    BIP_HttpServerSocketState_eWaitingForStartStreamerApi,  /* Caller has issued _RecvRequest to receive the Request and we are now waiting for _StartStreamer(). */
    BIP_HttpServerSocketState_eProcessingRequest,           /* We are now Processing given a HttpRequest by streaming media on the associated HttpSocket. */
    BIP_HttpServerSocketState_eDestroying,                  /* An error has occured on the ServerSocket object & we are now destroying it. */
    BIP_HttpServerSocketState_eMax
} BIP_HttpServerSocketState;

typedef struct BIP_HttpServerSocket
{
    BDBG_OBJECT( BIP_HttpServerSocket )
    BIP_CLASS_DEFINE_INSTANCE_VARS(BIP_HttpServerSocket);
    BIP_Status                          completionStatus;
    BLST_Q_ENTRY(BIP_HttpServerSocket)  httpServerSocketInUseListNext;          /* next BIP_HttpServerSocket object that is in use */
    BLST_Q_ENTRY(BIP_HttpServerSocket)  httpServerSocketRcvdRequestNext;        /* next BIP_HttpServerSocket object has a full Request & is waiting for _RecvRequest() from App!. */
    BIP_HttpServerSocketState           state;
    BIP_HttpServerHandle                hHttpServer;
    BIP_SocketHandle                    hSocket;
    BIP_HttpSocketHandle                hHttpSocket;
    BIP_HttpRequestHandle            hHttpRequest;
    BIP_HttpResponseHandle           hHttpResponse;
    BIP_HttpStreamerHandle              hHttpStreamer;
    BIP_HttpSocketCallbackState         requestReceivedCallbackState;
    bool                                requestProcessed;                       /* set when HttpStreamer invokes the requestProcessed callback. */
    const char                          *pRemoteIpAddress;
    bool                                httpSocketOwnedByCaller;
    struct
    {
        BIP_ArbHandle                   hArb;
        BIP_HttpServerRecvRequestSettings settings;
    } recvRequestApi;
    struct
    {
        unsigned                        numReceivedRequests;        /* # of requests received on this ServerSocket object (can be > 1 when HttpKeepAlive is enabled) */
    } stats;
} BIP_HttpServerSocket;


#define BIP_HTTP_SERVER_SOCKET_PRINTF_FMT  \
    "[hHttpServerSocket=%p numReqs=%u hHttpSrvr=%p hHttpSckt=%p hSckt=%p hHttpRqst=%p hHttpResp=%p hHttpStrmr=%p]"

#define BIP_HTTP_SERVER_SOCKET_PRINTF_ARG(pObj)   \
    (pObj),                                       \
    (pObj)->stats.numReceivedRequests,            \
    (pObj)->hHttpServer,                          \
    (pObj)->hHttpSocket,                          \
    (pObj)->hSocket,                              \
    (pObj)->hHttpRequest,                         \
    (pObj)->hHttpResponse,                        \
    (pObj)->hHttpStreamer


typedef struct BIP_HttpServer
{
    BDBG_OBJECT( BIP_HttpServer )

    BIP_CLASS_DEFINE_INSTANCE_VARS(BIP_HttpServer);

    /* HttpListener specific state */
    struct listener
    {
        BIP_ListenerHandle              hListener;                  /* listener object which invokes connected callbacks for incoming connections. */
        BIP_HttpServerListenerState     state;
        BIP_ListenerCallbackState       callbackState;
        bool                            timerActive;                /* true if timer is currently active */
        BIP_SocketHandle                hSocket;
    } listener;

    BLST_Q_HEAD( httpServerSocketInUseListHead, BIP_HttpServerSocket ) httpServerSocketInUseListHead;       /* list of HttpServerSocket objects: object is added to this list when it is created! */
    BLST_Q_HEAD( httpServerSocketRcvdRequestHead, BIP_HttpServerSocket ) httpServerSocketRcvdRequestHead; /* list of HttpServerSocket objects that have full Request Header: added when full Req is rcvd. _RecvRequest() uses 1st ServerSocket from this list! */
    BLST_Q_HEAD( httpStreamerListHead, BIP_HttpStreamer ) httpStreamerListHead;       /* list of HttpStreamer objects */

    BIP_HttpServerCreateSettings    createSettings;
    BIP_HttpServerSettings          settings;
    BIP_HttpServerStartSettings     startSettings;
    BIP_StringHandle                hPort;
    BIP_StringHandle                hIpAddress;
    BIP_StringHandle                hInterfaceName;

    BIP_HttpServerStartState        startState;                     /* httpServer State */
    BIP_Status                      completionStatus;
    B_MutexHandle                   hStateMutex;                    /* Mutex to synchronize a API invocation with callback invocation */

    BIP_DtcpIpServerHandle          hDtcpIpServer;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpServerSettings      *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpServerSettings      settings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpServerStartSettings settings;
    } startApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpRequestHandle    *phHttpRequest;
        BIP_HttpServerRecvRequestSettings settings;
    } recvRequestApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpRequestHandle    hHttpRequest;
        BIP_HttpServerRejectRequestSettings settings;
    } rejectRequestApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpStreamerHandle     *hHttpStreamer;
        BIP_HttpServerCreateStreamerSettings settings;
    } createStreamerApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpStreamerHandle      hHttpStreamer;
    } destroyStreamerApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpStreamerHandle      hHttpStreamer;
        BIP_HttpRequestHandle    hHttpRequest;
        BIP_HttpServerStartStreamerSettings settings;
    }startStreamerApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpStreamerHandle      hHttpStreamer;
    }stopStreamerApi;

    struct
    {
        unsigned                    numAcceptedConnections;     /* total connections accepted since _Start */
        unsigned                    numRequestTimeouts;         /* total connections which didn't receive full request within the keep-alive interval */
        unsigned                    numRcvdRequests;            /* total HTTP Requests received since start */
        unsigned                    numConcurrentRequestsQueued;/* total concurrent connections with HTTP Request pending RecvRequest from App */
        unsigned                    numSentResponses;           /* total HTTP Responses sent out since start */
        unsigned                    numRejectRequests;          /* total HTTP Requests Rejected sent out since start */
        unsigned                    numStartedStreamers;        /* total Streamers started since start  */
        unsigned                    numStoppedStreamers;        /* total Streamers stopped since start  */
    } stats;
} BIP_HttpServer;

#define BIP_HTTP_SERVER_PRINTF_FMT  \
    "[hHttpServer=%p Port=%s Iface=%s Type=%s MaxRqToQ=%d PersistentConnx=%s Timeout=%d hListener=%p]"

#define BIP_HTTP_SERVER_PRINTF_ARG(pObj)                                                             \
    (pObj),                                                                                          \
    (pObj)->startSettings.pPort          ? (pObj)->startSettings.pPort          : "NULL",            \
    (pObj)->startSettings.pInterfaceName ? (pObj)->startSettings.pInterfaceName : "NULL",            \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV4           ? "IpV4"          :  \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6           ? "IpV6"          :  \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_and_IpV4  ? "IpV6_and_IpV4" :  \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_over_IpV4 ? "IpV6_over_IpV4":  \
                                                                                  "<undefined>",     \
    (pObj)->startSettings.maxConcurrentRequestsToQueue,                                              \
    ((pObj)->startSettings.persistentConnectionTimeoutInMs > 0)  ? "Y" : "N",                        \
    (pObj)->startSettings.persistentConnectionTimeoutInMs,                                           \
    (pObj)->listener.hListener

#define BIP_HTTP_SERVER_STATS_PRINTF_FMT  \
    "[hHttpServer=%p connAccpt %u, queuedReq %u, rcvdReq %u, sentResp %u, rejectReq %u, startStrm %u, stopStrm %u, timeoutReq %u"

#define BIP_HTTP_SERVER_STATS_PRINTF_ARG(pObj)         \
    (pObj),                                            \
    (pObj)->stats.numAcceptedConnections,              \
    (pObj)->stats.numConcurrentRequestsQueued,         \
    (pObj)->stats.numRcvdRequests,                     \
    (pObj)->stats.numSentResponses,                    \
    (pObj)->stats.numRejectRequests,                   \
    (pObj)->stats.numStartedStreamers,                 \
    (pObj)->stats.numStoppedStreamers,                 \
    (pObj)->stats.numRequestTimeouts

void processHttpServerState( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket, int value, BIP_Arb_ThreadOrigin );


#endif /* BIP_HTTP_SERVER_IMPL_H */
