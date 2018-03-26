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

#ifndef _GICV2_H_
#define _GICV2_H_

#include <stdint.h>

int gicv2_dist_init(uintptr_t dist_base);
int gicv2_cpuif_init(uintptr_t cpuif_base);

int gicv2_sec_intr_enable(
    uintptr_t dist_base,
    uint32_t intr_id);

int gicv2_sec_intr_disable(
    uintptr_t dist_base,
    uint32_t intr_id);

int gicv2_sgi_intr_generate(
    uintptr_t dist_base,
    uint32_t intr_id,
    uint32_t cpu_mask);

/* Inline functions for speed */
#include "gicv2_priv.h"

static inline uint32_t gicv2_intr_max(uintptr_t dist_base)
{
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    return ((pdist[GICD_TYPER] & 0x1f) + 1) * 32;
}

static inline uint32_t gicv2_intr_iid(uintptr_t cpuif_base)
{
    volatile uint32_t *pcpuif = (uint32_t *)cpuif_base;
    return pcpuif[GICC_HPPIR];
}

static inline void gicv2_intr_ack(uintptr_t cpuif_base, uint32_t iid)
{
    volatile uint32_t *pcpuif = (uint32_t *)cpuif_base;
    pcpuif[GICC_IAR] = iid;
}

static inline void gicv2_intr_end(uintptr_t cpuif_base, uint32_t iid)
{
    volatile uint32_t *pcpuif = (uint32_t *)cpuif_base;
    pcpuif[GICC_EOIR] = iid;
}

#endif /* _GICV2_H_ */
