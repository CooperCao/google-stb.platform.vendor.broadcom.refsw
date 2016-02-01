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
 * Data Types shall have basic data structures used by all the services and 
 * utilities in the DVRExtLib.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_DATATYPES_H
#define _B_DVR_DATATYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
/*#include <sys/cachectl.h> */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <limits.h>
#include "bstd_defs.h"
#include "bstd.h"
#include "b_os_lib.h"
#include "b_os_time.h"
#include "bkni.h"
#include "blst_queue.h"
#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "bcmplayer.h"
#include "nexus_file_pvr.h"
#include "nexus_file_types.h"
#include "nexus_types.h"
#include "nexus_dma.h"
#include "nexus_platform_features.h"
#if DRM_SUPPORT
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#endif
#if KEYLADDER_SUPPORT
#include "nexus_keyladder.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_audio_decoder.h"
#include "nexus_video_decoder.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_packetsub.h"

/*************************************************************************************
Summary:
B_DVR_ERROR shall be the status returned from most of the dvr library APIs
**************************************************************************************/
typedef unsigned B_DVR_ERROR;

/****************************************************************************
 Summary:
 B_DVR_DataInjectionServiceHandle shall be an unique identifier for a
 B_DVR_DataInjectionService instance.
 ***************************************************************************/
typedef struct B_DVR_DataInjectionService *B_DVR_DataInjectionServiceHandle;

/****************************************************************************
 Summary:
 B_DVR_DataInjectionType shall indicate type of the data to be injected
 into an ES substream of a recording.
 ***************************************************************************/
typedef enum B_DVR_DataInjectionServiceType
{
    eB_DVR_DataInjectionServiceTypePSI, /* PSI Data */
    eB_DVR_DataInjectionServiceTypeRawData,  /* Raw ES Data */
    eB_DVR_DataInjectionServiceTypeMax
}B_DVR_DataInjectionServiceType;

/****************************************************************************
 Summary:
 B_DVR_DataInjectionOpenSettings shall have settings for dataSize
 that can be injected in one shot
 ***************************************************************************/
typedef struct B_DVR_DataInjectionServiceOpenSettings
{
    unsigned fifoSize;
 }B_DVR_DataInjectionServiceOpenSettings;

/****************************************************************************
 Summary:
 B_DVR_DataInjectionSettings shall indicate the settings for a data injection
 service instance.
 ***************************************************************************/
typedef struct B_DVR_DataInjectionServiceSettings
{
    unsigned pid;     /* This is valid only when app requests dataInjection in PSI table format */
    B_DVR_DataInjectionServiceType dataInjectionServiceType; /* Data injection type */
    NEXUS_PidChannelHandle pidChannel; /* 
                                        *  pidChannel handle used for data injection.
                                        *  application should open pid channel 
                                        *  using a DUMMY PID like 0x1fffe and also 
                                        *  the PID channel needs to be disabled.
                                        */
}B_DVR_DataInjectionServiceSettings;

/*************************************************************************************************
Summary:
B_DVRServiceHandle shall be an unique identifier for a DRM service instance.
**************************************************************************************************/
typedef struct B_DVR_DRMService *B_DVR_DRMServiceHandle;


/*************************************************************************************************
Summary: 
B_DVR_DRMServiceKeyType shall indicate what kind of digital protection is 
required for encryption or decryption
**************************************************************************************************/
typedef enum B_DVR_DRMServiceType{
    eB_DVR_DRMServiceTypeBroadcastMedia,    /* Content Protection for Cable MSO media through the cable input */
    eB_DVR_DRMServiceTypeContainerMedia,    /* Content protection for media in container formats i.e. internet media streams */   
    eB_DVR_DRMServiceTypeContainerMax
}B_DVR_DRMServiceType;

/*************************************************************************************************
Summary: 
B_DVR_DRMServiceKeyType shall indicate the key type used for content protection.
**************************************************************************************************/
typedef enum B_DVR_DRMServiceKeyType
{
    eB_DVR_DRMServiceKeyTypeProtected,   
    eB_DVR_DRMServiceKeyTypeClear,
    eB_DVR_DRMServiceKeyTypeMax
}B_DVR_DRMServiceKeyType;

/*************************************************************************************************
Summary: 
B_DVR_DRMUsage enumerates usage of the DRMServiceHandle for a particular service.
**************************************************************************************************/
typedef enum B_DVR_DRMUsage
{
    eB_DVR_DRMUsageRecord,   
    eB_DVR_DRMUsagePlayback,
    eB_DVR_DRMUsageMediaProbe,
    eB_DVR_DRMUsageMax
}B_DVR_DRMUsage;

/************************************************************************************
Summary:
B_DVR_Service shall indicate the service type
*************************************************************************************/
typedef enum B_DVR_Service
{   
    eB_DVR_ServiceTSB,        
    eB_DVR_ServicePlayback,   
    eB_DVR_ServiceRecord,     
    eB_DVR_ServiceMedia,      
    eB_DVR_ServiceHomeNetworkDRM, 
    eB_DVR_ServiceStorage,
    eB_DVR_ServiceDataInjection,
    eB_DVR_ServiceTranscode,
    eB_DVR_ServiceMax
}B_DVR_Service;


