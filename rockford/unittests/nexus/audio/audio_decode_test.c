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
 *----------------------------------------------------------------------------
 * workfile : sqa_audio_decode_test.c 
 * author : raghavendra chinnanaik 
 * Module Description: 
 * This is a nexus based test app, can be used to run audio tests in live mode(streamer input) 
 * Following are the capabilities : 
 * Decode of most audio codecs over DAC, SPDIF and HDMI 
 * Dynamic switching of SPDIF and HDMI modes(compressed, PCM, PCM5.1).
 * Transcode modes for SPDIF and HDMI(AC3, DTS).
 * Post Processings like DolbyVolume, SRS-TruVolume. 
 * Audio control tests like mute-unmute
 * PCM capture of decoder output and storing into wave files.
 *-----------------------------------------------------------------------------
*
* $brcm_Log: $
* 
******************************************************************************/
/* Nexus Audio test app: live-decode using streamer-input */
#include "nexus_platform.h"
#if NEXUS_NUM_SPDIF_OUTPUTS && NEXUS_NUM_AUDIO_DACS
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
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_ac3_encode.h"
#include "nexus_dts_encode.h"
#include "nexus_dolby_volume.h"
#include "nexus_tru_volume.h" 

#include "nexus_audio_capture.h"
#include "nexus_core_utils.h"



/* global variable declarations */

NEXUS_PlatformSettings platformSettings;
NEXUS_PlatformConfiguration platformConfig;
NEXUS_InputBand inputBand;
NEXUS_ParserBand parserBand;
NEXUS_ParserBandSettings parserBandSettings;
NEXUS_StcChannelHandle stcChannel;
NEXUS_StcChannelSettings stcSettings;
NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pcrPidChannel;
NEXUS_DisplayHandle display;
NEXUS_DisplaySettings displaySettings;
NEXUS_VideoWindowHandle window;
NEXUS_VideoDecoderHandle vdecode;
NEXUS_VideoDecoderStartSettings videoProgram;
NEXUS_AudioDecoderHandle audioDecoder = NULL, compressedDecoder = NULL;
NEXUS_AudioDecoderStartSettings audioProgram;
NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
NEXUS_DtsEncodeHandle dtsEncode;
NEXUS_Ac3EncodeHandle ac3Encode;
NEXUS_DolbyVolume258Handle dv258;
NEXUS_TruVolumeHandle truVolume = NULL;
NEXUS_AudioDecoderCodecSettings nCodecSettings; 
NEXUS_AudioOutputSettings OP_Settings; 
static int AC3_Encode_Status = 0, DTS_Encode_Status = 0; 
static int TruVolume_Status = 0, DV258_Status = 0;
int SPDIF_status = 0, HDMI_status = 0, DDP_TO_DD_CONVERT = 0;
char *audioType;
bool Audio_Mute_Flag = 0;


/* Fucntion to start audio decoder */
static void start_audio_decoder(NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder, const NEXUS_AudioDecoderStartSettings *audioProgram)
{
	if(audioDecoder){
		NEXUS_AudioDecoder_Start(audioDecoder, audioProgram);
		}
	if((compressedDecoder && HDMI_status == 1) || (compressedDecoder && SPDIF_status == 1 && DDP_TO_DD_CONVERT != 1)){  
	/* if(compressedDecoder){ */
			NEXUS_AudioDecoder_Start(compressedDecoder, audioProgram);
		}
	return;
}

/* Fucntion to stop audio decoder */
static void stop_audio_decoder(NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder)
{
	if(audioDecoder)
		NEXUS_AudioDecoder_Stop(audioDecoder);
	if((compressedDecoder != NULL && SPDIF_status == 1) || (compressedDecoder != NULL && HDMI_status == 1)){
		NEXUS_AudioDecoder_Stop(compressedDecoder);
		}

	return;
}

/* Fucntion to remove all inputs to various destination ports */
static void AudioOutput_RemoveInput(int DAC_Remove, int SPDIF_Remove, int HDMI_Remove){

	if(DAC_Remove == 1){
		/* remove inputs to DAC output */
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
		}

	if(SPDIF_Remove == 1){
		/* remove inputs to SPDIF output */
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
		}

	if(HDMI_Remove == 1){
		/* remove all inputs to HDMI output */
		NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
		}

	return ;

}


/* Fucntion to remove all inputs to various encoders */
static void encoder_removeinput(void){

	if(dtsEncode){
		/* remove all input to DTS Encoder */
		NEXUS_DtsEncode_RemoveAllInputs(dtsEncode);
		}
	if(ac3Encode){
		/* remove all inputs to AC3 Encoder */
		NEXUS_Ac3Encode_RemoveAllInputs(ac3Encode);
		}
	return;
}

