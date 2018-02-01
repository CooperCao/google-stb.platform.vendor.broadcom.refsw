/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* nexus unittest: transcode TS file source to mp4 file output in FNRT mode */

#include "nexus_platform.h"
#if !NEXUS_HAS_VIDEO_ENCODER
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#else

#include "nexus_video_decoder.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_video_input.h"
#include "nexus_core_utils.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_STREAM_MUX
#include "nexus_playpump.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "b_os_lib.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_input.h"
#include "nexus_sync_channel.h"
#endif
#include "bcmplayer.h"
#include "bmedia_probe.h"
#include "bfile_stdio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include "bstd.h"
#include "bkni.h"
#include "bfifo.h"

/* NRT a+v transcode STC av_window enable test */
#define BTST_ENABLE_NRT_STC_AV_WINDOW 1
#define BTST_ENABLE_AV_SYNC           1

/* TODO: set to 1 to disable the debug monitor for NRT mode source decode; set to 0 to enable it. */
#define BTST_DISABLE_NRT_DEBUG_DISPLAY 0

/* TODO: Set to 0 to enable auto stop of video using the LAST BTP */
#define BTST_DISABLE_AUTO_STOP 0

#define BTST_ENABLE_XCODE_LOOPBACK   0

/***********************************
 *   Display assignments
 */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
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


/***********************************
 *   Video Decoder assignments
 */
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#define BTST_XCODE_VID_DECODE_IDX 0
#else
#define BTST_XCODE_VID_DECODE_IDX       (NEXUS_NUM_VIDEO_DECODERS-1)
#endif

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#define BTST_INPUT_MAX_WIDTH 416
#define BTST_INPUT_MAX_HEIGHT 224
#endif

#define BTST_MUX_PCR_PID     0x11
#define BTST_MUX_VIDEO_PID   0x12
#define BTST_MUX_AUDIO_PID   0x13

/***********************************
 *   Chunk parameters
 */
#define BTST_DEFAULT_CHUNK_DURATION            10000 /* 10 secs */

BDBG_MODULE(transcode_hls);
BDBG_FILE_MODULE(chunk_hls);
BDBG_FILE_MODULE(btp_hls);
BDBG_FILE_MODULE(playpump_hls);
BDBG_FILE_MODULE(chunk_state);

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_STREAM_MUX
typedef struct EncodeSettings {
	char                    fname[256];
	NEXUS_VideoFormat       displayFormat;
	NEXUS_VideoFrameRate    encoderFrameRate;
	int                     encoderBitrate;
	int                     encoderGopStructureFramesP;
	int                     encoderGopStructureFramesB;
	NEXUS_VideoCodec        encoderVideoCodec;
	NEXUS_VideoCodecProfile encoderProfile;
	NEXUS_VideoCodecLevel   encoderLevel;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;
	bool                    bCustom;
	bool                    bAudioEncode;
	NEXUS_AudioCodec        encoderAudioCodec;
	NEXUS_VideoEncoderSettings videoEncoderConfig;
}  EncodeSettings;

typedef struct InputSettings{
	int                    resource;
	char                   fname[256];
	NEXUS_TransportType    eStreamType;
	NEXUS_VideoCodec       eVideoCodec;
	NEXUS_AudioCodec       eAudioCodec;
	NEXUS_AudioMultichannelFormat eMultiChanFmt;
	unsigned               iVideoPid;
	unsigned               iAudioPid;
	unsigned               iPcrPid;
	bool                   bAudioInput;
	bool                   bConfig;
} InputSettings;

typedef struct VideoChunk{
	long                       startRapIndex;
	long                       endRapIndex;
	BNAV_Player_Position       startPosition;
	BNAV_Player_Position       endPosition;
	uint32_t                   chunkId;
	off_t                      startOffset;
	off_t                      endOffset;
} VideoChunk;

#define HLS_DURATION_FIFO_SIZE 16
typedef struct TranscodeContext {
	/* xcoder handles */
	NEXUS_VideoEncoderStartSettings vidEncoderStartSettings;
	NEXUS_VideoEncoderHandle   videoEncoder;
	NEXUS_DisplayHandle        displayTranscode;
	NEXUS_VideoWindowHandle    windowTranscode;
	NEXUS_StcChannelHandle     stcVideoChannel;
	NEXUS_PlaypumpHandle       playpump;
	BKNI_EventHandle           dataReadyEvent;
	BKNI_EventHandle           chunkDoneEvent;
	NEXUS_PidChannelHandle     videoPidChannel;
	NEXUS_VideoDecoderHandle   videoDecoder;
	NEXUS_VideoDecoderStartSettings vidProgram;
	int                        file;
	B_ThreadHandle             playpumpThread;
	B_MutexHandle              mutexStarted;
	bool                       bStarted;
	int                        xcodeId;
	VideoChunk                 chunk;
	VideoChunk                 prevChunk;
	off_t                      fileOffset;

	/* audio */
	NEXUS_AudioMixerHandle     audioMixer;
	NEXUS_AudioEncoderHandle   audioEncoder;
	NEXUS_AudioMuxOutputHandle audioMuxOutput;
	NEXUS_AudioMuxOutputStartSettings audMuxStartSettings;
	NEXUS_StcChannelHandle     stcAudioChannel;
	NEXUS_PidChannelHandle     audioPidChannel;
	NEXUS_AudioDecoderHandle   audioDecoder;
	NEXUS_AudioDecoderStartSettings audProgram;

	BKNI_EventHandle           finishEvent;

	/* ts stream mux/record handles */
	NEXUS_PlaypumpHandle       playpumpTranscodeVideo;
	NEXUS_PidChannelHandle     pidChannelTranscodeVideo;
	NEXUS_PlaypumpHandle       playpumpTranscodeAudio;
	NEXUS_PidChannelHandle     pidChannelTranscodeAudio;
	NEXUS_StreamMuxHandle      streamMux;
	NEXUS_PlaypumpHandle       playpumpTranscodePcr;
	NEXUS_StcChannelHandle     stcChannelTranscode;
	int                        fileTranscode;
	FILE                      *fileHls;/* HLS index file */
	char                       hlsFname[256];
	BKNI_EventHandle           recpumpEvent;
	B_ThreadHandle             recpumpThread;
	NEXUS_RecpumpHandle        recpump;
	NEXUS_PidChannelHandle     pidChannelTranscodePcr;
	NEXUS_StreamMuxStartSettings tsMuxConfig;

	/* source local decode/display handles */
	NEXUS_DisplayHandle        display;
	NEXUS_VideoWindowHandle    window;


	/* video encoder settings */
	EncodeSettings             encodeSettings;

	/* input settings */
	InputSettings              inputSettings;

	FILE                      *indexFile;/* source TS stream's nav file */
	BNAV_Player_Handle         bcmplayer;

	/* xcode system settings */
#if BTST_ENABLE_AV_SYNC
	NEXUS_SyncChannelHandle    syncChannel;
#endif

	/* PSI system data */
	void                     *pat;
	void                     *pmt;
	unsigned                  ccValue;

	VideoChunk                latestVideoChunk;
	B_MutexHandle             mutexChunk;

	unsigned                  duration;/* in ms */
	B_ThreadHandle            monitorThread;
	BKNI_EventHandle          monitorDone;
	bool                      bShutdown;

	BFIFO_HEAD(HlsDurationFifo, unsigned) durationFifo;
	unsigned                  segmentDurations[HLS_DURATION_FIFO_SIZE];

#if BTST_ENABLE_XCODE_LOOPBACK
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
#endif
} TranscodeContext;

/* global context
 * function level context in this */
static TranscodeContext xcodeContext;

static NEXUS_PlatformConfiguration g_platformConfig;
static char g_keyReturn ='\0';
static bool g_bAutoQuit = false;
static bool g_bNoDspMixer = false;
static unsigned g_chunkDuration = BTST_DEFAULT_CHUNK_DURATION; /* ms */
static unsigned g_bitrate = 3000000; /* default video bitrate */
static B_EventHandle g_eotEvent = NULL;/* end of test event */
static off_t g_fileSize = 0;
static bool g_bNrt = true;

static void bringup_transcode( TranscodeContext  *pContext );
static void shutdown_transcode( TranscodeContext  *pContext );

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
		windowMadSettings.enable32Pulldown = true;
		windowMadSettings.enable22Pulldown = true;
		windowMadSettings.gameMode = bLowDelay?NEXUS_VideoWindowGameMode_e4Fields_0Delay : NEXUS_VideoWindowGameMode_eOff;
	}

	NEXUS_VideoWindow_SetMadSettings(hWindow, &windowMadSettings);
}


#if BTST_ENABLE_XCODE_LOOPBACK
static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

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
	NEXUS_PlaybackSettings playbackSettings;
	NEXUS_PlaybackPidChannelSettings playbackPidSettings;
	NEXUS_VideoDecoderSettings vidDecodeSettings;
	NEXUS_VideoDecoderStartSettings videoProgram;
	NEXUS_AudioDecoderStartSettings audioProgram;
	NEXUS_StcChannelSettings stcSettings;
#if BTST_ENABLE_AV_SYNC
	NEXUS_SyncChannelSettings syncChannelSettings;
#endif
	NEXUS_DisplaySettings displaySettings;
	EncodeSettings  *pEncodeSettings = &pContext->encodeSettings;
	InputSettings   *pInputSettings = &pContext->inputSettings;

	BDBG_MSG(("To start the loopback player for transcoder..."));
	NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	pContext->stcChannelLoopback = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
	BDBG_MSG(("Transcoder opened Loopback STC [%p].", pContext->stcChannelLoopback));
#if BTST_ENABLE_AV_SYNC
	/* create a sync channel for xcoder loopback decode */
	NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
	pContext->syncChannelLoopback = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

	printf("start xcode loopback display on HDMI and Component outputs...\n");
	pContext->videoDecoderLoopback = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
	NEXUS_VideoDecoder_GetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);
	BKNI_Memset(vidDecodeSettings.supportedCodecs, 0, sizeof(vidDecodeSettings.supportedCodecs));
	vidDecodeSettings.supportedCodecs[pEncodeSettings->encoderVideoCodec] = true; /* it's for regular HD decode heap allocation; it covers mpeg2/h264/mpeg4 HD size */
	/* channel change mode: hold until first new picture */
	vidDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
	NEXUS_VideoDecoder_SetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);
	pContext->playpumpLoopback = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
	assert(pContext->playpumpLoopback);
	pContext->playbackLoopback = NEXUS_Playback_Create();
	assert(pContext->playbackLoopback);

	pContext->filePlayLoopback = NEXUS_FilePlay_OpenPosix(pEncodeSettings->fname, NULL);
	if (!pContext->filePlayLoopback) {
		fprintf(stderr, "can't open file: %s\n", pEncodeSettings->fname);
		BDBG_ASSERT(0);
	}

	NEXUS_Playback_GetSettings(pContext->playbackLoopback, &playbackSettings);
	playbackSettings.playpump = pContext->playpumpLoopback;
	/* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
	playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)NEXUS_TransportType_eTs;
	playbackSettings.stcChannel = pContext->stcChannelLoopback; /* loopback channel  */
	playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop; /* when play hits the end, loop */
	playbackSettings.timeshifting = true; /* allow for timeshift the growing file  */
	NEXUS_Playback_SetSettings(pContext->playbackLoopback, &playbackSettings);

	/* Open the video pid channel */
	NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
	playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
	playbackPidSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)pEncodeSettings->encoderVideoCodec; /* must be told codec for correct handling */
	playbackPidSettings.pidTypeSettings.video.index = false;
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
	pContext->displayLoopback = NEXUS_Display_Open(0, &displaySettings);
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
    hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
    hdmiOutputSettings.hotplugCallback.context = g_platformConfig.outputs.hdmi[0];
    hdmiOutputSettings.hotplugCallback.param = (int)pContext->displayLoopback;
    NEXUS_HdmiOutput_SetSettings(g_platformConfig.outputs.hdmi[0], &hdmiOutputSettings);

	pContext->windowLoopback = NEXUS_VideoWindow_Open(pContext->displayLoopback, 0);

	NEXUS_VideoWindow_AddInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));

	/* enable window deinterlacer default setting: it'll grab MCVP first */
	window_mad(pContext->windowLoopback, false, true, false);

	/* if audio DAC0 for loopback player */
	if(pInputSettings->bAudioInput)
	{
		/* Open the audio loopback decoder */
		pContext->audioDecoderLoopback = NEXUS_AudioDecoder_Open(NEXUS_NUM_AUDIO_DECODERS-1, NULL);

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
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
			NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));

		/* add hdmi pcm audio output to loopback display */
		NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),
								   NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));

#if BTST_ENABLE_AV_SYNC
		/* connect sync channel before start decode */
		NEXUS_SyncChannel_GetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
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
		NEXUS_VideoDecoder_Start(pContext->videoDecoderLoopback, &videoProgram);

	/* delay the loopback until A2P passed to avoid stutter since playback jams PTS to STC instead of locking PCR so the encode buffer
	   model is not enforced at loopback decoder plus the O_DIRECT timeshift buffer seems to have some latency. */
	BKNI_Sleep(5000);

	/* Start playback */
	NEXUS_Playback_Start(pContext->playbackLoopback, pContext->filePlayLoopback, NULL);
}

void xcode_loopback_shutdown( TranscodeContext  *pContext )
{
#if BTST_ENABLE_AV_SYNC
	NEXUS_SyncChannelSettings syncChannelSettings;
#endif
	InputSettings   *pInputSettings = &pContext->inputSettings;

	BDBG_MSG(("To shutdown the loopback player for transcoder..."));
	NEXUS_Playback_Stop(pContext->playbackLoopback);
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
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
		NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));
		NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
		NEXUS_AudioDecoder_Close(pContext->audioDecoderLoopback);
		NEXUS_Playback_ClosePidChannel(pContext->playbackLoopback, pContext->audioPidChannelLoopback);
	}
	NEXUS_Playback_ClosePidChannel(pContext->playbackLoopback, pContext->videoPidChannelLoopback);
	NEXUS_FilePlay_Close(pContext->filePlayLoopback);
	NEXUS_Playback_Destroy(pContext->playbackLoopback);
	NEXUS_Playpump_Close(pContext->playpumpLoopback);

	NEXUS_VideoWindow_RemoveInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
	NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
	NEXUS_VideoDecoder_Close(pContext->videoDecoderLoopback);
	NEXUS_VideoWindow_Close(pContext->windowLoopback);
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_StopCallbacks(g_platformConfig.outputs.hdmi[0]);
#endif
	NEXUS_Display_Close(pContext->displayLoopback);
