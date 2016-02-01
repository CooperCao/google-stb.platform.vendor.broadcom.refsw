/***************************************************************************
*     (c)2008-2015 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: b_client.h $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: Broadcom IP (BIP) Client library API definition file
*
* Revision History:
*
* $brcm_Log: /nexus/lib/bip/include/bip_client.h $
*
*
***************************************************************************/
#ifndef BIP_CLIENT_H
#define BIP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_types.h"

#include "nexus_types.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"

/**
BIP Client APIs list:

BIP_ClientHandle BIP_Client_Create();
void BIP_Client_Destroy();

#if 0
void BIP_Client_GetDefaultSetupSettings();
BIP_Status BIP_Client_Setup();
#else
BIP_Status BIP_Client_TuneUrl(url);
BIP_Status BIP_Client_TuneUrlAync(url);
BIP_Status BIP_Client_TuneAyncUrl(url);
BIP_Status BIP_Client_AyncTuneUrl(url);
#endif

void BIP_Client_GetDefaultPidChannelSettings();
NEXUS_PidChannelHandle BIP_Client_OpenPidChannel();
BIP_Status BIP_Client_GetPidChannelSettings();
BIP_Status BIP_Client_SetPidChannelSettings();
void BIP_Client_ClosePidChannel();
void BIP_Client_CloseAllPidChannels();

void BIP_Client_GetDefaultSettings();
void BIP_Client_GetSettings();
BIP_Status BIP_Client_SetSettings();

void BIP_Client_GetDefaultStartSettings();
BIP_Status BIP_Client_Start();
void BIP_Client_Stop();

BIP_Status BIP_Client_GetStatus();

BIP_Status BIP_Client_Play():
BIP_Status BIP_Client_Pause():
BIP_Status BIP_Client_Seek():
BIP_Status BIP_Client_Trickmode():
BIP_Status BIP_Client_GetTrickModeSettings();

**/

typedef struct BIP_Client  *BIP_ClientHandle;

/**
Summary:
Create an instance required to play a IP Client session

Description:
Allocates resources that are needed to run a BIP Client Session. Should be called at the app initialization time
to avoid memory heap fragmentation.
Same instance should be used for consecutive sessions using by restarting sessions

Notes:
similar to BIP_ClientOpen/_Close()
allocates memory for worst case Client session usage but allows app to specify the usage (RtpSession vs HttpSession etc.)

Questions:

**/
BIP_ClientHandle  BIP_Client_Create(
    /* TODO: if parameters, then add GetDefaultInitSettings() */
    void
    );

/**
Summary:
Destroy a BIP Client instance

Description:
**/
void  BIP_Client_Destroy(
    BIP_ClientHandle hBipClient
    );

/**
Summary:
Callback ids invoked by BIP.
Some of these are invoked if BIP APIs called in the async mode. In such case, callback indicate that API is now completed. App still need to reinvoke the api corresponding to the event
for completion status. Only B_ERROR_SUCCESS return code indicates valid values in the returned Status structures.

Also, there are other events which are asynchronously generated and are not associated w/ any particular API.
App can invoke the GetStatus() API to retrieve the detailed event specific status.
**/
typedef enum
{
    /* In addition to setting up the network socket, HTTP Get Req & Resp are processed. Media probe is completed */
    /* For RTSP, OPTIONS,DESCRIBE and SETUP commands are sent and responses are processed */
    BIP_ClientCallbackId_eSetupDone,
    /* Event to indicate connection, request, or a command to server timed out */
    BIP_ClientCallbackId_eSetupTimeOut,
    /* Event to indicate SessionStart is completed (RTSP Play is completed) */
    BIP_ClientCallbackId_eSessionStartDone,
    /* Event to indicate completion of request to playback media at Normal 1x Speed */
    BIP_ClientCallbackId_ePlaying,
    /* Event to indicate completion of request to pause the current media playback */
    BIP_ClientCallbackId_ePaused,
    /* Event to indicate completion of request to pause the current media playback */
    BIP_ClientCallbackId_eStopped,
    /* Event to indicate completion of request to seek media playback to new position */
    BIP_ClientCallbackId_eSeekComplete,
    /* Event to indicate a forward (FFWD) or backward (FREW) scan is initiated */
    BIP_ClientCallbackId_eScanStarted,
    /* Event to indicate reached end of stream */
    BIP_ClientCallbackId_eEndofStreamReached,
    /* Event to indicate error during stream playback */
    BIP_ClientCallbackId_eErrorDuringStreamPlayback,
    /* Event to indicate start of buffering */
    BIP_ClientCallbackId_ePreChargeStart,
    /* Event to indicate buffer precharge is completed */
    BIP_ClientCallbackId_ePreChargeDone,
    /* Event to indicate IP tuner lock: when 1st packet is received in beginning or after a network timeout exceeding the defined network jitter for a live UDP/RTP/RTSP/HTTP channel */
    BIP_ClientCallbackId_eIpTunerLocked,
    /* Event to indicate IP tuner unlock: when no packet is received for network timeout exceeding the defined network jitter for a live UDP/RTP/RTSP/HTTP channel */
    BIP_ClientCallbackId_eIpTunerUnLocked,
    /* Event to indicate Buffer depth is violated at max end (for live TTS streams ) */
    BIP_ClientCallbackId_eMaxBufferDepthViolation,
    /* Event to indicate Buffer depth is violated at min end (for live TTS streams ) */
    BIP_ClientCallbackId_eMinBufferDepthViolation,
    /* Invalid event */
    BIP_ClientCallbackId_eMax
} BIP_ClientCallbackIds;

