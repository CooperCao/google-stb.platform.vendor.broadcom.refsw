/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description: Combination of services in DVR extension library.
 *
 ****************************************************************************/


#include <dvr_test.h>

BDBG_MODULE(dvr_test_complex);

#define MAX_AVPATH 				2
#define DURATION     			60000*2        		/* 2 minute */
#define DURATION_OF_RECORD		60000*2	        	/* 2 hours */
#define NUM_TSB_PATH 			2
#define NUM_TSB_CONVERSION		1
 /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)
#define TRANSCODE0_STC_CHANNEL_INDEX    4
#define TRANSCODE1_STC_CHANNEL_INDEX    5
#define MAX_TSB_TIME                    5*60*1000 /*5mins*/
#define NUM_LOOPS				100
#define MAX_CHANNELS     		3
#define MEDIA_SIZE				1024 *1024 * 200 /* 200MB */
 
 
uint8_t pat[BTST_TS_HEADER_BUF_LENGTH];
uint8_t pmt[BTST_TS_HEADER_BUF_LENGTH];
unsigned patCcValue;
unsigned pmtCcValue;
unsigned dataInsertionCount;

B_EventHandle TSBServiceReady;
B_EventHandle TSBConversionComplete;
B_EventHandle outOfMediaStorageEvent;
B_EventHandle endOfStreamEvent;
B_EventHandle TranscodeServiceReady;
B_EventHandle lockCallbackEvent;

unsigned int numOf_TSBServiceReady = 0;
unsigned int numOf_TSBConversionComplete = 0;
unsigned long currentEndTime;

char* videoCodecTranscode[2] = {"NEXUS_VideoCodec_eMpeg2", "NEXUS_VideoCodec_eH264"};
char* audioCodecTranscode[6] = {"NEXUS_AudioCodec_eMp3", "NEXUS_AudioCodec_eAac", 
                                "NEXUS_AudioCodec_eAacLoas", "NEXUS_AudioCodec_eAacPlus",
                                "NEXUS_AudioCodec_eAacPlusAdts", "NEXUS_AudioCodec_eAc3"};
char videoCodecName[8], audioCodecName[8];


extern void DvrTestLiveDecodeStart(int index);
extern void DvrTestLiveDecodeStop(int index);
extern void DvrTestPlaybackDecodeStart(int index);
extern void DvrTestPlaybackDecodeStop(int index);
extern void DvrTestTsbBufferPreallocate(int numOfTsb);
extern void DvrTestTsbBufferDeallocate(int numOfTsb);
extern void DvrTestTsbServiceStopOperation(int pathIndex);
extern NEXUS_VideoCodec B_DVR_PlaybackService_P_GetVideoCodec(B_DVR_PlaybackServiceHandle playbackService);
extern void B_DvrTestTsbBufferPreallocate(int numOfTsb, char *tsbProgramName);
extern void B_DvrTestTsbBufferDeallocate(int numOfTsb, char *tsbProgramName);
extern void DvrTestRecordServiceStart (CuTest * tc, DvrTest_Operation_Param * param);
extern void DvrTestRecordServiceStop (CuTest * tc, DvrTest_Operation_Param * param);
static void DvrTestInjectData(void *streamPathContext);
extern void Dvr_Get_VideoCodec_Name(unsigned vCodecType);
extern void Dvr_Get_AudioCodec_Name(unsigned aCodecType);
extern void Dvr_Set_VideoCodec_Name(unsigned vCodecType);
extern void Dvr_Set_AudioCodec_Name(unsigned aCodecType);
extern B_DVR_ERROR DvrTestFileToFileTranscodeStop(unsigned pathIndex);
extern void DvrTest_TuneChannel(CuTest * tc, int index);



