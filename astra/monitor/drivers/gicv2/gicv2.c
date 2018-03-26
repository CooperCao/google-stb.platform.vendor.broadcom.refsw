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

#include "monitor.h"
#include "gicv2.h"
#include "gicv2_priv.h"

#ifdef VERBOSE
#define FMT_02D "%s%d"
#define SPC_02D(i) ((i < 10) ? " " : "")

static void gicv2_dist_dump(uintptr_t dist_base)
{
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    int i;

    DBG_PRINT("\nGIC distributor registers:\n");
    DBG_PRINT("  GICD_CTLR:                0x%x\n", pdist[GICD_CTLR]);
    DBG_PRINT("  GICD_TYPER:               0x%x\n", pdist[GICD_TYPER]);
    DBG_PRINT("  GICD_IIDR:                0x%x\n", pdist[GICD_IIDR]);

    DBG_PRINT("  (1 bit per interrupt)\n");
    DBG_PRINT("  GICD_IGROUPRn[0-1]:       0x%0x 0x%0x\n",
              pdist[GICD_IGROUPRn],
              pdist[GICD_IGROUPRn + 1]);

    DBG_PRINT("  GICD_ISENABLERn[0-1]:     0x%0x 0x%0x\n",
              pdist[GICD_ISENABLERn],
              pdist[GICD_ISENABLERn + 1]);

    DBG_PRINT("  GICD_ICENABLERn[0-1]:     0x%0x 0x%0x\n",
              pdist[GICD_ICENABLERn],
              pdist[GICD_ICENABLERn + 1]);

    DBG_PRINT("  GICD_ISPENDRn[0-1]:       0x%0x 0x%0x\n",
              pdist[GICD_ISPENDRn],
              pdist[GICD_ISPENDRn + 1]);

    DBG_PRINT("  GICD_ICPENDRn[0-1]:       0x%0x 0x%0x\n",
              pdist[GICD_ICPENDRn],
              pdist[GICD_ICPENDRn + 1]);

    DBG_PRINT("  GICD_ISACTIVERn[0-1]:     0x%0x 0x%0x\n",
              pdist[GICD_ISACTIVERn],
              pdist[GICD_ISACTIVERn + 1]);

    DBG_PRINT("  GICD_ICACTIVERn[0-1]:     0x%0x 0x%0x\n",
              pdist[GICD_ICACTIVERn],
              pdist[GICD_ICACTIVERn + 1]);

    DBG_PRINT("  (8-bit per interrupts)\n");
    for (i = 0; i < 64; i += 4)
        DBG_PRINT("  GICD_IPRIORITYRn[" FMT_02D "-" FMT_02D "]:  0x%0x 0x%0x 0x%0x 0x%0x\n",
                  SPC_02D(i), i, SPC_02D(i + 3), i + 3,
                  pdist[GICD_IPRIORITYRn + i],
                  pdist[GICD_IPRIORITYRn + i + 1],
                  pdist[GICD_IPRIORITYRn + i + 2],
                  pdist[GICD_IPRIORITYRn + i + 3]);

    for (i = 0; i < 64; i += 4)
        DBG_PRINT("  GICD_ITARGETSRn[" FMT_02D "-" FMT_02D "]:   0x%0x 0x%0x 0x%0x 0x%0x\n",
                  SPC_02D(i), i, SPC_02D(i + 3), i + 3,
                  pdist[GICD_ITARGETSRn + i],
                  pdist[GICD_ITARGETSRn + i + 1],
                  pdist[GICD_ITARGETSRn + i + 2],
                  pdist[GICD_ITARGETSRn + i + 3]);

    for (i = 0; i < 16; i += 4)
        DBG_PRINT("  GICD_CPENDSGIRn[" FMT_02D "-" FMT_02D "]:   0x%0x 0x%0x 0x%0x 0x%0x\n",
                  SPC_02D(i), i, SPC_02D(i + 3), i + 3,
                  pdist[GICD_CPENDSGIRn + i],
                  pdist[GICD_CPENDSGIRn + i + 1],
                  pdist[GICD_CPENDSGIRn + i + 2],
                  pdist[GICD_CPENDSGIRn + i + 3]);

    DBG_PRINT("  (2-bit per interrupts)\n");
    DBG_PRINT("  GICD_ICFGRn[0-3]:         0x%0x 0x%0x 0x%0x 0x%0x\n",
              pdist[GICD_ICFGRn],
              pdist[GICD_ICFGRn + 1],
              pdist[GICD_ICFGRn + 2],
              pdist[GICD_ICFGRn + 3]);

    DBG_PRINT("  GICD_NSACRn[0-3]:         0x%0x 0x%0x 0x%0x 0x%0x\n",
              pdist[GICD_NSACRn],
              pdist[GICD_NSACRn + 1],
              pdist[GICD_NSACRn + 2],
              pdist[GICD_NSACRn + 3]);
}

