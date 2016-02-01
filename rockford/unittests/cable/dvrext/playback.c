/******************************************************************************
 *    (c)2013-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * *****************************************************************************/

/*
 *  playback :
 *  App demonstrates playback of segmented recording along with trick modes.
 *  Before running this application, run tsbConvert application that would
 *  create tsbConverted_0 recording which is used in this playback app.
 *  
 *  usage:
 *  ./nexus playback
 */

#include "util.h"

/*#define PERM_RECORDING_NAME "tsbConverted_0"*/
#define PERM_RECORDING_NAME "bgrec_0"
/*#define PERM_RECORDING_DIRECTORY "tsbConv"*/
#define PERM_RECORDING_DIRECTORY "bgrec"

typedef struct play_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    B_DVR_PlaybackServiceHandle playbackService;
    B_DVR_ServiceCallback serviceCallback;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    decode_ctx_t play_decode_ctx;
    B_MutexHandle mutex;
}play_ctx_t;

play_ctx_t *play_ctx = NULL;

void print_play_menu(void)
{
    printf("\n***********************************");
    printf("\n************Playback Operations*********");
    printf("\n***********************************\n");
    printf("\n\t  0. quit ");
    printf("\n\t  1. pause ");
    printf("\n\t  2. play ");
    printf("\n\t  3. rewind");
    printf("\n\t  4. fast-forward");
    printf("\n\t  5. slow motion forward");
    printf("\n\t  6. slow motion reverse");
    printf("\n\t  7. frame advance");
    printf("\n\t  8. frame reverse");
    printf("\n\t  9. forwardjump");
    printf("\n\t 10. reversejump");
    printf("\n************ ***********************\n");
    return;
}
void print_play_status(B_DVR_PlaybackServiceHandle playbackService)
{
    B_DVR_PlaybackServiceStatus playbackServiceStatus;
    B_DVR_PlaybackService_GetStatus(playbackService,&playbackServiceStatus);
    printf("\n Playback Status\n");
    printf("\n st:ct:et %lu ms:%lu ms:%lu ms \n",
               playbackServiceStatus.startTime,
               playbackServiceStatus.currentTime,
               playbackServiceStatus.endTime);
    return;
}