#if BTST_ENABLE_AV_SYNC
	if(pContext->syncChannelLoopback) {
		NEXUS_SyncChannel_Destroy(pContext->syncChannelLoopback);
		pContext->syncChannelLoopback = NULL;
	}
#endif

	NEXUS_StcChannel_Close(pContext->stcChannelLoopback);
}
#endif

static void
transcoderFinishCallback(void *context, int param)
{
	BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

	BSTD_UNUSED(param);
	xcodeContext.bShutdown = true;
	BDBG_MSG(("Finish callback invoked, now stop the file mux."));
	BKNI_SetEvent(finishEvent);
}

static bool g_bPauseRecpump = false;
static bool g_bPausePlaypump= false;
static void avsync_correlation_error( TranscodeContext *pContext )
{
	size_t size[2];
	const NEXUS_VideoEncoderDescriptor *vdesc[2];
	const NEXUS_AudioMuxOutputFrame *adesc[2];
	unsigned i, j;
	uint32_t v_opts, a_opts;
	uint64_t v_pts, a_pts, v_stc, a_stc;
	double v_opts2pts = 0.0, a_opts2pts = 0.0, error;
	bool validVframe = false, validAframe = false;

	NEXUS_VideoEncoder_GetBuffer(pContext->videoEncoder, &vdesc[0], &size[0], &vdesc[1], &size[1]);
	for(j=0;j<2 && !validVframe;j++) {
		for(i=0;i<size[j];i++) {
			if(vdesc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID)
			{
				v_opts = vdesc[j][i].originalPts;
				v_pts  = vdesc[j][i].pts;
				v_stc  = vdesc[j][i].stcSnapshot;
				v_opts2pts = (double)v_pts / 90 - (double)v_opts / 45;
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
				a_opts2pts = (double)a_pts / 90 - (double)a_opts / 45;
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

static void keyHandler( TranscodeContext  *pContext )
{
	char key[256];
	int choice;
	NEXUS_VideoEncoderStatus videoEncodeStatus;
	NEXUS_VideoDecoderStatus videoDecodeStatus;
	NEXUS_AudioDecoderStatus audioDecodeStatus;
	NEXUS_StreamMuxStatus muxStatus;
	int rcInput;
	uint32_t ulSecToSleep;

	/* Turn off buffering for stdin */
	setvbuf(stdin, NULL, _IONBF, 0);

	while (g_keyReturn != 'q')
	{
        fd_set rfds;
        struct timeval tv;
        int retval;

		printf("Menu:\n");
		printf("0) Pasue recpump\n");
		printf("1) Resume recpump\n");
		printf("2) Pasue playpump\n");
		printf("3) Resume playpump\n");
		printf("14) Get video xcoder status\n");
		printf("99) change DEBUG module setting\n");
		printf("100) sleep\n");
		printf("\nEnter 'q' to quit\n\n");

        do {
            /* Watch stdin (fd 0) to see when it has input. */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);

            /* Wait up to 100 miliseconds. */
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            retval = select(1, &rfds, NULL, NULL, &tv);
            if(retval<0) BDBG_WRN(("select returns %d\n", retval));
            else if(retval) BDBG_MSG(("Data is available"));
        } while ( g_keyReturn != 'q' && retval <= 0 );

        if ( g_keyReturn == 'q' )
        {
            break;
        }

		rcInput = scanf("%s", key);
		if(!strcmp(key, "q"))
		{
			g_keyReturn = 'q';
			printf("\nquit!\n\n");
			break;
		} else if(rcInput == EOF) return;
		choice = strtoul(key, NULL, 0);
		if(pContext->mutexStarted)
		B_Mutex_Lock(pContext->mutexStarted);
		if(!pContext->bStarted&& (choice!=99)) {
			printf("xcoder not started.\n");
			if(pContext->mutexStarted)
			B_Mutex_Unlock(pContext->mutexStarted);
			continue;
		}
		switch(choice)
		{
			case 0: /* pause recpump */
				g_bPauseRecpump = true;
				break;
			case 1: /* resume recpump */
				g_bPauseRecpump = false;
				break;
			case 2: /* pause playpump */
				g_bPausePlaypump = true;
				break;
			case 3: /* resume playpump */
				g_bPausePlaypump = false;
				break;
			case 14: /* get video encoder status */
				{
					NEXUS_VideoEncoderClearStatus clearStatus;
					void *bufferBase;
					void *metadataBufferBase;
					NEXUS_VideoEncoder_GetStatus(pContext->videoEncoder, &videoEncodeStatus);
					NEXUS_MemoryBlock_Lock(videoEncodeStatus.bufferBlock, &bufferBase);
					NEXUS_MemoryBlock_Lock(videoEncodeStatus.metadataBufferBlock, &metadataBufferBase);
					printf("\nVideo Encoder[%d] Status:\n", 0);
					printf("----------------------\n");
					printf("data buffer base address     = %p\n", bufferBase);
					printf("metadata buffer base address = %p\n", metadataBufferBase);
					printf("error flags                  = 0x%x\n", videoEncodeStatus.errorFlags);
					printf("error count                  = %u\n", videoEncodeStatus.errorCount);
					printf("picture drops due to error   = %u\n", videoEncodeStatus.picturesDroppedErrors);
					printf("picture drops due to FRC     = %u\n", videoEncodeStatus.picturesDroppedFRC);
					printf("pictures Encoded             = %u\n", videoEncodeStatus.picturesEncoded);
					printf("pictures Received            = %u\n", videoEncodeStatus.picturesReceived);
					printf("picture Id Last Encoded      = 0x%x\n", videoEncodeStatus.pictureIdLastEncoded);
					printf("----------------------\n\n");
					NEXUS_MemoryBlock_Unlock(videoEncodeStatus.bufferBlock);
					NEXUS_MemoryBlock_Unlock(videoEncodeStatus.metadataBufferBlock);
					NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
					NEXUS_VideoEncoder_ClearStatus(pContext->videoEncoder, &clearStatus);
					}
					NEXUS_VideoDecoder_GetStatus(pContext->videoDecoder, &videoDecodeStatus);
					printf("\nVideo Decoder[%d] Status:\n",0);
					printf("----------------------\n");
					printf("muted                = %d\n", videoDecodeStatus.muted);
					printf("pictureTag           = %lu\n", videoDecodeStatus.pictureTag);
					printf("current PTS          = 0x%x\n", videoDecodeStatus.pts);
					printf("fifo depth           = %u\n", videoDecodeStatus.fifoDepth);
					printf("fifo size            = %u\n", videoDecodeStatus.fifoSize);
					printf("queueDepth           = %u\n", videoDecodeStatus.queueDepth);
					printf("numDecoded           = %u\n", videoDecodeStatus.numDecoded);
					printf("numDisplayed         = %u\n", videoDecodeStatus.numDisplayed);
					printf("numDecodeErrors      = 0x%x\n", videoDecodeStatus.numDecodeErrors);
					printf("numDecodeOverflows   = 0x%x\n", videoDecodeStatus.numDecodeOverflows);
					printf("numDisplayErrors     = 0x%x\n", videoDecodeStatus.numDisplayErrors);
					printf("numDecodeDrops       = 0x%x\n", videoDecodeStatus.numDecodeDrops);
					printf("numPicturesReceived  = 0x%x\n", videoDecodeStatus.numPicturesReceived);
					printf("numDisplayDrops      = 0x%x\n", videoDecodeStatus.numDisplayDrops);
					printf("numDisplayUnderflows = 0x%x\n", videoDecodeStatus.numDisplayUnderflows);
					printf("PtsErrorCount        = 0x%x\n", videoDecodeStatus.ptsErrorCount);
					printf("----------------------\n\n");
					if(pContext->inputSettings.bAudioInput) {
						NEXUS_AudioDecoder_GetStatus(pContext->audioDecoder, &audioDecodeStatus);
						printf("Audio Decoder[0] Status:\n");
						printf("----------------------\n");
						printf("data buffer depth     = %u\n", audioDecodeStatus.fifoDepth);
						printf("data buffer size      = %u\n", audioDecodeStatus.fifoSize);
						printf("queued frames         = %u\n", audioDecodeStatus.queuedFrames);
						printf("numDecoded count      = %u\n", audioDecodeStatus.framesDecoded);
						printf("numDummyFrames        = %u\n", audioDecodeStatus.dummyFrames);
						printf("numFifoOverflows      = %u\n", audioDecodeStatus.numFifoOverflows);
						printf("numFifoUnderflows     = %u\n", audioDecodeStatus.numFifoUnderflows);
						printf("current PTS (45KHz)   = 0x%x\n", audioDecodeStatus.pts);
						printf("PTS error count       = %u\n", audioDecodeStatus.ptsErrorCount);
						printf("----------------------\n\n");
					}
					NEXUS_StreamMux_GetStatus(pContext->streamMux, &muxStatus);
					{unsigned hrs, mins, secs;
						secs = muxStatus.duration/1000;
						mins = secs/60;
						hrs  = mins/60;
						mins = mins%60;
						secs = secs%60;
					printf("\nTS Mux Status:\n");
					printf("----------------------\n");
					printf("completed media duration: %02u:%02u:%02u.%03u\n", hrs, mins, secs, muxStatus.duration%1000);
					printf("----------------------\n\n");
					if(pContext->inputSettings.bAudioInput) {
						avsync_correlation_error(pContext);
					}
				}
				break;
			case 100:
				printf("\n How many seconds to sleep?");
				scanf("%u", &ulSecToSleep);
				sleep(ulSecToSleep);
				break;
			case 99: /* debug module setting */
#if BDBG_DEBUG_BUILD
{
				int  iDbgLevel;
				char achName[256];
				printf("\nPlease enter the debug module name: ");
				scanf("%s", achName);
				printf("(%d)Trace (%d)Message (%d)Warning (%d)Error\n",
					BDBG_eTrace, BDBG_eMsg, BDBG_eWrn, BDBG_eErr);
				printf("Which debug level do you want to set it to? ");
				scanf("%d", &iDbgLevel);
				BDBG_SetModuleLevel(achName, (BDBG_Level)iDbgLevel);
}
#endif
				break;
			default:
				break;
		}
		if(pContext->mutexStarted)
		B_Mutex_Unlock(pContext->mutexStarted);
	}
	if(g_eotEvent) {
		B_Event_Set(g_eotEvent);
	}
}

static void xcode_index_filename(char *indexFile, const char *mediaFile, bool hls )
{
	char *tmpPtr;
	strncpy(indexFile, mediaFile, strlen(mediaFile));
	tmpPtr = strstr(indexFile, ".");
	if (tmpPtr) {
	    strncpy(tmpPtr+1, hls?"hls":"nav", 3);
	}
	else {
	    strncat(indexFile, hls?".hls":".nav", sizeof(indexFile)-1);
	}
	BDBG_MSG(("Media file name: %s, index file name %s", mediaFile, indexFile));
}

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}

static void open_tsmux( TranscodeContext  *pContext )
{
	NEXUS_StreamMuxCreateSettings muxCreateSettings;
	NEXUS_StreamMuxStartSettings muxConfig;
	NEXUS_PlaypumpOpenSettings playpumpConfig;
	NEXUS_RecpumpOpenSettings recpumpOpenSettings;
	NEXUS_RecpumpSettings recpumpSettings;

	NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
	playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
	playpumpConfig.numDescriptors = 64; /* set number of descriptors */
	playpumpConfig.streamMuxCompatible = true;
	pContext->playpumpTranscodeVideo = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
	assert(pContext->playpumpTranscodeVideo);
#if BTST_DISABLE_VIDEO_ENCODER_PES_PACING
	NEXUS_Playpump_SuspendPacing(pContext->playpumpTranscodeVideo, true);
#endif

	pContext->playpumpTranscodePcr = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
	assert(pContext->playpumpTranscodePcr);

	BKNI_CreateEvent(&pContext->finishEvent);
	NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
	muxCreateSettings.finished.callback = transcoderFinishCallback;
	muxCreateSettings.finished.context = pContext->finishEvent;
	pContext->streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
	NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
	muxConfig.transportType = NEXUS_TransportType_eTs;
	muxConfig.stcChannel = pContext->stcChannelTranscode;
	muxConfig.nonRealTime = g_bNrt;
	muxConfig.nonRealTimeRate = 32 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
	muxConfig.servicePeriod = 50; /* MSP */

	muxConfig.video[0].pid = BTST_MUX_VIDEO_PID;
	muxConfig.video[0].encoder = pContext->videoEncoder;
	muxConfig.video[0].playpump = pContext->playpumpTranscodeVideo;

	muxConfig.pcr.pid = BTST_MUX_PCR_PID;
	muxConfig.pcr.playpump = pContext->playpumpTranscodePcr;
	muxConfig.pcr.interval = 50;

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
	pContext->recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
	assert(pContext->recpump);

	BKNI_CreateEvent(&pContext->recpumpEvent);
	NEXUS_Recpump_GetSettings(pContext->recpump, &recpumpSettings);
	/* NOTE: enable band hold to allow recpump to stall TS mux to avoid ave data corruption in case file i/o is slow
	 * The encoders should make sure no output CDB/ITB buffer corruption (instead do frame drop) when output overflow! */
	recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = pContext->recpumpEvent;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = pContext->recpumpEvent; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
	NEXUS_Recpump_SetSettings(pContext->recpump, &recpumpSettings);

	/* store the mux config */
	pContext->tsMuxConfig = muxConfig;

	/* open PidChannels */
	pContext->pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(pContext->playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
	assert(pContext->pidChannelTranscodePcr);

	/* set record index file name and open the record file handle */
	xcode_index_filename(pContext->hlsFname, pContext->encodeSettings.fname, true);/* HLS segments index file */

}

static void xcode_audio_tsmux_record( TranscodeContext  *pContext )
{
	NEXUS_PlaypumpOpenSettings playpumpConfig;

	/* audio playpump here is for ts muxer */
	NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
	playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
	playpumpConfig.numDescriptors = 64; /* set number of descriptors */
	playpumpConfig.streamMuxCompatible = true;
	pContext->playpumpTranscodeAudio = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
	assert(pContext->playpumpTranscodeAudio);

	pContext->tsMuxConfig.audio[0].pid = BTST_MUX_AUDIO_PID;
	pContext->tsMuxConfig.audio[0].muxOutput = pContext->audioMuxOutput;
	pContext->tsMuxConfig.audio[0].playpump = pContext->playpumpTranscodeAudio;
}

static const struct {
    double frequency;
    NEXUS_VideoFrameRate nexusFramerate;
} verticalfrequency[NEXUS_VideoFrameRate_eMax] = {
    {0, NEXUS_VideoFrameRate_eUnknown},
    {23.976, NEXUS_VideoFrameRate_e23_976},
    {24, NEXUS_VideoFrameRate_e24},
    {25, NEXUS_VideoFrameRate_e25},
    {29.970, NEXUS_VideoFrameRate_e29_97},
    {30, NEXUS_VideoFrameRate_e30},
    {50, NEXUS_VideoFrameRate_e50},
    {59.940, NEXUS_VideoFrameRate_e59_94},
    {60, NEXUS_VideoFrameRate_e60},
    {14.985, NEXUS_VideoFrameRate_e14_985}};

static double convert_RefreshRate_FromFrameRate(NEXUS_VideoFrameRate frameRate)
{
    unsigned i;
    for(i=0;i<sizeof(verticalfrequency)/sizeof(*verticalfrequency);i++) {
        if (frameRate == verticalfrequency[i].nexusFramerate) {
            return verticalfrequency[i].frequency;
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return 0; /* NEXUS_VideoFrameRate_eUnknown */
}


static void
vidSrcStreamChangedCallback(void *context, int param)
{
	NEXUS_VideoDecoderStreamInformation streamInfo;
	NEXUS_VideoDecoderHandle decoder = (NEXUS_VideoDecoderHandle)context;

	BSTD_UNUSED(param);
	NEXUS_VideoDecoder_GetStreamInformation(decoder, &streamInfo);
	BDBG_MSG(("Video Source Stream Change callback: %ux%u@%.3f%c",
		streamInfo.sourceHorizontalSize,
		streamInfo.sourceVerticalSize,
		convert_RefreshRate_FromFrameRate(streamInfo.frameRate),
		streamInfo.streamProgressive? 'p' : 'i'));
}

static void
vidChunkDoneCallback(void *context, int param)
{
	BKNI_EventHandle event = (BKNI_EventHandle)context;

	BDBG_MSG(("Video[%d] chunk done callback...", param));
	BKNI_SetEvent(event);
}

static void seekNextChunk(VideoChunk *nextChunk /* output */)
{
    BNAV_Player_Position *startPosition = &xcodeContext.latestVideoChunk.startPosition;
    BNAV_Player_Position *endPosition   = &xcodeContext.latestVideoChunk.endPosition;
    unsigned long dpts = 0;
    long index;

    BDBG_ASSERT(nextChunk);

    /* the latest chunk is global to be protected by mutex */
    B_Mutex_Lock(xcodeContext.mutexChunk);
    index = BNAV_Player_FindIFrameFromIndex(xcodeContext.bcmplayer, xcodeContext.latestVideoChunk.startRapIndex, 1);
    BNAV_Player_GetPositionInformation(xcodeContext.bcmplayer, index, startPosition);
    /* find the end of chunk index */
    do {
        /* kludge: in case this function returns index backwards by 1, search 2 ahead */
        index = BNAV_Player_FindIFrameFromIndex(xcodeContext.bcmplayer, index+2, 1);
        if(index == -1) /* end of file */ {
            endPosition->offsetLo = (unsigned long) (-1);
            endPosition->offsetHi = (unsigned long) (-1);
            endPosition->index    = -1;
            break;
        }
        BNAV_Player_GetPositionInformation(xcodeContext.bcmplayer, index, endPosition);
        /* discontinuity may result in a short chunk; but it should be ok */
        dpts = (endPosition->pts >= startPosition->pts)? (endPosition->pts - startPosition->pts)
                : ((unsigned long)(-1) - startPosition->pts + 1 + endPosition->pts);
        BDBG_MODULE_MSG(chunk_hls, ("dpts[%lu], start[%ld]@pts=%08x, end[%ld] pts=%08x", dpts, startPosition->index, (unsigned)startPosition->pts, index, (unsigned)endPosition->pts));
    } while(dpts/45 < g_chunkDuration);

    /* round down offset to 188-byte TP aligned */
    startPosition->offsetLo = startPosition->offsetLo / 188 * 188;
    if(endPosition->index != -1) {/* end of file no need to round down */
        endPosition->offsetLo   = endPosition->offsetLo / 188 * 188;
    } else {
        long firstIndex, lastIndex;
        BNAV_Player_Position position;
        BNAV_Player_DefaultGetBounds(xcodeContext.bcmplayer, (void *)xcodeContext.indexFile, &firstIndex,&lastIndex);
        BNAV_Player_GetPositionInformation(xcodeContext.bcmplayer, lastIndex, &position);
        dpts = (position.pts >= startPosition->pts)? (position.pts - startPosition->pts)
                : ((unsigned long)(-1) - startPosition->pts + 1 + position.pts);
    }
    BDBG_MSG(("Chunk start at index %ld, end at index %ld with duration %u ms\n",
              xcodeContext.latestVideoChunk.startRapIndex, index, (unsigned)(dpts/45)));
    xcodeContext.latestVideoChunk.endRapIndex = index;

    xcodeContext.latestVideoChunk.startOffset = xcodeContext.latestVideoChunk.startPosition.offsetLo + ((uint64_t)xcodeContext.latestVideoChunk.startPosition.offsetHi<<32);
    xcodeContext.latestVideoChunk.endOffset   = xcodeContext.latestVideoChunk.endPosition.offsetLo + ((uint64_t)xcodeContext.latestVideoChunk.endPosition.offsetHi<<32);

    /* copy the chunk info */
    *nextChunk = xcodeContext.latestVideoChunk;

    /* bump up chunk id to prepare the next chunk */
    xcodeContext.latestVideoChunk.chunkId++;
    xcodeContext.latestVideoChunk.startRapIndex = xcodeContext.latestVideoChunk.endRapIndex;

    /* update duration fifo writer */
    if(BFIFO_WRITE_LEFT(&xcodeContext.durationFifo)==0) {
        BDBG_ERR(("Duration FIFO is full!"));
    }
    *BFIFO_WRITE(&xcodeContext.durationFifo) = dpts/45; /* ms */
    BFIFO_WRITE_COMMIT(&xcodeContext.durationFifo, 1);

    B_Mutex_Unlock(xcodeContext.mutexChunk);
}

typedef enum BTPCommand
{
   BTPCommand_eChunkId = 0x0D, /* reuse PICTURE_TAG */
   BTPCommand_eLast = 0x82, /* LAST */
   BTPCommand_eEOS = 0x0A /* protocol agnostic EOS or so-called INLINE_FLUSH or TPD */
} BTPCommand;

static void insertBTP(void *buf, uint32_t chunkId, unsigned pid, BTPCommand command)
{
    int i;
    static uint8_t btp[188] = {
            0x47, 0x1f, 0xff, 0x20,
            0xb7, 0x02, 0x2d, 0x00,
            'B',  'R',  'C',  'M',  /* signature */
            0x00, 0x00, 0x00, 0x0d, /* CHUNK_ID command (reuse PICTURE_TAG) */
            0x00, 0x00, 0x00, 0x00, /* dwod 1 */
            0x00, 0x00, 0x00, 0x00, /* dwod 2 */
            0x00, 0x00, 0x00, 0x00, /* dwod 3 */
            0x00, 0x00, 0x00, 0x00, /* dwod 4 */
            0x00, 0x00, 0x00, 0x00, /* dwod 5 */
            0x00, 0x00, 0x00, 0x00, /* dwod 6 */
            0x00, 0x00, 0x00, 0x00, /* dwod 7 */
            0x00, 0x00, 0x00, 0xbc, /* dwod 8 =188 to avoid RAVE dropping following packets */
            0x00, 0x00, 0x00, 0x00, /* dwod 9: chunkID in big endian */
            /* rest of BTP = 0x00 */
        };

    /* mutex to protect atomic static array from multi-thread */
    B_Mutex_Lock(xcodeContext.mutexChunk);
    btp[1] = (pid >> 8) & 0x1f;
    btp[2] = pid & 0xff;  /* PID */

    btp[12 + 3] = command;

    switch(command)
    {
       case BTPCommand_eChunkId:
       case BTPCommand_eLast:
       case BTPCommand_eEOS:
        /* big endian dword[9] for CHUNK_ID BTP command's chunkID payload */
        btp[12+36] = (unsigned char) ((chunkId & 0xff000000) >> 24);
        btp[12+36+1] = (unsigned char) ((chunkId & 0x00ff0000) >> 16);
        btp[12+36+2] = (unsigned char) ((chunkId & 0x0000ff00) >> 8);
        btp[12+36+3] = (unsigned char) (chunkId & 0x000000ff);
        break;
    }
    BKNI_Memcpy(buf,(void *)btp,188);
    BDBG_MODULE_MSG(btp_hls, ("BTP:"));
    for(i=0; i<52; i+=4) {
        BDBG_MODULE_MSG(btp_hls, ("%02x %02x %02x %02x", btp[i], btp[i+1], btp[i+2], btp[i+3]));
    }
    B_Mutex_Unlock(xcodeContext.mutexChunk);
}

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

/* video chunk xcoder playpump feeder */
static void videoPlaypump_thread(
    TranscodeContext  *pContext )
{
    typedef enum BTST_P_ChunkState {
        BTST_P_ChunkState_eGetNextChunk=0,
        BTST_P_ChunkState_eStart,
        BTST_P_ChunkState_eFeed,
        BTST_P_ChunkState_eToEnd,
        BTST_P_ChunkState_eWaitForDone,
        BTST_P_ChunkState_eEOF,
        BTST_P_ChunkState_eDone
    } BTST_P_ChunkState;

    /* while loop until EOF or quit */
    {
        BTST_P_ChunkState chunkState = BTST_P_ChunkState_eGetNextChunk;
        long startRapIndex;

        BDBG_MODULE_MSG(chunk_state, ("[%u] START --> eGetNextChunk", pContext->xcodeId));

        /* 2) while loop for chunk feed */
        while ( (chunkState != BTST_P_ChunkState_eDone) ) {
            void *buffer;
            size_t buffer_size;
            int n, playSize=0;
            NEXUS_Error rc;

            if(g_bPausePlaypump) {
                BKNI_Sleep(1000);
                continue;
            }
            if (NEXUS_Playpump_GetBuffer(pContext->playpump, &buffer, &buffer_size)) {
               break;
            }
            if (buffer_size < ((chunkState==BTST_P_ChunkState_eEOF)? 3*188 : 188)) {
                BKNI_WaitForEvent(pContext->dataReadyEvent, 1000);
                continue;
            }

            /* The first call to get_buffer will return the entire playback buffer.
               If we use it, we're going to have to wait until the descriptor is complete,
               and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
            if (buffer_size > MAX_READ)
                buffer_size = MAX_READ;

            if(g_keyReturn == 'q') { chunkState = BTST_P_ChunkState_eEOF; }
            switch(chunkState) {
               case BTST_P_ChunkState_eGetNextChunk:
                  /* if reached end of index file by another thread, stop this thread now */
                  B_Mutex_Lock(xcodeContext.mutexChunk);
                  startRapIndex = xcodeContext.latestVideoChunk.startRapIndex;
                  B_Mutex_Unlock(xcodeContext.mutexChunk);
                  if(startRapIndex == -1)
                  {
                     chunkState = BTST_P_ChunkState_eEOF;
                     BDBG_MODULE_MSG(chunk_state, ("[%u] eGetNextChunk --> eEOF", pContext->xcodeId));
                  }
                  else
                  {
                     /* 1) Seek the next chunk to start with; */
                     seekNextChunk(&pContext->chunk);
                     pContext->fileOffset = pContext->chunk.startOffset;
                     BDBG_MSG(("Xcoder[%d] to start chunk[%d] from "BDBG_UINT64_FMT" to "BDBG_UINT64_FMT,
                               pContext->xcodeId, pContext->chunk.chunkId,
                               BDBG_UINT64_ARG(pContext->chunk.startOffset),
                               BDBG_UINT64_ARG(pContext->chunk.endOffset)));
                     lseek(pContext->file, pContext->chunk.startOffset, SEEK_SET);
                     chunkState = BTST_P_ChunkState_eStart;
                     BDBG_MODULE_MSG(chunk_state, ("[%u] eGetNextChunk --> eStart", pContext->xcodeId));
                  }
                  continue;
            /* insert start of chunk BTP */
            case BTST_P_ChunkState_eStart:
                insertBTP(buffer, pContext->chunk.chunkId, xcodeContext.inputSettings.iVideoPid, BTPCommand_eChunkId);
                buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize = 188;
                chunkState = BTST_P_ChunkState_eFeed;
                BDBG_MODULE_MSG(chunk_state, ("[%u] eStart --> eFeed", pContext->xcodeId));
                break;
            /* feed until end of chunk */
            case BTST_P_ChunkState_eFeed:
                if ((buffer_size + pContext->fileOffset > (size_t) pContext->chunk.endOffset) && (pContext->chunk.endOffset != -1)) {
                    buffer_size = pContext->chunk.endOffset - pContext->fileOffset;
                    chunkState = BTST_P_ChunkState_eToEnd;
                    BDBG_MODULE_MSG(chunk_state, ("[%u] eFeed --> eToEnd", pContext->xcodeId));
                }
                break;
            /* insert end of chunk BTP */
            case BTST_P_ChunkState_eToEnd:/* end with current chunkID+1 */
                insertBTP(buffer, pContext->chunk.chunkId + 1, xcodeContext.inputSettings.iVideoPid, BTPCommand_eChunkId);
                buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize = 188;
                chunkState = BTST_P_ChunkState_eGetNextChunk;
                BDBG_MODULE_MSG(chunk_state, ("[%u] eToEnd --> eGetNextChunk", pContext->xcodeId));
                break;
            case BTST_P_ChunkState_eEOF:
               insertBTP(buffer, 0, xcodeContext.inputSettings.iVideoPid, BTPCommand_eEOS);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               insertBTP(buffer, 0, xcodeContext.inputSettings.iVideoPid, BTPCommand_eLast);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               insertBTP(buffer, 0, xcodeContext.inputSettings.iVideoPid, BTPCommand_eEOS);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, playSize);
               chunkState = BTST_P_ChunkState_eDone;
               BDBG_MODULE_MSG(chunk_state, ("[%u] eEOF --> eDone", pContext->xcodeId));
               continue;

            case BTST_P_ChunkState_eDone:
               continue;
            default:/* eDone */
                break;
            }

            n = read(pContext->file, buffer, buffer_size);
            if (n < 0) BDBG_ASSERT(0);
            playSize += n;
            pContext->fileOffset += n;/* update file offset */
            if (n == 0 && buffer_size) {
               chunkState = BTST_P_ChunkState_eEOF;
               BDBG_MODULE_MSG(chunk_state, ("[%u] eEOF", pContext->xcodeId));
               BDBG_MSG(("Video[%u] to end playfile...", pContext->xcodeId));
            }
            rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, playSize);
            BDBG_ASSERT(!rc);
            BDBG_MODULE_MSG(playpump_hls, ("V[%d] played %d bytes\n", pContext->xcodeId, n));
        }
    }

    printf("Video playpump is stopped.\n");
	if(g_eotEvent) {
		B_Event_Set(g_eotEvent);
	}
	g_keyReturn = 'q';
}

static void xcode_av_sync(
	TranscodeContext  *pContext,
	NEXUS_VideoEncoderSettings *pVideoEncoderConfig,
	NEXUS_VideoEncoderStartSettings *pVideoEncoderStartConfig )
{
	NEXUS_VideoEncoderDelayRange videoDelay;
	NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
#if BTST_ENABLE_AV_SYNC
	NEXUS_SyncChannelSettings syncChannelSettings;
	NEXUS_StcChannelPairSettings stcAudioVideoPair;
#endif
	unsigned Dee;

	/*****************************************
	 * calculate video encoder A2P delay
	 */
	/* NOTE: video encoder delay is in 27MHz ticks */
	NEXUS_VideoEncoder_GetDelayRange(pContext->videoEncoder, pVideoEncoderConfig, pVideoEncoderStartConfig, &videoDelay);
	BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));

	if(pContext->inputSettings.bAudioInput)
	{
		NEXUS_AudioMuxOutput_GetDelayStatus(pContext->audioMuxOutput, pContext->encodeSettings.encoderAudioCodec, &audioDelayStatus);
		BDBG_WRN(("\n\tAudio codec %d end-to-end delay = %u ms", pContext->encodeSettings.encoderAudioCodec, audioDelayStatus.endToEndDelay));

		Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
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

		/* Start audio mux output */
		pContext->audMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
		pContext->audMuxStartSettings.stcChannel = pContext->stcChannelTranscode;
	}
	else
		pVideoEncoderConfig->encoderDelay = videoDelay.min;

	/* connect sync channel */
#if BTST_ENABLE_AV_SYNC
	/* create a sync channel */
	NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
	pContext->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
	NEXUS_SyncChannel_GetSettings(pContext->syncChannel, &syncChannelSettings);
	syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder);
	if(pContext->inputSettings.bAudioInput)
	{
		if(pContext->encodeSettings.bAudioEncode)
			syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
		else
			syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed);

		/* NRT mode pairs AV stc channels */
		if(g_bNrt) {
			NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
			stcAudioVideoPair.connected = true;
			stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
			NEXUS_StcChannel_SetPairSettings(pContext->stcVideoChannel, pContext->stcAudioChannel, &stcAudioVideoPair);
		}
	}

	NEXUS_SyncChannel_SetSettings(pContext->syncChannel, &syncChannelSettings);
