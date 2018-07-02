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


#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "arm/arm.h"
#include "config.h"
#include "plat_config.h"
#include "uart_boot.h"

uintptr_t early_uart_base;

__init_data unsigned long phys_to_virt_offset;
__init_data unsigned long virt_to_phys_offset;

__init_data  ALIGN_PT uint64_t bootstrap_pt_blocks[NUM_BOOTSTRAP_BLOCKS * KERNEL_PAGE_TABLE_BLOCK_SIZE];
__init_data  int bootstrapNextBlock;

__init_data static uint64_t *pgTableTTBR0;
__init_data static uint64_t *pgTableTTBR1;

extern uintptr_t _bootstrap_img_start;
extern uintptr_t _bootstrap_img_end;

extern uintptr_t _kernel_img_start;
extern uintptr_t _kernel_img_end;

__bootstrap static inline uint64_t *bootstrapBlockAlloc()
{
    if (bootstrapNextBlock == NUM_BOOTSTRAP_BLOCKS) {
    // Need to increase NUM_BOOTSTRAP_BLOCKS
        while (1) {
            asm volatile("wfi":::);
        }
    }

    int idx = bootstrapNextBlock;
    uint8_t *rv = (uint8_t *)bootstrap_pt_blocks + (idx * KERNEL_PAGE_TABLE_BLOCK_SIZE * 8);
    bootstrapNextBlock++;

    uint64_t *pt = (uint64_t *)rv;
    for (int i=0; i<GRANULE_4KB_TABLE_NUM_ENTRIES; i++)
        pt[i] = 0UL;

    return pt;
}

__bootstrap static inline void bootstrapMapMemRange(uint64_t **pageTable, uint8_t *rangeBegin, uint8_t *rangeEnd,
            int memoryAttr, uint8_t *translatedRangeBegin)
{

    uint8_t *currPage = rangeBegin;
    uint8_t *translatedPage = translatedRangeBegin;

    uint64_t *l0Table = *pageTable;

    while (currPage <= rangeEnd) {

    // L0 table
    const int l0Idx = L0_PAGE_TABLE_SLOT(currPage);
    if (l0Table[l0Idx] == 0) {
        uint64_t *table = bootstrapBlockAlloc();
        l0Table[l0Idx] = MAKE_TABLE_DESCRIPTOR(table);
    }
    uint64_t *l1Table = (uint64_t *)TABLE_PHYS_ADDR(l0Table[l0Idx]);

    // L1 table
    const int l1Idx = L1_PAGE_TABLE_SLOT(currPage);
    if (l1Table[l1Idx] == 0) {
        uint64_t *table = bootstrapBlockAlloc();
        l1Table[l1Idx] = MAKE_TABLE_DESCRIPTOR(table);
    }
    uint64_t *l2Table = (uint64_t *)TABLE_PHYS_ADDR(l1Table[l1Idx]);

    // L2 table
    const int l2Idx = L2_PAGE_TABLE_SLOT(currPage);
    if (l2Table[l2Idx] == 0) {
        uint64_t *table = bootstrapBlockAlloc();
        l2Table[l2Idx] = MAKE_TABLE_DESCRIPTOR(table);
    }
    uint64_t *l3Table = (uint64_t *)TABLE_PHYS_ADDR(l2Table[l2Idx]);

    // L3 table
    const int l3Idx = L3_PAGE_TABLE_SLOT(currPage);
    uint64_t l3Entry = MAKE_PAGE_DESCRIPTOR(translatedPage);
    CHANGE_PAGE_DESCRIPTOR_AF(l3Entry, PAGE_DESCRIPTOR_PAGE_PRESENT);
    CHANGE_PAGE_DESCRIPTOR_SH(l3Entry, PAGE_DESCRIPTOR_PAGE_INNER_SHAREABLE);
    CHANGE_PAGE_DESCRIPTOR_AP(l3Entry, PAGE_DESCRIPTOR_EL1RW_EL0N);
    CHANGE_PAGE_DESCRIPTOR_AI(l3Entry, memoryAttr);

    l3Table[l3Idx] = l3Entry;

        currPage += PAGE_SIZE_4K_BYTES;
        translatedPage += PAGE_SIZE_4K_BYTES;
    }
}

