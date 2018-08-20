/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#include <cstddef>
#include <cstdint>

#include "libfdt.h"
#include "lib_printf.h"

#include "tzmemory.h"
#include "kernel.h"
#include "parse_utils.h"
#include "config.h"
#include "platform.h"

// Allocate the static non-const data from TzMem class.
int TzMem::freePagesStart;
int TzMem::freePagesEnd;
TzMem::PageFrame TzMem::pageFrames[TzMem::TZ_MAX_NUM_PAGES];
int TzMem::allocPageMap[TzMem::TZ_MAX_NUM_PAGES];
int TzMem::freePageMap[TzMem::TZ_MAX_NUM_PAGES];
TzMem::RangeFrame TzMem::rangeFrames[TzMem::TZ_MAX_NUM_RANGES];
int TzMem::numRanges;
SpinLock TzMem::lock;

uintptr_t TzMem::mrrHeapStart;
uintptr_t TzMem::mrrHeapEnd;

uintptr_t TzMem::krrHeapStart;
uintptr_t TzMem::krrHeapEnd;

uintptr_t TzMem::crrHeapStart;
uintptr_t TzMem::crrHeapEnd;

uint32_t TzMem::secRegion;

void TzMem::init(void *devTree) {

    spinLockInit(&lock);

    numRanges = 0;

    /* Parse the 'Memory' node in the device tree and determine our range of usable memory.
     *
     * Example node:
     *
     * memory {
     *
     *     #address-cells = <0x1>; // This property is the # of address cells for all sub-nodes
     *     #size-cells = <0x1>;    // This property is the # of size cells for all sub-nodes
     *     device_type = "memory";
     *     reg = <0x0 0x0 0x0 0x40000000>;
     *     ...
     * }
     *
     * Note: The "reg" property of memory node uses #address-cells and #size-cells of the parent node.
     */

    TzMem::mrrHeapStart = 0;
    TzMem::mrrHeapEnd   = 0;
    TzMem::krrHeapStart = 0;
    TzMem::krrHeapEnd   = 0;
    TzMem::crrHeapStart = 0;
    TzMem::crrHeapEnd   = 0;
    TzMem::secRegion    = 0;

    int nodeOffset = fdt_subnode_offset(devTree, 0, "memory");
    if (nodeOffset < 0) {
        kernelHalt("No memory node found in the device tree");
    }

    // Parse the #address-cells property of the parent node.
    int parentOffset = fdt_parent_offset(devTree, nodeOffset);
    int propLen;
    unsigned long addrCellSize;
    const struct fdt_property *fpAddrCells =
        fdt_get_property(devTree, parentOffset, "#address-cells", &propLen);
    if ((!fpAddrCells) || (propLen < sizeof(int)))
        addrCellSize = 1;
    else
        addrCellSize = parseInt((void *)fpAddrCells->data, propLen);
    int addrByteSize = addrCellSize * sizeof(int);

    // Parse the #size-cells property.
    unsigned long szCellSize;
    const struct fdt_property *fpSzCells =
        fdt_get_property(devTree, parentOffset, "#size-cells", &propLen);
    if ((!fpSzCells) || (propLen < sizeof(int)))
        szCellSize = 1;
    else
        szCellSize = parseInt((void *)fpSzCells->data, propLen);
    int szByteSize = szCellSize * sizeof(int);

    // Parse the 'reg' property
    const struct fdt_property *fpMemRanges =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);
    if (!fpMemRanges)
        kernelHalt("No valid memory range found in device tree memory node.");
    if (propLen < addrByteSize + szByteSize)
        kernelHalt("Incomplete memory range found in device tree memory node.");

    int rangeNum = propLen / (addrByteSize + szByteSize);
    if (rangeNum > TZ_MAX_NUM_RANGES)
        kernelHalt("Number of memory ranges exceeds limit.");

    const char *rangeData = fpMemRanges->data;

    if(rangeNum > 1)
        printf("%s Error: Detected more than one memory range in DT\n",__FUNCTION__);

    unsigned long rangeStart = (unsigned long)
        ((addrCellSize == 1) ?
         parseInt(rangeData, addrByteSize) :
         parseInt64(rangeData, addrByteSize));
    rangeData += addrByteSize;

    unsigned long rangeSize = (unsigned long)
        ((szCellSize == 1) ?
         parseInt(rangeData, szByteSize) :
         parseInt64(rangeData, szByteSize));
    rangeData += szByteSize;

    printf("DT memory range 0x%x, size 0x%x\n",
           (unsigned int)rangeStart, (unsigned int)rangeSize);

    /* MRR and KRR from DT */
    mrrHeapStart = rangeStart & PAGE_MASK;
    mrrHeapEnd   = mrrHeapStart + rangeSize;
    krrHeapStart = rangeStart & PAGE_MASK;
    krrHeapEnd   = krrHeapStart + rangeSize;
    addRange(krrHeapStart, krrHeapEnd-krrHeapStart);

    printf("TzMem init done\n");
}