#endif
}

static int open_video_transcode(
	TranscodeContext *pContext )
{
	NEXUS_StcChannelSettings stcSettings;
	NEXUS_VideoDecoderStartSettings videoProgram;
	NEXUS_VideoDecoderSettings videoDecodeSettings;
	NEXUS_VideoEncoderCapabilities encoderCap;
	NEXUS_VideoEncoderSettings videoEncoderConfig;
	NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
	NEXUS_PlaypumpSettings playpumpSettings;

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
	NEXUS_DisplayStgSettings stgSettings;
#endif
	NEXUS_DisplaySettings displaySettings;
	NEXUS_VideoWindowSettings windowSettings;
	NEXUS_VideoWindowScalerSettings scalerSettings;
	NEXUS_VideoFormatInfo fmtInfo;
	bool bInterlacedXcode;
	NEXUS_DisplayCustomFormatSettings customFormatSettings;
	NEXUS_Error rc;
	unsigned decoderId = 0;
	EncodeSettings  *pEncodeSettings = &pContext->encodeSettings;
	InputSettings   *pInputSettings = &pContext->inputSettings;

	if(pContext->videoEncoder) {
		return 0;
	}

	/* each transcoder potentially should only use 2 STCs on the same timebase:
	       1) NRT video decode STC; (RT/NRT)
	       2) NRT audio decode STC; (NRT)
	       3) encode/mux STC;  (RT)
	   NOTE: to avoid the 3rd one since NRT mode doesn't really need it,
	         encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
	 */
	NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
	stcSettings.timebase = NEXUS_Timebase_e0;
	stcSettings.mode = NEXUS_StcChannelMode_eAuto;
	/*stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;*/
	pContext->stcVideoChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
	BDBG_MSG(("Transcoder opened source vSTC [%p].", (void*)pContext->stcVideoChannel));

	if(!g_bNrt) {/* RT mode encoder has separate STC */
		NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
		stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
		stcSettings.mode = NEXUS_StcChannelMode_eAuto;/* for encoder&mux, only timebase matters */
		stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
		pContext->stcChannelTranscode = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
	} else {/* NRT audio decoder has separate STC similar like video decoder, which could be shared with encoder */
		pContext->stcChannelTranscode = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
	}

	/* bring up decoder and connect to local display */
	{
		NEXUS_VideoDecoderOpenSettings openSettings;
		NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
		openSettings.avc51Enabled = false;
		openSettings.svc3dSupported = false;
		openSettings.excessDirModeEnabled = false;
		openSettings.enhancementPidChannelSupported = false;
		do {
			pContext->videoDecoder = NEXUS_VideoDecoder_Open(NEXUS_NUM_VIDEO_DECODERS-decoderId-1, &openSettings); /* take default capabilities */
			decoderId++;
		} while (!pContext->videoDecoder && decoderId < NEXUS_NUM_VIDEO_DECODERS);
		BDBG_MSG(("Transcoder opened source decoder %d.", NEXUS_NUM_VIDEO_DECODERS - decoderId));
		NEXUS_VideoDecoder_GetSettings(pContext->videoDecoder, &videoDecodeSettings);
		videoDecodeSettings.streamChanged.callback = vidSrcStreamChangedCallback;
		videoDecodeSettings.streamChanged.context  = pContext->videoDecoder;
		BKNI_Memset(videoDecodeSettings.supportedCodecs, 0, sizeof(videoDecodeSettings.supportedCodecs));
		videoDecodeSettings.supportedCodecs[pInputSettings->eVideoCodec] = true; /* it's for regular HD decode heap allocation; it covers mpeg2/h264/mpeg4 HD size */
		videoDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture;

		/* FNRT settings */
		BKNI_CreateEvent(&pContext->chunkDoneEvent);
		videoDecodeSettings.fnrtSettings.chunkDone.param = 0;
		videoDecodeSettings.fnrtSettings.chunkDone.context  = pContext->chunkDoneEvent;
		videoDecodeSettings.fnrtSettings.chunkDone.callback = vidChunkDoneCallback;
		videoDecodeSettings.fnrtSettings.enable = false;
		videoDecodeSettings.fnrtSettings.preChargeCount = 0;

		NEXUS_VideoDecoder_SetSettings(pContext->videoDecoder, &videoDecodeSettings);

		/****************************************
		 * set up xcoder source
		 */
		{
			/* use stdio for file I/O to keep the example simple. */
			pContext->file = open(pInputSettings->fname, O_RDONLY | O_LARGEFILE, 0666);
			if (!pContext->file) {
			    fprintf(stderr, "can't open source file:%s\n", pInputSettings->fname);
			    BDBG_ASSERT(0);;
			}
			rc = fcntl(pContext->file, F_GETFL);
			if ((int)rc != -1) {
				rc = fcntl(pContext->file, F_SETFL, rc | FD_CLOEXEC);
			}
			if(g_fileSize == 0) {/* get the source media file size */
				g_fileSize = lseek(pContext->file, 0, SEEK_END);
				lseek(pContext->file, 0, SEEK_SET);
				BDBG_MSG(("Source file size: "BDBG_UINT64_FMT" bytes", BDBG_UINT64_ARG(g_fileSize)));
			}

			pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
			assert(pContext->playpump);
			BDBG_MSG(("Transcoder opened source playpump."));
			BKNI_CreateEvent(&pContext->dataReadyEvent);
			NEXUS_Playpump_GetSettings(pContext->playpump, &playpumpSettings);
			playpumpSettings.dataCallback.callback = play_callback;
			playpumpSettings.dataCallback.context = pContext->dataReadyEvent;
			NEXUS_Playpump_SetSettings(pContext->playpump, &playpumpSettings);

			/* Open the video pid channel */
			pContext->videoPidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, pInputSettings->iVideoPid, NULL);
		}

		/* Set up decoder Start structures now. We need to know the audio codec to properly set up
		the audio outputs. */
		NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
		videoProgram.codec = (NEXUS_VideoCodec)pInputSettings->eVideoCodec;
		videoProgram.pidChannel = pContext->videoPidChannel;
		videoProgram.stcChannel = pContext->stcVideoChannel;
		videoProgram.nonRealTime = g_bNrt;
		pContext->vidProgram = videoProgram;
	}

	/****************************************
	 * Bring up video display and outputs
	 */
	pContext->videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
	assert(pContext->videoEncoder);
	BDBG_MSG(("opened video encoder 0."));

	/* Bring up video encoder display */
	NEXUS_Display_GetDefaultSettings(&displaySettings);

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
	displaySettings.displayType = NEXUS_DisplayType_eAuto;
	displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
	displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */
