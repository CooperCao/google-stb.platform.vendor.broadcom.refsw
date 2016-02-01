/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
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
#include "dvr_test.h"

static char str_buf[1024];

B_EventHandle TSBServiceReadyEvent;
B_EventHandle TSBConversionCompleteEvent;
B_EventHandle TSBDataInjectionCompleteEvent;


BDBG_MODULE(dvr_test_common);

#if defined(ENABLE_DRM)
unsigned  char changeToLocalKey[16];
unsigned  char changeToLocalKey2[16];

unsigned  char changeToLocalKeyVideoEven[16];
unsigned  char changeToLocalKeyVideoOdd[16];
unsigned  char changeToLocalKeyAudioEven[16];
unsigned  char changeToLocalKeyAudioOdd[16];

unsigned char changeToLocalSession[16];
unsigned char changeToLocalControlWord[16];
unsigned char changeToIv[16];


unsigned char VidEvenCpsControlWord[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char VidOddCpsControlWord[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudEvenCpsControlWord[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudOddCpsControlWord[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   

#if 0
unsigned char VidEvenCpsControlWord2[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char VidOddCpsControlWord2[] = { 
                                            0x6e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   

unsigned char AudEvenCpsControlWord2[] = { 
                                            0x8e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudOddCpsControlWord2[] = {
                                            0x0e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
#else
unsigned char VidEvenCpsControlWord2[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char VidOddCpsControlWord2[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudEvenCpsControlWord2[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   
unsigned char AudOddCpsControlWord2[] = { 
                                            0x2e, 0xf6, 0xb6, 0xcc, 0x5b, 0x6c, 0x86, 0xf7, 
                                            0x92, 0xa2, 0x48, 0x70, 0xac, 0xd9, 0x46, 0x73  };   

#endif

unsigned char ucProcInKey3[16] = { 0x00,0x00,0x11,0x11 ,0x55,0x55,0x66,0x66,0x11,0x22,0x33,0x44 ,0x55,0x66,0x77,0x88 };
unsigned char ucProcInKey4[16] = { 0x28,0x30,0x46,0x70 ,0x71,0x4B,0x8C,0x75,0xEB,0x46,0x04,0xBB ,0x96,0xD0,0x48,0x88};
unsigned char ivkeys[16] = { 0xad, 0xd6, 0x9e, 0xa3,0x89, 0xc8, 0x17, 0x72, 0x1e, 0xd4, 0x0e, 0xab,0x3d, 0xbc, 0x7a, 0xf2 };

/* use one key */
void videoKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalKey[i] = VidEvenCpsControlWord[i];     
       BDBG_MSG(("[Vid] %d = 0x%x",i,changeToLocalKey[i]));
    }

}

void audioKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH ;i++) 
    {
         changeToLocalKey2[i] = AudEvenCpsControlWord[i] ;   
         BDBG_MSG(("[Aud] %d = 0x%x",i,changeToLocalKey2[i]));       
    }

}


/*use Even / Odd keys*/
void videoEvenKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<(KEY_LENGTH);i++)  
    {
         changeToLocalKeyVideoEven[i] = VidEvenCpsControlWord2[i];     
       BDBG_MSG(("[Vid Even] %d = 0x%x",i,changeToLocalKeyVideoEven[i]));
    }

}

void videoOddKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<(KEY_LENGTH*2);i++)  
    {
         changeToLocalKeyVideoOdd[i] = VidOddCpsControlWord2[i];     
       BDBG_MSG(("[Vid Odd] %d = 0x%x",i,changeToLocalKeyVideoOdd[i]));
    }

}

void audioEvenKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH ;i++) 
    {
         changeToLocalKeyAudioEven[i] = AudEvenCpsControlWord2[i] ;   
         BDBG_MSG(("[Aud Even] %d = 0x%x",i,changeToLocalKeyAudioEven[i]));       
    }

}

void audioOddKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle);

    for (i=0;i<KEY_LENGTH ;i++) 
    {
         changeToLocalKeyAudioOdd[i] = AudOddCpsControlWord2[i];   
         BDBG_MSG(("[Aud Odd] %d = 0x%x",i,changeToLocalKeyAudioOdd[i]));       
    }

}


void procInKey3_sessionKeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)  
    {
         changeToLocalSession[i] = ucProcInKey3[i];   
         BDBG_MSG(("[sessionKey] %d = 0x%x",i,changeToLocalSession[i]));
    }   
}

void procInKey4_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToLocalControlWord[i] = ucProcInKey4[i]);   
        BDBG_MSG(("[procInKey4] %d = 0x%x",i,changeToLocalControlWord[i]));    
    }   

}

void iv_KeyLoaderCallback(NEXUS_KeySlotHandle keySlotHandle)
{
    int i;
    BDBG_ASSERT(keySlotHandle); 

    for (i=0;i<KEY_LENGTH;i++)    
    {
        (changeToIv[i] = ivkeys[i]);   
        BDBG_MSG(("[IV] %d = 0x%x",i,changeToIv[i]));    
    }   

}

#endif

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;
    NEXUS_FrontendQamStatus qamStatus;
    NEXUS_Error rc;

    BSTD_UNUSED(param);

    rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(rc == NEXUS_SUCCESS){ 
        if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
            fprintf(stderr, "Frontend locked.\n");
        else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
            fprintf(stderr, "Frontend unlocked.\n");
        else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
            fprintf(stderr, "No signal at the tuned frequency.\n");
    }
    else if(rc == NEXUS_NOT_SUPPORTED){
    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        fprintf(stderr, "QAM Lock callback, frontend 0x%08x - lock status %d, %d\n", (unsigned)frontend, qamStatus.fecLock, qamStatus.receiverLock);
    }
    else
         if(rc){rc = BERR_TRACE(rc);}
}

static void DvrTest_Scheduler(void * param)
{
    BSTD_UNUSED(param);
    BDBG_MSG(("Starting Scheduler 0x%08x", (unsigned)g_dvrTest->PFscheduler));
    B_Scheduler_Run(g_dvrTest->PFscheduler);
    return;
}


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

void DvrTest_AudioVideoPathOpen(CuTest * tc, unsigned index)
{
	BSTD_UNUSED(tc);
	printf("\n %s:  index %d >>> ",__FUNCTION__,index);
    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].stcSettings.timebase = index?NEXUS_Timebase_e1:NEXUS_Timebase_e0;

    g_dvrTest->streamPath[index].stcChannel = NEXUS_StcChannel_Open(index, &g_dvrTest->streamPath[index].stcSettings);
    g_dvrTest->streamPath[index].videoProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].audioProgram.stcChannel =  g_dvrTest->streamPath[index].stcChannel;
    g_dvrTest->streamPath[index].window = NEXUS_VideoWindow_Open(g_dvrTest->display,index);
    g_dvrTest->streamPath[index].videoDecoder = NEXUS_VideoDecoder_Open(index, NULL);
    NEXUS_VideoWindow_AddInput(g_dvrTest->streamPath[index].window,
                               NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    if(!index)
    {
        g_dvrTest->streamPath[index].audioDecoder = NEXUS_AudioDecoder_Open(index, NULL);
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                    NEXUS_AudioDecoderConnectorType_eStereo)
                                   );
#endif
    }
    else
    {
        g_dvrTest->streamPath[index].audioDecoder = NULL;
    }
    
    printf("\n %s:  index %d <<<",__FUNCTION__,index);
    return;
}
void DvrTest_AudioVideoPathClose(CuTest * tc, int index)
{
	BSTD_UNUSED(tc);
	printf("\n %s:index %d >>>",__FUNCTION__,index);
    if(!index)
    {
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
#endif
        NEXUS_AudioDecoder_Close(g_dvrTest->streamPath[index].audioDecoder);
    }
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
#endif
    NEXUS_VideoWindow_RemoveAllInputs(g_dvrTest->streamPath[index].window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(g_dvrTest->streamPath[index].videoDecoder));
    NEXUS_VideoWindow_Close(g_dvrTest->streamPath[index].window);
    NEXUS_VideoDecoder_Close(g_dvrTest->streamPath[index].videoDecoder);
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

void DvrTest_TuneChannel(CuTest * tc, int index)
{
    NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
	BSTD_UNUSED(tc);
    printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);

    if (g_dvrTest->streamPath[index].userParams.isMtsif)
	{
	    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
		g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_dvrTest->streamPath[index].frontend);
	}
	else 
	{
		g_dvrTest->streamPath[index].parserBandSettings.sourceType =  NEXUS_ParserBandSourceType_eInputBand;
		g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrTest->streamPath[index].userParams.param1;
	}

	g_dvrTest->streamPath[index].parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);


    NEXUS_Frontend_GetDefaultQamSettings(&g_dvrTest->streamPath[index].qamSettings);
    g_dvrTest->streamPath[index].qamSettings.frequency =  channelInfo[currentChannel].frequency;
    g_dvrTest->streamPath[index].qamSettings.mode = channelInfo[currentChannel].modulation;
    g_dvrTest->streamPath[index].qamSettings.annex = channelInfo[currentChannel].annex;
    g_dvrTest->streamPath[index].qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;

	g_dvrTest->streamPath[index].qamSettings.lockCallback.callback = lock_callback;
    g_dvrTest->streamPath[index].qamSettings.lockCallback.context = g_dvrTest->streamPath[index].frontend;

    rc = NEXUS_Frontend_TuneQam(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamSettings);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].qamStatus);
	BDBG_ASSERT(!rc);
	
   	printf("\n %s: Chip ID %x ", __FUNCTION__, g_dvrTest->streamPath[index].userParams.chipId);
    printf("\n %s: receiver lock = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.receiverLock);
    printf("\n %s: Symbol rate = %d",__FUNCTION__,g_dvrTest->streamPath[index].qamStatus.symbolRate);

    printf("\n %s: index %d >>>",__FUNCTION__,index);
    return;
}

