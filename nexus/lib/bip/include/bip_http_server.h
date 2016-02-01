/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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

#ifndef BIP_HTTP_SERVER_H
#define BIP_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"

/** @addtogroup bip_http_server
 *
 * BIP_Server Interface Definition.
 *
 * Although developed in C, the BIP library has an object-based design,
 * with various objects being referenced by passing an object handles as
 * the first parameter to the C API calls.
 *
 * For Server side, BIP_HttpServer is the main interface providing the
 * HTTP Server implementation.  The data structures & API defined here
 * allows apps to start an HTTP Server that listens for HTTP Requests from
 * clients.
 *
 * BIP internally accepts an incoming TCP connection from a client, then
 * reads the initial set of bytes until a full HTTP Request is received.
 * When the new HTTP Request is received, BIP notifies the App via
 * BIP_HttpServerSettings.messageReceivedCallback or via completion
 * callback for an async _RecvRequest call.
 *
 * An App receives the request using the BIP_HttpServer_RecvRequest() API
 * and parses it using the BIP_HttpRequest helper class to obtain the
 * requested URL.
 *
 * If the App determines that the incoming HttpRequest is invalid and
 * cannot be satisfied, it can reject it by calling
 * BIP_HttpServer_RejectRequest() which results in an appropriate
 * HttpResponse being returned to the originating client.
 *
 * If the incoming HttpRequest is valid and is requesting media streaming,
 * the App sets up NEXUS if needed (e.g.  Frontend needs to be started for
 * streaming from live tuner).  Then, streaming is started by either
 * creating a new streamer object using BIP_HttpServer_CreateStreamer or
 * using an existing streaming handle and then starting it using
 * BIP_HttpServer_StartStreamer() API.
 *
 * Once streaming session is complete, the App is notified by an
 * endOfStreamingCallback indicating the streaming completion. It then
 * calls BIP_HttpServer_StopStreamer() to let BIP HTTP Server free up
 * any streaming related internal resources.
 *
 * App can also call BIP_HttpServer_StopStreamer() anytime to proactively
 * stop a streamer (e.g. when app is being shutdown.).
 *
 * App can either destroy the streamer using BIP_HttpServer_DestroyStreamer()
 * or retain the streamer handle for use with the next HTTP Request.  This
 * may be required for avoiding Nexus Heap memory fragmentation as
 * BIP_HttpServer_CreateStreamer() may open & use some Nexus resources
 * such as Playpump, Recpump, ASP, etc.  depending upon the streaming
 * input & output settings.
 *
 * Note: Since BIP will internally open some required Nexus Resources for
 * Streaming purposes such as Playpump (Playback Channel) , Recpump (RAVE
 * Ctx), and ASP (Advanced Streaming Processor) (when it becomes available
 * in h/w), Apps are required to not manage the indices for these Nexus
 * resources and INSTEAD APP MUST USE NEXUS_ANY_ID for obtaining any of
 * these resources for its usage.
 *
 * This is being done for the following reasons:
 *
 * - it simplifies app logic
 * where it only has to setup the Nexus Frontend for live inputs.
 * Creation of PID Channels, adding them to Recpump, & starting Recpump is
 * all taken care by the BIP library.  App only need to provide the parser
 * band & pids that need to be streamed.
 *
 * - it further simplifies app logic where it doesn't have to edit the
 * Recpump Settings as per the particular streaming protocol and instead
 * BIP internally takes care of that.  For example, setting of the custom
 * dataReady thresholds for various streaming algorithms can be done by
 * BIP itself.  For HLS Segmentation purposes, TPIT indexing needs to be
 * enabled in Recpump as it is used for segment creation at the RAP
 * boundaries, etc.  BIP will still provide flexibility to control any
 * custom Recpump Settings by exposing the NEXUS_Recpump_Settings thru one
 * of its APIs.
 *
 * - likewise, it helps apps by not having to know when to use a Playback
 * Channel for streaming vs when to stream out a content directly.  BIP
 * can internally decide and pick the file -> playback -> recpump route if
 * it has to pace outgoing stream, remove some timestamps, remove CA
 * encryption and re-encrypt w/ DTCP/IP, etc.
 *
 * Given this requirement of opening the Nexus Handles internally, apps
 * must be aware of risk of Nexus Heap Memory Fragmentation if BIP
 * Streamers are frequently created and destroyed.  BIP HTTP Server API
 * design provides mechanism for lowering this fragmentation risk by
 * allowing just stopping & starting of the Streamer object for handling
 * subsequent HTTP Requests.
 *
 * Here are simple example apps demontrating the usage of BIP_HttpServer APIs:
 * \code
 * - bip/examples/http_media_server/http_simple_streamer.c  ---> Simple File Streaming Example
 * - bip/examples/http_media_server/http_file_streamer.c    ---> Example to stream files from a media directory
 * - bip/examples/http_media_server/http_qam_streamer.c     ---> Example to stream any program from a given QAM frequency
 * - bip/examples/http_media_server/http_sat_streamer.c     ---> Example to stream any program from a given SAT frequency
 * - bip/examples/http_media_server/http_recpump_streamer.c ---> Example to stream any program from a given Recpump input
 * \endcode
 */

