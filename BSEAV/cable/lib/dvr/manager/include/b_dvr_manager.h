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
 *  ANY LIMITED REMEDY..
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * DVR Manager shall have a context for all the services provided including
 * mediaStorage. It shall read the media metaData and create all the 
 * lists in the system memory for faster access to the recordings.
 * It shall also check whether the number of service contexts requested
 * by the application is possible.
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_MANAGER_H
#define _B_DVR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
#include "b_dvr_list.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_playbackservice.h"
#include "b_dvr_recordservice.h"
#include "b_dvr_mediafile.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_segmentedfile.h"
#ifdef TRANSCODE_SUPPORT
#include "b_dvr_transcodeservice.h"
#endif

BDBG_OBJECT_ID_DECLARE(B_DVR_Manager);

struct B_DVR_Manager
{
    BDBG_OBJECT(B_DVR_Manager)
    B_DVR_RecordServiceHandle recordService[B_DVR_MAX_RECORDING];
    B_DVR_PlaybackServiceHandle  playbackService[B_DVR_MAX_PLAYBACK];
    B_DVR_TSBServiceHandle tsbService[B_DVR_MAX_TSB];
    B_DVR_DRMServiceHandle drmService[B_DVR_MAX_DRM];
    B_DVR_MediaFileHandle mediaFile[B_DVR_MAX_MEDIAFILE];
    B_DVR_DataInjectionServiceHandle dataInjectionService[B_DVR_MAX_DATAINJECTION];
    #ifdef TRANSCODE_SUPPORT
    B_DVR_TranscodeServiceHandle transcodeService[B_DVR_MAX_TRANSCODE];
    #endif
    B_DVR_MediaStorageHandle mediaStorage;
    B_DVR_ListHandle dvrList;
    unsigned recordServiceCount;
    unsigned playbackServiceCount;
    unsigned transcodeServiceCount;
    unsigned tsbServiceCount;
    unsigned drmServiceCount;
    unsigned dataInjectionServiceCount;
    unsigned mediaFileCount;
    B_MutexHandle dvrManagerMutex;
    B_SchedulerHandle scheduler;
    B_ThreadHandle thread;
    unsigned mediaSegmentSize;
    unsigned long systemMemoryUsed;
    unsigned long deviceMemoryUsed;
    B_MutexHandle memoryUsageMutex;
};

/*********************************************************************************
B_DVR_ManagerHandle shall be an unique identifier for the DVR manager.
*********************************************************************************/
typedef struct B_DVR_Manager *B_DVR_ManagerHandle;

typedef struct B_DVR_ManagerSettings
{
    unsigned mediaSegmentSize;
    NEXUS_ParserBand dataInjectionParserBand;
}B_DVR_ManagerSettings;
/*******************************************************************************
Summary:
B_DVR_Manager_GetHandle shall return the DVR manager global handle.
return value
B_DVR_ManagerHandle - Setting of the DVR manager.
********************************************************************************/
B_DVR_ManagerHandle B_DVR_Manager_GetHandle(void);

/**********************************************************************************
Summary:
B_DVR_Manager_Init shall allocate the data structures, initialize the scheduler,
and also initialize various recording lists.
Param[in]
pSettings - Settings for the DVR manager.
Param[out]
B_DVR_ManagerHandle - Handle for DVR manager
***********************************************************************************/
B_DVR_ManagerHandle B_DVR_Manager_Init(B_DVR_ManagerSettings *pSettings);

/***********************************************************************************
Summary:
B_DVR_Manager_Uninit shall uninitialize the DVR manager.
Param[in]
b_dvr_manager - Handle for DVR manager
return value
B_DVR_ERROR - Error code returned.
************************************************************************************/
B_DVR_ERROR B_DVR_Manager_UnInit(
    B_DVR_ManagerHandle manager);

/**********************************************************************************
Summary:
B_DVR_Manager_CreateRecording shall create the metadata info for a file downloaded through
IP network.
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
return value
B_DVR_ERROR - Error code returned.
***********************************************************************************/
B_DVR_ERROR B_DVR_Manager_CreateRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings);

/***************************************************************************************
Summary:
B_DVR_Manager_DeleteRecording shall delete a permanent recording from the storage space.
Deleting involves freeing the meta data for the recording in questions. 
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
Param[out]
B_DVR_ERROR - Error code returned.
****************************************************************************************/
B_DVR_ERROR B_DVR_Manager_DeleteRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings);


/***************************************************************************************
Summary:
B_DVR_Manager_MoveRecording shall move a permanent recording to another directory
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
Param[in]
destDir - input parameters destination dir
Param[in]
destName - input parameters destination name
Param[out]
B_DVR_ERROR - Error code returned.
****************************************************************************************/
B_DVR_ERROR B_DVR_Manager_MoveRecording(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *src,
    char *destDir,
    char *destName);

/***************************************************************************************
Summary:
B_DVR_Manager_GetNumberOfRecordings shall return number of permanent recordings available
on the media partition.
Param[in]
b_dvr_manager - Handle for DVR manager
mediaNodeSettings - input parameters identifying a set of mediaNodes.
                    mediaNodeSettings.programNama shall be null.
return value
unsigned - recording count
***************************************************************************************/
unsigned  B_DVR_Manager_GetNumberOfRecordings(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings);

/****************************************************************************************
Summary: 
B_DVR_Manager_GetRecordingList shall return list of recordings to the upper layer.
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a set of mediaNodes.
                    mediaNodeSettings.programNama shall be null.
Param[out]
recordingList - string array provided by the app to be filled by the dvr manager.
Param[in]
recordingsCount - count returned from B_DVR_GetNumberOfRecordings
return value
B_DVR_ERROR - Error code returned.
******************************************************************************************/
unsigned B_DVR_Manager_GetRecordingList(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned listCount,
    char (*recordingList)[B_DVR_MAX_FILE_NAME_LENGTH]);

