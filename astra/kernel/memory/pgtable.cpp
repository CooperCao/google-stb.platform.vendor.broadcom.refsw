/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include <hwtimer.h>
#include "lib_printf.h"
#include "kernel.h"
#include "pgtable.h"

static uint8_t kernPageTableMem[sizeof(PageTable)];

PageTable *PageTable::kernPageTable;
ALIGN_PT  uint8_t PageTable::pageTableBlocks[PAGE_SIZE_4K_BYTES * MAX_NUM_PAGE_TABLE_BLOCKS];
uint8_t* PageTable::freeBlockStack[MAX_NUM_PAGE_TABLE_BLOCKS + NUM_BOOTSTRAP_BLOCKS];
int PageTable::stackTop;
spinlock_t PageTable::allocLock;

void * sys_page_table;

void* PageTable::operator new(size_t sz, void *where) {
    UNUSED(sz);
    PageTable *pt = (PageTable *)where;
    pt->l1Dir = nullptr;
    spinlock_init("PageTable::lock", &pt->lock);

    return pt;
}

void PageTable::init() {
    uint8_t *currPage = &pageTableBlocks[0];
    for (int i = 0; i<MAX_NUM_PAGE_TABLE_BLOCKS; i++) {
        freeBlockStack[i] = currPage;
        currPage += PAGE_SIZE_4K_BYTES;
    }
    stackTop = MAX_NUM_PAGE_TABLE_BLOCKS-1;

    kernPageTable = new(kernPageTableMem) PageTable;

    uint64_t l1Table;
    register unsigned int ttbr0Low, ttbr0High;
    asm volatile("mrrc p15, 0, %[low], %[high], c2" : [low] "=r" (ttbr0Low), [high] "=r" (ttbr0High) : :);
    l1Table = ((uint64_t)ttbr0High << 32) | ttbr0Low;

    sys_page_table = (void *)l1Table;

    kernPageTable->l1Dir = (uint64_t *)TzMem::physToVirt((const void *)l1Table);
    kernPageTable->asid = KERNEL_ASID;
    // printf("Kernel page table base addr %p\n", (void *)l1Table);

    spinlock_init("pgtable.static.lock", &allocLock);

    printf("PageTable init done\n");
}

PageTable *PageTable::kernelPageTable() {
    return kernPageTable;
}

TzMem::VirtAddr PageTable::allocPageTableBlock() {
    SpinLocker locker(&allocLock);

    if (stackTop < 0) {
        err_msg("Ran out of page table blocks\n");
        kernelHalt("Page table blocks exhausted");
    }

    uint8_t *rv = freeBlockStack[stackTop--];
    return rv;
}

void PageTable::freePageTableBlock(TzMem::VirtAddr vaddr) {
    SpinLocker locker(&allocLock);

    if (stackTop == (MAX_NUM_PAGE_TABLE_BLOCKS + NUM_BOOTSTRAP_BLOCKS - 1)) {
        err_msg("Page table block stack corrupted. Some blocks were double freed.\n");
        kernelHalt("Page table block stack corruption");
    }

    freeBlockStack[++stackTop] = (uint8_t *)vaddr;
}

