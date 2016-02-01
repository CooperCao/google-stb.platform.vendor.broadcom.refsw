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
BDBG_MODULE(dvr_test_8TSB_Playback_RecordListing_Injection);

#define MAX_TSBPATH 8
static unsigned int TSB_BG_PB = 0;

#define DURATION0     60000        /* 1 minute */
#define DURATION1    150000        /* 2.5 minutes */
#define DURATION2    300000        /* 5 minutes */
#define DURATION3    600000      /* 10 minutes */
#define DURATION4   1200000     /* 20 minutes */
#define DURATION5   1800000     /* 30 minutes */

#define DURATION     DURATION3

typedef struct {
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned volumeIndex;
	void * ptr;
} RecordingListThreadParam;

typedef struct {
    int *cmds;
	void * ptr;
} TSBTrickPlayThreadParam;

typedef struct {
    unsigned injectionType;
    unsigned PAT_PID;
    unsigned PMT_PID;
	void * ptr;
} DataInjectThreadParam;


static int currentChannel = 2;

B_EventHandle TSBServiceReady;
B_EventHandle TSBConversionComplete;
B_EventHandle TSBRecordListComplete;
B_EventHandle DataInjectionComplete[MAX_TSBPATH];
B_EventHandle DataInjectionStop;
unsigned int num_TSBServiceReady = 0;
unsigned int num_TSBConversionComplete = 0;


char recordingList[MAX_TSBPATH][B_DVR_MAX_FILE_NAME_LENGTH];
int PlayCmds[3] = {0, 1, -1};     /* TSB Play Commands */

static void Operation8TSBWithTSBPBPlusContentListingPlusTSBInjection_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);
static void OperationLiveDecodeStart(CuTest * tc, int index);
static void OperationLiveDecodeStop(CuTest * tc, int index);
static void OperationPlaybackDecodeStart(CuTest * tc, int index);
static void OperationPlaybackDecodeStop(CuTest * tc, int index);
static void OperationTSBServiceStart(CuTest * tc, int pathIndex);
static void OperationTSBServiceStop(CuTest * tc, int pathIndex);
static void OperationTSBServicePlayStart(CuTest * tc, int pathIndex);
static void OperationTSBServicePlayStop(CuTest * tc, int pathIndex);
static B_DVR_ERROR OperationTSBDataInjectionStart(CuTest * tc, unsigned injectionType, unsigned pid, int pathIndex);
static void OperationTSBDataInjectionStop(CuTest * tc, int pathIndex);
static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath);
static void OperationTSBBufferDeallocate(CuTest * tc, int numAVPath);
static B_DVR_ERROR OperationCopyMediaFromMetaData(CuTest * tc, char *fileName);
static void OperationPrintMediaAttributes(CuTest * tc, unsigned attributeVal);
static void OperationDVRRecordingsInfo(void *context);
static void OperationTSBPlay(void *context);
static void OperationDataInject(void *context);


static void Operation8TSBWithTSBPBPlusContentListingPlusTSBInjection_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};

    printf("\n Operation8TSBWithTSBPBPlusContentListingPlusTSBInjection_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
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
                ++num_TSBServiceReady;
                if (num_TSBServiceReady == MAX_TSBPATH)
                {
                    B_Event_Set(TSBServiceReady);
                }
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
            printf("\n TSB Conversion completed for the index path %d", index);
            ++num_TSBConversionComplete;
            printf("\n Total number of TSB Conversion completed is %d\n", num_TSBConversionComplete);
            if (num_TSBConversionComplete == MAX_TSBPATH)
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
            printf("\n Data Injection is complete for the index path %d", index);
            B_Event_Set(DataInjectionComplete[index]);
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
    printf("\n Operation8TSBWithTSBPBPlusContentListingPlusTSBInjection_Callback <<<<<");
    return;
}


