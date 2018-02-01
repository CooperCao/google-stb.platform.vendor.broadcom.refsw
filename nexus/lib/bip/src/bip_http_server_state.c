/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "b_os_lib.h"
#include "bip_priv.h"
#include "bip_http_socket_impl.h"
#include "bip_http_streamer_impl.h"
#include "bip_http_server_impl.h"

BDBG_MODULE( bip_http_server );
BDBG_OBJECT_ID_DECLARE( BIP_HttpServer );

BIP_CLASS_DECLARE(BIP_HttpServer);          /* BIP_HttpServer class is defined in bip_http_server.c. */
BIP_CLASS_DECLARE(BIP_HttpServerSocket);    /* BIP_HttpServer class is defined in bip_http_server_socket.c. */

/* Forward declaration of state processing function */
/* Descriptive names of HTTP Server States */

struct BIP_HttpServerStartStateNames
{
    BIP_HttpServerStartState state;
    char *pStateName;
}gHttpServerStartState[] = {
    {BIP_HttpServerStartState_eUninitialized, "UnInitialized"},
    {BIP_HttpServerStartState_eIdle, "Idle"},
    {BIP_HttpServerStartState_eReadyToStart, "ReadyToStart"},
    {BIP_HttpServerStartState_eStarted, "Started"},
    {BIP_HttpServerStartState_eMax, "MaxState"}
};
#define BIP_HTTP_SERVER_START_STATE(state) \
    gHttpServerStartState[state].pStateName

struct BIP_HttpServerListenerStateNames
{
    BIP_HttpServerListenerState state;
    char *pStateName;
}gHttpServerListenerState[] = {
    {BIP_HttpServerListenerState_eIdle, "Idle"},
    {BIP_HttpServerListenerState_eNew, "New"},
    {BIP_HttpServerListenerState_eListening, "Listening"},
    {BIP_HttpServerListenerState_eRequestQueueFull, "RequestQueueFull"},
    {BIP_HttpServerListenerState_eMax, "MaxState"}
};
#define BIP_HTTP_SERVER_LISTENER_STATE(state) \
    gHttpServerListenerState[state].pStateName

void processHttpServerState( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket, int value, BIP_Arb_ThreadOrigin );
void processHttpServerState_ServerSocket( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket );
void processHttpServer_ServerSocket_ReceivedRequest( BIP_HttpServerHandle hHttpServer );

void processHttpServer_ServerSocket_RecvRequest( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket );
void BIP_HttpServerSocket_Destroy( BIP_HttpServerHandle hHttpServer, BIP_HttpServerSocketHandle hHttpServerSocket);

static void connectedCallbackFromListener (
    void *context,
    int   param
    )
{
    BIP_HttpServerHandle hHttpServer = context;

    BSTD_UNUSED(param);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServer %p, state %s -------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));

    processHttpServerState( hHttpServer, NULL, 0, BIP_Arb_ThreadOrigin_eBipCallback);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpServer %p: state %s <--------------------" BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));
    return;
}

void processHttpServerState_RecvRequestArb(
    BIP_HttpServerHandle hHttpServer
    )
{
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: ENTER ----->" BIP_MSG_PRE_ARG, (void *)hHttpServer ));

    processHttpServer_ServerSocket_ReceivedRequest( hHttpServer );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: EXIT  <-----" BIP_MSG_PRE_ARG, (void *)hHttpServer));
}

static void
destroyDtcpIpServer(
    BIP_HttpServerHandle hHttpServer
    )
{
    BDBG_ASSERT(hHttpServer);

    if ( hHttpServer->hDtcpIpServer )
    {
        BIP_DtcpIpServer_Destroy(  hHttpServer->hDtcpIpServer );
        hHttpServer->hDtcpIpServer = NULL;
    }
} /* destroyDtcpIpServer */

static void
stopDtcpIpServer(
    BIP_HttpServerHandle hHttpServer
    )
{
    BDBG_ASSERT(hHttpServer);

    if ( hHttpServer->hDtcpIpServer )
    {
        BIP_DtcpIpServer_Stop(  hHttpServer->hDtcpIpServer );
    }
} /* stopDtcpIpServer */

static BIP_Status startDtcpIpServer(
    BIP_HttpServerHandle hHttpServer,
    BIP_DtcpIpServerStartSettings *pStartSettings
    )
{
    BIP_Status bipStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;

    hHttpServer->hDtcpIpServer = BIP_DtcpIpServer_Create( NULL );
    if ( hHttpServer->hDtcpIpServer )
    {
        bipStatus = BIP_DtcpIpServer_Start( hHttpServer->hDtcpIpServer, pStartSettings );
    }

    if ( bipStatus != BIP_SUCCESS )
    {
        destroyDtcpIpServer( hHttpServer );
    }
    return ( bipStatus );
} /* startDtcpIpServer */

