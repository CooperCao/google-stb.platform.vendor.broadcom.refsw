/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef BKNI_MULTI_H__
#define BKNI_MULTI_H__

#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*************************************************************************
The multi-tasking kernel interface can only be used by multi-tasking
modules. In the Magnum architecture, this can only be SysLib modules.

See bkni.h for a kernel interface overview.
****************************************************************************/

/***************************************************************************
Summary:
    BKNI_INFINITE can be passed as a timeout value into BKNI_WaitForEvent
    for multi-threaded modules.
****************************************************************************/
#define BKNI_INFINITE -1

/***************************************************************************
Summary:
    Mutex object initialized by BKNI_CreateMutex.
****************************************************************************/
typedef struct BKNI_MutexObj *BKNI_MutexHandle;

/***************************************************************************
Summary:
    Create a mutex in an unacquired state.

Description:
    The mutex is returned in an unacquired state, which means that the first call
    to BKNI_AcquireMutex or BKNI_TryAcquireMutex for that mutex is guaranteed to succeed immediately.

    Note that there is no name parameter in BKNI_CreateMutex. We do not support named
    mutexes because they are essentially global variables that can lead to deadlock.
    Passing in a name for debugging purposes might lead to someone to think we
    support named mutexes.

Returns:
    BERR_SUCCESS if the mutex is created.
    BERR_OS_ERROR if mutex was not created.
****************************************************************************/
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *mutex);
#if BKNI_TRACK_MALLOCS
#define BKNI_CreateMutex(mutex) BKNI_CreateMutex_tagged(mutex, BSTD_FILE, BSTD_LINE)

BERR_Code BKNI_CreateMutex_tagged(
    BKNI_MutexHandle *mutex,
    const char *file,
    int line
    );
#endif

/***************************************************************************
Summary:
    Destroy a mutex.

Description:
    Destroying a mutex in an acquired state is not allowed and leads to undefined behavior.
****************************************************************************/
void BKNI_DestroyMutex(BKNI_MutexHandle mutex);
#if BKNI_TRACK_MALLOCS
#define BKNI_DestroyMutex(mutex) BKNI_DestroyMutex_tagged(mutex, BSTD_FILE, BSTD_LINE)

void BKNI_DestroyMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    );
#endif

/***************************************************************************
Summary:
    Try acquiring exclusive ownership of a mutex, but do not block.

Description:
    If no other task currently owns the mutex, BKNI_TryAcquireMutex will return
    with BERR_SUCCESS and the caller retains exclusive ownership until
    BKNI_ReleaseMutex is called for that mutex.

    If another task currently owns the mutex, BKNI_TryAcquireMutex will not block
    but will return immediately with BERR_TIMEOUT.

    Magnum code cannot nest calls to BKNI_TryAcquireMutex or BKNI_AcquireMutex.
    If a mutex has already been acquired in a task, that same task cannot acquire it
    a second time. It leads to undefined behavior. Some possible results include:

    * Deadlock. The function will never return and the task is hung forever.
    * Systemhalts.
    * Error returned and mutex is only acquired once. If the error code is not handled (which is a violation of Magnum rules), race conditions will be introduced.
    * No error returned and mutex is acquired twice (two releases are required). This may allow bad code to be hidden until another platform uncovers the problem.

    A platform's implementation may chose any of the above results, although we strongly
    recommend that the function fail and return an error code if possible.

Returns:
    BERR_SUCCESS - Mutex was acquired successfully.
    BERR_TIMEOUT - The mutex is already acquired by another task and so the mutex was not acquired.
    BERR_OS_ERROR - The system failed and the mutex was not acquired.

See Also:
    BKNI_AcquireMutex, BKNI_ReleaseMutex, Magnum ThreadSafe rules
****************************************************************************/
BERR_Code BKNI_TryAcquireMutex(
    BKNI_MutexHandle mutex
    );
#if BKNI_TRACK_MALLOCS
#define BKNI_TryAcquireMutex(MUTEX) BKNI_TryAcquireMutex_tagged(MUTEX, BSTD_FILE, BSTD_LINE)

BERR_Code BKNI_TryAcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    );
#endif


/***************************************************************************
Summary:
    Acquire exclusive ownership of a mutex, possibly blocking.

Description:
    Acquire exclusive ownership of a mutex. If another task currently
    owns the mutex, BKNI_AcquireMutex will block until the mutex can be acquired.
    After acquiring the mutex, the caller retains exclusive ownership until
    BKNI_ReleaseMutex is called for that mutex.

    Magnum code cannot nest calls to BKNI_AcquireMutex or BKNI_TryAcquireMutex.
    If a mutex has already been acquired in a task, that same task cannot acquire it
    a second time. It leads to undefined behavior. Some possible results include:

    * Deadlock. The function will never return and the task is hung forever.
    * Systemhalts.
    * Error returned and mutex is only acquired once. If the error code is not handled (which is a violation of Magnum rules), race conditions will be introduced.
    * No error returned and mutex is acquired twice (two releases are required). This may allow bad code to be hidden until another platform uncovers the problem.

    A platform's implementation may chose any of the above results, although we strongly
    recommend that the function fail and return an error code if possible.

Returns:
    BERR_SUCCESS - Mutex was acquired successfully.
    BERR_OS_ERROR - The system failed or was interrupted, and the mutex was not acquired.

See Also:
    BKNI_TryAcquireMutex, BKNI_ReleaseMutex, Magnum ThreadSafe rules
****************************************************************************/
BERR_Code BKNI_AcquireMutex(
    BKNI_MutexHandle mutex
    );
#if BKNI_TRACK_MALLOCS
#define BKNI_AcquireMutex(MUTEX) BKNI_AcquireMutex_tagged(MUTEX, BSTD_FILE, BSTD_LINE)

BERR_Code BKNI_AcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    );
#endif

/***************************************************************************
Summary:
    Release exclusive ownership of a mutex.

Description:
    If you successfully acquired the mutex using BKNI_AcquireMutex or BKNI_TryAcquireMutex,
    BKNI_ReleaseMutex will release the mutex.

    In Magnum code, releasing a mutex which was acquired in a different task is not
    allowed and leads to undefined behavior.
    Also, releasing a mutex which is not in an acquired state is not allowed
    and leads to undefined behavior.

    A platform implementation does not have to enforce the usage rules noted
    above. This is because it may not be possible in all cases, and the platform
    may want to actually allow this behavior in platform-specific (non-Magnum) code.

See Also:
    BKNI_AcquireMutex, BKNI_TryAcquireMutex, Magnum ThreadSafe rules
****************************************************************************/
void BKNI_ReleaseMutex(BKNI_MutexHandle mutex);

#ifdef __cplusplus
}
#endif

#endif /* BKNI_MULTI_H__ */
