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
#ifndef B_DVR_SEGMENTEDFILE_H__
#define B_DVR_SEGMENTEDFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_file.h"
#include "b_dvr_mediastorage.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_SegmentedFileNode);

/**********************************************************************************
Summary:
B_DVR_SegmentedFileNode shall have info on each segment in a segmented file. 
The total size of file node is made to be 512 bytes. At a later point direct IO can 
be enabled for reading the meta data files containing these file nodes. 
 *********************************************************************************/
typedef struct B_DVR_SegmentedFileNode
{
    BDBG_OBJECT(B_DVR_SegmentedFileNode)
    BLST_Q_ENTRY(B_DVR_SegmentedFileNode) nextFileNode;
    char fileName[B_DVR_MAX_FILE_NAME_LENGTH];
    off_t size;
    off_t linearStartOffset;
    off_t segmentOffset; 
    int index;
    unsigned refCount;
    B_DVR_Recording recordType;
    unsigned version;
    uint8_t reserved[200];
}B_DVR_SegmentedFileNode;

/************************************************************************************
Summary:
B_DVR_SegmentedFileEvent shall be used to enumerate all the 
events that could be generated during segmented file operation.
 ************************************************************************************/
typedef enum B_DVR_SegmentedFileEvent
{
    B_DVR_SegmentedFileEventReadError,
    B_DVR_SegmentedFileEventWriteError,
    B_DVR_SegmentedFileEventSeekError,
    B_DVR_SegmentedFileEventOpenError,
    B_DVR_SegmentedFileEventNotFoundError,
    B_DVR_SegmentedFileEventNoFileSpaceError,
    B_DVR_SegmentFileEventMax
}B_DVR_SegmentedFileEvent;

/************************************************************************************
Summary:
B_DVR_VideoFrameType shall be used to specify a type video frame
 ************************************************************************************/
typedef enum B_DVR_VideoFrameType
{
    eB_DVR_VideoFrameTypeI,
    eB_DVR_VideoFrameTypeP,
    eB_DVR_VideoFrameTypeB,
    eB_DVR_VideoFrameTypeMax
}B_DVR_VideoFrameType;

typedef void (*B_DVR_SegmentedFileCallback)(void * segmentedFileContext, B_DVR_SegmentedFileEvent event);
/**********************************************************************************
Summary:
B_DVR_SegmentedFileNode shall have info all the segments in a segmented file
 *********************************************************************************/
typedef struct B_DVR_SegmentedFile
{
    B_DVR_FileInterface fileInterface;
    B_DVR_FileIO fileIO;
    B_DVR_FileType fileType;
    B_DVR_File dvrFileMetaData;
    B_DVR_File dvrFile;
    B_DVR_SegmentedFileNode *pCachedFileNode;
    off_t linearCurrentOffset;
    off_t linearStartOffset;
    off_t linearEndOffset;
    unsigned long timeStamp; /*in milliseconds*/
    unsigned fileCount;
    unsigned fileIndex;
    unsigned maxFileCount;
    char fileName[B_DVR_MAX_FILE_NAME_LENGTH];
    unsigned refCnt;
    unsigned segmentedSize;
    B_DVR_Service service;
    unsigned serviceIndex;
    void * context;
    B_DVR_SegmentedFileCallback registeredCallback;
    unsigned volumeIndex;
    char *metaDataSubDir;
    char *tsbConvMetaDataSubDir;
    bool tsbConversion;
    unsigned itbThreshhold; /* This will be used for updating the media node periodically*/
    BLST_Q_HEAD(fileList, B_DVR_SegmentedFileNode) fileList;
    BLST_Q_HEAD(tsbFreeFileList, B_DVR_SegmentedFileNode) tsbFreeFileList;
    BKNI_MutexHandle metaDataFileMutex;
    B_DVR_MediaStorageHandle mediaStorage;
    struct B_DVR_SegmentedFile *tsbRecordFile;
    B_EventHandle event;
    off_t mediaLinearOffset; /* used only in deciding to switch to next nav segment */
}B_DVR_SegmentedFile;

