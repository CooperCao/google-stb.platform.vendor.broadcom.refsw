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

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "memory.h"
#include "platform.h"
#include "version.h"

extern uintptr_t _bootstrap_start;
extern uintptr_t _bootstrap_end;

extern uintptr_t _stacks_start;
extern uintptr_t _stacks_end;

#define BOOTSTRAP_START ((uintptr_t)&_bootstrap_start)
#define BOOTSTRAP_SIZE  ((uintptr_t)&_bootstrap_end - \
                         (uintptr_t)&_bootstrap_start)

#define BOOTSTRAP_STACK_START ((uintptr_t)&_stacks_start)
#define BOOTSTRAP_STACK_SIZE  (CPU_STACK_SIZE)

ptrdiff_t mon_load_link_offset;
uint32_t system_counter_freq;

__bootstrap void mon_bootstrap(
    uintptr_t mon_params_addr,
    uintptr_t plat_params_addr,
    uintptr_t uart_base,
    ptrdiff_t load_link_offset)
{
    /* Save counter frequency */
    uint32_t counter_freq;
    __asm__ volatile("mrs %[f], CNTFRQ_EL0" : [f] "=r" (counter_freq) ::);
    system_counter_freq = counter_freq;

    /* Save load-link offset */
    mon_load_link_offset = load_link_offset;

    /* Init platform UART for early prints */
    plat_uart_init(uart_base);

    INFO_MSG("MON64 version %d.%d.%d",
             VERSION_MAJOR,
             VERSION_MINOR,
             VERSION_BUILD);

    /* Init platform GIC for mmap regions */
    plat_gic_init();

    /* Copy out the params before enabling MMU */
    plat_early_init(plat_params_addr);
    mon_early_init(mon_params_addr);

    /*
     * Add bootstrap mmap regions:
     * - Aarch64 uses PC-relative addresses, so &label is load address
     * - Bootstrap and cpu0 stack is mapped twice: on-spot and with-offset,
     *   On-spot mapping allows call stack unwinding once mmu is enabled;
     * - It is assumed that the load addresses of these regions are NOT
     *   overlapping the link addresses of the monitor image.
     */
    mmap_add_region(
        BOOTSTRAP_START,
        BOOTSTRAP_START,
        BOOTSTRAP_SIZE,
        MT_CODE | MT_SECURE);

    mmap_add_region(
        BOOTSTRAP_STACK_START,
        BOOTSTRAP_STACK_START,
        BOOTSTRAP_STACK_SIZE,
        MT_RW_DATA | MT_SECURE);

    /* Add system mmap regions */
    mmap_add_sys_regions(load_link_offset);

#ifdef VERBOSE
    mmap_dump();
#endif

    /* Populate xlat tables */
    mmap_populate_xlat_tables();

    /* Enable mmu */
    enable_mmu();
}

__bootstrap void mon_secondary_bootstrap(void)
{
    /* Set counter frequency */
    uint32_t counter_freq = system_counter_freq;
    __asm__ volatile("msr CNTFRQ_EL0, %[f]" :: [f] "r" (counter_freq) :);

    /* Enable mmu */
    enable_mmu();
}