static void httpServerDestroyAllResources(
    BIP_HttpServerHandle hHttpServer
    )
{
    BIP_HttpStreamerHandle hHttpStreamer;
    BIP_HttpServerSocketHandle hHttpServerSocket;
    if (hHttpServer->startState == BIP_HttpServerStartState_eStarted)
    {
        BIP_Listener_Stop( hHttpServer->listener.hListener );
        stopDtcpIpServer(hHttpServer);
    }

    /* For each active HttpStreamer, call its _Stop() to let it destroy its resources. */
    for (
         hHttpStreamer = BLST_Q_FIRST( &hHttpServer->httpStreamerListHead );
         hHttpStreamer;
         hHttpStreamer = BLST_Q_FIRST( &hHttpServer->httpStreamerListHead )
        )
    {
        BLST_Q_REMOVE( &hHttpServer->httpStreamerListHead, hHttpStreamer, httpStreamerListNext);
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_HttpStreamer_Detroy for hHttpStreamer=%p" BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpStreamer ));
        BIP_HttpStreamer_Destroy( hHttpStreamer );
    }

    /* For each active HttpServerSocket, run its state machine to let it destroy its resources as well. */
    for (
         hHttpServerSocket = BLST_Q_FIRST( &hHttpServer->httpServerSocketInUseListHead );
         hHttpServerSocket;
         hHttpServerSocket = BLST_Q_FIRST( &hHttpServer->httpServerSocketInUseListHead )
        )
    {
        BLST_Q_REMOVE( &hHttpServer->httpServerSocketInUseListHead, hHttpServerSocket, httpServerSocketInUseListNext );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling Destory stateMachine for hHttpServerSocket=%p" BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket ));
        processHttpServerState_ServerSocket(hHttpServer, hHttpServerSocket);
    }

    destroyDtcpIpServer(hHttpServer);

    /* And finally complete the ARB. The caller will then destroy HttpServer's state related resources. */
    BIP_Arb_CompleteRequest( hHttpServer->destroyApi.hArb, BIP_SUCCESS );

} /* httpServerDestroyAllResources */

BIP_HttpServerSocketHandle BIP_HttpServerSocket_Create(
    BIP_HttpServerHandle hHttpServer
    )
{
    BIP_HttpServerSocketHandle hHttpServerSocket = NULL;
    hHttpServerSocket = B_Os_Calloc( 1, sizeof( BIP_HttpServerSocket ));
    if ( hHttpServerSocket )
    {
        BIP_Status               rc;

        /* Store pointer back to the BIP_HttpServer & BIP_Socket(parent & child objects) */
        hHttpServerSocket->hHttpServer = hHttpServer;
        hHttpServerSocket->hSocket = hHttpServer->listener.hSocket;
        hHttpServerSocket->state = BIP_HttpServerSocketState_eUninitialized;

        rc = BIP_CLASS_ADD_INSTANCE(BIP_HttpServerSocket, hHttpServerSocket);
        BIP_CHECK_GOTO((rc==BIP_SUCCESS), ( "BIP_CLASS_ADD_INSTANCE failed" ), error, rc, rc );

        BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_HTTP_SERVER_SOCKET_PRINTF_FMT
                      BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_SOCKET_PRINTF_ARG(hHttpServerSocket)));
    }
    return hHttpServerSocket;

error:
    B_Os_Free(hHttpServerSocket);
    return(NULL);
}

