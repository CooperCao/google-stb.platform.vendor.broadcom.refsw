/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef BIP_PLAYER_H
#define BIP_PLAYER_H

#include "bmedia_probe.h"
#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_player

BIP_Player Interface Definition.

This class is responsible playing IP media specified by a URL.  Playing of the
media is managed by a state machine. Below is a diagram of states and the transitions
resulting from BIP calls and other events.

\code


                                BIP_Player_Create() .--------------.
                               -------------------->|              |
                                                    |              |
                            BIP_Player_Disconnect() | Disconnected |
                            ----------------------->|              |
                                                    |              |
                                             .----->|              |
                                             |      '--------------'
                                             |              |
                  BIP_Player_Connect() fails |              |BIP_Player_Connect() begins
                                             |              v
                                             |       .------------.
                                             '-------| Connecting |
                                                     '------------'
                                                            |
                                                            |BIP_Player_Connect() succeeds
                                                            v
                                                     .------------.                                       .---------.
                                                     |            | BIP_Player_ProbeMediaInfo() begins    |         |
                                        .----------->|            |-------------------------------------->|         |
                                        |            | Connected  |                                       | Probing |
                                        |            |            | BIP_Player_ProbeMediaInfo() completes |         |
                                        |            |            |<--------------------------------------|         |
              BIP_Player_Prepare() fails|            |            |                                       '---------'
                                        |            '------------'
                                        |                  |
                                        |                  |BIP_Player_Prepare() begins
                                        |                  v
                                        |            .-----------.
                                        '------------| Preparing |
                                                     '-----------'
                                                           |
                                                           |BIP_Player_Prepare() succeeds
                                                           v
                                                     .-----------.
          .----------------------------------------->|           |
          |                                          |           |
          |   .------------------------------------->|           |
          |   |                                      | Prepared  |
          |   |   .--------------------------------->|           |
          |   |   |                                  |           |
          |   |   |                          .------>|           |
          |   |   |                          |       '-----------'
          |   |   |                          |             |
          |   |   | BIP_Player_Start() fails |             | BIP_Player_Start() begins
          |   |   |                          |             v
          |   |   |                          |       .----------.
          |   |   |                          '-------|          | BIP_Player_Start(startPaused) succeeds
          |   |   |                BIP_Player_Stop() | Starting |--------------------------------------------------------.
          |   |   '----------------------------------|          |                                                        |
          |   |                                      '----------'                                                        |
          |   |                                            |                                                             |
          |   |                                            | BIP_Player_Start() succeeds                                 |
          |   |                                            v                                                             v
          |   |                                      .----------.                                                   .----------.
          |   |                                      |          | BIP_Player_Pause()                                |          |
          |   |                                      |          |-------------------------------------------------->|          |
          |   |                                      |          |                                                   |          |
          |   |                 BIP_Player_Stop() or |          |                                 BIP_Player_Play() |          | BIP_Player_Seek()
          |   |                 endOfStream or       | Playing  |<--------------------------------------------------| Paused   |-----------------.
          |   |                 networkError         |          |                                                   |          |                 |
          |   '--------------------------------------|          |                           BIP_Player_PlayAtRate() |          |<----------------'
          |                                          |          |<--------------------------------------------------|          |
          |                                          |          |                                                   |          |BIP_Player_PlayByFrame()
          |                                          |          |                                                   |          |------------------------.
          |                                          |          | BIP_Player_Seek()                                 |          |                        |
          |                                          |          |------------------.                                |          |<-----------------------'
          |                                          |          |                  |                                |          |
          |                                          |          |<-----------------'                                |          |
          |                                          |          |BIP_Player_PlayAtRate()                            |          |
          |                                          |          |<--------------------.                             |          |
          |                                          |          |                     |                             |          |
          |                                          |          |---------------------'                             |          |
          |                                          '----------'                                                   |          |
          |                                                                                       BIP_Player_Stop() |          |
          '---------------------------------------------------------------------------------------------------------|          |
                                                                                                                    '----------'



\endcode
*/

/**
Summary:
The BIP_PlayerHandle refers to a specific BIP_Player instance.  In general,
it is passed in the APIs to indicate which BIP_Player the API should act upon.
**/
typedef struct BIP_Player  *BIP_PlayerHandle;

/****************************************************************************
 *   BIP_Player_Create()
 ****************************************************************************/
/**
Summary:
Settings for BIP_Player_Create().

See Also:
BIP_Player_GetDefaultCreateSettings
BIP_Player_Create
**/
typedef struct BIP_PlayerCreateSettings
{
    BIP_SETTINGS(BIP_PlayerCreateSettings)      /*!< Internal use... for init verification. */
} BIP_PlayerCreateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerCreateSettings);

/**
Summary:
Get default settings for BIP_Player_Create().

See Also:
BIP_PlayerCreateSettings
BIP_Player_Create
**/
#define BIP_Player_GetDefaultCreateSettings(pSettings)                        \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerCreateSettings)   \
        /* Set non-zero defaults explicitly. */                               \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Create a Player instance to play the stream associated with a URL.

Description:
Allocates resources that are needed to run a BIP Player.
Should be called at the app initialization time if app wants to avoid memory
heap fragmentation.  A single BIP_Player instance can play multiple streams
serially (one after the other) without having to be Destroyed and re-Created.

See Also:
BIP_Player_GetDefaultCreateSettings
BIP_PlayerCreateSettings
**/

    BIP_PlayerHandle BIP_Player_Create(
        BIP_PlayerCreateSettings *pSettings     /*!< [in] Optional settings structure. Pass NULL to use default settings. */
        );

/****************************************************************************
 *    BIP_Player_Destroy()
 ****************************************************************************/
/**
Summary:
Destroy a BIP Player instance

See Also:
BIP_Player_GetDefaultCreateSettings
BIP_PlayerCreate
**/
    void BIP_Player_Destroy(
        BIP_PlayerHandle hPlayer                /*!< [in] Handle of BIP_Player to be destroyed. */
        );

/****************************************************************************
 *   BIP_Player_Connect()
 *   BIP_Player_ConnectAsync()
 ****************************************************************************/
/**
Summary:
Data Availbility Model for the AV Stream to be played w/ the URL.

Description:
This enum allows Apps to specify how a AV Stream data is accessible,
if App knows this accessibility model ahead of time.
Apps can choose to leave the model as _eAuto which allows Player to
select the model based on the Server's HTTP Response.

However, if app knows that a particular model applies to a URL to be played,
then it should specify that model.  This guides Player to obtain
additional information associated with a particular model.

E.g. for time-shifted content, app can set the model to _eLimitedRandomAccess.
Player will then include getAvailableSeekRange header in the HTTP Request and
Server will then provide the available seek range which will allow player to
determine the current time-shift buffer duration.

Likewise, app can set it to _eNoRandomAccess to pure live channels.

Player internally uses the data model to determine if trickmodes are allowed on a stream.

See Also:
BIP_PlayerConnectSettings
BIP_Player_GetDefaultConnectSettings
BIP_Player_Connect
BIP_Player_ConnectAsync
**/
typedef enum BIP_PlayerDataAvailabilityModel
{
    BIP_PlayerDataAvailabilityModel_eAuto,                          /*!< Default: Player will determine the data accessibility model based on the Server's HTTP Response. */
    BIP_PlayerDataAvailabilityModel_eNoRandomAccess,                /*!< No Random Access is possible for this Stream: should be set for Live Channels with no associated Time-shift buffer. */
    BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess,           /*!< Stream has Limited Random Access due to Time-shifted playback: should be set for Live Channels w/ associated Time-shift buffer. */
    BIP_PlayerDataAvailabilityModel_eFullRandomAccess,              /*!< Stream has Full Random Access: should be set for pre-recorded DVR type content. */
    BIP_PlayerDataAvailabilityModel_eMax
} BIP_PlayerDataAvailabilityModel;

/**
Summary:
Settings for BIP_Player_Connect().

See Also:
BIP_Player_GetDefaultConnectSettings
BIP_Player_Connect
BIP_Player_ConnectAsync
**/
typedef struct BIP_PlayerConnectSettings
{
    int                             timeoutInMs;              /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */

    NEXUS_DmaHandle                 hDma;                     /*!< If set, BIP will use it, otherwise, it will internally open it if needed. */
    NEXUS_HeapHandle                hHeap;                    /*!< If set, BIP will use it, otherwise, it will internally open it if needed. */

    unsigned                        maxIpNetworkJitterInMs;   /*!< Max network jitter, defaults to 300msec. */
    const char                      *pProxyServerAddress;     /*!< If set, points to the proxy server's ip address */
    const char                      *pProxyServerPort;        /*!< If set, points to the proxy server's port# */
    const char                      *pNetworkInterfaceName;   /*!< If set, interface name to send multicast joins requests on (for receiving live UDP/RTP sessions) */

    const char                      *pUserAgent;              /*!< App can override the default UserAgent string used in the outgoing HTTP Get Request: string only */
    const char                      *pAdditionalHeaders;      /*!< Additional HTTP headers that app wants to include in the outgoing Get Request. Terminate each header with "\0xd\0xa". */

    BIP_HttpResponseHandle          *phHttpResponse;          /*!< If set, BIP_HttpResnponse handle is returned via this pointer */

    bool                            enableDtcpIp;             /*!< If this URL is proteced using DTCP/IP, this flag must be set to true. Defaults is false */
    BIP_DtcpIpServerStartSettings   dtcpIpServer;             /*!< If enableDtcpIp is true, this structure should be filled-in with DTCP/IP Server related Settings */

    void                            *pSslInitCtx;             /*!< If URL's scheme is HTTPs. BIP will use this SSL_CTX pointer if it is set, otherwise, BIP will internally allocate it. */

    BIP_PlayerDataAvailabilityModel dataAvailabilityModel;    /*!< Specifies how the Stream's AV data is accessible: default is eAuto where Player determines it based on the HTTP Response. */

    bool                            deferServerConnection;    /*!< Actual connection to the Server is deferred till the BIP_Player_Start state. */
                                                              /*!< Useful when app knows the media info or doesn't need to know it in recording type usage. */
                                                              /*!< Also, this option is currently only supported when BIP_PlayerPrepareSettings.enableHwOffload is set. */
                                                              /*!< And is mainly there for testing purposes to blindly record the incoming IP stream. */

    BIP_SETTINGS(BIP_PlayerConnectSettings)                   /*!< Internal use... for init verification. */
} BIP_PlayerConnectSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerConnectSettings);

