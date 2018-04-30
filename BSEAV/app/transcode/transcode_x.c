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

    BKNI_Printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
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

#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
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

/* The following isn't currently supported. */
#define USE_DDRE    0 /* set to 1 to enable DDRE&DV258 for multi-channel MS-11 -> AAC xcode test */

#if USE_DDRE
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"
#endif


BDBG_MODULE(transcode_x);
BDBG_FILE_MODULE(trnsx_input);
BDBG_FILE_MODULE(trnsx_dbg);
BDBG_FILE_MODULE(trnsx_dbg_audio);
BDBG_FILE_MODULE(trnsx_echo);
BDBG_FILE_MODULE(trnsx_parse);

#include "media_probe.c"


/*#define SIMUL_DISPLAY 1*/

/*
 * Local data types.
 */
/*
#if NEXUS_HAS_SYNC_CHANNEL
#define BTST_ENABLE_AV_SYNC 1
#else
#define BTST_ENABLE_AV_SYNC 0
#endif
*/
#define BTST_ENABLE_AV_SYNC 0

/* matching audio xcoders per DSP core with video */
#if NEXUS_NUM_VCE_CHANNELS
#define BTST_NUM_AUDIO_XCODE_PER_DSP  NEXUS_NUM_VCE_CHANNELS
#else
#define BTST_NUM_AUDIO_XCODE_PER_DSP  2
#endif

#if  NEXUS_HAS_FRONTEND
#define BTST_SUPPORT_FRONTEND    1
#else
#define BTST_SUPPORT_FRONTEND    0
#endif

/***********************************
 *  Default stream mux PID assignments
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
#define TRNSX_MAX_SESSIONS 6

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

typedef enum TRNSX_Module
{
    TRNSX_Module_eEncoder=0,
    TRNSX_Module_eVideoDecoder,
    TRNSX_Module_eAudioDecoder,
    TRNSX_Module_eMax

} TRNSX_Module;
#if 0
static const namevalue_t g_moduleStrs[] = {
    {"encoder", TRNSX_Module_eEncoder},
    {"video decoder", TRNSX_Module_eVideoDecoder},
    {"audio decoder", TRNSX_Module_eAudioDecoder},
    {NULL, 0}
};
#endif
typedef enum TRNSX_State
{
    TRNSX_State_eClosed=0,
    TRNSX_State_eOpened,
    TRNSX_State_eRunning,
    TRNSX_State_ePaused,
    TRNSX_State_eStopped,
    TRNSX_State_eMax

} TRNSX_State;

static const namevalue_t g_stateStrs[] = {
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

static const namevalue_t g_audioChannelFormatStrs[] = {
    {"none", NEXUS_AudioMultichannelFormat_eNone},
    {"stereo", NEXUS_AudioMultichannelFormat_eStereo},
    {"5.1", NEXUS_AudioMultichannelFormat_e5_1},
    {"7.1", NEXUS_AudioMultichannelFormat_e7_1},
    {NULL, 0}
};

/*
 * Utils for processing tables.
 */

unsigned TRNSX_Lookup(const namevalue_t *table, const char *name)
{
    unsigned i;
    for (i=0;table[i].name;i++) {
        if (!strcasecmp(table[i].name, name)) {
            return table[i].value;
        }
    }
    return 0;
}



#define TRNSX_SIZE_CMD 256
#define TRNSX_NUM_CMDS 256
#define TRNSX_MAX_VALUES 16

typedef struct TRSNX_Command
{
    char    szKey[TRNSX_SIZE_CMD];

    unsigned    uiNumValues;

    struct
    {
        char     szValue[TRNSX_SIZE_CMD];
        unsigned uiValue;

   } data[TRNSX_MAX_VALUES];

} TRSNX_Command;


/* macros for processing command parameters */

#define TRNSX_ELSE_IF_SIZE_T( _pcmd, _szkey, _element )                                     \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                      \
    {                                                                           \
        if ( _pcmd->uiNumValues ) _element = _pcmd->data[0].uiValue;                              \
        else BKNI_Printf("  current value for %s: %lu\n", _pcmd->szKey, (unsigned long) _element );  \
    }

#define TRNSX_ELSE_IF( _pcmd, _szkey, _element )                                     \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                      \
    {                                                                           \
        if ( _pcmd->uiNumValues ) _element = _pcmd->data[0].uiValue;                              \
        else BKNI_Printf("  current value for %s: %u\n", _pcmd->szKey, _element );  \
    }

#define TRNSX_ELSE_IF_LOOKUP( _pcmd, _szkey, _element, _list )                      \
    else if (!strcmp(_pcmd->szKey, _szkey ))                                        \
    {                                                                               \
        if ( _pcmd->uiNumValues ) _element = lookup(_list,_pcmd->data[0].szValue);             \
        else BKNI_Printf("  current value for %s: %d\n", _pcmd->szKey, _element );  \
    }

#define TRNSX_ERROR( _ptrans, _pcmd, _list, _bRet )                                                                                     \
    else                                                                                                                                \
    {                                                                                                                                   \
        BDBG_ERR(("%s::  command:: %s / unsupported parameter:: %s\n", BSTD_FUNCTION, lookup_name(_list, _ptrans->eActiveCmd),  _pcmd->szKey ));    \
        _bRet = false;                                                                                                                  \
    }                                                                                                                                   \

#define TRNSX_DEFAULT_PARAM_MSG( _bprint ) \
    if ( _bprint )  BKNI_Printf("  using default values\n");

/* macros for checking state */

#define TRNSX_CHECK_ENCODE_AUDIO( _ptrans ) \
    ( _ptrans->record.eTransportType == NEXUS_TransportType_eMpeg2Pes \
      || _ptrans->record.eTransportType == NEXUS_TransportType_eMp4 \
      || _ptrans->record.eTransportType == NEXUS_TransportType_eTs \
      || _ptrans->record.eTransportType == NEXUS_TransportType_eMp4Fragment \
    ) \
    && _ptrans->bEncodeAudio == true \
    && _ptrans->source.uiNumAudio != 0


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
    /* video encoder */
    TRNSX_Cmd_eVideoEncoderOpen,
    TRNSX_Cmd_eVideoEncoderClose,
    TRNSX_Cmd_eVideoEncoderStart,
    TRNSX_Cmd_eVideoEncoderStop,
    TRNSX_Cmd_eVideoEncoderSetSettings,
    TRNSX_Cmd_eVideoEncoderStatus,
    /* audio encoder */
    TRNSX_Cmd_eAudioEncoderOpen,
    TRNSX_Cmd_eAudioEncoderClose,
    /* display */
    TRNSX_Cmd_eDisplayOpen,
    TRNSX_Cmd_eDisplayClose,
    TRNSX_Cmd_eDisplayCustomFormatSettings,
    /* playback */
    TRNSX_Cmd_ePlaybackOpen,
    TRNSX_Cmd_ePlaybackClose,
    TRNSX_Cmd_ePlaybackStart,
    TRNSX_Cmd_ePlaybackStop,
    TRNSX_Cmd_ePlaybackSetSettings,
    /* video decoder */
    TRNSX_Cmd_eVideoDecoderOpen,
    TRNSX_Cmd_eVideoDecoderClose,
    TRNSX_Cmd_eVideoDecoderStart,
    TRNSX_Cmd_eVideoDecoderStop,
    TRNSX_Cmd_eVideoDecoderSetSettings,
    TRNSX_Cmd_eVideoDecoderStatus,
    /* audio decoder */
    TRNSX_Cmd_eAudioDecoderOpen,
    TRNSX_Cmd_eAudioDecoderClose,
    TRNSX_Cmd_eAudioDecoderStart,
    TRNSX_Cmd_eAudioDecoderStop,
    TRNSX_Cmd_eAudioDecoderSetSettings,
    TRNSX_Cmd_eAudioDecoderStatus,
    /* playpump */
    TRNSX_Cmd_ePlaypumpOpen,
    TRNSX_Cmd_ePlaypumpSetSettings,
    /* record */
    TRNSX_Cmd_eRecordStart,
    TRNSX_Cmd_eRecordStop,
    TRNSX_Cmd_eRecordPause,
    TRNSX_Cmd_eRecordResume,
    TRNSX_Cmd_eRecordSettings,
    /* mux */
    TRNSX_Cmd_eMuxOpen,
    TRNSX_Cmd_eMuxClose,
    TRNSX_Cmd_eMuxStart,
    TRNSX_Cmd_eMuxStop,
    /* miscellaneous*/
    TRNSX_Cmd_eSession,
    TRNSX_Cmd_eSelect,
    TRNSX_Cmd_eTranscode,
    TRNSX_Cmd_eSleep,
    TRNSX_Cmd_eMax
} TRNSX_Cmd;

static const namevalue_t g_cmdFullnameStrs[] = {
    {"",                        TRNSX_Cmd_eNone},
    {"video_encoder_open",      TRNSX_Cmd_eVideoEncoderOpen},
    {"video_encoder_close",     TRNSX_Cmd_eVideoEncoderClose},
    {"video_encoder_start",     TRNSX_Cmd_eVideoEncoderStart},
    {"video_encoder_stop",      TRNSX_Cmd_eVideoEncoderStop},
    {"video_encoder_set_settings", TRNSX_Cmd_eVideoEncoderSetSettings},
    {"video_encoder_status",    TRNSX_Cmd_eVideoEncoderStatus},
    {"audio_encoder_open",      TRNSX_Cmd_eAudioEncoderOpen},
    {"audio_encoder_close",     TRNSX_Cmd_eAudioEncoderClose},
    {"display_open",            TRNSX_Cmd_eDisplayOpen},
    {"display_close",           TRNSX_Cmd_eDisplayClose},
    {"display_custom_format",   TRNSX_Cmd_eDisplayCustomFormatSettings},
    {"playback_open",           TRNSX_Cmd_ePlaybackOpen},
    {"playback_close",          TRNSX_Cmd_ePlaybackClose},
    {"playback_start",          TRNSX_Cmd_ePlaybackStart},
    {"playback_stop",           TRNSX_Cmd_ePlaybackStop},
    {"playback_set_settings",   TRNSX_Cmd_ePlaybackSetSettings},
    {"video_decoder_open",      TRNSX_Cmd_eVideoDecoderOpen},
    {"video_decoder_close",     TRNSX_Cmd_eVideoDecoderClose},
    {"video_decoder_start",     TRNSX_Cmd_eVideoDecoderStart},
    {"video_decoder_stop",      TRNSX_Cmd_eVideoDecoderStop},
    {"video_decoder_set_settings", TRNSX_Cmd_eVideoDecoderSetSettings},
    {"video_decoder_status",    TRNSX_Cmd_eVideoDecoderStatus},
    {"audio_decoder_open",      TRNSX_Cmd_eAudioDecoderOpen},
    {"audio_decoder_close",     TRNSX_Cmd_eAudioDecoderClose},
    {"audio_decoder_start",     TRNSX_Cmd_eAudioDecoderStart},
    {"audio_decoder_stop",      TRNSX_Cmd_eAudioDecoderStop},
    {"audio_decoder_set_settings", TRNSX_Cmd_eAudioDecoderSetSettings},
    {"audio_decoder_status",    TRNSX_Cmd_eAudioDecoderStatus},
    {"playpump_open",           TRNSX_Cmd_ePlaypumpOpen},
    {"playpump_set_settings",   TRNSX_Cmd_ePlaypumpSetSettings},
    {"record_start",            TRNSX_Cmd_eRecordStart},
    {"record_stop",             TRNSX_Cmd_eRecordStop},
    {"record_pause",            TRNSX_Cmd_eRecordPause},
    {"record_resume",           TRNSX_Cmd_eRecordResume},
    {"record_settings",         TRNSX_Cmd_eRecordSettings},
    {"mux_open",                TRNSX_Cmd_eMuxOpen},
    {"mux_close",               TRNSX_Cmd_eMuxClose},
    {"mux_start",               TRNSX_Cmd_eMuxStart},
    {"mux_stop",                TRNSX_Cmd_eMuxStop},
    {"session",                 TRNSX_Cmd_eSession},
    {"select",                  TRNSX_Cmd_eSelect},
    {"transcode",               TRNSX_Cmd_eTranscode},
    {"sleep",                   TRNSX_Cmd_eSleep},
    {NULL, 0}
};

static const namevalue_t g_cmdShortnameStrs[] = {
    {"",        TRNSX_Cmd_eNone},
    {"veopen",  TRNSX_Cmd_eVideoEncoderOpen},
    {"veclose", TRNSX_Cmd_eVideoEncoderClose},
    {"vestart", TRNSX_Cmd_eVideoEncoderStart},
    {"vestop",  TRNSX_Cmd_eVideoEncoderStop},
    {"vess",    TRNSX_Cmd_eVideoEncoderSetSettings},
    {"vestatus",TRNSX_Cmd_eVideoEncoderStatus},
    {"aeopen",  TRNSX_Cmd_eAudioEncoderOpen},
    {"aeclose", TRNSX_Cmd_eAudioEncoderClose},
    {"dpopen",  TRNSX_Cmd_eDisplayOpen},
    {"dpclose", TRNSX_Cmd_eDisplayClose},
    {"dpcust",  TRNSX_Cmd_eDisplayCustomFormatSettings},
    {"pbopen",  TRNSX_Cmd_ePlaybackOpen},
    {"pbclose", TRNSX_Cmd_ePlaybackClose},
    {"pbstart", TRNSX_Cmd_ePlaybackStart},
    {"pbstop",  TRNSX_Cmd_ePlaybackStop},
    {"pbss",    TRNSX_Cmd_ePlaybackSetSettings},
    {"vdopen",  TRNSX_Cmd_eVideoDecoderOpen},
    {"vdclose", TRNSX_Cmd_eVideoDecoderClose},
    {"vdstart", TRNSX_Cmd_eVideoDecoderStart},
    {"vdstop",  TRNSX_Cmd_eVideoDecoderStop},
    {"vdss",    TRNSX_Cmd_eVideoDecoderSetSettings},
    {"vdstatus",TRNSX_Cmd_eVideoDecoderStatus},
    {"adopen",  TRNSX_Cmd_eAudioDecoderOpen},
    {"adclose", TRNSX_Cmd_eAudioDecoderClose},
    {"adstart", TRNSX_Cmd_eAudioDecoderStart},
    {"adstop",  TRNSX_Cmd_eAudioDecoderStop},
    {"adss",    TRNSX_Cmd_eAudioDecoderSetSettings},
    {"adstatus",TRNSX_Cmd_eAudioDecoderStatus},
    {"ppopen",  TRNSX_Cmd_ePlaypumpOpen},
    {"ppss",    TRNSX_Cmd_ePlaypumpSetSettings},
    {"recstart",TRNSX_Cmd_eRecordStart},
    {"recstop", TRNSX_Cmd_eRecordStop},
    {"recpause",TRNSX_Cmd_eRecordPause},
    {"recresume",TRNSX_Cmd_eRecordResume},
    {"recss",   TRNSX_Cmd_eRecordSettings},
    {"mxopen",  TRNSX_Cmd_eMuxOpen},
    {"mxclose", TRNSX_Cmd_eMuxClose},
    {"mxstart", TRNSX_Cmd_eMuxStart},
    {"mxstop",  TRNSX_Cmd_eMuxStop},
    {"",        TRNSX_Cmd_eSession},
    {"",        TRNSX_Cmd_eSelect},
    {"",        TRNSX_Cmd_eTranscode},
    {"",        TRNSX_Cmd_eSleep},
    {NULL, 0}
};

typedef struct psi_message_t {
    unsigned short pid;
    NEXUS_MessageHandle message;
    NEXUS_PidChannelHandle pidChannel;
    bool done;
} psi_message_t;

#if xyz
/* transcode_ts stream mux user data options.
 * Which to add to transcod_x? */

if(pTrans->source.eType != TRNSX_Source_eHDMI) {
    pContext->bEncodeCCUserData = g_bEncodeCC;

    if(pContext->inputSettings.streamMux.userData.enable) {
        printf("\n PMT pid [0=auto detection]:                           ");
        pTrans->source.uiPmtPid = getValue(input);
        printf("\n Do you want all TS user data passed thru? [1=y/0=n]   ");
        pTrans->streamMux.userData.bRemapPids = (0==getValue(input));

        if( !pTrans->streamMux.userData.bRemapPids )
        {
            pTrans->streamMux.userData.numPids = BTST_TS_USER_DATA_ALL;
        } else {
            printf("\n How many user data PIDs are passed thru?   ");
            pTrans->streamMux.userData.numPids = getValue(input);
            pTrans->streamMux.userData.numPids = (pTrans->streamMux.userData.numPids > NEXUS_MAX_MUX_PIDS) ?
                NEXUS_MAX_MUX_PIDS : pTrans->streamMux.userData.numPids;

            printf("\n Please enter the input user data PIDs list: ");
            for(choice=0; choice<(int)pTrans->streamMux.userData.numPids; choice++) {
                pTrans->source.userData.pids[choice] = getValue(input);
            }
            printf("\n Remap the output TS user data PIDs? [1=y/0=n]  ");
            pTrans->streamMux.userData.bRemapPids = getValue(input);

            for(choice=0; choice<(int)pTrans->streamMux.userData.numPids; choice++) {
                if(pTrans->streamMux.userData.bRemapPids) {
                    pTrans->streamMux.userData.pids[choice] = pTrans->streamMux.userData.uiPid+choice;
                } else {/* no change */
                    pTrans->streamMux.userData.pids[choice] = pTrans->source.userData.pids[choice];
                }
            }
        }
    }

#endif


typedef struct TRNSX_Transcode
{
    uint32_t      uiIndex;
    TRNSX_State eState;

    TRNSX_Cmd   eActiveCmd;
    bool        bNonRealTime;
    bool        bEncodeCCData;
    bool        bEncodeAudio;

    /*** Source ***/
    struct
    {
        TRNSX_Source        eType;

        /* common parameters */
        NEXUS_VideoCodec    eVideoCodec;
        int                 iVideoPid;
        unsigned            uiNumVideo;

        NEXUS_AudioCodec    eAudioCodec;
        int                 iAudioPid;
        unsigned            uiNumAudio;

        int                 iPcrPid;
        unsigned            uiNumPcr;
        unsigned            uiPmtPid; /* pContext->inputSettings.pmtPid */

        struct
        {
            unsigned            numPids;
            unsigned            pids[NEXUS_MAX_MUX_PIDS];

        } userData;

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

    } source;

    /*** Output ***/
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


    /*** AV sync related variables ***/
    struct
    {
        bool bEnable;
#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SyncChannelHandle     syncChannel;
        NEXUS_StcChannelPairSettings stcAudioVideoPair;
#endif
    } syncChannel;

    /*** Audio Decoder ***/
    struct
    {
        TRNSX_State eState;

        NEXUS_AudioDecoderHandle    audioDecoder;
        NEXUS_StcChannelHandle      stcChannel;
        NEXUS_PidChannelHandle      pidChannel;
        NEXUS_AudioMixerHandle      audioMixer;

        struct
        {
            bool                        bEnabled;
            NEXUS_AudioDecoderHandle    audioDecoder;
            NEXUS_PidChannelHandle      pidChannel;

        } secondary;
#if 0
    NEXUS_VideoDecoderStartSettings vidProgram;
    NEXUS_AudioDecoderStartSettings audProgram, secondaryProgram;

#endif

    } audioDecoder;

    /*** Audio Encoder ***/
    struct
    {
        NEXUS_AudioEncoderHandle    audioEncoder;
        NEXUS_AudioEncoderSettings  encoderSettings;

    } audioEncoder;

    /*** Audio Mux Output ***/
    struct
    {
        NEXUS_AudioMuxOutputHandle          audioMuxOutput;
        NEXUS_AudioMuxOutputStartSettings   startSettings;

    } audioMuxOutput;

    /*** Video Decoder ***/
    struct
    {
        TRNSX_State eState;
        NEXUS_VideoDecoderHandle    videoDecoder;

        NEXUS_PidChannelHandle      videoPidChannel;
        NEXUS_PidChannelHandle      pcrPidChannel;

        NEXUS_StcChannelSettings        stcSettings;    /* TODO: delete? */
        NEXUS_StcChannelHandle          stcChannel;
        NEXUS_VideoDecoderStartSettings startSettings;  /* TODO: delete? */
    } videoDecoder;

    /*** Video Encoder ***/
    struct
    {
        TRNSX_State eState;
        bool        bEncoderWatchdogged; /* make a noun a verb */

        NEXUS_VideoWindowHandle         window;
        NEXUS_VideoEncoderHandle        videoEncoder;
        NEXUS_StcChannelSettings        stcSettings;     /* TODO: delete? */
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

    /*** File Mux ***/
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


    /*** Stream Mux ***/
    struct
    {
        bool bSupported;
        bool bActive;
        bool bPrematureMuxStop;
        bool bAudioPesPacking;
        bool bDspMixer;
        bool bForce48KbpsAACplus;
        bool b32KHzAudio;
        NEXUS_AudioMultichannelFormat eMultiChanFmt;

        TRNSX_State eState;

        BKNI_EventHandle           finishEvent;

        NEXUS_TransportTimestampType transportTimestampType; /* g_TtsOutputType in transcode_ts */

        /* how to expose the following to the user?  Add a streammux command? */
        bool        bMCPB;
        unsigned    uiNumDescs;    /* Mux XPT pb descriptors (when MTU BPP is enabled, worst case < A2P * frameRate * 3) */

#if NEXUS_HAS_STREAM_MUX

        unsigned    uiPcrPid;
        unsigned    uiVideoPid;
        unsigned    uiAudioPid;
        unsigned    uiPmtPid;
        unsigned    uiPatPid;

        struct
        {
            /* can be set by the user */
            bool        enable;         /* bTsUserDataInput/g_bTsUserData, Enabled TS layer user data transcode */
            size_t      numPids;        /* pContext->inputSettings.numUserDataPids */
            bool        bRemapPids;  /* pContext->bRemapUserDataPid */
            unsigned    uiBasePid;      /* base PID for remapping. */

            unsigned    uiPid;

            struct
            {
                bool            bValid;
                unsigned        pid;
                TS_PMT_stream   stPMTStream;
                int             iLengthDesc;
                uint8_t         dataDescriptors[188];

            } streams[NEXUS_MAX_MUX_PIDS];

            psi_message_t   psi_message[BTST_MAX_MESSAGE_FILTERS];

        } userData;


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

        /*NEXUS_MessageHandle       userDataMessage[NEXUS_MAX_MUX_PIDS];*/
        NEXUS_PidChannelHandle    pidChannelUserData[NEXUS_MAX_MUX_PIDS];
        NEXUS_PidChannelHandle    pidChannelTranscodeUserData[NEXUS_MAX_MUX_PIDS];


#endif

    } streamMux;

    /*** Playback ***/
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

    unsigned        uiSelected;
    unsigned        uiMaxSessions;
    TRNSX_Transcode transcodes[TRNSX_MAX_SESSIONS];

    struct
    {
        bool bEnabled;
        NEXUS_DisplayHandle       display;
        NEXUS_VideoWindowHandle   window;
    } simulDisplay;


} TRNSX_TestContext;

/*
 * Local functions
 */
static NEXUS_Error TRNSX_Playback_Start( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans );
static NEXUS_Error TRNSX_Playback_Stop( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans );
static NEXUS_Error TRNSX_AudioDecoder_Start( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans );

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
        BKNI_Printf("%s (%u)%s",sep,table[i].value, table[i].name);
        sep = ",";
    }
    BKNI_Printf(" }\n");
}

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


    pTrans->source.eVideoCodec = NEXUS_VideoCodec_eUnknown;
    pTrans->source.eAudioCodec = NEXUS_AudioCodec_eUnknown;
    pTrans->source.uiNumVideo = 0;
    pTrans->source.uiNumAudio = 0;
    pTrans->source.uiNumPcr = 0;

    pTrans->source.bUseStreamAsIndex = probe_results.useStreamAsIndex;

    pTrans->source.eStreamType = probe_results.transportType;

    pTrans->source.transportTimestampType = probe_results.timestampType;


    pTrans->source.eVideoCodec = probe_results.video[0].codec;
    pTrans->source.iVideoPid   = probe_results.video[0].pid;
    pTrans->source.uiNumVideo = probe_results.num_video;
    pTrans->source.bFileIsValid = ( probe_results.num_video != 0 ); /*TODO: still needed?*/

    pTrans->source.eAudioCodec = probe_results.audio[0].codec;
    pTrans->source.iAudioPid   = probe_results.audio[0].pid;
    pTrans->source.uiNumAudio = probe_results.num_audio;

    pTrans->source.iPcrPid = probe_results.pcr[0].pid;;
    pTrans->source.uiNumPcr = probe_results.num_pcr;

    strncpy( pTrans->source.fname, filename, TRNSX_SIZE_FILE_NAME-1 );

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

    pTrans->source.bFileIsValid = false;

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

    pTrans->source.eVideoCodec = NEXUS_VideoCodec_eUnknown;
    pTrans->source.eAudioCodec = NEXUS_AudioCodec_eUnknown;

    pTrans->source.eStreamType = b_mpegtype2nexus(stream->type);

    strncpy( pTrans->source.fname, filename, TRNSX_SIZE_FILE_NAME-1 );


    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
        switch(track->type) {
            case bmedia_track_type_audio:
                if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                    pTrans->source.iAudioPid = track->number;
                    pTrans->source.eAudioCodec = b_audiocodec2nexus(track->info.audio.codec);
                    foundAudio = true;
                }
                break;
            case bmedia_track_type_video:
                if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                    pTrans->source.iVideoPid = track->number;
                    pTrans->source.eVideoCodec = b_videocodec2nexus(track->info.video.codec);
                    pTrans->source.bFileIsValid = true;
                    foundVideo = true;
                }
                break;
            case bmedia_track_type_pcr:
                pTrans->source.iPcrPid = track->number;
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
    BKNI_Printf("  pts =          0x%08x // 45 Khz\n", pstStatus->pts );
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

