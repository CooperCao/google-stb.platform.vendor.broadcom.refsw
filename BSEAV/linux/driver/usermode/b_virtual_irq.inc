/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
/*
 * This "macro" expects the following defines are available before including
 * this file into your C file:
 *
 * B_VIRTUAL_IRQ_SPIN_LOCK(flags) - locks the interrupt mgmt spinlock
 * B_VIRTUAL_IRQ_SPIN_UNLOCK(flags) - unlocks the interrupt mgmt spinlock
 * B_VIRTUAL_IRQ_GET_L1_WORD_COUNT() - returns the number of L1 status words on this platform
 * B_VIRTUAL_IRQ_MASK_L1(L1) - masks an L1 interrupt
 * B_VIRTUAL_IRQ_UNMASK_L1(L1) - unmasks an L1 interrupt
 * B_VIRTUAL_IRQ_SET_L1_STATUS(L1) - sets (value = 1) the status bit of an L1 interrupt
 * B_VIRTUAL_IRQ_INC_L1(L1) - increments the interrupt count of a given L1 interrupt
 * B_VIRTUAL_IRQ_WAKE_L1_LISTENERS() - wakes up any listeners waiting for L1 to fire
 */
#include "b_os_irq.h"
static bool g_brcm_virq_verbose = false;

struct b_virtual_irq_line_desc {
    struct b_os_irq_desc base;
    b_virtual_irq_line line;
    unsigned group_id; /* 0 - means invalid */
    bool disabled;
    bool fired;
};

static const struct b_virtual_irq_name_map_entry {
    b_virtual_irq_line irq_line;
    char name[16];
} g_virtual_irq_name_map[] = {
    {b_virtual_irq_line_iica,     "iica"},
    {b_virtual_irq_line_iicb,     "iicb"},
    {b_virtual_irq_line_iicc,     "iicc"},
    {b_virtual_irq_line_iicd,     "iicd"},
    {b_virtual_irq_line_iice,     "iice"},
    {b_virtual_irq_line_iicf,     "iicf"},
    {b_virtual_irq_line_iicg,     "iicg"},
    {b_virtual_irq_line_gio,      "gio"},
    {b_virtual_irq_line_gio_aon,  "gioa"},
    {b_virtual_irq_line_irb,      "irb"},
    {b_virtual_irq_line_icap,     "icap"},
    {b_virtual_irq_line_kbd1,     "kbd1"},
    {b_virtual_irq_line_kbd2,     "kbd2"},
    {b_virtual_irq_line_kbd3,     "kbd3"},
    {b_virtual_irq_line_ldk,      "ldk"},
    {b_virtual_irq_line_spi,      "spi"},
    {b_virtual_irq_line_ua,       "ua"},
    {b_virtual_irq_line_ub,       "ub"},
    {b_virtual_irq_line_uc,       "uc"},
    {b_virtual_irq_line_bicap_fifo_inact_intr,  "bicap_fifo_inact"},
    {b_virtual_irq_line_bicap_fifo_lvl_intr,    "bicap_fifo_lvl"},
    {b_virtual_irq_line_bicap_fifo_of_intr,     "bicap_fifo_of"},
    {b_virtual_irq_line_bicap_timeout0_intr,    "bicap_timeout0"},
    {b_virtual_irq_line_bicap_timeout1_intr,    "bicap_timeout1"},
    {b_virtual_irq_line_bicap_timeout2_intr,    "bicap_timeout2"},
    {b_virtual_irq_line_bicap_timeout3_intr,    "bicap_timeout3"},
};

static const char * b_virtual_irq_get_name(b_virtual_irq_line irq_line)
{
    unsigned i;
    const char * name = NULL;
    for (i = 0; i < sizeof(g_virtual_irq_name_map)/sizeof(g_virtual_irq_name_map[0]); i++) {
        if (irq_line == g_virtual_irq_name_map[i].irq_line) {
            name = g_virtual_irq_name_map[i].name;
            break;
        }
    }

    return name;
}

