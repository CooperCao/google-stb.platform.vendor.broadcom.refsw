/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_core_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_memory.h"
#include "priv/nexus_core.h"
#include "bmem_debug.h"
#include "b_objdb.h"
#include "priv/nexus_base_platform.h"
#include "bmma_range.h"

BDBG_MODULE(nexus_memory);
BDBG_FILE_MODULE(nexus_core);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

struct NEXUS_Heap {
    NEXUS_OBJECT(NEXUS_Heap);
    unsigned index;
    BMEM_Heap_Handle memHeap;
    BMMA_Heap_Handle mmaHeap;
    NEXUS_Core_MemoryRegion settings;
    NEXUS_DisplayHeapSettings displayHeapSettings;
    bool guardBanding;
    bool dynamic;
    NEXUS_HeapRuntimeSettings runtimeSettings;
    BMRC_MonitorRegion_Handle secureMonitorRegion;
};

void NEXUS_Memory_GetDefaultAllocationSettings( NEXUS_MemoryAllocationSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if !NEXUS_CPU_ARM
void NEXUS_Heap_GetDefaultMemcSettings( unsigned memcIndex, NEXUS_Core_MemoryRegion *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->memcIndex = memcIndex;
    /* by default full access allowed */
    pSettings->memoryType = NEXUS_MemoryType_eFull;

    /* driver must set pSettings->alignment based on OS report of max dcache line size */

#if BCHP_CHIP == 7405 || BCHP_CHIP == 7400 || BCHP_CHIP == 7325 || BCHP_CHIP == 7335 || \
    BCHP_CHIP == 7340 || BCHP_CHIP == 7342
#define NEXUS_MEMC1_NOT_ADDRESSABLE 1
#endif

    if (memcIndex > 0) {
#if NEXUS_MEMC1_NOT_ADDRESSABLE
        /* MEMC1 is not cpu addressable */
        pSettings->memoryType = 0; /* deviceOnly */
        /* device heaps default to 1K alignment */
        pSettings->alignment = 1024;
#else
        /* MEMC1 is cpu addressable, but only user mode access is allowed
        This applies to 7422, 7344, 7425 and beyond. */
        pSettings->memoryType = NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
#endif
    }
}
#endif

#if !BMMA_USE_STUB
static bool NEXUS_Heap_P_OutOfMemory(void *context, size_t size, const BMMA_AllocationSettings *pSettings, unsigned itteration)
{
    NEXUS_HeapHandle heap = context;
    BMMA_Heap_CompactionStatus compactionStatus;
    /* XXX this function could be called without holding Core module lock, so call other functions with greate care, niether lock could be obtained since,  in some cases it's called with lock acquired */
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(compactionStatus);
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);

    if( (heap->settings.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)!=NEXUS_MEMORY_TYPE_DYNAMIC) {
        BDBG_WRN(("Out of memory %u(%u)", (unsigned)size, itteration));
    }
    return false;
}

static void *NEXUS_CoreModule_P_Mmap(void *context, void *state, BMMA_DeviceOffset base, size_t length)
{
    void *ptr;
    NEXUS_HeapHandle heap  = context;
    BSTD_UNUSED(state);

    if((heap->settings.memoryType & NEXUS_MEMORY_TYPE_NOT_MAPPED)==NEXUS_MEMORY_TYPE_NOT_MAPPED) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
    if((heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)==NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
        struct NEXUS_MemoryMapNode *memoryMap = state;
        NEXUS_Error rc = NEXUS_P_MemoryMap_Map(memoryMap, base, length);
        if(rc!=NEXUS_SUCCESS) {
            rc = BERR_TRACE(rc);
            return NULL;
        }
        ptr = memoryMap->lockedMem;
    } else {
        ptr=(uint8_t *)heap->settings.pvAddrCached + (base - heap->settings.offset);
    }

    return ptr;
}

static void NEXUS_CoreModule_P_Unmap(void *context, void *state, void *ptr, size_t length)
{
    NEXUS_HeapHandle heap  = context;

    BSTD_UNUSED(state);
    BSTD_UNUSED(ptr);

    if((heap->settings.memoryType & NEXUS_MEMORY_TYPE_NOT_MAPPED)==NEXUS_MEMORY_TYPE_NOT_MAPPED) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return ;
    }

    if((heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)==NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
        struct NEXUS_MemoryMapNode *memoryMap = state;

#if !B_REFSW_SYSTEM_MODE_CLIENT
        if(NEXUS_P_CpuAccessibleAddress(ptr)) {
            NEXUS_FlushCache(ptr, length);
        }
#endif

        NEXUS_P_MemoryMap_Unmap(memoryMap, length);
    }
    return;
}

static void NEXUS_CoreModule_P_TraceAlloc(void *context, BMMA_DeviceOffset base, size_t size, const char *fname, unsigned line)
{
    NEXUS_HeapHandle heap = context;
    int memc;

    memc = heap->settings.memcIndex;
    if(g_NexusCore.publicHandles.memc[memc].mrc) {
        g_NexusCore.publicHandles.memc[memc].mem_monitor.alloc(g_NexusCore.publicHandles.memc[memc].mem_monitor.cnxt, base, size, fname, line);
    }
    return;
}

static void NEXUS_CoreModule_P_TraceFree(void *context, BMMA_DeviceOffset base, size_t size)
{
    NEXUS_HeapHandle heap = context;
    int memc;

    if((heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)!=NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED &&
       (heap->settings.memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED) == NEXUS_MEMORY_TYPE_DRIVER_CACHED
       ) {
        void *ptr = NEXUS_OffsetToCachedAddr(base);
        if(ptr) {
            NEXUS_FlushCache(ptr, size);
        }
    }

    memc = heap->settings.memcIndex;
    if(g_NexusCore.publicHandles.memc[memc].mrc) {
        g_NexusCore.publicHandles.memc[memc].mem_monitor.free(g_NexusCore.publicHandles.memc[memc].mem_monitor.cnxt, base);
    }
    return;
}
#endif /* BMMA_USE_STUB */