PageTable::PageTable(const PageTable& rhs, uint8_t aid, bool fork) {

    l1Dir = (uint64_t *)allocPageTableBlock();
    // printf("Created page table %p %p\n", TzMem::virtToPhys(l1Dir), l1Dir);

    this->asid = aid;
    spinlock_init("PageTable::lock", &lock);

    for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++) {
        if (rhs.l1Dir[i] == 0) {
            l1Dir[i] = 0;
            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[i]) : "memory");

            continue;
        }

        uint32_t rhsL2Block = (uint32_t)(rhs.l1Dir[i] & L2_BLOCK_ADDR_MASK);
        uint64_t *rhsL2Dir = (uint64_t *)TzMem::physToVirt((void *)rhsL2Block);

        uint64_t *l2Dir = (uint64_t *)allocPageTableBlock();
        uint64_t l1Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(l2Dir) & L2_BLOCK_ADDR_MASK);
        l1Entry |= 0x3;
        l1Dir[i] = l1Entry;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[i]) : "memory");

        for (int j=0; j<L2_PAGE_NUM_ENTRIES; j++) {
            if (rhsL2Dir[j] == 0) {
                l2Dir[j] = 0;
                // We wrote the entry into cached memory. Force it to
                // main memory by doing a DCCIMVAC on the entry.
                asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l2Dir[j]) : "memory");

                continue;
            }

            uint32_t rhsL3Block = (uint32_t)(rhsL2Dir[j] & L2_BLOCK_ADDR_MASK);
            uint64_t *rhsL3Dir = (uint64_t *)TzMem::physToVirt((void *)rhsL3Block);

            uint64_t *l3Dir = (uint64_t *)allocPageTableBlock();
            uint64_t l2Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(l3Dir) & L3_BLOCK_ADDR_MASK);
            l2Entry |= 0x3;
            l2Dir[j] = l2Entry;
            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l2Dir[j]) : "memory");

            for (int k=0; k<L3_PAGE_NUM_ENTRIES; k++) {
                uint64_t l3Entry = rhsL3Dir[k];

                if (fork &&
                    GET_MEMORY_ACCESS_SW_BITS(rhsL3Dir[k]) != SharedMem &&
                    GET_MEMORY_ACCESS_PERMS(rhsL3Dir[k]) == MEMORY_ACCESS_RW_USER) {

                    CLEAR_MEMORY_ACCESS_PERMS(l3Entry);
                    SET_MEMORY_ACCESS_PERMS(l3Entry, MEMORY_ACCESS_RO_USER);

                    CLEAR_MEMORY_ACCESS_SW_BITS(l3Entry);
                    SET_MEMORY_ACCESS_SW_BITS(l3Entry, AllocOnWrite);
                }
                else {
                    CLEAR_MEMORY_ACCESS_SW_BITS(l3Entry);
                    SET_MEMORY_ACCESS_SW_BITS(l3Entry, SharedMem);
                }

                l3Dir[k] = l3Entry;

                // We wrote the entry into cached memory. Force it to
                // main memory by doing a DCCIMVAC on the entry.
                asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[k]) : "memory");
            }
        }
    }
}

PageTable::~PageTable() {

    if (l1Dir == nullptr)
        return;

    for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++) {
        if (l1Dir[i] == 0)
            continue;

        uint32_t l2Block = (uint32_t)(l1Dir[i] & L2_BLOCK_ADDR_MASK);
        uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)l2Block);

        for (int j=0; j<L2_PAGE_NUM_ENTRIES; j++) {
            if (l2Dir[j] == 0)
                continue;

            uint32_t l3Block = (uint32_t)(l2Dir[j] & L3_BLOCK_ADDR_MASK);
            uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)l3Block);

            for (int k=0; k<L3_PAGE_NUM_ENTRIES; k++) {
                uint64_t l3Entry = l3Dir[k];
                if (l3Entry == 0)
                    continue;

                if ((l3Entry & 0x3) != 0x3)
                    continue;

                // if (GET_MEMORY_ACCESS_SW_BITS(l3Entry) == WriteAlloced) {
                if (GET_MEMORY_ACCESS_SW_BITS(l3Entry) != AllocOnWrite &&
                    GET_MEMORY_ACCESS_SW_BITS(l3Entry) != SharedMem) {
                    TzMem::PhysAddr paddr = (void *)(l3Entry & L3_PHYS_ADDR_MASK);
                    TzMem::freePage(paddr);
                }
            }

            freePageTableBlock(l3Dir);
        }

        freePageTableBlock(l2Dir);
    }

    freePageTableBlock(l1Dir);
    l1Dir = nullptr;
}

