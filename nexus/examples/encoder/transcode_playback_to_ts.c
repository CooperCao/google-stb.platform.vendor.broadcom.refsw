/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_adj.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_stream_mux.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include <assert.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#define BTST_ENABLE_USER_DATA_ENCODE     0
#define BTST_HAS_VIDEO_ENCODE_TEST       1
#define NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST  1
#define BTST_ENABLE_AUDIO_ENCODE         1

/* Choose one of the following for input: */
#define BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU     0
#define BTST_AC3_TRANSCODER_PASSTHRU    1
#define BTST_MPG_TRANSCODER_PASSTHRU    0
#define BTST_AAC_TRANSCODER_PASSTHRU    0

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */
  #define FILE_NAME "videos/avatar_AVC_15M.ts"/*  "videos/WildChina_Short.ts"*/
  #define VIDEO_PID 0x101
  #define VIDEO_CODEC NEXUS_VideoCodec_eH264
  #define AUDIO_PID 0x104
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#elif BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
  #define FILE_NAME "videos/TS.111.GenAcceptTest.Teletext.Subtitles.mpg"
  #define VIDEO_PID 0x8fd
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_PID 0x907
  #define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPEG */
#if BTST_ENABLE_USER_DATA_ENCODE
  #define FILE_NAME "videos/scte20only.ts"
  #define VIDEO_PID 0x10
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_PID 0x11
  #define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#else
  #define FILE_NAME "videos/cnnticker.mpg"
  #define VIDEO_PID 0x21
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_PID 0x22
  #define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#endif
#elif BTST_AAC_TRANSCODER_PASSTHRU /* AAC */
  #define FILE_NAME "videos/testjun26_188.mpg"
  #define VIDEO_PID 0x100
  #define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
  #define AUDIO_PID 0x110
  #define AUDIO_CODEC NEXUS_AudioCodec_eAac
#else
#error Please choose one of the options.
#endif


#define MAX_DSP_ENCODER_WIDTH   416
#define MAX_DSP_ENCODER_HEIGHT  224

BDBG_MODULE(transcode_playback_to_ts);


/* ============= utilities ==============*/

/* Generate a CRC for the specified data/length */
/* Initialize crc to 0 for new calculation.  Use an old result for subsequent calls. */
static uint32_t CRC32_mpeg(uint32_t crc, uint8_t *data, int length)
{
    int j;
    crc = ~crc;
    while (length--)
    {
        for (j=0; j<8; j++)
            crc = (crc<<1) ^ ((((*data >> (7-j)) ^ (crc >> 31)) & 1) ? 0x04c11db7 : 0);
        data++;
    }
    return crc;
}

#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
static void message_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BDBG_ERR(("message buffer %d overflows!", param));;
}
#endif

static void transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}

struct cmdSettings {
    NEXUS_VideoFormat displayFormat;
    NEXUS_VideoFrameRate encoderFrameRate;
    unsigned encoderBitrate;
    unsigned encoderGopStructureFramesP;
    unsigned encoderGopStructureFramesB;
    NEXUS_VideoCodec encoderVideoCodec;
    NEXUS_VideoCodecProfile encoderProfile;
    NEXUS_VideoCodecLevel encoderLevel;
} cmdSettings[] = {
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    {NEXUS_VideoFormat_eNtsc, NEXUS_VideoFrameRate_e29_97, 400*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e30},
#elif BTST_ENABLE_USER_DATA_ENCODE
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e60, 2*1000*1000, 29, 0, NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecLevel_eHigh},
#elif BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e60, 10*1000*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31},
#else
    {NEXUS_VideoFormat_e720p, NEXUS_VideoFrameRate_e23_976, 6*1000*1000, 23, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31},
    {NEXUS_VideoFormat_e1080i, NEXUS_VideoFrameRate_e29_97, 10*1000*1000, 23, 0, NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecLevel_eHigh},
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e29_97, 2*1000*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31}
#endif
};

