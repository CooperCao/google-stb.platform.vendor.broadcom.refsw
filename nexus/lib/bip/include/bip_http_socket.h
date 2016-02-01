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

#ifndef BIP_HTTP_SOCKET_H
#define BIP_HTTP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"
typedef struct BIP_HttpSocket *BIP_HttpSocketHandle;

/**
 * Summary:
 * Http Sokcet APIs
 *
 * Description:
 * Abstraction of BIP_Socket APIs to allow Caller to send & receive HTTP Messages (Request/Response) and Payload bytes (AV Data).
 **/

/**
 * Summary:
 * Get Default HttpSocket settings
 **/
typedef struct BIP_HttpSocketCreateSettings
{
    BIP_SETTINGS(BIP_HttpSocketCreateSettings)   /* Internal use... for init verification. */

    BIP_HttpRequestHandle           hHttpRequest;                       /*!< Optional: caller can provide HttpRequest handle to use for incoming or outgoing Request */
    BIP_HttpResponseHandle          hHttpResponse;                      /*!< Optional: caller can provide HttpResponse handle to use for incoming or outgoing Response */
    unsigned                        persistentConnectionTimeoutInMs;    /*!< Optional: Duration in msec after which "idle" Http Sockets will be timed out (idle means HTTP Request hasn't arrived for this duration). Default it is set as 5000 msec. */
    unsigned                        networkReadBufferSize;              /*!< Optional: specifies the networkReadBuffer size which can be used to read raw network data. We use this to read Http Request and Response messages. Default it is set as 4096 bytes.*/
    unsigned                        networkWriteBufferSize;             /*!< Optional: specifies the networkWriteBuffer size which can be used to write raw network data. We use this to write Http request and Response messages. Default it is set as 2048 bytes.*/
    /* Note: BIP_HttpPlayer, BIP_HlsPlayer, BIP_MpegDashPlayer, etc. may introduce create time settings. */
} BIP_HttpSocketCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpSocketCreateSettings);

/**
Summary:
Get default settings for BIP_HttpSocket_CreateFromBipSocket().

See Also:
BIP_HttpSocket_CreateFromBipSocket
**/
#define BIP_HTTP_SOCKET_PERSISTENT_CONNECTION_TIMEOUT_IN_MSEC   5000
#define BIP_HTTP_SOCKET_NETWORK_READ_BUFFER_SIZE    4096
#define BIP_HTTP_SOCKET_NETWORK_WRITE_BUFFER_SIZE   2048

#define BIP_HttpSocket_GetDefaultCreateSettings(pSettings)                                                    \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpSocketCreateSettings)                               \
        /* Set non-zero defaults explicitly. */                                                               \
        (pSettings)->persistentConnectionTimeoutInMs = BIP_HTTP_SOCKET_PERSISTENT_CONNECTION_TIMEOUT_IN_MSEC; \
        (pSettings)->networkReadBufferSize = BIP_HTTP_SOCKET_NETWORK_READ_BUFFER_SIZE;                        \
        (pSettings)->networkWriteBufferSize = BIP_HTTP_SOCKET_NETWORK_WRITE_BUFFER_SIZE;                      \
        BIP_SETTINGS_GET_DEFAULT_END

/**
 * Summary:
 * API to create new HTTP Socket using an existing BIP_Socket.
 *
 * Description:
 *
 * This API allows to create HttpSocket object using an existing BIP_Socket. The API is designed for
 * a media server like usage where upper module (BIP_HttpServer has already accepted on a BIP_Socket)
 * and would then like to abstact it using BIP_HttpSocket.
 *
 **/
BIP_HttpSocketHandle BIP_HttpSocket_CreateFromBipSocket(
    BIP_SocketHandle                    hBipSocket,
    const BIP_HttpSocketCreateSettings  *pSettings
    );

/**
 * Summary:
 * API to create new HTTP Socket from a URL
 *
 * Description:
 *
 * This API allows to create HttpSocket object using an HTTP URL. The API is designed for
 * a media player (client) usage and used internally by BIP_Player interface.
 *
 **/
BIP_HttpSocketHandle BIP_HttpSocket_CreateFromUrl(
     const char                     *url,
     BIP_HttpSocketCreateSettings   *pSettings
     );

/**
 * Summary:
 * Customizable HttpSocket Settings
 **/
