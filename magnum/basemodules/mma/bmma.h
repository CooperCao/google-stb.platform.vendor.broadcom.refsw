/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
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
#ifndef BMMA_H__
#define BMMA_H__

#include "bmma_types.h"
#if BMMA_USE_STUB
#include "bmma_stub.h"
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to provide API to allocate memory
and control memory mapping for software and hardware access.
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    A handle representing one heap of the memory allocator
****************************************************************************/
#ifndef BMMA_Heap_Handle
typedef struct BMMA_Heap *BMMA_Heap_Handle;
#endif

/***************************************************************************
Summary:
    A handle representing allocation in the heap.
****************************************************************************/
typedef struct BMMA_Block *BMMA_Block_Handle;

/***************************************************************************
Summary:
    Settings structure for allocation
See Also:
    BMMA_Alloc
****************************************************************************/
typedef struct BMMA_AllocationSettings {
    bool uncached; /* deprecated, if set to true then BMMA_GetUncached would return uncached address */
    unsigned boundary; /* If boundary is nonzero, blocks returned from BMMA_Alloc won't cross specified boundary (for example, if boundary is 4096, then allocated block, must not cross the page boundary). */
} BMMA_AllocationSettings;


/***************************************************************************
Summary:
    Fills in BMMA_AllocationSettings structure with default settings.
****************************************************************************/
void BMMA_GetDefaultAllocationSettings(BMMA_AllocationSettings *pSettings);

