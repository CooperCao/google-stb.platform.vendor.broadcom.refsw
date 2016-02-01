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
BDBG_MODULE(dvr_test_playbackservice);

extern NEXUS_VideoCodec B_DVR_PlaybackService_P_GetVideoCodec(B_DVR_PlaybackServiceHandle playbackService);
BKNI_EventHandle endOfStreamEvent, beginningOfStreamEvent, alarmTimerEvent;
unsigned long currentEndTime;
#define NUMBEROFLOOPS 1
#define DURATION   /*10800000*/ 60000     /* 180 minutes or 3 hours */


char * audioCodecType[64] =
{
    "NEXUS_AudioCodec_eUnknown = 0",    /* unknown/not supported audio format */
    "NEXUS_AudioCodec_eMpeg",           /* MPEG1/2, layer 1/2. This does not support layer 3 (mp3). */
    "NEXUS_AudioCodec_eMp3",            /* MPEG1/2, layer 3. */
    "NEXUS_AudioCodec_eAac",            /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAacAdts=NEXUS_AudioCodec_eAac",    /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAacLoas",        /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlus",        /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlusLoas =NEXUS_AudioCodec_eAacPlus",    /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlusAdts",    /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAc3",            /* Dolby Digital AC3 audio */
    "NEXUS_AudioCodec_eAc3Plus",        /* Dolby Digital Plus (AC3+ or DDP) audio */
    "NEXUS_AudioCodec_eDts",            /* Digital Digital Surround sound, uses non-legacy frame-sync */
    "NEXUS_AudioCodec_eLpcmDvd",        /* LPCM, DVD mode */
    "NEXUS_AudioCodec_eLpcmHdDvd",      /* LPCM, HD-DVD mode */
    "NEXUS_AudioCodec_eLpcmBluRay",     /* LPCM, Blu-Ray mode */
    "NEXUS_AudioCodec_eDtsHd",          /* Digital Digital Surround sound, HD, uses non-legacy frame-sync, decodes only DTS part of DTS-HD stream */
    "NEXUS_AudioCodec_eWmaStd",         /* WMA Standard */
    "NEXUS_AudioCodec_eWmaStdTs",       /* WMA Standard with a 24-byte extended header */
    "NEXUS_AudioCodec_eWmaPro",         /* WMA Professional */
    "NEXUS_AudioCodec_eAvs",            /* AVS */ 
    "NEXUS_AudioCodec_ePcm",            /* PCM audio - Generally used only with inputs such as SPDIF or HDMI. */ 
    "NEXUS_AudioCodec_ePcmWav",         /* PCM audio with Wave header - Used with streams containing PCM audio */
    "NEXUS_AudioCodec_eAmr",            /* Adaptive Multi-Rate compression (typically used w/3GPP) */
    "NEXUS_AudioCodec_eDra",            /* Dynamic Resolution Adaptation.  Used in Blu-Ray and China Broadcasts. */
    "NEXUS_AudioCodec_eCook",           /* Real Audio 8 LBR */
    "NEXUS_AudioCodec_eAdpcm",          /* MS ADPCM audio format */
    "NEXUS_AudioCodec_eSbc",            /* Sub Band Codec used in Bluetooth A2DP audio */
    "NEXUS_AudioCodec_eDtsLegacy",      /* Digital Digital Surround sound, legacy mode (14 bit), uses legacy frame-sync */
    "NEXUS_AudioCodec_eVorbis",         /* Vorbis audio codec.  Typically used with OGG or WebM container formats. */
    "NEXUS_AudioCodec_eLpcm1394",       /* IEEE-1394 LPCM audio  */
    "NEXUS_AudioCodec_eG711",           /* G.711 a-law and u-law companding.  Typically used for voice transmission. */
    "NEXUS_AudioCodec_eG723_1",         /* G.723.1 Dual Rate Speech Coder for Multimedia Communications.  Used in H.324 and 3GPP 3G-324M.  This is different from G.723, which was superceded by G.726. */
    "NEXUS_AudioCodec_eG726",           /* G.726 ADPCM speech codec.  Supercedes G.723 and G.721. */
    "NEXUS_AudioCodec_eG729",           /* G.729 CS-ACELP speech codec.  Often used in VOIP applications. */
    "NEXUS_AudioCodec_eFlac",           /* Free Lossless Audio Codec (see http://flac.sourceforge.net) */ 
    "NEXUS_AudioCodec_eMax"
};

