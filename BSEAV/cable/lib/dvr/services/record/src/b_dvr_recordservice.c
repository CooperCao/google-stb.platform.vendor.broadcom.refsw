/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
#include "b_dvr_manager.h"
#include "b_dvr_manager_priv.h"
#include "b_dvr_recordservice.h"
#include "b_dvr_file.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_list.h"
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_mediaprobe.h"
#ifdef TRANSCODE_SUPPORT
#include "b_dvr_transcodeservice.h"
#endif
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

BDBG_MODULE(b_dvr_recordservice);
BDBG_OBJECT_ID(B_DVR_RecordService);

struct B_DVR_RecordService
{
    BDBG_OBJECT(B_DVR_RecordService)
    unsigned index;
    NEXUS_RecpumpHandle recpump; 
    NEXUS_RecordHandle record;
    NEXUS_PlaybackHandle inProgressRecPlay;
    NEXUS_FileRecordHandle nexusFileRecord;
    NEXUS_PidChannelHandle recordPidChannels[B_DVR_MAX_PIDS];
    B_DVR_RecordServiceSettings settings;
    struct B_DVR_RecordServiceRequest recordServiceRequest;
    B_DVR_ServiceCallback registeredCallback;
    void *appContext;
    B_SchedulerEventId updateMediaNodeEventSchedulerID;
    B_SchedulerTimerId recordTimer;
    B_EventHandle updateMediaNodeEvent;
    B_EventHandle updatePlaybackEvent;
    B_DVR_MediaNode mediaNode;
    B_MutexHandle recordMutex;
    bool dataInjectionStarted;
    bool recordStarted;
    bool recpumpStarted;
    bool recordProbed;
    bool mpeg2Video;
    B_DVR_EsStreamInfo activeEsStreamInfo[B_DVR_MAX_PIDS];
    unsigned activeEsStreamCount;
    BKNI_EventHandle recPumpEvent;
    B_DVR_MediaProbeHandle mediaProbe;
    B_DVR_DRMServiceHandle recordDrm;
    B_DVR_DRMServiceHandle mediaProbeDrm;
    off_t probeStartOffset;
    unsigned long mediaNodeSyncCount;
    B_MutexHandle schedulerMutex;
    unsigned long lastPrintStatusTime;
    bool overflow;
    bool mediaSegmentSizeValidated;
};
#define SCT6_SIZE 24
static void dataready_callback(void *context, int param);
static void overflow_callback(void *context, int param);
static unsigned B_DVR_RecordService_P_CalculateBitRate(
    off_t byteOffsetStart, off_t byteOffsetEnd, unsigned startTime,unsigned endTime);
static void B_DVR_RecordService_P_SegmentedFileEvent(
    void *segmentedFileContext,B_DVR_SegmentedFileEvent fileEvent);
static void B_DVR_RecordService_P_UpdateMediaNode(void *recordContext);
void B_DVR_RecordService_P_PrintStatus(B_DVR_RecordServiceHandle recordService);
void B_DVR_RecordService_P_PrintRecInfo(B_DVR_RecordServiceHandle recordService);
static void B_DVR_RecordService_P_Overflow(void *recordContext,int param);
B_DVR_ERROR B_DVR_RecordService_P_Pause(B_DVR_RecordServiceHandle recordService);
B_DVR_ERROR B_DVR_RecordService_P_Resume(B_DVR_RecordServiceHandle recordService);
bool B_DVR_RecordService_P_DetectVideoStart(const uint8_t *itbBuffer,bool mpeg2Video,unsigned long *videoStartOffset);
unsigned int B_DVR_RecordService_P_GetCurrentTimestamp(void); 

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_MSG(("dataready_callback set event\n"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    B_DVR_RecordServiceHandle recordService = (B_DVR_RecordServiceHandle) context;
    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_WRN(("VOD recpump overflow handling %s",recordService->mediaNode->programName));
    B_Mutex_Lock(recordService->recordMutex);
    if(recordService->recpumpStarted) 
    {
        NEXUS_Recpump_Stop(recordService->recpump);
        NEXUS_Recpump_Start(recordService->recpump);
    }
    B_Mutex_Unlock(recordService->recordMutex);
    return;
}

static unsigned B_DVR_RecordService_P_CalculateBitRate(off_t byteOffsetStart, off_t byteOffsetEnd, unsigned startTime,unsigned endTime)
{
    off_t byteLen=0;
    unsigned timeLen=0;
    unsigned bitRate=0;
    BDBG_MSG(("B_DVR_RecordService_P_CalculateBitRate>>>"));
    timeLen = (endTime - startTime); /* in ms*/
    if(!timeLen)
    {
        BDBG_MSG(("bitRate zero since timeLen is zero"));
        goto done;
    }
    byteLen = byteOffsetEnd - byteOffsetStart;

    byteLen = byteLen*8; /*Convert bytes into bits*/
    bitRate = byteLen/timeLen;    /* bits per ms*/
    bitRate *= 1000;    /* bits per s*/
done:
    BDBG_MSG(("B_DVR_RecordService_P_CalculateBitRate %u<<<",bitRate));
    return bitRate;
}
static void B_DVR_RecordService_P_SegmentedFileEvent(void *segmentedFileContext,B_DVR_SegmentedFileEvent fileEvent)
{
    B_DVR_Event event=eB_DVR_EventMax;
    B_DVR_Service service;
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle)(segmentedFileContext);
    B_DVR_RecordServiceHandle recordService = NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_ASSERT(segmentedFile);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_P_SegmentedFileEvent >>>"));
    BDBG_MSG(("index %d service %d event %d",segmentedFile->serviceIndex,
              segmentedFile->service,event));

    recordService = dvrManager->recordService[segmentedFile->serviceIndex];
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    if(recordService->registeredCallback)
    {
        switch(fileEvent)
        {
        case B_DVR_SegmentedFileEventReadError:
            {
              BDBG_ERR(("Read error : not applicable for recordService"));
              goto error;
            }
            break;
        case B_DVR_SegmentedFileEventSeekError:
            {
                event = eB_DVR_EventAbortRecord;
                BDBG_ERR(("Abort record event - seek error"));
            }
            break;
        case B_DVR_SegmentedFileEventWriteError:
            {
                event = eB_DVR_EventAbortRecord;
                BDBG_ERR(("Abort record Event - Write Error"));
            }
            break;
        case B_DVR_SegmentedFileEventOpenError:
            {
                event = eB_DVR_EventAbortRecord;
                BDBG_ERR(("Abort Record Event - Open Error"));
                
            }
            break;
        case B_DVR_SegmentedFileEventNotFoundError:
            {
               BDBG_ERR(("file not found : not applicable for recordService"));
               goto error;
            }
            break;
        case B_DVR_SegmentedFileEventNoFileSpaceError:
            {
                if(segmentedFile->fileType == eB_DVR_FileTypeNavigation)
                { 
                    event = eB_DVR_EventOutOfNavigationStorage;
                    BDBG_ERR(("No space in nav partition"));
                }
                else
                {
                    event = eB_DVR_EventOutOfMediaStorage;
                    BDBG_ERR(("No space in media partition"));
                }
            }
            break;
        case B_DVR_SegmentFileEventMax:
            {
                BDBG_ERR(("Invalid segmented file event"));
                goto error;
            }
            break;
        }
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,segmentedFile->service,event,segmentedFile->service);
    }
error:
    BDBG_MSG(("B_DVR_RecordService_P_SegmentedFileEvent <<<<"));
    return;
}


static void B_DVR_RecordService_P_UpdateMediaNode(void *recordContext)
{
    B_DVR_RecordServiceHandle recordService = (B_DVR_RecordServiceHandle)recordContext;
    B_DVR_FilePosition startPosition,endPosition;
    B_DVR_Event event;
    B_DVR_Service service;
    bool recordStarted,recordProbed;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_P_UpdateMediaNode >>>"));
    B_Mutex_Lock(recordService->recordMutex);
    if(recordService->overflow)
    {
        BDBG_WRN(("%s program %s overflow",
                  __FUNCTION__,
                  recordService->mediaNode->programName));
        B_Mutex_Unlock(recordService->recordMutex);
        goto error;
    }

    recordStarted = recordService->recordStarted;
    recordProbed = recordService->recordProbed;
    if(!recordService->recordStarted)
    {
        NEXUS_RecordSettings recordSettings;
        NEXUS_Record_GetSettings(recordService->record, &recordSettings);
        recordSettings.pollingTimer = B_DVR_RECORD_POLLING_INTERVAL;
        NEXUS_Record_SetSettings(recordService->record, &recordSettings);
        recordService->recordStarted = true;
    }
    B_DVR_SegmentedFileRecord_GetBounds(recordService->nexusFileRecord,&startPosition,&endPosition);
    recordService->mediaNode->mediaLinearStartOffset = startPosition.mpegFileOffset;
    recordService->mediaNode->mediaLinearEndOffset =  endPosition.mpegFileOffset;
    recordService->mediaNode->navLinearStartOffset = startPosition.navFileOffset;
    recordService->mediaNode->navLinearEndOffset = endPosition.navFileOffset;
    recordService->mediaNode->mediaStartTime = startPosition.timestamp;
    recordService->mediaNode->mediaEndTime = endPosition.timestamp;
    recordService->mediaNode->maxStreamBitRate = 
        B_DVR_RecordService_P_CalculateBitRate(recordService->mediaNode->mediaLinearStartOffset,
                                               recordService->mediaNode->mediaLinearEndOffset,
                                               recordService->mediaNode->mediaStartTime,
                                               recordService->mediaNode->mediaEndTime);
    if(recordService->mediaNode->mediaLinearEndOffset >= recordService->mediaNodeSyncCount*B_DVR_MEDIA_SEGMENT_SIZE) 
    {
        BDBG_MSG(("sync .info file(%s): leo:%lld syncCount:%u",
                  recordService->mediaNode->mediaNodeFileName,
                  recordService->mediaNode->mediaLinearEndOffset,
                  recordService->mediaNodeSyncCount));
        B_DVR_List_UpdateMediaNode(dvrManager->dvrList,recordService->mediaNode,recordService->recordServiceRequest.volumeIndex,true);
        recordService->mediaNodeSyncCount++;
    }
    else
    {
        B_DVR_List_UpdateMediaNode(dvrManager->dvrList,recordService->mediaNode,recordService->recordServiceRequest.volumeIndex,false);
    }
    
    if(recordService->mediaProbe && 
       !recordService->recordProbed && 
       (recordService->mediaNode->mediaLinearEndOffset >= 
        recordService->probeStartOffset+B_DVR_MEDIA_PROBE_CHUNK_SIZE))
    {
        recordService->recordProbed = B_DVR_MediaProbe_Parse(recordService->mediaProbe,recordService->mediaProbeDrm,&recordService->probeStartOffset);
        if(!recordService->recordProbed)
        {
            BDBG_MSG(("attempt to probe failed. Try again"));
        }
    }
    if(recordService->updatePlaybackEvent)
    {
        B_Event_Set(recordService->updatePlaybackEvent);
    }

    if( (recordService->mediaNode->mediaEndTime-recordService->lastPrintStatusTime)
        >= B_DVR_PRINT_REC_STATUS_INTERVAL)
    {
        B_DVR_RecordService_P_PrintStatus(recordService);
        recordService->lastPrintStatusTime = 
            recordService->mediaNode->mediaEndTime;
    }

    B_Mutex_Unlock(recordService->recordMutex);

    if(recordService && 
       (recordStarted !=recordService->recordStarted && recordService->recordStarted) &&
       recordService->registeredCallback)
    {
        event = eB_DVR_EventStartOfRecording;
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_MSG(("eB_DVR_EventStartOfRecording is sent to upper layers"));
    }

    if(recordService && 
       (recordProbed!=recordService->recordProbed && recordService->recordProbed) &&
       recordService->registeredCallback)
    {
        service = eB_DVR_ServiceRecord;
        event = eB_DVR_EventMediaProbed;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_MSG(("mediaProbe complete callback sent"));
        if(recordService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_HD_STREAM)
        { 
            event = eB_DVR_EventHDStreamRecording;
        }
        else
        {
            event = eB_DVR_EventSDStreamRecording;
        }
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_MSG(("HD/SD recording callback sent"));
    }
    if(startPosition.mpegFileOffset > endPosition.mpegFileOffset) 
    {
        event = eB_DVR_EventAbortRecord;
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_ERR(("recording to be aborted startOffset:%lld endOffset:%lld",
                  startPosition.mpegFileOffset,endPosition.mpegFileOffset));
    }
error:
    BDBG_MSG(("B_DVR_RecordService_P_UpdateMediaNode <<<"));
    return;
}

