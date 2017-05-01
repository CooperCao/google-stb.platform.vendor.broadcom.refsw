/***************************************************************************
*  Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
***************************************************************************/

static unsigned NEXUS_Platform_P_GetAllignedMbytes(uint64_t n)
{
    return (n+ 1024*1024 - 1)/(1024*1024);
}

static void NEXUS_Platform_P_PrintBmemBootOptions(const NEXUS_PlatformOsRegion *regions, const char *title, const char *extra, char *buf, size_t buf_len)
{
    unsigned i;
    unsigned buf_off;
    bool space=false;
    BDBG_ASSERT(buf_len>0);
    buf[0] = '\0';
    for (buf_off=0,i=0;i<NEXUS_MAX_HEAPS;i++) {
        int rc;
        unsigned left = buf_len-buf_off;
        if(regions[i].length==0) {
            continue;
        }
        rc = BKNI_Snprintf(buf+buf_off,left,"%s%s=%um@%um", space?" ":"",regions[i].cma?"brcm_cma":"bmem", NEXUS_Platform_P_GetAllignedMbytes(regions[i].length), NEXUS_Platform_P_GetAllignedMbytes(regions[i].base));
        if(rc<0) {
            continue;
        }
        if(rc>(int)left) {
            buf_off = buf_len;
        } else {
            buf_off += rc;
        }
        space = true;
    }
    BDBG_LOG(("%s Linux boot parameters: '%s%s'", title, extra, buf));
    return;
}


static bool NEXUS_Platform_P_RangeTestFit(const nexus_p_memory_range *outer, NEXUS_Addr addr, size_t size)
{
    return (outer->addr <= addr) && ((outer->addr + outer->size) >= (addr + size));
}

static int NEXUS_Platform_P_GetMemcForRange(const nexus_p_memory_info *info, NEXUS_Addr addr, size_t size)
{
    unsigned i;

    for(i=0;i<sizeof(info->memc)/sizeof(info->memc[0]);i++) {
        unsigned j;
        for(j=0;j<sizeof(info->memc[0].range)/sizeof(info->memc[0].range[0]);j++) {
            if(j>=(unsigned)info->memc[i].count) {
                break;
            }
            if(NEXUS_Platform_P_RangeTestFit(&info->memc[i].range[j], addr, size)) {
                return i;
            }
        }
    }
    return -1;
}

static NEXUS_Addr NEXUS_P_AddrAlign(NEXUS_Addr v, size_t alignment)
{
    NEXUS_Addr r;
    r = v + (alignment - 1);
    r -= r%alignment;
    return r;
}


static void NEXUS_Platform_P_ShiftUpRegions(NEXUS_PlatformOsRegion *osRegion, unsigned position, unsigned last_region)
{
    int i;
    for(i=(int)last_region-1;i>=(int)position;i--) {
        osRegion[i+1] = osRegion[i];
    }
    return;
}

static unsigned NEXUS_Platform_P_PrepareSortedInsert(NEXUS_PlatformOsRegion *osRegion, unsigned last_region, const nexus_p_memory_range *range)
{
    unsigned i;
    for(i=0;i<last_region;i++) {
        if(osRegion[i].base > range->addr) {
            NEXUS_Platform_P_ShiftUpRegions(osRegion, i, last_region);
            return i;
        }
    }
    return last_region;

}

static void NEXUS_Platform_P_PrintRegions(const NEXUS_PlatformOsRegion *osRegion, unsigned n_regions)
{
    unsigned i;
    for(i=0;i<n_regions;i++) {
        BDBG_MSG(("%u: BMEM MEMC%u.%u %uMBytes(%u) at " BDBG_UINT64_FMT, i, osRegion[i].memcIndex, osRegion[i].subIndex, (unsigned)(osRegion[i].length/(1024*1024)), (unsigned)osRegion[i].length, BDBG_UINT64_ARG(osRegion[i].base) ));
    }
    return;
}

static bool NEXUS_Platform_P_IsOverlap(const NEXUS_PlatformOsRegion *osRegion, const nexus_p_memory_range *range)
{
    return ((range->addr+range->size) >= osRegion->base && (osRegion->base + osRegion->length) >= range->addr);
}