void DvrTest_PlaybackDecodeStart(CuTest * tc, int index)
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
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].tsbServiceRequest.volumeIndex;

            BSTD_UNUSED(playbackServiceSettings);
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            g_dvrTest->streamPath[index].playback = tsbServiceSettings.tsbPlaybackSettings.playback;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            
            
        }
        else
        {
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].playbackServiceRequest.volumeIndex;
 
            BSTD_UNUSED(tsbServiceSettings);
            B_DVR_PlaybackService_GetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
            g_dvrTest->streamPath[index].playback = playbackServiceSettings.playback;
            playbackServiceSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            playbackServiceSettings.videoDecoder[1]=NULL;
            playbackServiceSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            playbackServiceSettings.audioDecoder[1]=NULL;
            playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_PlaybackService_SetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
        }

        rc = B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n media node not found");
        }
        CuAssertTrue(tc, rc==B_DVR_SUCCESS );

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = g_dvrTest->streamPath[index].videoDecoder;
        g_dvrTest->streamPath[index].videoPlaybackPidChannels[0] =
             NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[0].pid,&playbackPidSettings);
		if(!index)
		{
	        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
	        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
	        playbackPidSettings.pidTypeSettings.audio.primary = g_dvrTest->streamPath[index].audioDecoder;
	        g_dvrTest->streamPath[index].audioPlaybackPidChannels[0] = 
	            NEXUS_Playback_OpenPidChannel(g_dvrTest->streamPath[index].playback,media.esStreamInfo[1].pid,&playbackPidSettings);
		}
        NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
        g_dvrTest->streamPath[index].videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
        g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPlaybackPidChannels[0];
        g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
        NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
            NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);
        }
        g_dvrTest->streamPath[index].playbackDecodeStarted=true;

    }
    else
    {
        printf("\n error!!!!!neither playback or tsbservice opened");
        CuAssertTrue(tc, 0);

    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}

#if defined(ENABLE_DRM)
void DvrTest_PlaybackHostTrick(int index)
{

	int i=0;	
	B_DVR_OperationSettings operationSettings;

	operationSettings.operationSpeed = 4;

	printf("\n\nTrick Test\n");
	for(i=0;i<3;i++)
	{
		switch(i)
		{
			case 0:
			{
				printf("\nFast Forward \n"); 
				operationSettings.operation = eB_DVR_OperationFastForward;				
			}
			break;
			
			case 1:
			{
				printf("\nFast Rewind \n"); 
				operationSettings.operation = eB_DVR_OperationFastRewind;				
			}
			break;
			
			case 2:
			{
				printf("\nNormal Playback \n"); 
				operationSettings.operation = eB_DVR_OperationPlay;				
			}
			break;
				
		}
		
		BKNI_Sleep(1000);
		if(g_dvrTest->streamPath[index].tsbService)
		{
			B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
		}
		else
		{
			B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[index].playbackService,&operationSettings);
		} 
	}

}
void DvrTest_PlaybackDecodeStartDrm(CuTest * tc, int index,int drmIndex,NEXUS_SecurityAlgorithmConfigDestination drmDest,bool tsbPlayback)
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
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].tsbServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].tsbServiceRequest.volumeIndex;

            BSTD_UNUSED(playbackServiceSettings);
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            g_dvrTest->streamPath[index].playback = tsbServiceSettings.tsbPlaybackSettings.playback;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
            tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[index].tsbService,&tsbServiceSettings);
            
            
        }
        else
        {
            mediaNodeSettings.subDir = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.subDir[0];
            mediaNodeSettings.programName = (char *)&g_dvrTest->streamPath[index].playbackServiceRequest.programName[0];
            mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[index].playbackServiceRequest.volumeIndex;
 
            BSTD_UNUSED(tsbServiceSettings);
            B_DVR_PlaybackService_GetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
            g_dvrTest->streamPath[index].playback = playbackServiceSettings.playback;
            playbackServiceSettings.videoDecoder[0]=g_dvrTest->streamPath[index].videoDecoder;
            playbackServiceSettings.videoDecoder[1]=NULL;
            playbackServiceSettings.audioDecoder[0]=g_dvrTest->streamPath[index].audioDecoder;
            playbackServiceSettings.audioDecoder[1]=NULL;
            playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            B_DVR_PlaybackService_SetSettings(g_dvrTest->streamPath[index].playbackService,&playbackServiceSettings);
        }

        rc = B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n media node not found");
        }
        CuAssertTrue(tc, rc==B_DVR_SUCCESS );

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

		if( drmDest == NEXUS_SecurityAlgorithmConfigDestination_eCa && tsbPlayback == false)
		{
			B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[index].drmService[drmIndex],
												   g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
				
			B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[index].drmService[drmIndex+1],
												  g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);

		}
		else if(drmDest == NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem && tsbPlayback == false)
		{
			printf("\nDRM Service for m2m playback started\n");
			B_DVR_PlaybackService_AddDrmSettings(g_dvrTest->streamPath[index].playbackService,
													  g_dvrTest->streamPath[index].drmService[drmIndex]);
		}
		else if(drmDest == NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem && tsbPlayback == true)
		{
			printf("\nDRM Service for m2m tsb playback started\n");
			B_DVR_TSBService_AddDrmSettings(g_dvrTest->streamPath[index].tsbService,
													  g_dvrTest->streamPath[index].drmService[drmIndex],
													  tsbPlayback);
		}
				
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
            NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[index].audioDecoder,&g_dvrTest->streamPath[index].audioProgram);
        }
        g_dvrTest->streamPath[index].playbackDecodeStarted=true;

    }
    else
    {
        printf("\n error!!!!!neither playback or tsbservice opened");
        CuAssertTrue(tc, 0);

    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}

#endif
void DvrTest_PlaybackDecodeStop(CuTest * tc, int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
	BSTD_UNUSED(tc);
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
    }
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
	if(!index)
	{
    	NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].audioPlaybackPidChannels[0]);
	}
    NEXUS_Playback_ClosePidChannel(g_dvrTest->streamPath[index].playback,g_dvrTest->streamPath[index].videoPlaybackPidChannels[0]);
    g_dvrTest->streamPath[index].playbackDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}


void DvrTest_LiveDecodeStart(CuTest * tc, int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        #if 0
        NEXUS_PidChannelSettings settings;

        NEXUS_PidChannel_GetDefaultSettings(&settings);
        settings.remap.pid = 0x1ff2;
        settings.remap.enabled = true;
        settings.remap.continuityCountEnabled = true;
        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                 channelInfo[currentChannel].videoPids[0],
                                                                                 &settings);
        #else
        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                 channelInfo[currentChannel].videoPids[0],
                                                                                 NULL);
        #endif
        CuAssertPtrNotNullMsg(tc, "NEXUS_PidChanel_Open failed", g_dvrTest->streamPath[index].videoPidChannels[0]);
    }
    else
    {
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                             channelInfo[currentChannel].videoPids[0],
                                                                             NULL);
    }
    CuAssertPtrNotNullMsg(tc, "NEXUS_PidChanel_Open failed", g_dvrTest->streamPath[index].videoPidChannels[0]);

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

        if(g_dvrTest->streamPath[index].tsbService)
        {
			#if 0
			NEXUS_PidChannelSettings settings;
            NEXUS_PidChannel_GetDefaultSettings(&settings);
            settings.remap.pid = 0x1ff3;
            settings.remap.enabled = true;
            settings.remap.continuityCountEnabled = true;
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                     channelInfo[currentChannel].audioPids[0],
                                                                                     &settings);
            #else
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                     channelInfo[currentChannel].audioPids[0],
                                                                                     NULL);
            #endif
        }
        else
        { 
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                     channelInfo[currentChannel].audioPids[0],
                                                                                 NULL);
        }
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


void DvrTest_LiveDecodeStop(CuTest * tc, int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
	BSTD_UNUSED(tc);
    printf("\n %s: index %d >>>",__FUNCTION__,index);
    NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[index].videoDecoder,&videoDecoderSettings);
    if(!index)
    {
        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[index].audioDecoder);
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
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};
    printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    BSTD_UNUSED(appContext);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
                B_DVR_TSBServiceStatus tsbServiceStatus;

                B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                printf("\n Beginning Of TSB Conversion start time %lu endTime %lu",tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
                B_Event_Set(TSBServiceReadyEvent);
                
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
            B_Event_Set(TSBConversionCompleteEvent);
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
            B_Event_Set(TSBDataInjectionCompleteEvent);
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
    default:
        {
            printf("\n invalid event");
        }
   }
    printf("\n DvrTest_Callback <<<<<");
    return;
}

#if NEXUS_NUM_HDMI_OUTPUTS
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
#endif