/**
Summary:
Get default settings for BIP_Player_Connect() and BIP_Player_ConnectAsync().

See Also:
BIP_PlayerConnectSettings
BIP_Player_Connect
BIP_Player_ConnectAsync
**/
#define BIP_Player_GetDefaultConnectSettings(pSettings)     \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerConnectSettings)    \
        /* Set non-zero defaults explicitly. */                                 \
        (pSettings)->maxIpNetworkJitterInMs = 300;                              \
        (pSettings)->timeoutInMs = -1; /* blocking */                           \
        (pSettings)->pUserAgent = "BIP Player 0.5";                             \
        (pSettings)->dataAvailabilityModel = BIP_PlayerDataAvailabilityModel_eAuto; \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Establish initial handshake with Server and optionally provide Server Response when applicable.

See Also:
BIP_Player_ConnectAsync
BIP_Player_GetDefaultCreateSettings
BIP_PlayerCreateSettings
BIP_Player_Disconnect
**/
BIP_Status  BIP_Player_Connect(
    BIP_PlayerHandle            hPlayer,        /*!< [in] Handle of BIP_Player. */
    const char                  *pUrl,          /*!< [in] URL of media to be played */
    BIP_PlayerConnectSettings   *pSettings      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Async version of BIP_Player_Connect().
Establish initial handshake with Server and optionally provide Server Response when applicable.

See Also:
BIP_Player_Connect
BIP_Player_GetDefaultCreateSettings
BIP_PlayerCreateSettings
BIP_Player_Disconnect
**/
BIP_Status  BIP_Player_ConnectAsync(
    BIP_PlayerHandle            hPlayer,          /*!< [in] Handle of BIP_Player. */
    const char                  *pUrl,            /*!< [in] URL of media to be played */
    BIP_PlayerConnectSettings   *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc            *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                  *pAsyncStatus     /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_ProbeMediaInfo()
 *  BIP_Player_ProbeMediaInfoAsync()
 ****************************************************************************/
/**
Summary:
Enum used to specify the media container type.  Includes all values from NEXUS_TransportType in
addition to the BIP_PlayerContainerType values listed.
**/
typedef enum BIP_PlayerContainerType
{
    BIP_PlayerContainerType_eNexusTransportType = NEXUS_TransportType_eMax, /*!< Based at the end of NEXUS_TransportType values. */
    BIP_PlayerContainerType_eHls,               /*!< Http Live Streaming. */
    BIP_PlayerContainerType_eMpegDash,          /*!< MPEG-DASH adaptive streaming. */
    BIP_PlayerContainerType_eMax
} BIP_PlayerContainerType;

/**
Summary:
Settings for BIP_Player_ProbeMediaInfo() and BIP_Player_ProbeMediaInfoAsync().

See Also:
BIP_Player_GetDefaultProbeMediaInfoSettings
BIP_Player_ProbeMediaInfo
BIP_Player_ProbeMediaInfoAsync
**/
typedef struct BIP_PlayerProbeMediaInfoSettings
{
    NEXUS_TransportType     transportType;              /*!< App can provide the stream transport type if it knows it apriori (to can speed up the media probe), default is NEXUS_TransportType_eUnknown */
    BIP_PlayerContainerType containerType;              /*!< App can provide the Player Container Type (HLS, MPEG DASH, etc.) if it knows it apriori. */
    int64_t                 contentLength;              /*!< App can indicate content length if it is known via some other means: it's useful for HTTP Chunk Xfer Encoding case where Server doesn't send ContentLength in HTTP Resp, default is 0 (not known).*/
                                                        /* TODO: Shall we default it to actual ContentLength that we read from the HTTP Response? */
    bmedia_probe_config     mediaProbeConfig;           /*!< App can override any mediaProbe configuration. */

    bool                    enableRunTimeBuffering;     /*!< If true, Player will determine detailed Stream info such as BitRate & duration so that it can buffer stream at start & runtime */
    bool                    dontUseIndex;               /*!< Dont use index for playback: false by default */
    bool                    enablePayloadScanning;      /*!< if true, set up probe engine to determine accurate stream duration using payload scanning & random access into stream. */
                                                        /*!< Note: setting this option will increase the media probe duration, but is needed to determine accurate info for perforing client side trick modes on TS & PES/VOB formats */

    int                     timeoutInMs;                /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */

    BIP_SETTINGS(BIP_PlayerProbeMediaInfoSettings)      /*!< Internal use... for init verification. */
} BIP_PlayerProbeMediaInfoSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerProbeMediaInfoSettings);

/**
Summary:
Get default settings for BIP_Player_ProbeMediaInfo() and BIP_Player_ProbeMediaInfoAsync().

See Also:
BIP_PlayerProbeMediaInfoSettings
BIP_Player_ProbeMediaInfo
BIP_Player_ProbeMediaInfoAsync
**/
#define BIP_Player_GetDefaultProbeMediaInfoSettings(pSettings)          \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerProbeMediaInfoSettings)    \
        /* Set non-zero defaults explicitly. */                         \
        bmedia_probe_default_cfg(&(pSettings)->mediaProbeConfig);       \
        (pSettings)->timeoutInMs = -1; /* blocking */                   \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Obtain MediaInfo associated w/ the Stream.

See Also:
BIP_Player_ProbeMediaInfoAsync
BIP_Player_GetDefaultProbeMediaInfoSettings
BIP_PlayerProbeMediaInfoSettings
**/
BIP_Status  BIP_Player_ProbeMediaInfo(
    BIP_PlayerHandle                        hPlayer,          /*!< [in] Handle of BIP_Player. */
    BIP_PlayerProbeMediaInfoSettings        *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_MediaInfoHandle                     *phMediaInfo      /*!< [out] Pointer to meta data resulting from the media probe. */
    );

/**
Summary:
Async version of BIP_Player_ProbeMediaInfo().
Obtain MediaInfo associated w/ the Stream.

See Also:
BIP_Player_ProbeMediaInfo
BIP_Player_GetDefaultProbeMediaInfoSettings
BIP_PlayerProbeMediaInfoSettings
**/
BIP_Status  BIP_Player_ProbeMediaInfoAsync(
    BIP_PlayerHandle                        hPlayer,          /*!< [in] Handle of BIP_Player. */
    BIP_PlayerProbeMediaInfoSettings        *pSettings,       /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_MediaInfoHandle                     *phMediaInfo,     /*!< [out] Pointer to meta data resulting from the media probe. */
    BIP_CallbackDesc                        *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                              *pAsyncStatus     /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_GetProbedStreamInfo()
 ****************************************************************************/
/**
Summary:
Media Stream Attributes associated with the URL.

Description:
These stream attributes enable player to determine various modes such as:
-Feed stream using Nexus Playpump vs Nexus Playback,
-Clock Recovery Mode (Live TTS Pacing, Live SyncSlip, Live PCR Pacing, vs Pull modes).

App should call BIP_Player_ProbeMediaInfo() first to determine the mediaInfo associated with this URL.
It can then directly fill-in the streamInfo from the mediaInfo result of that call. Or
it better yet, app can call BIP_Player_GetProbedStreamInfo() to automatically get the streamInfo filled-in from the previous probe results.

See Also:
BIP_Player_ProbeMediaInfo
BIP_Player_ProbeMediaInfoAsync
BIP_Player_GetProbedStreamInfo
**/
typedef struct BIP_PlayerStreamInfo
{
    /* Player Container is for HTTP based higher level protocols. */
    BIP_PlayerContainerType         containerType;
    NEXUS_TransportType             transportType;
    BIP_PlayerDataAvailabilityModel dataAvailabilityModel;          /*!< Specifies how the Stream's AV data is accessible. */

    bool                            serverSideTrickmodesSupported;  /*!< Server supports Server Side Trickmodes, possibly known via the DLNA.ORG_PS field in the contentFeatures.dlna.org header or other means (indicated in the URL). */
    int64_t                         contentLength;                  /*!< Content length of Stream, 0 if not known */
    unsigned                        durationInMs;                   /*!< Duration of Stream in milliseconds or 0 if unknown */
    bool                            liveChannel;                    /*!< If true, this Stream pertains to a Live Channel */
    unsigned                        avgBitRate;                     /*!< Average Stream bitrate in bits/second or 0 if unknown */
    unsigned                        maxBitRate;                     /*!< Maximum Stream bitrate in bits/second or UINT_MAX if unknown */
    bool                            rangeHeaderDisabled;            /*!< If true, Player wont use the Range header in the HTTP Get Requests */
                                                                    /*!< Some servers indicate it via via a flag in the contentFeatures.dlan.org HTTP Response Header. */
    bool                            timeSeekRangeHeaderEnabled;     /*!< If set, Player will use the TimeSeekRange header in the HTTP Get Requests for time based seek operations. */
                                                                    /*!< Some servers indicate it via via a flag in the contentFeatures.dlan.org HTTP Response Header. */
    bool                            usePlaypump;                    /*!< If set, BIP will use Nexus Playpump module for stream playback. By default, BIP will internally determine whether to use Playpump or Playback. */
    bool                            usePlayback;                    /*!< If set, BIP will use Nexus Playback module for stream playback. By default, BIP will internally determine whether to use Playpump or Playback. */

    struct  /** These fields only apply when containerType==NEXUS_TransportType_eTs */
    {
        unsigned                    pmtPid;                         /*!< If !=0, Player will monitor PMT changes and notify app (TODO: need to define this). */
        unsigned                    pcrPid;                         /*!< PCR PID must be provided for MPEG2 TS stream playback. */
        bool                        transportTimeStampEnabled;      /*!< Indicates if MPEG2 TS stream contains additional 4 byte timpstamp (192 byte Transport Packet) */
    } mpeg2Ts;

} BIP_PlayerStreamInfo;


/**
Summary:
Populates a BIP_PlayerStreamInfo from the results of previous call to
BIP_Player_ProbeMediaInfo() or BIP_Player_ProbeMediaInfoAsync().

Description:
The resulting BIP_PlayerStreamInfo struct can be modified as needed before
being passed to BIP_Player_Prepare().

See Also:
BIP_Player_ProbeMediaInfo
BIP_Player_ProbeMediaInfoAsync
BIP_PlayerStreamInfo
**/
BIP_Status BIP_Player_GetProbedStreamInfo(
    BIP_PlayerHandle        hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerStreamInfo    *pStreamInfo    /*!< [out] Pointer to the BIP_PlayerStreamInfo to be filled. */
);

/****************************************************************************
 *  BIP_Player_SetSettings()
 ****************************************************************************/
/**
Summary:
Player Settings that can be changed before or after starting the player.

Description:
Allows apps to set the required track configuration (AV Decoder Handle, AV Codecs, PidChannel Settings, etc.).

Also, allows apps to select the Audio & Video tracks based on various preferences:
    - Specific Audio or Video TrackIds or TrackGroupId. Or,
    - Audio Language, Video View Name, etc.

Note: Some of these settings can also be changed after BIP_Player_Start. These are:
    - perferredAudioLanguage to switch the audio track
    - perferredSubtitleLanguage to switch the subtitle track or enable it

See Also:
BIP_Player_GetDefaultSettings
BIP_Player_GetSettings
BIP_Player_SetSettings
**/

/**
Summary:
Clock Recovery Modes
**/
typedef enum BIP_PlayerClockRecoveryMode
{
    BIP_PlayerClockRecoveryMode_eInvalid,                   /*!< Invalid Mode. */
    BIP_PlayerClockRecoveryMode_ePull,                      /*!< Pull mode: file playback where client is pulling the content from server: clock is initialied to 1st PTS and then freely run */
    BIP_PlayerClockRecoveryMode_ePushWithPcrSyncSlip,       /*!< Content pushed by Server using PCR */
    BIP_PlayerClockRecoveryMode_ePushWithTtsNoSyncSlip,     /*!< SW Buffering & Content Paced by local Playback Hardware using TTS Timestamps */
    BIP_PlayerClockRecoveryMode_ePushWithPcrNoSyncSlip,     /*!< SW Buffering & Content Paced by local Playback Hardware using PCRs as Timestamps */
    BIP_PlayerClockRecoveryMode_eMax
} BIP_PlayerClockRecoveryMode;

/**
Summary:
TTS throttle parameters
**/
typedef struct BIP_PlayerTtsParams
{
    unsigned            initBufDepth;                   /*!< Sets the initial buffer depth (used as a target buffer depth by TTS pacing) */
    unsigned            minBufDepth;                    /*!< Minimum buffer depth before buffer depth violation callback is called */
    BIP_CallbackDesc    minBufDepthViolationCallback;   /*!< Callback to indicate minimum buffer depth violation. */
    unsigned            maxBufDepth;                    /*!< Minimum buffer depth before buffer depth violation callback is called */
    BIP_CallbackDesc    maxBufDepthViolationCallback;   /*!< Callback to indicate maximum buffer depth violation. */
    unsigned            pacingMaxError;                 /*!< Set the timestamp error bound, as used by the playback pacing logic */
    unsigned            maxClockMismatch;               /*!< Specify the maximum clock mismatch (in ppm) between server/encoder and STB */
    bool                bufDepthInMsec;                 /*!< If true, buffer depth is calculated in msec (instead of bytes) using the TTS(if present) or PCR. */
} BIP_PlayerTtsParams;

typedef struct BIP_PlayerSettings
{
    /** Required Track Configuration. */
    NEXUS_PlaybackPidChannelSettings    audioTrackSettings;             /*!< Specify these Audio related settings if Player will be decoding Audio. */
                                                                        /*!< Audio Decoder Handle: audioTrackSettings.pidTypeSettings.audio.[primary|simpleDecoder] */

    NEXUS_PlaybackPidChannelSettings    videoTrackSettings;             /*!< Specify these Video related settings if Player will be decoding Video. */
                                                                        /*!< Video decoder handle: videoTrackSettings.pidTypeSettings.video.[decoder|simpleDecoder] */

    NEXUS_PlaybackPidChannelSettings    extraVideoTrackSettings;        /*!< Specify extraVideoDecoder handle (simple/raw), any optional PidChannel Settings. */

    /** Track Selection Preferences. */
    unsigned                            audioTrackId;                   /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF). Defaults to UINT_MAX. */
                                                                        /*!< Audio Codec: audioTrackSettings.pidSettings.pidTypeSettings.audio.codec */

    unsigned                            audioConnectId;                 /*!< NxClient ConnectId associated with the Primary Simple Audio Decoder. Only applies in the NxClient Mode. */

    unsigned                            videoTrackId;                   /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF). Defaults to UINT_MAX. */
                                                                        /*!< Video Codec: videoTrackSettings.pidTypeSettings.video.codec */

    unsigned                            extraVideoTrackId;              /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF). Defaults to UINT_MAX. */

    unsigned                            trackGroupId;                   /*!< Select Tracks that belong the this TrackGroupId. Defaults to UINT_MAX. */
                                                                        /*!< For MPEG2-TS: program_number. */

    const char                          *pPreferredAudioLanguage;       /*!< Preferred language of audio track. If set & not found, then 1st available audio track is selected. */
                                                                        /*!< App should also set this preference (if available) if it wants to enableDynamicTrackSelection & doesn't set the trackSelectionCallback. */
                                                                        /*!< Player will then use this preference to auto-select an Audio track when there is a stream change. */

    const char                          *pPreferredVideoName;           /*!< Preferred name of video track (alternate video views.). If set & not found, then 1st available video track is selected. */
    const char                          *pPreferredSubtitleLanguage;    /*!< Preferred language of subtitle track. */

    NEXUS_PlaybackSettings              playbackSettings;               /*!< Allows app to set any specific playback Settings, BIP_PlayerGetDefaultConnectSettings provides the current defaults. */
                                                                        /*!< Required: playbackSettings.stcChannel in non-Simple Nexus mode.  */
                                                                        /*!< Required: playbackSettings.simpleStcChannel in the simple Nexus mode (NxClient or Multiprocess Nexus). */
                                                                        /*!< Optional: startPaused: set it to true if Player should be started in the Paused mode. App must call BIP_Player_Play() to start the playback. */
                                                                        /*!< Optional: define callbacks descriptors if app wants to get callbacks for various conditions such as end/beginning of streaming or error during playback. */
                                                                        /*!< Optional: define actions for various conditions such as end/beginning/error cases. Otherwise, current Nexus Playback Defaults are used. */
                                                                        /*!< Optional: define error handling modes for errors during play/seek/trick-mode operations. Otherwise, Nexus Playback defaults are used. */

    BIP_MediaInfoAudioAc3Descriptor     ac3Descriptor;                  /*!< Optional: Describes the audio service type. Eg: main, music and effects, visually impaired etc. */
                                                                        /*!< App should also set this preference (if available) if it wants to enableDynamicTrackSelection & doesn't set the trackSelectionCallback. */
                                                                        /*!< Player will then use this preference to auto-select an Audio track when there is a stream change. */
                                                                        /*!< Note: This preference can be set in addition to the pPreferredAudioLanguage above. */

    NEXUS_DisplayHandle                 hDisplay;                       /*!< Required if non-Simple Video decode is used else will be ignored. */
    NEXUS_VideoWindowHandle             hWindow;                        /*!< Required if non-Simple Video decode is used else will be ignored. */

    BIP_PlayerClockRecoveryMode         clockRecoveryMode;              /*!< Override the clock recovery mode that BIP would otherwise choose based on URL Scheme, Container type, Live Channel flag. Defaults to _eInvalid (BIP chooses internally). */
    BIP_PlayerTtsParams                 ttsParams;                      /*!< TTS related parameters */

    unsigned                            maxStreamBitRate;               /*!< Maximum bitrate of the AV Stream Player should select. Defaults to UINT_MAX */
    unsigned                            minStreamBitRate;               /*!< Minimum bitrate of the AV Stream Player should select. Defaults to 0. */

    bool                                enableDynamicTrackSelection;    /*!< If true, Player will detect if a track changes in the middle of the stream & re-select tracks based on the Track Preferences defined above unless */
                                                                        /*!< App has defined trackSelectionCallback. Then, Player will only invoke this callback & wait for App to re-select the tracks */
                                                                        /*!< (see comment for trackSelectionCallback below on how to do this). */

                                                                        /*!< NOTE: Player will ignore this flag if App selects initial tracks by providing explicit audio & video trackIds above and */
                                                                        /*!< DOES NOT set the trackSelectionCallback below. */

                                                                        /*!< NOTE: Player will only enable this logic if App DOES NOT select the initial tracks by providing explicit audio & video trackIds above and */
                                                                        /*!< instead either it omits trackIds or provides the above defined Track Selection Preferences. */

    BIP_CallbackDesc                    trackSelectionCallback;         /*!< If set, Player will NOT auto-select the Tracks when it detects Track changes mid-stream. Instead, it will invoke this callback and */
                                                                        /*!< expect App to re-select Tracks by first acquiring the new BIP_MediaInfo object via BIP_Player_GetStatus and */
                                                                        /*!< selecting them via the BIP_PlayerSettings Track Preferences in the BIP_Player_SetSettings(). */

    BIP_CallbackDesc                    newMediaInfoCallback;           /*!< Callback to indicate the new MediaInfo availablility due to track changes in the stream. */
                                                                        /*!< App can then get the new BIP_MediaInfo object via BIP_Player_GetStatus. */
                                                                        /*!< Note: App doesn't need to set both trackSelectionCallback & newMediaInfoCallback. */
                                                                        /*!< The purpose of this callback is to handle the case where App wants player to auto-select the tracks and */
                                                                        /*!< just wants to know when player selects the new track due to changes in the stream. */

    struct
    {
        bool                            convertLpcmToWave;              /*!< if Set, app is requesting to convert incoming LPCM files into Wave by inserting the Wave header before the data */
        unsigned                        bitsPerSample;
        unsigned                        sampleRate;
        unsigned                        numChannels;
    } lpcm;

    bool                                disableMediaPositionUsingWallClockTime; /*! If Set, disable media position using Wall Clock Method. Instead, use the PTS to determine the media position. */

    BIP_SETTINGS(BIP_PlayerSettings)      /*!< Internal use... for init verification. */
} BIP_PlayerSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerSettings);

