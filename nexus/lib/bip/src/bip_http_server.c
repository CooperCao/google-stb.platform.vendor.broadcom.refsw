/******************************************************************************
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
 *****************************************************************************/

/**
 * BIP_HttpServer Class allows sending & receiving of HTTP Request and Response over a BIP_Socket.
 * Once Headers are exchanged, the class provides APIs to send & receive HTTP Payload (AV data) over this HTTP Socket.
 *
 * API Rules for BIP_HttpServer object:
 * -Each BIP_HttpServer API is serialized via the API lock. This lock is obtained at the start of each API and released when API is completed.
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
 *  NOTE: State Machine will enable this callback once it HttpState is back in the Idle state.
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
 *  -If BIP_HttpServer object gets an errorCallback from BIP_Socket or gets an error during BIP_Socket_Send/_Recv API,
 *   it will invoke the errorCallback to the caller/class user.
 *   The caller would handle this error callback by destroying the HttpServer object.
 *
 *
 *
 * HTTP States: Both Client & Server flows go two different phases in HTTP Processing:
 * -Message Header Processing (Request & Response)
 *
 *  -Server module uses BIP_HttpServer's APIs to _RecvRequest Header (in blocking or async mode), then sends the Response Headers using the _SendResponse (in blocking or async mode).
 *   After sending the Response Headers, Server module will enter the Payload processing Phase if there is payload for this HTTP Request.
 *   Otherwise, for Request methods like HTTP HEAD, it can go back to Idle state to receive next Request. Caller uses noPayload flag in the StreamerStartSettings to indicate this no payload case.
 *
 *  -Client Player module uses BIP_HttpServer's APIs to _SendRequest Header (in blocking or async mode), then receives the Response Headers using the _RecvResponse (in blocking or async mode).
 *   After parsing the Response Headers, Client will enter the Payload processing Phase if there is payload in the response.
 *   Otherwise, for Request methods like HTTP HEAD, it can go back to eIdle state to allow caller to send next Request.
 *
 *
 * -Message Payload Processing
 *  -Server streamer module uses the _SendPayload API to send the requested number of payload bytes.
 *   While sending the Payload bytes, HttpServer class will insert any HTTP specific headers (if enabled by the caller) into payload (such as HTTP Transfer Chunk encoding).
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
 *      - will not enable any callbacks from BIP_HttpServer class as Nexus's io thread will drive reading the AV payloads
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

#include "bip_http_server_impl.h"

BDBG_MODULE( bip_http_server );
BDBG_OBJECT_ID( BIP_HttpServer );

BIP_CLASS_DECLARE(BIP_HttpServer);

BIP_SETTINGS_ID(BIP_HttpServerCreateSettings);
BIP_SETTINGS_ID(BIP_HttpServerSettings);
BIP_SETTINGS_ID(BIP_HttpServerStartSettings);
BIP_SETTINGS_ID(BIP_HttpServerRecvRequestSettings);
BIP_SETTINGS_ID(BIP_HttpServerRejectRequestSettings);
BIP_SETTINGS_ID(BIP_HttpServerCreateStreamerSettings);
BIP_SETTINGS_ID(BIP_HttpServerStartStreamerSettings);

/* Forward declaration of state processing function */
void processHttpServerStateFromArb( void *hObject, int value, BIP_Arb_ThreadOrigin );
void BIP_HttpServerSocket_Destroy( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket);


void BIP_HttpServer_GetDefaultCreateSettings(
    BIP_HttpServerCreateSettings *pSettings
    )
{
    B_Os_Memset( pSettings, 0, sizeof( BIP_HttpServerCreateSettings ));
    BIP_SETTINGS_SET(pSettings,BIP_HttpServerCreateSettings);
}

void BIP_HttpServer_GetDefaultSettings(
    BIP_HttpServerSettings *pSettings
    )
{
    B_Os_Memset( pSettings, 0, sizeof( BIP_HttpServerSettings ));
    BIP_SETTINGS_SET(pSettings,BIP_HttpServerSettings);
}

