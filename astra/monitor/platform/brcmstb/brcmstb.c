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
#include "brcmstb_params.h"
#include "brcmstb_params_v1.h"
#include "brcmstb_params_v2.h"
#include "brcmstb_priv.h"

brcmstb_params_t brcmstb_params __boot_params;

/* Default values for BCM7268 B0, BCM7271 B0 */
static brcmstb_params_t bcm7268b0_params =
{
    .hdr =
    {
        .type = BRCMSTB_PARAMS,
        .version = BRCMSTB_PARAMS_VERSION
    },
    .rgroups =
    {
        /* BRCMSTB_RGROUP_BOOTSRAM */
        {
            .base = 0xffe00000,
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
            .size = 0x00000400
        },
        /* BRCMSTB_RGROUP_HIF_CONTINUATION */
        {
            .base = 0xf0452000,
            .size = 0x00000100
        },
        /* BRCMSTB_RGROUP_HIF_CPU_INTR1 */
        {
            .base = 0xf0201500,
            .size = 0x00000040
        },
        /* BRCMSTB_RGROUP_AVS_CPU_DATA_MEM */
        {
            .base = 0xf04c4000,
            .size = 0x00000e80
        },
        /* BRCMSTB_RGROUP_AVS_HOST_L2 */
        {
            .base = 0xf04d1200,
            .size = 0x00000048
        },
        /* BRCMSTB_RGROUP_AVS_CPU_L2 */
        {
            .base = 0xf04d1100,
            .size = 0x00000030
        },
        /* BRCMSTB_RGROUP_SCPU_GLOBALRAM */
        {
            .base = 0xf0310000,
            .size = 0x00000400
        },
        /* BRCMSTB_RGROUP_SCPU_HOST_INTR2 */
        {
            .base = 0xf0311040,
            .size = 0x00000030
        },
        /* BRCMSTB_RGROUP_CPU_IPI_INTR2 */
        {
            .base = 0xf0311000,
            .size = 0x00000030
        },
        /* BRCMSTB_RGROUP_AON_CTRL */
        {
            .base = 0xf0410000,
            .size = 0x00000600
        },
    },
    .intrs =
    {
        /* BRCMSTB_INTR_AVS_CPU */
        25,
        /* BRCMSTB_INTR_SCPU_CPU */
        1,
    }
};

#ifdef DEBUG
static void brcmstb_params_dump()
{
    size_t i;

    DBG_PRINT("\nBrcmstb parameters:\n");
    DBG_PRINT("  Register groups:\n");
    for (i = 0; i < BRCMSTB_RGROUP_LAST; i++) {
        brcmstb_rgroup_t *prgroup;
        prgroup = &brcmstb_params.rgroups[i];
        DBG_PRINT("    rgroup %d: rev %d, base 0x%x, size 0x%x\n",
                  (int)i, prgroup->rev, prgroup->base, prgroup->size);
    }
    DBG_PRINT("\n");
    DBG_PRINT("  Interrupts:\n");
    for (i = 0; i < BRCMSTB_INTR_LAST; i++)
        DBG_PRINT("    intr %d: %d\n", (int)i, brcmstb_params.intrs[i]);
}
#endif

typedef brcmstb_params_t brcmstb_params_v3_t;

static void brcmstb_params_conv2(
    brcmstb_params_v3_t *pv3_params,
    brcmstb_params_v2_t *pv2_params)
{
    param_hdr_t *pv2_hdr;

    /* Check header */
    pv2_hdr = &pv2_params->hdr;

    DBG_ASSERT(pv2_hdr->type == BRCMSTB_PARAMS);
    DBG_ASSERT(pv2_hdr->version == 2);
#ifndef DEBUG
    UNUSED(pv2_hdr);
#endif

    /* Clear brcmstb params v3 to be safe */
    bootstrap_memset(pv3_params, 0, sizeof(brcmstb_params_v3_t));

    /* Convert header */
    pv3_params->hdr.type = BRCMSTB_PARAMS;
    pv3_params->hdr.version = 3;

    /* Copy brcmstb params v2 SOC register groups */
    bootstrap_memcpy(
        pv3_params->rgroups,
        pv2_params->rgroups,
        sizeof(brcmstb_rgroup_t) * BRCMSTB_RGROUP_LAST_V2);

    /* Copy brcmstb params interrupts */
    bootstrap_memcpy(
        pv3_params->intrs,
        pv2_params->intrs,
        sizeof(uint8_t) * BRCMSTB_INTR_LAST);
}