void play_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    play_ctx_t *play_ctx = (play_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    B_Mutex_Lock(play_ctx->mutex);
    switch(event)
    {
    case eB_DVR_EventStartOfPlayback:
        {
            B_DVR_OperationSettings operationSettings;
            B_DVR_PlaybackServiceStatus playbackServiceStatus;
            printf("\n BOF Playback \n");
            print_play_status(play_ctx->playbackService);
            B_DVR_PlaybackService_GetStatus(play_ctx->playbackService,&playbackServiceStatus);
            operationSettings.operation = eB_DVR_OperationSeek;
            operationSettings.seekTime = playbackServiceStatus.startTime;
            B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);
            operationSettings.operation = eB_DVR_OperationPlay;
            B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);
            print_play_menu();
        }
        break;
    case eB_DVR_EventEndOfPlayback:
        {
            printf("\n EOF Playback \n");
            print_play_status(play_ctx->playbackService);
            print_play_menu();
        }
        break;
    default:
        printf("\n invalid event \n");
    }
    B_Mutex_Unlock(play_ctx->mutex);
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    char programName[B_DVR_MAX_FILE_NAME_LENGTH]=PERM_RECORDING_NAME;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH]=PERM_RECORDING_DIRECTORY;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    B_DVR_PlaybackServiceRequest playbackServiceRequest;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_DisplaySettings displaySettings;
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    #endif
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PlatformSettings platformSettings;
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    } 
    B_Os_Init();
    BKNI_Init();

    /* 
     * allocate the app context
     */
    play_ctx = malloc(sizeof(play_ctx_t));
    assert(play_ctx);
    memset(play_ctx,0,sizeof(play_ctx_t));

    /* 
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    play_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(play_ctx->storage);
    
    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(play_ctx->storage,&registerSettings,&volumeIndex);
    }

    /* 
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(play_ctx->storage,&storageStatus);
    if(!storageStatus.numRegisteredVolumes || 
       !storageStatus.volumeInfo[STORAGE_VOLUME_INDEX].formatted) 
    {
        printf("\n no storage device registered and formatted");
        if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
        {
            printf("\n use mkvsfs app to register and format a storage device \n");
        }
        else
        {
            printf("\n use mksfs app to register and format a storage device \n");
        }
        B_DVR_MediaStorage_Close(play_ctx->storage);
        free(play_ctx);
        goto error;
    }
    /* 
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(play_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /* 
     *  initialize the DVR manager
     */
    play_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(play_ctx->manager);

    /* 
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(play_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    play_ctx->mutex = B_Mutex_Create(NULL);
    assert(play_ctx->mutex);

     NEXUS_Platform_GetDefaultSettings(&platformSettings);
     platformSettings.openFrontend = false;
    /* 
     *  initialized the platform
     */
    rc = NEXUS_Platform_Init(&platformSettings);
    assert(rc == 0);

    /* 
     *  get platform configuration
     */
    NEXUS_Platform_GetConfiguration(&play_ctx->platformconfig);

    /*
     *  open display
     */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    play_ctx->display = NEXUS_Display_Open(0, &displaySettings);
    assert(play_ctx->display);

    /*
     *  add hdmi output to the display and also set the hdmi settings.
     */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(play_ctx->display, 
                            NEXUS_HdmiOutput_GetVideoConnector(play_ctx->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(play_ctx->platformconfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = app_hotplug_callback;
    hdmiSettings.hotplugCallback.context = play_ctx->platformconfig.outputs.hdmi[0];
    hdmiSettings.hotplugCallback.param = (int)play_ctx->display;
    NEXUS_HdmiOutput_SetSettings(play_ctx->platformconfig.outputs.hdmi[0], &hdmiSettings);
    #endif

    /*
     *  open stc channel
     */
    NEXUS_StcChannel_GetDefaultSettings(0,&stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    play_ctx->play_decode_ctx.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    assert(play_ctx->play_decode_ctx.stcChannel);

    /*
     *  open video window
     */
    play_ctx->window = NEXUS_VideoWindow_Open(play_ctx->display,0);
    assert(play_ctx->window);

    /*
     *  open video decoder
     */
    play_ctx->play_decode_ctx.videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    assert(play_ctx->play_decode_ctx.videoDecoder);

    /*
     *  add video decoder as input to the video window
     */
    NEXUS_VideoWindow_AddInput(play_ctx->window,
                               NEXUS_VideoDecoder_GetConnector(play_ctx->play_decode_ctx.videoDecoder));


    /*
     *  open audio decoder
     */
    play_ctx->play_decode_ctx.audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    assert(play_ctx->play_decode_ctx.audioDecoder);

    /*
     *  add audio decoder as input to the HDMI audio output
     */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(play_ctx->platformconfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(play_ctx->play_decode_ctx.audioDecoder,
                                                               NEXUS_AudioDecoderConnectorType_eStereo));
    #endif

    /*
     *  get recorded program meta data
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = programName;
    mediaNodeSettings.subDir = subDir;
    mediaNodeSettings.volumeIndex=STORAGE_VOLUME_INDEX;
    B_DVR_Manager_GetMediaNode(play_ctx->manager,&mediaNodeSettings,&media);
    if(strcmp(mediaNodeSettings.programName,media.programName))
    {
        printf("\n program %s not found",programName);
        printf("\n run tsbconvert app to generate program: %s\n",programName);
        goto error_program;
    }

    /*
     *  populate playback service request
     */
    memset((void *)&playbackServiceRequest,0,sizeof(B_DVR_PlaybackServiceRequest));
    strcpy(playbackServiceRequest.programName,programName);
    strcpy(playbackServiceRequest.subDir,subDir);
    playbackServiceRequest.playpumpIndex = NEXUS_ANY_ID;
    playbackServiceRequest.volumeIndex = STORAGE_VOLUME_INDEX;

    /*
     *  open playback service
     */
    play_ctx->playbackService = B_DVR_PlaybackService_Open(&playbackServiceRequest);
    assert(play_ctx->playbackService);

    play_ctx->serviceCallback = (B_DVR_ServiceCallback)play_callback;
    B_DVR_PlaybackService_InstallCallback(play_ctx->playbackService,
                                          play_ctx->serviceCallback,
                                          (void*)play_ctx);

    /*
     *  pass A/V decoder handles and stc channel handle from the app
     *  to the playback service and also get playback handle
     *  from playback service
     */
    rc = B_DVR_PlaybackService_GetSettings(play_ctx->playbackService,&playbackServiceSettings);
    assert(rc == 0);
    playbackServiceSettings.audioDecoder[0] = play_ctx->play_decode_ctx.audioDecoder;
    playbackServiceSettings.videoDecoder[0] = play_ctx->play_decode_ctx.videoDecoder;
    playbackServiceSettings.stcChannel = play_ctx->play_decode_ctx.stcChannel;
    rc = B_DVR_PlaybackService_SetSettings(play_ctx->playbackService,&playbackServiceSettings);
    assert(rc == 0);

    /*
     *  open video PID channel 
     */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = play_ctx->play_decode_ctx.videoDecoder;

    /* Assuming index 0 for video */
    play_ctx->play_decode_ctx.vPidChannel = 
        NEXUS_Playback_OpenPidChannel(playbackServiceSettings.playback,
                                      media.esStreamInfo[0].pid,
                                      &playbackPidSettings);
    assert(play_ctx->play_decode_ctx.vPidChannel);

    /*
     *  open audio PID channel 
     */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = play_ctx->play_decode_ctx.audioDecoder;
    /* Assuming index 1 for audio */
    play_ctx->play_decode_ctx.aPidChannel = 
        NEXUS_Playback_OpenPidChannel(playbackServiceSettings.playback,
                                      media.esStreamInfo[1].pid,
                                      &playbackPidSettings);
    assert(play_ctx->play_decode_ctx.aPidChannel);

    /*
     *  populate video program settings for video decoder
     */
    play_ctx->play_decode_ctx.playback = true;
    NEXUS_VideoDecoder_GetDefaultStartSettings(&play_ctx->play_decode_ctx.videoProgram);
    play_ctx->play_decode_ctx.videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
    play_ctx->play_decode_ctx.videoProgram.pidChannel = play_ctx->play_decode_ctx.vPidChannel;
    play_ctx->play_decode_ctx.videoProgram.stcChannel = play_ctx->play_decode_ctx.stcChannel;

    /*
     *  populate audio program settings for audio decoder
     */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&play_ctx->play_decode_ctx.audioProgram);
    play_ctx->play_decode_ctx.audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
    play_ctx->play_decode_ctx.audioProgram.pidChannel = play_ctx->play_decode_ctx.aPidChannel;
    play_ctx->play_decode_ctx.audioProgram.stcChannel = play_ctx->play_decode_ctx.stcChannel;

    /*
     *  start A/V decoders
     */
    app_decode_start(&play_ctx->play_decode_ctx);

    /*
     *  start playback service
     */
    B_DVR_PlaybackService_Start(play_ctx->playbackService);

    /*
     *  various operations for playback service
     */
    print_play_menu();
    while(1)
    {   
        unsigned operation;
        B_DVR_OperationSettings operationSettings;
        printf("\n enter the operation => ");
        fflush(stdin);
        scanf("%u",&operation);
        print_play_status(play_ctx->playbackService);
        if(operation == eQuit) 
        {
            break;
        }
        print_play_menu();
        B_Mutex_Lock(play_ctx->mutex);
        switch(operation) 
        {
            case ePause:
            {
                operationSettings.operation = eB_DVR_OperationPause; 
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);   
            }
            break;
        case ePlay:
            {
                operationSettings.operation = eB_DVR_OperationPlay; 
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                
            }
            break;
        case eRewind:
            {
                operationSettings.operation = eB_DVR_OperationFastRewind;
                operationSettings.operationSpeed = 4;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
            }
            break;
        case eFastForward:
            {
                operationSettings.operation = eB_DVR_OperationFastForward;
                operationSettings.operationSpeed = 4;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
            }
            break;
        case eSlowMotionForward:
            {
                operationSettings.operation = eB_DVR_OperationSlowForward;
                operationSettings.operationSpeed = 4;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
            }
            break;
        case eSlowMotionReverse:
            {
                operationSettings.operation = eB_DVR_OperationSlowRewind;
                operationSettings.operationSpeed = 4;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
            }
            break;
        case eFrameAdvance:
            {
                operationSettings.operation = eB_DVR_OperationForwardFrameAdvance;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
            }
            break;
        case eFrameReverse:
            {
                operationSettings.operation = eB_DVR_OperationReverseFrameAdvance;
                B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  

            }
            break;
        case eForwardJump:
            {
                unsigned skip_interval=0;
                B_DVR_PlaybackServiceStatus playbackServiceStatus;
                B_Mutex_Unlock(play_ctx->mutex);
                printf("\n Enter the skip interval (ms):");
                fflush(stdin);
                scanf("%u",&skip_interval);
                B_Mutex_Lock(play_ctx->mutex);
                B_DVR_PlaybackService_GetStatus(play_ctx->playbackService,&playbackServiceStatus);
                playbackServiceStatus.currentTime += skip_interval;
                rc = B_DVR_PlaybackService_GetIFrameTimeStamp(play_ctx->playbackService,
                                                              &playbackServiceStatus.currentTime);
                if(rc == B_DVR_SUCCESS) 
                {
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime = playbackServiceStatus.currentTime;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                 }
                 else
                 {
                        printf("\n invalid skip interval \n");
                 }
            }
            break;
        case eReverseJump:
            {
                unsigned skip_interval=0;
                B_DVR_PlaybackServiceStatus playbackServiceStatus;
                B_Mutex_Unlock(play_ctx->mutex);
                printf("\n Enter the skip interval (ms):");
                fflush(stdin);
                scanf("%u",&skip_interval);
                B_Mutex_Lock(play_ctx->mutex);
                B_DVR_PlaybackService_GetStatus(play_ctx->playbackService,&playbackServiceStatus);
                playbackServiceStatus.currentTime -= skip_interval;
                rc = B_DVR_PlaybackService_GetIFrameTimeStamp(play_ctx->playbackService,
                                                              &playbackServiceStatus.currentTime);
                if(rc == B_DVR_SUCCESS) 
                {
                        operationSettings.operation = eB_DVR_OperationPause;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                        operationSettings.operation = eB_DVR_OperationSeek;
                        operationSettings.seekTime = playbackServiceStatus.currentTime;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                        operationSettings.operation = eB_DVR_OperationPlay;
                        B_DVR_PlaybackService_SetOperation(play_ctx->playbackService,&operationSettings);  
                 }
                 else
                 {
                        printf("\n invalid skip interval \n");
                 }
            }
            break;
        default:
            printf("\n invalid playback operation \n");
        }
        B_Mutex_Unlock(play_ctx->mutex);
    }

    /*
     *  stop playback service
     */
    B_DVR_PlaybackService_Stop(play_ctx->playbackService);

    /*
     *  stop A/V decoders
     */
    app_decode_stop(&play_ctx->play_decode_ctx);

    /*
     *  close A/V PID channels
     */
    NEXUS_Playback_ClosePidChannel(playbackServiceSettings.playback,
                                   play_ctx->play_decode_ctx.vPidChannel);
    NEXUS_Playback_ClosePidChannel(playbackServiceSettings.playback,
                                   play_ctx->play_decode_ctx.aPidChannel);

    /*
     *  close playback service
     */
    B_DVR_PlaybackService_RemoveCallback(play_ctx->playbackService);
    B_DVR_PlaybackService_Close(play_ctx->playbackService);

error_program:
    /*
     *  remove audio decoder from HDMI audio output
     */
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(play_ctx->platformconfig.outputs.hdmi[0]));
    #endif

    /*
     *  remove video decoder input from video window
     */
    NEXUS_VideoWindow_RemoveAllInputs(play_ctx->window);

    /*
     *  close video window
     */
    NEXUS_VideoWindow_Close(play_ctx->window);

    /*
     *  close stc channel
     */
    NEXUS_StcChannel_Close(play_ctx->play_decode_ctx.stcChannel);

    /*
     *  close A/V decoders
     */
    NEXUS_VideoDecoder_Close(play_ctx->play_decode_ctx.videoDecoder);
    NEXUS_AudioDecoder_Close(play_ctx->play_decode_ctx.audioDecoder);

    /*
     *  remove video windows from display
     */
    NEXUS_Display_RemoveAllOutputs(play_ctx->display);

    /*
     *  close display
     */
    NEXUS_Display_Close(play_ctx->display);

     /* 
     * Uninitialize the platform
     */
     NEXUS_Platform_Uninit();
     
    /*
     *  destroy the list of recorded programs if available
     */
    B_DVR_Manager_DestroyMediaNodeList(play_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     *  de-initialized dvr manager
     */
    B_DVR_Manager_UnInit(play_ctx->manager);

    /*
     *  unmount media storage
     */
    B_DVR_MediaStorage_UnmountVolume(play_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     *  close media storage
     */
    B_DVR_MediaStorage_Close(play_ctx->storage);
    B_Os_Uninit();
    BKNI_Uninit();
    free(play_ctx);
error:
    return 0;
}