/* Fucntion to handle processing of all inputs to various destination ports */
static void Audio_OutputPort_Input_Processing(int DAC_Input_Process, int SPDIF_Input_Process, int HDMI_Input_Process){

	if(DAC_Input_Process == 1){
		/* send PCM stereo input to DAC output */
		NEXUS_AudioOutput_AddInput(
			NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
			NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));	
		}

	if(SPDIF_status == 2 || HDMI_status == 2){
		NEXUS_Ac3Encode_AddInput(ac3Encode, 
			NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		AC3_Encode_Status = 1;
		}
	if(SPDIF_status == 3 || HDMI_status == 3){
		NEXUS_DtsEncode_AddInput(dtsEncode, 
			NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
		DTS_Encode_Status = 1;
		}
	

	if(SPDIF_Input_Process == 1){

		if(SPDIF_status == 1){
			if(audioProgram.codec == NEXUS_AudioCodec_eAc3Plus){
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
				}
			else{
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
				}

			}
		else if(SPDIF_status == 2){
			NEXUS_AudioOutput_AddInput(
				NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
				NEXUS_Ac3Encode_GetConnector(ac3Encode));
			}
		else if(SPDIF_status == 3){
			NEXUS_AudioOutput_AddInput(
				NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
				NEXUS_DtsEncode_GetConnector(dtsEncode));
			}
		else{
			NEXUS_AudioOutput_AddInput(
				NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
			}
		
		}
	
	if(HDMI_Input_Process == 1){
		
		if(HDMI_status == 1){
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
				NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
			}
		else if(HDMI_status == 2){
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
				NEXUS_Ac3Encode_GetConnector(ac3Encode));
			}
		else if(HDMI_status == 3){
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
				NEXUS_DtsEncode_GetConnector(dtsEncode));
			}
		else if(HDMI_status == 4){
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
		else{
			NEXUS_AudioOutput_AddInput(
				NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
			}
		
		}
	
	return ;

}


/* Fucntion to handle volume post-processing of all inputs to various destination ports */
static void Volume_PostProcessing(int DAC_PostProcess, int SPDIF_PostProcess, int HDMI_PostProcess, bool VolumePostProcessing_Handle){
	
	if(TruVolume_Status == 1){
		if(VolumePostProcessing_Handle == false){
			truVolume = NEXUS_TruVolume_Open(NULL);
			
			/* send decoder output as input to TruVolume Stage */
			NEXUS_TruVolume_AddInput(truVolume, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
			}

		if(DAC_PostProcess == 1){
			/* send processed(truvolume) stereo output as input to DAC output */
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
				NEXUS_TruVolume_GetConnector(truVolume));	
			}

		if(SPDIF_status == 2 || HDMI_status == 2){
			NEXUS_Ac3Encode_AddInput(ac3Encode, 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			} 

		if(SPDIF_status == 3 || HDMI_status == 3){
			NEXUS_DtsEncode_AddInput(dtsEncode, 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
		


		if(SPDIF_PostProcess == 1){
			if(SPDIF_status == 1){
				if(audioProgram.codec == NEXUS_AudioCodec_eAc3Plus){
					NEXUS_AudioOutput_AddInput(
						NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
						NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
					}
				else{
					NEXUS_AudioOutput_AddInput(
						NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
						NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
					}
				}
			else if(SPDIF_status == 2){
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_Ac3Encode_GetConnector(ac3Encode));
				}
			else if(SPDIF_status == 3){
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
					NEXUS_DtsEncode_GetConnector(dtsEncode));
				}
			else{
				/* send processed(truvolume) stereo output as input to SPDIF output */
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_TruVolume_GetConnector(truVolume));
				}
			}

		if(HDMI_PostProcess == 1){
		
			if(HDMI_status == 1){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
				}
			else if(HDMI_status == 2){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_Ac3Encode_GetConnector(ac3Encode));
				}
			else if(HDMI_status == 3){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_DtsEncode_GetConnector(dtsEncode));
				}
			else if(HDMI_status == 4){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
				}
			else{
				/* send processed(truvolume) stereo output as input to HDMI output */
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_TruVolume_GetConnector(truVolume));
				}	
			}

		TruVolume_Status = 1;

		}

	if(DV258_Status == 1){

		if(VolumePostProcessing_Handle == 0){
			/* create and open dv258 handle */
			dv258 = NEXUS_DolbyVolume258_Open(NULL);
			
			/* add decoder output as input to DolbyVolume */
			NEXUS_DolbyVolume258_AddInput(dv258, 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
			}

		if(DAC_PostProcess == 1){
			/* add output from Dolby Volume to DAC output */
			NEXUS_AudioOutput_AddInput(
				NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), 
				NEXUS_DolbyVolume258_GetConnector(dv258));
			}


		if(SPDIF_status == 2 || HDMI_status == 2){
			NEXUS_Ac3Encode_AddInput(ac3Encode, 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			} 

		if(SPDIF_status == 3 || HDMI_status == 3){
			NEXUS_DtsEncode_AddInput(dtsEncode, 
				NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
			}
		

		if(SPDIF_PostProcess == 1){
			if(SPDIF_status == 1){
				if(audioProgram.codec == NEXUS_AudioCodec_eAc3Plus){
					NEXUS_AudioOutput_AddInput(
						NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
						NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
					}
				else{
					NEXUS_AudioOutput_AddInput(
						NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
						NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
					}
				}
			else if(SPDIF_status == 2){
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_Ac3Encode_GetConnector(ac3Encode));
				}
			else if(SPDIF_status == 3){
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
					NEXUS_DtsEncode_GetConnector(dtsEncode));
				}
			else{
				/* send processed(truvolume) stereo output as input to SPDIF output */
				NEXUS_AudioOutput_AddInput(
					NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), 
					NEXUS_DolbyVolume258_GetConnector(dv258));
				}
			}

		if(HDMI_PostProcess == 1){
			if(HDMI_status == 1){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
				}
			else if(HDMI_status == 2){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_Ac3Encode_GetConnector(ac3Encode));
				}
			else if(HDMI_status == 3){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_DtsEncode_GetConnector(dtsEncode));
				}
			else if(HDMI_status == 4){
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
				}
			else{
				/* send processed(truvolume) stereo output as input to HDMI output */
				NEXUS_AudioOutput_AddInput(
					NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), 
					NEXUS_DolbyVolume258_GetConnector(dv258));
				}
			
			}	

		DV258_Status = 1;		

		}

		return ;

}


/* Function to evaluate DTS-Transcoding for various codecs */
static int Allow_DTS_Encode(void){

	if(audioProgram.codec != NEXUS_AudioCodec_eAac && audioProgram.codec != NEXUS_AudioCodec_eAacLoas && audioProgram.codec != NEXUS_AudioCodec_eAacPlusAdts && audioProgram.codec != NEXUS_AudioCodec_eAacPlus && audioProgram.codec != NEXUS_AudioCodec_eDts){
		printf("## DTS encode not supported for algo id %d \n", audioProgram.codec);
		return 1;
		} 
	else{
		return 0;
		}

} 

