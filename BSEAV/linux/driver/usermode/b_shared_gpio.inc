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
#include "b_os_irq.h"
#define B_SHARED_GPIO_SET_INT(BANK, PIN) ((BANK)->irq_status |= 1 << (PIN)->shift)
#define B_SHARED_GPIO_CLEAR_INT(BANK, PIN) ((BANK)->irq_status &= ~(1 << (PIN)->shift))
#define B_SHARED_GPIO_MASK_INT(BANK, PIN) ((BANK)->irq_mask |= 1 << (PIN)->shift)
#define B_SHARED_GPIO_UNMASK_INT(BANK, PIN) ((BANK)->irq_mask &= ~(1 << (PIN)->shift))
#define B_SHARED_GPIO_GET_INT_STATUS(BANK) ((BANK)->irq_status & ~((BANK)->irq_mask))

static bool g_brcm_gpio_verbose = false;

struct b_shared_gpio_bank;
struct b_shared_gpio_pin {
    struct b_shared_gpio_bank * bank; /* if NULL, invalid */
    unsigned shift;
    struct
    {
        struct b_os_irq_desc base;
        b_shared_gpio_irq_type type;
        bool disabled; /* bool */
    } irq;
};

struct b_shared_gpio_bank {
    uint32_t base_address;
    bool aon;
    uint32_t irq_status;
    uint32_t irq_mask;
    struct b_shared_gpio_pin pins[32];
};

static struct b_shared_gpio {
    b_shared_gpio_module_init_settings settings;
    b_shared_gpio_capabilities caps;
    struct b_shared_gpio_bank banks[B_SHARED_GPIO_MAX_BANKS];
    bool initialized;
} g_shared_gpio_state;

#if B_SHARED_GPIO_OS_SUPPORT >= 5
#define b_shared_gpio_get_feature_support() b_shared_gpio_check_linux_managed_gpio_irqs()
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t b_shared_gpio_check_linux_l1_isr(int irq, void *dev_id)
#else
static irqreturn_t b_shared_gpio_check_linux_l1_isr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    disable_irq_nosync(irq);
    return IRQ_HANDLED;
}

static bool b_shared_gpio_check_linux_managed_gpio_irqs(void)
{
    bool linux_managed = false;
    void * gio = (void *)&gio;
    void * gio_aon = (void *)&gio_aon;

    /*
     * The shared gpio submodule checks runtime (DT) support by trying to
     * acquire the gpio and aon gpio l2s. This is because we don't want
     * to create a dependency on RDB or DT here to get the bank base address
     * in order to request a gpio irq from linux.
     */

    if (g_shared_gpio_state.settings.gio_linux_irq && g_shared_gpio_state.settings.gio_aon_linux_irq)
    {
        if (request_irq(g_shared_gpio_state.settings.gio_linux_irq, b_shared_gpio_check_linux_l1_isr, IRQF_TRIGGER_HIGH, "nexus_gpio", &gio) == -EINVAL)
        {
            if (g_brcm_gpio_verbose) PINFO("linux +gpio\n");
            linux_managed = true;
        }
        else
        {
            if (g_brcm_gpio_verbose) PINFO("nexus +gpio\n");
            free_irq(g_shared_gpio_state.settings.gio_linux_irq, &gio);
            if (request_irq(g_shared_gpio_state.settings.gio_aon_linux_irq, b_shared_gpio_check_linux_l1_isr, IRQF_TRIGGER_HIGH, "nexus_gpio", &gio_aon) == -EINVAL)
            {
                if (g_brcm_gpio_verbose) PINFO("linux +aon.gpio\n");
                linux_managed = true;
            }
            else
            {
                if (g_brcm_gpio_verbose) PINFO("nexus +aon.gpio\n");
                free_irq(g_shared_gpio_state.settings.gio_aon_linux_irq, &gio_aon);
            }
        }
    }
    else
    {
        if (g_brcm_gpio_verbose) PINFO("linux -virq nexus +gpio +aon.gpio\n");
        linux_managed = false;
    }

    PINFO("shared gpio banks: %s", linux_managed ? "enabled" : "disabled");

    return linux_managed;
}
#else /* B_SHARED_GPIO_OS_SUPPORT < 5 */
#define brcmstb_gpio_irq(base_address, shift) -ENOSYS
#define b_shared_gpio_get_feature_support() false
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
    g_shared_gpio_state.initialized = true;
}

