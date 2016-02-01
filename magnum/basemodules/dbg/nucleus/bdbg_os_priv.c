/***************************************************************************
 *	   Copyright (c) 2006, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "nucleus.h"

static UNSIGNED initTime;


void BDBG_P_InitializeTimeStamp(void)
{
    initTime = NU_Retrieve_Clock();
}

void BDBG_P_GetTimeStamp(char *timeStamp, int size)
{
    UNSIGNED currTime;
    currTime = NU_Retrieve_Clock();
    currTime -= initTime;

    BKNI_Snprintf(timeStamp, size, "%010u", currTime);
    return;
}

void BDBG_P_Lock(BKNI_MutexHandle mutex)
{
	BKNI_AcquireMutex(mutex);
	return;
}

void BDBG_P_Unlock(BKNI_MutexHandle mutex)
{
	BKNI_ReleaseMutex(mutex);
	return;
}
