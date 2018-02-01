/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
*****************************************************************************/


/* TODO:
 * Need a lookup table for NEXUS_VideoEncoderStopMode
 * Feedback on session commands.
 * Is there only one display?
 */


#include "nexus_platform.h"
#if !NEXUS_HAS_VIDEO_ENCODER
#include <stdio.h>

int main(int argc, const char *argv[]) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#else

#include "nexus_video_decoder.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_adj.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"

#if NEXUS_HAS_PLAYBACK /*&& NEXUS_HAS_VIDEO_ENCODER*/
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#endif

#if NEXUS_HAS_STREAM_MUX
#include "nexus_record.h"
#include "nexus_file.h"
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

#if NEXUS_HAS_FILE_MUX
#include "nexus_file_mux.h"
#endif

/*#include "bcmplayer.h"*/
#include "bmedia_probe.h"
#include "bfile_stdio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "b_os_lib.h"
#include <sys/select.h>

#include "namevalue.inc"

BDBG_MODULE(transcode_x);
BDBG_FILE_MODULE(trnsx_cmds);
BDBG_FILE_MODULE(contcounter);

#include "media_probe.c"


/*#define SIMUL_DISPLAY 1*/

/*
 * Local data types.
 */

#if  NEXUS_HAS_FRONTEND
#define BTST_SUPPORT_FRONTEND    1
#else
#define BTST_SUPPORT_FRONTEND    0
#endif

/***********************************
 *   PID assignments
 * TODO: this was lifted for stream mux, should these be exposed to the user?
 */
#define BTST_MUX_PCR_PID        (0x11)
#define BTST_MUX_VIDEO_PID      (0x12)
#define BTST_MUX_AUDIO_PID      (0x13)
#define BTST_MUX_USER_DATA_PID  (BTST_MUX_AUDIO_PID+6)
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)

/***********************************
 *   Closed Caption user data count
 */
#define BTST_MAX_CC_DATA_COUNT          32

/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define BTST_PSI_QUEUE_CNT 4 /* every second insert a PSI, so it takes 4 secs to circle; hopefully its transfer won't be delay that far */


/* replaced all of the following with NEXUS_ANY_ID */
#if 0
/***********************************
 *   Playpump assignments
 * TODO: this was lifted for stream mux, does it play well with file mux?
 * TODO: could these just be NEXUS_ANY_ID?
 */
#define BTST_LOOPBACK_PLAYPUMP_IDX           0
#define BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX  (NEXUS_NUM_VIDEO_ENCODERS)
#define BTST_STREAM_MUX_AUDIO_PLAYPUMP_IDX  (NEXUS_NUM_VIDEO_ENCODERS+1)
#define BTST_STREAM_MUX_PCR_PLAYPUMP_IDX    (NEXUS_NUM_VIDEO_ENCODERS+2)
#define BTST_SOURCE_PLAYPUMP_IDX            (NEXUS_NUM_VIDEO_ENCODERS+3)
#endif

/***********************************
 *   TS user data all pass
 */
#define BTST_TS_USER_DATA_ALL           (unsigned)(-1)
#define BTST_MAX_MESSAGE_FILTERS        20


/* TODO: this should be based on the system capabilities*/
#define TRNSX_NUM_ENCODES 4

typedef enum TRNSX_Source
{
    TRNSX_Source_eFile=0,
    TRNSX_Source_eHDMI,
    TRNSX_Source_eQAM,
    TRNSX_Source_eMax

} TRNSX_Source;

static const namevalue_t g_sourceStrs[] = {
    {"file", TRNSX_Source_eFile},
    {"hdmi", TRNSX_Source_eHDMI},
    {"qam", TRNSX_Source_eQAM},
    {NULL, 0}
};

typedef enum TRNSX_State
{
    TRNSX_State_eUninit=0,
    TRNSX_State_eClosed,
    TRNSX_State_eOpened,
    TRNSX_State_eRunning,
    TRNSX_State_ePaused,
    TRNSX_State_eStopped,
    TRNSX_State_eMax

} TRNSX_State;

static const namevalue_t g_stateStrs[] = {
    {"uninit", TRNSX_State_eUninit},
    {"closed", TRNSX_State_eClosed},
    {"opened", TRNSX_State_eOpened},
    {"running", TRNSX_State_eRunning},
    {"paused", TRNSX_State_ePaused},
    {"stopped", TRNSX_State_eStopped},
    {NULL, 0}
};

typedef enum TRNSX_SleepUnit
{
    TRNSX_SleepUnit_eSecs=0,
    TRNSX_SleepUnit_eMsecs,
    TRNSX_SleepUnit_eMax

} TRNSX_SleepUnit;

static const namevalue_t g_sleepUnitStrs[] = {
    {"secs", TRNSX_SleepUnit_eSecs},
    {"msecs", TRNSX_SleepUnit_eMsecs},
    {NULL, 0}
};

typedef enum TRNSX_TransportType
{
    TRNSX_TransportType_eEs=0,  /* Read data from buffer */
    TRNSX_TransportType_eIVF,   /* Use the file mux */
    TRNSX_TransportType_ePES,
    TRNSX_TransportType_eMp4,
    TRNSX_TransportType_eTS,    /* Use the stream mux*/
    TRNSX_TransportType_eMp4Fragmented,
    TRNSX_TransportType_eMax

} TRNSX_TransportType;

static const namevalue_t g_trnsxTransportTypeStrs[] = {
    {"es", TRNSX_TransportType_eEs},
    {"ivf", TRNSX_TransportType_eIVF},
    {"pes", TRNSX_TransportType_ePES},
    {"mp4", TRNSX_TransportType_eMp4},
    {"ts", TRNSX_TransportType_eTS},
    {"fmp4", TRNSX_TransportType_eMp4Fragmented},
    {NULL, 0}
};

/*
 * Lookup tables for Nexus enums
 */

static const namevalue_t g_encoderTypeStrs[] = {
    {"multi", NEXUS_VideoEncoderType_eMulti},
    {"single", NEXUS_VideoEncoderType_eSingle},
    {"multiNRT", NEXUS_VideoEncoderType_eMultiNonRealTime},
    {"custom", NEXUS_VideoEncoderType_eCustom},
    {NULL, 0}
};

static const namevalue_t g_tristateEnableStrs[] = {
    {"eDisabled", NEXUS_TristateEnable_eDisable},
    {"eEnabled", NEXUS_TristateEnable_eEnable},
    {"eNotSet", NEXUS_TristateEnable_eNotSet},
    {NULL, 0}
};

static const namevalue_t g_entropyCodingStrs[] = {
    {"auto", NEXUS_EntropyCoding_eAuto},
    {"cavlc", NEXUS_EntropyCoding_eCavlc},
    {"cabac", NEXUS_EntropyCoding_eCabac},
    {NULL, 0}
};

static const namevalue_t g_encoderStopModeStrs[] = {
    {"normal", NEXUS_VideoEncoderStopMode_eNormal},
    {"immediate", NEXUS_VideoEncoderStopMode_eImmediate},
    {"abort", NEXUS_VideoEncoderStopMode_eAbort},
    {NULL, 0}
};

static const namevalue_t g_displayTypeStrs[] = {
    {"auto", NEXUS_DisplayType_eAuto},
    {"lvds", NEXUS_DisplayType_eLvds},
    {"dvo", NEXUS_DisplayType_eDvo},
    {"bypass", NEXUS_DisplayType_eBypass},
    {NULL, 0}
};

static const namevalue_t g_displayTimingGeneratorStrs[] = {
    {"primary", NEXUS_DisplayTimingGenerator_ePrimaryInput},
    {"secondary", NEXUS_DisplayTimingGenerator_eSecondaryInput},
    {"tertiary", NEXUS_DisplayTimingGenerator_eTertiaryInput},
    {"hdmidvo", NEXUS_DisplayTimingGenerator_eHdmiDvo},
    {"656output", NEXUS_DisplayTimingGenerator_e656Output},
    {"encoder", NEXUS_DisplayTimingGenerator_eEncoder},
    {"auto", NEXUS_DisplayTimingGenerator_eAuto},
    {NULL, 0}
};

static const namevalue_t g_transportTimestampTypeStrs[] = {
    {"none", NEXUS_TransportTimestampType_eNone},
    {"mod300", NEXUS_TransportTimestampType_eMod300},
    {"binary", NEXUS_TransportTimestampType_eBinary},
    {"mod300_32", NEXUS_TransportTimestampType_e32_Mod300},
    {"binary_32", NEXUS_TransportTimestampType_e32_Binary},
    {"mod300_28", NEXUS_TransportTimestampType_e28_4P_Mod300},
    {NULL, 0}
};

static const namevalue_t g_streamMuxInterleaveModeStrs[] = {
    {"escr", NEXUS_StreamMuxInterleaveMode_eCompliant},
    {"pts", NEXUS_StreamMuxInterleaveMode_ePts},
    {NULL, 0}
};

#define TRNSX_SIZE_CMD 256
#define TRNSX_NUM_CMDS 256

typedef struct TRSNX_Command
{
    char    szKey[TRNSX_SIZE_CMD];

    bool    bValid; /* The following contain data. */

   char     szValue[TRNSX_SIZE_CMD];
   int32_t  iValue;

} TRSNX_Command;


/* macros for processing command parameters */

#define TRNSX_ELSE_IF_SIZE_T( _pcmd, _szkey, _element )                                     \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                      \
    {                                                                           \
        if ( _pcmd->bValid ) _element = _pcmd->iValue;                              \
        else BKNI_Printf("  current value for %s: %lu\n", _pcmd->szKey, (unsigned long) _element );  \
    }

#define TRNSX_ELSE_IF( _pcmd, _szkey, _element )                                     \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                      \
    {                                                                           \
        if ( _pcmd->bValid ) _element = _pcmd->iValue;                              \
        else BKNI_Printf("  current value for %s: %u\n", _pcmd->szKey, _element );  \
    }

#define TRNSX_ELSE_IF_LOOKUP( _pcmd, _szkey, _element, _list )                      \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                        \
    {                                                                               \
        if ( _pcmd->bValid ) _element = lookup(_list,_pcmd->szValue);             \
        else BKNI_Printf("  current value for %s: %d\n", _pcmd->szKey, _element );  \
    }

#define TRNSX_ERROR( _ptrans, _pcmd, _list, _bRet )                                                                                     \
    else                                                                                                                                \
    {                                                                                                                                   \
        BDBG_ERR(("%s::  command:: %s / unsupported parameter:: %s\n", BSTD_FUNCTION, lookup_name(_list, _ptrans->eActiveCmd),  _pcmd->szKey ));    \
        _bRet = false;                                                                                                                  \
    }                                                                                                                                   \


#define PARAM_MSG BKNI_Printf("using defaults, eventually will add support for setting parameters.\n")

#if 0
#if  BTST_SUPPORT_FRONTEND
static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend %p\n", (void*)frontend);

    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)frontend,
        qamStatus.fecLock, qamStatus.receiverLock);
}
#endif
#endif

#define TRNSX_SIZE_FILE_NAME 256

/* Specifies the active command. */
typedef enum TRNSX_Cmd {
    TRNSX_Cmd_eNone=0,
    TRNSX_Cmd_eVideoEncoderSetSettings,
    TRNSX_Cmd_eVideoEncoderOpen,
    TRNSX_Cmd_eVideoEncoderClose,
    TRNSX_Cmd_eVideoEncoderStart,
    TRNSX_Cmd_eVideoEncoderStop,
    TRNSX_Cmd_eVideoEncoderStatus,
    TRNSX_Cmd_eDisplayOpen,
    TRNSX_Cmd_eDisplayClose,
    TRNSX_Cmd_eDisplayCustomFormatSettings,
    TRNSX_Cmd_ePlaybackSetSettings,
    TRNSX_Cmd_ePlaybackOpen,
    TRNSX_Cmd_ePlaybackClose,
    TRNSX_Cmd_ePlaybackStart,
    TRNSX_Cmd_ePlaybackStop,
    TRNSX_Cmd_eVideoDecoderOpen,
    TRNSX_Cmd_eVideoDecoderClose,
    TRNSX_Cmd_eVideoDecoderSetSettings,
    TRNSX_Cmd_eVideoDecoderStart,
    TRNSX_Cmd_eVideoDecoderStop,
    TRNSX_Cmd_eVideoDecoderStatus,
    TRNSX_Cmd_ePlaypumpSetSettings,
    TRNSX_Cmd_ePlaypumpOpen,
    TRNSX_Cmd_eRecordSettings,
    TRNSX_Cmd_eRecordStart,
    TRNSX_Cmd_eRecordStop,
    TRNSX_Cmd_eRecordPause,
    TRNSX_Cmd_eRecordResume,
    TRNSX_Cmd_eMuxOpen,
    TRNSX_Cmd_eMuxClose,
    TRNSX_Cmd_eMuxStart,
    TRNSX_Cmd_eMuxStop,
    TRNSX_Cmd_eSession,
    TRNSX_Cmd_eTranscode,
    TRNSX_Cmd_eSleep,
    TRNSX_Cmd_eMax
} TRNSX_Cmd;

static const namevalue_t g_cmdStrs[] = {
    {"",                        TRNSX_Cmd_eNone},
    {"video_encoder_set_settings", TRNSX_Cmd_eVideoEncoderSetSettings},
    {"video_encoder_open",      TRNSX_Cmd_eVideoEncoderOpen},
    {"video_encoder_close",     TRNSX_Cmd_eVideoEncoderClose},
    {"video_encoder_start",     TRNSX_Cmd_eVideoEncoderStart},
    {"video_encoder_stop",      TRNSX_Cmd_eVideoEncoderStop},
    {"video_encoder_status",    TRNSX_Cmd_eVideoEncoderStatus},
    {"display_open",            TRNSX_Cmd_eDisplayOpen},
    {"display_close",           TRNSX_Cmd_eDisplayClose},
    {"display_custom_format",   TRNSX_Cmd_eDisplayCustomFormatSettings},
    {"playback_set_settings",   TRNSX_Cmd_ePlaybackSetSettings},
    {"playback_open",           TRNSX_Cmd_ePlaybackOpen},
    {"playback_close",          TRNSX_Cmd_ePlaybackClose},
    {"playback_start",          TRNSX_Cmd_ePlaybackStart},
    {"playback_stop",           TRNSX_Cmd_ePlaybackStop},
    {"video_decoder_open",      TRNSX_Cmd_eVideoDecoderOpen},
    {"video_decoder_close",     TRNSX_Cmd_eVideoDecoderClose},
    {"video_decoder_set_settings", TRNSX_Cmd_eVideoDecoderSetSettings},
    {"video_decoder_start",     TRNSX_Cmd_eVideoDecoderStart},
    {"video_decoder_stop",      TRNSX_Cmd_eVideoDecoderStop},
    {"video_decoder_status",    TRNSX_Cmd_eVideoDecoderStatus},
    {"playpump_set_settings",   TRNSX_Cmd_ePlaypumpSetSettings},
    {"playpump_set_open",       TRNSX_Cmd_ePlaypumpOpen},
    {"record_settings",         TRNSX_Cmd_eRecordSettings},
    {"record_start",            TRNSX_Cmd_eRecordStart},
    {"record_stop",             TRNSX_Cmd_eRecordStop},
    {"record_pause",            TRNSX_Cmd_eRecordPause},
    {"record_resume",           TRNSX_Cmd_eRecordResume},
    {"mux_open",                TRNSX_Cmd_eMuxOpen},
    {"mux_close",               TRNSX_Cmd_eMuxClose},
    {"mux_start",               TRNSX_Cmd_eMuxStart},
    {"mux_stop",                TRNSX_Cmd_eMuxStop},
    {"session",                 TRNSX_Cmd_eSession},
    {"transcode",               TRNSX_Cmd_eTranscode},
    {"sleep",                   TRNSX_Cmd_eSleep},
    {NULL, 0}
};

#if 0
/* Prefix for the command prompt, reminds the user which command is active. */
static const namevalue_t g_promptPrefixStrs[] = {
    {"", TRNSX_Cmd_eNone},
    /* encoder */
    {"veSet", TRNSX_Cmd_eVideoEncoderSetSettings},
    {"veOpen", TRNSX_Cmd_eVideoEncoderOpen},
    {"veClose", TRNSX_Cmd_eVideoEncoderClose},
    {"veStart", TRNSX_Cmd_eVideoEncoderStart},
    {"veStop", TRNSX_Cmd_eVideoEncoderStop},
    {"veStatus", TRNSX_Cmd_eVideoEncoderStatus },
    /* display */
    {"dpopen", TRNSX_Cmd_eDisplayOpen},
    {"dpclose", TRNSX_Cmd_eDisplayClose},
    {"dpcust", TRNSX_Cmd_eDisplayCustomFormatSettings},
    /* playback */
    {"pbSet", TRNSX_Cmd_ePlaybackSetSettings},
    {"pbOpen", TRNSX_Cmd_ePlaybackOpen},
    {"pbClose", TRNSX_Cmd_ePlaybackClose},
    {"pbStart", TRNSX_Cmd_ePlaybackStart},
    {"psStop", TRNSX_Cmd_ePlaybackStop},
    /* video decoder */
    {"vdSet", TRNSX_Cmd_eVideoDecoderSetSettings},
    {"vdOpen", TRNSX_Cmd_eVideoDecoderOpen},
    {"vdClose", TRNSX_Cmd_eVideoDecoderClose},
    {"vdStart", TRNSX_Cmd_eVideoDecoderStart},
    {"vdStop", TRNSX_Cmd_eVideoDecoderStop},
    {"vdStatus", TRNSX_Cmd_eVideoDecoderStatus},
    /* playpump */
    {"ppSet", TRNSX_Cmd_ePlaypumpSetSettings},
    {"ppOpen", TRNSX_Cmd_ePlaypumpOpen},
    /* record */
    {"recSet", TRNSX_Cmd_eRecordSettings},
    {"recStart", TRNSX_Cmd_eRecordStart},
    {"recStop", TRNSX_Cmd_eRecordStop},
    {"recPause", TRNSX_Cmd_eRecordPause},
    {"recResume", TRNSX_Cmd_eRecordResume},
    /* mux */
    {"muxOpen", TRNSX_Cmd_eMuxOpen},
    {"muxClose", TRNSX_Cmd_eMuxClose},
    {"muxStart", TRNSX_Cmd_eMuxStart},
    {"muxStop", TRNSX_Cmd_eMuxStop},
    /* assorted */
    {"session", TRNSX_Cmd_eSession},
    {"", TRNSX_Cmd_eTranscode},
    {"sleep", TRNSX_Cmd_eSleep},
    {NULL, 0}
};
#endif

typedef struct psi_message_t {
    unsigned short pid;
    NEXUS_MessageHandle message;
    NEXUS_PidChannelHandle pidChannel;
    bool done;
} psi_message_t;

typedef struct TRNSX_Transcode
{
    uint32_t      uiIndex;
    TRNSX_State eState;

    TRNSX_Cmd   eActiveCmd;
    bool        bNonRealTime;
    bool        bEncodeCCData;

    /* Input */
    struct
    {
        TRNSX_Source        eSourceType;

        /* common parameters */
        NEXUS_VideoCodec    eVideoCodec;
        int                 iVideoPid;
        unsigned            uiNumVideo;

        NEXUS_AudioCodec    eAudioCodec;
        int                 iAudioPid;
        unsigned            uiNumAudio;

        int                 iPcrPid;
        unsigned            uiNumPcr;

        bool                bUseStreamAsIndex;

        int                 freq;
        NEXUS_TransportType eStreamType;
        NEXUS_TransportTimestampType transportTimestampType; /* g_TtsInputType in transcode_ts */

        /* file parameters */
        char    fname[TRNSX_SIZE_FILE_NAME];
        bool    bFileIsValid; /* File exists and contains appropriate video data. */

#if  BTST_SUPPORT_FRONTEND
        NEXUS_FrontendQamMode qamMode;
#endif

    } input;

    /* Output */
    struct
    {
        char    fname[TRNSX_SIZE_FILE_NAME];
        char    indexfname[TRNSX_SIZE_FILE_NAME];

        FILE *  hFile;
        FILE *  hIndexFile;
        bool    bGenerateIVF;
        bool    bWaitForEos;        /* wait for EOS when stopping. */

        NEXUS_TransportType     eTransportType;

        bool bCopyData;

        /* For copying data to a file. */

        B_ThreadHandle  hThread;
        B_EventHandle   hEventStopThread;
        B_EventHandle   hEventThreadExited;
        /*B_MutexHandle    mutexStarted;*/

        NEXUS_MemoryBlockHandle   bufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset */
        void *                    pDataBuffer;

    } record;

    /* Video Decoder */
    struct
    {
        TRNSX_State eState;
        NEXUS_VideoDecoderHandle    videoDecoder;

        NEXUS_PidChannelHandle      videoPidChannel;
        NEXUS_PidChannelHandle      pcrPidChannel;

        NEXUS_StcChannelSettings        stcSettings;
        NEXUS_StcChannelHandle          stcChannel;
        NEXUS_VideoDecoderStartSettings startSettings;
    } videoDecoder;

    /* Encoder */
    struct
    {
        TRNSX_State eState;

        NEXUS_VideoWindowHandle         window;
        NEXUS_VideoEncoderHandle        videoEncoder;
        NEXUS_StcChannelSettings        stcSettings;
        NEXUS_StcChannelHandle          stcChannel;

        NEXUS_VideoEncoderSettings      settings;
        NEXUS_VideoEncoderOpenSettings  openSettings;
        NEXUS_VideoEncoderStartSettings startSettings;
        NEXUS_VideoEncoderStopSettings  stopSettings;

        NEXUS_DisplayHandle                 display;
        NEXUS_DisplaySettings               displaySettings;
        bool                                bCustomDisplaySettings;  /* use the following */
        NEXUS_DisplayCustomFormatSettings   customFormatSettings;

    } videoEncoder;

    /* File Mux */
    struct
    {
        bool bSupported;
        bool bActive;
        TRNSX_State eState;

#if NEXUS_HAS_FILE_MUX
        NEXUS_FileMuxHandle fileMux;
        NEXUS_MuxFileHandle muxOutput;
        BKNI_EventHandle    finishEvent;

        NEXUS_FileMuxCreateSettings createSettings;
        NEXUS_FileMuxStartSettings  startSettings;
#endif
    } fileMux;


    /* Stream Mux */
    struct
    {
        bool bSupported;
        bool bActive;
        bool bPrematureMuxStop;

        TRNSX_State eState;

        BKNI_EventHandle           finishEvent;

        NEXUS_TransportTimestampType transportTimestampType; /* g_TtsOutputType in transcode_ts */

        /* how to expose the following to the user?  Add a streammux command? */
        bool        bMCPB;
        bool        bTsUserDataInput;
        unsigned    uiNumDescs;    /* Mux XPT pb descriptors (when MTU BPP is enabled, worst case < A2P * frameRate * 3) */
        size_t      numUserDataPids;
        bool        bAudioInput;

#if NEXUS_HAS_STREAM_MUX
        bool bRemux; /* pContext->bRemux, how is this set?*/

        NEXUS_StreamMuxHandle   streamMux;
        NEXUS_PlaypumpHandle    playpump;

        NEXUS_PidChannelHandle     pidChannelTranscodePcr;
        NEXUS_PidChannelHandle     pidChannelTranscodePat;
        NEXUS_PidChannelHandle     pidChannelTranscodePmt;
        NEXUS_PidChannelHandle     pidRemuxPcr;
        NEXUS_PidChannelHandle     pidRemuxPat;
        NEXUS_PidChannelHandle     pidRemuxPmt;

        NEXUS_StreamMuxCreateSettings   createSettings; /* TODO: should this be local to TRNSX_StreamMux_Open?*/
        NEXUS_StreamMuxStartSettings    startSettings;
        NEXUS_StreamMuxConfiguration    configuration;

        NEXUS_PlaypumpOpenSettings      playpumpOpenSettings;
        NEXUS_PlaypumpSettings          playpumpSettings;

        NEXUS_RecpumpHandle             recpump;
        NEXUS_RecordHandle              record;

        NEXUS_RecordSettings            recordSettings; /* TODO: should this be local to TRNSX_StreamMux_Open?*/

        /* For the output of MUX?  Should this be in the record structure? */
        NEXUS_FileRecordHandle          fileRecord;

        NEXUS_MessageSettings           messageSettings;
        NEXUS_MessageStartSettings      messageStartSettings;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

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
#endif

    } streamMux;

    /* Playback */
    struct
    {
        TRNSX_State             eState;
        NEXUS_FilePlayHandle    hFilePlay;

        NEXUS_PlaypumpHandle        playpump;
        NEXUS_PlaybackHandle        playback;
        NEXUS_PlaybackSettings              playbackSettings;
        NEXUS_PlaybackPidChannelSettings    playbackPidSettings;

    } playback;

    /* Sleep */
    struct
    {
        TRNSX_SleepUnit eUnit;
        uint32_t        uiDuration; /* in milliseconds */
    } sleep;


} TRNSX_Transcode;