/******************************************************************************
Summary:
B_DVR_SegmentedFileHandle shall be an internal handle for segmentedFile
*******************************************************************************/
typedef B_DVR_SegmentedFile *B_DVR_SegmentedFileHandle;


/******************************************************************************
Summary:
B_DVR_SegmentedFileSettings shall provide application specified
segment size and service type
*******************************************************************************/
typedef struct B_DVR_SegmentedFileSettings
{
    unsigned mediaSegmentSize;
    B_DVR_Service service;
    unsigned serviceIndex;
    char *metaDataSubDir;
    B_DVR_SegmentedFileCallback registeredCallback;
    B_DVR_MediaStorageHandle mediaStorage;
    unsigned volumeIndex;
    unsigned maxSegmentCount;
    B_EventHandle event;
    unsigned itbThreshhold;
    unsigned cdbSize;
    unsigned itbSize;
    unsigned long maxTSBTime; /*in ms*/
}B_DVR_SegmentedFileSettings;

/*************************************************************************************
 Summary:
B_DVR_PermanentRecord shall have the info on conversion of TSB to permanent recording.
 *************************************************************************************/
typedef struct B_DVR_PermanentRecord
{
    unsigned long recStartTime;
    unsigned long recCurrentTime;
    unsigned long recEndTime;
    char navFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char mediaFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    B_DVR_File mediaMetaDataFile;
    B_DVR_File navMetaDataFile;
    off_t recMediaStartOffset;
    off_t recMediaCurrentOffset;
    off_t recMediaEndOffset;
    off_t recNavStartOffset;
    off_t recNavCurrentOffset;
    off_t recNavEndOffset;
    B_DVR_SegmentedFileNode *pCachedNavFileNode;
    B_DVR_SegmentedFileNode *pCachedMediaFileNode;
    unsigned navFileCount;
    unsigned mediaFileCount;
    char *tsbConvMetaDataSubDir;
}B_DVR_PermanentRecord;

/******************************************************************************************
Summary:
B_DVR_SegmentedFileRecord shall contain hooks for io interface between nexus file and dvr 
segmented file.NEXUS  record module's io interfaces would be routed to the io interfaces 
for segmented files read, write, seek and tell etc.
*******************************************************************************************/
typedef struct B_DVR_SegmentedFileRecord
{
    struct NEXUS_FileRecord nexusFileRecord; 
    B_DVR_SegmentedFileHandle mediaWrite;
    B_DVR_SegmentedFileHandle navWrite;
    B_DVR_SegmentedFileHandle navRead;
    BNAV_Player_Handle navPlayer;
    BKNI_MutexHandle   navPlayerMutex;
    unsigned navEntrySize;
    unsigned cdbSize;
    unsigned itbSize;
    unsigned segmentNum;
    B_DVR_PermanentRecord permRec;
    unsigned long maxTSBTime; /* in ms */
}B_DVR_SegmentedFileRecord;

/*******************************************************************************************
Summary:
B_DVR_SegmentedFilePlay shall contain hooks for io interface between nexus file and dvr segmented file.
NEXUS  playback module's io interfaces would be routed to the io interfaces for segmented 
files read, write, seek and tell etc.
*********************************************************************************************/
typedef struct B_DVR_SegmentedFilePlay 
{
     struct NEXUS_FilePlay nexusFilePlay; 
     B_DVR_SegmentedFileHandle mediaRead;
     B_DVR_SegmentedFileHandle navRead;
     BNAV_Player_Handle navPlayer;
     BKNI_MutexHandle   navPlayerMutex;
     unsigned navEntrySize;
}B_DVR_SegmentedFilePlay;


