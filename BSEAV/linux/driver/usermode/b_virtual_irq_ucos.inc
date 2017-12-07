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
static int g_brcm_irq0_verbose = 0; /* bool */

struct b_virtual_irq_line_state {
    b_virtual_irq_line line;
    unsigned linux_irq;
    unsigned group_id; /* 0 - means invalid */
    int requested; /* bool */
    int disabled; /* bool */
    int linux_enabled; /* bool */
    int fired; /* bool */
};

static const struct b_virtual_irq_name_map_entry {
    b_virtual_irq_line irq_line;
    char name[5];
} g_virtual_irq_name_map[] = {
    {b_virtual_irq_line_iica, "iica"},
    {b_virtual_irq_line_iicb, "iicb"},
    {b_virtual_irq_line_iicc, "iicc"},
    {b_virtual_irq_line_iicd, "iicd"},
    {b_virtual_irq_line_iice, "iice"},
    {b_virtual_irq_line_iicf, "iicf"},
    {b_virtual_irq_line_iicg, "iicg"},
    {b_virtual_irq_line_gio,  "gio"},
    {b_virtual_irq_line_gio_aon,  "gioa"},
    {b_virtual_irq_line_irb,  "irb"},
    {b_virtual_irq_line_icap, "icap"},
    {b_virtual_irq_line_kbd1, "kbd1"},
    {b_virtual_irq_line_kbd2, "kbd2"},
    {b_virtual_irq_line_kbd3, "kbd3"},
    {b_virtual_irq_line_ldk,  "ldk"},
    {b_virtual_irq_line_spi,  "spi"},
    {b_virtual_irq_line_ua,   "ua"},
    {b_virtual_irq_line_ub,   "ub"},
    {b_virtual_irq_line_uc,   "uc"}
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
    {b_virtual_irq_line_uc,  brcmstb_l2_irq_uc}
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
#endif /* B_VIRTUAL_IRQ_OS_SUPPORT */

static struct b_virtual_irq_state
{
    b_virtual_irq_capabilities caps;
    struct b_virtual_irq_line_state irqs[b_virtual_irq_line_max];
    int initialized; /* bool */
} g_virtual_irq_state;

void b_virtual_irq_init_submodule(void)
{
    if (g_brcm_irq0_verbose) PINFO("virtual irq init submodule\n");

    memset(&g_virtual_irq_state, 0, sizeof(g_virtual_irq_state));
#if B_VIRTUAL_IRQ_OS_SUPPORT
    g_virtual_irq_state.caps.feature_supported = 1;
#else
    g_virtual_irq_state.caps.feature_supported = 0;
#endif
    g_virtual_irq_state.initialized = 1;
}

void b_virtual_irq_uninit_submodule(void)
{
    unsigned i = 0;
    if (g_brcm_irq0_verbose) PINFO("virtual irq uninit submodule\n");
    for (i = 0; i < sizeof(g_virtual_irq_state.irqs)/sizeof(g_virtual_irq_state.irqs[0]); i++) {
        if (g_virtual_irq_state.irqs[i].requested) {
            if (g_brcm_irq0_verbose) PINFO("Freeing %s virtual irq (linux %d)\n", b_virtual_irq_get_name(i), g_virtual_irq_state.irqs[i].linux_irq);
            g_virtual_irq_state.irqs[i].requested = 0;
            free_irq(g_virtual_irq_state.irqs[i].linux_irq, (void*)&g_virtual_irq_state.irqs[i]);
        }
    }
    g_virtual_irq_state.initialized = 0;
}

void b_virtual_irq_get_capabilities(b_virtual_irq_capabilities * caps)
{
    if (!caps) { BERR_TRACE(-EINVAL); return; }
    memset(caps, 0, sizeof(*caps));
    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }
    *caps = g_virtual_irq_state.caps;
}

static const char * const g_virtual_irqs[] =
{
#if B_VIRTUAL_IRQ_OS_SUPPORT
"UPG_BSC",
"UPG_BSC_AON",
"UPG_MAIN",
"UPG_MAIN_AON",
"UPG_SPI"
#else
NULL
#endif
};

bool b_virtual_irq_l1_is_virtual(const char * name)
{
    unsigned i;
    const char *virtualIrq;
    bool virtual = false;

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return 0; }

    for (i = 0; i < sizeof(g_virtual_irqs)/sizeof(g_virtual_irqs[0]); i++) {
        virtualIrq = g_virtual_irqs[i];
        if (virtualIrq && !strcmp(virtualIrq, name)) {
            virtual = true;
            break;
        }
    }
    return virtual;
}