typedef struct TRNSX_TestContext
{
    /*NEXUS_VideoDecoderHandle  videoDecoder;*/

    NEXUS_PlatformConfiguration     platformConfig;
    NEXUS_DisplayCapabilities       displayCapabilities;
    NEXUS_VideoEncoderCapabilities  encoderCapabilities;
    NEXUS_VideoDecoderCapabilities  decoderCapabilities;

    bool bInteractive;
    bool bEcho; /* echo commands from the input file */

    uint32_t         uiSelected;
    TRNSX_Transcode  transcodes[TRNSX_NUM_ENCODES];

    struct
    {
        bool bEnabled;
        NEXUS_DisplayHandle       display;
        NEXUS_VideoWindowHandle   window;
    } simulDisplay;


} TRNSX_TestContext;

/*
 * Utilities
 */

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

#if 0
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
#endif
#if 0
static void b_print_media_string(const bmedia_probe_stream *stream)
{
    char stream_info[512];
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    printf( "Media Probe:\n" "%s\n\n", stream_info);
}
#endif
int TRNSX_MediaProbe(
    TRNSX_Transcode * pTrans,
    const char *filename
    )
{
    int rc = 0;

#if 1
    struct probe_request probe_request;
    struct probe_results probe_results;
/*
    char *fname, *ptr;
    char program[20]={'\0',};
*/
    probe_media_get_default_request(&probe_request);
    probe_request.streamname = filename;
    probe_request.program = 0; /* always select program 0, the user can specify different pids later pContext->inputSettings.program;*/
    rc = probe_media_request(&probe_request, &probe_results);

    if (rc) {
        BDBG_ERR(("%s:: probe_media_request failed to parse '%s'", BSTD_FUNCTION, probe_request.streamname));
        return BERR_TRACE(rc);
    }

    if(probe_results.num_audio==0 && probe_results.num_video==0) {
        BDBG_ERR(("%s:: media probe failed to find any audio or video program in '%s'", BSTD_FUNCTION, probe_request.streamname));
        return BERR_TRACE(rc);
    }


    pTrans->input.eVideoCodec = NEXUS_VideoCodec_eUnknown;
    pTrans->input.eAudioCodec = NEXUS_AudioCodec_eUnknown;
    pTrans->input.uiNumVideo = 0;
    pTrans->input.uiNumAudio = 0;
    pTrans->input.uiNumPcr = 0;

    pTrans->input.bUseStreamAsIndex = probe_results.useStreamAsIndex;

    pTrans->input.eStreamType = probe_results.transportType;

    pTrans->input.transportTimestampType = probe_results.timestampType;


    pTrans->input.eVideoCodec = probe_results.video[0].codec;
    pTrans->input.iVideoPid   = probe_results.video[0].pid;
    pTrans->input.uiNumVideo = probe_results.num_video;
    pTrans->input.bFileIsValid = ( probe_results.num_video != 0 ); /*TODO: still needed?*/

    pTrans->input.eAudioCodec = probe_results.audio[0].codec;
    pTrans->input.iAudioPid   = probe_results.audio[0].pid;
    pTrans->input.uiNumAudio = probe_results.num_audio;

    pTrans->input.iPcrPid = probe_results.pcr[0].pid;;
    pTrans->input.uiNumPcr = probe_results.num_pcr;

    strncpy( pTrans->input.fname, filename, TRNSX_SIZE_FILE_NAME-1 );

#if 0
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
#endif

#else


    /* use media probe to set values */
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    FILE *fin;

    pTrans->input.bFileIsValid = false;

    probe = bmedia_probe_create();

    fin = fopen64(filename,"rb");
    if (!fin) {
        printf("%s:: can't open media file '%s' for probing\n", BSTD_FUNCTION,  filename);
        rc = -1;
        goto done;
    }

    fd = bfile_stdio_read_attach(fin);

    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = filename;
    probe_config.type = bstream_mpeg_type_unknown;
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
    bfile_stdio_read_detach(fd);

    fclose(fin);
    if(!stream) {
        printf("%s:: media probe can't parse stream '%s'\n", BSTD_FUNCTION, filename);
        rc = -1;
        goto done;
    }

    BKNI_Printf("input file: %s\n", filename );
    b_print_media_string(stream);

    /*xcodeContext.duration = stream->duration;*/

    pTrans->input.eVideoCodec = NEXUS_VideoCodec_eUnknown;
    pTrans->input.eAudioCodec = NEXUS_AudioCodec_eUnknown;

    pTrans->input.eStreamType = b_mpegtype2nexus(stream->type);

    strncpy( pTrans->input.fname, filename, TRNSX_SIZE_FILE_NAME-1 );


    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
        switch(track->type) {
            case bmedia_track_type_audio:
                if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                    pTrans->input.iAudioPid = track->number;
                    pTrans->input.eAudioCodec = b_audiocodec2nexus(track->info.audio.codec);
                    foundAudio = true;
                }
                break;
            case bmedia_track_type_video:
                if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                    pTrans->input.iVideoPid = track->number;
                    pTrans->input.eVideoCodec = b_videocodec2nexus(track->info.video.codec);
                    pTrans->input.bFileIsValid = true;
                    foundVideo = true;
                }
                break;
            case bmedia_track_type_pcr:
                pTrans->input.iPcrPid = track->number;
                break;
            default:
                break;
        }
    }

done:

    if (probe) {
        if (stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }
#endif

    return rc;
}  /* TRNSX_MediaProbe */


/*
 * Print routines
 */

static void TRNSX_Print_VideoDecoderStatus( NEXUS_VideoDecoderStatus * pstStatus )
{
    BKNI_Printf("  pts =          0x%08x (45 Khz)\n", pstStatus->pts );
    BKNI_Printf("  fifoDepth =    0x%08x\n", pstStatus->fifoDepth );
    BKNI_Printf("  fifoSize =     0x%08x\n", pstStatus->fifoSize);
    BKNI_Printf("  queueDepth =   0x%08x\n", pstStatus->queueDepth );
    BKNI_Printf("  numDecoded =   0x%08x\n", pstStatus->numDecoded );
    BKNI_Printf("  numDisplayed = 0x%08x\n", pstStatus->numDisplayed );
    BKNI_Printf("  numDecodeErrors =    0x%08x\n", pstStatus->numDecodeErrors );
    BKNI_Printf("  numDecodeOverflows = 0x%08x\n", pstStatus->numDecodeOverflows );
    BKNI_Printf("  numDisplayErrors =   0x%08x\n", pstStatus->numDisplayErrors );
    BKNI_Printf("  numDecodeDrops =     0x%08x\n", pstStatus->numDecodeDrops );
    BKNI_Printf("  numPicturesReceived =  0x%08x\n", pstStatus->numPicturesReceived );
    BKNI_Printf("  numDisplayDrops =      0x%08x\n", pstStatus->numDisplayDrops );
    BKNI_Printf("  numDisplayUnderflows = 0x%08x\n", pstStatus->numDisplayUnderflows );
    BKNI_Printf("  ptsErrorCount =        0x%08x\n", pstStatus->ptsErrorCount );
    BKNI_Printf("  avdStatusBlock =       0x%08x\n", pstStatus->avdStatusBlock );
    return;

}

static void TRNSX_Print_VideoEncoderSetSettings(
    NEXUS_VideoEncoderSettings *  pstSettings
    )
{
    BKNI_Printf("  bitrateMax:          %u\n", pstSettings->bitrateMax );
    BKNI_Printf("  bitrateTarget:       %u\n", pstSettings->bitrateTarget );
    BKNI_Printf("  variableFrameRate:   %u\n", pstSettings->variableFrameRate );
    BKNI_Printf("  sparseFrameRate:     %u\n", pstSettings->sparseFrameRate );
    BKNI_Printf("  enableFieldPairing:  %u\n", pstSettings->enableFieldPairing );
    BKNI_Printf("  frameRate:           %s (NEXUS_VideoFrameRate)\n", lookup_name(g_videoFrameRateStrs, pstSettings->frameRate) );
    BKNI_Printf("  streamStructure.newGopOnSceneChange: %u\n", pstSettings->streamStructure.newGopOnSceneChange );
    BKNI_Printf("  streamStructure.duration:            %u\n", pstSettings->streamStructure.duration );
    BKNI_Printf("  streamStructure.adaptiveDuration:    %u\n", pstSettings->streamStructure.adaptiveDuration );
    BKNI_Printf("  streamStructure.framesP:             %u\n", pstSettings->streamStructure.framesP );
    BKNI_Printf("  streamStructure.framesB:             %u\n", pstSettings->streamStructure.framesB );
    BKNI_Printf("  streamStructure.openGop:             %u\n", pstSettings->streamStructure.openGop );
    BKNI_Printf("  encoderDelay:        %u\n", pstSettings->encoderDelay );
    return;

}


static void TRNSX_Print_VideoEncoderOpenSettings(
    NEXUS_VideoEncoderOpenSettings *  pstSettings
    )
{
    BKNI_Printf("  data.fifoSize:           %u\n", pstSettings->data.fifoSize );
    BKNI_Printf("  index.fifoSize:          %u\n", pstSettings->index.fifoSize );
    BKNI_Printf("  memoryConfig.interlaced: %u\n", pstSettings->memoryConfig.interlaced );
    BKNI_Printf("  memoryConfig.maxWidth:   %u\n", pstSettings->memoryConfig.maxWidth );
    BKNI_Printf("  memoryConfig.maxHeight:  %u\n", pstSettings->memoryConfig.maxHeight );
    BKNI_Printf("  type:                    %s (NEXUS_VideoEncoderType)\n", lookup_name( g_encoderTypeStrs, pstSettings->type ) );
    BKNI_Printf("  maxChannelCount:         %u\n", pstSettings->maxChannelCount);
    BKNI_Printf("  enableDataUnitDetecton:  %u\n", pstSettings->enableDataUnitDetecton );
    return;
}