/**
 * Callback function to indicate either non-blocking API completion or some asynchronous event
 */
typedef void (*BIP_Client_Callback)( void *appCtx, BIP_ClientCallbackIds callbackId );

/**
 * LPCP Settings
 */
typedef struct BIP_ClientLpcmSettings
{
    bool        convertLpcmToWave;         /* if Set, app is requesting to convert incoming LPCM files into Wave by inserting the Wave header before the data */
    unsigned    bitsPerSample;
    unsigned    sampleRate;
    unsigned    numChannels;
} BIP_ClientLpcmSettings;

typedef enum BIP_ClientClockRecoveryMode
{
    BIP_ClientClockRecoveryMode_ePull,                  /* Pull mode: file playback where client is pulling the content from server: clock is initialied to 1st PTS and then freely run */
    BIP_ClientClockRecoveryMode_ePush,                  /* Push mode: server is pacing the content: in this type, we pick any clock recovery scheme unless overridden by a more specific push type below */
    BIP_ClientClockRecoveryMode_ePushWithPcrSyncSlip,   /* Content pushed by Server using PCR */
    BIP_ClientClockRecoveryMode_ePushWithTtsNoSyncSlip, /* SW Buffering & Content Paced by local Playback Hardware using TTS Timestamps */
    BIP_ClientClockRecoveryMode_ePushWithPcrNoSyncSlip, /* SW Buffering & Content Paced by local Playback Hardware using PCRs as Timestamps */
    BIP_ClientClockRecoveryMode_eMax                    /* SW Buffering & Content Paced by local Playback Hardware using PCRs as Timestamps */
} BIP_ClientClockRecoveryMode;

/**
 * TTS throttle parameters
 */
typedef struct BIP_ClientTtsParams
{
    bool        autoDetect;                 /* automatically detect if the stream is TTS or TS */
    unsigned    pacingMaxError;             /* set the timestamp error bound, as used by the playback pacing logic */
    /* TODO: I dont think app needs to pass-in the timebase. Nexus now allows its dynamic allocation, thus app doesn't need to know this */
    /* design question: should BIP internally open such h/w resources or make app pass them in? */
    /* Examples of such resources is: Timebase for decode & display, Stc Channel, how about Nexus playpump or playback */
    NEXUS_Timebase  timebase;               /* timebase that will be frequency controlled by the TTS throttle */
    unsigned        initBufDepth;           /* sets the initial buffer depth (used as a target buffer depth by TTS pacing) */
    unsigned        minBufDepth;            /* minimum buffer depth before buffer depth violation callback is called */
    unsigned        maxBufDepth;            /* minimum buffer depth before buffer depth violation callback is called */
    unsigned        maxClockMismatch;       /* specify the maximum clock mismatch (in ppm) between server/encoder and STB */
} BIP_ClientTtsParams;