void DvrTestInit(CuTest * tc)
{
    int decoderIndex,frontendIndex, pathIndex;
    int i;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    unsigned volumeIndex;

    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 
	
	BKNI_Init();
    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));

    mediaStorageOpenSettings.storageType = eB_DVR_MediaStorageTypeBlockDevice;
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);

    B_DVR_MediaStorage_GetStatus(g_dvrTest->mediaStorage,&mediaStorageStatus);
    
    volumeIndex=0;

    B_DVR_MediaStorage_MountVolume(g_dvrTest->mediaStorage, volumeIndex);

    DvrTest_PrintChannelMap();
    g_dvrTest->maxChannels = 0;
    g_dvrTest->streamPath[0].currentChannel = 0;
    if(NEXUS_NUM_VIDEO_DECODERS > 1)
    g_dvrTest->streamPath[1].currentChannel= 1;

    B_Os_Init();

    TSBServiceReadyEvent = B_Event_Create(NULL);
    TSBConversionCompleteEvent = B_Event_Create(NULL);
    TSBDataInjectionCompleteEvent = B_Event_Create(NULL);

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&g_dvrTest->platformconfig);
    for (frontendIndex=0,i=0;frontendIndex < MAX_STREAM_PATH; frontendIndex++ )
    {
        g_dvrTest->streamPath[frontendIndex].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
        if (g_dvrTest->streamPath[frontendIndex].frontend)
        {
            NEXUS_Frontend_GetCapabilities(g_dvrTest->streamPath[frontendIndex].frontend, &g_dvrTest->streamPath[frontendIndex].capabilities);
            if (g_dvrTest->streamPath[frontendIndex].capabilities.qam)
            {
                g_dvrTest->streamPath[i].frontend = g_dvrTest->platformconfig.frontend[frontendIndex];
/*                NEXUS_Frontend_GetQamStatus(g_dvrTest->streamPath[i].frontend, &g_dvrTest->streamPath[i].qamStatus);*/
                DvrTest_TuneChannel(tc, i);
                g_dvrTest->streamPath[i].liveDecodeStarted= false;
                i++;
                g_dvrTest->maxChannels++;
            }
        }
    }


    NEXUS_Display_GetDefaultSettings(&g_dvrTest->displaySettings);
    g_dvrTest->displaySettings.format = NEXUS_VideoFormat_e1080i;
    g_dvrTest->display = NEXUS_Display_Open(0, &g_dvrTest->displaySettings);
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(g_dvrTest->display, NEXUS_HdmiOutput_GetVideoConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    g_dvrTest->hdmiSettings.hotplugCallback.callback = hotplug_callback;
    g_dvrTest->hdmiSettings.hotplugCallback.context = g_dvrTest->platformconfig.outputs.hdmi[0];
    g_dvrTest->hdmiSettings.hotplugCallback.param = (int)g_dvrTest->display;
    NEXUS_HdmiOutput_SetSettings(g_dvrTest->platformconfig.outputs.hdmi[0], &g_dvrTest->hdmiSettings);
    #endif
    for(decoderIndex=0;decoderIndex < MAX_AV_PATH; decoderIndex++)
    {
        DvrTest_AudioVideoPathOpen(tc, decoderIndex);
        DvrTest_LiveDecodeStart(tc, decoderIndex);
        g_dvrTest->streamPath[decoderIndex].liveDecodeStarted = true;
        g_dvrTest->streamPath[decoderIndex].playbackDecodeStarted = false;
    }

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTest_Callback;
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,pathIndex);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
#if defined (ENABLE_DRM)
	g_dvrTest->dma = NEXUS_Dma_Open(0, NULL);	
	g_dvrTest->dma2 = NEXUS_Dma_Open(0, NULL);	
    CuAssertPtrNotNullMsg(tc, "DMA alloca failed", g_dvrTest->dma);
    CuAssertPtrNotNullMsg(tc, "DMA2 alloca failed", g_dvrTest->dma2);	
#endif
	
	g_dvrTest->glbstreamMutex = B_Mutex_Create(NULL);
    if( !g_dvrTest->glbstreamMutex)
    {
        printf("\n glbstreamMutex create error");
        assert(0);
    }
    g_dvrTest->PFscheduler= B_Scheduler_Create(NULL);
    if(!g_dvrTest->PFscheduler)
    {
        printf("\n scheduler create error");
        assert(0);
    }
    g_dvrTest->threadID = B_Thread_Create("DvrTest Scheduler", DvrTest_Scheduler,g_dvrTest, NULL);

    if(!g_dvrTest->threadID)
    {
        printf("\n scheduler thread create error");
        assert(0);
    }

}

void DvrTestExit(CuTest * tc)
{
    int pathIndex;

    printf("\n Closing all the services after running all tests \n");
    for(pathIndex=0;pathIndex < MAX_STREAM_PATH; pathIndex++)
    {
        if(pathIndex < MAX_AV_PATH)
        { 
            if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
            {
                DvrTest_PlaybackDecodeStop(tc, pathIndex);
            }
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
            }

            if(g_dvrTest->streamPath[pathIndex].playbackService)
            {
                B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
                B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
            }
            if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(tc, pathIndex);
            }
            DvrTest_AudioVideoPathClose(tc, pathIndex);
        }
        else
        {
            if(g_dvrTest->streamPath[pathIndex].recordService)
            {
                B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
                B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
            }
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,pathIndex);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }

    B_DVR_Manager_DestroyMediaNodeList(g_dvrTest->dvrManager,0);

    B_DVR_MediaStorage_UnmountVolume(g_dvrTest->mediaStorage,0);
    B_DVR_Manager_UnInit(g_dvrTest->dvrManager);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();
}


void Do_DvrTest_Operation(CuTest *tc, DvrTest_Operation dvrTestOp, DvrTest_Operation_Param * param) 
{
    int pathIndex;

    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    pathIndex = param->pathIndex;
    sprintf(str_buf, "channel %d not exist", pathIndex);
    CuAssert(tc, str_buf, g_dvrTest->maxChannels > pathIndex );
    switch (dvrTestOp)
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
                DvrTest_LiveDecodeStop(tc, pathIndex);
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
            DvrTest_TuneChannel(tc, pathIndex);
            DvrTest_LiveDecodeStart(tc, pathIndex);
            g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
        }
    case1_error:
        break;
    case eDvrTest_OperationChannelUp:
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                printf("\n Invalid operation! Stop the timeshift srvc on this channel");
                break;
            }
            if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(tc, pathIndex);
            }
            g_dvrTest->streamPath[pathIndex].currentChannel++;
            g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
            printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
            DvrTest_TuneChannel(tc, pathIndex);
            DvrTest_LiveDecodeStart(tc, pathIndex);
            g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
        break;
    case eDvrTest_OperationChannelDown:
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                printf("\n Invalid operation! Stop the timeshift srvc on this channel");
                break;
            }
            if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(tc, pathIndex);
            }
            g_dvrTest->streamPath[pathIndex].currentChannel--;
            g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
            printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
            DvrTest_TuneChannel(tc, pathIndex);
            DvrTest_LiveDecodeStart(tc, pathIndex);
            g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
        break;
    case eDvrTest_OperationTSBServiceStart:
        {
            unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
            B_DVR_TSBServiceInputEsStream inputEsStream;
            B_DVR_TSBServiceSettings tsbServiceSettings;
            B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
            if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStart(tc, pathIndex);
            }
            printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
            BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
            BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
                          sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
                          "%s%d",TSB_SERVICE_PREFIX,pathIndex);
            printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
            g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
            sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
            g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
            g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
            g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
			g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBTime = MAX_TSB_TIME;
            g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
            g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
            B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

            BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
            dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
            g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
            tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
            tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

            BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
            inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
            inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
            inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
            inputEsStream.pidChannel = NULL;
            B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

            inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
            inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
            inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
            B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

            #if 0
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);
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
    case eDvrTest_OperationTSBServiceStop:
        {
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    DvrTest_PlaybackDecodeStop(tc, pathIndex);
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
                
            }
            else
            {
                printf("\n timeshift srvc not started");
            }
            
        }
        break;
    case eDvrTest_OperationTSBServicePlayStart:
        {
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
                    DvrTest_LiveDecodeStop(tc, pathIndex);
                }
                DvrTest_PlaybackDecodeStart(tc, pathIndex);
                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                operationSettings.operation = eB_DVR_OperationPause; 
                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
            }

        }
        break;
    case eDvrTest_OperationTSBServicePlayStop:
        {
            if(!g_dvrTest->streamPath[pathIndex].tsbService)
            {
                printf("\n error TSBPlayStart without TSBServiceStart");
                break;
            }
            else
            {
                B_DVR_OperationSettings operationSettings;
                operationSettings.operation = eB_DVR_OperationTSBPlayStop;
                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                DvrTest_PlaybackDecodeStop(tc, pathIndex);                    
                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                {
                    DvrTest_LiveDecodeStart(tc, pathIndex);
                }
                
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
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                printf("\n Stop TSB service running on this path %d",pathIndex);
                goto error_playbackService;
            }
            fflush(stdin);
            strcpy(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir, param->subDir);
			strcpy(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, param->programName);
            g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
            g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
            g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
			B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService, g_dvrTest->dvrTestCallback, (void *)g_dvrTest);
            printf("\n B_DVR_PlaybackService_Open");
            if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(tc, pathIndex);
            }
            
            printf("\n DvrTest_LiveDecodeStop");
            DvrTest_PlaybackDecodeStart(tc, pathIndex);
            printf("\n DvrTest_PlaybackDecodeStart");
            B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
            printf("\n B_DVR_PlaybackService_Start");
            
        }
    error_playbackService:
        break;
    case eDvrTest_OperationPlaybackServiceStop:
        {
            B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
            DvrTest_PlaybackDecodeStop(tc, pathIndex);
            B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
            g_dvrTest->streamPath[pathIndex].playbackService = NULL;

            if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStart(tc, pathIndex);
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
                B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                B_DVR_RecordServiceSettings recordServiceSettings;
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                BDBG_MSG(("\n recording name: %s", param->programName));
                strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName, 
                        B_DVR_MAX_FILE_NAME_LENGTH);
                
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
                printf("\n recordService open >>>");
                g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                printf("\n recordService open <<");
                printf("\n recordService install callback >>");
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
                inputEsStream.pidChannel = NULL;
                B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
                inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
                inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
                inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
                B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
#if 0
                B_DVR_RecordService_GetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
                recordServiceSettings.esStreamCount = 2;
                recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