static void TRNSX_Print_VideoEncoderStartSettings(
    NEXUS_VideoEncoderStartSettings *  pstSettings
    )
{
    BKNI_Printf("  interlaced:           %u\n", pstSettings->interlaced);
    BKNI_Printf("  nonRealTime:          %u\n", pstSettings->nonRealTime);
    BKNI_Printf("  lowDelayPipeline:     %u\n", pstSettings->lowDelayPipeline);
    BKNI_Printf("  encodeUserData:       %u\n", pstSettings->encodeUserData);
    BKNI_Printf("  encodeBarUserData:    %u\n", pstSettings->encodeBarUserData);
    BKNI_Printf("  adaptiveLowDelayMode: %u\n", pstSettings->adaptiveLowDelayMode);
    BKNI_Printf("  codec:                %s (NEXUS_VideoCodec)\n",  lookup_name(g_videoCodecStrs, pstSettings->codec) );
    BKNI_Printf("  profile:              %s (NEXUS_VideoCodecProfile)\n",  lookup_name(g_videoCodecProfileStrs, pstSettings->profile ) );
    BKNI_Printf("  level:                %s (NEXUS_VideoCodecLevel)\n", lookup_name( g_videoCodecLevelStrs, pstSettings->level ));

    BKNI_Printf("  bounds.outputFrameRate.min:                 %s (NEXUS_VideoFrameRate)\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.outputFrameRate.min));
    BKNI_Printf("  bounds.outputFrameRate.max:                 %s (NEXUS_VideoFrameRate)\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.outputFrameRate.max));
    BKNI_Printf("  bounds.inputFrameRate.min:                  %s (NEXUS_VideoFrameRate)\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.inputFrameRate.min));
    BKNI_Printf("  bounds.inputDimension.max.width:            %u\n", pstSettings->bounds.inputDimension.max.width);
    BKNI_Printf("  bounds.inputDimension.max.height:           %u\n", pstSettings->bounds.inputDimension.max.height);
    BKNI_Printf("  bounds.inputDimension.maxInterlaced.width:  %u\n", pstSettings->bounds.inputDimension.maxInterlaced.width);
    BKNI_Printf("  bounds.inputDimension.maxInterlaced.height: %u\n", pstSettings->bounds.inputDimension.maxInterlaced.height);
    BKNI_Printf("  bounds.bitrate.upper.bitrateMax:            %u\n", pstSettings->bounds.bitrate.upper.bitrateMax);
    BKNI_Printf("  bounds.bitrate.upper.bitrateTarget:         %u\n", pstSettings->bounds.bitrate.upper.bitrateTarget);
    BKNI_Printf("  bounds.streamStructure.max.framesP:         %u\n", pstSettings->bounds.streamStructure.max.framesP);
    BKNI_Printf("  bounds.streamStructure.max.framesB:         %u\n", pstSettings->bounds.streamStructure.max.framesB);

    BKNI_Printf("  rateBufferDelay:                            %u\n", pstSettings->rateBufferDelay);
    BKNI_Printf("  numParallelEncodes:                         %u\n", pstSettings->numParallelEncodes);
    BKNI_Printf("  bypassVideoProcessing:                      %u\n", pstSettings->bypassVideoProcessing);
    BKNI_Printf("  entropyCoding:                              %s (NEXUS_EntropyCoding)\n", lookup_name( g_entropyCodingStrs, pstSettings->entropyCoding));
    BKNI_Printf("  hrdModeRateControl.disableFrameDrop:        %u\n", pstSettings->hrdModeRateControl.disableFrameDrop);
    BKNI_Printf("  segmentModeRateControl.enable:              %u\n", pstSettings->segmentModeRateControl.enable);
    BKNI_Printf("  segmentModeRateControl.duration:            %u\n", pstSettings->segmentModeRateControl.duration);
    BKNI_Printf("  segmentModeRateControl.upperTolerance:      %u\n", pstSettings->segmentModeRateControl.upperTolerance);
    BKNI_Printf("  segmentModeRateControl.lowerTolerance:      %u\n", pstSettings->segmentModeRateControl.lowerTolerance);
    BKNI_Printf("  memoryBandwidthSaving.singleRefP:           %u\n", pstSettings->memoryBandwidthSaving.singleRefP);
    BKNI_Printf("  memoryBandwidthSaving.requiredPatchesOnly:  %u\n", pstSettings->memoryBandwidthSaving.requiredPatchesOnly);
    return;

}

static void TRNSX_Print_VideoEncoderStopSettings( NEXUS_VideoEncoderStopSettings *  pstSettings )
{
    BKNI_Printf("  mode: %s (NEXUS_VideoEncoderStopMode)\n", lookup_name(g_encoderStopModeStrs, pstSettings->mode));
    return;
}

static void TRNSX_Print_VideoEncoderStatus( NEXUS_VideoEncoderStatus * pstStatus )
{
    BKNI_Printf("  errorFlags = 0x%08x\n", pstStatus->errorFlags );
    BKNI_Printf("  eventFlags = 0x%08x\n", pstStatus->eventFlags );
    BKNI_Printf("  errorCount = %u\n", pstStatus->errorCount);
    BKNI_Printf("  picturesReceived = %u\n", pstStatus->picturesReceived );
    BKNI_Printf("  picturesDroppedFRC = %u\n", pstStatus->picturesDroppedFRC );
    BKNI_Printf("  picturesDroppedHRD = %u\n", pstStatus->picturesDroppedHRD );
    BKNI_Printf("  picturesDroppedErrors = %u\n", pstStatus->picturesDroppedErrors );
    BKNI_Printf("  picturesEncoded = %u\n", pstStatus->picturesEncoded );
    BKNI_Printf("  pictureIdLastEncoded = 0x%08x\n", pstStatus->pictureIdLastEncoded );
    BKNI_Printf("  picturesPerSecond = %u\n", pstStatus->picturesPerSecond );
    BKNI_Printf("  data.fifoDepth  = 0x%08x\n", (uint32_t)pstStatus->data.fifoDepth );
    BKNI_Printf("  data.fifoSize   = 0x%08x\n", (uint32_t)pstStatus->data.fifoSize );
    BKNI_Printf("  index.fifoDepth = 0x%08x\n", (uint32_t)pstStatus->index.fifoDepth );
    BKNI_Printf("  index.fifoSize  = 0x%08x\n", (uint32_t)pstStatus->index.fifoSize  );
    return;
}

static void TRNSX_Print_DisplaySettings(
    NEXUS_DisplaySettings * pstSettings
    )
{
    BKNI_Printf("  displayType:         %s (NEXUS_DisplayType)\n", lookup_name(g_displayTypeStrs, pstSettings->displayType));
    BKNI_Printf("  timingGenerator:     %s (NEXUS_DisplayTimingGenerator)\n", lookup_name(g_displayTimingGeneratorStrs, pstSettings->timingGenerator));
    BKNI_Printf("  format:              %s (NEXUS_VideoFormat)\n", lookup_name( g_videoFormatStrs, pstSettings->format));
    BKNI_Printf("  aspectRatio:         %s (NEXUS_DisplayAspectRatio)\n", lookup_name( g_displayAspectRatioStrs, pstSettings->aspectRatio));
    BKNI_Printf("  sampleAspectRatio.x: %u\n", pstSettings->sampleAspectRatio.x);
    BKNI_Printf("  sampleAspectRatio.y: %u\n", pstSettings->sampleAspectRatio.y);
    BKNI_Printf("  dropFrame:           %s (NEXUS_TristateEnable)\n", lookup_name( g_tristateEnableStrs, pstSettings->dropFrame));
    return;
}

static void TRNSX_Print_DisplayCustomFormatSettings(
    TRNSX_Transcode * pTrans

    )
{
    NEXUS_DisplayCustomFormatSettings * pstSettings = &pTrans->videoEncoder.customFormatSettings;

    BKNI_Printf("  bcustom:             %d\n", pTrans->videoEncoder.bCustomDisplaySettings );
    BKNI_Printf("  width:               %u\n", pstSettings->width);
    BKNI_Printf("  height:              %u\n", pstSettings->height);
    BKNI_Printf("  refreshRate:         %u (in 1/1000th Hz)\n", pstSettings->refreshRate);
    BKNI_Printf("  interlaced:          %u\n", pstSettings->interlaced);
    BKNI_Printf("  aspectRatio:         %s (NEXUS_DisplayAspectRatio)\n", lookup_name( g_displayAspectRatioStrs, pstSettings->aspectRatio));
    BKNI_Printf("  sampleAspectRatio.x: %u\n", pstSettings->sampleAspectRatio.x);
    BKNI_Printf("  sampleAspectRatio.y: %u\n", pstSettings->sampleAspectRatio.y);
    BKNI_Printf("  dropFrameAllowed:    %u\n", pstSettings->dropFrameAllowed);
    return;
}

static void TRNSX_Print_MuxSettings( TRNSX_Transcode * pTrans )
{
    BKNI_Printf("  transport:  %s (NEXUS_TransportType)\n", lookup_name(g_transportTypeStrs, pTrans->record.eTransportType));
    BKNI_Printf("  ivf:        %d\n", pTrans->record.bGenerateIVF);
    return;
}

static void TRNSX_Print_StreamMuxStartSettings( NEXUS_StreamMuxStartSettings * pstSettings )
{
    BKNI_Printf("  transportType    %s (NEXUS_TransportType)\n", lookup_name( g_transportTypeStrs, pstSettings->transportType ));
    /*BKNI_Printf("  supportTts       %s (NEXUS_TransportTimestampType)\n", lookup_name( g_transportTimestampTypeStrs, pstSettings->supportTts ));*/
    BKNI_Printf("  nonRealTimeRate  %u\n", pstSettings->nonRealTimeRate);
    BKNI_Printf("  servicePeriod    %u\n", pstSettings->servicePeriod);
    BKNI_Printf("  latencyTolerance %u\n", pstSettings->latencyTolerance);
    BKNI_Printf("  interleaveMode   %s (NEXUS_StreamMuxInterleaveMode - escr or pts)\n", lookup_name( g_streamMuxInterleaveModeStrs, pstSettings->interleaveMode ));
    BKNI_Printf("  useInitialPts    %u\n", pstSettings->useInitialPts);
    BKNI_Printf("  initialPts       %u\n", pstSettings->initialPts);
    BKNI_Printf("  pcr.interval     %u\n", pstSettings->pcr.interval);
    BKNI_Printf("  insertPtsOnlyOnFirstKeyFrameOfSegment %u\n", pstSettings->insertPtsOnlyOnFirstKeyFrameOfSegment);
    return;
}

static void TRNSX_Print_RecordSettings( TRNSX_Transcode * pTrans )
{
    BKNI_Printf("  file:       %s\n", pTrans->record.fname);
    BKNI_Printf("  index file: %s\n", pTrans->record.indexfname);
    return;
}

static void TRNSX_Print_PlaybackOpenSettings( TRNSX_Transcode * pTrans )
{
    BKNI_Printf("  type:         %s (file/hdmi/qam)\n", lookup_name(g_sourceStrs, pTrans->input.eSourceType));
    BKNI_Printf("  file:         %s\n", pTrans->input.fname);
    BKNI_Printf("  video_codec:  %s (NEXUS_VideoCodec)\n", lookup_name( g_videoCodecStrs, pTrans->input.eVideoCodec ));
    BKNI_Printf("  video_pid:    %x (%d)\n", pTrans->input.iVideoPid, pTrans->input.iVideoPid );
    BKNI_Printf("  pcr_pid:      %x (%d)\n", pTrans->input.iPcrPid, pTrans->input.iPcrPid );
    BKNI_Printf("  transport:    %s (NEXUS_TransportType)\n", lookup_name(g_transportTypeStrs,  pTrans->input.eStreamType));
    return;
}

static void TRNSX_Print_PlaypumpOpenSettings( NEXUS_PlaypumpOpenSettings * pstSettings )
{
    BKNI_Printf("  fifoSize:            0x%08x\n", (uint32_t)pstSettings->fifoSize );
    BKNI_Printf("  numDescriptors:      %u\n", pstSettings->numDescriptors );
    BKNI_Printf("  streamMuxCompatible: %d\n", pstSettings->streamMuxCompatible );
    return;
}

static void TRNSX_Print_SessionSettings( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans )
{
    BKNI_Printf("  select:    %d\n", pCtxt->uiSelected );
    BKNI_Printf("  -- general settings --\n");
    BKNI_Printf("  nrt:       %d\n", pTrans->bNonRealTime );
    BKNI_Printf("  cc:        %d\n", pTrans->bEncodeCCData );
    BKNI_Printf("  -- stream mux settings --\n");
    BKNI_Printf("  tts_out: %s (NEXUS_TransportTimestampType)\n", lookup_name( g_transportTimestampTypeStrs, pTrans->streamMux.transportTimestampType ));
    return;
}

/*
 * Platfrom routines.
 */
static void TRNSX_Platform_SystemInfo(
    TRNSX_TestContext *  pCtxt
    )
{
    int32_t i;
#if  NEXUS_HAS_FILE_MUX
    BKNI_Printf("file mux IS supported\n");
#else
    BKNI_Printf("file mux is NOT supported\n");
#endif

#if NEXUS_HAS_STREAM_MUX
    BKNI_Printf("stream mux IS supported\n");
#else
    BKNI_Printf("stream mux is NOT supported\n");
#endif

    /* encoder info */
    BKNI_Printf("encoder\n");
    for ( i=0; i<NEXUS_MAX_VIDEO_ENCODERS; i++ )
    {
        BKNI_Printf("  encoder:%d supported:%d displayIndex:%d deviceIndex:%d channelIndex:%d\n",
                        i,
                        pCtxt->encoderCapabilities.videoEncoder[i].supported,
                        pCtxt->encoderCapabilities.videoEncoder[i].displayIndex,
                        pCtxt->encoderCapabilities.videoEncoder[i].deviceIndex,
                        pCtxt->encoderCapabilities.videoEncoder[i].channelIndex );
    }

    /* video decoder info */
    BKNI_Printf("HVD based decoders: %d\n", pCtxt->decoderCapabilities.numVideoDecoders );

    for ( i=0; i < NEXUS_MAX_VIDEO_DECODERS; i++ )
    {
        BKNI_Printf("  decoder:%d avdIndex:%d colorDepth:%d MFD_index:%d\n",
                        i,
                        pCtxt->decoderCapabilities.videoDecoder[i].avdIndex,
                        pCtxt->decoderCapabilities.videoDecoder[i].feeder.colorDepth,
                        pCtxt->decoderCapabilities.videoDecoder[i].feeder.index
                        );
    }

    BKNI_Printf("  numStcs:%d\n", pCtxt->decoderCapabilities.numStcs );
    BKNI_Printf("  DSP decoders: %d useForVp6: %d\n",
                        pCtxt->decoderCapabilities.dspVideoDecoder.total,
                        pCtxt->decoderCapabilities.dspVideoDecoder.useForVp6
                        );
    BKNI_Printf("  SID decoders: %d useForMotionJpeg: %d\n",
                        pCtxt->decoderCapabilities.sidVideoDecoder.total,
                        pCtxt->decoderCapabilities.sidVideoDecoder.useForMotionJpeg
                        );
    BKNI_Printf("  CPU decoders: %d useForVp9: %d\n",
                        pCtxt->decoderCapabilities.softVideoDecoder.total,
                        pCtxt->decoderCapabilities.softVideoDecoder.useForVp9
                        );

    /*display info */
    BKNI_Printf("display\n");
    for ( i=0; i < NEXUS_MAX_DISPLAYS; i++ )
    {
        BKNI_Printf("  display: %d numVideoWindows: %d\n", i, pCtxt->displayCapabilities.display[i].numVideoWindows);
    }

    BKNI_Printf("playpump: NEXUS_NUM_PLAYPUMPS: %d\n", NEXUS_NUM_PLAYPUMPS );
    /*BKNI_Printf("recpump:  BXPT_NUM_RAVE_CONTEXTS: %d\n", BXPT_NUM_RAVE_CONTEXTS );*/

    /* TODO: print the following? */
#if 0
typedef struct NEXUS_DisplayCapabilities
{
    bool displayFormatSupported[NEXUS_VideoFormat_eMax]; /* is NEXUS_DisplaySettings.format supported by any display in the system? */
    unsigned numLetterBoxDetect; /* see NEXUS_VideoWindowSettings.letterBoxDetect */
} NEXUS_DisplayCapabilities;
#endif

    return;

}


static NEXUS_Error TRNSX_Platform_Open(
    TRNSX_TestContext *  pCtxt
    )
{
    NEXUS_PlatformSettings platformSettings;
#if SIMUL_DISPLAY
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc=NEXUS_SUCCESS;

    B_Os_Init();

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

#if x /* TODO: from transcode_ts, also look at the code which retrieves capablities. */
    /* enable frontend if the 1st xcoder needs it for now; not always enable to avoid slow frontend init for other cases;
       TODO: init frontend in case 1st disable but 2nd enables frontend. */
    platformSettings.openFrontend = (pContext->inputSettings.resource == BTST_RESOURCE_QAM);
    /* audio PI supports 4 by default; we need one extra mixers for each transcoders; */
    platformSettings.audioModuleSettings.dspAlgorithmSettings.typeSettings[NEXUS_AudioDspAlgorithmType_eAudioEncode].count = NEXUS_NUM_VIDEO_ENCODERS;
    platformSettings.audioModuleSettings.maxAudioDspTasks += NEXUS_NUM_VIDEO_ENCODERS + 1;/* to support quad xcodes + loopback decode */
    platformSettings.audioModuleSettings.numCompressedBuffers += NEXUS_NUM_VIDEO_ENCODERS;/* for quad xcodes */
    platformSettings.audioModuleSettings.numPcmBuffers = NEXUS_NUM_VIDEO_ENCODERS + 1;/* to support quad xcodes and loopback decode */
    NEXUS_Platform_Init(&platformSettings);
#endif

    NEXUS_Platform_GetConfiguration(&pCtxt->platformConfig);

    NEXUS_GetVideoEncoderCapabilities(&pCtxt->encoderCapabilities);
    NEXUS_GetDisplayCapabilities(&pCtxt->displayCapabilities);
    NEXUS_GetVideoDecoderCapabilities(&pCtxt->decoderCapabilities);


#if SIMUL_DISPLAY
    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
    pCtxt->simulDisplay.display = NEXUS_Display_Open(0, &displaySettings);

    pCtxt->simulDisplay.window = NEXUS_VideoWindow_Open(pCtxt->simulDisplay.display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    if(pCtxt->platformConfig.outputs.component[0]){
         NEXUS_Display_AddOutput(pCtxt->simulDisplay.display, NEXUS_ComponentOutput_GetConnector(pCtxt->platformConfig.outputs.component[0]));
    }
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    if(pCtxt->platformConfig.outputs.hdmi[0]){
         NEXUS_Display_AddOutput(pCtxt->simulDisplay.display, NEXUS_HdmiOutput_GetVideoConnector(pCtxt->platformConfig.outputs.hdmi[0]));
    }
#endif
#endif

    return rc;

}

static NEXUS_Error TRNSX_Platform_Close(
    TRNSX_TestContext *  pCtxt
    )
{
#if SIMUL_DISPLAY
    NEXUS_VideoWindow_Close(pCtxt->simulDisplay.window);
    NEXUS_Display_Close(pCtxt->simulDisplay.display);
#else
    BSTD_UNUSED(pCtxt);
#endif
    NEXUS_Platform_Uninit();

    return NEXUS_SUCCESS;
}

/*
 * Callbacks and thread routines.
 */

static void
TRNSX_Callback_TranscoderFinished(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BKNI_Printf("%s callback invoked, now stop the mux.\n", BSTD_FUNCTION );
    BKNI_SetEvent(finishEvent);
}


static void TRNSX_Record_BufferCopyThread(
    void *context
    )
{
    size_t bytes;
    bool   bWaitingForEOS=true; /* by default wait for NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS */
    bool   bStoppedSignaled=false;

    TRNSX_Transcode * pTrans = (TRNSX_Transcode *)context;

    for(bytes=0;;)
    {
        size_t size[2];
        const NEXUS_VideoEncoderDescriptor *desc[2];
        unsigned i,j;
        unsigned uiNumProcessed;
        unsigned uiShutdownTries=0;

       /* To allow the user to pause the thread. */
        while ( !pTrans->record.bCopyData )
        {
            BKNI_Sleep(100);
        }

        NEXUS_VideoEncoder_GetBuffer(pTrans->videoEncoder.videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);

        if(size[0]==0 && size[1]==0)
        {
            if ( pTrans->record.hFile )
                fflush(pTrans->record.hFile);

            if ( pTrans->record.hIndexFile )
                fflush(pTrans->record.hIndexFile);

            if(( B_Event_Wait(pTrans->record.hEventStopThread, B_WAIT_NONE)==B_ERROR_SUCCESS ) && ( bStoppedSignaled == false ) )
            {
                bStoppedSignaled = true;

                /* Wait for the EOS if:
                 * - the stop mode is NOT "abort": if it is "abort", the EOS might not come.
                 * - AND the encoder is stopped
                 * - AND the user has said to wait (which is the default) */

                bWaitingForEOS &= (pTrans->videoEncoder.stopSettings.mode != NEXUS_VideoEncoderStopMode_eAbort);
                bWaitingForEOS &= (pTrans->videoEncoder.eState == TRNSX_State_eStopped);
                bWaitingForEOS &= (pTrans->record.bWaitForEos == true);

                BDBG_MSG(("%s: stopped signaled. bWaitingForEOS:%d encoder_stop_mode:%s encoder_state:%s bWaitForEos:%d",
                                    BSTD_FUNCTION,
                                    bWaitingForEOS,
                                    lookup_name(g_encoderStopModeStrs, pTrans->videoEncoder.stopSettings.mode),
                                    lookup_name(g_stateStrs, pTrans->videoEncoder.eState),
                                    pTrans->record.bWaitForEos ));
            }

            if ( bStoppedSignaled == true )
            {
                if (( bWaitingForEOS == false ) || ( uiShutdownTries > 100 ))
                {
                    break;
                }
                else
                {
                    /* A crude watchdog in case something goes off the rails. */
                    uiShutdownTries++;
                }
            }

            BKNI_Sleep(30);

            continue;
        }

        for(uiNumProcessed=0,j=0;j<2;j++)
        {
            uiNumProcessed+=size[j];
            for(i=0;i<size[j];i++)
            {
                const NEXUS_VideoEncoderDescriptor *pDesc = &desc[j][i];

                if ( pDesc->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS  )
                {
                    bWaitingForEOS = false;
                    BDBG_MSG(("%s: NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS received", BSTD_FUNCTION ));
                }

                if(pDesc->length > 0)
                {
                    /* ignore metadata descriptor in es capture */
                    if( pTrans->record.hFile && (pDesc->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0)
                    {
                        fwrite((const uint8_t *)pTrans->record.pDataBuffer + pDesc->offset, pDesc->length, 1, pTrans->record.hFile);
                    }

                    if ( pTrans->record.hIndexFile )
                    {
                        fprintf(pTrans->record.hIndexFile, "%8x %8x   %x%08x %08x     %5u   %5d   %8x "BDBG_UINT64_FMT"\n",
                                    pDesc->flags,
                                    pDesc->originalPts,
                                    (uint32_t)(pDesc->pts>>32),
                                    (uint32_t)(pDesc->pts & 0xffffffff),
                                    pDesc->escr,
                                    pDesc->ticksPerBit,
                                    pDesc->shr,
                                    pDesc->offset,
                                    BDBG_UINT64_ARG((uint64_t)pDesc->length));
                    }
                    bytes+= pDesc->length;
#if 0
                    if(pDesc->length > 0x100000)
                    {
                        BDBG_ERR(("++++ desc[%d][%d] length = "BDBG_UINT64_FMT", offset=0x%x", j,i, BDBG_UINT64_ARG((uint64_t)pDesc->length), pDesc->offset));
                    }
#endif
                }
            }
        }

        NEXUS_VideoEncoder_ReadComplete(pTrans->videoEncoder.videoEncoder, uiNumProcessed);

    }

    B_Event_Set(pTrans->record.hEventThreadExited);

    return;

}

/*
 * Transcode routines.
 */


static NEXUS_Error TRNSX_Transcode_GetDefaultStreamMuxSettings(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BSTD_UNUSED(pCtxt);

#if NEXUS_HAS_STREAM_MUX

    pTrans->streamMux.bSupported = true;

    /* TODO: use the session command to set the following? */

    pTrans->streamMux.bMCPB = true;     /* Default to multi channel playback */
    pTrans->streamMux.uiNumDescs = 512;
    pTrans->streamMux.bTsUserDataInput = false;
    pTrans->streamMux.bAudioInput = false;
    pTrans->streamMux.transportTimestampType = NEXUS_TransportTimestampType_eNone; /* g_TtsOutputType in transcode_ts */

    /* playpump open settings*/

    NEXUS_Playpump_GetDefaultOpenSettings( &pTrans->streamMux.playpumpOpenSettings );
    pTrans->streamMux.playpumpOpenSettings.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    pTrans->streamMux.playpumpOpenSettings.numDescriptors = 512; /* set number of descriptors */
    pTrans->streamMux.playpumpOpenSettings.streamMuxCompatible = true;

    /* playpump set settings*/

    NEXUS_Playpump_GetDefaultSettings( &pTrans->streamMux.playpumpSettings);
    pTrans->streamMux.playpumpSettings.transportType = NEXUS_TransportType_eEs;

    /* stream mux memory configuration */

    NEXUS_StreamMux_GetDefaultConfiguration(&pTrans->streamMux.configuration);

    pTrans->streamMux.configuration.userDataPids        = 0;    /* TODO: is this set by the user? */
    pTrans->streamMux.configuration.audioPids           = 0;    /* no audio support for now pTrans->streamMux.bAudioInput ? 1 : 0; */
    pTrans->streamMux.configuration.latencyTolerance    = 20;   /* g_muxLatencyTolerance =  20ms sw latency */
    pTrans->streamMux.configuration.servicePeriod       = 50;   /* g_msp = 50ms Mux Serivce Period */

    /* set pCtxt->encodeDelay; A2P delay affects mux resource allocation */
    /* TODO set after the video encoder starts?  Use pTrans->videoEncoder.settings.encoderDelay? */
    pTrans->streamMux.configuration.muxDelay            = 0;

    /* TODO: expose the following to the user or just use the session settings? */
#if 0
    pTrans->streamMux.configuration.nonRealTime = pTrans->bNonRealTime;
    pTrans->streamMux.configuration.supportTts = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);
#endif

    /* stream mux start settings */

    NEXUS_StreamMux_GetDefaultStartSettings(&pTrans->streamMux.startSettings);
    pTrans->streamMux.startSettings.transportType       = NEXUS_TransportType_eTs; /* TODO: make user selectable? */
    pTrans->streamMux.startSettings.nonRealTimeRate     = 8 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
    pTrans->streamMux.startSettings.servicePeriod       = 50;   /* g_msp = 50ms Mux Serivce Period */
    pTrans->streamMux.startSettings.latencyTolerance    = 20;   /* g_muxLatencyTolerance mux service latency tolerance */
    pTrans->streamMux.startSettings.interleaveMode      = NEXUS_StreamMuxInterleaveMode_eCompliant; /*is this the correct default? pCtxt->interleaveMode;*/
    pTrans->streamMux.startSettings.useInitialPts       = false;
    pTrans->streamMux.startSettings.initialPts          = 0;
    pTrans->streamMux.startSettings.pcr.interval        = 50; /* 50 msecs */
    pTrans->streamMux.startSettings.insertPtsOnlyOnFirstKeyFrameOfSegment = false; /*g_bOnePtsPerSegment;*/

#else
    BSTD_UNUSED(pTrans);
#endif

    return NEXUS_SUCCESS;
}


static NEXUS_Error TRNSX_Transcode_GetDefaultSettings(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BSTD_UNUSED(pCtxt);
    BKNI_Memset( pTrans, 0, sizeof( TRNSX_Transcode ));

    pTrans->input.eSourceType = TRNSX_Source_eFile; /* just support files for now. */

    /* TODO: how to initialize the following to reasonable values for each platform?
     * Is this the correct place to make these calls?
     * Are these calls returning the values for current box mode? */
    NEXUS_VideoEncoder_GetDefaultSettings(&pTrans->videoEncoder.settings);
    NEXUS_VideoEncoder_GetDefaultOpenSettings(&pTrans->videoEncoder.openSettings);
    NEXUS_VideoEncoder_GetDefaultStartSettings(&pTrans->videoEncoder.startSettings);
    NEXUS_VideoEncoder_GetDefaultStopSettings(&pTrans->videoEncoder.stopSettings);

    /* Display */
    NEXUS_Display_GetDefaultSettings(&pTrans->videoEncoder.displaySettings);
    pTrans->videoEncoder.displaySettings.displayType = NEXUS_DisplayType_eAuto;
    pTrans->videoEncoder.displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    pTrans->videoEncoder.displaySettings.format = NEXUS_VideoFormat_eNtsc;

    pTrans->videoEncoder.bCustomDisplaySettings = false;
    NEXUS_Display_GetDefaultCustomFormatSettings(&pTrans->videoEncoder.customFormatSettings);

    /* STC channel: decoder */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &pTrans->videoDecoder.stcSettings);
    pTrans->videoDecoder.stcSettings.timebase = NEXUS_Timebase_e0;
    pTrans->videoDecoder.stcSettings.mode = NEXUS_StcChannelMode_eAuto;

    /* STC channel: encoder requires different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &pTrans->videoEncoder.stcSettings);
    pTrans->videoEncoder.stcSettings.timebase    = NEXUS_Timebase_e0; /* TODO: if multiple encodes, can they all be _e0 or should it be +pTrans->uiIndex? */
    pTrans->videoEncoder.stcSettings.mode        = NEXUS_StcChannelMode_eAuto;
    pTrans->videoEncoder.stcSettings.pcrBits     = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    pTrans->videoEncoder.stcSettings.autoConfigTimebase = false; /* from transcode_ts */

    /* Decoder */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&pTrans->videoDecoder.startSettings);

    /* Record */
    pTrans->record.eTransportType = NEXUS_TransportType_eEs;
    pTrans->record.bWaitForEos = true;

    /* File Mux */
#if  NEXUS_HAS_FILE_MUX
    pTrans->fileMux.bSupported = true;
#endif

    TRNSX_Transcode_GetDefaultStreamMuxSettings( pCtxt, pTrans );

    return NEXUS_SUCCESS;
}

static void TRNSX_ResultsFile_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BDBG_ASSERT(pTrans);
    BSTD_UNUSED(pCtxt);

    /* Close any existing files, is this really needed? */

    if ( pTrans->record.hFile )
    {
        fclose( pTrans->record.hFile );
        pTrans->record.hFile = NULL;
    }

    if ( pTrans->record.hIndexFile )
    {
        fclose( pTrans->record.hIndexFile );
        pTrans->record.hIndexFile = NULL;
    }

    /* If the user provided a file name, open it now. */
    if ( strlen( pTrans->record.fname ) != 0 )
    {
        pTrans->record.hFile = fopen(pTrans->record.fname,"wb");
        if ( pTrans->record.hFile ) {
            BKNI_Printf("%s: opened results file: %s\n", BSTD_FUNCTION, pTrans->record.fname );
        }
        else {
            BKNI_Printf("%s: failed to open results file: %s\n", BSTD_FUNCTION, pTrans->record.fname );
            BDBG_ASSERT(pTrans->record.hFile);
        }

        pTrans->record.hIndexFile = fopen(pTrans->record.indexfname, "w");

        if ( pTrans->record.hIndexFile ) {
            BKNI_Printf("%s: opened index file: %s\n", BSTD_FUNCTION, pTrans->record.indexfname );
        }
        else {
            BKNI_Printf("%s: failed to open index file: %s\n", BSTD_FUNCTION, pTrans->record.indexfname );
            BDBG_ASSERT(pTrans->record.hIndexFile);
        }
    }

    return;

}

static void TRNSX_ResultsFile_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BDBG_ASSERT(pTrans);
    BSTD_UNUSED(pCtxt);

    if ( pTrans->record.hFile )
    {
        fclose(pTrans->record.hFile);
        pTrans->record.hFile = NULL;
    }


    if ( pTrans->record.hIndexFile )
    {
        fclose(pTrans->record.hIndexFile);
        pTrans->record.hIndexFile = NULL;
    }

    return;
}


/*
 * Wrapper routines to provide error checking and handle sequencing issues.
 */

/*
 * ES file wrappers
 */
static NEXUS_Error TRNSX_FileEs_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_VideoEncoderStatus videoEncoderStatus;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* Open the results files.  Will get an assert if the file can't be opened. */
    TRNSX_ResultsFile_Open( pCtxt, pTrans );

    /* Code to create an es file.
     * TODO: what error checking is need for failure to lock memory or create thread? */

    NEXUS_VideoEncoder_GetStatus(pTrans->videoEncoder.videoEncoder, &videoEncoderStatus);
    BDBG_ASSERT(videoEncoderStatus.bufferBlock);
    pTrans->record.bufferBlock = videoEncoderStatus.bufferBlock;
    NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pTrans->record.pDataBuffer);

    pTrans->record.hThread = B_Thread_Create("TRNSX Record ES", (B_ThreadFunc)TRNSX_Record_BufferCopyThread, pTrans, NULL);

    pTrans->record.hEventStopThread = B_Event_Create(NULL);
    pTrans->record.hEventThreadExited = B_Event_Create(NULL);

    B_Event_Reset( pTrans->record.hEventStopThread );
    B_Event_Reset( pTrans->record.hEventThreadExited );
    pTrans->record.bCopyData = true;

    return rc;
}

static NEXUS_Error TRNSX_FileEs_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    pTrans->record.bCopyData = true; /* want to be certain the thread isn't paused.*/

    /* Wait for the copy thread to exit. */

    B_Event_Set( pTrans->record.hEventStopThread );

    B_Event_Wait(pTrans->record.hEventThreadExited, B_WAIT_FOREVER);

    if ( pTrans->record.hThread )
    {
        B_Thread_Destroy( pTrans->record.hThread );
        pTrans->record.hThread = NULL;
    }

    if ( pTrans->record.hEventStopThread )
    {
        B_Event_Destroy( pTrans->record.hEventStopThread );
        pTrans->record.hEventStopThread = NULL;
    }

    if ( pTrans->record.hEventThreadExited )
    {
        B_Event_Destroy( pTrans->record.hEventThreadExited );
        pTrans->record.hEventThreadExited = NULL;
    }

    if ( pTrans->record.bufferBlock )
    {
        NEXUS_MemoryBlock_Unlock(pTrans->record.bufferBlock);
        pTrans->record.bufferBlock = NULL;
    }

    TRNSX_ResultsFile_Close( pCtxt, pTrans );

    return rc;

}

/*
 * Stream mux wrappers (ported from transcode_ts.c)
 */

/*******************************
 * Set up stream_mux and record
 */


#if 1
/*******************************
 * Add system data to stream_mux
 */
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189

static void TRNSX_StreamMux_RecpumpOverflowCallback(void *context, int param)
{
    TRNSX_Transcode * pTrans = context;
    BSTD_UNUSED(param);
    BDBG_ERR(("\n#### %s Context::%d stream mux recpump buffer overflows! ###\n", BSTD_FUNCTION, pTrans->uiIndex));
}


