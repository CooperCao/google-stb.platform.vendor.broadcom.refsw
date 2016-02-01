/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 *  1. This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/


#include "dvr_test.h"
/*
#include "msutil.h"
#include "msdiag.h"
#include "tshdrbuilder.h"
*/

BDBG_MODULE(dvr_test_transcodeservice);

#define MAX_AVPATH 				2
#define DURATION     			60000        /* 1 minute */
#define NUM_TSB_PATH 			8
#define NUM_TSB_CONVERSION		1
/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)
#define TRANSCODE0_STC_CHANNEL_INDEX    4
#define TRANSCODE1_STC_CHANNEL_INDEX    5

uint8_t pat[BTST_TS_HEADER_BUF_LENGTH];
uint8_t pmt[BTST_TS_HEADER_BUF_LENGTH];
unsigned patCcValue;
unsigned pmtCcValue;
unsigned dataInsertionCount;

unsigned int currentLiveChannel = 1;
B_EventHandle TSBServiceReady;
B_EventHandle TSBConversionComplete;
B_EventHandle outOfMediaStorageEvent;
B_EventHandle endOfStreamEvent;
B_EventHandle endOfStreamEvent;
B_EventHandle TranscodeServiceReady;

unsigned int num_OfTSBServiceReady = 0;
unsigned int num_OfTSBConversionComplete = 0;
unsigned long currentEndTime;


char VideoCodecName[8], AudioCodecName[8];
/* list of supported video codecs */
unsigned videoCodecArray[2] = {NEXUS_VideoCodec_eMpeg2, NEXUS_VideoCodec_eH264};
/* list of supported video codecs */
unsigned audioCodecArray[6] = {NEXUS_AudioCodec_eMp3, NEXUS_AudioCodec_eAac, 
                                                NEXUS_AudioCodec_eAacLoas, NEXUS_AudioCodec_eAacPlus,
                                                NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioCodec_eAc3};
char * audioCodecTranscodeList[9] =
{
    "NEXUS_AudioCodec_eMp3",            /* MPEG1/2, layer 3. */
    "NEXUS_AudioCodec_eAac",            /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAacAdts",    /* Advanced audio coding with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAacLoas",        /* Advanced audio coding with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlus",        /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlusLoas",    /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with LOAS (Low Overhead Audio Stream) sync and LATM mux */
    "NEXUS_AudioCodec_eAacPlusAdts",    /* AAC plus SBR. aka MPEG-4 High Efficiency (AAC-HE), with ADTS (Audio Data Transport Format) sync */
    "NEXUS_AudioCodec_eAc3",            /* Dolby Digital AC3 audio */
    "NEXUS_AudioCodec_eAc3Plus",        /* Dolby Digital Plus (AC3+ or DDP) audio */
};

char* videoCodecTranscodeList[16] =
{
	"NEXUS_VideoCodec_eMpeg2",
	"NEXUS_VideoCodec_eMpeg4Part2",
	"NEXUS_VideoCodec_eH263",
	"NEXUS_VideoCodec_eH264",
	"NEXUS_VideoCodec_eH264_Svc",
	"NEXUS_VideoCodec_eH264_Mvc",
	"NEXUS_VideoCodec_eVc1",
	"NEXUS_VideoCodec_eVc1SimpleMain",
	"NEXUS_VideoCodec_eDivx311",
	"NEXUS_VideoCodec_eAvs",
	"NEXUS_VideoCodec_eRv40",
	"NEXUS_VideoCodec_eVp6",
	"NEXUS_VideoCodec_eVp7",
	"NEXUS_VideoCodec_eVp8",
	"NEXUS_VideoCodec_eSpark",
	"NEXUS_VideoCodec_eMax"
};


extern void DvrTestLiveDecodeStart(int index);
extern void DvrTestLiveDecodeStop(int index);
extern void DvrTestPlaybackDecodeStart(int index);
extern void DvrTestPlaybackDecodeStop(int index);
extern void DvrTestTsbBufferPreallocate(int numOfTsb);
extern void DvrTestTsbBufferDeallocate(int numOfTsb);
extern void DvrTestTsbServiceStartOperation(int pathIndex);
extern void DvrTestTsbServiceStopOperation(int pathIndex);
extern void DvrTestTuneChannel(int index);
extern NEXUS_VideoCodec B_DVR_PlaybackService_P_GetVideoCodec(B_DVR_PlaybackServiceHandle playbackService);

void B_DvrTestPlaybackDecodeStop(int index)
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
                ++num_OfTSBServiceReady;
				printf("\nNumber of TSB ready %d \n", num_OfTSBServiceReady);
                /*if (num_OfTSBServiceReady == NUM_TSB_CONVERSION){*/
                B_Event_Set(TSBServiceReady);
				/*	printf("Set num_OfTSBServiceReady = 0\n");
					num_OfTSBServiceReady = 0;}*/
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
            ++num_OfTSBConversionComplete;
             if (num_OfTSBConversionComplete == NUM_TSB_CONVERSION)
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

