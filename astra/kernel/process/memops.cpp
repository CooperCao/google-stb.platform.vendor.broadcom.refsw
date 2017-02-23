/******************************************************************************
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
 *****************************************************************************/
#include "hwtimer.h"
#include "arm/arm.h"
#include "arm/spinlock.h"

#include "tztask.h"
#include "tzmemory.h"
#include "tzmman.h"
#include "objalloc.h"
#include "kernel.h"
#include "scheduler.h"
#include "console.h"

#include "lib_printf.h"

void *TzTask::brk(void *newBrk) {
    uint8_t *start = (uint8_t *)brkStart;
    uint8_t *curr = (uint8_t *)brkCurr;
    uint8_t *desired = (uint8_t *)newBrk;
    uint8_t *max = (uint8_t *)brkMax;

    //printf("brk: start %p curr %p desired %p max %p\n", start, curr, desired, max);

    if ((desired < start) || (desired > max))
        return curr;

    int offset = desired - (uint8_t *)PAGE_START_4K(desired);
    if (offset > 0)
        desired = (uint8_t *)PAGE_START_4K(desired) + PAGE_SIZE_4K_BYTES;

    // Do we want to release memory ?
    if (desired <= curr) {
        // Yes, release memory by moving the current break.

        uint8_t *page = curr;
        while(page >= desired) {
            TzMem::PhysAddr pa = pageTable->lookUp(page);
            pageTable->unmapPage(page);
            if (pa != nullptr)
                TzMem::freePage(pa);

            page -= PAGE_SIZE_4K_BYTES;
        }

        brkCurr = desired;
        return brkCurr;
    }

    // We need to allocate memory:
    // Check whether we have unmapped VAs of desired range.
    if (desired > max) {
        // This allocation will exceed brk limit. Do not allow
        return brkCurr;
    }
    uint8_t *page = curr;
    while (page < desired) {
        TzMem::PhysAddr pa = pageTable->lookUp(page);
        if (pa != nullptr) {
            // we do not have the desired VA range available.
            return brkCurr;
        }

        page += PAGE_SIZE_4K_BYTES;
    }

    // We have the VA range available. Allocate pages and map them
    page = curr;
    while (page < desired) {
        TzMem::PhysAddr pa = TzMem::allocPage(tid);
        if (pa == nullptr)
            break;

        pageTable->mapPage(page, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_USER, false);
        page += PAGE_SIZE_4K_BYTES;
    }

    if (page != desired) {
        // Physical page allocation failed. Release all
        // allocated paged and fail
        uint8_t *allocPage = page;
        while (allocPage < page) {
            TzMem::PhysAddr pa = pageTable->lookUp(allocPage);
            pageTable->unmapPage(allocPage);
            if (pa != nullptr)
                TzMem::freePage(pa);

            allocPage += PAGE_SIZE_4K_BYTES;
        }

        return brkCurr;
    }

    brkCurr = desired;
    return brkCurr;
}

int TzTask::mmap(TzMem::VirtAddr addr, TzMem::VirtAddr *allocated, size_t len, int prot, int flags) {
    uint8_t *desired = (uint8_t *)addr;
    int pageOffset = desired - (uint8_t *)PAGE_START_4K(desired);
    if ((pageOffset != 0) && (flags & MAP_FIXED))
        return -EINVAL; // We can only map at page boundaries.

    uint8_t *va = (pageOffset == 0) ? desired : (uint8_t *)PAGE_START_4K(desired) + PAGE_SIZE_4K_BYTES;

    int numPages = (len/PAGE_SIZE_4K_BYTES) + 1;

    int accessPerms = MEMORY_ACCESS_RW_KERNEL;
    bool noExec = true;
    if ((prot & PROT_READ) && (prot & PROT_WRITE))
        accessPerms = MEMORY_ACCESS_RW_USER;
    if ((prot & PROT_READ) && (!(prot & PROT_WRITE)))
        accessPerms = MEMORY_ACCESS_RO_USER;
    if ((!(prot & PROT_READ)) && (prot & PROT_WRITE))
        accessPerms = MEMORY_ACCESS_RW_USER;

    if (prot & PROT_EXEC)
        noExec = false;

    bool shared = (flags & MAP_SHARED);

    if (!pageTable->isAddrRangeUnMapped(va, numPages*PAGE_SIZE_4K_BYTES)) {
        if (flags & MAP_FIXED){
            uint8_t *lastva = (uint8_t *)va + numPages*PAGE_SIZE_4K_BYTES - 1;
            pageTable->unmapPageRange(va, lastva);
        }else{
        va = (uint8_t *)pageTable->reserveAddrRange(va, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
        if (va == nullptr)
            return -ENOMEM;
        }
    }

    int rv = image->addMmapSection(va, numPages, accessPerms, noExec, shared, tid);
    if (rv != 0)
        return rv;

    *allocated = va;
    return 0;
}

int TzTask::munmap(TzMem::VirtAddr addr, size_t length) {
    UNUSED(length);
    uint8_t *desired = (uint8_t *)addr;
    int pageOffset = desired - (uint8_t *)PAGE_START_4K(desired);
    if (pageOffset != 0)
        return -EINVAL;

    image->removeMmapSection(addr);

    return 0;
}

int TzTask::mprotect(TzMem::VirtAddr va, size_t len, int prot) {
    int accessPerms = MEMORY_ACCESS_RW_KERNEL;
    bool noExec = true;

    if (prot & PROT_READ)
        accessPerms = MEMORY_ACCESS_RO_USER;

    if (prot & PROT_WRITE)
        accessPerms = MEMORY_ACCESS_RW_USER;

    if (prot & PROT_EXEC)
        noExec = false;

    uint8_t *curr = (uint8_t *)va;
    size_t mapped = 0;

    while (mapped < len) {
        pageTable->changePageAccessPerms(curr, accessPerms, noExec);
        curr += PAGE_SIZE_4K_BYTES;
        mapped += PAGE_SIZE_4K_BYTES;
    }

    return 0;
}