/* export custom_arc=y tells Nexus to not set up or change ARC settings. Just leave it alone. */
static bool nexus_p_custom_arc(void)
{
    return NEXUS_GetEnv("custom_arc") != NULL;
}

NEXUS_HeapHandle NEXUS_Heap_Create_priv( unsigned index, const NEXUS_Core_MemoryRegion *pSettings )
{
    NEXUS_HeapHandle heap = NULL;
    BERR_Code rc;
#if BMMA_USE_STUB
    BMEM_Heap_Settings mem_heap_settings;
#else
    BMMA_Heap_CreateSettings heapSettings;
    BMMA_Bmem_Settings bmemSettings;
#endif

    BDBG_MSG(("NEXUS_Heap_Create %u: MEMC%u, offset " BDBG_UINT64_FMT ", length %d, addr %p, cached %p",
        index, pSettings->memcIndex, BDBG_UINT64_ARG(pSettings->offset), (unsigned)pSettings->length, pSettings->pvAddr, pSettings->pvAddrCached));

    g_NexusCore.publicHandles.heap[index].mem = NULL;
    g_NexusCore.publicHandles.heap[index].mma = NULL;
    g_NexusCore.publicHandles.heap[index].nexus = NULL;

    heap = BKNI_Malloc(sizeof(*heap));
    if (!heap) {
        rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Heap, heap);
    heap->settings = *pSettings;
    heap->dynamic = false;

    if (pSettings->memoryType == NEXUS_MEMORY_TYPE_RESERVED) {
        /* don't create a BMEM heap for a reserved heap */
        goto skip_bmem;
    }

#if BMMA_USE_STUB
    (void)BMEM_Heap_GetDefaultSettings(&mem_heap_settings);
    if(pSettings->offset + pSettings->length > ((uint64_t)1)<<32) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }
#else
     BMMA_Heap_GetDefaultCreateSettings(&heapSettings);