#endif

	NEXUS_GetVideoEncoderCapabilities(&encoderCap);
	if(!pEncodeSettings->bCustom)
	{
		displaySettings.format = (NEXUS_VideoFormat)pEncodeSettings->displayFormat;
		pContext->displayTranscode = NEXUS_Display_Open(encoderCap.videoEncoder[0].displayIndex, &displaySettings);
		assert(pContext->displayTranscode);
		BDBG_MSG(("Transcoder opened encoder display%d [%p].", encoderCap.videoEncoder[0].displayIndex, (void*)pContext->displayTranscode));
		pContext->windowTranscode = NEXUS_VideoWindow_Open(pContext->displayTranscode, 0);
		assert(pContext->windowTranscode);
		NEXUS_VideoFormat_GetInfo(displaySettings.format, &fmtInfo);
		bInterlacedXcode = fmtInfo.interlaced;
	}
	else
	{
		pContext->displayTranscode = NEXUS_Display_Open(encoderCap.videoEncoder[0].displayIndex, &displaySettings);
		assert(pContext->displayTranscode);
		BDBG_MSG(("Transcoder opened encoder display%d [%p].", encoderCap.videoEncoder[0].displayIndex, (void*)pContext->displayTranscode));
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

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
	/* NRT setup - AFAP mode */
	NEXUS_Display_GetStgSettings(pContext->displayTranscode, &stgSettings);
	stgSettings.enabled     = true;
	stgSettings.nonRealTime = g_bNrt;
	NEXUS_Display_SetStgSettings(pContext->displayTranscode, &stgSettings);
#endif

	NEXUS_VideoWindow_GetSettings(pContext->windowTranscode, &windowSettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
	windowSettings.position.width = BTST_INPUT_MAX_WIDTH;
	windowSettings.position.height = BTST_INPUT_MAX_HEIGHT;
	windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
	windowSettings.visible = false;
#endif
#else
	/* set transcoder minimum display format before AddInput to avoid black frame transition during dynamic resolution change */
	windowSettings.minimumDisplayFormat = NEXUS_VideoFormat_e1080p;
	windowSettings.scaleFactorRounding.enabled = false;
	windowSettings.scaleFactorRounding.horizontalTolerance = 0;
	windowSettings.scaleFactorRounding.verticalTolerance   = 0;
#endif
	NEXUS_VideoWindow_SetSettings(pContext->windowTranscode, &windowSettings);

	/* set transcoder window vnet mode bias to avoid black frame transition during dynamic resolution change for mfd source;
	   NOTE: HDMI input might dynamically switch to 1080p60 and xcode BVN path doesn't have bandwidth to capture 1080p60! */
	{
		NEXUS_VideoWindow_GetScalerSettings(pContext->windowTranscode, &scalerSettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
		scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
#else
		scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerAfterCapture;
#endif
		scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
		NEXUS_VideoWindow_SetScalerSettings(pContext->windowTranscode, &scalerSettings);
	}


	/* connect same decoder to the encoder display;
	* NOTE: simul display + transcode mode might have limitation in audio path;
	* here is for video transcode bringup/debug purpose;
	*/
	NEXUS_VideoWindow_AddInput(pContext->windowTranscode, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));

	/* VDC madr assignment is fixed */
	window_mad(pContext->windowTranscode, false, true, false);

	/**************************************
	 * encoder settings
	 */
	NEXUS_VideoEncoder_GetSettings(pContext->videoEncoder, &videoEncoderConfig);
	videoEncoderConfig.variableFrameRate = false;
	videoEncoderConfig.frameRate = pEncodeSettings->encoderFrameRate;
	videoEncoderConfig.bitrateMax = pEncodeSettings->encoderBitrate;
	videoEncoderConfig.streamStructure.framesP = pEncodeSettings->encoderGopStructureFramesP;
	videoEncoderConfig.streamStructure.framesB = pEncodeSettings->encoderGopStructureFramesB;

	NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
	videoEncoderStartConfig.codec = pEncodeSettings->encoderVideoCodec;
	videoEncoderStartConfig.profile = pEncodeSettings->encoderProfile;
	videoEncoderStartConfig.level = pEncodeSettings->encoderLevel;
	videoEncoderStartConfig.input = pContext->displayTranscode;
	videoEncoderStartConfig.stcChannel = pContext->stcChannelTranscode;
	videoEncoderStartConfig.interlaced = bInterlacedXcode;
	videoEncoderStartConfig.nonRealTime = g_bNrt;

	/* encoder setting is global */
	pContext->encodeSettings.videoEncoderConfig = videoEncoderConfig;

	/* store per encoder start settings since each instance has different interface */
	pContext->vidEncoderStartSettings = videoEncoderStartConfig;

	return 0;
}

static void close_video_transcode(
	TranscodeContext *pContext )
{
	if(!pContext->videoEncoder) {
		return;
	}

	/************************************
	 * Bring down transcoder
	 */
	{
		NEXUS_Playpump_CloseAllPidChannels(pContext->playpump);
		NEXUS_Playpump_Close(pContext->playpump);
		BKNI_DestroyEvent(pContext->dataReadyEvent);
		close(pContext->file);
	}

	/******************************************
	 * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
	 */
	{
		NEXUS_VideoWindow_RemoveInput(pContext->windowTranscode, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
		NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoder));
		NEXUS_VideoDecoder_Close(pContext->videoDecoder);
		pContext->videoDecoder = NULL;
		BKNI_DestroyEvent(pContext->chunkDoneEvent);
	}
	NEXUS_VideoWindow_Close(pContext->windowTranscode);
	NEXUS_Display_Close(pContext->displayTranscode);

	NEXUS_VideoEncoder_Close(pContext->videoEncoder);
	pContext->videoEncoder = NULL;

	NEXUS_StcChannel_Close(pContext->stcVideoChannel);
	NEXUS_StcChannel_Close(pContext->stcChannelTranscode);
}

