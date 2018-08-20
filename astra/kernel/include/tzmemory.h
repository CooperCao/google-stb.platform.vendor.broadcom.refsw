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

#ifndef TZ_MEMORY_H_
#define TZ_MEMORY_H_

#include <cstdint>
#include "config.h"
#include "arm/arm.h"
#include "arm/spinlock.h"
#include "kernel.h"
#include "lib_printf.h"

#define MEMORY_DEBUG 0
#define ENABLE_PAGE_TRACE 0

#if ENABLE_PAGE_TRACE
#define allocPage(pid) _allocPageTrace(__PRETTY_FUNCTION__, pid)
#define freePage(page) _freePageTrace(__PRETTY_FUNCTION__, page)
#else
#define allocPage(pid) _allocPage(pid)
#define freePage(pid)  _freePage(pid)
#endif

class TzMem {
public:
    static const unsigned int TZ_MAX_MEMORY = (64 * 1024 * 1024);
    static const unsigned int TZ_MAX_NUM_PAGES = (TZ_MAX_MEMORY / PAGE_SIZE_4K_BYTES);
    static const unsigned int TZ_MAX_NUM_RANGES = 2;

    typedef void* PhysAddr;
    typedef void* VirtAddr;

    /* MRR Heap*/
    static uintptr_t mrrHeapStart;
    static uintptr_t mrrHeapEnd;

    /* KRR Heap*/
    static uintptr_t krrHeapStart;
    static uintptr_t krrHeapEnd;

    /* CRR Heap*/
    static uintptr_t crrHeapStart;
    static uintptr_t crrHeapEnd;

    static uint32_t secRegion;

public:
    static void init(void *devTree);
    static void freeInitRamFS(VirtAddr vaStart, VirtAddr vaEnd);

    static PhysAddr _allocPage(int pid);
    static int _freePage(PhysAddr page);

#if ENABLE_PAGE_TRACE
    static PhysAddr _allocPageTrace(const char *caller, int pid) {
        PhysAddr page = _allocPage(pid);
        printf("allocPage: %d %s: %p\n", pid, caller, page);
        return page;
    }

    static void _freePageTrace(const char *caller, PhysAddr page) {
        int pid = _freePage(page);
        printf("freePage: %d %s: %p\n", pid, caller, page);
    }
#endif

    static PhysAddr allocContiguousPages(int numPages, int pid);
    static void freeContiguousPages(PhysAddr firstPage, int numPages);

    inline static int numFreePages() { return (freePagesEnd - freePagesStart); }
    inline static int numAllocatedPages() { return freePagesStart; }

    inline static void * virtToPhys(const void *virtAddr) {
        return (void *)((unsigned long)virtAddr + virt_to_phys_offset);
    }

    inline static void * physToVirt(const void *physAddr) {
        return (void *)((unsigned long)physAddr + phys_to_virt_offset);
    }

    inline static void cacheFlush(const void *startAddr, int numBytes) {
        ARCH_SPECIFIC_CACHEFLUSH(startAddr,numBytes);
    }

    inline static unsigned long addressTranslateCPR(const unsigned long entry) {
        register unsigned int high, low;

        asm volatile("mcr p15, 0, %[rt], c7, c8, 0 \r\n isb " : :[rt] "r" (entry) :);

        asm volatile("mrrc p15, 0, %[rt], %[rt2], c7" : [rt] "=r" (low), [rt2] "=r" (high) : :);
        unsigned long rv = ((low & PAGE_MASK) | (entry & ~PAGE_MASK));
        return rv;
    }

#if MEMORY_DEBUG
    // Debug dumps
    static void dumpRangeFrames();
    static void dumpPageFrames(int numPages);
    static void dumpPageMaps(int numPages);
#endif

private:

    struct PageFrame {
        uint32_t pageNum;
        uint32_t owner;
    };

    struct RangeFrame {
        uint32_t startPageNum;
        uint32_t endPageNum;
    };

    static PageFrame pageFrames[TZ_MAX_NUM_PAGES];
    static int freePagesStart, freePagesEnd;

    static int allocPageMap[TZ_MAX_NUM_PAGES];
    static int freePageMap[TZ_MAX_NUM_PAGES];
    static const int PAGE_NOT_ALLOCATED = -1;
    static const int PAGE_NOT_FREE = -1;

    static RangeFrame rangeFrames[TZ_MAX_NUM_RANGES];
    static int numRanges;

    static SpinLock lock;

private:
    static void addRange(const unsigned long startAddrVirt, const unsigned long size);

    inline static uint32_t pageFrameNum(const PhysAddr pagePhysAddr) {
        return (unsigned long)pagePhysAddr/PAGE_SIZE_4K_BYTES;
    }

    inline static void * frameNumToPage(const uint32_t pageFrameNum) {
        return (void *)((uintptr_t)pageFrameNum * PAGE_SIZE_4K_BYTES);
    }

    inline static uint32_t pageMapIdx(const PhysAddr pagePhysAddr) {
        uint32_t mapIdx = 0;
        uint32_t pageNum = pageFrameNum(pagePhysAddr);
        for (int i = 0; i < numRanges; i++) {
            if (pageNum >= rangeFrames[i].startPageNum &&
                pageNum < rangeFrames[i].endPageNum) {
                mapIdx += pageNum - rangeFrames[i].startPageNum;
                break;
            }
            mapIdx += rangeFrames[i].endPageNum - rangeFrames[i].startPageNum;
        }
        return mapIdx;
    }

    inline static void swapPageFrames(PageFrame *left, PageFrame *right) {
        PageFrame tmp;
        tmp.owner = left->owner;
        tmp.pageNum = left->pageNum;

        left->owner = right->owner;
        left->pageNum = right->pageNum;

        right->owner = tmp.owner;
        right->pageNum = tmp.pageNum;
    }

private:
    TzMem() = delete;
    ~TzMem() = delete;

    TzMem& operator = (const TzMem& ) = delete;
};



#endif /* TZ_MEMORY_H_ */
