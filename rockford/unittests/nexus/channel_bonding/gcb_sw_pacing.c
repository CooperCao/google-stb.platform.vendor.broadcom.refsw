/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_hdmi_output.h"
#include "nexus_frontend_satellite.h"
#include "nexus_record.h"
#include "nexus_recpump.h"
#include "nexus_pid_channel.h"
#if NEXUS_USE_7445_DBS
#include "nexus_frontend_4538.h"
#endif
#include "nexus_parser_band_channelbonding.h"

#include "b_os_lib.h"
#include "b_playback_ip_lib.h"
#include "../src/b_playback_ip_tts_throttle.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0 /* single, unbonded channel */
#define NUM_FRONTENDS 1
unsigned TUNER_FREQ[NUM_FRONTENDS] = {1119};
#define SYMRATE 20
#else
#define USE_AMC 0
#if USE_AMC
#include "../../../nexus/extensions/transport/tbg/examples/amc_helper.h"
#endif

#define NUM_FRONTENDS 3
unsigned AMC_KEY[NUM_FRONTENDS] = {0, 71, 73};
unsigned TUNER_FREQ[NUM_FRONTENDS] = {1265, 1420, 1580};
#if 1
#define SYMRATE 27.5469
#else
#define SYMRATE 26.1799
#endif
unsigned SAT_NUMERATOR = 3;
unsigned SAT_DENOMINATOR = 4;
#endif

#define DO_HEVC_DECODE 0

#if (!DO_HEVC_DECODE)
#define VIDEO_PID 0x31
#define AUDIO_PID 0x34
#define PCR_PID   0x31
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#else
#define VIDEO_PID 0x1001
#define AUDIO_PID 0x1002
#define PCR_PID   0x1001
#define VIDEO_CODEC NEXUS_VideoCodec_eH265
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#endif


BDBG_MODULE(gcb_sw_pacing);

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;

    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %s\n", status.demodLocked ? "locked" : "unlocked");
}

int main(int argc, char **argv)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendHandle frontend[NUM_FRONTENDS];
    NEXUS_FrontendSatelliteSettings satSettings;
#if NEXUS_USE_7445_DBS
    NEXUS_Frontend4538RuntimeSettings satRuntimeSettings;
#endif
    NEXUS_FrontendSatelliteStatus satStatus[NUM_FRONTENDS];
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_ParserBandStartBondingGroupSettings bondSettings;
    NEXUS_ParserBandBondingGroupStatus bondStatus;
    NEXUS_Error rc;
    unsigned i;
    B_PlaybackIp_TtsThrottleHandle ttsThrottle;
    B_PlaybackIp_TtsThrottle_Params throttleParams;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpStatus playpumpStatus;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    /* increase IBP maxDataRate */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    for (i=0; i<NUM_FRONTENDS; i++) {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[i] = 50*1024*1024;
    }
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* hardcode frontend handles. assume satellite capability and MTSIF */
    for (i=0; i<NUM_FRONTENDS; i++) {
        frontend[i] = platformConfig.frontend[i];
    }

    /* remap ADC for DBS board */
#if NEXUS_USE_7445_DBS
    for (i=0; i<NUM_FRONTENDS; i++) {
        NEXUS_Frontend_Get4538RuntimeSettings(frontend[i], &satRuntimeSettings);
        satRuntimeSettings.selectedAdc = 1;
        NEXUS_Frontend_Set4538RuntimeSettings(frontend[i], &satRuntimeSettings);
    }
#endif

    /* map frontends to parserbands */
    for (i=0; i<NUM_FRONTENDS; i++) {
        parserBand = NEXUS_ParserBand_e0 + i;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend[i]);
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.mode = NEXUS_FrontendSatelliteMode_eDvb;
    satSettings.symbolRate = SYMRATE * 1000 * 1000;
    satSettings.lockCallback.callback = lock_callback;
#if USE_AMC
    satSettings.mode = NEXUS_FrontendSatelliteMode_eQpskLdpc;
    satSettings.codeRate.numerator = SAT_NUMERATOR;
    satSettings.codeRate.denominator = SAT_DENOMINATOR;
    BDBG_ASSERT(NEXUS_SATELLITE_CAN_SET_AMC_BITS);
    satSettings.amc.enable = true;