static void b_virtual_irq_set_l1_status(b_virtual_irq_line line)
{
    unsigned virtual_l1 = g_virtual_irq_state.irqs[line].group_id - 1;

    if (!g_virtual_irq_state.irqs[line].group_id) { BERR_TRACE(-EINVAL); return; }

    B_VIRTUAL_IRQ_SET_L1_STATUS(virtual_l1);
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
        PERR("Can't map Linux L1 for %u(%d,%u,'%s')\n", line,linux_irq,l2,b_virtual_irq_get_name(line));
        BERR_TRACE(-EINVAL);
    }
#endif

    return linux_irq;
}

static void b_virtual_irq_inc_l1(b_virtual_irq_line line)
{
    unsigned virtual_l1 = g_virtual_irq_state.irqs[line].group_id - 1;

    if (!g_virtual_irq_state.irqs[line].group_id) { BERR_TRACE(-EINVAL); return; }

    B_VIRTUAL_IRQ_INC_L1(virtual_l1);
}

void b_virtual_irq_software_l2_isr(b_virtual_irq_line line)
{
    unsigned long flags;

    if (g_brcm_irq0_verbose) PINFO("virtual irq software l2 isr: %s\n", b_virtual_irq_get_name(line));

    if (!g_virtual_irq_state.initialized) { BERR_TRACE(-EINVAL); return; }

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    g_virtual_irq_state.irqs[line].fired = 1;
    b_virtual_irq_set_l1_status(line);
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

    B_VIRTUAL_IRQ_WAKE_L1_LISTENERS();

    b_virtual_irq_inc_l1(line);
}

//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
//static irqreturn_t b_virtual_irq_linux_l1_isr(int irq, void *dev_id)
//#else
static int /*irqreturn_t*/ b_virtual_irq_linux_l1_isr(int irq, void *dev_id, struct pt_regs *regs)
//#endif
{
#ifdef FIXME
    struct b_virtual_irq_line_state *line_state = dev_id;
    b_virtual_irq_line irq_line = line_state->line;
    unsigned long flags;

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    disable_irq_nosync(irq);
    g_virtual_irq_state.irqs[irq_line].linux_enabled = 0;
    if (!g_virtual_irq_state.irqs[irq_line].disabled)
    {
        g_virtual_irq_state.irqs[irq_line].fired = 1;
        b_virtual_irq_set_l1_status(irq_line);
    }
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

    if (!g_virtual_irq_state.irqs[irq_line].disabled)
    {
        if (g_brcm_irq0_verbose) PINFO("virtual irq linux l1 isr line %s linux irq %u\n", b_virtual_irq_get_name(irq_line), irq);
        B_VIRTUAL_IRQ_WAKE_L1_LISTENERS();
        b_virtual_irq_inc_l1(irq_line);
    }
    else
    {
        PWARN("virtual irq extra linux l1 isr line %s linux irq %u\n", b_virtual_irq_get_name(irq_line), irq);
    }

    return IRQ_HANDLED;
#else
    printf("b_virtual_irq_linux_l1_isr stubbed\n");
    return 0;
#endif
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
            if (g_virtual_irq_state.irqs[irq_line].fired) {
                status->status_word |= 1<<i;
            }
        }
    }
    if (g_brcm_irq0_verbose) PINFO("virtual irq status %#x\n", status->status_word);
}

