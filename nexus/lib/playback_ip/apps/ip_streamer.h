/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *  ip streamer include file
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#ifndef __IP_STREAMER_H__
#define __IP_STREAMER_H__

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "blst_queue.h"
#include "nexus_file_fifo.h"

#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "b_os_lib.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#endif
#include "nexus_audio_mixer.h"
#if NEXUS_HAS_STREAM_MUX
#include "nexus_stream_mux.h"
#endif
#ifdef NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif

#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif

#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

/* confirm the max jitter we can have on DoCSYS/IP Path */
#define DOCSIS_IP_NETWORK_JITTER 100
#define IP_STREAMER_DEFAULT_SERVER_PORT 5000
#define IP_STREAMER_DEFAULT_DTCP_IP_PORT 8000

/* Max Number of concurrent sessions that need to be supported for a platform */
#define IP_STREAMER_MAX_THREADS 48

/* Max Number of IP Frontends for a platforms: assume 1/2 for live ip streaming */
#define IP_STREAMER_NUM_PLAYPUMPS_FOR_LIVE_IP (NEXUS_NUM_PLAYPUMPS/2)
/* recording sessions over IP */
#define IP_STREAMER_NUM_RECPUMPS_FOR_RECORD (4)
/* how many live sessions (qam/sat/ip/vsb/streamer) can be streamed by recording via recpump */
#define IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING (NEXUS_NUM_RECPUMPS)

#if NEXUS_NUM_PARSER_BANDS
#define IP_STREAMER_NUM_PARSER_BANDS NEXUS_NUM_PARSER_BANDS

/* leaving two parser bands for satellite input: special case for 7346 */
#if (NEXUS_NUM_PARSER_BANDS > 2)
#define IP_STREAMER_NUM_STREAMER_INPUT NEXUS_NUM_PARSER_BANDS-2
#else
#define IP_STREAMER_NUM_STREAMER_INPUT 1
#endif
#else /* NEXUS_NUM_PARSER_BANDS == 0 */
#define IP_STREAMER_NUM_PARSER_BANDS 1
#define IP_STREAMER_NUM_STREAMER_INPUT 1
#endif

#ifdef NEXUS_HAS_VIDEO_ENCODER
typedef struct TranscoderDst {
    /* nexus resources needed to setup the decode and re-encode pipe */
    NEXUS_Timebase timebase;
    int displayIndex; /* compositor to connect to the video encoder */
    NEXUS_StcChannelHandle videoStcChannel;
    NEXUS_StcChannelHandle audioStcChannel;
    NEXUS_StcChannelHandle encodeStcChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_DisplayHandle displayMain;
    NEXUS_VideoWindowHandle windowMain;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle syncChannel;
#endif
    int decoderIndex;

    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio;
    NEXUS_PlaypumpHandle playpumpTranscodeSystemData;

    NEXUS_PidChannelHandle transcodeVideoPidChannelCopy;
    NEXUS_PidChannelHandle transcodeAudioPidChannelCopy;

    /* AV pid channel pointers used in the xcode path */
    /* stored here so that they are closed only when last instance of a xcode session is closed */
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define IP_STREAMER_PSI_QUEUE_CNT 4 /* every second insert a PSI, so it takes 4 secs to circle; hopefully its transfer won't be delay that far */
    void *pat[IP_STREAMER_PSI_QUEUE_CNT];
    void *pmt[IP_STREAMER_PSI_QUEUE_CNT];
    unsigned ccValue;
    B_MutexHandle mutexSystemData;
    B_SchedulerHandle schedulerSystemData;
    B_SchedulerTimerId systemDataTimer;
    B_ThreadHandle schedulerThread;
    bool systemDataTimerIsStarted;
    NEXUS_StreamMuxSystemData psi[2];
    BKNI_EventHandle finishEvent;  /* set when Finished callback is received from Mux mgr */
    bool inUse;
    int refCount;
    bool started; /* set when transcoding is started on this context */
    int contextId; /* transcoder context id */
    unsigned segmentCount;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_AudioDecoderStartSettings audioProgram;
} TranscoderDst;
#endif /* NEXUS_HAS_VIDEO_ENCODER */

