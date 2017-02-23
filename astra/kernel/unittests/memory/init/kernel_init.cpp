/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/


#include <stdint.h>

#include "arm/arm.h"
#include "config.h"

#include "lib_printf.h"
#include "libfdt.h"
#include "parse_utils.h"

#include "kernel.h"
#include "objalloc.h"
#include "mempool.h"
#include "scheduler.h"
#include "system.h"

extern "C" void tzKernelSecondary();

extern uint8_t tzDevTree[MAX_DT_SIZE_BYTES];
static unsigned long tzMemSize = 0;
static unsigned long tzImgSize = 0;

extern unsigned long _start, _end;

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) {} }

void setupPageAllocTests(void *devTree) {

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
        szCellSize = parseInt((void *)fpAddrCells->data, propLen);
    int szByteSize = szCellSize * sizeof(int);

    // Parse the 'reg' property
    const struct fdt_property *fpMemRanges =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);
    if (!fpMemRanges)
        kernelHalt("No valid memory range found in device tree memory node.");
    if (propLen < addrByteSize + szByteSize)
        kernelHalt("Incomplete memory range found in device tree memory node.");

    int rangeNum = propLen / (addrByteSize + szByteSize);
    if (rangeNum > TzMem::TZ_MAX_NUM_RANGES)
        kernelHalt("Number of memory ranges exceeds limit.");

    const char *rangeData = fpMemRanges->data;

    for (int i = 0; i < rangeNum; i++) {
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

        //printf("Adding memory range 0x%x, size 0x%x\n",
        //       (unsigned int)rangeStart, (unsigned int)rangeSize);
        tzMemSize += rangeSize;
        UNUSED(rangeStart);
    }

    tzImgSize = (unsigned long)&_end - (unsigned long)&_start;
}

static bool hasDuplicates(TzMem::PhysAddr *pages, int numPages) {
    UNUSED(pages); UNUSED(numPages);
    return false;

    for (int i=0; i<numPages; i++) {
        for (int j=0; j<numPages; j++) {
            if (j == i)
                continue;

            if (pages[j] == pages[i])
                return true;
        }
    }
    return false;
}

static TzMem::PhysAddr allocPages[TzMem::TZ_MAX_NUM_PAGES];

void runPageAllocTests() {

    /*
     * Validate the amount of memory available to the memory manager
     */
    int availMemSize = tzMemSize - tzImgSize;
    int nfPages = TzMem::numFreePages();
    //printf("numFreePages %d expected %d\n", nfPages, availMemSize/PAGE_SIZE_4K_BYTES);
    assert(nfPages*PAGE_SIZE_4K_BYTES == availMemSize);
    success_msg("Mem size test passed\n");

    // Allocate all the available pages
    for (int i=0; i<nfPages; i++) {
        TzMem::PhysAddr paddr = TzMem::allocPage(0);
        assert(paddr != nullptr);

        allocPages[i] = paddr;
    }
    if (hasDuplicates(allocPages, nfPages)) {
        err_msg("Max alloc test failed: Duplicate page allocations detected\n");
        return;
    }
    success_msg("Max alloc test passed\n");

    // Free all the allocated pages
    for (int i=0; i<nfPages; i++)
        TzMem::freePage(allocPages[i]);
    assert(TzMem::numFreePages() == nfPages);
    success_msg("Linear free all test passed\n");

    // Allocate all the available pages, free half of them and re-allocate them.
    for (int i=0; i<nfPages; i++) {
        TzMem::PhysAddr paddr = TzMem::allocPage(0);
        assert(paddr != nullptr);

        allocPages[i] = paddr;
    }
    if (hasDuplicates(allocPages, nfPages)) {
        err_msg("Free Plus Realloc test failed: Duplicate page allocations detected\n");
        return;
    }
    for (int i=0; i<nfPages; i+=2) {
        TzMem::freePage(allocPages[i]);
    }
    for (int i=0; i<nfPages; i+=2) {
        allocPages[i] = TzMem::allocPage(0);
        assert(allocPages[i] != nullptr);
    }
    if (hasDuplicates(allocPages, nfPages)) {
        err_msg("Free Plus Realloc test failed: Duplicate page allocations detected\n");
        return;
    }
    success_msg("Free plus realloc test passed\n");

    for (int i=0; i<nfPages; i++)
        TzMem::freePage(allocPages[i]);
    for (int i=0; i<nfPages/2; i+=16) {
        allocPages[i] = TzMem::allocContiguousPages(16, 0);
        assert(allocPages[i] != nullptr);
    }
    success_msg("Contiguous alloc test passed\n");

    for (int i=0; i<nfPages/2; i+=16) {
        TzMem::freeContiguousPages(allocPages[i], 16);
    }
    success_msg("Contiguous free test passed\n");

    for (int i=0; i<100000; i++) {
        TzMem::PhysAddr page = TzMem::allocPage(i);
        TzMem::freePage(page);
    }
    success_msg("Repeated alloc-free test passed\n");
}