/* Function to evaluate AC3-Transcoding for various codecs */
static int Allow_AC3_Encode(void){
	
	if(audioProgram.codec != NEXUS_AudioCodec_eAac && audioProgram.codec != NEXUS_AudioCodec_eAacLoas && audioProgram.codec != NEXUS_AudioCodec_eAacPlusAdts && audioProgram.codec != NEXUS_AudioCodec_eAacPlus){
		printf("## AC3 encode not supported for algo id %d \n", audioProgram.codec);
		return 1;
		} 
	else{
		return 0;
		}

}

/* Function to handle Volume post-processings Bypass of all inputs to various destination ports */		
static void Volume_PostProcessing_Bypass(void){

		/* stop audio decoder */
	stop_audio_decoder(audioDecoder, compressedDecoder);

	if(dtsEncode || ac3Encode){
		encoder_removeinput();
		}

	if(DV258_Status == 1){
		/* remove all inputs to dv258 */
		NEXUS_DolbyVolume258_RemoveAllInputs(dv258);
		DV258_Status = 0;
		}
	if(TruVolume_Status == 1){
		/* remove all inputs to truvolume */
		NEXUS_TruVolume_RemoveAllInputs(truVolume);
		TruVolume_Status = 0;
		}

	/* Remove all inputs all ouputs */
	AudioOutput_RemoveInput(1, 1, 1);

	/* add appropriate audio inputs to all output ports */
	Audio_OutputPort_Input_Processing(1, 1, 1);

	/* start audio decoder */
	start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

	return ;
}

/* Function to evaluate status of destination ports, volume post-processings, audio-control etc */
static void Mode_Status(int SPDIF_ST, int HDMI_ST, int DV_ST, int TV_ST, int Mute_ST){

	if(SPDIF_ST == 1 && HDMI_ST == 1 && DV_ST == 1 && TV_ST == 1 && Mute_ST == 1)
	printf("=== CURRENT STATUS ==== \n");

	if(SPDIF_ST == 1){
		printf("#### SPDIF is configured to ");
		if(SPDIF_status == 0)
			printf("PCM ");
		if(SPDIF_status == 1)
			printf("COMPRESSED ");
		if(SPDIF_status == 2)
			printf("AC3 Transcode ");
		if(SPDIF_status == 3)
			printf("DTS Transcode ");
		printf("### \n");
		}

	if(HDMI_ST == 1){
		printf("#### HDMI is configured to ");
		if(HDMI_status == 0)
			printf("PCM ");
		if(HDMI_status == 1)
			printf("COMPRESSED ");
		if(HDMI_status == 2)
			printf("AC3 Transcode ");
		if(HDMI_status == 3)
			printf("DTS Transcode ");
		if(HDMI_status == 4)
			printf("PCM5.1 ");
		printf("### \n");
		}

	if(TV_ST == 1){
		printf("#### SRS TruVolume is ");
		if(TruVolume_Status == 1)
			printf("ENABLED ");
		else
			printf("DISABLED ");
		printf("### \n");
		}

	if(DV_ST == 1){
		printf("#### Dolby Volume is ");
		if(DV258_Status == 1)
			printf("ENABLED ");
		else
			printf("DISABLED ");
		printf("### \n");
		}

	if(Mute_ST == 1){
		printf("#### Audio is ");
		if(Audio_Mute_Flag == 0)
			printf("Un-Muted ###\n");
		else
			printf("Muted ###\n");
		}

	printf("==================== \n");
	
	return ;

}

