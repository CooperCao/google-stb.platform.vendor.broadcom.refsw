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

#include "b_os_lib.h"
#include "bip_priv.h"
#include "bip_http_socket_impl.h"
#include "bip_http_server_impl.h"
#include "bip_http_streamer_impl.h"

BDBG_MODULE( bip_http_server );
BDBG_OBJECT_ID_DECLARE( BIP_HttpServer );
BIP_CLASS_DECLARE(BIP_HttpServerSocket);
BIP_CLASS_DECLARE(BIP_HttpServer);   /* BIP_HttpServer class is defined in bip_http_server.c. */

/* Forward declaration of state processing function */
/* Descriptive names of HTTP Server States */

struct BIP_HttpServerSocketStateNames
{
    BIP_HttpServerSocketState state;
    char *pStateName;
}gHttpServerSocketState[] = {
    {BIP_HttpServerSocketState_eUninitialized, "UnInitialized"},
    {BIP_HttpServerSocketState_eIdle, "Idle"},
    {BIP_HttpServerSocketState_eWaitingForRequestArrival, "WaitingForRequestArrival"},
    {BIP_HttpServerSocketState_eWaitingForRecvRequestApi, "WaitingForRecvRequestApi"},
    {BIP_HttpServerSocketState_eWaitingForStartStreamerApi, "WaitingForStartStreamerApi"},
    {BIP_HttpServerSocketState_eProcessingRequest, "ProcessingRequest"},
    {BIP_HttpServerSocketState_eDestroying, "Destroying"},
    {BIP_HttpServerSocketState_eMax, "MaxState"}
};
#define BIP_HTTP_SERVER_SOCKET_STATE(state) \
    gHttpServerSocketState[state].pStateName

/* Helper function to destroy HttpServerSocket & its associated objects */
void BIP_HttpServerSocket_Destroy(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSocketHandle hHttpServerSocket
    )
{
    BDBG_MSG((    BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_SERVER_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_SOCKET_PRINTF_ARG(hHttpServerSocket)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_HTTP_SERVER_SOCKET_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_HTTP_SERVER_SOCKET_PRINTF_ARG(hHttpServerSocket)));

    if (!hHttpServer || !hHttpServerSocket)
        return;

    BIP_CLASS_REMOVE_INSTANCE(BIP_HttpServerSocket, hHttpServerSocket);

    BLST_Q_REMOVE( &hHttpServer->httpServerSocketInUseListHead, hHttpServerSocket, httpServerSocketInUseListNext );

    if (hHttpServerSocket->httpServerSocketRcvdRequestNext.l_head != NULL)
    {
        /* Remove this Request from the RcvdRequestList */
        BLST_Q_REMOVE( &hHttpServer->httpServerSocketRcvdRequestHead, hHttpServerSocket, httpServerSocketRcvdRequestNext );

        /* Decrement the concurrentRequestsQueued count as this one is has run into an error! */
        hHttpServer->stats.numConcurrentRequestsQueued--;
    }

    if (!hHttpServerSocket->httpSocketOwnedByCaller)
    {
        if (hHttpServerSocket->hHttpSocket)
        {
            BIP_HttpSocket_Destroy(hHttpServerSocket->hHttpSocket);
        }
        else
        {
            /*
             * Since hHttpSocket was NULL, that means we will need to free up BIPSocket object.
             * Otherwise, hHttpSocket object owns BIPSocket object and takes care of freeing it.
             */
            if (hHttpServerSocket->hSocket) BIP_Socket_Destroy(hHttpServerSocket->hSocket);
        }
    }

    B_Os_Free( hHttpServerSocket );

    return;
} /* BIP_HttpServerSocket_Destroy */

static void requestProcessedCallbackFromHttpStreamer (
    void *context,
    int   param
    )
{
    BIP_Status                  rc;
    BIP_HttpServerSocketHandle  hHttpServerSocket = context;

    BSTD_UNUSED(param);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServerSocket %p: state %s -------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpServer, hHttpServerSocket->hHttpServer);
    if (rc != BIP_SUCCESS) { return; }

    rc = BIP_CLASS_LOCK_AND_CHECK_INSTANCE(BIP_HttpServerSocket, hHttpServerSocket);
    if (rc != BIP_SUCCESS)
    {
        BIP_CLASS_UNLOCK(BIP_HttpServer, hHttpServerSocket->hHttpServer);
        return;
    }

    BDBG_ASSERT(hHttpServerSocket);
    BDBG_ASSERT(hHttpServerSocket->hHttpServer);

    B_Mutex_Lock(hHttpServerSocket->hHttpServer->hStateMutex);
    hHttpServerSocket->requestProcessed = true;
    B_Mutex_Unlock(hHttpServerSocket->hHttpServer->hStateMutex);

    BIP_CLASS_UNLOCK(BIP_HttpServerSocket, hHttpServerSocket);
    BIP_CLASS_UNLOCK(BIP_HttpServer, hHttpServerSocket->hHttpServer);

    processHttpServerState( hHttpServerSocket->hHttpServer, hHttpServerSocket, 0, BIP_Arb_ThreadOrigin_eBipCallback);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: <--------------------" BIP_MSG_PRE_ARG));
    return;
}