static void TRNSX_Print_AudioDecoderStatus( NEXUS_AudioDecoderStatus * pstStatus )
{
    BKNI_Printf("  data buffer depth     = %u\n", pstStatus->fifoDepth);
    BKNI_Printf("  data buffer size      = %u\n", pstStatus->fifoSize);
    BKNI_Printf("  sample rate           = %u\n", pstStatus->sampleRate);
    BKNI_Printf("  queued frames         = %u\n", pstStatus->queuedFrames);
    BKNI_Printf("  numDecoded count      = %u\n", pstStatus->framesDecoded);
    BKNI_Printf("  numDummyFrames        = %u\n", pstStatus->dummyFrames);
    BKNI_Printf("  numFifoOverflows      = %u\n", pstStatus->numFifoOverflows);
    BKNI_Printf("  numFifoUnderflows     = %u\n", pstStatus->numFifoUnderflows);
    BKNI_Printf("  current PTS (45KHz)   = 0x%x\n", pstStatus->pts);
    BKNI_Printf("  PTS error count       = %u\n", pstStatus->ptsErrorCount);
    return;
}

static void TRNSX_Print_AudioMuxOutputStatus( NEXUS_AudioMuxOutputStatus * pstStatus )
{
    BKNI_Printf("  data buffer depth     = %u\n", pstStatus->data.fifoDepth);
    BKNI_Printf("  data buffer size      = %u\n", pstStatus->data.fifoSize);
    BKNI_Printf("  numEncoded frames     = %u\n", pstStatus->numFrames);
    BKNI_Printf("  numErrorFrames        = %u\n", pstStatus->numErrorFrames);
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
    BKNI_Printf("  frameRate:           %s // NEXUS_VideoFrameRate\n", lookup_name(g_videoFrameRateStrs, pstSettings->frameRate) );
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
    BKNI_Printf("  type:                    %s  // NEXUS_VideoEncoderType\n", lookup_name( g_encoderTypeStrs, pstSettings->type ) );
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
    BKNI_Printf("  codec:                %s  // NEXUS_VideoCodec\n",  lookup_name(g_videoCodecStrs, pstSettings->codec) );
    BKNI_Printf("  profile:              %s  // NEXUS_VideoCodecProfile\n",  lookup_name(g_videoCodecProfileStrs, pstSettings->profile ) );
    BKNI_Printf("  level:                %s  // NEXUS_VideoCodecLevel\n", lookup_name( g_videoCodecLevelStrs, pstSettings->level ));

    BKNI_Printf("  bounds.outputFrameRate.min:                 %s  // NEXUS_VideoFrameRate\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.outputFrameRate.min));
    BKNI_Printf("  bounds.outputFrameRate.max:                 %s  // NEXUS_VideoFrameRate\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.outputFrameRate.max));
    BKNI_Printf("  bounds.inputFrameRate.min:                  %s  // NEXUS_VideoFrameRate\n", lookup_name(g_videoFrameRateStrs, pstSettings->bounds.inputFrameRate.min));
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
    BKNI_Printf("  entropyCoding:                              %s   // NEXUS_EntropyCoding\n", lookup_name( g_entropyCodingStrs, pstSettings->entropyCoding));
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
    BKNI_Printf("  mode: %s  // NEXUS_VideoEncoderStopMode\n", lookup_name(g_encoderStopModeStrs, pstSettings->mode));
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

static void TRNSX_Print_AudioEncoderOpenSettings( NEXUS_AudioEncoderSettings *  pstSettings )
{
    BKNI_Printf("  codec: %s  // support codecs are: mp3, aac, aacplus or lpcm_1394\n", lookup_name(g_audioCodecStrs, pstSettings->codec ));
    return;
}

static void TRNSX_Print_DisplaySettings(
    NEXUS_DisplaySettings * pstSettings
    )
{
    BKNI_Printf("  displayType:         %s  // NEXUS_DisplayType\n", lookup_name(g_displayTypeStrs, pstSettings->displayType));
    BKNI_Printf("  timingGenerator:     %s  // NEXUS_DisplayTimingGenerator\n", lookup_name(g_displayTimingGeneratorStrs, pstSettings->timingGenerator));
    BKNI_Printf("  format:              %s  // NEXUS_VideoFormat\n", lookup_name( g_videoFormatStrs, pstSettings->format));
    BKNI_Printf("  aspectRatio:         %s  // NEXUS_DisplayAspectRatio\n", lookup_name( g_displayAspectRatioStrs, pstSettings->aspectRatio));
    BKNI_Printf("  sampleAspectRatio.x: %u\n", pstSettings->sampleAspectRatio.x);
    BKNI_Printf("  sampleAspectRatio.y: %u\n", pstSettings->sampleAspectRatio.y);
    BKNI_Printf("  dropFrame:           %s  // NEXUS_TristateEnable\n", lookup_name( g_tristateEnableStrs, pstSettings->dropFrame));
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
    BKNI_Printf("  aspectRatio:         %s  // NEXUS_DisplayAspectRatio\n", lookup_name( g_displayAspectRatioStrs, pstSettings->aspectRatio));
    BKNI_Printf("  sampleAspectRatio.x: %u\n", pstSettings->sampleAspectRatio.x);
    BKNI_Printf("  sampleAspectRatio.y: %u\n", pstSettings->sampleAspectRatio.y);
    BKNI_Printf("  dropFrameAllowed:    %u\n", pstSettings->dropFrameAllowed);
    return;
}

static void TRNSX_Print_MuxOpenSettings( TRNSX_Transcode * pTrans, bool bDumpParams )
{
    BKNI_Printf("  transport:  %s  // <es,ivf,pes,mp4,ts,fmp4> \n", lookup_name(g_transportTypeStrs, pTrans->record.eTransportType));

    if ( bDumpParams )
    {
        BKNI_Printf("                  // for 'es', the buffers will be read directly\n");
        BKNI_Printf("                  // for 'ivf', 'pes' and 'mp4' the file mux will be used\n");
        BKNI_Printf("                  // for 'ts' and 'fmp4' the stream mux wil be used\n");
    }

    if ( pTrans->record.bGenerateIVF ) BKNI_Printf("                   // generating an IVF file\n");

    if ( pTrans->record.eTransportType == NEXUS_TransportType_eTs || bDumpParams )
    {
        BKNI_Printf("  -- stream mux settings --\n");
        BKNI_Printf("  userdata.enable:    %d        // enable user data pass through\n", pTrans->streamMux.userData.enable);
        BKNI_Printf("  userdata.remappids: %d        // enable remapping of user data PIDs\n", pTrans->streamMux.userData.bRemapPids);
        BKNI_Printf("  userdata.base_pid:  %x (%d)  // base PID when remapping is enabled\n", pTrans->streamMux.userData.uiBasePid, pTrans->streamMux.userData.uiBasePid);
        BKNI_Printf("  pcr_pid:            %x (%d)\n", pTrans->streamMux.uiPcrPid, pTrans->streamMux.uiPcrPid);
        BKNI_Printf("  video_pid:          %x (%d)\n", pTrans->streamMux.uiVideoPid, pTrans->streamMux.uiVideoPid);
        BKNI_Printf("  audio_pid:          %x (%d)\n", pTrans->streamMux.uiAudioPid, pTrans->streamMux.uiAudioPid);
        BKNI_Printf("  pmt_pid:            %x (%d)\n", pTrans->streamMux.uiPmtPid, pTrans->streamMux.uiPmtPid);
        BKNI_Printf("  pat_pid:            %x (%d)\n", pTrans->streamMux.uiPatPid, pTrans->streamMux.uiPatPid);
        BKNI_Printf("  pespacking:         %d       // enable audio PES packing\n", pTrans->streamMux.bAudioPesPacking);
        BKNI_Printf("  dspmixer:           %d       // enable DSP mixer\n", pTrans->streamMux.bDspMixer);
        BKNI_Printf("  force48kps:         %d       // force 48Kbps bitrate for HE AAC encoding.\n", pTrans->streamMux.bForce48KbpsAACplus);
        BKNI_Printf("  32khzaudio:         %d       // force 32KHz sample rate for audio encoding\n", pTrans->streamMux.b32KHzAudio);
        BKNI_Printf("  multichanfmt:       %s       // audio multi channel format: none, stereo, 5.1 or 7.1\n", lookup_name(g_audioChannelFormatStrs, pTrans->streamMux.eMultiChanFmt));
    }
    return;

}

static void TRNSX_Print_StreamMuxStartSettings(  TRNSX_Transcode * pTrans, bool bDumpParams )
{
    if ( pTrans->record.eTransportType == NEXUS_TransportType_eTs
        || pTrans->record.eTransportType == NEXUS_TransportType_eMp4Fragment
        || bDumpParams )
    {
        NEXUS_StreamMuxStartSettings * pstSettings = &pTrans->streamMux.startSettings;

        BKNI_Printf("  -- stream mux settings --\n");
        BKNI_Printf("  transportType    %s  // NEXUS_TransportType\n", lookup_name( g_transportTypeStrs, pstSettings->transportType ));
        /*BKNI_Printf("  supportTts       %s (NEXUS_TransportTimestampType)\n", lookup_name( g_transportTimestampTypeStrs, pstSettings->supportTts ));*/
        BKNI_Printf("  nonRealTimeRate  %u\n", pstSettings->nonRealTimeRate);
        BKNI_Printf("  servicePeriod    %u\n", pstSettings->servicePeriod);
        BKNI_Printf("  latencyTolerance %u\n", pstSettings->latencyTolerance);
        BKNI_Printf("  interleaveMode   %s  // NEXUS_StreamMuxInterleaveMode - escr or pts\n", lookup_name( g_streamMuxInterleaveModeStrs, pstSettings->interleaveMode ));
        BKNI_Printf("  useInitialPts    %u\n", pstSettings->useInitialPts);
        BKNI_Printf("  initialPts       %u\n", pstSettings->initialPts);
        BKNI_Printf("  pcr.interval     %u\n", pstSettings->pcr.interval);
        BKNI_Printf("  insertPtsOnlyOnFirstKeyFrameOfSegment %u\n", pstSettings->insertPtsOnlyOnFirstKeyFrameOfSegment);
    }
    else
    {
        BKNI_Printf("no addtional parameters can be set for transport type of %s\n", lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) );
    }

    return;
}

static void TRNSX_Print_RecordSettings( TRNSX_Transcode * pTrans )
{
    BKNI_Printf("  file:       %s\n", pTrans->record.fname);
    BKNI_Printf("  index file: %s  // need a comment about when the NAV file is generated\n", pTrans->record.indexfname);
    return;
}

static void TRNSX_Print_PlaybackOpenSettings( TRNSX_Transcode * pTrans )
{
    unsigned i=0;

    BKNI_Printf("  type:               %s  // <file/hdmi/qam> : source type, cuurently only support file\n", lookup_name(g_sourceStrs, pTrans->source.eType));
    BKNI_Printf("  file:               %s  // <file_name> : file is probed by default and the following parameters filled in\n", pTrans->source.fname);
    BKNI_Printf("  video_codec:        %s  // NEXUS_VideoCodec\n", lookup_name( g_videoCodecStrs, pTrans->source.eVideoCodec ));
    BKNI_Printf("  transport:          %s  // NEXUS_TransportType\n", lookup_name(g_transportTypeStrs,  pTrans->source.eStreamType));
    BKNI_Printf("  video_pid:          %x (%d)\n", pTrans->source.iVideoPid, pTrans->source.iVideoPid );
    BKNI_Printf("  pcr_pid:            %x (%d)\n", pTrans->source.iPcrPid, pTrans->source.iPcrPid );
    BKNI_Printf("  pmt_pid:            %x (%d)\n", pTrans->source.uiPmtPid, pTrans->source.uiPmtPid );
    BKNI_Printf("  userdata.numPids:   %d       // read only\n", pTrans->source.userData.numPids );
    do
    {
        BKNI_Printf("  userdata.pid[%d]:    %x (%d)", i, pTrans->source.userData.pids[i], pTrans->source.userData.pids[i] );
        if ( i == 0 ) BKNI_Printf("   // specifed as a comma separated list, i.e. userdata.pid:1,2,3,4...");
        BKNI_Printf("\n");
        i++;
    } while ( i < pTrans->source.userData.numPids );

    return;
}

static void TRNSX_Print_PlaypumpOpenSettings( NEXUS_PlaypumpOpenSettings * pstSettings )
{
    BKNI_Printf("  fifoSize:            0x%x\n", (uint32_t)pstSettings->fifoSize );
    BKNI_Printf("  numDescriptors:      %u\n", pstSettings->numDescriptors );
    BKNI_Printf("  streamMuxCompatible: %d\n", pstSettings->streamMuxCompatible );
    return;
}

static void TRNSX_Print_SelectSettings( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans )
{
    BSTD_UNUSED(pTrans);
    BKNI_Printf("  session:    %d  // select a session to modify\n", pCtxt->uiSelected );
    return;
}

static void TRNSX_Print_SessionSettings( TRNSX_TestContext *  pCtxt, TRNSX_Transcode * pTrans )
{
    BSTD_UNUSED(pCtxt);
    BKNI_Printf("  -- general settings --\n");
    BKNI_Printf("  nrt:       %d  // enable NRT mode, a gobal setting that is applied to the module specify commands\n", pTrans->bNonRealTime );
    BKNI_Printf("  cc:        %d  // enable CC encoding, a gobal setting that is applied to the module specify commands\n", pTrans->bEncodeCCData );
    BKNI_Printf("  audio:     %d  // enable audio encoding, a gobal setting that is applied to the module specify commands\n", pTrans->bEncodeAudio );
    BKNI_Printf("  lipsync:   %d  // enable precision lip sync, a gobal setting that is applied to the module specify commands\n", pTrans->syncChannel.bEnable );
    BKNI_Printf("\n  -- stream mux settings --\n");
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
    unsigned i;
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
    platformSettings.openFrontend = (pTrans->source.eType == TRNSX_Source_eQAM);
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


    for ( i=0; i<NEXUS_MAX_VIDEO_ENCODERS && i<TRNSX_MAX_SESSIONS; i++ )
    {
        pCtxt->uiMaxSessions += (pCtxt->encoderCapabilities.videoEncoder[i].supported) ? 1 : 0;
    }

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
    /* TODO: add a check that all encodes have been stopped/closed?
     * Or trust the user? Could use system wrapper routines to stop/close. */

    BDBG_MODULE_MSG(trnsx_dbg,("%s", BSTD_FUNCTION ));

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

            /* If the video encoder watchdogs, exit this thread. */
            if ( pTrans->videoEncoder.bEncoderWatchdogged == true )
            {
                bStoppedSignaled = true;
                bWaitingForEOS = false;
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
 * Utility routines.
 */

static void TRNSX_StcChannel_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_StcChannelSettings    stcSettings;

    BSTD_UNUSED(pCtxt);

    /* The different modules can be opened in any order.  To simplify the creation of the associated
     * STC's, open them all at the same time.  If one is valid, we know that they have all be opened. */

    if ( pTrans->videoDecoder.stcChannel )
    {
        goto done;
    }

    if ( pTrans->source.eType == TRNSX_Source_eFile )
    {

        /* All modules will be on the same timebase. Only two STC's per timebase (from transcode_ts.c)
         *
         * NRT mode:
         * - video decode uses a different STC
         * - audio decode and encoder use the same STC
         *
         * RT mode:
         * - video and audio decode use the same STC
         * - encoder uses different STC
         */

        if ( pTrans->bNonRealTime )
        {
            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0+pTrans->uiIndex;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;

            pTrans->videoDecoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            BDBG_ASSERT(pTrans->videoDecoder.stcChannel);

            pTrans->videoEncoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            pTrans->audioDecoder.stcChannel = pTrans->videoEncoder.stcChannel;
            BDBG_ASSERT(pTrans->videoEncoder.stcChannel);

            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: NRT mode video STC is unique, audio and encoder share a STC", BSTD_FUNCTION, pTrans->uiIndex ));
        }
        else
        {
            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0+pTrans->uiIndex;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;

            pTrans->videoDecoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            pTrans->audioDecoder.stcChannel = pTrans->videoDecoder.stcChannel;
            BDBG_ASSERT(pTrans->videoDecoder.stcChannel);

            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase    = NEXUS_Timebase_e0+pTrans->uiIndex;
            stcSettings.mode        = NEXUS_StcChannelMode_eAuto;
            stcSettings.pcrBits     = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
            stcSettings.autoConfigTimebase = false; /* from transcode_ts */

            pTrans->videoEncoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            BDBG_ASSERT(pTrans->videoEncoder.stcChannel);

            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: RT mode video and audio share a STC, encoder STC is unique", BSTD_FUNCTION, pTrans->uiIndex ));

        }
    }
    else
    {
        BDBG_ERR(("%s[%d]:: only support source type of file", BSTD_FUNCTION, pTrans->uiIndex ));
        BDBG_ASSERT(0);
    }

done:

    return;

}

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
    pTrans->streamMux.userData.enable = false;
    pTrans->streamMux.bDspMixer = true;
    pTrans->streamMux.bForce48KbpsAACplus = false;
    pTrans->streamMux.b32KHzAudio = false;
    pTrans->streamMux.eMultiChanFmt = NEXUS_AudioMultichannelFormat_eNone;


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
    pTrans->streamMux.configuration.audioPids           = 0;    /* no audio support for now bEncodingAudio ? 1 : 0; */
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

    /* user data settings */

    pTrans->streamMux.uiPcrPid          = BTST_MUX_PCR_PID;
    pTrans->streamMux.uiVideoPid        = BTST_MUX_VIDEO_PID;
    pTrans->streamMux.uiAudioPid        = BTST_MUX_AUDIO_PID;
    pTrans->streamMux.uiPmtPid          = BTST_MUX_PMT_PID;
    pTrans->streamMux.uiPatPid          = BTST_MUX_PAT_PID;
    pTrans->streamMux.userData.uiPid    = BTST_MUX_USER_DATA_PID;
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

    pTrans->source.eType = TRNSX_Source_eFile; /* just support files for now. */

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
#if 0     /* TODO: delete? */
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
#endif
    /* Video Decoder */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&pTrans->videoDecoder.startSettings);

    /* Audio Decoder - NEXUS_AudioDecoder_GetDefaultOpenSettings is called in TRNSX_AudioDecoder_Open */
    pTrans->audioDecoder.secondary.bEnabled = false;

    /* Audio Encoder*/
    NEXUS_AudioEncoder_GetDefaultSettings(&pTrans->audioEncoder.encoderSettings);
    pTrans->audioEncoder.encoderSettings.codec = NEXUS_AudioCodec_eAac;

    /* Record */
    pTrans->record.eTransportType = NEXUS_TransportType_eEs;
    pTrans->record.bWaitForEos = true;

    /* File Mux */
#if  NEXUS_HAS_FILE_MUX
    pTrans->fileMux.bSupported = true;
#endif

    /* Sync Channel */
#if NEXUS_HAS_SYNC_CHANNEL
    pTrans->syncChannel.bEnable = true;  /* by default, turn on the sync channel, it can be disabled at the session level */
#else
    pTrans->syncChannel.bEnable = false;
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
            BKNI_Printf("%s[%d]:: opened results file: %s\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->record.fname );
        }
        else {
            BKNI_Printf("%s[%d]:: failed to open results file: %s\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->record.fname );
            BDBG_ASSERT(pTrans->record.hFile);
        }

        pTrans->record.hIndexFile = fopen(pTrans->record.indexfname, "w");

        if ( pTrans->record.hIndexFile ) {
            BKNI_Printf("%s[%d]:: opened index file: %s\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->record.indexfname );
        }
        else {
            BKNI_Printf("%s[%d]:: failed to open index file: %s\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->record.indexfname );
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
 * Sync channel routines
 */

static void TRNSX_SyncChannel_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    bool bOpenSyncChannel;

    BSTD_UNUSED(pCtxt);

    /* The following is from transcode_ts, what does it mean? */
    /*BDBG_ASSERT(!g_bSimulXcode); *//* TODO: add simul audio mode */

    bOpenSyncChannel = pTrans->syncChannel.bEnable; /* for now keep lip sync user selectable */

    bOpenSyncChannel &= ( pTrans->audioDecoder.eState != TRNSX_State_eClosed ); /* audio is being decoded */
    bOpenSyncChannel &= ( pTrans->videoDecoder.eState != TRNSX_State_eClosed ); /* video is being decoded */

    bOpenSyncChannel &= ( pTrans->syncChannel.syncChannel == NULL ); /* the channel hasn't already been opened */

    if ( bOpenSyncChannel )
    {
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        pTrans->syncChannel.syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
        BDBG_ASSERT(pTrans->syncChannel.syncChannel);
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: opened sync channel", BSTD_FUNCTION, pTrans->uiIndex ));
    }

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif

    return;
}

static void TRNSX_SyncChannel_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
#if NEXUS_HAS_SYNC_CHANNEL

    BSTD_UNUSED(pCtxt);
    if ( pTrans->syncChannel.syncChannel )
    {
        NEXUS_SyncChannel_Destroy(pTrans->syncChannel.syncChannel);
        pTrans->syncChannel.syncChannel = NULL;
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: destroy sync channel", BSTD_FUNCTION, pTrans->uiIndex ));
    }

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif

    return;
}


static void TRNSX_SyncChannel_Connect(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;

    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    BSTD_UNUSED(pCtxt);

    /*if(pTrans->source.eType != TRNSX_Source_eHDMI && !pContext->bNoVideo ) */

    /* pTrans->syncChannel.syncChannel will only be non NULL if both
     * the audio and video decoders have been opened */

    if ( pTrans->syncChannel.syncChannel )
    {
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: connect sync channel", BSTD_FUNCTION, pTrans->uiIndex ));

        /* connect sync channel */
        NEXUS_SyncChannel_GetSettings(pTrans->syncChannel.syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder);

        if( bEncodingAudio )
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        else
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);

#if BTST_ENABLE_NRT_STC_AV_WINDOW
        /* NRT mode pairs AV stc channels */
        if(pTrans->bNonRealTime && !pContext->bNoVideo && !pInputSettings->bVideoWindowDisabled) {
            NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
            stcAudioVideoPair.connected = true;
            stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
            NEXUS_StcChannel_SetPairSettings(pContext->stcVideoChannel, pTrans->audioDecoder.stcChannel, &stcAudioVideoPair);
        }
#endif
        syncChannelSettings.enablePrecisionLipsync = false;/* to support 60->30 frc transcode */

        NEXUS_SyncChannel_SetSettings(pTrans->syncChannel.syncChannel, &syncChannelSettings);

    }

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif

    return;

}


static void TRNSX_SyncChannel_Disconnect(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
#if NEXUS_HAS_SYNC_CHANNEL

    BSTD_UNUSED(pCtxt);

    if ( pTrans->syncChannel.syncChannel )
    {
        NEXUS_SyncChannelSettings syncChannelSettings;

        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: disconnect sync channel", BSTD_FUNCTION, pTrans->uiIndex ));

        /* TODO: should this be gated by the state of the audio and video decoders?  */
        NEXUS_SyncChannel_GetSettings(pTrans->syncChannel.syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(pTrans->syncChannel.syncChannel, &syncChannelSettings);
    }

#else
    BSTD_UNUSED(pCtxt);
    BSTD_UNUSED(pTrans);
#endif

    return;
}

/*
 * Stream mux wrappers (ported from transcode_ts.c)
 */

/*******************************
 * Set up stream_mux and record
 */


/*******************************
 * Add system data to stream_mux
 */

static void TRNSX_StreamMux_RecpumpOverflowCallback(void *context, int param)
{
    TRNSX_Transcode * pTrans = context;
    BSTD_UNUSED(param);
    BDBG_ERR(("#### %s Context::%d stream mux recpump buffer overflows! ###", BSTD_FUNCTION, pTrans->uiIndex));
}

static void TRNSX_StreamMux_MessageOverflowCallback(void *context, int param)
{
    TRNSX_Transcode * pTrans = context;
    BDBG_ERR(("#### %s Context%d message PID[%d] buffer overflows! ###", BSTD_FUNCTION, pTrans->uiIndex, param));
}

static void TRNSX_StreamMux_PsiMessageCallback(void *context, int param)
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

/* cloned from transcode_ts::add_psi_filter */

int TRNSX_StreamMux_SystemData_AddPsiFilter(
    TRNSX_TestContext * pCtxt,
    TRNSX_Transcode * pTrans,
    unsigned short pid,
    BKNI_EventHandle event
    )
{
    unsigned i;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_Error rc;
    psi_message_t *msg = NULL;

    BSTD_UNUSED(pCtxt);

    for (i=0;i<BTST_MAX_MESSAGE_FILTERS;i++) {
        if (!pTrans->streamMux.userData.psi_message[i].message) {
            msg = &pTrans->streamMux.userData.psi_message[i];
            break;
        }
    }
    if (!msg) {
        return -1;
    }
    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: adding PSI filter[%u] for PID %u", BSTD_FUNCTION, pTrans->uiIndex, i, pid));

    /*if(pTrans->source.eType == TRNSX_Source_eFile) */
    {
        msg->pidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pid, NULL);
    }
#if 0
    else {
        msg->pidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pid, NULL);
    }
#endif
    BDBG_ASSERT(msg->pidChannel);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = TRNSX_StreamMux_PsiMessageCallback;
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
/* cloned from transcode_ts::getUserDataPsiFromPmt */
static void TRNSX_StreamMux_SystemData_GetUserDataPsiFromPmt(
    TRNSX_TestContext * pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    BKNI_EventHandle event;
    NEXUS_Error rc;
    unsigned i=0, count = 0, loop = 0;
    bool bFound = false;
    bool bStopPlayback=false;

    /* 1) parse input PAT and set up PMT filters if no specified PMT PID;
     * 2) parse each PMTs to find one with matching video PID;
     * 3) match selected user data stream PID;
     * 4) get user data descriptors;
     */


    if( pTrans->source.eType == TRNSX_Source_eFile && pTrans->playback.eState != TRNSX_State_eRunning )
    {
        TRNSX_Playback_Start( pCtxt, pTrans );
        bStopPlayback = true;
    }

    /* to get input PMT, need to set up message filter */
    BKNI_CreateEvent(&event);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: starting PSI filter PID = %u", BSTD_FUNCTION, pTrans->uiIndex, pTrans->source.uiPmtPid));

    /* if user specified PMT PID, msg[0] is for the PMT; else msg[0] is for PAT; */
    TRNSX_StreamMux_SystemData_AddPsiFilter(pCtxt, pTrans, pTrans->source.uiPmtPid, event);

    /* Read the PAT/PMT */
    for (count=0;;count++) {
        const uint8_t *buffer;
        size_t size;
        unsigned programNum, message_length, streamNum;
        size_t num = NEXUS_MAX_MUX_PIDS;

        if (count == BTST_MAX_MESSAGE_FILTERS) {
            count = 0;
            if(++loop > BTST_MAX_MESSAGE_FILTERS) {BDBG_ERR(("%s[%d]:: failed to get input user data PMT!", BSTD_FUNCTION, pTrans->uiIndex)); rc = -1; break;}
        }

        if (!pTrans->streamMux.userData.psi_message[count].message || pTrans->streamMux.userData.psi_message[count].done) {
            continue;
        }

        rc = NEXUS_Message_GetBuffer(pTrans->streamMux.userData.psi_message[count].message, (const void **)&buffer, &size);
        BDBG_ASSERT(!rc);

        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(event, 5 * 1000); /* wait 5 seconds */
            if (rc == NEXUS_TIMEOUT) {
                BDBG_WRN(("%s[%d]:: no PSI messages", BSTD_FUNCTION,pTrans->uiIndex));
                rc = -1;
                break;
            }
            BDBG_ASSERT(!rc);
            continue;
        }

        /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
        message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
        BDBG_ASSERT(size >= (size_t)message_length);
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: message[%u] size = "BDBG_UINT64_FMT", table ID = %u", BSTD_FUNCTION, pTrans->uiIndex, count, BDBG_UINT64_ARG((uint64_t)size), buffer[0]));

        if (buffer[0] == 0) {
            /* 1) Program Association Table */
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: PAT: size=%d", BSTD_FUNCTION, pTrans->uiIndex, message_length));
            for (programNum=0;programNum<(unsigned)(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
                unsigned byteOffset = 8 + programNum*4;
                unsigned program = TS_READ_16( &buffer[byteOffset] );
                unsigned short pid = (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF);
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]::  program %d: PID %u", BSTD_FUNCTION, pTrans->uiIndex, program, pid));
                /* add PMT filters for all programs */
                TRNSX_StreamMux_SystemData_AddPsiFilter(pCtxt, pTrans, pid, event);
            }
        }
        else if (buffer[0] == 2) { /* PMT */
            TS_PMT_stream pmtStream;

            /* Program Table */
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: found PMT PID[%u]:\nprogram number %d size=%d", BSTD_FUNCTION, pTrans->uiIndex,
                pTrans->streamMux.userData.psi_message[count].pid, TS_READ_16(&buffer[3]), message_length));
            /* need to validate the PMT section */
            if(!TS_PMT_validate(buffer, size)) {BDBG_ERR(("invalid PMT")); goto Done_getUserDataPsi;}

            streamNum = TS_PMT_getNumStreams(buffer, size);
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: total streams: %d", BSTD_FUNCTION, pTrans->uiIndex, streamNum));

            /* 2) search for all streams to match the video PID if no specified PMT PID */
            if( 0 == pTrans->source.uiPmtPid)
            {
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: PMT PID is 0, search PMT table", BSTD_FUNCTION,pTrans->uiIndex));
                for (i=0;i<streamNum;i++)
                {
                    TS_PMT_getStream(buffer, size, i, &pmtStream);
                    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: \tPID: %d, stream_type: %d", BSTD_FUNCTION, pTrans->uiIndex, pmtStream.elementary_PID, pmtStream.stream_type));
                    if(pmtStream.elementary_PID == pTrans->source.iVideoPid)
                    {
                        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: Found matching video PID!",BSTD_FUNCTION,pTrans->uiIndex));
                        break;
                    }
                }
                if(i == streamNum)
                {
                    /* not found so continue to next PMT */
                    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: pass:%d did not find PMT table with video PID:%d",BSTD_FUNCTION, pTrans->uiIndex,count, pTrans->source.iVideoPid ));
                    goto Done_getUserDataPsi;

                }
            }
            bFound = true;/* found PMT */

            /* else, found the matching program */
            /* 3) search for all streams to extract user data PSI info */
            for (i=0;i<streamNum;i++) {
                unsigned streamOffset;

                TS_PMT_getStream(buffer, size, i, &pmtStream);

                /* 2) match user data PID */
                if(pTrans->streamMux.userData.numPids == BTST_TS_USER_DATA_ALL)
                {
                    /* all pass the VBI user data PES streams */
                    if(pmtStream.stream_type != TS_PSI_ST_13818_1_PrivatePES) continue;

                    if(num == NEXUS_MAX_MUX_PIDS) num = 0;/* start from 0 */
                    else {
                        ++num;
                        if(num >= NEXUS_MAX_MUX_PIDS) break;
                    }
                    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: \tset source user data PID: %d with elementary_PID: %x (%d)",
                                            BSTD_FUNCTION, pTrans->uiIndex, num, pmtStream.elementary_PID,pmtStream.elementary_PID));
                    pTrans->source.userData.pids[num] = pmtStream.elementary_PID;
                } else {
                    for(num=0; num < pTrans->streamMux.userData.numPids; num++) {
                        /* 3) bingo! save the stream info and stream descriptor */
                        if(pmtStream.elementary_PID == (uint16_t)pTrans->source.userData.pids[num]) {
                            break;
                        }
                    }
                    /* not found, check next stream */
                    if(num == pTrans->streamMux.userData.numPids) continue;
                }

                /* 4) save user data PSI info */
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: \tuser data PID: %d, stream_type: %d\n", BSTD_FUNCTION, pTrans->uiIndex, pmtStream.elementary_PID, pmtStream.stream_type));
                /* save pmt stream info and remap PID */
                pTrans->streamMux.userData.streams[num].stPMTStream = pmtStream;

                if(pTrans->streamMux.userData.bRemapPids) {
                    pTrans->streamMux.userData.streams[num].pid = pTrans->streamMux.userData.uiBasePid+num;
                    pTrans->streamMux.userData.streams[num].stPMTStream.elementary_PID = pTrans->streamMux.userData.streams[num].pid;
                } else {
                    pTrans->streamMux.userData.streams[num].pid = pTrans->streamMux.userData.streams[num].stPMTStream.elementary_PID;
                }
                /* save stream descriptor size */
                streamOffset = TS_PMT_P_getStreamByteOffset(buffer, size, i);
                pTrans->streamMux.userData.streams[num].iLengthDesc = TS_READ_16(&buffer[streamOffset+3])&0x3FF;
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: \tdescriptor length: %d\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->streamMux.userData.streams[num].iLengthDesc));

                /* sanity check descriptor size */
                if(pTrans->streamMux.userData.streams[num].iLengthDesc > 188) {
                BDBG_ERR(("%s[%d]:: User data descriptor length %d too long!",BSTD_FUNCTION, pTrans->uiIndex,  pTrans->streamMux.userData.streams[num].iLengthDesc));
                    pTrans->streamMux.userData.streams[num].bValid = false;/* invalidate */
                    goto Done_getUserDataPsi;
                }
                BKNI_Memcpy(pTrans->streamMux.userData.streams[num].dataDescriptors,&buffer[streamOffset+5],pTrans->streamMux.userData.streams[num].iLengthDesc);
                /* mark it valid finally */
                pTrans->streamMux.userData.streams[num].bValid = true;
            }
            if(pTrans->streamMux.userData.numPids == BTST_TS_USER_DATA_ALL) {
                pTrans->streamMux.userData.numPids = num+1;/* found num of user data streams */
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: found "BDBG_UINT64_FMT" user data PIDs to pass through.", BSTD_FUNCTION, pTrans->uiIndex, BDBG_UINT64_ARG((uint64_t)pTrans->streamMux.userData.numPids)));
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
        rc = NEXUS_Message_ReadComplete(pTrans->streamMux.userData.psi_message[count].message, message_length);
        BDBG_ASSERT(!rc);

        pTrans->streamMux.userData.psi_message[count].done = true; /* don't parse this table any more */

        /* Only do once. TODO: may periodically watch for updated PMT info. */
        if(bFound) break;
    }

    for (i=0;i<BTST_MAX_MESSAGE_FILTERS;i++) {
        if (pTrans->streamMux.userData.psi_message[i].message) {
            NEXUS_Message_Close(pTrans->streamMux.userData.psi_message[i].message);
            pTrans->streamMux.userData.psi_message[i].message = NULL;
        }
        if (pTrans->streamMux.userData.psi_message[i].pidChannel) {
            if(pTrans->source.eType == TRNSX_Source_eFile) {
                NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->streamMux.userData.psi_message[i].pidChannel);
            }
            else {
                NEXUS_PidChannel_Close(pTrans->streamMux.userData.psi_message[i].pidChannel);
            }
            pTrans->streamMux.userData.psi_message[i].pidChannel = NULL;
        }
    }
    /* free the event resource */
    BKNI_DestroyEvent(event);

    if ( bStopPlayback == true )
    {
        /* TODO: are there cases when playback should be closed as well? */
        TRNSX_Playback_Stop( pCtxt, pTrans );
    }


}

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */


