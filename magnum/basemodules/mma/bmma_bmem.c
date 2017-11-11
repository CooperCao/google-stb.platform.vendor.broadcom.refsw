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
#include "bstd.h"
#include "bmma_bmem.h"
#include "bmma_alloc_bmem.h"
#include "bmma_system.h"
#include "bkni.h"
#include "blst_slist.h"

BDBG_MODULE(BMMA_BMem);

BDBG_OBJECT_ID(BMEM_Heap);

struct BMEM_P_Heap {
    BDBG_OBJECT(BMEM_Heap)
    BMMA_Heap_Handle heap;
    BMEM_ModuleHandle parent;
    BLST_S_ENTRY(BMEM_P_Heap) link;
    BMMA_Bmem_Settings settings;
    bool standalone;
    BMMA_Handle  allocator; /* not NULL if standalone is set */
    BMEM_MonitorInterface monitorInterface;
};

BDBG_OBJECT_ID(BMEM_Mem);
struct BMEM_P_Mem {
    BDBG_OBJECT(BMEM_Mem)
    BLST_S_HEAD(BMEP_P_Heap_List, BMEM_P_Heap) heaps;
};

void BMEM_GetDefaultSettings (BMEM_Settings *pDefSettings)
{
    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
}

BERR_Code BMEM_Open ( BMEM_ModuleHandle   *phMem, const BMEM_Settings *pDefSettings)
{
    BMEM_ModuleHandle  m;
    BERR_Code rc;

    BSTD_UNUSED(pDefSettings);
    m = BKNI_Malloc(sizeof(*m));
    if(m==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}

    BDBG_OBJECT_INIT(m, BMEM_Mem);
    BLST_S_INIT(&m->heaps);
    *phMem = m;
    return BERR_SUCCESS;

err_alloc:
    return rc;
}

static void BMMA_Bmem_P_Detach(BMEM_Heap_Handle h, BMEM_ModuleHandle m)
{
    BLST_S_REMOVE(&m->heaps, h, BMEM_P_Heap, link);
    BDBG_OBJECT_DESTROY(h, BMEM_Heap);
    BKNI_Free(h);
    return;
}

void BMEM_Close(BMEM_ModuleHandle m)
{
    BMEM_Heap_Handle h;

    BDBG_OBJECT_ASSERT(m, BMEM_Mem);
    while(NULL!=(h=BLST_S_FIRST(&m->heaps))) {
        BDBG_ASSERT(h->parent == m);
        BMMA_Bmem_P_Detach(h, m);
    }
    BDBG_OBJECT_DESTROY(m, BMEM_Mem);
    BKNI_Free(m);
    return;
}


void BMMA_Bmem_GetDefaultSettings(BMMA_Bmem_Settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    return;
}

static void BMEM_Heap_P_Init(BMEM_ModuleHandle m, BMEM_Heap_Handle h)
{
    BLST_S_INSERT_HEAD(&m->heaps, h, link);
    h->monitorInterface.alloc = NULL;
    h->monitorInterface.free = NULL;
    return;
}


