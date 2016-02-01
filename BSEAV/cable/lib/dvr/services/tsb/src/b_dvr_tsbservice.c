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
#include "b_dvr_tsbservice.h"
#include "b_dvr_file.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_list.h"
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_mediaprobe.h"
#include <pthread.h>

BDBG_MODULE(b_dvr_tsbservice);
BDBG_OBJECT_ID(B_DVR_TSBService);
struct B_DVR_TSBService
{
    BDBG_OBJECT(B_DVR_TSBService)
    unsigned index;
    NEXUS_RecpumpHandle recpump; 
    NEXUS_RecordHandle record;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackHandle inProgressRecPlay;
    NEXUS_FileRecordHandle nexusFileRecord;
    NEXUS_FilePlayHandle nexusFilePlay;
    NEXUS_FilePlayHandle nexusFilePlay1;
    NEXUS_PidChannelHandle recordPidChannels[B_DVR_MAX_PIDS];
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_OperationSettings operationSettings;
    struct B_DVR_TSBServiceRequest tsbServiceRequest;
    struct B_DVR_TSBServicePermanentRecordingRequest permRecReq;
    bool tsbPlaybackStarted;
    bool tsbRecordStarted;
    bool tsbRecordProbed;
    bool recpumpStarted;
    bool tsbConversion;
    bool appendedConversion;
    B_DVR_ServiceCallback registeredCallback;
    void *appContext;
    B_DVR_MediaNode tsbMediaNode;
    B_DVR_MediaNode permanentRecordingMediaNode;
    B_SchedulerEventId tsbEventSchedulerID;
    B_EventHandle tsbRecordEvent;
    B_EventHandle updatePlaybackEvent;
    B_SchedulerTimerId tsbPlayAlarmTimer;
    unsigned long tsbPlayAlarmTime;
    bool dataInjectionStarted;
    B_MutexHandle tsbMutex;
    bool mpeg2Video;
    B_DVR_EsStreamInfo activeEsStreamInfo[B_DVR_MAX_PIDS];
    unsigned activeEsStreamCount;
    B_DVR_DRMServiceHandle recordDrm;
    B_DVR_DRMServiceHandle playbackDrm;
    B_DVR_DRMServiceHandle mediaProbeDrm;
    pthread_t appendedConversionThread;
    B_EventGroupHandle eventGroup;
    B_EventHandle appendedConversionUpdate;
    B_EventHandle appendedConversionThreadStop;
    B_DVR_SegmentedFileRecordAppendSettings appendSettings;
    B_DVR_MediaProbeHandle mediaProbe;
    off_t probeOffset;
    B_MutexHandle schedulerMutex;
    unsigned long lastPrintStatusTime;
    bool overflow;
    bool mediaSegmentSizeValidated;
};

static unsigned B_DVR_TSBService_P_CalculateBitRate(off_t byteOffsetStart, off_t byteOffsetEnd, unsigned startTime,unsigned endTime);
static void B_DVR_TSBService_P_PlaybackAlarmTimer(void *tsbContext);
static void B_DVR_TSBService_P_SegmentedFileEvent(void *segmentedFileContext,B_DVR_SegmentedFileEvent fileEvent);
static void B_DVR_TSBService_P_PlaybackStartOfStream(void *context,int param);
static void B_DVR_TSBService_P_PlaybackEndOfStream(void *context,int param);
static void B_DVR_TSBService_P_PlaybackAlarmTimer(void *tsbContext);
static void * B_DVR_TSBService_P_UpdateAppendedTsbConversion(void *tsbContext);
static void B_DVR_TSBService_P_UpdateMediaNode(void *tsbContext);
B_DVR_ERROR B_DVR_TSBService_P_PlaybackStart(B_DVR_TSBServiceHandle tsbService, unsigned long startTime);
B_DVR_ERROR B_DVR_TSBService_P_PlaybackStop(B_DVR_TSBServiceHandle tsbService);
void B_DVR_TSBService_P_PrintStatus(B_DVR_TSBServiceHandle tsbService);
void B_DVR_TSBService_P_PrintRecInfo(B_DVR_TSBServiceHandle tsbService, bool permRec);
static void B_DVR_TSBService_P_Overflow(void *tsbContext,int param);
B_DVR_ERROR B_DVR_TSBService_P_Pause(B_DVR_TSBServiceHandle tsbService);
B_DVR_ERROR B_DVR_TSBService_P_Resume(B_DVR_TSBServiceHandle tsbService);

static unsigned B_DVR_TSBService_P_CalculateBitRate(off_t byteOffsetStart, off_t byteOffsetEnd, unsigned startTime,unsigned endTime)
{
    off_t byteLen=0;
    unsigned timeLen=0;
    unsigned bitRate=0;
    timeLen = (endTime - startTime);
    if(!timeLen)
    {
        goto done;
    }
    byteLen = byteOffsetEnd - byteOffsetStart;

    byteLen = byteLen*8; /*Convert bytes into bits*/
    bitRate = byteLen/timeLen; /* bits per ms */
    bitRate *= 1000; /*bps*/
done:
    return bitRate;
}

static void B_DVR_TSBService_P_PlaybackAlarmTimer(void *tsbContext)
{
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)tsbContext;
    B_DVR_Event event;
    B_DVR_Service service;
    bool alarmOn=false;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    NEXUS_Playback_GetStatus(tsbService->playback,&nexusPlaybackStatus);

    if((nexusPlaybackStatus.state == NEXUS_PlaybackState_eTrickMode) &&
       (tsbService->operationSettings.operation == eB_DVR_OperationFastRewind ||
        tsbService->operationSettings.operation == eB_DVR_OperationSlowRewind ||
        tsbService->operationSettings.operationSpeed < 0)
       )
    {
        alarmOn = (nexusPlaybackStatus.position  <= tsbService->tsbPlayAlarmTime)? true:false;
    }
    else
    {
       alarmOn = (nexusPlaybackStatus.position  >= tsbService->tsbPlayAlarmTime)? true:false;
    }

    if(alarmOn)
    {
        tsbService->tsbPlayAlarmTimer=NULL;
        tsbService->tsbPlayAlarmTime = 0;
        if(tsbService->registeredCallback)
        {
            event = eB_DVR_EventPlaybackAlarm;
            service = eB_DVR_ServiceTSB;
            tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        }
    }
    else
    {

        B_Mutex_Lock(tsbService->tsbMutex);
        tsbService->tsbPlayAlarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                               tsbService->schedulerMutex,
                                                              1000,B_DVR_TSBService_P_PlaybackAlarmTimer,
                                                              (void*)tsbService);
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    return;
}

static void B_DVR_TSBService_P_SegmentedFileEvent(void *segmentedFileContext,B_DVR_SegmentedFileEvent fileEvent)
{
    B_DVR_Event event=eB_DVR_EventMax;
    B_DVR_Service service;
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle)(segmentedFileContext);
    B_DVR_TSBServiceHandle tsbService = NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_P_SegmentedFileEvent >>>"));

    if(!segmentedFile)
    {
        BDBG_WRN((" segmentfile is null in B_DVR_TSBService_P_SegmentedFileEvent"));
        goto error;
    }

    tsbService = dvrManager->tsbService[segmentedFile->serviceIndex];
    if(!tsbService)
    {
        BDBG_WRN(("tsbService is NULL in B_DVR_TSBService_P_SegmentedFileEvent"));
        goto error;
    }

    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);

    BDBG_MSG(("index %d service %d event %d",segmentedFile->serviceIndex,
              segmentedFile->service,fileEvent));
    
    if(tsbService->registeredCallback)
    {
        switch(fileEvent)
        {
        case B_DVR_SegmentedFileEventReadError:
            {
                event = eB_DVR_EventAbortTSBPlayback;
                BDBG_ERR(("Abort TSBPlayback Event - Read Error"));
            }
            break;
        case B_DVR_SegmentedFileEventSeekError:
            {
                if(segmentedFile->fileIO == eB_DVR_FileIOWrite)
                { 
                    event = eB_DVR_EventAbortTSBRecord;
                    BDBG_ERR(("Abort TSBRecord Event - Seek Error"));
                }
                else
                {
                    event = eB_DVR_EventAbortTSBPlayback;
                    BDBG_ERR(("Abort TSBRecord Event - Seek Error"));
                }
            }
            break;
        case B_DVR_SegmentedFileEventWriteError:
            {
                event = eB_DVR_EventAbortTSBRecord;
                BDBG_ERR(("Abort TSBRecord Event - Write Error"));
            }
            break;
        case B_DVR_SegmentedFileEventOpenError:
            {
                if(segmentedFile->fileIO == eB_DVR_FileIOWrite)
                { 
                    event = eB_DVR_EventAbortTSBRecord;
                    BDBG_ERR(("Abort TSBRecord Event - Open Error"));
                }
                else
                {
                    event = eB_DVR_EventAbortTSBPlayback;
                    BDBG_ERR(("Abort TSBPlayback Event - Open Error"));
                }
            }
            break;
        case B_DVR_SegmentedFileEventNotFoundError:
            {
                event = eB_DVR_EventAbortTSBPlayback;
                BDBG_ERR(("Abort TSBPlayback Event - file segment not found"));
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
        service = eB_DVR_ServiceTSB;
        tsbService->registeredCallback(tsbService->appContext,segmentedFile->service,event,segmentedFile->service);
    }
error:
    BDBG_MSG(("B_DVR_TSBService_P_SegmentedFileEvent <<<<"));
    return;
}

static void * B_DVR_TSBService_P_UpdateAppendedTsbConversion(void *tsbContext)
{
    unsigned numOfEvents;
    unsigned i;
    B_EventHandle triggeredEvents[2];
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)tsbContext;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord= (B_DVR_SegmentedFileRecordHandle)tsbService->nexusFileRecord;
    
    while(1)
    {
        B_EventGroup_Wait(tsbService->eventGroup,B_WAIT_FOREVER,triggeredEvents,2,&numOfEvents);
        for(i=0;i<numOfEvents;i++)
        {
            if(triggeredEvents[i] == tsbService->appendedConversionUpdate)
            {
                B_Mutex_Lock(tsbService->tsbMutex);
                B_DVR_SegmentedFileRecord_UpdateAppendedMediaMetaDataFile(&tsbService->appendSettings);
                B_DVR_SegmentedFileRecord_UpdateAppendedNavMetaDataFile(&tsbService->appendSettings);
                tsbService->permanentRecordingMediaNode->mediaEndTime += 
                     segmentedFileRecord->permRec.recCurrentTime - segmentedFileRecord->permRec.recStartTime;
                tsbService->permanentRecordingMediaNode->mediaLinearEndOffset +=
                     segmentedFileRecord->permRec.recMediaEndOffset - tsbService->appendSettings.tsbConvMediaCurrentOffset;
                tsbService->permanentRecordingMediaNode->navLinearEndOffset +=
                    segmentedFileRecord->permRec.recNavEndOffset - tsbService->appendSettings.tsbConvNavCurrentOffset;
                tsbService->appendSettings.tsbConvMediaCurrentOffset = segmentedFileRecord->permRec.recMediaEndOffset;
                tsbService->appendSettings.tsbConvNavCurrentOffset = segmentedFileRecord->permRec.recNavEndOffset;
                B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                           tsbService->permanentRecordingMediaNode,
                                           tsbService->tsbServiceRequest.volumeIndex,false);
                B_Mutex_Unlock(tsbService->tsbMutex);
                BDBG_MSG(("appended programName %s",tsbService->permanentRecordingMediaNode->programName));
                BDBG_MSG(("time -> recStart %lu recEnd %lu",
                          tsbService->permanentRecordingMediaNode->mediaStartTime,
                          tsbService->permanentRecordingMediaNode->mediaEndTime));
                BDBG_MSG(("media offset recStart %lld recEnd %lld",
                          tsbService->permanentRecordingMediaNode->mediaLinearStartOffset,
                          tsbService->permanentRecordingMediaNode->mediaLinearEndOffset));
                BDBG_MSG(("nav offset recStart %lld recEnd %lld",
                          tsbService->permanentRecordingMediaNode->navLinearStartOffset,
                          tsbService->permanentRecordingMediaNode->navLinearEndOffset));
                BDBG_MSG(("appended nav start %lld end %lld",
                          segmentedFileRecord->permRec.recNavStartOffset,
                          segmentedFileRecord->permRec.recNavEndOffset));
                BDBG_MSG(("appended navOffset %lld",tsbService->appendSettings.appendNavOffset));
            }
            if(triggeredEvents[i] == tsbService->appendedConversionThreadStop)
            {
                B_DVR_Event event;
                B_DVR_Service service;
                BDBG_MSG(("exit appendedConversionThread"));
                B_Mutex_Lock(tsbService->tsbMutex);
                tsbService->appendedConversion=false;
                B_Mutex_Unlock(tsbService->tsbMutex);
                B_DVR_SegmentedFileRecord_CloseAppendedPermMetaDataFile(&tsbService->appendSettings);
                if(tsbService->registeredCallback)
                { 
                    event = eB_DVR_EventTSBConverstionCompleted;
                    service = eB_DVR_ServiceTSB;
                    tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
                }
                pthread_exit(NULL);
            }
        }
    }
    return NULL;
}

