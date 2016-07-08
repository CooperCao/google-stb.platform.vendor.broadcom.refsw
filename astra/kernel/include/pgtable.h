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
#ifndef PGTABLE_H_
#define PGTABLE_H_

#include <cstdint>
#include <cstddef>
#include <new>

#include "config.h"
#include "tzmemory.h"

#include "arm/spinlock.h"

extern "C" void *sys_page_table;

class PageTable {
public:
    enum SwAttribs {
        None = 0,
        AllocOnWrite = 1,
        WriteAlloced = 2,
        SharedMem = 3
    };

    struct EntryAttribs {
        SwAttribs swAttribs;
        bool nonSecure;
    };

public:
    static void init();

    static PageTable *kernelPageTable();

    PageTable() { l1Dir = nullptr; asid = KERNEL_ASID; spinlock_init("PageTable::lock", &lock);}
    ~PageTable();

    PageTable(const PageTable& rhs, uint8_t asid, bool fork = false);

    void mapPage(const TzMem::VirtAddr vaddr, const TzMem::PhysAddr paddr,
                    const int memAttr = MAIR_MEMORY, const int memAccessPerms = MEMORY_ACCESS_RW_KERNEL, const bool executeNever = true, const bool mapShared = false, const bool nonSecure = false) {
        mapPageRange(vaddr, vaddr, paddr, memAttr, memAccessPerms, executeNever, mapShared, nonSecure);
    }

    void mapPageRange(const TzMem::VirtAddr vaddrFirstPage, const TzMem::VirtAddr vaddrLastPage, const TzMem::PhysAddr physAddrFirstPage,
                    const int memAttr = MAIR_MEMORY, const int memAccessPerms = MEMORY_ACCESS_RW_KERNEL, const bool executeNever = true, const bool mapShared = false, const bool nonSecure = false);

    void unmapPage(const TzMem::VirtAddr vaddr, const bool releaseVaddr = true) { unmapPageRange(vaddr, vaddr, releaseVaddr); }
    void unmapPageRange(const TzMem::VirtAddr vaddrFirstPage, const TzMem::VirtAddr vaddrLastPage, const bool releaseVaddr = true);

    enum ScanDirection {
        ScanForward,
        ScanBackward
    };

    bool isAddrRangeMapped(const TzMem::VirtAddr fromAddr, const unsigned int rangeSize);
    bool isAddrRangeUnMapped(const TzMem::VirtAddr fromAddr, const unsigned int rangeSize);

    TzMem::VirtAddr reserveAddrRange(const TzMem::VirtAddr fromAddr, unsigned int rangeSize, ScanDirection direction);
    void releaseAddrRange(TzMem::VirtAddr va, unsigned int rangeSize);

    TzMem::PhysAddr lookUp(TzMem::VirtAddr vaddr, EntryAttribs *attribs = nullptr) const;

    void makePageCopyOnWrite(const TzMem::VirtAddr pageVa);

    void changePageAccessPerms(const TzMem::VirtAddr va, const int accessPerms, bool executeNever);

    void copyOnWrite(const TzMem::VirtAddr va);

    inline void activate() {

        register TzMem::PhysAddr pageTablePA = TzMem::virtToPhys(l1Dir);
        // printf("%s: switching to page table %p base %p (%p)\n", __FUNCTION__, this, l1Dir, pageTablePA);

        /* Invalidate TLB, ICache and branch predictor */
        register unsigned int zero = 0;
        asm volatile(
                "mcr p15, 0, %[val], c8, c7, 0\r\n" //TLBIALL
                "mcr p15, 0, %[val], c8, c3, 0\r\n" //TLBIALLIS
                "mcr p15, 0, %[val], c7, c5, 0\r\n"
                "mcr p15, 0, %[val], c7, c5, 6\r\n"
                "dmb\r\n"
                "isb\r\n"
                : : [val] "r" (zero));

        /* Point TTBR0 to the page table */
        register unsigned int ttbr0Low = (unsigned int)pageTablePA;
        register unsigned int ttbr0High = 0; //(asid << 16);
        asm volatile("mcrr p15, 0, %[low], %[high], c2" : : [low] "r" (ttbr0Low), [high] "r" (ttbr0High));
        asm volatile("isb":::);

        /* Invalidate TLB, ICache and branch predictor */
        asm volatile(
                "mcr p15, 0, %[val], c8, c7, 0\r\n" //TLBIALL
                "mcr p15, 0, %[val], c8, c3, 0\r\n" //TLBIALLIS
                "mcr p15, 0, %[val], c7, c5, 0\r\n"
                "mcr p15, 0, %[val], c7, c5, 6\r\n"
                "dmb\r\n"
                "isb\r\n"
                : : [val] "r" (zero));

        // unsigned long entry = 0x9fffec00;
        // unsigned long translation = TzMem::addressTranslateCPR(entry);
        // printf("%p:  0x%lx ---> 0x%lx\n", pageTablePA, entry, translation);
    }

    // Only works on kernel page table
    void unmapBootstrap(const void *devTree);

    void dump();

private:
    void* operator new(size_t sz, void* where);

    TzMem::PhysAddr lookUpNoLock(TzMem::VirtAddr vaddr, EntryAttribs *attribs = nullptr) const;
    void reserveRange(const TzMem::VirtAddr vaddrFirstPage, const TzMem::VirtAddr vaddrLastPage);


private:
    uint64_t *l1Dir;
    uint8_t asid;
    mutable spinlock_t lock;

    static PageTable *kernPageTable;

    static uint8_t pageTableBlocks[PAGE_SIZE_4K_BYTES * MAX_NUM_PAGE_TABLE_BLOCKS];
    static uint8_t *freeBlockStack[MAX_NUM_PAGE_TABLE_BLOCKS + NUM_BOOTSTRAP_BLOCKS];
    static int stackTop;

    static spinlock_t allocLock;

private:

    PageTable(const PageTable& ) = delete;
    PageTable& operator = (const PageTable& ) = delete;

    TzMem::VirtAddr allocPageTableBlock();
    void freePageTableBlock(TzMem::VirtAddr );

    inline bool isAddrMapped(const TzMem::VirtAddr addr);

};


#endif /* PGTABLE_H_ */
