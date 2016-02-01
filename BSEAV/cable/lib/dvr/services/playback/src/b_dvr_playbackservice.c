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
#include "b_dvr_mediastorage.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_playbackservice.h"
#include "b_dvr_recordservice.h"
#include "b_dvr_tsbservice.h"

BDBG_MODULE(b_dvr_playbackservice);
BDBG_OBJECT_ID(B_DVR_PlaybackService);

struct B_DVR_PlaybackService
{
    BDBG_OBJECT(B_DVR_PlaybackService)
    unsigned index;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle nexusFilePlay;
    NEXUS_FilePlayHandle nexusFilePlay1;
    B_DVR_MediaNode mediaNode;
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_OperationSettings operationSettings;
    B_MutexHandle playbackMutex;
    B_DVR_PlaybackServiceRequest playbackServiceRequest;
    B_DVR_ServiceCallback registeredCallback;
    void *appContext;
    B_SchedulerTimerId alarmTimer;
    B_SchedulerEventId trackRecordSchedulerID;
    B_EventHandle trackRecordEvent;
    unsigned long alarmTime;
    int recordServiceIndex;
    int tsbServiceIndex;
    NEXUS_KeySlotHandle keySlot;
    NEXUS_DmaHandle dma;
    B_MutexHandle schedulerMutex;
};
void B_DVR_PlaybackService_P_InProgressRecordingUpdate(void * playbackContext)
{

    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_PlaybackServiceHandle playbackService = (B_DVR_PlaybackServiceHandle)playbackContext;
    B_DVR_SegmentedFilePlaySettings segmentedFilePlaySettings;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_PlaybackService_P_InProgressRecordingUpdate >>>"));

    B_Mutex_Lock(playbackService->playbackMutex);
    B_DVR_SegmentedFilePlay_GetSettings(playbackService->nexusFilePlay,&segmentedFilePlaySettings);
    B_DVR_SegmentedFilePlay_GetSettings(playbackService->nexusFilePlay1,&segmentedFilePlaySettings);
    if(playbackService->mediaNode->mediaLinearEndOffset > segmentedFilePlaySettings.mediaLinearEndOffset)
    {
          B_DVR_SegmentedFilePlay_UpdateFileList(playbackService->nexusFilePlay);
          B_DVR_SegmentedFilePlay_UpdateFileList(playbackService->nexusFilePlay1);
          segmentedFilePlaySettings.mediaLinearEndOffset = playbackService->mediaNode->mediaLinearEndOffset;
          segmentedFilePlaySettings.navLinearEndOffset = playbackService->mediaNode->navLinearEndOffset;
          B_DVR_SegmentedFilePlay_SetSettings(playbackService->nexusFilePlay,&segmentedFilePlaySettings);
          B_DVR_SegmentedFilePlay_SetSettings(playbackService->nexusFilePlay1,&segmentedFilePlaySettings);
    }
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_P_InProgressRecordingUpdate <<<"));
    return;
}

static void B_DVR_PlaybackService_P_AlarmTimer(void *playbackContext)
{
    B_DVR_PlaybackServiceHandle playbackService = (B_DVR_PlaybackServiceHandle)playbackContext;
    B_DVR_Event event;
    B_DVR_Service service;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    bool alarmOn=false;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    NEXUS_Playback_GetStatus(playbackService->playback,&nexusPlaybackStatus);

    if((nexusPlaybackStatus.state == NEXUS_PlaybackState_eTrickMode) &&
       (playbackService->operationSettings.operation == eB_DVR_OperationFastRewind ||
        playbackService->operationSettings.operation == eB_DVR_OperationSlowRewind || 
        playbackService->operationSettings.operationSpeed < 0)
       )
    {
        alarmOn = (nexusPlaybackStatus.position  <= playbackService->alarmTime)? true:false;
    }
    else
    {
       alarmOn = (nexusPlaybackStatus.position  >= playbackService->alarmTime)? true:false;
    }

    if(alarmOn)
    {
        playbackService->alarmTimer=NULL;
        playbackService->alarmTime = 0;
        if(playbackService->registeredCallback)
        {
            event = eB_DVR_EventPlaybackAlarm;
            service = eB_DVR_ServicePlayback;
            playbackService->registeredCallback(playbackService->appContext,playbackService->index,event,service);
        }
    }
    else
    {

        B_Mutex_Lock(playbackService->playbackMutex);
        playbackService->alarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                             playbackService->schedulerMutex,
                                                             1000,B_DVR_PlaybackService_P_AlarmTimer,
                                                             (void*)playbackService);
        B_Mutex_Unlock(playbackService->playbackMutex);
    }
    return;
}

static void B_DVR_PlaybackService_P_SegmentedFileEvent(void *segmentedFileContext,B_DVR_SegmentedFileEvent fileEvent)
{
    B_DVR_Event event=eB_DVR_EventMax;
    B_DVR_Service service;
    B_DVR_SegmentedFileHandle segmentedFile = (B_DVR_SegmentedFileHandle)(segmentedFileContext);
    B_DVR_PlaybackServiceHandle playbackService = NULL;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_ASSERT(segmentedFile);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_PlaybackService_P_SegmentedFileEvent >>>"));
    BDBG_MSG(("index %d service %d event %d",segmentedFile->serviceIndex,
              segmentedFile->service,fileEvent));

    playbackService = dvrManager->playbackService[segmentedFile->serviceIndex];
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    if(playbackService->registeredCallback)
    {
        switch(fileEvent)
        {
        case B_DVR_SegmentedFileEventReadError:
            {
                event = eB_DVR_EventAbortPlayback;
                BDBG_ERR(("Abort Playback Event - Read Error"));
            }
            break;
        case B_DVR_SegmentedFileEventSeekError:
            {
                event = eB_DVR_EventAbortPlayback;
                BDBG_ERR(("Abort Playback Event - Seek Error"));
            }
            break;
        case B_DVR_SegmentedFileEventWriteError:
            {
                BDBG_ERR(("Write Error - not applicable"));
                goto error;
            }
            break;
        case B_DVR_SegmentedFileEventOpenError:
            {
                event = eB_DVR_EventAbortPlayback;
                BDBG_ERR(("Abort Playback Event - Open Error"));
            }
            break;
        case B_DVR_SegmentedFileEventNotFoundError:
            {
                event = eB_DVR_EventAbortPlayback;
                BDBG_ERR(("Abort TSBPlayback Event - file segment not found"));
            }
            break;
        case B_DVR_SegmentedFileEventNoFileSpaceError:
            {
                BDBG_ERR(("no space : not applicable"));
                goto error;
            }
            break;
        case B_DVR_SegmentFileEventMax:
            {
                BDBG_ERR(("Invalid segmented file event"));
                goto error;
            }
            break;
        }
        service = eB_DVR_ServicePlayback;
        playbackService->registeredCallback(playbackService->appContext,segmentedFile->service,event,segmentedFile->service);
    }
error:
    BDBG_MSG(("B_DVR_PlaybackService_P_SegmentedFileEvent <<<<"));
    return;
}