/******************************************************************************
Summary:
B_DVR_SegmentedFilePlayHandle shall be an internal handle for segmentedFilePlay
*******************************************************************************/
typedef B_DVR_SegmentedFilePlay *B_DVR_SegmentedFilePlayHandle;

/******************************************************************************
Summary:
B_DVR_SegmentedFileRecord shall be an internal handle for segmentedFileRecord
*******************************************************************************/
typedef B_DVR_SegmentedFileRecord *B_DVR_SegmentedFileRecordHandle;


/************************************************************************************
Summary:
B_DVR_SegmentedFilePlaySettings shall be used to set the bounds for
nav and media segmented files using B_DVR_SegmentedFilePlay_SetSettings.
***********************************************************************************/
typedef struct B_DVR_SegmentedFilePlaySettings
{
    off_t navLinearStartOffset;
    off_t navLinearEndOffset;
    off_t mediaLinearStartOffset;
    off_t mediaLinearEndOffset;
    unsigned navEntrySize;
}B_DVR_SegmentedFilePlaySettings;


/************************************************************************************
Summary:
B_DVR_SegmentedFileRecordSettings shall be used to set the bounds for
nav and media segmented files using B_DVR_SegmentedFileRecord_SetSettings.
***********************************************************************************/
typedef struct B_DVR_SegmentedFileRecordSettings
{
    off_t navLinearStartOffset;
    off_t navLinearEndOffset;
    off_t mediaLinearStartOffset;
    off_t mediaLinearEndOffset;
    unsigned navEntrySize;
}B_DVR_SegmentedFileRecordSettings;

typedef struct B_DVR_SegmentedFileRecordAppendSettings
{
    char tempNavMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char navMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char tempMediaMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    char mediaMetaDataFileName[B_DVR_MAX_FILE_NAME_LENGTH];
    off_t tsbConvNavStartOffset;
    off_t tsbConvMediaStartOffset;
    off_t tsbConvNavCurrentOffset;
    off_t tsbConvMediaCurrentOffset;
    unsigned long tsbConvStartTime;
    off_t appendMediaOffset;
    off_t appendNavOffset;
    unsigned long appendTimeStamp;
    B_DVR_File readTempNavMetaDataFile;
    B_DVR_File readTempMediaMetaDataFile;
    B_DVR_File writeNavMetaDataFile;
    B_DVR_File writeMediaMetaDataFile;
    B_DVR_File writeNavFile;
    B_DVR_File readNavFile;
    B_DVR_SegmentedFileNode lastNavFileNode;
    B_DVR_SegmentedFileNode lastMediaFileNode;
    off_t navFileOffset;
    unsigned long mediaFileCount;
    unsigned long navFileCount;
    unsigned long tmpMediaFileCount;
    unsigned long tmpNavFileCount;
    unsigned navEntrySize;
    B_DVR_MediaStorageHandle mediaStorage;
    unsigned volumeIndex;
}B_DVR_SegmentedFileRecordAppendSettings;

/********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_Open shall open both  media file and  navigation files 
used for segmented playback.
*********************************************************************************/
NEXUS_FilePlayHandle B_DVR_SegmentedFilePlay_Open(
    const char *mediaFileName, 
    const char *navFileName, 
    B_DVR_SegmentedFileSettings *pSettings,
    NEXUS_FileRecordHandle nexusFileRecord);

/********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_Close shall close both media file and  navigation files
 used for segmented playback.
*********************************************************************************/
void B_DVR_SegmentedFilePlay_Close(NEXUS_FilePlayHandle);

/********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_Open shall open both media file and  navigation files
used for segmented recording.
*********************************************************************************/
NEXUS_FileRecordHandle B_DVR_SegmentedFileRecord_Open(
    const char *mediaFileName, 
    const char *navFileName, 
    B_DVR_SegmentedFileSettings *pSettings);

