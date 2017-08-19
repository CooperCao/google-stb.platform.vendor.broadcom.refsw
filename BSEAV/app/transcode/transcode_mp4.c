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
 ***************************************************************************/
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#if !NEXUS_HAS_VIDEO_ENCODER
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}

#else

#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_adj.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_file_mux.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#define BTST_HAS_VIDEO_ENCODE_TEST       1
#define NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST 1
#define BTST_ENABLE_AUDIO_ENCODE        1

#define BTST_AC3_TRANSCODER_PASSTHRU    1
#define BTST_MPG_TRANSCODER_PASSTHRU    0
#define BTST_AAC_TRANSCODER_PASSTHRU    0

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#define BTST_INPUT_MAX_WIDTH 416
#define BTST_INPUT_MAX_HEIGHT 224
#endif

BDBG_MODULE(transcode_mp4);
static void
transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
    BKNI_SetEvent(finishEvent);
}
#define TEST_ITERATIONS 3
struct cmdSettings {
    NEXUS_VideoFormat displayFormat;
    NEXUS_VideoFrameRate encoderFrameRate;
    unsigned encoderBitrate;
    unsigned encoderGopStructureFramesP;
    unsigned encoderGopStructureFramesB;
    NEXUS_VideoCodec encoderVideoCodec;
    NEXUS_VideoCodecProfile encoderProfile;
    NEXUS_VideoCodecLevel encoderLevel;
}cmdSettings[TEST_ITERATIONS] = {
    {NEXUS_VideoFormat_e720p24hz, NEXUS_VideoFrameRate_e23_976, 6*1000*1000, 23, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eBaseline, NEXUS_VideoCodecLevel_e31},
    {NEXUS_VideoFormat_e1080p24hz, NEXUS_VideoFrameRate_e23_976, 10*1000*1000, 23, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eMain, NEXUS_VideoCodecLevel_e40},
    {NEXUS_VideoFormat_e480p, NEXUS_VideoFrameRate_e29_97, 2*1000*1000, 29, 0, NEXUS_VideoCodec_eH264, NEXUS_VideoCodecProfile_eHigh, NEXUS_VideoCodecLevel_e30}
};

void print_usage(void) {
			printf("\ntranscode_mp4 [-h] [-pd] [-rbs BYTEs]:\n");
			printf("\nOptions:\n");
			printf("  -h          - to print the usage info\n");
			printf("  -pd         - to enable progressive download mp4 file mux. Default OFF.\n");
			printf("  -rbs BYTEs  - to specify relocation buffer size in bytes.\n");
			printf("     Note: for progressive download mp4 file mux, relocation buffer is\n");
			printf("     used to relocate mdat box at finalization stage for mp4 file mux.\n");
			printf("     The larger reloc buffer size, the quicker mp4 file mux can finish.\n");
			printf("     for example, 1MB reloc buffer finishes much faster than default 128 KB.\n");
			printf("  -in FILE    - specify input file (in current directory if full path not specified)\n");
			printf("  -out FILE   - specify output file (in current directory if full path not specified)\n");
			printf("  -duration N - number of seconds to transcode before moving on to next iteration\n");
			printf("                (if -duration not specified, will prompt the user to continue)\n");
			printf("  -repeat N   - number of iterations to perform\n");
			printf("      NOTE: if -duration given and -repeat is not given, it will quit after the first iteration\n");
			printf("      repeat count is ignored if no duration given\n");
			printf("  -temp DIR   - specify the directory to use for temporary files (default = 'videos')\n");
}