static void TRNSX_StreamMux_SystemData_InsertTimer(void *context)
{
    TRNSX_Transcode * pTrans = context;
    uint8_t ccByte;

    ++pTrans->streamMux.ccValue;/* increment CC synchronously with PAT/PMT */
    ccByte = *((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (pTrans->streamMux.ccValue & 0xf);
    *((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PAT pointer */
    pTrans->streamMux.psi[0].pData = (void*)((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 1);

    ccByte = *((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4);
    ccByte = (ccByte & 0xf0) | (pTrans->streamMux.ccValue & 0xf);
    *((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4) = ccByte;
    NEXUS_Memory_FlushCache((void*)((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4), 1);
    /* ping pong PMT pointer */
    pTrans->streamMux.psi[1].pData = (void*)((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 1);

    NEXUS_StreamMux_AddSystemDataBuffer(pTrans->streamMux.streamMux, &pTrans->streamMux.psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(pTrans->streamMux.streamMux, &pTrans->streamMux.psi[1]);
    BDBG_MODULE_MSG(contcounter,("Context%d insert PAT&PMT... ccPAT = %x ccPMT=%x", pTrans->uiIndex, *((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4) & 0xf,
        *((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue  % BTST_PSI_QUEUE_CNT] + 4) & 0xf));
    if(pTrans->streamMux.systemdataTimerIsStarted)
    {
        pTrans->streamMux.systemdataTimer = B_Scheduler_StartTimer(
            pTrans->streamMux.schedulerSystemdata,pTrans->streamMux.mutexSystemdata, 1000, TRNSX_StreamMux_SystemData_InsertTimer, pTrans);
        if(pTrans->streamMux.systemdataTimer==NULL) {BDBG_ERR(("schedule timer error %d", NEXUS_OUT_OF_SYSTEM_MEMORY));}
    }
    return;
}

void TRNSX_StreamMux_SystemData_AddPatPmt(
    /* TranscodeContext  *pContext,*/
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans,
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

    BSTD_UNUSED(pCtxt);

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
#if 0
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
#endif
    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MSG(("\nContext%d output PMT section:", pTrans->uiIndex));
    for(i=0; i < (int)pmtState.size; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTrans->streamMux.pat[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTrans->streamMux.pat[0] + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTrans->streamMux.pmt[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTrans->streamMux.pmt[0] + buf_used, pmt_pl_buf, pmt_pl_size);
    BDBG_MSG(("\nContext%d output PMT packet:", pTrans->uiIndex));
    for(i=0; i < BTST_TS_HEADER_BUF_LENGTH; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x",
            *((uint8_t*)pTrans->streamMux.pmt[0]+i), *((uint8_t*)pTrans->streamMux.pmt[0]+i+1), *((uint8_t*)pTrans->streamMux.pmt[0]+i+2), *((uint8_t*)pTrans->streamMux.pmt[0]+i+3),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+4), *((uint8_t*)pTrans->streamMux.pmt[0]+i+5), *((uint8_t*)pTrans->streamMux.pmt[0]+i+6), *((uint8_t*)pTrans->streamMux.pmt[0]+i+7)));
    }

}


static void TRNSX_StreamMux_SystemData_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    uint8_t vidStreamType;
    uint8_t audStreamType=0x4;
    uint16_t audPid = 0;
    unsigned i;
    NEXUS_AudioCodec audCodec = NEXUS_AudioCodec_eUnknown;

    BSTD_UNUSED(audCodec);

    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pTrans->streamMux.pat[i]);
        NEXUS_Memory_Allocate(BTST_TS_HEADER_BUF_LENGTH, NULL, &pTrans->streamMux.pmt[i]);
    }

    /* decide the stream type to set in PMT */
    /*if(!pContext->bNoVideo) {*/
    if(1) {
        switch(pTrans->videoEncoder.startSettings.codec)
        {
            case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
            case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
            case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
            case NEXUS_VideoCodec_eH265:          vidStreamType = 0x24; break;
            case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
            case NEXUS_VideoCodec_eVp9:           vidStreamType = 0x00; break; /* VP9 in TS doesn't actually work, but this is in place for VCE regression testing */
            default:
                BDBG_ERR(("Video encoder codec %d is not supported!\n", pTrans->videoEncoder.startSettings.codec));
                BDBG_ASSERT(0);
        }
    }
#if 0
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
#endif

    TRNSX_StreamMux_SystemData_AddPatPmt( pCtxt, pTrans, (0 != pTrans->streamMux.startSettings.pcr.interval ) ? BTST_MUX_PCR_PID : 0, BTST_MUX_VIDEO_PID, audPid, vidStreamType, audStreamType);
    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        if(i > 0)
        {
            BKNI_Memcpy(pTrans->streamMux.pat[i], pTrans->streamMux.pat[0], BTST_TS_HEADER_BUF_LENGTH);
            BKNI_Memcpy(pTrans->streamMux.pmt[i], pTrans->streamMux.pmt[0], BTST_TS_HEADER_BUF_LENGTH);
        }
        NEXUS_Memory_FlushCache(pTrans->streamMux.pat[i], BTST_TS_HEADER_BUF_LENGTH);
        NEXUS_Memory_FlushCache(pTrans->streamMux.pmt[i], BTST_TS_HEADER_BUF_LENGTH);
    }
    BKNI_Memset(pTrans->streamMux.psi, 0, sizeof(pTrans->streamMux.psi));
    pTrans->streamMux.psi[0].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pTrans->streamMux.psi[0].pData = (void*)((uint8_t*)pTrans->streamMux.pat[0] + 1);
    pTrans->streamMux.psi[0].timestampDelta = 0;
    pTrans->streamMux.psi[1].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pTrans->streamMux.psi[1].pData = (void*)((uint8_t*)pTrans->streamMux.pmt[0] + 1);
    pTrans->streamMux.psi[1].timestampDelta = 0;
    NEXUS_StreamMux_AddSystemDataBuffer(pTrans->streamMux.streamMux, &pTrans->streamMux.psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(pTrans->streamMux.streamMux, &pTrans->streamMux.psi[1]);
    BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pTrans->streamMux.pat[0] + 4) & 0xf,
        *((uint8_t*)pTrans->streamMux.pmt[0] + 4) & 0xf));

}


static void TRNSX_StreamMux_SystemData_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
)
{
    BSTD_UNUSED(pCtxt);
    if(!pTrans->streamMux.systemdataTimerIsStarted) {
        /* schedule a periodic timer to insert PAT/PMT */
        B_ThreadSettings settingsThread;
        pTrans->streamMux.mutexSystemdata = B_Mutex_Create(NULL);
        pTrans->streamMux.schedulerSystemdata = B_Scheduler_Create(NULL);
        /* create thread to run scheduler */
        B_Thread_GetDefaultSettings(&settingsThread);
        pTrans->streamMux.schedulerThread = B_Thread_Create("systemdata_Scheduler",
                                                            (B_ThreadFunc)B_Scheduler_Run,
                                                            pTrans->streamMux.schedulerSystemdata,
                                                            &settingsThread);
        if (NULL == pTrans->streamMux.schedulerThread)
        {
            BDBG_ERR(("failed to create scheduler thread"));
        }
        pTrans->streamMux.systemdataTimer = B_Scheduler_StartTimer( pTrans->streamMux.schedulerSystemdata,
                                                                    pTrans->streamMux.mutexSystemdata,
                                                                    1000,
                                                                    TRNSX_StreamMux_SystemData_InsertTimer,
                                                                    pTrans);

        if(pTrans->streamMux.systemdataTimer==NULL) {BDBG_ERR(("schedule timer error"));}
        pTrans->streamMux.systemdataTimerIsStarted = true;
    }
}

static void TRNSX_StreamMux_SystemData_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
)
{
    BSTD_UNUSED(pCtxt);

    /* cancel system data timer */
    if(pTrans->streamMux.systemdataTimerIsStarted)
    {
        B_Scheduler_CancelTimer(pTrans->streamMux.schedulerSystemdata, pTrans->streamMux.systemdataTimer);
        B_Scheduler_Stop(pTrans->streamMux.schedulerSystemdata);
        B_Scheduler_Destroy(pTrans->streamMux.schedulerSystemdata);
        if (pTrans->streamMux.schedulerThread)
        {
            B_Thread_Destroy(pTrans->streamMux.schedulerThread);
            pTrans->streamMux.schedulerThread = NULL;
        }
        B_Mutex_Destroy(pTrans->streamMux.mutexSystemdata);
        pTrans->streamMux.systemdataTimer = NULL;
        pTrans->streamMux.systemdataTimerIsStarted = false;
    }
}

static void TRNSX_StreamMux_SystemData_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
)
{
    unsigned i;

    BSTD_UNUSED(pCtxt);

    for(i=0; i<BTST_PSI_QUEUE_CNT; i++)
    {
        NEXUS_Memory_Free(pTrans->streamMux.pat[i]);
        NEXUS_Memory_Free(pTrans->streamMux.pmt[i]);
    }
}

/* Search for the following in transcode_ts
pContext->muxConfig
playpumpTranscodeMCPB - to understand where/how to use the playpump

g_TtsOutputType - set a global value? (now pTrans->streamMux.transportTimestampType )
- used in  void xcode_loopback_setup( TranscodeContext  *pContext ) as well

pidChannelTranscodePcr, pidRemuxPcr;
pidChannelTranscodePat, pidRemuxPat;
pidChannelTranscodePmt, pidRemuxPmt;
finishEvent;
bTsUserDataInput = g_bTsUserData; TODO: need to expose to user
numUserDataPids;
g_muxLatencyTolerance = 20;
bAudioInput
bNonRealTime

TODO: make the following session settings so they only need to be set once?
.servicePeriod = 50;   # g_msp = 50ms Mux Serivce Period
latencyTolerance  = 20;  # g_muxLatencyTolerance mux service latency tolerance


*/

/* cloned from transcode_ts::xcode_setup_mux_record*/


/*
search for pContext->streamMux in transcode-ts


*/

static void TRNSX_StreamMux_Open(
    /* TranscodeContext  *pContext,*/
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eOpened;

    bool bTtsOutput = (NEXUS_TransportTimestampType_eNone != pTrans->streamMux.transportTimestampType);

#if 0
    NEXUS_Playpump_GetDefaultOpenSettings(&pTrans->streamMux.playpumpOpenSettings);
    pTrans->streamMux.playpumpOpenSettings.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    pTrans->streamMux.playpumpOpenSettings.numDescriptors = pTrans->streamMux.uiNumDescs; /* set number of descriptors */
    pTrans->streamMux.playpumpOpenSettings.streamMuxCompatible = true;
#endif

    /*if (pTrans->streamMux.bMCPB)*/
    if ( 1 )
    {
        pTrans->streamMux.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &pTrans->streamMux.playpumpOpenSettings);
        assert(pTrans->streamMux.playpump);
#if 0
        NEXUS_Playpump_GetSettings(pTrans->streamMux.playpump, &pTrans->streamMux.playpumpSettings);
        pTrans->streamMux.playpumpSettings.transportType = NEXUS_TransportType_eEs;
#endif
        pTrans->streamMux.playpumpSettings.timestamp.forceRestamping = bTtsOutput; /* TODO: allow the user to set this directly? */
        NEXUS_Playpump_SetSettings(pTrans->streamMux.playpump, &pTrans->streamMux.playpumpSettings);
    } else {
        pTrans->streamMux.playpump = NULL;
    }

    /* If bMCPB is false, should the following values for playpumpSettings be used instead of the preceding values? */

#if 0
    if(!pCtxt->bNoVideo) {
       if (NULL == pTrans->streamMux.playpump) {
          pCtxt->playpumpTranscodeVideo = NEXUS_Playpump_Open(NEXUS_ANY_ID, &pTrans->streamMux.playpumpOpenSettings);
          BDBG_MSG(("Transcoder%d opened TS mux video PES playpump%d [%p].", pTrans->uiIndex, BTST_STREAM_MUX_VIDEO_PLAYPUMP_IDX+pTrans->uiIndex*4, (void*)pCtxt->playpumpTranscodeVideo));
#if BTST_DISABLE_VIDEO_ENCODER_PES_PACING
          NEXUS_Playpump_SuspendPacing(pCtxt->playpumpTranscodeVideo, true);
#endif
         NEXUS_Playpump_GetSettings(pCtxt->playpumpTranscodeVideo, &pTrans->streamMux.playpumpSettings);
         pTrans->streamMux.playpumpSettings.timestamp.forceRestamping = bTtsOutput;
         NEXUS_Playpump_SetSettings(pCtxt->playpumpTranscodeVideo, &pTrans->streamMux.playpumpSettings);
       } else {
          pCtxt->playpumpTranscodeVideo = pTrans->streamMux.playpump;
       }
        assert(pCtxt->playpumpTranscodeVideo);
    }
#endif

   /* Again, if bMCPB is false, what should be used for playpumpSettings? */

#if 0
   if (NULL == pTrans->streamMux.playpump) {
      pCtxt->playpumpTranscodePcr = NEXUS_Playpump_Open(NEXUS_ANY_ID, &pTrans->streamMux.playpumpOpenSettings);
      BDBG_MSG(("Transcoder%d opened TS mux PCR playpump%d [%p].", pTrans->uiIndex, BTST_STREAM_MUX_PCR_PLAYPUMP_IDX+pTrans->uiIndex*4, (void*)pCtxt->playpumpTranscodePcr));
      NEXUS_Playpump_GetSettings(pCtxt->playpumpTranscodePcr, &pTrans->streamMux.playpumpSettings);
      pTrans->streamMux.playpumpSettings.timestamp.forceRestamping = bTtsOutput || pCtxt->bRemux;
      pTrans->streamMux.playpumpSettings.blindSync = true; /* PCR channel has full transport packets, so blind sync mode */
      NEXUS_Playpump_SetSettings(pCtxt->playpumpTranscodePcr, &pTrans->streamMux.playpumpSettings);
   } else {
      pCtxt->playpumpTranscodePcr = pTrans->streamMux.playpump;
   }
    assert(pCtxt->playpumpTranscodePcr);
#endif

    BKNI_CreateEvent(&pTrans->streamMux.finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&pTrans->streamMux.createSettings);
    pTrans->streamMux.createSettings.finished.callback = TRNSX_Callback_TranscoderFinished; /*transcoderFinishCallback;*/
    pTrans->streamMux.createSettings.finished.context = pTrans->streamMux.finishEvent;

#if 0
    {
        /* reduce stream mux memory allocation if no TS user data passthru */
        NEXUS_StreamMuxConfiguration streamMuxConfig;
        NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfig);
        if(!pTrans->streamMux.bTsUserDataInput) {
            streamMuxConfig.userDataPids = 0;/* remove unnecessary memory allocation */
        } else if(pTrans->streamMux.numUserDataPids == BTST_TS_USER_DATA_ALL) {
            streamMuxConfig.userDataPids = NEXUS_MAX_MUX_PIDS;
        } else {
            streamMuxConfig.userDataPids = pTrans->streamMux.numUserDataPids;
        }
        streamMuxConfig.audioPids           = pTrans->streamMux.bAudioInput ? 1 : 0;
        streamMuxConfig.latencyTolerance    = pTrans->streamMux.muxLatencyTolerance;
        streamMuxConfig.servicePeriod       = g_msp;
        streamMuxConfig.nonRealTime         = pTrans->bNonRealTime;
        streamMuxConfig.muxDelay            = pCtxt->encodeDelay;/* A2P delay affects mux resource allocation */
        streamMuxConfig.supportTts          = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);

        NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfig,&pTrans->streamMux.createSettings.memoryConfiguration);
    }
#else
    /* reduce stream mux memory allocation if no TS user data passthru */

    if(!pTrans->streamMux.bTsUserDataInput) {
        pTrans->streamMux.configuration.userDataPids = 0;/* remove unnecessary memory allocation */
    } else if(pTrans->streamMux.numUserDataPids == BTST_TS_USER_DATA_ALL) {
        pTrans->streamMux.configuration.userDataPids = NEXUS_MAX_MUX_PIDS;
    } else {
        pTrans->streamMux.configuration.userDataPids = pTrans->streamMux.numUserDataPids;
    }

    /* set pCtxt->encodeDelay; A2P delay affects mux resource allocation */
    /* TODO: set after the video encoder starts?  Use pTrans->videoEncoder.settings.encoderDelay? */
    /* TODO: the call to NEXUS_VideoEncoder_GetDelayRange is needs to move for this to work. */
    pTrans->streamMux.configuration.muxDelay = pTrans->videoEncoder.settings.encoderDelay;

    pTrans->streamMux.configuration.nonRealTime = pTrans->bNonRealTime;
    pTrans->streamMux.configuration.supportTts = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);

    NEXUS_StreamMux_GetMemoryConfiguration( &pTrans->streamMux.configuration, &pTrans->streamMux.createSettings.memoryConfiguration );

#endif

    pTrans->streamMux.streamMux = NEXUS_StreamMux_Create(&pTrans->streamMux.createSettings);
    assert(pTrans->streamMux.streamMux);

    pTrans->streamMux.startSettings.stcChannel = pTrans->videoEncoder.stcChannel; /*TODO: pCtxt->stcChannelTranscode, is this right, what about audio? */
    pTrans->streamMux.startSettings.nonRealTime = pTrans->bNonRealTime; /* TODO: should this be exposed separately to the user? */

    /*if(!pCtxt->bNoVideo) */
    if( 1 )
    {
        pTrans->streamMux.startSettings.video[0].pid = BTST_MUX_VIDEO_PID;
        pTrans->streamMux.startSettings.video[0].encoder = pTrans->videoEncoder.videoEncoder;
        pTrans->streamMux.startSettings.video[0].playpump = pTrans->streamMux.playpump; /*pCtxt->playpumpTranscodeVideo;*/
    }

#if 0
    if(pTrans->streamMux.bAudioInput)
    {
        /* audio playpump here is for ts muxer */
       if (NULL == pTrans->streamMux.playpump) {
         NEXUS_Playpump_GetDefaultOpenSettings(&pTrans->streamMux.playpumpOpenSettings);
         pTrans->streamMux.playpumpOpenSettings.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
         pTrans->streamMux.playpumpOpenSettings.numDescriptors = pTrans->streamMux.uiNumDescs; /* set number of descriptors */
         pTrans->streamMux.playpumpOpenSettings.streamMuxCompatible = true;
         pCtxt->playpumpTranscodeAudio = NEXUS_Playpump_Open(NEXUS_ANY_ID, &pTrans->streamMux.playpumpOpenSettings);
         BDBG_MSG(("Transcoder%d opened TS mux audio PES playpump%d [%p].", pTrans->uiIndex, BTST_STREAM_MUX_AUDIO_PLAYPUMP_IDX+pTrans->uiIndex*4, (void*)pCtxt->playpumpTranscodeAudio));
         NEXUS_Playpump_GetSettings(pCtxt->playpumpTranscodeAudio, &pTrans->streamMux.playpumpSettings);
         pTrans->streamMux.playpumpSettings.timestamp.forceRestamping = bTtsOutput;
         NEXUS_Playpump_SetSettings(pCtxt->playpumpTranscodeAudio, &pTrans->streamMux.playpumpSettings);
       } else {
          pCtxt->playpumpTranscodeAudio = pTrans->streamMux.playpump;
       }
        assert(pCtxt->playpumpTranscodeAudio);

        pTrans->streamMux.startSettings.audio[0].pid = BTST_MUX_AUDIO_PID;
        pTrans->streamMux.startSettings.audio[0].muxOutput = pCtxt->audioMuxOutput;
        pTrans->streamMux.startSettings.audio[0].playpump = pCtxt->playpumpTranscodeAudio;
        pTrans->streamMux.startSettings.audio[0].pesPacking = g_audioPesPacking;
    }
#endif

    pTrans->streamMux.startSettings.pcr.pid         = BTST_MUX_PCR_PID;
    pTrans->streamMux.startSettings.pcr.playpump    = pTrans->streamMux.playpump; /*pCtxt->playpumpTranscodePcr;*/
    /* TODO: set after the video encoder starts?  Use pTrans->videoEncoder.settings.encoderDelay? */
    pTrans->streamMux.startSettings.muxDelay        = pTrans->videoEncoder.settings.encoderDelay;
    pTrans->streamMux.startSettings.supportTts      = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);


    {
        /* Set up xcoder record output
         *
         * NRT workaround for the XPT band hold and data ready events sharing the same threshold: avoid band hold from occuring,
         * otherwise, video stutter would happen!
         * 1) The nexus_record timer fires at 250 ms interval to service record data as bottom line;
         * 2) whenever nexus record timer fires, it'll consume up to Nx(3x(47x4096)) cdb data;
         * 3) so if file i/o can keep up, band hold threshold = 2x(3x(47x4096)) = 1.1MB can sustain record bit rate up to
         *       2 * 3 * 188K * 8 / 250ms = 36 Mbps without reaching band hold;
         * 4) larger band hold threshold can sustain higher record bit rate throughput.
         * NOTE: after SW7425-1663, recpump data ready threshold is decoupled with RAVE upper (band-hold) threshold, so
         * we do not need to mess with data ready threshold any more!
         */

        NEXUS_RecpumpOpenSettings       recpumpOpenSettings;

        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        BDBG_MSG(("To open recpump with dataReadyThreshold = %d indexReadyThreshold=%d",
                        recpumpOpenSettings.data.dataReadyThreshold,
                        recpumpOpenSettings.index.dataReadyThreshold));
        BDBG_MSG(("        recpump with data fifo size     = "BDBG_UINT64_FMT" index fifo size    ="BDBG_UINT64_FMT,
                        BDBG_UINT64_ARG((uint64_t)recpumpOpenSettings.data.bufferSize),
                        BDBG_UINT64_ARG((uint64_t)recpumpOpenSettings.index.bufferSize)));

        pTrans->streamMux.recpump = NEXUS_Recpump_Open( NEXUS_ANY_ID, &recpumpOpenSettings );
        assert(pTrans->streamMux.recpump);

        BDBG_MSG(("Transcoder%d opened TS mux recpump%d [%p].", pTrans->uiIndex, pTrans->uiIndex, (void*)pTrans->streamMux.recpump));

        pTrans->streamMux.record = NEXUS_Record_Create();
        assert(pTrans->streamMux.record);
    }

    /*******************************
     * create system data PAT/PMT
     */
    TRNSX_StreamMux_SystemData_Open( pCtxt, pTrans );

    /*******************************
     *  TS user data pass through setup
     */
#if 0
    if(pTrans->streamMux.bTsUserDataInput && pCtxt->inputSettings.resource != BTST_RESOURCE_HDMI) {
        size_t i;

        NEXUS_Message_GetDefaultSettings(&pTrans->streamMux.messageSettings);
        /* SCTE 270 spec max TS VBI user data bitrate=270Kbps, 256KB buffer can hold 7.5 seconds;
           worthy user data for video synchronization; TODO: may be reduced if unnecessary */
        pTrans->streamMux.messageSettings.bufferSize = 512*1024;
        pTrans->streamMux.messageSettings.maxContiguousMessageSize = 0; /* to support TS capture and in-place operation */
        pTrans->streamMux.messageSettings.overflow.callback = message_overflow_callback; /* report overflow error */
        pTrans->streamMux.messageSettings.overflow.context  = pCtxt;

        /* open source user data PID channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&pTrans->streamMux.playbackPidSettings);
        pTrans->streamMux.playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther; /* capture the TS packets with the user data PES */
        pTrans->streamMux.playbackPidSettings.pidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;

        BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pTrans->streamMux.numUserDataPids);
        for(i = 0; i < pTrans->streamMux.numUserDataPids; i++) {
            if(pCtxt->bUserDataStreamValid[i]) {
                pTrans->streamMux.messageSettings.overflow.param = pCtxt->inputSettings.remapUserDataPid[i];
                BDBG_MSG(("context%d opened message buffer for user data PID %d remapped %d", pTrans->uiIndex,
                    pCtxt->inputSettings.userDataPid[i], pCtxt->inputSettings.remapUserDataPid[i]));
                pTrans->streamMux.startSettings.userdata[i].message = NEXUS_Message_Open(&pTrans->streamMux.messageSettings);
                BDBG_ASSERT(pTrans->streamMux.startSettings.userdata[i].message);
                pCtxt->userDataMessage[i] = pTrans->streamMux.startSettings.userdata[i].message;

                if(pCtxt->bRemapUserDataPid) {
                    pTrans->streamMux.playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
                    pTrans->streamMux.playbackPidSettings.pidSettings.pidSettings.remap.pid     = pCtxt->inputSettings.remapUserDataPid[i];/* optional PID remap */
                }
                pCtxt->pidChannelUserData[i] = NEXUS_Playback_OpenPidChannel(pCtxt->playback,
                    pCtxt->inputSettings.userDataPid[i], &pTrans->streamMux.playbackPidSettings);
                BDBG_ASSERT(pCtxt->pidChannelUserData[i]);

                /* must start message before stream mux starts */
                NEXUS_Message_GetDefaultStartSettings(pTrans->streamMux.startSettings.userdata[i].message, &pTrans->streamMux.messageStartSettings);
                pTrans->streamMux.messageStartSettings.format = NEXUS_MessageFormat_eTs;
                pTrans->streamMux.messageStartSettings.pidChannel = pCtxt->pidChannelUserData[i];
                NEXUS_Message_Start(pTrans->streamMux.startSettings.userdata[i].message, &pTrans->streamMux.messageStartSettings);

                /* open transcode mux output user data PidChannels out of system data channel */
                pCtxt->pidChannelTranscodeUserData[i] = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump, /*pCtxt->playpumpTranscodePcr,*/
                    pCtxt->inputSettings.remapUserDataPid[i], NULL);
                BDBG_ASSERT(pCtxt->pidChannelTranscodeUserData[i]);
            }
        }
    }
#endif
    /* open PidChannels */
    pTrans->streamMux.pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, pTrans->streamMux.startSettings.pcr.pid, NULL);
    assert(pTrans->streamMux.pidChannelTranscodePcr);
    pTrans->streamMux.pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, BTST_MUX_PMT_PID, NULL);
    assert(pTrans->streamMux.pidChannelTranscodePmt);
    pTrans->streamMux.pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, BTST_MUX_PAT_PID, NULL);
    assert(pTrans->streamMux.pidChannelTranscodePat);

#if 0
    if(pCtxt->bRemux) {/* remux output setup */
        NEXUS_RemuxSettings remuxSettings;
        NEXUS_RemuxParserBandwidth remuxParserBandwidth;
        NEXUS_ParserBandSettings parserBandSettings;

        /* Configure remux output  */
        NEXUS_Remux_GetDefaultSettings(&remuxSettings);
        remuxSettings.outputClock = NEXUS_RemuxClock_e27Mhz_VCXO_A;/* this worked; TODO: what about other options? */
        remuxSettings.enablePcrJitterAdjust = true;
        remuxSettings.insertNullPackets = true; /* has to enable to get NULL packets inserted in output */
        pCtxt->remux = NEXUS_Remux_Open(NEXUS_ANY_ID, &remuxSettings);

        /* route remux output via a parser band to loopback for record */
        pCtxt->parserBandRemux = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        NEXUS_ParserBand_GetSettings(pCtxt->parserBandRemux, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
        parserBandSettings.sourceTypeSettings.remux = pCtxt->remux;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.acceptNullPackets = true; /* has to enable to get NULL packets inserted in output */
        parserBandSettings.allPass = true; /* has to enable to get NULL packets inserted in output */
        parserBandSettings.maxDataRate = 30000000; /* max rate */
        NEXUS_ParserBand_SetSettings(pCtxt->parserBandRemux, &parserBandSettings);
        NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
        remuxParserBandwidth.maxDataRate = 30000000;
        NEXUS_Remux_SetParserBandwidth(pCtxt->remux, &remuxParserBandwidth);
    }
#endif
    NEXUS_Record_GetSettings(pTrans->streamMux.record, &pTrans->streamMux.recordSettings);
    pTrans->streamMux.recordSettings.recpump = pTrans->streamMux.recpump;
    /* NOTE: enable band hold to allow recpump to stall TS mux to avoid ave data corruption in case file i/o is slow
     * The encoders should make sure no output CDB/ITB buffer corruption (instead do frame drop) when output overflow! */
    pTrans->streamMux.recordSettings.recpumpSettings.bandHold = pTrans->bNonRealTime? NEXUS_RecpumpFlowControl_eEnable : NEXUS_RecpumpFlowControl_eDisable;
    pTrans->streamMux.recordSettings.recpumpSettings.timestampType = pTrans->streamMux.transportTimestampType;
    pTrans->streamMux.recordSettings.recpumpSettings.localTimestamp = bTtsOutput/* && pTrans->bNonRealTime*/;
    pTrans->streamMux.recordSettings.recpumpSettings.adjustTimestampUsingPcrs = bTtsOutput;
    pTrans->streamMux.recordSettings.recpumpSettings.pcrPidChannel = pTrans->streamMux.pidChannelTranscodePcr;
    pTrans->streamMux.recordSettings.recpumpSettings.dropBtpPackets= true;
    pTrans->streamMux.recordSettings.recpumpSettings.data.overflow.callback = TRNSX_StreamMux_RecpumpOverflowCallback;
    pTrans->streamMux.recordSettings.recpumpSettings.data.overflow.context  = pTrans;
    pTrans->streamMux.recordSettings.pollingTimer = 50;
    if(pTrans->bNonRealTime) {/* write all data for NRT */
        pTrans->streamMux.recordSettings.writeAllTimeout = 100;
    }
    NEXUS_Record_SetSettings(pTrans->streamMux.record, &pTrans->streamMux.recordSettings);


    /* set record index file name and open the record file handle; disable indexer for REMUX output due to allpass parser pids! */
    if( 1 /*!pCtxt->bNoVideo*/ && ((pTrans->videoEncoder.startSettings.codec==NEXUS_VideoCodec_eMpeg2)
        || (pTrans->videoEncoder.startSettings.codec==NEXUS_VideoCodec_eH264)) && !pTrans->streamMux.bRemux)
    {
        /* TODO: use samee code as for the ES files. */
#if 0
        xcode_index_filename(pCtxt->indexfname, pCtxt->encodeSettings.fname);
#endif
    }
    else BDBG_WRN(("%s: no index record", BSTD_FUNCTION));

}


