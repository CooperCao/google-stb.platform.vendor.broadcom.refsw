/******************************************************************************
(c) 2007-2015 Broadcom Corporation

This program is the proprietary software of Broadcom Corporation and/or its
licensors, and may only be used, duplicated, modified or distributed pursuant
to the terms and conditions of a separate, written license agreement executed
between you and Broadcom (an "Authorized License").  Except as set forth in
an Authorized License, Broadcom grants no license (express or implied), right
to use, or waiver of any kind with respect to the Software, and Broadcom
expressly reserves all rights in and to the Software and all intellectual
property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

Except as expressly set forth in the Authorized License,

1. This program, including its structure, sequence and organization,
   constitutes the valuable trade secrets of Broadcom, and you shall use all
   reasonable efforts to protect the confidentiality thereof, and to use
   this information only in connection with your use of Broadcom integrated
   circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
   AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
   WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
   TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
   WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
   PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
   LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
   OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
   YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
   OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
   IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
   ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.

****************************************************************************/

#ifndef BIP_HTTP_STREAMER_H
#define BIP_HTTP_STREAMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"

typedef struct BIP_HttpStreamer *BIP_HttpStreamerHandle;

/**

Summary:
HTTP Streamer Interface for streaming AV Media from an input such as File, Tuner, etc.

Description:
HTTP Streamer Class is responsible for streaming out a media from a file, live tuner or buffered input.
Before BIP_HttpStreamer_Start() can be called, caller must provide detailed information about the input source &
how to output this media.

Streamer Input Settings are set via via BIP_HttpStreamer_Set[File|Tuner]InputSettings(). Caller must specifiy
Tracks to stream out using the BIP_HttpStreamer_AddTrack().

Streamer Output Settings (e.g. Streaming Protocol, Encryption Settings, etc.) are set via the BIP_HttpStreamer_SetOutputSettings().

Only after input & output settings are set, BIP_HttpStreamer_Start() can be called.

After that, caller must call BIP_HttpStreamer_ProcessRequest() and provide HttpRequest for which HttpResponse (optional)
may be sent and then streaming should be started.

Caller can either call BIP_HttpStreamer_Stop() when it gets the endOfStreamingCallback or if the server is being Stopped.


NOTE:
A Media Server application using BIP_HttpServer Class should not directly call the BIP_HttpStreamer APIs
EXCEPT the APIs to set Input & Out Settings.
Instead, it must use the Streamer interface exposed by the BIP_HttpServer class to Create, Start, Stop, & Destroy the HttpStreamer object.
This allows BIP_HttpServer to route the Requests for subsequent segments/chunks to the same Http Streamer object.

NOTE2:
In case App decides to use its own Socket & HTTP Implementation for initial HTTP Request & Response handling,
BIP_HttpStreamer class could still be used for just streaming out purposes.
App doesn't use BIP_HttpServer interface for Creating, Starting, Stopping, Destroying the Streamer.
Instead, it directly uses the BIP_HttpStreamer Interface for these APIs.
App will then create a BIP_Socket & BIP_HttpSocket objects using the socketFd on which streaming should be done.
And then App will pass that BIP_HttpSocket object to BIP_HttpStreamer_ProcessRequest() to start streaming on.

*/

/**
Summary:
API to Create a HttpStreamer context.
*/
typedef struct BIP_HttpStreamerCreateSettings
{
    BIP_SETTINGS(BIP_HttpStreamerCreateSettings)    /* Internal use... for init verification. */

    /* NOTE: it's ok to define these callbacks in the createSettings as these callbacks are not invoked until Streamer is started! */
    BIP_CallbackDesc    endOfStreamingCallback;     /* Callback to indicate that streaming has finished for this context, */
                                                    /* can be issued when streaming is completed or streaming fails due to client channel change, */
                                                    /* or any network errors during streaming. */
    BIP_CallbackDesc    softErrorCallback;          /* Callback to let caller know about soft errors *not* due to streaming, */
                                                    /* e.g. no data to stream from live source, failure to read from the file, etc. */
} BIP_HttpStreamerCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpStreamerCreateSettings);

#define BIP_HttpStreamer_GetDefaultCreateSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpStreamerCreateSettings) \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_HttpStreamerHandle BIP_HttpStreamer_Create(
    const BIP_HttpStreamerCreateSettings *pSettings
    );

