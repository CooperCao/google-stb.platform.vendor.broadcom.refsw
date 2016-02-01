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
 * Media storage shall be a storage manager in the DVR library.
 * Media storage manages the storage space provided by the application
 * by creating either segmented file system (SFS) or virtual
 * segmented file system.
 ***********************************************************************
 *                 SEGMENTED FILE SYSTEM (SFS)
 ************************************************************************* 
 * App provides, a storage device name,start sector and size in megabytes.
 * Media storage takes these parameters and creates 3 partitions -
 * 1. media 2. navigation  3. metadata.
 * The size of these partitions are in some ratio which is decided
 * internally in the media storage, but the total size of the 3 partitions
 * would be equal to the size passed by the application. The partition table is updated.
 * These 3 partitions shall have ext4 file system created on them.
 * After file system is created, media and navigation partitions will
 * be used up fully by creating fixed sized files. These files are
 * then used by different services in the library. media and navigation
 * file segments are linked to create meta data files to represent a
 * recorded program. These meta data files are created in meta
 * data partition. File can be deleted and created in meta data partition,
 * but media and navigation fixed sized files once created would never
 * be deleted. This achieves fragmentation free media storage management
 * and also high performance since all the blocks in a media and
 * navigation file segment are sequential.
 * 
 *************************************************************************** 
 *            VIRTUAL SEGMENTED FILE SYSTEM (VSFS)
 *************************************************************************** 
 *  App provides a directory on an existing partition in any device
 *  along with size to be managed by the media storage.
 *  Media storage pre-allocates 3 files in the app provided directory
 *  with different sizes - 1. media 2. navigation and 3. metadata
 *  The total size of these 3 files would be equal to free space
 *  size provided by the application. These 3 files would act as
 *  loop devices depicting the 3 partitions in the segmented
 *  file system. Similar to segmented file system,
 *  EXT4 file system shall be created on these 3 loop devices.
 *  Media and navigation loop devices shall have pre-allocated
 *  fixed sized files which are linked by meta data files
 *  located in the meta loop device to represent a recorded
 *  program. This achieves fragmentation free storage management.
 * 
 *  Multiple devices or directories can be registered
 *  with the media storage to create multiple volumes.
 *  Each volume would be assocated with exactly one
 *  directory in an existing partition of any device
 *  or one device with some free space available.
 * 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/vfs.h> 
#include <sys/statvfs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>

#include "b_os_lib.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "b_dvr_mediastorage.h"
#include "msutil.h"

BDBG_MODULE(b_dvr_mediastorage);

#define BDBG_TRACE(X) /* BDBG_MSG */
#define DEBUG_SEG       0
#define B_DVR_SCAN_DISK_CMD         "cat /proc/diskstats | grep \"sd. \" >%s"
#define B_DVR_SCAN_TEMP_FILE        "/tmp/scantmp.txt"

static void B_DVR_P_MediaStorage_CallbackThread(void *context);
static void B_DVR_P_MediaStorage_MountThread (void *context);
static void B_DVR_P_MediaStorage_FormatThread (void *context);
static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseNavReferenceCount(
            B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,const char * seg, uint8_t *refCnt);
static B_DVR_ERROR B_DVR_P_MediaStorage_ScanDisk(B_DVR_MediaStorageHandle mediaStorage);
static B_DVR_ERROR B_DVR_P_MediaStorage_FindAndMoveTsSegment(
        char *srcpath, char *destpath, char *pSegment);
static B_DVR_ERROR B_DVR_P_MediaStorage_MoveSegment(char *srcpath, char *destpath, const char *pSegment);
static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentReferenceCount(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char * seg, unsigned *count);
static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentReferenceCount(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char * seg, unsigned *count);
static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentSharedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count);

static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentSharedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count);

static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentUsedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count);

static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentUsedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned  *count);

/******************************************************************************
Summary:
B_DVR_MediaStorage shall be a context containing information on all the volumes 
controlled by the DVRExtLib.
*******************************************************************************/
BDBG_OBJECT_ID_DECLARE(B_DVR_MediaStorage);

BDBG_OBJECT_ID(B_DVR_MediaStorage);

typedef struct B_DVR_MediaStorage 
{
    BDBG_OBJECT(B_DVR_MediaStorage)
    B_DVR_MediaStorageVolume *volume[B_DVR_MEDIASTORAGE_MAX_VOLUME];
    unsigned numMountedVolumes; /* current number of mounted volumes */
    unsigned numRegisteredVolumes;  /* current number of registered volumes */
    B_DVR_ServiceCallback callback;
    void *appContext;
    B_MutexHandle mlock;
    B_MessageQueueHandle cbQueue;
    B_ThreadHandle thread;
    B_DVR_MediaStorageSettings settings;
    B_DVR_MediaStorageType storageType;
} B_DVR_MediaStorage;

typedef struct {
    B_DVR_MediaStorageHandle mediaStorage;
    unsigned volumeIndex;
} B_DVR_ThreadParam;

typedef struct {
    unsigned index;
    B_DVR_Event event;
} B_DVR_CallbackMessage;

static B_DVR_MediaStorageHandle gmediaStorageHandle=NULL;

void B_DVR_MediaStorage_GetDefaultOpenSettings(B_DVR_MediaStorageOpenSettings *pOpenSettings)
{
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));
    BDBG_ASSERT(pOpenSettings);
    /*
     * Block device based SFS is the preferred storage method
     */
    pOpenSettings->storageType = eB_DVR_MediaStorageTypeBlockDevice;
    BDBG_TRACE(("-------- %s ",__FUNCTION__));
}
B_DVR_MediaStorageHandle 
B_DVR_MediaStorage_Open(const B_DVR_MediaStorageOpenSettings *pOpenSettings)
{
    B_DVR_MediaStorage *mediaStorage;
    unsigned i;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    B_MessageQueueSettings              qSettings;
    
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));
    if (gmediaStorageHandle) return gmediaStorageHandle;
    BDBG_ASSERT(pOpenSettings);
    if (B_Os_Init() != B_ERROR_SUCCESS) {
        goto error;
    }

    mediaStorage = BKNI_Malloc(sizeof(B_DVR_MediaStorage));   
    if (!mediaStorage) goto error;
    
    gmediaStorageHandle = mediaStorage;
    
    BKNI_Memset(mediaStorage,0,sizeof(B_DVR_MediaStorage));
    BDBG_OBJECT_SET(mediaStorage,B_DVR_MediaStorage);

    mediaStorage->mlock = B_Mutex_Create(NULL);
    if (!mediaStorage->mlock) {
        BDBG_ERR(("%s mutex create error", __FUNCTION__));
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error_handle;
    }

    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) {
        mediaStorage->volume[i] = BKNI_Malloc(sizeof(B_DVR_MediaStorageVolume));
        if (!mediaStorage->volume[i]) goto error_mutex;

        BKNI_Memset(mediaStorage->volume[i],0,sizeof(B_DVR_MediaStorageVolume));
    }

    qSettings.maxMessages = B_DVR_MAX_MESSAGES;
    qSettings.maxMessageSize = sizeof(B_DVR_CallbackMessage);
    mediaStorage->cbQueue = B_MessageQueue_Create(&qSettings);

    if (!mediaStorage->cbQueue) {
        BDBG_ERR(("%s message queue create error", __FUNCTION__));
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error_volume;
    }
    
    mediaStorage->thread = B_Thread_Create("MediaStorage Callback Thread", 
                        B_DVR_P_MediaStorage_CallbackThread,gmediaStorageHandle, NULL);
                        
    if (!mediaStorage->thread) {
        BDBG_ERR(("%s scheduler thread create error", __FUNCTION__));
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto eror_queue;
    }

    if (ms_init()!=eMS_OK) 
        goto error_thread;

    if(pOpenSettings->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        BDBG_WRN(("mediaStorage type is a block device"));
        mediaStorage->storageType = pOpenSettings->storageType;
        err = B_DVR_P_MediaStorage_ScanDisk(mediaStorage);
        if (err) goto error_thread;    
    }
    else
    { 
        if(pOpenSettings->storageType == eB_DVR_MediaStorageTypeLoopDevice) 
        {
            mediaStorage->storageType = pOpenSettings->storageType;
            BDBG_WRN(("mediaStorage type is a loop device"));
        }
        else
        {
            BDBG_WRN(("invalid mediaStorage type"));
            goto error_thread;
        }
    }
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return mediaStorage;

error_thread:
    B_Thread_Destroy(mediaStorage->thread);
eror_queue:
    B_MessageQueue_Destroy(mediaStorage->cbQueue);
error_volume:
    for (i=0;i<B_DVR_MEDIASTORAGE_MAX_VOLUME;i++) {
        if(mediaStorage->volume[i]) {
            BKNI_Free(mediaStorage->volume[i]);
        }
    }
error_mutex:
    B_Mutex_Destroy(mediaStorage->mlock);
error_handle:
    BKNI_Free(mediaStorage);    
