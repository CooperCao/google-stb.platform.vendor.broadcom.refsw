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

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID_0 0x11 /* pid of first program */
#define VIDEO_PID_1 0x21 /* pid of second program */

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode0, vdecode1;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand0 = NEXUS_ParserBand_e0;
    NEXUS_ParserBand parserBand1 = NEXUS_ParserBand_e1;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display0, display1;
    NEXUS_VideoWindowHandle window00, window01, window10, window11;
    NEXUS_VideoDecoderStartSettings videoProgram0, videoProgram1;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* It's important to use two parser bands so that the default maxDataRate of 25 Mbps per parser band will not be exceeded. */
    NEXUS_ParserBand_GetSettings(parserBand0, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand0, &parserBandSettings);

    NEXUS_ParserBand_GetSettings(parserBand1, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand1, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram0);
    videoProgram0.codec = VIDEO_CODEC;
    videoProgram0.pidChannel = NEXUS_PidChannel_Open(parserBand0, VIDEO_PID_0, NULL);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram1);
    videoProgram1.codec = VIDEO_CODEC;
    videoProgram1.pidChannel = NEXUS_PidChannel_Open(parserBand1, VIDEO_PID_1, NULL);

    /* Open two StcChannels */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram0.pidChannel; /* PCR on video pid */
    videoProgram0.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram1.pidChannel; /* PCR on video pid */
    videoProgram1.stcChannel = NEXUS_StcChannel_Open(1, &stcSettings);

    /* Bring up video display and outputs */
    display0 = NEXUS_Display_Open(0, NULL);
    window00 = NEXUS_VideoWindow_Open(display0, 0);
    window01 = NEXUS_VideoWindow_Open(display0, 1);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display0, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display0, &displaySettings);
		}
    }
#endif

    display1 = NEXUS_Display_Open(1, NULL);
    if (!display1) {
        printf("This is not a HD/SD simul capable platform.\n");
        exit(1);
    }

    window10 = NEXUS_VideoWindow_Open(display1, 0);
    window11 = NEXUS_VideoWindow_Open(display1, 1);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    rc = NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    BDBG_ASSERT(!rc);
#endif

    vdecode0 = NEXUS_VideoDecoder_Open(0, NULL);
    vdecode1 = NEXUS_VideoDecoder_Open(1, NULL);
    if (!vdecode1) {
        printf("This is not a dual decode capable platform.\n");
        exit(1);
    }

    rc = NEXUS_VideoWindow_AddInput(window00, NEXUS_VideoDecoder_GetConnector(vdecode0));
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_AddInput(window01, NEXUS_VideoDecoder_GetConnector(vdecode1));
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_AddInput(window10, NEXUS_VideoDecoder_GetConnector(vdecode0));
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_AddInput(window11, NEXUS_VideoDecoder_GetConnector(vdecode1));
    BDBG_ASSERT(!rc);

    /* Start Decoders */
    rc = NEXUS_VideoDecoder_Start(vdecode0, &videoProgram0);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoDecoder_Start(vdecode1, &videoProgram1);
    BDBG_ASSERT(!rc);

    for (;;) {
        NEXUS_VideoDecoderStatus status;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode0, &status);
        NEXUS_StcChannel_GetStc(videoProgram0.stcChannel, &stc);
        printf("decode0 %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        NEXUS_VideoDecoder_GetStatus(vdecode1, &status);
        NEXUS_StcChannel_GetStc(videoProgram1.stcChannel, &stc);
        printf("decode1 %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);

        BKNI_Sleep(1000);
    }

    getchar();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
