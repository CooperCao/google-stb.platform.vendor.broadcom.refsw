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

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "memory.h"
#include "platform.h"
#include "version.h"
#include "boot_priv.h"

extern uintptr_t _bootstrap_start;
extern uintptr_t _bootstrap_end;

extern uintptr_t _stacks_start;
extern uintptr_t _stacks_end;

#define BOOTSTRAP_START ((uintptr_t)&_bootstrap_start)
#define BOOTSTRAP_SIZE  ((uintptr_t)&_bootstrap_end - \
                         (uintptr_t)&_bootstrap_start)

#define BOOTSTRAP_STACK_START ((uintptr_t)&_stacks_start)
#define BOOTSTRAP_STACK_SIZE  (CPU_STACK_SIZE)

boot_args_t boot_args;

__bootstrap void mon_bootstrap(
    uintptr_t mon_params_addr,
    uintptr_t plat_params_addr,
    uintptr_t uart_base,
    ptrdiff_t load_link_offset)
{
    /* Save load-link offset */
    boot_args.load_link_offset = load_link_offset;

    /* Save uart base */
    boot_args.uart_base = uart_base;

    /* Is wam boot? */
    boot_args.warm_boot = !mon_params_addr || !plat_params_addr;

    /* Save counter frequency */
    uint32_t cntfrq;
    __asm__ volatile("mrs %[f], CNTFRQ_EL0" : [f] "=r" (cntfrq) ::);
    boot_args.counter_freq = cntfrq;

    /* Init platform UART for early prints */
    plat_uart_init(uart_base);

    SYS_MSG("MON64 version %d.%d.%d",
            VERSION_MAJOR,
            VERSION_MINOR,
            VERSION_BUILD);

    /*
     * MMU is enabled in 3 passes:
     * - 1st pass: enable MMU to access boot params in non-secure mode;
     * - 2nd pass: enable MMU to check and decompress images;
     * - 3rd pass: enable MMU for normal operations.
     */

    /*
     * 1st pass MMU enable:
     * - boot params regions are mapped as non-secure;
     * - all system regions are mapped on-spot.
     */

    /* Add UART mmap region */
    mmap_add_region(
        ALIGN_TO_PAGE(uart_base),
        ALIGN_TO_PAGE(uart_base),
        PAGE_SIZE,
        MT_SOC_REG | MT_NSEC);

    /*
     * Add boot params mmap regions:
     * - Boot params are prepared by bootloader running in non-secure mode.
     *   Hence the boot params need to reside in non-secure memory;
     * - At the monitor init time, there is a BIU ARCH that prevents any
     *   secure access to non-secure memory;
     * - In order to access the boot params, MMU is enabled first with the
     *   boot params mapped as non-secure.
     */
    mmap_add_region(
        ALIGN_TO_PAGE(plat_params_addr),
        ALIGN_TO_PAGE(plat_params_addr),
        PAGE_SIZE,
        MT_RO_DATA | MT_NSEC);

    mmap_add_region(
        ALIGN_TO_PAGE(mon_params_addr),
        ALIGN_TO_PAGE(mon_params_addr),
        PAGE_SIZE,
        MT_RO_DATA | MT_NSEC);

    /* Add system mmap regions */
    mmap_add_sys_regions(0);

    /* Populate xlat tables */
    mmap_populate_xlat_tables();

    /* Enable MMU */
    enable_mmu();

    /* Copy out platform params for mmap regions */
    plat_early_init(plat_params_addr);

    /* Copy out monitior params */
    mon_early_init(mon_params_addr);

    /* Disable MMU */
    disable_mmu();

    /* Reset mmap regions */
    mmap_reset_regions();

    /*
     * 2nd pass MMU enable:
     * - enntire secure world memory is mapped;
     * - all system regions are mapped on-spot.
     */

    /* Add UART mmap region */
    mmap_add_region(
        ALIGN_TO_PAGE(uart_base),
        ALIGN_TO_PAGE(uart_base),
        PAGE_SIZE,
        MT_SOC_REG | MT_NSEC);

    /* Add images mmap regions */
    mon_images_mmap_init();

    /* Add system mmap regions */
    mmap_add_sys_regions(0);

    /* Populate xlat tables */
    mmap_populate_xlat_tables();

    /* Enable MMU */
    enable_mmu();

    /* Init images */
    mon_images_init();

    /* Disable MMU */
    disable_mmu();

    /* Reset mmap regions */
    mmap_reset_regions();

    /*
     * 3rd pass MMU enable:
     * - additional regions from boot params are mapped;
     * - S3 params and GIC regions are mapped;
     * - all system regions are mapped with-offset;
     * - bootstrap regions are double mapped on-spot to allow code return.
     */

    /* Add UART mmap region */
    mmap_add_region(
        ALIGN_TO_PAGE(uart_base),
        ALIGN_TO_PAGE(uart_base),
        PAGE_SIZE,
        MT_SOC_REG | MT_NSEC);

    /* Add platform mmap regions */
    plat_mmap_init();

    /* Add monitor mmap regions */
    mon_mmap_init();

    /* Retrieve s3 params */
    if (warm_boot())
        plat_early_s3_init();

    /* Init platform GIC for mmap regions */
    plat_gic_init();

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
        MT_CODE | MT_SEC);

    mmap_add_region(
        BOOTSTRAP_STACK_START,
        BOOTSTRAP_STACK_START,
        BOOTSTRAP_STACK_SIZE,
        MT_RW_DATA | MT_SEC);

    /* Add system mmap regions */
    mmap_add_sys_regions(load_link_offset);

    /* Populate xlat tables */
    mmap_populate_xlat_tables();

    /* Enable MMU */
    enable_mmu();
}

__bootstrap void mon_secondary_bootstrap(void)
{
    /* Set counter frequency */
    uint32_t cntfrq = counter_freq();
    __asm__ volatile("msr CNTFRQ_EL0, %[f]" :: [f] "r" (cntfrq) :);

    /* Enable MMU */
    enable_mmu();
}
