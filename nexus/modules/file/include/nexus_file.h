/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 **************************************************************************/
#ifndef NEXUS_FILE_H__
#define NEXUS_FILE_H__

#include "nexus_file_types.h"

#ifndef DIRECT_IO_SUPPORT
#define DIRECT_IO_SUPPORT 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
NEXUS_FileReadHandle is used for a file I/O read interface.
**/
typedef struct bfile_io_read *NEXUS_FileReadHandle;

/**
Summary:
NEXUS_FileWriteHandle is used for a file I/O write interface.
**/
typedef struct bfile_io_write *NEXUS_FileWriteHandle;


/*
Summary:
This is an object to retrieve data from the media.

Description:
This object has support for two independent data flows.
Usually it implemented by read(2) and seek(2) functions
This is used by NEXUS_Playback_Start.
*/
typedef struct NEXUS_FilePlay *NEXUS_FilePlayHandle;

/*
Summary:
This is an object to store data to the media.

Description:
Usually it implemented by write(2) function.
This object has support for two independent data flows.
This is used by NEXUS_Record_Start.
*/
typedef struct NEXUS_FileRecord *NEXUS_FileRecordHandle;

/**
Summary:
Callback used for NEXUS_File_AsyncRead and NEXUS_File_AsyncWrite
**/
typedef void (*NEXUS_File_Callback)(void *cntx, ssize_t size);

/**
Summary:
This value is used to indicate if timestamp or index position is invalid
**/
#define NEXUS_FILE_INVALID_POSITION ((unsigned long)-1)

/**
Summary:
NEXUS_FilePosition is used to define the position in the recorded file
**/
typedef struct NEXUS_FilePosition {
    off_t mpegFileOffset;       /*  Position in the mpeg file, in bytes. */
    unsigned long indexOffset;  /*  Offset of the current index entry in the index, in bytes.
                                    This field is set to NEXUS_FILE_INVALID_POSITION if index is invalid */

    unsigned long timestamp;    /*  Timestamp of the current index entry in the index.
                                    Timestamp is different from PTS. Timestamp is generated
                                    by bcmindexer, it begins at 0 for any recorded stream,
                                    and is guaranteed to be continuous increasing throughout
                                    the stream. The units of the timestamp depend on the
                                    implementation of bcmindexer, but they are defaulted
                                    to milliseconds.
                                    This field is set to NEXUS_FILE_INVALID_POSITION if index is invalid */
} NEXUS_FilePosition;


/**
Summary:
Asynchronous read

Description:
The following are notes about writing a new NEXUS_File implementation:

The callback parameter will be invoked when the file read is complete.
NEXUS_File_AsyncRead is not required to read the full amount of data specified by the length parameter.
The caller's state machine may block while waiting for NEXUS_File_AsyncRead to complete its transaction,
so it is best for File to be non-blocking. A blocking implementation of File is permitted,
but it will likely cause ill effects in other parts of the system.

The callback size parameter should follow these rules:
- If there is no data available but the file is not at the end (e.g. streaming data), callback should return size == BFILE_ERROR_NO_DATA.
- If there is no data available because the file is at the end, callback should return size == 0. The caller should then initiate end-of-file processing.
- If there is an error in the file I/O, callback should return size == -1. The caller should then terminate file processing.

The caller expects that the callback will be invoked from another thread.
The caller will be holding the module mutex when calling NEXUS_File_AsyncRead and the function passed in as a callback
will likely try to acquire the same module mutex. Unless there is a context switch allowing NEXUS_File_AsyncRead to return, a deadlock will result.
**/
void NEXUS_File_AsyncRead(
    NEXUS_FileReadHandle file,
    void *buf,
    size_t length, /* length of memory pointed to by buf in bytes. NEXUS_File_AsyncRead can read <= this amount. */
    NEXUS_ModuleHandle module, /* module lock will be acquired before callback is called */
    NEXUS_File_Callback callback, /* will be called when I/O is complete */
    void *cntx /* context passed to callback */
    );

