/***************************************************************************
 *     Copyright (c) 2009-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
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
#include "nexus_security.h"
#include "nexus_dma.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_manager.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_mediafile.h"
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_drmservice.h"
#include "msutil.h"
#include "msdiag.h"


BDBG_MODULE(dvr_drm_test);

#define MAX_STATIC_CHANNELS             4
#define MAX_PROGRAMS_PER_CHANNEL       6
#define MAX_PROGRAM_TITLE_LENGTH       128
#define MAX_AUDIO_STREAMS               4
#define MAX_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 (NEXUS_MAX_FRONTENDS-1)
#define MAX_AV_PATH                     2
#define TSB_SERVICE_PREFIX              "streamPath"
#define TRANSCODE_SERVICE_PREFIX      "transcoded"
#define MAX_TSB_BUFFERS                 5
#define TRANSCODE0_STC_CHANNEL_INDEX    4
#define TRANSCODE1_STC_CHANNEL_INDEX    5
#define KEY_LENGTH                    16
#define MAX_DRM_NUM                   32
#define SUPPORT_ONE_KEY
#define DMA_BLOCK_SIZE   				188
#define NUM_DMA_BLOCK     				1024

unsigned  char changeToLocalKey[16];
unsigned  char changeToLocalKey2[16];

unsigned  char changeToLocalKeyVideoEven[16];
unsigned  char changeToLocalKeyVideoOdd[16];
unsigned  char changeToLocalKeyAudioEven[16];
unsigned  char changeToLocalKeyAudioOdd[16];

unsigned char changeToLocalSession[16];
unsigned char changeToLocalControlWord[16];
unsigned char changeToIv[16];

int changeToLocalIv[16];


/***********************/
/* Same Key for Even & Odd **/ 
/***********************/
unsigned char VidEvenCpsControlWord[] = { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char VidOddCpsControlWord[] =  { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudEvenCpsControlWord[] = { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudOddCpsControlWord[] =  { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   


/***********************/
/* Diff. Keys for Even & Odd **/ 
/***********************/
unsigned char VidEvenCpsCW[] = { 0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char VidOddCpsCW[] =  { 0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudEvenCpsCW[] = { 0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudOddCpsCW[] =  { 0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7 ,0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   


/***********************/
/****** For Keyladder ******/ 
/***********************/
unsigned char ucProcInKey3[16] = { 0x00 ,0x00 ,0x11 ,0x11 ,0x55 ,0x55 ,0x66 ,0x66 ,0x11 ,0x22 ,0x33 ,0x44 ,0x55 ,0x66 ,0x77 ,0x88 };
unsigned char ucProcInKey4[16] = { 0x28 ,0x30 ,0x46 ,0x70 ,0x71 ,0x4B ,0x8C ,0x75 ,0xEB ,0x46 ,0x04 ,0xBB ,0x96 ,0xD0 ,0x48 ,0x88};
unsigned char ivkeys[16] = { 0xad ,0xd6 ,0x9e ,0xa3 ,0x89 ,0xc8 ,0x17 ,0x72 ,0x1e ,0xd4 ,0x0e ,0xab ,0x3d ,0xbc ,0x7a ,0xf2 };



/*******************************/
/* Same Keys for Even & Odd  callback **/ 
/*******************************/
void videoKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalKey[i] = VidEvenCpsControlWord[i];     
     /*  printf("[Vid] %d = 0x%x\n",i,changeToLocalKey[i]);*/
    }

}

void audioKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH ;i++) 
    {
         changeToLocalKey2[i] = AudEvenCpsControlWord[i] ;   
        /* printf("[Aud] %d = 0x%x\n",i,changeToLocalKey2[i]);       */
    }

}

/******************************/
/* Diff. Keys for Even & Odd  callback **/ 
/******************************/
void videoEvenKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalKeyVideoEven[i] = VidEvenCpsCW[i];     
       printf("[Vid Even] %d = 0x%x\n",i,changeToLocalKeyVideoEven[i]);
    }

}

void videoOddKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalKeyVideoOdd[i] = VidOddCpsCW[i];     
       printf("[Vid Odd] %d = 0x%x\n",i,changeToLocalKeyVideoOdd[i]);
    }

}

void audioEvenKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH ;i++) 
    {
         changeToLocalKeyAudioEven[i] = AudEvenCpsCW[i] ;   
         printf("[Aud Even] %d = 0x%x\n",i,changeToLocalKeyAudioEven[i]);       
    }

}

void audioOddKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH;i++) 
    {
         changeToLocalKeyAudioOdd[i] = AudOddCpsCW[i];   
         printf("[Aud Odd] %d = 0x%x\n",i,changeToLocalKeyAudioOdd[i]);       
    }

}

/******************/
/* Keyladder callback **/ 
/******************/
void procInKey3_sessionKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalSession[i] = ucProcInKey3[i];   
         printf("[sessionKey] %d = 0x%x",i,changeToLocalSession[i]);
    }   
}

void procInKey4_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToLocalControlWord[i] = ucProcInKey4[i]);   
        printf("[procInKey4] %d = 0x%x",i,changeToLocalControlWord[i]);    
    }   

}

void iv_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToIv[i] = ivkeys[i]);   
        printf("[IV] %d = 0x%x",i,changeToIv[i]);    
    }   

}


typedef struct DvrTest_ChannelInfo
{
    NEXUS_FrontendQamAnnex annex; /* To describe Annex A or Annex B format */
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolrate;
    unsigned int  frequency;
    char programTitle[MAX_PROGRAM_TITLE_LENGTH];
    int numAudioStreams;
    int numVideoStreams;
    unsigned int  videoPids[MAX_VIDEO_STREAMS];
    unsigned int  audioPids[MAX_AUDIO_STREAMS];
    unsigned int  pcrPid[MAX_VIDEO_STREAMS];
    NEXUS_VideoCodec videoFormat[MAX_VIDEO_STREAMS];
    NEXUS_AudioCodec audioFormat[MAX_AUDIO_STREAMS];
} DvrTest_ChannelInfo;

static DvrTest_ChannelInfo channelInfo[MAX_STATIC_CHANNELS] =
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

struct DvrTest_StreamPath
{
    DvrTest_ChannelInfo *tunedChannel;
    unsigned currentChannel;
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendQamStatus qamStatus;
    NEXUS_FrontendCapabilities capabilities;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelHandle stcAudioChannel;
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_TSBServiceRequest tsbServiceRequest;
    B_DVR_TranscodeServiceHandle transcodeService;
    B_DVR_TranscodeServiceRequest transcodeServiceRequest;
    BKNI_EventHandle transcodeFinishEvent;
    unsigned transcodeOption;
    B_DVR_PlaybackServiceHandle playbackService;
    NEXUS_PidChannelHandle audioPlaybackPidChannels[MAX_VIDEO_STREAMS];
    NEXUS_PidChannelHandle videoPlaybackPidChannels[MAX_AUDIO_STREAMS];
    NEXUS_PidChannelHandle audioPidChannels[MAX_VIDEO_STREAMS];
    NEXUS_PidChannelHandle videoPidChannels[MAX_AUDIO_STREAMS];
    NEXUS_PlaybackHandle playback;
    B_DVR_PlaybackServiceRequest playbackServiceRequest;
    B_DVR_RecordServiceRequest recordServiceRequest;
    B_DVR_RecordServiceHandle recordService;
    B_DVR_DataInjectionServiceHandle dataInjectionService;
    B_DVR_DRMServiceHandle drmService[MAX_DRM_NUM];
    B_DVR_DRMServiceRequest drmServiceRequest;  
    bool liveDecodeStarted;
    bool playbackDecodeStarted;
};

typedef struct DvrTest_StreamPath DvrTest_StreamPathHandle;

struct DvrTest
{
    B_DVR_ManagerHandle dvrManager;
    B_DVR_MediaStorageHandle mediaStorage;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageRegisterVolumeSettings mediaStorageRegisterSettings;
    NEXUS_PlatformConfiguration platformconfig;
    DvrTest_StreamPathHandle streamPath[MAX_STREAM_PATH];
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    int maxChannels;
    B_DVR_ServiceCallback dvrTestCallback;
    B_DVR_DRMService_KeyLoaderCallback dvrTestDrmCallback;
    void *buffer;
    B_DVR_MediaFileHandle mediaFile;
    B_DVR_DRMServiceSettings drmServiceSettings;
};
typedef struct DvrTest *DvrTestHandle;

DvrTestHandle g_dvrTest;

void DvrTest_MediaStorageMenu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t mediaStorage Operations");
    printf("\n ************************************************");
    printf("\n 1: format");
    printf("\n 2: register");
    printf("\n 3: Unregister");
    printf("\n 4: volume status");
    printf("\n 5: mount");
    printf("\n 6: unmount");
    printf("\n 7: get metadata folder path");
    printf("\n 8: print volume status");
    printf("\n 0: exit\n");
    printf("\n Select an operation: ");
    return;
}