static void B_DVR_TSBService_P_UpdateMediaNode(void *tsbContext)
{
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)tsbContext;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord;
    unsigned long returnedTimeStamp;
    B_DVR_Event event;
    B_DVR_Service service;
    bool tsbConversion,tsbRecordStarted,tsbRecordProbed;
    B_DVR_FilePosition tsbStartPosition,tsbEndPosition;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    B_Mutex_Lock(tsbService->tsbMutex);
    if(tsbService->overflow)
    {
        BDBG_WRN(("%s tsb program %s overflow",
                  __FUNCTION__,
                  tsbService->tsbMediaNode->programName));
        B_Mutex_Unlock(tsbService->tsbMutex);
        goto error;
    }
    segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)tsbService->nexusFileRecord;
    rc = B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbStartPosition,&tsbEndPosition);
    if(rc != B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:B_DVR_SegmentedFileRecord_GetBounds failed",__FUNCTION__));
        BDBG_ERR(("tsb program name %s index:%u",tsbService->tsbMediaNode->programName,tsbService->index));
        BDBG_ERR(("media time: recStart %ld recEnd %ld",
              tsbService->tsbMediaNode->mediaStartTime,
              tsbService->tsbMediaNode->mediaEndTime));
        BDBG_ERR(("media Offset: recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->mediaLinearStartOffset,
              tsbService->tsbMediaNode->mediaLinearEndOffset));
        BDBG_ERR(("nav offset : recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->navLinearStartOffset,
              tsbService->tsbMediaNode->navLinearEndOffset));
        B_Mutex_Unlock(tsbService->tsbMutex);
        goto error;
    }
    tsbConversion = tsbService->tsbConversion;
    tsbRecordStarted = tsbService->tsbRecordStarted;
    tsbRecordProbed = tsbService->tsbRecordProbed;
    if(!tsbService->tsbRecordStarted)
    {
        NEXUS_RecordSettings recordSettings;
        NEXUS_Record_GetSettings(tsbService->record, &recordSettings);
        recordSettings.pollingTimer = B_DVR_RECORD_POLLING_INTERVAL;
        NEXUS_Record_SetSettings(tsbService->record, &recordSettings);
        tsbService->tsbRecordStarted = true;
    }
    if(tsbService->tsbConversion)
    { 
        returnedTimeStamp = B_DVR_FileSegmentedRecord_CopyMetaDataFile(tsbService->nexusFileRecord,
                                                                       tsbStartPosition,
                                                                       tsbEndPosition);
        if (returnedTimeStamp == (unsigned long)-1)
        {
            BDBG_ERR(("recEndTime beyond TSB recording endTime %ld %ld %ld %ld",
                      segmentedFileRecord->permRec.recStartTime,
                      segmentedFileRecord->permRec.recCurrentTime,
                      segmentedFileRecord->permRec.recEndTime,
                      returnedTimeStamp));
            returnedTimeStamp=0;
        }
        else
        {
            if(tsbService->appendedConversion)
            {
                B_Event_Set(tsbService->appendedConversionUpdate);
            }
            else
            {
                tsbService->permanentRecordingMediaNode->mediaEndTime = 
                    segmentedFileRecord->permRec.recCurrentTime;
                tsbService->permanentRecordingMediaNode->mediaStartTime = 
                    segmentedFileRecord->permRec.recStartTime;
                tsbService->permanentRecordingMediaNode->mediaLinearStartOffset =
                    segmentedFileRecord->permRec.recMediaStartOffset;
                tsbService->permanentRecordingMediaNode->mediaLinearEndOffset =
                    segmentedFileRecord->permRec.recMediaEndOffset;
                tsbService->permanentRecordingMediaNode->navLinearStartOffset =
                    segmentedFileRecord->permRec.recNavStartOffset;
                tsbService->permanentRecordingMediaNode->navLinearEndOffset =
                    segmentedFileRecord->permRec.recNavEndOffset;
            }

            if (returnedTimeStamp >= segmentedFileRecord->permRec.recEndTime)
            {
                B_DVR_SegmentedFileRecord_ClosePermMetaDataFile((NEXUS_FileRecordHandle)segmentedFileRecord);
                tsbService->permanentRecordingMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                tsbService->tsbMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
                /*dumpMediaNode(tsbService->permanentRecordingMediaNode);*/
                tsbService->tsbConversion = false;
                if(tsbService->appendedConversion)
                {
                    B_Event_Set(tsbService->appendedConversionThreadStop);
                }
                BDBG_MSG(("Reached the end of tsb Conversion for program %s",tsbService->permanentRecordingMediaNode->programName));
           }
           tsbService->permanentRecordingMediaNode->maxStreamBitRate = 
               B_DVR_TSBService_P_CalculateBitRate(tsbService->permanentRecordingMediaNode->mediaLinearStartOffset,
                                                   tsbService->permanentRecordingMediaNode->mediaLinearEndOffset,
                                                   tsbService->permanentRecordingMediaNode->mediaStartTime,
                                                   tsbService->permanentRecordingMediaNode->mediaEndTime);
           if(!tsbService->appendedConversion)
           {
               B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                         tsbService->permanentRecordingMediaNode,
                                         tsbService->tsbServiceRequest.volumeIndex,false);
               if(tsbService->updatePlaybackEvent)
               {
                   B_Event_Set(tsbService->updatePlaybackEvent);
               }
           }
        }
    }
    
    tsbService->tsbMediaNode->mediaLinearStartOffset = tsbStartPosition.mpegFileOffset;
    tsbService->tsbMediaNode->mediaLinearEndOffset =  tsbEndPosition.mpegFileOffset;
    tsbService->tsbMediaNode->navLinearStartOffset = tsbStartPosition.navFileOffset;
    tsbService->tsbMediaNode->navLinearEndOffset = tsbEndPosition.navFileOffset;
    tsbService->tsbMediaNode->mediaStartTime = tsbStartPosition.timestamp;
    tsbService->tsbMediaNode->mediaEndTime = tsbEndPosition.timestamp;
    tsbService->tsbMediaNode->maxStreamBitRate = 
        B_DVR_TSBService_P_CalculateBitRate(tsbService->tsbMediaNode->mediaLinearStartOffset,
                                            tsbService->tsbMediaNode->mediaLinearEndOffset,
                                            tsbService->tsbMediaNode->mediaStartTime,
                                            tsbService->tsbMediaNode->mediaEndTime);
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               tsbService->tsbMediaNode,
                               tsbService->tsbServiceRequest.volumeIndex,false);
    if(!tsbService->tsbRecordProbed && 
       (tsbService->tsbMediaNode->mediaLinearEndOffset >= 
       (tsbService->probeOffset + B_DVR_MEDIA_PROBE_CHUNK_SIZE)))
    {
         BDBG_ASSERT(tsbService->mediaProbe);
         tsbService->tsbRecordProbed = B_DVR_MediaProbe_Parse(tsbService->mediaProbe,tsbService->mediaProbeDrm,&tsbService->probeOffset);
         if(!tsbService->tsbRecordProbed)
         {
             BDBG_MSG(("%s:attempt to probe failed - probe off:%lld",__FUNCTION__,tsbService->probeOffset));
             BDBG_MSG(("tsb program name %s index:%u",tsbService->tsbMediaNode->programName,tsbService->index));
             BDBG_MSG(("media Offset: recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->mediaLinearStartOffset,
              tsbService->tsbMediaNode->mediaLinearEndOffset));
             if(tsbService->tsbMediaNode->maxStreamBitRate < B_DVR_LOW_BITRATE_STREAM) 
             {
                 tsbService->probeOffset=0;
                 BDBG_MSG(("resetting tsbService->probeOffset."));
                 BDBG_MSG(("stream bit rate:%u",tsbService->tsbMediaNode->maxStreamBitRate));
             }
         }
         else
         {
             if(tsbService->mediaProbe)
             {   /* release the resources associated with media probe after probing is complete */
                 B_DVR_MediaProbe_Close(tsbService->mediaProbe);
                 tsbService->mediaProbe = NULL;
                 tsbService->probeOffset=0;
             }

             if(tsbService->tsbConversion)
             {
                 BKNI_Memcpy((void *)tsbService->permanentRecordingMediaNode->esStreamInfo,
                             (void *)tsbService->tsbMediaNode->esStreamInfo,
                             sizeof(B_DVR_EsStreamInfo)*tsbService->tsbMediaNode->esStreamCount);
                 tsbService->permanentRecordingMediaNode->mediaAttributes = tsbService->tsbMediaNode->mediaAttributes;
             }
         }
    }
    if( (tsbService->tsbMediaNode->mediaEndTime-tsbService->lastPrintStatusTime)
        >= B_DVR_PRINT_REC_STATUS_INTERVAL)
    {
        B_DVR_TSBService_P_PrintStatus(tsbService);
        tsbService->lastPrintStatusTime = 
            tsbService->tsbMediaNode->mediaEndTime;

    }
    B_Mutex_Unlock(tsbService->tsbMutex);

    if(!tsbService->registeredCallback) 
    {
        goto error;
    }
    if((tsbRecordStarted!=tsbService->tsbRecordStarted) && tsbService->tsbRecordStarted)
    {
        event = eB_DVR_EventStartOfRecording;
        service = eB_DVR_ServiceTSB;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_MSG(("eB_DVR_EventStartOfRecording sent to the upper layers"));
   }

    if((tsbRecordProbed!=tsbService->tsbRecordProbed) && tsbService->tsbRecordProbed)
   {
        service = eB_DVR_ServiceTSB;
        event = eB_DVR_EventMediaProbed;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_MSG(("mediaProbe complete callback sent"));
        
        if(tsbService->tsbMediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_HD_STREAM)
        { 
            event = eB_DVR_EventHDStreamRecording;
        }
        else
        {
            event = eB_DVR_EventSDStreamRecording;
        }
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_MSG(("HD/SD recording callback sent"));
       
   }

    if((tsbConversion!=tsbService->tsbConversion && !tsbService->tsbConversion) 
       && !tsbService->appendedConversion)
    {
        event = eB_DVR_EventTSBConverstionCompleted;
        service = eB_DVR_ServiceTSB;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_MSG(("tsbConversion complete callback sent"));
    }
    if(tsbStartPosition.mpegFileOffset > tsbEndPosition.mpegFileOffset)
    {
        service = eB_DVR_ServiceTSB;
        event = eB_DVR_EventAbortRecord;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_ERR(("recording to be aborted startOffset:%lld endOffset:%lld",
                  tsbStartPosition.mpegFileOffset,tsbEndPosition.mpegFileOffset));
    }
error:
    return;
}

static void B_DVR_TSBService_P_PlaybackStartOfStream(
    void *context,
    int param)
{
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)context;
    B_DVR_Event event =eB_DVR_EventStartOfPlayback;
    B_DVR_Service service = eB_DVR_ServiceTSB;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(tsbService->nexusFilePlay);
    BDBG_ASSERT(tsbService->nexusFilePlay1);
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStartOfStream %d >>>",tsbService->index));
    B_Mutex_Lock(tsbService->tsbMutex);
    B_DVR_SegmentedFilePlay_ResetCachedFileNode(tsbService->nexusFilePlay);
    B_DVR_SegmentedFilePlay_ResetCachedFileNode(tsbService->nexusFilePlay1);
    B_Mutex_Unlock(tsbService->tsbMutex);
    if(tsbService->registeredCallback)
    {
        tsbService->registeredCallback(tsbService->appContext,param,event,service);
    }
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStartOfStream >>>")); 
    return;
}

static void B_DVR_TSBService_P_PlaybackEndOfStream(
    void *context,
    int param)
{
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)context;
    B_DVR_Event event = eB_DVR_EventEndOfPlayback;
    B_DVR_Service service = eB_DVR_ServiceTSB;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(tsbService->nexusFilePlay);
    BDBG_ASSERT(tsbService->nexusFilePlay1);
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackEndOfStream %d >>>",tsbService->index));
    B_Mutex_Lock(tsbService->tsbMutex);
    B_DVR_SegmentedFilePlay_ResetCachedFileNode(tsbService->nexusFilePlay);
    B_DVR_SegmentedFilePlay_ResetCachedFileNode(tsbService->nexusFilePlay1);
    B_Mutex_Unlock(tsbService->tsbMutex);
    if(tsbService->registeredCallback)
    {
     tsbService->registeredCallback(tsbService->appContext,param,event,service);
    }
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackEndOfStream >>>")); 
    return;
}

B_DVR_ERROR B_DVR_TSBService_P_PlaybackStart(
    B_DVR_TSBServiceHandle tsbService,
    unsigned long startTime)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PlaybackSettings playbackSettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStart %d >>>",tsbService->index));

    NEXUS_Playback_GetSettings(tsbService->playback, &playbackSettings);
    if(tsbService->tsbServiceSettings.tsbPlaybackSettings.stcChannel) 
    {
        playbackSettings.stcChannel = tsbService->tsbServiceSettings.tsbPlaybackSettings.stcChannel;
    }
    else
    {
        playbackSettings.simpleStcChannel = tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleStcChannel;
    }
    playbackSettings.stcTrick = true;
    playbackSettings.startPaused = true;
    NEXUS_Playback_SetSettings(tsbService->playback, &playbackSettings);

    NEXUS_Record_AddPlayback(tsbService->record, tsbService->playback);
    NEXUS_Playback_Start(tsbService->playback, tsbService->nexusFilePlay, NULL);
    NEXUS_Playback_Seek(tsbService->playback,startTime);
    tsbService->tsbPlaybackStarted = true;
    if(tsbService->tsbPlayAlarmTime && !tsbService->tsbPlayAlarmTimer)
    { 
        tsbService->tsbPlayAlarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                               tsbService->schedulerMutex,
                                                               1000,B_DVR_TSBService_P_PlaybackAlarmTimer,
                                                               (void*)tsbService);
    }
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStart <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_P_PlaybackStop(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStop %d >>>",tsbService->index));
    tsbService->tsbPlayAlarmTimer = NULL;
    NEXUS_Playback_Stop(tsbService->playback);
    NEXUS_Record_RemovePlayback(tsbService->record,tsbService->playback);
    tsbService->tsbPlaybackStarted = false;
    BDBG_MSG(("B_DVR_TSBService_P_PlaybackStop <<<"));
    return rc;
}

NEXUS_VideoCodec B_DVR_TSBService_P_GetVideoCodec(B_DVR_TSBServiceHandle tsbService)
{
  unsigned esStreamIndex=0;
  NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eMpeg2 ;
  BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
  BDBG_MSG(("B_DVR_TSBService_P_GetVideoCodec >>>"));
  for(esStreamIndex=0;esStreamIndex < tsbService->activeEsStreamCount; esStreamIndex++)
  {
      if(tsbService->activeEsStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
      {
          videoCodec = tsbService->activeEsStreamInfo[esStreamIndex].codec.videoCodec;
          break;
      }
  }
  BDBG_MSG(("B_DVR_TSBService_P_GetVideoCodec <<<"));
  return videoCodec;
}

void B_DVR_TSBService_P_PrintStatus(B_DVR_TSBServiceHandle tsbService)
{
    unsigned sizeOfNavEntry=0;
    unsigned numOfFrames=0;
    unsigned duration=0;
    off_t len=0;
    NEXUS_RecpumpStatus recpumpStatus;
    BDBG_WRN(("**************************"));
    BDBG_WRN((" program:%s TSB Status",tsbService->tsbMediaNode->programName));
    BDBG_WRN((" startTime:%lums endTime:%lums",
              tsbService->tsbMediaNode->mediaStartTime,
              tsbService->tsbMediaNode->mediaEndTime));
    duration = tsbService->tsbMediaNode->mediaEndTime-tsbService->tsbMediaNode->mediaStartTime;
    duration = duration/1000;
    duration = duration/60;
    BDBG_WRN((" duration:%u mins",duration));
    BDBG_WRN((" media - startOffset:%lld endOffset:%lld",
              tsbService->tsbMediaNode->mediaLinearStartOffset,
              tsbService->tsbMediaNode->mediaLinearEndOffset));
    len = tsbService->tsbMediaNode->mediaLinearEndOffset - tsbService->tsbMediaNode->mediaLinearStartOffset;
    BDBG_WRN((" mediaLength:%lld",len));
    BDBG_WRN((" navigation - startOffset:%lld endOffset:%lld",
              tsbService->tsbMediaNode->navLinearStartOffset,
              tsbService->tsbMediaNode->navLinearEndOffset));
    sizeOfNavEntry = tsbService->mpeg2Video?sizeof(BNAV_Entry):sizeof(BNAV_AVC_Entry);
    numOfFrames = (tsbService->tsbMediaNode->navLinearEndOffset - 
                   tsbService->tsbMediaNode->navLinearStartOffset)/sizeOfNavEntry;
    BDBG_WRN((" numOfVideoFrames:%u",numOfFrames));
    BDBG_WRN((" bitRate:%ubps",tsbService->tsbMediaNode->maxStreamBitRate));
    BDBG_WRN((" mediaProbed:%s",tsbService->tsbRecordProbed?"true":"false"));
    BDBG_WRN((" mediaProbed offset:%lld",tsbService->probeOffset));
    BDBG_WRN((" tsbPlayback:%s",tsbService->tsbPlaybackStarted?"true":"false"));
    BDBG_WRN((" ***************************"));
    if(tsbService->permanentRecordingMediaNode) 
    {
        BDBG_WRN(("\n ***************************"));
        BDBG_WRN((" tsbConv program:%s Status",tsbService->permanentRecordingMediaNode->programName));
        BDBG_WRN((" startTime:%lums endTime:%lums",
                  tsbService->permanentRecordingMediaNode->mediaStartTime,
                  tsbService->permanentRecordingMediaNode->mediaEndTime));
        duration = tsbService->permanentRecordingMediaNode->mediaEndTime-
            tsbService->permanentRecordingMediaNode->mediaStartTime;
        duration = duration/1000;
        duration = duration/60;
        BDBG_WRN((" duration:%u mins",duration));
        BDBG_WRN((" media - startOffset:%lld endOffset:%lld",
                  tsbService->permanentRecordingMediaNode->mediaLinearStartOffset,
                  tsbService->permanentRecordingMediaNode->mediaLinearEndOffset));
        len = tsbService->permanentRecordingMediaNode->mediaLinearEndOffset - 
            tsbService->permanentRecordingMediaNode->mediaLinearStartOffset;
        BDBG_WRN((" mediaLength:%lld",len));
        BDBG_WRN((" navigation - startOffset:%lld endOffset:%lld",
                  tsbService->permanentRecordingMediaNode->navLinearStartOffset,
                  tsbService->permanentRecordingMediaNode->navLinearEndOffset));
        numOfFrames = (tsbService->permanentRecordingMediaNode->navLinearEndOffset - 
                       tsbService->permanentRecordingMediaNode->navLinearStartOffset)/sizeOfNavEntry;
        BDBG_WRN((" numOfVideoFrames:%u",numOfFrames));
        BDBG_WRN((" ***************************"));
    }

    NEXUS_Recpump_GetStatus(tsbService->recpump,&recpumpStatus);
    BDBG_WRN((" ***************************"));
    BDBG_WRN(("media FIFO Depth:%u",recpumpStatus.data.fifoDepth));
    BDBG_WRN(("media FIFO Size:%u",recpumpStatus.data.fifoSize));
    
    BDBG_WRN(("index FIFO Depth:%u",recpumpStatus.index.fifoDepth));
    BDBG_WRN(("index FIFO Size:%u",recpumpStatus.index.fifoSize));
    BDBG_WRN((" ***************************"));
    return;
}

void B_DVR_TSBService_P_PrintRecInfo(B_DVR_TSBServiceHandle tsbService, bool permRec)
{

    unsigned index=0;
    BDBG_WRN((" *****************************"));
    if(permRec) 
    {
        BDBG_WRN((" tsbConv program:%s",
                  tsbService->permanentRecordingMediaNode->programName));
        BDBG_WRN((" tsbConv %s",tsbService->tsbConversion?"started":"stopped"));
        BDBG_WRN((" startTime:%lums endTime:%lums",
                  tsbService->permRecReq.recStartTime,
                  tsbService->permRecReq.recEndTime));
    }
    else
    {
        BDBG_WRN((" tsb program:%s",
                  tsbService->tsbMediaNode->programName));
        BDBG_WRN((" tsb %s",tsbService->recpumpStarted?"started":"stopped"));
    }
    BDBG_WRN((" parserBand:%u",tsbService->tsbServiceSettings.parserBand));
    BDBG_WRN((" numberofPIDs:%u",tsbService->tsbMediaNode->esStreamCount));
    BDBG_WRN((" encrypted:%s",tsbService->mediaProbeDrm?"true":"false"));
    for(index=0;index<tsbService->tsbMediaNode->esStreamCount;index++) 
    {
        BDBG_WRN((" PID:%u",tsbService->tsbMediaNode->esStreamInfo[index].pid));
        if(tsbService->tsbMediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeVideo) 
        {
            BDBG_WRN((" pidtype:video codec:%u",tsbService->tsbMediaNode->esStreamInfo[index].codec.videoCodec));
        }
        else
        {
            if(tsbService->tsbMediaNode->esStreamInfo[index].pidType == eB_DVR_PidTypeAudio) 
            {
                BDBG_WRN((" pidtype:audio codec:%u",
                         tsbService->tsbMediaNode->esStreamInfo[index].codec.audioCodec));
            }
            else
            {
                BDBG_WRN((" pidtype: data"));
            }
        }
    }
    BDBG_WRN((" ******************************"));
    return;
}

static void B_DVR_TSBService_P_Overflow(void *tsbContext,int param)
{
    B_DVR_TSBServiceHandle tsbService = (B_DVR_TSBServiceHandle)tsbContext;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecpumpStatus recpumpStatus;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BSTD_UNUSED(param);
    BDBG_WRN(("B_DVR_TSBService_P_Overflow tsb program %s tsb index %u >>>",
              tsbService->tsbMediaNode->programName,
              tsbService->index));

    B_Mutex_Lock(tsbService->schedulerMutex);
    B_Mutex_Lock(tsbService->tsbMutex);
    if(!tsbService->tsbRecordStarted) 
    {
        BDBG_WRN(("index data not available yet"));
        goto error;
    }
    tsbService->overflow=true;
    NEXUS_Recpump_GetStatus(tsbService->recpump,&recpumpStatus);
    BDBG_WRN(("media FIFO Depth:%u",recpumpStatus.data.fifoDepth));
    BDBG_WRN(("media FIFO Size:%u",recpumpStatus.data.fifoSize));
    BDBG_WRN(("index FIFO Depth:%u",recpumpStatus.index.fifoDepth));
    BDBG_WRN(("index FIFO Size:%u",recpumpStatus.index.fifoSize));

    NEXUS_Record_GetSettings(tsbService->record, &recordSettings);
    recordSettings.overflowCallback.callback = NULL;
    recordSettings.overflowCallback.context = NULL;
    recordSettings.overflowCallback.param = 0;
    NEXUS_Record_SetSettings(tsbService->record, &recordSettings);
    B_Mutex_Unlock(tsbService->tsbMutex);
    if(tsbService->registeredCallback)
    {
        event = eB_DVR_EventOverFlow;
        service = eB_DVR_ServiceTSB;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_MSG(("tsb overflow callback sent"));
    }
    B_Mutex_Lock(tsbService->tsbMutex);
    rc = B_DVR_TSBService_P_Pause(tsbService);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("B_DVR_TSBService_P_Pause failed"));
        goto error;
    }
    NEXUS_Record_GetSettings(tsbService->record, &recordSettings);
    recordSettings.overflowCallback.callback = B_DVR_TSBService_P_Overflow;
    recordSettings.overflowCallback.context = tsbService;
    recordSettings.overflowCallback.param = tsbService->index;
    NEXUS_Record_SetSettings(tsbService->record, &recordSettings);
    rc = B_DVR_TSBService_P_Resume(tsbService);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("B_DVR_TSBService_P_Resume failed"));
        goto error;
    }
    tsbService->overflow = false;
