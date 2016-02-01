/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
/* xcode lib example app */
#ifndef XCODE_CONFIG_H__
#define XCODE_CONFIG_H__

#include "bxcode.h"
#include "b_os_lib.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "namevalue.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#include "nexus_graphics2d.h"

#define BTST_MUX_PCR_PID        (0x11)
#define BTST_MUX_VIDEO_PID      (0x12)
#define BTST_MUX_AUDIO_PID      (0x13)
#define BTST_MUX_USER_DATA_PID  (BTST_MUX_AUDIO_PID+6)
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)
#define BTST_NUM_SURFACES        3
#define BTST_FILE_NAME_LEN       80

typedef struct xcode_outputSettings {
    BXCode_OutputType       type;
    char                    data[BTST_FILE_NAME_LEN], index[BTST_FILE_NAME_LEN];
    char                    audioFiles[BXCODE_MAX_AUDIO_PIDS][BTST_FILE_NAME_LEN];
    bool                    fifo; /* output fifo file */

    /* transport output config */
    NEXUS_TransportTimestampType transportTimestamp;
    unsigned                nrtRate;
    bool                    segmented;
    bool                    file;
    /* MP4 output config */
    bool                    progressiveDownload;

    /* video output config */
    bool                    customizeDelay;
    bool                    lowDelay;
    bool                    ccUserdata;
    NEXUS_VideoFormat       format;
    NEXUS_DisplayCustomFormatSettings videoFormat;
    NEXUS_VideoFrameRate    framerate;
    unsigned                vBitrate;
    unsigned                targetBitrate;
    unsigned                rateBufferDelay;
    unsigned                gopFramesP;
    unsigned                gopFramesB;
    unsigned                gopDuration;
    bool                    openGop;
    bool                    newGopOnSceneChange;
    bool                    enableFieldPairing;
    bool                    variableFramerate;
    NEXUS_VideoCodec        vCodec;
    NEXUS_VideoCodecProfile profile;
    NEXUS_VideoCodecLevel   level;
    NEXUS_VideoOrientation   orientation;
    NEXUS_VideoWindowContentMode contentMode;
    /* Note:
       1. higher minimum input and output framerate means lower encode delay
       2. lower max resolution for lower encode delay
       3. higher bitrate bounds means lower encoder delay
       4. zero framesB bound means lower encode delay */
    NEXUS_VideoEncoderBounds bounds;
    /* video encoder memory config */
    struct {
        bool               progressiveOnly;
        unsigned           maxWidth, maxHeight;
    } videoEncoderMemConfig;

    /* audio output config */
    bool                    audioEncode[BXCODE_MAX_AUDIO_PIDS];
    NEXUS_AudioCodec        audioCodec[BXCODE_MAX_AUDIO_PIDS];

    /* even handles for output */
    BKNI_EventHandle        dataReadyEvent;
    B_ThreadHandle          recordThread;
}  xcode_outputSettings;

typedef struct xcode_inputSettings{
    BXCode_InputType       type;
    char                   data[BTST_FILE_NAME_LEN], index[BTST_FILE_NAME_LEN];
    NEXUS_TransportType    transportType;
    NEXUS_TransportTimestampType transportTimestamp;
    unsigned               pcrPid;
    unsigned               pmtPid;
    bool                   probe;
    bool                   useStreamAsIndex;
    unsigned               program;

    /* video input config */
    bool                   enableVideo;
    unsigned               videoPid;
    NEXUS_VideoCodec       vCodec;
    NEXUS_VideoOrientation orientation;
    unsigned               maxWidth, maxHeight;

    /* audio input config */
    bool                   enableAudio;
    unsigned               numAudios;
    unsigned               audioPid[BXCODE_MAX_AUDIO_PIDS];
    NEXUS_AudioCodec       aCodec[BXCODE_MAX_AUDIO_PIDS];
    unsigned               secondaryAudioPid;
    NEXUS_AudioMultichannelFormat multiChanFmt;
    bool                   secondaryAudio;
    bool                   pcmAudio;

    /* TS user data config */
    bool                   tsUserDataInput;
    bool                   remapUserDataPid;
    unsigned               userDataPid[NEXUS_MAX_MUX_PIDS];
    unsigned               remappedUserDataPid[NEXUS_MAX_MUX_PIDS];
    size_t                 numUserDataPids;

    /* frontend config */
#if  NEXUS_HAS_FRONTEND
    unsigned               freq;
    NEXUS_FrontendQamMode  qamMode;
    NEXUS_FrontendHandle   frontend;
#endif

    /* even handles for input */
    BKNI_EventHandle       dataReadyEvent, eofEvent;
    B_ThreadHandle         eofHandler;
    B_ThreadHandle         feedThread;
} xcode_inputSettings;