// Kernel image boundaries from the linker script.
extern unsigned long _start, _end;

void TzMem::addRange(const unsigned long startAddr, const unsigned long size) {

    uint8_t *currPage = (uint8_t *)PAGE_START_4K(startAddr + PAGE_SIZE_4K_BYTES - 1);
    uint8_t *lastPage = (uint8_t *)PAGE_START_4K(startAddr) + size;

    // Linker script is setup such that the image starts at a 4K page boundary.
    unsigned long tzImageStart = (unsigned long)&_start;
    tzImageStart = (unsigned long)virtToPhys((void *)tzImageStart);

    // Linker script is setup such that _end points to 4K page beyond end of image.
    unsigned long tzImageEnd = (unsigned long)&_end;
    tzImageEnd = (unsigned long)virtToPhys((void *)tzImageEnd);

    // printf("Adding range %d, currPage %p lastPage %p\n"
    //        "\ttzImageStart 0x%lx tzImageEnd 0x%lx\n",
    //        numRanges, currPage, lastPage, tzImageStart, tzImageEnd);

    if (numRanges == 0) {
        freePagesStart = 0;
        freePagesEnd = 0;
    }

    RangeFrame *rf = &rangeFrames[numRanges];
    rf->startPageNum = pageFrameNum(currPage);

    uint32_t mapIdx = pageMapIdx(currPage);
    while (currPage < lastPage) {
        // Skip pages from tzImageStart to tzImageEnd
        if (currPage >= (uint8_t *)tzImageStart &&
            currPage <  (uint8_t *)tzImageEnd) {

            allocPageMap[mapIdx] = PAGE_NOT_ALLOCATED;
            freePageMap[mapIdx] = PAGE_NOT_FREE;

            mapIdx++;
            currPage = currPage + PAGE_SIZE_4K_BYTES;
            continue;
        }

        if (freePagesEnd == TZ_MAX_NUM_PAGES)
            kernelHalt("TZ_MAX_NUM_PAGES needs to increase !");

        pageFrames[freePagesEnd].pageNum = pageFrameNum(currPage);
        pageFrames[freePagesEnd].owner = KERNEL_PID;

        allocPageMap[mapIdx] = PAGE_NOT_ALLOCATED;
        freePageMap[mapIdx] = freePagesEnd;

        // printf("Added page frame %d, page num %ld [ %p ]\n",
        //        freePagesEnd, pageFrames[freePagesEnd].pageNum, currPage);

        freePagesEnd++;
        mapIdx++;
        currPage = currPage + PAGE_SIZE_4K_BYTES;
    }

    rf->endPageNum = pageFrameNum(currPage);

    // printf("Added range %d, page num %ld - %ld\n",
    //        numRanges, rf->startPageNum, rf->endPageNum);
    numRanges++;

    // protect memory range
    //printf("memProtect [0x%lx - 0x%lx] \n", startAddr, startAddr + size - 1);
    //Platform::memProtect((void *)startAddr, (void *)(startAddr + size - 1));
}

void TzMem::freeInitRamFS(VirtAddr vaStart, VirtAddr vaEnd) {

    PhysAddr paStart = virtToPhys(vaStart);
    PhysAddr paEnd =  virtToPhys(vaEnd);

    uint8_t *currPage = (uint8_t *)PAGE_START_4K(paStart);
    uint8_t *lastPage = (uint8_t *)PAGE_START_4K(paEnd);

    RangeFrame *rf = &rangeFrames[numRanges];
    rf->startPageNum = pageFrameNum(currPage);

    uint32_t mapIdx = pageMapIdx(currPage);
    while (currPage < lastPage) {
        if (freePagesEnd == TZ_MAX_NUM_PAGES)
            kernelHalt("TZ_MAX_NUM_PAGES needs to increase !");

        pageFrames[freePagesEnd].pageNum = pageFrameNum(currPage);
        pageFrames[freePagesEnd].owner = KERNEL_PID;

        allocPageMap[mapIdx] = PAGE_NOT_ALLOCATED;
        freePageMap[mapIdx] = freePagesEnd;

        // printf("Added page frame %d, page num %ld [ %p ]\n",
        //        freePagesEnd, pageFrames[freePagesEnd].pageNum, currPage);

        freePagesEnd++;
        mapIdx++;
        currPage = currPage + PAGE_SIZE_4K_BYTES;
    }
    rf->endPageNum = pageFrameNum(currPage);
    numRanges++;
}