static void OperationLiveDecodeStart(CuTest * tc, int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
	BSTD_UNUSED(tc);
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


static void OperationLiveDecodeStop(CuTest * tc, int index)
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


static void OperationPlaybackDecodeStart(CuTest * tc, int index)
{
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_Media media;
    B_DVR_ERROR rc;
	BSTD_UNUSED(tc);
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
		CuAssertTrue(tc, 0);
    }
    printf("\n %s:index %d <<<",__FUNCTION__,index);

}


static void OperationPlaybackDecodeStop(CuTest * tc, int index)
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


static void OperationTSBServiceStart(CuTest * tc, int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;


    printf("TSB buffering begins\n");
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        if ((pathIndex == 0) || (pathIndex == 1))
            OperationLiveDecodeStart(tc, pathIndex);
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
        printf("\n Error in starting the tsbService %d",pathIndex);
		TSB_BG_PB = 1;
    }
    else
    {
        printf("tsbService started %d\n",pathIndex);
    }
	CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
}


static void OperationTSBServiceStop(CuTest *tc, int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
            OperationPlaybackDecodeStop(tc, pathIndex);
        }
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in stopping the timeshift service %d\n",pathIndex);
        }
        /* temporal workaround */
        BKNI_Sleep(1000);
        /* temporal workaround */
		CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
        B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
		sleep(2);
        dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].tsbService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in closing the timeshift service %d\n",pathIndex);
        }
		CuAssertTrue(tc, dvrError==B_DVR_SUCCESS);
    }
    else
    {
        printf("\n timeshift srvc not started");
    }

}


static void OperationTSBServicePlayStart(CuTest * tc, int pathIndex)
{
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus tsbServiceStatus;

    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        OperationLiveDecodeStop(tc, pathIndex);
    }
    OperationPlaybackDecodeStart(tc, pathIndex);
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
    operationSettings.operation = eB_DVR_OperationPause; 
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
}


static void OperationTSBServicePlayStop(CuTest * tc, int pathIndex)
{
    B_DVR_OperationSettings operationSettings;

    if(!g_dvrTest->streamPath[pathIndex].tsbService)
    {
        printf("\n error TSBPlayStart without TSBServiceStart");
    }
    else
    {
        operationSettings.operation = eB_DVR_OperationTSBPlayStop;
        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
        OperationPlaybackDecodeStop(tc, pathIndex);
        if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
        {
            OperationLiveDecodeStart(tc, pathIndex);
        }
    }
}


static B_DVR_ERROR OperationTSBDataInjectionStart(CuTest * tc, unsigned injectionType, unsigned pid, int pathIndex)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings settings;
    char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    uint8_t pkt[188*30];
    int dataFileID;
    int readSize;

	BSTD_UNUSED(tc);
    if (pid == 0x0)
        sprintf(dataFileName, "/mnt/nfs/PAT.TS");
    else
        sprintf(dataFileName, "/mnt/nfs/PMT.TS");
    
    if ((dataFileID = open(dataFileName, O_RDONLY,0666)) < 0)
    {
        printf("\n unable to open %s",dataFileName);
    }
    
    BKNI_Memset((void *)&pkt[0],0,188*30);
    
    readSize = (int) read(dataFileID,&pkt[0],188*30);
    
    if(readSize < 0)
    {
        printf("\n Data file is empty or not available");
    }
    
    B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,
        g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

    if ((injectionType != eB_DVR_DataInjectionServiceTypePSI) && 
         (injectionType!= eB_DVR_DataInjectionServiceTypeRawData))
         return B_DVR_INVALID_PARAMETER;

    settings.dataInjectionServiceType = injectionType;
    settings.pid = pid;
    B_DVR_DataInjectionService_SetSettings(g_dvrTest->streamPath[pathIndex].dataInjectionService,&settings);
    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,&pkt[0],readSize);
    }
    else
    {
        printf("TSB Service is not available\n");
    }
    return (rc);
}


static void OperationTSBDataInjectionStop(CuTest * tc, int pathIndex)
{
	BSTD_UNUSED(tc);
    if(g_dvrTest->streamPath[pathIndex].tsbService){
        B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
	}
    else{
        printf("TSB Service is not available\n");
	}
}


