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
unsigned long boot_mode;

__init_data unsigned long phys_to_virt_offset;
__init_data unsigned long virt_to_phys_offset;

__init_data uint64_t *pgTableTTBR0, pgTableTTBR1;

// 8MB max kernel image size (code + data).
#define KERNEL_PAGE_TABLE_BLOCK_SIZE  (PAGE_SIZE_4K_BYTES/8)

__init_data ALIGN_PT uint64_t bootstrap_pt_blocks[NUM_BOOTSTRAP_BLOCKS * KERNEL_PAGE_TABLE_BLOCK_SIZE];
__init_data int bootstrap_next_block;

extern unsigned int _bootstrap_img_start;
extern unsigned int _bootstrap_img_end;

extern unsigned int _kernel_img_start;
extern unsigned int _kernel_img_end;

__bootstrap static inline uint64_t *bootstrap_block_alloc(ptrdiff_t load_link_offset) {

    uint8_t *pt_blocks_raw = (uint8_t *)(bootstrap_pt_blocks) + load_link_offset;

    uint8_t *next_block_raw = (uint8_t *)&bootstrap_next_block + load_link_offset;
    int *next_block = (int *)next_block_raw;

    if (*next_block == NUM_BOOTSTRAP_BLOCKS) {
        ARCH_HALT();
        return NULL;
    }

    int idx = *next_block;
    uint8_t *rv = (pt_blocks_raw + (idx * KERNEL_PAGE_TABLE_BLOCK_SIZE * 8));
    (*next_block)++;

    return (uint64_t *)rv;
}

__bootstrap static inline void bootstrap_map_mem_range(uint8_t *range_begin, uint8_t *range_end,
            int memory_attr, uint8_t *translated_range_begin, ptrdiff_t load_link_offset) {

    uint64_t **ppg_table;

    if ((uintptr_t)range_begin & PAGE_TABLE_SELECTION_MASK)
        ppg_table = (uint64_t **)((uint32_t)&pgTableTTBR1 + load_link_offset);
    else
        ppg_table = (uint64_t **)((uint32_t)&pgTableTTBR0 + load_link_offset);

    if (*ppg_table == NULL) {
        *ppg_table = bootstrap_block_alloc(load_link_offset);

        uint64_t *pt_entry = *ppg_table;
        for (int i=0; i<L1_PAGE_NUM_ENTRIES; i++)
            pt_entry[i] = 0;
    }

    uint8_t *curr_page = range_begin;
    uint8_t *translated_page = translated_range_begin;

    uint64_t *l1_page_table = *ppg_table;

    while (curr_page <= range_end) {

        int l1_idx = L1_PAGE_TABLE_SLOT(curr_page);
        l1_idx = l1_idx & 0x01;
        if (l1_page_table[l1_idx] == 0) {

            uint64_t *pt_block = bootstrap_block_alloc(load_link_offset);

            for (int i=0; i<L2_PAGE_NUM_ENTRIES; i++)
                pt_block[i] = 0;

            uint64_t l1_entry = (uint64_t)((uint32_t)pt_block & L2_BLOCK_ADDR_MASK);
            l1_entry |= 0x3;
            l1_page_table[l1_idx] = l1_entry;
        }

        uint64_t l1_entry = l1_page_table[l1_idx];
        uint32_t block_addr = (uint32_t)(l1_entry & L2_BLOCK_ADDR_MASK);
        uint64_t *l2_page_table = (uint64_t *)block_addr;

        int l2_idx = (uint64_t)(((uint32_t)curr_page >> L2_BLOCK_SHIFT) & L2_BLOCK_MASK);
        if (l2_page_table[l2_idx] == 0) {
            uint64_t *pt_block = bootstrap_block_alloc(load_link_offset);

            for (int i=0; i<L3_PAGE_NUM_ENTRIES; i++)
                pt_block[i] = 0;

            uint64_t l2_entry = (uint64_t)((uint32_t)pt_block & L3_BLOCK_ADDR_MASK);
            l2_entry |= 0x3;

            l2_page_table[l2_idx] = l2_entry;
        }

        uint64_t l2_entry = l2_page_table[l2_idx];
        block_addr = (uint32_t)(l2_entry & L3_BLOCK_ADDR_MASK);
        uint64_t *l3_page_table = (uint64_t *)block_addr;
        int l3_idx =  (uint64_t)(((uint32_t)curr_page >> L3_BLOCK_SHIFT) & L3_BLOCK_MASK);

        uint64_t l3_entry = 0x3;
        l3_entry |=  (uint64_t)((uint32_t)translated_page & L3_PHYS_ADDR_MASK); //0xFFFFF000
        SET_MEMORY_ACCESS_FLAG(l3_entry, ACCESS_FLAG_NO_FAULT_GEN);
        SET_MEMORY_ATTR(l3_entry, memory_attr);
        SET_MEMORY_SH_ATTR(l3_entry, INNER_SHAREABLE);
        SET_MEMORY_ACCESS_PERMS(l3_entry, MEMORY_ACCESS_RW_KERNEL);

        l3_page_table[l3_idx] = l3_entry;

        curr_page += PAGE_SIZE_4K_BYTES;
        translated_page += PAGE_SIZE_4K_BYTES;
    }
}