#define TEST_ITERATIONS (sizeof(cmdSettings)/sizeof(cmdSettings[0]))

struct streamSettings {
    const char *fname;
    uint16_t   videoPid;
    NEXUS_VideoCodec videoCodec;
    uint16_t   audioPid;
    NEXUS_AudioCodec audioCodec;
} streamSettings = { FILE_NAME, VIDEO_PID, VIDEO_CODEC, AUDIO_PID, AUDIO_CODEC };

void print_usage(void)
{
    printf("Usage:\n"
        "  -h|--help   - this usage information\n"
        "  -sd         - enable SD display to show input video\n");
}

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindowMadSettings windowMadSettings;
#endif
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindowScalerSettings sclSettings;
#endif
    NEXUS_VideoWindowSettings windowSettings;
#endif
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMixerHandle audioMixer;
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoderHandle audioDecoder;
#else
    NEXUS_AudioDecoderHandle audioPassthrough;
#endif
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio;
    NEXUS_PidChannelHandle pidChannelTranscodeAudio;
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioEncoderSettings encoderSettings;
    NEXUS_AudioEncoderHandle audioEncoder;
#endif
    NEXUS_AudioCodec audioCodec;
#endif
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PidChannelHandle pidChannelTranscodeVideo;
    NEXUS_RecordPidChannelSettings recordPidSettings;
#endif
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_StreamMuxOutput muxOutput;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_PlaypumpHandle playpumpTranscodePcr;
    NEXUS_FileRecordHandle fileTranscode;
    NEXUS_StcChannelHandle stcChannelTranscode;
#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
    NEXUS_MessageSettings messageSettings;
    NEXUS_MessageStartSettings messageStartSettings;
    NEXUS_PidChannelHandle pidChannelUserData[2], pidChannelTranscodeUserData[2];
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#endif
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordSettings;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodePat;
    NEXUS_PidChannelHandle pidChannelTranscodePmt;
    void *pat;
    void *pmt;

    int i = 0, j = 0;
    int iteration = 1;
    char key;
    bool enableSdDisplay = false;

    for(j=0; j<argc; j++) {
        if(!strcmp(argv[j], "-h") || !strcmp(argv[j], "--help")) {
            print_usage();
            return 0;
        }
        if(!strcmp(argv[j], "-sd")) {
            enableSdDisplay = true;
            fprintf(stderr, "Enabled SD debug display.\n");
        }
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

again:
    BDBG_WRN(("Setting up transcode pipeline: format %d, fr %d, bitrate %u, gopP %d, codec %d",
                cmdSettings[i].displayFormat,
                cmdSettings[i].encoderFrameRate,
                cmdSettings[i].encoderBitrate,
                cmdSettings[i].encoderGopStructureFramesP,
                cmdSettings[i].encoderVideoCodec));
    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(streamSettings.fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", streamSettings.fname);
        return -1;
    }

#if NEXUS_HAS_SYNC_CHANNEL
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif


    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* encoders/mux require different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelTranscode = NEXUS_StcChannel_Open(1, &stcSettings);


    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);


    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e480p;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* app assumes that deinterlacer for main window is enabled by default */

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    /* Open the audio decoder */
#if BTST_ENABLE_AUDIO_ENCODE
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#else
    audioPassthrough = NEXUS_AudioDecoder_Open(1, NULL);
#endif

    /* Open the audio and pcr pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
#if BTST_ENABLE_AUDIO_ENCODE
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
#endif
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, streamSettings.audioPid, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = streamSettings.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#else
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
#endif

    /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
    NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
    audioMixerSettings.mixUsingDsp = true;
    audioMixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
    assert(audioMixer);

    /* Open audio mux output */
    audioMuxOutput = NEXUS_AudioMuxOutput_Create(NULL);
    assert(audioMuxOutput);