static void httpServerDestroy(
    BIP_HttpServerHandle hHttpServer
    )
{
    if (!hHttpServer) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hHttpServer %p" BIP_MSG_PRE_ARG, (void *)hHttpServer ));

    /* Remove this httpServer instance from it's class. */
    BIP_CLASS_REMOVE_INSTANCE(BIP_HttpServer, hHttpServer);

    if (hHttpServer->recvRequestApi.hArb) BIP_Arb_Destroy(hHttpServer->recvRequestApi.hArb);
    if (hHttpServer->rejectRequestApi.hArb) BIP_Arb_Destroy(hHttpServer->rejectRequestApi.hArb);
    if (hHttpServer->setSettingsApi.hArb) BIP_Arb_Destroy(hHttpServer->setSettingsApi.hArb);
    if (hHttpServer->getSettingsApi.hArb) BIP_Arb_Destroy(hHttpServer->getSettingsApi.hArb);
    if (hHttpServer->startApi.hArb) BIP_Arb_Destroy(hHttpServer->startApi.hArb);
    if (hHttpServer->stopApi.hArb) BIP_Arb_Destroy(hHttpServer->stopApi.hArb);
    if (hHttpServer->destroyApi.hArb) BIP_Arb_Destroy(hHttpServer->destroyApi.hArb);
    if (hHttpServer->createStreamerApi.hArb) BIP_Arb_Destroy(hHttpServer->createStreamerApi.hArb);
    if (hHttpServer->destroyStreamerApi.hArb) BIP_Arb_Destroy(hHttpServer->destroyStreamerApi.hArb);
    if (hHttpServer->startStreamerApi.hArb) BIP_Arb_Destroy(hHttpServer->startStreamerApi.hArb);
    if (hHttpServer->stopStreamerApi.hArb) BIP_Arb_Destroy(hHttpServer->stopStreamerApi.hArb);

    if (hHttpServer->hStateMutex) B_Mutex_Destroy( hHttpServer->hStateMutex );

    if (hHttpServer->listener.hListener) BIP_Listener_Destroy( hHttpServer->listener.hListener );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Destroyed" BIP_MSG_PRE_ARG, (void *)hHttpServer ));
    BDBG_OBJECT_DESTROY( hHttpServer, BIP_HttpServer );
    B_Os_Free( hHttpServer );

} /* httpServerDestroy */