#if B_VIRTUAL_IRQ_OS_SUPPORT
static const struct b_virtual_irq_map_entry {
    b_virtual_irq_line irq_line;
    brcmstb_l2_irq linux_irq;
} g_virtual_irq_map[] = {
    {b_virtual_irq_line_iica, brcmstb_l2_irq_iica},
    {b_virtual_irq_line_iicb, brcmstb_l2_irq_iicb},
    {b_virtual_irq_line_iicc, brcmstb_l2_irq_iicc},
    {b_virtual_irq_line_iicd, brcmstb_l2_irq_iicd},
    {b_virtual_irq_line_iice, brcmstb_l2_irq_iice},
#if BRCMSTB_H_VERSION >= 7
    {b_virtual_irq_line_iicf, brcmstb_l2_irq_iicf},
    {b_virtual_irq_line_iicg, brcmstb_l2_irq_iicg},
#endif
    {b_virtual_irq_line_gio,  brcmstb_l2_irq_gio},
    {b_virtual_irq_line_gio_aon,  brcmstb_l2_irq_gio_aon},
    {b_virtual_irq_line_irb,  brcmstb_l2_irq_irb},
    {b_virtual_irq_line_icap, brcmstb_l2_irq_icap},
    {b_virtual_irq_line_kbd1, brcmstb_l2_irq_kbd1},
    {b_virtual_irq_line_kbd2, brcmstb_l2_irq_kbd2},
    {b_virtual_irq_line_kbd3, brcmstb_l2_irq_kbd3},
    {b_virtual_irq_line_ldk,  brcmstb_l2_irq_ldk},
    {b_virtual_irq_line_spi,  brcmstb_l2_irq_spi},
    {b_virtual_irq_line_ua,  brcmstb_l2_irq_ua},
    {b_virtual_irq_line_ub,  brcmstb_l2_irq_ub},
    {b_virtual_irq_line_uc,  brcmstb_l2_irq_uc},
#if BRCMSTB_H_VERSION >= 11
    {b_virtual_irq_line_bicap_fifo_inact_intr, brcmstb_l2_irq_bicap_fifo_inact_intr},
    {b_virtual_irq_line_bicap_fifo_lvl_intr,   brcmstb_l2_irq_bicap_fifo_lvl_intr},
    {b_virtual_irq_line_bicap_fifo_of_intr,    brcmstb_l2_irq_bicap_fifo_of_intr},
    {b_virtual_irq_line_bicap_timeout0_intr,   brcmstb_l2_irq_bicap_timeout0_intr},
    {b_virtual_irq_line_bicap_timeout1_intr,   brcmstb_l2_irq_bicap_timeout1_intr},
    {b_virtual_irq_line_bicap_timeout2_intr,   brcmstb_l2_irq_bicap_timeout2_intr},
    {b_virtual_irq_line_bicap_timeout3_intr,   brcmstb_l2_irq_bicap_timeout3_intr}
#endif
};

static brcmstb_l2_irq b_virtual_irq_find_linux_l2_by_line(b_virtual_irq_line irq_line)
{
    unsigned i;
    brcmstb_l2_irq l2 = brcmstb_l2_irq_max;
    for (i = 0; i < sizeof(g_virtual_irq_map)/sizeof(g_virtual_irq_map[0]); i++) {
        if (irq_line == g_virtual_irq_map[i].irq_line) {
            l2 = g_virtual_irq_map[i].linux_irq;
            break;
        }
    }

    return l2;
}

#define b_virtual_irq_get_feature_support() b_virtual_irq_check_linux_managed_upg_irqs()

static bool b_virtual_irq_check_linux_managed_upg_irqs(void)
{
    int linux_irq = 0;
    /* if iica isn't supported by linux, nothing else can be */
    linux_irq = brcmstb_get_l2_irq_id(brcmstb_l2_irq_iica);
    BDBG_LOG(("virtual irq: %s (%d)", linux_irq <= 0 ? "disabled" : "enabled", linux_irq));
    return (linux_irq <= 0) ? false : true;
}
#else
#define b_virtual_irq_get_feature_support() false
#endif /* B_VIRTUAL_IRQ_OS_SUPPORT */

