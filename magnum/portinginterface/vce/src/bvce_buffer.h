/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */
#include "bmma.h"

#ifndef BVCE_BUFFER_H_
#define BVCE_BUFFER_H_

/* Handles */
typedef struct BVCE_P_Buffer *BVCE_P_Buffer_Handle;
typedef struct BVCE_P_Allocator *BVCE_P_Allocator_Handle;

/* Buffer */
typedef struct BVCE_P_Buffer_AllocSettings
{
   size_t uiSize;
   unsigned uiAlignment;
   bool bLockOffset;
   bool bLockAddress;
} BVCE_P_Buffer_AllocSettings;

void
BVCE_P_Buffer_GetDefaultAllocSettings(
   BVCE_P_Buffer_AllocSettings *pstAllocSettings
   );

BERR_Code
BVCE_P_Buffer_Alloc(
   BVCE_P_Allocator_Handle hAllocator,
   const BVCE_P_Buffer_AllocSettings *pstAllocSettings,
   BVCE_P_Buffer_Handle *phBuffer
   );

BMMA_Block_Handle
BVCE_P_Buffer_GetBlockHandle(
   BVCE_P_Buffer_Handle hBuffer
   );

size_t
BVCE_P_Buffer_GetSize(
   BVCE_P_Buffer_Handle hBuffer
   );

BMMA_DeviceOffset BVCE_P_Buffer_LockDeviceOffset(
   BVCE_P_Buffer_Handle hBuffer
   );

BMMA_DeviceOffset
BVCE_P_Buffer_GetDeviceOffset_isrsafe(
   BVCE_P_Buffer_Handle hBuffer
   );

unsigned
BVCE_P_Buffer_GetDeviceBaseOffset(
   BVCE_P_Buffer_Handle hBuffer
   );

void BVCE_P_Buffer_UnlockDeviceOffset(
   BVCE_P_Buffer_Handle hBuffer
   );

void* BVCE_P_Buffer_LockAddress(
   BVCE_P_Buffer_Handle hBuffer
   );

void*
BVCE_P_Buffer_GetAddress_isrsafe(
   BVCE_P_Buffer_Handle hBuffer
   );

void
BVCE_P_Buffer_FlushCache_isr(
   BVCE_P_Buffer_Handle hBuffer,
   void *pBuffer,
   size_t uiSize
   );

void BVCE_P_Buffer_UnlockAddress(
   BVCE_P_Buffer_Handle hBuffer
   );

void
BVCE_P_Buffer_Free(
   BVCE_P_Buffer_Handle hBuffer
   );

/* Allocator */
BERR_Code
BVCE_P_Allocator_Create(
   BMMA_Heap_Handle hHeap,
   const BVCE_P_Buffer_AllocSettings *pstAllocSettings, /* Non-Null Indicate Create a Sub-Heap Allocator */
   BVCE_P_Allocator_Handle *phAllocator
   );

size_t
BVCE_P_Allocator_GetSize(
   BVCE_P_Allocator_Handle hAllocator
   );

BMMA_DeviceOffset
BVCE_P_Allocator_GetDeviceOffset(
   BVCE_P_Allocator_Handle hAllocator
   );

void
BVCE_P_Allocator_Destroy(
   BVCE_P_Allocator_Handle hAllocator
   );

#endif /* BVCE_BUFFER_H_ */
