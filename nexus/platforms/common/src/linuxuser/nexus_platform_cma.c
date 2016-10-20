/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
***************************************************************************/
#include "nexus_platform_priv.h"
#include "bchp_common.h"
#include "bmma_range.h"

#if NEXUS_USE_CMA && !B_REFSW_SYSTEM_MODE_CLIENT

BDBG_MODULE(nexus_platform_cma);

#if NEXUS_BASE_OS_linuxkernel
#include <linux/brcmstb/brcmstb.h>
#if BRCMSTB_H_VERSION >= 1
typedef struct brcmstb_memory nexus_p_memory_info;
typedef struct brcmstb_range nexus_p_memory_range;
#else
typedef struct brcmstb_range {
    uint8_t addr;
    uint8_t size;  /* 0 means no region */
} nexus_p_memory_range;

typedef struct brcmstb_memory {
    struct {
        struct brcmstb_range range[1];
        int count;
    } memc[1];
    /* fixed map space */
    struct {
        struct brcmstb_range range[1];
        int count;
    } lowmem;
    /* bmem carveout regions */
    struct {
        struct brcmstb_range range[1];
        int count;
    } bmem;
    /* CMA carveout regions */
    struct {
        struct brcmstb_range range[1];
        int count;
    } cma;
    /* regions that nexus cannot recommend for bmem or CMA */
    struct {
        struct brcmstb_range range[1];
        int count;
    } reserved;
} nexus_p_memory_info;
#endif /* BRCMSTB_H_VERSION >= 1 */
#else /* NEXUS_BASE_OS_linuxkernel */
#include "bcm_driver.h"
typedef struct bcmdriver_memory_info nexus_p_memory_info;
typedef struct bcmdriver_memory_range nexus_p_memory_range;
#endif

BDBG_FILE_MODULE(nexus_platform_settings);

struct nexus_linux_cma_region_info {
    uint64_t length;
    NEXUS_Addr physicalAddress;
    unsigned memcIndex;
};

#if NEXUS_BASE_OS_linuxkernel
#include <linux/kconfig.h>
#include <linux/cdev.h>
#include "nexus_platform_features.h"

#include "b_vmalloc.inc"

#if defined(CONFIG_CMA)
#include <linux/brcmstb/cma_driver.h>
static int cma_dev_get_phys_info(struct cma_dev *cma_dev, struct nexus_linux_cma_region_info *region_info)
{
    region_info->physicalAddress = cma_dev->range.base;
    region_info->length = cma_dev->range.size;
    region_info->memcIndex = cma_dev->memc;
    return 0;
}
#else /* defined(CONFIG_CMA) */

struct cma_dev *cma_dev_get_cma_dev(unsigned region)
{
    BSTD_UNUSED(region);
    return NULL;
}

static unsigned get_cma_index(struct cma_dev *cma_dev)
{
    BSTD_UNUSED(cma_dev);
    return 0;
}

static int cma_dev_get_mem(struct cma_dev *cma_dev, NEXUS_Addr *addr, uint32_t num_bytes, uint32_t align_bytes)
{
    BSTD_UNUSED(cma_dev);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(num_bytes);
    BSTD_UNUSED(align_bytes);
    return -1;
}

static int cma_dev_put_mem(struct cma_dev *cma_dev, uint64_t addr, uint32_t num_bytes)
{
    BSTD_UNUSED(cma_dev);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(num_bytes);
    return -1;
}

static unsigned cma_dev_get_num_regions(struct cma_dev *cma_dev)
{
    BSTD_UNUSED(cma_dev);
    return 0;
}

static int cma_dev_get_region_info(struct cma_dev *cma_dev, unsigned region_num, unsigned *memc, NEXUS_Addr *addr, uint64_t *num_bytes)
{
    BSTD_UNUSED(cma_dev);
    BSTD_UNUSED(addr);
    BSTD_UNUSED(num_bytes);
    BSTD_UNUSED(memc);
    BSTD_UNUSED(region_num);
    return -1;
}

static int cma_dev_get_phys_info(struct cma_dev *cma_dev, struct nexus_linux_cma_region_info *region_info)
{
    BSTD_UNUSED(cma_dev);
    BSTD_UNUSED(region_info);
    return -1;
}

#endif /* defined(CONFIG_CMA) */

static NEXUS_Error NEXUS_Platform_P_GetMemoryInfo(nexus_p_memory_info *info)
{
#if BRCMSTB_H_VERSION >= 1
    brcmstb_memory_get(info);
    return NEXUS_SUCCESS;
#else
    return NEXUS_NOT_SUPPORTED;
#endif
}

#else /* NEXUS_BASE_OS_linuxkernel */
/* linuxuser emulation of linuxkernel cma_dev interface */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>
#include "bkni.h"

static int g_CmaFd=-1;