/********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_Close shall open both media file and  navigation files
used for segmented recording.
*********************************************************************************/
void B_DVR_SegmentedFileRecord_Close(
    NEXUS_FileRecordHandle nexusFileRecord);


/************************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetDefaultSettings shall be used to get the default
settings.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetDefaultSettings(
    B_DVR_SegmentedFilePlaySettings *pSettings);


/************************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetSettings shall be used to get the bounds for
nav and media segmented files.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetSettings(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_SegmentedFilePlaySettings *pSettings);

/************************************************************************************
Summary:
B_DVR_SegmentedFilePlay_SetSettings shall be used to set the bounds for
nav and media segmented files.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_SetSettings(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_SegmentedFilePlaySettings *pSettings);


/************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetDefaultSettings shall be used to get the default 
settings.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetDefaultSettings(
    B_DVR_SegmentedFileRecordSettings *pSettings);


/************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetSettings shall be used to set the nav entry size
for a recording based on the video codec type.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetSettings(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_SegmentedFileRecordSettings *pSettings);

/************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_SetSettings shall be used to set the nav entry size
for a recording based on the video codec type.
***********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_SetSettings(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_SegmentedFileRecordSettings *pSettings);

/********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetBounds shall be used to get the bounds
for a recording in segmented file format.
*********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetBounds(
    NEXUS_FileRecordHandle nexusFileRecord, 
    B_DVR_FilePosition *pFirst, 
    B_DVR_FilePosition *pLast);

/***********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetLocation shall return an offset based on an input timestamp 
provided in milliseconds or a nav index. If the nav index is negative, then 
timestamp would be used for getting the position info. 
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetLocation(
    NEXUS_FilePlayHandle filePlay, 
    long index,                 /* nav index of requested position */
    unsigned long timestamp,    /* Timestamp of requested position, units of milliseconds */
    B_DVR_FilePosition *pPosition);

/***********************************************************************************
Summary:
B_DVR_FileRecord_GetNextIFrame shall return IFrame positions and size based on the 
current position and direction for a TSB recording.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetNextIFrame(
    NEXUS_FileRecordHandle FileRecord, 
    B_DVR_FilePosition currentPosition,   
    B_DVR_FilePosition *iFramePosition,
    unsigned *sizeOfFrame,
    B_DVR_FileReadDirection direction);

/***********************************************************************************
Summary:
B_DVR_FilePlay_GetNextIFrame shall return IFrame positions and size based on the 
current position and direction for a permanent recording.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetNextIFrame(
    NEXUS_FilePlayHandle FilePlay, 
    B_DVR_FilePosition currentPosition,   
    B_DVR_FilePosition *iFramePosition,
    unsigned *sizeOfFrame,
    B_DVR_FileReadDirection direction);

/***********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_OpenPermMetaDataFile shall open the permanent recording
metaData files for TSB conversion.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_OpenPermMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord);

/***********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_ClosePermMetaDataFile shall close the permanent recording
metaData files for TSB conversion.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_ClosePermMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord);

/***********************************************************************************
Summary:
B_DVR_FileSegmentedRecord_CopyMetaDataFile shall copy the metaData files from
tsbService to the permanent metaData files.
************************************************************************************/
unsigned long B_DVR_FileSegmentedRecord_CopyMetaDataFile(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_FilePosition tsbStartPosition,
    B_DVR_FilePosition tsbEndPosition);

/***********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetLocation shall be used to return a byte offset 
for a recording based on an input time stamp in milliseconds or a nav index. 
If nav index provided is negative, then timestamp would be used for getting the 
position info. 
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetLocation(
    NEXUS_FileRecordHandle nexusFileRecord, 
    long index,                 /* nav index of requested position */
    unsigned long timestamp,    /* Timestamp of requested position, units of milliseconds */
    B_DVR_FilePosition *pPosition);