static void requestReceivedCallbackFromHttpSocket (
    void *context,
    int   param
    )
{
    BIP_HttpServerSocketHandle hHttpServerSocket = context;
    BSTD_UNUSED(param);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpServerSocket %p: state %s -------------------->" BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));

    processHttpServerState( hHttpServerSocket->hHttpServer, hHttpServerSocket, 0, BIP_Arb_ThreadOrigin_eBipCallback);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: <--------------------" BIP_MSG_PRE_ARG));
    return;
}

static BIP_Status sendRejectResponse(
    BIP_HttpSocketHandle     hHttpSocket,
    BIP_HttpRequestHandle    hHttpRequest,
    BIP_HttpResponseHandle   hHttpResponse,
    const char               *pCustomResponseHeaders, /*TODO: this can be removed withe new interface, if any custome header need to be added then we should pass in both header nad value name.*/
    BIP_HttpResponseStatus   httpStatus
    )
{
    BIP_Status completionStatus;
    BIP_HttpSocketSendResponseSettings sendResponseSettings;


    /* Prepare the Error Response. */
    BIP_HttpResponse_Clear(hHttpResponse, NULL);

    completionStatus = BIP_HttpResponse_SetStatus( hHttpResponse , httpStatus);
    /*TODO: Need to check what should we do in this case when SetStatus for reject response fails.*/

    BSTD_UNUSED( pCustomResponseHeaders );
#if 0
    /* TODO until HttpResponse class supports this API. */
    BIP_HttpResponse_AddRawHeader( hHttpResponse, pCustomResponseHeaders);
#endif

    BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
    sendResponseSettings.noPayload = true;
    sendResponseSettings.timeoutInMs = -1;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Sending Error Response: hHttpSocket %p" BIP_MSG_PRE_ARG, (void *)hHttpSocket));
    completionStatus = BIP_HttpSocket_SendResponse(hHttpSocket, hHttpRequest, hHttpResponse, 0 /* messageLength */, &sendResponseSettings);
    if (completionStatus == BIP_SUCCESS)
    {
        /* Reset the request & response objects to their original state incase it is being re-used. */
        BIP_HttpRequest_Clear(hHttpSocket->hHttpRequest, NULL);
        BIP_HttpResponse_Clear(hHttpSocket->hHttpResponse, NULL);
#if 0/* TODO: Check with Sanjeev if he wants to print the HttpSocketState and how.*/
        BDBG_MSG(( BIP_MSG_PRE_FMT "Error Response Sent: hHttpSocket %p, state %s" BIP_MSG_PRE_ARG, hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
#endif
        BDBG_MSG(( BIP_MSG_PRE_FMT "Error Response Sent: hHttpSocket %p" BIP_MSG_PRE_ARG, (void *)hHttpSocket));
    }
    return ( completionStatus );
} /* sendRejectResponse */

/* HttpServerSocket State Machine */
static void processHttpServer_ServerSocketState_RejectRequest(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSocketHandle hHttpServerSocket
    )
{
    BIP_Status completionStatus = BIP_ERR_INTERNAL;

    BDBG_ASSERT(hHttpServer);
    BDBG_ASSERT(hHttpServerSocket);
    if (!hHttpServer || !hHttpServerSocket)
        return;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: ENTER ----->"
                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    if ( BIP_Arb_IsBusy(hHttpServer->rejectRequestApi.hArb) == true )
    {
        /* We were waiting for StartStreamerApi from App but it decided to reject the request */
        /* so we will prepare an error response, send it out, and go back to the Idle state. */
        if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eWaitingForStartStreamerApi )
        {
            completionStatus = sendRejectResponse( hHttpServerSocket->hHttpSocket, hHttpServerSocket->hHttpRequest, hHttpServerSocket->hHttpResponse, hHttpServer->rejectRequestApi.settings.customResponseHeaders, hHttpServer->rejectRequestApi.settings.httpStatus );
            if (completionStatus == BIP_SUCCESS)
            {
                hHttpServer->stats.numRejectRequests++;
                {
                    /* Since there was no payload associated with this response, we go back to receiving next request! */
                    hHttpServerSocket->state = BIP_HttpServerSocketState_eIdle;
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "Error Response Sent: hHttpServerSocket %p, state %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state)));
            }
            /* TODO: add checks for other non-Error codes to support non-blocking cases! */
            else
            {
                /* Failed to SendResponse, its an error case! */
                hHttpServerSocket->state = BIP_HttpServerSocketState_eDestroying;
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServerSocket %p BIP_HttpSocket_RejectResponse() Failed: error Status: %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_StatusGetText(completionStatus) ));
            }
        }
        else
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer: %p, hHttpServerSocket %p: Invalid API sequence: Can't call BIP_HttpServer_RejectRequest in current httpServSocket state (: %s)"
                        BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
            hHttpServer->completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
        }
        BIP_Arb_CompleteRequest(hHttpServer->rejectRequestApi.hArb, completionStatus);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p Http_RejectResponse() completionStatus: %s,  EXIT <----- "
                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_StatusGetText(completionStatus) ));
} /* processHttpServer_ServerSocketState_RejectRequest */

