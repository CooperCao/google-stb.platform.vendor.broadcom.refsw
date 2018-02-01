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

#include <arch.h>
#include <arch_helpers.h>

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "platform.h"
#include "interrupt.h"
#include "service.h"
#include "mon_params.h"
#include "bl31_params.h"
#include "astra.h"
#include "psci.h"
#include "scmi.h"
#include "dvfs.h"
#include "cpu_data.h"
#include "context_mgt.h"

mon_params_t mon_params;
cpu_data_t cpu_data[MAX_NUM_CPUS];

#define IMG_IS_NSEC_EL2(img_info) \
    ((img_info.flags & IMG_NSEC_EL2) == IMG_NSEC_EL2)

#define IMG_IS_AARCH64(img_info) \
    ((img_info.flags & IMG_AARCH32_MASK) == IMG_AARCH64)

static void mon_params_conv1(uintptr_t mon_params_addr)
{
    param_header_t *pheader;
    bl31_params_t *pbl31_params;
    image_info_t *pimage_info;
    entry_point_info_t *pep_info;

    /* Convert header */
    pheader = (param_header_t *)mon_params_addr;

    DBG_ASSERT(pheader->type == PARAM_BL31);
    DBG_ASSERT(pheader->version == VERSION_1);
#ifndef DEBUG
    UNUSED(pheader);
#endif

    mon_params.hdr.type = MONITOR_PARAMS;
    mon_params.hdr.version = MONITOR_PARAMS_VERSION;

    pbl31_params = (bl31_params_t *)mon_params_addr;

    /* Convert monitor image info, if present */
    pimage_info = pbl31_params->bl31_image_info;

    if (pimage_info && pimage_info->h.type) {
        DBG_ASSERT(pimage_info->h.type == PARAM_IMAGE_BINARY);
        DBG_ASSERT(pimage_info->h.version == VERSION_1);

        mon_params.mon_img_info.base = pimage_info->image_base;
        mon_params.mon_img_info.size = pimage_info->image_size;
    }

    /* Convert secure world image info and parameters, if present */
    pimage_info = pbl31_params->bl32_image_info;
    pep_info    = pbl31_params->bl32_ep_info;

    if (pimage_info && pep_info && pimage_info->h.type) {
        DBG_ASSERT(pimage_info->h.type == PARAM_IMAGE_BINARY);
        DBG_ASSERT(pimage_info->h.version == VERSION_1);
        DBG_ASSERT(pimage_info->h.attr == SECURE);
        DBG_ASSERT(pep_info->h.type == PARAM_EP);
        DBG_ASSERT(pep_info->h.version == VERSION_1);
        DBG_ASSERT(pep_info->h.attr == SECURE);

        mon_params.tz_img_info.base = pimage_info->image_base;
        mon_params.tz_img_info.size = pimage_info->image_size;
        mon_params.tz_img_info.flags =
            ((pep_info->spsr & (MODE_RW_MASK << MODE_RW_SHIFT)) == MODE_RW_64) ?
            IMG_AARCH64 : IMG_AARCH32;

        mon_params.tz_entry_point = pep_info->pc;
        mon_params.tz_dev_tree = pep_info->args.arg0;
    }

    /* Convert normal world image info and parameters, if present */
    pimage_info = pbl31_params->bl33_image_info;
    pep_info    = pbl31_params->bl33_ep_info;

    if (pimage_info && pep_info && pimage_info->h.type) {
        DBG_ASSERT(pimage_info->h.type == PARAM_IMAGE_BINARY);
        DBG_ASSERT(pimage_info->h.version == VERSION_1);
        DBG_ASSERT(pimage_info->h.attr == NON_SECURE);
        DBG_ASSERT(pep_info->h.type == PARAM_EP);
        DBG_ASSERT(pep_info->h.version == VERSION_1);
        DBG_ASSERT(pep_info->h.attr == NON_SECURE);

        mon_params.nw_img_info.base = pimage_info->image_base;
        mon_params.nw_img_info.size = pimage_info->image_size;
        mon_params.nw_img_info.flags =
            ((pep_info->spsr & (MODE_RW_MASK << MODE_RW_SHIFT)) == MODE_RW_64) ?
            IMG_AARCH64 : IMG_AARCH32;

        mon_params.nw_entry_point = pep_info->pc;

        /* In case of 32-bit, device tree is in arg2 */
        if (mon_params.nw_img_info.flags == IMG_AARCH64) {
            DBG_ASSERT(pep_info->args.arg0);
            mon_params.nw_dev_tree = pep_info->args.arg0;
        }
        else {
            DBG_ASSERT(pep_info->args.arg2);
            mon_params.nw_dev_tree = pep_info->args.arg2;
        }
    }
}