void runPageTableTests() {

    PageTable *kernelPageTable = PageTable::kernelPageTable();

    // Find unmapped pages just below start of kernel code
    TzMem::VirtAddr stackPageStart = kernelPageTable->reserveAddrRange((void *)KERNEL_STACKS_START, PAGE_SIZE_4K_BYTES*4, PageTable::ScanBackward);
    TzMem::VirtAddr stackPageLast = (TzMem::VirtAddr)((unsigned long)stackPageStart + (PAGE_SIZE_4K_BYTES*4));
    printf("Located an unmapped virtual address range %p - %p for stack\n", stackPageStart, stackPageLast);

    TzMem::VirtAddr heapPageStart = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES*16, PageTable::ScanForward);
    TzMem::VirtAddr heapPageLast = (TzMem::VirtAddr)((unsigned long)heapPageStart + (PAGE_SIZE_4K_BYTES*16));
    printf("Located an unmapped virtual address range %p - %p for heap\n", heapPageStart, heapPageLast);

    //
    // Mix up the page allocator list of free pages. We want them to become non-contiguous
    //
    for (int i=0; i<500; i++) {
        TzMem::PhysAddr paddr = TzMem::allocPage(0);
        assert(paddr != nullptr);

        allocPages[i] = paddr;
    }
    for (int i=0; i<500; i++) {
        TzMem::freePage(allocPages[i]);
    }

    // Allocate physical memory for the stack and map it
    TzMem::VirtAddr stackCurr = stackPageStart;
    for (int i=0; i<4; i++) {
        TzMem::PhysAddr addr = TzMem::allocPage(0);
        if (addr == nullptr) {
            err_msg("%s: Could not allocate stack page\n", __FUNCTION__);
            return;
        }

        kernelPageTable->mapPage(stackCurr, addr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);
        stackCurr = (uint8_t *)stackCurr + PAGE_SIZE_4K_BYTES;
    }

    uint32_t *curr = (uint32_t *)stackPageStart;
    uint32_t *last = curr + ((PAGE_SIZE_4K_BYTES * 4)/4) - 1;
    while (curr != last) {
        *curr = 0xcafebabe;
        curr++;
    }
    curr = (uint32_t *)stackPageStart;
    while (curr != last) {
        if (*curr != 0xcafebabe) {
            err_msg("%s: stack read back curr %p failed.", __FUNCTION__, curr);
        }
        curr++;
    }
    success_msg("Stack test passed\n");

    // Allocate physical memory for the heap and map it
    TzMem::VirtAddr heapCurr = heapPageStart;
    for (int i=0; i<16; i++) {
        TzMem::PhysAddr addr = TzMem::allocPage(0);
        if (addr == nullptr) {
            err_msg("%s: Could not allocate heap page\n", __FUNCTION__);
            return;
        }

        kernelPageTable->mapPage(heapCurr, addr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);
        heapCurr = (uint8_t *)heapCurr + PAGE_SIZE_4K_BYTES;
    }

    curr = (uint32_t *)heapPageStart;
    last = curr + ((PAGE_SIZE_4K_BYTES * 16)/4) - 1;
    while (curr != last) {
        *curr = 0xcafebabe;
        curr++;
    }
    curr = (uint32_t *)heapPageStart;
    while (curr != last) {
        if (*curr != 0xcafebabe) {
            err_msg("%s: heap read back curr %p failed.", __FUNCTION__, curr);
        }
        curr++;
    }

    success_msg("Heap test passed\n");
}

