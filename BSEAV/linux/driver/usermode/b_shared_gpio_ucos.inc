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
 * B_SHARED_GPIO_FIRE_GPIO_L2() - fires a GPIO L2 interrupt
 * B_SHARED_GPIO_SPIN_LOCK(flags) - locks the interrupt mgmt spinlock
 * B_SHARED_GPIO_SPIN_UNLOCK(flags) - unlocks the interrupt mgmt spinlock
 */
#define B_SHARED_GPIO_SET_INT(BANK, PIN) ((BANK)->irq_status |= 1 << (PIN)->shift)
#define B_SHARED_GPIO_CLEAR_INT(BANK, PIN) ((BANK)->irq_status &= ~(1 << (PIN)->shift))
#define B_SHARED_GPIO_MASK_INT(BANK, PIN) ((BANK)->irq_mask |= 1 << (PIN)->shift)
#define B_SHARED_GPIO_UNMASK_INT(BANK, PIN) ((BANK)->irq_mask &= ~(1 << (PIN)->shift))
#define B_SHARED_GPIO_GET_INT_STATUS(BANK) ((BANK)->irq_status & ~((BANK)->irq_mask))

static int g_brcm_gpio_verbose = 0; /* bool */

struct b_shared_gpio_bank;
struct b_shared_gpio_pin {
    struct b_shared_gpio_bank * bank; /* if NULL, invalid */
    unsigned shift;
    struct
    {
        b_shared_gpio_irq_type type;
        unsigned linux_irq;
        int linux_enabled; /* bool */
        int disabled; /* bool */
        int wake_src; /* bool */
    } irq;
};

struct b_shared_gpio_bank {
    uint32_t base_address;
    int aon; /* bool */
    uint32_t irq_status;
    uint32_t irq_mask;
    struct b_shared_gpio_pin pins[32];
};

static struct b_shared_gpio {
    b_shared_gpio_module_init_settings settings;
    b_shared_gpio_capabilities caps;
    struct b_shared_gpio_bank banks[B_SHARED_GPIO_MAX_BANKS];
    int initialized; /* bool */
} g_shared_gpio_state;

#if B_SHARED_GPIO_OS_SUPPORT >= 5
#define b_shared_gpio_get_feature_support() b_shared_gpio_check_linux_managed_gpio_irqs()
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t b_shared_gpio_check_linux_l1_isr(int irq, void *dev_id)
#else
static irqreturn_t b_shared_gpio_check_linux_l1_isr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    unsigned long flags;
    B_SHARED_GPIO_SPIN_LOCK(flags);
    disable_irq(irq);
    B_SHARED_GPIO_SPIN_UNLOCK(flags);
    return IRQ_HANDLED;
}

static int b_shared_gpio_check_linux_managed_gpio_irqs(void)
{
#ifdef FIXME
    unsigned long flags;
    int linux_managed = 0; /* bool */
    void * gio = (void *)&gio;
    void * gio_aon = (void *)&gio_aon;

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (request_irq(g_shared_gpio_state.settings.gio_linux_irq, b_shared_gpio_check_linux_l1_isr, IRQF_TRIGGER_HIGH, "nexus_gpio", &gio) == -EINVAL)
    {
        linux_managed = 1;
    }
    else
    {
        free_irq(g_shared_gpio_state.settings.gio_linux_irq, &gio);
        if (request_irq(g_shared_gpio_state.settings.gio_aon_linux_irq, b_shared_gpio_check_linux_l1_isr, IRQF_TRIGGER_HIGH, "nexus_gpio", &gio_aon) == -EINVAL)
        {
            linux_managed = 1;
        }
        else
        {
            free_irq(g_shared_gpio_state.settings.gio_aon_linux_irq, &gio_aon);
        }
    }
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

    return linux_managed;
#else
    printf("b_shared_gpio_check_linux_managed_gpio_irqs stubbed\n");
    return 0;
#endif
}
#else /* B_SHARED_GPIO_OS_SUPPORT < 5 */
#define brcmstb_gpio_irq(base_address, shift) -ENOSYS
#define b_shared_gpio_get_feature_support() 0
#endif
#if B_SHARED_GPIO_OS_SUPPORT < 6
#define brcmstb_gpio_remove()
#endif

void b_shared_gpio_init_submodule(const b_shared_gpio_module_init_settings * settings)
{
    if (g_brcm_gpio_verbose) PINFO("gpio init submodule\n");

    memset(&g_shared_gpio_state, 0, sizeof(g_shared_gpio_state));
    g_shared_gpio_state.settings = *settings;
    g_shared_gpio_state.caps.feature_supported = b_shared_gpio_get_feature_support();
    g_shared_gpio_state.initialized = 1;
}