char* videoCodecList[19] =
{	"NEXUS_VideoCodec_eUnknown", "NEXUS_VideoCodec_eNone", "NEXUS_VideoCodec_eMpeg1",
	"NEXUS_VideoCodec_eMpeg2", "NEXUS_VideoCodec_eMpeg4Part2","NEXUS_VideoCodec_eH263",
	"NEXUS_VideoCodec_eH264","NEXUS_VideoCodec_eH264_Svc","NEXUS_VideoCodec_eH264_Mvc",
	"NEXUS_VideoCodec_eVc1","NEXUS_VideoCodec_eVc1SimpleMain","NEXUS_VideoCodec_eDivx311",
	"NEXUS_VideoCodec_eAvs","NEXUS_VideoCodec_eRv40","NEXUS_VideoCodec_eVp6",
	"NEXUS_VideoCodec_eVp7","NEXUS_VideoCodec_eVp8","NEXUS_VideoCodec_eSpark","NEXUS_VideoCodec_eMax"
};

char* playbackState[6] = 
{"NEXUS_PlaybackState_eStopped", "NEXUS_PlaybackState_ePlaying", "NEXUS_PlaybackState_ePaused",
"NEXUS_PlaybackState_eTrickMode", "NEXUS_PlaybackState_eAborted", "NEXUS_PlaybackState_eMax"
};

static void endOfStreamCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
    BSTD_UNUSED(appContext);
	BSTD_UNUSED(index);

    switch(event)
    {
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream!!!!");
			B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
			currentEndTime = playbackServiceStatus.currentTime;
			BDBG_MSG(("\nCurrent Time: %lu", currentEndTime));            
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
	default:
        {
            printf("\n invalid event");
        }
    }
    printf("\n endOfStreamCallback\n");
	BKNI_SetEvent(endOfStreamEvent);
    return;
}

static void beginningOfStreamCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    BSTD_UNUSED(appContext);
	BSTD_UNUSED(index);
    switch(event)
    {
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n End of playback stream");
            
        }
        break;
    case eB_DVR_EventStartOfPlayback:
        {
            if(service == eB_DVR_ServicePlayback)
            {
              printf("\n Beginnning of playback stream. Pausing!!!");
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
	default:
        {
            printf("\n invalid event");
        }
    }
    printf("\n beginnigOfStreamCallback\n");
	BKNI_SetEvent(beginningOfStreamEvent);
    return;
}

static void alarmTimerCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    BSTD_UNUSED(appContext);
	BSTD_UNUSED(index);
	BSTD_UNUSED(service);
    switch(event)
    {
    	case eB_DVR_EventPlaybackAlarm:
        {
            printf("\n Playback alarm event!!!");
        }
        break;
		default:
        {
            printf("\n Invalid event");
        }
    }
    printf("\n AlarmTimerCallback!!!\n");
	BKNI_SetEvent(alarmTimerEvent);
    return;
}


void DvrTestSetParam(DvrTest_Operation_Param * param)
{
	
	unsigned recordingCount;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    param->pathIndex = 0;
	strcpy(param->subDir, "bgrec");
	
	mediaNodeSettings.subDir = param->subDir;
    mediaNodeSettings.programName = NULL;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = 0;
    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
    B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param->programName, recordingList[0]);
}