void runMallocTests() {

    struct TestStruct {
        unsigned long ulA[32];
    };

    ObjCacheAllocator<TestStruct> allocator;
    static TestStruct *objects[1024];

    //printf("starting malloc tests\n");

    allocator.init();

    for (int i=0; i<128; i++) {
        objects[i] = allocator.alloc();
        //printf("Allocated %d %p\n", i, objects[i]);
    }

    for (int i=0; i<128; i++) {
        //printf("Freeing %d %p\n", i, objects[i]);
        allocator.free(objects[i]);
    }

    success_msg("Malloc alloc and free inorder tests passed\n");

    for (int i=0; i<128; i++) {
        objects[i] = allocator.alloc();
        //printf("Allocated %p\n", objects[i]);
    }

    for (int i=127; i>=64; i--) {
        //printf("Freeing %p\n", objects[i]);
        allocator.free(objects[i]);
        objects[i] = nullptr;
    }

    for (int i=64; i<128; i++) {
        objects[i] = allocator.alloc();
        //printf("Allocated %p\n", objects[i]);
    }

    for (int i=0; i<128; i+=2) {
        //printf("Freeing %p\n", objects[i]);
        allocator.free(objects[i]);
        objects[i] = nullptr;
    }

    for (int i=0; i<128; i+=2) {
        objects[i] = allocator.alloc();
        //printf("Allocated %p\n", objects[i]);
    }

    for (int i=127; i>=0; i--) {
        //printf("Freeing %p\n", objects[i]);
        allocator.free(objects[i]);
        objects[i] = nullptr;
    }

    success_msg("Malloc tests passed\n");
}

void runMemPoolTests() {
    MemPool pool(KERNEL_PID, 16*1024);

    // Large allocation and free in same order
    // as allocations.
    void *largeAllocs[3];
    for (int i=0; i<3; i++) {
        largeAllocs[i] = pool.alloc(4096);
        assert(largeAllocs[i] != nullptr);
    }

    for (int i=0; i<3; i++) {
        pool.free(largeAllocs[i]);
    }

    // Large allocation and free in the opposite
    // order of allocations.
    for (int i=0; i<3; i++) {
        largeAllocs[i] = pool.alloc(4096);
        assert(largeAllocs[i] != nullptr);
    }

    for (int i=2; i>=0; i--) {
        pool.free(largeAllocs[i]);
    }

    // Large allocation and free in mixed order
    for (int i=0; i<3; i++) {
        largeAllocs[i] = pool.alloc(4096);
        assert(largeAllocs[i] != nullptr);
    }

    pool.free(largeAllocs[0]);
    pool.free(largeAllocs[2]);
    pool.free(largeAllocs[1]);

    success_msg("large allocs tests passed\n");
}

void tzKernelInit(const void *devTree) {

    printf("%s: Memory subsystem unit tests\n", __FUNCTION__);

    if (arm::smpCpuNum() != 0) {
        tzKernelSecondary();
        return;
    }

    System::init(devTree);

    setupPageAllocTests(tzDevTree);

    runPageAllocTests();

    runPageTableTests();

    runMallocTests();

    runMemPoolTests();

    success_msg("Memory subsystem unit tests done\n");
    while (1) {
        asm volatile("wfi":::);
    }
}

void tzKernelSecondary() {

    System::initSecondaryCpu();

    ARCH_SPECIFIC_DISABLE_INTERRUPTS;

    schedule();
}

void kernelHalt(const char *reason) {
    err_msg("%s\n", reason);
    while (true) {}
}

extern "C" void __cxa_pure_virtual() {
    err_msg("Pure virtual function called !\n");
    kernelHalt("Pure virtual function call");
}