/**
Summary:
Asynchronous write
**/
void NEXUS_File_AsyncWrite(
    NEXUS_FileWriteHandle file,
    const void *buf,
    size_t length, /* length of data pointed to by buf in bytes */
    NEXUS_ModuleHandle module, /* module lock will be acquired before callback is called */
    NEXUS_File_Callback callback, /* will be called when I/O is complete */
    void *cntx /* context passed to callback */
    );

/**
Summary:
Synchronous file I/O using posix open/read calls.

Description:
This is used for reading indexes.
**/
NEXUS_FilePlayHandle NEXUS_FilePlay_OpenPosix(
    const char *dataFileName,
    const char *indexFileName
    );

/**
Summary:
Synchronous file I/O using posix open/write calls.

Description:
This is used for recording indexes.
**/
NEXUS_FileRecordHandle NEXUS_FileRecord_OpenPosix(
    const char *dataFileName,
    const char *indexFileName
    );


/**
Summary:
Synchronous file I/O using posix open/write calls that append to the existing file.

Description:
There are significant limitations to this feature.
Unless the ES data is formed correctly, the splice point in the stream will be seen as extended macroblocking
or audio glitching. The splice point is not tracked, so there is no way for the application to stop and
restart decode at that point. Also, there can be no pid or codec change.

Instead, we recommend the application splice together separate recordings in the user interface.
For instance, if there is a box reboot during a recording, or a channel change in the timeshift buffer, instead of
appending to the previous record, the application can keep track of the separate recordings and manage a clean transition
between them. When playback of the first recording ends, the application receives a callback and can start the second.
The transition is very fast and the playback and decoder stop/start is exactly what is required to make the transition
clean and general purpose.
**/
NEXUS_FileRecordHandle NEXUS_FileRecord_AppendPosix(
    const char *dataFileName,
    const char *indexFileName
    );

/**
Summary:
Cancel a stuck play file thread

Description:
If a file thread becomes stuck (e.g. NEXUS_File_Callback not received), you can cancel it by calling this function.
The thread will be marked as cancelled. Any pending result will be lost. A new thread will be created in its place.
**/
void NEXUS_FilePlay_Cancel(
    NEXUS_FilePlayHandle file
    );

/**
Summary:
Close a play file
**/
void NEXUS_FilePlay_Close(
    NEXUS_FilePlayHandle file
    );

/**
Summary:
Close a record file
**/
void NEXUS_FileRecord_Close(
    NEXUS_FileRecordHandle file
    );

/*
Summary:
This function returns bounds of the play file

Description:
This function could be used for files that were previously recorded using NEXUS_Record.

If NEXUS_FilePlayHandle is in use by NEXUS_Playback, this function will fail because it must
modify the state of the file read pointer. Use NEXUS_Playback_GetStatus or open a separate
NEXUS_FilePlayHandle instance.
*/
NEXUS_Error NEXUS_FilePlay_GetBounds(
    NEXUS_FilePlayHandle file, /* Handle returned by NEXUS_FifoRecord_Create */
    NEXUS_FilePosition *pFirst,  /* [out] first position in the file */
    NEXUS_FilePosition *pLast    /* [out] last position in the file */
    );

/*
Summary:
Get position information for a given timestamp.

Description:
Function fails if timestamp is out of bounds.

If NEXUS_FilePlayHandle is in use by NEXUS_Playback, this function will fail because it must
modify the state of the file read pointer. Use NEXUS_Playback_GetStatus or open a separate
NEXUS_FilePlayHandle instance.
*/
NEXUS_Error NEXUS_FilePlay_GetLocation(
    NEXUS_FilePlayHandle file, /* Handle returned by NEXUS_FifoRecord_Create */
    unsigned long timestamp,    /* Timestamp of requested position, units of milliseconds */
    NEXUS_FilePosition *pPosition  /* [out] position in the file of closest random access point */
    );

/*
Summary:
This structure is used to communicate user settings for NEXUS_FilePlayOffset_Open
*/
typedef struct NEXUS_FilePlayOffset_Settings {
    off_t dataOffset; /* sets new "beginning of file" for the data file, in units of bytes */
    unsigned long indexOffset; /* sets new "beginning of file" for the index file, in units of bytes */
} NEXUS_FilePlayOffset_Settings;

