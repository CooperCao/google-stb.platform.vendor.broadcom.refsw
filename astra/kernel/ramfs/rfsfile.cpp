/******************************************************************************
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
 *****************************************************************************/

#include "system.h"
#include "fs/ramfs.h"
#include "eventqueue.h"
#include "poll.h"
#include "arm/spinlock.h"
#include "objalloc.h"
#include "tztask.h"
#include "tzmman.h"
#include "lib_string.h"

static ObjCacheAllocator<RamFS::File> fileAllocator;

static uint8_t devRandStore[sizeof(RamFS::RandNumGen)];

void RamFS::File::init() {
    fileAllocator.init();
}

IFile *RamFS::File::create(uint16_t uid, uint16_t gid,uint32_t perms) {
    return new RamFS::File(uid, gid, perms);
}

IFile *RamFS::RandNumGen::create() {
    return new(devRandStore) RamFS::RandNumGen();
}

RamFS::File::File(uint16_t uid, uint16_t gid, uint32_t perms) {

    for (int i=0; i<NumL1BlockEntries; i++)
        l1BlockTable[i] = nullptr;

    this->perms = perms;
    this->uid = uid;
    this->gid = gid;

    linkCount = 1;
    numRefs = 0;

    fileSize = 0;

    memset(&times.lastReadAt, 0, sizeof(struct timespec));
    TzClock::RealTime::time(&times.lastModifiedAt);
    TzClock::RealTime::time(&times.lastStatusChangeAt);

    spinlock_init("ramfs.file.lock", &lock);
}

int RamFS::File::addRef() {
    atomic_incr(&numRefs);
    return numRefs;
}

int RamFS::File::release() {
    atomic_decr(&numRefs);
    return numRefs;
}

IFile* RamFS::File::clone() {
    SpinLocker locker(&lock);

    atomic_incr(&linkCount);

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    return this;
}

void RamFS::File::destroy(IFile *fp) {
    RamFS::File *file = (RamFS::File *)fp;
    atomic_decr(&file->linkCount);
    if ((file->linkCount == 0) && (file->numRefs == 0))
        delete file;
}

RamFS::File::~File() {
    erase();
}

void RamFS::File::erase() {

    for (int i=0; i<NumL1BlockEntries; i++) {
        uint64_t *l2Entry = (uint64_t *)l1BlockTable[i];
        if (l2Entry == nullptr)
            continue;

        for (int j=0; j<NumL2BlockEntries; j++) {

            uint64_t *l3Entry = (uint64_t *)l2Entry[j];
            if (l3Entry == nullptr)
                continue;

            for (int k=0; k<NumL3BlockEntries; k++) {
                uint64_t *pageEntry = (uint64_t *)l3Entry[k];
                if (pageEntry == nullptr)
                    continue;

                freePage(pageEntry);
                l3Entry[k] = 0;
            }

            freePage(l3Entry);
            l2Entry[j] = 0;
        }

        freePage(l2Entry);
        l1BlockTable[i] = nullptr;
    }

    fileSize = 0;
}

long RamFS::File::numBlocks() const {

    long rv = 0;
    for (int i=0; i<NumL1BlockEntries; i++) {
        uint64_t *l2Entry = (uint64_t *)l1BlockTable[i];
        if (l2Entry == nullptr)
            continue;

        for (int j=0; j<NumL2BlockEntries; j++) {

            uint64_t *l3Entry = (uint64_t *)l2Entry[j];
            if (l3Entry == nullptr)
                continue;

            for (int k=0; k<NumL3BlockEntries; k++) {
                uint64_t *pageEntry = (uint64_t *)l3Entry[k];
                if (pageEntry == nullptr)
                    continue;

                rv++;
            }
        }
    }

    return rv;
}

void *RamFS::File::operator new(size_t sz) {
    UNUSED(sz);
    return fileAllocator.alloc();
}

void* RamFS::RandNumGen::operator new(size_t sz, void *where) {
    UNUSED(sz);
    RandNumGen *rg = (RandNumGen *)where;

    return rg;
}

void RamFS::File::operator delete(void *f){
    RamFS::File *fp = (RamFS::File *)f;
    fileAllocator.free(fp);
}

void RamFS::File::open(RamFS::File *file) {
    file->addRef();
}

void RamFS::File::close(RamFS::File *file) {
    file->release();
    if ((file->numRefs == 0) && (file->linkCount == 0))
        delete file;
}

void RamFS::File::addWatcher(short pollEvent, short *pollResult, EventQueue *equeue) {

    // This is an in-memory file system. Reads and writes never block.
    *pollResult = 0;
    if (pollEvent & POLLIN)
        *pollResult |= POLLIN;
    if (pollEvent & POLLOUT)
        *pollResult |= POLLOUT;

    if (*pollResult != 0)
        equeue->signal();
}