static void brcmstb_params_conv1(
    brcmstb_params_v2_t *pv2_params,
    brcmstb_params_v1_t *pv1_params)
{
    param_hdr_t *pv1_hdr;
    size_t i;

    /* Check header */
    pv1_hdr = &pv1_params->hdr;

    DBG_ASSERT(pv1_hdr->type == BRCMSTB_PARAMS);
    DBG_ASSERT(pv1_hdr->version == 1);
#ifndef DEBUG
    UNUSED(pv1_hdr);
#endif

    /* Clear brcmstb params v2 to be safe */
    bootstrap_memset(pv2_params, 0, sizeof(brcmstb_params_v2_t));

    /* Copy brcmstb params v1 entirely */
    bootstrap_memcpy(
        pv2_params,
        pv1_params,
        sizeof(brcmstb_params_v1_t));

    /* Update header */
    pv2_params->hdr.version = 2;

    /* Fill in additional fields */
    for (i = 0; i < BRCMSTB_INTR_LAST; i++)
        pv2_params->intrs[i] = 0xFF;
}

void plat_early_init(uintptr_t plat_params_addr)
{
    if (plat_params_addr) {
        /* Cold boot, init brcmstb params */
        brcmstb_params_t *pbuiltin_params = NULL;
        param_hdr_t *phdr;

        phdr = (param_hdr_t *)plat_params_addr;

        ASSERT(phdr->type == BRCMSTB_PARAMS);
        ASSERT(phdr->version <= BRCMSTB_PARAMS_VERSION);

        /*
         * Do our best for Bolt backward compatibility. If platform params
         * is not up-to-date, we may miss some hardware defines.
         *
         * Try to identify the chip by reading the chip ID register from the
         * SUN_TOP_CTRL register in the out-of-date brcmstb params.
         *
         * The chip ID is used to decide if one of the built-in brcmstb params
         * can be used. This is best effort, and may NOT work for all chips.
         */
        if (phdr->type == BRCMSTB_PARAMS &&
            phdr->version < BRCMSTB_PARAMS_VERSION) {

            brcmstb_params_t *pv0_params =
                (brcmstb_params_t *)plat_params_addr;
            uintptr_t raddr =
                pv0_params->rgroups[BRCMSTB_RGROUP_SUN_TOP_CTRL].base;
            uint32_t chip_id = MMIO32(raddr);

            switch (chip_id & ~0xf) {
            case 0x72680010:
            case 0x72710010:
            case 0x72550000:
                pbuiltin_params = &bcm7268b0_params;
                break;
            }

            if (pbuiltin_params) {
                INFO_MSG("BCM%x %c%c chip detected",
                         ((chip_id >> 16)),              /* chip ID number */
                         ((chip_id >> 4 ) & 0xf) + 'A',  /* chip major revision */
                         ((chip_id      ) & 0xf) + '0'); /* chip minor revision */

                bootstrap_memcpy(
                    &brcmstb_params,
                    pbuiltin_params,
                    sizeof(brcmstb_params_t));
            }

            /* Adjustment for some chips */
            if ((chip_id & ~0xf) == 0x72550000) {
                brcmstb_rgroup_t *prgroup =
                    &brcmstb_params.rgroups[BRCMSTB_RGROUP_HIF_CPUBIUCTRL];
                prgroup->base = 0xf0202400;
            }
        }

        if (!pbuiltin_params) {
            switch (phdr->version) {
                brcmstb_params_v2_t brcmstb_params_v2;

            case BRCMSTB_PARAMS_VERSION:
                /* Copy brcmstb params directly */
                bootstrap_memcpy(
                    &brcmstb_params,
                    (brcmstb_params_t *)plat_params_addr,
                    sizeof(brcmstb_params_t));
                break;

            case 2:
                /* Convert brcmstb params from v2 */
                brcmstb_params_conv2(
                    &brcmstb_params,
                    (brcmstb_params_v2_t *)plat_params_addr);
                break;

            case 1:
                /* Convert brcmstb params from v1 */
                brcmstb_params_conv1(
                    &brcmstb_params_v2,
                    (brcmstb_params_v1_t *)plat_params_addr);

                brcmstb_params_conv2(
                    &brcmstb_params,
                    &brcmstb_params_v2);
                break;

            default:
                ERR_MSG("Unknown platform parameters version");
                SYS_HALT();
            }
        }
    }
    else {
        /* Warm boot, reuse brcmstb params */
        ASSERT(brcmstb_params.hdr.type == BRCMSTB_PARAMS);
        ASSERT(brcmstb_params.hdr.version == BRCMSTB_PARAMS_VERSION);
    }
}

void plat_mmap_init(void)
{
    size_t i;

    /* Add brcmstb SOC register group bases to mmap regions */
    for (i = 0; i < BRCMSTB_RGROUP_LAST; i++) {
        brcmstb_rgroup_t *prgroup;
        uintptr_t prgroup_page;

        prgroup = &brcmstb_params.rgroups[i];
        prgroup_page = ALIGN_TO_PAGE(prgroup->base);
        mmap_add_region(
            prgroup_page,
            prgroup_page,
            prgroup->base + prgroup->size - prgroup_page,
            MT_SOC_REG | MT_SEC);
    }

#ifdef DEBUG
    brcmstb_params_dump();
#endif
}

void plat_init(void)
{
    /* TBD */
    INFO_MSG("Brcmstb platform init done");
}
