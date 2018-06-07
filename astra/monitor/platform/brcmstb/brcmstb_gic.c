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

#include "monitor.h"
#include "memory.h"
#include "platform.h"
#include "gic.h"
#include "gicv2.h"
#include "brcmstb_priv.h"

static uintptr_t gic_dist_base;
static uintptr_t gic_cpuif_base;

void plat_gic_init(void)
{
    uintptr_t gic_base;

    /* Get GIC base from cbar_el1 register
     * In A53, cbar_el1 is a 64-bit register, no unwrapping necessary.
     */
    gic_base = read_cbar();
    gic_dist_base  = gic_base + BRCMSTB_GIC_DIST_BASE;
    gic_cpuif_base = gic_base + BRCMSTB_GIC_CPUIF_BASE;

#ifdef DEBUG
    DBG_PRINT("\nGIC base @ 0x%lx\n", gic_base);
#endif

    /* Add GIC base to mmap regions */
    mmap_add_region(
        gic_base,
        gic_base,
        BRCMSTB_GIC_MMAP_SIZE,
        MT_SOC_REG | MT_SEC);
}

int gic_dist_init()
{
    /* Call into GICv2 driver */
    return gicv2_dist_init(gic_dist_base);
}

int gic_cpuif_init()
{
    /* Call into GICv2 driver */
    return gicv2_cpuif_init(gic_cpuif_base);
}

int gic_sec_intr_enable(uint32_t intr_id)
{
    return gicv2_sec_intr_enable(
        gic_dist_base,
        intr_id);
}

int gic_sec_intr_disable(uint32_t intr_id)
{
    return gicv2_sec_intr_disable(
        gic_dist_base,
        intr_id);
}

int gic_sgi_intr_generate(
    uint32_t intr_id,
    uint32_t cpu_mask)
{
    return gicv2_sgi_intr_generate(
        gic_dist_base,
        intr_id,
        cpu_mask);
}

uint32_t gic_intr_max()
{
    return gicv2_intr_max(gic_dist_base);
}

uint32_t gic_intr_iid()
{
    return gicv2_intr_iid(gic_cpuif_base);
}

void gic_intr_ack(uint32_t iid)
{
    gicv2_intr_ack(gic_cpuif_base, iid);
}

void gic_intr_end(uint32_t iid)
{
    gicv2_intr_end(gic_cpuif_base, iid);
}
