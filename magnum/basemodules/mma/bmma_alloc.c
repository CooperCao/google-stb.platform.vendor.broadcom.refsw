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
#include "blst_list.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bmma_pool.h"
#include "bmma_range.h"
#include "bmma.h"
#include "bmma_system.h"
#include "blst_slist.h"

BDBG_MODULE(BMMA_Alloc);
BDBG_FILE_MODULE(BMMA_Trace);

BDBG_FILE_MODULE(BMEM);
BDBG_FILE_MODULE(BMEM_ALLOCATED);
BDBG_FILE_MODULE(BMEM_FREE);

#define BMMA_ALLOC_BMEM_SUPPORT 1

#if BMMA_ALLOC_BMEM_SUPPORT
#include "bmma_alloc_bmem.h"
#endif

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

BDBG_OBJECT_ID(BMMA_Allocator);
BDBG_OBJECT_ID(BMMA_Heap);
BDBG_OBJECT_ID(BMMA_Block);

struct BMMA_Heap {
    BDBG_OBJECT(BMMA_Heap)
    BLST_S_ENTRY(BMMA_Heap) link;
    BMMA_Handle parent; /* parent parent */
    BMMA_RangeAllocator_Handle rangeAllocator;
    BMMA_Heap_CreateSettings settings;
#if BMMA_ALLOC_BMEM_SUPPORT
    BMMA_Block_Handle taintedCache;
    BLST_D_HEAD(BMMA_P_Taint_Blocks, BMMA_Block) taintedBlocks;
#endif
};

struct BMMA_Allocator {
    BDBG_OBJECT(BMMA_Allocator)
    BKNI_MutexHandle lock;
    BLST_S_HEAD(BMMA_P_HeapList, BMMA_Heap) heaps;
    BMMA_CreateSettings settings;
    unsigned sequence;  /* global sequence counter */
    struct BMMA_Heap dummyHeap; /* used for the 'wrapper' allocations */
};

#define BMMA_P_LOCK_OFFSET_LIMIT  32760
struct BMMA_Block {
    BDBG_OBJECT(BMMA_Block)
    BMMA_Heap_Handle heap;
    int16_t lock_cnt;
    int16_t lock_offset_cnt;
    bool discardable;
    bool uncached;
    uint16_t ref_cnt; /* set to 1 by Alloc, increased by Acquire and decreased by Free, when goes to 0 memory freed */
    void *addr; /* valid if lock_cnt >=0 */
    BMMA_RangeAllocator_Block_Handle block; /* allocated block from range allocator */
#if BDBG_DEBUG_BUILD
    const char *fname;
    unsigned line;
#endif
#if BMMA_ALLOC_BMEM_SUPPORT
    bool tainted;
    BLST_D_ENTRY(BMMA_Block) linkTainted;
#endif
};

static BERR_Code BMMA_Heap_Compact_locked(BMMA_Heap_Handle h, BMMA_RangeAllocator_CompactionStatus *compactionStatus);
static void BMMA_Dbg_DumpHeap_locked(BMMA_Heap_Handle h);

void BMMA_GetDefaultCreateSettings(BMMA_CreateSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    return;
}

BERR_Code BMMA_Create(BMMA_Handle *mma, const BMMA_CreateSettings *settings)
{
    BMMA_Handle a;
    BERR_Code rc;

    BDBG_ASSERT(settings);
    BDBG_ASSERT(mma);
    a = BKNI_Malloc(sizeof(*a));
    if(a==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_malloc;}
    BDBG_OBJECT_INIT(a, BMMA_Allocator);
    a->settings = *settings;
    BLST_S_INIT(&a->heaps);
    rc = BKNI_CreateMutex(&a->lock);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_lock;}
    BKNI_Memset(&a->dummyHeap, 0, sizeof(a->dummyHeap));
    BDBG_OBJECT_SET(&a->dummyHeap, BMMA_Heap);
    BMMA_Heap_GetDefaultCreateSettings(&a->dummyHeap.settings);
    a->dummyHeap.parent = a;
#if BMMA_ALLOC_BMEM_SUPPORT
    a->dummyHeap.taintedCache = NULL;
    BLST_D_INIT(&a->dummyHeap.taintedBlocks);
#endif

    *mma= a;

    return BERR_SUCCESS;

err_lock:
    BKNI_Free(a);
err_malloc:
    return rc;
}

static void BMMA_P_Allocator_Destroy_Locked(BMMA_Handle a, BMMA_Heap_Handle h)
{
    BMMA_RangeAllocator_Status status;
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_OBJECT_ASSERT(a, BMMA_Allocator);
    BDBG_ASSERT(h->parent == a);
    BMMA_RangeAllocator_GetStatus(h->rangeAllocator, &status);
    if(status.allocatedBlocks) {
        BDBG_MODULE_ERR(BMEM, ("Leaked resource -- unfreed blocks detected"));
        BMMA_Dbg_DumpHeap_locked(h);
    }
    BLST_S_REMOVE(&a->heaps, h, BMMA_Heap, link);
    BMMA_RangeAllocator_Destroy(h->rangeAllocator);
    BDBG_OBJECT_DESTROY(h, BMMA_Heap);
    BKNI_Free(h);
    return;
}


void BMMA_Destroy(BMMA_Handle a)
{
    BMMA_Heap_Handle h;
    BDBG_OBJECT_ASSERT(a, BMMA_Allocator);
    BKNI_AcquireMutex(a->lock);
    while(NULL!=(h=BLST_S_FIRST(&a->heaps))) {
        BMMA_P_Allocator_Destroy_Locked(a, h);
    }
    BKNI_ReleaseMutex(a->lock);
    BKNI_DestroyMutex(a->lock);
    BDBG_OBJECT_DESTROY(a, BMMA_Allocator);
    BKNI_Free(a);
    return;
}

