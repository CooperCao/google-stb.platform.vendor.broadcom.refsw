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

//Posix spec on Pipe
//http://man7.org/linux/man-pages/man2/pipe.2.html
//http://man7.org/linux/man-pages/man7/pipe.7.html

#include "tzpipe.h"
#include "tztask.h"
#include "atomic.h"
#include "eventqueue.h"
#include "tzioc_common.h"

tzutils::Vector<Pipe *> Pipe::pipeEntries;
ObjCacheAllocator<Pipe> Pipe::pipeAllocator;
SpinLock Pipe::creationLock;

void Pipe::init() {
    pipeEntries.init();
    pipeAllocator.init();

    spinLockInit(&creationLock);
}

void *Pipe::operator new (size_t sz) {
    UNUSED(sz);
    return pipeAllocator.alloc();
}

void Pipe::operator delete(void *pipeHandle) {
    pipeAllocator.free((Pipe *)pipeHandle);
}

IFile *Pipe::create(uint16_t uid, uint16_t gid, long flags, uint32_t perms) {
    return new Pipe(uid, gid, perms, flags);
}

uintptr_t __tzioc_offset2addr(uintptr_t ulOffset) { return ulOffset;}

Pipe::Pipe(uint16_t ownr, uint16_t grp, uint32_t permissions, long flags):File(ownr, grp, permissions) {
    SpinLocker locker(&creationLock);

    writeWaitQueue.init();
    readWaitQueue.init();

    nonBlock = (flags == O_NONBLOCK);
    numWriteRefs = 0;
    numReadRefs = 0;

    int numPages = (MaxPipeSize - 1) / PAGE_SIZE_4K_BYTES + 1;
    PageTable *kernPageTable = PageTable::kernelPageTable();

    // reserve a new continuous virtual address range
    TzMem::VirtAddr vaddr = kernPageTable->reserveAddrRange(
        (void *)KERNEL_HEAP_START,
        numPages * PAGE_SIZE_4K_BYTES,
        PageTable::ScanForward);

    if (vaddr == nullptr) {
        err_msg("%s: Exhausted kernel heap space!\n", __PRETTY_FUNCTION__);
        kernelHalt("Virtual Address Space exhausted");
    }

    TzMem::VirtAddr currVaddr = vaddr;

    for (int i = 0; i < numPages; i++) {
        TzMem::PhysAddr paddr = TzMem::allocPage(KERNEL_PID);
        if (paddr == nullptr) {
            err_msg("%s: Exhausted physical memory!\n", __PRETTY_FUNCTION__);
            kernelHalt("Out of memory");
        }

        kernPageTable->mapPage(currVaddr, paddr, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true);

        currVaddr = (uint8_t *)currVaddr + PAGE_SIZE_4K_BYTES;
    }
    vaddrBuf = vaddr;

    __tzioc_ring_init(
        &cBuf,
        (uintptr_t)vaddr,
        MaxPipeSize,
        TZIOC_RING_CREATE | TZIOC_RING_WRITE | TZIOC_RING_READ,
        __tzioc_offset2addr);

    pipeEntries.pushBack(this);
}

Pipe::~Pipe() {
    int numPages = (MaxPipeSize - 1) / PAGE_SIZE_4K_BYTES + 1;
//    printf("%s: free from %d pages\n", __PRETTY_FUNCTION__, numPages);

    PageTable *kernPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr vaddr = vaddrBuf;
    for (int i = 0; i < numPages; i++) {
        TzMem::PhysAddr paddr = kernPageTable->lookUp(vaddr);

        kernPageTable->unmapPage(vaddr);
        TzMem::freePage(paddr);

        vaddr = (uint8_t *)vaddr + PAGE_SIZE_4K_BYTES;
    }
}

int Pipe::addReadRef() {
    atomic_incr(&numReadRefs);
    return numReadRefs;
}

int Pipe::addWriteRef() {
    atomic_incr(&numWriteRefs);
    return numWriteRefs;
}

