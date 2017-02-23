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
/* Nexus example app: single live a/v decode through 
    streamer => input_band => parser_band => remux => parser_band => decoders */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_input.h"
#include "nexus_audio_input.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3

#define VIDEO_PID_1 0x11
#define AUDIO_PID_1 0x14
#define VIDEO_PID_2 0x31
#define AUDIO_PID_2 0x34

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_RemuxHandle remux;
    NEXUS_RemuxSettings remuxSettings;
    NEXUS_ParserBand parserBand[4];
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel[2];
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle pcrPidChannel[2], pidChannelRemuxed[4];
    NEXUS_PidChannelHandle pidChannel[4];
    NEXUS_RemuxParserBandwidth remuxParserBandwidth;
    NEXUS_DisplayHandle display[2];
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window[2];
    NEXUS_VideoDecoderHandle vdecode[2];
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder[2];
    NEXUS_AudioDecoderStartSettings audioProgram;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    NEXUS_TransportCapabilities cap;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetTransportCapabilities(&cap);
    if (!cap.numRemux) {
        printf("This application is not supported on this chip. No remux.\n");
        return 0;
    }

    /* Map streamer input => input band 0 => parser band 0*/
    parserBand[0] = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand[0], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    parserBandSettings.allPass = false;
    NEXUS_ParserBand_SetSettings(parserBand[0], &parserBandSettings);

    /* Map streamer input => input band 1 => parser band 1*/
    parserBand[1] = NEXUS_ParserBand_e1;
    NEXUS_ParserBand_GetSettings(parserBand[1], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    parserBandSettings.allPass = false;
    NEXUS_ParserBand_SetSettings(parserBand[1], &parserBandSettings);

    /* Open pid channels 0 & 1 using parser band 0 and open pid channels 2 & 3 using parser band 1 */
    pidChannel[0] = NEXUS_PidChannel_Open(parserBand[0], VIDEO_PID_1, NULL);
    pidChannel[1] = NEXUS_PidChannel_Open(parserBand[0], AUDIO_PID_1, NULL);
    pidChannel[2] = NEXUS_PidChannel_Open(parserBand[1], VIDEO_PID_2, NULL);
    pidChannel[3] = NEXUS_PidChannel_Open(parserBand[1], AUDIO_PID_2, NULL);

    /* This is used for clock recovery. 
     * We can't use a PCR pid from a remux => parser_band loopback.
     * Open pcr pid channels 0 and 1 for 2 programs.
     */
    pcrPidChannel[0] = NEXUS_PidChannel_Open(parserBand[0], VIDEO_PID_1, NULL); 
    pcrPidChannel[1] = NEXUS_PidChannel_Open(parserBand[1], VIDEO_PID_2, NULL); 

    /* Open remux channel 0*/
    NEXUS_Remux_GetDefaultSettings(&remuxSettings);
    remuxSettings.allPass = false;

    /* Set the bandwidth for Parser band 0 that's going to be routed through the remux channel 0 */
    remux = NEXUS_Remux_Open(0, &remuxSettings);
    NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
    remuxParserBandwidth.maxDataRate = 30000000;
    remuxParserBandwidth.parserBand =  parserBand[0];
    NEXUS_Remux_SetParserBandwidth(remux,&remuxParserBandwidth);

    /* Set the bandwidth for Parser band 1 that's going to be routed through the remux channel 0 */
    NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
    remuxParserBandwidth.maxDataRate = 30000000;
    remuxParserBandwidth.parserBand =  parserBand[1];
    NEXUS_Remux_SetParserBandwidth(remux,&remuxParserBandwidth);

    /* Add the parser band 0 pid channels to the remux channel 0 */
    NEXUS_Remux_AddPidChannel(remux, pidChannel[0]);
    NEXUS_Remux_AddPidChannel(remux, pidChannel[1]);

    /* Add the parser band 1 pid channels to the remux channel 0 */
    NEXUS_Remux_AddPidChannel(remux, pidChannel[2]);
    NEXUS_Remux_AddPidChannel(remux, pidChannel[3]);

    NEXUS_Remux_Start(remux);

    /* First program sourced from remux channel 0 to first audio and video decoders and display */

   /* Map remux output to a parser band via loopback route */
    parserBand[2] = NEXUS_ParserBand_e2;
    NEXUS_ParserBand_GetSettings(parserBand[2], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = remux;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand[2], &parserBandSettings);

    /* Open the audio and video pid channels */
    pidChannelRemuxed[0] = NEXUS_PidChannel_Open(parserBand[2], VIDEO_PID_1, NULL);
    pidChannelRemuxed[1] = NEXUS_PidChannel_Open(parserBand[2], AUDIO_PID_1, NULL);

    /* Open the StcChannel 0 to do lipsync with the PCR for program 0 */ 
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel[0];
    stcChannel[0] = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs.  */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = pidChannelRemuxed[0];
    videoProgram.stcChannel = stcChannel[0];
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = pidChannelRemuxed[1];
    audioProgram.stcChannel = stcChannel[0];

    /* Bring up audio decoders and outputs */
    pcmDecoder[0] = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display[0] = NEXUS_Display_Open(0, &displaySettings);
    window[0] = NEXUS_VideoWindow_Open(display[0], 0);

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

    /* bring up decoder and connect to display */
    vdecode[0] = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window[0], NEXUS_VideoDecoder_GetConnector(vdecode[0]));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode[0], &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder[0], &audioProgram);
    
    /* Second program sourced from remux channel 0 to second audio and video decoders and second display */

    /* Map remux output to a parser band via loopback route */
    parserBand[3] = NEXUS_ParserBand_e3;
    NEXUS_ParserBand_GetSettings(parserBand[3], &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = remux;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand[3], &parserBandSettings);

    /* Open the audio and video pid channels */
    pidChannelRemuxed[2] = NEXUS_PidChannel_Open(parserBand[3], VIDEO_PID_2, NULL);
    pidChannelRemuxed[3] = NEXUS_PidChannel_Open(parserBand[3], AUDIO_PID_2, NULL);

    /* Open the StcChannel 0 to do lipsync with the PCR for program 0 */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e1;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel[1];
    stcChannel[1] = NEXUS_StcChannel_Open(1, &stcSettings);
   
    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs.  */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = pidChannelRemuxed[2];
    videoProgram.stcChannel = stcChannel[1];
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = pidChannelRemuxed[3];
    audioProgram.stcChannel = stcChannel[1];

    /* Bring up audio decoders and outputs */
    pcmDecoder[1] = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[1]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder[1], NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display[1] = NEXUS_Display_Open(1, &displaySettings);
    window[1] = NEXUS_VideoWindow_Open(display[1], 0);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display[1], NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif

    /* bring up decoder and connect to display */
    vdecode[1] = NEXUS_VideoDecoder_Open(1, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window[1], NEXUS_VideoDecoder_GetConnector(vdecode[1]));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode[1], &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder[1], &audioProgram);
    
    printf("Press ENTER to stop decode\n");
    getchar();

    NEXUS_VideoDecoder_Stop(vdecode[0]);
    NEXUS_AudioDecoder_Stop(pcmDecoder[0]);
    NEXUS_VideoDecoder_Stop(vdecode[1]);
    NEXUS_AudioDecoder_Stop(pcmDecoder[1]);

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode[0]));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode[1]));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder[1], NEXUS_AudioDecoderConnectorType_eStereo));

