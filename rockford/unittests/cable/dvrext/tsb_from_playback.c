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
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#if NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include "nexus_timebase.h"
#include "nexus_base_types.h"
#include "nexus_display.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_types.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"


#include "b_dvr_manager.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_datainjectionservice.h"
#include "tshdrbuilder.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <assert.h>

/*
 * tsb_from_playback simulates the tsb buffering for an IP source by 
 * recording the linear video streams from playback. 
 * The tsb is non-linear segmented file format.
 */
BDBG_MODULE(tsb_from_playback);

#define MAX_IP_SOURCE 4
#define MAX_STREAM_PATH 8
#define MAX_TSB_BUFFERS 8
#define MAX_TSB_TIME 5*60*1000 /* 5 minutes in milliseconds */
#define INJECT_INTERVAL 50
#define BTST_TS_HEADER_BUF_LENGTH   189
#define BTST_MUX_PMT_PID        (0x55)
#define BTST_MUX_PAT_PID        (0x0)

#define MEDIA_DEVICE_MOUNT_PATH "/data" /* this should be already available on the existing file system */
#define VSFS_DEVICE "/data/vsfs" /* This would be created by media storage */
#define VSFS_SIZE 50*1024 /* units = MB */
#define VOLUME_INDEX 0

#define STORAGE_DEVICE "/dev/sda"
/* command to zap the storage */
#define STORAGE_ZAP_COMMAND "sgdisk -Z /dev/sda"
#define STORAGE_PRINT_PARTITION_TABLE "sgdisk -p /dev/sda"


typedef struct ipSource_t
{
    char stream[256];
    unsigned vPid;
    unsigned aPid;
    NEXUS_VideoCodec vCodec;
    NEXUS_AudioCodec aCodec;
}ipSource_t;

ipSource_t ipSource[MAX_IP_SOURCE] = 
{
    { "/mnt/nfs/streams/trp_506_hockey_1080i_q256.mpg", 0x11, 0x14, NEXUS_VideoCodec_eMpeg2,NEXUS_AudioCodec_eAc3},
    { "stream1.ts", 0x11, 0x14, NEXUS_VideoCodec_eMpeg2,NEXUS_AudioCodec_eAc3 },
    { "stream1.ts", 0x11, 0x14, NEXUS_VideoCodec_eMpeg2,NEXUS_AudioCodec_eAc3 },
    { "stream1.ts", 0x11, 0x14, NEXUS_VideoCodec_eMpeg2,NEXUS_AudioCodec_eAc3 }
};
typedef struct streamPath_t
{ 
    unsigned index;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_FilePlayHandle filePlay;
    NEXUS_PidChannelHandle aPidChannel;
    NEXUS_PidChannelHandle vPidChannel;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_StcChannelHandle stcChannel;
    B_DVR_TSBServiceHandle tsbService;
    NEXUS_PidChannelHandle aTsbPlayPidChannel;
    NEXUS_PidChannelHandle vTsbPlayPidChannel;
    B_DVR_DataInjectionServiceHandle injectService;
    NEXUS_PidChannelHandle injectChannel;
    B_SchedulerTimerId injectTimer;
    B_DVR_ServiceCallback serviceCallback;
    uint8_t pat[BTST_TS_HEADER_BUF_LENGTH];
    uint8_t pmt[BTST_TS_HEADER_BUF_LENGTH];
    unsigned injectCount;
    unsigned patCcValue;
    unsigned pmtCcValue;
    bool injectInProgress;
    bool conversionInProgress;
    bool sourceDecodeStarted;
    bool tsbPlayDecodeStarted;
    B_MutexHandle mutex;
}streamPath_t;

typedef struct appContext_t
{
    NEXUS_PlatformConfiguration platformconfig;
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;
    #endif
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    streamPath_t streamPath;
    B_DVR_ManagerHandle manager;
    B_DVR_MediaStorageHandle storage;
    B_MutexHandle sMutex;
    B_SchedulerHandle s;
    B_ThreadHandle sThread;
}appContext_t;

typedef enum app_operation
{   
    operation_equit,
    operation_etsb_play_start,
    operation_etsb_play_stop,
    operation_etsb_play_pause,
    operation_etsb_play_unpause,
    operation_etsb_play_ff,
    operation_etsb_play_rew,
    operation_etsb_conv_start,
    operation_etsb_conv_stop,
    operation_etsb_max
}app_operation;

appContext_t *appContext = NULL;

