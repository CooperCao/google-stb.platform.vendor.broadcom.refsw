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
#ifndef BKNI_TAGGED_H
#define BKNI_TAGGED_H

#include "bkni_ntg.h"

#if BKNI_USE_TAGGED_API

void BKNI_DelayTagged(int microsec, const char *filename, unsigned lineno);
void *BKNI_MallocTagged(size_t size, const char *filename, unsigned lineno);
void BKNI_FreeTagged(void *mem, const char *filename, unsigned lineno);
BERR_Code BKNI_SleepTagged(int millisec, const char *filename, unsigned lineno);
BERR_Code BKNI_CreateEventTagged(BKNI_EventHandle *event, bool initiallySignalled, const char *filename, unsigned lineno);
BERR_Code BKNI_DestroyEventTagged(BKNI_EventHandle event, const char *filename, unsigned lineno);
BERR_Code BKNI_WaitForEventTagged(BKNI_EventHandle event, int timeoutMsec, const char *filename, unsigned lineno);
BERR_Code BKNI_SetEventTagged(BKNI_Event event, const char *filename, unsigned lineno);
BERR_Code BKNI_EnterCriticalSectionTagged(const char *filename, unsigned lineno);
BERR_Code BKNI_LeaveCriticalSectionTagged(const char *filename, unsigned lineno);

#define BKNI_Delay(microsec) BKNI_DelayTagged(microsec, BSTD_FILE, BSTD_LINE)
#define BKNI_Malloc(size) BKNI_MallocTagged(size, BSTD_FILE, BSTD_LINE)
#define BKNI_Free(mem) BKNI_FreeTagged(mem, BSTD_FILE, BSTD_LINE)
#define BKNI_Sleep(millisec) BKNI_SleepTagged(millisec, BSTD_FILE, BSTD_LINE)
#define BKNI_CreateEvent(event, initiallySignalled) BKNI_CreateEventTagged(event, initiallySignalled, BSTD_FILE, BSTD_LINE)
#define BKNI_DestroyEvent(event) BKNI_DestroyEventTagged(event, BSTD_FILE, BSTD_LINE)
#define BKNI_WaitForEvent(event, timeoutMsec) BKNI_WaitForEventTagged(event, timeoutMsec, BSTD_FILE, BSTD_LINE)
#define BKNI_SetEvent(event)  BKNI_SetEventTagged(event, BSTD_FILE, BSTD_LINE)
#define BKNI_EnterCriticalSection() BKNI_EnterCriticalSectionTagged(BSTD_FILE, BSTD_LINE)
#define BKNI_LeaveCriticalSection() BKNI_LeaveCriticalSectionTagged(BSTD_FILE, BSTD_LINE)

#endif  /* BKNI_USE_TAGGED_API */

#endif /* BKNI_TAGGED_H */