void DvrTestPlaybackServiceStatus (CuTest * tc, B_DVR_PlaybackServiceStatus playbackServiceStatus)
{
	B_DVR_ERROR rc =B_DVR_SUCCESS;
	BSTD_UNUSED(tc);

	rc = B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
	if (rc != B_DVR_SUCCESS)
	{
		printf("\nCan't get playback service status\n");
	}
	printf("Start time %lu: \n", playbackServiceStatus.startTime);
	printf("Current time %lu: \n", playbackServiceStatus.currentTime);
	printf("End time %lu: \n", playbackServiceStatus.endTime);
	printf("Current playback status: %s \n", playbackState[playbackServiceStatus.state]);
}

void DvrTestPlaybackServiceInfo (DvrTest_Operation_Param * param)
{
	unsigned esStreamIndex;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;

	mediaNodeSettings.subDir = param->subDir;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;

	mediaNodeSettings.programName = param->programName;
	B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
	printf("\n Media Size %u",(unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset));
	printf("\n nav size %u",(unsigned)(media.navLinearEndOffset - media.navLinearStartOffset));
	printf("\n Media end time (seconds) %u",(unsigned)((media.mediaEndTime)/1000));
	printf("\n Media time (seconds) %u",(unsigned)((media.mediaEndTime - media.mediaStartTime)/1000));
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
		printf("\n %u\n",media.esStreamInfo[esStreamIndex].pid);
		printf(" Audio Codec %s \n",audioCodecType[media.esStreamInfo[esStreamIndex].codec.audioCodec]);
		printf(" Bit rate %u \n", media.esStreamInfo[esStreamIndex].bitRate);
		printf(" Video Heigh/Width %u/%u \n", media.esStreamInfo[esStreamIndex].videoHeight, media.esStreamInfo[esStreamIndex].videoWidth);
		printf(" Profile %d \n", media.esStreamInfo[esStreamIndex].profile);
		printf(" Audio sample rate %u \n", media.esStreamInfo[esStreamIndex].audioSampleRate);
		printf(" Audio channel count %u \n", media.esStreamInfo[esStreamIndex].audioChannelCount);
	}
}

void DvrTestRecordServiceStart (CuTest * tc, DvrTest_Operation_Param * param)
{
    int pathIndex;
	
	BSTD_UNUSED(tc);
	pathIndex = param->pathIndex;
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
        BDBG_MSG(("\n recordService open >>>"));
        g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
        BDBG_MSG(("\n recordService open <<"));
        BDBG_MSG(("\n recordService install callback >>"));
        B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)g_dvrTest);
        BDBG_MSG(("\n recordService install callback <<"));
        
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

		printf("\n recordService start >>");
        B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
        printf("\n recordService start <<");
      
    }
}

void DvrTestRecordServiceStop (CuTest * tc, DvrTest_Operation_Param * param)
{
	int pathIndex;

	BSTD_UNUSED(tc);
	pathIndex = param->pathIndex;
    if(pathIndex > 0 && pathIndex < 2 )
    {
        printf("\n invalid path chosen. Path should be between 2-5");
    }
    else
    {
    	printf("\nStopping record service");
        B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
		printf("\nClosing record service");
        B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].recordService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
        
    }
}


