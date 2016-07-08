/***************************************************************************
 * Copyright (c)2016 Broadcom. All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2 (GPLv2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 ***************************************************************************/

#include "tzioc_common.h"

int __tzioc_heaps_init(
    struct tzioc_mem_heap *pHeaps,
    uint8_t *pTables,
    uint32_t ulHeapsOffset,
    uint32_t ulHeapsSize,
    struct tzioc_mem_heap_layout *pHeapsLayout)
{
    uint32_t ulTableIdx = 0;
    uint32_t ulHeapOffset = ulHeapsOffset;
    int i, j;

    UNUSED(ulHeapsSize);

    for (i = 0; i < TZIOC_MEM_HEAP_MAX; i++) {
        struct tzioc_mem_heap_layout *pHeapLayout = &pHeapsLayout[i];
        struct tzioc_mem_heap *pHeap = &pHeaps[i];
        uint8_t *pTable = &pTables[ulTableIdx];

        uint32_t ulBuffCount = pHeapLayout->ulBuffCount;
        uint32_t ulBuffSize  = pHeapLayout->ulBuffPages * TZIOC_MEM_PAGE_SIZE;

        pHeap->ulBuffSize   = ulBuffSize;
        pHeap->ulBuffCount  = ulBuffCount;
        pHeap->ulTableIdx   = ulTableIdx;
        pHeap->ulHeapOffset = ulHeapOffset;

        if (!ulBuffSize)
            break;

        for (j = 0; j < ulBuffCount; j++) {
            uint32_t ulBuffOffset = ulHeapOffset + ulBuffSize * j;
            uint32_t *pMagic = (uint32_t *)_tzioc_offset2addr(ulBuffOffset);

            *pMagic   = TZIOC_MEM_MAGIC_WORD;
            pTable[j] = TZIOC_CLIENT_ID_MAX;
        }

        ulTableIdx   += ulBuffCount;
        ulHeapOffset += ulBuffSize * ulBuffCount;
    }
    return 0;
}

int __tzioc_mem_alloc(
    uint8_t ucClientId,
    uint32_t ulSize,
    void **ppBuff)
{
    int i, j;

#ifdef TRACE_MEM_ALLOC
    LOGV("allocating memory, client %d, size %d", ucClientId, ulSize);
#endif

    if (!ppBuff) {
        return -EINVAL;
    }

    for (i = 0; i < TZIOC_MEM_HEAP_MAX; i++) {
        struct tzioc_mem_heap *pHeap;
        uint8_t *pTable;

        pHeap = &pTziocMemCB->heaps[i];
        pTable = &pTziocMemCB->tables[pHeap->ulTableIdx];

        if (!pHeap->ulBuffSize) {
            break;
        }

        if (ulSize > pHeap->ulBuffSize) {
            continue;
        }

        for (j = 0; j < pHeap->ulBuffCount; j++) {
            uint32_t ulBuffOffset;
            uint32_t *pMagic;

            if (pTable[j] != TZIOC_CLIENT_ID_MAX) {
                continue;
            }

            ulBuffOffset = pHeap->ulHeapOffset + pHeap->ulBuffSize * j;
            pMagic = (uint32_t *)_tzioc_offset2addr(ulBuffOffset);

            if (*pMagic != TZIOC_MEM_MAGIC_WORD) {
                LOGE("Memory corrupted at heap %d buffer %d", i, j);

                /* mark this buffer unusable */
                pTable[j] = TZIOC_CLIENT_ID_SYS;
                continue;
            }

            *pMagic = 0;
            pTable[j] = ucClientId;

#ifdef TRACE_MEM_ALLOC
            LOGV("allocated memory at %p (offset 0x%x)",
                 pMagic, (unsigned int)ulBuffOffset);
#endif
            *ppBuff = (void *)pMagic;
            return 0;
        }
    }
    return -ENOMEM;
}

int __tzioc_mem_free(
    uint8_t ucClientId,
    void *pBuff)
{
    uint32_t ulBuffOffset;
    int i, j;

#ifdef TRACE_MEM_ALLOC
    LOGV("freeing memory at %p, client %d", pBuff, ucClientId);
#endif

    if (!pBuff) {
        return -EINVAL;
    }

    ulBuffOffset = _tzioc_addr2offset((uint32_t)pBuff);

    for (i = 0; i < TZIOC_MEM_HEAP_MAX; i++) {
        struct tzioc_mem_heap *pHeap;
        uint8_t *pTable;

        pHeap = &pTziocMemCB->heaps[i];
        pTable = &pTziocMemCB->tables[pHeap->ulTableIdx];

        if (ulBuffOffset >= pHeap->ulHeapOffset &&
            ulBuffOffset <  pHeap->ulHeapOffset +
                            pHeap->ulBuffSize * pHeap->ulBuffCount) {
            uint32_t *pMagic;

            j = (ulBuffOffset - pHeap->ulHeapOffset) / pHeap->ulBuffSize;

            if (pTable[j] != ucClientId) {
                LOGE("Memory at %p (offset 0x%x) not allocated by client %d",
                     pBuff, (unsigned int)ulBuffOffset, ucClientId);
                return -EFAULT;
            }

            ulBuffOffset = pHeap->ulHeapOffset + pHeap->ulBuffSize * j;
            pMagic = (uint32_t *)_tzioc_offset2addr(ulBuffOffset);

            *pMagic   = TZIOC_MEM_MAGIC_WORD;
            pTable[j] = TZIOC_CLIENT_ID_MAX;

#ifdef TRACE_MEM_ALLOC
            LOGV("freed memory at %p (offset 0x%x)",
                 pMagic, (unsigned int)ulBuffOffset);
#endif
            return 0;
        }
    }
    return -ENOENT;
}