/* Reference transcode_ts::close_transcode */

static void TRNSX_StreamMux_Close(
    /* TranscodeContext  *pContext,*/
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BSTD_UNUSED(pCtxt);

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eClosed;

    /*******************************
     * release system data PAT/PMT
     */
    TRNSX_StreamMux_SystemData_Close(pCtxt, pTrans);


    /************************************
     * Bring down transcoder
     */
#if 0
    if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
        for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
            if(pContext->bUserDataStreamValid[i]) {
                NEXUS_Message_Stop(pContext->userDataMessage[i]);
                NEXUS_Message_Close(pContext->userDataMessage[i]);
            }
        }
    }
#endif


    if ( pTrans->streamMux.record )
    {
        NEXUS_Record_Destroy(pTrans->streamMux.record);
        pTrans->streamMux.record = NULL;
    }


    if ( pTrans->streamMux.recpump )
    {
        NEXUS_Recpump_Close(pTrans->streamMux.recpump);
        pTrans->streamMux.recpump = NULL;
    }

#if 0
    if(pContext->bRemux) {
        NEXUS_Remux_Close(pContext->remux);
        NEXUS_ParserBand_Close(pContext->parserBandRemux);
    }
#endif


    /* This is handle in decode stop/close, does the order matter? */
#if 0
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
#endif

    NEXUS_Playpump_CloseAllPidChannels(pTrans->streamMux.playpump);

    /******************************************
     * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
     */
#if 0

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
#endif

    if ( pTrans->streamMux.finishEvent )
    {
        BKNI_DestroyEvent(pTrans->streamMux.finishEvent);
        pTrans->streamMux.finishEvent = NULL;
    }

    if ( pTrans->streamMux.streamMux )
    {
        NEXUS_StreamMux_Destroy(pTrans->streamMux.streamMux);
        pTrans->streamMux.streamMux = NULL;
    }


    /*if ( *pTrans->streamMux.playpump != pContext->playpumpTranscodeMCPB )    */
    if ( pTrans->streamMux.playpump )
    {
        NEXUS_Playpump_Close(pTrans->streamMux.playpump);
        pTrans->streamMux.playpump = NULL;
    }

#if 0
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
#endif

#if 0
    if ( NULL != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeMCPB);
#endif

    /* Handled in encode and decoder close? */
#if 0
    /* NOTE: each transcoder context only needs two separate STCs;
       if NRT mode, the audio STC is the same as transcode STC; if RT mode, audio STC is the same as video STC. */
    if(!g_activeXcodeCount || !g_bSimulXcode) {
        NEXUS_StcChannel_Close(pContext->stcVideoChannel);
        NEXUS_StcChannel_Close(pContext->stcChannelTranscode);
    }
#endif

#if 0
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
#endif

    return;

}

/* use transcode_ts::start_transcode as the reference */

static NEXUS_Error TRNSX_StreamMux_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if NEXUS_HAS_STREAM_MUX
    NEXUS_PidChannelHandle     pidChannelTranscodeVideo;


    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_RecordPidChannelSettings recordPidSettings;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eRunning;

    /*******************************
     * Add system data to stream_mux
     */
    TRNSX_StreamMux_SystemData_Start( pCtxt, pTrans );

    /**************************************************
     * NOTE: transcoder record/mux resume
     */
    /* start mux */
    NEXUS_StreamMux_Start(pTrans->streamMux.streamMux,&pTrans->streamMux.startSettings, &muxOutput);
    pidChannelTranscodeVideo = muxOutput.video[0];

    /* add multiplex data to the same record */
    /* configure the video pid for indexing */
    /*if(!pContext->bNoVideo) {*/
    if( 1 ) {
        NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
        recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        /* don't index if remux output with allpass pids */
        recordPidSettings.recpumpSettings.pidTypeSettings.video.index = /* RAVE only support mpeg2&avc indexing */
                ((pTrans->videoEncoder.startSettings.codec==NEXUS_VideoCodec_eMpeg2) ||
                 (pTrans->videoEncoder.startSettings.codec==NEXUS_VideoCodec_eH264)) && !pTrans->streamMux.bRemux;
        recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = pTrans->videoEncoder.startSettings.codec;
#if 0
        if(pTrans->streamMux.bRemux) {/* mux -> remux ->parser -> record */
            NEXUS_Remux_AddPidChannel(pContext->remux, pidChannelTranscodeVideo);
            pContext->pidRemuxVideo = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.startSettings.video[0].pid, NULL);
            NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxVideo, &recordPidSettings);
        } else
#endif
        {
            NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pidChannelTranscodeVideo, &recordPidSettings);
        }
    }

#if 0
    /* for audio channel */
    if(pContext->inputSettings.bAudioInput)
    {
        pTrans->streamMux.pidChannelTranscodeAudio = muxOutput.audio[0];
        if(pTrans->streamMux.bRemux) {/* mux -> remux -> record */
            NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodeAudio);
            pContext->pidRemuxAudio = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.startSettings.audio[0].pid, NULL);
            NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxAudio, &recordPidSettings);
        } else {
            NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodeAudio, NULL);
        }
    }
#endif

#if 0
    /* for system data channel */
    if(pTrans->streamMux.bRemux) {/* mux -> remux -> parser -> record */
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePcr);
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePat);
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePmt);

        pContext->pidRemuxPcr = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.startSettings.pcr.pid, NULL);
        BDBG_ASSERT(pContext->pidRemuxPcr);
        pContext->pidRemuxPmt = NEXUS_PidChannel_Open(pContext->parserBandRemux, BTST_MUX_PMT_PID, NULL);
        BDBG_ASSERT(pContext->pidRemuxPmt);
        pContext->pidRemuxPat = NEXUS_PidChannel_Open(pContext->parserBandRemux, BTST_MUX_PAT_PID, NULL);
        BDBG_ASSERT(pContext->pidRemuxPat);
        /* it seems the null packets would be recorded without explicitly added here as parser band enabled allPass; */
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPcr, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPat, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPmt, NULL);
        if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pContext->inputSettings.numUserDataPids);
            for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
                if(pContext->bUserDataStreamValid[i]) {
                    NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodeUserData[i]);
                    pContext->pidRemuxUserData[i] = NEXUS_PidChannel_Open(pContext->parserBandRemux, pContext->inputSettings.remapUserDataPid[i], NULL);
                    NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxUserData[i], NULL);
                }
            }
        }
        NEXUS_Remux_Start(pContext->remux);
    }
    else
#endif
    {
        /* non-remux output */
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePcr, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePat, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePmt, NULL);
#if 0
        if(pContext->inputSettings.bTsUserDataInput && pContext->inputSettings.resource != BTST_RESOURCE_HDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pContext->inputSettings.numUserDataPids);
            for(i = 0; i < pContext->inputSettings.numUserDataPids; i++) {
                if(pContext->bUserDataStreamValid[i]) {
                    NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodeUserData[i], NULL);
                }
            }
        }
#endif
    }

    if (muxOutput.pcr)
    {
       NEXUS_Record_AddPidChannel(pTrans->streamMux.record, muxOutput.pcr, NULL);
    }
#if 0
    if(g_bFifo && pTrans->record.indexfname[0]) {
        NEXUS_FifoRecordSettings fifoRecordSettings;

        pContext->fifoFile = NEXUS_FifoRecord_Create(pTrans->record.fname, pTrans->record.indexfname);

        NEXUS_FifoRecord_GetSettings(pContext->fifoFile, &fifoRecordSettings);
        fifoRecordSettings.interval = 60;
        if(NEXUS_FifoRecord_SetSettings(pContext->fifoFile, &fifoRecordSettings)) {
            BDBG_ERR(("Transcoder[%u] failed set fifo record", pContext->contextId)); BDBG_ASSERT(0);
        }

        pTrans->streamMux.fileRecord = NEXUS_FifoRecord_GetFile(pContext->fifoFile);
        BDBG_MSG(("Opened record fifo file."));
    }
    else
#endif
    {
        NEXUS_FileRecordOpenSettings openSettings;
        NEXUS_FileRecord_GetDefaultOpenSettings(&openSettings);
        openSettings.data.filename = pTrans->record.fname;
        openSettings.data.directIo = ( pTrans->record.eTransportType != NEXUS_TransportType_eMp4Fragment );/*!g_bfMP4 ?? does this mean to not use directIo for fragment MP4? */
        openSettings.index.filename = pTrans->record.indexfname[0] ? pTrans->record.indexfname : NULL;
        pTrans->streamMux.fileRecord = NEXUS_FileRecord_Open(&openSettings);
    }

    if (!pTrans->streamMux.fileRecord) {
        fprintf(stderr, "can't create file: %s, %s\n", pTrans->record.fname, pTrans->record.indexfname);
        exit(1);
    }

    /* Start record of stream mux output */
    NEXUS_Record_Start(pTrans->streamMux.record, pTrans->streamMux.fileRecord);

    /****************************
     * start decoders
     */
#if 0
    if((pContext->inputSettings.resource != BTST_RESOURCE_HDMI) && !pContext->bNoStopDecode && !pContext->bNoVideo
        && (pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode))
    {
        NEXUS_VideoDecoderExtendedSettings extSettings;

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
#endif

    /* Start playback before mux set up PAT/PMT which may depend on PMT user data PSI extraction */

#if 0
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
#endif

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}

/* use transcode_ts::stop_transcode as the reference */


static NEXUS_Error TRNSX_StreamMux_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if NEXUS_HAS_STREAM_MUX
    /*NEXUS_VideoEncoderStopSettings videoEncoderStopSettings;*/

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eClosed;

   /*******************************
    * stop system data scheduler
    */
    if (!pTrans->streamMux.bPrematureMuxStop)
    {
        TRNSX_StreamMux_SystemData_Stop(pCtxt, pTrans);
    }


    /* TODO: how to enforce proper startup/shutdown order? */

#if 0
    /**************************************************
     * NOTE: stop sequence should be in front->back order
     */
    if((pContext->inputSettings.resource == BTST_RESOURCE_FILE) && !pContext->bNoStopDecode)
    {
        if(pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode) {
            NEXUS_Playback_Stop(pContext->playback);
        }
    }
#endif

#if 0
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
#endif

#if 0

     if(!pContext->bNoVideo) {
         if(pContext->inputSettings.resource != BTST_RESOURCE_HDMI && !pContext->bNoStopDecode) {
             if(pContext->contextId == g_simulDecoderMaster || !g_bSimulXcode)
             NEXUS_VideoDecoder_Stop(pContext->videoDecoder);
         }
         NEXUS_VideoEncoder_GetDefaultStopSettings(&videoEncoderStopSettings);
         videoEncoderStopSettings.mode = pTrans->videoEncoder.stopSettings.mode;
         /* eAbort stop mode will defer stop after mux stop to silence mux errors on pending descriptors */
         if(NEXUS_VideoEncoderStopMode_eAbort != pTrans->videoEncoder.stopSettings.mode) {
             NEXUS_VideoEncoder_Stop(pContext->videoEncoder, &videoEncoderStopSettings);
         }
     }
#endif

    if(NEXUS_VideoEncoderStopMode_eAbort != pTrans->videoEncoder.stopSettings.mode)
    {
        if (!pTrans->streamMux.bPrematureMuxStop)
        {
            NEXUS_StreamMux_Finish(pTrans->streamMux.streamMux);

            if(BKNI_WaitForEvent(pTrans->streamMux.finishEvent, 4000)!=BERR_SUCCESS)
            {
                fprintf(stderr, "%s:: timed out waiting for pTrans->streamMux.finishEvent\n", BSTD_FUNCTION);
            }
        }
     }

#if 0
     if(pTrans->streamMux.bRemux) {
         NEXUS_Remux_Stop(pContext->remux);
         NEXUS_Remux_RemoveAllPidChannels(pContext->remux);
         NEXUS_PidChannel_CloseAll(pContext->parserBandRemux);
     }
#endif

     NEXUS_Record_Stop(pTrans->streamMux.record);

     if (pTrans->streamMux.fileRecord == NULL )
     {
         NEXUS_FileRecord_Close(pTrans->streamMux.fileRecord);
         pTrans->streamMux.fileRecord = NULL;
     }
     /*****************************************
      * Note: remove all record PID channels before stream
      * mux stop since streammux would close the A/V PID channels
      */
     NEXUS_Record_RemoveAllPidChannels(pTrans->streamMux.record);
     if (!pTrans->streamMux.bPrematureMuxStop)
     {
        NEXUS_StreamMux_Stop(pTrans->streamMux.streamMux);
     }

#if 0
     /* stop video encoder after stream mux stop if in eAbort mode to silence the mux errors for pending descriptors. */
     if(!pContext->bNoVideo && NEXUS_VideoEncoderStopMode_eAbort == pTrans->videoEncoder.stopSettings.mode) {
         NEXUS_VideoEncoder_Stop(pContext->videoEncoder, &videoEncoderStopSettings);
     }
#endif

#if 0
    pTrans->videoEncoder.stopSettings.mode = 0; /* reset stop mode whenever used; it needs to be set everytime to be used */
#endif
    pTrans->streamMux.bPrematureMuxStop = false; /* reset mux stop mode whenever used; it needs to be set everytime to be used  */

     /* Temporary workaround to flush pending descriptors from NEXUS_AudioMuxOutput prior to restarting it.
        Restarting will flush the pending descriptors. */
 #if 0
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

     /*if ( g_bPrintStatus ) printStatus( pContext );*/

     /* stopped */
     /*pContext->bStarted = false;*/
#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}


#endif /* x */

/*
 * File mux wrappers.
 */
static NEXUS_Error TRNSX_FileMux_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if  NEXUS_HAS_FILE_MUX
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);


    BKNI_CreateEvent(&pTrans->fileMux.finishEvent);
    NEXUS_FileMux_GetDefaultCreateSettings(&pTrans->fileMux.createSettings);
    pTrans->fileMux.createSettings.finished.callback = TRNSX_Callback_TranscoderFinished;
    pTrans->fileMux.createSettings.finished.context = pTrans->fileMux.finishEvent;

    pTrans->fileMux.fileMux = NEXUS_FileMux_Create(&pTrans->fileMux.createSettings);
    NEXUS_FileMux_GetDefaultStartSettings(&pTrans->fileMux.startSettings, pTrans->record.eTransportType);

    pTrans->fileMux.startSettings.video[0].track = 1;
    pTrans->fileMux.startSettings.video[0].codec = pTrans->videoEncoder.startSettings.codec;
    pTrans->fileMux.startSettings.video[0].encoder = pTrans->videoEncoder.videoEncoder;
    snprintf(pTrans->fileMux.startSettings.tempDir, sizeof(pTrans->fileMux.startSettings.tempDir), "videos"); /*TODO: is this needed? */
    pTrans->fileMux.muxOutput = NEXUS_MuxFile_OpenPosix(pTrans->record.fname);

    if (!pTrans->fileMux.muxOutput) {
        fprintf(stderr, "can't open file:%s\n", pTrans->record.fname);
        return -1;
    }
    else {
        BKNI_Printf("%s: opened file: %s\n", BSTD_FUNCTION, pTrans->record.fname );
    }

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}

static NEXUS_Error TRNSX_FileMux_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if  NEXUS_HAS_FILE_MUX
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    NEXUS_MuxFile_Close(pTrans->fileMux.muxOutput);
    pTrans->fileMux.muxOutput = NULL;
    NEXUS_FileMux_Destroy(pTrans->fileMux.fileMux);
    pTrans->fileMux.fileMux = NULL;
    pTrans->fileMux.bActive = false;

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}

static NEXUS_Error TRNSX_FileMux_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if  NEXUS_HAS_FILE_MUX
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    B_Event_Reset( pTrans->fileMux.finishEvent );

    NEXUS_FileMux_Start(pTrans->fileMux.fileMux,&pTrans->fileMux.startSettings, pTrans->fileMux.muxOutput);
#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}

static NEXUS_Error TRNSX_FileMux_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

#if  NEXUS_HAS_FILE_MUX
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    NEXUS_FileMux_Finish(pTrans->fileMux.fileMux);
    /* wait for encode buffer to be drained; double delay margin */
    while(BKNI_WaitForEvent(pTrans->fileMux.finishEvent, 3000)!=BERR_SUCCESS) {
        fprintf(stderr, "%s: waiting for file mux to finish.\n", BSTD_FUNCTION );
    }
    BKNI_DestroyEvent(pTrans->fileMux.finishEvent);
    pTrans->fileMux.finishEvent = NULL;
    NEXUS_FileMux_Stop(pTrans->fileMux.fileMux);

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif
    return rc;
}

/*
 * Mux and record wrappers, these are for controlling the output.
 * They don't map directly to Nexus calls.
 */

static NEXUS_Error TRNSX_Record_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
            BKNI_Printf("%s: using 'es' mux\n", BSTD_FUNCTION );
            TRNSX_FileEs_Start( pCtxt, pTrans );
            break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            break;

        /* Stream Mux */
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMp4Fragment:
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;

}

static NEXUS_Error TRNSX_Record_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
            TRNSX_FileEs_Stop( pCtxt, pTrans );
            break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            break;

        /* Stream Mux */
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMp4Fragment:
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;

}

static NEXUS_Error TRNSX_Record_Pause(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
            BKNI_Printf("%s: pausing copy thread\n", BSTD_FUNCTION );
            pTrans->record.bCopyData = false;
            break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            break;

        /* Stream Mux */
        case TRNSX_TransportType_eTS:
        case NEXUS_TransportType_eMp4Fragment:
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;
}


static NEXUS_Error TRNSX_Record_Resume(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
            BKNI_Printf("%s: resuming copy thread\n", BSTD_FUNCTION );
            pTrans->record.bCopyData = true;
            break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            break;

        /* Stream Mux */
        case TRNSX_TransportType_eTS:
        case NEXUS_TransportType_eMp4Fragment:
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;
}


static NEXUS_Error TRNSX_Mux_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
             break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            BKNI_Printf("%s: using file mux\n", BSTD_FUNCTION );
            if ( pTrans->record.bGenerateIVF )
            {
                /* This is a little scary, required by Nexus.
                 * How will it play will dual transcode?
                 * Need to unset at the correct time? */
                setenv("IVF_SUPPORT", "y", 1);
            }
            TRNSX_FileMux_Open( pCtxt, pTrans );
            TRNSX_FileMux_Start( pCtxt, pTrans );
            break;

        /* Stream Mux */
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMp4Fragment:
            TRNSX_StreamMux_Open( pCtxt, pTrans );
            TRNSX_StreamMux_Start( pCtxt, pTrans );
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;
}

static NEXUS_Error TRNSX_Mux_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    switch( pTrans->record.eTransportType )
    {
        /* ES - buffer read */
        case NEXUS_TransportType_eEs:
             break;

        /* File Mux */
        case NEXUS_TransportType_eMpeg2Pes:
        case NEXUS_TransportType_eMp4:
            if ( pTrans->record.bGenerateIVF )
            {
                /* This is a little scary, required by Nexus.
                 * How will it play will dual transcode?
                 * Need to unset at the correct time? */
                unsetenv("IVF_SUPPORT");
            }
            TRNSX_FileMux_Stop( pCtxt, pTrans );
            TRNSX_FileMux_Close( pCtxt, pTrans );
            break;

        /* Stream Mux */
        case NEXUS_TransportType_eTs:
        case NEXUS_TransportType_eMp4Fragment:
            TRNSX_StreamMux_Stop( pCtxt, pTrans );
            TRNSX_StreamMux_Close( pCtxt, pTrans );
            break;

        default:
            BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
            rc = NEXUS_INVALID_PARAMETER;
            break;

    }

    return rc;

}

/*
 * Playback wrappers
 */
static NEXUS_Error TRNSX_Playback_Init(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    if ( pTrans->playback.eState != TRNSX_State_eOpened )
    {
        NEXUS_Playback_GetSettings(pTrans->playback.playback, &pTrans->playback.playbackSettings);
        pTrans->playback.playbackSettings.playpump = pTrans->playback.playpump;
        pTrans->playback.playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)pTrans->input.eStreamType;
        pTrans->playback.playbackSettings.stcChannel = pTrans->videoDecoder.stcChannel;
        NEXUS_Playback_SetSettings(pTrans->playback.playback, &pTrans->playback.playbackSettings);


        pTrans->playback.hFilePlay = NEXUS_FilePlay_OpenPosix(pTrans->input.fname, NULL);

        if (!pTrans->playback.hFilePlay) {
            fprintf(stderr, "%s: can't open file:%s\n", BSTD_FUNCTION, pTrans->input.fname);
            return -1;
        }

        /* Open the video pid channel */
        NEXUS_Playback_GetDefaultPidChannelSettings(&pTrans->playback.playbackPidSettings);
        pTrans->playback.playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        pTrans->playback.playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pTrans->input.eVideoCodec; /* must be told codec for correct handling */
        pTrans->playback.playbackPidSettings.pidTypeSettings.video.index = true;
        pTrans->playback.playbackPidSettings.pidTypeSettings.video.decoder = pTrans->videoDecoder.videoDecoder;
        pTrans->videoDecoder.videoPidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pTrans->input.iVideoPid, &pTrans->playback.playbackPidSettings);

        if(pTrans->input.iVideoPid != pTrans->input.iPcrPid)
        {
            pTrans->playback.playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
            pTrans->videoDecoder.pcrPidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pTrans->input.iPcrPid, &pTrans->playback.playbackPidSettings);
        }

    }

    pTrans->playback.eState = TRNSX_State_eOpened;

    return rc;

}

static NEXUS_Error TRNSX_Playback_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    TRNSX_Playback_Init( pCtxt, pTrans );

    NEXUS_Playback_Start(pTrans->playback.playback, pTrans->playback.hFilePlay, NULL);

    return rc;
}

/*
 * Display wrappers
 */
static unsigned TRNSX_VideoEncoderDisplay_GetIndex(
    TRNSX_TestContext *  pCtxt,
    unsigned encoderId
    )
{
    if (encoderId < NEXUS_MAX_VIDEO_ENCODERS && pCtxt->encoderCapabilities.videoEncoder[encoderId].supported) {
        return pCtxt->encoderCapabilities.videoEncoder[encoderId].displayIndex;
    }

    return 0;
}

static NEXUS_Error TRNSX_VideoEncoderDisplay_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     * TODO: is this still the case? */
    /* TODO: should this be enforced? */
    if ( !pTrans->videoEncoder.videoEncoder )
    {
        BDBG_ERR(("%s: the encoder must be opened before the display", BSTD_FUNCTION ));
        return NEXUS_NOT_INITIALIZED;
    }

    if ( pTrans->videoEncoder.display )
    {
        BDBG_ERR(("%s: the display has already been opened", BSTD_FUNCTION ));
        return NEXUS_NOT_INITIALIZED;
    }

    /* Bring up video encoder display */

    pTrans->videoEncoder.display = NEXUS_Display_Open( TRNSX_VideoEncoderDisplay_GetIndex( pCtxt, pTrans->uiIndex), &pTrans->videoEncoder.displaySettings );

    BDBG_ASSERT(pTrans->videoEncoder.display);

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    {
        NEXUS_DisplayStgSettings stgSettings;

        /* NRT setup - AFAP mode */
        NEXUS_Display_GetStgSettings(pTrans->videoEncoder.display, &stgSettings);
        stgSettings.enabled     = true;
        stgSettings.nonRealTime = pTrans->bNonRealTime;
        NEXUS_Display_SetStgSettings(pTrans->videoEncoder.display , &stgSettings);
    }
#endif

    pTrans->videoEncoder.window = NEXUS_VideoWindow_Open(pTrans->videoEncoder.display, 0);
    BDBG_ASSERT(pTrans->videoEncoder.window);

    if( pTrans->videoEncoder.bCustomDisplaySettings )
    {
        rc = NEXUS_Display_SetCustomFormatSettings(pTrans->videoEncoder.display, NEXUS_VideoFormat_eCustom2, &pTrans->videoEncoder.customFormatSettings);
        BDBG_ASSERT(!rc);
    }

    /*NEXUS_VideoWindow_AddInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));*/

#if SIMUL_DISPLAY
    /* Connect same decoder to encoder display.  This simul mode is for video encoder
     * bringup only; audio path may have limitation for simul display+transcode mode; */
    NEXUS_VideoWindow_AddInput(pCtxt->simulDisplay.window,          NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));
#endif

    return rc;

}

/*
 * Decoder wrappers
 */
static NEXUS_Error TRNSX_VideoDecoder_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoDecoder.eState != TRNSX_State_eUninit && pTrans->videoDecoder.eState != TRNSX_State_eClosed )
    {
        BDBG_ERR(("%s: the decoder is %s. It must be uninit or closed before you can call open.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eOpened;

    /* TODO: is this the appropriate place to wrap all the following rouintes? */
    /* TODO: what is the correct index to pass in?*/
    /* TODO: Use pCtxt->decoderCapabilities.numVideoDecoders to select and track the index passed to NEXUS_VideoDecoder_Open */

    pTrans->videoDecoder.videoDecoder = NEXUS_VideoDecoder_Open(pTrans->uiIndex, NULL); /* take default capabilities */

    pTrans->playback.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(pTrans->playback.playpump);

    pTrans->playback.playback = NEXUS_Playback_Create(); /* Calls NEXUS_Playback_GetDefaultSettings(). */
    BDBG_ASSERT(pTrans->playback.playback);

    pTrans->videoDecoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &pTrans->videoDecoder.stcSettings);

    return rc;

}


static NEXUS_Error TRNSX_VideoDecoder_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoDecoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s: the decoder is %s. It must be stopped before you can call close.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eClosed;

#if SIMUL_DISPLAY
    NEXUS_VideoWindow_RemoveInput(pCtxt->simulDisplay.window,          NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));