/**
Summary:
Get default settings for BIP_Player_SetSettings().

See Also:
BIP_Player_GetSettings
BIP_Player_SetSettings
BIP_PlayerSettings
**/
#define BIP_Player_GetDefaultSettings(pSettings) \
        /*!< [out] Pointer to caller's struct to be filled with default BIP_PlayerSettings. */ \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerSettings)    \
        /* Set non-zero defaults explicitly. */                         \
        NEXUS_Playback_GetDefaultSettings(&(pSettings)->playbackSettings); \
        NEXUS_Playback_GetDefaultPidChannelSettings(&(pSettings)->audioTrackSettings);\
        (pSettings)->audioTrackSettings.pidSettings.pidType = NEXUS_PidType_eAudio;\
        NEXUS_Playback_GetDefaultPidChannelSettings(&(pSettings)->videoTrackSettings);\
        (pSettings)->videoTrackSettings.pidSettings.pidType = NEXUS_PidType_eVideo;\
        NEXUS_Playback_GetDefaultPidChannelSettings(&(pSettings)->extraVideoTrackSettings);\
        (pSettings)->extraVideoTrackSettings.pidSettings.pidType = NEXUS_PidType_eVideo;\
        (pSettings)->videoTrackId = UINT_MAX;\
        (pSettings)->extraVideoTrackId = UINT_MAX;\
        (pSettings)->audioTrackId = UINT_MAX;\
        (pSettings)->trackGroupId = UINT_MAX;\
        (pSettings)->maxStreamBitRate = UINT_MAX;\
        (pSettings)->minStreamBitRate = 0;\
        (pSettings)->ttsParams.bufDepthInMsec     = false;\
        (pSettings)->ttsParams.pacingMaxError     = 2636;\
        (pSettings)->ttsParams.initBufDepth       = 625000;\
        (pSettings)->ttsParams.minBufDepth        = 125000;\
        (pSettings)->ttsParams.maxBufDepth        = 2250000;\
        (pSettings)->ttsParams.maxClockMismatch   = 60;\
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Returns current settings of a BIP Player.  These settings can be modified as desired, then passed to BIP_Player_SetSettings()