void mon_early_init(uintptr_t mon_params_addr)
{
    param_hdr_t *phdr;

    ASSERT(mon_params_addr);

    phdr = (param_hdr_t *)mon_params_addr;

    ASSERT(phdr->type == MONITOR_PARAMS);
    ASSERT(phdr->version <= MONITOR_PARAMS_VERSION);

    switch (phdr->version) {
    case MONITOR_PARAMS_VERSION:
        mon_params = *(mon_params_t *)mon_params_addr;
        break;
    case 1:
        mon_params_conv1(mon_params_addr);
        break;
    default:
        ERR_MSG("Unknown monitor parameters version");
        SYS_HALT();
    }

#ifdef DEBUG
    DBG_PRINT("\nMonitor parameters:\n");
    DBG_PRINT("  Monitor image info:\n");
    DBG_PRINT("    base:  %p\n", (void *)mon_params.mon_img_info.base);
    DBG_PRINT("    size:  0x%x\n", (unsigned)mon_params.mon_img_info.size);
    DBG_PRINT("    flags: 0x%x\n", (unsigned)mon_params.mon_img_info.flags);
    DBG_PRINT("\n");
    DBG_PRINT("  TZ image info:\n");
    DBG_PRINT("    base:  %p\n", (void *)mon_params.tz_img_info.base);
    DBG_PRINT("    size:  0x%x\n", (unsigned)mon_params.tz_img_info.size);
    DBG_PRINT("    flags: 0x%x\n", (unsigned)mon_params.tz_img_info.flags);
    DBG_PRINT("  TZ entry point: %p\n", (void *)mon_params.tz_entry_point);
    DBG_PRINT("  TZ device tree: %p\n", (void *)mon_params.tz_dev_tree);
    DBG_PRINT("\n");
    DBG_PRINT("  NW image info:\n");
    DBG_PRINT("    base:  %p\n", (void *)mon_params.nw_img_info.base);
    DBG_PRINT("    size:  0x%x\n", (unsigned)mon_params.nw_img_info.size);
    DBG_PRINT("    flags: 0x%x\n", (unsigned)mon_params.nw_img_info.flags);
    DBG_PRINT("  NW entry point: %p\n", (void *)mon_params.nw_entry_point);
    DBG_PRINT("  NW device tree: %p\n", (void *)mon_params.nw_dev_tree);
#endif
}