NEXUS_Error NEXUS_Platform_P_InitCma(void)
{
    struct bcmdriver_version get_version;
    int ret;

    g_CmaFd = open("/dev/brcm0", O_RDWR);
    if(g_CmaFd<0) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    get_version.version = 0;
    ret = ioctl(g_CmaFd, BRCM_IOCTL_GET_VERSION, &get_version);
    if (ret!=0 || get_version.version != BCMDRIVER_VERSION) {
        BDBG_ERR(("Not supported bcmdriver version %u != %u", get_version.version, BCMDRIVER_VERSION));
        ret = BERR_TRACE(BERR_OS_ERROR);
#if BCHP_CHIP != 11360
        return ret;
#endif
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UnInitCma(void)
{
    if (g_CmaFd >= 0) {
        close(g_CmaFd);
        g_CmaFd=-1;
    }
}

static NEXUS_Error NEXUS_Platform_P_GetMemoryInfo(nexus_p_memory_info *info)
{
    int ret;
    ret = ioctl(g_CmaFd, BRCM_IOCTL_GET_MEMORY_INFO, info);
    if(ret!=0) {
        return NEXUS_NOT_SUPPORTED; /* valid, if new code runs with the old kernel */
    }
    return NEXUS_SUCCESS;
}

/* convert unsigned to/from non-null pointer for api compat */
struct cma_dev *cma_dev_get_cma_dev(unsigned region)
{
    return (struct cma_dev *)((unsigned long)region+1);
}

static unsigned get_cma_index(struct cma_dev *cma_dev)
{
    return ((unsigned)(unsigned long)cma_dev)-1;
}

static int cma_dev_get_mem(struct cma_dev *cma_dev, NEXUS_Addr *addr, uint32_t num_bytes, uint32_t align_bytes)
{
    int ret;
    struct bcmdriver_cma_getmem get_mem_p;

    memset(&get_mem_p, 0, sizeof(get_mem_p));
    get_mem_p.cmaIndex = get_cma_index(cma_dev);
    get_mem_p.size = num_bytes;
    get_mem_p.alignment = align_bytes;

    ret = ioctl(g_CmaFd, BRCM_IOCTL_CMA_GETMEM, &get_mem_p);
    if (ret) {
        BDBG_ERR(("CMA_DEV_IOC_GETMEM failed: %d", ret));
        return -1;
    }

    *addr = get_mem_p.addr;
    BDBG_MSG(("alloc PA=" BDBG_UINT64_FMT " LEN=%#x", BDBG_UINT64_ARG(*addr), num_bytes));
    return 0;
}

static int cma_dev_put_mem(struct cma_dev *cma_dev, uint64_t addr, uint32_t num_bytes)
{
    int ret;
    struct bcmdriver_cma_putmem put_mem_p;

    memset(&put_mem_p, 0, sizeof(put_mem_p));
    put_mem_p.cmaIndex = get_cma_index(cma_dev);
    put_mem_p.addr = addr;
    put_mem_p.size = num_bytes;

    ret = ioctl(g_CmaFd, BRCM_IOCTL_CMA_PUTMEM, &put_mem_p);
    if (ret) {
        BDBG_ERR(("CMA_DEV_IOC_PUTMEM failed: %d", ret));
        return -1;
    }

    BDBG_MSG(("free PA=" BDBG_UINT64_FMT " %d bytes in cma %d", BDBG_UINT64_ARG(addr), num_bytes,get_cma_index(cma_dev) ));
    return 0;
}

static unsigned cma_dev_get_num_regions(struct cma_dev *cma_dev)
{
    int ret;
    struct bcmdriver_cma_numregions getnumregs_p;
    memset(&getnumregs_p, 0, sizeof(getnumregs_p));
    getnumregs_p.cmaIndex = get_cma_index(cma_dev);
    ret = ioctl(g_CmaFd, BRCM_IOCTL_CMA_NUMREGIONS, &getnumregs_p);
    if (ret) {
        /* failure is normal to discover bounds */
        BDBG_MSG(("CMA_DEV_IOC_GETNUMREGS failed: %d dev_index %d", ret, get_cma_index(cma_dev)));
        return 0;
    }

    return getnumregs_p.num;
}

static int cma_dev_get_region_info(struct cma_dev *cma_dev, unsigned region_num, unsigned *memc, NEXUS_Addr *addr, uint64_t *num_bytes)
{
    int ret;
    struct bcmdriver_cma_getregion getreginfo_p;
    memset(&getreginfo_p, 0, sizeof(getreginfo_p));

    getreginfo_p.cmaIndex = get_cma_index(cma_dev);
    getreginfo_p.region = region_num;
    ret = ioctl(g_CmaFd, BRCM_IOCTL_CMA_GETREGIONINFO, &getreginfo_p);
    if (ret) {
        BDBG_ERR(("CMA_DEV_IOC_GETREGINFO failed: %d", ret));
        return ret;
    }
    *memc = getreginfo_p.memc;
    *addr = getreginfo_p.addr;
    *num_bytes = getreginfo_p.size;
    return 0;
}

static int cma_dev_get_phys_info(struct cma_dev *cma_dev, struct nexus_linux_cma_region_info *region_info)
{
    int ret;
    struct bcmdriver_cma_getinfo physinfo_p;
    memset(&physinfo_p, 0, sizeof(physinfo_p));
    physinfo_p.cmaIndex = get_cma_index(cma_dev);
    ret = ioctl(g_CmaFd, BRCM_IOCTL_CMA_GETINFO, &physinfo_p);
    if (ret) {
        BDBG_MSG(("CMA_DEV_IOC_GETPHYSINFO failed: %d", ret));
        return -1;
    }
    region_info->physicalAddress = physinfo_p.addr;
    region_info->length = physinfo_p.size;
    region_info->memcIndex = physinfo_p.memc;
    return 0;
}

static unsigned vmalloc_size(unsigned required_vmalloc, uint64_t highest_installed_dram, unsigned lowest_bmem_addr )
{
    int ret;
    struct bcmdriver_get_vmalloc get_vmalloc;

    BKNI_Memset(&get_vmalloc, 0, sizeof(get_vmalloc));
    get_vmalloc.in.required_vmalloc = required_vmalloc;
    get_vmalloc.in.highest_installed_dram = highest_installed_dram;
    get_vmalloc.in.lowest_bmem_addr = lowest_bmem_addr;
    ret = ioctl(g_CmaFd, BRCM_IOCTL_GET_VMALLOC, &get_vmalloc);
    if (ret) {
        BDBG_MSG(("failed: %d", ret));
        return 0;
    }
    return get_vmalloc.out.vmalloc;
}

#endif /* NEXUS_BASE_OS_linuxkernel */

/**
generic linuxuser/linuxkernel cma code
**/

/* TODO: instead of extending these macros, refactor struct into linked list */
#define NEXUS_CMA_MAX_REGIONS NEXUS_MAX_HEAPS
#define NEXUS_CMA_MAX_INDEX (NEXUS_NUM_MEMC*NEXUS_NUM_MEMC_REGIONS)

struct nexus_linux_cma_memory_info_entry {
    unsigned total_regions;
    struct nexus_linux_cma_region_info region_info[NEXUS_CMA_MAX_REGIONS];
};

struct nexus_linux_cma_memory_info {
    struct nexus_linux_cma_memory_info_entry entries[NEXUS_CMA_MAX_INDEX];
};

struct nexus_cma_map {
    unsigned total_regions;
    struct{
        unsigned alignment;
        int length;
        NEXUS_Addr physicalAddress;
        unsigned memcIndex;
        unsigned nexus_heap_index;
    } region_info[NEXUS_CMA_MAX_REGIONS];
};

static void cma_reset_all(unsigned cma_index, struct cma_dev *cma_dev)
{
    uint32_t count = cma_dev_get_num_regions(cma_dev);
    while (count != 0) {
        int ret;
        uint32_t memc;
        NEXUS_Addr addr;
        uint64_t len;
        ret = cma_dev_get_region_info(cma_dev, 0, &memc, &addr, &len);
        if (ret) break;
        if (cma_dev_put_mem(cma_dev,addr, len) != 0)
        {
            BDBG_ERR(("cma:free failed PA " BDBG_UINT64_FMT ", size %u cma %d", BDBG_UINT64_ARG(addr),(unsigned)len,cma_index));
            break;
        }
        BDBG_WRN(("cma:free PA " BDBG_UINT64_FMT ", size %u cma %d", BDBG_UINT64_ARG(addr),(unsigned)len,cma_index));
        count = cma_dev_get_num_regions(cma_dev);
    }
}

/* allocate subregions in a given cma region */
static int cma_alloc_regions(unsigned cma_index, struct cma_dev *cma_dev,struct nexus_cma_map *pCmaMap)
{
    unsigned i;
    for (i=0;i<pCmaMap->total_regions;i++) {
        unsigned length = pCmaMap->region_info[i].length;
        if (cma_dev_get_mem(cma_dev, &pCmaMap->region_info[i].physicalAddress, length, pCmaMap->region_info[i].alignment) != 0) {
            BDBG_ERR(("cma failed to allocate %d bytes in cma region %d",length, cma_index));
            return -1;
        }
        if (pCmaMap->region_info[i].alignment &&
            (pCmaMap->region_info[i].physicalAddress & (pCmaMap->region_info[i].alignment-1))) {
            BDBG_WRN(("cma.%d failed alignment: offset " BDBG_UINT64_FMT ", alignment %#x", cma_index, BDBG_UINT64_ARG(pCmaMap->region_info[i].physicalAddress), pCmaMap->region_info[i].alignment));
        }
    }
    return 0;
}

static NEXUS_Error nexus_platform_get_cma_index(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, unsigned *pcma_index)
{
    unsigned i;
    int cma_offset=-1;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pMemory->osRegion[i].cma) {
            continue;
        }
        if(cma_offset<0) {
            cma_offset = i;
        }
        if (pMemory->osRegion[i].length && pMemory->osRegion[i].memcIndex == memcIndex && pMemory->osRegion[i].subIndex == subIndex) {
            *pcma_index = i - cma_offset;
            return NEXUS_SUCCESS;
        }
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

static NEXUS_Error NEXUS_Platform_P_GetCmaRegions(struct nexus_linux_cma_memory_info *linux_cma_info)
{
    unsigned i;
    BKNI_Memset(linux_cma_info,0,sizeof(*linux_cma_info));
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++)
    {
        unsigned j;
        unsigned region;
        struct cma_dev *cma_dev = cma_dev_get_cma_dev(i);
        if (!cma_dev) break;
        region = cma_dev_get_num_regions(cma_dev);
        linux_cma_info->entries[i].total_regions = region;
        for (j=0;j<region;j++)
        {
            int rc = cma_dev_get_region_info(cma_dev, j, &linux_cma_info->entries[i].region_info[j].memcIndex, &linux_cma_info->entries[i].region_info[j].physicalAddress, &linux_cma_info->entries[i].region_info[j].length);
            if (rc!=0) { return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
            if (j > 0 && linux_cma_info->entries[i].region_info[j].physicalAddress < linux_cma_info->entries[i].region_info[0].physicalAddress) {
                /* some versions of linux return in descending order. normalize to ascending. */
                unsigned k;
                struct nexus_linux_cma_memory_info_entry temp = linux_cma_info->entries[i];
                for (k=1;k<j+1;k++) {
                    linux_cma_info->entries[i].region_info[k] = temp.region_info[k-1];
                }
                linux_cma_info->entries[i].region_info[0] = temp.region_info[j];
            }
        }
    }
    return NEXUS_SUCCESS;
}

#include "nexus_platform_heaps.c"

static void NEXUS_P_MemcGetPermutation( const struct NEXUS_P_MemcRegionCandidates *candidates, unsigned permutationNo, struct NEXUS_P_MemcPermutation *permutation)
{
    unsigned i;
    for(i=0;i<candidates->count;i++) {
        unsigned offset = permutationNo % candidates->heap[i].size;
        permutation->offset[i] = offset;
        permutation->subIndex[i] = candidates->heap[i].possibleRegions[offset];
        permutationNo = permutationNo / candidates->heap[i].size;
    }
    return;
}


static NEXUS_Error NEXUS_Platform_P_BuildRegionCandidates(const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformOsRegion *osRegion, const struct NEXUS_P_MemcRegionOsMap *map, unsigned memc, struct NEXUS_P_MemcRegionCandidates *candidates)
{
    unsigned i;
    unsigned heapNo;
    BKNI_Memset(candidates, 0, sizeof(*candidates));
    for(i=0,heapNo=0;heapNo<NEXUS_MAX_HEAPS;heapNo++) {
        int subIndex,subIndexStep;
        unsigned count =0;

        if( heap[heapNo].size==0) {
            continue;
        }
        if(heap[heapNo].memcIndex != memc) {
            continue;
        }
        if( heap[heapNo].memoryType & (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED | NEXUS_MEMORY_TYPE_DRIVER_CACHED)) {
            subIndex = 0; /* start from the first subIndex */
            subIndexStep = 1;
        } else {
            subIndex=NEXUS_NUM_MEMC_REGIONS-1; /* start from the last subIndex */
            subIndexStep = -1;
        }
        for(;subIndex<NEXUS_NUM_MEMC_REGIONS && subIndex>=0;subIndex+=subIndexStep) {
            if(map->map[memc][subIndex]<0) { /* no such region */
                continue;
            }
            if( (heap[heapNo].memoryType & NEXUS_MEMORY_TYPE_HIGH_MEMORY) != NEXUS_MEMORY_TYPE_HIGH_MEMORY &&
                osRegion[map->map[memc][subIndex]].base >= ((uint64_t)1)<<32) { /* outside of 4GB window */
                continue;
            }
            candidates->heap[i].possibleRegions[count] = subIndex;
            count++;
        }
        candidates->heap[i].size = count;
        candidates->heap[i].nexusHeapNo = heapNo;
        if(count==0) {
            BDBG_ERR(("Can't find suitable place for heap %u(%u MBytes) on MEMC%u (0:%uMBytes 1:%uMBytes)", heapNo,(unsigned)(heap[heapNo].size/(1024*1024)),memc, map->map[memc][0]>=0 ? (unsigned)(osRegion[map->map[memc][0]].length/(1024*1024)):0, map->map[memc][1]>=0 ? (unsigned)(osRegion[map->map[memc][1]].length/(1024*1024)):0));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        i++;
    }
    candidates->count = i;
    candidates->permutations = 1;
    for(i=0;i<NEXUS_NUM_MEMC_REGIONS;i++) {
        if(map->map[memc][i]<0) { /* no such region */
            continue;
        }
        BDBG_MSG(("MEMC%u.%u %u(MBytes) at " BDBG_UINT64_FMT "", memc, i, (unsigned)(osRegion[map->map[memc][i]].length/(1024*1024)), BDBG_UINT64_ARG(osRegion[map->map[memc][i]].base)));
    }
    for(i=0;i<candidates->count;i++) {
        candidates->permutations *= candidates->heap[i].size;

#if BDBG_DEBUG_BUILD
        {
            char buf[128];
            NEXUS_MemoryStatus status;
            /* Must copy all members neeeded by NEXUS_Heap_ToString. NEXUS_MemoryStatus is not available at this time. */
            status.memcIndex = heap[candidates->heap[i].nexusHeapNo].memcIndex;
            status.heapType = heap[candidates->heap[i].nexusHeapNo].heapType;
            status.memoryType = heap[candidates->heap[i].nexusHeapNo].memoryType;
            NEXUS_Heap_ToString(&status, buf, sizeof(buf));
            BDBG_MSG(("MEMC%u heap:%u %uMBytes '%s' could be located in %u regions", memc, candidates->heap[i].nexusHeapNo, heap[candidates->heap[i].nexusHeapNo].size/(1024*1024), buf, candidates->heap[i].size));
        }
#endif
    }
    BDBG_MSG(("MEMC%u %u permutations for %u heaps", memc, candidates->permutations, candidates->count));
    return NEXUS_SUCCESS;
}

static int NEXUS_P_MemcPermutationMetric(const NEXUS_PlatformHeapSettings *heap, const struct NEXUS_P_MemcRegionCandidates *candidates, const struct NEXUS_P_MemcPermutation *permutation)
{
    int metric;
    unsigned i;
    for(metric=0,i=0;i<candidates->count;i++) {
        unsigned distance = permutation->offset[i];
        if(heap[candidates->heap[i].nexusHeapNo].memoryType & (NEXUS_MEMORY_TYPE_DRIVER_UNCACHED | NEXUS_MEMORY_TYPE_DRIVER_CACHED)) {
           distance *= candidates->count ; /* make it more costly to move heaps with the driver mapping */
        }
        metric += distance;
    }
    return metric;
}

static bool NEXUS_P_MemcTestPermutation(const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformOsRegion *osRegion, const struct NEXUS_P_MemcRegionOsMap *map, unsigned memc, struct NEXUS_P_MemcRegionCandidates *candidates, const struct NEXUS_P_MemcPermutation *permutation)
{
    unsigned i;
    size_t free[NEXUS_NUM_MEMC_REGIONS];
    size_t maxAlignment[NEXUS_NUM_MEMC_REGIONS];

    for(i=0;i<NEXUS_NUM_MEMC_REGIONS;i++) {
        if(map->map[memc][i]<0) {
            free[i] = 0;
        } else {
            free[i] = osRegion[map->map[memc][i]].length;
        }
        maxAlignment[i] = 1;
    }
    for(i=0;i<candidates->count;i++) {
        unsigned subIndex = permutation->subIndex[i];
        if(heap[candidates->heap[i].nexusHeapNo].alignment > maxAlignment[subIndex]) {
            maxAlignment[subIndex] = heap[candidates->heap[i].nexusHeapNo].alignment;
        }
    }

    for(i=0;i<candidates->count;i++) {
        size_t size;
        unsigned subIndex = permutation->subIndex[i];
        if(heap[candidates->heap[i].nexusHeapNo].size < 0) {
            size = -heap[candidates->heap[i].nexusHeapNo].size;
        } else {
            size = heap[candidates->heap[i].nexusHeapNo].size;
        }
        size = NEXUS_P_SizeAlign(size, maxAlignment[subIndex]);
        if(free[subIndex]<size) {
            BDBG_MSG(("MEMC%u.%u heap:%u (%u/%u) doesn't fit", memc, subIndex, candidates->heap[i].nexusHeapNo, i, candidates->count));
            return false;
        }
        free[subIndex] -= size;
    }
#if BDBG_DEBUG_BUILD
    BDBG_MSG(("MEMC%u %u heaps fit", memc, candidates->count));
    for(i=0;i<NEXUS_NUM_MEMC_REGIONS;i++) {
        if(map->map[memc][i]<0) {
            continue;
        }
        BDBG_MSG(("MEMC%u.%u left %u MBytes maxAlignment %u MBytes", memc, i, (unsigned)(free[i]/(1024*1024)), (unsigned)(maxAlignment[i]/(1024*1024)) ));
    }
#endif
    return true;
}


static NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings_allocateCma(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    unsigned i,j,region;
    NEXUS_Error rc;

    /* build nexus memory map*/
    for (i=0;i<NEXUS_MAX_HEAPS;i++)
    {
        int size = heap[i].size;

        if (!size) continue;

        BDBG_MODULE_MSG(nexus_platform_settings, ("request heap[%d]: MEMC%d/%d, size %d, %d", i, heap[i].memcIndex, state->subIndex[i],
                  heap[i].size, heap[i].memoryType));

        if (heap[i].memcIndex >= NEXUS_NUM_MEMC) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_index; }

        rc = nexus_platform_get_cma_index(pMemory, heap[i].memcIndex, state->subIndex[i], &j);
        if (rc) continue;
        region = state->cma_info[j].total_regions;
        state->cma_info[j].region_info[region].length = heap[i].size;
        state->cma_info[j].region_info[region].memcIndex = heap[i].memcIndex;
        state->cma_info[j].region_info[region].alignment = heap[i].alignment;
        state->cma_info[j].region_info[region].nexus_heap_index = i; /*save nexus heap index */
        state->cma_info[j].total_regions++;
    }

    /* build linux CMA map*/
    rc = NEXUS_Platform_P_GetCmaRegions(&state->linux_cma_info);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto err_region_info;}


    /* resolve all size = -1 */
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++)
    {
        unsigned j, dynamic_region = NEXUS_MAX_HEAPS, total = 0;
        for (j=0;j<state->cma_info[i].total_regions;j++) {
            if (state->cma_info[i].region_info[j].length == -1) {
                if (dynamic_region < NEXUS_MAX_HEAPS) {
                    BDBG_ERR(("only one size -1 heap allowed per region"));
                    rc =  BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    goto err_dynamic;
                }
                dynamic_region = j;
            }
            else {
                total += state->cma_info[i].region_info[j].length;
            }
        }
        if (dynamic_region < NEXUS_MAX_HEAPS) {
            state->cma_info[i].region_info[dynamic_region].length = pMemory->osRegion[i].length - total;
        }
    }

    /* compare linux and nexus cma maps*/
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++)
    {
        struct cma_dev *cma_dev = cma_dev_get_cma_dev(i);
        unsigned nexus_j, linux_j = 0;
        uint64_t used = 0;
        for (nexus_j=0;nexus_j<state->cma_info[i].total_regions;) {
            BDBG_MSG(("cma_info[%d].region_info[%d].length %d + used %u > linux_cma_info[%d].region_info[%d].length %d",
                i, nexus_j, (unsigned)state->cma_info[i].region_info[nexus_j].length, (unsigned)used, i, linux_j, (unsigned)state->linux_cma_info.entries[i].region_info[linux_j].length));
            if (linux_j == state->linux_cma_info.entries[i].total_regions) {
                break;
            }
            if (state->cma_info[i].region_info[nexus_j].length + used > state->linux_cma_info.entries[i].region_info[linux_j].length) {
                /* go to next region */
                if (++linux_j == state->linux_cma_info.entries[i].total_regions) break;
                used = 0;
                continue;
            }
            /* consume from this region */
            state->cma_info[i].region_info[nexus_j].physicalAddress = state->linux_cma_info.entries[i].region_info[linux_j].physicalAddress + used;
            used += state->cma_info[i].region_info[nexus_j].length;
            nexus_j++;
        }
        /* if not enough or too much cma memory, reset */
        if (linux_j == state->linux_cma_info.entries[i].total_regions || used < state->linux_cma_info.entries[i].region_info[linux_j].length) {
            if (state->linux_cma_info.entries[i].total_regions)
            {
                BDBG_WRN(("resetting all cma allocations in cma %d",i));
                cma_reset_all(i, cma_dev);
            }
            if (state->cma_info[i].total_regions)
            {
                rc = cma_alloc_regions(i, cma_dev, &state->cma_info[i]);
                if (rc) {rc = BERR_TRACE(rc); goto err_alloc_region;}
            }
        }
    }

    /*build nexus heap regions */
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++) {
        for (j=0;j<state->cma_info[i].total_regions;j++) {
            unsigned heap_index = state->cma_info[i].region_info[j].nexus_heap_index;
            /* if OS reports greater cache line size than BMEM's default, then increase */
            if (pMemory->max_dcache_line_size > pCoreSettings->heapRegion[heap_index].alignment) {
                pCoreSettings->heapRegion[heap_index].alignment = pMemory->max_dcache_line_size;
            }
            pCoreSettings->heapRegion[heap_index].offset =  state->cma_info[i].region_info[j].physicalAddress;
            pCoreSettings->heapRegion[heap_index].length =  state->cma_info[i].region_info[j].length;
            pCoreSettings->heapRegion[heap_index].memcIndex = state->cma_info[i].region_info[j].memcIndex;
            pCoreSettings->heapRegion[heap_index].memoryType = heap[heap_index].memoryType;
            pCoreSettings->heapRegion[heap_index].heapType = heap[heap_index].heapType;
        }
    }

    return NEXUS_SUCCESS;

