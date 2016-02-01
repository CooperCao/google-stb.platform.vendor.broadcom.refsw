/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_decoder.h"
#include "nexus_video_window.h"
#include "nexus_mosaic_display.h"
#include "nexus_stc_channel.h"
#include "nexus_parser_band.h"
#include "nexus_playback.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the following define the default input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

int main(int argc, char **argv)
{
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceHandle framebuffer, offscreen;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PixelFormat pixelFormat;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoWindowHandle parentWindow;
    NEXUS_VideoWindowHandle window[2]; /* mosaics */
    NEXUS_VideoWindowSettings windowSettings[2];
    NEXUS_VideoWindowMosaicSettings mosaicWindowSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
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
    int video_zoffset = 0, graphics_zoffset = 0;
    NEXUS_VideoFormat videoFormat = VIDEO_CODEC;
    unsigned videoPid = VIDEO_PID;
    int curarg = 1;
    const char *filename = NULL;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-box") && curarg+1 < argc) {
            sscanf(argv[++curarg], "%u,%u,%u,%u", &gfx_x, &gfx_y, &gfx_width, &gfx_height);
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1 < argc) {
            videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && curarg+1 < argc) {
            ++curarg;
            if (!strcmp(argv[curarg], "avc") || !strcmp(argv[curarg], "h264")) {
                videoFormat = NEXUS_VideoCodec_eH264;
            }
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
    /* set the source orientation to 2D since we will handle the clipping manually via mosaic windows */
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.customSourceOrientation = true;
    videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e2D;
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

        videoProgram.codec = videoFormat;

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoFormat;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        videoProgram.pidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);
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
        videoProgram.codec = videoFormat;
        videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
        videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    }

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

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

    parentWindow = NEXUS_VideoWindow_Open(display, 0);

    /* the following are only initial coordinates */
    window[0] = NEXUS_VideoWindow_OpenMosaic(parentWindow, 0);
    NEXUS_VideoWindow_GetSettings(window[0], &windowSettings[0]);
    windowSettings[0].position.x = 0;
    windowSettings[0].position.y = 0;
    windowSettings[0].position.width = displayFormatInfo.width/2;
    windowSettings[0].position.height = displayFormatInfo.height;
    rc = NEXUS_VideoWindow_SetSettings(window[0], &windowSettings[0]);
    BDBG_ASSERT(!rc);

    /* mosaic 0 uses the left half of the video */
    NEXUS_VideoWindow_GetMosaicSettings(window[0], &mosaicWindowSettings);
    mosaicWindowSettings.backendMosaic.enabled = true;
    mosaicWindowSettings.backendMosaic.clipRect.x = 0;
    mosaicWindowSettings.backendMosaic.clipRect.y = 0;
    mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2;
    mosaicWindowSettings.backendMosaic.clipRect.height = 1080;
    mosaicWindowSettings.backendMosaic.clipBase.width = 1920;
    mosaicWindowSettings.backendMosaic.clipBase.height = 1080;
    rc = NEXUS_VideoWindow_SetMosaicSettings(window[0], &mosaicWindowSettings);
    BDBG_ASSERT(!rc);

    window[1] = NEXUS_VideoWindow_OpenMosaic(parentWindow, 1);
    NEXUS_VideoWindow_GetSettings(window[1], &windowSettings[1]);
    windowSettings[1].position = windowSettings[0].position;
    windowSettings[1].position.x = windowSettings[1].position.x + displayFormatInfo.width / 2;
    rc = NEXUS_VideoWindow_SetSettings(window[1], &windowSettings[1]);
    BDBG_ASSERT(!rc);

    /* mosaic 1 uses the right half of the video */
    NEXUS_VideoWindow_GetMosaicSettings(window[1], &mosaicWindowSettings);
    mosaicWindowSettings.backendMosaic.enabled = true;
    mosaicWindowSettings.backendMosaic.clipRect.x = 1920/2;
    mosaicWindowSettings.backendMosaic.clipRect.y = 0;
    mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2;
    mosaicWindowSettings.backendMosaic.clipRect.height = 1080;
    mosaicWindowSettings.backendMosaic.clipBase.width = 1920;
    mosaicWindowSettings.backendMosaic.clipBase.height = 1080;
    rc = NEXUS_VideoWindow_SetMosaicSettings(window[1], &mosaicWindowSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoWindow_AddInput(window[0], NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_AddInput(window[1], NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);
    if (filename) {
        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);
    }

    /**************************************
    * graphics
    ***************************************/

    /* older platforms cannot support 32bpp pixel format on a large framebuffer due to bandwidth issues. 
       you can either use a 16bpp pixel format, or reduce the framebuffer width */
#if (BCHP_CHIP==7401) || (BCHP_CHIP==7403) || (BCHP_CHIP==7400)
    pixelFormat = NEXUS_PixelFormat_eA4_R4_G4_B4;
#else
    pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif
    /* create actual framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = pixelFormat;
    #define FRAMEBUFFER_WIDTH 1440
    createSurfaceSettings.width = FRAMEBUFFER_WIDTH;
    createSurfaceSettings.height = 1080;
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* create app offscreen buffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = pixelFormat;
    createSurfaceSettings.width = 720;
    createSurfaceSettings.height = 480;
    offscreen = NEXUS_Surface_Create(&createSurfaceSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    /* build the offscreen buffer */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = offscreen;
    fillSettings.color = 0x00000000;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = offscreen;
    fillSettings.rect.x = gfx_x;
    fillSettings.rect.y = gfx_y;
    fillSettings.rect.width = gfx_width;
    fillSettings.rect.height = gfx_height;
    fillSettings.color = 0xAAFFFFFF;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = 0x00000000;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = FRAMEBUFFER_WIDTH;
    graphicsSettings.clip.height = displayFormatInfo.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);


    for (i=0;;i++) {
        NEXUS_Graphics2DBlitSettings blitSettings;
        char buf[64];

#define abs(X) ((X)>0?(X):-(X))

        {
            /* TODO: if you want to customize the z-offset algorithm for video, do it here */
            NEXUS_VideoWindow_GetSettings(window[0], &windowSettings[0]);
            windowSettings[0].position.x = video_zoffset>0?video_zoffset:0;
            windowSettings[0].position.width = (displayFormatInfo.width/2) - abs(video_zoffset);
            rc = NEXUS_VideoWindow_SetSettings(window[0], &windowSettings[0]);
            BDBG_ASSERT(!rc);
            NEXUS_VideoWindow_GetMosaicSettings(window[0], &mosaicWindowSettings);
            mosaicWindowSettings.backendMosaic.clipRect.x = (video_zoffset<0)?-video_zoffset:0;
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2 - abs(video_zoffset);
            rc = NEXUS_VideoWindow_SetMosaicSettings(window[0], &mosaicWindowSettings);
            BDBG_ASSERT(!rc);

            NEXUS_VideoWindow_GetSettings(window[1], &windowSettings[1]);
            windowSettings[1].position.x = displayFormatInfo.width/2 + (video_zoffset<0?-video_zoffset:0);
            windowSettings[1].position.width = (displayFormatInfo.width/2) - abs(video_zoffset);
            rc = NEXUS_VideoWindow_SetSettings(window[1], &windowSettings[1]);
            BDBG_ASSERT(!rc);
            NEXUS_VideoWindow_GetMosaicSettings(window[1], &mosaicWindowSettings);
            mosaicWindowSettings.backendMosaic.clipRect.x = 1920/2 + ((video_zoffset>0)?video_zoffset:0);
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2 - abs(video_zoffset);
            rc = NEXUS_VideoWindow_SetMosaicSettings(window[1], &mosaicWindowSettings);
            BDBG_ASSERT(!rc);
        }

        {
            int zoffset = graphics_zoffset;
            int scaled_zoffset = zoffset * (int)createSurfaceSettings.width / (int)(FRAMEBUFFER_WIDTH/2);

            printf("mode: L/R half with %d zoffset\n", zoffset);
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset<0?-scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = zoffset>0?zoffset:0;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreen;
            blitSettings.source.rect.x = scaled_zoffset>0?scaled_zoffset:0;
            blitSettings.source.rect.y = 0;
            blitSettings.source.rect.width = createSurfaceSettings.width - abs(scaled_zoffset);
            blitSettings.source.rect.height = createSurfaceSettings.height;
            blitSettings.output.surface = framebuffer;
            blitSettings.output.rect.x = FRAMEBUFFER_WIDTH/2 - (zoffset<0?zoffset:0);
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.width = FRAMEBUFFER_WIDTH/2 - abs(zoffset);
            blitSettings.output.rect.height = displayFormatInfo.height;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        }

prompt:
        printf("Enter video,graphics z-offset:\n");
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

        sscanf(buf, "%d,%d", &video_zoffset, &graphics_zoffset);
    }

    return 0;
}