static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath)
{
    int index;
	BSTD_UNUSED(tc);
    
    for(index=0;index<numAVPath;index++)
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

    for(index=0;index<numAVPath;index++)
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


static void OperationTSBBufferDeallocate(CuTest * tc, int numAVPath)
{
    int index;
	BSTD_UNUSED(tc);
    for(index=0;index<numAVPath;index++)
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


static B_DVR_ERROR OperationCopyMediaFromMetaData(CuTest * tc, char *fileName)
{
    B_DVR_SegmentedFileNode fileNode;
    off_t returnOffset, fileNodeOffset;
    ssize_t sizeRead=0;
    unsigned index=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_File dvrFile;
    B_DVR_FileSettings settings;
    char temp[512];
	BSTD_UNUSED(tc);

    settings.fileIO = eB_DVR_FileIORead;
    settings.directIO = false;
    B_DVR_File_Open(&dvrFile,fileName, &settings);

    printf("dumpMetaDataFile %s>>>\n",fileName);
    do
    {
        fileNodeOffset = index*sizeof(fileNode);
        returnOffset = dvrFile.fileInterface.io.readFD.seek(
            &dvrFile.fileInterface.io.readFD,
            fileNodeOffset,SEEK_SET);

        if(returnOffset != fileNodeOffset)
        {
            printf("End of metadata file\n");
            break;
        }
         sizeRead = dvrFile.fileInterface.io.readFD.read(
             &dvrFile.fileInterface.io.readFD,&fileNode,sizeof(fileNode));

        if(sizeRead != sizeof(fileNode))
        {
            printf("end of metaDataFile\n");
        }
        else
        {
            printf("file node-%u %s %lu %lu %d\n",
                      index,
                      fileNode.fileName,
                      (unsigned long)fileNode.linearStartOffset,
                      (unsigned long)fileNode.size,
                      fileNode.recordType);

            sprintf(temp, "cp /mnt/dvr0-media/record/%s /mnt/nfs/", fileNode.fileName);
            printf("command: %s\n", temp);
            system(temp);
        }
        index++;
    }while(sizeRead);

    B_DVR_File_Close(&dvrFile);
    printf("dumpMetaDataFile <<<\n");
    return rc;
}


static void OperationPrintMediaAttributes(CuTest * tc, unsigned attributeVal)
{
    int attribute = 32;
	BSTD_UNUSED(tc);

    do {
        if (attribute & attributeVal)
        {
            switch (attribute)
            {
                case B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM:
                    printf("Segmented_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM:
                    printf("Encrypted_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HD_STREAM:
                    printf("HD_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_AUDIO_ONLY_STREAM:
                    printf("Audio_Only_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_HITS_STREAM:
                    printf("HITS_Stream ");
                break;
                case B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS:
                    printf("Recording_In_Progress ");
                break;
                default:
                    printf("Unknown_Attribute_Value ");
                break;
            }
        }
        attribute = attribute >> 1;
    } while (attribute > 0);
}


static void OperationDVRRecordingsInfo(void *context)
{
	CuTest * tc;
    int volIndex = 0;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned recordingCount;
    unsigned index;
    unsigned mediaTime;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned keyLength;
    unsigned count=0;
    unsigned char key[16];
    RecordingListThreadParam *pParam;
    bool quit = false;

    pParam = (RecordingListThreadParam*)context;
	tc = (CuTest *)pParam->ptr;
	
    strcpy(subDir, pParam->subDir);
    volIndex= pParam->volumeIndex;

    BKNI_Free(context);

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;
    
    recordingCount = MAX_TSBPATH;

    while (!TSB_BG_PB){
        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

        printf("\n******************************************");
        printf("\nList of Recordings (%s)", subDir);
        printf("\n******************************************");

        for(index=0;index<recordingCount;index++)
        {
            printf("\n------------------------------------------");
            printf("\n program index: %d", index);
            printf("\n program name: %s", recordingList[index]);
            mediaNodeSettings.programName = recordingList[index];
            B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
            printf("\n program metaData file: %s",media.mediaNodeFileName);
            printf("\n media metaData file: %s",media.mediaFileName);
            printf("\n nav metaData file: %s",media.navFileName);
            printf("\n media size: %u",(unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset));
            printf("\n media attributes: ");
            OperationPrintMediaAttributes(tc, media.mediaAttributes);
            printf("\n nav size: %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
            mediaTime = (unsigned)((media.mediaEndTime - media.mediaStartTime)/1000);
            printf("\n media time (seconds): %u", mediaTime);
            if (mediaTime >= DURATION/1000)
            {
                quit = true;
            }
            if (!(media.mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM)) 
            {
                printf("\n media stream is not encrypted");
            }
            else 
            {
                printf("\n media stream is encrypted");
                if (media.drmServiceType == eB_DVR_DRMServiceTypeBroadcastMedia)
                {
                    printf("\n drm service type: Broadcast Media");
                }
                else if (media.drmServiceType == eB_DVR_DRMServiceTypeContainerMedia)
                {
                    printf("\n drm service type: Container Media");
                }
                if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeProtected)
                {
                    printf("\n drm service key type: Protected");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeClear)
                {
                    printf("\n drm service key type: Clear");
                }
                else if (media.drmServiceKeyType == eB_DVR_DRMServiceKeyTypeMax)
                {
                    printf("\n drm service key type: Not Available");
                }

                B_DVR_Manager_GetKeyBlobLengthPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,&keyLength);
                B_DVR_Manager_GetKeyBlobPerEsStream(g_dvrTest->dvrManager,&mediaNodeSettings,media.esStreamInfo[0].pid,key);

                printf("\n key length = %d\n", keyLength);
                printf(" key = ");
                for(count=0;count<keyLength;count++)
                {
                    printf("0x%x ", key[count]);
                }
            }
            printf("\n------------------------------------------\n");
        }
        BKNI_Sleep(3000);
    }
}


static void OperationTSBPlay(void *context)
{
	CuTest * tc;
    B_DVR_TSBServiceStatus tsbServiceStatus;
    B_DVR_OperationSettings operationSettings;
    TSBTrickPlayThreadParam *pParam;
    int *ptr, index;


    pParam = (TSBTrickPlayThreadParam*)context;
	tc = pParam->ptr;
    ptr= pParam->cmds;

    BKNI_Free(context);

   while(!TSB_BG_PB) { 
       BKNI_Sleep(5000);

        if (*ptr >= 0)
        {
            switch(*ptr)
            {
                case 0:
                    printf("\n****************************\n");
                    printf("Command: TSB playback pause");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++) 
                    {
                        if(g_dvrTest->streamPath[index].tsbService) 
                        {
                            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
                            if (!tsbServiceStatus.tsbPlayback)
                            {
                                printf("\nTSB Playback is not started for the index %d", index);
                                OperationTSBServicePlayStart(tc, index);
                                printf("\nTSB Playback is starting for the index %d...", index);
                            }
                            else
                            {
                                if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                                {
                                    printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                                }
                                else
                                {
                                    B_DVR_OperationSettings operationSettings;
                                    operationSettings.operation = eB_DVR_OperationPause;
                                    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                                }
                            }
                        }
                        else 
                        {
                            printf("\nTSB Service is not enabled for the index %d", index);
                        }
                    }
                break;
                case 1:
                    printf("\n****************************\n");
                    printf("Command: TSB playback play");
                    printf("\n****************************\n");
                    for (index=0; index < 2; index++)
                    {
                        if(g_dvrTest->streamPath[index].tsbService) {
                            if(!g_dvrTest->streamPath[index].playbackDecodeStarted)
                            {
                                printf("\nTSB playback for the index %d is not started. Start TSB playback", index);
                            }
                            else
                            {
                                operationSettings.operation = eB_DVR_OperationPlay;
                                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService,&operationSettings);
                            }
                        }
                        else 
                        {
                            printf("\nTSB Service for the index %d is not enabled", index);
                        }
                    }
                break;
                default:
                    printf("Unknown Command\n");
                break;
            }
            ++ptr;
        }
        else
        {
            ptr = ptr-1;
        }
    };
}


static void OperationDataInject(void *context)
{
	CuTest * tc;
    DataInjectThreadParam *pParam;
    int index;
    unsigned injectionType;
    unsigned PAT_PID;
    unsigned PMT_PID;

    pParam = (DataInjectThreadParam*)context;

    injectionType = pParam->injectionType;
    PAT_PID = pParam->PAT_PID;
    PMT_PID = pParam->PMT_PID;
	tc = pParam->ptr;

    BKNI_Free(context);

   do {
       for (index = 0; index < MAX_TSBPATH; index++){
           /* Data Injection of PAT */
           OperationTSBDataInjectionStart(tc, injectionType, PAT_PID, index);
           while(B_Event_Wait(DataInjectionComplete[index], B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
           printf("\n$$$ Data Injection is complete for the index %d $$$\n", index);
           B_Event_Reset(DataInjectionComplete[index]);
           OperationTSBDataInjectionStop(tc, index);
           /* wait for 100 milisec */
           BKNI_Sleep(100);

           /* Data Injection of PMT */
           OperationTSBDataInjectionStart(tc, injectionType, PMT_PID, index);
           while(B_Event_Wait(DataInjectionComplete[index], B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
           printf("\n$$$ Data Injection is complete for the index %d $$$\n", index);
           B_Event_Reset(DataInjectionComplete[index]);
           OperationTSBDataInjectionStop(tc, index);
           /* wait for 100 milisec */
           BKNI_Sleep(100);
	   }
   }while(B_Event_Wait(DataInjectionStop, B_WAIT_NONE)!=B_ERROR_SUCCESS);
}


void DvrTestRecordServiceRecordWith8TSBWithTSBPBPlusContentListingPlusTSBInjection(CuTest * tc)
{
    unsigned volumeIndex = 0;
    int index; 
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    RecordingListThreadParam *pRecordingListParam;
    TSBTrickPlayThreadParam *pTSBTrickPlayParam;
    DataInjectThreadParam *pDataInjectionParam;
	int  ret1, ret2, ret3;

	pthread_t TSBTrickPlayThread, RecordingListThread, DataInjectionThread;

    TSBServiceReady = B_Event_Create(NULL);
    TSBConversionComplete= B_Event_Create(NULL);
    TSBRecordListComplete = B_Event_Create(NULL);
    DataInjectionStop = B_Event_Create(NULL);
    
    for (index=0; index<MAX_TSBPATH; index++)
    {
        DataInjectionComplete[index] = B_Event_Create(NULL);
    }

    printf("max stream path = %d\n", MAX_STREAM_PATH);
    printf("configured stream path = %d\n", MAX_TSBPATH);


    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)Operation8TSBWithTSBPBPlusContentListingPlusTSBInjection_Callback;
    OperationTSBBufferPreallocate(tc, MAX_TSBPATH);

    for (index = 0; index < MAX_TSBPATH; index++)
    {
        if(!g_dvrTest->streamPath[index].tsbService)
        {
            printf("\nTSB Service for the path index %d is not started\n", index);
            OperationTSBServiceStart(tc, index);
            printf("Starting TSB Service for the path index %d...\n", index);
        }
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

#if 0
    /* starting Data Injection */
    pDataInjectionParam = BKNI_Malloc(sizeof(DataInjectThreadParam));
    if (!pDataInjectionParam)
    {
        printf("Error in creating a Data InjectionThreadParam\n");
    }
    pDataInjectionParam->injectionType = eB_DVR_DataInjectionServiceTypeRawData;
    pDataInjectionParam->PAT_PID = 0x0;
    pDataInjectionParam->PMT_PID = 0x3f;
	pDataInjectionParam->ptr = tc;


	ret1 = pthread_create( &DataInjectionThread, NULL, (void *(*)(void *)) OperationDataInject, (void*) pDataInjectionParam);
#endif
    /* starting TSB conversion */
    for (index = 0; index < MAX_TSBPATH; index++)
    {
        printf("\n### TSB index %d is ready\n", index);
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_TSBServiceStatus tsbStatus;
            B_DVR_TSBServicePermanentRecordingRequest recReq;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            sprintf(recReq.programName, "TSBRecord%d", index);
            recReq.recStartTime = tsbStatus.tsbRecStartTime;
            recReq.recEndTime = recReq.recStartTime + DURATION;

            printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
            sprintf(recReq.subDir,"tsbConv");
            rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
            if(rc!=B_DVR_SUCCESS)
            {
                printf("\n TSB Conversion of AV path index % is failed", index);
				TSB_BG_PB = 1;
            }
			CuAssertTrue(tc, rc==B_DVR_SUCCESS);
        }
    }

#if 1
    /* starting TSB Trick Play */
    pTSBTrickPlayParam = BKNI_Malloc(sizeof(TSBTrickPlayThreadParam));
    if (!pTSBTrickPlayParam)
    {
        printf("Error in creating a TSBTrickPlayThreadParam\n");
    }

    pTSBTrickPlayParam->cmds = &PlayCmds[0];
	pTSBTrickPlayParam->ptr = tc;

	ret2 = pthread_create( &TSBTrickPlayThread, NULL, (void *(*)(void *)) OperationTSBPlay, (void*) pTSBTrickPlayParam);
#endif
    /* start displaying of a list of TSB recordings */
    pRecordingListParam = BKNI_Malloc(sizeof(RecordingListThreadParam));
    if (!pRecordingListParam)
    {
        printf("Error in creating a RecordingListThreadParam\n");
    }
    
    sprintf(pRecordingListParam->subDir, "tsbConv");
    pRecordingListParam->volumeIndex = volumeIndex;
	pRecordingListParam->ptr = tc;
    
	ret3 = pthread_create( &RecordingListThread, NULL, (void *(*)(void *)) OperationDVRRecordingsInfo, (void*) pRecordingListParam);

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

    /* stopping TSB conversion */
    printf("\n Stopping TSB Service Conversion");
    for (index = 0; index < MAX_TSBPATH; index++)
    {
        B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[index].tsbService);
    }

    /* stopping TSB data injection */
    B_Event_Set(DataInjectionStop);
	TSB_BG_PB = 1;
	pthread_join(RecordingListThread, NULL);
    pthread_join(TSBTrickPlayThread, NULL); 
#if 0
    pthread_join(DataInjectionThread, NULL);
#endif

    /* Stopping TSB playback service for the index 0 and 1 */
    for (index=0; index<2; index++)
    {
        B_DVR_TSBServiceStatus tsbStatus;
    
        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbStatus);
        if (tsbStatus.tsbPlayback)
        {
            OperationTSBServicePlayStop(tc, index);
        }
    }

    printf("\n Stopping all TSB Services");
    for(index=0;index< MAX_TSBPATH; index++)
    {
        if(g_dvrTest->streamPath[index].tsbService)
        {
            B_DVR_TSBServiceStatus tsbStatus;

            B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
            /* tsbConversion is false */
            if(!tsbStatus.tsbCoversion)
            {
                printf("\n Stopping TSB Conversion Service index %d", index);
                OperationTSBServiceStop(tc, index);
            }
        }
    }
#if 0
    /* copying media segments to the nfs mounted directory */
    for (index=0; index<MAX_TSBPATH; index++)
    {
        char temp[B_DVR_MAX_FILE_NAME_LENGTH];

        sprintf(temp, "/mnt/dvr0-metadata/tsbConv/TSBRecord%d.ts", index);
        OperationCopyMediaFromMetaData(tc, temp);
    }
#endif

    OperationTSBBufferDeallocate(tc, MAX_TSBPATH);
}

