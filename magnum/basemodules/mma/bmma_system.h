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
 *
 ***************************************************************************/
#ifndef BMMA_SYSTEM_H__
#define BMMA_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    A handle representing one memory allocator
****************************************************************************/
typedef struct BMMA_Allocator *BMMA_Handle;


/***************************************************************************
Summary:
    Settings structure for allocator
See Also:
    BMMA_Create
****************************************************************************/
typedef struct BMMA_CreateSettings {
    void (*flush_cache)(const void *addr, size_t length); /* function to flush CPU cache, it should be callable in both ISR and non-ISR contexts */
} BMMA_CreateSettings;

/***************************************************************************
Summary:
    Fills in BMMA_CreateSettings structure with default settings.
****************************************************************************/
void BMMA_GetDefaultCreateSettings(BMMA_CreateSettings *settings);

/***************************************************************************
Summary:
    Instantiates new allocator
****************************************************************************/
BERR_Code BMMA_Create(BMMA_Handle *allocator, const BMMA_CreateSettings *settings);

/***************************************************************************
Summary:
    Releases resources used by the allocator
****************************************************************************/
void BMMA_Destroy(BMMA_Handle allocator);

/***************************************************************************
Summary:
    Structure to report progress of compaction
See Also:
    BMMA_Heap_Compact
****************************************************************************/
typedef struct BMMA_Heap_CompactionStatus {
    unsigned freeBlocks;
    size_t largestFreeBlock;
    size_t bytesCopied;
} BMMA_Heap_CompactionStatus;

/***************************************************************************
Summary:
    Settings structure for heap
See Also:
    BMMA_Create
****************************************************************************/
typedef struct BMMA_Heap_CreateSettings {
    BMMA_DeviceOffset base;
    size_t size;
    unsigned minAlignment;
    struct {
        bool failSafe;
        bool silent; /* don't print error if allocation fails */
    } flags;
    size_t mmapStateSize; /* extra CPU accessible space reserved per-allocation to handle memory mapping requests */
    void *base_uncached; /* deprecated, if not NULL, corresponds to base address of statically mapped uncached address, it allows memory allocation with BMMA_AllocationSettings.uncached */
    void *context;
    BERR_Code (*relocate)(void *context, BMMA_DeviceOffset dst, BMMA_DeviceOffset src, size_t length); /* if not NULL,  then this function would be called to copy memory during compaction, otherwise it would mmap memory and use CPU to copy. if copy can't be performed and error returned, then compaction stops, heap remains valid */
    bool (*compaction_advance)(void *context, const BMMA_Heap_CompactionStatus *status); /* this function called on each step of compaction, if function return false, then compaction stops, heaps remains valid */
    bool (*out_of_memory)(void *context, size_t size, const BMMA_AllocationSettings *pSettings, unsigned iteration); /* this function called when block of memory can't be allocated, and if it returns 'true', allocator would  start compaction cycles and then re-try allocation */
    void *(*mmap)(void *context, void *state, BMMA_DeviceOffset base, size_t length);
    void (*unmap)(void *context, void *state, void *ptr, size_t length);
    BERR_Code (*alloc)(void *cnxt, BMMA_DeviceOffset base, size_t size, const char *fname, unsigned line); /* callback function called when new block was allocated */
    void (*free)(void *cnxt, BMMA_DeviceOffset base, size_t size); /* callback function called when block was deallocated */
} BMMA_Heap_CreateSettings;

/***************************************************************************
Summary:
    Fast Status structure for heap
Descrption:
    Fast status returns information about heap that could be obtained without traversing all freed/allocated blocks
See Also:
    BMMA_Heap_GetStatus
****************************************************************************/
typedef struct BMMA_Heap_FastStatus {
    size_t   size;               /* Size of the heap in bytes. */
    unsigned alignment;          /* Heap alignment (in bytes). */
    size_t   totalAllocated;     /* Total allocated memory in heap */
    unsigned numAllocated;       /* Number of allocated blocks */
    size_t   highWatermark;      /* Most memory used at one time */
    size_t   totalFree;          /* Total free memory in heap.  This is the combined size of all free
                                    blocks.  Just because a certain amount of memory is free does not
                                    mean that amount can be allocated. Allocated memory must still fit
                                    within a block, but this provides information on the overall usage of
                                    memory in a system. */
    unsigned numFree;            /* Number of free blocks */
    size_t   largestFree;        /* Size of the largest free block in bytes */
} BMMA_Heap_FastStatus;

/***************************************************************************
Summary:
    Fast Status structure for heap
Descrption:
    Fast status returns information about heap that is not available in the BMMA_Heap_FastStatus
    and requires traversing of all freed/allocated blocks. E.g. it could be slow for heap with large number
    of allocated memory blocks.
See Also:
    BMMA_Heap_GetStatus
****************************************************************************/
typedef struct BMMA_Heap_SlowStatus {
    size_t        largestAllocated;   /* Size of the largest allocated block in bytes */
    size_t        smallestAllocated;  /* Size of the smallest allocated block in bytes */
    size_t        smallestFree;       /* Size of the smallest free block in bytes */
} BMMA_Heap_SlowStatus;

