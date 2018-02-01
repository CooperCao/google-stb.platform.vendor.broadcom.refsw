/******************************************************************************
 * Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_core_utils.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_input.h"
#include "nexus_video_decoder.h"
#include "nexus_video_window.h"
#include "nexus_mosaic_display.h"
#include "nexus_stc_channel.h"
#include "nexus_parser_band.h"
#include "nexus_playback.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

/* the following define the default input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11
#define EXT_VIDEO_PID 0x12

int main(int argc, char **argv)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_FilePlayHandle file=NULL;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback=NULL;
    NEXUS_PlaybackSettings playbackSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i;
    unsigned gfx_x = 150;
    unsigned gfx_y = 370;
    unsigned gfx_width = 350;
    unsigned gfx_height = 50;
    int video_zoffset = 0;
    NEXUS_VideoCodec videoCodec = VIDEO_CODEC;
    unsigned videoPid = VIDEO_PID, extVideoPid = EXT_VIDEO_PID;
    int curarg = 1;
    const char *filename = NULL;
    bool fullres = false;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-box") && curarg+1 < argc) {
            sscanf(argv[++curarg], "%u,%u,%u,%u", &gfx_x, &gfx_y, &gfx_width, &gfx_height);
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1 < argc) {
            videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-ext_video") && curarg+1 < argc) {
            extVideoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp(argv[curarg], "avc") || !strcmp(argv[curarg], "h264")) {
                videoCodec = NEXUS_VideoCodec_eH264;
            }
            else if (!strcmp(argv[curarg], "mvc")) {
                videoCodec = NEXUS_VideoCodec_eH264_Mvc;
            }
        }
        else if (!strcmp(argv[curarg], "-fullres")) {
            fullres = true;
        }
        else {
            filename = argv[curarg];
        }
        curarg++;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    /* tell the decoder about the orientation of the source */
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    if (fullres) {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame;
    }
    else {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight;
    }
    NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    
    if (filename)
    {
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        playpump = NEXUS_Playpump_Open(0, NULL);
        playback = NEXUS_Playback_Create();

        file = NEXUS_FilePlay_OpenPosix(filename, NULL);
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", filename);
            return -1;
        }

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        playbackSettings.stcChannel = videoProgram.stcChannel;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);

        videoProgram.codec = videoCodec;

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        videoProgram.pidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);
        if (videoCodec==NEXUS_VideoCodec_eH264_Mvc) {
            videoProgram.enhancementPidChannel = NEXUS_Playback_OpenPidChannel(playback, extVideoPid, &playbackPidSettings);
        }
    }
    else
    {
        /* live */
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
        parserBandSettings.transportType = TRANSPORT_TYPE;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = videoCodec;
        videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        if (videoCodec==NEXUS_VideoCodec_eH264_Mvc) {
            videoProgram.enhancementPidChannel = NEXUS_PidChannel_Open(parserBand, extVideoPid, NULL);
        }

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
        videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    }

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    if (!fullres) {
        displaySettings.format = NEXUS_VideoFormat_e720p;
        displaySettings.display3DSettings.overrideOrientation = true;
        displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
    }
    else {
        displaySettings.format = NEXUS_VideoFormat_e720p_3DOU_AS;
        displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eHdmiDvo; /* HDMI master mode is required */
    }
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
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
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    /**************************************
    * video
    ***************************************/

    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    if (filename) {
        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);
    }

    for (i=0;;i++) {
        char buf[64];

#define abs(X) ((X)>0?(X):-(X))

        {
            /* TODO: if you want to customize the z-offset algorithm for video, do it here */
            unsigned width = fullres?displayFormatInfo.width:displayFormatInfo.width/2;
            
            NEXUS_VideoWindow_GetSettings(window, &windowSettings);
            windowSettings.position.x = video_zoffset<0?0:video_zoffset;
            windowSettings.position.width = width - abs(video_zoffset);
            windowSettings.window3DSettings.rightViewOffset = -video_zoffset;
            rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
            BDBG_ASSERT(!rc);
        }

prompt:
        printf("Enter video z-offset:\n");
        fgets(buf, 64, stdin);

        if (filename) {
            if (buf[0] == 'p') {
                static bool paused = false;
                if (paused)
                    NEXUS_Playback_Play(playback);
                else
                    NEXUS_Playback_Pause(playback);
                paused = !paused;
                goto prompt;
            }
        }

        sscanf(buf, "%d", &video_zoffset);
    }

    return 0;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