#endif


				printf("\n recordService start >>");
                B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                printf("\n recordService start <<");
              
            }
        }
        break;
    case eDvrTest_OperationRecordServiceStop:
        {
            if(pathIndex > 0 && pathIndex < 2 )
            {
                printf("\n invalid path chosen. Path should be between 2-5");
            }
            else
            {
                B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
                B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                g_dvrTest->streamPath[pathIndex].recordService = NULL;
                g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
                
            }
        }
        break;
    case eDvrTest_OperationDataInjectionStart:
        {
            B_DVR_DataInjectionServiceSettings settings;
            char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
            uint8_t pkt[188*30];
            unsigned pid = 0x0;
            int dataFileID;
            int readSize;
            
            fflush(stdin);
            printf("\n Enter data pid number:   ");
            scanf("%u",&pid);
            printf("\n data Pid %d",pid);
            
            printf("\n Enter a transport file name having the desired data PID");
            scanf("%s",dataFileName);
            printf("\n data Pid %s",dataFileName);

            if ((dataFileID = open(dataFileName, O_RDONLY,0666)) < 0)
            {
                printf("\n unable to open %s",dataFileName);
                break;
            }

            BKNI_Memset((void *)&pkt[0],0,188*30);
            
            readSize = (int) read(dataFileID,&pkt[0],188*30);
            
            if(readSize < 0)
            {
                printf("\n Data file is empty or not available");
                break;
            }

            B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
            
            settings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
            settings.pid = pid;
            B_DVR_DataInjectionService_SetSettings(
                                                   g_dvrTest->streamPath[pathIndex].dataInjectionService,
                                                   &settings);
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,&pkt[0],readSize);
            }
            else
            {
                if(g_dvrTest->streamPath[pathIndex].recordService)
                {
                    B_DVR_RecordService_InjectDataStart(g_dvrTest->streamPath[pathIndex].recordService,&pkt[0],readSize);
                    
                }
                else
                {
                    printf("\n neither record or tsb is active on path %d",pathIndex);
                }
            }
        }
        break;
    case eDvrTest_OperationDataInjectionStop:
        {
            
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);

            }
            else
            {
                if(g_dvrTest->streamPath[pathIndex].recordService)
                 {
                     B_DVR_RecordService_InjectDataStop(g_dvrTest->streamPath[pathIndex].recordService);
                     
                 }
                else
                {
                    printf("\n neither record or tsb is active on path %d",pathIndex);
                }
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
		case eDvrTest_OperationPlayGop:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationPlayGop;
					operationSettings.operationSpeed = NEXUS_NORMAL_PLAY_SPEED;
					operationSettings.mode_modifier = 8;/* send 8 frames per GOP, every GOP, (-) for reverse, (+) for forward */
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
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationSlowRewind;
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
		case eDvrTest_OperationSeek:
	        {
				if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
					B_DVR_OperationSettings operationSettings;
					B_DVR_PlaybackServiceStatus playbackServiceStatus;
					B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[pathIndex].playbackService, &playbackServiceStatus);
					operationSettings.seekTime = param->seekTime;
					operationSettings.operation = eB_DVR_OperationSeek;
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
		case eDvrTest_OperationMediaFileRead:
            {   
                char programName[B_DVR_MAX_FILE_NAME_LENGTH];
                B_DVR_MediaFileOpenMode openMode;
                B_DVR_MediaFilePlayOpenSettings openSettings;
                B_DVR_MediaFileStats mediaFileStats;
                B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
                unsigned long bufSize;
                ssize_t returnSize;
                off_t readOffset,returnOffset;
                void *buffer;
                int fd;
                fflush(stdin);
                printf("\n Enter sub folder name:");
                scanf("%s",openSettings.subDir);
                printf("\n Enter programName:");
                scanf("%s",programName);
                openMode = eB_DVR_MediaFileOpenModeStreaming;
                openSettings.playpumpIndex = 3;
                openSettings.volumeIndex = 0;
                bufSize = 188*4096;
                g_dvrTest->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
                if(!g_dvrTest->mediaFile)
                {
                    printf("\n error in opening mediaFile");
                    goto error_mediaFile_open;
                }
                fd = open("/mnt/nfs/nonseg.ts",O_CREAT|O_WRONLY,0666); 
                B_DVR_MediaFile_Stats(g_dvrTest->mediaFile,&mediaFileStats);
                buffer = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
                g_dvrTest->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);
                if(!g_dvrTest->buffer)
                {
                    printf("\n error_buf_alloc");
                    goto error_buf_alloc;
                }
                readOffset = mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
                mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
                returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->mediaFile,&mediaFileSeekSettings);
                if(returnOffset < 0)
                {
                    printf("\n error seeking");
                    goto error_seek;
                }


               for(;readOffset<mediaFileStats.endOffset;readOffset+=bufSize) 
               {
                   returnSize = B_DVR_MediaFile_Read(g_dvrTest->mediaFile,g_dvrTest->buffer,bufSize);
                   if(returnSize < 0)
                   {
                       printf("\n Error reading");
                       close(fd);
                       break;
                   }
                   else
                   {
                       write(fd, (void*)g_dvrTest->buffer,returnSize);
                       printf("\n read %u",returnSize);
                   }
                 
               }
            error_seek:   
               BKNI_Free(buffer);
               g_dvrTest->buffer = NULL;
            error_buf_alloc:
                B_DVR_MediaFile_Close(g_dvrTest->mediaFile);
                g_dvrTest->mediaFile = NULL;
            error_mediaFile_open:   
                printf("\n resuming other operations");
            }
            break;
        case eDvrTest_OperationMediaFileWrite:
            {
                char programName[B_DVR_MAX_FILE_NAME_LENGTH]="brcmseg";
                char nfsFileName[B_DVR_MAX_FILE_NAME_LENGTH]="trp_008_spiderman_lotr_oceans11_480i_q64.mpg";
                B_DVR_MediaFileOpenMode openMode;
                B_DVR_MediaFilePlayOpenSettings openSettings;
                int nfsFileID;
                unsigned long bufSize;
                void *buffer;


/*//                fflush(stdin);
//                printf("\n enter the nfs source file");
//                scanf("%s",nfsFileName);*/
                if ((nfsFileID = open(nfsFileName, O_RDONLY,0666)) < 0)
                {
                    printf("\n unable to open %s",nfsFileName);
                    break;
                }
/*//                printf("\n enter a program name for network recording");
//                scanf("%s",programName);*/
                sprintf(openSettings.subDir,"netRecording");
                openMode = eB_DVR_MediaFileOpenModeRecord;
                openSettings.playpumpIndex = 0;
                openSettings.volumeIndex = 0;
                bufSize = 188*4096;
                buffer  = BKNI_Malloc(bufSize + B_DVR_IO_BLOCK_SIZE);
                if(!buffer)
                {
                    printf("\n unable to allocate source read buffer");
                    close(nfsFileID);
                    break;
                }
                g_dvrTest->buffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)buffer);

                g_dvrTest->mediaFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);

                if(!g_dvrTest->mediaFile)
                {
                    printf("\n unable to open dvr mediaFile %s",programName);
                    free(buffer);
                    close(nfsFileID);
                    break;
                }

                
                while (read(nfsFileID,g_dvrTest->buffer,bufSize)>0)
                {
                    B_DVR_MediaFile_Write(g_dvrTest->mediaFile,g_dvrTest->buffer,bufSize);
                }
                printf("\n writing the nfs file to multiple file segments");
                B_DVR_MediaFile_Close(g_dvrTest->mediaFile);
                close(nfsFileID);
                BKNI_Free(buffer);
            }
            break;
        case eDvrTest_OperationListRecordings:
            {
                char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
                char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
                unsigned recordingCount;
                unsigned index;
                unsigned esStreamIndex;
                B_DVR_Media media;
                B_DVR_MediaNodeSettings mediaNodeSettings;

                fflush(stdin);
                printf("\n Enter subDir in metaData Dir:");
                scanf("%s",subDir);
                mediaNodeSettings.subDir = &subDir[0];
                mediaNodeSettings.programName = NULL;
                mediaNodeSettings.volumeIndex = 0;
                recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
                printf("\n number of recordings %u",recordingCount);
                if(recordingCount)
                {
                    recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
                    B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
                    printf("\n********************************************************************");
                    printf("\n                       Recording List                               ");
                    printf("\n********************************************************************");
                    for(index=0;index<recordingCount;index++)
                    {
                        printf("\n program-%d - %s",index,recordingList[index]);
                        mediaNodeSettings.programName = recordingList[index];
                        B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
                        printf("\n program metaData file %s",media.mediaNodeFileName);
                        printf("\n media metaData file %s",media.mediaFileName);
                        printf("\n nav metaData file %s",media.navFileName);
                        printf("\n media Size %u",(unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset));
                        printf("\n nav size %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
                        printf("\n media time (seconds) %u",(unsigned)((media.mediaEndTime - media.mediaStartTime)/1000));
                        printf("\n ******************PidInfo*************");
                        for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++) 
                        {
                            if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
                            {
                                printf("\n %u Video PID:",esStreamIndex);
                            }
                            else
                            {
                                printf("\n %u Audio PID:",esStreamIndex);
                            }
                            printf(" %u",media.esStreamInfo[esStreamIndex].pid);
                        }
                    }
                    BKNI_Free(recordingList);
                }
                else
                {
                    printf("\n no recordings found");
                }
            }
            break;
#if defined(ENABLE_DRM)

		case eDvrTest_OperationListRecordServiceCpsClearKeyStart:
			{
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;

                B_DVR_RecordServiceInputEsStream inputEsStream;				
                B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                B_DVR_RecordServiceSettings recordServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
                NEXUS_PidChannelHandle videoPidchHandle;
            	NEXUS_PidChannelHandle audioPidchHandle;
				
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
				printf("\n recording name: %s", param->programName);
				strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
                g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                printf("\n recordService open <<");
                printf("\n recordService install callback >>");
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
				inputEsStream.pidChannel = NULL;
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
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
				g_dvrTest->streamPath[pathIndex].drmService[0] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  


				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmService[1] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  

				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
				g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
				g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[0] ,&g_dvrTest->drmServiceSettings);

				g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[1] ,&g_dvrTest->drmServiceSettings);


                mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
                mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
                mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
                printf("subDir(%s) programName(%s) volumeIndex(%d)",mediaNodeSettings.subDir,
                                                                        mediaNodeSettings.programName,
                                                                        mediaNodeSettings.volumeIndex);					

				
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
				

 				dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);					

                B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                printf("\n recordService cps enc start >>");				
			}
			break;

		case eDvrTest_OperationListRecordServiceCpsClearKeyStop:
			{
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;    
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                
                videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                      channelInfo[currentChannel].videoPids[0]);    

                audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                      channelInfo[currentChannel].audioPids[0]);    
                dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[0],videoPidchHandle);
                dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[1],audioPidchHandle);
                
				B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
				B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
				B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
				g_dvrTest->streamPath[pathIndex].recordService = NULL;
				g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
				
                dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[0]);
                dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[1]);
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
			}
			break;

		case eDvrTest_OperationListRecordServiceCpsKeyladderStart:
			{
				unsigned char totalKey[48];
				int index;				
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				B_DVR_RecordServiceInputEsStream inputEsStream; 
                B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                B_DVR_RecordServiceSettings recordServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
                NEXUS_PidChannelHandle videoPidchHandle;
            	NEXUS_PidChannelHandle audioPidchHandle;
				
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
				printf("\n recording name: %s", param->programName);
				strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
                g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                printf("\n recordService open <<");
                printf("\n recordService install callback >>");
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
				inputEsStream.pidChannel = NULL;
				B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);

				videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);
				
				inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
				inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
				inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
				B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);

				audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,inputEsStream.esStreamInfo.pid);

				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
				g_dvrTest->streamPath[pathIndex].drmService[2] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
				g_dvrTest->streamPath[pathIndex].drmService[3] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
				
				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
				g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
				g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
				g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
				g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[2] ,&g_dvrTest->drmServiceSettings);
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[3] ,&g_dvrTest->drmServiceSettings);


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
				         printf("[totalKey] %d = 0x%x\n",index,totalKey[index]);
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
				
                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[2],videoPidchHandle);
                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[3],audioPidchHandle);					
