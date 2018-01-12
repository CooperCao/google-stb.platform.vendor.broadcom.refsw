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
#include "arm/arm.h"
#include "plat_config.h"
#include "system.h"
#include "mempool.h"
#include "pgtable.h"

#include "lib_printf.h"

MemPool::MemPool(int pid, size_t psize) : poolSize(psize), allocUnit(64), owner(pid) {
    PageTable *kernPageTable = PageTable::kernelPageTable();
    pool = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, poolSize, PageTable::ScanForward);
    if (pool == nullptr) {
        err_msg("Exhausted kernel heap space !\n");
        System::halt();
    }

    uint8_t *curr = (uint8_t *)pool;
    uint8_t *end = curr + poolSize;

    while (curr < end) {
        TzMem::PhysAddr pa = TzMem::allocPage(owner);
        if (pa == nullptr) {
            err_msg("%s: Out of physical memory\n", __PRETTY_FUNCTION__);
            System::halt();
        }

        kernPageTable->mapPage(curr, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);

        curr += PAGE_SIZE_4K_BYTES;
    }

    MemChunk *chunk = (MemChunk *)pool;
    chunk->size = poolSize - sizeof(MemChunk);
    chunk->next = nullptr;

    head = chunk;
}

MemPool::~MemPool() {
    PageTable *kernPageTable = PageTable::kernelPageTable();
    uint8_t *curr = (uint8_t *)pool;
    uint8_t *end = curr + poolSize;
    while (curr < end) {
        TzMem::PhysAddr pa = kernPageTable->lookUp(curr);
        TzMem::freePage(pa);
        kernPageTable->unmapPage(curr);
        curr += PAGE_SIZE_4K_BYTES;
    }
}

void *MemPool::alloc(const size_t size) {
    size_t padding = 0;
    if ((size % allocUnit) != 0)
        padding = allocUnit - (size % allocUnit);
    size_t allocSize = size + padding;

    // Search the free list for the first fit.
    MemChunk *curr = head;
    MemChunk **prev = &head;
    while ((curr != nullptr) && (curr->size < allocSize)) {
        prev = &curr->next;
        curr = curr->next;
    }
    if (curr == nullptr)
        return nullptr;

    // Don't split the chunk if the resulting chunk becomes too small.
    size_t remainSize = curr->size - allocSize;
    if (remainSize < sizeof(MemChunk) + allocUnit)
        remainSize = 0;

    if (remainSize != 0) {
        MemChunk *split = (MemChunk *)((uint8_t *)curr + sizeof(MemChunk) + allocSize);
        split->size = remainSize - sizeof(MemChunk);
        split->next = curr->next;
        *prev = split;
        //printf("%s: split chunk %p rv %p sz %d\n", __FUNCTION__, split, (uint8_t *)split + sizeof(MemChunk), split->size);

        curr->size = allocSize;
        curr->next = nullptr;
        return (void *)((uint8_t *)curr + sizeof(MemChunk));
    }

    *prev = curr->next;
    curr->next = nullptr;

    //printf("%s: alloc chunk %p rv %p sz %d\n", __FUNCTION__, curr, (uint8_t *)curr + sizeof(MemChunk), curr->size);
    return (void *)((uint8_t *)curr + sizeof(MemChunk));
}

void MemPool::free(void *allocedPtr) {
    MemChunk *chunk = (MemChunk *)((uint8_t *)allocedPtr - sizeof(MemChunk));

    if ((chunk->size == 0) || (chunk->size > poolSize - sizeof(MemChunk))) {
        err_msg("%s: Invalid ptr %p\n", __PRETTY_FUNCTION__, allocedPtr);
        System::halt();
    }

    // Use this chunk as head if there is no more free chunks.
    if (head == nullptr) {
        head = chunk;
        return;
    }

    // Check if this chunk can merge with the head.
    uint8_t *chunkEnd = (uint8_t *)chunk + sizeof(MemChunk) + chunk->size;
    uint8_t *headEnd = (uint8_t *)head + sizeof(MemChunk) + head->size;
    //printf("%s: head %p headSize %d headEnd %p \n\tchunk %p chunk->size %d chunkEnd %p\n", __FUNCTION__, head, head->size, headEnd, chunk, chunk->size, chunkEnd);
    if (chunk < head) {
        if (chunkEnd == (uint8_t *)head) {
            // The chunk merges to the left of the head
            chunk->size += sizeof(MemChunk) + head->size;
            chunk->next = head->next;

            //printf("%s: chunk %p merge to left of head %p headSize %d headNext %p\n", __FUNCTION__, chunk, head, head->size, head->next);
            head = chunk;
            return;
        }

        // The chunk could not merge with the head but it's address is
        // smaller than the head.
        chunk->next = head;
        //printf("%s: chunk %p left of head %p headSize %d headNext %p\n", __FUNCTION__, chunk, head, head->size, head->next);
        head = chunk;
        return;
    }
    else if (headEnd == (uint8_t *)chunk) {
        // The chunk merges to the right of the head
        head->size += sizeof(MemChunk) + chunk->size;

        // Can the larger head now merge with its neighbor ?
        headEnd = (uint8_t *)head + sizeof(MemChunk) + head->size;
        if (headEnd == (uint8_t *)head->next) {
            head->size += sizeof(MemChunk) + head->next->size;
            head->next = head->next->next;
        }

        //printf("%s: chunk %p merge to right of head %p headSize %d headNext %p\n", __FUNCTION__, chunk, head, head->size, head->next);
        return;
    }

    // Check if this chunk can merge with the next node in the
    // free list.
    MemChunk *curr = head->next;
    MemChunk *prev = head;
    while (curr != nullptr) {
        uint8_t *currEnd = (uint8_t *)curr + sizeof(MemChunk) + curr->size;
        if (chunk < curr) {
            if (chunkEnd == (uint8_t *)curr) {
                // The chunk merges to the left of the currNode
                chunk->size += sizeof(MemChunk) + curr->size;
                chunk->next = curr->next;

                prev->next = chunk;
                return;
            }

            // The chunk could not merge with the current node but it's address is
            // smaller than the head.
            chunk->next = curr;
            prev->next = chunk;
            return;
        }
        else if (currEnd == (uint8_t *)chunk) {
            // The chunk merges to the right of the head
            curr->size += sizeof(MemChunk) + chunk->size;

            // Can the larger node now merge with its neighbor ?
            currEnd = (uint8_t *)curr + sizeof(MemChunk) + curr->size;
            if (currEnd == (uint8_t *)curr->next) {
                curr->size += sizeof(MemChunk) + curr->next->size;
                curr->next = curr->next->next;
            }

            return;
        }

        prev = curr;
        curr = curr->next;
    }
}