void DvrTest_Menu(void)
{
    printf("\n ************************************************");
    printf("\n \t \t DVRExtLib DRM Operations");
    printf("\n ************************************************");
    printf("\n 1: change channel");
    printf("\n 2: start TSBS (clearKey) (Cps) ( scPolarity = eclear )");
    printf("\n 3: stop  TSBS (clearKey) (Cps)");
    printf("\n 4: start TSBPlayback ( in paused state )");
    printf("\n 5: start TSBS with vendor specific key callback");
    printf("\n 6: stop  TSBS with vendor specific keycallback");
    printf("\n 7: start TSBS Convert");
    printf("\n 8: stop  TSBS Convert");
    printf("\n 9: start Playback (clearKey) (Ca)");
    printf("\n 10: stop  Playback(clearKey) (Ca)");
    printf("\n 11: start bg record (clearKey, one key) (Cps)");
    printf("\n 12  stop  bg record (clearKey, one key) (Cps)");
    printf("\n 13. play");
    printf("\n 14. pause");
    printf("\n 15  rewind");
    printf("\n 16  fastforward");   
    printf("\n 17. slow rewind");
    printf("\n 18. slow forward");
    printf("\n 19. forward frame advance");
    printf("\n 20: reverse frame advance");
	printf("\n 21. start Playback (clearKey) (M2m)");
    printf("\n 22: stop  Playback (clearKey) (M2m)");	
	printf("\n 23. start Record (clearKey) (M2m)");
    printf("\n 24: stop  Record (clearKey) (M2m)");	
	printf("\n 25. start TSB (clearKey) (M2m)");
    printf("\n 26: stop  TSB (clearKey) (M2m)");	
	printf("\n 27. start Playback (keyladder) (M2m)");
    printf("\n 28: stop  Playback (keyladder) (M2m)");
	printf("\n 29. start Record (keyladder) (M2m)");
    printf("\n 30: stop  Record (keyladder) (M2m)");
	printf("\n 31. start TSB (keyladder) (Cps)");
    printf("\n 32: stop  TSB (keyladder) (Cps)");
    printf("\n 33: start TSBPlayback_m2m ( in paused state )");
    printf("\n 34: Test For Home Networking M2M Enc/Dec Using DMA");
	printf("\n 35. start Record (clearKey, Even/Odd) (Cps)");
    printf("\n 36: stop  Record (clearKey, Even/Odd) (Cps)");
	printf("\n 37. start Playback (clearKey, Even/Odd) (Ca)");
    printf("\n 38: stop  Playback (clearKey, Even/Odd) (Ca)");
    printf("\n 0: exit\n");
    printf("\n Select an operation: ");

    return;
}
typedef enum DvrTest_Operation
{
    eDvrTest_OperationQuit,                                         /*0*/
    eDvrTest_OperationChannelChange,                               /*1*/
    eDvrTest_OperationTSBServiceStartGenericClearKey,            /*2*/
    eDvrTest_OperationTSBServiceStopGenericClearKey,             /*3*/    
    eDvrTest_OperationTSBServicePlayStart,                         /*4*/    
    eDvrTest_OperationTSBServiceStartVendorSpecificKey, /*5*/    
    eDvrTest_OperationTSBServiceStopVendorSpecificKey,  /*6*/
    
    eDvrTest_OperationTSBServiceConvertStart,   /*7*/
    eDvrTest_OperationTSBServiceConvertStop,    /*8*/
    eDvrTest_OperationPlaybackServiceStart,     /*9*/
    eDvrTest_OperationPlaybackServiceStop,      /*10*/
    eDvrTest_OperationRecordServiceStart,       /*11*/
    eDvrTest_OperationRecordServiceStop,        /*12*/
    
    eDvrTest_OperationPlay,                       /*13*/
    eDvrTest_OperationPause,                      /*14*/
    eDvrTest_OperationRewind,                     /*15*/
    eDvrTest_OperationFastForward,              /*16*/              
    eDvrTest_OperationSlowRewind,               /*17*/
    eDvrTest_OperationSlowForward,              /*18*/
    eDvrTest_OperationFrameAdvance,             /*19*/
    eDvrTest_OperationFrameReverse,             /*20*/

    eDvrTest_OperationPlaybackServiceStartM2m,     		/*21*/
    eDvrTest_OperationPlaybackServiceStopM2m,      		/*22*/
    eDvrTest_OperationRecordServiceStartM2m,     			/*23*/
    eDvrTest_OperationRecordServiceStopM2m,      			/*24*/
    eDvrTest_OperationTSBServiceStartGenericClearKeyM2m,  /*25*/
    eDvrTest_OperationTSBServiceStopGenericClearKeyM2m,    /*26*/
    
    eDvrTest_OperationPlaybackServiceStartKeyladderM2m,	/*27*/
	eDvrTest_OperationPlaybackServiceStopKeyladderM2m, 	/*28*/
    eDvrTest_OperationRecordServiceStartKeyladderM2m,		/*29*/
	eDvrTest_OperationRecordServiceStopKeyladderM2m, 		/*30*/
    eDvrTest_OperationTSBServiceStartKeyladderCps,    		/*31*/
    eDvrTest_OperationTSBServiceStopKeyladderCps,    		/*32*/
    eDvrTest_OperationPlaybackServiceStartM2m_Trick,	    /*33*/
    eDvrTest_OperationHomeNetworkM2mTest,	    			/*34*/
    eDvrTest_OperationRecordServiceStartCpsEvenOddClearKey,  /*35*/
    eDvrTest_OperationRecordServiceStopCpsEvenOddClearKey,   /*36*/    
	eDvrTest_OperationPlaybackServiceStartCpsEvenOddClearKey,  /*37*/
	eDvrTest_OperationPlaybackServiceStopCpsEvenOddClearKey,	  /*38*/    
    eDvrTest_OperationMax
}DvrTest_Operation;

void DvrTest_PrintChannelMap(void)
{
    int channelCount, esStreamCount;
    for (channelCount = 0; channelCount < g_dvrTest->maxChannels; channelCount++)
    {
        printf("channel index   ==>  %d\n", channelCount);
        printf("\tannex          ==> %d\n",channelInfo[channelCount].annex);
        printf("\tmodulation     ==> %d\n",channelInfo[channelCount].modulation);
        printf("\tfrequency      == %d\n", channelInfo[channelCount].frequency);
        printf("\tsymbolrate     ==> %d\n", channelInfo[channelCount].symbolrate);
        printf("\tprogram_title  ==> %s\n",channelInfo[channelCount].programTitle);
        printf("\t no of audio streams  ==> %d\n",channelInfo[channelCount].numAudioStreams);
        printf("\t no of video streams  ==> %d\n",channelInfo[channelCount].numVideoStreams);
        printf("\t video_pids/format ==>");
        for(esStreamCount=0;esStreamCount<channelInfo[esStreamCount].numVideoStreams;esStreamCount++)
        {
         printf("\t 0x%04x/%d",channelInfo[channelCount].videoPids[esStreamCount],channelInfo[channelCount].videoFormat[esStreamCount]);
        }
        printf("\n");
        printf("\taudioPids; " );
        for(esStreamCount=0;esStreamCount<channelInfo[channelCount].numAudioStreams;esStreamCount++)
        {
         printf("\t 0x%04x/%d", channelInfo[channelCount].audioPids[esStreamCount],channelInfo[channelCount].audioFormat[esStreamCount]);
        }
        printf("\n");
        printf("\tpcrPid:");
        for(esStreamCount=0;esStreamCount<channelInfo[channelCount].numVideoStreams;esStreamCount++)
        {
          printf("\t 0x%04x",channelInfo[channelCount].pcrPid[channelCount]);
        }
        printf("\n");
    }
}

void DvrTest_AudioVideoPathOpen(unsigned index)
{
     printf("\n %s:  index %d >>> ",__FUNCTION__,index);
    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].stcSettings.timebase = index?NEXUS_Timebase_e1:NEXUS_Timebase_e0;

    g_dvrTest->streamPath[index].stcChannel = NEXUS_StcChannel_Open(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].stcAudioChannel = NEXUS_StcChannel_Open(index+MAX_AV_PATH, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].videoProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].audioProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].window = NEXUS_VideoWindow_Open(g_dvrTest->display,index);
    g_dvrTest->streamPath[index].videoDecoder = NEXUS_VideoDecoder_Open(index, NULL);
    NEXUS_VideoWindow_AddInput(g_dvrTest->streamPath[index].window,
                               NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    if(!index)
    {
        g_dvrTest->streamPath[index].audioDecoder = NEXUS_AudioDecoder_Open(index, NULL);
        
    }
    else
    {
    g_dvrTest->streamPath[index].audioDecoder = NULL;
    }
    
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}
void DvrTest_AudioVideoPathClose(int index)
{
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    if(!index)
    {
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioDecoder_Close(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_VideoWindow_RemoveAllInputs(g_dvrTest->streamPath[index].window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    NEXUS_VideoWindow_Close(g_dvrTest->streamPath[index].window);
    NEXUS_VideoDecoder_Close(g_dvrTest->streamPath[index].videoDecoder);
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_TuneChannel(int index)
{
    NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);
    g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrTest->streamPath[index].userParams.param1;
    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    g_dvrTest->streamPath[index].parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);

    NEXUS_Frontend_GetDefaultQamSettings(&g_dvrTest->streamPath[index].qamSettings);
    g_dvrTest->streamPath[index].qamSettings.frequency =  channelInfo[currentChannel].frequency;
    g_dvrTest->streamPath[index].qamSettings.mode = channelInfo[currentChannel].modulation;
    g_dvrTest->streamPath[index].qamSettings.annex = channelInfo[currentChannel].annex;
    g_dvrTest->streamPath[index].qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;

    rc = NEXUS_Frontend_TuneQam(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamSettings);
    BDBG_ASSERT(!rc);
    BKNI_Sleep(2000);
    rc = NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamStatus);
    BDBG_ASSERT(!rc);
    printf("\n %s: receiver lock = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.receiverLock);
    printf("\n %s: Symbol rate = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.symbolRate);

    printf("\n %s: index %d >>>",__FUNCTION__,index);
    return;
}

void DvrTest_PlaybackDecodeStart(int index,int drmIndex)
{
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_Media media;
    B_DVR_ERROR rc;
    printf("\n %s:index %d >>>",__FUNCTION__,index);

    if(g_dvrTest->streamPath[index].tsbService || g_dvrTest->streamPath[index].playbackService)
    {
    if(g_dvrTest->streamPath[index].tsbService)
    {
            mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
            mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[index].tsbServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[index].tsbServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].tsbServiceRequest.volumeIndex;

            BSTD_UNUSED(playbackServiceSettings);
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            g_dvrTest->streamPath[index].playback = tsbServiceSettings.tsbPlaybackSettings.playback;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            {
                tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeStcChannel;
            }
            else
            {
            tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            }
            
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            
            
        }
        else
        {
            mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
            mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[index].playbackServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[index].playbackServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].playbackServiceRequest.volumeIndex;
 
            BSTD_UNUSED(tsbServiceSettings);
            B_DVR_PlaybackService_GetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
            g_dvrTest->streamPath[index].playback = playbackServiceSettings.playback;
            playbackServiceSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            playbackServiceSettings.videoDecoder[1]=NULL;
            playbackServiceSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            playbackServiceSettings.audioDecoder[1]=NULL;
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            { 
                playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeStcChannel;
            }
            else
            {
            playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            }
            B_DVR_PlaybackService_SetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
        }
   
        rc = B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n media node not found");
            assert(!rc);
        }
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = g_dvrTest->streamPath[index].videoDecoder;
        g_dvrTest->streamPath[index].videoPlaybackPidChannels[0] =
             NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[0].pid,&playbackPidSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = g_dvrTest->streamPath[index].audioDecoder;
        g_dvrTest->streamPath[index].audioPlaybackPidChannels[0] = 
            NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[1].pid,&playbackPidSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
        g_dvrTest->streamPath[index].videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
        g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPlaybackPidChannels[0];
        g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
        NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);

		if((g_dvrTest->streamPath[index].drmService[drmIndex]) &&
			((g_dvrTest->drmServiceSettings.drmDest == NEXUS_SecurityAlgorithmConfigDestination_eCps)||
			(g_dvrTest->drmServiceSettings.drmDest == NEXUS_SecurityAlgorithmConfigDestination_eCa)))
		{
	        B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[index].drmService[drmIndex],
                                               g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
			drmIndex++;	
	        
	        B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[index].drmService[drmIndex],
                                              g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
        printf("\nDRM Service for tsb playback started\n");
        }
		else if ((g_dvrTest->streamPath[index].drmService[drmIndex]) &&
			(g_dvrTest->drmServiceSettings.drmDest == NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem))
		{

			B_DVR_PlaybackService_AddDrmSettings(g_dvrTest->streamPath[index].playbackService,
													  g_dvrTest->streamPath[index].drmService[drmIndex]);
			printf("\nDRM Service for m2m playback started\n");
    }    
    else
    {
			printf("\n any DRM is not started.\n");
    }


        if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
            g_dvrTest->streamPath[index].videoProgram.nonRealTime = true;
            
        }
        else
        {
            g_dvrTest->streamPath[index].videoProgram.nonRealTime = false;
        }
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            { 
                NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcAudioChannel,&g_dvrTest->streamPath[index].stcSettings);
                g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
                NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
                g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcAudioChannel;
                g_dvrTest->streamPath[index].audioProgram.nonRealTime = true;

            }
            else
            {
            g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
                g_dvrTest->streamPath[index].audioProgram.nonRealTime = false;
                NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                                                          NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                          NEXUS_AudioDecoderConnectorType_eStereo)
                                                                          );
            }

            NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);
        }
        g_dvrTest->streamPath[index].playbackDecodeStarted=true;

    }    
    else
    {
        printf("\n error!!!!!neither playback or tsbservice opened");
    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}

