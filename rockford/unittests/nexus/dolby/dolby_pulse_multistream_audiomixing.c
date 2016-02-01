/***************************************************************************
 *     (c)2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_trick.h"
#include "nexus_video_adj.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_display_vbi.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_surface.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "cmdline_args.h"

#include "nexus_ac3_encode.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "bstd.h"
#include "bkni.h"
#include "fileio_custom.h"

BDBG_MODULE(playback);

/*-------------------------------------------------------------------Command line arguments for Dolby pulse----------------------------------------------------------------------------------------*/
struct dolby_pulse_command_args {

	const char *config_file;
    bool transcode;
	int MixingOutput;
	uint16_t audpid1,audpid2,vidpid;

	NEXUS_AudioDecoderSettings decodeSettings;
	NEXUS_AudioDecoderCodecSettings codecSettings;


   };
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------Audio capture callback----------------------------------------------------------------------------------------------------------*/
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
            errCode = NEXUS_AudioCapture_WriteComplete(capture, bufferSize);
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
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------Parsing the config file and storing the values for the config parameters----------------------------------------------------------------------*/

static void parse_config_file(struct dolby_pulse_command_args *dolby)
{
	char line[400];
	char *value;
    FILE *f=fopen(dolby->config_file,"r");

	while(fgets( line, 400, f )){
	  
	  value = strchr(line, '=');
	  *value = 0;
	  value++;
	  
	  if(!strcmp(line,"OUTPUTMODE"))
		  dolby->decodeSettings.outputMode= atoi(value);	      
	  else if(!strcmp(line,"OUTPUTLFECHANNEL"))
		  dolby->decodeSettings.outputLfeMode=atoi(value);
	  else if(!strcmp(line,"DUALMONO_MODE "))
		  dolby->decodeSettings.dualMonoMode=atoi(value);

	  else if(!strcmp(line,"DOWNMIX_MODE"))
	  {
			  if(atoi(value)==0) {
				  dolby->codecSettings.codecSettings.aacPlus.downmixMode=2; /*NEXUS_AudioDecoderAacDownmixMode_eLtRt*/
			  }
			  else if(atoi(value)==1) {
				  dolby->codecSettings.codecSettings.aacPlus.downmixMode=3; /*NEXUS_AudioDecoderAacDownmixMode_eLoRo*/
			  }
	  }
	  else if(!strcmp(line,"RFMODE"))
	  {
			 if(atoi(value)==0) {
				  dolby->codecSettings.codecSettings.aacPlus.drcMode=NEXUS_AudioDecoderDolbyPulseDrcMode_eLine; /*NEXUS_AudioDecoderDolbyPulseDrcMode_eLine*/
			  }
			  else if(atoi(value)==1) {
				  dolby->codecSettings.codecSettings.aacPlus.drcMode= NEXUS_AudioDecoderDolbyPulseDrcMode_eRf; /*NEXUS_AudioDecoderDolbyPulseDrcMode_eRf*/
			  }	      
	  }
	  else if(!strcmp(line,"DRCCUT"))
		  dolby->codecSettings.codecSettings.aacPlus.cut=atoi(value);

	  else if(!strcmp(line,"DRCBOOST"))
		 dolby->codecSettings.codecSettings.aacPlus.boost = atoi(value);

     else if(!strcmp(line,"TRANSCODEENABLED"))
	  {
		  if(atoi(value)==1)
		 {
			  dolby->transcode=true;
		  }
		  else{
			  dolby->transcode=false;
		  }

	  }
	}

	return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------Run time change in the config parameters----------------------------------------------------------------------------------------*/
static void change_config(char *input,struct dolby_pulse_command_args *dolby)
{
    char *value;
   value = strchr(input, '=');
					*value = 0;
					value++;
                
				  if(strstr(input,"OUTPUTMODE"))
					  dolby->decodeSettings.outputMode= atoi(value);	      
				  else if(strstr(input,"OUTPUTLFECHANNEL"))
					  dolby->decodeSettings.outputLfeMode=atoi(value);
				  else if(strstr(input,"DUALMONO_MODE "))
					    dolby->decodeSettings.dualMonoMode=atoi(value);

				 else if(!strcmp(input,"DOWNMIX_MODE"))
				 {
					 if(atoi(value)==0) {
						 dolby->codecSettings.codecSettings.aacPlus.downmixMode=NEXUS_AudioDecoderAacDownmixMode_eLtRt; /*NEXUS_AudioDecoderAacDownmixMode_eLtRt*/
					 }
			     else if(atoi(value)==1) {
						dolby->codecSettings.codecSettings.aacPlus.downmixMode=NEXUS_AudioDecoderAacDownmixMode_eLoRo; /*NEXUS_AudioDecoderAacDownmixMode_eLoRo*/
					}
				 }
				 else if(!strcmp(input,"RFMODE"))
				 {
					 if(atoi(value)==0) {
						 dolby->codecSettings.codecSettings.aacPlus.drcMode=NEXUS_AudioDecoderDolbyPulseDrcMode_eLine; /*NEXUS_AudioDecoderDolbyPulseDrcMode_eLine*/
					 }
					 else if(atoi(value)==1) {
						 dolby->codecSettings.codecSettings.aacPlus.drcMode=NEXUS_AudioDecoderDolbyPulseDrcMode_eRf; /*NEXUS_AudioDecoderDolbyPulseDrcMode_eRf*/
					 }	      
				 }
				  else if(strstr(input,"DRCCUT"))
 					    dolby->codecSettings.codecSettings.aacPlus.cut=atoi(value);

				  else if(strstr(input,"DRCBOOST"))
					   dolby->codecSettings.codecSettings.aacPlus.boost = atoi(value);
			
                 return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------Printing the current values of the config parameters-----------------------------------------------------------------------------------------*/
static void print_settings(NEXUS_AudioDecoderSettings decodeSettings,NEXUS_AudioDecoderCodecSettings codecSettings,struct dolby_pulse_command_args *dolby)
{
	printf("\n\n---------------- The current values of the configuration parameters are as follows: ----------------- \n\n");
    
    printf("\n(1)OUTPUTMODE = %d\n(2)OUTPUTLFECHANNEL = %d\n(3)DUALMONO_MODE  = %d\n(4)DOWNMIX_MODE = %d\n(5)RFMODE = %d\n(6)DRCCUT = %d\n(7)DRCBOOST = %d\n(8)TRANSCODEENABLED = %d\n\n",decodeSettings.outputMode,decodeSettings.outputLfeMode,decodeSettings.dualMonoMode,(codecSettings.codecSettings.aacPlus.downmixMode)-2,codecSettings.codecSettings.aacPlus.drcMode,codecSettings.codecSettings.aacPlus.cut,codecSettings.codecSettings.aacPlus.boost,dolby->transcode);

	printf("\n\n To change the value of any config parameters, type the command as 'CONFIG PARAMETER NAME= VALUE' \n Type 'quit' to terminate the application. \n\n\n");
	return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------Command line options parsing----------------------------------------------------------------------------------------------------------*/
int dolby_pulse_cmdline_parse(int argc, const char *argv[], struct dolby_pulse_command_args *dolby)
{
    int i;
	unsigned int val;

   for (i=1;i<argc;i++) {
		if (!strcmp(argv[i], "-config")) {

            dolby->config_file=argv[++i];
			printf("\n\n\n You entered the config file name :%s\n\n\n",dolby->config_file);
			
		}
		else if(!strcmp(argv[i],"-vidpid")) {
			
			sscanf(argv[++i],"%x",&val);
			dolby->vidpid = (uint16_t)val;
			printf("\n\n\n You entered the vidpid as:0x%x\n\n\n",dolby->vidpid);
		}
		else if(!strcmp(argv[i],"-audpid1")) {
		
			sscanf(argv[++i],"%x",&val);
			dolby->audpid1 = (uint16_t)val;
			printf("\n\n\n You entered the main audpid as:0x%x\n\n\n",dolby->audpid1);
		}
		else if(!strcmp(argv[i],"-audpid2")) {
		
			sscanf(argv[++i],"%x",&val);
			dolby->audpid2 = (uint16_t)val;
			printf("\n\n\n You entered the associated audpid as:0x%x\n\n\n",dolby->audpid2);
		}
		else if(!strcmp(argv[i],"-single")) {
			dolby->MixingOutput=1;
			printf("\n\n\n You entered the option for decoding Main stream only\n\n");
		}
		else if(!strcmp(argv[i],"-associated")) {
			dolby->MixingOutput=2;
			printf("\n\n\n You entered the option for decoding Associated stream only\n\n");
		}
		else if(!strcmp(argv[i],"-mixing")) {
			dolby->MixingOutput=3;
			printf("\n\n\n You entered the option for decoding Main+Associated streams \n\n");
		}
           
        else {
            printf("unknown param %s\n", argv[i]);
            return -1;
        }
    }

   return 0;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


int main(int argc, const char *argv[]) {

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
	
    struct dolby_pulse_command_args dolby_args;
    
	FILE  *file_params=NULL;

    bool exit;
    
	NEXUS_AudioCaptureStartSettings captureSettings;
	NEXUS_AudioCaptureHandle capture;

    NEXUS_Ac3EncodeHandle ac3Encode;

    
	FILE *pFile;
	const char *pFileName = "audio_capture.dat";

	NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    
    NEXUS_PidChannelHandle videoPidChannel, mainPidChannel, secondaryPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle mainDecoder, secondaryDecoder;
    NEXUS_AudioDecoderStartSettings mainProgram, secondaryProgram;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioMixerHandle mixer;
/*------------------------------------------------------------Initialization------------------------------------------------------------------------------------------------------------------------*/  
	if (dolby_pulse_cmdline_parse(argc, argv, &dolby_args)) {
		 return 0;
	 }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

	 NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, dolby_args.vidpid, NULL);
    mainPidChannel = NEXUS_PidChannel_Open(parserBand, dolby_args.audpid1, NULL);
    secondaryPidChannel = NEXUS_PidChannel_Open(parserBand, dolby_args.audpid2, NULL);
    
	pFile = fopen(pFileName, "wb+");
        if ( NULL == pFile )
		{
			fprintf(stderr, "Unable to open file '%s' for writing captured data\n", pFileName);
			return 0;
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
    mainProgram.codec = NEXUS_AudioCodec_eAacPlus;
    mainProgram.pidChannel = mainPidChannel;
    mainProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&secondaryProgram);
    secondaryProgram.codec = NEXUS_AudioCodec_eAacPlus;
    secondaryProgram.pidChannel = secondaryPidChannel;
    secondaryProgram.stcChannel = stcChannel;
    secondaryProgram.secondaryDecoder = true;   /* Indicate this is a secondary channel for STC Channel/PCRlib functions */


    
    /* Bring up the primary audio decoder */
    mainDecoder = NEXUS_AudioDecoder_Open(0, NULL);

    /* Open audio descriptor decoder */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    secondaryDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);

    /* Open mixer to mix the description and primary audio */
    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);
   
	capture = NEXUS_AudioCapture_Open(0, NULL);
   
    ac3Encode = NEXUS_Ac3Encode_Open(NULL);

    
    file_params = fopen(dolby_args.config_file, "r");

	NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);
    
	NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);
    

		     if(file_params){

			 parse_config_file(&dolby_args);
             
             fclose(file_params);
		  }

    NEXUS_AudioDecoder_SetSettings(mainDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_SetCodecSettings(mainDecoder,&dolby_args.codecSettings);
	NEXUS_AudioDecoder_SetSettings(secondaryDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_SetCodecSettings(secondaryDecoder,&dolby_args.codecSettings);


	NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);

	printf("\n\n After setting the values from the config file, the current values of the parameters for the maindecoder are:\n");
	print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);

	NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);

	printf("\n\n After setting the values from the config file, the current values of the parameters for the secondarydecoder are:\n");
	print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------Adding the audio input to the corresponding output ports---------------------------------------------------------------------------*/
	/* Add both decoders to the mixer */
	NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
	NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

	/* Set the Mixer to use DSP mixing */
	NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    if(dolby_args.MixingOutput==2)
	{
	
		mixerSettings.master = NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
	}
 else
	{
	 mixerSettings.master = NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
	}
	NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);


    
    if(dolby_args.MixingOutput==1) {

                        if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									 NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
																	NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));


						}
						else if(dolby_args.transcode==true){
							NEXUS_Ac3Encode_AddInput(ac3Encode,NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
							printf("\n\n You should expect AC3 audio at the SPDIF output\n\n");
							NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode));

									
						}

	}
	else if(dolby_args.MixingOutput==2) {

						if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
																   NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));


						}
						else if(dolby_args.transcode==true){
							NEXUS_Ac3Encode_AddInput(ac3Encode,NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
							printf("\n\n You should expect AC3 audio at the SPDIF output\n\n");
							NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode));


						}

	}
	else if(dolby_args.MixingOutput==3) {

						if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
									NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
									NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
																   NEXUS_AudioMixer_GetConnector(mixer));


						}
						else if(dolby_args.transcode==true){
							NEXUS_Ac3Encode_AddInput(ac3Encode,NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eMultichannel));
							printf("\n\n You should expect AC3 audio at the SPDIF output\n\n");
							NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
							NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode));


						}

	}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

	BDBG_WRN(("Starting Capturing"));
 /* Start the capture -- no data will be received until the decoder starts */
    NEXUS_AudioCapture_GetDefaultStartSettings(&captureSettings);
    captureSettings.dataCallback.callback = capture_callback;
    captureSettings.dataCallback.context = capture;
    captureSettings.dataCallback.param = (int)pFile;
    NEXUS_AudioCapture_Start(capture, &captureSettings);