See Also:
BIP_Player_GetDefaultSettings
BIP_Player_SetSettings
BIP_PlayerSettings
**/
void  BIP_Player_GetSettings(
    BIP_PlayerHandle    hPlayer,    /*!< [in] Handle of BIP_Player. */
    BIP_PlayerSettings  *pSettings  /*!< [out] Pointer to caller's struct to be filled with the current BIP_PlayerSettings. */
    );

/**
Summary:
Changes configuration of BIP Player.

Description:
Some configuration can not be changed unless BIP_Player_Stop is called.

\pre Settings must have been obtained by calling either BIP_Player_GetDefaultSettings() or BIP_Player_GetSettings().

See Also:
BIP_Player_GetDefaultSettings
BIP_Player_GetSettings
BIP_PlayerSettings
**/
BIP_Status  BIP_Player_SetSettings(
    BIP_PlayerHandle            hPlayer,     /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerSettings    *pSettings   /*!< [in] Pointer to BIP_PlayerSettings structure. */
    );

/****************************************************************************
 *  BIP_Player_Prepare()
 *  BIP_Player_PrepareAsync()
 ****************************************************************************/
/**
Summary:
Settings for BIP_Player_Prepare() and BIP_Player_PrepareAsync().

See Also:
BIP_PlayerPrepareStatus
BIP_Player_GetDefaultPrepareSettings
BIP_Player_Prepare
BIP_Player_PrepareAsync
**/
typedef struct BIP_PlayerPrepareSettings
{
    unsigned                        timeshiftBufferMaxDurationInMs; /*!< For _eLimitedRandomAction Data Accessiblity model, Apps will need to provide the time-shift buffer duration if pauseTimeoutAction is set to NEXUS_PlaybackLoopMode_ePlay. */
    NEXUS_PlaybackLoopMode          pauseTimeoutAction;     /*!< For _eLimitedRandomAction Data Accessiblity model, Apps can specify action to take upon pause timeout. */
                                                            /*!< Options are: NEXUS_PlaybackLoopMode_ePlay (to start playing), NEXUS_PlaybackLoopMode_ePause (to stay paused, default one!. */

    NEXUS_PlaypumpHandle            hPlaypump;              /*!< If set, BIP will use it, otherwise, it will internally open it if needed. */
    NEXUS_PlaypumpHandle            hPlaypump2;             /*!< If set, BIP will use it, otherwise, it will internally open it if needed. Used for Adaptive protocols where alternate audio may have same trackId as the one in main video. */

    int                             timeoutInMs;            /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */

    bool                            enableAudioPrimer;      /*! <if set, BIP Player will prime all audio tracks to enable fast switching among audio tracks (for SAP support). */
                                                            /*! BIP Player will internally allocate the resources (Audio Decoders, PidChannels, Playpumps, etc) needed for priming various audio tracks). */

    bool                            enableHwOffload;        /* Optional: enables offload to h/w like ASP if available on a platform & doable for a particular mediaInput stream format */
    BIP_SETTINGS(BIP_PlayerPrepareSettings)                 /*!< Internal use... for init verification. */
} BIP_PlayerPrepareSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerPrepareSettings);

/**
Summary:
Status returned from BIP_Player_Prepare() and BIP_Player_PrepareAsync().

Description:
The values returned in BIP_PlayerPrepareStatus are usually needed for subsequent non-BIP operations (starting audio/video decoders, etc.).

See Also:
BIP_PlayerPrepareSettings
BIP_Player_GetDefaultPrepareSettings
BIP_Player_Prepare
BIP_Player_PrepareAsync
**/
typedef struct BIP_PlayerPrepareStatus
{
    NEXUS_PidChannelHandle  hVideoPidChannel;
    NEXUS_VideoCodec        videoCodec;

    NEXUS_PidChannelHandle  hExtraVideoPidChannel;
    NEXUS_VideoCodec        extraVideoCodec;

    NEXUS_PidChannelHandle  hAudioPidChannel;
    NEXUS_AudioCodec        audioCodec;

    BIP_MediaInfoHandle     hMediaInfo;
} BIP_PlayerPrepareStatus;

/**
Summary:
Get default settings for BIP_Player_Prepare() and BIP_Player_PrepareAsync().

See Also:
BIP_PlayerPrepareSettings
BIP_PlayerPrepareStatus
BIP_Player_Prepare
BIP_Player_PrepareAsync
**/
#define BIP_Player_GetDefaultPrepareSettings(pPrepareSettings) \
        BIP_SETTINGS_GET_DEFAULT_BEGIN((pPrepareSettings), BIP_PlayerPrepareSettings)    \
        /* Set non-zero defaults explicitly. */                         \
        (pPrepareSettings)->pauseTimeoutAction           = NEXUS_PlaybackLoopMode_ePause; \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