void DvrTest_PlaybackDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
        if(!(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime))
        { 
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
            NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        }
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
    g_dvrTest->streamPath[index].playbackDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_LiveDecodeStart(int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                             channelInfo[currentChannel].videoPids[0],
                                                                             NULL);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
    g_dvrTest->streamPath[index].videoProgram.codec = channelInfo[currentChannel].videoFormat[0];
    g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;

    NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
    NEXUS_Timebase_GetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);

    g_dvrTest->streamPath[index].stcSettings.autoConfigTimebase = true;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.offsetThreshold = 8;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.maxPcrError = 255;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableTimestampCorrection = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableJitterAdjustment = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoProgram.pidChannel;
    g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */

    NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);
    if(!index)
    {
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                    NEXUS_AudioDecoderConnectorType_eStereo)
                                   );

        g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                 channelInfo[currentChannel].audioPids[0],
                                                                                 NULL);

       NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
       g_dvrTest->streamPath[index].audioProgram.codec = channelInfo[currentChannel].audioFormat[0];
       g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPidChannels[0];
       g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
       NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);

    }
    g_dvrTest->streamPath[index].liveDecodeStarted=true;
    
    printf("\n %s:index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_LiveDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].audioPidChannels[0]);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].videoPidChannels[0]);
    g_dvrTest->streamPath[index].liveDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

static void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow",
                                            "underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec",
                                            "abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace",
                                            "format success","format fail","Mount success","mount fail","transcode finish","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService",
                                               "storagaService","dataInjectionService","transcodeService","invalidService"};
    printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    BSTD_UNUSED(appContext);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n Beginning Of TSB");
            }
            else
            {
                printf("\n Beginning of background recording");

            }
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n End Of TSB Conversion");
            }
            else
            {
                printf("\n End of background recording");

            }
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n HD Stream recording in TSB Service");
            }
            else
            {
                printf("\n HD Stream recording in Record Service");
            }

        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                printf("\n SD Stream recording in TSB Service");
            }
            else
            {
                printf("\n SD Stream recording in Record Service");
            }
        }
        break;
    case eB_DVR_EventTSBConverstionCompleted:
        {
            printf("\n TSB Conversion completed");
        }
        break;
    case eB_DVR_EventOverFlow:
        {
            printf("\n OverFlow");
        }
        break;
    case eB_DVR_EventUnderFlow:
        {
            printf("\nunderFlow");
        }
        break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream. Pausing");
            printf("\n Stop either TSBPlayback or Playback to enter live mode");
        }
        break;
    case eB_DVR_EventStartOfPlayback:
        {
            if(service == eB_DVR_ServicePlayback)
            {
              printf("\n Beginnning of playback stream. Pausing");
            }
            else
            {
                printf("\n Beginning of TSB. Pausing");
            }
        }
        break;
    case eB_DVR_EventPlaybackAlarm:
        {
            printf("\n playback alarm event");
        }
        break;
    case eB_DVR_EventAbortPlayback:
        {
            printf("\n abort playback");
        }
        break;
    case eB_DVR_EventAbortRecord:
        {
            printf("\n abort record");
        }
        break;
    case eB_DVR_EventAbortTSBRecord:
        {
            printf("\n abort TSB record");
        }
        break;
    case eB_DVR_EventAbortTSBPlayback:
        {
            printf("\n abort TSB Playback");
        }
        break;
    case eB_DVR_EventDataInjectionCompleted:
        {
            printf("\n data Injection complete");
        }
        break;
    case eB_DVR_EventOutOfMediaStorage:
        {
            printf("\n no media Storage space");
        }
        break;
    case eB_DVR_EventOutOfNavigationStorage:
        {
            printf("\n no navigation Storage space");
        }
        break;
    case eB_DVR_EventOutOfMetaDataStorage:
        {
            printf("\n no metaData Storage space");
        }
        break;
    case eB_DVR_EventTranscodeFinish:
        {
            printf("\n Transcode finish");
            BKNI_SetEvent(g_dvrTest->streamPath[index].transcodeFinishEvent);
        }
    default:
        {
            printf("\n invalid event");
        }
   }
    printf("\n DvrTest_Callback <<<<<");
    return;
}

static void DvrTestDrmCallback(NEXUS_KeySlotHandle keyHandle)
{
 
    NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    invSettings.keyDestEntryType  = NEXUS_SecurityKeyType_eOddAndEven;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    invSettings.keyDestBlckType   = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    invSettings.virtualKeyLadderID = NEXUS_SecurityVirtualKeyladderID_eVKLDummy;
    NEXUS_Security_InvalidateKey(keyHandle, &invSettings);
    
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm           = NEXUS_SecurityAlgorithm_eAes128;  
    AlgConfig.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eCipherStealing;
    AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID          = 0x1234;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
    AlgConfig.key2Select          = NEXUS_SecurityKey2Select_eReserved1;    
    AlgConfig.dest                = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    AlgConfig.bRestrictEnable     = false;
    AlgConfig.bEncryptBeforeRave  = false;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal]     = true;   
    AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] = NEXUS_SecurityAlgorithmScPolarity_eEven;
    AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal]     = NEXUS_SecurityAlgorithmScPolarity_eEven;

    if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CPS Algorithm failed \n");     
    }

        /* Load AV keys */
    key.keySize = 16; 
    key.dest =NEXUS_SecurityAlgorithmConfigDestination_eCps;
    key.keyIVType    = NEXUS_SecurityKeyIVType_eNoIV;   

    memcpy(key.keyData,VidEvenCpsControlWord,key.keySize);

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
    {
        printf("\nLoad CPS EVEN key failed \n");
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;

    if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
    {
        printf("\nLoad CPS ODD key failed \n");
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    if ( NEXUS_Security_LoadClearKey (keyHandle, &key) != 0 )
    {
        printf("\nLoad CPS CLEAR key failed \n");
    }
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    printf( "\n HDMI hotplug event: %s", status.connected?"connected":"not connected");
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

B_DVR_ERROR DvrTest_FileToFileTranscodeStart(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Media playbackMedia;
    B_DVR_RecordServiceSettings recordServiceSettings;
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TranscodeServiceInputEsStream transcodeInputEsStream;
    B_DVR_TranscodeServiceInputSettings transcodeInputSettings;
    B_DVR_RecordServiceInputEsStream recordInputEsStream;
    NEXUS_PidChannelHandle transcodePidChannel;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;
	int drmIndex = 31;
    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop >>>");
    /* stop live decode*/
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTest_LiveDecodeStop(pathIndex);
    }

    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop <<<");
    BKNI_Memset(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].playbackServiceRequest));
    fflush(stdin);
    printf("\n Enter a program name to be transcoded : ");
    scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
    fflush(stdin);
    printf("\n Enter the subDir for the program:");
    scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
    /* open playback service */
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open >>>");
    g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].playbackService)
    {
        printf("\n playback service open failed for program %s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
        goto error_playbackService;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<");
    B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<");
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(B_DVR_TranscodeServiceRequest));
    printf("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<");
    NEXUS_StcChannel_GetDefaultSettings(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    stcSettings.timebase = pathIndex?NEXUS_Timebase_e1: NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    /* open transcode stc channel */
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel = NEXUS_StcChannel_Open(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    if(!g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel)
    {
        printf("trancodeStcChannel open failed");
        goto error_transcodeStcChannel;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open >>>");
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.displayIndex = 3;
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType = eB_DVR_TranscodeServiceType_NonRealTime;
    /* open transcode service */
    g_dvrTest->streamPath[pathIndex].transcodeService = B_DVR_TranscodeService_Open(&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].transcodeService)
    {
        printf("\n transcode service open failed");
        goto error_transcodeService;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback >>>");
    B_DVR_TranscodeService_InstallCallback(g_dvrTest->streamPath[pathIndex].transcodeService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings >>>");
    B_DVR_TranscodeService_GetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    transcodeInputSettings.input = eB_DVR_TranscodeServiceInput_File;
    transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;
    transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    transcodeInputSettings.hdmiInput = NULL;
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings >>>");
    B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode >>>");
    B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[pathIndex].playbackService,&playbackMedia);
    printf("\n playback vPid %u aPid %u",playbackMedia.esStreamInfo[0].pid,playbackMedia.esStreamInfo[1].pid);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid>>>");
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&playbackMedia.esStreamInfo[0],sizeof(playbackMedia.esStreamInfo[0]));
    transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    transcodeInputEsStream.videoEncodeParams.codec = NEXUS_VideoCodec_eH264;
    transcodeInputEsStream.videoEncodeParams.interlaced = false;
    transcodeInputEsStream.videoEncodeParams.profile = NEXUS_VideoCodecProfile_eBaseline;
    transcodeInputEsStream.videoEncodeParams.videoDecoder = g_dvrTest->streamPath[pathIndex].videoDecoder;
    transcodeInputEsStream.videoEncodeParams.level = NEXUS_VideoCodecLevel_e31;
    /* add video ES stream*/
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid >>>");
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.audioEncodeParams.audioDecoder = g_dvrTest->streamPath[pathIndex].audioDecoder;
    transcodeInputEsStream.audioEncodeParams.audioPassThrough = false;
    transcodeInputEsStream.audioEncodeParams.codec = NEXUS_AudioCodec_eAac;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    if(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcAudioChannel;
    }
    else
    {
        transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    }
    transcodeInputEsStream.audioEncodeParams.audioDummy = g_dvrTest->platformconfig.outputs.audioDummy[pathIndex];
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&playbackMedia.esStreamInfo[1],sizeof(playbackMedia.esStreamInfo[1]));
    /* add audio ES stream*/
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>");
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodeInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    /* add pcr ES stream*/
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings >>>");
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings >>>");
    transcodeServiceSettings.displaySettings.format = NEXUS_VideoFormat_e720p24hz;
    transcodeServiceSettings.stgSettings.enabled = true;
    if(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
    {
        transcodeServiceSettings.stgSettings.nonRealTime = true;
    }
    else
    {
        transcodeServiceSettings.stgSettings.nonRealTime = false;
    }
    transcodeServiceSettings.madSettings.deinterlace = true;
    transcodeServiceSettings.madSettings.enable22Pulldown = true;
    transcodeServiceSettings.madSettings.enable32Pulldown = true;
    transcodeServiceSettings.videoEncodeSettings.bitrateMax = 10*1024*1024;
    transcodeServiceSettings.videoEncodeSettings.frameRate = NEXUS_VideoFrameRate_e23_976;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesB = 0;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesP = 23;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.trackInput = false;
    transcodeServiceSettings.videoEncodeSettings.enableFieldPairing = true;
    
    B_DVR_TranscodeService_SetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open >>>");
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName,
                  B_DVR_MAX_FILE_NAME_LENGTH,"%s%s",
                  TRANSCODE_SERVICE_PREFIX,
                  g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"transcode");
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.segmented = true;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = B_DVR_RecordServiceInputTranscode;

    /*Open record service*/
    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].recordService)
    {
        printf("record service open for transcode operation failed");
        goto error_recordService;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback >>>");
    B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings >>>");
    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.parserBand  = NEXUS_ANY_ID;
    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start >>>");
    /* start transcode */
    B_DVR_TranscodeService_Start(g_dvrTest->streamPath[pathIndex].transcodeService);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid >>>");
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[0].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    recordInputEsStream.esStreamInfo.codec.videoCodec = NEXUS_VideoCodec_eH264; /*playbackMedia.esStreamInfo[0].codec.videoCodec*/;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[0].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[0].pid);
        goto error_transcodePidChannel;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid >>>");
    recordInputEsStream.pidChannel=transcodePidChannel;
    /*Add transcoded video ES to the record*/
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid >>>");
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    recordInputEsStream.esStreamInfo.codec.audioCodec = NEXUS_AudioCodec_eAac; /*playbackMedia.esStreamInfo[1].codec.audioCodec;*/
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid);
        goto error_transcodePidChannel;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid >>>");
    recordInputEsStream.pidChannel=transcodePidChannel;
    /*Add transcoded audio ES to the record*/
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid >>>");
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid+1);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid+1);
        goto error_transcodePidChannel;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid >>>");
    recordInputEsStream.pidChannel=transcodePidChannel;
    /*Add transcoded audio ES to the record*/
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid <<<");
    

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start >>>");
    /* start recording */
    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_PlaybackDecodeStart >>>");
    /* start playback decode */
	g_dvrTest->streamPath[pathIndex].drmService[drmIndex] = NULL;
    DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_PlaybackDecodeStart <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start >>>");
    /* start playback */
    B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start <<<");
    
    printf("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent >>>");
    g_dvrTest->streamPath[pathIndex].transcodeFinishEvent = NULL;
    BKNI_CreateEvent(&g_dvrTest->streamPath[pathIndex].transcodeFinishEvent);
    if(!g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
    {
        printf("\n transcodeFinishEvent create failed");
        goto error_transcodeFinishEvent;
    }
    printf("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent <<<");
    return rc;

error_transcodeFinishEvent:
error_transcodePidChannel:
    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
error_recordService:
    B_DVR_TranscodeService_Close(g_dvrTest->streamPath[pathIndex].transcodeService);
error_transcodeService:
    NEXUS_StcChannel_Close(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel);
error_transcodeStcChannel:
    B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
error_playbackService:
    return rc;
}

