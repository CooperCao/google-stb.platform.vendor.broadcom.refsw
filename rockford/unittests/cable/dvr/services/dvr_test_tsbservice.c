/***************************************************************************
 *     Copyright (c) 2009-2012, Broadcom Corporation
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

BDBG_MODULE(dvr_test_tsbservice);

#define NUM_TSB_PATH 			8
#define MAX_CHANNELS     		3
#define MAX_AV_PATH             2
#define DURATION 				72000000
#define NUM_LOOPS				200
#define MAX_TSB_TIME            5*60*1000 /*5mins*/


unsigned int currentChannel = 2;
B_EventHandle TSBServiceReady;
B_EventHandle TSBConversionComplete;
B_EventHandle outOfMediaStorageEvent;
B_EventHandle endOfStreamEvent;
unsigned int numOfTSBServiceReady = 0;
unsigned int numOfTSBConversionComplete = 0;
unsigned long currentEndTime;


static void DvrTestCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};
	B_DVR_PlaybackServiceStatus playbackServiceStatus;

    printf("\n DvrTestCallback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    BSTD_UNUSED(appContext);
    switch(event)
    {
    case eB_DVR_EventStartOfRecording:
        {
            if(service == eB_DVR_ServiceTSB)
            {
				#if 1
				B_DVR_TSBServiceStatus tsbServiceStatus;

                B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                printf("\n####### Beginning Of TSB Conversion %d start time %lu endTime %lu ##########\n", index, tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
				#endif
                ++numOfTSBServiceReady;
				printf("\nNumber of TSB ready %d \n", numOfTSBServiceReady);
                if (numOfTSBServiceReady == NUM_TSB_PATH){
                    B_Event_Set(TSBServiceReady);
					printf("Set numOfTSBServiceReady = 0\n");
					numOfTSBServiceReady = 0;}
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
            ++numOfTSBConversionComplete;
             if (numOfTSBConversionComplete == NUM_TSB_PATH)
                    B_Event_Set(TSBConversionComplete);
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
        case eB_DVR_EventStartOfPlayback:
            {
                if(service == eB_DVR_ServicePlayback)
                {
                  printf("\n Beginnning of playback stream. Pausing");
                }
                else
                {
                    printf("\n Beginning of TSB. Pausing\n");
                 }
            }
            break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream. Pausing");
            printf("\n Stop either TSBPlayback or Playback to enter live mode");
			B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
			currentEndTime = playbackServiceStatus.currentTime;
			printf("\nCurrent Time: %lu", currentEndTime);
			B_Event_Set(endOfStreamEvent);
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
			B_Event_Set(outOfMediaStorageEvent);
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
            printf("\n invalid event!!!!!!!!!!\n");
        }
   }
    printf("\n DvrTestCallback !!!!!!!!!!!!\n");
    return;
}

void DvrTestTuneChannel(int index)
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

void DvrTestLiveDecodeStart(int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        /* NEXUS_PidChannelSettings settings; */

        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                    channelInfo[currentChannel].videoPids[0], NULL);
    }
    else
    {
    g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                   channelInfo[currentChannel].videoPids[0], NULL);
    }
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
            /* NEXUS_PidChannelSettings settings; */
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
        }
        else
        { 
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                        channelInfo[currentChannel].audioPids[0], NULL);
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


void DvrTestLiveDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
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

void DvrTestPlaybackDecodeStart(int index)
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
            mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
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
            assert(!rc);
        }
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
    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}


void DvrTestPlaybackDecodeStop(int index)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    BDBG_MSG(("\n %s: index %d >>>",__FUNCTION__,index));
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
    BDBG_MSG(("\n %s: index %d <<<",__FUNCTION__,index));
    return;
}


void DvrTestTsbServiceStartOperation(int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;

    BDBG_MSG(("\n TSBService starting on channel %u and path %d",currentChannel,pathIndex));
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
            sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
            "%s%d",TSB_SERVICE_PREFIX,pathIndex);
    BDBG_MSG(("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName));
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

    dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
    if(dvrError!=B_DVR_SUCCESS)
    {
        printf("\n Error in starting the TSBService %d",pathIndex);
    }
    else
    {
        printf("\nTSBService started %d\n",pathIndex);
    }

}

void DvrTestTsbServiceStopOperation(int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
            DvrTestPlaybackDecodeStop(pathIndex);
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

void DvrTestTsbBufferPreallocate(int numOfTsb)
{
    int index;
    
    for(index=0;index<numOfTsb;index++)
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_AllocSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings,
                                                    MAX_TSB_BUFFERS);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in allocating the tsb segments");
        }
    }

    for(index=0;index<numOfTsb;index++)
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
}

void DvrTestTsbBufferDeallocate(int numOfTsb)
{
    int index;

    for(index=0;index<numOfTsb;index++)
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",TSB_SERVICE_PREFIX,index);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }
}

void DvrTestChannelChange(int pathIndex)
{
	int channelUp = 1;

	if(g_dvrTest->streamPath[pathIndex].tsbService)
	{
		BDBG_MSG(("\n Invalid operation! Stop the timeshift srvc on this channel"));
		DvrTestTsbServiceStopOperation(pathIndex);
	}
	if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex <2)
	{
		DvrTestLiveDecodeStop(pathIndex);
	}
	if(channelUp)
	{
		printf("\nChannel Up");
		g_dvrTest->streamPath[pathIndex].currentChannel++;
		g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
		printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
	}
	else
	{
		printf("\nChannel Down\n");
		g_dvrTest->streamPath[pathIndex].currentChannel++;
		g_dvrTest->streamPath[pathIndex].currentChannel = (g_dvrTest->streamPath[pathIndex].currentChannel % g_dvrTest->maxChannels);
		printf("\n channel number : %d", g_dvrTest->streamPath[pathIndex].currentChannel);
	}
	DvrTestTuneChannel(pathIndex);
	if (pathIndex < MAX_AV_PATH)
	{
		printf("\n### Current channel %d title: %s in window %d #####", g_dvrTest->streamPath[pathIndex].currentChannel, channelInfo[pathIndex].programTitle, pathIndex);
		DvrTestLiveDecodeStart(pathIndex);
		g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
	}
}