static void B_DVR_PlaybackService_P_StartOfStream(
    void *context,
    int param)
{
    B_DVR_PlaybackServiceHandle playbackService = (B_DVR_PlaybackServiceHandle)context;
    B_DVR_Event event = eB_DVR_EventStartOfPlayback;
    B_DVR_Service service = eB_DVR_ServicePlayback;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_WRN(("%s index = %d", __FUNCTION__,playbackService->index));
    if(playbackService->registeredCallback)
    {
        playbackService->registeredCallback(playbackService->appContext,param,event,service);
    }
    return;
}

static void B_DVR_PlaybackService_P_EndOfStream(
    void *context,
    int param)
{
    B_DVR_PlaybackServiceHandle playbackService = (B_DVR_PlaybackServiceHandle)context;
    B_DVR_Event event = eB_DVR_EventEndOfPlayback;
    B_DVR_Service service = eB_DVR_ServicePlayback;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_WRN(("%s index = %d", __FUNCTION__,playbackService->index));
    if(playbackService->registeredCallback)
    {
        playbackService->registeredCallback(playbackService->appContext,param,event,service);
    }
    return;
}

NEXUS_VideoCodec B_DVR_PlaybackService_P_GetVideoCodec(B_DVR_PlaybackServiceHandle playbackService)
{
  unsigned esStreamIndex=0;
  NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eMpeg2 ;
  BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
  for(esStreamIndex=0;esStreamIndex < playbackService->mediaNode->esStreamCount; esStreamIndex++)
  {
      if(playbackService->mediaNode->esStreamInfo[esStreamIndex].pidType == eB_DVR_PidTypeVideo)
      {
          videoCodec = playbackService->mediaNode->esStreamInfo[esStreamIndex].codec.videoCodec;
          break;
      }
  }
  return videoCodec;
}