#endif

    if( (pSettings->memoryType & NEXUS_MEMORY_TYPE_HIGH_MEMORY) == NEXUS_MEMORY_TYPE_HIGH_MEMORY) {
        if( (pSettings->memoryType & NEXUS_MEMORY_TYPE_MANAGED) != NEXUS_MEMORY_TYPE_MANAGED || /* HIGH_MEMORY must be MANAGED - can't use 32-bit device address */
            (pSettings->memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) != NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED /*  currently HIGH_MEMORY requires ONDEMAND_MAPPED - no pointers in API, since they imply use of 32-bit offset */
            ) {
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto error;
        }
    } else if(pSettings->offset + pSettings->length > ((uint64_t)1)<<32) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }
    if( (pSettings->memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) == NEXUS_MEMORY_TYPE_DYNAMIC &&
       ((pSettings->memoryType & NEXUS_MEMORY_TYPE_NOT_MAPPED)!=NEXUS_MEMORY_TYPE_NOT_MAPPED &&
        (pSettings->memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)!=NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    if( (pSettings->memoryType & (NEXUS_MEMORY_TYPE_NOT_MAPPED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED)) == 0) {
        rc = NEXUS_P_AddMap(pSettings->offset, pSettings->pvAddrCached, pSettings->pvAddr, pSettings->length);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}
    }

#if BMMA_USE_STUB
    BDBG_ASSERT(pSettings->pvAddr);
    BDBG_ASSERT(pSettings->pvAddrCached);
    if( (pSettings->memoryType & (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED)) == (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED)) {
        /* BMEM_SafetyConfig_eTrack will store file/line information and will guardband every allocation,
        but will not slow down malloc/free with BMEM_Heap_Validate calls. Instead, export debug_mem=y causes
        BMEM_Heap_Validate to be called from a timer.
        guard bands require driver-context memory mapping. */
        mem_heap_settings.eSafetyConfig = BMEM_SafetyConfig_eTrack;
        mem_heap_settings.eBookKeeping = BMEM_BookKeeping_eSystem;
        heap->guardBanding = true;
    }
    else {
        /* BMEM_SafetyConfig_eNormal means file/line information is stored and no guardbands are provided.
        if there is no driver-context memory mapping, this is required. */
        mem_heap_settings.eSafetyConfig = BMEM_SafetyConfig_eNormal;
        mem_heap_settings.eBookKeeping = BMEM_BookKeeping_eSystem;
        heap->guardBanding = false;
    }
    rc = NEXUS_Memory_P_ConvertAlignment(pSettings->alignment, &mem_heap_settings.uiAlignment);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}

    mem_heap_settings.pCachedAddress = pSettings->pvAddrCached;

    rc = BMEM_Heap_Create(g_NexusCore.publicHandles.mem, pSettings->pvAddr, pSettings->offset, pSettings->length, &mem_heap_settings, &heap->memHeap);
    if (rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    BDBG_ASSERT(heap->memHeap);
    heap->mmaHeap = (BMMA_Heap_Handle)heap->memHeap;
    if (!nexus_p_custom_arc()) {
        rc = BMEM_InstallMonitor(heap->memHeap, &g_NexusCore.publicHandles.memc[pSettings->memcIndex].mem_monitor);
        if (rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
    }

#else /* BMMA_USE_STUB */
    heapSettings.minAlignment = pSettings->alignment;
    if( (heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
        if(heapSettings.minAlignment < 4096) { /* allocations must be page aligned */
            heapSettings.minAlignment = 4096;
        }
    }
    heapSettings.base = pSettings->offset;
    heapSettings.size = pSettings->length;
    heapSettings.context = heap;
    heapSettings.out_of_memory = NEXUS_Heap_P_OutOfMemory;
    heapSettings.mmap = NEXUS_CoreModule_P_Mmap;
    heapSettings.unmap = NEXUS_CoreModule_P_Unmap;
    heapSettings.alloc = NEXUS_CoreModule_P_TraceAlloc;
    heapSettings.free = NEXUS_CoreModule_P_TraceFree;
    heapSettings.mmapStateSize = sizeof(struct NEXUS_MemoryMapNode);
    heapSettings.flags.silent = (pSettings->memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)==NEXUS_MEMORY_TYPE_DYNAMIC;

    heapSettings.base_uncached = pSettings->pvAddrCached == pSettings->pvAddr ? NULL : pSettings->pvAddr;

    BDBG_ASSERT(g_NexusCore.publicHandles.mma);
    rc = BMMA_Heap_Create(&heap->mmaHeap, g_NexusCore.publicHandles.mma, &heapSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }

    BMMA_Bmem_GetDefaultSettings(&bmemSettings);
    bmemSettings.length = pSettings->length;
    bmemSettings.flush_cache = NEXUS_FlushCache;
    if( (pSettings->memoryType & NEXUS_MEMORY_TYPE_MANAGED)==NEXUS_MEMORY_TYPE_MANAGED) {
        bmemSettings.dummy = true;
    } else {
        if( (pSettings->offset + pSettings->length) >= ((uint64_t)1)<<32) {
            BDBG_ERR(("%u: can't create BMEM heap for range " BDBG_UINT64_FMT "..." BDBG_UINT64_FMT "", index, BDBG_UINT64_ARG(pSettings->offset), BDBG_UINT64_ARG(pSettings->offset + pSettings->length)));
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        bmemSettings.base = pSettings->offset;
        bmemSettings.cached = pSettings->pvAddrCached;
        bmemSettings.uncached = pSettings->pvAddr;
    }
    rc = BMMA_Bmem_Attach(heap->mmaHeap, g_NexusCore.publicHandles.mem, &heap->memHeap, &bmemSettings);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
#endif /* BMMA_USE_STUB */

    g_NexusCore.publicHandles.heap[index].mem = heap->memHeap;
    g_NexusCore.publicHandles.heap[index].mma = heap->mmaHeap;

skip_bmem:
    heap->index = index;
    g_NexusCore.publicHandles.heap[index].nexus = heap;
    return heap;

error:
    if (heap) {
        BKNI_Free(heap);
    }
    return NULL;
}

NEXUS_HeapHandle NEXUS_Heap_CreateInternal (void)
{
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_Heap_DestroyInternal( NEXUS_HeapHandle heap)
{
    BSTD_UNUSED(heap);
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
}

static void NEXUS_Heap_P_Finalizer( NEXUS_HeapHandle heap )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Heap, heap);
    BDBG_ASSERT(heap == g_NexusCore.publicHandles.heap[heap->index].nexus);
#if BMMA_USE_STUB
    if (heap->memHeap) {
        BMEM_Heap_Destroy(heap->memHeap);
    }
#else
    if(heap->mmaHeap) {
        BMMA_Bmem_Detach(heap->memHeap);
        BMMA_Heap_Destroy(heap->mmaHeap);
    }
#endif
    g_NexusCore.publicHandles.heap[heap->index].nexus = NULL;
    g_NexusCore.publicHandles.heap[heap->index].mem = NULL;
    g_NexusCore.publicHandles.heap[heap->index].mma = NULL;
    NEXUS_OBJECT_DESTROY(NEXUS_Heap, heap);
    BKNI_Free(heap);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Heap, NEXUS_Heap_Destroy_priv);

void NEXUS_Heap_GetDefaultDisplayHeapSettings( NEXUS_DisplayHeapSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fullHdBuffers.count = 4;
    pSettings->fullHdBuffers.format = NEXUS_VideoFormat_e1080p30hz;
    pSettings->fullHdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
    pSettings->hdBuffers.format = NEXUS_VideoFormat_e1080i;
    pSettings->hdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
    pSettings->sdBuffers.format = NEXUS_VideoFormat_ePalG;
    pSettings->sdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
}

void NEXUS_Heap_GetDisplayHeapSettings( NEXUS_HeapHandle heap, NEXUS_DisplayHeapSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    *pSettings = heap->displayHeapSettings;
}

NEXUS_Error NEXUS_Heap_SetDisplayHeapSettings( NEXUS_HeapHandle heap, const NEXUS_DisplayHeapSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    heap->displayHeapSettings = *pSettings;
    return 0;
}

NEXUS_Error NEXUS_Core_HeapMemcIndex_isrsafe(unsigned heapIndex, unsigned *pMemcIndex)
{
    if (heapIndex < NEXUS_MAX_HEAPS && g_pCoreHandles->heap[heapIndex].nexus) {
        *pMemcIndex = g_pCoreHandles->heap[heapIndex].nexus->settings.memcIndex;
        return NEXUS_SUCCESS;
    }
    return NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NEXUS_Heap_GetStatus_driver_priv( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus )
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (!heap) return NEXUS_INVALID_PARAMETER;

    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    pStatus->alignment = heap->settings.alignment;
    pStatus->memcIndex = heap->settings.memcIndex;
    pStatus->memoryType = heap->settings.memoryType;
    pStatus->heapType = heap->settings.heapType;
    pStatus->offset = heap->settings.offset;
    pStatus->size = heap->settings.length;
    pStatus->guardBanding = heap->guardBanding;
    pStatus->dynamic = heap->dynamic;
    pStatus->addr = NULL;

#if BMMA_USE_STUB
    {
        BMEM_Heap_Handle mheap;
        BMEM_HeapInfo info;

        mheap = NEXUS_Heap_GetMemHandle(heap);
        if (!mheap) {
            /* if there is no BMEM heap, return what we have */
            return NEXUS_SUCCESS;
        }

        /* get status from BMEM */
        BMEM_Heap_GetInfo(mheap, &info);
        pStatus->free = info.ulTotalFree;
        pStatus->numAllocs = info.ulNumAllocated;
        pStatus->numFreeBlocks = info.ulNumFree;
        pStatus->largestFreeBlock = info.ulLargestFree;
        pStatus->highWatermark = info.ulHighWatermark;
    }
#else /* BMMA_USE_STUB */
    if(heap->mmaHeap) {
        BMMA_Heap_FastStatus fastStatus;
        BERR_Code rc;

        rc = BMMA_Heap_GetStatus(heap->mmaHeap, &fastStatus, NULL);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto done;}
        pStatus->size = fastStatus.size;
        pStatus->free = fastStatus.totalFree;
        pStatus->numAllocs = fastStatus.numAllocated;
        pStatus->numFreeBlocks = fastStatus.numFree;
        pStatus->largestFreeBlock = fastStatus.largestFree;
        pStatus->highWatermark = fastStatus.highWatermark;
    }
done:
#endif /* BMMA_USE_STUB */

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Heap_GetStatus_driver( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus )
{
    return NEXUS_Heap_GetStatus_driver_priv(heap, pStatus);
}

NEXUS_Error NEXUS_Heap_GetStatus_priv( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus )
{
    NEXUS_Error rc;
    rc = NEXUS_Heap_GetStatus_driver_priv(heap, pStatus);
    if (!rc && (pStatus->memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED)) {
        pStatus->addr = NEXUS_OffsetToCachedAddr(pStatus->offset);
    }
    return rc;
}

NEXUS_Error NEXUS_Heap_AddRegion_priv(NEXUS_HeapHandle heap, NEXUS_Addr base, size_t size)
{
    BERR_Code rc;
    NEXUS_ASSERT_MODULE();
    NEXUS_OBJECT_ASSERT(NEXUS_Heap, heap);
    if( (heap->settings.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)!=NEXUS_MEMORY_TYPE_DYNAMIC) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    rc = BMMA_Heap_AddRegion(heap->mmaHeap, base, size);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    heap->dynamic = true;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Heap_RemoveRegion_priv(NEXUS_HeapHandle heap, NEXUS_Addr base, size_t size)
{
    BERR_Code rc;
    NEXUS_ASSERT_MODULE();
    NEXUS_OBJECT_ASSERT(NEXUS_Heap, heap);

    rc = BMMA_Heap_RemoveRegion(heap->mmaHeap, base, size);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    return NEXUS_SUCCESS;
}

void NEXUS_Heap_GetFreeRegions_priv(NEXUS_HeapHandle heap, NEXUS_MemoryRegion *regions, unsigned numEntries, unsigned *pNumReturned)
{
    NEXUS_ASSERT_MODULE();
    NEXUS_OBJECT_ASSERT(NEXUS_Heap, heap);

    NEXUS_ASSERT_STRUCTURE(NEXUS_MemoryRegion, BMMA_Heap_FreeRegion);
    NEXUS_ASSERT_FIELD(NEXUS_MemoryRegion, base, BMMA_Heap_FreeRegion, base);
    NEXUS_ASSERT_FIELD(NEXUS_MemoryRegion, length, BMMA_Heap_FreeRegion, length);
    NEXUS_ASSERT_FIELD(NEXUS_MemoryRegion, boundary, BMMA_Heap_FreeRegion, boundary);

    BMMA_Heap_GetFreeRegions(heap->mmaHeap, (BMMA_Heap_FreeRegion*)regions, numEntries, pNumReturned);

    return;
}

NEXUS_Error NEXUS_Memory_P_ConvertAlignment(unsigned alignment, unsigned *pAlignmentExponent)
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

BMEM_Heap_Handle NEXUS_Heap_GetMemHandle( NEXUS_HeapHandle heap  )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    return heap->memHeap;
}

BMMA_Heap_Handle NEXUS_Heap_GetMmaHandle( NEXUS_HeapHandle heap  )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    BDBG_ASSERT(heap->mmaHeap);
    return heap->mmaHeap;
}

NEXUS_HeapHandle NEXUS_Heap_GetHeapFromMmaHandle( BMMA_Heap_Handle mma )
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_pCoreHandles->heap[i].mma == mma) {
            return g_pCoreHandles->heap[i].nexus;
        }
    }
    BDBG_ERR(("Invalid heap specified"));
    return NULL;
}

