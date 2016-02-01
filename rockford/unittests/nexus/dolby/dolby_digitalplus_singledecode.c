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

struct dolby_digital_plus_command_args {


	const char *config_file;
    bool ddc_enabled;
	int capture_selection;
    int loopback;

	NEXUS_AudioDecoderSettings decodeSettings;
	NEXUS_AudioDecoderCodecSettings codecSettings;

   };
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

static bool g_isVideoEs = true;


const namevalue_t g_videoFormatStrs_dolby[] = {
    {"ntsc",      NEXUS_VideoFormat_eNtsc},
    {"480i",      NEXUS_VideoFormat_eNtsc},
    {"pal",       NEXUS_VideoFormat_ePal},
    {"576i",      NEXUS_VideoFormat_ePal},
    {"1080i",     NEXUS_VideoFormat_e1080i},
    {"720p",      NEXUS_VideoFormat_e720p},
    {"480p",      NEXUS_VideoFormat_e480p},
    {"576p",      NEXUS_VideoFormat_e576p},
    {NULL, 0}
};

const namevalue_t g_videoFrameRateStrs_dolby[] = {
    {"0",      NEXUS_VideoFrameRate_eUnknown},
    {"23.976", NEXUS_VideoFrameRate_e23_976},
    {"24",     NEXUS_VideoFrameRate_e24},
    {"25",     NEXUS_VideoFrameRate_e25},
    {"29.97",  NEXUS_VideoFrameRate_e29_97},
    {"30",     NEXUS_VideoFrameRate_e30},
    {"50",     NEXUS_VideoFrameRate_e50},
    {"59.94",  NEXUS_VideoFrameRate_e59_94},
    {"60",     NEXUS_VideoFrameRate_e60},
    {NULL, 0}
};

static const namevalue_t g_videoCodecStrs_dolby[] = {
    {"mpeg2", NEXUS_VideoCodec_eMpeg2},
    {"mpeg", NEXUS_VideoCodec_eMpeg2},
    {"mpeg1", NEXUS_VideoCodec_eMpeg1},
    {"avc", NEXUS_VideoCodec_eH264},
    {"h264", NEXUS_VideoCodec_eH264},
    {"h263", NEXUS_VideoCodec_eH263},
    {"avs", NEXUS_VideoCodec_eAvs},
    {"vc1", NEXUS_VideoCodec_eVc1},
    {"vc1sm", NEXUS_VideoCodec_eVc1SimpleMain},
    {"divx", NEXUS_VideoCodec_eMpeg4Part2},
    {"mpeg4", NEXUS_VideoCodec_eMpeg4Part2},
    {"divx311", NEXUS_VideoCodec_eDivx311},
    {"divx3", NEXUS_VideoCodec_eDivx311},
    {NULL, 0}
};

static const namevalue_t g_videoErrorHandling_dolby[] = {
    {"none", NEXUS_VideoDecoderErrorHandling_eNone},
    {"picture", NEXUS_VideoDecoderErrorHandling_ePicture},
    {"prognostic", NEXUS_VideoDecoderErrorHandling_ePrognostic},
    {NULL, 0}
};

/*----------------------------------------------------------------Audio capture callback------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------Starting and stopping of audio/video--------------------------------------------------------------------------------------------------*/
static void start_video(const struct util_opts_t *opts, NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *videoProgram)
{
    NEXUS_Error rc;
    if (opts->common.videoPid) {
        rc = NEXUS_VideoDecoder_Start(videoDecoder, videoProgram);
        BDBG_ASSERT(!rc);
    }
    return;
}
static void stop_video(const struct util_opts_t *opts, NEXUS_VideoDecoderHandle videoDecoder)
{
    if (opts->common.videoPid) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
    }
    return;
}