static struct b_virtual_irq_state
{
    b_virtual_irq_capabilities caps;
    struct b_virtual_irq_line_desc lines[b_virtual_irq_line_max];
    bool initialized;
} g_virtual_irq_state;

int b_virtual_irq_open_submodule(void)
{
    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq open submodule"));
    return 0;
}

void b_virtual_irq_close_submodule(void)
{
    unsigned i = 0;
    struct b_virtual_irq_line_desc * desc;

    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq close submodule"));

    for (i = 0; i < sizeof(g_virtual_irq_state.lines)/sizeof(g_virtual_irq_state.lines[0]); i++) {
        desc = &g_virtual_irq_state.lines[i];
        b_os_irq_uninit_tagged(&desc->base, "virq close", __LINE__);
    }
}

void b_virtual_irq_init_submodule(void)
{
    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq init submodule"));

    memset(&g_virtual_irq_state, 0, sizeof(g_virtual_irq_state));
    g_virtual_irq_state.caps.feature_supported = b_virtual_irq_get_feature_support();
    g_virtual_irq_state.initialized = true;
}

void b_virtual_irq_uninit_submodule(void)
{
    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq uninit submodule"));
    b_virtual_irq_close_submodule();
    g_virtual_irq_state.initialized = false;
}

void b_virtual_irq_get_capabilities(b_virtual_irq_capabilities * caps)
{
    if (!caps) { BERR_TRACE(-EINVAL); return; }
    memset(caps, 0, sizeof(*caps));
    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }
    *caps = g_virtual_irq_state.caps;
}

bool b_virtual_irq_l1_is_virtual(unsigned int irq)
{
    bool virtual = false;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return 0; }

    if (irq > 0) {
        virtual = b_os_irq_is_os_managed(irq);
    }
    return virtual;
}

static int b_virtual_irq_p_get_l1_shift(struct b_virtual_irq_line_desc * desc)
{
    if (!desc || !desc->group_id) { BERR_TRACE(-EINVAL); return -EINVAL; }
    else { return desc->group_id - 1; }
}

static void b_virtual_irq_set_l1_status(struct b_virtual_irq_line_desc * desc)
{
    int l1_shift;
    l1_shift = b_virtual_irq_p_get_l1_shift(desc);
    if (l1_shift < 0) { BERR_TRACE(l1_shift); return; }
    B_VIRTUAL_IRQ_SET_L1_STATUS(l1_shift);
}

int b_virtual_irq_get_linux_irq(b_virtual_irq_line line)
{
    int linux_irq = 0;

#if B_VIRTUAL_IRQ_OS_SUPPORT
    brcmstb_l2_irq l2 = b_virtual_irq_find_linux_l2_by_line(line);
    if (l2 != brcmstb_l2_irq_max)
    {
        linux_irq = brcmstb_get_l2_irq_id(l2);
    }
    if (linux_irq <= 0) {
        BDBG_ERR(("Can't map Linux L1 for %u(%d,%u,'%s')", line,linux_irq,l2,b_virtual_irq_get_name(line)));
        BERR_TRACE(-EINVAL);
    }
#endif

    return linux_irq;
}

static void b_virtual_irq_inc_l1(struct b_virtual_irq_line_desc * desc)
{
    int l1_shift;
    l1_shift = b_virtual_irq_p_get_l1_shift(desc);
    if (l1_shift < 0) { BERR_TRACE(l1_shift); return; }
    B_VIRTUAL_IRQ_INC_L1(l1_shift);
}

void b_virtual_irq_software_l2_isr(b_virtual_irq_line line)
{
    unsigned long flags;
    struct b_virtual_irq_line_desc * desc;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }

    desc = &g_virtual_irq_state.lines[line];

    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq software l2 isr: %s", b_os_irq_get_name(&desc->base)));

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    desc->fired = true;
    b_virtual_irq_set_l1_status(desc);
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

    B_VIRTUAL_IRQ_WAKE_L1_LISTENERS();

    b_virtual_irq_inc_l1(desc);
}

