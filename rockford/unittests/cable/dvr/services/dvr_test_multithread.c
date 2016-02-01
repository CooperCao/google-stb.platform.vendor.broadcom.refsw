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
BDBG_MODULE(dvr_test_multithread);

static unsigned dataInjectionCompleteVar = 0;
static unsigned RecordServiceReadyVar = 0;
#define MAX_SIMUL_RECORDS 2
#define MAX_SIMUL_PLAYBACKS 2
#define MAX_TSB_INDEX 7
#define MIN_TSB_INDEX 2
static char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

#define MAX_AVPATH 8
#define DURATION 1200000    /* 20 minutes */

static unsigned int currentChannel = 2;
static B_EventHandle TSBServiceReady;
static B_EventHandle TSBConversionComplete;
static B_EventHandle TSBStartComplete;
static B_EventHandle endOfStreamEvent;
static unsigned int num_TSBServiceReady = 0;
static unsigned int num_TSBConversionComplete = 0;
static unsigned int TSB_BG_PB = 0;


/* Random File name generator */
static const char *letter[] = { "a","b","c","d","e","f","g","h","i","j","k","l","m","n",
	                         "o","p","q","r","s","t","u","v","w","x","y","z","1","2",
	                         "3","4","5","6","7","8","9","9", 0 };


typedef struct
{
	unsigned num_sec; 		/* In Seconds */
	unsigned num_millisec; 	/* In millisec for data injection */
	unsigned pathIndex; 	/* Channel for Background Recording */
	char programName[25]; 	/* Program Name for Background Recording */
	char subDir [B_DVR_MAX_FILE_NAME_LENGTH]; /* Sub Dir for Playback */ 
	unsigned num_es;		/* Num of ES streams */
	unsigned num_es_pid1;	/* Allow 2 ES */
	unsigned num_es_pid2;
	unsigned pmt_pid;		/* PMT PID */
	char patfile[25];		/* PAT File Name */
	char pmtfile[25];		/* PMT File Name */
	unsigned long seekTime;  /* Seek Time for Playback */
}DvrTest_Record_Param;


static void OperationMultiThreadService_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);
static void OperationLiveDecodeStart(CuTest * tc, int index);
static void OperationLiveDecodeStop(CuTest * tc, int index);
static void OperationPlaybackDecodeStart(CuTest * tc, int index);
static void OperationPlaybackDecodeStop(CuTest * tc, int index);
static void OperationPlaybackServiceSeek(CuTest * tc, DvrTest_Record_Param *playbackparam);
static void OperationPlaybackServicePlay(CuTest * tc, DvrTest_Record_Param *playbackparam);
static void OperationPlaybackServiceStart(CuTest * tc, DvrTest_Record_Param *recordparam);
static void OperationPlaybackServiceStop(CuTest *tc, DvrTest_Record_Param *recordparam);
static int OperationRecordServiceStart(CuTest * tc, DvrTest_Record_Param *recordparam, int pidremap);
static int OperationRecordServiceStop(CuTest *tc, DvrTest_Record_Param *recordparam);
static void OperationTSBServiceStart(CuTest *tc, int pathIndex);
static void OperationTSBServiceStop(CuTest * tc, int pathIndex);
static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath);
static void OperationTSBBufferDeallocate(CuTest * tc, int numAVPath);
static void GetRandomProgramName( DvrTest_Record_Param *recordparam);
static void OperationRecordServiceRecordingsList(CuTest *tc, int volIndex, char *subDir);
static void OperationPrintMediaAttributes(unsigned attributeVal);


