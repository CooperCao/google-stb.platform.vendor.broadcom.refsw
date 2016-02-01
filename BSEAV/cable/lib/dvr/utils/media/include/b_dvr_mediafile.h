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
 * Module Description:MediaFile utility shall provide 
 * 1. Streaming function - shall support streaming of segmented/non segmented 
 *     media from the server to the clients.
 * 2. Playback function - Media playback shall be supported by internally
 *    using a playback service instance.
 * Revision History:
 *
 * $brcm_Log: $
 * 
  ****************************************************************************/
#ifndef _B_DVR_MEDIAFILE_H
#define _B_DVR_MEDIAFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"
#include "b_dvr_file.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_MediaFile);
/*********************************************************************************
 B_DVR_MediaFileHandle shall be an unique identifier for a mediaFile
**********************************************************************************/
typedef struct B_DVR_MediaFile *B_DVR_MediaFileHandle;


/**********************************************************************************
Summary:
B_DVR_MediaFileSeekMode shall indicate media file seek operation mode for
streaming media from the server to the clients.
 ***********************************************************************************/
typedef enum B_DVR_MediaFileSeekMode
{
    eB_DVR_MediaFileSeekModeTimestampBased,
    eB_DVR_MediaFileSeekModeByteOffsetBased,
    eB_DVR_MediaFileSeekModeMax
}B_DVR_MediaFileSeekMode;

/********************************************************************************
Summary:
B_DVR_MediaFileReadMode shall enumerate file read modes.
**********************************************************************************/
typedef enum B_DVR_MediaFileReadMode
{
    eB_DVR_MediaFileReadModeFull,
    eB_DVR_MediaFileReadModeIFrame,
    eB_DVR_MediaFileReadModeIPFrame,
    eB_DVR_MediaFileReadModeMax
}B_DVR_MediaFileReadMode;

/********************************************************************************
Summary:
B_DVR_MediaFileDirection shall enumerate direction of file read;
**********************************************************************************/
typedef enum B_DVR_MediaFileDirection
{
    eB_DVR_MediaFileDirectionForward,
    eB_DVR_MediaFileDirectionReverse,
    eB_DVR_MediaFileDirectionMax
}B_DVR_MediaFileDirection;

/********************************************************************************
Summary:
B_DVR_MediaFile_Operation shall have information on direction and mode of file read
**********************************************************************************/
typedef struct B_DVR_MediaFileOperation
{
     B_DVR_MediaFileDirection direction;
     B_DVR_MediaFileReadMode readMode;
}B_DVR_MediaFileOperation;

/********************************************************************************
Summary:
B_DVR_MediaFilePlay_OpenSettings shall have open settings for media play
**********************************************************************************/
typedef struct B_DVR_MediaFilePlayOpenSettings
{
    NEXUS_VideoDecoderHandle videoDecoder[2];
    NEXUS_AudioDecoderHandle audioDecoder[2];
    unsigned activeVideoPidIndex[2];
    unsigned activeAudioPidIndex;
    NEXUS_StcChannelHandle stcChannel;
    unsigned playpumpIndex;
    unsigned volumeIndex;
    char subDir[B_DVR_MAX_FILE_NAME_LENGTH];
}B_DVR_MediaFilePlayOpenSettings;

/********************************************************************************
Summary:
B_DVR_MediaFileReadMode shall enumerate file open modes.
**********************************************************************************/
typedef enum B_DVR_MediaFileOpenMode
{
    eB_DVR_MediaFileOpenModeStreaming,
    eB_DVR_MediaFileOpenModePlayback,
    eB_DVR_MediaFileOpenModeRecord,
    eB_DVR_MediaFileOpenModeMax
}B_DVR_MediaFileOpenMode;

/********************************************************************************
Summary:
B_DVR_MediaFileSeekSettings shall indicate media file seek settings.
**********************************************************************************/
typedef struct B_DVR_MediaFileSeekSettings
{
    union mediaSeek
    {
        off_t offset;
        unsigned long time; /* time in seconds */
    }mediaSeek;
    B_DVR_MediaFileSeekMode seekmode;
}B_DVR_MediaFileSeekSettings;

/********************************************************************************
Summary:
B_DVR_MediaFile_Settings shall indicate media file settings.
fileOperation could be either for streaming or playback
**********************************************************************************/
typedef struct B_DVR_MediaFileSettings
{
    B_DVR_DRMServiceHandle drmHandle;
    B_DVR_MediaFileSeekSettings seekSetting;
    union fileOperaton
    {
         B_DVR_MediaFileOperation streamingOperation;
         B_DVR_OperationSettings  playbackOperation;
    }fileOperation;
    
}B_DVR_MediaFileSettings;