/**
Summary:
Datatype to hold a reference to a BIP_HttpServer object.
**/
typedef struct BIP_HttpServer *BIP_HttpServerHandle;        /* Main HttpServer object */

/**
Summary:
Settings for BIP_HttpServerCreate().

See Also:
BIP_HttpServer_Create
BIP_HttpServer_GetDefaultCreateSettings
**/
typedef struct BIP_HttpServerCreateSettings
{
    BIP_SETTINGS(BIP_HttpServerCreateSettings) /* Internal use... for init verification. */

} BIP_HttpServerCreateSettings;

/**
Summary:
Get default settings for BIP_HttpServer_Create().

See Also:
BIP_HttpServer_Create
BIP_HttpServerCreateSettings
**/
void BIP_HttpServer_GetDefaultCreateSettings(
    BIP_HttpServerCreateSettings *pSettings
    );

/**
Summary:
Create an HttpServer

Description:
A BIP_HttpServer is responsible for:
-# Listening for incoming HTTP connections.
-# Receiving incoming HTTP Request Messages.
-# Sending outgoing HTTP Response Messages.
-# Starting BIP_Streamers to stream content.

Calling Context:
\code
    BIP_HttpServerHandle                 hHttpServer;
    BIP_HttpServerCreateSettings         createSettings;

    BIP_HttpServer_GetDefaultCreateSettings( &createSettings );
    createSettings.<settingName> =  <settingValue>;     //  Set any non-default settings as desired.
    hHttpServer = BIP_HttpServer_Create( &createSettings );
\endcode

Input: &createSettings  Optional address of a BIP_HttpServerCreateSettings structure. Passing NULL will use default settings.

Return:
    non-NULL :        A BIP_HttpServerHandle used for calling subsequent HttpServer related APIs
Return:
    NULL     :        Failure, an HttpServer instance could not be created.

See Also:
BIP_HttpServerCreateSettings
BIP_HttpServer_GetDefaultCreateSettings
BIP_HttpServer_Destroy
**/
BIP_HttpServerHandle BIP_HttpServer_Create(
    const BIP_HttpServerCreateSettings *pSettings
    );

/**
Summary:
Settings for BIP_HttpServer_SetSettings().

See Also:
BIP_HttpServer_GetSettings
BIP_HttpServer_SetSettings
**/
typedef struct BIP_HttpServerSettings
{
    BIP_SETTINGS(BIP_HttpServerSettings)   /* Internal use... for init verification. */

    BIP_CallbackDesc requestReceivedCallback;   /*!< Callback to indicate the arrival of an HTTP Request Header from a Client */
} BIP_HttpServerSettings;