#if BTST_ENABLE_AUDIO_ENCODE
    /* Open audio encoder */
    NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_AudioCodec_eAac;
    audioCodec = encoderSettings.codec;
    audioEncoder = NEXUS_AudioEncoder_Open(&encoderSettings);
    assert(audioEncoder);

    /* Connect decoder to mixer and set as master */
    NEXUS_AudioMixer_AddInput(audioMixer,
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(audioMixer, &audioMixerSettings);
    /* Connect mixer to encoder */
    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_AudioMixer_GetConnector(audioMixer));
    /* Connect mux to encoder */
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
#else
    NEXUS_AudioMixer_AddInput(audioMixer,
        NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
    audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed);
    NEXUS_AudioMixer_SetSettings(audioMixer, &audioMixerSettings);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput),
        NEXUS_AudioMixer_GetConnector(audioMixer));

    audioCodec = audioProgram.codec;
#endif
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
        NEXUS_AudioMixer_GetConnector(audioMixer));
#endif

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
#if BTST_HAS_VIDEO_ENCODE_TEST
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);
#endif
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    displaySettings.format = NEXUS_VideoFormat_e480p; /* could be custom2 format size too (MUST be progressive though) */
    /* STG display has designated encoder timing generator (STG) */
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
#endif
#else
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = cmdSettings[i].displayFormat;/* source is 60hz */
    displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */
    displaySettings.format = cmdSettings[i].displayFormat;
#endif
    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);/* cmp3 for transcoder */
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    windowSettings.position.width = MAX_DSP_ENCODER_WIDTH;
    windowSettings.position.height = MAX_DSP_ENCODER_HEIGHT;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false; /* STG based DSP encoder should not hide the video window */

    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
#endif
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
#endif

#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true; /* actually reverse 22 pulldown */
    windowMadSettings.enable32Pulldown = true; /* actually reverse 32 pulldown */
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &windowMadSettings);
#endif

    /* connect same decoder to the encoder display;
     * NOTE: simul display + transcode mode might have limitation in audio pathre;
     * here is for video transcode bringup purpose;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if(enableSdDisplay) {
        NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = streamSettings.videoCodec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, streamSettings.videoPid, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = streamSettings.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = true; /* encoder can detect film content and follow CET */
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
    videoEncoderConfig.frameRate = cmdSettings[i].encoderFrameRate;
    videoEncoderConfig.bitrateMax = cmdSettings[i].encoderBitrate;
    videoEncoderConfig.streamStructure.framesP = cmdSettings[i].encoderGopStructureFramesP;
    videoEncoderConfig.streamStructure.framesB = cmdSettings[i].encoderGopStructureFramesB;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = cmdSettings[i].encoderVideoCodec;
    videoEncoderStartConfig.profile = cmdSettings[i].encoderProfile;
    videoEncoderStartConfig.level = cmdSettings[i].encoderLevel;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.bounds.inputDimension.max.width = windowSettings.position.width;
    videoEncoderStartConfig.bounds.inputDimension.max.height = windowSettings.position.height;
#else

{   /* set proper encoder interlaced/progressive format. NOTE: fw currently doesn't support dynamic switch between interlaced and progressive. */
    NEXUS_VideoFormatInfo fmtInfo;
    NEXUS_VideoFormat_GetInfo(cmdSettings[i].displayFormat, &fmtInfo);
    videoEncoderStartConfig.interlaced = fmtInfo.interlaced;
}
#if BTST_ENABLE_USER_DATA_ENCODE
    videoEncoderStartConfig.encodeUserData = true;