/* We have already received the Request, now check for any API Arb completion or Callbacks to App. */
void processHttpServer_ServerSocket_ReceivedRequest(
    BIP_HttpServerHandle hHttpServer
    )
{
    BIP_HttpRequestMethod method;
    const char *pMethodName = NULL;
    BIP_HttpServerSocketHandle hHttpServerSocket;
    BIP_Status completionStatus = BIP_ERR_INTERNAL;

    /*
     * If we have a received a complete Request, then we can take any of the following actions:
     *  -App has already issued a BIP_HttpServer_RecvRequest(), so we complete the Arb w/ this object. Or,
     *  -App has enabled the requestReceivedCallback, so we queue up this callback. Or,
     *  -We do nothing and just keep the request queued up. App will later do either of the previous steps to receive it.
     */
    hHttpServerSocket = BLST_Q_FIRST( &hHttpServer->httpServerSocketRcvdRequestHead);
    if (hHttpServerSocket == NULL)
    {
        /* No complete request available at this time */
        if (BIP_Arb_IsBusy(hHttpServer->recvRequestApi.hArb))
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p: No Request available at this time" BIP_MSG_PRE_ARG, (void *)hHttpServer ));
            /* Complete the Arb! */
            BIP_Arb_CompleteRequest(hHttpServer->recvRequestApi.hArb, BIP_INF_TIMEOUT);
        }
        return;
    }

    /*
     * We have atleast one full request available, check if there a pending API request from app or a callback requested.
     */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : ENTER ----->"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    BDBG_ASSERT( hHttpServerSocket->state == BIP_HttpServerSocketState_eWaitingForRecvRequestApi );

    completionStatus = BIP_HttpRequest_GetMethod(hHttpServerSocket->hHttpRequest, &method, &pMethodName);

    if (BIP_Arb_IsBusy(hHttpServer->recvRequestApi.hArb))
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Request (method %s) Received on hHttpServerSocket %p, completing _RecvRequest ARB"
                    BIP_MSG_PRE_ARG, pMethodName, (void *)hHttpServerSocket ));
        *hHttpServer->recvRequestApi.phHttpRequest = hHttpServerSocket->hHttpRequest;
        if ( hHttpServer->recvRequestApi.settings.phHttpSocket )
        {
            *hHttpServer->recvRequestApi.settings.phHttpSocket = hHttpServerSocket->hHttpSocket;
            hHttpServerSocket->httpSocketOwnedByCaller = true;
        }

        /* Remove this Request from the RcvdRequestList */
        BLST_Q_REMOVE( &hHttpServer->httpServerSocketRcvdRequestHead, hHttpServerSocket, httpServerSocketRcvdRequestNext );
        /*
         * We reset the head her as destroy will remove the object from this list if we r still pointing to head.
         * Can happen if we get an error before app gets a chance to receive the request (client closed the connection).
         */
        hHttpServerSocket->httpServerSocketRcvdRequestNext.l_head = NULL;

        /* Decrement the concurrentRequestsQueued count as app is receiving one! */
        hHttpServer->stats.numConcurrentRequestsQueued--;

        /* Increment the total number requests received. */
        hHttpServer->stats.numRcvdRequests++;

        /* And change state to ReadyToSendResponse State as app has issued the _RecvRequest API. */
        hHttpServerSocket->state = BIP_HttpServerSocketState_eWaitingForStartStreamerApi;

        /* We have pending Arb from app, so complete it */
        BIP_Arb_CompleteRequest(hHttpServer->recvRequestApi.hArb, BIP_SUCCESS);
    }
    else if ( hHttpServer->settings.requestReceivedCallback.callback )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Request (method %s) Received on hHttpServerSocket %p, Setting up for Deferred Callback"
                    BIP_MSG_PRE_ARG, pMethodName, (void *)hHttpServerSocket ));

        /* and caller wants a request receivedCallback, so queue it up and invoke it later below via DoDeferred CB */
        BIP_Arb_AddDeferredCallback( hHttpServer->recvRequestApi.hArb, &hHttpServer->settings.requestReceivedCallback);
    }
    else
    {
        /* BIP Bug: Shouldn't happen, we should either have API pending or callback registered. */
        BDBG_ASSERT(NULL);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : EXIT <-----"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
} /* processHttpServer_ServerSocket_ReceivedRequest */