void B_DvrTestRecordServiceStart (int pathIndex, char *programName )
{
    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
    B_DVR_RecordServiceInputEsStream inputEsStream;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
    B_DVR_RecordServiceSettings recordServiceSettings;
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
    BDBG_MSG(("\n recording name: %s", programName));
    strncpy(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName, programName, 
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


void B_DvrTestRecordServiceStop (int pathIndex)
{
	printf("\nStopping record service");
    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
	printf("\nClosing record service");
    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
    g_dvrTest->streamPath[pathIndex].recordService = NULL;
    g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
}

void DvrTestMediaListing (char * subDir)
{
	unsigned recordingCount;
	unsigned esStreamIndex;
	int index;
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
	
	/* Display recording list */
	mediaNodeSettings.subDir = &subDir[0];
	mediaNodeSettings.programName = NULL;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	printf("\n number of recordings %u",recordingCount);
	if(recordingCount)
	{
		recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
		B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
		printf("\n********************************************************************");
		printf("\n						 Recording List 							  ");
		printf("\n********************************************************************");
		for(index=0;(unsigned)index < recordingCount;index++)
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
static void B_DvrTestCallback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
	char *eventString[eB_DVR_EventMax+1] = {"recStart","recEnd","hdStream","sdStream","tsbConvCompleted","overFlow",
											"underFlow","pbStart","pbEnd","pbAlarm","abortPb","abortRec","abortTSBRec",
											"abortTSBpb","dataInjection","noMediaDiskSpace","noNavDiskSpace",
											"noMetaDataDiskSpace","invalidEvent","eB_DVR_EventTranscodeFinish"};
	char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService",
												"storagaService","dataInjectionService","invalidService"};
	struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)appContext;
	/*B_Mutex_Lock(streamPath->streamPathMutex);*/
	B_Mutex_Lock(g_dvrTest->streamPath[index].streamPathMutex);
	printf("\n\n### Mutex locked ###\n\n");
	if(event != eB_DVR_EventDataInjectionCompleted)
	{
		printf("\n DvrTestCallback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
		printf("Print media files under directory tsbConv \n");
		DvrTestMediaListing("tsbConv");
		printf("\nPrint media files under directory bgrec \n");
		DvrTestMediaListing("bgrec");
	}
	switch(event)
	{
	case eB_DVR_EventStartOfRecording:
	    {
	        if(service == eB_DVR_ServiceTSB)
	        {
				printf("\n Beginning Of TSB Conversion ");
				++numOf_TSBServiceReady;
				if (numOf_TSBServiceReady == NUM_TSB_PATH)
				{
					B_Event_Set(TSBServiceReady);
					BDBG_MSG(("Reset num_OfTSBServiceReady"));
					numOf_TSBServiceReady = 0;
				}
	        }
	        else
	        {
	            printf("\n Beginning of background recording");
				/*B_Event_Set(startOfRecordingEvent);*/
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
				/*B_Event_Set(endOfRecordingEvent);*/
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
				/*B_Event_Set(endOfRecordingEvent);*/
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
				/*B_Event_Set(endOfRecordingEvent);*/
	        }
	    }
	    break;
	case eB_DVR_EventTSBConverstionCompleted:
	    {
	        printf("\n TSB Conversion completed");
	        ++numOf_TSBConversionComplete;
	         if (numOf_TSBConversionComplete == NUM_TSB_CONVERSION)
			 {
			 	B_Event_Set(TSBConversionComplete);
		 		printf("\nReset numOf_TSBConversionComplete to 0\n");
				numOf_TSBConversionComplete = 0;
	         }
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
			B_DVR_OperationSettings operationSettings;
			printf("\n\n### Set eB_DVR_OperationFastRewind ###\n");
            operationSettings.operation = eB_DVR_OperationFastRewind;
            operationSettings.operationSpeed = 2;
			printf("\nIndex = %d ", index);
			if (g_dvrTest->streamPath[index].tsbService)
            /*if(streamPath->tsbService)*/
            {  
                B_DVR_TSBServiceStatus status;
                printf("\n End of tsb. Pausing");
                B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService/*streamPath->tsbService*/,&status);
                printf("\n TSB playback Status:");
                BDBG_ERR(("\n Time bounds:"));
                BDBG_ERR(("\n start:%ld current:%ld end:%ld",
                          status.tsbRecStartTime,status.tsbCurrentPlayTime,
                          status.tsbRecEndTime));
                BDBG_ERR(("\n ByteOffset Bounds:")); 
                BDBG_ERR(("\n start:%lld current:%lld end:%lld",
                          status.tsbRecLinearStartOffset,status.tsbCurrentPlayOffset,
                          status.tsbRecLinearEndOffset));
                B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[index].tsbService/*streamPath->tsbService*/,&operationSettings);
				B_Event_Set(endOfStreamEvent);
            }
            else
            {
		        printf("\n End of playback stream. Pausing");
				B_Event_Set(endOfStreamEvent);
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
			if(streamPath->tsbService)
			{	
				BDBG_MSG(("\nB_DVR_TSBService_InjectDataStop >>>"));
				B_DVR_TSBService_InjectDataStop(streamPath->tsbService);
				B_Scheduler_CancelTimer(g_dvrTest->PFscheduler,streamPath->dataInjectTimer);
				BDBG_MSG(("\nB_DVR_TSBService_InjectDataStop <<<"));				
			}
            else
            {
				BDBG_MSG(("\n## B_DVR_RecordService_InjectDataStop >>>"));
				B_DVR_RecordService_InjectDataStop(streamPath->recordService);
				B_Scheduler_CancelTimer(g_dvrTest->PFscheduler,streamPath->dataInjectTimer);
				BDBG_MSG(("\n## B_DVR_RecordService_InjectDataStop <<<"));
            }
			
			printf("\nSchedule dataInjection in index %d \n", index);
			g_dvrTest->streamPath[index].dataInjectionStarted = false;
			g_dvrTest->streamPath[index].dataInjectTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
				                                                                 g_dvrTest->glbstreamMutex,
				                                                                5000,DvrTestInjectData,
				                                                                 &g_dvrTest->streamPath[index]);
	    }
	    break;
	case eB_DVR_EventTranscodeFinish:
        {
            printf("\n Transcode finish");
            BKNI_SetEvent(g_dvrTest->streamPath[index].transcodeFinishEvent);
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
	if(event != eB_DVR_EventDataInjectionCompleted) 
    {
		printf("\n DvrTestCallback !!!!!!!!!!!! <<<\n");
	}
	
	/*B_Mutex_Unlock(streamPath->streamPathMutex);*/
	B_Mutex_Unlock(g_dvrTest->streamPath[index].streamPathMutex);
	return;
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendFastStatus status;
    NEXUS_FrontendQamStatus qamStatus;
    NEXUS_Error rc;

    BSTD_UNUSED(param);

    rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
    if(rc == NEXUS_SUCCESS)
    {
        if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
            fprintf(stderr, "Frontend locked.\n");
        else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
            fprintf(stderr, "Frontend unlocked.\n");
        else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
            fprintf(stderr, "No signal at the tuned frequency.\n");
    }
    else if(rc == NEXUS_NOT_SUPPORTED)
	{
    	NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        fprintf(stderr, "QAM Lock callback, frontend 0x%08x - lock status %d, %d\n", (unsigned)frontend, qamStatus.fecLock, qamStatus.receiverLock);
    }
    else
         if(rc){rc = BERR_TRACE(rc);}
}

void B_DvrTest_Create_PatPmt(int pathIndex,uint16_t pcrPid,uint16_t vidPid,uint16_t audPid,NEXUS_VideoCodec videoCodec,NEXUS_AudioCodec audioCodec)
{
	uint8_t pat_pl_buf[BTST_TS_HEADER_BUF_LENGTH], pmt_pl_buf[BTST_TS_HEADER_BUF_LENGTH];
	size_t pat_pl_size, pmt_pl_size;
	size_t buf_used = 0;
	size_t payload_pked = 0;
	unsigned streamNum;
	uint8_t  vidStreamType=0x2;
	uint8_t  audStreamType=0x4;
	TS_PAT_state patState;
	TS_PSI_header psi_header;
	TS_PMT_state pmtState;
	TS_PAT_program program;
	TS_PMT_stream pmt_stream;
	TS_PID_info pidInfo;
	TS_PID_state pidState;
	unsigned index=0;

	switch(videoCodec)
	{
	    case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
	    case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
	    case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
	    case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
	    default:
	        printf("Video encoder codec %d is not supported!\n",videoCodec);
	}
	switch(audioCodec)
	{
	        case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
	        case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eAacAdts:      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0xf; break; /* ADTS */
	        case NEXUS_AudioCodec_eAacLoas:      audStreamType = 0x11; break;/* LOAS */
	        case NEXUS_AudioCodec_eAacPlusLoas:  audStreamType = 0x11; break;/* LOAS */
	        case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
	        default:
	            printf("Audio encoder codec %d is not supported!\n", audioCodec);
	}
	/* == CREATE PSI TABLES == */
	TS_PSI_header_Init(&psi_header);
	TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

	TS_PAT_program_Init(&program, 1, BTST_MUX_PMT_PID);
	TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BTST_TS_HEADER_BUF_LENGTH);

	TS_PMT_setPcrPid(&pmtState, pcrPid);

	TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
	TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

	if(audPid)
	{
	    TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
	    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
	}

	TS_PAT_finalize(&patState, &pat_pl_size);
	TS_PMT_finalize(&pmtState, &pmt_pl_size);

	/* == CREATE TS HEADERS FOR PSI INFORMATION == */
	TS_PID_info_Init(&pidInfo, 0x0, 1);
	pidInfo.pointer_field = 1;
	TS_PID_state_Init(&pidState);
	TS_buildTSHeader(&pidInfo, &pidState,g_dvrTest->streamPath[pathIndex].pat, BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
	BKNI_Memcpy((uint8_t*)g_dvrTest->streamPath[pathIndex].pat + buf_used, pat_pl_buf, pat_pl_size);
	printf("pmt_pl_size %u buf_used %u",pat_pl_size, buf_used);
	TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
	pidInfo.pointer_field = 1;
	TS_PID_state_Init(&pidState);
	TS_buildTSHeader(&pidInfo, &pidState,g_dvrTest->streamPath[pathIndex].pmt, BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
	BKNI_Memcpy((uint8_t*)g_dvrTest->streamPath[pathIndex].pmt + buf_used, pmt_pl_buf, pmt_pl_size);
	printf("pmt_pl_size %u buf_used %u",pmt_pl_size,buf_used);
	printf("\n \n PAT Dump\n[");
	for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
	{
	    printf("%02x ",g_dvrTest->streamPath[pathIndex].pat[index+1]);
	    if ((index % 16) == 15)
	    {
	        printf("]\n[");
	    }
	}
	printf("\n\n");
	BKNI_Printf("PMT Dump\n[");
	for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
	{
	    printf("%02x ",g_dvrTest->streamPath[pathIndex].pmt[index+1]);
	    if ((index % 16) == 15)
	    {
	        printf("]\n[");
	    }
	}
	printf("\n\n");
	return;
}

void DvrTestTsbServiceStart(int pathIndex, char *programName)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	unsigned int currentLiveChannel = 0;
	if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex < 2)
    {
        DvrTestLiveDecodeStart(pathIndex);
    }
    BDBG_MSG(("\n TSBService starting on channel %u and path %d",currentLiveChannel,pathIndex));
    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName,
            sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName),
            "%s%d",programName,pathIndex);
    BDBG_MSG(("\n TSBService programName %s",g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName));
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.volumeIndex = 0;
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir,
				sizeof(g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir),"%s","tsb");
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.recpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.playpumpIndex = pathIndex;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
	g_dvrTest->streamPath[pathIndex].tsbServiceRequest.maxTSBTime = MAX_TSB_TIME;
    g_dvrTest->streamPath[pathIndex].tsbServiceRequest.input = eB_DVR_TSBServiceInputQam;
    g_dvrTest->streamPath[pathIndex].tsbService = B_DVR_TSBService_Open(&g_dvrTest->streamPath[pathIndex].tsbServiceRequest);
	printf("\nB_DVR_TSBService_InstallCallback >>>");
    B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
	printf("\nB_DVR_TSBService_InstallCallback <<<");
    BKNI_Memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    dataInjectionOpenSettings.fifoSize = 188*30; /* max number of packets that can be dumped in one shot */
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    tsbServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    tsbServiceSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);

    BKNI_Memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo[currentLiveChannel].videoPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentLiveChannel].videoFormat[0];
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

    inputEsStream.esStreamInfo.pid = channelInfo[currentLiveChannel].audioPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentLiveChannel].audioFormat[0];
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

	printf("\nB_DvrTest_Create_PatPmt >>>");
	B_DvrTest_Create_PatPmt(pathIndex,
                                  channelInfo[currentLiveChannel].pcrPid[0],
                                  channelInfo[currentLiveChannel].videoPids[0],
                                  channelInfo[currentLiveChannel].audioPids[0],
                                  channelInfo[currentLiveChannel].videoFormat[0],
                                  channelInfo[currentLiveChannel].audioFormat[0]);
    g_dvrTest->streamPath[pathIndex].patMonitorTimer = NULL;
	BDBG_MSG(("\nCreate Mutex >>>"));
	g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
	BDBG_MSG(("\nCreate Mutex <<<"));
	assert(g_dvrTest->streamPath[pathIndex].streamPathMutex);
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