static void b_virtual_irq_isr(struct b_os_irq_desc * irq, void * context)
{
    struct b_virtual_irq_line_desc *desc = context;
    unsigned long flags;

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    if (!desc->disabled)
    {
        desc->fired = true;
        b_virtual_irq_set_l1_status(desc);
    }
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

    if (!desc->disabled)
    {
        if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq linux l1 isr line %s linux irq %u", b_os_irq_get_name(irq), b_os_irq_get_id(irq)));
        B_VIRTUAL_IRQ_WAKE_L1_LISTENERS();
        b_virtual_irq_inc_l1(desc);
    }
    else
    {
        BDBG_WRN(("virtual irq extra linux l1 isr line %s linux irq %u", b_os_irq_get_name(irq), b_os_irq_get_id(irq)));
    }
}

void b_virtual_irq_get_status(b_virtual_irq_status * status)
{
    unsigned i;
    b_virtual_irq_line irq_line;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }

    status->status_word = 0;
    for(i = 0; i < sizeof(status->lines)/sizeof(status->lines[0]); i++) {
        irq_line = status->lines[i];
        if (irq_line > b_virtual_irq_line_unused && irq_line < b_virtual_irq_line_max) {
            if (g_virtual_irq_state.lines[irq_line].fired) {
                status->status_word |= 1<<i;
            }
        }
    }
    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq status %#x", status->status_word));
}

int b_virtual_irq_clear(b_virtual_irq_line line)
{
    int result = 0;
    unsigned long flags;
    struct b_virtual_irq_line_desc * desc;

    if (line == b_virtual_irq_line_unused || line >= b_virtual_irq_line_max) { result = -EINVAL; BERR_TRACE(result); goto end; }
    if (!g_virtual_irq_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }
    desc = &g_virtual_irq_state.lines[line];
    if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq clear %s", b_os_irq_get_name(&desc->base)));
    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    desc->fired = false;
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

end:
    return result;
}

int b_virtual_irq_set_mask(b_virtual_irq_line line, bool disabled)
{
    int result = 0;
    unsigned long flags;
    struct b_virtual_irq_line_desc * desc;

    if (line == b_virtual_irq_line_unused || line >= b_virtual_irq_line_max) { result = -EINVAL; BERR_TRACE(result); goto end; }
    if (!g_virtual_irq_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }
    desc = &g_virtual_irq_state.lines[line];
    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    desc->disabled = disabled;
    b_os_irq_set_enable_spinlocked_tagged(&desc->base, !disabled, "virq set mask", __LINE__);
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

end:
    return result;
}

void b_virtual_irq_reenable_irqs_spinlocked(void)
{
    unsigned i;
    struct b_virtual_irq_line_desc * desc;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for(i = 0; i < sizeof(g_virtual_irq_state.lines)/sizeof(g_virtual_irq_state.lines[0]); i++)
    {
        desc = &g_virtual_irq_state.lines[i];
        if (!desc->disabled)
        {
            b_os_irq_set_enable_spinlocked_tagged(&desc->base, true, "virq re-enable", __LINE__);
        }
    }
}

static bool b_virtual_irq_is_indirect(const b_virtual_irq_group * group, b_virtual_irq_line line)
{
    bool indirect = false;
    unsigned i;
    for (i = 0; i < sizeof(group->indirect) / sizeof(group->indirect[0]); i++)
    {
        if (group->indirect[i] == line)
        {
            indirect = true;
            break;
        }
    }
    return indirect;
}