B_DVR_PlaybackServiceHandle B_DVR_PlaybackService_Open(
    B_DVR_PlaybackServiceRequest * playbackServiceRequest)
{
    B_DVR_PlaybackServiceHandle playbackService=NULL;
    B_DVR_ManagerHandle dvrManager;
    B_DVR_SegmentedFileSettings segmentedFileSettings;
    NEXUS_PlaybackSettings playbackSettings;
    B_DVR_SegmentedFilePlaySettings segmentedFilePlaySettings;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    unsigned index=0;
    B_DVR_ERROR rc = B_DVR_SUCCESS;

    BDBG_ASSERT(playbackServiceRequest);
    BDBG_MSG(("B_DVR_PlaybackService_Open >>>"));
    dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    playbackService = BKNI_Malloc(sizeof(*playbackService));
    if (!playbackService)
    {
        BDBG_ERR(("%s  playbackService alloc failed", __FUNCTION__));
        goto error_alloc;
    }
    BKNI_Memset(playbackService, 0, sizeof(*playbackService));
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*playbackService),
                                            true, __FUNCTION__,__LINE__);
    BDBG_OBJECT_SET(playbackService,B_DVR_PlaybackService);
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    /* find a playbackService slot in the dvrManager */
    for(index=0;index<B_DVR_MAX_PLAYBACK;index++)
    {
        if(!dvrManager->playbackService[index])
        {
            BDBG_MSG(("Playback Service Index %u",index));
            playbackService->index=index;
            break;
        }
    }
    if(index >=B_DVR_MAX_PLAYBACK)
    {
        BDBG_ERR(("MAX PlaybackService instances used up. Free up a PlaybackService instance"));
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        goto error_maxPlaybackServiceCount;
    }
    dvrManager->playbackService[playbackService->index] = playbackService;
    dvrManager->playbackServiceCount++;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    BKNI_Memcpy(&playbackService->playbackServiceRequest,playbackServiceRequest, sizeof(*playbackServiceRequest));
    playbackService->playbackMutex = B_Mutex_Create(NULL);
    if(!playbackService->playbackMutex)
    {
        BDBG_ERR(("%s mutex create failed", __FUNCTION__));
        goto error_mutex_create;
    }

    playbackService->schedulerMutex = B_Mutex_Create(NULL);
    if( !playbackService->schedulerMutex)
    {
        BDBG_ERR(("%s schedulerMutex create error", __FUNCTION__));
        goto error_schedulerMutex;
    }

    playbackService->playpump = NEXUS_Playpump_Open(playbackServiceRequest->playpumpIndex, NULL);
    if(!playbackService->playpump)
    {
        BDBG_ERR(("%s playpump open failed %d", __FUNCTION__,dvrManager->playbackServiceCount));
        goto error_playpump_open;
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpOpenSettings.fifoSize,
                                            true,__FUNCTION__,__LINE__);
    playbackService->playback = NEXUS_Playback_Create();
    playbackService->playbackServiceSettings.playback = playbackService->playback;
    if(!playbackService->playback)
    {
        BDBG_ERR(("%s playback create failed %d", __FUNCTION__,dvrManager->playbackServiceCount));
        goto error_playback_create;
    }

    if(playbackServiceRequest->subDir[0] == '\0')
    {
        mediaNodeSettings.subDir = NULL;
    }
    else
    {
        mediaNodeSettings.subDir = &playbackServiceRequest->subDir[0];
    }

    mediaNodeSettings.programName = &playbackServiceRequest->programName[0];
    mediaNodeSettings.volumeIndex = playbackServiceRequest->volumeIndex;
    playbackService->mediaNode = B_DVR_List_GetMediaNode(dvrManager->dvrList,
                                                         &mediaNodeSettings,
                                                         true);
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!playbackService->mediaNode)
    {
        playbackService->mediaNode = BKNI_Malloc(sizeof(*playbackService->mediaNode));
        if(!playbackService->mediaNode) 
        {
             BDBG_ERR(("error in allocating playbackService->mediaNode for %s",
                       playbackServiceRequest->programName));
             goto error_mediaNode;
        }
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*playbackService->mediaNode),
                                                true, __FUNCTION__,__LINE__);
        rc = B_DVR_List_GetMediaNodeFile(dvrManager->dvrList,
                                         &mediaNodeSettings,
                                         playbackService->mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("error in getting mediaNode for %s",
                       playbackServiceRequest->programName));
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*playbackService->mediaNode),
                                                    false, __FUNCTION__,__LINE__);
            BKNI_Free(playbackService->mediaNode);
            playbackService->mediaNode = NULL;
            goto error_mediaNode;
        }
        rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,
                                     mediaNodeSettings.volumeIndex,
                                     playbackService->mediaNode);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("error in adding mediaNode for %s to the dvr list",
                       playbackServiceRequest->programName));
            BDBG_OBJECT_DESTROY(playbackService->mediaNode,B_DVR_Media);
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*playbackService->mediaNode),
                                                    false, __FUNCTION__,__LINE__);
            BKNI_Free(playbackService->mediaNode);
            playbackService->mediaNode = NULL;
        }
    }
    #endif
    if(!playbackService->mediaNode)
    {
        BDBG_ERR(("%s media Node for %s not found", __FUNCTION__,playbackServiceRequest->programName));
        goto error_mediaNode;
    }
    BDBG_OBJECT_ASSERT(playbackService->mediaNode,B_DVR_Media);
    BDBG_MSG(("media %lld:%lld %ld:%ld",
              playbackService->mediaNode->mediaLinearStartOffset,
              playbackService->mediaNode->mediaLinearEndOffset,
              playbackService->mediaNode->mediaStartTime,
              playbackService->mediaNode->mediaEndTime));

    BDBG_MSG(("nav %lld:%lld",
              playbackService->mediaNode->navLinearStartOffset,
              playbackService->mediaNode->navLinearEndOffset
              ));

    /* 
     * Make sure that the start address is 4096xtransport packet size aligned. Otherwise nexus playback returns 
     * errors.
     */

    playbackService->mediaNode->mediaLinearStartOffset -= (playbackService->mediaNode->mediaLinearStartOffset%(188*4096));
    BDBG_MSG(("%lld:%lld %ld:%ld",
              playbackService->mediaNode->mediaLinearStartOffset,
              playbackService->mediaNode->mediaLinearEndOffset,
              playbackService->mediaNode->mediaStartTime,
              playbackService->mediaNode->mediaEndTime));
    
    playbackService->playbackServiceSettings.startTime = playbackService->mediaNode->mediaStartTime;
    playbackService->playbackServiceSettings.endTime = playbackService->mediaNode->mediaEndTime;
    segmentedFileSettings.event = NULL;        /*unused in segmented playback */
    segmentedFileSettings.maxSegmentCount = 0; /*unused in segmented playback */
    segmentedFileSettings.mediaSegmentSize = 0; /*unused in segmented playback */
    segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
    segmentedFileSettings.registeredCallback = B_DVR_PlaybackService_P_SegmentedFileEvent;
    segmentedFileSettings.service = eB_DVR_ServicePlayback;
    segmentedFileSettings.serviceIndex = playbackService->index;
    segmentedFileSettings.volumeIndex = playbackServiceRequest->volumeIndex;
    segmentedFileSettings.itbThreshhold = 0; /* This is used for generating the mediaNode update events only during recording */
    if(playbackService->mediaNode->mediaNodeSubDir[0]=='\0')
    {
        segmentedFileSettings.metaDataSubDir = NULL;
    }
    else
    {
        segmentedFileSettings.metaDataSubDir = (char *)playbackService->mediaNode->mediaNodeSubDir;
    }

    if(playbackService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED) 
    {
        rc = B_DVR_SegmentedFilePlay_SanityCheck(playbackService->mediaNode->mediaFileName,
                                                 playbackService->mediaNode->navFileName,
                                                 &segmentedFileSettings);
        if(rc!=B_DVR_SUCCESS) 
        {
            BDBG_ERR(("%s:Program:%s is corrupted and it's recommended to delete this program",
                      __FUNCTION__,playbackService->mediaNode->programName));
            goto error_nexusFilePlay_open;
        }
    }
    playbackService->nexusFilePlay = B_DVR_SegmentedFilePlay_Open(playbackService->mediaNode->mediaFileName,
                                                                  playbackService->mediaNode->navFileName,
                                                                  &segmentedFileSettings,NULL);
    playbackService->nexusFilePlay1 = B_DVR_SegmentedFilePlay_Open(playbackService->mediaNode->mediaFileName,
                                                                   playbackService->mediaNode->navFileName,
                                                                   &segmentedFileSettings,NULL);
    
        
    B_DVR_SegmentedFilePlay_GetDefaultSettings(&segmentedFilePlaySettings);
    segmentedFilePlaySettings.mediaLinearStartOffset = playbackService->mediaNode->mediaLinearStartOffset;
    segmentedFilePlaySettings.mediaLinearEndOffset = playbackService->mediaNode->mediaLinearEndOffset;
    segmentedFilePlaySettings.navLinearEndOffset = playbackService->mediaNode->navLinearEndOffset;
    segmentedFilePlaySettings.navLinearStartOffset = playbackService->mediaNode->navLinearStartOffset;
    if(B_DVR_PlaybackService_P_GetVideoCodec(playbackService) == NEXUS_VideoCodec_eMpeg2)
    { 
        segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_Entry);
    }
    else
    {
        segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_AVC_Entry);
    }
    if(playbackService->nexusFilePlay)
    {
        B_DVR_SegmentedFilePlay_SetSettings(playbackService->nexusFilePlay,&segmentedFilePlaySettings);
        BDBG_MSG(("B_DVR_SegmentedFilePlay_Open"));
    }
    if(playbackService->nexusFilePlay1)
    {
        B_DVR_SegmentedFilePlay_SetSettings(playbackService->nexusFilePlay1,&segmentedFilePlaySettings);
        BDBG_MSG(("B_DVR_SegmentedFilePlay_Open"));
    }

    if(!playbackService->nexusFilePlay || !playbackService->nexusFilePlay1)
    {
        BDBG_ERR(("filePlay open failed %s %s", playbackService->mediaNode->mediaFileName,
                  playbackService->mediaNode->navFileName));
        if(playbackService->nexusFilePlay) 
        {
            B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay);
        }
        if(playbackService->nexusFilePlay1) 
        {
            B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay1);
        }
        goto error_nexusFilePlay_open;
    }
    
    if(playbackService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED) 
    {
        B_DVR_FilePosition first,last;
        rc = B_DVR_SegmentedFilePlay_GetBounds(playbackService->nexusFilePlay,&first,&last);
        if(rc!=B_DVR_SUCCESS) 
        {
            B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay);
            B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay1);
            BDBG_ERR(("B_DVR_SegmentedFilePlay_GetBounds failed for aborted recording %s",
                      playbackService->mediaNode->programName));
            goto error_nexusFilePlay_open;
        }
        BDBG_WRN(("%s:updating the nav end off %lld with %lld",
                  __FUNCTION__,
                  playbackService->mediaNode->navLinearEndOffset,
                  last.navFileOffset));
        BDBG_WRN(("%s:updating the media end off %lld with %lld",
                   __FUNCTION__,
                   playbackService->mediaNode->mediaLinearEndOffset,
                   last.mpegFileOffset));
        BDBG_WRN(("%s:updating the media end time %u with %u",
                  __FUNCTION__,
                  playbackService->mediaNode->mediaEndTime,
                 last.timestamp));
        playbackService->mediaNode->mediaLinearEndOffset = last.mpegFileOffset;
        playbackService->mediaNode->navLinearEndOffset = last.navFileOffset;
        playbackService->mediaNode->mediaEndTime = last.timestamp;
        playbackService->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED);
        B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                   playbackService->mediaNode,
                                   playbackServiceRequest->volumeIndex,true);
    }

    NEXUS_Playback_GetSettings(playbackService->playback, &playbackSettings);
    playbackSettings.playpump = playbackService->playpump;
    playbackSettings.playpumpSettings.transportType = playbackService->mediaNode->transportType;
    playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.beginningOfStreamCallback.callback =  B_DVR_PlaybackService_P_StartOfStream;
    playbackSettings.beginningOfStreamCallback.context = playbackService;
    playbackSettings.beginningOfStreamCallback.param = playbackService->index;
     if(!(playbackService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS)
        && playbackServiceRequest->loopMode)
     {
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
     }
     else
     {
         if(playbackServiceRequest->loopMode)
         {
             BDBG_WRN(("%s overriding the loop mode request from the app because the requested"
                       "playback is for in-progress recording"));
         }
         playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    }
    playbackSettings.endOfStreamCallback.callback = B_DVR_PlaybackService_P_EndOfStream;
    playbackSettings.endOfStreamCallback.context = playbackService;
    playbackSettings.endOfStreamCallback.param = playbackService->index;
    if(playbackService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS)
    {
        playbackSettings.timeshifting = true;
    }
    else
    {
         playbackSettings.timeshifting = false;
    }
    NEXUS_Playback_SetSettings(playbackService->playback, &playbackSettings);
    
    BDBG_MSG(("B_DVR_PlaybackService_Open <<"));
    return playbackService;
