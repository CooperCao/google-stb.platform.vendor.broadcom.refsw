/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
#include "b_dvr_manager.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_mediafile.h"
#include "b_dvr_datainjectionservice.h"
#include "msutil.h"
#include "msdiag.h"
#include "tshdrbuilder.h"

BDBG_MODULE(dvr_test_v2);

#define MAX_STATIC_CHANNELS             7
#define MAX_PROGRAMS_PER_CHANNEL        6
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_AUDIO_STREAMS               4
#define MAX_VIDEO_STREAMS               4
#define MAX_STREAM_PATH                 (NEXUS_MAX_FRONTENDS-1)
#define MAX_AV_PATH                     2
#define TSB_SERVICE_PREFIX              "streamPath"
#define TSB_CONVERT_PREFIX              "TsbRec"
#define TRANSCODE_SERVICE_PREFIX      "transcoded"
#define MAX_TSB_BUFFERS                 8 /* keep buffering for 8 mins */
#define MAX_TSB_TIME                    5*60*1000 /*5mins*/
#define TRANSCODE0_STC_CHANNEL_INDEX    4
#define TRANSCODE1_STC_CHANNEL_INDEX    5
/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define BTST_TS_HEADER_BUF_LENGTH   189
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)
typedef struct DvrTest_ChannelInfo
{
    NEXUS_FrontendQamAnnex annex; /* To describe Annex A or Annex B format */
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolrate;
    unsigned int  frequency;
    char programTitle[MAX_PROGRAM_TITLE_LENGTH];
    int numAudioStreams;
    int numVideoStreams;
    unsigned int videoPids[MAX_VIDEO_STREAMS];
    unsigned int audioPids[MAX_AUDIO_STREAMS];
    unsigned int pcrPid[MAX_VIDEO_STREAMS];
    NEXUS_VideoCodec videoFormat[MAX_VIDEO_STREAMS];
    NEXUS_AudioCodec audioFormat[MAX_AUDIO_STREAMS];
} DvrTest_ChannelInfo;

static DvrTest_ChannelInfo channelInfo[MAX_STATIC_CHANNELS] =
{
    
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,429000000,"program1",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,435000000,"program2",1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,441000000,"program3", 1,1,{0x11},
    {0x14},{0x11},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}},
    {NEXUS_FrontendQamAnnex_eB,NEXUS_FrontendQamMode_e64,5056941,447000000,"program4",1,1,{0x21},
    {0x24},{0x21,},{NEXUS_VideoCodec_eMpeg2},{NEXUS_AudioCodec_eAc3}}
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
    #if NEXUS_HAS_STREAM_MUX
    B_DVR_TranscodeServiceHandle transcodeService;
    B_DVR_TranscodeServiceRequest transcodeServiceRequest;
    BKNI_EventHandle transcodeFinishEvent;
    unsigned transcodeOption;
    #endif
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
    NEXUS_PidChannelHandle dataInjectionPidChannel;
    bool liveDecodeStarted;
    bool playbackDecodeStarted;
    bool dataInjectionStarted;
    bool tsbConversionStarted;
    uint8_t pat[BTST_TS_HEADER_BUF_LENGTH];
    uint8_t pmt[BTST_TS_HEADER_BUF_LENGTH];
    unsigned patCcValue;
    unsigned pmtCcValue;
    unsigned psiCount;
    unsigned dataInsertionCount;
    B_SchedulerTimerId getStatus;
    B_SchedulerTimerId patMonitorTimer;
    B_SchedulerTimerId dataInjectTimer;
    B_DVR_MediaFileHandle patFile;
    void *patBuffer;
    off_t patSeekOffset;
    unsigned patSegmentCount;
    B_SchedulerTimerId streamingTimer;
    B_DVR_MediaFileHandle streamFile;
    void *streamBuf;
    void *streamBufAligned;
    B_SchedulerEventId tsbConvEventSchedulerID;
    B_EventHandle tsbConvEvent;
    B_DVR_TSBServicePermanentRecordingRequest permRecReq;
    B_MutexHandle streamPathMutex;
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
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    #endif
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    int maxChannels;
    B_DVR_ServiceCallback dvrTestCallback;
    void *buffer;
    B_DVR_MediaFileHandle mediaFile;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle thread;
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
    printf("\n \t \t DVRExtLib Operations");
    printf("\n ************************************************");
    printf("\n 1: change channel");
    printf("\n 2: start TSBService");
    printf("\n 3: stop  TSBService");
    printf("\n 4: start TSBPlayback in paused state");
    printf("\n 5: stop  TSBPlayback");
    printf("\n 6: start TSBService Convert");
    printf("\n 7: stop  TSBService Convert");
    printf("\n 8: pause TSBService/RecordService");
    printf("\n 9: resume TSBService/RecordService");
    printf("\n 10: start PlaybackService");
    printf("\n 11: stop  PlaybackService");
    printf("\n 12: start recordService");
    printf("\n 13: stop  recordService");
    printf("\n 14: start dataInjection");
    printf("\n 15: stop dataInjection");
    printf("\n 16: start TSB Streaming");
    printf("\n 17: stop TSB Streaming");
    printf("\n 18: start transcode service");
    printf("\n 19: stop transcode service");
    printf("\n 20. play");
    printf("\n 21. pause");
    printf("\n 22  rewind");
    printf("\n 23  fastforward");
    printf("\n 24. slow rewind");
    printf("\n 25. slow forward");
    printf("\n 26. forward frame advance");
    printf("\n 27: reverse frame advance");
    printf("\n 28: Seek Play");
    printf("\n 29: mediafile read");
    printf("\n 30: mediafile write");
    printf("\n 31: display recording list");
    printf("\n 32: delete a recording");
    printf("\n 33: print DVR memory usage");
    printf("\n 0: exit\n");
    printf("\n Select an operation: ");
    return;
}
typedef enum DvrTest_Operation
{
    eDvrTest_OperationQuit,
    eDvrTest_OperationChannelChange,
    eDvrTest_OperationTSBServiceStart,
    eDvrTest_OperationTSBServiceStop,
    eDvrTest_OperationTSBServicePlayStart,
    eDvrTest_OperationTSBServicePlayStop,
    eDvrTest_OperationTSBServiceConvertStart,
    eDvrTest_OperationTSBServiceConvertStop,
    eDvrTest_OperationRecordPause,
    eDvrTest_OperationRecordResume,
    eDvrTest_OperationPlaybackServiceStart,
    eDvrTest_OperationPlaybackServiceStop,
    eDvrTest_OperationRecordServiceStart,
    eDvrTest_OperationRecordServiceStop,
    eDvrTest_OperationDataInjectionStart,
    eDvrTest_OperationDataInjectionStop,
    eDvrTest_OperationStreamTSBStart,
    eDvrTest_OperationStreamTSBStop,
    eDvrTest_OperationTranscodeServiceStart,
    eDvrTest_OperationTranscodeServiceStop,
    eDvrTest_OperationPlay,
    eDvrTest_OperationPause,
    eDvrTest_OperationRewind,
    eDvrTest_OperationFastForward,
    eDvrTest_OperationSlowRewind,
    eDvrTest_OperationSlowForward,
    eDvrTest_OperationFrameAdvance,
    eDvrTest_OperationFrameReverse,
    eDvrTest_OperationSeekPlay,
    eDvrTest_OperationMediaFileRead,
    eDvrTest_OperationMediaFileWrite,
    eDvrTest_OperationListRecordings,
    eDvrTest_OperationDeleteRecording,
    eDvrTest_OperationGetMemoryUsage,
    eDvrTest_OperationMax
}DvrTest_Operation;

static void DvrTest_Scheduler(void * param)
{
    BSTD_UNUSED(param);
    BDBG_MSG(("Starting Scheduler 0x%08x", (unsigned)g_dvrTest->scheduler));
    B_Scheduler_Run(g_dvrTest->scheduler);
    return;
}
static void DvrTest_InjectData(void *streamPathContext)
{
    uint8_t ccByte=0;
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_Mutex_Lock(streamPath->streamPathMutex);
    if(!streamPath->dataInjectionStarted)
    {
        streamPath->dataInsertionCount++;
        if(streamPath->dataInsertionCount%2 == 0)
        {
            ++streamPath->patCcValue;
            ccByte = streamPath->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
            ccByte = (ccByte & 0xf0) | (streamPath->patCcValue & 0xf);
            streamPath->pat[4] = ccByte;
            if(streamPath->recordService) 
            {
                B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pat[1]),188);
            }
            else
            {
                B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pat[1]),188);
            }
        }
        else
        {
            ++streamPath->pmtCcValue;
            ccByte = streamPath->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
            ccByte = (ccByte & 0xf0) | (streamPath->pmtCcValue & 0xf);
            streamPath->pmt[4] = ccByte;
            if(streamPath->recordService) 
            {
                B_DVR_RecordService_InjectDataStart(streamPath->recordService,((void *)&streamPath->pmt[1]),188);
            }
            else
            {
                B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pmt[1]),188);
            }
        }
    }
    B_Mutex_Unlock(streamPath->streamPathMutex);
    return;
}