#endif
#endif

    /* encode setting and startSetting to be set after end-to-end delay is determined */

    /* get end-to-end delay (Dee) for audio and video encoders;
     * TODO: match AV delay! In other words,
     *   if (aDee > vDee) {
     *       vDee' = aDee' = aDee;
     *   }
     *   else {
     *       vDee' = aDee' = vDee;
     *   }
     */
    {
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        unsigned Dee;
#endif
        /* NOTE: video encoder delay is in 27MHz ticks */
        NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
        printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
        NEXUS_AudioMuxOutput_GetDelayStatus(audioMuxOutput, audioCodec, &audioDelayStatus);
        printf("\tAudio codec %d end-to-end delay = %u ms\n", audioCodec, audioDelayStatus.endToEndDelay);

        Dee = audioDelayStatus.endToEndDelay * 27000; /* in 27MHz ticks */
        if(Dee > videoDelay.min)
        {
            if(Dee > videoDelay.max)
            {
                BDBG_ERR(("\tAudio Dee is way too big! Use video Dee max!"));
                Dee = videoDelay.max;
            }
            else
            {
                printf("\tUse audio Dee %u ms %u ticks@27Mhz!\n", Dee/27000, Dee);
            }
        }
        else
        {
            Dee = videoDelay.min;
            printf("\tUse video Dee %u ms or %u ticks@27Mhz!\n\n", Dee/27000, Dee);
        }
        videoEncoderConfig.encoderDelay = Dee;

        /* Start audio mux output */
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&audioMuxStartSettings);
        audioMuxStartSettings.stcChannel = stcChannelTranscode;
        audioMuxStartSettings.presentationDelay = Dee/27000;/* in ms */
        NEXUS_AudioMuxOutput_Start(audioMuxOutput, &audioMuxStartSettings);
#else
        videoEncoderConfig.encoderDelay = videoDelay.min;
#endif
    }
    /* Note: video encoder SetSettings needs to be called after the encoder delay is determined; */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeVideo = NEXUS_Playpump_Open(1, &playpumpConfig);
    assert(playpumpTranscodeVideo);
#endif

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;
    playpumpTranscodeAudio = NEXUS_Playpump_Open(2, &playpumpConfig);
    assert(playpumpTranscodeAudio);
#endif
    playpumpTranscodePcr = NEXUS_Playpump_Open(3, &playpumpConfig);
    assert(playpumpTranscodePcr);

    BKNI_CreateEvent(&finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = finishEvent;
    streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.stcChannel = stcChannelTranscode;

#if BTST_HAS_VIDEO_ENCODE_TEST
    muxConfig.video[0].pid = 0x11;
    muxConfig.video[0].encoder = videoEncoder;
    muxConfig.video[0].playpump = playpumpTranscodeVideo;
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    muxConfig.audio[0].pid = 0x12;
    muxConfig.audio[0].muxOutput = audioMuxOutput;
    muxConfig.audio[0].playpump = playpumpTranscodeAudio;
#endif
    muxConfig.pcr.pid = 0x13;
    muxConfig.pcr.playpump = playpumpTranscodePcr;
    muxConfig.pcr.interval = 50;

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
#if BTST_ENABLE_AUDIO_ENCODE
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
#else
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed);
#endif
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    /* Start decoder */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
#else
    NEXUS_AudioDecoder_Start(audioPassthrough, &audioProgram);
#endif
#endif
    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    recpump = NEXUS_Recpump_Open(0, NULL);
    assert(recpump);

    record = NEXUS_Record_Create();
    assert(record);

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordSettings);