error_nexusFilePlay_open:
error_mediaNode:
    NEXUS_Playback_Destroy(playbackService->playback);
error_playback_create:
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpOpenSettings.fifoSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Playpump_Close(playbackService->playpump);
error_playpump_open:
    B_Mutex_Destroy(playbackService->schedulerMutex); 
error_schedulerMutex:
    B_Mutex_Destroy(playbackService->playbackMutex); 
error_mutex_create:
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->playbackService[playbackService->index] = NULL;
    dvrManager->playbackServiceCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
error_maxPlaybackServiceCount:
    BDBG_OBJECT_DESTROY(playbackService,B_DVR_PlaybackService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*playbackService),
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(playbackService);
error_alloc:
    BDBG_ERR(("B_DVR_PlaybackService_Open << error NULL"));
    return NULL;
}

B_DVR_ERROR B_DVR_PlaybackService_Close(
    B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    NEXUS_PlaypumpStatus playpumpStatus;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->playbackService[playbackService->index] = NULL;
    dvrManager->playbackServiceCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(playbackService->playbackMutex);
    B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay);
    B_DVR_SegmentedFilePlay_Close(playbackService->nexusFilePlay1);
    NEXUS_Playback_Destroy(playbackService->playback);
    NEXUS_Playpump_GetStatus(playbackService->playpump,&playpumpStatus);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_Device,
                                            playpumpStatus.fifoSize,
                                            false,__FUNCTION__,__LINE__);
    NEXUS_Playpump_Close(playbackService->playpump);
    B_Mutex_Destroy(playbackService->schedulerMutex); 
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                    playbackService->playbackServiceRequest.volumeIndex,
                                    playbackService->mediaNode)) 
    {
        BDBG_OBJECT_DESTROY(playbackService->mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*playbackService->mediaNode),
                                                false, __FUNCTION__,__LINE__);
        BKNI_Free(playbackService->mediaNode);
    }
    #endif
    B_Mutex_Unlock(playbackService->playbackMutex);
    B_Mutex_Destroy(playbackService->playbackMutex);
    BDBG_OBJECT_DESTROY(playbackService,B_DVR_PlaybackService);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*playbackService),
                                            false, __FUNCTION__,__LINE__);
    BKNI_Free(playbackService);
    return rc;
}