void DvrTestTsbServiceStop(int pathIndex)
{
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
		printf("\nStop TSB service on index %d >>>\n", pathIndex);
        dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
		printf("\nStop TSB service on index %d <<<\n", pathIndex);
        if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
        {
			printf("\nDvrTestPlaybackDecodeStop >>>");
            DvrTestPlaybackDecodeStop(pathIndex);
			printf("\nDvrTestPlaybackDecodeStop <<<");
        }
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in stopping the timeshift service %d\n",pathIndex);
        }
		printf("\nB_DVR_TSBService_RemoveCallback >>>");
        B_DVR_TSBService_RemoveCallback(g_dvrTest->streamPath[pathIndex].tsbService);
		printf("\nB_DVR_TSBService_Close >>>");
        dvrError = B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
		printf("\nB_DVR_DataInjectionService_RemoveCallback >>>");
		B_DVR_DataInjectionService_RemoveCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService);
		printf("\nB_DVR_DataInjectionService_Close >>>");
        B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
        g_dvrTest->streamPath[pathIndex].tsbService = NULL;
        g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
        BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
        if(dvrError != B_DVR_SUCCESS)
        {
            printf("\n Error in closing the timeshift service %d\n",pathIndex);
        }
		/*B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);*/
    }
    else
    {
        printf("\n timeshift srvc not started");
    }

}