B_DVR_ERROR DvrTest_FileToFileTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;

    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings >>>");
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop >>>");
    DvrTest_PlaybackDecodeStop(pathIndex);
    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Stop >>>");
    B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Stop <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop >>>");
    B_DVR_TranscodeService_Stop(g_dvrTest->streamPath[pathIndex].transcodeService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop <<<");
    /* wait for the encoder buffer model's data to be drained */
    printf("\n DvrTest_FileToFileTranscodeStop : BKNI_WaitForEvent >>>");
    if(BKNI_WaitForEvent(g_dvrTest->streamPath[pathIndex].transcodeFinishEvent,
                         (transcodeServiceSettings.videoEncodeSettings.encoderDelay/27000)*2)!=BERR_SUCCESS) 
    {
        printf("\n Transcode Finish timeout");
    }
    printf("\n DvrTest_FileToFileTranscodeStop : BKNI_WaitForEvent <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop >>>");
    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop <<<");
    /* B_DVR_RecordService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].recordService);*/
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback >>>");
    B_DVR_RecordService_RemoveCallback(g_dvrTest->streamPath[pathIndex].recordService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close >>>");
    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_RemoveCallback >>>");
    B_DVR_PlaybackService_RemoveCallback(g_dvrTest->streamPath[pathIndex].playbackService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_RemoveCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Close >>>");
    B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_PlaybackService_Close <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback >>>");
    B_DVR_TranscodeService_RemoveCallback(g_dvrTest->streamPath[pathIndex].transcodeService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams >>>");
    B_DVR_TranscodeService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].transcodeService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close >>>");
    B_DVR_TranscodeService_Close(g_dvrTest->streamPath[pathIndex].transcodeService);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close >>>");
    NEXUS_StcChannel_Close(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel);
    printf("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close <<<");
    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_LiveDecodeStart >>>");
    g_dvrTest->streamPath[pathIndex].transcodeService = NULL;
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest));
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTest_LiveDecodeStart(pathIndex);
    }
    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_LiveDecodeStart <<<");
    return rc;
}

B_DVR_ERROR DvrTest_QamToFileTranscodeStart(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

B_DVR_ERROR DvrTest_QamToFileTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

B_DVR_ERROR DvrTest_QamToTsbTranscodeStart(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

B_DVR_ERROR DvrTest_QamToTsbTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

B_DVR_ERROR DvrTest_QamToBufferTranscodeStart(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

B_DVR_ERROR DvrTest_QamToBufferTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BSTD_UNUSED(pathIndex);
    return rc;
}

int main(void)
{
    int decoderIndex,frontendIndex, pathIndex, operation;
    int i;
	NEXUS_DmaHandle dma;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    DvrTest_Operation dvrTestOp;
	
    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));
    
    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(NULL);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);

    while (1) 
    {
        unsigned volumeIndex;
        DvrTest_MediaStorageMenu();
        fflush(stdin);
        operation=0;
        scanf("%d", &operation);
        if(!operation)
        {
            break;
        }
        switch (operation) 
        {
        case 1:
            {
                B_DVR_MediaStorage_FormatVolume(g_dvrTest->mediaStorage,volumeIndex);
            }
            break;
        case 2:
            {  
                sprintf(g_dvrTest->mediaStorageRegisterSettings.device,"/dev/sda");
                g_dvrTest->mediaStorageRegisterSettings.startSec = 0;
                g_dvrTest->mediaStorageRegisterSettings.length = 0;
                if (B_DVR_MediaStorage_RegisterVolume(g_dvrTest->mediaStorage,&g_dvrTest->mediaStorageRegisterSettings,&volumeIndex) >0)
                    printf("\n Register media volume error");
                else
                    printf("\n Register media volume success vol:%d",volumeIndex);
            }
            break;
        case 3:
            {
                printf("Volume number to unregister: ");
                fflush(stdin);
                volumeIndex=0;
                printf("\n Enter volume index: ");
                scanf("%u", &volumeIndex);
                printf("volume %u",volumeIndex);
                if (B_DVR_MediaStorage_UnregisterVolume(g_dvrTest->mediaStorage,volumeIndex) >0)
                    printf("Unregister error\n");
                else
                    printf("Unregister success\n");
            }
            break;
        case 4:
            {
                B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);     
                printf("Number of registered volumes:%d\n", mediaStorageStatus.numRegisteredVolumes);
                printf("Number of mounted volumes:%d\n", mediaStorageStatus.numMountedVolumes);
                for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) 
                {
                    if (!mediaStorageStatus.volumeInfo[i].registered) 
                        continue;
                    printf("Volume %d ",i);
                    printf("\n %s", mediaStorageStatus.volumeInfo[i].mounted?"Mounted":"Not Mounted");
                    printf("\n\tdevice: %s", mediaStorageStatus.volumeInfo[i].device);
                    printf("\n\tname: %s", mediaStorageStatus.volumeInfo[i].name);
                    printf("\n\tmedia segment size: %d\n", mediaStorageStatus.volumeInfo[i].mediaSegmentSize);
               }
            }
               break;
        case 5:
            {
                fflush(stdin);
                volumeIndex=0;
                printf("\n Enter volume index : ");
                scanf("%u", &volumeIndex);
                B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);
            }
            break;
        case 6:
            {
                fflush(stdin);
                volumeIndex=0;
                printf("\n Enter volume index : ");
                scanf("%u", &volumeIndex);
               B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage,volumeIndex);
            }
            break;
        case 7:
            {
                char path[B_DVR_MAX_FILE_NAME_LENGTH];
                fflush(stdin);
                printf("\n Enter volume index : ");
                scanf("%u", &volumeIndex);
                B_DVR_MediaStorage_GetMetadataPath(g_dvrTest->mediaStorage,volumeIndex, path);
                printf("volume %u metadata path: %s", volumeIndex, path);
            }
            break;
        case 8:
            {
                B_DVR_MediaStorageStatus status;
                fflush(stdin);
                printf("\n Enter volume index : ");
                scanf("%u", &volumeIndex);
                B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&status);                
                ms_dump_volume_status(status.volumeInfo[volumeIndex].mountName);
            }
            break;

        default:
            break;
    
        }
    }

    DvrTest_PrintChannelMap();
    g_dvrTest->maxChannels = MAX_STATIC_CHANNELS;
    g_dvrTest->streamPath[0].currentChannel = 0;
    if(NEXUS_NUM_VIDEO_DECODERS > 1)
    g_dvrTest->streamPath[1].currentChannel= 1;

    B_Os_Init();

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);
	dma = NEXUS_Dma_Open(0, NULL);	
    for (frontendIndex=0,i=0;frontendIndex < MAX_STREAM_PATH; frontendIndex++ )
    {
        g_dvrTest->streamPath[frontendIndex].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
        if (g_dvrTest->streamPath[frontendIndex].frontend)
        {
            NEXUS_Frontend_GetCapabilities(g_dvrTest->streamPath[frontendIndex].frontend, &g_dvrTest->streamPath[frontendIndex].capabilities);
            if (g_dvrTest->streamPath[frontendIndex].capabilities.qam)
            {
                g_dvrTest->streamPath[i].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
                NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[i].frontend, &g_dvrTest->streamPath[i].qamStatus);
                DvrTest_TuneChannel(i);
                g_dvrTest->streamPath[i].liveDecodeStarted= false;
                i++;
            }
        }
    }


    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);

    for(decoderIndex=0;decoderIndex < MAX_AV_PATH; decoderIndex++)
    {
        DvrTest_AudioVideoPathOpen(decoderIndex);
        DvrTest_LiveDecodeStart(decoderIndex);
        g_dvrTest->streamPath[decoderIndex].liveDecodeStarted = true;
        g_dvrTest->streamPath[decoderIndex].playbackDecodeStarted = false;
    }

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTest_Callback;
    g_dvrTest->dvrTestDrmCallback = (B_DVR_DRMService_KeyLoaderCallback)DvrTestDrmCallback;
    g_dvrTest->dvrManager = B_DVR_Manager_Init(NULL);
    if(!g_dvrTest->dvrManager)
    {
        printf("Error in opening the dvr manager\n");
    } 

    B_DVR_Manager_CreateMediaNodeList(g_dvrTest->dvrManager,0);

    for(pathIndex=0;pathIndex<MAX_AV_PATH;pathIndex++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,pathIndex);
        rc = B_DVR_Manager_AllocSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings,
                                                    MAX_TSB_BUFFERS);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in allocating the tsb segments");
        }
    }

    for(pathIndex=0;pathIndex<MAX_AV_PATH;pathIndex++)
    {
        B_DVR_ERROR rc =0;
        char programName[B_DVR_MAX_FILE_NAME_LENGTH];
        char subDir[B_DVR_MAX_FILE_NAME_LENGTH]="tsb";
        B_DVR_MediaNodeSettings mediaNodeSettings;
        BKNI_Memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
        mediaNodeSettings.programName = programName;
        mediaNodeSettings.subDir = subDir;
        mediaNodeSettings.volumeIndex=0;
        mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,pathIndex);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }

        while(1)
    { 
        DvrTest_Menu();
        fflush(stdin);
        operation=0;
        pathIndex=0;
        scanf("%d", &operation);
        dvrTestOp = (DvrTest_Operation)operation; 
        if(dvrTestOp > eDvrTest_OperationQuit && operation < eDvrTest_OperationMax)
        {
            printf("\n enter streamPath Index for %d : ",operation);
            fflush(stdin);
            scanf("%d",&pathIndex);
         }

        if (!operation)
        {
            break;
        }
        if(!((dvrTestOp == eDvrTest_OperationRecordServiceStart) || (dvrTestOp == eDvrTest_OperationRecordServiceStop)))
        {
            if(pathIndex > MAX_AV_PATH)
            {
                printf("\n AV path is restricted to 0 and 1");
                continue;
            }
        }
        else
        {
            if(pathIndex >= MAX_STREAM_PATH)
            {
                printf("\n back ground recording can in the path 2-5");
                continue;
            }
        }
            switch(dvrTestOp)
            {
                case eDvrTest_OperationChannelChange:
                {
                    int dir=0;
                    printf("\n Channel Up-1 Down 0 : ");
                    fflush(stdin);
                    scanf("%d",&dir);
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        printf("\n Invalid operation! Stop the timeshift srvc on this channel");
                        goto case1_error;
                    }
                    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                    {
                        DvrTest_LiveDecodeStop(pathIndex);
                    }
                    if(dir)
                    {
                        g_dvrTest->streamPath[pathIndex].currentChannel++;
                        g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
                        printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
                    }
                    else
                    {
                        g_dvrTest->streamPath[pathIndex].currentChannel--;
                        g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
                        printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
                    }
                    DvrTest_TuneChannel(pathIndex);
                    DvrTest_LiveDecodeStart(pathIndex);
                    g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
                }
                case1_error:
                break;  
               case eDvrTest_OperationTSBServiceStartGenericClearKey:
               {
			   	B_DVR_TSBServiceInputEsStream inputEsStream;
                B_DVR_TSBServiceSettings tsbServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;
                B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                {
                    DvrTest_LiveDecodeStart(pathIndex);
                }
                printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
                BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
                              sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest),
                              "%s%d",TSB_SERVICE_PREFIX,pathIndex);
                printf("\n TSBService programName %s\n",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);                
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
                sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
                g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
				B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

	            BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
                dataInjectionOpenSettings.fifoSize = 30*188;
                g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
				tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
				tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
				B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

                g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
                g_dvrTest->streamPath[pathIndex].drmService[0] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
                g_dvrTest->streamPath[pathIndex].drmService[1] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                
                printf("\nDRM Service Opened\n");           

                BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
	            inputEsStream.pidChannel = NULL;
                B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);
    
                videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,inputEsStream.esStreamInfo.pid);

                inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);

                audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,inputEsStream.esStreamInfo.pid);
                
                B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

                g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
                g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
                g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
                dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[0] ,&g_dvrTest->drmServiceSettings);

				g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[1] ,&g_dvrTest->drmServiceSettings);

                mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
                mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
                mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
                mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
                BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                        mediaNodeSettings.programName,
                                                                        mediaNodeSettings.volumeIndex));

                dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);              
                dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                    &mediaNodeSettings,
                                                                    channelInfo[currentChannel].videoPids[0],
                                                                    changeToLocalKey);
                
				dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);				
				dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
																	&mediaNodeSettings,
																	channelInfo[currentChannel].audioPids[0],
																	changeToLocalKey2);


                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);
                