void B_DVR_RecordService_P_PrintStatus(B_DVR_RecordServiceHandle recordService)
{
    unsigned sizeOfNavEntry=0;
    unsigned numOfFrames=0;
    unsigned duration=0;
    off_t len=0;
    BKNI_Printf("\n **************************");
    BKNI_Printf("\n program:%s record Status",recordService->mediaNode->programName);
    BKNI_Printf("\n startTime:%ums endTime:%ums",
              recordService->mediaNode->mediaStartTime,
              recordService->mediaNode->mediaEndTime);
    duration = recordService->mediaNode->mediaEndTime-recordService->mediaNode->mediaStartTime;
    duration = duration/1000;
    duration = duration/60;
    BKNI_Printf("\n duration:%u mins",duration);
    BKNI_Printf("\n media - startOffset:%lld endOffset:%lld",
              recordService->mediaNode->mediaLinearStartOffset,
              recordService->mediaNode->mediaLinearEndOffset);
    len = recordService->mediaNode->mediaLinearEndOffset - recordService->mediaNode->mediaLinearStartOffset;
    BKNI_Printf("\n mediaLength:%lld",len);
    BKNI_Printf("\n navigation - startOffset:%lld endOffset:%lld",
              recordService->mediaNode->navLinearStartOffset,
              recordService->mediaNode->navLinearEndOffset);
    sizeOfNavEntry = recordService->mpeg2Video?sizeof(BNAV_Entry):sizeof(BNAV_AVC_Entry);
    numOfFrames = (recordService->mediaNode->navLinearEndOffset - 
                   recordService->mediaNode->navLinearStartOffset)/sizeOfNavEntry;
    BKNI_Printf("\n numOfVideoFrames:%u",numOfFrames);
    BKNI_Printf("\n bitRate:%ubps",recordService->mediaNode->maxStreamBitRate);
    BKNI_Printf("\n mediaProbed:%s",recordService->recordProbed?"true":"false");
    BKNI_Printf("\n mediaProbed offset:%lld",recordService->probeStartOffset);
    BKNI_Printf("\n ***************************\n");
    return;
}

void B_DVR_RecordService_P_PrintRecInfo(B_DVR_RecordServiceHandle recordService)
{

    unsigned index=0;
    BKNI_Printf("\n *****************************");
    BKNI_Printf("\n record program:%s",
                  recordService->mediaNode->programName);
    BKNI_Printf("\n record %s",recordService->recpumpStarted?"started":"stopped");
    BKNI_Printf("\n parserBand:%u",recordService->settings.parserBand);
    BKNI_Printf("\n numberofPIDs:%u",recordService->mediaNode->esStreamCount);
    BKNI_Printf("\n encrypted:%s",recordService->mediaProbeDrm?"true":"false");
    for(index=0;index<recordService->mediaNode->esStreamCount;index++) 
    {
        BKNI_Printf("\n PID:%u",recordService->mediaNode->esStreamInfo[index].pid);
        if(recordService->mediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeVideo) 
        {
            BKNI_Printf("\n pidtype:video codec:%u",recordService->mediaNode->esStreamInfo[index].codec.videoCodec);
        }
        else
        {
            if(recordService->mediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeAudio) 
            {
                BKNI_Printf("\n pidtype:audio codec:%u",
                         recordService->mediaNode->esStreamInfo[index].codec.audioCodec);
            }
            else
            {
                BKNI_Printf("pidtype: data");
            }
        }
    }
    BKNI_Printf("\n ******************************\n");
    return;
}

static void B_DVR_RecordService_P_Overflow(void *recordContext,int param)
{
    B_DVR_RecordServiceHandle recordService = (B_DVR_RecordServiceHandle)recordContext;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecpumpStatus recpumpStatus;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BSTD_UNUSED(param);
    BDBG_WRN(("B_DVR_RecordService_P_Overflow program %s index %u >>>",
              recordService->mediaNode->programName,
              recordService->index));

    B_Mutex_Lock(recordService->schedulerMutex);
    B_Mutex_Lock(recordService->recordMutex);
    if(!recordService->recordStarted) 
    {
        BDBG_WRN(("index data not available yet"));
        goto error;
    }
    recordService->overflow=true;
    NEXUS_Recpump_GetStatus(recordService->recpump,&recpumpStatus);
    BDBG_WRN(("media FIFO Depth:%u",recpumpStatus.data.fifoDepth));
    BDBG_WRN(("media FIFO Size:%u",recpumpStatus.data.fifoSize));
    BDBG_WRN(("index FIFO Depth:%u",recpumpStatus.index.fifoDepth));
    BDBG_WRN(("index FIFO Size:%u",recpumpStatus.index.fifoSize));

    NEXUS_Record_GetSettings(recordService->record, &recordSettings);
    recordSettings.overflowCallback.callback = NULL;
    recordSettings.overflowCallback.context = NULL;
    recordSettings.overflowCallback.param = 0;
    NEXUS_Record_SetSettings(recordService->record, &recordSettings);
    B_Mutex_Unlock(recordService->recordMutex);
    if(recordService->registeredCallback)
    {
        event = eB_DVR_EventOverFlow;
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_MSG(("record overflow callback sent"));
    }
    B_Mutex_Lock(recordService->recordMutex);
    rc = B_DVR_RecordService_P_Pause(recordService);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("B_DVR_RecordService_P_Pause failed"));
        goto error;
    }
    NEXUS_Record_GetSettings(recordService->record, &recordSettings);
    recordSettings.overflowCallback.callback = B_DVR_RecordService_P_Overflow;
    recordSettings.overflowCallback.context = recordService;
    recordSettings.overflowCallback.param = recordService->index;
    NEXUS_Record_SetSettings(recordService->record, &recordSettings);
    rc = B_DVR_RecordService_P_Resume(recordService);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("B_DVR_RecordService_P_Resume failed"));
        goto error;
    }
    recordService->overflow = false;