B_DVR_MediaStorageType get_storage_device_type(void)
{
    B_DVR_MediaStorageType mediaStorageType;
    char *storageType=NULL;

    storageType = getenv("storage_type");
    if(!storageType) 
    {
        printf("\n export storage_type=loop or export storage_type=block");
        printf("\n no storage device type chosen");
        mediaStorageType = eB_DVR_MediaStorageTypeMax;
    }
    else
    {
        if(!strcmp(storageType,"block")) 
        {
            printf("\n DVR storage type is block device");
            printf("\n DVR is using SFS file system");
            mediaStorageType = eB_DVR_MediaStorageTypeBlockDevice;
            system("umount /dev/sda1");
            system("umount /dev/sda2");
            system("umount /dev/sda3"); 
        }
        else
        {
            if(!strcmp(storageType,"loop")) 
            {
                printf("\n DVR storage type is block device");
                printf("\n DVR is using SFS file system");
                mediaStorageType = eB_DVR_MediaStorageTypeLoopDevice;
                system("umount /dev/loop0");
                system("umount /dev/loop1");
                system("umount /dev/loop2");
            }
            else
            {
                printf("\n export storage_type=loop or export storage_type=block");
                printf("\n invalid storage device type chosen");
                mediaStorageType = eB_DVR_MediaStorageTypeMax;
            }
        }
    }

    return mediaStorageType;
}

void scheduler(void * param)
{
    BSTD_UNUSED(param);
    B_Scheduler_Run(appContext->s);
    return;
}
void app_menu(void)
{
    printf("\n *****************************************************");
    printf("\n ******** tsb_from_playback options ******************");
    printf("\n *****************************************************");
    printf("\n 0 - quit");
    printf("\n 1 - tsb playback start");
    printf("\n 2 - tsb playback stop");
    printf("\n 3 - tsb playback pause");
    printf("\n 4 - tsb playback pause");
    printf("\n 5 - tsb playback fast forward");
    printf("\n 6 - tsb playback rewind");
    printf("\n 7 - tsb convert start");
    printf("\n 8 - tsb convert stop");
    printf("\n *****************************************************");
    return;
}

