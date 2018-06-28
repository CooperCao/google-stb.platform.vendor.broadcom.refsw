/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "bip.h"
#include "rest.h"
#include "rest_priv.h"
#include "bmon.h"
#include "bmon_openssl.h"
#include "plugin_launcher.h"

BDBG_MODULE( rest );

#define TEST_PAYLOAD_STRING "This is a test payload string."
#define URL_MAX_LEN         256
#define MYPID               ( pthread_self()&0xffffff )

static BIP_SocketHandle hSocket              = NULL;
BIP_ListenerHandle      hListener            = NULL;
static char             HtmlResponseHead[]   = "<!DOCTYPE html>\n<html lang=\"en\"><head><title>http_socket test</title></html><body>";
static char             HtmlResponseEnding[] = "</body></html>\n";

static char *Rest_DateStr(
    void
    )
{
    static char    fmt [64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));
    gettimeofday( &tv, NULL );
    if (( tm = gmtime( &tv.tv_sec )) != NULL)
    {
        strftime( fmt, sizeof fmt, "%a, %d %b %Y %T %Z", tm );
    }

    return( fmt );
}                                                          /* Rest_DateStr */

static void Rest_connect_count ( void )
{
    static unsigned long long connect_count      = 0;
    if ( (connect_count%1000) == 0 ) BDBG_LOG(( "connections %8llu ... %s", connect_count, Rest_DateStr() ));
    connect_count++;

    return;
}

static int Rest_GetSocketFd(
    BIP_HttpSocketHandle hHttpSocket
    )
{
    int socketFd = 0;

    if (hHttpSocket)
    {
        int                  res = 0;
        BIP_HttpSocketStatus SocketStatus;
        BIP_HttpSocket_GetStatus( hHttpSocket, &SocketStatus );
        socketFd = SocketStatus.socketFd;
        BDBG_MSG(( "%s - L%d socket %d ... local (%s) remote (%s)", __FUNCTION__, __LINE__, SocketStatus.socketFd,
                   SocketStatus.pLocalIpAddress, SocketStatus.pRemoteIpAddress ));
    }

    return( socketFd );
}

static int Rest_GetRemoteIpAddress(
    BIP_HttpSocketHandle hHttpSocket,
    char                *pRemoteAddress,
    int                  lRemoteAddressLen
    )
{
    if (hHttpSocket)
    {
        int                  res = 0;
        BIP_HttpSocketStatus SocketStatus;
        BIP_HttpSocket_GetStatus( hHttpSocket, &SocketStatus );
        if (pRemoteAddress && ( lRemoteAddressLen > strlen( SocketStatus.pRemoteIpAddress )))
        {
            strncpy( pRemoteAddress, SocketStatus.pRemoteIpAddress, lRemoteAddressLen );
            BDBG_MSG(( "%s - L%d local (%s) remote (%s)", __FUNCTION__, __LINE__, SocketStatus.pLocalIpAddress, SocketStatus.pRemoteIpAddress ));
        }
    }

    return( 0 );
}

static void Rest_SigIntHandler(
    void
    )
{
    BDBG_MSG(( "%s", __FUNCTION__ ));

    /* Stop & Destroy Listener. */
    if (hListener)
    {
        BDBG_MSG(( "%s ... BIP_Listener_Stop(%p)", __FUNCTION__, hListener ));
        BIP_Listener_Stop( hListener );
        BDBG_MSG(( "%s ... BIP_Listener_Destroy(%p)", __FUNCTION__, hListener ));
        BIP_Listener_Destroy( hListener );
    }

    BIP_Uninit();

    BDBG_MSG(( "%s - ALL DONE", __FUNCTION__ ));

    exit( 0 );
}                                                          /* Rest_SigIntHandler */

static void Rest_Callback_HttpReceive(
    void *context,
    int   param
    )
{
    BIP_Status    bipStatus;
    B_EventHandle event = context;

    BDBG_MSG(( "%s - param %d ... context 0x%p", __FUNCTION__, param, context ));

    if (event) {B_Event_Set( event ); }

error:

    return;
}