error:
    BDBG_ERR(("B_DVR_MediaStorage_Open error %d", err));
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return NULL;
}

B_DVR_MediaStorageHandle 
B_DVR_MediaStorage_GetHandle(void)
{
    if (!gmediaStorageHandle) {
        BDBG_ERR(("B_DVR_MediaStorage_Open should be called first"));
    }

    return gmediaStorageHandle;
}

void 
B_DVR_MediaStorage_Close(B_DVR_MediaStorageHandle mediaStorage)
{
    int i;
    B_DVR_CallbackMessage callbackMessage;
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
    callbackMessage.event = eB_DVR_EventMediaStorage_CallbackStop;
    B_MessageQueue_Post(mediaStorage->cbQueue,(void *)(&callbackMessage),sizeof(B_DVR_CallbackMessage));
    B_Thread_Destroy(mediaStorage->thread);
    B_MessageQueue_Destroy(mediaStorage->cbQueue);
    B_Mutex_Destroy(mediaStorage->mlock);

    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) {
        BKNI_Free(mediaStorage->volume[i]);
    }
    
    BDBG_OBJECT_DESTROY(mediaStorage,B_DVR_MediaStorage);
    gmediaStorageHandle = NULL;
    BKNI_Free(mediaStorage);

    BDBG_TRACE(("-------- %s",__FUNCTION__));
}

B_DVR_ERROR 
B_DVR_MediaStorage_RegisterVolume(B_DVR_MediaStorageHandle mediaStorage, B_DVR_MediaStorageRegisterVolumeSettings *pSettings, unsigned *volumeIndex)
{
    unsigned i;
    int ret;
    B_DVR_ERROR dvrStatus=B_DVR_SUCCESS;
    MS_createPartitionSettings_t settings;
    
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    B_Mutex_Lock(mediaStorage->mlock);
    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) 
    {
        if (!mediaStorage->volume[i]->registered) 
        {
            *volumeIndex = i;
            break;
        }
    }
    if(i>=B_DVR_MEDIASTORAGE_MAX_VOLUME) 
    {
        BDBG_ERR(("Max volume supported is 4. No more free slots for volume left"));
        ret = B_DVR_NOT_SUPPORTED;
        goto error;
    }

    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        settings.startSec = pSettings->startSec;
        settings.size = pSettings->length;

        ret = ms_create_volume(pSettings->device,&settings);
        if (ret==eMS_OK)
        {
            snprintf(mediaStorage->volume[i]->deviceName, B_DVR_MAX_DEVICENAME, "%s", pSettings->device);
            mediaStorage->volume[i]->registered = 1;
            mediaStorage->volume[i]->mounted = 0;
            mediaStorage->volume[i]->formatted = 0;
            snprintf(mediaStorage->volume[i]->mediaPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.mediaPartition);
            snprintf(mediaStorage->volume[i]->navPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.navPartition);
            snprintf(mediaStorage->volume[i]->metadataPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.metadataPartition);
            mediaStorage->numRegisteredVolumes++;
        } 
        else 
        {
            dvrStatus = B_DVR_INVALID_PARAMETER;
        }
        
    }
    else
    {
        BDBG_WRN(("virtual volume would be created with 3 loop devices associated"));
        settings.startSec = 0;
        settings.size = pSettings->length;
        ret = ms_create_virtual_volume(pSettings->device,&settings);
        if (ret==eMS_OK || ret == eMS_ERR_VOLUME_EXISTS)
        {
             if(ret == eMS_ERR_VOLUME_EXISTS) 
             {
                 MS_StorageStatus_t status;
                 ret = ms_check_virtual_volume(pSettings->device, &status);
                 if (ret!=eMS_OK ) 
                 {
                     BDBG_ERR(("ms_check_volume error"));
                     dvrStatus = B_DVR_OS_ERROR;
                     goto error;
                 }
                 if (status.state==eMS_StateReady || status.state==eMS_StateEmpty) 
                 {
                     BDBG_ERR(("device %s is a %s media volume\n",pSettings->device,(status.state==eMS_StateEmpty)?"empty":"formatted"));
                     mediaStorage->volume[i]->formatted = (status.state==eMS_StateEmpty)?0:1;
                     mediaStorage->volume[i]->registered = 1;
                     mediaStorage->volume[i]->startSector = status.startSector;
                     mediaStorage->volume[i]->endSector = status.endSector;
                     mediaStorage->volume[i]->mediaSegTotal = status.mediaSegTotal;
                     mediaStorage->numRegisteredVolumes++;            
                     strncpy(mediaStorage->volume[i]->mediaPartition, status.mediaPartition,B_DVR_MAX_PARTITIONNAME-1);
                     strncpy(mediaStorage->volume[i]->navPartition, status.navPartition,B_DVR_MAX_PARTITIONNAME-1);
                     strncpy(mediaStorage->volume[i]->metadataPartition, status.metadatapartition,B_DVR_MAX_PARTITIONNAME-1);
                     snprintf(mediaStorage->volume[i]->deviceName, B_DVR_MAX_DEVICENAME,"%s",pSettings->device);
                 }
             }
             else
             {
                 snprintf(mediaStorage->volume[i]->deviceName, B_DVR_MAX_DEVICENAME, "%s", pSettings->device);
                 mediaStorage->volume[i]->registered = 1;
                 mediaStorage->volume[i]->mounted = 0;
                 mediaStorage->volume[i]->formatted =0;
                 snprintf(mediaStorage->volume[i]->mediaPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.mediaPartition);
                 snprintf(mediaStorage->volume[i]->navPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.navPartition);
                 snprintf(mediaStorage->volume[i]->metadataPartition, B_DVR_MAX_PARTITIONNAME,"%s",settings.metadataPartition);
                 mediaStorage->numRegisteredVolumes++;
             }
         }
         else
         {
            dvrStatus = B_DVR_INVALID_PARAMETER;
         }
     }
error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return dvrStatus;
}

