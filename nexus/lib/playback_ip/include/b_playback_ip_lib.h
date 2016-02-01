/***************************************************************************
*     (c)2008-2016 Broadcom Corporation
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: header file for Playback IP App Lib
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef B_PLAYBACK_IP_LIB_H__
#define B_PLAYBACK_IP_LIB_H__

#include "bstd.h"
#include "nexus_types.h"
#include "nexus_file_fifo.h"
#ifndef DMS_CROSS_PLATFORMS
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_video_decoder_types.h"
#include "nexus_recpump.h"
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
#include "nexus_dma.h"
#endif
#ifdef NEXUS_HAS_SECURITY
#include "nexus_security.h"
#endif

#endif /* DMS_CROSS_PLATFORMS */
#ifndef _WIN32_WCE

#ifdef __vxworks
  #include <sysLib.h>
  #include <string.h>
  #include <stdlib.h>
  #include <logLib.h>
  #include <sys/times.h>
  #include <selectLib.h>
  #include <taskLib.h>
  #include <semLib.h>
  #include <sys/ioctl.h>
  #include <hostLib.h>
#else
  #include <memory.h>
  #include <pthread.h>
  #include <sys/time.h>
#endif

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#else /* _WIN32_WCE */
#include <windows.h>
    typedef HANDLE pthread_t;
    #define close(x) closesocket(x)
    unsigned long errno = 0;
    #define EINTR -1
#endif /* _WIN32_WCE */

/**************************************************************************
 * This IP Applib module provides IP STB playback functions including:
 *
 *     - opening/closing of IP sockets
 *     - playback thread create/destroy (which reads incoming data from socket & feeds to Playpump)
 *
 ***************************************************************************/

#ifdef __cplusplus

#ifdef LIVEMEDIA_SUPPORT
#include "RTSPClient.hh"
char * sendOptionsCmd ( RTSPClient* rtspClient );
char * sendDescribeCmd ( RTSPClient* rtspClient, const char * url );
int sendSetupCmd ( RTSPClient* rtspClient, MediaSubsession& subsession,
                   Boolean streamOutgoing, Boolean streamUsingTCP, Boolean forceMulticastOnUnspecified,
                   Authenticator* authenticator);
int sendPlayCmd ( RTSPClient* rtspClient, MediaSession& session,
                  double start, double end, float scale, Authenticator* authenticator );
int sendPauseCmd ( RTSPClient* rtspClient, MediaSession& session );
bool sendTeardownCmd ( RTSPClient* rtspClient, MediaSession& session );
bool sendSetParameterCmd ( RTSPClient* rtspClient, MediaSession* rtspMediaSession,
                           const char * parameter,  const char * value, Authenticator* authenticator );
bool sendGetParameterCmd ( RTSPClient* rtspClient, MediaSession* rtspMediaSession,
                           const char * parameterName, char*& parameterValue );
#endif /* LIVEMEDIA_SUPPORT */

extern "C" {
#endif

/***************************************************************************
Summary:
Generic Error Codes

Description:
These error codes will match those returned by nexus (NEXUS_Error) and
magnum (BERR_Code).  These may be used throughout application libraries
for consistency. Playback IP Applib specific Error codes follow the standard ones.
***************************************************************************/
typedef unsigned B_PlaybackIpError;
#ifdef DMS_CROSS_PLATFORMS
typedef unsigned int NEXUS_PlaybackPosition;
#endif

/**
Summary:
Standard Nexus error codes.
**/
#define B_ERROR_SUCCESS              0  /* success (always zero) */
#define B_ERROR_NOT_INITIALIZED      1  /* parameter not initialized */
#define B_ERROR_INVALID_PARAMETER    2  /* parameter is invalid */
#define B_ERROR_OUT_OF_MEMORY        3  /* out of heap memory */
#define B_ERROR_TIMEOUT              5  /* reached timeout limit */
#define B_ERROR_OS_ERROR             6  /* generic OS error */
#define B_ERROR_LEAKED_RESOURCE      7  /* resource being freed has attached resources that haven't been freed */
#define B_ERROR_NOT_SUPPORTED        8  /* requested feature is not supported */
#define B_ERROR_UNKNOWN              9  /* unknown */

/**
Playback IP Applib specific Error codes.
**/
#define B_ERROR_PROTO                10  /* protocol Error */
#define B_ERROR_SOCKET_ERROR         11  /* socket operation failed */
#define B_ERROR_IN_PROGRESS          12  /* Requested API is in progress, completion notified via callback */
#define B_ERROR_PSI_PROBE_FAILED     13  /* Media probe failed to obtain PSI info on stream */
#define B_ERROR_CHANNEL_CHANGE       14  /* API returning due to channel change where current channel is being stopped. */

/*
IP Library Client Side APIs:

B_PlaybackIp_Open();

B_PlaybackIp_SessionOpen();

B_PlaybackIp_SendRtspMethod();

B_PlaybackIp_SessionSetup();

B_PlaybackIp_SessionStart();

B_PlaybackIp_SessionStop();

B_PlaybackIp_SessionClose();

B_PlaybackIp_Close();

B_PlaybackIp_EventCallback():

B_PlaybackIp_Play():

B_PlaybackIp_Pause():

B_PlaybackIp_Seek():

B_PlaybackIp_Trickmode():

B_PlaybackIp_GetTrickModeSettings();

B_PlaybackIp_GetStatus();

B_PlaybackIp_GetSettings();

B_PlaybackIp_SetSettings();

B_PlaybackIp_GetDefaultSettings();

B_PlaybackIp_NetworkRecordingStart();

B_PlaybackIp_NetworkRecordingStop();

B_PlaybackIp_DetectTts();

Security related APIs

 */

/**
Summary:
Public handle for Playback IP App Lib
**/
typedef struct B_PlaybackIp * B_PlaybackIpHandle;

/**
Summary:
Playback IP App Lib open settings structure
**/
typedef struct B_PlaybackIpOpenSettings
{
    int unused;
} B_PlaybackIpOpenSettings;

/**
Summary:
This function initializes the App Lib based on the settings selected.
**/
B_PlaybackIpHandle B_PlaybackIp_Open(
    const B_PlaybackIpOpenSettings *pSettings
    );

/**
Summary:
This function de-initializes the App Lib. The private App Lib structure is freed.
**/
B_PlaybackIpError B_PlaybackIp_Close(
    B_PlaybackIpHandle playback_ip
    );

/**
Summary:
Enums defining supported transport protocols
**/
typedef enum B_PlaybackIpProtocol
{
    B_PlaybackIpProtocol_eUdp,          /* Plain UDP */
    B_PlaybackIpProtocol_eRtp,          /* RTP (requires LIVEMEDIA_SUPPORT compile flag) */
    B_PlaybackIpProtocol_eRtsp,         /* RTSP (requires LIVEMEDIA_SUPPORT compile flag) */
    B_PlaybackIpProtocol_eHttp,         /* HTTP over TCP */
    B_PlaybackIpProtocol_eRtpNoRtcp,    /* RTP (w/o LIVEMEDIA_SUPPORT), no RTCP supported */
    B_PlaybackIpProtocol_eRtpFec,       /* RTP Streaming with FEC: not yet supported */
    B_PlaybackIpProtocol_eHttps,        /* HTTP w/ TLS/SSL */
    B_PlaybackIpProtocol_eMax
} B_PlaybackIpProtocol;

/**
Summary:
Enums defining supported security protocols
**/
typedef enum B_PlaybackIpSecurityProtocol
{
    B_PlaybackIpSecurityProtocol_None,    /* Clear Content */
    B_PlaybackIpSecurityProtocol_Ssl,     /* SSL/TLS protected Content */
    B_PlaybackIpSecurityProtocol_DtcpIp,  /* DTCP-IP protected Content */
    B_PlaybackIpSecurityProtocol_RadEa,   /* Rhapsody's RAD-EA protected Content */
    B_PlaybackIpSecurityProtocol_Aes128,  /* Generic AES protocol: used in HLS protected content */
    B_PlaybackIpSecurityProtocol_Max      /* Max value */
} B_PlaybackIpSecurityProtocol;

/**
Summary:
Configurable options for security protocols
**/
typedef struct B_PlaybackIpSecurityOpenSettings
{
    B_PlaybackIpSecurityProtocol securityProtocol;  /* security protocol being used */
    void *initialSecurityContext;         /* optional security protocol specific context: used for passing-in initial DTCP context */
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA)
    NEXUS_DmaHandle dmaHandle;                      /* optional dma handle: used to PVR decryption before streaming out */
#endif
    bool enableDecryption;                /* decrypt n/w data only when this flag is set */
                                          /* needed for some security protocols (RAD/EA, DTCP/IP) where some initial data like HTTP Response is not encrypted */
    bool enableEncryption;                /* encrypt outgoing streamed data only when this flag is set */
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    NEXUS_KeySlotHandle pvrDecKeyHandle;  /* set if PVR Decryption needs to be performed before streaming out the live stream */
#endif
    union {
        struct {
            int voidVar;                  /* just a place holder as we dont need any settings for clear channels */
        } clear;
#ifdef B_HAS_SSL
        struct {
            int dummySsl;                 /* placeholder to avoid compilation warnings */
        } ssl;
#endif
        struct {
            char *dstIpAddress;
            int emiValue;
            unsigned akeTimeoutInMs;      /* Timeout to wait in msec upto which AKE handshake should be completed with the client, otherwise, API would fail. */
            bool exchangeKeyLabelValid;   /* Set to true to indicate exchangeKeyLabel field (below) is valid */
            unsigned exchangeKeyLabel;    /* If exchangeKeyLabelValid is set to true, DTCP/IP library will use ExchangeKey with this label value to derive Content key used for encryption. */
            unsigned pcpPayloadLengthInBytes; /* How many bytes of encrypted content should be contained within each PCP. */
                                              /* DTCP/IP Spec allows max payload length of upto 128MB [DTCP/IP Spec V1SE 4.26.1]. */
                                              /* 0 value indicates that PCP header length is determined internally by the library. */
        } dtcpIp;
#ifdef B_HAS_RAD_EA
        struct {
            char *eaUrl;                  /* URL for EA session */
            char *trackId;
            char *logon;
            char *playbackSessionId;
            char *cobrandId;
            int radversion;
            char *format;
            int bitrate;
            char *developerKey;
            int startOffset;
            bool radioChannel;
            char *channelId;
            int position;
            int count;
        } radEa;
#endif
#ifdef B_HAS_HTTP_AES_SUPPORT
        struct {
            unsigned char key[16]; /* 128 bit key for AES-128 */
            unsigned char iv[16]; /* 128 bit IV for AES-128 */
        } aes128;
#endif
    } settings;
} B_PlaybackIpSecurityOpenSettings;

