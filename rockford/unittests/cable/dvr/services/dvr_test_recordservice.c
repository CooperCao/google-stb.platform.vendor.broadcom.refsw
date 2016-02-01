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
BDBG_MODULE(dvr_test_recordservice);

static unsigned dataInjectionCompleteVar = 0;
static unsigned RecordServiceReadyVar = 0;
#define MAX_SIMUL_RECORDS 4
#define MAX_SIMUL_PLAYBACKS 2
static char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

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
}DvrTest_Record_Param;

#define INJECTPAT 1
#define INJECTPMT 2

/* Random File name generator */
const char *letter[] = { "a","b","c","d","e","f","g","h","i","j","k","l","m","n",
	                         "o","p","q","r","s","t","u","v","w","x","y","z","1","2",
	                         "3","4","5","6","7","8","9","9", 0 };

static int OperationRecordServiceScanParam(DvrTest_Record_Param *recordparam);
static int scanline(FILE *fp, char * line);
static void OperationRecordService_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service);
static void OperationLiveDecodeStart(CuTest * tc, int index);
static void OperationLiveDecodeStop(CuTest * tc, int index);
static void OperationPlaybackDecodeStart(CuTest * tc, int index);
static void OperationPlaybackDecodeStop(CuTest * tc, int index);
static void OperationPlaybackServiceStart(CuTest * tc, DvrTest_Record_Param *recordparam);
static void OperationPlaybackServiceStop(CuTest *tc, DvrTest_Record_Param *recordparam);
static int OperationRecordServiceWithDataInjection(CuTest * tc, DvrTest_Record_Param *recordparam, int patorpmt);
static int OperationRecordServiceStart(CuTest * tc, DvrTest_Record_Param *recordparam, int pidremap);
static int OperationRecordServiceStop(CuTest *tc, DvrTest_Record_Param *recordparam);
static void GetRandomProgramName( DvrTest_Record_Param *recordparam);

