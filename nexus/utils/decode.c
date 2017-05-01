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
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if !NEXUS_HAS_VIDEO_DECODER || !NEXUS_HAS_AUDIO
int main(void)
{
    printf("This application is not supported on this platform (needs audio and video decoder)!\n");
    return 0;
}
#else
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_display_vbi.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_surface.h"
#include "nexus_core_utils.h"

#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <sys/time.h>

#if NEXUS_HAS_ASTM
#include "nexus_astm.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include "cmdline_args.h"
#include "frontend.h"
#include "decoder_bitrate.h"

BDBG_MODULE(decode);

#include "hotplug.c"

typedef enum DecodeSource {
    DecodeSource_eStreamer,
    DecodeSource_eFrontend
} DecodeSource;

static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle pcrPidChannel, videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayVbiSettings displayVbiSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderOpenSettings openSettings;
    NEXUS_AudioDecoderHandle audioDecoder, audioPassthroughDecoder=NULL;
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    NEXUS_AudioDecoderStartSettings audioProgram;
#if NEXUS_HAS_ASTM
    NEXUS_AstmSettings astmSettings;
    NEXUS_AstmHandle astm;
#endif
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;
#endif
    struct util_opts_t opts;
    NEXUS_Error rc;
    unsigned start;
    FrontendSettings frontendSettings;
    DecodeSource source = DecodeSource_eStreamer;
    struct decoder_bitrate video_bitrate;
    struct decoder_bitrate audio_bitrate;
#if NEXUS_HAS_HDMI_OUTPUT
    struct hotplug_context hotplug_context;
