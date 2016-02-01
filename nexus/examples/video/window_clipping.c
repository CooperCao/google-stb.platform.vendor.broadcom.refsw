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
******************************************************************************/
/* Nexus example app: positioning a virtual window via source clipping */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_GRAPHICS2D
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_core_utils.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include "nexus_video_window.h"
#include "bstd.h"
#include "bkni.h"

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31

BDBG_MODULE(window_clipping);

#define SPACING 200

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_Rect windowPos;
    NEXUS_CalculateVideoWindowPositionSettings pos;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    unsigned i, step = 4;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. */
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
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS || NEXUS_NUM_HDMI_OUTPUTS
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
#else
    BDBG_ERR(("this example is writing for HD systems"));
    return -1;
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.position.x = 0;
    windowSettings.position.y = 0;
    windowSettings.position.width = videoFormatInfo.width;
    windowSettings.position.height = videoFormatInfo.height;
    windowSettings.alpha = 0xFF;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);

    windowPos = windowSettings.position; /* save original window position. we'll need this for NEXUS_VideoWindowSettings.clipBase */

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipBase = windowPos; /* clipping done in window coordinates */
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);

    printf("Press Enter to start manual examples.\n");
    getchar();

    /**
    These examples show how to do use clipping and window positioning to move a video window
    virtually off the edge of the screen.
    **/

    /* position at (+a, +a) */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipBase = windowPos; /* clipping done in window coordinates */
    windowSettings.clipRect.x = 0;
    windowSettings.clipRect.y = 0;
    windowSettings.clipRect.width = videoFormatInfo.width - SPACING;
    windowSettings.clipRect.height = videoFormatInfo.height - SPACING;
    windowSettings.position.x += SPACING;
    windowSettings.position.y += SPACING;
    windowSettings.position.width = videoFormatInfo.width - SPACING;
    windowSettings.position.height = videoFormatInfo.height - SPACING;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    /* position at (-a, +a) */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipBase = windowPos;
    windowSettings.clipRect.x = SPACING;
    windowSettings.clipRect.y = 0;
    windowSettings.clipRect.width = videoFormatInfo.width - SPACING;
    windowSettings.clipRect.height = videoFormatInfo.height - SPACING;
    windowSettings.position.x = 0;
    windowSettings.position.y = SPACING;
    windowSettings.position.width = videoFormatInfo.width - SPACING;
    windowSettings.position.height = videoFormatInfo.height - SPACING;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    /* position at (-a, -a) */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipBase = windowPos;
    windowSettings.clipRect.x = SPACING;
    windowSettings.clipRect.y = SPACING;
    windowSettings.clipRect.width = videoFormatInfo.width - SPACING;
    windowSettings.clipRect.height = videoFormatInfo.height - SPACING;
    windowSettings.position.x = 0;
    windowSettings.position.y = 0;
    windowSettings.position.width = videoFormatInfo.width - SPACING;
    windowSettings.position.height = videoFormatInfo.height - SPACING;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    /* position at (+a, -a) */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipBase = windowPos;
    windowSettings.clipRect.x = 0;
    windowSettings.clipRect.y = SPACING;
    windowSettings.clipRect.width = videoFormatInfo.width - SPACING;
    windowSettings.clipRect.height = videoFormatInfo.height - SPACING;
    windowSettings.position.x = SPACING;
    windowSettings.position.y = 0;
    windowSettings.position.width = videoFormatInfo.width - SPACING;
    windowSettings.position.height = videoFormatInfo.height - SPACING;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    /* back to (0, 0) */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipRect.x = 0;
    windowSettings.clipRect.y = 0;
    windowSettings.clipRect.width = 0;
    windowSettings.clipRect.height = 0;
    windowSettings.position.x = 0;
    windowSettings.position.y = 0;
    windowSettings.position.width = videoFormatInfo.width;
    windowSettings.position.height = videoFormatInfo.height;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipRect.x       = videoFormatInfo.width * 625 / 10000;  /* 6.25% on each side */
    windowSettings.clipRect.y       = 0;
    windowSettings.clipRect.width   = videoFormatInfo.width;
    windowSettings.clipRect.width  -= 2 * (videoFormatInfo.width * 625 / 10000);
    windowSettings.clipRect.height  = videoFormatInfo.height;
    windowSettings.position.x       = 0;
    windowSettings.position.y       = 0;
    windowSettings.position.width   = videoFormatInfo.width;
    windowSettings.position.height  = videoFormatInfo.height;
    windowSettings.contentMode      = NEXUS_VideoWindowContentMode_eBox;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BKNI_Sleep(1000);

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.clipRect.x       = 0;
    windowSettings.clipRect.y       = 0;
    windowSettings.clipRect.width   = videoFormatInfo.width;
    windowSettings.clipRect.height  = videoFormatInfo.height;
    windowSettings.position.x       = 0;
    windowSettings.position.y       = 0;
    windowSettings.position.width   = videoFormatInfo.width;
    windowSettings.position.height  = videoFormatInfo.height;
    windowSettings.contentMode      = NEXUS_VideoWindowContentMode_eFull;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);

    printf("Press Enter to start NEXUS_CalculateVideoWindowPosition examples\n");
    getchar();

    /**
    These examples show how to do the same clipping and window positioning using the
    NEXUS_CalculateVideoWindowPosition helper function.
    **/
    NEXUS_GetDefaultCalculateVideoWindowPositionSettings(&pos);
    pos.viewport.x = 0;
    pos.viewport.y = 0;
    pos.viewport.width = videoFormatInfo.width;
    pos.viewport.height = videoFormatInfo.height;
    pos.displayWidth = videoFormatInfo.width;
    pos.displayHeight = videoFormatInfo.height;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    /* iterate throught the same steps with NEXUS_CalculateVideoWindowPosition */
    pos.viewport.x = SPACING;
    pos.viewport.y = SPACING;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(1000);

    pos.viewport.x = -SPACING;
    pos.viewport.y = SPACING;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(1000);

    pos.viewport.x = -SPACING;
    pos.viewport.y = -SPACING;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(1000);

    pos.viewport.x = SPACING;
    pos.viewport.y = -SPACING;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(1000);

    pos.viewport.x = 0;
    pos.viewport.y = 0;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    printf("Press Enter for slide examples\n");
    getchar();

    printf("Sliding to the left\n");
    pos.viewport.x = 0;
    pos.viewport.y = 0;
    for(i = 0; i < videoFormatInfo.width/2; i += step)
    {
        pos.viewport.x -= step;
        rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);
    }

    printf("Sliding to the right\n");
    pos.viewport.x = 0;
    pos.viewport.y = 0;
    for(i = 0; i < videoFormatInfo.width/2; i += step)
    {
        pos.viewport.x += step;
        rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);
    }

    printf("Sliding to the up\n");
    pos.viewport.x = 0;
    pos.viewport.y = 0;
    for(i = 0; i < videoFormatInfo.height/2; i += step)
    {
        pos.viewport.y -= step;
        rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);
    }

    printf("Sliding to the down\n");
    pos.viewport.x = 0;
    pos.viewport.y = 0;
    for(i = 0; i < videoFormatInfo.height/2; i += step)
    {
        pos.viewport.y += step;
        rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);
    }

    pos.viewport.x = 0;
    pos.viewport.y = 0;
    rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    printf("Back to full screen. Press Enter to finish.\n");
    getchar();

    /* Shutdown */
    NEXUS_VideoDecoder_Stop(vdecode);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(videoProgram.stcChannel);
    NEXUS_PidChannel_Close(videoProgram.pidChannel);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