/**
Summary:
This structure used to communicate PSI information back to the caller.

Description:
IP Applib internally uses the Media Probe library to determine the PSI information of the content.
In addition, incase content is being played from a server using Broadcom's httpstreamer app,
(which supports server based trick modes), additional trick mode related info is also returned.
In DLNA case, this information may be already known to the app when it parsed the Protocol Info
fields of the DLNA Resources.
**/
typedef struct B_PlaybackIpPsiInfo
{
    bool psiValid;                              /* if true, only then remaining fields contain valid PSI information */
    NEXUS_VideoCodec videoCodec;                /* Rest of the fields are the standard PSI information */
    NEXUS_VideoCodec extraVideoCodec;           /* AVC Extension for SVC/MVC: extra video codec */
    NEXUS_AudioCodec audioCodec;
    int videoPid;
    int extraVideoPid;                          /* AVC Extension for SVC/MVC: extra video pid */
    /* TODO: convert audioPid into list to allow storing multiple audio pids */
    int audioPid;
    const char *mainAudioLanguage;              /* 3 ASCII codes (+1 for NULL char) for audio language code associated w/ this audio. */
    int pcrPid;
    int pmtPid;
    NEXUS_TransportType mpegType;
    bool transportTimeStampEnabled;             /* indicates if MPEG2 TS content contains 4 byte DLNA timpstamp */
    bool liveChannel;                           /* if set, this PSI info pertains to a live channel */
    unsigned duration;                          /* duration of stream in milliseconds or 0 if unknown */
    unsigned avgBitRate;                        /* average stream bitrate in bits/second or 0 if unknown */
    NEXUS_PlaybackPosition maxBufferDuration;   /* how much of content can be buffered in HTTP Cache, in milli-sec (based on max bitrate & HTTP cache size */
    /* these are server based trick modes related info: proprietary info provided by the Broadcom's ip streamer app */
    uint32_t firstPts;
    unsigned numPlaySpeedEntries;
    int httpFrameRepeat;                        /* frame repeat count: computed & sent by the HTTP Streamer server */
    int httpMinIFrameSpeed;                     /* Server only sends I Frames at this or higher speed */
    int frameRateInTrickMode;                   /* indicates to the client the frameRate that it should set in the trickmodes in order to achieve a given play speed */
#define IP_PVR_PLAYSPEED_MAX_COUNT  (128)
    int playSpeed[IP_PVR_PLAYSPEED_MAX_COUNT];
    char *playSpeedString;                      /* provides the playSpeed in the string format to allow receiving fractional speeds */
    off64_t contentLength;                        /* 0 if not known */

    /* detailed vidoe info */
    float videoFrameRate;                       /* video frame rate in fps, or 0 if unknown */
    uint16_t videoWidth;                        /* coded video width, or 0 if unknown */
    uint16_t videoHeight;                       /* coded video height, or 0 if unknown  */
    unsigned videoBitrate;                      /* video bitrate in Kbps, or 0 if unknown */

    /* detailed audio info */
    unsigned audioBitsPerSample;                /* number of bits in the each sample, or 0 if unknown */
    unsigned audioSampleRate;                   /* audio sampling rate in Hz, or 0 if unknown */
    unsigned audioNumChannels;                  /* number of channels, or 0 if unknown  */
    unsigned audioBitrate;                      /* audio bitrate in Kbps, or 0 if unknown */
    bool defaultAudioIsMuxedWithVideo;
    off64_t beginFileOffset;
    off64_t endFileOffset;
#ifdef STREAMER_CABLECARD_SUPPORT
    uint8_t pmtBuffer[4096];
    uint32_t pmtBufferSize;
#endif
    bool hlsSessionEnabled;                     /* set if current session is being receiving using HTTP Live Streaming (HLS) Protocol */
    bool mpegDashSessionEnabled;                /* set if current session is being receiving using MPEG-DASH Protocol */
#define AUDIO_PID_MAX_COUNT  (16)
    int extraAudioPid[AUDIO_PID_MAX_COUNT];
    NEXUS_AudioCodec extraAudioCodec[AUDIO_PID_MAX_COUNT];
    int extraAudioPidsCount;
    bool usePlaypump2ForAudio;                  /* true => open audio pidChannel on nexusHandles.playpump2 */
    unsigned colorDepth;                        /* For H265/HEVC video code, color depth: 8 --> 8 bits, 10 --> 10 bits, 0 for other Video Codecs. */
    unsigned numPrograms;                       /* total number of Programs through out stream */
    unsigned numTracks;                         /* total number of tracks through out stream */
    unsigned hlsAltAudioRenditionCount;
    struct {
        bool defaultAudio;                      /* If set, this entry should be the default Audio rendition to use if Player doesn't have a preference. */
        NEXUS_AudioCodec codec;                 /* audio codec of the alternate rendition entry. */
        int pid;                                /* pid associated with the alternate rendition entry. */
        const char *language;                   /* const pointer to the 3 byte ASCII codes for audio language code associated w/ this audio entry. */
        const char *languageName;               /* const pointer to the language description. */
        bool requiresPlaypump2;                 /* if true, then app must open & start a 2nd Nexus Playpump, open audio pidChannel & set nexusHandles.playpump2. */
        NEXUS_TransportType containerType;
        const char *groupId;                    /* const pointer to the associated groupId. */
    } hlsAltAudioRenditionInfo[AUDIO_PID_MAX_COUNT];
} B_PlaybackIpPsiInfo;
typedef struct B_PlaybackIpPsiInfo *B_PlaybackIpPsiInfoHandle;

/**
Summary:
This strcuture is used to communicate the buffering scheme employed by the IP STB
B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip:
    AV decoders lock to PCRs,
    CDBs are used as dejitter buffer,
    AV decodes are delayed to absorb the jitter
    all outputs are free run due to high jitter
    Free running outputs cause Sync-Slip & Video frames & Audio samples are repeated or dropped
B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip
    Requires sender to include 4 byte Timestamps in addition to 188 byte TS packets (DLNA TTS profile)
**/
typedef enum B_PlaybackIpClockRecoveryMode
{
    B_PlaybackIpClockRecoveryMode_ePull,                  /* Pull mode: clock is initialied to 1st PTS and then freely run */
    B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip,   /* Content pushed by Server using PCR */
    B_PlaybackIpClockRecoveryMode_ePushWithTtsNoSyncSlip, /* SW Buffering & Content Paced by local Playback Hardware using TTS Timestamps */
    B_PlaybackIpClockRecoveryMode_ePushWithPcrNoSyncSlip  /* SW Buffering & Content Paced by local Playback Hardware using PCRs as Timestamps */
} B_PlaybackIpClockRecoveryMode;