B_DVR_ERROR B_DVR_PlaybackService_Start(
    B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    NEXUS_PlaybackStatus status;
    NEXUS_PlaybackSettings playbackSettings;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    B_Mutex_Lock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_Start >>>>>"));

    if(playbackService->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS)
    {
        unsigned index=0;
        B_DVR_MediaNode recordMediaNode=NULL;
        playbackService->trackRecordEvent = B_Event_Create(NULL);
        if(!playbackService->trackRecordEvent )
        {
            BDBG_ERR(("%s event create failed"));
            goto error_trackRecordevent;
        }

        playbackService->trackRecordSchedulerID =  B_Scheduler_RegisterEvent(dvrManager->scheduler,
                                                                             playbackService->schedulerMutex,
                                                                             playbackService->trackRecordEvent,
                                                                             B_DVR_PlaybackService_P_InProgressRecordingUpdate,
                                                                             (void*)playbackService);
        if(!playbackService->trackRecordSchedulerID)
        {
            BDBG_ERR(("%s event registration  failed"));
            goto error_trackRecordSchedulerID;
        }

        playbackService->tsbServiceIndex = -1;
        playbackService->recordServiceIndex = -1;
        B_Mutex_Lock(dvrManager->dvrManagerMutex);
        for(index=0;index<B_DVR_MAX_TSB;index++)
        {
            if(dvrManager->tsbService[index])  
            { 
                recordMediaNode = B_DVR_TSBService_GetMediaNode(dvrManager->tsbService[index],true);
                if(recordMediaNode == playbackService->mediaNode)
                {
                    BDBG_MSG(("recordMediaNode is an ongoing tsb converted recording"));
                    BDBG_MSG(("recordMediaNode program %s subDir %s",recordMediaNode->programName,recordMediaNode->mediaNodeSubDir));
                    BDBG_MSG(("playbackMediaNode program %s subDir %s",playbackService->mediaNode->programName,playbackService->mediaNode->mediaNodeSubDir));
                    B_DVR_TSBService_SetPlaybackUpdateEvent(dvrManager->tsbService[index],playbackService->trackRecordEvent);
                    B_DVR_TSBService_AddInProgressRecPlayback(dvrManager->tsbService[index],
                                                              playbackService->playback);
                    playbackService->tsbServiceIndex = index;
                    break;
                }

            }
        }
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);

        if(!recordMediaNode)
        { 
            B_Mutex_Lock(dvrManager->dvrManagerMutex);
            for(index=0;index<B_DVR_MAX_RECORDING;index++)
            {
                if(dvrManager->recordService[index])  
                { 
                    recordMediaNode = B_DVR_RecordService_GetMediaNode(dvrManager->recordService[index]);
                    if(recordMediaNode == playbackService->mediaNode)
                    {
                        BDBG_MSG(("recordMediaNode is an ongoing TSB conversion"));
                        BDBG_MSG(("recordMediaNode program %s subDir %s",recordMediaNode->programName,recordMediaNode->mediaNodeSubDir));
                        BDBG_MSG(("playbackMediaNode program %s subDir %s",playbackService->mediaNode->programName,playbackService->mediaNode->mediaNodeSubDir));
                        B_DVR_RecordService_SetPlaybackUpdateEvent(dvrManager->recordService[index],playbackService->trackRecordEvent);
                        B_DVR_RecordService_AddInProgressRecPlayback(dvrManager->recordService[index],
                                                                     playbackService->playback);
                        playbackService->recordServiceIndex = index;
                        break;
                    }

                }
            }
            B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        }
    }
    else
    {
        playbackService->trackRecordEvent =NULL;
        playbackService->trackRecordSchedulerID = NULL;
        playbackService->tsbServiceIndex = -1;
        playbackService->recordServiceIndex = -1;
    }

    NEXUS_Playback_GetSettings(playbackService->playback, &playbackSettings);
    if(playbackService->playbackServiceSettings.stcChannel) 
    {
        playbackSettings.stcChannel = playbackService->playbackServiceSettings.stcChannel;
    }
    else
    {
        playbackSettings.simpleStcChannel = playbackService->playbackServiceSettings.simpleStcChannel;
    }
    playbackSettings.stcTrick = true;
    NEXUS_Playback_SetSettings(playbackService->playback, &playbackSettings);
    NEXUS_Playback_Start(playbackService->playback, playbackService->nexusFilePlay, NULL);
    NEXUS_Playback_GetStatus(playbackService->playback,&status);
    BDBG_MSG((" playback position first %d  last %d", status.first, status.last));
    BDBG_MSG(("media  first %d  last %d",playbackService->mediaNode->mediaStartTime, playbackService->mediaNode->mediaEndTime));
    BDBG_MSG(("B_DVR_PlaybackService_Start <<<"));

    if(playbackService->alarmTime && !playbackService->alarmTimer)
    { 
        playbackService->alarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                             playbackService->schedulerMutex,
                                                             1000,B_DVR_PlaybackService_P_AlarmTimer,
                                                             (void*)playbackService);
    }

    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;

