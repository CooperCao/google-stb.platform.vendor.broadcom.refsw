/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bip_http_socket_impl.h"

BDBG_MODULE( bip_http_socket );
BDBG_OBJECT_ID_DECLARE( BIP_HttpSocket );

/* Descriptive names of HTTP States */
struct BIP_HttpSocketStateNames
{
    BIP_HttpSocketState state;
    char *pStateName;
}gHttpSocketState[] = {
    {BIP_HttpSocketState_eUninitialized, "UnInitialized"},
    {BIP_HttpSocketState_eIdle, "Idle"},
    {BIP_HttpSocketState_eReceiveNewRequest, "ReceiveNewRequest"},
    {BIP_HttpSocketState_eReceivingRequest, "ReceivingRequest"},
    {BIP_HttpSocketState_eReceivingRequestDone, "ReceivingRequestDone"},
    {BIP_HttpSocketState_eReceivedRequest, "ReceivedRequest"},
    {BIP_HttpSocketState_eReceiveRequestTimedout, "ReceiveRequestTimedout"},
    {BIP_HttpSocketState_eReadyToSendResponse, "ReadyToSendResponse"},
    {BIP_HttpSocketState_eSendNewResponse, "SendNewResponse"},
    {BIP_HttpSocketState_eSendingResponse, "SendingResponse"},
    {BIP_HttpSocketState_eSendingResponseDone, "SendingResponseDone"},
    {BIP_HttpSocketState_eSentResponse, "SentResponse or ReadyToSendPayload"},
    {BIP_HttpSocketState_eSendResponseTimedout, "SendResponseTimedout"},
    {BIP_HttpSocketState_eSendNewPayload, "SendNewPayload"},
    {BIP_HttpSocketState_eSendingPayload, "SendingPayload"},
    {BIP_HttpSocketState_eSendingPayloadDone, "SendingPayloadDone"},
    {BIP_HttpSocketState_eSentPayload, "SentPayload"},
    {BIP_HttpSocketState_eSendPayloadTimedout, "SendPayloadTimedout"},
    {BIP_HttpSocketState_eSendingRequest, "SendingRequest"},
    {BIP_HttpSocketState_eSentRequest, "SentRequest"},
    {BIP_HttpSocketState_eReceiveNewResponse, "ReceiveNewResponse"},
    {BIP_HttpSocketState_eReceivingResponse, "ReceivingResponse"},
    {BIP_HttpSocketState_eReceivedResponse, "ReceivedResponse or ReadyToReceivePayload"},
    {BIP_HttpSocketState_eReceiveNewPayload, "ReceiveNewPayload"},
    {BIP_HttpSocketState_eReceivingPayload, "ReceivingPayload"},
    {BIP_HttpSocketState_eReceivedPayload, "ReceivedPayload"},
    {BIP_HttpSocketState_eError, "Error"},
    {BIP_HttpSocketState_eDestroying, "Destroying"},
    {BIP_HttpSocketState_eMax, "MaxState"}
};

#define BIP_HTTP_SOCKET_STATE(state) \
    gHttpSocketState[state].pStateName

/* Forward declaration of state processing function */
void processHttpState( void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin);

