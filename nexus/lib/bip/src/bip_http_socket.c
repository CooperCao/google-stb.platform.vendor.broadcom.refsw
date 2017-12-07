/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/**
 * BIP_HttpSocket Class allows sending & receiving of HTTP Request and Response over a BIP_Socket.
 * Once Headers are exchanged, the class provides APIs to send & receive HTTP Payload (AV data) over this HTTP Socket.
 *
 * API Rules for BIP_HttpSocket object:
 * -Each BIP_HttpSocket API is serialized via the API lock. This lock is obtained at the start of each API and released when API is completed.
 *  This allows only one API call to be working on one object at a time. The lock is internally obtained by the BIP_Arb_Acquire().
 *
 *
 * -Both _Recv & _Send APIs (for Request, Response, and Payloads) allow callers to send & receive in blocking, blocking w/ timeout, non-blocking,
 *  async, and async w/ timeout modes.
 *  If the Async Recv API is in progress (where API started the Read state machine and has returned),
 *  no subsequent API (of that type Recv or Send) will be allowed and error of BIP_INF_IN_PROGRESS will be returned until Async Completion Callback is invoked.
 *
 *
 * -Callbacks can come from the BIP_Socket child class. To synchronize access between thread running API code & callbacks from child classes,
 *  httpState lock is used to protect the object state and serialize access.
 *
 *
 * -Caller can either enable the request/response/payload received callbacks via SetSettings API or not set it.
 *  Irrespective of callback settings, callers can invoke _Recv & _Send Request/Response APIs in blocking, blocking w/ timeout, non-blocking or Async way.
 *
 *  If caller has enabled requestReceived callback, this object keeps the dataReadyCallback from child BIP_Socket object enabled. It will then get the
 *  dataReadyCallback from BIP_Socket class and read what it can using non-blocking Read. Once a complete HTTP Message is received, then
 *  messageReceived Callback is invoked to the parent class.  However, before invoking the callback, object disables the future dataReady CB from BIP_Socket.
 *  This is done as we don't want to queue up further Request messages until first one received and processed by the app (whereby its response is completely sent out).
 *  NOTE: State Machine will enable this callback once it HttpState is back in the Idle state. <TODO: make sure this happens!>
 *
 *  If caller has not enabled messageReceived callback, this object doesn't enable the callbacks from BIP_Socket class and thus does not proactively receive 1st Req/Resp Message or payloads.
 *  Any messages or payload bytes from the peer stay queued up in the kernel socket receive queue. Only when caller issues a _RecvRequest/Response/Payload call, this class will then try a non-blocking read
 *  from BIP_Socket, and if a complete message/payload is not read, it will enable the callbacks from child BIP_Socket class until a complete message is received and then again disable
 *  BIP_Socket callbacks.
 *
 *  Class will also handle the case where caller may proactively issue a _Recv call (despite enabling callbacks to indicate request/response/payload availability!)
 *  before getting the callbacks or a _Recv API may be invoked while dataReadyCallback from BIP_Socket is in progress.
 *  In such cases, we will invoke the callbacks to caller as _Recv API is already in progress to consume the request/response/payload.
 *
 *
 *  -If BIP_HttpSocket object gets an errorCallback from BIP_Socket or gets an error during BIP_Socket_Send/_Recv API,
 *   it will invoke the errorCallback to the caller/class user.
 *   The caller would handle this error callback by destroying the HttpSocket object.
 *
 *
 *
 * HTTP States: Both Client & Server flows go two different phases in HTTP Processing:
 * -Message Header Processing (Request & Response)
 *
 *  -Server module uses BIP_HttpSocket's APIs to _RecvRequest Header (in blocking or async mode), then sends the Response Headers using the _SendResponse (in blocking or async mode).
 *   After sending the Response Headers, Server module will enter the Payload processing Phase if there is payload for this HTTP Request.
 *   Otherwise, for Request methods like HTTP HEAD, it can go back to Idle state to receive next Request. Caller uses noPayload flag in the StreamerStartSettings to indicate this no payload case.
 *   <TODO: need to see how should http object find out if it is responding to HEAD request? Should it note the request or find out from the noPayload flag?>
 *
 *  -Client Player module uses BIP_HttpSocket's APIs to _SendRequest Header (in blocking or async mode), then receives the Response Headers using the _RecvResponse (in blocking or async mode).
 *   After parsing the Response Headers, Client will enter the Payload processing Phase if there is payload in the response.
 *   Otherwise, for Request methods like HTTP HEAD, it can go back to eIdle state to allow caller to send next Request.
 *
 *
 * -Message Payload Processing
 *  -Server streamer module uses the _SendPayload API to send the requested number of payload bytes.
 *   While sending the Payload bytes, HttpSocket class will insert any HTTP specific headers (if enabled by the caller) into payload (such as HTTP Transfer Chunk encoding).
 *   Once HTTP Socket module has sent all data upto the content length of the payload (if indicated in the HTTP Response Header),
 *   then it will issue endOfStream callback, or errorCallback in case of network error.
 *   Caller can destroy the HTTP Socket when it is done sending the payload (or we can add _StopSendingPayload() API to just stop sending).
 *   Streamer can enable the writeReadyCallback to drive its state machine to issue the SendPayload call.
 *
 *  -Player uses the _RecvPayload API to receive requested number of payload bytes. While receiving the Payload bytes, it will remove any HTTP specific headers from the payload
 *   (if HTTP Transfer Chunk Encoding is present) before returning the payload.
 *   Once HTTP Socket module has received all data upto the content length of the payload (if indicated in the HTTP Response Header),
 *   then it will issue endOfStream callback, errorCallback in case of network error.
 *
 *   Player can drive its state machine in couple of ways:
 *   - for playing formats that are driven from outside caller (such Nexus Playback for MP4/ASF/Client side trickmode cases), BIP_HttpPlayer
 *      - will not enable any callbacks from BIP_HttpSocket class as Nexus's io thread will drive reading the AV payloads
 *      - call _Recv api in either non-blocking or blocking with a small timeout (such as 100msec) to read from BIP_Socket
 *   - for playing formats such as MPEG2 TS with server side trickmodes, HLS, MPEG DASH like protocols, BIP_HttpPlayer, BIP_HlsPlayer, BIP_MpegDashPlayer can:
 *      - Use callback from Nexus Playpump for buffer availability and get buffer for reading,
 *      - Call _Recv API in Async mode with timeout,
 *      - In Async completion callback, feed any available read buffer to Playpump, otherwise, enable payload available callback to receive data,
 *      - Continue in this loop
 *   - Get driven by the payloadReadyCallback from this object.
 *
 *
*/

