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
 *
 * Module Description:
 *
 *****************************************************************************/
#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_platform_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_video_decoder_primer.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_core_utils.h"
#include "nexus_frontend.h"
#include "nexus_parser_band.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "tspsimgr3.h"
#include "namevalue.h"

BDBG_MODULE(fcc_client);

static void print_usage(void)
{
    printf(
        "Usage: nexus.client fcc_client OPTIONS\n"
        "\n"
        "Options:\n"
        "  --help|-h      print this help screen\n"
        "  -freq X        tune this frequency\n"
        "  -timeout X     change channels after X seconds. default is to prompt for user.\n"

        );
}

static NEXUS_FrontendHandle frontend;

static int tune_qam(NEXUS_ParserBand parserBand, unsigned freq)
{
    NEXUS_FrontendAcquireSettings frontendAcquireSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    int rc;

    NEXUS_Frontend_GetDefaultAcquireSettings(&frontendAcquireSettings);
    frontendAcquireSettings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&frontendAcquireSettings);
    if (!frontend) {
        BDBG_WRN(("Unable to find QAM-capable frontend. Run server with -frontend option."));
        return -1;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e64;
    qamSettings.symbolRate = 5056900;
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    BDBG_ASSERT(!rc);

    return 0;
}

static void first_pts_passed(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_WRN(("first pts passed"));
}

#define NUM_VIDEO_DECODES 4

