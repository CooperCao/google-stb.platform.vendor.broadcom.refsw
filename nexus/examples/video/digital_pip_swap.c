/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_AUDIO
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"

#include "bstd.h"
#include "bkni.h"
#include <string.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3

BDBG_MODULE(digital_pip_swap);

/**
digital pip swap
shows how to perform timebase management and audio
**/

void wait_for_next(void)
{
#if 0
    /* interactive mode */
    printf("Press ENTER to swap\n");
    getchar();
#else
    printf("Swap in 5 seconds...\n");
    BKNI_Sleep(5000);
#endif
}

struct {
    unsigned short videoPid, audioPid;
} g_program[2] = {
    {0x31, 0x34},
    {0x11, 0x14},
};

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand[2];
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderHandle videoDecoder[2];
    NEXUS_VideoDecoderStartSettings videoProgram[2];
    NEXUS_DisplayHandle display[2];
    NEXUS_VideoWindowHandle window[4];
    NEXUS_VideoWindowSettings windowSettings[4];
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    unsigned i;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram[2];
    unsigned programIndex = 0;
    bool zorderSwap = false;
    bool hdsd_simul = false;
    int curarg = 1;
    
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-z")) {
            zorderSwap = true;
        }
        else if (!strcmp(argv[curarg], "-sd")) {
            hdsd_simul = true;
        }
        curarg++;
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    if (hdsd_simul) {
        displaySettings.format = NEXUS_VideoFormat_e720p;
    }
    display[0] = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display[0], NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display[0], NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display[0], &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display[0], &displaySettings);
        }
    }
#endif
    window[0] = NEXUS_VideoWindow_Open(display[0], 0);
    window[1] = NEXUS_VideoWindow_Open(display[0], 1);
    BDBG_ASSERT(window[1]);
    if (hdsd_simul) {
        displaySettings.format = NEXUS_VideoFormat_eNtsc;
        display[1] = NEXUS_Display_Open(1, &displaySettings);
        window[2] = NEXUS_VideoWindow_Open(display[1], 0);
        window[3] = NEXUS_VideoWindow_Open(display[1], 1);
        BDBG_ASSERT(window[1]);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display[1], NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
    }
    else {
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(display[0], NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
    }

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* open decoders */
    for (i=0;i<2;i++) {
        parserBand[i] = NEXUS_ParserBand_e0 + i;

        /* It's important to use two parser bands so that the default maxDataRate of 25 Mbps per parser band will not be exceeded. */
        NEXUS_ParserBand_GetSettings(parserBand[i], &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = TRANSPORT_TYPE;
        NEXUS_ParserBand_SetSettings(parserBand[i], &parserBandSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram[i]);
        videoProgram[i].codec = VIDEO_CODEC;
        videoProgram[i].pidChannel = NEXUS_PidChannel_Open(parserBand[i], g_program[i].videoPid, NULL);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[i]);
        audioProgram[i].codec = AUDIO_CODEC;
        audioProgram[i].pidChannel = NEXUS_PidChannel_Open(parserBand[i], g_program[i].audioPid, NULL);

        NEXUS_StcChannel_GetDefaultSettings(i, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0 + i;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoProgram[i].pidChannel; /* PCR on video pid */
        videoProgram[i].stcChannel = NEXUS_StcChannel_Open(i, &stcSettings);

        videoDecoder[i] = NEXUS_VideoDecoder_Open(i, NULL); /* take default capabilities */
        if (!videoDecoder[i]) {
            BDBG_ERR(("platform doesn't support dual digital decode"));
            return -1;
        }
        if (!zorderSwap) {
            NEXUS_VideoDecoderSettings videoDecoderSettings;
            NEXUS_VideoDecoder_GetSettings(videoDecoder[i], &videoDecoderSettings);
            videoDecoderSettings.manualPowerState = true;
            rc = NEXUS_VideoDecoder_SetSettings(videoDecoder[i], &videoDecoderSettings);
            BDBG_ASSERT(!rc);
        }

        rc = NEXUS_VideoWindow_AddInput(window[i], NEXUS_VideoDecoder_GetConnector(videoDecoder[i]));
        BDBG_ASSERT(!rc);
        if (hdsd_simul) {
            rc = NEXUS_VideoWindow_AddInput(window[i+2], NEXUS_VideoDecoder_GetConnector(videoDecoder[i]));
            BDBG_ASSERT(!rc);
        }

        rc = NEXUS_VideoDecoder_Start(videoDecoder[i], &videoProgram[i]);
        BDBG_ASSERT(!rc);

        NEXUS_VideoWindow_GetSettings(window[i], &windowSettings[i]);
        if (hdsd_simul) {
            NEXUS_VideoWindow_GetSettings(window[i+2], &windowSettings[i+2]);
        }
    }

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0]) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    programIndex = 0;

    while (1) {
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram[programIndex]);

        wait_for_next();
        programIndex = 1 - programIndex; /* swap */

        NEXUS_AudioDecoder_Stop(audioDecoder);

        if (zorderSwap) {    
            NEXUS_Rect temp;
            BDBG_WRN(("begin zorder swap"));
            
            for (i=0;i<4;i+=2) {
                temp = windowSettings[programIndex+i].position;
                windowSettings[programIndex+i].position = windowSettings[(1-programIndex)+i].position;
                windowSettings[programIndex+i].zorder = 0;
                windowSettings[(1-programIndex)+i].position = temp;
                windowSettings[(1-programIndex)+i].zorder = 1;
            }
    
            /* to swap zorder, we must use NEXUS_DisplayUpdateMode to make the swap atomic */
            NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);
            rc = NEXUS_VideoWindow_SetSettings(window[0], &windowSettings[0]);
            BDBG_ASSERT(!rc);
            rc = NEXUS_VideoWindow_SetSettings(window[1], &windowSettings[1]);
            BDBG_ASSERT(!rc);
            if (hdsd_simul) {
                rc = NEXUS_VideoWindow_SetSettings(window[2], &windowSettings[2]);
                BDBG_ASSERT(!rc);
                rc = NEXUS_VideoWindow_SetSettings(window[3], &windowSettings[3]);
                BDBG_ASSERT(!rc);
            }
            NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
        }
        else {
            BDBG_WRN(("begin input swap"));
            /* we can remove and swap inputs because NEXUS_VideoDecoderSettings.manualPowerState = true */
            NEXUS_VideoWindow_RemoveAllInputs(window[0]);
            NEXUS_VideoWindow_RemoveAllInputs(window[1]);
            if (hdsd_simul) {
                NEXUS_VideoWindow_RemoveAllInputs(window[2]);
                NEXUS_VideoWindow_RemoveAllInputs(window[3]);
            }
            
            NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);
            rc = NEXUS_VideoWindow_AddInput(window[0], NEXUS_VideoDecoder_GetConnector(videoDecoder[programIndex]));
            BDBG_ASSERT(!rc);
            rc = NEXUS_VideoWindow_AddInput(window[1], NEXUS_VideoDecoder_GetConnector(videoDecoder[1-programIndex]));
            BDBG_ASSERT(!rc);
            if (hdsd_simul) {
                rc = NEXUS_VideoWindow_AddInput(window[2], NEXUS_VideoDecoder_GetConnector(videoDecoder[programIndex]));
                BDBG_ASSERT(!rc);
                rc = NEXUS_VideoWindow_AddInput(window[3], NEXUS_VideoDecoder_GetConnector(videoDecoder[1-programIndex]));
                BDBG_ASSERT(!rc);
            }
            NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
        }
        BDBG_WRN(("end"));
    }

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