/**
 *  Function: This function will compute the milliseconds between when the HTTP GET request was recieved and when
 *            the fork() API was successful. It will also compute the time from when the HTTP GET request was
 *            received and when the plugin has completed returning its JSON data to the rest server thread. Once
 *            these two values have been computed, this function will append the two deltas to the outgoing JSON
 *            string.
 **/
static int Rest_AppendDurationDeltas(
    char          *pPayload,
    int            lPayloadSize,
    struct timeval tv1,
    struct timeval tv2,
    struct timeval tv3,
    int            sizePayload
    )
{
    unsigned long long int microseconds1       = 0;
    unsigned long long int microseconds2       = 0;
    unsigned long long int microseconds3       = 0;
    unsigned long int      microseconds1_delta = 0;
    unsigned long int      microseconds2_delta = 0;
    unsigned long int      pPayload_len        = 0;
    char                   newline[2]          = "";
    char                   durationToFork[32];
    char                   durationToSend[32];

    if (pPayload == NULL) {return( sizePayload ); }

    pPayload_len        = strlen( pPayload );
    microseconds1       = ( tv1.tv_sec * 1000000LL ) + tv1.tv_usec; /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds2       = ( tv2.tv_sec * 1000000LL ) + tv2.tv_usec; /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds3       = ( tv3.tv_sec * 1000000LL ) + tv3.tv_usec; /* q-scale shift the seconds left to allow for addition of microseconds */
    microseconds1_delta = ( microseconds2 - microseconds1 );
    microseconds2_delta = ( microseconds3 - microseconds1 );
    BDBG_MSG(( "tv1 %lu.%06lu ... tv2 %lu.%06lu ... tv3 %lu.%06lu", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec, tv3.tv_sec, tv3.tv_usec ));

    if (pPayload_len > 2)
    {
        char *end_of_payload = &pPayload[pPayload_len - 1]; /* find " }" at end of payload string */
        if (*end_of_payload == '\n')
        {
            newline[0] = '\n';
            newline[1] = '\0';
            end_of_payload--;
        }

        if (*end_of_payload == '}')
        {
            *end_of_payload = '\0';

            sprintf( durationToFork, ",\"duration_to_fork\":\"%ld\"", microseconds1_delta/1000 /* convert to milliseconds */ );
            sprintf( durationToSend, ",\"duration_to_send\":\"%ld\"}%s", microseconds2_delta/1000 /* convert to milliseconds */, newline );
            BDBG_MSG(( "payload size (%d) ... len (%lu) ... duration1 (%s) (%s)", lPayloadSize, pPayload_len, durationToFork, durationToSend ));

            strncat( pPayload, durationToFork, lPayloadSize - strlen( pPayload ) - 1 );
            strncat( pPayload, durationToSend, lPayloadSize - strlen( pPayload ) - 1 );

            sizePayload = strlen( pPayload );

            BDBG_MSG(( "payload size (%d) ... len (%lu)", lPayloadSize, strlen( pPayload )));
        }
    }

    return( sizePayload );
}                                                          /* Rest_AppendDurationDeltas */

