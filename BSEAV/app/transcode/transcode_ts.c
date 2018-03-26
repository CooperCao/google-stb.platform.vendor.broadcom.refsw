/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
/* nexus unittest: transcode file/qam/hdmi source to ts output */

#include "nexus_platform.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_video_input.h"
#include "nexus_core_utils.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_graphics2d.h"
#if NEXUS_HAS_STREAM_MUX
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_file_fifo.h"
#include "tshdrbuilder.h"
#include "b_os_lib.h"
#include "nexus_audio_mixer.h"
#endif
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#define USE_DDRE    0 /* set to 1 to enable DDRE&DV258 for multi-channel MS-11 -> AAC xcode test */

#if USE_DDRE
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "bstd.h"
#include "bkni.h"
#include "namevalue.inc"

#define BTST_ENABLE_FAST_STOP_START 1

/* matching audio xcoders per DSP core with video */
#if NEXUS_NUM_VCE_CHANNELS
#define BTST_NUM_AUDIO_XCODE_PER_DSP  NEXUS_NUM_VCE_CHANNELS
#else
#define BTST_NUM_AUDIO_XCODE_PER_DSP  2
#endif

/* stop-start iterations */
#define TEST_ITERATIONS 3

/* NRT a+v transcode STC av_window enable test */
#define BTST_ENABLE_NRT_STC_AV_WINDOW 1

#if NEXUS_HAS_SYNC_CHANNEL
#define BTST_ENABLE_AV_SYNC 1
#else
#define BTST_ENABLE_AV_SYNC 0
#endif

/* to debug TS mux pacing */
#define BTST_DISABLE_VIDEO_ENCODER_PES_PACING 0

/***********************************
 *   Display assignments
 */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
/* source display locally as SD CVBS output by default */
#define BTST_SOURCE_DISPLAY_IDX     0
/* xcode loopback display as HD hdmi/component output by default */
#define BTST_LOOPBACK_DISPLAY_IDX   0
#else
/* source display locally as SD CVBS output by default */
#define BTST_SOURCE_DISPLAY_IDX     1
/* xcode loopback display as HD hdmi/component output by default */
#define BTST_LOOPBACK_DISPLAY_IDX   0
#endif

static unsigned get_display_index(unsigned encoder)
{
    static bool set = false;
    static NEXUS_VideoEncoderCapabilities cap;
    if (!set) {
        NEXUS_GetVideoEncoderCapabilities(&cap);
        set = true;
    }
    if (encoder < NEXUS_MAX_VIDEO_ENCODERS && cap.videoEncoder[encoder].supported) {
        return cap.videoEncoder[encoder].displayIndex;
    }
    return 0;
}

/***********************************
 *   Source ID assignments
 */
#define BTST_RESOURCE_FILE       0
#define BTST_RESOURCE_HDMI       1
#define BTST_RESOURCE_QAM        2

/***********************************
 *   PID assignments
 */
#define BTST_MUX_PCR_PID        (0x11)
#define BTST_MUX_VIDEO_PID      (0x12)
#define BTST_MUX_AUDIO_PID      (0x13)
#define BTST_MUX_USER_DATA_PID  (BTST_MUX_AUDIO_PID+6)
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)

/***********************************
 *   Playpump assignments
 */
#define BTST_LOOPBACK_PLAYPUMP_IDX           0
#define BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX  (NEXUS_NUM_VIDEO_ENCODERS)
#define BTST_STREAM_MUX_AUDIO_PLAYPUMP_IDX  (NEXUS_NUM_VIDEO_ENCODERS+1)
#define BTST_STREAM_MUX_PCR_PLAYPUMP_IDX    (NEXUS_NUM_VIDEO_ENCODERS+2)
#define BTST_SOURCE_PLAYPUMP_IDX            (NEXUS_NUM_VIDEO_ENCODERS+3)

/***********************************
 *   Video Decoder assignments
 */
/* TODO: to query nexus capabilities for usable video decoders */
#define BTST_LOOPBACK_VID_DECODE_IDX    (0)
#define BTST_LOOPBACK_AUD_DECODE_IDX    (NEXUS_NUM_AUDIO_DECODERS-1)

/***********************************
 *   STC assignments (2 per xcoder)
 */
#if BCHP_CHIP != 7445
#define BTST_XCODE_VIDEO_STC_IDX(contextId) (NEXUS_ANY_ID)
#define BTST_XCODE_OTHER_STC_IDX(contextId) (NEXUS_ANY_ID)
#define BTST_LOOPBACK_STC_IDX               (NEXUS_ANY_ID)
#else/* SW7445-111: XVD fw workaround supports up to lower 6 STCs */

/* SW7445-159: STC8/9 broadcast are wrong */
#if BCHP_VER < BCHP_VER_C0
#define BTST_SW7445_159_STC_WORKAROUND       1
#else
#define BTST_SW7445_159_STC_WORKAROUND       0
#endif

#if BTST_SW7445_159_STC_WORKAROUND
#define BTST_XCODE_VIDEO_STC_IDX(contextId) (NEXUS_NUM_VIDEO_ENCODERS-contextId-1)
#define BTST_XCODE_OTHER_STC_IDX(contextId) (NEXUS_NUM_VIDEO_ENCODERS+contextId)
#else
#define BTST_XCODE_VIDEO_STC_IDX(contextId) (NEXUS_NUM_VIDEO_DECODERS-contextId-1)
#define BTST_XCODE_OTHER_STC_IDX(contextId) (NEXUS_NUM_VIDEO_DECODERS+contextId)
#endif
#define BTST_LOOPBACK_STC_IDX               (0)
#endif

/***********************************
 *   Closed Caption user data count
 */
#define BTST_MAX_CC_DATA_COUNT          32

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define BTST_PSI_QUEUE_CNT 4 /* every second insert a PSI, so it takes 4 secs to circle; hopefully its transfer won't be delay that far */

/***********************************
 *   TS user data all pass
 */
#define BTST_TS_USER_DATA_ALL           (unsigned)(-1)
#define BTST_MAX_MESSAGE_FILTERS        20

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#define BTST_INPUT_MAX_WIDTH 416
#define BTST_INPUT_MAX_HEIGHT 224
#else
#define BTST_INPUT_MAX_WIDTH 1280
#define BTST_INPUT_MAX_HEIGHT 720
#endif
#endif

BDBG_MODULE(transcode_ts);
BDBG_FILE_MODULE(userdataCb);
BDBG_FILE_MODULE(contcounter);

#if NEXUS_HAS_STREAM_MUX
typedef struct EncodeSettings {
    char                    fname[256];
    NEXUS_VideoFormat       displayFormat;
    NEXUS_VideoFrameRate    encoderFrameRate;
    unsigned                encoderBitrate;
    unsigned                encoderTargetBitrate;
    bool                    vbr;
    unsigned                encoderGopStructureFramesP;
    unsigned                encoderGopStructureFramesB;
    unsigned                gopDuration;
    bool                    adaptiveDuration;
    bool                    openGop;
    bool                    newGopOnSceneChange;
    bool                    startGopOnSceneChange;
    bool                    singleRefP;
    bool                    requiredPatchesOnly;
    NEXUS_VideoCodec        encoderVideoCodec;
    NEXUS_VideoCodecProfile encoderProfile;
    NEXUS_VideoCodecLevel   encoderLevel;
    NEXUS_EntropyCoding     entropyCoding;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    bool                    bCustom;
    bool                    bAudioEncode;
    NEXUS_AudioCodec        encoderAudioCodec;

    bool                    enableFieldPairing;
    bool                    disableFrameDrop;

    /* 0 to use default 750ms rate buffer delay; allow user to adjust it to lower encode delay at cost of quality reduction! */
    unsigned                rateBufferDelay;
    struct {
       bool enable;
       unsigned duration; /* duration of the segment (in ms) */
       unsigned upperTolerance; /* percentage tolerance above the target bitrate, between 0 and 100 */
       unsigned lowerTolerance; /* percentage tolerance below the target bitrate, between 0 and 100 */
    } segmentRC; /* segment based rate control */

    /* Note:
       1. higher minimum input and output framerate means lower encode delay
       2. lower max resolution for lower encode delay
       3. higher bitrate bounds means lower encoder delay
       4. zero framesB bound means lower encode delay */
    NEXUS_VideoEncoderBounds bounds;

    /* 3D settings */
    bool                       b3D;
    NEXUS_VideoOrientation     display3dType;
    bool                       bInput3D;
    NEXUS_VideoOrientation     input3dType;

    /* video encoder memory config */
    struct {
        bool               progressiveOnly;
        unsigned           maxWidth, maxHeight;
    } videoEncoderMemConfig;
}  EncodeSettings;

typedef struct InputSettings{
    int                    resource;
    char                   fname[256];
    NEXUS_TransportType    eStreamType;
    NEXUS_VideoCodec       eVideoCodec;
    NEXUS_AudioCodec       eAudioCodec;
    NEXUS_AudioMultichannelFormat eMultiChanFmt;
    bool                   bPcmAudio;
    bool                   probe;
    bool                   bUseStreamAsIndex;
    unsigned               program;
    unsigned               iVideoPid;
    unsigned               iAudioPid;
    unsigned               iSecondaryPid;
    unsigned               iPcrPid;
    unsigned               pmtPid;
    unsigned               userDataPid[NEXUS_MAX_MUX_PIDS];
    unsigned               remapUserDataPid[NEXUS_MAX_MUX_PIDS];
    size_t                 numUserDataPids;
    int                    freq;
#if  NEXUS_HAS_FRONTEND
    NEXUS_FrontendQamMode  qamMode;
#endif
    bool                   bTsUserDataInput;
    bool                   bAudioInput;
    bool                   bVideoWindowDisabled;
    bool                   bConfig;
} InputSettings;

typedef struct psi_message_t {
    unsigned short pid;
    NEXUS_MessageHandle message;
    NEXUS_PidChannelHandle pidChannel;
    bool done;
} psi_message_t;

typedef struct TranscodeContext {
    /* gfx */
    NEXUS_SurfaceHandle        surface;
    NEXUS_Graphics2DHandle     gfx;
    BKNI_EventHandle           checkpointEvent, spaceAvailableEvent;

    /* xcoder handles */
    NEXUS_VideoEncoderHandle   videoEncoder;
    NEXUS_DisplayHandle        displayTranscode;
    NEXUS_VideoWindowHandle    windowTranscode;
    NEXUS_PlaypumpHandle       playpumpTranscodeVideo;
    NEXUS_PidChannelHandle     pidChannelTranscodeVideo, pidRemuxVideo;
    NEXUS_PlaypumpHandle       playpumpTranscodeAudio;
    NEXUS_PidChannelHandle     pidChannelTranscodeAudio, pidRemuxAudio;
    NEXUS_AudioMixerHandle     audioMixer;
    NEXUS_AudioEncoderHandle   audioEncoder;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
#if USE_DDRE
    NEXUS_DolbyDigitalReencodeHandle ddre;
    NEXUS_DolbyVolume258Handle dv258;
#endif
    NEXUS_StreamMuxHandle      streamMux;
    BKNI_EventHandle           finishEvent;
    NEXUS_PlaypumpHandle       playpumpTranscodePcr;
    NEXUS_StcChannelHandle     stcChannelTranscode;

   NEXUS_PlaypumpHandle       playpumpTranscodeMCPB;

    /* xcoder mux/record handles */
    NEXUS_FifoRecordHandle     fifoFile;
    NEXUS_FileRecordHandle     fileTranscode;
    NEXUS_RecpumpHandle        recpump;
    NEXUS_RecordHandle         record;
    NEXUS_PidChannelHandle     pidChannelTranscodePcr, pidRemuxPcr;
    NEXUS_PidChannelHandle     pidChannelTranscodePat, pidRemuxPat;
    NEXUS_PidChannelHandle     pidChannelTranscodePmt, pidRemuxPmt;
    NEXUS_RemuxHandle          remux;
    bool                       bRemux;
    NEXUS_ParserBand           parserBandRemux;
    NEXUS_PidChannelHandle     pidRemuxUserData[NEXUS_MAX_MUX_PIDS];


    /* source local decode/display handles */
    NEXUS_StcChannelHandle     stcVideoChannel;
    NEXUS_StcChannelHandle     stcAudioChannel;
    NEXUS_DisplayHandle        display;
    NEXUS_VideoWindowHandle    window;

#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle      hdmiInput;
#endif
    NEXUS_FilePlayHandle       file;
    NEXUS_PlaypumpHandle       playpump;
    NEXUS_PlaybackHandle       playback;
    NEXUS_ParserBand           parserBand;
    NEXUS_PidChannelHandle     videoPidChannel;
    NEXUS_PidChannelHandle     pcrPidChannel;
    NEXUS_VideoDecoderHandle   videoDecoder;
    NEXUS_PidChannelHandle     audioPidChannel, secondaryPidChannel;
    NEXUS_AudioDecoderHandle   audioDecoder, secondaryDecoder;
#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelHandle    syncChannel;
#endif
    NEXUS_VideoDecoderStartSettings vidProgram;
    NEXUS_AudioDecoderStartSettings audProgram, secondaryProgram;

    /* xcode loopback player context */
    NEXUS_FilePlayHandle       filePlayLoopback;
    NEXUS_PlaybackHandle       playbackLoopback;
    NEXUS_VideoDecoderHandle   videoDecoderLoopback;
    NEXUS_StcChannelHandle     stcChannelLoopback;
    NEXUS_AudioDecoderHandle   audioDecoderLoopback;

    NEXUS_PidChannelHandle     videoPidChannelLoopback;
    NEXUS_DisplayHandle        displayLoopback;
    NEXUS_VideoWindowHandle    windowLoopback;
    NEXUS_PidChannelHandle     audioPidChannelLoopback;

    NEXUS_PlaypumpHandle       playpumpLoopback;
#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelHandle    syncChannelLoopback;
#endif
    /* video encoder settings */
    EncodeSettings             encodeSettings;
    unsigned                   encodeDelay;
    NEXUS_VideoEncoderStartSettings vidEncoderStartSettings;
    NEXUS_AudioMuxOutputStartSettings audMuxStartSettings;
    NEXUS_StreamMuxStartSettings muxConfig;
    bool                       bEncodeCCUserData;

    /* input settings */
    InputSettings              inputSettings;

    NEXUS_FrontendHandle       frontend;
    char                       indexfname[256];

    /* PSI system data */
    void                     *pat[BTST_PSI_QUEUE_CNT];
    void                     *pmt[BTST_PSI_QUEUE_CNT];
    unsigned                  ccValue;
    NEXUS_StreamMuxSystemData psi[2];
    B_MutexHandle             mutexSystemdata;
    B_SchedulerHandle         schedulerSystemdata;
    B_SchedulerTimerId        systemdataTimer;
    B_ThreadHandle            schedulerThread;
    bool                      systemdataTimerIsStarted;

    NEXUS_MessageHandle       userDataMessage[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle    pidChannelUserData[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle    pidChannelTranscodeUserData[NEXUS_MAX_MUX_PIDS];
    bool                      bUserDataStreamValid[NEXUS_MAX_MUX_PIDS];
    int                       userDataDescLen[NEXUS_MAX_MUX_PIDS];
    uint8_t                   userDataDescriptors[NEXUS_MAX_MUX_PIDS][188];
    TS_PMT_stream             userDataStream[NEXUS_MAX_MUX_PIDS];
    bool                      bRemapUserDataPid;
    psi_message_t             psi_message[BTST_MAX_MESSAGE_FILTERS];

    /* xcode system settings */
    bool                      bNonRealTime;
    bool                      bLowDelay;
    unsigned                  uiStopMode;
    bool                      bAdaptiveLatencyStart;
    bool                      bCustomizeDelay;
    B_EventHandle             eofEvent;
    B_ThreadHandle            nrtEofHandle;
    B_MutexHandle             mutexStarted;

    int                       contextId;
    bool                      bStarted;

    bool                      bNoStopDecode;
    bool                      bPrematureMuxStop;

    bool                      bSeamlessFormatSwitch;
    bool                      b1080pCaptureFormat;
    bool                      bVariableFrameRate;

    bool                      bNoVideo;
    bool                      bypassVideoFilter;
    bool                      bMCPB;
    bool                      dropBardata;
    NEXUS_StreamMuxInterleaveMode interleaveMode;

    struct
    {
       bool bEnable;
       uint32_t uiValue;
    } ptsSeed;

    bool bShuttingDown;
} TranscodeContext;

/* global context
 * function level context in this */
static TranscodeContext xcodeContext[NEXUS_NUM_VIDEO_ENCODERS];

static NEXUS_PlatformConfiguration g_platformConfig;
static char g_keyReturn ='\0';
static int g_selectedXcodeContextId = 0;
static int g_activeXcodeCount = 0;
static bool g_bScriptMode = false; /* script mode (mutual exclusive with interactive mode) */
static uint32_t g_scriptModeSleepTime = 0;/* sleep time in seconds */
static bool g_bFifo = false; /* FIFO file or complete file record */
static bool g_bNonRealTimeWrap = false;
static bool g_bAutoQuit = false;
static bool g_bEncodeCC = false;
static bool g_bBandwidthSaving = false;
static bool g_bTsUserData = false;
static bool g_bNoDspMixer = false;
static bool g_bSecondAudio = false;
static bool g_bMultiChanAudio = false;
static bool g_bStartWithoutMad = false;
static bool g_bEnableDebugSdDisplay = false;
static bool g_bCustomDelay = false;
static NEXUS_TransportTimestampType g_TtsInputType  = NEXUS_TransportTimestampType_eNone;
static NEXUS_TransportTimestampType g_TtsOutputType = NEXUS_TransportTimestampType_eNone;
static unsigned g_muxLatencyTolerance = 20;/* 20ms sw latency */
static unsigned g_msp = 50;/* 50ms Mux Serivce Period */
static unsigned g_muxDescs = 512;/* Mux XPT pb descriptors (when MTU BPP is enabled, worst case < A2P * frameRate * 3) */
static unsigned g_rsp = 50;/* 50ms Record Service Period */
static unsigned g_nrtRate = 8;
static NEXUS_VideoEncoderType g_eVideoEncoderType = NEXUS_VideoEncoderType_eMulti;
static B_EventHandle g_eotEvent = NULL;/* end of test event */
static NEXUS_VideoWindowContentMode g_contentMode = NEXUS_VideoWindowContentMode_eFull;
static int g_loopbackXcodeId = 0;
static bool g_loopbackStarted = false;
/* to enable xcode output lookback to HDMI display */
#if !defined(NEXUS_HAS_HDMI_OUTPUT) || !defined(NEXUS_HAS_DISPLAY) || NEXUS_NUM_DSP_VIDEO_ENCODERS
static bool g_bLoopbackPlayer = false;
#else
static bool g_bLoopbackPlayer = true;
#endif
/* default xcoder N uses video decoder (max-1-N) downwards since decoder 0(most powerful) reserved for local display;
   if true, xcoder N uses video decoder N upwards. This may help with headless config to demo 4K hevc->720p avc xcode usage. */
static bool g_bDecoderZeroUp  = false;
static unsigned g_vdecMaxWidth  = 1920;
static unsigned g_vdecMaxHeight = 1080;
static bool g_bForce48KbpsAACplus = false;
static bool g_b32KHzAudio = false;
static bool g_bSimulXcode = false;
static int g_simulDecoderMaster = 0;
static unsigned g_pcrInterval = 50;
static bool g_bOnePtsPerSegment = false;
static bool g_audioPesPacking = false;
static bool g_bSparseFrameRate = false;
static bool g_bEnableFieldPairing = true;
static bool g_bPrintStatus = false;

static const namevalue_t g_audioChannelFormatStrs[] = {
    {"none", NEXUS_AudioMultichannelFormat_eNone},
    {"stereo", NEXUS_AudioMultichannelFormat_eStereo},
    {"5.1", NEXUS_AudioMultichannelFormat_e5_1},
    {"7.1", NEXUS_AudioMultichannelFormat_e7_1},
    {NULL, 0}
};
static const namevalue_t g_entropyCodingStrs[] = {
    {"auto", NEXUS_EntropyCoding_eAuto},
    {"cavlc", NEXUS_EntropyCoding_eCavlc},
    {"cabac", NEXUS_EntropyCoding_eCabac},
    {NULL, 0}
};

static void config_xcoder_context (TranscodeContext *pContext );
static int open_transcode(TranscodeContext *pContext );
static void close_transcode(TranscodeContext *pContext );
static void start_transcode(TranscodeContext *pContext );
static void stop_transcode(TranscodeContext *pContext );
static void bringup_transcode(TranscodeContext *pContext );
static void shutdown_transcode(TranscodeContext *pContext );
static void xcode_start_systemdata( TranscodeContext  *pContext );
static void xcode_stop_systemdata( TranscodeContext  *pContext );

static void print_value_list(const namevalue_t *table)
{
    unsigned i;
    const char *sep=" {";
    for (i=0;table[i].name;i++) {
        /* skip aliases */
        if (i > 0 && table[i].value == table[i-1].value) continue;
        printf("%s (%u)%s",sep,table[i].value, table[i].name);
        sep = ",";
    }
    printf(" }\n");
}

static unsigned mylookup(const namevalue_t *table, const char *name)
{
    unsigned i;
    unsigned value;
    char *endptr;
    const char *valueName;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    value = strtol(name, &endptr, 0);
    if(!endptr || *endptr) { /* if valid, *endptr = '\0' */
        value = table[0].value;
    }
    valueName = lookup_name(table, value);
    BDBG_MSG(("Unknown cmdline param '%s', using %u as value ('%s')", name, value, valueName?valueName:"unknown"));
    return value;
}

/* search for "name=value;" and get the value */
static unsigned getNameValue(char *input, const namevalue_t list[])
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    if (pToken) {
        /* ignoring "name=" for now */
        pToken++;
    }
    else {
        pToken = input;
    }
    return mylookup(list, pToken);
}

static unsigned getValue(char *input)
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    return strtoul(pToken? (pToken+1) : input, NULL, 0);
}

static const char *getString(char *input)
{
    char *pToken;
    scanf("%s", input);
    pToken = strstr(input, ";");
    if(pToken) { *pToken = '\0'; }
    pToken = strstr(input, "=");
    return (pToken? (pToken+1) : input);
}

/* xcode window PQ setting */
static void window_dnr(
    NEXUS_VideoWindowHandle  hWindow,
    bool bCustom )
{
    NEXUS_VideoWindowDnrSettings windowDnrSettings;

    NEXUS_VideoWindow_GetDnrSettings(hWindow, &windowDnrSettings);
    if(bCustom)
    {
        printf("DNR settings:\n");
        printf("MNR mode: (%d) Disable; (%d) Bypass; (%d) Enable; ",
            NEXUS_VideoWindowFilterMode_eDisable, NEXUS_VideoWindowFilterMode_eBypass, NEXUS_VideoWindowFilterMode_eEnable);
        scanf("%d", (int*)&windowDnrSettings.mnr.mode);
        if(windowDnrSettings.mnr.mode == NEXUS_VideoWindowFilterMode_eEnable)
        {
            printf("MNR level (valid range -100 ... 2^31; for filtering, adjQp = Qp * (level + 100) / 100: ");
            scanf("%d", (int*)&windowDnrSettings.mnr.level);
        }
        printf("BNR mode: (%d) Disable; (%d) Bypass; (%d) Enable; ",
            NEXUS_VideoWindowFilterMode_eDisable, NEXUS_VideoWindowFilterMode_eBypass, NEXUS_VideoWindowFilterMode_eEnable);
        scanf("%d", (int*)&windowDnrSettings.bnr.mode);
        if(windowDnrSettings.bnr.mode == NEXUS_VideoWindowFilterMode_eEnable)
        {
            printf("BNR level (valid range -100 ... 2^31): ");
            scanf("%d", (int*)&windowDnrSettings.bnr.level);
        }
        printf("DCR mode: (%d) Disable; (%d) Bypass; (%d) Enable; ",
            NEXUS_VideoWindowFilterMode_eDisable, NEXUS_VideoWindowFilterMode_eBypass, NEXUS_VideoWindowFilterMode_eEnable);
        scanf("%d", (int*)&windowDnrSettings.dcr.mode);
        if(windowDnrSettings.dcr.mode == NEXUS_VideoWindowFilterMode_eEnable)
        {
            printf("DCR level (valid range -100 ... 2^31): ");
            scanf("%d", (int*)&windowDnrSettings.dcr.level);
        }
        if((windowDnrSettings.dcr.mode == NEXUS_VideoWindowFilterMode_eEnable) ||
           (windowDnrSettings.bnr.mode == NEXUS_VideoWindowFilterMode_eEnable) ||
           (windowDnrSettings.mnr.mode == NEXUS_VideoWindowFilterMode_eEnable))
        {
            printf("User Qp (default 0 means calculated from decoder source, non-zero is user override): ");
            scanf("%u", &windowDnrSettings.qp);
        }
    }
    else /* default */
    {
        windowDnrSettings.mnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        windowDnrSettings.bnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        windowDnrSettings.dcr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        windowDnrSettings.mnr.level = 0;
        windowDnrSettings.bnr.level = 0;
        windowDnrSettings.dcr.level = 0;
        windowDnrSettings.qp = 0;
    }
    NEXUS_VideoWindow_SetDnrSettings(hWindow, &windowDnrSettings);
}

static void window_mad(
    NEXUS_VideoWindowHandle  hWindow,
    bool bCustom,
    bool bEnable,
    bool bLowDelay)
{
    int choice;
    NEXUS_VideoWindowMadSettings windowMadSettings;

    NEXUS_VideoWindow_GetMadSettings(hWindow, &windowMadSettings);

    windowMadSettings.deinterlace = bEnable;
    if(bCustom)
    {
        if(windowMadSettings.deinterlace)
        {
            printf("3:2 reverse pulldown? (0) Disable; (1) Enable; ");
            scanf("%d", &choice); windowMadSettings.enable32Pulldown = choice;
            printf("2:2 reverse pulldown? (0) Disable; (1) Enable; ");
            scanf("%d", &choice);windowMadSettings.enable22Pulldown = choice;
            printf("Game Mode: (%d) Off (2-field delay, best quality); (%d) 1-field delay; (%d) 0-field delay; ",
                NEXUS_VideoWindowGameMode_eOff, NEXUS_VideoWindowGameMode_e4Fields_1Delay, NEXUS_VideoWindowGameMode_e4Fields_0Delay);
            scanf("%d", (int*)&windowMadSettings.gameMode);
        }
    }
    else /* default */
    {
        windowMadSettings.pqEnhancement = NEXUS_MadPqEnhancement_eAuto;
        windowMadSettings.enable32Pulldown = true;
        windowMadSettings.enable22Pulldown = true;
        windowMadSettings.gameMode = bLowDelay?NEXUS_VideoWindowGameMode_e4Fields_0Delay : NEXUS_VideoWindowGameMode_eOff;
    }

    NEXUS_VideoWindow_SetMadSettings(hWindow, &windowMadSettings);
}

static const char *s3DTVFormat[NEXUS_VideoDecoder3DTVFormat_eMax] = {
    "2D",
    "3D_LeftRightHalf",
    "3D_TopBottomHalf",
    "3D_FramePackedOverUnder",
};

static void decode3dCallback(void *pContext, int param)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    NEXUS_VideoDecoder3DTVStatus status;
    bool valid;

    NEXUS_VideoDecoder_Get3DTVStatus(videoDecoder, &status);
    if(param==-1) {
        printf("Loopback decoder");
    } else {
        printf("Xcoder[%d]", param);
    }
    printf(" 3DTV status: %s\n", (status.codec==NEXUS_VideoCodec_eH264)?"AVC":"MPEG2");
    if (status.codec==NEXUS_VideoCodec_eH264) {
        valid = status.codecData.avc.valid;
    }
    else {
        valid = status.codecData.mpeg2.valid;
    }
    printf("   valid %d, format %s\n", valid, s3DTVFormat[status.format]);

    /* At this point, the application should set the necessary 3D configuration for its platform,
       based on the reported 3DTV format. For example, DTVs should configure the video windows;
       Settops should configure the HDMI VSI. */
}

static void
transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}
#include "nexus_video_decoder_userdata.h"
#include "budp_dccparse.h"
enum BTST_P_DecoderId {
    BTST_P_DecoderId_eLoopback,
    BTST_P_DecoderId_eSource0,
    BTST_P_DecoderId_eSource1,
    BTST_P_DecoderId_eSource2,
    BTST_P_DecoderId_eSource3,
    BTST_P_DecoderId_eSource4,
    BTST_P_DecoderId_eSource5,
    BTST_P_DecoderId_eMax
} BTST_P_DecoderId;

static const char *g_userDataDecoderName[] = {
    "loopback",
    "source0",
    "source1",
    "source2",
    "source3",
    "source4",
    "source5"
};
typedef struct BTST_P_DecoderContext {
    struct {
        FILE *fLog;
        bool  bFilterNull;
        bool  bInit;
    } output608, output708;
    NEXUS_VideoDecoderHandle   videoDecoder;
    NEXUS_VideoCodec           codec;
    BUDP_DCCparse_ccdata       ccData[BTST_MAX_CC_DATA_COUNT];
    BAVC_USERDATA_info         userData;
} BTST_P_DecoderContext;
static BTST_P_DecoderContext g_decoderContext[BTST_P_DecoderId_eMax];

static void userdataCallback(void *context, int param)
{
    NEXUS_Error rc;
    BTST_P_DecoderContext *pContext = (BTST_P_DecoderContext*)context;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext->videoDecoder;

    BDBG_ASSERT(param < BTST_P_DecoderId_eMax);
    BDBG_MODULE_MSG(userdataCb, ("decoder %d user data callback!",param));

    /* It's crucial that the callback process all userdata packets in the buffer.
    The app cannot assume that it can process one packet per callback. */
    while (1) {
        unsigned size;
        uint8_t *buffer;
        uint32_t offset = 0;
        NEXUS_UserDataHeader *pHeader;
        BAVC_USERDATA_info   *toBeParsed = &pContext->userData;

        /* get the header */
        rc = NEXUS_VideoDecoder_GetUserDataBuffer(videoDecoder, (void**)&buffer, &size);
        BDBG_ASSERT(!rc);

        /* The app can assume that userdata will be placed into the buffer in whole packets.
        The termination condition for the loop is that there are no more bytes in the buffer. */
        if (!size) break;

        /* guaranteed whole header + payload */
        pHeader = (NEXUS_UserDataHeader *)buffer;
        BDBG_ASSERT(size >= pHeader->blockSize);
        BDBG_ASSERT(pHeader->blockSize == sizeof(*pHeader) + pHeader->payloadSize);
        BDBG_ASSERT(pHeader->blockSize % 4 == 0); /* 32 bit aligned */

        /* 1) resume the to-be-parsed user data structure as input of UDP; */
        toBeParsed->pUserDataBuffer     = (void*)((uint8_t*)buffer + sizeof(NEXUS_UserDataHeader));
        toBeParsed->ui32UserDataBufSize = pHeader->payloadSize;
        toBeParsed->eUserDataType = pHeader->type;
        toBeParsed->eSourcePolarity = pHeader->polarity;
        toBeParsed->bTopFieldFirst = pHeader->topFieldFirst;
        toBeParsed->bRepeatFirstField = pHeader->repeatFirstField;
        toBeParsed->bPTSValid = pHeader->ptsValid;
        toBeParsed->ui32PTS = pHeader->pts;
        toBeParsed->ePicCodingType = pHeader->pictureCoding;
        BKNI_Memcpy(toBeParsed->ui32PicCodExt, pHeader->pictureCodingExtension, sizeof(toBeParsed->ui32PicCodExt));

        /* 2) call UDP to parse cc data; */
        while (offset < toBeParsed->ui32UserDataBufSize) {
            unsigned i;
            size_t bytesParsed = 0;
            uint8_t ccCount = 0;
            BUDP_DCCparse_ccdata *ccData = pContext->ccData;

            if (pContext->codec == NEXUS_VideoCodec_eH264) {
                BUDP_DCCparse_SEI_isr(toBeParsed, offset, &bytesParsed, &ccCount, ccData);
            }
            else {
                BUDP_DCCparse_isr(toBeParsed, offset, &bytesParsed, &ccCount, ccData);
            }

            for( i = 0; i < ccCount; i++ )
            {
                FILE *fLog = NULL;
                bool bHeader = false;

                if ( true == ccData[i].bIsAnalog )
                {
                   if ( ( ( true == pContext->output608.bFilterNull ) && ( 0 != (ccData[i].cc_data_1 & 0x7F) ) )
                        || ( ( false == pContext->output608.bFilterNull ) ) )
                   {
                      fLog = pContext->output608.fLog;
                      if ( pContext->output608.bInit )
                      {
                         bHeader = true;
                         pContext->output608.bInit = false;
                      }
                   }
                }
                else
                {
                   if ( ( ( true == pContext->output708.bFilterNull ) && ( 0 != (ccData[i].cc_data_1) ) )
                        || ( ( false == pContext->output708.bFilterNull ) ) )
                   {
                      fLog = pContext->output708.fLog;
                      if ( pContext->output708.bInit )
                      {
                         bHeader = true;
                         pContext->output708.bInit = false;
                      }
                   }
                }

                if ( NULL != fLog)
                {
                   if ( true == bHeader )
                   {
                      fprintf( fLog, "size,ccCnt,PTS,Polarity,Format,CC Valid,CC Priority,CC Line Offset,CC Type,CC Data[0],CC Data[1]\n");
                   }

                   fprintf(fLog, "%u,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d\n",
                      pHeader->payloadSize, ccCount,
                      pHeader->pts,
                      ccData[i].polarity,
                      ccData[i].format,
                      ccData[i].cc_valid,
                      ccData[i].cc_priority,
                      ccData[i].line_offset,
                      ccData[i].seq.cc_type,
                      ccData[i].cc_data_1,
                      ccData[i].cc_data_2
                      );
                }
            }
            offset += bytesParsed;
        }

        NEXUS_VideoDecoder_UserDataReadComplete(videoDecoder, pHeader->blockSize);
    }
}

#if NEXUS_HAS_FRONTEND
static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend %p\n", (void*)frontend);

    NEXUS_Frontend_GetQamAsyncStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)frontend,
        qamStatus.fecLock, qamStatus.receiverLock);
}
#endif

static void sourcePlayerUsage(void)
{
    printf("? - this help\n"
           "a - frame Advance\n"
           "f - Fast Forward\n"
           "l - Slow Motion\n"
           "p - Play\n"
           "r - Fast Rewind\n"
           "s - pauSe\n"
           "w - Wait 30msec\n"
           "+ - Jump forward 5 seconds\n"
           "- - Jump backward 5 seconds\n"
           "0 - Jump to the beginning\n"
           "q - Go back to upper level menu\n"
           );
}