#endif

    /*NEXUS_VideoWindow_RemoveInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));*/

    NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.videoPidChannel);
    pTrans->videoDecoder.videoPidChannel = NULL;

    if(NULL != pTrans->videoDecoder.pcrPidChannel)
    {
        NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.pcrPidChannel);
        pTrans->videoDecoder.pcrPidChannel = NULL;
    }

    NEXUS_FilePlay_Close(pTrans->playback.hFilePlay);
    pTrans->playback.hFilePlay = NULL;

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    NEXUS_VideoDecoder_Close(pTrans->videoDecoder.videoDecoder);
    pTrans->videoDecoder.videoDecoder = NULL;

    NEXUS_StcChannel_Close(pTrans->videoDecoder.stcChannel);
    pTrans->videoDecoder.stcChannel = NULL;

    NEXUS_Playback_Destroy(pTrans->playback.playback);
    pTrans->playback.playback = NULL;

    NEXUS_Playpump_Close(pTrans->playback.playpump);
    pTrans->playback.playpump = NULL;

    /* TODO: what if we just want to switch files without closing playpump? */
    pTrans->playback.eState = TRNSX_State_eClosed;

    return rc;
}

static NEXUS_Error TRNSX_VideoDecoder_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(TRNSX_VideoDecoder_Start);

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( strlen( pTrans->input.fname ) == 0 )
    {
        BDBG_ERR(("%s: you must specify a source file before starting the decoder.", BSTD_FUNCTION ));
        return NEXUS_NOT_INITIALIZED;
    }

    if ( pTrans->videoDecoder.eState != TRNSX_State_eOpened && pTrans->videoDecoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s: the decoder is %s. It must be opened or stopped before you can call start.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eRunning;

    /* Helping out the user here; perhaps they should be forced to call display open.
     * If the encoder display has not been opened, do so now. */
    if ( !pTrans->videoEncoder.display )
    {
        TRNSX_VideoEncoderDisplay_Open( pCtxt, pTrans );
    }

    TRNSX_Playback_Init( pCtxt, pTrans );

    NEXUS_VideoWindow_AddInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    pTrans->videoDecoder.startSettings.codec = (NEXUS_VideoCodec)pTrans->input.eVideoCodec;
    pTrans->videoDecoder.startSettings.pidChannel = pTrans->videoDecoder.videoPidChannel;
    pTrans->videoDecoder.startSettings.stcChannel = pTrans->videoDecoder.stcChannel;

    /* No guarantee TRNSX_Command_ParseParams will be called, set here.
     * TODO: how to reconcile with user wanting to specify via start settings? */
    pTrans->videoDecoder.startSettings.nonRealTime = pTrans->bNonRealTime;

    NEXUS_VideoDecoder_Start(pTrans->videoDecoder.videoDecoder, &pTrans->videoDecoder.startSettings);

    BDBG_LEAVE(TRNSX_VideoDecoder_Start);

    return rc;
}


static NEXUS_Error TRNSX_VideoDecoder_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoDecoder.eState != TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s: the decoder is %s. It must be running before you can call stop.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eStopped;

    NEXUS_VideoDecoder_Stop( pTrans->videoDecoder.videoDecoder );

    /* To be symmetrical with Decoder_Start */
    NEXUS_VideoWindow_RemoveInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    return rc;
}

static NEXUS_Error TRNSX_VideoDecoder_GetStatus(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_VideoDecoderStatus videoDecoderStatus;

    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoDecoder.videoDecoder )
    {
        NEXUS_VideoDecoder_GetStatus(pTrans->videoDecoder.videoDecoder, &videoDecoderStatus);
        TRNSX_Print_VideoDecoderStatus( &videoDecoderStatus );
    }
    else
    {
        BKNI_Printf("%s: video decoder has not been opened yet\n", BSTD_FUNCTION );
    }

    return rc;
}


/*
 * Video Encoder wrappers
 */

static NEXUS_Error TRNSX_VideoEncoder_SetSettings(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoEncoder.videoEncoder )
    {
        NEXUS_VideoEncoder_SetSettings(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings);
    }
    else
    {
        BDBG_ERR(("%s: encoder must be opened before settings can be set", BSTD_FUNCTION));
    }

    return rc;

}

static NEXUS_Error TRNSX_VideoEncoder_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoEncoder.eState != TRNSX_State_eUninit && pTrans->videoEncoder.eState != TRNSX_State_eClosed )
    {
        BDBG_ERR(("%s: the encoder is %s. It must be uninit or closed before you can call open.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoEncoder.eState = TRNSX_State_eOpened;

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     * TODO: is this still the case? */

    pTrans->videoEncoder.videoEncoder = NEXUS_VideoEncoder_Open(pTrans->uiIndex, &pTrans->videoEncoder.openSettings);
    BDBG_ASSERT(pTrans->videoEncoder.videoEncoder);

    /* encoder requires different STC broadcast mode from decoder */
    pTrans->videoEncoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &pTrans->videoEncoder.stcSettings);

#if 0
    /* Bring up video encoder display */

    pTrans->videoEncoder.display = NEXUS_Display_Open( TRNSX_VideoEncoderDisplay_GetIndex( pCtxt, pTrans->uiIndex), &pTrans->videoEncoder.displaySettings );

    BDBG_ASSERT(pTrans->videoEncoder.display);

#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    {
        NEXUS_DisplayStgSettings stgSettings;

        /* NRT setup - AFAP mode */
        NEXUS_Display_GetStgSettings(pTrans->videoEncoder.display, &stgSettings);
        stgSettings.enabled     = true;
        stgSettings.nonRealTime = pTrans->bNonRealTime;
        NEXUS_Display_SetStgSettings(pTrans->videoEncoder.display , &stgSettings);
    }
#endif

    pTrans->videoEncoder.window = NEXUS_VideoWindow_Open(pTrans->videoEncoder.display, 0);
    BDBG_ASSERT(pTrans->videoEncoder.window);

    if( pTrans->videoEncoder.bCustomDisplaySettings )
    {
        rc = NEXUS_Display_SetCustomFormatSettings(pTrans->videoEncoder.display, NEXUS_VideoFormat_eCustom2, &pTrans->videoEncoder.customFormatSettings);
        BDBG_ASSERT(!rc);
    }

    /*NEXUS_VideoWindow_AddInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));*/

#if SIMUL_DISPLAY
    /* Connect same decoder to encoder display.  This simul mode is for video encoder
     * bringup only; audio path may have limitation for simul display+transcode mode; */
    NEXUS_VideoWindow_AddInput(pCtxt->simulDisplay.window,          NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));
#endif
#endif
    return rc;

}


static NEXUS_Error TRNSX_VideoEncoder_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoEncoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s: the encoder is %s. It must be stopped before you can call close.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoEncoder.eState = TRNSX_State_eClosed;

    if ( pTrans->videoEncoder.window )
    {
        NEXUS_VideoWindow_Close(pTrans->videoEncoder.window);
        pTrans->videoEncoder.window = NULL;
    }

    if ( pTrans->videoEncoder.display )
    {
        NEXUS_Display_Close(pTrans->videoEncoder.display);
        pTrans->videoEncoder.display = NULL;
    }

    if ( pTrans->videoEncoder.videoEncoder )
    {
        NEXUS_VideoEncoder_Close(pTrans->videoEncoder.videoEncoder);
        pTrans->videoEncoder.videoEncoder = NULL;
    }

    if ( pTrans->videoEncoder.stcChannel )
    {
        NEXUS_StcChannel_Close(pTrans->videoEncoder.stcChannel);
        pTrans->videoEncoder.stcChannel = NULL;
    }

    return rc;
}


static NEXUS_Error TRNSX_VideoEncoder_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( strlen( pTrans->record.fname ) == 0 )
    {
        BDBG_WRN(("%s: starting the encoder, an output file has not been specified.", BSTD_FUNCTION ));
    }

    if ( pTrans->videoEncoder.eState != TRNSX_State_eOpened && pTrans->videoEncoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s: the encoder is %s. It must be opened or stopped before you can call start.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    /* Add a check if the encoder has been opened?  Could do an implicit open to make it easier on the user. */

    /* If the encoder display has not been opened, do so now. */
    if ( !pTrans->videoEncoder.display )
    {
        TRNSX_VideoEncoderDisplay_Open( pCtxt, pTrans );
    }

    pTrans->videoEncoder.eState = TRNSX_State_eRunning;

#if 0
    TRNSX_Record_Start( pCtxt, pTrans );
#endif

    pTrans->videoEncoder.startSettings.input = pTrans->videoEncoder.display;
    pTrans->videoEncoder.startSettings.stcChannel = pTrans->videoEncoder.stcChannel;

    /* No guarantee TRNSX_Command_ParseParams will be called, set here.
     * TODO: how to reconcile with user wanting to specify via start settings? */
    pTrans->videoEncoder.startSettings.nonRealTime = pTrans->bNonRealTime;

    /* NOTE: video encoder delay is in 27MHz ticks */
    /* TODO: should encoderDelay be set a init time?*/
    NEXUS_VideoEncoder_GetDelayRange(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings, &pTrans->videoEncoder.startSettings, &videoDelay);
    BKNI_Printf("%s: video encoder end-to-end delay = [%u ~ %u] ms\n", BSTD_FUNCTION, videoDelay.min/27000, videoDelay.max/27000);
    pTrans->videoEncoder.settings.encoderDelay = videoDelay.min;

    /* note the Dee is set by SetSettings, from where? */
    NEXUS_VideoEncoder_SetSettings(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings);
    NEXUS_VideoEncoder_Start(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.startSettings);

    return rc;
}

static NEXUS_Error TRNSX_VideoEncoder_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoEncoder.eState != TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s: the encoder is %s. It must be running before you can call stop.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoEncoder.eState = TRNSX_State_eStopped;

    NEXUS_VideoEncoder_Stop(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.stopSettings);

#if 0
    TRNSX_Record_Stop( pCtxt, pTrans );
#endif

    return rc;

}


static NEXUS_Error TRNSX_VideoEncoder_GetStatus(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_VideoEncoderStatus videoEncodeStatus;
    NEXUS_VideoEncoderClearStatus clearStatus;

    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->videoEncoder.videoEncoder )
    {
        NEXUS_VideoEncoder_GetStatus(pTrans->videoEncoder.videoEncoder, &videoEncodeStatus);
        TRNSX_Print_VideoEncoderStatus( &videoEncodeStatus );
        NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
        NEXUS_VideoEncoder_ClearStatus(pTrans->videoEncoder.videoEncoder, &clearStatus);
    }
    else
    {
        BKNI_Printf("%s: video encoder has not been opened yet\n", BSTD_FUNCTION );
    }

    return rc;
}

/*
 * "System" level wrapper routines.
 */

static NEXUS_Error TRNSX_Transcode_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(TRNSX_Transcode_Open);

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* Error checking.
     * What pTrans->eState are allowed on entry?
     */
    if ( pTrans->eState != TRNSX_State_eClosed &&  pTrans->eState != TRNSX_State_eUninit )
    {
        BKNI_Printf("%s: must be in the closed state before calling open, current state is %s\n", BSTD_FUNCTION, lookup_name( g_stateStrs, pTrans->eState ));
        return NEXUS_NOT_INITIALIZED;
    }

    if ( !strlen(pTrans->input.fname) )
    {
        BKNI_Printf("%s: session:%d must specify a source file\n", BSTD_FUNCTION, pTrans->uiIndex );
        return NEXUS_NOT_INITIALIZED;
    }

    if ( !strlen(pTrans->record.fname) )
    {
        BKNI_Printf("%s: session:%d must specify a results file\n", BSTD_FUNCTION, pTrans->uiIndex );
        return NEXUS_NOT_INITIALIZED;
    }

    if ( !strlen(pTrans->record.indexfname) )
    {
        BKNI_Printf("%s: session:%d must specify a results index file\n", BSTD_FUNCTION, pTrans->uiIndex );
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->eState = TRNSX_State_eOpened; /* protect with a mutex at the end of the routine? */


    TRNSX_VideoDecoder_Open( pCtxt, pTrans );

    TRNSX_VideoEncoder_Open( pCtxt, pTrans );

    TRNSX_VideoEncoderDisplay_Open( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Open);

    return rc;

}

