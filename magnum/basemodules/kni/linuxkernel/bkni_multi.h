/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef BKNI_MULTI_H__
#define BKNI_MULTI_H__

#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BKNI_MutexObj *BKNI_MutexHandle;

#if BKNI_TRACK_MALLOCS
#define BKNI_CreateMutex(mutex) BKNI_CreateMutex_tagged(mutex, __FILE__, __LINE__)
BERR_Code BKNI_CreateMutex_tagged(BKNI_MutexHandle *mutex, const char *file, int line);

#define BKNI_DestroyMutex(mutex) BKNI_DestroyMutex_tagged(mutex, __FILE__, __LINE__)
void BKNI_DestroyMutex_tagged(BKNI_MutexHandle mutex, const char *file, int line);

#define BKNI_AcquireMutex(MUTEX) BKNI_AcquireMutex_tagged(MUTEX,__FILE__,__LINE__)
#define BKNI_TryAcquireMutex(MUTEX) BKNI_TryAcquireMutex_tagged(MUTEX,__FILE__,__LINE__)
#define BKNI_ReleaseMutex(MUTEX) BKNI_ReleaseMutex_tagged(MUTEX,__FILE__,__LINE__)

BERR_Code BKNI_AcquireMutex_tagged(BKNI_MutexHandle mutex, const char *file, unsigned line);
BERR_Code BKNI_TryAcquireMutex_tagged(BKNI_MutexHandle mutex, const char *file, unsigned line);
void BKNI_ReleaseMutex_tagged(BKNI_MutexHandle mutex, const char *file, unsigned line);
#else
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *mutex);
void BKNI_DestroyMutex(BKNI_MutexHandle mutex);
BERR_Code BKNI_AcquireMutex(BKNI_MutexHandle mutex);
BERR_Code BKNI_TryAcquireMutex(BKNI_MutexHandle mutex);
void BKNI_ReleaseMutex(BKNI_MutexHandle mutex);
#endif

typedef struct BKNI_MutexSettings {
    bool  suspended; 
} BKNI_MutexSettings;
void BKNI_GetMutexSettings(BKNI_MutexHandle mutex, BKNI_MutexSettings *pSettings);
BERR_Code BKNI_SetMutexSettings(BKNI_MutexHandle mutex, const BKNI_MutexSettings *pSettings);

#ifdef __cplusplus
}
#endif

#endif /* BKNI_MULTI_H__ */