static void b_shared_gpio_p_close_pin(struct b_shared_gpio_pin * pin)
{
#ifdef FIXME
    unsigned long flags;

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (g_brcm_gpio_verbose) PINFO("gpio %#x:%u free linux irq %u\n", pin->bank->base_address, pin->shift, pin->irq.linux_irq);
    if (pin->irq.linux_irq)
    {
        free_irq(pin->irq.linux_irq, (void*)pin);
        pin->irq.linux_irq = 0;
    }
    B_SHARED_GPIO_MASK_INT(pin->bank, pin);
    B_SHARED_GPIO_CLEAR_INT(pin->bank, pin);
    pin->bank = NULL;
    B_SHARED_GPIO_SPIN_UNLOCK(flags);
#else
    printf("b_shared_gpio_p_close_pin stubbed\n");
#endif
}

void b_shared_gpio_uninit_submodule(void)
{
#ifdef FIXME
    unsigned b;
    unsigned p;
    struct b_shared_gpio_pin * pin;

    if (g_brcm_gpio_verbose) PINFO("gpio uninit submodule\n");

    for (b = 0; b < B_SHARED_GPIO_MAX_BANKS; b++)
    {
        for (p = 0; p < 32; p++)
        {
            pin = &g_shared_gpio_state.banks[b].pins[p];
            if (pin->bank)
            {
                b_shared_gpio_p_close_pin(pin);
            }
        }
    }
    brcmstb_gpio_remove();
    g_shared_gpio_state.initialized = 0;
#else
    printf("b_shared_gpio_uninit_submodule\n");
#endif
}

void b_shared_gpio_get_capabilities(b_shared_gpio_capabilities * caps)
{
    if (!caps) { BERR_TRACE(-EINVAL); return; }
    memset(caps, 0, sizeof(*caps));
    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }
    *caps = g_shared_gpio_state.caps;
}

//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static int b_shared_gpio_linux_l1_isr(int irq, void *dev_id)
//#else
//static irqreturn_t b_shared_gpio_linux_l1_isr(int irq, void *dev_id, struct pt_regs *regs)
//#endif
{
#ifdef FIXME
    struct b_shared_gpio_pin * pin = dev_id;
    unsigned long flags;

    if (g_brcm_gpio_verbose) PINFO("gpio linux l1 isr %#x:%u linux irq %u\n", pin->bank->base_address, pin->shift, irq);

    B_SHARED_GPIO_SPIN_LOCK(flags);
    disable_irq_nosync(irq);
    pin->irq.linux_enabled = 0;
    B_SHARED_GPIO_SET_INT(pin->bank, pin);
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

    if (B_SHARED_GPIO_GET_INT_STATUS(pin->bank))
    {
        /* if we have an unmasked L3 firing -> fake L2 */
        B_SHARED_GPIO_FIRE_GPIO_L2(pin->bank->aon);
    }

    return IRQ_HANDLED;
#else
    printf("b_shared_gpio_linux_l1_isr stubbed\n");
    return 0;
#endif
}

static struct b_shared_gpio_bank * b_shared_gpio_find_bank_by_base_address(uint32_t base_address)
{
    struct b_shared_gpio_bank * bank = NULL;
    unsigned i;

    for (i = 0; i < B_SHARED_GPIO_MAX_BANKS; i++)
    {
        if (g_shared_gpio_state.banks[i].base_address == base_address)
        {
            bank = &g_shared_gpio_state.banks[i];
            break;
        }
    }

    return bank;
}

static struct b_shared_gpio_bank * b_shared_gpio_get_bank(const b_shared_gpio_pin_desc * pin_desc)
{
#ifdef FIXME
    struct b_shared_gpio_bank * bank = NULL;

    if (!pin_desc->bank_base_address)
    {
        PERR("Invalid argument: NULL bank base address\n");
        BERR_TRACE(-EINVAL); goto end;
    }

    bank = b_shared_gpio_find_bank_by_base_address(pin_desc->bank_base_address);

end:
    return bank;
#else
    printf("b_shared_gpio_get_bank stubbed\n");
    return 0;
#endif
}

static struct b_shared_gpio_pin * b_shared_gpio_get_pin(struct b_shared_gpio_bank * bank, unsigned shift, int check_open)
{
#ifdef FIXME
    struct b_shared_gpio_pin * pin = NULL;