void NEXUS_Memory_PrintHeaps( void )
{
    unsigned i;
    NEXUS_Error rc;
    BDBG_MODULE_LOG(nexus_core, ("heap offset memc size        MB vaddr      used peak name"));
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus status;
        NEXUS_HeapHandle heap;
        char buf[128];
        unsigned size_percentage;
        void *addr;

        heap = g_pCoreHandles->heap[i].nexus;
        if (!heap) continue;
        rc = NEXUS_Heap_GetStatus_driver(heap, &status);
        if (rc) continue;
        size_percentage = status.size/100; /* avoid overflow by dividing size by 100 to get percentage */
        if ((status.memoryType & (NEXUS_MEMORY_TYPE_NOT_MAPPED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED | NEXUS_MEMORY_TYPE_RESERVED)) == 0) {
            addr = NEXUS_OffsetToCachedAddr(status.offset);
        }
        else {
            addr = NULL;
        }
        if (status.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) {
            status.offset = 0;
        }
        if(status.highWatermark > status.size) {
            status.highWatermark = status.size; /* 100% to avoid obscure number. highWatermark could be much larger than current size if NEXUS_Heap_RemoveRegion_priv was used */
        }
        NEXUS_Heap_ToString(&status, buf, sizeof(buf));
        BDBG_MODULE_LOG(nexus_core, ("%-2u " BDBG_UINT64_FMT " %u 0x%-8x %3d 0x%08lx %3d%% %3d%% %s",
            i, BDBG_UINT64_ARG(status.offset), status.memcIndex, (unsigned)status.size, (unsigned)(status.size/(1024*1024)), (unsigned long)addr,
            size_percentage?(status.size-status.free)/size_percentage:0,
            size_percentage?status.highWatermark/size_percentage:0,
            buf));
    }
}

void NEXUS_Memory_PrintStatus( void )
{
    unsigned i;
    for(i=0;i<sizeof(g_NexusCore.publicHandles.memc)/sizeof(g_NexusCore.publicHandles.memc[0]);i++) {
        BMRC_Monitor_Handle mrc = g_NexusCore.publicHandles.memc[i].rmm;
        if(mrc) {
            BMRC_Monitor_Print(mrc);
        }
    }
    return;
}

void NEXUS_Heap_Dump( NEXUS_HeapHandle heap )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
#if !BMMA_USE_STUB
    if (heap->mmaHeap) {
        BMMA_Dbg_DumpHeap(heap->mmaHeap);
    }
#else
    if (heap->memHeap) {
        BMEM_Dbg_DumpHeap(heap->memHeap);
    }
#endif
}