void BMMA_Heap_GetDefaultCreateSettings(BMMA_Heap_CreateSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->base_uncached = NULL;
    settings->flags.silent = false;
    return;
}

static void BMMA_P_Lock(BMMA_Heap_Handle h)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_OBJECT_ASSERT(h->parent, BMMA_Allocator);
    rc = BKNI_AcquireMutex(h->parent->lock);
    BDBG_ASSERT(rc==BERR_SUCCESS);
    return;
}

static void BMMA_P_Unlock(BMMA_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_OBJECT_ASSERT(h->parent, BMMA_Allocator);
    BKNI_ReleaseMutex(h->parent->lock);
    return;
}

static bool BMMA_P_Relocatable(void *context, void *user_block)
{
    BMMA_Heap_Handle h = context;
    BMMA_Block_Handle b = user_block;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    /* it should be already serialized */
    return !(b->lock_cnt > 0 || b->lock_offset_cnt > 0);
}

static void BMMA_P_Unmap(BMMA_Heap_Handle h, BMMA_Block_Handle b)
{
    BDBG_ASSERT(b->lock_cnt <= 0);
    if(b->lock_cnt == 0) {
        size_t size = BMMA_RangeAllocator_GetAllocationSize(b->block);
        void *addr = b->addr;
        b->lock_cnt = -1;
        b->addr = NULL;
        h->settings.unmap(h->settings.context, (uint8_t *)b+sizeof(*b), addr, size);
    }
    return;
}

