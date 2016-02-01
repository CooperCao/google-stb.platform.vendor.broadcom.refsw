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

/* nexus example that demonstrates switching between half-res 3D formats
   and how to handle video window sizing */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_input.h"
#include "nexus_stc_channel.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_video_window.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "bstd.h"
#include "bkni.h"
#include <string.h>


#define FULLSCREEN_VIDEOWINDOW 0 /* if 1, the video window fills the entire display.
                                    if 0, the video window is smaller than the display, so the L/R or O/U halves can be distinguished better */
#define MARGIN 100
#define ENABLE_GRAPHICS 1 /* enable 2D graphics when FULLSCREEN_VIDEOWINDOW is 0 */

int main(void)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_VideoInput videoInput;
    NEXUS_VideoInputSettings videoInputSettings;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings initWindowSettings, windowSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo displayFormatInfo;
#if (!FULLSCREEN_VIDEOWINDOW && ENABLE_GRAPHICS)
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_GraphicsSettings graphicsSettings;
#endif
    NEXUS_Error rc;
    int i;
    
    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* streamer input */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eH264;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p; /* initialize the display in 2D mode */
    display = NEXUS_Display_Open(0, &displaySettings);
    rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!rc);
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
#endif
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

    window = NEXUS_VideoWindow_Open(display, 0);
    /* bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.fifoSize = 3*1024*1024; /* this is stream-dependent */
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);

    /* initialize the videoInput in 2D mode */
    videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);    
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_VideoWindow_GetSettings(window, &initWindowSettings);
#if (!FULLSCREEN_VIDEOWINDOW)
    initWindowSettings.position.x = MARGIN;
    initWindowSettings.position.y = MARGIN;
    initWindowSettings.position.width = displayFormatInfo.width - MARGIN*2;
    initWindowSettings.position.height = displayFormatInfo.height - MARGIN*2;
    rc = NEXUS_VideoWindow_SetSettings(window, &initWindowSettings);
    BDBG_ASSERT(!rc);
#endif
    windowSettings = initWindowSettings;

#if (!FULLSCREEN_VIDEOWINDOW && ENABLE_GRAPHICS)
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width = displayFormatInfo.width;
    surfaceCreateSettings.height = displayFormatInfo.height;
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);

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
    graphicsSettings.clip.width = surfaceCreateSettings.width;
    graphicsSettings.clip.height = surfaceCreateSettings.height;
    rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);
#endif

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    for (;;) {       
        BKNI_Printf("window size: %ux%u\n", windowSettings.position.width, windowSettings.position.height);
        BKNI_Printf("options: 0: 2D, 1: SBS, 2: TAB\n");
        scanf("%d", &i);
       
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_VideoInput_GetSettings(videoInput, &videoInputSettings);
        if (i==0) {
            videoInputSettings.video3DSettings.overrideOrientation = false;
        }
        else if (i==1) {
            videoInputSettings.video3DSettings.overrideOrientation = true;
            videoInputSettings.video3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
        }
        else {
            videoInputSettings.video3DSettings.overrideOrientation = true;
            videoInputSettings.video3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
        }
        rc = NEXUS_VideoInput_SetSettings(videoInput, &videoInputSettings);
        BDBG_ASSERT(!rc);

        NEXUS_Display_GetSettings(display, &displaySettings);
        if (i==0) {
            displaySettings.display3DSettings.overrideOrientation = false;
        }
        else if (i==1) {
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
        }
        else {
            displaySettings.display3DSettings.overrideOrientation = true;
            displaySettings.display3DSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
        }          
        rc = NEXUS_Display_SetSettings(display, &displaySettings);
        BDBG_ASSERT(!rc);

#if (!FULLSCREEN_VIDEOWINDOW)
        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
        if (i==0) {
            windowSettings.position.x = initWindowSettings.position.x;
            windowSettings.position.y = initWindowSettings.position.y;
            windowSettings.position.width = initWindowSettings.position.width;
            windowSettings.position.height = initWindowSettings.position.height;
        }
        else if (i==1) {
            windowSettings.position.x = initWindowSettings.position.x/2;
            windowSettings.position.y = initWindowSettings.position.y;
            windowSettings.position.width = initWindowSettings.position.width/2;
            windowSettings.position.height = initWindowSettings.position.height;
        }
        else {
            windowSettings.position.x = initWindowSettings.position.x;
            windowSettings.position.y = initWindowSettings.position.y/2;
            windowSettings.position.width = initWindowSettings.position.width;
            windowSettings.position.height = initWindowSettings.position.height/2;
        }
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);
#else
        /* if video window is fullscreen, NEXUS_Display_SetSettings() updates the window sizes automatically */
        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
#endif

#if (!FULLSCREEN_VIDEOWINDOW && ENABLE_GRAPHICS)
        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = surfaceCreateSettings.width;
        graphicsSettings.clip.height = surfaceCreateSettings.height;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
        BDBG_ASSERT(!rc);
#endif

        rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
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