void NEXUS_Heap_ResetHighWatermark( NEXUS_HeapHandle heap )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
#if !BMMA_USE_STUB
    if (heap->mmaHeap) {
        BMMA_Heap_ResetHighWatermark(heap->mmaHeap);
    }
#else
    if (heap->memHeap) {
        BMEM_Heap_ResetHighWatermark(heap->memHeap);
    }
#endif
}

bool NEXUS_P_CpuAccessibleHeap( NEXUS_HeapHandle heap )
{
    BDBG_OBJECT_ASSERT(heap, NEXUS_Heap);
    /* only cached mapping should be accessed by CPU */
    return NEXUS_P_CpuAccessibleAddress(NEXUS_OffsetToCachedAddr(heap->settings.offset));
}

void NEXUS_MemoryBlock_GetDefaultAllocationSettings(NEXUS_MemoryBlockAllocationSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_P_CreateFromMma_priv(BMMA_Block_Handle mma_block)
{
    NEXUS_MemoryBlockHandle block;
    BERR_Code rc;

    block = BMMA_PoolAllocator_Alloc(g_NexusCore.memoryBlockPool);
    if(block==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    NEXUS_OBJECT_INIT(NEXUS_MemoryBlock, block);
    BKNI_Memset(&block->state, 0, sizeof(block->state));
    block->block = mma_block;
    block->lockCnt = 0;
    block->offset = 0;
    BLST_D_INSERT_HEAD(&g_NexusCore.allocatedBlocks, block, link);
    return block;

err_alloc:
    return NULL;
}

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_FromMma_priv(BMMA_Block_Handle mma_block)
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_ASSERT_MODULE();

    for(block=BLST_D_FIRST(&g_NexusCore.allocatedBlocks);block;block=BLST_D_NEXT(block,link)) {
        if(mma_block==block->block) {
            NEXUS_OBJECT_ACQUIRE(block, NEXUS_MemoryBlock, block); /* owner is unknown, so use object as the owner */
            return block;
        }
    }
    block = NEXUS_MemoryBlock_P_CreateFromMma_priv(mma_block);
    if(block) {
        BMMA_Block_Acquire(g_NEXUS_pCoreHandles->mma, mma_block);
    }
    return block;
}

NEXUS_Error NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv(void *lockedMem, size_t size,
    NEXUS_MemoryBlockHandle *pBlock, unsigned *pOffset)
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_Addr addr = NEXUS_AddrToOffset(lockedMem);

    NEXUS_ASSERT_MODULE();

    for(block=BLST_D_FIRST(&g_NexusCore.allocatedBlocks);block;block=BLST_D_NEXT(block,link)) {
        if(block->lockCnt && addr >= block->offset) {
            BMMA_BlockProperties mmaProperties;

            BMMA_Block_GetProperties(block->block, &mmaProperties);
            if(addr+size <= block->offset+mmaProperties.size) {
                *pBlock = block;
                if (pOffset) {
                    *pOffset = (uint8_t *)lockedMem - (uint8_t *)NEXUS_OffsetToCachedAddr(block->offset);
                }
                return 0;
            }
        }
    }
    return NEXUS_UNKNOWN;
}

#if BDBG_DEBUG_BUILD
struct NEXUS_FileNameNode
{
    BLST_AA_TREE_ENTRY(NEXUS_FileNameNode) node;
    unsigned refcnt;
    char fileName[sizeof(unsigned)]; /* this is actually of variable size, must be last field */
};

static int NEXUS_P_MemoryBlockTag_Compare(const struct NEXUS_FileNameNode *node, const char *fileName)
{
    return NEXUS_StrCmp(node->fileName, fileName);
}

BLST_AA_TREE_GENERATE_FIND(NEXUS_P_FileNameTree, const char *, NEXUS_FileNameNode, node, NEXUS_P_MemoryBlockTag_Compare)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_FileNameTree, const char *, NEXUS_FileNameNode, node, NEXUS_P_MemoryBlockTag_Compare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_FileNameTree, NEXUS_FileNameNode, node)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_FileNameTree, NEXUS_FileNameNode, node)

static struct NEXUS_FileNameNode *NEXUS_Memory_P_AllocFileName(const NEXUS_MemoryBlockTag *pTag)
{
    struct NEXUS_FileNameNode *node;

    node = BLST_AA_TREE_FIND(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree, pTag->fileName);
    if (!node) {
        int len = b_strlen(pTag->fileName);
        node = BKNI_Malloc( (sizeof(*node) - sizeof(node->fileName)) + (len+1)); /* dynamic size entry */
        if (!node) return NULL;
        node->refcnt = 0;
        b_strncpy(node->fileName, pTag->fileName, len + 1);
        BLST_AA_TREE_INSERT(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree, pTag->fileName, node);
    }
    node->refcnt++;
    BDBG_MSG_TRACE(("alloc %s refcnt %u", tag->fileName, tag->refcnt));
    return node;
}

static void NEXUS_Memory_P_FreeFileName(struct NEXUS_FileNameNode *tag)
{
    BDBG_MSG_TRACE(("free %s:%d refcnt %d", tag->tag.fileName, tag->tag.lineNumber, tag->refcnt-1));
    if (--tag->refcnt == 0) {
        BLST_AA_TREE_REMOVE(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree, tag);
        BKNI_Free(tag);
    }
}
#endif /* BDBG_DEBUG_BUILD */

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Allocate_driver(NEXUS_HeapHandle allocHeap, size_t numBytes, size_t nexusAlignment, const NEXUS_MemoryBlockAllocationSettings *pSettings,
    const NEXUS_MemoryBlockTag *pTag, NEXUS_MemoryBlockProperties *pProperties)
{
    unsigned alignment=0;
    NEXUS_Error rc;
    NEXUS_HeapHandle nexusHeap;
    NEXUS_MemoryBlockHandle block;
    BMMA_Block_Handle mma_block;
    struct NEXUS_FileNameNode *node;
    BMMA_AllocationSettings mmaSettings;

    BSTD_UNUSED(pSettings);

    if (numBytes <= 0) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto err_parameter; }
    if ( pTag->fileName[sizeof(pTag->fileName)-1] != '\0' ) { rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_parameter; } /* bad data passed to the driver we need to ensure that string is properly terminated */

    rc = NEXUS_Memory_P_ConvertAlignment(nexusAlignment, &alignment);
    if (rc) {
        BDBG_ERR(("Invalid alignment (%u) specified", (unsigned)nexusAlignment));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);goto err_parameter;
    }

    nexusHeap = NEXUS_P_DefaultHeap(allocHeap, NEXUS_DefaultHeapType_eAny);
    if (nexusHeap==NULL) {
        nexusHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
        if(nexusHeap==NULL) {
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_parameter;
        }
    }
    BDBG_OBJECT_ASSERT(nexusHeap, NEXUS_Heap);

    if (nexusHeap->settings.locked) { rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_parameter; }
    if(nexusHeap->mmaHeap==NULL) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_parameter;
    }

    BMMA_GetDefaultAllocationSettings(&mmaSettings);