static void start_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder, const NEXUS_AudioDecoderStartSettings *audioProgram,NEXUS_AudioCaptureHandle capture, FILE *pFile)
{
    NEXUS_Error rc;
    NEXUS_AudioCaptureStartSettings captureSettings;

    if (opts->common.audioPid) {
#if B_HAS_ASF
        /* if DRC for WMA pro is available apply now */
        if(audioProgram->codec == NEXUS_AudioCodec_eWmaPro && opts->common.dynamicRangeControlValid ){

            NEXUS_AudioDecoderCodecSettings codecSettings;

            BDBG_WRN(("wma-pro drc enabled,ref peak %d,ref target %d,ave peak %d, ave target %d",
                      opts->common.dynamicRangeControl.peakReference,opts->common.dynamicRangeControl.peakTarget,
                      opts->common.dynamicRangeControl.averageReference,opts->common.dynamicRangeControl.averageTarget));
            NEXUS_AudioDecoder_GetCodecSettings(audioDecoder, audioProgram->codec, &codecSettings);
            codecSettings.codec = audioProgram->codec;
            codecSettings.codecSettings.wmaPro.dynamicRangeControlValid = true;
            codecSettings.codecSettings.wmaPro.dynamicRangeControl.peakReference = opts->common.dynamicRangeControl.peakReference;
            codecSettings.codecSettings.wmaPro.dynamicRangeControl.peakTarget = opts->common.dynamicRangeControl.peakTarget;
            codecSettings.codecSettings.wmaPro.dynamicRangeControl.averageReference = opts->common.dynamicRangeControl.averageReference;
            codecSettings.codecSettings.wmaPro.dynamicRangeControl.averageTarget = opts->common.dynamicRangeControl.averageTarget;
            NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&codecSettings);
            }
#endif
  /* Start the capture -- no data will be received until the decoder starts */
    NEXUS_AudioCapture_GetDefaultStartSettings(&captureSettings);
    captureSettings.dataCallback.callback = capture_callback;
    captureSettings.dataCallback.context = capture;
    captureSettings.dataCallback.param = (int)pFile;
    NEXUS_AudioCapture_Start(capture, &captureSettings);

        if(opts->common.decodedAudio) {
            rc = NEXUS_AudioDecoder_Start(audioDecoder, audioProgram);
            BDBG_ASSERT(!rc);
        }
      
        if(compressedDecoder) {
            rc = NEXUS_AudioDecoder_Start(compressedDecoder, audioProgram);
            BDBG_ASSERT(!rc);
        }
    }
    return;
}