#define MAX_PROGRAMS_PER_FREQUENCY 24
#if NEXUS_HAS_FRONTEND
#define SATFILE_MAX_LEN 64
#define SATFILE_FORMAT "brcmsat.%lx"
typedef struct QamSrc {
    NEXUS_FrontendHandle frontendHandle;
    uint32_t frequency;
    int refCount;   /* how many streaming sessions are using this frontend */
    B_PlaybackIpPsiInfo psi[MAX_PROGRAMS_PER_FREQUENCY];
    int numProgramsFound; /* how many sub-channels are found in this frequency */
    bool psiAcquiring; /* set if PSI info is been acquired */
    bool skipFrontendTuning; /* set if frontend tuner is already setup during PSI discovery */
    BKNI_EventHandle psiAcquiredEvent;  /* set when PSI aquisition is completed */
    BKNI_EventHandle signalLockedEvent;  /* set when tuner locks in to the frequency */
    BKNI_EventHandle signalSpecADataRdy; /* set when all the spectrum analyzer data has been received*/
    int totalFftSamples; /* total number of FFT samples received for spectrum analyzer */
    uint32_t *fftData; /* pointer to the array that stroes the FFT data for spectrum analyzer */
    bool started; /* set when Src is started */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one src when it shared between multiple clients */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this qam frontend */
#endif
} QamSrc;

typedef struct SatSrc {
    NEXUS_FrontendHandle frontendHandle;
    uint32_t frequency;
    int refCount;   /* how many streaming sessions are using this frontend */
    B_PlaybackIpPsiInfo psi[MAX_PROGRAMS_PER_FREQUENCY];
    int numProgramsFound; /* how many sub-channels are found in this frequency */
    bool psiAcquiring; /* set if PSI info is been acquired */
    bool skipFrontendTuning; /* set if frontend tuner is already setup during PSI discovery */
    BKNI_EventHandle psiAcquiredEvent;  /* set when PSI aquisition is completed */
    BKNI_EventHandle signalLockedEvent;  /* set when tuner locks in to the frequency */
    bool started; /* set when Src is started */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one src when it shared between multiple clients */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this qam frontend */
#endif
} SatSrc;

typedef struct OfdmSrc {
    NEXUS_FrontendHandle frontendHandle;
    uint32_t frequency;
    int refCount;   /* how many streaming sessions are using this frontend */
    B_PlaybackIpPsiInfo psi[MAX_PROGRAMS_PER_FREQUENCY];
    int numProgramsFound; /* how many sub-channels are found in this frequency */
    bool psiAcquiring; /* set if PSI info is been acquired */
    bool skipFrontendTuning; /* set if frontend tuner is already setup during PSI discovery */
    BKNI_EventHandle psiAcquiredEvent;  /* set when PSI aquisition is completed */
    BKNI_EventHandle signalLockedEvent;  /* set when tuner locks in to the frequency */
    bool started; /* set when Src is started */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one src when it shared between multiple clients */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this qam frontend */
#endif
} OfdmSrc;
#endif /* NEXUS_HAS_FRONTEND */

typedef struct StreamerSrc {
    int refCount;   /* how many streaming sessions are using this frontend */
    B_PlaybackIpPsiInfo psi[MAX_PROGRAMS_PER_FREQUENCY];
    int numProgramsFound; /* how many sub-channels are found in this frequency */
    bool psiAcquiring; /* set if PSI info is been acquired */
    bool skipFrontendTuning; /* set if frontend tuner is already setup during PSI discovery */
    BKNI_EventHandle psiAcquiredEvent;  /* set when PSI aquisition is completed */
    BKNI_EventHandle signalLockedEvent;  /* set when tuner locks in to the frequency */
    bool started; /* set when Src is started */
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one src when it shared between multiple clients */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this streamer source */
#endif
} StreamerSrc;