BERR_Code BMMA_Bmem_Attach(BMMA_Heap_Handle heap, BMEM_ModuleHandle parent, BMEM_Heap_Handle *bmemHeap, const BMMA_Bmem_Settings *settings)
{
    BERR_Code rc;
    BMEM_Heap_Handle h;

    BDBG_ASSERT(heap);
    BDBG_ASSERT(bmemHeap);
    BDBG_ASSERT(settings);
    BDBG_MSG(("BMEM_Heap_Attach:>%p %p", (void *)heap, (void *)parent));

    if(settings->flush_cache == NULL) {rc=BERR_TRACE(BERR_NOT_SUPPORTED);goto err_cfg;}
    if(!settings->dummy) {
        if(settings->uncached == NULL || settings->cached == NULL) {rc=BERR_TRACE(BERR_NOT_SUPPORTED);goto err_cfg;}
    }

    h = BKNI_Malloc(sizeof(*h));
    if(h==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(h, BMEM_Heap);
    h->heap = heap;
    h->parent = parent;
    h->standalone = false;
    h->allocator = NULL;
    h->settings = *settings;
    *bmemHeap = h;
    BMEM_Heap_P_Init(parent, h);
    BDBG_MSG(("BMEM_Heap_Attach:<%p %p -> %p", (void *)heap, (void *)parent, (void *)h));
    return BERR_SUCCESS;

err_alloc:
err_cfg:
    return rc;
}


void BMMA_Bmem_Detach(BMEM_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_OBJECT_ASSERT(h->parent, BMEM_Mem);
    BDBG_ASSERT(!h->standalone);
    BMMA_Bmem_P_Detach(h, h->parent);
    return;
}

#if BDBG_DEBUG_BUILD
void *BMEM_P_Heap_TagAllocAligned(BMEM_Heap_Handle h, size_t ulSize, unsigned int  uiAlignment, unsigned int  Boundary,  const char*  pchFile, int iLine)
#else
#define BMMA_Alloc_tagged(heap, ulSize, uiAlignment, settings, pchFile, iLine)  BMMA_Alloc_(heap, ulSize, uiAlignment, settings)
#define BMMA_Free_tagged(block, pchFile, iLine) BMMA_Free(block)
#define BMMA_LockOffset_tagged(block, pchFile, iLine) BMMA_LockOffset(block)
#define BMMA_Lock_tagged(block, pchFile, iLine) BMMA_Lock(block)
#define BMMA_UnlockOffset_tagged(block, offset, pchFile, iLine) BMMA_UnlockOffset(block, offset)
#define BMMA_Unlock_tagged(block, addrCached, pchFile, iLine) BMMA_Unlock(block,addrCached)
void *BMEM_Heap_AllocAligned(BMEM_Heap_Handle h, size_t ulSize, unsigned int  uiAlignment, unsigned int  Boundary)
#endif
{
    BMMA_Block_Handle block;
    void *addrCached;
    BMMA_DeviceOffset offset;
    void *addr;

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);

    BDBG_MSG(("BMEM_Heap_Alloc:%p %u", (void *)h, (unsigned)ulSize));
    if(h->settings.dummy) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Heap:%p(%p) can't be used for allocation using BMEM API", (void *)h, (void *)h->heap));
        BDBG_ASSERT(0);
        goto err_alloc;
    }
    if(Boundary!=0) {
        BDBG_WRN(("Boundary is deprecated"));
    }

#if BDBG_DEBUG_BUILD
    block = BMMA_Alloc_tagged(h->heap, ulSize, 1<<uiAlignment, NULL, pchFile, iLine);
#else
    block = BMMA_Alloc(h->heap, ulSize, 1<<uiAlignment, NULL);
#endif
    if(block==NULL) {(void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto err_alloc;}
    offset = BMMA_LockOffset_tagged(block, pchFile, iLine);
    if(offset < h->settings.base || offset + ulSize > h->settings.base + h->settings.length) { (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto err_offset_out_of_range;}
    if(Boundary) {
        if( (offset & 1<<Boundary) != ((offset + ulSize) & 1<<Boundary) ) {
            BDBG_ERR(("%p:Boundary request wasn't satisfied: %#x %#x (%#x)", (void *)h, (unsigned)offset, (unsigned)(offset+ulSize), (unsigned)1<<Boundary));
            goto err_boundary;
        }
    }
    addrCached = BMMA_Lock_tagged(block, pchFile, iLine);
    if(addrCached==NULL) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_lock;}
    if((uint8_t *)addrCached < (uint8_t *)h->settings.cached || (uint8_t *)addrCached + ulSize > (uint8_t *)h->settings.cached + h->settings.length) { (void)BERR_TRACE(BERR_NOT_SUPPORTED);goto err_addr_out_of_range;}

    BMMA_Alloc_SetTaint(block);

    addr = (uint8_t *)h->settings.uncached + ((uint8_t *)addrCached - (uint8_t *)h->settings.cached);
    BDBG_MSG(("BMEM_Heap_Alloc:%p %u->%p", (void *)h, (unsigned)ulSize, addr));
    return addr;

err_addr_out_of_range:
    BMMA_Unlock_tagged(block, addrCached, pchFile, iLine);
err_lock:
err_boundary:
err_offset_out_of_range:
    BMMA_UnlockOffset_tagged(block, offset, pchFile, iLine);
    BMMA_Free_tagged(block, pchFile, iLine);
err_alloc:
/* err_cfg: */
    return NULL;
}