int b_virtual_irq_make_group(const b_virtual_irq_group * group)
{
    unsigned i;
    int result = 0;
    unsigned long flags;
    struct b_virtual_irq_line_desc * desc;

    if (!g_virtual_irq_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    /* 1. Verify consistency of data  - interrupts should be unmapped */
    if (group->l1_shift >= B_VIRTUAL_IRQ_GET_L1_WORD_COUNT() * 32) {
        result = -EINVAL; BERR_TRACE(result); goto end;
    }
    for (i = 0; i < sizeof(group->lines)/sizeof(group->lines[0]); i++) {
        b_virtual_irq_line irq_line = group->lines[i];
        if (irq_line==b_virtual_irq_line_unused) {
            continue;
        }
        if (irq_line>=b_virtual_irq_line_max) {
            result = -EINVAL; BERR_TRACE(result); goto end;
        }
    }
    /* 2. Update g_virtual_irq */
    for (i = 0; i < sizeof(group->lines)/sizeof(group->lines[0]); i++) {
        int linux_irq=0;
        b_virtual_irq_line irq_line = group->lines[i];
        const char * name = NULL;
        if (irq_line == b_virtual_irq_line_unused) {
            continue;
        }
        desc = &g_virtual_irq_state.lines[irq_line];
        if (desc->base.acquired) {
            if (desc->group_id != group->l1_shift+1) {
                BDBG_ERR(("Can't remap irq %u from %u to %u", irq_line, desc->group_id, group->l1_shift+1));
                /* can't remap to another L1 */
                result = -EINVAL; BERR_TRACE(result); goto end;
            }
            continue;
        }
        name = b_virtual_irq_get_name(irq_line);
        if (!b_virtual_irq_is_indirect(group, irq_line))
        {
            linux_irq = b_virtual_irq_get_linux_irq(irq_line);
            if (linux_irq <= 0) {
                result = -EINVAL; BERR_TRACE(result); goto end;
            }
            B_VIRTUAL_IRQ_SPIN_LOCK(flags);
            desc->line = irq_line;
            desc->fired = false;
            desc->disabled = true;
            desc->group_id = group->l1_shift + 1;
            B_VIRTUAL_IRQ_UNMASK_L1(group->l1_shift);
            B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
            b_os_irq_init(&desc->base, linux_irq, IRQF_TRIGGER_HIGH, name, &b_virtual_irq_isr, (void *)desc);
            result = b_os_irq_acquire_tagged(&desc->base, "virq make group", __LINE__);
            if (result)
            {
                BDBG_ERR(("request_irq failed with %d, irq:'%s' linux:%u line:%u", result, name, linux_irq , irq_line));
                B_VIRTUAL_IRQ_SPIN_LOCK(flags);
                B_VIRTUAL_IRQ_MASK_L1(group->l1_shift);
                B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
                result = -EINVAL; BERR_TRACE(-EINVAL); goto end;
            }
            if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq make group l1 %u direct l2 %s linux irq %u", group->l1_shift, name, linux_irq));
        }
        else
        {
            /* indirect virtual l1's still need a group id and unmasked l1 */
            if (g_brcm_virq_verbose) BDBG_LOG(("virtual irq make group l1 %u indirect l2 %s", group->l1_shift, name));
            B_VIRTUAL_IRQ_SPIN_LOCK(flags);
            desc->line = b_virtual_irq_line_max;
            desc->disabled = false; /* indirect L2 must start enabled */
            desc->group_id = group->l1_shift + 1;
            B_VIRTUAL_IRQ_UNMASK_L1(group->l1_shift);
            B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
            b_os_irq_init(&desc->base, 0, IRQF_TRIGGER_HIGH, name, &b_virtual_irq_isr, (void *)desc);
        }
    }

end:
    return result;
}

bool b_virtual_irq_is_os_managed(b_virtual_irq_line line)
{
    bool os_managed = false;

    if (line > b_virtual_irq_line_unused && line < b_virtual_irq_line_max) {
        int linux_irq = b_virtual_irq_get_linux_irq(line);
        if (linux_irq > 0) {
            os_managed = b_os_irq_is_os_managed(linux_irq);
            if (g_brcm_virq_verbose) BDBG_LOG(("%s +%s", os_managed?"linux":"nexus", b_virtual_irq_get_name(line)));
        }
    }

    return os_managed;
}