#if BDBG_DEBUG_BUILD
    node = NEXUS_Memory_P_AllocFileName(pTag);
    if (!node) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_tag;}

    mma_block = BMMA_Alloc_tagged(nexusHeap->mmaHeap, numBytes, 1<<alignment, &mmaSettings, node->fileName, pTag->lineNumber);
#else
    mma_block = BMMA_Alloc(nexusHeap->mmaHeap, numBytes, 1<<alignment, &mmaSettings);
#endif
    if(!mma_block) {
        bool silent = (nexusHeap->settings.memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)==NEXUS_MEMORY_TYPE_DYNAMIC;
        rc= silent ? BERR_OUT_OF_DEVICE_MEMORY : BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto err_mem_alloc;
    }

    block = NEXUS_MemoryBlock_P_CreateFromMma_priv(mma_block);
    if(!block) { rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_create;}

#if BDBG_DEBUG_BUILD
    block->fileNameNode = node;
#endif
    block->size = numBytes;
    block->nexusAlignment = nexusAlignment;
    block->heap = allocHeap;

    pProperties->size = numBytes;
    pProperties->memoryType = nexusHeap->settings.memoryType;

    return block;

err_create:
    BMMA_Free(mma_block);
err_mem_alloc:
#if BDBG_DEBUG_BUILD
    NEXUS_Memory_P_FreeFileName(node);
err_tag:
#endif
err_parameter:
    return NULL;
}

void NEXUS_MemoryBlock_GetProperties( NEXUS_MemoryBlockHandle block, NEXUS_MemoryBlockProperties *pProperties)
{
    BMMA_BlockProperties mmaProperties;
    NEXUS_HeapHandle nexusHeap;

    BMMA_Block_GetProperties(block->block, &mmaProperties);
    nexusHeap = NEXUS_Heap_GetHeapFromMmaHandle(mmaProperties.heap);
    pProperties->size = mmaProperties.size;
    pProperties->memoryType = 0;
    if(nexusHeap) {
        pProperties->memoryType = nexusHeap->settings.memoryType;
    }
    return ;
}

static void NEXUS_MemoryBlock_P_Finalizer(NEXUS_MemoryBlockHandle block)
{
    NEXUS_OBJECT_ASSERT(NEXUS_MemoryBlock, block);

    if(block->state.driver.valid) {
        NEXUS_MemoryBlock_Release_local(block, &block->state.driver.state);
    }
    if(block->lockCnt) {
        BMMA_UnlockOffset(block->block, block->offset);
    }
    BLST_D_REMOVE(&g_NexusCore.allocatedBlocks, block, link);
    BMMA_Free(block->block);
#if BDBG_DEBUG_BUILD
    if (block->fileNameNode) {
        NEXUS_Memory_P_FreeFileName(block->fileNameNode);
    }
#endif
    NEXUS_OBJECT_DESTROY(NEXUS_MemoryBlock, block);
    BMMA_PoolAllocator_Free(g_NexusCore.memoryBlockPool, block);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_MemoryBlock, NEXUS_MemoryBlock_Free_driver);

void NEXUS_MemoryBlock_P_Print(void)
{
    unsigned allocatedSize=0;
    unsigned allocatedCount=0;
    unsigned relocatableSize=0;
    unsigned relocatableCount=0;
    NEXUS_MemoryBlockHandle block;

    for(block=BLST_D_FIRST(&g_NexusCore.allocatedBlocks);block;block=BLST_D_NEXT(block,link)) {
        BMMA_BlockProperties mmaProperties;

        BMMA_Block_GetProperties(block->block, &mmaProperties);

        allocatedCount ++;
        allocatedSize += mmaProperties.size;
        if(block->lockCnt==0) {
            relocatableCount ++;
            relocatableSize += mmaProperties.size;
        }
    }
    BDBG_MODULE_LOG(nexus_core, ("memory blocks: Allocated:%u(%u) Relocatable:%u(%u)", allocatedCount, allocatedSize, relocatableCount, relocatableSize));
    return;
}

NEXUS_Addr NEXUS_MemoryBlock_GetOffset_priv(NEXUS_MemoryBlockHandle block, unsigned blockOffset)
{
    BMMA_DeviceOffset offset;

    NEXUS_OBJECT_ASSERT(NEXUS_MemoryBlock, block);

    offset = BMMA_LockOffset(block->block);
    BMMA_UnlockOffset(block->block,offset);
    if(offset) {
        offset += blockOffset;
    }
    return offset;
}

BMMA_Block_Handle NEXUS_MemoryBlock_GetBlock_priv(NEXUS_MemoryBlockHandle block)
{
    NEXUS_OBJECT_ASSERT(NEXUS_MemoryBlock, block);
    return block->block;
}