/***************************************************************************
Summary:
    Allocates memory from a heap.

Description:
    Returns a handle to an allocated block of memory.

Input:
    heap - heap to allocate from
    size - the number of bytes to allocate
    alignment - the alignment for allocated memory (in bytes should be power of 2). This alignment applies to the hardware address.
    pSettings - optional (can be NULL) pointer to the BMMA_AllocationSettings structure that describes additional constraints for allocated memory

    Note, if the size is zero, this function will return valid allocated memory block.

    Returned memory block can't be accessed by neither software or hardware, until it's pinned (locked) in memory  by calling the BMMA_Lock or BMMA_LockOffset functions.
    Both BMMA_Lock or BMMA_LockOffset are reference counted and when reference count reaches to 0, block could be relocated inside the heap.

Returns:
    o new memory allocation object, in case of success
    o NULL, in case of failure

See Also:
    BMMA_Free
    BMMA_Lock
    BMMA_LockOffset
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_Alloc(heap, size, alignment, pSettings) \
    BMMA_Alloc_tagged(heap, size, alignment, pSettings, BSTD_FILE, BSTD_LINE)
#else
BMMA_Block_Handle BMMA_Alloc(BMMA_Heap_Handle heap, size_t size, unsigned alignment, const BMMA_AllocationSettings *pSettings);
#endif

/***************************************************************************
Summary:
    Free memory allocated from heap.

Description:
    Any memory allocated with a call to BMMA_Alloc may be returned to the heap by calling this
    function.

    After the block has been freed, it is coalesced with any adjacent free
    blocks in order to maximize the size of free regions in the heap.

Input:
    block - allocation to free

See Also:
    BMMA_Free
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_Free(block) BMMA_Free_tagged(block, BSTD_FILE, BSTD_LINE)
#else
void BMMA_Free(BMMA_Block_Handle block);
#endif

/***************************************************************************
Summary:
     Allows software to access allocated memory.

Description:
    Memory allocated with a call to BMMA_Alloc can't be accessed by CPU until this function
    is called to establish mapping between allocated memory and CPU address space.

    This function returns a software address, which software could dereference to access allocated memory;

    This function return address in the CPU cached address space, so software should use BMMA_FlushCache
    as appropriate to establish coherence between CPU and any hardware that might be accessing the same memory block.

    After software doesn't need to access allocated block it should call BMMA_Unlock function, at which point
    address returned by this function may become invalid and shouldn't be used, and next call to this function may return
    different software address. Proper placing of BMMA_Lock/BMMA_Unlock functions is important in order to
    reduce utilization of limited resource - the CPU virtual address space as well as to allow heap compaction.

    Calls to this function and BMMA_Unlock are reference counted, and they could be nested.


Input:
    block - allocation to access

Returns:
    o software address on success
    o NULL in case of failure

See Also:
    BMMA_Unlock
    BMMA_FlushCache_isrsafe
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_Lock(block)  BMMA_Lock_tagged(block, BSTD_FILE, BSTD_LINE)
#else
void *BMMA_Lock(BMMA_Block_Handle block);
#endif


/***************************************************************************
Summary:
     Informs that address returned by BMMA_Lock is no longer used and could be recycled

See Also:
    BMMA_Lock
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_Unlock(block, addr)  BMMA_Unlock_tagged(block, addr, BSTD_FILE, BSTD_LINE)
#else
void BMMA_Unlock(BMMA_Block_Handle block, const void *addr);
#endif

/***************************************************************************
Summary:
    Allows hardware to access allocated memory.

Description:
    Memory allocated with a call to BMMA_Alloc can't be accessed by hardware until this function
    is called to pin allocated memory in place;

    This function returns an hardware address (device offset_, which software use to program hardware/


    After hardware doesn't need to access allocated block it should call BMMA_UnlockOffset function, at which point
    address returned by this function may become invalid and shouldn't be used, and next call to this function may return
    different hardware address. Proper placing of BMMA_LockOffset/BMMA_UnlockOffset functions is important in order to
    facilitate heap compaction.

    Since this function may not fail, its result should not be tested. In particularly, return value of '0'
    corresponds to a valid result.

    Calls to this function and BMMA_UnlockOffset are reference counted and they could be nested.


Input:
    block - allocation to access

Returns:
    o hardware address

See Also:
    BMMA_UnlockOffset
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_LockOffset(block) BMMA_LockOffset_tagged(block, BSTD_FILE, BSTD_LINE)
#else
BMMA_DeviceOffset BMMA_LockOffset(BMMA_Block_Handle block);
#endif

/***************************************************************************
Summary:
     Informs that address returned by BMMA_LockOffset_tagged is no longer used and allocation could be relocated
****************************************************************************/
#if BDBG_DEBUG_BUILD
#define BMMA_UnlockOffset(block, offset)  BMMA_UnlockOffset_tagged(block, offset, BSTD_FILE, BSTD_LINE)
#else
void BMMA_UnlockOffset(BMMA_Block_Handle block, BMMA_DeviceOffset offset);
#endif

/***************************************************************************
Summary:
    [Deprecated] Returns uncached address of allocated memory.

Description:
    This function used to return uncached address of allocated memory that is locked for the CPU access.

    For valid allocations that were allocated with BMMA_AllocationSettings.uncached and are locked for CPU access,
    this function would return an uncached address. For all other cases function has undefined behavior, in most
    of invalid cases function would return NULL.

Input:
    block - allocation to access

Returns:
    o uncached address

See Also:
    BMMA_Lock
    BMMA_Alloc

****************************************************************************/
void *BMMA_GetUncached(BMMA_Block_Handle block);

/***************************************************************************
Summary:
    Returns hardware address of allocated memory.

Description:
    This function used to return hardware address of allocated memory that is locked for the hardware access.

    For valid allocations that are locked this function would return an hardware address.
    For all other cases function has undefined behavior.

    Since this function may not fail, its result should not be tested. In particularly, return value of '0'
    corresponds to a valid result.

Input:
    block - allocation to access

Returns:
    o hardware address

See Also:
    BMMA_LockOffset
    BMMA_LockFromDevice

****************************************************************************/
BMMA_DeviceOffset BMMA_GetOffset_isr(BMMA_Block_Handle block);