/**
Summary:
Retrieve current settings before calling BIP_HttpServer_SetSettings().

See Also:
BIP_HttpServerSettings
BIP_HttpServer_SetSettings
**/
void BIP_HttpServer_GetSettings(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSettings *pSettings
    );

/**
Summary:
Change the current settings of a BIP_HttpServer.

Description:
Changes the run-time settings of a BIP_HttpServer object.  Before calling this
API, the current settings must first be retrived by calling BIP_HttpServer_GetSettings().

This API is used for changing settings that can be changed after the HttpServer is started.

Calling Context:
\code
    BIP_HttpServerHandle           hHttpServer;
    BIP_HttpServerSettings         settings;

    BIP_HttpServer_GetSettings( hHttpServer, &settings );
    settings.<settingName> =  <settingValue>;     //  Change settings as desired.
    rc = BIP_HttpServer_SetSettings( hHttpServer, &settings );
\endcode

Input: hHttpServer BIP_HttpServerHandle for the HttpServer being changed.
Input: &settings  BIP_HttpServerSettings struct that contains the settings being requested.

Return:
    BIP_SUCCESS : Normal successful completion.

See Also:
BIP_HttpServerSettings
BIP_HttpServer_GetSettings
**/
BIP_Status BIP_HttpServer_SetSettings(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerSettings *pSettings
    );

/**
Summary:
Settings for BIP_HttpServer_Start()

See Also:
BIP_DtcpIpServerStartSettings
BIP_HttpServer_GetDefaultStartSettings
BIP_HttpServer_Start
**/
typedef struct BIP_HttpServerStartSettings
{
    BIP_SETTINGS(BIP_HttpServerStartSettings)   /* Internal use... for init verification. */

    const char *pPort;                          /*!< Port on which HttpServer Listens for HTTP Requests from clients. */
                                                /*!< Defaults to port 80. */

    const char *pInterfaceName;                 /*!< Binds Media Server to this specific interface name and listens for HTTP Request exclusively on that interface. */
                                                /*!< Defaults to none, meaning HttpServer will listen on all interfaces! */

    BIP_NetworkAddressType ipAddressType;       /*!< If pInterfaceName is specified, then ipAddressType allows caller */
                                                /*!< To specify its perference about using the IP Address associated with this interface. */
                                                /*!< Apps can restrict to use just IPv4, IPv6, both IPv4 & IPv6, or IPv6 if available, otherwise IPv4 type of addresses. */
                                                /*!< This allows one instance of HttpServer to receive HTTP Requests on any type of IP Address. */

    unsigned    maxConcurrentRequestsToQueue;   /*!< Maximum HTTP Requests HttpServer will queue & will not accept new connections App calls BIP_HttpServer_RecvRequest to rcv a Request. */
                                                /*!< Defaults to 32 HttpRequests. */

    unsigned    persistentConnectionTimeoutInMs;/*!< Non-zero timeout values enables HTTP Persistent Connection support allowing multiple Requests & Responses to be carried over single TCP connection. */
                                                /*!< Timeout duration is in msec after which "idle" Http Sockets will be timed out (idle means HTTP Request hasn't arrived for this duration) */
                                                /*!< Defaults to 5000msec. */
                                                /*!< Value of 0 disables HTTP Persistent Connection support, i.e. Http Server will Close the TCP connection after the first HTTP Response Payload has been sent. */

    bool        enableDtcpIp;                   /*!< To stream media using DTCP/IP, this should be set to true. */
                                                /*!< Defaults to false: no DTCP/IP. */
    BIP_DtcpIpServerStartSettings dtcpIpServer; /*!< if enableDtcpIp is true, this structure should be filled-in with DTCP/IP Startup Settings. */

} BIP_HttpServerStartSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpServerStartSettings);

/**
Summary:
Get default settings for BIP_HttpServer_Start().

See Also:
BIP_HttpServerStartSettings
BIP_DtcpIpServerStartSettings
BIP_HttpServer_Start
**/
#define HTTP_SERVER_DEFAULT_LISTENING_PORT                  "80"
#define HTTP_SERVER_MAX_CONCURRENT_REQUESTS_RCVD            32
#define HTTP_SERVER_PERSISTENT_CONNECTION_TIMEOUT_IN_MSEC   5000