/**
Summary:
Handles to various Nexus modules

Description:
App is required to configure all the relevant Nexus Modules. IP Applib only needs these handles for control purposes.
E.g. in HTTP/RTSP case, one usage is to configure the AV decoders & Plaback/Playpump during trickplay transitions.
In RTP/UDP case, it uses these handles to flush the AV pipeline when there is a discontinuity due to network timeouts, etc.
**/
#ifndef DMS_CROSS_PLATFORMS
typedef struct B_PlaybackIpNexusHandles
{
    NEXUS_PlaypumpHandle playpump;          /* required for Live RTP/UDP/RTSP/Server Trickmodes based sessions */
    NEXUS_PlaypumpHandle playpump2;     /* required for MPEG-DASH sessions */
    NEXUS_PlaybackHandle playback;          /* required for HTTP based sessions */
    NEXUS_VideoDecoderHandle videoDecoder;  /* required if video decode is enabled else must be NULL */
    NEXUS_SimpleVideoDecoderHandle simpleVideoDecoder;  /* alternative solution for when video decoder is not available */
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder;  /* alternative solution for when audio decoder is not available */
    NEXUS_AudioDecoderHandle primaryAudioDecoder;   /* required if audio decode is enabled else must be NULL */
    NEXUS_AudioDecoderHandle secondaryAudioDecoder; /* required if audio decode is enabled else must be NULL */
    NEXUS_StcChannelHandle stcChannel;      /* required for video & audio enabled streams */
    NEXUS_SimpleStcChannelHandle simpleStcChannel;/* alternative to stcChannel handle for multiProcess nexus mode */

    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle extraVideoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;

    NEXUS_VideoDecoderStartSettings         videoStartSettings;
    NEXUS_AudioDecoderStartSettings         audioStartSettings;

    NEXUS_SimpleAudioDecoderStartSettings   simpleAudioStartSettings;
    NEXUS_SimpleVideoDecoderStartSettings   simpleVideoStartSettings;
} B_PlaybackIpNexusHandles;
#endif /* DMS_CROSS_PLATFORMS */
/**
Summary:
Various callback events
Note: Some of these events are generated in response to the IP Applib APIs invoked in the non-blocking mode.
Events only indicate that API is now completed. App still need to reinvoke the api corresponding to the event
for completion status. Only B_ERROR_SUCCESS return code indicates valid values in the returned Status structures.

Also, there are other events which are asynchronously generated and are not associated w/ any particular API.
App can invoke the GetStatus() API to retrieve the detailed event specific status.
**/
typedef enum
{
    /* Event to indicate SessionOpen is completed: socket, bind, connect and protocol specific messages are exchanged */
    B_PlaybackIpEvent_eSessionOpenDone,
    /* Event to indicate SessionSetup is completed: for HTTP Media probe is completed */
    /* For RTSP basic commands OPTIONS,DESCRIBE and SETUP are sent and response is processed */
    B_PlaybackIpEvent_eSessionSetupDone,
    /* Event to indicate SessionStart is completed (RTSP Play is completed) */
    B_PlaybackIpEvent_eSessionStartDone,
    /* Event to indicate completion of request to playback media at Normal 1x Speed */
    B_PlaybackIpEvent_ePlaying,
    /* Event to indicate completion of request to pause the current media playback */
    B_PlaybackIpEvent_ePaused,
    /* Event to indicate completion of request to pause the current media playback */
    B_PlaybackIpEvent_eStopped,
    /* Event to indicate completion of request to seek media playback to new position */
    B_PlaybackIpEvent_eSeekComplete,
    /* Event to indicate a forward (FFWD) or backward (FREW) scan is initiated */
    B_PlaybackIpEvent_eScanStarted,
#if 0
    /* TODO: need to look into if we can support these events */
    /*
    /// This event meant for enabling application to trigger for a state transition if required to reset the
    /// state to maintain synch with server. Taking as example, if server doesn't get any response within
    /// specified time out time, it closes the connection with particular client sending an ANNOUNCE message.
    /// Now if client needs to get streaming again from server, new session need to be achieved. So need to
    /// reset the current state ie closing the current active session,(show any UI or msg to user) and take
    /// decision according to user.
    /// Response to Server notification, notified to client via ANNOUNCEMENT protocol command.
    /// Server keeps track of client responses, as time out configured , and if client doesnt respond within the
    /// Timeout period, automatically closes the session with client, sending ANNOUNCE command.
    */
    B_PlaybackIpEvent_eServerClientSessionTimeOut,
    /*
    /// This event meant for enabling application to trigger for a state transition if required to reset the
    /// state to maintain synch with server.
    /// Response to Server notification, notified to client via ANNOUNCEMENT protocol command.
    */
    B_PlaybackIpEvent_eServerItemStepped,
    /*
    /// This event meant for enabling application to trigger for a state transition if required to reset the
    /// state to maintain synch with server.
    /// Response to Server notification, notified to client via ANNOUNCEMENT protocol command.
    */
    B_PlaybackIpEvent_eServerCurrentItemRemoved,
#endif
    /* TODO: may need to add a thread to keep track of one way messages from the server */
    /*
    /// This event meant for enabling application to trigger for a state transition if required to reset the
    /// state to maintain synch with server.
    /// Response to Server notification, notified to client via ANNOUNCEMENT protocol command.
    /// Server keeps track of current track and as and when it reaches to end of file, it will
    /// notify to client via ANNOUNCEMENT protocol command.
    */
    B_PlaybackIpEvent_eServerEndofStreamReached,
    /* Event to indicate error during stream playback */
    B_PlaybackIpEvent_eErrorDuringStreamPlayback,
    /* Event to indicate buffer precharge is completed */
    B_PlaybackIpEvent_ePreChargeDone,
    /* Event to indicate IP tuner lock: when 1st packet is received in beginning or after a network timeout exceeding the defined network jitter for a live UDP/RTP/RTSP/HTTP channel */
    B_PlaybackIpEvent_eIpTunerLocked,
    /* Event to indicate IP tuner unlock: when no packet is received for network timeout exceeding the defined network jitter for a live UDP/RTP/RTSP/HTTP channel */
    B_PlaybackIpEvent_eIpTunerUnLocked,
    /* Event to indicate when streaming thread has sent one full segment and is waiting for app to resume streaming of next segment */
    B_PlaybackIpEvent_eServerEndofSegmentReached,
    /* Event to indicate when streaming thread timesout waiting for app to start or resume streaming of next segment */
    B_PlaybackIpEvent_eServerStartStreamingTimedout,
    /* Event to indicate error during streaming loop: such as failed to write to client */
    B_PlaybackIpEvent_eServerErrorStreaming,
    /* Begin Of Stream event.*/
    B_PlaybackIpEvent_eClientBeginOfStream,
    /* Event to indicate when client playback thread has doesn't have anymore segments to play and is waiting for app to either resume via seeking little back or stop the playback */
    /* This can happen for live Event type hls channels when app does a fast fwd and we reach the end of currently available segments but not end of stream as it is a live event. */
    B_PlaybackIpEvent_eClientEndofSegmentsReached,
    /* Invalid event */
    B_PlaybackIpEvent_eMax
} B_PlaybackIpEventIds;

/**
Summary:
Callback function to indicate either non-blocking API completion or some asynchronous event
**/
typedef void (*B_PlaybackIp_EventCallback)(void * appCtx, B_PlaybackIpEventIds eventId);

/**
Summary:
This structure returns the actual socket related state back to the application.
**/
typedef struct B_PlaybackIpSocketState
{
    int fd;                             /* socket descriptor being used by the IP Applib to receive data */
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    struct hostent *he;
    socklen_t addr_len;
} B_PlaybackIpSocketState;
typedef struct B_PlaybackIpSocketState *B_PlaybackIpSocketStateHandle;

/**
Summary:
Socket Open Settings
**/
typedef struct B_PlaybackIpSocketOpenSettings
{
    char ipAddr[256];                   /* IP Address (in dotted decimal or char name) string */
    unsigned port;                      /* port number of IP Channel (in host byte order) */
    char *interfaceName;                /* interface name to send multicast joins requests on (for receiving live UDP/RTP sessions) */
    B_PlaybackIpProtocol protocol;      /* protocol: UDP, RTP, HTTP, RTSP */
    char *url;                          /* pointer to URL for HTTP or RTSP protocols: conveys content URL */
    bool useProxy;                      /* if set, ipAddr and port corresponds to the proxy server address:port and url contains the absolute URI */
} B_PlaybackIpSocketOpenSettings;
typedef struct B_PlaybackIpSocketOpenSettings *B_PlaybackIpSocketOpenSettingsHandle;

/**
Summary:
HTTP Session Open Settings
**/
typedef struct B_PlaybackIpHttpSessionOpenSettings
{
    unsigned networkBufferSize;         /* size of network cache, in bytes, where data is buffered (currently used for HTTP based Pull Modes) */
                                        /* 0: IP Applib uses a default size of couple of MB */
    char *additionalHeaders;            /* additional HTTP headers that app wants to include in the outgoing Get Request: need to be \r\n terminated */
    char *userAgent;                    /* app can override the default UserAgent string used in the outgoing HTTP Get Request: string only */
    bool copyResponseHeaders;           /* if true, a copy of the HTTP response Headers is provided back to app */
                                        /* this enables app to process any custom app specific Headers (getProtocolInfo header for obtain protocol info) */
    bool rvuCompliant;                  /* if set, this session should use RVU Compliant headers & Protocol */
    bool useHeadRequestMethod;          /* apps can set this to ask ip lib to use HTTP HEAD method instead of GET for outgoing requests */
} B_PlaybackIpHttpSessionOpenSettings;

/**
Summary:
RTSP Session Open Settings
**/
typedef struct B_PlaybackIpRtspSessionOpenSettings
{
    char *additionalHeaders;            /* additional headers that app wants to include in the outgoing RTSP OPTION Request */
    char *userAgent;                    /* app can override the default UserAgent string used in the outgoing OPTION Request*/
    bool copyResponseHeaders;           /* if true, a copy of the OPTION response Headers is provided back to app */
                                        /* this enables app to process & retrieve any custom app specific Headers (ServiceGroup, Version, AppData, etc.) */
} B_PlaybackIpRtspSessionOpenSettings;

/**
Summary:
Aggregate IP Session Open Settings
**/
typedef struct B_PlaybackIpSessionOpenSettings
{
    B_PlaybackIpSocketOpenSettings socketOpenSettings;  /* socket related settings */
    B_PlaybackIpSecurityOpenSettings security;          /* Security Open Settings */
    B_PlaybackIpClockRecoveryMode ipMode;               /* clock recovery mode */
                                                        /* if App apriori knows the PSI info, then it has enough info to decide which clock recovery mode to use */
                                                        /* ip_client.c has an example function useLiveModeForIpPlayback() to aid apps in determining this flag */
    int maxNetworkJitter;                               /* Maximum network jitter in millisec: used to determining packet loss conditions */
                                                        /* this is considered as unmarked discontinuity and AV pipeline is flushed at this point */
    bool useNexusPlaypump;                              /* true: tells IP Applib to feed network stream to the Nexus Playpump, False: tells IP Applib that Nexus Playback will pull stream from IP Applib */
                                                        /* if App apriori knows the PSI info, then it has enough info to decide weather to use Nexus Playpump or Playback */
                                                        /* ip_client.c has an example function useNexusPlaypumpForIpPlayback() to aid apps in determining this. */
    unsigned networkTimeout;                            /* provides the timeout value, in seconds, for network operations like select, read. */
                                                        /* 0: IP Applib uses a default of 30 sec */
    bool nonBlockingMode;                               /* if set, requires apps to also set following eventCallback function field */
                                                        /* if IP lib can't immediately complete an IP library API, it will return B_ERROR_IN_PROGRESS. */
                                                        /* The API is asynchronously carried out in the IP library thread context */
                                                        /* and app is notified of its completion via eventCallback. App is then required to *re-issue* the */
                                                        /* same IP library API to read the results (success/failure). */
    B_PlaybackIp_EventCallback eventCallback;           /* callback function if nonBlockingMode is desired or to receive any asynchronous events from the IP library */
    void *appCtx;                                       /* app context handle used in the callback function */
    union {
        B_PlaybackIpHttpSessionOpenSettings http;
        B_PlaybackIpRtspSessionOpenSettings rtsp;
    }u;
    NEXUS_HeapHandle heapHandle;                        /* nexus heap Handle, can be null if app doesn't have a preference */
} B_PlaybackIpSessionOpenSettings;

/* API to get default SessionOpen Settings */
B_PlaybackIpError B_PlaybackIp_GetDefaultSessionOpenSettings(
    B_PlaybackIpSessionOpenSettings *openSettings
    );