    if (shift >= 32)
    {
        PERR("Invalid argument: pin shift out of bounds %d\n", shift);
        BERR_TRACE(-EINVAL); goto end;
    }
    pin = &bank->pins[shift];
    if (check_open && !pin->bank)
    {
        pin = NULL;
        PERR("Pin not open: %#x %d\n", bank->base_address, shift);
        BERR_TRACE(-EINVAL); goto end;
    }

end:
    return pin;
#else
    printf("b_shared_gpio_get_pin stubbed\n");
    return 0;
#endif
}

void b_shared_gpio_init_banks(const b_shared_gpio_init_banks_settings * settings)
{
    unsigned i;

    if (g_brcm_gpio_verbose) PINFO("gpio init banks\n");

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for (i = 0; i < sizeof(settings->bank_base_addresses)/sizeof(settings->bank_base_addresses[0]); i++)
    {
        g_shared_gpio_state.banks[i].base_address = settings->bank_base_addresses[i];
        g_shared_gpio_state.banks[i].aon = settings->bank_is_aon[i];
    }
}

static const char * b_shared_gpio_get_irq_type_name(b_shared_gpio_irq_type irq_type)
{
    static const char * irq_type_names[] =
    {
        "disabled",
        "rising edge",
        "falling edge",
        "both edges",
        "low level",
        "high level",
        NULL
    };

    return irq_type_names[irq_type];
}

static int b_shared_gpio_irq_type_to_linux(b_shared_gpio_irq_type irq_type)
{
#ifdef FIXME
    int linux_irq_type = IRQF_TRIGGER_NONE;

    switch (irq_type)
    {
        case b_shared_gpio_irq_type_rising_edge:    /* Interrupt on a 0->1 transition */
            linux_irq_type = IRQF_TRIGGER_RISING;
            break;
        case b_shared_gpio_irq_type_falling_edge:   /* Interrupt on a 1->0 transition */
            linux_irq_type = IRQF_TRIGGER_FALLING;
            break;
        case b_shared_gpio_irq_type_edge:          /* Interrupt on both a 0->1 and a 1->0 transition */
            linux_irq_type = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
            break;
        case b_shared_gpio_irq_type_low:           /* Interrupt on a 0 value */
            linux_irq_type = IRQF_TRIGGER_LOW;
            break;
        case b_shared_gpio_irq_type_high:          /* Interrupt on a 1 value */
            linux_irq_type = IRQF_TRIGGER_HIGH;
            break;
        default:
            break;
    }

    return linux_irq_type;
#else
    printf("b_shared_gpio_irq_type_to_linux stubbed\n");
    return 0;
#endif
}

int b_shared_gpio_open_pin(const b_shared_gpio_pin_desc * pin_desc, b_shared_gpio_irq_type irq_type)
{
#ifdef FIXME
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    int linux_irq = 0;
    unsigned long flags;

    if (g_brcm_gpio_verbose) PINFO("gpio open pin %#x:%u:%s\n", pin_desc->bank_base_address, pin_desc->shift, b_shared_gpio_get_irq_type_name(irq_type));

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    if (irq_type == b_shared_gpio_irq_type_max) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, 0);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    pin->irq.type = irq_type;

    if (pin->irq.type == b_shared_gpio_irq_type_disabled)
    {
        pin->bank = bank;
        pin->shift = pin_desc->shift;
        pin->irq.linux_irq = 0;
        pin->irq.linux_enabled = 0;
        B_SHARED_GPIO_MASK_INT(bank, pin);
        B_SHARED_GPIO_CLEAR_INT(bank, pin);
        goto end;
    }

    linux_irq = brcmstb_gpio_irq(pin_desc->bank_base_address, pin_desc->shift);
    if (linux_irq <= 0)
    {
        PERR("Error finding gpio %#x:%u linux L1\n", pin_desc->bank_base_address, pin_desc->shift);
        result = -EINVAL; BERR_TRACE(result); goto end;
    }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    pin->bank = bank;
    pin->shift = pin_desc->shift;
    pin->irq.linux_irq = linux_irq;
    pin->irq.linux_enabled = 1;
    B_SHARED_GPIO_UNMASK_INT(bank, pin);
    B_SHARED_GPIO_CLEAR_INT(bank, pin);
    result = request_irq(linux_irq, b_shared_gpio_linux_l1_isr, b_shared_gpio_irq_type_to_linux(pin->irq.type), "nexus_gpio", (void*)pin);
    if (!result)
    {
        if (g_brcm_gpio_verbose) PINFO("gpio %x:%u linux irq %u (%s)\n", bank->base_address, pin->shift, linux_irq, b_shared_gpio_get_irq_type_name(irq_type));
    }
    else
    {
        PERR("request_irq failed with %d, irq:'%s' linux:%u gpio: %#x:%u\n", result, "nexus_gpio", linux_irq , bank->base_address, pin->shift);
        pin->bank = NULL;
        pin->irq.linux_irq = 0;
        pin->irq.linux_enabled = 0;
        pin->irq.type = b_shared_gpio_irq_type_disabled;
        B_SHARED_GPIO_MASK_INT(bank, pin);
        result = -EINVAL; BERR_TRACE(result);
    }
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
#else
    printf("b_shared_gpio_open_pin stubbed\n");
    return 0;