static int scanline(FILE *fp, char * line)
{
    char * ptr1;

    while (fgets(line, 1024, fp)) {
		if ( (ptr1 = strchr(line, '#')) != NULL) {
			continue;
		} else {
			int i = 0;
			int len = strlen(line);
			while (i < len) {
				if ( !(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'))
					return 0;
				i++;
			}
			continue;
		}
    }
    return -1;
}

static void OperationRecordService_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
	char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow","underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec","abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace","noMetaDataDiskSpace","invalidEvent"};
	char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService","storagaService","dataInjectionService","invalidService"};
	printf("\n OperationRecordService_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
	BSTD_UNUSED(appContext);
	switch(event)
	{
	case eB_DVR_EventStartOfRecording:
		{
			if(service == eB_DVR_ServiceTSB)
			{
				printf("\n Beginning Of TSB Conversion \n");
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
			printf("\n TSB Conversion completed \n");
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
	printf("\n OperationRecordService_Callback <<<<< \n");
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


static void OperationPlaybackDecodeStart(CuTest * tc, int index)
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


static int OperationRecordServiceWithDataInjection(CuTest * tc, DvrTest_Record_Param *recordparam, int patorpmt)
{
	B_DVR_ERROR rc;
    B_DVR_DataInjectionServiceSettings settings;
    char dataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    uint8_t pkt[188*30];
    unsigned pid = 0x0;
    int dataFileID;
    int readSize;

	/* wait until Record Service becomes ready */
	do{
        if (RecordServiceReadyVar)
            break;
        }while (1);

	if(patorpmt == INJECTPAT){
		if ((dataFileID = open(recordparam->patfile, O_RDONLY,0666)) < 0){
			printf("\n unable to open %s",dataFileName);
			return B_DVR_NOT_SUPPORTED;
		}
	}
	else {
		if ((dataFileID = open(recordparam->pmtfile, O_RDONLY,0666)) < 0){
			printf("\n unable to open %s",dataFileName);
			return B_DVR_NOT_SUPPORTED;
		}
	}
    BKNI_Memset((void *)&pkt[0],0,188*30);
    readSize = (int) read(dataFileID,&pkt[0],188);
	if(readSize < 0){
		printf("\n Data file is empty or not available");
		return B_DVR_NOT_SUPPORTED;
	}
    B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
    settings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    settings.pid = pid;
    B_DVR_DataInjectionService_SetSettings( g_dvrTest->streamPath[recordparam->pathIndex].dataInjectionService, &settings);
    if(g_dvrTest->streamPath[recordparam->pathIndex].recordService){
		rc = B_DVR_RecordService_InjectDataStart(g_dvrTest->streamPath[recordparam->pathIndex].recordService,&pkt[0],readSize);
		CuAssert(tc, "B_DVR_RecordService_InjectDataStart Failed", rc==B_DVR_SUCCESS );
	}
    else{
		printf("\n neither record or tsb is active on path %d",recordparam->pathIndex);
		return B_DVR_NOT_SUPPORTED;
	}
	if(g_dvrTest->streamPath[recordparam->pathIndex].recordService){
		do {
			if (dataInjectionCompleteVar){
				printf("\n$$$ Data Injection is Complete $$$\n");
				break;
			}
		} while (1);
		dataInjectionCompleteVar = 0;
		rc = B_DVR_RecordService_InjectDataStop(g_dvrTest->streamPath[recordparam->pathIndex].recordService);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
		return B_DVR_SUCCESS;
	}
	else{
		return B_DVR_NOT_SUPPORTED;
	}
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

static int OperationRecordServiceScanParam(DvrTest_Record_Param *recordparam)
{
	FILE *fr;
    char line[80];
	fr = fopen ("dvrtestrecordservice.txt", "rt");  /* open the file for reading */
	if (fr == NULL) {
		return -1;
	}
	while(!feof(fr)) {
		if (scanline(fr, line) == 0)
			sscanf(line,"num_sec %u\n", &recordparam->num_sec);
			sscanf(line,"num_millisec %u\n", &recordparam->num_millisec);
			sscanf(line,"pathIndex %u\n", &recordparam->pathIndex);
			sscanf(line,"num_es %u\n", &recordparam->num_es);
			sscanf(line,"num_es_pid1 0x%x\n", &recordparam->num_es_pid1);
			sscanf(line,"num_es_pid2 0x%x\n", &recordparam->num_es_pid2);
			sscanf(line,"pmt_pid 0x%x\n", &recordparam->pmt_pid);
			sscanf(line,"patfile %s\n", (char *)&recordparam->patfile);
			sscanf(line,"pmtfile %s\n", (char *)&recordparam->pmtfile);
		}
	fclose(fr);  /* close the file prior to exiting the routine */
	return 0;
}

void DvrTestRecordServiceRecordStartSimulStandalone(CuTest * tc)
{
	B_DVR_ERROR rc;
	unsigned i;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
    memset(&recordparam, 0, (sizeof(recordparam)));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;


	strcpy((char*)&recordparam[0].programName, "dvrtest_hn");
	for(i=1; i<MAX_SIMUL_RECORDS; i++) {
		GetRandomProgramName(&recordparam[i]);
		strcat(recordparam[i].programName, "r1");
		sleep(1);
	}
	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 2+i;
		printf("\n *** Recording #%d %s *** \n", i, (char *) recordparam[i].programName);
		rc = OperationRecordServiceStart(tc, &recordparam[i], 0);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}
	sleep(180);
}

void DvrTestRecordServiceRecordStopSimulStandalone(CuTest * tc)
{
	B_DVR_ERROR rc;
	unsigned i;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
    memset(&recordparam, 0, sizeof(recordparam));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;

	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 2+i;
		rc = OperationRecordServiceStop(tc, &recordparam[i]);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}
}


void DvrTestRecordServiceRecordStartSimulStandaloneWithDualPlayback(CuTest * tc)
{
	B_DVR_ERROR rc;
	unsigned i;
	unsigned recordingCount;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	DvrTest_Record_Param playbackparam[MAX_SIMUL_PLAYBACKS];
	B_DVR_MediaNodeSettings mediaNodeSettings;
    memset(&recordparam, 0, (sizeof(recordparam)));
	memset(&playbackparam, 0, (sizeof(playbackparam)));
	memset(&mediaNodeSettings, 0, (sizeof(mediaNodeSettings)));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;
	strcpy(playbackparam[0].subDir, "bgrec");
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
	}
	else{ 
			printf("No recordings found\n");
	}
	BKNI_Free((void *)(char *)recordingList);
	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
        GetRandomProgramName(&recordparam[i]);
		strcat(recordparam[i].programName, "r2");
		sleep(1);
	}
	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 2+i;
		printf("\n *** Recording #%d %s *** \n", i, (char *) recordparam[i].programName);
		rc = OperationRecordServiceStart(tc, &recordparam[i], 0);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}
	sleep(60);
}


void DvrTestRecordServiceRecordStopSimulStandaloneWithDualPlayback(CuTest * tc)
{
	B_DVR_ERROR rc;
	unsigned i;
	unsigned recordingCount;
	DvrTest_Record_Param recordparam[MAX_SIMUL_RECORDS];
	DvrTest_Record_Param playbackparam[MAX_SIMUL_PLAYBACKS];
	B_DVR_MediaNodeSettings mediaNodeSettings;
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;
	memset(&recordparam, 0, (sizeof(recordparam)));
	memset(&playbackparam, 0, (sizeof(playbackparam)));
	memset(&mediaNodeSettings, 0, (sizeof(mediaNodeSettings)));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;
	strcpy(playbackparam[0].subDir, "bgrec");
	mediaNodeSettings.subDir = playbackparam[0].subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);

	for(i=0; i<MAX_SIMUL_RECORDS; i++) {
		recordparam[i].pathIndex = 2+i;
		rc = OperationRecordServiceStop(tc, &recordparam[i]);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	}

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


void DvrTestRecordServiceRecordWithTimer(CuTest * tc)
{
	B_DVR_ERROR rc;
	DvrTest_Record_Param recordparam;
    memset(&recordparam, 0, sizeof(recordparam));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;

	if(OperationRecordServiceScanParam(&recordparam)) {
		printf("error in opening parameter file. Reverting to Command Line\n");
		fflush(stdin);
		printf("\n Enter the channel number. Path Index for Background Recording 2-5\n");
		scanf("%u", &recordparam.pathIndex);

		fflush(stdin);
		printf("\n Enter the recording time in seconds\n");
		scanf("%u", &recordparam.num_sec);
	}
	GetRandomProgramName(&recordparam);
	strcat(recordparam.programName, "r3");
	printf("*** Recording %s *** \n", (char *) recordparam.programName);

	rc = OperationRecordServiceStart(tc, &recordparam, 0); /* PID Remapping is disabled */
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	sleep(recordparam.num_sec);
	rc = OperationRecordServiceStop(tc, &recordparam);
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);

}

void DvrTestRecordServiceRecordWithDataInjection(CuTest * tc)
{
	B_DVR_ERROR rc;
	int i, slice;
	DvrTest_Record_Param recordparam;
    memset(&recordparam, 0, sizeof(recordparam));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;

	if(OperationRecordServiceScanParam(&recordparam)) {
		printf("error in opening parameter file. Reverting to Command Line\n");
		fflush(stdin);
		printf("\n Enter the channel number. Path Index for Background Recording 2-5\n");
		scanf("%u", &recordparam.pathIndex);

		fflush(stdin);
		printf("\n Enter the PAT/PMT injection time in seconds\n");
		scanf("%u", &recordparam.num_millisec);

		fflush(stdin);
		printf("\n Enter PAT File name:");
		scanf("%s", (char *)&recordparam.patfile);

		fflush(stdin);
		printf("\n Enter PMT File name:");
		scanf("%s", (char *)&recordparam.pmtfile);

		fflush(stdin);
		printf("\n Enter PMT PID:");
		scanf("0x%x", &recordparam.pmt_pid);
	}
	GetRandomProgramName(&recordparam);
	strcat(recordparam.programName, "r4");
	printf("*** Recording %s *** \n", (char *) recordparam.programName);

	rc = OperationRecordServiceStart(tc, &recordparam, 0); /* PID Remapping is disabled */
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	slice = (recordparam.num_millisec / 10);
	/* Inject data every 100 millisec. Sleep for 10000*2 microsec and iterate 5 times */
	for(i=0; i<5; i++) {
		rc = OperationRecordServiceWithDataInjection(tc, &recordparam, INJECTPAT);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
		usleep(recordparam.num_millisec * 100);
		rc = OperationRecordServiceWithDataInjection(tc, &recordparam, INJECTPMT);
		CuAssertTrue(tc, rc==B_DVR_SUCCESS);
		usleep(recordparam.num_millisec * 100);
	}
	sleep(60);
	rc = OperationRecordServiceStop(tc, &recordparam);
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);

}


