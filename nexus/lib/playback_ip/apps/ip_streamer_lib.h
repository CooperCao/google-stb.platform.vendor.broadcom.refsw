/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *  ip streamer interface include file
 *
 ******************************************************************************/
#ifndef __IP_STREAMER_LIB_H__
#define __IP_STREAMER_LIB_H__

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#ifndef DMS_CROSS_PLATFORMS
#include "nexus_display.h"
#endif /* DMS_CROSS_PLATFORMS */


/**
Summary:
Callback functions to NEXUS server process required in multiprocess environment
**/
typedef int (*B_IpStreamer_RequestDecoder) (void);

typedef void (*B_IpStreamer_ReleaseDecoder) (int decoderIndex);

typedef bool (*B_IpStreamer_ConnectAudioDummy) (NEXUS_AudioDecoderHandle decoder, int audioDummyIndex);

typedef void (*B_IpStreamer_DisconnectAudioDummy) (int audioDummyIndex);



typedef enum {
    IpStreamerSrc_eIp, /* For streaming out content coming from IP frontend */
    IpStreamerSrc_eQam, /* For streaming out content coming from QAM frontend */
    IpStreamerSrc_eVsb, /* For streaming out content coming from VSB frontend (off-air) */
    IpStreamerSrc_eStreamer, /* For streaming out content coming from Streamer input */
    IpStreamerSrc_eFile, /* For streaming out pre-recorded content from local disk */
    IpStreamerSrc_eSat, /* For streaming out content coming from Satellite frontend */
    IpStreamerSrc_eHdmi, /* For streaming out encoded content coming from HDMI in (BlueRay player) */
    IpStreamerSrc_eOfdm, /* For streaming out encoded content coming from OFDM-capable frontends */
    IpStreamerSrc_eMax /* Max allowed enum */
}IpStreamerSrc;

typedef struct IpStreamerStreamingOutCfg {
    char *streamingIpAddress; /* remote IP address to stream to */
    int streamingPort;  /* remote Port # to stream to */
    B_PlaybackIpProtocol streamingProtocol; /* Protocol: UDP/RTP */
    char *url;  /* url (file name) for RTP/UDP streaming */
} IpStreamerStreamingOutCfg;

typedef struct IpStreamerGlobalCfg{
    bool multiProcessEnv; /*set to true when running with trellis*/
    int dtcpAkePort;  /* dtcp-ip port to receive AKE requests on */
    int dtcpKeyFormat;  /* DTCP-IP key format*/
    bool ckc_check;     /* Enable content key confirmation for sink device */
    bool slaveMode; /* set for VMS, allows sharing of Nexus handles between two independent Apps */
    bool printStats;
    char *serverIpAddress;
    char interfaceName[16];
    char timeshiftDirPath[32]; /* directory where timeshifted files are stored */
    int timeshiftBufferInterval; /* how many seconds worth of stream should be cached in the timeshift buffer */
    int maxBitRate; /* max bitrate for the live sessions */
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    NEXUS_DmaHandle dmaHandle; /* Nexus M2M DMA handle */
#endif
    NEXUS_HeapHandle heapHandle; /* nexus heap Handle, can be null if app doesn't have a preference */

    /* following options need to be set only when IP Streamer is run in standalone mode (i.e. outside of DLNA DMS) */
    int listeningPort;  /* port to receive HTTP requests on */
    bool accelSocket; /* once accelerated sockets are supported on 7420, this option can be enabled */
    char rootDir[64]; /* root directory */
    int numStreamingSessions; /* max # of streaming threads to start */
    int active_sessions;
    int disableFrontend; /* if set, frontends (qam/vsb/ip) are not used for live streaming */
#define IP_STREAMER_MAX_STREAMING_OUT  8 /*typically bound by number of demods. ipStreamerConfig.xml numStreams should be less or equal to this define*/
    IpStreamerStreamingOutCfg streamingCfg[IP_STREAMER_MAX_STREAMING_OUT];
    bool freeDmaHandle;

    bool multicastEnable;
    bool optimizedHlsStreaming ;

    /*call back function to nexus server process*/
    B_IpStreamer_RequestDecoder requestDecoder;
    B_IpStreamer_ReleaseDecoder releaseDecoder;
    B_IpStreamer_ConnectAudioDummy connectAudioDummy;
    B_IpStreamer_DisconnectAudioDummy disconnectAudioDummy;

} IpStreamerGlobalCfg;

