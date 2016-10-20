/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
/*
 * This "macro" expects the following defines are available before including
 * this file into your C file:
 *
 * B_VIRTUAL_IRQ_SPIN_LOCK(flags) - locks the interrupt mgmt spinlock
 * B_VIRTUAL_IRQ_SPIN_UNLOCK(flags) - unlocks the interrupt mgmt spinlock
 * B_VIRTUAL_IRQ_GET_L1_WORD_COUNT() - returns the number of L1 status words on this platform
 * B_VIRTUAL_IRQ_UNMASK_L1(L1) - unmasks an L1 interrupt
 * B_VIRTUAL_IRQ_SET_L1_STATUS(L1) - sets (value = 1) the status bit of an L1 interrupt
 * B_VIRTUAL_IRQ_INC_L1(L1) - increments the interrupt count of a given L1 interrupt
 * B_VIRTUAL_IRQ_WAKE_L1_LISTENERS() - wakes up any listeners waiting for L1 to fire
 *
 * This file should only be included from bcm_driver.c
 */

void b_virtual_irq_reset_irqs(void)
{
    unsigned i;
    unsigned long flags;
    struct b_virtual_irq_line_desc * desc;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }

    /* TODO: reset to insmod state */

    for(i = 0; i < sizeof(g_virtual_irq_state.lines)/sizeof(g_virtual_irq_state.lines[0]); i++)
    {
        desc = &g_virtual_irq_state.lines[i];
        B_VIRTUAL_IRQ_SPIN_LOCK(flags);
        b_os_irq_set_enable_spinlocked_tagged(&desc->base, 1, "virq reset", __LINE__);
        B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
    }
}

void b_virtual_irq_test(void)
{
    unsigned i;
    struct b_virtual_irq_line_desc * desc;
    unsigned long flags;

    if (!g_virtual_irq_state.initialized || !g_virtual_irq_state.caps.feature_supported) { return; }

    for(i = 0; i < sizeof(g_virtual_irq_state.lines)/sizeof(g_virtual_irq_state.lines[0]); i++)
    {
        desc = &g_virtual_irq_state.lines[i];
        /* this doesn't "fire" l1, and it doesn't clear "fired" state either
         * it only updates the l1 status to match the fired state */
        if (desc->fired)
        {
            B_VIRTUAL_IRQ_SPIN_LOCK(flags);
            if (!desc->disabled)
            {
                b_virtual_irq_set_l1_status(desc);
            }
            B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
        }
    }
}
