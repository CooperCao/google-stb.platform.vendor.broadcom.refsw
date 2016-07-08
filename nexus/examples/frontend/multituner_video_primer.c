/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/* sample application showing multiple tuners used in conjunction with multiple primers to provide
   fast channel change across the entire channel map */
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "../../nxclient/apps/utils/tspsimgr3.h"
#include "blst_queue.h"
#include "nexus_ir_input.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_primer.h"
#include "nexus_audio_decoder.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_sync_channel.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(multituner_video_primer);

#define USE_QAM 1 /* otherwise, satellite */

#if USE_QAM
#define FRONTEND_MODE NEXUS_FrontendQamMode_e64
#else
#define FRONTEND_MODE NEXUS_FrontendSatelliteMode_eDvb
#define TONE_MODE true
#define VOLTAGE NEXUS_FrontendDiseqcVoltage_e13v
#endif

#define DISPLAY_FORMAT NEXUS_VideoFormat_e1080p
#define USE_SYNC_CHANNEL 1

static void scan_complete(void *data)
{
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void ir_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void print_usage(void)
{
    printf(
        "Usage: nexus multituner_video_primer [options]\n"
        "  --help or -h for help\n"
        "  -freq #[,#,#]            frequency list in MHz\n"
        );
}

struct channel_map {
    BLST_Q_ENTRY(channel_map) link;
    char name[32];
    unsigned freq;
    tspsimgr_scan_results scan_results;
};
static BLST_Q_HEAD(channellist_t, channel_map) g_channel_map;

struct frontend {
    BLST_Q_ENTRY(frontend) link;
    NEXUS_FrontendHandle frontend;
    struct channel_map *map;
};
static BLST_Q_HEAD(frontendlist_t, frontend) g_frontends;

struct program {
    BLST_Q_ENTRY(program) link;
    NEXUS_VideoDecoderPrimerHandle primer;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_ParserBand parserBand;
    struct frontend *frontend;
    unsigned videoPid;
};
static BLST_Q_HEAD(programlist_t, program) g_programs;

static void parse_channels(const char *freq_list)
{
    while (freq_list) {
        unsigned freq;
        if (sscanf(freq_list, "%u", &freq) == 1) {
            struct channel_map *c = malloc(sizeof(*c));
            c->freq = freq;
            snprintf(c->name, sizeof(c->name), "%s %dMhz", USE_QAM?"QAM":"SAT", freq);
            BLST_Q_INSERT_TAIL(&g_channel_map, c, link);
        }
        freq_list = strchr(freq_list, ',');
        if (freq_list) freq_list++;
    }
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
#if USE_QAM
    NEXUS_FrontendQamStatus qamStatus;
#else
    NEXUS_FrontendSatelliteStatus satStatus;
#endif

    BSTD_UNUSED(param);

#if USE_QAM
    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    BDBG_MSG(("QAM Lock callback, frontend %p - lock status %d, %d", (void*)frontend, qamStatus.fecLock, qamStatus.receiverLock));
#else
    NEXUS_Frontend_GetSatelliteStatus(frontend, &satStatus);
    BDBG_MSG(("SAT Lock callback, frontend %p - lock status %d", (void*)frontend, satStatus.demodLocked));
#endif
}

int tune(NEXUS_ParserBand parserBand, NEXUS_FrontendHandle frontend, unsigned freq, unsigned mode)
{
    NEXUS_ParserBandSettings parserBandSettings;
    int rc;
    NEXUS_FrontendUserParameters userParams;
#if USE_QAM
    NEXUS_FrontendQamSettings qamSettings;
#else
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_FrontendSatelliteSettings satSettings;
#endif

    if (freq==0) { /* streamer */
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        return 0;
    }

#if USE_QAM
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    switch (mode) {
    case NEXUS_FrontendQamMode_e64:
        qamSettings.symbolRate = 5056900;
        break;
    case NEXUS_FrontendQamMode_e256:
        qamSettings.symbolRate = 5360537;
        break;
    default:
        BDBG_ERR(("unknown qam mode %d", mode));
        rc = -1;
        goto error;
    }
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = mode;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = frontend;
#else
    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = freq * 1000000;
    satSettings.mode = mode;
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context = frontend;
#endif

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
    if (rc) {rc = BERR_TRACE(rc); goto error;}

#if (!USE_QAM)
    NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
    diseqcSettings.toneEnabled = TONE_MODE;
    diseqcSettings.voltage = VOLTAGE;
    NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
#endif

#if USE_QAM
    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
#else
    rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
#endif
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    /* TODO: wait for lock. for now, just start scanning and let it lock during scan */
    return 0;

error:
    return rc;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderPrimerCreateSettings primerCreateSettings;
    NEXUS_VideoDecoderPrimerSettings primerSettings;
    NEXUS_AudioDecoderHandle audioDecoder;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_VideoWindowHandle window;
    NEXUS_ParserBand parserBand;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_Timebase timebase = NEXUS_Timebase_e0;
    NEXUS_IrInputHandle irHandle;
    NEXUS_IrInputSettings irSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BKNI_EventHandle feEvent, irEvent;
    int curarg = 1;
    unsigned i;
    const char *freq_list = "765,777,789";
#if USE_SYNC_CHANNEL
    NEXUS_SyncChannelHandle syncChannel;
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif

    struct channel_map *map = NULL;
    struct frontend *frontend;
    struct program *program;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
#if NEXUS_HAS_FRONTEND
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            freq_list = argv[++curarg];
        }
#endif
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    NEXUS_Platform_Init(NULL);
    BKNI_CreateEvent(&feEvent);
    BKNI_CreateEvent(&irEvent);

    NEXUS_IrInput_GetDefaultSettings(&irSettings);
    irSettings.mode = NEXUS_IrInputMode_eRemoteA;
    irSettings.dataReady.callback = ir_callback;
    irSettings.dataReady.context = irEvent;
    irHandle = NEXUS_IrInput_Open(0, &irSettings);

    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    if (!parserBand) {
        return BERR_TRACE(-1);
    }

    parse_channels(freq_list);

    /* to keep things simple, we borrow TS PSI scanning capability from /nexus/nxclient/apps/utils/tspsimgr3.c */
    {
        tspsimgr_t tspsimgr;
        NEXUS_FrontendHandle f;
        NEXUS_FrontendAcquireSettings settings;
        bool useTuner = false;

        for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
            if (map->freq>0) {
                useTuner = true;
                break;
            }
        }

        if (useTuner) {
            NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
#if USE_QAM
            settings.capabilities.qam = true;
#else
            settings.capabilities.satellite = true;
#endif

            f = NEXUS_Frontend_Acquire(&settings);
            BDBG_ASSERT(f);
        }

        tspsimgr = tspsimgr_create();
        for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
            tspsimgr_scan_settings scan_settings;
            rc = tune(parserBand, f, map->freq, FRONTEND_MODE);
            if (rc) continue;

            tspsimgr_get_default_start_scan_settings(&scan_settings);
            scan_settings.parserBand = parserBand;
            scan_settings.scan_done = scan_complete;
            scan_settings.context = feEvent;
            rc = tspsimgr_start_scan(tspsimgr, &scan_settings);
            BDBG_ASSERT(!rc);

            BKNI_WaitForEvent(feEvent, 3000);
            /* even on timeout, work with partial scan results */

            tspsimgr_get_scan_results(tspsimgr, &map->scan_results);
            BDBG_WRN(("%d programs found on freq %u", map->scan_results.num_programs, map->freq));
        }
        tspsimgr_destroy(tspsimgr); /* TODO: continuous scan */
        if (useTuner) {
            NEXUS_Frontend_Release(f);
        }
    }

    /* acquire as many frontend handles as there are frequencies */
    for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