TzMem::PhysAddr TzMem::_allocPage(int pid) {

    SpinLocker locker(&lock);

    if (freePagesStart == freePagesEnd) {
        err_msg("%s: Ran out of pages.\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    PageFrame *pf = &pageFrames[freePagesStart];
    pf->owner = pid;
    PhysAddr page = frameNumToPage(pf->pageNum);

    uint32_t mapIdx = pageMapIdx(page);
    allocPageMap[mapIdx] = freePagesStart;
    freePageMap[mapIdx] = PAGE_NOT_FREE;

    freePagesStart++;
    return page;
}

int TzMem::_freePage(PhysAddr page) {

    SpinLocker locker(&lock);

    uint32_t mapIdx = pageMapIdx(page);
    if (allocPageMap[mapIdx] == PAGE_NOT_ALLOCATED) {
        err_msg("%s: Attempted free of un-allocated page %p, mapIdx = 0x%x\n",
                __PRETTY_FUNCTION__, page, (unsigned int)mapIdx);
        kernelHalt("Mismatched page free");
        return KERNEL_PID;
    }

    uint32_t pfIdx = allocPageMap[mapIdx];

    freePagesStart--; // Pointing to the end of allocated frame list.

    // If the page frame to be freed is not at the end of allocated frame list,
    // swap the page frame with the one at the end of allocated frame list.
    if (pfIdx != freePagesStart) {
        swapPageFrames(&pageFrames[pfIdx], &pageFrames[freePagesStart]);

        // Make the associated maps pointing to the moved page frame indices
        PhysAddr movedPage = frameNumToPage(pageFrames[pfIdx].pageNum);
        uint32_t movedMapIdx = pageMapIdx(movedPage);

        allocPageMap[movedMapIdx] = pfIdx;
        freePageMap[movedMapIdx] = PAGE_NOT_FREE;
    }

    int pid = pageFrames[freePagesStart].owner;
    pageFrames[freePagesStart].owner = KERNEL_PID;
    allocPageMap[mapIdx] = PAGE_NOT_ALLOCATED;
    freePageMap[mapIdx] = freePagesStart;
    return pid;
}

TzMem::PhysAddr TzMem::allocContiguousPages(int numPages, int pid) {

    SpinLocker locker(&lock);

    // Single scan algorithm.
    int freeStart = -1;
    int freeCount = 0;
    uint32_t mapIdx = 0;
    for (int i = 0; i < numRanges; i++) {
        RangeFrame *rf = &rangeFrames[i];

        for (int j = rf->startPageNum; j < rf->endPageNum; j++) {
            if (freePageMap[mapIdx] == PAGE_NOT_FREE) {
                freeStart = -1;
                freeCount = 0;
            }
            else {
                if (freeStart == -1)
                    freeStart = mapIdx;
                if (++freeCount == numPages)
                    goto SCAN_DONE;
            }
            mapIdx++;
        }

        // Need to restart scan across range boundaries.
        freeStart = -1;
        freeCount = 0;
    }

 SCAN_DONE:
    if ((freeStart == -1)||(freeCount < numPages)) {
        err_msg("%s: Could not find %d contiguous pages. Found %d\n",
                __PRETTY_FUNCTION__, numPages, freeCount);
        return nullptr;
    }

    for (mapIdx = freeStart; mapIdx < freeStart + numPages; mapIdx++) {

        uint32_t pfIdx = freePageMap[mapIdx];

        // If the page frame to be allocated is not at the head of free frame list,
        // swap the page frame the one at the head of the free frame list.
        if (pfIdx != freePagesStart) {
            swapPageFrames(&pageFrames[pfIdx], &pageFrames[freePagesStart]);

            // Make the associated maps pointing to the moved page frame indices
            PhysAddr movedPage = frameNumToPage(pageFrames[pfIdx].pageNum);
            uint32_t movedMapIdx = pageMapIdx(movedPage);

            allocPageMap[movedMapIdx] = PAGE_NOT_ALLOCATED;
            freePageMap[movedMapIdx] = pfIdx;
        }

        pageFrames[freePagesStart].owner = pid;
        allocPageMap[mapIdx] = freePagesStart;
        freePageMap[mapIdx] = PAGE_NOT_FREE;

        freePagesStart++;
    }

    uint32_t pfIdx = allocPageMap[freeStart];
    PhysAddr page = frameNumToPage(pageFrames[pfIdx].pageNum);
    return page;
}

void TzMem::freeContiguousPages(PhysAddr firstPage, int numPages) {

    PhysAddr page = firstPage;
    for (int i = 0; i < numPages; i++) {
        freePage(page);
        page = (uint8_t *)page + PAGE_SIZE_4K_BYTES;
    }
}

#if MEMORY_DEBUG
void TzMem::dumpRangeFrames() {

    printf("\n");

    printf("Num of ranges %d\n", numRanges);
    for (int i = 0; i < numRanges; i++) {
        RangeFrame *rf = &rangeFrames[i];
        printf("\tRange frame %d: startPageNum 0x%x, endPageNum 0x%x\n",
               i, (unsigned int)rf->startPageNum, (unsigned int)rf->endPageNum);
    }
    printf("\n");
}

void TzMem::dumpPageFrames(int numPages) {

    printf("\n");

    printf("Free pages start 0x%x\n", freePagesStart);
    printf("Free pages end 0x%x\n", freePagesEnd);

    printf("Start of page frames:\n");
    for (int i = 0; i < numPages; i++) {
        if (i >= freePagesEnd) break;

        PageFrame *pf = &pageFrames[i];
        printf("\tPage frame %d: pageNum 0x%x, owner %d\n",
               i, (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }

    printf("Before free pages start:\n");
    for (int i = freePagesStart - numPages; i < freePagesStart; i++) {
        if (i < 0) continue;

        PageFrame *pf = &pageFrames[i];
        printf("\tPage frame %d: pageNum 0x%x, owner %d\n",
               i, (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }

    printf("After free pages start:\n");
    for (int i = freePagesStart; i < freePagesStart + numPages; i++) {
        if (i >= freePagesEnd) break;

        PageFrame *pf = &pageFrames[i];
        printf("\tPage frame %d: pageNum 0x%x, owner %d\n",
               i, (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }

    printf("End of pages frames:\n");
    for (int i = freePagesEnd - numPages; i < freePagesEnd; i++) {
        if (i < 0) continue;

        PageFrame *pf = &pageFrames[i];
        printf("\tPage frame %d: pageNum 0x%x, owner %d\n",
               i, (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }

    printf("\n");
}

void TzMem::dumpPageMaps(int numPages) {

    printf("\n");

    printf("Start of page maps:\n");
    for (int i = 0; i < numPages; i++) {
        if (i >= TZ_MAX_NUM_PAGES) break;

        printf("\tPage maps 0x%x: alloc %d, free %d", (unsigned int)i,
               (unsigned int)allocPageMap[i], (unsigned int)freePageMap[i]);

        uint32_t pfIdx = 0;
        if (allocPageMap[i] != PAGE_NOT_ALLOCATED)
            pfIdx = allocPageMap[i];
        else if (freePageMap[i] != PAGE_NOT_FREE)
            pfIdx = freePageMap[i];
        else {
            printf("\n");
            continue;
        }

        PageFrame *pf = &pageFrames[pfIdx];
        printf(", pageNum 0x%x, owner %d\n",
               (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }

    unsigned long tzImageEnd = (unsigned long)&_end;
    tzImageEnd = (unsigned long) virtToPhys((void *)tzImageEnd);

    uint32_t startMapIdx = pageMapIdx((PhysAddr)tzImageEnd);

    printf("After TZ image end:\n");
    for (int i = startMapIdx; i < startMapIdx + numPages; i++) {
        if (i >= TZ_MAX_NUM_PAGES) break;

        printf("\tPage maps 0x%x: alloc %d, free %d", (unsigned int)i,
               (unsigned int)allocPageMap[i], (unsigned int)freePageMap[i]);

        uint32_t pfIdx = 0;
        if (allocPageMap[i] != PAGE_NOT_ALLOCATED)
            pfIdx = allocPageMap[i];
        else if (freePageMap[i] != PAGE_NOT_FREE)
            pfIdx = freePageMap[i];
        else {
            printf("\n");
            continue;
        }

        PageFrame *pf = &pageFrames[pfIdx];
        printf(", pageNum 0x%x, owner %d\n",
               (unsigned int)pf->pageNum, (unsigned int)pf->owner);
    }
    printf("\n");
}
#endif /* MEMORY_DEBUG */