static void jump_to_beginning(NEXUS_PlaybackHandle playback)
{
    unsigned pos;
    NEXUS_Error rc;
    NEXUS_PlaybackStatus playbackStatus;

    rc = NEXUS_Playback_GetStatus(playback, &playbackStatus);
    BDBG_ASSERT(!rc);
    pos = playbackStatus.first;
    printf("Jump to beginning %u, %u...%u\n", pos, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
    (void)NEXUS_Playback_Seek(playback, pos);
}

static void sourcePlayer( void *context )
{
    int rate = NEXUS_NORMAL_PLAY_SPEED;
    TranscodeContext  *pContext = (TranscodeContext  *)context;

    if(pContext->inputSettings.resource != BTST_RESOURCE_FILE || !pContext->bStarted) return;
    for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        NEXUS_VideoDecoderStatus videoStatus;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;
        NEXUS_FilePosition first, last;
        char cmd[16];
        unsigned i;
        bool quit=false;
        NEXUS_PlaybackTrickModeSettings trickmode_settings;


        printf("Xcoder%d source player CMD:>", pContext->contextId);
        sourcePlayerUsage();
        if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL) {
            break;
        }
        for(i=0;cmd[i]!='\0';i++) {
            switch(cmd[i]) {
            case '?':
                sourcePlayerUsage();
                break;
            case 'a':
                printf( "frame advance\n" );
                NEXUS_Playback_FrameAdvance(pContext->playback, rate >= 0);
                break;
            case 's':
                printf( "pause\n" );
                rate = 0;
                NEXUS_Playback_Pause(pContext->playback);
                break;
            case 'p':
                printf( "play\n" );
                NEXUS_Playback_Play(pContext->playback);
                rate = NEXUS_NORMAL_PLAY_SPEED;
                break;
            case 'f':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate<=0) {
                    rate = NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "fast forward %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playback, &trickmode_settings );
                break;
            case 'l':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                rate /=2;
                trickmode_settings.rate = rate;
                printf( "slow down %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playback, &trickmode_settings );
                break;
            case 'q':
                quit = true;
                break;
            case 'r':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate>=0) {
                    rate = -NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "rewind %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playback, &trickmode_settings );
                break;
            case 'w':
                BKNI_Sleep(30);
                break;
            case '-':
                NEXUS_Playback_GetStatus(pContext->playback, &playbackStatus);
                if (playbackStatus.position >= 5*1000) {
                    playbackStatus.position -= 5*1000;
                    /* it's normal for a Seek to fail if it goes past the beginning */
                    printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                    (void)NEXUS_Playback_Seek(pContext->playback, playbackStatus.position);
                }
                break;
            case '+':
                NEXUS_Playback_GetStatus(pContext->playback, &playbackStatus);
                /* it's normal for a Seek to fail if it goes past the end */
                playbackStatus.position += 5*1000;
                printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                (void)NEXUS_Playback_Seek(pContext->playback, playbackStatus.position);
                break;
            case '0':
                jump_to_beginning(pContext->playback);
                break;
            case '\n':
            case ' ':
                NEXUS_Playback_GetStatus(pContext->playback, &playbackStatus);
                NEXUS_StcChannel_GetStc(pContext->stcVideoChannel, &stc);
                NEXUS_VideoDecoder_GetStatus(pContext->videoDecoder, &videoStatus);
                if(pContext->audioDecoder) NEXUS_AudioDecoder_GetStatus(pContext->audioDecoder, &audioStatus);
                NEXUS_FilePlay_GetBounds(pContext->file, &first, &last);

                printf("%d file %u:%u, playback %u%% position=%u.%03u sec, video %u%% %ux%u (%#x:%#x:%d)",
                    (int)playbackStatus.trickModeSettings.rate,
                    (unsigned)first.mpegFileOffset,  (unsigned)last.mpegFileOffset,
                    playbackStatus.fifoSize ? (playbackStatus.fifoDepth * 100) / playbackStatus.fifoSize : 0,
                    (unsigned)playbackStatus.position/1000, (unsigned)playbackStatus.position%1000,
                    videoStatus.fifoSize ? (videoStatus.fifoDepth * 100) / videoStatus.fifoSize : 0,
                    videoStatus.source.width, videoStatus.source.height, videoStatus.pts, stc, videoStatus.ptsStcDifference
                    );
                if(pContext->audioDecoder) printf(", audio %u%% %uHz (%#x:%#x:%d)\n",
                    audioStatus.fifoSize ? (audioStatus.fifoDepth * 100) / audioStatus.fifoSize : 0,
                    audioStatus.sampleRate, audioStatus.pts, stc, audioStatus.ptsStcDifference);
                else printf("\n");
                break;
            default:
                break;
            }
        }
        if(quit)  {
            break;
        }

    }
}

static const struct {
    const char *pchFormat;
    NEXUS_VideoFormat nexusVideoFormat;
} formatTable[] = {
    {"480i", NEXUS_VideoFormat_eNtsc},
    {"576i", NEXUS_VideoFormat_ePal},
    {"480p60", NEXUS_VideoFormat_e480p},
    {"576p50", NEXUS_VideoFormat_e576p},
    {"1080i60",  NEXUS_VideoFormat_e1080i},
    {"1080i50", NEXUS_VideoFormat_e1080i50hz},
    {"1080p24", NEXUS_VideoFormat_e1080p24hz},
    {"1080p25", NEXUS_VideoFormat_e1080p25hz},
    {"1080p30", NEXUS_VideoFormat_e1080p30hz},
    {"1080p50", NEXUS_VideoFormat_e1080p50hz},
    {"1080p60", NEXUS_VideoFormat_e1080p60hz},
    {"720p60",  NEXUS_VideoFormat_e720p},
    {"720p50",  NEXUS_VideoFormat_e720p50hz},
    {"720p24",  NEXUS_VideoFormat_e720p24hz},
    {"720p25",  NEXUS_VideoFormat_e720p25hz},
    {"720p30",  NEXUS_VideoFormat_e720p30hz},
    {"2160p24", NEXUS_VideoFormat_e3840x2160p24hz},
    {"2160p25", NEXUS_VideoFormat_e3840x2160p25hz},
    {"2160p30", NEXUS_VideoFormat_e3840x2160p30hz},
    {"2160p50", NEXUS_VideoFormat_e3840x2160p50hz},
    {"2160p60", NEXUS_VideoFormat_e3840x2160p60hz}
};

static void
vidSrcStreamChangedCallback(void *context, int param)
{
    NEXUS_VideoDecoderStreamInformation streamInfo;
    NEXUS_VideoDecoderHandle decoder = (NEXUS_VideoDecoderHandle)context;

    NEXUS_VideoDecoder_GetStreamInformation(decoder, &streamInfo);
    if(param == -1) {
        BDBG_WRN(("Loopback Video Source Stream Change callback:"));
    } else {
        BDBG_WRN(("Xcoder%d Video Source Stream Change callback:", param));
    }
    BDBG_WRN((" %ux%u@%s%c",
        streamInfo.sourceHorizontalSize,
        streamInfo.sourceVerticalSize,
        lookup_name(g_videoFrameRateStrs, streamInfo.frameRate),
        streamInfo.streamProgressive? 'p' : 'i'));
}

typedef struct GenericCallbackContext
{
   void *context;
   void *handle;
} GenericCallbackContext;

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = (NEXUS_HdmiOutputHandle)((GenericCallbackContext*)pParam)->context;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)((GenericCallbackContext*)pParam)->handle;

    BSTD_UNUSED(iParam);
    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    fprintf(stderr, "HDMI hotplug event: %s\n", status.connected?"connected":"not connected");

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

void xcode_loopback_setup( TranscodeContext  *pContext )
{
    NEXUS_HdmiOutputSettings hdmiOutputSettings;
    GenericCallbackContext hotplugCallbackContext;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderSettings vidDecodeSettings;
    NEXUS_VideoDecoderExtendedSettings extSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    NEXUS_DisplaySettings displaySettings;
    EncodeSettings  *pEncodeSettings = &pContext->encodeSettings;
    InputSettings   *pInputSettings = &pContext->inputSettings;
    int cnt = 0;

    BDBG_MSG(("To start the loopback player for transcoder%d...", pContext->contextId));
    NEXUS_StcChannel_GetDefaultSettings(BTST_LOOPBACK_STC_IDX, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.autoConfigTimebase = false;
    pContext->stcChannelLoopback = NEXUS_StcChannel_Open(BTST_LOOPBACK_STC_IDX, &stcSettings);
    BDBG_MSG(("Transcoder%d opened Loopback STC [%p].", pContext->contextId, (void*)pContext->stcChannelLoopback));
#if BTST_ENABLE_AV_SYNC
    /* create a sync channel for xcoder loopback decode */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    pContext->syncChannelLoopback = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    if(!pContext->bNoVideo) {
        unsigned decoderId = 0;
        NEXUS_VideoDecoderCapabilities cap;

        NEXUS_GetVideoDecoderCapabilities(&cap);
        printf("start xcode loopback display on HDMI and Component outputs...\n");
        do {
            pContext->videoDecoderLoopback = NEXUS_VideoDecoder_Open(decoderId, NULL); /* take default capabilities */
            decoderId++;
        } while(!pContext->videoDecoderLoopback && decoderId < cap.numVideoDecoders);
        BDBG_MSG(("Transcoder%d opened Loopback video decoder%d [%p].", pContext->contextId, decoderId-1, (void*)pContext->videoDecoderLoopback));
        NEXUS_VideoDecoder_GetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);
        vidDecodeSettings.supportedCodecs[pEncodeSettings->encoderVideoCodec] = true; /* it's for regular HD decode heap allocation; it covers mpeg2/h264/mpeg4 HD size */
        if(pContext->bEncodeCCUserData) {/* to log loopback xcoded user data */
            char fname[256];
            vidDecodeSettings.userDataEnabled = true;
            vidDecodeSettings.appUserDataReady.callback = userdataCallback;
            g_decoderContext[BTST_P_DecoderId_eLoopback].videoDecoder = pContext->videoDecoderLoopback;
            g_decoderContext[BTST_P_DecoderId_eLoopback].codec = pEncodeSettings->encoderVideoCodec;
            sprintf(fname, "userdata_%s_608.csv", g_userDataDecoderName[BTST_P_DecoderId_eLoopback]);
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog        = fopen(fname, "wb");
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.bInit       = true;
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.bFilterNull = false;
            sprintf(fname, "userdata_%s_708.csv", g_userDataDecoderName[BTST_P_DecoderId_eLoopback]);
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog = fopen(fname, "wb");
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.bInit       = true;
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.bFilterNull = false;
            vidDecodeSettings.appUserDataReady.context  = &g_decoderContext[BTST_P_DecoderId_eLoopback];
            vidDecodeSettings.appUserDataReady.param    = BTST_P_DecoderId_eLoopback;/* loopback */
        }
        /* channel change mode: hold until first new picture */
        vidDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
        vidDecodeSettings.streamChanged.callback = vidSrcStreamChangedCallback;
        vidDecodeSettings.streamChanged.context  = pContext->videoDecoderLoopback;
        vidDecodeSettings.streamChanged.param  = -1;
        NEXUS_VideoDecoder_SetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);

        /* register the 3DTV status change callback */
        if(pEncodeSettings->b3D) {
            NEXUS_VideoDecoder_GetExtendedSettings(pContext->videoDecoderLoopback, &extSettings);
            extSettings.s3DTVStatusEnabled = true;
            extSettings.s3DTVStatusChanged.callback = decode3dCallback;
            extSettings.s3DTVStatusChanged.context = pContext->videoDecoderLoopback;
            extSettings.s3DTVStatusChanged.param = -1;/* for loopback decoder */
            NEXUS_VideoDecoder_SetExtendedSettings(pContext->videoDecoderLoopback, &extSettings);
        }
    }

    pContext->playpumpLoopback = NEXUS_Playpump_Open(BTST_LOOPBACK_PLAYPUMP_IDX, NULL);
    assert(pContext->playpumpLoopback);
    BDBG_MSG(("Transcoder%d opened Loopback playpump%d [%p].", pContext->contextId, BTST_LOOPBACK_PLAYPUMP_IDX, (void*)pContext->playpumpLoopback));
    pContext->playbackLoopback = NEXUS_Playback_Create();
    assert(pContext->playbackLoopback);

    if(g_bFifo && pContext->indexfname[0]) {
        pContext->filePlayLoopback = NEXUS_FifoPlay_Open(pEncodeSettings->fname, pContext->indexfname, pContext->fifoFile);
    }
    else {
        pContext->filePlayLoopback = NEXUS_FilePlay_OpenPosix(pEncodeSettings->fname, pContext->indexfname[0]? pContext->indexfname : NULL);
    }
    if (!pContext->filePlayLoopback) {
        fprintf(stderr, "can't open file: %s, index %s\n", pEncodeSettings->fname, pContext->indexfname);
        exit(1);
    }

    NEXUS_Playback_GetSettings(pContext->playbackLoopback, &playbackSettings);
    playbackSettings.playpump = pContext->playpumpLoopback;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)NEXUS_TransportType_eTs;
    playbackSettings.playpumpSettings.timestamp.type = g_TtsOutputType;
    playbackSettings.stcChannel = pContext->stcChannelLoopback; /* loopback channel  */
    playbackSettings.timeshifting = true; /* allow for timeshift the growing file  */
#if 0 /* not suitable for local playback due to STC jamming with 1st PTS which might underflow decoder buffer, but may be used for live streaming to reduce latency */
    playbackSettings.timeshiftingSettings.endOfStreamGap = 1800; /* may need end-to-end latency to match encoder A2P */
#endif
   if ( g_bEncodeCC )
   {
      playbackSettings.timeshiftingSettings.endOfStreamGap = 5000; /* Add a delay to prevent repeated CC data being dumped */
   }
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
    NEXUS_Playback_SetSettings(pContext->playbackLoopback, &playbackSettings);

    /* Open the video pid channel */
    if(!pContext->bNoVideo) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pEncodeSettings->encoderVideoCodec; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.video.index = pContext->indexfname[0]? true : false;
        playbackPidSettings.pidTypeSettings.video.decoder = pContext->videoDecoderLoopback;
        pContext->videoPidChannelLoopback = NEXUS_Playback_OpenPidChannel(pContext->playbackLoopback, BTST_MUX_VIDEO_PID, &playbackPidSettings);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = pEncodeSettings->encoderVideoCodec;
        videoProgram.pidChannel = pContext->videoPidChannelLoopback;
        videoProgram.stcChannel = pContext->stcChannelLoopback;

        /* Bring up loopback video display and outputs */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.timebase = NEXUS_Timebase_e0;
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format =
            (pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e25 ||
             pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e50) ?
            NEXUS_VideoFormat_e720p50hz : NEXUS_VideoFormat_e720p;
        pContext->displayLoopback = NEXUS_Display_Open(BTST_LOOPBACK_DISPLAY_IDX, &displaySettings);
        BDBG_MSG(("Transcoder%d opened Loopback display%d [%p].", pContext->contextId, BTST_LOOPBACK_DISPLAY_IDX, (void*)pContext->displayLoopback));
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if(g_platformConfig.outputs.component[0]){
            NEXUS_Display_AddOutput(pContext->displayLoopback, NEXUS_ComponentOutput_GetConnector(g_platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
            if(g_platformConfig.outputs.hdmi[0]){
                NEXUS_Display_AddOutput(pContext->displayLoopback, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
            }
#endif
        /* Install hotplug callback -- video only for now */
        NEXUS_HdmiOutput_GetSettings(g_platformConfig.outputs.hdmi[0], &hdmiOutputSettings);
        hotplugCallbackContext.context = g_platformConfig.outputs.hdmi[0];
        hotplugCallbackContext.handle = pContext->displayLoopback;
        hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
        hdmiOutputSettings.hotplugCallback.context = &hotplugCallbackContext;
        NEXUS_HdmiOutput_SetSettings(g_platformConfig.outputs.hdmi[0], &hdmiOutputSettings);

        pContext->windowLoopback = NEXUS_VideoWindow_Open(pContext->displayLoopback, 0);

        NEXUS_VideoWindow_AddInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));

        /* enable window deinterlacer default setting: it'll grab MCVP first */
        window_mad(pContext->windowLoopback, false, true, false);
    }

    /* if audio DAC0 for loopback player */
    if(pInputSettings->bAudioInput)
    {
        /* Open the audio loopback decoder */
        pContext->audioDecoderLoopback = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, NULL);
        BDBG_MSG(("Transcoder%d opened Loopback audio decoder%d [%p].", pContext->contextId, BTST_LOOPBACK_AUD_DECODE_IDX, (void*)pContext->audioDecoderLoopback));

        /* Open the audio pid channel */
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = pContext->audioDecoderLoopback;
            pContext->audioPidChannelLoopback = NEXUS_Playback_OpenPidChannel(pContext->playbackLoopback, BTST_MUX_AUDIO_PID, &playbackPidSettings);
        }

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);

        audioProgram.codec = pEncodeSettings->bAudioEncode?
            pEncodeSettings->encoderAudioCodec : pInputSettings->eAudioCodec;
        audioProgram.pidChannel = pContext->audioPidChannelLoopback;
        audioProgram.stcChannel = pContext->stcChannelLoopback;

        /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
        /* add hdmi pcm audio output to loopback display */
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));

#if BTST_ENABLE_AV_SYNC
        /* connect sync channel before start decode */
        NEXUS_SyncChannel_GetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
        if(!pContext->bNoVideo)
            syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback);
        if(pInputSettings->bAudioInput)
        {
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo);
        }

        NEXUS_SyncChannel_SetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
#endif

        /* start audio loopback decoder */
        NEXUS_AudioDecoder_Start(pContext->audioDecoderLoopback, &audioProgram);
    }

        /* Start video loopback decoder */
        if(!pContext->bNoVideo)
            NEXUS_VideoDecoder_Start(pContext->videoDecoderLoopback, &videoProgram);

    /* delay the loopback until A2P passed to avoid stutter since playback jams PTS to STC instead of locking PCR so the encode buffer
       model is not enforced at loopback decoder plus the O_DIRECT timeshift buffer seems to have some latency. */
    BKNI_Sleep(4000);
    /* wait for timeshift file index to be available */
    while (pContext->indexfname[0] && cnt++ < 2000) {
        NEXUS_RecordStatus status;
        NEXUS_Record_GetStatus(pContext->record, &status);
        if (status.picturesIndexed) {
            BDBG_WRN(("%u pictures indexed\n", status.picturesIndexed));
            break;
        }
        BKNI_Sleep(10);
    }
    if(cnt >= 2000) /* try 20 seconds */
    {
        fprintf(stderr, "**** Encoder stalls and cannot timeshift xcoded stream! Please debug! ****\n");
    }

    /* Linking loopback Playback to Record allows Playback to sleep until Record writes data. Avoids a busyloop near live. */
    if(NEXUS_Record_AddPlayback(pContext->record, pContext->playbackLoopback)) {
        BDBG_ERR(("Loopback player failed to timeshift playback transcoder[%u]'s record", pContext->contextId));
        BDBG_ASSERT(0);
    }

    /* Start playback */
    NEXUS_Playback_Start(pContext->playbackLoopback, pContext->filePlayLoopback, NULL);
}

void xcode_loopback_shutdown( TranscodeContext  *pContext )
{
#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    InputSettings   *pInputSettings = &pContext->inputSettings;

    BDBG_MSG(("To shutdown the loopback player for transcoder%d...", pContext->contextId));
    NEXUS_Playback_Stop(pContext->playbackLoopback);
    NEXUS_Record_RemovePlayback(pContext->record, pContext->playbackLoopback);
    if(!pContext->bNoVideo)
        NEXUS_VideoDecoder_Stop(pContext->videoDecoderLoopback);
    /* disconnect sync channel after stop decoder */
    if(pInputSettings->bAudioInput)
    {
        NEXUS_AudioDecoder_Stop(pContext->audioDecoderLoopback);
#if BTST_ENABLE_AV_SYNC
        NEXUS_SyncChannel_GetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
#endif
        NEXUS_AudioDecoder_Close(pContext->audioDecoderLoopback);
    }
    NEXUS_Playback_CloseAllPidChannels(pContext->playbackLoopback);
    NEXUS_FilePlay_Close(pContext->filePlayLoopback);
    NEXUS_Playback_Destroy(pContext->playbackLoopback);
    NEXUS_Playpump_Close(pContext->playpumpLoopback);

    if(!pContext->bNoVideo) {
        NEXUS_VideoWindow_RemoveInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
        NEXUS_VideoDecoder_Close(pContext->videoDecoderLoopback);
        NEXUS_VideoWindow_Close(pContext->windowLoopback);
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_Display_RemoveOutput(pContext->displayLoopback, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
        NEXUS_StopCallbacks(g_platformConfig.outputs.hdmi[0]);
#endif
        NEXUS_Display_Close(pContext->displayLoopback);
    }
#if BTST_ENABLE_AV_SYNC
    if(pContext->syncChannelLoopback) {
        NEXUS_SyncChannel_Destroy(pContext->syncChannelLoopback);
        pContext->syncChannelLoopback = NULL;
    }
#endif

    NEXUS_StcChannel_Close(pContext->stcChannelLoopback);

    if(g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog) {
        fclose(g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog);
        g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog = NULL;
    }
    if(g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog) {
        fclose(g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog);
        g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog = NULL;
    }
}

static void loopbackUsage(void)
{
    printf("? - this help\n"
           "a - loopback window Aspect ratio mode\n"
           "c - Close selected transcoder loopback player\n"
           "d - loopback Display format switch\n"
           "f - Fast Forward\n"
           "i - Select xcoder Id for loopback\n"
           "l - Slow Motion\n"
           "o - Open selected transcoder loopback player\n"
           "p - Play\n"
           "r - Fast Rewind\n"
           "s - pauSe\n"
           "w - Wait 30msec\n"
           "z - resiZe loopback video window\n"
           "+ - Jump forward 5 seconds\n"
           "- - Jump backward 5 seconds\n"
           "0 - Jump to the beginning\n"
           "q - Go back to upper level menu\n"
           );
}

static void loopbackPlayer( TranscodeContext  xcodeContext[] )
{
    int rate = NEXUS_NORMAL_PLAY_SPEED;
    TranscodeContext *pContext = &xcodeContext[g_loopbackXcodeId];
    for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        NEXUS_RecordStatus recordStatus;
        NEXUS_VideoDecoderStatus videoStatus;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;
        NEXUS_FilePosition first, last;
        char cmd[16];
        unsigned i;
        bool quit=false;
        int choice;
        NEXUS_PlaybackTrickModeSettings trickmode_settings;
        NEXUS_DisplaySettings displaySettings;
        NEXUS_VideoWindowSettings windowSettings;


        printf("Xcoder loopback player CMD:>");
        loopbackUsage();
        if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL) {
            break;
        }
        for(i=0;cmd[i]!='\0';i++) {
            if(!g_loopbackStarted && (cmd[i]!='i' && cmd[i]!='o' && cmd[i]!='q'))
                continue;
            switch(cmd[i]) {
            case '?':
                loopbackUsage();
                break;
            case 's':
                printf( "pause\n" );
                rate = 0;
                NEXUS_Playback_Pause(pContext->playbackLoopback);
                break;
            case 'p':
                printf( "play\n" );
                rate = NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_Play(pContext->playbackLoopback);
                break;
            case 'f':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate<=0) {
                    rate = NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "fast forward %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'l':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                rate /=2;
                trickmode_settings.rate = rate;
                printf( "slow down %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'q':
                quit = true;
                break;
            case 'r':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate>=0) {
                    rate = -NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "rewind %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'w':
                BKNI_Sleep(30);
                break;
            case '-':
                NEXUS_Playback_GetStatus(pContext->playbackLoopback, &playbackStatus);
                if (playbackStatus.position >= 5*1000) {
                    playbackStatus.position -= 5*1000;
                    /* it's normal for a Seek to fail if it goes past the beginning */
                    printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                    (void)NEXUS_Playback_Seek(pContext->playbackLoopback, playbackStatus.position);
                }
                break;
            case '+':
                NEXUS_Playback_GetStatus(pContext->playbackLoopback, &playbackStatus);
                /* it's normal for a Seek to fail if it goes past the end */
                playbackStatus.position += 5*1000;
                printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                (void)NEXUS_Playback_Seek(pContext->playbackLoopback, playbackStatus.position);
                break;
            case '0':
                jump_to_beginning(pContext->playbackLoopback);
                break;
            case '\n':
            case ' ':
                NEXUS_Playback_GetStatus(pContext->playbackLoopback, &playbackStatus);
                NEXUS_StcChannel_GetStc(pContext->stcChannelLoopback, &stc);
                NEXUS_VideoDecoder_GetStatus(pContext->videoDecoderLoopback, &videoStatus);
                if(pContext->audioDecoderLoopback) NEXUS_AudioDecoder_GetStatus(pContext->audioDecoderLoopback, &audioStatus);
                if(g_bFifo) {
                    NEXUS_FifoRecord_GetPosition(pContext->fifoFile, &first, &last);
                }
                else {
                    NEXUS_FilePlay_GetBounds(pContext->filePlayLoopback, &first, &last);
                }
                NEXUS_Record_GetStatus(pContext->record, &recordStatus);
                printf("%d file %u:%u, pictureIndices:%u, playback %u%% position=%u.%03u sec, video %u%% %ux%u (%#x:%#x:%d)",
                    (int)playbackStatus.trickModeSettings.rate,
                    (unsigned)first.mpegFileOffset,  (unsigned)last.mpegFileOffset, recordStatus.picturesIndexed,
                    playbackStatus.fifoSize ? (playbackStatus.fifoDepth * 100) / playbackStatus.fifoSize : 0,
                    (unsigned)playbackStatus.position/1000, (unsigned)playbackStatus.position%1000,
                    videoStatus.fifoSize ? (videoStatus.fifoDepth * 100) / videoStatus.fifoSize : 0,
                    videoStatus.source.width, videoStatus.source.height, videoStatus.pts, stc, videoStatus.ptsStcDifference
                    );
                if(pContext->audioDecoderLoopback) printf(", audio %u%% %uHz (%#x:%#x:%d)\n",
                    audioStatus.fifoSize ? (audioStatus.fifoDepth * 100) / audioStatus.fifoSize : 0,
                    audioStatus.sampleRate, audioStatus.pts, stc, audioStatus.ptsStcDifference);
                else printf("\n");
                break;
            case 'd': /* display format switch */
                NEXUS_Display_GetSettings(pContext->displayLoopback, &displaySettings);
                printf("switch loopback display (hdmi/component out) format to:\n");
                printf("\n displayFormat:\n");
                for(choice=0; (unsigned)choice < sizeof(formatTable)/sizeof(formatTable[0]); choice++)
                {
                    printf(" (%2d) %s\n", formatTable[choice].nexusVideoFormat, formatTable[choice].pchFormat);
                }
                scanf("%d", (int32_t*)&displaySettings.format);
                NEXUS_Display_SetSettings(pContext->displayLoopback, &displaySettings);
                break;
            case 'z': /* resize loopback video window */
                NEXUS_VideoWindow_GetSettings(pContext->windowLoopback, &windowSettings);
                printf("Resize loopback window to:\n");
                printf("position x: \n"); scanf("%d", &choice); windowSettings.position.x = (int16_t)choice;
                printf("position y: \n"); scanf("%d", &choice); windowSettings.position.y = (int16_t)choice;
                printf("position width: \n"); scanf("%d", &choice); windowSettings.position.width = (uint16_t)choice;
                printf("position height: \n"); scanf("%d", &choice); windowSettings.position.height= (uint16_t)choice;
                NEXUS_VideoWindow_SetSettings(pContext->windowLoopback, &windowSettings);
                break;
            case 'a': /* change loopback display aspect ratio correction mode */
                printf("\n Please select loopback window's aspect ratio correction mode:\n"
                    " (%d) Zoom\n"
                    " (%d) Box\n"
                    " (%d) Pan and Scan\n"
                    " (%d) Bypass aspect ratio correction\n"
                    " (%d) PanScan without additional aspect ratio correction\n",
                    (NEXUS_VideoWindowContentMode_eZoom),
                    (NEXUS_VideoWindowContentMode_eBox),
                    (NEXUS_VideoWindowContentMode_ePanScan),
                    (NEXUS_VideoWindowContentMode_eFull),
                    (NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection));
                NEXUS_VideoWindow_GetSettings(pContext->windowLoopback, &windowSettings);
                scanf("%d", (int *)&windowSettings.contentMode);
                printf("\n Enable letterbox auto cut? (0) Disable; (1) enable;\n");
                scanf("%d", &choice); windowSettings.letterBoxAutoCut = choice;
                windowSettings.letterBoxDetect = windowSettings.letterBoxAutoCut;
                NEXUS_VideoWindow_SetSettings(pContext->windowLoopback, &windowSettings);
                break;
            case 'i': /* select xcoder loopback ID */
                B_Mutex_Lock(pContext->mutexStarted);
                if(g_loopbackStarted) {
                    B_Mutex_Unlock(pContext->mutexStarted);
                    printf("Already started loopback on xcoder%d\n", g_loopbackXcodeId);
                    break;
                }
                B_Mutex_Unlock(pContext->mutexStarted);
                printf("Please select Xcoder ID for loopback [0 ~ %d]: ", NEXUS_NUM_VIDEO_ENCODERS-1);
                scanf("%d", &g_loopbackXcodeId);
                g_loopbackXcodeId = (g_loopbackXcodeId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):g_loopbackXcodeId;
                pContext = &xcodeContext[g_loopbackXcodeId];
                break;
            case 'o': /* open xcoder loopback */
                B_Mutex_Lock(pContext->mutexStarted);
                if(!g_loopbackStarted && (g_loopbackXcodeId != -1)) {
                    printf("To start xcoder%d loopback...\n", g_loopbackXcodeId);
                    xcode_loopback_setup(&xcodeContext[g_loopbackXcodeId]);
                    g_loopbackStarted = true;
                }
                B_Mutex_Unlock(pContext->mutexStarted);
                break;
            case 'c': /* close xcoder loopback */
                B_Mutex_Lock(pContext->mutexStarted);
                if(g_loopbackStarted) {
                    printf("To stop xcoder%d loopback...\n", g_loopbackXcodeId);
                    xcode_loopback_shutdown(&xcodeContext[g_loopbackXcodeId]);
                    g_loopbackStarted = false;
                }
                B_Mutex_Unlock(pContext->mutexStarted);
                break;
            default:
                break;
            }
        }
        if(quit)  {
            break;
        }

    }
}

#if NEXUS_HAS_FRONTEND
static void tune_qam( TranscodeContext  *pContext )
{
    char key[80];
    NEXUS_FrontendQamSettings qamSettings;
#ifdef BTST_PID_RETUNE /* TODO: add PID channel change */
    char input[80];
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
#if BTST_ENABLE_AV_SYNC
        NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    NEXUS_VideoDecoder_Stop(pContext->videoDecoder);
    NEXUS_PidChannel_Close(pContext->videoPidChannel);
    if(pContext->inputSettings.iPcrPid != pContext->inputSettings.iVideoPid && pContext->pcrPidChannel) {
        NEXUS_PidChannel_Close(pContext->pcrPidChannel);
        pContext->pcrPidChannel = NULL;
    }
#endif

    printf("\n Front End QAM freq (Mhz) ");
    pContext->inputSettings.freq = getValue(key);
    printf("\n Front End QAM Mode:\n");
    print_value_list(g_qamModeStrs);
    pContext->inputSettings.qamMode = getNameValue(key, g_qamModeStrs);

#ifdef BTST_PID_RETUNE
    pContext->inputSettings.eStreamType = NEXUS_TransportType_eTs;
    printf("\n source stream codec:\n");
    print_value_list(g_videoCodecStrs);
    pContext->inputSettings.eVideoCodec = getNameValue(key, g_videoCodecStrs);
    printf("\n Video pid:  "); pContext->inputSettings.iVideoPid = getValue(key);
    printf("\n Pcr   pid:  "); pContext->inputSettings.iPcrPid = getValue(key);

    pContext->videoPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pContext->inputSettings.iVideoPid, NULL);
    if(pContext->inputSettings.iPcrPid != pContext->inputSettings.iVideoPid) {
        pContext->pcrPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pContext->inputSettings.iPcrPid, NULL);
        BDBG_MSG(("Transcoder%d opened PCR PID channel for parser band %d.", pContext->contextId, pContext->parserBand));
    }

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = pContext->inputSettings.eVideoCodec;
    videoProgram.pidChannel = pContext->videoPidChannel;
    videoProgram.stcChannel = pContext->stcChannel;
    if(pContext->inputSettings.bAudioInput)
    {
        NEXUS_AudioDecoder_Stop(pContext->audioDecoder);
        NEXUS_PidChannel_Close(pContext->audioPidChannel);
#if BTST_ENABLE_AV_SYNC
        /* disconnect sync channel after decoders stop */
        NEXUS_SyncChannel_GetSettings(pContext->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(pContext->syncChannel, &syncChannelSettings);
#endif
        printf("\n Audio pid:  ");
        pContext->inputSettings.iAudioPid = getValue(key);
        pContext->audioPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pContext->inputSettings.iAudioPid, NULL);
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eAc3;
        audioProgram.pidChannel = pContext->audioPidChannel;
        audioProgram.stcChannel = pContext->stcChannel;
    }

    NEXUS_StcChannel_GetSettings(pContext->stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = (pContext->inputSettings.iPcrPid != pContext->inputSettings.iVideoPid)?
        pContext->pcrPidChannel : /* different PCR PID */
        pContext->videoPidChannel; /* PCR happens to be on video pid */
    NEXUS_StcChannel_SetSettings(pContext->stcChannel, &stcSettings);
#endif

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = pContext->inputSettings.freq* 1000000;
    qamSettings.mode = pContext->inputSettings.qamMode;
    switch (pContext->inputSettings.qamMode) {
    case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
    case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
    case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; /* TODO */ break;
    default: fprintf(stderr, "Invalid QAM mode!\n"); return;
    }
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = pContext->frontend;
    NEXUS_Frontend_TuneQam(pContext->frontend, &qamSettings);

#ifdef BTST_PID_RETUNE
    if(pContext->inputSettings.bAudioInput)
    {
#if BTST_ENABLE_AV_SYNC
        /* disconnect sync channel after decoders stop */
        NEXUS_SyncChannel_GetSettings(pContext->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder);
        syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, pContext->encodeSettings.bAudioEncode?
            NEXUS_AudioDecoderConnectorType_eStereo : NEXUS_AudioDecoderConnectorType_eCompressed);
        NEXUS_SyncChannel_SetSettings(pContext->syncChannel, &syncChannelSettings);
#endif
        NEXUS_AudioDecoder_Start(pContext->audioDecoder, &audioProgram);
    }
    NEXUS_VideoDecoder_Start(pContext->videoDecoder, &videoProgram);
