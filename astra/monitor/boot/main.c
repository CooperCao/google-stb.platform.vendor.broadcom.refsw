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

#include <arch.h>
#include <arch_helpers.h>
#include <string.h>

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "memory.h"
#include "platform.h"
#include "interrupt.h"
#include "service.h"
#include "mon_params.h"
#include "mon_params_v2.h"
#include "bl31_params.h"
#include "gzip.h"
#include "astra.h"
#include "psci.h"
#include "dvfs.h"
#include "cpu_data.h"
#include "context_mgt.h"
#include "boot_priv.h"

mon_params_t mon_params __boot_params;
cpu_data_t cpu_data[MAX_NUM_CPUS];

#define IMG_IS_NSEC_EL2(img_info) \
    ((img_info.flags & IMG_NSEC_EL2) == IMG_NSEC_EL2)

#define IMG_IS_AARCH64(img_info) \
    ((img_info.flags & IMG_AARCH32_MASK) == IMG_AARCH64)

#define GZIP_MAGIC_OFFSET               0x0
#define GZIP_MAGIC_NUMBER               0x8b1f

#define ARM_IMAGE_MAGIC_OFFSET          0x38
#define ARM_IMAGE_MAGIC_AARCH64         0x644d5241
#define ARM_IMAGE_MAGIC_AARCH32         0x324d5241

#define ZIMAGE_MAGIC_OFFSET             0x24
#define ZIMAGE_MAGIC_NUMBER             0x016f2818

static inline bool is_img_gzipped(void *pimg)
{
    uint16_t magic = *(uint16_t *)(pimg + GZIP_MAGIC_OFFSET);
    return (magic == GZIP_MAGIC_NUMBER);
}

static inline bool is_img_arm_aarch32(void *pimg)
{
    uint32_t magic = *(uint32_t *)(pimg + ARM_IMAGE_MAGIC_OFFSET);
    return  (magic == ARM_IMAGE_MAGIC_AARCH32);
}

static inline bool is_img_arm_aarch64(void *pimg)
{
    uint32_t magic = *(uint32_t *)(pimg + ARM_IMAGE_MAGIC_OFFSET);
    return  (magic == ARM_IMAGE_MAGIC_AARCH64);
}

static inline bool is_img_zimage(void *pimg)
{
    uint32_t magic = *(uint32_t *)(pimg + ZIMAGE_MAGIC_OFFSET);
    return  (magic == ZIMAGE_MAGIC_NUMBER);
}

#ifdef DEBUG
static void mon_params_dump()
{
    DBG_PRINT("\nMonitor parameters:\n");
    DBG_PRINT("  Num of clusters: %d\n", mon_params.num_clusters);
    DBG_PRINT("  Num of cpus: %d\n", mon_params.num_cpus);
    DBG_PRINT("  TZ cpus mask: 0x%x\n", mon_params.tz_cpus_mask);
    DBG_PRINT("  NW cpus mask: 0x%x\n", mon_params.nw_cpus_mask);
    DBG_PRINT("\n");
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
    DBG_PRINT("\n");
    DBG_PRINT("  TZ mailbox info:\n");
    DBG_PRINT("    addr:  %p\n", (void *)mon_params.tz_mbox_info.addr);
    DBG_PRINT("    size:  0x%x\n", (unsigned)mon_params.tz_mbox_info.size);
    DBG_PRINT("    flags: 0x%x\n", (unsigned)mon_params.tz_mbox_info.flags);
    DBG_PRINT("    sgi:   %d\n", mon_params.tz_mbox_info.sgi);
    DBG_PRINT("\n");
    DBG_PRINT("  NW mailbox info:\n");
    DBG_PRINT("    addr:  %p\n", (void *)mon_params.nw_mbox_info.addr);
    DBG_PRINT("    size:  0x%x\n", (unsigned)mon_params.nw_mbox_info.size);
    DBG_PRINT("    flags: 0x%x\n", (unsigned)mon_params.nw_mbox_info.flags);
    DBG_PRINT("    sgi:   %d\n", mon_params.nw_mbox_info.sgi);
}
#endif