static void DvrTest_PatMonitor(void *streamPathContext)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_DVR_MediaFileStats mediaFileStats;
    ssize_t returnSize;
    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
    off_t returnOffset;
    B_Mutex_Lock(streamPath->streamPathMutex);
    B_DVR_MediaFile_Stats(streamPath->patFile,&mediaFileStats);
    if(mediaFileStats.currentOffset < mediaFileStats.startOffset)
    {
        mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
        mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset+188*20*4096;
        returnOffset = B_DVR_MediaFile_Seek(streamPath->patFile,&mediaFileSeekSettings);
        if(returnOffset < 0)
        {
            printf("\n error seeking into patFile");
            assert(0);
        }
    }
    if(mediaFileStats.endOffset - mediaFileStats.currentOffset < 1024*188) 
    {
        goto error;
    }
    if((mediaFileStats.currentOffset/B_DVR_MEDIA_SEGMENT_SIZE) > streamPath->patSegmentCount) 
    {
        if(streamPath->tsbService) 
        {
            /* BDBG_ERR(("\n DvrTest_PatMonitor %s %lld >>>",streamPath->tsbServiceRequest.programName,mediaFileStats.currentOffset));*/
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
            /*printf("\n sync byte not found at buffer[0] %u",buffer[0]);*/
            for(syncIndex=0;syncIndex<188*2;syncIndex++)
            {
                if(buffer[syncIndex]==0x47)
                break; 
            }
            buffer = (uint8_t *)streamPath->patBuffer + syncIndex;
            returnSize -= syncIndex;
            /* printf("\n new sync byte %u",buffer[0]);*/
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
    streamPath->patMonitorTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                         g_dvrTest->schedulerMutex,
                                                         50,DvrTest_PatMonitor,
                                                         (void*)streamPath);
    B_Mutex_Unlock(streamPath->streamPathMutex);
    return;
}

static void DvrTest_StreamTSB(void *streamPathContext)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_DVR_MediaFileStats mediaFileStats;
    ssize_t returnSize;
    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
    off_t returnOffset;
    B_Mutex_Lock(streamPath->streamPathMutex);
    B_DVR_MediaFile_Stats(streamPath->streamFile,&mediaFileStats);
    if(mediaFileStats.currentOffset < mediaFileStats.startOffset)
    {
        mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
        mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
        returnOffset = B_DVR_MediaFile_Seek(streamPath->streamFile,&mediaFileSeekSettings);
        if(returnOffset < 0)
        {
            printf("\n error seeking into patFile");
            assert(0);
        }
    }
    if(mediaFileStats.endOffset - mediaFileStats.currentOffset < 4096*188) 
    {
        goto error;
    }
    if((mediaFileStats.endOffset- mediaFileStats.currentOffset) >= 4096*1024) 
    {
        returnSize = B_DVR_MediaFile_Read(streamPath->streamFile,streamPath->streamBufAligned,4096*188);
        if(returnSize <=0) 
        {
            printf("streaming Read failed");
            goto error;
        }
    }

 error:
    streamPath->streamingTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                         g_dvrTest->schedulerMutex,
                                                         100,DvrTest_StreamTSB,
                                                         (void*)streamPath);
    B_Mutex_Unlock(streamPath->streamPathMutex);
    return;
}


static void DvrTest_Service_GetStatus(void *streamPathContext)
{
    #if 0
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_Mutex_Lock(streamPath->streamPathMutex);
    if(streamPath->playbackService)
    {
        B_DVR_PlaybackServiceStatus status;
        B_DVR_PlaybackService_GetStatus(streamPath->playbackService,&status);
        BDBG_MSG(("\n Playback Status:"));
        BDBG_MSG(("\n Time -> start:%ld current:%ld end:%ld",status.startTime,status.currentTime,status.endTime));
        BDBG_MSG(("\n Offset -> start:%lld current:%lld end:%lld",status.linearStartOffset,status.linearCurrentOffset,status.linearEndOffset));
        streamPath->getStatus = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                   g_dvrTest->schedulerMutex,
                                                   60000,DvrTest_Service_GetStatus,
                                                   (void*)streamPath);
    }
    else
    {
        if(streamPath->tsbService) 
        {
            B_DVR_TSBServiceStatus status;
            BDBG_ERR(("\n TSB Status:"));
            B_DVR_TSBService_GetStatus(streamPath->tsbService,&status);
            BDBG_ERR(("TSB Rec:"));
            BDBG_ERR(("off %lld:%lld",status.tsbRecLinearStartOffset,status.tsbRecLinearEndOffset));
            BDBG_ERR(("time %u:%u",status.tsbRecStartTime,status.tsbRecEndTime));
            streamPath->getStatus = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                       g_dvrTest->schedulerMutex,
                                                       60000,DvrTest_Service_GetStatus,
                                                       (void*)streamPath);

         }
    }
    B_Mutex_Unlock(streamPath->streamPathMutex);
    #endif
    return;
}

static void DvrTest_Continue_TsbConversion(void *streamPathContext)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)streamPathContext;
    B_DVR_TSBServicePermanentRecordingRequest *permRecReq = &streamPath->permRecReq ;
    B_Mutex_Lock(streamPath->streamPathMutex);
    if(streamPath->tsbService && permRecReq->recEndTime < 3600000 && streamPath->tsbConversionStarted)
    {
        B_DVR_ERROR rc = B_DVR_SUCCESS;
        rc = B_DVR_TSBService_ConvertStop(streamPath->tsbService);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n Failed to stop conversion");
        }
        permRecReq->recStartTime = permRecReq->recEndTime;
        permRecReq->recEndTime = permRecReq->recStartTime + 1800000;
        BKNI_Snprintf(permRecReq->programName,sizeof(permRecReq->programName),"%s_%u_%ld",TSB_CONVERT_PREFIX,(unsigned)streamPath->parserBand,permRecReq->recStartTime);
        sprintf(permRecReq->subDir,"tsbConv");
        printf("\n \t program name %s \t subDir %s start %lu \t end %lu",
               permRecReq->programName,
               permRecReq->subDir,
               permRecReq->recStartTime,
               permRecReq->recEndTime);
        rc = B_DVR_TSBService_ConvertStart(streamPath->tsbService,permRecReq);
        if(rc!=B_DVR_SUCCESS)
        {
            printf("\n tsb Convert start failed");
        }
    }
    B_Mutex_Unlock(streamPath->streamPathMutex);
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

void DvrTest_TuneChannel(int index)
{
    NEXUS_Error rc;
    int currentChannel = g_dvrTest->streamPath[index].currentChannel;
    printf("\n %s: index %d <<<",__FUNCTION__,index);

    NEXUS_Frontend_GetUserParameters(g_dvrTest->streamPath[index].frontend, &g_dvrTest->streamPath[index].userParams);
    printf("\n %s: Input band=%d ",__FUNCTION__,g_dvrTest->streamPath[index].userParams.param1);
    g_dvrTest->streamPath[index].parserBand = (NEXUS_ParserBand)g_dvrTest->streamPath[index].userParams.param1;
    NEXUS_ParserBand_GetSettings(g_dvrTest->streamPath[index].parserBand, &g_dvrTest->streamPath[index].parserBandSettings);
    g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(g_dvrTest->streamPath[index].frontend);
    /*g_dvrTest->streamPath[index].parserBandSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)g_dvrTest->streamPath[index].userParams.param1;*/
    g_dvrTest->streamPath[index].parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
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

void DvrTest_PlaybackDecodeStart(int index)
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
            #if NEXUS_HAS_STREAM_MUX
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            {
                tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeStcChannel;
            }
            else
            #endif
            {
                tsbServiceSettings.tsbPlaybackSettings.stcChannel =g_dvrTest->streamPath[index].stcChannel;
            }
            
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
            #if NEXUS_HAS_STREAM_MUX
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            { 
                playbackServiceSettings.stcChannel =g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeStcChannel;
            }
            else
            #endif 
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
        #if NEXUS_HAS_STREAM_MUX
        if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
        {
            g_dvrTest->streamPath[index].videoProgram.nonRealTime = true;
            
        }
        else
        #endif
        {
            g_dvrTest->streamPath[index].videoProgram.nonRealTime = false;
        }
        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);

        if(!index)
        {
            NEXUS_AudioDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].audioProgram);
            g_dvrTest->streamPath[index].audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
            g_dvrTest->streamPath[index].audioProgram.pidChannel = g_dvrTest->streamPath[index].audioPlaybackPidChannels[0];
            #if NEXUS_HAS_STREAM_MUX
            if(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime)
            { 
                NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcAudioChannel,&g_dvrTest->streamPath[index].stcSettings);
                g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
                NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
                g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcAudioChannel;
                g_dvrTest->streamPath[index].audioProgram.nonRealTime = true;

            }
            else
            #endif
            {
                g_dvrTest->streamPath[index].audioProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;
                g_dvrTest->streamPath[index].audioProgram.nonRealTime = false;
                #if NEXUS_NUM_HDMI_OUTPUTS
                NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                                                          NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                          NEXUS_AudioDecoderConnectorType_eStereo)
                                                                          );
                #endif
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
        #if NEXUS_HAS_STREAM_MUX
        if(!(g_dvrTest->streamPath[index].transcodeService && g_dvrTest->streamPath[index].transcodeServiceRequest.transcodeType == eB_DVR_TranscodeServiceType_NonRealTime))
        #endif
        { 
            #if NEXUS_NUM_HDMI_OUTPUTS
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
            NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
            #endif
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
    g_dvrTest->streamPath[index].audioPidChannels[0] = NEXUS_PidChannel_Open(g_dvrTest->streamPath[index].parserBand,
                                                                             channelInfo[currentChannel].audioPids[0],
                                                                             NULL);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&g_dvrTest->streamPath[index].videoProgram);
    g_dvrTest->streamPath[index].videoProgram.codec = channelInfo[currentChannel].videoFormat[0];
    g_dvrTest->streamPath[index].videoProgram.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    g_dvrTest->streamPath[index].videoProgram.stcChannel = g_dvrTest->streamPath[index].stcChannel;

    NEXUS_StcChannel_GetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);
    NEXUS_Timebase_GetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);
    timebaseSettings.freeze = false;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    if(channelInfo[currentChannel].pcrPid[0] == channelInfo[currentChannel].audioPids[0])
    {
        timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrTest->streamPath[index].audioPidChannels[0];
    }
    else
    {
        timebaseSettings.sourceSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    }
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
    NEXUS_Timebase_SetSettings(g_dvrTest->streamPath[index].stcSettings.timebase, &timebaseSettings);

    g_dvrTest->streamPath[index].stcSettings.autoConfigTimebase = true;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.offsetThreshold = 8;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.maxPcrError = 255;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableTimestampCorrection = false;
    g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.disableJitterAdjustment = false;
    if(channelInfo[currentChannel].pcrPid[0] == channelInfo[currentChannel].audioPids[0])
    {
        g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.pidChannel = g_dvrTest->streamPath[index].audioPidChannels[0];
    }
    else
    {
        g_dvrTest->streamPath[index].stcSettings.modeSettings.pcr.pidChannel = g_dvrTest->streamPath[index].videoPidChannels[0];
    }
    
    g_dvrTest->streamPath[index].stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */

    NEXUS_StcChannel_SetSettings(g_dvrTest->streamPath[index].stcChannel,&g_dvrTest->streamPath[index].stcSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[index].videoDecoder, &g_dvrTest->streamPath[index].videoProgram);
    if(!index)
    {
        #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(g_dvrTest->streamPath[index].audioDecoder,
                                                                    NEXUS_AudioDecoderConnectorType_eStereo)
                                   );
        #endif
       
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
        #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(g_dvrTest->platformconfig.outputs.hdmi[0]));
        #endif
    }
    NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].audioPidChannels[0]);
    NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[index].videoDecoder);
    NEXUS_PidChannel_Close( g_dvrTest->streamPath[index].videoPidChannels[0]);
    g_dvrTest->streamPath[index].liveDecodeStarted = false;
    printf("\n %s: index %d <<<",__FUNCTION__,index);
    return;
}