bool RamFS::File::checkPermissions(int permsBit) {

    TzTask *currTask = TzTask::current();
    if (currTask == nullptr)
        return true;

    uint16_t owner = currTask->owner();
    uint16_t group = currTask->group();

    if ((permsBit != PERMS_READ_BIT) && (permsBit != PERMS_WRITE_BIT) && (permsBit != PERMS_EXECUTE_BIT)) {
        err_msg("%s: Bad permsBit 0x%x\n", __PRETTY_FUNCTION__, permsBit);
        return false;
    }

    // Root has all permissions
    if (owner == System::UID)
        return true;

    if (owner == uid) {
        if (!(OWNER_ACCESS_PERMS(perms) & permsBit))
            return false;
        else
            return true;
    }

    if (group == gid) {
        if (!(GROUP_ACCESS_PERMS(perms) & permsBit))
            return false;
        else
            return true;
    }

    if (!(OTHERS_ACCESS_PERMS(perms) & permsBit))
            return false;

    return true;
}

int RamFS::File::changePermissions(uint32_t newPerms) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    perms = newPerms;
    return 0;
}

int RamFS::File::changeOwner(uint16_t newOwner) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    uid = newOwner;
    return 0;
}

int RamFS::File::changeGroup(uint16_t newGroup) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    gid = newGroup;
    return 0;
}

size_t RamFS::File::read(const void *data, const size_t numBytes, const off_t offset) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_READ_BIT))
        return -EACCES;

    TzClock::RealTime::time(&times.lastReadAt);

    uint8_t *rdData = (uint8_t *)data;

    uint64_t readStart = offset;
    if (readStart > fileSize)
        return -EINVAL;

    uint64_t readStop = readStart + numBytes - 1;
    if (readStop >= fileSize)
        readStop = fileSize - 1;

    size_t rv = fetchFromBlockTable(rdData, readStart, readStop);
    return rv;
}

uint8_t *RamFS::File::offsetToPage(uint64_t offset) {
    int l1Idx = l1Index(offset);
    uint64_t *l2Block = (uint64_t *)l1BlockTable[l1Idx];
    if (l2Block == nullptr){
        printf("l1Idx %d null l2Block\n", l1Idx);
        return nullptr;
    }

    int l2Idx = l2Index(offset);
    uint64_t *l3Block = (uint64_t *)l2Block[l2Idx];
    if (l3Block == nullptr){
        printf("l2Idx %d null l3Block\n", l2Idx);
        return nullptr;
    }

    int l3Idx = l3Index(offset);
    uint8_t *l3Entry = (uint8_t *)l3Block[l3Idx];
    printf("l3Idx %d entry %p\n", l3Idx, l3Entry);
    return l3Entry;
}

size_t RamFS::File::fetchFromBlockTable(uint8_t *data, uint64_t absStartPos, uint64_t absStopPos) {

    // printf("\n%s: enter [ %p ]\n", __PRETTY_FUNCTION__, data);

    uint64_t startPos = absStartPos;
    uint64_t stopPos = absStopPos;
    uint8_t *rdData = data;

    size_t rv = 0;
    uint64_t currPos = startPos;

    int l1Start = l1Index(startPos);
    int l1Stop = l1Index(stopPos);
    // printf("l1Start %d l1Stop %d\n", l1Start, l1Stop);

    for (int i=l1Start; i<=l1Stop; i++) {
        uint64_t *l2Block = (uint64_t *)l1BlockTable[i];
        if (l2Block == nullptr) {
            rdData += L1RangeSize;
            currPos += L1RangeSize;

            continue;
        }

        int l2Start = l2Index(currPos);
        int l2Stop = (i == l1Stop) ? l2Index(stopPos) : NumL2BlockEntries-1;
        // printf("l2Start %d l2Stop %d\n", l2Start, l2Stop);

        for (int j=l2Start; j<=l2Stop; j++) {
            uint64_t *l3Block = (uint64_t *)l2Block[j];
            if (l3Block == nullptr) {
                rdData += L2RangeSize;
                currPos += L2RangeSize;

                continue;
            }

            int l3Start = l3Index(currPos);
            int l3Stop = ((j == l2Stop) && (i == l1Stop)) ? l3Index(stopPos) : NumL3BlockEntries-1;
            // printf("l3Start %d l3Stop %d\n", l3Start, l3Stop);

            for (int k=l3Start; k<=l3Stop; k++) {
                uint8_t *l3Entry = (uint8_t *)l3Block[k];
                if (l3Entry == nullptr) {
                    rdData += L3RangeSize;
                    currPos += L3RangeSize;

                    continue;
                }

                uint8_t *startAt = l3Entry + pageIndex(currPos);
                uint8_t *stopAt;
                if ((k == l3Stop) && (j == l2Stop) && ( i == l1Stop))
                    stopAt = l3Entry + pageIndex(stopPos);
                else
                    stopAt = l3Entry + PAGE_SIZE_4K_BYTES - 1;

                // printf("startAt %p stopAt %p\n", startAt, stopAt);
                int nb = stopAt - startAt + 1;
                memcpy(rdData, startAt, nb);

                rdData += nb;
                currPos += nb;
                rv += nb;
            }
        }
    }

    return rv;

}