err_region_info:
err_dynamic:
err_alloc_region:
err_index:
    return rc;
}


static NEXUS_Error NEXUS_Platform_P_Allocator_SelectRegion(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformOsRegion *osRegion, const NEXUS_PlatformHeapSettings *heap, bool testTrival)
{
    NEXUS_Error rc;
    unsigned i;
    /* Clear map from heap to subindex */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        state->subIndex[i] = -1;
    }

    for (i=0;i<NEXUS_MAX_MEMC;i++) { /* Execute allocation per MEMC */
        unsigned j;

        rc = NEXUS_Platform_P_BuildRegionCandidates(heap, osRegion, &state->osRegion, i, &state->candidates); /* find regions where each heap could be located */
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        state->best.metric=-1;
        for(j=0;j<state->candidates.permutations;j++) { /* iterate over all possible permutations */
            unsigned k;
            int metric;

            NEXUS_P_MemcGetPermutation( &state->candidates, j, &state->permutation);
            BDBG_MSG(("MEMC%u permutation %u", i, j));
            for(k=0;k<state->candidates.count;k++) {
                BDBG_MSG(("  heap %u -> MEMC%u.%u", state->candidates.heap[k].nexusHeapNo, i, state->permutation.subIndex[k]));
            }
            if( testTrival && state->candidates.permutations==1) {
                /* for real allocations and single permutation don't artificially test fitness, let allocation itself fail or succeed */
            } else if( !NEXUS_P_MemcTestPermutation(heap, osRegion, &state->osRegion, i, &state->candidates, &state->permutation)) {
                continue;
            }
            metric = NEXUS_P_MemcPermutationMetric(heap, &state->candidates, &state->permutation);
            BDBG_MSG(("permutation %u metric %d", j, metric));
            if(state->best.metric==-1 || metric < state->best.metric) {
                state->best.metric = metric;
                state->best.permutation = state->permutation;
            }
            if(metric<=1) { /* if there is close match, then terminate search earlier */
                break;
            }
        }
        if(state->best.metric!=-1) { /* on success, copy results */
            for(j=0;j<state->candidates.count;j++) {
                state->subIndex[state->candidates.heap[j].nexusHeapNo] = state->best.permutation.subIndex[j];
            }
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_SuggestBootParams(struct NEXUS_Platform_P_AllocatorState *state, const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformMemory *pMemory, const char *title)
{
    NEXUS_Error rc;
    unsigned vmalloc;

    BKNI_Memset(state, 0, sizeof(*state));

    rc = NEXUS_Platform_P_GetMemoryInfo(&state->bmem_hint.info);
    if(rc!=NEXUS_SUCCESS) {
        return;
    }
    rc = NEXUS_Platform_P_CalculateBootParams(state, heap, pMemory->max_dcache_line_size);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc);goto error;}

    vmalloc = NEXUS_Platform_P_GetVmallocSize(heap, &state->bmem_hint.info);
    if(vmalloc) {
        BKNI_Snprintf(state->bmem_hint.vmalloc_buf, sizeof(state->bmem_hint.vmalloc_buf),"vmalloc=%um ", vmalloc);
    } else {
        state->bmem_hint.vmalloc_buf[0]='\0';
    }

    NEXUS_Platform_P_PrintBmemBootOptions(state->bmem_hint.osMemory, title, state->bmem_hint.vmalloc_buf, state->bmem_hint.buf, sizeof(state->bmem_hint.buf));
    return;
error:
    BDBG_ERR(("Can't calculate %s boot configuration", title));
    return ;
}

static NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings_priv(const NEXUS_PlatformHeapSettings *heap, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings, bool allocateFromCma)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    struct NEXUS_Platform_P_AllocatorState *state;

    state = BKNI_Malloc(sizeof(*state));
    if(state==NULL) {rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    BKNI_Memset(state,0,sizeof(*state));
    NEXUS_Platform_P_Allocator_InitializeState(state, pMemory->osRegion, allocateFromCma);
    if(allocateFromCma) {
        /* Step 1. Dynamically assign heaps to regions */
#if defined(B_REFSW_ANDROID)
        if (!NEXUS_GetEnv("auto_subindex")) {
            for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                state->subIndex[i] = heap[i].subIndex;
            }
            goto done_subindex;
        }
#endif
        rc = NEXUS_Platform_P_Allocator_SelectRegion(state, pMemory->osRegion, heap, false);
        if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc);goto err_index;}

#if defined(B_REFSW_ANDROID)
done_subindex:
#endif /* B_REFSW_ANDROID */
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if(heap[i].size==0) {
                continue;
            }
            if(state->subIndex[i]==-1) {
                BDBG_WRN(("Can't allocate heap %u (%d bytes) on MEMC%u", i, heap[i].size, heap[i].memcIndex));
                if (!heap[i].optional) {
                    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
                        if(pMemory->osRegion[i].length==0) {
                            continue;
                        }
                        if(pMemory->osRegion[i].memcIndex!=heap[i].memcIndex) {
                            continue;
                        }
                        BDBG_LOG(("region[%u] MEMC%u.%u TOTAL:%u MBytes", i, pMemory->osRegion[i].memcIndex, pMemory->osRegion[i].subIndex, (unsigned)(pMemory->osRegion[i].length/(1024*1024))));
                    }
                    rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
                    goto err_index;
                }
            }
            BDBG_MSG(("heap %u (%d bytes) on MEMC%u.%d(%u)", i, heap[i].size, heap[i].memcIndex, state->subIndex[i], heap[i].subIndex));
        }
        rc = NEXUS_Platform_P_SetCoreCmaSettings_allocateCma(state, heap, pMemory, pCoreSettings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_alloc_region;}
    } else {
        /* BMEM allocator allocated from all regions at once */
        rc = NEXUS_Platform_P_SetCoreCmaSettings_allocateBmem(state, heap, pMemory, pCoreSettings);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_alloc_region;}
        if(state->extraMemory) {
            NEXUS_Platform_P_SuggestBootParams(state, heap, pMemory, "recommended");
        }
    }

    BKNI_Free(state);
    return NEXUS_SUCCESS;