static void DvrTest_Callback(void *appContext, int index,B_DVR_Event event,B_DVR_Service service)
{
    struct DvrTest_StreamPath *streamPath = (struct DvrTest_StreamPath *)appContext;
    char *eventString[eB_DVR_EventMax+1] = {"eB_DVR_EventStartOfRecording","eB_DVR_EventEndOfRecording", "eB_DVR_EventHDStreamRecording",
 "eB_DVR_EventSDStreamRecording","eB_DVR_EventTSBConverstionCompleted","eB_DVR_EventOverFlow","eB_DVR_EventUnderFlow",
 "eB_DVR_EventStartOfPlayback", "eB_DVR_EventEndOfPlayback","eB_DVR_EventPlaybackAlarm","eB_DVR_EventAbortPlayback",
 "eB_DVR_EventAbortRecord","eB_DVR_EventAbortTSBRecord","eB_DVR_EventAbortTSBPlayback","eB_DVR_EventDataInjectionCompleted",
 "eB_DVR_EventTranscodeFinish","eB_DVR_EventMediaProbed","eB_DVR_EventOutOfMediaStorage","eB_DVR_EventOutOfNavigationStorage",
 "eB_DVR_EventOutOfMetaDataStorage","eB_DVR_EventFormatSuccess","eB_DVR_EventFormatFail","eB_DVR_EventFormatFail_InvalidIndex",
 "eB_DVR_EventFormatFail_NotRegistered",  "eB_DVR_EventFormatFail_VolumeMounted", "eB_DVR_EventFormatFail_SystemErr", 
 "eB_DVR_EventMountSuccss","eB_DVR_EventMountFail", "eB_DVR_EventMountFail_InvalidIndex", "eB_DVR_EventMountFail_NotRegistered",
 "eB_DVR_EventMountFail_VolumeMounted","eB_DVR_EventMountFail_NotFormatted","eB_DVR_EventMountFail_SystemErr",
 "eB_DVR_EventMountFail_WrongLabel","eB_DVR_EventMax"};
    char *serviceString[eB_DVR_ServiceMax+1]= {"tsbService","playbackService","recordService","mediaUtil","drmService",
                                               "storagaService","dataInjectionService","transcodeService","invalidService"};
    B_Mutex_Lock(streamPath->streamPathMutex);
    if(event != eB_DVR_EventDataInjectionCompleted) 
    {
        printf("\n DvrTest_Callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
    }
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
            if(streamPath->tsbConvEvent)
            B_Event_Set(streamPath->tsbConvEvent);
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
            B_DVR_OperationSettings operationSettings;
            operationSettings.operation = eB_DVR_OperationFastRewind;
            operationSettings.operationSpeed = 3;
            if(streamPath->tsbService)
            {  
                B_DVR_TSBServiceStatus status;
                printf("\n End of tsb. Pausing");
                B_DVR_TSBService_GetStatus(streamPath->tsbService,&status);
                printf("\n TSB playback Status:");
                BDBG_ERR(("\n Time bounds:"));
                BDBG_ERR(("\n start:%ld current:%ld end:%ld",
                          status.tsbRecStartTime,status.tsbCurrentPlayTime,
                          status.tsbRecEndTime));
                BDBG_ERR(("\n ByteOffset Bounds:")); 
                BDBG_ERR(("\n start:%lld current:%lld end:%lld",
                          status.tsbRecLinearStartOffset,status.tsbCurrentPlayOffset,
                          status.tsbRecLinearEndOffset));
                #if 0
                B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
                #endif
            }
            else
            {
                B_DVR_PlaybackServiceStatus status;
                printf("\n End of playback stream. Pausing");
                B_DVR_PlaybackService_GetStatus(streamPath->playbackService,&status);
                printf("\n Playback Status:");
                BDBG_ERR(("\n Time bounds:"));
                BDBG_ERR(("\n start:%ld current:%ld end:%ld",
                          status.startTime,status.currentTime,
                          status.endTime));
                BDBG_ERR(("\n ByteOffset Bounds:")); 
                BDBG_ERR(("\n start:%lld current:%lld end:%lld",
                          status.linearStartOffset,status.linearCurrentOffset,
                          status.linearEndOffset));
                #if 0
                B_DVR_PlaybackService_SetOperation(streamPath->playbackService,&operationSettings);
                #endif
            } 
         }
         break;
    case eB_DVR_EventStartOfPlayback:
        {
            B_DVR_OperationSettings operationSettings;
            if(streamPath->tsbService)
            {
                B_DVR_TSBServiceStatus status;
                printf("\n Beginning of TSB. Pausing");
                B_DVR_TSBService_GetStatus(streamPath->tsbService,&status);

                printf("\n TSB playback Status:");
                BDBG_ERR(("\n Time bounds:"));
                BDBG_ERR(("\n start:%ld current:%ld end:%ld",
                          status.tsbRecStartTime,status.tsbCurrentPlayTime,
                          status.tsbRecEndTime));
                BDBG_ERR(("\n ByteOffset Bounds:")); 
                BDBG_ERR(("\n start:%lld current:%lld end:%lld",
                          status.tsbRecLinearStartOffset,status.tsbCurrentPlayOffset,
                          status.tsbRecLinearEndOffset));
                operationSettings.operation = eB_DVR_OperationSeek;
                operationSettings.seekTime= status.tsbRecStartTime+2000;
                B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
                operationSettings.operation = eB_DVR_OperationPlay;
                B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);

                
            }
            else
            {
                B_DVR_PlaybackServiceStatus status;
                printf("\n Beginnning of playback stream. Pausing");
                B_DVR_PlaybackService_GetStatus(streamPath->playbackService,&status);
                printf("\n Playback Status:");
                BDBG_ERR(("\n Time bounds:"));
                BDBG_ERR(("\n start:%ld current:%ld end:%ld",
                          status.startTime,status.currentTime,
                          status.endTime));
                BDBG_ERR(("\n ByteOffset Bounds:")); 
                BDBG_ERR(("\n start:%lld current:%lld end:%lld",
                          status.linearStartOffset,status.linearCurrentOffset,
                          status.linearEndOffset));
                #if 0
                operationSettings.operation = eB_DVR_OperationFastForward;
                operationSettings.operationSpeed = 4;
                B_DVR_PlaybackService_SetOperation(streamPath->playbackService,&operationSettings);
                #endif
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
            if(streamPath->recordService)
            {
                B_DVR_RecordService_InjectDataStop(streamPath->recordService);
            }
            else
            {
                 B_DVR_TSBService_InjectDataStop(streamPath->tsbService);
            }
            streamPath->dataInjectionStarted = false;
            streamPath->dataInjectTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                 g_dvrTest->schedulerMutex,
                                                                 50,DvrTest_InjectData,
                                                                 (void*)streamPath);
        }
        break;
    case eB_DVR_EventOutOfMediaStorage:
    case eB_DVR_EventOutOfNavigationStorage:
    case eB_DVR_EventOutOfMetaDataStorage:
        {
            printf("\n no storage space");
            if(streamPath->tsbService && streamPath->tsbConversionStarted)
            { 
                B_Scheduler_UnregisterEvent(g_dvrTest->scheduler,streamPath->tsbConvEventSchedulerID);
                B_Event_Destroy(streamPath->tsbConvEvent);
                streamPath->tsbConvEvent = NULL;
                B_Mutex_Unlock(streamPath->streamPathMutex);
                B_DVR_TSBService_ConvertStop(streamPath->tsbService);
                B_Mutex_Lock(streamPath->streamPathMutex);
                streamPath->tsbConversionStarted=false;
            }
        }
        break;