/***********************************************************************************
 Summary:
 B_DVR_MediaFilePlaySettings shall have the audio and video nexus handles to
 media file play. Application shall pass these handles in the B_DVR_MediaFile_PlayStart
 API.
 ***********************************************************************************/
typedef struct B_DVR_MediaFilePlaySettings
{
    unsigned activeAudioPID;
    unsigned activeVideoPID[2]; /*3D video requires 2 active video PIDs*/
    unsigned long startTime; /* in milliseconds */
    unsigned long endTime;  
 }B_DVR_MediaFilePlaySettings;

/***********************************************************************************
 Summary:
 B_DVR_MediaFileStats shall indicate media file stats.
 ***********************************************************************************/
typedef struct B_DVR_MediaFileStats
{
    off_t startOffset; 
    off_t endOffset;
    off_t currentOffset;
    unsigned long startTime;    /*time in milliseconds */
    unsigned long endTime;      /*time in milliseconds */
    unsigned long currentTime;  /*time in milliseconds */
    unsigned long moreData;
}B_DVR_MediaFileStats;

/***********************************************************************************
Summary: 
B_DVR_MediaFileMetaDataInfo shall have information required for creating media
info file for the media downloaded through network
*************************************************************************************/
typedef struct B_DVR_MediaFileMetaDataInfo
{
    bool encryptedMedia;
    NEXUS_TransportType transportType;
    unsigned keys[16];                 
    char mediaName[B_DVR_MAX_FILE_NAME_LENGTH];
}B_DVR_MediaFileMetaDataInfo;

/********************************************************************************
Summary:
B_DVR_MediaFileOpenMode shall enumerate file open modes.
**********************************************************************************/
typedef enum B_DVR_MediaFileFrameType
{
    B_DVR_MediaFileFrameTypeI,
    B_DVR_MediaFileFrameTypeP,
    B_DVR_MediaFileFrameTypeB,
    B_DVR_MediaFileFrameTypeMax
}B_DVR_MediaFileFrameType;

/******************************************************************************
Summary:
B_DVR_MediaFileChunkInfo shall have info on media data chunks 
read during streaming.
*******************************************************************************/
typedef struct B_DVR_MediaFileChunkInfo
{
   unsigned long   elements;
   unsigned long   remainingLength;
   unsigned long   startTime;
   unsigned long   duration;                     
   unsigned long   startPos;                     
}B_DVR_MediaFileChunkInfo;

/******************************************************************************
Summary:
B_DVR_MediaFileStreamFrameRate shall have current stream frame rate for achieving
trick mode operation
*******************************************************************************/
typedef struct B_DVR_MediaFileStreamFrameRate
{
   unsigned  sfRateNumerator;
   unsigned  sfRateDenominator;
}B_DVR_MediaFileStreamFrameRate;

/******************************************************************************
Summary:
B_DVR_MediaFileStreamFrameResource have info 
*******************************************************************************/
typedef struct B_DVR_MediaFileStreamFrameResource
{
   unsigned  long FrameRateptsdelta;
   unsigned  long FramesGOPBuf;
}B_DVR_MediaFileStreamFrameResource;

/*****************************************************************************
Summary:
B_DVR_MediaFile_Open shall open a media file (either a segmented or non
 segmneted media file).
Param[in] 
mediaName -  name of the media file name. It shall be the name of the
of the media info file created during recording or IP download.
Param[in]
openMode - is the file opened for streaming or playback?
return value
B_DVR_MediaFileHandle - A media file handle.
*****************************************************************************/
B_DVR_MediaFileHandle B_DVR_MediaFile_Open(
    const char * mediaName,
    B_DVR_MediaFileOpenMode openMode,
    B_DVR_MediaFilePlayOpenSettings *openSettings);