static void OperationMultiThreadService_Callback(void *appContext, int index, B_DVR_Event event, B_DVR_Service service)
{
	char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
	char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};
	printf("\n OperationMultiThreadService_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
	BSTD_UNUSED(appContext);
	switch(event)
	{
	case eB_DVR_EventStartOfRecording:
		{
			if(service == eB_DVR_ServiceTSB)
			{
                if((g_dvrTest->streamPath[index].tsbService) && (index >= MIN_TSB_INDEX) && (index <= MAX_TSB_INDEX)){
					B_DVR_TSBServiceStatus tsbServiceStatus;
					B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService,&tsbServiceStatus);
					printf("\n Beginning Of TSB Conversion start time %lu endTime %lu",tsbServiceStatus.tsbRecStartTime,tsbServiceStatus.tsbRecEndTime);
					++num_TSBServiceReady;
					if (num_TSBServiceReady == (MAX_AVPATH - MIN_TSB_INDEX))
						B_Event_Set(TSBServiceReady);
				}
			}
			else
			{
				RecordServiceReadyVar = 1;
				printf("\n Beginning of background recording \n");
			}
		}
		break;
	case eB_DVR_EventEndOfRecording:
		{
			if(service == eB_DVR_ServiceTSB)
			{
				printf("\n End Of TSB Conversion \n");
			}
			else
			{
				printf("\n End of background recording \n");

			}
		}
		break;
	case eB_DVR_EventHDStreamRecording:
		{
			if(service == eB_DVR_ServiceTSB)
			{
				printf("\n HD Stream recording in TSB Service\n");
			}
			else
			{
				printf("\n HD Stream recording in Record Service\n");
			}
		}
		break;
	case eB_DVR_EventSDStreamRecording:
		{
			if(service == eB_DVR_ServiceTSB)
			{
				printf("\n SD Stream recording in TSB Service\n");
			}
			else
			{
				printf("\n SD Stream recording in Record Service\n");
			}
		}
		break;
	case eB_DVR_EventTSBConverstionCompleted:
		{
			if((g_dvrTest->streamPath[index].tsbService) && (index >= MIN_TSB_INDEX) && (index <= MAX_TSB_INDEX)){
				printf("\n TSB Conversion completed");
				++num_TSBConversionComplete;
				 if (num_TSBConversionComplete == (MAX_AVPATH - MIN_TSB_INDEX))
						B_Event_Set(TSBConversionComplete);
			}
		}
		break;
	case eB_DVR_EventOverFlow:
		{
			printf("\n OverFlow \n");
		}
		break;
	case eB_DVR_EventUnderFlow:
		{
			printf("\nunderFlow\n");
		}
		break;
	case eB_DVR_EventEndOfPlayback:
		{
			printf("\n End of playback stream. Pausing \n");
			printf("\n Stop either TSBPlayback or Playback to enter live mode \n");
			B_Event_Set(endOfStreamEvent);
		}
		break;
	case eB_DVR_EventStartOfPlayback:
		{
			if(service == eB_DVR_ServicePlayback)
			{
			  printf("\n Beginnning of playback stream. Pausing \n");
			}
			else
			{
				printf("\n Beginning of TSB. Pausing \n");
			}
		}
		break;
	case eB_DVR_EventPlaybackAlarm:
		{
			printf("\n playback alarm event \n");
		}
		break;
	case eB_DVR_EventAbortPlayback:
		{
			printf("\n abort playback \n");
		}
		break;
	case eB_DVR_EventAbortRecord:
		{
			printf("\n abort record \n");
		}
		break;
	case eB_DVR_EventAbortTSBRecord:
		{
			printf("\n abort TSB record \n");
		}
		break;
	case eB_DVR_EventAbortTSBPlayback:
		{
			printf("\n abort TSB Playback \n");
		}
		break;
	case eB_DVR_EventDataInjectionCompleted:
		{
			dataInjectionCompleteVar = 1;
			printf("\n data Injection complete \n");
		}
		break;
	case eB_DVR_EventOutOfMediaStorage:
		{
			printf("\n no media Storage space \n");
		}
		break;
	case eB_DVR_EventOutOfNavigationStorage:
		{
			printf("\n no navigation Storage space \n");
		}
		break;
	case eB_DVR_EventOutOfMetaDataStorage:
		{
			printf("\n no metaData Storage space \n");
		}
		break;
	default:
		{
			printf("\n invalid event");
		}
   }
	printf("\n OperationMultiThreadService_Callback <<<<< \n");
	return;
}


void GetRandomProgramName( DvrTest_Record_Param *recordparam )
{
	char filename[12]="";
	int rnd;
	int length=0;

	/*srand(time(NULL));*/
	srand(clock());
 
	for(;;){
		rnd = rand() % 36;
		strcat(filename,letter[rnd]);

		if(strlen(filename)==12){
			break;
		}
		length++;
	}
	strcpy(recordparam->programName, filename);
}


