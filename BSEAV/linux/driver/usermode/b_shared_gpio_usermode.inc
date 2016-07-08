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
 * B_SHARED_GPIO_SPIN_LOCK(flags) - locks the interrupt mgmt spinlock
 * B_SHARED_GPIO_SPIN_UNLOCK(flags) - unlocks the interrupt mgmt spinlock
 *
 * This file should only be included from bcm_driver.c
 */
void b_shared_gpio_enable_irqs(void)
{
    unsigned i, j;

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for(i=0;i<sizeof(g_shared_gpio_state.banks)/sizeof(g_shared_gpio_state.banks[0]);i++)
    {
        for (j=0;j<sizeof(g_shared_gpio_state.banks[i].pins)/sizeof(g_shared_gpio_state.banks[i].pins[0]);j++)
        {
            if(g_shared_gpio_state.banks[i].pins[j].irq.linux_irq)
            {
                if(!g_shared_gpio_state.banks[i].pins[j].irq.disabled && !g_shared_gpio_state.banks[i].pins[j].irq.linux_enabled)
                {
                    if (g_brcm_gpio_verbose) PINFO("gpio irq enable %#x:%u linux irq %u\n", g_shared_gpio_state.banks[i].base_address, g_shared_gpio_state.banks[i].pins[j].shift, g_shared_gpio_state.banks[i].pins[j].irq.linux_irq);
                    g_shared_gpio_state.banks[i].pins[j].irq.linux_enabled = true;
                    enable_irq(g_shared_gpio_state.banks[i].pins[j].irq.linux_irq);
                }
            }
        }
    }
}

void b_shared_gpio_reenable_irqs(void)
{
    unsigned i, j;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for(i=0;i<sizeof(g_shared_gpio_state.banks)/sizeof(g_shared_gpio_state.banks[0]);i++)
    {
        for (j=0;j<sizeof(g_shared_gpio_state.banks[i].pins)/sizeof(g_shared_gpio_state.banks[i].pins[0]);j++)
        {
            if (g_shared_gpio_state.banks[i].pins[j].irq.linux_irq)
            {
                B_SHARED_GPIO_SPIN_LOCK(flags);
                if(!g_shared_gpio_state.banks[i].pins[j].irq.linux_enabled)
                {
                    if (g_brcm_gpio_verbose) PINFO("gpio reenable %#x:%u linux irq %u\n", g_shared_gpio_state.banks[i].base_address, g_shared_gpio_state.banks[i].pins[j].shift, g_shared_gpio_state.banks[i].pins[j].irq.linux_irq);
                    g_shared_gpio_state.banks[i].pins[j].irq.linux_enabled = true;
                    enable_irq(g_shared_gpio_state.banks[i].pins[j].irq.linux_irq);
                }
                B_SHARED_GPIO_SPIN_UNLOCK(flags);
            }
        }
    }
}