void DvrTest_Create_PatPmt(
    int pathIndex,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    NEXUS_VideoCodec videoCodec,
    NEXUS_AudioCodec audioCodec
)
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
            case NEXUS_AudioCodec_eAac    :      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlus:      audStreamType = 0x11; break;/* LOAS */
            /* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0x11; break;
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


void B_DvrTestTsbBufferPreallocate(int numOfTsb, char *tsbProgramName)
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",tsbProgramName,index);
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",tsbProgramName,index);
        rc = B_DVR_Manager_PrintSegmentedFileRecord(g_dvrTest->dvrManager,
                                                    &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in printing the tsb segments");
        }
    }
}
void B_DvrTestTsbBufferDeallocate(int numOfTsb, char *tsbProgramName)
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
        BKNI_Snprintf(programName,sizeof(programName),"%s%d",tsbProgramName,index);
        rc = B_DVR_Manager_FreeSegmentedFileRecord(g_dvrTest->dvrManager,
                                                   &mediaNodeSettings);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n error in freeing the tsb segments");
        }
    }
}

void B_DvrTestTsbServiceStartOperation(int pathIndex, char *programName)
{
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
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
    inputEsStream.esStreamInfo.pid = channelInfo[currentLiveChannel].videoPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo[currentLiveChannel].videoFormat[0];
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService, &inputEsStream);

    inputEsStream.esStreamInfo.pid = channelInfo[currentLiveChannel].audioPids[0];
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentLiveChannel].audioFormat[0];
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

B_DVR_ERROR B_DvrTestFileToFileTranscodeStart(unsigned pathIndex, unsigned VideoCodec, 
unsigned AudioCodec, char *subDir, char *programName)
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
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;

    printf("\n\n--------------------------------------\n");
    printf("Starting B_DvrTestFileToFileTranscodeStart\n");
    printf("--------------------------------------\n");

    /* stop live decode*/
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex <2)
    {
       	DvrTestLiveDecodeStop(pathIndex);
    }

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTestLiveDecodeStop <<<"));
    BKNI_Memset(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].playbackServiceRequest));


    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir, subDir);
    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, programName);
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;


    /* open playback service */
    printf("B_DvrTestFileToFileTranscodeStart: Open Playback Service\n");
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].playbackService)
    {
        printf("\n playback service open failed for program %s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
        goto error_playbackService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Open <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<"));
    B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_InstallCallback <<<"));
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(B_DVR_TranscodeServiceRequest));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : NEXUS_StcChannel_Open <<<"));
    NEXUS_StcChannel_GetDefaultSettings(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    stcSettings.timebase = pathIndex?NEXUS_Timebase_e1: NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */

    /* open transcode stc channel */
    printf("B_DvrTestFileToFileTranscodeStart: Open Transcode STC Channel\n");
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
    printf("B_DvrTestFileToFileTranscodeStart: Open Transcode Service\n");
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
    transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;
    transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    transcodeInputSettings.hdmiInput = NULL;
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings >>>"));
    B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode >>>"));
    B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[pathIndex].playbackService,&playbackMedia);
    BDBG_MSG(("\n playback vPid %u aPid %u",playbackMedia.esStreamInfo[0].pid,playbackMedia.esStreamInfo[1].pid));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid>>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&playbackMedia.esStreamInfo[0],sizeof(playbackMedia.esStreamInfo[0]));
    transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    transcodeInputEsStream.videoEncodeParams.codec = VideoCodec;
    transcodeInputEsStream.videoEncodeParams.interlaced = false;
    transcodeInputEsStream.videoEncodeParams.profile = NEXUS_VideoCodecProfile_eBaseline;
    transcodeInputEsStream.videoEncodeParams.videoDecoder = g_dvrTest->streamPath[pathIndex].videoDecoder;
    transcodeInputEsStream.videoEncodeParams.level = NEXUS_VideoCodecLevel_e31;

    /* add video ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Video ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.audioEncodeParams.audioDecoder = g_dvrTest->streamPath[pathIndex].audioDecoder;
    transcodeInputEsStream.audioEncodeParams.audioPassThrough = false;
    transcodeInputEsStream.audioEncodeParams.codec = AudioCodec;
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
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&playbackMedia.esStreamInfo[1],sizeof(playbackMedia.esStreamInfo[1]));

    /* add audio ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Audio ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodeInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;

    /* add pcr ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add PCR ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings >>>"));
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings >>>"));
    transcodeServiceSettings.displaySettings.format = NEXUS_VideoFormat_e1080i;
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
    transcodeServiceSettings.videoEncodeSettings.frameRate = NEXUS_VideoFrameRate_e29_97;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesB = 0;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesP = 23;
	/* Trackinput has been removed from NEXUS_VideoEncoderStreamStructure */
    /*transcodeServiceSettings.videoEncodeSettings.streamStructure.trackInput = false;*/
    transcodeServiceSettings.videoEncodeSettings.enableFieldPairing = true;
    
    B_DVR_TranscodeService_SetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName,
                  B_DVR_MAX_FILE_NAME_LENGTH,"%s_%s%s",
                  g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName,
                  VideoCodecName, AudioCodecName);
    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"transcode");
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.segmented = true;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = B_DVR_RecordServiceInputTranscode;


    DvrTest_Create_PatPmt(pathIndex,
                          playbackMedia.esStreamInfo[1].pid +1,
                          playbackMedia.esStreamInfo[0].pid, 
                          playbackMedia.esStreamInfo[1].pid,
                          VideoCodec,
                          AudioCodec);

    /*Open record service*/
    printf("B_DvrTestFileToFileTranscodeStart: Open Record Service\n");
    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].recordService)
    {
        printf("record service open for transcode operation failed");
        goto error_recordService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Open <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback >>>"));
    B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_InstallCallback <<<"));

    dataInjectionOpenSettings.fifoSize = 30*188;
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    assert(g_dvrTest->streamPath[pathIndex].dataInjectionService);
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings >>>"));
    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.parserBand  = NEXUS_ANY_ID;
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    recordServiceSettings.transcodeService = (void *)g_dvrTest->streamPath[pathIndex].transcodeService;
    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_SetSettings <<<"));


    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start >>>"));

    /* start transcode */
    printf("B_DvrTestFileToFileTranscodeStart: Start Transcoding\n");
    rc = B_DVR_TranscodeService_Start(g_dvrTest->streamPath[pathIndex].transcodeService);
    if (rc != B_DVR_SUCCESS)
    {
        printf("Error in Transcode Service Start\n");
        goto error_transcodeService;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_Start <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[0].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    recordInputEsStream.esStreamInfo.codec.videoCodec = VideoCodec;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[0].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[0].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded video ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded Video ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    recordInputEsStream.esStreamInfo.codec.audioCodec = AudioCodec;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel aPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded audio ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded Audio ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream aPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid >>>"));
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
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetPidChannel pcrPid <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded PCR ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded PCR ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start >>>"));

    /* start recording */
    printf("B_DvrTestFileToFileTranscodeStart: Start recording\n");
    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_RecordService_Start <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTestPlaybackDecodeStart >>>"));

    /* start playback decode */
    DvrTestPlaybackDecodeStart(pathIndex);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : DvrTestPlaybackDecodeStart <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start >>>"));

    /* start playback */
    printf("B_DvrTestFileToFileTranscodeStart: Start playback\n");
    B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_Start <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeFinishEvent = NULL;
    BKNI_CreateEvent(&g_dvrTest->streamPath[pathIndex].transcodeFinishEvent);
    if(!g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
    {
        printf("\n transcodeFinishEvent create failed");
        goto error_transcodeFinishEvent;
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStart : BKNI_CreateEvent <<<"));
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



B_DVR_ERROR DvrTestPlaybackInProgressTranscodeStart(unsigned pathIndex, unsigned VideoCodec, 
unsigned AudioCodec, char *subDir, char *programName)
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
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
	NEXUS_VideoCodec inputVideoCodec;

    printf("\n\n--------------------------------------\n");
    printf("B_DvrTestPlaybackInProgressTranscode\n");
    printf("--------------------------------------\n");

    /* stop live decode*/
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex <2)
    {
       	DvrTestLiveDecodeStop(pathIndex);
    }

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : DvrTestLiveDecodeStop <<<"));
    BKNI_Memset(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].playbackServiceRequest));


    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir, subDir);
    sprintf(g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName, programName);
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;


    /* open playback service */
    printf("B_DvrTestFileToFileTranscodeStart: Open Playback Service\n");
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].playbackService)
    {
        printf("\n playback service open failed for program %s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
        goto error_playbackService;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_Open <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_InstallCallback <<<"));
    B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_InstallCallback <<<"));
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(B_DVR_TranscodeServiceRequest));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : NEXUS_StcChannel_Open <<<"));
    NEXUS_StcChannel_GetDefaultSettings(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    stcSettings.timebase = pathIndex?NEXUS_Timebase_e1: NEXUS_Timebase_e0;/* should be the same timebase for end-to-end locking */
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */

    /* open transcode stc channel */
    printf("B_DvrTestFileToFileTranscodeStart: Open Transcode STC Channel\n");
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel = NEXUS_StcChannel_Open(pathIndex?TRANSCODE1_STC_CHANNEL_INDEX:TRANSCODE0_STC_CHANNEL_INDEX, &stcSettings);
    if(!g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel)
    {
        printf("trancodeStcChannel open failed");
        goto error_transcodeStcChannel;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : NEXUS_StcChannel_Open <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.displayIndex = 3;
    g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeType = eB_DVR_TranscodeServiceType_NonRealTime;

    /* open transcode service */
    printf("B_DvrTestFileToFileTranscodeStart: Open Transcode Service\n");
    g_dvrTest->streamPath[pathIndex].transcodeService = B_DVR_TranscodeService_Open(&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].transcodeService)
    {
        printf("\n transcode service open failed");
        goto error_transcodeService;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_Open <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_InstallCallback >>>"));
    B_DVR_TranscodeService_InstallCallback(g_dvrTest->streamPath[pathIndex].transcodeService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_InstallCallback <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetInputSettings >>>"));
    B_DVR_TranscodeService_GetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    transcodeInputSettings.input = eB_DVR_TranscodeServiceInput_File;
    transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;
    transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    transcodeInputSettings.hdmiInput = NULL;
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetInputSettings <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_SetInputSettings >>>"));
    B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_SetInputSettings <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_GetMediaNode >>>"));
    B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[pathIndex].playbackService,&playbackMedia);
    BDBG_MSG(("\n playback vPid %u aPid %u",playbackMedia.esStreamInfo[0].pid,playbackMedia.esStreamInfo[1].pid));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_GetMediaNode <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream vPid>>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&playbackMedia.esStreamInfo[0],sizeof(playbackMedia.esStreamInfo[0]));
    transcodeInputEsStream.stcChannel = g_dvrTest->streamPath[pathIndex].stcChannel;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    transcodeInputEsStream.videoEncodeParams.codec = VideoCodec;
    transcodeInputEsStream.videoEncodeParams.interlaced = false;
    transcodeInputEsStream.videoEncodeParams.profile = NEXUS_VideoCodecProfile_eBaseline;
    transcodeInputEsStream.videoEncodeParams.videoDecoder = g_dvrTest->streamPath[pathIndex].videoDecoder;
    transcodeInputEsStream.videoEncodeParams.level = NEXUS_VideoCodecLevel_e31;

    /* add video ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Video ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream aPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.audioEncodeParams.audioDecoder = g_dvrTest->streamPath[pathIndex].audioDecoder;
    transcodeInputEsStream.audioEncodeParams.audioPassThrough = false;
    transcodeInputEsStream.audioEncodeParams.codec = AudioCodec;
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
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&playbackMedia.esStreamInfo[1],sizeof(playbackMedia.esStreamInfo[1]));

    /* add audio ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Audio ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream aPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>"));
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodeInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid+1;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;

    /* add pcr ES stream*/
    printf("B_DvrTestFileToFileTranscodeStart: Add PCR ES stream\n");
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetSettings >>>"));
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetSettings <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_SetSettings >>>"));
    transcodeServiceSettings.displaySettings.format = NEXUS_VideoFormat_e1080i;
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
    transcodeServiceSettings.videoEncodeSettings.frameRate = NEXUS_VideoFrameRate_e29_97;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesB = 0;
    transcodeServiceSettings.videoEncodeSettings.streamStructure.framesP = 23;
    /*transcodeServiceSettings.videoEncodeSettings.streamStructure.trackInput = false;*/
    transcodeServiceSettings.videoEncodeSettings.enableFieldPairing = true;
    
    B_DVR_TranscodeService_SetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_SetSettings <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_Open >>>"));
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    BKNI_Snprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName,
                  B_DVR_MAX_FILE_NAME_LENGTH,"%s_%s%s",
                  g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName,
                  VideoCodecName, AudioCodecName);
    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"transcode");
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex = 0;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.segmented = true;
    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = B_DVR_RecordServiceInputTranscode;


    DvrTest_Create_PatPmt(pathIndex,
                          playbackMedia.esStreamInfo[1].pid +1,
                          playbackMedia.esStreamInfo[0].pid, 
                          playbackMedia.esStreamInfo[1].pid,
                          VideoCodec,
                          AudioCodec);

    /*Open record service*/
    printf("B_DvrTestFileToFileTranscodeStart: Open Record Service\n");
    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
    if(!g_dvrTest->streamPath[pathIndex].recordService)
    {
        printf("record service open for transcode operation failed");
        goto error_recordService;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_Open <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_InstallCallback >>>"));
    B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_InstallCallback <<<"));

    dataInjectionOpenSettings.fifoSize = 30*188;
    g_dvrTest->streamPath[pathIndex].dataInjectionService = B_DVR_DataInjectionService_Open(&dataInjectionOpenSettings);
    assert(g_dvrTest->streamPath[pathIndex].dataInjectionService);
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_SetSettings >>>"));
    BKNI_Memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.parserBand  = NEXUS_ANY_ID;
    recordServiceSettings.dataInjectionService = g_dvrTest->streamPath[pathIndex].dataInjectionService;
    recordServiceSettings.transcodeService = (void *)g_dvrTest->streamPath[pathIndex].transcodeService;
    B_DVR_RecordService_SetSettings(g_dvrTest->streamPath[pathIndex].recordService,&recordServiceSettings);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_SetSettings <<<"));


    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_Start >>>"));

    /* start transcode */
	inputVideoCodec = B_DVR_PlaybackService_P_GetVideoCodec(g_dvrTest->streamPath[0].playbackService);
	printf("\n\n Playback video Codec %s \n\n", videoCodecTranscodeList[inputVideoCodec]);
	printf("\n##########################################################\n");
	printf("Start Transcoding from audioCodec %s videoCodec %s to audioCodec %s videoCodec %s \n", 
				audioCodecArray[playbackMedia.esStreamInfo[1].codec.audioCodec], videoCodecTranscodeList[playbackMedia.esStreamInfo[0].codec.videoCodec],
				AudioCodecName,VideoCodecName);
	printf("\n##########################################################\n");
    rc = B_DVR_TranscodeService_Start(g_dvrTest->streamPath[pathIndex].transcodeService);
    if (rc != B_DVR_SUCCESS)
    {
        printf("Error in Transcode Service Start\n");
        goto error_transcodeService;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_Start <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel vPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[0].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    recordInputEsStream.esStreamInfo.codec.videoCodec = VideoCodec;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[0].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[0].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel vPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream vPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded video ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded Video ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream vPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel aPid >>>"));
    BKNI_Memset((void *)&recordInputEsStream,0,sizeof(recordInputEsStream));
    recordInputEsStream.esStreamInfo.pid = playbackMedia.esStreamInfo[1].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    recordInputEsStream.esStreamInfo.codec.audioCodec = AudioCodec;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMedia.esStreamInfo[1].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMedia.esStreamInfo[1].pid);
        goto error_transcodePidChannel;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel aPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream aPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded audio ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded Audio ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream aPid <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel pcrPid >>>"));
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
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_TranscodeService_GetPidChannel pcrPid <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream pcrPid >>>"));
    recordInputEsStream.pidChannel=transcodePidChannel;

    /*Add transcoded PCR ES to the record*/
    printf("B_DvrTestFileToFileTranscodeStart: Add Transcoded PCR ES to the record\n");
    B_DVR_RecordService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].recordService,&recordInputEsStream);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_AddInputEsStream pcrPid <<<"));

    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_Start >>>"));

    /* start recording */
    printf("B_DvrTestFileToFileTranscodeStart: Start recording\n");
    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_RecordService_Start <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : DvrTestPlaybackDecodeStart >>>"));

    /* start playback decode */
    DvrTestPlaybackDecodeStart(pathIndex);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : DvrTestPlaybackDecodeStart <<<"));
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_Start >>>"));

    /* start playback */
    printf("B_DvrTestFileToFileTranscodeStart: Start playback\n");
    B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : B_DVR_PlaybackService_Start <<<"));

	/* Start Playback the current trancoding file */