void DvrTestRecordServiceRecordWithPIDRemapping(CuTest * tc)
{
	B_DVR_ERROR rc;
	DvrTest_Record_Param recordparam;
    memset(&recordparam, 0, sizeof(recordparam));
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)OperationRecordService_Callback;

	if(OperationRecordServiceScanParam(&recordparam)) {
		printf("error in opening parameter file. Reverting to Command Line\n");
		fflush(stdin);
		printf("\n Enter the channel number. Path Index for Background Recording 2-5\n");
		scanf("%u", &recordparam.pathIndex);

		fflush(stdin);
		printf("\n Enter the recording time in seconds\n");
		scanf("%u", &recordparam.num_sec);

		printf("Hardcoding ES = 2; Modify the App for supporting more than 2\n");
		recordparam.num_es = 2;

		fflush(stdin);
		printf("\n Enter the number of Elementary Stream 1:");
		scanf("%u", &recordparam.num_es_pid1);

		fflush(stdin);
		printf("\n Enter the number of Elementary Stream 2:");
		scanf("%u", &recordparam.num_es_pid2);
	}

	GetRandomProgramName(&recordparam);
	strcat(recordparam.programName, "r5");
	printf("*** Recording %s *** \n", (char *) recordparam.programName);

	rc = OperationRecordServiceStart(tc, &recordparam, 1); /* Enable PID Remapping */
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);
	sleep(recordparam.num_sec);
	rc = OperationRecordServiceStop(tc, &recordparam);
	CuAssertTrue(tc, rc==B_DVR_SUCCESS);

}


