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

/*
 * futex.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: gambhire
 */


#include "futex.h"
#include "tztask.h"
#include "pgtable.h"
#include "system.h"

ObjCacheAllocator<Futex> Futex::allocator;
tzutils::Vector<Futex *> FutexStore::store;

void Futex::init() {
    allocator.init();
    FutexStore::init();
}

Futex::Futex(int *physAddr) : ctrAddr(physAddr){
    waitQueue.init();
    spinLockInit(&lock);

    PageTable *kernelPageTable = PageTable::kernelPageTable();
    TzMem::VirtAddr va = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanDirection::ScanForward);
    if (va == nullptr) {
        err_msg("Virtual address space exhausted\n");
        System::halt();
    }

    uint8_t *pageStart = (uint8_t *)PAGE_START_4K(physAddr);
    kernelPageTable->mapPage(va, pageStart, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
    int offset = (uint8_t *)physAddr - pageStart;

    ctrVA = (int *)((uint8_t *)va + offset);
}

Futex::~Futex() {
    uint8_t *pageStart = (uint8_t *)PAGE_START_4K(ctrVA);
    PageTable *kernelPageTable = PageTable::kernelPageTable();

    kernelPageTable->unmapPage(pageStart);
}

int Futex::wait(int val, uint64_t timeout) {
    spinLockAcquire(&lock);

    if (val != *ctrVA) {
        spinLockRelease(&lock);
        return -EWOULDBLOCK;
    }

    waitQueue.unlockAndWait(&lock, TzTask::current(), timeout);
    return 0;
}

int Futex::wake(int numTasks) {
    spinLockAcquire(&lock);

    int rc = waitQueue.signalSome(numTasks);

    spinLockRelease(&lock);
    return rc;
}

int Futex::requeue(int numTasks, Futex *other) {
    spinLockAcquire(&lock);
    int rc = waitQueue.signalSome(numTasks);
    waitQueue.migrateTasks(&other->waitQueue);

    spinLockRelease(&lock);
    return rc;
}

int Futex::cmpRequeue(int val, int numTasks, Futex *other) {
    spinLockAcquire(&lock);

    if (val != *ctrVA) {
        spinLockRelease(&lock);
        return -EWOULDBLOCK;
    }

    int rc = waitQueue.signalSome(numTasks);
    waitQueue.migrateTasks(&other->waitQueue);

    spinLockRelease(&lock);
    return rc;
}

void *Futex::operator new(size_t sz) {
    UNUSED(sz);
    void *rv = allocator.alloc();
    return rv;
}

void Futex::operator delete(void *futex) {
    allocator.free((Futex *)futex);
}