__bootstrap static inline void prepare_bootstrap_page_table(ptrdiff_t load_link_offset, void *uart_addr, void *dtree_phys_addr) {

    uint8_t *next_block_raw = (uint8_t *)&bootstrap_next_block + load_link_offset;
    int *next_block = (int *)next_block_raw;
    *next_block = 0;

    uint8_t *bootstrap_start = (uint8_t *)&_bootstrap_img_start + load_link_offset;
    uint8_t *bootstrap_end = (uint8_t *)&_bootstrap_img_end - 1 + load_link_offset;

    bootstrap_map_mem_range(bootstrap_start, bootstrap_end,
        MAIR_MEMORY, bootstrap_start, load_link_offset);

    uint8_t *kernel_img_start = (uint8_t *)&_kernel_img_start;
    uint8_t *kernel_img_end = (uint8_t *)&_kernel_img_end - 1;
    uint8_t *translated_img_start = (uint8_t *)&_kernel_img_start + load_link_offset;

    bootstrap_map_mem_range(kernel_img_start, kernel_img_end,
        MAIR_MEMORY, translated_img_start, load_link_offset);

    if (uart_addr != 0) {
        uint8_t *uart_start = PAGE_START_4K(uart_addr);
        uint8_t *uart_end = uart_start;
        uint8_t *translated_uart_start = uart_start;

        bootstrap_map_mem_range(uart_start, uart_end,
            MAIR_DEVICE, translated_uart_start, load_link_offset);
    }

    uint8_t *dt_start = PAGE_START_4K(dtree_phys_addr);
    uint8_t *dt_end = (uint8_t *)dtree_phys_addr + MAX_DT_SIZE_BYTES - 1;
    uint8_t *translated_dt_start = dt_start;

    bootstrap_map_mem_range(dt_start, dt_end,
        MAIR_MEMORY, translated_dt_start, load_link_offset);
}

__bootstrap static inline void enable_mmu(ptrdiff_t load_link_offset) {


    register uint64_t *pg_table0 = *(uint64_t **)((uint32_t)&pgTableTTBR0 + load_link_offset);
    register uint64_t *pg_table1 = *(uint64_t **)((uint32_t)&pgTableTTBR1 + load_link_offset);

    /* Setup the memory indirection attribute registers */
    register unsigned int mair0 = MAIR0;
    register unsigned int mair1 = MAIR1;
    asm volatile("mcr p15, 0, %[val], c10, c2, 0" : :[val] "r" (mair0));
    asm volatile("mcr p15, 0, %[val], c10, c2, 1" : :[val] "r" (mair1));

    /* Setup TTBCR: LPAE format page tables, Use both TTBR0 (0 -> 2GB) and
        TTBR1 (2GB -> 4GB ), Normal cacheable memory */
    register unsigned int ttbcr = 0xa5012501;  /* TTBCR_LPAE_ENABLE | TTBCR_OUTER_SHAREABLE0 | TTBCR_OUTER_CACHEABLE0_NORMAL |
                                                  TTBCR_INNER_CACHEABLE0_NORMAL; */
    asm volatile("mcr p15, 0, %[val], c2, c0, 2" : :[val] "r" (ttbcr));

    /* Point TTBR0 to the page table */
    register unsigned int ttbr0_low  = (unsigned int)pg_table0;
    register unsigned int ttbr0_high = 0;
    asm volatile("mcrr p15, 0, %[low], %[high], c2" : : [low] "r" (ttbr0_low), [high] "r" (ttbr0_high));

    /* Point TTBR1 to the page table */
    register unsigned int ttbr1_low  = (unsigned int)pg_table1;
    register unsigned int ttbr1_high = 0;
    asm volatile("mcrr p15, 1, %[low], %[high], c2" : : [low] "r" (ttbr1_low), [high] "r" (ttbr1_high));

    /* Invalidate TLB, ICache and branch predictor */
    register unsigned int zero = 0;
    asm volatile("mcr p15, 0, %[val], c8, c7, 0\r\n"
                 "mcr p15, 0, %[val], c7, c5, 0\r\n"
                 "mcr p15, 0, %[val], c7, c5, 6\r\n"
                 "dsb\r\n"
                 "isb\r\n"
                 : : [val] "r" (zero));

    /* Turn MMU on (along with cache and branch prediction) */
    register unsigned int sctlr = CORTEX_A15_SCTLR_DEFAULT | 0x1805;
    asm volatile("dsb\r\n"
                 "mcr p15, 0, %[val], c1, c0, 0\r\n"
                 "isb\r\n"
                 : : [val] "r" (sctlr));
}

extern eUartType early_uart_type;

__bootstrap void bootstrap_main(ptrdiff_t load_link_offset, uintptr_t machine_id, void *dtree_phys_addr, void *uart_addr) {

    // If this is not the boot CPU, enable the MMU using the page table
    // prepared by the boot CPU.
    register int mpidr;
    int cpu_num;
    asm volatile("mrc p15, 0, %[result], c0, c0, 5" : [result] "=r" (mpidr) : :);
    cpu_num = (mpidr & MPIDR_CPUID_MASK);
    if (cpu_num != 0) {
        enable_mmu(load_link_offset);
        return;
    }

    prepare_bootstrap_page_table(load_link_offset, uart_addr, dtree_phys_addr);

    enable_mmu(load_link_offset);

    boot_mode = (machine_id) ? ARMV7_BOOT_MODE : ARMV8_BOOT_MODE;
    phys_to_virt_offset = ULONG_MAX - load_link_offset + 1;
    virt_to_phys_offset = load_link_offset;

    // Save system uart base
    early_uart_base = (uint32_t)uart_addr;

    /* TODO: Detect & Change UART Type at Runtime */
    early_uart_type = UART_TYPE_NS16550a;
}
