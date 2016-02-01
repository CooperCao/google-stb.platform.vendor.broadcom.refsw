/***************************************************************************
 *     Copyright (c) 2009-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "bstd.h"
#include "nexus_timebase.h"
#include "nexus_base_types.h"
#include "nexus_frontend.h"
#include "nexus_frontend_qam.h"
#include "nexus_parser_band.h"
#include "nexus_display.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_types.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_output.h"
#include "nexus_audio_input.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_svideo_output.h"
#include "nexus_audio_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_spdif_output.h"
#include "b_dvr_mgr.h"
#include "b_dvr_timeshift_srvc.h"
#include "b_dvr_recsrvc.h"
#include "b_dvr_pbsrvc.h"
#include "b_dvr_datatypes.h"

BDBG_MODULE(dvrtest);

#define MAX_STATIC_CHANNELS             4
#define MAX_PROGRAMS_PER_CHANNEL        6
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_AUDIO_STREAMS				4
#define MAX_VIDEO_STREAMS				4
#define MAX_BACKGROUND_RECORDING        2
#define CA_3DES_CPS_ENCRYPT 			1

typedef enum{
	dvrtest_cfg_ca=0,
	dvrtest_cfg_cpd
} dvrtest_cfg_keyslot;


/* CA descramble config */
unsigned char VidEvenControlWord[] = { 
										   0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73,  
										   0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 
										};	 
unsigned char VidOddControlWord[] = { 
										   0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73,  
										   0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 
										};	 
unsigned char AudEvenControlWord[] = { 
										   0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73,  
										   0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 
									    };	
unsigned char AudOddControlWord[] = {
										   0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73,  
										   0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 
										};

typedef struct channel_info_t
{
	NEXUS_FrontendQamAnnex annex; /* To describe Annex A or Annex B format */
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolrate;
    unsigned int  frequency;
    char program_title[MAX_PROGRAM_TITLE_LENGTH];
    int num_audio_streams;
	int num_video_streams;
    unsigned int  video_pid[MAX_VIDEO_STREAMS];
    unsigned int  audio_pid[MAX_AUDIO_STREAMS];
    unsigned int  pcr_pid[MAX_VIDEO_STREAMS];
    NEXUS_VideoCodec video_format[MAX_VIDEO_STREAMS];
    NEXUS_AudioCodec audio_format[MAX_AUDIO_STREAMS];
} channel_info_t;

static channel_info_t dvr_channel_list[MAX_STATIC_CHANNELS] =
{
   {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,429000000,"hockey",1,1,{0x11},
   {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

   {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,435000000,"color beat",1,1,{0x11},
   {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},

   {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,447000000,"hockey match", 3,3,{0x11,0x21,0x41},
   {0x14,0x24,0x44},{0x11,0x21,0x41},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
   {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},

   {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e256,5360537,525000000,"sc_bugs_baywatch",3,3,{0x31,0x11,0x21},
   {0x34,0x14,0x24},{0x31,0x11,0x21},{NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2,NEXUS_VideoCodec_eMpeg2},
   {NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3,NEXUS_AudioCodec_eAc3}},

};

#define MAX_PB		2
#define MAX_REC		2
#define MAX_TUNER	2
static NEXUS_RecpumpHandle recpumpHandle[MAX_REC];
static NEXUS_RecordHandle  recordHandle[MAX_REC];
static NEXUS_PlaypumpHandle playpumpHandle[MAX_PB];
static NEXUS_PlaybackHandle playbackHandle[MAX_PB];
static NEXUS_PidChannelHandle playback_pidchannels[MAX_PB][2];


struct streampath_object
{
  NEXUS_FrontendHandle fe;
  NEXUS_FrontendQamSettings fe_qamsettings;
  NEXUS_FrontendQamStatus fe_qamstatus;
  NEXUS_FrontendCapabilities fe_capabilities;
  NEXUS_FrontendUserParameters fe_params;
  NEXUS_ParserBandSettings parserbandsettings;
  channel_info_t *tuned_channel;
  NEXUS_DisplayHandle display;
  NEXUS_DisplaySettings displaysettings;
  NEXUS_VideoDecoderHandle videodecoder;
  NEXUS_StcChannelHandle stcchannel;
  NEXUS_StcChannelSettings stcsettings;
  NEXUS_VideoWindowHandle window;
  NEXUS_ParserBand parserband;
  NEXUS_VideoDecoderStartSettings videoprogram;
  NEXUS_AudioDecoderHandle audioDecoder;
  NEXUS_AudioDecoderStartSettings audioprogram;
  NEXUS_KeySlotHandle videoKeyHandle[MAX_REC];
  NEXUS_KeySlotHandle audioKeyHandle[MAX_REC];
  NEXUS_KeySlotHandle videopbKeyHandle[MAX_PB];
  NEXUS_KeySlotHandle audiopbKeyHandle[MAX_PB];  
  NEXUS_PidChannelStatus pidStatus;
  NEXUS_SecurityKeySlotSettings keySlotSettings;  
  B_DVR_TimeShiftReq_Handle timeshiftreq;
  B_DVR_SegmentedRecReqHandle segmentedrecreq;
  B_DVR_RecReq_Handle  bg_recreq;
  B_DVR_PbSrvc_Req pb_req;
  int videoPID;
  int audioPID;
  int videoPbPID;
  int audioPbPID;
  int timeshift_started;
  int live_decode_started;
  int playback_started;
  int convert_tsb_permrec;
  int current_channel;
  int bg_recording_started;
};

typedef struct streampath_object streampath_handle;

struct dvrtest_object
{
  B_DVR_Mgr_Handle dvr_mgr_handle;
  NEXUS_PlatformConfiguration platformconfig;
  streampath_handle streampath[2];
  B_ThreadHandle   userinput_thread;
  NEXUS_HdmiOutputSettings hdmiSettings;
  int dvr_max_channels;

};
typedef struct dvrtest_object *dvrtest_handle;

dvrtest_handle g_dvrtest_handle;

NEXUS_Error ConfigureKeySlotFor3DesCA (NEXUS_KeySlotHandle keyHandle, 
											 unsigned char * pkeyEven, 
											 unsigned char * pKeyOdd,
											 dvrtest_cfg_keyslot cfgKeySlot)
{
	NEXUS_SecurityAlgorithmSettings AlgConfig;
	NEXUS_SecurityClearKey key;
	printf("%s enter %d\n",__FUNCTION__);

    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
	AlgConfig.algorithm = NEXUS_SecurityAlgorithm_e3DesAba;

	switch (cfgKeySlot)
		{
			case dvrtest_cfg_ca:
			{
	AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
	AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
				break;
			}
			case dvrtest_cfg_cpd:
			{
				AlgConfig.dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;
				AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
				break;
			}
			default:
				BDBG_MSG((" Fail to configure algorithm "));
		}

	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		printf("\nConfig video CA Algorithm failed \n");
		return 1;
	}

	/* Load AV keys */
	key.keySize = 16; 
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
 	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCa; /* For CACP keyslot , this function needs CA and next CPS */
    memcpy(key.keyData,pkeyEven,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA EVEN key failed \n");
		return 1;
	}

	key.keySize = 16; 
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
 	key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCa; /* For CACP keyslot , this function needs CA and next CPS */
    memcpy(key.keyData,pKeyOdd,key.keySize);
	if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
	{
		printf("\nLoad CA ODD key failed \n");
		return 1;
	}

	return 0;

}

NEXUS_Error initKeySlotFor3DesCA (int streampath_index)
{

	NEXUS_Security_GetDefaultKeySlotSettings(&g_dvrtest_handle->streampath[streampath_index].keySlotSettings);
	g_dvrtest_handle->streampath[streampath_index].keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp;	
	g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index] = 
				                                NEXUS_Security_AllocateKeySlot(&g_dvrtest_handle->streampath[streampath_index].keySlotSettings);
	if (!(g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index] ))
	{
		printf("\nAllocate CACP video keyslot 1  failed \n");
		return;
	}

	g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index] = 
		                                         NEXUS_Security_AllocateKeySlot(&g_dvrtest_handle->streampath[streampath_index].keySlotSettings);

	if (!(g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index] ))
	{
 		printf("\nAllocate CACP audio keyslot 1 failed \n");
		return;
}


	/* CA Config */
	if ( ConfigureKeySlotFor3DesCA (g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index], 
								  	    VidEvenControlWord, 
									    VidOddControlWord,
									    dvrtest_cfg_ca) != 0 )
	{
		printf("\nConfig video CA Algorithm failed for video keyslot 1 \n");
		return 1;
	}


	if ( ConfigureKeySlotFor3DesCA (g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index], 
									    AudEvenControlWord, 
									    AudOddControlWord,
									    dvrtest_cfg_ca) != 0 )
	{
		printf("\nConfig audio CA Algorithm failed for audio keyslot 1 \n");
		return 1;
	}

}