#if 1
	sleep(10);
	strcpy(g_dvrTest->streamPath[1].playbackServiceRequest.subDir, g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir);
	g_dvrTest->streamPath[1].playbackServiceRequest.playpumpIndex = 7;
    g_dvrTest->streamPath[1].playbackServiceRequest.volumeIndex = 0;
	
	printf("\n\n#### Playback directory %s ####\n\n", g_dvrTest->streamPath[1].playbackServiceRequest.subDir);
	strcpy(g_dvrTest->streamPath[1].playbackServiceRequest.programName, g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);
	printf("\n\n#### Playback stream %s ####\n\n", g_dvrTest->streamPath[1].playbackServiceRequest.programName);
    g_dvrTest->streamPath[1].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[1].playbackServiceRequest);
	B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[1].playbackService, g_dvrTest->dvrTestCallback, (void *)g_dvrTest);
    printf("\n B_DVR_PlaybackService_Open");
    if(g_dvrTest->streamPath[1].liveDecodeStarted)
    {
        DvrTestLiveDecodeStop(1);
		printf("\n## Live decode stopped ##\n");
    }
    DvrTestPlaybackDecodeStart(1);
    printf("\n DvrTest_PlaybackDecodeStart");
    B_DVR_PlaybackService_Start(g_dvrTest->streamPath[1].playbackService);