/**
Summary:
HTTP & RTSP specific Session Open Status (all output parameters should be included in this structure)
TODO: May need to separate the HTTP & RTSP status, so far it seems to have common fields.
**/
typedef struct B_PlaybackIpHttpSessionOpenStatus
{
    int statusCode;                     /* Returned status code for the Request */
    char *responseHeaders;              /* pointer to response headers string if copyResponseHeaders flag is set in open settings */
                                        /* IP Applib frees this memory when IP Session is closed */
} B_PlaybackIpHttpSessionOpenStatus, B_PlaybackIpRtspSessionOpenStatus;

/**
Summary:
Aggregate IP Session Open Status
**/
typedef struct B_PlaybackIpSessionOpenStatus
{
    B_PlaybackIpSocketState socketState;
    union {
        B_PlaybackIpHttpSessionOpenStatus http;
        B_PlaybackIpRtspSessionOpenStatus rtsp;
    }u;
} B_PlaybackIpSessionOpenStatus;

/**
Summary:
API to Open a IP Session

Description:
This API will do the initial socket related configuration (socket,bind,connect,setsockopt,multicast join, etc.).
Also, depending on the chosen protocol, it will start a session w/ the server. E.g. in HTTP case, it will
send out the HTTP Get Request and process the Get response. In RTSP case, it will send the RTSP Option message
and process response. This API can be used in both blocking & non-blocking mode in which a callback in invoked
to indicate completion and then App needs to re-invoke this API to read the status (success/failure).
**/
B_PlaybackIpError B_PlaybackIp_SessionOpen(
    B_PlaybackIpHandle playback_ip,                 /* [in] */
    B_PlaybackIpSessionOpenSettings *openSettings,  /* [in] */
    B_PlaybackIpSessionOpenStatus *openStatus       /* [out] */
    );

/**
Summary:
RTSP Methods that app can specifically choose to invoke
**/
typedef enum B_PlaybackIpRtspMethodName
{
    B_PlaybackIpRtspMethod_eOptions,        /* RTSP OPTIONS Method */
    B_PlaybackIpRtspMethod_eDescribe,       /* RTSP DESCRIBE Method */
    B_PlaybackIpRtspMethod_eSetup,          /* RTSP SETUP Method */
    B_PlaybackIpRtspMethod_eGetParameter,   /* RTSP GET PARAMETER Method */
    B_PlaybackIpRtspMethod_eAnnounce        /* RTSP ANNOUNCE Method */
} B_PlaybackIpRtspMethodName;

/**
Summary:
RTSP Method related Settings
**/
typedef struct B_PlaybackIpRtspMethodSettings
{
    B_PlaybackIpRtspMethodName name;        /* RTSP Method name: */
    char *additionalHeaders;                /* additional headers that app wants to include in the outgoing RTSP Method */
    char *userAgent;                        /* app can override the default UserAgent string used in the outgoing Method Request*/
    bool copyResponseHeaders;               /* if true, a copy of the RTPS Method response Headers is provided back to app */
                                            /* this enables app to process & retrieve any custom app specific Headers (ServiceGroup, Version, AppData, etc.) */
} B_PlaybackIpRtspMethodSettings;

/**
Summary:
RTSP Send Method Status
**/
typedef struct B_PlaybackIpRtspMethodStatus
{
    int statusCode;                 /* Returned status code for the Request */
    char *responseHeaders;          /* pointer to response headers string if copyResponseHeaders flag is set in open settings */
                                    /* this allows apps to parse any custom response headers from the RTSP server and extract any fields for further use */
                                    /* this buffer is re-used in subsequent calls so app can't hold on it */
} B_PlaybackIpRtspMethodStatus;

/**
Summary:
API to allow app to send a particular RTSP method to the server
**/
B_PlaybackIpError B_PlaybackIp_SendRtspMethod(
    B_PlaybackIpHandle playback_ip,                     /* [in] */
    B_PlaybackIpRtspMethodSettings *settings,           /* [in] */
    B_PlaybackIpRtspMethodStatus *status                /* [out]*/
    );

/**
Summary:
HTTP Session Setup Settings
**/
typedef struct B_PlaybackIpHttpSessionSetupSettings
{
    NEXUS_TransportType contentTypeHint; /* app can hint the content type if it is known */
    off64_t contentLengthHint;        /* app can hint content (useful for chunk xfer encoding case where server doesn't send it in HTTP resp, 0 if not known */
    bool enablePayloadScanning;     /* if true, set up probe engine to determine accurate stream duration using payload scanning & random access */
                                    /* Note: setting this option will delay the start of video, but is needed to determine accurate info for perforing trick modes on TS & PES/VOB formats */
    bool skipPsiParsing;            /* if true, IP Applib doesn't probe the content for PSI information, app should apriori have this info */
    long psiParsingTimeLimit;       /* max number of msec allowed for psi parsing/media probe operation */
    unsigned avgBitRate;            /* average stream bitrate in bps if already known or 0 if unknown */
                                    /* If App set skipPsiParsing, then it is required to set avgBitRate value if app also wants to enable Runtime Buffering at IP Playback level. */
    unsigned seekOffsetTime;        /* initial time offset in milliseconds */
    bool disableRangeHeader;        /* if true, IP Applib doesn't include the Range header in the Get Requests */
    bool dontUseIndex;              /* dont use index for playback: false by default */
    /*TODO: may be this should be part of the Start call */
    bool convertLpcmToWave;         /* if Set, app is requesting to convert incoming LPCM files into Wave by inserting the Wave header before the data */
    unsigned bitsPerSample;
    unsigned sampleRate;
    unsigned numChannels;
    bool liveChannel;               /* app should set this flag if it knows this session pertains to a live channel */
                                    /* DLNA app knows this via Broadcast flag in DLNA protocolInfo structure, Comcast XRE app knows this via the media URL, etc. */
    unsigned readTimeout;           /* in msec: for live channels, app should set readTimeout value such that ip lib returns whatever it has returned in this duration */
                                    /* this helps with lowering the read related latency by allowing apps to read independent of strema bitrate (HD/SD/Audio only channel types) */
                                    /* apps (or playback_ip internally) can always ask for say 64KB/96KB worth of data from the socket and returns in this timeout duration for low bitrate cases */
    bool dontFeedDataToPlaybackHw;  /* app doesn't want playback_ip to feed data to playback h/w, instead it will read data from playback ip using setupStatus.file i/f and itself feed to h/w */
} B_PlaybackIpHttpSessionSetupSettings;

/**
Summary:
RTSP Session Setup Settings
**/
typedef struct B_PlaybackIpRtspSessionSetupSettings
{
    char *additionalHeaders;        /* additional headers that app wants to include in the all subsequent RTSP Requests (OPTION/DESCRIBE/SETUP) */
    bool alwaysUseAdditionalHeaders; /* indicates if all subsequent call should use these additional headers */
    char *userAgent;                /* app can override the default UserAgent string used in the outgoing OPTION/SETUP Request*/
    bool copyResponseHeaders;       /* if true, a copy of the SETUP response Headers is provided back to app */
                                    /* this enables app to process any custom app specific Headers (ServiceGroup, Version, AppData, etc.) */
    NEXUS_TransportType contentTypeHint; /* app can hint the content type if it is known */
    bool skipPsiParsing;            /* if true, IP Applib doesn't probe the content for PSI information, app should apriori have this info */
    long psiParsingTimeLimit;       /* max number of msec allowed for psi parsing/media probe operation */
                                    /* this enables app to process any custom app specific Headers (ServiceGroup, Version, AppData, etc.) */
} B_PlaybackIpRtspSessionSetupSettings;

/**
Summary:
UDP Session Setup Settings
**/
typedef struct B_PlaybackIpUdpSessionSetupSettings
{
    NEXUS_TransportType contentTypeHint; /* app can hint the content type if it is known */
    bool skipPsiParsing;            /* if true, IP Applib doesn't probe the content for PSI information, app should apriori have this info */
    long psiParsingTimeLimit;       /* max number of msec allowed for psi parsing/media probe operation */
                                    /* this enables app to process any custom app specific Headers (ServiceGroup, Version, AppData, etc.) */
} B_PlaybackIpUdpSessionSetupSettings;

/**
Summary:
Aggregate Session Setup Settings
**/
typedef struct B_PlaybackIpSessionSetupSettings
{
    unsigned unused;
    union {
        B_PlaybackIpHttpSessionSetupSettings http;
        B_PlaybackIpRtspSessionSetupSettings rtsp;
        B_PlaybackIpUdpSessionSetupSettings udp;
    }u;
    B_PlaybackIpSecurityOpenSettings security; /* Security Open Settings */
} B_PlaybackIpSessionSetupSettings;

/* App specific Header that gets inserted before media is streamed out */
typedef struct
{
    bool valid; /* set if appHeader is valid */
    unsigned length; /* length of app header bytes: should be mod16 aligned if it needs to be encrypted */
    uint8_t data[192]; /* pointer to app header */
} B_PlaybackIpAppHeader;

/* API to get default SessionSetup Settings */
B_PlaybackIpError B_PlaybackIp_GetDefaultSessionSetupSettings(
    B_PlaybackIpSessionSetupSettings *setupSettings
    );
/**
Summary:
HTTP Setup Status (output parameters of SessionSetup() )
**/
typedef struct B_PlaybackIpHttpSessionSetupStatus
{
    B_PlaybackIpPsiInfo psi;        /* PSI info for the media: valid only if psi.psiValid field is true */
    NEXUS_FilePlayHandle file;      /* file handle abstraction of the the network socket which allows apps/Nexus Playback to pull n/w data using file operations (read/seek/bounds) */
    const void *stream;             /* pointer to the bmedia_probe_stream structure to allow apps to extract additional program/track info */
    B_PlaybackIpAppHeader appHeader; /* appSpecific binary header sent by the server */
} B_PlaybackIpHttpSessionSetupStatus;

