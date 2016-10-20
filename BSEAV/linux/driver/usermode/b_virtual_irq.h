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
#ifndef B_VIRTUAL_IRQ_H
#define B_VIRTUAL_IRQ_H 1

#include "b_virtual_irq_types.h"

/*
 * TODO: make this rely only on CONFIG_BRCMSTB_NEXUS_API and BRCMSTB_H_VERSION
 * and then use return value of brcmstb_get_l2_irq_id to determine at runtime
 * whether linux really has the interrupts or not
 */
#define B_VIRTUAL_IRQ_OS_SUPPORT (\
    (defined(CONFIG_ARM64) || defined(CONFIG_BCM7120_L2_IRQ)) && \
    defined(CONFIG_BRCMSTB_NEXUS_API) && \
    BRCMSTB_H_VERSION >= 5)

typedef struct b_virtual_irq_capabilities {
    bool feature_supported;
} b_virtual_irq_capabilities;

void b_virtual_irq_init_submodule(void);
void b_virtual_irq_uninit_submodule(void);
int b_virtual_irq_open_submodule(void);
void b_virtual_irq_close_submodule(void);

void b_virtual_irq_get_capabilities(b_virtual_irq_capabilities * caps);

bool b_virtual_irq_l1_is_virtual(const char * name); /* returns bool */
int b_virtual_irq_get_linux_irq(b_virtual_irq_line line);
void b_virtual_irq_software_l2_isr(b_virtual_irq_line line);

void b_virtual_irq_get_status(b_virtual_irq_status * status);
int b_virtual_irq_clear(b_virtual_irq_line line);
int b_virtual_irq_set_mask(b_virtual_irq_line line, bool disabled);
int b_virtual_irq_make_group(const b_virtual_irq_group * group);
/* this function expects the caller to acquire the irq spinlock */
void b_virtual_irq_reenable_irqs_spinlocked(void);

#endif /* B_VIRTUAL_IRQ_H */