#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
#endif
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[1]));
#endif

    NEXUS_VideoWindow_Close(window[0]);
    NEXUS_VideoWindow_Close(window[1]);
    NEXUS_Display_Close(display[0]);
    NEXUS_Display_Close(display[1]);

    NEXUS_AudioDecoder_Close(pcmDecoder[0]);
    NEXUS_VideoDecoder_Close(vdecode[0]);
    NEXUS_AudioDecoder_Close(pcmDecoder[1]);
    NEXUS_VideoDecoder_Close(vdecode[1]);

    NEXUS_PidChannel_Close(pidChannelRemuxed[0]);
    NEXUS_PidChannel_Close(pidChannelRemuxed[1]);    
    NEXUS_PidChannel_Close(pidChannelRemuxed[2]);
    NEXUS_PidChannel_Close(pidChannelRemuxed[3]);    

    NEXUS_StcChannel_Close(stcChannel[0]);
    NEXUS_StcChannel_Close(stcChannel[1]);
    NEXUS_PidChannel_Close(pcrPidChannel[0]);
    NEXUS_PidChannel_Close(pcrPidChannel[1]);

    NEXUS_Remux_Stop(remux);
      
    NEXUS_Remux_RemovePidChannel(remux, pidChannel[0]);
    NEXUS_Remux_RemovePidChannel(remux, pidChannel[1]);
    NEXUS_Remux_RemovePidChannel(remux, pidChannel[2]);
    NEXUS_Remux_RemovePidChannel(remux, pidChannel[3]);
    NEXUS_Remux_Close(remux);

    NEXUS_PidChannel_Close(pidChannel[0]);
    NEXUS_PidChannel_Close(pidChannel[1]);    
    NEXUS_PidChannel_Close(pidChannel[2]);
    NEXUS_PidChannel_Close(pidChannel[3]);    

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