/* TODO: this should be called from dvrfs when disconnected */
B_DVR_ERROR 
B_DVR_MediaStorage_UnregisterVolume(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex)
{
    unsigned err=B_DVR_SUCCESS;

    BSTD_UNUSED(volumeIndex);
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    B_Mutex_Lock(mediaStorage->mlock);

#if 0
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    if (mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("can't unregister: volume %d is mounted",volumeIndex));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* unregistering is deleting its volume */
    if (!mediaStorage->volume[volumeIndex]->registered) {
        BDBG_WRN(("volume %d is already unregistered",volumeIndex));
        err = B_DVR_SUCCESS;
        goto error;
    }
    mediaStorage->volume[volumeIndex]->registered = 0;
    /* clear partition information */
    BKNI_Memset(mediaStorage->volume[volumeIndex]->media,0,sizeof(B_DVR_MediaStoragePartition));
    BKNI_Memset(mediaStorage->volume[volumeIndex]->navigation,0,sizeof(B_DVR_MediaStoragePartition));
    BKNI_Memset(mediaStorage->volume[volumeIndex]->metadata,0,sizeof(B_DVR_MediaStoragePartition));
    mediaStorage->numRegisteredVolumes--;
    /* nothing to be done */

    B_Mutex_Unlock(mediaStorage->mlock);
    return B_DVR_SUCCESS;

error:	
#endif
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_MountVolume( B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int         ret;
    char mountname[B_DVR_MAX_MOUNTNAME];

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_INDEX;
        goto error;
    }

    if(!mediaStorage->volume[volumeIndex]->registered) {
        BDBG_ERR(("MediaVolume %d is not registered", volumeIndex));
        err = B_DVR_NOT_REGISTERED;
        goto error;
    }

    if(mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("MediaVolume %d is already mounted", volumeIndex));
        err = B_DVR_ALREADY_MOUNTED;
        goto error;
    }

    if(!mediaStorage->volume[volumeIndex]->formatted) {
        BDBG_ERR(("MediaVolume %d need to be formatted", volumeIndex));
        err = B_DVR_NOT_FORMATTED;
        goto error;
    }

    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        ret = ms_mount_volume(mediaStorage->volume[volumeIndex]->deviceName,mountname, &mediaStorage->volume[volumeIndex]->deletedCount);
    }
    else
    {
        ret = ms_mount_virtual_volume(mediaStorage->volume[volumeIndex]->mediaPartition,
                                      mediaStorage->volume[volumeIndex]->navPartition,
                                      mediaStorage->volume[volumeIndex]->metadataPartition,
                                      mountname, &mediaStorage->volume[volumeIndex]->deletedCount);
    }
    if (ret!=eMS_OK) {
        BDBG_ERR(("mount failed"));
        err = B_DVR_SYSTEM_ERR;
        goto error;
    }
    snprintf(mediaStorage->volume[volumeIndex]->volName, B_DVR_MAX_VOLNAME,"%s",basename(mountname));
    snprintf(mediaStorage->volume[volumeIndex]->mountName, B_DVR_MAX_MOUNTNAME,"%s",mountname);
    BDBG_MSG(("mountname %s, volName %s",mountname,mediaStorage->volume[volumeIndex]->volName));
    mediaStorage->volume[volumeIndex]->mounted = 1;
    mediaStorage->numMountedVolumes++;

    BDBG_MSG(("mounted MediaVolume %d", volumeIndex));

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_MountVolumeAsync (B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex)
{
    B_ThreadHandle mountThread;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    B_DVR_ThreadParam *pMountParam;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_MSG(("%s",__FUNCTION__));

    pMountParam = BKNI_Malloc(sizeof(B_DVR_ThreadParam));
    if (!pMountParam) {
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }

    pMountParam->mediaStorage = mediaStorage;
    pMountParam->volumeIndex = volumeIndex;

    mountThread = B_Thread_Create("Mount Thread", B_DVR_P_MediaStorage_MountThread,pMountParam, NULL);
    if (!mountThread) {
        BDBG_ERR(("%s scheduler thread create error", __FUNCTION__));
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }

error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_UnmountVolume(B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int         ret;
    char        mountname[B_DVR_MAX_MOUNTNAME];

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    if(!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("MediaVolume %d is not mounted", volumeIndex));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    snprintf(mountname, B_DVR_MAX_MOUNTNAME,"/mnt/%s", mediaStorage->volume[volumeIndex]->volName);
    ret = ms_unmount_volume(mountname);
    if (ret!=eMS_OK) {
        err = B_DVR_SYSTEM_ERR;
        goto error;
    } 
    mediaStorage->volume[volumeIndex]->mounted = 0;
    mediaStorage->volume[volumeIndex]->mountName[0] = '\0';
    mediaStorage->numMountedVolumes--;
   
    BDBG_MSG(("Unmounted MediaVolume %d", volumeIndex));
    
error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;	
}


B_DVR_ERROR 
B_DVR_MediaStorage_CheckVolume(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, B_DVR_MediaStorageVolumeStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BSTD_UNUSED(mediaStorage);
    BSTD_UNUSED(volumeIndex);
    BSTD_UNUSED(pStatus);

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return B_DVR_SUCCESS;
}

B_DVR_ERROR 
B_DVR_MediaStorage_FormatVolume (B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int         ret;
    unsigned long segments;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) { /* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_INDEX;
        goto error;
    }

    if(!mediaStorage->volume[volumeIndex]->registered) {
        BDBG_ERR(("MediaVolume %d is not registered", volumeIndex));
        err = B_DVR_NOT_REGISTERED;
        goto error;
    }

    /* check if mounted */
    if (mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("already mounted. should not be mounted to format"));
        err = B_DVR_ALREADY_MOUNTED;
        goto error;
    }

    /* not going to store file system information now*/
    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        ret = ms_format_virtual_volume(mediaStorage->volume[volumeIndex]->mediaPartition,
                                       mediaStorage->volume[volumeIndex]->navPartition,
                                       mediaStorage->volume[volumeIndex]->metadataPartition,
                                       &segments);
    }
    else
    {
        ret = ms_format_volume(mediaStorage->volume[volumeIndex]->deviceName, &segments);
    }
    if (ret!=eMS_OK) {
        err= B_DVR_SYSTEM_ERR;
        goto error;
    }
    
    BDBG_MSG(("MediaVolume %d format complete. %lu segments", volumeIndex, segments));
    mediaStorage->volume[volumeIndex]->formatted = 1;
    mediaStorage->volume[volumeIndex]->mediaSegTotal = (uint32_t)segments;

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}


B_DVR_ERROR 
B_DVR_MediaStorage_FormatVolumeAsync (B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex)
{
    B_ThreadHandle formatThread;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    B_DVR_ThreadParam *pFormatParam;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_MSG(("%s",__FUNCTION__));

    pFormatParam = BKNI_Malloc(sizeof(B_DVR_ThreadParam));
    if (!pFormatParam) {
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }

    pFormatParam->mediaStorage = mediaStorage;
    pFormatParam->volumeIndex = volumeIndex;

    formatThread = B_Thread_Create("Format Thread", B_DVR_P_MediaStorage_FormatThread,pFormatParam, NULL);
    if (!formatThread) {
        BDBG_ERR(("%s scheduler thread create error", __FUNCTION__));
        err = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }
    
error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}


/* count segments and return it. -1 if wrong type given */
static int B_DVR_P_MediaStorage_CountSegments(const char *path)
{
    DIR *pdir;    
    struct dirent *pent;
    int counter=0;

    pdir = opendir(path);
    if (!pdir) {
        fprintf(stderr,"%s opendir error:%s", path, strerror(errno));
        return -1;
    }
    while ((pent = readdir(pdir))) {
        if (!fnmatch("seg*", pent->d_name, 0)) {	/* found */
            counter++;
        }                
    }

    closedir(pdir);

    return counter;
}

B_DVR_ERROR 
B_DVR_MediaStorage_GetStatus(B_DVR_MediaStorageHandle mediaStorage, B_DVR_MediaStorageStatus *pStatus)
{
    unsigned i;
    char path[B_DVR_MAX_FILE_NAME_LENGTH];

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    BKNI_Memset(pStatus, 0, sizeof(B_DVR_MediaStorageStatus));
    pStatus->numRegisteredVolumes = mediaStorage->numRegisteredVolumes;
    pStatus->numMountedVolumes = mediaStorage->numMountedVolumes;
    pStatus->maxVolumeNumber = B_DVR_MEDIASTORAGE_MAX_VOLUME;

   
    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) {
        strncpy(pStatus->volumeInfo[i].name, mediaStorage->volume[i]->volName,B_DVR_MAX_VOLNAME-1);
        strncpy(pStatus->volumeInfo[i].device, mediaStorage->volume[i]->deviceName,B_DVR_MAX_DEVICENAME-1);
        strncpy(pStatus->volumeInfo[i].mountName, mediaStorage->volume[i]->mountName,B_DVR_MAX_MOUNTNAME-1);

        pStatus->volumeInfo[i].startSector = mediaStorage->volume[i]->startSector;
        pStatus->volumeInfo[i].endSector = mediaStorage->volume[i]->endSector;        
        pStatus->volumeInfo[i].formatted = mediaStorage->volume[i]->formatted;
        pStatus->volumeInfo[i].mounted = mediaStorage->volume[i]->mounted;
        pStatus->volumeInfo[i].registered = mediaStorage->volume[i]->registered;
        snprintf(pStatus->volumeInfo[i].mediaPartition, B_DVR_MAX_PARTITIONNAME,"%s",mediaStorage->volume[i]->mediaPartition);
        snprintf(pStatus->volumeInfo[i].navPartition, B_DVR_MAX_PARTITIONNAME,"%s",mediaStorage->volume[i]->navPartition);
        snprintf(pStatus->volumeInfo[i].metadataPartition, B_DVR_MAX_PARTITIONNAME,"%s",mediaStorage->volume[i]->metadataPartition);


        if(mediaStorage->volume[i]->registered) {
            pStatus->volumeInfo[i].mediaSegmentSize = B_DVR_MEDIA_SEGMENT_SIZE;
            pStatus->volumeInfo[i].mediaSegTotal = mediaStorage->volume[i]->mediaSegTotal;
        } else {
            pStatus->volumeInfo[i].mediaSegmentSize = 0;
            pStatus->volumeInfo[i].mediaSegTotal = 0;
        }

        if(mediaStorage->volume[i]->mounted) {
            snprintf(path, B_DVR_MAX_FILE_NAME_LENGTH,"%s-media/record",mediaStorage->volume[i]->mountName);
            pStatus->volumeInfo[i].mediaSegInuse = B_DVR_P_MediaStorage_CountSegments(path);
        } else {
            pStatus->volumeInfo[i].mediaSegInuse = 0;
        }
        pStatus->volumeInfo[i].deletedCount = mediaStorage->volume[i]->deletedCount;
    }
    
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return B_DVR_SUCCESS;
}

void 
B_DVR_MediaStorage_GetNames(B_DVR_MediaStorageHandle mediaStorage, B_DVR_MediaStorageStatus *pStatus)
{
    unsigned i;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    BKNI_Memset(pStatus, 0, sizeof(B_DVR_MediaStorageStatus));
    pStatus->numRegisteredVolumes = mediaStorage->numRegisteredVolumes;
    pStatus->numMountedVolumes = mediaStorage->numMountedVolumes;
    pStatus->maxVolumeNumber = B_DVR_MEDIASTORAGE_MAX_VOLUME;
   
    for (i=0; i<B_DVR_MEDIASTORAGE_MAX_VOLUME; i++) {
        strncpy(pStatus->volumeInfo[i].name, mediaStorage->volume[i]->volName, B_DVR_MAX_VOLNAME-1);
        strncpy(pStatus->volumeInfo[i].device, mediaStorage->volume[i]->deviceName, B_DVR_MAX_DEVICENAME-1);
        strncpy(pStatus->volumeInfo[i].mountName, mediaStorage->volume[i]->mountName, B_DVR_MAX_MOUNTNAME-1);
    }

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return;
}