void DvrTestTsbChannelChange (CuTest * tc)
{
	int loopCount = 0;
	int index;

	BSTD_UNUSED(tc);
	g_dvrTest->maxChannels = MAX_CHANNELS;
	TSBServiceReady = B_Event_Create(NULL);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTestCallback;
	DvrTestTsbBufferPreallocate(NUM_TSB_PATH);

	/* Run infinite loop for stress test purpose */
	while (loopCount <= NUM_LOOPS)
	{
		printf("\n\nNUMBER OF LOOPS %d \n\n", loopCount);
		loopCount++;
		for (index = 0; index < NUM_TSB_PATH; index++)
		{
			if(!g_dvrTest->streamPath[index].tsbService)
			{
				printf("\nTSB Service for the path index %d is not started\n", index);
				DvrTestTsbServiceStartOperation(index);
				printf("Starting TSB Service for the path index %d...\n", index);
			}
		}

		/* Wait until All TSB services are ready */
		while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
		BDBG_MSG(("\nAll %d TSB services are ready\n", index));

		/* Perform channel change */
		for (index = 0; index < NUM_TSB_PATH; index++)
		{
			printf("\nNumber of performing channel change %d \n", index);
			DvrTestChannelChange(index);
			sleep(5);
		}
	}
}

void DvrTest8TSBConvTilOutOfSpace(CuTest * tc)
{
    int index;
	unsigned recordingCount;
	DvrTest_Operation_Param param;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	unsigned long playbackEndTime;
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
	B_DVR_ERROR rc = B_DVR_SUCCESS;

	BSTD_UNUSED(tc);
	TSBServiceReady = B_Event_Create(NULL);
	TSBConversionComplete = B_Event_Create(NULL);
	outOfMediaStorageEvent = B_Event_Create(NULL);
	endOfStreamEvent = B_Event_Create(NULL);

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTestCallback;
    B_DVR_Manager_CreateMediaNodeList(g_dvrTest->dvrManager,0);

    DvrTestTsbBufferPreallocate(NUM_TSB_PATH);

    for (index = 0; index < NUM_TSB_PATH; index++)
    {
        if(!g_dvrTest->streamPath[index].tsbService)
        {
            printf("\nTSB Service for the path index %d is not started\n", index);
            DvrTestTsbServiceStartOperation(index);
            printf("Starting TSB Service for the path index %d...\n", index);
        }
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
	BDBG_MSG(("\n############All %d TSB services are ready#############\n", index));
    for (index = 0; index < NUM_TSB_PATH; index++)
    {
        BDBG_MSG(("\n### TSB index %d is ready ######\n", index));
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_ERROR rc = B_DVR_SUCCESS;
            B_DVR_TSBServiceStatus tsbStatus;
            B_DVR_TSBServicePermanentRecordingRequest recReq;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            sprintf(recReq.programName, "TSBRecord%d",index);
            recReq.recStartTime = tsbStatus.tsbRecStartTime;
            recReq.recEndTime = recReq.recStartTime + DURATION;

            printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
            sprintf(recReq.subDir,"tsbConv");
            rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
            if(rc!=B_DVR_SUCCESS)
            {
                printf("\n##### Can not start TSB Conversion %d ####\n", index);
            }
        }
		else
		{
			printf("\nTSB service is not started for path %d\n", index);
		}
    }

    /* wait until HD is full */
	BDBG_MSG(("\n####### Wait for outOfMediaStorageEvent ######\n"));
	while(B_Event_Wait(outOfMediaStorageEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    BDBG_MSG(("\n ### Media Storage is full. Stopping TSB Service Conversion ##########"));
    for (index = 0; index < NUM_TSB_PATH; index++)
    {
        B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[index].tsbService);
    }

/* Verifying just recorded streams */
	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "tsbConv");

	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

	if (recordingCount)
	{
		for(index=0; index<(int)recordingCount; index++)
		{
			strcpy(param.programName, recordingList[index]);
			BDBG_MSG(("\nProgram name to playback %s \n", param.programName));
			Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
			Do_DvrTest_Operation(tc, eDvrTest_OperationFastForward, &param);
			rc = B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
			if (rc != B_DVR_SUCCESS)
			{
				printf("\nCan't get playback service status");
			}
			BDBG_MSG(("\nPlayback end time %lu\n", playbackServiceStatus.endTime));
			playbackEndTime = playbackServiceStatus.endTime;
			B_Event_Wait(endOfStreamEvent, -1);
		    Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
			if ((playbackEndTime - currentEndTime)/1000 < 1)
			{
				printf("\nThe long recording have passed the integrity check\n");
			}
			else
				CuAssert(tc, "FAILED", playbackEndTime == currentEndTime);
		}
	}
	else 
		printf("No recordings found\n");

	if(endOfStreamEvent)
	{
		BDBG_MSG(("Destroy endOfStreamEvent\n"));
		BKNI_DestroyEvent(endOfStreamEvent);
	}
	BKNI_Free((void *)(char *)recordingList);
	DvrTestTsbBufferDeallocate(NUM_TSB_PATH);
}

