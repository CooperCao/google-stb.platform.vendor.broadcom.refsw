/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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

#include <windows.h>

static CRITICAL_SECTION g_mutex;

void
BDBG_P_InitializeTimeStamp(void)
{
	return;
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
	*timeStamp='\0';
	return;
}

BERR_Code BDBG_P_OsInit(void)
{
   InitializeCriticalSection(&g_mutex);
   return BERR_SUCCESS;
}

void BDBG_P_OsUninit(void)
{
   DeleteCriticalSection(&g_mutex);
}

void BDBG_P_Lock(void)
{
	EnterCriticalSection(&g_mutex);
	return;
}

void BDBG_P_Unlock(void)
{
	LeaveCriticalSection(&g_mutex);
	return;
}