#ifdef NEXUS_HAS_VIDEO_ENCODER
typedef struct IpStreamerTranscodeConfig {
    int transportBitrate; /* desired transport bitrate of transcoded stream */
    NEXUS_TransportType transportType; /* Transport type of the transcoded stream */
    bool outVideo;  /* set if Video in enabled in transcoded stream */
    bool outAudio;  /* set if Audio is enabled in transcoded stream */
    int outVideoPid;
    int outPcrPid;
    int outAudioPid;
    NEXUS_VideoCodec outVideoCodec; /* video codec of transcoded stream */
    NEXUS_DisplayAspectRatio outAspectRatio;
    NEXUS_VideoFrameRate outFrameRate;
    unsigned outWidth;
    unsigned outHeight;
    NEXUS_AudioCodec outAudioCodec; /* audio codec of transcoded stream */
    int outPatPid;
    int outPmtPid;
    bool outInterlaced;
    bool nonRealTime;
}IpStreamerTranscodeConfig;
#endif

/* App specific Header that gets inserted before media is streamed out */
typedef struct {
    bool valid; /* set if appHeader is valid */
    unsigned length; /* length of app header bytes: should be mod16 aligned if it needs to be encrypted */
    uint8_t data[192]; /* app header */
} IpStreamerAppHeader;

/* per session configuration */
typedef struct IpStreamerConfig{
    IpStreamerSrc srcType;

    bool mediaProbeOnly;     /* set this flag if the ip streaming context is being used for determining media probe info only */
    bool skipPsiAcquisition; /* set this flag if app already has PSI information for this file/channel */
    B_PlaybackIp_EventCallback eventCallback; /* callback function to receive any asynchronous events from the IP library */
    void *appCtx;
#ifdef NEXUS_HAS_FRONTEND
    /* QAM Src related settings */
    NEXUS_FrontendQamMode qamMode;  /* qam modulation mode */
    NEXUS_FrontendQamAnnex qamAnnex;
    NEXUS_FrontendSatelliteMode satMode;  /* sat mode */
    int diseqcVoltage;
    int symbolRate;
    bool toneEnabled;
    bool verticalPolarization;
    int fec;
    int newAdc; /*remap ADC mapping for this demod to adc #. Note: (for most 7445DBS boards, this needs to be 1) */
    bool isNewAdc;
    /* VSB Src related settings */
    NEXUS_FrontendVsbMode vsbMode;
    /* OFDM related config */
    int bandwidth;
    NEXUS_FrontendOfdmMode ofdmMode;
    NEXUS_FrontendOfdmCciMode cciMode;
    NEXUS_FrontendOfdmPriority ofdmPriority;
    bool terrestrial;
    int srcPosition;
    int srcFe;
#endif
    uint32_t frequency; /* frequency in Mhz (e.g. 333) */
    int subChannel; /* sub channel number: starts from 1 */
#ifdef STREAMER_CABLECARD_SUPPORT
    uint32_t sourceId;
#endif
    /* IP Src related settings */
    int iphVersion;
    char srcIpAddress[256]; /* IP Address of Receiving Host */
    int srcPort; /* Port # */
    char interfaceName[16]; /* Interface to send multicast join requests on */
    B_PlaybackIpProtocol srcProtocol; /* Protocol: UDP/RTP */
    bool accelSocket; /* once accelerated sockets are supported on 7420, this option can be enabled */

    B_PlaybackIpSecurityProtocol security;  /* which security protocol to use */

    /* File Source related settings */
    off_t beginFileOffset;
    off_t endFileOffset;
    bool fileOffsetEnabled; /* set if byte Range is requested */
    int playSpeed;  /* speed at which to stream the file */
    double beginTimeOffset; /* stream file starting from these many seconds */
    double endTimeOffset;

    /* Record Destination related settings */
    bool recDstEnabled; /* if set, local recording is enabled */

    /* Note: local data is being recorded in clear */
    char fileName[128];  /* name of the file where av data should be recorded to or streamed from */
    char indexFileName[128]; /* name of the file where av data index should be written to or used for streaming */
    char mediaInfoFilesDir[128];  /* directory name where the info & nav files should be created */

    /* IP Destination related settings */
    bool ipDstEnabled; /* if set, live channel is streamed over the network */
    int streamingFd; /* socket to which av content needs to be streamed to */
    int streamingFdLocal; /* socket fd on which to stream data for local client */
    bool useLiveIpMode; /* true: Push mode (locks to incoming PCR); false: Pull Mode (receiver controls the sender) */
    bool encryptionEnabled; /* if set, streaming data is encrypted as per the Dst (DTCP/IP for Ip Dst & Plain AES for Local Rec Dst) */
    bool pvrEncryptionEnabled; /* if set, encrypt stream before writing to timeshift fifo & decrypt it before DTCP/IP */
    bool pvrDecryptionEnabled; /* if set, encrypt stream before writing to timeshift fifo & decrypt it before DTCP/IP */
    void *pvrDecKeyHandle;   /* set if PvrDecEnabled field is set in the URL */
    int emiValue; /* EMI value for DTCP/IP headers */
    bool transcodeEnabled; /* if set, transcoding is enabled */
    bool usePlaybackForStreamingFiles;
    bool enableTimeshifting; /* enables timeshifting of live channels */
    bool enableTimestamps; /* enables 4 byte binary timestamp in the recording of the live streams */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    IpStreamerTranscodeConfig transcode;
#endif
#ifdef NEXUS_HAS_HDMI_INPUT
    int hdmiInputId; /* index of HDMI input to use */
#endif

    IpStreamerStreamingOutCfg streamingCfg; /* contains configuration for streaming out using RTP/UDP protocols */
    IpStreamerAppHeader appHeader;     /* app specific binary data */
    bool hlsSession;         /* set if this is a HLS session */
    bool dashSession;        /* set to help change paramaters for dash that are different from hls */
    int hlsSegmentSize;      /* size of a hls segment in bytes */
    bool autoRewind;        /* if set, automatically rewind file when eof is reached */

    bool satIpProtocolSession; /* set to true if this session follows the ASTRA's Sat/IP Protocol Spec */
#define IP_STREAMER_MAX_PIDS_PER_PROGRAM 16 /* should be enough for SPTS program */
    uint16_t pidList[IP_STREAMER_MAX_PIDS_PER_PROGRAM]; /* list of pids: sat/ip client can provide the list of pids it wants server to demux & stream out */
    int pidListCount; /* count of sat/ip client provided list of pids to demux */
    bool enableAllpass; /* sat/ip client can even ask server to stream the whole mpts stream out to the client */
    bool noDemuxOutput; /* sat/ip client can ask server to just setup the pipe and not demux & send any streams to the client */
    bool pilotToneSpecified; /* true if client specified a value for the pilotTone */
    bool pilotTone; /* client can set this to be on or off */

    unsigned long int * ipStreamerCtx;
    unsigned long int * liveStreamingCtx;

    /* following options need to be set only when IP Streamer is run in standalone mode (i.e. outside of DLNA DMS) */
    bool headRequest; /* set when client sends HTTP HEAD request */
} IpStreamerConfig;