static void dataReadyCallbackFromBipSocket (
    void *context,
    int   param
    )
{
    BIP_HttpSocketHandle hHttpSocket = context;
    BSTD_UNUSED(param);

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
    processHttpState(hHttpSocket, param, BIP_Arb_ThreadOrigin_eBipCallback);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

static void writeReadyCallbackFromBipSocket (
    void *context,
    int   param
    )
{
    BIP_HttpSocketHandle hHttpSocket = context;
    BSTD_UNUSED(param);

    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket );
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
    processHttpState(hHttpSocket, param, BIP_Arb_ThreadOrigin_eBipCallback);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

static void processHttpStateFromTimerCallback_RecvApi(
    void *pContext
    )
{
    BIP_HttpSocketHandle    hHttpSocket = (BIP_HttpSocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    B_Mutex_Lock(hHttpSocket->hStateMutex);
    if (hHttpSocket->recv.hApiTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
        hHttpSocket->recv.hApiTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock(hHttpSocket->hStateMutex);

    processHttpState( hHttpSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

void processHttpStateFromTimerCallback_InitialRequest(
    void *pContext
    )
{
    BIP_HttpSocketHandle    hHttpSocket = (BIP_HttpSocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    B_Mutex_Lock(hHttpSocket->hStateMutex);
    if (hHttpSocket->recv.hInitialRequestTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
        hHttpSocket->recv.hInitialRequestTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock(hHttpSocket->hStateMutex);

    processHttpState( hHttpSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

void processHttpStateFromTimerCallback_PersistentConnection(
    void *pContext
    )
{
    BIP_HttpSocketHandle    hHttpSocket = (BIP_HttpSocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    B_Mutex_Lock(hHttpSocket->hStateMutex);
    if (hHttpSocket->recv.hPersistentConnectionTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
        hHttpSocket->recv.hPersistentConnectionTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock(hHttpSocket->hStateMutex);

    processHttpState( hHttpSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

static void processHttpStateFromTimerCallback_Send(
    void *pContext
    )
{
    BIP_HttpSocketHandle    hHttpSocket = (BIP_HttpSocketHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));

    B_Mutex_Lock(hHttpSocket->hStateMutex);

    if (hHttpSocket->send.hTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Got BIP_Timer callback, marking timer as self-destructed" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
        hHttpSocket->send.hTimer = NULL;   /* Indicate timer not active. */
    }

    B_Mutex_Unlock(hHttpSocket->hStateMutex);

    processHttpState( hHttpSocket, 0, BIP_Arb_ThreadOrigin_eTimer);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p: <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

static BIP_Status updateDtcpIpRangeHeaders(
    BIP_HttpSocketHandle hHttpSocket
    )
{
    BIP_Status brc;
    const char *pDtcpIpRangeHeaderName = "Range.dtcp.com";
    const char *pRangeHeaderName = "Range";
    BIP_HttpHeaderHandle hHeader = NULL;
    BIP_HttpRequestHandle hRequest = NULL;
    const char *pValue = NULL;

    BDBG_ASSERT(hHttpSocket->hHttpRequest);

    hRequest = hHttpSocket->hHttpRequest;

    /* First check whether Range.dtcp.com headers exist, if yes then replace the header name with Range. */
    brc = BIP_HttpRequest_GetNextHeader(hRequest, NULL, pDtcpIpRangeHeaderName, &hHeader, NULL);
    BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );

    if( brc == BIP_SUCCESS)
    {
        /* first get any existing Range header */
        brc = BIP_HttpRequest_GetNextHeader(hRequest, NULL, pRangeHeaderName, &hHeader, NULL);
        BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );

        while( brc == BIP_SUCCESS )
        {
            BIP_HttpRequest_RemoveHeader( hRequest, hHeader);

            /* Now check if there is any more header exist with name Range*/
            brc = BIP_HttpRequest_GetNextHeader(hRequest, NULL, pRangeHeaderName, &hHeader, NULL);
            BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );
        }

        /* Now get all existing Range.dtcp.com header and replace them with header name Range.*/
        brc = BIP_HttpRequest_GetNextHeader(hRequest, NULL, pDtcpIpRangeHeaderName, &hHeader, &pValue);
        BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );

        while( brc == BIP_SUCCESS )
        {
            BIP_HttpHeaderHandle hNewHeader = NULL;

            hNewHeader = BIP_HttpRequest_AddHeader( hRequest, pRangeHeaderName, pValue, hHeader );
            BIP_CHECK_GOTO(( hNewHeader ), ( "BIP_HttpRequest_AddHeader Failed" ), error, brc, brc );

            /* Now remove the Range.dtcp.com.*/
            BIP_HttpRequest_RemoveHeader( hRequest, hHeader);

            /* Now check if there is any more header exist with name Range.dtcp.com*/
            brc = BIP_HttpRequest_GetNextHeader(hRequest, NULL, pDtcpIpRangeHeaderName, &hHeader, &pValue);
            BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );
        }
    }
error:
    return ( brc );
}

static BIP_Status addHttpRsvdHeaders(
    BIP_HttpResponseHandle hHttpResponse,
    int64_t messageLength,
    bool enableHttpChunkXferEncoding,
    bool disablePersistentConnection
    )
{
    BIP_Status bipStatus = BIP_SUCCESS;
    const char *acceptRangeHeaderValue;
    BIP_HttpHeaderHandle hHeader;

    if ( messageLength > 0 )
    {
        char tempString[22];/* (2^64 - 1) = 18446744073709551615 -> 20 digits, one for sign, one for '\0' */

        snprintf(tempString, sizeof(tempString), "%lld", (long long)messageLength);/*TODO:Later this will be part of custom apis.*/

        hHeader = BIP_HttpResponse_AddHeader(hHttpResponse , "Content-Length", tempString, NULL);
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Check if we can support Accept-Range header for this content and add that. */
    if ( messageLength > 0 )
    {
        /* For streams whose content length is known, we can support Range Requests. */
        acceptRangeHeaderValue = "bytes";
    }
    else
    {
        /* Live or Transcoded content, so can't support Range. */
        acceptRangeHeaderValue = "none";
    }

    if (enableHttpChunkXferEncoding)
    {
        hHeader = BIP_HttpResponse_AddHeader(hHttpResponse , "Transfer-Encoding", "chunked", NULL);
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    hHeader = BIP_HttpResponse_AddHeader(hHttpResponse , "Accept-Ranges", acceptRangeHeaderValue, NULL);
    BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

    if ( disablePersistentConnection == true )
    {
        /* Set Connection header to close indicates to the client that Persistent Connection is not supported. */
        hHeader = BIP_HttpResponse_AddHeader(hHttpResponse , "Connection", "close", NULL);
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

error:
    return ( bipStatus );
}

static BIP_Status deserializeFromBuffer(
    BIP_HttpSocketHandle hHttpSocket,
    bool                 *pDeserializeComplete
    )
{
    /* read some data, check for state completion! */
    size_t  consumedBytes = 0;
    BIP_Status brc = BIP_ERR_INTERNAL;

    /* deserialize incoming data to populate request object from these incoming bytes and also see if we got all Request Headers. */
    brc = BIP_HttpRequest_DeserializeFromBuffer(
            hHttpSocket->hHttpRequest,
            &(hHttpSocket->pNetworkReadBuffer[hHttpSocket->nwRdBuffOffset]),
            hHttpSocket->unconsumedDataSize,
            pDeserializeComplete,
            &consumedBytes
            );
    BIP_CHECK_GOTO(( brc == BIP_SUCCESS ), ( "BIP_HttpRequest_DeserializeFromBuffer Failed"), error, brc, hHttpSocket->recv.completionStatus );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, Read %zu bytes of Request Header, DeserializeComplete %s"
                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->unconsumedDataSize, *pDeserializeComplete?"Y":"N" ));
    /* check if deserialization from buffer to HttpRequest object is done, i.e whether we got a complete Http request header */
    if (*pDeserializeComplete == true)
    {
        {
            bool printEnabled = false;
            /* The following two prints will set the printEnabled flag if the BDBG module is enabled. */
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "Received HTTP request:" BIP_HTTP_SOCKET_PRINTF_FMT
                              BIP_MSG_PRE_ARG, BIP_HTTP_SOCKET_PRINTF_ARG(hHttpSocket)));
            BIP_MSG_SUM(( BIP_MSG_PRE_FMT "Printing HTTP request: %p %s"
                              BIP_MSG_PRE_ARG, (void *)hHttpSocket->hHttpRequest, (true?(printEnabled=true,""):"") ));
            if (printEnabled) {
                brc = BIP_HttpRequest_Print( hHttpSocket->hHttpRequest, NULL, NULL);
                BIP_CHECK_GOTO(( brc == BIP_SUCCESS ), ( "BIP_HttpRequest_Print Failed"), error, brc, hHttpSocket->recv.completionStatus );
            }
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, received all headers in Request, saved %zu bytes of unconsumedBufferLength for next request"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->unconsumedDataSize ));
    }

    /* Update nwRdBuffOffset.*/
    hHttpSocket->nwRdBuffOffset += consumedBytes;
    /* update unconsumedDataSize;*/
    hHttpSocket->unconsumedDataSize -= consumedBytes;

error:
    return brc;
}

static void processHttpRecvState(
    BIP_HttpSocketHandle hHttpSocket,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    bool                    reRunProcessState;
    BIP_SocketSettings      socketSettings;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hHttpSocket);
    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket);
    B_MUTEX_ASSERT_LOCKED(hHttpSocket->hStateMutex);

    reRunProcessState = true;
    if ( hHttpSocket->httpState != BIP_HttpSocketState_eError )
        /* dont reset the completionStatus if we are Error state, this way we can convey the actual status to caller! */
        hHttpSocket->recv.completionStatus = BIP_INF_IN_PROGRESS;
    while (reRunProcessState)
    {
        reRunProcessState = false;
        /*
         ***************************************************************************************************************
         * First, we check API Arbs to see if we need to carry out any relevant state processing.
         ***************************************************************************************************************
         */
        if (BIP_Arb_IsNew(hArb = hHttpSocket->recvRequestApi.hArb))
        {
            /* Caller has issued _RecvRequest API, make sure if caller is allowed to invoke it in the current state */
            if (hHttpSocket->httpState != BIP_HttpSocketState_eIdle &&
                hHttpSocket->httpState != BIP_HttpSocketState_eReceivingRequest &&
                hHttpSocket->httpState != BIP_HttpSocketState_eReceivedRequest)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_RejectRequest(): _RecvRequest Api not allowed in this httpState: %s "
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                if ( hHttpSocket->httpState != BIP_HttpSocketState_eError )
                    /* dont override the completionStatus if we are Error state, this way we can convey the actual status to caller! */
                    hHttpSocket->recv.completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpSocket->recv.completionStatus);
            }
            else
            {
                /*
                 * We allow _RecvRequest API in the these states:
                 *  -BIP_HttpSocketState_eIdle: requestReceivedCallback is not enabled and caller issues _RecvRequest API.
                 *  -BIP_HttpSocketState_eReceivingRequest: requestReceivedCallback was enabled & now caller issues _RecvRequest API.
                 *  -BIP_HttpSocketState_eReceivedRequest: requestReceivedCallback was enabled, full request header was received before caller issues _RecvRequest API.
                 */
                BIP_Arb_AcceptRequest(hArb);
                hHttpSocket->recv.completionStatus = BIP_INF_IN_PROGRESS;

                if (hHttpSocket->httpState != BIP_HttpSocketState_eReceivedRequest)    /* timer is only started if we haven't already received a request */
                {
                    /* Note the start time: it is useful for the non-blocking invocation where timeout is 0 and thus its a one shot call! */
                    B_Time_Get(&hHttpSocket->recv.apiStartTime);

                    /* Check if we need to enable the timer logic. */
                    if (hHttpSocket->recvRequestApi.settings.timeoutInMs >= 0)
                    {
                        BIP_TimerCreateSettings timerCreateSettings;
                        /* timer logic check is active for non-blocking (== 0) or actual timeout cases (timeout > 0) */
                        hHttpSocket->recv.apiTimerActive = true;

                        if (hHttpSocket->recvRequestApi.settings.timeoutInMs > 0)
                        {
                            /* setup a timer callback only for +ve timeout value */
                            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Starting timer for %d ms" BIP_MSG_PRE_ARG, (void *)hHttpSocket, hHttpSocket->recvRequestApi.settings.timeoutInMs ));
                            timerCreateSettings.input.callback    = processHttpStateFromTimerCallback_RecvApi;
                            timerCreateSettings.input.pContext    = hHttpSocket;
                            timerCreateSettings.input.timeoutInMs = hHttpSocket->recvRequestApi.settings.timeoutInMs;
                            hHttpSocket->recv.hApiTimer = BIP_Timer_Create(&timerCreateSettings);
                            BIP_CHECK_GOTO(( hHttpSocket->recv.hApiTimer ), ( "BIP_Timer_Create Failed"), error, BIP_ERR_INTERNAL, hHttpSocket->recv.completionStatus );
                        }
                    }
                }

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Accepted _RecvRequest Arb request: httpState %s!"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
            }

            /* Fall thru to continue with the state processing! */
            /* We may receive some data part of this Arb request and then accordingly update the BIP Socket Callbacks. */
        } /* New recvRequestApi Arb */

        /*
         ***************************************************************************************************************
         * Now walk thru various recv related states and carry out the appropriate processing.
         ***************************************************************************************************************
         */

        if ( hHttpSocket->httpState == BIP_HttpSocketState_eIdle )
        {
            if ( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eArmed ||
                    BIP_Arb_IsBusy( hHttpSocket->recvRequestApi.hArb )
               )
            {
                /*
                 * We are in idle state and caller has either enabled requestReceivedCallback or called _RecvRequest API,
                 * then we start the new request state processing.
                 */
                hHttpSocket->httpState = BIP_HttpSocketState_eReceiveNewRequest;
            }

            /*
             * We have finished the full HTTP Req/Res/Payload cycle on initial request and haven't yet enabled the persistentConnection timer.
             * So do so now as we are in the Idle state.
             * If persistenceConnectionTimeout interval is 0, we will immediately handle timeout below
             * just like if the timer were to expire.
             */
            if ( hHttpSocket->recv.initialRequestTimerActive == false && hHttpSocket->recv.persistentConnectionTimerActive == false )
            {
                BIP_TimerCreateSettings timerCreateSettings;

                if ( hHttpSocket->recv.persistentConnectionTimeoutInMs > 0 )
                {
                    timerCreateSettings.input.callback              = processHttpStateFromTimerCallback_PersistentConnection;
                    timerCreateSettings.input.pContext              = hHttpSocket;
                    timerCreateSettings.input.timeoutInMs           = hHttpSocket->recv.persistentConnectionTimeoutInMs;
                    hHttpSocket->recv.hPersistentConnectionTimer    = BIP_Timer_Create(&timerCreateSettings);
                    BIP_CHECK_GOTO(( hHttpSocket->recv.hPersistentConnectionTimer ), ( "BIP_Timer_Create Failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Starting PersistentConnection timer for %u ms" BIP_MSG_PRE_ARG, (void *)hHttpSocket, hHttpSocket->recv.persistentConnectionTimeoutInMs ));
                }
                B_Time_Get(&hHttpSocket->recv.persistentConnectionStartTime);
                hHttpSocket->recv.persistentConnectionTimerActive = true;
            }
        }

        if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceiveRequestTimedout )
        {
            /*
             * We had timed out while receiving a request in the previous iteration of this function (in the while loop),
             * thus we need to decide the next state we need to transition to. If caller has enabled requestReceivedCallback,
             * then we start the new request state processing even though we were run part of a RecvRequest Arb (this is done
             * so that we can receive the next request and notify the caller of the requestReceived).
             *
             * Otherwise (caller didn't specify callback), we move to the Idle state and let caller issue another _RecvRequest API.
             */
            if ( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eArmed )
                hHttpSocket->httpState = BIP_HttpSocketState_eReceiveNewRequest;
            else
                hHttpSocket->httpState = BIP_HttpSocketState_eIdle;
            hHttpSocket->recv.completionStatus = BIP_INF_IN_PROGRESS;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState: Going from ReceivedRequestTimedout to %s!, requestReceivedCallbackState %d, reRunProcessState %d"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->requestReceivedCallbackState, reRunProcessState ));
        }

        /*
         ***************************************************************************************************************
         * ReceiveNewRequest state: get ready to receive new request.
         ***************************************************************************************************************
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceiveNewRequest )
        {

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState: %s, Starting New RecvRequest Processing!"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));

            /* We switch to Receiving state */
            hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequest;

            /* Check if we had buffered some partialMsg. This can happen when we received multiple consecutive requests.*/
            if (hHttpSocket->unconsumedDataSize > 0)
            {
                bool    deserializeComplete;

                brc = deserializeFromBuffer( hHttpSocket, &deserializeComplete );
                BIP_CHECK_GOTO(( brc == BIP_SUCCESS ),
                        ( "deserializeFromBuffer Failed"), error, brc, hHttpSocket->recv.completionStatus );

                /* check if deserialization from buffer to HttpRequest object is done, i.e whether we got a complete Http request header */
                if (deserializeComplete == true)
                {
                    /* well, we had a complete request in the partialMsg, so we move to Done state */
                    hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                    hHttpSocket->recv.completionStatus = BIP_SUCCESS;

                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: had already received a complete msg in previous iteration"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
                }
                else
                {
                    /* we have some bytes but not complete header, so we continue below w/ the state machine */
                }
            }
            else
            {
                /* no partiaMsg, so continue below with the state machine */
            }
        } /* ReceiveNewRequest processing */

        /*
         ***************************************************************************************************************
         * ReceivingRequest state: try to recv some data from BIP_Socket & see if receive all request headers.
         * We will move to Done state either if get all request headers or error happens.
         ***************************************************************************************************************
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceivingRequest)
        {
            /*
             * Dont go thru the receiving state again if we had timedout in previous iteration of the while loop.
             * Otherwise, we may keep getting timeouts and thus wont come out of this loop until data is available.
             */
            ssize_t     requestBytesRead = 0;
            bool        deserializeComplete;
            BIP_SocketRecvSettings socketRecvSettings;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s" BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));

            deserializeComplete = false;
            brc = BIP_SUCCESS;
            while (!deserializeComplete && brc == BIP_SUCCESS)
            {
                /*
                 * We keep reading until we either get a completeHeader, or a read timeout, or error.
                 */
                BIP_Socket_GetDefaultSocketRecvSettings( &socketRecvSettings );
                socketRecvSettings.api.timeout = 0;     /* non-blocking call */
                socketRecvSettings.input.bytesToRead = hHttpSocket->createSettings.networkReadBufferSize; /* size of the buffer */
                socketRecvSettings.output.pBuffer = hHttpSocket->pNetworkReadBuffer; /* buffer ptr where to read the data */
                socketRecvSettings.output.pBytesRead = &requestBytesRead; /* how many bytes would be read! */

                /*
                 * Now try to read upto socketRecvSettings.input.bytesToRead bytes from network.
                 * NOTE: We do this using the non-blocking Recv on BIP_Socket.
                 * This is because we dont apriori know the length of the request headers,
                 * so we read what we can at the moment and check if we have received all request headers.
                 */
                /* go read some data */
                brc = BIP_Socket_Recv( hHttpSocket->hSocket, &socketRecvSettings);
                if (brc == BIP_SUCCESS && requestBytesRead > 0)
                {
                    /* Now requestBytesRead data is available to be consumed, so update the following since they will be used by deserializeFromBuffer function. */
                    hHttpSocket->nwRdBuffOffset = 0;
                    hHttpSocket->unconsumedDataSize = requestBytesRead;
                    brc = deserializeFromBuffer( hHttpSocket, &deserializeComplete );
                    BIP_CHECK_GOTO(( brc == BIP_SUCCESS ),
                            ( "deserializeFromBuffer Failed"), error, brc, hHttpSocket->recv.completionStatus );

                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, Read %zu bytes of Request Header, deserializeComplete %s"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), requestBytesRead, deserializeComplete?"Y":"N" ));

                    /* check if deserialization from buffer to HttpRequest object is done, i.e whether we got a complete Http request header */
                    if (deserializeComplete == true)
                    {
                        /* We just read all headers in request, so we move to Done state. */
                        hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                        hHttpSocket->recv.completionStatus = BIP_SUCCESS;
                    }
                    else
                    {
                        /* still dont have all headers in the request */
                        hHttpSocket->recv.completionStatus = BIP_INF_IN_PROGRESS;
                    }
                }
                else if (brc == BIP_INF_IN_PROGRESS || brc == BIP_INF_TIMEOUT)
                {
                    /* no data available at this time, we remain in the Receiving state and fall thru ... */
                    hHttpSocket->recv.completionStatus = BIP_INF_IN_PROGRESS;
                }
                else
                {
                    /* Otherwise, it is an error case, we move to Done state and set the error completion code. */
                    hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                    if (requestBytesRead == 0)
                    {
                        brc = hHttpSocket->recv.completionStatus = BIP_INF_END_OF_STREAM; /* 0 bytesRead means peer has closed the socket */
                        BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Socket closed: completionStatus %s"
                                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->recv.completionStatus) ));
                    }
                    else
                    {
                        hHttpSocket->recv.completionStatus = brc; /* we use the status that BIP_Socket has indicated! */
                        BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: ERROR: completionStatus %s"
                                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->recv.completionStatus) ));
                    }
                }
            } /* while !completeHeader && brc == BIP_SUCCESS */
        } /* ReceivingRequest processing */

        /*
         ***************************************************************************************************************
         * We are done processing current states. Now we check for any state completions either due to
         * -app calls Destroy API,
         * -timeout event
         * -receiving desired data (request, response, payload), or
         * -network error.
         ***************************************************************************************************************
         */