#endif
}
#else
static void tune_qam( TranscodeContext  *pContext )
{
    BSTD_UNUSED(pContext);
    BDBG_ERR(("No frontend supported"));
}
#endif

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void open_gfx(
    TranscodeContext  *pContext )
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Error rc;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 100;
    createSettings.height = 100;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(get_display_index(pContext->contextId));
    pContext->surface = NEXUS_Surface_Create(&createSettings);

    BKNI_CreateEvent(&pContext->checkpointEvent);
    BKNI_CreateEvent(&pContext->spaceAvailableEvent);

    pContext->gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(pContext->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = pContext->checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = pContext->spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(pContext->gfx, &gfxSettings);

    /* fill with black */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = pContext->surface;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0x60FF0000;
    NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);

    rc = NEXUS_Graphics2D_Checkpoint(pContext->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(pContext->checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(pContext->displayTranscode, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.position.x = 0;
    graphicsSettings.position.y = 0;
    graphicsSettings.position.width  = createSettings.width;
    graphicsSettings.position.height = createSettings.height;
    graphicsSettings.clip.width      = graphicsSettings.position.width;
    graphicsSettings.clip.height     = graphicsSettings.position.height;
    NEXUS_Display_SetGraphicsSettings(pContext->displayTranscode, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(pContext->displayTranscode, pContext->surface);
}

static void close_gfx(
    TranscodeContext  *pContext )
{
    NEXUS_GraphicsSettings graphicsSettings;

    if(NULL == pContext->gfx) return;

    /* disable gfx window */
    NEXUS_Display_GetGraphicsSettings(pContext->displayTranscode, &graphicsSettings);
    graphicsSettings.enabled = false;
    NEXUS_Display_SetGraphicsSettings(pContext->displayTranscode, &graphicsSettings);

    NEXUS_Graphics2D_Close(pContext->gfx);
    NEXUS_Surface_Destroy(pContext->surface);
    BKNI_DestroyEvent(pContext->checkpointEvent);
    BKNI_DestroyEvent(pContext->spaceAvailableEvent);
    pContext->gfx = NULL;
}

static void mux_duration( TranscodeContext *pContext )
{
    unsigned hrs, mins, secs;
    NEXUS_StreamMuxStatus status;
    NEXUS_StreamMux_GetStatus(pContext->streamMux, &status);
    secs = status.duration/1000;
    mins = secs/60;
    hrs  = mins/60;
    mins = mins%60;
    secs = secs%60;
    printf("Mux[%u] duration: %02u:%02u:%02u.%03u\n", pContext->contextId,
        hrs, mins, secs, status.duration%1000);
    printf("Mux[%u] avg. bitrate (kbps): v=%5u, a=%3u, s=%2u, u=%3u [efficiency=%3u%%] ("BDBG_UINT64_FMT" bytes)\n", pContext->contextId,
       status.video.averageBitrate/1000,
       status.audio.averageBitrate/1000,
       status.systemdata.averageBitrate/1000,
       status.userdata.averageBitrate/1000,
       status.efficiency,
       BDBG_UINT64_ARG(status.totalBytes));
    {
       unsigned i;

       for ( i = 0; i < NEXUS_MAX_MUX_PIDS; i++ )
       {
          if ( pContext->muxConfig.video[i].encoder )
          {
             printf("Mux[%u]    video[%2d] timestamp: 0x%08x\n", pContext->contextId,i,
                status.video.pid[i].currentTimestamp);
          }
       }
       for ( i = 0; i < NEXUS_MAX_MUX_PIDS; i++ )
          if ( pContext->muxConfig.audio[i].muxOutput )
          {
             printf("Mux[%u]    audio[%2d] timestamp: 0x%08x\n", pContext->contextId,i,
                status.audio.pid[i].currentTimestamp);
          }
       {
          {
             printf("Mux[%u]  sysdata[%2d] timestamp: 0x%08x\n", pContext->contextId,0,
                status.systemdata.pid[0].currentTimestamp);
          }
       }
       for ( i = 0; i < NEXUS_MAX_MUX_PIDS; i++ )
       {
          if ( pContext->muxConfig.userdata[i].message )
          {
             printf("Mux[%u] userdata[%2d] timestamp: 0x%08x\n", pContext->contextId,i,
                status.userdata.pid[i].currentTimestamp);
          }
       }
    }
}

static void avsync_correlation_error( TranscodeContext *pContext )
{
    size_t size[2];
    const NEXUS_VideoEncoderDescriptor *vdesc[2];
    const NEXUS_AudioMuxOutputFrame *adesc[2];
    unsigned i, j;
    uint32_t v_opts, a_opts;
    uint64_t v_pts, a_pts, v_stc, a_stc;
    double v_opts2pts = 0, a_opts2pts = 0, error;
    unsigned origPtsScale = (NEXUS_TransportType_eDssEs != pContext->inputSettings.eStreamType ||
        NEXUS_TransportType_eDssPes != pContext->inputSettings.eStreamType)? 45 : 27000;
    bool validVframe = false, validAframe = false;

    NEXUS_VideoEncoder_GetBuffer(pContext->videoEncoder, &vdesc[0], &size[0], &vdesc[1], &size[1]);
    for(j=0;j<2 && !validVframe;j++) {
        for(i=0;i<size[j];i++) {
            if(vdesc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID)
            {
                v_opts = vdesc[j][i].originalPts;
                v_pts  = vdesc[j][i].pts;
                v_stc  = vdesc[j][i].stcSnapshot;
                v_opts2pts = (double)v_pts / 90 - (double)v_opts / origPtsScale;
                BDBG_MSG(("V: opts=%u pts="BDBG_UINT64_FMT", stc="BDBG_UINT64_FMT", opts2pts=%f",
                      v_opts, BDBG_UINT64_ARG(v_pts), BDBG_UINT64_ARG(v_stc), v_opts2pts));
                validVframe = true;
                break;
            }
        }
    }
    NEXUS_AudioMuxOutput_GetBuffer(pContext->audioMuxOutput, &adesc[0], &size[0], &adesc[1], &size[1]);
    for(j=0;j<2 && !validAframe;j++) {
        for(i=0;i<size[j];i++) {
            if(adesc[j][i].flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_PTS_VALID)
            {
                a_opts = adesc[j][i].originalPts;
                a_pts  = adesc[j][i].pts;
                a_stc  = adesc[j][i].stcSnapshot;
                a_opts2pts = (double)a_pts / 90 - (double)a_opts / origPtsScale;
                BDBG_MSG(("A: opts=%u pts="BDBG_UINT64_FMT", stc="BDBG_UINT64_FMT", opts2pts=%f",
                      a_opts, BDBG_UINT64_ARG(a_pts), BDBG_UINT64_ARG(a_stc), a_opts2pts));
                validAframe = true;
                break;
            }
        }
    }
    if(validVframe && validAframe) {
        error = a_opts2pts - v_opts2pts;
        printf("AV sync correlation error (positive means video leads): %.1f ms\n\n", error);
        if(error > 20 || error < -20) BDBG_ERR(("AV sync error is greater than 20ms!\n"));
    }
}

/* build with export BPROFILE_SUPPORT=y to enable profiling of memory allocation */
#if !BPROFILE_SUPPORT
/* if BPROFILE_SUPPORT not enabled, stub out functions to prevent error */
#define NEXUS_Profile_MarkThread(s)
#define NEXUS_Profile_Start()
#define NEXUS_Profile_Stop(X)
#endif

static void printMenu(TranscodeContext *pContext)
{
    printf("Menu:\n");
#if BTST_ENABLE_XCODE_LOOPBACK
    printf(" 0) xcode loopback player control\n");
#endif
    printf(" 1) change video encoder resolution\n");
    printf(" 2) change video encoder bitrate\n");
    printf(" 3) change video encoder frame rate\n");
    printf(" 4) change xcode video window MAD settings\n");
    printf(" 5) change xcode video window DNR settings\n");
    if(pContext->inputSettings.resource == BTST_RESOURCE_QAM)
        printf(" 6) retune QAM channel\n");
    printf(" 7) change xcode window DNR demo mode\n");
    printf(" 8) change Video stream mux PES pacing\n");
    printf(" 9) change Audio stream mux PES pacing\n");
    printf("10) change PCR stream mux PES pacing\n");
    printf("11) change source video decoder scanout mode\n");
    printf("12) change xcode aspect ratio correction mode\n");
    printf("13) change xcode window scaler coeff index\n");
    printf("14) Get video encoder status\n");
    printf("15) Select xcoder context to config\n");
    printf("16) Start new RAP\n");
    printf("17) change GOP structure setting\n");
    printf("18) change Closed Caption user data transcode setting\n");
    printf("19) change xcode video window AFD/Bar clip setting\n");
    printf("20) Open/Start selected transcoder\n");
    printf("21) Stop/Close selected transcoder\n");
    printf("22) Start selected transcoder in adaptive low latency mode\n");
    printf("23) Stop  selected transcoder in immediate mode\n");
    printf("24) Start system data insertion\n");
    printf("25) Stop system data insertion\n");
    printf("26) Change audio encoder bitrate\n");
    printf("27) Toggle audio encoder sample rate to 32KHz\n");
    printf("28) Open Gfx window\n");
    printf("29) Close Gfx window\n");
    printf("30) Stop stream mux recpump\n");
    printf("31) Start stream mux recpump\n");
    printf("32) Toggle cc user data encode\n");
    printf("33) Toggle CBR/VBR encode on current xcode context\n");
    printf("34) Toggle 3D/2D encode on current xcode context\n");
    printf("35) Toggle 3D/2D decode input type on current xcode context\n");
    printf("36) Video encoder memory config on current xcode context\n");
    printf("37) Enable/disable stream mux ES channel\n");
    printf("38) Enable/disable entropy coding\n");
    printf("39) Enable/disable video decoder mute\n");
    printf("50) random dynamic resolution test\n");
    printf("51) random dynamic bitrate test\n");
    printf("52) Stop stream mux to test encoder overflow recovery\n");
    printf("53) back to back NRT stop/start test\n");
    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE)
        printf("90) Source trick mode player\n");
    printf("99) change DEBUG module setting\n");
    printf("100) sleep\n");
}

static void printStatus( TranscodeContext *pContext )
{
   NEXUS_VideoEncoderStatus videoEncodeStatus;
   NEXUS_VideoEncoderClearStatus clearStatus;
   NEXUS_VideoDecoderStatus videoDecodeStatus;
   NEXUS_AudioDecoderStatus audioDecodeStatus;
   NEXUS_AudioMuxOutputStatus audioMuxOutputStatus;
   NEXUS_RecpumpStatus recpumpStatus;
   if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
      if(!pContext->bNoVideo) {
         NEXUS_VideoDecoder_GetStatus(pContext->videoDecoder, &videoDecodeStatus);
         printf("\nVideo Decoder[%d] Status:\n", pContext->contextId);
         printf("----------------------\n");
         printf("data buffer depth     = %u\n", videoDecodeStatus.fifoDepth);
         printf("data buffer size      = %u\n", videoDecodeStatus.fifoSize);
         printf("queued frames         = %u\n", videoDecodeStatus.queueDepth);
         printf("numDecoded count      = %u\n", videoDecodeStatus.numDecoded);
         printf("numDisplayed          = %u\n", videoDecodeStatus.numDisplayed);
         printf("numDecodeDrops        = %u\n", videoDecodeStatus.numDecodeDrops);
         printf("numDisplayDrops       = %u\n", videoDecodeStatus.numDisplayDrops);
         printf("numDecodeOverflows    = %u\n", videoDecodeStatus.numDecodeOverflows);
         printf("numDisplayUnderflows  = %u\n", videoDecodeStatus.numDisplayUnderflows);
         printf("current PTS (45KHz)   = 0x%x\n", videoDecodeStatus.pts);
         printf("PTS error count       = %u\n", videoDecodeStatus.ptsErrorCount);
         printf("----------------------\n\n");
      }
      if(pContext->inputSettings.bAudioInput) {
         NEXUS_AudioDecoder_GetStatus(pContext->audioDecoder, &audioDecodeStatus);
         printf("Audio Decoder[%d] Status:\n", pContext->contextId);
         printf("----------------------\n");
         printf("data buffer depth     = %u\n", audioDecodeStatus.fifoDepth);
         printf("data buffer size      = %u\n", audioDecodeStatus.fifoSize);
         printf("sample rate           = %u\n", audioDecodeStatus.sampleRate);
         printf("queued frames         = %u\n", audioDecodeStatus.queuedFrames);
         printf("numDecoded count      = %u\n", audioDecodeStatus.framesDecoded);
         printf("numDummyFrames        = %u\n", audioDecodeStatus.dummyFrames);
         printf("numFifoOverflows      = %u\n", audioDecodeStatus.numFifoOverflows);
         printf("numFifoUnderflows     = %u\n", audioDecodeStatus.numFifoUnderflows);
         printf("current PTS (45KHz)   = 0x%x\n", audioDecodeStatus.pts);
         printf("PTS error count       = %u\n", audioDecodeStatus.ptsErrorCount);
         printf("----------------------\n\n");
         NEXUS_AudioMuxOutput_GetStatus(pContext->audioMuxOutput, &audioMuxOutputStatus);
         printf("Audio mux output[%d] Status:\n", pContext->contextId);
         printf("----------------------\n");
         printf("data buffer depth     = %u\n", audioMuxOutputStatus.data.fifoDepth);
         printf("data buffer size      = %u\n", audioMuxOutputStatus.data.fifoSize);
         printf("numEncoded frames     = %u\n", audioMuxOutputStatus.numFrames);
         printf("numErrorFrames        = %u\n", audioMuxOutputStatus.numErrorFrames);
         printf("----------------------\n\n");
      }
   }
   if(!pContext->bNoVideo) {
      NEXUS_VideoEncoder_GetStatus(pContext->videoEncoder, &videoEncodeStatus);
      printf("Video Encoder[%d] Status:\n", pContext->contextId);
      printf("----------------------\n");
      printf("output data buffer depth     = "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG((uint64_t)videoEncodeStatus.data.fifoDepth));
      printf("output data buffer size      = "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG((uint64_t)videoEncodeStatus.data.fifoSize));
      printf("error flags                  = 0x%x\n", videoEncodeStatus.errorFlags);
      printf("error count                  = %u\n", videoEncodeStatus.errorCount);
      printf("picture drops due to error   = %u\n", videoEncodeStatus.picturesDroppedErrors);
      printf("picture drops due to HRD     = %u\n", videoEncodeStatus.picturesDroppedHRD);
      printf("picture drops due to FRC     = %u\n", videoEncodeStatus.picturesDroppedFRC);
      printf("pictures Encoded             = %u\n", videoEncodeStatus.picturesEncoded);
      printf("pictures Received            = %u\n", videoEncodeStatus.picturesReceived);
      printf("picture Id Last Encoded      = 0x%x\n", videoEncodeStatus.pictureIdLastEncoded);
      printf("pictures per second          = %u\n", videoEncodeStatus.picturesPerSecond);
      printf("----------------------\n\n");
      NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
      NEXUS_VideoEncoder_ClearStatus(pContext->videoEncoder, &clearStatus);
   }
   if(pContext->recpump) {
      NEXUS_Recpump_GetStatus(pContext->recpump, &recpumpStatus);
      printf("Recpump[%d] Status:\n", pContext->contextId);
      printf("----------------------\n");
      printf("output data buffer depth     = "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG((uint64_t)recpumpStatus.data.fifoDepth));
      printf("output data buffer size      = "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG((uint64_t)recpumpStatus.data.fifoSize));
      printf("bytesRecorded                = 0x%x%08x\n", (uint32_t)(recpumpStatus.data.bytesRecorded>>32), (uint32_t)recpumpStatus.data.bytesRecorded);
      printf("----------------------\n\n");
   }
   mux_duration(pContext);
   /* HDMI input cannot measure AV sync via original PTS correlation */
   if(pContext->inputSettings.bAudioInput && !pContext->bNoVideo &&
      (pContext->inputSettings.resource != BTST_RESOURCE_HDMI))
      avsync_correlation_error(pContext);
}

static void keyHandler( TranscodeContext  xcodeContext[] )
{
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoWindowSplitScreenSettings splitSettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindowCoefficientIndexSettings stCoeffIndex;
    NEXUS_VideoWindowAfdSettings afdSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_StreamMuxSettings muxSettings;
    char key[256], *pStr;
    int choice, input;
    NEXUS_VideoEncoderSettingsOnInputChange encodeSettingsOnInputChange;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoInputSettings  videoInputSettings;
    NEXUS_Error rc;
    int rcInput = 0;
    TranscodeContext *pContext = &xcodeContext[g_selectedXcodeContextId];

    printMenu(pContext);
    NEXUS_Profile_MarkThread("keyhandler");
    for(;g_keyReturn != 'q';)
    {
        printf("\nEnter 'h' to print menu\n");
        printf("Enter 'q' to quit\n\n");

        /* use non-blocking i/o to handle key input to allow clean exit */
        while (g_keyReturn != 'q')
        {
           fd_set rfds;
           struct timeval tv;
           int retval;

           /* Watch stdin (fd 0) to see when it has input. */
           FD_ZERO(&rfds);
           FD_SET(0, &rfds);

           /* Wait up to 100 miliseconds. */
           tv.tv_sec = 0;
           tv.tv_usec = 100000;

           retval = select(1, &rfds, NULL, NULL, &tv);
           /* Don't rely on the value of tv now! */

           if (retval == -1)
               printf("select()\n");
           else if (retval)
           {
              BDBG_MSG(("Data is available now."));
               /* FD_ISSET(0, &rfds) will be true. */
              rcInput = scanf("%s", key);
              break;
           }
           /*else BDBG_MSG(("No data within timeout."));*/
        }
        if(g_bScriptMode && rcInput == EOF)
        {
            BKNI_Sleep(1000*g_scriptModeSleepTime);
            g_keyReturn = 'q';
            break;
        }
        if(!strcmp(key, "q") || !strcmp(key, "quit") || g_keyReturn == 'q')
        {
            g_keyReturn = 'q';
            break;
        }
        else if(!strcmp(key, "h") || !strcmp(key, "help")) {
            printMenu(pContext);
            continue;
        }
        pStr = strstr(key, "=");
        choice = strtoul(pStr? (pStr+1) : key, NULL, 0);
        if(pContext->bNoVideo && (choice!=14) && (choice!=20) && (choice!=21)
           && ((choice<24) || (choice>27)) && (choice<99)) continue;

        B_Mutex_Lock(pContext->mutexStarted);
        if(!pContext->videoEncoder && !pContext->audioMuxOutput && ((choice!=20) && (choice!=36) && (choice!=99) && (choice!=15))) {
            printf("context %d closed.\n", pContext->contextId);
            B_Mutex_Unlock(pContext->mutexStarted);
            continue;
        }
        if(!pContext->bStarted && ((choice==21) || (choice==23) || (choice==25) || (choice==30) ||
            (choice==52) || (choice==53) || (choice==14))){
            printf("context %d stopped.\n", pContext->contextId);
            B_Mutex_Unlock(pContext->mutexStarted);
            continue;
        }
        switch(choice)
        {
            case 0: /* xcoder loopback trick play control */
                if(g_loopbackXcodeId != -1) {
                    B_Mutex_Unlock(pContext->mutexStarted);
                    loopbackPlayer(xcodeContext);
                    B_Mutex_Lock(pContext->mutexStarted);
                } else {
                    printf("An encoder display took away display 0 so loopback player is unavailable!\n");
                }
                break;
            case 1: /* resolution change */
                printf("xcode resolution change:   ");

                /* typically resolution change combined with bit rate change */
                printf("Do you want to change bit rate synchronously as well? 0)No 1)Yes\n");
                input = getValue(key);
                if(input)
                {
                    printf("Bit rate change from %u to:   ", pContext->encodeSettings.encoderBitrate);
                    printf("\n Bitrate (bps):  ");
                    pContext->encodeSettings.encoderBitrate = getValue(key);

                    /* turn on the synchronous change feature first before resolution/bitrate change! */
                    NEXUS_VideoEncoder_GetSettingsOnInputChange(pContext->videoEncoder, &encodeSettingsOnInputChange);
                    encodeSettingsOnInputChange.bitrateMax = pContext->encodeSettings.encoderBitrate;
                    NEXUS_VideoEncoder_SetSettingsOnInputChange(pContext->videoEncoder, &encodeSettingsOnInputChange);
                }

                printf("\n custom format (0) No (1) Yes ");
                pContext->encodeSettings.bCustom = getValue(key);
                if(!pContext->encodeSettings.bCustom)
                {
                    printf("\n displayFormat:\n");
                    print_value_list(g_videoFormatStrs);
                    pContext->encodeSettings.displayFormat = getNameValue(key, g_videoFormatStrs);
                    NEXUS_Display_GetSettings(pContext->displayTranscode, &displaySettings);
                    displaySettings.format = pContext->encodeSettings.displayFormat;
                    rc = NEXUS_Display_SetSettings(pContext->displayTranscode, &displaySettings);
                    BERR_TRACE(rc);
                }
                else
                {
                    printf("\n Resolution width & height    ");
                    pContext->encodeSettings.customFormatSettings.width = getValue(key);
                    pContext->encodeSettings.customFormatSettings.height = getValue(key);
                    printf("\n Interlaced? (0) No (1) Yes   ");
                    pContext->encodeSettings.customFormatSettings.interlaced = getValue(key);
                    printf("\n vertical refresh rate (in 1/1000th Hz)   ");
                    pContext->encodeSettings.customFormatSettings.refreshRate = getValue(key);
                    printf("\n Aspect Ratio:\n");
                    print_value_list(g_displayAspectRatioStrs);
                    pContext->encodeSettings.customFormatSettings.aspectRatio = getNameValue(key, g_displayAspectRatioStrs);
                    if(NEXUS_DisplayAspectRatio_eSar == pContext->encodeSettings.customFormatSettings.aspectRatio)
                    {
                        printf("Please enter Sample Aspect Ratio X and Y: \n");
                        pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x = getValue(key);
                        pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y = getValue(key);
                    }
                    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
                    customFormatSettings.width = pContext->encodeSettings.customFormatSettings.width;
                    customFormatSettings.height = pContext->encodeSettings.customFormatSettings.height;
                    customFormatSettings.refreshRate = pContext->encodeSettings.customFormatSettings.refreshRate;
                    customFormatSettings.interlaced = pContext->encodeSettings.customFormatSettings.interlaced;
                    customFormatSettings.aspectRatio = pContext->encodeSettings.customFormatSettings.aspectRatio;
                    customFormatSettings.sampleAspectRatio.x = pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x;
                    customFormatSettings.sampleAspectRatio.y = pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y;
                    customFormatSettings.dropFrameAllowed = true;
                    rc = NEXUS_Display_SetCustomFormatSettings(pContext->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
                    BERR_TRACE(rc);
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            case 2: /* bitrate change */
                printf("Bit rate change from %u to:   ", pContext->encodeSettings.encoderBitrate);
                printf("\n Bitrate (bps)    ");
                pContext->encodeSettings.encoderBitrate = getValue(key);
                NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
                videoEncoderConfig.bitrateMax = pContext->encodeSettings.encoderBitrate;
                NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
                break;
            case 3: /* frame rate change */
                printf("frame rate change:   ");
                printf("\n Frame rate\n");
                print_value_list(g_videoFrameRateStrs);
                pContext->encodeSettings.encoderFrameRate = getNameValue(key, g_videoFrameRateStrs);
                NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
                printf("Current encode variable frame rate mode: %d\n", videoEncoderConfig.variableFrameRate);
                printf("Change encode variable frame rate mode: [0=N/1=Y]   ");
                videoEncoderConfig.variableFrameRate = getValue(key);
                videoEncoderConfig.frameRate = pContext->encodeSettings.encoderFrameRate;
                NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
                break;
            case 4: /* MAD setting */
                /* window PQ custom setting */
                printf("MAD settings:\n");
                printf("Enable? (0) Disable; (1) Enable; ");
                window_mad(pContext->windowTranscode, true, getValue(key), false);
                break;
            case 5: /* DNR setting */
                /* window PQ custom setting */
                window_dnr(pContext->windowTranscode, true);
                break;
            case 6: /* retune source channel */
                if(pContext->inputSettings.resource == BTST_RESOURCE_QAM)
                    tune_qam(pContext);
                break;
            case 7: /* xcode window demo mode */
                NEXUS_VideoWindow_GetSplitScreenSettings(pContext->windowTranscode, &splitSettings);
                printf("\n xcode window DNR demo mode: (%d)Disable (%d)Left (%d)Right   ",
                    NEXUS_VideoWindowSplitScreenMode_eDisable,
                    NEXUS_VideoWindowSplitScreenMode_eLeft,
                    NEXUS_VideoWindowSplitScreenMode_eRight);
                splitSettings.dnr = getValue(key);
                NEXUS_VideoWindow_SetSplitScreenSettings(pContext->windowTranscode, &splitSettings);
                break;
            case 8: /* toggle video stream mux pes pacing */
                printf("\n Disable xcode video PES pacing mode: (0)No (1)Yes    ");
                NEXUS_Playpump_SuspendPacing(pContext->playpumpTranscodeVideo, getValue(key));
                break;
            case 9: /* toggle audio stream mux pes pacing */
                printf("\n Disable xcode audio PES pacing mode: (0)No (1)Yes    ");
                NEXUS_Playpump_SuspendPacing(pContext->playpumpTranscodeAudio, getValue(key));
                break;
            case 10: /* toggle system data stream mux pes pacing */
                printf("\n Disable xcode system data PES pacing mode: (0)No (1)Yes  ");
                NEXUS_Playpump_SuspendPacing(pContext->playpumpTranscodePcr, getValue(key));
                break;
            case 11: /* change 1080p24/30 source video decoder's scan mode progressive 3:2/2:2 ITFP test */
                printf("\n Force 1080p24/30 source video decoder progressive scan for 3:2/2:2 ITFP test : (0)No (1)Yes  ");
                NEXUS_VideoDecoder_GetSettings(pContext->videoDecoder, &videoDecoderSettings);
                videoDecoderSettings.scanMode = getValue(key);
                NEXUS_VideoDecoder_SetSettings(pContext->videoDecoder, &videoDecoderSettings);
                break;
            case 12: /* change xcoder aspect ratio correction mode */
                printf("\n Please select transcoder aspect ratio correction mode:\n");
                print_value_list(g_contentModeStrs);
                NEXUS_VideoWindow_GetSettings(pContext->windowTranscode, &windowSettings);
                windowSettings.contentMode = getNameValue(key, g_contentModeStrs);
                NEXUS_VideoWindow_SetSettings(pContext->windowTranscode, &windowSettings);
                break;
            case 13: /* change xcoder window scaler coeffs */
                NEXUS_VideoWindow_GetCoefficientIndexSettings(pContext->windowTranscode, &stCoeffIndex);
                printf("\nPlease set transcoder window scaler coefficients index:\n");
                printf("current scaler coeff index = %d;   new index [101~126] = ", stCoeffIndex.sclHorzLuma);
                stCoeffIndex.sclHorzLuma = getValue(key);
                NEXUS_VideoWindow_SetCoefficientIndexSettings(pContext->windowTranscode, &stCoeffIndex);
                break;
            case 14: /* get video encoder status */
               printStatus( pContext );
                break;
            case 15: /* select xcoder context */
                printf("Please select Xcoder context to configure [0 ~ %d]: ", NEXUS_NUM_VIDEO_ENCODERS-1);
                g_selectedXcodeContextId = getValue(key);
                g_selectedXcodeContextId = (g_selectedXcodeContextId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):g_selectedXcodeContextId;
                B_Mutex_Unlock(pContext->mutexStarted);
                pContext = &xcodeContext[g_selectedXcodeContextId];
                B_Mutex_Lock(pContext->mutexStarted);
                break;
            case 16: /* start new RAP */
                NEXUS_VideoEncoder_InsertRandomAccessPoint(pContext->videoEncoder);
                break;
            case 17: /* dynamic change GOP structure */
                NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
                printf("Context%d current GOP structure: duration = %u, num of P = %u, num of B = %u, %s GOP%s.\n",
                    pContext->contextId,
                    videoEncoderConfig.streamStructure.duration, videoEncoderConfig.streamStructure.framesP,
                    videoEncoderConfig.streamStructure.framesB, videoEncoderConfig.streamStructure.openGop?"open":"close",
                    videoEncoderConfig.streamStructure.newGopOnSceneChange? ", start new GOP on scene change":"");
                printf("Change GOP to fixed duration? [0: no; x: x ms]");
                choice = getValue(key);
                videoEncoderConfig.streamStructure.duration = pContext->encodeSettings.gopDuration = choice;
                if(choice == 0) {
                    printf("Enable new GOP on scene change? [0=n/1=y] ");
                    choice = getValue(key);
                    videoEncoderConfig.streamStructure.newGopOnSceneChange = pContext->encodeSettings.newGopOnSceneChange = choice;
                }
                printf("Change num of P to: ");
                choice = getValue(key);
                videoEncoderConfig.streamStructure.framesP = pContext->encodeSettings.encoderGopStructureFramesP = choice;
                printf("Change num of B to: ");
                choice = getValue(key);
                videoEncoderConfig.streamStructure.framesB = pContext->encodeSettings.encoderGopStructureFramesB = choice;
                if(choice) {
                    printf("Enable open GOP? [0=n/1=y] ");
                    choice = getValue(key);
                    videoEncoderConfig.streamStructure.openGop = pContext->encodeSettings.openGop = choice;
                }
                NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
                break;
            case 18: /* change CC data transcode ON/OFF */
                printf("Context%d enabled CC data transcoding: %d\n", g_selectedXcodeContextId, g_bEncodeCC);
                printf("Change to [1=ON; 0=OFF]: ");
                g_bEncodeCC = getValue(key);
                pContext->bEncodeCCUserData = g_bEncodeCC;
                break;
            case 19: /* change AFD/Bar clip setting */
                NEXUS_VideoWindow_GetAfdSettings(pContext->windowTranscode, &afdSettings);
                printf("\nPlease set transcoder window AFD/Bar clip mode:\n");
                printf("current AFD/Bar clip mode = %d; new setting = [0=Dis/1=En]", afdSettings.mode);
                choice = getValue(key);
                afdSettings.mode = choice? NEXUS_AfdMode_eStream : NEXUS_AfdMode_eDisabled;
                NEXUS_VideoWindow_SetAfdSettings(pContext->windowTranscode, &afdSettings);
                break;
            case 22: /* start adaptive delay */
                pContext->bAdaptiveLatencyStart = (0 == pContext->uiStopMode) ? false : true;
#if BTST_ENABLE_FAST_STOP_START
                BDBG_MSG(("\t\nTo channel restart...\n"));
                start_transcode(pContext); /* start only */
                pContext->bNoStopDecode = false; /* reset */
                if(g_bLoopbackPlayer && (g_selectedXcodeContextId == g_loopbackXcodeId) && !g_loopbackStarted &&
                   (BTST_LOOPBACK_DISPLAY_IDX != get_display_index(pContext->contextId))) {
                    xcode_loopback_setup(pContext);
                    g_loopbackStarted = true;
                }
                break;
#endif
            case 20: /* start selected xcoder context */
                if(pContext->bStarted) {
                    printf("context %d already started.\n", pContext->contextId);
                    break;
                }
                printf("To start the xcoder%d...\n", pContext->contextId);
                if(!pContext->inputSettings.bConfig) config_xcoder_context(pContext);
                else {
                    printf("Reconfigure the transcoder%d? [1=y/0=n] ", pContext->contextId);
                    if(getValue(key)) config_xcoder_context(pContext);
                }
                if(g_bLoopbackPlayer) {
#if BTST_SW7445_159_STC_WORKAROUND
                    if(g_activeXcodeCount+1 >= NEXUS_NUM_VIDEO_ENCODERS && g_loopbackStarted) {
#else
                    if(((g_activeXcodeCount+1 >= NEXUS_NUM_VIDEO_DECODERS) ||
                        (BTST_LOOPBACK_DISPLAY_IDX == get_display_index(pContext->contextId))) && g_loopbackStarted) {
#endif
                        g_loopbackStarted = false;
                        xcode_loopback_shutdown(&xcodeContext[g_loopbackXcodeId]);
                        g_loopbackXcodeId = -1;/* disable loopback */
                    }
                }
                /* open & start transcoder */
                bringup_transcode(pContext);
                if(g_bLoopbackPlayer && (g_selectedXcodeContextId == g_loopbackXcodeId) && !g_loopbackStarted) {
                        /* need to have both free decoder and display to set up loopback player */
                        if(BTST_LOOPBACK_DISPLAY_IDX != get_display_index(pContext->contextId) &&
                            (g_activeXcodeCount < NEXUS_NUM_VIDEO_DECODERS)) {
                            xcode_loopback_setup(pContext);
                            g_loopbackStarted = true;
                        } else {
                            g_loopbackXcodeId = -1;/* disable loopback */
                        }
                }
                break;
            case 23: /* stop mode */
                printf("Stop encoder mode? [0=normal/1=immediate/2=abort]\n");
                pContext->uiStopMode = getValue(key);
                printf("Don't stop decoder? [1=y/0=n]\n");
                pContext->bNoStopDecode = getValue(key);
#if BTST_ENABLE_FAST_STOP_START
                BDBG_MSG(("\t\nTo channel stop...\n"));
                /* bringdown loopback path */
                if(g_bLoopbackPlayer && g_selectedXcodeContextId == g_loopbackXcodeId && g_loopbackStarted) {
                    g_loopbackStarted = false;
                    xcode_loopback_shutdown(pContext);
                }
                stop_transcode(pContext);/* stop only */
                break;
#endif
            case 21: /* stop selected xcoder context */
                printf("To stop xcoder%d [mode=%d]...\n", pContext->contextId, pContext->uiStopMode);
                /* bringdown loopback path */
                if(g_bLoopbackPlayer && g_selectedXcodeContextId == g_loopbackXcodeId && g_loopbackStarted) {
                    g_loopbackStarted = false;
                    xcode_loopback_shutdown(pContext);
                }
                BDBG_MSG(("activeXcodes: %d, loopbackId: %d", g_activeXcodeCount, g_loopbackXcodeId));
                /* if the encoder is shutdown with display matching with loopback display id, loopback is available now */
                if(g_bLoopbackPlayer && (BTST_LOOPBACK_DISPLAY_IDX == get_display_index(pContext->contextId) ||
                    (g_activeXcodeCount <= NEXUS_NUM_VIDEO_DECODERS && (g_loopbackXcodeId==-1)))) {
                    g_loopbackXcodeId = 0;/* was -1 */
                }
                /* stop & close transcoder */
                shutdown_transcode(pContext);
                if(g_bAutoQuit) {
                    int i;
                    for(i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
                        if(xcodeContext[i].bStarted) break;
                    }
                    /* when all contexts are stopped, quit the test */
                    if(i == NEXUS_NUM_VIDEO_ENCODERS) {
                        g_keyReturn = 'q';
                        B_Mutex_Unlock(pContext->mutexStarted);
                        goto Done;
                    }
                }
                break;
            case 24:
                xcode_start_systemdata(pContext);
                break;
            case 25:
                xcode_stop_systemdata(pContext);
                break;
            case 26:
                if(pContext->encodeSettings.bAudioEncode)
                {
                    NEXUS_AudioEncoderCodecSettings codecSettings;
                    NEXUS_AudioEncoder_GetCodecSettings(pContext->audioEncoder, pContext->encodeSettings.encoderAudioCodec, &codecSettings);
                    switch(pContext->encodeSettings.encoderAudioCodec) {
                    case NEXUS_AudioCodec_eAac:
                        printf("Context%d AAC audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pContext->contextId, codecSettings.codecSettings.aac.bitRate);
                        codecSettings.codecSettings.aac.bitRate = getValue(key);
                        break;
                    case NEXUS_AudioCodec_eAacPlus:
                    case NEXUS_AudioCodec_eAacPlusAdts:
                        printf("Context%d AAC+ audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pContext->contextId, codecSettings.codecSettings.aacPlus.bitRate);
                        codecSettings.codecSettings.aacPlus.bitRate = getValue(key);
                        break;
                    case NEXUS_AudioCodec_eMp3:
                        printf("Context%d MP3 audio encoder bitrate = %u bps.\nPlease enter the new bitrate:\n", pContext->contextId, codecSettings.codecSettings.mp3.bitRate);
                        codecSettings.codecSettings.mp3.bitRate = getValue(key);
                        break;
                    default:
                        printf("Unsupported audio encoder codec %d!\n", pContext->encodeSettings.encoderAudioCodec);
                        break;
                    }
                    NEXUS_AudioEncoder_SetCodecSettings(pContext->audioEncoder, &codecSettings);
                }
                else {
                    printf("Context%d audio encoder is disabled!\n", pContext->contextId);
                }
                break;
            case 27:
                if(pContext->encodeSettings.bAudioEncode)
                {
                    if(g_bNoDspMixer) {
                        NEXUS_AudioEncoderCodecSettings codecSettings;
                        NEXUS_AudioEncoder_GetCodecSettings(pContext->audioEncoder, pContext->encodeSettings.encoderAudioCodec, &codecSettings);
                        switch(pContext->encodeSettings.encoderAudioCodec) {
                        case NEXUS_AudioCodec_eAac:
                            printf("Context%d AAC audio encoder sample rate = %u KHz.\n", pContext->contextId, codecSettings.codecSettings.aac.sampleRate/1000);
                            codecSettings.codecSettings.aac.sampleRate = (codecSettings.codecSettings.aac.sampleRate == 32000)? 48000 : 32000;
                            printf("changed to %u KHz.\n", codecSettings.codecSettings.aac.sampleRate/1000);
                            break;
                        case NEXUS_AudioCodec_eAacPlus:
                        case NEXUS_AudioCodec_eAacPlusAdts:
                            printf("Context%d AAC+ audio encoder sample rate = %u Hz.\n", pContext->contextId, codecSettings.codecSettings.aacPlus.sampleRate);
                            codecSettings.codecSettings.aacPlus.sampleRate = (codecSettings.codecSettings.aacPlus.sampleRate == 32000)? 48000 : 32000;
                            printf("changed to %u KHz.\n", codecSettings.codecSettings.aacPlus.sampleRate/1000);
                            break;
                        default:
                            printf("Unsupported audio encoder codec %d for sample rate conversion!\n", pContext->encodeSettings.encoderAudioCodec);
                            break;
                        }
                        NEXUS_AudioEncoder_SetCodecSettings(pContext->audioEncoder, &codecSettings);
                    } else {/* if mixer is master, use mixer to change SR */
                        NEXUS_AudioMixerSettings mixerSettings;
                        NEXUS_AudioDecoder_Stop(pContext->audioDecoder);
                        NEXUS_AudioMixer_Stop(pContext->audioMixer);
                        NEXUS_AudioMixer_GetSettings(pContext->audioMixer, &mixerSettings);
                        mixerSettings.outputSampleRate = (mixerSettings.outputSampleRate==48000)?32000:48000;
                        NEXUS_AudioMixer_SetSettings(pContext->audioMixer, &mixerSettings);
                        NEXUS_AudioMixer_Start(pContext->audioMixer);
                        NEXUS_AudioDecoder_Start(pContext->audioDecoder, &pContext->audProgram);
                    }
                }
                else {
                    printf("Context%d audio encoder is not enabled! No SRC.\n", pContext->contextId);
                }
                break;
            case 28:
                printf("Open gfx window 100x100 at (0, 0)...\n");
                open_gfx(pContext);
                break;
            case 29:
                printf("Close gfx window...\n");
                close_gfx(pContext);
                break;
            case 30:
                printf("Stop stream mux %d's recpump...\n", pContext->contextId);
                NEXUS_Record_Stop(pContext->record);
                break;
            case 31:
                printf("Start stream mux %d's recpump...\n", pContext->contextId);
                NEXUS_Record_Start(pContext->record, pContext->fileTranscode);
                break;
            case 32:
                printf("Context%d Toggle cc user data xcode %s to %s.\n", pContext->contextId, pContext->bEncodeCCUserData?"on":"off", pContext->bEncodeCCUserData?"off":"on");
                pContext->bEncodeCCUserData ^= true;
                break;
            case 33:
                printf("Context%d Toggle VBR video encode %s to %s.\n", pContext->contextId, pContext->encodeSettings.vbr?"on":"off", pContext->encodeSettings.vbr?"off":"on");
                pContext->encodeSettings.vbr ^= true;
                break;
            case 34:
                printf("Context%d Toggle 3D/2D video encode %s to %s.\n", pContext->contextId, pContext->encodeSettings.b3D?"3D":"2D", pContext->encodeSettings.b3D?"2D":"3D");
                pContext->encodeSettings.b3D ^= true;
                NEXUS_Display_GetSettings(pContext->displayTranscode, &displaySettings);
                displaySettings.display3DSettings.overrideOrientation = true;
                displaySettings.display3DSettings.orientation         = NEXUS_VideoOrientation_e2D;
                if(pContext->encodeSettings.b3D)
                {
                    printf("\n 3D orientation:\n");
                    print_value_list(g_videoOrientation);
                    pContext->encodeSettings.display3dType = getNameValue(key, g_videoOrientation);
                    displaySettings.display3DSettings.orientation         = pContext->encodeSettings.display3dType;
                }
                NEXUS_Display_SetSettings(pContext->displayTranscode, &displaySettings);
                break;
            case 35:
                printf("Context%d Toggle 3D/2D video input orientation %s to %s.\n", pContext->contextId, pContext->encodeSettings.bInput3D?"3D":"2D", pContext->encodeSettings.bInput3D?"2D":"3D");
                pContext->encodeSettings.bInput3D ^= true;
                NEXUS_VideoInput_GetSettings(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder), &videoInputSettings);
                videoInputSettings.video3DSettings.overrideOrientation = true;
                videoInputSettings.video3DSettings.orientation = NEXUS_VideoOrientation_e2D;
                if(pContext->encodeSettings.bInput3D)
                {
                    printf("\n 3D orientation:\n");
                    print_value_list(g_videoOrientation);
                    pContext->encodeSettings.input3dType = getNameValue(key, g_videoOrientation);
                    videoInputSettings.video3DSettings.orientation = pContext->encodeSettings.input3dType;
                }

                rc = NEXUS_VideoInput_SetSettings(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder), &videoInputSettings);
                printf("******************** overide %d orientation %d", videoInputSettings.video3DSettings.overrideOrientation, videoInputSettings.video3DSettings.orientation);
                break;
            case 36:
                printf("Video encoder to support progressive-only formats? [1=Y/0=N]\n");
                pContext->encodeSettings.videoEncoderMemConfig.progressiveOnly = getValue(key);
                printf("Video encoder max width: \n");
                pContext->encodeSettings.videoEncoderMemConfig.maxWidth = getValue(key);
                printf("Video encoder max height: \n");
                pContext->encodeSettings.videoEncoderMemConfig.maxHeight = getValue(key);
                /* update encoder startSettings memory bound */
                if(pContext->encodeSettings.videoEncoderMemConfig.maxWidth) {
                    pContext->vidEncoderStartSettings.bounds.inputDimension.max.width = pContext->encodeSettings.videoEncoderMemConfig.maxWidth;
                }
                if(pContext->encodeSettings.videoEncoderMemConfig.maxHeight) {
                    pContext->vidEncoderStartSettings.bounds.inputDimension.max.height = pContext->encodeSettings.videoEncoderMemConfig.maxHeight;
                }
                break;
            case 37:
                NEXUS_StreamMux_GetSettings(pContext->streamMux, &muxSettings);
                printf("Enable video channel? [1=Y/0=N]\n");
                muxSettings.enable.video[0] = getValue(key);
                printf("Enable audio channel? [1=Y/0=N]\n");
                muxSettings.enable.audio[0] = getValue(key);
                NEXUS_StreamMux_SetSettings(pContext->streamMux, &muxSettings);
                break;
            case 38:
                printf("Cxt entropy coding setting: %s\n", pContext->encodeSettings.entropyCoding==NEXUS_EntropyCoding_eAuto?
                    "Auto" : (pContext->encodeSettings.entropyCoding==NEXUS_EntropyCoding_eCabac?"CABAC":"CAVLC"));
                printf("Change entropy coding to:\n");
                print_value_list(g_entropyCodingStrs);
                pContext->encodeSettings.entropyCoding = getNameValue(key, g_entropyCodingStrs);
                break;
            case 39: /* source mute mode */
                if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
                    NEXUS_VideoDecoderSettings settings;
                    NEXUS_VideoDecoder_GetSettings(pContext->videoDecoder,&settings);
                    settings.mute ^= true;
                    fprintf(stderr, "%s the video input\n", settings.mute? "mute":"unmute");
                    NEXUS_VideoDecoder_SetSettings(pContext->videoDecoder,&settings);
                }
                break;
            case 50: /* random dynamic resolution change */
#define BTST_MIN_XCODE_RESOLUTION_X 64
#define BTST_MIN_XCODE_RESOLUTION_Y 64
#define BTST_MAX_XCODE_RESOLUTION_X 1920
#define BTST_MAX_XCODE_RESOLUTION_Y 1080
                printf("random resolution change...\n");
                printf("How many times? "); choice = getValue(key);
                while(choice--) {
                    pContext->encodeSettings.bCustom = true;
                    pContext->encodeSettings.customFormatSettings.width = (rand()%(BTST_MAX_XCODE_RESOLUTION_X+1))& (~0xf);/* MB aligned*/
                    if(pContext->encodeSettings.customFormatSettings.width < BTST_MIN_XCODE_RESOLUTION_X)
                        pContext->encodeSettings.customFormatSettings.width += BTST_MIN_XCODE_RESOLUTION_X;
                    pContext->encodeSettings.customFormatSettings.height = rand()%(BTST_MAX_XCODE_RESOLUTION_Y+1) & (~0xf);
                    if(pContext->encodeSettings.customFormatSettings.height < BTST_MIN_XCODE_RESOLUTION_Y)
                        pContext->encodeSettings.customFormatSettings.height += BTST_MIN_XCODE_RESOLUTION_Y;
                    pContext->encodeSettings.customFormatSettings.interlaced = false;
                    pContext->encodeSettings.customFormatSettings.refreshRate = 30000;
                    pContext->encodeSettings.customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_eSar;
                    pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x = 1;
                    pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y = 1;
                    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
                    customFormatSettings.width = pContext->encodeSettings.customFormatSettings.width;
                    customFormatSettings.height = pContext->encodeSettings.customFormatSettings.height;
                    customFormatSettings.refreshRate = pContext->encodeSettings.customFormatSettings.refreshRate;
                    customFormatSettings.interlaced = pContext->encodeSettings.customFormatSettings.interlaced;
                    customFormatSettings.aspectRatio = pContext->encodeSettings.customFormatSettings.aspectRatio;
                    customFormatSettings.sampleAspectRatio.x = pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x;
                    customFormatSettings.sampleAspectRatio.y = pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y;
                    customFormatSettings.dropFrameAllowed = true;
                    printf("resolution loop %d: %ux%up30\n", choice, pContext->encodeSettings.customFormatSettings.width, pContext->encodeSettings.customFormatSettings.height);
                    rc = NEXUS_Display_SetCustomFormatSettings(pContext->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
                    BDBG_ASSERT(!rc);
                    BKNI_Sleep(3000); /* 3sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            case 51: /* random dynamic bitrate change */
#define BTST_MIN_XCODE_BITRATE 10000/* 10Kbps */
#define BTST_MAX_XCODE_BITRATE 30000000 /* 20 Mbps */
                printf("random bitrate change...\n");
                printf("How many times? "); scanf("%d", &choice);
                while(choice--) {
                    pContext->encodeSettings.encoderBitrate = rand()%BTST_MAX_XCODE_BITRATE;
                    if(pContext->encodeSettings.encoderBitrate < BTST_MIN_XCODE_BITRATE) pContext->encodeSettings.encoderBitrate += BTST_MIN_XCODE_BITRATE;
                    NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
                    videoEncoderConfig.bitrateMax = pContext->encodeSettings.encoderBitrate;
                    printf("bps loop %d: %u\n", choice, videoEncoderConfig.bitrateMax);
                    rc = NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
                    BDBG_ASSERT(!rc);
                    BKNI_Sleep(3000); /* 3sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            case 52:/* stop mux to overflow encoder for error recovery test */
                /* bringdown loopback path */
                if(g_bLoopbackPlayer && g_selectedXcodeContextId == g_loopbackXcodeId && g_loopbackStarted) {
                    g_loopbackStarted = false;
                    xcode_loopback_shutdown(pContext);
                }
                xcode_stop_systemdata(pContext);
                NEXUS_StreamMux_Stop(pContext->streamMux);
                pContext->bPrematureMuxStop = true;
                break;
            case 53: /* back to back stop/start test */
                printf("back to back stop/start test...\n");
                printf("How many times? "); choice = getValue(key);
                printf("Complete file transcode for each loop? [1=y/0=n]: "); input = getValue(key);
                while(choice--) {
                    while(input) {
                        B_Mutex_Unlock(pContext->mutexStarted);
                        BKNI_Sleep(1000);
                        B_Mutex_Lock(pContext->mutexStarted);
                        if(!pContext->bStarted) {
                            break;
                        }
                    }
                    /* bringdown loopback path */
                    if(g_bLoopbackPlayer && g_selectedXcodeContextId == g_loopbackXcodeId && g_loopbackStarted) {
                        g_loopbackStarted = false;
                        xcode_loopback_shutdown(pContext);
                    }
                    /* stop & close transcoder */
                    shutdown_transcode(pContext);
                    printf("start loop %d\n", choice);
                    /* open & start transcoder */
                    bringup_transcode(pContext);
                    if(g_bLoopbackPlayer && (g_selectedXcodeContextId == g_loopbackXcodeId) && !g_loopbackStarted &&
                       (BTST_LOOPBACK_DISPLAY_IDX != get_display_index(pContext->contextId))) {
                        xcode_loopback_setup(pContext);
                        g_loopbackStarted = true;
                    }
                    BKNI_Sleep(10000); /* 10 sec per loop */
                }
                /* resolution change typically could come with bit rate change; separate for now */
                break;
            case 100:
                printf("\n How many seconds to sleep?");
                BKNI_Sleep(1000*getValue(key));
                break;
            case 90: /* source trick mode player */
                if(pContext->inputSettings.resource == BTST_RESOURCE_FILE) {
                    B_Mutex_Unlock(pContext->mutexStarted);
                    sourcePlayer(pContext);
                    B_Mutex_Lock(pContext->mutexStarted);
                }
                break;
            case 99: /* debug module setting */
#if BDBG_DEBUG_BUILD
{
                char achName[256];
                printf("\nPlease enter the debug module name: ");
                strcpy(achName, getString(key));
                printf("(%d)Trace (%d)Message (%d)Warning (%d)Error\n",
                    BDBG_eTrace, BDBG_eMsg, BDBG_eWrn, BDBG_eErr);
                printf("Which debug level do you want to set it to? ");
                BDBG_SetModuleLevel(achName, getValue(key));
}
#endif
                break;
            default:
                break;
        }
        B_Mutex_Unlock(pContext->mutexStarted);
    }
Done:
    if(g_eotEvent) {
        B_Event_Set(g_eotEvent);
    }
}

static void message_overflow_callback(void *context, int param)
{
    TranscodeContext  *pContext = (TranscodeContext  *)context;
    printf("#### Context%d message PID[%d] buffer overflows! ###\n", pContext->contextId, param);
}
static void message_psi_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}
#define PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define PROGRAM_INFO_LENGTH(buf) (TS_READ_16(&buf[PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define DESCRIPTOR_BASE(buf) (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define STREAM_BASE(buf) (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + PROGRAM_INFO_LENGTH(buf))

static int TS_PMT_P_getStreamByteOffset( const uint8_t *buf, unsigned bfrSize, int streamNum )
{
    int byteOffset;
    int i;

    /* After the last descriptor */
    byteOffset = STREAM_BASE(buf);

    for (i=0; i < streamNum; i++)
    {
        if (byteOffset >= (int)bfrSize || byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf))
            return -1;
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
    }

    return byteOffset;
}

int add_psi_filter(const void *context, unsigned short pid, BKNI_EventHandle event)
{
    unsigned i;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_Error rc;
    psi_message_t *msg = NULL;
    TranscodeContext  *pContext = (TranscodeContext  *)context;

    for (i=0;i<BTST_MAX_MESSAGE_FILTERS;i++) {
        if (!pContext->psi_message[i].message) {
            msg = &pContext->psi_message[i];
            break;
        }
    }
    if (!msg) {
        return -1;
    }
    BDBG_MSG(("adding PSI filter[%u] for PID %u", i, pid));
    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE) {
        msg->pidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, pid, NULL);
    } else {
        msg->pidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pid, NULL);
    }
    BDBG_ASSERT(msg->pidChannel);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = message_psi_callback;
    settings.dataReady.context = event;
    msg->message = NEXUS_Message_Open(&settings);
    msg->pid = pid;
    msg->done = false;

    NEXUS_Message_GetDefaultStartSettings(msg->message, &startSettings);
    startSettings.pidChannel = msg->pidChannel;
    rc = NEXUS_Message_Start(msg->message, &startSettings);
    BDBG_ASSERT(!rc);

    return 0;
}

/* get TS user data PSI info from input PMT */
static void getUserDataPsiFromPmt(void *context)
{
    TranscodeContext  *pContext = (TranscodeContext  *)context;
    BKNI_EventHandle event;
    NEXUS_Error rc;
    unsigned i=0, count = 0, loop = 0;
    bool bFound = false;

    /* 1) parse input PAT and set up PMT filters if no specified PMT PID;
     * 2) parse each PMTs to find one with matching video PID;
     * 3) match selected user data stream PID;
     * 4) get user data descriptors;
     */

    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE) {
        /* assume source playback is not started yet, we need to get PSI about user data */
        NEXUS_Playback_Start(pContext->playback, pContext->file, NULL);
    }

    /* to get input PMT, need to set up message filter */
    BKNI_CreateEvent(&event);

    BDBG_WRN(("starting PSI filter PID = %u", pContext->inputSettings.pmtPid));

    /* if user specified PMT PID, msg[0] is for the PMT; else msg[0] is for PAT; */
    add_psi_filter(pContext, pContext->inputSettings.pmtPid, event);

    /* Read the PAT/PMT */
    for (count=0;;count++) {
        const uint8_t *buffer;
        size_t size;
        unsigned programNum, message_length, streamNum;
        size_t num = NEXUS_MAX_MUX_PIDS;

        if (count == BTST_MAX_MESSAGE_FILTERS) {
            count = 0;
            if(++loop > BTST_MAX_MESSAGE_FILTERS) {BDBG_ERR(("failed to get input user data PMT!")); rc = -1; break;}
        }

        if (!pContext->psi_message[count].message || pContext->psi_message[count].done) {
            continue;
        }

        rc = NEXUS_Message_GetBuffer(pContext->psi_message[count].message, (const void **)&buffer, &size);
        BDBG_ASSERT(!rc);

        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(event, 5 * 1000); /* wait 5 seconds */
            if (rc == NEXUS_TIMEOUT) {
                BDBG_WRN(("no PSI messages"));
                rc = -1;
                break;
            }
            BDBG_ASSERT(!rc);
            continue;
        }

        /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
        message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
        BDBG_ASSERT(size >= (size_t)message_length);
        BDBG_MSG(("message[%u] size = "BDBG_UINT64_FMT", table ID = %u", count, BDBG_UINT64_ARG((uint64_t)size), buffer[0]));

        if (buffer[0] == 0) {
            /* 1) Program Association Table */
            fprintf(stderr, "PAT: size=%d\n", message_length);
            for (programNum=0;programNum<(unsigned)(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
                unsigned byteOffset = 8 + programNum*4;
                unsigned program = TS_READ_16( &buffer[byteOffset] );
                unsigned short pid = (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF);
                fprintf(stderr, "  program %d: PID %u\n", program, pid);
                /* add PMT filters for all programs */
                add_psi_filter(pContext, pid, event);
            }
        }
        else if (buffer[0] == 2) { /* PMT */
            TS_PMT_stream pmtStream;

            /* Program Table */
            fprintf(stderr,"Context%d found PMT PID[%u]:\nprogram number %d size=%d\n", pContext->contextId,
                pContext->psi_message[count].pid, TS_READ_16(&buffer[3]), message_length);
            /* need to validate the PMT section */
            if(!TS_PMT_validate(buffer, size)) {BDBG_ERR(("invalid PMT")); goto Done_getUserDataPsi;}

            streamNum = TS_PMT_getNumStreams(buffer, size);
            fprintf(stderr, "total streams: %d\n", streamNum);

            /* 2) search for all streams to match the video PID if no specified PMT PID */
            if(0 == pContext->inputSettings.pmtPid) {
                for (i=0;i<streamNum;i++) {
                    TS_PMT_getStream(buffer, size, i, &pmtStream);
                    fprintf(stderr, "\tPID: %d, strem_type: %d\n", pmtStream.elementary_PID, pmtStream.stream_type);
                    if(pmtStream.elementary_PID == pContext->inputSettings.iVideoPid) {
                        fprintf(stderr, "Found matching video PID!\n");
                        break;
                    }
                }
                if(i == streamNum)  goto Done_getUserDataPsi; /* not found so continue to next PMT */
            }
            bFound = true;/* found PMT */

            /* else, found the matching program */
            /* 3) search for all streams to extract user data PSI info */
            for (i=0;i<streamNum;i++) {
                unsigned streamOffset;

                TS_PMT_getStream(buffer, size, i, &pmtStream);

                /* 2) match user data PID */
                if(pContext->inputSettings.numUserDataPids == BTST_TS_USER_DATA_ALL) {
                    /* all pass the VBI user data PES streams */
                    if(pmtStream.stream_type != TS_PSI_ST_13818_1_PrivatePES) continue;
                    if(num == NEXUS_MAX_MUX_PIDS) num = 0;/* start from 0 */
                    else {
                        ++num;
                        if(num >= NEXUS_MAX_MUX_PIDS) break;
                    }
                    pContext->inputSettings.userDataPid[num] = pmtStream.elementary_PID;
                } else {
                    for(num=0; num < pContext->inputSettings.numUserDataPids; num++) {
                        /* 3) bingo! save the stream info and stream descriptor */
                        if(pmtStream.elementary_PID == (uint16_t)pContext->inputSettings.userDataPid[num]) {
                            break;
                        }
                    }
                    /* not found, check next stream */
                    if(num == pContext->inputSettings.numUserDataPids) continue;
                }

                /* 4) save user data PSI info */
                fprintf(stderr, "\tuser data PID: %d, strem_type: %d\n", pmtStream.elementary_PID, pmtStream.stream_type);
                /* save pmt stream info and remap PID */
                pContext->userDataStream[num] = pmtStream;
                if(pContext->bRemapUserDataPid) {
                    pContext->userDataStream[num].elementary_PID = pContext->inputSettings.remapUserDataPid[num];
                } else {
                    pContext->inputSettings.remapUserDataPid[num] = pContext->userDataStream[num].elementary_PID;
                }
                /* save stream descriptor size */
                streamOffset = TS_PMT_P_getStreamByteOffset(buffer, size, i);
                pContext->userDataDescLen[num] = TS_READ_16(&buffer[streamOffset+3])&0x3FF;
                fprintf(stderr, "\tdescriptor length: %d\n", pContext->userDataDescLen[num]);

                /* sanity check descriptor size */
                if(pContext->userDataDescLen[num] > 188) {
                BDBG_ERR(("User data descriptor length %d too long!",pContext->userDataDescLen[num]));
                    pContext->bUserDataStreamValid[num] = false;/* invalidate */
                    goto Done_getUserDataPsi;
                }
                BKNI_Memcpy(pContext->userDataDescriptors[num],&buffer[streamOffset+5],pContext->userDataDescLen[num]);
                /* mark it valid finally */
                pContext->bUserDataStreamValid[num] = true;
            }
            if(pContext->inputSettings.numUserDataPids == BTST_TS_USER_DATA_ALL) {
                pContext->inputSettings.numUserDataPids = num+1;/* found num of user data streams */
                BDBG_MSG(("Context%d found "BDBG_UINT64_FMT" user data PIDs to pass through.", pContext->contextId, BDBG_UINT64_ARG((uint64_t)pContext->inputSettings.numUserDataPids)));
            }

        }
Done_getUserDataPsi:
        /* XPT HW is configured to pad all messages to 4 bytes. If we are calling NEXUS_Message_ReadComplete
        based on message length and not the size returned by NEXUS_Message_GetBuffer, then we must add that pad.
        If we are wrong, NEXUS_Message_ReadComplete will fail. */
        if (message_length % 4) {
            message_length += 4 - (message_length % 4);
        }
        /* only complete one PMT */
        rc = NEXUS_Message_ReadComplete(pContext->psi_message[count].message, message_length);
        BDBG_ASSERT(!rc);

        pContext->psi_message[count].done = true; /* don't parse this table any more */

        /* Only do once. TODO: may periodically watch for updated PMT info. */
        if(bFound) break;
    }

    for (i=0;i<BTST_MAX_MESSAGE_FILTERS;i++) {
        if (pContext->psi_message[i].message) {
            NEXUS_Message_Close(pContext->psi_message[i].message);
            pContext->psi_message[i].message = NULL;
        }
        if (pContext->psi_message[i].pidChannel) {
            if(pContext->inputSettings.resource == BTST_RESOURCE_FILE) {
                NEXUS_Playback_ClosePidChannel(pContext->playback, pContext->psi_message[i].pidChannel);
            }
            else {
                NEXUS_PidChannel_Close(pContext->psi_message[i].pidChannel);
            }
            pContext->psi_message[i].pidChannel = NULL;
        }
    }
    /* free the event resource */
    BKNI_DestroyEvent(event);

    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE) {
        NEXUS_Playback_Stop(pContext->playback);
    }
}

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189
void add_pat_pmt(
    TranscodeContext *pContext,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    uint8_t  vidStreamType,
    uint8_t  audStreamType
)
{
    uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;
    int i;

    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, BTST_MUX_PMT_PID);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

    if(pcrPid) {
       TS_PMT_setPcrPid(&pmtState, pcrPid);
    }

    if(vidPid) {
        TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    if(audPid) {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    /* add user data PID stream PSI info */
    if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
        getUserDataPsiFromPmt(pContext);
        for(i=0; i<NEXUS_MAX_MUX_PIDS; i++) {
            if(pContext->bUserDataStreamValid[i]) {
                TS_PMT_addStream(&pmtState, &pContext->userDataStream[i], &streamNum);
                TS_PMT_setDescriptor(&pmtState,
                    pContext->userDataDescriptors[i],
                    pContext->userDataDescLen[i],
                    streamNum);
            }
        }
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MSG(("\nContext%d output PMT section:", pContext->contextId));
    for(i=0; i < (int)pmtState.size; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pat[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pat[0] + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pmt[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pmt[0] + buf_used, pmt_pl_buf, pmt_pl_size);
    BDBG_MSG(("\nContext%d output PMT packet:", pContext->contextId));
    for(i=0; i < BTST_TS_HEADER_BUF_LENGTH; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x",
            *((uint8_t*)pContext->pmt[0]+i), *((uint8_t*)pContext->pmt[0]+i+1), *((uint8_t*)pContext->pmt[0]+i+2), *((uint8_t*)pContext->pmt[0]+i+3),
            *((uint8_t*)pContext->pmt[0]+i+4), *((uint8_t*)pContext->pmt[0]+i+5), *((uint8_t*)pContext->pmt[0]+i+6), *((uint8_t*)pContext->pmt[0]+i+7)));
    }

}

static void insertSystemDataTimer(void *context)
{
    TranscodeContext *pContext = context;
    uint8_t ccByte;

    ++pContext->ccValue;/* increment CC synchronously with PAT/PMT */
    ccByte = *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PAT pointer */
    pContext->psi[0].pData = (void*)((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 1);

    ccByte = *((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4);
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PMT pointer */
    pContext->psi[1].pData = (void*)((uint8_t*)pContext->pmt[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 1);

    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[1]);
    BDBG_MODULE_MSG(contcounter,("Context%d insert PAT&PMT... ccPAT = %x ccPMT=%x", pContext->contextId, *((uint8_t*)pContext->pat[pContext->ccValue % BTST_PSI_QUEUE_CNT] + 4) & 0xf,
        *((uint8_t*)pContext->pmt[pContext->ccValue  % BTST_PSI_QUEUE_CNT] + 4) & 0xf));
    if(pContext->systemdataTimerIsStarted)
    {
        pContext->systemdataTimer = B_Scheduler_StartTimer(
            pContext->schedulerSystemdata,pContext->mutexSystemdata, 1000, insertSystemDataTimer, pContext);
        if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error %d", NEXUS_OUT_OF_SYSTEM_MEMORY));}
    }
    return;
}

/*******************************
 * Add system data to stream_mux
 */
static void xcode_create_systemdata( TranscodeContext  *pContext )
{
    uint8_t vidStreamType = 0, audStreamType = 0;
    uint16_t audPid = 0;
    unsigned i;
    NEXUS_AudioCodec audCodec = NEXUS_AudioCodec_eUnknown;

    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pContext->pat[i]);
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pContext->pmt[i]);
    }

    /* decide the stream type to set in PMT */
    if(!pContext->bNoVideo) {
        switch(pContext->encodeSettings.encoderVideoCodec)
        {
            case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
            case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
            case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
            case NEXUS_VideoCodec_eH265:          vidStreamType = 0x24; break;
            case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
            case NEXUS_VideoCodec_eVp9:           vidStreamType = 0x00; break; /* VP9 in TS doesn't actually work, but this is in place for VCE regression testing */
            default:
                BDBG_ERR(("Video encoder codec %d is not supported!\n", pContext->encodeSettings.encoderVideoCodec));
                BDBG_ASSERT(0);
        }
    }
    if(pContext->encodeSettings.bAudioEncode)
    {/* audio transcode */
        audCodec = pContext->encodeSettings.encoderAudioCodec;
        audPid = BTST_MUX_AUDIO_PID;
    }
    else if(pContext->inputSettings.bAudioInput)
    {/* audio passthrough */
        audCodec = pContext->inputSettings.eAudioCodec;
        audPid = BTST_MUX_AUDIO_PID;
    }
    if(pContext->inputSettings.bAudioInput)
    {
        switch(audCodec)
        {
            case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
            case NEXUS_AudioCodec_eLpcm1394:     audStreamType = 0x83; break;
            default:
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audCodec));
        }
    }

    add_pat_pmt(pContext, (0 != g_pcrInterval ) ? BTST_MUX_PCR_PID : 0, BTST_MUX_VIDEO_PID, audPid, vidStreamType, audStreamType);
    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        if(i > 0)
        {
            BKNI_Memcpy(pContext->pat[i], pContext->pat[0], BTST_TS_HEADER_BUF_LENGTH);
            BKNI_Memcpy(pContext->pmt[i], pContext->pmt[0], BTST_TS_HEADER_BUF_LENGTH);
        }
        NEXUS_Memory_FlushCache(pContext->pat[i], BTST_TS_HEADER_BUF_LENGTH);
        NEXUS_Memory_FlushCache(pContext->pmt[i], BTST_TS_HEADER_BUF_LENGTH);
    }
    BKNI_Memset(pContext->psi, 0, sizeof(pContext->psi));
    pContext->psi[0].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[0].pData = (void*)((uint8_t*)pContext->pat[0] + 1);
    pContext->psi[0].timestampDelta = 0;
    pContext->psi[1].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[1].pData = (void*)((uint8_t*)pContext->pmt[0] + 1);
    pContext->psi[1].timestampDelta = 0;
    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[1]);
    BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->pat[0] + 4) & 0xf,
        *((uint8_t*)pContext->pmt[0] + 4) & 0xf));

}

