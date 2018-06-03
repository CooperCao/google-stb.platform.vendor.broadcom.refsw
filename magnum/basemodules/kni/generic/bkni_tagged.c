/***************************************************************************
 * Copyright (C) 2007-2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* implement tagged KNI functions */
#if BKNI_USE_TAGGED_API

#undef BKNI_Delay
#undef BKNI_Malloc
#undef BKNI_Free
#undef BKNI_Sleep
#undef BKNI_CreateEvent
#undef BKNI_DestroyEvent
#undef BKNI_WaitForEvent
#undef BKNI_SetEvent
#undef BKNI_EnterCriticalSection
#undef BKNI_LeaveCriticalSection
#undef BKNI_CreateMutex
#undef BKNI_DestroyMutex
#undef BKNI_AcquireMutex
#undef BKNI_ReleaseMutex


void 
BKNI_DelayTagged(int microsec, const char *filename, unsigned lineno)
{
    BDBG_P_PrintString("__ %s:%u Delay(%d)\n", filename, lineno, microsec);
    BKNI_Delay(microsec);
    return;
}

void *
BKNI_MallocTagged(size_t size, const char *filename, unsigned lineno)
{
    void *ptr;

    ptr = BKNI_Malloc(size);

    BDBG_P_PrintString("__ %s:%u Malloc(%d)->%#x\n", filename, lineno, (int)size, (unsigned)ptr);

    return ptr;
}

void 
BKNI_FreeTagged(void *ptr, const char *filename, unsigned lineno)
{
    BDBG_P_PrintString("__ %s:%u Free(%#x)\n", filename, lineno, (unsigned)ptr);
    BKNI_Free(ptr);
    return;
}

void
BKNI_SleepTagged(int millisec, const char *filename, unsigned lineno)
{
    BKNI_Sleep(millisec);
    BDBG_P_PrintString("__ %s:%u Sleep(%d)\n", filename, lineno, millisec);
    return;
}

BERR_Code 
BKNI_CreateEventTagged(BKNI_EventHandle *event, const char *filename, unsigned lineno)
{
    BERR_Code rc;

    rc = BKNI_CreateEvent(event);
    BDBG_P_PrintString("__ %s:%u CreateEvent(%#x)->result %d\n", filename, lineno, rc==BERR_SUCCESS?(unsigned)*event:0, (int)rc);
    return rc;
}

void
BKNI_DestroyEventTagged(BKNI_EventHandle event, const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u DestroyEvent(%#x)\n", filename, lineno, (unsigned)event);
    BKNI_DestroyEvent(event);
    return;
}

BERR_Code 
BKNI_WaitForEventTagged(BKNI_EventHandle event, int timeoutMsec, const char *filename, unsigned lineno)
{
    BERR_Code rc;

    rc = BKNI_WaitForEvent(event, timeoutMsec);
    BDBG_P_PrintString("__ %s:%u WaitForEvent(%#x, %d)->result %d\n", filename, lineno, (unsigned)event, timeoutMsec, (int)rc);
    return rc;
}

void
BKNI_SetEventTagged(BKNI_EventHandle event, const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u SetEvent(%#x)\n", filename, lineno, (unsigned)event);
    BKNI_SetEvent(event);
    return;
}

void
BKNI_EnterCriticalSectionTagged(const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u EnterCriticalSection\n", filename, lineno);
    BKNI_LeaveCriticalSection();
    return;
}

void
BKNI_LeaveCriticalSectionTagged(const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u LeaveCriticalSection\n", filename, lineno);
    BKNI_LeaveCriticalSection();
    return ;
}


BERR_Code 
BKNI_CreateMutexTagged(BKNI_MutexHandle *mutex, const char *filename, unsigned lineno)
{
    BERR_Code rc;

    rc = BKNI_CreateMutex(mutex);
    BDBG_P_PrintString("__ %s:%u CreateMutex(%#x)->result %d\n", filename, lineno, rc==BERR_SUCCESS?(unsigned)mutex:0, (int)rc);
    return rc;
}

void
BKNI_DestroyMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno)
{
    BDBG_P_PrintString("__ %s:%u DestroyMutex(%#x)\n", filename, lineno, (unsigned)mutex);
    BKNI_DestroyMutex(mutex);
    return;
}

void BKNI_AcquireMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno)
{
    BKNI_AcquireMutex(mutex);
    BDBG_P_PrintString("__ %s:%u AcquireMutex(%#x)\n", filename, lineno, (unsigned)mutex);
    return;
}

void
BKNI_ReleaseMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u ReleaseMutex(%#x)\n", filename, lineno, (unsigned)mutex);
    BKNI_ReleaseMutex(mutex);
    return;
}


#endif /* BKNI_USE_TAGGED_API */