void Pipe::open(Pipe *file, bool isReadEnd) {
    file->addRef();
    if (isReadEnd) file->addReadRef();
    else  file->addWriteRef();
}

bool Pipe::close(Pipe *pipeHandle, bool isReadEnd) {
    SpinLocker locker(&creationLock);

    atomic_decr(&pipeHandle->numRefs);

    if (isReadEnd) atomic_decr(&pipeHandle->numReadRefs);
    else  atomic_decr(&pipeHandle->numWriteRefs);

    if (pipeHandle->numRefs == 0) {
        int numPipes = pipeEntries.numElements();
        for (int i=0; i<numPipes; i++) {
            if (pipeEntries[i] == pipeHandle) {
                pipeEntries[i] = pipeEntries[numPipes - 1];
                pipeEntries.popBack(nullptr);
                delete pipeHandle;
                return true;
            }
        }
    }

    if (isReadEnd)
        pipeHandle->writeWaitQueue.signalOne();
    else
        pipeHandle->readWaitQueue.signalOne();

    return false;
}

size_t Pipe::write(const void *data, const size_t numBytes, const uint64_t offset) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    uintptr_t wrOffset, chunkSize, remainingBytes = numBytes;
    int err = 0;
    UNUSED(offset);

    /* minimum 1 byte of unwritten space has to be reserved */

    do {
        chunkSize = remainingBytes;
        remainingBytes = ((MaxPipeSize - 1) < remainingBytes) ? (remainingBytes - (MaxPipeSize - 1)) : 0;
        chunkSize = (remainingBytes == 0) ? chunkSize: (MaxPipeSize - 1);

        do {
            if (!numReadRefs) {
                err = -EPIPE; // error if no readers
                break;
            }

            wrOffset = cBuf.ulWrOffset;
            uintptr_t space = ring_space(&cBuf, wrOffset, cBuf.ulRdOffset);

            if (space < chunkSize) {
                if (nonBlock){
                    err = -EAGAIN; //error if call is nonblocking
                    break;
                }
                    writeWaitQueue.unlockAndWait(&lock, TzTask::current(), -1);
                    spinLockAcquire(&lock);
            }
            else {
                err = ring_poke(&cBuf, wrOffset, (uint8_t *)data, chunkSize);
                break;
            }

        } while(1);

        if (err) return err; // partial writes are not supported

        wrOffset = ring_wrap(&cBuf, wrOffset, chunkSize);

        cBuf.ulWrOffset = wrOffset;
        // Wake up one task pending on this read wait queue.
        readWaitQueue.signalOne();
    }while (remainingBytes);

    return numBytes;
}

ssize_t Pipe::writev(const iovec *iov, int iovcnt, const uint64_t offset) {

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

size_t Pipe::read(const void *data, const size_t numBytes, const off_t offset) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_READ_BIT))
        return -EACCES;

    UNUSED(offset);
    uintptr_t readCount = 0;
    uintptr_t rdOffset;
    int err = 0;

    do {
        if (!numWriteRefs) return 0; // return EOF if no writers

        rdOffset = cBuf.ulRdOffset;
        readCount = ring_bytes(&cBuf, cBuf.ulWrOffset, rdOffset);

        if (!readCount) {
            if (nonBlock){
                err = -EAGAIN; //error if call is nonblocking
                break;
            }
                readWaitQueue.unlockAndWait(&lock, TzTask::current(), -1);
                spinLockAcquire(&lock);
        }
        else {
            readCount = (numBytes < readCount) ? numBytes : readCount;
            err = ring_peek(&cBuf, rdOffset, (uint8_t *)data, readCount);
            break;
        }
    }while(1);

    if (err) return err;
    rdOffset = ring_wrap(&cBuf, rdOffset, readCount);

    cBuf.ulRdOffset = rdOffset;
    // Wake up one task pending on this write wait queue.
    if (readCount)
        writeWaitQueue.signalOne();

    return readCount;
}

ssize_t Pipe::readv(const iovec *iov, int iovcnt, const uint64_t offset) {

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