void DvrTestTsbServicePlayStart(int pathIndex)
{
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus tsbServiceStatus;

    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
        DvrTestLiveDecodeStop(pathIndex);
    }
    DvrTestPlaybackDecodeStart(pathIndex);
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
    operationSettings.operation = eB_DVR_OperationPause;
	printf("\n#### eB_DVR_OperationPause ###\n");
    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
}

void DvrTsbServicePlayStop(int pathIndex)
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
        DvrTestPlaybackDecodeStop(pathIndex);
        if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
        {
            DvrTestLiveDecodeStart(pathIndex);
        }
    }
}


void B_DvrTestTuneChannel(int index)
{	
	NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
	printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);

    if (g_dvrTest->streamPath[index].userParams.isMtsif)
	{
	    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
		g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.mtsif = g_dvrTest->streamPath[index].frontend;
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


void B_DvrTestChannelChange(CuTest * tc, int pathIndex)
{
	int channelUp = 1;

	if(g_dvrTest->streamPath[pathIndex].tsbService)
	{
		BDBG_MSG(("\n Invalid operation! Stop the timeshift srvc on this channel"));
		DvrTestTsbServiceStop(pathIndex);
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
	/*B_DvrTestTuneChannel(pathIndex);*/
	DvrTest_TuneChannel(tc, pathIndex);
	if (pathIndex < MAX_AV_PATH)
	{
		printf("\n### Current channel %d title: %s in window %d #####", g_dvrTest->streamPath[pathIndex].currentChannel, channelInfo[pathIndex].programTitle, pathIndex);
		DvrTestLiveDecodeStart(pathIndex);
		g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
	}
}


static void DvrTestPatMonitor(void *streamPathContext)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_DVR_MediaFileStats mediaFileStats;
    ssize_t returnSize;
    B_Mutex_Lock(streamPath->streamPathMutex);

	B_DVR_MediaFile_Stats(streamPath->patFile,&mediaFileStats);
	
    if(mediaFileStats.endOffset - mediaFileStats.currentOffset < 1024*188) 
    {
        goto error;
    }
    if((mediaFileStats.currentOffset/B_DVR_MEDIA_SEGMENT_SIZE) > streamPath->patSegmentCount) 
    {
        if(streamPath->tsbService) 
        {
            BDBG_ERR(("\n DvrTest_PatMonitor %s %lld >>>",streamPath->tsbServiceRequest.programName,mediaFileStats.currentOffset));
        }
        else
        {
            BDBG_ERR(("\n DvrTest_PatMonitor %s %lld >>>",streamPath->recordServiceRequest.programName,mediaFileStats.currentOffset));
        }
        BDBG_ERR(("media start %lld media Current %lld media end %lld",mediaFileStats.startOffset,mediaFileStats.currentOffset,mediaFileStats.currentOffset));
        if(!streamPath->psiCount)
        {
            BDBG_ERR((" @@@@ no PSI packets found between %lld-%lld",(mediaFileStats.currentOffset-B_DVR_MEDIA_SEGMENT_SIZE),mediaFileStats.currentOffset));
        }
        else
        {
            BDBG_ERR((" %u PSI packets found between %lld-%lld",streamPath->psiCount,(mediaFileStats.currentOffset-B_DVR_MEDIA_SEGMENT_SIZE),mediaFileStats.currentOffset));
            streamPath->psiCount =0;
            streamPath->patSegmentCount++;
        }
    }
    
    if((mediaFileStats.endOffset- mediaFileStats.currentOffset) >= 188*1024) 
    {
        unsigned packetCount=0;
        uint8_t *buffer;
        returnSize = B_DVR_MediaFile_Read(streamPath->patFile,streamPath->patBuffer,1024*188);
        if(returnSize <=0) 
        {
            printf("patRead failed");
            goto error;
        }
        buffer = (uint8_t *)streamPath->patBuffer;
        if(buffer[0]!=0x47) 
        {
            unsigned syncIndex=0;
            printf("\n sync byte not found at buffer[0] %u",buffer[0]);
            for(syncIndex=0;syncIndex<188*2;syncIndex++)
            {
                if(buffer[syncIndex]==0x47)
                break; 
            }
            buffer = (uint8_t *)streamPath->patBuffer + syncIndex;
            returnSize -= syncIndex;
            printf("\n new sync byte %u",buffer[0]);
        }
        for(packetCount=0;packetCount < ((unsigned)returnSize)/188;packetCount++) 
        {
            if( ((buffer[1] & 0x1F) == 0x00) && (buffer[2] == 0x00) )
            {
                streamPath->psiCount++;
            }
            buffer+=188;
        }
    }

 error:
    streamPath->patMonitorTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
                                                         g_dvrTest->glbstreamMutex,
                                                         100,DvrTestPatMonitor,
                                                         (void*)streamPath);
    B_Mutex_Unlock(streamPath->streamPathMutex);
    return;
}


static void DvrTestInjectData(void *streamPathContext)
{
    uint8_t ccByte=0;
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_Mutex_Lock(streamPath->streamPathMutex);
    if(!streamPath->dataInjectionStarted)
    {
		printf("\n if(!streamPath->dataInjectionStarted)\n");
        streamPath->dataInsertionCount++;
        if(streamPath->dataInsertionCount%2 == 0)
        {
            ++streamPath->patCcValue;
            ccByte = streamPath->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
            ccByte = (ccByte & 0xf0) | (streamPath->patCcValue & 0xf);
            streamPath->pat[4] = ccByte;
            if(streamPath->tsbService) 
            {
				printf("\n### B_DVR_TSBService_InjectDataStart ## \n");
                B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pat[1]),188);				
            }
            else
            {
				printf("\n### B_DVR_RecordService_InjectDataStart ## \n");
                B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pat[1]),188);
            }
        }
        else
        {
            ++streamPath->pmtCcValue;
            ccByte = streamPath->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
            ccByte = (ccByte & 0xf0) | (streamPath->pmtCcValue & 0xf);
            streamPath->pmt[4] = ccByte;
            if(streamPath->tsbService) 
            {
				printf("\n### B_DVR_TSBService_InjectDataStart ## \n");
                B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pat[1]),188);				
            }
            else
            {
				printf("\n### B_DVR_RecordService_InjectDataStart ## \n");
                B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pat[1]),188);
            }
        }
    }
    B_Mutex_Unlock(streamPath->streamPathMutex);
    return;
}