typedef struct BIP_ClientNexusHandles
{
    NEXUS_PlaypumpHandle            playpump;               /* required for Live RTP/UDP/RTSP/Server Trickmodes based sessions */
    NEXUS_PlaybackHandle            playback;               /* required for HTTP based sessions */
    NEXUS_VideoDecoderHandle        videoDecoder;           /* required if video decode is enabled else must be NULL */
    NEXUS_SimpleVideoDecoderHandle  simpleVideoDecoder;     /* alternative handle for multi-process cases */
    NEXUS_SimpleAudioDecoderHandle  simpleAudioDecoder;     /* alternative solution for when audio decoder is not available */
    NEXUS_AudioDecoderHandle        primaryAudioDecoder;    /* required if audio decode is enabled else must be NULL */
    NEXUS_AudioDecoderHandle        secondaryAudioDecoder;  /* required if audio decode is enabled else must be NULL */
    NEXUS_StcChannelHandle          stcChannel;             /* required for video & audio enabled streams */
    NEXUS_SimpleStcChannelHandle    simpleStcChannel;       /* alternative to stcChannel handle for multiProcess nexus mode */
    NEXUS_HeapHandle                heapHandle;             /* nexus heap Handle, can be null if app doesn't have a preference */
    /* TODO: these may longer be needed as they are internally allocated by BIP? */
    NEXUS_PidChannelHandle  videoPidChannel;
    NEXUS_PidChannelHandle  extraVideoPidChannel;
    NEXUS_PidChannelHandle  audioPidChannel;
    NEXUS_PidChannelHandle  pcrPidChannel;
} BIP_ClientNexusHandles;

typedef enum BIP_ClientSessionProtocol
{
    BIP_ClientSessionProtocol_eSimpleHttp,
    BIP_ClientSessionProtocol_eHls,
    BIP_ClientSessionProtocol_eDash,
    BIP_ClientSessionProtocol_eRvu,
    BIP_ClientSessionProtocol_eSimpleUdp,
    BIP_ClientSessionProtocol_eRtp,
    BIP_ClientSessionProtocol_eMax
} BIP_ClientSessionProtocol;

typedef struct BIP_ClientSetupSettings
{
    /* ----------- Required ClientSetup Configuration ----------- */
    char  *url;                             /* absolute stream URL: <protocol>://<server-name/ip>[:port][/<resource>] e.g. http://192.168.1.10:5000/test.ts, udp://192.168.1.101:1234, rtp://224.1.1.10:1234 */
    BIP_ClientNexusHandles  nexusHandles;   /* required nexus handles */
    BIP_Client_Callback     callbackFunc;   /* callback function to indicate any asynchronous events from the IP library */
    void                    *appCtx;        /* app context handle used in the callback function */

    /* ----------- Recommended ClientSetup Configuration: set if known to App ----------- */
    NEXUS_TransportType         transportTypeHint;      /* app can hint the stream transport type if it is known via DLNA means or fixed for a channel, default is NEXUS_TransportType_eUnknown */
    BIP_ClientSessionProtocol   sessionProtocolHint;    /* app can hint session protocol: eRvu, eHls, eDash, etc. */
    off64_t  contentLengthHint;                         /* app can hint content (useful for chunk xfer encoding case where server doesn't send it in HTTP resp, default is 0 (not known) */

    /* TODO: determine if we can auto adjust the network timeout based on the initial req/response as app may not know whether it is playing from a local server, fast OTT server, or slow OTT server  */
    unsigned    networkTimeout;                 /* provides the timeout value, in seconds, for network operations like select, read. */
    unsigned    networkBufferSize;              /* size of network cache, in bytes, where data is buffered (currently used for HTTP based Pull Modes), 0: IP Applib uses a default size of couple of MB */
    char        *additionalHeaders;             /* additional HTTP headers that app wants to include in the outgoing Get Request: need to be \r\n terminated */
    char        *userAgent;                     /* app can override the default UserAgent string used in the outgoing HTTP Get Request: string only */
    bool        copyResponseHeaders;            /* if true, a copy of the HTTP response Headers is provided to app to allow it to process any custom Headers (e.g. getProtocolInfo DLNA header) */

    /* ----------- Optional ClientSetup Configuration ----------- */
    char    *proxyServerIpAddress;          /* if set, points to the proxy server's ip address */
    int     proxyServerIpPort;              /* if set, points to the proxy server's port# */
    char    *networkInterfaceName;          /* optional: interface name to send multicast joins requests on (for receiving live UDP/RTP sessions) */
    bool    preChargeBuffer;                /* true:  tells IP Applib to start pre-charging its network buffer, false (dont pre-charge) by default */
    bool    dontFeedDataToPlaybackHw;       /* app doesn't want playback_ip to feed data to playback h/w, instead it will read data from playback ip using setupStatus.file i/f and itself feed to h/w */
    long    psiParsingTimeLimit;            /* max number of msec allowed for psi parsing/media probe operation */
    bool    dontUseIndex;                   /* dont use index for playback: false by default */
    bool    enablePayloadScanning;          /* if true, set up probe engine to determine accurate stream duration using payload scanning & random access into stream. */
                                            /* Note: setting this option will increase the media probe duration, but is needed to determine accurate info for perforing client side trick modes on TS & PES/VOB formats */
    bool        skipPsiParsing;             /* if true, IP Applib doesn't probe the content for PSI information, app should apriori have this info. TODO: does app need to pass-in any of the PSI info to BIP? */
    unsigned    avgBitRateHint;             /* average stream bitrate in bps if already known or 0 if unknown */
                                            /* If App set skipPsiParsing, then it is required to set avgBitRate value if app also wants to enable Runtime Buffering */
    bool  disableRangeHeader;               /* if true, IP Applib doesn't include the Range header in the Get Requests */

    /* TODO: security related config */
} BIP_ClientSetupSettings;