void DvrTestPlaybackServiceWithTimer (CuTest * tc)
{
	unsigned recordingCount;
	DvrTest_Operation_Param param;
	unsigned index;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	NEXUS_VideoCodec videoCodec;

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");

	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	if (recordingCount)
	{
		for(index=0; index<recordingCount; index++)
		{
			strcpy(param.programName, recordingList[index]);
			printf("\nProgram name to playback %s \n", param.programName);
			Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
			videoCodec = B_DVR_PlaybackService_P_GetVideoCodec(g_dvrTest->streamPath[0].playbackService);
			printf("\nVideo codec of playback stream: %s \n", videoCodecList[videoCodec]);
		    sleep(10);
		    Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
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

}

void DvrTestPlaybackServiceWithMediaInfo (CuTest * tc)
{
	unsigned recordingCount;
	DvrTest_Operation_Param param;
	unsigned index;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	NEXUS_VideoCodec videoCodec;

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");

	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	if (recordingCount)
	{
		for(index=0; index<recordingCount; index++)
		{
			strcpy(param.programName, recordingList[index]);
			printf("\nProgram name to playback %s \n", param.programName);
			Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
			videoCodec = B_DVR_PlaybackService_P_GetVideoCodec(g_dvrTest->streamPath[0].playbackService);
			printf("\nVideo codec of playback stream: %s \n", videoCodecList[videoCodec]);
			DvrTestPlaybackServiceInfo(&param);
		    sleep(10);
		    Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
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

}

void DvrTestPlaybackServiceMultiLoops (CuTest * tc)
{
	int i= 0;
	B_DVR_Media media;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	unsigned recordingCount;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
	
	strcpy(param.subDir, "bgrec");

	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param.programName, recordingList[0]);
	
	BKNI_CreateEvent(&endOfStreamEvent);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	printf("\nProgram name to playback: %s \n", param.programName);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[param.pathIndex].playbackService, &media);
	while (i <=NUMBEROFLOOPS)
	{
		BKNI_WaitForEvent(endOfStreamEvent, -1);
		printf("Set seektime to the beginning of stream after endOfStreamEvent\n");
		param.seekTime = media.mediaStartTime;
		Do_DvrTest_Operation(tc, eDvrTest_OperationSeek, &param);
		Do_DvrTest_Operation(tc, eDvrTest_OperationPlay, &param);
		sleep(1);
		i++;
		printf("Completed loop %d \n", i);
	}
	
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);

	if(endOfStreamEvent)
	{
		BDBG_MSG(("\nDestroy endOfStreamEvent\n"));
		BKNI_DestroyEvent(endOfStreamEvent);
	}
	BKNI_Free((void *)(char *)recordingList);
}


void DvrTestPlaybackServiceSingleLoop (CuTest * tc)
{
	DvrTest_Operation_Param param;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	unsigned recordingCount;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

    memset(&param, 0, sizeof(param));
	strcpy(param.subDir, "bgrec");
	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	
	BDBG_MSG(("get recording count\n"));
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param.programName, recordingList[0]);

	BKNI_CreateEvent(&endOfStreamEvent);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	printf("Program name to playback: %s \n", param.programName);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	DvrTestPlaybackServiceInfo(&param);
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
	if(endOfStreamEvent)
	{
		BDBG_MSG(("\nDestroy endOfStreamEvent\n"));
		BKNI_DestroyEvent(endOfStreamEvent);
	}
	BKNI_Free((void *)(char *)recordingList);
}

void DvrTestPlaybackServiceSetAlarmTimer (CuTest * tc)
{
	B_DVR_MediaNodeSettings mediaNodeSettings;
	unsigned recordingCount;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	unsigned long alarmTime = 20000;
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));

	strcpy(param.subDir, "bgrec");
	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param.programName, recordingList[0]);

	BKNI_CreateEvent(&alarmTimerEvent);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)alarmTimerCallback;
	printf("\nProgram name to playback: %s \n", param.programName);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	B_DVR_PlaybackService_SetAlarmTime(g_dvrTest->streamPath[0].playbackService, alarmTime);
	printf("\nAlarmCallback will start and playback will stop in %lu milliseconds \n", alarmTime);
	BKNI_WaitForEvent(alarmTimerEvent, -1);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);

	BKNI_Free((void *)(char *)recordingList);
	BKNI_DestroyEvent(alarmTimerEvent);
}