#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
{
    unsigned userDataPid[2] = {2501, 2514};

    NEXUS_Message_GetDefaultSettings(&messageSettings);
    /* SCTE 270 spec max TS VBI user data bitrate=270Kbps, 256KB buffer can hold 7.5 seconds;
       worthy user data for video synchronization; TODO: may be reduced if unnecessary */
    messageSettings.bufferSize = 256*1024;
    messageSettings.overflow.callback = message_callback; /* report overflow error */
    messageSettings.overflow.param = 0;
    muxConfig.userdata[0].message = NEXUS_Message_Open(&messageSettings);
    BDBG_ASSERT(muxConfig.userdata[0].message);
    messageSettings.overflow.param = 1;
    muxConfig.userdata[1].message = NEXUS_Message_Open(&messageSettings);
    BDBG_ASSERT(muxConfig.userdata[1].message);

    /* open source user data PID channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther; /* capture the TS packets with the user data PES */
    playbackPidSettings.pidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;
    playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
    playbackPidSettings.pidSettings.pidSettings.remap.pid     = 0x14;/* optional PID remap */

    pidChannelUserData[0] = NEXUS_Playback_OpenPidChannel(playback, userDataPid[0], &playbackPidSettings);
    assert(pidChannelUserData[0]);
    /* open second user data PID */
    playbackPidSettings.pidSettings.pidSettings.remap.pid     = 0x15;/* optional PID remap */
    pidChannelUserData[1] = NEXUS_Playback_OpenPidChannel(playback, userDataPid[1], &playbackPidSettings);
    assert(pidChannelUserData[1]);

    NEXUS_Message_GetDefaultStartSettings(muxConfig.userdata[0].message, &messageStartSettings);
    messageStartSettings.format = NEXUS_MessageFormat_eTs;
    messageStartSettings.pidChannel = pidChannelUserData[0];

    /* must start message before stream mux starts */
    NEXUS_Message_Start(muxConfig.userdata[0].message, &messageStartSettings);

    messageStartSettings.pidChannel = pidChannelUserData[1];
    NEXUS_Message_Start(muxConfig.userdata[1].message, &messageStartSettings);

    /* open transcode mux output user data PidChannels */
    pidChannelTranscodeUserData[0] = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 0x14, NULL);
    assert(pidChannelTranscodeUserData[0]);
    pidChannelTranscodeUserData[1] = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 0x15, NULL);
    assert(pidChannelTranscodeUserData[1]);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeUserData[0], NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeUserData[1], NULL);
}
#endif

#define BTST_PMT_PID 0x0055
    /* open PidChannels */
    pidChannelTranscodePcr = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, muxConfig.pcr.pid, NULL);
    assert(pidChannelTranscodePcr);
    pidChannelTranscodePmt = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, BTST_PMT_PID, NULL);
    assert(pidChannelTranscodePmt);
    pidChannelTranscodePat = NEXUS_Playpump_OpenPidChannel(playpumpTranscodePcr, 0, NULL);
    assert(pidChannelTranscodePat);

    /* start mux */
    NEXUS_StreamMux_Start(streamMux,&muxConfig, &muxOutput);
#if BTST_HAS_VIDEO_ENCODE_TEST
    pidChannelTranscodeVideo = muxOutput.video[0];
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    pidChannelTranscodeAudio = muxOutput.audio[0];
#endif