error:
        /* Check for completion by app destroying this object. */
        if ( hHttpSocket->shutdownState != BIP_HttpSocketShutdownState_eNormal )
        {
            hHttpSocket->recv.completionStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
            if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceivingRequest)
            {
                /* only change the state to done if we were receiving! */
                hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Completing Recv state due to Destroy API"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
            }
        }

        /*
         * Check for recv timeouts and set timeout to true if we have exceeded the time.
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceivingRequest && hHttpSocket->recv.apiTimerActive )
        {
            B_Time      timeNow;
            int         elapsedTimeInMs;

            B_Time_Get(&timeNow);
            if ( BIP_Arb_IsBusy( hHttpSocket->recvRequestApi.hArb ) )
            {
                /*
                 * Timer is active because of following reasons:
                 * -caller has issued a non-blocking RecvRequest API, or
                 * -caller has issued a blocking RecvRequest API w/ a timeout, or
                 * -caller has issued an async RecvRequest API w/ a timeout.
                 */
                elapsedTimeInMs = B_Time_Diff(&timeNow, &hHttpSocket->recv.apiStartTime);
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Read Timeout Check: So far: %d ms, limit is %d ms." BIP_MSG_PRE_ARG,
                            (void *)hHttpSocket, elapsedTimeInMs , hHttpSocket->recvRequestApi.settings.timeoutInMs ));

                if (elapsedTimeInMs >= hHttpSocket->recvRequestApi.settings.timeoutInMs)
                {
                    /* Note: this check will be true even when state machine has been called in the non-blocking mode */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: _RecvRequest Timedout"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                    hHttpSocket->recv.completionStatus = BIP_INF_TIMEOUT;
                    hHttpSocket->recv.apiTimerActive = false;  /* this flag enables us to not check for timeout in the 2nd loop around! */
                }
            }
        }

        /* Check if we have failed to receive a request in the initialRequest timeout interval. */
        if ( hHttpSocket->recv.initialRequestTimerActive )
        {
            B_Time      timeNow;
            int         elapsedTimeInMs;

            B_Time_Get(&timeNow);
            elapsedTimeInMs = B_Time_Diff(&timeNow, &hHttpSocket->recv.initialRequestStartTime);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: initialRequest Timeout Check: So far: %d ms, limit is %u ms." BIP_MSG_PRE_ARG,
                        (void *)hHttpSocket, elapsedTimeInMs , hHttpSocket->recv.initialRequestTimeoutInMs ));

            if ( (unsigned)elapsedTimeInMs >= hHttpSocket->recv.initialRequestTimeoutInMs )
            {
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: initialRequest Timer (%d) expired"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->recv.initialRequestTimeoutInMs));
                if ( hHttpSocket->httpState != BIP_HttpSocketState_eReceivingRequestDone )
                {
                    /* We only set the error completion status if we are not in the ReceivingRequestDone state. */
                    /* As during this timeout run, we could have either successfully recvd a request, or gotten some other error */
                    /* causing the state to become ReceivingRequestDone. */
                    hHttpSocket->recv.completionStatus = BIP_ERR_INITIAL_REQUEST_TIMEDOUT;
                    hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                }
                hHttpSocket->recv.initialRequestTimerActive = false;  /* this flag enables us to not check for timeout in the 2nd loop around! */
            }
        }

        /* Check if we have failed to receive a request in the persistentConnection timeout interval. */
        if ( hHttpSocket->recv.persistentConnectionTimerActive )
        {
            B_Time      timeNow;
            int         elapsedTimeInMs;

            B_Time_Get(&timeNow);
            elapsedTimeInMs = B_Time_Diff(&timeNow, &hHttpSocket->recv.persistentConnectionStartTime);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: persistentConnection Timeout Check: So far: %d ms, limit is %u ms." BIP_MSG_PRE_ARG,
                        (void *)hHttpSocket, elapsedTimeInMs , hHttpSocket->recv.persistentConnectionTimeoutInMs ));

            if ( (unsigned)elapsedTimeInMs >= hHttpSocket->recv.persistentConnectionTimeoutInMs )
            {
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: persistentConnection Timer (%d) expired"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->recv.persistentConnectionTimeoutInMs));
                if ( hHttpSocket->httpState != BIP_HttpSocketState_eReceivingRequestDone )
                {
                    /* We only set the error completion status if we are not in the ReceivingRequestDone state. */
                    /* As during this timeout run, we could have either successfully recvd a request, or gotten some other error */
                    /* causing the state to become ReceivingRequestDone. */
                    hHttpSocket->recv.completionStatus = BIP_ERR_PERSISTENT_CONNECTION_TIMEDOUT;
                    hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
                }
                hHttpSocket->recv.persistentConnectionTimerActive = false;  /* this flag enables us to not check for timeout in the 2nd loop around! */
            }
        }

        /* If completion status is not IN_PROGRESS, then we are done w/ current state! */
        if ( hHttpSocket->recv.completionStatus != BIP_INF_IN_PROGRESS )
        {
            hHttpSocket->httpState = BIP_HttpSocketState_eReceivingRequestDone;
        }

        /*
         ***************************************************************************************************************
         * Check for previous state completion  and transition to next http state
         ***************************************************************************************************************
         */
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Transition to next state, completionStatus %s, callers' reqRcvdCB State %d"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->recv.completionStatus), hHttpSocket->requestReceivedCallbackState ));
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceivingRequestDone )
        {
            /* We have finished receiving state, so transition to next state based on the completion status */
            if (hHttpSocket->recv.completionStatus == BIP_SUCCESS)
            {
                updateDtcpIpRangeHeaders( hHttpSocket );
                hHttpSocket->httpState = BIP_HttpSocketState_eReceivedRequest;
            }
            else if (hHttpSocket->recv.completionStatus == BIP_INF_TIMEOUT)
            {
                /*
                 * We timed out while receiving, we go to the eTimeout state. This allows to complete any inProgress Arbs.
                 * However, we will run the state machine again to transition back to either eIdle or eReceiving states.
                 * e.g. if app had enabled the newRequestCallback (thus we have enabled the dataReadyCallback),
                 * then we would need to goto ReceiveNewRequest so that a new dataReadyCallback can let the state machine run to receive the actual request.
                 */
                hHttpSocket->httpState = BIP_HttpSocketState_eReceiveRequestTimedout;
                reRunProcessState = true;
            }
            else if (hHttpSocket->recv.completionStatus == BIP_ERR_OBJECT_BEING_DESTROYED)
            {
                /* For us to be here in the ReceivingRequestDone state means that we were currently in receiving state! */
                /* So we move the Destroying state */
                hHttpSocket->httpState = BIP_HttpSocketState_eDestroying;
            }
            else /* error case */
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eError;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Changing to state: %s, completionStatus %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->recv.completionStatus) ));
            }

            if (hHttpSocket->recv.hApiTimer != NULL)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Cancelling ApiTimer " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
                BIP_Timer_Destroy(hHttpSocket->recv.hApiTimer);
                hHttpSocket->recv.hApiTimer = NULL;
                hHttpSocket->recv.apiTimerActive = false;
            }

            /* Cancel initialTimer (if running) for any status other than Recv API timeout. */
            if (hHttpSocket->recv.hInitialRequestTimer != NULL && hHttpSocket->recv.completionStatus != BIP_INF_TIMEOUT)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Cancelling InitialRequestTimer " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
                BIP_Timer_Destroy(hHttpSocket->recv.hInitialRequestTimer);
                hHttpSocket->recv.hInitialRequestTimer = NULL;
                hHttpSocket->recv.initialRequestTimerActive = false;
            }

            /* Cancel persistentConnectionTimer (if running) for any status other than Recv API timeout. */
            if (hHttpSocket->recv.hPersistentConnectionTimer != NULL && hHttpSocket->recv.completionStatus != BIP_INF_TIMEOUT)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Cancelling PersistentConnectionTimer " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
                BIP_Timer_Destroy(hHttpSocket->recv.hPersistentConnectionTimer);
                hHttpSocket->recv.hPersistentConnectionTimer = NULL;
                hHttpSocket->recv.persistentConnectionTimerActive = false;
            }
        }
        else if ( hHttpSocket->httpState == BIP_HttpSocketState_eReceivedRequest ||
                  hHttpSocket->httpState == BIP_HttpSocketState_eReceivedResponse ||
                  hHttpSocket->httpState == BIP_HttpSocketState_eReceivedPayload )
        {
            /*
             * We have already received a complete request/response/payload in the previous iteration of this function.
             * (e.g. we received payload during previous dataReadyCallback, had issued callback to caller, and now caller
             * had invoked _RecvPayload to read data but socket may be closed at this time.
             * Since we are in a state where there is pending data to be received by App, lets not yet
             * check for error. We will anyway get another errorCallback or app will issue another _Recv call.
             * otherwise, we will loose the request/response/payload (most important one) upon socket error
             * as socket error could be just EOF for a  Player case!
             */
            hHttpSocket->recv.completionStatus = BIP_SUCCESS;
        }
        else
        {
            /* for other states, we may not yet have any completion status to convey, so just continue w/ setting the callbacks. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Recv state %s not yet complete or applicable" BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s after Transition, completionStatus 0x%x"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->recv.completionStatus ));

        /*
         ***************************************************************************************************************
         * Check if we should enable or disable callbacks from BIP_Socket.
         ***************************************************************************************************************
         */
        {
            bool updateSettings = false;
            if (hHttpSocket->dataReadyCallbackState == BIP_SocketCallbackState_eDisabled)
            {
                /* callback is currently disabled, check if we should enable it */
                if ( (hHttpSocket->httpState == BIP_HttpSocketState_eReceivingRequest) ||
                     (hHttpSocket->httpState == BIP_HttpSocketState_eReceivingResponse) ||
                     (hHttpSocket->httpState == BIP_HttpSocketState_eReceivingPayload) )
                     /* TODO: change it to macro representing either of these 3 states! */
                {
                    /* We are still in one of the receiving states, so need to enable dataReadyCallback so as to complete this state. */
                    /* Note: we dont need to check if caller has enabled request/response/payload received callbacks or not. */
                    /* This is because we only transition to one of Receiving states if caller had enabled the callback or issued a _Recv API. */
                    BIP_Socket_GetSettings( hHttpSocket->hSocket, &socketSettings);
                    socketSettings.dataReadyCallback.context  = hHttpSocket;
                    socketSettings.dataReadyCallback.callback = dataReadyCallbackFromBipSocket;
                    updateSettings = true;
                    hHttpSocket->dataReadyCallbackState = BIP_SocketCallbackState_eEnabled;
                }
            }
            else
            {
                /* dataReadyCallback from BIP_Socket is currently enabled, check if we should disable it. */
                if ( (hHttpSocket->httpState != BIP_HttpSocketState_eReceivingRequest) &&
                     (hHttpSocket->httpState != BIP_HttpSocketState_eReceivingResponse) &&
                     (hHttpSocket->httpState != BIP_HttpSocketState_eReceivingPayload) )
                {
                    /* We are not receiving request, response or payload, meaning we have already received them. */
                    /* We will disable the dataReadyCallback from the BIP_Socket even though caller may have the receive callbacks enabled. */
                    /* The callback will get enabled when we transition back to one of the Receiving state. */
                    /* E.g. after a server receives 1st request, we disable this callback until we goback to the Idle state, */
                    /* (which happens when server has sent the Response and any associated Payload). Callback would get enabled */
                    /* if caller had it enabled as we would transition to Receiving State. */
                    /* Likewise, as a Player, callback is disabled after receiving a payload. It will get enabled */
                    /* when Player issues a _Recv call. */
                    BIP_Socket_GetSettings( hHttpSocket->hSocket, &socketSettings);
                    socketSettings.dataReadyCallback.callback = NULL;
                    updateSettings = true;
                    hHttpSocket->dataReadyCallbackState = BIP_SocketCallbackState_eDisabled;
                }
            }

            /* And then lets update the BIP_SocketSettings if required! */
            if (updateSettings)
            {
                brc = BIP_Socket_SetSettings(hHttpSocket->hSocket, &socketSettings);
                /* TODO: need to see how to handle this error w/o asserting? we may have read some data that we would want to return even though this error happesn! */
                BDBG_ASSERT((brc == BIP_SUCCESS));
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Updated BIP_Socket Settings for Callbacks: httpState %s, dataReady CB %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState),
                            hHttpSocket->dataReadyCallbackState ? "Enabled":"Disabled"
                            ));
            }
        }

        if (hHttpSocket->httpState == BIP_HttpSocketState_eError &&
                (hHttpSocket->recv.completionStatus == BIP_ERR_INITIAL_REQUEST_TIMEDOUT ||
                 hHttpSocket->recv.completionStatus == BIP_ERR_PERSISTENT_CONNECTION_TIMEDOUT
                )
           )
        {
            /* We are in the Error state as we didn't receive initial request in the timeout interval. */
            /* Caller will get notified of this either via the callback, or current/pending ARB completion, or next Recv Api call. */
            if ( hHttpSocket->hSocket )
            {
                /* Lets Destroy the BIP Socket as we want peer to quickly get notified about our decision of moving to error state due to our timeout. */
                BIP_Socket_Destroy( hHttpSocket->hSocket );
                hHttpSocket->hSocket = NULL;
            }
        }

        /*
         ***************************************************************************************************************
         * Now decide if we have inProgress Arbs or if we need to invoke any callbacks to consumer of HttpSocket.
         ***************************************************************************************************************
         */
        if (hHttpSocket->httpState == BIP_HttpSocketState_eReceivedRequest)
        {
            if ( BIP_Arb_IsBusy( hHttpSocket->recvRequestApi.hArb) )
            /* We have received complete request, now check if we have a inProgress _RecvRequest API or have a requestReceivedCallback */
            {
                /*
                 * Since caller had issued _RecvRequest API, we transition to ReadyToSendResponse state to indicate
                 * that we will soon give it the request and will be then ready to SendResponse.
                 */
                if ( hHttpSocket->recvRequestApi.phHttpRequest)
                {
                    *hHttpSocket->recvRequestApi.phHttpRequest = hHttpSocket->hHttpRequest;
                }
                if ( hHttpSocket->recvRequestApi.phHttpResponse )
                {
                    *hHttpSocket->recvRequestApi.phHttpResponse = hHttpSocket->hHttpResponse;
                }
                hHttpSocket->httpState = BIP_HttpSocketState_eReadyToSendResponse;

                brc = BIP_Arb_CompleteRequest( hHttpSocket->recvRequestApi.hArb, BIP_SUCCESS);
                BDBG_ASSERT( brc == BIP_SUCCESS );
            }
            else
            {
                /* We dont have a pending _RecvRequest from caller and we have received a full request */
                /* (may be either part of SetSettings call where caller enabled requestReceivedCallback or */
                /* later via dataReadyCallback from BIP_Socket. */
                BDBG_ASSERT( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eArmed );
                if ( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eArmed )
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: state %s: Adding Deferred Callback"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                    /* and caller wants a request receivedCallback, so queue it up and invoke it later below via DoDeferred CB */
                    brc = BIP_Arb_AddDeferredCallback( hHttpSocket->recvRequestApi.hArb, &hHttpSocket->settings.requestReceivedCallback);
                    BDBG_ASSERT( brc == BIP_SUCCESS );
                    hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eTriggered;
                }
            }
        }
        /* TODO: add checks for ReceivedResponse & ReceivedPayload states. */

        if ( (hHttpSocket->httpState == BIP_HttpSocketState_eError) || /* we had error, so complete the RecvRequest API */
             (hHttpSocket->httpState == BIP_HttpSocketState_eReceiveRequestTimedout)  ||  /* RecvRequest timedout, so complete the RecvRequest API */
             (hHttpSocket->httpState == BIP_HttpSocketState_eDestroying) ) /* we are in destroying state so we need to cancel the pending RecvRequest API */
        {
            /* Since we are in one of these states, we will need to finish any pending API call or issue callbacks. */
            if ( BIP_Arb_IsBusy( hHttpSocket->recvRequestApi.hArb) )
            {
                brc = BIP_Arb_CompleteRequest( hHttpSocket->recvRequestApi.hArb, hHttpSocket->recv.completionStatus);
                BDBG_ASSERT( brc == BIP_SUCCESS );
                if ( hHttpSocket->httpState == BIP_HttpSocketState_eError )
                {
                    /*
                     * We disable consumeCallbacks when we first transition to error State.
                     * This way subsequent APIs dont keep queuing up the consumer callback.
                     */
                    hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eDisabled;
                }
            }
            /* TODO: else add checks for other arbInProgress and mark them complete as well! */
            else
            {
                /* No Arbs are pending, but we got an error, we queue up the errorCallback for error State */
                if ( (hHttpSocket->httpState == BIP_HttpSocketState_eError) && hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eArmed )
                {
                    /* Note: it doesn't really matter which Arb we use to queue up this errorCallback, it just needs to be invoked by the Arb_DoDeferred below. */
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: state %s: Adding Deferred Callback"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                    brc = BIP_Arb_AddDeferredCallback( hHttpSocket->recvRequestApi.hArb, &hHttpSocket->settings.requestReceivedCallback);
                    BDBG_ASSERT( brc == BIP_SUCCESS );
                    hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eTriggered;
                }
            }
        }

        /*
         ***************************************************************************************************************
         * We have checked various states and carried out any additional state transitions based on the completion status.
         * We go back to top of state machine to see if we need to run it one more time before issuing the callbacks/ArbCompletions.
         * This can happen when a current API had timed out, so we may need to carry out some work to reset our current state.
         ***************************************************************************************************************
         */

    } /* while reRunProcessState */

    return;
} /* processHttpRecvState */