#endif


    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    frontend_init(&frontendSettings);
    if (cmdline_parse(argc, argv, &opts)) {
        return 0;
    }
    if (frontend_selected(&frontendSettings)) {
        source = DecodeSource_eFrontend;
    }
    else {
        source = DecodeSource_eStreamer;
    }
    if (cmdline_probe(&opts.common, opts.filename, &opts.indexname)) {
        return 1;
    }
    /* They MUST include a video and audio PID for the decode */
    if (!opts.common.videoPid && !opts.common.audioPid) {
        BDBG_ERR(("Missing video or audio pid specification; See usage."));
        print_usage(argv[0]);
        return 1;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    platformSettings.openFrontend = source == DecodeSource_eFrontend;
    if(opts.avc51) {
        platformSettings.videoDecoderModuleSettings.heapSize[0].general = (84*1024*1024);
        platformSettings.videoDecoderModuleSettings.numDecodes = 1;
    }

    if(opts.maxLiveDataRate) {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = opts.maxLiveDataRate;
        printf("Changing parser band %d data rate to %d bps\n\n",NEXUS_ParserBand_e0,opts.maxLiveDataRate);
    }

    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    decoder_bitrate_init(&video_bitrate);
    decoder_bitrate_init(&audio_bitrate);

    if (source == DecodeSource_eFrontend) {
        if (!frontend_check_capabilities(&frontendSettings, &platformConfig)) {
            BDBG_ERR(("Unable to find capable frontend"));
            return 1;
        }
        frontend_set_settings(&frontendSettings, &opts.common);
    }
    else {
    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    rc = NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    BDBG_ASSERT(!rc);
    }

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (source == DecodeSource_eFrontend) {
        frontend_set_parserbandsettings(&frontendSettings, &parserBandSettings);
    }
    else {
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    }
    parserBandSettings.transportType = opts.common.transportType;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    /* Open the audio and video pid channels */
    pcrPidChannel = NEXUS_PidChannel_Open(parserBand, opts.common.pcrPid?opts.common.pcrPid:opts.common.videoPid, NULL);
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, opts.common.videoPid, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, opts.common.audioPid, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

#if NEXUS_HAS_SYNC_CHANNEL
    if (opts.sync) {
        NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
        syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    }
#endif

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.common.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.frameRate = opts.common.videoFrameRate;
    videoProgram.aspectRatio = opts.common.aspectRatio;
    videoProgram.sampleAspectRatio.x = opts.common.sampleAspectRatio.x;
    videoProgram.sampleAspectRatio.y = opts.common.sampleAspectRatio.y;
    videoProgram.errorHandling = opts.videoErrorHandling;

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = opts.common.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0]) {
        rc = NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        BDBG_ASSERT(!rc);
    }
    NEXUS_AudioDecoder_GetSettings(audioDecoder, &audioDecoderSettings);
    audioDecoderSettings.wideGaThreshold = opts.looseAudioTsm;
    rc = NEXUS_AudioDecoder_SetSettings(audioDecoder, &audioDecoderSettings);
    BDBG_ASSERT(!rc);
    /* TODO: param for compressed passthrough of same or different pid */
    if (platformConfig.outputs.spdif[0]) {
        if ( opts.common.audioCodec == NEXUS_AudioCodec_eAc3Plus || opts.common.audioCodec == NEXUS_AudioCodec_eWmaPro )
        {
            /* These codecs pasthrough as part of decode (ac3+ is transcoded to ac3, wma pro is not transcoded) */
            BDBG_WRN(("spdif audio: decoder 0 -> compressed"));
            rc = NEXUS_AudioOutput_AddInput(
                NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eCompressed));
        }
        else if ( opts.common.audioCodec && opts.common.audioCodec != NEXUS_AudioCodec_eMlp )
        {
            BDBG_WRN(("spdif audio: decoder 1 -> compressed"));
            audioPassthroughDecoder = NEXUS_AudioDecoder_Open(1, NULL);
            rc = NEXUS_AudioOutput_AddInput(
                NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(audioPassthroughDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (opts.common.useHdmiOutput ) {
        NEXUS_HdmiOutputStatus hdmiStatus;
        rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
        /* If its connected we'll try to use compressed */
        if ( !rc && hdmiStatus.connected )
        {
            if (( audioProgram.codec == NEXUS_AudioCodec_eAc3 ) && ( hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eAc3] ))
            {
                if (!audioPassthroughDecoder) {
                    audioPassthroughDecoder = NEXUS_AudioDecoder_Open(1, NULL);
                }
                NEXUS_AudioOutput_AddInput(
                    NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(audioPassthroughDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            }
            else
            {
                NEXUS_AudioOutput_AddInput(
                    NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            }
        }
        else
        {
            /* if not connected we'll default to stereo on hotplug */
            NEXUS_AudioOutput_AddInput(
                NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
    }
#endif

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = opts.common.displayType;
    displaySettings.format = opts.common.displayFormat;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (opts.common.useCompositeOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
        BDBG_ASSERT(!rc);
    }
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (opts.common.useComponentOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        BDBG_ASSERT(!rc);
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    if (opts.common.useHdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiSettings;

        rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        BDBG_ASSERT(!rc);

        hotplug_context.hdmi = platformConfig.outputs.hdmi[0];
        hotplug_context.display = display;
        hotplug_context.ignore_edid = opts.common.ignore_edid;

        /* Install hotplug callback -- video only for now */
        NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context = &hotplug_context;
        NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

        /* Force a hotplug to switch to preferred format */
        hotplug_callback(&hotplug_context, 0);
    }
#endif

    if (opts.graphics) {
        NEXUS_SurfaceHandle framebuffer;
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_GraphicsSettings graphicsSettings;
        NEXUS_VideoFormatInfo videoFormatInfo;
        unsigned i,j;

        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 720;
        surfaceCreateSettings.height = videoFormatInfo.height;
        framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);
        NEXUS_Surface_GetMemory(framebuffer, &mem);
        for (i=0;i<surfaceCreateSettings.height;i++) {
            for (j=0;j<surfaceCreateSettings.width;j++) {
                /* create checker board */
                ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (((i/50)%2) ^ ((j/50)%2)) ? 0x00000000 : 0xFFFFFFFF;
            }
        }

        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = surfaceCreateSettings.width;
        graphicsSettings.clip.height = surfaceCreateSettings.height;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
        BDBG_ASSERT(!rc);
    }

    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.contentMode = opts.common.contentMode;
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    if(opts.common.videoCdb) {
        openSettings.fifoSize = opts.common.videoCdb*1024;
    }
    if(opts.avc51) {
        openSettings.avc51Enabled = opts.avc51;
    }
    /* bring up decoder and connect to display */
    switch (opts.common.extVideoCodec) {
    case NEXUS_VideoCodec_eH264_Svc:
    case NEXUS_VideoCodec_eH264_Mvc:
        openSettings.enhancementPidChannelSupported = true;
        break;
    default:
        break;
    }
    videoDecoder = NEXUS_VideoDecoder_Open(opts.common.videoDecoder, &openSettings);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    if (opts.common.sourceOrientation != NEXUS_VideoDecoderSourceOrientation_e2D) {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = opts.common.sourceOrientation;
    }
    videoDecoderSettings.supportedCodecs[opts.common.extVideoCodec] = opts.common.extVideoCodec;
    if (opts.common.maxWidth && opts.common.maxHeight) {
        videoDecoderSettings.maxWidth = opts.common.maxWidth;
        videoDecoderSettings.maxHeight = opts.common.maxHeight;
    }
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetVbiSettings(display, &displayVbiSettings);
    displayVbiSettings.vbiSource = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    displayVbiSettings.closedCaptionEnabled = opts.closedCaptionEnabled;
    displayVbiSettings.closedCaptionRouting = opts.closedCaptionEnabled;
    rc = NEXUS_Display_SetVbiSettings(display, &displayVbiSettings);
    BDBG_ASSERT(!rc);

#if NEXUS_HAS_ASTM
    if (opts.astm) {
        /* create an astm instance and connect */
        NEXUS_Astm_GetDefaultSettings(&astmSettings);
        astmSettings.stcChannel = stcChannel;
        astmSettings.videoDecoder = videoDecoder;
        astmSettings.audioDecoder[0] = audioDecoder;
        astmSettings.stcMaster = audioDecoder; /* audio master is preferred */
        astmSettings.enableAutomaticLifecycleControl = true;
        astmSettings.enabled = true;
        astm = NEXUS_Astm_Create(&astmSettings);
    }
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    if (opts.sync) {
        NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
        syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        rc = NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
        BDBG_ASSERT(!rc);
    }
#endif
    if (source == DecodeSource_eFrontend) {
        frontend_tune(&frontendSettings);
    }

    /* Start Decoders */
    start = b_get_time();
    if (opts.common.videoPid) {
        rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
        decoder_bitrate_control_video(&video_bitrate, videoDecoder, true);
    }
    if (opts.common.audioPid) {
        decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, true);
        rc = NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
        BDBG_ASSERT(!rc);
        if(audioPassthroughDecoder) {
            rc = NEXUS_AudioDecoder_Start(audioPassthroughDecoder,  &audioProgram);
            /* This may fail in some cases because we haven't atached any outputs.  Don't assert. */
            /*BDBG_ASSERT(!rc);*/
        }
    }

#if 1
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;
        unsigned bitrate = 0;
        int strength;
        unsigned levelPercent, qualityPercent;

        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);

#define MEGABYTE (1000*1000)

        if (opts.common.videoPid) {
            NEXUS_VideoDecoderFifoStatus fifoStatus;
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            NEXUS_VideoDecoder_GetFifoStatus(videoDecoder, &fifoStatus);

            bitrate = decoder_bitrate_video(&video_bitrate, &status);
            printf("video %.4dx%.4d%s%s (%s), pts %#x, stc %#x (diff %d), fifo=%d%%, %d.%01dMbps, queueDepth=%d, latency=%dmsec\n",
                   status.source.width, status.source.height,
                   status.interlaced ? "i" : "p",
                   lookup_name(g_videoFrameRateStrs, status.frameRate),
                   lookup_name(g_videoFormatStrs, status.format),
                   status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0,
                bitrate/MEGABYTE,(bitrate%MEGABYTE)*10/MEGABYTE,
                status.queueDepth,
                fifoStatus.bufferLatency);
        }
        if (opts.common.audioPid) {
            NEXUS_AudioDecoder_GetStatus(audioDecoder, &audioStatus);
            bitrate = decoder_bitrate_audio(&audio_bitrate, &audioStatus);
            printf("audio            pts %#x, stc %#x (diff %d), fifo=%d%% %uKbps\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0,
                 (bitrate + 500)/1000
                );
        }
        if (frontend_getStrength(&frontendSettings, &strength, &levelPercent, &qualityPercent)) {
            printf("strength: %d.%.1dBm level: %d%% quality: %d%%\n", strength / 10, abs(strength) % 10, levelPercent, qualityPercent);
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    NEXUS_Platform_Uninit();
#endif

    return 0;
}
#endif /* !NEXUS_HAS_VIDEO_DECODER || !NEXUS_HAS_AUDIO */
/*
************************************************

examples / test cases

# basic decode of streamer input
nexus decode -video 0x31 -audio 0x34 -video_type mpeg -audio_type ac3

************************************************
*/