/**************************************************************************************
Summary:
B_DVR_Manager_GetVendorSpecificMetaData 
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
Param[in]
size - size of the vendor specific meta data.
Param[out]
vendorData - Vendor specific meta data
return value
B_DVR_ERROR - Error code returned.
***************************************************************************************/
B_DVR_ERROR B_DVR_Manager_GetVendorSpecificMetaData(
     B_DVR_ManagerHandle manager,
     B_DVR_MediaNodeSettings *mediaNodeSettings,
     void *vendorData,
     unsigned *size,
     unsigned offset);

/**************************************************************************************
Summary:
B_DVR_SetVendorSpecificMetaData 
Param[in]
b_dvr_manager - Handle for DVR manager
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
Param[in]
vendorData - Vendor specific meta data
Param[in]
size - size of the vendor specific meta data.
return value
B_DVR_ERROR - Error code returned.
***************************************************************************************/
B_DVR_ERROR B_DVR_Manager_SetVendorSpecificMetaData(
     B_DVR_ManagerHandle manager,
     B_DVR_MediaNodeSettings *mediaNodeSettings,
     void *vendorData,
     unsigned *size);

/***************************************************************************************
Summary:
B_DVR_Manager_GetMediaNode shall return a mediaNode based on the mediaNodeSettings
passed by the application.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - input parameters identifying a mediaNode.
Param[out]
mediaNode - mediaNode based on mediaNodeSettings
return value
B_DVR_ERROR - Error code returned
***************************************************************************************/
B_DVR_ERROR B_DVR_Manager_GetMediaNode(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    B_DVR_MediaNode mediaNode);

/**************************************************************************************
Summary:
B_DVR_Manager_CreateMediaNodeList shall create the mediaNode list for all the tsb
and permanent recording in a volume on a storage device.
Param[in]
manager - dvr manager handle
Param[in]
volumeIndex - volumeIndex in a storage device.
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_CreateMediaNodeList(
    B_DVR_ManagerHandle manager,
    unsigned volumeIndex);

/**************************************************************************************
Summary:
B_DVR_Manager_DestroyMediaNodeList shall destroy the mediaNode list for all the tsb
and permanent recording in a volume on a storage device.
Param[in]
manager - dvr manager handle
Param[in]
volumeIndex - volumeIndex in a storage device.
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_DestroyMediaNodeList(
    B_DVR_ManagerHandle manager,
    unsigned volumeIndex);

/**************************************************************************************
Summary:
B_DVR_Manager_SetKeyBlobPerEsStream shall set the keyBlob for an ES stream.
In case of even and odd keys, keyblob shall have both even and odd keys.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
Param[in]
pid - PID value for an elementary stream
Param[in]
keyBlob - keys for an ES stream
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_SetKeyBlobPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned pid,
    uint8_t *keyBlob);

/**************************************************************************************
Summary:
B_DVR_Manager_GetKeyBlobPerEsStream shall get the keyBlob for an ES stream.
In case of even and odd keys, keyblob shall have both even and odd keys.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
Param[in]
pid - PID value for an elementary stream
Param[out]
keyBlob - keys for an ES stream
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_GetKeyBlobPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned pid,
    uint8_t *keyBlob);

/**************************************************************************************
Summary:
B_DVR_Manager_SetKeyBlobLengthPerEsStream shall set the keyLength for all the ES streams
in a program.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
Param[in]
pid - PID value for an elementary stream
Param[in]
keyLength - key length for all the ES streams
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_SetKeyBlobLengthPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned keyLength);

/**************************************************************************************
Summary:
B_DVR_Manager_GetKeyBlobLengthPerEsStream shall get the keyLength for all the ES streams
in a program.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
Param[in]
pid - PID value for an elementary stream
Param[out]
keyLength - key length for all the ES streams
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/
B_DVR_ERROR B_DVR_Manager_GetKeyBlobLengthPerEsStream(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned *keyLength);


/**************************************************************************************
Summary:
B_DVR_Manager_AllocSegmentedFileRecord shall create .info, .ts and .nav metaData
files and also allocate media and nav segments associated with .ts and .nav 
metadata files. 
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/

B_DVR_ERROR B_DVR_Manager_AllocSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings,
    unsigned maxFileSegments);

/**************************************************************************************
Summary:
B_DVR_Manager_FreeSegmentedFileRecord shall delete .info, .ts and .nav metaData
files and also free media and nav segments associated with .ts and .nav 
metadata files.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/

B_DVR_ERROR B_DVR_Manager_FreeSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings);

/**************************************************************************************
Summary:
B_DVR_Manager_PrintSegmentedFileRecord shall print the contents of.info, .ts and .nav metaData
files and also  media and nav segments names associated with .ts and .nav 
metadata files.
Param[in]
manager - dvr manager handle
Param[in]
mediaNodeSettings - settings for identifying a mediaNode.
return value 
B_DVR_ERROR - Error code returned.
**************************************************************************************/

B_DVR_ERROR B_DVR_Manager_PrintSegmentedFileRecord(
    B_DVR_ManagerHandle manager,
    B_DVR_MediaNodeSettings *mediaNodeSettings);

/**************************************************************************************
Summary:
B_DVR_Manager_GetSystemMemoryUsage shall return system memory used for 
dvr purposes. 
Param[in]
manager - dvr manager handle. 
memoryType - type of memory 
return value 
device/system memory size in kB.
**************************************************************************************/
unsigned long B_DVR_Manager_GetMemoryUsage(
    B_DVR_ManagerHandle manager, 
    B_DVR_MemoryType memoryType);

#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_MANAGER_H */
