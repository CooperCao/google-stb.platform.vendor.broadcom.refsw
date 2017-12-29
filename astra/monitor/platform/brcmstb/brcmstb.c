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
#include "memory.h"
#include "brcmstb_params.h"
#include "brcmstb_priv.h"

brcmstb_params_t brcmstb_params;

/* Default values for BCM7268 B0, BCM7271 B0 */
static brcmstb_params_t bcm7268b0_params =
{
    .rgroups =
    {
        /* BRCMSTB_RGROUP_BOOTSRAM_SECURE */
        {
            .base = 0xf0260000,
            .size = 0x00020000
        },
        /* BRCMSTB_RGROUP_SUN_TOP_CTRL */
        {
            .base = 0xf0404000,
            .size = 0x00000714
        },
        /* BRCMSTB_RGROUP_HIF_CPUBIUCTRL */
        {
            .base = 0xf0205000,
            .size = 0x00000400,
        },
        /* BRCMSTB_RGROUP_HIF_CONTINUATION */
        {
            .base = 0xf0452000,
            .size = 0x00000100
        }
    }
};

void plat_early_init(uintptr_t plat_params_addr)
{
    param_hdr_t *phdr;
    brcmstb_rgroup_t *prgroup;
    size_t rgroup_idx;

    /*
     * For Bolt backward compatibility, if platform parameters is NOT
     * present, try to identify the chip by reading the chip ID register
     * from the default location. This is best effort, and may NOT work
     * for all chips.
     *
     * Note: The old Bolt may NOT actually set the x1 register correctly,
     * leaving it with an address that is in GIC mmap range.
     */
    if (!plat_params_addr ||
        plat_params_addr > 0x10000000) {

        switch (MMIO32(BRCMSTB_CHIP_ID_ADDR)) {
        case 0x72680010:
            INFO_MSG("BCM7268 B0 chip detected");
            brcmstb_params = bcm7268b0_params;
            break;
        case 0x72710010:
            INFO_MSG("BCM7271 B0 chip detected");
            brcmstb_params = bcm7268b0_params;
            break;
        default:
            ERR_MSG("No platform parameters and chip ID unknown");
            SYS_HALT();
        }
    }
    else {
        phdr = (param_hdr_t *)plat_params_addr;

        ASSERT(phdr->type == BRCMSTB_PARAMS);
        ASSERT(phdr->version <= BRCMSTB_PARAMS_VERSION);

        switch (phdr->version) {
        case BRCMSTB_PARAMS_VERSION:
            brcmstb_params = *(brcmstb_params_t *)phdr;
            break;
        default:
            ERR_MSG("Unknown platform parameters version");
            SYS_HALT();
        }
    }

#ifdef DEBUG
    DBG_PRINT("\nBrcmstb parameters:\n");
    for (rgroup_idx = 0; rgroup_idx < BRCMSTB_RGROUP_LAST; rgroup_idx++) {
        prgroup = &brcmstb_params.rgroups[rgroup_idx];
        DBG_PRINT("  Rgroup %d: rev %d, base 0x%x, size 0x%x\n",
                  (int)rgroup_idx,
                  prgroup->rev, prgroup->base, prgroup->size);
    }
#endif

    /* Add brcmstb reg group bases to mmap regions */
    for (rgroup_idx = 0; rgroup_idx < BRCMSTB_RGROUP_LAST; rgroup_idx++) {
        uintptr_t prgroup_page;

        prgroup = &brcmstb_params.rgroups[rgroup_idx];
        prgroup_page = ALIGN_TO_PAGE(prgroup->base);
        mmap_add_region(
            prgroup_page,
            prgroup_page,
            prgroup->base + prgroup->size - prgroup_page,
            MT_SOC_REG | MT_SECURE);
    }
}

void plat_init(void)
{
    /* TBD */
    INFO_MSG("Brcmstb platform init done");
}
