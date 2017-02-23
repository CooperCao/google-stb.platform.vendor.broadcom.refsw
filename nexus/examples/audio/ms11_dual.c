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
 *
 * Module Description:
 *
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_audio_input.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(ms11_dual);

/* Change this to true to enable SPDIF re-encoding to AC3 */
static bool g_compressedSpdif = false;

/* Change this to true to use Dolby Volume 258 on the mixed data */
static bool g_useDv258 = false;

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, mainPidChannel, secondaryPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle mainDecoder = NULL, secondaryDecoder = NULL;
    NEXUS_AudioDecoderStartSettings mainProgram, secondaryProgram;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioMixerHandle mixer = NULL;
    NEXUS_DolbyDigitalReencodeHandle ddre = NULL;
    NEXUS_DolbyVolume258Handle dv258;
    NEXUS_AudioCodec codec = NEXUS_AudioCodec_eAc3Plus;
    unsigned audioPid=48, secondaryPid=49, pcrPid=50;
    int i;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    NEXUS_AudioOutputHandle audioDacHandle = NULL;
    NEXUS_AudioOutputHandle audioSpdifHandle = NULL;
    NEXUS_AudioOutputHandle audioHdmiHandle = NULL;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
#if NEXUS_AUDIO_MODULE_FAMILY != NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    platformSettings.audioModuleSettings.maximumProcessingBranches = 2;
#endif
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders == 0 || !audioCapabilities.dsp.mixer || !audioCapabilities.dsp.dolbyDigitalReencode)
    {
        printf("This application is not supported on this platform (needs decoders and mixers and DDRE)!\n");
        return 0;
    }

    if (audioCapabilities.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }

    if (audioCapabilities.numOutputs.spdif > 0)
    {
        audioSpdifHandle = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioCapabilities.numOutputs.hdmi > 0)
    {
        audioHdmiHandle = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
    }
    #endif

    /* Parse command-line arguments */
    for ( i = 1; i < argc; i++ )
    {
        if ( !strcmp(argv[i], "-compressed_audio") )
        {
            g_compressedSpdif = true;
        }
        else if ( !strcmp(argv[i], "-dv258") )
        {
            g_useDv258 = true;
        }
        else if ( !strcmp(argv[i], "-audio_type") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "ac3") || !strcmp(argv[i], "ac3plus") )
            {
                codec = NEXUS_AudioCodec_eAc3Plus;
            }
            else if ( !strcmp(argv[i], "aac") || !strcmp(argv[i], "aac_adts") || !strcmp(argv[i], "aacplus_adts") )
            {
                codec = NEXUS_AudioCodec_eAacPlusAdts;
            }
            else if ( !strcmp(argv[i], "aacplus") || !strcmp(argv[i], "aac_loas") || !strcmp(argv[i], "aacplus_loas") )
            {
                codec = NEXUS_AudioCodec_eAacPlusLoas;
            }
            else
            {
                BDBG_ERR(("Codec %s is not supported by this application", argv[i]));
                return -1;
            }
        }
        else if ( !strcmp(argv[i], "-audio") && (i+1) < argc )
        {
            i++;
            audioPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-pcr") && (i+1) < argc )
        {
            i++;
            pcrPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-secondary_audio") && (i+1) < argc )
        {
            i++;
            secondaryPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-no_secondary_audio") && (i+1) < argc )
        {
            secondaryPid = 0;
        }
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
        else if ( !strcmp(argv[i], "-loudness") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "atsc") )
            {
                platformSettings.audioModuleSettings.loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eAtscA85;
            }
            else if ( !strcmp(argv[i], "ebu") )
            {
                platformSettings.audioModuleSettings.loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eEbuR128;
            }
        }
#endif
        else 
        {
            BDBG_WRN(("usage: %s [-compressed_audio] [-dv258] [-audio_type <ac3plus|aac_adts|aac_loas>] [-audio <pid>] [-secondary_audio <pid>] [-pcr <pid>]", argv[0]));
            return -1;
        }
    }

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, pcrPid, NULL);
    mainPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
    if ( secondaryPid > 0 )
    {
        secondaryPidChannel = NEXUS_PidChannel_Open(parserBand, secondaryPid, NULL);
    }
    else
    {
        secondaryPidChannel = NULL;
    }

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&mainProgram);
    mainProgram.codec = codec;
    mainProgram.pidChannel = mainPidChannel;
    mainProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&secondaryProgram);
    secondaryProgram.codec = codec;
    secondaryProgram.pidChannel = secondaryPidChannel;
    secondaryProgram.stcChannel = stcChannel;
    secondaryProgram.secondaryDecoder = true;   /* Indicate this is a secondary channel for STC Channel/PCRlib functions */

    /* Bring up the primary audio decoder */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    mainDecoder = NEXUS_AudioDecoder_Open(0, &audioOpenSettings);
    if (!mainDecoder)
    {
        printf("Unable to open Main Decoder!\n");
        return 0;
    }

    /* Open audio descriptor decoder */
    if ( audioCapabilities.numDecoders > 1 )
    {
        secondaryDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);
    }

    /* Open mixer to mix the description and primary audio */
    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);

    /* Add both decoders to the mixer */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

    if (secondaryDecoder)
    {
        NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
    }

    /* Set the Mixer to use DSP mixing */
    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel);
    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);

    /* Add the DDRE processor after mixing */
    ddre = NEXUS_DolbyDigitalReencode_Open(NULL);

    if ( g_useDv258 )
    {
        /* Open DV258 and add between mixer and DDRE */
        dv258 = NEXUS_DolbyVolume258_Open(NULL);
        NEXUS_DolbyVolume258_AddInput(dv258, NEXUS_AudioMixer_GetConnector(mixer));
        NEXUS_DolbyDigitalReencode_AddInput(ddre, NEXUS_DolbyVolume258_GetConnector(dv258));
    }
    else
    {
        /* Just connect DDRE to the mixer output */
        NEXUS_DolbyDigitalReencode_AddInput(ddre, NEXUS_AudioMixer_GetConnector(mixer));
    }

    if (audioDacHandle) {
            /* Add DAC and SPDIF */
        NEXUS_AudioOutput_AddInput(
            audioDacHandle,
            NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
    }
    if ( g_compressedSpdif )
    {
        if (audioSpdifHandle) {
            NEXUS_AudioOutput_AddInput(
                audioSpdifHandle,
                NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_DolbyDigitalReencodeConnectorType_eCompressed));
        }
        #if NEXUS_NUM_HDMI_OUTPUTS
        if (audioHdmiHandle) {
            NEXUS_AudioOutput_AddInput(
                audioHdmiHandle,
                NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_DolbyDigitalReencodeConnectorType_eCompressed));
        }
        #endif
    }
    else
    {
        if (audioSpdifHandle) {
            NEXUS_AudioOutput_AddInput(
                audioSpdifHandle,
                NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
        }
        #if NEXUS_NUM_HDMI_OUTPUTS
        if (audioHdmiHandle) {
            NEXUS_AudioOutput_AddInput(
                audioHdmiHandle,
                NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_DolbyDigitalReencodeConnectorType_eStereo));
        }
        #endif
    }

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
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
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    BDBG_WRN(("Starting Main"));
    NEXUS_AudioDecoder_Start(mainDecoder, &mainProgram);

    if ( secondaryPidChannel && secondaryDecoder )
    {
        BDBG_WRN(("Starting Secondary"));
        NEXUS_AudioDecoder_Start(secondaryDecoder, &secondaryProgram);
    }

    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    if ( secondaryPidChannel && secondaryDecoder)
    {
        NEXUS_AudioDecoder_Stop(secondaryDecoder);
    }
    NEXUS_AudioDecoder_Stop(mainDecoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(mixer));
    NEXUS_AudioMixer_Close(mixer);
    NEXUS_DolbyDigitalReencode_Close(ddre);
    if (g_useDv258)
    {
        NEXUS_DolbyVolume258_Close(dv258);
    }
    if (secondaryDecoder)
    {
        NEXUS_AudioDecoder_Close(secondaryDecoder);
    }
    NEXUS_AudioDecoder_Close(mainDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(mainPidChannel);
    if ( secondaryPidChannel )
    {
        NEXUS_PidChannel_Close(secondaryPidChannel);
    }
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
