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

#ifndef BIP_HTTP_SOCKET_IMPL_H
#define BIP_HTTP_SOCKET_IMPL_H

#include "bip_arb.h"
#include "bip_priv.h"

/* HTTP States for Sending Request, Receiving Request, Sending Response, Receiveing Response, Sending Payload & Receiving Payloads.  */
typedef enum BIP_HttpSocketState
{
    /* States for Receiving Req Header, Sending Resp followed by optional Response Payload */
    BIP_HttpSocketState_eUninitialized,         /* Initial state: object is just created but fully initialzed */
    BIP_HttpSocketState_eIdle,                  /* Idle state: no Request has been Received or Sent out */
    BIP_HttpSocketState_eReceiveNewRequest,     /* transitional state: Prepare to Read a Request Header: initiated via _RecvRequest, _RecvResponse, or _SetSettings in eIdle state */
    BIP_HttpSocketState_eReceivingRequest,      /* Receiving a New Request and haven't yet received a complete Request Message header */
    BIP_HttpSocketState_eReceivingRequestDone,  /* Just finished receiving a full request and now need to take some actions before transitioning to next state */
    BIP_HttpSocketState_eReceivedRequest,       /* requestReceivedCallback has been invoked about New Request and now waiting for caller to call _RecvRequest */
    BIP_HttpSocketState_eReceiveRequestTimedout,/* Timedout while receiving request */
    BIP_HttpSocketState_eReadyToSendResponse,   /* Caller has issued _RecvRequest to receive the Request and we are now waiting for _SendResponse */
    BIP_HttpSocketState_eSendNewResponse,       /* Caller has issued _SendResponse to send the Response and we are getting ready to initiate sending sequence! */
    BIP_HttpSocketState_eSendingResponse,       /* App has called _SendResponse and we are now sending the Response Header */
    BIP_HttpSocketState_eSendingResponseDone,   /* Just finished sending a full response header. */
    BIP_HttpSocketState_eSentResponse,          /* We are done sending the Response header and now waiting for App to _SendPayload */
    BIP_HttpSocketState_eReadyToSendPayload=    /* same state as eSentResponse */
        BIP_HttpSocketState_eSentResponse,
                                                /* If there is no payload to send for this HTTP Response, then we transition to Idle state to receive next request */
    BIP_HttpSocketState_eSendResponseTimedout,  /* Timedout while trying to send the response */
    BIP_HttpSocketState_eSendNewPayload,        /* App has called _SendPayload and we are now setting up to send the Payload bytes */
    BIP_HttpSocketState_eSendingPayload,        /* We are currently sending the Payload bytes */
    BIP_HttpSocketState_eSendingPayloadDone,    /* Just finished sending the payload. */
    BIP_HttpSocketState_eSentPayload,           /* Payload has been sent */
    BIP_HttpSocketState_eSendPayloadTimedout,   /* Timedout while trying to send the payload */

    /* States for Sending Request Header, Receiving Response followed by optional Response Payload */
    BIP_HttpSocketState_eSendingRequest,        /* App has called _SendRequest and we are sending the Request Header and are not yet done sending it */
    BIP_HttpSocketState_eSentRequest,           /* We have sent Request header and now are waiting for Response Message Header to arrive from Server */
    BIP_HttpSocketState_eReceiveNewResponse,           /* transitional state: Prepare to Read a Response: initiated via _RecvResponse, _RecvResponseAync, or _SetSettings */
    BIP_HttpSocketState_eReceivingResponse,     /* App has called _RecvResponse and we are now receiving the response */
    BIP_HttpSocketState_eReceivedResponse,      /* responseReceivedCallback has been invoked about the new Response and now waiting for caller to call _RecvResponse */
                                                /* If there is no payload to receive for this HTTP Response, then we transition to Idle state to send next request */
    BIP_HttpSocketState_eReadyToReceivePayload= /* same as ReceivedResponse */
        BIP_HttpSocketState_eReceivedResponse,
    BIP_HttpSocketState_eReceiveNewPayload,     /* App has called _RecvPayload and we are now receiving the payload bytes */
    BIP_HttpSocketState_eReceivingPayload,      /* App has called _RecvPayload and we are now receiving the payload bytes */
    BIP_HttpSocketState_eReceivedPayload,       /* transitional state: we have finished reading all payload bytes but haven't yet taken the completion action */
                                                /* issued the async completion callback or woken up the blocked _RecvPayload thread */
                                                /* and will either goto eIdle state if we have(received contentLength bytes or back to RespRcvd to receive more payload bytes */
                                                /* Completion action: wakeup a blocked _RecvPayload thread, || issue completion callback for _RecvPayloadAsync */
                                                /* After this action, we will either goto eIdle state if we have received contentLength bytes or back to _eReadyToRecvPayload to send more payload bytes */
    BIP_HttpSocketState_eError,                 /* Got Error due to peer closing or network error, happens when either errorCallback comes or BIP_SocketRecv or _SocketSend fails. */
    BIP_HttpSocketState_eDestroying,            /* App has called _Destroy API */
    BIP_HttpSocketState_eMax
} BIP_HttpSocketState;