#define BTST_TS_HEADER_BUF_LENGTH   189

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

    TS_PAT_program_Init(&program, 1, pTrans->streamMux.uiPmtPid);
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
    if(pTrans->streamMux.userData.enable && pTrans->source.eType != TRNSX_Source_eHDMI)
    {
        TRNSX_StreamMux_SystemData_GetUserDataPsiFromPmt(pCtxt, pTrans);

        for(i=0; i<NEXUS_MAX_MUX_PIDS; i++)
        {
            if(pTrans->streamMux.userData.streams[i].bValid) {
                TS_PMT_addStream(&pmtState, &pTrans->streamMux.userData.streams[i].stPMTStream, &streamNum);
                TS_PMT_setDescriptor(&pmtState,
                    pTrans->streamMux.userData.streams[i].dataDescriptors,
                    pTrans->streamMux.userData.streams[i].iLengthDesc,
                    streamNum);
            }
        }
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: output PMT section:", BSTD_FUNCTION, pTrans->uiIndex));

    for(i=0; i < (int)pmtState.size; i+=8)
    {
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: %02x %02x %02x %02x %02x %02x %02x %02x", BSTD_FUNCTION, pTrans->uiIndex,
        pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTrans->streamMux.pat[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTrans->streamMux.pat[0] + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, pTrans->streamMux.uiPmtPid, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTrans->streamMux.pmt[0], BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTrans->streamMux.pmt[0] + buf_used, pmt_pl_buf, pmt_pl_size);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: output PMT packet:", BSTD_FUNCTION, pTrans->uiIndex));

    for(i=0; i < BTST_TS_HEADER_BUF_LENGTH; i+=8)
    {
        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: %02x %02x %02x %02x %02x %02x %02x %02x", BSTD_FUNCTION, pTrans->uiIndex,
            *((uint8_t*)pTrans->streamMux.pmt[0]+i),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+1),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+2),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+3),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+4),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+5),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+6),
            *((uint8_t*)pTrans->streamMux.pmt[0]+i+7)));
    }

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: insert PAT&PMT... ccPAT = %x ccPMT=%x",
                        BSTD_FUNCTION, pTrans->uiIndex,
                        *((uint8_t*)pTrans->streamMux.pat[pTrans->streamMux.ccValue % BTST_PSI_QUEUE_CNT] + 4) & 0xf,
                        *((uint8_t*)pTrans->streamMux.pmt[pTrans->streamMux.ccValue  % BTST_PSI_QUEUE_CNT] + 4) & 0xf));

    if(pTrans->streamMux.systemdataTimerIsStarted)
    {
        pTrans->streamMux.systemdataTimer = B_Scheduler_StartTimer(
            pTrans->streamMux.schedulerSystemdata,pTrans->streamMux.mutexSystemdata, 1000, TRNSX_StreamMux_SystemData_InsertTimer, pTrans);
        if(pTrans->streamMux.systemdataTimer==NULL) {BDBG_ERR(("%s[%d]:: schedule timer error %d", BSTD_FUNCTION, pTrans->uiIndex,  NEXUS_OUT_OF_SYSTEM_MEMORY));}
    }
    return;
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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->playback.eState )));

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eOpened;


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
                BDBG_ERR(("%s:: video encoder codec %d is not supported!\n", BSTD_FUNCTION, pTrans->videoEncoder.startSettings.codec));
                BDBG_ASSERT(0);
        }
    }
