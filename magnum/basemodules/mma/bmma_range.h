/***************************************************************************
 * Copyright (C) 2012-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#ifndef BMMA_RANGE_H__
#define BMMA_RANGE_H__

#include "bmma_types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct BMMA_RangeAllocator *BMMA_RangeAllocator_Handle;
typedef struct BMMA_RangeAllocator_Block *BMMA_RangeAllocator_Block_Handle;

typedef struct BMMA_RangeAllocator_CompactionStatus {
    unsigned freeBlocks;
    size_t largestFreeBlock;
    size_t bytesCopied;
} BMMA_RangeAllocator_CompactionStatus;

typedef struct BMMA_RangeAllocator_Region {
    BMMA_DeviceOffset base;
    BMMA_Size length;
} BMMA_RangeAllocator_Region;

typedef struct BMMA_RangeAllocator_BlockSettings {
    unsigned alignment;
    unsigned boundary;
#if BDBG_DEBUG_BUILD
    const char *fname;
    unsigned line;
#endif
} BMMA_RangeAllocator_BlockSettings;

typedef struct BMMA_RangeAllocator_CreateSettings {
    BMMA_DeviceOffset base;
    size_t size;
    size_t allocationHeader;
    unsigned minAlignment;
    bool silent;
    bool printLeakedRanges;
    bool verbose;
    void *context;
    size_t allocatorMinFreeSize;
    int (*allocator)(void *context,const BMMA_RangeAllocator_Region **freeRegions, unsigned nFreeRegions, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation);
    bool (*relocatable)(void *context, void *user_block);
    BERR_Code (*relocate)(void *context, void *user_block, BMMA_DeviceOffset dst, BMMA_DeviceOffset src, size_t length);
    bool (*advance)(void *context, const BMMA_RangeAllocator_CompactionStatus *status);
} BMMA_RangeAllocator_CreateSettings;

typedef struct BMMA_RangeAllocator_Status
{
    size_t allocatedSpace;
    size_t freeSpace;
    size_t highWatermark;
    size_t largestFree; /* Size of the largest free block in bytes */
    unsigned allocatedBlocks;
    unsigned freeBlocks;
    bool dynamic; /* regions were removed or added to the heap */
} BMMA_RangeAllocator_Status;

typedef enum BMMA_RangeAllocator_IteratorOrder {
    BMMA_RangeAllocator_IteratorOrder_eUnordered,
    BMMA_RangeAllocator_IteratorOrder_eLargestFirst,
    BMMA_RangeAllocator_IteratorOrder_eSmallestFirst
} BMMA_RangeAllocator_IteratorOrder;

typedef struct BMMA_RangeAllocator_IteratorConfig {
    bool freeOnly;
    BMMA_RangeAllocator_IteratorOrder order;
} BMMA_RangeAllocator_IteratorConfig;


void BMMA_RangeAllocator_GetDefaultCreateSettings(BMMA_RangeAllocator_CreateSettings *settings);
BERR_Code BMMA_RangeAllocator_Create(BMMA_RangeAllocator_Handle *allocator, const BMMA_RangeAllocator_CreateSettings *settings);
void BMMA_RangeAllocator_Destroy(BMMA_RangeAllocator_Handle allocator);
void BMMA_RangeAllocator_GetDefaultAllocationSettings(BMMA_RangeAllocator_BlockSettings *settings);
BERR_Code BMMA_RangeAllocator_Alloc(BMMA_RangeAllocator_Handle allocator, BMMA_RangeAllocator_Block_Handle *block, size_t size, const BMMA_RangeAllocator_BlockSettings *settings);
BMMA_DeviceOffset BMMA_RangeAllocator_GetAllocationBase_isrsafe(BMMA_RangeAllocator_Block_Handle allocation);
#define BMMA_RangeAllocator_GetAllocationBase(handle) BMMA_RangeAllocator_GetAllocationBase_isrsafe(handle)
size_t BMMA_RangeAllocator_GetAllocationSize_isrsafe(BMMA_RangeAllocator_Block_Handle allocation);
#define BMMA_RangeAllocator_GetAllocationSize(handle) BMMA_RangeAllocator_GetAllocationSize_isrsafe(handle)
void *BMMA_RangeAllocator_GetAllocationHeader(BMMA_RangeAllocator_Block_Handle allocation);
void BMMA_RangeAllocator_Free(BMMA_RangeAllocator_Handle allocator, BMMA_RangeAllocator_Block_Handle  block);
void BMMA_RangeAllocator_GetStatus(BMMA_RangeAllocator_Handle allocator, BMMA_RangeAllocator_Status *status);
BERR_Code BMMA_RangeAllocator_Compact(BMMA_RangeAllocator_Handle allocator, BMMA_RangeAllocator_CompactionStatus *status);
BERR_Code BMMA_RangeAllocator_Iterate(BMMA_RangeAllocator_Handle allocator, bool (*iterator)(void *context, void *block_header, const BMMA_RangeAllocator_Region *region, bool boundary), void *context, const BMMA_RangeAllocator_IteratorConfig *config);
void BMMA_RangeAllocator_Verify(BMMA_RangeAllocator_Handle allocator, bool print);
void BMMA_RangeAllocator_ResetHighWatermark(BMMA_RangeAllocator_Handle allocator);
bool BMMA_RangeAllocator_AllocateInRegion_InFront(const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation);
bool BMMA_RangeAllocator_AllocateInRegion_InBack(const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation);

BERR_Code BMMA_RangeAllocator_AddRegion(BMMA_RangeAllocator_Handle allocator, const BMMA_RangeAllocator_Region *region);

#define BMMA_ERR_REGION_BUSY    BERR_MAKE_CODE(BERR_MMA_ID, 0)  /* region is used for active allocation */
BERR_Code BMMA_RangeAllocator_RemoveRegion(BMMA_RangeAllocator_Handle allocator, const BMMA_RangeAllocator_Region *region);

BERR_Code BMMA_RangeAllocator_CreateBlock(BMMA_RangeAllocator_Block_Handle *block, BMMA_DeviceOffset base, size_t size);
void BMMA_RangeAllocator_DestroyBlock(BMMA_RangeAllocator_Block_Handle block);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* #ifndef BMMA_RANGE_H__ */

/* End of File */