error:
    B_Mutex_Unlock(recordService->recordMutex);
    B_Mutex_Unlock(recordService->schedulerMutex);
    if(rc!=B_DVR_SUCCESS && recordService->registeredCallback)
    {
        event = eB_DVR_EventAbortRecord;
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
        BDBG_WRN(("record abort callback sent"));
    }
    BDBG_WRN(("B_DVR_RecordService_P_Overflow <<<"));
    return;
}
B_DVR_ERROR B_DVR_RecordService_P_Pause(B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_FilePosition startPosition,endPosition;
    B_DVR_SegmentedFileRecordSettings segmentedFileRecordSettings;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord=NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_FilePosition position;
    unsigned long timeStamp=0;
    off_t mediaLinearOffset=0;
    unsigned mediaSegmentNum1=0,mediaSegmentNum2=0;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_P_Pause>>>"));
    if(recordService->updateMediaNodeEventSchedulerID) 
    {
        B_Scheduler_UnregisterEvent(dvrManager->scheduler,recordService->updateMediaNodeEventSchedulerID);
    }
    B_Event_Reset(recordService->updateMediaNodeEvent);
    segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(recordService->nexusFileRecord);
    recordService->updateMediaNodeEventSchedulerID = NULL;
    if(recordService->recpumpStarted)
    {
        NEXUS_Record_Stop(recordService->record);
    }
    recordService->recpumpStarted = false;
    recordService->recordStarted = false;

    BDBG_MSG(("media time: recStart %ld recEnd %ld",
              recordService->mediaNode->mediaStartTime,
              recordService->mediaNode->mediaEndTime));
    BDBG_MSG(("media Offset: recStart %lld recEnd %lld",
              recordService->mediaNode->mediaLinearStartOffset,
              recordService->mediaNode->mediaLinearEndOffset));
    BDBG_MSG(("nav offset : recStart %lld recEnd %lld",
              recordService->mediaNode->navLinearStartOffset,
              recordService->mediaNode->navLinearEndOffset));

    rc = B_DVR_SegmentedFileRecord_GetBounds(recordService->nexusFileRecord,&startPosition,&endPosition);
    if(rc != B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:B_DVR_SegmentedFileRecord_GetBounds failed",__FUNCTION__));
        BDBG_ERR(("program name %s index:%u",recordService->mediaNode->programName,recordService->index));
        BDBG_ERR(("media time: recStart %ld recEnd %ld",
                recordService->mediaNode->mediaStartTime,
                recordService->mediaNode->mediaEndTime));
        BDBG_ERR(("media Offset: recStart %lld recEnd %lld",
                recordService->mediaNode->mediaLinearStartOffset,
                recordService->mediaNode->mediaLinearEndOffset));
        BDBG_ERR(("nav offset : recStart %lld recEnd %lld",
                recordService->mediaNode->navLinearStartOffset,
                recordService->mediaNode->navLinearEndOffset));
        rc= B_DVR_UNKNOWN;
        goto error;
     }
     recordService->mediaNode->mediaLinearStartOffset = startPosition.mpegFileOffset;
     recordService->mediaNode->mediaLinearEndOffset =  endPosition.mpegFileOffset;
     recordService->mediaNode->navLinearStartOffset = startPosition.navFileOffset;
     recordService->mediaNode->navLinearEndOffset = endPosition.navFileOffset;
     recordService->mediaNode->mediaStartTime = startPosition.timestamp;
     recordService->mediaNode->mediaEndTime = endPosition.timestamp;
    
    mediaLinearOffset = recordService->mediaNode->mediaLinearEndOffset;
    BDBG_MSG(("mediaLinearOffset before alignment %lld",mediaLinearOffset));
    mediaSegmentNum1 = mediaLinearOffset/B_DVR_MEDIA_SEGMENT_SIZE;
    BDBG_MSG(("mediaSegNo1:%u",mediaSegmentNum1));
    mediaLinearOffset-= (mediaLinearOffset%(4096*(188/4)));
    BDBG_MSG(("mediaLinearOffset after alignment %lld",mediaLinearOffset));
    mediaSegmentNum2 = mediaLinearOffset/B_DVR_MEDIA_SEGMENT_SIZE;
    BDBG_MSG(("mediaSegNo2:%u",mediaSegmentNum2));
    if(mediaSegmentNum1!=mediaSegmentNum2) 
    {
        recordService->mediaNode->mediaLinearEndOffset -= 
            (recordService->mediaNode->mediaLinearEndOffset%B_DVR_MEDIA_SEGMENT_SIZE);
    }
    else
    {
        recordService->mediaNode->mediaLinearEndOffset = mediaLinearOffset;
    }
    BDBG_MSG(("final mediaLinearOffset %lld",recordService->mediaNode->mediaLinearEndOffset));

    rc = B_DVR_SegmentedFileRecord_GetTimestamp(recordService->nexusFileRecord,
                                           recordService->mediaNode->mediaLinearEndOffset,
                                           &timeStamp);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s: B_DVR_SegmentedFileRecord_GetTimestamp failed",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    rc = B_DVR_SegmentedFileRecord_GetLocation(recordService->nexusFileRecord, 
                                          -1,
                                          timeStamp,
                                          &position);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s: B_DVR_SegmentedFileRecord_GetLocation failed",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    rc = B_DVR_SegmentedFileRecord_GetSettings(recordService->nexusFileRecord,
                                               &segmentedFileRecordSettings);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in B_DVR_SegmentedFileRecord_GetSettings for %s",
                  recordService->mediaNode->programName));
        goto error;
    }

    recordService->mediaNode->mediaEndTime = position.timestamp;
    recordService->mediaNode->navLinearEndOffset = position.navFileOffset 
                                                   + segmentedFileRecordSettings.navEntrySize;
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               recordService->mediaNode,
                               recordService->recordServiceRequest.volumeIndex,false);

    BDBG_MSG(("media time: recStart %ld recEnd %ld",
              recordService->mediaNode->mediaStartTime,
              recordService->mediaNode->mediaEndTime));
    BDBG_MSG(("media Offset: recStart %lld recEnd %lld",
              recordService->mediaNode->mediaLinearStartOffset,
              recordService->mediaNode->mediaLinearEndOffset));
    BDBG_MSG(("nav offset : recStart %lld recEnd %lld",
              recordService->mediaNode->navLinearStartOffset,
              recordService->mediaNode->navLinearEndOffset));

    segmentedFileRecordSettings.mediaLinearEndOffset = recordService->mediaNode->mediaLinearEndOffset;
    segmentedFileRecordSettings.navLinearEndOffset = recordService->mediaNode->navLinearEndOffset;
    rc = B_DVR_SegmentedFileRecord_SetSettings(recordService->nexusFileRecord,
                                               &segmentedFileRecordSettings);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in syncing the media & nav offsets after pausing the record service for %s",
                  recordService->mediaNode->programName));
    }
    recordService->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED;
error:
    BDBG_MSG(("B_DVR_RecordService_P_Pause<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_P_Resume(B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_RecordAppendSettings nexusRecordAppendSettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_P_Resume>>>"));
    recordService->updateMediaNodeEventSchedulerID =  B_Scheduler_RegisterEvent(dvrManager->scheduler,
                                                                 recordService->schedulerMutex,
                                                                 recordService->updateMediaNodeEvent,
                                                                 B_DVR_RecordService_P_UpdateMediaNode,
                                                                 (void*)recordService);
    if(!recordService->updateMediaNodeEventSchedulerID)
    {
        BDBG_ERR(("%s event registration  failed", __FUNCTION__));
        rc=B_DVR_OS_ERROR;
        goto error_event_registration;
    }

    nexusRecordAppendSettings.startingOffset = recordService->mediaNode->mediaLinearEndOffset;
    nexusRecordAppendSettings.timestamp = recordService->mediaNode->mediaEndTime;
    NEXUS_Record_StartAppend(recordService->record,
                             recordService->nexusFileRecord,
                             &nexusRecordAppendSettings);
    recordService->recpumpStarted = true;
    recordService->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED);
error_event_registration:
    BDBG_MSG(("B_DVR_RecordService_P_Resume<<<"));
    return rc;
}
bool B_DVR_RecordService_P_DetectVideoStart(const uint8_t *itbBuffer,bool mpeg2Video,unsigned long *videoStartOffset)
{
    bool detected = false;
    struct sct6 { unsigned word[6]; };
    const struct sct6 *sct = (const struct sct6 *)itbBuffer;
    if(mpeg2Video) 
    {
        uint8_t sc = sct->word[2] >> 24;
        detected = (sc == 0xB3)? true:false; 
    }
    else
    {
        unsigned scType = sct->word[0]>>24;
        detected = ((scType == 1) && ((sct->word[2] & 0x20) >> 5))?true:false;
    }
    if(detected) 
    {
        BDBG_WRN(("SCT6 %08x %08x %08x %08x %08x %08x",
                  sct->word[0],sct->word[1],
                  sct->word[2],sct->word[3],
                  sct->word[4],sct->word[5]));
        *videoStartOffset =  sct->word[3];
    }
    else
    {
        *videoStartOffset = 0;
    }
    return detected;
}

unsigned int B_DVR_RecordService_P_GetCurrentTimestamp(void)
{   
    struct timespec now;
    if(clock_gettime(CLOCK_MONOTONIC, &now)!=0) {
        return 0;
    } else {
        return now.tv_sec * 1000 + now.tv_nsec / 1000000;
    }
}