#if 0
    if(pContext->encodeSettings.bAudioEncode)
    {/* audio transcode */
        audCodec = pTrans->audioEncoder.encoderSettings.codec;
        audPid = pTrans->streamMux.uiAudioPid;
    }
    else if(bEncodingAudio)
    {/* audio passthrough */
        audCodec = pContext->inputSettings.eAudioCodec;
        audPid = pTrans->streamMux.uiAudioPid;
    }
    if(bEncodingAudio)
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

    TRNSX_StreamMux_SystemData_AddPatPmt(
                pCtxt, pTrans,
                (0 != pTrans->streamMux.startSettings.pcr.interval ) ? pTrans->streamMux.uiPcrPid : 0,
                pTrans->streamMux.uiVideoPid, audPid, vidStreamType, audStreamType);

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
    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: insert PAT&PMT... ccPAT = %x ccPMT=%x", BSTD_FUNCTION, pTrans->uiIndex,
                        *((uint8_t*)pTrans->streamMux.pat[0] + 4) & 0xf,
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
static void TRNSX_StreamMux_AV_Sync(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    unsigned Dee = 0;

    BSTD_UNUSED(pCtxt);

    /******************************************
     * add configurable delay to video path
     */
    /* disable Inverse Telecine Field Pairing for extreme low delay mode
     * NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
     * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */

    /*if(!pContext->bNoVideo) */
    {
        /* TODO: give the use of key:value pairs, is any of the following needed?
         * Should any of the following defaults be used?
         * Can the block just be deleted? */

        /*****************************************
         * calculate video encoder A2P delay
         */
        /* NOTE: video encoder delay is in 27MHz ticks; the min is based on the bound settings. */

        /* TODO: also called in TRNSX_VideoEncoder_Start */

        NEXUS_VideoEncoder_GetDelayRange(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings, &pTrans->videoEncoder.startSettings, &videoDelay);
        BKNI_Printf("%s[%d]:: video encoder end-to-end delay = [%u ~ %u] ms\n", BSTD_FUNCTION, pTrans->uiIndex, videoDelay.min/27000, videoDelay.max/27000);

        Dee = videoDelay.min;
    }

    if( bEncodingAudio )
    {
        NEXUS_AudioMuxOutput_GetDelayStatus(pTrans->audioMuxOutput.audioMuxOutput, pTrans->audioEncoder.encoderSettings.codec, &audioDelayStatus);
        BKNI_Printf("%s[%d]:: audio codec: %s  end-to-end delay = %u ms\n",
                        BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_audioCodecStrs, pTrans->audioEncoder.encoderSettings.codec), audioDelayStatus.endToEndDelay);

        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */

        /*if(!pContext->bNoVideo) */
        {
            if(Dee > videoDelay.min)
            {
                if(Dee > videoDelay.max)
                {
                    BDBG_ERR(("%s[%d]:: Audio Dee is way too big! Use video Dee max!", BSTD_FUNCTION, pTrans->uiIndex));
                    Dee = videoDelay.max;
                }
                else
                {
                    BKNI_Printf("%s[%d]:: Use audio Dee %u ms %u ticks@27Mhz!\n", BSTD_FUNCTION, pTrans->uiIndex, Dee/27000, Dee);
                }
            }
            else
            {
                Dee = videoDelay.min;
                BKNI_Printf("%s[%d]:: Use video Dee %u ms %u ticks@27Mhz!\n", BSTD_FUNCTION, pTrans->uiIndex, Dee/27000, Dee);
            }
            pTrans->videoEncoder.settings.encoderDelay = Dee;
        }

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&pTrans->audioMuxOutput.startSettings);

        /* audio NRT requires mux out to take NRT audio decode STC which is assigned with transcode STC. */
        /* NOTE: NEXUS_AudioMuxOutput_Start is called in TRNSX_StreamMux_Start, i.e.
         * that is where audioMuxOutput.startSettings is used */
        pTrans->audioMuxOutput.startSettings.stcChannel        = pTrans->videoEncoder.stcChannel;
        pTrans->audioMuxOutput.startSettings.presentationDelay = Dee/27000;/* in ms */
        pTrans->audioMuxOutput.startSettings.nonRealTime       = pTrans->bNonRealTime;

    }
    else
        pTrans->videoEncoder.settings.encoderDelay = Dee;

}

/* cloned from transcode::open_transcode */
/* This rouinte will only be called if audio is being encoded. */

static void TRNSX_StreamMux_OpenAudio(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{


    /* Note: this routine will only be called if audio is being encoded. */
    /* TODO: make a wrapper around audio encoder open or is this really audio encoder open? */

#if 1

    if ( pTrans->audioDecoder.audioDecoder == NULL )
    {
        BDBG_ERR(("%s: audio decoder is not open", BSTD_FUNCTION ));
        BDBG_ASSERT(0);
    }


    /*if(pInputSettings->bAudioInput)*/
    if(1)
    {
        NEXUS_AudioMixerSettings audioMixerSettings;

        /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
        if(pTrans->streamMux.bDspMixer) {
            NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
            audioMixerSettings.mixUsingDsp = true;
            /*if(pEncodeSettings->bAudioEncode) */
            if ( 1 )
            {
                audioMixerSettings.outputSampleRate = 48000;/* fixed to allow gap filling for bogus audio PID */
            }
            audioMixerSettings.dspIndex    = pTrans->uiIndex / BTST_NUM_AUDIO_XCODE_PER_DSP;
            pTrans->audioDecoder.audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
            assert(pTrans->audioDecoder.audioMixer);
            BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioMixer_Open", BSTD_FUNCTION, pTrans->uiIndex ));
        }

        /* Open audio mux output */
        pTrans->audioMuxOutput.audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
        assert(pTrans->audioMuxOutput.audioMuxOutput);
        BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioMuxOutput_Create", BSTD_FUNCTION, pTrans->uiIndex ));

        /*if(pEncodeSettings->bAudioEncode || (pTrans->source.eType == TRNSX_Source_eHDMI))*/
        if ( 1 )
        {
            NEXUS_AudioEncoderCodecSettings codecSettings;
            NEXUS_AudioCodec audioCodec;

            /* Open audio encoder */
            audioCodec = pTrans->audioEncoder.encoderSettings.codec;
            pTrans->audioEncoder.audioEncoder = NEXUS_AudioEncoder_Open(&pTrans->audioEncoder.encoderSettings);
            assert(pTrans->audioEncoder.audioEncoder);
            BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioEncoder_Open", BSTD_FUNCTION, pTrans->uiIndex ));

            if(audioCodec == NEXUS_AudioCodec_eAacPlus && pTrans->streamMux.bForce48KbpsAACplus) {
                NEXUS_AudioEncoder_GetCodecSettings(pTrans->audioEncoder.audioEncoder, audioCodec, &codecSettings);
                codecSettings.codecSettings.aacPlus.bitRate = 48000;
                NEXUS_AudioEncoder_SetCodecSettings(pTrans->audioEncoder.audioEncoder, &codecSettings);
                BDBG_WRN(("%s[%d]:: Force AAC plus bitrate = 48Kbps", BSTD_FUNCTION, pTrans->uiIndex ));
            }

            /* TODO: should much of the following be moved to TRNSX_StreamMux_Start?
             * That might make it so that audio and the mux could be opened in any order? */

            if(!pTrans->streamMux.bDspMixer) {
                /* Connect decoder to encoder */
                NEXUS_AudioEncoder_AddInput(pTrans->audioEncoder.audioEncoder,
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                BDBG_WRN(("%s[%d]:: No DSP mixer...", BSTD_FUNCTION, pTrans->uiIndex ));
            }
            else {
                /* Connect decoder to mixer and set as master */
                NEXUS_AudioMixer_AddInput(pTrans->audioDecoder.audioMixer,
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder,
                        (pTrans->streamMux.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                        NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo));
                if(pTrans->audioDecoder.secondary.bEnabled) {
                    NEXUS_AudioMixer_AddInput(pTrans->audioDecoder.audioMixer,
                        NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.secondary.audioDecoder,
                            (pTrans->streamMux.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                            NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo));
                }
                audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder,
                        (pTrans->streamMux.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo && USE_DDRE)?
                        NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo);
                if(pTrans->streamMux.b32KHzAudio) audioMixerSettings.outputSampleRate = 32000;
                NEXUS_AudioMixer_SetSettings(pTrans->audioDecoder.audioMixer, &audioMixerSettings);

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
                        NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
                    NEXUS_DolbyDigitalReencode_AddInput(pContext->ddre,
                        NEXUS_DolbyVolume258_GetConnector(pContext->dv258));

                    /* Connect dv258 to encoder */
                    NEXUS_AudioEncoder_AddInput(pTrans->audioEncoder.audioEncoder,
                        NEXUS_DolbyDigitalReencode_GetConnector(pContext->ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
                #else
                    /* Connect mixer to encoder */
                    NEXUS_AudioEncoder_AddInput(pTrans->audioEncoder.audioEncoder,
                        NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
                #endif
            }

            /* Connect mux to encoder */
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput),
                NEXUS_AudioEncoder_GetConnector(pTrans->audioEncoder.audioEncoder));
        }
#if 0
        else
        {
            if(!pTrans->streamMux.bDspMixer) {
                /* Connect decoder to mux out */
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput),
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            }
            else {
                NEXUS_AudioMixer_AddInput(pTrans->audioDecoder.audioMixer,
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
                audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);
                NEXUS_AudioMixer_SetSettings(pTrans->audioDecoder.audioMixer, &audioMixerSettings);
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput),
                    NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
            }
            audioCodec = audioProgram.codec; /* pTrans->source.eAudioCodec*/
        }
#endif
        /* Attach outputs for real-time transcoding */
        if(!pTrans->bNonRealTime)
        {
            if(!pTrans->streamMux.bDspMixer)
            {
#if 1
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]),
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo ));
#else
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]),
                    NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, pEncodeSettings->bAudioEncode?
                        NEXUS_AudioDecoderConnectorType_eStereo :
                        NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
            }
            else {
            #if USE_DDRE
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]),
                    NEXUS_DolbyDigitalReencode_GetConnector(pContext->ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
            #else
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]),
                    NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
            #endif
            }
        }
    }


#if where_to_do_this
    TRNSX_StreamMux_AV_Sync( pCtxt, pTrans );
#endif /* where_to_do_this */

#endif

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
    bool bTtsOutput = (NEXUS_TransportTimestampType_eNone != pTrans->streamMux.transportTimestampType);

    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s encode_audio:%d",
                        BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->streamMux.eState ), bEncodingAudio ));
#if 0
    if ( bEncodingAudio )
    {
        /*TRNSX_StreamMux_OpenAudio( pCtxt, pTrans );*/
        /* TODO: is the the right place for this?*/
        TRNSX_StreamMux_AV_Sync( pCtxt, pTrans );
    }
#endif
    /* initialize with the number of pids specified by the user */
    pTrans->streamMux.userData.numPids = ( pTrans->source.userData.numPids ) ? pTrans->source.userData.numPids : BTST_TS_USER_DATA_ALL;

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eOpened;

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
        if(!pTrans->streamMux.userData.enable) {
            streamMuxConfig.userDataPids = 0;/* remove unnecessary memory allocation */
        } else if(pTrans->streamMux.userData.numPids == BTST_TS_USER_DATA_ALL) {
            streamMuxConfig.userDataPids = NEXUS_MAX_MUX_PIDS;
        } else {
            streamMuxConfig.userDataPids = pTrans->streamMux.userData.numPids;
        }
        streamMuxConfig.audioPids           = bEncodingAudio ? 1 : 0;
        streamMuxConfig.latencyTolerance    = pTrans->streamMux.muxLatencyTolerance;
        streamMuxConfig.servicePeriod       = g_msp;
        streamMuxConfig.nonRealTime         = pTrans->bNonRealTime;
        streamMuxConfig.muxDelay            = pCtxt->encodeDelay;/* A2P delay affects mux resource allocation */
        streamMuxConfig.supportTts          = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);

        NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfig,&pTrans->streamMux.createSettings.memoryConfiguration);
    }
#else
    /* reduce stream mux memory allocation if no TS user data passthru */

    if(!pTrans->streamMux.userData.enable) {
        pTrans->streamMux.configuration.userDataPids = 0;/* remove unnecessary memory allocation */
    } else if(pTrans->streamMux.userData.numPids == BTST_TS_USER_DATA_ALL) {
        pTrans->streamMux.configuration.userDataPids = NEXUS_MAX_MUX_PIDS;
    } else {
        pTrans->streamMux.configuration.userDataPids = pTrans->streamMux.userData.numPids;
    }

    /* TOD0: transcode_x used 1 or 0, is this a better approach? */
    pTrans->streamMux.configuration.audioPids = ( bEncodingAudio ) ? pTrans->source.uiNumAudio : 0;

    /* set pCtxt->encodeDelay; A2P delay affects mux resource allocation */
    /* TODO: set after the video encoder starts?  Use pTrans->videoEncoder.settings.encoderDelay? */
    /* TODO: the call to NEXUS_VideoEncoder_GetDelayRange is needs to move for this to work. */

    if ( pTrans->videoEncoder.eState != TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s[%d]:: videoEncoder.eState: %s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));
        BDBG_ERR(("The state should be running, videoEncoder.settings.encoderDelay is initialized in TRNSX_VideoEncoder_Start"));
    }

    pTrans->streamMux.configuration.muxDelay = pTrans->videoEncoder.settings.encoderDelay;

    pTrans->streamMux.configuration.nonRealTime = pTrans->bNonRealTime;
    pTrans->streamMux.configuration.supportTts = (pTrans->streamMux.transportTimestampType != NEXUS_TransportTimestampType_eNone);

    NEXUS_StreamMux_GetMemoryConfiguration( &pTrans->streamMux.configuration, &pTrans->streamMux.createSettings.memoryConfiguration );
    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: pTrans->streamMux.configuration:: videoPids:%d audioPids:%d userDataPids:%d", BSTD_FUNCTION,pTrans->uiIndex,
                        pTrans->streamMux.configuration.videoPids,
                        pTrans->streamMux.configuration.audioPids,
                        pTrans->streamMux.configuration.userDataPids ));
#endif

    pTrans->streamMux.streamMux = NEXUS_StreamMux_Create(&pTrans->streamMux.createSettings);
    assert(pTrans->streamMux.streamMux);

    pTrans->streamMux.startSettings.stcChannel = pTrans->videoEncoder.stcChannel; /*TODO: pCtxt->stcChannelTranscode, is this right, what about audio? */
    pTrans->streamMux.startSettings.nonRealTime = pTrans->bNonRealTime; /* TODO: should this be exposed separately to the user? */

    /*if(!pCtxt->bNoVideo) */
    if( 1 )
    {
        pTrans->streamMux.startSettings.video[0].pid = pTrans->streamMux.uiVideoPid;
        pTrans->streamMux.startSettings.video[0].encoder = pTrans->videoEncoder.videoEncoder;
        pTrans->streamMux.startSettings.video[0].playpump = pTrans->streamMux.playpump; /*pCtxt->playpumpTranscodeVideo;*/
    }


    if( bEncodingAudio )
    {
        /* TODO: will there ever be a need for a separate playpump for audio? */
#if will_this_be_needed
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
#endif
        pTrans->streamMux.startSettings.audio[0].pid        = pTrans->streamMux.uiAudioPid;
        pTrans->streamMux.startSettings.audio[0].muxOutput  = pTrans->audioMuxOutput.audioMuxOutput;
        pTrans->streamMux.startSettings.audio[0].playpump   = pTrans->streamMux.playpump; /*pCtxt->playpumpTranscodeAudio;*/
        pTrans->streamMux.startSettings.audio[0].pesPacking = pTrans->streamMux.bAudioPesPacking;

    }

    pTrans->streamMux.startSettings.pcr.pid         = pTrans->streamMux.uiPcrPid;
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


    if(pTrans->streamMux.userData.enable && pTrans->source.eType != TRNSX_Source_eHDMI)
    {
        unsigned i;
        NEXUS_MessageSettings               messageSettings;
        NEXUS_MessageStartSettings          messageStartSettings;
        NEXUS_PlaybackPidChannelSettings    playbackPidSettings;

        NEXUS_Message_GetDefaultSettings(&messageSettings);
        /* SCTE 270 spec max TS VBI user data bitrate=270Kbps, 256KB buffer can hold 7.5 seconds;
           worthy user data for video synchronization; TODO: may be reduced if unnecessary */
        messageSettings.bufferSize = 512*1024;
        messageSettings.maxContiguousMessageSize = 0; /* to support TS capture and in-place operation */
        messageSettings.overflow.callback = TRNSX_StreamMux_MessageOverflowCallback; /* report overflow error */
        messageSettings.overflow.context  = pTrans;

        /* open source user data PID channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther; /* capture the TS packets with the user data PES */
        playbackPidSettings.pidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;

        BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pTrans->streamMux.userData.numPids);
        for(i = 0; i < pTrans->streamMux.userData.numPids; i++)
        {
            if(pTrans->streamMux.userData.streams[i].bValid)
            {
                messageSettings.overflow.param = pTrans->streamMux.userData.streams[i].pid;

                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: opened message buffer for user data source PID %d record PID %d",
                            BSTD_FUNCTION,
                            pTrans->uiIndex,
                            pTrans->source.userData.pids[i],
                            pTrans->streamMux.userData.streams[i].pid));

                pTrans->streamMux.startSettings.userdata[i].message = NEXUS_Message_Open(&messageSettings);
                BDBG_ASSERT(pTrans->streamMux.startSettings.userdata[i].message);

                /*pTrans->streamMux.userDataMessage[i] = pTrans->streamMux.startSettings.userdata[i].message;*/

                if(pTrans->streamMux.userData.bRemapPids) {
                    playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
                    playbackPidSettings.pidSettings.pidSettings.remap.pid     = pTrans->streamMux.userData.streams[i].pid;/* optional PID remap */

                    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: enable userdata PID remapping, record PID: %d(%x)",
                                               BSTD_FUNCTION,
                                               pTrans->uiIndex,
                                               playbackPidSettings.pidSettings.pidSettings.remap.pid,
                                               playbackPidSettings.pidSettings.pidSettings.remap.pid));
                }

                pTrans->streamMux.pidChannelUserData[i] = NEXUS_Playback_OpenPidChannel(
                                                        pTrans->playback.playback,
                                                        pTrans->source.userData.pids[i],
                                                        &playbackPidSettings);

                BDBG_ASSERT(pTrans->streamMux.pidChannelUserData[i]);

                /* must start message before stream mux starts */
                NEXUS_Message_GetDefaultStartSettings(pTrans->streamMux.startSettings.userdata[i].message, &messageStartSettings);
                messageStartSettings.format = NEXUS_MessageFormat_eTs;
                messageStartSettings.pidChannel = pTrans->streamMux.pidChannelUserData[i];
                NEXUS_Message_Start(pTrans->streamMux.startSettings.userdata[i].message, &messageStartSettings);

                /* open transcode mux output user data PidChannels out of system data channel */
                pTrans->streamMux.pidChannelTranscodeUserData[i] = NEXUS_Playpump_OpenPidChannel(
                                                                        pTrans->streamMux.playpump, /*pCtxt->playpumpTranscodePcr,*/
                                                                        pTrans->streamMux.userData.streams[i].pid,
                                                                        NULL);
                BDBG_ASSERT(pTrans->streamMux.pidChannelTranscodeUserData[i]);
            }
        }
    }


    /* open PidChannels */
    pTrans->streamMux.pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, pTrans->streamMux.startSettings.pcr.pid, NULL);
    assert(pTrans->streamMux.pidChannelTranscodePcr);
    pTrans->streamMux.pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, pTrans->streamMux.uiPmtPid, NULL);
    assert(pTrans->streamMux.pidChannelTranscodePmt);
    pTrans->streamMux.pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(pTrans->streamMux.playpump /*pCtxt->playpumpTranscodePcr*/, pTrans->streamMux.uiPatPid, NULL);
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