/**
Summary:
RTSP Setup Status (output parameters of SessionSetup() )
**/
#define MAX_SCALE_LIST_ENTRIES 16
typedef struct B_PlaybackIpRtspSessionSetupStatus
{
    B_PlaybackIpPsiInfo psi;        /* PSI info for the media: valid only if psi.psiValid field is true */
    int statusCode;                 /* Returned status code for the Request */
    char *responseHeaders;          /* pointer to response headers string if copyResponseHeaders flag is set in open settings */
                                    /* this allows apps to parse any custom response headers from the RTSP server and extract any fields for further use */
                                    /* this buffer is re-used in subsequent calls so app can't hold on it */
    int scaleListEntries;           /* number of entries in the scale list provided by the server, 0 if none */
    float scaleList[MAX_SCALE_LIST_ENTRIES]; /* scale list provided by the server, 0 if none */
    const void *stream;             /* pointer to the bmedia_probe_stream structure to allow apps to extract additional program/track info */
} B_PlaybackIpRtspSessionSetupStatus;

/**
Summary:
RTP/UDP Setup Status (output parameters of SessionSetup() )
**/
typedef struct B_PlaybackIpUdpSessionSetupStatus
{
    B_PlaybackIpPsiInfo psi;        /* PSI info for the media: valid only if psi.psiValid field is true */
    const void *stream;             /* pointer to the bmedia_probe_stream structure to allow apps to extract additional program/track info */
} B_PlaybackIpUdpSessionSetupStatus;

/**
Summary:
Aggregate Session Setup Status
**/
typedef struct B_PlaybackIpSessionSetupStatus
{
    unsigned unused;
    union {
        B_PlaybackIpHttpSessionSetupStatus http;
        B_PlaybackIpRtspSessionSetupStatus rtsp;
        B_PlaybackIpUdpSessionSetupStatus udp;
    }u;
}B_PlaybackIpSessionSetupStatus;

/**
Summary:
Session Setup API
Description:
This API establishes media session with the server and is applicable for HTTP and RTSP protocols only. E.g. for HTTP
protocol, is app has requested, this API can interact with the server to probe the media details (PSI, bitrate, etc.).
For RTSP protocol, this API will send OPTIONS, DESCRIBE, and SETUP messages to the server and process the response.

App can choose to work in the non-blocking mode in which case API returns E_IN_PROGRESS. Later when api is completed,
a callback is invoked w/ event B_PlaybackIpEvent_eSessionSetupDone. App should then re-call this api to determine

Note: this API doesn't do any work for UDP/RTP protocols.
**/
B_PlaybackIpError B_PlaybackIp_SessionSetup(
    B_PlaybackIpHandle playback_ip,                     /* [in] */
    B_PlaybackIpSessionSetupSettings *setupSettings,    /* [in] */
    B_PlaybackIpSessionSetupStatus *setupStatus         /* [out]*/
    );

/**
Summary:
API to close IP Playback session (informs server about session going away: RTSP TEARDOWN, HTTP: socket close)
**/
B_PlaybackIpError B_PlaybackIp_SessionClose(
    B_PlaybackIpHandle playback_ip /* Handle returned by bplayback_ip_open */
    );

/**
Summary:
IP Session Start Settings
**/
typedef struct B_PlaybackIpHttpSessionStartSettings
{
    bool preChargeBuffer;           /* true:  tells IP Applib to start pre-charging its network buffer */
                                    /* false: tells IP Applib to stop pre-charging if its pre-charging its network buffer */
    NEXUS_PlaybackPosition initialPlayPositionOffsetInMs; /* Initial play position offset that media play position will start to track. */
                                                          /* Mainly applicable for timeshift cases where play position starts somewhere in the timeshift window. */
} B_PlaybackIpHttpSessionStartSettings;

/**
Summary:
RTSP Session Start Settings
**/
typedef struct B_PlaybackIpRtspSessionStartSettings
{
    B_PlaybackIpProtocol mediaTransportProtocol;    /* Raw UDP or RTP: protocol used to transport media from server */
                                    /* if set to B_PlaybackIpProtocol_eMax, IP library determines it from the Transport header in the SETUP response, default of RTP */
    NEXUS_PlaybackPosition start;   /* start position in milli-sec: 0 means from the start */
    NEXUS_PlaybackPosition end;     /* end position in milli-sec: 0 means play to the end */
    int keepAliveInterval;          /* interval in seconds to send out the Keep alive heartbeats to the server, 0: IP library chooses the interval */
} B_PlaybackIpRtspSessionStartSettings;

/**
Summary:
RTP Session Start Settings
**/
typedef struct B_PlaybackIpRtpSessionStartSettings
{
    unsigned rtpPayloadType;            /* RTP Dynamic Payload type for AV Profile if known, 0 otherwise. */
} B_PlaybackIpRtpSessionStartSettings;

typedef struct B_PlaybackIpHlsAltAudioRenditionInfo
{
    int pid;                            /* pid of the alternate audio if app wants to play that instead of the main audio. */
    const char *language;               /* string containing 3 ASCII codes (+1 for NULL char) for audio language code associated w/ this audio. */
    const char *groupId;                /* groupId of the alternate audio. */
    NEXUS_TransportType containerType;  /* container type of the audio stream. */
} B_PlaybackIpHlsAltAudioRenditionInfo;

/**
Summary:
Aggregate Session Setup Settings
**/
typedef struct B_PlaybackIpSessionStartSettings
{
    bool nexusHandlesValid;         /* set this flag if nexusHandles are being set */
#ifndef DMS_CROSS_PLATFORMS
    B_PlaybackIpNexusHandles nexusHandles; /* nexus handles: Playback handle is only needed for HTTP sessions */
#endif /* DMS_CROSS_PLATFORMS */
    union {
        B_PlaybackIpHttpSessionStartSettings http;
        B_PlaybackIpRtspSessionStartSettings rtsp;
    }u;
    B_PlaybackIpRtpSessionStartSettings  rtp;
    NEXUS_TransportType mpegType;
    bool mediaPositionUsingWallClockTime;     /* playback position is maitained using decode rate: frames decoded/frame rate */
    bool startAlternateAudio;                /* if set to true, then lib will start alternate audio using the next field, alternateAudio, info. */
    B_PlaybackIpHlsAltAudioRenditionInfo alternateAudio; /* Allows run time selection of audio language. */
                                              /* if non-zero, app must set the language code below & playpump2 handle in nexusHandles. */
    bool audioOnlyPlayback;                   /* if set to true, HTTP Player can allocate smaller internal buffers (especially for HLS Player). */
    bool startPaused;                         /* Flag to indicate that app is starting in the paused mode. App will need to call B_PlaybackIp_Play() to resume playing. */
    bool monitorPsiAndRemapPids;              /* Flag to indicate if PBIP should monitor the PSI changes during runtime & remap the pids. */
} B_PlaybackIpSessionStartSettings;

/**
Summary:
HTTP Setup Status (output parameters of SessionStart() )
**/
typedef struct B_PlaybackIpHttpSessionStartStatus
{
    NEXUS_PlaybackPosition curBufferDuration;   /* if preChargeBuffer flag is set, this field indicates how much of content is currently buffered in HTTP Cache, in milli-sec */
    NEXUS_PlaybackPosition maxBufferDuration;   /* if preChargeBuffer flag is set, this field indicates how much of content can be buffered in HTTP Cache, in milli-sec (based on max bitrate & HTTP cache size */
} B_PlaybackIpHttpSessionStartStatus;

/* TODO: B_PlaybackIpRtspSessionStartStatus  --> B_PlaybackIpRtspSessionStatus if all return status seems to be the same */
/**
Summary:
RTSP Setup Status (output parameters of SessionStart() )
**/
typedef struct B_PlaybackIpRtspSessionStartStatus
{
    int statusCode;                 /* Returned status code for the Request */
    char *responseHeaders;          /* pointer to response headers string if copyResponseHeaders flag is set in open settings */
                                    /* this allows apps to parse any custom response headers from the RTSP server and extract any fields for further use */
                                    /* this buffer is re-used in subsequent calls so app can't hold on it */
} B_PlaybackIpRtspSessionStartStatus;

/**
Summary:
Aggregate Session Setup Status
**/
typedef struct B_PlaybackIpSessionStartStatus
{
    unsigned unused;
    union {
        B_PlaybackIpHttpSessionStartStatus http;
        B_PlaybackIpRtspSessionStartStatus rtsp;
    }u;
} B_PlaybackIpSessionStartStatus;

/**
Summary:
API to start IP Session
For HTTP protocol, it starts the buffering thread.
For RTSP protocol, it sends PLAY command to the server and status
UDP/RTP: starts the thread to receive UDP/RTP data and start processing it. This also happens for RTSP case.
**/
B_PlaybackIpError B_PlaybackIp_SessionStart(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus *startStatus
    );

/**
Summary:
API to stop IP Session
**/
B_PlaybackIpError B_PlaybackIp_SessionStop(
    B_PlaybackIpHandle playback_ip
    );

/**
Summary:
TTS throttle parameters
**/
typedef struct B_PlaybackIp_TtsThrottle_Params {
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_PlaypumpHandle playPump;      /* handle of the playpump associated with this TTS throttle */
#endif /* DMS_CROSS_PLATFORMS */
    NEXUS_Timebase timebase;            /* timebase that will be frequency controlled by the TTS throttle */
    unsigned initBufDepth;              /* sets the initial buffer depth (used as a target buffer depth by TTS pacing) */
    unsigned minBufDepth;               /* minimum buffer depth before buffer depth violation callback is called */
    unsigned maxBufDepth;               /* minimum buffer depth before buffer depth violation callback is called */
    unsigned maxClockMismatch;          /* specify the maximum clock mismatch (in ppm) between server/encoder and STB */
    void (*bufDepthViolationCallback)(void *context, bool isMax); /* function will be called if max/min buffer depth is violated */
    void *bufDepthViolationCbContext;   /* context passed into buf_depth_violation_callback */
} B_PlaybackIp_TtsThrottle_Params;