static void OperationPrintMediaAttributes(unsigned attributeVal)
{
    int attribute = 32;

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


static void OperationTSBServiceStart(CuTest * tc, int pathIndex)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	BSTD_UNUSED(tc);

    printf("TSB buffering begins for the index %d\n", pathIndex);

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


static void OperationTSBServiceStop(CuTest * tc, int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
	BSTD_UNUSED(tc);

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


static void OperationTSBBufferPreallocate(CuTest * tc, int numAVPath)
{
    int index;
	BSTD_UNUSED(tc);
    
    for(index=MIN_TSB_INDEX;index<numAVPath;index++)
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

    for(index=MIN_TSB_INDEX;index<numAVPath;index++)
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

    for(index=MIN_TSB_INDEX;index<numAVPath;index++)
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
}


static void OperationLiveDecodeStart(CuTest * tc, int index)
{
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    NEXUS_TimebaseSettings timebaseSettings;
    printf("\n %s:index %d >>>",__FUNCTION__,index);
    /* Open the pid channels */
    if(g_dvrTest->streamPath[index].tsbService)
    { 
        g_dvrTest->streamPath[index].videoPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                 channelInfo[currentChannel].videoPids[0],
																				 NULL);
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
            g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                                     channelInfo[currentChannel].audioPids[0],
                                                                                     NULL);
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


static int OperationRecordServiceStart(CuTest * tc, DvrTest_Record_Param *recordparam, int pidremap)
{
	if((recordparam->pathIndex > 0 && recordparam->pathIndex < 2 ))
	{
		printf("\n Invalid path chosen. Path should be between 2-5 for Background Recording");
		return B_DVR_NOT_SUPPORTED;
	}
	else
	{
		unsigned currentChannel = g_dvrTest->streamPath[recordparam->pathIndex].currentChannel;
        B_DVR_RecordServiceInputEsStream inputEsStream;
		B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
		B_DVR_RecordServiceSettings recordServiceSettings;
		BKNI_Memset((void *)&g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest));
		BDBG_MSG(("\n recording name: %s", recordparam->programName));
		strncpy(g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest.programName, recordparam->programName, B_DVR_MAX_FILE_NAME_LENGTH);

		g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest.volumeIndex =0;
		sprintf(g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest.subDir,"bgrec");
		g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest.recpumpIndex = recordparam->pathIndex;
		g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
		printf("\n recordService open >>>");
		g_dvrTest->streamPath[recordparam->pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[recordparam->pathIndex].recordServiceRequest);
		printf("\n recordService install callback >>");
		B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[recordparam->pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);

		BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
		recordServiceSettings.parserBand  = g_dvrTest->streamPath[recordparam->pathIndex].parserBand;
		dataInjectionOpenSettings.fifoSize = 30*188;
		g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
		CuAssertPtrNotNullMsg(tc, "B_DVR_DataInjectionService_Open failed", g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService);
		recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService;
		B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&recordServiceSettings);

		BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
		inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].videoPids[0];
		inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
		inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentChannel].videoFormat[0];
		inputEsStream.pidChannel = NULL;
        B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&inputEsStream);
		inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
		inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
		inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
		B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&inputEsStream);

		if(pidremap) {
			B_DVR_RecordService_GetSettings(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&recordServiceSettings);
			recordServiceSettings.esStreamCount = recordparam->num_es;
			recordServiceSettings.RemapppedEsStreamInfo[0].pid = recordparam->num_es_pid1;
			recordServiceSettings.RemapppedEsStreamInfo[1].pid = recordparam->num_es_pid2;
			B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&recordServiceSettings);
		}
		B_DVR_RecordService_Start(g_dvrTest->streamPath[recordparam->pathIndex].recordService, NULL);
		CuAssertPtrNotNullMsg(tc, "B_DVR_RecordService_Start failed", g_dvrTest->streamPath[recordparam->pathIndex].recordService);
		return B_DVR_SUCCESS;
	}
}