void PageTable::reserveRange(TzMem::VirtAddr vaddrFirstPage, TzMem::VirtAddr vaddrLastPage) {
    TzMem::VirtAddr vaddr = vaddrFirstPage;

    if (l1Dir == nullptr) {
        l1Dir = (uint64_t *)allocPageTableBlock();
        for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++) {
            l1Dir[i] = 0;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[i]) : "memory");
        }
    }

    while (vaddr < vaddrLastPage) {
        const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
        uint64_t l1Entry = l1Dir[l1Idx];
        // printf("Mapping vaddr %p: l1Idx %d l1Entry 0x%x%x\n", vaddr, l1Idx, (unsigned int)(l1Entry >> 32), (unsigned int)(l1Entry & 0xffffffff));
        if (l1Entry == 0) {

            uint64_t *ptBlock = (uint64_t *)allocPageTableBlock();
            for (int i=0; i<L2_PAGE_NUM_ENTRIES; i++)
                ptBlock[i] = 0;

            l1Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(ptBlock) & L2_BLOCK_ADDR_MASK);;
            l1Entry |= 0x3;
            l1Dir[l1Idx] = l1Entry;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[l1Idx]) : "memory");
        }

        uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
        uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
        uint64_t l2Entry = l2Dir[l2Idx];
        // printf("Mapping vaddr %p: l2Idx %d l2Entry 0x%x%x\n", vaddr, l2Idx, (unsigned int)(l2Entry >> 32), (unsigned int)(l2Entry & 0xffffffff));
        if (l2Entry == 0) {

            uint64_t *ptBlock = (uint64_t *)allocPageTableBlock();
            for (int i=0; i<L3_PAGE_NUM_ENTRIES; i++)
                ptBlock[i] = 0;

            l2Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(ptBlock) & L3_BLOCK_ADDR_MASK);
            l2Entry |= 0x3;

            l2Dir[l2Idx] = l2Entry;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l2Dir[l2Idx]) : "memory");
        }

        blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
        uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
        //printf("Reserving vaddr %p: l3Idx %d\n", vaddr, l3Idx);

        uint64_t l3Entry = 0x3;
        l3Entry |=  (uint64_t)((uint32_t)0xFFFFFFFF & L3_PHYS_ADDR_MASK); //0xFFFFF000
        SET_MEMORY_ACCESS_FLAG(l3Entry, ACCESS_FLAG_FAULT_GEN);

        l3Dir[l3Idx] = l3Entry;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");

        vaddr = (TzMem::VirtAddr)((uint8_t *)vaddr + PAGE_SIZE_4K_BYTES);
    }
}

void PageTable::releaseAddrRange(TzMem::VirtAddr vaddrFirstPage, unsigned int rangeSize) {
    TzMem::VirtAddr vaddr = vaddrFirstPage;
    TzMem::VirtAddr vaddrLastPage = PAGE_START_4K((uint8_t *)vaddr + rangeSize);

    if (l1Dir == nullptr)
        return;

    while (vaddr < vaddrLastPage) {
        const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
        uint64_t l1Entry = l1Dir[l1Idx];
        // printf("UnMapping vaddr %p: l1Idx %d l1Entry 0x%x%x\n", vaddr, l1Idx, (unsigned int)(l1Entry >> 32), (unsigned int)(l1Entry & 0xffffffff));
        if (l1Entry == 0)
            return;

        uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
        uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
        uint64_t l2Entry = l2Dir[l2Idx];
        // printf("UnMapping vaddr %p: l2Idx %d l2Entry 0x%x%x\n", vaddr, l2Idx, (unsigned int)(l2Entry >> 32), (unsigned int)(l2Entry & 0xffffffff));
        if (l2Entry == 0)
            return;

        blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
        uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
        //printf("Release vaddr %p: l3Idx %d\n", vaddr, l3Idx);

        uint64_t l3Entry = 0x0;
        l3Dir[l3Idx] = l3Entry;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");

        vaddr = (TzMem::VirtAddr)((uint8_t *)vaddr + PAGE_SIZE_4K_BYTES);
    }

}