static bool isHttpStreamerValid(
    BIP_HttpServerHandle    hHttpServer,
    BIP_HttpStreamerHandle  hNewHttpStreamer
    )
{
    BIP_HttpStreamerHandle hHttpStreamer;

    for (
         hHttpStreamer = BLST_Q_FIRST( &hHttpServer->httpStreamerListHead );
         hHttpStreamer;
         hHttpStreamer = BLST_Q_NEXT( hHttpStreamer, httpStreamerListNext )
        )
    {
        if ( hHttpStreamer == hNewHttpStreamer ) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpStreamer=%p is valid!" BIP_MSG_PRE_ARG, (void *)hHttpStreamer ));
            return true;
        }
    }

    return false;
} /* isHttpStreamerValid */

/* HttpServerSocket State Machine to Start Receiving a Request and complete it! */
void processHttpServer_ServerSocket_RecvRequest(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSocketHandle hHttpServerSocket
    )
{
    BIP_Status brc = BIP_INF_IN_PROGRESS;
    BIP_Status completionStatus = BIP_INF_IN_PROGRESS;

    BDBG_ASSERT(hHttpServer);
    BDBG_ASSERT(hHttpServerSocket);
    if (!hHttpServer || !hHttpServerSocket)
        return;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : ENTER ----->"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));

    if ( hHttpServerSocket->state >= BIP_HttpServerSocketState_eWaitingForRecvRequestApi )
    {
        /* we are have already received the request, so nothing to do in the receive state! */
        goto out;
    }

    /*
     * Carry out the recv state processing of a particular httpServerSocket object:
     * After HttpServerSocket object is created (during the connectedCB from listener),
     * we attempt to receive request from httpSocket. If we dont have a request available,
     * out initial state is changed to waitingForRequest.
     * At this time, we enable reqRcvdCB from HttpSocket and wait for it to notify us
     * when full request is available.
     *
     * HttpSocket only fires this call for two cases:
     *  -Full Request Msg is received, notify app about it (either via Arb or CB) or
     *  -An error happens on the HttpSocket (Destroy HttpServerSocket).
     * Calling HttpSocket_RecvRequest will give this picture!
     */
    if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eUninitialized )
    {
        /* Add httpSocket object to the inUseList so that we can track them. */
        BLST_Q_INSERT_TAIL( &hHttpServer->httpServerSocketInUseListHead, hHttpServerSocket, httpServerSocketInUseListNext );

        /*
         * Create BIP_HttpSocket object.
         * Note: HttpRequest & HttpResponse objects are internally allocated by HttpSocket and owned by it.
         */
        {
            BIP_HttpSocketCreateSettings settings;

            BIP_HttpSocket_GetDefaultCreateSettings( &settings );
            settings.persistentConnectionTimeoutInMs = hHttpServer->startSettings.persistentConnectionTimeoutInMs;
            hHttpServerSocket->hHttpSocket = BIP_HttpSocket_CreateFromBipSocket( hHttpServerSocket->hSocket, &settings );
            BIP_CHECK_GOTO(( hHttpServerSocket->hHttpSocket != NULL ), ( "BIP_HttpSocket_CreateFromBipSocket Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, completionStatus );
        }

        {
            BIP_SocketStatus socketStatus;

            BIP_Socket_GetStatus( hHttpServerSocket->hSocket, &socketStatus);
            hHttpServerSocket->pRemoteIpAddress = socketStatus.pRemoteIpAddress;
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket %p: Created from Peer %s" BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket, hHttpServerSocket->pRemoteIpAddress ));

        /* HttpServerSocket object is now in idle state */
        hHttpServerSocket->state = BIP_HttpServerSocketState_eIdle;
    }

    if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eIdle )
    {
        hHttpServerSocket->state = BIP_HttpServerSocketState_eWaitingForRequestArrival;
        completionStatus = BIP_INF_IN_PROGRESS;
    }

    if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eWaitingForRequestArrival )
    {
        /*
         * Try to receive a Request in non-blocking mode, if we dont get the request,
         * we rely in the BIP_HttpSocket to give us a requestRecvdCallback to indicate
         * its completion or an error (if that happened).
         */
        BIP_Status brc;
        BIP_HttpSocketRecvRequestSettings recvRequestSettings;

        BIP_HttpSocket_GetDefaultRecvRequestSettings( &recvRequestSettings );
        recvRequestSettings.timeoutInMs = 0; /* non-blocking mode */

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpSocket %p: Calling BIP_HttpSocket_RecvRequest in non-blocking mode"
                    BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpSocket));

        brc = BIP_HttpSocket_RecvRequest( hHttpServerSocket->hHttpSocket, &hHttpServerSocket->hHttpRequest, &hHttpServerSocket->hHttpResponse, &recvRequestSettings );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpSocket %p: Calling BIP_HttpSocket_RecvRequest BIP_HttpSocket_RecvRequest returned %s"
                    BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpSocket, BIP_StatusGetText(brc)));

        if (brc == BIP_INF_TIMEOUT)
        {
            /*
             * Non-blocking BIP_HttpSocket_RecvRequest() returned TIMEOUT status
             * which means that full request message is not yet available! We will wait for the callback to come.
             */
        }
        else
        {
            /* We either got an error or Success, thus are done from Receiving. */
            completionStatus = brc;
        }
    }