#if USE_QAM
        settings.capabilities.qam = true;
#else
        settings.capabilities.satellite = true;
#endif

        frontend = malloc(sizeof(*frontend));
        memset(frontend, 0, sizeof(*frontend));

        if (map->freq>0) {
            frontend->frontend = NEXUS_Frontend_Acquire(&settings);
            if (!frontend->frontend) {
                BDBG_ERR(("Unable to find %s-capable frontend", USE_QAM?"QAM":"SAT"));
                return -1;
            }
        }
        frontend->map = map;
        BLST_Q_INSERT_TAIL(&g_frontends, frontend, link);
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);
    
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = DISPLAY_FORMAT;
    display = NEXUS_Display_Open(0, &displaySettings);
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
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    /* required for 4K content */
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    for (frontend = BLST_Q_FIRST(&g_frontends); frontend; frontend = BLST_Q_NEXT(frontend, link)) {
        map = frontend->map;
        for (i=0; i < map->scan_results.num_programs; i++) {
            unsigned pid;
            program = malloc(sizeof(*program));
            memset(program, 0, sizeof(*program));
            program->frontend = frontend;
            program->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID); /* one parser band per program ensures ample bandwidth */           
            tune(program->parserBand, frontend->frontend, map->freq, FRONTEND_MODE);

            NEXUS_VideoDecoderPrimer_GetDefaultCreateSettings(&primerCreateSettings);
            if (map->scan_results.program_info[i].video_pids[0].codec==NEXUS_VideoCodec_eH265) {
                primerCreateSettings.fifoSize = 14*1024*1024;
            }
            program->primer = NEXUS_VideoDecoderPrimer_Create(&primerCreateSettings);
            if (!program->primer) {
                BDBG_ERR(("Unable to create primer"));
                return -1;
            }

            NEXUS_VideoDecoderPrimer_GetSettings(program->primer, &primerSettings);
#if USE_SYNC_CHANNEL
            primerSettings.pastTolerance = 1000;
            primerSettings.ptsStcDiffCorrectionEnabled = true;
#endif
            rc = NEXUS_VideoDecoderPrimer_SetSettings(program->primer, &primerSettings);
            BDBG_ASSERT(!rc);

            NEXUS_VideoDecoder_GetDefaultStartSettings(&program->videoProgram);
            pid = map->scan_results.program_info[i].video_pids[0].pid;
            program->videoProgram.codec = map->scan_results.program_info[i].video_pids[0].codec;
            program->videoProgram.pidChannel = NEXUS_PidChannel_Open(program->parserBand, pid, NULL);
            program->videoPid = pid;

            NEXUS_AudioDecoder_GetDefaultStartSettings(&program->audioProgram);
            pid = map->scan_results.program_info[i].audio_pids[0].pid;
            program->audioProgram.codec = map->scan_results.program_info[i].audio_pids[0].codec;
            program->audioProgram.pidChannel = NEXUS_PidChannel_Open(program->parserBand, pid, NULL);

            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.autoConfigTimebase = false; /* must do it manually */
            stcSettings.timebase = timebase;
            stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
            stcSettings.modeSettings.pcr.pidChannel = program->videoProgram.pidChannel;
            stcSettings.stcIndex = 0;
            program->videoProgram.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            program->audioProgram.stcChannel = program->videoProgram.stcChannel;

            BLST_Q_INSERT_TAIL(&g_programs, program, link);
        }
    }

