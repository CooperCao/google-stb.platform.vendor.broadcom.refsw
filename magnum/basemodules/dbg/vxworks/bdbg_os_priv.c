/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
		BDBG_P_PrintString_isrsafe("### debug: clock_gettime returned %d, ignored", rc);
	}
	else {
		pTime->tv_sec  = vxworks_time.tv_sec;
		pTime->tv_usec = vxworks_time.tv_nsec / 1000;	/* convert to usec */
		if (pTime->tv_usec >= 1000000)	 /* if this isn't right, things go bad downstream */
		{
			BDBG_P_PrintString_isrsafe("!!! Assert (time->tv_usec >= 1000000) Failed at %s:%d\n", __FILE__, __LINE__);
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
BDBG_P_GetTimeStamp_isrsafe(char *timeStamp, int size_t)
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