error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    B_Mutex_Unlock(tsbService->schedulerMutex);
    if(rc!=B_DVR_SUCCESS && tsbService->registeredCallback)
    {
        event = eB_DVR_EventAbortTSBRecord;
        service = eB_DVR_ServiceTSB;
        tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
        BDBG_WRN(("tsb abort callback sent"));
    }
    BDBG_WRN(("B_DVR_TSBService_P_Overflow <<<"));
    return;
}
B_DVR_ERROR B_DVR_TSBService_P_Pause(B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_FilePosition tsbStartPosition,tsbEndPosition;
    B_DVR_SegmentedFileRecordSettings segmentedFileRecordSettings;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord=NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_FilePosition position;
    unsigned long timeStamp=0;
    off_t mediaLinearOffset=0;
    unsigned mediaSegmentNum1=0,mediaSegmentNum2=0;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_P_Pause>>>"));
    if(tsbService->tsbEventSchedulerID) 
    {
        B_Scheduler_UnregisterEvent(dvrManager->scheduler,tsbService->tsbEventSchedulerID);
    }
    B_Event_Reset(tsbService->tsbRecordEvent);
    segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    tsbService->tsbEventSchedulerID = NULL;
    if(tsbService->recpumpStarted)
    {
        NEXUS_Record_Stop(tsbService->record);
    }
    tsbService->recpumpStarted = false;
    tsbService->tsbRecordStarted = false;

    BDBG_MSG(("media time: recStart %ld recEnd %ld",
              tsbService->tsbMediaNode->mediaStartTime,
              tsbService->tsbMediaNode->mediaEndTime));
    BDBG_MSG(("media Offset: recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->mediaLinearStartOffset,
              tsbService->tsbMediaNode->mediaLinearEndOffset));
    BDBG_MSG(("nav offset : recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->navLinearStartOffset,
              tsbService->tsbMediaNode->navLinearEndOffset));

    rc = B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbStartPosition,&tsbEndPosition);
    if(rc != B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s:B_DVR_SegmentedFileRecord_GetBounds failed",__FUNCTION__));
        BDBG_ERR(("tsb program name %s index:%u",tsbService->tsbMediaNode->programName,tsbService->index));
        BDBG_ERR(("media time: recStart %ld recEnd %ld",
                tsbService->tsbMediaNode->mediaStartTime,
                tsbService->tsbMediaNode->mediaEndTime));
        BDBG_ERR(("media Offset: recStart %lld recEnd %lld",
                tsbService->tsbMediaNode->mediaLinearStartOffset,
                tsbService->tsbMediaNode->mediaLinearEndOffset));
        BDBG_ERR(("nav offset : recStart %lld recEnd %lld",
                tsbService->tsbMediaNode->navLinearStartOffset,
                tsbService->tsbMediaNode->navLinearEndOffset));
        rc= B_DVR_UNKNOWN;
        goto error;
     }
     tsbService->tsbMediaNode->mediaLinearStartOffset = tsbStartPosition.mpegFileOffset;
     tsbService->tsbMediaNode->mediaLinearEndOffset =  tsbEndPosition.mpegFileOffset;
     tsbService->tsbMediaNode->navLinearStartOffset = tsbStartPosition.navFileOffset;
     tsbService->tsbMediaNode->navLinearEndOffset = tsbEndPosition.navFileOffset;
     tsbService->tsbMediaNode->mediaStartTime = tsbStartPosition.timestamp;
     tsbService->tsbMediaNode->mediaEndTime = tsbEndPosition.timestamp;
    
    mediaLinearOffset = tsbService->tsbMediaNode->mediaLinearEndOffset;
    BDBG_MSG(("mediaLinearOffset before alignment %lld",mediaLinearOffset));
    mediaSegmentNum1 = mediaLinearOffset/B_DVR_MEDIA_SEGMENT_SIZE;
    BDBG_MSG(("mediaSegNo1:%u",mediaSegmentNum1));
    mediaLinearOffset-= (mediaLinearOffset%(4096*(188/4)));
    BDBG_MSG(("mediaLinearOffset after alignment %lld",mediaLinearOffset));
    mediaSegmentNum2 = mediaLinearOffset/B_DVR_MEDIA_SEGMENT_SIZE;
    BDBG_MSG(("mediaSegNo2:%u",mediaSegmentNum2));
    if(mediaSegmentNum1!=mediaSegmentNum2) 
    {
        tsbService->tsbMediaNode->mediaLinearEndOffset -= 
            (tsbService->tsbMediaNode->mediaLinearEndOffset%B_DVR_MEDIA_SEGMENT_SIZE);
    }
    else
    {
        tsbService->tsbMediaNode->mediaLinearEndOffset = mediaLinearOffset;
    }
    BDBG_MSG(("final mediaLinearOffset %lld",tsbService->tsbMediaNode->mediaLinearEndOffset));

    rc = B_DVR_SegmentedFileRecord_GetTimestamp(tsbService->nexusFileRecord,
                                           tsbService->tsbMediaNode->mediaLinearEndOffset,
                                           &timeStamp);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s: B_DVR_SegmentedFileRecord_GetTimestamp failed",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    rc = B_DVR_SegmentedFileRecord_GetLocation(tsbService->nexusFileRecord, 
                                          -1,
                                          timeStamp,
                                          &position);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("%s: B_DVR_SegmentedFileRecord_GetLocation failed",__FUNCTION__));
        rc = B_DVR_UNKNOWN;
        goto error;
    }
    rc = B_DVR_SegmentedFileRecord_GetSettings(tsbService->nexusFileRecord,
                                               &segmentedFileRecordSettings);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in B_DVR_SegmentedFileRecord_GetSettings for %s",
                  tsbService->tsbMediaNode->programName));
        goto error;
    }

    tsbService->tsbMediaNode->mediaEndTime = position.timestamp;
    tsbService->tsbMediaNode->navLinearEndOffset = position.navFileOffset 
                                                   + segmentedFileRecordSettings.navEntrySize;
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               tsbService->tsbMediaNode,
                               tsbService->tsbServiceRequest.volumeIndex,false);

    BDBG_MSG(("media time: recStart %ld recEnd %ld",
              tsbService->tsbMediaNode->mediaStartTime,
              tsbService->tsbMediaNode->mediaEndTime));
    BDBG_MSG(("media Offset: recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->mediaLinearStartOffset,
              tsbService->tsbMediaNode->mediaLinearEndOffset));
    BDBG_MSG(("nav offset : recStart %lld recEnd %lld",
              tsbService->tsbMediaNode->navLinearStartOffset,
              tsbService->tsbMediaNode->navLinearEndOffset));

    segmentedFileRecordSettings.mediaLinearEndOffset = tsbService->tsbMediaNode->mediaLinearEndOffset;
    segmentedFileRecordSettings.navLinearEndOffset = tsbService->tsbMediaNode->navLinearEndOffset;
    rc = B_DVR_SegmentedFileRecord_SetSettings(tsbService->nexusFileRecord,
                                               &segmentedFileRecordSettings);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in syncing the media & nav offsets after pausing the TSB service for %s",
                  tsbService->tsbMediaNode->programName));
    }
    tsbService->tsbMediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED;
error:
    BDBG_MSG(("B_DVR_TSBService_P_Pause<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_P_Resume(B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_RecordAppendSettings nexusRecordAppendSettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_P_Resume>>>"));
    tsbService->tsbEventSchedulerID =  B_Scheduler_RegisterEvent(dvrManager->scheduler,
                                                                 tsbService->schedulerMutex,
                                                                 tsbService->tsbRecordEvent,
                                                                 B_DVR_TSBService_P_UpdateMediaNode,
                                                                 (void*)tsbService);
    if(!tsbService->tsbEventSchedulerID)
    {
        BDBG_ERR(("%s event registration  failed", __FUNCTION__));
        rc=B_DVR_OS_ERROR;
        goto error_event_registration;
    }

    nexusRecordAppendSettings.startingOffset = tsbService->tsbMediaNode->mediaLinearEndOffset;
    nexusRecordAppendSettings.timestamp = tsbService->tsbMediaNode->mediaEndTime;
    NEXUS_Record_StartAppend(tsbService->record,
                             tsbService->nexusFileRecord,
                             &nexusRecordAppendSettings);
    tsbService->recpumpStarted = true;
    tsbService->tsbMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED);
error_event_registration:
    BDBG_MSG(("B_DVR_TSBService_P_Resume<<<"));
    return rc;
}

B_DVR_TSBServiceHandle B_DVR_TSBService_Open(
    B_DVR_TSBServiceRequest *tsbServiceRequest)
{
    B_DVR_TSBServiceHandle tsbService=NULL;
    B_DVR_ManagerHandle dvrManager;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    B_DVR_SegmentedFileSettings segmentedFileSettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned index=0;
    
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT((tsbServiceRequest));
    BDBG_ASSERT((tsbServiceRequest->maxTSBBufferCount));
    BDBG_ASSERT((tsbServiceRequest->maxTSBTime));
    BDBG_MSG(("B_DVR_TSBService_Open >>>"));

    if(dvrManager->tsbServiceCount >= B_DVR_MAX_TSB)
    {
        BDBG_WRN(("TSB service max count reached %u",dvrManager->tsbServiceCount));
        goto error_tsbServiceCount;
    }
    tsbService = BKNI_Malloc(sizeof(*tsbService));
    if (!tsbService)
    {
        BDBG_ERR(("%s  tsbService alloc failed", __FUNCTION__));
        goto error_alloc1;
    }
    BKNI_Memset(tsbService, 0, sizeof(*tsbService));
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*tsbService),
                                            true,__FUNCTION__,__LINE__);
    BDBG_OBJECT_SET(tsbService,B_DVR_TSBService);
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    /* find a tsbService slot in the dvrManager */
    for(index=0;index<B_DVR_MAX_TSB;index++)
    {
        if(!dvrManager->tsbService[index])
        {
            BDBG_MSG(("TSB Service Index %u",index));
            tsbService->index=index;
            break;
        }
    }
    if(index>=B_DVR_MAX_TSB)
    {
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        BDBG_ERR(("MAX TSBService instances used up. Free up a TSBService instance"));
        goto error_maxTSBServiceCount;
    }
    dvrManager->tsbServiceCount++;
    dvrManager->tsbService[tsbService->index] = tsbService;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
    mediaNodeSettings.programName = tsbServiceRequest->programName;
    mediaNodeSettings.subDir = tsbServiceRequest->subDir;
    mediaNodeSettings.volumeIndex = tsbServiceRequest->volumeIndex;
#ifdef MEDIANODE_ONDEMAND_CACHING
    tsbService->tsbMediaNode = BKNI_Malloc(sizeof(*tsbService->tsbMediaNode));
    if(!tsbService->tsbMediaNode) 
    {
        BDBG_ERR(("error in allocating tsbService->mediaNode for %s",
                  tsbServiceRequest->programName));
        goto error_mediaNode;
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*tsbService->tsbMediaNode),
                                            true, __FUNCTION__,__LINE__);
    rc = B_DVR_List_GetMediaNodeFile(dvrManager->dvrList,
                                     &mediaNodeSettings,
                                     tsbService->tsbMediaNode);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in getting mediaNode for %s",
                  tsbServiceRequest->programName));
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*tsbService->tsbMediaNode),
                                                false, __FUNCTION__,__LINE__);
        BKNI_Free(tsbService->tsbMediaNode);
        tsbService->tsbMediaNode = NULL;
        goto error_mediaNode;
    }
    rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,
                                 mediaNodeSettings.volumeIndex,
                                 tsbService->tsbMediaNode);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("error in adding mediaNode for %s to the dvr list",
                  tsbServiceRequest->programName));
        BDBG_OBJECT_DESTROY(tsbService->tsbMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                               sizeof(*tsbService->tsbMediaNode),
                                               false, __FUNCTION__,__LINE__);
        BKNI_Free(tsbService->tsbMediaNode);
        tsbService->tsbMediaNode = NULL;
    }
    #else
    tsbService->tsbMediaNode = B_DVR_List_GetMediaNode(dvrManager->dvrList,&mediaNodeSettings,true);
    #endif
    if (!tsbService->tsbMediaNode)
    {
        BDBG_ERR(("pre_allocated mediaNode not found for %s/%s",tsbServiceRequest->subDir,tsbServiceRequest->programName));
        goto error_mediaNode;
    }
    tsbService->tsbMediaNode->recording = eB_DVR_RecordingTSB;
    tsbService->tsbMediaNode->transportType = NEXUS_TransportType_eTs;
    tsbService->tsbMediaNode->esStreamCount=0;
    tsbService->tsbMediaNode->mediaLinearEndOffset=0;
    tsbService->tsbMediaNode->mediaLinearStartOffset =0;
    tsbService->tsbMediaNode->mediaStartTime=0;
    tsbService->tsbMediaNode->mediaEndTime=0;
    tsbService->tsbMediaNode->navLinearStartOffset=0;
    tsbService->tsbMediaNode->navLinearEndOffset=0;
    tsbService->overflow = false;
    tsbService->tsbMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED);
    BKNI_Memcpy((void*)&tsbService->tsbServiceRequest,(void *)tsbServiceRequest, sizeof(*tsbServiceRequest));
    tsbService->tsbMutex = B_Mutex_Create(NULL);
    if( !tsbService->tsbMutex)
    {
        BDBG_ERR(("%s mutex create failed", __FUNCTION__));
        goto error_mutex;
    }

    tsbService->schedulerMutex = B_Mutex_Create(NULL);
    if( !tsbService->schedulerMutex)
    {
        BDBG_ERR(("%s schedulerMutex create error", __FUNCTION__));
        goto error_schedulerMutex;
    }

    tsbService->tsbRecordEvent = B_Event_Create(NULL);
    if(!tsbService->tsbRecordEvent )
    {
        BDBG_ERR(("%s event create failed", __FUNCTION__));
        goto error_event;
    }

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    if(tsbServiceRequest->recpumpCdbHeap) 
    {
       recpumpOpenSettings.data.heap = tsbServiceRequest->recpumpCdbHeap;
    }
    if(tsbServiceRequest->recpumpIndexHeap) 
    {
        recpumpOpenSettings.index.heap = tsbServiceRequest->recpumpIndexHeap;
    }
    if(tsbServiceRequest->input == eB_DVR_TSBServiceInputIp) 
    {
        /* Set threshold to 80%. with band hold enabled, it's not actually a dataready threshold. it's
           a bandhold threshold. we are relying on the timer that's already in record. */
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 8 / 10;
        recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 8 / 10;
    }
    else
    {
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.dataReadyThreshold*4;
        recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.dataReadyThreshold*4;
    }
    tsbService->recpump = NEXUS_Recpump_Open(tsbServiceRequest->recpumpIndex,&recpumpOpenSettings);
    if(!tsbService->recpump)
    {
        BDBG_ERR(("%s recpump open failed %d", __FUNCTION__,dvrManager->recordServiceCount));
        goto error_recpump;
    }

    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.data.bufferSize,
                                            true,__FUNCTION__,__LINE__);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.index.bufferSize,
                                            true,__FUNCTION__,__LINE__);
    BDBG_MSG(("CDB size %u threshold %u",recpumpOpenSettings.data.bufferSize,recpumpOpenSettings.data.dataReadyThreshold));
    BDBG_MSG(("ITB size %u threshold %u",recpumpOpenSettings.index.bufferSize,recpumpOpenSettings.index.dataReadyThreshold));
    tsbService->record = NEXUS_Record_Create();
    if(!tsbService->record)
    {
        BDBG_ERR(("%s record create failed %d", __FUNCTION__,dvrManager->recordServiceCount));
        goto error_record;
    }
    
    BDBG_MSG(("nav %s media %s info %s subDir %s",
              tsbService->tsbMediaNode->navFileName,
              tsbService->tsbMediaNode->mediaFileName,
              tsbService->tsbMediaNode->mediaNodeFileName,
              tsbServiceRequest->subDir
              ));

    rc = B_DVR_List_UpdateMediaNode(dvrManager->dvrList,tsbService->tsbMediaNode,tsbServiceRequest->volumeIndex,false);
    if(rc!=B_DVR_SUCCESS)
    {
        goto error_add_media;
    }
    NEXUS_Record_GetSettings(tsbService->record, &recordSettings);
    recordSettings.recpump = tsbService->recpump;
    recordSettings.pollingTimer = B_DVR_RECORD_POLLING_INTERVAL/5; /* 50ms initially */
    recordSettings.allowLargeTimeGaps=true;
    recordSettings.overflowCallback.callback = B_DVR_TSBService_P_Overflow;
    recordSettings.overflowCallback.context = tsbService;
    recordSettings.overflowCallback.param = tsbService->index;
    if(tsbServiceRequest->input == eB_DVR_TSBServiceInputIp) 
    {
        /* Enable bandhold. Required for record from playpump (IP source) */
        recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    }
    NEXUS_Record_SetSettings(tsbService->record, &recordSettings);
    segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
    segmentedFileSettings.maxSegmentCount = tsbService->tsbServiceRequest.maxTSBBufferCount;
    segmentedFileSettings.registeredCallback = B_DVR_TSBService_P_SegmentedFileEvent;
    segmentedFileSettings.volumeIndex = tsbService->tsbServiceRequest.volumeIndex;
    segmentedFileSettings.mediaSegmentSize = dvrManager->mediaSegmentSize;
    segmentedFileSettings.serviceIndex = tsbService->index;
    segmentedFileSettings.service = eB_DVR_ServiceTSB;
    segmentedFileSettings.event =  tsbService->tsbRecordEvent;
    segmentedFileSettings.itbThreshhold = recpumpOpenSettings.index.dataReadyThreshold;
    segmentedFileSettings.maxTSBTime = tsbServiceRequest->maxTSBTime;
    segmentedFileSettings.cdbSize = recpumpOpenSettings.data.bufferSize;
    segmentedFileSettings.itbSize = recpumpOpenSettings.index.bufferSize;
    if(tsbService->tsbMediaNode->mediaNodeSubDir[0]=='\0')
    {
        segmentedFileSettings.metaDataSubDir = NULL;
    }
    else
    {
        segmentedFileSettings.metaDataSubDir = (char *)tsbService->tsbMediaNode->mediaNodeSubDir; 
    }
    tsbService->nexusFileRecord = B_DVR_SegmentedFileRecord_Open(tsbService->tsbMediaNode->mediaFileName,
                                                                 tsbService->tsbMediaNode->navFileName,
                                                                 &segmentedFileSettings);
    BDBG_MSG(("segment size %d",segmentedFileSettings.mediaSegmentSize));

    if(!tsbService->nexusFileRecord)
    {
      BDBG_ERR(("nexusFileRecord open failed %s %s",tsbService->tsbMediaNode->mediaFileName,tsbService->tsbMediaNode->navFileName));
      rc = B_DVR_UNKNOWN;
      goto error_nexusFileRecord;
    }
        
    segmentedFileSettings.itbThreshhold =0; /*This is used for updating the mediaNode periodically only during recording */
    tsbService->nexusFilePlay = B_DVR_SegmentedFilePlay_Open(tsbService->tsbMediaNode->mediaFileName,
                                                             tsbService->tsbMediaNode->navFileName,
                                                             &segmentedFileSettings,
                                                             tsbService->nexusFileRecord);
    if(tsbService->nexusFilePlay == NULL)
    {
        BDBG_ERR(("nexusFilePlay open failed %s %s",tsbService->tsbMediaNode->mediaFileName,
                  tsbService->tsbMediaNode->navFileName));
        rc = B_DVR_UNKNOWN;
        goto error_nexusFilePlay;
    }

    tsbService->nexusFilePlay1 = B_DVR_SegmentedFilePlay_Open(tsbService->tsbMediaNode->mediaFileName,
                                                             tsbService->tsbMediaNode->navFileName,
                                                             &segmentedFileSettings,
                                                             tsbService->nexusFileRecord);
    if(tsbService->nexusFilePlay1 == NULL)
    {
        BDBG_ERR(("nexusFilePlay open failed %s %s",tsbService->tsbMediaNode->mediaFileName,
                  tsbService->tsbMediaNode->navFileName));
        rc = B_DVR_UNKNOWN;
        goto error_nexusFilePlay1;
    }
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    if(tsbServiceRequest->playpumpHeap) 
    {
        playpumpOpenSettings.heap = tsbServiceRequest->playpumpHeap;
    }
    tsbService->playpump = NEXUS_Playpump_Open(tsbServiceRequest->playpumpIndex, &playpumpOpenSettings);
    if(!tsbService->playpump)
    {
        BDBG_ERR(("%s playpump open failed %d", __FUNCTION__,dvrManager->playbackServiceCount));
        goto error_playpump;
    }

    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpOpenSettings.fifoSize,
                                            true,__FUNCTION__,__LINE__);
    tsbService->playback = NEXUS_Playback_Create();
    tsbService->tsbServiceSettings.tsbPlaybackSettings.playback= tsbService->playback;
    if(!tsbService->playback)
    {
        BDBG_ERR(("%s playback create failed %d", __FUNCTION__,dvrManager->playbackServiceCount));
        goto error_playback;
    }
    
    NEXUS_Playback_GetSettings(tsbService->playback, &playbackSettings);
    playbackSettings.playpump = tsbService->playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.beginningOfStreamCallback.callback =  B_DVR_TSBService_P_PlaybackStartOfStream;
    playbackSettings.beginningOfStreamCallback.context = tsbService;
    playbackSettings.beginningOfStreamCallback.param = tsbService->index;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.endOfStreamCallback.callback = B_DVR_TSBService_P_PlaybackEndOfStream;
    playbackSettings.endOfStreamCallback.context = tsbService;
    playbackSettings.endOfStreamCallback.param = tsbService->index;
    playbackSettings.timeshifting = true;
    NEXUS_Playback_SetSettings(tsbService->playback, &playbackSettings);
         
    BDBG_MSG(("B_DVR_TSBService_Open index %d count %u <<<",tsbService->index,dvrManager->tsbServiceCount));
    return tsbService;