void app_create_patpmt(
    streamPath_t *streamPath,
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
    
    printf("\n %s >>>",__FUNCTION__);
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
            case NEXUS_AudioCodec_eAac    :      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlus:      audStreamType = 0x11; break;/* LOAS */
            /* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0x11; break;
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
    TS_buildTSHeader(&pidInfo, &pidState,streamPath->pat, BTST_TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)streamPath->pat + buf_used, pat_pl_buf, pat_pl_size);
    printf("pmt_pl_size %u buf_used %u",pat_pl_size, buf_used);
    TS_PID_info_Init(&pidInfo, BTST_MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState,streamPath->pmt, BTST_TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)streamPath->pmt + buf_used, pmt_pl_buf, pmt_pl_size);
    printf("pmt_pl_size %u buf_used %u",pmt_pl_size,buf_used);
    printf("\n \n PAT Dump\n[");
    for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
    {
        printf("%02x ",streamPath->pat[index+1]);
        if ((index % 16) == 15)
        {
            printf("]\n[");
        }
    }
    printf("\n\n");
    BKNI_Printf("PMT Dump\n[");
    for (index=0;index<BTST_TS_HEADER_BUF_LENGTH;index++) 
    {
        printf("%02x ",streamPath->pmt[index+1]);
        if ((index % 16) == 15)
        {
            printf("]\n[");
        }
    }
    printf("\n\n");
    printf("\n %s <<<",__FUNCTION__);
    return;
}

void tsb_injectdata(void *context)
{
    uint8_t ccByte=0;
    streamPath_t *streamPath = (streamPath_t *)context;
    B_Mutex_Lock(streamPath->mutex);
    B_DVR_TSBService_InjectDataStop(streamPath->tsbService);
    streamPath->injectInProgress = false;
    streamPath->injectCount++;
    if(streamPath->injectCount%2 == 0)
    {
        ++streamPath->patCcValue;
        ccByte = streamPath->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
        ccByte = (ccByte & 0xf0) | (streamPath->patCcValue & 0xf);
        streamPath->pat[4] = ccByte;
        B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pat[1]),188);
    }
    else
    {
        ++streamPath->pmtCcValue;
        ccByte = streamPath->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
        ccByte = (ccByte & 0xf0) | (streamPath->pmtCcValue & 0xf);
        streamPath->pmt[4] = ccByte;
        B_DVR_TSBService_InjectDataStart(streamPath->tsbService,((void *)&streamPath->pmt[1]),188);
    }
    streamPath->injectInProgress = true;
    B_Mutex_Unlock(streamPath->mutex);
    return;
}


void service_callback(void *context, int index,B_DVR_Event event,B_DVR_Service service)
{
    streamPath_t *streamPath = (streamPath_t *)context;
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
    B_Mutex_Lock(streamPath->mutex);
    if(event != eB_DVR_EventDataInjectionCompleted) 
    {
        printf("\n service_callback >>>> index %u event %s service %s",(unsigned)index,eventString[event],serviceString[service]);
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
                printf("\n start:%ld current:%ld end:%ld",status.tsbRecStartTime,status.tsbCurrentPlayTime,status.tsbRecEndTime);
                operationSettings.operation = eB_DVR_OperationSeek;
                operationSettings.seekTime= status.tsbRecStartTime+2000;
                B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
                operationSettings.operation = eB_DVR_OperationPlay;
                B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
            }
             
        }
        break;
    case eB_DVR_EventDataInjectionCompleted:
        {
          
            streamPath->injectTimer = B_Scheduler_StartTimer(appContext->s,
                                                             appContext->sMutex,
                                                             INJECT_INTERVAL,
                                                             tsb_injectdata,
                                                             (void*)streamPath);
        }
        break;
    case eB_DVR_EventMediaProbed:
        {
            if(streamPath->tsbService)
            {
                printf("\n TSB Service media probe complete");
            }
        }
        break;
    default:
        printf("\n invalid event");
    }

    if(event != eB_DVR_EventDataInjectionCompleted) 
    {
        printf("\n service_callback <<<<<");
    }

    B_Mutex_Unlock(streamPath->mutex);
    return;
}
#if NEXUS_NUM_HDMI_OUTPUTS
void app_hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    printf("\n %s >>>",__FUNCTION__);
    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    printf( "\n HDMI hotplug event: %s", status.connected?"connected":"not connected");
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            printf("\n Current format not supported by attached monitor. "
                   "Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
    printf("\n %s <<<",__FUNCTION__);
    return;
}
#endif

B_DVR_ERROR app_decode_start(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    printf("\n %s >>>",__FUNCTION__);
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = ipSource[0].vCodec;
    videoProgram.pidChannel = streamPath->vPidChannel;
    videoProgram.stcChannel = streamPath->stcChannel;
    NEXUS_StcChannel_GetSettings(streamPath->stcChannel,&stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
    NEXUS_StcChannel_SetSettings(streamPath->stcChannel,&stcSettings);
    videoProgram.nonRealTime = false;
    NEXUS_VideoDecoder_Start(streamPath->videoDecoder, &videoProgram);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = ipSource[0].aCodec;
    audioProgram.pidChannel = streamPath->aPidChannel;
    audioProgram.stcChannel = streamPath->stcChannel;
    audioProgram.nonRealTime = false;
    NEXUS_AudioDecoder_Start(streamPath->audioDecoder,&audioProgram);
    streamPath->sourceDecodeStarted=true;
    printf("\n %s <<<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_decode_stop(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    printf("\n %s >>>",__FUNCTION__);
    NEXUS_VideoDecoder_GetSettings(streamPath->videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(streamPath->videoDecoder,&videoDecoderSettings);
    NEXUS_AudioDecoder_Stop(streamPath->audioDecoder);
    NEXUS_VideoDecoder_Stop(streamPath->videoDecoder);
    streamPath->sourceDecodeStarted = false;
    printf("\n %s <<<",__FUNCTION__);
    return rc;

}
B_DVR_ERROR app_tsb_decode_start(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_Media media;
    B_DVR_MediaNodeSettings mNodeSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    NEXUS_PlaybackPidChannelSettings pidChannelSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
    printf("\n %s >>>",__FUNCTION__);
    B_DVR_TSBService_GetSettings(streamPath->tsbService,&tsbServiceSettings);
    tsbServiceSettings.tsbPlaybackSettings.videoDecoder[0]= streamPath->videoDecoder;
    tsbServiceSettings.tsbPlaybackSettings.videoDecoder[1]=NULL;
    tsbServiceSettings.tsbPlaybackSettings.audioDecoder[0]=streamPath->audioDecoder;
    tsbServiceSettings.tsbPlaybackSettings.audioDecoder[1]=NULL;
    tsbServiceSettings.tsbPlaybackSettings.stcChannel = streamPath->stcChannel;
    B_DVR_TSBService_SetSettings(streamPath->tsbService,&tsbServiceSettings);

    mNodeSettings.subDir = "tsb";
    mNodeSettings.programName = "tsb0";
    mNodeSettings.volumeIndex = VOLUME_INDEX;
    rc = B_DVR_Manager_GetMediaNode(appContext->manager,&mNodeSettings,&media);
    assert(rc == 0);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    pidChannelSettings.pidTypeSettings.video.codec = media.esStreamInfo[0].codec.videoCodec;
    pidChannelSettings.pidTypeSettings.video.index = true;
    pidChannelSettings.pidTypeSettings.video.decoder = streamPath->videoDecoder;
    streamPath->vTsbPlayPidChannel =
         NEXUS_Playback_OpenPidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                       media.esStreamInfo[0].pid,
                                       &pidChannelSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidChannelSettings.pidTypeSettings.audio.primary = streamPath->audioDecoder;
    streamPath->aTsbPlayPidChannel = 
        NEXUS_Playback_OpenPidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                      media.esStreamInfo[1].pid,
                                      &pidChannelSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = media.esStreamInfo[0].codec.videoCodec;
    videoProgram.pidChannel = streamPath->vTsbPlayPidChannel;
    videoProgram.stcChannel = streamPath->stcChannel;
    NEXUS_StcChannel_GetSettings(streamPath->stcChannel,&stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
    NEXUS_StcChannel_SetSettings(streamPath->stcChannel,&stcSettings);
    videoProgram.nonRealTime = false;
    NEXUS_VideoDecoder_Start(streamPath->videoDecoder, &videoProgram);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = media.esStreamInfo[1].codec.audioCodec;
    audioProgram.pidChannel = streamPath->aTsbPlayPidChannel;
    audioProgram.stcChannel = streamPath->stcChannel;
    audioProgram.nonRealTime = false;
    NEXUS_AudioDecoder_Start(streamPath->audioDecoder,&audioProgram);
    streamPath->tsbPlayDecodeStarted = true;
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_decode_stop(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    printf("\n %s >>>",__FUNCTION__);
    NEXUS_VideoDecoder_GetSettings(streamPath->videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(streamPath->videoDecoder,&videoDecoderSettings);
    NEXUS_AudioDecoder_Stop(streamPath->audioDecoder);
    NEXUS_VideoDecoder_Stop(streamPath->videoDecoder);
    B_DVR_TSBService_GetSettings(streamPath->tsbService,&tsbServiceSettings);
    NEXUS_Playback_ClosePidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                   streamPath->aTsbPlayPidChannel);
    NEXUS_Playback_ClosePidChannel(tsbServiceSettings.tsbPlaybackSettings.playback,
                                   streamPath->vTsbPlayPidChannel);
    streamPath->tsbPlayDecodeStarted = false;
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}
B_DVR_ERROR app_tsb_convert_start(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_TSBServiceStatus tsbStatus;
    B_DVR_TSBServicePermanentRecordingRequest permRecReq;
    printf("\n %s >>>",__FUNCTION__);
    B_DVR_TSBService_GetStatus(streamPath->tsbService, &tsbStatus);
    printf("\n current tsb Rec start %ld ms end %ld ms",tsbStatus.tsbRecStartTime, tsbStatus.tsbRecEndTime);
    fflush(stdin);
    printf("\n enter start time:");
    scanf("%lu",&permRecReq.recStartTime);
    permRecReq.recEndTime = permRecReq.recStartTime + 1800000;
    sprintf(permRecReq.programName,"%s_%ld","tsbConv",permRecReq.recStartTime);
    sprintf(permRecReq.subDir,"tsbConv");
    printf("\n \t program name %s \t subDir %s start %lu \t end %lu",
           permRecReq.programName,
           permRecReq.subDir,
           permRecReq.recStartTime,
           permRecReq.recEndTime);
    rc = B_DVR_TSBService_ConvertStart(streamPath->tsbService,&permRecReq);
    assert(rc == 0);
    streamPath->conversionInProgress = true;
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_convert_stop(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    printf("\n %s >>>",__FUNCTION__);
    rc = B_DVR_TSBService_ConvertStop(streamPath->tsbService);
    assert(rc == 0);
    streamPath->conversionInProgress = false;
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_start(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    B_DVR_TSBServiceStatus tsbServiceStatus;
    printf("\n %s >>>",__FUNCTION__);
    B_DVR_TSBService_GetStatus(streamPath->tsbService,&tsbServiceStatus);
    operationSettings.operation = eB_DVR_OperationTSBPlayStart;
    operationSettings.seekTime = tsbServiceStatus.tsbRecEndTime;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_stop(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    printf("\n %s >>>",__FUNCTION__);
    operationSettings.operation = eB_DVR_OperationTSBPlayStop;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_pause(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    printf("\n %s >>>",__FUNCTION__);
    operationSettings.operation = eB_DVR_OperationPause;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_unpause(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    printf("\n %s >>>",__FUNCTION__);
    operationSettings.operation = eB_DVR_OperationPlay;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_ff(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    printf("\n %s >>>",__FUNCTION__);
    operationSettings.operation = eB_DVR_OperationFastForward;
    operationSettings.operationSpeed = 4;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

B_DVR_ERROR app_tsb_play_rew(streamPath_t *streamPath)
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    B_DVR_OperationSettings operationSettings;
    printf("\n %s >>>",__FUNCTION__);
    operationSettings.operation = eB_DVR_OperationFastRewind;
    operationSettings.operationSpeed = 4;
    B_DVR_TSBService_SetOperation(streamPath->tsbService,&operationSettings);
    printf("\n %s <<<",__FUNCTION__);
    return rc;
}

int main(void) 
{
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    NEXUS_PlatformSettings platformSettings;
    B_DVR_MediaStorageOpenSettings mediaStorageOpenSettings;
    B_DVR_MediaStorageStatus storageStatus;
    B_DVR_MediaNodeSettings mNodeSettings;
    B_DVR_TSBServiceInputEsStream inputEsStream;
    B_DVR_TSBServiceSettings tsbServiceSettings;
    B_DVR_TSBServiceRequest tsbServiceRequest;
    NEXUS_PlaybackSettings playbackSettings;
    B_DVR_DataInjectionServiceOpenSettings injectOpenSettings;
    B_DVR_DataInjectionServiceSettings injectSettings;
    NEXUS_PlaybackPidChannelSettings pidChannelSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    unsigned operation;
    B_DVR_MediaStorageType mediaStorageType;

    mediaStorageType = get_storage_device_type();
    if(mediaStorageType == eB_DVR_MediaStorageTypeMax) 
    {
        goto error;
    }
    
    appContext = malloc(sizeof(appContext_t));
    assert(appContext);
    memset(appContext,0,sizeof(appContext_t));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if(rc) 
    {
        printf("\n NEXUS_Platform_Init failed \n ");
        free(appContext);
        goto error;
    }

    appContext->streamPath.filePlay = NEXUS_FilePlay_OpenPosix(ipSource[0].stream, NULL);
    if(!appContext->streamPath.filePlay) 
    {
        printf("\n playback source stream %s not available",ipSource[0].stream);
        printf("\n modify ipSource[0].stream");
        NEXUS_Platform_Uninit();
        free(appContext);
        goto error;
    }

    mediaStorageOpenSettings.storageType = mediaStorageType;
    appContext->storage = B_DVR_MediaStorage_Open(&mediaStorageOpenSettings);
    assert(appContext->storage);

    if(mediaStorageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        unsigned volumeIndex=0;
        B_DVR_MediaStorageRegisterVolumeSettings registerSettings;
        memset(&registerSettings,0,sizeof(registerSettings));
        sprintf(registerSettings.device,VSFS_DEVICE);
        registerSettings.startSec = 0;
        registerSettings.length = VSFS_SIZE;
        B_DVR_MediaStorage_RegisterVolume(appContext->storage,&registerSettings,&volumeIndex);
    }

    B_DVR_MediaStorage_GetStatus(appContext->storage,&storageStatus);
    if(!storageStatus.numRegisteredVolumes || 
       !storageStatus.volumeInfo[VOLUME_INDEX].formatted) 
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
        B_DVR_MediaStorage_Close(appContext->storage);
        NEXUS_FilePlay_Close(appContext->streamPath.filePlay);
        NEXUS_Platform_Uninit();
        free(appContext);
        goto error;
    }
    rc = B_DVR_MediaStorage_MountVolume(appContext->storage,VOLUME_INDEX);
    assert(rc == 0);

    appContext->manager = B_DVR_Manager_Init(NULL);
    assert(appContext->manager);

    rc = B_DVR_Manager_CreateMediaNodeList(appContext->manager,VOLUME_INDEX);
    assert(rc == 0);

    memset((void *)&mNodeSettings,0,sizeof(mNodeSettings));
    mNodeSettings.programName = "tsb0";
    mNodeSettings.subDir = "tsb";
    mNodeSettings.volumeIndex=VOLUME_INDEX;
    rc = B_DVR_Manager_AllocSegmentedFileRecord(appContext->manager,&mNodeSettings,MAX_TSB_BUFFERS);
    assert(rc == 0);
    B_DVR_Manager_PrintSegmentedFileRecord(appContext->manager,&mNodeSettings);

    appContext->sMutex = B_Mutex_Create(NULL);
    assert(appContext->sMutex);
    appContext->s = B_Scheduler_Create(NULL);
    assert(appContext->s);
    appContext->sThread = B_Thread_Create("tsb_from_playback",scheduler,appContext, NULL);
    assert(appContext->sThread);
    
    appContext->streamPath.mutex = B_Mutex_Create(NULL);
    assert(appContext->streamPath.mutex);

    
    NEXUS_Platform_GetConfiguration(&appContext->platformconfig);
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    appContext->display = NEXUS_Display_Open(0, &displaySettings);
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(appContext->display, NEXUS_HdmiOutput_GetVideoConnector(appContext->platformconfig.outputs.hdmi[0]));
    NEXUS_HdmiOutput_GetSettings(appContext->platformconfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = app_hotplug_callback;
    hdmiSettings.hotplugCallback.context = appContext->platformconfig.outputs.hdmi[0];
    hdmiSettings.hotplugCallback.param = (int)appContext->display;
    NEXUS_HdmiOutput_SetSettings(appContext->platformconfig.outputs.hdmi[0], &hdmiSettings);
    #endif
    NEXUS_StcChannel_GetDefaultSettings(0,&stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    appContext->streamPath.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    assert(appContext->streamPath.stcChannel);
    appContext->streamPath.window = NEXUS_VideoWindow_Open(appContext->display,0);
    assert(appContext->streamPath.window);
    appContext->streamPath.videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    assert(appContext->streamPath.videoDecoder);
    NEXUS_VideoWindow_AddInput(appContext->streamPath.window,
                               NEXUS_VideoDecoder_GetConnector(appContext->streamPath.videoDecoder));

    appContext->streamPath.audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(appContext->platformconfig.outputs.hdmi[0]),
                               NEXUS_AudioDecoder_GetConnector(appContext->streamPath.audioDecoder,
                                                              NEXUS_AudioDecoderConnectorType_eStereo));

    #endif
    assert(appContext->streamPath.audioDecoder);

    
    appContext->streamPath.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    assert(appContext->streamPath.playpump);
    appContext->streamPath.playback = NEXUS_Playback_Create();
    assert(appContext->streamPath.playback);

    
    NEXUS_Playback_GetSettings(appContext->streamPath.playback, &playbackSettings);
    playbackSettings.playpump = appContext->streamPath.playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_Playback_SetSettings(appContext->streamPath.playback, &playbackSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    pidChannelSettings.pidTypeSettings.video.codec = ipSource[0].vCodec;
    pidChannelSettings.pidTypeSettings.video.index = true;
    pidChannelSettings.pidTypeSettings.video.decoder = appContext->streamPath.videoDecoder;
    appContext->streamPath.vPidChannel = NEXUS_Playback_OpenPidChannel(appContext->streamPath.playback, 
                                                                       ipSource[0].vPid,
                                                                       &pidChannelSettings);
    assert(appContext->streamPath.vPidChannel);
    NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidChannelSettings.pidTypeSettings.audio.primary = appContext->streamPath.audioDecoder;
    appContext->streamPath.aPidChannel = NEXUS_Playback_OpenPidChannel(appContext->streamPath.playback, 
                                                                       ipSource[0].aPid,
                                                                       &pidChannelSettings);
    assert(appContext->streamPath.aPidChannel);

    app_decode_start(&appContext->streamPath);

    NEXUS_Playback_GetSettings(appContext->streamPath.playback, &playbackSettings);
    playbackSettings.stcChannel = appContext->streamPath.stcChannel;
    playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    NEXUS_Playback_SetSettings(appContext->streamPath.playback, &playbackSettings);

    NEXUS_Playback_Start(appContext->streamPath.playback, appContext->streamPath.filePlay, NULL);

    memset((void *)&tsbServiceRequest,0,sizeof(B_DVR_TSBServiceRequest));
    sprintf(tsbServiceRequest.programName,"%s","tsb0");
    tsbServiceRequest.volumeIndex = VOLUME_INDEX;
    sprintf(tsbServiceRequest.subDir,"%s","tsb");
    tsbServiceRequest.recpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.playpumpIndex = NEXUS_ANY_ID;
    tsbServiceRequest.maxTSBBufferCount = MAX_TSB_BUFFERS;
    tsbServiceRequest.maxTSBTime = MAX_TSB_TIME;
    tsbServiceRequest.input = eB_DVR_TSBServiceInputIp;
    appContext->streamPath.tsbService = B_DVR_TSBService_Open(&tsbServiceRequest);
    assert(appContext->streamPath.tsbService);

    appContext->streamPath.serviceCallback = (B_DVR_ServiceCallback)service_callback;
    B_DVR_TSBService_InstallCallback(appContext->streamPath.tsbService,
                                     appContext->streamPath.serviceCallback,
                                     (void*)&appContext->streamPath);

    
    memset((void *)&injectOpenSettings,0,sizeof(injectOpenSettings));
    injectOpenSettings.fifoSize = 30*188; 
    appContext->streamPath.injectService  = B_DVR_DataInjectionService_Open(&injectOpenSettings);
    assert(appContext->streamPath.injectService);

    B_DVR_DataInjectionService_InstallCallback(appContext->streamPath.injectService,
                                               appContext->streamPath.serviceCallback,
                                               (void*)&appContext->streamPath);

    appContext->streamPath.injectChannel = NEXUS_Playback_OpenPidChannel(appContext->streamPath.playback,
                                                                         0x1ffe,
                                                                         NULL);
    assert(appContext->streamPath.injectChannel);

    memset(&injectSettings,0,sizeof(injectSettings));
    injectSettings.dataInjectionServiceType = eB_DVR_DataInjectionServiceTypeRawData;
    injectSettings.pidChannel = appContext->streamPath.injectChannel;
    B_DVR_DataInjectionService_SetSettings(appContext->streamPath.injectService,&injectSettings);

    memset((void *)&tsbServiceSettings,0,sizeof(tsbServiceSettings));
    tsbServiceSettings.dataInjectionService = appContext->streamPath.injectService;
    tsbServiceSettings.playpumpIp = appContext->streamPath.playpump;
    B_DVR_TSBService_SetSettings(appContext->streamPath.tsbService,&tsbServiceSettings);


    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = ipSource[0].vPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeVideo;
    inputEsStream.esStreamInfo.codec.videoCodec = ipSource[0].vCodec;
    inputEsStream.pidChannel = NULL;
    B_DVR_TSBService_AddInputEsStream(appContext->streamPath.tsbService,&inputEsStream);

    memset((void *)&inputEsStream,0,sizeof(inputEsStream));
    inputEsStream.esStreamInfo.pid = ipSource[0].aPid;
    inputEsStream.esStreamInfo.pidType = eB_DVR_PidTypeAudio;
    inputEsStream.esStreamInfo.codec.audioCodec = ipSource[0].aCodec;
    B_DVR_TSBService_AddInputEsStream(appContext->streamPath.tsbService,&inputEsStream);

    app_create_patpmt(&appContext->streamPath,
                      ipSource[0].vPid,
                      ipSource[0].vPid,
                      ipSource[0].aPid,
                      ipSource[0].vCodec,
                      ipSource[0].aCodec);
    rc = B_DVR_TSBService_Start(appContext->streamPath.tsbService,NULL);
    assert(rc == 0);
    
    rc = B_DVR_TSBService_InjectDataStart(appContext->streamPath.tsbService,
                                          (void *)&appContext->streamPath.pat[1],
                                          188);
    assert(rc == 0);
    appContext->streamPath.injectInProgress = true;

    while(1) 
    {
        app_menu();
        fflush(stdin);
        operation=0;
        printf("\n choose an operation: ");
        scanf("%u", &operation);
        if(operation == operation_equit) 
        {
            if(appContext->streamPath.sourceDecodeStarted) 
            {
                app_decode_stop(&appContext->streamPath);
            }
            if(appContext->streamPath.tsbPlayDecodeStarted) 
            {
                app_tsb_play_stop(&appContext->streamPath);
                app_tsb_decode_stop(&appContext->streamPath);
            }
            if(appContext->streamPath.conversionInProgress) 
            {
                app_tsb_convert_stop(&appContext->streamPath);
            }
            break;
        }
        switch(operation) 
        {
            case operation_etsb_play_start:
            {
                if(appContext->streamPath.sourceDecodeStarted) 
                {
                    app_decode_stop(&appContext->streamPath);
                }
                if(!appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_decode_start(&appContext->streamPath);
                    app_tsb_play_start(&appContext->streamPath);
                }
            }
            break;
        case operation_etsb_play_stop:
            {
                if(appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_play_stop(&appContext->streamPath);
                    app_tsb_decode_stop(&appContext->streamPath);
                }
                if(!appContext->streamPath.sourceDecodeStarted) 
                {
                    app_decode_start(&appContext->streamPath);
                }

            }
            break;
        case operation_etsb_play_pause:
            {
                if(appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_play_pause(&appContext->streamPath);
                }
            }
            break;
        case operation_etsb_play_unpause:
            {    
                if(appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_play_unpause(&appContext->streamPath);
                }

            }
            break;
        case operation_etsb_play_ff:
            {
                if(appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_play_ff(&appContext->streamPath);
                }
            }
        case operation_etsb_play_rew:
            {
                if(appContext->streamPath.tsbPlayDecodeStarted) 
                {
                    app_tsb_play_rew(&appContext->streamPath);
                }
            }
            break;
        case operation_etsb_conv_start:
            {
                if(!appContext->streamPath.conversionInProgress) 
                {
                    app_tsb_convert_start(&appContext->streamPath);
                }
            }
            break;
        case operation_etsb_conv_stop:
            {
                if(appContext->streamPath.conversionInProgress) 
                {
                    app_tsb_convert_stop(&appContext->streamPath);
                }
            }
            break;
        default:
            printf("\n invalid option");
        }
    }
    
    B_Scheduler_Stop(appContext->s);
    if(appContext->streamPath.injectInProgress) 
    {
        B_DVR_TSBService_InjectDataStop(appContext->streamPath.tsbService);
        if(appContext->streamPath.injectTimer)
        {
            B_Scheduler_CancelTimer(appContext->s, 
                                appContext->streamPath.injectTimer);
        }
    }
    NEXUS_Playback_ClosePidChannel(appContext->streamPath.playback,
                                   appContext->streamPath.injectChannel);
    B_DVR_DataInjectionService_RemoveCallback(appContext->streamPath.injectService);
    B_DVR_TSBService_Stop(appContext->streamPath.tsbService);
    B_DVR_DataInjectionService_Close(appContext->streamPath.injectService);
    B_DVR_TSBService_RemoveCallback(appContext->streamPath.tsbService);
    B_DVR_TSBService_Close(appContext->streamPath.tsbService);
    NEXUS_Playback_Stop(appContext->streamPath.playback);
    NEXUS_Playback_ClosePidChannel(appContext->streamPath.playback,
                                   appContext->streamPath.aPidChannel);
    NEXUS_Playback_ClosePidChannel(appContext->streamPath.playback,
                                   appContext->streamPath.vPidChannel);
    NEXUS_Playpump_Close(appContext->streamPath.playpump);
    NEXUS_Playback_Destroy(appContext->streamPath.playback);
    NEXUS_FilePlay_Close(appContext->streamPath.filePlay);

    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(appContext->platformconfig.outputs.hdmi[0]));
    NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(appContext->platformconfig.outputs.hdmi[0]));
    #endif
    NEXUS_AudioDecoder_Close(appContext->streamPath.audioDecoder);

    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(appContext->platformconfig.outputs.hdmi[0]));
    #endif
    NEXUS_VideoWindow_RemoveAllInputs(appContext->streamPath.window);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(appContext->streamPath.videoDecoder));
    NEXUS_VideoWindow_Close(appContext->streamPath.window);
    NEXUS_VideoDecoder_Close(appContext->streamPath.videoDecoder);
    NEXUS_StcChannel_Close(appContext->streamPath.stcChannel);
    NEXUS_Display_Close(appContext->display);
    B_Thread_Destroy(appContext->sThread);
    B_Scheduler_Destroy(appContext->s);
    B_Mutex_Destroy(appContext->sMutex);
    B_Mutex_Destroy(appContext->streamPath.mutex);
    memset((void *)&mNodeSettings,0,sizeof(mNodeSettings));
    mNodeSettings.programName = "tsb0";
    mNodeSettings.subDir = "tsb";
    mNodeSettings.volumeIndex=VOLUME_INDEX;
    B_DVR_Manager_FreeSegmentedFileRecord(appContext->manager,&mNodeSettings);
    B_DVR_Manager_DestroyMediaNodeList(appContext->manager,VOLUME_INDEX);
    B_DVR_Manager_UnInit(appContext->manager);
    B_DVR_MediaStorage_UnmountVolume(appContext->storage, VOLUME_INDEX);
    B_DVR_MediaStorage_Close(appContext->storage);
    NEXUS_Platform_Uninit();
    free(appContext);
error:
    return 0;
}
