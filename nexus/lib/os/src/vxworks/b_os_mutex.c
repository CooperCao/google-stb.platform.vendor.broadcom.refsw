/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
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
#include <semLib.h>

#include "b_os_lib.h"
#include "bkni.h"
#include "bkni_multi.h"

typedef struct B_Mutex
{
    BKNI_MutexHandle mutexHandle;
    bool locked;
} B_Mutex;

BDBG_MODULE(b_os_mutex);

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
    B_Mutex *pMutex;
    BKNI_MutexHandle mutex;
    B_Error errCode;

    BSTD_UNUSED(pSettings);

    pMutex = B_Os_Malloc(sizeof(B_Mutex));
    if ( NULL == pMutex )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    errCode = BKNI_CreateMutex(&mutex);
    if (errCode != BERR_SUCCESS)
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        return NULL;
    }

    pMutex->mutexHandle = mutex;
    pMutex->locked  = false;

    return pMutex;
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
    BKNI_DestroyMutex(mutex->mutexHandle);
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
    BDBG_ASSERT(NULL != mutex);
    BKNI_AcquireMutex(mutex->mutexHandle);
    mutex->locked  = true;
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
    BERR_Code rc;
    BDBG_ASSERT(NULL != mutex);

    rc = BKNI_TryAcquireMutex(mutex->mutexHandle);
    if ( rc == BERR_TIMEOUT )
        return B_ERROR_TIMEOUT;

    mutex->locked  = true;
    return B_ERROR_SUCCESS;
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
    BDBG_ASSERT(NULL != mutex);
    mutex->locked  = false;
    BKNI_ReleaseMutex(mutex->mutexHandle);
}

/* Private debugging routine */
bool B_Mutex_P_IsLocked(
    B_MutexHandle mutex
    )
{
    return mutex->locked;
}