static void *Rest_ProcessHttpRequestThread(
    void *data
    )
{
    int                    idx   = 0;
    int                    bytes = 0;
    int                    rc    = 0;
    int                    socketFd   = 0;
    B_Error                berr;
    Bmon_ProcessThread_t  *pcontext           = (Bmon_ProcessThread_t *)data;
    BIP_HttpResponseStatus HttpResponseStatus = BIP_HttpResponseStatus_e200_OK;
    BIP_Status             bipStatus          = BIP_ERR_INTERNAL;
    char                   DateNow[64];
    struct timeval         tv1 = {0, 0};
    struct timeval         tv2 = {0, 0};
    struct timeval         tv3 = {0, 0};
    unsigned long long int microseconds1       = 0;
    unsigned long long int microseconds2       = 0;
    unsigned long int      microseconds_delta  = 0;
    char                  *plugin_name         = NULL;
    char                  *plugin_args         = NULL;
    char                  *token_ptr           = NULL;
    char                  *pPayload            = NULL;
    int                    sizePayload         = 0;
    BIP_SocketHandle       hSocket             = NULL;
    BIP_HttpSocketHandle   hHttpSocket         = NULL;
    BIP_HttpResponseHandle hHttpResponse;
    BIP_HttpRequestHandle  hHttpRequest;
    char                  *pTmpUrlPath         = NULL;
    B_EventHandle          hEventFromCallback2 = NULL;
    const char            *pUserAgentValue     = NULL;
    BIP_HttpHeaderHandle   hHeader             = NULL;
    char                   lRemoteIpAddress[INET6_ADDRSTRLEN];

    gettimeofday( &tv1, NULL );
    BDBG_MSG(( "%s - pcontext %p ... data (%p)", __FUNCTION__, pcontext, data ));

    rc = pthread_detach( pthread_self());
    BIP_CHECK_GOTO(( rc == 0 ), ( "pthread_detach() Failed" ), error, errno, rc );

    strncpy( DateNow, Rest_DateStr(), sizeof( DateNow ));

    /* Wait for new connection & Accept it. */
    {
        BIP_HttpSocketSettings httpSocketSettings;

        /* Got the event, so Accept the Connection. */
        BDBG_MSG(( "%s - L%u BIP_Listener_Accept()", __FUNCTION__, __LINE__ ));
        hSocket = BIP_Listener_Accept( hListener, 0 );
        BIP_CHECK_GOTO(( hSocket != NULL ), ( "BIP_Listener_Accept Failed" ), error, BIP_ERR_INTERNAL, bipStatus );

        Rest_connect_count(); /* debug */

        /* Create HTTP Socket. */
        hHttpSocket = BIP_HttpSocket_CreateFromBipSocket( hSocket, NULL );
        if (hHttpSocket == NULL)
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "BIP_HttpSocket_CreateFromBipSocket Failed" BIP_MSG_PRE_ARG ));
            BIP_Socket_Destroy( hSocket );
            bipStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
            goto error;
        }
        socketFd = Rest_GetSocketFd( hHttpSocket );
        BDBG_MSG(( "%s - L%u BIP_HttpSocket_CreateFromBipSocket(%p)", __FUNCTION__, __LINE__, hHttpSocket ));
        BDBG_MSG(( "%s - L%u Accepted Connx: hSocket %p, hHttpSocket %p ... socketFd %d", __FUNCTION__, __LINE__,
                   (void *)hSocket, (void *)hHttpSocket, Rest_GetSocketFd(hHttpSocket) ));
        BIP_CHECK_GOTO(( socketFd != 0 ), ( "hHttpSocket->socketFd is zero" ), error, BIP_ERR_INTERNAL, bipStatus );

        /* Create Event */
        hEventFromCallback2 = B_Event_Create( NULL );
        BIP_CHECK_GOTO(( hEventFromCallback2 != NULL ), ( "B_Event_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        BDBG_MSG(( "%s ... B_Event_Create() returned (%p)", __FUNCTION__, hEventFromCallback2 ));

        /* Now Setup the RequestReceived Callback for this HttpSocket. */
        BIP_HttpSocket_GetSettings( hHttpSocket, &httpSocketSettings );
        httpSocketSettings.requestReceivedCallback.callback = Rest_Callback_HttpReceive;
        httpSocketSettings.requestReceivedCallback.context  = hEventFromCallback2;
        httpSocketSettings.requestReceivedCallback.param    = 1;
        bipStatus = BIP_HttpSocket_SetSettings( hHttpSocket, &httpSocketSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SetSettings Failed" ), error, bipStatus, bipStatus );
    }

    /* Wait for HttpRequest. */
    {
        BDBG_MSG(( "%s - L%u B_Event_Wait(%p, 1000 )", __FUNCTION__, __LINE__, hEventFromCallback2 ));
        berr = B_Event_Wait( hEventFromCallback2, 1000 );
        BIP_CHECK_GOTO(( berr == B_ERROR_SUCCESS ), ( "Client didn't send HTTP Request after connecting, so closing BIP HttpSocket %p", (void *)hHttpSocket ), error, BIP_ERR_INITIAL_REQUEST_TIMEDOUT, bipStatus );
        BDBG_MSG(( "%s - L%u B_Event_Wait() #2 returned success", __FUNCTION__, __LINE__ ));
    }

    /* Receive Request. */
    {
        const char                       *pRequestMethodName = NULL;
        BIP_HttpHeaderHandle              hHeader            = NULL;
        BIP_HttpSocketRecvRequestSettings recvRequestSettings;
        BIP_HttpRequestMethod             requestMethod;

        hHttpRequest  = NULL;
        hHttpResponse = NULL;
        BIP_HttpSocket_GetDefaultRecvRequestSettings( &recvRequestSettings );
        recvRequestSettings.timeoutInMs = 0;               /* non-blocking */
        bipStatus = BIP_HttpSocket_RecvRequest( hHttpSocket, &hHttpRequest, &hHttpResponse, &recvRequestSettings );
        BDBG_MSG(( "%s - L%u BIP_HttpSocket_RecvRequest() ... bipStatus %d", __FUNCTION__, __LINE__, bipStatus ));
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_RecvRequest Failed" ), error, bipStatus, bipStatus );

        bipStatus = BIP_HttpRequest_GetMethod( hHttpRequest, &requestMethod, &pRequestMethodName );
        BDBG_MSG(( "%s - L%u BIP_HttpRequest_GetMethod() ... bipStatus %d", __FUNCTION__, __LINE__, bipStatus ));
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetMethod Failed" ), error, bipStatus, bipStatus );

        /* Retrieve the requested URL */
        bipStatus = BIP_HttpRequest_GetTarget( hHttpRequest, (const char **) &pTmpUrlPath );
        BDBG_MSG(( "%s - L%u Request Method %s ... url (%s) ... socket (%d)", __FUNCTION__, __LINE__, pRequestMethodName,  pTmpUrlPath, Rest_GetSocketFd(hHttpSocket) ));

        /*BIP_HttpRequest_Print( hHttpRequest, NULL, NULL );*/

        bipStatus = BIP_HttpRequest_GetNextHeader( hHttpRequest, NULL, "User-Agent", &hHeader, &pUserAgentValue );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpRequest_GetNextHeader() Failed" ), error, bipStatus, bipStatus );
        memset( &lRemoteIpAddress, 0, sizeof( lRemoteIpAddress ));
        Rest_GetRemoteIpAddress( hHttpSocket, lRemoteIpAddress, sizeof( lRemoteIpAddress ));
        BDBG_MSG(( "lRemoteIpAddress (%s)", lRemoteIpAddress ));

        BDBG_MSG(( "pTmpUrl (%s) ... hHttpSocket %p ... hHttpRequest %p ... hHttpResponse %p", pTmpUrlPath, hHttpSocket, hHttpRequest, hHttpResponse ));
        BDBG_MSG(( "pTmpUrl (%s) ... socket (%d) ... pthread %x", pTmpUrlPath, Rest_GetSocketFd(hHttpSocket), pthread_self() ));fflush(stderr);fflush(stdout);
    }

    /* Launch plugin executable */
    {
        plugin_launcher_t *p_launcher_params = NULL;

        p_launcher_params = calloc(1, sizeof(plugin_launcher_t) );
        BIP_CHECK_GOTO(( p_launcher_params ), ( "Memory Allocation Failed for p_launcher_params" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        pPayload = calloc( 1, REST_BUFFER_SIZE );
        BIP_CHECK_GOTO(( pPayload ), ( "Memory Allocation Failed for pPayload" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        BDBG_MSG(( "B_Os_Calloc() for pPayload %p ... socket (%d) ... REST_BUFFER_SIZE %d", pPayload, Rest_GetSocketFd(hHttpSocket), REST_BUFFER_SIZE ));

        /* fill in the launcher's required parameters */
        p_launcher_params->server_port = atoi( DEFAULT_REST_PORT_STRING );
        p_launcher_params->p_payload = pPayload;
        p_launcher_params->size_payload = REST_BUFFER_SIZE;
        strncpy( p_launcher_params->uri, pTmpUrlPath, sizeof(p_launcher_params->uri) - 1 );
        strncpy( p_launcher_params->s_user_agent, pUserAgentValue, sizeof(p_launcher_params->s_user_agent) - 1 );
        strncpy( p_launcher_params->s_remote_ip_address, lRemoteIpAddress, sizeof(p_launcher_params->s_remote_ip_address) - 1 );

        BDBG_MSG(( "plugin_launch(%s) ... sizePayload (%d)", p_launcher_params->uri, p_launcher_params->size_payload ));fflush(stderr);fflush(stdout);
        BDBG_MSG(( "plugin_launch() ... pPayload (%p) ... remote (%s)", p_launcher_params->p_payload, p_launcher_params->s_remote_ip_address ));fflush(stderr);fflush(stdout);
        BDBG_MSG(( "plugin_launch() ... sUserAgent (%s)", p_launcher_params->s_user_agent ));fflush(stderr);fflush(stdout);
        BDBG_MSG(( "plugin_launch() ... socket (%d) ... pthread %x", Rest_GetSocketFd(hHttpSocket), pthread_self() ));fflush(stderr);fflush(stdout);

        /* launch the plugin and let the plugin fill in the payload buffer */
        sizePayload = plugin_launcher( p_launcher_params );
        BDBG_MSG(( "After plugin_launch pid %d ... (%s) ... sizePayload (%d)", MYPID, pTmpUrlPath, sizePayload ));fflush(stderr);fflush(stdout);
        if ( sizePayload == 0 ) { /* PARENT failed to fork() and execve() the plugin */
            /* 2018-06-26 - the plugin launcher will fill-in the payload with a valid JSON-formatted error message */
        } else if ( sizePayload < 0 ) { /* CHILD failed to execve() */
            /* Something unexpected when wrong in the CHILD thread handling the plugin. We are never supposed to get here. */
            BDBG_MSG(( "After plugin_launch ... sizePayload (%d) ... (%s)", sizePayload, pPayload ));fflush(stderr);fflush(stdout);
            BDBG_MSG(( "CHILD exiting" ));fflush(stderr);fflush(stdout);
            pthread_exit( 0 );
        } else { /* PARENT successfully launched plugin and plugin responded with some data */
        }
        BDBG_MSG(( "After plugin_launch ... sizePayload (%d) ", sizePayload ));fflush(stderr);fflush(stdout);
        BDBG_MSG(( "After plugin_launch ... socket (%d) ... pthread %x", Rest_GetSocketFd(hHttpSocket), pthread_self() ));fflush(stderr);fflush(stdout);
        fflush(stdout);fflush(stderr);
        BIP_CHECK_GOTO(( sizePayload > 0 ), ( "plugin_launcher() Failed" ), error, BIP_ERR_INTERNAL, sizePayload );

        if ( p_launcher_params ) {
            free( p_launcher_params );
            p_launcher_params = NULL;
        }
    }

    BIP_CHECK_GOTO( (Rest_GetSocketFd(hHttpSocket)> 0), ( "hHttpSocket fd is zero" ), error, BIP_ERR_INTERNAL, sizePayload );

    /* Send Response Headers. */
    {
        BIP_HttpHeaderHandle               hHeader = NULL;
        BIP_HttpSocketSendResponseSettings sendResponseSettings;
        BIP_HttpResponseHttpVersion        HttpResponseVersion = {1, 0};

        /* Prepare the Error Response. */
        BIP_HttpResponse_Clear( hHttpResponse, NULL );
        BIP_HttpResponse_SetStatus( hHttpResponse, HttpResponseStatus );

        BIP_HttpSocket_GetDefaultSendResponseSettings( &sendResponseSettings );
        sendResponseSettings.timeoutInMs = -1;             /* blocking call, should complete immediately. */

        /*BIP_HttpResponse_SetHttpVersion( hHttpResponse, &HttpResponseVersion );*/

        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Content-type", "text/html", NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader(Content-type) Failed" ), error, errno, errno );

        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Date", DateNow, NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader(Date) Failed" ), error, errno, errno );

        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Connection", "close", NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader(Connection) Failed" ), error, errno, errno );

        /* Access-Control-Allow-Origin is a header sent in a server response which indicates that the client is allowed
           to see the contents of a result */
        hHeader = BIP_HttpResponse_AddHeader( hHttpResponse, "Access-Control-Allow-Origin", "*", NULL );
        BIP_CHECK_GOTO(( hHeader ), ( "BIP_HttpResponse_AddHeader(Access-Control-Allow-Origin) Failed" ), error, errno, errno );

        BDBG_MSG(( "BIP_HttpSocket_SendResponse() ... hHttpSocket %p; hHttpRequest %p; hHttpResponse %p; sizePayload %d",
                   hHttpSocket, hHttpRequest, hHttpResponse, sizePayload ));
        fflush(stdout);fflush(stderr);
        bipStatus = BIP_HttpSocket_SendResponse( hHttpSocket, hHttpRequest, hHttpResponse, sizePayload, &sendResponseSettings );
        BDBG_MSG(( "BIP_HttpSocket_SendResponse() ... bipStatus %d", bipStatus ));
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendResponse Failed" ), error, bipStatus, bipStatus );

        /*BIP_HttpResponse_Print( hHttpResponse, NULL, NULL );
        fflush(stdout);fflush(stderr);*/
    }

    /* Send Payload. */
    {
        BIP_HttpSocketSendPayloadSettings sendPayloadSettings;

        BIP_HttpSocket_GetDefaultSendPayloadSettings( &sendPayloadSettings );
        sendPayloadSettings.timeoutInMs = -1;              /* blocking call, should complete immediately for <250K write size. */
        BDBG_MSG(( "L%u - BIP_HttpSocket_SendPayload() ... size (%d)", __LINE__, sizePayload ));
        fflush(stdout);fflush(stderr);

        bipStatus = BIP_HttpSocket_SendPayload( hHttpSocket, (uint8_t *)pPayload, sizePayload, &sendPayloadSettings );
        if (bipStatus != BIP_SUCCESS)
        {
            BDBG_SetModuleLevel( "bip_http_socket", BDBG_eMsg );
            BDBG_SetModuleLevel( "bip_http_request", BDBG_eMsg );
            BDBG_SetModuleLevel( "bip_http_response", BDBG_eMsg );
        }
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_HttpSocket_SendPayload Failed" ), error, bipStatus, bipStatus );
        BDBG_MSG(( "L%u - BIP_HttpSocket_SendPayload() ... size (%d) ... DONE", __LINE__, sizePayload ));
        fflush(stdout);fflush(stderr);
    }