static void TRNSX_StreamMux_CloseAudio(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: bEncodingAudio:%d", BSTD_FUNCTION, pTrans->uiIndex, bEncodingAudio));

    if(bEncodingAudio)
    {
        /*if(pContext->encodeSettings.bAudioEncode || (pTrans->source.eType == TRNSX_Source_eHDMI))*/
        if ( 1 )
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput));
            NEXUS_AudioEncoder_RemoveAllInputs(pTrans->audioEncoder.audioEncoder);
            NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(pTrans->audioEncoder.audioEncoder));
            NEXUS_AudioEncoder_Close(pTrans->audioEncoder.audioEncoder);
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioMixer_RemoveAllInputs(pTrans->audioDecoder.audioMixer);
            }
            if(!pTrans->bNonRealTime) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]));
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]));
            }
#if USE_DDRE
            NEXUS_DolbyDigitalReencode_RemoveAllInputs(pContext->ddre);
            NEXUS_DolbyDigitalReencode_Close(pContext->ddre);
            NEXUS_DolbyVolume258_RemoveAllInputs(pContext->dv258);
            NEXUS_DolbyVolume258_Close(pContext->dv258);
#endif
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
                NEXUS_AudioMixer_Close(pTrans->audioDecoder.audioMixer);
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            NEXUS_AudioDecoder_Close(pTrans->audioDecoder.audioDecoder);
            if(pTrans->audioDecoder.secondary.bEnabled) {
                NEXUS_AudioDecoder_Close(pTrans->audioDecoder.secondary.audioDecoder);
            }
        }
        else
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput));
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioMixer_RemoveAllInputs(pTrans->audioDecoder.audioMixer);
            }
            if(!pTrans->bNonRealTime) {
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]));
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(pCtxt->platformConfig.outputs.audioDummy[pTrans->uiIndex]));
            }
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pTrans->audioDecoder.audioMixer));
                NEXUS_AudioMixer_Close(pTrans->audioDecoder.audioMixer);
            }
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pTrans->audioDecoder.audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            NEXUS_AudioDecoder_Close(pTrans->audioDecoder.audioDecoder);
        }

#if 0
        if ( pContext->playpumpTranscodeAudio != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeAudio);
#endif
        NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(pTrans->audioMuxOutput.audioMuxOutput));
        NEXUS_AudioMuxOutput_Destroy(pTrans->audioMuxOutput.audioMuxOutput);

        pTrans->audioMuxOutput.audioMuxOutput = NULL;

    }

    return;

}


static void TRNSX_StreamMux_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    size_t i;

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

    if(pTrans->streamMux.userData.enable && pTrans->source.eType != TRNSX_Source_eHDMI) {
        for(i = 0; i < pTrans->streamMux.userData.numPids; i++) {
            if(pTrans->streamMux.userData.streams[i].bValid) {
                NEXUS_Message_Stop(pTrans->streamMux.startSettings.userdata[i].message /*pTrans->streamMux.userDataMessage[i]*/);
                NEXUS_Message_Close(pTrans->streamMux.startSettings.userdata[i].message/*pTrans->streamMux.userDataMessage[i]*/);
            }
        }
    }

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
    if(pTrans->source.eType == TRNSX_Source_eFile && (!g_activeXcodeCount || !g_bSimulXcode))
    {
        NEXUS_Playback_CloseAllPidChannels(pTrans->playback.playback);
        NEXUS_FilePlay_Close(pContext->file);
        NEXUS_Playback_Destroy(pTrans->playback.playback);
        NEXUS_Playpump_Close(pContext->playpump);
        pContext->playpump = NULL;
    }
    else if(pTrans->source.eType == TRNSX_Source_eQAM)
    {
        if(bEncodingAudio)
            NEXUS_PidChannel_Close(pTrans->audioDecoder.pidChannel);
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
        if(pTrans->source.eType == TRNSX_Source_eHDMI) {
            NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(pContext->hdmiInput));
        }
        else
#endif
        {
#if NEXUS_HAS_SYNC_CHANNEL
            /* disconnect sync channel after decoders stop */
            if(bEncodingAudio) {
                NEXUS_SyncChannel_Destroy(pTrans->syncChannel.syncChannel);
            }
#endif
            if(g_bEnableDebugSdDisplay && (pTrans->uiIndex == 0))
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
        if(pTrans->uiIndex == 0 && g_bEnableDebugSdDisplay) {
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

    /* Handled in AudioDecoder_Close*/
    /*TRNSX_StreamMux_CloseAudio( pCtxt, pTrans );*/


#if 0
    if ( NULL != pContext->playpumpTranscodeMCPB ) NEXUS_Playpump_Close(pContext->playpumpTranscodeMCPB);
#endif

    /* Handled in encode and decoder close? */
#if 0
    /* NOTE: each transcoder context only needs two separate STCs;
       if NRT mode, the audio STC is the same as transcode STC; if RT mode, audio STC is the same as video STC. */
    if(!g_activeXcodeCount || !g_bSimulXcode) {
        NEXUS_StcChannel_Close(pContext->stcVideoChannel);
        NEXUS_StcChannel_Close(pTrans->videoEncoder.stcChannel);
    }
#endif

#if 0
#if NEXUS_HAS_HDMI_INPUT
    if(pTrans->source.eType == TRNSX_Source_eHDMI) {
        NEXUS_HdmiInput_Close(pContext->hdmiInput);
    }
#endif
#if NEXUS_HAS_FRONTEND
    if(pTrans->source.eType == TRNSX_Source_eQAM) {
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
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

#if NEXUS_HAS_STREAM_MUX
    NEXUS_PidChannelHandle     pidChannelTranscodeVideo;


    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_RecordPidChannelSettings recordPidSettings;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s encode_audio:%d",
                               BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->streamMux.eState ), bEncodingAudio));


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
    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: pTrans->streamMux.startSettings:: video pid[0]:%0x pcr pid[0]:%0x audio pid:%0x",
                        BSTD_FUNCTION,
                        pTrans->uiIndex,
                        (unsigned int)pTrans->streamMux.startSettings.video[0].pid,
                        (unsigned int)pTrans->streamMux.startSettings.pcr.pid,
                        (unsigned int)pTrans->streamMux.startSettings.audio[0].pid));


    rc = NEXUS_StreamMux_Start(pTrans->streamMux.streamMux,&pTrans->streamMux.startSettings, &muxOutput);
    BDBG_ASSERT(!rc);

    pidChannelTranscodeVideo = muxOutput.video[0];

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: muxOutput.video[0]: %0x (NEXUS_PidChannelHandle) muxOutput.audio[0]: %0x (NEXUS_PidChannelHandle)",
                               BSTD_FUNCTION,
                               pTrans->uiIndex,
                               (unsigned int)muxOutput.video[0],
                               (unsigned int)muxOutput.audio[0]));

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
            rc = NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pidChannelTranscodeVideo, &recordPidSettings);
            BDBG_ASSERT(!rc);
        }
    }


    /* for audio channel */
    if( bEncodingAudio )
    {
        NEXUS_PidChannelHandle pidChannelTranscodeAudio = muxOutput.audio[0];
#if 0
        if(pTrans->streamMux.bRemux) {/* mux -> remux -> record */
            NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodeAudio);
            pContext->pidRemuxAudio = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.startSettings.audio[0].pid, NULL);
            NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxAudio, &recordPidSettings);
        }
        else
#endif
        {
            rc = NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pidChannelTranscodeAudio, NULL);
            BDBG_ASSERT(!rc);
        }
    }

#if 0
    /* for system data channel */
    if(pTrans->streamMux.bRemux) {/* mux -> remux -> parser -> record */
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePcr);
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePat);
        NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodePmt);

        pContext->pidRemuxPcr = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.startSettings.pcr.pid, NULL);
        BDBG_ASSERT(pContext->pidRemuxPcr);
        pContext->pidRemuxPmt = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.uiPmtPid, NULL);
        BDBG_ASSERT(pContext->pidRemuxPmt);
        pContext->pidRemuxPat = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.uiPatPid, NULL);
        BDBG_ASSERT(pContext->pidRemuxPat);
        /* it seems the null packets would be recorded without explicitly added here as parser band enabled allPass; */
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPcr, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPat, NULL);
        NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pContext->pidRemuxPmt, NULL);
        if(pTrans->streamMux.userData.enable && pTrans->source.eType != TRNSX_Source_eHDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pTrans->streamMux.userData.numPids);
            for(i = 0; i < pTrans->streamMux.userData.numPids; i++) {
                if(pTrans->streamMux.userData.streams[i].bValid) {
                    NEXUS_Remux_AddPidChannel(pContext->remux, pTrans->streamMux.pidChannelTranscodeUserData[i]);
                    pContext->pidRemuxUserData[i] = NEXUS_PidChannel_Open(pContext->parserBandRemux, pTrans->streamMux.userData.streams[i].pid, NULL);
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
        rc = NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePcr, NULL);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePat, NULL);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Record_AddPidChannel(pTrans->streamMux.record, pTrans->streamMux.pidChannelTranscodePmt, NULL);
        BDBG_ASSERT(!rc);
#if 1
        if(pTrans->streamMux.userData.enable && pTrans->source.eType != TRNSX_Source_eHDMI) {
            size_t i;
            BDBG_ASSERT(NEXUS_MAX_MUX_PIDS >= pTrans->streamMux.userData.numPids);
            for(i = 0; i < pTrans->streamMux.userData.numPids; i++) {
                if(pTrans->streamMux.userData.streams[i].bValid) {
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
            BDBG_ERR(("Transcoder[%u] failed set fifo record", pTrans->uiIndex)); BDBG_ASSERT(0);
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
    if((pTrans->source.eType != TRNSX_Source_eHDMI) && !pContext->bNoStopDecode && !pContext->bNoVideo
        && (pTrans->uiIndex == g_simulDecoderMaster || !g_bSimulXcode))
    {
        NEXUS_VideoDecoderExtendedSettings extSettings;

        /* register the 3DTV status change callback */
        NEXUS_VideoDecoder_GetExtendedSettings(pContext->videoDecoder, &extSettings);
        extSettings.s3DTVStatusEnabled = true;
        extSettings.s3DTVStatusChanged.callback = decode3dCallback;
        extSettings.s3DTVStatusChanged.context = pContext->videoDecoder;
        extSettings.s3DTVStatusChanged.param = pTrans->uiIndex;
        NEXUS_VideoDecoder_SetExtendedSettings(pContext->videoDecoder, &extSettings);

        /* Start decoder */
        NEXUS_VideoDecoder_Start(pContext->videoDecoder, &pContext->vidProgram);
    }
#endif

    if( bEncodingAudio )
    {

        if ( pTrans->videoEncoder.eState != TRNSX_State_eRunning )
        {
            BDBG_ERR(("%s[%d]:: videoEncoder.eState: %s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));
            BDBG_ERR(("The state should be running, audioMuxOutput.startSettings is initialized by calls made in TRNSX_VideoEncoder_Start"));
        }

        /* Start audio mux output, needs to be here not in TRNSX_AudioDecoder_Start. */
        rc = NEXUS_AudioMuxOutput_Start(pTrans->audioMuxOutput.audioMuxOutput, &pTrans->audioMuxOutput.startSettings);
        if ( NEXUS_SUCCESS != rc ) {
            BDBG_ERR(("%s:%d - NEXUS_AudioMuxOutput_Start failed", __FILE__, __LINE__));
            BDBG_ASSERT(0);
        }

        if( 1 )
        /*if(!pContext->bNoStopDecode) */
        {
            /* Start audio mixer, must be started after Audio Mux */
            if(pTrans->streamMux.bDspMixer) {
                rc = NEXUS_AudioMixer_Start(pTrans->audioDecoder.audioMixer);
                if ( NEXUS_SUCCESS != rc ) {
                    BDBG_ERR(("%s:%d - NEXUS_AudioMixer_Start failed", __FILE__, __LINE__));
                    BDBG_ASSERT(0);
                }
            }

            /* Audio decoders are started in TRNSX_AudioDecoder_Start.
             * TODO: is it okay to start the decoder before AudioMux and AudioMixer? */
#if 0
            /*rc = NEXUS_AudioDecoder_Start(pTrans->audioDecoder.audioDecoder, &pContext->audProgram);*/
            rc = TRNSX_AudioDecoder_Start( pCtxt, pTrans );
            if ( NEXUS_SUCCESS != rc ) {
                BDBG_ERR(("%s:%d - NEXUS_AudioDecoder_Start(PRIMARY) failed", __FILE__, __LINE__));
                return rc;
            }
            if(pTrans->audioDecoder.secondary.bEnabled) {
                BDBG_WRN(("Starting Secondary audio"));
                rc = NEXUS_AudioDecoder_Start(pTrans->audioDecoder.secondary.audioDecoder, &pContext->secondaryProgram);
                if ( NEXUS_SUCCESS != rc ) {
                    BDBG_ERR(("%s:%d - NEXUS_AudioDecoder_Start(SECONDARY) failed", __FILE__, __LINE__));
                    return rc;
                }
            }
#endif
        }
    }

    /* Start playback before mux set up PAT/PMT which may depend on PMT user data PSI extraction */
    /* Handled elsewhere, this can be deleted. */

#if 0
    if((pTrans->source.eType == TRNSX_Source_eFile) && !pContext->bNoStopDecode
        && (pTrans->uiIndex == g_simulDecoderMaster || !g_bSimulXcode))
    {
        NEXUS_Playback_Start(pTrans->playback.playback, pContext->file, NULL);
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
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

#if NEXUS_HAS_STREAM_MUX
    /*NEXUS_VideoEncoderStopSettings videoEncoderStopSettings;*/

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    /* TODO: add error checking for proper sequencing. */
    pTrans->streamMux.eState = TRNSX_State_eStopped;

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
    if((pTrans->source.eType == TRNSX_Source_eFile) && !pContext->bNoStopDecode)
    {
        if(pTrans->uiIndex == g_simulDecoderMaster || !g_bSimulXcode) {
            NEXUS_Playback_Stop(pTrans->playback.playback);
        }
    }
#endif

#if 1
    /* TODO: should this be in TRNSX_AudioDecoder_Stop? */
    if( bEncodingAudio )
    {
        /*if(!pContext->bNoStopDecode) */
        if ( 1 )
        {
            NEXUS_AudioDecoder_Stop(pTrans->audioDecoder.audioDecoder);
            BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioDecoder_Stop", BSTD_FUNCTION, pTrans->uiIndex ));

            if(pTrans->audioDecoder.secondary.bEnabled) {
                NEXUS_AudioDecoder_Stop(pTrans->audioDecoder.secondary.audioDecoder);
            }
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioMixer_Stop(pTrans->audioDecoder.audioMixer);
                BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioMixer_Stop", BSTD_FUNCTION, pTrans->uiIndex ));
            }
        }
        NEXUS_AudioMuxOutput_Stop(pTrans->audioMuxOutput.audioMuxOutput);
        BDBG_MODULE_MSG(trnsx_dbg_audio,("%s[%d]:: NEXUS_AudioMuxOutput_Stop", BSTD_FUNCTION, pTrans->uiIndex ));
    }
#endif

#if 0

     if(!pContext->bNoVideo) {
         if(pTrans->source.eType != TRNSX_Source_eHDMI && !pContext->bNoStopDecode) {
             if(pTrans->uiIndex == g_simulDecoderMaster || !g_bSimulXcode)
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
                fprintf(stderr, "%s[%d]:: timed out waiting for pTrans->streamMux.finishEvent\n", BSTD_FUNCTION, pTrans->uiIndex);
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
 #if 1
     if ( bEncodingAudio )
     {
         const NEXUS_AudioMuxOutputFrame *pBuf,*pBuf2;
         size_t size,size2;
         NEXUS_Error errCode;

         do {
            errCode = NEXUS_AudioMuxOutput_GetBuffer(pTrans->audioMuxOutput.audioMuxOutput, &pBuf, &size, &pBuf2, &size2);
            if ( BERR_SUCCESS == errCode )
            {
                size += size2;
                if ( size > 0 )
                {
                    BDBG_WRN(("Flushing "BDBG_UINT64_FMT" outstanding audio descriptors", BDBG_UINT64_ARG((uint64_t)size)));
                    NEXUS_AudioMuxOutput_ReadComplete(pTrans->audioMuxOutput.audioMuxOutput, size);
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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: transport type %s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType)));

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: transport type %s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType)));

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
static NEXUS_Error TRNSX_Playback_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->playback.eState )));

    /*if ( pTrans->playback.eState == TRNSX_State_eClosed )*/
    if ( 1 )
    {
        if ( pTrans->playback.playpump == NULL )
        {
            pTrans->playback.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: called NEXUS_Playpump_Open", BSTD_FUNCTION, pTrans->uiIndex));
            BDBG_ASSERT(pTrans->playback.playpump);
        }

        if ( pTrans->playback.playback == NULL )
        {
            pTrans->playback.playback = NEXUS_Playback_Create(); /* Calls NEXUS_Playback_GetDefaultSettings(). */
            BDBG_MODULE_MSG(trnsx_dbg,("%s:[%d]::called NEXUS_Playback_Create", BSTD_FUNCTION, pTrans->uiIndex));
            BDBG_ASSERT(pTrans->playback.playback);

            NEXUS_Playback_GetSettings(pTrans->playback.playback, &pTrans->playback.playbackSettings);
            pTrans->playback.playbackSettings.playpump = pTrans->playback.playpump;
            pTrans->playback.playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)pTrans->source.eStreamType;
            pTrans->playback.playbackSettings.stcChannel = pTrans->videoDecoder.stcChannel;
            NEXUS_Playback_SetSettings(pTrans->playback.playback, &pTrans->playback.playbackSettings);
        }

        if ( pTrans->playback.hFilePlay == NULL )
        {
            pTrans->playback.hFilePlay = NEXUS_FilePlay_OpenPosix(pTrans->source.fname, NULL);
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: called NEXUS_FilePlay_OpenPosix", BSTD_FUNCTION, pTrans->uiIndex));

            if (!pTrans->playback.hFilePlay) {
                fprintf(stderr, "%s[%d]:: can't open file:%s\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->source.fname);
                return -1;
            }
        }

        /* Open the video pid channel */
        if ( pTrans->videoDecoder.videoDecoder && pTrans->videoDecoder.videoPidChannel == NULL )
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&pTrans->playback.playbackPidSettings);
            pTrans->playback.playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pTrans->playback.playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pTrans->source.eVideoCodec; /* must be told codec for correct handling */
            pTrans->playback.playbackPidSettings.pidTypeSettings.video.index = true;
            pTrans->playback.playbackPidSettings.pidTypeSettings.video.decoder = pTrans->videoDecoder.videoDecoder;
            pTrans->videoDecoder.videoPidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pTrans->source.iVideoPid, &pTrans->playback.playbackPidSettings);
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: opened video PID channel", BSTD_FUNCTION, pTrans->uiIndex));

            if(pTrans->source.iVideoPid != pTrans->source.iPcrPid)
            {
                pTrans->playback.playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
                pTrans->videoDecoder.pcrPidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pTrans->source.iPcrPid, &pTrans->playback.playbackPidSettings);
                BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: opened video PCR channel", BSTD_FUNCTION, pTrans->uiIndex));
            }
        }

        /* Open the audio pid channel. */
        if ( pTrans->audioDecoder.audioDecoder && pTrans->audioDecoder.pidChannel == NULL )
        {
            if(pTrans->source.eType == TRNSX_Source_eFile)
            {
                NEXUS_PlaybackPidChannelSettings playbackPidSettings;
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);

                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
                playbackPidSettings.pidTypeSettings.audio.primary = pTrans->audioDecoder.audioDecoder;

                if(pTrans->audioDecoder.secondary.bEnabled) {
                    playbackPidSettings.pidTypeSettings.audio.secondary = pTrans->audioDecoder.secondary.audioDecoder;
                }

                pTrans->audioDecoder.pidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pTrans->source.iAudioPid, &playbackPidSettings);

                if(pTrans->audioDecoder.secondary.bEnabled) {
                    /* TODO: how to specify secondary PID? */
                    /*pTrans->audioDecoder.secondary.pidChannel = NEXUS_Playback_OpenPidChannel(pTrans->playback.playback, pInputSettings->iSecondaryPid, &playbackPidSettings);*/
                    BDBG_ERR(("%s[%d]:: need to specify a secondary PID", BSTD_FUNCTION, pTrans->uiIndex ));
                    BDBG_ASSERT(0);
                }
            }
            else
            {
                /* from transcode_ts*/
#if 0
#if NEXUS_HAS_FRONTEND
                        if(pTrans->source.eType == TRNSX_Source_eQAM)
                        {
                            pTrans->audioDecoder.pidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pTrans->source.iAudioPid, NULL);
                            if(pTrans->audioDecoder.secondary.bEnabled) {
                                pTrans->audioDecoder.secondary.pidChannel = NEXUS_PidChannel_Open(pContext->parserBand, pInputSettings->iSecondaryPid, NULL);
                            }
                        }
                        else