static BERR_Code BMMA_P_Relocate(void *context, void *user_block, BMMA_DeviceOffset dst, BMMA_DeviceOffset src, size_t length)
{
    BMMA_Heap_Handle h = context;
    BMMA_Block_Handle b = user_block;
    uint8_t *addr;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_OBJECT_ASSERT(b, BMMA_Block);

    /* it should be already serialized */
    BDBG_ASSERT(b->lock_offset_cnt==0);
    BMMA_P_Unmap(h, b);
    if(b->discardable) { goto done;}
    if(h->settings.relocate) {
        return h->settings.relocate(h->settings.context, dst, src, length);
    }
    if(src>dst && dst + length < src) { /* overlapped, mmap dst .. src + length */
        addr = h->settings.mmap(h->settings.context, NULL, dst, length + (src-dst));
        if(!addr) { return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
        BDBG_MSG(("BMMA_P_Relocate:%p copy " BDBG_UINT64_FMT "(%p)->" BDBG_UINT64_FMT "(%p) %u", (void *)h, BDBG_UINT64_ARG(src), addr+(src-dst), BDBG_UINT64_ARG(dst), addr, (unsigned)length));
        BKNI_Memmove(addr, addr+(src-dst), length);
    } else if(dst>src && src + length < dst) { /* overlapped, mmap  src .. dst + length */
        addr = h->settings.mmap(h->settings.context, NULL, src, length + (dst-src));
        if(!addr) { return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
        BKNI_Memmove(addr+(dst-src), addr, length);
        BDBG_MSG(("BMMA_P_Relocate:%p copy " BDBG_UINT64_FMT "(%p)->" BDBG_UINT64_FMT "(%p) %u", (void *)h, BDBG_UINT64_ARG(src), addr, BDBG_UINT64_ARG(dst), addr+(dst-src), (unsigned)length));
     } else {
        uint8_t *dst_addr = h->settings.mmap(h->settings.context, NULL, dst, length);
        if(!dst_addr) {return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);}
        addr = h->settings.mmap(h->settings.context, NULL, src, length);
        BDBG_MSG(("BMMA_P_Relocate:%p copy " BDBG_UINT64_FMT "(%p)->" BDBG_UINT64_FMT "(%p) %u", (void *)h, BDBG_UINT64_ARG(src), addr, BDBG_UINT64_ARG(dst), dst_addr, (unsigned)length));
        if(addr) {
            BKNI_Memcpy(dst_addr, addr, length);
        }
        h->settings.unmap(h->settings.context, NULL, dst_addr, length);
        if(!addr) {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    h->settings.unmap(h->settings.context, NULL, addr, length);


done:
    if(h->settings.free) {
        h->settings.free(h->settings.context, src, length);
    }
    if(h->settings.alloc) {
        h->settings.alloc(h->settings.context, dst, length,
#if BDBG_DEBUG_BUILD
                                  b->fname, b->line
#else
                                  NULL, 0
#endif
                                  );
    }
    return BERR_SUCCESS;
}

static bool BMMA_P_Advance(void *context, const BMMA_RangeAllocator_CompactionStatus *status)
{
    BMMA_Heap_Handle h = context;
    BMMA_Heap_CompactionStatus  compactionStatus;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    if(!h->settings.compaction_advance) {
        return true;
    }
    compactionStatus.freeBlocks = status->freeBlocks;
    compactionStatus.largestFreeBlock = status->largestFreeBlock;
    compactionStatus.bytesCopied = status->bytesCopied;
    return h->settings.compaction_advance(h->settings.context, &compactionStatus);
}

BERR_Code BMMA_Heap_Create(BMMA_Heap_Handle *heap, BMMA_Handle parent, const BMMA_Heap_CreateSettings *settings)
{
    BMMA_Heap_Handle  h;
    BERR_Code rc;
    BMMA_RangeAllocator_CreateSettings  rangeAllocatorSettings;

    BDBG_ASSERT(heap);
    BDBG_ASSERT(settings);
    BDBG_OBJECT_ASSERT(parent, BMMA_Allocator);
    if(settings->size==0) {rc=BERR_TRACE(BERR_INVALID_PARAMETER);goto err_settings;}
    if(settings->mmap == NULL || settings->unmap == NULL) {rc=BERR_TRACE(BERR_INVALID_PARAMETER);goto err_settings;}
    if(settings->minAlignment==0) {BERR_TRACE(BERR_INVALID_PARAMETER);}
    h = BKNI_Malloc(sizeof(*h));
    if(h==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(h, BMMA_Heap);
#if BMMA_ALLOC_BMEM_SUPPORT
    h->taintedCache = NULL;
    BLST_D_INIT(&h->taintedBlocks);
#endif
    h->settings = *settings;
    if(settings->minAlignment==0) {
        h->settings.minAlignment = sizeof(void*);
    }
    h->parent = parent;
    BMMA_RangeAllocator_GetDefaultCreateSettings(&rangeAllocatorSettings);
    rangeAllocatorSettings.size = h->settings.size;
    rangeAllocatorSettings.base = h->settings.base;
    rangeAllocatorSettings.allocationHeader = sizeof(struct BMMA_Block) + settings->mmapStateSize;
    rangeAllocatorSettings.printLeakedRanges = false; /* leaked blocks printed internally */
    rangeAllocatorSettings.verbose = false; /* don't print list of allocation if out of memory */
    rangeAllocatorSettings.minAlignment = h->settings.minAlignment;
    rangeAllocatorSettings.context = h;
    rangeAllocatorSettings.relocatable = BMMA_P_Relocatable;
    rangeAllocatorSettings.relocate = BMMA_P_Relocate;
    rangeAllocatorSettings.advance = BMMA_P_Advance;
    rangeAllocatorSettings.silent = h->settings.flags.silent;
    rc = BMMA_RangeAllocator_Create(&h->rangeAllocator, &rangeAllocatorSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_range_allocator;}

    BMMA_P_Lock(h);
    BLST_S_INSERT_HEAD(&parent->heaps, h, link);
    BMMA_P_Unlock(h);
    *heap = h;
    return BERR_SUCCESS;

err_range_allocator:
    BKNI_Free(h);
err_alloc:
err_settings:
    return rc;
}

void BMMA_Heap_Destroy(BMMA_Heap_Handle h)
{
    BMMA_Handle parent;
    BMMA_P_Lock(h);
    parent = h->parent;
    BMMA_P_Allocator_Destroy_Locked(parent, h);
    BKNI_ReleaseMutex(parent->lock);
    return;
}

void BMMA_GetDefaultAllocationSettings(BMMA_AllocationSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->uncached = false;
    pSettings->boundary = 0;
    return;
}


#if BDBG_DEBUG_BUILD
BMMA_Block_Handle BMMA_Alloc_tagged(BMMA_Heap_Handle h, size_t size, unsigned alignment, const BMMA_AllocationSettings *pSettings, const char *fname, unsigned line)
#else
BMMA_Block_Handle BMMA_Alloc(BMMA_Heap_Handle h, size_t size, unsigned alignment, const BMMA_AllocationSettings *pSettings)
#endif
{
    BMMA_RangeAllocator_BlockSettings blockSettings;
    BMMA_RangeAllocator_Block_Handle  block;
    BMMA_Block_Handle b=NULL;
    BERR_Code rc;
    unsigned itteration;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);

    BDBG_MSG_TRACE(("Alloc:>%p %u(%u) %s:%u", (void *)h, size, alignment, fname, line));

    BMMA_RangeAllocator_GetDefaultAllocationSettings(&blockSettings);
    blockSettings.alignment = h->settings.minAlignment;
    if(alignment > blockSettings.alignment) {
        blockSettings.alignment = alignment;
    }
    if(pSettings) {
        if(pSettings->uncached) {
            if(h->settings.base_uncached == NULL) {
                rc = BERR_TRACE(BERR_NOT_SUPPORTED);return NULL;
            }
        }
        blockSettings.boundary = pSettings->boundary;
    }
    BMMA_P_Lock(h);
    for(itteration=0;;) {
#if BDBG_DEBUG_BUILD
        blockSettings.fname = fname;
        blockSettings.line  = line;
#endif
        rc = BMMA_RangeAllocator_Alloc(h->rangeAllocator, &block, size, &blockSettings);
        if(rc!=BERR_SUCCESS) {
            if(!h->settings.flags.silent) {
                BDBG_ERR(("Alloc Failed:>%p %u(%u) %s:%u", (void *)h, (unsigned)size, alignment, fname, line));
            }
            if(h->settings.out_of_memory) {
                bool try_again;
                try_again = h->settings.out_of_memory(h->settings.context, size, pSettings, itteration);
                if(try_again) {
                    BMMA_RangeAllocator_CompactionStatus compactionStatus;
                    itteration++;
                    rc = BMMA_Heap_Compact_locked(h, &compactionStatus);
                    if(rc==BERR_SUCCESS) {continue;}
                    rc = BERR_TRACE(rc);
                    break;
                }
            }
            rc = h->settings.flags.silent ? rc : BERR_TRACE(rc);
            break;
        }
        b = BMMA_RangeAllocator_GetAllocationHeader(block);
        BDBG_ASSERT(b);
        BKNI_Memset(b, 0, sizeof(*b) + h->settings.mmapStateSize);
        BDBG_OBJECT_INIT(b, BMMA_Block);
        b->heap = h;
        b->lock_cnt = -1;
        b->lock_offset_cnt = 0;
        b->discardable = true;
        b->uncached = false;
        b->ref_cnt = 1;
        if(pSettings) {
            b->uncached = pSettings->uncached;
        }
        b->addr = NULL;
        b->block = block;
#if BDBG_DEBUG_BUILD
        b->fname = fname;
        b->line = line;
#endif
#if BMMA_ALLOC_BMEM_SUPPORT
        b->tainted = false;
#endif
#if BDBG_DEBUG_BUILD
        if(b) {
            BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
            BDBG_MODULE_MSG(BMEM, ("%p: %p:" BDBG_UINT64_FMT "..." BDBG_UINT64_FMT "(%#x),%u %s:%u", (void *)h, (void *)b, BDBG_UINT64_ARG(offset), BDBG_UINT64_ARG(offset+size), (unsigned)alignment, (unsigned)size, b->fname, b->line));
        }
#endif
        if(h->settings.alloc) {
            BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
            h->settings.alloc(h->settings.context, offset, size,
#if BDBG_DEBUG_BUILD
                                 b->fname, b->line
#else
                                 NULL, 0
#endif
                                      );
        }
        break;
    }

    BMMA_P_Unlock(h);
    BDBG_MSG(("Alloc:%p size=%u align=%u handle=%p %s:%u", (void *)h, (unsigned)size, alignment, (void *)b, fname, line));
    return b;
}

#if BDBG_DEBUG_BUILD
void BMMA_Free_tagged(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
void BMMA_Free(BMMA_Block_Handle b)
#endif
{
    BMMA_RangeAllocator_Block_Handle block;
    BMMA_Heap_Handle h;
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
#if BDBG_DEBUG_BUILD
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
#endif
    BDBG_MSG_TRACE(("Free:>%p %s:%u", (void *)b, fname, line));
    BMMA_P_Lock(b->heap);
    BDBG_ASSERT(b->ref_cnt>0);
    h = b->heap;
    b->ref_cnt--;
    if(b->ref_cnt==0) {
        block = b->block;
#if BMMA_ALLOC_BMEM_SUPPORT
        if(b->tainted) {
            if(h->taintedCache==b) {
                h->taintedCache = BLST_D_PREV(b, linkTainted);
            }
            BLST_D_REMOVE(&h->taintedBlocks, b, linkTainted);
        }
#endif
        if(h != &h->parent->dummyHeap) {
            size_t size = BMMA_RangeAllocator_GetAllocationSize(b->block);
            if(b->lock_cnt>0) {
                if(!b->uncached) {
                    b->heap->settings.unmap(h->settings.context, (uint8_t *)b + sizeof(*b), b->addr, size);
                }
            }
            if(h->settings.free) {
                BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
                h->settings.free(h->settings.context, offset, size);
            }
        }

        BDBG_OBJECT_DESTROY(b, BMMA_Block);
        BDBG_MSG(("Free:handle=%p %s:%u", (void *)b, fname, line));
        if(h != &h->parent->dummyHeap) {
            BMMA_RangeAllocator_Free(h->rangeAllocator, block);
        } else {
            BMMA_RangeAllocator_DestroyBlock(block);
            BKNI_Free(b);
        }
    }
    BMMA_P_Unlock(h);
    return;
}

#if BDBG_DEBUG_BUILD
static void BMMA_LockOffset_tagged_locked(BMMA_Block_Handle b, const char *fname, unsigned line)
{
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
    BDBG_MSG_TRACE(("BMMA_LockOffset:%p %u %s:%u", b, b->lock_offset_cnt, fname, line));
#else
#define BMMA_LockOffset_tagged_locked(b, fname, line) BMMA_LockOffset_locked(b)
static void BMMA_LockOffset_locked(BMMA_Block_Handle b)
{
#endif
    b->discardable = false;
    if(b->lock_offset_cnt>=0 && b->lock_offset_cnt<BMMA_P_LOCK_OFFSET_LIMIT) {
        b->lock_offset_cnt++;
    } else {
        if(b->lock_offset_cnt != BMMA_P_LOCK_OFFSET_LIMIT) {
            BDBG_ERR(("BMMA_LockOffset: %p: allocated at %s:%u has bad lock_offset_cnt:%d ->%s:%u", (void *)b,b->fname,b->line,b->lock_offset_cnt,fname?fname:"",line));
            b->lock_offset_cnt = BMMA_P_LOCK_OFFSET_LIMIT;
        }
    }
    return ;
}

#if BDBG_DEBUG_BUILD
static void BMMA_UnlockOffset_tagged_locked(BMMA_Block_Handle b, const char *fname, unsigned line)
{
    BSTD_UNUSED(fname);
    BSTD_UNUSED(line);
    BDBG_MSG_TRACE(("BMMA_UnlockOffset:%p %u %s:%u", b, b->lock_offset_cnt, fname, line));
#else
#define BMMA_UnlockOffset_tagged_locked(b,file,line) BMMA_UnlockOffset_locked(b)
static void BMMA_UnlockOffset_locked(BMMA_Block_Handle b)
{
#endif
    if(b->lock_offset_cnt>0 && b->lock_offset_cnt<BMMA_P_LOCK_OFFSET_LIMIT) {
        b->lock_offset_cnt--;
    } else {
        BDBG_ERR(("BMMA_UnlockOffset: %p: allocated at %s:%u has bad lock_offset_cnt:%d ->%s:%u", (void *)b,b->fname,b->line,b->lock_offset_cnt,fname?fname:"",line));
    }
    return;
}

#if BDBG_DEBUG_BUILD
static void *BMMA_Lock_tagged_locked(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
#define BMMA_Lock_tagged_locked(b,fname,line) BMMA_Lock_locked(b)
static void *BMMA_Lock_locked(BMMA_Block_Handle b)
#endif
{
    void *addr=NULL;

    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    if(b->lock_cnt>=0) {
        if(b->lock_cnt==0) {
            BMMA_LockOffset_tagged_locked(b, fname, line);
        }
        BDBG_ASSERT(b->lock_cnt<32767);
        b->lock_cnt++;
        addr = b->addr;
        if(addr==NULL && b->heap == &b->heap->parent->dummyHeap) {
            (void)BERR_TRACE(BERR_OS_ERROR);
        }
    } else {
        BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
        size_t size = BMMA_RangeAllocator_GetAllocationSize(b->block);
        BMMA_LockOffset_tagged_locked(b, fname, line);
        addr = b->heap->settings.mmap(b->heap->settings.context, (uint8_t *)b + sizeof(*b), offset, size);
        if(addr==NULL) {
            (void)BERR_TRACE(BERR_OS_ERROR);
            BMMA_UnlockOffset_tagged_locked(b, fname, line);
            goto done;
        }
        b->addr = addr;
        b->lock_cnt=1;
    }
done:
    b->discardable = false;
    return addr;
}

#if BDBG_DEBUG_BUILD
void BMMA_Unlock_tagged_locked(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
#define BMMA_Unlock_tagged_locked(b,fname,line) BMMA_Unlock_locked(b)
void BMMA_Unlock_locked(BMMA_Block_Handle b)
#endif
{
    BDBG_ASSERT(b->lock_cnt>0);
    b->lock_cnt--;
    if(b->lock_cnt==0) {
        BDBG_ASSERT(b->heap!=&b->heap->parent->dummyHeap); /* must be real allocation, not an wrapper */
        if(!b->uncached) {
            size_t size = BMMA_RangeAllocator_GetAllocationSize(b->block);
            b->heap->settings.unmap(b->heap->settings.context, (uint8_t *)b+sizeof(*b), b->addr, size);
        }
        BMMA_UnlockOffset_tagged_locked(b, fname, line);
    }
    return;
}

#if BDBG_DEBUG_BUILD
void *BMMA_Lock_tagged(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
void *BMMA_Lock(BMMA_Block_Handle b)
#endif
{
    void *addr;

    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BMMA_P_Lock(b->heap);
    addr = BMMA_Lock_tagged_locked(b, fname, line);
    BMMA_P_Unlock(b->heap);

    BDBG_MODULE_MSG(BMMA_Trace, ("BMMA_Lock:%p(%u:%s:%u) %p %s:%u", (void *)b, b->lock_cnt, b->fname, b->line, addr, fname, line));

    return addr;
}

#if BDBG_DEBUG_BUILD
void BMMA_Unlock_tagged(BMMA_Block_Handle b, const void *addr, const char *fname, unsigned line)
#else
void BMMA_Unlock(BMMA_Block_Handle b, const void *addr)
#endif
{
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BDBG_MODULE_MSG(BMMA_Trace, ("BMMA_Unlock:%p(%u:%s:%u) %p %s:%u", (void *)b, b->lock_cnt, b->fname, b->line, addr, fname, line));
    BMMA_P_Lock(b->heap);
    if(addr == NULL || b->addr != addr) {
        BDBG_ERR(("BMMA_Unlock: %p: allocated at %s:%u attempt to use wrong pointer %p(%p) at %s:%u, ", (void *)b,b->fname,b->line,addr,b->addr,fname?fname:"",line));
        BDBG_ASSERT(0);
    }
    BMMA_Unlock_tagged_locked(b, fname, line);
    BMMA_P_Unlock(b->heap);
    return;
}

void *BMMA_GetUncached(BMMA_Block_Handle b)
{
    void *addr;
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BMMA_P_Lock(b->heap);
    BDBG_ASSERT(b->lock_cnt > 0);
    if(b->uncached) {
        BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
        addr = (uint8_t *)b->heap->settings.base_uncached + (offset - b->heap->settings.base);
    } else {
        addr = NULL;
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BMMA_P_Unlock(b->heap);
    return addr;
}

#if BDBG_DEBUG_BUILD
BMMA_DeviceOffset BMMA_LockOffset_tagged(BMMA_Block_Handle b, const char *fname, unsigned line)
#else
BMMA_DeviceOffset BMMA_LockOffset(BMMA_Block_Handle b)
#endif
{
    BMMA_DeviceOffset offset;
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BMMA_P_Lock(b->heap);
    BMMA_LockOffset_tagged_locked(b, fname, line);
    offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
    BMMA_P_Unlock(b->heap);
    BDBG_MODULE_MSG(BMMA_Trace, ("BMMA_LockOffset:%p(%u:%s:%u) " BDBG_UINT64_FMT " %s:%u", (void *)b, b->lock_offset_cnt, b->fname, b->line, BDBG_UINT64_ARG(offset), fname, line));
    return offset;
}


#if BDBG_DEBUG_BUILD
void BMMA_UnlockOffset_tagged(BMMA_Block_Handle b, BMMA_DeviceOffset offset, const char *fname, unsigned line)
#else
void BMMA_UnlockOffset(BMMA_Block_Handle b, BMMA_DeviceOffset offset)
#endif
{
    BMMA_DeviceOffset block_offset;
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BMMA_P_Lock(b->heap);
    block_offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
    BDBG_MODULE_MSG(BMMA_Trace, ("BMMA_UnlockOffset:%p(%u:%s:%u) " BDBG_UINT64_FMT " %s:%u", (void *)b, b->lock_offset_cnt, b->fname, b->line, BDBG_UINT64_ARG(block_offset), fname, line));
    if(block_offset != offset) {
        BDBG_ERR(("BMMA_UnlockOffset: %p: allocated at %s:%u attempt to use wrong offset " BDBG_UINT64_FMT "(" BDBG_UINT64_FMT ") at %s:%u, ", (void *)b,b->fname,b->line, BDBG_UINT64_ARG(offset),BDBG_UINT64_ARG(block_offset),fname?fname:"",line));
        BDBG_ASSERT(0);
    }
    BMMA_UnlockOffset_tagged_locked(b, fname, line);
    BMMA_P_Unlock(b->heap);
    return;
}

void BMMA_MarkDiscarable(BMMA_Block_Handle b)
{
    BMMA_P_Lock(b->heap);
    b->discardable = true;
    BMMA_P_Unlock(b->heap);
    return;
}

static BERR_Code BMMA_Heap_Compact_locked(BMMA_Heap_Handle h, BMMA_RangeAllocator_CompactionStatus *compactionStatus)
{
    return BMMA_RangeAllocator_Compact(h->rangeAllocator, compactionStatus);
}

BERR_Code BMMA_Heap_Compact(BMMA_Heap_Handle h, BMMA_Heap_CompactionStatus *status)
{
    BMMA_RangeAllocator_CompactionStatus compactionStatus;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BDBG_ASSERT(status);

    BMMA_P_Lock(h);
    rc = BMMA_Heap_Compact_locked(h, &compactionStatus);
    BMMA_P_Unlock(h);
    status->freeBlocks = compactionStatus.freeBlocks;
    status->largestFreeBlock = compactionStatus.largestFreeBlock;
    status->bytesCopied = compactionStatus.bytesCopied;

    return BERR_TRACE(rc);
}

BMMA_DeviceOffset BMMA_GetOffset_isr(BMMA_Block_Handle b)
{
    /* can't lock in this function */
    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    if(b->lock_offset_cnt>0 && !b->discardable) {
        BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase_isrsafe(b->block);
        return offset;
    }
    BDBG_ERR(("BMMA_GetOffset_isr: %p: allocated at %s:%u is not accessible lock_offset_cnt:%d discardable:%u", (void *)b,b->fname,b->line,b->lock_offset_cnt,(unsigned)b->discardable));
    BDBG_ASSERT(0);
    return 0;
}

void BMMA_FlushCache_isrsafe(BMMA_Block_Handle b, const void *addr, size_t size)
{
    unsigned offset;
    size_t block_size = BMMA_RangeAllocator_GetAllocationSize(b->block);
    /* can't lock in this function */
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BDBG_ASSERT(b->lock_cnt>0);
    if((const uint8_t *)addr < (const uint8_t *)b->addr) {
        BDBG_ERR(("BMMA_FlushCache:%p(%p[%u[) addr:%p not from this memory block", (void *)b, b->addr, (unsigned)block_size, addr));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    offset = (const uint8_t *)addr - (const uint8_t *)b->addr;
    if((offset+size)>block_size) {
        BDBG_ERR(("BMMA_FlushCache:%p(%p[%u[) offset:%u points outside of this memory block", (void *)b, b->addr, (unsigned)block_size, (unsigned)(offset+size)));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    b->heap->parent->settings.flush_cache((uint8_t *)b->addr+offset, size);
    return;
}

static bool BMMA_Heap_P_GetStatus_Iterator(void *context, void *header, const BMMA_RangeAllocator_Region *region, bool boundary)
{
    BMMA_Heap_SlowStatus *slowStatus = context;
    BSTD_UNUSED(boundary);
    if(header==NULL) { /* free */
        if(slowStatus->smallestFree==0 || region->length < slowStatus->smallestFree) {
            slowStatus->smallestFree = region->length;
        }
    } else {
        if(slowStatus->smallestAllocated==0 || region->length < slowStatus->smallestAllocated) {
            slowStatus->smallestAllocated = region->length;
        }
        if(slowStatus->largestAllocated ==0 || region->length > slowStatus->largestAllocated) {
            slowStatus->largestAllocated = region->length;
        }
    }
    return true;
}


BERR_Code BMMA_Heap_GetStatus(BMMA_Heap_Handle h, BMMA_Heap_FastStatus *fastStatus, BMMA_Heap_SlowStatus *slowStatus)
{
    BMMA_RangeAllocator_Status status;
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BMMA_P_Lock(h);
    BMMA_RangeAllocator_GetStatus(h->rangeAllocator, &status);
    fastStatus->totalAllocated = status.allocatedSpace;
    fastStatus->numAllocated = status.allocatedBlocks;
    fastStatus->highWatermark = status.highWatermark;
    fastStatus->numFree = status.freeBlocks;
    fastStatus->totalFree = status.freeSpace;
    fastStatus->largestFree = status.largestFree;
    fastStatus->size = status.allocatedSpace + status.freeSpace;
    fastStatus->alignment = h->settings.minAlignment;
    if(slowStatus) {
        slowStatus->largestAllocated = 0;
        slowStatus->smallestAllocated = 0;
        slowStatus->smallestFree = 0;
        BMMA_RangeAllocator_Iterate(h->rangeAllocator, BMMA_Heap_P_GetStatus_Iterator, slowStatus, NULL);
    }
    BMMA_P_Unlock(h);
    return BERR_SUCCESS;
}

static bool BMMA_Heap_P_DumpHeap_IteratorAllocated(void *context, void *header, const BMMA_RangeAllocator_Region *region, bool boundary)
{
    BMMA_Heap_Handle h = context;
    BMMA_Block_Handle b = header;
    BSTD_UNUSED(region);
    BSTD_UNUSED(h);
    BSTD_UNUSED(boundary);
#if BDBG_DEBUG_BUILD
    if(b) {
        BMMA_DeviceOffset offset = BMMA_RangeAllocator_GetAllocationBase(b->block);
        size_t size = BMMA_RangeAllocator_GetAllocationSize(b->block);
        BDBG_MODULE_LOG(BMEM_ALLOCATED, ("" BDBG_UINT64_FMT ",%-8u,%s:%u,%u%s", BDBG_UINT64_ARG(offset),(unsigned)size, b->fname, b->line, (unsigned)(b->lock_offset_cnt),b->discardable?",discardable":"" ));
        BDBG_MODULE_MSG(BMEM_ALLOCATED, ("" BDBG_UINT64_FMT ",%-8u,%s:%u,%u%s", BDBG_UINT64_ARG(region->base),(unsigned)region->length, b->fname, b->line, (unsigned)(b->lock_offset_cnt),b->discardable?".discardable":"" ));
    }
#endif
    return true;
}

static bool BMMA_Heap_P_DumpHeap_IteratorFree(void *context, void *header, const BMMA_RangeAllocator_Region *region, bool boundary)
{
    BMMA_Heap_Handle h = context;
    BMMA_Block_Handle b = header;
    BSTD_UNUSED(h);
    BSTD_UNUSED(boundary);
    if(!b) {
        if(region->length>256) {
            BDBG_MODULE_LOG(BMEM_FREE, ("" BDBG_UINT64_FMT ",%-8u", BDBG_UINT64_ARG(region->base),(unsigned)region->length));
        } else {
            BDBG_MODULE_MSG(BMEM_FREE, ("" BDBG_UINT64_FMT ",%-8u", BDBG_UINT64_ARG(region->base),(unsigned)region->length));
        }
    }
    return true;
}

static void BMMA_Dbg_DumpHeap_locked(BMMA_Heap_Handle h)
{
    BDBG_MODULE_LOG(BMEM_ALLOCATED, ("heap:%p(" BDBG_UINT64_FMT "):", (void *)h, BDBG_UINT64_ARG(h->settings.base)));
    BDBG_MODULE_LOG(BMEM_ALLOCATED, ("offset,totalsize,filename,line,lock"));
    BMMA_RangeAllocator_Iterate(h->rangeAllocator, BMMA_Heap_P_DumpHeap_IteratorAllocated, h, NULL);
    BDBG_MODULE_LOG(BMEM_FREE, ("heap:%p(" BDBG_UINT64_FMT "):", (void *)h, BDBG_UINT64_ARG(h->settings.base)));
    BDBG_MODULE_LOG(BMEM_FREE, ("offset,totalsize"));
    BMMA_RangeAllocator_Iterate(h->rangeAllocator, BMMA_Heap_P_DumpHeap_IteratorFree, h, NULL);

    return;
}


void BMMA_Dbg_DumpHeap(BMMA_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BMMA_P_Lock(h);
    BMMA_Dbg_DumpHeap_locked(h);
    BMMA_P_Unlock(h);
    return;
}

void BMMA_Dbg_ValidateHeap(BMMA_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BMMA_P_Lock(h);
    BMMA_RangeAllocator_Verify(h->rangeAllocator, false);
    BMMA_P_Unlock(h);
    return;
}

void BMMA_Heap_ResetHighWatermark(BMMA_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BMMA_RangeAllocator_ResetHighWatermark(h->rangeAllocator);
    return;
}

void BMMA_Block_GetProperties(BMMA_Block_Handle b, BMMA_BlockProperties *properties)
{
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    properties->heap = b->heap;
    properties->size = BMMA_RangeAllocator_GetAllocationSize(b->block);
    return;
}

BERR_Code BMMA_Heap_AddRegion(BMMA_Heap_Handle h, BMMA_DeviceOffset base, size_t size)
{
    BERR_Code rc;
    BMMA_RangeAllocator_Region region;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    region.base = base;
    region.length = size;
    BMMA_P_Lock(h);
    rc = BMMA_RangeAllocator_AddRegion(h->rangeAllocator, &region);
    BMMA_P_Unlock(h);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    return BERR_SUCCESS;
}

BERR_Code BMMA_Heap_RemoveRegion(BMMA_Heap_Handle h, BMMA_DeviceOffset base, size_t size)
{
    BERR_Code rc;
    BMMA_RangeAllocator_Region region;

    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    region.base = base;
    region.length = size;
    BMMA_P_Lock(h);
    rc = BMMA_RangeAllocator_RemoveRegion(h->rangeAllocator, &region);
    BMMA_P_Unlock(h);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    return BERR_SUCCESS;
}

struct BMMA_Heap_P_GetFreeRegions_State {
    struct BMMA_Heap_FreeRegion *regions;
    unsigned numEntries;
    unsigned count;
};

static bool BMMA_Heap_P_GetFreeRegions_Iterator(void *context, void *header, const BMMA_RangeAllocator_Region *region, bool boundary)
{
    struct BMMA_Heap_P_GetFreeRegions_State *state = context;
    BSTD_UNUSED(header);
    if(state->count >= state->numEntries) {
        return false;
    }
    state->regions[state->count].base = region->base;
    state->regions[state->count].length = region->length;
    state->regions[state->count].boundary = boundary;
    state->count ++;
    return true;
}


void BMMA_Heap_GetFreeRegions(BMMA_Heap_Handle h, BMMA_Heap_FreeRegion *regions, unsigned numEntries, unsigned *pNumReturned)
{
    struct BMMA_Heap_P_GetFreeRegions_State state;
    BMMA_RangeAllocator_IteratorConfig config;

    BDBG_ASSERT(regions);
    BDBG_ASSERT(pNumReturned);
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    config.freeOnly = true;
    config.order = BMMA_RangeAllocator_IteratorOrder_eLargestFirst;
    state.regions = regions;
    state.numEntries = numEntries;
    state.count = 0;
    BMMA_P_Lock(h);
    BMMA_RangeAllocator_Iterate(h->rangeAllocator, BMMA_Heap_P_GetFreeRegions_Iterator, &state, &config);
    BMMA_P_Unlock(h);
    *pNumReturned = state.count;
    return;
}

#if BMMA_ALLOC_BMEM_SUPPORT
void BMMA_Alloc_SetTaint(BMMA_Block_Handle b)
{
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BDBG_ASSERT(!b->tainted);
    BMMA_P_Lock(b->heap);
    b->tainted = true;
    BLST_D_INSERT_HEAD(&b->heap->taintedBlocks, b, linkTainted);
    BMMA_P_Unlock(b->heap);
    return;
}

static BMMA_Block_Handle BMMA_P_FindTaintedBlock(BMMA_Heap_Handle h, void *addr)
{
    BMMA_Block_Handle  cache,b;

    cache=h->taintedCache;
    if(cache) {
        for(b=cache;b;b=BLST_D_PREV(b, linkTainted)) { /* scan backward to most recent added nodes */
            if(addr == b->addr) {
                BDBG_ASSERT(b->lock_cnt>0);
                BDBG_ASSERT(b->tainted);
                goto done;
            }
        }
        b=BLST_D_NEXT(cache, linkTainted);
    } else {
        b=BLST_D_FIRST(&h->taintedBlocks);
    }
    for(;b;b=BLST_D_NEXT(b, linkTainted)) { /* scan backward */
        if(addr == b->addr) {
            BDBG_ASSERT(b->lock_cnt>0);
            BDBG_ASSERT(b->tainted);
            goto done;
        }
    }
    return NULL;

done:
    h->taintedCache = b;
    return b;
}


BMMA_Block_Handle BMMA_Alloc_GetTaintByAddress(BMMA_Heap_Handle h, void *addr)
{
    BMMA_Block_Handle  b;
    BDBG_OBJECT_ASSERT(h, BMMA_Heap);
    BMMA_P_Lock(h);
    b = BMMA_P_FindTaintedBlock(h, addr);
    BMMA_P_Unlock(h);
    return b;
}

#if BDBG_DEBUG_BUILD
BMMA_Block_Handle BMMA_Block_Create_tagged(BMMA_Handle a, BMMA_DeviceOffset offset, size_t size, void *address, const char *fname, unsigned line)
#else
BMMA_Block_Handle BMMA_Block_Create(BMMA_Handle a, BMMA_DeviceOffset offset, size_t size, void *address)
#endif
{
    BMMA_Block_Handle b=NULL;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(a, BMMA_Allocator);

    BDBG_MSG(("Create:>%p " BDBG_UINT64_FMT "(%u) %p %s:%u", (void *)a, BDBG_UINT64_ARG(offset), (unsigned)size, address, fname, line));

    b = BKNI_Malloc(sizeof(*b));
    if(b==NULL) {
        rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);return NULL;
    }
    BDBG_OBJECT_INIT(b, BMMA_Block);
    rc = BMMA_RangeAllocator_CreateBlock(&b->block, offset, size);
    if(rc!=BERR_SUCCESS) {
        rc=BERR_TRACE(rc);
        BKNI_Free(b);
        return NULL;
    }
    b->heap = &a->dummyHeap;
    b->lock_cnt = 1;
    b->lock_offset_cnt = 1;
    b->discardable = false;
    b->uncached = false;
    b->ref_cnt = 1;
    b->addr = address;
#if BDBG_DEBUG_BUILD
    b->fname = fname;
    b->line = line;
#endif
#if BMMA_ALLOC_BMEM_SUPPORT
    b->tainted = false;
#endif
    BDBG_MSG(("Create:>%p " BDBG_UINT64_FMT "(%u) %p %s:%u ->%p", (void *)a, BDBG_UINT64_ARG(offset), (unsigned)size, address, fname, line, (void *)b));
    return b;
}

void BMMA_Block_Acquire(BMMA_Handle a, BMMA_Block_Handle b)
{
    BDBG_MSG(("Acquire:>%p %p", (void *)a, (void *)b));
    BDBG_OBJECT_ASSERT(a, BMMA_Allocator);
    BDBG_OBJECT_ASSERT(b, BMMA_Block);
    BDBG_ASSERT(b->heap->parent == a);
    BKNI_AcquireMutex(a->lock);
    b->ref_cnt++;
    BKNI_ReleaseMutex(a->lock);
    BDBG_MSG(("Acquire:>%p %p -> %u", (void *)a, (void *)b, b->ref_cnt));
    return;
}
#endif /* BMMA_ALLOC_BMEM_SUPPORT */