static int OperationRecordServiceStop(CuTest *tc, DvrTest_Record_Param *recordparam)
{
	BSTD_UNUSED(tc);
    if(recordparam->pathIndex > 0 && recordparam->pathIndex < 2 ){
        printf("\n invalid path chosen. Path should be between 2-5");
		return B_DVR_NOT_SUPPORTED;
	}
    else{
        B_DVR_RecordService_Stop(g_dvrTest->streamPath[recordparam->pathIndex].recordService);
		sleep(2);
		B_DVR_RecordService_Close(g_dvrTest->streamPath[recordparam->pathIndex].recordService);
		B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService);
		g_dvrTest->streamPath[recordparam->pathIndex].recordService = NULL;
		g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService = NULL;
		return B_DVR_SUCCESS;
	}
}


static void OperationPlaybackServiceSeek(CuTest * tc, DvrTest_Record_Param *playbackparam)
{
	B_DVR_OperationSettings operationSettings;
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
	BSTD_UNUSED(tc);
	B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService, &playbackServiceStatus);
	operationSettings.seekTime = playbackparam->seekTime;
	operationSettings.operation = eB_DVR_OperationSeek;
	B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService, &operationSettings);
}


static void OperationPlaybackServicePlay(CuTest * tc, DvrTest_Record_Param *playbackparam)
{
    B_DVR_OperationSettings operationSettings;
	BSTD_UNUSED(tc);
    operationSettings.operation = eB_DVR_OperationPlay;
    B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService, &operationSettings);
}


static void OperationPlaybackServiceStart(CuTest * tc, DvrTest_Record_Param *playbackparam)
{
	fflush(stdin);

	strcpy(g_dvrTest->streamPath[playbackparam->pathIndex].playbackServiceRequest.subDir, "bgrec");
	strcpy(g_dvrTest->streamPath[playbackparam->pathIndex].playbackServiceRequest.programName, playbackparam->programName);
	g_dvrTest->streamPath[playbackparam->pathIndex].playbackServiceRequest.playpumpIndex = playbackparam->pathIndex;
	g_dvrTest->streamPath[playbackparam->pathIndex].playbackServiceRequest.volumeIndex = 0;
	g_dvrTest->streamPath[playbackparam->pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[playbackparam->pathIndex].playbackServiceRequest);
	B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService, g_dvrTest->dvrTestCallback, (void *)g_dvrTest);
	printf("\n B_DVR_PlaybackService_Open");
	if(g_dvrTest->streamPath[playbackparam->pathIndex].liveDecodeStarted)
	{
		OperationLiveDecodeStop(tc, playbackparam->pathIndex);
	}
	printf("\n OperationLiveDecodeStop");
	OperationPlaybackDecodeStart(tc, playbackparam->pathIndex);
	printf("\n OperationPlaybackDecodeStart");
	B_DVR_PlaybackService_Start(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService);
	printf("\n B_DVR_PlaybackService_Start");
}


static void OperationPlaybackServiceStop(CuTest *tc, DvrTest_Record_Param *playbackparam)
{
	BSTD_UNUSED(tc);
	B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService);
	OperationPlaybackDecodeStop(tc, playbackparam->pathIndex);
	B_DVR_PlaybackService_Close(g_dvrTest->streamPath[playbackparam->pathIndex].playbackService);
	g_dvrTest->streamPath[playbackparam->pathIndex].playbackService = NULL;

	if(!g_dvrTest->streamPath[playbackparam->pathIndex].liveDecodeStarted)
	{
		OperationLiveDecodeStart(tc, playbackparam->pathIndex);
	}
}


void OperationRecordServiceRecordingsList(CuTest * tc, int volIndex, char *subDir)
{
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    unsigned recordingCount;
    unsigned index;
    unsigned esStreamIndex;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned i, *ptr;

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volIndex;
    
    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

    if(recordingCount)
    {
        printf("\n******************************************");
        printf("\nList of Recordings (%s)", subDir);
        printf("\n******************************************");
        recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
        CuAssertPtrNotNull(tc, recordingList);
        B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);

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
            OperationPrintMediaAttributes(media.mediaAttributes);
            printf("\n nav size: %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
            printf("\n media time (seconds): %u",(unsigned)((media.mediaEndTime - media.mediaStartTime)/1000));
            printf("\n [ PID Info ]");
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
            if (!(media.mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM)) 
            {
                printf("\n media stream is not encrypted\n");
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
                printf("\n drm key length = %u", media.keyLength);
                if (media.keyLength > 0) 
                {
                    ptr = &media.keyBlob[0][0];
                    printf("\n key Blob = ");
                    for (i = 0; i < media.keyLength; i++)
                    {
                        printf(" 0x%2x ", *(ptr + i));
                    }
                }
                
            }
            printf("\n------------------------------------------\n");

        }
        BKNI_Free(recordingList);
    }
    else
    {
        printf("\n No recordings found under %s directory!!!\n", subDir);
    }
}