#endif
#endif

                BDBG_ERR(("%s[%d]:: have only implemented PID channels for file playback", BSTD_FUNCTION, pTrans->uiIndex ));
                BDBG_ASSERT(0);
            }
        }

    }

    pTrans->playback.eState = TRNSX_State_eOpened;

    return rc;

}

static NEXUS_Error TRNSX_Playback_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->playback.eState )));

    if ( pTrans->playback.eState == TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s[%d]:: playback must be stopped before it can be closed.", BSTD_FUNCTION, pTrans->uiIndex ));
        rc = NEXUS_UNKNOWN;
    }

#if 0
    if ( pTrans->videoDecoder.videoPidChannel )
    {
        NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.videoPidChannel);
        pTrans->videoDecoder.videoPidChannel = NULL;
    }

    if(NULL != pTrans->videoDecoder.pcrPidChannel)
    {
        NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.pcrPidChannel);
        pTrans->videoDecoder.pcrPidChannel = NULL;
    }
#else
    NEXUS_Playback_CloseAllPidChannels(pTrans->playback.playback);
    pTrans->videoDecoder.videoPidChannel = NULL;
    pTrans->videoDecoder.pcrPidChannel = NULL;
    pTrans->audioDecoder.pidChannel = NULL;
#endif

    if ( pTrans->playback.hFilePlay )
    {
        NEXUS_FilePlay_Close(pTrans->playback.hFilePlay);
        pTrans->playback.hFilePlay = NULL;
    }

    if ( pTrans->playback.playback )
    {
        NEXUS_Playback_Destroy(pTrans->playback.playback);
        pTrans->playback.playback = NULL;
    }

    if ( pTrans->playback.playpump )
    {
        NEXUS_Playpump_Close(pTrans->playback.playpump);
        pTrans->playback.playpump = NULL;
    }

    pTrans->playback.eState = TRNSX_State_eClosed;

    return rc;

}

static NEXUS_Error TRNSX_Playback_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    TRNSX_Playback_Open( pCtxt, pTrans );

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->playback.eState )));

    NEXUS_Playback_Start(pTrans->playback.playback, pTrans->playback.hFilePlay, NULL);

    pTrans->playback.eState = TRNSX_State_eRunning;

    return rc;
}

static NEXUS_Error TRNSX_Playback_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pCtxt);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->playback.eState )));

    NEXUS_Playback_Stop(pTrans->playback.playback);

    pTrans->playback.eState = TRNSX_State_eStopped;

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
    unsigned displayIndex = 0;

    if ( encoderId >= NEXUS_MAX_VIDEO_ENCODERS )
    {
        BDBG_ERR(("%s: encoder index %d is >= NEXUS_MAX_VIDEO_ENCODERS %d", BSTD_FUNCTION, encoderId, NEXUS_MAX_VIDEO_ENCODERS));
    }
    else if ( pCtxt->encoderCapabilities.videoEncoder[encoderId].supported )
    {
        displayIndex = pCtxt->encoderCapabilities.videoEncoder[encoderId].displayIndex;
        BDBG_MODULE_MSG(trnsx_dbg,("%s: encoder:%d using display index: %d", BSTD_FUNCTION, encoderId, displayIndex));
    }
    else
    {
        BDBG_ERR(("%s: there is not a display associated with encoder %d", BSTD_FUNCTION, encoderId));
    }

    return displayIndex;

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
        BDBG_ERR(("%s[%d]:: the encoder must be opened before the display", BSTD_FUNCTION, pTrans->uiIndex ));
        return NEXUS_NOT_INITIALIZED;
    }

    if ( pTrans->videoEncoder.display )
    {
        BDBG_ERR(("%s[%d]:: the display has already been opened", BSTD_FUNCTION, pTrans->uiIndex ));
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
        BDBG_MODULE_MSG(trnsx_dbg, ("%s[%d]:: calling NEXUS_Display_SetCustomFormatSettings", BSTD_FUNCTION, pTrans->uiIndex));
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
 * Audio decoder wrappers.
 */

/*
 inputSettings.bAudioInput  // primary enable? == bEncodingAudio

 inputSettings.bPcmAudio // if true, audio must be encoded, only for HDMI input

 encodeSettings.bAudioEncode
 encodeSettings.encoderAudioCodec
 codecSettings.codecSettings.aac.sampleRate
 codecSettings.codecSettings.aac.bitRate
 case 26: and case 27:
 g_bMultiChanAudio
 g_bSecondAudio


 only encode audio if the user enable and the stream contains audio

 ----------------------------------------------------

 pContext->audioDecoder == pTrans->audioDecoder.audioDecoder


 */

static NEXUS_Error TRNSX_AudioDecoder_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s encode_audio:%d",
                            BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->audioDecoder.eState ), bEncodingAudio));

    pTrans->audioDecoder.eState = TRNSX_State_eOpened;


    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);


    audioDecoderOpenSettings.dspIndex = pTrans->uiIndex / BTST_NUM_AUDIO_XCODE_PER_DSP;

#if 0
    if(g_bMultiChanAudio) {
        audioDecoderOpenSettings.multichannelFormat = pTrans->streamMux.eMultiChanFmt;
    }
#endif

    /* TODO: What index to pass in, same as video decoder, i.e. pTrans->uiIndex? */

    pTrans->audioDecoder.audioDecoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &audioDecoderOpenSettings);

    if(pTrans->audioDecoder.secondary.bEnabled) {
        pTrans->audioDecoder.secondary.audioDecoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &audioDecoderOpenSettings);
    }

    TRNSX_StcChannel_Open( pCtxt,  pTrans );

    TRNSX_SyncChannel_Open( pCtxt, pTrans );  /* will be opened if all the condition are met */

    if ( bEncodingAudio )
    {
        TRNSX_StreamMux_OpenAudio( pCtxt, pTrans );
    }


    /* from transcode_ts:config_xcoder_context*/
#if 0
    if(bEncodingAudio)
    {
        if (pTrans->source.eType == TRNSX_Source_eHDMI)
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
            pTrans->audioEncoder.encoderSettings.codec = pContext->inputSettings.eAudioCodec;
            if(g_bMultiChanAudio) {
                printf("\n Multi-channel audio format: \n");
                print_value_list(g_audioChannelFormatStrs);
                pTrans->streamMux.eMultiChanFmt = getNameValue(input, g_audioChannelFormatStrs);
            }
            if(pTrans->audioDecoder.secondary.bEnabled) {
                printf("\n Secondary Audio Pid:                ");
                pContext->inputSettings.iSecondaryPid = getValue(input);
            }
        }
    }

    if(pContext->encodeSettings.bAudioEncode)
    {
    }
    else {
        if(pTrans->audioDecoder.secondary.bEnabled) {printf("Multi audio streams cannot be passed through...\n"); BDBG_ASSERT(0);}
    }

#endif



    /* TODO: any error checking of the state? */
    pTrans->audioDecoder.eState = TRNSX_State_eOpened;

    return rc;
}

static NEXUS_Error TRNSX_AudioDecoder_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->audioDecoder.eState )));

    if ( pTrans->audioDecoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s[%d]:: the decoder is %s. It must be stopped before you can call close.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->audioDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->audioDecoder.eState = TRNSX_State_eClosed;

    /* TODO: this routine is basically a NOP at the moment.  The auido decoder close is
     * currently handled in TRNSX_StreamMux_CloseAudio. */

    TRNSX_StreamMux_CloseAudio( pCtxt, pTrans );

    TRNSX_SyncChannel_Close( pCtxt, pTrans );

#if video_decoder_close

    /*NEXUS_VideoWindow_RemoveInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));*/

    /* Do the PID channels need to be closed prior to calling NEXUS_VideoInput_Shutdown?
     * That seems to be the case based on experimenting. */
    TRNSX_Playback_Close( pCtxt, pTrans );

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    NEXUS_VideoDecoder_Close(pTrans->videoDecoder.videoDecoder);
    pTrans->videoDecoder.videoDecoder = NULL;

#endif


    /* TODO: since the audio decoder will share a STC channel with either the video decoder or encoder,
     * does the following call be in a wrapper? */
#if 0
    NEXUS_StcChannel_Close(pTrans->audioDecoder.stcChannel);
    pTrans->audioDecoder.stcChannel = NULL;
#endif

#if 0
    if(pTrans->source.eType == TRNSX_Source_eFile && (!g_activeXcodeCount || !g_bSimulXcode))
    {
        NEXUS_Playback_CloseAllPidChannels(pTrans->playback.playback);
        NEXUS_FilePlay_Close(pContext->file);
        NEXUS_Playback_Destroy(pTrans->playback.playback);
        NEXUS_Playpump_Close(pContext->playpump);
        pContext->playpump = NULL;
    }
    else if(pTrans->source.eType == TRNSX_Source_eQAM)
    {
        if(bEncodingAudio)
            NEXUS_PidChannel_Close(pTrans->audioDecoder.pidChannel);
        if(pContext->videoPidChannel)
            NEXUS_PidChannel_Close(pContext->videoPidChannel);
        if(pContext->pcrPidChannel) {
            NEXUS_PidChannel_Close(pContext->pcrPidChannel);
            pContext->pcrPidChannel = NULL;
        }
    }

#endif


    return rc;
}

static NEXUS_Error TRNSX_AudioDecoder_Start(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AudioDecoderStartSettings audioStartSettings;
    bool bEncodingAudio = TRNSX_CHECK_ENCODE_AUDIO(pTrans);

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s encode_audio:%d",
            BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->audioDecoder.eState ), bEncodingAudio ));

    /* Needs to be called to open audioDecoder.pidChannel,
     * which can't be opened until playback is open*/
    TRNSX_Playback_Open( pCtxt, pTrans );

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioStartSettings);

#if 0
    if(pTrans->source.eType == TRNSX_Source_eHDMI) {
#if NEXUS_HAS_HDMI_INPUT
        audioStartSettings.input = NEXUS_HdmiInput_GetAudioConnector(pContext->hdmiInput);
        audioStartSettings.latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;/* dsp mixer after hdmi cannot use low delay mode */
#endif
    }
    else
#endif
    {
        audioStartSettings.codec = pTrans->source.eAudioCodec;
        audioStartSettings.pidChannel = pTrans->audioDecoder.pidChannel;
        audioStartSettings.nonRealTime = pTrans->bNonRealTime;
    }
    audioStartSettings.stcChannel = pTrans->audioDecoder.stcChannel;

#if 0
    if(pTrans->audioDecoder.secondary.bEnabled) {
        audioStartSettings.pidChannel = pTrans->audioDecoder.secondary.pidChannel;
        audioStartSettings.secondaryDecoder = true;   /* Indicate this is a secondary channel for STC Channel/PCRlib functions */
        pContext->secondaryProgram = audioStartSettings;
    }
#endif

#if 1
    /* TODO: is this conditional needed? Should the user be trusted? */

    if( bEncodingAudio )
    {

        /* Start audio mux output */
/*
        rc = NEXUS_AudioMuxOutput_Start(pTrans->audioMuxOutput.audioMuxOutput, &pTrans->audioMuxOutput.startSettings);
        BDBG_ASSERT(!rc);
*/
        /*if(!pContext->bNoStopDecode) */
        if ( 1 )
        {

            /* Start audio mixer */
/*
            if(pTrans->streamMux.bDspMixer) {
                rc = NEXUS_AudioMixer_Start(pTrans->audioDecoder.audioMixer);
                BDBG_ASSERT(!rc);
            }
*/
            /* TODO: right place for this call? */
            TRNSX_SyncChannel_Connect( pCtxt, pTrans );

            rc = NEXUS_AudioDecoder_Start(pTrans->audioDecoder.audioDecoder, &audioStartSettings);
            BDBG_ASSERT(!rc);

#if 0
            if(pTrans->audioDecoder.secondary.bEnabled) {
                BDBG_WRN(("Starting Secondary audio"));
                rc = NEXUS_AudioDecoder_Start(pTrans->audioDecoder.secondary.audioDecoder, &pContext->secondaryProgram);
                BDBG_ASSERT(!rc));
            }
#endif
        }
    }

#endif

    /* TODO: any error checking of the state? */
    pTrans->audioDecoder.eState = TRNSX_State_eRunning;

    return rc;
}

static NEXUS_Error TRNSX_AudioDecoder_Stop(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->audioDecoder.eState )));

    /* currently handled in TRNSX_StreamMux_Stop.*/

    /* TODO: right place for this call? */
    TRNSX_SyncChannel_Disconnect( pCtxt, pTrans );

#if 0
    if(bEncodingAudio)
    {
        if(!pContext->bNoStopDecode) {
            NEXUS_AudioDecoder_Stop(pTrans->audioDecoder.audioDecoder);
            if(pTrans->audioDecoder.secondary.bEnabled) {
                NEXUS_AudioDecoder_Stop(pTrans->audioDecoder.secondary.audioDecoder);
            }
            if(pTrans->streamMux.bDspMixer) {
                NEXUS_AudioMixer_Stop(pTrans->audioDecoder.audioMixer);
            }
        }
        NEXUS_AudioMuxOutput_Stop(pTrans->audioMuxOutput.audioMuxOutput);
    }

    /* Temporary workaround to flush pending descriptors from NEXUS_AudioMuxOutput prior to restarting it.
       Restarting will flush the pending descriptors. */
#if 1
    if (bEncodingAudio)
    {
        const NEXUS_AudioMuxOutputFrame *pBuf,*pBuf2;
        size_t size,size2;
        NEXUS_Error errCode;

        do {
           errCode = NEXUS_AudioMuxOutput_GetBuffer(pTrans->audioMuxOutput.audioMuxOutput, &pBuf, &size, &pBuf2, &size2);
           if ( BERR_SUCCESS == errCode )
           {
               size += size2;
               if ( size > 0 )
               {
                   BDBG_WRN(("Flushing "BDBG_UINT64_FMT" outstanding audio descriptors", BDBG_UINT64_ARG((uint64_t)size)));
                   NEXUS_AudioMuxOutput_ReadComplete(pTrans->audioMuxOutput.audioMuxOutput, size);
               }
           }
        } while ( size > 0 );
    }
#endif

#endif


    /* TODO: any error checking of the state? */
    pTrans->audioDecoder.eState = TRNSX_State_eStopped;

    return rc;
}

static NEXUS_Error TRNSX_AudioDecoder_GetStatus(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{

    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( pTrans->audioDecoder.audioDecoder )
    {
        NEXUS_AudioDecoderStatus audioDecoderStatus;
        NEXUS_AudioDecoder_GetStatus(pTrans->audioDecoder.audioDecoder, &audioDecoderStatus);

        BKNI_Printf("Audio Decoder[%d] Status:\n", pTrans->uiIndex);
        TRNSX_Print_AudioDecoderStatus( &audioDecoderStatus );

        if ( pTrans->audioMuxOutput.audioMuxOutput )
        {
            NEXUS_AudioMuxOutputStatus audioMuxOutputStatus;
            NEXUS_AudioMuxOutput_GetStatus(pTrans->audioMuxOutput.audioMuxOutput, &audioMuxOutputStatus);

            BKNI_Printf("Audio mux output[%d] Status:\n", pTrans->uiIndex);
            TRNSX_Print_AudioMuxOutputStatus( &audioMuxOutputStatus );
        }
    }
    else
    {
        BKNI_Printf("%s: audio decoder has not been opened yet\n", BSTD_FUNCTION );
    }

    /* from transcode_ts::printStatus */
#if 0
/* HDMI input cannot measure AV sync via original PTS correlation */
if(bEncodingAudio && !pContext->bNoVideo &&
   (pTrans->source.eType != TRNSX_Source_eHDMI))
   avsync_correlation_error(pContext);

#endif


    return rc;
}



/*
 * Video decoder wrappers
 */
static NEXUS_Error TRNSX_VideoDecoder_Open(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoDecoder.eState )));

    if ( pTrans->videoDecoder.eState != TRNSX_State_eClosed )
    {
        BDBG_ERR(("%s[%d]:: the decoder is %s. It must closed before you can call open.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eOpened;

    pTrans->videoDecoder.videoDecoder = NEXUS_VideoDecoder_Open(pTrans->uiIndex, NULL); /* take default capabilities */

    TRNSX_StcChannel_Open( pCtxt,  pTrans );

    TRNSX_SyncChannel_Open( pCtxt, pTrans );  /* will be opened if all the conditions are met */

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoDecoder.eState )));

    if ( pTrans->videoDecoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s[%d]:: the decoder is %s. It must be stopped before you can call close.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eClosed;

#if SIMUL_DISPLAY
    NEXUS_VideoWindow_RemoveInput(pCtxt->simulDisplay.window,          NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));
#endif

    /*NEXUS_VideoWindow_RemoveInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));*/

    /* TODO: Move the following two close calls to TRNSX_Playback_Close?*/
#if 0
    NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.videoPidChannel);
    pTrans->videoDecoder.videoPidChannel = NULL;

    if(NULL != pTrans->videoDecoder.pcrPidChannel)
    {
        NEXUS_Playback_ClosePidChannel(pTrans->playback.playback, pTrans->videoDecoder.pcrPidChannel);
        pTrans->videoDecoder.pcrPidChannel = NULL;
    }

    NEXUS_FilePlay_Close(pTrans->playback.hFilePlay);
    pTrans->playback.hFilePlay = NULL;
#endif
    /* Do the PID channels need to be closed prior to calling NEXUS_VideoInput_Shutdown?
     * That seems to be the case based on experimenting. */
    TRNSX_Playback_Close( pCtxt, pTrans );

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    NEXUS_VideoDecoder_Close(pTrans->videoDecoder.videoDecoder);
    pTrans->videoDecoder.videoDecoder = NULL;

    /* TODO: since the audio decoder will share a STC channel with either the video decoder or encoder,
     * does the following call be in a wrapper? */
    NEXUS_StcChannel_Close(pTrans->videoDecoder.stcChannel);
    pTrans->videoDecoder.stcChannel = NULL;

    /*TRNSX_Playback_Close( pCtxt, pTrans );*/

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoDecoder.eState )));

    if ( strlen( pTrans->source.fname ) == 0 )
    {
        BDBG_ERR(("%s[%d]:: you must specify a source file before starting the decoder.", BSTD_FUNCTION, pTrans->uiIndex ));
        return NEXUS_NOT_INITIALIZED;
    }

    if ( pTrans->videoDecoder.eState != TRNSX_State_eOpened && pTrans->videoDecoder.eState != TRNSX_State_eStopped )
    {
        BDBG_ERR(("%s[%d]:: the decoder is %s. It must be opened or stopped before you can call start.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoDecoder.eState = TRNSX_State_eRunning;

    /* Helping out the user here; perhaps they should be forced to call display open.
     * If the encoder display has not been opened, do so now. */
    if ( !pTrans->videoEncoder.display )
    {
        TRNSX_VideoEncoderDisplay_Open( pCtxt, pTrans );
    }


    /* Needs to be called to open videoDecoder.pidChannel,
     * which can't be opened until playback is open*/
    TRNSX_Playback_Open( pCtxt, pTrans );

    NEXUS_VideoWindow_AddInput(pTrans->videoEncoder.window, NEXUS_VideoDecoder_GetConnector(pTrans->videoDecoder.videoDecoder));

    pTrans->videoDecoder.startSettings.codec = (NEXUS_VideoCodec)pTrans->source.eVideoCodec;
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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoDecoder.eState )));

    if ( pTrans->videoDecoder.eState != TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s[%d]:: the decoder is %s. It must be running before you can call stop.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->videoDecoder.eState)));
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
        BKNI_Printf("Video Decoder[%d] Status:\n", pTrans->uiIndex);
        TRNSX_Print_VideoDecoderStatus( &videoDecoderStatus );
    }
    else
    {
        BKNI_Printf("%s{%d]:: video decoder has not been opened yet\n", BSTD_FUNCTION, pTrans->uiIndex );
    }

    return rc;
}


/*
 * Video Encoder wrappers
 */

static void TRNSX_VideoEncoder_WatchdogHandler (void *context, int param)
{
   TRNSX_Transcode * pTrans = (TRNSX_Transcode*) context;
   BSTD_UNUSED(param);

   /* Simply flag the event for now. */
   pTrans->videoEncoder.bEncoderWatchdogged = true;

   BDBG_ERR(("%s[%d]:: encoder watchdog fired.", BSTD_FUNCTION, pTrans->uiIndex));

   return;
}

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));

    if ( pTrans->videoEncoder.eState != TRNSX_State_eClosed )
    {
        BDBG_ERR(("%s: the encoder is %s. It must be before you can call open.", BSTD_FUNCTION, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
        return NEXUS_NOT_INITIALIZED;
    }

    pTrans->videoEncoder.eState = TRNSX_State_eOpened;


    pTrans->videoEncoder.openSettings.watchdogCallback.callback = TRNSX_VideoEncoder_WatchdogHandler;
    pTrans->videoEncoder.openSettings.watchdogCallback.context = pTrans;
    pTrans->videoEncoder.bEncoderWatchdogged = false;

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     * TODO: is this still the case? */

    pTrans->videoEncoder.videoEncoder = NEXUS_VideoEncoder_Open(pTrans->uiIndex, &pTrans->videoEncoder.openSettings);
    BDBG_ASSERT(pTrans->videoEncoder.videoEncoder);

    /* encoder requires different STC broadcast mode from decoder */
    /*pTrans->videoEncoder.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &pTrans->videoEncoder.stcSettings);*/
    TRNSX_StcChannel_Open( pCtxt,  pTrans );

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));

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

    /* TODO: since the audio decoder will share a STC channel with either the video decoder or encoder,
     * does the following call be in a wrapper? */

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
    /*NEXUS_VideoEncoderDelayRange videoDelay;*/
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));

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
#if 1
    TRNSX_StreamMux_AV_Sync( pCtxt, pTrans );