typedef mon_params_t mon_params_v3_t;

static void mon_params_conv2(
    mon_params_v3_t *pv3_params,
    mon_params_v2_t *pv2_params)
{
    param_hdr_t *pv2_hdr;

    /* Check header */
    pv2_hdr = &pv2_params->hdr;

    DBG_ASSERT(pv2_hdr->type == MONITOR_PARAMS);
    DBG_ASSERT(pv2_hdr->version == 2);
#ifndef DEBUG
    UNUSED(pv2_hdr);
#endif

    /* Clear mon params v3 to be safe */
    bootstrap_memset(pv3_params, 0, sizeof(mon_params_v3_t));

    /* Copy mon params v2 entirely */
    bootstrap_memcpy(
        pv3_params,
        pv2_params,
        sizeof(mon_params_v2_t));

    /* Update header */
    pv3_params->hdr.version = 3;

    /* Fill in additional fields */
    pv3_params->num_clusters = MAX_NUM_CLUSTERS;
    pv3_params->num_cpus = MAX_NUM_CPUS;

    pv3_params->tz_cpus_mask = (1 << MAX_NUM_CPUS) - 1;
    pv3_params->nw_cpus_mask = (1 << MAX_NUM_CPUS) - 1;

    pv3_params->tz_mbox_info.addr = 0;
    pv3_params->nw_mbox_info.addr = 0;
}

static void mon_params_conv1(
    mon_params_v2_t *pv2_params,
    bl31_params_t *pbl31_params)
{
    param_header_t *pheader;
    image_info_t *pimage_info;
    entry_point_info_t *pep_info;

    /* Check header */
    pheader = &pbl31_params->h;

    DBG_ASSERT(pheader->type == PARAM_BL31);
    DBG_ASSERT(pheader->version == VERSION_1);
#ifndef DEBUG
    UNUSED(pheader);
#endif

    /* Clear mon params v2 to be safe */
    bootstrap_memset(pv2_params, 0, sizeof(mon_params_v2_t));

    /* Convert header */
    pv2_params->hdr.type = MONITOR_PARAMS;
    pv2_params->hdr.version = 2;

    /* Convert monitor image info, if present */
    pimage_info = pbl31_params->bl31_image_info;

    if (pimage_info && pimage_info->h.type) {
        DBG_ASSERT(pimage_info->h.type == PARAM_IMAGE_BINARY);
        DBG_ASSERT(pimage_info->h.version == VERSION_1);

        pv2_params->mon_img_info.base = pimage_info->image_base;
        pv2_params->mon_img_info.size = pimage_info->image_size;
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

        pv2_params->tz_img_info.base = pimage_info->image_base;
        pv2_params->tz_img_info.size = pimage_info->image_size;
        pv2_params->tz_img_info.flags =
            ((pep_info->spsr & (MODE_RW_MASK << MODE_RW_SHIFT)) == MODE_RW_64) ?
            IMG_AARCH64 : IMG_AARCH32;

        pv2_params->tz_entry_point = pep_info->pc;
        pv2_params->tz_dev_tree = pep_info->args.arg0;
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

        pv2_params->nw_img_info.base = pimage_info->image_base;
        pv2_params->nw_img_info.size = pimage_info->image_size;
        pv2_params->nw_img_info.flags =
            ((pep_info->spsr & (MODE_RW_MASK << MODE_RW_SHIFT)) == MODE_RW_64) ?
            IMG_AARCH64 : IMG_AARCH32;

        pv2_params->nw_entry_point = pep_info->pc;

        /* In case of 32-bit, device tree is in arg2 */
        if (pv2_params->nw_img_info.flags == IMG_AARCH64) {
            DBG_ASSERT(pep_info->args.arg0);
            pv2_params->nw_dev_tree = pep_info->args.arg0;
        }
        else {
            DBG_ASSERT(pep_info->args.arg2);
            pv2_params->nw_dev_tree = pep_info->args.arg2;
        }
    }
}