#if defined(ENABLE_DRM)

void DvrTestRecordServiceClearKeyCpsEncWithTimer(CuTest * tc)
{
    int num_sec;
    int pathIndex = 0;
    char *programName;
    DvrTest_Operation_Param param;
	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyCpsEncWithTimer ************* \n");
	printf(" ****** CPS ENCRYPTION USING CLEARKEY [START] ******************* \n");
	printf(" **************************************************************** \n");

	pathIndex = 2;
    num_sec = 20;
    programName = "CpsEncClearkey";
    
    memset(&param, 0, sizeof(param));
    param.pathIndex = pathIndex;
    strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);
	
	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");

    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsClearKeyStart, &param); 
    sleep(num_sec);
    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsClearKeyStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyCpsEncWithTimer ************* \n");
	printf(" ****** CPS ENCRYPTION USING CLEARKEY [END] ********************* \n");
	printf(" **************************************************************** \n");
	
}

void DvrTestRecordServiceKeyladderCpsEncWithTimer(CuTest * tc)
{
    int num_sec;
    int pathIndex = 0;
    char *programName;
    DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceKeyladderCpsEncWithTimer ************ \n");
	printf(" ****** CPS ENCRYPTION USING KEYLADDER [START] ****************** \n");
	printf(" **************************************************************** \n");

	pathIndex = 2;
    num_sec = 20;
    programName = "CpsEncKeyladder";
    
    memset(&param, 0, sizeof(param));
    param.pathIndex = pathIndex;
    strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");	

    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsKeyladderStart, &param); 
    sleep(num_sec);
    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsKeyladderStop, &param); 
	
	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceKeyladderCpsEncWithTimer ************ \n");
	printf(" ****** CPS ENCRYPTION USING KEYLADDER [END] ******************** \n");
	printf(" **************************************************************** \n");	
}


void DvrTestRecordServiceClearKeyM2mEncWithTimer(CuTest * tc)
{
    int num_sec;
    int pathIndex = 0;
    char *programName;
    DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyM2mEncWithTimer ************* \n");
	printf(" ****** M2M ENCRYPTION USING CLEARKEY [START] ******************* \n");
	printf(" **************************************************************** \n");

	pathIndex = 2;
    num_sec = 20;
    programName = "M2mEncClearkey";
    
    memset(&param, 0, sizeof(param));
    param.pathIndex = pathIndex;
    strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");		

    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceM2mClearKeyStart, &param); 
    sleep(num_sec);
    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceM2mClearKeyStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyM2mEncWithTimer ************* \n");
	printf(" ****** M2M ENCRYPTION USING CLEARKEY [END] ********************* \n");
	printf(" **************************************************************** \n");		
	
}

void DvrTestRecordServiceKeyladderM2mEncWithTimer(CuTest * tc)
{
    int num_sec;
    int pathIndex = 0;
    char *programName;
    DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceKeyladderM2mEncWithTimer ************ \n");
	printf(" ****** M2M ENCRYPTION USING KEYLADDER [START] ****************** \n");
	printf(" **************************************************************** \n");

	pathIndex = 2;
    num_sec = 20;
    programName = "M2mEncKeyladder";
    
    memset(&param, 0, sizeof(param));
    param.pathIndex = pathIndex;
    strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");		

    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceM2mKeyladderStart, &param); 
    sleep(num_sec);
    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceM2mKeyladderStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceKeyladderM2mEncWithTimer ************ \n");
	printf(" ****** M2M ENCRYPTION USING KEYLADDER [END] ******************** \n");
	printf(" **************************************************************** \n");	
	
}


void DvrTestRecordServiceClearKeyCpsPolarityEncWithTimer(CuTest * tc)
{
    int num_sec;
    int pathIndex = 0;
    char *programName;
    DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyCpsPolarityEncWithTimer ***** \n");
	printf(" ****** CPS ENC (EVEV/ODD) USING CLEARKEY [START] *************** \n");
	printf(" **************************************************************** \n");

	pathIndex = 2;
    num_sec = 20;
    programName = "CpsEncClearkeyEvenOdd";

    memset(&param, 0, sizeof(param));
    param.pathIndex = pathIndex;
    strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");	

    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStart, &param); 
    sleep(num_sec);
    Do_DvrTest_Operation(tc, eDvrTest_OperationListRecordServiceCpsPolarityClearKeyStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestRecordServiceClearKeyCpsPolarityEncWithTimer ***** \n");
	printf(" ****** CPS ENC (EVEV/ODD) USING CLEARKEY [END] ***************** \n");
	printf(" **************************************************************** \n");		
}	

#endif