API to Prepare Player for Playing.

See Also:
BIP_Player_PrepareAsync
BIP_Player_GetDefaultPrepareSettings
BIP_PlayerPrepareSettings
BIP_PlayerPrepareStatus
**/
BIP_Status BIP_Player_Prepare(
    BIP_PlayerHandle                         hPlayer,           /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerPrepareSettings          *pPrepareSettings, /*!< [in] Required. How to prepare() */
    const BIP_PlayerSettings                 *pPlayerSettings,  /*!< [in] Required. Current player settings to use */
    const BIP_PlayerProbeMediaInfoSettings   *pProbeSettings,   /*!< [in] Optional. How to probe */
    const BIP_PlayerStreamInfo               *pStreamInfo,      /*!< [in] Optional. Use this stream info. */
    BIP_PlayerPrepareStatus                  *pStatus           /*!< [out] Required. Important values returned by BIP_Player_Prepare() */
    );

/**
Summary:
Async version of BIP_Player_Prepare().
API to Prepare Player for Playing.

See Also:
BIP_Player_Prepare
BIP_Player_GetDefaultPrepareSettings
BIP_PlayerPrepareSettings
BIP_PlayerPrepareStatus
**/
BIP_Status BIP_Player_PrepareAsync(
    BIP_PlayerHandle                         hPlayer,           /*!< [in]  Handle of BIP_Player. */
    const BIP_PlayerPrepareSettings          *pPrepareSettings, /*!< [in]  Optional. Prepare related settings. */
    const BIP_PlayerSettings                 *pPlayerSettings,  /*!< [in]  Optional. Initial Player settings to use */
    const BIP_PlayerProbeMediaInfoSettings   *pProbeSettings,   /*!< [in]  Optional. How to probe */
    const BIP_PlayerStreamInfo               *pStreamInfo,      /*!< [in]  Optional. Use this Stream info. */
    BIP_PlayerPrepareStatus                  *pPrepareStatus,   /*!< [out] Optional. Various status returned by BIP_Player_Prepare(). */
    BIP_CallbackDesc                         *pAsyncCallback,   /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus      /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_OpenPidChannel()
*****************************************************************************/
/**
Summary:
Settings for BIP_Player_OpenPidChannel()

See Also:
BIP_Player_Prepare
BIP_Player_PrepareAsync
BIP_Player_GetDefaultOpenPidChannelSettings
**/
typedef struct BIP_PlayerOpenPidChannelSettings
{
    NEXUS_PlaybackPidChannelSettings    pidSettings;            /*!< Specify any custom pidChannel settings. */

    BIP_SETTINGS(BIP_PlayerOpenPidChannelSettings)              /*!< Internal use... for init verification. */
} BIP_PlayerOpenPidChannelSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerOpenPidChannelSettings);

/**
Summary:
Get default settings for BIP_Player_OpenPidChannel() and BIP_Player_OpenPidChannelAsync().

See Also:
BIP_PlayerOpenPidChannelSettings
BIP_Player_OpenPidChannel
BIP_Player_OpenPidChannelAsync
**/
#define BIP_Player_GetDefaultOpenPidChannelSettings(pSettings)     \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerOpenPidChannelSettings)    \
        /* Set non-zero defaults explicitly. */                                 \
        NEXUS_Playback_GetDefaultPidChannelSettings(&(pSettings)->pidSettings);\
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
OpenPidChannel for a specific track.

Description:
This is an optional API that allows app to open a pid channel associated a given trackId.

Note:
BIP_Player_Prepare() API already opens pid channel associated w/ Audio & Video track.
This API allows app to open any additional pid channels & use them as desired by the App.

Here are few ways this API can be used by App:
-for audio priming: open pid channels on all audio tracks (in addition to the one returned by BIP_Player_Prepare())
-for background recording where app may want to have independent pid channels for recording all tracks
-for streaming out of a pid channel.

See Also:
BIP_Player_OpenPidChannel
BIP_Player_ClosePidChannel
BIP_Player_CloseAllPidChannel
BIP_PlayerOpenPidChannelSettings
BIP_Player_GetDefaultOpenPidChannelSettings
**/
BIP_Status  BIP_Player_OpenPidChannel(
    BIP_PlayerHandle                        hPlayer,        /*!< [in] Handle of BIP_Player. */
    unsigned                                trackId,        /*!< Unique Track ID (PID for MPEG2-TS, track_ID for ISOBMFF). */
    const BIP_PlayerOpenPidChannelSettings  *pSettings,     /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    NEXUS_PidChannelHandle                  *phPidChannel   /*!< [out] pointer to the pid channel handle that is opened for the given trackId. */
    );

/**
Summary:
Close a particular PidChannel that was opened using BIP_Player_OpenPidChannel()

Note:
App is only allowed to close the Channels it has explicitly opened via BIP_Player_OpenPidChannel()
using these explict BIP_Player_ClosePidChannel API. The AV PidChannel which were returned in the
status of BIP_Player_Prepare() are internally closed by the Player during BIP_Player_Disconnect().

See Also:
BIP_Player_OpenPidChannel
BIP_Player_ClosePidChannel
BIP_Player_CloseAllPidChannel
BIP_PlayerOpenPidChannelSettings
BIP_Player_GetDefaultOpenPidChannelSettings
**/
BIP_Status  BIP_Player_ClosePidChannel(
    BIP_PlayerHandle                        hPlayer,        /*!< [in] Handle of BIP_Player. */
    NEXUS_PidChannelHandle                  hPidChannel     /*!< [in] pointer to the pid channel handle that is opened for the given trackId. */
    );

/**
Summary:
Close All PidChannel that were opened using BIP_Player_OpenPidChannel()

See Also:
BIP_Player_OpenPidChannel
BIP_Player_ClosePidChannel
BIP_Player_CloseAllPidChannel
BIP_PlayerOpenPidChannelSettings
BIP_Player_GetDefaultOpenPidChannelSettings
**/
BIP_Status  BIP_Player_CloseAllPidChannels(
    BIP_PlayerHandle                        hPlayer         /*!< [in] Handle of BIP_Player. */
    );

/****************************************************************************
 *  BIP_Player_Start()
 *  BIP_Player_StartAsync()
*****************************************************************************/
/**
Summary:
Settings for BIP_Player_Start() and BIP_Player_StartAsync().

See Also:
BIP_Player_Start
BIP_Player_StartAsync
BIP_Player_GetDefaultStartSettings
**/
typedef struct BIP_PlayerStartSettings
{
    unsigned                                timePositionInMs;       /*!< initial time offset in milliseconds */
    int                                     timeoutInMs;            /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */

    /*!< Following settings are required if App wants to change tracks during runtime (which requires BIP Player to internally Stop & Start Decoders). */
    NEXUS_VideoDecoderStartSettings         videoStartSettings;
    NEXUS_AudioDecoderStartSettings         audioStartSettings;

    NEXUS_SimpleAudioDecoderStartSettings   simpleAudioStartSettings;
    NEXUS_SimpleVideoDecoderStartSettings   simpleVideoStartSettings;

    BIP_SETTINGS(BIP_PlayerStartSettings)                           /*!< Internal use... for init verification. */
} BIP_PlayerStartSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerStartSettings);

/**
Summary:
Get default settings for BIP_Player_Start() and BIP_Player_StartAsync().

See Also:
BIP_PlayerStartSettings
BIP_Player_Start
BIP_Player_StartAsync
**/
#define BIP_Player_GetDefaultStartSettings(pSettings)     \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerStartSettings)    \
        /* Set non-zero defaults explicitly. */                                 \
        (pSettings)->timeoutInMs = -1; /* blocking */                           \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Start BIP Player session.