int main(int argc, char **argv)
{
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_STREAM_MUX
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowMadSettings windowMadSettings;
    NEXUS_VideoWindowSettings windowSettings;
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindowScalerSettings sclSettings;
#endif

    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_AudioDecoderHandle audioDecoder, audioPassthrough;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioEncoderSettings audioEncoderSettings;
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
#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
#endif
    NEXUS_FileMuxHandle fileMux;
    NEXUS_MuxFileHandle muxOutput;
    NEXUS_FileMuxCreateSettings muxCreateSettings;
    BKNI_EventHandle finishEvent;
    NEXUS_FileMuxStartSettings muxConfig;
    NEXUS_StcChannelHandle stcChannelTranscode;

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#endif

    NEXUS_DisplaySettings displaySettings;
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */
    const char *fname = "videos/avatar_AVC_15M.ts"/*  "videos/WildChina_Short.ts"*/;
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPEG */
    const char *fname = "videos/cnnticker.mpg";
#else /* AAC */
    const char *fname = "videos/testjun26_188.mpg";
#endif
    const char *mp4File = "videos/stream.mp4";
    const char *temp_dir = "videos";

    int i = 0;
    int iteration = 1;
    char key;
    bool bProgressiveDownload = false;
    unsigned relocBufSize = 0;
    unsigned duration = 0;
    unsigned max_iterations = 0;

    for(i=0; i<argc; i++) {
        if(!strcmp("-h",argv[i])) {
            print_usage();
            return 0;
        }
        if(!strcmp("-pd",argv[i])) {
            bProgressiveDownload = true;
            fprintf(stderr, "Enabled progressive download mp4 muxing...\n");
        }
        if(!strcmp("-rbs",argv[i])) {
            relocBufSize = strtoul(argv[++i], NULL, 0);
            fprintf(stderr, "Relocation buffer size = %u bytes.\n", relocBufSize);
        }
        if (!strcmp("-in", argv[i]) && (i+1 < argc)) {
            fname = argv[++i];
            fprintf(stderr, "Transcoding file %s\n", fname);
        }
        if (!strcmp("-out", argv[i]) && (i+1 < argc)) {
            mp4File = argv[++i];
            fprintf(stderr, "Output to file %s\n", mp4File);
        }
        if(!strcmp("-duration", argv[i]) && (i+1 < argc)) {
            duration = strtoul(argv[++i], NULL, 0);
        }
        if(!strcmp("-repeat", argv[i]) && (i+1 < argc)) {
            max_iterations = strtoul(argv[++i], NULL, 0);
        }
        if (!strcmp("-temp", argv[i]) && (i+1 < argc)) {
            temp_dir = argv[++i];
            fprintf(stderr, "Using directory %s for temporary files\n", temp_dir);
        }
    }
    i = 0;
    if (max_iterations && !duration)
    {
      max_iterations = 0; /* ignore repeat count if no duration specified */
      fprintf(stderr, "Missing -duration; ignoring -repeat\n");
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

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
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
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.format = NEXUS_VideoFormat_e720p;
#else
    displaySettings.format = NEXUS_VideoFormat_e480p;
#endif
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
   if(platformConfig.outputs.component[0]){
      NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
   }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif
    window = NEXUS_VideoWindow_Open(display, 0);

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindow_GetMadSettings(window, &windowMadSettings);
    windowMadSettings.deinterlace = false;
    NEXUS_VideoWindow_SetMadSettings(window, &windowMadSettings);
#else
    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(window, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(window, &windowMadSettings);
#endif

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    /* Open the audio decoder */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    audioPassthrough = NEXUS_AudioDecoder_Open(1, NULL);

    /* Open the audio and pcr pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.audio.secondary = audioPassthrough;
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x104, &playbackPidSettings);
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPG */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x22, &playbackPidSettings);
#else /* AAC */
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x110, &playbackPidSettings);
#endif

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
#if BTST_AC3_TRANSCODER_PASSTHRU /* AC3 */
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
#elif BTST_MPG_TRANSCODER_PASSTHRU /* MPG */
    audioProgram.codec = NEXUS_AudioCodec_eMpeg;
#else /* AAC */
    audioProgram.codec = NEXUS_AudioCodec_eAac;
#endif
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

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
    NEXUS_AudioEncoder_GetDefaultSettings(&audioEncoderSettings);
    audioEncoderSettings.codec = NEXUS_AudioCodec_eAacAdts;
    audioCodec = audioEncoderSettings.codec;
    audioEncoder = NEXUS_AudioEncoder_Open(&audioEncoderSettings);
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
    /* Connect decoder to mixer and set as master */
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

    /* Bring up video encoder display */
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.format = NEXUS_VideoFormat_e480p;
    displayTranscode = NEXUS_Display_Open(1, &displaySettings);
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p;/* source is 60hz */
    displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */
    displaySettings.format = cmdSettings[i].displayFormat;
    displayTranscode = NEXUS_Display_Open(NEXUS_ENCODER_DISPLAY_IDX, &displaySettings);/* cmp3 for transcoder */
#endif
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = BTST_INPUT_MAX_WIDTH;
    windowSettings.position.height = BTST_INPUT_MAX_HEIGHT;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    windowSettings.visible = false;
#endif
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);

    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
#endif

    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &windowMadSettings);

    /* GM : Is this required? For DSP encoder it causes the last 16 pixels to be cropped off */
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    /* enable automatic aspect ratio correction mode to convert dynamic source aspect ratio to fixed display aspect ratio since it seems
       MP4 container doesn't tolerate dynamic aspect ratio.*/
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.contentMode = NEXUS_VideoWindowContentMode_eBox;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
#endif

    /* connect same decoder to the encoder display;
     * NOTE: simul display + transcode mode might have limitation in audio pathre;
     * here is for video transcode bringup purpose;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
#if BTST_AC3_TRANSCODER_PASSTHRU /* avatar */
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, /*0x201*/0x101, &playbackPidSettings);
#elif BTST_MPG_TRANSCODER_PASSTHRU /* cnnticker */
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x21, &playbackPidSettings);
#else /* testjun26_188 */
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x100, &playbackPidSettings);
#endif

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
#if BTST_AC3_TRANSCODER_PASSTHRU /* avatar */
    videoProgram.codec = NEXUS_VideoCodec_eH264;
