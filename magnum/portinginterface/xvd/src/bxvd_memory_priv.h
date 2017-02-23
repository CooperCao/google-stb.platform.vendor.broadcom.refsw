/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *   This module contains the prototypes for the XVD memory sub-manager.
 *
 ****************************************************************************/
#ifndef _BXVD_MEMORY_PRIV_H_
#define _BXVD_MEMORY_PRIV_H_

#include "bstd.h"
#include "bxvd.h"
#include "bmma_range.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BXVD_P_MEMORY_DEBUG_FUNCS 0

#define BXVD_P_MEMORY_MAX_ALLOCATIONS 17
#define BXVD_P_MEMORY_FREE_REGION_INDEX (-1)

typedef int32_t BXVD_P_Memory_RegionIndex;

typedef enum BXVD_P_Memory_DumpFlag
{
   BXVD_P_Memory_DumpFlag_eAll,
   BXVD_P_Memory_DumpFlag_eAllocatedOnly
} BXVD_P_Memory_DumpFlag;

typedef enum BXVD_P_Memory_Protection
{
   BXVD_P_Memory_Protection_eProtect,
   BXVD_P_Memory_Protection_eDontProtect
} BXVD_P_Memory_Protection;

typedef struct BXVD_P_AllocatedRegion
{
      bool                             bRegionFree;
      BXVD_P_Memory_RegionIndex        iRegionIndex;
      uint32_t                         uiRegionAlignment;
      BMMA_RangeAllocator_Block_Handle hBMMA_RangeAllocBlock;
      BMMA_DeviceOffset                RegionAddr;
      uint32_t                         uiRegionSize;
      uint32_t                         uiRegionTag;
} BXVD_P_AllocatedRegion;

/*
 * XVD memory sub-manager context structure
 */
typedef struct BXVD_P_Memory
{
      BXVD_Handle                 hXvd;
      BMMA_RangeAllocator_Handle  hBMMA_RangeAllocator;
      uint32_t                    uiRegionCount;
      uint32_t                    uiBlockSize;
      BMMA_DeviceOffset           BlockBaseAddr;
      BXVD_P_AllocatedRegion     aAllocatedRegions[BXVD_P_MEMORY_MAX_ALLOCATIONS];
      uint32_t                   uiFreeSpace;
} BXVD_P_Memory;


/*
 * XVD memory sub-manager context handle
 */
typedef struct BXVD_P_Memory *BXVD_P_MemoryHandle;

/***************************************************************************
Summary:
  Open a BXVD_P_Memory instance.

Description:
  This API is used to open an instance of the BXVD_Memory sub-manager.

Returns:
  BERR_SUCCESS

See Also:
  BXVD_P_Memory_Close
  BXVD_P_Memory_Allocate
  BXVD_P_Memory_Free
  BXVD_P_Memory_GetDecContextMem
  BXVD_P_Memory_GetCabacMem
  BXVD_P_Memory_GetPicBufMem
****************************************************************************/
BERR_Code BXVD_P_Memory_Open(BXVD_Handle hXvd,
                             BXVD_P_MemoryHandle *phXvdMem,
                             BMMA_DeviceOffset pAddr,
                             uint32_t uiSize,
                             BXVD_P_Memory_Protection eProtect);

/***************************************************************************
Summary:
  Close a BXVD_P_Memory instance.

Description:
  This API is used to close an instance of the BXD_Memory sub-manager opened
  via a BXVD_P_Memory_Open call.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_P_Memory_Open

****************************************************************************/
BERR_Code BXVD_P_Memory_Close(BXVD_P_MemoryHandle hXvdMem);

/***************************************************************************
Summary:
  Allocates a region of memory.

Description:
  Allocate a region of memory of size uiAllocationSize and alignment
  uiAlignment. An optional user defined identification tag can be supplied
  via the uiRegionTag parameter but this should not be confused with the
  allocator supplied region index.

Returns:
  Pointer to allocated memory or NULL on error.

See Also:
  BXVD_P_Memory_Open
  BXVD_P_Memory_AllocateAtIndex
  BXVD_P_Memory_Free

****************************************************************************/
BMMA_DeviceOffset BXVD_P_Memory_Allocate(BXVD_P_MemoryHandle hXvdMem,
                                         uint32_t uiAllocationSize,
                                         uint32_t uiAlignment,
                                         uint32_t uiRegionTag);

/***************************************************************************
Summary:
  Free a heap region based on its address.

Description:
  Free a region of memory allocated by a BXVD_P_Memory_Allocate call. This
  API frees the region and returns it to the free region pool.

Returns:
  BERR_SUCCESS

See Also:
  BXVD_P_Memory_Open
  BXVD_P_Memory_Allocate
  BXVD_P_Memory_AllocateAtIndex

****************************************************************************/
BERR_Code BXVD_P_Memory_Free(BXVD_P_MemoryHandle hXvdMem,
                             BMMA_DeviceOffset pvAddr);

/***************************************************************************
Summary:
  Dump a memory region.

Description:
  This function dumps diagnostic information about the region or regions
  referred to by hXvdMem.
  If the eDumpFlag is set to BXVD_P_Memory_DumpFlag_eAll, all regions will
  be dumped regardless of their allocation status. If the eDumpFlag is set
  to BXVD_P_Memory_DumpFlag_eAllocatedOnly, just the regions that have been
  allocated will be dumped.

Returns:
  Nothing

See Also:
  BXVD_P_Memory_Open
  BXVD_P_Memory_Allocate
  BXVD_P_Memory_AllocateAtIndex
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXVD_P_Memory_Dump(BXVD_P_MemoryHandle hXvdMem,
                             BXVD_P_Memory_DumpFlag eDumpFlag);
#endif

/***************************************************************************
Summary:
  Return a region's allocation status given its region index.

Description:
  This function will return a region's allocation status based on its index.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_P_Memory_Allocate
  BXVD_P_Memory_GetRegionIndex
****************************************************************************/
BERR_Code BXVD_P_Memory_IsRegionAllocated(BXVD_P_MemoryHandle hXvdMem,
                                          BXVD_P_Memory_RegionIndex regionIndex,
                                          bool *pbAllocated);

#ifdef __cplusplus
}
#endif

#endif /* _BXVD_MEMORY_PRIV_H_ */
