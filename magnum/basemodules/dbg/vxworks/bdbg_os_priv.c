/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
#include "semLib.h"
#include <timers.h>

/* platform spefic type for timestamps */
typedef struct vxworks_timeval {
	uint32_t	tv_sec;     /* seconds */
	uint32_t	tv_usec;    /* microseconds */
}vxworks_timeval;

static vxworks_timeval initTimeStamp;
static SEM_ID	g_bdbg_crit_lock;
static int g_inDbgCriticalSection;

static void
BDBG_P_GetTime(vxworks_timeval *pTime)
{
	int rc;
	struct timespec vxworks_time;

	rc = clock_gettime(CLOCK_REALTIME, &vxworks_time);
	if (rc!=0) {
		BDBG_P_PrintString("### debug: clock_gettime returned %d, ignored", rc);
	}
	else {
		pTime->tv_sec  = vxworks_time.tv_sec;
		pTime->tv_usec = vxworks_time.tv_nsec / 1000;	/* convert to usec */
		if (pTime->tv_usec >= 1000000)	 /* if this isn't right, things go bad downstream */
		{
			BDBG_P_PrintString("!!! Assert (time->tv_usec >= 1000000) Failed at %s:%d\n", __FILE__, __LINE__);
		BKNI_Fail();
	}
	}
	return;
}

void
BDBG_P_InitializeTimeStamp(void)
{
	BDBG_P_GetTime(&initTimeStamp);
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
  vxworks_timeval currentTime;
	int hours, minutes, seconds;
 	int milliseconds;
	int rc;

	BDBG_P_GetTime(&currentTime);

	if (currentTime.tv_usec < initTimeStamp.tv_usec)
	{
		milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec + 1000000)/1000;
		currentTime.tv_sec--;
	}
	else	{
		milliseconds = (currentTime.tv_usec - initTimeStamp.tv_usec)/1000;
	}

	/* Calculate the time	*/
	hours = (currentTime.tv_sec - initTimeStamp.tv_sec)/3600;
	minutes = (((currentTime.tv_sec - initTimeStamp.tv_sec)/60))%60;
	seconds = (currentTime.tv_sec - initTimeStamp.tv_sec)%60;

	/* print the formatted time including the milliseconds	*/
	rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
	return;
}

BERR_Code BDBG_P_OsInit(void)
{
	BERR_Code retVal = BERR_SUCCESS;
	
	g_bdbg_crit_lock = semMCreate((SEM_Q_PRIORITY | SEM_DELETE_SAFE));
	if( !g_bdbg_crit_lock )
	{
		retVal = BERR_OS_ERROR;
	}
	g_inDbgCriticalSection = 0;
	return retVal;
}

void BDBG_P_OsUninit(void)
{
	int errCode;

	errCode = semDelete( g_bdbg_crit_lock );
	BDBG_ASSERT(0 == errCode);
}

void BDBG_P_Lock(void)
{
	int errCode;

	errCode = semTake( g_bdbg_crit_lock, WAIT_FOREVER );
	BDBG_ASSERT(0 == errCode);
	BDBG_ASSERT( !g_inDbgCriticalSection );
	
	g_inDbgCriticalSection = 1;
	return;
}

void BDBG_P_Unlock(void)
{
	int errCode;
	BDBG_ASSERT( g_inDbgCriticalSection );
	g_inDbgCriticalSection = 0;

	errCode = semGive( g_bdbg_crit_lock );
	BDBG_ASSERT(0 == errCode);
	return;
}

/* End of file */
