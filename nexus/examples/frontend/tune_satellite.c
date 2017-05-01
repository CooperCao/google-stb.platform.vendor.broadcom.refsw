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
/* Example to tune a satellite channel using nexus */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_frontend.h"
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

#if NEXUS_FRONTEND_7366
#include "nexus_frontend_7366.h"
#endif
#if NEXUS_FRONTEND_4538
#include "nexus_frontend_4538.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define USE_DVB 1 /* if set to 0 or undefined, use DSS */

#define PARSER_BAND NEXUS_ParserBand_e2; /* Change this value to match your platform */

bool diseqcSupport = false;

 static void print_usage(void)
 {
     printf(
         "Usage: tune_satellite\n"
         "  --help or -h for help\n"
         "  -mode {0,7,1} DVB-S=0, DVB-S2(8pskldpc)=7, DSS=1 (default is DVB-S)\n"
         "  -freq   #     frequency in MHz, e.g. 1119, or Hz, e.g. 1119000000\n"
         "  -sym    #     symbol rate in Mbaud, e.g. 20, 22.5, or baud, e.g. 20000000\n"
         "  -vpid   #     video PID (default: 0x31)\n"
         "  -apid   #     audio PID (default: 0x34)\n"
         "  -pcrpid #     PCR PID (default: video PID)\n"
         "  -vcodec       MPEG2=2, AVC=5 (default is MPEG2)\n"
         "  -acodec       MPEG=1, AAC=3, AACplus=5, AC3=7 (default is AC3)\n"
         );
     printf(
         "  -nodsq        disable diseqc setting/status\n"
         "  -voltage #    13 or 18\n"
         "  -tone #       off=0, on=1\n"
         "  -tonemode #   tone=0, envelope=1 (default: envelope)\n"
         "  -adc #        remap to adc #"
#if NEXUS_USE_7445_DBS
         " (for most 7445DBS boards, this needs to be 1)"
#endif
         "\n"
         );
 }

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendSatelliteStatus status;
    NEXUS_FrontendDiseqcStatus disqecStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Frontend(%p) - lock callback\n", (void*)frontend);

    NEXUS_Frontend_GetSatelliteStatus(frontend, &status);
    fprintf(stderr, "  demodLocked = %s\n", status.demodLocked ? "locked" : "unlocked");
    if (diseqcSupport) {
        NEXUS_Frontend_GetDiseqcStatus(frontend, &disqecStatus);
        fprintf(stderr, "  diseqc tone = %s, voltage = %d\n", disqecStatus.toneEnabled ? "on" : "off", disqecStatus.voltage);
    }
}