static void xcode_start_systemdata( TranscodeContext  *pContext )
{
    if(!pContext->systemdataTimerIsStarted) {
        /* schedule a periodic timer to insert PAT/PMT */
        B_ThreadSettings settingsThread;
        pContext->mutexSystemdata = B_Mutex_Create(NULL);
        pContext->schedulerSystemdata = B_Scheduler_Create(NULL);
        /* create thread to run scheduler */
        B_Thread_GetDefaultSettings(&settingsThread);
        pContext->schedulerThread = B_Thread_Create("systemdata_Scheduler",
            (B_ThreadFunc)B_Scheduler_Run,
            pContext->schedulerSystemdata,
            &settingsThread);
        if (NULL == pContext->schedulerThread)
        {
            BDBG_ERR(("failed to create scheduler thread"));
        }
        pContext->systemdataTimer = B_Scheduler_StartTimer(
            pContext->schedulerSystemdata,pContext->mutexSystemdata, 1000, insertSystemDataTimer, pContext);
        if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error"));}
        pContext->systemdataTimerIsStarted = true;
    }
}

static void xcode_stop_systemdata( TranscodeContext  *pContext )
{
    /* cancel system data timer */
    if(pContext->systemdataTimerIsStarted)
    {
        B_Scheduler_CancelTimer(pContext->schedulerSystemdata, pContext->systemdataTimer);
        B_Scheduler_Stop(pContext->schedulerSystemdata);
        B_Scheduler_Destroy(pContext->schedulerSystemdata);
        if (pContext->schedulerThread)
        {
            B_Thread_Destroy(pContext->schedulerThread);
            pContext->schedulerThread = NULL;
        }
        B_Mutex_Destroy(pContext->mutexSystemdata);
        pContext->systemdataTimer = NULL;
        pContext->systemdataTimerIsStarted = false;
    }
}

static void xcode_release_systemdata( TranscodeContext  *pContext )
{
    unsigned i;

    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        NEXUS_Memory_Free(pContext->pat[i]);
        NEXUS_Memory_Free(pContext->pmt[i]);
    }
}

/************************************
 * Set up encoder AV sync.
 * encode setting and startSetting to be set after end-to-end delay is determined */
/* get end-to-end delay (Dee) for audio and video encoders;
* TODO: match AV delay! In other words,
*   if (aDee > vDee) {
*       vDee' = aDee' = aDee;
*   }
*   else {
*       vDee' = aDee' = vDee;
*   }
*/
static void xcode_av_sync(
    TranscodeContext  *pContext,
    NEXUS_VideoEncoderSettings *pVideoEncoderConfig,
    NEXUS_VideoEncoderStartSettings *pVideoEncoderStartConfig )
{
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    unsigned Dee = 0;

    /******************************************
     * add configurable delay to video path
     */
    /* disable Inverse Telecine Field Pairing for extreme low delay mode
     * NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
     * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
    if(!pContext->bNoVideo) {
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
        pVideoEncoderStartConfig->bounds.inputDimension.max.width = BTST_INPUT_MAX_WIDTH;
        pVideoEncoderStartConfig->bounds.inputDimension.max.height = BTST_INPUT_MAX_HEIGHT;
        if(pContext->encodeSettings.bCustom)
        {/* custom DSP transcode resolution; TODO: remove this if soft encoder can support dynamic resolution. */
            pVideoEncoderStartConfig->bounds.inputDimension.max.width = (BTST_INPUT_MAX_WIDTH<pContext->encodeSettings.customFormatSettings.width)?
                BTST_INPUT_MAX_WIDTH : pContext->encodeSettings.customFormatSettings.width;
            pVideoEncoderStartConfig->bounds.inputDimension.max.height = (BTST_INPUT_MAX_HEIGHT<pContext->encodeSettings.customFormatSettings.height)?
                BTST_INPUT_MAX_HEIGHT: pContext->encodeSettings.customFormatSettings.height;
        }
    #if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
        else {
            NEXUS_VideoFormatInfo fmtInfo;
            NEXUS_VideoFormat_GetInfo(pContext->encodeSettings.displayFormat, &fmtInfo);
            pVideoEncoderStartConfig->bounds.inputDimension.max.width = (BTST_INPUT_MAX_WIDTH<fmtInfo.width)?
                BTST_INPUT_MAX_WIDTH : fmtInfo.width;
            pVideoEncoderStartConfig->bounds.inputDimension.max.height = (BTST_INPUT_MAX_HEIGHT<fmtInfo.height)?
                BTST_INPUT_MAX_HEIGHT: fmtInfo.height;
        }
    #endif
#else
        if(pContext->bCustomizeDelay)
        {
            pVideoEncoderConfig->enableFieldPairing = pContext->encodeSettings.enableFieldPairing;

            /* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
            pVideoEncoderStartConfig->rateBufferDelay = pContext->encodeSettings.rateBufferDelay;

            /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
             * Note: lower minimum framerate means longer encode delay */
            pVideoEncoderStartConfig->bounds.inputFrameRate.min = pContext->encodeSettings.bounds.inputFrameRate.min;

            /* to allow 15 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
            pVideoEncoderStartConfig->bounds.outputFrameRate.min = pContext->encodeSettings.bounds.outputFrameRate.min;
            pVideoEncoderStartConfig->bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

            /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
            if(pContext->encodeSettings.bounds.inputDimension.max.width) {
                pVideoEncoderStartConfig->bounds.inputDimension.max.width = pContext->encodeSettings.bounds.inputDimension.max.width;
            }
            if(pContext->encodeSettings.bounds.inputDimension.max.height) {
                pVideoEncoderStartConfig->bounds.inputDimension.max.height = pContext->encodeSettings.bounds.inputDimension.max.height;
            }

            pVideoEncoderStartConfig->bounds.bitrate.upper.bitrateMax = pContext->encodeSettings.bounds.bitrate.upper.bitrateMax;
            pVideoEncoderStartConfig->bounds.bitrate.upper.bitrateTarget = pContext->encodeSettings.bounds.bitrate.upper.bitrateTarget;
            pVideoEncoderStartConfig->bounds.streamStructure.max.framesB = pContext->encodeSettings.bounds.streamStructure.max.framesB;
        }
        else
        {
            pVideoEncoderConfig->enableFieldPairing = g_bEnableFieldPairing;

            /* 0 to use default rate buffer delay; */
            pVideoEncoderStartConfig->rateBufferDelay = 0;

            /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
             * Note: lower minimum framerate means longer encode delay */
            pVideoEncoderStartConfig->bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

            /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
            pVideoEncoderStartConfig->bounds.inputDimension.max.width =
                (pContext->encodeSettings.videoEncoderMemConfig.maxWidth)?
                pContext->encodeSettings.videoEncoderMemConfig.maxWidth : 1920;
            pVideoEncoderStartConfig->bounds.inputDimension.max.height =
                pContext->encodeSettings.videoEncoderMemConfig.maxHeight?
                pContext->encodeSettings.videoEncoderMemConfig.maxHeight : 1088;
        }
