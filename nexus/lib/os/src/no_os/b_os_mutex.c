/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
* Description:
*   API name: Event
*    Library routines for heap management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "bkni.h"
#include "bkni_multi.h"
/*
#include <pthread.h>
*/

typedef struct B_Mutex
{
    uint32_t mutexId;
    uint32_t owner;
} B_Mutex;

BDBG_MODULE(b_os_mutex);

#define THREAD_ID_INVALID ((pthread_t)0)

/* Eventually, this can be expanded to support recursive and possibly inversion-safe mutexes */

/***************************************************************************
Summary:
Get default mutex settings.
***************************************************************************/
void B_Mutex_GetDefaultSettings(
    B_MutexSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Create a mutex using the specified settings.
***************************************************************************/
B_MutexHandle B_Mutex_Create(
    const B_MutexSettings *pSettings   /* Pass NULL for defaults */
    )
{
#if 1
    B_Mutex *pMutex;
    B_Error errCode;
/*    int rc; */

    BSTD_UNUSED(pSettings);

    pMutex = B_Os_Malloc(sizeof(B_Mutex));
    if ( NULL == pMutex )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

	/*
    rc = pthread_mutex_init(&pMutex->mutexId, NULL);
    if ( rc )
    {
        B_Os_Free(pMutex);
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        return NULL;
    }

    pMutex->owner = THREAD_ID_INVALID;
	*/

    return pMutex;
#else
    B_Mutex *pMutex;
    BSTD_UNUSED(pSettings);
    return pMutex;
#endif
}

/***************************************************************************
Summary:
Destroy a mutex
***************************************************************************/
void B_Mutex_Destroy(
    B_MutexHandle mutex
    )
{
    BDBG_ASSERT(NULL != mutex);
	/*
	pthread_mutex_destroy(&(mutex->mutexId));
	*/
    B_Os_Free(mutex);
}
/***************************************************************************
Summary:
Lock a mutex
***************************************************************************/
void B_Mutex_Lock(
    B_MutexHandle mutex
    )
{
#if 1
	BSTD_UNUSED(mutex);
#else
    int rc;
    BDBG_ASSERT(NULL != mutex);

    #if BDBG_DEBUG_BUILD
    if ( pthread_equal(pthread_self(), mutex->owner) )
    {
        BDBG_ERR(("Deadlock on non-recursive mutex"));
        BDBG_ASSERT(false);
    }
    #endif

    /* This should never fail */
    rc = pthread_mutex_lock(&(mutex->mutexId));
    BDBG_ASSERT(0 == rc);

    #if BDBG_DEBUG_BUILD
    mutex->owner = pthread_self();
    #endif
#endif	
}

/***************************************************************************
Summary:
Try to lock a mutex.

Description:
This will attempt to lock the mutex without blocking.  If the mutex is
available, it will be locked and B_Error_eSuccess will be returned.  If
the mutex is unavailable, B_ERROR_TIMEOUT will be returned.
***************************************************************************/
B_Error B_Mutex_TryLock(
    B_MutexHandle mutex
    )
{
#if 1
	BSTD_UNUSED(mutex);
    return B_ERROR_SUCCESS;
#else
    BDBG_ASSERT(NULL != mutex);

    if ( pthread_mutex_trylock(&mutex->mutexId) )
    {
        return B_ERROR_TIMEOUT;
    }

    #if BDBG_DEBUG_BUILD
    mutex->owner = pthread_self();
    #endif
#endif
}

/***************************************************************************
Summary:
Unlock a mutex.

Description:
Mutexes must be unlocked by the same thread that successfully called
B_Mutex_Lock or B_Mutex_TryLock.
***************************************************************************/
void B_Mutex_Unlock(
    B_MutexHandle mutex
    )
{
#if 1
	BSTD_UNUSED(mutex);
#else
    BDBG_ASSERT(NULL != mutex);

    #if BDBG_DEBUG_BUILD
    if ( !pthread_equal(pthread_self(), mutex->owner) )
    {
        BDBG_ERR(("Attempt to unlock mutex from thread other than current owner"));
        BDBG_ASSERT(false);
    }
    #endif

    mutex->owner = THREAD_ID_INVALID;

    pthread_mutex_unlock(&mutex->mutexId);
#endif	
}

/* Private debugging routine */
bool B_Mutex_P_IsLocked(
    B_MutexHandle mutex
    )
{
#if 1
	BSTD_UNUSED(mutex);
	return true;
#else
    #if BDBG_DEBUG_BUILD
    if ( pthread_equal(pthread_self(), mutex->owner) )
    {
        return true;
    }
    return false;
    #else
    BSTD_UNUSED(mutex);
    return true;
    #endif
#endif	
}

