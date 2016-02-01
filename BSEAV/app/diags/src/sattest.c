/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 *****************************************************************************/
/* Example to tune a satellite channel using nexus */

#if NEXUS_HAS_FRONTEND
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
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "prompt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>

#define USE_DVB 1 /* if set to 0 or undefined, use DSS */

#define PARSER_BAND NEXUS_ParserBand_e2; /* Change this value to match your platform */

/* the following define the input and its characteristics -- these will vary by input type */
#if USE_DVB
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x31
#define AUDIO_PID 0x34
#define FREQ 1119000000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDvb
#define SYMBOL_RATE 20000000
#define TONE_ENABLED true
#define TONE_MODE NEXUS_FrontendDiseqcToneMode_eEnvelope
#define VOLTAGE NEXUS_FrontendDiseqcVoltage_e13v
#else
#define TRANSPORT_TYPE NEXUS_TransportType_eDssEs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x78
#define AUDIO_PID 0x79
#define FREQ 1396820000
#define SATELLITE_MODE NEXUS_FrontendSatelliteMode_eDss
#define SYMBOL_RATE 20000000
#define TONE_ENABLED false
#define TONE_MODE NEXUS_FrontendDiseqcToneMode_eTone
#define VOLTAGE NEXUS_FrontendDiseqcVoltage_e18v
#endif

static int get_number(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return atoi(buf);
}

static pthread_t bertThread;

bool diseqcSupport = false;
bool bertEnable = false;
bool pnDataInvert = false;
bool quit = false;
bool reset = false;

static void *bertHandler(void *pParam)
{
    char ch;
    pParam = pParam;
    while (1) {
        ch = getchar();
        if (ch=='q') quit=true;
        if (ch=='r') reset=true;
    }
    return 0;
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;
    NEXUS_FrontendDiseqcStatus disqecStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %d\n", status.demodLocked);
    fprintf(stderr, "   fecCorrected = %d, fecUncorrected = %d, fecClean = %d\n", status.fecCorrected, status.fecUncorrected, status.fecClean);
    if (bertEnable) {
        fprintf(stderr, "  berErrorCount = %d\n", status.berErrorCount);
    }

    if (diseqcSupport) {
        NEXUS_Frontend_GetDiseqcStatus(frontend, &disqecStatus);
        fprintf(stderr, "  diseqc tone = %d, voltage = %d\n", disqecStatus.toneEnabled, disqecStatus.voltage);
    }
}

int get_sat_status(NEXUS_FrontendHandle frontend)
{
    NEXUS_Error rc;
    NEXUS_FrontendSatelliteStatus satStatus;
    rc = NEXUS_Frontend_GetSatelliteStatus(frontend, &satStatus);
    if (rc) {
        printf("unable to read status\n");
    }
    else {
        printf(
            "Sat Status\n"
            "  symbolRate %d\n"
            "  locked  %d\n"
            "  snr  %3.1f\n"
            "  fecCorrected = %d\n"
            "  fecUncorrected = %d\n"
            "  fecClean = %d\n",
                satStatus.settings.symbolRate,
                satStatus.demodLocked,
                satStatus.snrEstimate/(float)100,
                satStatus.fecCorrected,
                satStatus.fecUncorrected,
                satStatus.fecClean);
        return (satStatus.demodLocked);
    }
    return 0;
}

void get_bert_status(NEXUS_FrontendHandle frontend)
{
    NEXUS_FrontendBertStatus bertStatus;
    NEXUS_Frontend_GetBertStatus(frontend, &bertStatus);
    printf(
        "  BERT Status\n"
        "    locked = %d\n"
        "    syncAcquired = %d\n"
        "    syncLost = %d\n"
        "    bitCount = 0x%08x%08x\n"
        "    errorCount = 0x%08x%08x\n"
        "    BER = %e\n",
        bertStatus.locked,
        bertStatus.syncAcquired,
        bertStatus.syncLost,
        (uint32_t)((bertStatus.bitCount >> 32) & 0xffffffff), (uint32_t)(bertStatus.bitCount & 0xffffffff),
        (uint32_t)((bertStatus.errorCount >> 32) & 0xffffffff), (uint32_t)(bertStatus.errorCount & 0xffffffff),
        (float)bertStatus.errorCount/bertStatus.bitCount);
}

