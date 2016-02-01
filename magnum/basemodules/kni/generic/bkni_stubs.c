/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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

BDBG_MODULE(kernelinterface);

#include <stdio.h>

/* needed to support tagged interface */
#undef BKNI_Delay
#undef BKNI_Sleep
#undef BKNI_CreateEvent
#undef BKNI_DestroyEvent
#undef BKNI_WaitForEvent
#undef BKNI_SetEvent
#undef BKNI_EnterCriticalSection
#undef BKNI_LeaveCriticalSection
#undef BKNI_CreateMutex
#undef BKNI_DestroyMutex
#undef BKNI_TryAcquireMutex
#undef BKNI_AcquireMutex
#undef BKNI_ReleaseMutex
#undef BKNI_CreateTask
#undef BKNI_DestroyTask

struct BKNI_EventObj
{
    int dummy;
};

struct BKNI_MutexObj
{
    int dummy;
};

struct BKNI_TaskObj
{
    int dummy;
};


BERR_Code 
BKNI_Init(void)
{
    return  BERR_SUCCESS;
}

void 
BKNI_Uninit(void)
{
    return;
}


BERR_Code 
BKNI_CreateMutex(BKNI_MutexHandle *pMutex)
{
    *pMutex = (BKNI_MutexHandle)&pMutex;
    return BERR_SUCCESS;
}

void
BKNI_DestroyMutex(BKNI_MutexHandle mutex)
{
    return ;
}

BERR_Code 
BKNI_TryAcquireMutex(BKNI_MutexHandle mutex)
{
    return BERR_SUCCESS;
}

BERR_Code 
BKNI_AcquireMutex(BKNI_MutexHandle mutex)
{
    return BERR_SUCCESS;
}

void
BKNI_ReleaseMutex(BKNI_MutexHandle mutex)
{
    return ;
}

void 
BKNI_Fail(void)
{
    volatile int i;


    i = *(int *)0; /* try to fail, take 1 */
    i = 0;
    i = 1/i;  /* try to fail, take 2 */

    for(;;) { } /* hang here */
}


int 
BKNI_Printf(const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = vprintf(fmt, arglist);
    va_end(arglist);

    return rc;
}


int 
BKNI_Snprintf(char *str, size_t len, const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = vsprintf(str, fmt, arglist);
    va_end(arglist);
    if ((size_t)rc > len) {
        BDBG_ERR(("Buffer overflow in the BKNI_Snprintf"));
    }
    return rc;
}

int 
BKNI_Vprintf(const char *fmt, va_list ap)
{
    return vprintf(fmt, ap);
}


void BKNI_Delay(unsigned int microsec)
{
    volatile long l;

    while(microsec--) {
          for(l=1000;l<0;l++) {}
    }
    return;
}

BERR_Code 
BKNI_Sleep(unsigned int millisec)
{
    BKNI_Delay(millisec*1000);
    return BERR_SUCCESS;
}


BERR_Code 
BKNI_CreateEvent(BKNI_EventHandle *pEvent)
{
    *pEvent = (BKNI_EventHandle)&pEvent;
    return BERR_SUCCESS;
}



void
BKNI_DestroyEvent(BKNI_EventHandle event)
{
    return ;
}

BERR_Code 
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    return BERR_TRACE(BERR_OS_ERROR);
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    return ;
}

void 
BKNI_EnterCriticalSection(void)
{
    return ;
}

void
BKNI_LeaveCriticalSection(void)
{
    return ;
}

void 
BDBG_AssertIsrContext(const char *filename, unsigned lineno)
{
    BSTD_UNUSED(filename);
    BSTD_UNUSED(lineno);
    return;
}