typedef struct BIP_HttpSocketSettings
{
    BIP_CallbackDesc requestReceivedCallback;           /* callback to indicate a HTTP Request message is available and can be receieved using BIP_HttpSocket_RecvRequest(). */
    BIP_CallbackDesc responseReceivedCallback;          /* callback to indicate a HTTP Response message is available and can be receieved using BIP_HttpSocket_RecvResponse(). */
    BIP_CallbackDesc payloadReceivedCallback;           /* callback to indicate payload bytes are available and can be received using BIP_HttpSocket_RecvPayload(). */
    BIP_CallbackDesc readyToSendPayloadCallback;        /* callback to indicate that additional payload bytes can be sent */
    BIP_CallbackDesc endOfStreamCallback;               /* callback to indicate when all Payload bytes have been sent or received (count matched with the Content Length in Response Header) */
    BIP_CallbackDesc errorCallback;                     /* callback to indicate error (peer closed or network error on socket) while receiving/sending message or payload bytes */
} BIP_HttpSocketSettings;

/**
 * Summary:
 * Get current HttpSocket Settings
 **/
void BIP_HttpSocket_GetSettings(
    BIP_HttpSocketHandle    hHttpSocket,
    BIP_HttpSocketSettings  *pSettings
    );

/**
 * Summary:
 * Update current HttpSocket Settings
 **/
BIP_Status BIP_HttpSocket_SetSettings(
    BIP_HttpSocketHandle    hHttpSocket,
    BIP_HttpSocketSettings  *pSettings
    );

/**
 * Summary:
 * API to Receive a HTTP Request Message Header.
 *
 * Description:
 * This API allows caller to receive the next incoming HTTP Request. API will fill-in the HttpRequest & HttpResponse
 * handles in the caller pointer pointer variables.
 *
 * Caller can optionally specify the blocking/non-blocking/async mode by filling-in the BIP_HttpSocketRecvRequestSettings.
 * if asyncCallback is provided, then it is an async API. Otherwise, it is a sync API w/ some timeout value.
 *
 * When timeoutInMs param is 0, API works in the non-blocking mode. It will return Request handle if available, NULL otherwise.
 *
 * For blocking modes, timeoutInMs can be -1 making it block until a Request arrives or to > 0 value indicating to wait for
 * that many milliseconds. API will return a Request handle if available within that timeout value or NULL otherwise.
 *
 * If timeout is specified for async mode, then async completion callback is issued either if request is available
 * in that duration or if timeout happens. pAsyncStatus indicates the async completion status.
 *
 **/
typedef struct BIP_HttpSocketRecvRequestSettings
{
    BIP_SETTINGS(BIP_HttpSocketRecvRequestSettings)   /* Internal use... for init verification. */

    int               timeoutInMs;     /* 0: non-blocking; -1: blocking, > 0: timeout in milliseconds */
    BIP_CallbackDesc  asyncCallback;   /* async completion callback: must be set if async usage of API is desired */
    BIP_Status       *pAsyncStatus;    /* status of async API.  */
                                       /* status = BIP_INF_IN_PROGRESS when API returns BIP_SUCCESS */
                                       /* status = BIP_ERR_ALREADY_IN_PROGRESS if API is retried before async completion callback */
} BIP_HttpSocketRecvRequestSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpSocketRecvRequestSettings);

#define BIP_HttpSocket_GetDefaultRecvRequestSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpSocketRecvRequestSettings) \
        /* Set non-zero defaults explicitly. */                                      \
        (pSettings)->timeoutInMs = 0; /* non-blocking mode. */                       \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_HttpSocket_RecvRequest(
    BIP_HttpSocketHandle                hHttpSocket,
    BIP_HttpRequestHandle            *phHttpRequest,    /* pointer to received request handle. Only set upon successful completion, unchanged if error */
    BIP_HttpResponseHandle           *phHttpResponse,   /* pointer to response handle that must be used to send out the response for this request */
    BIP_HttpSocketRecvRequestSettings   *pSettings
    );

/**
 * Summary:
 * API to Send a HTTP Response on HTTP Socket
 *
 * Description:
 * A server type caller (BIP_HttpServer on behalf of media server app) will invoke this API to send back the HTTP Response for a previously received HTTP Request.
 **/
typedef struct BIP_HttpSocketSendResponseSettings
{
    BIP_SETTINGS(BIP_HttpSocketSendResponseSettings) /* Internal use... for init verification. */
    bool                noPayload;      /* set if caller has no payload to send in the response, e.g. for HEAD or Error Response. */
    int                 timeoutInMs;    /* 0: non-blocking; -1: blocking, > 0: timeout in milliseconds */
    BIP_CallbackDesc    asyncCallback;  /* async completion callback: must be set if async usage of API is desired */
    BIP_Status          *pAsyncStatus;  /* status of async API.  */
                                        /* status = BIP_INF_IN_PROGRESS when API returns BIP_SUCCESS */
                                        /* status = BIP_ERR_ALREADY_IN_PROGRESS if API is retried before async completion callback */
} BIP_HttpSocketSendResponseSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpSocketSendResponseSettings);