void thread2_pb_start(void * ptr)
{
	unsigned recordingCount;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	DvrTest_Record_Param playbackparam[MAX_SIMUL_PLAYBACKS];
	B_DVR_MediaNodeSettings mediaNodeSettings;
	B_DVR_Media media;

	CuTest * tc;
	tc = (CuTest *) ptr;

	printf("\n $$$$$$$$$$$$$ Running Thread 4 - Dual Playback $$$$$$$$$$$$$$$ \n");

    memset(&recordparam, 0, (sizeof(recordparam)));
	memset(&playbackparam, 0, (sizeof(playbackparam)));
	memset(&mediaNodeSettings, 0, (sizeof(mediaNodeSettings)));
	strcpy(playbackparam[0].subDir, "bgrec");

	endOfStreamEvent = B_Event_Create(NULL);
	mediaNodeSettings.subDir = playbackparam[0].subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	if (recordingCount)
	{
		B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
		strcpy(playbackparam[0].programName, recordingList[0]);
		strcpy(playbackparam[1].programName, recordingList[1]);

		playbackparam[0].pathIndex = 0;
		printf("\nProgram name to playback %s \n", playbackparam[0].programName);
		OperationPlaybackServiceStart(tc, &playbackparam[0]);

		playbackparam[1].pathIndex = 1;
		printf("\nProgram name to playback %s \n", playbackparam[1].programName);
		OperationPlaybackServiceStart(tc, &playbackparam[1]);

		while(!TSB_BG_PB) {
			while(B_Event_Wait(endOfStreamEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
			printf("Set seektime to the beginning of stream after endOfStreamEvent\n");

			B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[playbackparam[0].pathIndex].playbackService, &media);
			playbackparam[0].seekTime = media.mediaStartTime;
			OperationPlaybackServiceSeek(tc, &playbackparam[0]);
			OperationPlaybackServicePlay(tc, &playbackparam[0]);
	
			B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[playbackparam[1].pathIndex].playbackService, &media);
			playbackparam[1].seekTime = media.mediaStartTime;
			OperationPlaybackServiceSeek(tc, &playbackparam[1]);
			OperationPlaybackServicePlay(tc, &playbackparam[1]);
		}
	}
	else{ 
			printf("No recordings found\n");
	}
	BKNI_Free((void *)(char *)recordingList);
}


void thread2_pb_stop(void * ptr)
{
	unsigned recordingCount;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	DvrTest_Record_Param playbackparam[MAX_SIMUL_PLAYBACKS];
	B_DVR_MediaNodeSettings mediaNodeSettings;
	CuTest * tc;
	tc = (CuTest *) ptr;

	memset(&recordparam, 0, (sizeof(recordparam)));
	memset(&playbackparam, 0, (sizeof(playbackparam)));
	memset(&mediaNodeSettings, 0, (sizeof(mediaNodeSettings)));
	strcpy(playbackparam[0].subDir, "bgrec");
	mediaNodeSettings.subDir = playbackparam[0].subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

	if (recordingCount){
		playbackparam[0].pathIndex = 0;
		OperationPlaybackServiceStop(tc, &playbackparam[0]);

		playbackparam[1].pathIndex = 1;
		OperationPlaybackServiceStop(tc, &playbackparam[1]);
	}
	else{
		printf("\n No Playbacks found to stop\n");
	}
}


void thread3_bg_start(void * ptr)
{
	B_DVR_ERROR rc;
	unsigned i;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	CuTest * tc;
	tc = (CuTest *) ptr;
    memset(&recordparam, 0, (sizeof(recordparam)));

	printf("\n $$$$$$$$$$$$$ Running Thread 3 - Background Record $$$$$$$$$$$$$$$ \n");

	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		GetRandomProgramName(&recordparam[i]);
		strcat(recordparam[i].programName, "r6");
		sleep(1);
	}
	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 6+i;
		printf("\n *** Recording #%d %s *** \n", i, (char *) recordparam[i].programName);
		rc = OperationRecordServiceStart(tc, &recordparam[i], 0);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}
	sleep(25);
}