/**
Summary:
API to Get/Set Streamer runtime settings
*/
typedef struct BIP_HttpStreamerSettings
{
    BIP_StreamerSettings    streamerSettings;
    bool                    enableDtcpIp;           /* Set this to modify the runtime DTCP/IP Settings. */
    BIP_DtcpIpSettings      dtcpIp;                 /* DTCP/IP Runtime settings: please see bip_dtcp_ip.h for details. */
} BIP_HttpStreamerSettings;

void BIP_HttpStreamer_GetSettings(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpStreamerSettings *pSettings
    );

BIP_Status BIP_HttpStreamer_SetSettings(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpStreamerSettings *pSettings
    );

/**
Summary:
API to Set File Input specific settings to the Streamer.

Description:

See Also:
BIP_MediaInfoStream in bip_media_info.h
BIP_StreamerFileInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultFileInputSettings
*/
BIP_Status BIP_HttpStreamer_SetFileInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    const char                      *pMediaFileAbsolutePathName,        /* Media input source: Name of media file. */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,               /* Stream related information of this file. */
    BIP_StreamerFileInputSettings   *pFileInputSettings
    );

/**
Summary:
API to Set Tuner Input specific settings to the Streamer.

Description:

See Also:
BIP_MediaInfoStream in bip_media_info.h
BIP_StreamerTunerInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultTunerInputSettings
*/
BIP_Status BIP_HttpStreamer_SetTunerInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    NEXUS_ParserBand                hParserBand,                /* ParserBand being used for the live channel */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerTunerInputSettings  *pTunerInputSettings
    );

/**
Summary:
API to Set Ip Input specific settings to the Streamer.

Description:

See Also:
BIP_MediaInfoStream in bip_media_info.h
BIP_StreamerIpInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultIpInputSettings
*/
BIP_Status BIP_HttpStreamer_SetIpInputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    BIP_PlayerHandle                hPlayer,                    /* BIP Player instance being used for the live ip channel */
    BIP_StreamerStreamInfo          *pStreamerStreamInfo,
    BIP_StreamerIpInputSettings     *pIpInputSettings
    );

/**
Summary:
API to Set Recpump Input specific settings to the Streamer.

Description:
BIP Streamer will use the Recpump buffer as media input (instead of File or Tuner input) and stream it out.

Usage:
Apps can use this input type to setup usages that are not handled via currently supported
BIP Streamer inputs. E.g. If app wants to have a custom way to setup the AV pipe
(say for special transcode usage), then BIP will then simply stream out the the data
available in the Recpump fifo.

See Also:
BIP_StreamerRecpumpInputSettings in bip_streamer.h
BIP_Streamer_GetDefaultRecpumpInputSettings
*/
BIP_Status BIP_HttpStreamer_SetRecpumpInputSettings(
    BIP_HttpStreamerHandle              hHttpStreamer,
    NEXUS_RecpumpHandle                 hRecpump,
    BIP_StreamerRecpumpInputSettings    *pRecpumpInputSettings
    );

/**
Summary:
API to Add one Track to the Streamer.
Description:
BIP_HttpStreamer_AddTrack API is used to add a track number that needs to be streamed out.
Caller should invoke it multiple times once for each track.

It is useful to either specify individual tracks (within a program) of a MPTS fle stream
or in the Tuner case to choose specific set of tracks (within a program) among the multiple programs.
In addition, in Single Program case, it allows BIP Streamer to include track related info
(such as PIDS, Codecs, etc.) in the HTTP Response itself.

See Also:
BIP_MediaInfoTrack in bip_media_info.h
BIP_StreamerTrackSettings in bip_streamer.h
BIP_Streamer_GetDefaultTrackSettings
*/
BIP_Status BIP_HttpStreamer_AddTrack(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_StreamerTrackInfo       *pStreamerTrackInfo,
    BIP_StreamerTrackSettings   *pTrackSettings
    );

/**
Summary:
API to Set Streamer Output Settings.

See Also:
BIP_HttpStreamer_GetDefaultOutputSettings
**/
typedef enum BIP_HttpStreamerProtocol
{
    BIP_HttpStreamerProtocol_eDirect,               /* Stream media directly over HTTP w/o using any additional adaptive streaming protocols on top of HTTP */
    BIP_HttpStreamerProtocol_eHls,                  /* Stream media using Apple's Http Live Streaming (HLS) Protocol */
    BIP_HttpStreamerProtocol_eMpegDash,             /* Stream media using MPEG DASH ISO Standard Protocol */
    BIP_HttpStreamerProtocol_eMax
} BIP_HttpStreamerProtocol;