static NEXUS_Error NEXUS_Platform_P_SetHostMemoryFromInfo(const nexus_p_memory_info *info, NEXUS_PlatformOsRegion *osRegion, unsigned n_regions, const NEXUS_PlatformHeapSettings *heap, unsigned alignment)
{
    const NEXUS_Addr low_limit = 16 * 1024 * 1024; /* exclude first 16 MBytes */
    unsigned last_region;
    unsigned i,j;

    last_region = 0;
    for(i=0;i<sizeof(info->memc)/sizeof(info->memc[0]);i++) {
        for(j=0;j<sizeof(info->memc[0].range)/sizeof(info->memc[0].range[0]);j++) {
            unsigned insert;
            nexus_p_memory_range range = info->memc[i].range[j];
            if(j>=(unsigned)info->memc[i].count) {
                break;
            }
            if(range.addr < low_limit) {
                if(range.addr + range.size <= low_limit) {
                    continue;
                }
                range.size -= low_limit - range.addr;
                range.addr = low_limit;
            }
            if(last_region>=n_regions) {
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            insert = NEXUS_Platform_P_PrepareSortedInsert(osRegion, last_region, &range);
            last_region++;
            osRegion[insert].base = range.addr;
            osRegion[insert].length = range.size;
            osRegion[insert].memcIndex = i;
            osRegion[insert].subIndex = j;
            osRegion[insert].cma = false;
        }
    }
    NEXUS_Platform_P_PrintRegions(osRegion, last_region);
    for(i=0;i<sizeof(info->reserved.range)/sizeof(info->reserved.range[0]);i++) {
        if((int)i>=info->reserved.count) {
            break;
        }
        for(j=0;j<last_region;j++) {
            if(NEXUS_Platform_P_IsOverlap(&osRegion[j], &info->reserved.range[i])) {
                if(info->reserved.range[i].addr <= osRegion[j].base) {
                    if( (info->reserved.range[i].addr + info->reserved.range[i].size) >= (osRegion[j].base + osRegion[j].length)) {
                        osRegion[j].length = 0;  /* remove region */
                    } else {
                        BDBG_ASSERT((info->reserved.range[i].addr + info->reserved.range[i].size) >= osRegion[j].base);
                        osRegion[j].length -= (info->reserved.range[i].addr + info->reserved.range[i].size) - osRegion[j].base; /* adjust size */
                        osRegion[j].base = (info->reserved.range[i].addr + info->reserved.range[i].size); /* and start */
                    }
                } else if((info->reserved.range[i].addr + info->reserved.range[i].size) >= (osRegion[j].base + osRegion[j].length)) {
                    BDBG_ASSERT(osRegion[j].base + osRegion[j].length >= info->reserved.range[i].addr);
                    osRegion[j].length -= (osRegion[j].base + osRegion[j].length) - info->reserved.range[i].addr; /* adjust size */
                } else {
                    size_t new_size = info->reserved.range[i].addr - osRegion[j].base;
                    /* reserved region in middle - would have to spilt osRegion in two */
                    if( new_size >= alignment) {
                        if(last_region>=n_regions) {
                            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                        }
                        NEXUS_Platform_P_ShiftUpRegions(osRegion, j, last_region);
                        last_region++;
                        osRegion[j].length = new_size; /* adjust size */
                        j++;
                    }
                    osRegion[j].length = (osRegion[j].base + osRegion[j].length) - (info->reserved.range[i].addr + info->reserved.range[i].size); /* adjust size */
                    osRegion[j].base = info->reserved.range[i].addr + info->reserved.range[i].size; /* and start */
                }
            }
        }
    }
    for(i=0;i<last_region;i++) {
        BDBG_MSG(("raw MEMC%u.%u %uMBytes(%#x) at " BDBG_UINT64_FMT "", osRegion[i].memcIndex, i, (unsigned)(osRegion[i].length/(1024*1024)), (unsigned)(osRegion[i].length), BDBG_UINT64_ARG(osRegion[i].base)));
    }

    /* filter out all small regions */
    for(;;) {
        bool removed = false;
        for(i=0;i<NEXUS_MAX_MEMC;i++) {
            int maxLength = -1;
            int minLength = -1;

            for(j=0;j<last_region;j++) {
                if(osRegion[j].memcIndex != i) {
                    continue;
                }
                if(osRegion[j].length==0) {
                    continue;
                }
                if(maxLength<0 || osRegion[j].length > osRegion[maxLength].length) {
                    maxLength=j;
                }
                if(minLength<0 || osRegion[j].length < osRegion[minLength].length) {
                    minLength=j;
                }
            }
            if(minLength>=0 && maxLength>=0) {
                if(osRegion[minLength].length < osRegion[maxLength].length / 8 && osRegion[minLength].base < ((uint64_t)1)<<32) {
                    BDBG_MSG(("removing MEMC%u.%u %uMBytes(%#x) at " BDBG_UINT64_FMT "", osRegion[minLength].memcIndex, j, (unsigned)(osRegion[minLength].length/(1024*1024)), (unsigned)(osRegion[minLength].length), BDBG_UINT64_ARG(osRegion[minLength].base)));
                    osRegion[minLength].length = 0; /* remove too small region */
                    removed = true;
                }
            }
        }
        if(!removed) {
            break;
        }
    }
    for(i=0;i<NEXUS_MAX_MEMC;i++) {
        unsigned maxAlignment = alignment;
        for(j=0; j<NEXUS_MAX_HEAPS;j++) {
            if(heap[j].size==0) {
                continue;
            }
            if(heap[j].memcIndex != i) {
                continue;
            }
            if(maxAlignment < heap[j].alignment) {
                maxAlignment = heap[j].alignment;
            }
        }
        for(j=0; j<last_region;j++) { /* align regions on worst heap alignment */
            if(NEXUS_Platform_P_GetMemcForRange(info, osRegion[j].base, osRegion[j].length) == (int)j) {
                NEXUS_Addr new_addr = NEXUS_P_AddrAlign(osRegion[j].base, maxAlignment);
                size_t delta_addr = new_addr - osRegion[j].base;
                size_t length = osRegion[j].length;

                if(delta_addr < length) {
                    length -= delta_addr;
                    length -= length % maxAlignment;
                } else {
                    length = 0;
                }
                osRegion[j].length = length;
                osRegion[j].base = new_addr;
            }
        }
    }
    for(j=i=0;i<last_region;i++) { /* collapse 0 length regions */
        if(osRegion[i].length) {
            if(i!=j) {
                osRegion[j] = osRegion[i];
            }
            j++;
        }
    }
    last_region = j;
    for(i=last_region;i<NEXUS_MAX_HEAPS;i++) { /* clear extra regions */
        osRegion[i].length = 0;
    }
    for(i=0;i<NEXUS_MAX_MEMC;i++) { /* assign subIndex */
        unsigned subIndex=0;
        for(j=0;j<last_region;j++) {
            if(osRegion[j].memcIndex==i) {
                osRegion[j].subIndex = subIndex;
                subIndex++;
            }
        }
    }
    NEXUS_Platform_P_PrintRegions(osRegion, last_region);
    if(last_region>NEXUS_MAX_HEAPS) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_SetHostBmemMemoryFromInfo(const nexus_p_memory_info *info, NEXUS_PlatformOsRegion *osRegion)
{
    unsigned i;
    const unsigned ranges = sizeof(info->bmem.range)/sizeof(info->bmem.range[0]);
    unsigned subIndex[NEXUS_MAX_MEMC];

    for(i=0;i<sizeof(info->memc)/sizeof(info->memc[0]);i++) {
        unsigned j;
        for(j=0;j<sizeof(info->memc[0].range)/sizeof(info->memc[0].range[0]);j++) {
            if(j>=(unsigned)info->memc[i].count) {
                break;
            }
            BDBG_MSG(("MEMC%u.%u %u(MBytes) at " BDBG_UINT64_FMT "", i, j, (unsigned)((info->memc[i].range[j].size)/(1024*1024)), BDBG_UINT64_ARG(info->memc[i].range[j].addr)));
        }
    }

    for(i=0;i<NEXUS_MAX_MEMC;i++) {
        subIndex[i]=0;
    }

    for(i=0;i<ranges;i++) {
        int memc;
        if(info->bmem.range[i].size==0) {
            break;
        }
        memc = NEXUS_Platform_P_GetMemcForRange(info, info->bmem.range[i].addr, info->bmem.range[i].size);
        if(memc<0) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        BDBG_ASSERT(memc<NEXUS_MAX_MEMC);
        if(i>=NEXUS_MAX_HEAPS) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        osRegion[i].base = info->bmem.range[i].addr;
        osRegion[i].length = info->bmem.range[i].size;
        osRegion[i].memcIndex = memc;
        osRegion[i].subIndex = subIndex[memc];
        osRegion[i].cma = false;
        subIndex[memc]++;
        BDBG_MSG(("BMEM MEMC%u.%u %uMBytes at " BDBG_UINT64_FMT, memc, osRegion[i].subIndex, (unsigned)(osRegion[i].length/(1024*1024)), BDBG_UINT64_ARG(osRegion[i].base) ));
        if(i>0) {
            /* handle neighbouring bmem regions, they should be already sorted by address */
            if(osRegion[i].memcIndex == osRegion[i-1].memcIndex && osRegion[i].base == (osRegion[i-1].base + osRegion[i-1].length)) {
                BDBG_WRN(("shrink bmem[%u]" BDBG_UINT64_FMT ":%u neighboring bmem[%u]" BDBG_UINT64_FMT ":%u", i-1, BDBG_UINT64_ARG(osRegion[i-1].base), (unsigned)osRegion[i-1].length, i, BDBG_UINT64_ARG(osRegion[i].base), (unsigned)osRegion[i].length));
                if(osRegion[i-1].length > 4096) {
                    osRegion[i-1].length -= 4096;
                } else {
                    osRegion[i-1].length = 1;
                }
            }
        }
    }
    return NEXUS_SUCCESS;
}

struct NEXUS_P_MemcRegionCandidates { /* structure keeps all regions where heap could be placed */
    unsigned count; /* actual number of heaps on  MEMC */
    unsigned permutations; /* total number of permutations */
    struct {
        unsigned nexusHeapNo; /* nexus heap number */
        unsigned size; /* number of possibilities for the heap */
        unsigned possibleRegions[NEXUS_NUM_MEMC_REGIONS]; /* all regions this heap could use, in order of priorities */
    }heap[NEXUS_MAX_HEAPS];
};

struct NEXUS_P_MemcRegionOsMap {
    int map[NEXUS_MAX_MEMC][NEXUS_NUM_MEMC_REGIONS]; /* map from memc/subIndex to osRegion */
};


struct NEXUS_P_MemcPermutation {
    unsigned subIndex[NEXUS_MAX_HEAPS];
    unsigned offset[NEXUS_MAX_HEAPS];
};

struct NEXUS_Platform_P_AllocatorRange {
    BMMA_RangeAllocator_Region region;
    unsigned heapIndex;
    bool unused;
};

struct NEXUS_Platform_P_AllocatorState {
    struct nexus_cma_map cma_info[NEXUS_CMA_MAX_INDEX];
    struct nexus_linux_cma_memory_info linux_cma_info;
    struct NEXUS_P_MemcRegionOsMap osRegion;
    int subIndex[NEXUS_MAX_HEAPS];
    struct NEXUS_P_MemcRegionCandidates candidates;
    struct NEXUS_P_MemcPermutation permutation;
    bool extraMemory;
    struct {
        const NEXUS_PlatformHeapSettings *heapSettings;
        BMMA_RangeAllocator_Region firstRegion;
        BMMA_RangeAllocator_Block_Handle heaps[NEXUS_MAX_HEAPS];
        bool realAllocation;
        bool hasRegionRestrictedHeaps;
        struct {
            int regions;
            struct NEXUS_Platform_P_AllocatorRange heapRegion[3*NEXUS_MAX_HEAPS];
        } placement;
    } allocator;
    struct {
        struct NEXUS_P_MemcPermutation permutation;
        int metric;
    } best;
    struct {
        nexus_p_memory_info info;
        NEXUS_PlatformOsRegion osMemory[NEXUS_MAX_HEAPS];
        char buf[128];
        char vmalloc_buf[32];
    } bmem_hint;
};

static bool NEXUS_Platform_P_AllocateInRegionOne(const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation, bool inFront)
{
    if(inFront) {
        return BMMA_RangeAllocator_AllocateInRegion_InFront(region, size, settings, allocation);
    } else {
        return BMMA_RangeAllocator_AllocateInRegion_InBack(region, size, settings, allocation);
    }
}

static void NEXUS_Platform_P_IntersectRegion(const BMMA_RangeAllocator_Region *src, BMMA_RangeAllocator_Region *dst, NEXUS_Addr base, uint64_t length)
{
    uint64_t src_end = src->base + src->length;
    uint64_t end = base + length;
    uint64_t dst_end = end <= src_end ? end : src_end;
    dst->base = base >= src->base ? base : src->base;
    if(dst_end > dst->base) {
        dst->length = dst_end - dst->base;
    } else {
        dst->length = 0;
    }
    return;
}

static bool NEXUS_Platform_P_AllocateInRegion(struct NEXUS_Platform_P_AllocatorState *state, bool inFront, const BMMA_RangeAllocator_Region *region, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation)
{
    const uint64_t _4GB = ((uint64_t)1)<<32;
    BMMA_RangeAllocator_Region tmp;

    if( (state->allocator.heapSettings->memoryType & NEXUS_MEMORY_TYPE_HIGH_MEMORY) != NEXUS_MEMORY_TYPE_HIGH_MEMORY) {
        if( (region->base + region->length) > _4GB) {
            return false;
        }
    }
    if(state->allocator.heapSettings->placement.region.valid) {
        NEXUS_Platform_P_IntersectRegion(region, &tmp, state->allocator.heapSettings->placement.region.base, state->allocator.heapSettings->placement.region.length);
        if(tmp.length==0) {
            return false;
        }
        region = &tmp;
    }
    if(state->allocator.heapSettings->placement.sage) {
        bool crosses=false;
        BMMA_RangeAllocator_Region newRegion;
        /* all heaps that SAGE is using must not cross two addresses
         * 256M
         * 1024M
         */
        const unsigned _256M = 256 * 1024 * 1024;
        const unsigned _1024M = 1024 * 1024 * 1024;
        if(region->base < _1024M && region->base + region->length > _1024M) {
            crosses = true;
            if(inFront) {
                newRegion.base = region->base;
                newRegion.length = _1024M - region->base;
                if(newRegion.base < _256M) {
                    newRegion.length = _1024M - _256M;
                    newRegion.base =_256M;
                }
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }
                newRegion.base = _1024M;
                newRegion.length = (region->base  + region->length) - _1024M;
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }
            } else {
                newRegion.base = _1024M;
                newRegion.length = (region->base  + region->length) - _1024M;
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }

                newRegion.base = region->base;
                newRegion.length = _1024M - region->base;
                if(newRegion.base < _256M) {
                    newRegion.length = _1024M - _256M;
                    newRegion.base =_256M;
                }
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }
            }
        }
        if(region->base < _256M && region->base + region->length > _256M) {
            crosses = true;
            if(inFront) {
                newRegion.base = region->base;
                newRegion.length = _256M - newRegion.base;
                if(BMMA_RangeAllocator_AllocateInRegion_InBack(&newRegion, size, settings, allocation)) {
                    return true;
                }
                newRegion.base = _256M;
                newRegion.length = (region->base + region->length) - _256M;
                if(newRegion.base + newRegion.length > _1024M) {
                    newRegion.length = _1024M - _256M;
                }
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }
            } else {
                newRegion.base = _256M;
                newRegion.length = (region->base + region->length) - _256M;
                if(newRegion.base + newRegion.length > _1024M) {
                    newRegion.length = _1024M - _256M;
                }
                if(NEXUS_Platform_P_AllocateInRegionOne(&newRegion, size, settings, allocation, inFront)) {
                    return true;
                }
                newRegion.base = region->base;
                newRegion.length = _256M - newRegion.base;
                if(BMMA_RangeAllocator_AllocateInRegion_InBack(&newRegion, size, settings, allocation)) {
                    return true;
                }
            }
        }
        if(crosses) {
            return false;
        }
    }

    return NEXUS_Platform_P_AllocateInRegionOne(region, size, settings, allocation, inFront);
}