error:
    /* Check for completion of receiving state! */
    if ( completionStatus == BIP_SUCCESS )
    {
        const char *pUrl;
        BIP_HttpStreamerHandle hHttpStreamer = NULL;
        BIP_HttpRequestMethod method;
        const char *pMethodName = NULL;


        /* We have received request on this ServerSocket, increment count to reflect the number of received requests. */
        hHttpServerSocket->stats.numReceivedRequests++;

        /*
         * Now determine if this new request belongs to an existing HttpStreamer. This will be
         * true for HTTP Adaptive Streaming Protocols (like HLS & DASH). If we determine this
         * to be the case, Request is handed over to the HttpStreamer for processing.
         *
         * Otherwise, we queue the request up in the RequestRcvdList.
         * Then we run the request complete logic to determine if we need to complete an Arb,
         * issue app a callback, or just queue up the request until app calls _RecvRequest().
         */

        completionStatus = BIP_HttpRequest_GetTarget( hHttpServerSocket->hHttpRequest, &pUrl);
        if ( completionStatus == BIP_SUCCESS )
        {
            BIP_HttpStreamer_GetStreamerIdFromUrl( pUrl, (unsigned *)&hHttpStreamer );
        }

        completionStatus = BIP_HttpRequest_GetMethod(hHttpServerSocket->hHttpRequest, &method, &pMethodName);

        if ( hHttpStreamer && isHttpStreamerValid( hHttpServer, hHttpStreamer ) == true )
        {
            /* HttpStreamer handle address was embedded in the incoming URL, so the request belongs to a HttpStreamer object. */
            BIP_HttpStreamerProcessRequestSettings requestSettings;
            BIP_CallbackDesc                       requestProcessedCallback;

            BIP_HttpStreamer_GetDefaultProcessRequestSettings( &requestSettings );
            requestSettings.hHttpRequest = hHttpServerSocket->hHttpRequest;
            requestProcessedCallback.callback = requestProcessedCallbackFromHttpStreamer;
            requestProcessedCallback.context = hHttpServerSocket;
            completionStatus = BIP_HttpStreamer_ProcessRequest( hHttpStreamer, hHttpServerSocket->hHttpSocket, &requestProcessedCallback, &requestSettings );
            if (completionStatus == BIP_SUCCESS)
            {
                hHttpServerSocket->state = BIP_HttpServerSocketState_eProcessingRequest;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpStreamer %p: _ProcessRequest() is submitted for URL %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpStreamer, pUrl ));
            }
            else
            {
                /* Streamer failed to process the message, so we are done (as it already sends an error response). */
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpStreamer %p: ProcessRequest Failed: completionStatus: %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpStreamer, BIP_StatusGetText(completionStatus) ));

                hHttpServer->stats.numRejectRequests++;
                /* Since there was no payload associated with this response, we go back to receiving next request! */
                hHttpServerSocket->state = BIP_HttpServerSocketState_eIdle;
            }
        }
        else
        {
            /*
             * Request doesn't belong to an existing HttpStreamer object and thus needs to be given to the App.
             *
             * Cache the ServerSocket object pointer in the Request object.
             * NOTE: This is used to de-reference the ServerSocket object from the
             * Request object in the HttpServer_RejectRequest or HttpServer_StartStreamer APIs.
             */
            completionStatus = BIP_HttpRequest_SetUserData(hHttpServerSocket->hHttpRequest, hHttpServerSocket);
            if (completionStatus == BIP_SUCCESS)
            {
                /* Add this ServerSocket to a list that is used by HttpServer_RecvRequest() to receive requests from */
                BLST_Q_INSERT_TAIL( &hHttpServer->httpServerSocketRcvdRequestHead, hHttpServerSocket, httpServerSocketRcvdRequestNext );
                hHttpServer->stats.numConcurrentRequestsQueued++;

                /* Debug print. */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p: Request (Method %s) Received & Inserted in RcvdRequest List!"
                                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , pMethodName));

                /* Change state to reflect the request is received! */
                hHttpServerSocket->state = BIP_HttpServerSocketState_eWaitingForRecvRequestApi;

                /* Run the ReceivedRequest state to see if we can complete any Arbs or notify app if requested via callback. */
                processHttpServer_ServerSocket_ReceivedRequest( hHttpServer );
            }
        }
    }
    if ( completionStatus == BIP_INF_IN_PROGRESS )
    {
        /*
         * Request not available at this time, stay in this state until
         * we receive a full request and get the callback,
         * client closes or network error happens, or
         * persistent connection timeout expires in HttpSocket and we get a callback from it.
         */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: HttpRequest not available at this time"
                    BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    }
    else if ( completionStatus != BIP_SUCCESS)
    {
        /*
         * All other return cases point to an error on HttpSocket.
         * Either peer closed the socket before sending the full Request
         * or keepalive timer expiered before request was received.
         *
         */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: BIP_HttpSocket_RecvRequest returned error, status: %s"
                    BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state), BIP_StatusGetText(completionStatus) ));

        /* Change state to reflect the error state! */
        hHttpServerSocket->state = BIP_HttpServerSocketState_eDestroying;
    }

    /*
     ***************************************************************************************************************
     * Check if we should enable or disable callbacks from BIP_HttpSocket.
     ***************************************************************************************************************
     */
    {
        bool updateSettings = false;
        BIP_HttpSocketSettings httpSocketSettings;

        if ( hHttpServerSocket->requestReceivedCallbackState == BIP_HttpSocketCallbackState_eDisabled )
        {
            /* Callback is currently disabled, so we need to enable it only if we are going to be waiting for requests. */
            if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eWaitingForRequestArrival)
            {
                BIP_HttpSocket_GetSettings( hHttpServerSocket->hHttpSocket, &httpSocketSettings );
                httpSocketSettings.requestReceivedCallback.callback = requestReceivedCallbackFromHttpSocket;
                httpSocketSettings.requestReceivedCallback.context = hHttpServerSocket;
                updateSettings = true;
                hHttpServerSocket->requestReceivedCallbackState = BIP_HttpSocketCallbackState_eEnabled;
            }
        }
        else
        {
            /* Callback is current enabled, check if it needs to be disabled */
            if ( hHttpServerSocket->state != BIP_HttpServerSocketState_eWaitingForRequestArrival)
            {
                /*
                 * Now that we are no longer waiting for a request and thus must have received a full request,
                 * we want to handle this request first. So we tell HttpSocket object
                 * to not send us anymore of these reqRcvdCallbacks until we are ready for the next one.
                 */
                BIP_HttpSocket_GetSettings( hHttpServerSocket->hHttpSocket, &httpSocketSettings );
                httpSocketSettings.requestReceivedCallback.callback = NULL;
                updateSettings = true;
                hHttpServerSocket->requestReceivedCallbackState = BIP_HttpSocketCallbackState_eDisabled;
            }
        }
        if (updateSettings)
        {
            brc = BIP_HttpSocket_SetSettings( hHttpServerSocket->hHttpSocket, &httpSocketSettings );
            if (brc == BIP_SUCCESS)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p: BIP_HttpSocket Settings for Callbacks: httpServerSocketState %s, reqRcvdCallback %s, updated %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state),
                            hHttpServerSocket->requestReceivedCallbackState ? "Enabled":"Disabled", updateSettings? "Yes":"No"));
            }
            else
            {
                BDBG_ERR((BIP_MSG_PRE_FMT "hHttpServerSocket %p: BIP_HttpSocket_SetSettings Failed for hHttpSocket %p" BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpSocket));
            }
        }
    }