B_DVR_ERROR 
B_DVR_MediaStorage_GetSettings(B_DVR_MediaStorageHandle mediaStorage, B_DVR_MediaStorageSettings *pSettings)
{
    B_DVR_ERROR ret = B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
#ifdef LABEL_SUPPORT
    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        BKNI_Memcpy(pSettings,&mediaStorage->settings,sizeof(B_DVR_MediaStorageSettings));
    }
    else
    {
        BDBG_ERR(("label support not available for loop devices based VSFS"));
        ret = B_DVR_INVALID_PARAMETER;
    }
#else
    BSTD_UNUSED(pSettings);
#endif
    return ret;
}

B_DVR_ERROR 
B_DVR_MediaStorage_SetSettings(B_DVR_MediaStorageHandle mediaStorage, B_DVR_MediaStorageSettings *pSettings)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
#ifdef LABEL_SUPPORT
    int ret;
#endif

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

#ifdef LABEL_SUPPORT
    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        if (pSettings->labelSupport) 
        {
            ret = ms_set_label((int)true, pSettings->label, pSettings->labelSize);
        } 
        else 
        {
            ret = ms_set_label((int)false,(unsigned char*)0,0);
        }
        if (ret==eMS_OK) 
        {
            BKNI_Memcpy(&mediaStorage->settings, pSettings,sizeof(B_DVR_MediaStorageSettings));        
        }
        else 
        {
            err = B_DVR_SYSTEM_ERR;
        }
    }
    else
    {
        BDBG_ERR(("label support not available for loop devices based VSFS"));
        err = B_DVR_INVALID_PARAMETER;
    }
#else
    BSTD_UNUSED(pSettings);
#endif 

    B_Mutex_Unlock(mediaStorage->mlock);
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_InstallCallback(B_DVR_MediaStorageHandle mediaStorage, B_DVR_ServiceCallback callback, void * appContext)
{
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);
    mediaStorage->callback = callback;
    mediaStorage->appContext = appContext;

    BDBG_MSG(("callback: %08x", callback));
    B_Mutex_Unlock(mediaStorage->mlock);

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return B_DVR_SUCCESS;
}

B_DVR_ERROR 
B_DVR_MediaStorage_RemoveCallback(B_DVR_MediaStorageHandle mediaStorage)
{
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);
    mediaStorage->callback = NULL;
    mediaStorage->appContext = NULL;
    B_Mutex_Unlock(mediaStorage->mlock);

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return B_DVR_SUCCESS;
}

B_DVR_ERROR 
B_DVR_MediaStorage_AllocateMediaSegment(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char *subDir, char *pSegment)
{
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret = eMS_OK;
    MS_DirQuota_t dirQuota;
    unsigned refCnt, used;
    int quotaIsSet = 0;
    
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        if (ret==eMS_OK) quotaIsSet = 1;
    }

    if (quotaIsSet) {
        if (dirQuota.quota<=(dirQuota.used+dirQuota.shared)) {    /* full */
            BDBG_ERR(("err: quota if full"));
            err = B_DVR_MEDIASEG_FULL;
            goto error;
        }
    }

    /* destination path for media segment */
    snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/record/", mediaStorage->volume[volumeIndex]->volName);    

    if (!quotaIsSet) /* root dir or quota not set */
    {
        snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/depot/", mediaStorage->volume[volumeIndex]->volName);
    } else {
        /* search for a file in preallocated */
        snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH, "/mnt/%s-media/preallocated/", mediaStorage->volume[volumeIndex]->volName);
        err = B_DVR_P_MediaStorage_IncreaseMediaSegmentUsedQuota(mediaStorage,volumeIndex,subDir,&used);
        if (err!=B_DVR_SUCCESS) {
            goto error;
        }        
    }

    /* search for a segment and move */
    err = B_DVR_P_MediaStorage_FindAndMoveTsSegment(srcpath,destpath,pSegment);
    if (err != B_DVR_SUCCESS) {
        snprintf(pSegment, B_DVR_MAX_FILE_NAME_LENGTH,"-");  
        BDBG_ERR(("No more free segments"));
        err = B_DVR_MEDIASEG_FULL;
        goto error;
    }

#if DEBUG_SEG
    BDBG_WRN(("%s : %s",__FUNCTION__,pSegment)); 
#endif
    err = B_DVR_P_MediaStorage_IncreaseMediaSegmentReferenceCount(mediaStorage,volumeIndex,pSegment,&refCnt);
    if (err!=B_DVR_SUCCESS) {
        goto error;
    }

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;

}