void processHttpServerState_Listener(
    BIP_HttpServerHandle hHttpServer
    )
{
    BIP_Status completionStatus = BIP_INF_IN_PROGRESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer:state %p: %s Queued: Cur %u, Max %u, Total: Accepted %u, ReqTimedout %u, ReqRcvd %u, RespSent %u, StreamerStarted %u, Stopped %u: ENTER ----->"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state),
                hHttpServer->stats.numConcurrentRequestsQueued, hHttpServer->startSettings.maxConcurrentRequestsToQueue,
                hHttpServer->stats.numAcceptedConnections, hHttpServer->stats.numRequestTimeouts,
                hHttpServer->stats.numRcvdRequests, hHttpServer->stats.numSentResponses,
                hHttpServer->stats.numStartedStreamers, hHttpServer->stats.numStoppedStreamers
                ));

    /*
     ***************************************************************************************************************
     * HttpListener State processing: Try to accept a new connection.
     * If available, create a ServerSocket object and start its receive processing.
     ***************************************************************************************************************
     */
    if (hHttpServer->startState != BIP_HttpServerStartState_eStarted)
        return;

     if (hHttpServer->listener.state == BIP_HttpServerListenerState_eRequestQueueFull)
     {
         /* Check if the listener was disabled due to concurrentRequestQueued exceeding the limit! */
         if ( hHttpServer->stats.numConcurrentRequestsQueued < hHttpServer->startSettings.maxConcurrentRequestsToQueue )
         {
             /* Gotback to Listening state as we have now fallen below the maxConcurrentRequests limit. */
             hHttpServer->listener.state = BIP_HttpServerListenerState_eListening;
             BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpServer %p: httpServerListenerState %s: request Queue is no longer Full (%d), resuming to Listening state.."
                         BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state), hHttpServer->stats.numConcurrentRequestsQueued ));
         }
     }

     if (hHttpServer->listener.state == BIP_HttpServerListenerState_eListening)
     {
         /*
          * Listener should be mostly in this state until we reach the maxConcurrentRequestsToQueue limit.
          * In listening state, so we attempt to accept a new connection.
          * We remain in this state until we exceed the maxConcurrentRequests or BIP_HttpServer_Stop is called!
          */
         completionStatus = BIP_INF_IN_PROGRESS;
         BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_Listener_Accept to see if new connectionRequest is pending " BIP_MSG_PRE_ARG, (void *)hHttpServer));

         hHttpServer->listener.hSocket = BIP_Listener_Accept( hHttpServer->listener.hListener, 0 /* timeout */); /* non-blocking accept call */
         if (hHttpServer->listener.hSocket)
         {
             completionStatus = BIP_SUCCESS;
             BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_Listener_Accept returned hSocket %p" BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServer->listener.hSocket));
         }
         else
         {
             /* No new connection available to accept at this time, we remain in the same state! */
             BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_Listener_Accept returned nothing, no new connection available at this time"
                         BIP_MSG_PRE_ARG, (void *)hHttpServer ));
         }
     }

     /* Now use completionStatus to transition to next state */
     if (completionStatus == BIP_SUCCESS)
     {
         BIP_HttpServerSocketHandle hHttpServerSocket;

         /*
          * Success status, we must have accepted a new connection request.
          * Now create a new HttpServerSocket and run its state machine.
          */
         hHttpServer->stats.numAcceptedConnections++;

         hHttpServerSocket = BIP_HttpServerSocket_Create(hHttpServer);
         if ( hHttpServerSocket )
         {
             /* Run HttpSocket's recv state machine to start the receiving the Http Request on this Server */
             processHttpServer_ServerSocket_RecvRequest( hHttpServer, hHttpServerSocket );
             if (hHttpServerSocket->state == BIP_HttpServerSocketState_eDestroying)
             {
                 /* Ran into an error while starting to recv request, clean up. */
                 BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket %p: Destroying ServerSocket object & EXIT "
                             BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket ));
                 BIP_HttpServerSocket_Destroy(hHttpServer, hHttpServerSocket);
                 hHttpServerSocket = NULL;
             }
             else
             {
                 /*
                  * Successfully started the recv state process.
                  * Check if we have reached/exceed the concurrentRequests limit. This count is incremented
                  * when ServerSocket state machine successfully starts the receiving request processing on it.
                  *
                  * When App calls BIP_HttpServer_RecvRequest to receive a new request,
                  * ServerSocket Recv state machine decrements this count for each successful call before returning.
                  *
                  * If count exceeds the max, then we disable listener until app receives these requests or the partial
                  * ones timeout due to keep-alive.
                  */
                 if ( hHttpServer->stats.numConcurrentRequestsQueued >= hHttpServer->startSettings.maxConcurrentRequestsToQueue )
                 {
                     hHttpServer->listener.state = BIP_HttpServerListenerState_eRequestQueueFull;
                     BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpServer %p: httpServerListenerState %s: Request Queue is now Full: Queued %u, Total: accepted %u, ReqRcvd %u"
                                 BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state), hHttpServer->stats.numConcurrentRequestsQueued, hHttpServer->stats.numAcceptedConnections, hHttpServer->stats.numRcvdRequests ));
                 }
             }
         }
         else
         {
             /* Memory allocation failed, destroy the bip socket! */
             BIP_Socket_Destroy( hHttpServer->listener.hSocket );
             hHttpServer->listener.hSocket = NULL;
             BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: Failed to allocate memory (%zu bytes) for HttpServerSocket Object, Destroyed the incoming BIP_Socket connection"
                         BIP_MSG_PRE_ARG, (void *)hHttpServer, sizeof(BIP_HttpServerSocket) ));
         }
     } /* SUCCESS case */
     else if (completionStatus == BIP_INF_IN_PROGRESS)
     {
         BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: httpServerListenerState %s: nothing to accept at this time! "
                     BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state)));
     }
     else
     {
         BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: httpServerListenerState %s: Error in ListeningState, we keep listening!, completionStatus 0x%x "
                     BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state), completionStatus ));
     }

     /*
      * Check if we should enable or disable connectedCallback from Listener.
      * It only gets disabled when we are not in the Listening state.
      * This happens when we have accepted connections to exceed the maxConcurrentRequestsToQueue requests.
      */
     {
         bool updateSettings = false;
         BIP_ListenerSettings listenerSettings;

         if ( hHttpServer->listener.callbackState == BIP_ListenerCallbackState_eDisabled )
         {
             if (hHttpServer->listener.state == BIP_HttpServerListenerState_eListening)
             {
                 BIP_Listener_GetSettings( hHttpServer->listener.hListener, &listenerSettings );
                 listenerSettings.connectedCallback.callback = connectedCallbackFromListener;
                 listenerSettings.connectedCallback.context = hHttpServer;
                 hHttpServer->listener.callbackState = BIP_ListenerCallbackState_eEnabled;
                 updateSettings = true;
             }
         }
         else if ( hHttpServer->listener.callbackState == BIP_ListenerCallbackState_eEnabled )
         {
             if (hHttpServer->listener.state != BIP_HttpServerListenerState_eListening)
             {
                 BIP_Listener_GetSettings( hHttpServer->listener.hListener, &listenerSettings );
                 listenerSettings.connectedCallback.callback = NULL;
                 hHttpServer->listener.callbackState = BIP_ListenerCallbackState_eDisabled;
                 updateSettings = true;
             }
         }
         if (updateSettings)
         {
             completionStatus = BIP_Listener_SetSettings( hHttpServer->listener.hListener, &listenerSettings );
             /* Check if we had failed to update the Listener's SetSettings */
             if ( completionStatus != BIP_SUCCESS )
             {
                 /* Just need to log the error for now */
                 BDBG_ERR((BIP_MSG_PRE_FMT "hHttpServer %p: state %s: ERROR: BIP_Listener_SetSettings Failed: completionStatus 0x%x"
                             BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), completionStatus ));
             }
         }
         BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_Listener Settings, httpServerState %s, connectedCallback %s, updated %s"
                     BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState),
                     hHttpServer->listener.callbackState ? "Enabled":"Disabled", updateSettings? "Yes":"No"));
     }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer:state %p: %s : EXIT   <-----" BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_LISTENER_STATE(hHttpServer->listener.state)));
} /* processHttpServerState_Listener */