void mon_main(void)
{
    cpu_data_t *pcpu_data;
    cpu_context_t *pcpu_ctx;

    INFO_MSG("Entering monitor main function...");

    /* Init platform */
    plat_init();

    /* Init interrupts */
    interrupt_init();

    /* Init services */
    service_init();

    /* Init astra */
    astra_init();

    /* Init PSCI */
    psci_init(
        MAX_NUM_CLUSTERS,
        MAX_NUM_CPUS,
        NULL);

    /* Init SCMI */
    scmi_init();

    /* Init DVFS */
    dvfs_init();

    /* Set tpidr_el3 to point to CPU data */
    pcpu_data = get_cpu_data_by_index(get_cpu_index());
    write_tpidr_el3((uint64_t)pcpu_data);

    /* init CPU data */
    pcpu_data->cpu_flags = CPU_SEC_MASK | CPU_NSEC_MASK;

    /* Notify PSCI that CPU is up */
    psci_cpu_up();

    /* Init secure world context */
    if (mon_params.tz_entry_point &&
        sec_enable(pcpu_data)) {

        DBG_MSG("Initializing secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[SECURE],
            mon_params.tz_entry_point,
            mon_params.tz_dev_tree,
            SECURE,
            false,
            IMG_IS_AARCH64(mon_params.tz_img_info));
    }

    /* Init normal world context */
    if (mon_params.nw_entry_point &&
        nsec_enable(pcpu_data)) {

        DBG_MSG("Initializing non-secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[NON_SECURE],
            mon_params.nw_entry_point,
            mon_params.nw_dev_tree,
            NON_SECURE,
            IMG_IS_NSEC_EL2(mon_params.nw_img_info),
            IMG_IS_AARCH64(mon_params.nw_img_info));

        /* Setup non-secure EL2 */
        cm_setup_nsec_el2(
            &pcpu_data->cpu_ctx[NON_SECURE],
            IMG_IS_NSEC_EL2(mon_params.nw_img_info));
    }

    /* Get first context */
    pcpu_ctx = &pcpu_data->cpu_ctx[sec_enable(pcpu_data) ?
                                   SECURE : NON_SECURE];

    /* Populate addition registers from first context */
    el1_sysregs_context_restore(get_sysregs_ctx(pcpu_ctx));
    fpregs_context_restore(get_fpregs_ctx(pcpu_ctx));

    /* Set first context as next context */
    cm_set_next_context(pcpu_ctx);

    INFO_MSG("Starting in %s world...",
             sec_enable(pcpu_data) ? "secure" : "non-secure");
}

void mon_secondary_main(void)
{
    cpu_data_t *pcpu_data;
    cpu_context_t *pcpu_ctx;

    INFO_MSG("Entering monitor secondary main function...");

    /* Init interrupts */
    interrupt_init();

    /* Set tpidr_el3 to point to CPU data */
    pcpu_data = get_cpu_data_by_index(get_cpu_index());
    write_tpidr_el3((uint64_t)pcpu_data);

    /* init CPU data */
    pcpu_data->cpu_flags = CPU_SEC_MASK | CPU_NSEC_MASK;

    /* Notify PSCI that CPU is up */
    psci_cpu_up();

    /* Init secure world context */
    if (mon_params.tz_entry_point &&
        sec_enable(pcpu_data)) {

        DBG_MSG("Initializing secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[SECURE],
            mon_params.tz_entry_point, /* Use the same secure entry point */
            (uintptr_t)NULL,           /* No need for secure device tree */
            SECURE,
            false,
            IMG_IS_AARCH64(mon_params.tz_img_info));
    }

    /* Init normal world context */
    if (mon_params.nw_entry_point &&
        nsec_enable(pcpu_data)) {

        DBG_MSG("Initializing non-secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[NON_SECURE],
            get_nsec_entry_point(), /* Use PSCI saved non-secure entry point */
            get_nsec_context_id(),  /* Pass PSCI saved non-secure context id */
            NON_SECURE,
            IMG_IS_NSEC_EL2(mon_params.nw_img_info),
            IMG_IS_AARCH64(mon_params.nw_img_info));

        /* Setup non-secure EL2 */
        cm_setup_nsec_el2(
            &pcpu_data->cpu_ctx[NON_SECURE],
            IMG_IS_NSEC_EL2(mon_params.nw_img_info));
    }

    /* Get first context */
    pcpu_ctx = &pcpu_data->cpu_ctx[sec_enable(pcpu_data) ?
                                   SECURE : NON_SECURE];

    /* Populate addition registers from first context */
    el1_sysregs_context_restore(get_sysregs_ctx(pcpu_ctx));
    fpregs_context_restore(get_fpregs_ctx(pcpu_ctx));

    /* Set first context as next context */
    cm_set_next_context(pcpu_ctx);

    INFO_MSG("Starting secondary in %s world...",
             sec_enable(pcpu_data) ? "secure" : "non-secure");
}
