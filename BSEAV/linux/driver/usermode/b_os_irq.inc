/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "b_os_irq.h"

static int g_brcm_os_irq_verbose = 0; /* bool */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t b_os_irq_linux_handler_isr(int irq, void *dev_id)
#else
static irqreturn_t b_os_irq_linux_handler_isr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    struct b_os_irq_desc * desc = dev_id;
    unsigned long flags;

    if (g_brcm_os_irq_verbose) BDBG_LOG(("linux l1 isr %s irq %u", desc->name, desc->id));

    B_OS_IRQ_SPIN_LOCK(flags);
    b_os_irq_set_enable_spinlocked_tagged(desc, 0, "isr", __LINE__);
    B_OS_IRQ_SPIN_UNLOCK(flags);
    if (desc->handler.handle_isr)
    {
        desc->handler.handle_isr(desc, desc->handler.context);
    }

    return IRQ_HANDLED;
}

static void b_os_irq_init(struct b_os_irq_desc * irq, int id, int type, const char * name, b_os_irq_handler handle_isr, void * context)
{
    memset(irq, 0, sizeof(*irq));
    irq->id = id;
    irq->type = type;
    snprintf(irq->name, 16, "%s", name);
    irq->handler.handle_isr = handle_isr;
    irq->handler.context = context;
}

static void b_os_irq_uninit_tagged(struct b_os_irq_desc * irq, const char * func, unsigned line)
{
    unsigned long flags;
    B_OS_IRQ_SPIN_LOCK(flags);
    b_os_irq_set_wake_capability_spinlocked_tagged(irq, 0, func, line);
    b_os_irq_set_enable_spinlocked_tagged(irq, 0, func, line);
    B_OS_IRQ_SPIN_UNLOCK(flags);
    b_os_irq_release_tagged(irq, func, line);
}

static int b_os_irq_get_id(struct b_os_irq_desc * irq)
{
    int rc;
    if (!irq) { rc = -EINVAL; BERR_TRACE(rc); return rc; }
    return irq->id;
}

static const char * b_os_irq_get_name(struct b_os_irq_desc * irq)
{
    if (!irq) { BERR_TRACE(-EINVAL); return NULL; }
    return irq->name;
}

static bool b_os_irq_is_enabled(struct b_os_irq_desc * irq)
{
    if (!irq) { BERR_TRACE(-EINVAL); return false; }
    return irq->enabled;
}

static bool b_os_irq_is_wakeable(struct b_os_irq_desc * irq)
{
    if (!irq) { BERR_TRACE(-EINVAL); return false; }
    return irq->wakeable;
}

static void b_os_irq_p_set_acquire(struct b_os_irq_desc * irq, bool acquired)
{
    unsigned long flags;
    B_OS_IRQ_SPIN_LOCK(flags);
    irq->acquired = acquired;
    irq->enabled = acquired;
    B_OS_IRQ_SPIN_UNLOCK(flags);
}

static int b_os_irq_acquire_tagged(struct b_os_irq_desc * irq, const char * func, unsigned line)
{
    int result = 0;

    b_os_irq_p_set_acquire(irq, 1);
    result = request_irq(irq->id, b_os_irq_linux_handler_isr, irq->type, irq->name, (void*)irq);
    if (!result)
    {
        if (g_brcm_os_irq_verbose) BDBG_LOG(("%s:%d: linux irq %s (%u) acquired", func, line, irq->name, irq->id));
    }
    else
    {
        BDBG_ERR(("%s:%d: request_irq failed with %d, irq:'%s' id:%u", func, line, result, irq->name, irq->id));
        b_os_irq_p_set_acquire(irq, 0);
        result = -EINVAL; BERR_TRACE(result);
    }

    return result;
}

static void b_os_irq_release_tagged(struct b_os_irq_desc * irq, const char * func, unsigned line)
{
    if (g_brcm_os_irq_verbose) BDBG_LOG(("%s:%d: free linux irq %s (%u)", func, line, irq->name, irq->id));
    if (irq->id)
    {
        if (irq->acquired)
        {
            free_irq(irq->id, (void*)irq);
        }
        b_os_irq_p_set_acquire(irq, 0);
    }
}

static void b_os_irq_set_enable_spinlocked_tagged(struct b_os_irq_desc * irq, bool enabled, const char * func, unsigned line)
{
    if (!irq->id || !irq->acquired) return;
    if (enabled != irq->enabled)
    {
        if (g_brcm_os_irq_verbose) BDBG_LOG(("%s:%d: os irq %u %s", func, line, irq->id, enabled ? "enabled" : "disabled"));
        if (enabled)
        {
            enable_irq(irq->id);
        }
        else
        {
            disable_irq_nosync(irq->id);
        }
        irq->enabled = enabled;
    }
#if 0
    else
    {
        /* this will happen very frequently, so disable even from verbose setting for now */
        if (0) BDBG_LOG(("%s:%d: os irq %u already %s", func, line, irq->id, enabled ? "enabled" : "disabled"));
    }
#endif
}

static void b_os_irq_set_wake_capability_spinlocked_tagged(struct b_os_irq_desc * irq, bool wakeable, const char * func, unsigned line)
{
    int result = 0;
    if (!irq->id || !irq->enabled) return;
    if (wakeable != irq->wakeable)
    {
        if (g_brcm_os_irq_verbose) BDBG_LOG(("%s:%d: os irq %s (%u) %s", func, line, irq->name, irq->id, wakeable ? "wakeable" : "unwakeable"));
        if (wakeable)
        {
            result = enable_irq_wake(irq->id);
            if (result)
            {
                BDBG_WRN(("Failed to mark irq %s (%u) as wakeup source, will not wake from this irq", irq->name, irq->id));
                BDBG_WRN(("If this message occurs for a non-AON gpio irq for S2 you need to upgrade BOLT"));
            }
        }
        else
        {
            result = disable_irq_wake(irq->id);
        }
        if (!result)
        {
            irq->wakeable = wakeable;
        }
    }
#if 0
    else
    {
        /* this will happen very frequently, so disable even from verbose setting for now */
        if (0) BDBG_LOG(("%s:%d: os irq %u already %s", func, line, irq->id, wakeable ? "wakeable" : "unwakeable"));
    }
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static irqreturn_t b_os_irq_check_linux_l1_isr(int irq, void *dev_id)
#else
static irqreturn_t b_os_irq_check_linux_l1_isr(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    disable_irq_nosync(irq);
    return IRQ_HANDLED;
}

bool b_os_irq_is_os_managed(int irq)
{
    int rc;
    void * dev = (void *)&dev;
    bool os_managed = false;

    if (irq > 0) {
        rc = request_irq(irq, b_os_irq_check_linux_l1_isr, IRQF_TRIGGER_HIGH, "nexus_irq", &dev);
        if (rc == -EINVAL || rc == -EBUSY) {
            os_managed = true;
        }
        else
        {
            free_irq(irq, &dev);
        }
    }

    return os_managed;
}
