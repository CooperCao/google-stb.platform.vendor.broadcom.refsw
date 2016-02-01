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

/* this example demonstrates picture-in-graphics (PIG) for half-res 3D formats
   on legacy (65nm) platforms */

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_core_utils.h"
#include "nexus_hdmi_output.h"
#include "nexus_display.h"
#include "nexus_video_input.h"
#include "nexus_mosaic_display.h"
#include "nexus_graphics2d.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_window.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <string.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int main(int argc, char **argv)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle parentWindow;
    NEXUS_VideoWindowHandle window[2]; /* mosaics */
    NEXUS_VideoWindowSettings windowSettings[2];
    NEXUS_VideoWindowMosaicSettings mosaicWindowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DSettings gfxSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    bool sbs = true; /* side-by-side or over-under */
    bool started = false;

    if (argc > 1 && !strcmp(argv[1], "ou")) {
        sbs = false;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
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

    parentWindow = NEXUS_VideoWindow_Open(display, 0);

    /* open decoder now, but can't start until first connection to window */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

    /* but allow NEXUS_VideoWindow_RemoveInput without stopping decoder by setting manualPowerState.
    this prevents an implicit BXVD_CloseChannel internally. */
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.manualPowerState = true; 
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    /* bring up graphics */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    createSurfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSurfaceSettings.width = displayFormatInfo.width;
    createSurfaceSettings.height = displayFormatInfo.height;
    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    /* make whole screen transparent */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = 0x00;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = displayFormatInfo.width;
    graphicsSettings.clip.height = displayFormatInfo.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);
    
    while (1) {
        NEXUS_Rect windowRect[2];

        if (sbs) {
            windowRect[0].x = 50;
            windowRect[0].y = 100;
            windowRect[0].width = displayFormatInfo.width/3;
            windowRect[0].height = displayFormatInfo.height*2/3;
        }
        else {
            windowRect[0].x = 100;
            windowRect[0].y = 50;
            windowRect[0].width = displayFormatInfo.width*2/3;
            windowRect[0].height = displayFormatInfo.height/3;
        }
        windowRect[1] = windowRect[0];
        if (sbs) {
            windowRect[1].x = windowRect[1].x + displayFormatInfo.width / 2;
        }
        else {
            windowRect[1].y = windowRect[1].y + displayFormatInfo.height / 2;
        }

        /* do graphics first because the API doesn't block until the vsync */
        /* fill whole screen with blue */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer;
        fillSettings.color = 0xFF104080;
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        /* punch through the picture-in-graphics hole for first window */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer;
        fillSettings.rect = windowRect[0];
        fillSettings.color = 0x00cacaca;
        NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        /* second window */
        fillSettings.rect = windowRect[1];
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        /* set the source orientation to 2D since we will handle the clipping manually via mosaic windows */
        NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e2D;
        rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
        BDBG_ASSERT(!rc);

        window[0] = NEXUS_VideoWindow_OpenMosaic(parentWindow, 0);
        NEXUS_VideoWindow_GetSettings(window[0], &windowSettings[0]);
        windowSettings[0].position = windowRect[0];
        windowSettings[0].alpha = 0xFF;
        rc = NEXUS_VideoWindow_SetSettings(window[0], &windowSettings[0]);
        BDBG_ASSERT(!rc);

        /* mosaic 0 uses the left half of the video */
        NEXUS_VideoWindow_GetMosaicSettings(window[0], &mosaicWindowSettings);
        mosaicWindowSettings.backendMosaic.enabled = true;
        mosaicWindowSettings.backendMosaic.clipRect.x = 0;
        mosaicWindowSettings.backendMosaic.clipRect.y = 0;
        if (sbs) {
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2;
            mosaicWindowSettings.backendMosaic.clipRect.height = 1080;
        }
        else {
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920;
            mosaicWindowSettings.backendMosaic.clipRect.height = 1080/2;
        }
        mosaicWindowSettings.backendMosaic.clipBase.width = 1920;
        mosaicWindowSettings.backendMosaic.clipBase.height = 1080;
        rc = NEXUS_VideoWindow_SetMosaicSettings(window[0], &mosaicWindowSettings);
        BDBG_ASSERT(!rc);

        window[1] = NEXUS_VideoWindow_OpenMosaic(parentWindow, 1);
        NEXUS_VideoWindow_GetSettings(window[1], &windowSettings[1]);
        windowSettings[1].position = windowRect[1];
        windowSettings[1].alpha = 0xFF;
        rc = NEXUS_VideoWindow_SetSettings(window[1], &windowSettings[1]);
        BDBG_ASSERT(!rc);

        /* mosaic 1 uses the right half of the video */
        NEXUS_VideoWindow_GetMosaicSettings(window[1], &mosaicWindowSettings);
        mosaicWindowSettings.backendMosaic.enabled = true;
        if (sbs) {
            mosaicWindowSettings.backendMosaic.clipRect.x = 1920/2;
            mosaicWindowSettings.backendMosaic.clipRect.y = 0;
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920/2;
            mosaicWindowSettings.backendMosaic.clipRect.height = 1080;
        }
        else {
            mosaicWindowSettings.backendMosaic.clipRect.x = 0;
            mosaicWindowSettings.backendMosaic.clipRect.y = 1080/2;
            mosaicWindowSettings.backendMosaic.clipRect.width = 1920;
            mosaicWindowSettings.backendMosaic.clipRect.height = 1080/2;
        }
        mosaicWindowSettings.backendMosaic.clipBase.width = 1920;
        mosaicWindowSettings.backendMosaic.clipBase.height = 1080;
        rc = NEXUS_VideoWindow_SetMosaicSettings(window[1], &mosaicWindowSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_VideoWindow_AddInput(window[0], NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoWindow_AddInput(window[1], NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);

        if (!started) {
            rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            BDBG_ASSERT(!rc);
            started = true;
        }

        printf("Press ENTER for full screen mode\n");
        getchar();

        /* make whole screen transparent */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer;
        fillSettings.color = 0x00;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        /* tear down backend mosaic */
        NEXUS_VideoWindow_RemoveAllInputs(window[0]);
        NEXUS_VideoWindow_RemoveAllInputs(window[1]);
        NEXUS_VideoWindow_Close(window[0]);
        NEXUS_VideoWindow_Close(window[1]);
        
        NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.customSourceOrientation = false;
        rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
        BDBG_ASSERT(!rc);

        /* set up main window */
        rc = NEXUS_VideoWindow_AddInput(parentWindow, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);

        printf("Press ENTER for PIG mode\n");
        getchar();

        /* tear down main window */
        NEXUS_VideoWindow_RemoveAllInputs(parentWindow);
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    }

    return 0;
}