BIP_HttpServerHandle BIP_HttpServer_Create(
    const BIP_HttpServerCreateSettings *pCreateSettings
    )
{
    BIP_Status                      brc;
    BIP_HttpServerHandle            hHttpServer = NULL;
    BIP_HttpServerCreateSettings    defaultSettings;
    BIP_ListenerCreateSettings      listenerCreateSettings;

    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_HttpServerCreateSettings);

    /* Create the httpServer object */
    hHttpServer = B_Os_Calloc( 1, sizeof( BIP_HttpServer ));
    BIP_CHECK_GOTO(( hHttpServer != NULL ), ( "Failed to allocate memory (%zu bytes) for HttpServer Object", sizeof(BIP_HttpServer) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_OBJECT_SET( hHttpServer, BIP_HttpServer );

    brc = BIP_CLASS_ADD_INSTANCE(BIP_HttpServer, hHttpServer);
    BIP_CHECK_GOTO((brc==BIP_SUCCESS), ( "BIP_CLASS_ADD_INSTANCE() Failed " ), error, brc, brc );

    /* Create mutex to synchronize state machine from being run via callbacks (BIP_Socket or timer) & Caller calling APIs. */
    hHttpServer->hStateMutex = B_Mutex_Create(NULL);
    BIP_CHECK_GOTO(( hHttpServer->hStateMutex ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    if (NULL == pCreateSettings)
    {
        BIP_HttpServer_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hHttpServer->createSettings = *pCreateSettings;

    BIP_HttpServer_GetDefaultSettings(&hHttpServer->settings);

    /* Create API ARBs: one per API */
    hHttpServer->getSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->getSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->setSettingsApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->setSettingsApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->startApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->startApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->stopApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->stopApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->recvRequestApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->recvRequestApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->rejectRequestApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->rejectRequestApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->createStreamerApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->createStreamerApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->destroyStreamerApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->destroyStreamerApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->startStreamerApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->startStreamerApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->stopStreamerApi.hArb = BIP_Arb_Create(NULL, NULL);
    BIP_CHECK_GOTO(( hHttpServer->stopStreamerApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Created ARBs" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    /* Now Create the Listener Object */
    BIP_Listener_GetDefaultCreateSettings( &listenerCreateSettings );
    hHttpServer->listener.hListener = BIP_Listener_Create ( &listenerCreateSettings );
    BIP_CHECK_GOTO(( hHttpServer->listener.hListener != NULL ), ( "BIP_Listener_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    hHttpServer->startState = BIP_HttpServerStartState_eIdle;
    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_HTTP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_HTTP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));

    return ( hHttpServer );

error:
    httpServerDestroy( hHttpServer );
    return ( NULL );
} /* BIP_HttpServer_Create */

/**
 * Summary:
 * Destroy HTTP Server
 *
 * Description:
 **/
void BIP_HttpServer_Destroy(
    BIP_HttpServerHandle hHttpServer
    )
{
    BIP_Status brc;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_SERVER_PRINTF_FMT
                      BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));

    /* We go thru the state machine to destroy any HttpStreamer & HttpServerSocket objects */
    /* which were not Destroyed by the App via the BIP_HttpServer_DestroyStreamer(). */

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->destroyApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_Destroy" ), error, brc, brc );

error:
    /* Now free the HttpServer's resources. */
    httpServerDestroy( hHttpServer );

} /* BIP_HttpServer_Destroy */

void BIP_HttpServer_GetSettings(
        BIP_HttpServerHandle    hHttpServer,
        BIP_HttpServerSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _GetSettings API. */
    hArb = hHttpServer->getSettingsApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->getSettingsApi.pSettings = pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_GetSettings" ), error, brc, brc );

    BIP_SETTINGS_SET(pSettings,BIP_HttpServerSettings);

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return;
}

BIP_Status BIP_HttpServer_SetSettings(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpServerSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BDBG_ASSERT( pSettings );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->setSettingsApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->setSettingsApi.settings = *pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_SetSettings" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return ( brc );
} /* BIP_HttpServer_SetSettings */

BIP_Status BIP_HttpServer_Start(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpServerStartSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpServerStartSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerStartSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    if (pSettings == NULL)
    {
        BIP_HttpServer_GetDefaultStartSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    else
    {   /* If they passed settings to us, do some validation. */
        BIP_CHECK_GOTO(( pSettings->pPort ), ( "pSettings->pPort can't be NULL, app must provide Port Number to listen on!"), error, BIP_ERR_INVALID_PARAMETER, brc );
        BIP_CHECK_GOTO(( pSettings->pPort && (atoi(pSettings->pPort) > 0) ), ( "pSettings->pPort is not valid %s", pSettings->pPort ), error, BIP_ERR_INVALID_PARAMETER, brc );
        if (pSettings->enableDtcpIp)
        {
            BIP_CHECK_GOTO(( pSettings->dtcpIpServer.pAkePort ), ( "pSettings->dtcpIpServer.pAkePort can't be NULL, app must provide Port Number to listen on for DTCP AKE Requests!"), error, BIP_ERR_INVALID_PARAMETER, brc );
            BIP_CHECK_GOTO(( pSettings->dtcpIpServer.pAkePort && (atoi(pSettings->dtcpIpServer.pAkePort) > 0) ), ( "pSettings->dtcpIpServer.pAkePort is not valid %s", pSettings->dtcpIpServer.pAkePort ), error, BIP_ERR_INVALID_PARAMETER, brc );
        }
    }

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->startApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->startApi.settings = *pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_Start" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return ( brc );
} /* BIP_HttpServer_Start */

BIP_Status BIP_HttpServer_Stop(
    BIP_HttpServerHandle    hHttpServer
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );

    BDBG_MSG((    BIP_MSG_PRE_FMT "Stopping: " BIP_HTTP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_HTTP_SERVER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->stopApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_Stop" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return( brc );
} /* BIP_HttpServer_Stop */

BIP_Status BIP_HttpServer_RecvRequest(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpRequestHandle  *phHttpRequest,
    BIP_HttpServerRecvRequestSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpServerRecvRequestSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerRecvRequestSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( phHttpRequest ), ( "phHttpRequest can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to RecvRequest state among another thread calling the same API. */
    hArb = hHttpServer->recvRequestApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->recvRequestApi.phHttpRequest = phHttpRequest;
    if (pSettings == NULL)
    {
        BIP_HttpServer_GetDefaultRecvRequestSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    hHttpServer->recvRequestApi.settings = *pSettings;

    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS || brc == BIP_INF_TIMEOUT), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_RecvRequest" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return( brc );
}

BIP_Status BIP_HttpServer_RejectRequest(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpRequestHandle  hHttpRequest,
    BIP_HttpServerRejectRequestSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_ApiSettings apiSettings;
    BIP_HttpServerRejectRequestSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerRejectRequestSettings);

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( hHttpRequest ), ( "hHttpRequest can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    if (pSettings == NULL)
    {
        BIP_HttpServer_GetDefaultRejectRequestSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    else
    {
        BIP_CHECK_GOTO(( pSettings->httpStatus != BIP_HttpResponseStatus_e200_OK ), ( "pSettings->httpStatus %d can't be BIP_HttpResponseStatus_e200_OK", pSettings->httpStatus ), error, BIP_ERR_INVALID_PARAMETER, brc );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p, hHttpRequest %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpRequest ));

    /* Serialize access to SednResponse state among another thread calling the same API. */
    hArb = hHttpServer->rejectRequestApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->rejectRequestApi.hHttpRequest = hHttpRequest;
    hHttpServer->rejectRequestApi.settings = *pSettings;
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    B_Os_Memset(&apiSettings, 0, sizeof(apiSettings));
    apiSettings.timeout = -1; /* blocking call as it just aborts the associted httpServerSocket. */
    brc = BIP_Arb_Submit(hArb, &arbSettings, &apiSettings);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS || brc == BIP_INF_IN_PROGRESS),
                   ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_RejectRequest" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return( brc );
}

void BIP_HttpServer_GetDefaultCreateStreamerSettings(
    BIP_HttpServerCreateStreamerSettings *pSettings
    )
{
    BIP_Status brc;

    BDBG_ASSERT( pSettings );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    B_Os_Memset( pSettings, 0, sizeof(*pSettings) );
    BIP_SETTINGS_SET(pSettings,BIP_HttpServerCreateStreamerSettings);

error:
    return;
}

BIP_HttpStreamerHandle BIP_HttpServer_CreateStreamer(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpServerCreateStreamerSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpStreamerHandle hHttpStreamer = NULL;
    BIP_HttpServerCreateStreamerSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerCreateStreamerSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    hArb = hHttpServer->createStreamerApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    if (pSettings == NULL)
    {
        BIP_HttpServer_GetDefaultCreateStreamerSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    hHttpServer->createStreamerApi.settings = *pSettings;
    hHttpServer->createStreamerApi.hHttpStreamer = &hHttpStreamer;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() BIP_HttpServer_CreateStreamer" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));

    return ( hHttpStreamer );
} /* BIP_HttpServer_CreateStreamer */

BIP_Status BIP_HttpServer_StartStreamer(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpRequestHandle hHttpRequest,
    BIP_HttpServerStartStreamerSettings *pSettings
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;
    BIP_HttpServerStartStreamerSettings defaultSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );
    BIP_SETTINGS_ASSERT(pSettings, BIP_HttpServerStartStreamerSettings);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( hHttpStreamer ), ( "hHttpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO( (hHttpRequest), ( "hHttpRequest must be set" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->startStreamerApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    /* Move the API arguments into it's argument list so the state machine can find them. */
    hHttpServer->startStreamerApi.hHttpStreamer = hHttpStreamer;
    hHttpServer->startStreamerApi.hHttpRequest = hHttpRequest;
    if (pSettings == NULL)
    {
        BIP_HttpServer_GetDefaultStartStreamerSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    hHttpServer->startStreamerApi.settings = *pSettings;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_StartStreamer" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: hHttpStreamer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpStreamer, BIP_StatusGetText(brc) ));

    return ( brc );
} /* BIP_HttpServer_StartStreamer */

BIP_Status BIP_HttpServer_StopStreamer(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpStreamerHandle  hHttpStreamer
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( hHttpStreamer ), ( "hHttpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->stopStreamerApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    hHttpServer->stopStreamerApi.hHttpStreamer = hHttpStreamer;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_StopStreamer" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return( brc );
} /* BIP_HttpServer_StopStreamer */

void BIP_HttpServer_DestroyStreamer(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpStreamerHandle  hHttpStreamer
    )
{
    BIP_Status brc = BIP_SUCCESS;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p: --------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer));

    BIP_CHECK_GOTO(( hHttpServer ), ( "hHttpServer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    BIP_CHECK_GOTO(( hHttpStreamer ), ( "hHttpStreamer pointer can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );

    /* Serialize access to Settings state among another thread calling the same _SetSettings API. */
    hArb = hHttpServer->destroyStreamerApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    hHttpServer->destroyStreamerApi.hHttpStreamer = hHttpStreamer;

    /* Get ready to run the state machine. */
    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hHttpServer;
    arbSettings.arbProcessor = processHttpServerStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_HttpServer_DestroyStreamer" ), error, brc, brc );

error:
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: completionStatus %s  <--------------------- " BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_StatusGetText(brc) ));

    return;
} /* BIP_HttpServer_StopStreamer */