void PageTable::mapPageRange(const TzMem::VirtAddr vaddrFirstPage, const TzMem::VirtAddr vaddrLastPage,
            const TzMem::PhysAddr paddrFirstPage, const int memAttr, const int memAccessPerms, const bool executeNever, const bool mapShared, const bool nonSecure) {

    // printf("page table %p: map [%p, %p] to [%p]\n", this, vaddrFirstPage, vaddrLastPage, paddrFirstPage);
    SpinLocker locker(&lock);

    TzMem::VirtAddr vaddr = vaddrFirstPage;
    TzMem::PhysAddr paddr = paddrFirstPage;

    if (l1Dir == nullptr) {
        l1Dir = (uint64_t *)allocPageTableBlock();
        for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++) {
            l1Dir[i] = 0;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[i]) : "memory");
        }
    }

    while (vaddr <= vaddrLastPage) {

        const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
        uint64_t l1Entry = l1Dir[l1Idx];
        // printf("Mapping vaddr %p: l1Idx %d l1Entry 0x%x%x\n", vaddr, l1Idx, (unsigned int)(l1Entry >> 32), (unsigned int)(l1Entry & 0xffffffff));
        if (l1Entry == 0) {

            uint64_t *ptBlock = (uint64_t *)allocPageTableBlock();
            for (int i=0; i<L2_PAGE_NUM_ENTRIES; i++)
                ptBlock[i] = 0;

            l1Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(ptBlock) & L2_BLOCK_ADDR_MASK);;
            l1Entry |= 0x3;
            l1Dir[l1Idx] = l1Entry;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[l1Idx]) : "memory");
        }

        uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
        uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
        uint64_t l2Entry = l2Dir[l2Idx];
        // printf("Mapping vaddr %p: l2Idx %d l2Entry 0x%x%x\n", vaddr, l2Idx, (unsigned int)(l2Entry >> 32), (unsigned int)(l2Entry & 0xffffffff));
        if (l2Entry == 0) {

            uint64_t *ptBlock = (uint64_t *)allocPageTableBlock();
            for (int i=0; i<L3_PAGE_NUM_ENTRIES; i++)
                ptBlock[i] = 0;

            l2Entry = (uint64_t)((uint32_t)TzMem::virtToPhys(ptBlock) & L3_BLOCK_ADDR_MASK);
            l2Entry |= 0x3;

            l2Dir[l2Idx] = l2Entry;

            // We wrote the entry into cached memory. Force it to
            // main memory by doing a DCCIMVAC on the entry.
            asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l2Dir[l2Idx]) : "memory");
        }

        blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
        uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);
        int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
        //printf("Mapping vaddr %p: l3Idx %d\n", vaddr, l3Idx);

        uint64_t currEntry = l3Dir[l3Idx];

        uint64_t l3Entry = 0x3;
        l3Entry |=  (uint64_t)((uint32_t)paddr & L3_PHYS_ADDR_MASK); //0xFFFFF000

        SET_MEMORY_ACCESS_FLAG(l3Entry, ACCESS_FLAG_NO_FAULT_GEN);
        SET_MEMORY_ATTR(l3Entry, memAttr);
        SET_MEMORY_SH_ATTR(l3Entry, INNER_SHAREABLE);
        SET_MEMORY_ACCESS_PERMS(l3Entry, memAccessPerms);

        if (executeNever)
            SET_MEMORY_ACCESS_NO_EXEC(l3Entry);

        if (nonSecure)
            SET_MEMORY_NS_BIT(l3Entry, NS_BIT_NON_SECURE);

        if (GET_MEMORY_ACCESS_SW_BITS(currEntry) == AllocOnWrite)
            SET_MEMORY_ACCESS_SW_BITS(l3Entry, WriteAlloced);

        if (mapShared)
            SET_MEMORY_ACCESS_SW_BITS(l3Entry, SharedMem);

        l3Dir[l3Idx] = l3Entry;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");

        vaddr = (TzMem::VirtAddr)((uint8_t *)vaddr + PAGE_SIZE_4K_BYTES);
        paddr = (TzMem::PhysAddr)((uint8_t *)paddr + PAGE_SIZE_4K_BYTES);
    }

    // Issue a memory barrier: Data accesses and instructions that follow this point should not get
    // re-ordered to run before this point
    asm volatile("dmb":::"memory");
    asm volatile("isb":::"memory");
}