static void gicv2_cpuif_dump(uintptr_t cpuif_base)
{
    volatile uint32_t *pcpuif = (uint32_t *)cpuif_base;

    DBG_PRINT("\nGIC CPU interface registers:\n");
    DBG_PRINT("  GICC_CTLR:                0x%x\n", pcpuif[GICC_CTLR]);
    DBG_PRINT("  GICC_IIDR:                0x%x\n", pcpuif[GICC_IIDR]);
    DBG_PRINT("  GICC_PMR:                 0x%x\n", pcpuif[GICC_PMR]);
    DBG_PRINT("  GICC_BPR:                 0x%x\n", pcpuif[GICC_BPR]);
    DBG_PRINT("  GICC_IAR:                 0x%x\n", pcpuif[GICC_IAR]);
    DBG_PRINT("  GICC_RPR:                 0x%x\n", pcpuif[GICC_RPR]);
    DBG_PRINT("  GICC_HPPIR:               0x%x\n", pcpuif[GICC_HPPIR]);
    DBG_PRINT("  GICC_ABPR:                0x%x\n", pcpuif[GICC_ABPR]);
    DBG_PRINT("  GICC_AIAR:                0x%x\n", pcpuif[GICC_AIAR]);
    DBG_PRINT("  GICC_AHPPIR:              0x%x\n", pcpuif[GICC_AHPPIR]);
}
#endif

int gicv2_dist_init(uintptr_t dist_base)
{
    static uint32_t init_cnt = 0;
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    size_t nintr32;
    size_t i, j;

    DBG_MSG("Initializing GIC distributor base @ 0x%lx...", dist_base);

    /* Disable distributor */
    pdist[GICD_CTLR] = 0x0;

    /* Get number of interrupts in multiples of 32 */
    nintr32 = (pdist[GICD_TYPER] & 0x1f) + 1;

    if (!init_cnt) {
        /* Init SPI:
         * - Set to default non-secure priority (0xc0)
         * - Assign to Group 1 by default
         * - Clear status
         * - isable forwarding
         */
        for (i = 1; i < nintr32; i++) {
            for (j = 0; j < 8; j++)
                pdist[GICD_IPRIORITYRn + i * 8 + j] = 0xd0d0d0d0;

            pdist[GICD_IGROUPRn   + i] = 0xffffffff;
            pdist[GICD_ICACTIVERn + i] = 0xffffffff;
            pdist[GICD_ICPENDRn   + i] = 0xffffffff;
            pdist[GICD_ICENABLERn + i] = 0xffffffff;
        }
    }

    /* Init SGI and PPI:
     * - Set to default non-secure priority (0xc0)
     * - Assign to Group 1 by default
     * - Clear status
     * - (SGI) Enable forwarding
     * - (PPI) clear-enable (disable) forwarding
     */
    for (j = 0; j < 8; j++)
        pdist[GICD_IPRIORITYRn + j] = 0xc0c0c0c0;

    pdist[GICD_IGROUPRn  ] = 0xffffffff;
    pdist[GICD_ICACTIVERn] = 0xffffffff;
    pdist[GICD_ICPENDRn  ] = 0xffffffff;
    pdist[GICD_ISENABLERn] = 0x0000ffff;
    pdist[GICD_ICENABLERn] = 0xffff0000;

    /* Enable distributor */
    pdist[GICD_CTLR] = 0x3;
    init_cnt++;

#ifdef VERBOSE
    gicv2_dist_dump(dist_base);
#endif
    return 0;
}