void DvrTestDataInjectionStart (int pathIndex/*, char *programName*/)
{
	B_DVR_DataInjectionServiceSettings settings;
    B_DVR_MediaFileOpenMode openMode;
    B_DVR_MediaFilePlayOpenSettings openSettings;
    B_DVR_MediaFileStats mediaFileStats;
    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
    off_t returnOffset;
	char *programName;

    
    g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
    g_dvrTest->streamPath[pathIndex].patCcValue=0;
    g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
    g_dvrTest->streamPath[pathIndex].psiCount=0;
    g_dvrTest->streamPath[pathIndex].patSegmentCount=0;
	printf("\nDataInjectionService_InstallCallback >>>");
    B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,
								g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);								
	printf("\nDataInjectionService_InstallCallback <<<");
	settings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    settings.pid = 0x0; /*unused*/
    B_DVR_DataInjectionService_SetSettings(g_dvrTest->streamPath[pathIndex].dataInjectionService, &settings);
	
	openMode = eB_DVR_MediaFileOpenModeStreaming;

    openSettings.playpumpIndex = 8;
    openSettings.volumeIndex = 0;
    if(g_dvrTest->streamPath[pathIndex].tsbService) 
    {
        strcpy(openSettings.subDir,g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir);
		BDBG_MSG(("\n## Data inject on programName %s in directory %s ###\n", g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName, 
																			openSettings.subDir));
        programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
    }
    else
    {
        strcpy(openSettings.subDir,g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir);		
        programName = g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName;
		BDBG_MSG(("\n## Data inject on programName %s in directory %s ###\n", programName, openSettings.subDir));
    }
    g_dvrTest->streamPath[pathIndex].patFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
    if(!g_dvrTest->streamPath[pathIndex].patFile)
    {
        printf("\n failed to open pat file");
        assert(0);                
    }
    g_dvrTest->streamPath[pathIndex].patBuffer = BKNI_Malloc(188*1024 + B_DVR_IO_BLOCK_SIZE);
    g_dvrTest->streamPath[pathIndex].patBuffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)g_dvrTest->streamPath[pathIndex].patBuffer);
    BKNI_Memset(g_dvrTest->streamPath[pathIndex].patBuffer,0,188*1024);
    if(!g_dvrTest->streamPath[pathIndex].patBuffer)
    {
        printf("\n error patBuffer alloc");
        assert(0);
    }

    mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
    mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
    returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->streamPath[pathIndex].patFile,&mediaFileSeekSettings);
    if(returnOffset < 0)
    {
        printf("\n error seeking into patFile");
        assert(0);
    }

    g_dvrTest->streamPath[pathIndex].dataInjectTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
                                                       g_dvrTest->glbstreamMutex,
                                                       100,DvrTestInjectData,
                                                       &g_dvrTest->streamPath[pathIndex]);

    g_dvrTest->streamPath[pathIndex].patMonitorTimer = B_Scheduler_StartTimer(g_dvrTest->PFscheduler,
                                                       g_dvrTest->glbstreamMutex,
                                                       100,DvrTestPatMonitor,
                                                       &g_dvrTest->streamPath[pathIndex]);

    if(g_dvrTest->streamPath[pathIndex].tsbService)
    {
        B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
    }
    else if(g_dvrTest->streamPath[pathIndex].recordService)
    {
		B_DVR_RecordService_InjectDataStart(g_dvrTest->streamPath[pathIndex].recordService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
    }
    else
    {
        printf("\n neither record or tsb is active on path %d",pathIndex);
    }
    g_dvrTest->streamPath[pathIndex].dataInjectionStarted=true;            
}

void DvrTestDataInjectionStop(int pathIndex)
{
	if(g_dvrTest->streamPath[pathIndex].dataInjectionStarted) 
    {		
        g_dvrTest->streamPath[pathIndex].dataInjectionStarted=false;
        if(g_dvrTest->streamPath[pathIndex].tsbService)
        {
	      	B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
        }
        else if (g_dvrTest->streamPath[pathIndex].recordService)
        {
            B_DVR_RecordService_InjectDataStop(g_dvrTest->streamPath[pathIndex].recordService);
        }
        else
        {
            printf("\n neither record or tsb is active on path %d",pathIndex);
        }
    }

    if(g_dvrTest->streamPath[pathIndex].dataInjectTimer)
    {
		BDBG_MSG(("\nB_Scheduler_CancelTimer dataInjectTimer\n"));
        B_Scheduler_CancelTimer(g_dvrTest->PFscheduler,g_dvrTest->streamPath[pathIndex].dataInjectTimer);
    }
    if(g_dvrTest->streamPath[pathIndex].patMonitorTimer) 
    {
		BDBG_MSG(("\nB_Scheduler_CancelTimer patMonitorTimer\n"));
        B_Scheduler_CancelTimer(g_dvrTest->PFscheduler,g_dvrTest->streamPath[pathIndex].patMonitorTimer);
    }
    if(g_dvrTest->streamPath[pathIndex].patFile) 
    {
		BDBG_MSG(("\nB_DVR_MediaFile_Close\n"));
        B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].patFile);
    }
    g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
    g_dvrTest->streamPath[pathIndex].patCcValue=0;
    g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
    g_dvrTest->streamPath[pathIndex].patSegmentCount=0;
}