B_DVR_RecordServiceHandle B_DVR_RecordService_Open(
    B_DVR_RecordServiceRequest *recordServiceRequest)
{
    B_DVR_RecordServiceHandle recordService=NULL;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    BKNI_EventHandle recPumpEvent;
    NEXUS_RecpumpSettings recpumpSettings;
    B_DVR_SegmentedFileSettings segmentedFileSettings;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_RecordServiceInput  inputType = recordServiceRequest->input;
    unsigned index=0;

    BDBG_ASSERT(recordServiceRequest);
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("Enter recordService index = %d",dvrManager->recordServiceCount));
    recordService = BKNI_Malloc(sizeof(*recordService));
    if (!recordService)
    {
        BDBG_ERR(("%s  recordService alloc failed", __FUNCTION__));
        goto error_alloc1;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*recordService),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset(recordService, 0, sizeof(*recordService));
    BDBG_OBJECT_SET(recordService,B_DVR_RecordService);
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    /* find a recordService slot in the dvrManager */
    for(index=0;index<B_DVR_MAX_RECORDING;index++)
    {
        if(!dvrManager->recordService[index])
        {
            BDBG_MSG(("record Service Index %u",index));
            recordService->index=index;
            break;
        }
    }
    if(index>=B_DVR_MAX_RECORDING)
    {
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        BDBG_ERR(("MAX recordService instances used up. Free up a recordService instance"));
        goto error_maxRecordServiceCount;
    }
    dvrManager->recordServiceCount++;
    dvrManager->recordService[recordService->index] = recordService;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    recordService->mediaNode = BKNI_Malloc(sizeof(*recordService->mediaNode));
    if (!recordService->mediaNode)
    {
        BDBG_ERR(("%s  recordService mediaNode alloc failed", __FUNCTION__));
        goto error_alloc2;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*recordService->mediaNode),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset((void*)recordService->mediaNode, 0, sizeof(*recordService->mediaNode));
    BDBG_OBJECT_SET(recordService->mediaNode,B_DVR_Media);
    recordService->mediaNode->recording = eB_DVR_RecordingPermanent;
    recordService->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM;
    recordService->mediaNode->transportType = NEXUS_TransportType_eTs;
    recordService->overflow = false;
    
    BKNI_Memcpy((void*)&recordService->recordServiceRequest,(void *)recordServiceRequest, sizeof(*recordServiceRequest));
    recordService->recordMutex = B_Mutex_Create(NULL);
    if( !recordService->recordMutex)
    {
        BDBG_ERR(("%s mutex create failed", __FUNCTION__));
        goto error_mutex;
    }

    recordService->schedulerMutex = B_Mutex_Create(NULL);
    if( !recordService->schedulerMutex)
    {
        BDBG_ERR(("%s schedulerMutex create error", __FUNCTION__));
        goto error_schedulerMutex;
    }

    if(inputType == eB_DVR_RecordServiceInputQam ||
       inputType == B_DVR_RecordServiceInputTranscode ||
       inputType == B_DVR_RecordServiceInputIp)
    {
       recordService->updateMediaNodeEvent = B_Event_Create(NULL);
       if(!recordService->updateMediaNodeEvent )
       {
         BDBG_ERR(("%s event create failed", __FUNCTION__));
         goto error_event;
       }

       NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
       if(recordServiceRequest->recpumpCdbHeap) 
       {
           recpumpOpenSettings.data.heap = recordServiceRequest->recpumpCdbHeap;
       }
       if(recordServiceRequest->recpumpIndexHeap) 
       {
           recpumpOpenSettings.index.heap = recordServiceRequest->recpumpIndexHeap;
       }
       if( (recordServiceRequest->input == B_DVR_RecordServiceInputTranscode) ||
           (recordServiceRequest->input == B_DVR_RecordServiceInputIp))
       { 
           #ifdef TRANSCODE_SUPPORT
           /* for transcode and IP input recpump band hold is enabled */
           if(recordServiceRequest->input == B_DVR_RecordServiceInputTranscode)
           {
               recordService->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_TRANSCODED_STREAM;
               recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.dataReadyThreshold*11;
               recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.dataReadyThreshold*11;
           }
           else
           #endif
           {
               recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize*8/10;
               recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize*8/10;
           }
       }
       else
       {
           recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.dataReadyThreshold*4;
           recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.dataReadyThreshold*4;
       }
       recordService->recpump = NEXUS_Recpump_Open(recordServiceRequest->recpumpIndex,&recpumpOpenSettings);
       if(!recordService->recpump)
       {
         BDBG_ERR(("%s recpump open failed %d", __FUNCTION__,dvrManager->recordServiceCount));
         goto error_recpump;
       }

       B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                               recpumpOpenSettings.data.bufferSize,
                                               true, __FUNCTION__,__LINE__);
       B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                               recpumpOpenSettings.index.bufferSize,
                                               true, __FUNCTION__,__LINE__);

       BDBG_MSG(("CDB size %u threshold %u",recpumpOpenSettings.data.bufferSize,recpumpOpenSettings.data.dataReadyThreshold));
       BDBG_MSG(("ITB size %u threshold %u",recpumpOpenSettings.index.bufferSize,recpumpOpenSettings.index.dataReadyThreshold));

       recordService->record = NEXUS_Record_Create();
       if(!recordService->record)
       {
         BDBG_ERR(("%s record create failed %d", __FUNCTION__,dvrManager->recordServiceCount));
         goto error_record;
       }

       BKNI_Snprintf(recordService->mediaNode->navFileName, sizeof(recordService->mediaNode->navFileName),"%s%s",
                   recordServiceRequest->programName,B_DVR_NAVIGATION_FILE_EXTENTION);
       BKNI_Snprintf(recordService->mediaNode->mediaFileName,sizeof(recordService->mediaNode->mediaFileName),"%s%s",
                  recordServiceRequest->programName,B_DVR_MEDIA_FILE_EXTENTION);
       BKNI_Snprintf(recordService->mediaNode->mediaNodeFileName,sizeof(recordService->mediaNode->mediaNodeFileName),"%s%s",
                  recordServiceRequest->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);

       strncpy(recordService->mediaNode->programName,recordServiceRequest->programName,B_DVR_MAX_FILE_NAME_LENGTH);
       recordService->mediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';

       BDBG_MSG(("media %s, nav %s mediainfo %s",recordService->mediaNode->mediaFileName,
              recordService->mediaNode->navFileName,recordService->mediaNode->mediaNodeFileName));

       if(recordServiceRequest->subDir[0] != '\0')
       { 
         strncpy(recordService->mediaNode->mediaNodeSubDir,recordServiceRequest->subDir,B_DVR_MAX_FILE_NAME_LENGTH);
         recordService->mediaNode->mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
       }
       #ifdef MEDIANODE_ONDEMAND_CACHING
       rc = B_DVR_List_AddMediaNodeFile(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
       if(rc!=B_DVR_SUCCESS)
       {
         BDBG_MSG(("error in creating %s",recordService->mediaNode->mediaNodeFileName));
         goto error_add_media;
       }
       #endif
       rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
       if(rc!=B_DVR_SUCCESS)
       {
           BDBG_MSG(("error in adding mediaNode for %s to the dvr list",recordService->mediaNode->mediaNodeFileName));
           #ifdef MEDIANODE_ONDEMAND_CACHING
           B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
           #endif
         goto error_add_media;
       }
       recordService->mediaNodeSyncCount=1;
       segmentedFileSettings.registeredCallback = B_DVR_RecordService_P_SegmentedFileEvent;
       segmentedFileSettings.volumeIndex = recordService->recordServiceRequest.volumeIndex;
       segmentedFileSettings.mediaSegmentSize = dvrManager->mediaSegmentSize;
       segmentedFileSettings.serviceIndex = recordService->index;
       segmentedFileSettings.service = eB_DVR_ServiceRecord;
       segmentedFileSettings.maxSegmentCount = 0;
       segmentedFileSettings.event = recordService->updateMediaNodeEvent;
       segmentedFileSettings.itbThreshhold = recpumpOpenSettings.index.dataReadyThreshold;
       segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
       segmentedFileSettings.maxSegmentCount = 0;
       segmentedFileSettings.cdbSize = recpumpOpenSettings.data.bufferSize;
       segmentedFileSettings.itbSize = recpumpOpenSettings.index.bufferSize;
       if(recordService->mediaNode->mediaNodeSubDir[0]=='\0')
       {
         segmentedFileSettings.metaDataSubDir = NULL;
       }
       else
       {
         segmentedFileSettings.metaDataSubDir = (char *) recordService->mediaNode->mediaNodeSubDir;
       }
       recordService->nexusFileRecord = B_DVR_SegmentedFileRecord_Open(recordService->mediaNode->mediaFileName,
                                                                 recordService->mediaNode->navFileName,
                                                                 &segmentedFileSettings);
       BDBG_MSG(("segment size %d",segmentedFileSettings.mediaSegmentSize));

       if(!recordService->nexusFileRecord)
       {
         BDBG_ERR(("nexusFileRecord open failed %s %s",recordService->mediaNode->mediaFileName,recordService->mediaNode->navFileName));
         rc = B_DVR_UNKNOWN;
         goto error_nexusFileRecord;
       }
       NEXUS_Record_GetSettings(recordService->record, &recordSettings);
       recordSettings.recpump = recordService->recpump;
       recordSettings.allowLargeTimeGaps = true;
       recordSettings.pollingTimer = B_DVR_RECORD_POLLING_INTERVAL/5; /* 50ms initially*/
       recordSettings.overflowCallback.callback = B_DVR_RecordService_P_Overflow;
       recordSettings.overflowCallback.context = recordService;
       recordSettings.overflowCallback.param = recordService->index;
       if ((recordServiceRequest->input == B_DVR_RecordServiceInputTranscode) ||
           (recordServiceRequest->input == B_DVR_RecordServiceInputIp))
       {
           recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
       }
       NEXUS_Record_SetSettings(recordService->record, &recordSettings);
    }
    else if(inputType == B_DVR_RecordServiceInputStreamingVOD)
    {
       BDBG_MSG(("Enter recordService StreamingVOD Start"));
       
       BKNI_CreateEvent(&recPumpEvent);

       recordService->recPumpEvent = recPumpEvent;

       NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
       if(recordServiceRequest->recpumpCdbHeap) 
       {
           recpumpOpenSettings.data.heap = recordServiceRequest->recpumpCdbHeap;
       }
       if(recordServiceRequest->recpumpIndexHeap) 
       {
           recpumpOpenSettings.index.heap = recordServiceRequest->recpumpIndexHeap;
       }
       recordService->recpump = NEXUS_Recpump_Open(recordServiceRequest->recpumpIndex, &recpumpOpenSettings);

       if(!recordService->recpump)
       {
         BDBG_ERR(("%s recpump open failed %d", __FUNCTION__,dvrManager->recordServiceCount));
         goto error_recpumpath;
       }

       BDBG_MSG(("Enter recordService StreamingVOD Successful"));

       if(recordServiceRequest->programName[0] == '\0')
       {
          NEXUS_RecpumpStatus recpumpStatus;
          NEXUS_Recpump_GetStatus(recordService->recpump,&recpumpStatus);
          BKNI_Snprintf(recordServiceRequest->programName,sizeof(recordServiceRequest->programName),"%s%d%d","StreamingVOD",recordServiceRequest->recpumpIndex, recpumpStatus.rave.index);
       }
       BDBG_MSG(("recordService programname= %s",recordServiceRequest->programName));
       
       BKNI_Snprintf(recordService->mediaNode->mediaNodeFileName,sizeof(recordService->mediaNode->mediaNodeFileName),"%s%s",
                  recordServiceRequest->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);

       strncpy(recordService->mediaNode->programName,recordServiceRequest->programName,B_DVR_MAX_FILE_NAME_LENGTH);
       recordService->mediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
           
       BDBG_MSG(("mediainfo %s",recordService->mediaNode->mediaNodeFileName));

       if(recordServiceRequest->subDir[0] != '\0')
       { 
         BDBG_MSG(("Enter recordService StreamingVOD --- Copy sub dir"));
         strncpy(recordService->mediaNode->mediaNodeSubDir,recordServiceRequest->subDir,B_DVR_MAX_FILE_NAME_LENGTH);
         recordService->mediaNode->mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
       }
       #ifdef MEDIANODE_ONDEMAND_CACHING
       rc = B_DVR_List_AddMediaNodeFile(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
       if(rc!=B_DVR_SUCCESS)
       {
          BDBG_MSG(("error in creating %s for VOD streaming",recordService->mediaNode->mediaNodeFileName));
          NEXUS_Recpump_Close(recordService->recpump);
          goto error_recpumpath;
       }
       #endif
       rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
       if(rc!=B_DVR_SUCCESS)
       {
          BDBG_MSG(("error in adding mediaNode for %s to the dvr list for VOD streaming",
                    recordService->mediaNode->mediaNodeFileName));
          #ifdef MEDIANODE_ONDEMAND_CACHING
          B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,recordServiceRequest->volumeIndex,recordService->mediaNode);
          #endif
          NEXUS_Recpump_Close(recordService->recpump);
          goto error_recpumpath;
       }
       BDBG_MSG(("Before NEXUS_Recpump_GetSettings"));
       NEXUS_Recpump_GetSettings(recordService->recpump, &recpumpSettings);
       recpumpSettings.data.dataReady.callback = dataready_callback;
       recpumpSettings.data.dataReady.context = recordService->recPumpEvent;
       recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
       recpumpSettings.index.dataReady.context = recordService->recPumpEvent; /* same event */
       recpumpSettings.data.overflow.callback = overflow_callback;
       recpumpSettings.data.overflow.context = recordService;
       recpumpSettings.index.overflow.callback = overflow_callback;
       recpumpSettings.index.overflow.context = recordService;
       NEXUS_Recpump_SetSettings(recordService->recpump, &recpumpSettings);
       BDBG_MSG(("After NEXUS_Recpump_GetSettings"));    
    }
    else
    {
       BDBG_ERR(("B_DVR_RecordService_Open error <<<"));
    }   

    BDBG_MSG(("B_DVR_RecordService_Open <<<"));
    return recordService;