/*
 * Print the channel map loaded into channel list data structure
 * from the channels.txt file.
 */
void dvrtest_channel_mgr_print_map()
{
	int i,j;
    for (i = 0; i < g_dvrtest_handle->dvr_max_channels; i++)
    {
        printf("channel number   ==>  %d\n", i);
        printf("\tannex          ==> %d\n",dvr_channel_list[i].annex);
        printf("\tmodulation     ==> %d\n",dvr_channel_list[i].modulation);
        printf("\tfrequency      == %d\n", dvr_channel_list[i].frequency);
        printf("\tsymbolrate     ==> %d\n", dvr_channel_list[i].symbolrate);
        printf("\tprogram_title  ==> %s\n",dvr_channel_list[i].program_title);
        printf("\t no of audio streams  ==> %d\n",dvr_channel_list[i].num_audio_streams);
        printf("\t no of video streams  ==> %d\n",dvr_channel_list[i].num_video_streams);
        printf("\t video_pids/format ==>");
        for(j=0;j<dvr_channel_list[i].num_video_streams;j++)
        {
         printf("\t 0x%04x/%d",dvr_channel_list[i].video_pid[j],dvr_channel_list[i].video_format[j]);
        }
        printf("\n");
        printf("\taudio_pids ==> " );
        for(j=0;j<dvr_channel_list[i].num_audio_streams;j++)
        {
         printf("\t 0x%04x/%d", dvr_channel_list[i].audio_pid[j],dvr_channel_list[i].audio_format[j]);
        }
        printf("\n");
        printf("\tpcr_pid ==>");
        for(j=0;j<dvr_channel_list[i].num_video_streams;j++)
        {
          printf("\t 0x%04x",dvr_channel_list[i].pcr_pid[j]);
        }
        printf("\n");
	}
}

void dvrtest_streampath_resource_open(int streampath_index)
{
    printf("%s enter path %d \n",__FUNCTION__,streampath_index);
    NEXUS_DisplaySettings displayCfg;
    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(streampath_index, &g_dvrtest_handle->streampath[streampath_index].stcsettings);
    g_dvrtest_handle->streampath[streampath_index].stcsettings.timebase = streampath_index?NEXUS_Timebase_e1:NEXUS_Timebase_e0;

    g_dvrtest_handle->streampath[streampath_index].stcchannel = NEXUS_StcChannel_Open(streampath_index, &g_dvrtest_handle->streampath[streampath_index].stcsettings);
    g_dvrtest_handle->streampath[streampath_index].videoprogram.stcChannel =  g_dvrtest_handle->streampath[streampath_index].stcchannel;
    g_dvrtest_handle->streampath[streampath_index].audioprogram.stcChannel =  g_dvrtest_handle->streampath[streampath_index].stcchannel;


    if(streampath_index)
    {

     NEXUS_Display_GetDefaultSettings(&displayCfg);
        displayCfg.displayType = NEXUS_DisplayType_eAuto;
        displayCfg.format = NEXUS_VideoFormat_eNtsc;
     g_dvrtest_handle->streampath[streampath_index].display = NEXUS_Display_Open(streampath_index, &displayCfg);
     NEXUS_Display_GetSettings(g_dvrtest_handle->streampath[streampath_index].display,&g_dvrtest_handle->streampath[streampath_index].displaysettings);
     NEXUS_Display_AddOutput(g_dvrtest_handle->streampath[streampath_index].display,
                         NEXUS_CompositeOutput_GetConnector(g_dvrtest_handle->platformconfig.outputs.composite[0]));
    }
    else
    {


        NEXUS_Display_GetDefaultSettings(&displayCfg);
        displayCfg.displayType = NEXUS_DisplayType_eAuto;
        displayCfg.format = NEXUS_VideoFormat_e1080i;
        g_dvrtest_handle->streampath[streampath_index].display = NEXUS_Display_Open(streampath_index, &displayCfg);
        NEXUS_Display_GetSettings(g_dvrtest_handle->streampath[streampath_index].display,&g_dvrtest_handle->streampath[streampath_index].displaysettings);
        NEXUS_Display_AddOutput(g_dvrtest_handle->streampath[streampath_index].display, NEXUS_ComponentOutput_GetConnector(g_dvrtest_handle->platformconfig.outputs.component[0]));

    }

    g_dvrtest_handle->streampath[streampath_index].window = NEXUS_VideoWindow_Open(g_dvrtest_handle->streampath[streampath_index].display,0);

    BDBG_ASSERT(g_dvrtest_handle->streampath[streampath_index].window );
    if(streampath_index)
    {
       NEXUS_VideoWindowSettings videoWindowSettings;
       NEXUS_VideoWindow_GetSettings(g_dvrtest_handle->streampath[streampath_index].window, &videoWindowSettings);
       videoWindowSettings.position.x = 0;
       videoWindowSettings.position.y = 0;
       videoWindowSettings.position.width = 720;
       videoWindowSettings.position.height = 480;
       videoWindowSettings.zorder = 0;
       NEXUS_VideoWindow_SetSettings(g_dvrtest_handle->streampath[streampath_index].window , &videoWindowSettings);
    }
    else
    {
       NEXUS_VideoWindowSettings videoWindowSettings;
       NEXUS_VideoWindow_GetSettings(g_dvrtest_handle->streampath[streampath_index].window, &videoWindowSettings);
       videoWindowSettings.position.x = 0;
       videoWindowSettings.position.y = 0;
       videoWindowSettings.position.width = 1920;
       videoWindowSettings.position.height = 1080;
       videoWindowSettings.zorder = 0;
       NEXUS_VideoWindow_SetSettings(g_dvrtest_handle->streampath[streampath_index].window , &videoWindowSettings);
    }

    /* bring up decoder and connect to display */
    g_dvrtest_handle->streampath[streampath_index].videodecoder = NEXUS_VideoDecoder_Open(streampath_index, NULL); /* take default capabilities */

    NEXUS_VideoWindow_AddInput(g_dvrtest_handle->streampath[streampath_index].window, NEXUS_VideoDecoder_GetConnector(g_dvrtest_handle->streampath[streampath_index].videodecoder));
    g_dvrtest_handle->streampath[streampath_index].audioDecoder = NEXUS_AudioDecoder_Open(streampath_index, NULL);

    /* Bring up audio decoders and outputs */
    if(streampath_index == 0 )
    {
        NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(g_dvrtest_handle->platformconfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(g_dvrtest_handle->streampath[streampath_index].audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(g_dvrtest_handle->platformconfig.outputs.audioDacs[1]),
        NEXUS_AudioDecoder_GetConnector(g_dvrtest_handle->streampath[streampath_index].audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }

    printf("%s exit path %d \n",__FUNCTION__,streampath_index);
   return;
}
void dvrtest_streampath_resource_close(int streampath_index)
{
    printf("%s enter path %d \n",__FUNCTION__,streampath_index);
    NEXUS_Display_RemoveAllOutputs(g_dvrtest_handle->streampath[streampath_index].display);
    NEXUS_VideoWindow_RemoveAllInputs(g_dvrtest_handle->streampath[streampath_index].window);
    if(streampath_index)
     {
        NEXUS_VideoOutput_Shutdown(NEXUS_CompositeOutput_GetConnector(g_dvrtest_handle->platformconfig.outputs.composite[0]));
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_dvrtest_handle->platformconfig.outputs.audioDacs[0]));
     }
    else
    {
      NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_dvrtest_handle->platformconfig.outputs.audioDacs[1]));
      NEXUS_VideoOutput_Shutdown(NEXUS_ComponentOutput_GetConnector(g_dvrtest_handle->platformconfig.outputs.component[0]));
    }
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrtest_handle->streampath[streampath_index].videodecoder));
    NEXUS_VideoWindow_Close(g_dvrtest_handle->streampath[streampath_index].window);
    NEXUS_VideoDecoder_Close(g_dvrtest_handle->streampath[streampath_index].videodecoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_dvrtest_handle->streampath[streampath_index].audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(g_dvrtest_handle->streampath[streampath_index].audioDecoder);
    NEXUS_Display_Close(g_dvrtest_handle->streampath[streampath_index].display);
    printf("%s exit path %d \n",__FUNCTION__,streampath_index);
    return;
}

