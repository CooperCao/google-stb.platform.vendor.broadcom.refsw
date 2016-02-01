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

#include "blst_queue.h"
#include "bip.h"

BDBG_MODULE(http_socket_simple);

#define DEFAULT_SERVER_PORT_STRING "80"
#define TEST_PAYLOAD_STRING "This is a test payload string."

static void callbackFromBip (
    void *context,
    int   param
    )
{
    B_EventHandle event = context;

    BDBG_MSG(( "Callback from BIP: param %d", param ));
    B_Event_Set( event );
}

#define BUFFER_SIZE (1024*1024)
char buffer[BUFFER_SIZE];
int main(void)
{
    BIP_Status bipStatus;
    NEXUS_Error nrc;
    B_Error berr;
    B_EventHandle hEventFromBip = NULL;
    BIP_SocketHandle hSocket = NULL;
    BIP_ListenerHandle hListener = NULL;
    BIP_HttpSocketHandle hHttpSocket = NULL;
    BIP_HttpRequestHandle hHttpRequest;
    BIP_HttpResponseHandle hHttpResponse;

    /* Initialize BIP */
    {
        bipStatus = BIP_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Initialize NEXUS */
    {
        nrc = NEXUS_Platform_Init( NULL );
        BIP_CHECK_GOTO(( nrc == NEXUS_SUCCESS ), ( "NEXUS_Platform_Init Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }


    /* Create & Start Listener. */
    {
        BIP_ListenerCreateSettings  listenerCreateSettings;
        BIP_ListenerSettings        listenerSettings;
        BIP_ListenerStartSettings   listenerStartSettings;

        /* Create Event */
        hEventFromBip = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( hEventFromBip != NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        /* Create Listener */
        BIP_Listener_GetDefaultCreateSettings( &listenerCreateSettings );
        hListener = BIP_Listener_Create ( &listenerCreateSettings );
        BIP_CHECK_GOTO(( hListener != NULL ), ( "BIP_Listener_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        BIP_Listener_GetSettings( hListener, &listenerSettings );
        listenerSettings.connectedCallback.callback = callbackFromBip;
        listenerSettings.connectedCallback.context = hEventFromBip;
        listenerSettings.connectedCallback.param = 0;
        bipStatus = BIP_Listener_SetSettings( hListener, &listenerSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Listener_SetSettings Failed" ), error, bipStatus, bipStatus );

        BIP_Listener_GetDefaultStartSettings( &listenerStartSettings );
        listenerStartSettings.pPort = DEFAULT_SERVER_PORT_STRING;
        bipStatus = BIP_Listener_Start( hListener, &listenerStartSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Listener_Start Failed" ), error, bipStatus, bipStatus );
        BDBG_WRN(("Listener started on port %s", listenerStartSettings.pPort));
    }

    /* Now wait to get the connected callback from BIP Listener. */
    {
        berr = B_Event_Wait( hEventFromBip, B_WAIT_FOREVER );
        if ( berr == B_ERROR_TIMEOUT ) goto error;    /* Event not set, so callback yet. */
    }

    /* Wait for new connection & Accept it. */
    {
        BIP_HttpSocketSettings httpSocketSettings;

        /* Got the event, so Accept the Connection. */
        hSocket = BIP_Listener_Accept( hListener, 0);
        BIP_CHECK_GOTO(( hSocket != NULL ), ( "BIP_Listener_Accept Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        /* Create HTTP Socket. */
        hHttpSocket = BIP_HttpSocket_CreateFromBipSocket( hSocket, NULL);
        if ( hHttpSocket == NULL )
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "BIP_HttpSocket_CreateFromBipSocket Failed" BIP_MSG_PRE_ARG ));
            BIP_Socket_Destroy( hSocket );
            bipStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        BDBG_MSG(( BIP_MSG_PRE_FMT "Accepted Connx: hSocket %p, hHttpSocket %p" BIP_MSG_PRE_ARG, hSocket, hHttpSocket ));

        /* Now Setup the RequestReceived Callback for this HttpSocket. */
        BIP_HttpSocket_GetSettings( hHttpSocket, &httpSocketSettings );
        httpSocketSettings.requestReceivedCallback.callback = callbackFromBip;
        httpSocketSettings.requestReceivedCallback.context = hEventFromBip;
        httpSocketSettings.requestReceivedCallback.param = 1;
        bipStatus = BIP_HttpSocket_SetSettings( hHttpSocket, &httpSocketSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SetSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Wait for HttpRequest. */
    {
        berr = B_Event_Wait( hEventFromBip, 1000 );
        BIP_CHECK_GOTO(( berr == B_ERROR_SUCCESS ), ( "Client didn't send HTTP Request after connecting, so closing BIP HttpSocket %p", hHttpSocket ), error, BIP_ERR_INITIAL_REQUEST_TIMEDOUT, bipStatus );
    }

    /* Receive Request. */
    {
        BIP_HttpSocketRecvRequestSettings recvRequestSettings;
        BIP_HttpRequestMethod requestMethod;
        const char *pRequestMethodName;

        hHttpRequest = NULL;
        hHttpResponse = NULL;
        BIP_HttpSocket_GetDefaultRecvRequestSettings( &recvRequestSettings );
        recvRequestSettings.timeoutInMs = 0; /* non-blocking */
        bipStatus = BIP_HttpSocket_RecvRequest( hHttpSocket, &hHttpRequest, &hHttpResponse, &recvRequestSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_RecvRequest Failed" ), error, bipStatus, bipStatus );

        bipStatus = BIP_HttpRequest_GetMethod(hHttpRequest, &requestMethod, &pRequestMethodName);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), error, bipStatus, bipStatus );

        BDBG_MSG((BIP_MSG_PRE_FMT "Request Method %s" BIP_MSG_PRE_ARG, pRequestMethodName ));

    }

    /* Send Response. */
    {
        BIP_HttpSocketSendResponseSettings sendResponseSettings;

        /* Prepare the Error Response. */
        BIP_HttpResponse_Clear( hHttpResponse, NULL );
        BIP_HttpResponse_SetStatus( hHttpResponse, BIP_HttpResponseStatus_e200_OK );

        BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
        sendResponseSettings.timeoutInMs = -1; /* blocking call, should complete immediately. */

        bipStatus = BIP_HttpSocket_SendResponse(hHttpSocket, hHttpRequest, hHttpResponse, BUFFER_SIZE, &sendResponseSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendResponse Failed" ), error, bipStatus, bipStatus );
    }

    /* Send Payload. */
    {
        BIP_HttpSocketSendPayloadSettings sendPayloadSettings;

        BKNI_Memset( buffer, 0, BUFFER_SIZE );
        BIP_HttpSocket_GetDefaultSendPayloadSettings( &sendPayloadSettings );
        sendPayloadSettings.timeoutInMs = -1; /* blocking call, should complete immediately for <250K write size. */
        /*
           bipStatus = BIP_HttpSocket_SendPayload(hHttpSocket, (uint8_t *)TEST_PAYLOAD_STRING, strlen(TEST_PAYLOAD_STRING), &sendPayloadSettings);
           */
        bipStatus = BIP_HttpSocket_SendPayload(hHttpSocket, (uint8_t *)buffer, BUFFER_SIZE, &sendPayloadSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendPayload Failed" ), error, bipStatus, bipStatus );
    }

    /* Done sending, so destroy it. */
    {
        BIP_HttpSocket_Destroy( hHttpSocket );
        hHttpSocket = NULL;
    }

    BDBG_LOG(( BIP_MSG_PRE_FMT " Shutting HttpSocketServer down..." BIP_MSG_PRE_ARG ));

error:
    if (hHttpSocket) BIP_HttpSocket_Destroy( hHttpSocket );

    /* Stop & Destroy Listener. */
    if ( hListener )
    {
        BIP_Listener_Stop( hListener );
        BIP_Listener_Destroy( hListener );
    }

    if ( hEventFromBip ) B_Event_Destroy ( hEventFromBip );

    NEXUS_Platform_Uninit();
errorBipInit:
    BIP_Uninit();

    BDBG_LOG(( "All done!" ));
    return (0); /* main */
}