#include "bip_http_socket_impl.h"

BDBG_MODULE( bip_http_socket );
BDBG_OBJECT_ID( BIP_HttpSocket );
BIP_SETTINGS_ID(BIP_HttpSocketCreateSettings);
BIP_SETTINGS_ID(BIP_HttpSocketRecvRequestSettings);
BIP_SETTINGS_ID(BIP_HttpSocketSendResponseSettings);
BIP_SETTINGS_ID(BIP_HttpSocketSendPayloadSettings);

/* Forward declaration of state processing function */
void processHttpState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin);
void processHttpStateFromTimerCallback_InitialRequest( void *pContext ) ;

void BIP_HttpSocket_GetDefaultSettings(
    BIP_HttpSocketSettings *pSettings
    )
{
    BKNI_Memset( pSettings, 0, sizeof( BIP_HttpSocketSettings ));
}

static void httpSocketDestroy(
    BIP_HttpSocketHandle hHttpSocket
    )
{
    if (!hHttpSocket) return;

    if (hHttpSocket->sendPayloadApi.hArb) BIP_Arb_Destroy(hHttpSocket->sendPayloadApi.hArb);
    if (hHttpSocket->sendResponseApi.hArb) BIP_Arb_Destroy(hHttpSocket->sendResponseApi.hArb);
    if (hHttpSocket->recvRequestApi.hArb) BIP_Arb_Destroy(hHttpSocket->recvRequestApi.hArb);
    if (hHttpSocket->setSettingsApi.hArb) BIP_Arb_Destroy(hHttpSocket->setSettingsApi.hArb);
    if (hHttpSocket->getSettingsApi.hArb) BIP_Arb_Destroy(hHttpSocket->getSettingsApi.hArb);
    if (hHttpSocket->destroyApi.hArb) BIP_Arb_Destroy(hHttpSocket->destroyApi.hArb);

    if (hHttpSocket->pNetworkReadBuffer) B_Os_Free( hHttpSocket->pNetworkReadBuffer );
    if (hHttpSocket->pNetworkWriteBuffer) B_Os_Free( hHttpSocket->pNetworkWriteBuffer );
    if (hHttpSocket->hHttpResponse && hHttpSocket->destroyResponse) BIP_HttpResponse_Destroy(hHttpSocket->hHttpResponse, hHttpSocket);
    if (hHttpSocket->hHttpRequest && hHttpSocket->destroyRequest) BIP_HttpRequest_Destroy(hHttpSocket->hHttpRequest, hHttpSocket);

    if (hHttpSocket->hStateMutex) B_Mutex_Destroy( hHttpSocket->hStateMutex );

    if (hHttpSocket->hSocket) BIP_Socket_Destroy( hHttpSocket->hSocket );

    if ( hHttpSocket->hShutdownEvent ) B_Event_Destroy( hHttpSocket->hShutdownEvent );
    BDBG_OBJECT_DESTROY( hHttpSocket, BIP_HttpSocket );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
    B_Os_Free( hHttpSocket );
} /* httpSocketDestroy */