#if 0

                BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
                tsbServiceSettings.tsbRecordSettings.esStreamCount =2;
                tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[0].pid = 0x1ff2;
                tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[1].pid = 0x1ff3;
                B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);
#endif
                dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("\n Error in starting the tsbService %d",pathIndex);
                }
                else
                {
                    printf("tsbService started %d\n",pathIndex);
                }

            }
            case2_error:
            break;
    
            case eDvrTest_OperationTSBServiceStopGenericClearKey:
            {
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;    
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                
                videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                      channelInfo[currentChannel].videoPids[0]);    

                audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                      channelInfo[currentChannel].audioPids[0]);    

                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {   
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);
                    
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[0]);
                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[1]);
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
                    
                    B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);                   
                    dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
                    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                    g_dvrTest->streamPath[pathIndex].tsbService = NULL;
                    g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in closing the timeshift service %d\n",pathIndex);
                    }

                }
                else
                {
                    printf("\n timeshift srvc not started");
                }
            }
            
            break;

            case eDvrTest_OperationTSBServicePlayStart:
                {
					int drmIndex;
                    NEXUS_PidChannelHandle videoPidchHandle;
                    NEXUS_PidChannelHandle audioPidchHandle;    
                    printf("pathIndex = %d",pathIndex);
                    if(!g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        printf("\n error TSBPlayStart without TSBServiceStart");
                        break;
                    }
                    else
                    {
                        B_DVR_OperationSettings operationSettings;
                        B_DVR_TSBServiceStatus tsbServiceStatus;
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
                        operationSettings.operation = eB_DVR_OperationTSBPlayStart;
                        operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
            
                        if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                        {
                            unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                            
                            videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                                  channelInfo[currentChannel].videoPids[0]);    
                            
                            audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                                  channelInfo[currentChannel].audioPids[0]);    

                            B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
                            B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);

                            DvrTest_LiveDecodeStop(pathIndex);

                            B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[0]);
                            B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[1]);
                            BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
                        }

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
                        g_dvrTest->streamPath[pathIndex].drmService[5] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmService[6] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        printf("\nDRM Service for tsb playback Opened\n");
                        g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                        g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                        g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[5] ,&g_dvrTest->drmServiceSettings);

                        g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[6] ,&g_dvrTest->drmServiceSettings);
                        printf("\nDRM Service for tsb playback Configured\n");              
						drmIndex = 5;
                        DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
                        
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                        operationSettings.operation = eB_DVR_OperationPause; 
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);

                   }
            
            
                }
                break;

            case eDvrTest_OperationTSBServiceStartVendorSpecificKey:
            {
			   	B_DVR_TSBServiceInputEsStream inputEsStream;				
                B_DVR_TSBServiceSettings tsbServiceSettings;
                NEXUS_KeySlotHandle keyslot;
                NEXUS_KeySlotHandle keyslot2;
                NEXUS_SecurityKeySlotSettings keySlotSettings;
                B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                {
                    DvrTest_LiveDecodeStart(pathIndex);
                }
                printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
                BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
                              sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest),
                              "%s%d",TSB_SERVICE_PREFIX,pathIndex);
                printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);              

                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
                sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
                g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
                g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("Error in opening timeshift service %d\n",pathIndex);
                    goto case2_error;
                }

                B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

	            BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
                dataInjectionOpenSettings.fifoSize = 188*30;
				tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
				tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;

                NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
                keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp;

                keyslot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);  
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = keyslot; /* application provided keyhandle */
                g_dvrTest->streamPath[pathIndex].drmService[3] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("Error in opening drm service %d\n",pathIndex);
                    goto case2_error;
                }

                keyslot2 = NEXUS_Security_AllocateKeySlot(&keySlotSettings);  
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = keyslot2; /* application provided keyhandle */
                g_dvrTest->streamPath[pathIndex].drmService[4] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("Error in opening drm service 2 %d\n",pathIndex);
                    goto case2_error;
                }

                B_DVR_DRMService_InstallKeyloaderCallback(g_dvrTest->streamPath[pathIndex].drmService[3],g_dvrTest->dvrTestDrmCallback,(void*)keyslot);
                B_DVR_DRMService_InstallKeyloaderCallback(g_dvrTest->streamPath[pathIndex].drmService[4],g_dvrTest->dvrTestDrmCallback,(void*)keyslot2);

		  /*********  
		   Vendor can use this API to save the keyBlob in customer specific portion 
                B_DVR_Manager_SetVendorSpecificMetaData(B_DVR_ManagerHandle manager,B_DVR_MediaNodeSettings * mediaNodeSettings,void * vendorData,unsigned size)
***********/                

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[3],g_dvrTest->streamPath[pathIndex].videoPidChannels[0]);
                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[4], g_dvrTest->streamPath[pathIndex].audioPidChannels[0]);
                

                BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);
                
                inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);
                
                BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
                tsbServiceSettings.tsbRecordSettings.esStreamCount =2;
                tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[0].pid = 0x1ff2;
                tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[1].pid = 0x1ff3;
                B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

                dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
                if(dvrError!=B_DVR_SUCCESS)
                {
                    printf("\n Error in starting the tsbService %d",pathIndex);
                }
                else
                {
                    printf("tsbService started %d\n",pathIndex);
                }               
            }
            break;

            
            case eDvrTest_OperationTSBServiceStopVendorSpecificKey:
            {
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;    
                
                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {   

                    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                    
                    videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                          channelInfo[currentChannel].videoPids[0]);    
                    
                    audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
                                                                          channelInfo[currentChannel].audioPids[0]);

                
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[3],videoPidchHandle);
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[4],audioPidchHandle);

                    
                    if(dvrError!=B_DVR_SUCCESS)
                    {
                        printf("\n Error in removing keyslot drmservice2");                 
                    }

                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
                    dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
                    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                    g_dvrTest->streamPath[pathIndex].tsbService = NULL;
                    g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));

                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in closing the timeshift service %d\n",pathIndex);
                    }

                    B_DVR_DRMService_RemoveKeyloaderCallback(g_dvrTest->streamPath[pathIndex].drmService[3]);
                    B_DVR_DRMService_RemoveKeyloaderCallback(g_dvrTest->streamPath[pathIndex].drmService[4]);       
                    
                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[3]);
                    if(dvrError!=B_DVR_SUCCESS)
                    {
                        printf("\n Error in invalidating keyslot drmservice");                  
                    }               
                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[4]);                              
                    if(dvrError!=B_DVR_SUCCESS)
                    {
                        printf("\n Error in invalidating keyslot drmservice2");                 
                    }
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));


                }
                else
                {
                    printf("\n timeshift srvc not started");
                }

            }
            break;

             case eDvrTest_OperationTSBServiceConvertStart:
            {
                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {
                    B_DVR_ERROR rc = B_DVR_SUCCESS;
                    B_DVR_TSBServiceStatus tsbStatus;
                    B_DVR_TSBServicePermanentRecordingRequest recReq;

                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService, &tsbStatus);
                    printf("\n current tsb Rec start %ld ms end %ld ms",tsbStatus.tsbRecStartTime, tsbStatus.tsbRecEndTime);
                    fflush(stdin);
                    printf("\n Enter perm rec program names :");
                    scanf("%s",recReq.programName);
                    printf("\n Enter start time:");
                    scanf("%lu",&recReq.recStartTime);
                    printf("\n Enter end time: ");
                    scanf("%lu",&recReq.recEndTime);
                    printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
                    sprintf(recReq.subDir,"tsbConv");
                     rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[pathIndex].tsbService,&recReq);
                    if(rc!=B_DVR_SUCCESS)
                    {
                        printf("\n Invalid paramters");
                    }
                }
                else
                {
                    printf("\n tsb service not start for path %d",pathIndex);
                }

            }
            break;
        case eDvrTest_OperationTSBServiceConvertStop:
            {
               B_DVR_ERROR rc = B_DVR_SUCCESS;

               rc = B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[pathIndex].tsbService);
               if(rc!=B_DVR_SUCCESS)
               {
                   printf("\n Failed to stop conversion");
               }
            }
            break;
        case eDvrTest_OperationPlaybackServiceStart:
            {
                 B_DVR_MediaNodeSettings mediaNodeSettings; 
                 B_DVR_Media mNode;
                 unsigned keylength;
                 unsigned char vKey[16];
                 unsigned char aKey[16];
                 int count,drmIndex;
                 
                 if(g_dvrTest->streamPath[pathIndex].tsbService)
                 {
                     printf("\n Stop TSB service running on this path %d",pathIndex);
                     goto error_playbackService;
                 }

                 fflush(stdin);
                 printf("\n Enter the sub folder name for recording to be playedback");
                 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
                 printf("\n Enter the recording name");
                 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
                 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
                 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;

                 mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
                 mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
                 mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
                 mediaNodeSettings.volumeIndex = 0;
                 
                 B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);
     
                 B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);

                 B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,vKey);
                 B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[1].pid,aKey);

				 printf("keylength = %d\n",keylength);
                 for(count=0;count<(int)keylength;count++)
                 {
                    printf("[%d] vKey[0x%x] , aKey[0x%x]\n",count,vKey[count],aKey[count]);
                 }

                 g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
                 printf("\n B_DVR_PlaybackService_Open");
                 
                 if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                 {
                     DvrTest_LiveDecodeStop(pathIndex);              
                 }

                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
				 g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
                 g_dvrTest->streamPath[pathIndex].drmService[5] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);            

                 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
                 g_dvrTest->streamPath[pathIndex].drmService[6] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);            

                 printf("\nDRM Service for tsb playback Opened\n");
                 g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                 g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                 g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
                 g_dvrTest->drmServiceSettings.keys = vKey;
                 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[5] ,&g_dvrTest->drmServiceSettings);

                 g_dvrTest->drmServiceSettings.keys = aKey;
                 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[6] ,&g_dvrTest->drmServiceSettings);
                 printf("\nDRM Service for tsb playback Configured\n");              

				 drmIndex = 5;
                 printf("\n DvrTest_LiveDecodeStop");
                 DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
                 printf("\n DvrTest_PlaybackDecodeStart");
                 B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
                 printf("\n B_DVR_PlaybackService_Start");
                 
            }
            error_playbackService:
            break;
            case eDvrTest_OperationPause:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationPause;
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                    }
                    
                }
            }
            break;  
            
            case eDvrTest_OperationPlaybackServiceStop:
            {
                B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[5],
                                                       g_dvrTest->streamPath[pathIndex].videoPlaybackPidChannels[0]);
                
                B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[6],
                                                       g_dvrTest->streamPath[pathIndex].audioPlaybackPidChannels[0]);
                 
                 B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
                 DvrTest_PlaybackDecodeStop(pathIndex);
                 
                 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[5]);
                 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[6]);
                 
                 B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
                 g_dvrTest->streamPath[pathIndex].playbackService = NULL;

                 if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                 {
                     DvrTest_LiveDecodeStart(pathIndex);
                 }

            }
            break;
            case eDvrTest_OperationRecordServiceStart:
                {
                    if((pathIndex > 0 && pathIndex < 2 ))
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						B_DVR_RecordServiceInputEsStream inputEsStream;										
                        B_DVR_RecordServiceSettings recordServiceSettings;
                        NEXUS_PidChannelHandle videoPidchHandle;
                        NEXUS_PidChannelHandle audioPidchHandle;    
                        B_DVR_MediaNodeSettings mediaNodeSettings;
                        B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                        fflush(stdin);
                        printf("\n Enter recording name:");
                        scanf("%s",g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);
            
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                        sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
					    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;						
						
                        g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                        printf("\n recordService open <<");

                        B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
                        printf("\n recordService install callback <<");

						BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
						recordServiceSettings.parserBand  = g_dvrTest->streamPath[pathIndex].parserBand;
						dataInjectionOpenSettings.fifoSize = 30*188;
						g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
						recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
						B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);

                        BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                        inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
                        videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);
                   
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                        inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
                        audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);                      

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
                        g_dvrTest->streamPath[pathIndex].drmService[7] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        if(dvrError!=B_DVR_SUCCESS)
                        {
                            printf("Error in opening drm service 5%d\n",pathIndex);
                        }

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmService[8] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        if(dvrError!=B_DVR_SUCCESS)
                        {
                            printf("Error in opening drm service 6 %d\n",pathIndex);
                        }

                        printf("\nDRM Service for back ground Opened\n");   
                        
                        g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                        g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                        g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[7] ,&g_dvrTest->drmServiceSettings);
                            
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[8] ,&g_dvrTest->drmServiceSettings);

                        mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
                        mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
                        mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
                        mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
                        BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                                mediaNodeSettings.programName,
                                                                                mediaNodeSettings.volumeIndex));
                        
                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].videoPids[0],
                                                                            changeToLocalKey);
                        
                        
                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].audioPids[0],
                                                                            changeToLocalKey2);
                        
                        
                        dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[7],videoPidchHandle);                 
                        dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[8],audioPidchHandle);

                        BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
                        recordServiceSettings.esStreamCount = 2;
                        recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                        recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                        B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
                        printf("\n recordService start >>");
                        B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                        printf("\n recordService start <<");
                    
                     }
                 }
                 break;
            case eDvrTest_OperationRecordServiceStop:
                {
                    NEXUS_PidChannelHandle videoPidchHandle;
                    NEXUS_PidChannelHandle audioPidchHandle;    
                    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                    
                    if(pathIndex > 0 && pathIndex < 2 )
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);

                        videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                                  channelInfo[currentChannel].videoPids[0]);
                        audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                                  channelInfo[currentChannel].audioPids[0]);

                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[7],videoPidchHandle);

                
                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[8],audioPidchHandle);
        

                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[7]);
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[8]);
                        
                        B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                        g_dvrTest->streamPath[pathIndex].recordService = NULL;
                        g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
                        
                    }
                }
                break;
            
            
            case eDvrTest_OperationPlay:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationPlay;
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                    }
                    
                }
                                
            }
            break;
            case eDvrTest_OperationRewind:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationFastRewind;
                    operationSettings.operationSpeed = 4;
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                    } 
                }
            }
            break;
            case eDvrTest_OperationFastForward:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationFastForward;
                    operationSettings.operationSpeed = 4;
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                    } 
                }
            }
            break;
            
            case eDvrTest_OperationSlowRewind:
                {
                    B_DVR_OperationSettings operationSettings;
                    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                    {
                        DvrTest_LiveDecodeStop(pathIndex);
                    }
#if 0					
                    if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStart(pathIndex);
            
                    }