void dvrtest_channel_tune(int streampath_index)
{
    NEXUS_Error rc;
    int current_channel = g_dvrtest_handle->streampath[streampath_index].current_channel;
    printf("%s enter %d \n",__FUNCTION__,streampath_index);
    NEXUS_Frontend_GetDefaultQamSettings(&g_dvrtest_handle->streampath[streampath_index].fe_qamsettings);
    g_dvrtest_handle->streampath[streampath_index].fe_qamsettings.frequency =  dvr_channel_list[current_channel].frequency;
    g_dvrtest_handle->streampath[streampath_index].fe_qamsettings.mode = dvr_channel_list[current_channel].modulation;
    g_dvrtest_handle->streampath[streampath_index].fe_qamsettings.annex = dvr_channel_list[current_channel].annex;
    rc = NEXUS_Frontend_TuneQam(g_dvrtest_handle->streampath[streampath_index].fe, &g_dvrtest_handle->streampath[streampath_index].fe_qamsettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(2000);
    rc = NEXUS_Frontend_GetQamStatus(g_dvrtest_handle->streampath[streampath_index].fe, &g_dvrtest_handle->streampath[streampath_index].fe_qamsettings);
    BDBG_ASSERT(!rc);
    printf("receiver lock = %d\n",g_dvrtest_handle->streampath[streampath_index].fe_qamstatus.receiverLock);
    printf("Symbol rate = %d\n", g_dvrtest_handle->streampath[streampath_index].fe_qamstatus.symbolRate);

     /* Get the input band for this tuner */
    NEXUS_Frontend_GetUserParameters(g_dvrtest_handle->streampath[streampath_index].fe, &g_dvrtest_handle->streampath[streampath_index].fe_params);
    printf("Input band=%d \n",g_dvrtest_handle->streampath[streampath_index].fe_params.param1);

    g_dvrtest_handle->streampath[streampath_index].parserband = (NEXUS_ParserBand)streampath_index;

    NEXUS_ParserBand_GetSettings(g_dvrtest_handle->streampath[streampath_index].parserband, &g_dvrtest_handle->streampath[streampath_index].parserbandsettings);
    g_dvrtest_handle->streampath[streampath_index].parserbandsettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrtest_handle->streampath[streampath_index].fe_params.param1;
    g_dvrtest_handle->streampath[streampath_index].parserbandsettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    g_dvrtest_handle->streampath[streampath_index].parserbandsettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_dvrtest_handle->streampath[streampath_index].parserband, &g_dvrtest_handle->streampath[streampath_index].parserbandsettings);


    printf("%s exit %d \n",__FUNCTION__,streampath_index);
    return;
}


void dvrtest_live_decode_start(int streampath_index)
{
    printf("%s enter %d\n",__FUNCTION__,streampath_index);
    int current_channel = g_dvrtest_handle->streampath[streampath_index].current_channel;
	int vpid,apid;

	NEXUS_TimebaseSettings timebaseSettings;
	NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrtest_handle->streampath[streampath_index].videoprogram);
    g_dvrtest_handle->streampath[streampath_index].videoprogram.codec = dvr_channel_list[current_channel].video_format[0];
    g_dvrtest_handle->streampath[streampath_index].videoprogram.pidChannel = videoPidChannel = NEXUS_PidChannel_Open(g_dvrtest_handle->streampath[streampath_index].parserband,
                                                                                                   dvr_channel_list[current_channel].video_pid[0], NULL);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrtest_handle->streampath[streampath_index].audioprogram);
    g_dvrtest_handle->streampath[streampath_index].audioprogram.codec = dvr_channel_list[current_channel].audio_format[0];
    g_dvrtest_handle->streampath[streampath_index].audioprogram.pidChannel = audioPidChannel = NEXUS_PidChannel_Open(g_dvrtest_handle->streampath[streampath_index].parserband,
                                                                                                    dvr_channel_list[current_channel].audio_pid[0], NULL);
#if CA_3DES_CPS_ENCRYPT
	/* Add video PID channel to keyslot */
	initKeySlotFor3DesCA(streampath_index);
	NEXUS_PidChannel_GetStatus ( videoPidChannel, &g_dvrtest_handle->streampath[streampath_index].pidStatus);
	vpid = g_dvrtest_handle->streampath[streampath_index].videoPID = 	g_dvrtest_handle->streampath[streampath_index].pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index], vpid)!= 0 )
	{
		printf("\nConfig Video PIDPointerTable failed \n");
		return 1;
	}

	NEXUS_PidChannel_GetStatus (audioPidChannel, &g_dvrtest_handle->streampath[streampath_index].pidStatus);
	apid =g_dvrtest_handle->streampath[streampath_index].audioPID= g_dvrtest_handle->streampath[streampath_index].pidStatus.pidChannelIndex;
	if ( NEXUS_Security_AddPidChannelToKeySlot(g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index], apid) != 0 )
	{
			printf("\nConfig Audio PIDPointerTable failed \n");
			return 1;
	}
#endif

    NEXUS_StcChannel_GetSettings(g_dvrtest_handle->streampath[streampath_index].stcchannel,&g_dvrtest_handle->streampath[streampath_index].stcsettings);

    NEXUS_Timebase_GetSettings(g_dvrtest_handle->streampath[streampath_index].stcsettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrtest_handle->streampath[streampath_index].videoprogram.pidChannel;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrtest_handle->streampath[streampath_index].stcsettings.timebase, &timebaseSettings);

    g_dvrtest_handle->streampath[streampath_index].stcsettings.autoConfigTimebase = true;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.modeSettings.pcr.offsetThreshold = 8;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.modeSettings.pcr.maxPcrError = 255;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.modeSettings.pcr.disableTimestampCorrection = false;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.modeSettings.pcr.disableJitterAdjustment = false;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.modeSettings.pcr.pidChannel = g_dvrtest_handle->streampath[streampath_index].videoprogram.pidChannel;
    g_dvrtest_handle->streampath[streampath_index].stcsettings.mode = NEXUS_StcChannelMode_ePcr; /* live */

    NEXUS_StcChannel_SetSettings(g_dvrtest_handle->streampath[streampath_index].stcchannel,&g_dvrtest_handle->streampath[streampath_index].stcsettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrtest_handle->streampath[streampath_index].videodecoder, &g_dvrtest_handle->streampath[streampath_index].videoprogram);
    NEXUS_AudioDecoder_Start(g_dvrtest_handle->streampath[streampath_index].audioDecoder, &g_dvrtest_handle->streampath[streampath_index].audioprogram);

    printf("%s exit %d \n",__FUNCTION__,streampath_index);
    return;
}


void dvrtest_live_decode_stop(int streampath_index)
{
  NEXUS_VideoDecoderSettings Settings;
  printf("%s enter %d \n",__FUNCTION__,streampath_index);

  NEXUS_VideoDecoder_GetSettings(g_dvrtest_handle->streampath[streampath_index].videodecoder,&Settings);
  Settings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
  NEXUS_VideoDecoder_SetSettings(g_dvrtest_handle->streampath[streampath_index].videodecoder,&Settings);

#if	CA_3DES_CPS_ENCRYPT
  NEXUS_Security_RemovePidChannelFromKeySlot(g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index],
												   g_dvrtest_handle->streampath[streampath_index].videoPID);
  NEXUS_Security_RemovePidChannelFromKeySlot(g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index],
												   g_dvrtest_handle->streampath[streampath_index].audioPID);
  NEXUS_Security_FreeKeySlot(g_dvrtest_handle->streampath[streampath_index].videoKeyHandle[streampath_index]);
  NEXUS_Security_FreeKeySlot(g_dvrtest_handle->streampath[streampath_index].audioKeyHandle[streampath_index]);
#endif

  NEXUS_AudioDecoder_Stop(g_dvrtest_handle->streampath[streampath_index].audioDecoder);
  NEXUS_VideoDecoder_Stop(g_dvrtest_handle->streampath[streampath_index].videodecoder);
  NEXUS_PidChannel_Close( g_dvrtest_handle->streampath[streampath_index].videoprogram.pidChannel);
  NEXUS_PidChannel_Close( g_dvrtest_handle->streampath[streampath_index].audioprogram.pidChannel);
  printf("%s exit %d \n",__FUNCTION__,streampath_index);
  return;
}