typedef struct IpSrc {
    B_PlaybackIpHandle playbackIp;
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_PlaypumpHandle playpump;
#endif /* DMS_CROSS_PLATFORMS */
    char srcIpAddress[256];     /* IP Address of Receiving Host */
    int srcPort;     /* Port # */
    B_PlaybackIpProtocol srcProtocol; /* Protocol: UDP/RTP */
    int refCount;   /* how many streaming sessions are using this frontend */
    B_PlaybackIpPsiInfo psi[MAX_PROGRAMS_PER_FREQUENCY];
    int numProgramsFound; /* how many sub-channels in this IP tuner */
    bool psiAcquiring; /* set if PSI info is been acquired */
    BKNI_EventHandle psiAcquiredEvent;  /* set when PSI aquisition is completed */
    bool started; /* set when Src is started */
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this parser band */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    TranscoderDst *transcoderDst; /* handle to the transcoder context */
    IpStreamerTranscodeConfig transcode;
#endif
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one src when it shared between multiple clients */
} IpSrc;

typedef struct ParserBand {
    NEXUS_ParserBand parserBand;
    uint32_t frequency;
    int subChannel;
    int refCount;
    BKNI_MutexHandle lock;  /* lock to synchronize simultaneous access to one parser band when it shared between multiple clients */
    bool transcodeEnabled;  /* set when the 1st transcoding session is started for this parser band */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    TranscoderDst *transcoderDst; /* handle to the transcoder context */
    IpStreamerTranscodeConfig transcode;
#endif
} ParserBand;

typedef struct IpDst {
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_RecpumpHandle recpumpHandle;
    NEXUS_RecpumpHandle recpumpAllpassHandle;
#ifdef NEXUS_HAS_RECORD
    NEXUS_RecordHandle recordHandle;
    NEXUS_FileRecordHandle fileRecordHandle;
    NEXUS_FifoRecordHandle fifoFileHandle;
#endif
    NEXUS_Timebase timebase;
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle;
#endif /* DMS_CROSS_PLATFORMS */
    bool inUse;
    BKNI_EventHandle dataReadyEvent; /* indicates when data is available in the recpump buffer */
} IpDst;

typedef struct RecDst {
#ifdef NEXUS_HAS_RECORD
    NEXUS_RecpumpHandle recpumpHandle;
    NEXUS_RecordHandle recordHandle;
    NEXUS_FileRecordHandle fileRecordHandle;
#endif
    bool inUse;
} RecDst;

#ifdef NEXUS_HAS_HDMI_INPUT
typedef struct HdmiSrc {
    int hdmiInputId;
    NEXUS_Timebase timebase;
    NEXUS_HdmiInputHandle hdmiInput;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    TranscoderDst *transcoderDst; /* handle to the transcoder context */
#endif
    bool inUse;
    int refCount;
} HdmiSrc;
#endif

typedef struct FileSrc {
#ifdef NEXUS_HAS_VIDEO_ENCODER
    TranscoderDst *transcoderDst; /* handle to the transcoder context */
#endif
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlaypumpHandle playpumpHandle;
    NEXUS_PlaybackHandle playbackHandle;
    NEXUS_FilePlayHandle filePlayHandle;
    NEXUS_StcChannelHandle playbackStcChannel;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_Timebase timebase;
#endif
    B_PlaybackIpFileStreamingHandle fileStreamingHandle;
    bool inUse;
    int refCount;
} FileSrc;