/**
 * BIP_Socket Callback States: has two states:
 *  disabled    no callback specified by caller || we have received one msg/payload and now waiting for app to _Recv it.
 *  enabled     callback specified by caller || a blocking _Recv call is waiting for completion.
 **/
typedef enum BIP_SocketCallbackState
{
    BIP_SocketCallbackState_eDisabled,
    BIP_SocketCallbackState_eEnabled
} BIP_SocketCallbackState;

/* States to keep track of when we should send callbacks to our client. */
typedef enum
{
    BIP_HttpSocketConsumerCallbackState_eDisabled,               /* RequesteReceived callback is not enabled by app. */
    BIP_HttpSocketConsumerCallbackState_eArmed,                  /* Callback enabled, fire callback when request is received. */
    BIP_HttpSocketConsumerCallbackState_eTriggered,              /* Callback has fired, don't re-arm until request is read. */
    BIP_HttpSocketConsumerCallbackState_eMax                     /* eMax */
} BIP_HttpSocketConsumerCallbackState;

/* States used for shutting down HttpSocket object prior to its object destruction. */
typedef enum
{
    BIP_HttpSocketShutdownState_eNormal,                         /* Normal existing (not destroy) state.          */
    BIP_HttpSocketShutdownState_eStartShutdown,                  /* Starting shutting down... No further APIs accepted. */
    BIP_HttpSocketShutdownState_eFinishingArbs,                  /* Waiting for busy arbs to finish                */
    BIP_HttpSocketShutdownState_eShutdownDone,                   /* Object is shut down and ready to be destroyed  */
    BIP_HttpSocketShutdownState_eMax                             /* eMax: last enum.                               */
} BIP_HttpSocketShutdownState;

