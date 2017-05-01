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

#include "tzioc_mem.h"

#include "libfdt.h"
#include "parse_utils.h"
#include "lib_printf.h"

// Static non-const data from TzIocMem class
SpinLock TzIoc::TzIocMem::lock;
struct tzioc_mem_cb TzIoc::TzIocMem::memCB;
tzutils::Vector<TzIoc::TzIocMem::PaddrMap> TzIoc::TzIocMem::paddrMaps;

// Exported mem control block to common code
struct tzioc_mem_cb *pTziocMemCB;

void TzIoc::TzIocMem::init(void *devTree)
{
    // Get parent tzioc node
    int parentOffset = fdt_subnode_offset(devTree, 0, "tzioc");
    int propLen;

    // Parse #address-cells property of parent node
    unsigned long adCellSize;
    const struct fdt_property *fpAdCells =
        fdt_get_property(devTree, parentOffset, "#address-cells", &propLen);

    if ((!fpAdCells) || (propLen < sizeof(int)))
        adCellSize = 1;
    else
        adCellSize = parseInt((void *)fpAdCells->data, propLen);

    // Parse #size-cells property of parent node
    unsigned long szCellSize;
    const struct fdt_property *fpSzCells =
        fdt_get_property(devTree, parentOffset, "#size-cells", &propLen);

    if ((!fpSzCells) || (propLen < sizeof(int)))
        szCellSize = 1;
    else
        szCellSize = parseInt((void *)fpAdCells->data, propLen);

    int adByteSize = adCellSize * sizeof(int);
    int szByteSize = szCellSize * sizeof(int);

    // Get tz-heaps node
    int nodeOffset = fdt_subnode_offset(devTree, parentOffset, "tz-heaps");
    if (nodeOffset < 0) {
        kernelHalt("No tz-heaps node found in the device tree");
    }

    // Parse 'reg' property
    const struct fdt_property *fpHeapsReg =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);

    if ((!fpHeapsReg) || (propLen != adByteSize + szByteSize))
        kernelHalt("Invalid reg property in device tree tz-heaps node.");

    const char *regData = fpHeapsReg->data;
    uint32_t heapsOffset = (uint32_t)((adCellSize == 1) ?
         parseInt(regData, adByteSize) :
         parseInt64(regData, adByteSize));
    regData += adByteSize;

    uint32_t heapsSize = (uint32_t)((szCellSize == 1) ?
         parseInt(regData, szByteSize) :
         parseInt64(regData, szByteSize));
    regData += szByteSize;

    printf("TzIoc TZOS heaps at 0x%x, size 0x%x\n",
           (unsigned int)heapsOffset, (unsigned int)heapsSize);

    // Parse 'layout' property
    struct tzioc_mem_heap_layout heapsLayout[TZIOC_MEM_HEAP_MAX];
    memset(heapsLayout, 0, sizeof(heapsLayout));

    const struct fdt_property *fpHeapsLayout =
        fdt_get_property(devTree, nodeOffset, "layout", &propLen);

    if (!fpHeapsLayout) {
        printf("Invalid (or missing) layout property in device tree tz-heaps node\n");
        printf("Using default layout algorithm\n");

        // Use default layout algorithm
        uint32_t numPages = heapsSize >> TZIOC_MEM_PAGE_SHIFT;

        // Set initial heaps and assignment unit
        uint32_t numHeaps = TZIOC_MEM_HEAP_MAX;
        uint32_t numUnits = (1 << (numHeaps - 1)) * numHeaps;
        uint32_t unitPages  = (numPages + numUnits - 1) / numUnits;

        // Reduce number of heaps if pages are not enough
        while (numPages < ((1 << (numHeaps - 1)) * numHeaps)) {
            numHeaps--;
            unitPages = 1; /* unitPages can not go below 1 */
        }

        // Assign pages to heaps
        uint32_t availPages = numPages;
        for (int i = numHeaps - 1; i >= 0; i--) {
            uint32_t buffPages = unitPages << i;
            uint32_t heapPages = availPages / (i + 1);
            uint32_t buffCount = heapPages / buffPages;

            heapsLayout[i].ulBuffPages = buffPages;
            heapsLayout[i].ulBuffCount = buffCount;

            availPages -= buffPages * buffCount;
        }
    }
    else {
        const char *layoutData = fpHeapsLayout->data;
        uint32_t layoutCount = propLen / (sizeof(int) * 2);

        for (int i = 0; i < layoutCount; i++) {
            heapsLayout[i].ulBuffPages =
                parseInt(layoutData, sizeof(int));
            layoutData += sizeof(int);

            heapsLayout[i].ulBuffCount =
                parseInt(layoutData, sizeof(int));
            layoutData += sizeof(int);
        }
    }

    printf("TzIoc TZOS heaps layout ");
    for (int i = 0; i < TZIOC_MEM_HEAP_MAX; i++) {
        if (!heapsLayout[i].ulBuffPages) break;
        printf("(%d %d) ",
               (int)heapsLayout[i].ulBuffPages,
               (int)heapsLayout[i].ulBuffCount);
    }
    printf("\n");

    // Init spinlock
    spinLockInit(&lock);

    // Init shared memory

    // Init mem control block
    __tzioc_heaps_init(
        memCB.heaps,
        memCB.tables,
        heapsOffset,
        heapsSize,
        heapsLayout);

    // Export mem control block to common code
    pTziocMemCB = &memCB;

    printf("TzIoc mem module initialized\n");
}