NEXUS_Error NEXUS_MemoryBlock_LockOffset(NEXUS_MemoryBlockHandle block, NEXUS_Addr *addr)
{
    if(block->lockCnt==0) {
        block->offset = BMMA_LockOffset(block->block);
        if(block->offset==0) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    *addr = block->offset;
    block->lockCnt++;
    return NEXUS_SUCCESS;
}

void NEXUS_MemoryBlock_UnlockOffset(NEXUS_MemoryBlockHandle block)
{
    if(block->lockCnt>0) {
        block->lockCnt--;
        if(block->lockCnt==0) {
            BMMA_UnlockOffset(block->block, block->offset);
            block->offset = 0;
        }
    }
    return;
}

NEXUS_Error NEXUS_MemoryBlock_GetUserState(NEXUS_MemoryBlockHandle memoryBlock, NEXUS_MemoryBlockUserState *userState)
{
    unsigned i;
    const struct b_objdb_client *client = b_objdb_get_client();

    NEXUS_OBJECT_ASSERT(NEXUS_MemoryBlock, memoryBlock);
    BDBG_ASSERT(client);
    for(i=0;i<sizeof(memoryBlock->state.application)/sizeof(memoryBlock->state.application[0]);i++) {
        if(memoryBlock->state.application[i].client == client) {
            *userState = memoryBlock->state.application[i].state;
            return NEXUS_SUCCESS;
        }
    }
    userState->state = NULL;
    return NEXUS_NOT_AVAILABLE;
}

void NEXUS_MemoryBlock_SetUserState( NEXUS_MemoryBlockHandle memoryBlock, const NEXUS_MemoryBlockUserState *userState)
{
    unsigned i;
    const struct b_objdb_client *client = b_objdb_get_client();

    NEXUS_OBJECT_ASSERT(NEXUS_MemoryBlock, memoryBlock);
    BDBG_ASSERT(client);
    for(i=0;i<sizeof(memoryBlock->state.application)/sizeof(memoryBlock->state.application[0]);i++) {
        if(memoryBlock->state.application[i].client == client) {
            if(userState) {
                memoryBlock->state.application[i].state = *userState;
            } else {
                memoryBlock->state.application[i].client = NULL;
            }
        }
    }
    if(userState) {
        for(i=0;i<sizeof(memoryBlock->state.application)/sizeof(memoryBlock->state.application[0]);i++) {
            if(memoryBlock->state.application[i].client == NULL) {
                memoryBlock->state.application[i].client = client;
                memoryBlock->state.application[i].state = *userState;
                return;
            }
        }
    }

    return;
}


void NEXUS_CoreModule_Uninit_Client_priv(struct b_objdb_client *client)
{
    NEXUS_MemoryBlockHandle block;
    /* when client exits, clean-up all stored per-client state.
     * On the client side, it could lead to the resource leak, but not to dereferencing of dangle pointer */
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(client);

    for(block=BLST_D_FIRST(&g_NexusCore.allocatedBlocks); block; block=BLST_D_NEXT(block, link)) {
        unsigned i;
        for(i=0;i<sizeof(block->state.application)/sizeof(block->state.application[0]);i++) {
            if(block->state.application[i].client == client) {
                block->state.application[i].client = NULL;
                break;
            }
        }
    }

    return;
}

void NEXUS_Core_DumpHeaps_priv(void *context)
{
    unsigned i;
    BSTD_UNUSED(context);
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_pCoreHandles->heap[i].nexus) {
            NEXUS_Heap_Dump(g_pCoreHandles->heap[i].nexus);
        }
    }
}

NEXUS_Error NEXUS_Memory_P_Init(void)
{
#if BDBG_DEBUG_BUILD
    BLST_AA_TREE_INIT(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree);
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_Memory_P_Uninit(void)
{
#if BDBG_DEBUG_BUILD
    struct NEXUS_FileNameNode *tag;
    while ((tag = BLST_AA_TREE_FIRST(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree))) {
        BLST_AA_TREE_REMOVE(NEXUS_P_FileNameTree, &g_NexusCore.fileNameTree, tag);
        BKNI_Free(tag);
    }
#endif
}

void NEXUS_Memory_GetVideoSecureHeap( NEXUS_Addr *pOffset, unsigned *pSize )
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus status;
        if (!g_pCoreHandles->heap[i].nexus) continue;
        NEXUS_Heap_GetStatus_driver(g_pCoreHandles->heap[i].nexus, &status);
        if (status.heapType == NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION) {
            *pOffset = status.offset;
            *pSize = status.size;
            return;
        }
    }
    *pOffset = 0;
    *pSize = 0;
    return;
}