int main(int argc, char **argv)  {
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_SimpleVideoDecoderHandle videoDecoder[NUM_VIDEO_DECODES];
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle pcrPidChannel = NULL;
    NEXUS_SimpleStcChannelHandle stcChannel[NUM_VIDEO_DECODES];
    NEXUS_ParserBand parserBand[NUM_VIDEO_DECODES];
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    unsigned timeout = 0;
    int curarg = 1;
    unsigned freq = 765;
    tspsimgr_scan_results scan_results;
    unsigned program = 0;
    unsigned i;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-freq") && curarg+1<argc) {
            freq = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    NEXUS_Platform_GetClientConfiguration(&clientConfig);

    /* this example does fcc on a single frequency only. see nexus/nxclient/apps/live for a multi-tuner fcc client app. */
    parserBand[0] = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    rc = tune_qam(parserBand[0], freq);
    if (rc) return -1;

    {
        tspsimgr_t scan;
        tspsimgr_scan_settings scan_settings;

        scan = tspsimgr_create();
        tspsimgr_get_default_start_scan_settings(&scan_settings);
        scan_settings.parserBand = parserBand[0];
        rc = tspsimgr_start_scan(scan, &scan_settings);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(3000); /* TODO */
        tspsimgr_stop_scan(scan);
        rc = tspsimgr_get_scan_results(scan, &scan_results);
        BDBG_ASSERT(!rc);
        tspsimgr_destroy(scan);

        BDBG_WRN(("found %d program(s)", scan_results.num_programs));
        if (!scan_results.num_programs) {
            return -1;
        }
    }

    if (clientConfig.resources.simpleVideoDecoder.total) {
        for (i=0;i<clientConfig.resources.simpleVideoDecoder.total && i < NUM_VIDEO_DECODES;i++) {
            NEXUS_VideoDecoderSettings settings;

            videoDecoder[i] = NEXUS_SimpleVideoDecoder_Acquire(clientConfig.resources.simpleVideoDecoder.id[i]);
            if (!videoDecoder[i]) break;

            NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder[i], &settings);
            settings.firstPtsPassed.callback = first_pts_passed;
            rc = NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder[i], &settings);
            BDBG_ASSERT(!rc);

            stcChannel[i] = NEXUS_SimpleStcChannel_Create(NULL);
            BDBG_ASSERT(stcChannel[i]);

            if (i > 0) {
                /* duplicate parser band */
                NEXUS_ParserBandSettings settings;
                parserBand[i] = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
                NEXUS_ParserBand_GetSettings(parserBand[0], &settings);
                NEXUS_ParserBand_SetSettings(parserBand[i], &settings);
            }
        }
        NEXUS_Frontend_ReapplyTransportSettings(frontend);

        /* for a simple example, reduce # of channels to what we can decode and prime. */
        if (i < scan_results.num_programs) {
            scan_results.num_programs = i;
        }
    }
    if (!videoDecoder[0]) {
        BDBG_ERR(("video decoder not available"));
        rc = -1;
        goto done;
    }
    if (clientConfig.resources.simpleAudioDecoder.total) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(clientConfig.resources.simpleAudioDecoder.id[0]);
        if (!audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
            /* for this example, audio is optional */
        }
    }

    /* set up video decoders and primers ahead of channel change */
    for (i=0;i<scan_results.num_programs;i++) {
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

        if (scan_results.program_info[i].pcr_pid) {
            pcrPidChannel = NEXUS_PidChannel_Open(parserBand[i], scan_results.program_info[i].pcr_pid, NULL);
            NEXUS_SimpleStcChannel_GetSettings(stcChannel[i], &stcSettings);
            stcSettings.mode = NEXUS_StcChannelMode_ePcr;
            stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
            stcSettings.modeSettings.pcr.offsetThreshold = 0xFF;
            rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel[i], &stcSettings);
            BDBG_ASSERT(!rc);
        }

        if (scan_results.program_info[i].num_video_pids) {
            videoProgram.settings.pidChannel = NEXUS_PidChannel_Open(parserBand[i], scan_results.program_info[i].video_pids[0].pid, NULL);
            videoProgram.settings.codec = scan_results.program_info[i].video_pids[0].codec;
            NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder[i], stcChannel[i]);
        }

        rc = NEXUS_SimpleVideoDecoder_StartPrimer(videoDecoder[i], &videoProgram);
        BDBG_ASSERT(!rc);
    }

    while (1) {
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);

        if (scan_results.program_info[program].num_audio_pids) {
            audioProgram.primary.pidChannel = NEXUS_PidChannel_Open(parserBand[program], scan_results.program_info[program].audio_pids[0].pid, NULL);
            audioProgram.primary.codec = scan_results.program_info[program].audio_pids[0].codec;
            NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel[program]);
        }

        BDBG_WRN(("decoding program %d: video %#x %s, audio %#x %s",
            program,
            scan_results.program_info[program].video_pids[0].pid, lookup_name(g_videoCodecStrs, scan_results.program_info[program].video_pids[0].codec),
            scan_results.program_info[program].audio_pids[0].pid, lookup_name(g_audioCodecStrs, scan_results.program_info[program].audio_pids[0].codec)));

        /* Start decoders */
        rc = NEXUS_SimpleVideoDecoder_StopPrimerAndStartDecode(videoDecoder[program]);
        BDBG_ASSERT(!rc);
        if (audioDecoder && audioProgram.primary.pidChannel) {
            NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
            /* decode may fail if audio codec not supported */
        }
        if (!timeout) {
            BDBG_WRN(("Press ENTER to change channels"));
            getchar();
        }
        else {
            BDBG_WRN(("Changing channels in %d second(s)", timeout));
            BKNI_Sleep(timeout * 1000);
        }

        /* Bring down system */
        rc = NEXUS_SimpleVideoDecoder_StopDecodeAndStartPrimer(videoDecoder[program]);
        BDBG_ASSERT(!rc);
        if (audioDecoder) {
            NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
        }

        if (audioProgram.primary.pidChannel) {
            NEXUS_PidChannel_Close(audioProgram.primary.pidChannel);
        }
        if (++program == scan_results.num_programs) {
            program = 0;
        }
    }

    for (i=0;i<scan_results.num_programs;i++) {
        NEXUS_SimpleVideoDecoder_Release(videoDecoder[i]);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    }

done:
    NEXUS_Platform_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