error_trackRecordSchedulerID:
    B_Event_Destroy(playbackService->trackRecordEvent);
error_trackRecordevent:
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_Stop(
    B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);

    B_Mutex_Lock(playbackService->schedulerMutex);
    if(playbackService->trackRecordSchedulerID)
    { 
        B_Scheduler_UnregisterEvent(dvrManager->scheduler,playbackService->trackRecordSchedulerID);
        B_Event_Reset(playbackService->trackRecordEvent);
    }
    if(playbackService->alarmTimer)
    {
        B_Scheduler_CancelTimer(dvrManager->scheduler,playbackService->alarmTimer);
    }
    B_Mutex_Unlock(playbackService->schedulerMutex);

    B_Mutex_Lock(playbackService->playbackMutex);
    if(playbackService->trackRecordSchedulerID)
    {
        if(playbackService->tsbServiceIndex >= 0 && 
            dvrManager->tsbService[playbackService->tsbServiceIndex])
        {
            B_DVR_MediaNode recordMediaNode=NULL;
            recordMediaNode = B_DVR_TSBService_GetMediaNode(dvrManager->tsbService[playbackService->tsbServiceIndex],true);
            if(recordMediaNode == playbackService->mediaNode)
            {
                B_DVR_TSBService_RemoveInProgressRecPlayback(dvrManager->tsbService[playbackService->tsbServiceIndex]);
                B_DVR_TSBService_SetPlaybackUpdateEvent(dvrManager->tsbService[playbackService->tsbServiceIndex],
                                                        NULL);
            }
        }
        if(playbackService->recordServiceIndex >= 0 &&
           dvrManager->recordService[playbackService->recordServiceIndex])
        {
            B_DVR_MediaNode recordMediaNode=NULL;
            recordMediaNode = B_DVR_RecordService_GetMediaNode(dvrManager->recordService[playbackService->recordServiceIndex]);
            if(recordMediaNode == playbackService->mediaNode)
            {

                B_DVR_RecordService_RemoveInProgressRecPlayback(dvrManager->recordService[playbackService->recordServiceIndex]);
                B_DVR_RecordService_SetPlaybackUpdateEvent(dvrManager->recordService[playbackService->recordServiceIndex],
                                                        NULL);
            }
        }
        playbackService->trackRecordSchedulerID = NULL;
    }
    if(playbackService->trackRecordEvent)
    {
        B_Event_Destroy(playbackService->trackRecordEvent);
    }
    playbackService->alarmTimer = NULL;
    NEXUS_Playback_Stop(playbackService->playback);
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_SetOperation(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_OperationSettings *operationSettings)
{
    B_DVR_ERROR rc =B_DVR_SUCCESS;

    BDBG_MSG(("%s index %d", __FUNCTION__,playbackService->index));

    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_ASSERT(operationSettings);
    BKNI_Memcpy((void*)&playbackService->operationSettings,operationSettings,sizeof(*operationSettings));
    
    B_Mutex_Lock(playbackService->playbackMutex);

    
    switch(operationSettings->operation)
    {
    case eB_DVR_OperationPause:
        {
            BDBG_MSG(("eB_DVR_OperationPause"));
            NEXUS_Playback_Pause(playbackService->playback);
        }
        break;
    case eB_DVR_OperationPlay:
        {
            BDBG_MSG(("eB_DVR_OperationPlay"));
            NEXUS_Playback_Play(playbackService->playback);
        }
        break;
    case eB_DVR_OperationPlayGop:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationPlayGop"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayGop;
            trickModeSettings.mode_modifier = operationSettings->mode_modifier;
            trickModeSettings.rate = operationSettings->operationSpeed;
            NEXUS_Playback_TrickMode(playbackService->playback,&trickModeSettings);
        }
        break;
     case eB_DVR_OperationPlayTimeSkip:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationPlayTimeSkip"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_eTimeSkip;
            trickModeSettings.mode_modifier = operationSettings->mode_modifier;
            trickModeSettings.rate = operationSettings->operationSpeed;
            NEXUS_Playback_TrickMode(playbackService->playback,&trickModeSettings);
        }
        break;
    case eB_DVR_OperationFastForward:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationFastForward"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;;
            trickModeSettings.mode_modifier = operationSettings->operationSpeed;
            trickModeSettings.rate = NEXUS_NORMAL_DECODE_RATE;
            NEXUS_Playback_TrickMode(playbackService->playback,&trickModeSettings);
       }
       break;
    case eB_DVR_OperationFastRewind:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationFastRewind"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
            trickModeSettings.mode_modifier = -operationSettings->operationSpeed;
            trickModeSettings.rate = NEXUS_NORMAL_DECODE_RATE;
            NEXUS_Playback_TrickMode(playbackService->playback,&trickModeSettings);
        }
        break;
    case eB_DVR_OperationSlowForward:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationSlowRewind"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode_modifier = 1;
            trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED/operationSettings->operationSpeed;
            trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_eNormal;
            NEXUS_Playback_TrickMode(playbackService->playback, &trickModeSettings);
        }
        break;
    case eB_DVR_OperationSlowRewind:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            BDBG_MSG(("eB_DVR_OperationSlowRewind"));
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.mode_modifier = -1;
            trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED/operationSettings->operationSpeed;
            if(B_DVR_PlaybackService_P_GetVideoCodec(playbackService) == NEXUS_VideoCodec_eMpeg2)
            {
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
            }
            else
            {
                trickModeSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
            }
            NEXUS_Playback_TrickMode(playbackService->playback, &trickModeSettings);
        }
        break;
    case eB_DVR_OperationSpeed:
        {
            NEXUS_PlaybackTrickModeSettings trickModeSettings;
            NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
            trickModeSettings.maxDecoderRate = 4 * NEXUS_NORMAL_PLAY_SPEED;
            trickModeSettings.rate = operationSettings->operationSpeed;
            NEXUS_Playback_TrickMode(playbackService->playback, &trickModeSettings);
        }
        break;
    case eB_DVR_OperationReverseFrameAdvance:
        {
            NEXUS_PlaybackStatus playbackStatus;
            NEXUS_Playback_GetStatus(playbackService->playback, &playbackStatus);
            if(playbackStatus.state == NEXUS_PlaybackState_ePaused)
            {
                NEXUS_Playback_FrameAdvance(playbackService->playback,false);
            }
            else
            {
                BDBG_WRN(("eB_DVR_OperationReverseFrameAdvance invalid"));
            }
        }
        break;
    case eB_DVR_OperationForwardFrameAdvance:
        {
            NEXUS_PlaybackStatus playbackStatus;
            BDBG_MSG(("eB_DVR_OperationForwardFrameAdvance"));
            NEXUS_Playback_GetStatus(playbackService->playback, &playbackStatus);
            if(playbackStatus.state == NEXUS_PlaybackState_ePaused)
            {
                NEXUS_Playback_FrameAdvance(playbackService->playback,true);
            }
            else
            {
                BDBG_WRN(("eB_DVR_OperationForwardFrameAdvance invalid"));
            }
        }
        break;
    case eB_DVR_OperationSeek:
        {
            if(operationSettings->seekTime < playbackService->mediaNode->mediaStartTime && 
               operationSettings->seekTime > playbackService->mediaNode->mediaEndTime)
            {
                BDBG_ERR(("Invalid time as it's out of bounds"));
            }
            else
            {
                if(playbackService->alarmTimer)
                {
                    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
                    B_Scheduler_CancelTimer(dvrManager->scheduler,playbackService->alarmTimer);
                    playbackService->alarmTimer = NULL;
                }
                NEXUS_Playback_Seek(playbackService->playback,operationSettings->seekTime);

            }
        }
        break;
    default:
        BDBG_WRN(("invalid operation %d",operationSettings->operation));
        break;
    }
    
    BDBG_MSG((" %s rc %d", __FUNCTION__,rc));
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_GetStatus(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceStatus *pStatus)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    NEXUS_PlaybackStatus status;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_ASSERT(pStatus);
    BDBG_MSG(("B_DVR_PlaybackService_GetStatus >>>"));
    B_Mutex_Lock(playbackService->playbackMutex);
    NEXUS_Playback_GetStatus(playbackService->playback,&status);
    pStatus->startTime = playbackService->mediaNode->mediaStartTime;
    pStatus->endTime =  playbackService->mediaNode->mediaEndTime;
    pStatus->currentTime = status.position;
    pStatus->linearEndOffset = playbackService->mediaNode->mediaLinearEndOffset;
    pStatus->linearStartOffset = playbackService->mediaNode->mediaLinearStartOffset;
    pStatus->state = status.state;

    if(playbackService->mediaNode->mediaEndTime > 0) 
    {
        pStatus->linearCurrentOffset = playbackService->mediaNode->mediaLinearEndOffset/(off_t)playbackService->mediaNode->mediaEndTime;
        pStatus->linearCurrentOffset *= pStatus->currentTime;
    }
    else
    {
        pStatus->linearCurrentOffset =0;
    }
    
    BDBG_MSG(("%lld:%lld %ld:%ld:%ld",
              playbackService->mediaNode->mediaLinearStartOffset,
              playbackService->mediaNode->mediaLinearEndOffset,
              playbackService->mediaNode->mediaStartTime,
              status.readPosition,
              playbackService->mediaNode->mediaEndTime));
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_GetStatus <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_GetSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceSettings *pSettings)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_ASSERT(pSettings);
    B_Mutex_Lock(playbackService->playbackMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&playbackService->playbackServiceSettings,sizeof(B_DVR_PlaybackServiceSettings));
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_SetSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_PlaybackServiceSettings *pSettings)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_ASSERT(pSettings);
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->playbackServiceSettings.audioDecoder[0] = pSettings->audioDecoder[0];
    playbackService->playbackServiceSettings.audioDecoder[1] = pSettings->audioDecoder[1];
    playbackService->playbackServiceSettings.videoDecoder[0] = pSettings->videoDecoder[0];
    playbackService->playbackServiceSettings.videoDecoder[1] = pSettings->videoDecoder[1];
    playbackService->playbackServiceSettings.stcChannel = pSettings->stcChannel;
    playbackService->playbackServiceSettings.simpleAudioDecoder[0] = pSettings->simpleAudioDecoder[0];
    playbackService->playbackServiceSettings.simpleAudioDecoder[1] = pSettings->simpleAudioDecoder[1];
    playbackService->playbackServiceSettings.simpleVideoDecoder[0] = pSettings->simpleVideoDecoder[0];
    playbackService->playbackServiceSettings.simpleVideoDecoder[1] = pSettings->simpleVideoDecoder[1];
    playbackService->playbackServiceSettings.simpleStcChannel = pSettings->simpleStcChannel;
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_InstallCallback(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->registeredCallback = registeredCallback;
    playbackService->appContext = appContext;
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_RemoveCallback(
    B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_ERROR rc= B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->registeredCallback = NULL;
    playbackService->appContext = NULL;
    B_Mutex_Unlock(playbackService->playbackMutex);
    return rc;
}

B_DVR_MediaNode B_DVR_PlaybackService_GetMediaNode(
    B_DVR_PlaybackServiceHandle playbackService
    )
{
     B_DVR_MediaNode mediaNode=NULL; 
    if(playbackService) 
    {
        BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
        B_Mutex_Lock(playbackService->playbackMutex);
        mediaNode = playbackService->mediaNode;    
        B_Mutex_Unlock(playbackService->playbackMutex);
    }
    return mediaNode;
}

B_DVR_ERROR B_DVR_PlaybackService_GetFileHandle(
    B_DVR_PlaybackServiceHandle playbackService,
    struct NEXUS_FilePlay *nexusFilePlay)
{

    B_DVR_ERROR rc = B_DVR_SUCCESS;
    if(playbackService) 
    {
        BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
        B_Mutex_Lock(playbackService->playbackMutex);
        nexusFilePlay = playbackService->nexusFilePlay1;
        B_Mutex_Unlock(playbackService->playbackMutex);
    }
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_SetAlarmTime(
    B_DVR_PlaybackServiceHandle playbackService,
    unsigned long alarmTime)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_PlaybackService_SetAlarmTime >>> alarmTime %ul",alarmTime));
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->alarmTime = alarmTime;
    if(playbackService->alarmTimer)
    {
        B_Scheduler_CancelTimer(dvrManager->scheduler,playbackService->alarmTimer);
        playbackService->alarmTimer = NULL;
    }
    NEXUS_Playback_GetStatus(playbackService->playback,&nexusPlaybackStatus);
    if(nexusPlaybackStatus.state != NEXUS_PlaybackState_eStopped || 
       nexusPlaybackStatus.state != NEXUS_PlaybackState_eAborted)
    {
    
        playbackService->alarmTimer = B_Scheduler_StartTimer(dvrManager->scheduler,
                                                             playbackService->schedulerMutex,
                                                             1000,B_DVR_PlaybackService_P_AlarmTimer,
                                                             (void*)playbackService);
    }
    else
    {
        BDBG_WRN(("Playback not started. So alarm will be scheduled after playback start"));
    }
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_SetAlarmTime <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_AddDrmSettings(
    B_DVR_PlaybackServiceHandle playbackService,
    B_DVR_DRMServiceHandle drmService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_PlaybackSettings playbackSettings;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_OBJECT_ASSERT(drmService,B_DVR_DRMService);
    BDBG_MSG(("B_DVR_PlaybackService_AddDrmSettings >>>"));
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->dma = drmService->dma;
    playbackService->keySlot = drmService->keySlot;
    BDBG_ASSERT(playbackService->playback);
    NEXUS_Playback_GetSettings(playbackService->playback, &playbackSettings);
    playbackSettings.playpumpSettings.securityDma = drmService->dma ;
    playbackSettings.playpumpSettings.securityDmaDataFormat= NEXUS_DmaDataFormat_eMpeg;
    playbackSettings.playpumpSettings.securityContext = drmService->keySlot ; 
    NEXUS_Playback_SetSettings(playbackService->playback, &playbackSettings);
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_AddDrmSettings <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_RemoveDrmSettings(
    B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_MSG(("B_DVR_PlaybackService_RemoveDrmSettings >>>"));
    B_Mutex_Lock(playbackService->playbackMutex);
    playbackService->dma = NULL;
    playbackService->keySlot = NULL;
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_RemoveDrmSettings <<<"));
    return rc;
}

B_DVR_ERROR B_DVR_PlaybackService_GetIFrameTimeStamp(
    B_DVR_PlaybackServiceHandle playbackService, 
    unsigned long *timeStamp)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_FilePosition position,iFramePosition;
    NEXUS_PlaybackStatus nexusPlaybackStatus;
    B_DVR_FileReadDirection direction = eB_DVR_FileReadDirectionForward;
    unsigned sizeOfFrame=0;
    BDBG_OBJECT_ASSERT(playbackService,B_DVR_PlaybackService);
    BDBG_MSG(("B_DVR_PlaybackService_GetIFrameTimeStamp >>>"));
    B_Mutex_Lock(playbackService->playbackMutex);

    if(*timeStamp > playbackService->mediaNode->mediaEndTime
       || *timeStamp < playbackService->mediaNode->mediaStartTime) 
    {
        rc = B_DVR_INVALID_PARAMETER;
        BDBG_ERR(("%s:%s Invalid timeStamp startTime %u endTime %u timeStamp %u",
                  __FUNCTION__,playbackService->mediaNode->programName,
                  playbackService->mediaNode->mediaStartTime,
                  playbackService->mediaNode->mediaEndTime,timeStamp));
        goto error;
    }

    rc = B_DVR_SegmentedFilePlay_GetLocation(playbackService->nexusFilePlay1,
                                             -1,*timeStamp,&position);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:%s Unable to get position info for timeStamp:%u",
                  __FUNCTION__,playbackService->mediaNode->programName,
                  timeStamp)); 
        goto error;
    }

    rc = NEXUS_Playback_GetStatus(playbackService->playback,&nexusPlaybackStatus);
    if(rc!=B_DVR_SUCCESS) 
    {
        BDBG_ERR(("%s:%s Unable to get tsb playback Status",
                  __FUNCTION__,playbackService->mediaNode->programName)); 
        goto error;
    }

    if(nexusPlaybackStatus.position > *timeStamp)
    {
        direction = eB_DVR_FileReadDirectionReverse;
    }
    rc = B_DVR_SegmentedFilePlay_GetNextIFrame(playbackService->nexusFilePlay1, 
                                               position,&iFramePosition,
                                               &sizeOfFrame,
                                               direction); 
    if(rc!=B_DVR_SUCCESS) 
    {
         BDBG_ERR(("%s:%s Unable to get I Frame location direction:%u timeStamp:%ld",
                   __FUNCTION__,playbackService->mediaNode->programName,direction,
                   position.timestamp)); 
         goto error;

    }

    BDBG_MSG(("%s:%s iFrame Info",__FUNCTION__,playbackService->mediaNode->programName));
    BDBG_MSG(("timeStamp %u index %u mediaLinearOffset %lld navLinearOffset %lld",
              iFramePosition.timestamp,iFramePosition.index,iFramePosition.mpegFileOffset, 
              iFramePosition.navFileOffset));
    *timeStamp = iFramePosition.timestamp;
error:
    B_Mutex_Unlock(playbackService->playbackMutex);
    BDBG_MSG(("B_DVR_PlaybackService_GetIFrameTimeStamp <<<"));
    return rc;
}
