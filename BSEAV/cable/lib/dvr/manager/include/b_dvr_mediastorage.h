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
 *            VIRTUAL SEGMENTED FILE SYSTEM  (VSFS)
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_STORAGE_H
#define _B_DVR_STORAGE_H

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
     
#ifdef __cplusplus
     extern "C" {
#endif

#define B_DVR_MAX_VOLNAME        10
#define B_DVR_MAX_DEVICENAME     256
#define B_DVR_MAX_MOUNTNAME      30

/* 
 * partition names are usually shorter, but 
 * loop devices linked to pre-allocated files 
 * to depict partitions can have longer path 
 */
#define B_DVR_MAX_PARTITIONNAME  256
#define B_DVR_MAX_MESSAGES       20

/************************************************************************************
 Summary:
 B_DVR_MediaStorageHandle shall be a media storage context handle.
 ***********************************************************************************/
typedef struct B_DVR_MediaStorage *B_DVR_MediaStorageHandle;

/**********************************************************************************
Summmary: 
B_DVR_MediaStorageType depicts either a loop or block device. 
Block devices are used in SFS and loop devices are used in VSFS. 
***********************************************************************************/
typedef enum B_DVR_MediaStorageType
{
    eB_DVR_MediaStorageTypeBlockDevice, /* segmented file system */
    eB_DVR_MediaStorageTypeLoopDevice,  /* virtual segmented file system */
    eB_DVR_MediaStorageTypeMax
}B_DVR_MediaStorageType;
/*******************************************************************************
Summary:
B_DVR_MediaStorageVolume shall encapsulate all the storage partitions (SFS)
or all the pre-allocated files depicting the 
partitions (VSFS) and acts as a virtual media device. 
Each volume is confined to a single physical storage device (SFS) or 
single directory (VSFS).There could be multiple volumes 
associated with unique devices or directories. 
********************************************************************************/

typedef struct B_DVR_MediaStorageVolume 
{
    char deviceName[B_DVR_MAX_DEVICENAME]; /* device name. i.e. /dev/sda (SFS) or directory name (VSFS)*/
    char volName[B_DVR_MAX_VOLNAME];   /* decided at mount time. dvr<index number> */
    char mountName[B_DVR_MAX_MOUNTNAME];   /* prefix of full mount path. e.g /mnt/dvr0 */
    char mediaPartition[B_DVR_MAX_PARTITIONNAME];    /* name of the partition device (SFS) or name of the pre-allocated media loop device (VSFS)*/
    char navPartition[B_DVR_MAX_PARTITIONNAME];      /* name of the partition device (SFS or name of the pre-allocated navigation loop device (VSFS)*/
    char metadataPartition[B_DVR_MAX_PARTITIONNAME]; /* name of the partition device (SFS) or name of the pre-allocated meta data loop device ()*/
    int registered;
    int formatted;          /* formatted if it has correct media folders and files */
    int mounted;
    uint64_t   startSector;                         /* starting sector of media partition (SFS) or would be zero for VSFS*/
    uint64_t   endSector;                           /* end sector of metadata partition (SFS) or meta data loop device (VSFS)*/
    uint32_t   mediaSegTotal;     /* total number of media segments */
    int  deletedCount;          /* number of recording deleted by housecleaning */    
}B_DVR_MediaStorageVolume;

/***************************************************************************
Summary:
B_DVR_MediaStorageRegisterVolumeSettings shall contain settings for 
creating new MediaVolume.
device - device name
startSec -starting sector number
length - size of the dvr volume to create in MBytes
***************************************************************************/

typedef struct B_DVR_MediaStorageRegisterVolumeSettings
{
    char device[B_DVR_MAX_DEVICENAME];
    uint64_t startSec; /* applicable only when storageType = eB_DVR_MediaStorageTypeBlockDevice;*/
    uint32_t length; /* in mega bytes*/
} B_DVR_MediaStorageRegisterVolumeSettings;

/***************************************************************************
Summary:
B_DVR_MediaStorageOpenSettings shall have media storage open settings. 
see  B_DVR_MediaStorageType definition
***************************************************************************/
typedef struct B_DVR_MediaStorageOpenSettings 
{
    B_DVR_MediaStorageType storageType;
} B_DVR_MediaStorageOpenSettings;

/***************************************************************************
Summary:
B_DVR_MediaStorageVolumeStatus shall have media volume integrity status
***************************************************************************/
typedef struct B_DVR_MediaStorageVolumeStatus 
{
    int partitionStatus;
    int folderStatus;
} B_DVR_MediaStorageVolumeStatus;

/**************************************************************************
Summary:
B_DVR_MediaStorageVolumeInfo shall have media volume information.
****************************************************************************/
typedef struct B_DVR_MediaStorageVolumeInfo
{
    int mounted;
    int registered;
    int formatted;
    char name[B_DVR_MAX_VOLNAME];   /* volume name */
    char device[B_DVR_MAX_DEVICENAME]; /* hdd device name (SFS) or directory name (VSFS)*/
    char mountName[B_DVR_MAX_MOUNTNAME];   /* volume name */
    uint32_t mediaSegmentSize;  /* media segment size */ 
    uint32_t mediaSegTotal;     /* total number of media segments */
    uint32_t mediaSegInuse;     /* number of media segments in use */
    char mediaPartition[B_DVR_MAX_PARTITIONNAME];    /* name of the partition device or loop device*/
    char navPartition[B_DVR_MAX_PARTITIONNAME];      /* name of the partition device or loop device */
    char metadataPartition[B_DVR_MAX_PARTITIONNAME]; /* name of the partition device or loop device*/
    uint64_t   startSector;                         /* starting sector of media partition (SFS) or zero (VSFS) */
    uint64_t   endSector;                           /* end sector of metadata partition or meta data loop device*/
    int  deletedCount;          /* number of recording deleted by housecleaning */
} B_DVR_MediaStorageVolumeInfo;

/****************************************************************************
Summary:
B_DVR_MediaStorageStatus shall have the media volume info and 
media volume status.
*****************************************************************************/
typedef struct B_DVR_MediaStorageStatus 
{
	int numMountedVolumes;
	int numRegisteredVolumes;
	int maxVolumeNumber;
    B_DVR_MediaStorageVolumeInfo volumeInfo[B_DVR_MEDIASTORAGE_MAX_VOLUME];
} B_DVR_MediaStorageStatus;

/*****************************************************************************
Summary:
B_DVR_MediaStorageSettings shall have the settings for a media storage. 
This is applicable only for SFS. 
******************************************************************************/
typedef struct B_DVR_MediaStorageSettings 
{
    bool labelSupport;
    unsigned char *label;       /* pointer to label data */
    unsigned int labelSize;     /* size of the label data */
} B_DVR_MediaStorageSettings;

/***************************************************************************
Summary: 
B_DVR_MediaStorage_GetDefaultOpenSettings provides default open 
settings 
***************************************************************************/
void B_DVR_MediaStorage_GetDefaultOpenSettings(
    B_DVR_MediaStorageOpenSettings *pOpenSettings);

/***************************************************************************
Summary:
B_DVR_MediaStorage_Open shall open a media storage context. 
In the case of SFS, if a storage device is already 
registered, then media storage will detect it automatically through 
proc file system. 
In the case of VSFS, the storage directory will have to be 
registered everytime the application restarts. Automatic detection 
isn't done. 
Param[in] 
pOpenSettings - media storage open settings.
Param[out]
B_DVR_MediaStorageHandle - media storage handle.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_MediaStorageHandle B_DVR_MediaStorage_Open(
    const B_DVR_MediaStorageOpenSettings *pOpenSettings);

/***************************************************************************
Summary:
B_DVR_MediaStorage_Close shall close a media storage context.
Param[in]
mediaStorage - media storage handle.
return value
B_DVR_ERROR
***************************************************************************/
void B_DVR_MediaStorage_Close(
    B_DVR_MediaStorageHandle mediaStorage);

/***************************************************************************
Summary:
B_DVR_MediaStorage_RegisterVolume shall register a MediaVolume
Param[in]
mediaStorage - Handle for media storage.
Param[in]
pSettings - Volume registration settings.
Param[out]
volumeIndex - index of registered volume 
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_RegisterVolume(
    B_DVR_MediaStorageHandle mediaStorage, 
    B_DVR_MediaStorageRegisterVolumeSettings *pSettings,
    unsigned *volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_UnregisterVolume shall Unregister a MediaVolume
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume to be unregistered.
return value
B_DVR_ERROR
***************************************************************************/

B_DVR_ERROR B_DVR_MediaStorage_UnregisterVolume(
    B_DVR_MediaStorageHandle handle, 
    unsigned volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_MountVolume shall mount a mediaVolume 
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
Param[in]
path - path where the volume shall be mounted.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_MountVolume(
    B_DVR_MediaStorageHandle handle,
    unsigned volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_MountVolumeAsync shall mount a mediaVolume asyncronously
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
Param[in]
path - path where the volume shall be mounted.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_MountVolumeAsync(
    B_DVR_MediaStorageHandle handle,
    unsigned volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_UnmountVolume shall unmount a mediaVolume
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_UnmountVolume(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_CheckVolume shall check media volume integrity
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
Param[out]
pStatus - Status of the volume returned.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_CheckVolume(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    B_DVR_MediaStorageVolumeStatus *pStatus);

/***************************************************************************
Summary:
B_DVR_MediaStorage_FormatVolume shall format a MediaVolume.
Media partitions will be formatted and media folders will be created 
in media partition. Empty Media files will be created in depot media folder.
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
Parame[in]
pSettings - Settings for formatting various partitions in a volume.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_FormatVolumeAsync (
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex);

/***************************************************************************
Summary:
B_DVR_MediaStorage_FormatVolume shall format a MediaVolume.
Media partitions will be formatted and media folders will be created 
in media partition. Empty Media files will be created in depot media folder.
Param[in]
mediaStorage - Handle for media storage.
Param[in]
volumeIndex - Index of the volume.
Parame[in]
pSettings - Settings for formatting various partitions in a volume.
return value
B_DVR_ERROR
***************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_FormatVolume (
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex);
  
/*******************************************************************************
Summary:
Get current status information of registered volumes 
Param[in]
mediaStorage - Handle for media storage.
Param[out]
pStatus - status of the media storage returned.
return value
B_DVR_ERROR
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_GetStatus(
    B_DVR_MediaStorageHandle mediaStorage, 
    B_DVR_MediaStorageStatus *pStatus);

/*******************************************************************************
Summary:
Get current status information of registered volumes 
Param[in]
mediaStorage - Handle for media storage.
Param[out]
pStatus - status of the media storage returned.
return value
B_DVR_ERROR
********************************************************************************/
void 
B_DVR_MediaStorage_GetNames(
    B_DVR_MediaStorageHandle mediaStorage, 
    B_DVR_MediaStorageStatus *pStatus);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetSettings shall get the settings of a media storage context
This is applicable only for SFS.
Param[in]
mediaStorage - Handle for media storage.
Param[out]
pSettings - Settings of media storage returned 
return value
B_DVR_ERROR
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_GetSettings(
    B_DVR_MediaStorageHandle mediaStorage, 
    B_DVR_MediaStorageSettings *pSettings);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_SetSettings set the settings of a media storage context. 
This is applicable only for SFS. 
Param[in]
mediaStorage - Handle for media storage.
Param[in]
pSettings - Settings for media storage.
return value
B_DVR_ERROR
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_SetSettings(
    B_DVR_MediaStorageHandle mediaStorage, 
    B_DVR_MediaStorageSettings *pSettings);


/***************************************************************************
Summary:
B_DVR_MediaStorage_InstallCallback shall install the application provided
callback for the media storage context.
Param[in]
mediaStorage - handle for mediaStorage.
Param[in]
callback - Application provided function pointer.
Param[in]
appContext - Application context pointer.
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_InstallCallback(
    B_DVR_MediaStorageHandle mediaStorage,
    B_DVR_ServiceCallback callback,
    void * appContext);

/***************************************************************************
Summary:
B_DVR_MediaStorage_RemoveCallback shall remove the application provided
callback from a media storage instance.
Param[in]
mediaStorage - Handle for mediaStorage.
return value
B_DVR_ERROR
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_RemoveCallback(
    B_DVR_MediaStorageHandle mediaStorage);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_AllocateMediaSegment shall allocate a media segment for a Record
service instance when requested by moving a file from depot folder to Record folder accordingly.
Internal reference count will be increased.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume to allocate
Param[out]
file - name of the segmented file.
return value
B_DVR_ERROR - return value.
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_AllocateMediaSegment(
    B_DVR_MediaStorageHandle mediaStorage,     
    unsigned volumeIndex,
    const char *subDir,
	char *pSegment);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_FreeMediaSegment shall free a media segment from segmented
recording by moving a file from Record folder to depot folder accordingly.
intenal reference count will be decreased.
Free only when the reference count is 0.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume to free
Param[in]
file - name of the segmented file.
return value: 
B_DVR_ERROR.
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_FreeMediaSegment(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
        const char *subDir,
	const char pSegment[]);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_AllocateNavSegment shall allocate a navigation segment for a Record
service instance when requested by moving a file from depot folder to Record folder accordingly.
Internal reference count will be increased.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume to allocate
Param[out]
file - name of the segmented file.
return value
B_DVR_ERROR - return value.
********************************************************************************/
B_DVR_ERROR B_DVR_MediaStorage_AllocateNavSegment(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
    char *pSegment);


/*******************************************************************************
Summary:
B_DVR_MediaStorage_FreeNavSegment shall free a navigation segment from segmented
recording by moving a file from Record folder to depot folder accordingly.
intenal reference count will be decreased.
Free only when the reference count is 0.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume to free
Param[in]
file - name of the segmented file.
return value: 
B_DVR_ERROR.
********************************************************************************/    
B_DVR_ERROR B_DVR_MediaStorage_FreeNavSegment(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
	const char *pSegment);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetMediaPath gives the path of the record folder of the media partition.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume 
Param[out]
file - path of the record folder of the volume
return value: 
B_DVR_ERROR.
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_GetMediaPath(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
	char *path);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetNavPath gives the path of the record folder of the nav partition
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume 
Param[out]
file - path of the metadata folder of the volume
return value: 
B_DVR_ERROR.
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_GetNavPath(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
	char *path);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetMetadataPath gives the path of the metadata folder of the volume.
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volumeIndex - Index of the volume 
Param[out]
file - path of the metadata folder of the volume
return value: 
B_DVR_ERROR.
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_GetMetadataPath(
	B_DVR_MediaStorageHandle mediaStorage, 
	unsigned volumeIndex, 
	char *path);

    
/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetHandle gives the handle of the mediastorage.
return value: 
B_DVR_MediaStorageHandle or NULL if failed.
********************************************************************************/  
B_DVR_MediaStorageHandle B_DVR_MediaStorage_GetHandle(
    void);


/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetVolumeIndex gives the volume Index from volume name.
Param[in]
volName  - name of the volume
Param[out]
volIndex - index of the volume
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_GetVolumeIndex(
    B_DVR_MediaStorageHandle mediaStorage, 
    const char* volName, 
    unsigned *volIndex);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_IncreaseMediaSegmentRefCount increases reference counter 
for the given segment
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volIndex - index of the volume
Param[in]
seg - name of the segment
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_IncreaseMediaSegmentRefCount(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    const char *subDir,
    const char *seg);

/*******************************************************************************
Summary:
B_DVR_MediaStorage_IncreaseNavSegmentRefCount increases reference counter 
for the given segment
Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volIndex - index of the volume
Param[in]
seg - name of the segment
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_IncreaseNavSegmentRefCount(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    const char *seg);
    
/*******************************************************************************
Summary:
B_DVR_MediaStorage_CheckDevice checks a device if it is a valid media volume.  
if valid, add volume information

Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
device - device name
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_CheckDevice(
    B_DVR_MediaStorageHandle mediaStorage, 
    const char *device,
    int *volIndex);

/*******************************************************************************
Summary:
remove volume information of a device

Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
device - device name
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_UncheckDevice(
    B_DVR_MediaStorageHandle mediaStorage, 
    const char *device, 
    int *volIndex);


/*******************************************************************************
Summary:
B_DVR_MediaStorage_UpdateDirQuota updates quota increamenting number of segments

Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volIndex - index of the volume
Param[in]
subDir - name of subdirectory
Param[in]
segments - number of segments to add
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_UpdateDirQuota(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    const char *subDir, 
    int segments);


/*******************************************************************************
Summary:
B_DVR_MediaStorage_SetDirQuota sets directory quota  

Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volIndex - index of the volume
Param[in]
subDir - name of subdirectory
Param[in]
sizeKbytes - quota size in Kbytes
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_SetDirQuota(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    const char *subDir, 
    uint64_t sizeKbytes);


/*******************************************************************************
Summary:
B_DVR_MediaStorage_GetDirQuota gets directory quota

Param[in]
mediaStorage - Handle for mediaStorage
Param[in]
volIndex - index of the volume
Param[in]
subDir - name of subdirectory
Param[out]
sizeKbytes - quota size in Kbytes
return value: 
B_DVR_ERROR
********************************************************************************/  
B_DVR_ERROR B_DVR_MediaStorage_GetDirQuota(
    B_DVR_MediaStorageHandle mediaStorage,
    unsigned volumeIndex,
    const char *subDir, 
    uint64_t *sizeKbytes,
    uint64_t *freeKbytes);

#ifdef __cplusplus
    }
#endif
    
#endif /*_B_DVR_STORAGE_H */