/* Initializes IP Streamers' global context for the given configuration */
void * B_IpStreamer_Init(IpStreamerGlobalCfg *ipStreamerGlobalCfg);
void B_IpStreamer_UnInit(void *ipStreamerGlobalCtx);

typedef struct IpStreamerOpenSettings {
    int streamingFd;         /* socket file descriptor using which live/file content should be streamed, not used for recording only or media probing case */
    int streamingFdLocal; /* socket fd on which to stream data for local client */
    char *requestUri;        /* URI associated with this streaming context: identifies live vs file streaming, recording on/off */
    bool mediaProbeOnly;     /* set this flag if the ip streaming context is being used for determining media probe info only */
    bool skipPsiAcquisition; /* set this flag if app already has PSI information for this file/channel */
    B_PlaybackIp_EventCallback eventCallback; /* callback function to receive any asynchronous events from the IP library */
    void *appCtx;
    void *pvrDecKeyHandle;   /* set if PvrDecEnabled field is set in the URL */
    bool hlsSession;         /* set if this is a HLS session */
    int hlsSegmentSize;      /* size of a hls segment in bytes */
    IpStreamerAppHeader appHeader;     /* app specific binary data */
    bool autoRewind;        /* if set, automatically rewind file when eof is reached */
    bool liveStreamingSession;        /* if set, session is live streaming (satellite) */
}IpStreamerOpenSettings;

/* Opens per session context for a give session configuration */
void *B_IpStreamer_SessionOpen(void *ipStreamerGlobalCtx, IpStreamerOpenSettings *openSettings);
void B_IpStreamer_SessionClose(void *ipStreamerCtx);

/* returns the PSI info associated with a live channel in psiOut */
int B_IpStreamer_SessionAcquirePsiInfo(void *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut);