typedef struct B_PlaybackIp_TtsParams {
    bool autoDetect;                    /* automatically detect if the stream is TTS or TS */
    unsigned pacingMaxError;            /* set the timestamp error bound, as used by the playback pacing logic */
    B_PlaybackIp_TtsThrottle_Params throttleParams;
} B_PlaybackIp_TtsParams;

/**
Summary:
Playback IP App Lib settings structure
**/
typedef struct B_PlaybackIpSettings
{
    B_PlaybackIpClockRecoveryMode ipMode; /* clock recovery mode for Live channels */
                                          /* ip_client.c has an example function useLiveModeForIpPlayback() to aid apps in determining this flag */
    int maxNetworkJitter;               /* Maximum network jitter in millisec: used to determining packet loss conditions */
                                        /* this is considered as unmarked discontinuity and AV pipeline is flushed at this point */
    B_PlaybackIp_TtsParams ttsParams;   /* TTS related parameters */

    bool nexusHandlesValid;             /* set this flag if nexusHandles are being set */
#ifndef DMS_CROSS_PLATFORMS
    B_PlaybackIpNexusHandles nexusHandles; /* provides handles to various Nexus modules */
#endif /* DMS_CROSS_PLATFORMS */
    bool useNexusPlaypump;              /* true: tells IP Applib to feed network stream to the Nexus Playpump, False: tells IP Applib that Nexus Playback will pull stream from IP Applib */
                                        /* ip_client.c has an example function useNexusPlaypumpForIpPlayback() to aid apps in determining the value of this flag */
    bool preChargeBuffer;               /* true:  tells IP Applib to start pre-charging its network buffer */
                                        /* false: tells IP Applib to stop pre-charging if its pre-charging its network buffer */
    unsigned networkTimeout;            /* provides the timeout value, in seconds, for network operations like select, read. */
                                        /* 0: IP Applib uses a default of 30 sec */
    unsigned networkBufferSize;         /* size of network cache, in bytes, where data is buffered (currently used for HTTP based Pull Modes) */
                                        /* 0: IP Applib uses a default size of couple of MB */
    bool enableEndOfStreamLooping;      /* if set, ip library will loop around after server sends EOF. Only applicable to sessions using NEXUS playpump. */
                                        /* Session using NEXUS playpback accomplish this via the playback endOfStreamAction flag */
    bool startAlternateAudio;           /* if set to true, then lib will start alternate audio using the next field, alternateAudio, info. */
    B_PlaybackIpHlsAltAudioRenditionInfo alternateAudio; /* Allows run time selection of audio language. */
    bool stopAlternateAudio;            /* if set to true, then library will stop the current alternate audio rendition. */
    bool waitOnEndOfStream;             /* if set, ip library will issue the endOfStreaming eventCallback to app & wait for next API (instead of exiting the thread). */
    bool playPositionOffsetValid;       /* flag to indicate if playPositionOffsetInMs is valid. */
    NEXUS_PlaybackPosition playPositionOffsetInMs; /* Updated Play position offset that should be used as the starting play position to play from. */
                                        /* Mainly applicable for timeshift cases to align current position w/ the server's current position. */
} B_PlaybackIpSettings;

/**
Summary:
This function returns the default and recommended values for the IP App Lib public settings.
**/
B_PlaybackIpError B_PlaybackIp_GetDefaultSettings(
    B_PlaybackIpSettings *pSettings
    );

/**
Summary:
This function returns the current values for the Playback IP App Lib public settings.
**/
B_PlaybackIpError B_PlaybackIp_GetSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSettings *pSettings
    );

/**
Summary:
This function updates the current values for the Playback IP App Lib public settings.
**/
B_PlaybackIpError B_PlaybackIp_SetSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpSettings *pSettings
    );

/**
Summary:
Supported Trick play types

Description:
These types allows app to specify whether it wants to use client based (ByteRange or TimeSeekRange) or server based (PlaySpeed) trickplays
**/
typedef enum B_PlaybackIpTrickModesMethod
{
    B_PlaybackIpTrickModesMethod_UseByteRange,      /* Client side trick play: Use HTTP Range header to request chunks of data */
                                                    /* requires apps to configure using Nexus Playback for this mode */
    B_PlaybackIpTrickModesMethod_UseTimeSeekRange,  /* Server side trick play: Use Time ranges instead of byte ranges to achieve affect of trick modes */
                                                    /* requires apps to configure using Nexus Playpump for this mode */
    B_PlaybackIpTrickModesMethod_UsePlaySpeed,      /* Server side trick play: inform server about the desired play speed and play the returned scaled data */
                                                    /* requires apps to configure using Nexus Playpump for this mode */
    B_PlaybackIpTrickModesMethod_UseRvuSpec,        /* RVU trickmodes: combination of client & server side trick modes */
                                                    /* requires apps to configure using Nexus Playpump for this mode */
    B_PlaybackIpTrickModesMethod_Max
} B_PlaybackIpTrickModesMethod;

/**
Summary:
Various methods to pause a playback session

Description:
Only applicable for HTTP transport where either client can stall TCP connection or disconnect & reconnect w/ time seek
**/
typedef enum B_PlaybackIpPauseMethod
{
    B_PlaybackIpPauseMethod_UseConnectionStalling,  /* stall TCP connection by stop reading from socket: provides instant resumption */
    B_PlaybackIpPauseMethod_UseDisconnectAndSeek,   /* close current TCP connection and re-open new one upon play w/ new seek offset */
    B_PlaybackIpPauseMethod_Max
} B_PlaybackIpPauseMethod;

/**
Summary:
Trickmode Methods for AdaptiveStreaming Playback

Description:
Only applicable for HLS streaming for now.
**/
typedef enum B_PlaybackIpAdaptiveStreamingTrickModeMethod
{
    B_PlaybackIpAdaptiveStreamingTrickModeMethod_eUseSegmentWithLowestBandwidth,
    B_PlaybackIpAdaptiveStreamingTrickModeMethod_eUseSegmentWithMatchingRateAndNetworkBandwidth,
    B_PlaybackIpAdaptiveStreamingTrickModeMethod_eMax
} B_PlaybackIpAdaptiveStreamingTrickModeMethod;

/**
Summary:
Trickmode settings
**/
typedef struct B_PlaybackIpTrickModesSettings
{
    B_PlaybackIpTrickModesMethod method;        /* type of trick mode to perform */
    int rate;                                   /* desired rate/speed (in the units of 1, 2, 3, -1, -2, -3): simplified rate to provide next speed */
    bool absoluteRateDefined;                   /* if set, IP library should simply use the rate value defined in the next absoluteRate field & ignore the previous rate field */
    int  absoluteRate;                          /* Speed control based on units of NEXUS_NORMAL_DECODE_RATE: */
                                                /* NEXUS_NORMAL_DECODE_RATE = normal play. */
                                                /* 0 = pause. */
                                                /* NEXUS_NORMAL_DECODE_RATE*2 = 2x fast-forward. */
                                                /* NEXUS_NORMAL_DECODE_RATE/2 = 2x slow-motion. */

    int frameRateInTrickMode;                   /* frameRate that it should set in the trickmodes in order to achieve a given play speed */
                                                /* server w/ playspeed support provides this value via the DLNA FrameRateInTrickMode header */
    /* note: this is being deprecated */
    unsigned frameRepeat;                       /* specifies the number of times the decoder should display each frame/field */
                                                /* typically set based on the frameRepeat value provided by the server */
    B_PlaybackIpPauseMethod pauseMethod;        /* indicates how to pause: just stall pipeline/connection or Disconnect and reconnect to the paused location  */
    NEXUS_PlaybackPosition seekPosition;        /* seek position in millisecond */
    bool seekPositionIsRelative;                /* if set, then seekPosition is a relative to current position */
    bool seekBackward;                          /* if set, then seekPosition is a relative to current position in the backward direction */
    bool enableAccurateSeek;                    /* If set, library will try to seek to the PTS accurate position. Primariliy targetted for HLS Playback */
                                                /* where segments can be of longer duration (such as 10sec) and seek position may be within the segment. */
                                                /* Library will interpolate the expected PTS using first PTS & seek duration and ask decoder to drop frames until this PTS. */
                                                /* Note: this approach only reliably works if stream has no discontinuities and shouldn't be set in such playback scenarios. */
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_VideoDecoderDecodeMode decodeMode;    /* allows apps to instruct decoder to only play certain frames types (e.g. I, IP), default is all fed frames */
#endif   /* DMS_CROSS_PLATFORMS  */
    bool nonBlockingMode;                       /* if set, requires apps to also set following eventCallback function field */
    char *playSpeedString;                      /* Instead of rate or absoluteRate values, apps can optionally provide explicit playSpeedString for server side trickmodes */
    bool playSpeedStringDefined;                /* if set, explicit playSpeedString is used instead of rate/absoluteRate values */
    bool dontUseTimeSeekRangeInPlaySpeed;       /* if set, DLNA TimeSeekRange Header will not be sent in the DLNA PlaySpeed Header. */
                                                /* App may use it when a Server supports PlaySpeed but not TimeSeek. Server will decide the point from where to stream content. */
    B_PlaybackIpAdaptiveStreamingTrickModeMethod adaptiveStreamingTrickmodeMethod; /* For HLS Trickmodes, this provides further control on how the segments should be selected during trickmodes. */
} B_PlaybackIpTrickModesSettings;

/**
Summary:
API to pause the IP Session (applicable for RTSP & HTTP Sessions)
**/
B_PlaybackIpError B_PlaybackIp_Pause(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *trickMode
    );

/**
Summary:
API to resume the IP Session (applicable for RTSP & HTTP Sessions)
**/
B_PlaybackIpError B_PlaybackIp_Play(
    B_PlaybackIpHandle playback_ip
    );

/**
Summary:
API to resume the IP Session (applicable for RTSP & HTTP Sessions)
**/
B_PlaybackIpError B_PlaybackIp_PlayAsync(
    B_PlaybackIpHandle playback_ip
    );

/**
Summary:
API to carry out the trickMode commands (applicable for RTSP & HTTP Sessions)
**/
B_PlaybackIpError B_PlaybackIp_TrickMode(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *trickMode
    );