error:

    /* Done sending, so destroy it. */
    {
        struct timeval tv = {0, 0};
        BDBG_MSG(( "BIP_HttpSocket_Destroy(%p) ", __LINE__, hHttpSocket ));
        if (hHttpSocket) {BIP_HttpSocket_Destroy( hHttpSocket ); }
        hHttpSocket = NULL;

        gettimeofday( &tv, NULL );
        microseconds2      = ( tv.tv_sec * 1000000LL );    /* q-scale shift the seconds left to allow for addition of microseconds */
        microseconds2     += tv.tv_usec;                   /* add in microseconds */
        microseconds_delta = ( microseconds2 - microseconds1 );

        BDBG_MSG(( "B_OS_FREE(%p) pcontext ", pcontext ));
        B_OS_FREE( pcontext );

        if (pPayload) {free( pPayload ); }
    }

    if (hEventFromCallback2)
    {
        BDBG_MSG(( "%s ... B_Event_Destroy(%p)", __FUNCTION__, hEventFromCallback2 ));
        B_Event_Destroy( hEventFromCallback2 );
        hEventFromCallback2 = NULL;
    }

    fflush( stderr ); fflush( stdout );

    pthread_exit( 0 );
}                                                          /* Rest_ProcessHttpRequestThread */

static void Rest_Callback_NewConnection(
    void *context,
    int   param
    )
{
    static unsigned long int NewConnectionCount = 0;
    BIP_Status               bipStatus          = BIP_ERR_INTERNAL;
    BIP_ListenerHandle       hListener          = context;

    fflush( stdout ); fflush( stderr );
    BDBG_MSG(( " " ));
    BDBG_MSG(( "%s - param %d ... hListener 0x%x ... count %lu", __FUNCTION__, param, hListener, NewConnectionCount ));
    NewConnectionCount++;

    {
        pthread_t                ProcessHttpRequestThreadId = 0;
        void                    *(*threadFunc)( void * );
        static unsigned long int threadOption = 0;

        Bmon_ProcessThread_t *pcontext = B_Os_Calloc( 1, sizeof( *pcontext ));
        BIP_CHECK_GOTO(( pcontext ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        pcontext->hListener = hListener;

        threadFunc   = Rest_ProcessHttpRequestThread;
        threadOption = (unsigned long int)pcontext;
        /*BDBG_LOG(( "%s  - pcontext %p ... threadOption(%lx)", __FUNCTION__, pcontext, threadOption ));*/

        if (pthread_create( &ProcessHttpRequestThreadId, NULL, threadFunc, (void *)threadOption ))
        {
            BDBG_ERR(( "pthread_create failed; %s", strerror( errno )));
        }
        else
        {
            BDBG_MSG(( "pthread_created succeeded" ));
        }
    }

error:
    return;
}                                                          /* Rest_Callback_NewConnection */

int main(
    int   argc,
    char *argv[]
    )
{
    int            rc      = 0;
    int            counter = 0;
    BIP_Status     bipStatus;
    NEXUS_Error    nrc;
    struct timeval tv = {0, 0};

    signal( SIGINT,  (__sighandler_t) Rest_SigIntHandler );

    /* Initialize BIP */
    {
        bipStatus = BIP_Init( NULL );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    /* Create & Start Listener. */
    {
        BIP_ListenerCreateSettings listenerCreateSettings;
        BIP_ListenerSettings       listenerSettings;
        BIP_ListenerStartSettings  listenerStartSettings;

        /* Create Listener */
        BIP_Listener_GetDefaultCreateSettings( &listenerCreateSettings );
        hListener = BIP_Listener_Create( &listenerCreateSettings );
        BIP_CHECK_GOTO(( hListener != NULL ), ( "BIP_Listener_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        BIP_Listener_GetSettings( hListener, &listenerSettings );
        listenerSettings.connectedCallback.callback = Rest_Callback_NewConnection;
        listenerSettings.connectedCallback.context  = hListener;
        listenerSettings.connectedCallback.param    = 0;
        bipStatus = BIP_Listener_SetSettings( hListener, &listenerSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Listener_SetSettings Failed" ), error, bipStatus, bipStatus );

        BIP_Listener_GetDefaultStartSettings( &listenerStartSettings );
        listenerStartSettings.pPort = DEFAULT_REST_PORT_STRING;
        bipStatus = BIP_Listener_Start( hListener, &listenerStartSettings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Listener_Start Failed" ), error, bipStatus, bipStatus );
        BDBG_MSG(( "Listener started on port %s", listenerStartSettings.pPort ));
    }

    while (1)
    {
        sleep( 1 );
        counter++;
    }                                                      /* while */

error:
    Rest_SigIntHandler();

errorBipInit:
    BIP_Uninit();

    BDBG_MSG(( "All done!" ));
    return( 0 );                                           /* main */
} /* main */