int b_virtual_irq_clear(b_virtual_irq_line line)
{
    int result = 0;
    unsigned long flags;

    if (g_brcm_irq0_verbose) PINFO("virtual irq clear %s\n", b_virtual_irq_get_name(line));

    if (!g_virtual_irq_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    if (line == b_virtual_irq_line_unused || line >= b_virtual_irq_line_max) { result = -EINVAL; BERR_TRACE(result); goto end; }

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    g_virtual_irq_state.irqs[line].fired = 0;
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

end:
    return result;
}

int b_virtual_irq_set_mask(b_virtual_irq_line line, bool disabled)
{
#ifdef FIXME
    int result = 0;
    unsigned long flags;

    if (!g_virtual_irq_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    if (line == b_virtual_irq_line_unused || line >= b_virtual_irq_line_max) { result = -EINVAL; BERR_TRACE(result); goto end; }

    B_VIRTUAL_IRQ_SPIN_LOCK(flags);
    g_virtual_irq_state.irqs[line].disabled = disable;
    if (disable)
    {
        if (g_virtual_irq_state.irqs[line].linux_enabled)
        {
            if (g_brcm_irq0_verbose) PINFO("virtual irq linux l1 int disable %s %u\n", b_virtual_irq_get_name(line), g_virtual_irq_state.irqs[line].linux_irq);
            disable_irq_nosync(g_virtual_irq_state.irqs[line].linux_irq);
            g_virtual_irq_state.irqs[line].linux_enabled = 0;
        }
    }
    else
    {
        if (!g_virtual_irq_state.irqs[line].linux_enabled && g_virtual_irq_state.irqs[line].requested)
        {
            if (g_brcm_irq0_verbose) PINFO("virtual irq linux l1 int enable %s %u\n", b_virtual_irq_get_name(line), g_virtual_irq_state.irqs[line].linux_irq);
            enable_irq(g_virtual_irq_state.irqs[line].linux_irq);
            g_virtual_irq_state.irqs[line].linux_enabled = 1;
        }
    }
    B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);

end:
    return result;
#else
    printf("b_virtual_irq_set_mask stubbed\n");
    return 0;
#endif
}

static int b_virtual_irq_is_indirect(const b_virtual_irq_group * group, b_virtual_irq_line line)
{
    int indirect = 0;
    unsigned i;
    for (i = 0; i < sizeof(group->indirect) / sizeof(group->indirect[0]); i++)
    {
        if (group->indirect[i] == line)
        {
            indirect = 1;
            break;
        }
    }
    return indirect;
}

int b_virtual_irq_make_group(const b_virtual_irq_group * group)
{
#ifdef FIXME
    unsigned i;
    int result = 0;
    unsigned long flags;

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
        if (g_virtual_irq_state.irqs[irq_line].requested) {
            if (g_virtual_irq_state.irqs[irq_line].group_id != group->l1_shift+1) {
                PERR("Can't remap irq %u from %u to %u\n", irq_line, g_virtual_irq_state.irqs[irq_line].group_id, group->l1_shift+1);
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
            g_virtual_irq_state.irqs[irq_line].line = irq_line;
            g_virtual_irq_state.irqs[irq_line].linux_irq = linux_irq;
            g_virtual_irq_state.irqs[irq_line].fired = 0;
            g_virtual_irq_state.irqs[irq_line].disabled = 1;
            g_virtual_irq_state.irqs[irq_line].group_id = group->l1_shift + 1;
            B_VIRTUAL_IRQ_UNMASK_L1(group->l1_shift);
            g_virtual_irq_state.irqs[irq_line].requested = 1;
            g_virtual_irq_state.irqs[irq_line].linux_enabled = 1;
            B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
            /* request_irq can block, do not include in spinlock */
            result = request_irq(linux_irq, b_virtual_irq_linux_l1_isr, IRQF_TRIGGER_HIGH, name, (void*)&g_virtual_irq_state.irqs[irq_line]);
            if (result)
            {
                PERR("request_irq failed with %d, irq:'%s' linux:%u line:%u\n", result, name, linux_irq , irq_line);
                B_VIRTUAL_IRQ_SPIN_LOCK(flags);
                B_VIRTUAL_IRQ_MASK_L1(group->l1_shift);
                g_virtual_irq_state.irqs[irq_line].requested = 1;
                g_virtual_irq_state.irqs[irq_line].linux_enabled = 1;
                B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
                result = -EINVAL; BERR_TRACE(-EINVAL); goto end;
            }
            if (g_brcm_irq0_verbose) PINFO("virtual irq make group l1 %u direct l2 %s linux irq %u\n", group->l1_shift, name, linux_irq);
        }
        else
        {
            /* indirect virtual l1's still need a group id and unmasked l1 */
            if (g_brcm_irq0_verbose) PINFO("virtual irq make group l1 %u indirect l2 %s\n", group->l1_shift, name);
            B_VIRTUAL_IRQ_SPIN_LOCK(flags);
            g_virtual_irq_state.irqs[irq_line].line = b_virtual_irq_line_max;
            g_virtual_irq_state.irqs[irq_line].requested = 0;
            g_virtual_irq_state.irqs[irq_line].linux_enabled = 0;
            g_virtual_irq_state.irqs[irq_line].linux_irq = 0;
            g_virtual_irq_state.irqs[irq_line].disabled = 0; /* indirect L2 must start enabled */
            g_virtual_irq_state.irqs[irq_line].group_id = group->l1_shift + 1;
            B_VIRTUAL_IRQ_UNMASK_L1(group->l1_shift);
            B_VIRTUAL_IRQ_SPIN_UNLOCK(flags);
        }
        if (result != 0) { goto end; }
    }

end:
    return result;
#else
    printf("b_virtual_irq_make_group stubbed\n");
    return 0;
#endif
}