static NEXUS_Error TRNSX_Transcode_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
 {
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    BDBG_ENTER(TRNSX_Transcode_Start);

    if ( pTrans->eState != TRNSX_State_eOpened )
    {
        BKNI_Printf("must be in the open state before calling start, current state is %s\n", lookup_name( g_stateStrs, pTrans->eState ));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->eState = TRNSX_State_eRunning; /* protect with a mutex at the end of the routine? */

    /* Start video decoder. */
    TRNSX_VideoDecoder_Start( pCtxt, pTrans );

    /* Start playback */
    /*NEXUS_Playback_Start(pTrans->playback.playback, pTrans->playback.hFilePlay, NULL);*/
    TRNSX_Playback_Start( pCtxt, pTrans );

    BKNI_Sleep(1000);

    TRNSX_Mux_Start( pCtxt, pTrans );
    TRNSX_Record_Start( pCtxt, pTrans );

    TRNSX_VideoEncoder_Start( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Start);

    return rc;

}

static NEXUS_Error TRNSX_Transcode_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    BDBG_ENTER(TRNSX_Transcode_Stop);

    /* TODO: add "sequence" error checking. */

    if ( pTrans->eState != TRNSX_State_eRunning )
    {
        BKNI_Printf("must be in the running state before calling stop, current state is %s\n", lookup_name( g_stateStrs, pTrans->eState ));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->eState = TRNSX_State_eStopped; /* protect with a mutex at the end of the routine? */

    /*
     * NOTE: stop sequence should be in front->back order
     */
    TRNSX_VideoEncoder_Stop( pCtxt, pTrans );

    TRNSX_Record_Stop( pCtxt, pTrans );
    TRNSX_Mux_Stop( pCtxt, pTrans );

    NEXUS_Playback_Stop(pTrans->playback.playback);

    TRNSX_VideoDecoder_Stop( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Stop);

    return rc;

}

static NEXUS_Error TRNSX_Transcode_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(TRNSX_Transcode_Close);

    /* error checking */

    if ( pTrans->eState != TRNSX_State_eStopped )
    {
        BKNI_Printf("must be in the stopped state before calling close, current state is %s\n", lookup_name( g_stateStrs, pTrans->eState ));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->eState = TRNSX_State_eClosed; /* protect with a mutex at the end of the routine? */

    TRNSX_VideoDecoder_Close( pCtxt, pTrans );

    TRNSX_VideoEncoder_Close( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Close);

    return rc;

}

static void TRNSX_Transcode_Status(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BSTD_UNUSED(pCtxt);

    /* Session state. */
    BKNI_Printf("session: %d\n", pTrans->uiIndex );

    /* Input parameters. */
    BKNI_Printf("input:\n");
    BKNI_Printf("  source type: %s\n", lookup_name(g_sourceStrs, pTrans->input.eSourceType) );
    BKNI_Printf("  source file: %s\n", pTrans->input.fname );
    BKNI_Printf("  stream type: %s\n", lookup_name(g_transportTypeStrs, pTrans->input.eStreamType) );
    BKNI_Printf("  video codec: %s\n", lookup_name(g_videoCodecStrs, pTrans->input.eVideoCodec) );
    BKNI_Printf("  video pid: %d\n", pTrans->input.iVideoPid );
    BKNI_Printf("  PCR pid: %d\n", pTrans->input.iPcrPid );

    /* Ouput parameters */
    BKNI_Printf("output:\n");
    BKNI_Printf("  transport: %s\n", lookup_name( g_transportTypeStrs, pTrans->record.eTransportType));
    BKNI_Printf("  ivf:       %d\n", pTrans->record.bGenerateIVF);
    BKNI_Printf("  file:      %s\n", pTrans->record.fname );

    /* Encoder setttings. */
    BKNI_Printf("encoder:\n");
    BKNI_Printf("  state:%s\n", lookup_name(g_stateStrs, pTrans->videoEncoder.eState ));
    BKNI_Printf("  video format:%s\n", lookup_name(g_videoFormatStrs, pTrans->videoEncoder.displaySettings.format ));

    /* Decoder setttings. */
    BKNI_Printf("video:\n");
    BKNI_Printf("  state:%s\n", lookup_name(g_stateStrs, pTrans->videoDecoder.eState ));

    /* Playback setttings. */
    BKNI_Printf("playback:\n");
    BKNI_Printf("  state:%s\n", lookup_name(g_stateStrs, pTrans->playback.eState ));


    return;

}

static bool TRNSX_Parse_Input(
    char *          pszInput,
    TRSNX_Command * pstCmd,
    uint32_t        uiNumCmds
    )
{
    /*char *  pToken;*/
    char *  pSrc;
    char *  pDst;
    /*TRSNX_Command * pstCmdSaved;*/
    char    szCleaned[TRNSX_SIZE_CMD];
    /*int32_t iSize;*/
    bool    bSuccess=true;
    uint32_t i;
    uint32_t uiState;

    bool bStripWhite;

    BKNI_Memset( pstCmd, 0, sizeof( *pstCmd )*uiNumCmds );
    BKNI_Memset( &szCleaned, 0, sizeof( szCleaned ) );

    pDst = &szCleaned[0];
    pSrc = pszInput;

#if 0
    for ( pSrc = pszInput; *pSrc; pSrc++ )
    {
        if ( *pSrc == '#' )
        {
            /* The beginning of a comment, igonre the rest of the line. */
            break;
        }
        else if ( *pSrc != ' ' && *pSrc != '\n' )
        {
            *pDst++ = tolower(*pSrc);
        }
    }

    pToken = strstr(szCleaned, ":");

    if ( pToken )
    {
        iSize = strcspn( szCleaned, ":" );
        strncpy( pstCmd->szKey, szCleaned, iSize );

        pstCmd->bValid = true;
        strcpy( pstCmd->szValue, pToken+1 );
        pstCmd->iValue = atoi(pstCmd->szValue);
    }
    else
    {
        strncpy( pstCmd->szKey, szCleaned, TRNSX_SIZE_CMD-1 );
    }
#endif

    /*pstCmdSaved = pstCmd;*/

    /* Hitting a CR will dump the settings of the currently active comamnd.
     * Need this special case logic since CR's are filtered
     * in the main parsing loop.*/

    if( *pszInput == '\n' )
    {
        strcpy( pstCmd->szKey, "CR" );
        pstCmd->bValid = true;
        goto done;
    }

    /* A clunky way to keep track of state. */
#define SEARCH_FOR_KEY      0x1
#define LOAD_KEY            0x2
#define SEARCH_FOR_VALUE    0x4
#define LOAD_VALUE          0x8

    uiState = SEARCH_FOR_KEY;
    bStripWhite = false;

    for ( pSrc=pszInput, i=0; *pSrc && i < uiNumCmds-1 ; i++ )
    {

        /*TODO: where to add error checking for syntax error on the input. */

        if ( *pSrc == ':' )
        {
            pSrc++;
            uiState     = SEARCH_FOR_VALUE;
            bStripWhite = false;
        }
        else if ( *pSrc == '#' || *pSrc == '\n' )
        {
            /* The beginning of a comment or the end of the input. */

            /* Need to check for zero length, i.e the case of "key:\n"*/
            if (( strlen( pstCmd->szValue )  != 0 ) && ( uiState == LOAD_VALUE ))
            {
                pstCmd->bValid = true;
                /*pstCmd->iValue = atoi(pstCmd->szValue);*/
                pstCmd->iValue = strtoul(pstCmd->szValue, NULL, 0);
            }
            break;
        }
        else if ( *pSrc == ' ' || *pSrc == '\t' || *pSrc == '\r' )
        {
            pSrc++;

            if ( uiState == LOAD_VALUE )
            {
                /* If white space while loading a value, the end of the value has been reached. */

                pstCmd->bValid = true;
                /*pstCmd->iValue = atoi(pstCmd->szValue);*/
                pstCmd->iValue = strtoul(pstCmd->szValue, NULL, 0);
                pstCmd++;

                uiState     = SEARCH_FOR_KEY;
                bStripWhite = false;
            }
            else
            {
                /* If white space while searching for a key or value or loading a key, flag
                 * that white space is being stripped but don't change state. */

                bStripWhite = true;
            }
        }
        else
        {
            bool bFileName = false;

            if ( uiState == SEARCH_FOR_KEY )
            {
                pDst        = &pstCmd->szKey[0];
                uiState     = LOAD_KEY;
            }
            else if ( uiState == SEARCH_FOR_VALUE )
            {
                pDst        = &pstCmd->szValue[0];
                uiState     = LOAD_VALUE;
            }
            else if ( uiState == LOAD_KEY && bStripWhite == true )
            {
                /* The next key in a key/key sequence, i.e. not ':value'. */
                pstCmd++;

                pDst        = &pstCmd->szKey[0];
                uiState     = LOAD_VALUE;
            }

            bStripWhite = false;

            if ( uiState == LOAD_VALUE )
            {
                /* Don't change the case of file names. */
                bFileName = !strcmp(pstCmd->szKey, "file");
                bFileName |= !strcmp(pstCmd->szKey, "ifile");
                bFileName |= !strcmp(pstCmd->szKey, "ofile");
            }

            *pDst++ = ( bFileName ) ? *pSrc++ : tolower(*pSrc++);

        }



    }

done:

#if 0
    for ( pstCmd = pstCmdSaved, i=0; ( strlen(pstCmd->szKey) !=0 ) && i < uiNumCmds-1 ; pstCmd++, i++ )
    {
        BKNI_Printf("%s::%s (%d) %s %d\n", BSTD_FUNCTION, pstCmd->szKey, pstCmd->bValid, pstCmd->szValue, pstCmd->iValue );
    }
#endif
#if 0
    /* check if the last command had the proper syntax.
     * Ignore issues with the quit command. */
    if ( ( strlen(pstCmd->szKey) !=0 && pstCmd->bValid == false )
          && (strcmp(pstCmd->szKey, "quit") && strcmp(pstCmd->szKey, "q")) )
    {
        /*BKNI_Printf("error::%s (%d) %s %d\n", pstCmd->szKey, pstCmd->bValid, pstCmd->szValue, pstCmd->iValue );*/
        bSuccess = false;
        BDBG_ERR(("%s: bad syntax: %s", BSTD_FUNCTION, pszInput ));
        BDBG_ERR(("%s: commands need to be of the form key:value", BSTD_FUNCTION ));
        BKNI_Memset( pstCmd, 0, sizeof( *pstCmd ));
    }
#endif
    return bSuccess;
}

/*
 * Execute the active command.
 */
static void TRNSX_Command_Execute(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans,
    TRSNX_Command * pstCmd
    )
{
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);
    BDBG_ASSERT(pstCmd);

    BDBG_MODULE_MSG(trnsx_cmds,("%s: executing %s", BSTD_FUNCTION, lookup_name(g_cmdStrs, pTrans->eActiveCmd )));

    switch( pTrans->eActiveCmd )
    {
        /*** encoder ***/
        case TRNSX_Cmd_eVideoEncoderSetSettings:    TRNSX_VideoEncoder_SetSettings( pCtxt, pTrans );   break;
        case TRNSX_Cmd_eVideoEncoderOpen:           TRNSX_VideoEncoder_Open( pCtxt, pTrans );          break;
        case TRNSX_Cmd_eVideoEncoderClose:          TRNSX_VideoEncoder_Close( pCtxt, pTrans );         break;
        case TRNSX_Cmd_eVideoEncoderStart:          TRNSX_VideoEncoder_Start( pCtxt, pTrans );         break;
        case TRNSX_Cmd_eVideoEncoderStop:           TRNSX_VideoEncoder_Stop( pCtxt, pTrans );          break;
        case TRNSX_Cmd_eVideoEncoderStatus:         TRNSX_VideoEncoder_GetStatus( pCtxt, pTrans );     break;

        /*** playpump ***/
        case TRNSX_Cmd_ePlaypumpSetSettings:
        case TRNSX_Cmd_ePlaypumpOpen:
            /* These parameters are used in TRNSX_StreamMux_Open, the associated Nexus call will be made there. */
            break;

        /*** display ***/
        case TRNSX_Cmd_eDisplayClose:
        case TRNSX_Cmd_eDisplayOpen:
            /* TODO: nothing to do since the call to Display_Open is in a wrapper.
             * If the display calls are exposed, they will need to be called here. */
            break;

        case TRNSX_Cmd_eDisplayCustomFormatSettings:
            /* called in TRNSX_VideoEncoder_Open. */
            break;

        /*** playback ***/
        case TRNSX_Cmd_ePlaybackSetSettings:    break;
        case TRNSX_Cmd_ePlaybackOpen:       break;
        case TRNSX_Cmd_ePlaybackClose:      break;
        case TRNSX_Cmd_ePlaybackStart:      TRNSX_Playback_Start( pCtxt, pTrans );          break;
        case TRNSX_Cmd_ePlaybackStop:       NEXUS_Playback_Stop(pTrans->playback.playback); break;

        /*** decoder ***/
        case TRNSX_Cmd_eVideoDecoderSetSettings:    break;

        case TRNSX_Cmd_eVideoDecoderOpen:   TRNSX_VideoDecoder_Open( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eVideoDecoderClose:  TRNSX_VideoDecoder_Close( pCtxt, pTrans );     break;
        case TRNSX_Cmd_eVideoDecoderStart:  TRNSX_VideoDecoder_Start( pCtxt, pTrans );     break;
        case TRNSX_Cmd_eVideoDecoderStop:   TRNSX_VideoDecoder_Stop( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eVideoDecoderStatus: TRNSX_VideoDecoder_GetStatus( pCtxt, pTrans ); break;

        case TRNSX_Cmd_eSession:    break;
        case TRNSX_Cmd_eTranscode:  break;

        case TRNSX_Cmd_eRecordSettings:     break;
        case TRNSX_Cmd_eRecordStart:    TRNSX_Record_Start( pCtxt, pTrans );    break;
        case TRNSX_Cmd_eRecordStop:     TRNSX_Record_Stop( pCtxt, pTrans );     break;
        case TRNSX_Cmd_eRecordPause:    TRNSX_Record_Pause( pCtxt, pTrans );    break;
        case TRNSX_Cmd_eRecordResume:   TRNSX_Record_Resume( pCtxt, pTrans );    break;

        case TRNSX_Cmd_eMuxOpen:        break;
        case TRNSX_Cmd_eMuxClose:       break;
        case TRNSX_Cmd_eMuxStart:       TRNSX_Mux_Start( pCtxt, pTrans );   break;
        case TRNSX_Cmd_eMuxStop:        TRNSX_Mux_Stop( pCtxt, pTrans );    break;

        case TRNSX_Cmd_eSleep:
            BKNI_Sleep( (pTrans->sleep.eUnit == TRNSX_SleepUnit_eSecs) ? pTrans->sleep.uiDuration * 1000 : pTrans->sleep.uiDuration );
            break;

        default:
             BKNI_Printf("%s: hit default, support has not been added for command %d\n", BSTD_FUNCTION, pTrans->eActiveCmd );
             break;
    }

    pTrans->eActiveCmd = TRNSX_Cmd_eNone;

    return;

}

/*
 * Set the parameters associated with the active command.
 */
static bool TRNSX_Command_ParseParams(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans,
    TRSNX_Command * pstCmd
    )
{
    bool bRet = true;
    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);
    BDBG_ASSERT(pstCmd);

    bool bPrint = !strcmp(pstCmd->szKey, "CR");
    bPrint &= pCtxt->bInteractive;

    switch( pTrans->eActiveCmd )
    {
        case TRNSX_Cmd_eVideoEncoderSetSettings:
            if ( bPrint ) {
                TRNSX_Print_VideoEncoderSetSettings( &pTrans->videoEncoder.settings );
            }
            TRNSX_ELSE_IF( pstCmd, "bitratemax", pTrans->videoEncoder.settings.bitrateMax )
            TRNSX_ELSE_IF( pstCmd, "bitratetarget", pTrans->videoEncoder.settings.bitrateTarget )
            TRNSX_ELSE_IF( pstCmd, "variableframerate", pTrans->videoEncoder.settings.variableFrameRate )
            TRNSX_ELSE_IF( pstCmd, "sparseframerate", pTrans->videoEncoder.settings.sparseFrameRate )
            TRNSX_ELSE_IF( pstCmd, "enablefieldpairing", pTrans->videoEncoder.settings.enableFieldPairing )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "framerate", pTrans->videoEncoder.settings.frameRate, g_videoFrameRateStrs )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.newgoponscenechange", pTrans->videoEncoder.settings.streamStructure.newGopOnSceneChange )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.duration", pTrans->videoEncoder.settings.streamStructure.duration )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.adaptiveduration", pTrans->videoEncoder.settings.streamStructure.adaptiveDuration )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.framesp", pTrans->videoEncoder.settings.streamStructure.framesP )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.framesb", pTrans->videoEncoder.settings.streamStructure.framesB )
            TRNSX_ELSE_IF( pstCmd, "streamstructure.opengop", pTrans->videoEncoder.settings.streamStructure.openGop )
            TRNSX_ELSE_IF( pstCmd, "encoderdelay", pTrans->videoEncoder.settings.encoderDelay )
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoEncoderOpen:
            if ( bPrint ) {
                TRNSX_Print_VideoEncoderOpenSettings( &pTrans->videoEncoder.openSettings );
            }
            TRNSX_ELSE_IF( pstCmd, "data.fifosize", pTrans->videoEncoder.openSettings.data.fifoSize )
            TRNSX_ELSE_IF( pstCmd, "index.fifosize", pTrans->videoEncoder.openSettings.index.fifoSize )
            TRNSX_ELSE_IF( pstCmd, "memoryconfig.interlaced", pTrans->videoEncoder.openSettings.memoryConfig.interlaced )
            TRNSX_ELSE_IF( pstCmd, "memoryconfig.maxwidth", pTrans->videoEncoder.openSettings.memoryConfig.maxWidth )
            TRNSX_ELSE_IF( pstCmd, "memoryconfig.maxheight", pTrans->videoEncoder.openSettings.memoryConfig.maxHeight )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "type", pTrans->videoEncoder.openSettings.type, g_encoderTypeStrs )
            TRNSX_ELSE_IF( pstCmd, "maxchannelcount", pTrans->videoEncoder.openSettings.maxChannelCount )
            TRNSX_ELSE_IF( pstCmd, "enabledataunitdetecton", pTrans->videoEncoder.openSettings.enableDataUnitDetecton)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoEncoderStatus: break;
        case TRNSX_Cmd_eVideoEncoderClose:
            if ( bPrint ) PARAM_MSG;
            break;

        case TRNSX_Cmd_eVideoEncoderStart:
            /* Set nonRealTime based on the session value. */
            pTrans->videoEncoder.startSettings.nonRealTime = pTrans->bNonRealTime;

            /* Set encodeUserData based on the session value. */
            pTrans->videoEncoder.startSettings.encodeUserData = pTrans->bEncodeCCData;

            if ( bPrint ) {
                TRNSX_Print_VideoEncoderStartSettings( &pTrans->videoEncoder.startSettings );
            }
            TRNSX_ELSE_IF( pstCmd, "interlaced", pTrans->videoEncoder.startSettings.interlaced)
            TRNSX_ELSE_IF( pstCmd, "nonrealtime", pTrans->videoEncoder.startSettings.nonRealTime)
            TRNSX_ELSE_IF( pstCmd, "lowdelaypipeline", pTrans->videoEncoder.startSettings.lowDelayPipeline)
            TRNSX_ELSE_IF( pstCmd, "encodeuserdata", pTrans->videoEncoder.startSettings.encodeUserData)
            TRNSX_ELSE_IF( pstCmd, "encodebaruserdata", pTrans->videoEncoder.startSettings.encodeBarUserData)
            TRNSX_ELSE_IF( pstCmd, "adaptivelowdelaymode", pTrans->videoEncoder.startSettings.adaptiveLowDelayMode)

            TRNSX_ELSE_IF_LOOKUP( pstCmd, "codec", pTrans->videoEncoder.startSettings.codec, g_videoCodecStrs )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "profile", pTrans->videoEncoder.startSettings.profile ,g_videoCodecProfileStrs )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "level", pTrans->videoEncoder.startSettings.level, g_videoCodecLevelStrs)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "bounds.outputframerate.min", pTrans->videoEncoder.startSettings.bounds.outputFrameRate.min, g_videoFrameRateStrs)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "bounds.outputframerate.max", pTrans->videoEncoder.startSettings.bounds.outputFrameRate.max, g_videoFrameRateStrs)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "bounds.inputframerate.min", pTrans->videoEncoder.startSettings.bounds.inputFrameRate.min, g_videoFrameRateStrs)

            TRNSX_ELSE_IF( pstCmd, "bounds.inputdimension.max.width", pTrans->videoEncoder.startSettings.bounds.inputDimension.max.width)
            TRNSX_ELSE_IF( pstCmd, "bounds.inputdimension.max.height", pTrans->videoEncoder.startSettings.bounds.inputDimension.max.height)
            TRNSX_ELSE_IF( pstCmd, "bounds.inputdimension.maxinterlaced.width", pTrans->videoEncoder.startSettings.bounds.inputDimension.maxInterlaced.width)
            TRNSX_ELSE_IF( pstCmd, "bounds.inputdimension.maxinterlaced.height", pTrans->videoEncoder.startSettings.bounds.inputDimension.maxInterlaced.height)
            TRNSX_ELSE_IF( pstCmd, "bounds.bitrate.upper.bitratemax", pTrans->videoEncoder.startSettings.bounds.bitrate.upper.bitrateMax)
            TRNSX_ELSE_IF( pstCmd, "bounds.bitrate.upper.bitratetarget", pTrans->videoEncoder.startSettings.bounds.bitrate.upper.bitrateTarget)
            TRNSX_ELSE_IF( pstCmd, "bounds.streamstructure.max.framesp", pTrans->videoEncoder.startSettings.bounds.streamStructure.max.framesP)
            TRNSX_ELSE_IF( pstCmd, "bounds.streamstructure.max.framesb", pTrans->videoEncoder.startSettings.bounds.streamStructure.max.framesB)

            TRNSX_ELSE_IF( pstCmd, "ratebufferdelay", pTrans->videoEncoder.startSettings.rateBufferDelay)
            TRNSX_ELSE_IF( pstCmd, "numparallelencodes", pTrans->videoEncoder.startSettings.numParallelEncodes)
            TRNSX_ELSE_IF( pstCmd, "bypassvideoprocessing", pTrans->videoEncoder.startSettings.bypassVideoProcessing)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "entropycoding", pTrans->videoEncoder.startSettings.entropyCoding, g_entropyCodingStrs)
            TRNSX_ELSE_IF( pstCmd, "hrdmoderatecontrol.disableframedrop", pTrans->videoEncoder.startSettings.hrdModeRateControl.disableFrameDrop)
            TRNSX_ELSE_IF( pstCmd, "segmentmoderatecontrol.enable", pTrans->videoEncoder.startSettings.segmentModeRateControl.enable)
            TRNSX_ELSE_IF( pstCmd, "segmentmoderatecontrol.duration", pTrans->videoEncoder.startSettings.segmentModeRateControl.duration)
            TRNSX_ELSE_IF( pstCmd, "segmentmoderatecontrol.uppertolerance", pTrans->videoEncoder.startSettings.segmentModeRateControl.upperTolerance)
            TRNSX_ELSE_IF( pstCmd, "segmentmoderatecontrol.lowertolerance", pTrans->videoEncoder.startSettings.segmentModeRateControl.lowerTolerance)
            TRNSX_ELSE_IF( pstCmd, "memorybandwidthsaving.singlerefp", pTrans->videoEncoder.startSettings.memoryBandwidthSaving.singleRefP)
            TRNSX_ELSE_IF( pstCmd, "memorybandwidthsaving.requiredratchesonly", pTrans->videoEncoder.startSettings.memoryBandwidthSaving.requiredPatchesOnly)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoEncoderStop:
            if ( bPrint ) {
                TRNSX_Print_VideoEncoderStopSettings( &pTrans->videoEncoder.stopSettings );
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "mode", pTrans->videoEncoder.stopSettings.mode, g_encoderStopModeStrs)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eDisplayClose: break;
        case TRNSX_Cmd_eDisplayOpen:
            if ( bPrint ) {
                TRNSX_Print_DisplaySettings( &pTrans->videoEncoder.displaySettings );
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "displaytype", pTrans->videoEncoder.displaySettings.displayType, g_displayTypeStrs)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "timinggenerator", pTrans->videoEncoder.displaySettings.timingGenerator, g_displayTimingGeneratorStrs)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "format", pTrans->videoEncoder.displaySettings.format, g_videoFormatStrs )
            TRNSX_ELSE_IF( pstCmd, "sampleaspectratio.x", pTrans->videoEncoder.displaySettings.sampleAspectRatio.x)
            TRNSX_ELSE_IF( pstCmd, "sampleaspectratio.y", pTrans->videoEncoder.displaySettings.sampleAspectRatio.y)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "dropframe", pTrans->videoEncoder.displaySettings.dropFrame, g_tristateEnableStrs )
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eDisplayCustomFormatSettings:
            if ( bPrint ) {
                TRNSX_Print_DisplayCustomFormatSettings( pTrans );
            }
            TRNSX_ELSE_IF( pstCmd, "bcustom", pTrans->videoEncoder.bCustomDisplaySettings)
            TRNSX_ELSE_IF( pstCmd, "width", pTrans->videoEncoder.customFormatSettings.width)
            TRNSX_ELSE_IF( pstCmd, "height", pTrans->videoEncoder.customFormatSettings.height)
            TRNSX_ELSE_IF( pstCmd, "interlaced", pTrans->videoEncoder.customFormatSettings.interlaced)
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "aspectratio", pTrans->videoEncoder.customFormatSettings.aspectRatio, g_displayAspectRatioStrs ) /*NEXUS_DisplayAspectRatio*/
            TRNSX_ELSE_IF( pstCmd, "sampleaspectratio.x", pTrans->videoEncoder.customFormatSettings.sampleAspectRatio.x)
            TRNSX_ELSE_IF( pstCmd, "sampleaspectratio.y", pTrans->videoEncoder.customFormatSettings.sampleAspectRatio.y)
            TRNSX_ELSE_IF( pstCmd, "dropframeallowed", pTrans->videoEncoder.customFormatSettings.dropFrameAllowed)
            else if (!strcmp(pstCmd->szKey, "refreshrate" ))
            {
                if ( pstCmd->bValid )
                {
                    pTrans->videoEncoder.customFormatSettings.refreshRate = pstCmd->iValue;
                    if ( pstCmd->iValue < 1 * 1000 || pstCmd->iValue > 240 * 1000 )
                    {
                        BDBG_WRN(("Is this value correct? refreshRate is specified in units of in 1/1000th Hz"));
                        BDBG_WRN(("Was expecting something like 60*1000."));
                    }
                }
                else
                    BKNI_Printf("  current value for %s: %d\n", pstCmd->szKey, pTrans->videoEncoder.customFormatSettings.refreshRate );
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoDecoderStart:
            /* Set nonRealTime based on the session value. */
            pTrans->videoDecoder.startSettings.nonRealTime = pTrans->bNonRealTime;
            if ( bPrint ) PARAM_MSG;
            break;

        case TRNSX_Cmd_ePlaypumpSetSettings:
            if ( bPrint ) {
                BKNI_Printf("  transport:  %s\n", lookup_name( g_transportTypeStrs, pTrans->streamMux.playpumpSettings.transportType ));
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "transport", pTrans->streamMux.playpumpSettings.transportType, g_transportTypeStrs)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

            break;
        case TRNSX_Cmd_ePlaypumpOpen:
            if ( bPrint ) {
                TRNSX_Print_PlaypumpOpenSettings( &pTrans->streamMux.playpumpOpenSettings );
            }
            TRNSX_ELSE_IF( pstCmd, "fifoSize", pTrans->streamMux.playpumpOpenSettings.fifoSize)
            TRNSX_ELSE_IF( pstCmd, "numdescriptors", pTrans->streamMux.playpumpOpenSettings.numDescriptors )
            TRNSX_ELSE_IF( pstCmd, "streammuxcompatible", pTrans->streamMux.playpumpOpenSettings.streamMuxCompatible)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoDecoderSetSettings:
        case TRNSX_Cmd_eVideoDecoderOpen:
        case TRNSX_Cmd_eVideoDecoderClose:
        case TRNSX_Cmd_eVideoDecoderStop:
            if ( bPrint ) PARAM_MSG;
            break;

        case TRNSX_Cmd_eVideoDecoderStatus: break;

        case TRNSX_Cmd_eSession:
            if ( bPrint ) {
                TRNSX_Print_SessionSettings( pCtxt, pTrans );
            }
            else if (!strcmp(pstCmd->szKey, "select")) {
                if ( pstCmd->bValid == false )
                {
                    BKNI_Printf("session %d is selected\n", pCtxt->uiSelected );
                }
                else
                {
                    if ( pstCmd->iValue == 0 )
                    {
                        pCtxt->uiSelected = pstCmd->iValue;
                        BKNI_Printf("selecting session:%d\n", pstCmd->iValue );
                    }
                    else if ( pstCmd->iValue < TRNSX_NUM_ENCODES-1)
                    {
                        BKNI_Printf(" currently only support session 0\n");
                    }
                    else
                    {
                        BKNI_Printf("Session selection value %d is out of range, it should be less than %d\n", pstCmd->iValue, TRNSX_NUM_ENCODES );
                    }
                }
            }
            TRNSX_ELSE_IF( pstCmd, "nrt", pTrans->bNonRealTime)
            TRNSX_ELSE_IF( pstCmd, "cc", pTrans->bEncodeCCData)
            TRNSX_ELSE_IF_LOOKUP(pstCmd, "tts_out", pTrans->streamMux.transportTimestampType, g_transportTimestampTypeStrs )
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eTranscode:
            /* TODO: these should be called in TRNSX_Commmand_Execute. */
            if ( pstCmd->bValid && !strcmp(pstCmd->szKey, "action"))
            {
                if (!strcmp(pstCmd->szValue, "open")) {
                    /* Will this will need to change based on the source type. */
                    TRNSX_Transcode_Open( pCtxt, pTrans );
                }
                else if (!strcmp(pstCmd->szValue, "start")) {
                    TRNSX_Transcode_Start( pCtxt, pTrans );
                }
                else if (!strcmp(pstCmd->szValue, "stop")) {
                    TRNSX_Transcode_Stop( pCtxt, pTrans );
                }
                else if (!strcmp(pstCmd->szValue, "close")) {
                    TRNSX_Transcode_Close( pCtxt, pTrans );
                }
                else {
                    BKNI_Printf("unknown transcode operation: %s\n", pstCmd->szValue );
                    bRet = false;
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eMuxStart:
            switch( pTrans->record.eTransportType )
            {
                case NEXUS_TransportType_eEs:
                    if ( bPrint ) {
                        BKNI_Printf("transport type of %s does not support any start settings\n", lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) );
                    }
                    break;

                /* File Mux */
                case NEXUS_TransportType_eMpeg2Pes:
                case NEXUS_TransportType_eMp4:
                    if ( bPrint ) {
                        BKNI_Printf("transport type of %s does not support any start settings\n", lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) );
                    }
                    break;

                /* Stream Mux */
                case NEXUS_TransportType_eTs:
                case NEXUS_TransportType_eMp4Fragment:
                    if ( bPrint ) {
                        TRNSX_Print_StreamMuxStartSettings( &pTrans->streamMux.startSettings );
                    }
                    TRNSX_ELSE_IF_LOOKUP( pstCmd, "transporttype", pTrans->streamMux.startSettings.transportType, g_transportTypeStrs ) /* NEXUS_TransportType */
                    /* The global value for the following isn't applied until mux_open is called, how to reconcile?*/
                    /*TRNSX_ELSE_IF_LOOKUP(pstCmd, "supportTts", pTrans->streamMux.startSettings.supportTts, g_transportTimestampTypeStrs )*/
                    TRNSX_ELSE_IF( pstCmd, "nonRealTimeRate", pTrans->streamMux.startSettings.nonRealTimeRate)
                    TRNSX_ELSE_IF( pstCmd, "servicePeriod", pTrans->streamMux.startSettings.servicePeriod)
                    TRNSX_ELSE_IF( pstCmd, "latencyTolerance", pTrans->streamMux.startSettings.latencyTolerance)
                    TRNSX_ELSE_IF_LOOKUP( pstCmd, "interleaveMode", pTrans->streamMux.startSettings.interleaveMode,g_streamMuxInterleaveModeStrs) /* NEXUS_StreamMuxInterleaveMode*/
                    TRNSX_ELSE_IF( pstCmd, "useInitialPts", pTrans->streamMux.startSettings.useInitialPts)
                    TRNSX_ELSE_IF( pstCmd, "initialPts", pTrans->streamMux.startSettings.initialPts)
                    TRNSX_ELSE_IF( pstCmd, "pcr.interval", pTrans->streamMux.startSettings.pcr.interval)
                    TRNSX_ELSE_IF( pstCmd, "insertPtsOnlyOnFirstKeyFrameOfSegment", pTrans->streamMux.startSettings.insertPtsOnlyOnFirstKeyFrameOfSegment)
                    TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
                    break;

                default:
                    BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
                    break;

            }
            break;

        case TRNSX_Cmd_eMuxStop:
        case TRNSX_Cmd_eMuxClose:
            break;

        case TRNSX_Cmd_eMuxOpen:
            if ( bPrint ) {
                TRNSX_Print_MuxSettings( pTrans );
            }
            else if (!strcmp(pstCmd->szKey, "transport"))
            {
                if ( pstCmd->bValid )
                {
                    bool bFileMuxError = false;
                    bool bStreamMuxError = false;
                    TRNSX_TransportType eTransportType = lookup(g_trnsxTransportTypeStrs,pstCmd->szValue);

                    pTrans->fileMux.bActive = false;
                    pTrans->streamMux.bActive = false;
                    pTrans->record.bGenerateIVF = false;

                    /* error checking
                     * TODO: when using a script file, need to bomb out. */
                    bFileMuxError = ( eTransportType == TRNSX_TransportType_eIVF );
                    bFileMuxError |= ( eTransportType == TRNSX_TransportType_ePES );
                    bFileMuxError |= ( eTransportType == TRNSX_TransportType_eMp4 );
                    bFileMuxError &= !pTrans->fileMux.bSupported;

                    bStreamMuxError |= ( eTransportType == TRNSX_TransportType_eTS );
                    bStreamMuxError |= ( eTransportType == TRNSX_TransportType_eMp4Fragmented );
                    bStreamMuxError &= !pTrans->streamMux.bSupported;

                    if ( bFileMuxError || bStreamMuxError )
                    {
                        pTrans->record.eTransportType = NEXUS_TransportType_eEs;
                        BDBG_ERR(("%s mux is not supported, defaulting to NEXUS_TransportType_eEs", (bFileMuxError) ? "File" : "Stream" ));
                    }
                    else
                    {
                        switch( eTransportType )
                        {
                            case TRNSX_TransportType_eEs:
                                pTrans->record.eTransportType = NEXUS_TransportType_eEs;
                                break;

                            case TRNSX_TransportType_eIVF:
                                pTrans->fileMux.bActive = true;
                                pTrans->record.eTransportType = NEXUS_TransportType_eMpeg2Pes;
                                pTrans->record.bGenerateIVF = true;
                                break;

                            case TRNSX_TransportType_ePES:
                                pTrans->fileMux.bActive = true;
                                pTrans->record.eTransportType = NEXUS_TransportType_eMpeg2Pes;
                                break;

                            case TRNSX_TransportType_eMp4:
                                pTrans->fileMux.bActive = true;
                                pTrans->record.eTransportType = NEXUS_TransportType_eMp4;
                                break;

                            case TRNSX_TransportType_eTS:
                                pTrans->streamMux.bActive = true;
                                pTrans->record.eTransportType = NEXUS_TransportType_eTs;
                                break;

                             case TRNSX_TransportType_eMp4Fragmented:
                                pTrans->streamMux.bActive = true;
                                pTrans->record.eTransportType = NEXUS_TransportType_eMp4Fragment;
                                break;

                            default:
                                BDBG_ERR(("Transport type %s is not supported defaulting to NEXUS_TransportType_eEs", lookup_name(g_transportTypeStrs,eTransportType)));
                                pTrans->record.eTransportType = NEXUS_TransportType_eEs;
                                break;

                        }
                    }

                }
                else
                {
                    BKNI_Printf("  current value for %s: %s\n", pstCmd->szKey, lookup_name(g_transportTypeStrs,pTrans->record.eTransportType ));
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eRecordStart:
        case TRNSX_Cmd_eRecordPause:
        case TRNSX_Cmd_eRecordResume:
            break;

        case TRNSX_Cmd_eRecordStop:
            if ( bPrint ) {
                BKNI_Printf("  bWaitForEos: %d\n", pTrans->record.bWaitForEos);
            }
            TRNSX_ELSE_IF( pstCmd, "bwaitforeos", pTrans->record.bWaitForEos)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eRecordSettings:
            if ( bPrint ) {
                TRNSX_Print_RecordSettings( pTrans );
            }
            else if (!strcmp(pstCmd->szKey, "file"))
            {
                if ( pstCmd->bValid == false )
                {
                    /*BKNI_Printf("%s: failed to specify a file\n", pstCmd->szKey );*/
                    BKNI_Memset( pTrans->record.fname, 0, sizeof(pTrans->record.fname) );
                    BKNI_Memset( pTrans->record.indexfname, 0, sizeof(pTrans->record.fname) );
                }
                else
                {
                    char * pToken;

                    /* TODO: close old file if opening a new one? */

                    strncpy(pTrans->record.fname, pstCmd->szValue, TRNSX_SIZE_FILE_NAME-1);

                    /* Create the index file name.
                     * TODO: this should only occur for the correct file types */
                    BKNI_Memset(pTrans->record.indexfname, 0, sizeof(pTrans->record.indexfname));

                    pToken = strstr(pTrans->record.fname, ".");

                    if ( pToken )
                    {
                        int32_t iSize;
                        iSize = strcspn( pTrans->record.fname, "." );
                        strncpy( pTrans->record.indexfname, pTrans->record.fname, iSize );
                        strncat(pTrans->record.indexfname, ".nav", sizeof(pTrans->record.indexfname) - strlen(pTrans->record.indexfname) - 1);
                    }
                    else
                    {
                        strncpy( pTrans->record.indexfname, pTrans->record.fname, strlen(pTrans->record.fname) );
                        strncat(pTrans->record.indexfname, ".nav", sizeof(pTrans->record.indexfname) - strlen(pTrans->record.indexfname) - 1);
                    }

                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_ePlaybackSetSettings:
        case TRNSX_Cmd_ePlaybackStart:
        case TRNSX_Cmd_ePlaybackStop:
        case TRNSX_Cmd_ePlaybackClose:
            if ( bPrint ) PARAM_MSG;
            break;

        case TRNSX_Cmd_ePlaybackOpen:
            if ( bPrint ) {
                TRNSX_Print_PlaybackOpenSettings( pTrans );
            }
            else if ( (pstCmd->bValid == true ) && !strcmp(pstCmd->szKey, "type"))
            {
                if (!strcmp(pstCmd->szValue, "file"))
                {
                    pTrans->input.eSourceType = TRNSX_Source_eFile;
                }
                else
                {
                    BKNI_Printf("%s: currently only support the default of file:\n", BSTD_FUNCTION );
                    pTrans->input.eSourceType = TRNSX_Source_eFile;
                    /* TODO: need to add error handling, when reading from a command file, exit */
                }
            }
            else if (!strcmp(pstCmd->szKey, "file"))
            {
                if ( pstCmd->bValid == false )
                {
                    BKNI_Printf("%s: failed to specify a file\n", pstCmd->szKey );
                }
                else
                {
                    TRNSX_MediaProbe(pTrans, pstCmd->szValue);

                    if ( pTrans->input.bFileIsValid == false )
                    {
                        BKNI_Printf("%s: failed to open %s\n", pstCmd->szKey, pstCmd->szValue );
                    }

                }
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "video_codec",pTrans->input.eVideoCodec, g_videoCodecStrs )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "transport",pTrans->input.eStreamType, g_transportTypeStrs )
            TRNSX_ELSE_IF( pstCmd, "video_pid", pTrans->input.iVideoPid)
            TRNSX_ELSE_IF( pstCmd, "pcr_pid", pTrans->input.iPcrPid)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        case TRNSX_Cmd_eSleep:
            if ( bPrint ) {
                BKNI_Printf("  unit:     %s\n", lookup_name(g_sleepUnitStrs, pTrans->sleep.eUnit));
                BKNI_Printf("  duration: %d\n", pTrans->sleep.uiDuration);
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "unit",pTrans->sleep.eUnit, g_sleepUnitStrs )
            TRNSX_ELSE_IF( pstCmd, "duration", pTrans->sleep.uiDuration)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdStrs, bRet )
            break;

        default:
            BKNI_Printf("%s: hit default for pTrans->eActiveCmd.  Command %d is undefined.\n", BSTD_FUNCTION, pTrans->eActiveCmd );
            bRet = false;
            break;
    }


    return bRet;
}

static void TRNSX_Help(void)
{
    /* Can operate in two modes, calling each Nexus command or use defaults, set parameters of interest,
     * then use wrapper routines to make Nexus calls. */

#if 0
    BKNI_Printf(
        "session - prints the state of all the sessions\n"
        "session:<n> - selects session<n>.  All subsequent commands apply to that session.\n"
        "session:open - creates a session with the default settings\n"
        "session:start - begins playback and encoding\n"
        "session:stop -  stops playback and encoding\n"
        "session:close - closes the session\n"
        "session:status - prints the sessions' status\n\n"
        );
#endif

    BKNI_Printf(
        "-- session --\n"
        "session <parameters are below> end:0\n"
        "  - select:<n> : specify which sesssion is active, currently only '0' can be selected\n"
        "  - nrt:<0,1>  : enable NRT mode. A global flag which must be set before any of the commands below.\n"
        "  - cc:<0,1>   : encode CC user data. A global flag which must be set before any of the commands below.\n\n"
        );


    BKNI_Printf(
        "-- encoder --\n"
        "video_encoder_set_settings (vess) -> NEXUS_VideoEncoder_SetSettings( NEXUS_VideoEncoderSettings )\n"
        "video_encoder_open (veopen)       -> NEXUS_VideoEncoder_Open( NEXUS_VideoEncoderOpenSettings )\n"
        );

    BKNI_Printf(
        "video_encoder_close (veclose)    -> NEXUS_VideoEncoder_Close()\n"
        "video_encoder_start (vestart)    -> NEXUS_VideoEncoder_Start( NEXUS_VideoEncoderStartSettings )\n"
        "video_encoder_stop (vestop)      -> NEXUS_VideoEncoder_Stop( NEXUS_VideoEncoderStopSettings )\n"
        "video_encoder_status (vestatus)  -> NEXUS_VideoEncoder_GetStatus()\n\n"
        );

    BKNI_Printf(
        "-- decoder --\n"
        "video_decoder_set_settings (vdss) -> NEXUS_VideoDecoder_SetSettings( NEXUS_VideoDecoderSettings )\n"
        "video_decoder_open (vdopen)       -> NEXUS_VideoDecoder_Open( NEXUS_VideoDecoderOpenSettings )\n"
        );

    BKNI_Printf(
        "video_decoder_close (vdclose)    -> NEXUS_VideoDecoder_Close()\n"
        "video_decoder_start (vdstart)    -> NEXUS_VideoDecoder_Start( NEXUS_VideoDecoderStartSettings )\n"
        "video_decoder_stop (vdstop)      -> NEXUS_VideoDecoder_Stop( NEXUS_VideoDecoderStopSettings )\n\n"
        );

    BKNI_Printf(
        "-- playback --\n"
        "playback_set_settings (pbss) -> NEXUS_Playback_SetSettings( NEXUS_PlaybackSettings )\n"
        "playback_start (pbstart)     -> NEXUS_Playback_Start( NEXUS_PlaybackStartSettings )\n"
        "playback_stop (pbstop)       -> NEXUS_Playback_Stop( NEXUS_PlaybackStopSettings )\n"
        );

    BKNI_Printf(
        "playback_open (pbopen) <parameters are below> end:0\n"
        "  - type:<file,qam,hdmi> : currently only supports type of file\n"
        "  - file:<file_name> : (Note: the file is probed by default and the following parameters filled in).\n"
        );

    BKNI_Printf(
        "  - transport:<NEXUS_TransportType>\n"
        "  - pcr_pid:<n>\n"
        "  - video_codec:<NEXUS_VideoCodec>\n"
        "  - video_pid:<n>\n\n"
        );

    BKNI_Printf(
        "-- display --\n"
        "display_open (dpopen)      -> NEXUS_Display_Open( NEXUS_DisplaySettings )\n"
        "  NEXUS_Display_Open() is not currently exposed, the above settings are applied when video_encoder_open is executed.\n"
        "display_custom_format (dpcust)  -> NEXUS_Display_SetCustomFormatSettings( NEXUS_DisplayCustomFormatSettings )\n\n"
        );

    BKNI_Printf(
        "-- record --\n"
        "record_settings (recss) <parameters are below> end:0\n"
        "  - file:<file_name>\n"
        "record_start (recstart) end:0\n"
        "record_stop (recstop) bWaitForEos:<0/1> end:0\n"
        "record_pause (recpause) end:0\n"
        "record_resume (recresume) end:0\n\n"
        );

    BKNI_Printf(
        "-- mux --\n"
        "mux_open (mxopen) <parameters are below> end:0\n"
        "  - transport:<es,ivf,pes,mp4,ts,mp4f>\n"
        "    - for 'es', the buffers will be read directly\n"
        "    - for 'ivf', 'pes' and 'mp4' the file mux will be used.\n"
        "    - for 'ts' and 'mp4f' the stream mux wil be used, but it is NOT SUPPORTED yet.\n"
        "mux_start (mxstart) end:0\n"
        "mux_stop (mxstop) end:0\n\n"
        );

    BKNI_Printf(
        "-- control --\n"
        "sleep <parameters are below> end:0\n"
        "  - units:<secs,msecs> : specifies the units of duration, default is secs\n"
        "  - duration:<n>\n\n"
        );

    BKNI_Printf(
        "NOTE: a <CR> will print the contents of the structure associated with the active command.\n\n"
        );

    BKNI_Printf(
        "-- general --\n"
        "info             - print the platform capabilities\n"
        "status           - print the status of the currently selected session\n"
        "file:<file_name> - read comands from a file\n"
        "echo             - echo commands as they are read from a file\n"
        "enum:<n>         - print Nexus and other enum's\n"
        "h, help, ?       - prints this help\n"
        "q or quit        - exit the app\n"
        );

    return;

}

int main(int argc, const char *argv[])  {

#if NEXUS_HAS_PLAYBACK

    int32_t i,j;

    TRNSX_TestContext stTestContext;
    TRNSX_Transcode * pTrans;

    char    szCmdFileName[TRNSX_SIZE_FILE_NAME];
    FILE *  hCmdFile=NULL;

    BKNI_Memset(&stTestContext, 0, sizeof(stTestContext));
    BKNI_Memset(&szCmdFileName, 0, sizeof(szCmdFileName) );

    /* Check for command line parameters. */

    for (j=0;j<argc;j++)
    {
        if (!strcmp(argv[j], "-h") || !strcmp(argv[j], "-help") || !strcmp(argv[j], "?")) {
            TRNSX_Help();
            /*TRNSX_Platform_Close(&stTestContext);*/
            return 0;
        }
        else if (!strcmp(argv[j], "-simdisp")) {
            /* enable simultaneous display
             * TODO: now to deal with multiple transcodes? */

            stTestContext.simulDisplay.bEnabled = true;
        }
        else if (!strcmp(argv[j], "-echo")) {
            stTestContext.bEcho = true;
        }
        else if (!strcmp(argv[j], "-file")) {
            if ( hCmdFile )
            {
                fclose(hCmdFile);
                hCmdFile = NULL;
            }
            strncpy(szCmdFileName, argv[++j], TRNSX_SIZE_FILE_NAME);
            hCmdFile = fopen(szCmdFileName,"r");
            if ( !hCmdFile )
            {
                BDBG_ERR(("%s:: failed to open command file: %s\n", BSTD_FUNCTION, szCmdFileName));
                exit(1);
            }
        }
    }

    TRNSX_Platform_Open(&stTestContext);

    /*
     * TODO: cleanup the session initialzation.
     */

    for( i=0; i < TRNSX_NUM_ENCODES; i++ )
    {
        pTrans = &stTestContext.transcodes[i];

        TRNSX_Transcode_GetDefaultSettings( &stTestContext, pTrans );

        pTrans->uiIndex = i;
        /*pTrans->videoDecoder.videoDecoder = stTestContext.videoDecoder;*/

    }

    BKNI_Sleep(1000);   /* Wait to let the initial console output finish. */

    /*TRNSX_Platform_SystemInfo( &stTestContext );*/

    /* Parse the user input.
     * For now always assume that the source will be file, will need to add a command to select the input type.
     * TODO: add support for setting the following
     * - input stream: video/pcr PID's
     */

    while( 1 )
    {
        TRSNX_Command astCmd[TRNSX_NUM_CMDS];
        TRSNX_Command * pstCmd;

        char szInput[TRNSX_SIZE_CMD];

        /* Get the pointer to the selected session. */
        pTrans = &stTestContext.transcodes[stTestContext.uiSelected];

        stTestContext.bInteractive = ( hCmdFile ) ? false : true;

        if ( stTestContext.bInteractive )
        {
            BKNI_Printf("s%d%s%s> ",
                            pTrans->uiIndex,
                            ( pTrans->eActiveCmd == TRNSX_Cmd_eNone ) ? "" : ".",
                            lookup_name(g_cmdStrs, pTrans->eActiveCmd));
        }

        if ( hCmdFile )
        {
            /* read commands from a file. */
            fgets(szInput, sizeof(szInput)-1, hCmdFile);
            if ( feof(hCmdFile) )
            {
                fclose( hCmdFile );
                hCmdFile=NULL;
            }
        }
        else
        {
            /* interactive, get commands from the user. */
            fgets(szInput, sizeof(szInput)-1, stdin);
        }

        TRNSX_Parse_Input( szInput, &astCmd[0], TRNSX_NUM_CMDS );

        if ( !stTestContext.bInteractive && stTestContext.bEcho )
        {
            BKNI_Printf("input cmd: %s", szInput);
        }

        for ( j=0; j < TRNSX_NUM_CMDS-1; j++ )
        {

            pstCmd = &astCmd[j];

            /* Past the last command in the list. */
            if ( strlen(pstCmd->szKey ) == 0 ) break;

            /* For file debug. */
            if ( !stTestContext.bInteractive ) BDBG_MODULE_MSG(trnsx_cmds,("%s: iValue:%d szValue:%s" , pstCmd->szKey, pstCmd->iValue, pstCmd->szValue ));

            if (!strcmp(pstCmd->szKey, "quit") || !strcmp(pstCmd->szKey, "q")) {
                TRNSX_Platform_Close( &stTestContext );
                exit(0);
            }
            else if (!strcmp(pstCmd->szKey, "h") || !strcmp(pstCmd->szKey, "help") || !strcmp(pstCmd->szKey, "?")) {
                TRNSX_Help();
            }
            else if (!strcmp(pstCmd->szKey, "info")) {
                TRNSX_Platform_SystemInfo( &stTestContext );
            }
            else if (!strcmp(pstCmd->szKey, "status")) {
                TRNSX_Transcode_Status( &stTestContext, pTrans ); /* status for this session not all sessions. */
            }
            else if (!strcmp(pstCmd->szKey, "file") && pTrans->eActiveCmd == TRNSX_Cmd_eNone ) /* TODO: cleanup eActiveCmd*/
            {
                if ( pstCmd->bValid == false )
                {
                    BKNI_Printf("command file: %s\n", szCmdFileName );
                }
                else
                {
                    if ( hCmdFile )
                    {
                        fclose(hCmdFile);
                        hCmdFile = NULL;
                    }
                    strncpy(szCmdFileName, pstCmd->szValue, strlen(pstCmd->szValue));
                    hCmdFile = fopen(szCmdFileName,"r");

                    if ( !hCmdFile )  BKNI_Printf("failed to open command file: %s\n", szCmdFileName);
                }

            }

            /*** Dump enums. ***/
            else if (!strcmp(pstCmd->szKey, "enum")) {
                if (!strcmp(pstCmd->szValue, "codec"))              { print_value_list(g_videoCodecStrs);           }
                else if (!strcmp(pstCmd->szValue, "transport"))     { print_value_list(g_transportTypeStrs);        }
                else if (!strcmp(pstCmd->szValue, "profile"))       { print_value_list(g_videoCodecProfileStrs);    }
                else if (!strcmp(pstCmd->szValue, "level"))         { print_value_list(g_videoCodecLevelStrs);      }
                else if (!strcmp(pstCmd->szValue, "framerate"))     { print_value_list(g_videoFrameRateStrs);       }
                else if (!strcmp(pstCmd->szValue, "format"))        { print_value_list(g_videoFormatStrs);          }
                else if (!strcmp(pstCmd->szValue, "aspect"))        { print_value_list(g_displayAspectRatioStrs);   }
                else if (!strcmp(pstCmd->szValue, "encoder_type"))  { print_value_list(g_encoderTypeStrs);        }
                else if (!strcmp(pstCmd->szValue, "tristate"))      { print_value_list(g_tristateEnableStrs);     }
                else if (!strcmp(pstCmd->szValue, "entropy"))       { print_value_list(g_entropyCodingStrs);     }
                else if (!strcmp(pstCmd->szValue, "stop"))          { print_value_list(g_encoderStopModeStrs);     }
                else if (!strcmp(pstCmd->szValue, "display_type"))  { print_value_list(g_displayTypeStrs);     }
                else if (!strcmp(pstCmd->szValue, "timestamp"))     { print_value_list(g_transportTimestampTypeStrs);     }
                else if (!strcmp(pstCmd->szValue, "transxtype"))    { print_value_list(g_trnsxTransportTypeStrs);     }
                else if (!strcmp(pstCmd->szValue, "timing_generator")) { print_value_list(g_displayTimingGeneratorStrs);     }
                else {
                    BKNI_Printf("enum command: currently supports\n"
                                "  codec        - video codec\n"
                                "  profile      - codec profile\n"
                                "  level        - codec level\n"
                                "  framerate    - video frame rate\n"
                                "  transport    - transport type\n"
                                "  transxtype   - transcode_x transport type, includes IVF\n"
                                "  aspect       - display aspect ratio\n"
                                );

                    BKNI_Printf("  format       - display format\n"
                                "  encoder_type - encoder type\n"
                                "  tristate     - Nexus trisate enable\n"
                                "  entropy      - entropy coding\n"
                                "  stop         - video encoder stop mode\n"
                                "  display_type - display type\n"
                                "  timing_generator - timing generator\n"
                                "  timestamp    - timestamp type\n"
                                );

                }
            }

            /*** Execute the active command. ***/

            else if (!strcmp(pstCmd->szKey, "end")) {
                TRNSX_Command_Execute( &stTestContext, pTrans, pstCmd );
            }

            /*** Close the active command. ***/

            else if (!strcmp(pstCmd->szKey, "set")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eNone;
            }

            /*** Set the parameters associated with the active command. ***/
            else if ( pTrans->eActiveCmd != TRNSX_Cmd_eNone )
            {
                bool bRet=true;
                bRet = TRNSX_Command_ParseParams( &stTestContext, pTrans, pstCmd );

                /* When reading from a script file, exit the app to help debug the scripts. */
                if (( bRet == false ) &&  !stTestContext.bInteractive )
                {
                    BKNI_Printf("TRNSX_Command_ParseParams indicates bad line in script file:\n  line read: %s", szInput);
                    exit(0);
                }
            }

            /*** Session related commands. ***/

            else if (!strcmp(pstCmd->szKey, "session"))
            {
                pTrans->eActiveCmd = TRNSX_Cmd_eSession;
            }

            /*** transcode wrappers ***/
            else if (!strcmp(pstCmd->szKey, "transcode"))
            {
                pTrans->eActiveCmd = TRNSX_Cmd_eTranscode;
            }

            /*** sleep related commands. ***/

            else if (!strcmp(pstCmd->szKey, "sleep"))
            {
                pTrans->eActiveCmd = TRNSX_Cmd_eSleep;
            }

            /*** Encoder commands ***/

            /* NEXUS_VideoEncoder_SetSettings */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_set_settings") || !strcmp(pstCmd->szKey, "vess")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderSetSettings;
            }

            /* NEXUS_VideoEncoder_Open */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_open") || !strcmp(pstCmd->szKey, "veopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderOpen;
            }

            /* NEXUS_VideoEncoder_Close */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_close") || !strcmp(pstCmd->szKey, "veclose")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderClose;
            }

            /* NEXUS_VideoEncoder_Start */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_start") || !strcmp(pstCmd->szKey, "vestart")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderStart;
            }

            /* NEXUS_VideoEncoder_Stop */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_stop") || !strcmp(pstCmd->szKey, "vestop")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderStop;
            }

            /* NEXUS_VideoEncoder_Stop */
            else  if (!strcmp(pstCmd->szKey, "video_encoder_status") || !strcmp(pstCmd->szKey, "vestatus")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoEncoderStatus;
            }

            /*** Decoder commands ***/

            /* NEXUS_VideoDecoder_SetSettings */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_set_settings") || !strcmp(pstCmd->szKey, "vdss")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderSetSettings;
            }

            /* NEXUS_VideoDecoder_Open */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_open") || !strcmp(pstCmd->szKey, "vdopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderOpen;
            }

            /* NEXUS_VideoDecoder_Close */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_close") || !strcmp(pstCmd->szKey, "vdclose")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderClose;
            }

            /* NEXUS_VideoDecoder_Start */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_start") || !strcmp(pstCmd->szKey, "vdstart")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderStart;
            }

            /* NEXUS_VideoDecoder_Stop */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_stop") || !strcmp(pstCmd->szKey, "vdstop")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderStop;
            }

            /* NEXUS_VideoDecoder_GetStatus */
            else  if (!strcmp(pstCmd->szKey, "video_decoder_status") || !strcmp(pstCmd->szKey, "vdstatus")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eVideoDecoderStatus;
            }

            /*** Playback commands ***/

            /* NEXUS_Playback_SetSettings */
            else  if (!strcmp(pstCmd->szKey, "playback_set_settings") || !strcmp(pstCmd->szKey, "pbss")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaybackSetSettings;
            }

            /* a transcode_x command */
            else if (!strcmp(pstCmd->szKey, "playback_open") || !strcmp(pstCmd->szKey, "pbopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaybackOpen;
            }

            /* a transcode_x command */
            else if (!strcmp(pstCmd->szKey, "playback_close") || !strcmp(pstCmd->szKey, "pbclose")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaybackClose;
            }

            /* NEXUS_Playback_Start */
            else  if (!strcmp(pstCmd->szKey, "playback_start") || !strcmp(pstCmd->szKey, "pbstart")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaybackStart;
            }

            /* NEXUS_Playback_Stop */
            else  if (!strcmp(pstCmd->szKey, "playback_stop") || !strcmp(pstCmd->szKey, "pbstop")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaybackStop;
            }

            /*** Display commands ***/

            /* display settingss */
            else  if (!strcmp(pstCmd->szKey, "display_open") || !strcmp(pstCmd->szKey, "dpopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eDisplayOpen;
            }
            else  if (!strcmp(pstCmd->szKey, "display_close") || !strcmp(pstCmd->szKey, "dpclose")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eDisplayClose;
            }
            /* NEXUS_Display_SetCustomFormatSettings */
            else  if (!strcmp(pstCmd->szKey, "display_custom_format") || !strcmp(pstCmd->szKey, "dpcust")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eDisplayCustomFormatSettings;
            }

            /*** Playpump commands ***/

            else  if (!strcmp(pstCmd->szKey, "playpump_open") || !strcmp(pstCmd->szKey, "ppopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaypumpOpen;
            }
            else  if (!strcmp(pstCmd->szKey, "playpump_set_settings") || !strcmp(pstCmd->szKey, "ppss")) {
                pTrans->eActiveCmd = TRNSX_Cmd_ePlaypumpSetSettings;
            }

            /*** record commands ***/

            else  if (!strcmp(pstCmd->szKey, "record_settings") || !strcmp(pstCmd->szKey, "recss")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eRecordSettings;
            }
            else  if (!strcmp(pstCmd->szKey, "record_start") || !strcmp(pstCmd->szKey, "recstart")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eRecordStart;
            }
            else  if (!strcmp(pstCmd->szKey, "record_stop") || !strcmp(pstCmd->szKey, "recstop")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eRecordStop;
            }
            else  if (!strcmp(pstCmd->szKey, "record_pause") || !strcmp(pstCmd->szKey, "recpause")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eRecordPause;
            }
            else  if (!strcmp(pstCmd->szKey, "record_resume") || !strcmp(pstCmd->szKey, "recresume")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eRecordResume;
            }

            /*** mux commands ***/

            else  if (!strcmp(pstCmd->szKey, "mux_open") || !strcmp(pstCmd->szKey, "mxopen")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eMuxOpen;
            }
            else  if (!strcmp(pstCmd->szKey, "mux_close") || !strcmp(pstCmd->szKey, "mxclose")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eMuxClose;
            }
            else  if (!strcmp(pstCmd->szKey, "mux_start") || !strcmp(pstCmd->szKey, "mxstart")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eMuxStart;
            }
            else  if (!strcmp(pstCmd->szKey, "mux_stop") || !strcmp(pstCmd->szKey, "mxstop")) {
                pTrans->eActiveCmd = TRNSX_Cmd_eMuxStop;
            }

            /* Unsupported command. */
            else {
                if ( strcmp(pstCmd->szKey, "CR") )
                {
                    /* If not simply a CR, then a bad command. */
                    BKNI_Printf("%s: unknown command: %s\n", BSTD_FUNCTION, pstCmd->szKey );

                    /* When reading from a script file, exit the app to help debug the scripts. */
                    if ( !stTestContext.bInteractive )
                    {
                        BKNI_Printf("bad line in script file:\n  line read: %s", szInput);
                        exit(0);
                    }
                }
            }

        }

    }

#endif
    return 0;
}

#endif   /* end ELSE NEXUS_HAS_VIDEO_ENCODER */