error_nexusFileRecord:
error_add_media:
    NEXUS_Record_Destroy(recordService->record);
error_record:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.data.bufferSize,
                                            false, __FUNCTION__,__LINE__);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.index.bufferSize,
                                            false, __FUNCTION__,__LINE__);
    NEXUS_Recpump_Close(recordService->recpump);
error_recpump:
    B_Event_Destroy(recordService->updateMediaNodeEvent);
error_recpumpath:
error_event:
    B_Mutex_Destroy(recordService->schedulerMutex);
error_schedulerMutex:
    B_Mutex_Destroy(recordService->recordMutex);
error_mutex:
    BDBG_OBJECT_DESTROY(recordService->mediaNode,B_DVR_Media);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*recordService->mediaNode),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(recordService->mediaNode);
error_alloc2:
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->recordServiceCount--;
    dvrManager->recordService[recordService->index] = NULL;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
error_maxRecordServiceCount:
    BDBG_OBJECT_DESTROY(recordService,B_DVR_RecordService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*recordService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(recordService);    
error_alloc1:
    BDBG_MSG(("B_DVR_RecordService_Open error <<<"));
    return NULL;
}

B_DVR_ERROR B_DVR_RecordService_Close(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned esStreamIndex=0;   
    NEXUS_RecpumpStatus recpumpStatus;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_Close >>>"));

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->recordService[recordService->index] = NULL;
    dvrManager->recordServiceCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(recordService->recordMutex);
    NEXUS_Recpump_GetStatus(recordService->recpump,&recpumpStatus);
    switch (inputType)
    {
       case eB_DVR_RecordServiceInputQam:
       case B_DVR_RecordServiceInputTranscode:
         recordService->mediaNodeSyncCount=1;
         B_DVR_SegmentedFileRecord_Close(recordService->nexusFileRecord);
         NEXUS_Record_RemoveAllPidChannels(recordService->record);
         if( (recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam) ||
             (recordService->recordServiceRequest.input == B_DVR_RecordServiceInputIp) ) 
         { 
            for(esStreamIndex=0;esStreamIndex<recordService->activeEsStreamCount;esStreamIndex++)
            {
                if(recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam)
                {
                    NEXUS_PidChannel_Close(recordService->recordPidChannels[esStreamIndex]);
                }
                else
                {
                    NEXUS_Playpump_ClosePidChannel(recordService->settings.playpumpIp,
                                                       recordService->recordPidChannels[esStreamIndex]);
                 }
             }
         }
         NEXUS_Record_Destroy(recordService->record);
         B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                                 recpumpStatus.openSettings.data.bufferSize,
                                                 false, __FUNCTION__,__LINE__);
         B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                                 recpumpStatus.openSettings.index.bufferSize,
                                                 false, __FUNCTION__,__LINE__);
         NEXUS_Recpump_Close(recordService->recpump);
         B_Event_Destroy(recordService->updateMediaNodeEvent);
         #ifdef MEDIANODE_ONDEMAND_CACHING
         if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                         recordService->recordServiceRequest.volumeIndex,
                                         recordService->mediaNode)) 
         {
             BDBG_OBJECT_DESTROY(recordService->mediaNode,B_DVR_Media);
             B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                     sizeof(*recordService->mediaNode),
                                                     false,__FUNCTION__,__LINE__);
             BKNI_Free(recordService->mediaNode);
         }
         #endif
         break;
       case B_DVR_RecordServiceInputStreamingVOD:
         for(esStreamIndex=0;esStreamIndex<recordService->activeEsStreamCount;esStreamIndex++)
         {
            NEXUS_PidChannel_Close(recordService->recordPidChannels[esStreamIndex]);
         }
         B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                                 recpumpStatus.openSettings.data.bufferSize,
                                                 false, __FUNCTION__,__LINE__);
         B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                                 recpumpStatus.openSettings.index.bufferSize,
                                                 false, __FUNCTION__,__LINE__);
         NEXUS_Recpump_Close(recordService->recpump);
         BKNI_DestroyEvent(recordService->recPumpEvent);
         #ifdef MEDIANODE_ONDEMAND_CACHING
         B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,recordService->recordServiceRequest.volumeIndex,recordService->mediaNode);
         #endif
         if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                        recordService->recordServiceRequest.volumeIndex,
                                        recordService->mediaNode)) 
         {
             BDBG_OBJECT_DESTROY(recordService->mediaNode,B_DVR_Media);
             B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                     sizeof(*recordService->mediaNode),
                                                     false,__FUNCTION__,__LINE__);
             BKNI_Free(recordService->mediaNode);
         }
         BDBG_MSG(("recordService StreamingVOD ---remove mediaNode "));      
         break;
       default:
         BDBG_MSG(("B_DVR_RecordService_Close:: RecordServiceInputError"));
         break;
    }
    B_Mutex_Destroy(recordService->schedulerMutex);
    B_Mutex_Unlock(recordService->recordMutex);
    B_Mutex_Destroy(recordService->recordMutex);
    BDBG_OBJECT_DESTROY(recordService,B_DVR_RecordService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*recordService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(recordService);
    BDBG_MSG(("B_DVR_RecordService_Close <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_Start(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceSettings *recordServiceSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_SegmentedFileRecordSettings fileRecordSettings;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;    
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();

    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(recordService->activeEsStreamCount);
    BDBG_MSG(("B_DVR_RecordService_Start >>>> index %d",recordService->index));
    B_Mutex_Lock(recordService->recordMutex);
    if((inputType == eB_DVR_RecordServiceInputQam)  || 
       (inputType == B_DVR_RecordServiceInputTranscode) ||
       (inputType == B_DVR_RecordServiceInputIp))
    {
        recordService->updateMediaNodeEventSchedulerID =  B_Scheduler_RegisterEvent(dvrManager->scheduler,
                                                                                    recordService->schedulerMutex,
                                                                                    recordService->updateMediaNodeEvent,
                                                                                    B_DVR_RecordService_P_UpdateMediaNode,
                                                                                    (void*)recordService);
        if(!recordService->updateMediaNodeEventSchedulerID)
        {
            BDBG_ERR(("%s event registration  failed", __FUNCTION__));
            rc = B_DVR_OS_ERROR;
            goto error_event_registration;
        }
    }
    if(recordServiceSettings)
    {
        unsigned esStreamIndex=0;
        NEXUS_PidChannelRemapSettings remapSettings;
        BDBG_ASSERT(recordServiceSettings->esStreamCount == recordService->mediaNode->esStreamCount);
        for (esStreamIndex=0;esStreamIndex<recordServiceSettings->esStreamCount;esStreamIndex++) 
        {
            remapSettings.enabled = true;
            remapSettings.continuityCountEnabled = true;
            remapSettings.pid = recordServiceSettings->RemapppedEsStreamInfo[esStreamIndex].pid;
            NEXUS_PidChannel_SetRemapSettings(recordService->recordPidChannels[esStreamIndex],&remapSettings);
            recordService->mediaNode->esStreamInfo[esStreamIndex].pid = remapSettings.pid;
        }
    }

    if(inputType == B_DVR_RecordServiceInputStreamingVOD)
    {
       NEXUS_Recpump_Start(recordService->recpump);
       recordService->recpumpStarted = true;
       recordService->mediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);  
       recordService->mediaNode->mediaStartTime = B_DVR_RecordService_P_GetCurrentTimestamp();
       recordService->mediaNode->mediaLinearStartOffset = 0;
       recordService->mediaNode->mediaEndTime = B_DVR_RecordService_P_GetCurrentTimestamp();
       recordService->mediaNode->mediaLinearEndOffset = 0; 
       BDBG_MSG(("VOD start time %u end time %u",recordService->mediaNode->mediaStartTime,recordService->mediaNode->mediaEndTime));
    }
    else
    {
       B_DVR_SegmentedFileRecord_GetDefaultSettings(&fileRecordSettings);
       if(recordService->mpeg2Video)
       { 
          fileRecordSettings.navEntrySize = sizeof(BNAV_Entry);
       }
       else
       {
          fileRecordSettings.navEntrySize = sizeof(BNAV_AVC_Entry);
       }
       B_DVR_SegmentedFileRecord_SetSettings(recordService->nexusFileRecord,&fileRecordSettings);

       recordService->mediaProbe = B_DVR_MediaProbe_Open(recordService->mediaNode,
                                                         recordService->nexusFileRecord,
                                                         recordService->recordServiceRequest.volumeIndex,
                                                         recordService->index);
       if(!recordService->mediaProbe)
       {
           BDBG_ERR(("mediaProbe open failed"));
       }
       NEXUS_Record_Start(recordService->record,recordService->nexusFileRecord);
       recordService->recpumpStarted = true;
       recordService->mediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
    }
    B_DVR_RecordService_P_PrintRecInfo(recordService);
error_event_registration:
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_Start <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_Stop(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_FilePosition startPosition,endPosition;
    B_DVR_Event event;
    B_DVR_Service service;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_Stop >>> index %d",recordService->index));

    B_Mutex_Lock(recordService->schedulerMutex);
    if(recordService->updateMediaNodeEventSchedulerID) 
    {
        B_Scheduler_UnregisterEvent(dvrManager->scheduler,recordService->updateMediaNodeEventSchedulerID);
    }
    if(recordService->updateMediaNodeEvent) 
    {
        B_Event_Reset(recordService->updateMediaNodeEvent);
    }
    B_Mutex_Unlock(recordService->schedulerMutex);
    if(recordService->dataInjectionStarted)
    {
        B_DVR_RecordService_InjectDataStop(recordService);
    }

    B_Mutex_Lock(recordService->recordMutex);
    recordService->updateMediaNodeEventSchedulerID = NULL;
    if(inputType == B_DVR_RecordServiceInputStreamingVOD)
    {
       NEXUS_Recpump_Stop(recordService->recpump);
       recordService->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
       B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                  recordService->mediaNode,
                                  recordService->recordServiceRequest.volumeIndex,false);
       recordService->recpumpStarted = false;
    }
    else
    {
       NEXUS_Record_Stop(recordService->record);
       if(recordService->mediaProbe)
       {
           B_DVR_MediaProbe_Close(recordService->mediaProbe);
           recordService->mediaProbe = NULL;
           recordService->probeStartOffset = 0;
       }
       B_DVR_SegmentedFileRecord_GetBounds(recordService->nexusFileRecord,&startPosition,&endPosition);
       recordService->mediaNode->mediaLinearStartOffset = startPosition.mpegFileOffset;
       recordService->mediaNode->mediaLinearEndOffset =  endPosition.mpegFileOffset;
       recordService->mediaNode->navLinearStartOffset = startPosition.navFileOffset;
       recordService->mediaNode->navLinearEndOffset = endPosition.navFileOffset;
       recordService->mediaNode->mediaStartTime = startPosition.timestamp;
       recordService->mediaNode->mediaEndTime = endPosition.timestamp;
       recordService->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
       B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                  recordService->mediaNode,
                                  recordService->recordServiceRequest.volumeIndex,true);
       recordService->recordStarted = false;
       recordService->recpumpStarted = false;
       if(recordService->inProgressRecPlay) 
       {
           NEXUS_Record_RemovePlayback(recordService->record,recordService->inProgressRecPlay);
           recordService->inProgressRecPlay = NULL;
       }
       recordService->updatePlaybackEvent = NULL;
    }
    B_DVR_RecordService_P_PrintRecInfo(recordService);
    B_Mutex_Unlock(recordService->recordMutex);

    if(recordService->registeredCallback)
    {
        event = eB_DVR_EventEndOfRecording;
        service = eB_DVR_ServiceRecord;
        recordService->registeredCallback(recordService->appContext,recordService->index,event,service);
    }
    BDBG_MSG(("B_DVR_RecordService_Stop <<<"));
    return rc;
}