void thread3_bg_stop(void * ptr)
{
	B_DVR_ERROR rc;
	unsigned i;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	CuTest * tc;
	tc = (CuTest *) ptr;
    memset(&recordparam, 0, sizeof(recordparam));

	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 6+i;
		rc = OperationRecordServiceStop(tc, &recordparam[i]);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}
}


void thread4_record_listing(CuTest * ptr)
{
	unsigned volumeIndex = 0, i = 0;
	CuTest * tc;
	tc = (CuTest *) ptr;

	printf("\n $$$$$$$$$$$$$ Running Thread 2 - Record List $$$$$$$$$$$$$$$ \n");

	while ((!TSB_BG_PB) /* &&  (i < 20) */){
		i++;
		OperationRecordServiceRecordingsList(tc, volumeIndex, "tsbConv");
		OperationRecordServiceRecordingsList(tc, volumeIndex, "bgrec");
		sleep(3);
	} /* Run until TSB is over */
}


void thread1_tsb_start(void * ptr)
{
    int index;
	CuTest * tc;
	tc = (CuTest *) ptr;

	printf("\n $$$$$$$$$$$$$ Running Thread 1 - 6 TSB Record $$$$$$$$$$$$$$$ \n");

	TSBServiceReady = B_Event_Create(NULL);
	TSBConversionComplete = B_Event_Create(NULL);

	OperationTSBBufferPreallocate(tc, MAX_AVPATH);

	for (index=MIN_TSB_INDEX; index < MAX_AVPATH; index++)
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
	for (index=MIN_TSB_INDEX; index < MAX_AVPATH; index++)
	{
		printf("\n### TSB index %d is ready\n", index);
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
				printf("\n Invalid paramters");
				TSB_BG_PB = 1;
			}
			CuAssertTrue(tc, rc==B_DVR_SUCCESS);
		}
	}
	
	B_Event_Set(TSBStartComplete);
	/* wait until TSB Conversion is completed */
	while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
	printf("\n Stopping TSB Service Conversion");
	for (index=MIN_TSB_INDEX; index < MAX_AVPATH; index++)
	{
		B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[index].tsbService);
	}


	printf("\n Closing all the services before stopping the test app");
	for(index=MIN_TSB_INDEX;index< MAX_AVPATH; index++)
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
	TSB_BG_PB = 1;
	OperationTSBBufferDeallocate(tc, MAX_AVPATH);
}


void DvrTestRecordServiceRecordWith6TSB2PBPlusContentListing(CuTest * tc)
{
	/* This is a multi Threaded Application running TSB on one thread, Playback on Second, and Content listing on 3rd */
    pthread_t thread1_tsb, thread2_pb, thread4_listing;
	int  ret1, ret2, ret4;
	TSBStartComplete = B_Event_Create(NULL);

	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationMultiThreadService_Callback;

	ret1 = pthread_create( &thread1_tsb, NULL, (void *(*)(void *)) thread1_tsb_start, (void *) tc);
	/* wait until TSB Start is completed */
	while(B_Event_Wait(TSBStartComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
	ret2 = pthread_create( &thread2_pb, NULL, (void *(*)(void *)) thread2_pb_start, (void*) tc);
	ret4 = pthread_create( &thread4_listing, NULL, (void *(*)(void *)) thread4_record_listing, (void*) tc); 

#if 0
    ret3 = pthread_create( &thread3_bg, NULL, (void *(*)(void *)) thread3_bg_start, (void*) tc);
#endif
	pthread_join( thread1_tsb, NULL); 
	pthread_join( thread4_listing, NULL);
    pthread_join( thread2_pb, NULL);
	thread2_pb_stop(tc);
#if 0
	pthread_join( thread3_bg, NULL); 
	if(TSB_BG_PB){
	thread3_bg_stop(tc);
		thread2_pb_stop(tc);
		TSB_BG_PB = 0;
	}
#endif
}
