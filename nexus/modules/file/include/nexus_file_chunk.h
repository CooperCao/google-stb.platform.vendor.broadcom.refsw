/***************************************************************************
*     (c)2010-2013 Broadcom Corporation
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
* API Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FILE_CHUNK_H__
#define NEXUS_FILE_CHUNK_H__

#include "nexus_file.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings for NEXUS_ChunkedFilePlay_OpenFile
**/
typedef struct NEXUS_ChunkedFilePlayOpenSettings {
    off_t chunkSize; /* Size per chunk of file */
} NEXUS_ChunkedFilePlayOpenSettings;

/*
Summary:
Get default settings
*/
void NEXUS_ChunkedFilePlay_GetDefaultOpenSettings(
    NEXUS_ChunkedFilePlayOpenSettings *pSettings /* [out] */
    );

/**
Summary:
Specialized function to play back files recorded with NEXUS_ChunkedFileRecord_OpenFile

Description:
NEXUS_ChunkedFilePlay_OpenFile opens a file for playback.  It assumes that the file is split
into a series of chunks (of chunkSize), and they are contained in a directory
dataFileDirectoryName.

When working with chunked record and play, the specified chunkSize should match for both
operations.
**/
NEXUS_FilePlayHandle NEXUS_ChunkedFilePlay_Open(
    const char *dataFileDirectoryName,
    const char *indexFileName,
    const NEXUS_ChunkedFilePlayOpenSettings *pSettings
    );

/**
Summary:
Settings for NEXUS_ChunkedFileRecord_OpenFile
**/
typedef struct NEXUS_ChunkedFileRecordOpenSettings {
    off_t chunkSize; /* Size per chunk of file */
} NEXUS_ChunkedFileRecordOpenSettings;

/*
Summary:
Get default settings
*/
void NEXUS_ChunkedFileRecord_GetDefaultOpenSettings(
    NEXUS_ChunkedFileRecordOpenSettings *pSettings /* [out] */
    );

/**
Summary:
Specialized function to record files in chunks

Description:
NEXUS_ChunkedFileRecord_OpenFile opens a file for recording.  It assumes that the directory
dataFileDirectoryName exists, and that the individual chunks of the data file should be placed
in this directory.

The index is not chunked.

When working with chunked record and play, the specified chunkSize should match for both
operations.
**/
NEXUS_FileRecordHandle NEXUS_ChunkedFileRecord_Open(
    const char *dataFileName,
    const char *indexFileName,
    const NEXUS_ChunkedFileRecordOpenSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_FILE_CHUNK_H__ */