void dvrtest_pb_decode_start(int streampath_index)
{
	int vpid,apid;

    printf("%s enter %d\n",__FUNCTION__,streampath_index);
    int current_channel = g_dvrtest_handle->streampath[streampath_index].current_channel;
	NEXUS_PlaybackPidChannelSettings playback_pidsettings;
	NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlaybackSettings playbackSettings;
	NEXUS_SecurityKeySlotSettings keySlotSettings;

    BDBG_MSG(("\n setting audio video pids"));

    NEXUS_Playback_GetSettings(playbackHandle[streampath_index], &playbackSettings);
	playbackSettings.playpump = playpumpHandle[streampath_index];
    NEXUS_Playback_SetSettings(playbackHandle[streampath_index], &playbackSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playback_pidsettings);
    playback_pidsettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playback_pidsettings.pidTypeSettings.video.codec = dvr_channel_list[current_channel].video_format[0];
    playback_pidsettings.pidTypeSettings.video.index = true;
    playback_pidsettings.pidTypeSettings.video.decoder = g_dvrtest_handle->streampath[streampath_index].videodecoder;
    playback_pidchannels[streampath_index][B_DVR_DEFAULT_VPID_CH_INDEX] =
    NEXUS_Playback_OpenPidChannel(playbackHandle[streampath_index], dvr_channel_list[current_channel].video_pid[0], &playback_pidsettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playback_pidsettings);
    playback_pidsettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playback_pidsettings.pidTypeSettings.audio.primary = g_dvrtest_handle->streampath[streampath_index].audioDecoder;
    playback_pidchannels[streampath_index][B_DVR_DEFAULT_APID_CH_INDEX] =
    NEXUS_Playback_OpenPidChannel(playbackHandle[streampath_index], dvr_channel_list[current_channel].audio_pid[0], &playback_pidsettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrtest_handle->streampath[streampath_index].videoprogram);
    g_dvrtest_handle->streampath[streampath_index].videoprogram.codec = dvr_channel_list[current_channel].video_format[0];
    g_dvrtest_handle->streampath[streampath_index].videoprogram.pidChannel = playback_pidchannels[streampath_index][B_DVR_DEFAULT_VPID_CH_INDEX];

    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrtest_handle->streampath[streampath_index].audioprogram);
    g_dvrtest_handle->streampath[streampath_index].audioprogram.codec = dvr_channel_list[current_channel].audio_format[0];
    g_dvrtest_handle->streampath[streampath_index].audioprogram.pidChannel = playback_pidchannels[streampath_index][B_DVR_DEFAULT_APID_CH_INDEX];

#if	CA_3DES_CPS_ENCRYPT
	NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
	keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
	g_dvrtest_handle->streampath[streampath_index].videopbKeyHandle[streampath_index] = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
	g_dvrtest_handle->streampath[streampath_index].audiopbKeyHandle[streampath_index] = NEXUS_Security_AllocateKeySlot(&keySlotSettings);

	ConfigureKeySlotFor3DesCA(g_dvrtest_handle->streampath[streampath_index].videopbKeyHandle[streampath_index],
		                          VidEvenControlWord, VidOddControlWord,dvrtest_cfg_cpd);
	ConfigureKeySlotFor3DesCA(g_dvrtest_handle->streampath[streampath_index].audiopbKeyHandle[streampath_index],
								  AudEvenControlWord, AudOddControlWord,dvrtest_cfg_cpd);

	NEXUS_PidChannel_GetStatus(playback_pidchannels[streampath_index][B_DVR_DEFAULT_VPID_CH_INDEX],
									&g_dvrtest_handle->streampath[streampath_index].pidStatus);

	vpid = g_dvrtest_handle->streampath[streampath_index].videoPbPID = g_dvrtest_handle->streampath[streampath_index].pidStatus.pidChannelIndex;
	NEXUS_Security_AddPidChannelToKeySlot(g_dvrtest_handle->streampath[streampath_index].videopbKeyHandle[streampath_index], vpid);

	
	NEXUS_PidChannel_GetStatus(playback_pidchannels[streampath_index][B_DVR_DEFAULT_APID_CH_INDEX],
									&g_dvrtest_handle->streampath[streampath_index].pidStatus);

	apid = g_dvrtest_handle->streampath[streampath_index].audioPbPID = g_dvrtest_handle->streampath[streampath_index].pidStatus.pidChannelIndex;
	NEXUS_Security_AddPidChannelToKeySlot(g_dvrtest_handle->streampath[streampath_index].audiopbKeyHandle[streampath_index], apid);
#endif

	/*STC*/
    NEXUS_Playback_GetSettings(playbackHandle[streampath_index], &playbackSettings);
	playbackSettings.stcChannel = g_dvrtest_handle->streampath[streampath_index].stcchannel;
	playbackSettings.stcTrick = true;
    NEXUS_Playback_SetSettings(playbackHandle[streampath_index], &playbackSettings);

    NEXUS_StcChannel_GetSettings(g_dvrtest_handle->streampath[streampath_index].stcchannel,&g_dvrtest_handle->streampath[streampath_index].stcsettings);

    NEXUS_Timebase_GetSettings(g_dvrtest_handle->streampath[streampath_index].stcsettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    timebaseSettings.sourceSettings.pcr.pidChannel = NULL;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrtest_handle->streampath[streampath_index].stcsettings.timebase, &timebaseSettings);

    g_dvrtest_handle->streampath[streampath_index].stcsettings.mode = NEXUS_StcChannelMode_eAuto;
    NEXUS_StcChannel_SetSettings(g_dvrtest_handle->streampath[streampath_index].stcchannel,&g_dvrtest_handle->streampath[streampath_index].stcsettings);


    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrtest_handle->streampath[streampath_index].videodecoder, &g_dvrtest_handle->streampath[streampath_index].videoprogram);
    NEXUS_AudioDecoder_Start(g_dvrtest_handle->streampath[streampath_index].audioDecoder, &g_dvrtest_handle->streampath[streampath_index].audioprogram);

    printf("%s exit %d \n",__FUNCTION__,streampath_index);
    return;
}

void dvrtest_pb_decode_stop(int streampath_index)
{
  NEXUS_VideoDecoderSettings Settings;

  printf("%s enter %d \n",__FUNCTION__,streampath_index);
  NEXUS_VideoDecoder_GetSettings(g_dvrtest_handle->streampath[streampath_index].videodecoder,&Settings);
  Settings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
  NEXUS_VideoDecoder_SetSettings(g_dvrtest_handle->streampath[streampath_index].videodecoder,&Settings);
#if	CA_3DES_CPS_ENCRYPT
 NEXUS_Security_RemovePidChannelFromKeySlot(g_dvrtest_handle->streampath[streampath_index].videopbKeyHandle[streampath_index],
												  g_dvrtest_handle->streampath[streampath_index].videoPbPID);
 NEXUS_Security_RemovePidChannelFromKeySlot(g_dvrtest_handle->streampath[streampath_index].audiopbKeyHandle[streampath_index],
												   g_dvrtest_handle->streampath[streampath_index].audioPbPID);
 NEXUS_Security_FreeKeySlot(g_dvrtest_handle->streampath[streampath_index].videopbKeyHandle[streampath_index]);
 NEXUS_Security_FreeKeySlot(g_dvrtest_handle->streampath[streampath_index].audiopbKeyHandle[streampath_index]);
#endif
  NEXUS_AudioDecoder_Stop(g_dvrtest_handle->streampath[streampath_index].audioDecoder);
  NEXUS_VideoDecoder_Stop(g_dvrtest_handle->streampath[streampath_index].videodecoder);
  NEXUS_PidChannel_Close( playback_pidchannels[streampath_index][B_DVR_DEFAULT_VPID_CH_INDEX]);
  NEXUS_PidChannel_Close( playback_pidchannels[streampath_index][B_DVR_DEFAULT_APID_CH_INDEX]);

  printf("%s exit %d \n",__FUNCTION__,streampath_index);
  return;
}


void dvrtest_srvc_status_callback(int path_index,B_DVR_Srvc_Event srvc_event,B_DVR_Srvc_Type srvc_type)
{
  char *service_type[3] = {"TimeShift_Srvc", "Recording_Srvc", "Playback_Srvc"};
  char *event_type[10] = {"START_OF_PLAYBACK", "END_OF_PLAYBACK","START_OF_RECORDING","END_OF_RECORDING","TSB_PROGRAM_ADDED"
                        "RECORD_PROGRAM_ADDED","TSB_PROGRAM_DELETED","NO_STORAGE_SPACE"};
   switch (srvc_event)
  {
   case START_OF_PLAYBACK:
    {
     printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
     /*TODO -> App to take the required action*/
      break;
    }
   case END_OF_PLAYBACK:
   {
      printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
      /*TODO -> App to take the required action*/
      break;
   }
   case START_OF_RECORDING:
    {
     printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
     /*TODO -> App to take the required action*/
     break;
    }
   case END_OF_RECORDING:
    {
      printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
      /*TODO -> App to take the required action*/
      break;
    }
   case TSB_PROGRAM_ADDED:
    {
      printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
      /*TODO -> App to take the required action*/
      break;
    }
   case RECORD_PROGRAM_ADDED:
    {
      printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
      /*TODO -> App to take the required action*/
      break;
    }
   case TSB_PROGRAM_DELETED:
    {
       printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
       /*TODO -> App to take the required action*/
       break;
    }
   case NO_STORAGE_SPACE:
    {
        printf("stream path = %d, event= %s service type = %s\n",path_index,event_type[srvc_event],service_type[srvc_type]);
        /*TODO -> App to take the required action*/
        break;
    }
   default:
       printf("dvrtest_srvc_status_callback -> Invalid event \n");
       break;
}
  return;
}