#if USE_SYNC_CHANNEL
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);

    NEXUS_SyncChannel_GetSettings(syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    syncChannelSettings.enableMuteControl = false;
    NEXUS_SyncChannel_SetSettings(syncChannel, &syncChannelSettings);
#endif

    for (program = BLST_Q_FIRST(&g_programs); program; program = BLST_Q_NEXT(program, link)) {
        NEXUS_VideoDecoderPrimer_Start(program->primer, &program->videoProgram);
    }
  
    program = BLST_Q_FIRST(&g_programs);
    while (1) {
        bool chanup = true;
        NEXUS_TimebaseSettings timebaseSettings;

        NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.sourceSettings.pcr.pidChannel = program->videoProgram.pidChannel;
        timebaseSettings.sourceSettings.pcr.maxPcrError = 0xff;
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
        NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
        
        NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(program->primer, videoDecoder);
        NEXUS_AudioDecoder_Start(audioDecoder, &program->audioProgram);
        BKNI_Printf("decoding freq %u, pid %#x\n", program->frontend->map->freq, program->videoPid);
#if 0
        getchar();
#else
        while (1) {
            NEXUS_IrInputEvent irInput;
            size_t num;
            bool overflow;

            NEXUS_IrInput_GetEvents(irHandle, &irInput, 1, &num, &overflow);
            if (num == 0) {
                BKNI_WaitForEvent(irEvent, 0xFFFFFFFF);
            }
            else if (irInput.code == 0x500b && !irInput.repeat) { /* chanup */
                chanup = true;
                break;
            }
            else if (irInput.code == 0x400c && !irInput.repeat) { /* chandown */
                chanup = false;
                break;
            }
        }
#endif

#if 0
        NEXUS_VideoDecoderPrimer_StopDecodeAndStartPrimer(program->primer, videoDecoder);
#else
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_VideoDecoderPrimer_Start(program->primer, &program->videoProgram);
#endif
        NEXUS_AudioDecoder_Stop(audioDecoder);

        if (chanup) {
            program = BLST_Q_NEXT(program, link);
            if (program==NULL) {
                program = BLST_Q_FIRST(&g_programs);
            }
        }
        else {
            program = BLST_Q_PREV(program, link);
            if (program==NULL) {
                program = BLST_Q_LAST(&g_programs);
            }
        }
    }

#if USE_SYNC_CHANNEL
    NEXUS_SyncChannel_Destroy(syncChannel);
#endif

    for (program = BLST_Q_FIRST(&g_programs); program; program = BLST_Q_NEXT(program, link)) {
        NEXUS_VideoDecoderPrimer_Stop(program->primer);
        NEXUS_StcChannel_Close(program->videoProgram.stcChannel);
        NEXUS_PidChannel_Close(program->videoProgram.pidChannel);
        NEXUS_VideoDecoderPrimer_Destroy(program->primer);
        NEXUS_ParserBand_Close(program->parserBand);
    }

    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_Display_RemoveAllOutputs(display);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);

    BKNI_DestroyEvent(feEvent);
    BKNI_DestroyEvent(irEvent);
    NEXUS_Platform_Uninit();
    return 0;
}
#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