error_playback:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpOpenSettings.fifoSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Playpump_Close(tsbService->playpump);
error_playpump:
    B_DVR_SegmentedFilePlay_Close(tsbService->nexusFilePlay1);
error_nexusFilePlay1:
    B_DVR_SegmentedFilePlay_Close(tsbService->nexusFilePlay);
error_nexusFilePlay:
    B_DVR_SegmentedFileRecord_Close(tsbService->nexusFileRecord);
error_nexusFileRecord:
error_add_media:
    NEXUS_Record_Destroy(tsbService->record);
error_record:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.data.bufferSize,
                                            false,__FUNCTION__,__LINE__);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpOpenSettings.index.bufferSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Recpump_Close(tsbService->recpump);
error_recpump:
    B_Event_Destroy(tsbService->tsbRecordEvent);
error_event:
    B_Mutex_Destroy(tsbService->schedulerMutex);
error_schedulerMutex:
    B_Mutex_Destroy(tsbService->tsbMutex);
error_mutex:
error_mediaNode:
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->tsbServiceCount--;
    dvrManager->tsbService[tsbService->index] = NULL;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
error_maxTSBServiceCount:
    BDBG_OBJECT_DESTROY(tsbService,B_DVR_TSBService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*tsbService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(tsbService);    
error_alloc1:
error_tsbServiceCount:
    BDBG_MSG(("B_DVR_TSBService_Open <<<"));
    return NULL;
}


B_DVR_ERROR B_DVR_TSBService_Close(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    unsigned esStreamIndex=0;
    NEXUS_RecpumpStatus recpumpStatus;
    NEXUS_PlaypumpStatus playpumpStatus;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_Close %d >>>",tsbService->index));

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->tsbService[tsbService->index] = NULL;
    dvrManager->tsbServiceCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(tsbService->tsbMutex);
    NEXUS_Playback_Destroy(tsbService->playback);
    NEXUS_Playpump_GetStatus(tsbService->playpump,&playpumpStatus);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpStatus.fifoSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Playpump_Close(tsbService->playpump);
    B_DVR_SegmentedFilePlay_Close(tsbService->nexusFilePlay);
    B_DVR_SegmentedFilePlay_Close(tsbService->nexusFilePlay1);
    B_DVR_SegmentedFileRecord_Close(tsbService->nexusFileRecord);
    NEXUS_Record_RemoveAllPidChannels(tsbService->record);
    if((tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam) ||
       (tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputIp)) 
    {
        for(esStreamIndex=0;esStreamIndex< tsbService->activeEsStreamCount;esStreamIndex++)
        {
            if(tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam)
            { 
                NEXUS_PidChannel_Close(tsbService->recordPidChannels[esStreamIndex]);
            }
            else
            {
                NEXUS_Playpump_ClosePidChannel(tsbService->tsbServiceSettings.playpumpIp,
                                               tsbService->recordPidChannels[esStreamIndex]);
            }
        }
    }
    NEXUS_Record_Destroy(tsbService->record);
    NEXUS_Recpump_GetStatus(tsbService->recpump,&recpumpStatus);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpStatus.openSettings.data.bufferSize,
                                            false,__FUNCTION__,__LINE__);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            recpumpStatus.openSettings.index.bufferSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Recpump_Close(tsbService->recpump);
    tsbService->tsbMediaNode->mediaAttributes=0;
    tsbService->tsbMediaNode->mediaAttributes |=B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM;
    tsbService->tsbMediaNode->esStreamCount=0;
    BKNI_Memset((void*)tsbService->tsbMediaNode->esStreamInfo,0,sizeof(B_DVR_EsStreamInfo)* B_DVR_MAX_PIDS);
    tsbService->tsbMediaNode->mediaEndTime=0;
    tsbService->tsbMediaNode->mediaStartTime=0;
    tsbService->tsbMediaNode->mediaLinearStartOffset=0;
    tsbService->tsbMediaNode->mediaLinearEndOffset=0;
    tsbService->tsbMediaNode->navLinearEndOffset=0;
    tsbService->tsbMediaNode->navLinearStartOffset=0;
    #ifdef MEDIANODE_ONDEMAND_CACHING
    rc = B_DVR_List_RemoveMediaNode(dvrManager->dvrList,tsbService->tsbServiceRequest.volumeIndex,tsbService->tsbMediaNode);
    if(rc == B_DVR_SUCCESS) {
    BDBG_OBJECT_DESTROY(tsbService->tsbMediaNode,B_DVR_Media);
    BKNI_Free(tsbService->tsbMediaNode);
    }
    #endif
    B_Event_Destroy(tsbService->tsbRecordEvent);
    B_Mutex_Destroy(tsbService->schedulerMutex);
    B_Mutex_Unlock(tsbService->tsbMutex);
    B_Mutex_Destroy(tsbService->tsbMutex);
    BDBG_OBJECT_DESTROY(tsbService,B_DVR_TSBService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*tsbService),
                                            false,__FUNCTION__,__LINE__);
    BKNI_Free(tsbService);
    BDBG_MSG(("B_DVR_TSBService_Close tsbCount %u<<<",dvrManager->tsbServiceCount));
    return rc;
}