int main(void)
{
    int streampath_index, frontend_index, pathindex, operation;
    NEXUS_Error nexus_err;
	int i;
    B_DVR_Mgr_Settings dvr_settings;
    b_dvr_lib_err dvr_err = B_DVR_SUCCESS;
    g_dvrtest_handle = BKNI_Malloc(sizeof(struct dvrtest_object));
    g_dvrtest_handle->dvr_max_channels = MAX_STATIC_CHANNELS;
    dvrtest_channel_mgr_print_map();
    g_dvrtest_handle->streampath[0].current_channel = 0;
    if(NEXUS_NUM_VIDEO_DECODERS > 1)
    g_dvrtest_handle->streampath[1].current_channel= 1;

	B_Os_Init();

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrtest_handle->platformconfig);
    /* Find the QAM frontend */


        for (frontend_index=0,streampath_index=0; frontend_index < NEXUS_MAX_FRONTENDS; frontend_index++ )
        {
          g_dvrtest_handle->streampath[streampath_index].fe = g_dvrtest_handle->platformconfig.frontend[frontend_index];
          if (g_dvrtest_handle->streampath[streampath_index].fe)
          {
            NEXUS_Frontend_GetCapabilities(g_dvrtest_handle->streampath[streampath_index].fe, &g_dvrtest_handle->streampath[streampath_index].fe_capabilities);
             if (g_dvrtest_handle->streampath[streampath_index].fe_capabilities.qam)
             {
                  nexus_err = NEXUS_Frontend_GetQamStatus(g_dvrtest_handle->streampath[streampath_index].fe, &g_dvrtest_handle->streampath[streampath_index].fe_qamstatus);
                  printf("NEXUS_Frontend_GetQamStatus returned = %d \n", nexus_err);
                  BDBG_ASSERT(!nexus_err);
                 dvrtest_channel_tune(streampath_index);
                 g_dvrtest_handle->streampath[streampath_index].live_decode_started = 0;
                 g_dvrtest_handle->streampath[streampath_index].timeshift_started = 0;
                 g_dvrtest_handle->streampath[streampath_index].live_decode_started = 0;
                 g_dvrtest_handle->streampath[streampath_index].playback_started = 0;
                 g_dvrtest_handle->streampath[streampath_index].bg_recording_started = 0;
                 g_dvrtest_handle->streampath[streampath_index].convert_tsb_permrec = 0;
                 streampath_index++;

                 if(streampath_index == NEXUS_NUM_VIDEO_DECODERS)
                 break;
             }
          }
        }



   for(streampath_index=0;streampath_index < NEXUS_NUM_VIDEO_DECODERS; streampath_index++)
   {
     dvrtest_streampath_resource_open(streampath_index);
     dvrtest_live_decode_start(streampath_index);
     g_dvrtest_handle->streampath[streampath_index].live_decode_started = 1;

   }

	BKNI_Memset(&dvr_settings, 0, sizeof(dvr_settings));
  /* open playback/playpump/recpump/record handles*/

	for (i=0;i<MAX_PB;i++)
	{
    	playpumpHandle[i] = NEXUS_Playpump_Open(i, NULL);
	    if (!playpumpHandle[i])
		{
			BDBG_ERR(("NEXUS Error at %d, Exiting...\n", __LINE__));
		}
	    playbackHandle[i] = NEXUS_Playback_Create();
	    if (!playbackHandle[i])
		{
			BDBG_ERR(("NEXUS Error  at %d, Exiting...\n", __LINE__));
	    }
	}

	for (i=0;i<MAX_REC;i++)
	{
	    recpumpHandle[i] = NEXUS_Recpump_Open(i, NULL);
	    if (!recpumpHandle[i])
		{
			BDBG_ERR(("NEXUS Error at %d, Exiting...\n", __LINE__));
		}
	    /* Setup Nexus Playback, For Playback Modes (e.g. HTTP) or PVR*/
	    recordHandle[i] = NEXUS_Record_Create();
	    if (!recordHandle[i])
		{
			BDBG_ERR(("NEXUS Error  at %d, Exiting...\n", __LINE__));
	    }
	}

   dvr_settings.bseg_rec = true;
   dvr_settings.btimeshift = true;
   sprintf(dvr_settings.dir_media, "%s","/data/videos/media/");
   sprintf(dvr_settings.dir_mediainfo, "%s","/data/videos/mediainfo/");
   sprintf(dvr_settings.dir_navigation, "%s","/data/videos/navigation/");
   printf("media dir %s\n",dvr_settings.dir_media);
   printf("mediainfo dir %s\n",dvr_settings.dir_mediainfo);
   printf("navigation dir %s\n",dvr_settings.dir_navigation);
   dvr_settings.max_pb = MAX_PB;
   dvr_settings.max_rec = MAX_REC;
   dvr_settings.max_tuners = MAX_TUNER;

   printf("Opening DVR manager interface \n");

   g_dvrtest_handle->dvr_mgr_handle = B_DVR_Mgr_Open(&dvr_settings);

   if(!g_dvrtest_handle->dvr_mgr_handle)
   {
       printf("Error in opening the dvr manager\n");
   }



   while(1)
  {

   printf("\n\n ------------------------------------------------------"
          "\n                   DVR Operations                     "
          "\n\n------------------------------------------------------");
   printf("\n 0 - Quit");
   printf("\n 1 -  Channel change ");
   printf("\n 2 -  Start time shifting ");
   printf("\n 3 -  Stop time shifting ");
   printf("\n 4 -  Start timeshift permanent segmented recording ");
   printf("\n 5 -  Stop timeshift permanent segmented recording ");
   printf("\n 6 -  Start a playback ");
   printf("\n 7  - Stop a playback ");
   printf("\n 8 -  Switch to timeshift playback ");
   printf("\n 9 -  Switch to live from tsb playback ");
   printf("\n 10 - Fast Forward in timeshift/playback ");
   printf("\n 11 - Fast Rewind in timeshift/playback ");
   printf("\n 12 - Forward slow motion in timeshift/playback ");
   printf("\n 13 - Reverse slow motion in timeshift/playback ");
   printf("\n 14 - Forward frame advance in timeshift/playback ");
   printf("\n 15 - Reverse frame advance in timeshift/playback ");
   printf("\n 16 - Forward skip 5 secs in timeshift/playback ");
   printf("\n 17 - Reverse skip 5 secs in timeshift/playback ");
   printf("\n 18 - Stats from timeshift recording ");
   printf("\n 19 - Pause timeshift/playback ");
   printf("\n 20 - Play timeshift/playback ");
   printf("\n 21 - Stop live decode ");
   printf("\n 22 - Start background recording ");
   printf("\n 23 - Stop background recording ");
   printf("\n 24 - Delete a recording");
   printf("\n 25 - Move a recording");
   printf("\n 26 - print the recording list");
   printf("\n Enter the DVR operation number - ");

   fflush(stdin);
   operation=0;
   scanf("%d", &operation);
   printf("\n operation = %d",operation);
   if(operation > 0 && operation <= 23)
   {
       printf("\n Enter the stream path index - \n");
       fflush(stdin);
       pathindex=0;
 	  scanf("%d",&pathindex);
       printf("\n path %d\n",pathindex);
   }

   if (!operation)
   {
       break;
   }
   switch(operation)
   {

     case 1:
         {
           int dir=0;
           printf("Channel Up-1 Down - 0 \n");
           fflush(stdin);
           scanf("%d",&dir);
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
              printf("\n Invalid operation! Stop the timeshift srvc on this channel");
              goto case_1_err;
           }
           if(g_dvrtest_handle->streampath[pathindex].playback_started)
           {
              printf("\n Invalid operation! Stop the playback srvc on this channel");
              goto case_1_err;
           }
           if(g_dvrtest_handle->streampath[pathindex].live_decode_started)
           {
           dvrtest_live_decode_stop(pathindex);
           }
           if (dir)
           {
             g_dvrtest_handle->streampath[pathindex].current_channel++;
             g_dvrtest_handle->streampath[pathindex].current_channel = (g_dvrtest_handle->streampath[pathindex].current_channel % g_dvrtest_handle->dvr_max_channels);
             printf("channel number : %d", g_dvrtest_handle->streampath[pathindex].current_channel);

           }
           else
           {
             g_dvrtest_handle->streampath[pathindex].current_channel--;
             g_dvrtest_handle->streampath[pathindex].current_channel = (g_dvrtest_handle->streampath[pathindex].current_channel % g_dvrtest_handle->dvr_max_channels);
             printf("channel number : %d", g_dvrtest_handle->streampath[pathindex].current_channel);
           }
           dvrtest_channel_tune(pathindex);
           dvrtest_live_decode_start(pathindex);
           g_dvrtest_handle->streampath[pathindex].live_decode_started =1;
         }
         case_1_err:
          break;
   case 2:
         {
          if(g_dvrtest_handle->streampath[pathindex].live_decode_started)
          {
          int i;
           int current_channel = g_dvrtest_handle->streampath[pathindex].current_channel;
          dvr_err = B_DVR_TimeShift_Srvc_Open(g_dvrtest_handle->dvr_mgr_handle,pathindex);
          if(dvr_err!=B_DVR_SUCCESS)
          {
           printf("Error in opening timeshift service %d\n",pathindex);
           goto case_2_err;
          }
          g_dvrtest_handle->streampath[pathindex].timeshiftreq = BKNI_Malloc(sizeof(struct B_DVR_TimeShiftReq_Object));
          BKNI_Memset( g_dvrtest_handle->streampath[pathindex].timeshiftreq,0,sizeof(struct B_DVR_TimeShiftReq_Object));

#if CA_3DES_CPS_ENCRYPT
		  g_dvrtest_handle->streampath[pathindex].timeshiftreq->bencryptedrecording = true;
#else
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->bencryptedrecording = false;
#endif
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->ballpassrecording = false;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->bpacing = false;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->bsegmented = true;
          printf("\n **** Enter the temporary timeshift recording name *****\n");
          printf("\n This should be always unique irrespective of how many channel changes happen "
                 "\n or how many times the STB is rebootted  i.e if a permanent record is created using this "
                 "\n temporary timeshift recording name, all the segments of this perm rec will have a prefix "
                 "\n of temp timeshift record name \n");
          fflush(stdin);
          scanf("%s",g_dvrtest_handle->streampath[pathindex].timeshiftreq->channelname);

          g_dvrtest_handle->streampath[pathindex].timeshiftreq->parserband = g_dvrtest_handle->streampath[pathindex].parserband;
		  g_dvrtest_handle->streampath[pathindex].timeshiftreq->transporttype = NEXUS_TransportType_eTs;

          g_dvrtest_handle->streampath[pathindex].timeshiftreq->active_vpid_index = 0;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[0].pid = dvr_channel_list[current_channel].video_pid[0];
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[0].pidtype = NEXUS_PidType_eVideo;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[0].codec.vcodec = dvr_channel_list[current_channel].video_format[0];
          g_dvrtest_handle->streampath[pathindex].timeshiftreq->active_apid_index = 1;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[1].pid = dvr_channel_list[current_channel].audio_pid[0];
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[1].pidtype = NEXUS_PidType_eAudio;
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[1].codec.acodec = dvr_channel_list[current_channel].audio_format[0];
           g_dvrtest_handle->streampath[pathindex].timeshiftreq->no_of_pids = 2;
           for(i=0;i<g_dvrtest_handle->streampath[pathindex].timeshiftreq->no_of_pids;i++)
          {
             printf("no:%d pid no: %d pidtype %d codec %d \n",i,g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[i].pid,
             g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[i].pidtype,g_dvrtest_handle->streampath[pathindex].timeshiftreq->pids[i].codec.vcodec);
          }

		  g_dvrtest_handle->streampath[pathindex].timeshiftreq->RegisteredCallback = dvrtest_srvc_status_callback;


		 g_dvrtest_handle->streampath[pathindex].timeshiftreq->playback = playbackHandle[pathindex];
		 g_dvrtest_handle->streampath[pathindex].timeshiftreq->playpump = playpumpHandle[pathindex];
		 g_dvrtest_handle->streampath[pathindex].timeshiftreq->record = recordHandle[pathindex];
		 g_dvrtest_handle->streampath[pathindex].timeshiftreq->recpump = recpumpHandle[pathindex];

#if CA_3DES_CPS_ENCRYPT
		g_dvrtest_handle->streampath[pathindex].timeshiftreq->videoKeyHandle[pathindex] = 
										g_dvrtest_handle->streampath[pathindex].videoKeyHandle[pathindex];
		g_dvrtest_handle->streampath[pathindex].timeshiftreq->audioKeyHandle[pathindex] = 
										g_dvrtest_handle->streampath[pathindex].audioKeyHandle[pathindex];
#endif	
          dvr_err = B_DVR_TimeShift_Srvc_Start(g_dvrtest_handle->dvr_mgr_handle,pathindex,g_dvrtest_handle->streampath[pathindex].timeshiftreq);
          if(dvr_err!=B_DVR_SUCCESS)
          {
           printf("Error in starting the timeshifting service %d \n",pathindex);
          }
          else
          {
            printf("setting the timeshift started flag %d\n",pathindex);
            g_dvrtest_handle->streampath[pathindex].timeshift_started = 1;
          }
         }
          else
          {
           printf("\n live decode not started ");
          }
         }
         case_2_err:
          break;

   case 3:
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
           dvr_err = B_DVR_TimeShift_Srvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
           if(dvr_err != B_DVR_SUCCESS)
           {
            printf("Error in stopping the timeshift service %d\n",pathindex);
           }
           g_dvrtest_handle->streampath[pathindex].timeshift_started = 0;
           g_dvrtest_handle->streampath[pathindex].live_decode_started =1;
           BKNI_Free(g_dvrtest_handle->streampath[pathindex].timeshiftreq);
           dvr_err = B_DVR_TimeShift_Srvc_Close(g_dvrtest_handle->dvr_mgr_handle,pathindex);
           if(dvr_err != B_DVR_SUCCESS)
           {
            printf("Error in closing the timeshift service %d\n",pathindex);
           }
           }
           else
          {
            printf("\n timeshift srvc not started");
          }


           break;
   case 4:
       {
          if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
          {
              g_dvrtest_handle->streampath[pathindex].segmentedrecreq = BKNI_Malloc(sizeof(struct B_DVR_SegmentedRecReq_Object));
              BKNI_Memset( g_dvrtest_handle->streampath[pathindex].timeshiftreq,0,sizeof(struct B_DVR_SegmentedRecReq_Object));
              printf("\n Enter the program name =>  \n");
              fflush(stdin);
              scanf("%s",g_dvrtest_handle->streampath[pathindex].segmentedrecreq->program_name);
              printf("\n Enter the start time (msecs)of the permanent recording ==> \n");
              fflush(stdin);
              scanf("%d",&g_dvrtest_handle->streampath[pathindex].segmentedrecreq->rec_start_time);
              printf("\n Enter the end time ==>\n");
              fflush(stdin);
              scanf("%d",&g_dvrtest_handle->streampath[pathindex].segmentedrecreq->rec_end_time);
              printf("\n input done");

          dvr_err = B_DVR_Tsb_RecSrcv_Start(g_dvrtest_handle->dvr_mgr_handle,
                                                pathindex,g_dvrtest_handle->streampath[pathindex].segmentedrecreq);
          if(dvr_err != B_DVR_SUCCESS)
          {
            printf("Error in starting the timeshift permanent rec %d \n",pathindex);
          }
              g_dvrtest_handle->streampath[pathindex].convert_tsb_permrec = 1;
          }
          else
          {
           printf("Conversion of timeshift to perm recording not possible -> timeshift srvc not started");
          }
   }
          break;
   case 5:
         {
           if(g_dvrtest_handle->streampath[pathindex].convert_tsb_permrec)
           {
              dvr_err = B_DVR_Tsb_RecSrvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
          if(dvr_err!=B_DVR_SUCCESS)
          {
            printf("Error in stopping the timeshift permanent rec %d \n",pathindex);
          }
              BKNI_Free(g_dvrtest_handle->streampath[pathindex].segmentedrecreq);
              g_dvrtest_handle->streampath[pathindex].convert_tsb_permrec = 0;
           }
           else
           {
             printf("\n conversion of tsb to permrec not started");
           }
         }
         break;
   case 6:
        {
          int type_of_rec;
          printf("\n Enter the type of recording to playback : Timeshift/Perm Rec/Deleted Rec (0/1/2)");
          scanf("%d",&type_of_rec);
          if(g_dvrtest_handle->streampath[pathindex].live_decode_started)
          {
              dvrtest_live_decode_stop(pathindex);
              g_dvrtest_handle->streampath[pathindex].live_decode_started =0;
          }

          if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
          {
             dvr_err = B_DVR_TimeShift_Srvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
             if(dvr_err != B_DVR_SUCCESS)
              {
               printf("Error in stopping the timeshift service %d\n",pathindex);
              }
              g_dvrtest_handle->streampath[pathindex].timeshift_started = 0;
              BKNI_Free(g_dvrtest_handle->streampath[pathindex].timeshiftreq);
              dvr_err = B_DVR_TimeShift_Srvc_Close(g_dvrtest_handle->dvr_mgr_handle,pathindex);
              if(dvr_err != B_DVR_SUCCESS)
              {
               printf("Error in closing the timeshift service %d\n",pathindex);
              }

          }

          dvr_err = B_DVR_PbSrvc_Open(g_dvrtest_handle->dvr_mgr_handle,pathindex);
          if(dvr_err!=B_DVR_SUCCESS)
          {
           printf("Error in opening the playback service %d\n",pathindex);
           goto case_6_err;
          }

          g_dvrtest_handle->streampath[pathindex].pb_req = BKNI_Malloc(sizeof(struct B_DVR_PbSrvc_ReqObject));
          BKNI_Memset( g_dvrtest_handle->streampath[pathindex].pb_req,0,sizeof(struct B_DVR_PbSrvc_ReqObject));
          g_dvrtest_handle->streampath[pathindex].pb_req->RegisteredCallback = dvrtest_srvc_status_callback;
          g_dvrtest_handle->streampath[pathindex].pb_req->type_of_rec = type_of_rec;
		 g_dvrtest_handle->streampath[pathindex].pb_req->playback = playbackHandle[pathindex];
		 g_dvrtest_handle->streampath[pathindex].pb_req->playpump = playpumpHandle[pathindex];


          printf("\n Enter the program name to be played back\n");
          fflush(stdin);
          scanf("%s",g_dvrtest_handle->streampath[pathindex].pb_req->program_name);

		/* open PID channels and start decoding*/
		dvrtest_pb_decode_start(pathindex);

		 g_dvrtest_handle->streampath[pathindex].pb_req->videoCodec = g_dvrtest_handle->streampath[pathindex].videoprogram.codec;

          printf("\n enter pb file %s",g_dvrtest_handle->streampath[pathindex].pb_req->program_name);
          dvr_err = B_DVR_PbSrvc_Start(g_dvrtest_handle->dvr_mgr_handle,
                                       pathindex,g_dvrtest_handle->streampath[pathindex].pb_req);
          if(dvr_err!=B_DVR_SUCCESS)
          {
           printf("Error in starting the playback service %d \n",pathindex);

          }
          else
          {
          g_dvrtest_handle->streampath[pathindex].playback_started = 1;

          }
        }

         case_6_err:
          break;

   case 7 :
       {
           if(g_dvrtest_handle->streampath[pathindex].playback_started)
           {
			dvrtest_pb_decode_stop(pathindex);

            dvr_err = B_DVR_PbSrvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
           if(dvr_err!=B_DVR_SUCCESS)
           {
            printf("Error in stopping the playback service %d\n",pathindex);
           }
           BKNI_Free(g_dvrtest_handle->streampath[pathindex].pb_req);

			/* stop decoding and close PID channels*/

             dvr_err = B_DVR_PbSrvc_Close(g_dvrtest_handle->dvr_mgr_handle,pathindex);
             if(dvr_err!=B_DVR_SUCCESS)
             {
               printf("Error in closing the playback service %d\n",pathindex);
             }
             else
             {
             g_dvrtest_handle->streampath[pathindex].playback_started = 0;
           }
           }
           else
           {
            printf("\n playback srvc not started");
           }
          break;
       }
   case 8:
       {
           B_DVR_State b_dvr_state;
			dvrtest_live_decode_stop(pathindex);

           b_dvr_state.b_dvr_operation = B_DVR_START_TSB;
          if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
          {
			dvrtest_pb_decode_start(pathindex);
           B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
          }
          break;
        }
   case 9:
         {
           B_DVR_State b_dvr_state;
           b_dvr_state.b_dvr_operation = B_DVR_STOP_TSB;
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
			dvrtest_pb_decode_stop(pathindex);
           }
			dvrtest_live_decode_start(pathindex);
        }
          break;
   case 10:
         {
           B_DVR_State b_dvr_state;
           b_dvr_state.b_dvr_operation = B_DVR_FASTFORWARD;
           b_dvr_state.b_dvr_op_param.trickspeed = 4;
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
              if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
             B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
            }
          }

          break;
   case 11:
       {
           B_DVR_State b_dvr_state;
           b_dvr_state.b_dvr_operation = B_DVR_FASTREWIND;
           b_dvr_state.b_dvr_op_param.trickspeed = 4;
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {

            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {

            if(g_dvrtest_handle->streampath[pathindex].playback_started)
             {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
             }
            else
             {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
             }

           }
         }
          break;
   case 12:
        {
           B_DVR_State b_dvr_state;
           b_dvr_state.b_dvr_operation = B_DVR_SLOW_FORWARD;
           b_dvr_state.b_dvr_op_param.trickspeed = 4;
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
           if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
       }
          break;
   case 13:
       {
           B_DVR_State b_dvr_state;
           b_dvr_state.b_dvr_operation = B_DVR_SLOW_REVERSE;
           b_dvr_state.b_dvr_op_param.trickspeed = 4;
           if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
            if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
       }
          break;
   case 14:
         {
            B_DVR_State b_dvr_state;
            b_dvr_state.b_dvr_operation = B_DVR_FRAMEADVANCE_FORWARD;
            if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
            if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
         }
          break;
   case 15:
         {
            B_DVR_State b_dvr_state;
            b_dvr_state.b_dvr_operation = B_DVR_FRAMEADVANCE_REVERSE;
            if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
            if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
         }
          break;
   case 16:
         {
            B_DVR_State b_dvr_state;
			B_DVR_TimeShift_Srvc_Status status;
			NEXUS_PlaybackStatus pb_status;

            b_dvr_state.b_dvr_operation = B_DVR_GOTO;
            if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
			B_DVR_TimeShift_Srvc_GetStatus(g_dvrtest_handle->dvr_mgr_handle,pathindex, &status);
            b_dvr_state.b_dvr_op_param.start_time = status.first +	5000; /*5 sec*/
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
           if(g_dvrtest_handle->streampath[pathindex].playback_started)
            {
            B_DVR_PbSrvc_GetStatus(g_dvrtest_handle->dvr_mgr_handle,pathindex, &pb_status);
            b_dvr_state.b_dvr_op_param.start_time = pb_status.first	+ 5000; /*5 sec*/
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
         }
          break;
   case 17:
       {
            B_DVR_State b_dvr_state;
			B_DVR_TimeShift_Srvc_Status status;
			NEXUS_PlaybackStatus pb_status;

            b_dvr_state.b_dvr_operation = B_DVR_GOTO;
            if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
			B_DVR_TimeShift_Srvc_GetStatus(g_dvrtest_handle->dvr_mgr_handle,pathindex, &status);
            b_dvr_state.b_dvr_op_param.start_time = status.first -	5000; /*5 sec*/
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
           if(g_dvrtest_handle->streampath[pathindex].playback_started)
            {
            B_DVR_PbSrvc_GetStatus(g_dvrtest_handle->dvr_mgr_handle,pathindex ,&pb_status);
            b_dvr_state.b_dvr_op_param.start_time = pb_status.first- 5000; /*5 sec*/
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }              else
              {
                printf("\n neither playback nor timeshift srvc started! Invalid operation");
              }
           }
         }
         break;
   case 18:
       {
	   		B_DVR_TimeShift_Srvc_Status status;
          if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
          {
          B_DVR_TimeShift_Srvc_GetStatus(g_dvrtest_handle->dvr_mgr_handle,pathindex, &status);
          }

       }
       break;
   case 19:
       {
         B_DVR_State b_dvr_state;
         b_dvr_state.b_dvr_operation = B_DVR_PAUSE;
         if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {
            if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback or timeshift srvc started! Invalid operation");
              }
           }
       }
       break;
     case 20:
       {
         B_DVR_State b_dvr_state;
         b_dvr_state.b_dvr_operation = B_DVR_PLAY;
         if(g_dvrtest_handle->streampath[pathindex].timeshift_started)
           {
            B_DVR_TimeShift_Srvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
           else
           {

            if(g_dvrtest_handle->streampath[pathindex].playback_started)
              {
            B_DVR_PbSrvc_Operation(g_dvrtest_handle->dvr_mgr_handle,pathindex,b_dvr_state);
           }
              else
              {
                printf("\n neither playback or timeshift srvc started! Invalid operation");
              }
           }
       }
       break;
   case 21:
          if(g_dvrtest_handle->streampath[pathindex].live_decode_started)
           {
             dvrtest_live_decode_stop(pathindex);
               g_dvrtest_handle->streampath[pathindex].live_decode_started =0;
          }
       break;
   case 22:
          { int segmented=0;
            fflush(stdin);
            printf("\n For segmeneted bg rec enter 1 else 0");
            scanf("%d",&segmented);

           if(g_dvrtest_handle->streampath[pathindex].timeshift_started || g_dvrtest_handle->streampath[pathindex].playback_started ||
              g_dvrtest_handle->streampath[pathindex].live_decode_started)
           {
              printf("\n Tuner is on either live or timeshifting or playback decode! Invalid operation");
           }
           else
           {
             printf("\n Tuner is free on path %d",pathindex);
             int i;
             int current_channel = g_dvrtest_handle->streampath[pathindex].current_channel;
             dvr_err = B_DVR_RecSrvc_Open(g_dvrtest_handle->dvr_mgr_handle,pathindex);
             if(dvr_err!=B_DVR_SUCCESS)
             {
              printf("Error in opening background recording service %d\n",pathindex);
              goto case_2_err;
             }
             g_dvrtest_handle->streampath[pathindex].bg_recreq = BKNI_Malloc(sizeof(struct B_DVR_RecReq_Object));
             BKNI_Memset( g_dvrtest_handle->streampath[pathindex].bg_recreq,0,sizeof(struct B_DVR_RecReq_Object));
             g_dvrtest_handle->streampath[pathindex].bg_recreq->bencryptedrecording = false;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->ballpassrecording = false;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->bsegmented = (i=!0)?true:false;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->bpacing = false;
             printf("\n **** Enter the background recording name *****\n");
             printf("\n This should be always unique irrespective of how many channel changes happen "
                    "\n or how many times the STB is rebootted  i.e if a permanent record is created using this "
                    "\n temporary timeshift recording name, all the segments of this perm rec will have a prefix "
                    "\n of temp timeshift record name \n");
             fflush(stdin);
             scanf("%s",g_dvrtest_handle->streampath[pathindex].bg_recreq->program_name);

              printf("\n Enter the start time (msecs)of the permanent bg recording ==> \n");
              fflush(stdin);
              scanf("%d",&g_dvrtest_handle->streampath[pathindex].bg_recreq->rec_start_time);
              printf("\n Enter the end time ==>\n");
              fflush(stdin);
              scanf("%d",&g_dvrtest_handle->streampath[pathindex].bg_recreq->rec_end_time);

             g_dvrtest_handle->streampath[pathindex].bg_recreq->parserband = g_dvrtest_handle->streampath[streampath_index].parserband;
			 g_dvrtest_handle->streampath[pathindex].bg_recreq->transporttype = NEXUS_TransportType_eTs;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->active_vpid_index = 0;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[0].pid = dvr_channel_list[current_channel].video_pid[0];
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[0].pidtype = NEXUS_PidType_eVideo;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[0].codec.vcodec = dvr_channel_list[current_channel].video_format[0];
             g_dvrtest_handle->streampath[pathindex].bg_recreq->active_apid_index = 1;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[1].pid = dvr_channel_list[current_channel].audio_pid[0];
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[1].pidtype = NEXUS_PidType_eAudio;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[1].codec.acodec = dvr_channel_list[current_channel].audio_format[0];
             g_dvrtest_handle->streampath[pathindex].bg_recreq->no_of_pids = 2;
             for(i=0;i<g_dvrtest_handle->streampath[pathindex].bg_recreq->no_of_pids;i++)
             {
                printf("no:%d pid no: %d pidtype %d codec %d \n",i,g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[i].pid,
                g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[i].pidtype,g_dvrtest_handle->streampath[pathindex].bg_recreq->pids[i].codec.vcodec);
             }

             g_dvrtest_handle->streampath[pathindex].bg_recreq->parserband = g_dvrtest_handle->streampath[pathindex].parserband;
             g_dvrtest_handle->streampath[pathindex].bg_recreq->RegisteredCallback = dvrtest_srvc_status_callback;

		 g_dvrtest_handle->streampath[pathindex].bg_recreq->record = recordHandle[pathindex];
		 g_dvrtest_handle->streampath[pathindex].bg_recreq->recpump = recpumpHandle[pathindex];


             dvr_err = B_DVR_RecSrvc_Start(g_dvrtest_handle->dvr_mgr_handle,pathindex,g_dvrtest_handle->streampath[pathindex].bg_recreq);
             if(dvr_err!=B_DVR_SUCCESS)
             {
              printf("Error in starting the background recording service %d \n",pathindex);
             }
             else
             {
               printf("setting the background recording started flag %d\n",pathindex);
               g_dvrtest_handle->streampath[pathindex].bg_recording_started = 1;
             }

           }

           }
           break;
   case 23:
        {
            if(g_dvrtest_handle->streampath[pathindex].bg_recording_started)
            {
               dvr_err = B_DVR_RecSrvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
               if(dvr_err != B_DVR_SUCCESS)
               {
                 printf("Error in stopping the background recording service %d\n",pathindex);
               }

               BKNI_Free(g_dvrtest_handle->streampath[pathindex].bg_recreq);
               dvr_err = B_DVR_RecSrvc_Close(g_dvrtest_handle->dvr_mgr_handle,pathindex);
               if(dvr_err != B_DVR_SUCCESS)
               {
                printf("Error in closing the background recording service %d\n",pathindex);
               }

               g_dvrtest_handle->streampath[pathindex].bg_recording_started = 0;
            }
            else
           {
             printf("\n background recording srvc not started");
           }

        }
   case 24:
       {
          int type_of_rec;
          char program_name[256];
          printf("\n Delete a recording");
          printf("\n Enter the type of recording : Timeshift/Perm Rec/Deleted Rec (0/1/2)");
          scanf("%d",&type_of_rec);
          printf("\n Enter the recording name without extensions");
          scanf("\n %s",program_name);
          switch(type_of_rec)
          {
          case 0:
                B_DVR_Delete_Recording(g_dvrtest_handle->dvr_mgr_handle,program_name,TIMESHIFT_REC);
                break;
          case 1:
                B_DVR_Delete_Recording(g_dvrtest_handle->dvr_mgr_handle,program_name,PERMANENT_REC);
               break;
          case 2:
               B_DVR_Delete_Recording(g_dvrtest_handle->dvr_mgr_handle,program_name,DELETED_REC);
               break;
          default:
               printf("\n Invalid recording type");
          }

       }
        break;
   case 25:
       {
           int type_of_rec_source, type_of_rec_dest;
           char program_name[256];
           printf("\n Delete a recording");
           printf("\n Enter the type of source recording : Timeshift/Perm Rec/Deleted Rec (0/1/2)");
           scanf("%d",&type_of_rec_source);
           printf("\n Enter the type of dest recording : Timeshift/Perm Rec/Deleted Rec (0/1/2)");
           scanf("%d",&type_of_rec_dest);
           printf("\n Enter the recording name without extensions");
           scanf("\n %s",program_name);
           B_DVR_Move_Recording(g_dvrtest_handle->dvr_mgr_handle,program_name,type_of_rec_source,type_of_rec_dest);

       }
        break;
   case 26:
       {
       printf("\n dvr_test : B_DVR_Display_RecordingList ==>");
         printf("\n dvr_test : Permanent Recording list ==>");
         B_DVR_Display_RecordingList(g_dvrtest_handle->dvr_mgr_handle,PERMANENT_REC);
         printf("\n dvr_test : Timeshift Recording list ==>");
         B_DVR_Display_RecordingList(g_dvrtest_handle->dvr_mgr_handle,TIMESHIFT_REC);
         printf("\n dvr_test : Deleted Recording list ==>");
         B_DVR_Display_RecordingList(g_dvrtest_handle->dvr_mgr_handle,DELETED_REC);
       }
       break;
   default:
          printf("Invalid DVR Operation - Select the operation again");

   }

}
  main_exit:
      printf("\n Closing all the services before stopping the test app");
  for(streampath_index=0;streampath_index < NEXUS_NUM_VIDEO_DECODERS; streampath_index++)
  {

    /*
     * Check if timeshifting/playback is on? If yes, invoke dvr lib APIs to shut them off.
     */
    if(g_dvrtest_handle->streampath[streampath_index].timeshift_started)
     {
       dvr_err = B_DVR_TimeShift_Srvc_Stop(g_dvrtest_handle->dvr_mgr_handle,streampath_index);
       if(dvr_err != B_DVR_SUCCESS)
       {
         printf("Error in stopping the timeshift service %d\n",pathindex);
       }
       g_dvrtest_handle->streampath[streampath_index].timeshift_started = 0;
       BKNI_Free(g_dvrtest_handle->streampath[streampath_index].timeshiftreq);
       dvr_err = B_DVR_TimeShift_Srvc_Close(g_dvrtest_handle->dvr_mgr_handle,streampath_index);
       if(dvr_err != B_DVR_SUCCESS)
       {
         printf("Error in closing the timeshift service %d\n",streampath_index);
       }
    }

    if(g_dvrtest_handle->streampath[streampath_index].live_decode_started)
    {
    dvrtest_live_decode_stop(streampath_index);
    }

    if(g_dvrtest_handle->streampath[streampath_index].playback_started)
    {
        dvr_err = B_DVR_PbSrvc_Stop(g_dvrtest_handle->dvr_mgr_handle,pathindex);
        if(dvr_err!=B_DVR_SUCCESS)
        {
          printf("Error in stopping the playback service %d\n",pathindex);
        }
        BKNI_Free(g_dvrtest_handle->streampath[pathindex].pb_req);
        dvr_err = B_DVR_PbSrvc_Close(g_dvrtest_handle->dvr_mgr_handle,pathindex);
        if(dvr_err!=B_DVR_SUCCESS)
        {
          printf("Error in closing the playback service %d\n",pathindex);
        }
        g_dvrtest_handle->streampath[pathindex].playback_started = 0;
    }
    dvrtest_streampath_resource_close(streampath_index);
  }
  B_DVR_Mgr_Close();
  B_Os_Uninit();
  NEXUS_Platform_Uninit();

  return 0;
}