/*
Summary:
This function initializes NEXUS_FilePlayOffset_Settings structure
*/
void NEXUS_FilePlayOffset_GetDefaultSettings(
    NEXUS_FilePlayOffset_Settings *pSettings /* [out] */
    );

/*
Summary:
This function is used to wrap NEXUS_FilePlay object to apply an offset for index and data
*/
NEXUS_FilePlayHandle NEXUS_FilePlayOffset_Open(
    NEXUS_FilePlayHandle file,
    const NEXUS_FilePlayOffset_Settings *pSettings
    );


/**
Summary:
NEXUS_MuxFileHandle used to save mux data
**/
typedef struct NEXUS_MuxFile *NEXUS_MuxFileHandle;

/**
Summary:
NEXUS_MuxFileHandle used for a file I/O mux interface.
**/
typedef struct bfile_io_mux *NEXUS_MuxFileIoHandle;

/**
Summary:
Close a mux file
**/
void NEXUS_MuxFile_Close(NEXUS_MuxFileHandle file);

/**
Summary:
Synchronous file I/O using posix open/write calls.

Description:
This is used for a file mux .
**/
NEXUS_MuxFileHandle NEXUS_MuxFile_OpenPosix(
    const char *fileName
    );

/**
Summary:
Asynchronous mux read
**/
void NEXUS_File_AsyncMuxWrite(
    NEXUS_MuxFileIoHandle file,
    off_t offset,
    const NEXUS_FileMuxIoWriteAtom *atoms, /* pointer to elements to be written */
    size_t atom_count, /* number of elements */
    NEXUS_ModuleHandle module, /* module lock will be acquired before callback is called */
    NEXUS_File_Callback callback, /* will be called when I/O is complete */
    void *cntx /* context passed to callback */
    );


/**
Summary:
Asynchronous mux read
**/
void NEXUS_File_AsyncMuxRead (
    NEXUS_MuxFileIoHandle file,
    off_t offset,
    const NEXUS_FileMuxIoReadAtom *atoms, /* pointer to elements to be read */
    size_t atom_count, /* number of elements */
    NEXUS_ModuleHandle module, /* module lock will be acquired before callback is called */
    NEXUS_File_Callback callback, /* will be called when I/O is complete */
    void *cntx /* context passed to callback */
    );

/**
Summary:
NEXUS_FileRecordOpenSettings is used to define run-time configuration when opening file for record
**/
typedef struct NEXUS_FileRecordOpenSettings
{
    struct {
        const char *filename; /* relative path from current directory. */
        bool directIo;
        bool sync; /* immediate write to disk */
    } data;
    struct {
        const char *filename; /* relative path from current directory. use NULL if file is unused. */
        bool sync; /* immediate write to disk */
    } index;
    bool append; /* controls whether new file created or data gets appended to the existing file.
                    see NEXUS_FileRecord_AppendPosix for limitations. */
} NEXUS_FileRecordOpenSettings;    
    
/**
Summary:
Initializes NEXUS_FileRecordOpenSettings with default values
**/
void NEXUS_FileRecord_GetDefaultOpenSettings(
    NEXUS_FileRecordOpenSettings *pSettings
);

/**
Summary:
Opens file for record
**/
NEXUS_FileRecordHandle NEXUS_FileRecord_Open(
    const NEXUS_FileRecordOpenSettings *pSettings
);


/**
Summary:
NEXUS_FilePlayOpenSettings is used to define run-time configuration when opening file for playback
**/
typedef struct NEXUS_FilePlayOpenSettings
{
    struct {
        const char *filename; /* relative path from current directory. */
        bool directIo;
    } data;
    struct {
        const char *filename; /* relative path from current directory. use NULL if file is unused. */
    } index;
} NEXUS_FilePlayOpenSettings;

/**
Summary:
Initializes NEXUS_FilePlayOpenSettings with default values
**/
void NEXUS_FilePlay_GetDefaultOpenSettings(
    NEXUS_FilePlayOpenSettings *pSettings
);

/**
Summary:
Opens file for playback
**/
NEXUS_FilePlayHandle NEXUS_FilePlay_Open(
    const NEXUS_FilePlayOpenSettings *pSettings
);


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_FILE_H__ */