#if 0
				BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
				recordServiceSettings.esStreamCount = 2;
				recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
				recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
				B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
#endif
                B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                printf("\n recordService m2m enc start >>");					
			}
			break;

		case eDvrTest_OperationListRecordServiceCpsKeyladderStop:
			{
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;    
                unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                
                videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                      channelInfo[currentChannel].videoPids[0]);    

                audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
                                                                      channelInfo[currentChannel].audioPids[0]);    
                dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[2],videoPidchHandle);
                dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[3],audioPidchHandle);
                
				B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
				B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
				B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
				g_dvrTest->streamPath[pathIndex].recordService = NULL;
				g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
				
                dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[2]);
                dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[3]);
                BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));				
			}
			break;

		case eDvrTest_OperationListRecordServiceM2mClearKeyStart:
			{
                if((pathIndex > 0 && pathIndex < 2 ))
                {
                    printf("\n invalid path chosen. Path should be between 2-5");
                }
                else
                {
                    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
					B_DVR_RecordServiceInputEsStream inputEsStream; 
                    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                    B_DVR_RecordServiceSettings recordServiceSettings;
                    B_DVR_MediaNodeSettings mediaNodeSettings;
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
					printf("\n recording name: %s", param->programName);
					strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);
                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;					
                    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                    printf("\n recordService open <<");
                    printf("\n recordService install callback >>");
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
					inputEsStream.pidChannel = NULL;
					B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);
					inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
					inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
					inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
					B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&inputEsStream);

					g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoKeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
					g_dvrTest->streamPath[pathIndex].drmService[4] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  

					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[4] ,&g_dvrTest->drmServiceSettings);


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
														g_dvrTest->streamPath[pathIndex].drmService[4],0);


                    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                    printf("\n recordService m2m enc start >>");

                
                 }				
			}
			break;

		case eDvrTest_OperationListRecordServiceM2mClearKeyStop:
			{
                if(pathIndex > 0 && pathIndex < 2 )
                {
                    printf("\n invalid path chosen. Path should be between 2-5");
                }
                else
                {
                    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
					B_DVR_RecordService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].recordService,0);
                    B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[4]);
                    
                    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                    g_dvrTest->streamPath[pathIndex].recordService = NULL;
                    g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
                }				
			}
			break;

		case eDvrTest_OperationListRecordServiceM2mKeyladderStart:
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
					printf("\n recording name: %s", param->programName);
					strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);

                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;					
					printf("\n recordService open >>>");
					g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                    printf("\n recordService install callback >>");
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
					inputEsStream.pidChannel = NULL;
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
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
                    g_dvrTest->streamPath[pathIndex].drmService[5] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                    g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                    g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                    g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
					g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
					g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
                    dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[5] ,&g_dvrTest->drmServiceSettings);
					

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


					/*B_DVR_RecordService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].recordService,
														g_dvrTest->streamPath[pathIndex].drmService[5]);*/
#if 0
                    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
                    recordServiceSettings.esStreamCount = 2;
                    recordServiceSettings.RemapppedEsStreamInfo[0].pid = 0x1ff2;
                    recordServiceSettings.RemapppedEsStreamInfo[1].pid = 0x1ff3;
                    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