void PageTable::unmapPageRange(const TzMem::VirtAddr vaddrFirstPage, const TzMem::VirtAddr vaddrLastPage, const bool releaseVaddr) {

    // printf("page table %p: unmap [%p, %p]\n", this, vaddrFirstPage, vaddrLastPage);
    SpinLocker locker(&lock);

    TzMem::VirtAddr vaddr = vaddrFirstPage;

    while (vaddr <= vaddrLastPage) {
        const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
        if (l1Dir[l1Idx] == 0) {
            err_msg("%s:\n\t Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
            kernelHalt("Attempted unmap a page that wasn't mapped");
        }

        uint64_t l1Entry = l1Dir[l1Idx];
        uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
        uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

        int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
        if (l2Dir[l2Idx] == 0) {
            err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
            kernelHalt("Attempted unmap a page that wasn't mapped");
        }

        uint64_t l2Entry = l2Dir[l2Idx];
        blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
        uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

        int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
        if (l3Dir[l3Idx] == 0) {
            err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
            kernelHalt("Attempted unmap a page that wasn't mapped");
        }

        // Unmap the page.
        //printf("UnMapping vaddr %p: l3Idx %d\n", vaddr, l3Idx);
        uint64_t l3Entry;
        if (releaseVaddr) {
            // Release the page.
            l3Entry = 0;
        }
        else {
            // Keep the page reserved.
            l3Entry = 0x3;
            l3Entry |=  (uint64_t)((uint32_t)0xFFFFFFFF & L3_PHYS_ADDR_MASK); //0xFFFFF000
            SET_MEMORY_ACCESS_FLAG(l3Entry, ACCESS_FLAG_FAULT_GEN);
        }

        l3Dir[l3Idx] = l3Entry;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");

        vaddr = (TzMem::VirtAddr)((uint8_t *)vaddr + PAGE_SIZE_4K_BYTES);

        if (!releaseVaddr)
            continue;

        // Check if the l3Dir can be freed.
        bool canFree = true;
        for (int i=0; i<L3_PAGE_NUM_ENTRIES; i++) {
            if (l3Dir[i] != 0) {
                // No there is at-least one valid entry in this dir.
                canFree = false;
                break;
            }
        }
        if (!canFree)
            continue;
        // There are no valid entries in L3 dir. Free it.
        freePageTableBlock(l3Dir);
        l2Dir[l2Idx] = 0;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l2Dir[l2Idx]) : "memory");

        // Now check if l2Dir can be freed.
        canFree = true;
        for (int i=0; i<L2_PAGE_NUM_ENTRIES; i++) {
            if (l2Dir[i] != 0) {
                // No there is at-least one valid entry in this dir.
                canFree = false;
                break;
            }
        }
        if (!canFree)
            continue;

        // There are no valid entries in L3 dir. Free it.
        freePageTableBlock(l2Dir);
        l1Dir[l1Idx] = 0;
        // We wrote the entry into cached memory. Force it to
        // main memory by doing a DCCIMVAC on the entry.
        asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l1Dir[l1Idx]) : "memory");

        // Now check if l1Dir can be freed.
        canFree = true;
        for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++) {
            if (l1Dir[i] != 0) {
                // No there is at-least one valid entry in this dir.
                canFree = false;
                break;
            }
        }
        if (!canFree)
            continue;

        // There are no valid entries in L3 dir. Free it.
        freePageTableBlock(l1Dir);
        l1Dir = nullptr;
    }

    // Flush the TLB to remove any stale references to the now unmapped pages.
    // Also invalidate the branch predictor.
    asm volatile("dsb\r\n"
                     "mov r4, #0\r\n"
                     "mcr p15, 0, r4, c8, c7, 0\r\n"
                     "mcr p15, 0, r4, c7, c5, 6\r\n"
                     "dsb\r\n"
                     "isb\r\n"
                     : : : "r4");
}

TzMem::PhysAddr PageTable::lookUp(TzMem::VirtAddr vaddr, PageTable::EntryAttribs *attribs) const {
    SpinLocker locker(&lock);

    return lookUpNoLock(vaddr, attribs);
}