/**
Summary:
App specific private Header that is inserted *onetime* before media is streamed out.

Description:
This Header allows Media Server App to specify any private data that BIP Streamer should insert before starting to stream the media content.
**/
typedef struct BIP_AppInitialPayload
{
    bool        valid;                                  /* set to true to indicate the validity of this structure. */
    unsigned    length;                                 /* length of app private payload bytes: should be mod16 aligned if it needs to be encrypted */
    uint8_t     *pPayload;                              /* pointer to the private payload */
} BIP_AppInitialPayload;

typedef struct BIP_HttpStreamerOutputSettings
{
    BIP_SETTINGS(BIP_HttpStreamerOutputSettings) /* Internal use... for init verification. */

    struct BIP_StreamerOutputSettings   streamerSettings;

    bool                                enableHttpChunkXferEncoding;    /* if true, HTTP Chunk Tranfer Encoding headers would be inserted in the output streams. Useful when streaming live channels as content length is not available for live channels. */
                                                                        /* if input stream has transportTimestampEnabled and output settings doesn't set enableTransportTimestamp, then 4 byte timestamp is removed from the output */
    BIP_AppInitialPayload               appInitialPayload;              /* Optional: app specific private payload */
    NEXUS_HeapHandle                    heapHandle;                     /* Optional: heap for fifo allocation */
    bool                                enableHwOffload;                /* Optional: enables offload to h/w like ASP if available on a platform & doable for a particular mediaInput stream format */
    bool                                disableAvHeadersInsertion;      /* Optional: dont insert AV Headers (TrackInfo) in outputgoing HTTP Response. */
    bool                                disableDlnaContentFeatureHeaderInsertion; /* Optional: dont insert ContentFeatures.DLNA.ORG header into the outgoing response. */
    bool                                enableDtcpIp;                   /* Optional: To stream media using DTCP/IP, this should be set to true. */
                                                                        /* Defaults to false => DTCP/IP is not enabled for this streamer. */
    BIP_DtcpIpOutputSettings            dtcpIpOutput;                   /* if enableDtcpIp is true, this structure should be filled-in with DTCP/IP output settings. */
} BIP_HttpStreamerOutputSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpStreamerOutputSettings);

#define BIP_HttpStreamer_GetDefaultOutputSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpStreamerOutputSettings) \
        BIP_Streamer_GetDefaultOutputSettings(&(pSettings)->streamerSettings)     \
        /* Set non-zero defaults explicitly. */                                   \
        BIP_SETTINGS_GET_DEFAULT_END


BIP_Status BIP_HttpStreamer_SetOutputSettings(
    BIP_HttpStreamerHandle          hHttpStreamer,
    BIP_HttpStreamerProtocol        streamerProtocol,
    BIP_HttpStreamerOutputSettings  *pOutputSettings
    );

/**
Summary:
API to optionally transcode the streamer output.

Description:
This API allows caller to specify a paritcular transcode profile that should
be applied to the input. All AV encoding parameters can be specified using this API.

In addition, app can call this API to multiple times to specify differnt transcode profiles
associated with the input. This is useful for the Streamer protocols such as HLS & MPEG DASH
which use these profiles for adaptive streaming.

Note:
Caller must call BIP_HttpStreamer_Set[File|Tuner]Input() & BIP_HttpStreamer_SetOutput() before
calling this API.

See Also:
BIP_Transcode_GetDefaultProfile(profile)
BIP_Transcode_GetDefaultProfileFor_720p30_AVC_AAC_MPEG2_TS(profile)
**/
BIP_Status BIP_HttpStreamer_AddTranscodeProfile(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_TranscodeProfile        *pTranscodeProfile
    );

/**
Summary:
API to optionally set the Nexus Handles required for Transcode operation.

Description:
This is an optional API. If app doesn't call it, then BIP will internally open/acquire Nexus
Handles needed for the Transcode.

See Also:
BIP_Transcode_GetDefaultNexusHandles
**/
BIP_Status BIP_HttpStreamer_SetTranscodeHandles(
    BIP_HttpStreamerHandle      hHttpStreamer,
    BIP_TranscodeNexusHandles   *pTranscodeNexusHandles
    );