err_alloc_region:
err_index:
    if(!allocateFromCma) {
        BDBG_LOG((" "));
        BDBG_LOG((" ")); /* second line */
        NEXUS_Platform_P_SuggestBootParams(state, heap, pMemory, "required");
        BDBG_LOG((" "));
        BDBG_LOG((" ")); /* second line */
    }
    BKNI_Free(state);
err_alloc:
    return rc;
}

NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings(const NEXUS_PlatformSettings *pSettings, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    NEXUS_Error rc;
    nexus_p_memory_info *info;

    bool useCma = pMemory->osRegion[0].cma;
    if(!useCma) {
        unsigned i;
        bool dynamicHeaps = false;
        bool pinnedHeaps = false;
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if(pSettings->heap[i].size==0) {
                continue;
            }
            if(pSettings->heap[i].offset) {
                pinnedHeaps = true;
            }
            if(pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) {
                dynamicHeaps = true;
            }
        }
        if(pinnedHeaps) { /* verify that all heaps are pinned */
            unsigned max_dcache_line_size = pMemory->max_dcache_line_size;
            for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                if(pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) {
                    continue;
                }
                if(pSettings->heap[i].size==0) {
                    continue;
                }
                if(pSettings->heap[i].size<0) {
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                }
                if(pSettings->heap[i].offset==0) {
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                }
                pCoreSettings->heapRegion[i].offset = pSettings->heap[i].offset;
                pCoreSettings->heapRegion[i].length = pSettings->heap[i].size;
                pCoreSettings->heapRegion[i].memcIndex = pSettings->heap[i].memcIndex;
                pCoreSettings->heapRegion[i].memoryType = pSettings->heap[i].memoryType;
                pCoreSettings->heapRegion[i].heapType = pSettings->heap[i].heapType;
                if (max_dcache_line_size > pCoreSettings->heapRegion[i].alignment) {
                    pCoreSettings->heapRegion[i].alignment = max_dcache_line_size;
                }
            }
        }
        if(!dynamicHeaps) { /* no dynamic heaps */
            if(!pinnedHeaps) {
                rc = NEXUS_Platform_P_SetCoreCmaSettings_priv(pSettings->heap, pMemory, pCoreSettings, false);
            } else {
                rc = NEXUS_SUCCESS;
            }
        } else {
            NEXUS_PlatformHeapSettings *heap;
            heap = BKNI_Malloc(sizeof(pSettings->heap));
            if(heap==NULL) { return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);}
            /* 1. Allocate not dynamic heaps in BMEM regions */
            for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                heap[i] = pSettings->heap[i];
                if(pSettings->heap[i].size==0) {
                    continue;
                }
                if(pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_DYNAMIC) {
                    heap[i].size = 0;
                }
            }
            if(!pinnedHeaps) {
                rc = NEXUS_Platform_P_SetCoreCmaSettings_priv(heap, pMemory, pCoreSettings, false);
                if(rc!=NEXUS_SUCCESS) {
                    BKNI_Free(heap);
                    return BERR_TRACE(rc);
                }
            }
            /* 2. Allocate dynamic heaps in CMA regions */
            for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                heap[i] = pSettings->heap[i];
                if(pSettings->heap[i].size==0) {
                    continue;
                }
                if(!(pSettings->heap[i].memoryType & NEXUS_MEMORY_TYPE_DYNAMIC)) {
                    heap[i].size = 0;
                }
            }
            rc = NEXUS_Platform_P_SetCoreCmaSettings_priv(heap, pMemory, pCoreSettings, true);
            BKNI_Free(heap);
        }
        /* XXX TODO need to test presence of dynamic heaps, and then get CMA copy of NEXUS_PlatformMemory and run NEXUS_Platform_P_SetCoreCmaSettings_priv for dynamic heaps */
    } else {
        rc = NEXUS_Platform_P_SetCoreCmaSettings_priv(pSettings->heap, pMemory, pCoreSettings, true);
    }
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    info = BKNI_Malloc(sizeof(*info));
    if(!info) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    rc = NEXUS_Platform_P_GetMemoryInfo(info);
    if(rc!=NEXUS_SUCCESS) {
        BKNI_Free(info);
        info = NULL;
    }
    rc = NEXUS_Platform_P_SetCoreCmaSettings_Verify(info, pSettings->heap, pCoreSettings, pMemory);
    if(info) {
        BKNI_Free(info);
    }
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Platform_P_SetCoreCmaSettings_Adjust(pSettings->heap, pCoreSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_GetHostCmaMemory(NEXUS_PlatformMemory *pMemory, unsigned cma_offset)
{
    unsigned i;
    unsigned subIndex[NEXUS_MAX_MEMC];

    /* 1. get all regions */
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++) {
        struct nexus_linux_cma_region_info region_info;
        int rc;
        struct cma_dev *cma_dev = cma_dev_get_cma_dev(i);
        if (!cma_dev) break;


        rc = cma_dev_get_phys_info(cma_dev, &region_info);
        if (rc) break;

        if(i+cma_offset >= NEXUS_MAX_HEAPS) {
            (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
            break;
        }

        pMemory->osRegion[i+cma_offset].base = region_info.physicalAddress;
        pMemory->osRegion[i+cma_offset].length = region_info.length;
        pMemory->osRegion[i+cma_offset].memcIndex = region_info.memcIndex;
        pMemory->osRegion[i+cma_offset].cma = true;
    }
    /* 2. sort entries by address */
    for(i=cma_offset;i<NEXUS_MAX_HEAPS;i++) {
        unsigned min = i;
        unsigned j;
        for(j=min+1;j<NEXUS_CMA_MAX_INDEX;j++) {
            if(pMemory->osRegion[j].length==0) {
                break;
            }
            if(pMemory->osRegion[min].base > pMemory->osRegion[j].base) {
                min = j;
            }
        }
        if(min!=i) {
            NEXUS_PlatformOsRegion tmp = pMemory->osRegion[i];
            pMemory->osRegion[i] = pMemory->osRegion[min];
            pMemory->osRegion[min] = tmp;
        }
    }

    /* 3. assign subIndex */
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        subIndex[i] = 0;
    }
    for(i=cma_offset;i<NEXUS_MAX_HEAPS;i++) {
        if(pMemory->osRegion[i].length==0) {
            break;
        }
        pMemory->osRegion[i].subIndex = subIndex[pMemory->osRegion[i].memcIndex];
        subIndex[pMemory->osRegion[i].memcIndex]++;

        BDBG_MODULE_MSG(nexus_platform_settings, ("cma.%u MEMC%u[%u] " BDBG_UINT64_FMT " " BDBG_UINT64_FMT "(%u MBytes)", i, pMemory->osRegion[i].memcIndex, pMemory->osRegion[i].subIndex, BDBG_UINT64_ARG(pMemory->osRegion[i].base), BDBG_UINT64_ARG(pMemory->osRegion[i].length), (unsigned)(pMemory->osRegion[i].length/(1024*1024))));
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_GetHostMemory(NEXUS_PlatformMemory *pMemory)
{
    NEXUS_Error rc;
    nexus_p_memory_info *info;

    info = BKNI_Malloc(sizeof(*info));
    if(!info) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    rc = NEXUS_Platform_P_GetMemoryInfo(info);
    if(rc==NEXUS_SUCCESS) {
        int i;
        for(i=0;i<info->lowmem.count;i++) {
            BDBG_MSG(("MEMC%d lowmem %uMBytes(%u) at " BDBG_UINT64_FMT, NEXUS_Platform_P_GetMemcForRange(info, info->lowmem.range[i].addr, info->lowmem.range[i].size), (unsigned)(info->lowmem.range[i].size/(1024*1024)), (unsigned)info->lowmem.range[i].size, BDBG_UINT64_ARG(info->lowmem.range[i].addr)));
        }
        for(i=0;i<info->bmem.count;i++) {
            int j;
            BDBG_MSG(("MEMC%d BMEM %uMBytes(%u) at " BDBG_UINT64_FMT, NEXUS_Platform_P_GetMemcForRange(info, info->bmem.range[i].addr, info->bmem.range[i].size), (unsigned)(info->bmem.range[i].size/(1024*1024)), (unsigned)info->bmem.range[i].size, BDBG_UINT64_ARG(info->bmem.range[i].addr)));
            for(j=0;j<info->lowmem.count;j++) {
                if(NEXUS_Platform_P_RangeTestIntersect(&info->lowmem.range[j], info->bmem.range[i].addr, info->bmem.range[i].size)) {
                    BDBG_ERR(("MEMC%d BMEM %uMBytes(%u) at " BDBG_UINT64_FMT " intersects with lowmem %uMBytes(%u) at " BDBG_UINT64_FMT, NEXUS_Platform_P_GetMemcForRange(info, info->bmem.range[i].addr, info->bmem.range[i].size), (unsigned)(info->bmem.range[i].size/(1024*1024)), (unsigned)info->bmem.range[i].size, BDBG_UINT64_ARG(info->bmem.range[i].addr), (unsigned)(info->lowmem.range[j].size/(1024*1024)), (unsigned)info->lowmem.range[j].size, BDBG_UINT64_ARG(info->lowmem.range[j].addr)));
                }
            }
        }
        /* sort bmem regions since pMemory->osRegion expects sorted regions, use naive bubble sort */
        for(i=0;i<info->bmem.count;i++) {
            int j;
            for(j=i+1;j<info->bmem.count;j++) {
                if(info->bmem.range[i].addr > info->bmem.range[j].addr) {
                    nexus_p_memory_range temp;
                    BDBG_WRN(("swap bmem[%u]" BDBG_UINT64_FMT ":%u with bmem[%u]" BDBG_UINT64_FMT ":%u", i, BDBG_UINT64_ARG(info->bmem.range[i].addr), (unsigned)info->bmem.range[i].size, j, BDBG_UINT64_ARG(info->bmem.range[j].addr), (unsigned)info->bmem.range[j].size));
                    temp = info->bmem.range[i];
                    info->bmem.range[i] = info->bmem.range[j];
                    info->bmem.range[j] = temp;
                }
            }
        }
        rc = NEXUS_Platform_P_SetHostBmemMemoryFromInfo(info, pMemory->osRegion);
    }
    BKNI_Free(info);
    if(rc==NEXUS_SUCCESS) {
        unsigned bmem_entries;
        char buf[128];

        for(bmem_entries=0;bmem_entries<NEXUS_MAX_HEAPS;bmem_entries++) {
            if (pMemory->osRegion[bmem_entries].length==0) {
                break;
            }
        }
        rc = NEXUS_Platform_P_GetHostCmaMemory(pMemory,bmem_entries);
        NEXUS_Platform_P_PrintBmemBootOptions(pMemory->osRegion, "current", "", buf, sizeof(buf));
        if(bmem_entries!=0) {
            return rc;
        }
    } else if(rc!=NEXUS_NOT_SUPPORTED) {
        return BERR_TRACE(rc);
    }
    /* rely on CMA data */
    rc = NEXUS_Platform_P_GetHostCmaMemory(pMemory,0);
    return rc;
}


/* the equivalent of "cmatool listall" */
static void NEXUS_Platform_P_PrintCma(const NEXUS_PlatformMemory *pMemory)
{
    unsigned i, j;
    for (i=0;i<NEXUS_CMA_MAX_INDEX;i++) {
        struct cma_dev *cma_dev = cma_dev_get_cma_dev(i);
        unsigned num_regions;
        if (!cma_dev || !pMemory->osRegion[i].length) continue;
        num_regions = cma_dev_get_num_regions(cma_dev);
        BDBG_LOG(("cma.%u  " BDBG_UINT64_FMT " " BDBG_UINT64_FMT " (MEMC%d/%d)", i, BDBG_UINT64_ARG(pMemory->osRegion[i].base), BDBG_UINT64_ARG(pMemory->osRegion[i].length), pMemory->osRegion[i].memcIndex, pMemory->osRegion[i].subIndex));
        for (j=0; j<num_regions; j++) {
            int rc;
            unsigned memc=0;
            NEXUS_Addr addr=0;
            uint64_t len=0;
            rc = cma_dev_get_region_info(cma_dev, j, &memc, &addr, &len);
            if (rc) continue;
            BDBG_LOG((" alloc " BDBG_UINT64_FMT " " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(addr), BDBG_UINT64_ARG(len)));
        }
    }
}

NEXUS_Addr NEXUS_Platform_P_AllocCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, unsigned size, unsigned alignment)
{
    NEXUS_Addr addr=0;
    int rc;
    unsigned cma_index=0;
    struct cma_dev *cma_dev;

    rc = nexus_platform_get_cma_index(pMemory, memcIndex, subIndex, &cma_index);
    if (rc) {BERR_TRACE(rc); return 0;}
    cma_dev = cma_dev_get_cma_dev(cma_index);
    if (!cma_dev) {BERR_TRACE(-1); return 0;}

    rc = cma_dev_get_mem(cma_dev, &addr, size, alignment);
    if (rc) {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        BDBG_ERR(("unable to alloc %d bytes (%#x aligned) from cma.%d (MEMC%d/%d)", size, alignment, cma_index, memcIndex, subIndex));
        NEXUS_Platform_P_PrintCma(pMemory);
        return 0;
    }
    return addr;
}

void NEXUS_Platform_P_FreeCma(const NEXUS_PlatformMemory *pMemory, unsigned memcIndex, unsigned subIndex, NEXUS_Addr addr, unsigned size)
{
    int rc;
    unsigned cma_index=0;
    struct cma_dev *cma_dev;

    rc = nexus_platform_get_cma_index(pMemory, memcIndex, subIndex, &cma_index);
    if (rc) {BERR_TRACE(rc); return;}
    cma_dev = cma_dev_get_cma_dev(cma_index);
    if (!cma_dev) {BERR_TRACE(-1); return;}

    (void)cma_dev_put_mem(cma_dev, addr, size);
}

NEXUS_Error NEXUS_Platform_P_CalcSubMemc(const NEXUS_Core_PreInitState *preInitState, BCHP_MemoryLayout *pMemory)
{
    NEXUS_Error rc;
    nexus_p_memory_info info;
    unsigned i, j;

    rc = NEXUS_Platform_P_GetMemoryInfo(&info);
    if (rc) {
        BCHP_MemoryInfo memoryInfo;
        BCHP_GetMemoryInfo_PreInit(preInitState->hReg, &memoryInfo);
        /* Running on Linux 3.14-1.7 and earlier (no bmem) requires emulating Linux's report of memory layout. */
        BKNI_Memset(&info, 0, sizeof(info));
        info.memc[0].range[0].size = memoryInfo.memc[0].size;
    }

    BDBG_CASSERT(BCHP_MAX_MEMC_REGIONS <= NEXUS_NUM_MEMC_REGIONS);
    BDBG_CASSERT(NEXUS_MAX_MEMC <= sizeof(pMemory->memc)/sizeof(pMemory->memc[0]));
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        for (j=0;j<BCHP_MAX_MEMC_REGIONS && j<(unsigned)info.memc[i].count;j++) {
            pMemory->memc[i].region[j].addr = info.memc[i].range[j].addr;
            pMemory->memc[i].region[j].size = info.memc[i].range[j].size;
            pMemory->memc[i].size += info.memc[i].range[j].size;
        }
    }
    return NEXUS_SUCCESS;
}
#endif /* NEXUS_USE_CMA */
