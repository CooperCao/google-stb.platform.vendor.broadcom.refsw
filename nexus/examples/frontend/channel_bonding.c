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
 *****************************************************************************/

/* DVB-S2X channel bonding */

#include "nexus_platform.h"
#include "nexus_frontend.h"
#include "nexus_frontend_channelbonding.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_record.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;
    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %s\n", status.demodLocked ? "locked" : "unlocked");
}

#define NUM_CHANNELS 3
#define VIDEOCODEC NEXUS_VideoCodec_eH264
#define VIDEOPID 0x101
#define AUDIOPID 0x104
#define PCRPID VIDEOPID
#define MASTER_OFFSET 0

int main(int argc, char **argv)
{
    NEXUS_FrontendHandle frontend[NUM_CHANNELS];
    NEXUS_FrontendType frontendType;
    unsigned frequency[NUM_CHANNELS] = { 1000, 1100, 1200 };
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
    NEXUS_FrontendStartBondingGroupSettings bondSettings;
    unsigned symRate = 30000000;
    NEXUS_FrontendSatelliteMode mode = NEXUS_FrontendSatelliteMode_eDvbs2Acm;
    NEXUS_TransportType transportType = NEXUS_TransportType_eTs;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;
    unsigned i, dcbgIndex = 0;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    for (i=0; i<NUM_CHANNELS; i++) {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[i] = 40*1000*1000;
    }
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;

    /* find bonding-capable frontend index */
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++) {
        if (platformConfig.frontend[i]==NULL) { continue; }
        NEXUS_Frontend_GetType(platformConfig.frontend[i], &frontendType);
        if (frontendType.chip.familyId >= 0x45302 && frontendType.chip.familyId <= 0x45316) {
            dcbgIndex = i;
            BDBG_ASSERT(dcbgIndex+NUM_CHANNELS < NEXUS_MAX_FRONTENDS);
            break;
        }
    }
    if (i==NEXUS_MAX_FRONTENDS) {
        fprintf(stderr, "No bonding-capable frontends found\n");
        return 0;
    }

    NEXUS_Frontend_GetDefaultStartBondingGroupSettings(&bondSettings);
    for (i=0; i<NUM_CHANNELS; i++) {
        frontend[i] = platformConfig.frontend[dcbgIndex+i];
        BDBG_ASSERT(frontend[i]);
        bondSettings.slave[i] = frontend[i];
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = frequency[0] * 1000 * 1000;
    satSettings.mode = mode;
    satSettings.symbolRate = symRate;
    satSettings.codeRate.numerator = 11;
    satSettings.codeRate.denominator = 20;
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context = NULL;
    satSettings.ldpcPilot = false;

    /* connect master frontend handle to a host parserband */
    parserBand = NEXUS_ParserBand_e0 + MASTER_OFFSET;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
    parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend[MASTER_OFFSET]);
    parserBandSettings.transportType = transportType;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    for (i=0; i<NUM_CHANNELS; i++) {
        satSettings.frequency = frequency[i] * 1000 * 1000;
        satSettings.lockCallback.context = frontend[i];
        rc = NEXUS_Frontend_TuneSatellite(frontend[i], &satSettings);
        BDBG_ASSERT(!rc);
    }

    while (1) {
        NEXUS_FrontendSatelliteStatus satStatus[NUM_CHANNELS];
        bool allLocked = true;

        for (i=0; i<NUM_CHANNELS; i++) {
            rc = NEXUS_Frontend_GetSatelliteStatus(frontend[i], &satStatus[i]);
            BDBG_ASSERT(!rc);
        }
        BKNI_Printf("lock %u %u %u\n", satStatus[0].demodLocked, satStatus[1].demodLocked, satStatus[2].demodLocked);

        for (i=0; i<NUM_CHANNELS; i++) {
            if (!satStatus[i].demodLocked) {
                allLocked = false;
                break;
            }
        }
        if (allLocked) { break; }
        BKNI_Sleep(1000);
    }

    BKNI_Printf("Press Enter to start bonding group...\n");
    getchar();

    rc = NEXUS_Frontend_StartBondingGroup(frontend[MASTER_OFFSET], &bondSettings);
    BDBG_ASSERT(!rc);

    while (1) {
        NEXUS_FrontendBondingGroupStatus status;
        NEXUS_Frontend_GetBondingGroupStatus(frontend[MASTER_OFFSET], &status);
        BKNI_Printf("Wait for bond lock...\n");
        if (status.locked) { break; }
        BKNI_Sleep(1000);
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if (!rc && hdmiStatus.connected)
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if (!hdmiStatus.videoFormatSupported[displaySettings.format]) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }

    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    videoDecoderOpenSettings.fifoSize = 5*1024*1024;
    videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEOPID, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIOPID, NULL);
    pcrPidChannel = NEXUS_PidChannel_Open(parserBand, PCRPID, NULL);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEOCODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

#if 1
    printf("Press enter to stop decode\n");
    getchar();
#else
    while (1) {
        NEXUS_VideoDecoderStatus videoStatus;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(videoDecoder, &videoStatus);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            videoStatus.source.width, videoStatus.source.height, videoStatus.pts, stc, videoStatus.ptsStcDifference, videoStatus.fifoSize?(videoStatus.fifoDepth*100)/videoStatus.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(audioDecoder, &audioStatus);
        printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        BKNI_Sleep(1000);
    }
#endif

    NEXUS_Frontend_StopBondingGroup(frontend[MASTER_OFFSET]);

    BKNI_Printf("Stopped...\n");
    getchar();

    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    if (pcrPidChannel != videoPidChannel) {
        NEXUS_PidChannel_Close(pcrPidChannel);
    }

    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_RemoveAllOutputs(display);
    NEXUS_Display_Close(display);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();
    return 0;
}