#endif

        /*****************************************
         * calculate video encoder A2P delay
         */
        /* NOTE: video encoder delay is in 27MHz ticks; the min is based on the bound settings. */
        NEXUS_VideoEncoder_GetDelayRange(pContext->videoEncoder, pVideoEncoderConfig, pVideoEncoderStartConfig, &videoDelay);
        Dee = videoDelay.min;
        BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
    }

    if(pContext->inputSettings.bAudioInput)
    {
        NEXUS_AudioMuxOutput_GetDelayStatus(pContext->audioMuxOutput, pContext->encodeSettings.encoderAudioCodec, &audioDelayStatus);
        BDBG_WRN(("\n\tAudio codec %d end-to-end delay = %u ms", pContext->encodeSettings.encoderAudioCodec, audioDelayStatus.endToEndDelay));

        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */

        if(!pContext->bNoVideo) {
            if(Dee > videoDelay.min)
            {
                if(Dee > videoDelay.max)
                {
                    BDBG_ERR(("Audio Dee is way too big! Use video Dee max!"));
                    Dee = videoDelay.max;
                }
                else
                {
                    BDBG_WRN(("Use audio Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
                }
            }
            else
            {
                Dee = videoDelay.min;
                BDBG_WRN(("Use video Dee %u ms %u ticks@27Mhz!", Dee/27000, Dee));
            }
            pVideoEncoderConfig->encoderDelay = Dee;
        }

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        /* audio NRT requires mux out to take NRT audio decode STC which is assigned with transcode STC. */
        audioMuxStartSettings.stcChannel        = pContext->stcChannelTranscode;
        audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
        audioMuxStartSettings.nonRealTime       = pContext->bNonRealTime;
        pContext->audMuxStartSettings = audioMuxStartSettings;
    }
    else
        pVideoEncoderConfig->encoderDelay = Dee;

    /* store the encoder A2P delay based on the bounds and av sync for mux resource allocation */
    pContext->encodeDelay = Dee;
}

static void xcode_index_filename(char *indexFile, const char *mediaFile )
{
    char *tmpPtr=NULL;
    int size = strlen(mediaFile);
    strcpy(indexFile, mediaFile);
    while(size-->1) {
        if(indexFile[size] == '.') {
            tmpPtr = &indexFile[size];
            break;
        }
    }
    if (tmpPtr) {
        strcpy(tmpPtr+1, "nav");
    }
    else {
        strcat(indexFile, ".nav");
    }
    BDBG_MSG(("Media file name: %s, index file name %s", mediaFile, indexFile));
}

static void recpumpOverflowCallback(void *context, int param)
{
    TranscodeContext  *pContext = (TranscodeContext  *)context;
    BSTD_UNUSED(param);
    BDBG_ERR(("\n#### Context%d stream mux recpump buffer overflows! ###\n", pContext->contextId));
}

/* file i/o page size */
#define BTST_RECORD_PAGE_SIZE        (4096)
#define BTST_RECORD_ATOM_SIZE        (47*BTST_RECORD_PAGE_SIZE)
/* nexus_record limits the per file write transaction size */
#define BTST_RECORD_WRITE_SIZE_LIMIT (3*BTST_RECORD_ATOM_SIZE)

/*******************************
 * Set up stream_mux and record
 */
static void xcode_setup_mux_record( TranscodeContext  *pContext )
{
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecordSettings recordSettings;
    NEXUS_MessageSettings messageSettings;
    NEXUS_MessageStartSettings messageStartSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    size_t i;
    bool bTtsOutput = (NEXUS_TransportTimestampType_eNone != g_TtsOutputType);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = g_muxDescs; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;

    if (pContext->bMCPB) {
       pContext->playpumpTranscodeMCPB = NEXUS_Playpump_Open(BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX+pContext->contextId*4, &playpumpConfig);
      BDBG_MSG(("Transcoder%d opened TS mux MCPB playpump%d [%p].", pContext->contextId, BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX+pContext->contextId*4, (void*)pContext->playpumpTranscodeVideo));
      NEXUS_Playpump_GetSettings(pContext->playpumpTranscodeMCPB, &playpumpSettings);
      playpumpSettings.transportType = NEXUS_TransportType_eEs;
      playpumpSettings.timestamp.forceRestamping = bTtsOutput;
      NEXUS_Playpump_SetSettings(pContext->playpumpTranscodeMCPB, &playpumpSettings);
      assert(pContext->playpumpTranscodeMCPB);
    } else {
       pContext->playpumpTranscodeMCPB = NULL;
    }

    if(!pContext->bNoVideo) {
       if (NULL == pContext->playpumpTranscodeMCPB) {
          pContext->playpumpTranscodeVideo = NEXUS_Playpump_Open(BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX+pContext->contextId*4, &playpumpConfig);
          BDBG_MSG(("Transcoder%d opened TS mux video PES playpump%d [%p].", pContext->contextId, BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX+pContext->contextId*4, (void*)pContext->playpumpTranscodeVideo));
#if BTST_DISABLE_VIDEO_ENCODER_PES_PACING
          NEXUS_Playpump_SuspendPacing(pContext->playpumpTranscodeVideo, true);
#endif
         NEXUS_Playpump_GetSettings(pContext->playpumpTranscodeVideo, &playpumpSettings);
         playpumpSettings.timestamp.forceRestamping = bTtsOutput;
         NEXUS_Playpump_SetSettings(pContext->playpumpTranscodeVideo, &playpumpSettings);
       } else {
          pContext->playpumpTranscodeVideo = pContext->playpumpTranscodeMCPB;
       }
        assert(pContext->playpumpTranscodeVideo);
    }

   if (NULL == pContext->playpumpTranscodeMCPB) {
      pContext->playpumpTranscodePcr = NEXUS_Playpump_Open(BTST_STREAM_MUX_PCR_PLAYPUMP_IDX+pContext->contextId*4, &playpumpConfig);
      BDBG_MSG(("Transcoder%d opened TS mux PCR playpump%d [%p].", pContext->contextId, BTST_STREAM_MUX_PCR_PLAYPUMP_IDX+pContext->contextId*4, (void*)pContext->playpumpTranscodePcr));
      NEXUS_Playpump_GetSettings(pContext->playpumpTranscodePcr, &playpumpSettings);
      playpumpSettings.timestamp.forceRestamping = bTtsOutput || pContext->bRemux;
      playpumpSettings.blindSync = true; /* PCR channel has full transport packets, so blind sync mode */
      NEXUS_Playpump_SetSettings(pContext->playpumpTranscodePcr, &playpumpSettings);
   } else {
      pContext->playpumpTranscodePcr = pContext->playpumpTranscodeMCPB;
   }
    assert(pContext->playpumpTranscodePcr);

    BKNI_CreateEvent(&pContext->finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = pContext->finishEvent;
    {/* reduce stream mux memory allocation if no TS user data passthru */
        NEXUS_StreamMuxConfiguration streamMuxConfig;
        NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfig);
        if(!pContext->inputSettings.bTsUserDataInput) {
            streamMuxConfig.userDataPids = 0;/* remove unnecessary memory allocation */
        } else if(pContext->inputSettings.numUserDataPids == BTST_TS_USER_DATA_ALL) {
            streamMuxConfig.userDataPids = NEXUS_MAX_MUX_PIDS;
        } else {
            streamMuxConfig.userDataPids = pContext->inputSettings.numUserDataPids;
        }
        streamMuxConfig.audioPids = pContext->inputSettings.bAudioInput? 1 : 0;
        streamMuxConfig.latencyTolerance = g_muxLatencyTolerance;
        streamMuxConfig.servicePeriod    = g_msp;
        streamMuxConfig.nonRealTime      = pContext->bNonRealTime;
        streamMuxConfig.muxDelay         = pContext->encodeDelay;/* A2P delay affects mux resource allocation */
        streamMuxConfig.supportTts       = (g_TtsOutputType != NEXUS_TransportTimestampType_eNone);
        NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfig,&muxCreateSettings.memoryConfiguration);
    }
    pContext->streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = pContext->stcChannelTranscode;
    muxConfig.nonRealTime = pContext->bNonRealTime;
    muxConfig.nonRealTimeRate = g_nrtRate * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
    muxConfig.servicePeriod = g_msp; /* MSP */
    muxConfig.latencyTolerance = g_muxLatencyTolerance; /* mux service latency tolerance */
    muxConfig.interleaveMode = pContext->interleaveMode;
    muxConfig.useInitialPts = pContext->ptsSeed.bEnable;
    muxConfig.initialPts = pContext->ptsSeed.uiValue;

    if(!pContext->bNoVideo) {
        muxConfig.video[0].pid = BTST_MUX_VIDEO_PID;
        muxConfig.video[0].encoder = pContext->videoEncoder;
        muxConfig.video[0].playpump = pContext->playpumpTranscodeVideo;
    }

    if(pContext->inputSettings.bAudioInput)
    {
        /* audio playpump here is for ts muxer */
       if (NULL == pContext->playpumpTranscodeMCPB) {
         NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
         playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
         playpumpConfig.numDescriptors = g_muxDescs; /* set number of descriptors */
         playpumpConfig.streamMuxCompatible = true;
         pContext->playpumpTranscodeAudio = NEXUS_Playpump_Open(BTST_STREAM_MUX_AUDIO_PLAYPUMP_IDX+pContext->contextId*4, &playpumpConfig);
         BDBG_MSG(("Transcoder%d opened TS mux audio PES playpump%d [%p].", pContext->contextId, BTST_STREAM_MUX_AUDIO_PLAYPUMP_IDX+pContext->contextId*4, (void*)pContext->playpumpTranscodeAudio));
         NEXUS_Playpump_GetSettings(pContext->playpumpTranscodeAudio, &playpumpSettings);
         playpumpSettings.timestamp.forceRestamping = bTtsOutput;
         NEXUS_Playpump_SetSettings(pContext->playpumpTranscodeAudio, &playpumpSettings);
       } else {
          pContext->playpumpTranscodeAudio = pContext->playpumpTranscodeMCPB;
       }
        assert(pContext->playpumpTranscodeAudio);

        muxConfig.audio[0].pid = BTST_MUX_AUDIO_PID;
        muxConfig.audio[0].muxOutput = pContext->audioMuxOutput;
        muxConfig.audio[0].playpump = pContext->playpumpTranscodeAudio;
        muxConfig.audio[0].pesPacking = g_audioPesPacking;
    }

    muxConfig.pcr.pid = BTST_MUX_PCR_PID;
    muxConfig.pcr.playpump = pContext->playpumpTranscodePcr;
    muxConfig.pcr.interval = g_pcrInterval;
    muxConfig.muxDelay     = pContext->encodeDelay;
    muxConfig.supportTts   = (g_TtsOutputType != NEXUS_TransportTimestampType_eNone);
    muxConfig.insertPtsOnlyOnFirstKeyFrameOfSegment = g_bOnePtsPerSegment;

    /******************************************
     * Set up xcoder record output
     */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    /******************************************
     *  NRT workaround for the XPT band hold and data ready events sharing the same threshold: avoid band hold from occuring,
     * otherwise, video stutter would happen!
     * 1) The nexus_record timer fires at 250 ms interval to service record data as bottom line;
     * 2) whenever nexus record timer fires, it'll consume up to Nx(3x(47x4096)) cdb data;
     * 3) so if file i/o can keep up, band hold threshold = 2x(3x(47x4096)) = 1.1MB can sustain record bit rate up to
     *       2 * 3 * 188K * 8 / 250ms = 36 Mbps without reaching band hold;
     * 4) larger band hold threshold can sustain higher record bit rate throughput.
     * NOTE: after SW7425-1663, recpump data ready threshold is decoupled with RAVE upper (band-hold) threshold, so
     * we do not need to mess with data ready threshold any more!
     */
    BDBG_MSG(("To open recpump with dataReadyThreshold = %d indexReadyThreshold=%d",
        recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.index.dataReadyThreshold));
    BDBG_MSG(("        recpump with data fifo size     = "BDBG_UINT64_FMT" index fifo size    ="BDBG_UINT64_FMT,
        BDBG_UINT64_ARG((uint64_t)recpumpOpenSettings.data.bufferSize), BDBG_UINT64_ARG((uint64_t)recpumpOpenSettings.index.bufferSize)));
    pContext->recpump = NEXUS_Recpump_Open(pContext->contextId, &recpumpOpenSettings);
    assert(pContext->recpump);
    BDBG_MSG(("Transcoder%d opened TS mux recpump%d [%p].", pContext->contextId, pContext->contextId, (void*)pContext->recpump));

    pContext->record = NEXUS_Record_Create();
    assert(pContext->record);

    /*******************************
     * create system data PAT/PMT
     */
    xcode_create_systemdata(pContext);

    /*******************************
     *  TS user data pass through setup
     */
    if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
        NEXUS_Message_GetDefaultSettings(&messageSettings);
        /* SCTE 270 spec max TS VBI user data bitrate=270Kbps, 256KB buffer can hold 7.5 seconds;
           worthy user data for video synchronization; TODO: may be reduced if unnecessary */
        messageSettings.bufferSize = 512*1024;
        messageSettings.maxContiguousMessageSize = 0; /* to support TS capture and in-place operation */
        messageSettings.overflow.callback = message_overflow_callback; /* report overflow error */
        messageSettings.overflow.context  = pContext;

        /* open source user data PID channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther; /* capture the TS packets with the user data PES */
        playbackPidSettings.pidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;

        BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pContext->inputSettings.numUserDataPids);
        for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
            if(pContext->bUserDataStreamValid[i]) {
                messageSettings.overflow.param = pContext->inputSettings.remapUserDataPid[i];
                BDBG_MSG(("context%d opened message buffer for user data PID %d remapped %d", pContext->contextId,
                    pContext->inputSettings.userDataPid[i], pContext->inputSettings.remapUserDataPid[i]));
                muxConfig.userdata[i].message = NEXUS_Message_Open(&messageSettings);
                BDBG_ASSERT(muxConfig.userdata[i].message);
                pContext->userDataMessage[i] = muxConfig.userdata[i].message;

                if(pContext->bRemapUserDataPid) {
                    playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
                    playbackPidSettings.pidSettings.pidSettings.remap.pid     = pContext->inputSettings.remapUserDataPid[i];/* optional PID remap */
                }
                pContext->pidChannelUserData[i] = NEXUS_Playback_OpenPidChannel(pContext->playback,
                    pContext->inputSettings.userDataPid[i], &playbackPidSettings);
                BDBG_ASSERT(pContext->pidChannelUserData[i]);

                /* must start message before stream mux starts */
                NEXUS_Message_GetDefaultStartSettings(muxConfig.userdata[i].message, &messageStartSettings);
                messageStartSettings.format = NEXUS_MessageFormat_eTs;
                messageStartSettings.pidChannel = pContext->pidChannelUserData[i];
                NEXUS_Message_Start(muxConfig.userdata[i].message, &messageStartSettings);

                /* open transcode mux output user data PidChannels out of system data channel */
                pContext->pidChannelTranscodeUserData[i] = NEXUS_Playpump_OpenPidChannel(pContext->playpumpTranscodePcr,
                    pContext->inputSettings.remapUserDataPid[i], NULL);
                BDBG_ASSERT(pContext->pidChannelTranscodeUserData[i]);
            }
        }
    }

    /* store the mux config */
    pContext->muxConfig = muxConfig;

    /* open PidChannels */
    pContext->pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(pContext->playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    assert(pContext->pidChannelTranscodePcr);
    pContext->pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(pContext->playpumpTranscodePcr, BTST_MUX_PMT_PID, NULL);
    assert(pContext->pidChannelTranscodePmt);
    pContext->pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(pContext->playpumpTranscodePcr, BTST_MUX_PAT_PID, NULL);
    assert(pContext->pidChannelTranscodePat);

    if(pContext->bRemux) {/* remux output setup */
        NEXUS_RemuxSettings remuxSettings;
        NEXUS_RemuxParserBandwidth remuxParserBandwidth;
        NEXUS_ParserBandSettings parserBandSettings;

        /* Configure remux output  */
        NEXUS_Remux_GetDefaultSettings(&remuxSettings);
        remuxSettings.outputClock = NEXUS_RemuxClock_e27Mhz_VCXO_A;/* this worked; TODO: what about other options? */
        remuxSettings.enablePcrJitterAdjust = true;
        remuxSettings.insertNullPackets = true; /* has to enable to get NULL packets inserted in output */
        pContext->remux = NEXUS_Remux_Open(pContext->contextId, &remuxSettings);

        /* route remux output via a parser band to loopback for record */
        pContext->parserBandRemux = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        NEXUS_ParserBand_GetSettings(pContext->parserBandRemux, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
        parserBandSettings.sourceTypeSettings.remux = pContext->remux;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.acceptNullPackets = true; /* has to enable to get NULL packets inserted in output */
        parserBandSettings.allPass = true; /* has to enable to get NULL packets inserted in output */
        parserBandSettings.maxDataRate = 30000000; /* max rate */
        NEXUS_ParserBand_SetSettings(pContext->parserBandRemux, &parserBandSettings);
        NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
        remuxParserBandwidth.maxDataRate = 30000000;
        NEXUS_Remux_SetParserBandwidth(pContext->remux, &remuxParserBandwidth);
    }

    NEXUS_Record_GetSettings(pContext->record, &recordSettings);
    recordSettings.recpump = pContext->recpump;
    /* NOTE: enable band hold to allow recpump to stall TS mux to avoid ave data corruption in case file i/o is slow
     * The encoders should make sure no output CDB/ITB buffer corruption (instead do frame drop) when output overflow! */
    recordSettings.recpumpSettings.bandHold = pContext->bNonRealTime? NEXUS_RecpumpFlowControl_eEnable : NEXUS_RecpumpFlowControl_eDisable;
    recordSettings.recpumpSettings.timestampType = g_TtsOutputType;
    recordSettings.recpumpSettings.localTimestamp = bTtsOutput/* && pContext->bNonRealTime*/;
    recordSettings.recpumpSettings.adjustTimestampUsingPcrs = bTtsOutput;
    recordSettings.recpumpSettings.pcrPidChannel = pContext->pidChannelTranscodePcr;
    recordSettings.recpumpSettings.dropBtpPackets= true;
    recordSettings.recpumpSettings.data.overflow.callback = recpumpOverflowCallback;
    recordSettings.recpumpSettings.data.overflow.context  = pContext;
    recordSettings.pollingTimer = 50;
    if(pContext->bNonRealTime) {/* write all data for NRT */
        recordSettings.writeAllTimeout = 100;
    }
    NEXUS_Record_SetSettings(pContext->record, &recordSettings);


    /* set record index file name and open the record file handle; disable indexer for REMUX output due to allpass parser pids! */
    if(!pContext->bNoVideo && ((pContext->encodeSettings.encoderVideoCodec==NEXUS_VideoCodec_eMpeg2) ||
       (pContext->encodeSettings.encoderVideoCodec==NEXUS_VideoCodec_eH264)) && !pContext->bRemux)
    {
        xcode_index_filename(pContext->indexfname, pContext->encodeSettings.fname);
    }
    else BDBG_WRN(("no index record"));

}

#if NEXUS_HAS_HDMI_INPUT
static uint8_t SampleEDID[] =
{
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x74, 0x22, 0x05, 0x01, 0x11, 0x20,
    0x00, 0x14, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
    0x17, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
    0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
    0x58, 0x2C, 0x25, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
    0x43, 0x4D, 0x37, 0x34, 0x32, 0x32, 0x2F, 0x37, 0x34, 0x32, 0x35, 0x0A, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x17, 0x3D, 0x0F, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x89,

    0x02, 0x03, 0x3C, 0x71, 0x7F, 0x03, 0x0C, 0x00, 0x40, 0x00, 0xB8, 0x2D, 0x2F, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE3, 0x05, 0x1F, 0x01, 0x49, 0x90, 0x05, 0x20, 0x04, 0x03, 0x02, 0x07,
    0x06, 0x01, 0x29, 0x09, 0x07, 0x01, 0x11, 0x07, 0x00, 0x15, 0x07, 0x00, 0x01, 0x1D, 0x00, 0x72,
    0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A,
    0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x18,
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x0B, 0x88, 0x21, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D
};

static void
avInfoFrameChangedCallback(void *context, int param)
{
    NEXUS_HdmiInputStatus status;
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;

    BSTD_UNUSED(param);
    NEXUS_HdmiInput_GetStatus(hdmiInput, &status);
    BDBG_WRN(("HDMI Source AV InfoFrame Change callback: video format %ux%u@%.3f%c",
        status.avWidth,
        status.avHeight,
        (double)status.vertFreq/100,
        status.interlaced? 'i' : 'p'));
}
#endif

static void play_endOfStreamCallback(void *context, int param)
{
    TranscodeContext  *pContext = (TranscodeContext  *)context;
    BSTD_UNUSED(param);

    BDBG_WRN(("Context%d endOfStream\n", pContext->contextId));

    if(!g_bNonRealTimeWrap)
    {
        /* terminate the NRT context */
        B_Event_Set(pContext->eofEvent);
    }
    return;
}

static void nrt_endOfStreamHandler(void *context)
{
    int i;
    TranscodeContext  *pContext = (TranscodeContext  *)context;

    while(B_Event_Wait(pContext->eofEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    BDBG_WRN(("Context%d file xcode ending...\n", pContext->contextId));

    /* stop the NRT context at EOS event */
    B_Mutex_Lock(pContext->mutexStarted);
    if(pContext->bStarted) {
        if(g_bLoopbackPlayer && g_loopbackStarted && (g_loopbackXcodeId == pContext->contextId)) {
            fprintf(stderr, "File EOS to stop xcoder%d loopback...\n", g_loopbackXcodeId);
            xcode_loopback_shutdown(pContext);
            g_loopbackStarted = false;
        }
        shutdown_transcode(pContext);
        BDBG_WRN(("Transcode context%d completes.", pContext->contextId));
    }
    B_Mutex_Unlock(pContext->mutexStarted);
    if(g_bAutoQuit) { /* when all contexts are stopped, quit automatically */
        for(i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
            if(xcodeContext[i].bStarted) return;
        }
        if(g_eotEvent) {
            g_keyReturn = 'q';
            B_Event_Set(g_eotEvent);
        }
    }
    return;
}

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
static void BTST_VideoEncoder_WatchdogHandler (void *context, int param)
{
   NEXUS_VideoEncoderStopSettings videoEncoderStopSettings;
   TranscodeContext *pContext = (TranscodeContext*) context;
   BSTD_UNUSED(param);
   if (!pContext->bShuttingDown)
   {
      BDBG_WRN(("Video encoder %d watchdog callback fired. To restart it...", pContext->contextId));
      B_Mutex_Lock(pContext->mutexStarted);
      if(pContext->bNonRealTime) {/* NRT mode needs to stop&start whole context */
        /* bringdown loopback path */
        if(g_bLoopbackPlayer && pContext->contextId == g_loopbackXcodeId && g_loopbackStarted) {
            g_loopbackStarted = false;
            xcode_loopback_shutdown(pContext);
        }
        /* stop transcoder */
        pContext->uiStopMode = NEXUS_VideoEncoderStopMode_eAbort;
        stop_transcode(pContext);
        /* restart file xcoder */
        start_transcode(pContext);
        if(g_bLoopbackPlayer && (pContext->contextId == g_loopbackXcodeId) && !g_loopbackStarted &&
           (BTST_LOOPBACK_DISPLAY_IDX != get_display_index(pContext->contextId))) {
            xcode_loopback_setup(pContext);
            g_loopbackStarted = true;
        }
      } else {/* RT mode only need to stop/start video encoder */
        NEXUS_VideoEncoder_GetDefaultStopSettings(&videoEncoderStopSettings);
        videoEncoderStopSettings.mode = NEXUS_VideoEncoderStopMode_eAbort;
        NEXUS_VideoEncoder_Stop(pContext->videoEncoder, &videoEncoderStopSettings);
        NEXUS_VideoEncoder_Start(pContext->videoEncoder, &pContext->vidEncoderStartSettings);
      }
      B_Mutex_Unlock(pContext->mutexStarted);
   }
}
#endif

#include "bdbg_fifo.h"

#define LOGGER_MAX_COUNT 2
typedef struct LoggerContext
{
   bool enable;
   unsigned frequency;
   B_MutexHandle mutex;
   B_ThreadHandle thread;
   B_SchedulerHandle scheduler;
   B_SchedulerTimerId timer;

   uint8_t *auiEntry;
   size_t uiMaxEntrySize;

   struct
   {
      NEXUS_DebugFifoInfo debugFifoInfo;
      void *buffer;
      BDBG_FifoReader_Handle fifoReader;
      FILE *hFile;
   } stFifoList[LOGGER_MAX_COUNT];
} LoggerContext;

static LoggerContext g_loggerContext;

static void schedule_logger(void *context);
static void loggerDumpTimer(void *);
static bool add_logger_source(NEXUS_DebugFifoInfo *pstDebugFifoInfo, char *moduleName);
static void start_logger(void);
static void stop_logger(void);

static void schedule_logger(void *context)
{
   LoggerContext *pLogger = (LoggerContext *) context;

   pLogger->timer = B_Scheduler_StartTimer(
      pLogger->scheduler,
      pLogger->mutex,
      pLogger->frequency,
      loggerDumpTimer,
      &g_loggerContext);

   if(NULL == pLogger->timer) BDBG_ERR(("logger timer error"));
}

static void loggerDumpTimer(void *context)
{
   LoggerContext *pLogger = (LoggerContext *) context;
   unsigned i;

   for ( i = 0; i < LOGGER_MAX_COUNT; i++ )
   {
      if ( NULL != pLogger->stFifoList[i].fifoReader )
      {
         BERR_Code rc;

         while (1)
         {
            rc = BDBG_FifoReader_Read(
               pLogger->stFifoList[i].fifoReader,
               pLogger->auiEntry,
               pLogger->stFifoList[i].debugFifoInfo.elementSize
               );

            if ( BERR_SUCCESS == rc )
            {
               fwrite( pLogger->auiEntry, 1, pLogger->stFifoList[i].debugFifoInfo.elementSize, pLogger->stFifoList[i].hFile );
            }
            else if ( BERR_FIFO_OVERFLOW == rc )
            {
               BDBG_FifoReader_Resync( pLogger->stFifoList[i].fifoReader );
               BDBG_WRN(("<< VCE Encoder Format Log Message overflowed! Resyncing...(increase poll frequency!)>>"));
            }
            else if ( BERR_FIFO_NO_DATA == rc )
            {
               break;
            }
         }
      }
   }

   schedule_logger(pLogger);
}

static bool add_logger_source(NEXUS_DebugFifoInfo *pstDebugFifoInfo, char *moduleName)
{
   LoggerContext *pLogger = &g_loggerContext;

   if ( true == pLogger->enable )
   {
      signed iEmptySlot = -1;

      /* See if the BDBG_Fifo is already registered */
      {
         signed i;
         for ( i = 0; i < LOGGER_MAX_COUNT; i++ )
         {
            if ( 0 != pLogger->stFifoList[i].fifoReader )
            {
               if ( (pLogger->stFifoList[i].debugFifoInfo.buffer == pstDebugFifoInfo->buffer)
                    && (pLogger->stFifoList[i].debugFifoInfo.elementSize == pstDebugFifoInfo->elementSize)
                    && (pLogger->stFifoList[i].debugFifoInfo.offset == pstDebugFifoInfo->offset) )
               {
                  /* We already have this BDBG_Fifo, so exit */
                  return true;
               }
            }
            else if ( -1 == iEmptySlot )
            {
               iEmptySlot = i;
            }
         }
      }


      if ( -1 == iEmptySlot )
      {
         BDBG_ERR(("No space to add a BDBG_Fifo"));
         return false;
      }

      /* Add the BDBG_Fifo the list */
      B_Mutex_Lock(pLogger->mutex);
      {
         BERR_Code rc;
         char filename[256];
         pLogger->stFifoList[iEmptySlot].debugFifoInfo = *pstDebugFifoInfo;
         NEXUS_MemoryBlock_Lock(pLogger->stFifoList[iEmptySlot].debugFifoInfo.buffer, &pLogger->stFifoList[iEmptySlot].buffer);
         rc = BDBG_FifoReader_Create(&pLogger->stFifoList[iEmptySlot].fifoReader, (void*) ((uint8_t*) pLogger->stFifoList[iEmptySlot].buffer + pLogger->stFifoList[iEmptySlot].debugFifoInfo.offset) );
         BDBG_ASSERT(!rc);
         /* See if we have enough temp memory */
         if (pLogger->uiMaxEntrySize < pLogger->stFifoList[iEmptySlot].debugFifoInfo.elementSize)
         {
            /* We need to allocate a buffer large enough to handle a single entry */
            /* Free any previously allocted entry */
            if (NULL!=pLogger->auiEntry)
            {
               BKNI_Free(pLogger->auiEntry);
            }

            pLogger->uiMaxEntrySize = pLogger->stFifoList[iEmptySlot].debugFifoInfo.elementSize;
            pLogger->auiEntry = BKNI_Malloc(pLogger->uiMaxEntrySize);
            BDBG_ASSERT(pLogger->auiEntry);
         }

         /* Create the file */
         BKNI_Snprintf(filename, 256, "BDBG_Fifo[%d]_%s.bin", iEmptySlot, moduleName);
         pLogger->stFifoList[iEmptySlot].hFile = fopen(filename, "wb");
         BDBG_ASSERT(pLogger->stFifoList[iEmptySlot].hFile);
      }
      B_Mutex_Unlock(pLogger->mutex);
      return true;
   }
   return false;
}

static void start_logger(void)
{
   if ( true == g_loggerContext.enable )
   {
      B_ThreadSettings settingsThread;
      g_loggerContext.mutex = B_Mutex_Create(NULL);
      g_loggerContext.scheduler = B_Scheduler_Create(NULL);

      /* create thread to run scheduler */
      B_Thread_GetDefaultSettings(&settingsThread);
      g_loggerContext.thread = B_Thread_Create("logger_Scheduler",
         (B_ThreadFunc)B_Scheduler_Run,
         g_loggerContext.scheduler,
         &settingsThread);
      if (NULL == g_loggerContext.thread)
      {
         BDBG_ERR(("failed to create logger thread"));
         return;
      }
      schedule_logger(&g_loggerContext);
   }
}

static void stop_logger(void)
{
   if ( true == g_loggerContext.enable )
   {
      unsigned i;

      if (NULL != g_loggerContext.scheduler)
      {
         if (NULL != g_loggerContext.timer)
         {
            B_Scheduler_CancelTimer(g_loggerContext.scheduler, g_loggerContext.timer);
            g_loggerContext.timer = NULL;
         }

         B_Scheduler_Stop(g_loggerContext.scheduler);
         B_Scheduler_Destroy(g_loggerContext.scheduler);
         g_loggerContext.scheduler = NULL;
      }

      if (NULL != g_loggerContext.thread)
      {
         B_Thread_Destroy(g_loggerContext.thread);
         g_loggerContext.thread = NULL;
      }
      if (NULL != g_loggerContext.mutex)
      {
         B_Mutex_Destroy(g_loggerContext.mutex);
         g_loggerContext.mutex = NULL;
      }

      for ( i = 0; i < LOGGER_MAX_COUNT; i++ )
      {
         if (NULL != g_loggerContext.stFifoList[i].hFile)
         {
            fclose(g_loggerContext.stFifoList[i].hFile);
            g_loggerContext.stFifoList[i].hFile = NULL;
         }

         if (NULL != g_loggerContext.stFifoList[i].fifoReader)
         {
            BDBG_FifoReader_Destroy(g_loggerContext.stFifoList[i].fifoReader);
            g_loggerContext.stFifoList[i].fifoReader = NULL;
         }

#if 0
         if (NULL != g_loggerContext.stFifoList[i].buffer)
         {
            NEXUS_MemoryBlock_Unlock(g_loggerContext.stFifoList[i].debugFifoInfo.buffer);
            g_loggerContext.stFifoList[i].buffer = NULL;
         }
#endif
      }

      if ( NULL != g_loggerContext.auiEntry )
      {
         BKNI_Free( g_loggerContext.auiEntry );
         g_loggerContext.auiEntry = NULL;
      }
   }
}

static int open_transcode(
    TranscodeContext  *pContext )
{
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_ParserBandSettings parserBandSettings;
#endif
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderSettings videoDecodeSettings;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderOpenSettings videoEncoderOpenSettings;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;

    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioCodec audioCodec;

    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;
#endif

#if BTST_ENABLE_AV_SYNC
    NEXUS_SyncChannelSettings syncChannelSettings;
#if BTST_ENABLE_NRT_STC_AV_WINDOW
    NEXUS_StcChannelPairSettings stcAudioVideoPair;
#endif
#endif

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    NEXUS_DisplayStgSettings stgSettings;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_Error rc;
#endif
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowSettings windowSettings;
    bool bInterlacedXcode;
    unsigned decoderId = 0;
    EncodeSettings  *pEncodeSettings = &pContext->encodeSettings;
    InputSettings   *pInputSettings = &pContext->inputSettings;

    if(pContext->videoEncoder || pContext->audioMuxOutput) {
        BDBG_WRN(("Context%d already opened!", pContext->contextId));
        return 0;
    }

#if NEXUS_HAS_HDMI_INPUT
    if(pInputSettings->resource == BTST_RESOURCE_HDMI) {
        NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0+pContext->contextId, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
        NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0+pContext->contextId, &timebaseSettings);
        BDBG_MSG(("Transcoder%d set timebase %d.", pContext->contextId, NEXUS_Timebase_e0+pContext->contextId));

        NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
        hdmiInputSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;
        /* use NEXUS_HdmiInput_OpenWithEdid ()
            if EDID PROM (U1304 and U1305) is NOT installed;
            reference boards usually have the PROMs installed.
            this example assumes Port1 EDID has been removed
        */

        /* all HDMI Tx/Rx combo chips have EDID RAM */
        hdmiInputSettings.useInternalEdid = true ;
        pContext->hdmiInput = NEXUS_HdmiInput_OpenWithEdid(0, &hdmiInputSettings,
            &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
        if(!pContext->hdmiInput) {
            fprintf(stderr, "Can't get hdmi input\n");
            return -1;
        }
        NEXUS_HdmiInput_GetSettings(pContext->hdmiInput, &hdmiInputSettings);
        hdmiInputSettings.aviInfoFrameChanged.callback = avInfoFrameChangedCallback;
        hdmiInputSettings.aviInfoFrameChanged.context  = pContext->hdmiInput;
        NEXUS_HdmiInput_SetSettings(pContext->hdmiInput, &hdmiInputSettings);
    }
    else /* TODO: add sync channel support for hdmi source */
#endif
    if(!pContext->bNoVideo && pInputSettings->bAudioInput) {
        BDBG_ASSERT(!g_bSimulXcode); /* TODO: add simul audio mode */
#if BTST_ENABLE_AV_SYNC
        /* create a sync channel */
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        pContext->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif
    }

    /* bring up decoder and connect to local display */
    if(pInputSettings->resource != BTST_RESOURCE_HDMI) {
        if(!pContext->bNoVideo) {
            NEXUS_VideoDecoderCapabilities cap;
            NEXUS_VideoDecoderOpenSettings openSettings;

            if(!g_bSimulXcode || !g_activeXcodeCount) {
                unsigned decId = decoderId;
                NEXUS_GetVideoDecoderCapabilities(&cap);
                NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
                openSettings.avc51Enabled = false;
                openSettings.svc3dSupported = false;
                openSettings.excessDirModeEnabled = false;
                openSettings.enhancementPidChannelSupported = false;
                if(g_bSimulXcode) g_simulDecoderMaster = pContext->contextId;
                do {
                    decId = g_bDecoderZeroUp? decoderId : (cap.numVideoDecoders-decoderId-1);
                    if(cap.memory[decId].used && cap.videoDecoder[decId].feeder.index != (unsigned)(-1)) {
                        pContext->videoDecoder = NEXUS_VideoDecoder_Open(decId,
                            &openSettings); /* take default capabilities */
                    }
                    decoderId++;
                } while(!pContext->videoDecoder && decoderId < cap.numVideoDecoders);
                BDBG_MSG(("Transcoder%d opened source decoder %d.", pContext->contextId, decId));
                NEXUS_VideoDecoder_GetSettings(pContext->videoDecoder, &videoDecodeSettings);
                videoDecodeSettings.streamChanged.callback = vidSrcStreamChangedCallback;
                videoDecodeSettings.streamChanged.context  = pContext->videoDecoder;
                videoDecodeSettings.streamChanged.param  = pContext->contextId;
                videoDecodeSettings.supportedCodecs[pInputSettings->eVideoCodec] = true; /* it's for regular HD decode heap allocation; it covers mpeg2/h264/mpeg4 HD size */
                if(pContext->bEncodeCCUserData) {/* to log source user data */
                    char fname[256];
                    int idx = BTST_P_DecoderId_eSource0 + pContext->contextId;

                    videoDecodeSettings.userDataEnabled = true;
                    videoDecodeSettings.appUserDataReady.callback = userdataCallback;
                    g_decoderContext[idx].videoDecoder = pContext->videoDecoder;
                    g_decoderContext[idx].codec = pInputSettings->eVideoCodec;
                    sprintf(fname, "userdata_%s_608.csv", g_userDataDecoderName[idx]);
                    g_decoderContext[idx].output608.fLog        = fopen(fname, "wb");
                    g_decoderContext[idx].output608.bInit       = true;
                    g_decoderContext[idx].output608.bFilterNull = false;
                    sprintf(fname, "userdata_%s_708.csv", g_userDataDecoderName[idx]);
                    g_decoderContext[idx].output708.fLog = fopen(fname, "wb");
                    g_decoderContext[idx].output708.bInit       = true;
                    g_decoderContext[idx].output708.bFilterNull = false;
                    videoDecodeSettings.appUserDataReady.context  = &g_decoderContext[idx];
                    videoDecodeSettings.appUserDataReady.param    = idx;
                }
                videoDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture;
                /* set max WxH for decoder 0 to demo xcode from 4k source */
                if(0 == decId) {
                    videoDecodeSettings.maxWidth = g_vdecMaxWidth;
                    videoDecodeSettings.maxHeight= g_vdecMaxHeight;
                }
                NEXUS_VideoDecoder_SetSettings(pContext->videoDecoder, &videoDecodeSettings);
            } else {
                pContext->videoDecoder = xcodeContext[g_simulDecoderMaster].videoDecoder;
            }
        }

        /****************************************
         * set up xcoder source
         */
#if NEXUS_HAS_FRONTEND
        if(pInputSettings->resource == BTST_RESOURCE_QAM)
        {
            NEXUS_FrontendCapabilities capabilities;
            BDBG_ASSERT(pContext->contextId < NEXUS_MAX_FRONTENDS);
            pContext->frontend = g_platformConfig.frontend[pContext->contextId];
            if (pContext->frontend) {
                NEXUS_Frontend_GetCapabilities(pContext->frontend, &capabilities);
                /* Does this frontend support qam? */
                if ( !capabilities.qam )
                {
                    fprintf(stderr, "This platform doesn't support QAM frontend!\n");
                    return -1;
                }
            }

            NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
            qamSettings.frequency = pInputSettings->freq* 1000000;
            qamSettings.mode = pInputSettings->qamMode;
            switch (pInputSettings->qamMode) {
            case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
            case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
            case NEXUS_FrontendQamMode_e1024: qamSettings.symbolRate = 0; /* TODO */ break;
            default: fprintf(stderr, "Invalid QAM mode!\n"); return -1;
            }
            qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
            qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
            qamSettings.lockCallback.callback = lock_callback;
            qamSettings.lockCallback.context = pContext->frontend;

            NEXUS_Frontend_GetUserParameters(pContext->frontend, &userParams);

            /* Map a parser band to the demod's input band. */
            pContext->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
            NEXUS_ParserBand_GetSettings(pContext->parserBand, &parserBandSettings);
            if (userParams.isMtsif) {
                parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
                parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(pContext->frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
            }
            else {
                parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
                parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
            }
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
            NEXUS_ParserBand_SetSettings(pContext->parserBand, &parserBandSettings);
            if(!pContext->bNoVideo) {
                pContext->videoPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pInputSettings->iVideoPid, NULL);
                BDBG_MSG(("Transcoder%d opened PID channel for parser band %p.", pContext->contextId, (void*)pContext->parserBand));
            }
            if(pInputSettings->iPcrPid != pInputSettings->iVideoPid) {
                pContext->pcrPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pInputSettings->iPcrPid, NULL);
                BDBG_MSG(("Transcoder%d opened PCR PID channel for parser band %p.", pContext->contextId, (void*)pContext->parserBand));
            }

            /* each live transcoder should only use 2 STCs on the same timebase:
                   1) audio/video decode STC; (RT)
                   2) encode/mux STC;  (RT)
               NOTE: to avoid the 3rd one since NRT mode doesn't really need it,
                     encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
             */
            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;
            /* Tuner input has live PCR locked timebase */
            stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
            stcSettings.modeSettings.pcr.pidChannel = (pInputSettings->iPcrPid != pInputSettings->iVideoPid)?
                pContext->pcrPidChannel : /* different PCR PID */
                pContext->videoPidChannel; /* PCR happens to be on video pid */
            stcSettings.autoConfigTimebase = true;/* decoder STC auto config timebase so no need to manually config timebase */
            pContext->stcVideoChannel = NEXUS_StcChannel_Open(BTST_XCODE_VIDEO_STC_IDX(pContext->contextId), &stcSettings);
            BDBG_MSG(("Transcoder%d opened source vSTC [%p].", pContext->contextId, (void*)pContext->stcVideoChannel));

            pContext->stcAudioChannel = pContext->stcVideoChannel;
            BDBG_MSG(("Transcoder%d has source aSTC [%p].", pContext->contextId, (void*)pContext->stcAudioChannel));

            /* encoders/mux require different STC broadcast mode from decoder for RT mode; NRT mode can share decoder STC */
            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;/* should be the same timebase for end-to-end locking */
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;/* for encoder&mux, only timebase matters */
            stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
            stcSettings.autoConfigTimebase = false;/* encoder STC must NOT auto config timebase; NOTE this must be false at open */
            pContext->stcChannelTranscode = NEXUS_StcChannel_Open(BTST_XCODE_OTHER_STC_IDX(pContext->contextId), &stcSettings);
            BDBG_MSG(("Transcoder%d opened encoder STC [%p].", pContext->contextId, (void*)pContext->stcChannelTranscode));

            NEXUS_Frontend_TuneQam(pContext->frontend, &qamSettings);
        }
#else
        if(pInputSettings->resource == BTST_RESOURCE_QAM)
        {
            BDBG_ERR(("QAM Inout not supported"));
            BKNI_Fail();
        }
#endif
        else if(g_simulDecoderMaster == pContext->contextId || !g_bSimulXcode)
        {
            char srcIndexFileName[256] = {'\0',};
            FILE *indexFile;
            /* 4 playpumps per transcoder context: 1x src + 3x mux playpumps; */
            pContext->playpump = NEXUS_Playpump_Open(BTST_SOURCE_PLAYPUMP_IDX+pContext->contextId*4, NULL);
            assert(pContext->playpump);
            BDBG_MSG(("Transcoder%d opened source playpump %d.", pContext->contextId, BTST_SOURCE_PLAYPUMP_IDX+pContext->contextId*4));
            pContext->playback = NEXUS_Playback_Create();
            assert(pContext->playback);

            if(pInputSettings->eStreamType == NEXUS_TransportType_eTs) {
                xcode_index_filename(srcIndexFileName, pInputSettings->fname);
                indexFile = fopen(srcIndexFileName, "r");
                if (!indexFile) {
                    fprintf(stderr, "Source file %s supports limited trick mode without index.\n", pInputSettings->fname);
                    srcIndexFileName[0] = '\0';
                } else {
                    fclose(indexFile);
                }
            } else if(pInputSettings->eStreamType != NEXUS_TransportType_eDssEs && pInputSettings->eStreamType != NEXUS_TransportType_eDssPes) {
                BKNI_Snprintf(srcIndexFileName, 256, "%s", pInputSettings->fname);
            }

            pContext->file = NEXUS_FilePlay_OpenPosix(pInputSettings->fname, srcIndexFileName[0]? srcIndexFileName : NULL);
            if (!pContext->file) {
                fprintf(stderr, "can't open file:%s\n", pInputSettings->fname);
                exit(1);
            }

            /* file source could run in RT or NRT mode.
                   each transcoder should only use 2 STCs on the same timebase:
                   1) NRT video decode STC; (RT/NRT)
                   2) NRT audio decode STC; (NRT)
                   3) encode/mux STC;  (RT)
               NOTE: encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
             */
            NEXUS_StcChannel_GetDefaultSettings(BTST_XCODE_VIDEO_STC_IDX(pContext->contextId), &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcSettings.modeSettings.Auto.transportType = pInputSettings->eStreamType;
            stcSettings.autoConfigTimebase = true;/* decoder STC auto config timebase */
            pContext->stcVideoChannel = NEXUS_StcChannel_Open(BTST_XCODE_VIDEO_STC_IDX(pContext->contextId), &stcSettings);
            BDBG_MSG(("Transcoder%d opened source vSTC [%p].", pContext->contextId, (void*)pContext->stcVideoChannel));

            /* NRT mode uses separate STCs for audio and video decoders; */
            stcSettings.autoConfigTimebase = false;/* encoder STC must NOT auto config timebase; NOTE must set false at open */
            pContext->stcAudioChannel = (pContext->bNonRealTime && pInputSettings->bAudioInput)?
                NEXUS_StcChannel_Open(BTST_XCODE_OTHER_STC_IDX(pContext->contextId), &stcSettings) :
                pContext->stcVideoChannel;
            BDBG_MSG(("Transcoder%d opened source aSTC [%p].", pContext->contextId, (void*)pContext->stcAudioChannel));

            /* encoders/mux require different STC broadcast mode from decoder for RT mode; NRT mode can share decoder STC */
            if(pContext->bNonRealTime && pInputSettings->bAudioInput) {
                pContext->stcChannelTranscode = pContext->stcAudioChannel;
            } else {
                NEXUS_StcChannel_GetDefaultSettings(BTST_XCODE_OTHER_STC_IDX(pContext->contextId), &stcSettings);
                stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;/* should be the same timebase for end-to-end locking */
                stcSettings.mode = NEXUS_StcChannelMode_eAuto;/* for encoder&mux, only timebase matters */
                stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
                stcSettings.autoConfigTimebase = false;/* encoder STC must NOT auto config timebase */
                pContext->stcChannelTranscode = NEXUS_StcChannel_Open(BTST_XCODE_OTHER_STC_IDX(pContext->contextId), &stcSettings);
            }
            BDBG_MSG(("Transcoder%d opened encoder STC [%p].", pContext->contextId, (void*)pContext->stcChannelTranscode));

            NEXUS_Playback_GetSettings(pContext->playback, &playbackSettings);
            playbackSettings.playpump = pContext->playpump;
            /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
            playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)pInputSettings->eStreamType;
            playbackSettings.stcChannel = (pContext->bNoVideo)?
                pContext->stcAudioChannel : pContext->stcVideoChannel; /* playback shares the same STC channel as a/v decoders */

            /* NRT mode file transcode doesn not need loop */
            playbackSettings.endOfStreamAction = (!g_bNonRealTimeWrap)? NEXUS_PlaybackLoopMode_ePause : NEXUS_PlaybackLoopMode_eLoop; /* when play hits the end, wait for record */
            playbackSettings.endOfStreamCallback.callback = play_endOfStreamCallback;
            playbackSettings.endOfStreamCallback.context  = pContext;
            if(pInputSettings->eStreamType == NEXUS_TransportType_eTs) {/* 192-byte TTS */
                playbackSettings.playpumpSettings.timestamp.type = g_TtsInputType;
            }
            NEXUS_Playback_SetSettings(pContext->playback, &playbackSettings);

            /* Open the video pid channel */
            if(!pContext->bNoVideo) {
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pInputSettings->eVideoCodec; /* must be told codec for correct handling */
                playbackPidSettings.pidTypeSettings.video.index = true;
                playbackPidSettings.pidTypeSettings.video.decoder = pContext->videoDecoder;
                pContext->videoPidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, pInputSettings->iVideoPid, &playbackPidSettings);
            }
        } else {
            pContext->stcChannelTranscode = xcodeContext[g_simulDecoderMaster].stcChannelTranscode;
            pContext->stcVideoChannel = xcodeContext[g_simulDecoderMaster].stcVideoChannel;
            pContext->playpump = xcodeContext[g_simulDecoderMaster].playpump;
            pContext->playback = xcodeContext[g_simulDecoderMaster].playback;
            pContext->file = xcodeContext[g_simulDecoderMaster].file;
        }

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = (NEXUS_VideoCodec)pInputSettings->eVideoCodec;
        videoProgram.pidChannel = pContext->videoPidChannel;
        videoProgram.stcChannel = pContext->stcVideoChannel;
        videoProgram.nonRealTime = pContext->bNonRealTime;
        pContext->vidProgram = videoProgram;
    }
    else {/* HDMI input */
        /* HDMI source runs in RT mode.
               each transcoder should only use 1 STCs on the same timebase:
               1) encode/mux STC;  (RT)
           NOTE: encoder/mux STC is only required in RT mode; and for sloppiness, open the other stc to simplify close.
         */
        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.autoConfigTimebase = false; /* hdmi input timebase will be configured by hdmi input module! */
        pContext->stcVideoChannel = NEXUS_StcChannel_Open(BTST_XCODE_VIDEO_STC_IDX(pContext->contextId), &stcSettings);
        BDBG_MSG(("Transcoder%d opened source vSTC [%p].", pContext->contextId, (void*)pContext->stcVideoChannel));

        pContext->stcAudioChannel = pContext->stcVideoChannel;
        BDBG_MSG(("Transcoder%d has source aSTC [%p].", pContext->contextId, (void*)pContext->stcAudioChannel));

        /* encoders/mux require different STC broadcast mode from decoder for RT mode; NRT mode can share decoder STC */
        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0+pContext->contextId;/* should be the same timebase for end-to-end locking */
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;/* for encoder&mux, only timebase matters */
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
        stcSettings.autoConfigTimebase = false;/* encoder STC must NOT auto config timebase */
        pContext->stcChannelTranscode = NEXUS_StcChannel_Open(BTST_XCODE_OTHER_STC_IDX(pContext->contextId), &stcSettings);
        BDBG_MSG(("Transcoder%d opened encoder STC [%p].", pContext->contextId, (void*)pContext->stcChannelTranscode));
    }

    if(pContext->bNoVideo) goto SetUpAudioXcoder;

    /****************************************
     * Bring up video display and outputs
     */
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS /* can't use SD display for debug with DSP xcode, as it's in use */
    if(pContext->contextId == 0 && g_bEnableDebugSdDisplay) {/* only simul display the 1st context source for now */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format =
            (pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e25 ||
             pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e50) ?
            NEXUS_VideoFormat_ePal : NEXUS_VideoFormat_eNtsc;
        pContext->display = NEXUS_Display_Open(BTST_SOURCE_DISPLAY_IDX, &displaySettings);
        BDBG_MSG(("Transcoder%d opened display%d [%p]", pContext->contextId, BTST_SOURCE_DISPLAY_IDX, (void*)pContext->display));
#if NEXUS_NUM_COMPOSITE_OUTPUTS /* CVBS display from transcoder 0's source */
        if(g_platformConfig.outputs.composite[0] && (0==pContext->contextId)){
            NEXUS_Display_AddOutput(pContext->display, NEXUS_CompositeOutput_GetConnector(g_platformConfig.outputs.composite[0]));
        }
#endif
        pContext->window = NEXUS_VideoWindow_Open(pContext->display, 0);
    }
#else
    g_bEnableDebugSdDisplay = false;
#endif

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
    * which might cause encoder display GISB error since encoder display would
    * trigger RDC to program mailbox registers in ViCE2;
    */
    NEXUS_VideoEncoder_GetDefaultOpenSettings(&videoEncoderOpenSettings);
    videoEncoderOpenSettings.type = g_eVideoEncoderType;
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderOpenSettings.watchdogCallback.context = pContext;
    videoEncoderOpenSettings.watchdogCallback.callback= BTST_VideoEncoder_WatchdogHandler;
    if ( videoEncoderOpenSettings.memoryConfig.interlaced != false )
    {
       videoEncoderOpenSettings.memoryConfig.interlaced = !pEncodeSettings->videoEncoderMemConfig.progressiveOnly;
    }
    if(pEncodeSettings->videoEncoderMemConfig.maxWidth && pEncodeSettings->videoEncoderMemConfig.maxHeight) {
        videoEncoderOpenSettings.memoryConfig.maxWidth   = pEncodeSettings->videoEncoderMemConfig.maxWidth;
        videoEncoderOpenSettings.memoryConfig.maxHeight  = pEncodeSettings->videoEncoderMemConfig.maxHeight;
    } else {/* save default */
        pEncodeSettings->videoEncoderMemConfig.maxWidth = videoEncoderOpenSettings.memoryConfig.maxWidth;
        pEncodeSettings->videoEncoderMemConfig.maxHeight = videoEncoderOpenSettings.memoryConfig.maxHeight;
    }

    BDBG_MSG(("encoder bound %ux%u%c", videoEncoderOpenSettings.memoryConfig.maxWidth, videoEncoderOpenSettings.memoryConfig.maxHeight,
        videoEncoderOpenSettings.memoryConfig.interlaced?'i':'p'));
#endif
    pContext->videoEncoder = NEXUS_VideoEncoder_Open(pContext->contextId, &videoEncoderOpenSettings);
    assert(pContext->videoEncoder);
    BDBG_MSG(("Transcoder%d opened video encoder %d.", pContext->contextId, pContext->contextId));

    if ( true == g_loggerContext.enable )
    {
       NEXUS_VideoEncoderStatus status;

       NEXUS_VideoEncoder_GetStatus(pContext->videoEncoder, &status);
       add_logger_source(&status.debugLog, "vce");
    }

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.timebase = NEXUS_Timebase_e0 + pContext->contextId;/* timebase must match with decoder/encoder STC to track end-to-end */

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */
    displaySettings.display3DSettings.overrideOrientation = pEncodeSettings->b3D;
    displaySettings.display3DSettings.orientation         = pEncodeSettings->display3dType;

    if(!pEncodeSettings->bCustom)
    {
        NEXUS_VideoFormatInfo fmtInfo;
        displaySettings.format = (NEXUS_VideoFormat)pEncodeSettings->displayFormat;
        pContext->displayTranscode = NEXUS_Display_Open(get_display_index(pContext->contextId), &displaySettings);
        assert(pContext->displayTranscode);
        BDBG_MSG(("Transcoder%d opened encoder display%d [%p].", pContext->contextId, get_display_index(pContext->contextId), (void*)pContext->displayTranscode));
        pContext->windowTranscode = NEXUS_VideoWindow_Open(pContext->displayTranscode, 0);
        assert(pContext->windowTranscode);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &fmtInfo);
        bInterlacedXcode = fmtInfo.interlaced;
    }
    else
    {
        pContext->displayTranscode = NEXUS_Display_Open(get_display_index(pContext->contextId), &displaySettings);
        assert(pContext->displayTranscode);
        BDBG_MSG(("Transcoder%d opened encoder display%d [%p].", pContext->contextId, get_display_index(pContext->contextId), (void*)pContext->displayTranscode));
        pContext->windowTranscode = NEXUS_VideoWindow_Open(pContext->displayTranscode, 0);
        assert(pContext->windowTranscode);

        NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
        customFormatSettings.width = pEncodeSettings->customFormatSettings.width;
        customFormatSettings.height = pEncodeSettings->customFormatSettings.height;
        customFormatSettings.refreshRate = pEncodeSettings->customFormatSettings.refreshRate;
        customFormatSettings.interlaced = pEncodeSettings->customFormatSettings.interlaced;
        customFormatSettings.aspectRatio = pEncodeSettings->customFormatSettings.aspectRatio;
        customFormatSettings.sampleAspectRatio.x = pEncodeSettings->customFormatSettings.sampleAspectRatio.x;
        customFormatSettings.sampleAspectRatio.y = pEncodeSettings->customFormatSettings.sampleAspectRatio.y;
        customFormatSettings.dropFrameAllowed = true;
        rc = NEXUS_Display_SetCustomFormatSettings(pContext->displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
        assert(!rc);
        bInterlacedXcode = customFormatSettings.interlaced;
    }
#else /* DSP soft transcode relies on non-STG encoder display (RM based) timing generator, SD only */
        displaySettings.format =
           (pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e25 ||
            pEncodeSettings->encoderFrameRate == NEXUS_VideoFrameRate_e50)?
            NEXUS_VideoFormat_ePal : NEXUS_VideoFormat_eNtsc;/* SD display could work as normal */
        pContext->displayTranscode = NEXUS_Display_Open(get_display_index(pContext->contextId), &displaySettings);
        assert(pContext->displayTranscode);
        BDBG_MSG(("Transcoder%d opened encoder display%d [%p].", pContext->contextId, get_display_index(pContext->contextId), (void*)pContext->displayTranscode));
#if NEXUS_NUM_COMPOSITE_OUTPUTS /* CVBS display from transcoder 0's source */
        if(g_platformConfig.outputs.composite[0]){
            NEXUS_Display_AddOutput(pContext->displayTranscode, NEXUS_CompositeOutput_GetConnector(g_platformConfig.outputs.composite[0]));
        }
#endif
        if(!pInputSettings->bVideoWindowDisabled) {
            pContext->windowTranscode = NEXUS_VideoWindow_Open(pContext->displayTranscode, 0);
            assert(pContext->windowTranscode);
        }
        bInterlacedXcode = false;
#endif

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    /* NRT setup - AFAP mode */
    NEXUS_Display_GetStgSettings(pContext->displayTranscode, &stgSettings);
    stgSettings.enabled     = true;
    stgSettings.nonRealTime = pContext->bNonRealTime;
    NEXUS_Display_SetStgSettings(pContext->displayTranscode, &stgSettings);
#endif

    if(!pInputSettings->bVideoWindowDisabled) {
    NEXUS_VideoWindow_GetSettings(pContext->windowTranscode, &windowSettings);
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    if(pEncodeSettings->bCustom)
    {/* custom DSP transcode resolution */
        windowSettings.position.width = pEncodeSettings->customFormatSettings.width;
        windowSettings.position.height = pEncodeSettings->customFormatSettings.height;
    }
    /* cannot exceed maximum size */
    windowSettings.position.width = (windowSettings.position.width>BTST_INPUT_MAX_WIDTH)?
        BTST_INPUT_MAX_WIDTH : windowSettings.position.width;
    windowSettings.position.height = (windowSettings.position.height>BTST_INPUT_MAX_WIDTH)?
        BTST_INPUT_MAX_HEIGHT : windowSettings.position.height;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
#else
    /* set transcoder minimum display format before AddInput to avoid black frame transition during dynamic resolution change */
    windowSettings.scaleFactorRounding.enabled = false;
    windowSettings.scaleFactorRounding.horizontalTolerance = 0;
    windowSettings.scaleFactorRounding.verticalTolerance   = 0;
    if (pContext->bSeamlessFormatSwitch) {
        windowSettings.minimumDisplayFormat = pContext->b1080pCaptureFormat? NEXUS_VideoFormat_e1080p:NEXUS_VideoFormat_e720p;
    }
#endif
    windowSettings.forceCapture = false;
    windowSettings.contentMode = g_contentMode;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    windowSettings.preferSyncLock = true;
#endif
    NEXUS_VideoWindow_SetSettings(pContext->windowTranscode, &windowSettings);

    /* connect same decoder to the encoder display;
    * NOTE: simul display + transcode mode might have limitation in audio path;
    * here is for video transcode bringup/debug purpose;
    */
    if(pInputSettings->resource == BTST_RESOURCE_HDMI) {
#if NEXUS_HAS_HDMI_INPUT
        NEXUS_VideoInput videoInput = NEXUS_HdmiInput_GetVideoConnector(pContext->hdmiInput);
        NEXUS_VideoWindow_AddInput(pContext->windowTranscode, videoInput);
#endif
    }
    else
    {
        NEXUS_VideoWindow_AddInput(pContext->windowTranscode, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
    }

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    /* xcode window PQ default setting */
    window_dnr(pContext->windowTranscode, false);
#endif

    /* VDC madr default enabled */
    window_mad(pContext->windowTranscode, false, !g_bStartWithoutMad, pContext->bLowDelay?true:false);
    }

    /* default disable simul display since the nexus sync channel tried to add vsync delay to balance two BVN path delays
       which caused forced capture on transcode window that costs more memory allocation; */
    if(g_bEnableDebugSdDisplay)
    {
        if(pContext->contextId == 0) {/* only simul display the 1st context source for now */
            if(pInputSettings->resource == BTST_RESOURCE_HDMI) {
#if NEXUS_HAS_HDMI_INPUT
                NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_HdmiInput_GetVideoConnector(pContext->hdmiInput));
#endif
            }
            else {
                NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
            }
        }
    }

#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    /* TODO: for some reason, this must be done after addInput */
    if(!pInputSettings->bVideoWindowDisabled) {
        NEXUS_VideoWindow_GetSettings(pContext->windowTranscode, &windowSettings);
        windowSettings.preferSyncLock = true;
        NEXUS_VideoWindow_SetSettings(pContext->windowTranscode, &windowSettings);
    }
#endif

    /**************************************
     * encoder settings
     */
    NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = pContext->bVariableFrameRate;
    videoEncoderConfig.sparseFrameRate = g_bSparseFrameRate;
    videoEncoderConfig.frameRate = pEncodeSettings->encoderFrameRate;
    videoEncoderConfig.bitrateMax = pEncodeSettings->encoderBitrate;
    videoEncoderConfig.bitrateTarget = pEncodeSettings->vbr? pEncodeSettings->encoderTargetBitrate : 0;
    videoEncoderConfig.streamStructure.framesP = pEncodeSettings->encoderGopStructureFramesP;
    videoEncoderConfig.streamStructure.framesB = pEncodeSettings->encoderGopStructureFramesB;
    videoEncoderConfig.streamStructure.openGop = pEncodeSettings->openGop;
    videoEncoderConfig.streamStructure.duration = pEncodeSettings->gopDuration;
    videoEncoderConfig.streamStructure.adaptiveDuration = pEncodeSettings->adaptiveDuration;
    videoEncoderConfig.streamStructure.newGopOnSceneChange = pEncodeSettings->newGopOnSceneChange;
    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = pEncodeSettings->encoderVideoCodec;
    videoEncoderStartConfig.profile = pEncodeSettings->encoderProfile;
    videoEncoderStartConfig.entropyCoding = pEncodeSettings->entropyCoding;
    videoEncoderStartConfig.level = pEncodeSettings->encoderLevel;
    videoEncoderStartConfig.input = pContext->displayTranscode;
    videoEncoderStartConfig.stcChannel = pContext->stcChannelTranscode;
    videoEncoderStartConfig.interlaced = bInterlacedXcode;
    videoEncoderStartConfig.nonRealTime = pContext->bNonRealTime;
    videoEncoderStartConfig.lowDelayPipeline = pContext->bLowDelay;
    videoEncoderStartConfig.encodeUserData = pContext->bEncodeCCUserData;
    videoEncoderStartConfig.encodeBarUserData = !pContext->dropBardata;
    videoEncoderStartConfig.bypassVideoProcessing = pContext->bypassVideoFilter;
    videoEncoderStartConfig.memoryBandwidthSaving.singleRefP = pEncodeSettings->singleRefP;
    videoEncoderStartConfig.memoryBandwidthSaving.requiredPatchesOnly = pEncodeSettings->requiredPatchesOnly;
    videoEncoderStartConfig.hrdModeRateControl.disableFrameDrop = pEncodeSettings->disableFrameDrop;

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    videoEncoderStartConfig.window = pContext->windowTranscode;
#endif
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.segmentModeRateControl.enable = pEncodeSettings->segmentRC.enable;
    videoEncoderStartConfig.segmentModeRateControl.duration = pEncodeSettings->segmentRC.duration;
    videoEncoderStartConfig.segmentModeRateControl.upperTolerance = pEncodeSettings->segmentRC.upperTolerance;
    videoEncoderStartConfig.segmentModeRateControl.lowerTolerance = pEncodeSettings->segmentRC.lowerTolerance;
#endif

SetUpAudioXcoder:
    /*********************************
     * Set up audio xcoder
     */
    if(pInputSettings->bAudioInput)
    {
        NEXUS_AudioMixerSettings audioMixerSettings;
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;

        /* Open the audio decoder */
        /* Connect audio decoders to outputs */
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        audioDecoderOpenSettings.dspIndex    = pContext->contextId / BTST_NUM_AUDIO_XCODE_PER_DSP;
        if(g_bMultiChanAudio) {
            audioDecoderOpenSettings.multichannelFormat = pContext->inputSettings.eMultiChanFmt;
        }
        pContext->audioDecoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &audioDecoderOpenSettings);
        if(g_bSecondAudio) {
            pContext->secondaryDecoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &audioDecoderOpenSettings);
        }

        /* Open the audio and pcr pid channel */
#if NEXUS_HAS_FRONTEND
        if(pInputSettings->resource == BTST_RESOURCE_QAM)
        {
            pContext->audioPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pInputSettings->iAudioPid, NULL);
            if(g_bSecondAudio) {
                pContext->secondaryPidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pInputSettings->iSecondaryPid, NULL);
            }
        }
        else
