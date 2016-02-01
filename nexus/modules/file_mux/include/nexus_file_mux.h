/***************************************************************************
 *     (c)2010-2012 Broadcom Corporation
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
 **************************************************************************/
#ifndef NEXUS_FILE_MUX_H__
#define NEXUS_FILE_MUX_H__

#include "nexus_video_encoder_output.h"
#include "nexus_audio_mux_output.h"
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_encoder.h"
#else
typedef void *NEXUS_SimpleEncoderHandle;
#endif
#include "nexus_file.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Muxing MP4 files to store on hard-drive like media
**/

/**
Summary:
**/
typedef struct NEXUS_FileMux *NEXUS_FileMuxHandle;

/**
Summary:
**/
typedef struct NEXUS_FileMuxCreateSettings
{
    NEXUS_CallbackDesc finished; /* NEXUS_FileMux_Finish has completed. NEXUS_FileMux_Stop can be called for a clean finish. */
    size_t storageDescriptors; /* number of allocated storage descriptors */
    struct {
        bool supported;
        size_t metadataCache;/* the number of "samples" stored in memory before being flushed to external storage */
        size_t heapSize;  /* space for allocating temporary storage for boxes created during moov creation */
        size_t sizeEntriesCache;  /* the number of entries in the size storage */
        size_t relocationBuffer; /* Size of the buffer to relocate mdat section */
    } mp4;
    struct {
        bool supported;
        size_t packetLength; /* Size of ASF packets */
    } asf;
    struct {
        bool supported;
    } pes;
} NEXUS_FileMuxCreateSettings;

/**
Summary:
**/
void NEXUS_FileMux_GetDefaultCreateSettings(
    NEXUS_FileMuxCreateSettings *pSettings   /* [out] */
    );

/**
Summary:
**/
NEXUS_FileMuxHandle NEXUS_FileMux_Create(
    const NEXUS_FileMuxCreateSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    );

/**
Summary:
**/
void NEXUS_FileMux_Destroy(
    NEXUS_FileMuxHandle handle
    );


#define NEXUS_MAX_MUX_PIDS 16
typedef struct NEXUS_FileMuxVideoPid {
    unsigned track; 
    NEXUS_VideoCodec codec;
    NEXUS_VideoEncoderHandle encoder; /* if NULL, then not enabled */
    NEXUS_SimpleEncoderHandle simpleEncoder; /* if NULL, then not enabled */
} NEXUS_FileMuxVideoPid;

typedef struct NEXUS_FileMuxAudioPid {
    unsigned track;
    NEXUS_AudioCodec codec;
    NEXUS_AudioMuxOutputHandle muxOutput; /* if NULL, then not enabled */
    NEXUS_SimpleEncoderHandle simpleEncoder; /* if NULL, then not enabled */
} NEXUS_FileMuxAudioPid;

/**
Summary:
**/
typedef struct NEXUS_FileMuxStartSettings
{
    NEXUS_TransportType streamType;

    NEXUS_FileMuxVideoPid video[NEXUS_MAX_MUX_PIDS];
    NEXUS_FileMuxAudioPid audio[NEXUS_MAX_MUX_PIDS];

    char tempDir[64]; /* location for temporary files */

    unsigned expectedDuration;   /* expected duration of encoded file 0 if unknown, in ms */
    union {
        struct {
            bool progressiveDownloadCompatible;   /* ensure output is compatible with progressive download */
            unsigned createTime;    /* Seconds since Midnight Jan 1st 1904 , UTC time */
        } mp4;
        struct {
            uint8_t fileId[16];     /* 128-bit Globally Unique Identifier (GUID) this file */       
            bool broadcastOnly;      /* file created as broadcast type, when creating this type of file all output data is sequential */
        } asf;
    } streamSettings;
} NEXUS_FileMuxStartSettings;

/**
Summary:
**/
void NEXUS_FileMux_GetDefaultStartSettings(
    NEXUS_FileMuxStartSettings *pSettings, /* [out] */
    NEXUS_TransportType streamType
    );

/**
Summary:
**/
NEXUS_Error NEXUS_FileMux_Start(
    NEXUS_FileMuxHandle handle,
    const NEXUS_FileMuxStartSettings *pSettings,
    NEXUS_MuxFileHandle file
    );

/**
Summary:
Finalize all data stored to output file

Description:
To produce valid output file,  call NEXUS_FileMux_Finish, wait for finished callback, then call NEXUS_FileMux_Stop.
Time for MUX to finish would depend on nature of format, specified options and in some cases proportional to duration of file.
**/
void NEXUS_FileMux_Finish(
    NEXUS_FileMuxHandle handle
    );

/**
Summary:
Stop muxing data immediately.

Description:
Resulted file might be not playable unless NEXUS_FileMux_Finish called first and muxing completely finished.
**/
void NEXUS_FileMux_Stop(
    NEXUS_FileMuxHandle handle
    );

         
/**
Summary:
**/
typedef struct NEXUS_FileMuxStatus
{
    unsigned duration; /* file duration (in miliseconds) completed at current time */
} NEXUS_FileMuxStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_FileMux_GetStatus(
    NEXUS_FileMuxHandle handle,
    NEXUS_FileMuxStatus *pStatus /* [out] */
    );
#ifdef __cplusplus
}
#endif


#endif /* NEXUS_FILE_MUX_H__ */