void *TzIoc::TzIocMem::alloc(
    struct tzioc_client *pClient,
    uint32_t ulSize)
{
    void *pBuff;

    spinLockAcquire(&lock);
    int err = __tzioc_mem_alloc(pClient->id, ulSize, &pBuff);
    spinLockRelease(&lock);

    if (err) {
        printf("TzIoc mem alloc failed, client %d\n", pClient->id);
        return NULL;
    }

    return pBuff;
}

void TzIoc::TzIocMem::free(
    struct tzioc_client *pClient,
    void *pBuff)
{
    UNUSED(pClient);

    spinLockAcquire(&lock);
    __tzioc_mem_free(pClient->id, pBuff);
    spinLockRelease(&lock);
}

int TzIoc::TzIocMem::mapPaddr(
    struct tzioc_client *pClient,
    uintptr_t ulPaddr,
    uint32_t ulSize,
    uint32_t ulFlags,
    uintptr_t *pulVaddr)
{
    TzTask *task = (TzTask *)pClient->task;
    PageTable *pageTable = task->userPageTable();

    int startOffset = ulPaddr & ~PAGE_MASK;
    int endOffset   = startOffset + ulSize - 1;
    int numPages    = endOffset / PAGE_SIZE_4K_BYTES + 1;

    void *paPageStart = PAGE_START_4K(ulPaddr);
    void *vaPageStart = pageTable->reserveAddrRange(
        (void *)PAGE_SIZE_4K_BYTES,
        numPages * PAGE_SIZE_4K_BYTES,
        PageTable::ScanForward);

    if (!vaPageStart)
        return -ENOMEM;

    pageTable->mapPageRange(
        vaPageStart,
        (void *)((uintptr_t)vaPageStart + endOffset),
        paPageStart,
        (ulFlags & TZIOC_MEM_DEVICE) ?
            MAIR_DEVICE :               // device memory
            MAIR_MEMORY,                // memory with caching
        (ulFlags & TZIOC_MEM_RD_ONLY) ?
            MEMORY_ACCESS_RO_USER :     // read only
            MEMORY_ACCESS_RW_USER,      // read write
        (ulFlags & TZIOC_MEM_NO_EXEC) ?
            true :                      // no execute
            false,                      // allow execute
        true,                           // shared memory
        true);                          // non-secure

    uint32_t ulVaddr =
        (uintptr_t)vaPageStart + startOffset;

    // record mapping
    PaddrMap paddrMap = {
        pClient,
        ulPaddr,
        ulVaddr,
        ulSize};

    paddrMaps.pushBack(paddrMap);

    *pulVaddr = ulVaddr;
    return 0;
}

int TzIoc::TzIocMem::unmapPaddr(
    struct tzioc_client *pClient,
    uintptr_t ulPaddr,
    uint32_t ulSize)
{
    // find mapping
    PaddrMap paddrMap = {NULL, 0, 0, 0};
    int numPaddrMaps = paddrMaps.numElements();

    for (int i = 0; i < numPaddrMaps; i++) {
        if (paddrMaps[i].pClient == pClient &&
            paddrMaps[i].ulPaddr == ulPaddr &&
            paddrMaps[i].ulSize  == ulSize) {

            paddrMap = paddrMaps[i];
            paddrMaps[i] = paddrMaps[numPaddrMaps - 1];
            paddrMaps.popBack(NULL);
            break;
        }
    }

    if (!paddrMap.pClient)
        return -ENOENT;

    TzTask *task = (TzTask *)pClient->task;
    PageTable *pageTable = task->userPageTable();

    int startOffset = paddrMap.ulPaddr & ~PAGE_MASK;
    int endOffset   = startOffset + paddrMap.ulSize - 1;
    int numPages    = endOffset / PAGE_SIZE_4K_BYTES + 1;

    void *vaPageStart = (void *)(paddrMap.ulVaddr - startOffset);

    pageTable->unmapPageRange(
        vaPageStart,
        (void *)((uintptr_t)vaPageStart + endOffset));

    pageTable->releaseAddrRange(
        vaPageStart,
        numPages * PAGE_SIZE_4K_BYTES);

    return 0;
}

