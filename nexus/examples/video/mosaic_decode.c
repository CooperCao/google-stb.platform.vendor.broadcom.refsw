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
#include "nexus_video_input.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_mosaic_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(mosaic_decode);

#define NUM_MOSAICS 4

struct {
    unsigned short videoPid;
    unsigned short pcrPid;
    NEXUS_VideoCodec videoCodec;
} g_programs[NUM_MOSAICS] = {
    {0x1001, 0x100, NEXUS_VideoCodec_eH264},
    {0x1002, 0x100, NEXUS_VideoCodec_eH264},
    {0x1003, 0x100, NEXUS_VideoCodec_eH264},
    {0x1004, 0x100, NEXUS_VideoCodec_eH264}
};

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode[NUM_MOSAICS];
    NEXUS_VideoDecoderStartSettings videoProgram[NUM_MOSAICS];
    NEXUS_PidChannelHandle pcrPidChannel[NUM_MOSAICS]; /* up to a max of NUM_MOSAICS */
    unsigned short pcrPidAssignment[NUM_MOSAICS]; /* this corresponds to the pcrPidChannel */
    NEXUS_StcChannelHandle stcChannel[NUM_MOSAICS]; /* up to a max of NUM_MOSAICS */

    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowHandle mosaicWindow[NUM_MOSAICS];
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Timebase timebase = NEXUS_Timebase_e0;
    NEXUS_TimebaseSettings timebaseSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    unsigned i;
    NEXUS_Error rc;

    BKNI_Memset(pcrPidChannel, 0, sizeof(pcrPidChannel));
    BKNI_Memset(pcrPidAssignment, 0, sizeof(pcrPidAssignment));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* bring up display */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
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

    /* bring up decoder and connect to display */
    for (i=0;i<NUM_MOSAICS;i++) {
        NEXUS_VideoDecoderOpenMosaicSettings openSettings;
        NEXUS_StcChannelSettings stcSettings;
        unsigned j;

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram[i]);
        videoProgram[i].codec = g_programs[i].videoCodec;
        videoProgram[i].pidChannel = NEXUS_PidChannel_Open(parserBand, g_programs[i].videoPid, NULL);

        NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&openSettings);
        openSettings.maxWidth = 352; /* CIF */
        openSettings.maxHeight = 288; /* CIF */
        vdecode[i] = NEXUS_VideoDecoder_OpenMosaic(0, i, &openSettings);

        /* see if this PCR is already used by another mosaic. */
        for (j=0;pcrPidChannel[j];j++) {
            if (g_programs[i].pcrPid == pcrPidAssignment[j]) {
                /* It is, so reuse the StcChannel. We can't route the same PCR to more than one PCR_OFFSET block. */
                videoProgram[i].stcChannel = stcChannel[j];
                break;
            }
        }
        if (!videoProgram[i].stcChannel) {
            /* It is not. So open a new pcr pid and StcChannel. */
            BDBG_ASSERT(!pcrPidChannel[j]);
            pcrPidAssignment[j] = g_programs[i].pcrPid;
            pcrPidChannel[j] = NEXUS_PidChannel_Open(parserBand, g_programs[i].pcrPid, NULL);

            /* each PCR gets its own StcChannel (i.e. PCR_OFFSET HW), but we use a shared Timebase (i.e. DPCR and STC HW) */
            NEXUS_StcChannel_GetDefaultSettings(j, &stcSettings);
            stcSettings.timebase = timebase;
            stcSettings.autoConfigTimebase = false;
            stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
            stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel[j];
            stcSettings.stcIndex = 0; /* shared STC for all mosaics */
            stcChannel[j] = NEXUS_StcChannel_Open(j, &stcSettings);
            videoProgram[i].stcChannel = stcChannel[j];
        }

        mosaicWindow[i] = NEXUS_VideoWindow_OpenMosaic(window, i);
        rc = NEXUS_VideoWindow_AddInput(mosaicWindow[i], NEXUS_VideoDecoder_GetConnector(vdecode[i]));
        BDBG_ASSERT(!rc);

        NEXUS_VideoWindow_GetSettings(mosaicWindow[i], &windowSettings);
        windowSettings.position.x = 50 + (120 * i);
        windowSettings.position.y = 50;
        windowSettings.position.width = 100;
        windowSettings.position.height = 100;
        windowSettings.visible = true;
        windowSettings.zorder = i;
        rc = NEXUS_VideoWindow_SetSettings(mosaicWindow[i], &windowSettings);
        BDBG_ASSERT(!rc);
    }

    /* share one timebase. ideally the timebase is set to the pcr associated with the current audio. */
    NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.sourceSettings.pcr.pidChannel = pcrPidChannel[0];
    rc = NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
    BDBG_ASSERT(!rc);

    /* Start Decoders */
    for (i=0;i<NUM_MOSAICS;i++) {
        rc = NEXUS_VideoDecoder_Start(vdecode[i], &videoProgram[i]);
        BDBG_ASSERT(!rc);
    }

#if 1
    for (;;) {
        for (i=0;i<NUM_MOSAICS;i++) {
            NEXUS_VideoDecoderStatus status;
            uint32_t stc;
            NEXUS_VideoDecoder_GetStatus(vdecode[i], &status);
            NEXUS_StcChannel_GetStc(videoProgram[i].stcChannel, &stc);
            printf("decode[%d] %dx%d, pts %#x, stc %#x (diff %d)\n",
                i, status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to exit\n");
    getchar();
#endif

    for (i=0;i<NUM_MOSAICS;i++) {
        NEXUS_VideoDecoder_Stop(vdecode[i]);
        NEXUS_PidChannel_Close(videoProgram[i].pidChannel);
        NEXUS_VideoWindow_Close(mosaicWindow[i]);
        NEXUS_VideoDecoder_Close(vdecode[i]);
    }
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
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