/*******************************************************************************
Summary:
B_DVR_MediaFile_Close shall close a media file.

Param[in]
B_DVR_MediaFileHandle - media file handle returned in B_DVR_MediaFile_Open.
return value
B_DVR_ERROR 
*******************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_Close(
    B_DVR_MediaFileHandle mediaFile);

/*******************************************************************************
Summary:
B_DVR_MediaFile_SetSettings shall set the setting for the media file to be read.
Param[In]
B_DVR_MediaFileHandle - media file handle 
Param[In]
pSettings - media file settings.
return value
B_DVR_ERROR 
********************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_SetSettings(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSettings *pSettings);

/*********************************************************************************
Summary:
B_DVR_MediaFile_GetSettings shall get the media file settings.
Param[in]
B_DVR_MediaFileHandle - media file handle 
Param[out]
B_DVR_MediaFileSettings - media file settings
return value
B_DVR_ERROR
***********************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetDefaultSettings(
    B_DVR_MediaFileSettings *pSettings);

/*********************************************************************************
Summary:
B_DVR_MediaFile_GetSettings shall get the media file read settings.
Param[in]
B_DVR_MediaFileHandle - media file handle 
Param[out]
B_DVR_MediaFileSettings - media file settings
return value
B_DVR_ERROR
***********************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetSettings(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSettings *pSettings);

/********************************************************************************
Summary: 
B_DVR_MediaFile_Read shall sequetially read the data from the media files based
on the B_DVR_MediaFileSettings.
Param[in]
mediaFileHandle - media file handle 
Param[in/out]
buffer - buffer pointer passed by the application and used by 
the dvr library to copy the data from the media file.
Param[in] - size of the media_bufer to be read.
return value
returns number of bytes read.
if this is less then size passed in, then either end of file occured
or an error occured.
*********************************************************************************/
ssize_t B_DVR_MediaFile_Read(
    B_DVR_MediaFileHandle mediaFile,
    unsigned *buffer, ssize_t size);

/********************************************************************************
Summary: 
B_DVR_MediaFile_Write shall sequetially write the data from the non segmented media files based
on the B_DVR_MediaFileSettings into segmented media files.
Param[in]
mediaFileHandle - media file handle 
Param[in]
buffer - buffer pointer passed by the application and used by 
the dvr library to copy the data from the media file.
Param[in] - size of the media_bufer to be read.
return value
returns number of bytes written
if this is less then size passed in, then there could be a problem in 
file IO or there is no storage space.
*********************************************************************************/
ssize_t B_DVR_MediaFile_Write(
    B_DVR_MediaFileHandle mediaFile,
    unsigned *buffer, ssize_t size);

/********************************************************************************
Summary: 
B_DVR_MediaFileSeek shall set the seek offset in a media file
either based on the offset into the file or based on the time stamps.
Param[in]
mediafile_handle - media file handle
Param[in]
mediafile_seeksettings - seeksettings carrying time stamps or offsets
return value
0 for success or -1 for failure
*********************************************************************************/
int B_DVR_MediaFile_Seek(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSeekSettings *seekSettings);

/********************************************************************************
Summary: 
B_DVR_MediaFile_Stats shall get the range of the media file
in terms start offset and end offset or start time or end time.
Param[in]
mediaFileHandle - media file handle
Param[in/out]
B_DVR_MediaFileStats - media file stats.
Param[out]
B_DVR_ERROR - return value
*********************************************************************************/

B_DVR_ERROR B_DVR_MediaFile_Stats(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileStats *stats);