#endif
}

void b_shared_gpio_close_pin(const b_shared_gpio_pin_desc * pin_desc)
{
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;

    if (g_brcm_gpio_verbose) PINFO("gpio close pin %#x:%u\n", pin_desc->bank_base_address, pin_desc->shift);

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { BERR_TRACE(-EINVAL); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, 1);
    if (!pin) { BERR_TRACE(-EINVAL); goto end; }

    b_shared_gpio_p_close_pin(pin);

end:
    return;
}

void b_shared_gpio_get_int_status(b_shared_gpio_int_status * status)
{
    unsigned i;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    for (i = 0; i < sizeof(status->bank_status)/sizeof(status->bank_status[0]); i++)
    {
        status->bank_status[i] = B_SHARED_GPIO_GET_INT_STATUS(&g_shared_gpio_state.banks[i]);
        if (g_brcm_gpio_verbose) PINFO("gpio status bank %u : %#x\n", i, status->bank_status[i]);
    }
    B_SHARED_GPIO_SPIN_UNLOCK(flags);
}

int b_shared_gpio_clear_int(const b_shared_gpio_pin_desc * pin_desc)
{
    int result = 0;
    struct b_shared_gpio_bank * bank;
    unsigned long flags;

    if (g_brcm_gpio_verbose) PINFO("gpio clear int %#x:%u\n", pin_desc->bank_base_address, pin_desc->shift);

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    B_SHARED_GPIO_CLEAR_INT(bank, pin_desc);
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
}

int b_shared_gpio_set_int_mask(const b_shared_gpio_pin_desc * pin_desc, int disable)
{
#ifdef FIXME
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, 1);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (disable && !pin->irq.disabled)
    {
        if (g_brcm_gpio_verbose) PINFO("gpio int mask %#x:%u\n", bank->base_address, pin->shift);
        B_SHARED_GPIO_MASK_INT(bank, pin);
        if (pin->irq.linux_enabled)
        {
            disable_irq_nosync(pin->irq.linux_irq);
            pin->irq.linux_enabled = 0;
        }
    }
    else if (!disable && pin->irq.disabled)
    {
        if (g_brcm_gpio_verbose) PINFO("gpio int unmask %#x:%u\n", bank->base_address, pin->shift);
        B_SHARED_GPIO_UNMASK_INT(bank, pin);
        if (!pin->irq.linux_enabled)
        {
            enable_irq(pin->irq.linux_irq);
            pin->irq.linux_enabled = 1;
        }
    }
    pin->irq.disabled = disable;
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
#else
    printf("b_shared_gpio_set_int_mask stubbed\n");
    return 0;
#endif
}

int b_shared_gpio_set_standby(const b_shared_gpio_pin_desc * pin_desc, int enable)
{
#ifdef FIXME
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, 1);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (pin->irq.linux_enabled)
    {
        if (enable && !pin->irq.wake_src)
        {
            if (g_brcm_gpio_verbose) PINFO("gpio suspend %#x:%u\n", bank->base_address, pin->shift);
            result = enable_irq_wake(pin->irq.linux_irq);
            if (!result)
            {
                pin->irq.wake_src = 1;
            }
            else
            {
                PWARN("Failed to mark gpio %#x:%u as wakeup source, will not wake from this gpio\n", bank->base_address, pin->shift);
                PWARN("If this message occurs for a non-AON gpio for S2 you need to upgrade BOLT\n");
            }
        }
        else if (!enable && pin->irq.wake_src)
        {
            if (g_brcm_gpio_verbose) PINFO("gpio resume %#x:%u\n", bank->base_address, pin->shift);
            result = disable_irq_wake(pin->irq.linux_irq);
            if (!result)
            {
                pin->irq.wake_src = 0;
            }
        }
    }
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
#else
    printf("b_shared_gpio_set_standby stubbed\n");
    return 0;
#endif
}