size_t RamFS::File::write(const void *data, const size_t numBytes, const uint64_t offset) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    TzClock::RealTime::time(&times.lastModifiedAt);

    size_t rv = 0;
    uint8_t *wrData = (uint8_t *)data;

    uint64_t wrStart = offset;
    uint64_t wrStop = wrStart + numBytes - 1;

    rv = writeToBlockTable(wrData, wrStart, wrStop);

    if ((offset + rv) > fileSize)
        fileSize = offset+rv;

    return rv;
}

ssize_t RamFS::File::writev(const iovec *iov, int iovcnt, const uint64_t offset) {

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    uint64_t currOffset = offset;
    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        if (iov[i].iov_len == 0)
            continue;

        size_t nw = write(iov[i].iov_base, iov[i].iov_len, currOffset);
        rv += nw;
        currOffset += nw;
    }

    return rv;
}

ssize_t RamFS::File::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

    if (!checkPermissions(PERMS_READ_BIT))
        return -EACCES;

    uint64_t currOffset = offset;
    ssize_t rv = 0;
    for (int i=0; i<iovcnt; i++) {
        if (iov[i].iov_len == 0)
            continue;

        size_t nr = read(iov[i].iov_base, iov[i].iov_len, currOffset);
        rv += nr;
        currOffset += nr;
    }

    return rv;
}

void RamFS::File::freeze() {
    PageTable *kernPageTable = PageTable::kernelPageTable();

    for (int i=0; i<NumL1BlockEntries; i++) {
        uint64_t *l2Block = (uint64_t *)l1BlockTable[i];
        if (l2Block == nullptr)
            continue;

        kernPageTable->changePageAccessPerms(l2Block, MEMORY_ACCESS_RO_KERNEL, false);

        for (int j=0; j<NumL2BlockEntries; j++) {
            uint64_t *l3Block = (uint64_t *)l2Block[j];
            if (l3Block == nullptr)
                continue;

            kernPageTable->changePageAccessPerms(l3Block, MEMORY_ACCESS_RO_KERNEL, false);

            for (int k=0; k<NumL3BlockEntries; k++) {
                uint8_t * l3Entry = (uint8_t *)l3Block[k];
                if (l3Entry == nullptr)
                    continue;

                kernPageTable->changePageAccessPerms(l3Entry, MEMORY_ACCESS_RO_KERNEL, false);
            }
        }
    }
}