out:
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : EXIT  <-----"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    return;
} /* processHttpServer_ServerSocket_RecvRequest */

static void processHttpServer_ServerSocketState_Streaming(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSocketHandle hHttpServerSocket
    )
{
    BIP_Status completionStatus = BIP_SUCCESS;

    BDBG_ASSERT(hHttpServer);
    BDBG_ASSERT(hHttpServerSocket);

    if (!hHttpServer || !hHttpServerSocket)
        return;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: ENTER ------->"
                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    if ( BIP_Arb_IsBusy(hHttpServer->startStreamerApi.hArb) == true )
    {
        /* App has called BIP_HttpServer_StartStreamer(), lets do its processing! */
        if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eWaitingForStartStreamerApi )
        {
            /* We are in the right state to start streaming, lets do so! */
            /* TODO: For now assuming, a blocking mode for this but TODO to extend to support other modes! */
            BIP_HttpStreamerStartSettings streamerStartSettings;

            BIP_HttpStreamer_GetDefaultStartSettings( &streamerStartSettings );
            streamerStartSettings = hHttpServer->startStreamerApi.settings.streamerStartSettings;
            streamerStartSettings.hInitDtcpIp = hHttpServer->hDtcpIpServer;

            completionStatus = BIP_HttpStreamer_Start( hHttpServerSocket->hHttpStreamer, &streamerStartSettings );
            if (completionStatus == BIP_SUCCESS)
            {
                /*
                 * We have started the streamer. Now send it 1st request to process.
                 * We provide the Request, Response, and HttpSocket to use for streaming media.
                 */
                BIP_HttpStreamerProcessRequestSettings requestSettings;
                BIP_CallbackDesc                       requestProcessedCallback;

                BIP_HttpStreamer_GetDefaultProcessRequestSettings( &requestSettings );
                requestSettings.hHttpRequest = hHttpServerSocket->hHttpRequest;
                requestProcessedCallback.callback = requestProcessedCallbackFromHttpStreamer;
                requestProcessedCallback.context = hHttpServerSocket;
                completionStatus = BIP_HttpStreamer_ProcessRequest( hHttpServerSocket->hHttpStreamer, hHttpServerSocket->hHttpSocket, &requestProcessedCallback, &requestSettings );
                if (completionStatus == BIP_SUCCESS)
                {
                    hHttpServerSocket->state = BIP_HttpServerSocketState_eProcessingRequest;
                    hHttpServer->stats.numStartedStreamers++;
                    hHttpServer->stats.numSentResponses++;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpStreamer %p: Streamer is Started: 1st Request is handed over!"
                                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpStreamer ));
                }
                else
                {
                    /* Streamer failed to process the 1st message, we treat this as soft error and let app send response via RejectRequest(). */
                    BDBG_WRN(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpStreamer %p: ProcessRequest Failed: completionStatus: %s"
                                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpStreamer, BIP_StatusGetText(completionStatus) ));
                    hHttpServerSocket->state = BIP_HttpServerSocketState_eWaitingForStartStreamerApi;
                }
            }
            else
            {
                /* Failed to start the streamer, we are done! */
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, hHttpStreamer %p: Failed to Start Streamer: completionStatus: %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpServerSocket, (void *)hHttpServerSocket->hHttpStreamer, BIP_StatusGetText(completionStatus) ));
            }
        }
        else
        {
            /* We are not in the right state to start streaming! */
            BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpServer %p: hHttpServerSocket %p: Can't call BIP_HttpServer_StartStreaming() in this state: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket, BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
            completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
        }