static bool disablePersistentConnection(
    unsigned                    persistentConnectionTimeoutInMs,
    BIP_HttpRequestHandle    hHttpRequest,
    BIP_HttpResponseHandle   hHttpResponse,
    int64_t                     messageLength,
    bool                        noPayload
    )
{
    BIP_Status                      brc;
    bool                            disablePersistentConnection = false; /* enable persistent connection by default. */
    BIP_HttpRequestHttpVersion      requestVersion;
    const char                      *connectionHeaderValue = NULL;
    BIP_HttpHeaderHandle            hHeader = NULL;

    BSTD_UNUSED( hHttpResponse );

    brc = BIP_HttpRequest_GetHttpVersion( hHttpRequest, &requestVersion );
    BIP_CHECK_GOTO(( brc == BIP_SUCCESS ), ( "BIP_HttpRequest_GetHttpVersion Failed"), error, brc, brc );

    brc = BIP_HttpRequest_GetNextHeader(hHttpRequest, NULL, "Connection", &hHeader, &connectionHeaderValue);
    BIP_CHECK_GOTO(( brc == BIP_SUCCESS || brc == BIP_INF_NOT_AVAILABLE), ( "BIP_HttpRequest_GetNextHeader Failed" ), error, brc, brc );

    if ( persistentConnectionTimeoutInMs == 0 )
    {
        disablePersistentConnection = true;
    }
    else if ( BIP_HttpRequestVersionIs_1_0(&requestVersion) )
    {
        disablePersistentConnection = true;
    }
    else if ( connectionHeaderValue && strcasecmp( connectionHeaderValue, "close" ) == 0 )
    {
        disablePersistentConnection = true;
    }
    else if ( noPayload == true )
    {
        /* The Response didn't include the payload (e.g. HEAD Response) and there is no reason to disable persistence, so keep it on. */
        disablePersistentConnection = false;
    }
    else if ( messageLength <= 0 )
    {
        /* The message length of the response is not known, so we can't support persistence as client will not know how much to differentiate message payloads of different responses. */
        disablePersistentConnection = true;
    }
    else
    {
        /* No reason to turn-off persistentConnections, so keep it on. */
        disablePersistentConnection = false;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "disablePersistentConnection: %s" BIP_MSG_PRE_ARG, disablePersistentConnection ? "Y" : "N" ));
    return ( disablePersistentConnection );