B_DVR_ERROR 
B_DVR_MediaStorage_AllocateNavSegment(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, char *pSegment)
{
    DIR *pdir;
    struct dirent *pent;
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    int found=0;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret;
    MS_Registry_t *registry;
    char segnumStr[10];
    unsigned segnum;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* source path for media segment */
    snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-nav/depot/", mediaStorage->volume[volumeIndex]->volName);

    /* destination path for media segment */
    snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-nav/record/", mediaStorage->volume[volumeIndex]->volName);

    /* search for a file in depot */
    pdir = opendir(srcpath);
    while ((pent = readdir(pdir))) {
        if (!fnmatch("seg*nav", pent->d_name, 0)) {  /* found */
            /* move to destination */
            snprintf(pSegment, B_DVR_MAX_FILE_NAME_LENGTH,"%s",pent->d_name);    /* give filie name only */
            strncat(srcpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(srcpath));
            strncat(destpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(destpath));

            if(rename(srcpath,destpath)<0) {
                BDBG_ERR(("%s->%s failed:%s",srcpath,destpath,strerror(errno)));
                err = B_DVR_SYSTEM_ERR;
                goto error_dir;
            }

            memset(segnumStr,0,10);
            strncpy(segnumStr, pSegment+3, 6);
            segnum = (unsigned)atoi(segnumStr);

            registry = BKNI_Malloc(sizeof(MS_Registry_t));
            if (!registry) {
                BDBG_ERR(("malloc failed"));
                err = B_DVR_SYSTEM_ERR;
                goto error_dir;
            }
            ret = ms_read_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
            if (ret!=eMS_OK) {
                BKNI_Free(registry);
                err = B_DVR_SYSTEM_ERR;
                goto error_dir;
            }
            
            registry->navSegRefCount[segnum]++;

#if DEBUG_SEG
            BDBG_WRN(("%s : %s",__FUNCTION__,pSegment)); 
            
            BDBG_WRN(("increase refcount for %s to %d",pSegment, registry->navSegRefCount[segnum]));
#else
            BDBG_MSG(("increase refcount for %s to %d",pSegment, registry->navSegRefCount[segnum]));
#endif
            ret = ms_update_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
            if (ret!=eMS_OK) {
                BKNI_Free(registry);
                BDBG_ERR(("ms_update_registry failed %d", ret));
                err = B_DVR_SYSTEM_ERR;
                goto error_dir;
            }
            BKNI_Free(registry);
            BDBG_MSG(("%s -> %s", srcpath, destpath)); 
            found = 1;
            break;
        }
    }

    /* free segment not found */
    if (!found) {
        snprintf(pSegment, B_DVR_MAX_FILE_NAME_LENGTH,"-");	/* return null file name */
        err = B_DVR_NAVSEG_FULL;
        goto error_dir;
    }

error_dir:
    closedir(pdir);

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;

}

    
B_DVR_ERROR 
B_DVR_MediaStorage_FreeMediaSegment(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char *subDir, const char pSegment[])
{
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_ERROR err = B_DVR_SUCCESS;
    unsigned refCnt=0, shared, used;
    MS_DirQuota_t dirQuota;
    int ret=eMS_OK;
    int quotaIsSet = 0;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

#if DEBUG_SEG
    BDBG_WRN(("%s : %s",__FUNCTION__,pSegment)); 
#endif
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    err = B_DVR_P_MediaStorage_DecreaseMediaSegmentReferenceCount(mediaStorage,volumeIndex,pSegment,&refCnt);
    if (err) {
        BDBG_ERR(("B_DVR_P_MediaStorage_DecreaseMediaSegmentReferenceCount failed %d",err));
        goto error;
    }

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        if (ret==eMS_OK) quotaIsSet = 1;
    }

    /* if reference count != 0, segment will stay in record */
    if (refCnt) {
        if (quotaIsSet) {
            err = B_DVR_P_MediaStorage_DecreaseMediaSegmentSharedQuota(mediaStorage,volumeIndex,subDir,&shared);
            if (err) {
                BDBG_ERR(("B_DVR_P_MediaStorage_DecreaseMediaSegmentSharedQuota failed %d",err));
            }
        }
        goto error; // returning success
    }

    /* if reference count == 0, move to depot or preallocated*/
    BDBG_MSG(("refcount 0. freeing segment %s",pSegment));

    /* source path for media segment */
    snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/record/", mediaStorage->volume[volumeIndex]->volName);

    /* search for a file in source, record */

    if (quotaIsSet) {
        /* move to preallocated */ 
        snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/preallocated/", mediaStorage->volume[volumeIndex]->volName);
        B_DVR_P_MediaStorage_DecreaseMediaSegmentUsedQuota(mediaStorage,volumeIndex,subDir,&used);
    } else { 
        /* move to depot */
        snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/depot/", mediaStorage->volume[volumeIndex]->volName);
    }

    err = B_DVR_P_MediaStorage_MoveSegment(srcpath,destpath,pSegment);
    /* segment file not found */
    if (err!=B_DVR_SUCCESS) {
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_FreeNavSegment(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char *pSegment)
{
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_ERROR err = B_DVR_SUCCESS;
    uint8_t refCnt;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

#if DEBUG_SEG
    BDBG_WRN(("%s : %s",__FUNCTION__,pSegment)); 
#endif
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    err = B_DVR_P_MediaStorage_DecreaseNavReferenceCount(mediaStorage,volumeIndex,pSegment,&refCnt);
    if (err) {
        BDBG_ERR(("B_DVR_P_MediaStorage_DecreaseNavReferenceCount failed %d",err));
        goto error;
    }        
    if (refCnt) { /* ref count not 0 */
        goto error; /* return success */
    }

    /* move file if refcnt is 0 */

    /* dest path for navigation segment */
    snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-nav/depot/", mediaStorage->volume[volumeIndex]->volName);

    /* destination path for navigation segment */
    snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-nav/record/", mediaStorage->volume[volumeIndex]->volName);

    err = B_DVR_P_MediaStorage_MoveSegment(srcpath,destpath,pSegment);

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_GetMediaPath(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, char *path)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;

/*    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));    */

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    snprintf(path, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/record",mediaStorage->volume[volumeIndex]->volName);

error:
/*    BDBG_TRACE(("-------- %s",__FUNCTION__)); */
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_GetNavPath(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, char *path)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;

/*    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));    */

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) { /* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    snprintf(path, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-nav/record",mediaStorage->volume[volumeIndex]->volName);

error:
/*    BDBG_TRACE(("-------- %s",__FUNCTION__)); */
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_GetMetadataPath(B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, char *path)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;

/*    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));    */

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    /* check if mounted */
    if (!mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("err: volume not mounted"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    snprintf(path, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-metadata",mediaStorage->volume[volumeIndex]->volName);

error:
/*    BDBG_TRACE(("-------- %s",__FUNCTION__)); */
    return err;
}

B_DVR_ERROR
B_DVR_MediaStorage_GetVolumeIndex(B_DVR_MediaStorageHandle mediaStorage, const char* volName, unsigned *volIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int vol;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    /* check volume */
    vol = mediaStorage->numRegisteredVolumes;
    while(vol--) {
        if (!strcmp(mediaStorage->volume[vol]->volName, volName)) {
            break;
        }
    }

    if (vol<0) {
        BDBG_ERR(("invalid volume name"));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    *volIndex = vol;

error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}


B_DVR_ERROR B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, const char * seg)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    unsigned refCnt,shared;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

#if DEBUG_SEG
    BDBG_WRN(("%s : %s",__FUNCTION__,seg)); 
#endif
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
   
    B_Mutex_Lock(mediaStorage->mlock);
   
    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    err = B_DVR_P_MediaStorage_IncreaseMediaSegmentSharedQuota(mediaStorage,volumeIndex,subDir,&shared);
    if (err!=B_DVR_SUCCESS) {
        goto error;
    }

    err = B_DVR_P_MediaStorage_IncreaseMediaSegmentReferenceCount(mediaStorage,volumeIndex,seg, &refCnt);
    if (err!=B_DVR_SUCCESS) {
        goto error;
    }
error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR B_DVR_MediaStorage_IncreaseNavSegmentRefCount(
        B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,const char * seg)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    char segnumStr[10];
    unsigned segnum;
    int ret;
    MS_Registry_t *registry;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

#if DEBUG_SEG
    BDBG_WRN(("%s : %s",__FUNCTION__,seg)); 
#endif
    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);
   
    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    memset(segnumStr,0,10);
    strncpy(segnumStr, seg+3, 6);
    segnum = (unsigned)atoi(segnumStr);

    if (segnum>=MS_MAX_SEGMENTS) {
        BDBG_ERR(("segnum bigger than max %u > %u", segnum, MS_MAX_SEGMENTS));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    registry = BKNI_Malloc(sizeof(MS_Registry_t));
    if (!registry) {
        BDBG_ERR(("malloc failed"));
        err = B_DVR_SYSTEM_ERR;;
        goto error;
    }

    ret = ms_read_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        err = B_DVR_SYSTEM_ERR;
        goto error;
    }

    registry->navSegRefCount[segnum]++;
#if DEBUG_SEG
    BDBG_WRN(("increase refcount for %s to %d",seg, registry->navSegRefCount[segnum]));
#else
    BDBG_MSG(("increase refcount for %s to %d",seg, registry->navSegRefCount[segnum]));
#endif
    ret = ms_update_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    BKNI_Free(registry);
    if (ret!=eMS_OK) {
        err = B_DVR_SYSTEM_ERR;
        goto error;
    }

error:
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

/*
   device valid:
      registered: volIndex same as current
      not registered: register. volIndex same as current 
   device not valid:
      registerd: unregister. assign volIndex 
      not registered: 
*/
B_DVR_ERROR 
B_DVR_MediaStorage_CheckDevice(B_DVR_MediaStorageHandle mediaStorage, const char *device, int *volIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret;
    MS_StorageStatus_t status;
    int volumeNum=0;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    B_Mutex_Lock(mediaStorage->mlock);

    for(volumeNum=0; volumeNum<MS_MAX_DVRVOLUME; volumeNum++) {
        if (!strcmp(mediaStorage->volume[volumeNum]->deviceName, device) ) {
            BDBG_WRN(("%s is already registered",device));
            err = B_DVR_SUCCESS;  /* already registered device, return success and pass vol index */
            *volIndex = volumeNum;
            goto error;
        }
    }

    ret = ms_check_volume(device,&status);
    if (ret!=eMS_OK ) {
        BDBG_ERR(("ms_check_volume error"));
        err = B_DVR_OS_ERROR;
        goto error;
    }

    for(volumeNum=0; volumeNum<MS_MAX_DVRVOLUME; volumeNum++) {
        if (mediaStorage->volume[volumeNum]->registered==0) 
            break;  /* found empty slot */
    }

    if (volumeNum==MS_MAX_DVRVOLUME) {
        BDBG_WRN(("device %s number of dvr volume full(%d)",device,MS_MAX_DVRVOLUME));
        *volIndex = -1;
        err = B_DVR_NOT_SUPPORTED;
        goto error; /* do nothing */
    }

    if (status.state==eMS_StateReady || status.state==eMS_StateEmpty) {
        BDBG_MSG(("device %s is a %s media volume\n",device,(status.state==eMS_StateEmpty)?"empty":"formatted"));
        mediaStorage->volume[volumeNum]->formatted = (status.state==eMS_StateEmpty)?0:1;
        mediaStorage->volume[volumeNum]->registered = 1;
        mediaStorage->numRegisteredVolumes++;            
        mediaStorage->volume[volumeNum]->startSector = status.startSector;
        mediaStorage->volume[volumeNum]->endSector = status.endSector;
        mediaStorage->volume[volumeNum]->mediaSegTotal = status.mediaSegTotal;
        strncpy(mediaStorage->volume[volumeNum]->mediaPartition, status.mediaPartition,B_DVR_MAX_PARTITIONNAME-1);
        strncpy(mediaStorage->volume[volumeNum]->navPartition, status.navPartition,B_DVR_MAX_PARTITIONNAME-1);
        strncpy(mediaStorage->volume[volumeNum]->metadataPartition, status.metadatapartition,B_DVR_MAX_PARTITIONNAME-1);
        snprintf(mediaStorage->volume[volumeNum++]->deviceName, B_DVR_MAX_DEVICENAME,"%s",device);
        *volIndex = volumeNum;
    } else {
        err = B_DVR_INVALID_PARAMETER;     /* not a valid media volume */
        goto error;
    }

    BDBG_WRN(("new device %s added",device));

error:
    B_Mutex_Unlock(mediaStorage->mlock);  
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

B_DVR_ERROR 
B_DVR_MediaStorage_UncheckDevice(B_DVR_MediaStorageHandle mediaStorage, const char *device, int *volIndex)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int volumeNum=0;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    for(volumeNum=0; volumeNum<MS_MAX_DVRVOLUME; volumeNum++) {
        if (!strcmp(mediaStorage->volume[volumeNum]->deviceName, device) ) {
            BDBG_MSG(("%s is registered, index %d",device,volumeNum));
            break;
        }
    }

    if (volumeNum==MS_MAX_DVRVOLUME) {
        BDBG_WRN(("%s not found",device));
        *volIndex = -1;
        goto error; /* do nothing */
    }

    B_DVR_MediaStorage_UnmountVolume(mediaStorage,volumeNum);

    B_Mutex_Lock(mediaStorage->mlock);  
    mediaStorage->numRegisteredVolumes--;            
    memset(mediaStorage->volume[volumeNum],0,sizeof(B_DVR_MediaStorageVolume));
    B_Mutex_Unlock(mediaStorage->mlock);  

    *volIndex = volumeNum;

    BDBG_WRN(("device %s removed",device));

    BDBG_TRACE(("-------- %s",__FUNCTION__));

error:
    return err;
}

B_DVR_ERROR B_DVR_MediaStorage_UpdateDirQuota(B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,const char *subDir, int segments)
{
    int ret;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    MS_DirQuota_t dirQuota;
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char segment[B_DVR_MAX_FILE_NAME_LENGTH];
    int i;
    int mediaCountDepot, mediaCountRecord, mediaCountPrealloc;
    int navCountDepot,navCountRecord;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
   
    B_Mutex_Lock(mediaStorage->mlock);

    ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    if (ret!=eMS_OK) {
        err = B_DVR_SUCCESS; /* quota is not set, nothing to do*/
        goto error;
    }
   
    BDBG_MSG(("Adding %d segments to quota", segments));

    if (segments>0) {
        if (dirQuota.quota<(dirQuota.used+(unsigned)segments)) {  /* check file size if it fits in dir quota */
            BDBG_ERR(("size to big. available segments:%d needed segments:%d",dirQuota.quota-dirQuota.used,segments));
            err = B_DVR_INVALID_PARAMETER;
            goto error;
        }
        
        dirQuota.used += (unsigned)segments;

        /* check segment availablitity in alloc */        
        ms_get_segnum(mediaStorage->volume[volumeIndex]->mountName,
            &mediaCountDepot, &mediaCountRecord, &mediaCountPrealloc,
            &navCountDepot,&navCountRecord);

        if (mediaCountPrealloc<segments) {   /* not enough segments */
            BDBG_ERR(("preallocated segment %d<moving %d",mediaCountDepot,(int)dirQuota.quota));
            err = B_DVR_UNKNOWN;    
            goto error;
        }

        /* move n segments from preallocated to depot*/
        for (i=0; i<segments;i++) {
            snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/preallocated/", mediaStorage->volume[volumeIndex]->volName);
            snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/depot/", mediaStorage->volume[volumeIndex]->volName);
            err = B_DVR_P_MediaStorage_FindAndMoveTsSegment(srcpath,destpath,segment);
            if(err!=B_DVR_SUCCESS) {
                BDBG_ERR(("B_DVR_P_MediaStorage_FindAndMoveTsSegment error %d", err));
                goto error;
            }
        }
    } else if (segments<0) {
        if (((int)dirQuota.used+segments) <0) {  
            BDBG_ERR(("Invalid number of segments used:%u, segments:%d", dirQuota.used, segments));
            err = B_DVR_INVALID_PARAMETER;
            goto error;
        }
        
        dirQuota.used += segments;

        /* check segment availablitity in depot*/        
        ms_get_segnum(mediaStorage->volume[volumeIndex]->mountName,
            &mediaCountDepot, &mediaCountRecord, &mediaCountPrealloc,
            &navCountDepot,&navCountRecord);

        segments *= -1;

        if (mediaCountDepot<(int)segments) {   /* not enough segments */
            BDBG_ERR(("preallocated segment %d<moving %d",mediaCountDepot,(int)dirQuota.quota));
            err = B_DVR_UNKNOWN;    
            goto error;
        }
        
        /* move n segments from depot to preallocated */
        for (i=0; i<segments;i++) {
            snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/depot/", mediaStorage->volume[volumeIndex]->volName);
            snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/preallocated/", mediaStorage->volume[volumeIndex]->volName);
            err = B_DVR_P_MediaStorage_FindAndMoveTsSegment(srcpath,destpath,segment);
            if(err!=B_DVR_SUCCESS) {
                BDBG_ERR(("B_DVR_P_MediaStorage_FindAndMoveTsSegment error %d", err));
                goto error;
            }
        }    
    }
        
    ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    if (ret!=eMS_OK) {
        err = B_DVR_SYSTEM_ERR;
        BDBG_ERR(("ms_update_quota error %d", ret));
        goto error;
    }

error:
    B_Mutex_Unlock(mediaStorage->mlock);  
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err; 
}


B_DVR_ERROR B_DVR_MediaStorage_SetDirQuota(B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,const char *subDir, uint64_t sizeKbytes)
{
    int ret;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    MS_DirQuota_t dirQuota;
    char srcpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char destpath[B_DVR_MAX_FILE_NAME_LENGTH];
    char segment[B_DVR_MAX_FILE_NAME_LENGTH];
    int i;
    int mediaCountDepot, mediaCountRecord, mediaCountPrealloc;
    int navCountDepot,navCountRecord;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
   
    B_Mutex_Lock(mediaStorage->mlock);

    ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    if (ret==eMS_OK) {
        err = B_DVR_NOT_SUPPORTED; /* cannot set again */
        goto error;
    }

    dirQuota.quota = (int)sizeKbytes/(B_DVR_MEDIA_SEGMENT_SIZE/1024)+1;
    dirQuota.used = 0;
    dirQuota.shared = 0;

    /* preallocate segments */
    /* check segment availablitity in depot */
    
    ms_get_segnum(mediaStorage->volume[volumeIndex]->mountName,
        &mediaCountDepot, &mediaCountRecord, &mediaCountPrealloc,
        &navCountDepot,&navCountRecord);

    if (mediaCountDepot<(int)dirQuota.quota) {   /* not enough segments */
        BDBG_ERR(("available segment %d<quota %d",mediaCountDepot,(int)dirQuota.quota));
        err = B_DVR_MEDIASEG_FULL;
        goto error;
    }
    /* ok, let's move */

    for (i=0; i<dirQuota.quota;i++) {
        snprintf(srcpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/depot/", mediaStorage->volume[volumeIndex]->volName);
        snprintf(destpath, B_DVR_MAX_FILE_NAME_LENGTH,"/mnt/%s-media/preallocated/", mediaStorage->volume[volumeIndex]->volName);
        err = B_DVR_P_MediaStorage_FindAndMoveTsSegment(srcpath,destpath,segment);
        if(err!=B_DVR_SUCCESS) {
            goto error;
        }
    }
    
    ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    if (ret!=eMS_OK) {
        err = B_DVR_SYSTEM_ERR;
        goto error;
    }

error:
    B_Mutex_Unlock(mediaStorage->mlock);  
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err; 
}

B_DVR_ERROR B_DVR_MediaStorage_GetDirQuota(
      B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,
      const char *subDir, uint64_t *sizeKbytes, uint64_t *freeKbytes)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret;
    MS_DirQuota_t dirQuota;
    
    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
   
    B_Mutex_Lock(mediaStorage->mlock);

    ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    if (ret!=eMS_OK) {
        err = B_DVR_SYSTEM_ERR;
        goto error;
    }

    *sizeKbytes = (uint64_t)(dirQuota.quota)*(B_DVR_MEDIA_SEGMENT_SIZE/1024);
    *freeKbytes = (uint64_t)(dirQuota.quota-dirQuota.used)*(B_DVR_MEDIA_SEGMENT_SIZE/1024);
    BDBG_MSG(("quota sizeKbytes = %llu",*sizeKbytes));
    BDBG_MSG(("quota freeKbytes = %llu",*freeKbytes));

error:
    B_Mutex_Unlock(mediaStorage->mlock);  
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err; 
}


static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseNavReferenceCount(
            B_DVR_MediaStorageHandle mediaStorage,unsigned volumeIndex,const char * seg, uint8_t *refCnt)
{
    char segnumStr[10];
    unsigned segnum;
    int ret;
    MS_Registry_t *registry;

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);
   
    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        return B_DVR_INVALID_PARAMETER;
    }

    /* check file name */
    if (strlen(seg)!=strlen("seg000000.nav")) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;        
    }       

    if (strncmp(seg,"seg",3) || strncmp(seg+9,".nav",3)) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;        
    }       

    memset(segnumStr,0,10);
    strncpy(segnumStr, seg+3, 6);
    segnum = (unsigned)atoi(segnumStr);

    if (segnum>=MS_MAX_SEGMENTS) {
        BDBG_ERR(("segnum bigger than max %u > %u", segnum, MS_MAX_SEGMENTS));
        return B_DVR_INVALID_PARAMETER;
    }

    registry = BKNI_Malloc(sizeof(MS_Registry_t));
    if (!registry) {
        BDBG_ERR(("malloc failed"));
        return B_DVR_SYSTEM_ERR;
    } 

    ret = ms_read_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    registry->navSegRefCount[segnum]--;
#if DEBUG_SEG
    BDBG_WRN(("decrease refcount for %s to %d",seg, registry->navSegRefCount[segnum]));
#else
    BDBG_MSG(("decrease refcount for %s to %d",seg, registry->navSegRefCount[segnum]));
#endif
    ret = ms_update_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    *refCnt = registry->navSegRefCount[segnum];
    BKNI_Free(registry);

    return B_DVR_SUCCESS;
} 