#if 0
        /* TODO: look into this when async support is added to the streamer. */
        if (hHttpServer->startStreamerApi.pSettings->api.pAsyncStatus)
            *hHttpServer->startStreamerApi.pSettings->api.pAsyncStatus = completionStatus;
#endif

        /* Complete the ARB */
        BIP_Arb_CompleteRequest( hHttpServer->startStreamerApi.hArb, completionStatus);
    }

    /* App hasn't called _StartStreamer, _RejectRequest APIs. */
    /* If we are currently processing a request and had gotten a callback from httpStreamer indicating it has finished processing the request, */
    /* we complete the HttpServerSocket state processing. */
    if ( hHttpServerSocket->state == BIP_HttpServerSocketState_eProcessingRequest && hHttpServerSocket->requestProcessed == true)
    {
        /*
         * We have gotten the requestProcessed callback from HttpStreamer indicating it has processed the request.
         * So either streamer finished because of EOF/NetworkError due to client channel change, or
         * App had issued a _StopStreamer() to the HttpStreamer which made it invoke requestProcessed callback to us.
         *
         * In that case, we switch the HttpSocket back to the Idle state so that we can either
         * receive & process next request on it or handle the error case.
         */

        /* Set HttpSocket back to Idle state, so that we can receive either the next request or error condition on it. */
        BIP_HttpSocket_SetStateToIdle_priv( hHttpServerSocket->hHttpSocket );

        /* Reset the request & response objects to their original state incase it is being re-used. */
        BIP_HttpRequest_Clear(hHttpServerSocket->hHttpSocket->hHttpRequest, NULL);
        BIP_HttpResponse_Clear(hHttpServerSocket->hHttpSocket->hHttpResponse, NULL);

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: reset to Idle State"
                    BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
        hHttpServerSocket->requestProcessed = false;
        hHttpServerSocket->state = BIP_HttpServerSocketState_eIdle;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServerSocket %p, current state %s: completionStatus: %s EXIT <-------"
                BIP_MSG_PRE_ARG, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state), BIP_StatusGetText(completionStatus) ));

    return;
} /* processHttpServer_ServerSocketState_Streaming */