static BERR_Code BMEM_Heap_P_Free(BMEM_Heap_Handle  h, void  *addr)
{
    BMMA_Block_Handle b;

    b = BMMA_Alloc_GetTaintByAddress(h->heap, addr);
    if(b==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BMMA_Free(b);
    return BERR_SUCCESS;
}

BERR_Code BMEM_Heap_FreeCached(BMEM_Heap_Handle  h, void *CachedAddress)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);

    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if((uint8_t *)CachedAddress< (uint8_t *)h->settings.cached || (uint8_t *)CachedAddress >= (uint8_t *)h->settings.cached + h->settings.length) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BMEM_Heap_P_Free(h, CachedAddress);
}

BERR_Code BMEM_Heap_Free(BMEM_Heap_Handle  h, void  *Address)
{
    void *addr;

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);

    BDBG_MSG(("BMEM_Heap_Free:%p %p", (void *)h, Address));

    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if((uint8_t *)Address < (uint8_t *)h->settings.uncached || (uint8_t *)Address >= (uint8_t *)h->settings.uncached + h->settings.length) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    addr = (uint8_t *)h->settings.cached + ((uint8_t *)Address - (uint8_t *)h->settings.uncached);
    return BMEM_Heap_P_Free(h, addr);
}

static BERR_Code BMEM_Heap_P_ConvertOffsetToAddress_isrsafe(BMEM_Heap_Handle  h, uint32_t ulOffset, void  **ppvAddress)
{
    if(ulOffset >=  h->settings.base && ulOffset < h->settings.base + h->settings.length) {
        *ppvAddress = (uint8_t *)h->settings.uncached  + (ulOffset - h->settings.base);
        return BERR_SUCCESS;
    }
    return BERR_INVALID_PARAMETER;
}

