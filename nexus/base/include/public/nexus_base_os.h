/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/
#ifndef NEXUS_BASE_OS_H
#define NEXUS_BASE_OS_H

/* This is part of the public API. */
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_base_types.h"       /* this is also public API */

/**
SW services provided by base which can be used inside and outside nexus.
This file should not contain any nexus internals, including NEXUS_ModuleHandle, ISR context or magnum non-basemodules.
**/

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Handle for a thread.

Description:

See Also:
NEXUS_Thread_Create
**/
typedef struct NEXUS_Thread *NEXUS_ThreadHandle;

/**
Summary:
Settings used for creating a thread

Description:

See Also:
NEXUS_Thread_GetDefaultSettings
NEXUS_Thread_Create
**/
typedef struct NEXUS_ThreadSettings
{
    unsigned priority;  /* 0=highest, 100=lowest */
    size_t  stackSize; /* In Bytes, may be rounded up to OS minimum */
} NEXUS_ThreadSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_Thread_Create
**/
void NEXUS_Thread_GetDefaultSettings( /* attr{local=true} */
    NEXUS_ThreadSettings *pSettings    /* [out] Default Settings for OS */
    );

/**
Summary:
Create a thread.

Description:

See Also:
NEXUS_Thread_Destroy
NEXUS_Thread_GetDefaultSettings
**/
NEXUS_ThreadHandle NEXUS_Thread_Create( /* attr{local=true} */
    const char *pThreadName,                 /* Thread Name, optional */
    void (*pThreadFunc)(void *),             /* Thread Main Routine */
    void *pContext,                          /* Context provided to callback */
    const NEXUS_ThreadSettings *pSettings    /* Thread Settings */
    );

/**
Summary:
Destroy a thread after its function has exited.

Description:
This does not cancel the execution of the pThreadFunc. pThreadFunc must exit on its own, then
NEXUS_Thread_Destroy can clean up the resources.

See Also:
NEXUS_Thread_Create
**/
void NEXUS_Thread_Destroy( /* attr{local=true} */
    NEXUS_ThreadHandle thread /* Thread Handle, returned from NEXUS_Thread_Create() */
    );

/**
Summary:
Activate gathering of run-time software profile information.
Profiling support shall be enabled at the compile time.
**/
NEXUS_Error NEXUS_Profile_Start( /* attr{local=true} */
    void
    );

/**
Summary:
Finishes gathering of run-time software profile information and prints report.
**/
void NEXUS_Profile_Stop( /* attr{local=true} */
    const char *name /* title of the profile report */
    );

/**
Summary:
Mark thread that could originate profiling samples
**/
void NEXUS_Profile_MarkThread( /* attr{local=true} */
    const char *name /* thread name */
    );

/**
Summary:
Flush the data cache for an address range.

Description:
This function will flush and invalidate the address range provided
from the data cache.  It is guaranteed that at least the address range
provided will be flushed, however larger amounts may be flushed depending
on the CPU and underlying OS primitives available.
****************************************************************************/
#define NEXUS_FlushCache NEXUS_FlushCache_isrsafe

/**
Summary:
ISRSAFE variant of NEXUS_FlushCache

Description:
**/

void NEXUS_FlushCache_isrsafe( /* attr{local=true} */
    const void *address, /* cached address to flush */
    size_t size /* size in bytes to flush */
    );

/**
Summary:
Returns the value for an environment variable.

Description:
The implementation of this function varies by OS.
For Linux user mode, this will be environment variables set in the shell.
For Linux kernel mode, it could be insmod parameters.

Returns:
A null-terminated string for the environment variable.
NULL means that it does not exist.

See Also:
NEXUS_SetEnv
*/
#define NEXUS_GetEnv NEXUS_GetEnv_isrsafe
const char *NEXUS_GetEnv( /* attr{local=true} */
    const char *name
    );

/**
Summary:
Sets the value for an environment variable.

Description:
If value is NULL, the internal state for the given name will be cleared.

See Also:
NEXUS_GetEnv
*/
void NEXUS_SetEnv( /* attr{local=true} */
    const char *name,
    const char *value
    );


/**
Summary:
Convert a null-terminated ASCII string to an integer.

Description:
Nexus wrapper for C89,Posix.1 atoi(3) function
*/
int NEXUS_atoi( /* attr{local=true} */
    const char *str
    );

/**
Summary:
Standard string functions
**/
int NEXUS_P_Base_StrCmp_isrsafe( /* attr{local=true} */
    const char *str1,
    const char *str2
    );
#define NEXUS_P_Base_StrCmp NEXUS_P_Base_StrCmp_isrsafe

int b_strlen( /* attr{local=true} */
    const char *s
    );

char *b_strncpy( /* attr{local=true} */
    char *dest,
    const char *src,
    int n);

typedef struct NEXUS_Timestamp {
    uint64_t val; /* Private format. Subject to change. */
} NEXUS_Timestamp;

void NEXUS_GetTimestamp( /* attr{local=true} */
    NEXUS_Timestamp *timestamp
    );

struct timeval;
void NEXUS_GetWallclockFromTimestamp( /* attr{local=true} */
    const NEXUS_Timestamp *timestamp,
    struct timeval *tv
    );

#ifdef __cplusplus
}
#endif

#endif /* !defined NEXUS_BASE_OS_H */


