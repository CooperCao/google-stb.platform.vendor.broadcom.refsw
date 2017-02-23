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

#include "tzioc_common.h"

//#define TRACE_MEM_ALLOC

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
            uintptr_t *pMagic = (uintptr_t *)_tzioc_offset2addr(ulBuffOffset);

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
    LOGV("allocating memory, client %d, size %d", ucClientId, (int)ulSize);
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
            uintptr_t ulBuffOffset;
            uintptr_t *pMagic;

            if (pTable[j] != TZIOC_CLIENT_ID_MAX) {
                continue;
            }

            ulBuffOffset = pHeap->ulHeapOffset + pHeap->ulBuffSize * j;
            pMagic = (uintptr_t *)_tzioc_offset2addr(ulBuffOffset);

            if (*pMagic != TZIOC_MEM_MAGIC_WORD) {
                LOGE("Memory corrupted at heap %d buffer %d", i, j);

                /* mark this buffer unusable */
                pTable[j] = TZIOC_CLIENT_ID_SYS;
                continue;
            }

            *pMagic = 0;
            pTable[j] = ucClientId;

#ifdef TRACE_MEM_ALLOC
            LOGV("allocated memory at %p (offset 0x%zx)",
                 pMagic, (size_t)ulBuffOffset);
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
    uintptr_t ulBuffOffset;
    int i, j;

#ifdef TRACE_MEM_ALLOC
    LOGV("freeing memory at %p, client %d", pBuff, ucClientId);
#endif

    if (!pBuff) {
        return -EINVAL;
    }

    ulBuffOffset = _tzioc_addr2offset((uintptr_t)pBuff);

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
                LOGE("Memory at %p (offset 0x%zx) not allocated by client %d",
                     pBuff, (size_t)ulBuffOffset, ucClientId);
                return -EFAULT;
            }

            ulBuffOffset = pHeap->ulHeapOffset + pHeap->ulBuffSize * j;
            pMagic = (uint32_t *)_tzioc_offset2addr(ulBuffOffset);

            *pMagic   = TZIOC_MEM_MAGIC_WORD;
            pTable[j] = TZIOC_CLIENT_ID_MAX;

#ifdef TRACE_MEM_ALLOC
            LOGV("freed memory at %p (offset 0x%zx)",
                 pMagic, (size_t)ulBuffOffset);
#endif
            return 0;
        }
    }
    return -ENOENT;
}
