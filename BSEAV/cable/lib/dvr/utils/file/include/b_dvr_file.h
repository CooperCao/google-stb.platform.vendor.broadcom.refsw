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
 * DVR File shall provide APIs for non segmented file operations. All these
 * APIs shall be used internally by DVR manager and various services, but have
 * been made public for writing unit tests for exercising these APIs for testing.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef B_DVR_FILE_H__
#define B_DVR_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"

/**********************************************************************************************
 Summary :
  B_DVR_FileIO enumerates the IO types. 
  Each IO type supports a set of Operating system/File System specific file operations.
  eB_DVR_FileIORead type shall allow read, seek and bounds check operations.
  eB_DVR_FileIOWrite type shall allow write and truncate operations.
***********************************************************************************************/
typedef enum B_DVR_FileIO
{
    eB_DVR_FileIORead, 
    eB_DVR_FileIOWrite
}B_DVR_FileIO;

/***********************************************************************************************
Summary:
B_DVR_FileType shall indicate whether the file read/write is for navigation or media
***********************************************************************************************/
typedef enum B_DVR_FileType
{
   eB_DVR_FileTypeMedia,
   eB_DVR_FileTypeNavigation,
   eB_DVR_FileTypeMax
}B_DVR_FileType;

/***********************************************************************************************
Summary:
B_DVR_FileInterface is wrapper for hooks into OS/File system specific IO read or write interfaces.
************************************************************************************************/
typedef struct B_DVR_FileInterface
{
    union io
        {
            struct bfile_io_read readFD;   
            struct bfile_io_write writeFD; 
        }io;
}B_DVR_FileInterface;

/**********************************************************************************************
 Summary :
  B_DVR_FileReadDirection enumerates reading direction for files
**********************************************************************************************/
typedef enum B_DVR_FileReadDirection
{
    eB_DVR_FileReadDirectionForward, 
    eB_DVR_FileReadDirectionReverse,
    eB_DVR_FileReadDirectionMax
}B_DVR_FileReadDirection;

/************************************************************************************************************
Summary:
B_DVR_File shall be a descriptor for a non segmented file.
This encapsulation helps in making supporting various file IO APIs across the operating systems 
and file systems.
*************************************************************************************************************/
typedef struct B_DVR_File
{
    B_DVR_FileInterface fileInterface;
    B_DVR_FileIO fileIO;
    int fd; /*OS/File System provided file descriptor*/
    bool directIO; /* Is direct IO supported */
    char fileName[B_DVR_MAX_FILE_NAME_LENGTH];
}B_DVR_File;

typedef B_DVR_File *B_DVR_FileHandle;

typedef struct B_DVR_FileSettings
{
    bool directIO;
    B_DVR_FileIO fileIO;
    bool create;
}B_DVR_FileSettings;

/************************************************************************************************
Summary:
A data structure containing details of file offsets in various types of recordings on a 
storage device.
*************************************************************************************************/
typedef struct B_DVR_FilePosition 
{
    off_t mpegFileOffset;
    off_t navFileOffset;
    unsigned long seqHdrOffset;
    unsigned long timestamp;
    unsigned long index;
    unsigned long pts;
} B_DVR_FilePosition;

/********************************************************************************
Summary:
B_DVR_File_Open shall open a DVR file
**********************************************************************************/
B_DVR_ERROR B_DVR_File_Open(
    B_DVR_FileHandle file, 
    const char *fname,
    B_DVR_FileSettings *pSettings);


/*********************************************************************************
Summary:
B_DVR_File_Close shall close a DVR file.
**********************************************************************************/
void B_DVR_File_Close(
    struct B_DVR_File *file);

#ifdef __cplusplus
}
#endif

#endif /* B_DVR_FILE_H__ */