/***************************************************************************
Summary:
    Fills in BMMA_Heap_CreateSettings structure with default settings.
****************************************************************************/
void BMMA_Heap_GetDefaultCreateSettings(BMMA_Heap_CreateSettings *settings);

/***************************************************************************
Summary:
    Instantiates new heap
****************************************************************************/
BERR_Code BMMA_Heap_Create(BMMA_Heap_Handle *heap, BMMA_Handle allocator, const BMMA_Heap_CreateSettings *settings);

/***************************************************************************
Summary:
    Releases resources used by the heap
****************************************************************************/
void BMMA_Heap_Destroy(BMMA_Heap_Handle heap);

/***************************************************************************
Summary:
    Starts heap compaction
****************************************************************************/
BERR_Code BMMA_Heap_Compact(BMMA_Heap_Handle heap, BMMA_Heap_CompactionStatus *status);

/***************************************************************************
Summary:
    Starts heap compaction
****************************************************************************/
BERR_Code BMMA_Heap_GetStatus(BMMA_Heap_Handle heap,
        BMMA_Heap_FastStatus *fastStatus,
        BMMA_Heap_SlowStatus *slowStatus /* optional pointer to BMMA_Heap_SlowStatus */
        );

/*{private}*****************************************************************
Summary:
    Private fake allocation function.
****************************************************************************/
#if BDBG_DEBUG_BUILD
BMMA_Block_Handle BMMA_Block_Create_tagged(BMMA_Handle allocator, BMMA_DeviceOffset offset, size_t size, void *address, const char *fname, unsigned line);
#else
BMMA_Block_Handle BMMA_Block_Create(BMMA_Handle allocator, BMMA_DeviceOffset offset, size_t size, void *address);
#endif

/***************************************************************************
Summary:
    Create new BMMA_BlockHandle

Description:
    Returns a handle for already allocated memory

    Purpose of this function is to provide valid BMMA_BlockHandle for memory that it was already allocated by some other means.

    Note, if the size is zero, this function will return valid memory block.

    BMMA_BlockHandle created by this function could be use the same way as BMMA_BlockHandle returned by the BMMA_Alloc function,
    including use of BMMA_Free to destroy BMMA_BlockHandle.

Input:
    allocator - the BMMA memory allocator handle
    offset - device offset of already allocated memory, this is the value to be returned by the BMMA_LockOffset function
    size - the number of allocated bytes
    address - virtual address of already allocated memory, this is the value to be returned by BMMA_Lock function, optional.


Returns:
    o new memory allocation object, in case of success
    o NULL, in case of failure

See Also:
    BMMA_Alloc
    BMMA_Free
    BMMA_Lock
    BMMA_LockOffset
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_Block_Create(allocator, offset, size, address) BMMA_Block_Create_tagged(allocator, offset, size, address, BSTD_FILE, BSTD_LINE)
#endif

/***************************************************************************
Summary:
    Increases reference counter BMMA_BlockHandle

Description:
    Purpose of this function is to manage access to shared memory allocation

See Also:
    BMMA_Free
****************************************************************************/
void BMMA_Block_Acquire(BMMA_Handle allocator, BMMA_Block_Handle block);

/***************************************************************************
Summary:
    BMMA_Block_Release is the same as BMMA_Block_Free
****************************************************************************/
#define BMMA_Block_Release(b) BMMA_Block_Free(b)

/***************************************************************************
Summary:
    Dumps to the debug output all active allocations
****************************************************************************/
void BMMA_Dbg_DumpHeap(BMMA_Heap_Handle heap);

/***************************************************************************
Summary:
    Validates integrity of internal data structures
****************************************************************************/
void BMMA_Dbg_ValidateHeap(BMMA_Heap_Handle heap);

/***************************************************************************
Summary:
    Resets high memory watermark tracked by the heap.
****************************************************************************/
void BMMA_Heap_ResetHighWatermark(BMMA_Heap_Handle heap);

typedef struct BMMA_BlockProperties {
    BMMA_Heap_Handle heap;
    size_t size;
} BMMA_BlockProperties;

void BMMA_Block_GetProperties(BMMA_Block_Handle block, BMMA_BlockProperties *properties);

BERR_Code BMMA_Heap_AddRegion(BMMA_Heap_Handle heap, BMMA_DeviceOffset base, size_t size);
BERR_Code BMMA_Heap_RemoveRegion(BMMA_Heap_Handle heap, BMMA_DeviceOffset base, size_t size);
typedef struct BMMA_Heap_FreeRegion {
    BMMA_DeviceOffset base;
    size_t length;
    bool boundary;
} BMMA_Heap_FreeRegion;

void BMMA_Heap_GetFreeRegions(BMMA_Heap_Handle heap, struct BMMA_Heap_FreeRegion *regions, unsigned numEntries, unsigned *pNumReturned);

#endif /* #ifndef BMMA_SYSTEM_H__ */

/* End of File */