#endif
                    operationSettings.operation = eB_DVR_OperationSlowRewind;
                    operationSettings.operationSpeed = 4;
            
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        if(g_dvrTest->streamPath[pathIndex].playbackService)
                        {
                            B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                        }
                        else
                        {
                            printf("\n Invalid operation since neither playback nor tsb service started");
                            DvrTest_LiveDecodeStart(pathIndex);
                        }
                        
                    }
                }
                break;
                
                case eDvrTest_OperationSlowForward:
                    {
                        if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                        {
                            printf("\n playback not started. Start either TSB playback or linear playback");
                            break;
                        }
                        else
                        {
                            B_DVR_OperationSettings operationSettings;
                            operationSettings.operation = eB_DVR_OperationSlowForward;
                            operationSettings.operationSpeed = 4;
                            if(g_dvrTest->streamPath[pathIndex].tsbService)
                            {
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                            }
                            else
                            {
                                B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                            } 
                        }
                    }
                break;
                case eDvrTest_OperationFrameAdvance:
                    {
                        if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                        {
                            printf("\n playback not started. Start either TSB playback or linear playback");
                            break;
                        }
                        else
                        {
                            B_DVR_OperationSettings operationSettings;
                            operationSettings.operation = eB_DVR_OperationForwardFrameAdvance;
                            if(g_dvrTest->streamPath[pathIndex].tsbService)
                            {
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                            }
                            else
                            {
                                B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                            } 
                        }
                    }
                    break;
                case eDvrTest_OperationFrameReverse:
                    {
                        if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                        {
                            printf("\n playback not started. Start either TSB playback or linear playback");
                            break;
                        }
                        else
                        {
                            B_DVR_OperationSettings operationSettings;
                            operationSettings.operation = eB_DVR_OperationReverseFrameAdvance;
                            if(g_dvrTest->streamPath[pathIndex].tsbService)
                            {
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                            }
                            else
                            {
                                B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                            } 
                        }
                    }
                    break;
				case eDvrTest_OperationPlaybackServiceStartM2m:
					{
						B_DVR_MediaNodeSettings mediaNodeSettings; 
						B_DVR_Media mNode;
						unsigned keylength;
						unsigned char key[16];
						int count;
						int drmIndex;
						
						if(g_dvrTest->streamPath[pathIndex].tsbService)
						{
							printf("\n Stop TSB service running on this path %d",pathIndex);
							goto error_playbackService;
						}
						
						fflush(stdin);
						printf("\n Enter the sub folder name for recording to be playedback");
						scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
						printf("\n Enter the recording name");
						scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
						g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
						g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
						
						mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
						mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
						mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
						mediaNodeSettings.volumeIndex = 0;
						
						B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);
						B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);
						B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);
						
						for(count=0;count<(int)keylength;count++)
						{
						   BDBG_MSG(("[%d] vKey[0x%x] \n",count,key[count]));
						}
						
						g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
						printf("\n B_DVR_PlaybackService_Open");
						
						if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
						{
							DvrTest_LiveDecodeStop(pathIndex);				
						}

						g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = dma;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
						g_dvrTest->streamPath[pathIndex].drmService[9] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
						printf("\nDRM Service for playback Opened\n");

						g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
						g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
						g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
						g_dvrTest->drmServiceSettings.keys = key;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
						dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[9] ,&g_dvrTest->drmServiceSettings);
						
						printf("\nDRM Service for tsb playback Configured\n");				
						drmIndex =9;
						printf("\n DvrTest_LiveDecodeStop");
						DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
						printf("\n DvrTest_PlaybackDecodeStart");
						B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
						printf("\n B_DVR_PlaybackService_Start");
						
				    }

					break;
				case eDvrTest_OperationPlaybackServiceStopM2m:
				{
						 
						 B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
						 B_DVR_PlaybackService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].playbackService);
						 
						 DvrTest_PlaybackDecodeStop(pathIndex);						 
						 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[9]);
						 
						 B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
						 g_dvrTest->streamPath[pathIndex].playbackService = NULL;
						
						 if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
						 {
							 DvrTest_LiveDecodeStart(pathIndex);
						 }

				}
				break;
				
				case eDvrTest_OperationRecordServiceStartM2m:
				{
					if((pathIndex >= 0 && pathIndex < 2 ))
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						B_DVR_RecordServiceInputEsStream inputEsStream;						
                        B_DVR_RecordServiceSettings recordServiceSettings;
                        B_DVR_MediaNodeSettings mediaNodeSettings;
                        B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                        fflush(stdin);
                        printf("\n Enter recording name:");
                        scanf("%s",g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);
            
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                        sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
                        g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                        printf("\n recordService open <<");

                        B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
                        printf("\n recordService install callback <<");

	                    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
	                    recordServiceSettings.parserBand  = g_dvrTest->streamPath[pathIndex].parserBand;
	                    dataInjectionOpenSettings.fifoSize = 30*188;
	                    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
	                    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
	                    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);						

                        BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                        inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
					
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                        inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);					

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = dma;
                        g_dvrTest->streamPath[pathIndex].drmService[10] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                        g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                        g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[10] ,&g_dvrTest->drmServiceSettings);

						
                        mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
                        mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
                        mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
                        mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
                        BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                                mediaNodeSettings.programName,
                                                                                mediaNodeSettings.volumeIndex));

                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].videoPids[0],
                                                                            changeToLocalKey);
                        

                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].audioPids[0],
                                                                            changeToLocalKey);


						B_DVR_RecordService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].recordService,
															g_dvrTest->streamPath[pathIndex].drmService[10]);

                        BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
                        recordServiceSettings.esStreamCount = 2;
                        recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                        recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                        B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);

                        B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                        printf("\n recordService start <<");
                    
                     }

				}
				break;
				
				case eDvrTest_OperationRecordServiceStopM2m:
				{                    
                    
                    if(pathIndex > 0 && pathIndex < 2 )
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
						B_DVR_RecordService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].recordService);
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[10]);
                        
                        B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                        g_dvrTest->streamPath[pathIndex].recordService = NULL;
                        g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
                    }
				}	
				break;				
            	case eDvrTest_OperationTSBServiceStartGenericClearKeyM2m:
			   {
				B_DVR_TSBServiceInputEsStream inputEsStream;				
				B_DVR_TSBServiceSettings tsbServiceSettings;
				B_DVR_MediaNodeSettings mediaNodeSettings;
				B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
				int tsbPlayback = false;
				
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(pathIndex);
				}
				printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
				BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
				BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
							  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest),
							  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
				printf("\n TSBService programName %s\n",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);				
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
				sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
				g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
				B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

				BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
				dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
				g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
				tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
				tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
				B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = dma;
				g_dvrTest->streamPath[pathIndex].drmService[12] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				

				printf("\nDRM Service Opened\n");			

				BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
				inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
				inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
				inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
	            inputEsStream.pidChannel = NULL;				
				B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);
				
				inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
				inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
				inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
				B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);

				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
				g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[12] ,&g_dvrTest->drmServiceSettings);

				mediaNodeSettings.recordingType = eB_DVR_RecordingTSB;
				mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
				mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
				mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
				BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
																		mediaNodeSettings.programName,
																		mediaNodeSettings.volumeIndex));

				dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);				
				dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
																	&mediaNodeSettings,
																	channelInfo[currentChannel].videoPids[0],
																	changeToLocalKey);
				
				dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,KEY_LENGTH);				
				dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
																	&mediaNodeSettings,
																	channelInfo[currentChannel].audioPids[0],
																	changeToLocalKey);

				dvrError = B_DVR_TSBService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,
																g_dvrTest->streamPath[pathIndex].drmService[12],tsbPlayback);