void DvrTestPlaybackServiceWithTrickMode (CuTest * tc)
{
	int i;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
	unsigned recordingCount;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	DvrTest_Operation_Param param;

    memset(&param, 0, sizeof(param));
	strcpy(param.subDir, "bgrec");
	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param.programName, recordingList[0]);
	BKNI_CreateEvent(&endOfStreamEvent);

	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	
	printf("\nProgram name to playback: %s \n", param.programName);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	sleep(1);
	DvrTestPlaybackServiceStatus(tc, playbackServiceStatus);
	printf("\nStart eDvrTest_OperationFastForward\n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationFastForward, &param);
	DvrTestPlaybackServiceStatus (tc, playbackServiceStatus);
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	printf("\nStart eB_DVR_OperationPlayGop\n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlayGop, &param);
	DvrTestPlaybackServiceStatus(tc, playbackServiceStatus);
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	printf("\nStart eDvrTest_OperationRewind\n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationRewind, &param);
	DvrTestPlaybackServiceStatus(tc, playbackServiceStatus);
	sleep(2);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlay, &param);
	printf("\nStart eDvrTest_OperationSlowForward\n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationSlowForward, &param);
	DvrTestPlaybackServiceStatus (tc, playbackServiceStatus);
	sleep(10);
	printf("\nStart eDvrTest_OperationSlowRewind\n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationSlowRewind, &param);
	DvrTestPlaybackServiceStatus (tc, playbackServiceStatus);
	sleep(5);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPause, &param);
	printf("\nStart eDvrTest_OperationFrameAdvance\n");
	for(i=0; i<=10; i++)
	{
		Do_DvrTest_Operation(tc, eDvrTest_OperationFrameAdvance, &param);
		sleep(1);
	}
	Do_DvrTest_Operation(tc, eDvrTest_OperationPause, &param);
	printf("\nStart eDvrTest_OperationFrameReverse\n");
	for(i=0; i<=10; i++)
	{
		Do_DvrTest_Operation(tc, eDvrTest_OperationFrameReverse, &param);
		sleep(1);
	}
	
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
	if (recordingList)
	{
		printf("\nFree memory allocation for recordingList\n");
		BKNI_Free((void *)(char *)recordingList);
	}
	if (endOfStreamEvent)
	{
		printf("\nDestroy endOfStreamEvent\n");
		BKNI_DestroyEvent(endOfStreamEvent);
	}
	if (beginningOfStreamEvent)
	{
		printf("\nDestroy beginningOfStreamEvent\n");
		BKNI_DestroyEvent(beginningOfStreamEvent);
	}
}

void DvrTestDualPlaybackServiceWithTimer (CuTest * tc)
{
	unsigned recordingCount;
	DvrTest_Operation_Param param;
	unsigned index, pathIndex;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];

	memset(&param, 0, sizeof(param));
	strcpy(param.subDir, "bgrec");

	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	strcpy(param.programName, recordingList[0]);
	B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
	if (recordingCount)
	{
		for(index=0; index<recordingCount; index++)
		{
			strcpy(param.programName, recordingList[index]);
			printf("\nProgram name to playback %s \n", param.programName);
			for(pathIndex=0; pathIndex<2; pathIndex++)
			{
				param.pathIndex = pathIndex;
				Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
			}
			sleep(10);
			Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
			param.pathIndex = 0;
		    Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
		}
	}
	else 
		printf("No recordings found\n");
		BKNI_Free((void *)(char *)recordingList);
}

void DvrTestPlaybackServiceEventCallback (CuTest * tc)
{
	DvrTest_Operation_Param param;
    memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");

	BKNI_CreateEvent(&endOfStreamEvent);
	BKNI_CreateEvent(&beginningOfStreamEvent);
	
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)beginningOfStreamCallback;
	DvrTestSetParam(&param);
	printf("Program Name to playback: %s \n", param.programName);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	printf("endOfStreamEvent fired \n");
	BKNI_WaitForEvent(beginningOfStreamEvent, -1);
	printf("beginningOfStreamEvent fired \n");
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
}