TzMem::PhysAddr PageTable::lookUpNoLock(TzMem::VirtAddr vaddr, PageTable::EntryAttribs *attribs) const {


    if (l1Dir == nullptr)
        return nullptr;

    const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
    const uint64_t l1Entry = l1Dir[l1Idx];
    if (l1Entry == 0)
        return nullptr;

    uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
    const uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((uint64_t *)blockAddr);
    const int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
    const uint64_t l2Entry = l2Dir[l2Idx];
    if (l2Entry == 0)
        return nullptr;

    blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
    const uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((uint64_t *)blockAddr);
    const int l3Idx = L3_PAGE_TABLE_SLOT(vaddr);
    const uint64_t l3Entry = l3Dir[l3Idx];

    if ((l3Entry & 0x3) == 0x3) {
        uint64_t physAddrBase = l3Entry & L3_PHYS_ADDR_MASK;
        uint64_t physAddr = physAddrBase | ((uint32_t)vaddr & ~L3_PHYS_ADDR_MASK);

        if (attribs != nullptr) {
            uint8_t swBits = GET_MEMORY_ACCESS_SW_BITS(l3Entry);
            uint8_t nsBit = GET_MEMORY_NS_BIT(l3Entry);
            attribs->swAttribs = (PageTable::SwAttribs)swBits;
            attribs->nonSecure = (nsBit == 1) ? true : false;
        }

        return (TzMem::PhysAddr)(physAddr);
    }

    return nullptr;
}

bool PageTable::isAddrMapped(const TzMem::VirtAddr addr) {
    SpinLocker locker(&lock);
    TzMem::VirtAddr pageBoundary = (TzMem::VirtAddr)((unsigned long)addr & PAGE_MASK);
    if (lookUpNoLock(pageBoundary) == nullptr)
        return false;
    return true;

}

bool PageTable::isAddrRangeMapped(const TzMem::VirtAddr addr, const unsigned int rangeSize) {
    SpinLocker locker(&lock);
    int numPages = rangeSize/PAGE_SIZE_4K_BYTES;
    if (addr != PAGE_START_4K(addr))
        numPages++;

    uint8_t *currPage = (uint8_t *)PAGE_START_4K(addr);
    for (int i=0; i<numPages; i++) {
        if (lookUpNoLock(currPage) == nullptr)
            return false;

        currPage += PAGE_SIZE_4K_BYTES;
    }

    return true;
}

bool PageTable::isAddrRangeUnMapped(const TzMem::VirtAddr addr, const unsigned int rangeSize) {
    SpinLocker locker(&lock);
    int numPages = rangeSize/PAGE_SIZE_4K_BYTES;
    if (addr != PAGE_START_4K(addr))
        numPages++;

    uint8_t *currPage = (uint8_t *)PAGE_START_4K(addr);
    for (int i=0; i<numPages; i++) {
        if (lookUpNoLock(currPage) != nullptr)
            return false;

        currPage += PAGE_SIZE_4K_BYTES;
    }

    return true;
}

TzMem::VirtAddr PageTable::reserveAddrRange(const TzMem::VirtAddr fromAddr, unsigned int rangeSize, ScanDirection direction) {

    SpinLocker locker(&lock);

    TzMem::VirtAddr nextAddr = fromAddr;
    TzMem::VirtAddr rangeStart = nextAddr;
    int currRangeSize = 0;

    while ((currRangeSize < rangeSize) && ((unsigned long)nextAddr > 0x00000000UL) && ((unsigned long)nextAddr < 0xFFFFFFFFUL)) {

        TzMem::PhysAddr pa = lookUpNoLock(nextAddr);
        bool mapped = (pa != nullptr);

        nextAddr = (direction == ScanForward) ? (uint8_t *)nextAddr + PAGE_SIZE_4K_BYTES : (uint8_t *)nextAddr - PAGE_SIZE_4K_BYTES;
        if (mapped) {
            currRangeSize = 0;
            rangeStart = nextAddr;
        }
        else {
            currRangeSize += PAGE_SIZE_4K_BYTES;
        }
    }

    if (direction == ScanForward) {
        // rangeStart points to the first page
        // nextAddr points to the page beyond the last one
        reserveRange((uint8_t *)rangeStart,
                     (uint8_t *)nextAddr - 1);
        return rangeStart;
    }
    else {
        // rangeStart points to the last page
        // nextAddr points to the page before the first one
        reserveRange((uint8_t *)nextAddr   + PAGE_SIZE_4K_BYTES,
                     (uint8_t *)rangeStart + PAGE_SIZE_4K_BYTES - 1);
        return (uint8_t *)nextAddr + PAGE_SIZE_4K_BYTES;
    }
}