void mon_early_init(uintptr_t mon_params_addr)
{
    if (mon_params_addr) {
        /* Cold boot, init mon params */
        param_hdr_t *phdr;

        phdr = (param_hdr_t *)mon_params_addr;

        ASSERT(phdr->type == MONITOR_PARAMS);
        ASSERT(phdr->version <= MONITOR_PARAMS_VERSION);

        switch (phdr->version) {
            mon_params_v2_t mon_params_v2;

        case MONITOR_PARAMS_VERSION:
            /* Copy mon params directly */
            bootstrap_memcpy(
                &mon_params,
                (mon_params_t *)mon_params_addr,
                sizeof(mon_params_t));
            break;

        case 2:
            /* Convert mon params from v2 */
            mon_params_conv2(
                &mon_params,
                (mon_params_v2_t *)mon_params_addr);
            break;

        case 1:
            /* Convert mon params from v1, i.e. BL31 params */
            mon_params_conv1(
                &mon_params_v2,
                (bl31_params_t *)mon_params_addr);

            mon_params_conv2(
                &mon_params,
                &mon_params_v2);
            break;

        default:
            ERR_MSG("Unknown monitor parameters version");
            SYS_HALT();
        }
    }
    else {
        /* Warm boot, reuse mon params */
        ASSERT(mon_params.hdr.type == MONITOR_PARAMS);
        ASSERT(mon_params.hdr.version == MONITOR_PARAMS_VERSION);
    }
}

void mon_images_mmap_init(void)
{
    /* Add image addresses to mmap regions */
    mmap_add_region(
        ALIGN_TO_PAGE(mon_params.tz_entry_point),
        ALIGN_TO_PAGE(mon_params.tz_entry_point),
        MAX_TZ_IMG_SIZE + MAX_TZ_CMP_SIZE,
        MT_RW_DATA | MT_SEC);

    mmap_add_region(
        ALIGN_TO_PAGE(mon_params.nw_entry_point),
        ALIGN_TO_PAGE(mon_params.nw_entry_point),
        PAGE_SIZE,
        MT_RW_DATA | MT_NSEC);
}

void mon_images_init(void)
{
    /* Update image flags */
    if (mon_params.tz_entry_point) {
        void *pimg = (void *)mon_params.tz_entry_point;

        /* Check if image is gzipped */
        if (is_img_gzipped(pimg)) {
            size_t cmpSize, imgSize;

            DBG_MSG("Decompressing secure image...");

            /* Use max compressed image if image size NOT given */
            cmpSize = (mon_params.tz_img_info.size) ?
                mon_params.tz_img_info.size :
                MAX_TZ_CMP_SIZE;

            /* Copy image to higher address */
            memcpy(pimg + MAX_TZ_IMG_SIZE, pimg, cmpSize);

            /* GZIP decompress image */
            if (gzip_decompress(
                    pimg, &imgSize,
                    pimg + MAX_TZ_IMG_SIZE, &cmpSize)) {
                ERR_MSG("Failed to decompress secure image");
                SYS_HALT();
            }

            DBG_MSG("  compressed size: %d", (int)cmpSize);
            DBG_MSG("  decompressed size: %d", (int)imgSize);
        }

        /* Overwrite image flags with ARM image header info */
        if (is_img_arm_aarch64(pimg))
            mon_params.tz_img_info.flags &= ~IMG_AARCH32_MASK;
        else if (is_img_arm_aarch32(pimg))
            mon_params.tz_img_info.flags |= IMG_AARCH32_MASK;
    }

    if (mon_params.nw_entry_point) {
        void *pimg = (void *)mon_params.nw_entry_point;

        if (is_img_arm_aarch64(pimg))
            /* Linux uses ARM image header only for 64-bit */
            mon_params.nw_img_info.flags &= ~IMG_AARCH32_MASK;
        else if (is_img_zimage(pimg))
            /* Linux uses zImage header for 32-bit */
            mon_params.nw_img_info.flags |= IMG_AARCH32_MASK;
    }
}