void DvrTestPlaybackVerificationOfLongRecording (CuTest * tc)
{
	DvrTest_Operation_Param param;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	B_DVR_Media media;
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
	B_DVR_ERROR rc =B_DVR_SUCCESS;
	unsigned long playbackEndTime;

    memset(&param, 0, sizeof(param));
	strcpy(param.subDir, "bgrec");
	strcpy(param.programName, "3HourRecord");
	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = param.programName;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	DvrTestRecordServiceStart(tc, &param);
	do{
		B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
	}while((media.mediaEndTime - media.mediaStartTime) <= DURATION);
	DvrTestRecordServiceStop(tc, &param);

	BKNI_CreateEvent(&endOfStreamEvent);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	BDBG_MSG(("\nProgram name to playback: %s", param.programName));
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);

	rc = B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
	if (rc != B_DVR_SUCCESS)
	{
		printf("\nCan't get playback service status");
	}
	BDBG_MSG(("\nPlayback end time %lu\n", playbackServiceStatus.endTime));
	playbackEndTime = playbackServiceStatus.endTime;
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
	if ((playbackEndTime - currentEndTime)/1000 < 1)
	{
		printf("\n### The long recording have passed the integrity check ###\n");
	}
	else
		CuAssert(tc, "FAILED", playbackEndTime == currentEndTime);
	if(endOfStreamEvent)
	{
		BDBG_MSG(("\nDestroy endOfStreamEvent"));
		BKNI_DestroyEvent(endOfStreamEvent);
	}
}

void DvrTestPlaybackInProgressRecording (CuTest * tc)
{
	DvrTest_Operation_Param param;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	B_DVR_Media media;
	B_DVR_PlaybackServiceStatus playbackServiceStatus;
	B_DVR_ERROR rc =B_DVR_SUCCESS;
	unsigned long playbackEndTime;

    memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");
	strcpy(param.programName, "InProgressRecord");
	mediaNodeSettings.subDir = param.subDir;
	mediaNodeSettings.programName = param.programName;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	
	DvrTestRecordServiceStart(tc, &param);
	sleep(10);
	BKNI_CreateEvent(&endOfStreamEvent);
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)endOfStreamCallback;
	BDBG_MSG(("\nStart playback the current recording"));
	BDBG_MSG(("\nProgram name to playback: %s", param.programName));
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	do{
	B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
	}while ((media.mediaEndTime  < (media.mediaStartTime + DURATION)));
	BDBG_MSG(("\nStopping current record and playback"));
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
	DvrTestRecordServiceStop(tc, &param);
	BDBG_MSG(("\nStart playback of just recorded stream"));
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);
	rc = B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[0].playbackService, &playbackServiceStatus);
	if (rc != B_DVR_SUCCESS)
	{
		printf("\nCan't get playback service status");
	}
	BDBG_MSG(("\nPlayback start time %lu\n", playbackServiceStatus.startTime));
	BDBG_MSG(("\nPlayback end time %lu\n", playbackServiceStatus.endTime));
	playbackEndTime = playbackServiceStatus.endTime;
	BKNI_WaitForEvent(endOfStreamEvent, -1);
	Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
	if ((playbackEndTime - currentEndTime)/1000 < 1)
	{
		printf("\n### The recording has passed the integrity check ###\n");
	}
	else
		CuAssert(tc, "FAILED", playbackEndTime == currentEndTime);
	if(endOfStreamEvent)
	{
		BDBG_MSG(("\nDestroy endOfStreamEvent"));
		BKNI_DestroyEvent(endOfStreamEvent);
	}
}


#if defined(ENABLE_DRM)

void DvrTestPlaybackServiceClearKeyCaPolarityDecWithTimer(CuTest * tc)
{

	int num_sec;
	char *programName;
	DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyCaPolarityDecWithTimer***** \n");
	printf(" ****** CA DEC (EVEN/ODD) USING CLEARKEY [START] **************** \n");
	printf(" **************************************************************** \n");

    num_sec = 20;
    programName = "CpsEncClearkeyEvenOdd";

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");
	strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStart, &param); 
	sleep(num_sec);
	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceCaPolarityClearKeyStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyCaPolarityDecWithTimer **** \n");
	printf(" ****** CA DEC (EVEN/ODD) USING CLEARKEY [END] ****************** \n");
	printf(" **************************************************************** \n");	

}

