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
    if(parentOffset<0) {
        kernelHalt("No tzioc node found in the device tree");
    }
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
        szCellSize = parseInt((void *)fpSzCells->data, propLen);

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


static bool setRR(TzIoc::TzIocMem::rrType rrX, uintptr_t *heapStart, uintptr_t *heapEnd)
{

    if((*heapStart != 0) && (*heapEnd != 0))
    {
        return true;
    }

    uintptr_t bspFeatureTableAddr = STB_REG_ADDR(STB_SUN_TOP_CTRL_BSP_FEATURE_TABLE_ADDR);
    //printf("bspFeatureTableAdd 0x%lx \n",bspFeatureTableAddr);

    uintptr_t bspFeatureTableHeaderOffset= REG_RD(bspFeatureTableAddr) + ARCH_DEVICE_BASE;
    //printf("bspFeatureTableHeaderOffset 0x%lx \n",bspFeatureTableHeaderOffset);

    uintptr_t rrTableOffset = REG_RD(bspFeatureTableHeaderOffset + 0x8) + ARCH_DEVICE_BASE;
    //printf("rrTableOffset 0x%lx \n",rrTableOffset);

    uint32_t rrTableHeader = REG_RD(rrTableOffset) + ARCH_DEVICE_BASE;
    //printf("rrTableHeader 0x%x \n",rrTableHeader);

    uint32_t rrTableHeaderVersion = rrTableHeader >> 24;
    uint32_t rrTableNumRegions    = rrTableHeader & 0xFF;
    //printf("rrTableHeaderVersion 0x%x rrTableNumRegions 0x%x \n",rrTableHeaderVersion, rrTableNumRegions);

    /* Only support Table version 2*/
    if((rrTableHeaderVersion == 0x2) && (rrTableNumRegions > 1) &&
        (rrX == TzIoc::TzIocMem::eCRR)){

        /* CRR is first entry of the table */
        uint32_t index     = 0;
        uintptr_t crrIndex  = (index * 2 * 4) + 4 + rrTableOffset;
        *heapStart = REG_RD(crrIndex);
        *heapEnd  = REG_RD(crrIndex + 4);
        //printf("TZIOC: CRR is Set: crrHeapStart 0x%lx crrHeapSize 0x%lx\n", *heapStart, *heapEnd);
        return true;
    }else{
        *heapStart = 0;
        *heapEnd  = 0;
        //printf("crrHeapStart 0x%lx crrHeapSize 0x%lx\n", *heapStart, *heapEnd);
    }
    return false;
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
    bool ns_memory = true; /* All memory mapped is NS by default*/

    for (idx = 0; idx < ucCount; idx++) {
        uintptr_t ulPaddr = pRegions[idx].ulPaddr;
        uint32_t  ulSize  = pRegions[idx].ulSize;
        uint32_t  ulFlags = pRegions[idx].ulFlags;

        if (ulPaddr == 0 || ulSize == 0) {
            err = -EINVAL;
            goto ERR_EXIT;
        }

        int startOffset = ulPaddr & ~PAGE_MASK;
        int endOffset   = startOffset + ulSize - 1;
        int numPages    = endOffset / PAGE_SIZE_4K_BYTES + 1;

        /* Check if PA is allowed to be mapped
           1. PA falls into MRR and KRR - Return Error
           2. If CRR is set then Map
                else Return Error
           3. PA falls in CRR memory - Check App ID
        */

        uintptr_t pageStart = ulPaddr & PAGE_MASK;
        uintptr_t pageEnd   = pageStart + (numPages * PAGE_SIZE_4K_BYTES);

        /* Check if the PA wrapped */
        if(pageStart > pageEnd){
            err = -EINVAL;
            goto ERR_EXIT;
        }

        /* PA should not be in MRR */
        if(((pageStart >= TzMem::mrrHeapStart) && (pageStart <= TzMem::mrrHeapEnd)) ||
           ((pageEnd   >= TzMem::mrrHeapStart) && (pageEnd   <= TzMem::mrrHeapEnd))){
            err = -EPERM;
            goto ERR_EXIT;
        }

        /* PA should not be in KRR */
        if(((pageStart >= TzMem::krrHeapStart) && (pageStart <= TzMem::krrHeapEnd)) ||
           ((pageEnd   >= TzMem::krrHeapStart) && (pageEnd   <= TzMem::krrHeapEnd))){
            err = -EPERM;
            goto ERR_EXIT;
        }

        /* Set RR only If RR needs to be secured by Astra */
        if(TzMem::secRegion == 1)
        {
            /* Check if CRR is set*/
            bool crr_set = setRR(TzIoc::TzIocMem::eCRR, &TzMem::crrHeapStart, &TzMem::crrHeapEnd);
            if(crr_set == false){
                err = -EINVAL;
                goto ERR_EXIT;
            }

            //printf("TZIOC: Heaps: \nMRR [0x%lx-0x%lx] \nKRR [0x%lx-0x%lx] \nCRR [0x%lx-0x%lx] \n",
            //       TzMem::mrrHeapStart, TzMem::mrrHeapEnd,
            //       TzMem::krrHeapStart, TzMem::krrHeapEnd,
            //       TzMem::crrHeapStart, TzMem::crrHeapEnd);

            if(((pageStart >= TzMem::crrHeapStart) && (pageStart <= TzMem::crrHeapEnd)) ||
               ((pageEnd   >= TzMem::crrHeapStart) && (pageEnd   <= TzMem::crrHeapEnd))){
                err = -EPERM;
                goto ERR_EXIT;
            }
        }
        void *paPageStart = PAGE_START_4K(ulPaddr);
        void *vaPageStart = pageTable->reserveAndMapAddrRange(
            paPageStart,
            numPages * PAGE_SIZE_4K_BYTES,
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
            ns_memory);                     // non-secure

        if (!vaPageStart) {
           err = -ENOMEM;
           goto ERR_EXIT;
        }
        uintptr_t ulVaddr =
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
        uintptr_t ulPaddr = pRegions[idx].ulPaddr;
        uintptr_t ulVaddr = pRegions[idx].ulVaddr;
        uint32_t  ulSize  = pRegions[idx].ulSize;

        // find mapping
        PaddrMap paddrMap = {NULL, 0, 0, 0};
        int numPaddrMaps = paddrMaps.numElements();

        for (int i = 0; i < numPaddrMaps; i++) {
            if (paddrMaps[i].pClient == pClient &&
                (paddrMaps[i].ulPaddr == ulPaddr ||
                 paddrMaps[i].ulVaddr == ulVaddr) &&
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
        void *vaPageStart = (void *)(paddrMap.ulVaddr - startOffset);
        pageTable->unmapPageRange(
            vaPageStart,
            (void *)((uintptr_t)vaPageStart + endOffset));
    }
    return 0;
}

int TzIoc::TzIocMem::paddr2vaddr(
    struct tzioc_client *pClient,
    uintptr_t ulPaddr,
    uintptr_t *pulVaddr)
{
    // find mapping
    PaddrMap *pPaddrMap = NULL;
    int numPaddrMaps = paddrMaps.numElements();

    for (int i = 0; i < numPaddrMaps; i++) {
        if (paddrMaps[i].pClient == pClient &&
            paddrMaps[i].ulPaddr <= ulPaddr &&
            paddrMaps[i].ulPaddr + paddrMaps[i].ulSize > ulPaddr) {

            /* no copy, no pop */
            pPaddrMap = &paddrMaps[i];
            break;
        }
    }

    if (!pPaddrMap)
        return -ENOENT;

    *pulVaddr = pPaddrMap->ulVaddr + (ulPaddr - pPaddrMap->ulPaddr);
    return 0;
}

int TzIoc::TzIocMem::vaddr2paddr(
    struct tzioc_client *pClient,
    uintptr_t ulVaddr,
    uintptr_t *pulPaddr)
{
    // find mapping
    PaddrMap *pPaddrMap = NULL;
    int numPaddrMaps = paddrMaps.numElements();

    for (int i = 0; i < numPaddrMaps; i++) {
        if (paddrMaps[i].pClient == pClient &&
            paddrMaps[i].ulVaddr <= ulVaddr &&
            paddrMaps[i].ulVaddr + paddrMaps[i].ulSize > ulVaddr) {

            /* no copy, no pop */
            pPaddrMap = &paddrMaps[i];
            break;
        }
    }

    if (!pPaddrMap)
        return -ENOENT;

    *pulPaddr = pPaddrMap->ulPaddr + (ulVaddr - pPaddrMap->ulVaddr);
    return 0;
}

void TzIoc::TzIocMem::cleanupClient(
    struct tzioc_client *pClient)
{
    // find mapping
    int numPaddrMaps = paddrMaps.numElements();

    for (int i = 0; i < numPaddrMaps; i++) {
        if (paddrMaps[i].pClient == pClient) {

            /* no copy, just pop */
            paddrMaps[i] = paddrMaps[numPaddrMaps - 1];
            paddrMaps.popBack(NULL);

            // continue on from current index
            i--;
            numPaddrMaps--;
        }
    }
}