/**
Summary:
API to carry out the seek trickMode command (HTTP & RTSP sessions Sessions)
**/
B_PlaybackIpError B_PlaybackIp_Seek(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *trickMode
    );

/**
Summary:
API to get trickmode parameter for a given rate
**/
B_PlaybackIpError B_PlaybackIp_GetTrickModeByRate(
    B_PlaybackIpHandle playback_ip,
    int rate,
    B_PlaybackIpTrickModesSettings *trickMode
    );

/**
Summary:
API to provide the current/default trickmode Settings
**/
B_PlaybackIpError B_PlaybackIp_GetTrickModeSettings(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpTrickModesSettings *ipTrickModeSettings
    );

/**
Summary:
API to Advance by one frame in either forward or backward direction.
Requires caller to Pause the Playback Ip Session before calling this API.
**/
B_PlaybackIpError B_PlaybackIp_FrameAdvance(
    B_PlaybackIpHandle playback_ip,
    bool forward,                        /* if true, one frame is played in forward direction, otherwise, one frame is played in the backward direction. */
    bool nonBlockingMode                 /* if set, API is run in the non-blocking mode and its completion is provided via B_PlaybackIpSessionOpenSettings.eventCallback. */
    );

/**
Summary:
State of a IP playback context: relevant for non-live sessions
(e.g. for HTTP or RTSP VoD playbacks)
**/
typedef enum B_PlaybackIpState
{
    B_PlaybackIpState_eClosed,                  /* Initial closed state */
    B_PlaybackIpState_eOpened,                  /* Playback IP session context is allocated, but not initialized */
    B_PlaybackIpState_eSessionOpenInProgress,   /* SessionOpen is in progress */
    B_PlaybackIpState_eSessionOpened,           /* SessionOpen is successfully completed */
    B_PlaybackIpState_eStopped = B_PlaybackIpState_eSessionOpened, /* SessionStop is completed: equivalent to eSetup State */
    B_PlaybackIpState_eSessionSetupInProgress,  /* SessionSetup is in progress */
    B_PlaybackIpState_eSessionSetup,            /* SessionSetup is completed */
    B_PlaybackIpState_eSessionStartInProgress,  /* SessionStart is in progress */
    B_PlaybackIpState_ePlaying,                 /* SessionStart is completed and we are ready to play */
    B_PlaybackIpState_eStopping,                /* SessionStop is in progress */
    B_PlaybackIpState_ePaused,                  /* IP Session has been paused */
    B_PlaybackIpState_eWaitingToEnterTrickMode, /* IP Session is waiting to enter trick mode state as either Nexus IO or HTTP thread is currently using the session */
    B_PlaybackIpState_eEnteringTrickMode,       /* IP Session is entering trick mode state */
    B_PlaybackIpState_eTrickMode,               /* IP Session is in trick mode state */
    B_PlaybackIpState_eBuffering,               /* IP Session is buffering data */
    B_PlaybackIpState_eMax                      /* Invalid State */
} B_PlaybackIpState;

typedef struct B_PlaybackIpRtpRxStats
{
    uint32_t packetsReceived;                   /* total IP/UDP/RTP packets received */
    uint64_t bytesReceived;                     /* total AV Payload bytes received in RTP Packets */
    uint32_t packetsDiscarded;                  /* total packets discarded: */
    uint32_t packetsOutOfSequence;              /* total packets whose RTP sequence doesn't match the expected next sequence */
    uint32_t packetsLost;                       /* total packets that were lost */
    uint32_t packetsLostBeforeErrorCorrection;  /* */
    uint32_t lossEvents;                        /* total number of times an expected packet was lost */
    uint32_t lossEventsBeforeErrorCorrection;   /* */
} B_PlaybackIpRtpRxStats;

typedef struct B_PlaybackIpRtpTxStats
{
    unsigned packetsSent;                       /* */
    unsigned bytesSent;                         /* */
} B_PlaybackIpRtpTxStats;

typedef struct B_PlaybackIpHlsPlayerStats
{
    unsigned lastSegmentDownloadTime;           /* time, in msec, taken to download the last segment: spans from HTTP Get Request to full segment download */
    unsigned lastSegmentBitrate;                /* time, in msec, taken to acquire the last segment: spans from HTTP Get Request to full download of segment */
    unsigned lastSegmentDuration;               /* duration, in msec, of the last segment fed to the playback h/w channel */
    unsigned lastSegmentSequence;               /* sequence number of the last segment fed to the playback h/w channel */
    const char *lastSegmentUrl;                 /* URL (i.e. playlist name) of the last segment fed to the playback h/w channel */
    bool     bounded;                           /* set if EXT-ENDLIST tag is present, meaning the Playlist represents the bounded content. */
} B_PlaybackIpHlsPlayerStats;

typedef struct B_PlaybackIpSessionInfo
{
    const char *ipAddr;                         /* Server's IP address */
    unsigned port;                              /* Server's Port # */
    const char *url;                            /* URL: doesn't contain the IP:Port */
    B_PlaybackIpProtocol protocol;              /* Streaming Protocol: HTTP/UDP/RTP... */
    B_PlaybackIpSecurityProtocol securityProtocol; /* Security Protocol: DTCP/IP, SSL, etc. */
    bool hlsSessionEnabled;                     /* set if current session is being receiving using HTTP Live Streaming (HLS) Protocol */
    bool mpegDashSessionEnabled;                /* set if current session is being receiving using MPEG-DASH Protocol */
} B_PlaybackIpSessionInfo;

/**
Summary:
SSL Specific Settings
**/
typedef struct B_PlaybackIpStatus
{
    NEXUS_PlaybackPosition curBufferDuration;   /* how much of content is currently buffered in HTTP Cache, in milli-sec */
    NEXUS_PlaybackPosition maxBufferDuration;   /* how much of content can be buffered in HTTP Cache, in milli-sec (based on max bitrate & HTTP cache size */
    B_PlaybackIpState ipState;                  /* current state of the IP Applib */
    bool serverClosed;                          /* set to true if server has closed the connection before the GetStatus() call */
    int httpStatusCode;                         /* Returned status code for the HTTP Request */
    /* need to see if we will continue to need these fields for drawing the playback bar, this info can be obtained directly from Playback */
    /* can only be an issue for HTTP playback from httpstreamer */
    NEXUS_PlaybackPosition first;               /* first timestamp of stream played in mSec */
    NEXUS_PlaybackPosition last;                /* last timestamp of stream played in mSec */
    NEXUS_PlaybackPosition position;            /* current timestamp of stream played in mSec */
    unsigned numRecvTimeouts;                   /* count of the number of receive timeouts since start of IP playback */
    off_t totalConsumed;                        /* total bytes consumed by the Media player */
    B_PlaybackIpRtpRxStats rtpStats;            /* RTP Rx related commulative stats: only updated if RTP session is being played */
    B_PlaybackIpSessionInfo sessionInfo;        /* Provides session related info: IP, Port, URL, Prototol, Security, etc. */
    B_PlaybackIpHlsPlayerStats hlsStats;        /* valid if sessionInfo.hlsSessionEnabled is true */
} B_PlaybackIpStatus;

/**
Summary:
This function returns the IP Playback status (current PTS, Playpump FIFO Info, etc.)
TODO: add info regarding socket stats (bytes received, etc.) in the Status Structure
**/
B_PlaybackIpError B_PlaybackIp_GetStatus(
    B_PlaybackIpHandle playback_ip,
    B_PlaybackIpStatus *ipStatus    /* [out]: returned status */
    );

/**
Summary:
API to detect if the stream contains timestamps or not
**/
B_PlaybackIpError B_PlaybackIp_DetectTts(
    B_PlaybackIpHandle playback_ip,
    bool *isTts
    );

#ifdef B_HAS_SSL
/**
Summary:
SSL Specific Settings
**/
typedef struct B_PlaybackIpSslInitSettings
{
    char *rootCaCertPath;                 /* path for root CA cert, can be turned into array of multiple CA certs */
    bool clientAuth;                      /* client Authentication. Set true for following paramerters */
    char *ourCertPath;                    /* path for our certificate */
    char *privKeyPath;                    /* path for private keys */
    char *password;                       /* set to NULL if no password */
    bool sslLibInitDone;                  /* Set true if previously called SSL_library_init() */
} B_PlaybackIpSslInitSettings;

/**
Summary:
API to create SSL context.
**/
void* B_PlaybackIp_SslInit(             /* returns ssl context */
        B_PlaybackIpSslInitSettings *initSettings
        );

/**
Summary:
API to destroy SSL context
**/
void B_PlaybackIp_SslUnInit(void *ctx);

#endif

#ifdef B_HAS_RAD_EA
/**
Summary:
API to create RAD/EA context.
**/
void* B_PlaybackIp_RadEaInit(             /* returns radea context */
        B_PlaybackIpSslInitSettings *initSettings
        );

void B_PlaybackIp_RadEaUnInit(void *ctx);

#endif

#ifdef B_HAS_DTCP_IP
/**
Summary:
APIs to initialize & un-initialize DTCP/IP context.
**/
void * B_PlaybackIp_DtcpIpInit(void *);
void B_PlaybackIp_DtcpIpUnInit(void *);
#endif

/**
Summary:
This function sets up the network record functionality where a live stream
is recorded on the specified server.
**/
B_PlaybackIpError B_PlaybackIp_NetworkRecordingStart(
    B_PlaybackIpSocketOpenSettingsHandle socketOpenSettings,
    const char *fname, const char *indexname,
    B_PlaybackIpPsiInfoHandle psi,
    NEXUS_FileRecordHandle *fileRecordPtr
    );

/**
Summary:
This function stops the network recording session.
**/
void B_PlaybackIp_NetworkRecordingStop(
    struct NEXUS_FileRecord *file
    );