static void B_DVR_P_MediaStorage_CallbackThread(void *context)
{
    B_DVR_MediaStorageHandle mediaStorage = context;
    B_DVR_CallbackMessage callbackMessage;
    size_t receivedSize;
    B_Error berr;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    while(1) 
    {
        if(callbackMessage.event == eB_DVR_EventMediaStorage_CallbackStop) 
        {
            break;
        }
        berr = B_MessageQueue_Wait(mediaStorage->cbQueue,(void *)&callbackMessage,
                    sizeof(B_DVR_CallbackMessage),&receivedSize,B_WAIT_FOREVER);
        if (berr!=B_ERROR_SUCCESS) continue;

        if (mediaStorage->callback) {
            BDBG_MSG(("callback exec: %08x:%08x,%d,%d,%d", mediaStorage->callback, mediaStorage->appContext, 
                    callbackMessage.index, callbackMessage.event, eB_DVR_ServiceStorage ));    
            mediaStorage->callback(mediaStorage->appContext, callbackMessage.index,
                    callbackMessage.event, eB_DVR_ServiceStorage);
        }       
   }
 
    BDBG_TRACE(("-------- %s",__FUNCTION__));
}


void 
B_DVR_P_MediaStorage_MountThread(void *context)
{  
    char mountname[B_DVR_MAX_MOUNTNAME];
    B_DVR_ThreadParam *pParam;
    B_DVR_MediaStorageHandle mediaStorage;
    unsigned volumeIndex;
    B_DVR_CallbackMessage callbackMessage;
    int ret;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    pParam = (B_DVR_ThreadParam*)context;
    mediaStorage = pParam->mediaStorage;
    volumeIndex = pParam->volumeIndex;

    BKNI_Free(context);

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    callbackMessage.index = volumeIndex;
    callbackMessage.event = eB_DVR_EventMountSuccss;
    
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        callbackMessage.event = eB_DVR_EventMountFail_InvalidIndex;  
        goto error;
        }

    if(!mediaStorage->volume[volumeIndex]->registered) {
        BDBG_ERR(("MediaVolume %d is not registered", volumeIndex));
        callbackMessage.event = eB_DVR_EventMountFail_NotRegistered;  
        goto error;
    }
    
    if(mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("MediaVolume %d is already mounted", volumeIndex));
        callbackMessage.event = eB_DVR_EventMountFail_VolumeMounted;
        goto error;
        }

    if(!mediaStorage->volume[volumeIndex]->formatted) {
        BDBG_ERR(("MediaVolume %d need to be formatted", volumeIndex));
        callbackMessage.event = eB_DVR_EventMountFail_NotFormatted; 
        goto error;
        }

    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeBlockDevice) 
    {
        ret = ms_mount_volume(mediaStorage->volume[volumeIndex]->deviceName,mountname, &mediaStorage->volume[volumeIndex]->deletedCount);
    }
    else
    {
        ret = ms_mount_virtual_volume(mediaStorage->volume[volumeIndex]->mediaPartition,
                                      mediaStorage->volume[volumeIndex]->navPartition,
                                      mediaStorage->volume[volumeIndex]->metadataPartition,
                                      mountname, &mediaStorage->volume[volumeIndex]->deletedCount);
    }

    if (ret==eMS_ERR_WRONG_LABEL) {
        BDBG_ERR(("mount failed"));
        callbackMessage.event = eB_DVR_EventMountFail_WrongLabel;
        goto error;
    } else if (ret!=eMS_OK) {
        BDBG_ERR(("mount failed"));
        callbackMessage.event = eB_DVR_EventMountFail_SystemErr;
        goto error;
    }
    snprintf(mediaStorage->volume[volumeIndex]->volName, B_DVR_MAX_VOLNAME,"%s",basename(mountname));
    snprintf(mediaStorage->volume[volumeIndex]->mountName, B_DVR_MAX_MOUNTNAME,"%s",mountname);
    BDBG_MSG(("mountname %s, volName %s",mountname,mediaStorage->volume[volumeIndex]->volName));
    mediaStorage->volume[volumeIndex]->mounted = 1;
    mediaStorage->numMountedVolumes++;

    BDBG_MSG(("mounted MediaVolume %d", volumeIndex));
    