/*----------------------------------------------------------------Video and display settings--------------------------------------------------------------------------------------------------------*/					
					
    /* Bring up video display and outputs */
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eCustom1; /* 50Hz for PAL streams */
    display = NEXUS_Display_Open(0, &displaySettings);
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
    fprintf(stderr, "Panel output ready\n");
#else
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
   if(platformConfig.outputs.component[0]){
    	NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
   }
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#endif
    window = NEXUS_VideoWindow_Open(display, 0);

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------Start Decoders--------------------------------------------------------------------------------------------------------- */
	NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    BDBG_WRN(("Starting Main"));
	NEXUS_AudioDecoder_Start(mainDecoder, &mainProgram);
	
    BDBG_WRN(("Starting Secondary"));
	NEXUS_AudioDecoder_Start(secondaryDecoder, &secondaryProgram);
    
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------Handling the runtime change in the config parameters---------------------------------------------------------------------*/
	for(exit=false;!exit;) {
        char buffer[256];
        char *buf;


        printf("Dolby_Pulse>"); fflush(stdout);
        fgets(buffer, 256, stdin);
        if(feof(stdin)) {
            break;
        }

        buffer[strlen(buffer)-1] = 0;

        buf = strtok(buffer, ";");
        if (!buf) continue;

		do {
            if (!strcmp(buf, "?")) {
               printf("\nRun the app by running the following command:\n");
		       printf("\n ./\\nexus dolby_pulse -config configfilename -single/\\-associated/\\-mixing -vidpid videopid_value -audpid1 mainaudiopid_value -audpid2 associatedaudiopid_value \n");
               printf("\n\n To change the value of any config parameters, type the command as 'CONFIG PARAMETER NAME= VALUE' \n Type 'quit' to terminate the application. \n\n\n");
		
            }
			 else if (!strcmp(buf, "quit")) {
                exit = true;
                break;
            }
        	else if ((strstr(buf, "OUTPUTMODE") != NULL)||(strstr(buf, "OUTPUTLFECHANNEL") != NULL)||(strstr(buf, "DUALMONO_MODE ") != NULL)||(strstr(buf, "DOWNMIX_MODE") != NULL)||(strstr(buf, "RFMODE") != NULL)||(strstr(buf, "DRCCUT") != NULL)||(strstr(buf, "DRCBOOST") != NULL)||(strstr(buf, "NUMOUTPORTS") != NULL)||(strstr(buf, "TRANSCODEENABLED") != NULL)) {
				
				
			
				if(dolby_args.MixingOutput==1) {
					NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);
	
					change_config(buf,&dolby_args);
					NEXUS_AudioDecoder_SetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_SetCodecSettings(mainDecoder,&dolby_args.codecSettings);
	
					NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);
	
					printf("\n\n After changing the values for the main output are:\n");
					print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);
	
				}
				else if(dolby_args.MixingOutput==2) {
					NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);
	
					change_config(buf,&dolby_args);
					NEXUS_AudioDecoder_SetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_SetCodecSettings(secondaryDecoder,&dolby_args.codecSettings);
	
					NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);
	
					printf("\n\n After changing the values for the associated output are:\n");
					print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);
	
				}
				else if(dolby_args.MixingOutput==3) {

					NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);
	
					change_config(buf,&dolby_args);
					NEXUS_AudioDecoder_SetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_SetCodecSettings(mainDecoder,&dolby_args.codecSettings);
	
					NEXUS_AudioDecoder_GetSettings(mainDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(mainDecoder,mainProgram.codec,&dolby_args.codecSettings);
	
					printf("\n\n After changing the values for the main output are:\n");
					print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);


					NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);

					change_config(buf,&dolby_args);
					NEXUS_AudioDecoder_SetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_SetCodecSettings(secondaryDecoder,&dolby_args.codecSettings);

					NEXUS_AudioDecoder_GetSettings(secondaryDecoder,&dolby_args.decodeSettings);
					NEXUS_AudioDecoder_GetCodecSettings(secondaryDecoder,secondaryProgram.codec,&dolby_args.codecSettings);

					printf("\n\n After changing the values for the associated output are:\n");
					print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);

				}

        
			}
            else if (!*buf) {
                /* allow blank line */
            }
            else {
                printf("unknown command: %s\n", buf);
            }
        }
        while ((buf = strtok(NULL, ";")));
    }
    

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------Closing and uninitializing the resources-----------------------------------------------------------------------------------------------*/
	if(dolby_args.MixingOutput==1) {

						if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
																   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));


						}
						else if(dolby_args.transcode==true){
							
							NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
							NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode) );



						}

	}
	else if(dolby_args.MixingOutput==2) {

						if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
																   NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioDecoderConnectorType_eStereo));



						}
						else if(dolby_args.transcode==true){
							
							NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
                            NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode) );
						}

	}
	else if(dolby_args.MixingOutput==3) {

						if(dolby_args.transcode==false) {
								NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
									NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_AudioMixer_GetConnector(mixer) );

						}
						else if(dolby_args.transcode==true){
							
							NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
													   NEXUS_Ac3Encode_GetConnector(ac3Encode));
							NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												  NEXUS_AudioMixer_GetConnector(mixer));
							NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
														   NEXUS_Ac3Encode_GetConnector(ac3Encode) );
						}

	}

	/*NEXUS_AudioOutput_RemoveInput(
		NEXUS_AudioCapture_GetConnector(capture),
							   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));*/

  
 /* example shutdown */
    NEXUS_AudioDecoder_Stop(secondaryDecoder);
    NEXUS_AudioDecoder_Stop(mainDecoder);
   
	NEXUS_StopCallbacks(capture);


    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(mixer));
    NEXUS_AudioMixer_Close(mixer);
    NEXUS_AudioDecoder_Close(secondaryDecoder);
    NEXUS_AudioDecoder_Close(mainDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(mainPidChannel);
    NEXUS_PidChannel_Close(secondaryPidChannel);
    NEXUS_Platform_Uninit();

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    return 0;
}