#endif
	BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : BKNI_CreateEvent >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeFinishEvent = NULL;
    BKNI_CreateEvent(&g_dvrTest->streamPath[pathIndex].transcodeFinishEvent);
    if(!g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
    {
        printf("\n transcodeFinishEvent create failed");
        goto error_transcodeFinishEvent;
    }
    BDBG_MSG(("\n B_DvrTestPlaybackInProgressTranscode : BKNI_CreateEvent <<<"));
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

B_DVR_ERROR DvrTestFileToFileTranscodeStop(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;
	int i;

    printf("\n\n-------------------------------------\n");
    printf("Starting DvrTestFileToFileTranscodeStop\n");
    printf("-------------------------------------\n");

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings >>>"));
	if(g_dvrTest->streamPath[pathIndex].transcodeService)
	{
		B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_GetSettings <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTestPlaybackDecodeStop >>>"));
	if (g_dvrTest->streamPath[pathIndex].playbackService)
	{
		B_DvrTestPlaybackDecodeStop(pathIndex);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTestPlaybackDecodeStop <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Stop Playback Service >>>"));
	if (g_dvrTest->streamPath[pathIndex].playbackService)
	{
		B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
	}
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Stop Playback Service <<<"));
	
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop >>>"));
    if (g_dvrTest->streamPath[pathIndex].transcodeService)
    {
    	B_DVR_TranscodeService_Stop(g_dvrTest->streamPath[pathIndex].transcodeService);
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Stop <<<"));

    /* wait for the encoder buffer model's data to be drained */
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : BKNI_WaitForEvent >>>"));
	if (g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
	{
	    if(BKNI_WaitForEvent(g_dvrTest->streamPath[pathIndex].transcodeFinishEvent,
	                         (transcodeServiceSettings.videoEncodeSettings.encoderDelay/27000)*2)!=BERR_SUCCESS) 
	    {
	        printf("\n Transcode Finish timeout");
	    }
	}

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Stop and Close Record Service >>>"));
    if (g_dvrTest->streamPath[pathIndex].recordService)
    {
	    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
		B_DVR_RecordService_RemoveCallback(g_dvrTest->streamPath[pathIndex].recordService);
		B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Record Service Closed <<<"));

	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Playback Service Closed >>>"));
	if (g_dvrTest->streamPath[pathIndex].playbackService)
	{
		B_DVR_PlaybackService_RemoveCallback(g_dvrTest->streamPath[pathIndex].playbackService);
		B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
	}
	BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : Playback Service Closed <<<"));
	
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback >>>"));
	if(g_dvrTest->streamPath[pathIndex].transcodeService)
	{
    	B_DVR_TranscodeService_RemoveCallback(g_dvrTest->streamPath[pathIndex].transcodeService);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveCallback <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams >>>"));
	if(g_dvrTest->streamPath[pathIndex].transcodeService)
	{
		B_DVR_TranscodeService_RemoveAllInputEsStreams(g_dvrTest->streamPath[pathIndex].transcodeService);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_RemoveAllInputEsStreams <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close >>>"));
	if(g_dvrTest->streamPath[pathIndex].transcodeService)
	{
		B_DVR_TranscodeService_Close(g_dvrTest->streamPath[pathIndex].transcodeService);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : B_DVR_TranscodeService_Close <<<"));

    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close >>>"));
	if (g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel)
	{
		NEXUS_StcChannel_Close(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest.transcodeStcChannel);
	}
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : NEXUS_StcChannel_Close <<<"));
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTestLiveDecodeStart >>>"));
    g_dvrTest->streamPath[pathIndex].transcodeService = NULL;
    BKNI_Memset((void*)&g_dvrTest->streamPath[pathIndex].transcodeServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].transcodeServiceRequest));

    printf("DvrTestFileToFileTranscodeStop: Start Live Decode\n");
    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTestLiveDecodeStart(pathIndex);
    }
    BDBG_MSG(("\n DvrTest_FileToFileTranscodeStop : DvrTestLiveDecodeStart <<<"));
	if (g_dvrTest->streamPath[pathIndex].transcodeFinishEvent)
	{
		BKNI_DestroyEvent(g_dvrTest->streamPath[pathIndex].transcodeFinishEvent);
	}

	B_Event_Set(TranscodeServiceReady);

    return rc;
}

void Dvr_Get_VideoCodec_Name(unsigned vCodecType)
{

    switch (vCodecType)
    {
        case NEXUS_VideoCodec_eMpeg1:
            printf("MPEG1");
        break;
        case NEXUS_VideoCodec_eMpeg2:
            printf("MPEG2");
        break;
        case NEXUS_VideoCodec_eMpeg4Part2:
            printf("MPEG4Part2");
        break;
        case NEXUS_VideoCodec_eH263:
            printf("H263");
        break;
        case NEXUS_VideoCodec_eH264:
            printf("H264");
        break;
        case NEXUS_VideoCodec_eH264_Svc:
            printf("H264SVC");
        break;
        case NEXUS_VideoCodec_eH264_Mvc:
            printf("H264MVC");
        break;
        case NEXUS_VideoCodec_eVc1:
            printf("VC1");
        break;
        case NEXUS_VideoCodec_eVc1SimpleMain:
            printf("VC1SimpleMain");
        break;
        case NEXUS_VideoCodec_eDivx311:
            printf("DIVX311");
        break;
        case NEXUS_VideoCodec_eAvs:
            printf("AVS");
        break;
        default:
        break;
    }
}

void Dvr_Get_AudioCodec_Name(unsigned aCodecType)
{

    switch (aCodecType)
    {
        case NEXUS_AudioCodec_eMpeg:
            printf("MPEG");
        break;
        case NEXUS_AudioCodec_eMp3:
            printf("MP3");
        break;
        case NEXUS_AudioCodec_eAac:
            printf("AAC");
        break;
#if 0
        case NEXUS_AudioCodec_eAacAdts:
            printf("AAC");
        break;
#endif
        case NEXUS_AudioCodec_eAacLoas:
            printf("AAC LOAS");
        break;
        case NEXUS_AudioCodec_eAacPlus:
            printf("AAC+");
        break;
#if 0
        case NEXUS_AudioCodec_eAacPlusLoas:
            printf("AAC+");
        break;
#endif
        case NEXUS_AudioCodec_eAacPlusAdts:
            printf("AAC+ ADTS");
        break;
        case NEXUS_AudioCodec_eAc3:
            printf("AC3");
        break;
        case NEXUS_AudioCodec_eAc3Plus:
            printf("AC3+");
        break;
        default:
        break;
    }

}


void Dvr_Set_VideoCodec_Name(unsigned vCodecType)
{

    switch (vCodecType)
    {
        case NEXUS_VideoCodec_eMpeg1:
            sprintf(VideoCodecName, "mpg1");
        break;
        case NEXUS_VideoCodec_eMpeg2:
            sprintf(VideoCodecName, "mpg2");
        break;
        case NEXUS_VideoCodec_eMpeg4Part2:
            sprintf(VideoCodecName, "mpg4");
        break;
        case NEXUS_VideoCodec_eH263:
            sprintf(VideoCodecName, "h263");
        break;
        case NEXUS_VideoCodec_eH264:
            sprintf(VideoCodecName, "h264");
        break;
        case NEXUS_VideoCodec_eH264_Svc:
            sprintf(VideoCodecName, "h264s");
        break;
        case NEXUS_VideoCodec_eH264_Mvc:
            sprintf(VideoCodecName, "h264m");;
        break;
        case NEXUS_VideoCodec_eVc1:
            sprintf(VideoCodecName, "vc1");
        break;
        case NEXUS_VideoCodec_eVc1SimpleMain:
            sprintf(VideoCodecName, "vc1s");
        break;
        case NEXUS_VideoCodec_eDivx311:
            sprintf(VideoCodecName, "divx");
        break;
        case NEXUS_VideoCodec_eAvs:
            sprintf(VideoCodecName, "avs");;
        break;
        default:
        break;
    }
}

void Dvr_Set_AudioCodec_Name(unsigned aCodecType)
{

    switch (aCodecType)
    {
        case NEXUS_AudioCodec_eMpeg:
            sprintf(AudioCodecName, "mpg");
        break;
        case NEXUS_AudioCodec_eMp3:
            sprintf(AudioCodecName, "mp3");
        break;
        case NEXUS_AudioCodec_eAac:
            sprintf(AudioCodecName, "aac");
        break;
        case NEXUS_AudioCodec_eAacLoas:
            sprintf(AudioCodecName, "aacl");
        break;
        case NEXUS_AudioCodec_eAacPlus:
            sprintf(AudioCodecName, "aac+");
        break;
        case NEXUS_AudioCodec_eAacPlusAdts:
            sprintf(AudioCodecName, "aac+a");
        break;
        case NEXUS_AudioCodec_eAc3:
            sprintf(AudioCodecName, "ac3");
        break;
        case NEXUS_AudioCodec_eAc3Plus:
            sprintf(AudioCodecName, "ac3+");
        break;
        default:
        break;
    }

}


void DvrTestFileToFileTranscode (CuTest * tc)
{
    int index;
    int videoCodecIndex, audioCodecIndex;
    unsigned volumeIndex = 0;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned VideoCodec, AudioCodec;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH] = "tsbConv";
    char programName[B_DVR_MAX_FILE_NAME_LENGTH] = "TSBRec";

    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned esStreamIndex;
    unsigned VideoPID, AudioPID;
    unsigned record_videoCodec, record_audioCodec;

	BSTD_UNUSED(tc);
    g_dvrTest->maxChannels = MAX_STATIC_CHANNELS;
    g_dvrTest->streamPath[0].currentChannel = currentLiveChannel;
	/*DvrTestTuneChannel(0);*/

    TSBServiceReady = B_Event_Create(NULL);
    TSBConversionComplete= B_Event_Create(NULL);
    endOfStreamEvent = B_Event_Create(NULL);
    TranscodeServiceReady = B_Event_Create(NULL);

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTestCallback;
    DvrTestTsbBufferPreallocate(NUM_TSB_CONVERSION);

	/* Start TSB service */
    for (index = 0; index < NUM_TSB_CONVERSION; index++)
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

	/* Start TSB Conversion Service */
	for (index=0; index < NUM_TSB_CONVERSION ; index++)
	{
	    if(g_dvrTest->streamPath[index].tsbService)
	    {
	        B_DVR_ERROR rc = B_DVR_SUCCESS;
	        B_DVR_TSBServiceStatus tsbStatus;
	        B_DVR_TSBServicePermanentRecordingRequest recReq;

	        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
	        sprintf(recReq.programName, programName);
	        recReq.recStartTime = tsbStatus.tsbRecStartTime;
	        recReq.recEndTime = recReq.recStartTime + DURATION;
	        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
	        sprintf(recReq.subDir, subDir);
	        rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
	        if(rc!=B_DVR_SUCCESS)
	        {
	            printf("\n Invalid paramters");
	        }
	    }
	}

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_Event_Reset(TSBConversionComplete);
	B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[0].tsbService);
	DvrTestTsbServiceStopOperation(0);

    /* check the recording's video codec and audio codec to exclude them 
    from the selection of codecs for transcoding */

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volumeIndex;

    printf("\n*******************************\n");
    printf("  Information of a record\n");
    printf("*******************************\n");

    B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager, &mediaNodeSettings, &media);

    for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++)
    {
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
        {
            VideoPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Video PID: %d\n", VideoPID);
            record_videoCodec = media.esStreamInfo[esStreamIndex].codec.videoCodec;
            printf("Video Codec Type: %d (", record_videoCodec);
            Dvr_Get_VideoCodec_Name(record_videoCodec);
            printf(")\n");
        }
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeAudio)
        {
            AudioPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Audio PID: %d\n", AudioPID);
            record_audioCodec = media.esStreamInfo[esStreamIndex].codec.audioCodec;
            printf("Audio Codec Type: %d (", record_audioCodec);
            Dvr_Get_AudioCodec_Name(record_audioCodec);
            printf(")\n");
        }
    }
    printf("*******************************\n");


    B_Event_Set(TranscodeServiceReady);

    for (videoCodecIndex = 0; videoCodecIndex < 2; videoCodecIndex++)
    {
        for (audioCodecIndex = 0; audioCodecIndex < 1/*6*/; audioCodecIndex++)
        {
            VideoCodec = videoCodecArray[videoCodecIndex];
            AudioCodec = audioCodecArray[audioCodecIndex];
            /* filtering out video codec and audio codec of the record */
            if ((VideoCodec != record_videoCodec) && (AudioCodec != record_audioCodec))
            {
                printf ("Video Codec = %d Audio Codec = %d\n", VideoCodec, AudioCodec);
                Dvr_Set_VideoCodec_Name(VideoCodec);
                Dvr_Set_AudioCodec_Name(AudioCodec);
                printf ("Video Codec Name = %s Audio Codec Name = %s\n", VideoCodecName, AudioCodecName);

                while(B_Event_Wait(TranscodeServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
                B_Event_Reset(TranscodeServiceReady);
                rc = B_DvrTestFileToFileTranscodeStart(0, VideoCodec, AudioCodec, subDir, programName);
				printf("\n### Running B_DvrTestFileToFileTranscodeStart for audioCodecIndex %d ###\n", audioCodecIndex);
				if (rc == B_DVR_SUCCESS)
                {
                    /* wailt until the playback point is reached to the end */
                    while(B_Event_Wait(endOfStreamEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
                    B_Event_Reset(endOfStreamEvent);
                    DvrTestFileToFileTranscodeStop(0);
                }
                else
                    goto closing_services;
            }
        }
    }
	closing_services:
	DvrTestTsbBufferDeallocate(NUM_TSB_CONVERSION);
}

void DvrTestPlaybackInProgressTranscode (CuTest * tc)
{
    int index;
    int videoCodecIndex, audioCodecIndex = 0;
    unsigned volumeIndex = 0;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned VideoCodec, AudioCodec;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH] = "tsbConv";
    char programName[B_DVR_MAX_FILE_NAME_LENGTH] = "InProgressTranscode";

    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    unsigned esStreamIndex;
    unsigned VideoPID, AudioPID;
    unsigned record_videoCodec, record_audioCodec;

	BSTD_UNUSED(tc);
    g_dvrTest->maxChannels = MAX_STATIC_CHANNELS;
    g_dvrTest->streamPath[0].currentChannel = currentLiveChannel;
	/*DvrTestTuneChannel(0);*/

    TSBServiceReady = B_Event_Create(NULL);
    TSBConversionComplete= B_Event_Create(NULL);
    endOfStreamEvent = B_Event_Create(NULL);
    TranscodeServiceReady = B_Event_Create(NULL);

    g_dvrTest->dvrTestCallback = (B_DVR_ServiceCallback)DvrTestCallback;
    B_DvrTestTsbBufferPreallocate(NUM_TSB_CONVERSION, programName);

	/* Start TSB service */
    for (index = 0; index < NUM_TSB_CONVERSION; index++)
    {
        if(!g_dvrTest->streamPath[index].tsbService)
        {
            printf("\nTSB Service for the path index %d is not started\n", index);
            B_DvrTestTsbServiceStartOperation(index, programName);
            printf("Starting TSB Service for the path index %d...\n", index);
        }
    }

    /* wait until TSB Service becomes ready */
    while(B_Event_Wait(TSBServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);

	/* Start TSB Conversion Service */
	for (index=0; index < NUM_TSB_CONVERSION ; index++)
	{
	    if(g_dvrTest->streamPath[index].tsbService)
	    {
	        B_DVR_ERROR rc = B_DVR_SUCCESS;
	        B_DVR_TSBServiceStatus tsbStatus;
	        B_DVR_TSBServicePermanentRecordingRequest recReq;

	        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[index].tsbService, &tsbStatus);
	        sprintf(recReq.programName, programName);
			printf("\n\n#### recReq.programName %s #### \n", recReq.programName);
	        recReq.recStartTime = tsbStatus.tsbRecStartTime;
	        recReq.recEndTime = recReq.recStartTime + DURATION;
	        printf("\n \t program name %s \t start %lu \t end %lu",recReq.programName,recReq.recStartTime,recReq.recEndTime);
	        sprintf(recReq.subDir, subDir);
	        rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[index].tsbService,&recReq);
	        if(rc!=B_DVR_SUCCESS)
	        {
	            printf("\n Invalid paramters");
	        }
	    }
	}

    /* wait until TSB Conversion is completed */
    while(B_Event_Wait(TSBConversionComplete, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
    B_Event_Reset(TSBConversionComplete);
	B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[0].tsbService);
	DvrTestTsbServiceStopOperation(0);

    /* check the recording's video codec and audio codec to exclude them 
    from the selection of codecs for transcoding */

    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.recordingType = eB_DVR_RecordingPermanent;
    mediaNodeSettings.volumeIndex = volumeIndex;

    printf("\n*******************************\n");
    printf("  Information of a record\n");
    printf("*******************************\n");

    B_DVR_Manager_GetMediaNode(g_dvrTest->dvrManager, &mediaNodeSettings, &media);

    for(esStreamIndex=0;esStreamIndex < media.esStreamCount;esStreamIndex++)
    {
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
        {
            VideoPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Video PID: %d\n", VideoPID);
            record_videoCodec = media.esStreamInfo[esStreamIndex].codec.videoCodec;
            printf("Video Codec Type: %d (", record_videoCodec);
            Dvr_Get_VideoCodec_Name(record_videoCodec);
            printf(")\n");
        }
        if(media.esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeAudio)
        {
            AudioPID = media.esStreamInfo[esStreamIndex].pid;
            printf("Audio PID: %d\n", AudioPID);
            record_audioCodec = media.esStreamInfo[esStreamIndex].codec.audioCodec;
            printf("Audio Codec Type: %d (", record_audioCodec);
            Dvr_Get_AudioCodec_Name(record_audioCodec);
            printf(")\n");
        }
    }
    printf("*******************************\n");


    B_Event_Set(TranscodeServiceReady);

    for (videoCodecIndex = 0; videoCodecIndex < 2; videoCodecIndex++)
    {
            VideoCodec = videoCodecArray[videoCodecIndex];
            AudioCodec = audioCodecArray[audioCodecIndex];
            /* filtering out video codec and audio codec of the record */
            if ((VideoCodec != record_videoCodec)/* && (AudioCodec != record_audioCodec)*/)
            {
                printf ("Video Codec = %d Audio Codec = %d\n", VideoCodec, AudioCodec);
                Dvr_Set_VideoCodec_Name(VideoCodec);
                Dvr_Set_AudioCodec_Name(AudioCodec);
                printf ("Video Codec Name = %s Audio Codec Name = %s\n", VideoCodecName, AudioCodecName);

                while(B_Event_Wait(TranscodeServiceReady, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
                B_Event_Reset(TranscodeServiceReady);
				printf("\n### Running B_DvrTestFileToFileTranscodeStart for audioCodecIndex %d ###\n", audioCodecIndex);
                rc = DvrTestPlaybackInProgressTranscodeStart(0, VideoCodec, AudioCodec, subDir, programName);
				if (rc == B_DVR_SUCCESS)
                {
                    /* wailt until the playback point is reached to the end */
                    while(B_Event_Wait(endOfStreamEvent, B_WAIT_FOREVER)!=B_ERROR_SUCCESS);
					B_Event_Reset(endOfStreamEvent);
					DvrTestFileToFileTranscodeStop(0);					
                }
                else
                    CuFail(tc, "TRANSCODE SERVICE FAILED");
            }
    }
	B_DvrTestTsbBufferDeallocate(NUM_TSB_CONVERSION, programName);
}