/* App specific Header that gets inserted before media is streamed out */
typedef struct
{
    bool        valid;      /* set if appHeader is valid */
    unsigned    length;     /* length of app header bytes: should be mod16 aligned if it needs to be encrypted */
    uint8_t     data[192];  /* pointer to app header */
} BIP_ClientAppHeader;

typedef struct BIP_VideoTrackInfo
{
    bool                valid;
    int                 pid;
    NEXUS_VideoCodec    codec;                  /* Rest of the fields are the standard PSI information */
    int                 extraPid;               /* AVC Extension for SVC/MVC: extra video pid */
    NEXUS_VideoCodec    extraCodec;             /* AVC Extension for SVC/MVC: extra video codec */
    float               frameRate;              /* video frame rate in fps, or 0 if unknown */
    uint16_t            width;                  /* coded video width, or 0 if unknown */
    uint16_t            height;                 /* coded video height, or 0 if unknown  */
    unsigned            bitrate;                /* video bitrate in Kbps, or 0 if unknown */
} BIP_VideoTrackInfo;

typedef struct BIP_AudioTrackInfo
{
    bool                valid;                              /* if true, only then remaining fields contain valid PSI information */
    int                 pid;
    NEXUS_AudioCodec    codec;
    /* TODO: add audio language: look into what bmedia probe has */
    unsigned    bitsPerSample;                  /* number of bits in the each sample, or 0 if unknown */
    unsigned    sampleRate;                     /* audio sampling rate in Hz, or 0 if unknown */
    unsigned    numChannels;                    /* number of channels, or 0 if unknown  */
    unsigned    bitrate;                        /* audio bitrate in Kbps, or 0 if unknown */
} BIP_AudioTrackInfo;

typedef struct BIP_StreamInfo
{
    bool  valid;                              /* if true, only then remaining fields contain valid PSI information */
    /* TODO: see if PCRs are specific to each program */
    int     pcrPid;
    int     firstPmtPid;
    NEXUS_TransportType         mpegType;
    bool                        transportTimeStampEnabled;             /* indicates if MPEG2 TS content contains 4 byte DLNA timpstamp */
    unsigned                    numVideoTracks;
    BIP_VideoTrackInfo          firstVideoTrackInfo;
    unsigned                    numAudioTracks;
    BIP_AudioTrackInfo          firstAudioTrackInfo;
    off64_t                     contentLength;      /* 0 if not known */
    unsigned                    duration;           /* duration of stream in milliseconds or 0 if unknown */
    unsigned                    avgBitRate;         /* average stream bitrate in bits/second or 0 if unknown */
    NEXUS_PlaybackPosition      maxBufferDuration;  /* how much of content can be buffered in HTTP Cache, in milli-sec (based on max bitrate & HTTP cache size */
    BIP_ClientSessionProtocol   bipSessionProtocol; /* app can hint session protocol: eRvu, eHls, eDash, etc. */
    unsigned                    numPlaySpeedEntries;
#define IP_PVR_PLAYSPEED_MAX_COUNT  (128)
    int  playSpeed[IP_PVR_PLAYSPEED_MAX_COUNT];
} BIP_StreamInfo;
typedef struct BIP_StreamInfo  *BIP_StreamInfoHandle;