void mon_mmap_init(void)
{
    mbox_info_t *pmbox;

    /* Add mailbox addresses to mmap regions */
    pmbox = &mon_params.tz_mbox_info;
    if (pmbox->addr) {
        ASSERT(IS_PAGE_ALIGNED(pmbox->addr));
        mmap_add_region(
            pmbox->addr,
            pmbox->addr,
            pmbox->size,
            MT_MAILBOX | MT_SEC);
    }

    pmbox = &mon_params.nw_mbox_info;
    if (pmbox->addr) {
        ASSERT(IS_PAGE_ALIGNED(pmbox->addr));
        mmap_add_region(
            pmbox->addr,
            pmbox->addr,
            pmbox->size,
            MT_MAILBOX | MT_NSEC);
    }

#ifdef DEBUG
    mon_params_dump();
#endif
}

void mon_main(void)
{
    cpu_data_t *pcpu_data;
    cpu_context_t *pcpu_ctx;

    INFO_MSG("Entering monitor main function...");

    /* Init platform s3 in warm boot */
    if (warm_boot())
        plat_s3_init();

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
        mon_params.num_clusters,
        mon_params.num_cpus,
        NULL);

    /* Init DVFS */
    dvfs_init(mon_params.num_cpus);

    /* Set tpidr_el3 to point to CPU data */
    pcpu_data = get_cpu_data_by_index(get_cpu_index());
    write_tpidr_el3((uint64_t)pcpu_data);

    /* init CPU data */
    pcpu_data->cpu_flags =
        ((mon_params.tz_entry_point &&
         (mon_params.tz_cpus_mask & (1 << get_cpu_index()))) ? CPU_SEC_MASK : 0) |
        ((mon_params.nw_entry_point &&
         (mon_params.nw_cpus_mask & (1 << get_cpu_index()))) ? CPU_NSEC_MASK : 0);

    /* Notify PSCI that CPU is up */
    psci_cpu_up();

    /* Init secure world context */
    if (sec_enable(pcpu_data)) {
        uintptr_t entry_point;
        uintptr_t dev_tree;

        entry_point = mon_params.tz_entry_point;
        dev_tree = mon_params.tz_dev_tree;

        SYS_MSG("Secure entry point @ %p", (void *)entry_point);
        SYS_MSG("Secure device tree @ %p", (void *)dev_tree);

        DBG_MSG("Initializing secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[SECURE],
            entry_point,
            dev_tree,
            SECURE,
            false,
            IMG_IS_AARCH64(mon_params.tz_img_info));
    }

    /* Init normal world context */
    if (nsec_enable(pcpu_data)) {
        uintptr_t entry_point;
        uintptr_t dev_tree;

        if (warm_boot()) {
            entry_point = plat_s3_nsec_reentry_point();
            dev_tree = 0;

            SYS_MSG("Non-secure reentry point @ %p", (void *)entry_point);
        }
        else {
            entry_point = mon_params.nw_entry_point;
            dev_tree = mon_params.nw_dev_tree;

            SYS_MSG("Non-secure entry point @ %p", (void *)entry_point);
            SYS_MSG("Non-secure device tree @ %p", (void *)dev_tree);
        }

        DBG_MSG("Initializing non-secure context...");
        cm_init_context(
            &pcpu_data->cpu_ctx[NON_SECURE],
            entry_point,
            dev_tree,
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

    SYS_MSG("Starting in %s world...",
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
    pcpu_data->cpu_flags =
        ((mon_params.tz_entry_point &&
         (mon_params.tz_cpus_mask & (1 << get_cpu_index()))) ? CPU_SEC_MASK : 0) |
        ((mon_params.nw_entry_point &&
         (mon_params.nw_cpus_mask & (1 << get_cpu_index()))) ? CPU_NSEC_MASK : 0);

    /* Notify PSCI that CPU is up */
    psci_cpu_up();

    /* Init secure world context */
    if (sec_enable(pcpu_data)) {

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
    if (nsec_enable(pcpu_data)) {

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

    SYS_MSG("Starting secondary in %s world...",
            sec_enable(pcpu_data) ? "secure" : "non-secure");
}