/* state info common to all threads */
typedef struct IpStreamerGlobalCtx {
    IpStreamerGlobalCfg globalCfg;
    void *dtcpCtx;

#if NEXUS_HAS_FRONTEND
    /* list of all available vsb sources: one per vsb tuner */
    BKNI_MutexHandle vsbSrcMutex;
    QamSrc vsbSrcList[NEXUS_MAX_FRONTENDS];

    /* list of all available qam sources: one per qam tuner */
    BKNI_MutexHandle qamSrcMutex;
    QamSrc qamSrcList[NEXUS_MAX_FRONTENDS];

    /* list of all available sat sources: one per sat tuner */
    BKNI_MutexHandle satSrcMutex;
    SatSrc satSrcList[NEXUS_MAX_FRONTENDS];

    /* list of all available ofdm sources: one per ofdm tuner */
    BKNI_MutexHandle ofdmSrcMutex;
    OfdmSrc ofdmSrcList[NEXUS_MAX_FRONTENDS];
#endif /* NEXUS_HAS_FRONTEND */

    /* list of all available ip sources: one per playback channel */
    BKNI_MutexHandle ipSrcMutex; /* lock to synchronize access to the list of ip srcs */
#ifndef DMS_CROSS_PLATFORMS
    IpSrc ipSrcList[IP_STREAMER_NUM_PLAYPUMPS_FOR_LIVE_IP];
#if NEXUS_NUM_PARSER_BANDS
    ParserBand parserBandList[NEXUS_NUM_PARSER_BANDS];
    StreamerSrc streamerSrcList[NEXUS_NUM_PARSER_BANDS];
#else
    ParserBand parserBandList[IP_STREAMER_NUM_PARSER_BANDS];
    StreamerSrc streamerSrcList[IP_STREAMER_NUM_PARSER_BANDS];
#endif
    IpDst ipDstList[NEXUS_NUM_RECPUMPS];
    RecDst recDstList[NEXUS_NUM_RECPUMPS/2];
#endif /* DMS_CROSS_PLATFORMS */
    /* list of all available pid parsers */
    BKNI_MutexHandle parserBandMutex;
    /* list of all available streamer sources: one per parser band */
    BKNI_MutexHandle streamerSrcMutex;
    /* list of all available Ip destinations: one per rave context */
    BKNI_MutexHandle ipDstMutex; /* lock to synchronize access to the list of ip srcs */
    /* list of all available record destinations: one per rave context */
    BKNI_MutexHandle recDstMutex;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    /* list of all available xcode destinations */
    BKNI_MutexHandle transcoderDstMutex;
    TranscoderDst transcoderDstList[NEXUS_NUM_VIDEO_ENCODERS];
#endif

#ifdef NEXUS_HAS_HDMI_INPUT
    BKNI_MutexHandle hdmiSrcMutex;
    HdmiSrc hdmiSrcList[NEXUS_NUM_HDMI_INPUTS];
#endif

    /* following fields are only used when IP Streamer is used in the standalone mode (i.e. w/o DLNA DMS) */
    /* listening socket's fd */
    int listeningFd;
#ifdef B_USE_HTTP_KEEPALIVE
    fd_set rfds;
    int maxFd;
#endif
} IpStreamerGlobalCtx;