#endif
        if(pInputSettings->resource == BTST_RESOURCE_FILE)
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = pContext->audioDecoder;
            if(g_bSecondAudio) {
                playbackPidSettings.pidTypeSettings.audio.secondary = pContext->secondaryDecoder;
            }
            pContext->audioPidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, pInputSettings->iAudioPid, &playbackPidSettings);
            if(g_bSecondAudio) {
                pContext->secondaryPidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, pInputSettings->iSecondaryPid, &playbackPidSettings);
            }
        }

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        if(pInputSettings->resource == BTST_RESOURCE_HDMI) {
#if NEXUS_HAS_HDMI_INPUT
            audioProgram.input = NEXUS_HdmiInput_GetAudioConnector(pContext->hdmiInput);
            audioProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;/* dsp mixer after hdmi cannot use low delay mode */
#endif
        } else {
            audioProgram.codec = pInputSettings->eAudioCodec;
            audioProgram.pidChannel = pContext->audioPidChannel;
            audioProgram.nonRealTime= pContext->bNonRealTime;
        }
        audioProgram.stcChannel = pContext->stcAudioChannel;
        pContext->audProgram = audioProgram;
        if(g_bSecondAudio) {
            audioProgram.pidChannel = pContext->secondaryPidChannel;
            audioProgram.secondaryDecoder = true;   /* Indicate this is a secondary channel for STC Channel/PCRlib functions */
            pContext->secondaryProgram = audioProgram;
        }

        /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
        if(!g_bNoDspMixer) {
            NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
            audioMixerSettings.mixUsingDsp = true;
            if(pEncodeSettings->bAudioEncode) {
                audioMixerSettings.outputSampleRate = 48000;/* fixed to allow gap filling for bogus audio PID */
            }
            audioMixerSettings.dspIndex    = pContext->contextId / BTST_NUM_AUDIO_XCODE_PER_DSP;
            pContext->audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
            assert(pContext->audioMixer);
        }

        /* Open audio mux output */
        pContext->audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
        assert(pContext->audioMuxOutput);

        if(pEncodeSettings->bAudioEncode || (pInputSettings->resource == BTST_RESOURCE_HDMI))
        {
            NEXUS_AudioEncoderCodecSettings codecSettings;
            /* Open audio encoder */
            NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
            encoderSettings.codec = pEncodeSettings->encoderAudioCodec;
            encoderSettings.codec = pEncodeSettings->encoderAudioCodec;
            audioCodec = pEncodeSettings->encoderAudioCodec;
            pContext->audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
            assert(pContext->audioEncoder);


            if(audioCodec == NEXUS_AudioCodec_eAacPlus && g_bForce48KbpsAACplus) {
                NEXUS_AudioEncoder_GetCodecSettings(pContext->audioEncoder, audioCodec, &codecSettings);
                codecSettings.codecSettings.aacPlus.bitRate = 48000;
                NEXUS_AudioEncoder_SetCodecSettings(pContext->audioEncoder, &codecSettings);
                BDBG_WRN(("Force AAC plus bitrate = 48Kbps"));
            }

            if(g_bNoDspMixer) {
                /* Connect decoder to encoder */
                NEXUS_AudioEncoder_AddInput(pContext->audioEncoder,
                    NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                BDBG_WRN(("No DSP mixer..."));
            }
            else {
                /* Connect decoder to mixer and set as master */
                NEXUS_AudioMixer_AddInput(pContext->audioMixer,
                    NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder,
                        (pContext->inputSettings.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                        NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo));
                if(g_bSecondAudio) {
                    NEXUS_AudioMixer_AddInput(pContext->audioMixer,
                        NEXUS_AudioDecoder_GetConnector(pContext->secondaryDecoder,
                            (pContext->inputSettings.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                            NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo));
                }
                audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder,
                        (pContext->inputSettings.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                        NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo);
                if(g_b32KHzAudio) audioMixerSettings.outputSampleRate = 32000;
                NEXUS_AudioMixer_SetSettings(pContext->audioMixer, &audioMixerSettings);

                #if USE_DDRE
                    /* Add the DDRE processor after mixing */
                    pContext->ddre = NEXUS_DolbyDigitalReencode_Open(NULL);
                    { NEXUS_DolbyVolume258Settings setting;
                    /* Open DV258 and add between encoder and DDRE */
                    NEXUS_DolbyVolume258_GetDefaultSettings(&setting);
                    setting.enabled = true;
                    pContext->dv258 = NEXUS_DolbyVolume258_Open(&setting);
                    }
                    NEXUS_DolbyVolume258_AddInput(pContext->dv258,
                        NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
                    NEXUS_DolbyDigitalReencode_AddInput(pContext->ddre,
                        NEXUS_DolbyVolume258_GetConnector(pContext->dv258));

                    /* Connect dv258 to encoder */
                    NEXUS_AudioEncoder_AddInput(pContext->audioEncoder,
                        NEXUS_DolbyDigitalReencode_GetConnector(pContext->ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
                #else
                    /* Connect mixer to encoder */
                    NEXUS_AudioEncoder_AddInput(pContext->audioEncoder,
                        NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
                #endif
            }

            /* Connect mux to encoder */
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput),
                NEXUS_AudioEncoder_GetConnector(pContext->audioEncoder));
        }
        else
        {
            if(g_bNoDspMixer) {
                /* Connect decoder to mux out */
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput),
                    NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            }
            else {
                NEXUS_AudioMixer_AddInput(pContext->audioMixer,
                    NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
                NEXUS_AudioMixer_SetSettings(pContext->audioMixer, &audioMixerSettings);
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput),
                    NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
            }
            audioCodec = audioProgram.codec;
        }

        /* Attach outputs for real-time transcoding */
        if(!pContext->bNonRealTime)
        {
            if(g_bNoDspMixer) {
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]),
                    NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, pEncodeSettings->bAudioEncode?
                        NEXUS_AudioDecoderConnectorType_eStereo :
                        NEXUS_AudioDecoderConnectorType_eCompressed));
            }
            else {
            #if USE_DDRE
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]),
                    NEXUS_DolbyDigitalReencode_GetConnector(pContext->ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
            #else
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]),
                    NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
            #endif
            }
        }
    }

    /************************************
     * Set up encoder AV sync.
     * encode setting and startSetting to be set after end-to-end delay is determined */
    /* get end-to-end delay (Dee) for audio and video encoders;
    * TODO: match AV delay! In other words,
    *   if (aDee > vDee) {
    *       vDee' = aDee' = aDee;
    *   }
    *   else {
    *       vDee' = aDee' = vDee;
    *   }
    */
    xcode_av_sync(pContext, &videoEncoderConfig, &videoEncoderStartConfig);

    if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI && !pContext->bNoVideo && pInputSettings->bAudioInput) {
#if BTST_ENABLE_AV_SYNC
        /* connect sync channel */
        NEXUS_SyncChannel_GetSettings(pContext->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder);
        if(pContext->encodeSettings.bAudioEncode)
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        else
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
#if BTST_ENABLE_NRT_STC_AV_WINDOW
        /* NRT mode pairs AV stc channels */
        if(pContext->bNonRealTime && !pContext->bNoVideo && !pInputSettings->bVideoWindowDisabled) {
            NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
            stcAudioVideoPair.connected = true;
            stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
            NEXUS_StcChannel_SetPairSettings(pContext->stcVideoChannel, pContext->stcAudioChannel, &stcAudioVideoPair);
        }
#endif
        syncChannelSettings.enablePrecisionLipsync = false;/* to support 60->30 frc transcode */

        NEXUS_SyncChannel_SetSettings(pContext->syncChannel, &syncChannelSettings);
#endif
    }

    /****************************
     * setup video encoder
     */
    if(!pContext->bNoVideo) {
        NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
    }

    /************************************************
     * Set up xcoder stream_mux and record
     */
    xcode_setup_mux_record(pContext);

    /* store encoder start settings */
    pContext->vidEncoderStartSettings = videoEncoderStartConfig;

    g_activeXcodeCount++;
    return 0;
}

static void close_transcode(
    TranscodeContext  *pContext )
{
    size_t i;

    if(!pContext->videoEncoder && !pContext->audioMuxOutput) {
        return;
    }

    g_activeXcodeCount--;

    /*******************************
     * release system data PAT/PMT
     */
    xcode_release_systemdata(pContext);

    /************************************
     * Bring down transcoder
     */
    if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
        for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
            if(pContext->bUserDataStreamValid[i]) {
                NEXUS_Message_Stop(pContext->userDataMessage[i]);
                NEXUS_Message_Close(pContext->userDataMessage[i]);
            }
        }
    }
    NEXUS_Record_Destroy(pContext->record);
    NEXUS_Recpump_Close(pContext->recpump);
    if(pContext->bRemux) {
        NEXUS_Remux_Close(pContext->remux);
        NEXUS_ParserBand_Close(pContext->parserBandRemux);
    }

    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE && (!g_activeXcodeCount || !g_bSimulXcode))
    {
        NEXUS_Playback_CloseAllPidChannels(pContext->playback);
        NEXUS_FilePlay_Close(pContext->file);
        NEXUS_Playback_Destroy(pContext->playback);
        NEXUS_Playpump_Close(pContext->playpump);
        pContext->playpump = NULL;
    }
    else if(pContext->inputSettings.resource == BTST_RESOURCE_QAM)
    {
        if(pContext->inputSettings.bAudioInput)
            NEXUS_PidChannel_Close(pContext->audioPidChannel);
        if(pContext->videoPidChannel)
            NEXUS_PidChannel_Close(pContext->videoPidChannel);
        if(pContext->pcrPidChannel) {
            NEXUS_PidChannel_Close(pContext->pcrPidChannel);
            pContext->pcrPidChannel = NULL;
        }
    }
    NEXUS_Playpump_CloseAllPidChannels(pContext->playpumpTranscodePcr);

    /******************************************
     * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
     */
    if(!pContext->bNoVideo) {
#if NEXUS_HAS_HDMI_INPUT
        if(pContext->inputSettings.resource == BTST_RESOURCE_HDMI) {
            NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(pContext->hdmiInput));
        }
        else
#endif
        {
#if BTST_ENABLE_AV_SYNC
            /* disconnect sync channel after decoders stop */
            if(pContext->inputSettings.bAudioInput) {
                NEXUS_SyncChannel_Destroy(pContext->syncChannel);
            }
#endif
            if(g_bEnableDebugSdDisplay && (pContext->contextId == 0))
            {
                NEXUS_VideoWindow_RemoveInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
            }
            if(!pContext->inputSettings.bVideoWindowDisabled)
                NEXUS_VideoWindow_RemoveInput(pContext->windowTranscode, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
            if(!g_activeXcodeCount || !g_bSimulXcode) {
                NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
                NEXUS_VideoDecoder_Close(pContext->videoDecoder);
                pContext->videoDecoder = NULL;
            }
        }
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
        if(pContext->contextId == 0 && g_bEnableDebugSdDisplay) {
            NEXUS_VideoWindow_Close(pContext->window);
            NEXUS_Display_Close(pContext->display);
        }
#endif
        NEXUS_VideoWindow_Close(pContext->windowTranscode);
        close_gfx(pContext);
        NEXUS_Display_Close(pContext->displayTranscode);

        if ( pContext->playpumpTranscodeVideo != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeVideo);
        NEXUS_VideoEncoder_Close(pContext->videoEncoder);
        pContext->videoEncoder = NULL;
    }

    BKNI_DestroyEvent(pContext->finishEvent);
    NEXUS_StreamMux_Destroy(pContext->streamMux);

    if ( pContext->playpumpTranscodePcr != pContext->playpumpTranscodeMCPB )    NEXUS_Playpump_Close(pContext->playpumpTranscodePcr);

    if(pContext->inputSettings.bAudioInput)
    {
        if(pContext->encodeSettings.bAudioEncode || (pContext->inputSettings.resource == BTST_RESOURCE_HDMI))
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
            NEXUS_AudioEncoder_RemoveAllInputs(pContext->audioEncoder);
            NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(pContext->audioEncoder));
            NEXUS_AudioEncoder_Close(pContext->audioEncoder);
            if(!g_bNoDspMixer) {
                NEXUS_AudioMixer_RemoveAllInputs(pContext->audioMixer);
            }
            if(!pContext->bNonRealTime) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]));
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]));
            }