void PageTable::unmapBootstrap(const void *devTree) {

    if (this != kernPageTable)
        return;

    TzMem::PhysAddr dtStart = (uint8_t *)devTree;
    TzMem::PhysAddr dtEnd = (uint8_t *)devTree + MAX_DT_SIZE_BYTES - 1;

    //printf("unmapping [%p-%p] %p\n", dtStart, dtEnd, devTree);
    unmapPageRange(dtStart, dtEnd);
}

void PageTable::copyOnWrite(void *va) {
    UNUSED(va);
    //TODO: Move from TzTask::dataAbortException
}

void PageTable::makePageCopyOnWrite(const TzMem::VirtAddr vaddr) {
    const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
    if (l1Dir[l1Idx] == 0) {
        err_msg("%s:\n\t Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l1Entry = l1Dir[l1Idx];
    uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
    uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

    int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
    if (l2Dir[l2Idx] == 0) {
        err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l2Entry = l2Dir[l2Idx];
    blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
    uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

    int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
    if (l3Dir[l3Idx] == 0) {
        err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l3Entry = l3Dir[l3Idx];

    CLEAR_MEMORY_ACCESS_PERMS(l3Entry);
    SET_MEMORY_ACCESS_PERMS(l3Entry, MEMORY_ACCESS_RO_USER);

    CLEAR_MEMORY_ACCESS_SW_BITS(l3Entry);
    SET_MEMORY_ACCESS_SW_BITS(l3Entry, AllocOnWrite);

    l3Dir[l3Idx] = l3Entry;

    // We wrote the entry into cached memory. Force it to
    // main memory by doing a DCCIMVAC on the entry.
    asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");
}

void PageTable::changePageAccessPerms(TzMem::VirtAddr vaddr, int accessPerms, bool noExec) {

    const int l1Idx = L1_PAGE_TABLE_SLOT(vaddr);
    if (l1Dir[l1Idx] == 0) {
        err_msg("%s:\n\t Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l1Entry = l1Dir[l1Idx];
    uint32_t blockAddr = (uint32_t)(l1Entry & L2_BLOCK_ADDR_MASK);
    uint64_t *l2Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

    int l2Idx = L2_PAGE_TABLE_SLOT(vaddr);
    if (l2Dir[l2Idx] == 0) {
        err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l2Entry = l2Dir[l2Idx];
    blockAddr = (uint32_t)(l2Entry & L3_BLOCK_ADDR_MASK);
    uint64_t *l3Dir = (uint64_t *)TzMem::physToVirt((void *)blockAddr);

    int l3Idx =  L3_PAGE_TABLE_SLOT(vaddr);
    if (l3Dir[l3Idx] == 0) {
        err_msg("%s:\n\t  Attempt to unmap page %p that wasnt mapped\n", __PRETTY_FUNCTION__, vaddr);
        kernelHalt("Attempted unmap a page that wasn't mapped");
    }

    uint64_t l3Entry = l3Dir[l3Idx];

    CLEAR_MEMORY_ACCESS_PERMS(l3Entry);
    SET_MEMORY_ACCESS_PERMS(l3Entry, accessPerms);

    if (noExec)
        SET_MEMORY_ACCESS_NO_EXEC(l3Entry);

    l3Dir[l3Idx] = l3Entry;

    // We wrote the entry into cached memory. Force it to
    // main memory by doing a DCCIMVAC on the entry.
    asm volatile ("mcr p15, 0, %[rt], c7, c14, 1" : : [rt] "r" (&l3Dir[l3Idx]) : "memory");
}