static int NEXUS_Platform_P_AllocateInRegions(void *context,const BMMA_RangeAllocator_Region **freeRegions, unsigned nFreeRegions, size_t size, const BMMA_RangeAllocator_BlockSettings *settings, BMMA_RangeAllocator_Region *allocation)
{
    struct NEXUS_Platform_P_AllocatorState *state = context;
    BDBG_MSG(("NEXUS_Platform_P_AllocateInRegions: nFreeRegions %u", nFreeRegions));
    if(state->allocator.hasRegionRestrictedHeaps) {
        if(state->allocator.realAllocation) {
            bool inFront = true;
            if(nFreeRegions>=1) {
                if(state->allocator.heapSettings->placement.first && state->allocator.heapSettings->placement.sage) {
                   /* inFront = false; */
                }
                if(NEXUS_Platform_P_AllocateInRegion(state, inFront, freeRegions[0], size, settings, allocation)) {
                    BDBG_MSG(("NEXUS_Platform_P_AllocateInRegions: ALLOC: 0 %s %#x..%#x -> %#x..%#x", inFront? "inFront":"", (unsigned)freeRegions[0]->base, (unsigned)(freeRegions[0]->base+freeRegions[0]->length), (unsigned)allocation->base, (unsigned)(allocation->base+allocation->length)));
                    return 0;
                }
            }
        } else {
            bool inFront = false;
            if(nFreeRegions>=2) {
                if(NEXUS_Platform_P_AllocateInRegion(state, true, freeRegions[1], size, settings, allocation)) {
                    BDBG_MSG(("NEXUS_Platform_P_AllocateInRegions: PLACE 1 %s %#x..%#x -> %#x..%#x", inFront?"inFront":"", (unsigned)freeRegions[1]->base, (unsigned)(freeRegions[1]->base+freeRegions[1]->length), (unsigned)allocation->base, (unsigned)(allocation->base+allocation->length)));
                    return 1;
                }
            }
            if(nFreeRegions>=1) {
                if(NEXUS_Platform_P_AllocateInRegion(state, inFront, freeRegions[0], size, settings, allocation)) {
                    BDBG_MSG(("NEXUS_Platform_P_AllocateInRegions: PLACE: 0 %s %#x..%#x -> %#x..%#x", inFront? "inFront":"", (unsigned)freeRegions[0]->base, (unsigned)(freeRegions[0]->base+freeRegions[0]->length), (unsigned)allocation->base, (unsigned)(allocation->base+allocation->length)));
                    return 0;
                }
            }
        }
    } else {
        if(nFreeRegions>=1) {
            bool inFront = false;
            inFront = false;
            if(NEXUS_Platform_P_AllocateInRegion(state, inFront, freeRegions[0], size, settings, allocation)) {
                BDBG_MSG(("NEXUS_Platform_P_AllocateInRegions: SIMPLE0 %s %#x..%#x -> %#x..%#x", inFront? "inFront":"", (unsigned)freeRegions[0]->base, (unsigned)(freeRegions[0]->base+freeRegions[0]->length), (unsigned)allocation->base, (unsigned)(allocation->base+allocation->length)));
                return 0;
            }
        }
    }
    return -1;
}