int TzIoc::TzIocMem::mapPaddrs(
    struct tzioc_client *pClient,
    uint8_t ucCount,
    struct tzioc_mem_region *pRegions)
{
    TzTask *task = (TzTask *)pClient->task;
    PageTable *pageTable = task->userPageTable();
    int idx;
    int err = 0;

    for (idx = 0; idx < ucCount; idx++) {
        uint32_t ulPaddr = pRegions[idx].ulPaddr;
        uint32_t ulSize  = pRegions[idx].ulSize;
        uint32_t ulFlags = pRegions[idx].ulFlags;

        if (ulPaddr == 0 || ulSize == 0) {
            err = -EINVAL;
            goto ERR_EXIT;
        }

        int startOffset = ulPaddr & ~PAGE_MASK;
        int endOffset   = startOffset + ulSize - 1;
        int numPages    = endOffset / PAGE_SIZE_4K_BYTES + 1;

        void *paPageStart = PAGE_START_4K(ulPaddr);
        void *vaPageStart = pageTable->reserveAddrRange(
            (void *)PAGE_SIZE_4K_BYTES,
            numPages * PAGE_SIZE_4K_BYTES,
            PageTable::ScanForward);

        if (!vaPageStart) {
            err = -ENOMEM;
            goto ERR_EXIT;
        }

        pageTable->mapPageRange(
            vaPageStart,
            (void *)((uintptr_t)vaPageStart + endOffset),
            paPageStart,
            (ulFlags & TZIOC_MEM_DEVICE) ?
                MAIR_DEVICE :               // device memory
                MAIR_MEMORY,                // memory with caching
            (ulFlags & TZIOC_MEM_RD_ONLY) ?
                MEMORY_ACCESS_RO_USER :     // read only
                MEMORY_ACCESS_RW_USER,      // read write
            (ulFlags & TZIOC_MEM_NO_EXEC) ?
                true :                      // no execute
                false,                      // allow execute
            true,                           // shared memory
            true);                          // non-secure

        uint32_t ulVaddr =
            (uintptr_t)vaPageStart + startOffset;

        // record mapping
        PaddrMap paddrMap = {
            pClient,
            ulPaddr,
            ulVaddr,
            ulSize};

        paddrMaps.pushBack(paddrMap);

        pRegions[idx].ulVaddr = ulVaddr;
    }
    return 0;

 ERR_EXIT:
    // unmap previous mapped memory regions
    if (idx > 0) {
        unmapPaddrs(pClient, idx, pRegions);
    }
    return err;
}

int TzIoc::TzIocMem::unmapPaddrs(
    struct tzioc_client *pClient,
    uint8_t ucCount,
    struct tzioc_mem_region *pRegions)
{
    TzTask *task = (TzTask *)pClient->task;
    PageTable *pageTable = task->userPageTable();
    int idx;

    for (idx = 0; idx < ucCount; idx++) {
        uint32_t ulPaddr = pRegions[idx].ulPaddr;
        uint32_t ulSize  = pRegions[idx].ulSize;

        // find mapping
        PaddrMap paddrMap = {NULL, 0, 0, 0};
        int numPaddrMaps = paddrMaps.numElements();

        for (int i = 0; i < numPaddrMaps; i++) {
            if (paddrMaps[i].pClient == pClient &&
                paddrMaps[i].ulPaddr == ulPaddr &&
                paddrMaps[i].ulSize  == ulSize) {

                paddrMap = paddrMaps[i];
                paddrMaps[i] = paddrMaps[numPaddrMaps - 1];
                paddrMaps.popBack(NULL);
                break;
            }
        }

        if (!paddrMap.pClient)
            continue;

        int startOffset = paddrMap.ulPaddr & ~PAGE_MASK;
        int endOffset   = startOffset + paddrMap.ulSize - 1;
        int numPages    = endOffset / PAGE_SIZE_4K_BYTES + 1;

        void *vaPageStart = (void *)(paddrMap.ulVaddr - startOffset);

        pageTable->unmapPageRange(
            vaPageStart,
            (void *)((uintptr_t)vaPageStart + endOffset));

        pageTable->releaseAddrRange(
            vaPageStart,
            numPages * PAGE_SIZE_4K_BYTES);
    }
    return 0;
}