error:
    /* For error cases, we disable persistent connection. */
    disablePersistentConnection = true;
    return ( disablePersistentConnection );
}

static bool processHttpSendState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_HttpSocketHandle    hHttpSocket = hObject;
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    bool                    reRunProcessState;
    BIP_SocketSettings      socketSettings;

    BSTD_UNUSED(value);
    BSTD_UNUSED(threadOrigin);

    BDBG_ASSERT(hHttpSocket);
    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket);
    B_MUTEX_ASSERT_LOCKED(hHttpSocket->hStateMutex);

    {
        reRunProcessState = false;
        hHttpSocket->send.completionStatus = BIP_INF_IN_PROGRESS;
        /*
         ***************************************************************************************************************
         * First, we check API Arbs to see if state processing is being run thru any of these APIs.
         ***************************************************************************************************************
         */
        if (BIP_Arb_IsNew(hArb = hHttpSocket->sendResponseApi.hArb))
        {
            /* Caller has issued _SendResponse API */
            if (hHttpSocket->httpState != BIP_HttpSocketState_eReadyToSendResponse)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_RejectRequest(): _SendResponse Api not allowed in this httpState: %s "
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                hHttpSocket->send.completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpSocket->send.completionStatus);
            }
            else
            {
                /*
                 * We only allow _SendResponse API in the the BIP_HttpSocketState_eReadyToSendResponse state
                 * as we would have received an incoming request and caller would have received this request.
                 */
                BIP_Arb_AcceptRequest(hArb);
                hHttpSocket->send.completionStatus = BIP_INF_IN_PROGRESS;
                hHttpSocket->httpState = BIP_HttpSocketState_eSendNewResponse;
                /* Note the start time: it is useful for the non-blocking invocation where timeout is 0 and thus its a one shot call! */
                B_Time_Get(&hHttpSocket->send.startTime);

                /* Check if we need to enable the timer logic. */
                if (hHttpSocket->sendResponseApi.settings.timeoutInMs >= 0)
                {
                    BIP_TimerCreateSettings timerCreateSettings;
                    /* timer logic check is active for non-blocking (== 0) or actual timeout cases (timeout > 0) */
                    hHttpSocket->send.timerActive = true;

                    if (hHttpSocket->sendResponseApi.settings.timeoutInMs > 0)
                    {
                        /* setup a timer callback only for +ve timeout value */
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Starting timer for %d ms" BIP_MSG_PRE_ARG, (void *)hHttpSocket, hHttpSocket->sendResponseApi.settings.timeoutInMs ));
                        timerCreateSettings.input.callback    = processHttpStateFromTimerCallback_Send;
                        timerCreateSettings.input.pContext    = hHttpSocket;
                        timerCreateSettings.input.timeoutInMs = hHttpSocket->sendResponseApi.settings.timeoutInMs;
                        hHttpSocket->send.hTimer = BIP_Timer_Create(&timerCreateSettings);
                        BIP_CHECK_GOTO(( hHttpSocket->send.hTimer ), ( "BIP_Timer_Create Failed"), error, BIP_ERR_INTERNAL, hHttpSocket->send.completionStatus );
                    }
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Accepted _SendResponse Arb request: httpState %s!"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
            }
            /* Fall thru to continue with the state processing! */
        }
        else if (BIP_Arb_IsNew(hArb = hHttpSocket->sendPayloadApi.hArb))
        {
            /* Caller has issued _SendPayload API */
            if (hHttpSocket->httpState != BIP_HttpSocketState_eReadyToSendPayload)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_RejectRequest(): _SendPayload Api not allowed in this httpState: %s "
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                hHttpSocket->send.completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
                BIP_Arb_RejectRequest(hArb, hHttpSocket->send.completionStatus);
            }
            else
            {
                /*
                 * We only allow _SendPayload API in the the BIP_HttpSocketState_eReadyToSendPayload state
                 * as HTTP Server is supposed to send a HTTP Response Message before it can send any payload.
                 */
                BIP_Arb_AcceptRequest(hArb);
                hHttpSocket->send.completionStatus = BIP_INF_IN_PROGRESS;
                hHttpSocket->httpState = BIP_HttpSocketState_eSendNewPayload;
                /* Note the start time: it is useful for the non-blocking invocation where timeout is 0 and thus its a one shot call! */
                B_Time_Get(&hHttpSocket->send.startTime);

                /* Check if we need to enable the timer logic. */
                if (hHttpSocket->sendPayloadApi.settings.timeoutInMs>= 0)
                {
                    BIP_TimerCreateSettings timerCreateSettings;
                    /* timer logic check is active for non-blocking (== 0) or actual timeout cases (timeout > 0) */
                    hHttpSocket->send.timerActive = true;

                    if (hHttpSocket->sendPayloadApi.settings.timeoutInMs > 0)
                    {
                        /* setup a timer callback only for +ve timeout value */
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Starting timer for %d ms" BIP_MSG_PRE_ARG, (void *)hHttpSocket, hHttpSocket->sendPayloadApi.settings.timeoutInMs ));
                        timerCreateSettings.input.callback    = processHttpStateFromTimerCallback_Send;
                        timerCreateSettings.input.pContext    = hHttpSocket;
                        timerCreateSettings.input.timeoutInMs = hHttpSocket->sendPayloadApi.settings.timeoutInMs;
                        hHttpSocket->send.hTimer = BIP_Timer_Create(&timerCreateSettings);
                        BIP_CHECK_GOTO(( hHttpSocket->send.hTimer ), ( "BIP_Timer_Create Failed"), error, BIP_ERR_INTERNAL, hHttpSocket->send.completionStatus );
                    }
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Accepted _SendPayload Arb request: httpState %s!"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
            }
            /* Fall thru to continue with the state processing! */
        }
        /* TODO: add checks for SendRequest Arb */

        /*
         ***************************************************************************************************************
         * Now walk thru various send states and carry out the appropriate processing.
         ***************************************************************************************************************
         */

        /*
         ***************************************************************************************************************
         * SendNewResponse state: get ready to send new response.
         ***************************************************************************************************************
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendNewResponse )
        {
            /* Determine if persistentConnection can be supported on this HttpSocket or not. Recv State machine will use this decision. */
            hHttpSocket->disablePersistentConnection = disablePersistentConnection(
                    hHttpSocket->createSettings.persistentConnectionTimeoutInMs,
                    hHttpSocket->sendResponseApi.hHttpRequest,
                    hHttpSocket->sendResponseApi.hHttpResponse,
                    hHttpSocket->sendResponseApi.messageLength,
                    hHttpSocket->sendResponseApi.settings.noPayload
                    );
            if ( hHttpSocket->disablePersistentConnection == true )
            {
                /* We reset the timer when persistentConnection is disabled. */
                hHttpSocket->recv.persistentConnectionTimeoutInMs = 0;
            }

            brc = addHttpRsvdHeaders(
                    hHttpSocket->sendResponseApi.hHttpResponse,
                    hHttpSocket->sendResponseApi.messageLength,
                    hHttpSocket->sendResponseApi.settings.enableHttpChunkXferEncoding,
                    hHttpSocket->disablePersistentConnection);
            BIP_CHECK_GOTO(( (brc == BIP_SUCCESS) ), ( "Failed to add Reserved Http Headers to the Response"), error, brc, hHttpSocket->send.completionStatus );

            /* This is probably a good time to print the HttpResponse. */
            {
                bool printEnabled = false;
                /* The following two prints will set the printEnabled flag if the BDBG module is enabled. */
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "Sending HTTP response:" BIP_HTTP_SOCKET_PRINTF_FMT
                              BIP_MSG_PRE_ARG, BIP_HTTP_SOCKET_PRINTF_ARG(hHttpSocket)));
                BIP_MSG_SUM(( BIP_MSG_PRE_FMT "Printing HTTP response: %p %s"
                              BIP_MSG_PRE_ARG, (void *)hHttpSocket->sendResponseApi.hHttpResponse, (true?(printEnabled=true,""):"") ));
                if (printEnabled) {
                    BIP_HttpResponse_Print( hHttpSocket->sendResponseApi.hHttpResponse, NULL, NULL);
                }
            }

            /* Nothing much else to do in the New state, so we switch to Sending state */
            hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponse;
        } /* SendNewResponse processing */
        else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendNewPayload )
        {
            BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Sending HTTP payload: hHttpSocket=%p state=%s bufPtr=%p bytesToSend=%zu"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), (void *)hHttpSocket->sendPayloadApi.pBuffer, hHttpSocket->sendPayloadApi.bytesToSend ));

            /* starting a new send, so reset the bytesSent. */
            hHttpSocket->send.payloadBytesSent = 0;
            hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayload;
        } /* SendNewPayload processing */

        /*
         ***************************************************************************************************************
         * SendingResponse state: we try to send response data using BIP_Socket_Send in the non-blocking mode.
         * We will move to SendingDone state when finish sending all data, error, or timeout case.
         ***************************************************************************************************************
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponse )
        {
            bool    serializeComplete = false;
            BIP_SocketSendSettings socketSendSettings;
            ssize_t bytesSent = 0;
            ssize_t totalBytesSent = 0;
            size_t  serializedBytes;

            while (!serializeComplete)
            {
                brc = BIP_HttpResponse_SerializeToBuffer(
                        hHttpSocket->sendResponseApi.hHttpResponse,
                        hHttpSocket->pNetworkWriteBuffer,
                        hHttpSocket->createSettings.networkWriteBufferSize,
                        &serializeComplete,
                        &serializedBytes
                        );
                BIP_CHECK_GOTO(( (brc == BIP_SUCCESS) && serializedBytes ),
                        ( "BIP_HttpResponse_SerializeToBuffer Failed"), error, brc, hHttpSocket->send.completionStatus );

                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, Calling BIP_Socket_Send: bytesToSend %zu"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), serializedBytes ));

                /*
                 * Now try to send upto responseBufferSize bytes to network using BIP_Socket_Send.
                 * NOTE: We do this using the non-blocking Send on BIP_Socket.
                 */
                totalBytesSent = 0;
                while(serializedBytes && (brc == BIP_SUCCESS))
                {
                    BIP_Socket_GetDefaultSocketSendSettings( &socketSendSettings );
                    socketSendSettings.api.timeout = 0; /* non-blocking mode */
                    socketSendSettings.input.bytesToSend = serializedBytes;
                    socketSendSettings.input.pBuffer = &(hHttpSocket->pNetworkWriteBuffer[totalBytesSent]);
                    socketSendSettings.output.pBytesSent = &bytesSent;

                    brc = BIP_Socket_Send( hHttpSocket->hSocket, &socketSendSettings);

                    serializedBytes -= bytesSent;
                    totalBytesSent += bytesSent;
                }

                if(brc == BIP_SUCCESS)
                {
                    if(serializeComplete)
                    {
                        hHttpSocket->send.completionStatus = BIP_SUCCESS;
                         /* We just sent the complete response, so we are done sending! */
                        hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponseDone;
                        break; /* from the loop */
                    }
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, sent header bytes (cur %zu)  in Response"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), totalBytesSent ));
                }
                else if (brc == BIP_INF_IN_PROGRESS || brc == BIP_INF_TIMEOUT)
                {
                    /* TODO: this needs bit more work after finalizing the BIP_Socket behavior! */
                    /* cant' send data at this time, we remain in the Sending state and fall thru ... */
                    hHttpSocket->send.completionStatus = BIP_INF_IN_PROGRESS;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Got Timeout from BIP_Socket, brc 0x%x"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), brc));
                    break; /* we break out from the loop as we can't send any bytes rightnow */
                }
                else
                {
                    /* Otherwise, it is an error case, we move to Done state and set the error completion code. */
                    hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponseDone;
                    hHttpSocket->send.completionStatus = brc; /* we use the status that BIP_Socket has indicated! */
                    BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: completionStatus 0x%x"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->send.completionStatus ));

                    break; /* we break out from the loop as we can't send any bytes rightnow */
                }
            } /* while !completeHeader */
        } /* SendingResponse processing */
        else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayload )
        {
            BIP_SocketSendSettings socketSendSettings;
            ssize_t bytesSent = 0;

            /*
             * Now try to send payload bytes to network using BIP_Socket_Send.
             * NOTE: We do this using the non-blocking Send on BIP_Socket.
             */
            BIP_Socket_GetDefaultSocketSendSettings( &socketSendSettings );
            socketSendSettings.api.timeout = 0; /* non-blocking mode */
            socketSendSettings.input.pBuffer = (const char *)hHttpSocket->sendPayloadApi.pBuffer + hHttpSocket->send.payloadBytesSent;
            socketSendSettings.input.bytesToSend = hHttpSocket->sendPayloadApi.bytesToSend - hHttpSocket->send.payloadBytesSent;
            socketSendSettings.output.pBytesSent = &bytesSent;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, Calling BIP_Socket_Send: bytesToSend %zu "
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), socketSendSettings.input.bytesToSend ));

            brc = BIP_Socket_Send( hHttpSocket->hSocket, &socketSendSettings);

            if (brc == BIP_SUCCESS && bytesSent > 0)
            {
                /* We sent some bytes, update the total payloadBytesSent. */
                hHttpSocket->send.payloadBytesSent += bytesSent;
                hHttpSocket->stats.numBytesSent += bytesSent;

                /* check if we have sent the whole payload */
                if ( hHttpSocket->send.payloadBytesSent >= hHttpSocket->sendPayloadApi.bytesToSend )
                {
                    hHttpSocket->send.completionStatus = BIP_SUCCESS;
                    /* We just sent the complete payload, so we are done sending! */
                    hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayloadDone;
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, sent all (%zu) payload bytes (cur %zu)"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->send.payloadBytesSent, bytesSent ));
                }
                else
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s, sent bytes (cur %zu) in Payload, asked %zu, remaining %zu"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), bytesSent,
                                hHttpSocket->sendPayloadApi.bytesToSend, (hHttpSocket->sendPayloadApi.bytesToSend - hHttpSocket->send.payloadBytesSent)
                             ));
                }
            }
            else if (brc == BIP_INF_IN_PROGRESS || brc == BIP_INF_TIMEOUT)
            {
                /* TODO: this needs bit more work after finalizing the BIP_Socket behavior! */
                /* cant' send data at this time, we remain in the Sending state and fall thru ... */
                hHttpSocket->send.completionStatus = BIP_INF_IN_PROGRESS;
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Got Timeout from BIP_Socket, brc 0x%x"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), brc));
            }
            else
            {
                /* Otherwise, it is an error case, we move to Done state and set the error completion code. */
                hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayloadDone;
                hHttpSocket->send.completionStatus = brc; /* we use the status that BIP_Socket has indicated! */
                BDBG_ERR(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: BIP_Socket_Send Failed w/ completionStatus %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->send.completionStatus) ));
            }
        } /* SendingPayload processing */


        /*
         ***************************************************************************************************************
         * We are done processing current states. Now we check for any state completions either due to
         * -app calls Destroy API,
         * -timeout event,
         * -send desired data (request, response, payload), or
         * -network error!
         ***************************************************************************************************************
         */