/* per thread state info */
typedef struct IpStreamerCtx
{
    IpStreamerGlobalCtx *globalCtx;
    IpStreamerConfig cfg;
    IpStreamerSrc srcType;
    B_PlaybackIpSocketState socketState;
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle patPidChannel;
    NEXUS_PidChannelHandle pmtPidChannel;
    NEXUS_PidChannelHandle allPassPidChannel;
#endif /* DMS_CROSS_PLATFORMS */
#ifdef STREAMER_CABLECARD_SUPPORT
    NEXUS_PidChannelHandle caPidChannel;
#endif
#if NEXUS_HAS_FRONTEND
    QamSrc *qamSrc;
    QamSrc *vsbSrc;
    SatSrc *satSrc;
    OfdmSrc *ofdmSrc;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendVsbSettings vsbSettings;
    NEXUS_FrontendOfdmSettings ofdmSettings;
#endif /* NEXUS_HAS_FRONTEND */
    IpSrc *ipSrc;   /* either ipSrc or qamSrc point is valid based on srcType */
    StreamerSrc *streamerSrc;
    FileSrc *fileSrc;
    IpDst *ipDst;
#ifdef NEXUS_HAS_HDMI_INPUT
    HdmiSrc *hdmiSrc;
#endif
    ParserBand *parserBandPtr;
    RecDst *recDst;
    uint64_t curBytesReceived;  /* # of bytes received from a live channel: either streamed on IP or recorded locally */
    int underflowWarningCount;  /* how many seconds we haven't received any new data in the streaming/recording buffer */

    bool ipStreamingInProgress; /* set when we are streaming a live channel to an IP client */
    bool fileStreamingInProgress; /* set when we are streaming a media file to an IP client */
    bool localRecordingInProgress; /* set when we are recording to local disk */
    bool overflow; /* set when we overflow happeneded */
    bool skipPsiAcquisition; /* if set, skip PSI acquisition, gets set when same channel is re-tuned */
#ifdef B_HAS_DTCP_IP
    void *AkeHandle;
    void *dtcpCtx;
#endif
    bool transcodeEnabled;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    TranscoderDst *transcoderDst;
    NEXUS_PidChannelHandle transcodeVideoPidChannel;
    NEXUS_PidChannelHandle transcodeAudioPidChannel;
    NEXUS_PidChannelHandle transcodePcrPidChannel;
    NEXUS_PidChannelHandle transcodePatPidChannel;
    NEXUS_PidChannelHandle transcodePmtPidChannel;
    bool transcodingInProgress; /* set when we are transcoding a session */
#endif
#ifdef NEXUS_HAS_SECURITY
    NEXUS_KeySlotHandle pvrEncKeyHandle;
    NEXUS_KeySlotHandle pvrDecKeyHandle;
#endif
    NEXUS_PidChannelHandle pidChannelList[IP_STREAMER_MAX_PIDS_PER_PROGRAM];
    BKNI_EventHandle statusEvent;  /* set when ip library is done streaming the file or live channel */
    BLST_Q_ENTRY(IpStreamerCtx) next; /* next ipStreamerCtx using HLS Session */
    bool waitingForNextRequest; /* flag to indicate this context is done streaming a segment and now waiting for an event to resume streaming of the next segment */
    B_PlaybackIpEventIds eventId; /* eventid to use for this purpose */
    struct sockaddr_in remoteAddr; /* address of the remote client */
    unsigned hlsNextSegmentNum; /* segment number of the next segment to send */
    char *urlPtr; /* cached URL ptr */
    IpStreamerSettings sessionSettings;
    bool pendingReqForNextSegment; /* set when a thread receieves request for next segment before another thread is done sending current segment for the same hls session */
    BKNI_EventHandle currentSegmentSentEvent;  /* set when 1st thread is done sending the current segment and another thread has received req for the next segment */
    unsigned int PlayCount;
#ifdef B_USE_HTTP_KEEPALIVE
    bool destroySession; /* set when live streaming thread is done streaming */
#endif
    bool resetTranscodePipe; /* sets when streaming lib callback indicates network error to send a segment due to client closing the connection */
    unsigned hlsCurrentSegmentNumber; /* segment number of the current segment being sent */
    unsigned hlsRequestedSegmentNumber; /* segment number of the current segment being sent */
    bool channelChange; /* new request is for different file/url, so tear-down this pipe and restart it */
    unsigned cookie; /* cookie to track HLS sessions */
    bool firstSegmentReqReceived; /* set after 1st segment request from a client is received */
    BKNI_MutexHandle lock;
    bool switchPlaylist; /* set when we have to switch the playlist */
    int nextSegmentStreamingFd; /* socket to which av content needs to be streamed to */
}IpStreamerCtx;
BLST_Q_HEAD(ipStreamerCtxQueue, IpStreamerCtx);
#endif /* __IP_STREAMER_H__ */