/***********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetTimestamp shall be used to return a byte offset 
for a playback based on the input time stamp in millisecond for a TSB recording.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_GetTimestamp(
    NEXUS_FileRecordHandle nexusFileRecord,
    off_t offset,
    unsigned long *timestamp);


/***********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetTimestamp shall be used to return a byte offset 
for a playback based on the input time stamp in milliseconds for a permanent recording.
************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetTimestamp(
    NEXUS_FilePlayHandle nexusFilePlay,
    off_t offset,
    unsigned long *timestamp);

/***********************************************************************************
Summary:
B_DVR_SegmentedFileRecord_GetNumOfFrames shall return number of specific type of
video frame between a given start and end byte offset for a TSB recording.
************************************************************************************/
int B_DVR_SegmentedFileRecord_GetNumOfFrames(
    NEXUS_FileRecordHandle nexusFileRecord,
    B_DVR_VideoFrameType,
    off_t startOffset,
    off_t endOffset);


/***********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetNumOfFrames  shall return number of specific type of
video frame between a given start and end byte offset for a permanent recording stream.
************************************************************************************/
int B_DVR_SegmentedFilePlay_GetNumOfFrames(
    NEXUS_FilePlayHandle nexusFilePlay,
    B_DVR_VideoFrameType,
    off_t startOffset,
    off_t endOffset);

/*************************************************************************************
Summary:
B_DVR_SegmentedFilePlay_UpdateFileList shall update the fileNode list for segmented
nav and media files during the playback of an in progress recording.
**************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_UpdateFileList(NEXUS_FilePlayHandle nexusFilePlay);

/******************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_Alloc shall pre-allocate the media and nav segments for a 
recording.
******************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_Alloc(const char *mediaFileName,
                                      const char *navFileName,
                                      B_DVR_SegmentedFileSettings *pSettings);

/******************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_Free shall free the media and nav segments for a 
recording.
******************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_Free(const char *mediaFileName,
                                      const char *navFileName,
                                      B_DVR_SegmentedFileSettings *pSettings);

/******************************************************************************************
Summary:
B_DVR_SegmentedFileRecord_Free shall print the media and nav segments' properties for a 
recording on the console
******************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFileRecord_Print(const char *mediaFileName,
                                           const char *navFileName,
                                           B_DVR_SegmentedFileSettings *pSettings);


/******************************************************************************************
Summary:
B_DVR_SegmentedFilePlay_SanityCheck shall check the validity of segmented files in 
both navigation and media meta data files. 
******************************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_SanityCheck(const char *mediaFileName,
                                        const char *navFileName,
                                        B_DVR_SegmentedFileSettings *pSettings);


/********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_GetBounds shall be used to get the bounds
for a recording in segmented file format during playback.
*********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_GetBounds(
    NEXUS_FilePlayHandle nexusFilePlay, 
    B_DVR_FilePosition *pFirst, 
    B_DVR_FilePosition *pLast);

/********************************************************************************
Summary:
B_DVR_SegmentedFilePlay_ResetCachedFileNode shall be used to reset the cached 
media/navigation file nodes and also close the associated file segments. 
This is required only for the TSB playback since the cached file Nodes 
may be freed during TSB wrapping. 
*********************************************************************************/
B_DVR_ERROR B_DVR_SegmentedFilePlay_ResetCachedFileNode(
    NEXUS_FilePlayHandle nexusFilePlay);

B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedNavFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings);
B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedNavMetaDataFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings);
B_DVR_ERROR B_DVR_SegmentedFileRecord_UpdateAppendedMediaMetaDataFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings);
B_DVR_ERROR B_DVR_SegmentedFileRecord_OpenAppendedPermMetaDataFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings);
B_DVR_ERROR B_DVR_SegmentedFileRecord_CloseAppendedPermMetaDataFile(
    B_DVR_SegmentedFileRecordAppendSettings *appendSettings);
#ifdef __cplusplus
}
#endif

#endif /* B_DVR_SEGMENTEDFILE_H__ */