BIP_HttpSocketHandle BIP_HttpSocket_CreateFromBipSocket(
    BIP_SocketHandle hSocket,
    const BIP_HttpSocketCreateSettings *pCreateSettings
    )
{
    BIP_Status                      brc;
    BIP_HttpSocketHandle            hHttpSocket = NULL;
    BIP_HttpSocketCreateSettings    defaultSettings;

    BIP_CHECK_GOTO(( hSocket), ( "hSocket is NULL: Must provide valid BIP_SocketHandle" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BDBG_ASSERT( hSocket );
    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_HttpSocketCreateSettings);

    /* Create the httpSocket object */
    hHttpSocket = B_Os_Calloc( 1, sizeof( BIP_HttpSocket ));
    BIP_CHECK_GOTO(( hHttpSocket != NULL ), ( "Failed to allocate memory (%zu bytes) for HttpSocket Object", sizeof(BIP_HttpSocket) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_OBJECT_SET( hHttpSocket, BIP_HttpSocket );
    hHttpSocket->hSocket = hSocket;

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_Socket or timer) & Caller calling APIs. */
    hHttpSocket->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hHttpSocket->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    if (NULL == pCreateSettings)
    {
        BIP_HttpSocket_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hHttpSocket->createSettings = *pCreateSettings;

    hHttpSocket->recv.persistentConnectionTimeoutInMs = hHttpSocket->createSettings.persistentConnectionTimeoutInMs;
    if (pCreateSettings->hHttpRequest == NULL)
    {
        /* caller didn't provide HttpRequest object, so lets create one */
        hHttpSocket->hHttpRequest = BIP_HttpRequest_Create( hHttpSocket, NULL);
        BIP_CHECK_GOTO(( hHttpSocket->hHttpRequest ), ( "BIP_HttpRequest_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
        hHttpSocket->destroyRequest = true;
    }

    /* allocate pNetworkReadBuffer to hold incoming request message.*/
    {
        hHttpSocket->pNetworkReadBuffer = B_Os_Calloc( 1, hHttpSocket->createSettings.networkReadBufferSize);
        BIP_CHECK_GOTO(( hHttpSocket->pNetworkReadBuffer != NULL ), ( "Failed to allocate memory (%d bytes) for Http Message Buffer", hHttpSocket->createSettings.networkReadBufferSize ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
    }
    /* allocate pNetworkWriteBuffer to hold outgoing response message.*/
    {
        hHttpSocket->pNetworkWriteBuffer = B_Os_Calloc( 1, hHttpSocket->createSettings.networkWriteBufferSize);
        BIP_CHECK_GOTO(( hHttpSocket->pNetworkWriteBuffer != NULL ), ( "Failed to allocate memory (%d bytes) for Http Message Buffer", hHttpSocket->createSettings.networkWriteBufferSize ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
    }

    if (pCreateSettings->hHttpResponse == NULL)
    {
        /* caller didn't provide HttpResponse object, so lets create one */
        hHttpSocket->hHttpResponse = BIP_HttpResponse_Create(hHttpSocket, NULL);
        BIP_CHECK_GOTO(( hHttpSocket->hHttpResponse ), ( "BIP_HttpResponse_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
        hHttpSocket->destroyResponse = true;
    }

#ifdef TODO /* Response class is not yet ready */
    /* TODO: Check with Sanjeev , this should not be required anymore.*/
    /* determine the size of the response buffer: we need to know the size of partialMsgBuffer */
    {
        char *pResponseBuffer;
        size_t responseBufferSize;
        brc = BIP_HttpResponse_GetWriteBuffer(hHttpSocket->hHttpRequest, &pResponseBuffer, &responseBufferSize);
        BIP_CHECK_GOTO(( brc == BIP_SUCCESS && pResponseBuffer && responseBufferSize ), ( "BIP_HttpResponse_GetWriteBuffer Failed"), error, BIP_ERR_INTERNAL, brc );
        if (responseBufferSize > hHttpSocket->partialMsgBufferSize)
            /* use bigger of the request & response buffers for the holding the partial msg bytes */
            hHttpSocket->partialMsgBufferSize = responseBufferSize;
    }
#endif

    BIP_HttpSocket_GetDefaultSettings(&hHttpSocket->settings);

    /* Create API ARBs: one per API */
    hHttpSocket->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->recvRequestApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->recvRequestApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->sendResponseApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->sendResponseApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->sendPayloadApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->sendPayloadApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpSocket->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->hShutdownEvent = B_Event_Create(NULL);
    BIP_CHECK_GOTO(( hHttpSocket->hShutdownEvent ), ( "B_Event_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpSocket->httpState = BIP_HttpSocketState_eIdle;

    /* Start a timer to monitor the arrival of initial request. If it doesn't arrive in this interval, HttpSocket is marked in the error state. */
    {
        BIP_TimerCreateSettings timerCreateSettings;

        hHttpSocket->recv.initialRequestTimeoutInMs = BIP_HTTP_SOCKET_INITIAL_REQUEST_TIMEOUT_IN_MS;
        timerCreateSettings.input.callback          = processHttpStateFromTimerCallback_InitialRequest;
        timerCreateSettings.input.pContext          = hHttpSocket;
        timerCreateSettings.input.timeoutInMs       = hHttpSocket->recv.initialRequestTimeoutInMs;
        hHttpSocket->recv.hInitialRequestTimer          = BIP_Timer_Create(&timerCreateSettings);
        BIP_CHECK_GOTO(( hHttpSocket->recv.hInitialRequestTimer ), ( "BIP_Timer_Create Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
        hHttpSocket->recv.initialRequestTimerActive = true;
        B_Time_Get(&hHttpSocket->recv.initialRequestStartTime);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created hHttpSocket %p: state %d" BIP_MSG_PRE_ARG, (void *)hHttpSocket, hHttpSocket->httpState));

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created from BipSocket: " BIP_HTTP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SOCKET_PRINTF_ARG(hHttpSocket)));

    /* Now run the state machine. */
    return ( hHttpSocket );

error:
    httpSocketDestroy( hHttpSocket );
    return ( NULL );
} /* BIP_HttpSocket_CreateFromBipSocket */

/**
 * Summary:
 * Destroy http socket
 *
 * Description:
 **/
void BIP_HttpSocket_Destroy(
    BIP_HttpSocketHandle hHttpSocket
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SOCKET_PRINTF_ARG(hHttpSocket)));
    BDBG_MSG((    BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SOCKET_PRINTF_ARG(hHttpSocket)));

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpSocket->destroyApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;

    B_Event_Reset( hHttpSocket->hShutdownEvent );

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, brc, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Waiting for HttpSocket Shutdown Event Completion!" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    brc = B_Event_Wait( hHttpSocket->hShutdownEvent, B_WAIT_FOREVER );
    BIP_CHECK_LOGERR((brc == B_ERROR_SUCCESS), ( "B_Event_Wait failed, rc:0x%X", brc ), BIP_ERR_B_OS_LIB, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Received HttpSocket Shutdown Event, so start destroying the object" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, brc ));
    httpSocketDestroy( hHttpSocket );
    return;

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: completionStatus 0x%x <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, brc ));
    return;
} /* BIP_HttpSocket_Destroy */

void BIP_HttpSocket_GetSettings(
        BIP_HttpSocketHandle    hHttpSocket,
        BIP_HttpSocketSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpSocket));

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpSocket->getSettingsApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpSocket->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p (bipStatus: %s): <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_StatusGetText(brc)));

    return;
}

BIP_Status BIP_HttpSocket_SetSettings(
    BIP_HttpSocketHandle    hHttpSocket,
    BIP_HttpSocketSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_ASSERT( pSettings );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpSocket));

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpSocket->setSettingsApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpSocket->setSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: completionStatus %s <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_StatusGetText(brc) ));

    return( brc );
} /* BIP_HttpSocket_SetSettings */

/**
 * Summary:
 * HTTP Socket API to Receive a HTTP Request Message Header.
 **/
BIP_Status BIP_HttpSocket_RecvRequest(
    BIP_HttpSocketHandle                hHttpSocket,
    BIP_HttpRequestHandle            *phHttpRequest,
    BIP_HttpResponseHandle           *phHttpResponse,
    BIP_HttpSocketRecvRequestSettings   *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_HttpSocketRecvRequestSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_ASSERT( pSettings );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpSocketRecvRequestSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpSocket));
    BIP_CHECK_GOTO(( phHttpRequest ), ( "phHttpRequest can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( phHttpResponse ), ( "phHttpResponse can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    if ( pSettings == NULL )
    {
        BIP_HttpSocket_GetDefaultRecvRequestSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    /* Serialize access to RecvRequest state among another thread calling the same API. */
    hArb = hHttpSocket->recvRequestApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpSocket->recvRequestApi.settings = *pSettings;
    hHttpSocket->recvRequestApi.phHttpRequest = phHttpRequest;
    hHttpSocket->recvRequestApi.phHttpResponse = phHttpResponse;
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    apiSettings.timeout = pSettings->timeoutInMs;
    apiSettings.asyncCallback = pSettings->asyncCallback;
    apiSettings.pAsyncStatus = pSettings->pAsyncStatus;
    brc = BIP_Arb_Submit(hArb, &arbSettings, &apiSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p (bipStatus: %s): <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_StatusGetText(brc)));

error:
    return( brc );
}

/**
 * Summary:
 * HTTP Socket API to send HTTP Response Message on HTTP Socket
 **/
BIP_Status BIP_HttpSocket_SendResponse(
    BIP_HttpSocketHandle                hHttpSocket,
    BIP_HttpRequestHandle            hHttpRequest,
    BIP_HttpResponseHandle           hHttpResponse,
    int64_t                             messageLength,
    BIP_HttpSocketSendResponseSettings  *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpSocketSendResponseSettings defaultSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_ASSERT( pSettings );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpSocketSendResponseSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpSocket));

    BIP_CHECK_GOTO(( hHttpRequest ), ( "hHttpRequest can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( hHttpResponse ), ( "hHttpResponse can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    if ( pSettings == NULL )
    {
        BIP_HttpSocket_GetDefaultSendResponseSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }

    /* Serialize access to SednResponse state among another thread calling the same API. */
    hArb = hHttpSocket->sendResponseApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpSocket->sendResponseApi.settings = *pSettings;
    hHttpSocket->sendResponseApi.hHttpRequest = hHttpRequest;
    hHttpSocket->sendResponseApi.hHttpResponse = hHttpResponse;
    hHttpSocket->sendResponseApi.messageLength = messageLength;
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    apiSettings.timeout = pSettings->timeoutInMs;
    apiSettings.asyncCallback = pSettings->asyncCallback;
    apiSettings.pAsyncStatus = pSettings->pAsyncStatus;
    brc = BIP_Arb_Submit(hArb, &arbSettings, &apiSettings);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS || brc == BIP_INF_IN_PROGRESS),
                    ( "BIP_Arb_SubmitRequest() Failed" ), error, brc, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p (bipStatus: %s): <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_StatusGetText(brc)));
error:
    return( brc );
}

/**
 * Summary:
 * HTTP Socket API to Send HTTP Payload on HTTP Socket
 **/
BIP_Status BIP_HttpSocket_SendPayload(
    BIP_HttpSocketHandle hHttpSocket,
    const uint8_t *pBuffer,
    size_t bytesToSend,
    BIP_HttpSocketSendPayloadSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpSocketSendPayloadSettings defaultSettings;
    BIP_ApiSettings apiSettings;

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_ASSERT( pSettings );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpSocketSendPayloadSettings);

    BIP_CHECK_GOTO(( hHttpSocket ), ( "hHttpSocket can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( pBuffer ), ( "pBuffer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( bytesToSend > 0 ), ( "bytesToSend (%zu) must be > 0 ", bytesToSend ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpSocket));

    if ( pSettings == NULL )
    {
        BIP_HttpSocket_GetDefaultSendPayloadSettings( &defaultSettings );
        pSettings = &defaultSettings;
    }
    /* Serialize access to SednResponse state among another thread calling the same API. */
    hArb = hHttpSocket->sendPayloadApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpSocket->sendPayloadApi.pBuffer = pBuffer;
    hHttpSocket->sendPayloadApi.bytesToSend = bytesToSend;
    hHttpSocket->sendPayloadApi.settings = *pSettings;
    arbSettings.hObject = hHttpSocket;
    arbSettings.arbProcessor = processHttpState;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    apiSettings.timeout = pSettings->timeoutInMs;
    apiSettings.asyncCallback = pSettings->asyncCallback;
    apiSettings.pAsyncStatus = pSettings->pAsyncStatus;
    brc = BIP_Arb_Submit( hArb, &arbSettings, &apiSettings );
    BIP_CHECK_GOTO((brc == BIP_SUCCESS || brc == BIP_INF_IN_PROGRESS),
                  ( "BIP_Arb_SubmitRequest() Failed" ), error, brc, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p (bipStatus: %s): <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_StatusGetText(brc)));

error:
    return( brc );
}

BIP_Status BIP_HttpSocket_GetStatus(
    BIP_HttpSocketHandle hHttpSocket,
    BIP_HttpSocketStatus *pHttpSocketStatus
    )
{
    BIP_Status bipStatus;
    BIP_SocketStatus socketStatus;

    bipStatus = BIP_Socket_GetStatus( hHttpSocket->hSocket, &socketStatus);
    if (bipStatus == BIP_SUCCESS)
    {
        pHttpSocketStatus->socketFd = socketStatus.socketFd;
        pHttpSocketStatus->pLocalIpAddress = socketStatus.pLocalIpAddress;
        pHttpSocketStatus->pRemoteIpAddress = socketStatus.pRemoteIpAddress;
    }
    return (BIP_SUCCESS);
}
