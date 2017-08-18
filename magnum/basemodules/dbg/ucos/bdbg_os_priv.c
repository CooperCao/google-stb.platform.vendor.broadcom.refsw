/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/

/* uCOS include files */
#include "type.h"
#include "ucos.h"
#include "ucosdrv.h"
 
#include "bstd.h"
#include "bkni.h"
#include "bdbg_os_priv.h"

/****************************************************************************
    Global functions
****************************************************************************/
/* tap into bkni's function to check for a BKNI critical section. It's ugly,
   but there is no BKNI standard interface to do this.
*/
extern inline bool CHECK_CRITICAL(void);

/****************************************************************************
    Static data
****************************************************************************/
/* platform spefic type for timestamps */
static ULONG initTicks;
/* OS-specific mutex */
static OS_EVENT * g_pBdbgMutex = NULL;

/****************************************************************************
****************************************************************************/
void
BDBG_P_InitializeTimeStamp(void)
{
    initTicks = OSTimeGet();
}

/****************************************************************************
****************************************************************************/
void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
    ULONG currentTicks;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;
    
    currentTicks = OSTimeGet();

    /* 1 tick every 10 ms */
    milliseconds = (currentTicks - initTicks)*10;
    
    /* Calculate the time   */
    hours = (milliseconds)/3600000;
    milliseconds = milliseconds % 3600000;
    minutes = (((milliseconds)/60000))%60000;
    milliseconds = milliseconds % 60000;
    seconds = (milliseconds/1000)%1000;
    milliseconds = milliseconds % 1000;
        
    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;
}

/****************************************************************************
****************************************************************************/
BERR_Code 
BDBG_P_OsInit(void)
{
    BERR_Code rc;
    
    /* This lock needs to work between both ISR and Task contexts. We're using
       a mutex because we only have a tiny amount of code running in ISR 
       context -- the L1 handler -- and we don't expect it to need the BDBG 
       lock.
       
       The lock is only used when a debug module is registered for the first
       time.
       
       We need to go directly to the OS for our mutex because we have code
       in BKNI which checks to make sure no one is trying to call AcquireMutex
       from within a Magnum ISR context. 
    */
    if (g_pBdbgMutex != NULL) {
        rc = BERR_TRACE(BERR_UNKNOWN);
        BKNI_Fail();
    }

    g_pBdbgMutex = OSSemCreate(1);
    if (g_pBdbgMutex == NULL) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        BKNI_Fail();
    }

    return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_OsUninit(void)
{
    /* No way to release kernel object in uCOS */
    BKNI_Printf("%s: uCOS Event Leak - no method to destroy semaphore\n", BSTD_FUNCTION);
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_Lock(void)
{
    UBYTE ucosError;

    if (g_pBdbgMutex == NULL) {
        ucosError = BERR_TRACE(BERR_NOT_INITIALIZED);
        return;
    }
    
    if (CHECK_CRITICAL()) {
        return;
    }
    
    /* Wait forever on this mutex */
    OSSemPend(g_pBdbgMutex, 0, &ucosError); 

    if (ucosError != OS_NO_ERR) {
        ucosError = BERR_TRACE(ucosError);
    }
}

/****************************************************************************
****************************************************************************/
void 
BDBG_P_Unlock(void)
{
    BERR_Code rc;
    uint32_t mutexCount;

    if (g_pBdbgMutex == NULL) {
        rc = BERR_TRACE(BERR_NOT_INITIALIZED);
        return;
    }

    if (CHECK_CRITICAL()) {
        return;
    }
    
    OS_ENTER_CRITICAL();
    mutexCount = g_pBdbgMutex->OSEventCnt;
    OS_EXIT_CRITICAL();
    if (mutexCount > 0) {
        /* Mutex already posted. Don't do it again */
        rc = BERR_TRACE(mutexCount);
    } else {
        OSSemPost(g_pBdbgMutex);
    }
}

/* End of file */