static void stop_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder, NEXUS_AudioCaptureHandle capture)
{
	fprintf(stderr, "Stopping capture\n");
	NEXUS_StopCallbacks(capture);
	NEXUS_AudioCapture_Stop(capture);

    if (opts->common.audioPid) {
        if(opts->common.decodedAudio) {
            NEXUS_AudioDecoder_Stop(audioDecoder);
			NEXUS_AudioCapture_Stop(capture);
        }
     
        if(compressedDecoder) {
            NEXUS_AudioDecoder_Stop(compressedDecoder);
        }
    }
    return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------Playback monitoring-----------------------------------------------------------------------------------------------------------------------*/
#define  B_HAS_PLAYBACK_MONITOR 0
#if B_HAS_PLAYBACK_MONITOR
#include <pthread.h>
#include "bkni_multi.h"

typedef struct PlaybackMonitorState {
    bool terminate;
    pthread_t thread;
    BKNI_MutexHandle lock;
    const struct util_opts_t *opts;
    NEXUS_PlaybackHandle playback;
    NEXUS_VideoDecoderHandle videoDecoder;
    const NEXUS_VideoDecoderStartSettings *videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderHandle compressedDecoder;
    const NEXUS_AudioDecoderStartSettings *audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_FilePlayHandle customFile;
    NEXUS_FilePlayHandle stickyFile;
    const NEXUS_PlaybackStartSettings *playbackStartSettings;
} PlaybackMonitorState;

static void *monitor_thread(void *state_)
{
    const PlaybackMonitorState *state=state_;
    while(!state->terminate) {
        NEXUS_PlaybackStatus status;
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        BERR_Code rc;
        bool failed;

        rc = NEXUS_Playback_GetStatus(state->playback, &status);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(1000);
        FileIoSticky_GetFailBit(state->stickyFile, &failed);
        if(!failed) {
            continue;
        }
        BDBG_WRN(("restarting from %u", status.position));
        BKNI_AcquireMutex(state->lock);
        NEXUS_Playback_Stop(state->playback);
        FileIoSticky_ClearFailBit(state->stickyFile);
        if(state->customFile) {
            FileIoCustomProbabilities probabilities;
            FileIoCustom_GetProbabilities(state->customFile, NULL, &probabilities);
            probabilities.error = 0;
            probabilities.nodata = 0;
            probabilities.partial_read = 0;
            FileIoCustom_SetProbabilities(state->customFile, &probabilities, &probabilities);
        }

        if (state->opts->common.videoPid) {
            /* don't show black frame */
            NEXUS_VideoDecoder_GetSettings(state->videoDecoder, &videoDecoderSettings);
            videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
            rc=NEXUS_VideoDecoder_SetSettings(state->videoDecoder, &videoDecoderSettings);
            BDBG_ASSERT(!rc);
        }
        /* stop decoder */
        stop_video(state->opts, state->videoDecoder);
      stop_audio(state->opts, state->audioDecoder, state->compressedDecoder,capture);

        NEXUS_Playback_GetSettings(state->playback, &playbackSettings);
        playbackSettings.startPaused = true;
        rc = NEXUS_Playback_SetSettings(state->playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        /* Start decoders */
        start_video(state->opts, state->videoDecoder, state->videoProgram);
        start_audio(state->opts, state->audioDecoder, state->compressedDecoder, state->audioProgram,capture,pFile);


        /* start playback  */
        rc = NEXUS_Playback_Start(state->playback, state->file, state->playbackStartSettings);
        BDBG_ASSERT(!rc);

        /* seek into desired location */
        rc = NEXUS_Playback_Seek(state->playback, status.position);
        BDBG_ASSERT(!rc);

        /* start playing */
        rc = NEXUS_Playback_Play(state->playback);
        BDBG_ASSERT(!rc);
        BKNI_ReleaseMutex(state->lock);
    }
    return NULL;
}

static void 
monitor_thread_start(PlaybackMonitorState *state)
{
    int rc;
    BKNI_CreateMutex(&state->lock);
    state->terminate = false;
    rc = pthread_create(&state->thread, NULL, monitor_thread, state);
    BDBG_ASSERT(rc==0);
    return;
}

static void 
monitor_thread_stop(PlaybackMonitorState *state)
{
    state->terminate = true;
    pthread_join(state->thread, NULL);
}
#endif /* B_HAS_PLAYBACK_MONITOR */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------Parsing the config file and initializing the parameters-------------------------------------------------------------------------------------*/

static void parse_config_file(struct dolby_digital_plus_command_args *dolby)
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
				  dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
			  }
			  else if(atoi(value)==1) {
				  dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible; /*Lt/Rt*/
			  }
			  else if(atoi(value)==2) {
				  dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard; /*Lo/Ro*/
			  }

	  }
	  else if(!strcmp(line,"RFMODE"))
	  {
			  if(atoi(value)==0) {
				  dolby->codecSettings.codecSettings.ac3Plus.drcMode=NEXUS_AudioDecoderDolbyDrcMode_eLine;
			  }
			  else if(atoi(value)==1) {
				  dolby->codecSettings.codecSettings.ac3Plus.drcMode=NEXUS_AudioDecoderDolbyDrcMode_eRf;
			  }	      
	  }
	  else if(!strcmp(line,"DRCCUT"))
		  dolby->codecSettings.codecSettings.ac3Plus.cut = atoi(value);

	  else if(!strcmp(line,"DRCBOOST"))
		 dolby->codecSettings.codecSettings.ac3Plus.boost = atoi(value);

     else if(!strcmp(line,"DDC_ENABLED"))
	  {
		  if(atoi(value)==1)
		 {
			  dolby->ddc_enabled=true;
		  }
		  else{
			  dolby->ddc_enabled=false;
		  }

	  }
	}

	return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------Runtime change in the config parameters values-----------------------------------------------------------------------------------------------*/