/* stop without close */
static void stop_video_transcode(
	TranscodeContext *pContext )
{

	if(!pContext->bStarted) {
		return;
	}

	B_Mutex_Lock(pContext->mutexStarted);
	/**************************************************
	 * NOTE: stop sequence should be in front->back order
	 */
	NEXUS_Playpump_Stop(pContext->playpump);

	NEXUS_VideoDecoder_Stop(pContext->videoDecoder);
	NEXUS_VideoEncoder_Stop(pContext->videoEncoder, NULL);

	B_Thread_Destroy(pContext->playpumpThread);

	/* stopped */
	pContext->bStarted = false;
	B_Mutex_Unlock(pContext->mutexStarted);

}

/* start without open */
static void start_video_transcode(
	TranscodeContext *pContext )
{
	if(pContext->bStarted) {
		BDBG_WRN(("Video xcoder was already started!"));
		return;
	}

	/****************************
	 * start decoders
	 */
	/* Start decoder */
	NEXUS_VideoDecoder_Start(pContext->videoDecoder, &pContext->vidProgram);

	/****************************
	 * setup video encoder
	 */
	NEXUS_VideoEncoder_SetSettings(pContext->videoEncoder, &xcodeContext.encodeSettings.videoEncoderConfig);

	/************************************************
	 * Start video encoder
	 */
	NEXUS_VideoEncoder_Start(pContext->videoEncoder, &pContext->vidEncoderStartSettings);
	/* Start playpump thread */
	NEXUS_Playpump_Start(pContext->playpump);
	g_bPausePlaypump = g_bPauseRecpump = false;
	pContext->playpumpThread = B_Thread_Create("HLS playpump thread", (B_ThreadFunc)videoPlaypump_thread, pContext, NULL);

	/* started */
	pContext->bStarted = true;
}

static int open_audio_transcode(
	TranscodeContext *pContext )
{
	NEXUS_AudioEncoderSettings encoderSettings;
	NEXUS_AudioMixerSettings audioMixerSettings;
	NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
	NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
	EncodeSettings  *pEncodeSettings = &pContext->encodeSettings;
	InputSettings   *pInputSettings = &pContext->inputSettings;

	if(pContext->audioMuxOutput || !pInputSettings->bAudioInput) {
		return 0;
	}

	/* each transcoder potentially should only use 2 STCs on the same timebase:
	       1) NRT video decode STC; (RT/NRT)
	       2) NRT audio decode STC; (NRT)
	       3) encode/mux STC;  (RT)
	   NOTE: to avoid the 3rd one since NRT mode doesn't really need it,
	         encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
	 */
	if(g_bNrt) {
		pContext->stcAudioChannel = pContext->stcChannelTranscode;
		BDBG_MSG(("Transcoder%d opened source aSTC  [%p].", pContext->xcodeId, (void*)pContext->stcAudioChannel));
	} else {
		pContext->stcAudioChannel = pContext->stcVideoChannel;
	}


	/****************************************
	 * set up xcoder source
	 */
	/* Open the video pid channel */
	pContext->audioPidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, pInputSettings->iAudioPid, NULL);

	/* Open the audio decoder */
	/* Connect audio decoders to outputs */
	NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
	pContext->audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);

	/* Set up decoder Start structures now. We need to know the audio codec to properly set up
	the audio outputs. */
	NEXUS_AudioDecoder_GetDefaultStartSettings(&pContext->audProgram);
	pContext->audProgram.codec = pInputSettings->eAudioCodec;
	pContext->audProgram.pidChannel = pContext->audioPidChannel;
	pContext->audProgram.nonRealTime= g_bNrt;
	pContext->audProgram.stcChannel = pContext->stcAudioChannel;

	/* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
	if(!g_bNoDspMixer) {
		NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
		audioMixerSettings.mixUsingDsp = true;
		pContext->audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
		assert(pContext->audioMixer);
	}

	/* Open audio mux output */
	pContext->audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
	assert(pContext->audioMuxOutput);
	BDBG_MSG(("Audio mux output= %p", (void*)pContext->audioMuxOutput));

	if(pEncodeSettings->bAudioEncode)
	{
		/* Open audio encoder */
		NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
		encoderSettings.codec = pEncodeSettings->encoderAudioCodec;
		pContext->audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
		assert(pContext->audioEncoder);

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
					(xcodeContext.inputSettings.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo)?
					NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo));
			audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder,
					(xcodeContext.inputSettings.eMultiChanFmt > NEXUS_AudioMultichannelFormat_eStereo)?
					NEXUS_AudioDecoderConnectorType_eMultichannel : NEXUS_AudioDecoderConnectorType_eStereo);
			NEXUS_AudioMixer_SetSettings(pContext->audioMixer, &audioMixerSettings);

			/* Connect mixer to encoder */
			NEXUS_AudioEncoder_AddInput(pContext->audioEncoder,
				NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
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
		pEncodeSettings->encoderAudioCodec = pContext->audProgram.codec;
	}

	/* Start audio mux output */
	NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
	/* audio NRT requires mux out to take NRT decode STC */
	audioMuxStartSettings.stcChannel        = pContext->stcChannelTranscode;
	audioMuxStartSettings.nonRealTime       = g_bNrt;
	/* store per encoder start settings since each instance has different interface */
	pContext->audMuxStartSettings = audioMuxStartSettings;

	/* Attach outputs for real-time transcoding */
	if(!g_bNrt)
	{
		if(g_bNoDspMixer) {
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]),
				NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, pEncodeSettings->bAudioEncode?
					NEXUS_AudioDecoderConnectorType_eStereo :
					NEXUS_AudioDecoderConnectorType_eCompressed));
		}
		else {
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]),
				NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
		}
	}
	return 0;
}

static void close_audio_transcode(
	TranscodeContext *pContext )
{
	if(!pContext->audioMuxOutput) {
		return;
	}

	/************************************
	 * Bring down transcoder
	 */

	/******************************************
	 * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
	 */
	if(xcodeContext.encodeSettings.bAudioEncode)
	{
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
		NEXUS_AudioEncoder_RemoveAllInputs(pContext->audioEncoder);
		NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(pContext->audioEncoder));
		NEXUS_AudioEncoder_Close(pContext->audioEncoder);
		if(!g_bNoDspMixer) {
			NEXUS_AudioMixer_RemoveAllInputs(pContext->audioMixer);
			NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
			NEXUS_AudioMixer_Close(pContext->audioMixer);
		}
		if(!g_bNrt) {
			NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]));
			NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]));
		}
		NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
		NEXUS_AudioDecoder_Close(pContext->audioDecoder);
	}
	else
	{
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
		if(!g_bNoDspMixer) {
			NEXUS_AudioMixer_RemoveAllInputs(pContext->audioMixer);
			NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pContext->audioMixer));
			NEXUS_AudioMixer_Close(pContext->audioMixer);
		}
		if(!g_bNrt) {
			NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]));
			NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_platformConfig.outputs.audioDummy[0]));
		}
		NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
		NEXUS_AudioDecoder_Close(pContext->audioDecoder);
	}

	NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(pContext->audioMuxOutput));
	NEXUS_AudioMuxOutput_Destroy(pContext->audioMuxOutput);
	pContext->audioMuxOutput = NULL;
}

/* stop without close */
static void stop_audio_transcode(
	TranscodeContext *pContext )
{
	/**************************************************
	 * NOTE: stop sequence should be in front->back order
	 */

	NEXUS_AudioDecoder_Stop(pContext->audioDecoder);

	if(!g_bNoDspMixer) {
		NEXUS_AudioMixer_Stop(pContext->audioMixer);
	}
	NEXUS_AudioMuxOutput_Stop(pContext->audioMuxOutput);
}

/* start without open */
static void start_audio_transcode(
	TranscodeContext *pContext )
{
	/****************************
	 * start decoders
	 */
	/* Start audio mux output */
	NEXUS_AudioMuxOutput_Start(pContext->audioMuxOutput, &pContext->audMuxStartSettings);
	/* Start audio mixer */
	if(!g_bNoDspMixer) {
		NEXUS_AudioMixer_Start(pContext->audioMixer);
	}
	NEXUS_AudioDecoder_Start(pContext->audioDecoder, &pContext->audProgram);
}

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#include "tshdrbuilder.h"
#define BTST_TS_HEADER_BUF_LENGTH   189
#define BTST_MUX_PMT_PID            0x55
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

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if(audPid)
    {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MSG(("\noutput PMT section:"));
    for(i=0; i < (int)pmtState.size; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pat, BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pat + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->pmt, BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->pmt + buf_used, pmt_pl_buf, pmt_pl_size);
    BDBG_MSG(("\nOutput PMT packet:"));
    for(i=0; i < BTST_TS_HEADER_BUF_LENGTH; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x",
            *((uint8_t*)pContext->pmt+i), *((uint8_t*)pContext->pmt+i+1), *((uint8_t*)pContext->pmt+i+2), *((uint8_t*)pContext->pmt+i+3),
            *((uint8_t*)pContext->pmt+i+4), *((uint8_t*)pContext->pmt+i+5), *((uint8_t*)pContext->pmt+i+6), *((uint8_t*)pContext->pmt+i+7)));
    }

}

static void updateSystemData(void *context)
{
	TranscodeContext *pContext = context;
	uint8_t ccByte;

	++pContext->ccValue;/* increment CC synchronously with PAT/PMT */
	ccByte = *((uint8_t*)pContext->pat + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

	/* need to increment CC value for PAT/PMT packets */
	ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
	*((uint8_t*)pContext->pat + 4) = ccByte;

	ccByte = *((uint8_t*)pContext->pmt + 4);
	ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
	*((uint8_t*)pContext->pmt + 4) = ccByte;

	BDBG_MSG(("to insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->pat + 4) & 0xf,
		*((uint8_t*)pContext->pmt + 4) & 0xf));
	return;
}

/*******************************
 * Add system data to stream_mux
 */
static void xcode_create_systemdata( TranscodeContext  *pContext )
{
	uint8_t vidStreamType = 0x0, audStreamType = 0x0;
	uint16_t audPid = 0;
	NEXUS_AudioCodec audCodec = NEXUS_AudioCodec_eUnknown;

	{
		pContext->pat = BKNI_Malloc(BTST_TS_HEADER_BUF_LENGTH);
		pContext->pmt = BKNI_Malloc(BTST_TS_HEADER_BUF_LENGTH);
	}

	/* decide the stream type to set in PMT */
	switch(pContext->encodeSettings.encoderVideoCodec)
	{
		case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
		case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
		case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
		case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
		default:
			BDBG_ERR(("Video encoder codec %d is not supported!\n", pContext->encodeSettings.encoderVideoCodec));
			BDBG_ASSERT(0);
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
			default:
				BDBG_ERR(("Audio encoder codec %d is not supported!\n", audCodec));
		}
	}

	add_pat_pmt(pContext, BTST_MUX_PCR_PID, BTST_MUX_VIDEO_PID, audPid, vidStreamType, audStreamType);
	BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->pat + 4) & 0xf,
		*((uint8_t*)pContext->pmt + 4) & 0xf));

}

static void xcode_release_systemdata( TranscodeContext  *pContext )
{
	BKNI_Free(pContext->pat);
	BKNI_Free(pContext->pmt);
}