/* TODO: */

typedef struct BIP_ClientSetupStatus
{
    /* Can be a BIP_SocketHandle */
    int     socketFd;                               /* socket descriptor being used by the IP Applib to receive data */
    int     httpStatusCode;                         /* Returned status code for the Request */

    char                    *responseHeaders;               /* pointer to response headers string if copyResponseHeaders flag is set in open settings */
    BIP_StreamInfo          streamInfo;                     /* PSI info for the media: valid only if psi.psiValid field is true */

    /* TODO: not sure if app needs to know about playpump & playback */
    bool    useNexusPlaypump;
    bool    useNexusPlayback;

    BIP_ClientClockRecoveryMode     clockRecoveryMode;
    BIP_ClientAppHeader             appHeader;              /* appSpecific binary header sent by the server */
} BIP_ClientSetupStatus;

/**
Summary:
Get default settings for setting up the BIP Client session

Description:
TODO: define BIP_ClientSetupSettings structure
**/
void  BIP_Client_GetDefaultSetupSettings(
    BIP_ClientSetupSettings *pSettings /* [out] */
    );

/**
Summary:
Setup BIP Client session related state for a given URL.

Description:
Establishes session with the server, allows apps to process any server responses via callback
and then optionally carries out the media probe functionality on the stream

Notes:
equivalent of the two APIs: BIP_ClientSessionOpen() & BIP_ClientSessionSetup()
typically invoked part of the play() api of the media player app
apps may fill in both PSI & security options in the callback based on what it finds via server response

Questions:
if we want to do the PSI discovery on live channels using PSIP library, then we will need playpump handles at this stage
this is currently done in the app. Other option is to do this via media probe as well and see if that suffices
**/
BIP_Status  BIP_Client_Setup(
    BIP_ClientHandle                hBipClient,
    const BIP_ClientSetupSettings   *pSettings,
    BIP_ClientSetupStatus           *setupStatus /* [out]*/
    );

typedef struct BIP_ClientPidChannelSettings
{
    /* just a place holder for now */
    unsigned  unused;
} BIP_ClientPidChannelSettings;

/**
Summary:
Get default settings for the pid channel structure.

Description:
TODO: define BIP_ClientPidChannelSettings
**/
void  BIP_Client_GetDefaultPidChannelSettings(
    BIP_ClientPidChannelSettings *pSettings /* [out] */
    );

