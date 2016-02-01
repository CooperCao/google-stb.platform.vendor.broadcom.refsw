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
 * record:
 * App demonstrates background recording in segmented file format.
 * App requires a QAM source and also the storage device
 * should to be formatted in segmented file system format using
 * mksfs app. Back ground recordings typically don't have
 * any A/V output attached. Back ground recordings [completed or inprogress]
 * can be played back through playback service from the program list.
 *
 * usage:
 * ./nexus record
 */

#include "util.h"

#define BG_REC_NAME "bgrec_0"   /* meta data name for a background permanent recording instance */
#define BG_REC_DIRECTORY "bgrec" /* meta data sub directory for a background permanent recording instance */
#define BG_REC_LEN 15*60*1000 /* 15 minutes recording. Units in ms.*/

/*
 *  Channel Info needs to be modified based on the
 *  user test environment.
 */
channelInfo_t channelInfo =
{
    NEXUS_FrontendQamAnnex_eB,
    NEXUS_FrontendQamMode_e64,
    5056941,
    429000000,
    0x11,0x14,0x11,
    NEXUS_VideoCodec_eMpeg2,
    NEXUS_AudioCodec_eAc3
};

typedef struct rec_ctx_t
{
    NEXUS_PlatformConfiguration platformconfig;
    NEXUS_ParserBand parserBand;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle lockChangedEvent;
    B_DVR_RecordServiceHandle recordService;
    B_DVR_ServiceCallback serviceCallback;
    B_EventHandle recStarted;
    psiInfo_t psiInfo;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_MutexHandle schedulerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle schedulerThread;
}rec_ctx_t;

rec_ctx_t *rec_ctx = NULL;
B_DVR_Media media;
char recName[B_DVR_MAX_FILE_NAME_LENGTH]=BG_REC_NAME;
char recSubDir[B_DVR_MAX_FILE_NAME_LENGTH]=BG_REC_DIRECTORY;

void print_recording_info(rec_ctx_t *rec_ctx)
{
    B_DVR_MediaNodeSettings programInfo;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    programInfo.programName = recName;
    programInfo.subDir = recSubDir;
    programInfo.volumeIndex = STORAGE_VOLUME_INDEX;
    rc = B_DVR_Manager_GetMediaNode(rec_ctx->manager,&programInfo,&media);
    assert(rc == 0);
    app_print_rec_metadata(&media);
    return;
}
void rec_scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(rec_ctx->scheduler);
    return;
}

void rec_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    rec_ctx_t *rec_ctx = (rec_ctx_t *)context;
    BSTD_UNUSED(index);
    BSTD_UNUSED(service);
    switch(event)
    {
        case eB_DVR_EventDataInjectionCompleted:
        {
           rec_ctx->psiInfo.timer = B_Scheduler_StartTimer(rec_ctx->scheduler,
                                                                rec_ctx->schedulerMutex,
                                                                INJECT_INTERVAL,
                                                                app_inject_psi,
                                                                (void*)&rec_ctx->psiInfo);
        }
        break;
    case eB_DVR_EventStartOfRecording:
        {
            B_Event_Set(rec_ctx->recStarted);
            printf("\n rec started");
        }
        break;
    case eB_DVR_EventEndOfRecording:
        {
            printf("\n rec stopped");
        }
        break;
    case eB_DVR_EventHDStreamRecording:
        {
            printf("\n rec source is HD");
            printf("\n press enter to quit \n");
        }
        break;
    case eB_DVR_EventSDStreamRecording:
        {
            printf("\n rec  source is SD");
            printf("press enter to quit \n");
        }
        break;
    case eB_DVR_EventMediaProbed:
        {
            printf("\n recording is probed and stream parameters are available");
            printf("\n press enter to quit \n");
        }
        break;
    case eB_DVR_EventOverFlow:
        {
           printf("\n record overflow");
        }
        break;
     default:
        printf("\n invalid event");
    }
    return;
}