/* Settings that can be updated when live streaming start is already called */
typedef struct IpStreamerSettings
{
    bool streamingEnabled; /* flag to indicate to live streaming thread that streaming has now been enabled (Fast Channel Change) */
    bool resumeStreaming; /* flag to indicate to live streaming thread to resume streaming for the new HLS segment */
    int streamingFd; /* socket fd on which to stream data on */
    bool xcodeModifyBitrate; /* set when app wants to dynamically modify the bitrate of current encoding session (w/o restarting it) */
    unsigned int xcodeBitrate; /* bitrate */
    bool hlsSession;         /* set if this is a HLS session */
    int hlsSegmentSize;      /* size of a hls segment in bytes */
    bool resetStreaming;     /* flag to reset the streaming pipe during a seek event */
} IpStreamerSettings;
/* Opens per session context for a give session configuration */
int B_IpStreamer_SessionSetSettings(void *ipStreamerCtx, IpStreamerSettings *sessionSettings);

/* Opens per session context for a give session configuration */
int B_IpStreamer_SessionStart(void *ipStreamerCtx, B_PlaybackIpPsiInfo *psi);
void B_IpStreamer_SessionStop(void *ipStreamerCtx);

typedef struct IpStreamerSessionStatus {
    bool active;       /* set as long as session is active (no underflow from source, client still alive & receiving, no recording error, etc.*/
    /* TODO: add more status fields like: socket state, bytes streamed, bytes recorded, etc. */
    int recordingDuration; /* duration in seconds of the session being recorded */
}IpStreamerSessionStatus;

/* returns true if Streaming session is successfully ongoing, false otherwise where upon app should stop & close streaming session */
void B_IpStreamer_SessionStatus(void *ipStreamerCtx, IpStreamerSessionStatus *status);

/* frontend statuses that can be returned */
typedef enum IpStreamer_FrontendMode
{
    /* satellite modes */
    IpStreamer_FrontendMode_eDvb,
    IpStreamer_FrontendMode_eDss,
    IpStreamer_FrontendMode_eDcii,
    IpStreamer_FrontendMode_eQpskTurbo,
    IpStreamer_FrontendMode_eTurboQpsk=IpStreamer_FrontendMode_eQpskTurbo,
    IpStreamer_FrontendMode_e8pskTurbo,
    IpStreamer_FrontendMode_eTurbo8psk=IpStreamer_FrontendMode_e8pskTurbo,
    IpStreamer_FrontendMode_eTurbo,
    IpStreamer_FrontendMode_eQpskLdpc,
    IpStreamer_FrontendMode_e8pskLdpc,
    IpStreamer_FrontendMode_eLdpc,
    IpStreamer_FrontendMode_eBlindAcquisition,

    /* cable modes */
    IpStreamer_FrontendMode_e16,
    IpStreamer_FrontendMode_e32,
    IpStreamer_FrontendMode_e64,
    IpStreamer_FrontendMode_e128,
    IpStreamer_FrontendMode_e256,
    IpStreamer_FrontendMode_e512,
    IpStreamer_FrontendMode_e1024,
    IpStreamer_FrontendMode_e2048,
    IpStreamer_FrontendMode_e4096,
    IpStreamer_FrontendMode_eAuto_64_256, /* Automatically scan both QAM-64 and QAM-256.
                                           Not available on all chipsets. */
    /* terrestrial modes */
    IpStreamer_FrontendMode_eDvbt,
    IpStreamer_FrontendMode_eDvbt2,
    IpStreamer_FrontendMode_eIsdbt,

    IpStreamer_FrontendMode_eMax
} IpStreamer_FrontendMode;

typedef enum IpStreamer_FrontendAnnex
{
    IpStreamer_FrontendQamAnnex_eA,
    IpStreamer_FrontendQamAnnex_eB,
    IpStreamer_FrontendQamAnnex_eMax
} IpStreamer_FrontendAnnex;

typedef enum IpStreamer_FrontendInversion
{
    IpStreamer_FrontendInversion_eScan,
    IpStreamer_FrontendInversion_eNormal,
    IpStreamer_FrontendInversion_eI,
    IpStreamer_FrontendInversion_eQ,
    IpStreamer_FrontendInversion_eMax
} IpStreamer_FrontendInversion;

typedef struct IpStreamerSoftDecision {
    int i;
    int q;
}IpStreamerSoftDecision;