/*************************************************************************************
Summary:
B_DVR_Recording shall indicate different dynamically linked lists of media nodes
stored in the DRAM.
**************************************************************************************/
typedef enum B_DVR_Recording
{
    eB_DVR_RecordingTSB,                /* Recording type is time shift buffering state */
    eB_DVR_RecordingPermanent,          /* Recording type is permanent recording state */
    eB_DVR_RecordingMax
}B_DVR_Recording;


/**************************************************************************************
Summary:
B_DVR_PidType shall provide info on type of elementary stream in a stream.
***************************************************************************************/
typedef enum B_DVR_PidType
{
    eB_DVR_PidTypeAudio,
    eB_DVR_PidTypeVideo,
    eB_DVR_PidTypeData,
    eB_DVR_PidTypeMax
}B_DVR_PidType;


/**************************************************************************************
Summary:
B_DVR_EsStreamInfo shall be used for information exchange between DVRExtLib and upper
software layers. For instance pidChannel would be filled in by the DVRExtLib and 
sent to the application. drmService handle per PID is always coming from the application.
pid, pidType and codec information shall be provided by the application in recording/TSB
buffering and the same shall be provided by the DVRExtLib during playback.
This information shall be stored in the mediaNode also for playback purposes.
***************************************************************************************/
typedef struct B_DVR_EsStreamInfo
{
    unsigned pid;
    B_DVR_PidType pidType;
    union
    {
        NEXUS_VideoCodec videoCodec;
        NEXUS_AudioCodec audioCodec;
    }codec;
   unsigned bitRate;      /*bit rate is in bps (only valid for audio and video ES)*/
   /* The below parameters are applicable only for video ES*/
   unsigned videoHeight; 
   unsigned videoWidth;  
   unsigned videoFrameRate; /* 1000 FPS units */
   unsigned profile;  /* 1. MPEG2 -> ISO/IEC 13818-2 , Table 8-2 . Profile identification 
                         2. AVC ->    
                       */
   unsigned level;    /* 1. MPEG2 -> ISO/IEC 13818-2 , Table 8-3 . Level identification 
                         2. AVC ->
                      */
   /* The below parameters are applicable only for audio ES*/
   unsigned audioChannelCount; /* number of channels, or 0 if unknown  */
   unsigned audioSampleSize; /* number of bits in the each sample, or 0 if unknown */
   unsigned audioSampleRate; /* audio sampling rate in Hz, or 0 if unknown */
} B_DVR_EsStreamInfo;

/****************************************************************************************
Summary:
B_DVR_Event shall provide info about various events raised by DVR library
******************************************************************************************/
typedef enum B_DVR_Event{
    eB_DVR_EventStartOfRecording,
    eB_DVR_EventEndOfRecording,
    eB_DVR_EventHDStreamRecording,
    eB_DVR_EventSDStreamRecording,
    eB_DVR_EventTSBConverstionCompleted,
    eB_DVR_EventOverFlow,
    eB_DVR_EventUnderFlow,
    eB_DVR_EventStartOfPlayback,
    eB_DVR_EventEndOfPlayback,
    eB_DVR_EventPlaybackAlarm,
    eB_DVR_EventAbortPlayback,
    eB_DVR_EventAbortRecord,
    eB_DVR_EventAbortTSBRecord,
    eB_DVR_EventAbortTSBPlayback,
    eB_DVR_EventDataInjectionCompleted,
    eB_DVR_EventTranscodeFinish,
    eB_DVR_EventMediaProbed,
    eB_DVR_EventOutOfMediaStorage,
    eB_DVR_EventOutOfNavigationStorage,
    eB_DVR_EventOutOfMetaDataStorage,
    eB_DVR_EventFormatSuccess,
    eB_DVR_EventFormatFail,
    eB_DVR_EventFormatFail_InvalidIndex,
    eB_DVR_EventFormatFail_NotRegistered,
    eB_DVR_EventFormatFail_VolumeMounted,
    eB_DVR_EventFormatFail_SystemErr,    
    eB_DVR_EventMountSuccss,
    eB_DVR_EventMountFail,
    eB_DVR_EventMountFail_InvalidIndex,
    eB_DVR_EventMountFail_NotRegistered,
    eB_DVR_EventMountFail_VolumeMounted,
    eB_DVR_EventMountFail_NotFormatted,
    eB_DVR_EventMountFail_SystemErr,
    eB_DVR_EventMountFail_WrongLabel,
    eB_DVR_EventMediaStorage_CallbackStop,
    eB_DVR_EventMax
}B_DVR_Event;