/***************************************************************************
Summary:
    Allows software to maintain coherency with hardware accessing the same memory

Description:

    When software and hardware accessing the same memory location, hardware accesses routed directly to the memory and
    software access are routed through complicated buffer, that is called CPU cache. And proper use of this function
    is important in order to establish coherency (e.g. software and hardware to have consistent view of the memory content).

    This function flushed cache for a single allocation, and memory should be locked for software access prior to calling
    this function, and flushed region should reside inside the allocation.

    There are potentially three CPU caches you should be aware of:
        1) L1 cache - this is a write-back cache
        2) L2 cache - this is a write-back cache
        3) RAC (read ahead cache) - this is a read-only cache which are "read ahead" up to a 4K boundary

    BMMA_FlushCache_isrsafe will do a wback_invalidate of the L1 and L2 caches and an invalidate of the RAC.

    The following cache flush rules will allow software to maintain cache coherency.

    Rule 1: After hardware writes to RAM, you must flush the cache before the CPU reads from that memory. That is:

        1. Hardware writes to RAM
        2. Flush cache for that RAM
        3. CPU reads from that RAM

    Rule 2: After the CPU writes to RAM, you must flush the cache before Hardware does any access to that memory. That is:

        1. CPU writes to RAM
        2. Flush cache for that RAM
        3. Hardware reads from that RAM


Input:
    block - allocation to flush cache

See Also:
    BMMA_Lock
    BMMA_LockFromDevice
****************************************************************************/
void BMMA_FlushCache_isrsafe(BMMA_Block_Handle block, const void *addr, size_t size);

#define BMMA_FlushCache_isr  BMMA_FlushCache_isrsafe
#define BMMA_FlushCache      BMMA_FlushCache_isrsafe

/***************************************************************************
Summary:
    Allows software to hint that contents of the block is not important

Description:
    This function could be used as a hint to inform allocator that content of the allocated block
    doesn't have to be preserved. This could be a case, for example, for temporary buffers that.

    Proper use of this function allows to decrease overhead of compaction, by skipping the content
    copying when moving allocated blocks around.

    Any function call to pin memory (BMMA_Lock, BMMA_LockOffset or BMMA_LockFromDevice) would clear hint.

See Also:
    BMMA_Lock
    BMMA_LockOffset
    BMMA_LockFromDevice

****************************************************************************/
void BMMA_MarkDiscarable(BMMA_Block_Handle block);

#if BDBG_DEBUG_BUILD
/*{private}*****************************************************************
Summary:
    Private allocation function.
****************************************************************************/
BMMA_Block_Handle BMMA_Alloc_tagged(BMMA_Heap_Handle heap, size_t size, unsigned alignment, const BMMA_AllocationSettings *pSettings, const char *fname, unsigned line);
/*{private}*****************************************************************
Summary:
    Private free function.
****************************************************************************/
void BMMA_Free_tagged(BMMA_Block_Handle block, const char *fname, unsigned line);
/*{private}*****************************************************************
Summary:
    Private software lock function.
****************************************************************************/
void *BMMA_Lock_tagged(BMMA_Block_Handle block, const char *fname, unsigned line);
/*{private}*****************************************************************
Summary:
    Private software unlock function.
****************************************************************************/
void BMMA_Unlock_tagged(BMMA_Block_Handle block, const void *addr, const char *fname, unsigned line);
/*{private}*****************************************************************
Summary:
    Private hardware lock function.
****************************************************************************/
BMMA_DeviceOffset BMMA_LockOffset_tagged(BMMA_Block_Handle block, const char *fname, unsigned line);
/*{private}*****************************************************************
Summary:
    Private hardware unlock function.
****************************************************************************/
void BMMA_UnlockOffset_tagged(BMMA_Block_Handle block, BMMA_DeviceOffset offset, const char *fname, unsigned line);
#endif /* BDBG_DEBUG_BUILD */

#ifdef __cplusplus
} /* end extern "C" */
#endif



#endif /* #ifndef BMMA_H__ */