static void capture_callback(void *pParam, int param)
{
    NEXUS_AudioCaptureHandle capture = pParam;
    FILE *pFile = (FILE *)param;
    NEXUS_Error errCode;

    for ( ;; )
    {
        void *pBuffer;
        size_t bufferSize;

        /* Check available buffer space */
        errCode = NEXUS_AudioCapture_GetBuffer(capture, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            fprintf(stderr, "Error getting capture buffer\n");
            NEXUS_AudioCapture_Stop(capture);
            return;
        }

        if ( bufferSize > 0 )
        {
            /* Write samples to disk */
            if ( 1 != fwrite(pBuffer, bufferSize, 1, pFile) )
            {
                fprintf(stderr, "Error writing to disk\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }

            /*fprintf(stderr, "Data callback - Wrote %d bytes\n", (int)bufferSize);*/
            errCode = NEXUS_AudioCapture_ReadComplete(capture, bufferSize);
            if ( errCode )
            {
                fprintf(stderr, "Error committing capture buffer\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }
        }
        else
        {
            break;
        }
    }
}

/* Function for audio capture */
static void audiodecoder_output_capture(void){



	NEXUS_AudioCaptureHandle capture; 
	NEXUS_AudioCaptureStartSettings captureSettings;
		
	char *CapFileName, CapFile[50]; 
	FILE *pFile; 
	bool wavFile=false; 
	int numSeconds=30;


	CapFileName = CapFile; 
	

	printf("## Enter capture file name(Ex - abc,wav) : ");
	/* fgets(CapFile, 256, stdin); */
	scanf("%s", CapFile);

	printf("\n## Enter Capture Duration in seconds : ");
	scanf("%d", &numSeconds);
	
	if ( strlen(CapFileName) > 4 )
	{
		if ( strstr(CapFileName, ".wav") == CapFileName + (strlen(CapFileName) - 4) )
		{
			/* Filename ends in .wav, write as .wav file */
			wavFile = true;
		}
	}

	
	pFile = fopen(CapFileName, "wb+");
	
	
	if ( NULL == pFile )
	{
		fprintf(stderr, "Unable to open file '%s' for writing\n", CapFileName);
		/* break ; */
	}
	
	if ( NULL == audioDecoder)
	{
		fprintf(stderr, "Unable to open decoder 0\n");
		return ;
	}
	
	
	capture = NEXUS_AudioCapture_Open(0, NULL);
	if ( NULL == capture )
	{
		fprintf(stderr, "Unable to open capture channel\n");
		return ;
	}
	
	/* Setup WAV file if desired (*always little-endian) */
	if ( wavFile )
	{
		fwrite("RIFF", 4, 1, pFile);	/* Byte 0..3 RIFF */
		fputc(0, pFile);				/* Byte 4..7 file size - 4*/
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);
		fwrite("WAVE", 4, 1, pFile);	/* Byte 8..11 WAVE */
		fwrite("fmt ", 4, 1, pFile);	/* Byte 12..15 fmt */
		fputc(16, pFile);				/* Byte 16..19 format chunk length (16 bytes) */
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(1, pFile);				/* Byte 20..21 compression code (1=PCM) */
		fputc(0, pFile);
		fputc(2, pFile);				/* Byte 22..23 Number of channels (2) */
		fputc(0, pFile);
		fputc(0, pFile);				/* Byte 24..27 Sample Rate (actual value later from decoder) */
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);				/* Byte 28..31 Average Bytes/Second (actual value later from decder) */
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(2, pFile);				/* Byte 32..33 Block Align (4 -- 2 bytes/channel * 2 channels) */
		fputc(0, pFile);
		fputc(16, pFile);				/* Byte 34..35 Bits Per Sample (16) */
		fputc(0, pFile);
		fwrite("data", 4, 1, pFile);	/* Byte 36..39 data */
		fputc(0, pFile);				/* Byte 40..43 data size - 4*/
		fputc(0, pFile);
		fputc(0, pFile);
		fputc(0, pFile);
	}
	
	
	NEXUS_AudioOutput_AddInput(
		NEXUS_AudioCapture_GetConnector(capture), NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
	
	/* Start the capture -- no data will be received until the decoder starts */
	NEXUS_AudioCapture_GetDefaultStartSettings(&captureSettings);
	captureSettings.dataCallback.callback = capture_callback;
	captureSettings.dataCallback.context = capture;
	captureSettings.dataCallback.param = (int)pFile;
	NEXUS_AudioCapture_Start(capture, &captureSettings);
	
	
	printf("\n@@@ DEBUG : Capture Filename is %s \n", CapFileName); 
	
	/* start audio decoder */
	start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram); 
	NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
	
	fprintf(stderr, "\nStarted capture for %d seconds.\n", numSeconds);
	BKNI_Sleep(numSeconds * 1000);
	
	fprintf(stderr, "Stopping capture\n");
	NEXUS_StopCallbacks(capture);
	NEXUS_AudioCapture_Stop(capture);
	
	/* After StopCallbacks, we are guaranteed no more callbacks will arrive.  If we're writing raw data, we're done.
	   If we're writing a .wav file, seek back to the beginning and finish up the header */
	if ( wavFile )
	{
		unsigned long fileLength;
		NEXUS_AudioDecoderStatus decoderStatus;
	
		fileLength = ftell(pFile);
	
		printf("%lu bytes written to file\n", fileLength);
		if ( fileLength == 44 )
		{
			printf("Warning, empty file detected.  Double-check data source\n");
		}
	
		NEXUS_AudioDecoder_GetStatus(audioDecoder, &decoderStatus);
	
		fseek(pFile, 4, SEEK_SET);	/* Need to write file size - 4 to this offset */
		fileLength -= 4;
		fputc(fileLength & 0xff, pFile);
		fputc((fileLength >> 8) & 0xff, pFile);
		fputc((fileLength >> 16) & 0xff, pFile);
		fputc((fileLength >> 24) & 0xff, pFile);
		fseek(pFile, 24, SEEK_SET); /* Need to write sample rate here */
		fputc(decoderStatus.sampleRate & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>8) & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>16) & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>24) & 0xff, pFile);
		/* Need to write sampleRate * 4 here */
		decoderStatus.sampleRate *= 4;
		fputc(decoderStatus.sampleRate & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>8) & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>16) & 0xff, pFile);
		fputc((decoderStatus.sampleRate>>24) & 0xff, pFile);
		fseek(pFile, 40, SEEK_SET);  /* Need to write data size (file size - 44) to this offset */
		fileLength -= 40;
		fputc(fileLength & 0xff, pFile);
		fputc((fileLength >> 8) & 0xff, pFile);
		fputc((fileLength >> 16) & 0xff, pFile);
		fputc((fileLength >> 24) & 0xff, pFile);
	}

	/* stop audio decoder */
	stop_audio_decoder(audioDecoder, compressedDecoder); 
	NEXUS_VideoDecoder_Stop(vdecode);
	
	NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(capture));
	/* NEXUS_AudioCapture_Stop(capture); */

	NEXUS_AudioCapture_Close(capture);
	
	fflush(pFile);
	fclose(pFile);	


return ;
}