/* This thread records transcoded TS stream into .ts file and the intended HLS segments info into .hls file */
static void videoRecpump_thread(
    TranscodeContext  *pContext )
{
	unsigned segment=0;
	unsigned timeout=0;
	bool firstTime=true;
	off_t totalRecordBytes=0;
	off_t bytesRecordedTillCurrentRai=0;
	NEXUS_Error rc;

	/* prepare pat/pmt packets */
	xcode_create_systemdata(pContext);

	/* record the segment #, segment byte offset in TS file, and segment duration in ms */
	fprintf(pContext->fileHls, "segment,fileOffset,duration(ms)\n");
	write(pContext->fileTranscode, (uint8_t*)pContext->pat+1, 188);
	write(pContext->fileTranscode, (uint8_t*)pContext->pmt+1, 188);
	updateSystemData(pContext);

	while((timeout < 3000) && (!pContext->bShutdown)) {
		const void *buffer, *indexBuf;
		size_t datasize=0, indexsize=0;


		if(g_bPauseRecpump) {
			BKNI_Sleep(100);
			continue;
		}

		/* record CDB data */
		rc = NEXUS_Recpump_GetDataBuffer(pContext->recpump, &buffer, &datasize);
		if(rc) break;

		/* ITB contains TPIT results */
		rc = NEXUS_Recpump_GetIndexBuffer(pContext->recpump, &indexBuf, &indexsize);
		if(rc) break;

		#define BRCM_TPIT_ENTRY_SIZE 24 /* HLS NOTE: Each TPIT entry is 6 dwords: 24 bytes */

		if (indexsize) {/* assume one RAI index per segment */
			off_t highByte;

			BDBG_MSG(("indexsize = "BDBG_UINT64_FMT, BDBG_UINT64_ARG((uint64_t)indexsize)));
			/* byte offset since the start of the record session */
			highByte = ((off_t)*((uint32_t*)indexBuf+2) >> 24);
			bytesRecordedTillCurrentRai = highByte << 32;
			bytesRecordedTillCurrentRai |= (off_t)*((uint32_t*)indexBuf+3);
			BDBG_MSG(("Seg[%u] byte offset: "BDBG_UINT64_FMT, segment, BDBG_UINT64_ARG((uint64_t)bytesRecordedTillCurrentRai)));
			BDBG_MSG(("fifo: R[%p], W[%p], duration=%u, idxSize="BDBG_UINT64_FMT, (void*)BFIFO_READ(&pContext->durationFifo), (void*)BFIFO_WRITE(&pContext->durationFifo), *BFIFO_READ(&pContext->durationFifo), BDBG_UINT64_ARG((uint64_t)indexsize)));
			/* log the segment info */
			/* increment segment counter: assume one RAI per segment. */
			fprintf(pContext->fileHls, "%u,", segment++);
			highByte = (bytesRecordedTillCurrentRai+segment*188*2)>>32;
			fprintf(pContext->fileHls, "0x%x%08x,%u\n", (uint32_t)highByte,
				(uint32_t)(bytesRecordedTillCurrentRai+segment*188*2),
				*BFIFO_READ(&pContext->durationFifo));

			/* HLS NOTE: update duration fifo reader: assume one RAI per segment */
			BFIFO_READ_COMMIT(&pContext->durationFifo, 1);
#define BTST_TPIT_DEBUG
#ifdef BTST_TPIT_DEBUG
			BDBG_MSG(("RAI tpit entry: index bytesRead "BDBG_UINT64_FMT", tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x", BDBG_UINT64_ARG((uint64_t)datasize), *(uint32_t*)indexBuf, *((uint32_t*)indexBuf+2), *((uint32_t*)indexBuf+3)));
#endif
			indexBuf = (uint8_t *)indexBuf + BRCM_TPIT_ENTRY_SIZE;/* consume one index */
			rc = NEXUS_Recpump_IndexReadComplete(pContext->recpump, BRCM_TPIT_ENTRY_SIZE);
			BDBG_ASSERT(!rc);
		}

		/* record the segment TS stream; also insert PAT&PMT at start of each segments */
		if (datasize) {
			/* if dataSize + totalRecordBytes < bytesRecordedTillCurrentRai or no new index, write dataSize of current segment;
			    else, write up to bytesRecordedTillCurrentRai, then PAT/PMT for the next segment; */
			if((0==indexsize) || (datasize + totalRecordBytes < (size_t) bytesRecordedTillCurrentRai)) {
				write(pContext->fileTranscode, buffer, datasize);
				BDBG_MSG(("1) Wrote "BDBG_UINT64_FMT" data.", BDBG_UINT64_ARG((uint64_t)datasize)));
			} else {/* one or more indices */
				/* record rest of the previous segment before the new RAI packet and increment buffer pointer */
				write(pContext->fileTranscode, buffer, bytesRecordedTillCurrentRai - totalRecordBytes);
				buffer = (uint8_t*)buffer + bytesRecordedTillCurrentRai - totalRecordBytes;
				BDBG_MSG(("2) Wrote "BDBG_UINT64_FMT" data ("BDBG_UINT64_FMT", "BDBG_UINT64_FMT").",
					  BDBG_UINT64_ARG((uint64_t)(bytesRecordedTillCurrentRai - totalRecordBytes)),
					  BDBG_UINT64_ARG((uint64_t)(bytesRecordedTillCurrentRai)),
					  BDBG_UINT64_ARG((uint64_t)(totalRecordBytes))));

				if(firstTime) {/* first pat/pmt already written previously for the 1st segment */
					firstTime = false;
				} else {
					/* insert the PAT & PMT, then update continuity counter */
					write(pContext->fileTranscode, (uint8_t*)pContext->pat+1, 188);
					write(pContext->fileTranscode, (uint8_t*)pContext->pmt+1, 188);
					updateSystemData(pContext);
					BDBG_MSG(("wrote pat and pmt for segment %u", segment-1));
				}
				/* update the data read complete size */
				datasize = bytesRecordedTillCurrentRai - totalRecordBytes;
			}
			rc = NEXUS_Recpump_DataReadComplete(pContext->recpump, datasize);
			totalRecordBytes += datasize;
			BDBG_ASSERT(!rc);
		}
#define BTST_CHUNK_TIMEOUT_INTERVAL   250 /* 250ms */
		if (!datasize && !indexsize) {
			if(BKNI_WaitForEvent(pContext->recpumpEvent, BTST_CHUNK_TIMEOUT_INTERVAL) == BERR_TIMEOUT) {
				timeout+=BTST_CHUNK_TIMEOUT_INTERVAL;
			}
			continue;
		} else {
			timeout = 0;
		}
	}
	xcode_release_systemdata(pContext);

	printf("recpump thread is stopped!\n");
	if(g_eotEvent) {
		B_Event_Set(g_eotEvent);
	}
	g_keyReturn = 'q';
}

static void start_mux(
	TranscodeContext  *pContext )
{
	NEXUS_StreamMuxOutput muxOutput;
	NEXUS_RecpumpAddPidChannelSettings recpumpAddPidSettings;
    NEXUS_RecpumpTpitFilter filter;
    NEXUS_Error rc;

	NEXUS_StreamMux_Start(pContext->streamMux,&pContext->tsMuxConfig, &muxOutput);
	pContext->pidChannelTranscodeVideo = muxOutput.video[0];

	/* add multiplex data to the same record */
	/* configure the video pid for indexing */
	NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recpumpAddPidSettings);
	recpumpAddPidSettings.pidType = NEXUS_PidType_eVideo;
	recpumpAddPidSettings.pidTypeSettings.video.index = false;/* HLS NOTE: only use TPIT filter to index RAI */
	NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelTranscodeVideo, &recpumpAddPidSettings);

	if(pContext->inputSettings.bAudioInput)
	{
		pContext->pidChannelTranscodeAudio = muxOutput.audio[0];
		NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelTranscodeAudio, NULL);
	}
	NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelTranscodePcr, NULL);

	/* HLS NOTE: set up TPIT filter to grep RAI to index HLS segments (one RAI per segment */
	NEXUS_Recpump_GetDefaultTpitFilter(&filter);
	filter.config.mpeg.randomAccessIndicatorEnable = true;
	filter.config.mpeg.randomAccessIndicatorCompValue = true;
	rc = NEXUS_Recpump_SetTpitFilter(pContext->recpump, pContext->pidChannelTranscodeVideo, &filter);
	BDBG_ASSERT(!rc);

	NEXUS_Recpump_Start(pContext->recpump);

	/* Start record of stream mux output */
	/* HLS NOTE: since encoder GOP setting is infinite IPPP, the segment boundary I-frame is the only I of a segment;
	   or all the I-frames in TS output are at the starting of each segments! Here we reuse the nexus record module to
	   produce the TS record file while the index output file's I-frame index tells segment boundary. If the HLS transcode
	   output is directly used for HLS streaming, app may choose to use recpump and disables SCD index but enables
	   RAI TPIT filter to detect the TS output segment boundaries on the fly! See /nexus/examples/transport/tpit.c for
	   the setup of recpump TPIT filter. */
	pContext->fileTranscode = open(pContext->encodeSettings.fname, O_CREAT | O_TRUNC | O_RDWR | O_LARGEFILE, 0666);
	if (pContext->fileTranscode<0) { BERR_TRACE(BERR_NOT_SUPPORTED); BDBG_ASSERT(0); }
	rc = fcntl(pContext->fileTranscode, F_GETFL);
	if ((int)rc != -1) {
		rc = fcntl(pContext->fileTranscode, F_SETFL, rc | FD_CLOEXEC);
	}

	pContext->fileHls = fopen(pContext->hlsFname, "w");
	if (!pContext->fileHls) { BERR_TRACE(BERR_NOT_SUPPORTED); BDBG_ASSERT(0); }

	pContext->recpumpThread = B_Thread_Create("HLS recpump thread", (B_ThreadFunc)videoRecpump_thread, pContext, NULL);
}

static void finish_mux(
	TranscodeContext  *pContext )
{
	NEXUS_StreamMux_Finish(pContext->streamMux);
	if(BKNI_WaitForEvent(pContext->finishEvent, 3000)!=BERR_SUCCESS) {
		fprintf(stderr, "TIMEOUT\n");
	}
	NEXUS_Recpump_Stop(pContext->recpump);
	fclose(pContext->fileHls);
	close(pContext->fileTranscode);
	/*****************************************
	 * Note: remove all record PID channels before stream
	 * mux stop since streammux would close the A/V PID channels
	 */
	NEXUS_Recpump_RemoveAllPidChannels(pContext->recpump);
}

static void stop_mux(
	TranscodeContext  *pContext )
{
	NEXUS_StreamMux_Stop(pContext->streamMux);
	NEXUS_StreamMux_Destroy(pContext->streamMux);
	NEXUS_Recpump_Close(pContext->recpump);
	BKNI_DestroyEvent(pContext->recpumpEvent);
	B_Thread_Destroy(pContext->recpumpThread);
	NEXUS_Playpump_Close(pContext->playpumpTranscodePcr);
	NEXUS_Playpump_Close(pContext->playpumpTranscodeVideo);
	if(pContext->inputSettings.bAudioInput) NEXUS_Playpump_Close(pContext->playpumpTranscodeAudio);
}

static void b_print_media_string(const bmedia_probe_stream *stream)
{
    char stream_info[512];
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    printf( "\n\nMedia Probe:\n" "%s\n\n", stream_info);
}

static bool g_bProbe = false;
struct {
    NEXUS_VideoCodec nexus;
    bvideo_codec settop;
} g_videoCodec[] = {
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_none},
    {NEXUS_VideoCodec_eUnknown, bvideo_codec_unknown},
    {NEXUS_VideoCodec_eMpeg1, bvideo_codec_mpeg1},
    {NEXUS_VideoCodec_eMpeg2, bvideo_codec_mpeg2},
    {NEXUS_VideoCodec_eMpeg4Part2, bvideo_codec_mpeg4_part2},
    {NEXUS_VideoCodec_eH263, bvideo_codec_h263},
    {NEXUS_VideoCodec_eH264, bvideo_codec_h264},
    {NEXUS_VideoCodec_eH264_Svc, bvideo_codec_h264_svc},
    {NEXUS_VideoCodec_eH264_Mvc, bvideo_codec_h264_mvc},
    {NEXUS_VideoCodec_eVc1, bvideo_codec_vc1},
    {NEXUS_VideoCodec_eVc1SimpleMain, bvideo_codec_vc1_sm},
    {NEXUS_VideoCodec_eDivx311, bvideo_codec_divx_311},
    {NEXUS_VideoCodec_eRv40, bvideo_codec_rv40},
    {NEXUS_VideoCodec_eVp6, bvideo_codec_vp6},
    {NEXUS_VideoCodec_eVp8, bvideo_codec_vp8},
    {NEXUS_VideoCodec_eSpark, bvideo_codec_spark},
    {NEXUS_VideoCodec_eMotionJpeg, bvideo_codec_mjpeg},
    {NEXUS_VideoCodec_eAvs, bvideo_codec_avs}
};

struct {
    NEXUS_AudioCodec nexus;
    baudio_format settop;
} g_audioCodec[] = {
   {NEXUS_AudioCodec_eUnknown, baudio_format_unknown},
   {NEXUS_AudioCodec_eMpeg, baudio_format_mpeg},
   {NEXUS_AudioCodec_eMp3, baudio_format_mp3},
   {NEXUS_AudioCodec_eAac, baudio_format_aac},
   {NEXUS_AudioCodec_eAacPlus, baudio_format_aac_plus},
   {NEXUS_AudioCodec_eAacPlusAdts, baudio_format_aac_plus_adts},
   {NEXUS_AudioCodec_eAacPlusLoas, baudio_format_aac_plus_loas},
   {NEXUS_AudioCodec_eAc3, baudio_format_ac3},
   {NEXUS_AudioCodec_eAc3Plus, baudio_format_ac3_plus},
   {NEXUS_AudioCodec_eDts, baudio_format_dts},
   {NEXUS_AudioCodec_eLpcmHdDvd, baudio_format_lpcm_hddvd},
   {NEXUS_AudioCodec_eLpcmBluRay, baudio_format_lpcm_bluray},
   {NEXUS_AudioCodec_eDtsHd, baudio_format_dts_hd},
   {NEXUS_AudioCodec_eWmaStd, baudio_format_wma_std},
   {NEXUS_AudioCodec_eWmaPro, baudio_format_wma_pro},
   {NEXUS_AudioCodec_eLpcmDvd, baudio_format_lpcm_dvd},
   {NEXUS_AudioCodec_eAvs, baudio_format_avs},
   {NEXUS_AudioCodec_eAmr, baudio_format_amr},
   {NEXUS_AudioCodec_eDra, baudio_format_dra},
   {NEXUS_AudioCodec_eCook, baudio_format_cook},
   {NEXUS_AudioCodec_ePcmWav, baudio_format_pcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_adpcm},
   {NEXUS_AudioCodec_eAdpcm, baudio_format_dvi_adpcm},
   {NEXUS_AudioCodec_eVorbis, baudio_format_vorbis},
   {NEXUS_AudioCodec_eLpcm1394, baudio_format_lpcm_1394},
   {NEXUS_AudioCodec_eFlac, baudio_format_flac},
   {NEXUS_AudioCodec_eApe, baudio_format_ape},
   {NEXUS_AudioCodec_eDtsLegacy, baudio_format_dts_cd},
};