#if BTST_HAS_VIDEO_ENCODE_TEST
    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = cmdSettings[i].encoderVideoCodec;

    /* add multiplex data to the same record */
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeVideo, &recordPidSettings);
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodeAudio, NULL);
#endif
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePcr, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePat, NULL);
    NEXUS_Record_AddPidChannel(record, pidChannelTranscodePmt, NULL);

    fileTranscode = NEXUS_FileRecord_OpenPosix("videos/stream.mpg", "videos/stream.nav");
    assert(fileTranscode);

    /* Start record of stream mux output */
    NEXUS_Record_Start(record, fileTranscode);

    {
static const uint8_t s_auiTSPacket_PAT[188] =
{
   0x47,0x40,0x00,0x30,0xa6,0x40,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,'P', 'A', 'T',
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0x00,0x00,0xb0,0x0d,0x00,
   0x00,0x81,0x00,0x00,0x00,0x01,0xe0,0x55,
   0x65,0x80,0x5e,0xdc,
};
#define BTST_PMT_TRANSPORT_IDX BTST_SYSTEM_TRANSPORT_IDX
static const uint8_t s_auiTSPacket_PMT[188] =
{
   0x47,0x40,BTST_PMT_PID,0x30,
#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
   0x66,/* adaptation field length */
#else
   0x9c,/* adaptation field length */
#endif
   0x40,/* AF flags */
   0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,'P', 'M', 'T',
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,
   /* placeholder for VBI user data PMT section and descriptors */
   0x00,/* pointer */
   0x02,0xb0,0x4d,0x00,0x01,0xc1,0x00,0x00,0xe0,0x13,0xf0,0x00,
   /* VBI user data PID */
   0x06,0xe0,0x14,0xf0,0x19,
   0x52,0x01,0x0e,0x56,0x14,0x64,0x75,0x74,
   0x10,0x88,0x6e,0x6f,0x72,0x17,0x77,0x64,
   0x75,0x74,0x08,0x88,0x73,0x77,0x65,0x16,
   0x91,
   /* VBI user data PID */
   0x06,0xe0,0x15,0xf0,0x13,
   0x0a,0x04,0x73,0x77,0x65,0x00,
#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
   0x52,0x01,
   0x12,0x59,0x08,0x73,0x77,0x65,0x10,0x00,
   0x02,0x00,0x02,
#else
   0x00,/* pointer */
   0x02,0xb0,0x17,0x00,0x01,0xc1,0x00,0x00,0xe0,0x13,0xf0,0x00,
#endif
   0x1b,0xe0,0x11,0xf0,0x00,/* video */
   0x81,0xe0,0x12,0xf0,0x00,/* audio */
   0x3d,0x19,0x07,0x2f
};
        NEXUS_StreamMuxSystemData psi[2];
        /* Get CRC right to be playable by VLCplayer etc 3rd party SW */
        uint32_t uiCRC = CRC32_mpeg(0, (uint8_t *) s_auiTSPacket_PAT + 184 - (8+4*1), 8+4*1);
        fprintf(stderr, "PAT crc=%x\n", uiCRC);
        NEXUS_Memory_Allocate(188, NULL, &pat);
        NEXUS_Memory_Allocate(188, NULL, &pmt);
        BKNI_Memcpy(pat, s_auiTSPacket_PAT, sizeof(s_auiTSPacket_PAT));
        BKNI_Memcpy(pmt, s_auiTSPacket_PMT, sizeof(s_auiTSPacket_PMT));
        ((uint8_t*)pat)[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pat)[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pat)[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pat)[187] = (uiCRC >>  0) & 0xFF;

#if BTST_HAS_VIDEO_ENCODE_TEST
        /* video codec */
        switch(videoEncoderStartConfig.codec)
        {
            case NEXUS_VideoCodec_eMpeg2:         ((uint8_t *) pmt)[184-2*5] = 0x2; break;
            case NEXUS_VideoCodec_eMpeg4Part2:    ((uint8_t *) pmt)[184-2*5] = 0x10; break;
            case NEXUS_VideoCodec_eH264:          ((uint8_t *) pmt)[184-2*5] = 0x1b; break;
            case NEXUS_VideoCodec_eVc1SimpleMain: ((uint8_t *) pmt)[184-2*5] = 0xea; break;
            default:
                BDBG_ERR(("Video encoder codec %d is not supported!\n", videoEncoderStartConfig.codec));
                BDBG_ASSERT(0);
        }
#endif
        /* audio stream type */
        switch(audioCodec)
        {
            case NEXUS_AudioCodec_eMpeg:         ((uint8_t *) pmt)[184-5] = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          ((uint8_t *) pmt)[184-5] = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      ((uint8_t *) pmt)[184-5] = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  ((uint8_t *) pmt)[184-5] = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacLoas:      ((uint8_t *) pmt)[184-5] = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAacPlusLoas:  ((uint8_t *) pmt)[184-5] = 0x11; break;/* LOAS */
            case NEXUS_AudioCodec_eAc3:          ((uint8_t *) pmt)[184-5] = 0x81; break;
            default:
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audioCodec));
        }
        /* A+V two ES */
#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU
        uiCRC = CRC32_mpeg(0, (uint8_t *) pmt + 184 - (12+5*2)-54, 12+5*2+54);