typedef struct BTST_Transcoder_t
{
    unsigned               id;

    /* user config */
    bool                   custom; /* if true, user configures test settings; else, default test config. */
    bool                   loop;
    bool                   nonRealTime;
    unsigned               vpipes;
    xcode_inputSettings    input;
    xcode_outputSettings   output;

    /* gfx */
    NEXUS_SurfaceHandle    framebuffer;

    /* xcode settings */
    BXCode_StartSettings   startSettings;
    BXCode_Settings        settings;

    B_MutexHandle          mutexStarted;
    bool                   started;

    BXCode_Handle          hBxcode;
    NEXUS_Timebase         timebase;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindowHandle window;
#endif
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_VideoImageInputStatus imageInputStatus;
    struct {
        NEXUS_SurfaceHandle handle;
        bool submitted;
    } surface[BTST_NUM_SURFACES];
} BTST_Transcoder_t;

typedef struct BTST_Context_t
{
    BTST_Transcoder_t xcodeContext[NEXUS_MAX_VIDEO_ENCODERS];
    bool loopbackPlayer;
    bool loopbackStarted;
    unsigned loopbackXcodeId;
    bool autoQuit;
    bool decoderZeroUp;
    bool enableDebugSdDisplay;
    bool scriptMode;
    unsigned scriptModeSleepTime;
    unsigned selectedXcodeContextId;
    unsigned activeXcodeCount;
    BKNI_EventHandle       doneEvent;

    /* xcode loopback player context */
    bool                       logClosedCaption;
    NEXUS_Timebase             timebaseLoopback;
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
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle    syncChannelLoopback;
#endif

    /* input handles */
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle      hdmiInput;
#endif
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendHandle       frontend;
#endif

    /* gfx handle */
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle       checkpointEvent, spaceAvailableEvent;
    unsigned               gfxRefCnt;
} BTST_Context_t;

extern BTST_Context_t g_testContext;
extern const namevalue_t g_outputTypeStrs[];
extern const namevalue_t g_inputTypeStrs[];
extern NEXUS_PlatformConfiguration g_platformConfig;
extern char g_keyReturn;

void print_usage(void);
void xcode_index_filename(char *indexFile, const char *mediaFile, bool segmented, NEXUS_TransportType type);
void config_xcoder_context (BTST_Transcoder_t *pContext);
int cmdline_parse(int argc, char **argv, BTST_Context_t *pContext);
void xcode_loopback_setup( BTST_Context_t  *pContext );
void xcode_loopback_shutdown( BTST_Context_t *pContext );
void loopbackPlayer( BTST_Context_t *pContext );
void printInputStatus(BTST_Transcoder_t  *pContext);
void printOutputStatus(BTST_Transcoder_t  *pContext);
void printMenu(void);
void print_xcoder_inputSetting(BTST_Transcoder_t *pContext);
void print_xcoder_outputSetting(BTST_Transcoder_t *pContext);
void keyHandler( BTST_Context_t *pContext );
NEXUS_Error bringup_transcode(BTST_Transcoder_t  *pContext);
void shutdown_transcode(BTST_Transcoder_t  *pContext);
unsigned get_display_index(unsigned encoder);
void close_gfx( BTST_Transcoder_t *pContext );

#endif /* !defined XCODE_CONFIG_H__ */