#if NEXUS_HAS_STREAM_MUX
    case eB_DVR_EventTranscodeFinish:
        {
            printf("\n Transcode finish");
            BKNI_SetEvent(g_dvrTest->streamPath[index].transcodeFinishEvent);
        }
        break;
#endif
    case eB_DVR_EventMediaProbed:
        {
            if(streamPath->tsbService)
            {
                printf("\n TSB Service media probe complete");
            }
            if(streamPath->recordService)
            {
                printf("\n record service media probe complete");
            }
        }
        break;
    default:
        {
            printf("\n invalid event");
        }
   }
    if(event != eB_DVR_EventDataInjectionCompleted) 
    {
        printf("\n DvrTest_Callback <<<<<");
    }
    B_Mutex_Unlock(streamPath->streamPathMutex);
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
            BDBG_ERR(("Video encoder codec %d is not supported!\n",videoCodec));
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
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audioCodec));
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

#if NEXUS_HAS_STREAM_MUX
B_DVR_ERROR DvrTest_FileToFileTranscodeStart(unsigned pathIndex)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode playbackMediaNode;
    B_DVR_RecordServiceSettings recordServiceSettings;
    NEXUS_StcChannelSettings stcSettings;
    B_DVR_TranscodeServiceInputEsStream transcodeInputEsStream;
    B_DVR_TranscodeServiceInputSettings transcodeInputSettings;
    B_DVR_RecordServiceInputEsStream recordInputEsStream;
    NEXUS_PidChannelHandle transcodePidChannel;
    B_DVR_TranscodeServiceSettings transcodeServiceSettings;
    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;

    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop >>>");
    /* stop live decode*/
    if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
    {
       DvrTest_LiveDecodeStop(pathIndex);
    }

    printf("\n DvrTest_FileToFileTranscodeStart : DvrTest_LiveDecodeStop <<<");
    BKNI_Memset(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].playbackServiceRequest));
    g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
    if(!g_dvrTest->streamPath[pathIndex].streamPathMutex) 
    {
        printf("\n g_dvrTest->streamPath[pathIndex].streamPathMutex create failed");
        assert(0);
    }

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
    B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
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
    B_DVR_TranscodeService_InstallCallback(g_dvrTest->streamPath[pathIndex].transcodeService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_InstallCallback <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings >>>");
    B_DVR_TranscodeService_GetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    transcodeInputSettings.input = eB_DVR_TranscodeServiceInput_File;
    transcodeInputSettings.playbackService =g_dvrTest->streamPath[pathIndex].playbackService;
    transcodeInputSettings.parserBand = g_dvrTest->streamPath[pathIndex].parserBand;
    #if NEXUS_NUM_HDMI_INPUTS
    transcodeInputSettings.hdmiInput = NULL;
    #endif
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetInputSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings >>>");
    B_DVR_TranscodeService_SetInputSettings(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetInputSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode >>>");
    playbackMediaNode = B_DVR_PlaybackService_GetMediaNode(g_dvrTest->streamPath[pathIndex].playbackService);
    assert(playbackMediaNode);
    printf("\n playback vPid %u aPid %u",playbackMediaNode->esStreamInfo[0].pid,playbackMediaNode->esStreamInfo[1].pid);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_PlaybackService_GetMediaNode <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream vPid>>>");
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,&playbackMediaNode->esStreamInfo[0],sizeof(playbackMediaNode->esStreamInfo[0]));
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
    BKNI_Memcpy((void *)&transcodeInputEsStream.esStreamInfo,(void *)&playbackMediaNode->esStreamInfo[1],sizeof(playbackMediaNode->esStreamInfo[1]));
    /* add audio ES stream*/
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream aPid <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid >>>");
    BKNI_Memset((void*)&transcodeInputEsStream,0,sizeof(transcodeInputEsStream));
    transcodeInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodeInputEsStream.esStreamInfo.pid = playbackMediaNode->esStreamInfo[1].pid+1;
    transcodeInputEsStream.playpumpIndex = NEXUS_ANY_ID;
    /* add pcr ES stream*/
    B_DVR_TranscodeService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].transcodeService,&transcodeInputEsStream);

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_AddInputEsStream pcrPid <<<");

    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings >>>");
    B_DVR_TranscodeService_GetSettings(g_dvrTest->streamPath[pathIndex].transcodeService,
                                       &transcodeServiceSettings);
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_GetSettings <<<");
    printf("\n DvrTest_FileToFileTranscodeStart : B_DVR_TranscodeService_SetSettings >>>");
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

    DvrTest_Create_PatPmt(pathIndex,
                          playbackMediaNode->esStreamInfo[1].pid +1,
                          playbackMediaNode->esStreamInfo[0].pid, 
                          playbackMediaNode->esStreamInfo[1].pid,
                          NEXUS_VideoCodec_eH264,
                          NEXUS_AudioCodec_eAac);
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
    recordInputEsStream.esStreamInfo.pid = playbackMediaNode->esStreamInfo[0].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    recordInputEsStream.esStreamInfo.codec.videoCodec = NEXUS_VideoCodec_eH264; /*playbackMedia.esStreamInfo[0].codec.videoCodec*/;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                               playbackMediaNode->esStreamInfo[0].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMediaNode->esStreamInfo[0].pid);
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
    recordInputEsStream.esStreamInfo.pid = playbackMediaNode->esStreamInfo[1].pid;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    recordInputEsStream.esStreamInfo.codec.audioCodec = NEXUS_AudioCodec_eAac; /*playbackMedia.esStreamInfo[1].codec.audioCodec;*/
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMediaNode->esStreamInfo[1].pid);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMediaNode->esStreamInfo[1].pid);
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
    recordInputEsStream.esStreamInfo.pid = playbackMediaNode->esStreamInfo[1].pid+1;
    recordInputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    transcodePidChannel = B_DVR_TranscodeService_GetPidChannel(g_dvrTest->streamPath[pathIndex].transcodeService,
                                                                playbackMediaNode->esStreamInfo[1].pid+1);
    if(!transcodePidChannel)
    {
        printf("\n error in getting the transcode pid Channel for %u",playbackMediaNode->esStreamInfo[1].pid+1);
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
    DvrTest_PlaybackDecodeStart(pathIndex);
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
    B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
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
#endif

int main(void)
{
    
    int decoderIndex,frontendIndex, pathIndex, operation;
    int i;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus mediaStorageStatus;
    B_DVR_ERROR dvrError = B_DVR_SUCCESS;
    DvrTest_Operation dvrTestOp;
    printf("\n mediaNode V0 %u \n",sizeof(B_DVR_Media));
    system("umount /dev/sda1");
    system("umount /dev/sda2");
    system("umount /dev/sda3"); 

    BKNI_Init();
    g_dvrTest = BKNI_Malloc(sizeof(*g_dvrTest));

    BKNI_Memset(g_dvrTest,0,sizeof(*g_dvrTest));
    
    mediaStorageOpenSettings.storageType = eB_DVR_MediaStorageTypeBlockDevice;
    g_dvrTest->mediaStorage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);

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
                /*g_dvrTest->mediaStorageRegisterSettings.length = 20000;*/
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
        DvrTest_AudioVideoPathOpen(decoderIndex);
        DvrTest_LiveDecodeStart(decoderIndex);
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

    for(pathIndex=0;pathIndex<MAX_STREAM_PATH;pathIndex++)
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

    for(pathIndex=0;pathIndex<MAX_STREAM_PATH;pathIndex++)
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
    printf("\n sizeof(fileNode) %u sizeof(mediaNode) %u",sizeof(B_DVR_SegmentedFileNode),sizeof(B_DVR_Media));
    g_dvrTest->schedulerMutex = B_Mutex_Create(NULL);
    if( !g_dvrTest->schedulerMutex)
    {
        printf("\n schedulerMutex create error");
        assert(0);
    }
    g_dvrTest->scheduler = B_Scheduler_Create(NULL);
    if(!g_dvrTest->scheduler)
    {
        printf("\n scheduler create error");
        assert(0);
    }
    g_dvrTest->thread = B_Thread_Create("DvrTest Scheduler", DvrTest_Scheduler,g_dvrTest, NULL);

    if(!g_dvrTest->thread)
    {
        printf("\n scheduler thread create error");
        assert(0);
    }
    
    while(1)
    { 
        DvrTest_Menu();
        fflush(stdin);
        operation=0;
        pathIndex=0;
        scanf("%d", &operation);
        dvrTestOp = (DvrTest_Operation)operation; 
        if(dvrTestOp > eDvrTest_OperationQuit && operation < eDvrTest_OperationMediaFileRead)
        {
            printf("\n enter streamPath Index for %d : ",operation);
            fflush(stdin);
            scanf("%d",&pathIndex);
         }

        if (!operation)
        {
            break;
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
                if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex < 2)
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
                if(pathIndex < 2)
                { 
                    DvrTest_LiveDecodeStart(pathIndex);
                    g_dvrTest->streamPath[pathIndex].liveDecodeStarted=true;
                }
            }
            case1_error:
            break;
    case eDvrTest_OperationTSBServiceStart:
        {
            unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
            B_DVR_TSBServiceInputEsStream inputEsStream;
            B_DVR_TSBServiceSettings tsbServiceSettings;
            B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
            if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted && pathIndex < MAX_AV_PATH)
            {
                DvrTest_LiveDecodeStart(pathIndex);
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
            B_DVR_TSBService_InstallCallback(g_dvrTest->streamPath[pathIndex].tsbService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);

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
            B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);

            inputEsStream.esStreamInfo.pid = channelInfo[currentChannel].audioPids[0];
            inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
            inputEsStream.esStreamInfo.codec.audioCodec = channelInfo[currentChannel].audioFormat[0];
            B_DVR_TSBService_AddInputEsStream(g_dvrTest->streamPath[pathIndex].tsbService,&inputEsStream);

            #if 0
            B_DVR_TSBService_GetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);
            tsbServiceSettings.tsbRecordSettings.esStreamCount =2;
            tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[0].pid = 0x1ff2;
            tsbServiceSettings.tsbRecordSettings.RemappedEsStreamInfo[1].pid = 0x1ff3;
            B_DVR_TSBService_SetSettings(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceSettings);
            #endif
            DvrTest_Create_PatPmt(pathIndex,
                                  channelInfo[currentChannel].pcrPid[0],
                                  channelInfo[currentChannel].videoPids[0],
                                  channelInfo[currentChannel].audioPids[0],
                                  channelInfo[currentChannel].videoFormat[0],
                                  channelInfo[currentChannel].audioFormat[0]);
            g_dvrTest->streamPath[pathIndex].patMonitorTimer = NULL;
            g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
            assert(g_dvrTest->streamPath[pathIndex].streamPathMutex);
            dvrError = B_DVR_TSBService_Start(g_dvrTest->streamPath[pathIndex].tsbService,NULL);
            if(dvrError!=B_DVR_SUCCESS)
            {
                printf("\n Error in starting the tsbService %d",pathIndex);
            }
            else
            {
                printf("tsbService started %d\n",pathIndex);
            }
            g_dvrTest->streamPath[pathIndex].getStatus = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                                g_dvrTest->schedulerMutex,
                                                                                60000,DvrTest_Service_GetStatus,
                                                                                (void*)&g_dvrTest->streamPath[pathIndex]);
            assert(g_dvrTest->streamPath[pathIndex].getStatus);
        }
        break;
        case eDvrTest_OperationTSBServiceStop:
            {
                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {
                    if(g_dvrTest->streamPath[pathIndex].getStatus) 
                    {
                         B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].getStatus);
                    }
                    dvrError = B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted && pathIndex<MAX_AV_PATH)
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
                    g_dvrTest->streamPath[pathIndex].dataInjectionService=NULL;
                    g_dvrTest->streamPath[pathIndex].tsbService = NULL;
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n Error in closing the timeshift service %d\n",pathIndex);
                    }
                    B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
                    
                }
                else
                {
                   printf("\n timeshift srvc not started");
                }
               
            }
            break;
        case eDvrTest_OperationTSBServicePlayStart:
            {
                if(!g_dvrTest->streamPath[pathIndex].tsbService || pathIndex>=MAX_AV_PATH)
                {
                    printf("\n error TSBPlayStart without TSBServiceStart or pathIndex>=MAX_AV_PATH");
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
                        DvrTest_LiveDecodeStop(pathIndex);
                    }
                    DvrTest_PlaybackDecodeStart(pathIndex);
                    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                }
            }
            break;
        case eDvrTest_OperationTSBServicePlayStop:
            {
                if(!g_dvrTest->streamPath[pathIndex].tsbService || pathIndex >= MAX_AV_PATH)
                {
                    printf("\n either tsbService inactive or pathIndex>=MAX_AV_PATH");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    operationSettings.operation = eB_DVR_OperationTSBPlayStop;
                    B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    DvrTest_PlaybackDecodeStop(pathIndex);
                    if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                    {
                        DvrTest_LiveDecodeStart(pathIndex);
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
                    B_DVR_TSBServicePermanentRecordingRequest *permRecReq = &g_dvrTest->streamPath[pathIndex].permRecReq ;

                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService, &tsbStatus);
                    printf("\n current tsb Rec start %ld ms end %ld ms",tsbStatus.tsbRecStartTime, tsbStatus.tsbRecEndTime);
                    fflush(stdin);
                    printf("\n Enter start time:");
                    scanf("%lu",&permRecReq->recStartTime);
                    permRecReq->recEndTime = permRecReq->recStartTime + 1800000;
                    BKNI_Snprintf(permRecReq->programName,sizeof(permRecReq->programName),"%s_%u_%ld",
                                  TSB_CONVERT_PREFIX,
                                  (unsigned )g_dvrTest->streamPath[pathIndex].parserBand,
                                  permRecReq->recStartTime);
                    sprintf(permRecReq->subDir,"tsbConv");

                    printf("\n \t program name %s \t subDir %s start %lu \t end %lu",
                           permRecReq->programName,
                           permRecReq->subDir,
                           permRecReq->recStartTime,
                           permRecReq->recEndTime);
                    rc = B_DVR_TSBService_ConvertStart(g_dvrTest->streamPath[pathIndex].tsbService,permRecReq);
                    if(rc!=B_DVR_SUCCESS)
                    {
                        printf("\n Invalid paramters");
                    }
                    g_dvrTest->streamPath[pathIndex].tsbConversionStarted = true;
                    g_dvrTest->streamPath[pathIndex].tsbConvEvent = B_Event_Create(NULL);
                    if(!g_dvrTest->streamPath[pathIndex].tsbConvEvent)
                    {
                        printf("tsbConvEvent create failed");
                        assert(0);
                    }

                    g_dvrTest->streamPath[pathIndex].tsbConvEventSchedulerID =  B_Scheduler_RegisterEvent(g_dvrTest->scheduler,
                                                                                 g_dvrTest->schedulerMutex,
                                                                                 g_dvrTest->streamPath[pathIndex].tsbConvEvent,
                                                                                 DvrTest_Continue_TsbConversion,
                                                                                 (void*)&g_dvrTest->streamPath[pathIndex]);
                    if(!g_dvrTest->streamPath[pathIndex].tsbConvEventSchedulerID)
                    {
                        printf("tsbConvEvent registration  failed");
                        assert(0);
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

               if(g_dvrTest->streamPath[pathIndex].tsbService)
               { 
                   B_Scheduler_UnregisterEvent(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].tsbConvEventSchedulerID);
                   B_Event_Destroy(g_dvrTest->streamPath[pathIndex].tsbConvEvent);
                   g_dvrTest->streamPath[pathIndex].tsbConvEvent = NULL;
                   rc = B_DVR_TSBService_ConvertStop(g_dvrTest->streamPath[pathIndex].tsbService);
                   if(rc!=B_DVR_SUCCESS)
                   {
                       printf("\n Failed to stop conversion");
                   }
                   g_dvrTest->streamPath[pathIndex].tsbConversionStarted=false;
               }
            }
            break;
        case eDvrTest_OperationRecordPause:
            {
                if(g_dvrTest->streamPath[pathIndex].tsbService) 
                {
                    B_DVR_TSBServiceStatus status;
                    BDBG_ERR(("\n TSB Status:"));
                    B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&status);
                    BDBG_ERR(("off %lld:%lld",status.tsbRecLinearStartOffset,status.tsbRecLinearEndOffset));
                    BDBG_ERR(("time %u:%u",status.tsbRecStartTime,status.tsbRecEndTime));
                    dvrError = B_DVR_TSBService_Pause(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n error in pausing tsbService index %d\n",pathIndex);
                    }
                }
                else
                {
                    if(g_dvrTest->streamPath[pathIndex].recordService) 
                    {
                        B_DVR_RecordServiceStatus status;
                        BDBG_ERR(("\n Record Status:"));
                        B_DVR_RecordService_GetStatus(g_dvrTest->streamPath[pathIndex].recordService,&status);
                        BDBG_ERR(("off %lld:%lld",status.linearStartOffset,status.linearCurrentOffset));
                        BDBG_ERR(("time %u:%u",status.startTime,status.linearCurrentOffset));
                        dvrError = B_DVR_RecordService_Pause(g_dvrTest->streamPath[pathIndex].recordService);
                        if(dvrError != B_DVR_SUCCESS)
                        {
                            printf("\n error in pausing recordService index %d\n",pathIndex);
                        }
                    }
                    else
                    {
                        printf("\n neither record or tsb service started");
                    }
                }
            }
            break;
        case eDvrTest_OperationRecordResume:
            {
                if(g_dvrTest->streamPath[pathIndex].tsbService) 
                {
                    dvrError = B_DVR_TSBService_Resume(g_dvrTest->streamPath[pathIndex].tsbService);
                    if(dvrError != B_DVR_SUCCESS)
                    {
                        printf("\n error in resuming tsbService index %d\n",pathIndex);
                    }
                }
                else
                {
                    if(g_dvrTest->streamPath[pathIndex].recordService) 
                    {
                        dvrError = B_DVR_RecordService_Resume(g_dvrTest->streamPath[pathIndex].recordService);
                        if(dvrError != B_DVR_SUCCESS)
                        {
                            printf("\n error in resuming recordService index %d\n",pathIndex);
                        }
                    }
                    else
                    {
                        printf("\n neither record or tsb service started");
                    }
                }
            }
            break;
        case eDvrTest_OperationPlaybackServiceStart:
            {
                 if(g_dvrTest->streamPath[pathIndex].tsbService || pathIndex >=MAX_AV_PATH)
                 {
                     printf("\n either TSB is on or pathIndex>=MAX_AV_PATH");
                     goto error_playbackService;
                 }
                 fflush(stdin);
                 printf("\n Enter the sub folder name for recording to be playedback");
                 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.subDir);
                 printf("\n Enter the recording name");
                 scanf("%s",g_dvrTest->streamPath[pathIndex].playbackServiceRequest.programName);
                 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.playpumpIndex = pathIndex;
                 g_dvrTest->streamPath[pathIndex].playbackServiceRequest.volumeIndex = 0;
                 g_dvrTest->streamPath[pathIndex].playbackService = B_DVR_PlaybackService_Open(&g_dvrTest->streamPath[pathIndex].playbackServiceRequest);
                 printf("\n B_DVR_PlaybackService_Open");
                 if(!g_dvrTest->streamPath[pathIndex].playbackService)
                 {
                     printf("\n playback service open failed");
                     goto error_playbackService;
                 }
                 if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                 {
                     DvrTest_LiveDecodeStop(pathIndex);
                 }

                 B_DVR_PlaybackService_InstallCallback(g_dvrTest->streamPath[pathIndex].playbackService,
                                                  g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
                 printf("\n DvrTest_LiveDecodeStop");
                 DvrTest_PlaybackDecodeStart(pathIndex);
                 printf("\n DvrTest_PlaybackDecodeStart");
                 B_DVR_PlaybackService_Start(g_dvrTest->streamPath[pathIndex].playbackService);
                 printf("\n B_DVR_PlaybackService_Start");
                 g_dvrTest->streamPath[pathIndex].getStatus = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                                 g_dvrTest->schedulerMutex,
                                                                                 60000,DvrTest_Service_GetStatus,
                                                                                 (void*)&g_dvrTest->streamPath[pathIndex]);
                g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
                if(!g_dvrTest->streamPath[pathIndex].streamPathMutex) 
                {
                    printf("\n g_dvrTest->streamPath[pathIndex].streamPathMutex create failed");
                    assert(0);
                }
                 
            }
            error_playbackService:
            break;
        case eDvrTest_OperationPlaybackServiceStop:
            {

                 if(g_dvrTest->streamPath[pathIndex].playbackService)
                 {
                     if(g_dvrTest->streamPath[pathIndex].getStatus) 
                     {
                         B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].getStatus);
                     }
                     B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
                     DvrTest_PlaybackDecodeStop(pathIndex);
                     B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
                     g_dvrTest->streamPath[pathIndex].playbackService = NULL;
                     if(!g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
                     {
                         DvrTest_LiveDecodeStart(pathIndex);
                     }
                     B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
                 }
                 else
                 {
                     printf("\n playbackService on active on path %d",pathIndex);
                 }
            }
            break;
        case eDvrTest_OperationRecordServiceStart:
            {
                if((pathIndex > 0 && pathIndex < 2 ) || g_dvrTest->streamPath[pathIndex].tsbService)
                {
                    printf("\n pathIndex < 2 or tsbService is active");
                }
                else
                {
                    unsigned currentChannel = g_dvrTest->streamPath[pathIndex].currentChannel;
                    B_DVR_RecordServiceInputEsStream inputEsStream;
                    B_DVR_DataInjectionServiceOpenSettings dataInjectionOpenSettings;
                    B_DVR_RecordServiceSettings recordServiceSettings;
                    BKNI_Memset((void *)&g_dvrTest->streamPath[pathIndex].recordServiceRequest,0,sizeof(g_dvrTest->streamPath[pathIndex].recordServiceRequest));
                    fflush(stdin);
                    printf("\n Enter recording name:");
                    scanf("%s",g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName);

                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.volumeIndex =0;
                    sprintf(g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir,"bgrec");
                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.recpumpIndex = pathIndex;
                    g_dvrTest->streamPath[pathIndex].recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
                    printf("\n recordService open >>>");
                    g_dvrTest->streamPath[pathIndex].recordService = B_DVR_RecordService_Open(&g_dvrTest->streamPath[pathIndex].recordServiceRequest);
                    printf("\n recordService open <<");
                    printf("\n recordService install callback >>");
                    B_DVR_RecordService_InstallCallback(g_dvrTest->streamPath[pathIndex].recordService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
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
                    DvrTest_Create_PatPmt(pathIndex,
                                  channelInfo[currentChannel].pcrPid[0],
                                  channelInfo[currentChannel].videoPids[0],
                                  channelInfo[currentChannel].audioPids[0],
                                  channelInfo[currentChannel].videoFormat[0],
                                  channelInfo[currentChannel].audioFormat[0]);
                    printf("\n recordService start >>");
                    B_DVR_RecordService_Start(g_dvrTest->streamPath[pathIndex].recordService,NULL);
                    printf("\n recordService start <<");

                    g_dvrTest->streamPath[pathIndex].streamPathMutex = B_Mutex_Create(NULL);
                    if(!g_dvrTest->streamPath[pathIndex].streamPathMutex) 
                    {
                        printf("\n g_dvrTest->streamPath[pathIndex].streamPathMutex create failed");
                        assert(0);
                    }

                
                 }
             }
             break;
        case eDvrTest_OperationRecordServiceStop:
            {
                if(g_dvrTest->streamPath[pathIndex].recordService)
                {
                    B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
                    B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                    B_DVR_DataInjectionService_Close(g_dvrTest->streamPath[pathIndex].dataInjectionService);
                    g_dvrTest->streamPath[pathIndex].dataInjectionService = NULL;
                    g_dvrTest->streamPath[pathIndex].recordService = NULL;
                    B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
                }
            }
            break;
        case eDvrTest_OperationDataInjectionStart:
            {
                B_DVR_DataInjectionServiceSettings settings;
                B_DVR_MediaFileOpenMode openMode;
                B_DVR_MediaFilePlayOpenSettings openSettings;
                char *programName;
                B_DVR_MediaFileStats mediaFileStats;
                B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
                off_t returnOffset;
                NEXUS_PidChannelSettings pidChannelSettings; 
                
                
                g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
                g_dvrTest->streamPath[pathIndex].patCcValue=0;
                g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
                g_dvrTest->streamPath[pathIndex].psiCount=0;
                g_dvrTest->streamPath[pathIndex].patSegmentCount=0;

                B_DVR_DataInjectionService_InstallCallback(g_dvrTest->streamPath[pathIndex].dataInjectionService,g_dvrTest->dvrTestCallback,(void*)&g_dvrTest->streamPath[pathIndex]);
                pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2)-pathIndex;
                pidChannelSettings.enabled = false;
                g_dvrTest->streamPath[pathIndex].dataInjectionPidChannel 
                     = NEXUS_PidChannel_Open(g_dvrTest->streamPath[pathIndex].parserBand,
                                             0x1ffe,
                                             &pidChannelSettings);
                settings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
                settings.pidChannel = g_dvrTest->streamPath[pathIndex].dataInjectionPidChannel;
                settings.pid = 0x0; /*unused*/
                B_DVR_DataInjectionService_SetSettings(
                    g_dvrTest->streamPath[pathIndex].dataInjectionService,
                    &settings);

                openMode = eB_DVR_MediaFileOpenModeStreaming;

                openSettings.playpumpIndex = 8;
                openSettings.volumeIndex = 0;
                if(g_dvrTest->streamPath[pathIndex].tsbService) 
                {
                    strcpy(openSettings.subDir,g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir);
                    programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
                }
                else
                {
                    strcpy(openSettings.subDir,g_dvrTest->streamPath[pathIndex].recordServiceRequest.subDir);
                    programName = g_dvrTest->streamPath[pathIndex].recordServiceRequest.programName;
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

                g_dvrTest->streamPath[pathIndex].dataInjectTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                   g_dvrTest->schedulerMutex,
                                                                   50,DvrTest_InjectData,
                                                                   &g_dvrTest->streamPath[pathIndex]);

                g_dvrTest->streamPath[pathIndex].patMonitorTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                   g_dvrTest->schedulerMutex,
                                                                   50,DvrTest_PatMonitor,
                                                                   &g_dvrTest->streamPath[pathIndex]);

                if(g_dvrTest->streamPath[pathIndex].tsbService)
                {
                    B_DVR_TSBService_InjectDataStart(g_dvrTest->streamPath[pathIndex].tsbService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
                }
                else
                {
                    if(g_dvrTest->streamPath[pathIndex].recordService)
                    {
                        B_DVR_RecordService_InjectDataStart(g_dvrTest->streamPath[pathIndex].recordService,(void *)&g_dvrTest->streamPath[pathIndex].pat[1],188);
                    }
                    else
                    {
                        printf("\n neither record or tsb is active on path %d",pathIndex);
                    }
                }
                g_dvrTest->streamPath[pathIndex].dataInjectionStarted=true;
            }
            break;

        case eDvrTest_OperationDataInjectionStop:
            {
                if(g_dvrTest->streamPath[pathIndex].dataInjectionStarted) 
                {
                    g_dvrTest->streamPath[pathIndex].dataInjectionStarted=false;
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
                    NEXUS_PidChannel_Close(g_dvrTest->streamPath[pathIndex].dataInjectionPidChannel);
                }

                if(g_dvrTest->streamPath[pathIndex].dataInjectTimer)
                {
                     B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].dataInjectTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patMonitorTimer) 
                {
                    B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].patMonitorTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patFile) 
                {
                    B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].patFile);
                }
                g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
                g_dvrTest->streamPath[pathIndex].patCcValue=0;
                g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
                g_dvrTest->streamPath[pathIndex].patSegmentCount=0;
            }
            break;
        case eDvrTest_OperationStreamTSBStart:
            {
                if(g_dvrTest->streamPath[pathIndex].tsbService) 
                {
                    B_DVR_MediaFileOpenMode openMode;
                    B_DVR_MediaFilePlayOpenSettings openSettings;
                    char *programName;
                    B_DVR_MediaFileStats mediaFileStats;
                    B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
                    off_t returnOffset;
                    openMode = eB_DVR_MediaFileOpenModeStreaming;
                    openSettings.playpumpIndex = 8;
                    openSettings.volumeIndex = 0;
                    strcpy(openSettings.subDir,g_dvrTest->streamPath[pathIndex].tsbServiceRequest.subDir);
                    programName = g_dvrTest->streamPath[pathIndex].tsbServiceRequest.programName;
                    g_dvrTest->streamPath[pathIndex].streamFile = B_DVR_MediaFile_Open(programName,openMode,&openSettings);
                    if(!g_dvrTest->streamPath[pathIndex].streamFile)
                    {
                        printf("\n failed to open streamFile");
                        assert(0);                
                    }
                    g_dvrTest->streamPath[pathIndex].streamBuf = BKNI_Malloc(188*4096 + B_DVR_IO_BLOCK_SIZE);
                    g_dvrTest->streamPath[pathIndex].streamBufAligned = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)g_dvrTest->streamPath[pathIndex].streamBuf);
                    BKNI_Memset(g_dvrTest->streamPath[pathIndex].streamBufAligned,0,188*4096);
                    if(!g_dvrTest->streamPath[pathIndex].streamBufAligned)
                    {
                        printf("\n error patBuffer alloc");
                        assert(0);
                    }
                    mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
                    mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
                    returnOffset = B_DVR_MediaFile_Seek(g_dvrTest->streamPath[pathIndex].streamFile,&mediaFileSeekSettings);
                    if(returnOffset < 0)
                    {
                        printf("\n error seeking into streamFile");
                        assert(0);
                    }

                    g_dvrTest->streamPath[pathIndex].streamingTimer = B_Scheduler_StartTimer(g_dvrTest->scheduler,
                                                                                              g_dvrTest->schedulerMutex,
                                                                                              100,DvrTest_StreamTSB,
                                                                                              &g_dvrTest->streamPath[pathIndex]);
                    
                }
                else
                {
                    printf("\n TSBService not started");
                }
            }
            break;
        case eDvrTest_OperationStreamTSBStop:
            {

                if(g_dvrTest->streamPath[pathIndex].tsbService) 
                {
                    if(g_dvrTest->streamPath[pathIndex].streamingTimer) 
                    {
                        B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].streamingTimer);
                    }
                    if(g_dvrTest->streamPath[pathIndex].streamFile) 
                    {
                        B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].streamFile);
                    }
                    if(g_dvrTest->streamPath[pathIndex].streamBuf)
                    {
                        BKNI_Free(g_dvrTest->streamPath[pathIndex].streamBuf);
                    }
                }
                else
                {
                    printf("\n tsb service not started for path %u",pathIndex);
                }

            }
            break;
        case eDvrTest_OperationTranscodeServiceStart:
            {
                #if NEXUS_HAS_STREAM_MUX
                B_DVR_ERROR rc = B_DVR_SUCCESS;
                printf("\n *****************************************************");
                printf("\n *********** Transcoding Options *********************");
                printf("\n *****************************************************");
                printf("\n\t 1.FileToFileTranscode");
                printf("\n\t 2.QamToFileTranscode");
                printf("\n\t 3.QamToTsbTranscode");
                printf("\n\t 4.QamToBufferTranscode");
                fflush(stdin);
                printf("\n Enter a transcode option : ");
                scanf("%u",&g_dvrTest->streamPath[pathIndex].transcodeOption);
                printf("\n transcode option %d",g_dvrTest->streamPath[pathIndex].transcodeOption);

                if(g_dvrTest->streamPath[pathIndex].transcodeOption > 1 
                   && g_dvrTest->streamPath[pathIndex].transcodeOption < 1)
                {
                    printf("\n invalid transcode option or option not supported yet.");
                    break;
                }

                switch(g_dvrTest->streamPath[pathIndex].transcodeOption)
                {
                case 1:
                    {
                        rc = DvrTest_FileToFileTranscodeStart(pathIndex);
                
                    }
                    break;
               case 2:
                    {
                        rc = DvrTest_QamToFileTranscodeStart(pathIndex);
                    }
                    break;
                case 3:
                    {
                        rc = DvrTest_QamToTsbTranscodeStart(pathIndex);
                    }
               case 4:
                    {
                        rc = DvrTest_QamToBufferTranscodeStart(pathIndex);
                    }
                default:
                    printf("invalid transcode option");
                }
            #else
                printf("\n transcode service not supported on this platform \n");
            #endif
            }
            break;
        case eDvrTest_OperationTranscodeServiceStop:
            {
                #if NEXUS_HAS_STREAM_MUX
                B_DVR_ERROR rc = B_DVR_SUCCESS;
                switch(g_dvrTest->streamPath[pathIndex].transcodeOption)
                {
                case 1:
                    {
                        rc = DvrTest_FileToFileTranscodeStop(pathIndex);

                    }
                    break;
                case 2:
                    {
                        rc = DvrTest_QamToFileTranscodeStop(pathIndex);
                    }
                    break;
                case 3:
                    {
                        rc = DvrTest_QamToTsbTranscodeStop(pathIndex);
                    }
                    break;
                case 4:
                    {
                        rc = DvrTest_QamToBufferTranscodeStop(pathIndex);
                    }
                default:
                    printf("invalid transcode option");
                }
            #else
                printf("\n transcode service not supported on this platform \n");
            #endif
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
                        B_DVR_TSBServiceStatus status;
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&status);
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime= status.tsbRecEndTime;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                        operationSettings.operation = eB_DVR_OperationPause;
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
        case eDvrTest_OperationSeekPlay:
            {
                if(!g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
                {
                    printf("\n playback not started. Start either TSB playback or linear playback");
                    break;
                }
                else
                {
                    B_DVR_OperationSettings operationSettings;
                    NEXUS_VideoDecoderSettings videoDecoderSettings;
                    int increment=0;
                    int direction=1;
                    operationSettings.operation = eB_DVR_OperationPause;
                    if(g_dvrTest->streamPath[pathIndex].tsbService)
                    {
                        B_DVR_TSBServiceStatus tsbServiceStatus;
                        B_DVR_TSBService_GetStatus(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus);
                        if(tsbServiceStatus.state != NEXUS_PlaybackState_ePaused) 
                        {
                            printf("\n Paused");
                            B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                        }
                        printf("\n startTime %ld CurrentPlayTime %ld endTime %ld",
                               tsbServiceStatus.tsbRecStartTime,
                               tsbServiceStatus.tsbCurrentPlayTime,
                               tsbServiceStatus.tsbRecEndTime);
                        printf("\n Enter the increment: ");
                        fflush(stdin);
                        scanf("%d",&increment);
                        printf("\n Enter the direction (1-forward 0-reverse): ");
                        fflush(stdin);
                        scanf("%d",&direction);
                        if(direction) 
                        {
                            tsbServiceStatus.tsbCurrentPlayTime += increment;
                        }
                        else
                        {
                            tsbServiceStatus.tsbCurrentPlayTime -= increment;
                        }
                        printf("\n requested seek time %ld",tsbServiceStatus.tsbCurrentPlayTime);
                        B_DVR_TSBService_GetIFrameTimeStamp(g_dvrTest->streamPath[pathIndex].tsbService,&tsbServiceStatus.tsbCurrentPlayTime);
                        printf("\n I Frame seek time %ld",tsbServiceStatus.tsbCurrentPlayTime);
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime = tsbServiceStatus.tsbCurrentPlayTime;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                        
                        NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[pathIndex].videoDecoder,&videoDecoderSettings);
                        videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
                        NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[pathIndex].videoDecoder,&videoDecoderSettings);
                        
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[pathIndex].audioDecoder);
                        NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[pathIndex].videoDecoder);
                        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[pathIndex].videoDecoder, &g_dvrTest->streamPath[pathIndex].videoProgram);
                        NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[pathIndex].audioDecoder,&g_dvrTest->streamPath[pathIndex].audioProgram);
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_TSBService_SetOperation(g_dvrTest->streamPath[pathIndex].tsbService,&operationSettings);
                    }
                    else
                    {
                        B_DVR_PlaybackServiceStatus playbackServiceStatus;
                        B_DVR_PlaybackService_GetStatus(g_dvrTest->streamPath[pathIndex].playbackService,&playbackServiceStatus);
                        if(playbackServiceStatus.state != NEXUS_PlaybackState_ePaused) 
                        {
                            printf("\n Paused");
                            B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                        }
                        printf("\n startTime %ld CurrentPlayTime %ld endTime %ld",
                               playbackServiceStatus.startTime,
                               playbackServiceStatus.currentTime,
                               playbackServiceStatus.endTime);
                        printf("\n Enter the increment: ");
                        fflush(stdin);
                        scanf("%d",&increment);
                        printf("\n Enter the direction (1-forward 0-reverse): ");
                        fflush(stdin);
                        scanf("%d",&direction);
                        if(direction) 
                        {
                            playbackServiceStatus.currentTime += increment;
                        }
                        else
                        {
                            playbackServiceStatus.currentTime -= increment;
                        }
                        printf("\n requested seek time %ld",playbackServiceStatus.currentTime);
                        B_DVR_PlaybackService_GetIFrameTimeStamp(g_dvrTest->streamPath[pathIndex].playbackService,&playbackServiceStatus.currentTime);
                        printf("\n I Frame seek time %ld",playbackServiceStatus.currentTime);
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime = playbackServiceStatus.currentTime;
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                        
                        NEXUS_VideoDecoder_GetSettings(g_dvrTest->streamPath[pathIndex].videoDecoder,&videoDecoderSettings);
                        videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
                        NEXUS_VideoDecoder_SetSettings(g_dvrTest->streamPath[pathIndex].videoDecoder,&videoDecoderSettings);
                        
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_PlaybackService_SetOperation(g_dvrTest->streamPath[pathIndex].playbackService,&operationSettings);
                        NEXUS_AudioDecoder_Stop(g_dvrTest->streamPath[pathIndex].audioDecoder);
                        NEXUS_VideoDecoder_Stop(g_dvrTest->streamPath[pathIndex].videoDecoder);
                        NEXUS_VideoDecoder_Start(g_dvrTest->streamPath[pathIndex].videoDecoder, &g_dvrTest->streamPath[pathIndex].videoProgram);
                        NEXUS_AudioDecoder_Start(g_dvrTest->streamPath[pathIndex].audioDecoder,&g_dvrTest->streamPath[pathIndex].audioProgram);
                        operationSettings.operation = eB_DVR_OperationPause;
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
                B_Time startTime,endTime;
                unsigned long avgTime=0;
                unsigned long totalTime=0;
                unsigned long bps = 0;
                unsigned long numChunks=0;
                fflush(stdin);
                printf("\n Enter sub folder name:");
                scanf("%s",openSettings.subDir);
                printf("\n Enter programName:");
                scanf("%s",programName);
                openMode = eB_DVR_MediaFileOpenModeStreaming;
                openSettings.playpumpIndex = 3;
                openSettings.volumeIndex = 0;
                bufSize = 188*1024*4;
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
                   B_Time_Get(&startTime);
                   B_DVR_MediaFile_Stats(g_dvrTest->mediaFile,&mediaFileStats);
                   returnSize = B_DVR_MediaFile_Read(g_dvrTest->mediaFile,g_dvrTest->buffer,bufSize);
                   B_Time_Get(&endTime);
                   BDBG_ERR(("time taken to read %u :%ld at %lld",
                             (unsigned long)returnSize,
                             B_Time_Diff(&endTime,&startTime),
                             mediaFileStats.currentOffset));
                   totalTime+=B_Time_Diff(&endTime,&startTime);
                   if(returnSize < 0)
                   {
                       printf("\n Error reading");
                       close(fd);
                       break;
                   }
                   else
                   {
                       write(fd, (void*)g_dvrTest->buffer,returnSize);
                   }
                   numChunks++;
               }
               avgTime = totalTime/numChunks;
               BDBG_ERR(("Avg time to read %u:%u(ms)",bufSize,avgTime));
               bps = (mediaFileStats.endOffset - mediaFileStats.startOffset)/totalTime;
               bps = bps*8*1000;
               BDBG_ERR((" Avg mediaFile_read rate %u(bps)",bps));
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
                /*ssize_t returnSize;*/

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
                        printf("\n refCount %u",media.refCount);
                        printf("\n version %u",media.version);
                        printf("\n size of mediaNode %u",sizeof(media));
                        printf("\n size of refCount %u",sizeof(media.refCount));
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
        case eDvrTest_OperationDeleteRecording:
            {
                 char programName[256];
                 unsigned volIndex;
                 char subDir[256];
                 B_DVR_MediaNodeSettings mediaNodeSettings;
                 B_DVR_ERROR err;
                 printf("volumeIndex?: ");
                 scanf("%u",&volIndex);
                 printf("program name to delete: ");
                 scanf("%s",programName);    
                 printf("subdir: ");
                 scanf("%s",subDir);    
                 if(subDir[0]=='/') subDir[0]='\0';
    
                 mediaNodeSettings.programName =programName;
                 mediaNodeSettings.subDir = subDir;
                 mediaNodeSettings.volumeIndex = volIndex;
                 printf("deleting volume %u:%s\n",mediaNodeSettings.volumeIndex, mediaNodeSettings.programName);
                 err = B_DVR_Manager_DeleteRecording(g_dvrTest->dvrManager,&mediaNodeSettings);
                 if (err!=B_DVR_SUCCESS) 
                 printf("check the file name!\n");
            }
            break;
        case eDvrTest_OperationGetMemoryUsage:
            {
                unsigned long memoryUsed=0;
                memoryUsed = B_DVR_Manager_GetMemoryUsage(g_dvrTest->dvrManager,eB_DVR_MemoryType_Device);
                printf("\n device memory used : %uKB \n", memoryUsed);
                memoryUsed = B_DVR_Manager_GetMemoryUsage(g_dvrTest->dvrManager,eB_DVR_MemoryType_System);
                printf("\n system memory used : %uKB \n", memoryUsed);
            }
            break;
        default:
           printf("\n Invalid DVR Operation - Select the operation again");
        }
   
    }
    
    printf("\n Closing all the services before stopping the test app");
    for(pathIndex=0;pathIndex < MAX_STREAM_PATH; pathIndex++)
    {
        if(pathIndex < MAX_AV_PATH)
        { 
            if(g_dvrTest->streamPath[pathIndex].playbackDecodeStarted)
            {
                DvrTest_PlaybackDecodeStop(pathIndex);
            }
            if(g_dvrTest->streamPath[pathIndex].playbackService)
            {
                if(g_dvrTest->streamPath[pathIndex].getStatus) 
                {
                    B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].getStatus);
                }
                B_DVR_PlaybackService_Stop(g_dvrTest->streamPath[pathIndex].playbackService);
                B_DVR_PlaybackService_Close(g_dvrTest->streamPath[pathIndex].playbackService);
                B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
            }
            if(g_dvrTest->streamPath[pathIndex].liveDecodeStarted)
            {
                DvrTest_LiveDecodeStop(pathIndex);
            }
            DvrTest_AudioVideoPathClose(pathIndex);
        }
        else
        {
            if(g_dvrTest->streamPath[pathIndex].tsbService)
            {
                if(g_dvrTest->streamPath[pathIndex].tsbConversionStarted) 
                {
                    B_Scheduler_UnregisterEvent(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].tsbConvEventSchedulerID);
                    B_Event_Destroy(g_dvrTest->streamPath[pathIndex].tsbConvEvent);
                    g_dvrTest->streamPath[pathIndex].tsbConvEvent = NULL;
                    g_dvrTest->streamPath[pathIndex].tsbConversionStarted=false;
                }
                if(g_dvrTest->streamPath[pathIndex].dataInjectionStarted) 
                {
                    g_dvrTest->streamPath[pathIndex].dataInjectionStarted=false;
                    B_DVR_TSBService_InjectDataStop(g_dvrTest->streamPath[pathIndex].tsbService);
                }
                if(g_dvrTest->streamPath[pathIndex].dataInjectTimer)
                {
                     B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].dataInjectTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patMonitorTimer) 
                {
                    B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].patMonitorTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patFile) 
                {
                    B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].patFile);
                }
                g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
                g_dvrTest->streamPath[pathIndex].patCcValue=0;
                g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
                g_dvrTest->streamPath[pathIndex].patSegmentCount=0;
                if(g_dvrTest->streamPath[pathIndex].getStatus) 
                {
                    B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].getStatus);
                }
                B_DVR_TSBService_Stop(g_dvrTest->streamPath[pathIndex].tsbService);
                B_DVR_TSBService_Close(g_dvrTest->streamPath[pathIndex].tsbService);
                B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
            }
            if(g_dvrTest->streamPath[pathIndex].recordService)
            {
                if(g_dvrTest->streamPath[pathIndex].dataInjectionStarted) 
                {
                    g_dvrTest->streamPath[pathIndex].dataInjectionStarted=false;
                    B_DVR_RecordService_InjectDataStop(g_dvrTest->streamPath[pathIndex].recordService);
                }
                if(g_dvrTest->streamPath[pathIndex].dataInjectTimer)
                {
                     B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].dataInjectTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patMonitorTimer) 
                {
                    B_Scheduler_CancelTimer(g_dvrTest->scheduler,g_dvrTest->streamPath[pathIndex].patMonitorTimer);
                }
                if(g_dvrTest->streamPath[pathIndex].patFile) 
                {
                    B_DVR_MediaFile_Close(g_dvrTest->streamPath[pathIndex].patFile);
                }
                g_dvrTest->streamPath[pathIndex].dataInsertionCount =0;
                g_dvrTest->streamPath[pathIndex].patCcValue=0;
                g_dvrTest->streamPath[pathIndex].pmtCcValue =0;
                g_dvrTest->streamPath[pathIndex].patSegmentCount=0;
                B_DVR_RecordService_Stop(g_dvrTest->streamPath[pathIndex].recordService);
                B_DVR_RecordService_Close(g_dvrTest->streamPath[pathIndex].recordService);
                B_Mutex_Destroy(g_dvrTest->streamPath[pathIndex].streamPathMutex);
            }
        }
    }

    for(pathIndex=0;pathIndex<MAX_STREAM_PATH;pathIndex++)
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
    B_Scheduler_Stop(g_dvrTest->scheduler);
    B_Thread_Destroy(g_dvrTest->thread);
    B_Scheduler_Destroy(g_dvrTest->scheduler);
    B_Mutex_Destroy(g_dvrTest->schedulerMutex);
    B_Os_Uninit();
    NEXUS_Platform_Uninit();
    BKNI_Free(g_dvrTest);
    BKNI_Uninit();
    return 0;
    
}