static bool NEXUS_Platform_P_IterateHeapPlacement(void *context, void *block_header, const BMMA_RangeAllocator_Region *region, bool boundary)
{
    struct NEXUS_Platform_P_AllocatorState *state = context;
    int regions = state->allocator.placement.regions;

    if(regions >= (int)(sizeof(state->allocator.placement.heapRegion)/sizeof(state->allocator.placement.heapRegion[0]))) {
        return false;
    }
    state->allocator.placement.heapRegion[regions].region = *region;
    if(block_header) {
        state->allocator.placement.heapRegion[regions].unused = false;
        state->allocator.placement.heapRegion[regions].heapIndex = *(unsigned *)block_header;
    } else {
        state->allocator.placement.heapRegion[regions].unused = true;
        state->allocator.placement.heapRegion[regions].heapIndex = 0;
    }
    regions++;
    state->allocator.placement.regions = regions;
    BSTD_UNUSED(boundary);
    return true;
}

#if 0
static NEXUS_Error NEXUS_Platform_P_AllocateInRange(unsigned memcIndex, BMMA_RangeAllocator_Region *range, const NEXUS_PlatformHeapSettings *heap, unsigned heapIndex, unsigned max_dcache_line_size, struct NEXUS_Platform_P_AllocatorRange *result)
{
    NEXUS_Addr heap_offset;
    size_t alignment;
    size_t heap_size;

    alignment = heap[heapIndex].alignment;
    if(alignment<4096) {
        alignment = 4096;
    }
    if(alignment<max_dcache_line_size) {
        alignment=max_dcache_line_size;
    }
    heap_offset = NEXUS_P_AddrAlign(range->base, alignment);
    if(heap[heapIndex].size>0) {
        heap_size = heap[heapIndex].size;
    } else {
        if(range->base + range->length >= heap_offset) {
            heap_size = (range->base + range->base) - heap_offset;
            if(heap_size < (unsigned)(-(heap[heapIndex].size))) {
                return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            }
        } else {
            heap_size = 1;
        }
    }
    if(range->base + range->length < heap_offset + heap_size) {
        BDBG_MSG(("MEMC%u Can't place HEAP[%u] %uMBytes with alignment %#x", memcIndex, heapIndex, (unsigned)(heap_size/(1024*1024)),heap[heapIndex].alignment));
        return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
    }
    range->length -= (heap_offset - range->base) + heap_size;
    range->base = heap_offset + heap_size;
    result->region.base = heap_offset;
    result->region.length = heap_size;
    result->unused = false;
    result->heapIndex = heapIndex;
    BDBG_MSG(("MEMC%u Placed HEAP[%u] %uMBytes(%#x Bytes) with alignment %#x at " BDBG_UINT64_FMT " (%u MByte)", memcIndex, heapIndex, (unsigned)(heap_size/(1024*1024)), (unsigned)heap_size,heap[heapIndex].alignment, BDBG_UINT64_ARG(heap_offset), (unsigned)(heap_offset/(1024*1024))));

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_AllocateBmemHeaps_old(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformOsRegion *osMemory, const NEXUS_PlatformHeapSettings *heap, unsigned max_dcache_line_size, unsigned memcIndex)
{
    unsigned i;
    NEXUS_Error rc;
    unsigned maxAlignment = max_dcache_line_size;
    BMMA_RangeAllocator_Region bmem_range;
    unsigned dynamic_region = NEXUS_MAX_HEAPS; /* invalid */
    unsigned heap_index;
    unsigned subIndex;

    bmem_range.length = 0;
    bmem_range.base = 0;
    for(subIndex=0,i=0;i<NEXUS_MAX_HEAPS;i++) {
        if(osMemory[i].length >0 && osMemory[i].memcIndex == memcIndex && !osMemory[i].cma) {
            BDBG_MSG(("MEMC%u.%u %uMBytes at " BDBG_UINT64_FMT "", memcIndex, subIndex, (unsigned)(osMemory[i].length/(1024*1024)),BDBG_UINT64_ARG(osMemory[i].base)));
            if(bmem_range.length!=0) {
                BDBG_WRN(("MEC%u Multiple regions are not supported", memcIndex));
                continue;
            }
            bmem_range.base = osMemory[i].base;
            bmem_range.length = osMemory[i].length;
            subIndex++;
        }
    }
    for(heap_index=0;heap_index<NEXUS_MAX_HEAPS;heap_index++) {
        if(heap[heap_index].size==0) {
            continue;
        }
        if(heap[heap_index].memcIndex != memcIndex) {
            continue;
        }
        if(maxAlignment<heap[heap_index].alignment) {
            maxAlignment=heap[heap_index].alignment;
        }
        if(heap[heap_index].size<0) {
            if(dynamic_region != NEXUS_MAX_HEAPS) {
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            dynamic_region = heap_index;
            continue;
        }
        rc = NEXUS_Platform_P_AllocateInRange(memcIndex, &bmem_range, heap, heap_index, max_dcache_line_size, &state->allocator.placement.heapRegion[state->allocator.placement.regions]);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        state->allocator.placement.regions++;
    }
    if(dynamic_region != NEXUS_MAX_HEAPS) {
        rc = NEXUS_Platform_P_AllocateInRange(memcIndex, &bmem_range, heap, dynamic_region, max_dcache_line_size, &state->allocator.placement.heapRegion[state->allocator.placement.regions]);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        state->allocator.placement.regions++;
    }
    return NEXUS_SUCCESS;
}

#endif

static NEXUS_Error NEXUS_Platform_P_AllocateBmemHeaps(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformOsRegion *osMemory, const NEXUS_PlatformHeapSettings *heap, unsigned max_dcache_line_size, unsigned memcIndex, bool realAllocation)
{
    unsigned i;
    unsigned subIndex;
    NEXUS_Error rc;
    BMMA_RangeAllocator_Handle allocator = NULL;
    unsigned stage;
    unsigned min_heap_alignment;
    unsigned max_heap_alignment;

    state->allocator.placement.regions = 0;
    state->allocator.realAllocation = realAllocation;
    state->allocator.hasRegionRestrictedHeaps = false;
    BKNI_Memset(state->allocator.heaps, 0, sizeof(state->allocator.heaps));
    BDBG_MSG(("MEMC%u %s heaps", memcIndex, realAllocation?"allocating":"sizing"));
#if 0
    if(0 && realAllocation) {
        BDBG_WRN(("USING OLD ALLOCATION ALGORITHM"));
        return NEXUS_Platform_P_AllocateBmemHeaps_old(state, osMemory, heap, max_dcache_line_size, memcIndex);
    }
#endif

    min_heap_alignment = max_dcache_line_size;
    if(min_heap_alignment<4096) {
        min_heap_alignment = 4096;
    }
    max_heap_alignment = min_heap_alignment;
    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        if(heap[i].size==0) {
            continue;
        }
        if(heap[i].memcIndex != memcIndex) {
            continue;
        }
        if(max_heap_alignment<heap[i].alignment) {
            max_heap_alignment = heap[i].alignment;
        }
        if(heap[i].placement.region.valid) {
            state->allocator.hasRegionRestrictedHeaps = true;
        }
    }

    /* 1. Create RangeAllocator for MEMC */
    for(subIndex=0,i=0;i<NEXUS_MAX_HEAPS;i++) {
        if(osMemory[i].length >0 && osMemory[i].memcIndex == memcIndex && !osMemory[i].cma) {
            size_t length = osMemory[i].length;


            BDBG_MSG(("MEMC%u.%u %uMBytes(%#x) at " BDBG_UINT64_FMT "", memcIndex, subIndex, (unsigned)(osMemory[i].length/(1024*1024)), (unsigned)(length), BDBG_UINT64_ARG(osMemory[i].base)));
            subIndex++;
            if( length+osMemory[i].base == ((uint64_t)1 << 32)) {
                BDBG_WRN(("truncating MEMC%u.%u %uMBytes(%#x) at " BDBG_UINT64_FMT "", memcIndex, subIndex-1, (unsigned)(osMemory[i].length/(1024*1024)), (unsigned)(length), BDBG_UINT64_ARG(osMemory[i].base)));
                if(length<=min_heap_alignment) {
                    BDBG_WRN(("skipping MEMC%u.%u %uMBytes(%#x) at " BDBG_UINT64_FMT "", memcIndex, subIndex-1, (unsigned)(osMemory[i].length/(1024*1024)), (unsigned)(length), BDBG_UINT64_ARG(osMemory[i].base)));
                    continue;
                }
                length-=min_heap_alignment;
            }
            if(allocator==NULL) {
                BMMA_RangeAllocator_CreateSettings settings;
                BMMA_RangeAllocator_GetDefaultCreateSettings(&settings);
                state->allocator.firstRegion.base = osMemory[i].base;
                state->allocator.firstRegion.length = osMemory[i].length;
                settings.base = osMemory[i].base;
                settings.size = length;
                if(settings.base + settings.size == ((uint64_t)1) << 32) {
                    settings.size -=1;
                }
                settings.minAlignment = min_heap_alignment;
                settings.allocationHeader = sizeof(unsigned);
                settings.context = state;
                settings.allocator = NEXUS_Platform_P_AllocateInRegions;
                if(state->allocator.hasRegionRestrictedHeaps) {
                    settings.allocatorMinFreeSize = max_heap_alignment;
                }
                rc = BMMA_RangeAllocator_Create(&allocator, &settings);
                if(rc!=BERR_SUCCESS) {
                    return BERR_TRACE(rc);
                }
            } else {
                BMMA_RangeAllocator_Region region;
                region.base = osMemory[i].base;
                region.length = length;
                rc = BMMA_RangeAllocator_AddRegion(allocator, &region);
                if(rc!=BERR_SUCCESS) {
                    return BERR_TRACE(rc);
                }
            }
        }
    }

    /* 2. Allocate heaps with BMMA_RangeAllocator */
    for(stage=0;stage<4;stage++) {
        for(i=0;i<NEXUS_MAX_HEAPS;i++) {
            unsigned heapIndex = i;
            size_t heap_size;
            BMMA_RangeAllocator_BlockSettings blockSettings;
            unsigned *header;
            BMMA_DeviceOffset deviceOffset;

            if(!state->allocator.hasRegionRestrictedHeaps) {
                heapIndex = (NEXUS_MAX_HEAPS-1)-i; /* allocate starting from largest number, and allocate from highest addresses */
            }

            if(heap[heapIndex].size==0) {
                continue;
            }
            if(state->allocator.heaps[heapIndex]) {
                continue;
            }
            if(heap[heapIndex].memcIndex != memcIndex) {
                continue;
            }
            if(stage!=3 && heap[heapIndex].size<0) {
                continue;
            }
            if(stage == 0 && !heap[heapIndex].placement.first) {
                continue;
            }
            if(stage == 1 && heap[heapIndex].alignment == 0) {
                continue;
            }

            if(heap[heapIndex].size>=0) {
                heap_size = heap[heapIndex].size;
                if(heap_size==0) {
                    continue;
                }
            } else {
                heap_size = -heap[heapIndex].size;
                if(realAllocation && allocator) {
                    struct BMMA_RangeAllocator_Status status;
                    BMMA_RangeAllocator_GetStatus(allocator, &status);
                    if(heap_size + 2*heap[heapIndex].alignment<status.largestFree) {
                        heap_size = status.largestFree - 2*heap[heapIndex].alignment;
                    }
                }
            }
            if(allocator==NULL) {
                BDBG_ERR(("MEMC%u no regions for HEAP[%u] %uMBytes", memcIndex, heapIndex, (unsigned)(heap_size/(1024*1024))));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            BMMA_RangeAllocator_GetDefaultAllocationSettings(&blockSettings);
            blockSettings.alignment = heap[heapIndex].alignment;
            state->allocator.heapSettings = &heap[heapIndex];
            rc = BMMA_RangeAllocator_Alloc(allocator, &state->allocator.heaps[heapIndex], heap_size, &blockSettings);
            if(rc!=BERR_SUCCESS) {
                BDBG_ERR(("MEMC%u Can't place HEAP[%u%s] %uMBytes(%u Bytes) with alignment %#x%s", memcIndex, heapIndex, ((heap[heapIndex].memoryType&NEXUS_MEMORY_TYPE_HIGH_MEMORY)==NEXUS_MEMORY_TYPE_HIGH_MEMORY)?":HIGH":"", (unsigned)(heap_size/(1024*1024)), (unsigned)heap_size,heap[heapIndex].alignment, heap[heapIndex].placement.sage?" SAGE":""));
                return BERR_TRACE(rc);
            }
            deviceOffset = BMMA_RangeAllocator_GetAllocationBase(state->allocator.heaps[heapIndex]);
            BDBG_MSG(("MEMC%u Placed HEAP[%u%s] %uMBytes(%#x Bytes) with alignment %#x%s at " BDBG_UINT64_FMT " (%u MByte)", memcIndex, heapIndex, ((heap[heapIndex].memoryType&NEXUS_MEMORY_TYPE_HIGH_MEMORY)==NEXUS_MEMORY_TYPE_HIGH_MEMORY)?":HIGH":"", (unsigned)(heap_size/(1024*1024)),(unsigned)heap_size,heap[heapIndex].alignment, heap[heapIndex].placement.sage?" SAGE":"", BDBG_UINT64_ARG(deviceOffset), (unsigned)(deviceOffset/(1024*1024))));
            header = BMMA_RangeAllocator_GetAllocationHeader(state->allocator.heaps[heapIndex]);
            *header = heapIndex;
        }
    }

    /* 3. Record where heaps were placed and destroy allocator */
    if(allocator) {
        BKNI_Memset(&state->allocator.placement, 0, sizeof(state->allocator.placement));
        BMMA_RangeAllocator_Iterate(allocator, NEXUS_Platform_P_IterateHeapPlacement,state,NULL);

        for(i=0;i<NEXUS_MAX_HEAPS;i++) {
            int j;
            if(state->allocator.heaps[i]) {
                BMMA_RangeAllocator_Free(allocator, state->allocator.heaps[i]);
            }
            if(heap[i].memcIndex != memcIndex) {
                continue;
            }
            if(heap[i].size==0) {
                continue;
            }
            for(j=0;;j++) {
                if(j>=state->allocator.placement.regions) {
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* for whatever reason heap was not allocated */
                }
                if(!state->allocator.placement.heapRegion[j].unused && state->allocator.placement.heapRegion[j].heapIndex==i) {
                    break;
                }
            }
        }
        BMMA_RangeAllocator_Destroy(allocator);
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings_allocateBmem(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    NEXUS_Error rc;
    unsigned memcIndex;
    uint64_t totalUsed=0;
    uint64_t totalUnused=0;
    unsigned totalAllocations=0;
    unsigned max_dcache_line_size = pMemory->max_dcache_line_size;
    unsigned maxAlignment = pMemory->max_dcache_line_size;
    unsigned heapIndex;

    for(heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
        if(heap[heapIndex].size==0) {
            continue;
        }
        if(maxAlignment < heap[heapIndex].alignment) {
            maxAlignment = heap[heapIndex].alignment;
        }
    }

    for(memcIndex=0;memcIndex<NEXUS_MAX_MEMC;memcIndex++) {
        int i;
        unsigned allocations=0;
        uint64_t unused=0;

        rc = NEXUS_Platform_P_AllocateBmemHeaps(state, pMemory->osRegion, heap, pMemory->max_dcache_line_size, memcIndex, true);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        for(i=0;i<state->allocator.placement.regions;i++) {
            unsigned heapIndex;
            struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
            if(cur->unused) {
                unused += cur->region.length;
                continue;
            }
            allocations++;
            for(heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
                if(heapIndex==cur->heapIndex) {
                    BDBG_ASSERT(memcIndex == heap[heapIndex].memcIndex);
                    if (max_dcache_line_size > pCoreSettings->heapRegion[heapIndex].alignment) {
                        pCoreSettings->heapRegion[heapIndex].alignment = max_dcache_line_size;
                    }
                    totalUsed += cur->region.length;
                    pCoreSettings->heapRegion[heapIndex].offset = cur->region.base;
                    pCoreSettings->heapRegion[heapIndex].length = cur->region.length;
                    pCoreSettings->heapRegion[heapIndex].memcIndex = heap[heapIndex].memcIndex;
                    pCoreSettings->heapRegion[heapIndex].memoryType = heap[heapIndex].memoryType;
                    pCoreSettings->heapRegion[heapIndex].heapType = heap[heapIndex].heapType;
                    break;
                }
            }
        }
        if(allocations || unused) {
            BDBG_MSG(("MEMC%u unused %uMBytes of BMEM memory", memcIndex, (unsigned)(unused/(1024*1024))));
            totalUnused += unused;
            totalAllocations += allocations;
        }
    }
    BDBG_MSG(("unused %uMBytes of BMEM memory (used %uMBytes), thresholds (%u,%u)MBytes ", (unsigned)(totalUnused/(1024*1024)), (unsigned)(totalUsed/(1024*1024)), (unsigned)((maxAlignment*totalAllocations)/(1024*1024)), (unsigned)((1024*1024*totalAllocations)/(1024*1024)) ));
    if(totalUnused>maxAlignment*totalAllocations && totalUnused>(1024*1024*totalAllocations)) {
        state->extraMemory = true;
        BDBG_WRN(("unused %uMBytes of BMEM memory (used %uMBytes)", (unsigned)(totalUnused/(1024*1024)), (unsigned)(totalUsed/(1024*1024))));
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_Allocator_InitializeState (struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformOsRegion *osRegion, bool allocateFromCma)
{
    unsigned i,j;
    const unsigned n_regions = NEXUS_MAX_HEAPS;

    /* Build map from memc/region to osRegion */
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        for(j=0;j<NEXUS_NUM_MEMC_REGIONS;j++) {
            state->osRegion.map[i][j] = -1;
        }
    }
    for (i=0;i<n_regions;i++) {
        if(osRegion[i].length==0) {
            continue;
        }
        if(osRegion[i].cma != allocateFromCma) {
            continue;
        }
        BDBG_ASSERT(osRegion[i].memcIndex < NEXUS_MAX_MEMC);
        BDBG_ASSERT(osRegion[i].subIndex < NEXUS_NUM_MEMC_REGIONS);
        BDBG_ASSERT(state->osRegion.map[osRegion[i].memcIndex][osRegion[i].subIndex] == -1);
        state->osRegion.map[osRegion[i].memcIndex][osRegion[i].subIndex] = i;
    }
    return;
}

static const nexus_p_memory_range *NEXUS_Platform_P_GetLowestRegion(const nexus_p_memory_info *info)
{
    unsigned i;
    const nexus_p_memory_range *min_range=NULL;
    for(i=0;i<sizeof(info->memc)/sizeof(info->memc[0]);i++) {
        unsigned j;
        for(j=0;j<sizeof(info->memc[0].range)/sizeof(info->memc[0].range[0]);j++) {
            if(j>=(unsigned)info->memc[i].count) {
                break;
            }
            if(min_range==NULL || min_range->addr>=info->memc[i].range[j].addr) {
                min_range = &info->memc[i].range[j];
            }
        }
    }
    return min_range;
}

static const nexus_p_memory_range *NEXUS_Platform_P_GetNextRegion(const nexus_p_memory_info *info,const nexus_p_memory_range *region)
{
    unsigned i;
    for(i=0;i<sizeof(info->memc)/sizeof(info->memc[0]);i++) {
        unsigned j;
        for(j=0;j<sizeof(info->memc[0].range)/sizeof(info->memc[0].range[0]);j++) {
            if(j>=(unsigned)info->memc[i].count) {
                break;
            }
            if(region->addr + region->size == info->memc[i].range[j].addr) {
                return &info->memc[i].range[j];
            }
        }
    }
    return NULL;
}

static unsigned NEXUS_Platform_P_GetVmallocSize(const NEXUS_PlatformHeapSettings *heap, const nexus_p_memory_info *info)
{
    unsigned required_vmalloc = 0;
    unsigned i;
    unsigned lowest_bmem_addr = 0;
    const unsigned ranges = sizeof(info->bmem.range)/sizeof(info->bmem.range[0]);
    uint64_t highest_installed_dram=0;
    unsigned vmalloc;
    const nexus_p_memory_range *range;

    /* Use the same register size maths as NEXUS_Platform_P_MapRegisters */
    required_vmalloc += (BCHP_REGISTER_END - (BCHP_REGISTER_START & ~0xFFF));

    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        if( heap[i].size==0) {
            continue;
        }
        if( heap[i].memoryType & (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED | NEXUS_MEMORY_TYPE_DRIVER_CACHED)) {
            required_vmalloc += heap[i].size;
        }
    }
    for(i=0;i<ranges;i++) {
        if(info->bmem.range[i].size==0) {
            break;
        }
        if(lowest_bmem_addr==0 || lowest_bmem_addr >= info->bmem.range[i].addr) {
            lowest_bmem_addr=info->bmem.range[i].addr;
        }
    }

    range = NEXUS_Platform_P_GetLowestRegion(info);
    if(range==NULL) {
        (void)BERR_TRACE(NEXUS_UNKNOWN);
        return 0;
    }
    for(;;) {
        const nexus_p_memory_range *next_range = NEXUS_Platform_P_GetNextRegion(info, range);
        if(next_range==NULL) {
            break;
        }
        range = next_range;
    }

    highest_installed_dram = range->addr + range->size;
    vmalloc = vmalloc_size(required_vmalloc, highest_installed_dram, lowest_bmem_addr);
    BDBG_MSG(("required_vmalloc:%uMB lowest_bmem_addr:%uMB highest_installed_dram:%uMB -> vmalloc:%uMB", (unsigned)(required_vmalloc/(1024*1024)), (unsigned)(lowest_bmem_addr/(1024*1024)),(unsigned)(highest_installed_dram/(1024*1024)), (unsigned)vmalloc));

    return vmalloc;
}

static void NEXUS_Platform_P_AllocatorRangeRemove(struct NEXUS_Platform_P_AllocatorRange *base, int first, int regions)
{
    int i;

    for(i=first;i<regions;i++) {
        base[i] = base[i+1];
    }
    return;
}

static void NEXUS_Platform_P_SuggestBootParamCompactRanges(struct NEXUS_Platform_P_AllocatorState *state, unsigned minAlignment)
{
    int i;
    int regions = state->allocator.placement.regions;
#if 0
    for(i=0;i<regions;i++) {
        struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
        BDBG_LOG(("%d: %s %uMB@%uMB", i,cur->unused?"free":"alloc", (unsigned)(cur->region.length/(1024*1024)), (unsigned)(cur->region.base/(1024*1024))));
    }
#endif
    for(i=0;i<regions-1;) {
        struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
        struct NEXUS_Platform_P_AllocatorRange *next = &state->allocator.placement.heapRegion[i+1];
        if(cur->region.base + cur->region.length == next->region.base) {
            if(cur->unused == next->unused) { /* combine two regions */
                cur->region.length += next->region.length;
                regions--;
                NEXUS_Platform_P_AllocatorRangeRemove(state->allocator.placement.heapRegion, i+1, regions);
                continue;
            } else if(cur->unused) {
                if(cur->region.length < minAlignment) { /* merge unused region with next allocated */
                    cur->region.length += next->region.length;
                    cur->unused = next->unused;
                    regions--;
                    NEXUS_Platform_P_AllocatorRangeRemove(state->allocator.placement.heapRegion, i+1, regions);
                    continue;
                }
            } else if( /* !cur->unused && */ next->region.length < minAlignment) { /* merge allocated region with next unused */
               cur->region.length += next->region.length;
               regions--;
               NEXUS_Platform_P_AllocatorRangeRemove(state->allocator.placement.heapRegion, i+1, regions);
               continue;
            }
        }
        i++;
    }
    for(i=0;i<regions-1;i++) {
        struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
        struct NEXUS_Platform_P_AllocatorRange *next = &state->allocator.placement.heapRegion[i+1];
        if(cur->region.base + cur->region.length == next->region.base) {
            if(cur->unused) { /* use this FREE region to pad ALLOCATED region to the alignment boundary */
                unsigned misAlignment;
                BDBG_ASSERT(!next->unused);
                misAlignment = next->region.base % minAlignment;
                if(misAlignment) {
                    BDBG_ASSERT(misAlignment < minAlignment);
                    if(cur->region.length >= misAlignment) {
                        cur->region.length -= misAlignment;
                        next->region.length += misAlignment;
                        next->region.base -= misAlignment;
                    }
                }
            }
        }
    }
#if 0
    for(i=0;i<regions;i++) {
        struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
        BDBG_LOG(("%d: %s %uMB@%uMB", i,cur->unused?"free":"alloc", (unsigned)(cur->region.length/(1024*1024)), (unsigned)(cur->region.base/(1024*1024))));
    }
#endif
    state->allocator.placement.regions = regions;
    return;
}


static NEXUS_Error NEXUS_Platform_P_CalculateBootParams(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformHeapSettings *heap, unsigned max_dcache_line_size)
{
    NEXUS_Error rc;
    const unsigned minAlignment = 1 * 1024 * 1024; /* so to keep boot params 1MBytes aligned */
    unsigned memcIndex;

    BKNI_Memset(&state->bmem_hint.info.bmem, 0, sizeof(state->bmem_hint.info.bmem));
    BKNI_Memset(&state->bmem_hint.info.cma, 0, sizeof(state->bmem_hint.info.cma));

    rc = NEXUS_Platform_P_SetHostMemoryFromInfo(&state->bmem_hint.info, state->bmem_hint.osMemory, sizeof(state->bmem_hint.osMemory)/sizeof(state->bmem_hint.osMemory[0]), heap, minAlignment);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    state->bmem_hint.info.bmem.count=0;

    NEXUS_Platform_P_Allocator_InitializeState(state, state->bmem_hint.osMemory, false);
    for(memcIndex=0;memcIndex<NEXUS_MAX_MEMC;memcIndex++) {
        unsigned i;
        rc = NEXUS_Platform_P_AllocateBmemHeaps(state, state->bmem_hint.osMemory, heap, max_dcache_line_size, memcIndex, false);
        if(rc!=NEXUS_SUCCESS) { return BERR_TRACE(rc); }

        if(state->allocator.placement.regions>0) {
            unsigned maxAlignment = minAlignment;
            int bmemIndex;

            for(i=0;i<NEXUS_MAX_HEAPS;i++) {
                if(heap[i].memcIndex != memcIndex) {
                    continue;
                }
                if(maxAlignment<heap[i].alignment) {
                    maxAlignment = heap[i].alignment;
                }
            }
            NEXUS_Platform_P_SuggestBootParamCompactRanges(state, maxAlignment);
            bmemIndex = state->bmem_hint.info.bmem.count;
            for(i=0;(int)i<state->allocator.placement.regions;i++) {
                const struct NEXUS_Platform_P_AllocatorRange *cur = &state->allocator.placement.heapRegion[i];
                if(!cur->unused) {
                    if(bmemIndex >= (int)(sizeof(state->bmem_hint.info.bmem.range)/sizeof(state->bmem_hint.info.bmem.range[0]))) {
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                    state->bmem_hint.info.bmem.range[bmemIndex].addr = cur->region.base;
                    state->bmem_hint.info.bmem.range[bmemIndex].size = cur->region.length;
                    bmemIndex++;
                }
            }
            state->bmem_hint.info.bmem.count = bmemIndex;
        }
    }

    BKNI_Memset(state->bmem_hint.osMemory, 0, sizeof(state->bmem_hint.osMemory));
    rc = NEXUS_Platform_P_SetHostBmemMemoryFromInfo(&state->bmem_hint.info, state->bmem_hint.osMemory);
    if(rc!=NEXUS_SUCCESS) { return BERR_TRACE(rc);}
    return NEXUS_SUCCESS;
}

static bool NEXUS_Platform_P_TestIntersect(NEXUS_Addr addr1, size_t size1, NEXUS_Addr addr2, size_t size2)
{
    return (addr1 < addr2+size2) && (addr2 < addr1+size1);
}

static bool NEXUS_Platform_P_TestContain(NEXUS_Addr addr1, size_t size1, NEXUS_Addr addr2, size_t size2)
{
    return (addr2 >= addr1) && (addr2 + size2 <= addr1 + size1);
}

static bool NEXUS_Platform_P_RangeTestIntersect(const nexus_p_memory_range *outer, NEXUS_Addr addr, size_t size)
{
    return NEXUS_Platform_P_TestIntersect(outer->addr, outer->size, addr, size);
}



static NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings_Verify(const nexus_p_memory_info *info, const NEXUS_PlatformHeapSettings *heapSettings, const NEXUS_Core_Settings *pCoreSettings, const NEXUS_PlatformMemory *pMemory)
{
    unsigned i;

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        const NEXUS_PlatformHeapSettings *heap = &heapSettings[i];
        const NEXUS_Core_MemoryRegion *region;
        size_t size;
        unsigned j;
        if(heap->size==0) {
            continue;
        }
        region = &pCoreSettings->heapRegion[i];
        if(heap->size>0) {
            size = heap->size;
        } else {
            size = -heap->size;
        }
        if(region->length < size) {
            BDBG_ERR(("heap[%u] at %u smaller %u then requested", i, (unsigned)region->length, (unsigned)size));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if(heap->alignment) {
            if(region->offset % heap->alignment != 0) {
                BDBG_ERR(("heap[%u] at offset " BDBG_UINT64_FMT " not %u aligned", i, BDBG_UINT64_ARG(region->offset), (unsigned)heap->alignment));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        if(heap->placement.sage) {
            const unsigned _256M = 256 * 1024 * 1024;
            const unsigned _1024M = 1024 * 1024 * 1024;
            if(
               NEXUS_Platform_P_TestIntersect(region->offset, region->length, _256M, 0) ||
               NEXUS_Platform_P_TestIntersect(region->offset, region->length, _1024M, 0)) {
                BDBG_ERR(("heap[%u] at " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " can't cross 256M or 1024M boundary", i, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length)));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        if(heap->placement.region.valid) {
            if(!NEXUS_Platform_P_TestContain(heap->placement.region.base, heap->placement.region.length, region->offset, region->length)) {
                BDBG_ERR(("heap[%u] at " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " must be placed in " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " range", i, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), BDBG_UINT64_ARG(heap->placement.region.base), BDBG_UINT64_ARG(heap->placement.region.base + heap->placement.region.length)));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        if((heap->memoryType&NEXUS_MEMORY_TYPE_SECURE)==NEXUS_MEMORY_TYPE_SECURE && info ) {
            for(j=0;j<(unsigned)info->lowmem.count;j++) {
                if(NEXUS_Platform_P_RangeTestIntersect(&info->lowmem.range[j], region->offset, region->length) && !NEXUS_Platform_P_IsOs64()) {
                    BDBG_ERR(("SECURE heap[%u] at " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " intersects with lowmem %uMBytes(%u) at " BDBG_UINT64_FMT "", i, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), (unsigned)(info->lowmem.range[j].size/(1024*1024)), (unsigned)info->lowmem.range[j].size, BDBG_UINT64_ARG(info->lowmem.range[j].addr)));
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
            }
        }
        for (j=0;j<NEXUS_MAX_HEAPS;j++) {
            const NEXUS_Core_MemoryRegion *testRegion = &pCoreSettings->heapRegion[j];
            if(heapSettings[j].size==0) {
                continue;
            }
            if(i==j) {
                continue;
            }
            if(NEXUS_Platform_P_TestIntersect(region->offset, region->length, testRegion->offset, testRegion->length)) {
                BDBG_ERR(("heap[%u] at " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " intersects with heap[%u] " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , i, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), j, BDBG_UINT64_ARG(testRegion->offset), BDBG_UINT64_ARG(testRegion->offset+testRegion->length)));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        for (j=0;j<NEXUS_MAX_HEAPS;j++) {
            const NEXUS_PlatformOsRegion *osRegion = &pMemory->osRegion[j];
            if(osRegion->length==0) {
                continue;
            }
            if(NEXUS_Platform_P_TestContain(osRegion->base, osRegion->length, region->offset, region->length)) {
                if(heap->memcIndex != osRegion->memcIndex) {
                    BDBG_ERR(("heap[%u] at MEMC%u " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " allocated from MEMC%u " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT , i, heap->memcIndex, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length), osRegion->memcIndex, BDBG_UINT64_ARG(osRegion->base), BDBG_UINT64_ARG(osRegion->base+osRegion->length)));
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
                break;
            }
        }
        if(j==NEXUS_MAX_HEAPS) {
            BDBG_ERR(("heap[%u] at " BDBG_UINT64_FMT ".." BDBG_UINT64_FMT " allocated outside of known MEMC ranges", i, BDBG_UINT64_ARG(region->offset), BDBG_UINT64_ARG(region->offset+region->length)));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings_Adjust(const NEXUS_PlatformHeapSettings *heapSettings, NEXUS_Core_Settings *pCoreSettings)
{
    unsigned heapIndex;
    for (heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
        const NEXUS_PlatformHeapSettings *heap = &heapSettings[heapIndex];
        if(heap->size==0) {
            continue;
        }
        if(pCoreSettings->heapRegion[heapIndex].length==0) {
            continue;
        }
        if( (heap->memoryType & NEXUS_MEMORY_TYPE_SECURE) == NEXUS_MEMORY_TYPE_SECURE) {
            unsigned j;
            /* find unsecure heap adjacent to the secure heap */
            for(j=0;j<NEXUS_MAX_HEAPS;j++) {
                if(j==heapIndex) {
                    continue;
                }
                if(heapSettings[j].size==0) {
                    continue;
                }
                if(heap->memcIndex != heapSettings[j].memcIndex) {
                    continue;
                }
                if( (heapSettings[j].memoryType & NEXUS_MEMORY_TYPE_SECURE) == NEXUS_MEMORY_TYPE_SECURE) {
                    continue;
                }
                if( (heapSettings[j].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) == NEXUS_HEAP_TYPE_PICTURE_BUFFERS) {
                    /* can't modify size of the picture buffer heaps */
                    continue;
                }
                if(pCoreSettings->heapRegion[j].offset + pCoreSettings->heapRegion[j].length == pCoreSettings->heapRegion[heapIndex].offset && pCoreSettings->heapRegion[j].length>4096) {
                    BDBG_MSG(("MEMC%u Adjusting size of unsecure heap:%u adjacent to secure heap:%u", heap->memcIndex, j, heapIndex));
                    pCoreSettings->heapRegion[j].length -= 4096;
                }
            }
        }
    }
    return NEXUS_SUCCESS;
}