error:
    B_MessageQueue_Post(mediaStorage->cbQueue,(void *)(&callbackMessage),sizeof(B_DVR_CallbackMessage));
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return;
}

void  
B_DVR_P_MediaStorage_FormatThread (void *context)
{
    B_DVR_ThreadParam *pParam;
    B_DVR_MediaStorageHandle mediaStorage;
    unsigned volumeIndex;
    B_DVR_CallbackMessage callbackMessage;
    unsigned long segments;
    int ret;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    pParam = (B_DVR_ThreadParam*)context;
    mediaStorage = pParam->mediaStorage;
    volumeIndex = pParam->volumeIndex;

    BKNI_Free(context);

    BDBG_OBJECT_ASSERT(mediaStorage, B_DVR_MediaStorage);

    B_Mutex_Lock(mediaStorage->mlock);

    callbackMessage.index = volumeIndex;
    callbackMessage.event = eB_DVR_EventFormatSuccess;

    /* check volume */
    if (volumeIndex >= B_DVR_MEDIASTORAGE_MAX_VOLUME) {	/* invalid index */
        BDBG_ERR(("invalid volume."));
        callbackMessage.event = eB_DVR_EventFormatFail_InvalidIndex;
        goto error;
    }

    if(!mediaStorage->volume[volumeIndex]->registered) {
        BDBG_ERR(("MediaVolume %d is not registered", volumeIndex));
        callbackMessage.event = eB_DVR_EventFormatFail_NotRegistered;
        goto error;
    }

    /* check if mounted */
    if (mediaStorage->volume[volumeIndex]->mounted) {
        BDBG_ERR(("already mounted. should not be mounted to format"));
        callbackMessage.event = eB_DVR_EventFormatFail_VolumeMounted; 
        goto error;
    }

    B_Mutex_Unlock(mediaStorage->mlock);
    if(mediaStorage->storageType == eB_DVR_MediaStorageTypeLoopDevice) 
    {
        ret = ms_format_virtual_volume(mediaStorage->volume[volumeIndex]->mediaPartition,
                                       mediaStorage->volume[volumeIndex]->navPartition,
                                       mediaStorage->volume[volumeIndex]->metadataPartition,
                                       &segments);
    }
    else
    {
        ret = ms_format_volume(mediaStorage->volume[volumeIndex]->deviceName, &segments);
    }
    B_Mutex_Lock(mediaStorage->mlock);    
    if (ret!=eMS_OK) {
        callbackMessage.event = eB_DVR_EventFormatFail_SystemErr;
        goto error;
    }

    mediaStorage->volume[volumeIndex]->formatted = 1;
    mediaStorage->volume[volumeIndex]->mediaSegTotal = (uint32_t)segments;
    BDBG_MSG(("MediaVolume %d format complete. %lu segments", volumeIndex, segments));
error:
    B_MessageQueue_Post(mediaStorage->cbQueue,(void *)(&callbackMessage),sizeof(B_DVR_CallbackMessage));
    B_Mutex_Unlock(mediaStorage->mlock);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return;
}