/**
Summary:
API to Get a Streamer Response Header from the BIP prepared HTTP Response Message.

Description:
This API allows App to get value of a given HTTP Header.
Since BIP internally prepares the HTTP Response and thus
adds the required and necessary HTTP headers, Apps can use
this API to check/validate most of the BIP generated
HTTP headers.

Input:  hHttpStreamer       HttpStreamer handle
Input:  pHeaderName         pointer to the string containg the header name
Input:  pHeaderValue        address of the pointer that will hold the header value

Output: pHeaderValue        points to a BIP owned string containing the header value

Return:
    BIP_SUCCESS:                                pHeaderValue contains the header value.
    BIP_INF_HTTP_MESSAGE_HEADER_NOT_PRESENT:    requested header is not present.
    Other BIP Status codes:                     other BIP error.
**/
BIP_Status BIP_HttpStreamer_GetResponseHeader(
    BIP_HttpStreamerHandle  hHttpStreamer,          /*!< in: Streamer handle which will prepare & send the HTTP Response. */
    const char              *pHeaderName,           /*!< in: header name whose value needs to be returned. */
    const char              **pHeaderValue          /*!< in: address of a char pointer. */
                                                    /*!< out: points to a BIP owned string containing value of the header. */
    );

/**
Summary:
API to Set a Streamer Response Header to the BIP prepared HTTP Response Message.

Description:
This API allows App to set a HTTP Header & its value.
Since BIP internally prepares the HTTP Response and thus
adds the required and necessary HTTP headers, Apps can use
this API to set or override most of the BIP generated
HTTP headers.

In addition, this API allows apps to set any app specific
custom/private HTTP headers.

Note: BIP will not allow apps to override certain HTTP headers
that it has the correct values. A good example is HTTP Content-Length
header as its value can change depending upon encryption settings,
HTTP Chunk Xfer Encoding, etc.

Note2: BIP will internally send out the HTTP Response
when app calls BIP_HttpServer_StartStreamer().

Input:  hHttpStreamer       HttpStreamer handle
Input:  pHeaderName         pointer to the string containg the header name
Input:  pHeaderValue        pointer to the string contains the header value

Output: None

Return:
    BIP_SUCCESS:                                header was successfully set in the HTTP Response message
    BIP_ERR_HTTP_MESSAGE_HEADER_NOT_SET         app can't set or modify this particular HTTP Header
    Other BIP Status codes:                     other BIP error
**/
BIP_Status BIP_HttpStreamer_SetResponseHeader(
    BIP_HttpStreamerHandle  hHttpStreamer,          /*!< in: Streamer handle which will prepare & send the HTTP Response. */
    const char              *pHeaderName,           /*!< in: header name whose value needs to be returned. */
    const char              *pHeaderValue           /*!< in: header value to set. */
    );

/**
Summary:
API to start the streamer.

Description:
This API is used to start a streaming session.

Note: App must directly call the BIP_HttpStreamer_Set[File|Tuner|Ip|Bfile]InputSettings()
and BIP_HttpStreamer_SetOutputSettings() APIs before calling this BIP_HttpStreamer_Start API.

Note2: Caller is also required to call BIP_HttpStreamer_ProcessRequest() API below to start
the actual streaming of media.  This is done to faciliate handling of Adaptive Streaming protocols,
such as HLS & MPEG DASH> In both cases, same streamer context will be used for handling
multiple Http Requests (for different segments) one after another. This way App only need to handle
the very 1st Adaptive Streaming Request for the Master Playlist/MPD and subsequent Requests for child Playlists
or individual Media Segments are handled internally in the BIP.

**/
typedef struct BIP_HttpStreamerStartSettings
{
    BIP_SETTINGS(BIP_HttpStreamerStartSettings)         /* Internal use... for init verification. */

    BIP_DtcpIpServerHandle  hInitDtcpIp;            /*!< in: optional DTCP/IP init handle returned by the DtcpAppLib_Startup(). */
    int                     timeoutInMs;            /*!< 0: non-blocking; -1: blocking, > 0: timeout in milliseconds. */
    BIP_CallbackDesc        asyncCallback;          /*!< async completion callback: must be set if async usage of API is desired. */
    BIP_Status              *pAsyncStatus;          /*!< status of async API.  */
    int                     inactivityTimeoutInMs;  /*!< <=0: no timeout, > 0: inactivity timeout duration in milliseconds. */
                                                    /*!< Examples of inactivity: direct streamer has not streamed any data over this interval (client paused). */
                                                    /*!< Examples of inactivity: HLS streamer is not receiving any next segment request over this interval (client paused or crashed). */
                                                    /*!< Note: when inactivityTimeoutInMs expires, BIP_HttpStreamer will either: */
                                                    /*!<    1. Call inactivityTimeoutCallback (if set), or */
                                                    /*!<    1. issue endOfStreamingCallback (if set). */
    BIP_CallbackDesc        inactivityTimeoutCallback;/*!< Callback to let app know about inactive streamer. */
                                                    /*!< Invoked after streamer has been inactive for inactivityTimeoutInMs duration. */
} BIP_HttpStreamerStartSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpStreamerStartSettings);