int main(void)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_MediaNodeSettings mediaNodeSettings;
    B_DVR_RecordServiceInputEsStream inputEsStream;
    B_DVR_RecordServiceSettings recordServiceSettings;
    B_DVR_RecordServiceRequest recordServiceRequest;
    B_DVR_DataInjectionServiceOpenSettings injectOpenSettings;
    B_DVR_DataInjectionServiceSettings injectSettings;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    B_DVR_MediaStorageType mediaStorageType;
    NEXUS_PlatformSettings platformSettings;
    unsigned i=0;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax)
    {
        goto error;
    }

    /*
     * allocate the app context
     */
    rec_ctx = malloc(sizeof(rec_ctx_t));
    assert(rec_ctx);
    memset(rec_ctx,0,sizeof(rec_ctx_t));
    /*
     *  initialize the platform
     */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    for (i=0;i<NEXUS_MAX_PARSER_BANDS;i++)
    {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[i]= 50000000; /*50 Mbps*/
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    if(rc)
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(rec_ctx);
        goto error;
    }

    /*
     *  get platform configuration
     */
    NEXUS_Platform_GetConfiguration(&rec_ctx->platformconfig);

    /*
     * acquire the frontend with qam capability
     */
    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    rec_ctx->frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if(!rec_ctx->frontend)
    {
        printf("\n No QAM frontend found");
        printf("\n app works with QAM source only at present");
        printf("\n use a platform with QAM source\n");
        NEXUS_Platform_Uninit();
        free(rec_ctx);
        goto error;

    }

    /*
     * create an event to receive the locked event from frontend
     */
    BKNI_CreateEvent(&rec_ctx->lockChangedEvent);
    assert(rec_ctx->lockChangedEvent);

     /*
     * tune to a channel channel
     */
    rec_ctx->parserBand = app_tune_channel(rec_ctx->frontend,
                                           rec_ctx->lockChangedEvent,
                                           &channelInfo);

    if(rec_ctx->parserBand == NEXUS_ParserBand_eInvalid)
    {
        printf("\n check and change channelInfo parameters for tuning \n");
        BKNI_DestroyEvent(rec_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(rec_ctx);
        goto error;
    }
    /*
     * open media storage
     */
    mediaStorageOpenSettings.storageType = mediaStorageType;
    rec_ctx->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(rec_ctx->storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice)
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(rec_ctx->storage,&registerSettings,&volumeIndex);
    }

    /*
     * check if the media storage is in segmented file system format
     */
    B_DVR_MediaStorage_GetStatus(rec_ctx->storage,&storageStatus);
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
        B_DVR_MediaStorage_Close(rec_ctx->storage);
        BKNI_DestroyEvent(rec_ctx->lockChangedEvent);
        NEXUS_Platform_Uninit();
        free(rec_ctx);
        goto error;
    }
    /*
     *  mount media storage -> media, nav and metadata partitions
     */
    rc = B_DVR_MediaStorage_MountVolume(rec_ctx->storage, STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     *  initialize the DVR manager
     */
    rec_ctx->manager = B_DVR_Manager_Init(NULL);
    assert(rec_ctx->manager);

    /*
     *  create the list of recording if available
     */
    rc = B_DVR_Manager_CreateMediaNodeList(rec_ctx->manager,STORAGE_VOLUME_INDEX);
    assert(rc == 0);

    /*
     * create app scheduler mutex
     */
    rec_ctx->schedulerMutex = B_Mutex_Create(NULL);
    assert(rec_ctx->schedulerMutex);

    /*
     * create app scheduler
     */
    rec_ctx->scheduler = B_Scheduler_Create(NULL);
    assert(rec_ctx->scheduler);

    /*
     *  create the scheduler thread
     */
    rec_ctx->schedulerThread = B_Thread_Create("rec",rec_scheduler,rec_ctx, NULL);
    assert(rec_ctx->schedulerThread);

    /*
     *  check if the permanent recording already exisits. If yes, delete
     *  and let the conversion create a recording with the same name.
     *  This is to ensure that no 2 recordings have the same name.
     */
    memset((void *)&mediaNodeSettings,0,sizeof(mediaNodeSettings));
    mediaNodeSettings.programName = recName;
    mediaNodeSettings.subDir = recSubDir;
    mediaNodeSettings.volumeIndex = STORAGE_VOLUME_INDEX;
    rc = B_DVR_Manager_GetMediaNode(rec_ctx->manager,&mediaNodeSettings,&media);
    if (rc == B_DVR_SUCCESS)
    {
        rc = B_DVR_Manager_DeleteRecording(rec_ctx->manager,&mediaNodeSettings);
        assert(rc == 0);
    }

    rec_ctx->recStarted = B_Event_Create(NULL);
    assert(rec_ctx->recStarted);
    /*
     * populate recordService request parameters
     */
    memset((void *)&recordServiceRequest,0,sizeof(B_DVR_RecordServiceRequest));
    strcpy(recordServiceRequest.programName,recName);
    strcpy(recordServiceRequest.subDir,recSubDir);
    recordServiceRequest.input = eB_DVR_RecordServiceInputQam;
    recordServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    recordServiceRequest.volumeIndex = STORAGE_VOLUME_INDEX;
    recordServiceRequest.segmented = true;

    /*
     * open a record service
     */
    rec_ctx->recordService = B_DVR_RecordService_Open(&recordServiceRequest);
    assert(rec_ctx->recordService);

    /*
     * install a callback from Record service to receive asynchronous events
     * from a Record service instance
     */
    rec_ctx->serviceCallback = (B_DVR_ServiceCallback)rec_callback;
    B_DVR_RecordService_InstallCallback(rec_ctx->recordService,
                                        rec_ctx->serviceCallback,
                                        (void*)rec_ctx);


    /*
     * populate data injection settings
     */
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188;
    rec_ctx->psiInfo.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(rec_ctx->psiInfo.injectService);

    /*
     * install a callback to receive events from data injection service
     */
    B_DVR_DataInjectionService_InstallCallback(rec_ctx->psiInfo.injectService,
                                               rec_ctx->serviceCallback,
                                               (void*)rec_ctx);

    /*
     * allocate a PID channel for data injection.
     */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = (NEXUS_NUM_PID_CHANNELS-2);
    pidChannelSettings.enabled = false;
    rec_ctx->psiInfo.injectChannel = NEXUS_PidChannel_Open(rec_ctx->parserBand,
                                                           0x1ffe,
                                                           &pidChannelSettings);
    assert(rec_ctx->psiInfo.injectChannel);

    /*
     * associate a PID channel with data injection service
     * and also set the type of injection (transport packet or PAT/PMT data)
     */
    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = rec_ctx->psiInfo.injectChannel;
    B_DVR_DataInjectionService_SetSettings(rec_ctx->psiInfo.injectService,&injectSettings);

    /*
     * associate a data injection service instance and an inBand parser
     * to a record service instance
     */
    memset((void *)&recordServiceSettings,0,sizeof(recordServiceSettings));
    recordServiceSettings.dataInjectionService = rec_ctx->psiInfo.injectService;
    recordServiceSettings.parserBand = rec_ctx->parserBand;
    B_DVR_RecordService_SetSettings(rec_ctx->recordService,&recordServiceSettings);

    /*
     * add video es stream info to a Record service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = channelInfo.vCodec;
    inputEsStream.pidChannel = NULL;
    B_DVR_RecordService_AddInputEsStream(rec_ctx->recordService,&inputEsStream);

    /*
     * add audio es stream info to a Record service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = channelInfo.aCodec;
    B_DVR_RecordService_AddInputEsStream(rec_ctx->recordService,&inputEsStream);

    /*
     * add data stream info to a Record service instance
     */
    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = channelInfo.pcrPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeData;
    B_DVR_RecordService_AddInputEsStream(rec_ctx->recordService,&inputEsStream);

    /*
     * create PAT and PMT packets based on A/V and data PID info
     */
    app_create_psi(&rec_ctx->psiInfo,
                   channelInfo.pcrPid,
                   channelInfo.vPid,
                   channelInfo.aPid,
                   channelInfo.vCodec,
                   channelInfo.aCodec);

    /*
     * start back ground recording
     */
    rc = B_DVR_RecordService_Start(rec_ctx->recordService,NULL);
    assert(rc == 0);

    /*
     * start data injection
     */
    rec_ctx->psiInfo.recordService = rec_ctx->recordService;
    rec_ctx->psiInfo.psiStop = false;
    rc = B_DVR_RecordService_InjectDataStart(rec_ctx->recordService,
                                          (void *)&rec_ctx->psiInfo.pat[1],
                                          188);
    assert(rc == 0);
    rec_ctx->psiInfo.injectInProgress = true;

    /*
     *   wait for start of recording event .i.e. 1st I frame
     *   availability
     */
    B_Event_Wait(rec_ctx->recStarted, 1000);

    printf("\n press enter to quit");
    getchar();
    /*
     *  prevent data injection from kicking another
     *  packet injection
     */
    B_Mutex_Lock(rec_ctx->schedulerMutex);
    rec_ctx->psiInfo.psiStop = true;
    B_Mutex_Unlock(rec_ctx->schedulerMutex);

    /*
     * stop the data injection timer and service
     */
     if(rec_ctx->psiInfo.injectInProgress)
    {
        if(rec_ctx->psiInfo.timer)
        {
            B_Scheduler_CancelTimer(rec_ctx->scheduler,
                                    rec_ctx->psiInfo.timer);
        }
        B_DVR_RecordService_InjectDataStop(rec_ctx->recordService);
    }

    /*
     * stop record service
     */
    B_DVR_RecordService_Stop(rec_ctx->recordService);

    /*
     * close data injection PID channel
     */
    NEXUS_PidChannel_Close(rec_ctx->psiInfo.injectChannel);

    /*
     * close data injection service
     */
    B_DVR_DataInjectionService_RemoveCallback(rec_ctx->psiInfo.injectService);
    B_DVR_DataInjectionService_Close(rec_ctx->psiInfo.injectService);

    /*
     * close record service
     */
    B_DVR_RecordService_RemoveCallback(rec_ctx->recordService);
    B_DVR_RecordService_Close(rec_ctx->recordService);

    /*
     * print recorded program meta data
     */
    print_recording_info(rec_ctx);

    /*
     * destroy list of record programs if available
     */
    B_DVR_Manager_DestroyMediaNodeList(rec_ctx->manager,STORAGE_VOLUME_INDEX);

    /*
     * de-initialize the dvr manager
     */
    B_DVR_Manager_UnInit(rec_ctx->manager);

    /*
     * unmount the media storage
     */
    B_DVR_MediaStorage_UnmountVolume(rec_ctx->storage, STORAGE_VOLUME_INDEX);

    /*
     * close media storage
     */
    B_DVR_MediaStorage_Close(rec_ctx->storage);

    /*
     * stop record app scheduler
     */
    B_Scheduler_Stop(rec_ctx->scheduler);
    B_Scheduler_Destroy(rec_ctx->scheduler);
    B_Thread_Destroy(rec_ctx->schedulerThread);
    B_Mutex_Destroy(rec_ctx->schedulerMutex);
    BKNI_DestroyEvent(rec_ctx->lockChangedEvent);
    B_Event_Destroy(rec_ctx->recStarted);
    /*
     * Uninitialize the platform
     */
    NEXUS_Platform_Uninit();
    free(rec_ctx);
error:
    return 0;
}