#define BIP_HttpServer_GetDefaultStartSettings(pSettings)                                                 \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpServerStartSettings)                            \
        /* Set non-zero defaults explicitly. */                                                           \
        (pSettings)->pPort = HTTP_SERVER_DEFAULT_LISTENING_PORT;                                          \
        (pSettings)->maxConcurrentRequestsToQueue = HTTP_SERVER_MAX_CONCURRENT_REQUESTS_RCVD;             \
        (pSettings)->persistentConnectionTimeoutInMs = HTTP_SERVER_PERSISTENT_CONNECTION_TIMEOUT_IN_MSEC; \
        BIP_DtcpIpServer_GetDefaultStartSettings(&(pSettings)->dtcpIpServer);                             \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Start a BIP_HttpServer

Description:
BIP will start listening on server socket and accept incoming HTTP requests after this call.

See Also:
BIP_HttpServerStartSettings
BIP_DtcpIpServerStartSettings
BIP_HttpServer_GetDefaultStartSettings
BIP_HttpServer_Stop
**/
BIP_Status BIP_HttpServer_Start(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerStartSettings *pSettings
    );

/**
Summary:
Stop a BIP_HttpServer

Description:
BIP will not accept any new connection requests after this call.

See Also:
BIP_HttpServer_Start
**/
BIP_Status BIP_HttpServer_Stop(
    BIP_HttpServerHandle hHttpServer
    );

/**
Summary:
Settings for BIP_HttpServer_RecvRequest().

See Also:
BIP_HttpServer_GetDefaultRecvRequestSettings
BIP_HttpServer_RecvRequest
*/

/* API Settings Structure: */
typedef struct BIP_HttpServerRecvRequestSettings
{
    BIP_SETTINGS(BIP_HttpServerRecvRequestSettings) /* Internal use... for init verification. */

    BIP_HttpSocketHandle  *phHttpSocket;  /*!< Optional: pointer of variable that will contain the HttpSocket handle associated with the incoming Request. Only set upon successful completion. */
} BIP_HttpServerRecvRequestSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpServerRecvRequestSettings);

/**
Summary:
Get default settings for BIP_HttpServer_RecvRequest().

See Also:
BIP_HttpServerRecvRequestSettings
BIP_HttpServer_RecvRequest
**/

#define BIP_HttpServer_GetDefaultRecvRequestSettings(pSettings)                             \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpServerRecvRequestSettings)        \
        /* Set non-zero defaults explicitly. */                                             \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Receive an incoming HTTP Request from a client.

Description:
This API attempts to return the next available HTTP request that has arrived. If no request is
available, it returns immediately with a status of BIP_INF_TIMEOUT.

Calling Context:
\code
    BIP_HttpRequestHandle                hHttpRequest  = NULL;
    BIP_HttpServerRecvRequestSettings    recvRequestSettings;
    BIP_Status                           bipStatus;

    BIP_HttpServer_GetDefaultRecvRequestSettings( &recvRequestSettings );
    recvRequestSettings.<settingName> =  <settingValue>;     //  Change settings as desired.
    bipStatus = BIP_HttpServer_RecvRequest( hHttpServer, &recvRequestSetings, &hHttpRequest );
\endcode

Input: hHttpServer BIP_HttpServerHandle for the HttpServer doing the receive.
Input: &recvRequestSetting optional BIP_HttpServerRecvRequestSettings for additional settings. Passing NULL will use default settings.
Output: &hHttpRequest  where to put the handle of received request.

Return:
    BIP_SUCCESS     : *phHttpRequest     contains a handle to the received BIP_HttpRequest object.
Return:
    BIP_INF_TIMEOUT : no HttpRequest is available.

Note: The Request handle is created internally by BIP and thus owned by BIP. App should only use it to
retrieve Headers from Request.

