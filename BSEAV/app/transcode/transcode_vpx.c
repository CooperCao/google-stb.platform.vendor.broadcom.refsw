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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_FILE_MUX
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
#include "namevalue.inc"

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
#define BTST_INPUT_MAX_WIDTH 416
#define BTST_INPUT_MAX_HEIGHT 224
#endif

BDBG_MODULE(transcode_vpx);
static void
transcoderFinishCallback(void *context, int param)
{
    BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_WRN(("Finish callback invoked, now stop the file mux."));
    BKNI_SetEvent(finishEvent);
}

static unsigned get_display_index(unsigned encoder)
{
    NEXUS_VideoEncoderCapabilities cap;
    NEXUS_GetVideoEncoderCapabilities(&cap);
    if (encoder < NEXUS_MAX_VIDEO_ENCODERS && cap.videoEncoder[encoder].supported) {
        return cap.videoEncoder[encoder].displayIndex;
    }
    return 0;
}

static void print_usage(void) {
            printf("\ntranscode_vpx [-h]:\n");
            printf("\nOptions:\n");
            printf("  -h          - to print the usage info\n");
            printf("  -avc        - to encode h264 video\n");
            printf("  -vp9        - to encode vp9 video\n");
            printf("  -ivf        - to output IVF file\n");
            printf("  -nrt        - to encode in RT mode\n");
            printf("  -in FILE    - to transcode from the input FILE.\n");
            printf("  -out FILE    - to transcode to the output FILE.\n");
            printf("  -video_size W,H - Encode output size WxH.\n");
            printf("  -video_bitrate N - Encode output bitrate N bps.\n");
            print_list_option("video_framerate", g_videoFrameRateStrs);
}

/* include media probe */
#include "media_probe.c"
int main(int argc, char **argv)
{
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_FILE_MUX
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowMadSettings windowMadSettings;
    NEXUS_VideoWindowSettings windowSettings;
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindowScalerSettings sclSettings;
#endif

    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
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
#if !defined(NEXUS_NUM_DSP_VIDEO_ENCODERS) || (NEXUS_NUM_DSP_VIDEO_ENCODERS && NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT)
    NEXUS_DisplayStgSettings stgSettings;
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
#endif
    NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eH264;/* input codec */
    NEXUS_TransportType transportType = NEXUS_TransportType_eTs;
    unsigned videoPid = 0x101;
    unsigned width = 720, height = 480;
    NEXUS_VideoFrameRate frameRate = NEXUS_VideoFrameRate_e29_97;
    unsigned bitrate = 2000000;
    bool probe = false;
    bool nonRealTime = false; /* default RT */
    bool avc = false;
    bool vp9 = false;
    const char *fname = "videos/avatar_AVC_15M.ts";
    const char *outputFile = getenv("IVF_SUPPORT")? "videos/stream.ivf":"videos/stream.pes";/* default PES file output */

    int i = 0;

    for(i=0; i<argc; i++) {
        if(!strcmp("-h",argv[i])) {
            print_usage();
            return 0;
        }
        if(!strcmp("-avc",argv[i]) || !strcmp("-264",argv[i]) || !strcmp("-h264",argv[i])) {
            avc = true; /* enabled vp8 encoding */
            fprintf(stderr, "AVC encoding\n");
        }
        if(!strcmp("-vp9",argv[i]) || !strcmp("-VP9",argv[i])) {
            vp9 = true; /* enabled vp8 encoding */
            fprintf(stderr, "VP9 encoding\n");
        }
        if(!strcmp("-ivf",argv[i])) {
            outputFile = "videos/stream.ivf";
            fprintf(stderr, "IVF output file format\n");
            setenv("IVF_SUPPORT", "y", 1);
        }
        if(!strcmp("-nrt", argv[i])) {
            nonRealTime = true;
            fprintf(stderr, "NRT mode.\n");
        }
        if(!strcmp("-in",argv[i]) && (i+1 < argc)) {
            #define BTST_P_DEFAULT(a, b) {if(a==0) a = b;}
            fname = argv[++i];
            probe = true; /* enabled media probe */
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
            nonRealTime = true; /* enabled NRT mode by default */
#endif
        }
        if(!strcmp("-out",argv[i]) && (i+1 < argc)) {
            #define BTST_P_DEFAULT(a, b) {if(a==0) a = b;}
            outputFile = argv[++i];
        }
        if(!strcmp("-video_size",argv[i]) && (i+1 < argc)) {
            if (sscanf(argv[++i], "%u,%u", &width, &height) != 2) {
                print_usage();
                return -1;
            }
            fprintf(stderr, "Video size %ux%u\n", width, height);
        }
        if(!strcmp("-video_bitrate",argv[i]) && (i+1 < argc)) {
            bitrate = strtoul(argv[++i], NULL, 0);
            fprintf(stderr, "Video bitrate is %u bps\n", bitrate);
        }
        if(!strcmp("-video_framerate",argv[i]) && (i+1 < argc)) {
            frameRate = lookup(g_videoFrameRateStrs, argv[++i]);
            fprintf(stderr, "Video frame rate is %s fps\n", argv[i]);
        }
    }
    fprintf(stderr, "Input file %s\n", fname);
    fprintf(stderr, "Output file %s\n", outputFile);
    i = 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if(probe) {
        struct probe_request probe_request;
        struct probe_results probe_results;
        int rc;

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = fname;
        rc = probe_media_request(&probe_request, &probe_results);
        if (rc) {
            BDBG_ERR(("media probe failed to parse '%s'", probe_request.streamname));
            return BERR_TRACE(rc);
        }
        if(probe_results.num_audio==0 && probe_results.num_video==0) {
            BDBG_ERR(("media probe failed to find any audio or video program in '%s'", probe_request.streamname));
            return BERR_TRACE(rc);
        }
        transportType = probe_results.transportType;
        videoCodec = probe_results.video[0].codec;
        videoPid   = probe_results.video[0].pid;
    }

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
    playbackSettings.playpumpSettings.transportType = transportType;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
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

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
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

    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);

    /* Bring up video encoder display */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    displaySettings.format = NEXUS_VideoFormat_e480p;
    displayTranscode = NEXUS_Display_Open(get_display_index(0), &displaySettings);
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displayTranscode = NEXUS_Display_Open(get_display_index(0), &displaySettings);
    NEXUS_Display_GetStgSettings(displayTranscode, &stgSettings);
    stgSettings.nonRealTime = nonRealTime;
    stgSettings.enabled = true;;
    NEXUS_Display_SetStgSettings(displayTranscode, &stgSettings);
    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    customFormatSettings.width = width;
    customFormatSettings.height = height;
    customFormatSettings.refreshRate = 59940;
    customFormatSettings.interlaced = false;
    NEXUS_Display_SetCustomFormatSettings(displayTranscode, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
#endif
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = BTST_INPUT_MAX_WIDTH;
    windowSettings.position.height = BTST_INPUT_MAX_HEIGHT;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);

    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
