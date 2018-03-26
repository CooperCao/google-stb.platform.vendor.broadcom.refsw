/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_hdmi_output.h"
#include "nexus_display.h"
#include "nexus_video_input.h"
#include "nexus_mosaic_display.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_window.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <string.h>

BDBG_MODULE(backend_mosaic);

#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x31

int main(void)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle parentWindow;
#define NUM_MOSAIC 4
    NEXUS_VideoWindowHandle window[NUM_MOSAIC]; /* mosaics */
    NEXUS_VideoWindowSettings windowSettings[NUM_MOSAIC];
    NEXUS_Rect rect[NUM_MOSAIC];
    NEXUS_VideoWindowMosaicSettings mosaicWindowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    unsigned i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* cut up 1920x1080 into 4 tiles */
    for (i=0;i<NUM_MOSAIC;i++) {
        rect[i].x = i%2?960:0;
        rect[i].y = i<2?0:540;
        rect[i].width = 960;
        rect[i].height = 540;
    }

    /* streamer input */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
#endif

    parentWindow = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);

    for (i=0;i<NUM_MOSAIC;i++) {
        window[i] = NEXUS_VideoWindow_OpenMosaic(parentWindow, i);

        NEXUS_VideoWindow_GetSettings(window[i], &windowSettings[i]);
        windowSettings[i].position = rect[i];
        rc = NEXUS_VideoWindow_SetSettings(window[i], &windowSettings[i]);
        BDBG_ASSERT(!rc);

        NEXUS_VideoWindow_GetMosaicSettings(window[i], &mosaicWindowSettings);
        mosaicWindowSettings.backendMosaic.enabled = true;
        /* show backendMosaic by rearranging tiles in clockwise direction */
        {
        const unsigned remapping[] = {2,0,3,1};
        mosaicWindowSettings.backendMosaic.clipRect = rect[remapping[i]];
        }
        mosaicWindowSettings.backendMosaic.clipBase.width = 1920;
        mosaicWindowSettings.backendMosaic.clipBase.height = 1080;
        rc = NEXUS_VideoWindow_SetMosaicSettings(window[i], &mosaicWindowSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_VideoWindow_AddInput(window[i], NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);
    }

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    BDBG_ASSERT(!rc);

    BDBG_LOG(("Press ENTER to exit"));
    getchar();

    for (i=0;i<NUM_MOSAIC;i++) {
        NEXUS_VideoWindow_Close(window[i]);
    }
    NEXUS_VideoWindow_Close(parentWindow);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_PidChannel_Close(videoProgram.pidChannel);
    NEXUS_StcChannel_Close(videoProgram.stcChannel);
    NEXUS_Platform_Uninit();

    return 0;
}