B_DVR_ERROR B_DVR_TSBService_Start(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *tsbServiceSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_SegmentedFileRecordSettings fileRecordSettings;
    B_DVR_SegmentedFilePlaySettings filePlaySettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(tsbService->activeEsStreamCount);
    BDBG_MSG(("B_DVR_TSBService_Start >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    tsbService->tsbEventSchedulerID =  B_Scheduler_RegisterEvent(dvrManager->scheduler,
                                                                 tsbService->schedulerMutex,
                                                                 tsbService->tsbRecordEvent,
                                                                 B_DVR_TSBService_P_UpdateMediaNode,
                                                                 (void*)tsbService);
    if(!tsbService->tsbEventSchedulerID)
    {
        BDBG_ERR(("%s event registration  failed", __FUNCTION__));
        rc=B_DVR_OS_ERROR;
        goto error_event_registration;
    }
    if(tsbServiceSettings)
    {
        unsigned esStreamIndex=0;
        NEXUS_PidChannelRemapSettings remapSettings;
        BDBG_ASSERT(tsbServiceSettings->tsbRecordSettings.esStreamCount == tsbService->tsbMediaNode->esStreamCount);
        for(esStreamIndex=0;esStreamIndex<tsbService->tsbMediaNode->esStreamCount;esStreamIndex++ )
        {
            remapSettings.enabled = true;
            remapSettings.continuityCountEnabled = true;
            remapSettings.pid = tsbServiceSettings->tsbRecordSettings.RemappedEsStreamInfo[esStreamIndex].pid;
            NEXUS_PidChannel_SetRemapSettings(tsbService->recordPidChannels[esStreamIndex],&remapSettings);
            tsbService->tsbMediaNode->esStreamInfo[esStreamIndex].pid = tsbServiceSettings->tsbRecordSettings.RemappedEsStreamInfo[esStreamIndex].pid;
        }
    }

    B_DVR_SegmentedFileRecord_GetDefaultSettings(&fileRecordSettings);
    B_DVR_SegmentedFilePlay_GetDefaultSettings(&filePlaySettings);
    if(tsbService->mpeg2Video)
    { 
        filePlaySettings.navEntrySize = fileRecordSettings.navEntrySize = sizeof(BNAV_Entry);
    }
    else
    {
        filePlaySettings.navEntrySize = fileRecordSettings.navEntrySize = sizeof(BNAV_AVC_Entry);
    }
    B_DVR_SegmentedFileRecord_SetSettings(tsbService->nexusFileRecord,&fileRecordSettings);
    B_DVR_SegmentedFilePlay_SetSettings(tsbService->nexusFilePlay,&filePlaySettings);
    B_DVR_SegmentedFilePlay_SetSettings(tsbService->nexusFilePlay1,&filePlaySettings);

    tsbService->mediaProbe = B_DVR_MediaProbe_Open(tsbService->tsbMediaNode,
                                                   tsbService->nexusFileRecord,
                                                   tsbService->tsbServiceRequest.volumeIndex,
                                                   tsbService->index);
    if(!tsbService->mediaProbe)
    {
        BDBG_ERR(("mediaProbe open failed"));
    }
    NEXUS_Record_Start(tsbService->record,tsbService->nexusFileRecord);
    tsbService->recpumpStarted = true;
    B_DVR_TSBService_P_PrintRecInfo(tsbService,false);    
error_event_registration:
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_Start <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_Stop(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_Stop >>>>"));

    B_Mutex_Lock(tsbService->schedulerMutex);
    if(tsbService->tsbEventSchedulerID) 
    {
        B_Scheduler_UnregisterEvent(dvrManager->scheduler,tsbService->tsbEventSchedulerID);
    }
    B_Event_Reset(tsbService->tsbRecordEvent);
    if(tsbService->tsbPlayAlarmTimer) 
    {
        B_Scheduler_CancelTimer(dvrManager->scheduler,tsbService->tsbPlayAlarmTimer);
    }
    B_Mutex_Unlock(tsbService->schedulerMutex);
    if(tsbService->tsbConversion)
    {
        B_DVR_TSBService_ConvertStop(tsbService);
    }
    if(tsbService->dataInjectionStarted)
    {
        B_DVR_TSBService_InjectDataStop(tsbService);
    }

    B_Mutex_Lock(tsbService->tsbMutex);
    tsbService->tsbEventSchedulerID = NULL;
    if(tsbService->recpumpStarted) 
    {
        NEXUS_Record_Stop(tsbService->record);
    }
    if(tsbService->tsbPlaybackStarted)
    {
        B_DVR_TSBService_P_PlaybackStop(tsbService);
    }
    if(tsbService->mediaProbe)
    {
        B_DVR_MediaProbe_Close(tsbService->mediaProbe);
        tsbService->mediaProbe = NULL;
        tsbService->probeOffset=0;
    }
    tsbService->recpumpStarted = false;
    tsbService->tsbRecordStarted = false;
    B_DVR_TSBService_P_PrintRecInfo(tsbService,false);
    B_Mutex_Unlock(tsbService->tsbMutex);

    if(tsbService->registeredCallback)
     {
         event = eB_DVR_EventEndOfRecording;
         service = eB_DVR_ServiceTSB;
         tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
     }
    
    BDBG_MSG(("B_DVR_TSBService_Stop <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_SetOperation(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_OperationSettings *operationSettings)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;

    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_SetOperation %d op:%d >>>>",tsbService->index,operationSettings->operation));

    if(operationSettings->operation == eB_DVR_OperationTSBPlayStop) 
    {
        B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
        B_Mutex_Lock(tsbService->schedulerMutex);
        if(tsbService->tsbPlayAlarmTimer) 
        {
            B_Scheduler_CancelTimer(dvrManager->scheduler,tsbService->tsbPlayAlarmTimer);
        }
        B_Mutex_Unlock(tsbService->schedulerMutex);
    }

    B_Mutex_Lock(tsbService->tsbMutex);
    BKNI_Memcpy((void*)&tsbService->operationSettings,operationSettings,sizeof(*operationSettings));
    switch(operationSettings->operation)
    {
    case eB_DVR_OperationTSBPlayStart:
        {
           BDBG_MSG(("eB_DVR_OperationTSBPlayStart"));
           B_DVR_TSBService_P_PlaybackStart(tsbService,operationSettings->seekTime);
        }
        break;
    case eB_DVR_OperationTSBPlayStop:
        {
           BDBG_MSG(("eB_DVR_OperationTSBPlayStop"));
           B_DVR_TSBService_P_PlaybackStop(tsbService);
        }
        break;
    case eB_DVR_OperationPause:
        {
           BDBG_MSG(("eB_DVR_OperationPause"));
           if(tsbService->tsbPlaybackStarted)
           {
               NEXUS_Playback_Pause(tsbService->playback);
           }
           else
           {
               BDBG_MSG(("live mode pause invalid"));
           }
       }
       break;
    case eB_DVR_OperationPlay:
       {
           NEXUS_PlaybackStatus playbackStatus;
           BDBG_MSG(("eB_DVR_OperationPlay"));
           NEXUS_Playback_GetStatus(tsbService->playback, &playbackStatus);
           if(tsbService->tsbPlaybackStarted && playbackStatus.state != NEXUS_PlaybackState_ePlaying)
           {
               NEXUS_Playback_Play(tsbService->playback);
           }
           else
           {
               if(!tsbService->tsbPlaybackStarted)
               {
                   BDBG_ERR(("live mode plaback invalid"));
               }
               if(playbackStatus.state == NEXUS_PlaybackState_ePlaying)
               {
                   BDBG_WRN(("already in TSB play mode"));
               }
          }
        }
        break;
    case eB_DVR_OperationPlayGop:
        {
            if(tsbService->tsbPlaybackStarted)
            { 
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                BDBG_MSG(("eB_DVR_OperationPlayGop"));
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayGop;
                trickModeSettings.mode_modifier = operationSettings->mode_modifier;
                trickModeSettings.rate = operationSettings->operationSpeed;
                NEXUS_Playback_TrickMode(tsbService->playback,&trickModeSettings);
            }
            else
            {
                BDBG_ERR(("live mode plaback invalid"));
            }
        }
        break;
    case eB_DVR_OperationPlayTimeSkip:
       {
           if(tsbService->tsbPlaybackStarted)
           { 
               NEXUS_PlaybackTrickModeSettings trickModeSettings;
               BDBG_MSG(("eB_DVR_OperationPlayTimeSkip"));
               NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
               trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_eTimeSkip;
               trickModeSettings.mode_modifier = operationSettings->mode_modifier;
               trickModeSettings.rate = operationSettings->operationSpeed;
               NEXUS_Playback_TrickMode(tsbService->playback,&trickModeSettings);
           }
           else
           {
               BDBG_ERR(("live mode plaback invalid"));
           }
       }
       break;

    case eB_DVR_OperationFastForward:
        {
            BDBG_MSG(("eB_DVR_OperationFastForward"));
            if(tsbService->tsbPlaybackStarted)
            {
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;;
                trickModeSettings.mode_modifier = operationSettings->operationSpeed;
                trickModeSettings.rate = NEXUS_NORMAL_DECODE_RATE;
                NEXUS_Playback_TrickMode(tsbService->playback,&trickModeSettings);
           }
           else
           {
               BDBG_WRN(("live mode: eB_DVR_OperationFastForward invalid"));
           }
       }
       break;
    case eB_DVR_OperationFastRewind:
        {
            BDBG_MSG(("eB_DVR_OperationFastRewind"));
            if(tsbService->tsbPlaybackStarted)
            {
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
                trickModeSettings.mode_modifier = -operationSettings->operationSpeed;
                trickModeSettings.rate = NEXUS_NORMAL_DECODE_RATE;
                NEXUS_Playback_TrickMode(tsbService->playback,&trickModeSettings);
            }
            else
            {
              BDBG_WRN(("live mode: eB_DVR_OperationFastRewind invalid"));
            }
        }
        break;
    case eB_DVR_OperationSlowForward:
        {
            BDBG_MSG(("eB_DVR_OperationSlowRewind"));
            if(tsbService->tsbPlaybackStarted)
            {
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.mode_modifier = 1;
                trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED/operationSettings->operationSpeed;
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_eNormal;
                NEXUS_Playback_TrickMode(tsbService->playback, &trickModeSettings);
           }
           else
           {
               BDBG_WRN(("live mode: eB_DVR_OperationSlowRewind invalid"));
           }
        }
        break;
    case eB_DVR_OperationSlowRewind:
        {
            BDBG_MSG(("eB_DVR_OperationSlowRewind"));
            if(tsbService->tsbPlaybackStarted)
            {
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.mode_modifier = -1;
                trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED/operationSettings->operationSpeed;
                if(B_DVR_TSBService_P_GetVideoCodec(tsbService) == NEXUS_VideoCodec_eMpeg2)
                {
                    trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
                }
                else
                {
                    trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
                }
                
                NEXUS_Playback_TrickMode(tsbService->playback, &trickModeSettings);
           }
           else
           {
               BDBG_WRN(("live mode: eB_DVR_OperationSlowRewind invalid"));
           }
        }
        break;
    case eB_DVR_OperationSpeed:
        {
            if(tsbService->tsbPlaybackStarted)
            {
                NEXUS_PlaybackStatus playbackStatus;
                NEXUS_PlaybackTrickModeSettings trickModeSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
                trickModeSettings.maxDecoderRate = 4 * NEXUS_NORMAL_PLAY_SPEED;
                trickModeSettings.rate = operationSettings->operationSpeed;
                BDBG_MSG(("eB_DVR_OperationSpeed: maxDecoderRate %d, rate %d\n",
                    trickModeSettings.maxDecoderRate, trickModeSettings.rate));
                NEXUS_Playback_GetStatus(tsbService->playback, &playbackStatus);
                if(playbackStatus.state == NEXUS_PlaybackState_ePaused)
                {
                    BDBG_WRN(("eB_DVR_OperationSpeed in paused state"));
                    NEXUS_Playback_Play(tsbService->playback);
                }
                NEXUS_Playback_TrickMode(tsbService->playback, &trickModeSettings);
            }
            else
            {
                BDBG_WRN(("live mode: eB_DVR_OperationSpeed invalid"));
            }
        }
        break;
    case eB_DVR_OperationReverseFrameAdvance:
        {
            NEXUS_PlaybackStatus playbackStatus;
            BDBG_MSG(("eB_DVR_OperationReverseFrameAdvance"));
            NEXUS_Playback_GetStatus(tsbService->playback, &playbackStatus);
            if(tsbService->tsbPlaybackStarted && playbackStatus.state == NEXUS_PlaybackState_ePaused)
            {
                NEXUS_Playback_FrameAdvance(tsbService->playback,false);
            }
            else
            {
                BDBG_WRN(("live mode: eB_DVR_OperationReverseFrameAdvance invalid"));
            }
        }
        break;
    case eB_DVR_OperationForwardFrameAdvance:
        {
            NEXUS_PlaybackStatus playbackStatus;
            BDBG_MSG(("eB_DVR_OperationForwardFrameAdvance"));
            NEXUS_Playback_GetStatus(tsbService->playback, &playbackStatus);
            if(tsbService->tsbPlaybackStarted && playbackStatus.state == NEXUS_PlaybackState_ePaused)
            {
                NEXUS_Playback_FrameAdvance(tsbService->playback,true);
            }
            else
            {
                BDBG_WRN(("live mode: eB_DVR_OperationForwardFrameAdvance invalid"));
            }
        }
        break;
    case eB_DVR_OperationSeek:
        {
            B_DVR_FilePosition startPosition,endPosition;
            BDBG_MSG(("eB_DVR_OperationSeek: seekTime %lu",operationSettings->seekTime));
            B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord, &startPosition, &endPosition);
            if(operationSettings->seekTime < startPosition.timestamp && 
               operationSettings->seekTime > endPosition.timestamp)
            {
                BDBG_ERR(("Invalid time as it's out of bounds"));
            }
            else
            {
               if(tsbService->tsbPlayAlarmTimer)
               { 
                   B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
                   B_Scheduler_CancelTimer(dvrManager->scheduler,tsbService->tsbPlayAlarmTimer);
                   tsbService->tsbPlayAlarmTimer = NULL;
                }
                NEXUS_Playback_Seek(tsbService->playback,operationSettings->seekTime);

            }
        }
        break;
    default:
        BDBG_WRN(("invalid operation %d",operationSettings->operation));
        break;
    }
    
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_SetOperation <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_SetSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned esStreamIndex=0;
    NEXUS_PidChannelRemapSettings remapSettings;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->parserBand <= NEXUS_NUM_PARSER_BANDS);
    BDBG_MSG(("B_DVR_TSBService_SetSettings >>>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    if(!BKNI_Memcmp(pSettings,&tsbService->tsbServiceSettings,sizeof(*pSettings)))
    {
        BDBG_WRN(("%s: pSettings match the internal tsbServiceSettings",__FUNCTION__));
        B_Mutex_Unlock(tsbService->tsbMutex); 
        return rc;
    }

    tsbService->tsbServiceSettings.parserBand = pSettings->parserBand;
    tsbService->tsbServiceSettings.dataInjectionService = pSettings->dataInjectionService;
    tsbService->tsbServiceSettings.playpumpIp = pSettings->playpumpIp;
    if(BKNI_Memcmp(&pSettings->tsbRecordSettings,
                   &tsbService->tsbServiceSettings.tsbRecordSettings,
                   sizeof(pSettings->tsbRecordSettings))) 
    {
        if(!tsbService->recpumpStarted)
        {
            if(pSettings->tsbRecordSettings.esStreamCount)
            {
                BDBG_ASSERT(pSettings->tsbRecordSettings.esStreamCount == tsbService->tsbMediaNode->esStreamCount);
                for(esStreamIndex=0;esStreamIndex<tsbService->tsbMediaNode->esStreamCount;esStreamIndex++ )
                {
                    remapSettings.enabled = true;
                    remapSettings.continuityCountEnabled = true;
                    remapSettings.pid = pSettings->tsbRecordSettings.RemappedEsStreamInfo[esStreamIndex].pid;
                    NEXUS_PidChannel_SetRemapSettings(tsbService->recordPidChannels[esStreamIndex],&remapSettings);
                    tsbService->tsbMediaNode->esStreamInfo[esStreamIndex].pid = pSettings->tsbRecordSettings.RemappedEsStreamInfo[esStreamIndex].pid;
                }
            }
        }
        else
        {
            BDBG_WRN(("%s PID remapping parserBand and dataInjectionService change "
                      "is not allowed after recpumpStart",__FUNCTION__));
        }
    }

    if(BKNI_Memcmp(&pSettings->tsbPlaybackSettings,
                   &tsbService->tsbServiceSettings.tsbPlaybackSettings,
                   sizeof(pSettings->tsbPlaybackSettings))) 
    {
        if(!tsbService->tsbPlaybackStarted) 
        {
            tsbService->tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0] = pSettings->tsbPlaybackSettings.audioDecoder[0];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1] = pSettings->tsbPlaybackSettings.audioDecoder[1];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0] = pSettings->tsbPlaybackSettings.videoDecoder[0];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1] = pSettings->tsbPlaybackSettings.videoDecoder[1];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.stcChannel = pSettings->tsbPlaybackSettings.stcChannel;
            tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleAudioDecoder[0] = pSettings->tsbPlaybackSettings.simpleAudioDecoder[0];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleAudioDecoder[1] = pSettings->tsbPlaybackSettings.simpleAudioDecoder[1];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleVideoDecoder[0] = pSettings->tsbPlaybackSettings.simpleVideoDecoder[0];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleVideoDecoder[1] = pSettings->tsbPlaybackSettings.simpleVideoDecoder[1];
            tsbService->tsbServiceSettings.tsbPlaybackSettings.simpleStcChannel = pSettings->tsbPlaybackSettings.simpleStcChannel;

        }
        else
        {
            BDBG_WRN(("%s A/V decode and STC handle change "
                      "is not allowed after recpumpStart",__FUNCTION__));
        }
    }

    if(tsbService->tsbConversion && 
       tsbService->permanentRecordingMediaNode &&
       segmentedFileRecord->permRec.recEndTime != pSettings->tsbConvEndTime)
    {
        if(pSettings->tsbConvEndTime < tsbService->permanentRecordingMediaNode->mediaStartTime) 
        {
            BDBG_WRN(("%s: The desired TsbConv endTime (%u) is less than tsbConv startTime (%u)",
                      __FUNCTION__,
                      pSettings->tsbConvEndTime,
                      tsbService->permanentRecordingMediaNode->mediaStartTime));
            rc = B_DVR_NOT_SUPPORTED;
        }
        else
        { 
            B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
            BDBG_MSG(("%s: tsbConv endTime changed from %u to %u",
                      __FUNCTION__,
                      segmentedFileRecord->permRec.recEndTime,
                      pSettings->tsbConvEndTime));
            segmentedFileRecord->permRec.recEndTime = pSettings->tsbConvEndTime;
            /*
             * if the changed converstion end time is less or equal to the current TSB time
             * and conversion is still on, then trigger the media node update to trigger
             * conversion complete callback to the app
             *
             */
            if((pSettings->tsbConvEndTime <= tsbService->tsbMediaNode->mediaEndTime)
                && tsbService->tsbConversion)
            {
                B_Event_Set(tsbService->tsbRecordEvent);
            }
        }
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_SetSettings <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_GetSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceSettings *pSettings)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_TSBService_GetSettings >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&tsbService->tsbServiceSettings,sizeof(B_DVR_TSBServiceSettings));
    if(tsbService->tsbConversion) 
    {
        B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
        pSettings->tsbConvEndTime = segmentedFileRecord->permRec.recEndTime;
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_GetSettings <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_GetStatus(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceStatus *tsbServiceStatus)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    #if 0
    B_DVR_FilePosition currentPosition;
    #endif
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(tsbServiceStatus);
    BDBG_MSG(("B_DVR_TSBService_GetStatus >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    tsbServiceStatus->tsbPlayback = tsbService->tsbPlaybackStarted;
    tsbServiceStatus->tsbCoversion = tsbService->tsbConversion;
    if(tsbService->tsbPlaybackStarted)
    {
        NEXUS_Playback_GetStatus(tsbService->playback,&nexusPlaybackStatus);
        if(nexusPlaybackStatus.position < tsbService->tsbMediaNode->mediaStartTime) 
        {
			BDBG_MSG(("B_DVR_TSBService_GetStatus: flooring position %lu to %lu\n", 
                      nexusPlaybackStatus.position, tsbService->tsbMediaNode->mediaStartTime));
            nexusPlaybackStatus.position = tsbService->tsbMediaNode->mediaStartTime;
        }
        if(nexusPlaybackStatus.position > tsbService->tsbMediaNode->mediaEndTime) 
        {
			BDBG_MSG(("B_DVR_TSBService_GetStatus: ceiling position %lu to %lu\n", 
                      nexusPlaybackStatus.position, tsbService->tsbMediaNode->mediaEndTime));
            nexusPlaybackStatus.position = tsbService->tsbMediaNode->mediaEndTime;
        }

        tsbServiceStatus->tsbCurrentPlayTime = nexusPlaybackStatus.position;
        /* 
         * TODO: GetLocation is performance demanding. No one use tsbCurrentPlayOffset. 
         * Investigate later. 
         */
#if 0 
        rc = B_DVR_SegmentedFileRecord_GetLocation(tsbService->nexusFileRecord,
                                                   -1,
                                                   nexusPlaybackStatus.position,
                                                   &currentPosition);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("%s: B_DVR_SegmentedFileRecord_GetLocation failed"));
            rc = B_DVR_UNKNOWN;
            goto error;
        }
        tsbServiceStatus->tsbCurrentPlayOffset = currentPosition.mpegFileOffset;
#else
        tsbServiceStatus->tsbCurrentPlayOffset = (off_t)-1; // no one use
#endif
        tsbServiceStatus->state = nexusPlaybackStatus.state;
    }
    else
    {
        tsbServiceStatus->state = NEXUS_PlaybackState_eMax;
        tsbServiceStatus->tsbCurrentPlayOffset = 0;
        tsbServiceStatus->tsbCurrentPlayTime = 0;
    }
    tsbServiceStatus->tsbRecLinearStartOffset = tsbService->tsbMediaNode->mediaLinearStartOffset;
    tsbServiceStatus->tsbRecStartTime = tsbService->tsbMediaNode->mediaStartTime;
    tsbServiceStatus->tsbRecLinearEndOffset = tsbService->tsbMediaNode->mediaLinearEndOffset;
    tsbServiceStatus->tsbRecEndTime= tsbService->tsbMediaNode->mediaEndTime;
    if(tsbService->tsbConversion)
    {
        tsbServiceStatus->permRecLinearCurrentOffset = tsbService->permanentRecordingMediaNode->mediaLinearEndOffset;
        tsbServiceStatus->permRecCurrentTime = tsbService->permanentRecordingMediaNode->mediaEndTime;
    }
    else
    {
        tsbServiceStatus->permRecLinearCurrentOffset = 0;
        tsbServiceStatus->permRecCurrentTime = 0;
    }
#if 0
error:
#endif
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_GetStatus: tsbCurrentPlayTime   %lu\n", tsbServiceStatus->tsbCurrentPlayTime)); 
    BDBG_MSG(("B_DVR_TSBService_GetStatus: tsbCurrentPlayOffset %jd\n", tsbServiceStatus->tsbCurrentPlayOffset)); 
    BDBG_MSG(("B_DVR_TSBService_GetStatus: tsbRecLinearStart/EndOffset %jd ~ %jd\n", 
        tsbServiceStatus->tsbRecLinearStartOffset, tsbServiceStatus->tsbRecLinearEndOffset)); 
    BDBG_MSG(("B_DVR_TSBService_GetStatus: tsbRecStart/EndTime %lu ~ %lu\n", 
        tsbServiceStatus->tsbRecStartTime, tsbServiceStatus->tsbRecEndTime)); 
    BDBG_MSG(("B_DVR_TSBService_GetStatus <<<"));
    return rc;
}