/***********************************************************************************
Summary: 
B_DVR_MediaFile_Create shall create the media info file for the media 
that's downloaded through streaming via network.
************************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_Create(
    B_DVR_MediaFileMetaDataInfo *metaDataInfo);

/*****************************************************************************
Summary:
B_DVR_MediaFilePlayStart shall playback a media file (either a segmented or
 non segmneted media file).
Param[in] 
B_DVR_MediaFileHandle - media file handle returned in B_DVR_MediaFile_Open.
Param [out]
B_DVR_ERROR - return value
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_PlayStart(
    B_DVR_MediaFileHandle mediaFile, 
    B_DVR_MediaFilePlaySettings *playSettings);

/*******************************************************************************
Summary:
B_DVR_MediaFile_PlayStop shall stop playback of a media file.

Param[in]
B_DVR_MediaFile_Handle - media file handle returned in B_DVR_MediaFile_Open.
return value
B_DVR_ERROR - return value
*******************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_PlayStop(
    B_DVR_MediaFileHandle mediaFile);

/*****************************************************************************
Summary:
B_DVR_MediaFile_Delete shall delete a media file (either a segmented or
 non segmneted media file).
Param[in] 
mediaName -  name of the media file name. It shall be the name of the
of the media info file created during recording or IP download.
return value
B_DVR_ERROR 
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_Delete(
    B_DVR_MediaNodeSettings *mediaNodeSettings);

/*****************************************************************************
Summary:
B_DVR_MediaFile_GetTimeStamp will get time stamp for a given location
Param[in] 
mediaFile - media file handle
offset - location offset
Param[out] 
Timestamp - the corresponding time offset for the location
return value
B_DVR_ERROR 
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetTimeStamp
    ( B_DVR_MediaFileHandle mediaFile,
      off_t offset,
     unsigned long *timestamp);

/*****************************************************************************
Summary:
B_DVR_GetOffsetFromTimeStap will get time stamp from the gived offset location
Param[in] 
mediaFile - media file handle
Timestamp - the corresponding time offset for the location
Param[out] 
offset - location offset
return value
B_DVR_ERROR 
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetOffsetFromTimeStap
    ( B_DVR_MediaFileHandle mediaFile,
      off_t offset,
     unsigned long *timestamp);

/***********************************************************************************
Summary:
B_DVR_GetLocation shall return a location in the stream based on the input time stamp
Param[in] 
mediaFile - media file handle
timestamp - timestamp input (ms)
Param[out] 
position - file position (it will be always on a frame boundary) 
return value
B_DVR_ERROR 
************************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetLocation(
		B_DVR_MediaFileHandle mediaFile,
		unsigned long timestamp,
		B_DVR_FilePosition *position);

/***********************************************************************************
Summary:
B_DVR_GetNextFrame shall return next frame's position based on currentOffset 
in the mediaFile. 
Param[in] 
mediaFile - media file handle
Param[out] 
framePosition - frame position info
return value
B_DVR_ERROR 
************************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetNextFrame(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_FilePosition *framePosition);

/***********************************************************************************
Summary:
B_DVR_MediaFile_GetChunkData shall return all chunk data for one element
Param[in] 
mediaFile - media file handle
Param[out] 
elements: how many elements will be returned for chunk read
duration:  Total duration on this chunkread
remainlength: how much left on doing this chunkread
return value
B_DVR_ERROR 
************************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetChunkInfo(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileChunkInfo *chunkInfo
);

/*****************************************************************************
Summary:
B_DVR_MediaFile_GetNumOfFrames shall get number of frames (I,P or B)
within the start and end offset of a media.
Param[in] 
mediaFile - media file handle
Param[in] 
frameType - I or P or B
Param[in] 
startOffset
Param[in] 
endOffset
return value
Number of frames of type B_DVR_MediaFileFrameType
*****************************************************************************/
int B_DVR_MediaFile_GetNumOfFrames(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileFrameType frameType,
    off_t startOffset,
    off_t endOffset);

/*****************************************************************************
Summary:
B_DVR_MediaFile_TrickModeIPFrameSpeedRate shall skip how many I/IP frame to reach 
the trickmmde speed requirement
Param[in] 
mediaFile - media file handle
Param[in] 
skipStep - Skip How many I/Ip frames
dumpStep - How many times to repeate the same frame 
interDumpFlag -- internal checking for more dumplicate requirement
return value
Status for function successful
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_TrickModeIPFrameSkip(
    B_DVR_MediaFileHandle mediaFile,
    unsigned skipStep,
    unsigned dumpStep,
    unsigned *interDumpFlag);

/*****************************************************************************
Summary:
B_DVR_MediaFile_StreamIPFrameRate get stream I/IP Frame rate
Param[in] 
mediaFile - media file handle
return value
Status for function successful
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_StreamIPFrameRate(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileStreamFrameRate *frameRate);

/*****************************************************************************
Summary:
B_DVR_MediaFile_UserSettings pass the speed rate parameters
Param[in] 
mediaFile - media file handle
numerator - speed rate numerator
Status for function successful
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_UserSettings(
    B_DVR_MediaFileHandle mediaFile,
    unsigned numerator);

/*****************************************************************************
Summary:
B_DVR_MediaFile_StreamIPFrameRate get stream related information
Param[in] 
mediaFile - media file handle
frameRes - hold the returned value on resource information
return value
Status for function successful
*****************************************************************************/
B_DVR_ERROR B_DVR_MediaFile_GetStreamInfo(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileStreamFrameResource *frameRes);

/*****************************************************************************
Summary:
B_DVR_MediaFile_GetMediaNode shall return a mediaNode associated with a 
media file instance. 
Param[in] 
mediaFile - media file handle
return value
media Node
*****************************************************************************/
B_DVR_MediaNode B_DVR_MediaFile_GetMediaNode(
    B_DVR_MediaFileHandle mediaFile);

#ifdef __cplusplus
}
#endif

#endif /*_B_DVR_MEDIAFILE_H */