int SatTest(void)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendSatelliteSettings satSettings;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_HdmiOutputStatus hdmiStatus;
    #endif
    NEXUS_Error rc;
    int channel=-1;
    int frequency=FREQ;
    unsigned int modulation=SATELLITE_MODE;
    unsigned int symbol_rate=SYMBOL_RATE;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.satellite = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        return -1;
    }

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    channel = userParams.id;

    while (1)
    {
        printf("\n\n");
        printf("================\n");
        printf("  Satellite Test Menu  \n");
        printf("================\n");
        printf("    0) Exit\n");
        printf("    1) Change channel number (current channel=%d)\n", channel);
        printf("    2) Change frequency (current frequency=%d)\n", frequency);
        printf("    3) Change modulation (current modulation=%d)\n", modulation);
        printf("    4) Change symbol rate (current symbol rate=%d)\n", symbol_rate);
        printf("    5) Acquire and decode video\n");
        printf("    6) Change BERT enable (current setting is %s)\n", bertEnable ? "yes" : "no");
        printf("    7) Change PN data invert (current setting is %s)\n", pnDataInvert ? "yes" : "no");

        switch(Prompt()) {
            case 0:
                return 0;

            case 1:
                while (1) {
                    printf("Enter channel number\n");
                    channel = NoPrompt();

                    if (frontend)
                        NEXUS_Frontend_Release(frontend);

                    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
                    settings.mode=NEXUS_FrontendAcquireMode_eByIndex;
                    settings.index=channel;
                    frontend = NEXUS_Frontend_Acquire(&settings);
                    if (frontend) break;
                }

                break;

            case 2:
                while (1) {
                    printf("Enter frequency\n");
                    frequency = get_number();
                    break;
                }

                break;

            case 3:
                while (1) {
                    printf("%d=NEXUS_FrontendSatelliteMode_eDvb\n", NEXUS_FrontendSatelliteMode_eDvb);
                    printf("%d=NEXUS_FrontendSatelliteMode_eDss\n", NEXUS_FrontendSatelliteMode_eDss);
                    printf("%d=NEXUS_FrontendSatelliteMode_eDcii\n", NEXUS_FrontendSatelliteMode_eDcii);
                    printf("%d=NEXUS_FrontendSatelliteMode_eQpskTurbo\n", NEXUS_FrontendSatelliteMode_eQpskTurbo);
                    printf("%d=NEXUS_FrontendSatelliteMode_e8pskTurbo\n", NEXUS_FrontendSatelliteMode_e8pskTurbo);
                    printf("%d=NEXUS_FrontendSatelliteMode_eTurbo\n", NEXUS_FrontendSatelliteMode_eTurbo);
                    printf("%d=NEXUS_FrontendSatelliteMode_eQpskLdpc\n", NEXUS_FrontendSatelliteMode_eQpskLdpc);
                    printf("%d=NEXUS_FrontendSatelliteMode_e8pskLdpc\n", NEXUS_FrontendSatelliteMode_e8pskLdpc);
                    printf("%d=NEXUS_FrontendSatelliteMode_eLdpc\n", NEXUS_FrontendSatelliteMode_eLdpc);
                    printf("%d=NEXUS_FrontendSatelliteMode_eBlindAcquisition\n", NEXUS_FrontendSatelliteMode_eBlindAcquisition);

                    printf("Enter modulation\n");
                    modulation = NoPrompt();

                    if (modulation < NEXUS_FrontendSatelliteMode_eMax) break;
                }

                break;

            case 4:
                printf("Enter symbol rate\n");
                symbol_rate = get_number();
                break;

            case 5:

                NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
                satSettings.frequency = frequency;
                printf("modulation=%d (before)\n", satSettings.mode);
                satSettings.mode = modulation;
                printf("modulation=%d (after)\n", satSettings.mode);
                satSettings.lockCallback.callback = lock_callback;
                satSettings.lockCallback.context = frontend;
                satSettings.bertEnable = bertEnable;
                satSettings.pnDataInvert = pnDataInvert;
                satSettings.symbolRate = symbol_rate;
                NEXUS_Frontend_GetUserParameters(frontend, &userParams);

                /* Map a parser band to the demod's input band. */
                parserBand = PARSER_BAND;
                NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
                if (userParams.isMtsif) {
                    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
                    parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
                }
                else {
                    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
                    parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
                }
                parserBandSettings.transportType = TRANSPORT_TYPE;
                NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

                {
                    NEXUS_FrontendCapabilities capabilities;
                    NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
                    diseqcSupport = capabilities.diseqc;
                }

                if (diseqcSupport) {
                    NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
                    diseqcSettings.toneEnabled = TONE_ENABLED;
                    diseqcSettings.toneMode = TONE_MODE;
                    diseqcSettings.voltage = VOLTAGE;
                    NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
                    printf("Set DiseqcSettings\n");
                }

                NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
                stcSettings.timebase = NEXUS_Timebase_e0;
                stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

                audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
                NEXUS_AudioOutput_AddInput(
                    NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                    NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                #if NEXUS_NUM_HDMI_OUTPUTS
                    NEXUS_AudioOutput_AddInput(
                        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                #endif

                NEXUS_Display_GetDefaultSettings(&displaySettings);
                displaySettings.format = NEXUS_VideoFormat_eNtsc;
                display = NEXUS_Display_Open(0, &displaySettings);
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

                window = NEXUS_VideoWindow_Open(display, 0);
                videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
                NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

                videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
                audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);

                NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
                videoProgram.codec = VIDEO_CODEC;
                videoProgram.pidChannel = videoPidChannel;
                videoProgram.stcChannel = stcChannel;
                NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
                audioProgram.codec = AUDIO_CODEC;
                audioProgram.pidChannel = audioPidChannel;
                audioProgram.stcChannel = stcChannel;

                NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
                stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
                stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
                rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
                BDBG_ASSERT(!rc);

                rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
                BDBG_ASSERT(!rc);

                while (1) {
                    if (get_sat_status(frontend)) break;
                    BKNI_Sleep(1000);
                }

                if (bertEnable) {
                    int rc;
                    pthread_attr_t threadAttr;
                    struct sched_param schedParam;

                    /* Create thread for RF4CE callbacks */
                    pthread_attr_init(&threadAttr);
                    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
                    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
                    pthread_attr_getschedparam(&threadAttr, &schedParam);
                    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
                    pthread_attr_setschedparam(&threadAttr, &schedParam);
                    pthread_attr_setstacksize(&threadAttr, 8*1024);
                    rc = pthread_create(&bertThread,
                                        &threadAttr,
                                        bertHandler,
                                        NULL);
                    if ( rc ) {
                        printf("Unable to create thread");
                        assert(1);
                    }

                    quit = false;
                    reset = false;
                    while (1) {
                        get_sat_status(frontend);
                        get_bert_status(frontend);
                        BKNI_Sleep(1000);
                        if (quit) break;
                        if (reset) {
                            reset = false;
                            NEXUS_Frontend_ResetStatus(frontend);
                        }
                    }
                    pthread_cancel(bertThread);
                } else {
                    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
                    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

                        printf("Press enter to exit\n");
                        getchar();

                    NEXUS_AudioDecoder_Stop(audioDecoder);
                    NEXUS_VideoDecoder_Stop(videoDecoder);
                    NEXUS_PidChannel_Close(videoPidChannel);
                    NEXUS_PidChannel_Close(audioPidChannel);

                    NEXUS_VideoDecoder_Close(videoDecoder);
                    NEXUS_VideoWindow_Close(window);

                    NEXUS_Display_Close(display);

                    NEXUS_AudioDecoder_Close(audioDecoder);
                    NEXUS_StcChannel_Close(stcChannel);
                }

                break;

            case 6:
                bertEnable = bertEnable ? false : true;
                break;

            case 7:
                pnDataInvert = pnDataInvert ? false : true;
                break;

            default:
                printf("Invalid selection\n");
                break;
        }
    }

    return 0;
}
#endif /* if NEXUS_HAS_FRONTEND */