Note1: The Request handle handle remains valid until:
- App calls BIP_HttpServer_StopStreamer(), or
- App calls BIP_HttpServer_RejectRequest()

See Also:
BIP_HttpServerRecvRequestSettings
BIP_HttpServer_GetDefaultRecvRequestSettings
BIP_HttpServer_RejectRequest
*/
BIP_Status BIP_HttpServer_RecvRequest(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpRequestHandle  *phHttpRequest,  /*!< Required: where to put the handle of received request. Only set upon successful completion. */
    BIP_HttpServerRecvRequestSettings *pSettings
    );

/**
Summary:
Settings for BIP_HttpServer_RejectRequest().

See Also:
BIP_HttpServer_RejectRequest
BIP_HttpServer_GetDefaultRejectRequestSettings
**/
typedef struct BIP_HttpServerRejectRequestSettings
{
    BIP_SETTINGS(BIP_HttpServerRejectRequestSettings) /* Internal use... for init verification. */

    BIP_HttpResponseStatus  httpStatus;              /*!< Enums for 3-digit HTTP status-codes from RFC7231 (defined in bip_http_response.h ) */
                                                     /*!< Default is BIP_HttpResponseStatus_e400_BadRequest */
    const char             *customResponseHeaders;

    /* add the status code */
} BIP_HttpServerRejectRequestSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpServerRejectRequestSettings);

/**
Summary:
Get default settings for BIP_HttpServer_RejectRequest().

See Also:
BIP_HttpServerRejectRequestSettings
BIP_HttpServer_RejectRequest
**/

#define BIP_HttpServer_GetDefaultRejectRequestSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpServerRejectRequestSettings) \
        /* Set non-zero defaults explicitly. */                                        \
        (pSettings)->httpStatus = BIP_HttpResponseStatus_e400_BadRequest;              \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Reject an incoming BIP_HttpRequest

Description:
If the App determines that an incoming HttpRequest is invalid and/or
cannot be satisfied, it can call this API to reject the request with the
indicated HTTP status code.   This will cause an HTTP response to be
returned to the originating client and mark the request as being "completed".

Calling Context:
\code
    BIP_HttpServerHandle                 hHttpServer;
    BIP_HttpServerRejectRequestSettings  settings;
    BIP_HttpRequestHandle                hHttpRequest;
    BIP_Status                           bipStatus;

    BIP_HttpServer_GetDefaultRejectRequestSettings( &settings );
    settings.<settingName> =  <settingValue>;     //  Set any non-default settings as desired.
    bipStatus = BIP_HttpServer_RejectRequest( hHttpServer, hHttpRequest, &settings );
\endcode

Input: hHttpServer BIP_HttpServerHandle of HttpServer that owns the HttpRequest.
Input: hHttpRequest Handle of the BIP_HttpRequest being rejected.
Input: &rejectSettings Address of a BIP_HttpServerRejectRequestSettings struct for additional settings.  Passing NULL will use default values.

Return:
    BIP_SUCCESS :  An httpResponse has been sent to the originating client and the HttpRequest handle no longer valid for use by the App.

See Also:
BIP_HttpServerRejectRequestSettings
BIP_HttpServer_GetDefaultRejectRequestSettings
**/
BIP_Status BIP_HttpServer_RejectRequest(
    BIP_HttpServerHandle        hHttpServer,
    BIP_HttpRequestHandle    hHttpRequest,           /*!< Required: handle of  the HttpRequest being rejected. */
    BIP_HttpServerRejectRequestSettings *pSettings
    );