#endif

    for (i=0; i<NUM_FRONTENDS; i++) {
        satSettings.lockCallback.context = frontend[i];
        satSettings.frequency = TUNER_FREQ[i] * 1000 * 1000;
#if USE_AMC
        satSettings.amc.xseed = amc_keys[AMC_KEY[i]].xseed;
        satSettings.amc.plhdrscr1 = amc_keys[AMC_KEY[i]].plhdrscr1;
        satSettings.amc.plhdrscr2 = amc_keys[AMC_KEY[i]].plhdrscr2;
        satSettings.amc.plhdrscr3 = amc_keys[AMC_KEY[i]].plhdrscr3;
#endif
        rc = NEXUS_Frontend_TuneSatellite(frontend[i], &satSettings);
        BDBG_ASSERT(!rc);
    }

    /* wait for initial lock */
    while (1) {
        bool allLocked = true;
        BKNI_Printf("lock ");
        for (i=0; i<NUM_FRONTENDS; i++) {
            NEXUS_Frontend_GetSatelliteStatus(frontend[i], &satStatus[i]);
            BKNI_Printf("%u ", satStatus[i].demodLocked);
        }
        BKNI_Printf("\n");

        for (i=0; i<NUM_FRONTENDS; i++) {
            if (!satStatus[i].demodLocked) {
                allLocked = false;
                break;
            }
        }
        if (allLocked) { break; }

        BKNI_Sleep(100);
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
#if DO_HEVC_DECODE
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);
}
#endif

    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

restart:
    NEXUS_ParserBand_GetDefaultStartBondingGroupSettings(&bondSettings);
    bondSettings.bondingPid = 0x1ffe;
    bondSettings.soft.pcrPid = PCR_PID;
    bondSettings.soft.timebase = NEXUS_Timebase_e0; /* this is a freerun timebase that drives the pacing counter */

    /* use PB0 as master */
    for (i=0; i+1<NUM_FRONTENDS; i++) {
        bondSettings.slave[i] = (NEXUS_ParserBand_e0 + i + 1);
    }

    rc = NEXUS_ParserBand_StartBondingGroup(NEXUS_ParserBand_e0, &bondSettings);
    BDBG_ASSERT(!rc);

    playpump = NEXUS_ParserBand_GetBondingGroupPlaypump(NEXUS_ParserBand_e0);
    BDBG_ASSERT(playpump);
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, AUDIO_PID, NULL);

    /* STC channel settings */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e1; /* this is a separate timebase tracking the PCR, after re-sequentializing */
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel;
    stcSettings.modeSettings.pcr.maxPcrError = 100 * 91; /* 100ms in 90Khz clock */
    stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
    BDBG_ASSERT(stcChannel);

    while (1) {
        rc = NEXUS_ParserBand_GetBondingGroupStatus(NEXUS_ParserBand_e0, &bondStatus);
        BDBG_ASSERT(!rc);
        BKNI_Printf("bond lock: %u\n", bondStatus.locked);
        if (bondStatus.locked) { break; }
        BKNI_Sleep(100);
    }

#if 1
    /* clock recovery using playback_ip */
{
    B_Os_Init();

    ttsThrottle = B_PlaybackIp_TtsThrottle_Open();

    B_PlaybackIp_TtsThrottle_ParamsInit(&throttleParams);
    throttleParams.initBufDepth = 500*1000;
    throttleParams.maxBufDepth = 5000*1000;
    throttleParams.minBufDepth = 0;
    throttleParams.maxClockMismatch = 100;
    throttleParams.playPump = NEXUS_ParserBand_GetBondingGroupPlaypump(NEXUS_ParserBand_e0);
    throttleParams.timebase = bondSettings.soft.timebase;
    throttleParams.scatterGatherPlaypump = true;
    B_PlaybackIp_TtsThrottle_SetSettings(ttsThrottle, &throttleParams);

    do {
        NEXUS_Playpump_GetStatus(throttleParams.playPump, &playpumpStatus);
        BDBG_WRN(("playpump: %u", (unsigned)playpumpStatus.fifoDepth));
        BKNI_Sleep(50);
    } while (playpumpStatus.fifoDepth < throttleParams.initBufDepth);

    B_PlaybackIp_TtsThrottle_Start(ttsThrottle);
}
#endif

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

#if 0
    BDBG_WRN(("started... press ENTER to quit"));
    getchar();
#else
    while (1) {
        getchar();
        BDBG_WRN(("restart"));
        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(audioPidChannel);
        NEXUS_ParserBand_StopBondingGroup(NEXUS_ParserBand_e0);
        goto restart;
    }
#endif

    B_PlaybackIp_TtsThrottle_Stop(ttsThrottle);
    NEXUS_ParserBand_StopBondingGroup(NEXUS_ParserBand_e0);

    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);

    NEXUS_Platform_Uninit();

    return 0;
}