ssize_t B_DVR_RecordService_Read(
    B_DVR_RecordServiceHandle recordService,
    unsigned *buffer, ssize_t size)
{
    ssize_t returnSize=0;
    const void *data_buffer, *index_buffer;
    size_t data_buffer_size=0,index_buffer_size=0;
    unsigned long videoStartOffset=0;
    unsigned long currentCDBStartOffset=0;
    NEXUS_RecpumpStatus recpumpStatus;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_MSG(("B_DVR_RecordService_Read >>>> "));
    B_Mutex_Lock(recordService->recordMutex);
    while(1)
    {
        if (NEXUS_Recpump_GetDataBuffer(recordService->recpump, &data_buffer, &data_buffer_size))
        {
           BDBG_WRN(("B_DVR_RecordService_Read -- Failed on Recpump_GetDataBuffer"));
           break;
        }

        if (NEXUS_Recpump_GetIndexBuffer(recordService->recpump, &index_buffer, &index_buffer_size))
        {
           BDBG_WRN(("B_DVR_RecordService_Read -- Failed on Recpump_GetIndexBuffer"));
           break;
        }

        if (data_buffer_size == 0 && index_buffer_size == 0) 
        {
            /*BDBG_WRN(("B_DVR_RecordService_Read --- Waitforevent timeout in 1s"));*/
            BKNI_WaitForEvent(recordService->recPumpEvent, 1000);
            if (NEXUS_Recpump_GetDataBuffer(recordService->recpump, &data_buffer, &data_buffer_size))
            {
                BDBG_WRN(("B_DVR_RecordService_Read -- Failed on Recpump_GetDataBuffer"));
                break;
            }
            if (NEXUS_Recpump_GetIndexBuffer(recordService->recpump, &index_buffer, &index_buffer_size))
            {
                BDBG_WRN(("B_DVR_RecordService_Read -- Failed on Recpump_GetIndexBuffer"));
                break;
            }
            if (data_buffer_size == 0 && index_buffer_size == 0) 
            {
                BDBG_WRN(("B_DVR_RecordService_Read -- Time out without data..."));
                break;
            }
        }

        BDBG_MSG(("B_DVR_RecordService_Read --- Gotdata data_buffer_size=%u index_buffer_size=%u",data_buffer_size,index_buffer_size));
        
        if (index_buffer_size) 
        {
            if(!recordService->recordStarted) 
            {
                unsigned sctSize = SCT6_SIZE;
                unsigned sctCount = index_buffer_size/sctSize;
                bool detected = false;
                const uint8_t *itbBuffer = index_buffer;
                for(;!detected && sctCount>0;sctCount--, itbBuffer += sctSize) 
                {
                   detected = B_DVR_RecordService_P_DetectVideoStart(itbBuffer,
                                                                     recordService->mpeg2Video,
                                                                     &videoStartOffset);
                }
                recordService->recordStarted = detected?true:false;
            }
            NEXUS_Recpump_IndexReadComplete(recordService->recpump, index_buffer_size);
            if(!recordService->recordStarted) 
            {
                BDBG_WRN(("Decoder friendly start offset isnt available in the data read from CDB"));
                NEXUS_Recpump_DataReadComplete(recordService->recpump,data_buffer_size);
                currentCDBStartOffset += data_buffer_size;
                continue;
            }
            BDBG_MSG(("B_DVR_RecordService_Read index_buffer_size = %u ",index_buffer_size));
        }
        if (data_buffer_size) 
        {
            const uint8_t *cdbBuffer = data_buffer;
            if(videoStartOffset) 
            {   
                videoStartOffset -= currentCDBStartOffset;
                data_buffer_size -= videoStartOffset;
                cdbBuffer  += videoStartOffset;
            }
           if (data_buffer_size <= (size_t)size)
               returnSize = (ssize_t)data_buffer_size;
           else
               returnSize = size;
           BDBG_MSG(("B_DVR_RecordService_Read readsize=%u inputbuffersize=%ld videoStartOffset=%u ",
                     data_buffer_size,size,videoStartOffset));
           BKNI_Memcpy((void *)buffer,(void *)cdbBuffer,returnSize);
           NEXUS_Recpump_DataReadComplete(recordService->recpump, (size_t)returnSize + (size_t)videoStartOffset);
           BDBG_MSG(("B_DVR_RecordService_Read returnsize = %ld ",returnSize));
           recordService->mediaNode->mediaLinearEndOffset += (off_t)returnSize; 
           recordService->mediaNode->mediaEndTime = B_DVR_RecordService_P_GetCurrentTimestamp();
           NEXUS_Recpump_GetStatus(recordService->recpump, &recpumpStatus);
           recordService->mediaNode->maxStreamBitRate = 
               B_DVR_RecordService_P_CalculateBitRate(0,recpumpStatus.data.bytesRecorded,
                                                      recordService->mediaNode->mediaStartTime,
                                                      recordService->mediaNode->mediaEndTime);
           BDBG_MSG(("VOD read  bytesRecorded %lld bitrate %u bps",
                     recpumpStatus.data.bytesRecorded,
                     recordService->mediaNode->maxStreamBitRate));
        }
        break;
    }
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_Read <<<< "));
    return returnSize;
}

B_DVR_ERROR B_DVR_RecordService_GetStatus(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceStatus *pStatus)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_ASSERT(pStatus);
    B_Mutex_Lock(recordService->recordMutex);
    pStatus->startTime = recordService->mediaNode->mediaStartTime;
    pStatus->linearStartOffset = recordService->mediaNode->mediaLinearStartOffset;
    pStatus->currentTime = recordService->mediaNode->mediaEndTime;
    pStatus->linearCurrentOffset = recordService->mediaNode->mediaLinearEndOffset;
    B_Mutex_Unlock(recordService->recordMutex);
    return rc;
}