static B_DVR_ERROR B_DVR_P_MediaStorage_ScanDisk(B_DVR_MediaStorageHandle mediaStorage)
{
    char cmd[B_DVR_MAX_FILE_NAME_LENGTH];
    FILE *fp;
    int  fd;
    char device[B_DVR_MAX_DEVICENAME];
    char path[B_DVR_MAX_FILE_NAME_LENGTH];
    int ret;
    MS_StorageStatus_t status;
    int volumeNum=0;

    snprintf(cmd, B_DVR_MAX_FILE_NAME_LENGTH,B_DVR_SCAN_DISK_CMD,B_DVR_SCAN_TEMP_FILE);
    ret = system(cmd);

    if (ret<0) {
        BDBG_ERR(("system error: %s",strerror(errno)));
        return B_DVR_OS_ERROR;
    }

    fp = fopen(B_DVR_SCAN_TEMP_FILE,"r"); /* add flag O_CLOEXEC */
    if (!fp) {
        BDBG_ERR(("open %s failed:%s",B_DVR_SCAN_TEMP_FILE,strerror(errno)));
        return B_DVR_OS_ERROR;
    }

    fd = fileno(fp);
    if (fd == -1) {
        BDBG_ERR(("fileno %s failed:%s",B_DVR_SCAN_TEMP_FILE,strerror(errno)));
        fclose(fp);
        return B_DVR_OS_ERROR;
    }
    
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
        BDBG_ERR(("fcntl %s failed:%s",B_DVR_SCAN_TEMP_FILE,strerror(errno)));
        fclose(fp);
        return B_DVR_OS_ERROR;
    }

    while(1){
        memset(device,0,B_DVR_MAX_DEVICENAME);
        ret = fscanf(fp,"%*d %*d %s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d",device);
        if (ret<0) break;
        
        BDBG_MSG(("device %s found\n",device));
        snprintf(path, B_DVR_MAX_FILE_NAME_LENGTH,"/dev/%s",device);
        ret = ms_check_volume(path,&status);
        if (ret!=eMS_OK ) {
            BDBG_ERR(("ms_check_volume error"));
            fclose(fp);
            return B_DVR_OS_ERROR;
        }

        if (status.state==eMS_StateReady || status.state==eMS_StateEmpty) {
            BDBG_MSG(("device %s is a %s media volume\n",path,(status.state==eMS_StateEmpty)?"empty":"formatted"));
            mediaStorage->volume[volumeNum]->formatted = (status.state==eMS_StateEmpty)?0:1;
            mediaStorage->volume[volumeNum]->registered = 1;
            mediaStorage->volume[volumeNum]->startSector = status.startSector;
            mediaStorage->volume[volumeNum]->endSector = status.endSector;
            mediaStorage->volume[volumeNum]->mediaSegTotal = status.mediaSegTotal;
            mediaStorage->numRegisteredVolumes++;            
            strncpy(mediaStorage->volume[volumeNum]->mediaPartition, status.mediaPartition,B_DVR_MAX_PARTITIONNAME-1);
            strncpy(mediaStorage->volume[volumeNum]->navPartition, status.navPartition,B_DVR_MAX_PARTITIONNAME-1);
            strncpy(mediaStorage->volume[volumeNum]->metadataPartition, status.metadatapartition,B_DVR_MAX_PARTITIONNAME-1);
            snprintf(mediaStorage->volume[volumeNum++]->deviceName, B_DVR_MAX_DEVICENAME,"%s",path);
            if (volumeNum==MS_MAX_DVRVOLUME) break; /* reached max */
        }
    }   

    fclose(fp);

    return B_DVR_SUCCESS;
}


static B_DVR_ERROR B_DVR_P_MediaStorage_FindAndMoveTsSegment(char *srcpath, char *destpath, char *pSegment)
{
    DIR *pdir;
    struct dirent *pent;
    B_DVR_ERROR err=B_DVR_SYSTEM_ERR;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    pdir = opendir(srcpath);
    while ((pent = readdir(pdir))) {
        if (!fnmatch("seg*ts", pent->d_name, 0)) {  /* found */
            /* move to destination */
            snprintf(pSegment, B_DVR_MAX_FILE_NAME_LENGTH,"%s",pent->d_name);    /* give file name only */

            strncat(srcpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(srcpath));
            strncat(destpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(destpath));

            if(rename(srcpath,destpath)<0) {
                BDBG_ERR(("%s->%s failed:%s",srcpath,destpath,strerror(errno)));
                err = B_DVR_SYSTEM_ERR;
                goto error_dir;
            }

            BDBG_MSG(("%s -> %s", srcpath, destpath));            
            err = B_DVR_SUCCESS;
            break;
        }
    }

error_dir:
    closedir(pdir);
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_MoveSegment(char *srcpath, char *destpath, const char *pSegment)
{
    B_DVR_ERROR err = B_DVR_SUCCESS;

    strncat(srcpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(srcpath));
    strncat(destpath,pSegment,B_DVR_MAX_FILE_NAME_LENGTH-strlen(destpath));

            if(rename(srcpath,destpath)<0) {
                BDBG_ERR(("%s->%s failed:%s",srcpath,destpath,strerror(errno)));
                err = B_DVR_SYSTEM_ERR;
            }

    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentReferenceCount(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char * seg, unsigned *count)
{
    char segnumStr[10];
    unsigned segnum;
    int ret=eMS_OK;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    MS_Registry_t *registry;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check file name */
    if (strlen(seg)!=strlen("seg000000.ts")) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;        
    }       

    if (strncmp(seg,"seg",3) || strncmp(seg+9,".ts",3)) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;
    }

    memset(segnumStr,0,10);
    strncpy(segnumStr, seg+3, 6);
    segnum = (unsigned)atoi(segnumStr);

    if (segnum>=MS_MAX_SEGMENTS) {
        BDBG_ERR(("segnum bigger than max %u > %u", segnum, MS_MAX_SEGMENTS));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    registry = BKNI_Malloc(sizeof(MS_Registry_t));
    if (!registry) {
        BDBG_ERR(("malloc failed"));
        goto error;
    }

    ret = ms_read_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    registry->mediaSegRefCount[segnum]++;
    *count = registry->mediaSegRefCount[segnum];
#if DEBUG_SEG
    BDBG_WRN(("increase refcount for %s to %d",seg, registry->mediaSegRefCount[segnum]));
#else
    BDBG_MSG(("increase refcount for %s to %d",seg, registry->mediaSegRefCount[segnum]));
#endif
    ret = ms_update_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    BKNI_Free(registry);
    return B_DVR_SUCCESS;

error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}


static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentReferenceCount(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex, const char * seg, unsigned *count)
{
    char segnumStr[10];
    unsigned segnum;
    int ret=eMS_OK;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    MS_Registry_t *registry;

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check file name */
    if (strlen(seg)!=strlen("seg000000.ts")) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;        
    }       

    if (strncmp(seg,"seg",3) || strncmp(seg+9,".ts",3)) {
        BDBG_ERR(("invalid segment file name %s",seg));
        return B_DVR_INVALID_PARAMETER;
    }

    memset(segnumStr,0,10);
    strncpy(segnumStr, seg+3, 6);
    segnum = (unsigned)atoi(segnumStr);

    if (segnum>=MS_MAX_SEGMENTS) {
        BDBG_ERR(("segnum bigger than max %u > %u", segnum, MS_MAX_SEGMENTS));
        err = B_DVR_INVALID_PARAMETER;
        goto error;
    }

    registry = BKNI_Malloc(sizeof(MS_Registry_t));
    if (!registry) {
        BDBG_ERR(("malloc failed"));
        goto error;
    }

    ret = ms_read_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    registry->mediaSegRefCount[segnum]--;
    *count = registry->mediaSegRefCount[segnum];
#if DEBUG_SEG
    BDBG_WRN(("decrease refcount for %s to %d",seg, registry->mediaSegRefCount[segnum]));
#else
    BDBG_MSG(("decrease refcount for %s to %d",seg, registry->mediaSegRefCount[segnum]));
#endif
    ret = ms_update_registry(mediaStorage->volume[volumeIndex]->mountName,registry);
    if (ret!=eMS_OK) {
        BKNI_Free(registry);
        return B_DVR_SYSTEM_ERR;
    }

    BKNI_Free(registry);
    return B_DVR_SUCCESS;

error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentSharedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count)
{
    MS_DirQuota_t dirQuota;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret;    

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        /* update quota */
        if (ret==eMS_OK) { /* quota is set */
            if (dirQuota.quota <= (dirQuota.shared+dirQuota.used)) {
                err = B_DVR_MEDIASEG_FULL;
                goto error;
            }
            dirQuota.shared++;
            *count = dirQuota.shared;
            ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
            if (ret!=eMS_OK) 
                err = B_DVR_SYSTEM_ERR;
        }
    } 

error:
    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentSharedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count)
{
    MS_DirQuota_t dirQuota;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret = eMS_ERR_SYSTEM;    

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    }    
    /* update quota */
    if (ret==eMS_OK) { /* quota is set */
        dirQuota.shared--;
        *count = dirQuota.shared;
        ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        if (ret!=eMS_OK) 
            err = B_DVR_SYSTEM_ERR;
    }

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_IncreaseMediaSegmentUsedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned *count)
{
    MS_DirQuota_t dirQuota;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret = eMS_ERR_SYSTEM;    

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    }    
    /* update quota */
    if (ret==eMS_OK) {  /* quota is set */
        dirQuota.used++;
        *count = dirQuota.used;
        ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        if (ret!=eMS_OK) 
            err = B_DVR_SYSTEM_ERR;
    }

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}

static B_DVR_ERROR B_DVR_P_MediaStorage_DecreaseMediaSegmentUsedQuota(
        B_DVR_MediaStorageHandle mediaStorage, unsigned volumeIndex,const char *subDir, unsigned  *count)
{
    MS_DirQuota_t dirQuota;
    B_DVR_ERROR err = B_DVR_SUCCESS;
    int ret = eMS_ERR_SYSTEM;    

    BDBG_TRACE(("++++++++ %s ",__FUNCTION__));

    /* check quota */
    if (subDir!=NULL && subDir[0]) {
        ret = ms_read_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
    }    
    /* update quota */
    if (ret==eMS_OK) {  /* quota is set */
        dirQuota.used--;
        *count = dirQuota.used;        
        ret = ms_update_quota(mediaStorage->volume[volumeIndex]->mountName,subDir,&dirQuota);
        if (ret!=eMS_OK) 
            err = B_DVR_SYSTEM_ERR;
    }

    BDBG_TRACE(("-------- %s",__FUNCTION__));
    return err;
}