B_DVR_ERROR B_DvrTestFileToFileTranscode(unsigned pathIndex, unsigned videoCodec, 
unsigned audioCodec, char *subDir, char *programName)
{
	B_DVR_ERROR rc = B_DVR_SUCCESS;
	B_DVR_Media media;
	B_DVR_RecordServiceSettings recordServiceSettings;
	NEXUS_StcChannelSettings stcSettings;
	B_DVR_TranscodeServiceInputEsStream transcodeInputEsStream;
	B_DVR_TranscodeServiceInputSettings transcodeInputSettings;
	B_DVR_RecordServiceInputEsStream recordInputEsStream;
	NEXUS_PidChannelHandle transcodePidChannel;
	B_DVR_TranscodeServiceSettings transcodeServiceSettings;
	B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	B_DVR_MediaNodeSettings mediaNodeSettings;
	BSTD_UNUSED(videoCodec);
	BSTD_UNUSED(audioCodec);
	
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop >>>"));
    /* stop live decode*/
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTestLiveDecodeStop(pathIndex);
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop <<<"));
	
	mediaNodeSettings.subDir = subDir;
	mediaNodeSettings.programName = programName;
	mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	mediaNodeSettings.volumeIndex = 0;
	B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
	BDBG_MSG(("\nMedia video pid %u ", media.esStreamInfo[0].pid));
	BDBG_MSG(("\nMeida audio pid %u ", media.esStreamInfo[1].pid));
	
	g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
	if(!g_dvrTest->streamPath[pathIndex].streamPathMutex) 
	{
		printf("\n g_dvrTest->streamPath[pathIndex].streamPathMutex create failed");
		assert(0);
	}
	
	BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(B_DVR_TranscodeServiceRequest));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open >>>"));
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
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open >>>"));
	g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.displayIndex = 3;
	g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType = eB_DVR_TranscodeServiceType_NonRealTime;
	/* open transcode service */
	g_dvrTest->streamPath[pathIndex].transcodeService = B_DVR_TranscodeService_Open(&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest);
	if(!g_dvrTest->streamPath[pathIndex].transcodeService)
	{
		printf("\n transcode service open failed");
		goto error_transcodeService;
	}
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Open <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback >>>"));
	B_DVR_TranscodeService_InstallCallback(g_dvrTest->streamPath[pathIndex].transcodeService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings >>>"));
	B_DVR_TranscodeService_GetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
	transcodeInputSettings.input = eB_DVR_TranscodeServiceInput_File;
	/*transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;*/
	transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
	transcodeInputSettings.hdmiInput = NULL;
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings >>>"));
	B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid>>>"));
	BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
	BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&media.esStreamInfo[0],sizeof(media.esStreamInfo[0]));
	
	transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
	transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
	transcodeInputEsStream.videoEncodeParams.codec = NEXUS_VideoCodec_eH264;
	transcodeInputEsStream.videoEncodeParams.interlaced = false;
	transcodeInputEsStream.videoEncodeParams.profile = NEXUS_VideoCodecProfile_eBaseline;
	transcodeInputEsStream.videoEncodeParams.videoDecoder = g_dvrTest->streamPath[pathIndex].videoDecoder;
	transcodeInputEsStream.videoEncodeParams.level = NEXUS_VideoCodecLevel_e31;
	/* add video ES stream*/
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid >>>"));
	B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid >>>"));
	BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
	transcodeInputEsStream.audioEncodeParams.audioDecoder = g_dvrTest->streamPath[pathIndex].audioDecoder;
	transcodeInputEsStream.audioEncodeParams.audioPassThrough = false;
	transcodeInputEsStream.audioEncodeParams.rateSmoothingEnable=true;
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
#if (BCHP_VER >= BCHP_VER_B1)
	transcodeInputEsStream.audioEncodeParams.audioDummy = g_dvrTest->platformconfig.outputs.audioDummy[pathIndex];
#endif
	BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&media.esStreamInfo[1],sizeof(media.esStreamInfo[1]));

	/* add audio ES stream*/
	B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>"));
	BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
	transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
	transcodeInputEsStream.esStreamInfo.pid = media.esStreamInfo[1].pid+1;
	transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
	/* add pcr ES stream*/
	B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings >>>"));
	B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
									   &transcodeServiceSettings);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings <<<"));
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings >>>"));
	transcodeServiceSettings.displaySettings.format = NEXUS_VideoFormat_e1080i;
	transcodeServiceSettings.stgSettings.enabled = true;
	if(0) /*g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)*/
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
	transcodeServiceSettings.videoEncodeSettings.frameRate = NEXUS_VideoFrameRate_e29_97;
	transcodeServiceSettings.videoEncodeSettings.streamStructure.framesB = 0;
	transcodeServiceSettings.videoEncodeSettings.streamStructure.framesP = 23;
	transcodeServiceSettings.videoEncodeSettings.enableFieldPairing = true;
	
	B_DVR_TranscodeService_SetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
									   &transcodeServiceSettings);
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open >>>"));
	g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
	BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName,
				  B_DVR_MAX_FILE_NAME_LENGTH,"%s",programName);
	
	BDBG_MSG(("\n\n### Transcoded file name %s ###\n\n", g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName));
	sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"transcode"/*subDir*/);
	g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex = 0;
	g_dvrTest->streamPath[pathIndex].recordServiceRequest.segmented = true;
	g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = B_DVR_RecordServiceInputTranscode;

	#if 0
	B_DvrTest_Create_PatPmt(pathIndex,
						  playbackMedia.esStreamInfo[1].pid +1,
						  playbackMedia.esStreamInfo[0].pid, 
						  playbackMedia.esStreamInfo[1].pid,
						  NEXUS_VideoCodec_eH264,
						  NEXUS_AudioCodec_eAac);
	#endif
	/*Open record service*/
	g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
	if(!g_dvrTest->streamPath[pathIndex].recordService)
	{
		printf("record service open for transcode operation failed");
		goto error_recordService;
	}
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open <<<");
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback >>>");
	B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback <<<");

	dataInjectionOpenSettings.fifoSize = 30*188;
	g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
	assert(g_dvrTest->streamPath[pathIndex].dataInjectionService);
	recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings >>>");
	BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
	recordServiceSettings.parserBand  = NEXUS_ANY_ID;
	recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
	recordServiceSettings.transcodeService = (void *)g_dvrTest->streamPath[pathIndex].transcodeService;
	B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings <<<");

	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start >>>");

	/* start transcode */
	B_DVR_TranscodeService_Start(g_dvrTest->streamPath[pathIndex].transcodeService);
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start <<<");

	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid >>>");
	BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
	recordInputEsStream.esStreamInfo.pid = media.esStreamInfo[0].pid;
	recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
	recordInputEsStream.esStreamInfo.codec.videoCodec = NEXUS_VideoCodec_eH264;
	transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
																media.esStreamInfo[0].pid);
	if(!transcodePidChannel)
	{
		printf("\n error in getting the transcode pid Channel for %u",media.esStreamInfo[0].pid);
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
	printf("\n BKNI_Memset <<<");
	recordInputEsStream.esStreamInfo.pid = media.esStreamInfo[1].pid;
	recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
	recordInputEsStream.esStreamInfo.codec.audioCodec = NEXUS_AudioCodec_eAac;
	printf("\n B_DVR_TranscodeService_GetPidChannel >>>");
	transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
																media.esStreamInfo[1].pid);
	if(!transcodePidChannel)
	{
		printf("\n error in getting the transcode pid Channel for *%u",/*playbackMedia.esStreamInfo[1].pid*/media.esStreamInfo[1].pid);
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
	recordInputEsStream.esStreamInfo.pid = media.esStreamInfo[1].pid+1;
	recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
	transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
																media.esStreamInfo[1].pid+1);
	if(!transcodePidChannel)
	{
		printf("\n error in getting the transcode pid Channel for *%u",/*playbackMedia.esStreamInfo[1].pid+1*/media.esStreamInfo[1].pid+1);
		goto error_transcodePidChannel;
	}
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid <<<");
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid >>>");
	recordInputEsStream.pidChannel=transcodePidChannel;

	/*Add transcoded audio ES to the record*/
	B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid <<<");
	
	/* start recording */
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start >>>");
	B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
	printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start <<<");
	printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_PlaybackDecodeStart >>>");

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
	return rc;
}