int main(int argc, char **argv)
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
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    bool done = false;

    int newAdc = 0;
    bool isNewAdc = false;
    bool disableDiseqc = false;
    int newFrontend = 0;
    bool specifyFrontend = false;
    unsigned frequency = 1119000000;
    unsigned symRate = 20000000;
    NEXUS_FrontendSatelliteMode mode = NEXUS_FrontendSatelliteMode_eDvb;
    NEXUS_TransportType transportType = NEXUS_TransportType_eTs;
    bool toneEnabled = true;
    NEXUS_FrontendDiseqcToneMode toneMode = NEXUS_FrontendDiseqcToneMode_eEnvelope;
    NEXUS_FrontendDiseqcVoltage voltage = NEXUS_FrontendDiseqcVoltage_e13v;

    int curarg = 1;
    int videoPid = 0x31, audioPid = 0x34, pcrPid = -1;
    unsigned int videoCodec = NEXUS_VideoCodec_eMpeg2;
    unsigned int audioCodec = NEXUS_AudioCodec_eAc3;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-mode") && argc>curarg+1) {
            mode = atoi(argv[++curarg]);
            if (mode == 1) {
                transportType = NEXUS_TransportType_eDssEs;
            }
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            float f;
            if (sscanf(argv[++curarg], "%f", &f) != 1) f = 0;
            if (f < 1000000)
                frequency = (unsigned)(f*1000) * 1000;
            else
                frequency = (unsigned)f;
        }
        else if (!strcmp(argv[curarg], "-sym") && argc>curarg+1) {
            const char *s = argv[++curarg];
            if (strchr(s,'.')) {
                float n;
                sscanf(s, "%f", &n);
                symRate = ((unsigned)(n*100.0))*10000;
            } else {
                symRate = atoi(s);
                if (symRate < 1000000)
                    symRate *= 1000000;
            }
        }
        else if (!strcmp(argv[curarg], "-vpid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                videoPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                videoPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-apid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                audioPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                audioPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-pcrpid")) {
            if((!strncmp(argv[++curarg], "0X", 2)) || (!strncmp(argv[curarg], "0x", 2))){
                pcrPid = strtol(argv[curarg], NULL, 0);
            }
            else{
                pcrPid = atoi(argv[curarg]);
            }
        }
        else if (!strcmp(argv[curarg], "-vcodec")) {
            videoCodec = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-acodec")) {
            audioCodec = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-adc") && argc>curarg+1) {
            newAdc = atoi(argv[++curarg]);
            isNewAdc = true;
        }
        else if (!strcmp(argv[curarg], "-nodsq")) {
            disableDiseqc = true;
        }
        else if (!strcmp(argv[curarg], "-tone") && argc>curarg+1) {
            toneEnabled = atoi(argv[++curarg]) != 0;
        }
        else if (!strcmp(argv[curarg], "-voltage") && argc>curarg+1) {
            int v = atoi(argv[++curarg]);
            if (v==13)
                voltage = NEXUS_FrontendDiseqcVoltage_e13v;
            else if (v==18)
                voltage = NEXUS_FrontendDiseqcVoltage_e18v;
            else {
                print_usage();
                return 1;
            }
        }
        else if (!strcmp(argv[curarg], "-demod") && argc>curarg+1) {
            specifyFrontend = true;
            newFrontend = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

#if NEXUS_USE_7445_DBS
    if (!specifyFrontend && !isNewAdc) {
        isNewAdc = true;
        newAdc = 1;
    }
#endif

    /* If the PCR PID hasn't been set but the video PID has then assume PCR is on the video */
    if ((pcrPid == -1) && (videoPid != -1)) {
        pcrPid = videoPid;
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (!specifyFrontend) {
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.satellite = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
    } else {
        frontend = platformConfig.frontend[newFrontend];
    }

    if (!frontend) {
        fprintf(stderr, "Unable to find satellite-capable frontend\n");
        return -1;
    }

    if (isNewAdc) {
        NEXUS_FrontendSatelliteRuntimeSettings settings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(frontend, &settings);
        settings.selectedAdc = newAdc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(frontend, &settings);
    }

    NEXUS_Frontend_GetDefaultSatelliteSettings(&satSettings);
    satSettings.frequency = frequency;
    satSettings.mode = mode;
    satSettings.symbolRate = symRate;
    satSettings.lockCallback.callback = lock_callback;
    satSettings.lockCallback.context = frontend;
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
    parserBandSettings.transportType = transportType;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    if (!disableDiseqc) {
        NEXUS_FrontendCapabilities capabilities;
        NEXUS_Frontend_GetCapabilities(frontend, &capabilities);
        diseqcSupport = capabilities.diseqc;
    }

    if (diseqcSupport) {
        NEXUS_Frontend_GetDiseqcSettings(frontend, &diseqcSettings);
        diseqcSettings.toneEnabled = toneEnabled;
        diseqcSettings.toneMode = toneMode;
        diseqcSettings.voltage = voltage;
        NEXUS_Frontend_SetDiseqcSettings(frontend, &diseqcSettings);
        printf("Set DiseqcSettings\n");
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0]) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    if (NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
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

    while (!done) {
        NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
        NEXUS_VideoDecoderStartSettings videoProgram;
        NEXUS_AudioDecoderStartSettings audioProgram;

        videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = videoCodec;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = audioCodec;
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
        rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Frontend_TuneSatellite(frontend, &satSettings);
        BDBG_ASSERT(!rc);

        while (1)
        {
            NEXUS_FrontendSatelliteStatus satStatus;
            rc = NEXUS_Frontend_GetSatelliteStatus(frontend, &satStatus);
            if (rc) {
                printf("unable to read status\n");
            }
            else {
                printf(
                    "Sat Status\n"
                    "  symbolRate %d\n"
                    "  locked  %d\n",
                        satStatus.settings.symbolRate,
                        satStatus.demodLocked);
                if(satStatus.demodLocked)
                    break;
            }
            BKNI_Sleep(1000);
        }

        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
        NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

        printf("Press enter to tune again (or 'q' to exit)\n");
        {
            char s[512];
            memset(s,0,512);
            fgets(s, 512, stdin);
            s[strlen(s)-1] = 0; /* chop off \n */
            if (s[0]=='q')
                done = true;
        }

        NEXUS_AudioDecoder_Stop(audioDecoder);
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(audioPidChannel);
    }

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);

    NEXUS_Platform_Uninit();
    return 0;
}
#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