__bootstrap static inline void makeBootstrapPageTables(void *dtreePhysAddr, void *uartAddr)
{
    bootstrapNextBlock = 0;

    pgTableTTBR0 = bootstrapBlockAlloc();
    pgTableTTBR1 = bootstrapBlockAlloc();

    register uint8_t *bootstrapStart;
    register uint8_t *bootstrapEnd;
    asm volatile("adr %[rt], _bootstrap_img_start":[rt] "=r" (bootstrapStart)::);
    asm volatile("adr %[rt], _bootstrap_img_end":[rt] "=r" (bootstrapEnd)::);
    bootstrapMapMemRange(&pgTableTTBR0, bootstrapStart, bootstrapEnd, MAIR_MEMORY, bootstrapStart);

    register uint8_t *kernelLinkAddr;
    register uint8_t *kernelLinkAddrEnd;
    register uint8_t *kernelLoadAddr;

    asm volatile("ldr %[rt], =_kernel_img_start":[rt] "=r" (kernelLinkAddr)::);
    asm volatile("ldr %[rt], =_kernel_img_end":[rt] "=r" (kernelLinkAddrEnd)::);
    asm volatile("adr %[rt], _kernel_img_start":[rt] "=r" (kernelLoadAddr)::);
    bootstrapMapMemRange(&pgTableTTBR1, kernelLinkAddr, kernelLinkAddrEnd, MAIR_MEMORY, kernelLoadAddr);

    /* Backward compatibility */
    if (!uartAddr) uartAddr = (void *)UART0_BASE;

    uint8_t *puart_start = PAGE_START_4K(uartAddr);
    uint8_t *puart_end = puart_start;
    uint8_t *vtranslated_uart_start = (uint8_t *)(ARCH_DEVICE_BASE + puart_start);
    bootstrapMapMemRange(&pgTableTTBR1, puart_start, puart_end, MAIR_DEVICE, vtranslated_uart_start);
    early_uart_base = (uintptr_t) vtranslated_uart_start;
    /* TODO: Detect & Change UART Type at Runtime */
    early_uart_init(UART_TYPE_NS16550a);

    register uint8_t *dtStart = (uint8_t *)dtreePhysAddr;
    register uint8_t *dtEnd = dtStart + MAX_DT_SIZE_BYTES - 1;
    bootstrapMapMemRange(&pgTableTTBR0, dtStart, dtEnd, MAIR_MEMORY, dtStart);
}

__bootstrap static inline void enableMMU()
{
    asm volatile("dsb sy":::);

    /* Setup the memory indirection attribute register */
    register unsigned long mair = MAIR_EL1_VAL;
    asm volatile("msr MAIR_EL1, %[rt]"::[rt] "r" (mair):"memory");

    /* read ID_AA64MMFR0_EL1 to derive IPA size supported by this CPU */
    register unsigned long idAA64;
    asm volatile("mrs %[xt], ID_AA64MMFR0_EL1":[xt] "=r" (idAA64)::);
    const unsigned long trGran4 = (idAA64 >> 28) & 0xfUL;
    const unsigned long asid = (idAA64 >> 4) & 0xfUL;
    const unsigned long paRange = idAA64 & 0x7UL;

    if (trGran4 != 0) {
        // The CPU does not support 4KB pages !
        while (1) {
            asm volatile("wfi":::);
        }
    }

    /* Setup TCR_EL1:
     * - 48bit VAs
     */
    register unsigned long tcrEL1 = (asid == 0x2UL) ? 0x1UL << 36 : 0x0UL;    // 16-bit ASIDs
    tcrEL1 |= (paRange << 32)   // Use the largest PA supported by the CPU
              | (0x2UL << 30)   // 4KB granule for TTBR1_EL1
              | (0x3UL << 28)   // TTBR1_EL1 table walks are inner shareable
              | (0x1UL << 26)   // TTBR1_EL1 tables are in cacheable memory
              | (0x1UL << 24)   // TTBR1_EL1 tables are inner cacheable
              | (16UL << 16)    // 48-bit VAs with TTBR_EL1
              | (0x3UL << 12)   // TTBR0_EL1 table walks are inner shareable
              | (0x2UL << 10)   // TTBR0_EL1 tables are in cacheable memory
              | (0x1UL << 8)    // TTBR0_EL1 tables are inner cacheable
              | 16UL            // 48-bit VAs with TTBR_EL0
              ;
    asm volatile("msr TCR_EL1, %[xt]"::[xt] "r" (tcrEL1):);

    /* Point both ttbr0_el1 and ttbr1_el1 to the page table */
    asm volatile("msr TTBR0_EL1, %[xt]"::[xt] "r" (pgTableTTBR0));
    asm volatile("msr TTBR1_EL1, %[xt]"::[xt] "r" (pgTableTTBR1));

    /* Invalidate the tlb and instruction cache.*/
    asm volatile("tlbi VMALLE1":::"memory");
    asm volatile("ic IALLU":::"memory");

    /* turn MMU on */
    register unsigned long sctlrEL1 = (0x1UL << 12)    ; // Instruction cache enable
    sctlrEL1 |= (0x1UL << 4)    // EL0 stack ops should be 16byte aligned
             | (0x1UL << 3)     // EL1 stack ops should be 16byte aligned
             | (0x1UL << 2)     // Enable caches
             | (0x0UL << 1)     // Alignment fault checking Disabled
             | 0x1UL            // EL0 and EL1 MMU on
             ;
    asm volatile("msr SCTLR_EL1, %[xt]"::[xt] "r" (sctlrEL1):"memory");
    asm volatile("isb":::);
}

__bootstrap void bootstrapMain(void *deviceTreePhysAddr, void *uartAddr, ptrdiff_t loadLinkOffset)
{
    // If this is not the boot CPU, enable the MMU using the page table
    // prepared by the boot CPU.
    register int mpidr;
    int cpu_num;

    asm volatile("mrs %[result], mpidr_el1" : [result] "=r" (mpidr) : :);
    cpu_num = (mpidr & MPIDR_CPUID_MASK);
    if (cpu_num != 0) {
        enableMMU();
        return;
    }

    makeBootstrapPageTables(deviceTreePhysAddr, uartAddr);

    phys_to_virt_offset = ULONG_MAX - loadLinkOffset + 1;
    virt_to_phys_offset = loadLinkOffset;

    enableMMU();
}