static void change_config(char *input,struct dolby_digital_plus_command_args *dolby)
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
								dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic;
							}
							else if(atoi(value)==1) {
								dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible; /*Lt/Rt*/
							}	
							else if(atoi(value)==2) {
								dolby->codecSettings.codecSettings.ac3Plus.stereoDownmixMode=NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard; /*Lo/Ro*/
							}
						}

					else if(!strcmp(input,"RFMODE"))
					{
						 if(atoi(value)==0) {
							 dolby->codecSettings.codecSettings.ac3Plus.drcMode=NEXUS_AudioDecoderDolbyDrcMode_eLine;
						 }
						 else if(atoi(value)==1) {
							 dolby->codecSettings.codecSettings.ac3Plus.drcMode=NEXUS_AudioDecoderDolbyDrcMode_eRf;
						 }
					}         
	  				  else if(strstr(input,"DRCCUT"))
					    dolby->codecSettings.codecSettings.ac3Plus.cut=atoi(value);

				  else if(strstr(input,"DRCBOOST"))
					   dolby->codecSettings.codecSettings.ac3Plus.boost = atoi(value);
			
                  else if(strstr(input,"DDC_ENABLED"))
				  {
					  if(atoi(value)==1) {
						  dolby->ddc_enabled=true;
					  }
					  else{
						  dolby->ddc_enabled=false;
					  }

				  }    
				  return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------Priting the current values of the config parameters--------------------------------------------------------------------------*/