/*returns 0 if successful, else return 1 */
int B_IpStreamer_GetFrontendSoftDecision(void * dlnaGlobalCtx, int frontendNo, IpStreamerSoftDecision *softDecision, int length);

typedef struct IpStreamerStatus {
    IpStreamer_FrontendMode mode;
    IpStreamer_FrontendInversion spectralInversion;

    unsigned codeRateNumerator; /* Code rate detected */
    unsigned codeRateDenominator;
    unsigned frequency;         /* actual tuner frequency */

    bool tunerLocked;           /* true if the tuner is locked */
    bool demodLocked;           /* true if the demodulator is locked */
    bool bertLocked;            /* true if the BER tester is locked.  If so, see berEstimate. */

    unsigned channel;           /* Channel number */
    unsigned symbolRate;        /* In baud */
    int symbolRateError;        /* In baud */

    int carrierOffset;          /* In Hz */
    int carrierError;           /* In Hz */
    unsigned sampleClock;       /* In Hz */
    unsigned outputBitRate;     /* Output bit rate in bps */

    unsigned ifAgcLevel;        /* IF AGC level in units of 1/10 percent */
    unsigned rfAgcLevel;        /* tuner AGC level in units of 1/10 percent */
    unsigned intAgcLevel;       /* Internal AGC level in units of 1/10 percent */
    unsigned snrEstimate;       /* SNR in 1/100 dB */
    unsigned berEstimate;       /* Bit error rate as log10 of 0.0-1.0 range.
                                    1.0  => 100% => 0
                                    0.1  => 10%  => -1
                                    0.01 => 1%   => -2
                                    0    => 0%   => 1 (special value for NONE)
                                    If bertLocked == false, it's set to 1. */

    unsigned fecPhase;          /* 0, 90, 180, 270 */
    unsigned fecCorrected;      /* cumulative block correctable errors */
    unsigned fecUncorrected;    /* cumulative block uncorrectable errors */
    unsigned fecClean;          /* cumulative clean block count */
    unsigned bitErrCorrected;   /* cumulative bit correctable errors */
    unsigned reacquireCount;    /* cumulative reacquisition count */
    unsigned berErrors;         /* BER error count - valid if bertLocked is true */
    unsigned preViterbiErrorCount;    /* accumulated pre-Viterbi error count */
    unsigned mpegErrors;        /* mpeg frame error count */
    unsigned mpegCount;         /* total mpeg frame count */
    unsigned ifAgc;             /* if agc value from hw unmodified */
    unsigned rfAgc;             /* rf agc value from hw unmodified */
    unsigned agf;               /* AGF integrator value */
    unsigned timeElapsed;       /* time elapsed in milliseconds since the last call to NEXUS_Frontend_ResetStatus.
                                   the elapsed time is measured at the same time that the rest of the values in NEXUS_FrontendSatelliteStatus are captured. */

    float power;

    unsigned chipId;
    unsigned chipVersion;
    unsigned bondingOption;
    unsigned apMicrocodeVersion;
    unsigned hostConfigurationVersion;

    /* cable specifics */
    bool receiverLock;
    bool fecLock;
    int dsChannelPower;
    IpStreamer_FrontendAnnex annex;
}IpStreamerStatus;

int B_IpStreamer_GetFrontendStatus(void * dlnaGlobalCtx, int frontendNo, IpStreamerSrc *src, IpStreamerStatus *status);

typedef struct IpStreamerSpecAParams
{
    int channel;            /* frontendNo */
    uint32_t fStart;
    uint32_t fStop;
    uint32_t fftSize;
    uint32_t numOfAvgBins;
    uint32_t numOfSamples;
    uint32_t *data;         /* pointer to the array where to store the data */
} IpStreamerSpecAParams;

int B_IpStreamer_SpecAOpen(void * dlnaGlobalCtx, int frontendNo, int* selectedFrontendNo);
int B_IpStreamer_SpecAClose(void * dlnaGlobalCtx, int frontendNo);
int B_IpStreamer_GetFrontendSpecAData(void * dlnaGlobalCtx, int frontendNo, IpStreamerSpecAParams *specAParams);

#ifdef NEXUS_HAS_VIDEO_ENCODER
bool B_IpStreamer_InsertPatPmtTables(void *ipStreamerCtx);
#endif

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && (NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==1)
extern NEXUS_KeySlotHandle _createKeyHandle(NEXUS_SecurityOperation operationType);
#endif
extern int getEnvVariableValue(char *pName, unsigned long defaultValue);



#endif /* __IP_STREAMER_LIB_H__ */