void DvrTestPlaybackServiceClearKeyM2mDecWithTimer(CuTest * tc)
{
	int num_sec;
	char *programName;
	DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyM2mDecWithTimer************ \n");
	printf(" ****** M2M DECRYPTION USING CLEARKEY [START] ******************* \n");
	printf(" **************************************************************** \n");

    num_sec = 20;
    programName = "CpsEncClearkey";

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");	
	strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");	

	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mClearKeyStart, &param); 
	sleep(num_sec);
	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mClearKeyStop, &param); 
	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyM2mDecWithTimer ************* \n");
	printf(" ****** M2M DECRYPTION USING CLEARKEY [END] ********************* \n");
	printf(" **************************************************************** \n");

}
void DvrTestPlaybackServiceKeyladderM2mDecWithTimer(CuTest * tc)
{
	int num_sec;
	char *programName;
	DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceKeyladderM2mDecWithTimer ********** \n");
	printf(" ****** M2M DECRYPTION USING KEYLADDER [START] ****************** \n");
	printf(" **************************************************************** \n");

    num_sec = 20;
    programName = "CpsEncKeyladder";

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");
	strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");	

	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mKeyladderStart, &param); 
	sleep(num_sec);
	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mKeyladderStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceKeyladderM2mDecWithTimer ********** \n");
	printf(" ****** M2M DECRYPTION USING KEYLADDER [END] ******************** \n");
	printf(" **************************************************************** \n"); 

}

void DvrTestPlaybackServiceClearKeyM2mDecWithTimerForM2mEnc(CuTest * tc)
{
	int num_sec;
	char *programName;
	DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyM2mDecWithTimerForM2mEnc*** \n");
	printf(" ****** M2M DECRYPTION USING CLEARKEY [START] ******************* \n");
	printf(" **************************************************************** \n");

    num_sec = 20;
    programName = "M2mEncClearkey";

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");	
	strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");		

	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mClearKeyStart, &param); 
	sleep(num_sec);
	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mClearKeyStop, &param); 
	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceClearKeyM2mDecWithTimerForM2mEnc ** \n");
	printf(" ****** M2M DECRYPTION USING CLEARKEY [END] ********************* \n");
	printf(" **************************************************************** \n");

}
void DvrTestPlaybackServiceKeyladderM2mDecWithTimerForM2mEnc(CuTest * tc)
{
	int num_sec;
	char *programName;
	DvrTest_Operation_Param param;

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceKeyladderM2mDecWithTimerForM2mEnc * \n");
	printf(" ****** M2M DECRYPTION USING KEYLADDER [START] ****************** \n");
	printf(" **************************************************************** \n");

    num_sec = 20;
    programName = "M2mEncKeyladder";

	memset(&param, 0, sizeof(param));
	param.pathIndex = 0;
	strcpy(param.subDir, "bgrec");
	strncpy(param.programName, programName, B_DVR_MAX_FILE_NAME_LENGTH);

	printf(" pathIndex = %d\n",param.pathIndex);
	printf(" second to record = %d sec\n",num_sec);
	printf(" program name = %s\n",param.programName);
	printf(" **************************************************************** \n");		

	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mKeyladderStart, &param); 
	sleep(num_sec);
	Do_DvrTest_Operation(tc, eDvrTest_OperationListPlaybackServiceM2mKeyladderStop, &param); 

	printf("\n \n \n **************************************************************** \n");
	printf(" ****** DvrTestPlaybackServiceKeyladderM2mDecWithTimerForM2mEnc * \n");
	printf(" ****** M2M DECRYPTION USING KEYLADDER [END] ******************** \n");
	printf(" **************************************************************** \n"); 

}

#endif