static void b_shared_gpio_p_close_pin(struct b_shared_gpio_pin * pin)
{
    unsigned long flags;

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (g_brcm_gpio_verbose) PINFO("gpio %#x:%u free linux irq %u\n", pin->bank->base_address, pin->shift, b_os_irq_get_id(&pin->irq.base));
    B_SHARED_GPIO_MASK_INT(pin->bank, pin);
    B_SHARED_GPIO_CLEAR_INT(pin->bank, pin);
    pin->bank = NULL;
    B_SHARED_GPIO_SPIN_UNLOCK(flags);
    b_os_irq_uninit_tagged(&pin->irq.base, "shgio close pin", __LINE__);
}

void b_shared_gpio_uninit_submodule(void)
{
    if (g_brcm_gpio_verbose) PINFO("gpio uninit submodule\n");
    b_shared_gpio_close_submodule();
    g_shared_gpio_state.initialized = 0;
}

static int b_shared_gpio_close_comatose_pin(struct b_shared_gpio_pin * pin)
{
    bool wakeable = false;

    if (!pin->irq.base.wakeable)
    {
        b_shared_gpio_p_close_pin(pin);
    }
    else
    {
        PINFO("leaving pin %#x:%u open because it is a wake source\n", pin->bank->base_address, pin->shift);
        wakeable = true;
    }

    return wakeable ? 1 : 0;
}

static int b_shared_gpio_close_any_pin(struct b_shared_gpio_pin * pin)
{
    b_shared_gpio_p_close_pin(pin);
    return 0;
}

static bool b_shared_gpio_visit_all_open_pins(int (*pin_visitor)(struct b_shared_gpio_pin *))
{
    bool wakeable = false;
    unsigned b;
    unsigned p;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;

    for (b = 0; b < B_SHARED_GPIO_MAX_BANKS; b++)
    {
        bank = &g_shared_gpio_state.banks[b];
        for (p = 0; p < 32; p++)
        {
            pin = &bank->pins[p];
            if (pin->bank) /* means this pin is open */
            {
                wakeable = wakeable || ((pin_visitor(pin) == 1) ? true : false);
            }
        }
    }

    return wakeable;
}

void b_shared_gpio_close_submodule(void)
{
    bool hasWakeable = false;

    if (g_brcm_gpio_verbose) PINFO("gpio close submodule\n");

    /*
     * this will check if there are any wake sources among the open pins and return 1 if so.
     * important: this will leave wakeable pins and their interrupt resources allocated in the kernel
     * which will mean no one else can use them.  If you want to really free them, make sure that
     * you are not in standby mode when closing the driver.
     */
    hasWakeable = b_shared_gpio_visit_all_open_pins(&b_shared_gpio_close_comatose_pin);
    if (!hasWakeable)
    {
        /* we can free back to the os if none are wake sources */
        brcmstb_gpio_remove();
    }
}

int b_shared_gpio_open_submodule(void)
{
    if (g_brcm_gpio_verbose) PINFO("gpio open submodule\n");

    /* this will close any open pin we still have open after last open/close sequence to driver */
    (void)b_shared_gpio_visit_all_open_pins(&b_shared_gpio_close_any_pin);
    /* blindly free back to the os all pins we might have taken last time we opened */
    brcmstb_gpio_remove();

    return 0;
}

void b_shared_gpio_get_capabilities(b_shared_gpio_capabilities * caps)
{
    if (!caps) { BERR_TRACE(-EINVAL); return; }
    memset(caps, 0, sizeof(*caps));
    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }
    *caps = g_shared_gpio_state.caps;
}

