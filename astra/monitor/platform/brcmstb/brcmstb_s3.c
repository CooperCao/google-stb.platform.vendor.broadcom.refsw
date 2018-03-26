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

#include "monitor.h"
#include "memory.h"
#include "brcmstb_priv.h"
#include "brcmstb_s3.h"

static volatile uint32_t *aon_regs;
static struct brcmstb_s3_params *ps3_params;

/***************************************************************************
 *SYSTEM_DATA_RAM%i - System Data RAM Address 0..255
 ***************************************************************************/
#define BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE                  0x00000200
#define BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_START                 0
#define BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_END                   255

void plat_early_s3_init(void)
{
    uintptr_t raddr;
    uintptr_t ps3_params_page;

    raddr = BRCMSTB_REG_ADDR(AON_CTRL, SYSTEM_DATA_RAMi_ARRAY_BASE);

    aon_regs = (volatile uint32_t *)raddr;

    ps3_params = (struct brcmstb_s3_params *)
        (((uintptr_t)aon_regs[AON_REG_CONTROL_LOW ]      ) |
         ((uintptr_t)aon_regs[AON_REG_CONTROL_HIGH] << 32));

    ASSERT(ps3_params);

    ps3_params_page = ALIGN_TO_PAGE((uintptr_t)ps3_params);
    mmap_add_region(
        ps3_params_page,
        ps3_params_page,
        sizeof(struct brcmstb_s3_params),
        MT_RW_DATA);
}

void plat_s3_init(void)
{
    /* TBD */
    INFO_MSG("Brcmstb platform s3 init done");
}

uintptr_t plat_s3_nsec_reentry_point(void)
{
    return ps3_params->reentry;
}