#define BIP_HttpStreamer_GetDefaultStartSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpStreamerStartSettings) \
        /* Set non-zero defaults explicitly. */                                  \
        (pSettings)->timeoutInMs = -1; /* blocking mode is set by default. */    \
        (pSettings)->inactivityTimeoutInMs = -1; /* no inactivity timeout enabled by default. */ \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_HttpStreamer_Start(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpStreamerStartSettings *pSettings
    );

/**
Summary:
Stop HttpStreamer.

Description:
API to stop a streaming session.

Note: After stopping a BIP_HttpStreamer session, caller can re-use it and
Start a new Streamer session on it for the next BIP_HttpRequest.

**/
BIP_Status BIP_HttpStreamer_Stop(
    BIP_HttpStreamerHandle hHttpStreamer
    );

/**
Summary:
Ask HTTP Streamer to process a give HTTP Request

Description:
This api will process a given HTTP Request as the BIP_HttpStreamerStartSettings. Streaming will commence after this call.

**/
typedef struct BIP_HttpStreamerProcessRequestSettings
{
    BIP_SETTINGS(BIP_HttpStreamerProcessRequestSettings)   /* Internal use... for init verification. */

    BIP_HttpRequestHandle   hHttpRequest;                   /* Optional: If set, Streamer will prepare & send a response for this Request before streaming. */
} BIP_HttpStreamerProcessRequestSettings;
BIP_SETTINGS_ID_DECLARE(BIP_HttpStreamerProcessRequestSettings);

#define BIP_HttpStreamer_GetDefaultProcessRequestSettings(pSettings)                      \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_HttpStreamerProcessRequestSettings) \
        /* Set non-zero defaults explicitly. */                                           \
        BIP_SETTINGS_GET_DEFAULT_END

BIP_Status BIP_HttpStreamer_ProcessRequest(
    BIP_HttpStreamerHandle                  hStreamer,
    BIP_HttpSocketHandle                    hHttpSocket,
    BIP_CallbackDesc                        *pRequestProcessedCallback,   /* Required: Callback to indicate that streaming has finished streaming for a request. */
                                                                        /* Caller can then resume the ownership of hHttpSocket, hHttpRequest, hHttpResponse objects. */
    BIP_HttpStreamerProcessRequestSettings  *pSettings
    );

/**
Summary:
API to return the Http Streamer status

Description:
**/
typedef struct BIP_HttpStreamerStats
{
    uint64_t    numBytesStreamed;           /* total bytesStreamed. */
} BIP_HttpStreamerStats;

typedef struct BIP_HttpStreamerStatus
{
    unsigned                inactivityTimeInMs;     /* Duration in which HttpStreamer has not streamed out any payload bytes. */
    BIP_HttpStreamerStats   stats;
    BIP_StreamerStatus      streamerStatus;
} BIP_HttpStreamerStatus;

BIP_Status BIP_HttpStreamer_GetStatus(
    BIP_HttpStreamerHandle hHttpStreamer,
    BIP_HttpStreamerStatus *pStatus
    );

/**
Summary:
API to Destroy a HttpStreamer context
**/
void BIP_HttpStreamer_Destroy(
    BIP_HttpStreamerHandle streamer
    );

/**
Summary:
API to Print Steamer Status
*/
void BIP_HttpStreamer_PrintStatus(
    BIP_HttpStreamerHandle hHttpStreamer
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_STREAMER_H */
