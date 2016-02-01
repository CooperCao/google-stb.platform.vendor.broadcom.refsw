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
#ifndef BKNI_MULTI_TAGGED_H
#define BKNI_MULTI_TAGGED_H

#include "bkni_multi_ntg.h"

#if BKNI_USE_TAGGED_API

BERR_Code BKNI_CreateMutexTagged(BKNI_MutexHandle *mutex, const char *filename, unsigned lineno);
BERR_Code BKNI_DestroyMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno);
BERR_Code BKNI_AcquireMutexTagged(BKNI_MutexHandle mutex, int timeoutMillisec, const char *filename, unsigned lineno);
BERR_Code BKNI_ReleaseMutexTagged(BKNI_MutexHandle mutex, const char *filename, unsigned lineno);
BERR_Code BKNI_CreateTaskTagged(BKNI_TaskHandle task,BKNI_TaskFunction task_fn, BKNI_TaskPriority priority, void *data, const char *filename, unsigned lineno);
BERR_Code BKNI_DestroyTaskTagged(BKNI_TaskHandle task, int *returnCode, const char *filename, unsigned lineno);

#define BKNI_CreateMutex(mutex) BKNI_CreateMutexTagged(mutex, BSTD_FILE, BSTD_LINE)
#define BKNI_DestroyMutex(mutex) BKNI_DestroyMutexTagged(mutex, BSTD_FILE, BSTD_LINE)
#define BKNI_AcquireMutex(mutex, timeoutMillisec) BKNI_AcquireMutexTagged(mutex, timeoutMillisec, BSTD_FILE, BSTD_LINE)
#define BKNI_ReleaseMutex(mutex) BKNI_ReleaseMutexTagged(mutex, BSTD_FILE, BSTD_LINE)
#define BKNI_CreateTask(task, task_fn, priority, data) BKNI_CreateTaskTagged(task, task_fn, priority, data, BSTD_FILE, BSTD_LINE)
#define BKNI_DestroyTask(task, returnCode) BKNI_DestroyTaskTagged(task, returnCode, BSTD_FILE, BSTD_LINE)

#endif /* BKNI_USE_TAGGED_API */

#endif /* BKNI_MULTI_TAGGED_H */