B_DVR_ERROR B_DVR_TSBService_InstallCallback(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT((registeredCallback));
    BDBG_MSG(("B_DVR_TSBService_InstallCallback >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    tsbService->registeredCallback = registeredCallback;
    tsbService->appContext = appContext;
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_InstallCallback <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_RemoveCallback(B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_RemoveCallback >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    tsbService->registeredCallback=NULL;
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_RemoveCallback <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_InjectDataStart(
    B_DVR_TSBServiceHandle tsbService,
    uint8_t *buf,
    unsigned length)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings injectSettings;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(tsbService->tsbServiceSettings.dataInjectionService);
    B_Mutex_Lock(tsbService->tsbMutex);
    if(!tsbService->dataInjectionStarted) 
    {
        B_DVR_DataInjectionService_GetSettings(tsbService->tsbServiceSettings.dataInjectionService,&injectSettings);
        BDBG_ASSERT(injectSettings.pidChannel);
        B_DVR_DataInjectionService_Start(tsbService->tsbServiceSettings.dataInjectionService,
                                         injectSettings.pidChannel,
                                         buf,
                                         length);
        NEXUS_Record_AddPidChannel(tsbService->record,
                                   injectSettings.pidChannel,
                                   NULL);
        tsbService->dataInjectionStarted = true;
    }
    else
    {
        BDBG_WRN(("Data injection already started for tsbService %u",tsbService->index));
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_InjectDataStop(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_DataInjectionServiceSettings injectSettings;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(tsbService->tsbServiceSettings.dataInjectionService);
    B_Mutex_Lock(tsbService->tsbMutex);
    if(tsbService->dataInjectionStarted)
    {
        B_DVR_DataInjectionService_GetSettings(tsbService->tsbServiceSettings.dataInjectionService,&injectSettings);
        B_DVR_DataInjectionService_Stop(tsbService->tsbServiceSettings.dataInjectionService);
        NEXUS_Record_RemovePidChannel(tsbService->record,
                                      injectSettings.pidChannel);
        tsbService->dataInjectionStarted = false;
    }
    else
    {
        BDBG_WRN(("Data injection already stopped for tsbService %u",tsbService->index));
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    return rc;
}

NEXUS_PidChannelHandle B_DVR_TSBService_GetDataInjectionPidChannel(
    B_DVR_TSBServiceHandle tsbService)
{
    NEXUS_PidChannelHandle dataInjectionPidChannel=NULL;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_GetDataInjectionPidChannel >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    if(tsbService->tsbServiceSettings.dataInjectionService)
    {
        B_DVR_DataInjectionServiceSettings injectSettings;
        B_DVR_DataInjectionService_GetSettings(tsbService->tsbServiceSettings.dataInjectionService,
                                               &injectSettings);
        dataInjectionPidChannel = injectSettings.pidChannel;
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_GetDataInjectionPidChannel <<<"));
    return dataInjectionPidChannel;
}

 
B_DVR_ERROR B_DVR_TSBService_ConvertStart(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServicePermanentRecordingRequest *permRecReq)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    unsigned long returnedTimeStamp;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    char metaDataPath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_FilePosition tsbRecStart,tsbRecEnd;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();

    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(permRecReq);
    B_Mutex_Lock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_ConvertStart >>>>"));
    if(tsbService->tsbConversion)
    {
        BDBG_ERR((" TSB conversion already in progress"));
        goto error;
    }
    rc = B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbRecStart,&tsbRecEnd); 
    if(rc != B_DVR_SUCCESS) 
    {
        BDBG_ERR(("B_DVR_SegmentedFileRecord_GetBounds failed"));
        goto error;
    }

    if(permRecReq->recStartTime > tsbRecEnd.timestamp)
    {
       BDBG_ERR(("TSB Convert start time error"));
       BDBG_ERR(("tsbGetBounds start time %lums end time %lums",tsbRecStart.timestamp,tsbRecEnd.timestamp));
       BDBG_ERR(("tsb mediaNode start time %lums end time %lums",
                 tsbService->tsbMediaNode->mediaStartTime,
                 tsbService->tsbMediaNode->mediaEndTime));
       BDBG_ERR(("permRec start time %lums end time %lums",permRecReq->recStartTime,permRecReq->recEndTime));
       
       rc = B_DVR_INVALID_PARAMETER;
       goto error;
    }
    
    B_DVR_MediaStorage_GetMetadataPath(dvrManager->mediaStorage,tsbService->tsbServiceRequest.volumeIndex,metaDataPath);
    tsbService->permanentRecordingMediaNode = BKNI_Malloc(sizeof(B_DVR_Media));
    if(!tsbService->permanentRecordingMediaNode) 
    {
        BDBG_ERR(("permanentRecordingMediaNode alloc failed tsb index:%u  permRec:%s",
                  tsbService->index,permRecReq->programName));
        goto error;
    }
    BKNI_Memset((void*)tsbService->permanentRecordingMediaNode,0,sizeof(B_DVR_Media));
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*tsbService->permanentRecordingMediaNode), 
                                            true,__FUNCTION__,__LINE__);
    BDBG_OBJECT_SET(tsbService->permanentRecordingMediaNode,B_DVR_Media);
    BKNI_Memcpy((void*)(tsbService->permanentRecordingMediaNode),(void *)tsbService->tsbMediaNode,sizeof(B_DVR_Media));
    BKNI_Memcpy(&tsbService->permRecReq,permRecReq,sizeof(*permRecReq));
    if(permRecReq->subDir[0]=='\0')
    { 
        BKNI_Snprintf(segmentedFileRecord->permRec.navFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s%s",
                      metaDataPath,permRecReq->programName,B_DVR_NAVIGATION_FILE_EXTENTION);
        BKNI_Snprintf(segmentedFileRecord->permRec.mediaFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s%s",
                      metaDataPath,permRecReq->programName,B_DVR_MEDIA_FILE_EXTENTION);
    }
    else
    {
        BKNI_Snprintf(segmentedFileRecord->permRec.navFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s/%s%s",
                      metaDataPath,permRecReq->subDir,permRecReq->programName,B_DVR_NAVIGATION_FILE_EXTENTION);
        BKNI_Snprintf(segmentedFileRecord->permRec.mediaFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s/%s%s",
                      metaDataPath,permRecReq->subDir, permRecReq->programName,B_DVR_MEDIA_FILE_EXTENTION);
    }

    BDBG_WRN((" ConvertStart %s %s",tsbService->permanentRecordingMediaNode->programName,tsbService->permanentRecordingMediaNode->mediaNodeFileName));

    BKNI_Snprintf(tsbService->permanentRecordingMediaNode->navFileName, 
                  sizeof(tsbService->permanentRecordingMediaNode->navFileName),"%s%s",
                  permRecReq->programName,B_DVR_NAVIGATION_FILE_EXTENTION);
    BKNI_Snprintf(tsbService->permanentRecordingMediaNode->mediaFileName,
                  sizeof(tsbService->permanentRecordingMediaNode->mediaFileName),"%s%s",
                  permRecReq->programName,B_DVR_MEDIA_FILE_EXTENTION);
    BKNI_Snprintf(tsbService->permanentRecordingMediaNode->mediaNodeFileName,
                  sizeof(tsbService->permanentRecordingMediaNode->mediaNodeFileName),"%s%s",
                  permRecReq->programName,B_DVR_MEDIA_NODE_FILE_EXTENTION);
    strncpy(tsbService->permanentRecordingMediaNode->programName,permRecReq->programName,B_DVR_MAX_FILE_NAME_LENGTH);
    tsbService->permanentRecordingMediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';
    tsbService->permanentRecordingMediaNode->recording = eB_DVR_RecordingPermanent;
    segmentedFileRecord->permRec.recStartTime = permRecReq->recStartTime;
    segmentedFileRecord->permRec.recCurrentTime = permRecReq->recStartTime;
    segmentedFileRecord->permRec.recEndTime = permRecReq->recEndTime;
    segmentedFileRecord->permRec.navFileCount =0;
    segmentedFileRecord->permRec.mediaFileCount =0;
    segmentedFileRecord->permRec.pCachedMediaFileNode = NULL;
    segmentedFileRecord->permRec.pCachedNavFileNode = NULL;
    segmentedFileRecord->permRec.tsbConvMetaDataSubDir = NULL;  
    if(permRecReq->subDir[0] != '\0')
    { 
        strncpy(tsbService->permanentRecordingMediaNode->mediaNodeSubDir,permRecReq->subDir,B_DVR_MAX_FILE_NAME_LENGTH);
        tsbService->permanentRecordingMediaNode->mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';
        segmentedFileRecord->permRec.tsbConvMetaDataSubDir = (char *)tsbService->permanentRecordingMediaNode->mediaNodeSubDir;
    }

    #ifdef MEDIANODE_ONDEMAND_CACHING
    rc = B_DVR_List_AddMediaNodeFile(dvrManager->dvrList,tsbService->tsbServiceRequest.volumeIndex,tsbService->permanentRecordingMediaNode);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("error in creating %s",tsbService->permanentRecordingMediaNode->mediaNodeFileName));
        goto error;
    }
    #endif
    rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,tsbService->tsbServiceRequest.volumeIndex,tsbService->permanentRecordingMediaNode);
    if(rc!=B_DVR_SUCCESS)
    {
        #ifdef MEDIANODE_ONDEMAND_CACHING
        B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,tsbService->tsbServiceRequest.volumeIndex,tsbService->permanentRecordingMediaNode);
        #endif
        BDBG_ERR(("error in adding mediaNode for %s",tsbService->permanentRecordingMediaNode->mediaNodeFileName));
        goto error;
    }
    rc = B_DVR_SegmentedFileRecord_OpenPermMetaDataFile(tsbService->nexusFileRecord);
    if(rc != B_DVR_SUCCESS)
    {
         BDBG_MSG(("%s: Error in B_DVR_SegmentedFile_OpenPermMetaDataFile",__FUNCTION__));
         #ifdef MEDIANODE_ONDEMAND_CACHING
         B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,tsbService->tsbServiceRequest.volumeIndex,tsbService->permanentRecordingMediaNode);
         #endif
         /* no need to check for error here as the conversion hasn't started*/
         B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                    tsbService->tsbServiceRequest.volumeIndex,
                                    tsbService->permanentRecordingMediaNode);
         rc = B_DVR_OS_ERROR;
         goto error;
    }
    tsbService->permanentRecordingMediaNode->mediaLinearStartOffset = segmentedFileRecord->permRec.recMediaStartOffset;
    tsbService->permanentRecordingMediaNode->mediaStartTime = segmentedFileRecord->permRec.recStartTime;
    tsbService->permanentRecordingMediaNode->navLinearStartOffset = segmentedFileRecord->permRec.recNavStartOffset;
    returnedTimeStamp = B_DVR_FileSegmentedRecord_CopyMetaDataFile(tsbService->nexusFileRecord,tsbRecStart,tsbRecEnd);
    if((long)returnedTimeStamp < 0) 
    {
        returnedTimeStamp = segmentedFileRecord->permRec.recStartTime;
    }
    tsbService->permanentRecordingMediaNode->mediaEndTime = segmentedFileRecord->permRec.recCurrentTime;
    tsbService->permanentRecordingMediaNode->mediaStartTime = segmentedFileRecord->permRec.recStartTime;
    tsbService->permanentRecordingMediaNode->mediaLinearStartOffset = segmentedFileRecord->permRec.recMediaStartOffset;
    tsbService->permanentRecordingMediaNode->mediaLinearEndOffset = segmentedFileRecord->permRec.recMediaEndOffset;
    tsbService->permanentRecordingMediaNode->navLinearStartOffset = segmentedFileRecord->permRec.recNavStartOffset;
    tsbService->permanentRecordingMediaNode->navLinearEndOffset = segmentedFileRecord->permRec.recNavEndOffset;
    if (returnedTimeStamp < tsbService->permRecReq.recEndTime)
    {
        tsbService->permanentRecordingMediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
        tsbService->tsbMediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
    }
    else
    {
        tsbService->permanentRecordingMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
        tsbService->tsbMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
    }
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               tsbService->permanentRecordingMediaNode,
                               tsbService->tsbServiceRequest.volumeIndex,true);
    if (returnedTimeStamp < tsbService->permRecReq.recEndTime)
    {
         BDBG_MSG((" End time beyong TSB Rec:returnedTimestamp %lu end time %lu",returnedTimeStamp,tsbService->permRecReq.recEndTime));
         tsbService->tsbConversion = true;
   }
    else
    {
        BDBG_MSG(("Conversion from TSB to Perm Rec succeeded\n"));
        B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(tsbService->nexusFileRecord);
        tsbService->tsbConversion = false;
        BDBG_MSG((" End of TSB Conversion"));
        if(tsbService->registeredCallback)
        {
            B_Mutex_Unlock(tsbService->tsbMutex);
            event = eB_DVR_EventTSBConverstionCompleted;
            service = eB_DVR_ServiceTSB;
            tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
            B_Mutex_Lock(tsbService->tsbMutex);
        }
    }
    B_DVR_TSBService_P_PrintRecInfo(tsbService,true);
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_ConvertStart <<<<<"));
    return rc;
error:
    if(tsbService->permanentRecordingMediaNode) 
    {
        BDBG_OBJECT_DESTROY(tsbService->permanentRecordingMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*tsbService->permanentRecordingMediaNode), 
                                                false,__FUNCTION__,__LINE__);
        BKNI_Free(tsbService->permanentRecordingMediaNode);
        tsbService->permanentRecordingMediaNode = NULL;
    }
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_ConvertStart <<<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_ConvertStop(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    B_Mutex_Lock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_ConvertStop >>>"));
    BDBG_MSG(("%s: index %d",__FUNCTION__,tsbService->index));
    if(!tsbService->permanentRecordingMediaNode) 
    {
        BDBG_ERR(("Converstion already stopped"));
        goto error;
    }
    mediaNodeSettings.programName = tsbService->permanentRecordingMediaNode->programName;
    mediaNodeSettings.subDir = tsbService->permanentRecordingMediaNode->mediaNodeSubDir;
    mediaNodeSettings.volumeIndex=tsbService->tsbServiceRequest.volumeIndex;
    B_DVR_Manager_PrintSegmentedFileRecord(dvrManager,&mediaNodeSettings);
    if (tsbService->tsbConversion)
    {
        B_DVR_FilePosition tsbRecStart,tsbRecEnd;
        tsbService->tsbConversion = false;
        B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbRecStart,&tsbRecEnd); 
        if(segmentedFileRecord->permRec.recEndTime > tsbRecEnd.timestamp)
        {
            segmentedFileRecord->permRec.recEndTime = tsbRecEnd.timestamp;
        }
        B_DVR_FileSegmentedRecord_CopyMetaDataFile(tsbService->nexusFileRecord,tsbRecStart,tsbRecEnd);
        tsbService->permanentRecordingMediaNode->mediaEndTime = segmentedFileRecord->permRec.recCurrentTime;
        tsbService->permanentRecordingMediaNode->mediaStartTime = segmentedFileRecord->permRec.recStartTime;
        tsbService->permanentRecordingMediaNode->mediaLinearStartOffset = segmentedFileRecord->permRec.recMediaStartOffset;
        tsbService->permanentRecordingMediaNode->mediaLinearEndOffset = segmentedFileRecord->permRec.recMediaEndOffset;
        tsbService->permanentRecordingMediaNode->navLinearStartOffset = segmentedFileRecord->permRec.recNavStartOffset;
        tsbService->permanentRecordingMediaNode->navLinearEndOffset =  segmentedFileRecord->permRec.recNavEndOffset;
        tsbService->permanentRecordingMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
        B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                   tsbService->permanentRecordingMediaNode,
                                   tsbService->tsbServiceRequest.volumeIndex,true);
        B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(tsbService->nexusFileRecord);
        if(tsbService->registeredCallback)
        { 
            B_Mutex_Unlock(tsbService->tsbMutex);
            event = eB_DVR_EventTSBConverstionCompleted;
            service = eB_DVR_ServiceTSB;
            tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
            B_Mutex_Lock(tsbService->tsbMutex);
        }
    }
    B_DVR_TSBService_P_PrintRecInfo(tsbService,true);
    BKNI_Memset(&segmentedFileRecord->permRec,0,sizeof(segmentedFileRecord->permRec));
    BKNI_Memset(&tsbService->permRecReq,0,sizeof(tsbService->permRecReq));
    
    if(tsbService->inProgressRecPlay)
    {
        NEXUS_Record_RemovePlayback(tsbService->record,tsbService->inProgressRecPlay);
        tsbService->inProgressRecPlay = NULL;
    }
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                   tsbService->tsbServiceRequest.volumeIndex,
                                   tsbService->permanentRecordingMediaNode))
    {
        BDBG_OBJECT_DESTROY(tsbService->permanentRecordingMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*tsbService->permanentRecordingMediaNode),
                                                false,__FUNCTION__,__LINE__);
        BKNI_Free(tsbService->permanentRecordingMediaNode);
    }
    #endif
    tsbService->updatePlaybackEvent = NULL;
    tsbService->permanentRecordingMediaNode=NULL;