typedef struct BIP_HttpSocket
{
    BDBG_OBJECT( BIP_HttpSocket )
    BLST_Q_ENTRY( BIP_HttpSocket )  httpSocketListNext;         /* next BIP_HttpSocket object: list owned by HttpServer */
    BLST_Q_ENTRY( BIP_HttpSocket )  inUseListNext;              /* next BIP_HttpSocket object: list owned by HttpServer */

    BIP_HttpSocketCreateSettings    createSettings;
    BIP_HttpSocketSettings          settings;

    BIP_HttpSocketState             httpState;                  /* httpSocket State */
    B_MutexHandle                   hStateMutex;                /* Mutex to synchronize a API invocation with callback invocation */

    BIP_HttpServerSocketHandle      hServerSocket;              /* ServerSocket Handle: container object inside BIP_HttpServer Interface, one per BIP_HttpSocket */
    BIP_SocketHandle                hSocket;                    /* BIP_Socket handle associated this this object */
    bool                            destroyRequest;             /* Set to true if BIP_HttpRequestHandle is internaly created */
    BIP_HttpRequestHandle           hHttpRequest;               /* HttpRequest object handle (incoming or outgoing Request) */
    bool                            destroyResponse;            /* Set to true if BIP_HttpResponseHandle is internaly created */
    BIP_HttpResponseHandle          hHttpResponse;              /* HttpResponse object handle (incoming or outgoing Response) */

    char                            *pNetworkReadBuffer;        /* buffer to read incoming request message from network */
    int                             nwRdBuffOffset;             /* nwRdBuffOffset for unconsumed data which is not consumed by HttpRequest object since it is not part of current HttpRequest */
    size_t                          unconsumedDataSize;         /* unconsumed data size */

    char                            *pNetworkWriteBuffer;       /* buffer to write outgoing response message to network */

    bool                            timerActive;                /* true if timer is currently active */
                                                                /* Or can also hold a partial message bytes where _RecvRquest/Response API timed out before full message was received */
    BIP_SocketCallbackState         dataReadyCallbackState;     /* state of BIP_Socket's dataReadyCallback */
    BIP_SocketCallbackState         errorCallbackState;         /* state of BIP_Socket's errorCallback */
    BIP_SocketCallbackState         writeReadyCallbackState;    /* state of BIP_Socket's writeReadyCallback */
    BIP_HttpSocketConsumerCallbackState     requestReceivedCallbackState;
    BIP_HttpSocketShutdownState     shutdownState;
    B_EventHandle                   hShutdownEvent;             /* this event is set when state machine has finished processing the destroy Arb related work. */

    bool                            disablePersistentConnection;
    struct
    {
        BIP_Status                  completionStatus;

        BIP_TimerHandle             hApiTimer;
        B_Time                      apiStartTime;
        bool                        apiTimerActive;             /* true if timer is currently active */

#define BIP_HTTP_SOCKET_INITIAL_REQUEST_TIMEOUT_IN_MS   5000
        BIP_TimerHandle             hInitialRequestTimer;
        B_Time                      initialRequestStartTime;
        unsigned                    initialRequestTimeoutInMs;
        bool                        initialRequestTimerActive;  /* true if timer is currently active */

        BIP_TimerHandle             hPersistentConnectionTimer;
        B_Time                      persistentConnectionStartTime;
        unsigned                    persistentConnectionTimeoutInMs;
        bool                        persistentConnectionTimerActive;  /* true if timer is currently active */
    } recv;

    struct
    {
        BIP_Status                  completionStatus;
        BIP_TimerHandle             hTimer;
        B_Time                      startTime;
        bool                        timerActive;                /* true if timer is currently active */
        size_t                      payloadBytesSent;           /* how many bytes are sent in the PayloadSending state. */
                                                                /* reset to 0 in SendNewPayload state. */
    } send;

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpSocketSettings      *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpSocketSettings      *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpRequestHandle    *phHttpRequest;
        BIP_HttpResponseHandle   *phHttpResponse;
        BIP_HttpSocketRecvRequestSettings settings;
    } recvRequestApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_HttpRequestHandle    hHttpRequest;
        BIP_HttpResponseHandle   hHttpResponse;
        int64_t                     messageLength;
        BIP_HttpSocketSendResponseSettings settings;
    } sendResponseApi;
    struct
    {
        BIP_ArbHandle               hArb;
        const uint8_t               *pBuffer;
        size_t                      bytesToSend;
        BIP_HttpSocketSendPayloadSettings settings;
    } sendPayloadApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;
    struct
    {
        uint64_t                    numBytesSent;            /* total bytes sent. */
        uint64_t                    numBytesReceived;        /* total bytes received. */
        unsigned                    numRequestRecevied;      /* total request received. */
        unsigned                    numRequestSent;          /* total request send. */
        unsigned                    numResponseRecevied;     /* total response received. */
        unsigned                    numResponseSent;         /* total response sent. */
    } stats;
} BIP_HttpSocket;


#define BIP_HTTP_SOCKET_PRINTF_FMT  \
    "[hHttpSocket=%p hBipSocket=%p hHttpReq=%p hHttpResp=%p iniReqTimeout=%d reqRcvd=%ld respSent=%ld reqSent=%ld respRcvd=%ld bytesSent=%lld bytesRcvd=%lld]"

#define BIP_HTTP_SOCKET_PRINTF_ARG(pObj)    \
    (pObj),                                 \
    (pObj)->hSocket,                        \
    (pObj)->hHttpRequest,                   \
    (pObj)->hHttpResponse,                  \
    (pObj)->recv.initialRequestTimeoutInMs, \
    (pObj)->stats.numRequestRecevied,       \
    (pObj)->stats.numResponseSent,          \
    (pObj)->stats.numRequestSent,           \
    (pObj)->stats.numResponseRecevied,      \
    (pObj)->stats.numBytesSent,             \
    (pObj)->stats.numBytesReceived          \

#endif /* BIP_HTTP_SOCKET_IMPL_H */