#elif BTST_MPG_TRANSCODER_PASSTHRU /* cnnticker */
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
#else /* testjun26_188 */
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
#endif
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    videoEncoderConfig.bitrateMax = 400*1000;
#else
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e30;
    videoEncoderConfig.frameRate = cmdSettings[i].encoderFrameRate;
    videoEncoderConfig.bitrateMax = cmdSettings[i].encoderBitrate;
    videoEncoderConfig.streamStructure.framesP = cmdSettings[i].encoderGopStructureFramesP;
    videoEncoderConfig.streamStructure.framesB = cmdSettings[i].encoderGopStructureFramesB;
#endif


    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    videoEncoderStartConfig.bounds.inputDimension.max.width = 416;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 224;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
#else
    videoEncoderStartConfig.codec = cmdSettings[i].encoderVideoCodec;
    videoEncoderStartConfig.profile = cmdSettings[i].encoderProfile;
    videoEncoderStartConfig.level = cmdSettings[i].encoderLevel;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
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
#endif


    BKNI_CreateEvent(&finishEvent);
    NEXUS_FileMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = finishEvent;

    /* larger relocation buffer can speed up the progressive download compatible mp4 file mux finish time */
    if(relocBufSize) muxCreateSettings.mp4.relocationBuffer = relocBufSize;

    fileMux = NEXUS_FileMux_Create(&muxCreateSettings);
    NEXUS_FileMux_GetDefaultStartSettings(&muxConfig, NEXUS_TransportType_eMp4);

    /* progressive download comptible means the meta data will be relocated to the beginning of the
     * mp4 file, which means longer finish time to stop the file mux  */
    muxConfig.streamSettings.mp4.progressiveDownloadCompatible = bProgressiveDownload;

#if BTST_HAS_VIDEO_ENCODE_TEST
    muxConfig.video[0].track = 1;
    muxConfig.video[0].codec = videoEncoderStartConfig.codec;
    muxConfig.video[0].encoder = videoEncoder;
#endif
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    muxConfig.audio[0].track = 2;
    muxConfig.audio[0].codec = audioProgram.codec;
#if BTST_ENABLE_AUDIO_ENCODE
    muxConfig.audio[0].codec = audioEncoderSettings.codec;
#endif
    muxConfig.audio[0].muxOutput = audioMuxOutput;
#endif
    snprintf(muxConfig.tempDir, sizeof(muxConfig.tempDir), temp_dir);
    muxOutput = NEXUS_MuxFile_OpenPosix(mp4File);
    if (!muxOutput) {
        fprintf(stderr, "can't open file:%s\n", mp4File);
        return -1;
    }

    /* start mux */
    NEXUS_FileMux_Start(fileMux,&muxConfig, muxOutput);

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    syncChannelSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed);
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

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
#endif

    if (duration)
    {
        BKNI_Sleep(duration*1000);
    }
    else
    {
      /* Playback state machine is driven from inside Nexus. */
      printf("Press ENTER to continue; type 'q' to quit\n");
      key = getchar();
    }

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioDecoder_Stop(audioDecoder);
#else
    NEXUS_AudioDecoder_Stop(audioPassthrough);
#endif
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
    NEXUS_VideoEncoder_Stop(videoEncoder,NULL);
#endif
    NEXUS_FileMux_Finish(fileMux);
    /* wait for encode buffer to be drained; double delay margin */
    while(BKNI_WaitForEvent(finishEvent, 3000)!=BERR_SUCCESS) {
        fprintf(stderr, "File mux unfinished! Wait for another 3 seconds...\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_FileMux_Stop(fileMux);
    NEXUS_MuxFile_Close(muxOutput);


#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel);
#endif
    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

    NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(display);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_FileMux_Destroy(fileMux);

#if BTST_HAS_VIDEO_ENCODE_TEST
    NEXUS_VideoEncoder_Close(videoEncoder);
#endif

#if NEXUS_HAS_AUDIO_MUX_OUTPUT_TEST
#if BTST_ENABLE_AUDIO_ENCODE
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioEncoder_RemoveAllInputs(audioEncoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(audioEncoder));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
    NEXUS_AudioEncoder_Close(audioEncoder);
#else
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(audioMuxOutput));
#endif

    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    NEXUS_AudioMixer_RemoveAllInputs(audioMixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(audioMixer));
    NEXUS_AudioMixer_Close(audioMixer);
    NEXUS_AudioMuxOutput_Destroy(audioMuxOutput);

    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed));

    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_AudioDecoder_Close(audioPassthrough);
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);

    i = iteration++%TEST_ITERATIONS;
    if (((duration == 0) && (key != 'q')) || ((unsigned)iteration <= max_iterations))
    {
       BDBG_WRN(("Start %d iteration.....", iteration));
       goto again;
    }
    NEXUS_Platform_Uninit();

#endif
    return 0;
}

#endif