/**
Summary:
Opens a pid channel using either Nexus Playback or Playpump (depending upon the usage whether Playpump or Playback will be used)

Description:
BIP_Client_OpenPidChannel should be called after BIP_Client_Setup but before BIP_Client_Start.
Some IP Session Protocols may change the PidChannels (MPEG DASH & HLS may have pids change while switching to alternate bitrate streams or during marked discontinuities (ad-insertion)
**/
NEXUS_PidChannelHandle  BIP_Client_OpenPidChannel(
    BIP_ClientHandle                    hBipClient,
    unsigned                            pid,
    const BIP_ClientPidChannelSettings  *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
BIP_Client_GetPidChannelSettings is used to get current configuration for the pidChannel opened with BIP_Client_OpenPidChannel

Description:
This function can fail if the pidChannel does not belong to this BIP client.
**/
BIP_Status  BIP_Client_GetPidChannelSettings(
    BIP_ClientHandle                hBipClient,
    NEXUS_PidChannelHandle          pidChannel,
    BIP_ClientPidChannelSettings    *pSettings /* [out] */
    );

/**
Summary:
BIP_Client_SetPidChannelSettings is used to change current configuration for the pidChannel opened with BIP_Client_OpenPidChannel

Description:
Main purpose of the BIP_Client_SetPidChannelSettings is to change decoder assigment.
**/
BIP_Status  BIP_Client_SetPidChannelSettings(
    BIP_ClientHandle                    hBipClient,
    NEXUS_PidChannelHandle              pidChannel,
    const BIP_ClientPidChannelSettings  *pSettings
    );

/**
Summary:
Closes a pid channel opened by BIP_Client_OpenPidChannel
**/
void  BIP_Client_ClosePidChannel(
    BIP_ClientHandle        hBipClient,
    NEXUS_PidChannelHandle  pidChannel
    );

/**
Summary:
Close all pid channels opened by BIP_Client_OpenPidChannel
**/
void  BIP_Client_CloseAllPidChannels(
    BIP_ClientHandle hBipClient
    );

/* Settings that can be modified prior to start or during runtime */
typedef struct BIP_ClientSettings
{
    /* just a place holder for now */
    NEXUS_TransportType             transportType;      /* app can hint the stream transport type if it is known via DLNA means or fixed for a channel, default is NEXUS_TransportType_eUnknown */
    BIP_ClientClockRecoveryMode     clockRecoveryMode;  /* Override the clock recovery mode that BIP would otherwise use based on protocol & live flag */
    BIP_ClientTtsParams             ttsParams;          /* TTS related parameters */
    int  maxIpNetworkJitter;                            /* Maximum network jitter in millisec (defaults to 300msec): used in clock-recovery setup & in determining packet loss condition */

    /* ----------- Optional ClientSetup Configuration ----------- */
    NEXUS_PlaypumpSettings  playpumpSettings;   /* allows app to set any specific playpump Settings, BIP_ClientGetSetupDefault settings provides the current defaults */
    NEXUS_PlaybackSettings  playbackSettings;   /* allows app to set any specific playback Settings, BIP_ClientGetSetupDefault settings provides the current defaults */
    bool        enableEndOfStreamLooping;       /* if set, ip library will loop around after server sends EOF */
    unsigned    readTimeout;                    /* in msec: for live channels, app should set readTimeout value such that ip lib returns whatever it has returned in this duration */
                                                /* this helps with lowering the read related latency by allowing apps to read independent of stream bitrate (HD/SD/Audio only channel types) */
                                                /* apps (or playback_ip internally) can always ask for say 64KB/96KB worth of data from the socket and returns in this timeout duration for low bitrate cases */
    bool  dontFeedDataToPlaybackHw;             /* app doesn't want playback_ip to feed data to playback h/w, instead it will read data from playback ip using setupStatus.file i/f and itself feed to h/w */
    BIP_ClientLpcmSettings  lpcmSettings;

    /* TODO: security related config */

    /* settings that can be modified during runtime */
} BIP_ClientSettings;
/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void  BIP_Client_GetDefaultSettings(
    BIP_ClientSettings *pSettings /* [out] */
    );

/**
Summary:
Returns current configuration of BIP Client session
**/
void  BIP_Client_GetSettings(
    BIP_ClientHandle    hBipClient,
    BIP_ClientSettings  *pSettings /* [out] */
    );

/**
Summary:
Changes configuration of BIP Client session.

Description:
Some configuration can not be changed unless session is stopped and all pids are closed.
**/

BIP_Status  BIP_Client_SetSettings(
    BIP_ClientHandle            hBipClient,
    const BIP_ClientSettings    *pSettings
    );

typedef struct BIP_ClientStartSettings
{
    unsigned  seekOffsetTime;        /* initial time offset in milliseconds */
} BIP_ClientStartSettings;
/*
Summary:
Get default settings
*/
void  BIP_Client_GetDefaultStartSettings(
    BIP_ClientStartSettings *pSettings /* [out] */
    );

/*
Summary:
Start BIP Client session.

Description:
If BIP_ClientSettings.startPaused is true, data flow through the nexus media framework and into transport hardware does not begin
until BIP_Client_Play is called.

*/
BIP_Status  BIP_Client_Start(
    BIP_ClientHandle                hBipClient,
    const BIP_ClientStartSettings   *pSettings
    );

/*
Summary:
Stop BIP Client session.
*/
void  BIP_Client_Stop(
    BIP_ClientHandle hBipClient
    );

typedef struct BIP_ClientStatus
{
    NEXUS_PlaybackPosition  currentPosition;    /* current timestamp of stream played in mSec */
    off_t  totalConsumed;                       /* total bytes consumed by the Media player */
} BIP_ClientStatus;

/*
Summary:
Get current status
*/
BIP_Status  BIP_Client_GetStatus(
    BIP_ClientHandle    hBipClient,
    BIP_ClientStatus    *pStatus /* [out] */
    );

#if 0
/* TODO: add signature for trickmode APIs */
BIP_Client_Play() :
    BIP_Client_Pause() :
        BIP_Client_Seek() :
            BIP_Client_Trickmode() :
                BIP_Client_GetTrickModeSettings();
BIP_Client_GetStatus();
#endif /* if 0 */

#ifdef __cplusplus
}
#endif
#endif /* #ifndef BIP_CLIENT_H */