int gicv2_cpuif_init(uintptr_t cpuif_base)
{
    volatile uint32_t *pcpuif = (uint32_t *)cpuif_base;

    DBG_MSG("Initializing GIC CPU interface base @ 0x%lx...", cpuif_base);

    /* Disable CPU interface */
    pcpuif[GICC_CTLR] = 0x0;

    /* Set priority masking to allow all interrupts */
    pcpuif[GICC_PMR] = 0xff;

    /* Set binary point to use entire priority value */
    pcpuif[GICC_BPR] = 0x0;

    /* Enable CPU interface with Group 0 using FIQ */
    pcpuif[GICC_CTLR] = 0xb;

#ifdef VERBOSE
    gicv2_cpuif_dump(cpuif_base);
#endif
    return 0;
}

int gicv2_sec_intr_enable(
    uintptr_t dist_base,
    uint32_t intr_id)
{
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    size_t i, p;

    DBG_MSG("Enabling secure interrupt %d @ 0x%lx...", intr_id, dist_base);

    /* Check group */
    i = intr_id >> 5;
    p = intr_id & 0x1f;
    if (pdist[GICD_IGROUPRn + i] & (0x1 << p))
        /* Assign to Group 0 */
        pdist[GICD_IGROUPRn + i] &= ~(0x1 << p);
    else
        /* Already in Group 0 */
        return 0;

    /* Set to default secure priority (0x40) */
    i = intr_id >> 2;
    p = intr_id & 0x3;
    pdist[GICD_IPRIORITYRn + i] &= ~(0xff << (p << 3));
    pdist[GICD_IPRIORITYRn + i] |=  (0x40 << (p << 3));

    /* Clear status and enable forwarding */
    i = intr_id >> 5;
    p = intr_id & 0x1f;
    pdist[GICD_ICACTIVERn + i] = (0x1 << p);
    pdist[GICD_ICPENDRn   + i] = (0x1 << p);
    pdist[GICD_ISENABLERn + i] = (0x1 << p);

#ifdef VERBOSE
    gicv2_dist_dump(dist_base);
#endif
    return 0;
}

int gicv2_sec_intr_disable(
    uintptr_t dist_base,
    uint32_t intr_id)
{
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    size_t i, p;

    DBG_MSG("Disabling secure interrupt %d @ 0x%lx...", intr_id, dist_base);

    /* Check group */
    i = intr_id >> 5;
    p = intr_id & 0x1f;
    if (pdist[GICD_IGROUPRn + i] & (0x1 << p))
        /* Already in Group 1 */
        return 0;
    else
        /* Assign to Group 1 */
        pdist[GICD_IGROUPRn + i] |= (0x1 << p);

    /* Set to default non-secure priority (0xc0) */
    i = intr_id >> 2;
    p = intr_id & 0x3;
    pdist[GICD_IPRIORITYRn + i] &= ~(0xff << (p << 3));
    pdist[GICD_IPRIORITYRn + i] |= ~(0xc0 << (p << 3));

    /* Clear status and disable forwarding for non-SGI */
    i = intr_id >> 5;
    p = intr_id & 0x1f;
    pdist[GICD_ICACTIVERn + i] = (0x1 << p);
    pdist[GICD_ICPENDRn   + i] = (0x1 << p);
    if (intr_id >= 16)
        pdist[GICD_ICENABLERn + i] = (0x1 << p);

#ifdef VERBOSE
    gicv2_dist_dump(dist_base);
#endif
    return 0;
}

int gicv2_sgi_intr_generate(
    uintptr_t dist_base,
    uint32_t intr_id,
    uint32_t cpu_mask)
{
    volatile uint32_t *pdist = (uint32_t *)dist_base;
    uint32_t nsec;
    size_t i, p;

#ifdef VERBOSE
    DBG_MSG("Generating SGI interrupt %d @ 0x%lx...", intr_id, dist_base);
#endif

    /* Check group */
    i = intr_id >> 5;
    p = intr_id & 0x1f;
    nsec = (pdist[GICD_IGROUPRn + i] & (0x1 << p)) ? 1 : 0;

    pdist[GICD_SGIR] =
        ((intr_id  &  0xf) <<  0) |
        ((nsec           ) << 15) |
        ((cpu_mask & 0xff) << 16);

    return 0;
}