static void b_shared_gpio_isr(struct b_os_irq_desc * irq, void * context)
{
    struct b_shared_gpio_pin * pin = context;
    unsigned long flags;

    if (g_brcm_gpio_verbose) PINFO("gpio isr %#x:%u irq %u\n", pin->bank->base_address, pin->shift, b_os_irq_get_id(irq));

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (!pin->irq.disabled)
    {
        B_SHARED_GPIO_SET_INT(pin->bank, pin);
    }
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

    if (B_SHARED_GPIO_GET_INT_STATUS(pin->bank))
    {
        /* if we have an unmasked L3 firing -> fake L2 */
        B_SHARED_GPIO_FIRE_GPIO_L2(pin->bank->aon);
    }
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
    struct b_shared_gpio_bank * bank = NULL;

    if (!pin_desc->bank_base_address)
    {
        PERR("Invalid argument: NULL bank base address\n");
        BERR_TRACE(-EINVAL); goto end;
    }

    bank = b_shared_gpio_find_bank_by_base_address(pin_desc->bank_base_address);

end:
    return bank;
}

static struct b_shared_gpio_pin * b_shared_gpio_get_pin(struct b_shared_gpio_bank * bank, unsigned shift, bool check_open)
{
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
}

void b_shared_gpio_init_banks(const b_shared_gpio_init_banks_settings * settings)
{
    unsigned i;

    if (g_brcm_gpio_verbose) PINFO("gpio init banks\n");

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for (i = 0; i < sizeof(settings->bank_base_addresses)/sizeof(settings->bank_base_addresses[0]); i++)
    {
        memset(g_shared_gpio_state.banks + i, 0, sizeof(g_shared_gpio_state.banks[i]));
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
}

int b_shared_gpio_open_pin(const b_shared_gpio_pin_desc * pin_desc, b_shared_gpio_irq_type irq_type)
{
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    int linux_irq = 0;
    unsigned long flags;
    uint32_t bank_address;

    if (g_brcm_gpio_verbose) PINFO("gpio open pin %#x:%u:%s\n", pin_desc->bank_base_address, pin_desc->shift, b_shared_gpio_get_irq_type_name(irq_type));

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    if (irq_type == b_shared_gpio_irq_type_max) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, 0);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    memset(pin, 0, sizeof(*pin));
    pin->bank = bank;
    pin->shift = pin_desc->shift;
    pin->irq.type = irq_type;
    pin->irq.disabled = true; /* open_pin must end with the irq disabled/masked -> client must explicitly enable it */

    linux_irq = 0;
    B_SHARED_GPIO_SPIN_LOCK(flags);
    B_SHARED_GPIO_MASK_INT(bank, pin);
    B_SHARED_GPIO_CLEAR_INT(bank, pin);
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

    if (pin->irq.type != b_shared_gpio_irq_type_disabled)
    {
        bank_address = pin_desc->bank_base_address;

#if defined(CONFIG_ARM) && (BRCMSTB_H_VERSION == 8)
        /* This is a workaround for an issue inside of the 4.1-1.3 kernel release which fails                                                                                                                 * to handle address mappings for chips with BCHP_PHYSICAL_OFFSET of 0xD0000000. */
#if defined(BCHP_PHYSICAL_OFFSET) && defined (BCHP_CHIP) /* Kernel mode Nexus */
#if ((BCHP_CHIP == 7260) || (BCHP_CHIP == 7268) || (BCHP_CHIP == 7271))
        bank_address &= ~0x20000000;
#endif /* BCHP_CHIP */
#else /* bcmdriver */
        if (g_breg.bchp_physical_offset == 0xD0000000)
        {
            bank_address &= ~0x20000000;
        }
#endif /* BCHP_PHYSICAL_OFFSET */
#endif /* CONFIG_ARM */

        linux_irq = brcmstb_gpio_irq(bank_address, pin_desc->shift);
    }
    /* init struct even if irq is invalid */
    b_os_irq_init(&pin->irq.base, linux_irq, b_shared_gpio_irq_type_to_linux(irq_type), "nexus gpio", b_shared_gpio_isr, pin);
    /* but only request if valid */
    if (linux_irq > 0)
    {
        result = b_os_irq_acquire_tagged(&pin->irq.base, "shgio open pin", __LINE__);
        if (!result)
        {
            if (g_brcm_gpio_verbose) PINFO("gpio %x:%u linux irq %u (%s)\n", bank->base_address, pin->shift, linux_irq, b_shared_gpio_get_irq_type_name(irq_type));
        }
        else
        {
            PERR("request_irq failed with %d, irq:'%s' linux:%u gpio: %#x:%u\n", result, "nexus_gpio", linux_irq , bank->base_address, pin->shift);
            B_SHARED_GPIO_SPIN_LOCK(flags);
            pin->bank = NULL;
            pin->irq.type = b_shared_gpio_irq_type_disabled;
            B_SHARED_GPIO_SPIN_UNLOCK(flags);
            result = -EINVAL; BERR_TRACE(result);
        }
    }
    else
    {
        PERR("Error finding gpio %#x:%u linux L1\n", pin_desc->bank_base_address, pin_desc->shift);
        result = -EINVAL; BERR_TRACE(result);
    }

end:
    return result;
}