error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_ConvertStop >>>>"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_SetAlarmTime(
    B_DVR_TSBServiceHandle tsbService,
    unsigned long tsbPlayAlarmTime)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_TSBService_SetAlarmTime >>> alarmTime %lu",tsbPlayAlarmTime));
    tsbService->tsbPlayAlarmTime = tsbPlayAlarmTime;
    if(tsbService->tsbPlayAlarmTimer)
    {
        B_Scheduler_CancelTimer(dvrManager->scheduler,tsbService->tsbPlayAlarmTimer);
        tsbService->tsbPlayAlarmTimer = NULL;
    }
    NEXUS_Playback_GetStatus(tsbService->playback,&nexusPlaybackStatus);
    if(nexusPlaybackStatus.state != NEXUS_PlaybackState_eStopped || 
       nexusPlaybackStatus.state != NEXUS_PlaybackState_eAborted)
    {
        
        tsbService->tsbPlayAlarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                               tsbService->schedulerMutex,
                                                               1000,B_DVR_TSBService_P_PlaybackAlarmTimer,
                                                               (void*)tsbService);
    }
    else
    {
        BDBG_WRN(("Playback not started. So alarm will be scheduled after playback start"));

    }
    BDBG_MSG(("B_DVR_TSBService_SetAlarmTime <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_AddInputEsStream(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceInputEsStream *inputEsStream)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaNode tsbMediaNode=NULL;
    NEXUS_RecordPidChannelSettings pidSettings;
    B_DVR_EsStreamInfo *esStreamInfo=NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    unsigned esStreamCount,activeEsStreamCount;
    bool setRapTpit = false;
    NEXUS_RecpumpTpitFilter filter;
    
    BDBG_MSG(("B_DVR_TSBService_AddEsStream >>> "));
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(inputEsStream);
    B_Mutex_Lock(tsbService->tsbMutex);
    esStreamInfo = &inputEsStream->esStreamInfo;
    tsbMediaNode = tsbService->tsbMediaNode;
    esStreamCount = tsbMediaNode->esStreamCount;
    activeEsStreamCount = tsbService->activeEsStreamCount;
    BKNI_Memcpy((void*)&tsbMediaNode->esStreamInfo[esStreamCount],(void *)&inputEsStream->esStreamInfo,sizeof(B_DVR_EsStreamInfo));
    BKNI_Memcpy((void*)&tsbService->activeEsStreamInfo[activeEsStreamCount],(void *)&inputEsStream->esStreamInfo,sizeof(B_DVR_EsStreamInfo));

    if(esStreamInfo->pidType == eB_DVR_PidTypeVideo)
    {
        NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
        pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
        pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
        pidSettings.recpumpSettings.pidTypeSettings.video.codec = esStreamInfo->codec.videoCodec;
        tsbService->mpeg2Video = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?true:false;
        setRapTpit = (esStreamInfo->codec.videoCodec == NEXUS_VideoCodec_eMpeg2)?false:true;
    }

    if(inputEsStream->pidChannel)
    {
        tsbService->recordPidChannels[activeEsStreamCount] = inputEsStream->pidChannel;
    }
    else
    {
        if((tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam) ||
            (tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputIp)) 
        {
            if(tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam) 

            {
                tsbService->recordPidChannels[activeEsStreamCount] = 
                    NEXUS_PidChannel_Open(tsbService->tsbServiceSettings.parserBand,
                                          esStreamInfo->pid,
                                          NULL);
            }
            else
            {
                tsbService->recordPidChannels[activeEsStreamCount] = 
                    NEXUS_Playpump_OpenPidChannel(tsbService->tsbServiceSettings.playpumpIp,
                                                 esStreamInfo->pid,
                                                 NULL);
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
        NEXUS_Record_AddPidChannel(tsbService->record,tsbService->recordPidChannels[activeEsStreamCount],NULL);
    }
    else
    {
        NEXUS_Record_AddPidChannel(tsbService->record,tsbService->recordPidChannels[activeEsStreamCount],&pidSettings);
    }
    BDBG_MSG(("pid %d pidT %d codec %d pidchannel %d",esStreamInfo->pid,esStreamInfo->pidType,
              esStreamInfo->codec, tsbService->recordPidChannels[activeEsStreamCount]));

    if(!tsbService->recordPidChannels[activeEsStreamCount])
    {
        BDBG_ERR(("PID channel open failed"));
        rc = B_DVR_UNKNOWN;
        goto error;
    }

    if (setRapTpit) 
    {
        NEXUS_Recpump_GetDefaultTpitFilter(&filter);
        filter.config.mpeg.randomAccessIndicatorEnable = true;
        filter.config.mpeg.randomAccessIndicatorCompValue = true;
        NEXUS_Recpump_SetTpitFilter(tsbService->recpump, tsbService->recordPidChannels[activeEsStreamCount], &filter);
    }

    tsbMediaNode->esStreamCount++;
    tsbService->activeEsStreamCount++;
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               tsbService->tsbMediaNode,
                               tsbService->tsbServiceRequest.volumeIndex,false);

error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AddEsStream <<< "));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_RemoveInputEsStream(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServiceInputEsStream *inputEsStream)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned esStreamIndex=0;
    bool pidFound = false;
    B_DVR_EsStreamInfo *esStreamInfo=NULL;
    BDBG_MSG(("B_DVR_TSBService_RemoveEsStream >>> "));
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(inputEsStream);
    B_Mutex_Lock(tsbService->tsbMutex);
    esStreamInfo = &inputEsStream->esStreamInfo;
    for(esStreamIndex=0;esStreamIndex < tsbService->activeEsStreamCount;esStreamIndex++)
    {
        if(tsbService->activeEsStreamInfo[esStreamIndex].pid == esStreamInfo->pid
           && tsbService->activeEsStreamInfo[esStreamIndex].pidType == esStreamInfo->pidType)
        {
            pidFound = true;
            break;
        }
    }
    if(pidFound)
    { 
        B_DVR_EsStreamInfo esStreamInfo;
        NEXUS_Record_RemovePidChannel(tsbService->record,tsbService->recordPidChannels[esStreamIndex]);
        if(!inputEsStream->pidChannel)
        {
            if((tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam) ||
              (tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputIp)) 
            {
                if(tsbService->tsbServiceRequest.input == eB_DVR_TSBServiceInputQam)
                {
                    NEXUS_PidChannel_Close(tsbService->recordPidChannels[esStreamIndex]);
                }
                else
                {
                    NEXUS_Playpump_ClosePidChannel(tsbService->tsbServiceSettings.playpumpIp,
                                                   tsbService->recordPidChannels[esStreamIndex]);
                }
            }
        }
        BKNI_Memcpy((void *)&esStreamInfo,(void *)&tsbService->tsbMediaNode->esStreamInfo[esStreamIndex],sizeof(esStreamInfo));
        for(;esStreamIndex < tsbService->activeEsStreamCount;esStreamIndex++)
        {
            BKNI_Memcpy((void *)&tsbService->activeEsStreamInfo[esStreamIndex],
                        (void *)&tsbService->activeEsStreamInfo[esStreamIndex+1],
                        sizeof(B_DVR_EsStreamInfo));
            BKNI_Memcpy((void *)&tsbService->tsbMediaNode->esStreamInfo[esStreamIndex],
                        (void *)&tsbService->tsbMediaNode->esStreamInfo[esStreamIndex+1],
                        sizeof(B_DVR_EsStreamInfo));
            BKNI_Memcpy((void *)&tsbService->recordPidChannels[esStreamIndex],
                        (void *)&tsbService->recordPidChannels[esStreamIndex+1],
                        sizeof(NEXUS_PidChannelHandle));
        }

        BKNI_Memcpy((void *)&tsbService->tsbMediaNode->esStreamInfo[tsbService->tsbMediaNode->esStreamCount-1],
                    (void *)&esStreamInfo,sizeof(esStreamInfo));
        tsbService->activeEsStreamCount--;
    }
    BDBG_MSG(("B_DVR_TSBService_RemoveEsStream <<< "));
    B_Mutex_Unlock(tsbService->tsbMutex);
    return rc;
}

NEXUS_PidChannelHandle B_DVR_TSBService_GetPidChannel(
    B_DVR_TSBServiceHandle tsbService,
    unsigned pid)
{
    unsigned esStreamIndex;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    B_Mutex_Lock(tsbService->tsbMutex);
    for(esStreamIndex=0;esStreamIndex<tsbService->activeEsStreamCount;esStreamIndex++)
    {
        if(tsbService->activeEsStreamInfo[esStreamIndex].pid == pid)
        {
            break;
        }
    }
    if(esStreamIndex >= tsbService->activeEsStreamCount)
    {
        BDBG_ERR(("No pid channel opened for PID %u",pid));
        goto error;
    }

    B_Mutex_Unlock(tsbService->tsbMutex);
    return tsbService->recordPidChannels[esStreamIndex];
error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    return NULL;
}

B_DVR_ERROR B_DVR_TSBService_AddDrmSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_DRMServiceHandle drmService,
    B_DVR_DRMUsage drmUsage)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_ASSERT(drmService);
    BDBG_MSG(("B_DVR_TSBService_AddDrmSettings usage:%d>>>",drmUsage));

    B_Mutex_Lock(tsbService->tsbMutex);
    switch(drmUsage)
    {
    case eB_DVR_DRMUsageRecord:
        {
            NEXUS_RecordSettings recordSettings;
            tsbService->recordDrm = drmService;
            BDBG_ASSERT(tsbService->recpump);
            NEXUS_Record_GetSettings(tsbService->record, &recordSettings);
            recordSettings.recpumpSettings.securityDma = drmService->dma ;
            recordSettings.recpumpSettings.securityDmaDataFormat= NEXUS_DmaDataFormat_eMpeg;
            recordSettings.recpumpSettings.securityContext = drmService->keySlot ; 
            NEXUS_Record_SetSettings(tsbService->record, &recordSettings);
        }
        break;
    case eB_DVR_DRMUsagePlayback:
        {
            NEXUS_PlaybackSettings playbackSettings;
            tsbService->playbackDrm = drmService;
            BDBG_ASSERT(tsbService->playback);
            NEXUS_Playback_GetSettings(tsbService->playback, &playbackSettings);
            playbackSettings.playpumpSettings.securityDma = drmService->dma ;
            playbackSettings.playpumpSettings.securityDmaDataFormat= NEXUS_DmaDataFormat_eMpeg;
            playbackSettings.playpumpSettings.securityContext = drmService->keySlot ; 
            NEXUS_Playback_SetSettings(tsbService->playback, &playbackSettings);
        }
        break;
    case eB_DVR_DRMUsageMediaProbe:
        {
            tsbService->mediaProbeDrm = drmService;
        }
        break;
    default:
        BDBG_ERR(("Invalid drmUsage %d",drmUsage));
        rc = B_DVR_INVALID_PARAMETER;
    }

    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AddDrmSettings <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_RemoveDrmSettings(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_DRMUsage drmUsage)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    
    BDBG_MSG(("B_DVR_TSBService_RemoveDrmSettings usage %d >>>",drmUsage));

    B_Mutex_Lock(tsbService->tsbMutex);

    switch(drmUsage)
    {
    case eB_DVR_DRMUsageRecord:
        {
            tsbService->recordDrm = NULL;
        }
        break;
    case eB_DVR_DRMUsagePlayback:
        {
            tsbService->playbackDrm = NULL;
        }
        break;
    case eB_DVR_DRMUsageMediaProbe:
        {
            tsbService->mediaProbeDrm = NULL;
        }
        break;
    default:
        BDBG_ERR(("Invalid drmUsage %d",drmUsage));
        rc = B_DVR_INVALID_PARAMETER;
    }

    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_RemoveDrmSettings <<<"));
    
    return rc;
}