B_DVR_ERROR  B_DVR_RecordService_GetSettings(
    B_DVR_RecordServiceHandle recordService, 
    B_DVR_RecordServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_ASSERT(pSettings);
    BKNI_Memcpy((void *)pSettings,(void *)&recordService->settings,sizeof(*pSettings));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_SetSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_ASSERT(pSettings);
    
    if(!recordService->recpumpStarted && pSettings->esStreamCount)
    { 
        unsigned esStreamIndex=0;
        NEXUS_PidChannelRemapSettings remapSettings;
        BDBG_ASSERT(pSettings->esStreamCount == recordService->mediaNode->esStreamCount);
        for (esStreamIndex=0;esStreamIndex<pSettings->esStreamCount;esStreamIndex++) 
        {
            remapSettings.enabled = true;
            remapSettings.continuityCountEnabled = true;
            remapSettings.pid = pSettings->RemapppedEsStreamInfo[esStreamIndex].pid;
            NEXUS_PidChannel_SetRemapSettings(recordService->recordPidChannels[esStreamIndex],&remapSettings);
            recordService->mediaNode->esStreamInfo[esStreamIndex].pid = remapSettings.pid;
        }
    
    }
    recordService->settings.parserBand = pSettings->parserBand;
    recordService->settings.dataInjectionService = pSettings->dataInjectionService;
    recordService->settings.playpumpIp = pSettings->playpumpIp;
    #ifdef TRANSCODE_SUPPORT
    recordService->settings.transcodeService = pSettings->transcodeService;
    #endif
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_InstallCallback(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_ServiceCallback registeredCallback,
    void *appContext)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    B_Mutex_Lock(recordService->recordMutex);
    recordService->registeredCallback = registeredCallback;
    recordService->appContext = appContext;
    B_Mutex_Unlock(recordService->recordMutex);
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_RemoveCallback(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    B_Mutex_Lock(recordService->recordMutex);
    recordService->registeredCallback=NULL;
    B_Mutex_Unlock(recordService->recordMutex);
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_AddInputEsStream(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceInputEsStream *inputEsStream)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode mediaNode=NULL;
    NEXUS_RecordPidChannelSettings pidSettings;
    unsigned esStreamCount,activeEsStreamCount; 
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;
    B_DVR_EsStreamInfo *esStreamInfo=NULL;
    bool setRapTpit = false;
    
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(inputEsStream);
    BDBG_MSG(("B_DVR_RecordService_AddEsStream >>> "));
    B_Mutex_Lock(recordService->recordMutex);
    mediaNode = recordService->mediaNode;
    esStreamInfo = &inputEsStream->esStreamInfo;
    esStreamCount = mediaNode->esStreamCount;
    activeEsStreamCount = recordService->activeEsStreamCount;
    BKNI_Memcpy((void*)&mediaNode->esStreamInfo[esStreamCount],(void *)&inputEsStream->esStreamInfo,sizeof(B_DVR_EsStreamInfo));
    BKNI_Memcpy((void*)&recordService->activeEsStreamInfo[activeEsStreamCount],(void *)&inputEsStream->esStreamInfo,sizeof(B_DVR_EsStreamInfo));

    if(inputType == B_DVR_RecordServiceInputStreamingVOD)
     {
        if(esStreamInfo->pidType == eB_DVR_PidTypeVideo)
        {
           NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
           addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
           addPidChannelSettings.pidTypeSettings.video.index = true;
           addPidChannelSettings.pidTypeSettings.video.codec = esStreamInfo->codec.videoCodec;
           recordService->mpeg2Video = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?true:false;
           setRapTpit = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?false:true;
        }

        if(inputEsStream->pidChannel)
        {
            recordService->recordPidChannels[activeEsStreamCount] = inputEsStream->pidChannel;
        }
        else
        {
             recordService->recordPidChannels[activeEsStreamCount] = NEXUS_PidChannel_Open(recordService->settings.parserBand,
                                                                                           esStreamInfo->pid,
                                                                                           NULL);
        }

        if(esStreamInfo->pidType != eB_DVR_PidTypeVideo)
        {
           NEXUS_Recpump_AddPidChannel(recordService->recpump,recordService->recordPidChannels[activeEsStreamCount],NULL);
        }
        else
        {
           NEXUS_Recpump_AddPidChannel(recordService->recpump,recordService->recordPidChannels[activeEsStreamCount],&addPidChannelSettings);
        }
    
     }
     else
     {
        if(esStreamInfo->pidType == eB_DVR_PidTypeVideo)
        {
           NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
           pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
           pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
           pidSettings.recpumpSettings.pidTypeSettings.video.codec = esStreamInfo->codec.videoCodec;
           recordService->mpeg2Video = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?true:false;
           setRapTpit = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?false:true;
        }

        if(inputEsStream->pidChannel)
        {
            recordService->recordPidChannels[activeEsStreamCount] = inputEsStream->pidChannel;
        }
        else
        {
            if( (recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam) || 
                (recordService->recordServiceRequest.input == B_DVR_RecordServiceInputIp)) 
            {
                if(recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam)
                {
                    recordService->recordPidChannels[activeEsStreamCount] = 
                        NEXUS_PidChannel_Open(recordService->settings.parserBand,esStreamInfo->pid,NULL);
                }
                else
                {
                    recordService->recordPidChannels[activeEsStreamCount] = 
                        NEXUS_Playpump_OpenPidChannel(recordService->settings.playpumpIp,esStreamInfo->pid,NULL);
                }
            }
            else
            {
                BDBG_ERR(("PID channel open failed"));
                rc = B_DVR_UNKNOWN;
                goto error;
            }
       }
     
        if(esStreamInfo->pidType != eB_DVR_PidTypeVideo)
        {
           NEXUS_Record_AddPidChannel(recordService->record,recordService->recordPidChannels[activeEsStreamCount],NULL);
        }
        else
        {
           NEXUS_Record_AddPidChannel(recordService->record,recordService->recordPidChannels[activeEsStreamCount],&pidSettings);
        }   
    }
    BDBG_MSG(("pid %d pidT %d codec %d pidchannel %d",esStreamInfo->pid,esStreamInfo->pidType,
              esStreamInfo->codec, recordService->recordPidChannels[activeEsStreamCount]));

    if(!recordService->recordPidChannels[activeEsStreamCount])
    {
        BDBG_ERR(("PID channel open failed"));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    if (setRapTpit) 
    {
        NEXUS_RecpumpTpitFilter filter;
        NEXUS_Recpump_GetDefaultTpitFilter(&filter);
        filter.config.mpeg.randomAccessIndicatorEnable = true;
        filter.config.mpeg.randomAccessIndicatorCompValue = true;
        NEXUS_Recpump_SetTpitFilter(recordService->recpump, recordService->recordPidChannels[activeEsStreamCount], &filter);
    }

    mediaNode->esStreamCount++;
    recordService->activeEsStreamCount++;
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               recordService->mediaNode,
                               recordService->recordServiceRequest.volumeIndex,true);
error:
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_AddEsStream <<< "));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_RemoveInputEsStream(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_RecordServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned esStreamIndex=0;
    B_DVR_EsStreamInfo *esStreamInfo = NULL;
    bool pidFound = false;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_MSG(("B_DVR_RecordService_RemoveEsStream >>> "));
    B_Mutex_Lock(recordService->recordMutex);
    esStreamInfo = &inputEsStream->esStreamInfo;
    for(esStreamIndex=0;esStreamIndex < recordService->activeEsStreamCount;esStreamIndex++)
    {
        if(recordService->activeEsStreamInfo[esStreamIndex].pid == esStreamInfo->pid
           && recordService->activeEsStreamInfo[esStreamIndex].pidType == esStreamInfo->pidType)
        {
            pidFound = true;
            break;
        }
    }

    if(pidFound)
    { 
        B_DVR_EsStreamInfo esStreamInfo;
        if(inputType == B_DVR_RecordServiceInputStreamingVOD)
        {
           NEXUS_Recpump_RemovePidChannel(recordService->recpump,recordService->recordPidChannels[esStreamIndex]);
        }
        else
        {
           NEXUS_Record_RemovePidChannel(recordService->record,recordService->recordPidChannels[esStreamIndex]);
        }
        if(!inputEsStream->pidChannel)
        { 
            if( (recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam) || 
                (recordService->recordServiceRequest.input == B_DVR_RecordServiceInputIp) ) 
            {

                if(recordService->recordServiceRequest.input == eB_DVR_RecordServiceInputQam)
                {
                    NEXUS_PidChannel_Close(recordService->recordPidChannels[esStreamIndex]);
                }
                else
                {
                    NEXUS_Playpump_ClosePidChannel(recordService->settings.playpumpIp,
                                                   recordService->recordPidChannels[esStreamIndex]);
                }
            }

        }
        BKNI_Memcpy((void *)&esStreamInfo,(void *)&recordService->mediaNode->esStreamInfo[esStreamIndex],sizeof(esStreamInfo));
        for(;esStreamIndex < recordService->activeEsStreamCount;esStreamIndex++)
        {
            BKNI_Memcpy((void *)&recordService->activeEsStreamInfo[esStreamIndex],
                        (void *)&recordService->activeEsStreamInfo[esStreamIndex+1],
                        sizeof(B_DVR_EsStreamInfo));
            BKNI_Memcpy((void *)&recordService->mediaNode->esStreamInfo[esStreamIndex],
                        (void *)&recordService->mediaNode->esStreamInfo[esStreamIndex+1],
                        sizeof(B_DVR_EsStreamInfo));
            BKNI_Memcpy((void *)&recordService->recordPidChannels[esStreamIndex],
                        (void *)&recordService->recordPidChannels[esStreamIndex+1],
                        sizeof(NEXUS_PidChannelHandle));
        }

        BKNI_Memcpy((void *)&recordService->mediaNode->esStreamInfo[recordService->mediaNode->esStreamCount-1],
                    (void *)&esStreamInfo,sizeof(esStreamInfo));
        recordService->activeEsStreamCount--;
    }
    BDBG_MSG(("B_DVR_RecordService_RemoveEsStream <<< "));
    B_Mutex_Unlock(recordService->recordMutex);
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_InjectDataStart(
    B_DVR_RecordServiceHandle recordService,
    uint8_t *buf,
    unsigned length)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_ASSERT(recordService->settings.dataInjectionService);
    BDBG_MSG(("B_DVR_RecordService_InjectDataStart >>>"));
    B_Mutex_Lock(recordService->recordMutex);
    if(!recordService->dataInjectionStarted) 
    {
        #ifdef TRANSCODE_SUPPORT
        if(recordService->recordServiceRequest.input == B_DVR_RecordServiceInputTranscode)
        {
            NEXUS_PidChannelHandle pidChannelTranscodePsi=NULL;
            B_DVR_TranscodeServiceHandle transcodeService = (B_DVR_TranscodeServiceHandle)recordService->settings.transcodeService;
            pidChannelTranscodePsi = B_DVR_TranscodeService_GetDataInjectionPidChannel(transcodeService);
            BDBG_ASSERT(pidChannelTranscodePsi);
            B_DVR_DataInjectionService_Start(recordService->settings.dataInjectionService,
                                             pidChannelTranscodePsi,
                                             buf,length);
            NEXUS_Record_AddPidChannel(recordService->record,
                                       pidChannelTranscodePsi,
                                       NULL);
        }
        else
        #endif
        {
            B_DVR_DataInjectionServiceSettings injectSettings;
            B_DVR_DataInjectionService_GetSettings(recordService->settings.dataInjectionService,&injectSettings);
            BDBG_ASSERT(injectSettings.pidChannel);
            B_DVR_DataInjectionService_Start(recordService->settings.dataInjectionService,
                                             injectSettings.pidChannel,
                                             buf,length);
            if(inputType == B_DVR_RecordServiceInputStreamingVOD)
            {
                NEXUS_Recpump_AddPidChannel(recordService->recpump,injectSettings.pidChannel,NULL);
            }
            else
            {
                NEXUS_Record_AddPidChannel(recordService->record,injectSettings.pidChannel,NULL);
            }
        }
        recordService->dataInjectionStarted = true;
    }
    else
    {
        BDBG_ERR(("Data injection already started for recordService %u",recordService->index));
    }
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_InjectDataStart <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_InjectDataStop(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_RecordServiceInput  inputType = recordService->recordServiceRequest.input;    
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_ASSERT(recordService->settings.dataInjectionService);
    BDBG_MSG(("B_DVR_RecordService_InjectDataStop >>>"));
    B_Mutex_Lock(recordService->recordMutex);
    if(recordService->dataInjectionStarted) 
    {
        #ifdef TRANSCODE_SUPPORT
        if(recordService->recordServiceRequest.input == B_DVR_RecordServiceInputTranscode)
        {
            NEXUS_PidChannelHandle pidChannelTranscodePsi=NULL;
            B_DVR_TranscodeServiceHandle transcodeService = (B_DVR_TranscodeServiceHandle)recordService->settings.transcodeService;
            BDBG_ASSERT(transcodeService);
            pidChannelTranscodePsi = B_DVR_TranscodeService_GetDataInjectionPidChannel(transcodeService);
            BDBG_ASSERT(pidChannelTranscodePsi);
            B_DVR_DataInjectionService_Stop(recordService->settings.dataInjectionService);
            NEXUS_Record_RemovePidChannel(recordService->record,
                                          pidChannelTranscodePsi);
        }
        else
        #endif
        {
            B_DVR_DataInjectionServiceSettings injectSettings;
            B_DVR_DataInjectionService_GetSettings(recordService->settings.dataInjectionService,&injectSettings);
            B_DVR_DataInjectionService_Stop(recordService->settings.dataInjectionService);
            if(inputType == B_DVR_RecordServiceInputStreamingVOD)
            {
                NEXUS_Recpump_RemovePidChannel(recordService->recpump,
                                               injectSettings.pidChannel);
            }
            else
            {
                NEXUS_Record_RemovePidChannel(recordService->record,
                                              injectSettings.pidChannel);
            }
        }
        recordService->dataInjectionStarted = false;
    }
    else
    {
        BDBG_ERR(("Data injection already stopped for recordService %u",recordService->index));
    }
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_InjectDataStop <<<"));
    return rc;
}

NEXUS_PidChannelHandle B_DVR_RecordService_GetDataInjectionPidChannel(
    B_DVR_RecordServiceHandle recordService)
{
    NEXUS_PidChannelHandle dataInjectionPidChannel = NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_RecordService_GetDataInjectionPidChannel >>>"));
    B_Mutex_Lock(recordService->recordMutex);
    if(recordService->settings.dataInjectionService)
    { 
        #ifdef TRANSCODE_SUPPORT
        if(recordService->recordServiceRequest.input == B_DVR_RecordServiceInputTranscode)
        {
            B_DVR_TranscodeServiceHandle transcodeService = (B_DVR_TranscodeServiceHandle)recordService->settings.transcodeService;
            BDBG_ASSERT(transcodeService);
            dataInjectionPidChannel = B_DVR_TranscodeService_GetDataInjectionPidChannel(transcodeService);
        }
        else
        #endif
        { 
            B_DVR_DataInjectionServiceSettings injectSettings;
            B_DVR_DataInjectionService_GetSettings(recordService->settings.dataInjectionService,
                                                   &injectSettings);
            dataInjectionPidChannel = injectSettings.pidChannel;
        }
    }
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_GetDataInjectionPidChannel %x >>>",dataInjectionPidChannel));
    return dataInjectionPidChannel;
}

NEXUS_PidChannelHandle B_DVR_RecordService_GetPidChannel(
    B_DVR_RecordServiceHandle recordService,
    unsigned pid)
{
    unsigned esStreamIndex;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_MSG(("B_DVR_RecordService_GetPidChannel >>>"));
    B_Mutex_Lock(recordService->recordMutex);
    for(esStreamIndex=0;esStreamIndex<recordService->activeEsStreamCount;esStreamIndex++)
    {
        if(recordService->activeEsStreamInfo[esStreamIndex].pid == pid)
        {
            break;
        }
    }
    if(esStreamIndex >= recordService->activeEsStreamCount)
    {
        BDBG_ERR(("No pid channel opened for PID %u",pid));
        goto error;
    }

    BDBG_MSG(("B_DVR_RecordService_GetPidChannel %x <<<",recordService->recordPidChannels[esStreamIndex]));
    B_Mutex_Unlock(recordService->recordMutex);
    return recordService->recordPidChannels[esStreamIndex];
error:
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_GetPidChannel Err <<<"));
    return NULL;
}

B_DVR_ERROR B_DVR_RecordService_AddDrmSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMUsage drmUsage)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("B_DVR_B_DVR_RecordService_AddDrmSettings usage:%d>>>",drmUsage));
    B_Mutex_Lock(recordService->recordMutex);
    switch(drmUsage)
    {
    case eB_DVR_DRMUsageRecord:
        {
            NEXUS_RecordSettings recordSettings;
            recordService->recordDrm = drmService;
            BDBG_ASSERT(recordService->recpump);
            NEXUS_Record_GetSettings(recordService->record, &recordSettings);
            recordSettings.recpumpSettings.securityDma = drmService->dma ;
            recordSettings.recpumpSettings.securityDmaDataFormat= NEXUS_DmaDataFormat_eMpeg;
            recordSettings.recpumpSettings.securityContext = drmService->keySlot ; 
            NEXUS_Record_SetSettings(recordService->record, &recordSettings);
        }
        break;
    case eB_DVR_DRMUsageMediaProbe:
        {
            recordService->mediaProbeDrm = drmService;
        }
        break;
    default:
        BDBG_ERR(("Invalid drmUsage %d",drmUsage));
        rc = B_DVR_INVALID_PARAMETER;
    }

    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_B_DVR_RecordService_AddDrmSettings <<< "));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_RemoveDrmSettings(
    B_DVR_RecordServiceHandle recordService,
    B_DVR_DRMUsage drmUsage)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    
    BDBG_MSG(("B_DVR_B_DVR_RecordService_RemoveDrmSettings usage %d>>>",drmUsage));

    B_Mutex_Lock(recordService->recordMutex);
    switch(drmUsage)
    {
    case eB_DVR_DRMUsageRecord:
        {
            recordService->recordDrm = NULL;
        }
        break;
    case eB_DVR_DRMUsageMediaProbe:
        {
            recordService->mediaProbeDrm = NULL;
        }
        break;
    default:
        BDBG_ERR(("Invalid drmUsage %d",drmUsage));
        rc = B_DVR_INVALID_PARAMETER;
    }

    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_B_DVR_RecordService_RemoveDrmSettings <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_SetPlaybackUpdateEvent(
    B_DVR_RecordServiceHandle recordService,
    B_EventHandle updatePlaybackEvent)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_RecordService_SetPlaybackUpdateEvent>>>"));
    if(recordService)
    {
        BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
        B_Mutex_Lock(recordService->recordMutex);
        recordService->updatePlaybackEvent = updatePlaybackEvent;
        B_Mutex_Unlock(recordService->recordMutex);
    }
    BDBG_MSG(("B_DVR_RecordService_SetPlaybackUpdateEvent <<<"));
    return rc;
}