/* HttpServerSocket State Machine */
void processHttpServerState_ServerSocket(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSocketHandle hHttpServerSocket
    )
{
    BDBG_ASSERT(hHttpServer);
    BDBG_ASSERT(hHttpServerSocket);
    if (!hHttpServer || !hHttpServerSocket)
        return;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : ENTER ----->"
                BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));

    /*
     * Process different states of the given ServerSocket. We process receive state last
     * as streaming or send response states may be complete and thus we may need to
     * resume on the receive side processing.
     * E.g. After we send out HTTP Response for HEAD Request (no payload), we want to
     * resume the receive state processing to receive the next request.
     *
     * Note: we only run ServerSocket state machine if the HttpServer is not being destroyed
     * otherwise, app is shutting down the server.
     */
    if ( BIP_Arb_IsBusy(hHttpServer->destroyApi.hArb) == false )
    {
        processHttpServer_ServerSocketState_Streaming( hHttpServer, hHttpServerSocket );
        processHttpServer_ServerSocketState_RejectRequest( hHttpServer, hHttpServerSocket );
        processHttpServer_ServerSocket_RecvRequest( hHttpServer, hHttpServerSocket );
    }
    else
    {
        /*
         * App is Destroying the HttpServer. Either App would have directly Destroyed the associated HttpStreamers
         * by calling BIP_HttpServer_DestroyStreamer(), otherwise in the httpServerDestroy(), we would have
         * called BIP_HttpStreamer_Destroy() for all current streamers which will issue any
         * pending requestProcessed Callback to this HttpServerSocket and thus make its state as idle.
         *
         * In all states of HttpServerSocket, there is no specific action other than just freeing up the resources
         * like HttpSocket, HttpRequest, etc.
         *
         * That is why we dont need to run thru the ServerSocket state machine and we directly destroy it.
         */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : ServerDestory case: Changing to Destorying state"
                    BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
        hHttpServerSocket->state = BIP_HttpServerSocketState_eDestroying;
    }

    if (hHttpServerSocket->state == BIP_HttpServerSocketState_eDestroying)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : Destroying ServerSocket object & EXIT  <-----"
                    BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
        BIP_HttpServerSocket_Destroy(hHttpServer, hHttpServerSocket);
    }
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpServer %p, hHttpServerSocket:state %p: %s : EXIT  <-----"
                    BIP_MSG_PRE_ARG, (void *)hHttpServer, (void *)hHttpServerSocket , BIP_HTTP_SERVER_SOCKET_STATE(hHttpServerSocket->state) ));
    }
} /* processHttpServerState_ServerSocket */