NEXUS_FileRecordHandle B_DVR_TSBService_GetFileRecordHandle(
    B_DVR_TSBServiceHandle tsbService)
{

    NEXUS_FileRecordHandle fileRecordHandle=NULL;
    BDBG_MSG(("B_DVR_TSBService_GetFileRecordHandle >>>"));
    if(tsbService) 
    {
        BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
        B_Mutex_Lock(tsbService->tsbMutex);
        fileRecordHandle = tsbService->nexusFileRecord;
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    BDBG_MSG(("B_DVR_TSBService_GetFileRecordHandle <<<"));
    return fileRecordHandle;
}

B_DVR_MediaNode B_DVR_TSBService_GetMediaNode(
    B_DVR_TSBServiceHandle tsbService, bool permRec)
{
    B_DVR_MediaNode mediaNode=NULL;
    BDBG_MSG(("B_DVR_TSBService_GetMediaNode >>>"));
    if(tsbService) 
    {
        BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
        B_Mutex_Lock(tsbService->tsbMutex);
        mediaNode= permRec?tsbService->permanentRecordingMediaNode:tsbService->tsbMediaNode;
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    BDBG_MSG(("B_DVR_TSBService_GetMediaNode <<<<"));
    return mediaNode;
}

B_DVR_ERROR B_DVR_TSBService_AppendedConvertStart(
    B_DVR_TSBServiceHandle tsbService,
    B_DVR_TSBServicePermanentRecordingRequest *permRecReq)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Event event;
    B_DVR_Service service;
    unsigned long returnedTimeStamp;
    void *pThreadRc;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    char metaDataPath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_FilePosition tsbRecStart,tsbRecEnd;
    off_t trunc;
    char tmp[] = "tmp";
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();

    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_ASSERT(permRecReq);
    B_Mutex_Lock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AppendedConvertStart >>>>"));
    if(tsbService->tsbConversion)
    {
        BDBG_ERR((" TSB conversion already in progress"));
        rc = B_DVR_NOT_SUPPORTED;
        goto error_os;
    }
    BDBG_MSG(("index %d name %s startTime %ld endTime %ld",
              tsbService->index,permRecReq->programName,permRecReq->recStartTime,
              permRecReq->recEndTime));

    rc = B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbRecStart,&tsbRecEnd); 
    if(rc != B_DVR_SUCCESS)
    {
        BDBG_ERR(("B_DVR_SegmentedFileRecord_GetBounds failed"));
        goto error_os;
    }
    BDBG_MSG(("time-> tsbStart: %lu tsbEnd: %lu",tsbRecStart.timestamp,tsbRecEnd.timestamp));
    BDBG_MSG(("media offset-> tsbStart: %lld tsbEnd: %lld",tsbRecStart.mpegFileOffset,tsbRecEnd.mpegFileOffset));
    BDBG_MSG(("nav offset-> tsbStart: %lld tsbEnd: %lld",tsbRecStart.navFileOffset,tsbRecEnd.navFileOffset));
    if(permRecReq->recStartTime > tsbRecEnd.timestamp)
    {
       BDBG_ERR(("TSB Convert start time > end time"));
       rc = B_DVR_INVALID_PARAMETER;
       goto error_os;
    }

    tsbService->eventGroup = B_EventGroup_Create(NULL);
    if(!tsbService->eventGroup)
    {
        BDBG_ERR(("tsbService->eventGroup eventGroup create failed"));
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    tsbService->appendedConversionUpdate = B_Event_Create(NULL);
    if(!tsbService->appendedConversionUpdate)
    {
        BDBG_ERR(("tsbService->appendedConversionThreadStop event create failed"));
        B_EventGroup_Destroy(tsbService->eventGroup);
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    tsbService->appendedConversionThreadStop = B_Event_Create(NULL);
    if(!tsbService->appendedConversionThreadStop)
    {
        BDBG_ERR(("tsbService->appendedConversionThreadStop event create failed"));
        B_Event_Destroy(tsbService->appendedConversionUpdate);
        B_EventGroup_Destroy(tsbService->eventGroup);
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    rc = B_EventGroup_AddEvent(tsbService->eventGroup,tsbService->appendedConversionUpdate);
    if(rc)
    {
        BDBG_ERR(("adding tsbService->appendedConversionUpdate to tsbService->eventGroup failed"));
        B_Event_Destroy(tsbService->appendedConversionThreadStop);
        B_Event_Destroy(tsbService->appendedConversionUpdate);
        B_EventGroup_Destroy(tsbService->eventGroup);
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    B_EventGroup_AddEvent(tsbService->eventGroup,tsbService->appendedConversionThreadStop);
    if(rc)
    {
        BDBG_ERR(("adding tsbService->appendedConversionThreadStop to tsbService->eventGroup failed"));
        B_EventGroup_RemoveEvent(tsbService->eventGroup,tsbService->appendedConversionUpdate);
        B_Event_Destroy(tsbService->appendedConversionThreadStop);
        B_Event_Destroy(tsbService->appendedConversionUpdate);
        B_EventGroup_Destroy(tsbService->eventGroup);
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    pthread_create (&tsbService->appendedConversionThread, NULL,B_DVR_TSBService_P_UpdateAppendedTsbConversion,tsbService);
    if(!tsbService->appendedConversionThread)
    {
        BDBG_ERR(("tsbService->appendedConversionThread thread create failed"));
        B_EventGroup_RemoveEvent(tsbService->eventGroup,tsbService->appendedConversionThreadStop);
        B_EventGroup_RemoveEvent(tsbService->eventGroup,tsbService->appendedConversionUpdate);
        B_Event_Destroy(tsbService->appendedConversionThreadStop);
        B_Event_Destroy(tsbService->appendedConversionUpdate);
        B_EventGroup_Destroy(tsbService->eventGroup);
        rc = B_DVR_OS_ERROR;
        goto error_os;
    }
    mediaNodeSettings.programName = permRecReq->programName;
    mediaNodeSettings.subDir = permRecReq->subDir;
    mediaNodeSettings.volumeIndex = tsbService->tsbServiceRequest.volumeIndex;
    tsbService->permanentRecordingMediaNode = B_DVR_List_GetMediaNode(dvrManager->dvrList,&mediaNodeSettings,true);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!tsbService->permanentRecordingMediaNode)
    {
        tsbService->permanentRecordingMediaNode= BKNI_Malloc(sizeof(*tsbService->permanentRecordingMediaNode));
        if(!tsbService->permanentRecordingMediaNode) 
        {
             BDBG_ERR(("error in allocating tsbService->permanentRecordingMediaNode for %s",
                       permRecReq->programName));
             goto error_permMediaNode;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*tsbService->permanentRecordingMediaNode),
                                                true, __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(dvrManager->dvrList,
                                         &mediaNodeSettings,
                                         tsbService->permanentRecordingMediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("error in getting mediaNode for %s",
                       permRecReq->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*tsbService->permanentRecordingMediaNode),
                                                    false, __FUNCTION__,__LINE__);
            BKNI_Free(tsbService->permanentRecordingMediaNode);
            tsbService->permanentRecordingMediaNode = NULL;
            goto error_permMediaNode;
        }
        rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,
                                     mediaNodeSettings.volumeIndex,
                                     tsbService->permanentRecordingMediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("error in adding mediaNode for %s to the dvr list",
                       permRecReq->programName));
            BDBG_OBJECT_DESTROY(tsbService->permanentRecordingMediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*tsbService->permanentRecordingMediaNode),
                                                    false, __FUNCTION__,__LINE__);
            BKNI_Free(tsbService->permanentRecordingMediaNode);
            tsbService->permanentRecordingMediaNode = NULL;
        }
    }
    #endif

    if(!tsbService->permanentRecordingMediaNode)
    {
        BDBG_ERR((" error in finding the mediaNode for %s in subDir %s",permRecReq->programName,permRecReq->subDir));
        rc = B_DVR_INVALID_PARAMETER;
        goto error_permMediaNode;
    }
    BDBG_MSG(("program to be appended %s",tsbService->permanentRecordingMediaNode->programName));
    BDBG_MSG(("time-> recStart: %lu recEnd: %lu",
              tsbService->permanentRecordingMediaNode->mediaStartTime,
              tsbService->permanentRecordingMediaNode->mediaEndTime));
    BDBG_MSG(("media offset-> tsbStart: %lld tsbEnd: %lld",
              tsbService->permanentRecordingMediaNode->mediaLinearStartOffset,
              tsbService->permanentRecordingMediaNode->mediaLinearEndOffset));
    BDBG_MSG(("nav offset-> tsbStart: %lld tsbEnd: %lld",
              tsbService->permanentRecordingMediaNode->navLinearStartOffset,
              tsbService->permanentRecordingMediaNode->navLinearEndOffset));
    B_DVR_MediaStorage_GetMetadataPath(dvrManager->mediaStorage,tsbService->tsbServiceRequest.volumeIndex,metaDataPath);
    BKNI_Memcpy(&tsbService->permRecReq,permRecReq,sizeof(*permRecReq));
    if(permRecReq->subDir[0]=='\0')
    { 
        BKNI_Snprintf(segmentedFileRecord->permRec.navFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s%s",
                      metaDataPath,tsbService->permanentRecordingMediaNode->navFileName,tmp);
        strncpy(tsbService->appendSettings.tempNavMetaDataFileName,segmentedFileRecord->permRec.navFileName,B_DVR_MAX_FILE_NAME_LENGTH);
        tsbService->appendSettings.tempNavMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
        BKNI_Snprintf(segmentedFileRecord->permRec.mediaFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s%s",
                      metaDataPath,tsbService->permanentRecordingMediaNode->mediaFileName,tmp);
        strncpy(tsbService->appendSettings.tempMediaMetaDataFileName,segmentedFileRecord->permRec.mediaFileName,B_DVR_MAX_FILE_NAME_LENGTH);
        tsbService->appendSettings.tempMediaMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';
        BKNI_Snprintf(tsbService->appendSettings.navMetaDataFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s",
                      metaDataPath,tsbService->permanentRecordingMediaNode->navFileName);
        BKNI_Snprintf(tsbService->appendSettings.mediaMetaDataFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s",
                      metaDataPath,tsbService->permanentRecordingMediaNode->mediaFileName);
    }
    else
    {
        BKNI_Snprintf(segmentedFileRecord->permRec.navFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s/%s%s",
                      metaDataPath,permRecReq->subDir,tsbService->permanentRecordingMediaNode->navFileName,tmp);
        strncpy(tsbService->appendSettings.tempNavMetaDataFileName,segmentedFileRecord->permRec.navFileName,B_DVR_MAX_FILE_NAME_LENGTH);
        tsbService->appendSettings.tempNavMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
        BKNI_Snprintf(segmentedFileRecord->permRec.mediaFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s/%s%s",
                      metaDataPath,permRecReq->subDir, tsbService->permanentRecordingMediaNode->mediaFileName,tmp);
        strncpy(tsbService->appendSettings.tempMediaMetaDataFileName,segmentedFileRecord->permRec.mediaFileName,B_DVR_MAX_FILE_NAME_LENGTH);
        tsbService->appendSettings.tempMediaMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';
        BKNI_Snprintf(tsbService->appendSettings.navMetaDataFileName, sizeof(segmentedFileRecord->permRec.navFileName),"%s/%s/%s",
                      metaDataPath,permRecReq->subDir,tsbService->permanentRecordingMediaNode->navFileName);
        BKNI_Snprintf(tsbService->appendSettings.mediaMetaDataFileName,sizeof(segmentedFileRecord->permRec.mediaFileName),"%s/%s/%s",
                      metaDataPath,permRecReq->subDir, tsbService->permanentRecordingMediaNode->mediaFileName);
    }

    BDBG_MSG(("temp: mediaMetaData %s",segmentedFileRecord->permRec.mediaFileName)); 
    BDBG_MSG(("temp: navMetaData %s",segmentedFileRecord->permRec.navFileName)); 

    BDBG_MSG(("appended mediaMetaData %s",tsbService->appendSettings.mediaMetaDataFileName)); 
    BDBG_MSG(("appended navMetaData %s",tsbService->appendSettings.navMetaDataFileName)); 

    segmentedFileRecord->permRec.recStartTime = permRecReq->recStartTime;
    segmentedFileRecord->permRec.recCurrentTime = permRecReq->recStartTime;
    segmentedFileRecord->permRec.recEndTime = permRecReq->recEndTime;
    segmentedFileRecord->permRec.navFileCount =0;
    segmentedFileRecord->permRec.mediaFileCount =0;
    segmentedFileRecord->permRec.pCachedMediaFileNode = NULL;
    segmentedFileRecord->permRec.pCachedNavFileNode = NULL;
    rc = B_DVR_SegmentedFileRecord_OpenPermMetaDataFile(tsbService->nexusFileRecord);
    if(rc != B_DVR_SUCCESS)
    {
         BDBG_ERR(("Error in B_DVR_SegmentedFile_OpenPermMetaDataFile"));
         rc = B_DVR_OS_ERROR;
         goto error_OpenMetaDataFile;
    }

    tsbService->appendSettings.mediaStorage = dvrManager->mediaStorage;
    tsbService->appendSettings.volumeIndex = tsbService->tsbServiceRequest.volumeIndex;
    tsbService->appendSettings.navEntrySize = segmentedFileRecord->navEntrySize;
    trunc = tsbService->permanentRecordingMediaNode->mediaLinearEndOffset%188;
    tsbService->permanentRecordingMediaNode->mediaLinearEndOffset-=trunc;
    tsbService->appendSettings.appendMediaOffset = tsbService->permanentRecordingMediaNode->mediaLinearEndOffset;
    tsbService->appendSettings.appendNavOffset = tsbService->permanentRecordingMediaNode->navLinearEndOffset;
    tsbService->appendSettings.appendTimeStamp = tsbService->permanentRecordingMediaNode->mediaEndTime;
    rc = B_DVR_SegmentedFileRecord_OpenAppendedPermMetaDataFile(&tsbService->appendSettings);
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("Error in B_DVR_SegmentedFileRecord_OpenAppendedPermMetaDataFile"));
        rc = B_DVR_OS_ERROR;
        goto error_OpenAppendedMetaDataFile;
    }
   
    returnedTimeStamp = B_DVR_FileSegmentedRecord_CopyMetaDataFile(tsbService->nexusFileRecord,tsbRecStart,tsbRecEnd);
    if((long)returnedTimeStamp < 0) 
    {
        returnedTimeStamp = segmentedFileRecord->permRec.recStartTime;
    }
    if (returnedTimeStamp < tsbService->permRecReq.recEndTime)
    {
        tsbService->permanentRecordingMediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
        tsbService->tsbMediaNode->mediaAttributes |= (B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
    }
    else
    {
        tsbService->permanentRecordingMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
        tsbService->tsbMediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS);
    }

    BDBG_MSG(("start media %lld ",segmentedFileRecord->permRec.recMediaStartOffset)); 
    BDBG_MSG(("start nav %lld ",segmentedFileRecord->permRec.recNavStartOffset));

    
    tsbService->appendSettings.tsbConvMediaStartOffset = segmentedFileRecord->permRec.recMediaStartOffset;
    tsbService->appendSettings.tsbConvNavStartOffset = segmentedFileRecord->permRec.recNavStartOffset;
    tsbService->appendSettings.tsbConvStartTime = segmentedFileRecord->permRec.recStartTime;
    tsbService->appendSettings.tsbConvMediaCurrentOffset = segmentedFileRecord->permRec.recMediaEndOffset;
    tsbService->appendSettings.tsbConvNavCurrentOffset = segmentedFileRecord->permRec.recNavEndOffset;
    BDBG_MSG(("media perm rec start %lld end %lld",
              segmentedFileRecord->permRec.recMediaStartOffset,
              segmentedFileRecord->permRec.recMediaEndOffset));
   BDBG_MSG(("nav perm rec start %lld end %lld",
              segmentedFileRecord->permRec.recNavStartOffset,
              segmentedFileRecord->permRec.recNavEndOffset));
    B_DVR_SegmentedFileRecord_UpdateAppendedMediaMetaDataFile(&tsbService->appendSettings);
    B_DVR_SegmentedFileRecord_UpdateAppendedNavMetaDataFile(&tsbService->appendSettings);
    tsbService->permanentRecordingMediaNode->mediaEndTime += 
        segmentedFileRecord->permRec.recCurrentTime - segmentedFileRecord->permRec.recStartTime;
     tsbService->permanentRecordingMediaNode->mediaLinearEndOffset +=
        segmentedFileRecord->permRec.recMediaEndOffset - segmentedFileRecord->permRec.recMediaStartOffset;
     tsbService->permanentRecordingMediaNode->navLinearEndOffset +=
        segmentedFileRecord->permRec.recNavEndOffset - segmentedFileRecord->permRec.recNavStartOffset;
     BDBG_MSG(("program to be appended %s",tsbService->permanentRecordingMediaNode->programName));
     BDBG_MSG(("time-> recStart: %lu recEnd: %lu",
               tsbService->permanentRecordingMediaNode->mediaStartTime,
               tsbService->permanentRecordingMediaNode->mediaEndTime));
     BDBG_MSG(("media offset-> tsbStart: %lld tsbEnd: %lld",
               tsbService->permanentRecordingMediaNode->mediaLinearStartOffset,
               tsbService->permanentRecordingMediaNode->mediaLinearEndOffset));
     BDBG_MSG(("nav offset-> tsbStart: %lld tsbEnd: %lld",
               tsbService->permanentRecordingMediaNode->navLinearStartOffset,
               tsbService->permanentRecordingMediaNode->navLinearEndOffset));

    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                               tsbService->permanentRecordingMediaNode,
                               tsbService->tsbServiceRequest.volumeIndex,true);
    if (returnedTimeStamp < tsbService->permRecReq.recEndTime)
    {
         BDBG_MSG((" End time beyong TSB Rec:returnedTimestamp %lu end time %lu",returnedTimeStamp,tsbService->permRecReq.recEndTime));
         tsbService->tsbConversion = true;
         tsbService->appendedConversion = true;
   }
    else
    {
        BDBG_MSG(("Conversion from TSB to Perm Rec succeeded\n"));
        rc = B_DVR_SegmentedFileRecord_CloseAppendedPermMetaDataFile(&tsbService->appendSettings);
        B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(tsbService->nexusFileRecord);
        tsbService->tsbConversion = false;
        tsbService->appendedConversion=false;
        BDBG_MSG((" End of TSB Conversion"));
        if(tsbService->registeredCallback)
        {
            B_Mutex_Unlock(tsbService->tsbMutex);
            event = eB_DVR_EventTSBConverstionCompleted;
            service = eB_DVR_ServiceTSB;
            tsbService->registeredCallback(tsbService->appContext,tsbService->index,event,service);
            B_Mutex_Lock(tsbService->tsbMutex);
        }
    }

    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AppendedConvertStart <<<<<"));
    return rc;
error_OpenAppendedMetaDataFile:
      B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(tsbService->nexusFileRecord);
error_OpenMetaDataFile:
error_permMediaNode:
    pthread_join(tsbService->appendedConversionThread, &pThreadRc);
error_os:
   B_Mutex_Unlock(tsbService->tsbMutex);
   BDBG_MSG(("B_DVR_TSBService_AppendedConvertStart <<<<<"));
   return rc;
}

B_DVR_ERROR B_DVR_TSBService_AppendedConvertStop(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(tsbService->nexusFileRecord);
    void        *pThreadRc;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    B_Mutex_Lock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AppendedConvertStop >>>"));
    BDBG_MSG(("%s: index %d",__FUNCTION__,tsbService->index));
    if (tsbService->tsbConversion)
    {
        B_DVR_FilePosition tsbRecStart,tsbRecEnd;
        tsbService->tsbConversion = false;
        B_DVR_SegmentedFileRecord_GetBounds(tsbService->nexusFileRecord,&tsbRecStart,&tsbRecEnd); 
        if(segmentedFileRecord->permRec.recEndTime > tsbRecEnd.timestamp)
        {
            segmentedFileRecord->permRec.recEndTime = tsbRecEnd.timestamp;
        }
        B_DVR_FileSegmentedRecord_CopyMetaDataFile(tsbService->nexusFileRecord,tsbRecStart,tsbRecEnd);
        B_Event_Set(tsbService->appendedConversionUpdate);
        B_Event_Set(tsbService->appendedConversionThreadStop);
        B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(tsbService->nexusFileRecord);
        pthread_join(tsbService->appendedConversionThread, &pThreadRc);
    }
    BKNI_Memset(&segmentedFileRecord->permRec,0,sizeof(segmentedFileRecord->permRec));
    BKNI_Memset(&tsbService->permRecReq,0,sizeof(tsbService->permRecReq));
    BKNI_Memset(&tsbService->appendSettings,0,sizeof(tsbService->appendSettings));
    B_EventGroup_RemoveEvent(tsbService->eventGroup,tsbService->appendedConversionThreadStop);
    B_EventGroup_RemoveEvent(tsbService->eventGroup,tsbService->appendedConversionUpdate);
    B_Event_Destroy(tsbService->appendedConversionThreadStop);
    B_Event_Destroy(tsbService->appendedConversionUpdate);
    B_EventGroup_Destroy(tsbService->eventGroup);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                    tsbService->tsbServiceRequest.volumeIndex,
                                    tsbService->permanentRecordingMediaNode))
    {
        BDBG_OBJECT_DESTROY(tsbService->permanentRecordingMediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*tsbService->permanentRecordingMediaNode),
                                                false,__FUNCTION__,__LINE__);
        BKNI_Free(tsbService->permanentRecordingMediaNode);
    }
    #endif
    tsbService->permanentRecordingMediaNode=NULL;
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_AppendedConvertStop >>>>"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_SetPlaybackUpdateEvent(
    B_DVR_TSBServiceHandle tsbService,
    B_EventHandle updatePlaybackEvent)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_TSBService_SetPlaybackUpdateEvent>>>"));
    if(tsbService) 
    {
        BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
        B_Mutex_Lock(tsbService->tsbMutex);
        tsbService->updatePlaybackEvent = updatePlaybackEvent;
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    BDBG_MSG(("B_DVR_TSBService_SetPlaybackUpdateEvent <<<"));
    return rc;
}


B_DVR_ERROR B_DVR_TSBService_Pause(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_Pause>>>"));
    B_Mutex_Lock(tsbService->schedulerMutex);
    B_Mutex_Lock(tsbService->tsbMutex);
    if(tsbService->dataInjectionStarted)
    {
        BDBG_ERR(("%s: Data Injection should be stopped before calling pause",
                  tsbService->tsbMediaNode->programName));
        rc = B_DVR_NOT_SUPPORTED;
        goto error;
    }
    rc = B_DVR_TSBService_P_Pause(tsbService);
error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    B_Mutex_Unlock(tsbService->schedulerMutex);
    BDBG_MSG(("B_DVR_TSBService_Pause<<<"));
    return rc;
}


B_DVR_ERROR B_DVR_TSBService_Resume(
    B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_Resume>>>"));
    B_Mutex_Lock(tsbService->tsbMutex);
    rc = B_DVR_TSBService_P_Resume(tsbService);
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_Resume<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_GetIFrameTimeStamp(
    B_DVR_TSBServiceHandle tsbService, 
    unsigned long *timeStamp)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_FilePosition position,iFramePosition;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    B_DVR_FileReadDirection direction = eB_DVR_FileReadDirectionForward;
    unsigned sizeOfFrame=0;
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    BDBG_MSG(("B_DVR_TSBService_GetIFrameTimeStamp >>>"));
    B_Mutex_Lock(tsbService->tsbMutex);

    if(*timeStamp > tsbService->tsbMediaNode->mediaEndTime
       || *timeStamp < tsbService->tsbMediaNode->mediaStartTime) 
    {
        rc = B_DVR_INVALID_PARAMETER;
        BDBG_ERR(("%s:%s Invalid timeStamp startTime %lu endTime %lu timeStamp %lu",
                  __FUNCTION__,tsbService->tsbMediaNode->programName,
                  tsbService->tsbMediaNode->mediaStartTime,
                  tsbService->tsbMediaNode->mediaEndTime,*timeStamp));
        goto error;
    }

    rc = B_DVR_SegmentedFilePlay_GetLocation(tsbService->nexusFilePlay1,
                                             -1,*timeStamp,&position);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:%s Unable to get position info for timeStamp:%lu",
                  __FUNCTION__,tsbService->tsbMediaNode->programName,
                  *timeStamp));
        goto error;
    }

    rc = NEXUS_Playback_GetStatus(tsbService->playback,&nexusPlaybackStatus);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:%s Unable to get tsb playback Status",
                  __FUNCTION__,tsbService->tsbMediaNode->programName)); 
        goto error;
    }

    if(nexusPlaybackStatus.position > *timeStamp)
    {
        direction = eB_DVR_FileReadDirectionReverse;
    }
    rc = B_DVR_SegmentedFilePlay_GetNextIFrame(tsbService->nexusFilePlay1, 
                                               position,&iFramePosition,
                                               &sizeOfFrame,
                                               direction); 
    if(rc!=B_DVR_SUCCESS) 
    {
         BDBG_ERR(("%s:%s Unable to get I Frame location direction:%u timeStamp:%lu",
                   __FUNCTION__,tsbService->tsbMediaNode->programName,direction,
                   position.timestamp)); 
         goto error;

    }

    BDBG_MSG(("%s:%s iFrame Info",__FUNCTION__,tsbService->tsbMediaNode->programName));
    BDBG_MSG(("timeStamp %lu index %lu mediaLinearOffset %lld navLinearOffset %lld",
              iFramePosition.timestamp,iFramePosition.index,iFramePosition.mpegFileOffset, 
              iFramePosition.navFileOffset));
    *timeStamp = iFramePosition.timestamp;
error:
    B_Mutex_Unlock(tsbService->tsbMutex);
    BDBG_MSG(("B_DVR_TSBService_GetIFrameTimeStamp <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_AddInProgressRecPlayback(
   B_DVR_TSBServiceHandle tsbService,
   NEXUS_PlaybackHandle playback)
{

    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_TSBService_AddInProgressRecPlayback>>>"));
    if(tsbService && playback) 
    {
        BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
        B_Mutex_Lock(tsbService->tsbMutex);
        NEXUS_Record_AddPlayback(tsbService->record, playback);
        tsbService->inProgressRecPlay = playback;
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    BDBG_MSG(("B_DVR_TSBService_AddInProgressRecPlayback <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_TSBService_RemoveInProgressRecPlayback(B_DVR_TSBServiceHandle tsbService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_MSG(("B_DVR_TSBService_RemoveInProgressRecPlayback>>>"));
    BDBG_OBJECT_ASSERT(tsbService,B_DVR_TSBService);
    if(tsbService->inProgressRecPlay) 
    {
        B_Mutex_Lock(tsbService->tsbMutex);
        NEXUS_Record_RemovePlayback(tsbService->record,tsbService->inProgressRecPlay);
        tsbService->inProgressRecPlay = NULL;
        B_Mutex_Unlock(tsbService->tsbMutex);
    }
    BDBG_MSG(("B_DVR_TSBService_RemoveInProgressRecPlayback <<<"));
    return rc;
}
