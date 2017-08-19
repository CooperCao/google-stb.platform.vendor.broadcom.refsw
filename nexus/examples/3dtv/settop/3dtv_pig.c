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

/**
3dtv_pig.c demonstrates picture-in-graphics (PIG) for half and 
full resolution 3D formats. For full-res, MVC decode is used for video.

All examples under nexus/examples/3dtv require the new BVN in 40nm 
platforms and supporting SW.
**/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_input.h"
#include "nexus_stc_channel.h"
#include "nexus_video_window.h"
#include "nexus_core_utils.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <string.h>

typedef enum mode3d {
    mode3d_eHalfresSbs,
    mode3d_eHalfresTab,
    mode3d_eFullres
} mode3d;

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eH264
#define FULL_RES_CODEC NEXUS_VideoCodec_eH264_Mvc
#define VIDEO_PID 0x31
#define FULL_RES_PID 0x32

int main(int argc, char **argv)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoInput videoInput;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo displayFormatInfo;
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_SurfaceCreateSettings createSurfaceSettings;
    NEXUS_GraphicsFramebuffer3D framebuffer3d;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings graphicsSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    mode3d mode = mode3d_eHalfresSbs;

    if (argc > 1) {
        if (!strcmp(argv[1], "tab")) {
            mode = mode3d_eHalfresTab;
        }
        else if (!strcmp(argv[1], "fullres")) {
            mode = mode3d_eFullres;
        }
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Bringup display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    switch (mode) {
        case mode3d_eHalfresSbs:
            displaySettings.format = NEXUS_VideoFormat_e720p;
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            break;
        case mode3d_eHalfresTab:
            displaySettings.format = NEXUS_VideoFormat_e720p;
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
        case mode3d_eFullres:
            displaySettings.format = NEXUS_VideoFormat_e720p_3DOU_AS;
            displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eHdmiDvo; /* HDMI master mode is required */
            break;
        default:
            break;
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
        NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
        NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(platformConfig.outputs.hdmi[0], &vsi);
        switch (mode) {
        default: /* unused */
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone;
            break;
        case mode3d_eHalfresSbs:
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf;
            break;
        case mode3d_eFullres:
        case mode3d_eHalfresTab:
            vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
            vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eTopAndBottom;
            break;
        }
        NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(platformConfig.outputs.hdmi[0], &vsi);
    }
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    /* Map input band and parser band */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    /* TODO: separate source and display settings */
    if (mode == mode3d_eFullres) {
        videoProgram.codec = FULL_RES_CODEC;
        videoProgram.enhancementPidChannel = NEXUS_PidChannel_Open(parserBand, FULL_RES_PID, NULL);
    }

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.fifoSize = 10*1024*1024; /* this is stream-dependent */
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    /* tell the decoder about the orientation of the source */
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    /* TODO: separate source and display settings */
    if (mode == mode3d_eFullres) {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame;
    }
    else {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight;
    }
    NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    
    videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);
    
    /* bring up graphics */
    NEXUS_Surface_GetDefaultCreateSettings(&createSurfaceSettings);
    switch (mode) {
        case mode3d_eHalfresSbs:
            createSurfaceSettings.width = displayFormatInfo.width/2;
            createSurfaceSettings.height = displayFormatInfo.height;
            break;
        case mode3d_eHalfresTab:
            createSurfaceSettings.width = displayFormatInfo.width;
            createSurfaceSettings.height = displayFormatInfo.height/2;
            break;
        case mode3d_eFullres:
            createSurfaceSettings.width = displayFormatInfo.width;
            createSurfaceSettings.height = displayFormatInfo.height;
            break;
    }

    createSurfaceSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSurfaceSettings);

    /* set window position */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    switch (mode) {
        case mode3d_eHalfresSbs:
            windowSettings.position.x = 50;
            windowSettings.position.y = 100;
            windowSettings.position.width = displayFormatInfo.width/3;
            windowSettings.position.height = displayFormatInfo.height*2/3;
            break;
        case mode3d_eHalfresTab:
            windowSettings.position.x = 100;
            windowSettings.position.y = 50;
            windowSettings.position.width = displayFormatInfo.width*2/3;
            windowSettings.position.height = displayFormatInfo.height/3;
            break;
        case mode3d_eFullres:
            windowSettings.position.x = 50;
            windowSettings.position.y = 50;
            windowSettings.position.width = createSurfaceSettings.width/2;
            windowSettings.position.height = createSurfaceSettings.height/2;
            break;
    }
    windowSettings.alpha = 0xFF;
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    /* fill whole screen with blue */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color = 0xFF104080;

    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    /* punch through the picture-in-graphics hole */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.rect = windowSettings.position;
    fillSettings.color = 0x00cacaca;
    NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    NEXUS_Graphics2D_Checkpoint(gfx, NULL);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.clip.width = createSurfaceSettings.width;
    graphicsSettings.clip.height = createSurfaceSettings.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Graphics_GetDefaultFramebuffer3D(&framebuffer3d);
    /* same framebuffer for both eyes */
    framebuffer3d.main = framebuffer;
    framebuffer3d.right = framebuffer;
    switch (mode) {
        case mode3d_eHalfresSbs:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            break;
        case mode3d_eHalfresTab:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
        case mode3d_eFullres:
            framebuffer3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            break;
    }
    rc = NEXUS_Display_SetGraphicsFramebuffer3D(display, &framebuffer3d);
    BDBG_ASSERT(!rc);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    for (;;) {
        NEXUS_VideoDecoderStatus status;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        BKNI_Sleep(1000);
    }

    getchar();
    return 0;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