#if 0
				BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
            	tsbServiceSettings.tsbRecordSettings.esStreamCount =2;
            	tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[0].pid = 0x1ff2;
            	tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[1].pid = 0x1ff3;
            	B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);
#endif
				dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
				if(dvrError!=B_DVR_SUCCESS)
				{
					printf("\n Error in starting the tsbService %d",pathIndex);
				}
				else
				{
					printf("tsbService started %d\n",pathIndex);
				}

			}
				break;

				case eDvrTest_OperationTSBServiceStopGenericClearKeyM2m:
				{
					 bool tsbPlayback=false;
					 if(g_dvrTest->streamPath[pathIndex].tsbService)
					 {	 
						 
						 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
						 dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);
						 
						 if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
						 {
							 DvrTest_PlaybackDecodeStop(pathIndex);
						 }
						 if(dvrError != B_DVR_SUCCESS)
						 {
							 printf("\n Error in stopping the timeshift service %d\n",pathIndex);
						 }
				
						 dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[12]);
						 BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
						 
						 B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);					 
						 dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
						 B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
						 g_dvrTest->streamPath[pathIndex].tsbService = NULL;
						 g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
						 BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
						 if(dvrError != B_DVR_SUCCESS)
						 {
							 printf("\n Error in closing the timeshift service %d\n",pathIndex);
						 }
				
					 }
					 else
					 {
						 printf("\n timeshift srvc not started");
					 }
				 }

				break;
				
				case eDvrTest_OperationPlaybackServiceStartKeyladderM2m:
				{
					B_DVR_MediaNodeSettings mediaNodeSettings; 
					B_DVR_Media mNode;
					unsigned keylength;
					unsigned char key[32];
					unsigned char session[16];
					unsigned char controlWd[16];
					unsigned char iv[16];
					int count;
					int drmIndex;
					
					if(g_dvrTest->streamPath[pathIndex].tsbService)
					{
						printf("\n Stop TSB service running on this path %d",pathIndex);
						goto error_playbackService;
					}
					
					fflush(stdin);
					printf("\n Enter the sub folder name for recording to be playedback");
					scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
					printf("\n Enter the recording name");
					scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
					g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
					mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
					mediaNodeSettings.volumeIndex = 0;
					
					B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);
					
					B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);
					
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);
					
					for(count=0;count<(int)(keylength);count++)
					{
					   BDBG_MSG(("[%d] key[0x%x]",count,key[count]));
					   if (count<16) session[count] = key[count];
					   else if (count >=16 && count < 32) controlWd[count-(keylength/3)] = key[count];
					   else if (count >= 32) iv[count - (keylength-16)] = key[count];
					}
					
					g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
					printf("\n B_DVR_PlaybackService_Open");
					
					if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					{
						DvrTest_LiveDecodeStop(pathIndex);				
					}
					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = dma;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
					g_dvrTest->streamPath[pathIndex].drmService[13] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
					printf("\nDRM Service for playback Opened\n");
					
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					g_dvrTest->drmServiceSettings.sessionKeys = session;
					g_dvrTest->drmServiceSettings.keys = controlWd;	
					g_dvrTest->drmServiceSettings.ivKeys = iv;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[13] ,&g_dvrTest->drmServiceSettings);
					
					printf("\nDRM Service for tsb playback Configured\n");				
					drmIndex = 13;					

					DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
					printf("\n DvrTest_PlaybackDecodeStart");
					B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
					printf("\n B_DVR_PlaybackService_Start");
					

				}
				break;

				case eDvrTest_OperationPlaybackServiceStopKeyladderM2m:
				{
					B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
					B_DVR_PlaybackService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].playbackService);
					
					DvrTest_PlaybackDecodeStop(pathIndex);						
					B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[13]);
					
					B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
					g_dvrTest->streamPath[pathIndex].playbackService = NULL;
					
					if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					{
						DvrTest_LiveDecodeStart(pathIndex);
					}

				}
				break;
				
				case eDvrTest_OperationRecordServiceStartKeyladderM2m:
				{
					unsigned char totalKey[48];
					int index;
					if((pathIndex >= 0 && pathIndex < 2 ))
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						B_DVR_RecordServiceInputEsStream inputEsStream;						
                        B_DVR_RecordServiceSettings recordServiceSettings;
                        B_DVR_MediaNodeSettings mediaNodeSettings;
                        B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                        fflush(stdin);
                        printf("\n Enter recording name:");
                        scanf("%s",g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);
            
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                        sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;

                        g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                        B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
						
						BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
						recordServiceSettings.parserBand  = g_dvrTest->streamPath[pathIndex].parserBand;
						dataInjectionOpenSettings.fifoSize = 30*188;
						g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
						recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
						B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);

                        BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                        inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);

                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                        inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
					
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = dma;
                        g_dvrTest->streamPath[pathIndex].drmService[14] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                        g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                        g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
						g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
						g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
						g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[14] ,&g_dvrTest->drmServiceSettings);
						
                        mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
                        mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
                        mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
                        mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
                        BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                                mediaNodeSettings.programName,
                                                                                mediaNodeSettings.volumeIndex));

						for(index=0;index<48;index++)
						{
							 if(index < 16) totalKey[index] = changeToLocalSession[index];
							 else if (index >= 16 && index <32) totalKey[index] = changeToLocalControlWord[index-KEY_LENGTH];
							 else if(index >= 32) totalKey[index] = changeToIv[index -(KEY_LENGTH*2)];
 					         BDBG_MSG(("[totalKey] %d = 0x%x",index,totalKey[index]));
						}

                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*3));              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].videoPids[0],
                                                                            totalKey);
                        

                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*3));              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].audioPids[0],
                                                                            totalKey);


						B_DVR_RecordService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].recordService,
															g_dvrTest->streamPath[pathIndex].drmService[14]);

                        BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
                        recordServiceSettings.esStreamCount = 2;
                        recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                        recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                        B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);

                        B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                        printf("\n recordService start <<");

					}
				}
				break;

				case eDvrTest_OperationRecordServiceStopKeyladderM2m:
				{
                    if(pathIndex > 0 && pathIndex < 2 )
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
						B_DVR_RecordService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].recordService);
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[14]);
                        
                        B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);

                        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                        g_dvrTest->streamPath[pathIndex].recordService = NULL;
                        g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  

                    }

				}
				break;

				case eDvrTest_OperationTSBServiceStartKeyladderCps:
				{

				}
				break;

				case eDvrTest_OperationTSBServiceStopKeyladderCps:
				{

				}
				break;
				case eDvrTest_OperationPlaybackServiceStartM2m_Trick:
				{
					 NEXUS_PidChannelHandle videoPidchHandle;
					 NEXUS_PidChannelHandle audioPidchHandle;	 
					 int drmIndex;
					 printf("pathIndex = %d",pathIndex);
					 if(!g_dvrTest->streamPath[pathIndex].tsbService)
					 {
						 printf("\n error TSBPlayStart without TSBServiceStart");
						 break;
					 }
					 else
					 {
						 B_DVR_OperationSettings operationSettings;
						 B_DVR_TSBServiceStatus tsbServiceStatus;
						 B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
						 operationSettings.operation = eB_DVR_OperationTSBPlayStart;
						 operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
					
						 if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
						 {
							 unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
							 
							 videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																				   channelInfo[currentChannel].videoPids[0]);	 
							 
							 audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																				   channelInfo[currentChannel].audioPids[0]);	 
					
							 B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
							 B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);
					
							 DvrTest_LiveDecodeStop(pathIndex);
					
							 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[0]);
							 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[1]);
							 BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
						 }
					
						 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
						 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
						 g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
						 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
						 g_dvrTest->streamPath[pathIndex].drmService[5] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				 
					
						 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
						 g_dvrTest->streamPath[pathIndex].drmService[6] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				 
					
						 printf("\nDRM Service for tsb playback Opened\n");
						 g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
						 g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
						 g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
						 g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
 						 g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
						 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[5] ,&g_dvrTest->drmServiceSettings);
					
						 g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
						 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[6] ,&g_dvrTest->drmServiceSettings);
						 printf("\nDRM Service for tsb playback Configured\n"); 			 
						 drmIndex=5;
						 DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
						 
						 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
						 operationSettings.operation = eB_DVR_OperationPause; 
						 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);

					}
				}
				break;
				
				case eDvrTest_OperationHomeNetworkM2mTest:
				{
					int count;
					uint8_t *pEncSrc;

					BKNI_EventHandle encEvent;
					BKNI_EventHandle DecEvent;

					NEXUS_DmaHandle encDma;
					NEXUS_DmaHandle decDma;					

					B_DVR_DRMServiceStreamBufferInfo *encBufferInfo;
					B_DVR_DRMServiceStreamBufferInfo *decBufferInfo;

	                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;

					encDma = NEXUS_Dma_Open(0, NULL);	
					decDma = NEXUS_Dma_Open(0, NULL);	
					
					BKNI_CreateEvent(&encEvent);
					BKNI_CreateEvent(&DecEvent);										

	                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
	                {
	                    DvrTest_LiveDecodeStart(pathIndex);
	                }
	                printf("\n live starting on channel %u and path %d",currentChannel,pathIndex);

	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; /* if Vendor didn't create Keyslot handle */
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceHomeNetworkDRM;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = encDma;
	                g_dvrTest->streamPath[pathIndex].drmService[15] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = decDma;					
					g_dvrTest->streamPath[pathIndex].drmService[16] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
					g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
					g_dvrTest->drmServiceSettings.ivKeys = changeToIv;				
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[15] ,&g_dvrTest->drmServiceSettings);

					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[16] ,&g_dvrTest->drmServiceSettings);

					NEXUS_Memory_Allocate(DMA_BLOCK_SIZE * NUM_DMA_BLOCK, NULL, (void *)&pEncSrc);

				    /* Try different pattern */
				    for (count = 0; count < DMA_BLOCK_SIZE * NUM_DMA_BLOCK; count++)
					{
				        pEncSrc[count]     = (uint8_t)count;
					}

					encBufferInfo = BKNI_Malloc(sizeof(B_DVR_DRMServiceStreamBufferInfo));
					BKNI_Memset(encBufferInfo,0,sizeof(B_DVR_DRMServiceStreamBufferInfo));	
					
					for (count=0;count<500;count++)
					{

					encBufferInfo->streamBuf = pEncSrc;
					encBufferInfo->streamBufLen = DMA_BLOCK_SIZE * NUM_DMA_BLOCK;
					encBufferInfo->event = encEvent;
						dvrError = B_DVR_DRMService_EncryptData(g_dvrTest->streamPath[pathIndex].drmService[15],encBufferInfo);

						switch(dvrError)
						{
							case B_DVR_DMAJOB_IN_QUEUE:
					BKNI_WaitForEvent(encEvent, BKNI_INFINITE);
								break;
							case B_DVR_DMAJOB_FAIL:
								printf("0x%x DMA Job fail. try it again\n",(unsigned int)g_dvrTest->streamPath[pathIndex].drmService[15]->dmaJob);	
								break;
							/*For small chuck of data, NEXUS_DmaJob_ProcessBlocks will return NEXUS_SUCCESS(B_DVR_DMAJOB_SUCCESS) 
							   without even going through DMA ISR which actally sets the event*/	
							case B_DVR_DMAJOB_SUCCESS:
								printf("0x%x DMA Job success\n",(unsigned int)g_dvrTest->streamPath[pathIndex].drmService[15]->dmaJob);
								break;
						}
						

					}
														
					NEXUS_Dma_Close(encDma);
					printf("\ndma1 closed\n");
					
					decBufferInfo = BKNI_Malloc(sizeof(B_DVR_DRMServiceStreamBufferInfo));
					BKNI_Memset(decBufferInfo,0,sizeof(B_DVR_DRMServiceStreamBufferInfo));	
					
					for (count=0;count<500;count++)
					{
					decBufferInfo->streamBuf = encBufferInfo->streamBuf;
					decBufferInfo->streamBufLen = DMA_BLOCK_SIZE * NUM_DMA_BLOCK;
					decBufferInfo->event = DecEvent;

					dvrError = B_DVR_DRMService_DecryptData(g_dvrTest->streamPath[pathIndex].drmService[16],decBufferInfo);						
						switch(dvrError)
						{
								case B_DVR_DMAJOB_IN_QUEUE:
									BKNI_WaitForEvent(DecEvent , BKNI_INFINITE);
									break;
								case B_DVR_DMAJOB_FAIL:
									printf("0x%x DMA Job fail. try it again\n",(unsigned int)g_dvrTest->streamPath[pathIndex].drmService[16]->dmaJob);	
									break;
								/*For small chuck of data, NEXUS_DmaJob_ProcessBlocks will return NEXUS_SUCCESS(B_DVR_DMAJOB_SUCCESS) 
								   without even going through DMA ISR which actally sets the event*/	
								case B_DVR_DMAJOB_SUCCESS:
									printf("0x%x DMA Job success\n",(unsigned int)g_dvrTest->streamPath[pathIndex].drmService[16]->dmaJob);
									break;
						}
					}

					BKNI_Free(encBufferInfo);
					BKNI_Free(decBufferInfo);
					NEXUS_Dma_Close(decDma);
					printf("dma2 closed\n");


				}
				break;

				case eDvrTest_OperationRecordServiceStartCpsEvenOddClearKey:
				{
                    if((pathIndex > 0 && pathIndex < 2 ))
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                    	uint8_t videoKey[32];
						uint8_t audioKey[32];
						int keyIndex,numOfKeyChange;
                        unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						B_DVR_RecordServiceInputEsStream inputEsStream;						
                        B_DVR_RecordServiceSettings recordServiceSettings;
                        NEXUS_PidChannelHandle videoPidchHandle;
                        NEXUS_PidChannelHandle audioPidchHandle;    
                        B_DVR_MediaNodeSettings mediaNodeSettings;
                        B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                
                        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                        fflush(stdin);
                        printf("\n Enter recording name:");
                        scanf("%s",g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);
            
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                        sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
                        g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;						
						
                        g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                        printf("\n recordService open <<");

                        B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
                        printf("\n recordService install callback <<");

						BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
						recordServiceSettings.parserBand  = g_dvrTest->streamPath[pathIndex].parserBand;
						dataInjectionOpenSettings.fifoSize = 30*188;
						g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
						recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
						B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);

                        BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
                        inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
                        videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);
                   
                        inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                        inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                        inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
                        audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);                      

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoEvenKeyLoaderCallback;						
                        g_dvrTest->streamPath[pathIndex].drmService[20] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);      

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoOddKeyLoaderCallback;						
                        g_dvrTest->streamPath[pathIndex].drmService[21] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioEvenKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmService[22] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioOddKeyLoaderCallback;
                        g_dvrTest->streamPath[pathIndex].drmService[23] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                        printf("\nDRM Service for Even / Odd Audio & Video Opened\n");   
                        
                        g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                        g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                        g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eEven;
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKeyVideoEven;						
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[20] ,&g_dvrTest->drmServiceSettings);
                            
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKeyVideoOdd;
						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[21] ,&g_dvrTest->drmServiceSettings);

						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eEven;
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKeyAudioEven;						
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[22] ,&g_dvrTest->drmServiceSettings);
                            
                        g_dvrTest->drmServiceSettings.keys = changeToLocalKeyAudioOdd;
						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;
                        dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[23] ,&g_dvrTest->drmServiceSettings);

						for(keyIndex=0;keyIndex<(KEY_LENGTH*2);keyIndex++)
						{
							(keyIndex<16)? (videoKey[keyIndex] = changeToLocalKeyVideoEven[keyIndex]):(videoKey[keyIndex] = changeToLocalKeyVideoOdd[keyIndex-KEY_LENGTH]);
							(keyIndex<16)? (audioKey[keyIndex] = changeToLocalKeyAudioEven[keyIndex]):(audioKey[keyIndex] = changeToLocalKeyAudioOdd[keyIndex-KEY_LENGTH]);		
							printf("videoKey[%d] = 0x%x , audioKey[%d] = 0x%x\n",keyIndex,videoKey[keyIndex],keyIndex,audioKey[keyIndex]);
						}
			
                        mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
                        mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
                        mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
                        mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
                        BDBG_MSG(("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                                mediaNodeSettings.programName,
                                                                                mediaNodeSettings.volumeIndex));
                        
                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*2));              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].videoPids[0],
                                                                            videoKey);
                        
                        dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*2));              
                        dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
                                                                            &mediaNodeSettings,
                                                                            channelInfo[currentChannel].audioPids[0],
                                                                            audioKey);
                        
                        dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);                 
                        dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],audioPidchHandle);

                        BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
                        recordServiceSettings.esStreamCount = 2;
                        recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                        recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                        B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
                        printf("\n recordService start >>");
                        B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                        printf("\n recordService start <<");

                    	for(numOfKeyChange=0;numOfKeyChange<10;numOfKeyChange++)
                    	{
							if(numOfKeyChange%2)
							{
								dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);				 
								dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],audioPidchHandle); 
								printf("Even Key\n");
							}
							else
							{
								dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[21],videoPidchHandle);
								dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[23],audioPidchHandle);
								printf("Odd Key\n");								
							}
							BKNI_Sleep(400);								
						}
                     }

				}
				break;

				case eDvrTest_OperationRecordServiceStopCpsEvenOddClearKey:
				{
                    NEXUS_PidChannelHandle videoPidchHandle;
                    NEXUS_PidChannelHandle audioPidchHandle;    
                    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                    
                    if(pathIndex > 0 && pathIndex < 2 )
                    {
                        printf("\n invalid path chosen. Path should be between 2-5");
                    }
                    else
                    {
                        B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);

                        videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                                  channelInfo[currentChannel].videoPids[0]);
                        audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                                  channelInfo[currentChannel].audioPids[0]);

                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);
                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[21],videoPidchHandle);                
                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],audioPidchHandle);
                        B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[23],audioPidchHandle);        

                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[20]);
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[21]);                        
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[22]);
                        B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[23]);
						
                        B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);						
                        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                        g_dvrTest->streamPath[pathIndex].recordService = NULL;
                        g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
                        
                    }

				}
				break;

				case eDvrTest_OperationPlaybackServiceStartCpsEvenOddClearKey:
				{
					 B_DVR_MediaNodeSettings mediaNodeSettings; 
					 B_DVR_Media mNode;
					 unsigned keylength;
					 unsigned char vKey[32];
					 unsigned char aKey[32];
					 int count,drmIndex;
					 
					 if(g_dvrTest->streamPath[pathIndex].tsbService)
					 {
						 printf("\n Stop TSB service running on this path %d",pathIndex);
					 }
				
					 fflush(stdin);
					 printf("\n Enter the sub folder name for recording to be playedback");
					 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
					 printf("\n Enter the recording name");
					 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
					 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
					 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
				
					 mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
					 mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
					 mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
					 mediaNodeSettings.volumeIndex = 0;
					 
					 B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);
				
					 B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);
				
					 B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,vKey);
					 B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[1].pid,aKey);
				
					 printf("keylength = %d\n",keylength);
					 for(count=0;count<(int)keylength;count++)
					 {
						printf("[%d] vKey[0x%x] , aKey[0x%x]\n",count,vKey[count],aKey[count]);
					 }
				
					 g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
					 printf("\n B_DVR_PlaybackService_Open");
					 
					 if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					 {
						 DvrTest_LiveDecodeStop(pathIndex); 			 
					 }
				
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
					 g_dvrTest->streamPath[pathIndex].drmService[25] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			 
				
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					 g_dvrTest->streamPath[pathIndex].drmService[26] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			 
				
					 printf("\nDRM Service for playback Opened\n");
					 g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					 g_dvrTest->drmServiceSettings.keyLength = keylength; /* 32 bytes due to include even and odd keys at one time */
					 g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
					 g_dvrTest->drmServiceSettings.keys = vKey;
					 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[25] ,&g_dvrTest->drmServiceSettings);
				
					 g_dvrTest->drmServiceSettings.keys = aKey;
					 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[26] ,&g_dvrTest->drmServiceSettings);
					 printf("\nDRM Service for playback Configured\n"); 			 
					 drmIndex = 25;
					 printf("\n DvrTest_LiveDecodeStop");
					 DvrTest_PlaybackDecodeStart(pathIndex,drmIndex);
					 printf("\n DvrTest_PlaybackDecodeStart");
					 B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
					 printf("\n B_DVR_PlaybackService_Start");
					 
				}
				break;
				case eDvrTest_OperationPlaybackServiceStopCpsEvenOddClearKey:
				{
					B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[25],
														   g_dvrTest->streamPath[pathIndex].videoPlaybackPidChannels[0]);
					
					B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[26],
														   g_dvrTest->streamPath[pathIndex].audioPlaybackPidChannels[0]);
					 
					 B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
					 DvrTest_PlaybackDecodeStop(pathIndex);
					 
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[25]);
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[26]);
					 
					 B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
					 g_dvrTest->streamPath[pathIndex].playbackService = NULL;
					
					 if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					 {
						 DvrTest_LiveDecodeStart(pathIndex);
					 }

				}
				break;
				

	        	default:
		           printf("\n Invalid DVR Operation - Select the operation again");
            }
        }
return 0;
}
