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
/* Nexus example app: single live a/v decode from a streamer */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_pid_channel.h"
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
#include "nexus_audio_equalizer.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <assert.h>
#include <stdlib.h>

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

int main(void)
{
	int tmp;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;

	NEXUS_AudioEqualizerHandle	eqForDac= NULL;
	NEXUS_AudioEqualizerStageHandle	eqStgForDac;
    NEXUS_AudioEqualizerSettings eqSettings;
    NEXUS_AudioEqualizerStageSettings eqStgSettings;

    NEXUS_AudioEqualizerStageType eqType;
    NEXUS_Error errCode;
    
    NEXUS_AudioCapabilities audioCaps;
    NEXUS_AudioOutputHandle audioDacHandle = NULL;
    NEXUS_AudioOutputHandle audioSpdifHandle = NULL;
    NEXUS_AudioOutputHandle audioHdmiHandle = NULL;
        
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_GetAudioCapabilities(&audioCaps);
    if ( false == audioCaps.equalizer.supported )
    {
        printf("This chipset doesn't support equalizers\n");
        return 0;
    }

    if (audioCaps.numDecoders == 0)
    {
        printf("This application is not supported on this platform (requires decoder).\n");
        return 0;
    }

    if (audioCaps.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }

    if (audioCaps.numOutputs.spdif > 0)
    {
        audioSpdifHandle = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioCaps.numOutputs.hdmi > 0)
    {
        audioHdmiHandle = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
    }
    #endif

    /* For this example, get data from a streamer input. The input band is platform-specific.
    See nexus/examples/frontend for input from a demodulator. */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, AUDIO_PID, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


	/************ Create equalizer **********/
    NEXUS_AudioEqualizer_GetDefaultSettings(&eqSettings);

    eqForDac = NEXUS_AudioEqualizer_Create(&eqSettings);
    assert(NULL != eqForDac);

    printf("\n\t Enter Type of Equalizer: ");
    printf("\n 1 for ToneControl");
    printf("\n 2 for FiveBand");
    printf("\n 3 for SevenBand");
    printf("\n 4 for Subsonic");
    printf("\n 5 for Subwoofer\n");
    scanf("%d", &tmp);

    switch ( tmp )
    {
    default:
    case 1:
        eqType = NEXUS_AudioEqualizerStageType_eToneControl;
        break;
    case 2:
        eqType = NEXUS_AudioEqualizerStageType_eFiveBand;
        break;
    case 3:
        eqType = NEXUS_AudioEqualizerStageType_eSevenBand;
        break;
    case 4:
        eqType = NEXUS_AudioEqualizerStageType_eSubsonic;
        break;
    case 5:
        eqType = NEXUS_AudioEqualizerStageType_eSubwoofer;
        break;
    }

    NEXUS_AudioEqualizerStage_GetDefaultSettings(eqType,&eqStgSettings);

    eqStgForDac = NEXUS_AudioEqualizerStage_Create(&eqStgSettings);
    assert(NULL != eqStgForDac);
    
    errCode = NEXUS_AudioEqualizer_AddStage(eqForDac, eqStgForDac);
    assert(errCode == NEXUS_SUCCESS);

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
    if (audioDacHandle) {

        errCode = NEXUS_AudioOutput_SetEqualizer(
            audioDacHandle,
            eqForDac);
        assert(errCode == NEXUS_SUCCESS);

        NEXUS_AudioOutput_AddInput(
            audioDacHandle,
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    if (audioSpdifHandle) {
        if ( AUDIO_CODEC == NEXUS_AudioCodec_eAc3 )
        {
            /* Only pass through AC3 */
            NEXUS_AudioOutput_AddInput(
                audioSpdifHandle,
                NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
        }
        else
        {
            NEXUS_AudioOutput_AddInput(
                audioSpdifHandle,
                NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
    }
    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioHdmiHandle) {
        NEXUS_AudioOutput_AddInput(
            audioHdmiHandle,
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    #endif

    /* Bring up display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
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

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( AUDIO_CODEC == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

#if 1
    printf("Press q to quit\n");
	scanf("%d", &tmp);
#else
	{
		char ctmp[256];
		bool running=true;
		while(running)
		{
			if((eqType <= NEXUS_AudioEqualizerStageType_eSevenBand))
			{
				printf("\nDo you want  Set Equalizer Settings\n? [y/n]: ");
	   			fgets(ctmp, 256, stdin);

	   			
	   			if ( ctmp[0] == 'y' || ctmp[0] == 'Y' )
			    {
			    	float tmpFloat;
				
	                switch ( eqType )
	                {
	                case NEXUS_AudioEqualizerStageType_eToneControl:
	                    printf("\nSelect Bass Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.toneControl.bassSettings.gain = (unsigned int)(tmpFloat*100.0);
	                    printf("\nSelect Treble Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.toneControl.trebleSettings.gain = (unsigned int)(tmpFloat*100.0);
	                    break;
	                case NEXUS_AudioEqualizerStageType_eFiveBand:
	                    printf("\nSelect 100Hz Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);

	                    eqStgSettings.modeSettings.fiveBand.gain100Hz = (unsigned int)(tmpFloat*100.0);
	                    printf("\nSelect 300Hz Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.fiveBand.gain300Hz = (unsigned int)(tmpFloat*100.0);
	                    printf("\nSelect 1kHz Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.fiveBand.gain1000Hz = (unsigned int)(tmpFloat*100.0);
	                    printf("\nSelect 3kHz Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.fiveBand.gain3000Hz = (unsigned int)(tmpFloat*100.0);
	                    printf("\nSelect 10kHz Level [-10.0dB - +10.0dB]: ");
	                    scanf("%f", &tmpFloat);
	                    eqStgSettings.modeSettings.fiveBand.gain10000Hz = (unsigned int)(tmpFloat*100.0);
	                    break;
	                case NEXUS_AudioEqualizerStageType_eSevenBand:
	                    {
	                        int tmp2, band_idx;
	                        
	                        printf("\nEnter  setting for 7 bands of the equalizer, each band requires:\n");
	                        printf("  Peak frequency in Hz\n");
	                        printf("  Peak frequency Q gain in 1/100 (eg, for 0.33, enter 33)\n");
	                        printf("  Gain in 1/100dB, range -12dB to +12dB, (eg, for 9.1dB, enter 9.1)\n");
	                        for( band_idx = 0; band_idx < 7; band_idx++)
	                        {
	                            printf("\nEnter band[%d] Peak, Q, Gain values (separated by space): \n", band_idx);
	                            scanf("%d %d %f", &tmp, &tmp2, &tmpFloat);
	                            eqStgSettings.modeSettings.sevenBand.bandSettings[band_idx].peak = tmp;
	                            eqStgSettings.modeSettings.sevenBand.bandSettings[band_idx].q    = tmp2;
								/* 100.0 is used to keep the 0.xx value */
	                            eqStgSettings.modeSettings.sevenBand.bandSettings[band_idx].gain = (unsigned int) (tmpFloat*100.0);
	                        }
	                    }
	                	break;
					default:
	                    break;
	                }
	                NEXUS_AudioEqualizerStage_SetSettings( eqStgForDac,&eqStgSettings);
				}
					
			
			}
			printf("\nEnter 0 to quit");
			scanf("%d", &tmp);
			if(tmp==0)
			{
				running = false;
			}    	
	    }
	}
#endif

    /* example shutdown */
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
    }

    NEXUS_AudioEqualizer_Destroy(eqForDac);
    NEXUS_AudioEqualizerStage_Destroy(eqStgForDac);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_VideoDecoder_Stop(vdecode);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
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
