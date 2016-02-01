/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

BERR_Code 
BKNI_SleepTagged(int millisec, const char *filename, unsigned lineno)
{
    BERR_Code rc;

    rc = BKNI_Sleep(millisec);
    BDBG_P_PrintString("__ %s:%u Sleep(%d)->result %d\n", filename, lineno, millisec, (int)rc);
    return rc;
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

BERR_Code BKNI_AcquireMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno)
{
    BERR_Code rc;

    rc = BKNI_AcquireMutex(mutex);
    BDBG_P_PrintString("__ %s:%u AcquireMutex(%#x)->result %d\n", filename, lineno, (unsigned)mutex, (int)rc);
    return rc;
}

void
BKNI_ReleaseMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno)
{

    BDBG_P_PrintString("__ %s:%u ReleaseMutex(%#x)\n", filename, lineno, (unsigned)mutex);
    BKNI_ReleaseMutex(mutex);
    return;
}


#endif /* BKNI_USE_TAGGED_API */