#else
    NEXUS_VideoEncoder_GetDelayRange(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings, &pTrans->videoEncoder.startSettings, &videoDelay);
    BKNI_Printf("%s: video encoder end-to-end delay = [%u ~ %u] ms\n", BSTD_FUNCTION, videoDelay.min/27000, videoDelay.max/27000);
    pTrans->videoEncoder.settings.encoderDelay = videoDelay.min;
#endif

    /* note the Dee is set by SetSettings, from where? */
    NEXUS_VideoEncoder_SetSettings(pTrans->videoEncoder.videoEncoder, &pTrans->videoEncoder.settings);


    if ( pTrans->videoEncoder.bCustomDisplaySettings )
    {
        pTrans->videoEncoder.startSettings.interlaced = pTrans->videoEncoder.customFormatSettings.interlaced;

        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: using custom display settings: videoEncoder.startSettings.interlaced:%d ",
                                        BSTD_FUNCTION,
                                        pTrans->uiIndex,
                                        pTrans->videoEncoder.customFormatSettings.interlaced ));
    }
    else
    {
        NEXUS_VideoFormatInfo fmtInfo;
        NEXUS_VideoFormat_GetInfo(pTrans->videoEncoder.displaySettings.format, &fmtInfo);
        pTrans->videoEncoder.startSettings.interlaced = fmtInfo.interlaced;

        BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: display format: %s videoEncoder.startSettings.interlaced:%d ",
                            BSTD_FUNCTION,
                            pTrans->uiIndex,
                            lookup_name(g_videoFormatStrs, pTrans->videoEncoder.displaySettings.format ),
                            pTrans->videoEncoder.customFormatSettings.interlaced ));
    }

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->videoEncoder.eState )));

    if ( pTrans->videoEncoder.eState != TRNSX_State_eRunning )
    {
        BDBG_ERR(("%s[%d]:: the encoder is %s. It must be running before you can call stop.", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs ,pTrans->videoEncoder.eState)));
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
        BKNI_Printf("Video Encoder[%d] Status:\n", pTrans->uiIndex);
        TRNSX_Print_VideoEncoderStatus( &videoEncodeStatus );
        NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
        NEXUS_VideoEncoder_ClearStatus(pTrans->videoEncoder.videoEncoder, &clearStatus);
    }
    else
    {
        BKNI_Printf("%s[%d]:: video encoder has not been opened yet\n", BSTD_FUNCTION, pTrans->uiIndex );
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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->eState )));

    BDBG_ASSERT(pCtxt);
    BDBG_ASSERT(pTrans);

    if ( !strlen(pTrans->source.fname) )
    {
        BKNI_Printf("%s: session:%d must specify a source file\n", BSTD_FUNCTION, pTrans->uiIndex );
        return NEXUS_NOT_INITIALIZED;
    }

    if ( !strlen(pTrans->record.fname) )
    {
        BKNI_Printf("%s: session:%d must specify a results file\n", BSTD_FUNCTION, pTrans->uiIndex );
        return NEXUS_NOT_INITIALIZED;
    }

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

    BDBG_ENTER(TRNSX_Transcode_Start);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->eState )));

    rc = TRNSX_Transcode_Open( pCtxt, pTrans );

    if ( rc != NEXUS_SUCCESS ) goto done;

    pTrans->eState = TRNSX_State_eRunning;

    /* Start video decoder. */
    TRNSX_VideoDecoder_Start( pCtxt, pTrans );

    /* Start playback */
    TRNSX_Playback_Start( pCtxt, pTrans );

    BKNI_Sleep(1000);

    TRNSX_Mux_Start( pCtxt, pTrans );
    TRNSX_Record_Start( pCtxt, pTrans );

    TRNSX_VideoEncoder_Start( pCtxt, pTrans );

done:

    BDBG_LEAVE(TRNSX_Transcode_Start);

    return rc;

}

static NEXUS_Error TRNSX_Transcode_Close(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(TRNSX_Transcode_Close);

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->eState )));

    TRNSX_VideoDecoder_Close( pCtxt, pTrans );

    TRNSX_VideoEncoder_Close( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Close);

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

    BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: state on entry:%s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_stateStrs, pTrans->eState )));

    pTrans->eState = TRNSX_State_eStopped;

    /*
     * NOTE: stop sequence should be in front->back order
     */
    TRNSX_VideoEncoder_Stop( pCtxt, pTrans );

    TRNSX_Record_Stop( pCtxt, pTrans );
    TRNSX_Mux_Stop( pCtxt, pTrans );

    TRNSX_Playback_Stop( pCtxt, pTrans );

    TRNSX_VideoDecoder_Stop( pCtxt, pTrans );

    TRNSX_Transcode_Close( pCtxt, pTrans );

    BDBG_LEAVE(TRNSX_Transcode_Stop);

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
    BKNI_Printf("  source type: %s\n", lookup_name(g_sourceStrs, pTrans->source.eType) );
    BKNI_Printf("  source file: %s\n", pTrans->source.fname );
    BKNI_Printf("  stream type: %s\n", lookup_name(g_transportTypeStrs, pTrans->source.eStreamType) );
    BKNI_Printf("  video codec: %s\n", lookup_name(g_videoCodecStrs, pTrans->source.eVideoCodec) );
    BKNI_Printf("  video pid: %d\n", pTrans->source.iVideoPid );
    BKNI_Printf("  PCR pid: %d\n", pTrans->source.iPcrPid );

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

    /* Hitting a CR will dump the settings of the currently active comamnd.
     * Need this special case logic since CR's are filtered
     * in the main parsing loop.*/

    if( *pszInput == '\n' )
    {
        strcpy( pstCmd->szKey, "CR" );
        pstCmd->uiNumValues += 1;
        goto done;
    }

    /* A clunky way to keep track of state. */
#define SEARCH_FOR_KEY      0x1
#define LOAD_KEY            0x2
#define SEARCH_FOR_VALUE    0x3
#define LOAD_VALUE          0x4
#define IS_THERE_A_COMMA    0x5 /* just loaded a value but it could be a comma separated list */
#define START_OF_STRING     0x6
#define LOAD_STRING         0x7

    uiState = SEARCH_FOR_KEY;
    bStripWhite = false;

    BDBG_MODULE_MSG(trnsx_parse,("%s: input: %s", BSTD_FUNCTION, pszInput ));

    for ( pSrc=pszInput, i=0; *pSrc && i < uiNumCmds-1 ; i++ )
    {
        BDBG_MODULE_MSG(trnsx_parse,("%s: uiState:%d  bStripWhite:%d ", BSTD_FUNCTION, uiState, bStripWhite ));

        /*TODO: where to add error checking for syntax error on the source. */

        if ( *pSrc == '"' )
        {
            pSrc++;
            if ( uiState == LOAD_STRING || uiState == START_OF_STRING )
            {
                uiState = LOAD_KEY;
            }
            else
            {
                uiState = START_OF_STRING;
            }
        }
        else if (( *pSrc == ':' ) && ( uiState != LOAD_STRING ))
        {
            pSrc++;
            uiState     = SEARCH_FOR_VALUE;
            bStripWhite = false;
        }
        else if (( *pSrc == '#' || *pSrc == '\n' ) && ( uiState != LOAD_STRING ))
        {
            /* The beginning of a comment or the end of the input. */

            /* Need to check for zero length, i.e the case of "key:\n"*/
            if (( strlen( pstCmd->data[pstCmd->uiNumValues].szValue )  != 0 ) && ( uiState == LOAD_VALUE ))
            {
                pstCmd->data[pstCmd->uiNumValues].uiValue = strtoul(pstCmd->data[pstCmd->uiNumValues].szValue, NULL, 0);
                pstCmd->uiNumValues += 1;
            }
            break;
        }
        else if (( *pSrc == ' ' || *pSrc == '\t' || *pSrc == '\r' ) && ( uiState != LOAD_STRING ))
        {
            pSrc++;

            if ( uiState == LOAD_VALUE )
            {
                /* If white space while loading a value, the end of the value may have reached;
                 * it could be a comma separated list.  If the next non-white space character is a
                 * comma, then there is another value.  If the next non-white space character is NOT a
                 * comman, then it is the start of the next key. */

                pstCmd->data[pstCmd->uiNumValues].uiValue = strtoul(pstCmd->data[pstCmd->uiNumValues].szValue, NULL, 0);
                pstCmd->uiNumValues++;
                /*pstCmd++;*/
                pDst        = &pstCmd->data[pstCmd->uiNumValues].szValue[0];

                uiState     = IS_THERE_A_COMMA;
                bStripWhite = false;
            }
            else
            {
                /* If white space while searching for a key or value or loading a key, flag
                 * that white space is being stripped but don't change state. */

                bStripWhite = true;
            }
        }
        else if (( *pSrc == ',' ) && ( uiState != LOAD_STRING ))
        {
            pSrc++;
            if ( uiState == LOAD_VALUE )
            {
                /* A comma loading a value, the end of the value has been reached.
                 * Another value is expected. */

                pstCmd->data[pstCmd->uiNumValues].uiValue = strtoul(pstCmd->data[pstCmd->uiNumValues].szValue, NULL, 0);
                pstCmd->uiNumValues++;
                pDst        = &pstCmd->data[pstCmd->uiNumValues].szValue[0];


            }
            else if ( uiState != IS_THERE_A_COMMA )
            {
                /* IS_THERE_A_COMMA would be true if there was white space between value and the comma*/
                BDBG_ERR(("%s:: a comma in the wrong place, parse state is %d", BSTD_FUNCTION, uiState ));
                BDBG_ERR(("%s:: -%s- -%s- -%d-", BSTD_FUNCTION, pstCmd->szKey, pstCmd->data[pstCmd->uiNumValues].szValue, pstCmd->uiNumValues));
            }
            uiState     = SEARCH_FOR_VALUE;
            bStripWhite = false;
        }
        else
        {
            bool bFileName = false;

            if ( uiState == IS_THERE_A_COMMA )
            {
                /* Since no comma was received, this must be the start of the next key.
                 * As opposed to the next value in a comma separated list. */
                pstCmd++;
                pDst        = &pstCmd->szKey[0];
                uiState     = LOAD_KEY;
            }
            else if ( uiState == SEARCH_FOR_KEY )
            {
                pDst        = &pstCmd->szKey[0];
                uiState     = LOAD_KEY;
            }
            else if ( uiState == SEARCH_FOR_VALUE )
            {
                pDst        = &pstCmd->data[pstCmd->uiNumValues].szValue[0];
                uiState     = LOAD_VALUE;
            }
            else if ( uiState == START_OF_STRING )
            {
                pDst        = &pstCmd->data[pstCmd->uiNumValues].szValue[0];
                uiState     = LOAD_STRING;
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
                /* Don't change the case of file names or strings */
                bFileName = !strcmp(pstCmd->szKey, "file");
                bFileName |= !strcmp(pstCmd->szKey, "ifile");
                bFileName |= !strcmp(pstCmd->szKey, "ofile");
                bFileName |= ( uiState == LOAD_STRING );
            }

            *pDst++ = ( bFileName ) ? *pSrc++ : tolower(*pSrc++);

        }



    }

done:

#if 0
    for ( pstCmd = pstCmdSaved, i=0; ( strlen(pstCmd->szKey) !=0 ) && i < uiNumCmds-1 ; pstCmd++, i++ )
    {
        BKNI_Printf("%s::%s (%d) %s %d\n", BSTD_FUNCTION, pstCmd->szKey, pstCmd->uiNumValues, pstCmd->data[0].szValue, pstCmd->data[0].uiValue );
    }