BERR_Code BMEM_Heap_ConvertOffsetToAddress(BMEM_Heap_Handle  h, uint32_t ulOffset, void  **ppvAddress)
{
    BERR_Code rc;
    BMEM_ModuleHandle m;
    BMEM_Heap_Handle  heap;

    BDBG_MSG(("BMEM_Heap_ConvertOffsetToAddress:%p %u", (void *)h, ulOffset));
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    rc = BMEM_Heap_P_ConvertOffsetToAddress_isrsafe(h, ulOffset, ppvAddress);
    if(rc==BERR_SUCCESS) {return rc;}
    m = h->parent;
    if(m) {
        BDBG_OBJECT_ASSERT(m, BMEM_Mem);
        for(heap=BLST_S_FIRST(&m->heaps);heap;heap=BLST_S_NEXT(heap,link)) {
            if(heap!=h) {
                rc = BMEM_Heap_P_ConvertOffsetToAddress_isrsafe(heap, ulOffset, ppvAddress);
                if(rc==BERR_SUCCESS) {return rc;}
            }
        }
    }
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code BMEM_Heap_P_ConvertAddressToOffset_isrsafe( BMEM_Heap_Handle  h, void *pvAddress, uint32_t  *pulOffset)
{
    BDBG_MSG(("BMEM_Heap_P_ConvertAddressToOffset:%p %p (%p..%p)", (void *)h, pvAddress, h->settings.uncached, (void *)((uint8_t *)h->settings.uncached + h->settings.length)));
    if(h->settings.uncached && (uint8_t *)pvAddress >=  (uint8_t *)h->settings.uncached && (uint8_t *)pvAddress < (uint8_t *)h->settings.uncached + h->settings.length) {
        *pulOffset = h->settings.base + ((uint8_t *)pvAddress - (uint8_t *)h->settings.uncached);
        BDBG_MSG(("BMEM_Heap_P_ConvertAddressToCached:%p uncached %p->%u", (void *)h, pvAddress, (unsigned)*pulOffset));
        return BERR_SUCCESS;
    } else if(h->settings.cached && (uint8_t *)pvAddress >=  (uint8_t *)h->settings.cached && (uint8_t *)pvAddress < (uint8_t *)h->settings.cached + h->settings.length) {
        *pulOffset = h->settings.base + ((uint8_t *)pvAddress - (uint8_t *)h->settings.cached);
        BDBG_MSG(("BMEM_Heap_P_ConvertAddressToCached:%p cached %p->%u", (void *)h, pvAddress, (unsigned)*pulOffset));
        return BERR_SUCCESS;
    }
    return BERR_INVALID_PARAMETER;
}

BERR_Code BMEM_Heap_ConvertAddressToOffset_isrsafe( BMEM_Heap_Handle  h, void *pvAddress, uint32_t  *pulOffset)
{
    BERR_Code rc;
    BMEM_ModuleHandle m;
    BMEM_Heap_Handle  heap;

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_MSG(("BMEM_Heap_ConvertAddressToOffset_isrsafe:%p %p", (void *)h, pvAddress));
    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    rc = BMEM_Heap_P_ConvertAddressToOffset_isrsafe(h, pvAddress, pulOffset);
    if(rc==BERR_SUCCESS) {return rc;}
    m = h->parent;
    if(m) {
        BDBG_OBJECT_ASSERT(m, BMEM_Mem);
        for(heap=BLST_S_FIRST(&m->heaps);heap;heap=BLST_S_NEXT(heap,link)) {
            if(heap!=h) {
                rc = BMEM_Heap_P_ConvertAddressToOffset_isrsafe(heap, pvAddress, pulOffset);
                if(rc==BERR_SUCCESS) {return rc;}
            }
        }
    }
    BDBG_ERR(("BMEM_Heap_ConvertAddressToOffset_isrsafe:%p %p", (void *)h, pvAddress));
#if 0
    for(heap=BLST_S_FIRST(&m->heaps);heap;heap=BLST_S_NEXT(heap,link)) {
        BDBG_LOG(("BMEM_Heap_ConvertAddressToOffset:%p %p (%p..%p)", heap, pvAddress, heap->settings.uncached, (uint8_t *)heap->settings.uncached + heap->settings.length));
    }
    BDBG_ASSERT(0);
#endif
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code BMEM_Heap_P_ConvertAddressToCached_isrsafe( BMEM_Heap_Handle  h, void *pvAddress,void **ppvCachedAddress)
{
    BDBG_MSG(("BMEM_Heap_P_ConvertAddressToCached:%p %p (%p..%p)", (void *)h, pvAddress, h->settings.uncached, (uint8_t *)h->settings.uncached + h->settings.length));
    if((uint8_t *)pvAddress >=  (uint8_t *)h->settings.uncached && (uint8_t *)pvAddress < (uint8_t *)h->settings.uncached + h->settings.length) {
        *ppvCachedAddress = (uint8_t *)h->settings.cached + ((uint8_t *)pvAddress - (uint8_t *)h->settings.uncached);
        BDBG_MSG(("BMEM_Heap_P_ConvertAddressToCached:%p %p->%p", (void *)h, pvAddress, *ppvCachedAddress));
        return BERR_SUCCESS;
    }
    return BERR_INVALID_PARAMETER;
}

BERR_Code BMEM_Heap_ConvertAddressToCached_isrsafe( BMEM_Heap_Handle  h, void *pvAddress,void **ppvCachedAddress)
{
    BERR_Code rc;
    BMEM_ModuleHandle m;
    BMEM_Heap_Handle  heap;

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("BMEM_Heap_ConvertAddressToCached:%p %p", (void *)h, pvAddress));
    rc = BMEM_Heap_P_ConvertAddressToCached_isrsafe(h, pvAddress, ppvCachedAddress);
    if(rc==BERR_SUCCESS) {return rc;}
    m = h->parent;
    if(m) {
        BDBG_OBJECT_ASSERT(m, BMEM_Mem);
        for(heap=BLST_S_FIRST(&m->heaps);heap;heap=BLST_S_NEXT(heap,link)) {
            if(heap!=h) {
                rc = BMEM_Heap_P_ConvertAddressToCached_isrsafe(heap, pvAddress, ppvCachedAddress);
                if(rc==BERR_SUCCESS) {return rc;}
            }
        }
    }
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code BMEM_Heap_FlushCache_isrsafe(BMEM_Heap_Handle  h, void *pvCachedAddress, size_t size)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    if(h->settings.dummy) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    h->settings.flush_cache(pvCachedAddress, size);
    return BERR_SUCCESS;
}

BERR_Code BMEM_Heap_FlushCache(BMEM_Heap_Handle  h, void *pvCachedAddress, size_t size)
{
    return BMEM_Heap_FlushCache_isrsafe(h, pvCachedAddress, size);
}

BERR_Code BMEM_Heap_FlushCache_isr(BMEM_Heap_Handle  h, void *pvCachedAddress, size_t  size)
{
    return BMEM_Heap_FlushCache_isrsafe(h, pvCachedAddress, size);
}

static void *BMMA_Bmem_P_StandaloneMmap(void *context, void *state, BMMA_DeviceOffset base, size_t length)
{
    BMEM_Heap_Handle h = context;
    void *ptr;

    BSTD_UNUSED(state);

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_ASSERT(h->standalone);
    BDBG_ASSERT(base >= h->settings.base);
    ptr = (uint8_t *)h->settings.cached + (base - h->settings.base);
    h->settings.flush_cache(ptr, length);
    return ptr;
}

static void BMMA_Bmem_P_StandaloneUnmap(void *context, void *state, void *ptr, size_t length)
{
    BMEM_Heap_Handle h = context;
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BSTD_UNUSED(state);

    h->settings.flush_cache(ptr, length);
    return;
}

static void BMMA_Bmem_P_StandaloneFlushCache(const void *addr, size_t size)
{
    BSTD_UNUSED(addr);
    BSTD_UNUSED(size);
    return;
}

BERR_Code BMEM_Heap_GetDefaultSettings ( BMEM_Heap_Settings *pHeapSettings)
{
    BDBG_ASSERT(pHeapSettings);
    BKNI_Memset(pHeapSettings, 0, sizeof(*pHeapSettings));
    pHeapSettings->uiAlignment = sizeof(void *);
    pHeapSettings->flush = BMMA_Bmem_P_StandaloneFlushCache;
    pHeapSettings->flush_isr = BMMA_Bmem_P_StandaloneFlushCache;
    return BERR_SUCCESS;
}

static BERR_Code BMMA_Bmem_P_TraceAlloc(void *cnxt, BMMA_DeviceOffset base, size_t size, const char *fname, unsigned line)
{
    BMEM_Heap_Handle h = cnxt;
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    if(h->monitorInterface.alloc) {
        h->monitorInterface.alloc(h->monitorInterface.cnxt, base, size, fname, line);
    }
    return BERR_SUCCESS;
}

static void BMMA_Bmem_P_TraceFree(void *cnxt, BMMA_DeviceOffset base, size_t size)
{
    BMEM_Heap_Handle h = cnxt;
    BSTD_UNUSED(size);

    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    if(h->monitorInterface.free) {
        h->monitorInterface.free(h->monitorInterface.cnxt, base);
    }
    return;
}

BERR_Code BMEM_Heap_Create(BMEM_ModuleHandle  hMem, void *pvAddress, uint32_t ulOffset, size_t zSize, BMEM_Heap_Settings *pHeapSettings, BMEM_Heap_Handle   *phHeap)
{
    BERR_Code rc;
    BMEM_Heap_Handle h;
    BMMA_CreateSettings allocatorSettings;
    BMMA_Heap_CreateSettings heapSettings;

    BDBG_OBJECT_ASSERT(hMem, BMEM_Mem);
    BDBG_ASSERT(pHeapSettings);
    BDBG_ASSERT(phHeap);
    BDBG_MSG(("BMEM_Heap_Create:>%p %p:%#x %u", (void *)hMem, pvAddress, (unsigned)ulOffset, (unsigned)zSize));
    if(pHeapSettings->pCachedAddress==NULL && pHeapSettings->flush != BMMA_Bmem_P_StandaloneFlushCache) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    h = BKNI_Malloc(sizeof(*h));
    if(h==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(h, BMEM_Heap);

    BMMA_Bmem_GetDefaultSettings(&h->settings);
    h->standalone = true;
    h->parent = hMem;
    h->settings.base = ulOffset;
    h->settings.length = zSize;
    h->settings.uncached = pvAddress;
    h->settings.cached = pHeapSettings->pCachedAddress ? pHeapSettings->pCachedAddress : pvAddress;
    h->settings.flush_cache = pHeapSettings->flush;

    BMMA_GetDefaultCreateSettings(&allocatorSettings);
    rc = BMMA_Create(&h->allocator, &allocatorSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_allocator;}

    BMMA_Heap_GetDefaultCreateSettings(&heapSettings);
    heapSettings.base = ulOffset;
    heapSettings.size = zSize;
    heapSettings.minAlignment = 1<<pHeapSettings->uiAlignment;
    heapSettings.context = h;
    heapSettings.mmap = BMMA_Bmem_P_StandaloneMmap;
    heapSettings.unmap = BMMA_Bmem_P_StandaloneUnmap;
    heapSettings.alloc = BMMA_Bmem_P_TraceAlloc;
    heapSettings.free = BMMA_Bmem_P_TraceFree;
    rc = BMMA_Heap_Create(&h->heap, h->allocator, &heapSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_heap;}
    BMEM_Heap_P_Init(hMem, h);

    *phHeap = h;
    BDBG_MSG(("BMEM_Heap_Create:<%p %p:%#x %u ->%p", (void *)hMem, pvAddress, (unsigned)ulOffset, (unsigned)zSize, (void *)h));
    return BERR_SUCCESS;

err_heap:
    BMMA_Destroy(h->allocator);
err_allocator:
    BKNI_Free(h);
err_alloc:
    return rc;
}

void BMEM_Heap_Destroy(BMEM_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_MSG(("BMEM_Heap_Destroy:<%p", (void *)h));
    BDBG_ASSERT(h->standalone);
    BMMA_Heap_Destroy(h->heap);
    BMMA_Destroy(h->allocator);
    BDBG_OBJECT_ASSERT(h->parent, BMEM_Mem);
    BMMA_Bmem_P_Detach(h, h->parent);
    return;
}

#if 1
/* STUBS BEGIN*/
void BMEM_Heap_ResetHighWatermark( BMEM_Heap_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    return;
}

void BMEM_Heap_GetInfo (BMEM_Heap_Handle h, BMEM_HeapInfo *phi)
{
    BERR_Code rc;
    BMMA_Heap_FastStatus fastStatus;
    BMMA_Heap_SlowStatus slowStatus;
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_ASSERT(phi);
    BKNI_Memset(phi, 0, sizeof(*phi));
    if(h->settings.dummy) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ASSERT(0);
        return;
    }

    phi->pvAddress = h->settings.uncached;
    phi->ulOffset = h->settings.base;
    phi->zSize = h->settings.length;
    rc = BMMA_Heap_GetStatus(h->heap, &fastStatus, &slowStatus);
    if(rc==BERR_SUCCESS) {
        unsigned i;
        for(i=1;i<32;i++) {
            if(1u<<i>=fastStatus.alignment) {
                break;
            }
        }
        phi->uiAlignment = i;
        phi->ulLargestFree = fastStatus.largestFree;
        phi->ulSmallestFree = slowStatus.smallestFree;
        phi->ulTotalFree = fastStatus.totalFree;
        phi->ulNumFree = fastStatus.numFree;
        phi->ulLargestAllocated = slowStatus.largestAllocated;
        phi->ulSmallestAllocated = slowStatus.smallestAllocated;
        phi->ulTotalAllocated = fastStatus.totalAllocated;
        phi->ulHighWatermark = fastStatus.highWatermark;
        phi->ulNumAllocated = fastStatus.numAllocated;
        phi->ulNumErrors = 0;
    }
    return;
}

BERR_Code BMEM_Heap_InstallMonitor(BMEM_Heap_Handle h, BMEM_MonitorInterface *monitor)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    BDBG_ASSERT(monitor);

    if(!h->standalone) {return BERR_TRACE(BERR_NOT_SUPPORTED);}

    h->monitorInterface = *monitor;

    return BERR_SUCCESS;
}

void BMEM_Dbg_DumpHeap(BMEM_Handle h)
{
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    return;
}

BERR_Code BMEM_Heap_Validate(BMEM_Heap_Handle h)
{
    BDBG_MSG(("BMEM_Validate: %p", (void *)h));
    BDBG_OBJECT_ASSERT(h, BMEM_Heap);
    return BERR_SUCCESS;
}

/* STUBS END */
#endif