int main(int argc, char **argv)
{		

	bool i,j, cap = false;
	/* char pcr[26] = "0x", video[26] = "0x", audio[26] = "0x", audiotype[5] = "0x", S1[26], S2[26], S3[26], S4[5]; */
	char pcr[10], video[10], audio[10] , audiotype[10], S1[10], S2[10], S3[10], S4[10];
	char *pcr_id, *video_id, *audio_id;
	long pcr_pid, video_pid, audio_pid; 

    if ( argc > 1 ){
		if((!strcmp(argv[1],"-help")) || (!strcmp(argv[1],"-HELP")) || (!strcmp(argv[1],"--help")) || (!strcmp(argv[1],"--HELP")) || (!strcmp(argv[1],"-h")) || (!strcmp(argv[1],"-H"))){
				printf("\n ### USAGE DETAILS ###");
				printf("\n - have the streamer connected to the reference setup");
				printf("\n - run any test stream from the streamer");
				printf("\n - run the test app using the following command/syntax :"); 
				printf("\n   nexus <app-name> [Ex- nexus sqa_audio_decode_test]");
				printf("\n   audio PCM capture can also be enabled, by running the app with -c option in the commandline");
				printf("\n - after running, user will be prompted to enter few input parameters");
				printf("\n - decode starts after the input parameters are entered");	
				printf("\n - using the '?' user can get a menu for usage-modes/options supported");
				printf("\n ###################### \n");
				return 0;
			}
		else if(!strcmp(argv[1],"-c") || !strcmp(argv[1],"-C")){
			printf("\n - audio capture will be enabled \n");
			cap = true;
			}
		else{
			printf("\n INVALID COMMAND LINE OPTION USED \n For USAGE DETAILS please use : -h or -help \n To enable capture use : -c or -C \n");
			return 0;
			}
		
    	}
		

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings); 


	printf("==========================================\n");
	printf("## Enter PCR PID (Example - 0x11) :0x");
	/* fgets(pcr, 26, stdin);  */
	scanf("%s", S1);
	strncat(strncat(pcr, "0x", 2), S1, strlen(S1));
	printf("==========================================\n");
	printf("## Enter Video PID (Example - 0x11) :0x");
	/* fgets(video, 26, stdin); */
	scanf("%s", S2);
	strncat(strncat(video, "0x", 2), S2, strlen(S2));
	printf("==========================================\n");
	printf("## Enter Audio PID (Example - 0x14) :0x");
	/* fgets(audio, 26, stdin); */
	scanf("%s", S3);
	strncat(strncat(audio, "0x", 2), S3, strlen(S3));
	printf("==========================================\n");

	pcr_pid = strtol(pcr, &pcr_id, 0);
	video_pid = strtol(video, &video_id, 0);
	audio_pid = strtol(audio, &audio_id, 0);


    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, video_pid, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, audio_pid, NULL);
	pcrPidChannel = NEXUS_PidChannel_Open(parserBand, pcr_pid, NULL);
	
    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    /* stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; PCR happens to be on video pid */
	stcSettings.modeSettings.pcr.pidChannel = pcrPidChannel;
	
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    /* audioProgram.codec = NEXUS_AudioCodec_eAc3; */


	/* #### audio codec mapping #### */
	for(j=false; j!=true;){
		
        char *abuf;
		abuf = audiotype;


		printf("--- Audio Types Supported ---\n");
		printf(" AC3 - 0x81\n DDP - 0x6\n AAC - 0xf\n AAC+adts - 0x12\n LOAS - 0x11\n MPEG Layer1/2 - 0x3\n MPEG Layer3 - 0x1\n DTS - 0x82\n DRA - 0xda\n");
		printf("-----------------------------\n");
		printf("## Enter Audio Type : 0x");
		scanf("%s", S4);
		strncat(strncat(audiotype, "0x", 2), S4, strlen(S4));
		printf("==========================================\n"); 

			if(!strcmp(S4,"81")){
				audioProgram.codec = NEXUS_AudioCodec_eAc3;
				j = true;
				}
			else if(!strcmp(S4,"6")){
				audioProgram.codec = NEXUS_AudioCodec_eAc3Plus;
				j = true;
				}
			else if(!strcmp(S4,"f") || !strcmp(S4,"F")){
				audioProgram.codec = NEXUS_AudioCodec_eAac;
				j = true;
				}
			else if(!strcmp(S4,"11")){
				audioProgram.codec = NEXUS_AudioCodec_eAacPlus;
				j = true;
				}
			else if(!strcmp(S4,"12")){
				audioProgram.codec = NEXUS_AudioCodec_eAacPlusAdts;
				j = true;
				}
			else if(!strcmp(S4,"3")){
				audioProgram.codec = NEXUS_AudioCodec_eMpeg;
				j = true;
				}
			else if(!strcmp(S4,"1")){
				audioProgram.codec = NEXUS_AudioCodec_eMp3;
				j = true;
				}
			else if(!strcmp(S4,"82")){
				audioProgram.codec = NEXUS_AudioCodec_eDts;
				j = true;
				}
			else if(!strcmp(S4,"da") || !strcmp(S4,"da")){
				audioProgram.codec = NEXUS_AudioCodec_eDra;
				j = true;
				}

            else if (!*abuf) {
            }
			else{
				printf("#### Audio Codec 0x%s not supported  ####\n", S4);
				}

		}

	
	printf("-------------------------\n");
	printf(" PCR PID - %s\n Video PID - %s\n Audio PID - %s\n Audio Type - %s\n",pcr,video,audio,audiotype);
	printf("-------------------------\n");
	Mode_Status(1,1,1,1,1);

		
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel; 


    /* Bring up audio decoders and outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
	audioDecoderOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;

    audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);


	
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	    NEXUS_AudioOutput_AddInput(
			NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
	    	NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
			SPDIF_status = 1;
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

    /* Bring up display and outputs */
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
    fprintf(stderr, "Panel output ready\n");
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if(platformConfig.outputs.component[0]){
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
#endif 

    if(platformConfig.outputs.component[0]){
        NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    }

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS    
    if (platformConfig.outputs.svideo[0]) {
        NEXUS_Display_AddOutput(display, NEXUS_SvideoOutput_GetConnector(platformConfig.outputs.svideo[0]));
    }
#endif    
#endif
    window = NEXUS_VideoWindow_Open(display, 0);

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));


	if(cap == true)
		{
		/* @@@@ Audio Capture Code */
		audiodecoder_output_capture(); 
		}

	/* Re-Start video and audio decoder after Capture */
	NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
	NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram); 
		


    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }


	for(i=false; i!=true;){
        char buffer[256];
        char *buf;

		printf("\n== Enter ? for Main Menu == \n");
		printf("decode>"); fflush(stdout);
        fgets(buffer, 256, stdin);
		
        if(feof(stdin)) {
            break;
        }

        buffer[strlen(buffer)-1] = '\0'; 
        buf = strtok(buffer, ";");
		
        if (!buf) continue;

		do{
			if (!strcmp(buf, "?")) {
				printf(
					" Audio Options \n"
					" ============================================================== \n"
					);
				printf(
					" SPDIF/HDMI modes : \n"
					" ------------------------------ \n"
					"  1 : spdif-comp  - switch SPDIF to compressed mode\n" 
					"  2 : spdif-pcm  - switch SPDIF to pcm mode\n" 
					"  3 : spdif-ac3encode	- switch SPDIF to AC3 Encode\n" 
					"  4 : spdif-dtsencode	- switch SPDIF to DTS Encode\n" 
					"  5 : hdmi-comp  - switch HDMI to compressed mode\n" 
					"  6 : hdmi-pcm  - switch HDMI to pcm mode\n" 
					"  7 : hdmi-pcm5.1	- switch HDMI to pcm5.1 mode\n"
					"  8 : hdmi-ac3encode  - switch HDMI to AC3 Encode\n" 
					"  9 : hdmi-dtsencode  - switch HDMI to DTS Encode\n"
					"  10 : Status\n"	
					);
				printf(
					" ------------------------------ \n"
					"  Post Processings : \n"	
					" ------------------------------ \n"	
					"  D : DolbyVolume(DV258) Toggle \n"
					"  S : SRSTruVolume Toggle \n"
					"  M : Mute/Unmute \n"
					" ------------------------------ \n"	
					"  q : Quit\n"
					" ------------------------------ \n"	
					);

				break ;
			}

			
            else if (!strcmp(buf, "q")) {
                i = true;
				break ; 
                
            }


			/* Start of SPDIF Compressed option */
            else if (!strcmp(buf, "1")) {

				/* Verify whether SPDIF is already configured to compressed mode */
				if(SPDIF_status == 1){
					printf("#### SPDIF already configured to compressed mode ####\n");
				} 

				else{

					SPDIF_status = 1;

					if(audioProgram.codec == NEXUS_AudioCodec_eAc3Plus){
						DDP_TO_DD_CONVERT = 1;
						}

					/* diabling passthru mode for AAC, refer to JON's comments in SW7425-880 */
					if(audioProgram.codec == NEXUS_AudioCodec_eAac || audioProgram.codec == NEXUS_AudioCodec_eAacLoas || audioProgram.codec == NEXUS_AudioCodec_eAacPlusAdts || audioProgram.codec == NEXUS_AudioCodec_eAacPlus){
						SPDIF_status = 0;
						printf("--- NOTE : Forcing SPDIF to PCM mode --- \n");
						}
					
					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove inpus to all encoders */
					encoder_removeinput();


						if( TruVolume_Status == 1 || DV258_Status ==1 ){

							/* remove all audio inputs to all output ports before volume post processing */
							AudioOutput_RemoveInput(1, 1, 1);

							/* do volume post processing on all output ports */
							Volume_PostProcessing(1, 1, 1, true);
							
							}
						else{

							/* remove inputs to SPDIF output */
							AudioOutput_RemoveInput(0, 1, 0);

							/* add compressed input to SPDIF output */
							Audio_OutputPort_Input_Processing(0, 1, 0);
							}

						/* start audio decoder */
						start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

						/* printf("### SPDIF configured to Compressed ### \n"); */
						Mode_Status(1,0,0,0,0);
					}

				break ;

            }
			/* end of SPDIF Compressed option */

			/* Start of SPDIF PCM option */
            else if (!strcmp(buf, "2")) {

				/* Verify whether SPDIF is already configured to PCM mode */
				if(SPDIF_status == 0){
					printf("#### SPDIF already configured to PCM mode ####\n");
				}

				else{
					SPDIF_status = 0;
					
					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					if(dtsEncode || ac3Encode){
						encoder_removeinput();
						}

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
						
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
						
						/* do volume post processing on all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
						/* remove inputs to SPDIF output */
						AudioOutput_RemoveInput(0, 1, 0);
						
						/* add PCM input to SPDIF output */
						Audio_OutputPort_Input_Processing(0, 1, 0);
						}


					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### SPDIF configured to PCM ### \n"); */
					Mode_Status(1,0,0,0,0);

					}

				break ;

            	}

			/* #### start of SPDIF "AC3 encode" option */
            else if (!strcmp(buf, "3")) {

				/* verify whether SPDIF is already configured to AC3 encode mode */
				if(SPDIF_status == 2){
						printf("#### SPDIF already configured to AC3 Transcode mode ####\n");
					}

				else if(Allow_AC3_Encode()){
					break ;
					}

				else{

					SPDIF_status = 2;

					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove audio inputs to all encodres */
					encoder_removeinput();

					
					if (ac3Encode == NULL)
						ac3Encode = NEXUS_Ac3Encode_Open(NULL);

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
						
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
						
						/* do volume post processing on all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
						/* remove inputs to SPDIF output */
						AudioOutput_RemoveInput(0, 1, 0);
						
						/* add PCM input to SPDIF output */
						Audio_OutputPort_Input_Processing(0, 1, 0);
						}

					
					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### SPDIF configured to AC3 Transcode ### \n"); */
					Mode_Status(1,0,0,0,0);
					}

				break ;

				}		
				/* #### end of SPDIF "AC3 encode" option */

			/* #### start of SPDIF "DTS encode" option */
            else if (!strcmp(buf, "4")) {

				/* verify whether SPDIF is already configured to DTS encode mode */
				if(SPDIF_status == 3){
						printf("#### SPDIF already configured to DTS Transcode mode ####\n");
					}

				
				else if(Allow_DTS_Encode()){	
					}
				
				else{

					SPDIF_status = 3;
					
					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove audio inputs to all encoders */
					encoder_removeinput();

					if (dtsEncode== NULL)
						dtsEncode= NEXUS_DtsEncode_Open(NULL);

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
						
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
						
						/* do volume post processing on all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
						/* remove inputs to SPDIF output */
						AudioOutput_RemoveInput(0, 1, 0);
						
						/* add PCM input to SPDIF output */
						Audio_OutputPort_Input_Processing(0, 1, 0);
						}
					
					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### SPDIF configured to DTS Transcode ### \n"); */
					Mode_Status(1,0,0,0,0);

					}

				break ;
				
				}		
			
			/* Start of HDMI compressed option */
			else if(!strcmp(buf, "5")){

				/* Verify whether HDMI is already configured to compressed mode */
				if(HDMI_status == 1){
					printf("#### HDMI already configured to compressed mode ####");
				}

				else{

					HDMI_status = 1;

					/* diabling passthru mode for AAC, refer to JON's comments in SW7425-880 */
					if(audioProgram.codec == NEXUS_AudioCodec_eAac || audioProgram.codec == NEXUS_AudioCodec_eAacLoas || audioProgram.codec == NEXUS_AudioCodec_eAacPlusAdts || audioProgram.codec == NEXUS_AudioCodec_eAacPlus){
						HDMI_status = 0;
						printf("### NOTE : Forcing HDMI to PCM mode ### \n");
						}

					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove audio inputs to all encoders */
					encoder_removeinput();

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
					
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
					
						/* do volume post processing for all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
					
						/* remove inputs to HDMI output */
						AudioOutput_RemoveInput(0, 0, 1);
					
						/* add compressed input to HDMI output */
						Audio_OutputPort_Input_Processing(0, 0, 1);
						}

					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### HDMI configured to Compressed ### \n"); */
					Mode_Status(0,1,0,0,0);

				}

				break ;

			}

			/* start of HDMI PCM option */
			else if(!strcmp(buf, "6")){

				/* verify whether HDMI is already configured to PCM option */
				if(HDMI_status == 0){
					printf("#### HDMI output already configured to PCM mode ####\n");
				}

				else{

					HDMI_status = 0;

					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove audio inputs to all encoders */
					encoder_removeinput();

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
					
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
					
						/* do volume post processing for all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
					
						/* remove inputs to HDMI output */
						AudioOutput_RemoveInput(0, 0, 1);
					
						/* add compressed input to HDMI output */
						Audio_OutputPort_Input_Processing(0, 0, 1);
						}


					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### HDMI configured to PCM ### \n"); */
					Mode_Status(0,1,0,0,0);

				}

				break ;
				
			}

			if(!strcmp(buf, "7")){

				/* verify whether HDMI is already configured to PCM5.1 mode */
				if(HDMI_status == 4){
					printf("#### HDMI already configured to PCM5.1 mode ####\n");
				}

				/*
				else if(audioProgram.codec == NEXUS_AudioCodec_eMpeg || audioProgram.codec == NEXUS_AudioCodec_eMp3){
					printf("## PCM5.1 mode not supported for algo id %d ## \n", audioProgram.codec);
					}
				*/
			
				else{

					if(audioProgram.codec == NEXUS_AudioCodec_eMpeg || audioProgram.codec == NEXUS_AudioCodec_eMp3){
						HDMI_status = 0;
						printf("=== NOTE : Forcing HDMI to PCM mode \nMultichannel PCM mode not supported for MPEG1/2/3 audio ### \n");
						}
					else{
						HDMI_status = 4;
						/* printf("### configuring HDMI to PCM5.1 ### \n"); */
						}

					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					if(dtsEncode || ac3Encode){
						encoder_removeinput();
						}

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
					
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
					
						/* do volume post processing for all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
					
						/* remove inputs to HDMI output */
						AudioOutput_RemoveInput(0, 0, 1);


						/* add PCM multichannel input to HDMI output */
						Audio_OutputPort_Input_Processing(0, 0, 1);
						}
							
					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### configuring HDMI to PCM5.1 ### \n"); */
					Mode_Status(0,1,0,0,0);

					}

				break ;

				}			

			/* #### start of HDMI "AC3 encode" option */
            else if (!strcmp(buf, "8")) {

				/* verify whether HDMI is already configured to AC3 encode mode */
				if(HDMI_status == 2){
						printf("#### HDMI already configured to AC3 Transcode mode ####\n");
					}
				
				else if(Allow_AC3_Encode()){
					}

				else{
					
					HDMI_status = 2;

					/* stop audio decoder */
					stop_audio_decoder(audioDecoder, compressedDecoder);

					/* remove audio inputs to all encoders */
					encoder_removeinput();

					if(ac3Encode == NULL)
						ac3Encode = NEXUS_Ac3Encode_Open(NULL);

					if( TruVolume_Status == 1 || DV258_Status ==1 ){
					
						/* remove all audio inputs to all output ports before volume post processing */
						AudioOutput_RemoveInput(1, 1, 1);
					
						/* do volume post processing for all output ports */
						Volume_PostProcessing(1, 1, 1, true);
						
						}
					else{
					
						/* remove inputs to HDMI output */
						AudioOutput_RemoveInput(0, 0, 1);
					
						/* add compressed input to HDMI output */
						Audio_OutputPort_Input_Processing(0, 0, 1);
						}

					/* start audio decoder */
					start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

					/* printf("### HDMI configured to AC3 Transcode ### \n"); */
					Mode_Status(0,1,0,0,0);

					}

				break ;

            	}
				/* #### end of HDMI "AC3 encode" option */

				/* #### start of HDMI "DTS encode" option */
				else if (!strcmp(buf, "9")) {

					
					/* verify whether HDMI is already configured to DTS encode mode */
					if(HDMI_status == 3){
							printf("#### HDMI already configured to DTS Transcode mode ####\n");
						}

					else if(Allow_DTS_Encode()){
						}
					
					else{

						HDMI_status = 3;

						/* stop audio decoder */
						stop_audio_decoder(audioDecoder, compressedDecoder);

						/* remove audio inputs to all encoders */
						encoder_removeinput();

						if(dtsEncode == NULL)
							dtsEncode = NEXUS_DtsEncode_Open(NULL);
						
						if( TruVolume_Status == 1 || DV258_Status ==1 ){
						
							/* remove all audio inputs to all output ports before volume post processing */
							AudioOutput_RemoveInput(1, 1, 1);
						
							/* do volume post processing for all output ports */
							Volume_PostProcessing(1, 1, 1, true);
							
							}
						else{
						
							/* remove inputs to HDMI output */
							AudioOutput_RemoveInput(0, 0, 1);
						
							/* add AC3 encode output as input to HDMI output */
							Audio_OutputPort_Input_Processing(0, 0, 1);
							}


						/* start audio decoder */
						start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

						/* printf("### HDMI configured to DTS Transcode ### \n"); */
						Mode_Status(0,1,0,0,0);

						}

					break ;

					}
					/* #### end of HDMI "DTS encode" option */
							

            if(!strcmp(buf, "10")){
				Mode_Status(1,1,1,1,1);
				break ;
            	}

            else if(!strcmp(buf, "D") || !strcmp(buf, "d")){ 


							if(TruVolume_Status == 1){

								printf("\n#### SRSTruVolume is currently enabled.####\n"
									"## Among Volume PostProcessings(DolbyVolume or SRSTruVolume) \n"
									"only one can be enabled at a time ##\n"
									"## So disabling SRSTruVolume first, to enable DolbyVolume now ##\n\n");
								
								Volume_PostProcessing_Bypass();
								TruVolume_Status = 0;
								
								}
							
								if(DV258_Status == 0){
									
									printf("==#### Turning on DolbyVolume ####==\n");
									
									/* stop audio decoder */
									stop_audio_decoder(audioDecoder, compressedDecoder);
									
									/* remove inputs to all encoders */
									encoder_removeinput();
									
									/* remove all audio inputs to all output ports */
									AudioOutput_RemoveInput(1, 1, 1);
									
									DV258_Status = 1;
									
									/* do dv258 postprocessing for all output ports */
									Volume_PostProcessing(1, 1, 1, false);
									
									/* start audio decoder */
									start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);

									}

								else{

									printf("==#### Turing OFF DolbyVolume ####==\n");
									Volume_PostProcessing_Bypass();

									DV258_Status = 0;

									}
								

							break ;
						
							}



            else if(!strcmp(buf, "S") || !strcmp(buf, "s")){ 


							if(DV258_Status == 1){

								printf("\n#### DolbyVolume is currently enabled.####\n"
									"## Among Volume PostProcessings(DolbyVolume or SRSTruVolume) \n"
									"only one can be enabled at a time ##\n"
									"## So disabling DolbyVolume first, to enable SRSTruVolume now ##\n\n");

								Volume_PostProcessing_Bypass();
								DV258_Status = 0;
								
								}
							
								if(TruVolume_Status == 0){
									
									printf("==#### Turning on SRS TruVolume ####==\n");
									
									/* stop audio decoder */
									stop_audio_decoder(audioDecoder, compressedDecoder);
									
									/* remove inputs to all encoders */
									encoder_removeinput();
									
									/* remove all audio inputs to all output ports */
									AudioOutput_RemoveInput(1, 1, 1);
									
									TruVolume_Status = 1;
									
									/* do truvolume post processing to all output ports */
									Volume_PostProcessing(1, 1, 1, false);
									
									/* start audio decoder */
									start_audio_decoder(audioDecoder, compressedDecoder, &audioProgram);


									}

								else{

									printf("==#### Turing OFF SRS TruVolume ####==\n");
									Volume_PostProcessing_Bypass();

									TruVolume_Status = 0;

									}
								

							break ;
						
							}


			
            else if(!strcmp(buf, "M") || !strcmp(buf, "m")){

				NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &OP_Settings);
				NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &OP_Settings);
				NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &OP_Settings);
				

				if(Audio_Mute_Flag == 0){
					OP_Settings.muted = 1; 
					Audio_Mute_Flag = 1; 
					Mode_Status(0,0,0,0,1);
					}
				else{
					OP_Settings.muted = 0;
					Audio_Mute_Flag = 0; 
					Mode_Status(0,0,0,0,1);
					}

				NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &OP_Settings);
				NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &OP_Settings);
				NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &OP_Settings);				
				
				break;
            }

					
            else if (!*buf) {
                /* allow blank line */
            }
			
            else {
                printf("unknown command: %s\n", buf);
            }

			}
		while((buf = strtok(NULL, ";")));

		}


	    /* example shutdown */
	    NEXUS_AudioDecoder_Stop(audioDecoder);
	    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
	    {
	        NEXUS_AudioDecoder_Stop(compressedDecoder);
	    }

		
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
	    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
		NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
	    NEXUS_AudioDecoder_Close(audioDecoder);
	    NEXUS_AudioDecoder_Close(compressedDecoder);
	    NEXUS_VideoDecoder_Stop(vdecode);
		NEXUS_VideoWindow_RemoveAllInputs(window);
	    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(vdecode));
	    NEXUS_VideoWindow_Close(window);
	    NEXUS_Display_Close(display);
	    NEXUS_VideoDecoder_Close(vdecode); 
	    NEXUS_StcChannel_Close(stcChannel);
	    NEXUS_PidChannel_Close(videoPidChannel);
	    NEXUS_PidChannel_Close(audioPidChannel);
	    NEXUS_Platform_Uninit();

		return 0;


	
    
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return -1;
}
#endif