#if USE_DDRE
            NEXUS_DolbyDigitalReencode_RemoveAllInputs(pContext->ddre);
            NEXUS_DolbyDigitalReencode_Close(pContext->ddre);
            NEXUS_DolbyVolume258_RemoveAllInputs(pContext->dv258);
            NEXUS_DolbyVolume258_Close(pContext->dv258);
#endif
            if(!g_bNoDspMixer) {
                NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
                NEXUS_AudioMixer_Close(pContext->audioMixer);
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            NEXUS_AudioDecoder_Close(pContext->audioDecoder);
            if(g_bSecondAudio) {
                NEXUS_AudioDecoder_Close(pContext->secondaryDecoder);
            }
        }
        else
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
            if(!g_bNoDspMixer) {
                NEXUS_AudioMixer_RemoveAllInputs(pContext->audioMixer);
            }
            if(!pContext->bNonRealTime) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]));
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[pContext->contextId]));
            }
            if(!g_bNoDspMixer) {
                NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
                NEXUS_AudioMixer_Close(pContext->audioMixer);
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            NEXUS_AudioDecoder_Close(pContext->audioDecoder);
        }

        if ( pContext->playpumpTranscodeAudio != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeAudio);
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
        NEXUS_AudioMuxOutput_Destroy(pContext->audioMuxOutput);
        pContext->audioMuxOutput = NULL;
    }

    if ( NULL != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeMCPB);

    /* NOTE: each transcoder context only needs two separate STCs;
       if NRT mode, the audio STC is the same as transcode STC; if RT mode, audio STC is the same as video STC. */
    if(!g_activeXcodeCount || !g_bSimulXcode) {
        NEXUS_StcChannel_Close(pContext->stcVideoChannel);
        NEXUS_StcChannel_Close(pContext->stcChannelTranscode);
    }

#if NEXUS_HAS_HDMI_INPUT
    if(pContext->inputSettings.resource == BTST_RESOURCE_HDMI) {
        NEXUS_HdmiInput_Close(pContext->hdmiInput);
    }
#endif
#if NEXUS_HAS_FRONTEND
    if(pContext->inputSettings.resource == BTST_RESOURCE_QAM) {
        NEXUS_ParserBand_Close(pContext->parserBand);
    }
#endif
}

/* stop without close */
static void stop_transcode(
    TranscodeContext  *pContext )
{
    NEXUS_VideoEncoderStopSettings videoEncoderStopSettings;

    if(!pContext->bStarted) {
        return;
    }

    /*******************************
     * stop system data scheduler
     */
    if (!pContext->bPrematureMuxStop)
    {
       xcode_stop_systemdata(pContext);
    }
    /**************************************************
     * NOTE: stop sequence should be in front->back order
     */
    if((pContext->inputSettings.resource == BTST_RESOURCE_FILE) && !pContext->bNoStopDecode)
    {
        if(pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode) {
            NEXUS_Playback_Stop(pContext->playback);
        }
    }
    if(pContext->inputSettings.bAudioInput)
    {
        if(!pContext->bNoStopDecode) {
            NEXUS_AudioDecoder_Stop(pContext->audioDecoder);
            if(g_bSecondAudio) {
                NEXUS_AudioDecoder_Stop(pContext->secondaryDecoder);
            }
            if(!g_bNoDspMixer) {
                NEXUS_AudioMixer_Stop(pContext->audioMixer);
            }
        }
        NEXUS_AudioMuxOutput_Stop(pContext->audioMuxOutput);
    }

    if(!pContext->bNoVideo) {
        if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI && !pContext->bNoStopDecode) {
            if(pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode)
            NEXUS_VideoDecoder_Stop(pContext->videoDecoder);
        }
        NEXUS_VideoEncoder_GetDefaultStopSettings(&videoEncoderStopSettings);
        videoEncoderStopSettings.mode = pContext->uiStopMode;
        /* eAbort stop mode will defer stop after mux stop to silence mux errors on pending descriptors */
        if(NEXUS_VideoEncoderStopMode_eAbort != pContext->uiStopMode) {
            NEXUS_VideoEncoder_Stop(pContext->videoEncoder, &videoEncoderStopSettings);
        }
    }

    if(NEXUS_VideoEncoderStopMode_eAbort != pContext->uiStopMode) {

       if (!pContext->bPrematureMuxStop)
       {
         NEXUS_StreamMux_Finish(pContext->streamMux);
         if(BKNI_WaitForEvent(pContext->finishEvent, 4000)!=BERR_SUCCESS) {
            fprintf(stderr, "TIMEOUT\n");
         }
       }
    }
    if(pContext->bRemux) {
        NEXUS_Remux_Stop(pContext->remux);
        NEXUS_Remux_RemoveAllPidChannels(pContext->remux);
        NEXUS_PidChannel_CloseAll(pContext->parserBandRemux);
    }

    NEXUS_Record_Stop(pContext->record);
    NEXUS_FileRecord_Close(pContext->fileTranscode);
    /*****************************************
     * Note: remove all record PID channels before stream
     * mux stop since streammux would close the A/V PID channels
     */
    NEXUS_Record_RemoveAllPidChannels(pContext->record);
    if (!pContext->bPrematureMuxStop)
    {
       NEXUS_StreamMux_Stop(pContext->streamMux);
    }
    /* stop video encoder after stream mux stop if in eAbort mode to silence the mux errors for pending descriptors. */
    if(!pContext->bNoVideo && NEXUS_VideoEncoderStopMode_eAbort == pContext->uiStopMode) {
        NEXUS_VideoEncoder_Stop(pContext->videoEncoder, &videoEncoderStopSettings);
    }

   pContext->uiStopMode = 0; /* reset stop mode whenever used; it needs to be set everytime to be used */
   pContext->bPrematureMuxStop = false; /* reset mux stop mode whenever used; it needs to be set everytime to be used  */

    /* Temporary workaround to flush pending descriptors from NEXUS_AudioMuxOutput prior to restarting it.
       Restarting will flush the pending descriptors. */
    #if 1
    if (pContext->inputSettings.bAudioInput)
    {
        const NEXUS_AudioMuxOutputFrame *pBuf,*pBuf2;
        size_t size,size2;
        NEXUS_Error errCode;

        do {
           errCode = NEXUS_AudioMuxOutput_GetBuffer(pContext->audioMuxOutput, &pBuf, &size, &pBuf2, &size2);
           if ( BERR_SUCCESS == errCode )
           {
               size += size2;
               if ( size > 0 )
               {
                   BDBG_WRN(("Flushing "BDBG_UINT64_FMT" outstanding audio descriptors", BDBG_UINT64_ARG((uint64_t)size)));
                   NEXUS_AudioMuxOutput_ReadComplete(pContext->audioMuxOutput, size);
               }
           }
        } while ( size > 0 );
    }
    #endif

    if ( g_bPrintStatus ) printStatus( pContext );

    /* stopped */
    pContext->bStarted = false;
}

/* start without open */
static void start_transcode(
    TranscodeContext  *pContext )
{
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_VideoDecoderExtendedSettings extSettings;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    if(pContext->bStarted) {
        return;
    }

    /*******************************
     * Add system data to stream_mux
     */
    xcode_start_systemdata(pContext);
    /**************************************************
     * NOTE: transcoder record/mux resume
     */
    /* start mux */
    NEXUS_StreamMux_Start(pContext->streamMux,&pContext->muxConfig, &muxOutput);
    pContext->pidChannelTranscodeVideo = muxOutput.video[0];

    /* add multiplex data to the same record */
    /* configure the video pid for indexing */
    if(!pContext->bNoVideo) {
        NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
        recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        /* don't index if remux output with allpass pids */
        recordPidSettings.recpumpSettings.pidTypeSettings.video.index = /* RAVE only support mpeg2&avc indexing */
                ((pContext->encodeSettings.encoderVideoCodec==NEXUS_VideoCodec_eMpeg2) ||
                 (pContext->encodeSettings.encoderVideoCodec==NEXUS_VideoCodec_eH264)) && !pContext->bRemux;
        recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = pContext->encodeSettings.encoderVideoCodec;
        if(pContext->bRemux) {/* mux -> remux ->parser -> record */
            NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodeVideo);
            pContext->pidRemuxVideo = NEXUS_PidChannel_Open(pContext->parserBandRemux, pContext->muxConfig.video[0].pid, NULL);
            NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxVideo, &recordPidSettings);
        } else {
            NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodeVideo, &recordPidSettings);
        }
    }

    /* for audio channel */
    if(pContext->inputSettings.bAudioInput)
    {
        pContext->pidChannelTranscodeAudio = muxOutput.audio[0];
        if(pContext->bRemux) {/* mux -> remux -> record */
            NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodeAudio);
            pContext->pidRemuxAudio = NEXUS_PidChannel_Open(pContext->parserBandRemux, pContext->muxConfig.audio[0].pid, NULL);
            NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxAudio, &recordPidSettings);
        } else {
            NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodeAudio, NULL);
        }
    }

    /* for system data channel */
    if(pContext->bRemux) {/* mux -> remux -> parser -> record */
        NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodePcr);
        NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodePat);
        NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodePmt);

        pContext->pidRemuxPcr = NEXUS_PidChannel_Open(pContext->parserBandRemux, pContext->muxConfig.pcr.pid, NULL);
        BDBG_ASSERT(pContext->pidRemuxPcr);
        pContext->pidRemuxPmt = NEXUS_PidChannel_Open(pContext->parserBandRemux, BTST_MUX_PMT_PID, NULL);
        BDBG_ASSERT(pContext->pidRemuxPmt);
        pContext->pidRemuxPat = NEXUS_PidChannel_Open(pContext->parserBandRemux, BTST_MUX_PAT_PID, NULL);
        BDBG_ASSERT(pContext->pidRemuxPat);
        /* it seems the null packets would be recorded without explicitly added here as parser band enabled allPass; */
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxPcr, NULL);
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxPat, NULL);
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxPmt, NULL);
        if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pContext->inputSettings.numUserDataPids);
            for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
                if(pContext->bUserDataStreamValid[i]) {
                    NEXUS_Remux_AddPidChannel(pContext->remux, pContext->pidChannelTranscodeUserData[i]);
                    pContext->pidRemuxUserData[i] = NEXUS_PidChannel_Open(pContext->parserBandRemux, pContext->inputSettings.remapUserDataPid[i], NULL);
                    NEXUS_Record_AddPidChannel(pContext->record, pContext->pidRemuxUserData[i], NULL);
                }
            }
        }
        NEXUS_Remux_Start(pContext->remux);
    } else {/* non-remux output */
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodePcr, NULL);
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodePat, NULL);
        NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodePmt, NULL);
        if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pContext->inputSettings.numUserDataPids);
            for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
                if(pContext->bUserDataStreamValid[i]) {
                    NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodeUserData[i], NULL);
                }
            }
        }
    }

    if (muxOutput.pcr)
    {
       NEXUS_Record_AddPidChannel(pContext->record, muxOutput.pcr, NULL);
    }

    if(g_bFifo && pContext->indexfname[0]) {
        NEXUS_FifoRecordSettings fifoRecordSettings;

        pContext->fifoFile = NEXUS_FifoRecord_Create(pContext->encodeSettings.fname, pContext->indexfname);

        NEXUS_FifoRecord_GetSettings(pContext->fifoFile, &fifoRecordSettings);
        fifoRecordSettings.interval = 60;
        if(NEXUS_FifoRecord_SetSettings(pContext->fifoFile, &fifoRecordSettings)) {
            BDBG_ERR(("Transcoder[%u] failed set fifo record", pContext->contextId)); BDBG_ASSERT(0);
        }

        pContext->fileTranscode = NEXUS_FifoRecord_GetFile(pContext->fifoFile);
        BDBG_MSG(("Opened record fifo file."));
    }
    else {
        pContext->fileTranscode = NEXUS_FileRecord_OpenPosix(pContext->encodeSettings.fname,
            pContext->indexfname[0]? pContext->indexfname : NULL);
    }
    if (!pContext->fileTranscode) {
        fprintf(stderr, "can't create file: %s, %s\n", pContext->encodeSettings.fname, pContext->indexfname);
        exit(1);
    }

    /* Start record of stream mux output */
    NEXUS_Record_Start(pContext->record, pContext->fileTranscode);

    /****************************
     * start decoders
     */
    if((pContext->inputSettings.resource != BTST_RESOURCE_HDMI) && !pContext->bNoStopDecode && !pContext->bNoVideo
        && (pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode)) {
        /* register the 3DTV status change callback */
        NEXUS_VideoDecoder_GetExtendedSettings(pContext->videoDecoder, &extSettings);
        extSettings.s3DTVStatusEnabled = true;
        extSettings.s3DTVStatusChanged.callback = decode3dCallback;
        extSettings.s3DTVStatusChanged.context = pContext->videoDecoder;
        extSettings.s3DTVStatusChanged.param = pContext->contextId;
        NEXUS_VideoDecoder_SetExtendedSettings(pContext->videoDecoder, &extSettings);

        /* Start decoder */
        NEXUS_VideoDecoder_Start(pContext->videoDecoder, &pContext->vidProgram);
    }

    if(pContext->inputSettings.bAudioInput)
    {
        /* Start audio mux output */
        errCode = NEXUS_AudioMuxOutput_Start(pContext->audioMuxOutput, &pContext->audMuxStartSettings);
        if ( NEXUS_SUCCESS != errCode ) {
            BDBG_ERR(("%s:%d - NEXUS_AudioMuxOutput_Start failed", __FILE__, __LINE__));
            return;
        }
        if(!pContext->bNoStopDecode) {
            /* Start audio mixer */
            if(!g_bNoDspMixer) {
                errCode = NEXUS_AudioMixer_Start(pContext->audioMixer);
                if ( NEXUS_SUCCESS != errCode ) {
                    BDBG_ERR(("%s:%d - NEXUS_AudioMixer_Start failed", __FILE__, __LINE__));
                    return;
                }
            }

            errCode = NEXUS_AudioDecoder_Start(pContext->audioDecoder, &pContext->audProgram);
            if ( NEXUS_SUCCESS != errCode ) {
                BDBG_ERR(("%s:%d - NEXUS_AudioDecoder_Start(PRIMARY) failed", __FILE__, __LINE__));
                return;
            }
            if(g_bSecondAudio) {
                BDBG_WRN(("Starting Secondary audio"));
                errCode = NEXUS_AudioDecoder_Start(pContext->secondaryDecoder, &pContext->secondaryProgram);
                if ( NEXUS_SUCCESS != errCode ) {
                    BDBG_ERR(("%s:%d - NEXUS_AudioDecoder_Start(SECONDARY) failed", __FILE__, __LINE__));
                    return;
                }
            }
        }
    }

    /* Start playback before mux set up PAT/PMT which may depend on PMT user data PSI extraction */
    if((pContext->inputSettings.resource == BTST_RESOURCE_FILE) && !pContext->bNoStopDecode
        && (pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode))
    {
        NEXUS_Playback_Start(pContext->playback, pContext->file, NULL);
    }

    /************************************************
     * Start video encoder
     */
    if(!pContext->bNoVideo) {
        pContext->vidEncoderStartSettings.adaptiveLowDelayMode = pContext->bAdaptiveLatencyStart;
        pContext->vidEncoderStartSettings.encodeUserData       = pContext->bEncodeCCUserData;
        errCode = NEXUS_VideoEncoder_Start(pContext->videoEncoder, &pContext->vidEncoderStartSettings);
        BERR_TRACE(errCode);
    }
    /* started */
    pContext->bStarted = true;

    if(errCode != NEXUS_SUCCESS) stop_transcode(pContext);
}

static void bringup_transcode(
    TranscodeContext  *pContext )
{
    /* open the transcode context */
    open_transcode(pContext);

    /*******************************
     * START transcoder
     */
    start_transcode(pContext);

    /* increment eof reference count */
    if((BTST_RESOURCE_FILE==pContext->inputSettings.resource) && (NULL==pContext->eofEvent)) {
        pContext->eofEvent = B_Event_Create(NULL);
        B_Event_Reset(pContext->eofEvent);
        pContext->nrtEofHandle = B_Thread_Create("File mode EOF handler", (B_ThreadFunc)nrt_endOfStreamHandler, pContext, NULL);
    }
}
static void shutdown_transcode(
    TranscodeContext  *pContext )
{
    /*******************************
     * START transcoder
     */
    stop_transcode(pContext);
    BDBG_MSG(("stopped transcoder %d", pContext->contextId));

    /* close the transcode context */
    close_transcode(pContext);
    BDBG_MSG(("closed transcoder %d", pContext->contextId));

    if(g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output608.fLog) {/* to log loopback xcoded user data */
        fclose(g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output608.fLog);
        g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output608.fLog = NULL;
    }
    if(g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output708.fLog) {/* to log loopback xcoded user data */
        fclose(g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output708.fLog);
        g_decoderContext[BTST_P_DecoderId_eSource0 + pContext->contextId].output708.fLog = NULL;
    }
}
#endif

static const namevalue_t g_inputTypeStrs[] = {
    {"file", BTST_RESOURCE_FILE},
    {"hdmi", BTST_RESOURCE_HDMI},
    {"qam", BTST_RESOURCE_QAM},
    {NULL, 0}
};

static void print_input_parameters(
    TranscodeContext *pContext )
{
    printf("\n****************************************************************\n");
    printf("\n Input Parameters\n");
    printf("\n SourceType   :  %s", lookup_name(g_inputTypeStrs, pContext->inputSettings.resource));
    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE)
        printf("\n filename     :  %s", pContext->inputSettings.fname);
#if NEXUS_HAS_FRONTEND
    if(pContext->inputSettings.resource == BTST_RESOURCE_QAM)
        printf("\n freq         :  %d MHz\n qam mode     :  %s", pContext->inputSettings.freq, lookup_name(g_qamModeStrs, pContext->inputSettings.qamMode));
#endif

    if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI)
    {
        printf("\n TransportType:  %s", lookup_name(g_transportTypeStrs, pContext->inputSettings.eStreamType));
        if(!pContext->bNoVideo) {
            printf("\n VideoCodec   :  %s", lookup_name(g_videoCodecStrs, pContext->inputSettings.eVideoCodec));
            printf("\n vPid         :  %#x\n", pContext->inputSettings.iVideoPid);
        }
        printf(" PcrPid       :  %#x\n", pContext->inputSettings.iPcrPid);
    }
    if(pContext->inputSettings.bAudioInput)
    {
        printf("\n Audio  Pid   :  %#x\n Audio codec  :  %s", pContext->inputSettings.iAudioPid, lookup_name(g_audioCodecStrs, pContext->inputSettings.eAudioCodec));
    }
    printf("\n****************************************************************\n");
}

static void print_output_parameters(
    TranscodeContext *pContext )
{
    printf("\n****************************************************************\n");
    printf("\n Encode Parameters\n");
    printf("\n filename    : %s", pContext->encodeSettings.fname);
    if(!pContext->bNoVideo) {
        if(pContext->encodeSettings.bCustom) {
            printf("\n Video Format: %ux%u%c%.3f", pContext->encodeSettings.customFormatSettings.width,
                pContext->encodeSettings.customFormatSettings.height,
                pContext->encodeSettings.customFormatSettings.interlaced?'i':'p',
                (float)pContext->encodeSettings.customFormatSettings.refreshRate/1000);
        } else {
            printf("\n Video Format: %s", lookup_name(g_videoFormatStrs, pContext->encodeSettings.displayFormat));
        }
        printf("\n Frame Rate  : %s", lookup_name(g_videoFrameRateStrs, pContext->encodeSettings.encoderFrameRate));
        printf("\n Bit Rate    : %d", pContext->encodeSettings.encoderBitrate);
        printf("\n P frames    : %d\n B frames    : %u", pContext->encodeSettings.encoderGopStructureFramesP, pContext->encodeSettings.encoderGopStructureFramesB);
        printf("\n Video Codec : %s\n Profile     : %s\n Level       : %s",
            lookup_name(g_videoCodecStrs, pContext->encodeSettings.encoderVideoCodec),
            lookup_name(g_videoCodecProfileStrs, pContext->encodeSettings.encoderProfile),
            lookup_name(g_videoCodecLevelStrs, pContext->encodeSettings.encoderLevel));
    }
    if(pContext->encodeSettings.bAudioEncode)
        printf("\n Audio Codec : %s", lookup_name(g_audioCodecStrs, pContext->encodeSettings.encoderAudioCodec));
    printf("\n****************************************************************\n");
    printf("\n%s transcode...%s \n", pContext->bNonRealTime? "Non-Realtime":"Real time",
        pContext->bLowDelay? "Low Delay pipeline Mode": "Normal delay pipeline Mode");
    printf("%s\n", pContext->bNoVideo? "No video transcode" : "");
}

static void config_xcoder_context (
    TranscodeContext *pContext )
{
    int choice;
    char input[80];
    pContext->inputSettings.bConfig = true;
    printf("\n Choose source: \n");
    print_value_list(g_inputTypeStrs);
    pContext->inputSettings.resource = getNameValue(input, g_inputTypeStrs);

    switch (pContext->inputSettings.resource)
    {
    case (BTST_RESOURCE_HDMI):
#if !NEXUS_HAS_HDMI_INPUT
        fprintf(stderr, "HDMI input is not supported!\n");
        return;
#endif
        break;
#if NEXUS_HAS_FRONTEND
    case (BTST_RESOURCE_QAM):
        {
            printf("\n Front End QAM freq (Mhz): ");
            pContext->inputSettings.freq = getValue(input);
            printf("\n Front End QAM Mode:\n");
            print_value_list(g_qamModeStrs);
            pContext->inputSettings.qamMode = getNameValue(input, g_qamModeStrs);
            printf("\n source stream type: (2) Ts             "); pContext->inputSettings.eStreamType = NEXUS_TransportType_eTs;
            if(!pContext->bNoVideo) {
                printf("\n source stream codec:\n");
                print_value_list(g_videoCodecStrs);
                pContext->inputSettings.eVideoCodec = getNameValue(input, g_videoCodecStrs);
                printf("\n Video pid:                ");
                pContext->inputSettings.iVideoPid = getValue(input);
            }
            printf("\n Pcr   pid:                ");
            pContext->inputSettings.iPcrPid = getValue(input);
            break;
        }
#else
    case (BTST_RESOURCE_QAM):
        printf("\n QAM tuning not supported ");
        break;
#endif
    case (BTST_RESOURCE_FILE):
    default:
        {
            printf("\n source stream file: ");
            strcpy(pContext->inputSettings.fname, getString(input));
            printf("\n source stream type:\n");
            print_value_list(g_transportTypeStrs);
            pContext->inputSettings.eStreamType = getNameValue(input, g_transportTypeStrs);
            if(!pContext->bNoVideo) {
                printf("\n source stream codec:\n");
                print_value_list(g_videoCodecStrs);
                pContext->inputSettings.eVideoCodec = getNameValue(input, g_videoCodecStrs);
                printf("\n Video pid:                 ");
                pContext->inputSettings.iVideoPid = getValue(input);
            }
            printf("\n Pcr   pid:                 ");
            pContext->inputSettings.iPcrPid = getValue(input);
        }
    }

    printf("\n Input Audio Parameters");
    printf("\n Enable Audio channel: (0) No (1) Yes             ");
    pContext->inputSettings.bAudioInput = getValue(input);
    if(pContext->inputSettings.bAudioInput)
    {
        if (pContext->inputSettings.resource == BTST_RESOURCE_HDMI)
        {
            printf("\n Is HDMI input audio uncompressed? (0)No; (1)Yes    ");
            pContext->inputSettings.bPcmAudio = getValue(input);
        }
        else
        {
            printf("\n Audio Pid:           ");
            pContext->inputSettings.iAudioPid = getValue(input);
        }

        /* PCM audio must be encoded */
        if(pContext->inputSettings.bPcmAudio)
        {
            pContext->encodeSettings.bAudioEncode = true;
        }
        else
        {
            printf("\n Audio Codec:\n");
            print_value_list(g_audioCodecStrs);
            pContext->inputSettings.eAudioCodec = getNameValue(input, g_audioCodecStrs);
            pContext->encodeSettings.encoderAudioCodec = pContext->inputSettings.eAudioCodec;
            if(g_bMultiChanAudio) {
                printf("\n Multi-channel audio format: \n");
                print_value_list(g_audioChannelFormatStrs);
                pContext->inputSettings.eMultiChanFmt = getNameValue(input, g_audioChannelFormatStrs);
            }
            if(g_bSecondAudio) {
                printf("\n Secondary Audio Pid:                ");
                pContext->inputSettings.iSecondaryPid = getValue(input);
            }
        }
    }

    print_input_parameters(pContext);

    printf("\n\n Encode Settings");
    printf("\n Output file name:      ");
    strcpy(pContext->encodeSettings.fname, getString(input));
    if(!pContext->bNoVideo) {
        printf("\n custom format: (0) No (1) Yes      ");
        pContext->encodeSettings.bCustom = getValue(input);
        if(!pContext->encodeSettings.bCustom)
        {
            printf("\n enocder displayFormat:\n");
            print_value_list(g_videoFormatStrs);
            pContext->encodeSettings.displayFormat = getNameValue(input, g_videoFormatStrs);
        }
        else
        {
            pContext->encodeSettings.displayFormat = NEXUS_VideoFormat_eCustom2;
            printf("\n Resolution width & height:      ");
            pContext->encodeSettings.customFormatSettings.width = getValue(input);
            pContext->encodeSettings.customFormatSettings.height = getValue(input);
            printf("\n Interlaced? (0) No (1) Yes   ");
            pContext->encodeSettings.customFormatSettings.interlaced = getValue(input);
            printf("\n Vertical refresh rate (in 1/1000th Hz):      ");
            pContext->encodeSettings.customFormatSettings.refreshRate = getValue(input);
            printf("\n Aspect Ratio:\n");
            print_value_list(g_displayAspectRatioStrs);
            pContext->encodeSettings.customFormatSettings.aspectRatio = getNameValue(input, g_displayAspectRatioStrs);
            if(NEXUS_DisplayAspectRatio_eSar == pContext->encodeSettings.customFormatSettings.aspectRatio)
            {
                printf("Please enter Sample Aspect Ratio X and Y: \n");
                pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x = getValue(input);
                pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y = getValue(input);
            }
        }

        printf("\n Frame rate:\n");
        print_value_list(g_videoFrameRateStrs);
        pContext->encodeSettings.encoderFrameRate = getNameValue(input, g_videoFrameRateStrs);
        if(pContext->encodeSettings.b3D) {
            printf("\n 3D orientation type:\n");
            print_value_list(g_videoOrientation);
            pContext->encodeSettings.display3dType = getNameValue(input, g_videoOrientation);
        }

        printf("\n Max Bitrate (bps):\t\t");
        pContext->encodeSettings.encoderBitrate = getValue(input);
        if(pContext->encodeSettings.vbr) {
            printf("\n VBR target Bitrate (bps):\t\t");
            pContext->encodeSettings.encoderTargetBitrate = getValue(input);
        }
        printf("\n # of P frames per GOP:\t\t");
        pContext->encodeSettings.encoderGopStructureFramesP = getValue(input);
        if(pContext->encodeSettings.encoderGopStructureFramesP && g_bBandwidthSaving) {
            printf("\n single reference for P picture:\t\t");
            pContext->encodeSettings.singleRefP = getValue(input);
            printf("\n disable optional patches from reference picture:\t\t");
            pContext->encodeSettings.requiredPatchesOnly = getValue(input);
        }
        printf("\n # of B frames between reference I or P frames:\t\t");
        pContext->encodeSettings.encoderGopStructureFramesB = getValue(input);
        printf("\n Encode Video Codec: (%d) MPEG2 (%d) H264 \t\t", NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264);
        pContext->encodeSettings.encoderVideoCodec = getNameValue(input, g_videoCodecStrs);
        if(pContext->encodeSettings.encoderVideoCodec != NEXUS_VideoCodec_eMpeg2 &&
           pContext->encodeSettings.encoderVideoCodec != NEXUS_VideoCodec_eH264 &&
           g_bFifo) {
            BDBG_WRN(("Video encoder codec %u cannot be index'd, so record file cannot be in fifo mode. Will record in linear storage.",
                pContext->encodeSettings.encoderVideoCodec));
        }
        printf("\n Profile:\n");
        print_value_list(g_videoCodecProfileStrs);
        pContext->encodeSettings.encoderProfile = getNameValue(input, g_videoCodecProfileStrs);
        printf("\n Level:\n");
        print_value_list(g_videoCodecLevelStrs);
        pContext->encodeSettings.encoderLevel = getNameValue(input, g_videoCodecLevelStrs);
    }

    if(pContext->inputSettings.bAudioInput)
    {
        if(pContext->inputSettings.bPcmAudio)
        {
            pContext->encodeSettings.bAudioEncode = true;
        }
        else
        {
            printf("\n Enable Audio transcode: (0) No (1) Yes  \t\t");
            pContext->encodeSettings.bAudioEncode = getValue(input);
        }

        if(pContext->encodeSettings.bAudioEncode)
        {
            printf("\n Encode Audio Codec: (%d)mp3 (%d)aac (%d)aacplus+ (%d)lpcm_1394 \t",
                NEXUS_AudioCodec_eMp3,
                NEXUS_AudioCodec_eAac,
                NEXUS_AudioCodec_eAacPlus,
                NEXUS_AudioCodec_eLpcm1394);
            pContext->encodeSettings.encoderAudioCodec = getNameValue(input, g_audioCodecStrs);
        }
        else {
            if(g_bSecondAudio) {printf("Multi audio streams cannot be passed through...\n"); BDBG_ASSERT(0);}
        }
    }
    if(pContext->inputSettings.resource == BTST_RESOURCE_FILE)
    {
        printf("\nNon-Realtime transcode? [1=y/0=n]\n");
        pContext->bNonRealTime = getValue(input);
    }

    /* Reset custom settings */
    pContext->bCustomizeDelay = false;
    pContext->bLowDelay = false;

    if((!pContext->bNonRealTime || g_bCustomDelay) && !pContext->bNoVideo)
    {
        printf("\n Customize video encoder delay setting which might affect quality? [1=y/0=n]\n");
        pContext->bCustomizeDelay = getValue(input);
        if(pContext->bCustomizeDelay)
        {
            printf("\n enable Inverse Telecine Field Pairing coding? [1=y/0=n]\n");
            pContext->encodeSettings.enableFieldPairing = getValue(input);

            printf("\n video encoder rate buffer delay (ms):\n");
            pContext->encodeSettings.rateBufferDelay = getValue(input);

            printf("\n video encode display minimum frame rate:\n");
            print_value_list(g_videoFrameRateStrs);
            pContext->encodeSettings.bounds.inputFrameRate.min = getNameValue(input, g_videoFrameRateStrs);

            printf("\n video encoder output minimum frame rate:\n");
            print_value_list(g_videoFrameRateStrs);
            pContext->encodeSettings.bounds.outputFrameRate.min = getNameValue(input, g_videoFrameRateStrs);

            printf("\n video encode maximum resolution width:\n");
            pContext->encodeSettings.bounds.inputDimension.max.width = getValue(input);
            printf("\n video encode maximum resolution height:\n");
            pContext->encodeSettings.bounds.inputDimension.max.height = getValue(input);

            printf("\n video encoder pipeline low delay? [1=Y/0=N]:\n");
            pContext->bLowDelay = getValue(input);
            if(pContext->bLowDelay && (g_eVideoEncoderType != NEXUS_VideoEncoderType_eSingle ||
                pContext->encodeSettings.encoderGopStructureFramesP != 0xFFFFFFFF ||
                pContext->encodeSettings.enableFieldPairing)) {
                BDBG_WRN(("Encoder HW pipeline low delay mode can only be enabled if '-type single' and framesP = 0xFFFFFFFF (or -1) and Field Pairing disabled"));
                BDBG_ASSERT(0);
            }
            printf("\n upper bound for dynamic video bitrate:\n");
            pContext->encodeSettings.bounds.bitrate.upper.bitrateMax = getValue(input);

            if(pContext->encodeSettings.vbr) {
                printf("\n VBR target Bitrate upper bound (bps):\t\t");
                pContext->encodeSettings.bounds.bitrate.upper.bitrateTarget = getValue(input);
            }
            printf("\n maximum number of B pictures between I or P reference pictures:\n");
            pContext->encodeSettings.bounds.streamStructure.max.framesB = getValue(input);
        }
    }

    /* TS user data config */
    if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
        pContext->bEncodeCCUserData = g_bEncodeCC;
        pContext->inputSettings.bTsUserDataInput = g_bTsUserData;
        if(pContext->inputSettings.bTsUserDataInput) {
            printf("\n PMT pid [0=auto detection]:                           ");
            pContext->inputSettings.pmtPid = getValue(input);
            printf("\n Do you want all TS user data passed thru? [1=y/0=n]   ");
            pContext->bRemapUserDataPid = (0==getValue(input));
            if(!pContext->bRemapUserDataPid) {
                pContext->inputSettings.numUserDataPids = BTST_TS_USER_DATA_ALL;
            } else {
                printf("\n How many user data PIDs are passed thru?   ");
                pContext->inputSettings.numUserDataPids = getValue(input);
                pContext->inputSettings.numUserDataPids = (pContext->inputSettings.numUserDataPids > NEXUS_MAX_MUX_PIDS) ?
                    NEXUS_MAX_MUX_PIDS : pContext->inputSettings.numUserDataPids;

                printf("\n Please enter the input user data PIDs list: ");
                for(choice=0; choice<(int)pContext->inputSettings.numUserDataPids; choice++) {
                    pContext->inputSettings.userDataPid[choice] = getValue(input);
                }
                printf("\n Remap the output TS user data PIDs? [1=y/0=n]  ");
                pContext->bRemapUserDataPid = getValue(input);
                for(choice=0; choice<(int)pContext->inputSettings.numUserDataPids; choice++) {
                    if(pContext->bRemapUserDataPid) {
                        pContext->inputSettings.remapUserDataPid[choice] = BTST_MUX_USER_DATA_PID+choice;
                    } else {/* no change */
                        pContext->inputSettings.remapUserDataPid[choice] = pContext->inputSettings.userDataPid[choice];
                    }
                }
            }
        }
    }

    print_output_parameters(pContext);
}

void print_usage(void) {
            printf("\ntranscode_ts usage:\n");
            printf("  Without options, it transcodes default stream file /data/videos/avatar_AVC_15M.ts into TS file: /data/BAT/encode.ts\n");
            printf("\nOptions:\n");
            printf("  -h        - to print the usage info\n");
            printf("  -cxt N    - to select xcoder N\n");
            printf("  -scr N    - to run in script mode and quit in N seconds\n");
            printf("  -cfg      - to set the test configuration\n");
            printf("  -fifo     - to use FIFO file record instead of unbounded file record for long term test\n");
            printf("  -NRTwrap|loop  - to loop source file for long term file transcode test\n");
            printf("  -autoquit - to quit the test automatically when all transcoders are stopped\n");
            printf("  -cc       - to enable Closed Caption user data transcode\n");
            printf("  -tsud     - to enable TS layer user data passthrough.\n");
            printf("  -nodspmix - to disable audio DSP fw mixer.\n");
            printf("  -multichan - to enable audio multi-channel mode setting.\n");
            printf("  -msaudio  - to enable audio multi-stream decode mode.\n");
            printf("  -nrtRate N - to set stream mux NRT rate as N times fast as RT mode.\n");
            printf("  -delayTolerance MSEC - to set TS mux SW service latency tolerance in miliseconds (bigger value = more mux delay.\n");
            printf("  -muxDescs N - to set TS mux transport descriptors number; default 128.\n");
            printf("  -msp MSEC - to set TS mux service period in miliseconds.\n");
            printf("  -rsp MSEC - to set record service period in miliseconds.\n");
            printf("  -tts_in binary|mod300  - Input TTS transport packets prepended with 4-byte timestamps in binary or mod300 mode.\n");
            printf("  -tts_out binary|mod300  - Output TTS transport packets prepended with 4-byte timestamps in binary or mod300 mode.\n");
            printf("  -madoff   - to start transcoder with MADR disabled.\n");
            printf("  -win {box|zoom} - to start transcoder with specified aspect ratio correction mode (default bypass).\n");
            printf("  -loopbackoff - to start transcoder with loopback player disabled.\n");
            printf("  -vdecZeroUp [-maxSize W H] - to start transcoder with video decoder 0 up, optionally decoder 0 with max WxH size.\n");
            printf("  -vbr      - to enable VBR video encode.\n");
            printf("  -type {single|multi|multiNRT}\n");
            printf("     Note: 1) single type supports single encode, low delay mode and up to 1080p30 realtime encode\n");
            printf("           2) multiNRT type supports up to 1080p30 with dual NRT encodes\n");
            printf("           3) multi type is the default, supports up to 2 encodes with combined throughput up to 1080p30\n");
            printf("  -3d       - enable 3D encode\n");
            printf("  -seamless 1080p|720p - seamless transcode format switch up to 1080p or 720p\n");
            printf("  -vfr      - enable variable frame rate transcode\n");
            printf("  -sfr      - enable sparse frame rate transcode\n");
            printf("  -sd       - enable CVBS debug display for source decoder.\n");
            printf("  -openGop  - enable open GOP encode.\n");
            printf("  -dynamicGop - enable new GOP on scene change.\n");
            printf("  -gopDuration N [-adaptiveDuration] - GOP duration in N ms; optionally with adaptive duration ramppup.\n");
            printf("  -progressive_only - Video encoder output format is progressive only.\n");
            printf("  -maxDimension WIDTH HEIGHT  - Video encoder maximum resolution.\n");
            printf("  -noVideo - Audio only transcode without video.\n");
            printf("  -noVideoWindow - Transcode without video window (could have gfx).\n");
            printf("  -bypassVideoFilter - Bypass transcoder video filtering.\n");
            printf("  -force48KbpsAacPlus - Force 48Kbps AAC plus encoding.\n");
            printf("  -32KHzAudio - output 32KHz sample rate audio encoding.\n");
            printf("  -simul    - 1toN simultaneous transcoding for ABR.\n");
            printf("  -remux    - enable remux for stream mux output.\n");
            printf("  -adaptiveLowdelay - enable adaptive low delay mode for video encoder.\n");
            printf("  -segment [duration MS] [bitRateTolerance UPPER,LOWER] - enable segment based rate control video encoding.\n");
            printf("       Note, UPPER and LOWER tolerance are in percentages.\n");
            printf("  -cabacOff - disable entropy coding.\n");
            printf("  -bwSaving [singleRefP] [requiredPatchesOnly] - enable P-frame coding bandwidth saving mode.\n");
            printf("  -in [FILE [-program N] [-rt]] - Probe the input FILE and take program N as source, optionally run in RT mode.\n");
            printf("  -video_size W,H - Encode output size WxH.\n");
            printf("  -video_bitrate N - Encode output bitrate N bps.\n");
            printf("  -disableFrameDrop - Disable HRD drop.\n");
            print_list_option("video_framerate", g_videoFrameRateStrs);
            print_list_option("video_refreshrate", g_videoFrameRateStrs);
            printf("  -dropBardata - Drop bar data encoding.\n");
            printf("  -mcpb - Use MCPB playback channel for muxing\n");
            printf("  -logger N - Enable BDBG_Fifo logger with a poll frequency of N ms\n");
            printf("  -printStatus - Print status after transcode is stopped\n");
            printf("  -pcrInterval N - Set the PCR interval (0=disable PCR insertion)\n");
            printf("  -interleaveMode N - Set the A/V interleave mode to N where N = [escr, pts]\n");
            printf("  -audioPesPacking - Enable audio PES packing\n");
            printf("  -onePtsPerSegment - Insert only one PTS per segment\n");
            printf("  -ptsSeed PTS - Start NRT transcode with specified video PTS (in 45Khz)\n");
            printf("  -itfpoff - Disable ITFP");
}