B_DVR_MediaNode B_DVR_RecordService_GetMediaNode(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_MediaNode mediaNode=NULL;
    BDBG_MSG(("B_DVR_RecordService_GetMediaNode>>>"));
    if(recordService)
    {
        BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
        B_Mutex_Lock(recordService->recordMutex);
        mediaNode = recordService->mediaNode;
        B_Mutex_Unlock(recordService->recordMutex);
    }
    BDBG_MSG(("B_DVR_RecordService_GetMediaNode <<<"));
    return mediaNode;
}

B_DVR_ERROR B_DVR_RecordService_Pause(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_MSG(("B_DVR_RecordService_Pause>>>"));
    B_Mutex_Lock(recordService->schedulerMutex);
    B_Mutex_Lock(recordService->recordMutex);
    if(recordService->dataInjectionStarted)
    {
        BDBG_ERR(("%s: Data Injection should be stopped before calling pause",
                  recordService->mediaNode->programName));
        rc = B_DVR_NOT_SUPPORTED;
        goto error;
    }
    rc = B_DVR_RecordService_P_Pause(recordService);
error:
    B_Mutex_Unlock(recordService->recordMutex);
    B_Mutex_Unlock(recordService->schedulerMutex);
    BDBG_MSG(("B_DVR_RecordService_Pause<<<"));
    return rc;
}


B_DVR_ERROR B_DVR_RecordService_Resume(
    B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    BDBG_MSG(("B_DVR_RecordService_Resume>>>"));
    B_Mutex_Lock(recordService->recordMutex);
    rc = B_DVR_RecordService_P_Resume(recordService);
    B_Mutex_Unlock(recordService->recordMutex);
    BDBG_MSG(("B_DVR_RecordService_Resume<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_AddInProgressRecPlayback(
   B_DVR_RecordServiceHandle recordService,
   NEXUS_PlaybackHandle playback)
{

    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_RecordService_AddInProgressRecPlayback>>>"));
    if(recordService && playback) 
    {
        BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
        B_Mutex_Lock(recordService->recordMutex);
        NEXUS_Record_AddPlayback(recordService->record, playback);
        recordService->inProgressRecPlay = playback;
        B_Mutex_Unlock(recordService->recordMutex);
    }
    BDBG_MSG(("B_DVR_RecordService_AddInProgressRecPlayback <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_RecordService_RemoveInProgressRecPlayback(B_DVR_RecordServiceHandle recordService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_RecordService_RemoveInProgressRecPlayback>>>"));
    BDBG_OBJECT_ASSERT(recordService,B_DVR_RecordService);
    if(recordService->inProgressRecPlay) 
    {
        B_Mutex_Lock(recordService->recordMutex);
        NEXUS_Record_RemovePlayback(recordService->record,recordService->inProgressRecPlay);
        recordService->inProgressRecPlay = NULL;
        B_Mutex_Unlock(recordService->recordMutex);
    }
    BDBG_MSG(("B_DVR_RecordService_RemoveInProgressRecPlayback <<<"));
    return rc;
}