/* *********************************************** */
/*       HTTP Streaming Server Specific APIs       */
/* *********************************************** */
typedef enum B_PlaybackIpConnectionState
{
    B_PlaybackIpConnectionState_eSetup,  /* Socket connection w/ peer is being setup and not yet xfering AV data */
    B_PlaybackIpConnectionState_eActive, /* Socket connection w/ peer is active (sending or receiving AV data) */
    B_PlaybackIpConnectionState_eTimeout,/* Socket connection w/ peer has timeouts in either sending or receiving data */
    B_PlaybackIpConnectionState_eError,  /* Socket connection w/ peer has encoutered some errors: peer abnormally closed */
    B_PlaybackIpConnectionState_eEof,    /* Socket connection w/ peer has been gracefully closed by the peer */
    B_PlaybackIpConnectionState_eMax     /* Max value */
} B_PlaybackIpConnectionState;

typedef struct B_PlaybackIpLiveStreamingRtpUdpOpenSettings {
    int streamingPort; /* Port # to stream for RTP/UDP protocols */
    char streamingIpAddress[32]; /* IP address to stream for RTP/UDP protocols */
    const char *interfaceName; /* For UDP/RTP protocol, app should provide the interface name to stream out on (defaults to eth0) */
} B_PlaybackIpLiveStreamingRtpUdpOpenSettings;

typedef struct B_PlaybackIpLiveStreamingPatPmtData {
    const uint8_t *patBufferPtr;
    size_t patBufferSize;
    const uint8_t *pmtBufferPtr;
    size_t pmtBufferSize;
} B_PlaybackIpLiveStreamingPatPmtData;

/* For Streaming from a Live (QAM/VSM/IP) or File (local disk) sources */
typedef struct B_PlaybackIpLiveStreamingOpenSettings
{
    char fileName[128]; /* Name of the file to stream */
    off64_t beginFileOffset;
    off64_t endFileOffset;
    bool autoRewind;    /* set to indicate stream rewind upon eof */
    int playSpeed;
    B_PlaybackIpProtocol protocol; /* protocol: UDP, RTP, HTTP */
    int streamingFd; /* socket fd on which to stream data on (not needed for RTP/UDP protocols) */
    B_PlaybackIpLiveStreamingRtpUdpOpenSettings rtpUdpSettings;
    B_PlaybackIpSecurityOpenSettings securitySettings; /* Security Open Settings */
    B_PlaybackIpPsiInfoHandle psi;    /* since app had to know the PSI settings for live streams, it passes them to IP library */
    NEXUS_FifoRecordHandle fifoFileHandle;
    NEXUS_RecpumpHandle recpumpHandle;
    B_PlaybackIp_EventCallback eventCallback;           /* callback function if nonBlockingMode is desired or to receive any asynchronous events from the IP library */
    void *appCtx;                                       /* app context handle used in the callback function */
    bool streamingDisabled; /* flag to indicate to live streaming thread that streaming is disabled for this thread (Fast Channel Change or HLS Streaming). App will later set streamingEnabled via SetSettings */
    BKNI_EventHandle dataReadyEvent;                    /* recpump uses this event to notify IP library about availability of data in the recpump buffer */
    bool transportTimestampEnabled;                     /* indicates if MPEG2 TS content contains 4 byte DLNA timpstamp */
    bool hlsSession;         /* set if this is a HLS session */
    int hlsSegmentSize;      /* size of a hls segment in bytes */
    B_PlaybackIpAppHeader appHeader;    /* app header that gets inserted before media is streamed out */
    bool enableTimeshifting;    /* set if app has enabled timeshifting */
    bool enableTimestamps;      /* set if app has enabled timestamps in the recorded live streams */
    NEXUS_HeapHandle heapHandle; /* nexus heap Handle, can be null if app doesn't have a preference */
    bool sendNullRtpPktsOnTimeout; /* some protocols require us to send Null RTP packets (no payload or no TS NULL packets) upon certain Timeout */
    unsigned dataReadyTimeoutInterval; /* interval when we should send Null RTP packets */
    unsigned ipTtl; /* Time To Live (TTL) for IPv4 */
    B_PlaybackIpLiveStreamingPatPmtData psiTables;
    bool dontCloseSocket;
} B_PlaybackIpLiveStreamingOpenSettings;

/* Settings that can be updated when live streaming start is already called */
typedef struct B_PlaybackIpLiveStreamingSettings
{
    bool streamingEnabled; /* flag to indicate to live streaming thread that streaming has now been enabled (Fast Channel Change) */
    bool resumeStreaming; /* flag to indicate to live streaming thread to resume streaming for the new HLS segment */
    bool resetStreaming; /* flag set during the seek event to indicate streaming thread to reset its state */
    bool abortStreaming; /* flag set during the back to back seek events to indicate streaming thread to abort its current streaming operation as new request has come in */
    int streamingFd; /* socket fd on which to stream data on */
    int hlsSegmentSize;      /* size of a hls segment in bytes */
    B_PlaybackIpLiveStreamingPatPmtData psiTables;
} B_PlaybackIpLiveStreamingSettings;

/***************************************************************************
Summary:
This function starts streaming the live content from rave buffers to a network client.
***************************************************************************/
typedef struct B_PlaybackIpLiveStreaming *B_PlaybackIpLiveStreamingHandle;
B_PlaybackIpLiveStreamingHandle
B_PlaybackIp_LiveStreamingOpen(
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings
    );

/***************************************************************************
Summary:
This function stops the live streaming function.
***************************************************************************/
void B_PlaybackIp_LiveStreamingClose(B_PlaybackIpLiveStreamingHandle liveStreamingHandle);
B_PlaybackIpError
B_PlaybackIp_LiveStreamingStart(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    );
void
B_PlaybackIp_LiveStreamingStop(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    );

typedef struct B_PlaybackIpLiveStreamingStatus
{
    off64_t bytesStreamed;
    B_PlaybackIpConnectionState connectionState;
    B_PlaybackIpRtpTxStats rtpStats;            /* RTP Tx related commulative stats: only updated if RTP session is being streamed out */
} B_PlaybackIpLiveStreamingStatus;
void
B_PlaybackIp_LiveStreamingGetStatus(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle,
    B_PlaybackIpLiveStreamingStatus *status
    );
B_PlaybackIpError
B_PlaybackIp_LiveStreamingSetSettings(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle,
    B_PlaybackIpLiveStreamingSettings *settings
    );

typedef struct B_PlaybackIpFileStreaming *B_PlaybackIpFileStreamingHandle;
typedef struct B_PlaybackIpFileStreamingOpenSettings
{
    char fileName[128]; /* Name of the file to stream */
    char mediaInfoFilesDir[128];  /* directory name where the info & nav files should be created */
    off64_t beginFileOffset;
    off64_t endFileOffset;
    bool autoRewind;    /* set to indicate stream rewind upon eof */
    int playSpeed;
    double beginTimeOffset;
    double endTimeOffset;
    B_PlaybackIpProtocol protocol; /* protocol: UDP, RTP, HTTP */
    int streamingFd; /* socket fd on which to stream data on */
    B_PlaybackIpLiveStreamingRtpUdpOpenSettings rtpUdpSettings;
    bool mediaProbeOnly; /* if set, session is opened for media probe only */
    B_PlaybackIpSecurityOpenSettings securitySettings; /* Security Open Settings */
    B_PlaybackIp_EventCallback eventCallback;           /* callback function if nonBlockingMode is desired or to receive any asynchronous events from the IP library */
    void *appCtx;                                       /* app context handle used in the callback function */
    bool transportTimestampEnabled;                     /* indicates if MPEG2 TS content contains 4 byte DLNA timpstamp */
    B_PlaybackIpAppHeader appHeader;    /* app header that gets inserted before media is streamed out */
    bool disableIndexGeneration; /* if set, nav index is not generated during media probing */
    NEXUS_HeapHandle heapHandle; /* nexus heap Handle, can be null if app doesn't have a preference */
    bool disableHlsPlaylistGeneration; /* if set, HLS Playlists are not generated during media probing */
    unsigned programIndex;  /* Default value should be 1 to match bmedia indexing. used to determe which program to look at  */
    bool generateAllProgramsInfoFiles; /* default is false, used to get infor for particular program;  true means it will generate Info files for all Programs in streams */
} B_PlaybackIpFileStreamingOpenSettings;

/***************************************************************************
Summary:
This function starts streaming the content from local file to a network client.
***************************************************************************/
B_PlaybackIpFileStreamingHandle
B_PlaybackIp_FileStreamingOpen(
    const B_PlaybackIpFileStreamingOpenSettings *fileStreamingSettings
    );

/***************************************************************************
Summary:
This function stops streaming content from a file.
***************************************************************************/
void
B_PlaybackIp_FileStreamingClose(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    );

B_PlaybackIpError
B_PlaybackIp_FileStreamingStart(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    );

void
B_PlaybackIp_FileStreamingStop(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle
    );

B_PlaybackIpError
B_PlaybackIp_FileStreamingGetMediaInfo(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpPsiInfoHandle psi
    );

typedef struct B_PlaybackIpFileStreamingStatus
{
    off64_t bytesStreamed;
    B_PlaybackIpConnectionState connectionState;
} B_PlaybackIpFileStreamingStatus;

void
B_PlaybackIp_FileStreamingGetStatus(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpFileStreamingStatus *status
    );

/***************************************************************************
Summary:
These functions are exported in order to allow BIP to reuse nav, hls, mpd generation within PBIP
***************************************************************************/
bool
openMediaIndexer(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    const char                      *indexFileName,
    B_PlaybackIpPsiInfoHandle       psi
    );

int
getStreamGopInfo(
    B_PlaybackIpFileStreamingHandle fileStreamingHandle,
    B_PlaybackIpPsiInfoHandle       psi
    );

void
replaceFileExtension(
    char *newName,
    int newNameSize,
    char *curName,
    char *newExtension
    );

int
generateHlsPlaylist(
    unsigned    mediaDuration,
    const char *mediaFileNameFull,
    const char *playlistFileDirPath
    );

int
generateMpdPlaylist(
    unsigned    mediaDuration,
    const char *mediaFileNameFull,
    const char *playlistFileDirPath
    );

#ifdef __cplusplus
}
#endif
#endif /* #ifndef B_PLAYBACK_IP_LIB_H__ */