static void print_settings(NEXUS_AudioDecoderSettings decodeSettings,NEXUS_AudioDecoderCodecSettings codecSettings,struct dolby_digital_plus_command_args *dolby)
{
	printf("\n\n---------------- The current values of the configuration parameters are as follows: ----------------- \n\n");
    
    printf("\n(1)OUTPUTMODE = %d\n(2)OUTPUTLFECHANNEL = %d\n(3)DUALMONO_MODE  = %d\n(4)DOWNMIX_MODE = %d\n(5)RFMODE = %d\n(6)DRCCUT = %d\n(7)DRCBOOST = %d\n(8)DDC_ENABLED = %d\n\n",decodeSettings.outputMode,decodeSettings.outputLfeMode,decodeSettings.dualMonoMode,(codecSettings.codecSettings.ac3Plus.stereoDownmixMode),codecSettings.codecSettings.ac3Plus.drcMode,codecSettings.codecSettings.ac3Plus.cut,codecSettings.codecSettings.ac3Plus.boost,dolby->ddc_enabled);

	printf("\n\n To change the value of any config parameters, type the command as 'CONFIG PARAMETER NAME= VALUE' \n Type 'quit' to terminate the application. \n\n\n");
	return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------Parsing of the command line arguments------------------------------------------------------------------------------------------*/


int dolby_digital_plus_cmdline_parse(int argc, const char *argv[], struct util_opts_t *opts, struct dolby_digital_plus_command_args *dolby)
{
    int i;
	int codectypereturn = 0;

	dolby->loopback=0;

    memset(opts,0,sizeof(*opts));
    opts->common.transportType = NEXUS_TransportType_eTs;
    opts->common.videoCodec = NEXUS_VideoCodec_eMpeg2;
    opts->common.audioCodec = NEXUS_AudioCodec_eMpeg;
    opts->common.contentMode = NEXUS_VideoWindowContentMode_eFull;
    opts->common.compressedAudio = false;
    opts->common.decodedAudio = true;
#if NEXUS_DTV_PLATFORM
    opts->common.displayType = NEXUS_DisplayType_eLvds;
    opts->common.displayFormat = NEXUS_VideoFormat_eCustom0;
#else
    opts->common.useCompositeOutput = true;
    opts->common.useComponentOutput = true;
    opts->common.displayFormat = NEXUS_VideoFormat_eNtsc;
    opts->common.displayType = NEXUS_DisplayType_eAuto;
#endif
    opts->stcChannelMaster = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    opts->common.tsTimestampType = NEXUS_TransportTimestampType_eNone;
    opts->beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts->endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts->common.videoFrameRate = NEXUS_VideoFrameRate_eUnknown;
    opts->videoErrorHandling = NEXUS_VideoDecoderErrorHandling_eNone;
    opts->common.playpumpTimestampReordering = true;
    opts->customFileIo = false;
    opts->playbackMonitor = false;

    for (i=1;i<argc;i++) {
        
        if (!strcmp(argv[i], "-probe")) {
            opts->common.probe = true;
        }
		else if (!strcmp(argv[i], "-audio_type") ) { /* Added the feature to enforce the audiotype considering few special cases where streams have both types of packets*/

			opts->common.audioCodec=lookup(g_audioCodecStrs, argv[++i]);

			if(opts->common.audioCodec == NEXUS_AudioCodec_eAc3Plus) {
				printf("\n You have selected Ac3Plus as an audiocodec \n");
				codectypereturn = 1;
			}
			else if(opts->common.audioCodec == NEXUS_AudioCodec_eAc3) {
				printf("\n You have selected Ac3 as an audiocodec \n");
				codectypereturn = 2;
			}
            g_isVideoEs = false;
        }
		else if (!strcmp(argv[i], "-config")) {

            dolby->config_file=argv[++i];
			printf("\n\n\n You entered the config file name :%s\n\n\n",dolby->config_file);
			
		}
		else if (!strcmp(argv[i], "-loopback")) {

            dolby->loopback=1;
			printf("\n\n\n You have enabled the loopback option\n\n\n");

			
		}
	   else if (!opts->filename) {
            opts->filename = argv[i];
        }
        else if (!opts->indexname) {
            opts->indexname = argv[i];
        }
        else {
            printf("unknown param %s\n", argv[i]);
            return -1;
        }
    }

    opts->common.displayType = NEXUS_DisplayType_eAuto;

    /* this allows the user to set: "-mpeg_type es -video_type mpeg" and forget the "-video 1" option */
    if (opts->common.transportType == NEXUS_TransportType_eEs && !opts->common.videoPid && !opts->common.audioPid) {
        if (g_isVideoEs) {
            opts->common.videoPid = 1;
        }
        else {
            opts->common.audioPid = 1;
        }
    }

    return codectypereturn;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


int main(int argc, const char *argv[]) {
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel = NULL, audioPidChannel = NULL, pcrPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayVbiSettings displayVbiSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder, compressedDecoder=NULL;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file,customFile=NULL, stickyFile=NULL;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackStatus pstatus;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;
    NEXUS_VideoDecoderOpenSettings openSettings;
	
    NEXUS_Error rc;

    
	struct dolby_digital_plus_command_args dolby_args;
    struct util_opts_t opts;

	FILE  *file_params=NULL;

    bool exit;
	NEXUS_SurfaceHandle framebuffer = NULL;

	
	NEXUS_AudioCaptureHandle capture;

	
	FILE *pFile;
	const char *pFileName = "audio_capture.dat";

	int audcodec,i;
/*-------------------------------------------------------------------Initialization----------------------------------------------------------------------------------------------------------------*/  
	audcodec = dolby_digital_plus_cmdline_parse(argc, argv, &opts, &dolby_args);
		 
    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    if(opts.avc51) {
		 for (i=0;i<NEXUS_NUM_XVD_DEVICES;i++) {
			 platformSettings.videoDecoderModuleSettings.heapSize[i].general = (84*1024*1024);
		 }
        platformSettings.videoDecoderModuleSettings.numDecodes = 1;
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    
	pFile = fopen(pFileName, "wb+");
        if ( NULL == pFile )
		{
			fprintf(stderr, "Unable to open file '%s' for writing captured data\n", pFileName);
			return 0;
		}
		if(dolby_args.loopback==1) {
			opts.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
		}
		else{
			opts.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
		}
 
		if (cmdline_probe(&opts.common, opts.filename, &opts.indexname)) {
        return 1;
    }
 /*------------------------------Added the feature to enforce the audiotype considering few special cases where streams have both types of packets--------------------------------------------------*/
	 if(audcodec == 1) {
		 printf("\n\n***********************Applying Ac3Plus codec ***********************\n\n");
		 opts.common.audioCodec = NEXUS_AudioCodec_eAc3Plus;
	 }
	 else if(audcodec == 2) {
		 printf("\n\n***********************Applying Ac3 codec ***********************\n\n");
		 opts.common.audioCodec = NEXUS_AudioCodec_eAc3;
	 }
		 
 /*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/    

    if (!opts.filename) {
        print_usage(argv[0]);
        return 0;
    }

    playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    if ((opts.indexname && !strcmp(opts.indexname, "same")) ||
        opts.common.transportType == NEXUS_TransportType_eMkv ||
        opts.common.transportType == NEXUS_TransportType_eMp4
        )
    {
        opts.indexname = opts.filename;
    }

    file = NEXUS_FilePlay_OpenPosix(opts.filename, opts.indexname);
    if (!file) {
        fprintf(stderr, "can't open files:%s %s\n", opts.filename, opts.indexname);
        return -1;
    }
    if(opts.customFileIo) {
        customFile = file = FileIoCustom_Attach(file);
        BDBG_ASSERT(file);
    }
    if(opts.playbackMonitor) {
        stickyFile = file = FileIoSticky_Attach(file);
        BDBG_ASSERT(file);
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = opts.stcChannelMaster;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = opts.common.transportType;
    playbackSettings.playpumpSettings.timestamp.pacing = false;
    playbackSettings.playpumpSettings.timestamp.type = opts.common.tsTimestampType;

    playbackSettings.stcChannel = stcChannel;
    playbackSettings.stcTrick = opts.stcTrick;
    playbackSettings.beginningOfStreamAction = opts.beginningOfStreamAction;
    playbackSettings.endOfStreamAction = opts.endOfStreamAction;
    playbackSettings.enableStreamProcessing = opts.streamProcessing;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    
    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    BDBG_ASSERT(audioDecoder);
	

	capture = NEXUS_AudioCapture_Open(0, NULL);
    BDBG_ASSERT(capture);

    file_params = fopen(dolby_args.config_file, "r");
    
	NEXUS_AudioDecoder_GetSettings(audioDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(audioDecoder,audioProgram.codec,&dolby_args.codecSettings);

    
			 if(file_params){

			 parse_config_file(&dolby_args);
             
			 fclose(file_params);
		  }
			 

                
             
	NEXUS_AudioDecoder_SetSettings(audioDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&dolby_args.codecSettings);

    
    NEXUS_AudioDecoder_GetSettings(audioDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(audioDecoder,audioProgram.codec,&dolby_args.codecSettings);

	printf("\n\n After setting the values from the config file, the current values of the parameters for the maindecoder are:\n");
	print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/   

/*----------------------------------------------------Adding the audio to the respective audio output ports--------------------------------------------------------------------------------------------------------------------------------*/
    if(dolby_args.ddc_enabled==true) {

                        printf("\n\n You should expect AC3 audio at the SPDIF output\n\n");
			
						NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed)); /*AC3 Bit stream*/

						NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
											   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

						NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed)); /*AC3 Bit stream*/
            }
					else if(dolby_args.ddc_enabled==false){
						printf("\n\n You should expect PCM audio at the SPDIF output\n\n");
						NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
						NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
						NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
						
					}
					else if(opts.common.compressedAudio) {
						compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
						BDBG_ASSERT(compressedDecoder);
						rc = NEXUS_AudioOutput_AddInput(
							NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
							NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
							BDBG_ASSERT(!rc);
					} 

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/	
 
    
/*----------------------------------------------------Video/Display configuration-----------------------------------------------------------------------------------------------------------------------------*/
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = opts.common.displayType;
    displaySettings.format = opts.common.displayFormat;
    display = NEXUS_Display_Open(0, &displaySettings);
    if (opts.common.useCompositeOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
        BDBG_ASSERT(!rc);
    }
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (opts.common.useComponentOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        BDBG_ASSERT(!rc);
    }
#endif

    if (opts.graphics) {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_GraphicsSettings graphicsSettings;
        NEXUS_VideoFormatInfo videoFormatInfo;
        unsigned i,j;

        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 720;
        surfaceCreateSettings.height = videoFormatInfo.height;
        framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);
        NEXUS_Surface_GetMemory(framebuffer, &mem);
        for (i=0;i<surfaceCreateSettings.height;i++) {
            for (j=0;j<surfaceCreateSettings.width;j++) {
                /* create checker board */
                ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (((i/50)%2) ^ ((j/50)%2)) ? 0x00000000 : 0xFFFFFFFF;
            }
        }
        NEXUS_Surface_Flush(framebuffer);

        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = surfaceCreateSettings.width;
        graphicsSettings.clip.height = surfaceCreateSettings.height;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
        BDBG_ASSERT(!rc);
    }

    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.contentMode = opts.common.contentMode;
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    if(opts.common.videoCdb) {
        openSettings.fifoSize = opts.common.videoCdb*1024;
    }
    if(opts.avc51) {
        openSettings.avc51Enabled = opts.avc51;
    }
    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, &openSettings);
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_Display_GetVbiSettings(display, &displayVbiSettings);
    displayVbiSettings.vbiSource = NEXUS_VideoDecoder_GetConnector(videoDecoder);
    displayVbiSettings.closedCaptionEnabled = opts.closedCaptionEnabled;
    displayVbiSettings.closedCaptionRouting = opts.closedCaptionEnabled;
    rc = NEXUS_Display_SetVbiSettings(display, &displayVbiSettings);
    BDBG_ASSERT(!rc);

    /* Open the audio and video pid channels */
    if (opts.common.videoCodec != NEXUS_VideoCodec_eNone) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidSettings.allowTimestampReordering = opts.common.playpumpTimestampReordering;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.codec = opts.common.videoCodec;
        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.videoPid, &playbackPidSettings);
    }
   
		if (opts.common.audioCodec != NEXUS_AudioCodec_eUnknown) {
			NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
			playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
			playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
			playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = opts.common.audioCodec;
			audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.audioPid, &playbackPidSettings);
		}
    
  
	
    if (opts.common.pcrPid && opts.common.pcrPid!=opts.common.videoPid && opts.common.pcrPid!=opts.common.audioPid) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.pcrPid, &playbackPidSettings);
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.common.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.frameRate = opts.common.videoFrameRate;
    videoProgram.aspectRatio = opts.common.aspectRatio;
    videoProgram.sampleAspectRatio.x = opts.common.sampleAspectRatio.x;
    videoProgram.sampleAspectRatio.y = opts.common.sampleAspectRatio.y;
    videoProgram.errorHandling = opts.videoErrorHandling;
    videoProgram.timestampMode = opts.common.decoderTimestampMode;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = opts.common.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------Start decoders with initializing of the config parameters with the config file values---------------------------------*/
    start_video(&opts, videoDecoder, &videoProgram);
    start_audio(&opts, audioDecoder, compressedDecoder, &audioProgram,capture,pFile);

	NEXUS_AudioDecoder_GetSettings(audioDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_GetCodecSettings(audioDecoder,audioProgram.codec,&dolby_args.codecSettings);

	parse_config_file(&dolby_args);


	NEXUS_AudioDecoder_SetSettings(audioDecoder,&dolby_args.decodeSettings);
	NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&dolby_args.codecSettings);

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------Start playback-------------------------------------------------------------------------------------------------*/
    NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
    if(opts.autoBitrate) {
        playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
    }
    rc = NEXUS_Playback_Start(playback, file, &playbackStartSettings);
    BDBG_ASSERT(!rc);
