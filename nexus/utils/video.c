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
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if !NEXUS_HAS_VIDEO_DECODER
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#else
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif
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
#include <string.h>

#include "cmdline_args.h"

BDBG_MODULE(video);

#include "hotplug.c"

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle pcrPidChannel, videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    struct util_opts_t opts;
    NEXUS_Error rc;
    bool is_digital = false;
    NEXUS_VideoInput videoInput;
#if NEXUS_HAS_HDMI_INPUT
#if NEXUS_NUM_HDMI_INPUTS > 0
    NEXUS_HdmiInputHandle hdmi0;
#endif
#if NEXUS_NUM_HDMI_INPUTS > 1
    NEXUS_HdmiInputHandle hdmi1;
#endif
#endif
    NEXUS_VideoFormatInfo displayFormatInfo;
#if NEXUS_HAS_HDMI_OUTPUT
    struct hotplug_context hotplug_context;
#endif

    if (cmdline_parse(argc, argv, &opts)) {
        return 0;
    }
    if (cmdline_probe(&opts.common, opts.filename, &opts.indexname)) {
        return 1;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    rc = NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    BDBG_ASSERT(!rc);

    /* Map a parser band to the streamer input band */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = opts.common.transportType;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    /* Open the audio and video pid channels */
    pcrPidChannel = NEXUS_PidChannel_Open(parserBand, opts.common.pcrPid?opts.common.pcrPid:opts.common.videoPid, NULL);
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, opts.common.videoPid, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.common.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.frameRate = opts.common.videoFrameRate;
    videoProgram.aspectRatio = opts.common.aspectRatio;
    videoProgram.sampleAspectRatio.x = opts.common.sampleAspectRatio.x;
    videoProgram.sampleAspectRatio.y = opts.common.sampleAspectRatio.y;

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);

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
#if NEXUS_HAS_HDMI_INPUT
#if NEXUS_NUM_HDMI_INPUTS > 0
    hdmi0 = NEXUS_HdmiInput_Open(0, NULL);
#endif
#if NEXUS_NUM_HDMI_INPUTS > 1
    hdmi1 = NEXUS_HdmiInput_Open(1, NULL);
#endif
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
        hotplug_callback(&hotplug_context,0);
    }