/**
Summary:
Settings for BIP_HttpServer_CreateStreamer().

See Also:
BIP_HttpServer_CreateStreamer
BIP_HttpServer_GetDefaultCreateStreamerSettings
**/
typedef struct BIP_HttpServerCreateStreamerSettings
{
    BIP_SETTINGS(BIP_HttpServerCreateStreamerSettings) /* Internal use... for init verification. */

    BIP_CallbackDesc    endOfStreamingCallback;   /*!< Callback to indicate that streaming has finished
                                                     for this HttpStreamer: can be issued when streaming is
                                                     completed or streaming fails (either due to client
                                                     channel change or any other network errors during
                                                     streaming). */
    BIP_CallbackDesc    softErrorCallback;        /*!< Callback to let app know about soft errors *not*
                                                     due to streaming (e.g. no data to stream from live
                                                     source, failure to read from the file, etc.). */
} BIP_HttpServerCreateStreamerSettings;

/**
Summary:
Get default settings for BIP_HttpServer_CreateStreamer().

See Also:
BIP_HttpServer_CreateStreamer
BIP_HttpServerCreateStreamerSettings
**/
void BIP_HttpServer_GetDefaultCreateStreamerSettings(
    BIP_HttpServerCreateStreamerSettings *pSettings
    );

/**
Summary:
Create an HttpStreamer

Description:
A BIP_HttpStreamer is responsible for:
-# Streaming media (or data) to one client.
-# Notifying the App (by callback) when the streaming is completed.

Note: App is responsible for calling BIP_HttpServer_DestoryStreamer after it receives
endoOfStreaming or softError Callbacks from BIP.

Note2: App *directly* calls the InputSettings & OutSettings Set APIs of BIP_HttpStreamer
interface. The other BIP_HttpStreamer API are invoked via the BIP_HttpServer class.


Calling Context:
\code
    BIP_HttpServerHandle                    hHttpServer;
    BIP_HttpStreamerHandle                  hHttpStreamer;
    BIP_HttpServerCreateStreamerSettings    createSettings;

    BIP_HttpServer_GetDefaultCreateStreamerSettings( &createSettings );
    createSettings.<settingName> =  <settingValue>;     //  Set any non-default settings as desired.
    hHttpStreamer = BIP_HttpServer_CreateStreamer( hHttpServer, &createSettings );
\endcode


Input: hHttpServer BIP_HttpServerHandle of HttpServer that will own the HttpStreamer.
Input: &createSettings  Optional address of a BIP_HttpServerCreateStreamerSettings structure. Passing NULL will use default settings.

Return:
    non-NULL :        A BIP_HttpStreamerHandle used for calling subsequent HttpServer and HttpStreamer related APIs
Return:
    NULL     :        Failure, an HttpStreamer instance could not be created.

See Also:
BIP_HttpServer_GetDefaultCreateStreamerSettings
BIP_HttpServerCreateStreamerSettings
BIP_HttpServer_DestroyStreamer
**/
BIP_HttpStreamerHandle BIP_HttpServer_CreateStreamer(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpServerCreateStreamerSettings *pSettings
    );


/**
Summary:
Settings for BIP_HttpServer_StartStreamer()

See Also:
BIP_HttpServerStartStreamerSettings
BIP_HttpServer_GetDefaultStartStreamerSettings
BIP_HttpServer_StartStreamer
*/
typedef struct BIP_HttpServerStartStreamerSettings
{
    BIP_SETTINGS(BIP_HttpServerStartStreamerSettings) /* Internal use... for init verification. */

    BIP_HttpStreamerStartSettings   streamerStartSettings;
} BIP_HttpServerStartStreamerSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpServerStartStreamerSettings );

/**
Summary:
Get default settings for BIP_HttpServer_StartStreamer().

See Also:
BIP_HttpServerStartStreamerSettings
BIP_HttpServer_StartStreamer
**/
#define BIP_HttpServer_GetDefaultStartStreamerSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpServerStartStreamerSettings ) \
        /* Set non-zero defaults explicitly. */                                  \
        BIP_HttpStreamer_GetDefaultStartSettings(&(pSettings)->streamerStartSettings); \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Start a BIP_HttpStreamer.

Description:
This API starts a streaming session. Upon notification of streaming completion (by callback), app must call
BIP_HttpServer_StopStreamer() to free up Streamer related resources. This can be done either
after app receives endOfStreamer Callback or when server is being shutdown.