/* include media probe */
#include "media_probe.c"
#ifdef DIAGS
int transcode_main(int argc, char **argv)  {
#else
int main(int argc, char **argv)  {
#endif

#if NEXUS_HAS_STREAM_MUX

    NEXUS_PlatformSettings platformSettings;
    TranscodeContext *pContext = xcodeContext;
    int i, iteration = 1;

    BSTD_UNUSED(argv);
    BKNI_Memset(pContext, 0, sizeof(xcodeContext));

    /* Turn off buffering for stdin before taking input */
    setvbuf(stdin, NULL, _IONBF, 0);

    if (argc == 1){ /* default test config for auto test */
        print_usage();
        printf("\nYou're testing with the default configuration:\n");
        /* Input setting */
        g_bNonRealTimeWrap = true;/* looping source file */
        pContext->inputSettings.resource = BTST_RESOURCE_FILE;
        BKNI_Snprintf(pContext->inputSettings.fname, 256, "/data/videos/avatar_AVC_15M.ts");
        pContext->inputSettings.eStreamType=NEXUS_TransportType_eTs;
        pContext->inputSettings.eVideoCodec=NEXUS_VideoCodec_eH264;
        pContext->inputSettings.eAudioCodec=NEXUS_AudioCodec_eAc3;
        pContext->inputSettings.bPcmAudio=false;
        pContext->inputSettings.iVideoPid=0x101;
        pContext->inputSettings.iAudioPid=0x104;
        pContext->inputSettings.iPcrPid=0x101;
        pContext->inputSettings.freq=0;
#if NEXUS_HAS_FRONTEND
        pContext->inputSettings.qamMode=NEXUS_FrontendQamMode_e64;
#endif
        pContext->inputSettings.bAudioInput=true;/* enable audio */
        pContext->inputSettings.bConfig = false;

        print_input_parameters(pContext);

        /*Encode settings */
        BKNI_Snprintf(pContext->encodeSettings.fname, 256, "/data/BAT/encode.ts");
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
        pContext->encodeSettings.displayFormat=NEXUS_VideoFormat_e480p;
        #if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT /* 416x224p30@400Kbps */
        pContext->encodeSettings.encoderBitrate=400000;
        #else/* 480p30@1.5Mbps */
        pContext->encodeSettings.encoderBitrate=1500000;
        #endif
        pContext->encodeSettings.encoderProfile=NEXUS_VideoCodecProfile_eBaseline;
        pContext->encodeSettings.encoderLevel=NEXUS_VideoCodecLevel_e30;
#else
        pContext->encodeSettings.displayFormat=NEXUS_VideoFormat_e720p;
        /* restrict encoder memory config for 720p xcode by default */
        pContext->encodeSettings.videoEncoderMemConfig.progressiveOnly = true;
        pContext->encodeSettings.videoEncoderMemConfig.maxWidth = 1280;
        pContext->encodeSettings.videoEncoderMemConfig.maxHeight = 720;
        pContext->encodeSettings.encoderProfile=NEXUS_VideoCodecProfile_eBaseline;
        pContext->encodeSettings.encoderLevel=NEXUS_VideoCodecLevel_e31;
        pContext->encodeSettings.encoderBitrate=3000000;
#endif
        pContext->encodeSettings.encoderFrameRate=NEXUS_VideoFrameRate_e29_97;
        pContext->encodeSettings.encoderGopStructureFramesP=29;
        pContext->encodeSettings.encoderGopStructureFramesB=0;
        pContext->encodeSettings.encoderVideoCodec=NEXUS_VideoCodec_eH264;
        pContext->encodeSettings.bCustom=false;
        pContext->encodeSettings.bAudioEncode=true;/* transcode audio to AAC */
        pContext->encodeSettings.encoderAudioCodec=NEXUS_AudioCodec_eAac;

        print_output_parameters(pContext);
    }
    if ( argc >= 2) {
        for(i=1; i<argc; i++) {
            if(!strcmp("-h",argv[i])) {
                print_usage();
                return 0;
            }
            if(!strcmp("-in",argv[i]) && (i+1 < argc)) {
                #define BTST_P_DEFAULT(a, b) {if(a==0) a = b;}
                strcpy(pContext->inputSettings.fname, argv[++i]);
                pContext->inputSettings.probe = true; /* enabled media probe */
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
                pContext->bNonRealTime = true; /* enabled NRT mode by default */
#endif
                fprintf(stderr, "Input file %s\n", pContext->inputSettings.fname);
                if((i+2) < argc) { /* check optional -program N */
                    if(!strcmp("-program", argv[i+1])) {
                        i++;
                        pContext->inputSettings.program = strtoul(argv[++i], NULL, 0);
                        fprintf(stderr, "Selected program %u.\n", pContext->inputSettings.program);
                    }
                }
                if((i+1) < argc) { /* check optional -rt */
                    if(!strcmp("-rt", argv[i+1])) {
                        i++;
                        pContext->bNonRealTime = false;
                        fprintf(stderr, "RT mode.\n");
                    }
                }
                /* set default encoder settings */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
                BTST_P_DEFAULT(pContext->encodeSettings.displayFormat, NEXUS_VideoFormat_e480p);
                #if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT /* 416x224p30@400Kbps */
                BTST_P_DEFAULT(pContext->encodeSettings.encoderBitrate, 400000);
                #else /* 480p@2Mbps */
                pContext->encodeSettings.bCustom = true;
                pContext->encodeSettings.displayFormat = NEXUS_VideoFormat_eCustom2;
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.width, 720);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.height, 480);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.refreshRate, 59940);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.aspectRatio, NEXUS_DisplayAspectRatio_eSar);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x, 1);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y, 1);
                BTST_P_DEFAULT(pContext->encodeSettings.encoderBitrate, 2000000);
                #endif
#else
                pContext->encodeSettings.bCustom = true;
                pContext->encodeSettings.displayFormat = NEXUS_VideoFormat_eCustom2;
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.width, 1280);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.height, 720);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.refreshRate, 59940);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.aspectRatio, NEXUS_DisplayAspectRatio_eSar);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x, 1);
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y, 1);
                BTST_P_DEFAULT(pContext->encodeSettings.encoderBitrate, 3000000);
#endif
                BTST_P_DEFAULT(pContext->encodeSettings.encoderFrameRate, NEXUS_VideoFrameRate_e29_97);
                pContext->encodeSettings.encoderGopStructureFramesP=29;
                pContext->encodeSettings.encoderGopStructureFramesB=0;
                pContext->encodeSettings.encoderVideoCodec=NEXUS_VideoCodec_eH264;
                pContext->encodeSettings.encoderProfile=NEXUS_VideoCodecProfile_eMain;/* default CABAC on */
                pContext->encodeSettings.encoderLevel=NEXUS_VideoCodecLevel_e31;
                pContext->encodeSettings.bAudioEncode=true;/* transcode audio to AAC */
                pContext->encodeSettings.encoderAudioCodec=NEXUS_AudioCodec_eAac;
            }
            if(!strcmp("-video_size",argv[i]) && (i+1 < argc)) {
                if (sscanf(argv[++i], "%u,%u", &pContext->encodeSettings.customFormatSettings.width,
                   &pContext->encodeSettings.customFormatSettings.height) != 2) {
                    print_usage();
                    return -1;
                }
                pContext->encodeSettings.bCustom = true;/* custom encoder size */
                BTST_P_DEFAULT(pContext->encodeSettings.customFormatSettings.refreshRate, 59940);
                fprintf(stderr, "Video size %ux%u\n", pContext->encodeSettings.customFormatSettings.width, pContext->encodeSettings.customFormatSettings.height);
            }
            if(!strcmp("-interlaced",argv[i])) {
                pContext->encodeSettings.customFormatSettings.interlaced = true;
                fprintf(stderr, "Video is interlaced\n");
            }
            if(!strcmp("-video_bitrate",argv[i]) && (i+1 < argc)) {
                pContext->encodeSettings.encoderBitrate = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Video bitrate is %u bps\n", pContext->encodeSettings.encoderBitrate);
            }
            if(!strcmp("-video_framerate",argv[i]) && (i+1 < argc)) {
                pContext->encodeSettings.encoderFrameRate = mylookup(g_videoFrameRateStrs, argv[++i]);
                fprintf(stderr, "Video frame rate is %s fps\n", argv[i]);
            }
            if(!strcmp("-video_refreshrate",argv[i]) && (i+1 < argc)) {
                float rate;
                sscanf(argv[++i], "%f", &rate);
                pContext->encodeSettings.customFormatSettings.refreshRate = rate * 1000;
                fprintf(stderr, "Video refresh rate is %f Hz\n", rate);
            }
            if(!strcmp("-fifo",argv[i])) {
                g_bFifo = true;
                fprintf(stderr, "Enabled fifo record...\n");
            }
            if(!strcmp("-NRTwrap",argv[i])||!strcmp("-loop",argv[i])) {
                g_bNonRealTimeWrap = true;
                fprintf(stderr, "Enabled file wraparound...\n");
            }
            if(!strcmp("-autoquit",argv[i])) {
                g_bAutoQuit= true;
                fprintf(stderr, "Enabled auto quit...\n");
            }
            if(!strcmp("-cc",argv[i])) {
                g_bEncodeCC = pContext->bEncodeCCUserData = true;
                fprintf(stderr, "Enabled CC data transcode...\n");
            }
            if(!strcmp("-bwSaving",argv[i])) {
                g_bBandwidthSaving = true;
                fprintf(stderr, "Enabled P-frame bandwidth saving mode...\n");
                if(++i >= argc) break;
                if(!strcmp("singleRefP",argv[i])) {
                    pContext->encodeSettings.singleRefP = true;
                    if(++i >= argc) break;
                }
                if(!strcmp("requiredPatchesOnly",argv[i])) {
                    pContext->encodeSettings.requiredPatchesOnly = true;
                }
            }
            if(!strcmp("-tsud",argv[i])) {
                g_bTsUserData = true;
                fprintf(stderr, "Enabled TS layer user data transcode...\n");
            }
            if(!strcmp("-nodspmix",argv[i])) {
                g_bNoDspMixer = true;
                fprintf(stderr, "No DSP mixer for audio transcode...\n");
            }
            if(!strcmp("-multichan",argv[i])) {
                g_bMultiChanAudio = true;
                fprintf(stderr, "Multi-channel format for audio source. must have fw mixer..\n");
            }
            if(!strcmp("-msaudio",argv[i])) {
                g_bSecondAudio = true;
                fprintf(stderr, "Multi-stream audio source. must have fw mixer.\n");
            }
            if(!strcmp("-nrtRate",argv[i])) {
                g_nrtRate = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "NRT rate = %u x\n", g_nrtRate);
            }
            if(!strcmp("-delayTolerance",argv[i])) {
                g_muxLatencyTolerance = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Mux sw latency tolerance = %u ms.\n", g_muxLatencyTolerance);
            }
            if(!strcmp("-muxDescs",argv[i])) {
                g_muxDescs = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Number of mux transport descriptors = %u.\n", g_muxDescs);
            }
            if(!strcmp("-msp",argv[i])) {
                g_msp = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Mux Service Period = %u ms.\n", g_msp);
            }
            if(!strcmp("-rsp",argv[i])) {
                g_rsp = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Record Service Period = %u ms.\n", g_rsp);
            }
            if(!strcmp("-vbr",argv[i])) {
                pContext->encodeSettings.vbr = true;
                fprintf(stderr, "VBR video encode.\n");
            }
            if(!strcmp("-remux",argv[i])) {
                pContext->bRemux = true;
                fprintf(stderr, "Remux encode.\n");
            }
            if(!strcmp("-adaptiveLowDelay",argv[i])) {
                pContext->bAdaptiveLatencyStart = true;
                fprintf(stderr, "Adaptive low delay encode start.\n");
            }
            if(!strcmp("-cabacOff",argv[i])) {
                pContext->encodeSettings.entropyCoding = NEXUS_EntropyCoding_eCavlc;
                fprintf(stderr, "Disable entropy coding.\n");
            }
         if(!strcmp("-disableFrameDrop",argv[i])) {
            pContext->encodeSettings.disableFrameDrop = true;
            fprintf(stderr, "Disable HRD frame drop.\n");
         }
            if(!strcmp("-customDelay",argv[i])) {
                g_bCustomDelay = true;
                fprintf(stderr, "customize video encoder delay.\n");
            }
            if(!strcmp("-vfr",argv[i])) {
                pContext->bVariableFrameRate = true;
                fprintf(stderr, "Variable frame rate video encode.\n");
            }
            if(!strcmp("-sfr",argv[i])) {
                g_bSparseFrameRate = true;
                fprintf(stderr, "Sparse frame rate video encode.\n");
            }
            if(!strcmp("-dynamicGop",argv[i])) {
                pContext->encodeSettings.newGopOnSceneChange = true;
                fprintf(stderr, "Enabled new GOP on scene change.\n");
            }
            if(!strcmp("-openGop",argv[i])) {
                pContext->encodeSettings.openGop = true;
                fprintf(stderr, "Enabled open GOP.\n");
            }
            if(!strcmp("-gopDuration",argv[i])) {
                pContext->encodeSettings.gopDuration = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "GOP duration = %u ms.\n", pContext->encodeSettings.gopDuration);
                if((i+1) < argc) { /* check optional -maxSize */
                    if(!strcmp("-adaptiveDuration", argv[i+1])) {
                        i++;
                        pContext->encodeSettings.adaptiveDuration = true;
                        fprintf(stderr, "Adaptive GOP duration.\n");
                    }
                }
            }
            if(!strcmp("-3d",argv[i])) {
                pContext->encodeSettings.b3D = true;
                fprintf(stderr, "3D video encode.\n");
            }
            if(!strcmp("-simul",argv[i])) {
                g_bSimulXcode = true;
                fprintf(stderr, "Simultaneous ABR encode.\n");
            }
            if(!strcmp("-noVideo",argv[i])) {
                pContext->bNoVideo = true;
                fprintf(stderr, "No video encode.\n");
            }
            if(!strcmp("-noVideoWindow",argv[i])) {
                pContext->inputSettings.bVideoWindowDisabled = true;
                fprintf(stderr, "No video input encode (maybe GUI encode).\n");
            }
            if(!strcmp("-bypassVideoFilter",argv[i])) {
                pContext->bypassVideoFilter = true;
                fprintf(stderr, "Bypass video filters.\n");
            }
            if(!strcmp("-seamless",argv[i])) {
                pContext->bSeamlessFormatSwitch = true;
                fprintf(stderr, "Seamless transcode format switch.\n");
                if(++i > argc-1) break;
                if(!strcmp("1080p",argv[i])) {
                    pContext->b1080pCaptureFormat = true;
                    fprintf(stderr, "Up to 1080p.\n");
                }
            }
            if(!strcmp("-type",argv[i])) {
                i++;
                if(!strcmp("single",argv[i])) {
                    g_eVideoEncoderType = NEXUS_VideoEncoderType_eSingle;
                }
                else if(!strcmp("multiNRT",argv[i])) {
                    g_eVideoEncoderType = NEXUS_VideoEncoderType_eMultiNonRealTime;
                }
                else {
                    g_eVideoEncoderType = NEXUS_VideoEncoderType_eMulti;
                }
                fprintf(stderr, "Set video encoder type %d...\n", g_eVideoEncoderType);
            }
            if(!strcmp("-scr",argv[i])) {
                g_bScriptMode = true;
                g_scriptModeSleepTime = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Enabled script mode and quit in %u seconds...\n", g_scriptModeSleepTime);
            }
            if(!strcmp("-cxt",argv[i])) {
                g_selectedXcodeContextId = strtoul(argv[++i], NULL, 0);
                g_selectedXcodeContextId = (g_selectedXcodeContextId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):g_selectedXcodeContextId;
                BKNI_Memcpy(&xcodeContext[g_selectedXcodeContextId], pContext, sizeof(TranscodeContext));
                BKNI_Memset(pContext, 0, sizeof(TranscodeContext));
                pContext = &xcodeContext[g_selectedXcodeContextId];
                fprintf(stderr, "Select xcoder context %d...\n", g_selectedXcodeContextId);
            }
            if(!strcmp("-madoff",argv[i])) {
                g_bStartWithoutMad = true;
                fprintf(stderr, "Start transcoder with MADR off.\n");
            }
            if(!strcmp("-win",argv[i])) {
                i++;
                if(!strcmp("box", argv[i])) {
                    g_contentMode = NEXUS_VideoWindowContentMode_eBox;
                } else if(!strcmp("zoom", argv[i])) {
                    g_contentMode = NEXUS_VideoWindowContentMode_eZoom;
                }
                fprintf(stderr, "Start transcoder with content mode %d.\n", g_contentMode);
            }
            if(!strcmp("-loopbackoff",argv[i])) {
                g_bLoopbackPlayer = false;
                fprintf(stderr, "Start transcoder with loopback player off.\n");
            }
            if(!strcmp("-vdecZeroUp", argv[i])) {
                g_bDecoderZeroUp = true;
                fprintf(stderr, "Start transcoder with video decoder 0 up.\n");
                if((i+3) < argc) { /* check optional -maxSize */
                    if(!strcmp("-maxSize", argv[i+1])) {
                        i++;
                        g_vdecMaxWidth  = strtoul(argv[++i], NULL, 0);
                        g_vdecMaxHeight = strtoul(argv[++i], NULL, 0);
                        fprintf(stderr, "Max decode resolution: %u x %u\n", g_vdecMaxWidth, g_vdecMaxHeight);
                    }
                }
            }
            if(!strcmp("-sd",argv[i])) {
                g_bEnableDebugSdDisplay = true;
                fprintf(stderr, "Enabled CVBS debug display for source decoder.\n");
            }
            if(!strcmp("-force48KbpsAacPlus",argv[i])) {
                g_bForce48KbpsAACplus = true;
                fprintf(stderr, "Force 48Kbps bitrate for HE AAC encoding.\n");
            }
            if(!strcmp("-32KHzAudio",argv[i])) {
                g_b32KHzAudio = true;
                fprintf(stderr, "Force 32KHz sample rate for audio encoding.\n");
            }
            if(!strcmp("-tts_in",argv[i])) {
                i++;
                if(!strcmp("binary",argv[i])) {
                    g_TtsInputType = NEXUS_TransportTimestampType_e32_Binary;
                }
                else {
                    g_TtsInputType = NEXUS_TransportTimestampType_e32_Mod300;
                }
                fprintf(stderr, "Input TS transport packets prepended with 4-byte timestamps mode %d.\n", g_TtsInputType);
            }
            if(!strcmp("-tts_out",argv[i])) {
                i++;
                if(!strcmp("binary",argv[i])) {
                    g_TtsOutputType = NEXUS_TransportTimestampType_e32_Binary;
                }
                else {
                    g_TtsOutputType = NEXUS_TransportTimestampType_e32_Mod300;
                }
                fprintf(stderr, "Output TS transport packets prepended with 4-byte timestamps mode %d.\n", g_TtsOutputType);
            }
            if(!strcmp("-progressive_only",argv[i])) {
                pContext->encodeSettings.videoEncoderMemConfig.progressiveOnly = true;
                fprintf(stderr, "Xcode output is progressive only format.\n");
            }
            if(!strcmp("-maxDimension",argv[i])) {
                pContext->encodeSettings.videoEncoderMemConfig.maxWidth = strtoul(argv[++i], NULL, 0);
                pContext->encodeSettings.videoEncoderMemConfig.maxHeight = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "Max encode resolution: %u x %u\n",
                    pContext->encodeSettings.videoEncoderMemConfig.maxWidth, pContext->encodeSettings.videoEncoderMemConfig.maxHeight);
            }

            if(!strcmp("-segment",argv[i])) {
                pContext->encodeSettings.segmentRC.enable = true;
                pContext->encodeSettings.segmentRC.duration = 3000; /* 3 seconds default segment duration */
                pContext->encodeSettings.segmentRC.upperTolerance = 20; /* 20% upper bitrate tolerance */
                pContext->encodeSettings.segmentRC.lowerTolerance = 20; /* 20% lower bitrate tolerance */
                fprintf(stderr, "Enabled segment based rate control video encoding.\n");
                if((i+2) < argc) { /* check optional -segment duration and upper/lower tolerance */
                    if(!strcmp("-duration", argv[i+1])) {
                        i++;
                        pContext->encodeSettings.segmentRC.duration  = strtoul(argv[++i], NULL, 0);
                        fprintf(stderr, "segment duration: %u ms\n", pContext->encodeSettings.segmentRC.duration);
                    }
                }
                if(!strcmp("-bitRateTolerance",argv[i]) && (i+1 < argc)) {/* check optional segment bitrate tolerance */
                    if (sscanf(argv[++i], "%u,%u", &pContext->encodeSettings.segmentRC.upperTolerance,
                       &pContext->encodeSettings.segmentRC.lowerTolerance) != 2) {
                        print_usage();
                        return -1;
                    }
                }
            }
            if(!strcmp("-dropBardata",argv[i])) {
                pContext->dropBardata = true;
                fprintf(stderr, "Drop bar data encoding.\n");
            }
            if(!strcmp("-mcpb",argv[i])) {
                pContext->bMCPB = true;
                fprintf(stderr, "Use MCPB transport channel\n");
            }
            if(!strcmp("-logger",argv[i])) {
               g_loggerContext.enable = true;
               g_loggerContext.frequency = strtoul(argv[++i], NULL, 0);
               fprintf(stderr, "Logging enabled (%u ms poll frequency)\n", g_loggerContext.frequency);
            }
            if(!strcmp("-printStatus",argv[i])) {
               g_bPrintStatus = true;
               fprintf(stderr, "Auto Print Status when transcoder is closed\n");
            }
            if(!strcmp("-pcrInterval",argv[i])) {
                g_pcrInterval = strtoul(argv[++i], NULL, 0);
                fprintf(stderr, "PCR interval set to %u ms...\n", g_pcrInterval);
            }
            if(!strcmp("-interleaveMode",argv[i])) {
                i++;
                if(!strcmp("pts",argv[i])) {
                   pContext->interleaveMode = NEXUS_StreamMuxInterleaveMode_ePts;
                   fprintf(stderr, "Use PTS interleave\n");
                }
            }
            if(!strcmp("-audioPesPacking",argv[i])) {
               g_audioPesPacking = true;
               fprintf(stderr, "Use Audio PES Packing\n");
            }
            if(!strcmp("-onePtsPerSegment",argv[i])) {
               g_bOnePtsPerSegment = true;
               fprintf(stderr, "One PTS per Segment\n");
            }
            if(!strcmp("-ptsSeed",argv[i])) {/* check if pts seed is requested */
               pContext->ptsSeed.bEnable = true;
               if (i+1 < argc) {
                  pContext->ptsSeed.uiValue = strtoul(argv[++i], NULL, 0);

                  fprintf(stderr, "Enabled pts seed (%08x) in 45 Khz...\n", pContext->ptsSeed.uiValue );
               } else {
                  print_usage();
                  return -1;
               }
            }
            if(!strcmp("-itfpoff",argv[i])) {
               g_bEnableFieldPairing = false;
                fprintf(stderr, "Disable field pairing (ITFP).\n");
            }
        }
        if(g_bSecondAudio) g_bNoDspMixer = false;
        if(!pContext->inputSettings.probe) {
            config_xcoder_context(pContext);/* transcoder 0 context user settings */
        }
    }

    /* init platform */
    B_Os_Init();
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    /* enable frontend if the 1st xcoder needs it for now; not always enable to avoid slow frontend init for other cases;
       TODO: init frontend in case 1st disable but 2nd enables frontend. */
    platformSettings.openFrontend = (pContext->inputSettings.resource == BTST_RESOURCE_QAM);
    /* audio PI supports 4 by default; we need one extra mixers for each transcoders; */
    platformSettings.audioModuleSettings.dspAlgorithmSettings.typeSettings[NEXUS_AudioDspAlgorithmType_eAudioEncode].count = NEXUS_NUM_VIDEO_ENCODERS;
    platformSettings.audioModuleSettings.maxAudioDspTasks += NEXUS_NUM_VIDEO_ENCODERS + 1;/* to support quad xcodes + loopback decode */
    platformSettings.audioModuleSettings.numCompressedBuffers += NEXUS_NUM_VIDEO_ENCODERS;/* for quad xcodes */
    platformSettings.audioModuleSettings.numPcmBuffers = NEXUS_NUM_VIDEO_ENCODERS + 1;/* to support quad xcodes and loopback decode */
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&g_platformConfig);
    {
        unsigned count = 0;
        bool headless = false;
        NEXUS_VideoEncoderCapabilities capabilities;
        NEXUS_DisplayCapabilities displayCapabilities;
        NEXUS_GetVideoEncoderCapabilities(&capabilities);
        for(i=0; i< NEXUS_MAX_VIDEO_ENCODERS; i++) {
            BDBG_MSG(("%u video encoder[%u]/display[%u] supported? %s.", BCHP_CHIP, i, capabilities.videoEncoder[i].displayIndex,
                capabilities.videoEncoder[i].supported? "Yes":"No"));
            count += capabilities.videoEncoder[i].supported;
            /* if encoder masters display 0, assumed headless */
            if(capabilities.videoEncoder[i].supported && capabilities.videoEncoder[i].displayIndex == 0) headless = true;
        }
        if(count == 0) { BDBG_WRN(("This box mode doesn't support video encoding!")); goto done; }
        NEXUS_GetDisplayCapabilities(&displayCapabilities);
        /* if display 0 has no video window, assumed headless */
        if(displayCapabilities.display[0].numVideoWindows == 0) headless = true;
        BDBG_MSG(("Display[0] has %u video windows.", displayCapabilities.display[0].numVideoWindows));
        if(headless) {
            g_bLoopbackPlayer = false;
            BDBG_WRN(("This box mode is headless, so disable loopback player!"));
        }
    }

    for(i=0; i< NEXUS_NUM_VIDEO_ENCODERS; i++) {
        xcodeContext[i].mutexStarted = B_Mutex_Create(NULL);
        xcodeContext[i].contextId = i;
    }

    if(pContext->inputSettings.probe) {
        struct probe_request probe_request;
        struct probe_results probe_results;
        int rc;
        char *fname, *ptr;
        char program[20]={'\0',};

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = pContext->inputSettings.fname;
        probe_request.program = pContext->inputSettings.program;
        rc = probe_media_request(&probe_request, &probe_results);
        if (rc) {
            BDBG_ERR(("media probe failed to parse '%s'", probe_request.streamname));
            return BERR_TRACE(rc);
        }
        if(probe_results.num_audio==0 && probe_results.num_video==0) {
            BDBG_ERR(("media probe failed to find any audio or video program in '%s'", probe_request.streamname));
            return BERR_TRACE(rc);
        }
        pContext->inputSettings.bUseStreamAsIndex = probe_results.useStreamAsIndex;
        pContext->inputSettings.eStreamType = probe_results.transportType;
        g_TtsInputType = probe_results.timestampType;
        pContext->bNoVideo = (probe_results.num_video == 0);
        pContext->inputSettings.eVideoCodec = probe_results.video[0].codec;
        pContext->inputSettings.iVideoPid   = probe_results.video[0].pid;
        pContext->inputSettings.bAudioInput = probe_results.num_audio;
        pContext->inputSettings.eAudioCodec = probe_results.audio[0].codec;
        pContext->inputSettings.iAudioPid   = probe_results.audio[0].pid;
        print_input_parameters(pContext);
        /* strip out directory path */
        fname = ptr = pContext->inputSettings.fname;
        while(*ptr) {
            if(*ptr == '/') fname = ptr + 1;
            ptr++;
        };
        /* insert program string or not in output file name */
        if(pContext->inputSettings.program) BKNI_Snprintf(program, 20, "_program%u", pContext->inputSettings.program);
        if(BKNI_Snprintf(pContext->encodeSettings.fname, 256, "/data/videos/%s%s_%ux%u%c%s_%ubps.ts", fname,
            program, pContext->encodeSettings.customFormatSettings.width, pContext->encodeSettings.customFormatSettings.height,
            pContext->encodeSettings.customFormatSettings.interlaced?'i':'p',
            lookup_name(g_videoFrameRateStrs, pContext->encodeSettings.encoderFrameRate),
            pContext->encodeSettings.encoderBitrate) > 256) {
            BDBG_ERR(("Output file name(%s) is too long > 256 characters!", pContext->encodeSettings.fname));
            return -1;
        }
        print_output_parameters(pContext);
        /* allow interactive menu control */
        pContext->inputSettings.bConfig = true;
        /* automatically quit the test at the end of file */
        g_bAutoQuit = true;
    }
again:
    /* mutex to protect init bringup of the xcoder and loopback player */
    B_Mutex_Lock(pContext->mutexStarted);

    start_logger();

    /* bringup the transcode context */
    bringup_transcode(pContext);

    /****************************************************
     * set up xcoder0 loopback decode/display for debug purpose
     */
    if(g_bLoopbackPlayer && BTST_LOOPBACK_DISPLAY_IDX != get_display_index(pContext->contextId)) {
        xcode_loopback_setup(pContext);
        g_loopbackStarted = true;
        g_loopbackXcodeId = g_selectedXcodeContextId;
    } else {/* disable loopback player if loopback display is used by the encoder display */
        g_loopbackXcodeId = -1;
    }
    B_Mutex_Unlock(pContext->mutexStarted);

    /****************************************************
     *                       key handler                                                 *
     *****************************************************/
    /* wait for 'q' to exit the test */
    if(g_bScriptMode)
    {
        keyHandler(xcodeContext);
    }
    else if (pContext->inputSettings.bConfig)
    {
        B_ThreadSettings threadSettings;
        B_ThreadHandle keyHandle;

        g_eotEvent = B_Event_Create(NULL);
        B_Event_Reset(g_eotEvent);
        B_Thread_GetDefaultSettings(&threadSettings);
        keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, xcodeContext, &threadSettings);
        NEXUS_Profile_Start();
        while(B_Event_Wait(g_eotEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
        BDBG_MSG(("main thread to quit..."));
        g_keyReturn = 'q';
        BKNI_Sleep(1);
        NEXUS_Profile_Stop("transcode_ts");
        B_Thread_Destroy(keyHandle);
        B_Event_Destroy(g_eotEvent);
        g_eotEvent = NULL;
    }
    else
    {
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
        NEXUS_VideoEncoderSettings videoEncoderConfig;
        NEXUS_DisplaySettings displaySettings;
        BDBG_WRN(("Auto test iteration: %d\n", iteration));
        BKNI_Sleep(30000);/* 30 seconds */
        /* To Do add BVN status and ViCE status check */
        /* Enable MADR*/
        window_mad(pContext->windowTranscode, false, true, false);
        /* change resolution */
        NEXUS_Display_GetSettings(pContext->displayTranscode, &displaySettings);
        displaySettings.format = NEXUS_VideoFormat_e480p;
        NEXUS_Display_SetSettings(pContext->displayTranscode, &displaySettings);
        BDBG_WRN(("format switch to 480p60"));
        NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.bitrateMax = 2000000;
        NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
        BDBG_WRN(("bitrate switch to 2Mbps"));
        NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.variableFrameRate = pContext->bVariableFrameRate;
        videoEncoderConfig.sparseFrameRate = g_bSparseFrameRate;
        videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
        videoEncoderConfig.streamStructure.framesP = 29;
        videoEncoderConfig.streamStructure.framesB = 0;
        NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
        BDBG_WRN(("frame rate switch to 30fps\n"));
        BKNI_Sleep(30000);/* 30 seconds */

        /* change resolution */
        NEXUS_Display_GetSettings(pContext->displayTranscode, &displaySettings);
        displaySettings.format = NEXUS_VideoFormat_e720p;
        NEXUS_Display_SetSettings(pContext->displayTranscode, &displaySettings);
        BDBG_WRN(("format switch to 720p60"));
        NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.bitrateMax = 10000000; /* 10Mbps */
        NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
        BDBG_WRN(("bitrate switch to 10 Mbps"));
        NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.variableFrameRate = pContext->bVariableFrameRate;
        videoEncoderConfig.sparseFrameRate = g_bSparseFrameRate;
        videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e60;
        videoEncoderConfig.streamStructure.framesP = 29;
        videoEncoderConfig.streamStructure.framesB = 0;
        NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &videoEncoderConfig);
        BDBG_WRN(("frame rate switch to 60fps"));
#else
        BDBG_WRN(("Auto test iteration: %d\n", iteration));
#endif
        BKNI_Sleep(30000);
        g_keyReturn = iteration > TEST_ITERATIONS ? 'q' : 'c'; /* continue looping until quit */
    }

    /* bringdown loopback path */
    if(g_bLoopbackPlayer && g_loopbackStarted) {
        xcode_loopback_shutdown(&xcodeContext[g_loopbackXcodeId]);
    }
    for(i = 0; i < NEXUS_NUM_VIDEO_ENCODERS; i++) {
        B_Mutex_Lock(xcodeContext[i].mutexStarted);
        if(xcodeContext[i].videoEncoder || xcodeContext[i].audioMuxOutput) {
           xcodeContext[i].bShuttingDown = true;
           shutdown_transcode(&xcodeContext[i]);
           xcodeContext[i].bShuttingDown = false;
        }
        B_Mutex_Unlock(xcodeContext[i].mutexStarted);
        if(xcodeContext[i].nrtEofHandle) {
            B_Event_Set(xcodeContext[i].eofEvent);
            BKNI_Sleep(1);
            B_Thread_Destroy(xcodeContext[i].nrtEofHandle);
            xcodeContext[i].nrtEofHandle = NULL;
        }
        if(xcodeContext[i].eofEvent) {
            B_Event_Destroy(xcodeContext[i].eofEvent);
            xcodeContext[i].eofEvent = NULL;
        }
    }

    if(g_keyReturn != 'q')
    {
        iteration++;
        BDBG_WRN(("Start %d iteration.....", iteration));
        pContext = &xcodeContext[0];
        goto again;
    }

    stop_logger();

    for(i=0; i< NEXUS_NUM_VIDEO_ENCODERS; i++) {
        B_Mutex_Destroy(xcodeContext[i].mutexStarted);
    }
done:
    /* uninit platform */
    NEXUS_Platform_Uninit();
    B_Os_Uninit();
#endif
    return 0;
}
#else
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#endif

/* End of file */