#endif

    if (opts.graphics) {
        NEXUS_SurfaceHandle framebuffer;
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_GraphicsSettings graphicsSettings;
        unsigned i,j;

        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 720;
        surfaceCreateSettings.height = displayFormatInfo.height;
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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    printf("no input connected. type ? for help.\n");
    while (1)
    {
        char buf[256];
        int x, y, width, height;

        printf("video>"); fflush(stdout);
        fgets(buf, 256, stdin);
        buf[strlen(buf)-1] = 0; /* chop off \n */

        NEXUS_VideoWindow_GetSettings(window, &windowSettings);

        if (strstr(buf, "help") || !strcmp(buf, "?")) {
            printf(
            "Commands:\n"
            "  input(cvbs0|cvbs1|svideo0|svideo1|component0|component1|hdmi0|hdmi1|digital|vga)\n"
            "  position(x,y,width,height)\n"
            "  clip(clipRect.x,clipRect.y,clipRect.width,clipRect.height [,baseRect.width,baseRect.height])\n"
            "  clip%%(horz %%, vert %%) - floating point supported\n"
            "  contentMode(box|zoom|full)\n"
            "  autopq(true|false)\n"
            "  visible(true|false)\n"
            "  cti(x)\n"
            "  q - quit\n"
            );
        }
        else if (strstr(buf, "quit") || !strcmp(buf, "q")) {
            break;
        }
        else if (strstr(buf, "autopq")) {
            rc = NEXUS_DisplayModule_SetAutomaticPictureQuality();
            if (rc) BERR_TRACE(rc);
        }
        else if (strstr(buf, "cti(")) {
            rc = sscanf(&buf[4], "%d", &x);
            if (rc == 1) {
                NEXUS_VideoWindowCoefficientIndexSettings ctiSettings;
                NEXUS_VideoWindow_GetCoefficientIndexSettings(window, &ctiSettings);
                ctiSettings.sclHorzLuma    = x;
                ctiSettings.sclVertLuma    = x;
                ctiSettings.sclHorzChroma  = x;
                ctiSettings.sclVertChroma  = x;
                ctiSettings.hsclHorzLuma   = x;
                ctiSettings.hsclHorzChroma = x;
                ctiSettings.madHorzLuma    = x;
                ctiSettings.madHorzChroma  = x;
                rc = NEXUS_VideoWindow_SetCoefficientIndexSettings(window, &ctiSettings);
                if (rc) BERR_TRACE(rc);
            }
        }
        else if (strstr(buf, "visible(")) {
            char *input = &buf[8];
            windowSettings.visible = input[0] == 't';
            rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
            if (rc) BERR_TRACE(rc);
        }
        else if (strstr(buf, "position(")) {
            rc = sscanf(&buf[9], "%d,%d,%d,%d", &x, &y, &width, &height);
            if (rc == 4) {
                windowSettings.position.x = x;
                windowSettings.position.y = y;
                windowSettings.position.width = width;
                windowSettings.position.height = height;
                rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
                if (rc) BERR_TRACE(rc);
            }
        }
        else if (strstr(buf, "clip%(")) {
            float w,h;
            rc = sscanf(&buf[6], "%f,%f", &w, &h);
            if (rc == 2) {
                NEXUS_CalculateVideoWindowPositionSettings pos;
                NEXUS_VideoWindowContentMode contentMode = windowSettings.contentMode;

                NEXUS_GetDefaultCalculateVideoWindowPositionSettings(&pos);
                pos.horizontalClipping = w * 100;
                pos.verticalClipping = h * 100;
                pos.viewport = windowSettings.position;
                pos.displayWidth = displayFormatInfo.width;
                pos.displayHeight = displayFormatInfo.height;
                rc = NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
                if (rc) BERR_TRACE(rc);

                /* restore the contentMode. we don't support manual a/r, so don't let the helper func override */
                windowSettings.contentMode = contentMode;

                rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
                if (rc) BERR_TRACE(rc);
            }
        }
        else if (strstr(buf, "clip(")) {
            int base_width, base_height;
            rc = sscanf(&buf[5], "%d,%d,%d,%d,%d,%d", &x, &y, &width, &height, &base_width, &base_height);
            if (rc >= 4) {
                windowSettings.clipRect.x = x;
                windowSettings.clipRect.y = y;
                windowSettings.clipRect.width = width;
                windowSettings.clipRect.height = height;
                windowSettings.clipBase.x = 0;
                windowSettings.clipBase.y = 0;
                if (rc > 5) {
                    windowSettings.clipBase.width = base_width;
                    windowSettings.clipBase.height = base_height;
                }
                else {
                    windowSettings.clipBase.width = displayFormatInfo.width;
                    windowSettings.clipBase.height = displayFormatInfo.height;
                }
                rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
                if (rc) BERR_TRACE(rc);
            }
        }
        else if (strstr(buf, "contentMode(")) {
            char *input = &buf[12];

            if (strstr(input, "zoom")) {
                windowSettings.contentMode = NEXUS_VideoWindowContentMode_eZoom;
            }
            else if (strstr(input, "box")) {
                windowSettings.contentMode = NEXUS_VideoWindowContentMode_eBox;
            }
            else if (strstr(input, "full")) {
                windowSettings.contentMode = NEXUS_VideoWindowContentMode_eFull;
            }

            rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
            if (rc) BERR_TRACE(rc);
        }
        else if (strstr(buf, "input(")) {
            char *input = &buf[6];

            if (is_digital) {
                NEXUS_VideoDecoder_Stop(videoDecoder);
            }
            NEXUS_VideoWindow_RemoveAllInputs(window);
            videoInput = NULL;
            is_digital = false;

            if (0) {}
#if NEXUS_HAS_HDMI_INPUT
#if NEXUS_NUM_HDMI_INPUTS > 0
            else if (strstr(input, "hdmi0")) {
                videoInput = NEXUS_HdmiInput_GetVideoConnector(hdmi0);
            }
#endif
#if NEXUS_NUM_HDMI_INPUTS > 1
            else if (strstr(input, "hdmi1")) {
                videoInput = NEXUS_HdmiInput_GetVideoConnector(hdmi1);
            }
#endif
#endif
            else if (strstr(input, "digital") || strstr(input, "digital0")) {
                videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
                is_digital = true;
            }

            if (videoInput) {
                rc = NEXUS_VideoWindow_AddInput(window, videoInput);
                if (rc) BERR_TRACE(rc);
            }

            if (is_digital) {
                rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
                BDBG_ASSERT(!rc);
            }
        }
        else if (!*buf) {
            /* allow blank line */
        }
        else {
            printf("unknown command: '%s' (use 'help' for list)\n", buf);
            continue;
        }
    }

    return 0;
}

#endif /* !NEXUS_HAS_VIDEO_DECODER  */

/*
************************************************

examples / test cases

# basic decode of external input
nexus video

************************************************
*/