NOTE:
App must call the BIP_HttpStreamer_Set[File|Tuner|Ip|Bfile]InputSettings()
and BIP_HttpStreamer_SetOutputSettings() APIs before calling this BIP_HttpServer_StartStreamer API.

InputSettings are specific to File|Tuner inputs.
Likewise, App must fill-in the StreamerOutput Settings as well.

NOTE2:
The APIs to set these input & output settings are directly exposed by the BIP_HttpStreamer Interface
as these are just the settings.

Streamer is still started by the BIP_HttpServer_StartStreamer().

NOTE3:
App is responsible for sending HTTP Error Response using the BIP_HttpServer_RejectRequest() upon failure.
If this API fails (return status != BIP_SUCCESS), app must specify the same Http Request handle (as used in this API)
in the BIP_HttpServer_RejectRequest() in order to send the error response back to the client.
That also allows BIP to internally free up resources associated with the HTTP Request.

Also, note that instead of sending an error response back to the client, App can make some "adjustments" such as
free up streamers that may be inactive due to client pause or crash and then retry the BIP_HttpServer_StartStreamer().

Calling Context:
\code
    BIP_HttpServerHandle                   hHttpServer;
    BIP_HttpStreamerHandle                 hHttpStreamer;
    BIP_HttpServerStartStreamerSettings    settings;
    BIP_Status                             bipStatus;

    BIP_HttpServer_GetDefaultStartStreamerSettings( &settings );
    settings.<settingName> =  <settingValue>;     //  Change settings as desired.
    bipStatus = BIP_HttpServer_StartStreamer( hHttpServer, hHttpStreamer, hHttpRequest, &settings );
\endcode
Input: hHttpServer BIP_HttpServerHandle of HttpServer that owns the HttpStreamer being started.
Input: hHttpStreamer BIP_HttpStreamerHandle of the HttpStreamer being started.
Input: hHttpRequest handle of the BIP_HttpRequest that has requested that the HttpStreamer be started.
Input: &startSettings Optional address of a BIP_HttpServerStartStreamerSettings structure with additional settings. Passing NULL will use default settings.
Return:
    BIP_SUCCESS :                   Normal successful completion. BIP will send the Successful HTTP Response to the client.
    BIP_INF_RESOURCE_NOT_AVAILBLE : Resource(s) needed for streaming not available. App can either stop other streamer or call BIP_HttpServer_RejectRequest().
    Other Error cases :             Please read the NOTE3 above!

See Also:
BIP_HttpServer_RejectRequest
BIP_HttpServerStartStreamerSettings
BIP_HttpServer_GetDefaultStartStreamerSettings
BIP_HttpServer_StopStreamer
**/
BIP_Status BIP_HttpServer_StartStreamer(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpRequestHandle hHttpRequest,     /*!< App using BIP_HttpServer interface must provide Request handle for this Streamer. */
    BIP_HttpServerStartStreamerSettings *pSettings
    );

/**
Summary:
Stop a BIP_HttpStreamer

Description:
Stops streaming activity on an HttpStreamer

Note:
After stopping a BIP_HttpStreamer session, app can re-use it and Start a new Streamer session on it for the next BIP_HttpRequest.

See Also:
BIP_HttpServer_StartStreamer
**/

BIP_Status BIP_HttpServer_StopStreamer(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpStreamerHandle hHttpStreamer
    );


/**
Summary:
Destroys a BIP_HttpStreamer

See Also:
BIP_HttpServer_CreateStreamer
**/
void BIP_HttpServer_DestroyStreamer(
    BIP_HttpServerHandle hHttpServer,
    BIP_HttpStreamerHandle hHttpStreamer
    );

/**
Summary:
Destroys a BIP_HttpServer

See Also:
BIP_HttpServer_Create
**/
void BIP_HttpServer_Destroy(
    BIP_HttpServerHandle hHttpServer
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_SERVER_H */