#define BIP_HttpSocket_GetDefaultSendResponseSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpSocketSendResponseSettings) \
        /* Set non-zero defaults explicitly. */                                       \
        (pSettings)->timeoutInMs = 0; /* non-blocking mode. */                        \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_HttpSocket_SendResponse(
    BIP_HttpSocketHandle                hHttpSocket,
    BIP_HttpRequestHandle               hHttpRequestHandle,     /* received request handle. */
    BIP_HttpResponseHandle              hHttpResponseHandle,    /* response handle to be sent out. */
    int64_t                             messageLength,          /* message length of the Http Response Message body, 0 if not known (e.g. for Live Channels or Xcoded Stream). */
    BIP_HttpSocketSendResponseSettings  *pSettings
    );


/**
 * Summary:
 * API to Send Payload for a HTTP Socket.
 *
 * Description:
 * A Server type caller (BIP_HttpStreamer) will use this API to send the Payload after sending the HttpResponse.
 * A client type caller (BIP_HttpPlayer) may also use it to send any payload associated w/ Http Request Message (e.g. w/ HTTP POST method).
 **/
typedef struct BIP_HttpSocketSendPayloadSettings
{
    BIP_SETTINGS(BIP_HttpSocketSendPayloadSettings) /* Internal use... for init verification. */

    int               timeoutInMs;     /* 0: non-blocking; -1: blocking, > 0: timeout in milliseconds */
    BIP_CallbackDesc  asyncCallback;   /* async completion callback: must be set if async usage of API is desired */
    BIP_Status       *pAsyncStatus;    /* status of async API.  */
                                       /* status = BIP_INF_IN_PROGRESS when API returns BIP_SUCCESS */
                                       /* status = BIP_ERR_ALREADY_IN_PROGRESS if API is retried before async completion callback */
} BIP_HttpSocketSendPayloadSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpSocketSendPayloadSettings);

#define BIP_HttpSocket_GetDefaultSendPayloadSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpSocketSendPayloadSettings) \
        /* Set non-zero defaults explicitly. */                                      \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_HttpSocket_SendPayload(
    BIP_HttpSocketHandle                hHttpSocket,
    const uint8_t                       *pBuffer,
    size_t                              bytesToSend,
    BIP_HttpSocketSendPayloadSettings   *pSettings
    );

#if 0
/* NOTE: These APIs are not yet Supported. */

/**
 * Summary:
 * API to Receive a HTTP Response Message on HTTP Socket
 *
 * Description:
 * A client type caller (BIP_HttpPlayer) will use this API receive the HTTP Response from the server
 * either after getting messageReceivedCallback or when it directly calls this API.
 *
 **/
BIP_Status BIP_HttpSocket_RecvResponse( BIP_HttpSocketHandle hHttpSocket, BIP_HttpResponseHandle httpResponse, int timeout );

/**
 * Summary:
 * API to Send a HTTP Request Message on HTTP Socket
 *
 * Description:
 * A client type caller (e.g. BIP_HttpPlayer) will use this API to send the HTTP Request to a server.
 **/
BIP_Status BIP_HttpSocket_SendRequest( BIP_HttpSocketHandle hHttpSocket, BIP_HttpRequestHandle httpRequest, int timeout );

/**
 * Summary:
 * API to Receive Payload for a HTTP Message
 *
 * Description:
 * A client type caller (BIP_HttpPlayer) will use it to receive any payload associated w/ Http Response Message.
 * A Server type caller (BIP_HttpStreamer) will use this API to receive the Payload for a HTTP Message (like POST).
 *
 **/
BIP_Status BIP_HttpSocket_RecvPayload( BIP_HttpSocketHandle hHttpSocket, uint8_t *pBuffer, size_t bytesToRead, int timeout, size_t *pBytesRead );
#endif

/**
 * Summary:
 * Destroy socket
 *
 * Description:
 **/
void BIP_HttpSocket_Destroy(
    BIP_HttpSocketHandle hHttpSocket
    );

/**
 * Summary:
 * API to return the Http Socket status
 *
 * Description:
 **/
typedef struct BIP_HttpSocketStatus
{
    int socketFd;
    const char *pLocalIpAddress;        /* IP Address string for the local IP address being used for this socketFd. */
    const char *pRemoteIpAddress;       /* IP Address string for the remote IP address being used for this socketFd. */
} BIP_HttpSocketStatus;
BIP_Status BIP_HttpSocket_GetStatus(
    BIP_HttpSocketHandle hHttpSocket,
    BIP_HttpSocketStatus *pStatus
    );

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_SOCKET_H */