#define CONVERT(g_struct) \
    unsigned i; \
    for (i=0;i<sizeof(g_struct)/sizeof(g_struct[0]);i++) { \
        if (g_struct[i].settop == settop_value) { \
            return g_struct[i].nexus; \
        } \
    } \
    printf("unable to find value %d in %s\n", settop_value, #g_struct); \
    return g_struct[0].nexus

NEXUS_VideoCodec b_videocodec2nexus(bvideo_codec settop_value)
{
    CONVERT(g_videoCodec);
}

NEXUS_AudioCodec b_audiocodec2nexus(baudio_format settop_value)
{
    CONVERT(g_audioCodec);
}
int media_probe(InputSettings *opts, const char *filename)
{
    int rc = 0;

    /* use media probe to set values */
    bmedia_probe_t probe = NULL;
    bmedia_probe_config probe_config;
    const bmedia_probe_stream *stream = NULL;
    const bmedia_probe_track *track = NULL;
    bfile_io_read_t fd = NULL;
    bool foundAudio = false, foundVideo = false;
    FILE *fin;

    probe = bmedia_probe_create();

    fin = fopen64(filename,"rb");
    if (!fin) {
        printf("can't open media file '%s' for probing\n", filename);
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
        printf("media probe can't parse stream '%s'\n", filename);
        rc = -1;
        goto done;
    }

    b_print_media_string(stream);
    xcodeContext.duration = stream->duration;

    if (g_bProbe) {
        opts->eVideoCodec = NEXUS_VideoCodec_eUnknown;
        opts->eAudioCodec = NEXUS_AudioCodec_eUnknown;

        for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
            switch(track->type) {
                case bmedia_track_type_audio:
                    if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                        opts->iAudioPid = track->number;
                        opts->eAudioCodec = b_audiocodec2nexus(track->info.audio.codec);
                        foundAudio = true;
                    }
                    break;
                case bmedia_track_type_video:
                    if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                        opts->iVideoPid = track->number;
                        opts->eVideoCodec = b_videocodec2nexus(track->info.video.codec);
                        foundVideo = true;
                    }
                    break;
                case bmedia_track_type_pcr:
                    opts->iPcrPid = track->number;
                    break;
                default:
                    break;
            }
        }
    }

done:
    if (probe) {
        if (stream) {
            bmedia_probe_stream_free(probe, stream);
        }
        bmedia_probe_destroy(probe);
    }

    return rc;
}

static struct timeval initTimeStamp;
static void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
    struct timeval currentTime;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;

    rc = gettimeofday(&currentTime, NULL);
    if (rc!=0) {
        BDBG_P_PrintString("### debug: gettimeofday returned %d, ignored", rc);
    }

    if (currentTime.tv_usec < initTimeStamp.tv_usec)
    {
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec + 1000000)/1000;
        currentTime.tv_sec--;
    }
    else    {
        milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec)/1000;
    }

    /* Calculate the time   */
    hours = (currentTime.tv_sec - initTimeStamp.tv_sec)/3600;
    minutes = (((currentTime.tv_sec - initTimeStamp.tv_sec)/60))%60;
    seconds = (currentTime.tv_sec - initTimeStamp.tv_sec)%60;

    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;

}

static void monitor_thread( void *pContext )
{
	unsigned hrs, mins, secs, duration;
	char time[80];
	NEXUS_StreamMuxStatus tsMuxStatus;
	NEXUS_StreamMuxHandle tsMux = (NEXUS_StreamMuxHandle)pContext;

	fprintf(stderr, "\n\n");
	do{
		BKNI_Sleep(1000);
		NEXUS_StreamMux_GetStatus(tsMux, &tsMuxStatus);
		duration = tsMuxStatus.duration;
		secs = duration/1000;
		mins = secs/60;
		hrs  = mins/60;
		mins = mins%60;
		secs = secs%60;
		BDBG_P_GetTimeStamp(time, 80);
		if(duration) {
			fprintf(stderr, " Elapsed: %s; Mux completed: %02u:%02u:%02u.%03u; %.1f percents\n", time,
			hrs, mins, secs, duration%1000, (float)100*duration/xcodeContext.duration);
		}
	} while (!xcodeContext.bShutdown);
	fprintf(stderr, "\n\n");

	BKNI_SetEvent(xcodeContext.monitorDone);
}

static void bringup_transcode(
	TranscodeContext  *pContext )
{
	NEXUS_Error rc;
	BNAV_Player_Settings settings;
	char srcIndexFileName[256] = {'\0',};

	xcode_index_filename(srcIndexFileName, pContext->inputSettings.fname, false);
	pContext->indexFile = fopen(srcIndexFileName, "r");
	if (!pContext->indexFile) {
		fprintf(stderr, "Source file %s has no index.\n", pContext->inputSettings.fname);
		BDBG_ASSERT(0);
	}

	if (media_probe(&pContext->inputSettings, pContext->inputSettings.fname)) {
		BERR_TRACE(NEXUS_UNKNOWN);
		return;
	}

	BNAV_Player_GetDefaultSettings(&settings);
	settings.videoPid = pContext->inputSettings.iVideoPid;
	settings.readCb = (BP_READ_CB)fread;
	settings.tellCb = (BP_TELL_CB)ftell;
	settings.seekCb = (BP_SEEK_CB)fseek;
	settings.filePointer = pContext->indexFile;
	rc = BNAV_Player_Open(&pContext->bcmplayer, &settings);
	BDBG_ASSERT(!rc);

	gettimeofday(&initTimeStamp,NULL);

	BFIFO_INIT(&pContext->durationFifo, pContext->segmentDurations, HLS_DURATION_FIFO_SIZE);
	/* open the transcode context */
	open_video_transcode(pContext);

	/************************************************
	 * Set up video xcoder mux and record
	 */
	open_tsmux(pContext);

	if(pContext->inputSettings.bAudioInput) {/* open audio xcoders */
		open_audio_transcode(pContext);
		xcode_audio_tsmux_record(pContext);
	}
	xcode_av_sync(pContext, &pContext->encodeSettings.videoEncoderConfig, &pContext->vidEncoderStartSettings);

	/**************************************************
	 * NOTE: transcoder record/mux start
	 */
	/* start mux */
	start_mux(pContext);

	/* start monitor thread for progress */
	BKNI_CreateEvent(&pContext->monitorDone);
	pContext->monitorThread = B_Thread_Create("HLS file transcode monitor thread", (B_ThreadFunc)monitor_thread, (void*)pContext->streamMux, NULL);

	/*******************************
	 * START transcoder
	 */
	start_video_transcode(pContext);
	if(pContext->inputSettings.bAudioInput) {/* start audio xcoders */
		start_audio_transcode(pContext);
	}
#if BTST_ENABLE_XCODE_LOOPBACK
	xcode_loopback_setup(pContext);
#endif
}

static void shutdown_transcode(
	TranscodeContext  *pContext )
{
	unsigned i=0;
	if(pContext->inputSettings.bAudioInput)
	{
		NEXUS_AudioDecoderStatus status;
		for(i=0; i<2; ) {
			NEXUS_AudioDecoder_GetStatus(pContext->audioDecoder, &status);
			BDBG_MSG(("audio decoder0 has %u queued frames...", status.queuedFrames));
			if(status.queuedFrames < 3) {
				i++;
			}
			BKNI_Sleep(1000);
		}
	}
	if(pContext->inputSettings.bAudioInput) {/* start audio xcoders */
		stop_audio_transcode(pContext);
	}
#if BTST_DISABLE_AUTO_STOP
	stop_video_transcode(pContext);
#endif
	finish_mux(pContext);

	pContext->bShutdown = true;

	while(BKNI_WaitForEvent(pContext->monitorDone,BKNI_INFINITE) == BERR_TIMEOUT);
	BKNI_DestroyEvent(pContext->monitorDone);
	B_Thread_Destroy(pContext->monitorThread);
	BKNI_Sleep(1000);/* allow recpump thread to complete */

	/* stop mux */
	BDBG_MSG(("stopping mux..."));
	stop_mux(pContext);
	BKNI_DestroyEvent(pContext->finishEvent);

#if !BTST_DISABLE_AUTO_STOP
	stop_video_transcode(pContext);
#endif

#if BTST_ENABLE_AV_SYNC
	/* disconnect sync channel after decoders stop */
	NEXUS_SyncChannel_Destroy(pContext->syncChannel);
#endif

	if(pContext->inputSettings.bAudioInput) {
		close_audio_transcode(pContext);
	}
	/* close the transcode context */
	close_video_transcode(pContext);

	BNAV_Player_Close(pContext->bcmplayer);
	fclose(pContext->indexFile);
#if BTST_ENABLE_XCODE_LOOPBACK
	xcode_loopback_shutdown(pContext);
#endif
}
#endif

