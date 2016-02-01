/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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
 
#include <windows.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

static DWORD s_initCount = 0;
void
BDBG_P_InitializeTimeStamp(void)
{
    s_initCount = GetTickCount();
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
	DWORD tickCount;
	int hours, minutes, seconds, milliseconds;

    tickCount = GetTickCount() - s_initCount;
    milliseconds = tickCount % 1000;
    seconds = tickCount / 1000;

	
	/* Calculate the time	*/
	hours = seconds/3600;
	minutes = (seconds/60)%60;
	seconds %= 60;
		
	/* print the formatted time including the milliseconds	*/
	BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
	return;
}

// Modifications to BDBG data structures mus be done under lock 
// compatible with BKNI _isr code rules. Can't be BKNI_Mutex!
static BOOL s_Initialized=FALSE;
static CRITICAL_SECTION s_DbgLock;


BERR_Code BDBG_P_OsInit(void)
{
    /* g_mutex is statically initialized */
    return 0;
}

void BDBG_P_OsUninit(void)
{
}


void BDBG_P_Lock(BKNI_MutexHandle mutex)
{
    if(!s_Initialized)
    {
        InitializeCriticalSection(&s_DbgLock);
        s_Initialized=TRUE;
    }
    EnterCriticalSection(&s_DbgLock);
}

void BDBG_P_Unlock(BKNI_MutexHandle mutex)
{
    LeaveCriticalSection(&s_DbgLock);
}
/* End of file */