#endif
#if 0
    /* check if the last command had the proper syntax.
     * Ignore issues with the quit command. */
    if ( ( strlen(pstCmd->szKey) !=0 && pstCmd->uiNumValues == 0 )
          && (strcmp(pstCmd->szKey, "quit") && strcmp(pstCmd->szKey, "q")) )
    {
        /*BKNI_Printf("error::%s (%d) %s %d\n", pstCmd->szKey, pstCmd->uiNumValues, pstCmd->data[0].szValue, pstCmd->data[0].uiValue );*/
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

    BDBG_MODULE_MSG(trnsx_input,("%s[%d]:: executing %s", BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_cmdFullnameStrs, pTrans->eActiveCmd )));

    switch( pTrans->eActiveCmd )
    {
        /*** video encoder ***/
        case TRNSX_Cmd_eVideoEncoderSetSettings:    TRNSX_VideoEncoder_SetSettings( pCtxt, pTrans );   break;
        case TRNSX_Cmd_eVideoEncoderOpen:           TRNSX_VideoEncoder_Open( pCtxt, pTrans );          break;
        case TRNSX_Cmd_eVideoEncoderClose:          TRNSX_VideoEncoder_Close( pCtxt, pTrans );         break;
        case TRNSX_Cmd_eVideoEncoderStart:          TRNSX_VideoEncoder_Start( pCtxt, pTrans );         break;
        case TRNSX_Cmd_eVideoEncoderStop:           TRNSX_VideoEncoder_Stop( pCtxt, pTrans );          break;
        case TRNSX_Cmd_eVideoEncoderStatus:         TRNSX_VideoEncoder_GetStatus( pCtxt, pTrans );     break;

        /*** audio encoder ***/
        case TRNSX_Cmd_eAudioEncoderOpen:
        case TRNSX_Cmd_eAudioEncoderClose:
            break;

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
            /* Called in TRNSX_VideoEncoder_Open when the displayed has been opened.
             * If the display is already opened, call immediately */

            if( pTrans->videoEncoder.bCustomDisplaySettings && pTrans->videoEncoder.display )
            {
                NEXUS_Error rc = NEXUS_SUCCESS;
                BDBG_MODULE_MSG(trnsx_dbg, ("%s[%d]:: calling NEXUS_Display_SetCustomFormatSettings", BSTD_FUNCTION, pTrans->uiIndex));
                rc = NEXUS_Display_SetCustomFormatSettings(pTrans->videoEncoder.display, NEXUS_VideoFormat_eCustom2, &pTrans->videoEncoder.customFormatSettings);
                if ( rc )
                {
                    BDBG_ERR(("%s[%d]:: NEXUS_Display_SetCustomFormatSettings returned: %d", BSTD_FUNCTION, pTrans->uiIndex, rc));
                }
            }
            break;

        /*** playback ***/
        case TRNSX_Cmd_ePlaybackSetSettings:    break;
        case TRNSX_Cmd_ePlaybackOpen:       break;
        case TRNSX_Cmd_ePlaybackClose:      break;
        case TRNSX_Cmd_ePlaybackStart:      TRNSX_Playback_Start( pCtxt, pTrans );   break;
        case TRNSX_Cmd_ePlaybackStop:       TRNSX_Playback_Stop( pCtxt, pTrans );    break;

        /*** video decoder ***/
        case TRNSX_Cmd_eVideoDecoderSetSettings:    break;

        case TRNSX_Cmd_eVideoDecoderOpen:   TRNSX_VideoDecoder_Open( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eVideoDecoderClose:  TRNSX_VideoDecoder_Close( pCtxt, pTrans );     break;
        case TRNSX_Cmd_eVideoDecoderStart:  TRNSX_VideoDecoder_Start( pCtxt, pTrans );     break;
        case TRNSX_Cmd_eVideoDecoderStop:   TRNSX_VideoDecoder_Stop( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eVideoDecoderStatus: TRNSX_VideoDecoder_GetStatus( pCtxt, pTrans ); break;

        /*** audio decoder ***/
        case TRNSX_Cmd_eAudioDecoderOpen:           TRNSX_AudioDecoder_Open( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eAudioDecoderClose:          TRNSX_AudioDecoder_Close( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eAudioDecoderStart:          TRNSX_AudioDecoder_Start( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eAudioDecoderStop:           TRNSX_AudioDecoder_Stop( pCtxt, pTrans );      break;
        case TRNSX_Cmd_eAudioDecoderStatus:         TRNSX_AudioDecoder_GetStatus( pCtxt, pTrans ); break;
        case TRNSX_Cmd_eAudioDecoderSetSettings:    break;

        case TRNSX_Cmd_eSession:    break;
        case TRNSX_Cmd_eSelect:     break;
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
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: begin sleep", BSTD_FUNCTION, pTrans->uiIndex));
            BKNI_Sleep( (pTrans->sleep.eUnit == TRNSX_SleepUnit_eSecs) ? pTrans->sleep.uiDuration * 1000 : pTrans->sleep.uiDuration );
            BDBG_MODULE_MSG(trnsx_dbg,("%s[%d]:: end sleep", BSTD_FUNCTION, pTrans->uiIndex));
            break;

        case TRNSX_Cmd_eNone:
            /* treat any dangling "end:0" as a NOP */
            break;

        default:
             BKNI_Printf("%s[%d]:: hit default, support has not been added for command %d\n", BSTD_FUNCTION, pTrans->uiIndex, pTrans->eActiveCmd );
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
    bool bPrint;
    bool bDumpParams;
    unsigned i;

    bPrint = !strcmp(pstCmd->szKey, "CR");
    bPrint &= pCtxt->bInteractive;

    bDumpParams = !strcmp(pstCmd->szKey, "help");
    bPrint |= bDumpParams;

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
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
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
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoEncoderStatus:
        case TRNSX_Cmd_eVideoEncoderClose:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
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
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoEncoderStop:
            if ( bPrint ) {
                TRNSX_Print_VideoEncoderStopSettings( &pTrans->videoEncoder.stopSettings );
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "mode", pTrans->videoEncoder.stopSettings.mode, g_encoderStopModeStrs)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        /*** audio encoder ***/
        case TRNSX_Cmd_eAudioEncoderOpen:
            if ( bPrint ) {
                TRNSX_Print_AudioEncoderOpenSettings( &pTrans->audioEncoder.encoderSettings );
            }
            else if (!strcmp(pstCmd->szKey, "codec"))
            {
                if ( pstCmd->uiNumValues )
                {
                    NEXUS_AudioCodec eAudioCodec = lookup(g_audioCodecStrs,pstCmd->data[0].szValue);

                    switch( eAudioCodec )
                    {
                        case NEXUS_AudioCodec_eMp3:
                        case NEXUS_AudioCodec_eAacPlus:
                        case NEXUS_AudioCodec_eLpcm1394:
                        case NEXUS_AudioCodec_eAac:
                            pTrans->audioEncoder.encoderSettings.codec = eAudioCodec;
                            break;

                        default:
                            BDBG_ERR(("%s[%d]:: udio codec %s is not supported defaulting to NEXUS_AudioCodec_eAac",
                                            BSTD_FUNCTION, pTrans->uiIndex, lookup_name(g_audioCodecStrs,eAudioCodec)));
                            pTrans->audioEncoder.encoderSettings.codec = NEXUS_AudioCodec_eAac;
                            break;
                    }

                }
                else
                {
                    BKNI_Printf("  current value for %s: %s\n", pstCmd->szKey, lookup_name(g_audioCodecStrs,pTrans->audioEncoder.encoderSettings.codec ));
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eAudioEncoderClose:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        /*** display ***/
        case TRNSX_Cmd_eDisplayClose:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

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
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
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
                if ( pstCmd->uiNumValues )
                {
                    pTrans->videoEncoder.customFormatSettings.refreshRate = pstCmd->data[0].uiValue;
                    if ( pstCmd->data[0].uiValue < 1 * 1000 || pstCmd->data[0].uiValue > 240 * 1000 )
                    {
                        BDBG_WRN(("%s[%d]:: is this value correct? refreshRate is specified in units of in 1/1000th Hz", BSTD_FUNCTION, pTrans->uiIndex));
                        BDBG_WRN(("%s[%d]:: was expecting something like 60*1000.", BSTD_FUNCTION, pTrans->uiIndex));
                    }
                }
                else
                    BKNI_Printf("  current value for %s: %d\n", pstCmd->szKey, pTrans->videoEncoder.customFormatSettings.refreshRate );
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoDecoderStart:
            /* Set nonRealTime based on the session value. */
            /* TODO: delete the following assignment? */
            pTrans->videoDecoder.startSettings.nonRealTime = pTrans->bNonRealTime;
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_ePlaypumpSetSettings:
            if ( bPrint ) {
                BKNI_Printf("  transport:  %s\n", lookup_name( g_transportTypeStrs, pTrans->streamMux.playpumpSettings.transportType ));
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "transport", pTrans->streamMux.playpumpSettings.transportType, g_transportTypeStrs)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

            break;
        case TRNSX_Cmd_ePlaypumpOpen:
            if ( bPrint ) {
                TRNSX_Print_PlaypumpOpenSettings( &pTrans->streamMux.playpumpOpenSettings );
            }
            TRNSX_ELSE_IF( pstCmd, "fifoSize", pTrans->streamMux.playpumpOpenSettings.fifoSize)
            TRNSX_ELSE_IF( pstCmd, "numdescriptors", pTrans->streamMux.playpumpOpenSettings.numDescriptors )
            TRNSX_ELSE_IF( pstCmd, "streammuxcompatible", pTrans->streamMux.playpumpOpenSettings.streamMuxCompatible)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eVideoDecoderSetSettings:
        case TRNSX_Cmd_eVideoDecoderOpen:
        case TRNSX_Cmd_eVideoDecoderClose:
        case TRNSX_Cmd_eVideoDecoderStop:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_eVideoDecoderStatus:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_eAudioDecoderOpen:
        case TRNSX_Cmd_eAudioDecoderClose:
        case TRNSX_Cmd_eAudioDecoderStart:
        case TRNSX_Cmd_eAudioDecoderStop:
        case TRNSX_Cmd_eAudioDecoderSetSettings:
        case TRNSX_Cmd_eAudioDecoderStatus:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_eSelect:
            if ( bPrint ) {
                TRNSX_Print_SelectSettings( pCtxt, pTrans );
            }
            else if (!strcmp(pstCmd->szKey, "session")) {
                if ( pstCmd->uiNumValues == 0 )
                {
                    BKNI_Printf("session %d is selected\n", pCtxt->uiSelected );
                }
                else
                {
                    if ( (unsigned)pstCmd->data[0].uiValue < pCtxt->uiMaxSessions )
                    {
                        pCtxt->uiSelected = pstCmd->data[0].uiValue;
                        /* TODO: need to sort out the session selection and when the pTrans pointer is set in the main loop. */
                        /* Need for interactive mode when selecting a session for the first time.
                         * Without this, eActiveCmd for the newly selected session is TRNSX_Cmd_eNone. */
                        /*pCtxt->transcodes[pCtxt->uiSelected].eActiveCmd = TRNSX_Cmd_eSession;*/
                    }
                    else
                    {
                        BKNI_Printf("Session selection value %d is out of range, it should be less than %d\n", pstCmd->data[0].uiValue, pCtxt->uiMaxSessions );
                    }
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;


        case TRNSX_Cmd_eSession:
            if ( bPrint ) {
                TRNSX_Print_SessionSettings( pCtxt, pTrans );
            }
            TRNSX_ELSE_IF( pstCmd, "nrt", pTrans->bNonRealTime)
            TRNSX_ELSE_IF( pstCmd, "cc", pTrans->bEncodeCCData)
            TRNSX_ELSE_IF( pstCmd, "audio", pTrans->bEncodeAudio)
            TRNSX_ELSE_IF( pstCmd, "lipsync", pTrans->syncChannel.bEnable)
            TRNSX_ELSE_IF_LOOKUP(pstCmd, "tts_out", pTrans->streamMux.transportTimestampType, g_transportTimestampTypeStrs )
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eTranscode:
            /* TODO: these should be called in TRNSX_Commmand_Execute. */
            if ( pstCmd->uiNumValues && !strcmp(pstCmd->szKey, "action"))
            {
                if (!strcmp(pstCmd->data[0].szValue, "start")) {
                    TRNSX_Transcode_Start( pCtxt, pTrans );
                }
                else if (!strcmp(pstCmd->data[0].szValue, "stop")) {
                    TRNSX_Transcode_Stop( pCtxt, pTrans );
                }
                else {
                    BKNI_Printf("unknown transcode operation: %s\n", pstCmd->data[0].szValue );
                    bRet = false;
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eMuxStart:
            switch( pTrans->record.eTransportType )
            {
                case NEXUS_TransportType_eEs:
                    if ( bPrint ) {
                        TRNSX_Print_StreamMuxStartSettings( pTrans, bDumpParams );
                    }
                    break;

                /* File Mux */
                case NEXUS_TransportType_eMpeg2Pes:
                case NEXUS_TransportType_eMp4:
                    if ( bPrint ) {
                        TRNSX_Print_StreamMuxStartSettings( pTrans, bDumpParams );
                    }
                    break;

                /* Stream Mux */
                case NEXUS_TransportType_eTs:
                case NEXUS_TransportType_eMp4Fragment:
                    if ( bPrint ) {
                        TRNSX_Print_StreamMuxStartSettings( pTrans, bDumpParams );
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
                    TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
                    break;

                default:
                    BDBG_ERR(("%s: unsupported transport type: %s", BSTD_FUNCTION, lookup_name( g_transportTypeStrs, pTrans->record.eTransportType) ));
                    break;

            }
            break;

        case TRNSX_Cmd_eMuxStop:
        case TRNSX_Cmd_eMuxClose:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_eMuxOpen:
            if ( bPrint ) {
                TRNSX_Print_MuxOpenSettings( pTrans, bDumpParams );
            }
            else if (!strcmp(pstCmd->szKey, "transport"))
            {
                if ( pstCmd->uiNumValues )
                {
                    bool bFileMuxError = false;
                    bool bStreamMuxError = false;
                    TRNSX_TransportType eTransportType = lookup(g_trnsxTransportTypeStrs,pstCmd->data[0].szValue);

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
            else if ( pTrans->record.eTransportType == NEXUS_TransportType_eTs )
            {
                if ( bPrint ) {
                    TRNSX_Print_MuxOpenSettings( pTrans, bDumpParams );
                }
                TRNSX_ELSE_IF( pstCmd, "userdata.enable", pTrans->streamMux.userData.enable)
                TRNSX_ELSE_IF( pstCmd, "userdata.remappids", pTrans->streamMux.userData.bRemapPids)
                TRNSX_ELSE_IF( pstCmd, "userdata.base_pid", pTrans->streamMux.userData.uiBasePid)
                TRNSX_ELSE_IF( pstCmd, "pcr_pid", pTrans->streamMux.uiPcrPid)
                TRNSX_ELSE_IF( pstCmd, "video_pid", pTrans->streamMux.uiVideoPid)
                TRNSX_ELSE_IF( pstCmd, "audio_pid", pTrans->streamMux.uiAudioPid)
                /*TRNSX_ELSE_IF( pstCmd, "userdata.pid", pTrans->streamMux.userData.uiPid)*/
                TRNSX_ELSE_IF( pstCmd, "pmt_pid", pTrans->streamMux.uiPmtPid)
                TRNSX_ELSE_IF( pstCmd, "pat_pid", pTrans->streamMux.uiPatPid)
                TRNSX_ELSE_IF( pstCmd, "pespacking", pTrans->streamMux.bAudioPesPacking)
                TRNSX_ELSE_IF( pstCmd, "dspmixer", pTrans->streamMux.bDspMixer)
                TRNSX_ELSE_IF( pstCmd, "force48kps", pTrans->streamMux.bForce48KbpsAACplus)
                TRNSX_ELSE_IF( pstCmd, "32khzaudio", pTrans->streamMux.b32KHzAudio)
                TRNSX_ELSE_IF_LOOKUP( pstCmd, "multichanfmt", pTrans->streamMux.eMultiChanFmt,g_audioChannelFormatStrs) /* NEXUS_AudioMultichannelFormat*/
                TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eRecordStart:
        case TRNSX_Cmd_eRecordPause:
        case TRNSX_Cmd_eRecordResume:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_eRecordStop:
            if ( bPrint ) {
                BKNI_Printf("  bWaitForEos: %d  // wait for EOS before exiting the copy thread\n", pTrans->record.bWaitForEos);
            }
            TRNSX_ELSE_IF( pstCmd, "bwaitforeos", pTrans->record.bWaitForEos)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eRecordSettings:
            if ( bPrint ) {
                TRNSX_Print_RecordSettings( pTrans );
            }
            else if (!strcmp(pstCmd->szKey, "file"))
            {
                if ( pstCmd->uiNumValues == 0 )
                {
                    /*BKNI_Printf("%s: failed to specify a file\n", pstCmd->szKey );*/
                    BKNI_Memset( pTrans->record.fname, 0, sizeof(pTrans->record.fname) );
                    BKNI_Memset( pTrans->record.indexfname, 0, sizeof(pTrans->record.fname) );
                }
                else
                {
                    char * pToken;

                    /* TODO: close old file if opening a new one? */

                    strncpy(pTrans->record.fname, pstCmd->data[0].szValue, TRNSX_SIZE_FILE_NAME-1);

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
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_ePlaybackSetSettings:
        case TRNSX_Cmd_ePlaybackStart:
        case TRNSX_Cmd_ePlaybackStop:
        case TRNSX_Cmd_ePlaybackClose:
            TRNSX_DEFAULT_PARAM_MSG( bPrint )
            break;

        case TRNSX_Cmd_ePlaybackOpen:
            if ( bPrint ) {
                TRNSX_Print_PlaybackOpenSettings( pTrans );
            }
            else if ( (pstCmd->uiNumValues != 0 ) && !strcmp(pstCmd->szKey, "type"))
            {
                if (!strcmp(pstCmd->data[0].szValue, "file"))
                {
                    pTrans->source.eType = TRNSX_Source_eFile;
                }
                else
                {
                    BKNI_Printf("%s: currently only support the default of file:\n", BSTD_FUNCTION );
                    pTrans->source.eType = TRNSX_Source_eFile;
                    /* TODO: need to add error handling, when reading from a command file, exit */
                }
            }
            else if (!strcmp(pstCmd->szKey, "file"))
            {
                if ( pstCmd->uiNumValues == 0 )
                {
                    BKNI_Printf("%s: failed to specify a file\n", pstCmd->szKey );
                }
                else
                {
                    TRNSX_MediaProbe(pTrans, pstCmd->data[0].szValue);

                    if ( pTrans->source.bFileIsValid == false )
                    {
                        BKNI_Printf("%s: failed to open %s\n", pstCmd->szKey, pstCmd->data[0].szValue );
                    }

                }
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "video_codec",pTrans->source.eVideoCodec, g_videoCodecStrs )
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "transport",pTrans->source.eStreamType, g_transportTypeStrs )
            TRNSX_ELSE_IF( pstCmd, "video_pid", pTrans->source.iVideoPid)
            TRNSX_ELSE_IF( pstCmd, "pcr_pid", pTrans->source.iPcrPid)
            TRNSX_ELSE_IF( pstCmd, "pmt_pid", pTrans->source.uiPmtPid)
            else if (!strcmp(pstCmd->szKey, "userdata.pid" ))
            {
                if ( pstCmd->uiNumValues )
                {
                    BKNI_Memset( pTrans->source.userData.pids, 0, sizeof ( pTrans->source.userData.pids ) );

                    pTrans->source.userData.numPids = pstCmd->uiNumValues;

                    for ( i=0; i < pstCmd->uiNumValues && i < NEXUS_MAX_MUX_PIDS; i++ )
                    {
                        pTrans->source.userData.pids[i] = pstCmd->data[i].uiValue;
                    }
                }
                else
                {
                    BKNI_Printf("  current value for %s:\n", pstCmd->szKey);
                    BKNI_Printf("   userdata.numPids:   %d\n", pTrans->source.userData.numPids );
                    i=0;
                    do
                    {
                        BKNI_Printf("   userdata.pid[%d]:  %x (%d)\n", i, pTrans->source.userData.pids[i], pTrans->source.userData.pids[i] );
                        i++;
                    } while ( i < pTrans->source.userData.numPids );
                }
            }
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        case TRNSX_Cmd_eSleep:
            if ( bPrint ) {
                BKNI_Printf("  unit:     %s  // <secs,msecs> : units of duration, default is secs\n", lookup_name(g_sleepUnitStrs, pTrans->sleep.eUnit));
                BKNI_Printf("  duration: %d\n", pTrans->sleep.uiDuration);
            }
            TRNSX_ELSE_IF_LOOKUP( pstCmd, "unit",pTrans->sleep.eUnit, g_sleepUnitStrs )
            TRNSX_ELSE_IF( pstCmd, "duration", pTrans->sleep.uiDuration)
            TRNSX_ERROR( pTrans, pstCmd, g_cmdFullnameStrs, bRet )
            break;

        default:
            BKNI_Printf("%s: hit default for pTrans->eActiveCmd.  Command %d is undefined.\n", BSTD_FUNCTION, pTrans->eActiveCmd );
            bRet = false;
            break;
    }


    return bRet;
}

static void TRNSX_Help( bool bDumpParams )
{
    unsigned i;
    TRNSX_TestContext   stContext;
    TRNSX_Transcode     stTranscode;
    TRSNX_Command       stCmd;

    /* Can operate in two modes, calling each Nexus command or use defaults, set parameters of interest,
     * then use wrapper routines to make Nexus calls. */

    if ( !bDumpParams )
    {
        BKNI_Printf(
            "--- general ---\n"
            "h, help, ?       - prints this help\n"
            "p, params        - prints the parameters that can be set for each module level command\n"
            "file:<file_name> - read comands from a file\n"
            "-- the following are for interactive mode only\n"
            "info             - print the platform capabilities\n"
            "status           - print the status of the currently selected session\n"
            "enum:<n>         - print Nexus and other enum's\n"
            "q or quit        - exit the app\n"
            );

        BKNI_Printf("\n--- module specific commands ---\n");
    }

    BKNI_Memset( stCmd.szKey, 0, sizeof( stCmd.szKey ));
    BKNI_Memcpy( stCmd.szKey, "help", sizeof("help") );
    TRNSX_Transcode_GetDefaultSettings( &stContext, &stTranscode );

    for ( i=1; i < TRNSX_Cmd_eMax; i++ )
    {

        if ( i == TRNSX_Cmd_eTranscode ) continue; /* hide these for now. */

        switch ( i )
        {
            case TRNSX_Cmd_eVideoEncoderOpen:   BKNI_Printf("\n--- video encoder ----\n");  break;
            case TRNSX_Cmd_eAudioEncoderOpen:   BKNI_Printf("\n--- audio encoder ----\n");  break;
            case TRNSX_Cmd_eDisplayOpen:        BKNI_Printf("\n--- display ----\n");  break;
            case TRNSX_Cmd_ePlaybackOpen:       BKNI_Printf("\n--- playback ----\n");  break;
            case TRNSX_Cmd_eVideoDecoderOpen:   BKNI_Printf("\n--- video decoder ----\n");  break;
            case TRNSX_Cmd_eAudioDecoderOpen:   BKNI_Printf("\n--- audio decoder ----\n");  break;
            case TRNSX_Cmd_ePlaypumpOpen:       BKNI_Printf("\n--- playpump ----\n");  break;
            case TRNSX_Cmd_eRecordStart:        BKNI_Printf("\n--- record ----\n");  break;
            case TRNSX_Cmd_eMuxOpen:            BKNI_Printf("\n--- mux ----\n");  break;
            case TRNSX_Cmd_eSession:            BKNI_Printf("\n--- miscellaneous ----\n");  break;
            default: break;
        }

        BKNI_Printf("%s",lookup_name(g_cmdFullnameStrs, i ) );

        if ( strcmp(lookup_name(g_cmdShortnameStrs, i ), "") )
        {
            BKNI_Printf(" (%s)", lookup_name(g_cmdShortnameStrs, i ) );

        }

        BKNI_Printf(":\n");

        if ( bDumpParams )
        {
            stTranscode.eActiveCmd = i;
            TRNSX_Command_ParseParams( &stContext, &stTranscode, &stCmd );
            BKNI_Printf("\n");
        }

    }


    if ( !bDumpParams )
    {
        BKNI_Printf("\n--- usage ---\n");
        BKNI_Printf("TODO: need more info here.\n");
        BKNI_Printf("In interactive mode, a <CR> will print the current parameters of active command.\n");
    }

    return;

}

static unsigned TRNSX_Is_Input_A_Valid_Command(
    TRNSX_TestContext *  pCtxt,
    TRNSX_Transcode * pTrans,
    TRSNX_Command * pstCmd
    )
{
    unsigned uiReturn=0;

    BSTD_UNUSED(pCtxt);

    uiReturn = TRNSX_Lookup(g_cmdFullnameStrs, pstCmd->szKey );
    uiReturn |= TRNSX_Lookup(g_cmdShortnameStrs, pstCmd->szKey );

    if ( uiReturn )
    {
        pTrans->eActiveCmd = uiReturn;
    }

    return uiReturn;
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
            TRNSX_Help(false);
            return 0;
        }
        else if (!strcmp(argv[j], "-p") || !strcmp(argv[j], "-params") ) {
            TRNSX_Help(true);
            return 0;
        }
#if 0
        else if (!strcmp(argv[j], "-simdisp")) {
            /* enable simultaneous display
             * TODO: now to deal with multiple transcodes? */

            stTestContext.simulDisplay.bEnabled = true;
        }
#endif
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

    for( i=0; i < TRNSX_MAX_SESSIONS; i++ )
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
                            lookup_name(g_cmdFullnameStrs, pTrans->eActiveCmd));
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

        BDBG_MODULE_MSG( trnsx_echo, ("%s: input string: %s", BSTD_FUNCTION, szInput));

        for ( j=0; j < TRNSX_NUM_CMDS-1; j++ )
        {

            pstCmd = &astCmd[j];

            /* Past the last command in the list. */
            if ( strlen(pstCmd->szKey ) == 0 ) break;

            /* For file debug. */
            if ( !stTestContext.bInteractive )
                BDBG_MODULE_MSG(trnsx_input,("%s: uiValue:%d szValue:%s" , pstCmd->szKey, pstCmd->data[0].uiValue, pstCmd->data[0].szValue ));

            if (!strcmp(pstCmd->szKey, "quit") || !strcmp(pstCmd->szKey, "q")) {
                TRNSX_Platform_Close( &stTestContext );
                exit(0);
            }
            else if (!strcmp(pstCmd->szKey, "h") || !strcmp(pstCmd->szKey, "help") || !strcmp(pstCmd->szKey, "?")) {
                TRNSX_Help(false);
            }
            else if (!strcmp(pstCmd->szKey, "p") || !strcmp(pstCmd->szKey, "params") ) {
                TRNSX_Help(true);
            }
            else if (!strcmp(pstCmd->szKey, "info")) {
                TRNSX_Platform_SystemInfo( &stTestContext );
            }
            else if (!strcmp(pstCmd->szKey, "status")) {
                TRNSX_Transcode_Status( &stTestContext, pTrans ); /* status for this session not all sessions. */
            }
            else if (!strcmp(pstCmd->szKey, "echo")) {
                BKNI_Printf("%s\n", pstCmd->data[0].szValue);
            }
            else if (!strcmp(pstCmd->szKey, "file") && pTrans->eActiveCmd == TRNSX_Cmd_eNone ) /* TODO: cleanup eActiveCmd*/
            {
                if ( pstCmd->uiNumValues == 0 )
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
                    strncpy(szCmdFileName, pstCmd->data[0].szValue, strlen(pstCmd->data[0].szValue));
                    hCmdFile = fopen(szCmdFileName,"r");

                    if ( !hCmdFile )  BKNI_Printf("failed to open command file: %s\n", szCmdFileName);
                }

            }

            /*** Dump enums. ***/
            else if (!strcmp(pstCmd->szKey, "enum")) {
                if (!strcmp(pstCmd->data[0].szValue, "codec"))              { print_value_list(g_videoCodecStrs);           }
                else if (!strcmp(pstCmd->data[0].szValue, "transport"))     { print_value_list(g_transportTypeStrs);        }
                else if (!strcmp(pstCmd->data[0].szValue, "profile"))       { print_value_list(g_videoCodecProfileStrs);    }
                else if (!strcmp(pstCmd->data[0].szValue, "level"))         { print_value_list(g_videoCodecLevelStrs);      }
                else if (!strcmp(pstCmd->data[0].szValue, "framerate"))     { print_value_list(g_videoFrameRateStrs);       }
                else if (!strcmp(pstCmd->data[0].szValue, "format"))        { print_value_list(g_videoFormatStrs);          }
                else if (!strcmp(pstCmd->data[0].szValue, "aspect"))        { print_value_list(g_displayAspectRatioStrs);   }
                else if (!strcmp(pstCmd->data[0].szValue, "encoder_type"))  { print_value_list(g_encoderTypeStrs);        }
                else if (!strcmp(pstCmd->data[0].szValue, "tristate"))      { print_value_list(g_tristateEnableStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "entropy"))       { print_value_list(g_entropyCodingStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "stop"))          { print_value_list(g_encoderStopModeStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "display_type"))  { print_value_list(g_displayTypeStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "timestamp"))     { print_value_list(g_transportTimestampTypeStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "transxtype"))    { print_value_list(g_trnsxTransportTypeStrs);     }
                else if (!strcmp(pstCmd->data[0].szValue, "timing_generator")) { print_value_list(g_displayTimingGeneratorStrs);     }
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

            /*** Did the user specify a valid command? ***/
            else if (  TRNSX_Is_Input_A_Valid_Command( &stTestContext, pTrans, pstCmd ) )
            {
                BDBG_MODULE_MSG( trnsx_echo, ("%s: valid command: %s", BSTD_FUNCTION, lookup_name(g_cmdFullnameStrs, pTrans->eActiveCmd )));
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