static void config_xcoder_context (
	TranscodeContext *pContext )
{
	int choice;
	char input[80];
	pContext->inputSettings.bConfig = true;

	printf("\n source stream file: ");
	scanf("%s", pContext->inputSettings.fname);
	printf("\n source stream type:  (%d) Es (%d) Ts           ",
		NEXUS_TransportType_eEs, NEXUS_TransportType_eTs);	   scanf("%d", (int32_t*)&pContext->inputSettings.eStreamType);
	pContext->inputSettings.eStreamType = NEXUS_TransportType_eTs;
	printf("\n source stream codec: (%d) Mpeg2 (%d) H264      ",
		NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264);
	scanf("%d", (int32_t*)&pContext->inputSettings.eVideoCodec);
	printf("\n Video pid:                                           ");	   scanf("%s", input); pContext->inputSettings.iVideoPid = strtoul(input, NULL, 0);
	printf("\n Pcr   pid:                                           ");	   scanf("%s", input); pContext->inputSettings.iPcrPid = strtoul(input, NULL, 0);
	printf("\n Input Audio Parameters");
	printf("\n Enable Audio channel: (0) No (1) Yes             ");	   scanf("%d", &choice);
	pContext->inputSettings.bAudioInput = (choice!=0);
	if(pContext->inputSettings.bAudioInput)
	{
		printf("\n Audio Pid:                                           "); scanf("%s", input); pContext->inputSettings.iAudioPid = strtoul(input, NULL, 0);

		printf("\n Audio Codec:\n"
			" (%d)Mpg\n"
			" (%d)AAC\n"
			" (%d)AAC+Loas\n"
			" (%d)AAC+Adts\n"
			" (%d)AC3\n"
			" (%d)AC3+\n",
			NEXUS_AudioCodec_eMpeg,
			NEXUS_AudioCodec_eAac,
			NEXUS_AudioCodec_eAacPlusLoas,
			NEXUS_AudioCodec_eAacPlusAdts,
			NEXUS_AudioCodec_eAc3,
			NEXUS_AudioCodec_eAc3Plus);
		scanf("%d", (int32_t*)&pContext->inputSettings.eAudioCodec);
		pContext->encodeSettings.encoderAudioCodec = pContext->inputSettings.eAudioCodec;
	}

	printf("\n****************************************************************\n");
	printf("\n Input Parameters\n");
	printf("\n filename:       %s", pContext->inputSettings.fname);

	if(pContext->inputSettings.bAudioInput)
	{
		printf("\n Enable Audio channel Pid: %#x\n audio codec: %d           ", pContext->inputSettings.iAudioPid, pContext->inputSettings.eAudioCodec);
	}
	printf("\n Config %d", pContext->inputSettings.bConfig);
	printf("\n****************************************************************\n");

	printf("\n\n Encode Settings");
	printf("\n encode stream:                                                                        "); scanf("%s", pContext->encodeSettings.fname);
	printf("\n custom format: (0) No (1) Yes                                                         "); scanf("%d", &choice);
	pContext->encodeSettings.bCustom = choice;
	if(!pContext->encodeSettings.bCustom)
	{
		printf("\n enocder displayFormat:\n"
			" (%2d) 480i\n"
			" (%2d) 576i\n"
			" (%2d) 480p\n"
			" (%2d) 576p\n"
			" (%2d) 1080i60\n"
			" (%2d) 1080i50\n"
			" (%2d) 1080p24\n"
			" (%2d) 1080p25\n"
			" (%2d) 1080p30\n"
			" (%2d) 1080p50\n"
			" (%2d) 1080p60\n"
			" (%2d) 720p60\n"
			" (%2d) 720p50\n"
			" (%2d) 720p24\n"
			" (%2d) 720p25\n"
			" (%2d) 720p30Hz:\n",
			(NEXUS_VideoFormat_eNtsc),
			(NEXUS_VideoFormat_ePal),
			(NEXUS_VideoFormat_e480p),
			(NEXUS_VideoFormat_e576p),
			(NEXUS_VideoFormat_e1080i),
			(NEXUS_VideoFormat_e1080i50hz),
			(NEXUS_VideoFormat_e1080p24hz),
			(NEXUS_VideoFormat_e1080p25hz),
			(NEXUS_VideoFormat_e1080p30hz),
			(NEXUS_VideoFormat_e1080p50hz),
			(NEXUS_VideoFormat_e1080p60hz),
			(NEXUS_VideoFormat_e720p),
			(NEXUS_VideoFormat_e720p50hz),
			(NEXUS_VideoFormat_e720p24hz),
			(NEXUS_VideoFormat_e720p25hz),
			(NEXUS_VideoFormat_e720p30hz));
		scanf("%d", (int32_t*)&pContext->encodeSettings.displayFormat);
	}
	else
	{
		pContext->encodeSettings.displayFormat = NEXUS_VideoFormat_eCustom2;
		printf("\n Resolution width & height:                                                        ");
		scanf("%d %d", (int32_t*)&pContext->encodeSettings.customFormatSettings.width, (int32_t*)&pContext->encodeSettings.customFormatSettings.height);
		printf("\n Interlaced? (0) No (1) Yes	");
		scanf("%d", &choice); pContext->encodeSettings.customFormatSettings.interlaced = choice;
		printf("\n Vertical refresh rate (in 1/1000th Hz):                                           "); scanf("%u", (unsigned*)&pContext->encodeSettings.customFormatSettings.refreshRate);
		printf("\n Aspect Ratio:\n"
			" (%d) Auto\n"
			" (%d) 4x3\n"
			" (%d) 16x9\n"
			" (%d) SAR\n",
			NEXUS_DisplayAspectRatio_eAuto,
			NEXUS_DisplayAspectRatio_e4x3,
			NEXUS_DisplayAspectRatio_e16x9,
			NEXUS_DisplayAspectRatio_eSar);
		scanf("%d", (int32_t*)&pContext->encodeSettings.customFormatSettings.aspectRatio);
		if(NEXUS_DisplayAspectRatio_eSar == pContext->encodeSettings.customFormatSettings.aspectRatio)
		{
			printf("Please enter Sample Aspect Ratio X and Y: \n");
			scanf("%u %u", &pContext->encodeSettings.customFormatSettings.sampleAspectRatio.x,
				&pContext->encodeSettings.customFormatSettings.sampleAspectRatio.y);
		}
	}

	printf("\n Frame rate:\n"
		" (%d) 23.976\n"
		" (%d) 24\n"
		" (%d) 25\n"
		" (%d) 29.97Hz\n"
		" (%d) 30 Hz\n"
		" (%d) 50 Hz\n"
		" (%d) 59.94\n"
		" (%d) 60\n"
		" (%d) 14.985\n",
		NEXUS_VideoFrameRate_e23_976,
		NEXUS_VideoFrameRate_e24,
		NEXUS_VideoFrameRate_e25,
		NEXUS_VideoFrameRate_e29_97,
		NEXUS_VideoFrameRate_e30,
		NEXUS_VideoFrameRate_e50,
		NEXUS_VideoFrameRate_e59_94,
		NEXUS_VideoFrameRate_e60,
		NEXUS_VideoFrameRate_e14_985);
	scanf("%d", (int*)&pContext->encodeSettings.encoderFrameRate);
	printf("\n Bitrate (bps):                                                                       "); scanf("%d", (int*)&pContext->encodeSettings.encoderBitrate);
	printf("\n # of P frames per GOP:                                                       Infinity"); pContext->encodeSettings.encoderGopStructureFramesP = -1;
	printf("\n # of B frames between reference I or P frames:                                      0"); pContext->encodeSettings.encoderGopStructureFramesB = 0;
	printf("\n Encode Video Codec: (%d) MPEG2 (%d) MPEG4 Part2 (%d) H264                            ",
		NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eMpeg4Part2, NEXUS_VideoCodec_eH264);
	scanf("%d", (int*)&pContext->encodeSettings.encoderVideoCodec);
	printf("\n Profile: (%d) Simple (%d) AdvancedSimple (%d) Main (%d) High (%d) Baseline                               ",
		NEXUS_VideoCodecProfile_eSimple, NEXUS_VideoCodecProfile_eAdvancedSimple, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecProfile_eHigh, NEXUS_VideoCodecProfile_eBaseline);
	scanf("%d", (int*)&pContext->encodeSettings.encoderProfile);
	printf("\n Level:\n"
		" (%2d) 1.0\n"
		" (%2d) 2.0\n"
		" (%2d) 2.1\n"
		" (%2d) 2.2\n"
		" (%2d) 3.0\n"
		" (%2d) 3.1\n"
		" (%2d) 3.2\n"
		" (%2d) 4.0\n"
		" (%2d) 4.1\n"
		" (%2d) 4.2\n"
		" (%2d) 5.0\n"
		" (%2d) Low\n"
		" (%2d) Main\n"
		" (%2d) High\n",
		NEXUS_VideoCodecLevel_e10, NEXUS_VideoCodecLevel_e20, NEXUS_VideoCodecLevel_e21, NEXUS_VideoCodecLevel_e22, NEXUS_VideoCodecLevel_e30,
		NEXUS_VideoCodecLevel_e31, NEXUS_VideoCodecLevel_e32, NEXUS_VideoCodecLevel_e40, NEXUS_VideoCodecLevel_e41,
		NEXUS_VideoCodecLevel_e42, NEXUS_VideoCodecLevel_e50, NEXUS_VideoCodecLevel_eLow, NEXUS_VideoCodecLevel_eMain, NEXUS_VideoCodecLevel_eHigh);
	scanf("%d", (int*)&pContext->encodeSettings.encoderLevel);

	if(pContext->inputSettings.bAudioInput)
	{
		printf("\n Enable Audio transcode: (0) No (1) Yes                                                       "); scanf("%d", &choice);
		pContext->encodeSettings.bAudioEncode = choice;

		if(pContext->encodeSettings.bAudioEncode)
		{
			printf("\n Encode Audio Codec: (%d)Mpg (%d)Mp3 (%d)AAC (%d)AAC+ (%d)AAC+Loas (%d)AAC+Adts (%d)AC3                              ",
				NEXUS_AudioCodec_eMpeg,
				NEXUS_AudioCodec_eMp3,
				NEXUS_AudioCodec_eAac,
				NEXUS_AudioCodec_eAacPlus,
				NEXUS_AudioCodec_eAacPlusLoas,
				NEXUS_AudioCodec_eAacPlusAdts,
				NEXUS_AudioCodec_eAc3);
			scanf("%d", (int32_t*)&pContext->encodeSettings.encoderAudioCodec);
		}
	}

	printf("\n****************************************************************\n");
	printf("\n Encode Parameters\n");
	printf("\n filename:  %s", pContext->encodeSettings.fname);
	printf("\n display format:  %d", pContext->encodeSettings.displayFormat);
	printf("\n Frame Rate: %d\n Bit Rate: %d", pContext->encodeSettings.encoderFrameRate, pContext->encodeSettings.encoderBitrate);
	printf("\n P frames: %d;     B frames: %d", pContext->encodeSettings.encoderGopStructureFramesP, pContext->encodeSettings.encoderGopStructureFramesB);
	printf("\n Video Codec: %4d;  Profile: %4d;  Level: %4d", pContext->encodeSettings.encoderVideoCodec, pContext->encodeSettings.encoderProfile,pContext->encodeSettings.encoderLevel );
	if(pContext->encodeSettings.bAudioEncode)
		printf("\n Audio Codec: %4d", pContext->encodeSettings.encoderAudioCodec);
	printf("\n****************************************************************\n");
}

void usage(void) {
			printf("\ntranscode_ts usage:\n");
			printf("  Without options, it transcodes default stream file /data/videos/avatar_AVC_15M.ts into TS file: /data/videos/stream.ts\n");
			printf("\nOptions:\n");
			printf("  -h        - to print the usage info\n");
			printf("  -cfg      - to set the test configuration\n");
			printf("  -chunktime MSEC - to set video chunk duration in miliseconds.\n");
			printf("  -probe      - to probe the source PIDs and codecs.\n");
			printf("  -bitrate N  - to specify video encode bitrate in bps.\n");
			printf("  -audio      - to enable audio transcode.\n");
			printf("  -RT         - to enable Real Time mode transcode.\n");
}

int main(int argc, char **argv)  {

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_STREAM_MUX

	NEXUS_PlatformSettings platformSettings;
	TranscodeContext *pContext = &xcodeContext;
	int i;
	bool bAudio = false;

	BSTD_UNUSED(argv);
	BKNI_Memset(pContext, 0, sizeof(xcodeContext));
	if ( argc >= 2) {
		for(i=1; i<argc; i++) {
			if(!strcmp("-h",argv[i])) {
				usage();
				return 0;
			}
			if(!strcmp("-cfg",argv[i])) {
				pContext->inputSettings.bConfig = true;
				fprintf(stderr, "Manually configure settings...\n");
			}
			if(!strcmp("-autoquit",argv[i])) {
				g_bAutoQuit= true;
				fprintf(stderr, "Enabled auto quit...\n");
			}
			if(!strcmp("-chunktime",argv[i])) {
				g_chunkDuration = strtoul(argv[++i], NULL, 0);
				fprintf(stderr, "Video chunk duration = %u ms\n", g_chunkDuration);
			}
			if(!strcmp("-bitrate",argv[i])) {
			    g_bitrate = strtoul(argv[++i], NULL, 0);
			    fprintf(stderr, "Video encode bitrate = %u bps.\n", g_bitrate);
			}
			if(!strcmp("-audio",argv[i])) {
			    bAudio = true;
			    fprintf(stderr, "Audio is enabled.\n");
			}
			if(!strcmp("-RT",argv[i])) {
			    g_bNrt = false;
			    fprintf(stderr, "RT mode.\n");
			}
		}
	}

	/* Turn off buffering for stdin */
	setvbuf(stdin, NULL, _IONBF, 0);

	if(pContext->inputSettings.bConfig) {
		config_xcoder_context(pContext);/* transcoder 0 context user settings */
	}
	else { /* default test config for auto test */
			usage();
			printf("\nYou're testing with the default configuration:\n");
			/* Input setting */
			BKNI_Snprintf(pContext->inputSettings.fname, 256, "/data/videos/avatar_AVC_15M.ts");
			pContext->inputSettings.eStreamType=NEXUS_TransportType_eTs;
			pContext->inputSettings.eVideoCodec=NEXUS_VideoCodec_eH264;
			pContext->inputSettings.eAudioCodec=NEXUS_AudioCodec_eAc3;
			pContext->inputSettings.iVideoPid=0x101;
			pContext->inputSettings.iAudioPid=0x104;
			pContext->inputSettings.iPcrPid=0x101;
			pContext->inputSettings.bAudioInput=bAudio;
			pContext->inputSettings.bConfig = false;
			pContext->inputSettings.eMultiChanFmt = NEXUS_AudioMultichannelFormat_eStereo;
			printf("\n****************************************************************\n");
			printf("\n Input Parameters\n");
			printf("\n filename:       %s", pContext->inputSettings.fname);

			{
				printf("\n Streamtype:  %d \n VideoCodec:  %d", pContext->inputSettings.eStreamType, pContext->inputSettings.eVideoCodec);
				printf("\n vPid:  %4d;  PcrPid:   %4d", pContext->inputSettings.iVideoPid, pContext->inputSettings.iPcrPid);
			}
			if(pContext->inputSettings.bAudioInput)
			{
				printf("\n Enable Audio channel Pid: %d\n audio codec: %d           ", pContext->inputSettings.iAudioPid, pContext->inputSettings.eAudioCodec);
			}
			printf("\n Config %1d", pContext->inputSettings.bConfig);
			printf("\n****************************************************************\n");


			/*Encode settings */
			BKNI_Snprintf(pContext->encodeSettings.fname, 256, "/data/videos/stream.ts");
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
			pContext->encodeSettings.displayFormat=NEXUS_VideoFormat_e480p;
#else
			pContext->encodeSettings.displayFormat=NEXUS_VideoFormat_e720p24hz;
#endif
			pContext->encodeSettings.encoderFrameRate=NEXUS_VideoFrameRate_e23_976;
			pContext->encodeSettings.encoderGopStructureFramesP=-1;/* HLS NOTE: infinite IPPP gop and let chunk boundary to close GOP for RAI tpit indexing */
			pContext->encodeSettings.encoderGopStructureFramesB=0;
			pContext->encodeSettings.encoderVideoCodec=NEXUS_VideoCodec_eH264;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
			pContext->encodeSettings.encoderProfile=NEXUS_VideoCodecProfile_eBaseline;
			pContext->encodeSettings.encoderLevel=NEXUS_VideoCodecLevel_e30;
			pContext->encodeSettings.encoderBitrate=400000;
#else
			pContext->encodeSettings.encoderProfile=NEXUS_VideoCodecProfile_eHigh;
			pContext->encodeSettings.encoderLevel=NEXUS_VideoCodecLevel_e31;
			pContext->encodeSettings.encoderBitrate=g_bitrate;
#endif
			pContext->encodeSettings.bCustom=false;
			pContext->encodeSettings.bAudioEncode=true;
			pContext->encodeSettings.encoderAudioCodec=NEXUS_AudioCodec_eAac;

			printf("\n****************************************************************\n");
			printf("\n Encode Parameters\n");
			printf("\n filename:  %s", pContext->encodeSettings.fname);
			printf("\n display format:  %d", pContext->encodeSettings.displayFormat);
			printf("\n Frame Rate: %d\n Bit Rate: %d", pContext->encodeSettings.encoderFrameRate, pContext->encodeSettings.encoderBitrate);
			printf("\n P frames: %d;     B frames: %d", pContext->encodeSettings.encoderGopStructureFramesP, pContext->encodeSettings.encoderGopStructureFramesB);
			printf("\n Video Codec: %4d;  Profile: %4d;  Level: %4d", pContext->encodeSettings.encoderVideoCodec, pContext->encodeSettings.encoderProfile,pContext->encodeSettings.encoderLevel );
			if(pContext->encodeSettings.bAudioEncode)
				printf("\n Audio Codec: %4d", pContext->encodeSettings.encoderAudioCodec);
			printf("\n****************************************************************\n");
	}

	/* init platform */
	B_Os_Init();
	NEXUS_Platform_GetDefaultSettings(&platformSettings);
	/* enable frontend if the 1st xcoder needs it for now; not always enable to avoid slow frontend init for other cases;
	   TODO: init frontend in case 1st disable but 2nd enables frontend. */
	platformSettings.openFrontend = false;
	NEXUS_Platform_Init(&platformSettings);
	NEXUS_Platform_GetConfiguration(&g_platformConfig);

	pContext->mutexChunk         = B_Mutex_Create(NULL);

	/* bringup the transcode context */
	pContext->mutexStarted = B_Mutex_Create(NULL);
	bringup_transcode(pContext);

	/****************************************************
	 *                       key handler                                                 *
	 *****************************************************/
	/* wait for 'q' to exit the test */
	{
		B_ThreadSettings threadSettings;
		B_ThreadHandle keyHandle;

		g_eotEvent = B_Event_Create(NULL);
		B_Event_Reset(g_eotEvent);
		B_Thread_GetDefaultSettings(&threadSettings);
		keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, &xcodeContext, &threadSettings);
		while(B_Event_Wait(g_eotEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
		B_Thread_Destroy(keyHandle);
		B_Event_Destroy(g_eotEvent);
		g_eotEvent = NULL;
	}

	shutdown_transcode(pContext);
	B_Mutex_Destroy(pContext->mutexStarted);
	pContext->mutexStarted = NULL;

	B_Mutex_Destroy(pContext->mutexChunk);

	/* uninit platform */
	NEXUS_Platform_Uninit();
	B_Os_Uninit();
#endif
	return 0;
}

#endif
/* End of file */