/********************************************************************************************
Summary:
B_DVR_Operation shall indicate various DVR operations requested by the application
********************************************************************************************/
typedef enum B_DVR_Operation
{
    eB_DVR_OperationTSBPlayStart = 0,
    eB_DVR_OperationTSBPlayStop,
    eB_DVR_OperationPause,
    eB_DVR_OperationPlay,
    eB_DVR_OperationPlayGop,
    eB_DVR_OperationPlayTimeSkip,
    eB_DVR_OperationFastForward,
    eB_DVR_OperationFastRewind,
    eB_DVR_OperationSlowRewind,
    eB_DVR_OperationSlowForward,
    eB_DVR_OperationForwardFrameAdvance,
    eB_DVR_OperationReverseFrameAdvance,
    eB_DVR_OperationSeek,
    eB_DVR_OperationSpeed,
    eB_DVR_OperationMax
}B_DVR_Operation;

/***********************************************************************************************
Summary:
D_DVR_OperationSettings shall be used by the appplication to provide TSB and Playback operation
commands for TSB playback and Playback Services.
***********************************************************************************************/
typedef struct D_DVR_OperationSettings
{
    B_DVR_Operation operation;
    int operationSpeed; /* TODO document various speeds*/
    int mode_modifier;
    unsigned long  seekTime; /* time in milliseconds*/
}B_DVR_OperationSettings;


/***********************************************************************************************
 Summary:
 A callback type to be used by the application to register before starting any services 
 provided in the dvr library.
***********************************************************************************************/
typedef void (*B_DVR_ServiceCallback)(void * appContext, unsigned index, B_DVR_Event event, B_DVR_Service service);


/************************************************************************************************
Summary:
B_DVR_Media shall contain all the information regarding a stream for playing back or streaming
out to clients. This data shall be written as it is in binary format into a .info file 
in the metaData partition of the mediaStorage.
*************************************************************************************************/
typedef struct __attribute__((aligned(4),packed)) B_DVR_Media
{
    BDBG_OBJECT(B_DVR_Media)
    BLST_Q_ENTRY(B_DVR_Media) nextMedia;
    /* Fixed portion of the meta data 
       Start ---
     */
   /*********************************************************
    defined in b_dvr_const.h
    B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM       (0x00000001)
    B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM       (0x00000002)
    B_DVR_MEDIA_ATTRIBUTE_HD_STREAM              (0x00000004)
    B_DVR_MEDIA_ATTRIBUTE_AUDIO_ONLY_STREAM      (0x00000008)
    B_DVR_MEDIA_ATTRIBUTE_HITS_STREAM            (0x00000010)
    B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS  (0x00000020)
    B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED      (0x00000400)
    B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE  (0x00000800)
   **********************************************************/
    unsigned mediaAttributes; 
    NEXUS_TransportType transportType;
    char mediaFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char navFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char mediaNodeFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH];
    char programName[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_EsStreamInfo esStreamInfo[B_DVR_MAX_PIDS];
    unsigned maxStreamBitRate;
    unsigned esStreamCount;
    unsigned long mediaStartTime;
    unsigned long mediaEndTime;
    off_t mediaLinearStartOffset;
    off_t mediaLinearEndOffset;
    off_t navLinearStartOffset;
    off_t navLinearEndOffset;
    B_DVR_Recording recording;
    B_DVR_DRMServiceType drmServiceType;
    B_DVR_DRMServiceKeyType drmServiceKeyType;
    /*
     * These are used for only clear keys for the 
     * PIDs in a stream.
     */
    unsigned keyBlob[B_DVR_MAX_PIDS][B_DVR_MAX_KEY_LENGTH];
    unsigned keyLength;
    unsigned version;
    /* Fixed portion of the meta data 
       end ---
     */
    bool reserved;
    unsigned reservedDataCount;
    bool customData;
    unsigned maxCustomDataCount;
    unsigned refCount;
}B_DVR_Media;

typedef B_DVR_Media *B_DVR_MediaNode;

/*******************************************************************************
Summary:
B_DVR_MediaNodeSettings shall be used as an input parameter by
the app/middleware get the mediaNode associated with the request
settings.
*******************************************************************************/
typedef struct B_DVR_MediaNodeSettings
{
    unsigned volumeIndex;  
    char *programName;
    char *subDir;
}B_DVR_MediaNodeSettings;

/*******************************************************************************
Summary:
B_DVR_MemoryType refers to DVR memory enumeration.
*******************************************************************************/
typedef enum B_DVR_MemoryType
{
    eB_DVR_MemoryType_Device,
    eB_DVR_MemoryType_System,
    eB_DVR_MemoryType_Max
}B_DVR_MemoryType;

#define B_DVR_IO_BLOCK_SIZE 512
#define B_DVR_IO_ALIGN_TRUNC(n) ((n)&(~(B_DVR_IO_BLOCK_SIZE-1)))
#define B_DVR_IO_ALIGN_ROUND(n) (((n)+B_DVR_IO_BLOCK_SIZE-1)&(~(B_DVR_IO_BLOCK_SIZE-1)))
#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_DATATYPES_H */