/*
driver-side lock/unlock
assumes block will always be unlocked immediately after lock. no need to keep state. */
static NEXUS_Error NEXUS_MemoryBlock_P_Lock(NEXUS_MemoryBlockHandle block, void **ppMemory)
{
    NEXUS_Addr addr;
    void *ptr;
    BERR_Code rc = NEXUS_MemoryBlock_LockOffset(block, &addr);
    if (rc) {rc = BERR_TRACE(rc); goto err_lockoffset;}
    if ((block->heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
        ptr = NEXUS_Platform_P_MapMemory(addr, block->size, NEXUS_MemoryMapType_eCached);
        if (!ptr) {rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_map;}
    }
    else {
        ptr = NEXUS_OffsetToCachedAddr(addr);
        if (!ptr) {rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_map;}
    }
    *ppMemory = ptr;
    return NEXUS_SUCCESS;
err_map:
    NEXUS_MemoryBlock_UnlockOffset(block);
err_lockoffset:
    return rc;
}

static void NEXUS_MemoryBlock_P_Unlock(NEXUS_MemoryBlockHandle block, void *ptr)
{
    if ((block->heap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
        NEXUS_Platform_P_UnmapMemory(ptr, block->size, NEXUS_MemoryMapType_eCached);
    }
    NEXUS_MemoryBlock_UnlockOffset(block);
    return;
}

NEXUS_Error NEXUS_Memory_MoveUnlockedBlocks( NEXUS_HeapHandle fromHeap, NEXUS_HeapHandle toHeap, unsigned maxNumber, unsigned *pNumMoved )
{
    NEXUS_MemoryBlockHandle block;
    NEXUS_Error rc = NEXUS_SUCCESS;

    *pNumMoved = 0;
    for (block=BLST_D_FIRST(&g_NexusCore.allocatedBlocks);block && maxNumber;block=BLST_D_NEXT(block,link)) {
        NEXUS_MemoryBlockHandle newBlock;
        NEXUS_MemoryBlockTag tag;
        NEXUS_MemoryBlockProperties blockProperties; /* unused */
        void *fromPtr, *toPtr;
        unsigned size, alignment;

        if (block->heap != fromHeap || block->lockCnt) continue;

        rc = NEXUS_MemoryBlock_P_Lock(block, &fromPtr);
        if (rc) return BERR_TRACE(rc);

        tag.fileName[0] = 0; /* TODO */
        tag.lineNumber = 0; /* TODO */
        size = block->size;
        alignment = block->nexusAlignment;
        if (toHeap->settings.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) {
            /* must do 4K aligned allocation because of on-demand mmap */
            if (alignment < 4096) {
                alignment = 4096;
            }
            if (size % 4096) {
                size += 4096 - (size % 4096);
            }
        }
        newBlock = NEXUS_MemoryBlock_Allocate_driver(toHeap, size, alignment, NULL, &tag, &blockProperties);
        if (!newBlock) {
            rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            NEXUS_MemoryBlock_Unlock(block);
            break;
        }

        rc = NEXUS_MemoryBlock_P_Lock(newBlock, &toPtr);
        if (rc) {
            NEXUS_MemoryBlock_Free(newBlock);
            NEXUS_MemoryBlock_P_Unlock(block, fromPtr);
            return BERR_TRACE(rc);
        }

        BKNI_Memcpy(toPtr, fromPtr, block->size);
        BDBG_MSG(("moved %u bytes from heap[%u] %#x to heap[%u] %#x", block->size, fromHeap->index, (unsigned)block->offset, toHeap->index, (unsigned)newBlock->offset));

        NEXUS_MemoryBlock_P_Unlock(newBlock, toPtr);
        NEXUS_MemoryBlock_P_Unlock(block, fromPtr);
        {
            /* swap so when we free the new NEXUS_MemoryBlock, it frees the old BMMA_Block */
            BMMA_Block_Handle temp = block->block;
            block->block = newBlock->block;
            newBlock->block = temp;
        }
        NEXUS_MemoryBlock_Free_driver(newBlock);
        block->heap = toHeap;
        BDBG_ASSERT(!block->lockCnt);
        /* now we have the old NEXUS_MemoryBlock with a new BMMA_Block in the toHeap, still unlocked.
        no change in local state required because the moved block was unlocked. */

        maxNumber--;
        (*pNumMoved)++;
    }
    return rc;
}

void NEXUS_Heap_GetRuntimeSettings_priv( NEXUS_HeapHandle heap, NEXUS_HeapRuntimeSettings *pSettings )
{
    *pSettings = heap->runtimeSettings;
}

NEXUS_Error NEXUS_Heap_SetRuntimeSettings_priv( NEXUS_HeapHandle heap, const NEXUS_HeapRuntimeSettings *pSettings )
{
    NEXUS_Error rc;
    BMRC_MonitorRegion_Settings regionSettings;
    static const BMRC_Monitor_HwBlock secureClients[] = {
        BMRC_Monitor_HwBlock_eAUD,
        BMRC_Monitor_HwBlock_eAVD,
        BMRC_Monitor_HwBlock_eBSP,
        BMRC_Monitor_HwBlock_eBVN,
        BMRC_Monitor_HwBlock_eMEMC,
        BMRC_Monitor_HwBlock_eM2MC,
        BMRC_Monitor_HwBlock_e3D,
        BMRC_Monitor_HwBlock_ePREFETCH,
        BMRC_Monitor_HwBlock_eSCPU,
        BMRC_Monitor_HwBlock_eVICE,
        BMRC_Monitor_HwBlock_eXPT
    };

    if (nexus_p_custom_arc()) {
        return NEXUS_SUCCESS;
    }

    if (!g_NexusCore.publicHandles.memc[heap->settings.memcIndex].rmm) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (pSettings->secure == (heap->secureMonitorRegion != NULL)) {
        return NEXUS_SUCCESS;
    }
    BMRC_MonitorRegion_GetDefaultSettings(&regionSettings);
    regionSettings.blockWrite = false;
    regionSettings.blockRead = false;

    regionSettings.listType = BMRC_Monitor_ListType_eOtherClients;
    regionSettings.addr = heap->settings.offset;
    regionSettings.length = heap->settings.length;
    if (pSettings->secure) {
        rc = BMRC_MonitorRegion_Add(g_NexusCore.publicHandles.memc[heap->settings.memcIndex].rmm, &heap->secureMonitorRegion, &regionSettings, secureClients, sizeof(secureClients)/sizeof(secureClients[0]));
        if(rc!=BERR_SUCCESS) return BERR_TRACE(rc);
        if (heap->settings.memoryType & NEXUS_MEMORY_TYPE_SECURE) {
           heap->settings.memoryType &= ~NEXUS_MEMORY_TYPE_SECURE_OFF;
        }
    }
    else {
        BMRC_MonitorRegion_Remove(g_NexusCore.publicHandles.memc[heap->settings.memcIndex].rmm, heap->secureMonitorRegion);
        heap->secureMonitorRegion = NULL;
        if (heap->settings.memoryType & NEXUS_MEMORY_TYPE_SECURE) {
           heap->settings.memoryType |= NEXUS_MEMORY_TYPE_SECURE_OFF;
        }
    }

    heap->runtimeSettings = *pSettings;
    return NEXUS_SUCCESS;
}

NEXUS_HeapHandle NEXUS_Heap_Lookup(NEXUS_HeapLookupType lookupType)
{
    unsigned i, heapType;
    switch (lookupType) {
    case NEXUS_HeapLookupType_eMain: heapType = NEXUS_HEAP_TYPE_MAIN; break;
    case NEXUS_HeapLookupType_eCompressedRegion: heapType = NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION; break;
    case NEXUS_HeapLookupType_eExportRegion: heapType = NEXUS_HEAP_TYPE_EXPORT_REGION; break;
    default: return NULL;
    }
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (g_pCoreHandles->heap[i].nexus && g_pCoreHandles->heap[i].nexus->settings.heapType == heapType) {
            return g_pCoreHandles->heap[i].nexus;
        }
    }
    return NULL;
}
