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
#include <cstdint>

#include "config.h"

#include "arm/arm.h"
#include "plat_config.h"
#include "platform.h"
#include "pgtable.h"
#include "tzmemory.h"

#include "brcmstb.h"

#include "lib_printf.h"
#include "lib_string.h"


/*
 * The bootloader in BRCM STB systems scrambles system memory upon bootup. Scrambling is done by
 * asking a hardware random-number-generator for to generate a random 256bit number and the entire
 * memory is then encrypted using this key. As a result the contents of memory are now
 * reasonable random.
 *
 * Create an un-predictable seed by doing a checksum on any given page of memory.
 */
size_t Platform::getRandomSeed(uint8_t *seed, size_t seedNumBytes) {
    static int callNum = 0;
    int numBytes = (seedNumBytes > PAGE_SIZE_4K_BYTES) ? PAGE_SIZE_4K_BYTES : seedNumBytes;

    /*
     * Allocate and map a page of memory
     *
     */
    PageTable *kernPageTable = PageTable::kernelPageTable();
    TzMem::VirtAddr vaddr = kernPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (vaddr == nullptr) {
        err_msg("Exhausted kernel heap space !\n");
        return 0;
    }
    TzMem::PhysAddr page = TzMem::allocPage(KERNEL_PID);
    if (page == nullptr) {
        err_msg("Exhausted physical memory !\n");
        kernelHalt("Out Of Memory");
    }
    kernPageTable->mapPage(vaddr, page, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);


    /* Replace every word on the page with a sum of the words that follow it.
     * This diffuses bits from all of the page into the first N words. We can
     * then use the first N words as the seed.
     */
    uint32_t *currWord = (uint32_t *)vaddr + PAGE_SIZE_4K_WORDS - 1;
    uint32_t *firstWord = (uint32_t *)vaddr;
    uint32_t sum = callNum;
    while (currWord >= firstWord) {
        sum += *currWord;
        *currWord = sum;

        currWord--;
    }

    memcpy(seed, vaddr, numBytes);

    kernPageTable->unmapPage(vaddr);
    TzMem::freePage(page);

    callNum++;
    return numBytes;
}