void processHttpServerState(
        BIP_HttpServerHandle hHttpServer,               /* HttpServer object handle */
        BIP_HttpServerSocketHandle hHttpServerSocket,   /* HttpServerSocket object: may not be always set */
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    bool                    reRunProcessState;

    BSTD_UNUSED(value);

    brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpServer, hHttpServer);
    if (brc != BIP_SUCCESS) { return; }

    /* Just validate the HttpServerSocket handle, but don't keep it locked. */
    if (hHttpServerSocket)
    {
        brc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpServerSocket, hHttpServerSocket);
        if (brc != BIP_SUCCESS)
        {
            BIP_CLASS_UNLOCK(BIP_HttpServer, hHttpServer);
            return;
        }
        BIP_CLASS_UNLOCK(BIP_HttpServerSocket, hHttpServerSocket);
    }

    BDBG_ASSERT(hHttpServer);
    BDBG_OBJECT_ASSERT( hHttpServer, BIP_HttpServer);

    /*
     ***************************************************************************************************************
     * HttpServer State Machine Processing:
     *
     *  -Handles HttpServer specific states: Server state transitions like this when Server APIs are invoked:
     *   _HttpServer_Create() -> _eIdle: Server object is created!
     *   _HttpServer_SetSettings() -> _eReadyToStart: Server has required settings to start listening for incoming connections.
     *   _HttpServer_Start() -> _eStarted : all connections, HTTP Req, Resp, Payload streaming happens in this state!
     *   _HttpServer_Stop() -> _eStopped: Listener is stopped!
     *
     ***************************************************************************************************************
     */

    /*
     ***************************************************************************************************************
     * HttpServerSocket State Machine Processing:
     *
     *  -HttpServerSocket is container object that hold pointers to all objects used for a connection such as:
     *      HttpSocketHandle, HttpRequestHandle, HttpResponseHandle, HttpStreamerHandle, etc.
     *  -One HttpServerSocket object per TCP Connection,
     *
     *  When processHttpServerState() can be called as following:
     *      From Arb on behalf of app called APIs (BIP_HttpServer_* APIs)
     *      From BIP_Listener via the connectedCallbackFromListener
     *      From BIP_HttpSocket via the requestedReceivedCallback
     *      From BIP_Timer via the timer Callback
     *
     ***************************************************************************************************************
     */
    B_Mutex_Lock( hHttpServer->hStateMutex );
    reRunProcessState = true;
    while (reRunProcessState)
    {
        reRunProcessState = false;
        BDBG_MSG(( BIP_MSG_PRE_FMT "ENTRY ---> hHttpServer %p: state %s, hHttpServerSocket %p: threadOrigin %d "
                    BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), (void *)hHttpServerSocket , threadOrigin ));
        /*
         ***************************************************************************************************************
         * First, we check API Arbs to see if state processing is being run thru any of these APIs.
         ***************************************************************************************************************
         */
        if (BIP_Arb_IsNew(hArb = hHttpServer->getSettingsApi.hArb))
        {
            /* App is request current HttpServer settings. */
            BIP_Arb_AcceptRequest(hArb);

            /* Return the current cached settings. */
            BKNI_Memcpy(hHttpServer->getSettingsApi.pSettings, &hHttpServer->settings, sizeof (*hHttpServer->getSettingsApi.pSettings) );

            hHttpServer->completionStatus = BIP_SUCCESS;
            /* We are done this API Arb, so set its completion status. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: GetSettings Arb request is complete: state %s!"
                        BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));
            BIP_Arb_CompleteRequest( hHttpServer->getSettingsApi.hArb, hHttpServer->completionStatus);
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->setSettingsApi.hArb))
        {
            BIP_Arb_AcceptRequest(hArb);
            /*
             * We cache the settings into server object. We will update the
             * BIP_Listener's settings in the HttpListener state handling.
             * When SetSettings is called before starting the Server,
             * we will only update the BIP_Listener's settings after BIP_HttpServer_Start().
             *
             * Note: API side code has already verified the requied Setting parameters!
             */
            hHttpServer->settings = hHttpServer->setSettingsApi.settings;

            if ( hHttpServer->startState == BIP_HttpServerStartState_eIdle )
            {
                /*
                 * We are in idle state and caller has set the Server's settings, so we are ready to get started.
                 */
                hHttpServer->startState = BIP_HttpServerStartState_eReadyToStart;
            }

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: SetSettings Arb request is complete : state %s!"
                        BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));
            hHttpServer->completionStatus = BIP_SUCCESS;
            BIP_Arb_CompleteRequest( hHttpServer->setSettingsApi.hArb, hHttpServer->completionStatus);
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->startApi.hArb))
        {
            /* App is starting the Server, make sure we are in the correct state to do so! */
            if (hHttpServer->startState != BIP_HttpServerStartState_eReadyToStart)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hServer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpServer_Start not allowed in this state: %s, BIP_HttpServer_SetSettings() must be called to set the Server settings!"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState)));
                hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpServer->completionStatus);
            }
            else
            {
                BIP_ListenerSettings listenerSettings;
                BIP_ListenerStartSettings listenerStartSettings;
                /*
                 * We are in ReadyToStart state and caller has called BIP_HttpServer_Start(). So lets get started.
                 */
                BIP_Arb_AcceptRequest(hArb);

                hHttpServer->startSettings = hHttpServer->startApi.settings;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Accepted _Start Arb: state %s!"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));

                /* Start dtcp/ip server if enabled. */
                if (hHttpServer->startSettings.enableDtcpIp)
                {
                    hHttpServer->completionStatus = startDtcpIpServer(hHttpServer, &hHttpServer->startSettings.dtcpIpServer);
                }

                if (hHttpServer->completionStatus == BIP_SUCCESS)
                {
                    /* Update Listener's settings: port, IP address, and interface, etc. */
                    BIP_Listener_GetSettings( hHttpServer->listener.hListener, &listenerSettings );
                    listenerSettings.connectedCallback.callback = connectedCallbackFromListener;
                    listenerSettings.connectedCallback.context = hHttpServer;
                    hHttpServer->completionStatus = BIP_Listener_SetSettings( hHttpServer->listener.hListener, &listenerSettings );

                    if (hHttpServer->completionStatus == BIP_SUCCESS)
                    {
                        BIP_Listener_GetDefaultStartSettings(&listenerStartSettings);
                        listenerStartSettings.pPort = hHttpServer->startSettings.pPort;
                        listenerStartSettings.pInterfaceName = hHttpServer->startSettings.pInterfaceName;
                        listenerStartSettings.ipAddressType = hHttpServer->startSettings.ipAddressType;

                        /* Successfully updated BIP_Listener's Settings, now Start it! */
                        hHttpServer->completionStatus = BIP_Listener_Start( hHttpServer->listener.hListener, &listenerStartSettings );
                    }
                }
                /* we complete ARB with status of the these BIP Listener calls */
                BIP_Arb_CompleteRequest( hHttpServer->startApi.hArb, hHttpServer->completionStatus);

                BIP_CHECK_GOTO(( hHttpServer->completionStatus == BIP_SUCCESS ),
                        ( "hHttpServer %p, state %s: status %s, Failed to Start the HttpServer", (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), BIP_StatusGetText(hHttpServer->completionStatus)),
                        error, hHttpServer->completionStatus, hHttpServer->completionStatus );

                /* We have successfully started the BIP_Listener at this point, so we switch to the Started state! */
                hHttpServer->startState = BIP_HttpServerStartState_eStarted;
                hHttpServer->listener.state = BIP_HttpServerListenerState_eListening;

                BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Started: " BIP_HTTP_SERVER_PRINTF_FMT
                              BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_PRINTF_ARG(hHttpServer)));

                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpServer %p, state %s: BIP_HttpServer Started on Port %s -----<>"
                              BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), hHttpServer->startSettings.pPort));
            }
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->stopApi.hArb))
        {
            if (hHttpServer->startState != BIP_HttpServerStartState_eStarted)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpServer_Sttop not allowed in this state: %s, we must be in BIP_HttpServerStartState_eStarted state! "
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState)));
                hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpServer->completionStatus);
            }
            else
            {
                /*
                 * We are in BIP_HttpServerStartState_eStarted, so initiate stop sequence.
                 */
                BIP_Arb_AcceptRequest(hArb);

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Accepted _Stop Arb: state %s!"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));
                /*
                 * We may have associated httpServerSockets either in the RcvdRequest state or
                 * in SendingResponse or later states (streaming etc.).
                 *
                 * Should we purge the httpSocketSockets which are in the pending list or let app still have them via RecvRequest API?
                 * TODO: Add this logic below.
                 * Rest of the httpServerSockets should continue to exist until they are naturally closed.
                 */
                hHttpServer->completionStatus = BIP_Listener_Stop( hHttpServer->listener.hListener );
                BIP_Arb_CompleteRequest( hHttpServer->stopApi.hArb, hHttpServer->completionStatus);
                BIP_CHECK_GOTO(( hHttpServer->completionStatus == BIP_SUCCESS ), ( "hHttpServer %p, state %s: BIP_Listener_Stop Failed", (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ),
                    error, hHttpServer->completionStatus, hHttpServer->completionStatus );

                stopDtcpIpServer(hHttpServer);

                /* Since we stopped the BIP_Listener, we are back to ReadyToStart state! */
                hHttpServer->startState = BIP_HttpServerStartState_eReadyToStart;

                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpServer %p, state %s: BIP_HttpServer Stopped on Port %s -----<>"
                              BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), hHttpServer->startSettings.pPort));
            }
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->recvRequestApi.hArb))
        {
            /* Caller has issued _RecvRequest API */
            if ( hHttpServer->startState != BIP_HttpServerStartState_eStarted )
                /* should we allow recvReq if we have them in the pending list even though server has been stopped!? */
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpServer_RecvRequest() is not allowed in this state: %s (May need to call BIP_HttpServer_Start() first!"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState)));
                hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpServer->completionStatus);
            }
            else
            {
                /*
                 * We allow _RecvRequest API for a server only in its Started state.
                 */
                BIP_Arb_AcceptRequest(hArb);

                hHttpServer->completionStatus = BIP_INF_IN_PROGRESS;

                /*
                 * We are in Started state and caller has issued _RecvRequest().
                 * So run the recvRequestApi related state machine.
                 */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: Accepted RecvRequest Arb request: state %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));
                processHttpServerState_RecvRequestArb( hHttpServer );
            }
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->rejectRequestApi.hArb))
        {
            if ( hHttpServer->startState != BIP_HttpServerStartState_eStarted )
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpServer_RejectRequest() not allowed in this state: %s "
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState)));
                hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpServer->completionStatus );
            }
            else
            {
                /*
                 * App wants to Reject the Request for its own reasons.
                 * Get the ServerSocket object reference from the Response object &
                 * run its state machine to check if we are allowed to reject request in that state
                 * and then prepare & send the HTTP Error Response.
                 */
                brc = BIP_HttpRequest_GetUserData(hHttpServer->rejectRequestApi.hHttpRequest, (void **)&hHttpServerSocket);
                BDBG_ASSERT( brc == BIP_SUCCESS );

                if ( hHttpServerSocket == NULL )
                {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: hHttpServerSocket is NULL, invalid API sequence!"
                                BIP_MSG_PRE_ARG, (void *)hHttpServer ));
                    hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                    BIP_Arb_RejectRequest( hArb, hHttpServer->completionStatus );
                }
                else
                {
                    /*
                     * Accept ARB and continue w/ state machine processing below.
                     */
                    BIP_Arb_AcceptRequest(hArb);
                    hHttpServer->completionStatus = BIP_INF_IN_PROGRESS;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer:state %p:%s: Accepted AbortResponse Arb request, hHttpServerSocket:state %p"
                                BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), (void *)hHttpServerSocket ));
                    /* This request will be further processed in the processHttpServerSocket state below! */
                }
            }
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->createStreamerApi.hArb))
        {
            BIP_HttpStreamerCreateSettings createSettings;
            BIP_HttpStreamerHandle hHttpStreamer;

            /* Note: we dont do any HttpServer state check as BIP_HttpServer_CreateStreamer() can be called before BIP_Server_Start() */
            BIP_Arb_AcceptRequest(hArb);
            BIP_HttpStreamer_GetDefaultCreateSettings( &createSettings );
            createSettings.endOfStreamingCallback = hHttpServer->createStreamerApi.settings.endOfStreamingCallback;
            createSettings.softErrorCallback = hHttpServer->createStreamerApi.settings.softErrorCallback;
            hHttpStreamer = BIP_HttpStreamer_Create( &createSettings );

            /* set the return value */
            *hHttpServer->createStreamerApi.hHttpStreamer = hHttpStreamer;

            if (hHttpStreamer)
            {
                /* Add this Streamer Handle to a list so that we can track these streamers. Need it for assigning them requests for session type streamer protocols like HLS & DASH. */
                BLST_Q_INSERT_TAIL( &hHttpServer->httpStreamerListHead, hHttpStreamer, httpStreamerListNext);

                hHttpServer->completionStatus = BIP_SUCCESS;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_HttpServer_CreateStreamer Arb request is complete : hHttpStreamer %p"
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)*hHttpServer->createStreamerApi.hHttpStreamer ));
            }
            else
            {
                hHttpServer->completionStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_HttpServer_CreateStreamer Failed "
                            BIP_MSG_PRE_ARG, (void *)hHttpServer ));
            }

            BIP_Arb_CompleteRequest( hHttpServer->createStreamerApi.hArb, hHttpServer->completionStatus);
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->destroyStreamerApi.hArb))
        {
            BIP_Arb_AcceptRequest(hArb);

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_HttpServer_DestroyStreamer Arb request is accepted : hHttpStreamer %p"
                        BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServer->destroyStreamerApi.hHttpStreamer ));

            /* Destroy the Streamer */
            BLST_Q_REMOVE( &hHttpServer->httpStreamerListHead, hHttpServer->destroyStreamerApi.hHttpStreamer, httpStreamerListNext);
            BIP_HttpStreamer_Destroy( hHttpServer->destroyStreamerApi.hHttpStreamer );

            /*
             * NOTE: if there is an associated serverSocket object owning this streamer object,
             * its deletions is deferred to its state machine.
             * This is done because recv state machine of server socket would detect a receive request error
             * from HttpSocket when peer has closed/aborted a connection or will get a timeout error via the persistenConnection timeout
             * from HttpSocket when new request doesn't arrive in that interval.
             */
            hHttpServer->completionStatus = BIP_SUCCESS;
            BIP_Arb_CompleteRequest( hHttpServer->destroyStreamerApi.hArb, hHttpServer->completionStatus);
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->startStreamerApi.hArb))
        {
            /* Caller has issued _StartStreamer API */
            if ( hHttpServer->startState != BIP_HttpServerStartState_eStarted )
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: Calling BIP_Arb_RejectRequest(): BIP_HttpServer_StartStreamer() Api not allowed in this state: %s "
                            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState)));
                hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpServer->completionStatus );
            }
            else
            {
                /*
                 * Server is in the correct state, now get the ServerSocket object reference from the Request object.
                 * We only allow starting streamer after the response has been sent back.
                 */
                brc = BIP_HttpRequest_GetUserData(hHttpServer->startStreamerApi.hHttpRequest, (void **)&hHttpServerSocket);
                BDBG_ASSERT( brc == BIP_SUCCESS );

                if ( hHttpServerSocket == NULL )
                {
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: hHttpServerSocket is %p NULL, BIP_HttpServer_StartStreamer() invoked in correct sequence!"
                                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket ));
                    hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                    BIP_Arb_RejectRequest( hArb, hHttpServer->completionStatus );
                }
                else
                {
                    /* We can start streamer, so lets get started! */
                    /* Note: we dont accept the ARB here as we want ServerSocket to do one time work. */
                    /* So Start Streamer state machine processing w/ Accept the ARB. */

                    /* Now we can associate this streamer w/ the ServerSocket object until _StopStreamer() is called */
                    /* NOTE: API side has already validated this pointer to be non-NULL */
                    hHttpServerSocket->hHttpStreamer = hHttpServer->startStreamerApi.hHttpStreamer;
                    hHttpServerSocket->completionStatus = BIP_INF_IN_PROGRESS;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer:state %p:%s: hHttpStreamer %p: Starting StartStreamer request!"
                                BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState), (void *)hHttpServerSocket->hHttpStreamer));
                }
            }
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->stopStreamerApi.hArb))
        {
            /*
             * App has issued _StopStreamer API. We allow it Stop the HttpStreamers after
             * Stopping the HttpServer. That is why we dont check for the HttpServer states.
             */
            BIP_Arb_AcceptRequest(hArb);

            /*
             * Also, we dont go thru the ServerSocket state machine here as ServerSocket either
             * is already dis-associated w/ the Streamer using the RequestProcessed Callback or
             * will be after Streamer gets this Stop call and issues the RequestProcessed Callback.
             */

            hHttpServer->completionStatus = BIP_HttpStreamer_Stop( hHttpServer->stopStreamerApi.hHttpStreamer);
            hHttpServer->stats.numStoppedStreamers++;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer %p: Streamer is Stopped" BIP_MSG_PRE_ARG, (void *)hHttpServer->stopStreamerApi.hHttpStreamer));
            BIP_Arb_CompleteRequest( hHttpServer->stopStreamerApi.hArb, hHttpServer->completionStatus);
        }
        else if (BIP_Arb_IsNew(hArb = hHttpServer->destroyApi.hArb))
        {
            /*
             * App has issued BIP_HttpServer_Destroy API.
             * We will cleanup any HttpServer related resources below using the httpServerDestroyAllResources().
             */
            BIP_Arb_AcceptRequest(hArb);

            hHttpServerSocket = NULL;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: BIP_HttpServer_Destroy() in progress..." BIP_MSG_PRE_ARG, (void *)hHttpServer ));
        }

        /*
         * If Server is not not being destroyed, then Server's startState processing is done at this point.
         * First we process HttpServerSocket associated states for APIs
         * _RecvRequest, _StartStreamer, _RejectRequest.
         *
         * Then, we do Server's Listener specific state processing where
         * we accept new connection requests & start to receive the requests.
         *
         */
        if ( BIP_Arb_IsBusy(hHttpServer->destroyApi.hArb) == false )
        {
            /* Do the HttpServerSocket specific state processing first */
            if ( hHttpServerSocket )
            {
                processHttpServerState_ServerSocket( hHttpServer, hHttpServerSocket );
            }
            processHttpServerState_Listener( hHttpServer );
            BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stats: " BIP_HTTP_SERVER_STATS_PRINTF_FMT
                          BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_STATS_PRINTF_ARG(hHttpServer)));
        }
        else
        {
            /* Server is being destroyed, so destroy all of associated resources suchas HttpStreamers & HttpServerSockets. */
            httpServerDestroyAllResources( hHttpServer );
        }

