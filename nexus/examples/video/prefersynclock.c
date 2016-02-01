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
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(prefersynclock);

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID_0 0x11
#define VIDEO_PID_1 0x31

void wait_for_next(void)
{
#if 0
    /* interactive mode */
    printf("Press ENTER to switch\n");
    getchar();
#else
    printf("Switch in 5 seconds...\n");
    BKNI_Sleep(5000);
#endif
}

static void print_status(NEXUS_VideoWindowHandle window0, NEXUS_VideoWindowHandle window1,
    unsigned expectPrefer0, unsigned expectIsLocked0,
    unsigned expectPrefer1, unsigned expectIsLocked1)
{
    NEXUS_VideoWindowStatus status0, status1;
    NEXUS_VideoWindowSettings settings0, settings1;
    int rc;

    /* export msg_modules=BVDC_SRC */
    rc = NEXUS_VideoWindow_GetStatus(window0, &status0);
    BDBG_ASSERT(!rc);
    rc = NEXUS_VideoWindow_GetStatus(window1, &status1);
    BDBG_ASSERT(!rc);
    NEXUS_VideoWindow_GetSettings(window0, &settings0);
    NEXUS_VideoWindow_GetSettings(window1, &settings1);

    BDBG_WRN(("expect: win0=%d,%d win1=%d,%d",
        expectPrefer0, expectIsLocked0,
        expectPrefer1, expectIsLocked1));
    BDBG_WRN(("actual: win0=%d,%d win1=%d,%d",
        settings0.preferSyncLock, status0.isSyncLocked,
        settings1.preferSyncLock, status1.isSyncLocked));
    BDBG_ASSERT(settings0.preferSyncLock == expectPrefer0);
    BDBG_ASSERT(status0.isSyncLocked     == expectIsLocked0);
    BDBG_ASSERT(settings1.preferSyncLock == expectPrefer1);
    BDBG_ASSERT(status1.isSyncLocked     == expectIsLocked1);
    wait_for_next();
}

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode0, vdecode1 = NULL;
    NEXUS_VideoDecoderStartSettings videoProgram0, videoProgram1;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand0 = NEXUS_ParserBand_e0;
    NEXUS_ParserBand parserBand1 = NEXUS_ParserBand_e1;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display0, display1;
    NEXUS_VideoWindowHandle window0, window1;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoWindowSettings settings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error errCode;
#endif
    int rc;

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

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram0.pidChannel; /* PCR on video pid */
    videoProgram0.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram1.pidChannel; /* PCR on video pid */
    videoProgram1.stcChannel = NEXUS_StcChannel_Open(1, &stcSettings);

    BDBG_WRN(("Scenario #1: two decoders to one display"));
    
    display0 = NEXUS_Display_Open(0, NULL);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display0, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    errCode = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !errCode && hdmiStatus.connected )
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

    window0 = NEXUS_VideoWindow_Open(display0, 0);
    window1 = NEXUS_VideoWindow_Open(display0, 1);

    vdecode0 = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode0));
    vdecode1 = NEXUS_VideoDecoder_Open(1, NULL);
    NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode1));

    NEXUS_VideoDecoder_Start(vdecode0, &videoProgram0);
    NEXUS_VideoDecoder_Start(vdecode1, &videoProgram1);

    /* this is the default */
    print_status(window0, window1, 0, 1, 0, 0);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* switch to PIP */
    print_status(window0, window1, 0, 0, 1, 1);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = false;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* setting to false does not change anything */
    print_status(window0, window1, 0, 0, 0, 1);

    NEXUS_VideoWindow_GetSettings(window0, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window0, &settings);
    BKNI_Sleep(1000);BDBG_ASSERT(!rc);

    /* switch back to main */
    print_status(window0, window1, 1, 1, 0, 0);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* duplicate preferSyncLock = true should result in harmless warning */
    print_status(window0, window1, 1, 1, 1, 0);
    
    NEXUS_VideoDecoder_Stop(vdecode0);
    NEXUS_VideoDecoder_Stop(vdecode1);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode0));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode1));
    NEXUS_VideoDecoder_Close(vdecode1);
    NEXUS_VideoWindow_Close(window1);
    
    BDBG_WRN(("Scenario #2: one decoder to two displays"));
    
    display1 = NEXUS_Display_Open(1, NULL);
    NEXUS_Display_AddOutput(display1, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    window1 = NEXUS_VideoWindow_Open(display1, 0);
    NEXUS_VideoWindow_GetSettings(window0, &settings);
    settings.preferSyncLock = false;
    rc = NEXUS_VideoWindow_SetSettings(window0, &settings);
    BDBG_ASSERT(!rc);
    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = false;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);
    
    NEXUS_VideoWindow_AddInput(window0, NEXUS_VideoDecoder_GetConnector(vdecode0));
    NEXUS_VideoWindow_AddInput(window1, NEXUS_VideoDecoder_GetConnector(vdecode0));
    NEXUS_VideoDecoder_Start(vdecode0, &videoProgram0);

    /* this is the default */
    print_status(window0, window1, 0, 1, 0, 0);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* switch to SD */
    print_status(window0, window1, 0, 0, 1, 1);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = false;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* setting to false does not change anything */
    print_status(window0, window1, 0, 0, 0, 1);

    NEXUS_VideoWindow_GetSettings(window0, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window0, &settings);
    BKNI_Sleep(1000);BDBG_ASSERT(!rc);

    /* switch back to HD */
    print_status(window0, window1, 1, 1, 0, 0);

    NEXUS_VideoWindow_GetSettings(window1, &settings);
    settings.preferSyncLock = true;
    rc = NEXUS_VideoWindow_SetSettings(window1, &settings);
    BDBG_ASSERT(!rc);

    /* duplicate preferSyncLock = true should result in harmless warning */
    print_status(window0, window1, 1, 1, 1, 0);

    NEXUS_VideoWindow_Close(window0);
    NEXUS_VideoWindow_Close(window1);
    NEXUS_Display_Close(display0);
    NEXUS_Display_Close(display1);
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