#else
        uiCRC = CRC32_mpeg(0, (uint8_t *) pmt + 184 - (12+5*2), 12+5*2);
#endif
        fprintf(stderr, "PMT crc=%x\n", uiCRC);

        ((uint8_t*)pmt)[184] = (uiCRC >> 24) & 0xFF;
        ((uint8_t*)pmt)[185] = (uiCRC >> 16) & 0xFF;
        ((uint8_t*)pmt)[186] = (uiCRC >>  8) & 0xFF;
        ((uint8_t*)pmt)[187] = (uiCRC >>  0) & 0xFF;
        NEXUS_Memory_FlushCache(pat, sizeof(s_auiTSPacket_PAT));
        NEXUS_Memory_FlushCache(pmt, sizeof(s_auiTSPacket_PMT));
        BKNI_Memset(psi, 0, sizeof(psi));
        psi[0].size = 188;
        psi[0].pData = pat;
        psi[0].timestampDelta = 100;
        psi[1].size = 188;
        psi[1].pData = pmt;
        psi[1].timestampDelta = 100;
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &psi[0]);
        NEXUS_StreamMux_AddSystemDataBuffer(streamMux, &psi[1]);
}

    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to continue; type 'q' to quit\n");
    key = getchar();

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST

#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Stop(audioDecoder);
#else
    NEXUS_AudioDecoder_Stop(audioPassthrough);
#endif
    NEXUS_AudioMixer_Stop(audioMixer);
    NEXUS_AudioMuxOutput_Stop(audioMuxOutput);
#endif
#if NEXUS_HAS_SYNC_CHANNEL
    /* disconnect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    syncChannelSettings.audioInput[0] = NULL;
    syncChannelSettings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif


#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_Stop(videoEncoder, NULL);
#endif
    NEXUS_StreamMux_Finish(streamMux);
    /* wait for the encoder buffer model's data to be drained */
    if(BKNI_WaitForEvent(finishEvent, (videoEncoderConfig.encoderDelay/27000)*2)!=BERR_SUCCESS) {
        fprintf(stderr, "TIMEOUT\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_Record_Stop(record);
    /* Note: remove all record PID channels before stream mux stop since streammux would close the A/V PID channels */
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_StreamMux_Stop(streamMux);
#if BTST_ENABLE_TS_LAYER_USER_DATA_PASS_THRU /* stop message after stream mux stops */
    NEXUS_Message_Stop(muxConfig.userdata[0].message);
    NEXUS_Message_Close(muxConfig.userdata[0].message);
    NEXUS_Message_Stop(muxConfig.userdata[1].message);
    NEXUS_Message_Close(muxConfig.userdata[1].message);
#endif

    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
    NEXUS_FileRecord_Close(fileTranscode);

    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_Playpump_CloseAllPidChannels(playpumpTranscodePcr);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

    if(enableSdDisplay) NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(display);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_StreamMux_Destroy(streamMux);

    NEXUS_Playpump_Close(playpumpTranscodePcr);

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_Playpump_Close(playpumpTranscodeVideo);
    NEXUS_VideoEncoder_Close(videoEncoder);
#endif

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioEncoder_Close(audioEncoder);
#endif
    NEXUS_AudioMixer_RemoveAllInputs(audioMixer);
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(audioMixer));
    NEXUS_AudioMixer_Close(audioMixer);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif

    NEXUS_Playpump_Close(playpumpTranscodeAudio);
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);

#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#else
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));
#endif
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
#endif
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Close(audioDecoder);
#else
    NEXUS_AudioDecoder_Close(audioPassthrough);
#endif
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);
    NEXUS_Memory_Free(pat);
    NEXUS_Memory_Free(pmt);

    if(key != 'q')
    {
        i = iteration++%TEST_ITERATIONS;
        BDBG_WRN(("Start %d iteration.....", iteration));
        goto again;
    }

    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX */
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return 0;
}
#endif