size_t RamFS::File::writeToBlockTable(uint8_t *data, uint64_t absStartPos, uint64_t absStopPos) {

    // printf("\n%s: enter [ %p ]\n", __PRETTY_FUNCTION__, data);

    uint64_t startPos = absStartPos;
    uint64_t stopPos = absStopPos;
    uint8_t *wrData = data;

    size_t rv = 0;
    uint64_t currPos = startPos;

    int l1Start = l1Index(startPos);
    int l1Stop = l1Index(stopPos);
    // printf("l1Start %d l1Stop %d\n", l1Start, l1Stop);

    for (int i=l1Start; i<=l1Stop; i++) {
        uint64_t *l2Block = (uint64_t *)l1BlockTable[i];
        if (l2Block == nullptr) {
            l2Block = (uint64_t *)allocPage();
            if (l2Block == nullptr)
                return rv;

            // printf("%d: l2Block %p\n", i, l2Block);
            l1BlockTable[i] = l2Block;
        }

        int l2Start = l2Index(currPos);
        int l2Stop = (i == l1Stop) ? l2Index(stopPos) : NumL2BlockEntries-1;
        // printf("l2Start %d l2Stop %d\n", l2Start, l2Stop);

        for (int j=l2Start; j<=l2Stop; j++) {
            uint64_t *l3Block = (uint64_t *)l2Block[j];
            if (l3Block == nullptr) {
                l3Block = (uint64_t *)allocPage();
                if (l3Block == nullptr)
                    return rv;

                // printf("%d: l3Block %p\n", j, l3Block);
                l2Block[j] = (uint64_t)l3Block;
            }

            int l3Start = l3Index(currPos);
            int l3Stop = ((j == l2Stop) && (i == l1Stop)) ? l3Index(stopPos) : NumL3BlockEntries-1;
            // printf("l3Start %d l3Stop %d\n", l3Start, l3Stop);
            for (int k=l3Start; k<=l3Stop; k++) {
                uint8_t *l3Entry = (uint8_t *)l3Block[k];
                if (l3Entry == nullptr) {
                    l3Entry = (uint8_t *)allocPage();
                    if (l3Entry == nullptr)
                        return rv;

                    // printf("l3Entry %p\n", l3Entry);
                    l3Block[k] = (uint64_t)l3Entry;
                }

                uint8_t *startAt = l3Entry + pageIndex(currPos);
                uint8_t *stopAt;
                if ((k == l3Stop) && (j == l2Stop) && (i == l1Stop))
                    stopAt = l3Entry + pageIndex(stopPos);
                else
                    stopAt = l3Entry + PAGE_SIZE_4K_BYTES - 1;

                // printf("startAt %p stopAt %p\n", startAt, stopAt);
                int nb = stopAt - startAt + 1;
                memcpy(startAt, wrData, nb);

                wrData += nb;
                currPos += nb;
                rv += nb;

                // printf("wrData %p rv %d\n", (uint8_t *)wrData, rv);
            }
        }
    }

    return rv;
}

TzMem::VirtAddr RamFS::File::allocPage() {
    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr vaddr = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (vaddr == nullptr) {
        err_msg("virtual address space exhausted\n");
        return nullptr;
    }

    TzMem::PhysAddr page = TzMem::allocPage(KERNEL_PID);
    if (page == nullptr) {
        err_msg("Exhausted physical memory !\n");
        return nullptr;
    }

    kernPageTable->mapPage(vaddr, page, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

    unsigned long *curr = (unsigned long *)vaddr;
    unsigned long *end = (unsigned long *)((uint8_t *)curr + PAGE_SIZE_4K_BYTES);
    while (curr != end) {
        *curr = 0UL;
        curr++;
    }

    return vaddr;
}

void RamFS::File::freePage(TzMem::VirtAddr vaddr) {
    PageTable *kernPageTable = PageTable::kernelPageTable();
    TzMem::PhysAddr phys = kernPageTable->lookUp(vaddr);
    kernPageTable->unmapPage(vaddr);
    TzMem::freePage(phys);
}

void RamFS::File::accessTimes(AccessTimes *tm) {
    memcpy(tm, &times, sizeof(struct AccessTimes));
}

static char zeroPage[PAGE_SIZE_4K_BYTES];

int RamFS::File::mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset) {
    uint8_t *va = (uint8_t *)addr;

    if ((prot & PROT_READ) && (!checkPermissions(PERMS_READ_BIT)))
        return -EACCES;
    if ((prot & PROT_WRITE) && (!checkPermissions(PERMS_WRITE_BIT)))
        return -EACCES;
    if ((prot & PROT_EXEC) && (!checkPermissions(PERMS_EXECUTE_BIT)))
        return -EACCES;

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
    bool sharedMem = (flags & MAP_SHARED);

    PageTable *pageTable = TzTask::current()->userPageTable();
    PageTable *kernelPageTable = PageTable::kernelPageTable();

    int numPages = (length/PAGE_SIZE_4K_BYTES) + 1;
    int pageOffset = offset - (uint64_t)PAGE_START_4K(offset);

    uint8_t *curr = va;
    uint64_t currOffset = offset;
    uint8_t *stop = va + (numPages * PAGE_SIZE_4K_BYTES);

    // printf("----> va %p stop %p numPages %d length %d\n", va, stop, numPages, length);

    while (curr < stop) {
        uint8_t *filePage = offsetToPage(currOffset);
        if (filePage == nullptr) {
            // We ran into a unmapped hole. Try to fill it with zeros.
            int rv = write(zeroPage, PAGE_SIZE_4K_BYTES, currOffset);
            if (rv < PAGE_SIZE_4K_BYTES)
                break;

            // We could fill the hole. Retry
            continue;
        }

        TzMem::PhysAddr pa = kernelPageTable->lookUp(filePage);
        pageTable->mapPage(curr, pa, MAIR_MEMORY, accessPerms, noExec, sharedMem);
        curr += PAGE_SIZE_4K_BYTES;
        currOffset += PAGE_SIZE_4K_BYTES;
    }

    *mappedAddr = va + pageOffset;
    return 0;

}