#else
    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.forceCapture = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);
#endif

    /* enable deinterlacer to improve quality */
    NEXUS_VideoWindow_GetMadSettings(windowTranscode, &windowMadSettings);
    windowMadSettings.deinterlace = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.enable32Pulldown = true;
    NEXUS_VideoWindow_SetMadSettings(windowTranscode, &windowMadSettings);

    /* connect same decoder to the encoder display;
     * NOTE: simul display + transcode mode might have limitation in audio pathre;
     * here is for video transcode bringup purpose;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    if(!nonRealTime) NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = videoCodec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = false;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.nonRealTime = nonRealTime;
    videoProgram.codec = videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    videoEncoderConfig.bitrateMax = 400*1000;
    BSTD_UNUSED(frameRate);
#else
    videoEncoderConfig.frameRate = frameRate;
    videoEncoderConfig.bitrateMax = bitrate;
    videoEncoderConfig.streamStructure.framesP = 29;
    videoEncoderConfig.streamStructure.framesB = 0;
#endif


    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
    videoEncoderStartConfig.bounds.inputDimension.max.width = BTST_INPUT_MAX_WIDTH;
    videoEncoderStartConfig.bounds.inputDimension.max.height = BTST_INPUT_MAX_HEIGHT;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
    BSTD_UNUSED(avc);
    BSTD_UNUSED(vp9);
#else
    videoEncoderStartConfig.codec = avc? NEXUS_VideoCodec_eH264 : vp9? NEXUS_VideoCodec_eVp9 : NEXUS_VideoCodec_eVp8;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eMain;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelTranscode;
    videoEncoderStartConfig.nonRealTime = nonRealTime;
#endif

    /* encode setting and startSetting to be set after end-to-end delay is determined */

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    printf("\n\tVideo encoder end-to-end delay = %u ms; maximum allowed: %u ms\n", videoDelay.min/27000, videoDelay.max/27000);

        videoEncoderConfig.encoderDelay = videoDelay.min;
    /* Note: video encoder SetSettings needs to be called after the encoder delay is determined; */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);

    BKNI_CreateEvent(&finishEvent);
    NEXUS_FileMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = transcoderFinishCallback;
    muxCreateSettings.finished.context = finishEvent;

    fileMux = NEXUS_FileMux_Create(&muxCreateSettings);
    NEXUS_FileMux_GetDefaultStartSettings(&muxConfig, NEXUS_TransportType_eMpeg2Pes);

    muxConfig.video[0].track = 1;
    muxConfig.video[0].codec = videoEncoderStartConfig.codec;
    muxConfig.video[0].encoder = videoEncoder;
    snprintf(muxConfig.tempDir, sizeof(muxConfig.tempDir), "videos");
    muxOutput = NEXUS_MuxFile_OpenPosix(outputFile);
    if (!muxOutput) {
        fprintf(stderr, "can't open file:%s\n", outputFile);
        return -1;
    }

    /* start mux */
    NEXUS_FileMux_Start(fileMux,&muxConfig, muxOutput);

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    /* Start decoder */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to continue; type 'q' to quit\n");
    getchar();

    /* Bring down system */
    NEXUS_Playback_Stop(playback);
    NEXUS_VideoDecoder_Stop(videoDecoder);
#if NEXUS_HAS_SYNC_CHANNEL
    /* disconnect sync channel */
    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    syncChannelSettings.audioInput[0] = NULL;
    syncChannelSettings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif


    NEXUS_VideoEncoder_Stop(videoEncoder,NULL);
    NEXUS_FileMux_Finish(fileMux);
    /* wait for encode buffer to be drained; double delay margin */
    while(BKNI_WaitForEvent(finishEvent, 3000)!=BERR_SUCCESS) {
        fprintf(stderr, "File mux unfinished! Wait for another 3 seconds...\n");
    }
    BKNI_DestroyEvent(finishEvent);
    NEXUS_FileMux_Stop(fileMux);
    NEXUS_MuxFile_Close(muxOutput);


    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

    if(!nonRealTime) NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoWindow_RemoveInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(display);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_FileMux_Destroy(fileMux);

    NEXUS_VideoEncoder_Close(videoEncoder);

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_StcChannel_Close(stcChannelTranscode);

    NEXUS_Platform_Uninit();
#endif

    return 0;
}

#endif