void b_shared_gpio_close_pin(const b_shared_gpio_pin_desc * pin_desc)
{
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;

    if (g_brcm_gpio_verbose) PINFO("gpio close pin %#x:%u\n", pin_desc->bank_base_address, pin_desc->shift);

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { BERR_TRACE(-EINVAL); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, true);
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

int b_shared_gpio_set_int_mask(const b_shared_gpio_pin_desc * pin_desc, bool disabled)
{
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, true);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    B_SHARED_GPIO_SPIN_LOCK(flags);
    if (disabled && !pin->irq.disabled)
    {
        if (g_brcm_gpio_verbose) PINFO("gpio int mask %#x:%u\n", bank->base_address, pin->shift);
        B_SHARED_GPIO_MASK_INT(bank, pin);
        b_os_irq_set_enable_spinlocked_tagged(&pin->irq.base, !disabled, "shgio set mask", __LINE__);
    }
    else if (!disabled && pin->irq.disabled)
    {
        if (g_brcm_gpio_verbose) PINFO("gpio int unmask %#x:%u\n", bank->base_address, pin->shift);
        B_SHARED_GPIO_UNMASK_INT(bank, pin);
        b_os_irq_set_enable_spinlocked_tagged(&pin->irq.base, !disabled, "shgio set mask", __LINE__);
    }
    pin->irq.disabled = disabled;
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
}

int b_shared_gpio_set_standby(const b_shared_gpio_pin_desc * pin_desc, bool enabled)
{
    int result = 0;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;
    unsigned long flags;

    if (!g_shared_gpio_state.initialized) { result = -EINVAL; BERR_TRACE(result); goto end; }

    bank = b_shared_gpio_get_bank(pin_desc);
    if (!bank) { result = -EINVAL; BERR_TRACE(result); goto end; }

    pin = b_shared_gpio_get_pin(bank, pin_desc->shift, true);
    if (!pin) { result = -EINVAL; BERR_TRACE(-result); goto end; }

    PINFO("gpio set_standby %#x:%u: linux_enabled = %d; en = %d; wake_src = %d\n",
        bank->base_address, pin->shift,
        b_os_irq_is_enabled(&pin->irq.base),
        enabled,
        b_os_irq_is_wakeable(&pin->irq.base));

    B_SHARED_GPIO_SPIN_LOCK(flags);
    b_os_irq_set_wake_capability_spinlocked_tagged(&pin->irq.base, enabled, "shgio set standby", __LINE__);
    B_SHARED_GPIO_SPIN_UNLOCK(flags);

end:
    return result;
}

void b_shared_gpio_reenable_irqs_spinlocked(void)
{
    unsigned b, p;
    struct b_shared_gpio_bank * bank;
    struct b_shared_gpio_pin * pin;

    if (!g_shared_gpio_state.initialized) { BERR_TRACE(-EINVAL); return; }

    for (b = 0; b < sizeof(g_shared_gpio_state.banks)/sizeof(g_shared_gpio_state.banks[0]); b++)
    {
        bank = &g_shared_gpio_state.banks[b];
        for (p = 0; p < sizeof(bank->pins)/sizeof(bank->pins[0]); p++)
        {
            pin = &bank->pins[p];
            if (!b_os_irq_get_id(&pin->irq.base) || pin->irq.disabled) continue;
            if (g_brcm_gpio_verbose) PINFO("gpio irq re-enable %#x:%u linux irq %u\n", bank->base_address, pin->shift, b_os_irq_get_id(&pin->irq.base));
            b_os_irq_set_enable_spinlocked_tagged(&pin->irq.base, true, "shgio re-enable", __LINE__);
        }
    }
}