See Also:
BIP_Player_StartAsync
BIP_PlayerStartSettings
BIP_Player_GetDefaultStartSettings
**/
BIP_Status  BIP_Player_Start(
    BIP_PlayerHandle                hPlayer,        /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerStartSettings   *pSettings      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Async version of BIP_Player_Start().
Start BIP Player session.

See Also:
BIP_Player_Start
BIP_PlayerStartSettings
BIP_Player_GetDefaultStartSettings
**/
BIP_Status  BIP_Player_StartAsync(
    BIP_PlayerHandle                hPlayer,         /*!< [in] Handle of BIP_Player. */
    const BIP_PlayerStartSettings   *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                      *pAsyncStatus    /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_Stop()
 ****************************************************************************/
/**
Summary:
Stop BIP Player session.

See Also:
BIP_Player_Start
**/
void  BIP_Player_Stop(
    BIP_PlayerHandle hPlayer        /*!< [in] Handle of BIP_Player. */
    );

/****************************************************************************
 *  BIP_Player_Pause()
*****************************************************************************/
/**
Summary:
Various methods to pause a playback session

Description:
Only applicable for HTTP transport where either client can stall TCP connection or disconnect & reconnect w/ time seek
**/
typedef enum BIP_PlayerPauseMethod
{
    BIP_PlayerPauseMethod_eUseConnectionStalling,  /*!< Stall TCP connection by stop reading from socket: provides instant resumption. */
    BIP_PlayerPauseMethod_eUseDisconnectAndSeek,   /*!< Close current TCP connection and re-open it when Player resumes playback. */
    BIP_PlayerPauseMethod_eMax
} BIP_PlayerPauseMethod;

/**
Summary:
Settings for BIP_Player_Pause().

See Also:
BIP_Player_GetDefaultPauseSettings
BIP_Player_Pause
**/
typedef struct BIP_PlayerPauseSettings
{
    BIP_PlayerPauseMethod       pauseMethod;

    BIP_SETTINGS(BIP_PlayerPauseSettings)      /*!< Internal use... for init verification. */
} BIP_PlayerPauseSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerPauseSettings);

/**
Summary:
Get default settings for BIP_Player_Pause().

See Also:
BIP_PlayerPauseSettings
BIP_Player_Pause
**/
#define BIP_Player_GetDefaultPauseSettings(pSettings)                               \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerPauseSettings)          \
        /* Set non-zero defaults explicitly. */                                     \
        (pSettings)->pauseMethod = BIP_PlayerPauseMethod_eUseConnectionStalling;    \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Pause playback of a BIP_Player.

See Also:
BIP_Player_Play
BIP_PlayerPauseSettings
BIP_Player_GetDefaultPauseSettings
**/
BIP_Status BIP_Player_Pause(
    BIP_PlayerHandle        hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerPauseSettings *pSettings
    );

/****************************************************************************
 *  BIP_Player_Play()
 *  BIP_Player_PlayAsync()
 ****************************************************************************/
/**
Summary:
Start (or resume) playback from the current stream position at normal speed.

See Also:
BIP_Player_PlayAsync
BIP_Player_Pause
BIP_Player_Seek
BIP_Player_PlayByFrame
BIP_Player_PlayAtRate
**/
BIP_Status BIP_Player_Play(
    BIP_PlayerHandle    hPlayer        /*!< [in] Handle of BIP_Player. */
    );

/**
Summary:
Async version of BIP_Player_Play().
Start (or resume) playback from the current stream position at normal speed.

See Also:
BIP_Player_Play
BIP_Player_Pause
BIP_Player_Seek
BIP_Player_PlayByFrame
BIP_Player_PlayAtRate
**/
BIP_Status BIP_Player_PlayAsync(
    BIP_PlayerHandle    hPlayer,         /*!< [in] Handle of BIP_Player. */
    BIP_CallbackDesc    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status          *pAsyncStatus    /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_Seek()
 *  BIP_Player_SeekAsync()
 ****************************************************************************/
/**
Summary:
Seek to a new position in the stream.

See Also:
BIP_Player_SeekAsync
BIP_Player_Pause
BIP_Player_Play
BIP_Player_PlayByFrame
BIP_Player_PlayAtRate
**/
BIP_Status BIP_Player_Seek(
    BIP_PlayerHandle    hPlayer,            /*!< [in] Handle of BIP_Player. */
    unsigned            seekPositionInMs    /*!< [in] Time position in milliseconds to Seek to. */
    );

/**
Summary:
Async version of BIP_Player_Seek().
Seek to a new position in the stream.

See Also:
BIP_Player_Seek
BIP_Player_Pause
BIP_Player_Play
BIP_Player_PlayByFrame
BIP_Player_PlayAtRate
**/
BIP_Status BIP_Player_SeekAsync(
    BIP_PlayerHandle    hPlayer,           /*!< [in] Handle of BIP_Player. */
    unsigned            seekPositionInMs,  /*!< [in] Time position in milliseconds to Seek to. */
    BIP_CallbackDesc    *pAsyncCallback,   /*!< [in] Async completion callback: called at API completion. */
    BIP_Status          *pAsyncStatus      /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_PlayByFrame()
 *  BIP_Player_PlayByFrameAsync()
 ****************************************************************************/
/**
Summary:
From the Paused state, play a single frame then remain paused.

See Also:
BIP_Player_PlayByFrameAsync
BIP_Player_Pause
BIP_Player_Play
BIP_Player_Seek
BIP_Player_PlayAtRate
**/

BIP_Status BIP_Player_PlayByFrame(
    BIP_PlayerHandle    hPlayer,        /*!< [in] Handle of BIP_Player. */
    bool                forward         /*!< [in] If true, play 1 frame in the forward direction, otherwise, 1 frame in the reverse direction. */
    );

/**
Summary:
Async version of BIP_Player_PlayByFrame().
From the Paused state, play a single frame then remain paused.

See Also:
BIP_Player_PlayByFrame
BIP_Player_Pause
BIP_Player_Play
BIP_Player_Seek
BIP_Player_PlayAtRate
**/

BIP_Status BIP_Player_PlayByFrameAsync(
    BIP_PlayerHandle    hPlayer,          /*!< [in] Handle of BIP_Player. */
    bool                forward,          /*!< [in] If true, play 1 frame in the forward direction, otherwise, 1 frame in the reverse direction. */
    BIP_CallbackDesc    *pAsyncCallback,  /*!< [in] Async completion callback: called at API completion. */
    BIP_Status          *pAsyncStatus     /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_PlayAtRate()
 *  BIP_Player_PlayAtRateAsync()
 ****************************************************************************/
/**
Summary:
Various methods that can be used to play at non-normal rates.
**/
typedef enum BIP_PlayerPlayRateMethod
{
    BIP_PlayerPlayRateMethod_eAuto,                         /*!< Player will automatically determine the appropriate trickmode method: uses Server side trickmode (PlaySpeed) if supported by Server, otherwise, uses Client side trickmode. */
    BIP_PlayerPlayRateMethod_eUseByteRange,                 /*!< Client side trick play: Uses HTTP Range header to request chunks of data */
    BIP_PlayerPlayRateMethod_eUseTimeSeekRange,             /*!< Server side trick play: Uses Time ranges instead of byte ranges to achieve affect of trick modes */
    BIP_PlayerPlayRateMethod_eUsePlaySpeed,                 /*!< Server side trick play: informs server about the desired play speed and play the returned scaled data */
    BIP_PlayerPlayRateMethod_eMax
} BIP_PlayerPlayRateMethod;

/**
Summary:
Various methods that can be used with adaptive protocols (HLS, MPEG-DASH) to play at non-normal rates.
**/
typedef enum BIP_PlayerAdaptiveStreamingPlayRateMethod
{
    BIP_PlayerAdaptiveStreamingPlayRateMethod_eUseSegmentWithLowestBandwidth,
    BIP_PlayerAdaptiveStreamingPlayRateMethod_eUseSegmentWithMatchingRateAndNetworkBandwidth,
    BIP_PlayerAdaptiveStreamingPlayRateMethod_eMax
} BIP_PlayerAdaptiveStreamingPlayRateMethod;

/**
Summary:
Settings for BIP_Player_PlayAtRate() and BIP_Player_PlayAtRateAsync().

See Also:
BIP_Player_GetDefaultPlayAtRateSettings
BIP_Player_PlayAtRate
BIP_Player_PlayAtRateAsync
**/
typedef struct BIP_PlayerPlayAtRateSettings
{
    BIP_SETTINGS(BIP_PlayerPlayAtRateSettings)

    BIP_PlayerPlayRateMethod                   playRateMethod;
    BIP_PlayerAdaptiveStreamingPlayRateMethod  adaptiveStreamingPlayRateMethod;
    NEXUS_PlaybackTrickModeSettings            playRateSettings;
} BIP_PlayerPlayAtRateSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerPlayAtRateSettings);

/**
Summary:
Get default settings for BIP_Player_PlayAtRate() and BIP_Player_PlayAtRateAsync().

See Also:
BIP_PlayerPlayAtRateSettings
BIP_Player_PlayAtRate
BIP_Player_PlayAtRateAsync
**/
#define BIP_Player_GetDefaultPlayAtRateSettings(pSettings)                                  \
        BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, BIP_PlayerPlayAtRateSettings)             \
        /* Set non-zero defaults explicitly. */                                             \
        NEXUS_Playback_GetDefaultTrickmodeSettings(&(pSettings)->playRateSettings);         \
        (pSettings)->playRateMethod = BIP_PlayerPlayRateMethod_eAuto;                       \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Start/resume playback at non-normal rate (fast or slow, forward or reverse).

Description:
If Paused, begin playback at the specified rate.
If Playing, change to the specified playback rate.

See Also:
BIP_Player_PlayAtRateAsync
BIP_Player_Pause
BIP_Player_Play
BIP_Player_Seek
BIP_Player_PlayByFrame
BIP_Player_GetDefaultPlayAtRateSettings
BIP_PlayerPlayAtRateSettings
**/
BIP_Status BIP_Player_PlayAtRateAsString(
    BIP_PlayerHandle                    hPlayer,        /*!< [in] Handle of BIP_Player. */
    const char                          *pRate,         /*!< [in] Playback rate as a multiple of normal rate. */
    const BIP_PlayerPlayAtRateSettings  *pSettings      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

BIP_Status BIP_Player_PlayAtRate(
    BIP_PlayerHandle                    hPlayer,        /*!< [in] Handle of BIP_Player. */
    int                                 rate,           /*!< [in] Playback rate defined in NEXUS_NORMAL_PLAY_SPEED units. See details in nexus_playback.h */
    const BIP_PlayerPlayAtRateSettings  *pSettings      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Async version of BIP_Player_PlayAtRate().
Start/resume playback at non-normal rate (fast or slow, forward or reverse).

Description:
If Paused, begin playback at the specified rate.
If Playing, change to the specified playback rate.

See Also:
BIP_Player_PlayAtRate
BIP_Player_Pause
BIP_Player_Play
BIP_Player_Seek
BIP_Player_PlayByFrame
BIP_Player_GetDefaultPlayAtRateSettings
BIP_PlayerPlayAtRateSettings
**/
BIP_Status BIP_Player_PlayAtRateAsStringAsync(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    const char                          *pRate,          /*!< [in] Playback rate as a multiple of normal rate. */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    );

BIP_Status BIP_Player_PlayAtRateAsync(
    BIP_PlayerHandle                    hPlayer,         /*!< [in] Handle of BIP_Player. */
    int                                 rate,            /*!< [in] Playback rate defined in NEXUS_NORMAL_PLAY_SPEED units. See details in nexus_playback.h */
    const BIP_PlayerPlayAtRateSettings  *pSettings,      /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    BIP_CallbackDesc                    *pAsyncCallback, /*!< [in] Async completion callback: called at API completion. */
    BIP_Status                          *pAsyncStatus    /*!< [out] Completion status of async API. */
    );

/****************************************************************************
 *  BIP_Player_Disconnect()
 ****************************************************************************/
/**
Summary:
Disconnect BIP Player.

See Also:
BIP_Player_Connect
**/
void  BIP_Player_Disconnect(
    BIP_PlayerHandle hPlayer        /*!< [in] Handle of BIP_Player. */
    );


/****************************************************************************
 *  BIP_Player_GetStatus()
 ****************************************************************************/
/**
Summary:
Status returned by BIP_Player_GetStatus().

See Also:
BIP_Player_GetStatus
**/
typedef struct BIP_PlayerHlsContainerStats
{
    unsigned lastSegmentDownloadTime;           /* time, in msec, taken to download the last segment: spans from HTTP Get Request to full segment download */
    unsigned lastSegmentBitrate;                /* bit rate, in bips per sec (bps), associated with the segments currently being downloaded & played out. */
    unsigned lastSegmentDuration;               /* duration, in msec, of the last segment fed to the playback h/w channel */
    unsigned lastSegmentSequence;               /* sequence number of the last segment fed to the playback h/w channel */
    const char *lastSegmentUrl;                 /* URL (i.e. playlist name) of the last segment fed to the playback h/w channel */
    bool     bounded;                           /* set to true if EXT-ENDLIST tag is present, meaning the Playlist represents the bounded content. */
} BIP_PlayerHlsContainerStats;

typedef struct BIP_PlayerRtpStats
{
    uint32_t packetsReceived;                   /* total IP/UDP/RTP packets received */
    uint64_t bytesReceived;                     /* total AV Payload bytes received in RTP Packets */
    uint32_t packetsDiscarded;                  /* total packets discarded: */
    uint32_t packetsOutOfSequence;              /* total packets whose RTP sequence doesn't match the expected next sequence */
    uint32_t packetsLost;                       /* total packets that were lost */
    uint32_t packetsLostBeforeErrorCorrection;  /* */
    uint32_t lossEvents;                        /* total number of times an expected packet was lost */
    uint32_t lossEventsBeforeErrorCorrection;   /* */
} BIP_PlayerRtpStats;

typedef struct BIP_PlayerStats
{
    off_t                       totalConsumed;  /*!< Total bytes consumed by the Media player */
    BIP_PlayerHlsContainerStats hlsStats;       /*!< HLS Container related stats. */
    BIP_PlayerRtpStats          rtpStats;       /*!< RTP related stats. */
} BIP_PlayerStats;

typedef enum BIP_PlayerState
{
    BIP_PlayerState_eDisconnecting,     /* App has called BIP_Player_Disconnect() and is in the process of Disconnecting. */
    BIP_PlayerState_eDisconnected,      /* App has finished calling BIP_Player_Create() for a given URL. */
                                        /* Player can also enter this state if app has called BIP_Player_Disconnect() any timer after BIP_Player_Connect(). */
    BIP_PlayerState_eConnecting,        /* App has called BIP_Player_Connect(). Player is in the process of setting up TCP & HTTP level interaction w/ the Server. */
    BIP_PlayerState_eConnected,         /* Player has successfully finished BIP_Player_Connect() and has issued async completion callback (if needed). */
    BIP_PlayerState_eProbing,           /* App has called BIP_Player_ProbeForMediaInfo(). Player is in the process of probing MediaInfo. */
    BIP_PlayerState_ePreparing,         /* App has called BIP_Player_Prepare(). Player is in the process of setting up Playpump/Playback/PidChannels/Clock-Recovery, etc. */
    BIP_PlayerState_ePrepared,          /* Player has successfully finished BIP_Player_Prepare() & issued async completion callback (if needed). */
                                        /* Player can also enter this state if app has called BIP_Player_Stop() after Starting the Player. */
    BIP_PlayerState_eStarting,          /* App has called BIP_Player_Start(). Player is in the process of starting the Media Stream Playback. */
    BIP_PlayerState_eStarted,           /* App has successfully finished the BIP_Player_Start() & issued the async completion callback. Media Stream is now Playing unless Player was prepared in the pause mode. */
    BIP_PlayerState_ePaused,            /* App has called BIP_Player_Paused() to Pause the Media Playback or Player was prepared to Start in the Pause mode. */

    BIP_PlayerState_eAborted,           /* Player has run into some during either in the Started or Paused state or during transition from these states. App can only be Stopped/Disconnected/Destroyed now. */

    BIP_PlayerState_eMax
} BIP_PlayerState;

typedef enum BIP_PlayerSubState
{
    BIP_PlayerSubState_eIdle,                               /* Idle state: player is not performing some active task at this time. */

    /* Substates for BIP_PlayerState_eConnecting. */
    BIP_PlayerSubState_eConnectingNew,                      /* Player is setting up the TCP Connection, building & sending HTTP Request to Server. */
    BIP_PlayerSubState_eConnectingWaitForResponse,          /* Player is waiting for HTTP Response from Server & its processing (including HTTP Redirect handling). */
    BIP_PlayerSubState_eConnectingWaitForDtcpIpAke,         /* If HTTP Response indicated content being protected w/ DTCP/IP, Player is in process of doing DTCP AKE handshake w/ DTCP/IP Server. */
    BIP_PlayerSubState_eConnectingDone,                     /* Player has completed the Connect related processing with either SUCCESS or failure results. */

    /* Substates for BIP_PlayerState_eProbing. */
    BIP_PlayerSubState_eProbingNew,                         /* Player is in the process setting up the Media Probe. */
    BIP_PlayerSubState_eProbingWaitForMediaInfo,            /* Probe is waiting for Media Probe  to finish. */
    BIP_PlayerSubState_eProbingDone,                        /* Probe is complete and Player is going to notify app about it. */

    /* Substates for BIP_PlayerState_ePreparing. */
    BIP_PlayerSubState_ePreparingNew,                       /* Player is the new state to start preparing the media. */
    BIP_PlayerSubState_ePreparingWaitForMediaInfo,          /* Player had started the Media Probe and is waiting for it to finish. */
    BIP_PlayerSubState_ePreparingMediaInfoAvailable,        /* Prepare work is now continuing: selection of Nexus Playback/Playpump, creating of AV PidChannels, live/pull mode clock recovery setup, etc. */
    BIP_PlayerSubState_ePreparingDone,                      /* Prepare is now complete. */

    /* Substates for BIP_PlayerState_eStarting. */
    BIP_PlayerSubState_eStartingNew,                        /* Player is in the process of starting. */
    BIP_PlayerSubState_eStartingWaitForPbipStart,           /* Player is waiting for some internal tasks to finish before completing. */
    BIP_PlayerSubState_eStartingWaitForConnectCompletion,   /* Player has issued BIP_Socket_Connect & is waiting for connect completion. */
    BIP_PlayerSubState_eStartingWaitForHttpResponse,        /* Player has sent HTTP Request & is waiting for HTTP Response before completing starting state. */
    BIP_PlayerSubState_eStartingDone,                       /* Player is started. */

    /* Substates for BIP_PlayerState_eStarted. */
    BIP_PlayerSubState_eStartedPlayingNormal,               /* Player is now Playing the Media at Normal 1x Rate. */
    BIP_PlayerSubState_eStartedPlayingTrickmode,            /* Player is now Playing the Media at Trickmode non-1x Rate. */
    BIP_PlayerSubState_eStartedWaitForPauseCompletion,      /* Player is now waiting for Pause operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_eStartedWaitForSeekCompletion,       /* Player is now waiting for Seek operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_eStartedWaitForPlayCompletion,       /* Player is now waiting for Play operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_eStartedWaitForPlayAtRateCompletion, /* Player is now waiting for PlaySpeed operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_eStartedWaitForStopCompletion,       /* Player is now waiting for Stop operation to complete. */

    /* Substates for BIP_PlayerState_ePaused. */
    BIP_PlayerSubState_ePaused,                             /* Player is now paused. */
    BIP_PlayerSubState_ePausedWaitForPlayCompletion,        /* Player is now waiting for Play operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_ePausedWaitForPlayByFrameCompletion, /* Player is now waiting for PlayByFrame operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_ePausedWaitForSeekCompletion,        /* Player is now waiting for Seek operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_ePausedWaitForPlayAtRateCompletion,  /* Player is now waiting for PlaySpeed operation to complete: wait for HTTP Response from Server. */
    BIP_PlayerSubState_ePausedWaitForStopCompletion,        /* Player is now waiting for Stop operation to complete. */

    /* Substates for BIP_PlayerState_eAborted. */
    BIP_PlayerSubState_eAborted,                            /* Player is now in Aborted subState. */
    BIP_PlayerSubState_eAbortedWaitForStopCompletion,       /* Player is now waiting for Stop operation to complete. */

    /* Substates for BIP_PlayerState_eDisconnecting. */
    BIP_PlayerSubState_eDisconnectingNew,                   /* Player is now disconnecting. */
    BIP_PlayerSubState_eDisconnectingDone,                  /* Call Arb_CompleteRequest() */

    BIP_PlayerSubState_eMax
} BIP_PlayerSubState;

/**
Summary:
Get Player Mode

See Also:
BIP_PlayerStatus
**/
typedef enum BIP_PlayerMode
{
    BIP_PlayerMode_eVideoOnlyDecode,        /* Only Video Decoder handle is provided. */
    BIP_PlayerMode_eAudioOnlyDecode,        /* Only Audio Decoder handle is provided. */
    BIP_PlayerMode_eAudioVideoDecode,       /* Both AV Decoder handles are provided. */
    BIP_PlayerMode_eRecord                  /* AV Decoder handles's are not provided. */
} BIP_PlayerMode;

typedef struct BIP_PlayerStatus
{
    NEXUS_PlaybackPosition          currentPositionInMs;    /*!< Current position of stream played in the firstPositionInMs - lastPositionInMs range. */
    NEXUS_PlaybackPosition          firstPositionInMs;      /*!< First seekable position: 0 for live channels w/o timeshift, seekRangeBegin for timeshifted content, 0 for pre-recorded files. */
    NEXUS_PlaybackPosition          lastPositionInMs;       /*!< Last seekable position:  0 for live channels w/o timeshift, seekRangeEnd for timeshifted content, media duration for pre-recorded files. */
                                                            /*!< Note: first & last positions are cached values from the last Response from the Server */
                                                            /*!< (either for an explicit getAvailableSeekRange.dlna.org request or via the HTTP Response). */
    BIP_PlayerStats                 stats;                  /*!< Player Status. */
    BIP_PlayerState                 state;                  /*!< Media Player's main state. */
    BIP_PlayerSubState              subState;               /*!< Media Player's sub-state. */
    BIP_PlayerClockRecoveryMode     clockRecoveryMode;      /*!< ClockRecoveryMode current being used for Playing the stream. */
    int                             rate;                   /*!< Current Playback rate. */
    BIP_PlayerDataAvailabilityModel dataAvailabilityModel;  /*!< Specifies how the Stream's AV data is accessible. */
    BIP_PlayerPrepareStatus         prepareStatus;          /*!< Status associated w/ the last Prepare work done via the BIP_Player_Prepare() or BIP_Player_SetSettings(). */
    BIP_PlayerMode                  mode;
    BIP_MediaInfoHandle             hMediaInfo;             /*!< Current MediaInfo Handle associated with the stream. */
                                                            /*!< This handle can be different from the one returned during initial Probe if App enables Player to detect any PSI changes in the stream & seemlessly play the next trackGroup (program). */
                                                            /*!< App enables this logic by not explicitly selecting the initial AV Tracks using their trackIds & instead selecting them either using a preference or leaving the trackIds un-initialized. */
                                                            /*!< Please review the BIP_PlayerSettings for further details. */

    NEXUS_SimpleAudioDecoderHandle  hSimpleAudioDecoder;    /*!< Audio Decoder handle of the currently playing audio track (can be different from the initial one if enableAudioPrimer is set & audio tracks are switched). */
    NEXUS_PidChannelHandle          hAudioPidChannel;       /*!< PID Channel handle associated with the currently playing audio track. */

} BIP_PlayerStatus;

/**
Summary:
Get current status of a BIP_Player

See Also:
BIP_PlayerStatus
**/
BIP_Status  BIP_Player_GetStatus(
    BIP_PlayerHandle    hPlayer,        /*!< [in] Handle of BIP_Player. */
    BIP_PlayerStatus    *pStatus        /*!< [out] Pointer to caller's struct to receive the current status. */
    );

/**
Summary:
Print the current status of a BIP_Player

See Also:
BIP_Player_GetStatus
**/
void  BIP_Player_PrintStatus(
    BIP_PlayerHandle    hPlayer        /*!< [in] Handle of BIP_Player. */
    );

/**
Summary:
Settings for BIP_Player_GetStatusFromServer() and BIP_Player_GetStatusFromServerAsync().

See Also:
BIP_Player_GetStatus
BIP_Player_GetDefaultGetStatusFromServerSettings
BIP_Player_GetStatusFromServer
BIP_Player_GetStatusFromServerAsync
**/
typedef struct BIP_PlayerGetStatusFromServerSettings
{
    bool        getAvailableSeekRange;              /*!< If true, Player will return the seekable time range for the stream currently being played. */
                                                    /*!< The seekable range is available via BIP_PlayerStatusFromServer.availableSeekRange structure. */
    int         timeoutInMs;                        /*!< API timeout: This API fails if it is not completed in this timeout interval. -1: waits until completion or error. */

    BIP_SETTINGS(BIP_PlayerGetStatusFromServerSettings)         /*!< Internal use... for init verification. */
} BIP_PlayerGetStatusFromServerSettings;
BIP_SETTINGS_ID_DECLARE(BIP_PlayerGetStatusFromServerSettings);

/**
Summary:
Status returned from BIP_Player_GetStatusFromServer() and BIP_Player_GetStatusFromServerAsync().

Description:
The values returned in BIP_PlayerStatusFromServer are usually needed for determining the Server stats such as seekable range for timeshifted content, etc.

See Also:
BIP_PlayerGetStatusFromServerSettings
BIP_Player_GetDefaultGetStatusFromServerSettings
BIP_Player_GetStatusFromServer
BIP_Player_GetStatusFromServerAsync
**/
typedef struct BIP_PlayerStatusFromServer
{
    struct
    {
        NEXUS_PlaybackPosition  firstPositionInMs;                  /*!< First seekable position in the current stream being played. */
        NEXUS_PlaybackPosition  lastPositionInMs;                   /*!< Last seekable position in the current stream being played. */
        NEXUS_PlaybackPosition  durationInMs;                       /*!< Duration of Seekable Range in Ms. */
    } availableSeekRange;

    NEXUS_PlaybackPosition      currentPositionInMs;                /*!< Current position of stream played in mSec */
} BIP_PlayerStatusFromServer;

/**
Summary:
Get default settings for BIP_Player_GetStatusFromServer() and BIP_Player_GetStatusFromServerAsync().

See Also:
BIP_PlayerGetStatusFromServerSettings
BIP_PlayerStatusFromServer
BIP_Player_GetStatusFromServer
BIP_Player_GetStatusFromServerAsync
**/
#define BIP_Player_GetDefaultGetStatusFromServerSettings(pGetStatusFromServerSettings) \
        BIP_SETTINGS_GET_DEFAULT_BEGIN((pGetStatusFromServerSettings), BIP_PlayerGetStatusFromServerSettings)    \
        /* Set non-zero defaults explicitly. */                         \
        (pGetStatusFromServerSettings)->getAvailableSeekRange = true;   \
        (pGetStatusFromServerSettings)->timeoutInMs = -1;   /* blocking */            \
        BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
API to Get Status From Server.

See Also:
BIP_Player_GetStatusFromServerAsync
BIP_Player_GetDefaultStatusFromServerSettings
BIP_PlayerGetStatusFromServerSettings
BIP_PlayerStatusFromServer
**/
BIP_Status BIP_Player_GetStatusFromServer(
    BIP_PlayerHandle                         hPlayer,               /*!< [in]  Handle of BIP_Player. */
    BIP_PlayerGetStatusFromServerSettings    *pSettings,            /*!< [in]  Required: What settings to get from Server. */
    BIP_PlayerStatusFromServer               *pServerStatus         /*!< [out] Required: Important values returned by BIP_Player_GetStatusFromServer() */
    );

/**
Summary:
Async API to Get Status From Server.

See Also:
BIP_Player_GetStatusFromServer
BIP_Player_GetDefaultStatusFromServerSettings
BIP_PlayerGetStatusFromServerSettings
BIP_PlayerStatusFromServer
**/
BIP_Status BIP_Player_GetStatusFromServerAsync(
    BIP_PlayerHandle                         hPlayer,               /*!< [in]  Handle of BIP_Player. */
    BIP_PlayerGetStatusFromServerSettings    *pSettings,            /*!< [in]  Required: What settings to get from Server. */
    BIP_PlayerStatusFromServer               *pServerStatus,        /*!< [out] Required: Important values returned by BIP_Player_GetStatusFromServer() */
    BIP_CallbackDesc                         *pAsyncCallback,       /*!< [in]  Async completion callback: called at API completion. */
    BIP_Status                               *pAsyncStatus          /*!< [out] status of async API. */
    );

#ifdef __cplusplus
}
#endif
#endif /* #ifndef BIP_PLAYER_H */