error:
        /*
         * Update the HttpServer object state based on the completion status of any APIs.
         */
        BDBG_MSG((BIP_MSG_PRE_FMT "%s: done .." BIP_MSG_PRE_ARG, BSTD_FUNCTION));
    } /* while reRunProcessState */

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing HTTP State for hHttpServer %p: state %s"
            BIP_MSG_PRE_ARG, (void *)hHttpServer, BIP_HTTP_SERVER_START_STATE(hHttpServer->startState) ));

    /*
     * Done with state processing. We have to unlock state machine before issuing callbacks!
     * Issue any direct callbacks to caller (which happens when caller hasn't yet called either _Recv & _Send API.
     * And instead state machine was run via a _SetSettings or callbacks from BIP_Socket.
     */
    B_Mutex_Unlock( hHttpServer->hStateMutex );

    BIP_CLASS_UNLOCK(BIP_HttpServer, hHttpServer);

    brc = BIP_Arb_DoDeferred( hHttpServer->recvRequestApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );

    BDBG_MSG(( BIP_MSG_PRE_FMT "EXIT <--- hHttpServer %p" BIP_MSG_PRE_ARG, (void *)hHttpServer ));
    return;
}

void processHttpServerStateFromArb(
    void *hObject,
    int value
    )
{
    processHttpServerState( (BIP_HttpServerHandle) hObject, NULL /*hHttpServerSocket*/, value, BIP_Arb_ThreadOrigin_eArb);
}
