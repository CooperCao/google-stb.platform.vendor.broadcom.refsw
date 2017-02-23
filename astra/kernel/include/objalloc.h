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

#ifndef OBJALLOC_H_
#define OBJALLOC_H_

#include <cstddef>
#include <new>

#include "arm/arm.h"
#include "arm/spinlock.h"
#include "plat_config.h"
#include "pgtable.h"
#include "tzmemory.h"

#include "lib_printf.h"

template <typename T>
class ObjCacheAllocator {
public:

    void init() {
        firstChunk = nullptr;
        spinLockInit(&lock);
    }

    inline unsigned int objSize() {
        // Round up the size of T to the next 64 byte boundary
        unsigned int rv = sizeof(T);
        rv += 64 - (rv % 64);

        return rv;
    }

    T* alloc() {
        SpinLocker locker(&lock);

        if (firstChunk == nullptr) {
            firstChunk = allocChunk();
            if (firstChunk == nullptr)
                return nullptr;
        }

        ChunkHead *currChunk = (ChunkHead *)firstChunk;
        while ((currChunk->nextFreeObj == nullptr) && (currChunk->nextChunk != nullptr))
            currChunk = currChunk->nextChunk;

        if (currChunk->nextFreeObj == nullptr) {
            currChunk->nextChunk = (ChunkHead *)allocChunk();
            if (currChunk->nextChunk == nullptr)
                return nullptr;

            ChunkHead *chunkHeader = currChunk->nextChunk;
            chunkHeader->prevChunk = currChunk;

            currChunk = currChunk->nextChunk;
        }

        void *rv = currChunk->nextFreeObj;
        T* nextFreeT = (T *)(*(unsigned long *)rv);
        currChunk->nextFreeObj = nextFreeT;

        currChunk->freeByteCount -= objSize();

        //printf("%s: alloc %p firstChunk %p\n", __PRETTY_FUNCTION__, rv, firstChunk);
        return (T *)rv;

    }

    void free(T* obj) {
        SpinLocker locker(&lock);

        int size = (int)objSize();
        char *cobj = (char *)obj;
        for (int i=0; i<size; i++)
            cobj[i] = 0xab;

        unsigned long chunkStart = (unsigned long)obj & ObjToChunkMask;
        ChunkHead *chunkHead = (ChunkHead *)chunkStart;

        // printf("%s: Free %p. ChunkStart 0x%lx\n", __PRETTY_FUNCTION__, obj, chunkStart);
        // printf("\tnextFreeObj %p \n", chunkHead->nextFreeObj);

        unsigned long *objStart = (unsigned long *)obj;
        *objStart = (unsigned long)(chunkHead->nextFreeObj);
        chunkHead->nextFreeObj = objStart;
        chunkHead->freeByteCount += objSize();

        // printf("\tfreeCount: %lu ChunkSize %d\n", chunkHead->freeCount, ChunkSize);

        if (chunkHead->freeByteCount == ChunkSize) {
            // Free this chunk
            // printf("\tprev %p next %p chunkHead %p\n", chunkHead->prevChunk, chunkHead->nextChunk, chunkHead);
            if (chunkHead->prevChunk != nullptr) {
                chunkHead->prevChunk->nextChunk = chunkHead->nextChunk;
                if (chunkHead->nextChunk != nullptr)
                    chunkHead->nextChunk->prevChunk = chunkHead->prevChunk;
            }
            else {
                // This is the first chunk in the chain
                firstChunk = chunkHead->nextChunk;
                if (chunkHead->nextChunk != nullptr)
                    chunkHead->nextChunk->prevChunk = nullptr;
            }

            freeChunk(chunkHead);
        }
    }

private:
    void *allocChunk() {

        PageTable *kernPageTable = PageTable::kernelPageTable();
        TzMem::VirtAddr vaddr = (void *)0x1;
        // Find a properly aligned virtual address
        void *searchBase = (void *)KERNEL_HEAP_START;
        while (true) {
            vaddr = kernPageTable->reserveAddrRange(searchBase, ChunkSize, PageTable::ScanForward);
            if (vaddr == nullptr) {
                err_msg("Exhausted kernel heap space !\n");
                return nullptr;
            }

            if (((unsigned long)vaddr & ObjToChunkMask) == (unsigned long)vaddr)
                break;

            kernPageTable->releaseAddrRange(vaddr, ChunkSize);

            long incr = ChunkSize - ((long)vaddr & (~ObjToChunkMask));

            searchBase = (uint8_t *)vaddr + incr;
        }

        TzMem::VirtAddr curr = vaddr;
        for (int i=0; i<ChunkSize; i += PAGE_SIZE_4K_BYTES) {
            TzMem::PhysAddr page = TzMem::allocPage(KERNEL_PID);
            if (page == nullptr) {
                err_msg("Exhausted physical memory !\n");
                kernelHalt("Out Of Memory");
            }
            kernPageTable->mapPage(curr, page, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

            curr = (uint8_t *)curr + PAGE_SIZE_4K_BYTES;
        }

        ChunkHead *chunkHead = (ChunkHead *)vaddr;
        uint8_t *chunkEnd = (uint8_t *)vaddr + ChunkSize;

        chunkHead->nextChunk = nullptr;
        chunkHead->prevChunk = nullptr;
        chunkHead->nextFreeObj = (uint8_t *)vaddr + sizeof(ChunkHead);
        chunkHead->freeByteCount = ChunkSize;

        uint8_t *currObj = (uint8_t *)(chunkHead->nextFreeObj);
        while ((currObj + (2 * objSize())) < chunkEnd) {
            unsigned long *t = (unsigned long *)currObj;
            *t = (unsigned long)(currObj + objSize());

            currObj += objSize();
        }

        *(unsigned long *)currObj = 0x0;

        // printf("Allocated new chunk %p\n", chunkHead);

        return chunkHead;
    }

    void freeChunk(void *chunk) {
        PageTable *kernPageTable = PageTable::kernelPageTable();
        TzMem::VirtAddr curr = chunk;

        //printf("%s: Free chunk %p\n", __FUNCTION__, chunk);

        for (int i=0; i<ChunkSize; i += PAGE_SIZE_4K_BYTES) {
            TzMem::PhysAddr phys = kernPageTable->lookUp(curr);
            kernPageTable->unmapPage(curr);
            TzMem::freePage(phys);

            // printf("\tUnmap %p. free %p\n", curr, phys);
            curr = (uint8_t *)curr + PAGE_SIZE_4K_BYTES;
        }
    }

private:
    void *firstChunk;
    SpinLock lock;

    struct ChunkHead {
        ChunkHead *nextChunk;
        ChunkHead *prevChunk;
        void *nextFreeObj;
        unsigned long freeByteCount;
    };

private:
    static const int ChunkSize = 16*1024;
    static const unsigned long ObjToChunkMask = ARCH_SPECIFIC_CHUNK_MASK;
};



#endif /* OBJALLOC_H_ */