error:
        /* Check for completion by app destroying this object. */
        if ( hHttpSocket->shutdownState != BIP_HttpSocketShutdownState_eNormal )
        {
            hHttpSocket->send.completionStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
            if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponse)
            {
                /* only change the state to done if we were sending! */
                hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponseDone;
            }
            else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayload)
            {
                /* only change the state to done if we were sending! */
                hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayloadDone;
            }
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: Completing Send state due to Destroy API"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
        }

        /*
         * Check for timeouts and set timeout to true if we have exceeded the time.
         */
        if ( hHttpSocket->send.timerActive && hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponse )
        {
            B_Time      timeNow;
            int         elapsedTimeInMs;

            B_Time_Get(&timeNow);
            elapsedTimeInMs = B_Time_Diff(&timeNow,&hHttpSocket->send.startTime);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Send Timeout Check: So far: %d ms, limit is %d ms." BIP_MSG_PRE_ARG,
                        (void *)hHttpSocket, elapsedTimeInMs , hHttpSocket->sendResponseApi.settings.timeoutInMs ));

            if (elapsedTimeInMs >= hHttpSocket->sendResponseApi.settings.timeoutInMs)
            {
                /* Note: this check will be true even when state machine has been called in the non-blocking mode */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: _SendResponse Timedout"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                /* we will need to save all bytes which were so far read into the partialMsgBuffer so that they can be used in the next send attempt */
                hHttpSocket->send.completionStatus = BIP_INF_TIMEOUT;
                hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponseDone;
                hHttpSocket->send.timerActive = false;  /* this flag enables us to not check for timeout in the 2nd loop around! */
            }
        }
        else if ( hHttpSocket->send.timerActive && hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayload )
        {
            B_Time      timeNow;
            int         elapsedTimeInMs;

            B_Time_Get(&timeNow);
            elapsedTimeInMs = B_Time_Diff(&timeNow,&hHttpSocket->send.startTime);
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Send Timeout Check: So far: %d ms, limit is %d ms." BIP_MSG_PRE_ARG,
                        (void *)hHttpSocket, elapsedTimeInMs , hHttpSocket->sendPayloadApi.settings.timeoutInMs ));

            if (elapsedTimeInMs >= hHttpSocket->sendPayloadApi.settings.timeoutInMs)
            {
                /* Note: this check will be true even when state machine has been called in the non-blocking mode */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s: _SendPayload Timedout"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                hHttpSocket->send.completionStatus = BIP_INF_TIMEOUT;
                hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayloadDone;
                hHttpSocket->send.timerActive = false;  /* this flag enables us to not check for timeout in the 2nd loop around! */
            }
        }

        /* If completion status is not IN_PROGRESS, then we are done w/ current state! */
        if ( hHttpSocket->send.completionStatus != BIP_INF_IN_PROGRESS )
        {
            if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponse) hHttpSocket->httpState = BIP_HttpSocketState_eSendingResponseDone;
            else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayload) hHttpSocket->httpState = BIP_HttpSocketState_eSendingPayloadDone;
        }

        if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponseDone )
        {
            if (hHttpSocket->send.completionStatus == BIP_SUCCESS)
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eSentResponse;
            }
            else if (hHttpSocket->send.completionStatus == BIP_INF_TIMEOUT)
            {
                /*
                 * We timed out while sending, we go to the Timedout state.
                 * Caller will have to issue _SendResponse again to send out the response!
                 */
                hHttpSocket->httpState = BIP_HttpSocketState_eSendResponseTimedout;
            }
            else if (hHttpSocket->send.completionStatus == BIP_ERR_OBJECT_BEING_DESTROYED)
            {
                /* For us to be here in the SendingResponseDone state means that we were currently in sending state! */
                /* So we move the Destroying state */
                hHttpSocket->httpState = BIP_HttpSocketState_eDestroying;
            }
            else /* error case */
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eError;
            }
        }
        else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayloadDone )
        {
            if (hHttpSocket->send.completionStatus == BIP_SUCCESS)
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eSentPayload;
            }
            else if (hHttpSocket->send.completionStatus == BIP_INF_TIMEOUT)
            {
                /*
                 * We timed out while sending, we go to the Timedout state.
                 * Caller will have to issue _SendPayload again to send out the response!
                 */
                hHttpSocket->httpState = BIP_HttpSocketState_eSendPayloadTimedout;
            }
            else if (hHttpSocket->send.completionStatus == BIP_ERR_OBJECT_BEING_DESTROYED)
            {
                /* For us to be here in the SendingPayloadDone state means that we were currently in sending state! */
                /* So we move the Destroying state */
                hHttpSocket->httpState = BIP_HttpSocketState_eDestroying;
            }
            else /* error case */
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eError;
            }
        }
        /* TODO: Need to add checks for SendingRequestDone state */
        else
        {
            /* for other send states, we may not yet have any completion status to convey, so just continue w/ setting the callbacks. */
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: send state %s not yet complete!" BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: httpState %s after Transition, completionStatus 0x%x"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->send.completionStatus ));

        /*
         ***************************************************************************************************************
         * Check if we should enable or disable callbacks from BIP_Socket.
         ***************************************************************************************************************
         */
        {
            bool updateSettings = false;
            /* Check for writeReadyCallback as it pertains to sending side */
            if (hHttpSocket->writeReadyCallbackState == BIP_SocketCallbackState_eDisabled)
            {
                /* callback is currently disabled, check if we should enable it */
                if ( (hHttpSocket->httpState == BIP_HttpSocketState_eSendingResponse) ||
                     (hHttpSocket->httpState == BIP_HttpSocketState_eSendingPayload) ||
                     (hHttpSocket->httpState == BIP_HttpSocketState_eSendingRequest) )
                     /* TODO: change it to macro representing any of these 3 states! */
                {
                    /*
                     * We are still in one of the sending states, that means that we couldn't finish sending the attempt above.
                     * So we need to enable writeReadyCallback so that we can then send remaining data to complete this state.
                     */
                    BIP_Socket_GetSettings( hHttpSocket->hSocket, &socketSettings);
                    socketSettings.writeReadyCallback.context  = hHttpSocket;
                    socketSettings.writeReadyCallback.callback = writeReadyCallbackFromBipSocket;
                    updateSettings = true;
                    hHttpSocket->writeReadyCallbackState = BIP_SocketCallbackState_eEnabled;
                }
            }
            else
            {
                /* writeReadyCallback from BIP_Socket is currently enabled, check if we should disable it. */
                if ( (hHttpSocket->httpState != BIP_HttpSocketState_eSendingRequest) &&
                     (hHttpSocket->httpState != BIP_HttpSocketState_eSendingResponse) &&
                     (hHttpSocket->httpState != BIP_HttpSocketState_eSendingPayload) )
                {
                    /*
                     * We are not sending request, response or payload, meaning we have already sent them.
                     * We will disable the writeReadyCallback from the BIP_Socket.
                     * The callback will get enabled when caller again calls any of the Send APIs.
                     */
                    BIP_Socket_GetSettings( hHttpSocket->hSocket, &socketSettings);
                    socketSettings.writeReadyCallback.callback = NULL;
                    updateSettings = true;
                    hHttpSocket->writeReadyCallbackState = BIP_SocketCallbackState_eDisabled;
                }
            }

            /* And then lets update the BIP_SocketSettings if required! */
            if (updateSettings)
            {
                brc = BIP_Socket_SetSettings(hHttpSocket->hSocket, &socketSettings);
                BDBG_ASSERT((brc == BIP_SUCCESS));    /* TODO: need to see how to handle this error w/o asserting? we may have read some data that we would want to return even though this error happesn! */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Updated BIP_Socket Settings for Callbacks: httpState %s, dataReady CB %s, writeReady CB %s, error CB %s"
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState),
                            hHttpSocket->dataReadyCallbackState ? "Enabled":"Disabled",
                            hHttpSocket->writeReadyCallbackState ? "Enabled":"Disabled",
                            hHttpSocket->errorCallbackState ? "Enabled":"Disabled"
                            ));
            }
        }

        /*
         ***************************************************************************************************************
         * Now decide if we have inProgress Arbs or if we need to invoke any callbacks to consumer of HttpSocket.
         ***************************************************************************************************************
         */
        if ( hHttpSocket->httpState == BIP_HttpSocketState_eSentResponse && BIP_Arb_IsBusy (hHttpSocket->sendResponseApi.hArb) )
        {
            /* We have sent complete response, so we are ready to send the payload */
            brc = BIP_Arb_CompleteRequest( hHttpSocket->sendResponseApi.hArb, BIP_SUCCESS );
            BDBG_ASSERT( brc == BIP_SUCCESS );

            if (hHttpSocket->sendResponseApi.settings.noPayload)
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eIdle;

                BIP_HttpRequest_Clear(hHttpSocket->hHttpRequest, NULL);
                BIP_HttpResponse_Clear( hHttpSocket->hHttpResponse, NULL);

                if ( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eTriggered )
                {
                    /* We are going to idle state & app had enabled the callback (that's why we had triggered it), so arm the callback. */
                    hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eArmed;
                }
                BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: state %s: re-running the state machine as we are in back in Idle state after sending a response w/ no payload, CB state %d "
                            BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->requestReceivedCallbackState ));
                reRunProcessState = true;
            }
            else
            {
                hHttpSocket->httpState = BIP_HttpSocketState_eReadyToSendPayload;
            }
        }
        else if ( hHttpSocket->httpState == BIP_HttpSocketState_eSentPayload && BIP_Arb_IsBusy (hHttpSocket->sendPayloadApi.hArb) )
        {
            /* We have sent complete response, so we are ready to send the payload */
            brc = BIP_Arb_CompleteRequest( hHttpSocket->sendPayloadApi.hArb, BIP_SUCCESS );
            BDBG_ASSERT( brc == BIP_SUCCESS );

            hHttpSocket->httpState = BIP_HttpSocketState_eReadyToSendPayload;
        }

        if ( (hHttpSocket->httpState == BIP_HttpSocketState_eError) || /* we had error, so complete the RecvRequest API */
             (hHttpSocket->httpState == BIP_HttpSocketState_eDestroying) ) /* we are in destroying state so we need to cancel the pending RecvRequest API */
        {
            /* Since we are in one of these states, we will need to finish any pending API call or issue callbacks. */
            if ( BIP_Arb_IsBusy( hHttpSocket->sendResponseApi.hArb) )
            {
                brc = BIP_Arb_CompleteRequest( hHttpSocket->sendResponseApi.hArb, hHttpSocket->send.completionStatus);
                BDBG_ASSERT( brc == BIP_SUCCESS );
            }
            else if ( BIP_Arb_IsBusy( hHttpSocket->sendPayloadApi.hArb) )
            {
                brc = BIP_Arb_CompleteRequest( hHttpSocket->sendPayloadApi.hArb, hHttpSocket->send.completionStatus);
                BDBG_ASSERT( brc == BIP_SUCCESS );
            }
            /* TODO: else add checks for SendRequest API and mark it complete as well! */
            else
            {
                /* No Send APIs are pending, but we got an error, we queue up the errorCallback for error State */
                if ( hHttpSocket->httpState == BIP_HttpSocketState_eError)
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: state %s: detected error, not much to do in send state!"
                                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
                }
            }
        }
    }

    return reRunProcessState;
} /* processHttpSendState */

