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
/* Example to tune a OFDM channel using nexus */
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
#if NEXUS_HAS_COMPONENT_OUTPUT
#include "nexus_component_output.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(tune_ofdm);

static void print_usage(void)
{
    printf(
        "Usage: tune_ofdm\n"
        "  --help or -h for help\n"
        "  -mode {0,1,3} DVB-T=0, DVB-T2=1, ISDB-T=3 (default is DVB-T)\n"
        "  -freq   #     frequency in MHz\n"
        "  -vpid   #     video PID\n"
        "  -apid   #     audio PID\n"
        "  -pcrpid #     PCR PID\n"
        "  -vcodec       MPEG2=2, AVC=5 (default is MPEG2)\n"
        "  -acodec       MPEG=1, AAC=3, AACplus=5 (default is MPEG)\n"
        "  -lite         Enable DVB-T2 lite profile\n"
        "  -plp    #     plp ID\n"
        );
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;

    BSTD_UNUSED(param);
    NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
        printf("OFDM Lock callback: Fast lock status = Unlocked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
        printf("OFDM Lock callback: Fast lock status = Locked.\n");
    else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
        printf("OFDM Lock callback: Fast lock status = NoSignal.\n");
    else
        printf("OFDM Lock callback: Fast lock status = Unknown.\n");
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    fprintf(stderr, "async_status_ready_callback\n");
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_FrontendOfdmMode mode = NEXUS_FrontendOfdmMode_eDvbt;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel = NULL;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display = NULL;
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_DisplayHandle displaySD = NULL;
#endif
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window = NULL;
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_VideoWindowHandle windowSD = NULL;
#endif
    NEXUS_VideoDecoderHandle videoDecoder = NULL;
    NEXUS_AudioDecoderHandle pcmDecoder = NULL, compressedDecoder = NULL;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    unsigned int i = 0, j = 0, statusMax = 0;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FrontendIsdbtStatusReady isdbtStatusReady;
    NEXUS_FrontendIsdbtStatus isdbtStatus;
    NEXUS_FrontendDvbtStatusReady dvbtStatusReady;
    NEXUS_FrontendDvbtStatus dvbtStatus;
    NEXUS_FrontendDvbt2StatusReady dvbt2StatusReady;
    NEXUS_FrontendDvbt2Status dvbt2Status;
    BKNI_EventHandle statusEvent;

    bool dvbT2Lite = false, plp = false;
    int curarg = 1;
    unsigned int freq = 0;
    int videoPid = -1, audioPid = -1, pcrPid = -1, plpId=0;
    unsigned int videoCodec = NEXUS_VideoCodec_eMpeg2;
    unsigned int audioCodec = NEXUS_AudioCodec_eMpeg;

    NEXUS_Error (*RequestAsyncStatus)(NEXUS_FrontendHandle, unsigned int);

    /* Read command line for freqency, pid and codec information */

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-mode")) {
            mode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            float f;
            if (sscanf(argv[++curarg], "%f", &f) != 1) f = 0;
            if (f < 1000000)
                freq = (unsigned)(f*1000) * 1000;
            else
                freq = (unsigned)f;
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
        else if (!strcmp(argv[curarg], "-lite")) {
            dvbT2Lite = true;
        }
        else if (!strcmp(argv[curarg], "-plp")) {
            plp = true;
            plpId = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /* If the PCR PID hasn't been set but the video PID has then assume PCR is on the video */
    if ((pcrPid == -1) && (videoPid != -1)) {
        pcrPid = videoPid;
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.ofdm = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find OFDM-capable frontend\n");
        return -1;
    }

    BKNI_CreateEvent(&statusEvent);

    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);

    if (mode == NEXUS_FrontendOfdmMode_eDvbt) {
        /* If freq and video PID not set on command line use defaults */
        if (freq == 0) {
            freq = 578000000;
            printf("Using default frequency for DVB-T %d MHz\n", freq/1000000);
        }
        if (videoPid == -1) {
            printf("Using built in default pids and codecs for DVB-T\n");
            videoPid = 0x579;
            audioPid = 0x57a;
            pcrPid = 0x579;

            videoCodec = NEXUS_VideoCodec_eMpeg2;
            audioCodec = NEXUS_AudioCodec_eMpeg;
        }

        ofdmSettings.bandwidth = 8000000;
        ofdmSettings.manualTpsSettings = false;
        ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
        ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;

        statusMax = NEXUS_FrontendDvbtStatusType_eMax;
        RequestAsyncStatus = NEXUS_Frontend_RequestDvbtAsyncStatus;

     }
     else if (mode == NEXUS_FrontendOfdmMode_eDvbt2) {
        /* If freq and video PID not set on command line use defaults */
        if (freq == 0) {
            freq = 602000000;
            printf("Using default frequency for DVB-T2 %d MHz\n", freq/1000000);
        }
        if (videoPid == -1) {
            printf("Using built in default pids and codecs for DVB-T2\n");
            videoPid = 0x65;
            audioPid = 0x66;
            pcrPid = 0x65;

            videoCodec = NEXUS_VideoCodec_eH264;
            audioCodec = NEXUS_AudioCodec_eAacPlus;
        }

        ofdmSettings.bandwidth = 8000000;
        ofdmSettings.dvbt2Settings.plpId = plpId;
        if (plp)
            ofdmSettings.dvbt2Settings.plpMode = false;
        else
            ofdmSettings.dvbt2Settings.plpMode = true;

        if (dvbT2Lite) {
            ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eLite;
        } else {
            ofdmSettings.dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
        }
        statusMax = NEXUS_FrontendDvbt2StatusType_eMax;
        RequestAsyncStatus = NEXUS_Frontend_RequestDvbt2AsyncStatus;
    }
    else if(mode == NEXUS_FrontendOfdmMode_eIsdbt) {
        /* If freq and video PID not set on command line use defaults */
        if (freq == 0) {
            freq = 473000000;
            printf("Using default frequency for ISDB-T %d MHz\n", freq/1000000);
        }
        if (videoPid == -1) {
            printf("Using built in default pids and codecs for ISDB-T\n");
            videoPid = 0x230;
            audioPid = 0x300;
            pcrPid = 0x150;
            videoCodec = NEXUS_VideoCodec_eMpeg2;
            audioCodec = NEXUS_AudioCodec_eAac;
        }

        ofdmSettings.bandwidth = 6000000;

        statusMax = NEXUS_FrontendIsdbtStatusType_eMax;
        RequestAsyncStatus = NEXUS_Frontend_RequestIsdbtAsyncStatus;
    }

    printf("\nTuning Parameters: mode: %s frequency: %d videoPid: %d audioPid: %d videoCodec: %s audioCodec: %s \n\n",
           (mode == NEXUS_FrontendOfdmMode_eDvbt2) ? "DVB-T2" : (mode == NEXUS_FrontendOfdmMode_eDvbt) ? "DVB-T" : "ISDB-T",
           freq/1000000,videoPid, audioPid,
           (videoCodec == 2)? "MPEG2" : (videoCodec == 5)? "AVC" : "Unknown",
           (audioCodec == 1)? "MPEG"  : (audioCodec == 3)? "AAC" : (audioCodec == 5) ? "AACplus" : "Unknown");

    ofdmSettings.frequency = freq;
    ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ofdmSettings.terrestrial = true;
    ofdmSettings.spectrumMode = NEXUS_FrontendOfdmSpectrumMode_eAuto;
    ofdmSettings.mode = mode;
    ofdmSettings.lockCallback.callback = lock_callback;
    ofdmSettings.lockCallback.context = frontend;
    ofdmSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    ofdmSettings.asyncStatusReadyCallback.context = statusEvent;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    parserBand = NEXUS_ParserBand_e0;
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
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);

    if (videoPid != -1) {
        videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        pcrPidChannel = NEXUS_PidChannel_Open(parserBand, pcrPid, NULL);

        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
    }
    if (audioPid != -1) {
        audioPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
    }

    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */

    if (videoPid != -1) {
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = videoCodec;
        videoProgram.pidChannel = videoPidChannel;
        videoProgram.stcChannel = stcChannel;
    }

    if (audioPid != -1) {
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = audioCodec;
        audioProgram.pidChannel = audioPidChannel;
        audioProgram.stcChannel = stcChannel;

        pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
        compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);

        if (platformConfig.outputs.audioDacs[0]) {
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
        if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 ) {
            /* Only pass through AC3 */
            if (NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0])) {
                NEXUS_AudioOutput_AddInput(
                    NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            }
#if NEXUS_HAS_HDMI_OUTPUT
            if (NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0])) {
                NEXUS_AudioOutput_AddInput(
                    NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
            }
#endif
        } else {
            if (NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0])) {
                NEXUS_AudioOutput_AddInput( NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            }
#if NEXUS_HAS_HDMI_OUTPUT
            if (NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0])) {
                NEXUS_AudioOutput_AddInput(
                    NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                    NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
            }
#endif
        }
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    /* If we have a composite output connected it to the decoder as a secondary SD display */
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySD = NEXUS_Display_Open(1, &displaySettings);
    windowSD = NEXUS_VideoWindow_Open(displaySD, 0);

    NEXUS_Display_AddOutput(displaySD, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( (rc != NEXUS_SUCCESS) && hdmiStatus.connected ) {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_VideoWindow_AddInput(windowSD, NEXUS_VideoDecoder_GetConnector(videoDecoder));
#endif

    NEXUS_Frontend_TuneOfdm(frontend, &ofdmSettings);

    if (videoPid != -1) {
        NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    }

    if (audioPid != -1) {
        NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
        if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
        {
            /* Only pass through AC3 */
            NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
        }
    }

    printf("Press enter to get partial status data.\n");
    getchar();

    for ( i = 0; i < statusMax; ++i) {

        rc = RequestAsyncStatus(frontend,i);
        if (rc) {rc = BERR_TRACE(rc); goto done; }

        rc = BKNI_WaitForEvent(statusEvent, 1000);
        if (rc) {rc = BERR_TRACE(rc); goto done; }

        if (mode == NEXUS_FrontendOfdmMode_eDvbt)
            rc = NEXUS_Frontend_GetDvbtAsyncStatusReady(frontend, &dvbtStatusReady);
        else if (mode == NEXUS_FrontendOfdmMode_eDvbt2)
            rc = NEXUS_Frontend_GetDvbt2AsyncStatusReady(frontend, &dvbt2StatusReady);
        else if (mode == NEXUS_FrontendOfdmMode_eIsdbt)
            rc = NEXUS_Frontend_GetIsdbtAsyncStatusReady(frontend, &isdbtStatusReady);

        if (rc) {rc = BERR_TRACE(rc); goto done;}

        for (j=0; j < statusMax; j++) {
            bool ready;

            if (mode == NEXUS_FrontendOfdmMode_eDvbt)
                ready = dvbtStatusReady.type[j];
            else if (mode == NEXUS_FrontendOfdmMode_eDvbt2)
                ready = dvbt2StatusReady.type[j];
            else if (mode == NEXUS_FrontendOfdmMode_eIsdbt)
                ready = isdbtStatusReady.type[j];

            if (ready)
            {
                printf("Retrieving status type %d.\n", j);

                if (mode == NEXUS_FrontendOfdmMode_eDvbt) {
                    rc = NEXUS_Frontend_GetDvbtAsyncStatus(frontend, (NEXUS_FrontendDvbtStatusType)j, &dvbtStatus);
                    if (rc) {rc = BERR_TRACE(rc); goto done;}
                    if (j == NEXUS_FrontendDvbtStatusType_eBasic) {
                        printf("Ofdm lock = %d\n", dvbtStatus.status.basic.fecLock);
                    }
                }
                else if (mode == NEXUS_FrontendOfdmMode_eDvbt2) {
                    rc = NEXUS_Frontend_GetDvbt2AsyncStatus(frontend, (NEXUS_FrontendDvbt2StatusType)j, &dvbt2Status);
                    if (rc) {rc = BERR_TRACE(rc); goto done;}
                }
                else if (mode == NEXUS_FrontendOfdmMode_eIsdbt) {
                    rc = NEXUS_Frontend_GetIsdbtAsyncStatus(frontend, (NEXUS_FrontendIsdbtStatusType)j, &isdbtStatus);
                    if (rc) {rc = BERR_TRACE(rc); goto done;}
                    if (j == NEXUS_FrontendIsdbtStatusType_eBasic) {
                        printf("Ofdm lock = %d\n", isdbtStatus.status.basic.fecLock);
                    }
                }

                printf("Press enter to get next partial status.\n");
                getchar();
            }
        }
    }

    printf("Press enter to exit.\n");
    getchar();

done:
    /* example shutdown */

    if (audioPid != -1) {
        NEXUS_AudioDecoder_Stop(pcmDecoder);
        if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 ) {
            NEXUS_AudioDecoder_Stop(compressedDecoder);
        }
    }

    if (videoPid != -1) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(pcrPidChannel);
    }

    if (audioPid != -1) {
        NEXUS_PidChannel_Close(audioPidChannel);
    }

    NEXUS_Frontend_Untune(frontend);
    if (statusEvent) {
        BKNI_DestroyEvent(statusEvent);
    }

    if (audioPid != -1) {
        if (platformConfig.outputs.audioDacs[0])
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
        if (platformConfig.outputs.spdif[0])
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
        if (platformConfig.outputs.hdmi[0])
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
        NEXUS_AudioDecoder_Close(pcmDecoder);
        NEXUS_AudioDecoder_Close(compressedDecoder);
    }

    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_VideoWindow_Close(windowSD);
    NEXUS_Display_Close(displaySD);
#endif
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Frontend_Release(frontend);
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
