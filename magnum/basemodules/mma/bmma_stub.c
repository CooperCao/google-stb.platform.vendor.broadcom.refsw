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
#include "bstd.h"
#include "bmma.h"
#include "bmma_system.h"
#include "bkni.h"
#include "bmem.h"

BDBG_MODULE(BMMA_Stub);

void BMMA_GetDefaultAllocationSettings(BMMA_AllocationSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->uncached = false;
    return;
}

struct BMMA_Block {
    bool bmem;
    bool uncached;
    uint16_t ref_cnt; /* set to 1 by Alloc, increased by Acquire and decreased by Free, when goes to 0 memory freed */
    union {
        struct {
            BMEM_Heap_Handle heap;
            void *addr; /* uncached */
        } bmem;
        struct {
            void *addr; /* cached */
            uint32_t offset;
        } wrapper;
    } u;
};

static BERR_Code BMMA_P_ConvertAlignment(unsigned alignment, unsigned *pAlignmentExponent)
{
    unsigned i;
    if ( alignment == 0 )
    {
        /* Default alignment */
        i = 0;
    }
    else
    {
        for ( i = 0; i < (sizeof(size_t)*8); i++ )
        {
            if ( alignment == 1UL<<i )
            {
                break;
            }
        }

        if ( i >= sizeof(size_t)*8 )
        {
            BDBG_ERR(("Invalid alignment (%d) specified", alignment));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    *pAlignmentExponent = i;
    return 0;
}

#if BDBG_DEBUG_BUILD
BMMA_Block_Handle BMMA_Alloc_tagged(BMMA_Heap_Handle heap, size_t size, unsigned alignment, const BMMA_AllocationSettings *settings, const char *fname, unsigned line)
#else
BMMA_Block_Handle BMMA_Alloc(BMMA_Heap_Handle heap, size_t size, unsigned alignment, const BMMA_AllocationSettings *settings)
#endif
{
    BMEM_Heap_Handle memHeap = (BMEM_Heap_Handle)heap;
    BERR_Code rc;
    BMMA_Block_Handle block;
    block = BKNI_Malloc(sizeof(*block));
    if(block==NULL) {
        return NULL;
    }
    if(size==0) {
        size = 1;
    }
    block->ref_cnt = 1;
    block->bmem= true;
    block->u.bmem.heap = memHeap;
    block->uncached = false;
    if(settings) {
        block->uncached = settings->uncached;
    }
    rc = BMMA_P_ConvertAlignment(alignment, &alignment);
    if(rc!=BERR_SUCCESS) {
        return NULL;
    }
#if BDBG_DEBUG_BUILD
    block->u.bmem.addr = BMEM_P_Heap_TagAllocAligned(memHeap, size, alignment, 0, fname, line);
#else
    block->u.bmem.addr = BMEM_Heap_AllocAligned(memHeap, size, alignment, 0);
#endif
    if(block->u.bmem.addr==NULL) {
        BKNI_Free(block);
        return NULL;
    }
    return block;
}

#if BDBG_DEBUG_BUILD
void BMMA_Free_tagged(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
void BMMA_Free(BMMA_Block_Handle b)
#endif
{
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    BDBG_ASSERT(b->ref_cnt>0);
    b->ref_cnt--;
    if(b->ref_cnt==0) {
        if(b->bmem) {
            BMEM_Heap_Free(b->u.bmem.heap, b->u.bmem.addr);
        }
        BKNI_Free(b);
    }
    return;
}

#if BDBG_DEBUG_BUILD
BMMA_DeviceOffset BMMA_LockOffset_tagged(BMMA_Block_Handle block, const char *fname, unsigned line)
#else
BMMA_DeviceOffset BMMA_LockOffset(BMMA_Block_Handle block)
#endif
{
    uint32_t offset;
    if(block->bmem) {
        BERR_Code rc = BMEM_Heap_ConvertAddressToOffset(block->u.bmem.heap, block->u.bmem.addr, &offset);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);return 0;}
    } else {
        offset = block->u.wrapper.offset;
    }
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    return offset;
}

#if BDBG_DEBUG_BUILD
void BMMA_UnlockOffset_tagged(BMMA_Block_Handle block, BMMA_DeviceOffset offset, const char *fname, unsigned line)
#else
void BMMA_UnlockOffset(BMMA_Block_Handle block, BMMA_DeviceOffset offset)
#endif
{
    BSTD_UNUSED(block);
    BSTD_UNUSED(offset);
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    return;
}

#if BDBG_DEBUG_BUILD
void *BMMA_Lock_tagged(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
void *BMMA_Lock(BMMA_Block_Handle b)
#endif
{
    void *cached;
    if(b->bmem) {
        BERR_Code rc = BMEM_Heap_ConvertAddressToCached(b->u.bmem.heap, b->u.bmem.addr, &cached);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);return NULL;}
    } else {
        cached = b->u.wrapper.addr;
    }
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    return cached;
}