void BIP_HttpSocket_SetStateToIdle_priv(
    BIP_HttpSocketHandle hHttpSocket
    )
{
    BDBG_ASSERT(hHttpSocket);
    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket);

    B_Mutex_Lock( hHttpSocket->hStateMutex );
    hHttpSocket->httpState = BIP_HttpSocketState_eIdle;

    BIP_HttpRequest_Clear(hHttpSocket->hHttpRequest, NULL);
    BIP_HttpResponse_Clear( hHttpSocket->hHttpResponse, NULL);
    if ( hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eTriggered )
    {
        /* We are going to idle state & app had enabled the callback (that's why we had triggered it), so arm the callback. */
        hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eArmed;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: state %s: Switched back to Idle state, requestReceivedCallbackState %d "
                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), hHttpSocket->requestReceivedCallbackState ));
    B_Mutex_Unlock( hHttpSocket->hStateMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hHttpSocket %p --------------------> "  BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
    processHttpState(hHttpSocket, 0, BIP_Arb_ThreadOrigin_eUnknown);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hHttpSocket %p <-------------------- " BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
}

void processHttpState(
    void *hObject,
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_HttpSocketHandle    hHttpSocket = hObject;
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    BIP_Status              completionStatus = BIP_ERR_INTERNAL;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hHttpSocket);
    BDBG_OBJECT_ASSERT( hHttpSocket, BIP_HttpSocket);

    /*
     ***************************************************************************************************************
     * HTTP Socket State Machine Processing:
     *
     *  -Acquire state lock
     *  -Process API specific Arbs one by one, this most likely initiates the state machine processing.
     *  -Walk thru various states to see which one matches the current state and carry out its processing.
     *  -Once current state processing is done, then look for errors or timeout cases (driven by error/timeout callbacks).
     *  -Determine the next state to transition to based on the completionStatus and change state.
     *  -Determine if BIP_Socket Callbacks should be enabled or disabled.
     *  -Determine if any inProgress ARBs should be completed and set flags to indicate that.
     *  -Determine if callbacks need to be issued to the caller and set flags to indicate that.
     *  -Release state lock
     *  -Issue Arb Completions & invoke callbacks as determined above!
     *
     ***************************************************************************************************************
     */
    B_Mutex_Lock( hHttpSocket->hStateMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Entry: httpState %s" BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
    /*
     ***************************************************************************************************************
     * First, we check API Arbs to see if state processing is being run thru any of these APIs.
     ***************************************************************************************************************
     */
    if (BIP_Arb_IsNew(hArb = hHttpSocket->getSettingsApi.hArb))
    {
        /* State machine is run by the _GetSettings API. */
        BIP_Arb_AcceptRequest(hArb);

        /* Copy over the current cached settings. */
        BKNI_Memcpy(hHttpSocket->getSettingsApi.pSettings, &hHttpSocket->settings, sizeof (*hHttpSocket->getSettingsApi.pSettings) );

        completionStatus = BIP_SUCCESS;
        /* We are done this API Arb, so set its completion status. */
        brc = BIP_Arb_CompleteRequest( hHttpSocket->getSettingsApi.hArb, completionStatus);
        BDBG_ASSERT( brc == BIP_SUCCESS );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Processed GetSettings Arb request: httpState %s!"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));

        /* Fall thru to continue with the state processing! */
    }
    else if (BIP_Arb_IsNew(hArb = hHttpSocket->setSettingsApi.hArb))
    {
        BIP_Arb_AcceptRequest(hArb);

        /* See if the caller enabled the requestReceived callback... */
        if ( ! hHttpSocket->settings.requestReceivedCallback.callback  &&
                hHttpSocket->setSettingsApi.pSettings->requestReceivedCallback.callback)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Enabling RequestReceived callbacks" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
            BDBG_ASSERT(hHttpSocket->requestReceivedCallbackState == BIP_HttpSocketConsumerCallbackState_eDisabled);
            hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eArmed;
        }

        /* See if they disabled the requestReceived callback... */
        if (hHttpSocket->settings.requestReceivedCallback.callback  &&
                ! hHttpSocket->setSettingsApi.pSettings->requestReceivedCallback.callback)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Disabling RequestReceived callbacks to app" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
            hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eDisabled;
        }

        /* Cache the new Settings. Then, we will continue w/ the state processing. */
        hHttpSocket->settings = *hHttpSocket->setSettingsApi.pSettings;

        completionStatus = BIP_SUCCESS;
        brc = BIP_Arb_CompleteRequest( hHttpSocket->setSettingsApi.hArb, completionStatus);
        BDBG_ASSERT( brc == BIP_SUCCESS );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Accepted & Completed SetSettings Arb request: httpState %s: requestReceivedCallback %s"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState),
                    hHttpSocket->settings.requestReceivedCallback.callback ? "Y":"N"
                 ));

        /* Fall thru to continue with the state processing! */
        /* We may receive some data part of this Arb request and then accordingly update the BIP Socket Callbacks. */
    }
    else if (BIP_Arb_IsNew(hArb = hHttpSocket->destroyApi.hArb))
    {
        if ( hHttpSocket->shutdownState != BIP_HttpSocketShutdownState_eNormal )
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hSocket %p: Calling BIP_Arb_RejectRequest(): Already being shutdown, httpState: %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
            hHttpSocket->send.completionStatus = BIP_ERR_INVALID_API_SEQUENCE;
            BIP_Arb_RejectRequest(hArb, hHttpSocket->send.completionStatus);
        }
        else
        {
            /* State machine is run by the Destroy API. */
            BIP_Arb_AcceptRequest(hArb);

            hHttpSocket->httpState = BIP_HttpSocketState_eDestroying;
            hHttpSocket->shutdownState = BIP_HttpSocketShutdownState_eStartShutdown;
            completionStatus = BIP_SUCCESS;

            /* We are done this API Arb, so set its completion status. */
            brc = BIP_Arb_CompleteRequest( hHttpSocket->destroyApi.hArb, completionStatus);
            BDBG_ASSERT( brc == BIP_SUCCESS );

            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: Shutdown of HttpSocket object started part of Destroy API: httpState %s!"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        /* Fall thru to continue with the state processing to abort any pending API ARBs. */
    }

    /*
     ***************************************************************************************************************
     * Now call functions to handle recv & send specific states
     ***************************************************************************************************************
     */
    {
        bool reRunProcessState = true;

        while (reRunProcessState)
        {
            reRunProcessState = false;
            processHttpRecvState( hHttpSocket, 0, threadOrigin );
            reRunProcessState = processHttpSendState( hHttpSocket, 0, threadOrigin );
        }
    }

    /*
     ***************************************************************************************************************
     * Now do the object related destruction.
     ***************************************************************************************************************
     */
    if ( hHttpSocket->shutdownState == BIP_HttpSocketShutdownState_eStartShutdown )
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Starting HttpSocket Destruction: hHttpSocket %p: httpState %s"
                    BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState)));
        /* Disable any callbacks to the consumer. */
        hHttpSocket->requestReceivedCallbackState = BIP_HttpSocketConsumerCallbackState_eDisabled;
        hHttpSocket->shutdownState = BIP_HttpSocketShutdownState_eFinishingArbs;
    }

    if ( hHttpSocket->shutdownState == BIP_HttpSocketShutdownState_eFinishingArbs )
    {
        bool arbStillBusy = false;

        if ( !BIP_Arb_IsIdle( hHttpSocket->getSettingsApi.hArb ) )
        {
            arbStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: BIP_HttpSocket Destruction is waiting for getSettingsApi to finish: state %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        if ( !BIP_Arb_IsIdle( hHttpSocket->setSettingsApi.hArb ) )
        {
            arbStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: BIP_HttpSocket Destruction is waiting for setSettingsApi to finish: state %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        if ( !BIP_Arb_IsIdle( hHttpSocket->recvRequestApi.hArb ) )
        {
            arbStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: BIP_HttpSocket Destruction is waiting for recvRequestApi to finish: state %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        if ( !BIP_Arb_IsIdle( hHttpSocket->sendResponseApi.hArb ) )
        {
            arbStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: BIP_HttpSocket Destruction is waiting for sendResponseApi to finish: state %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        if ( !BIP_Arb_IsIdle( hHttpSocket->sendPayloadApi.hArb ) )
        {
            arbStillBusy = true;
            BDBG_MSG(( BIP_MSG_PRE_FMT "hHttpSocket %p: BIP_HttpSocket Destruction is waiting for sendPayloadApi to finish: state %s"
                        BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState) ));
        }

        if (!arbStillBusy )
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "All Arbs are idle: Finish HttpSocket Destruction: hHttpSocket %p" BIP_MSG_PRE_ARG, (void *)hHttpSocket ));
            hHttpSocket->shutdownState = BIP_HttpSocketShutdownState_eShutdownDone;
            B_Event_Set( hHttpSocket->hShutdownEvent );
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Finished Processing HTTP State for hHttpSocket %p: httpState %s, completionStatus: recv %s, send %s"
                BIP_MSG_PRE_ARG, (void *)hHttpSocket, BIP_HTTP_SOCKET_STATE(hHttpSocket->httpState), BIP_StatusGetText(hHttpSocket->recv.completionStatus), BIP_StatusGetText(hHttpSocket->send.completionStatus) ));

    /*
     * Done with state processing. We have to unlock state machine before issuing callbacks!
     * These callbacks were queued up above via the Arb_AddDeferredCallbacks.
     */
    B_Mutex_Unlock( hHttpSocket->hStateMutex );

    brc = BIP_Arb_DoDeferred( NULL, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );

    return;
} /* processHttpState */