B_DVR_ERROR B_DvrTestFileToFileTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;

    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings >>>");
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings <<<");
	#if 1
	if (g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
	{
		printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop >>>");
	    DvrTestPlaybackDecodeStop(pathIndex);
	    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_PlaybackDecodeStop <<<");
	}
	#endif
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
	if (g_dvrTest->streamPath[pathIndex].recordService)
	{		
	    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop >>>");
	    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
	    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Stop <<<");
		printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback >>>");
	    B_DVR_RecordService_RemoveCallback(g_dvrTest->streamPath[pathIndex].recordService);
	    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_RemoveCallback <<<");
	    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close >>>");
	    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
	    printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_RecordService_Close <<<");
	}
    /* B_DVR_RecordService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].recordService);*/

	printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_DataInjectionService_Close>>>");
	B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService); 
	printf("\n DvrTest_FileToFileTranscodeStop : B_DVR_DataInjectionService_Close >>>");
    g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;

	
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
    B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
	   DvrTestLiveDecodeStart(pathIndex);
    }
    printf("\n DvrTest_FileToFileTranscodeStop : DvrTest_LiveDecodeStart <<<");
    return rc;
}


void DvrTestComplex (CuTest * tc)
{
	int loopCount = 0;
	int index;
	char programName[B_DVR_MAX_FILE_NAME_LENGTH] = "TSB";
	B_DVR_OperationSettings operationSettings;
	B_DVR_TSBServiceStatus tsbServiceStatus;
	DvrTest_Operation_Param param;
	char subDir[B_DVR_MAX_FILE_NAME_LENGTH] = "tsbConv";
	char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH];
	unsigned recordingCount;
	unsigned esStreamIndex;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
	B_DVR_ERROR rc = B_DVR_SUCCESS;	
	unsigned recordVideoCodec, recordAudioCodec;
	char * subDirList[3] = {"tsbConv", "bgrec", "transcode"};
	int subDirIndex;


	g_dvrTest->maxChannels = MAX_CHANNELS;
    g_dvrTest->streamPath[0].currentChannel = 0;
	memset(&param, 0, sizeof(param));

	TSBServiceReady = B_Event_Create(NULL);
    TSBConversionComplete= B_Event_Create(NULL);
    endOfStreamEvent = B_Event_Create(NULL);
    TranscodeServiceReady = B_Event_Create(NULL);
		
	g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)B_DvrTestCallback;
	B_DvrTestTsbBufferPreallocate(NUM_TSB_PATH, programName);

	
	/* Run infinite loop for stress test purpose */
 	while (loopCount < NUM_LOOPS)
	{
		printf("\n\nNUMBER OF LOOPS %d \n\n", loopCount);
		loopCount++;
		printf("\nStart TSB service \n");
		for (index = 0; index <NUM_TSB_PATH; index++)
		{
			if(!g_dvrTest->streamPath[index].tsbService)
			{
				printf("\nTSB Service for the path index %d is not started\n", index);
				DvrTestTsbServiceStart(index, programName);
				printf("Starting TSB Service for the path index %d...\n", index);
			}
			else
			{
				printf("\nTSB Service for the path index %d is already started\n", index);
			}
		}
		while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
		BDBG_MSG(("\nAll %d TSB services are ready\n", index));
		
		/* Start Data injection in index 0 */
		printf("\nStart DvrTestDataInjectionStart on program %s\n", programName);
		DvrTestDataInjectionStart(0);

		/* Perform TSB playback */
		
		if(g_dvrTest->streamPath[0].tsbService) 
		{			
		    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[0].tsbService,&tsbServiceStatus);
		    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
		    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
		    if(g_dvrTest->streamPath[0].liveDecodeStarted)
		    {
		        DvrTestLiveDecodeStop(0);
		    }
		    DvrTestPlaybackDecodeStart(0);
		    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
		    operationSettings.operation = eB_DVR_OperationPause;
			printf("\n#### eB_DVR_OperationPause ###\n");
		    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
			operationSettings.operation = eB_DVR_OperationFastForward;
			operationSettings.operationSpeed = 2;
			printf("\n\n### TSB play in fast forward in 2x speed ###\n\n");
            B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
			sleep(1);
			operationSettings.operation = eB_DVR_OperationPlay;
			printf("\n\n### TSB play in normal speed ###\n\n");
			B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[0].tsbService,&operationSettings);
		}

		/* Stop Data injection */
		DvrTestDataInjectionStop(0);
		
		/* Start TSB Conversion Service */
		for (index=0; index < NUM_TSB_CONVERSION ; index++)
		{
		    if(g_dvrTest->streamPath[index].tsbService)
		    {
		        /*B_DVR_TSBServiceStatus tsbStatus;*/
		        B_DVR_TSBServicePermanentRecordingRequest recReq;

		        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbServiceStatus);
				BKNI_Snprintf(recReq.programName, sizeof(recReq.programName), "%s%d",programName,index);
		        recReq.recStartTime = tsbServiceStatus.tsbRecStartTime;
		        recReq.recEndTime = recReq.recStartTime + DURATION;
		        printf("\n \t program name %s \t start %lu \t end %lu \n",recReq.programName,recReq.recStartTime,recReq.recEndTime);
				BKNI_Snprintf(recReq.subDir, sizeof(recReq.subDir), "%s", subDir);
		        rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
				if(rc!=B_DVR_SUCCESS)
		        {
		            printf("\n Invalid paramters");
		        }
		    }
		}
			
	
		/* Display recording list */
		mediaNodeSettings.subDir = &subDir[0];
	    mediaNodeSettings.programName = NULL;
	    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
	    mediaNodeSettings.volumeIndex = 0;
	    recordingCount = B_DVR_Manager_GetNumberOfRecordings(g_dvrTest->dvrManager,&mediaNodeSettings);
	    printf("\n number of recordings %u",recordingCount);
		recordingList = BKNI_Malloc(sizeof(char *)*recordingCount*B_DVR_MAX_FILE_NAME_LENGTH);
	    B_DVR_Manager_GetRecordingList(g_dvrTest->dvrManager,&mediaNodeSettings,recordingCount,recordingList);
		if(recordingCount)
	    {			
	        printf("\n********************************************************************");
	        printf("\n                       Recording List                               ");
	        printf("\n********************************************************************");
	        for(index=0;(unsigned)index < recordingCount;index++)
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
					recordVideoCodec = media.esStreamInfo[esStreamIndex].codec.videoCodec;
					printf("\n Video Codec: %d\n", recordVideoCodec);
					recordAudioCodec = media.esStreamInfo[esStreamIndex].codec.audioCodec;
					printf("\n Audio Codec: %d\n", recordAudioCodec);
	            }
	        }
	    }
	    else
	    {
	        printf("\n no recordings found");
	    }

	    /* wait until TSB Conversion is completed */
		printf("\n### Wait for TSBConversionComplete ### \n");
	    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
	    B_Event_Reset(TSBConversionComplete);
		

		/*Stop TSB conversion */
		for (index = 0; index < 2/*NUM_TSB_CONVERSION*/; index++)
		{	
			if (g_dvrTest->streamPath[index].tsbService)
			{
				printf("\nStop TSB conversion on index %d ", index);
				B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[index].tsbService);
				DvrTestTsbServiceStop(index);
				if(!g_dvrTest->streamPath[index].liveDecodeStarted)
		        {
		            DvrTestLiveDecodeStart(index);
		        }
			}
			else
				printf("\nTSB service is not started in index %d", index);
		}

		
		/* Perform channel change */
		for (index = 0; index < NUM_TSB_PATH; index++)
		{
			printf("\nNumber of performing channel change %d \n", index);
			B_DvrTestChannelChange(tc, index);
			sleep(5);
		}

		BKNI_Snprintf(param.subDir, sizeof(param.subDir), "%s", "bgrec");
		BKNI_Snprintf(param.programName, sizeof(param.programName),"%s", "3HourRecord");
		mediaNodeSettings.subDir = param.subDir;
		mediaNodeSettings.programName = param.programName;
		mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
		mediaNodeSettings.volumeIndex = 0;
		
		/*DvrTestRecordServiceStart(tc, &param);*/
		B_DvrTestRecordServiceStart(1, param.programName);
		printf("\nWait util the recording's size hit 20MB");
		do {		
			B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
		}while (((unsigned)(media.mediaLinearEndOffset-media.mediaLinearStartOffset)) <= 1024 *1024 * 20);

		/* Start playback the current record */
		BDBG_MSG(("\nStart playback the current recording"));
		BDBG_MSG(("\nProgram name to playback: %s", param.programName));
		param.pathIndex = 0;
		Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStart, &param);

		/* Start transcoding the current record */
		printf("\nStart transcoding file %s in directory %s\n", param.programName, param.subDir);
		B_DvrTestFileToFileTranscode(0, NEXUS_VideoCodec_eH264, NEXUS_AudioCodec_eAac, param.subDir/*"bgrec"*/, param.programName/*"3HourRecord"*/);

		do{
			B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
		}while((media.mediaEndTime - media.mediaStartTime) <= DURATION_OF_RECORD);

		/*Stop recording*/
		printf("\nStop recording");
		B_DvrTestRecordServiceStop(1);

		/*Stop transcoding */
		printf("\nStop transcoding ");
		B_DvrTestFileToFileTranscodeStop(0);

		printf("\nStop playback in index %d ", param.pathIndex);
		Do_DvrTest_Operation(tc, eDvrTest_OperationPlaybackServiceStop, &param);
		
		/*Deleting recordings */		
		for (subDirIndex=0;subDirIndex<3; subDirIndex++)
		{
			mediaNodeSettings.subDir = subDirList[subDirIndex];
			if (strcmp(mediaNodeSettings.subDir, "tsbConv") == 0)
			{
				for(index=0;(unsigned)index < recordingCount;index++)
				{
					mediaNodeSettings.programName = recordingList[index];
					rc = B_DVR_Manager_DeleteRecording(g_dvrTest->dvrManager, &mediaNodeSettings);
					printf("\n### Deleting media file %s in directory %s ####\n", recordingList[index], mediaNodeSettings.subDir);
				}
			}
			else
			{
				mediaNodeSettings.programName = "3HourRecord";
			    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
				B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager,&mediaNodeSettings,&media);
				printf("\nDelete file %s in directory %s \n", mediaNodeSettings.programName, mediaNodeSettings.subDir);
				rc = B_DVR_Manager_DeleteRecording(g_dvrTest->dvrManager, &mediaNodeSettings);
			}
			if (rc != B_DVR_SUCCESS)
			{
				printf("\nCan't not delete file, please check file name, directory, ...\n");
			}
		}
		BKNI_Free(recordingList);		
	}
	DvrTestTsbBufferDeallocate(NUM_TSB_CONVERSION);
}