#if B_HAS_PLAYBACK_MONITOR
    {
        PlaybackMonitorState monitorState;
        BKNI_Memset(&monitorState, 0, sizeof(monitorState));
        monitorState.opts = &opts;
        monitorState.playback = playback;
        monitorState.videoDecoder = videoDecoder;
        monitorState.videoProgram = &videoProgram;
        monitorState.audioDecoder = audioDecoder;
        monitorState.compressedDecoder = compressedDecoder;
        monitorState.audioProgram = &audioProgram;
        monitorState.file = file;
        monitorState.stickyFile = stickyFile;
        monitorState.customFile = customFile;
        monitorState.playbackStartSettings = &playbackStartSettings;
        if(stickyFile) {
            monitor_thread_start(&monitorState);
        }
#endif

   if(dolby_args.loopback==1) {
	for(exit=false;!exit;) {
        char buffer[256];
        char *buf;
       
        printf("Dolby_DigitalPlus>"); fflush(stdout);
        fgets(buffer, 256, stdin);
        if(feof(stdin)) {
            break;
        }

        buffer[strlen(buffer)-1] = 0;

        buf = strtok(buffer, ";");
        if (!buf) continue;
#if B_HAS_PLAYBACK_MONITOR
        if(stickyFile) {
            BKNI_AcquireMutex(monitorState.lock);
        }
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------Managing the runtime change of the config parameters values-------------------------------------------------------------*/
        do {
            if (!strcmp(buf, "?")) {
                printf(
                "Commands:\n"
                "  quit - quit\n"
                "  Parametername=value  - Change the value of any user configurable parameter\n"
                );
            }
			 else if (!strcmp(buf, "quit")) {
                exit = true;
                break;
            }
			else if ((strstr(buf, "OUTPUTMODE") != NULL)||(strstr(buf, "OUTPUTLFECHANNEL") != NULL)||(strstr(buf, "DUALMONO_MODE ") != NULL)||(strstr(buf, "DOWNMIX_MODE") != NULL)||(strstr(buf, "RFMODE") != NULL)||(strstr(buf, "DRCCUT") != NULL)||(strstr(buf, "DRCBOOST") != NULL)||(strstr(buf, "DDC_ENABLED") != NULL)) {
				

				NEXUS_AudioDecoder_GetSettings(audioDecoder,&dolby_args.decodeSettings);
				NEXUS_AudioDecoder_GetCodecSettings(audioDecoder,audioProgram.codec,&dolby_args.codecSettings);


				change_config(buf,&dolby_args);
					
				
                NEXUS_AudioDecoder_SetSettings(audioDecoder,&dolby_args.decodeSettings);
				NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&dolby_args.codecSettings);

				NEXUS_AudioDecoder_GetSettings(audioDecoder,&dolby_args.decodeSettings);
				NEXUS_AudioDecoder_GetCodecSettings(audioDecoder,audioProgram.codec,&dolby_args.codecSettings);

				printf("\n\n After changing the values, the new values are:\n");
				print_settings(dolby_args.decodeSettings,dolby_args.codecSettings,&dolby_args);

        
        	}
            else if (!*buf) {
                /* allow blank line */
            }
            else {
                printf("unknown command: %s\n", buf);
            }
        }
        while ((buf = strtok(NULL, ";")));

#if B_HAS_PLAYBACK_MONITOR
        if(stickyFile) {
            BKNI_ReleaseMutex(monitorState.lock);
        }
#endif
    }
   }
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------- Terminate the application if there is no loopback option set--------------------------------------------------------------------------*/
    else{
	 #if B_HAS_PLAYBACK_MONITOR
					if(stickyFile) {
						BKNI_AcquireMutex(monitorState.lock);
					}
     #endif

	rc = NEXUS_Playback_GetStatus(playback, &pstatus);
	BDBG_ASSERT(!rc);

	while(pstatus.state!=2) {
		sleep(1);
		rc = NEXUS_Playback_GetStatus(playback, &pstatus);
		BDBG_ASSERT(!rc);
     }
    
    #if B_HAS_PLAYBACK_MONITOR
			if(stickyFile) {
				BKNI_ReleaseMutex(monitorState.lock);
			}
	#endif

  }
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------Stopping,closing and uninitializing the resources------------------------------------------------------------------------------------*/
#if B_HAS_PLAYBACK_MONITOR
    if(stickyFile) {
        monitor_thread_stop(&monitorState);
    }
    }
#endif
    NEXUS_Playback_Stop(playback);
    stop_video(&opts, videoDecoder);
   stop_audio(&opts, audioDecoder, compressedDecoder,capture);


    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);


    NEXUS_VideoWindow_RemoveInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    if (framebuffer) {
        NEXUS_Surface_Destroy(framebuffer);
    }

	if(dolby_args.ddc_enabled==true) {
						printf("\n\n You should expect AC3 audio at the SPDIF output\n\n");

						NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed)); /*AC3 Bit stream*/

						NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
											   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

						NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed)); /*AC3 Bit stream*/

					}
					else{
						printf("\n\n You should expect PCM audio at the SPDIF output\n\n");
						NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
												  NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
						NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
						NEXUS_AudioOutput_RemoveInput(NEXUS_AudioCapture_GetConnector(capture),
												   NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

					}


	if(compressedDecoder) {
        NEXUS_AudioOutput_RemoveInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    }

	NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    if(compressedDecoder) {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
        NEXUS_AudioDecoder_Close(compressedDecoder);
	}
    NEXUS_Platform_Uninit();
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    return 0;
}

