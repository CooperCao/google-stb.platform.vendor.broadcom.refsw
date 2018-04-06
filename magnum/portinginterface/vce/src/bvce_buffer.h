/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVCE_BUFFER_H_
#define BVCE_BUFFER_H_

/* Handles */
typedef struct BVCE_P_Buffer *BVCE_P_Buffer_Handle;
typedef struct BVCE_P_Allocator *BVCE_P_Allocator_Handle;

/* Buffer */
typedef struct BVCE_P_Buffer_AllocSettings
{
   unsigned uiSize;
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

unsigned
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
BVCE_P_Buffer_FlushCache_isrsafe(
   BVCE_P_Buffer_Handle hBuffer,
   void *pBuffer,
   unsigned uiSize
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

unsigned
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