#endif
                    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                    printf("\n recordService start <<");

				}				
			}
			break;

		case eDvrTest_OperationListRecordServiceM2mKeyladderStop:
			{
                if(pathIndex > 0 && pathIndex < 2 )
                {
                    printf("\n invalid path chosen. Path should be between 2-5");
                }
                else
                {
                    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
					/*B_DVR_RecordService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].recordService);*/
                    B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[5]);
                    
                    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                    g_dvrTest->streamPath[pathIndex].recordService = NULL;
                    g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
                }				
			}
			break;

	        case eDvrTest_OperationListPlaybackServiceM2mClearKeyStart:
			{
				B_DVR_MediaNodeSettings mediaNodeSettings; 
				B_DVR_Media mNode;
				unsigned keylength;
				unsigned char key[16];
				int count;
				int drmIndex;
				bool tsbPlayback = false;

			    if(g_dvrTest->streamPath[pathIndex].tsbService)
	            {
	                printf("\n Stop TSB service running on this path %d",pathIndex);
	                goto error_m2mplaybackService;
	            }
				
				strncpy(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);					
				sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir,"bgrec");	
				
				g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
				
				mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
				mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
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
					DvrTest_LiveDecodeStop(tc,pathIndex);				
				}
				
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
				g_dvrTest->streamPath[pathIndex].drmService[8] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
				printf("\nDRM Service for playback Opened\n");
				
				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
				g_dvrTest->drmServiceSettings.keys = key;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[8] ,&g_dvrTest->drmServiceSettings);				
				printf("\nDRM Service for playback Configured\n");				
				
				drmIndex =8;
				DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,tsbPlayback);
				printf("\n DvrTest_PlaybackDecodeStartM2m");
				B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
				printf("\n B_DVR_PlaybackService_Start");

			}
			error_m2mplaybackService:
			break;
		case eDvrTest_OperationListPlaybackServiceM2mClearKeyStop:
			{
				B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
				B_DVR_PlaybackService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].playbackService);
				
				DvrTest_PlaybackDecodeStop(tc,pathIndex);						
				B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[8]);
				
				B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
				g_dvrTest->streamPath[pathIndex].playbackService = NULL;
				
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}

			}
		break;
	    case eDvrTest_OperationListPlaybackServiceM2mKeyladderStart:
			{
				B_DVR_MediaNodeSettings mediaNodeSettings; 
				B_DVR_Media mNode;
				unsigned keylength;
				unsigned char key[48];
				unsigned char session[16];
				unsigned char controlWd[16];
				unsigned char iv[16];
				int count;
				int drmIndex;
				bool tsbPlayback = false;
				printf("\n Enter the recording name: %s", param->programName);
				strncpy(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);					
				sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir,"bgrec");	

				g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
				
				mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
				mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
				mediaNodeSettings.volumeIndex = 0;
				
				B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);
				B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);
				B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);
				
				printf("\nkeylength = %d\n",keylength);
				
				for(count=0;count<(int)keylength;count++)
				{
				   printf("key(from node) %d : [0x%x]\n",count,key[count]);
				   if (count<16) session[count] = key[count];
				   else if (count >=16 && count < 32) controlWd[count-(keylength/3)] = key[count];
				   else if (count >= 32) iv[count - (keylength-16)] = key[count];
				}
				
				g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
				printf("\n B_DVR_PlaybackService_Open");
				
				if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStop(tc,pathIndex);				
				}
				
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
				g_dvrTest->streamPath[pathIndex].drmService[9] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
				printf("\nDRM Service for playback Opened\n");
				
				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
				g_dvrTest->drmServiceSettings.sessionKeys = session;
				g_dvrTest->drmServiceSettings.keys = controlWd; 
				g_dvrTest->drmServiceSettings.ivKeys = iv;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[9] ,&g_dvrTest->drmServiceSettings);
				
				printf("\nDRM Service for playback Configured\n");				

				drmIndex = 9;					
				DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,tsbPlayback);
				printf("\n DvrTest_PlaybackDecodeStartDrm");
				B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
				printf("\n B_DVR_PlaybackService_Start");

				DvrTest_PlaybackHostTrick(pathIndex);

			}
			break;
	    case eDvrTest_OperationListPlaybackServiceM2mKeyladderStop:		
			{
				B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
				B_DVR_PlaybackService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].playbackService);
				
				DvrTest_PlaybackDecodeStop(tc,pathIndex);						
				B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[9]);
				
				B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
				g_dvrTest->streamPath[pathIndex].playbackService = NULL;
				
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}

			}
			break;

		case eDvrTest_OperationTSBServiceCpsClearKeyStart:
			{
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				B_DVR_TSBServiceInputEsStream inputEsStream; 
				B_DVR_TSBServiceSettings tsbServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;				
				B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc, pathIndex);
				}
				printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
				BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
				BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
							  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
							  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
				printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
				sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
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
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
                g_dvrTest->streamPath[pathIndex].drmService[20] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

                g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
                g_dvrTest->streamPath[pathIndex].drmService[21] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest); 
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

                g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
                g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
                g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
                g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
                g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
                dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[20] ,&g_dvrTest->drmServiceSettings);

				g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[21] ,&g_dvrTest->drmServiceSettings);

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

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[21],audioPidchHandle);				
				
				dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
				if(dvrError!=B_DVR_SUCCESS)
				{
					printf("\n Error in starting the tsbService %d",pathIndex);
				}
				else
				{
					printf("tsbService started using CPS clearkey %d\n",pathIndex);
				}

			}
			break;

		case eDvrTest_OperationTSBServiceCpsClearKeyStop:
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
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[21],audioPidchHandle);
                    
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(tc,pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[20]);
                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[21]);
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

		case eDvrTest_OperationTSBServiceCpsKeyladderStart:
			{
				unsigned char totalKey[48];
				int index;					
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				B_DVR_TSBServiceInputEsStream inputEsStream; 
				B_DVR_TSBServiceSettings tsbServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
                NEXUS_PidChannelHandle videoPidchHandle;
                NEXUS_PidChannelHandle audioPidchHandle;				
				B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc, pathIndex);
				}
				printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
				BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
				BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
							  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
							  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
				printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
				sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
				g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
				B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
				
				BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
				dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
				g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
				tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
				tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
				B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
                g_dvrTest->streamPath[pathIndex].drmService[22] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                
                g_dvrTest->streamPath[pathIndex].drmService[23] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest); 
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

				g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
				g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
				g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
				g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eClear;
				g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
				g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
				g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;				
                dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[22] ,&g_dvrTest->drmServiceSettings);
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[23] ,&g_dvrTest->drmServiceSettings);

                mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
                mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
                mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
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

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],videoPidchHandle);

                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[23],audioPidchHandle);				
				
				dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
				if(dvrError!=B_DVR_SUCCESS)
				{
					printf("\n Error in starting the tsbService %d",pathIndex);
				}
				else
				{
					printf("tsbService started using CPS keyladder  %d\n",pathIndex);
				}

			}
			break;

		case eDvrTest_OperationTSBServiceCpsKeyladderStop:
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
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],videoPidchHandle);
                    dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[23],audioPidchHandle);
                    
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(tc,pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[22]);
                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[23]);
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


			case eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStart:
			{
				 B_DVR_MediaNodeSettings mediaNodeSettings; 
				 B_DVR_Media mNode;
				 unsigned keylength;
				 unsigned char key[16];
				 int count;
				 int drmIndex;
				 bool tsbPlayback = true;
			
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
						 DvrTest_LiveDecodeStop(tc,pathIndex);
					 }
			
					sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");	
					
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
					mediaNodeSettings.volumeIndex = 0;
					
					B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);				
					B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength); 			
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);					 
			
					for(count=0;count<(int)keylength;count++)
					{
					   BDBG_MSG(("key %d = [0x%x] \n",count,key[count]));
					}
			
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma2;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
					 g_dvrTest->streamPath[pathIndex].drmService[30] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				 
					 printf("\nDRM Service for tsb playback Opened\n");
			
					 g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					 g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					 g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					 g_dvrTest->drmServiceSettings.keys = key;
					 g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[30] ,&g_dvrTest->drmServiceSettings);
					 printf("\nDRM Service for tsb playback Configured\n"); 			 
				
					 drmIndex =30;
					 DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,tsbPlayback);
			
					 printf("\ntsb pause before play start\n");
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 operationSettings.operation = eB_DVR_OperationPause; 
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 BKNI_Sleep(2000);
					 printf("\ntsb play start \n"); 				
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 operationSettings.operation = eB_DVR_OperationPlay; 
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 DvrTest_PlaybackHostTrick(pathIndex);					 
			
				}
				
			
			}
			break;
			
			case eDvrTest_OperationTSBServiceM2mClearKeyPlaybackStop:
			{
			
				 int tsbPlayback = true;
				 NEXUS_PidChannelHandle videoPidchHandle;
				 NEXUS_PidChannelHandle audioPidchHandle;	 
				 unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
			
			
				 if(!g_dvrTest->streamPath[pathIndex].tsbService)
				 {
					 printf("\n error TSB Play is not started");
					 break;
				 }
				 else
				 {
			
					 videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].videoPids[0]);	 
					 
					 audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].audioPids[0]);	 
			
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[20],videoPidchHandle);
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[21],audioPidchHandle);
			
					 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					 dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);					 
			
					 if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
					 {
						 DvrTest_PlaybackDecodeStop(tc,pathIndex);
					 }
			
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[20]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[21]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[30]); 		
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
			
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}
			
			}
			break;
			
			case eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStart:
			{
				 B_DVR_MediaNodeSettings mediaNodeSettings; 
				 B_DVR_Media mNode;
				 unsigned keylength;
				 unsigned char key[48];
				 unsigned char session[16];
				 unsigned char controlWd[16];
				 unsigned char iv[16];
				 int count;
				 int drmIndex;
				 bool tsbPlayback = true;
			
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
						 DvrTest_LiveDecodeStop(tc,pathIndex);
					 }
			
					sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");	
					
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
					mediaNodeSettings.volumeIndex = 0;
					
					B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);				
					B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength); 			
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);					 
			
					printf("\nkeylength = %d\n",keylength);
					
					for(count=0;count<(int)keylength;count++)
					{
					   printf("key(from node) %d : [0x%x]\n",count,key[count]);
					   if (count<16) session[count] = key[count];
					   else if (count >=16 && count < 32) controlWd[count-(keylength/3)] = key[count];
					   else if (count >= 32) iv[count - (keylength-16)] = key[count];
					}
			
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma2;
					 g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
					 g_dvrTest->streamPath[pathIndex].drmService[31] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				 
					 printf("\nDRM Service for tsb playback Opened\n");
			
					 g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					 g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					 g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
					 g_dvrTest->drmServiceSettings.sessionKeys = session;
					 g_dvrTest->drmServiceSettings.keys = controlWd; 
					 g_dvrTest->drmServiceSettings.ivKeys = iv;
					 g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					 dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[31] ,&g_dvrTest->drmServiceSettings);
					 printf("\nDRM Service for tsb playback Configured\n"); 			 
				
					 drmIndex =31;
					 DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem,tsbPlayback);
			
					 printf("\ntsb pause before play start\n");
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 operationSettings.operation = eB_DVR_OperationPause; 
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 BKNI_Sleep(2000);
					 printf("\ntsb play start \n"); 				
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					 operationSettings.operation = eB_DVR_OperationPlay; 
					 B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings); 
					 DvrTest_PlaybackHostTrick(pathIndex);
			
				}
			}
			break;
			
			case eDvrTest_OperationTSBServiceM2mKeyladderPlaybackStop:
			{
				 int tsbPlayback = true;
				 NEXUS_PidChannelHandle videoPidchHandle;
				 NEXUS_PidChannelHandle audioPidchHandle;	 
				 unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
			
			
				 if(!g_dvrTest->streamPath[pathIndex].tsbService)
				 {
					 printf("\n error TSB Play is not started");
					 break;
				 }
				 else
				 {
			
					 videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].videoPids[0]);	 
					 
					 audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].audioPids[0]);	 
			
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[22],videoPidchHandle);
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[23],audioPidchHandle);
			
					 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					 dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);					 
			
					 if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
					 {
						 DvrTest_PlaybackDecodeStop(tc,pathIndex);
					 }
			
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[22]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[23]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[31]); 		
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
			
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}
			
			}
			break;

		
			case eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStart:
				{
					unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
					B_DVR_RecordServiceInputEsStream inputEsStream; 
					B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
					B_DVR_RecordServiceSettings recordServiceSettings;
					B_DVR_MediaNodeSettings mediaNodeSettings;
					NEXUS_PidChannelHandle videoPidchHandle;
					NEXUS_PidChannelHandle audioPidchHandle;
					unsigned char localkey[32];
					int count=0;
					
					BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
					printf("\n recording name: %s", param->programName);
					strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);
					g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
					sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
					g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
					printf("\n recordService open >>>");
					g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
					printf("\n recordService open <<");
					printf("\n recordService install callback >>");
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
					inputEsStream.pidChannel = NULL;
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
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceRecord;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoEvenKeyLoaderCallback;					
					g_dvrTest->streamPath[pathIndex].drmService[30] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = videoOddKeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmService[31] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioEvenKeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmService[32] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioOddKeyLoaderCallback;
					g_dvrTest->streamPath[pathIndex].drmService[33] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);  
					
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
					g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eEven;
					g_dvrTest->drmServiceSettings.keys = changeToLocalKeyVideoEven;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[30] ,&g_dvrTest->drmServiceSettings);
			
					g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;	
					g_dvrTest->drmServiceSettings.keys = changeToLocalKeyVideoOdd;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[31] ,&g_dvrTest->drmServiceSettings);
			
					g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eEven; 	
					g_dvrTest->drmServiceSettings.keys = changeToLocalKeyAudioEven;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[32] ,&g_dvrTest->drmServiceSettings);
			
					g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;	
					g_dvrTest->drmServiceSettings.keys = changeToLocalKeyAudioOdd;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[33] ,&g_dvrTest->drmServiceSettings);
					
					mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir[0];
					mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName[0];
					mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex;
					printf("subDir(%s) programName(%s) volumeIndex(%d)\n",mediaNodeSettings.subDir,
																			mediaNodeSettings.programName,
																			mediaNodeSettings.volumeIndex); 				
			
					for(count=0; count<(KEY_LENGTH*2);count++)
					{
					   (count<16)? (localkey[count] = changeToLocalKeyVideoEven[count]) :(localkey[count] = changeToLocalKeyVideoOdd[count-KEY_LENGTH]);
					   printf("video localkey[%d] = 0x%x\n",count,localkey[count]);
					}
					
					dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*2));				
					dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
																		&mediaNodeSettings,
																		channelInfo[currentChannel].videoPids[0],
																		localkey);
					
					for(count =0; count<(KEY_LENGTH*2);count++)
					{
					   (count<16)? (localkey[count] = changeToLocalKeyAudioEven[count]) :(localkey[count] = changeToLocalKeyAudioOdd[count-KEY_LENGTH]);
					   printf("audio localkey[%d] = 0x%x\n",count,localkey[count]);
					}
					dvrError = B_DVR_Manager_SetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,(KEY_LENGTH*2));				
					dvrError = B_DVR_Manager_SetKeyBlobPerEsStream(g_dvrTest->dvrManager,
																		&mediaNodeSettings,
																		channelInfo[currentChannel].audioPids[0],
																		localkey);					
					
					dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[30],videoPidchHandle);
					dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[32],audioPidchHandle);				
			
					B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
					printf("\n recordService cps enc start >>");				
					printf("\n Key change between even and odd.");
					printf("\n Start various keyloading \n");
					
					for(count=0;count<30;count++)
					{
						if(count%2)
						{
							printf(" loaded even keys \n");
							dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[30],videoPidchHandle);
							dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[32],audioPidchHandle);				
							BKNI_Sleep(1000);
						}
						else
						{
							printf(" loaded odd keys\n");
							dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[31],videoPidchHandle);
							dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[33],audioPidchHandle);				
							BKNI_Sleep(1000);
						}
					}
				}
				break;
			
			case eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStop:
				{
					NEXUS_PidChannelHandle videoPidchHandle;
					NEXUS_PidChannelHandle audioPidchHandle;	
					unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
					
					videoPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
																		  channelInfo[currentChannel].videoPids[0]);	
			
					audioPidchHandle = B_DVR_RecordService_GetPidChannel(g_dvrTest->streamPath[pathIndex].recordService,
																		  channelInfo[currentChannel].audioPids[0]);	
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[30],videoPidchHandle);
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[31],audioPidchHandle);
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[32],videoPidchHandle);
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[33],audioPidchHandle);
					
					B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
					B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
					B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
					g_dvrTest->streamPath[pathIndex].recordService = NULL;
					g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;  
			
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[30]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[31]); 					
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[32]);
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[33]);
					BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].drmServiceRequest,0,sizeof(B_DVR_DRMServiceRequest));
				}
				break;
			
			case eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStart:			
				{
					B_DVR_MediaNodeSettings mediaNodeSettings; 
					B_DVR_Media mNode;
					unsigned keylength;
					unsigned char vKey[16];
					unsigned char aKey[16];
					int count;
					int drmIndex=0;
					bool tsbPlayback = false;
					printf("\n Enter the recording name: %s\n", param->programName);
					strncpy(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, param->programName,B_DVR_MAX_FILE_NAME_LENGTH);	
					sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir,"bgrec");				
					g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName;
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
						DvrTest_LiveDecodeStop(tc,pathIndex);				
					}
			
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;
					g_dvrTest->streamPath[pathIndex].drmService[34] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					g_dvrTest->streamPath[pathIndex].drmService[35] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);			
					
					printf("\nDRM Service for ca clearkey playback Opened\n");
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
					g_dvrTest->drmServiceSettings.keys = vKey;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[34] ,&g_dvrTest->drmServiceSettings);
					
					g_dvrTest->drmServiceSettings.keys = aKey;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[35] ,&g_dvrTest->drmServiceSettings);
					printf("\nDRM Service for ca clearkey  playback Configured\n"); 			
			
					drmIndex = 34;
					printf("\n DvrTest_PlaybackDecodeStartDrm");
					DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eCa,tsbPlayback);
			
				
					B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
					printf("\n B_DVR_PlaybackService_Start");
			
					printf("\n Trick test ");
					DvrTest_PlaybackHostTrick(pathIndex);
			
				}
				break;
			
			case eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStop:
				{
					B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[34],
														   g_dvrTest->streamPath[pathIndex].videoPlaybackPidChannels[0]);
					
					B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[35],
														   g_dvrTest->streamPath[pathIndex].audioPlaybackPidChannels[0]);
					 
					 B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
					 DvrTest_PlaybackDecodeStop(tc,pathIndex);
					 
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[34]);
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[35]);
					 
					 B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
					 g_dvrTest->streamPath[pathIndex].playbackService = NULL;
			
					 if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					 {
						 DvrTest_LiveDecodeStart(tc,pathIndex);
					 }
				}
				break;


		case eDvrTest_OperationTSBServiceM2mClearKeyStart:
			{
				bool tsbPlayback = false;
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				B_DVR_TSBServiceInputEsStream inputEsStream;
				B_DVR_TSBServiceSettings tsbServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
				B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc, pathIndex);
				}
				printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
				BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
				BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
							  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
							  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
				printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
				sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
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
                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
                g_dvrTest->streamPath[pathIndex].drmService[24] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

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
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[24] ,&g_dvrTest->drmServiceSettings);
				

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
																g_dvrTest->streamPath[pathIndex].drmService[24],
																tsbPlayback);

				
				dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
				if(dvrError!=B_DVR_SUCCESS)
				{
					printf("\n Error in starting the tsbService %d",pathIndex);
				}
				else
				{
					printf("tsbService started using m2m clearkey %d\n",pathIndex);
				}

			}
			break;
			
		case eDvrTest_OperationTSBServiceM2mClearKeyStop:
			{
				int tsbPlayback = false;;
				
                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {   
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(tc,pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[24]);
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
			
		case eDvrTest_OperationTSBServiceM2mKeyladderStart:
			{
				unsigned char totalKey[48];
				int index;
				bool tsbPlayback = false;
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
				B_DVR_TSBServiceInputEsStream inputEsStream;
				B_DVR_TSBServiceSettings tsbServiceSettings;
                B_DVR_MediaNodeSettings mediaNodeSettings;
				B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc, pathIndex);
				}
				printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
				BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
				BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
							  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
							  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
				printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
				sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
				g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
				g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
				B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
				
				BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
				dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
				g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
				tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
				tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
				B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

				g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eM2m;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
				g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = g_dvrTest->dma;
                g_dvrTest->streamPath[pathIndex].drmService[25] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

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
				g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
				g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
				g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
				g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
				dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[25] ,&g_dvrTest->drmServiceSettings);

                mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
                mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
                mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
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
				
				dvrError = B_DVR_TSBService_AddDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,
																g_dvrTest->streamPath[pathIndex].drmService[25],
																tsbPlayback);

				
				dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
				if(dvrError!=B_DVR_SUCCESS)
				{
					printf("\n Error in starting the tsbService %d",pathIndex);
				}
				else
				{
					printf("tsbService started using m2m keyladder %d\n",pathIndex);
				}

			}
			break;
			
		case eDvrTest_OperationTSBServiceM2mKeyladderStop:
			{
				int tsbPlayback = false;;
				
                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {   
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                    {
                        DvrTest_PlaybackDecodeStop(tc,pathIndex);
                    }
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in stopping the timeshift service %d\n",pathIndex);
                    }

                    dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[25]);
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
			case eDvrTest_OperationTSBServiceCaClearKeyPlaybackStart:
			{
                B_DVR_MediaNodeSettings mediaNodeSettings; 
                B_DVR_Media mNode;
                unsigned keylength;
                unsigned char vKey[16];
                unsigned char aKey[16];
                int count;
				bool tsbPlayback = false;
				printf("pathIndex = %d",pathIndex);
				if(!g_dvrTest->streamPath[pathIndex].tsbService)
				{
					printf("\n error TSBPlayStart without TSBServiceStart");
					break;
				}
				else
				{
					int drmIndex=0;			
					B_DVR_OperationSettings operationSettings;
					B_DVR_TSBServiceStatus tsbServiceStatus;
					B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
					operationSettings.operation = eB_DVR_OperationTSBPlayStart;
					operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
				
					if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					{
						DvrTest_LiveDecodeStop(tc,pathIndex);
					}

					sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");	
					
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
					mediaNodeSettings.volumeIndex = 0;
					
					B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);					
					B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);					
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,vKey);
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[1].pid,aKey);

					for(count=0;count<(int)keylength;count++)
					{
					   printf("[%d] vKey[0x%x] , aKey[0x%x] \n",count,vKey[count],aKey[count]);
					}

					g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeClear;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
					g_dvrTest->streamPath[pathIndex].drmService[42] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				
					g_dvrTest->streamPath[pathIndex].drmService[43] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				
				
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
					g_dvrTest->drmServiceSettings.keys = vKey;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[42] ,&g_dvrTest->drmServiceSettings);
					g_dvrTest->drmServiceSettings.keys = aKey;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[43] ,&g_dvrTest->drmServiceSettings);
					printf("\nDRM Service for tsb playback Configured\n");				

					drmIndex = 42;
					DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eCa,tsbPlayback);

					printf("\ntsb pause before play start\n");
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					operationSettings.operation = eB_DVR_OperationPause; 
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					BKNI_Sleep(2000);
					printf("\ntsb play start \n");				   
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					operationSettings.operation = eB_DVR_OperationPlay; 
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings); 
					DvrTest_PlaybackHostTrick(pathIndex);

					}

			}
			break;

			case eDvrTest_OperationTSBServiceCaClearKeyPlaybackStop:
			{

				NEXUS_PidChannelHandle videoPidchHandle;
				NEXUS_PidChannelHandle audioPidchHandle;

						unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						
				if(!g_dvrTest->streamPath[pathIndex].tsbService)
	            {
	                printf("\n error TSBPlayStart without TSBServiceStart");
	                break;
	            }
	            else
	            {
					dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);	            
						videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																			  channelInfo[currentChannel].videoPids[0]);	
						
						audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																			  channelInfo[currentChannel].audioPids[0]);	
				
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[40],videoPidchHandle);
					dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[41],audioPidchHandle);
				
					B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[42],
	                                                       g_dvrTest->streamPath[pathIndex].videoPlaybackPidChannels[0]);
	                
	                B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[43],
	                                                       g_dvrTest->streamPath[pathIndex].audioPlaybackPidChannels[0]);

	                B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[40]);
	                B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[41]);
	                B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[42]);
	                B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[43]);

					if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
					{
	                	DvrTest_PlaybackDecodeStop(tc, pathIndex);    
					}
					
	                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
	                {
	                    DvrTest_LiveDecodeStart(tc, pathIndex);
	                }
				
		}

			}
			break;

			case eDvrTest_OperationTSBServiceCaKeyladderPlaybackStart:
			{
				B_DVR_MediaNodeSettings mediaNodeSettings; 
				B_DVR_Media mNode;
				unsigned keylength;
				unsigned char key[48];
				unsigned char session[16];
				unsigned char controlWd[16];
				unsigned char iv[16];
				int count;
				bool tsbPlayback = false;
				printf("pathIndex = %d",pathIndex);
				if(!g_dvrTest->streamPath[pathIndex].tsbService)
				{
					printf("\n error TSBPlayStart without TSBServiceStart");
					break;
				}
				else
				{
					int drmIndex=0; 		
					B_DVR_OperationSettings operationSettings;
					B_DVR_TSBServiceStatus tsbServiceStatus;
					B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
					operationSettings.operation = eB_DVR_OperationTSBPlayStart;
					operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
				
					if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					{
				
						DvrTest_LiveDecodeStop(tc,pathIndex);
					}
	
					sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");	
					
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;	
					
					mediaNodeSettings.subDir = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir;
					mediaNodeSettings.programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
					mediaNodeSettings.volumeIndex = 0;
					
					B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&mNode);					
					B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keylength);					
					B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,mNode.esStreamInfo[0].pid,key);
		
					printf("\nkeylength = %d\n",keylength);

					for(count=0;count<(int)keylength;count++)
					{
					   printf("key(from node) %d : [0x%x]\n",count,key[count]);
					   if (count<16) session[count] = key[count];
					   else if (count >=16 && count < 32) controlWd[count-(keylength/3)] = key[count];
					   else if (count >= 32) iv[count - (keylength-16)] = key[count];
					}

					g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;		
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = NULL;					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCa;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServicePlayback;					
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;					
					g_dvrTest->streamPath[pathIndex].drmService[46] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				
					g_dvrTest->streamPath[pathIndex].drmService[47] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);				
				
					g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
					g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
					g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCa;
					g_dvrTest->drmServiceSettings.sessionKeys = session;
					g_dvrTest->drmServiceSettings.keys = controlWd; 
					g_dvrTest->drmServiceSettings.ivKeys = iv;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eDecrypt;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[46] ,&g_dvrTest->drmServiceSettings);
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[47] ,&g_dvrTest->drmServiceSettings);
					printf("\nDRM Service for tsb playback Configured\n");				

					drmIndex = 46;
					DvrTest_PlaybackDecodeStartDrm(tc,pathIndex,drmIndex,NEXUS_SecurityAlgorithmConfigDestination_eCa,tsbPlayback);
					
					printf("\ntsb pause before play start\n");
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					operationSettings.operation = eB_DVR_OperationPause; 
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					BKNI_Sleep(2000);
					printf("\ntsb play start \n");				   
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					operationSettings.operation = eB_DVR_OperationPlay; 
					B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
					DvrTest_PlaybackHostTrick(pathIndex);
					}

			}
			break;

			case eDvrTest_OperationTSBServiceCaKeyladderPlaybackStop:
			{
				NEXUS_PidChannelHandle videoPidchHandle;
				NEXUS_PidChannelHandle audioPidchHandle;
                B_DVR_OperationSettings operationSettings;
				unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;

				if(!g_dvrTest->streamPath[pathIndex].tsbService)
	            {
	                printf("\n error TSBPlayStart without TSBServiceStart");
	                break;
	            }
	            else
	            {

					 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService); 			 
					 videoPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].videoPids[0]);	 
					 
					 audioPidchHandle = B_DVR_TSBService_GetPidChannel(g_dvrTest->streamPath[pathIndex].tsbService,
																		   channelInfo[currentChannel].audioPids[0]);
					 
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[44],videoPidchHandle);
					 dvrError = B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[45],audioPidchHandle);

	                operationSettings.operation = eB_DVR_OperationTSBPlayStop;
					 B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[46],
	                                                       g_dvrTest->streamPath[pathIndex].videoPlaybackPidChannels[0]);
	                
					 B_DVR_DRMService_RemovePidChannel(g_dvrTest->streamPath[pathIndex].drmService[47],
	                                                       g_dvrTest->streamPath[pathIndex].audioPlaybackPidChannels[0]);
					 
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[44]);
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[45]);
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[46]);
					 B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[47]);
					
	                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
	                DvrTest_PlaybackDecodeStop(tc, pathIndex);                    
	                if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
	                {
	                    DvrTest_LiveDecodeStart(tc, pathIndex);
	                }
	                
     	       }
				
			}
			break;

			case eDvrTest_OperationTSBServiceM2mToM2mClearKeyPlaybackStop:
			{
			
				 int tsbPlayback = true;
				 if(!g_dvrTest->streamPath[pathIndex].tsbService)
				 {
					 printf("\n error TSB Play is not started");
					 break;
				 }
				 else
				 {
			
					 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					 dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);					 
			
					 if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
					 {
						 DvrTest_PlaybackDecodeStop(tc,pathIndex);
					 }
			
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[24]); /* DRM handle for M2M record */
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[30]); /* DRM handle for M2M playback */		
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
			
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}
			
			}
			break;

 			case eDvrTest_OperationTSBServiceM2mToM2mKeyladderPlaybackStop:
			{
			
				 int tsbPlayback = true;
			
				 if(!g_dvrTest->streamPath[pathIndex].tsbService)
				 {
					 printf("\n error TSB Play is not started");
					 break;
				 }
				 else
				 {
			
					 dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
					 dvrError = B_DVR_TSBService_RemoveDrmSettings(g_dvrTest->streamPath[pathIndex].tsbService,tsbPlayback);					 
			
					 if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
					 {
						 DvrTest_PlaybackDecodeStop(tc,pathIndex);
					 }
			
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[25]); /* DRM handle for M2M record */
					dvrError = B_DVR_DRMService_Close(g_dvrTest->streamPath[pathIndex].drmService[31]); /* DRM handle for M2M playback */		
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
			
				if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
				{
					DvrTest_LiveDecodeStart(tc,pathIndex);
				}
			
			}
			break;

			case eDvrTest_OperationTSBServiceCpsClearKeyStartForCa:
				{
					unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
					B_DVR_TSBServiceInputEsStream inputEsStream;
					B_DVR_TSBServiceSettings tsbServiceSettings;
	                B_DVR_MediaNodeSettings mediaNodeSettings;
	                NEXUS_PidChannelHandle videoPidchHandle;
	                NEXUS_PidChannelHandle audioPidchHandle;				
					B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
					if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
					{
						DvrTest_LiveDecodeStart(tc, pathIndex);
					}
					printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
					BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
					BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
								  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
								  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
					printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
					sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
					g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
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
	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
					g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
	                g_dvrTest->streamPath[pathIndex].drmService[40] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                

	                g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = audioKeyLoaderCallback;
	                g_dvrTest->streamPath[pathIndex].drmService[41] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest); 
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

	                g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
	                g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
	                g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
	                g_dvrTest->drmServiceSettings.keys = changeToLocalKey;
	                g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;
					g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;
	                dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[40] ,&g_dvrTest->drmServiceSettings);

					g_dvrTest->drmServiceSettings.keys = changeToLocalKey2;
					dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[41] ,&g_dvrTest->drmServiceSettings);

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

	                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[40],videoPidchHandle);

	                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[41],audioPidchHandle);				
					
					dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
					if(dvrError!=B_DVR_SUCCESS)
					{
						printf("\n Error in starting the tsbService %d",pathIndex);
					}
					else
					{
						printf("tsbService started using CPS clearkey %d\n",pathIndex);
					}

				}
				break;		

				case eDvrTest_OperationTSBServiceCpsKeyladderStartForCa:
					{
						unsigned char totalKey[48];
						int index;					
						unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
						B_DVR_TSBServiceInputEsStream inputEsStream;						
						B_DVR_TSBServiceSettings tsbServiceSettings;
		                B_DVR_MediaNodeSettings mediaNodeSettings;
		                NEXUS_PidChannelHandle videoPidchHandle;
		                NEXUS_PidChannelHandle audioPidchHandle;				
						B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
						if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
						{
							DvrTest_LiveDecodeStart(tc, pathIndex);
						}
						printf("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex);
						BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
						BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
									  sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
									  "%s%d",TSB_SERVICE_PREFIX,pathIndex);
						printf("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName);
						g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
						sprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,"tsb");
						g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
						g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
						g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
						g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
						B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
						
						BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
						dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
						g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
						tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
						tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
						B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

						g_dvrTest->streamPath[pathIndex].drmServiceRequest.drmServiceType = eB_DVR_DRMServiceTypeBroadcastMedia;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.sessionKeyLoader = procInKey3_sessionKeyLoaderCallback;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.controlWordKeyLoader = procInKey4_KeyLoaderCallback;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.ivKeyLoader = iv_KeyLoaderCallback;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.keyType = eB_DVR_DRMServiceKeyTypeProtected;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.securityEngine = NEXUS_SecurityEngine_eCaCp;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.keySlot = NULL; 
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.service = eB_DVR_ServiceTSB;
						g_dvrTest->streamPath[pathIndex].drmServiceRequest.dma = NULL;
		                g_dvrTest->streamPath[pathIndex].drmService[44] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest);                
		                g_dvrTest->streamPath[pathIndex].drmService[45] = B_DVR_DRMService_Open(&g_dvrTest->streamPath[pathIndex].drmServiceRequest); 
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

						g_dvrTest->drmServiceSettings.drmAlgoType = NEXUS_SecurityAlgorithm_eAes128;
						g_dvrTest->drmServiceSettings.keyLength = KEY_LENGTH;
						g_dvrTest->drmServiceSettings.drmDest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
						g_dvrTest->drmServiceSettings.scPolarity = NEXUS_SecurityAlgorithmScPolarity_eOdd;
						g_dvrTest->drmServiceSettings.sessionKeys = changeToLocalSession;
						g_dvrTest->drmServiceSettings.keys = changeToLocalControlWord;
						g_dvrTest->drmServiceSettings.ivKeys = changeToIv;
						g_dvrTest->drmServiceSettings.operation = NEXUS_SecurityOperation_eEncrypt;				
		                dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[44] ,&g_dvrTest->drmServiceSettings);
						dvrError = B_DVR_DRMService_Configure(g_dvrTest->streamPath[pathIndex].drmService[45] ,&g_dvrTest->drmServiceSettings);

		                mediaNodeSettings.subDir = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir[0];
		                mediaNodeSettings.programName = (char *) &g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName[0];
		                mediaNodeSettings.volumeIndex = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex;
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

		                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[44],videoPidchHandle);

		                dvrError = B_DVR_DRMService_AddPidChannel(g_dvrTest->streamPath[pathIndex].drmService[45],audioPidchHandle);				
						
						dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
						if(dvrError!=B_DVR_SUCCESS)
						{
							printf("\n Error in starting the tsbService %d",pathIndex);
						}
						else
						{
							printf("tsbService started using CPS keyladder  %d\n",pathIndex);
						}

					}
					break;				
	
#endif			
    default:
        printf("\n Invalid DVR Operation - Select the operation again");
    }
}