#if BDBG_DEBUG_BUILD
void BMMA_Unlock_tagged(BMMA_Block_Handle block, const void *addr, const char *fname, unsigned line)
#else
void BMMA_Unlock(BMMA_Block_Handle block, const void *addr)
#endif
{
    BSTD_UNUSED(block);
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    BSTD_UNUSED(addr);
    return;
}

void *BMMA_GetUncached(BMMA_Block_Handle b)
{
    void *addr;
    if(b->uncached && b->bmem) {
        addr = b->u.bmem.addr;
    } else {
        addr = NULL;
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return addr;
}

BMMA_DeviceOffset BMMA_GetOffset_isr(BMMA_Block_Handle block)
{
    uint32_t offset;
    BKNI_ASSERT_ISR_CONTEXT();
    if(block->bmem) {
        BERR_Code rc = BMEM_Heap_ConvertAddressToOffset_isr(block->u.bmem.heap, block->u.bmem.addr, &offset);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);return 0;}
    } else {
        offset = block->u.wrapper.offset;
    }
    return offset;
}


void BMMA_FlushCache(BMMA_Block_Handle block, const void *addr, size_t length)
{
    if(block->bmem) {
        BMEM_Heap_FlushCache(block->u.bmem.heap, (void *)addr, length);
    } else {
        (void )BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

void BMMA_MarkDiscarable(BMMA_Block_Handle block)
{
    BSTD_UNUSED(block);
    return;
}

BMMA_Block_Handle BMMA_Block_Create_tagged(BMMA_Handle allocator, BMMA_DeviceOffset offset, size_t size, void *address, const char *fname, unsigned line)
{
    BMMA_Block_Handle block = BKNI_Malloc(sizeof(*block));
    if(block==NULL) {
        return NULL;
    }
    block->ref_cnt = 1;
    block->uncached = false;
    block->bmem = false;
    block->u.wrapper.offset = offset;
    block->u.wrapper.addr = address;
    return block;
    BSTD_UNUSED(allocator);
    BSTD_UNUSED(size);
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
}

void BMMA_Block_Acquire(BMMA_Handle a, BMMA_Block_Handle b)
{
    BSTD_UNUSED(a);
    b->ref_cnt++;
    return;
}

void BMMA_Block_GetProperties(BMMA_Block_Handle b, BMMA_BlockProperties *properties)
{
    BSTD_UNUSED(b);
    BKNI_Memset(properties, 0, sizeof(properties));
    return;
}

BERR_Code BMMA_Heap_AddRegion(BMMA_Heap_Handle h, BMMA_DeviceOffset base, size_t size)
{
    BSTD_UNUSED(h);
    BSTD_UNUSED(base);
    BSTD_UNUSED(size);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BMMA_Heap_GetFreeRegions(BMMA_Heap_Handle h, BMMA_Heap_FreeRegion *regions, unsigned numEntries, unsigned *pNumReturned)
{
    BSTD_UNUSED(h);
    BSTD_UNUSED(regions);
    BSTD_UNUSED(numEntries);
    *pNumReturned = 0;
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

BERR_Code BMMA_Heap_RemoveRegion(BMMA_Heap_Handle h, BMMA_DeviceOffset base, size_t size)
{
    BSTD_UNUSED(h);
    BSTD_UNUSED(base);
    BSTD_UNUSED(size);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
