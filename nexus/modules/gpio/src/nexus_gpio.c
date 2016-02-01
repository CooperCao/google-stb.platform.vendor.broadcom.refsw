/***************************************************************************
 *     (c)2007-2015 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#include "nexus_gpio_module.h"
#include "priv/nexus_core.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_gpio_standby_priv.h"
#include "bint.h"
#include "bchp_common.h"
#include "bchp_gio.h"

#ifdef BCHP_UPG_MAIN_IRQ_REG_START
/* New B53 IRQ register mappings */
#include "bchp_int_id_upg_main_irq.h"
#include "bchp_int_id_upg_main_aon_irq.h"
#define BCHP_INT_ID_gio_irqen BCHP_INT_ID_UPG_MAIN_IRQ_gio
#define BCHP_INT_ID_aon_gio_irqen BCHP_INT_ID_UPG_MAIN_AON_IRQ_gio

#else /* B15, MIPS */

#include "bchp_int_id_irq0.h"

#ifdef BCHP_GIO_AON_REG_START
#include "bchp_int_id_irq0_aon.h"
#include "bchp_gio_aon.h"
#if defined (BCHP_INT_ID_IRQ0_AON_gpio_irqen)
#define BCHP_INT_ID_aon_gio_irqen BCHP_INT_ID_IRQ0_AON_gpio_irqen
#else
#define BCHP_INT_ID_aon_gio_irqen BCHP_INT_ID_IRQ0_AON_gio_irqen
#endif
#endif /* BCHP_GIO_AON_REG_START */

#endif /* BCHP_UPG_MAIN_IRQ_REG_START */

/**
The Nexus Gpio module does not use the Magnum GIO porting interface.
Interrupts are requested directly from INT. GPIO registers are accessed directly.
**/

BDBG_MODULE(nexus_gpio);

NEXUS_ModuleHandle g_NEXUS_gpioModule;
struct {
    NEXUS_GpioModuleSettings settings;
    struct {
        BINT_CallbackHandle callback;
        BINT_Id id;
        unsigned refCnt;
    } intCallbacks [1
    #ifdef BCHP_INT_ID_aon_gio_irqen
        + 1
    #endif
            ];
    BLST_S_HEAD(NEXUS_GpioList, NEXUS_Gpio) list;
    bool standby;
} g_NEXUS_gpio;
static void NEXUS_Gpio_P_isr(void *context, int param);
static void NEXUS_Gpio_P_ReadSettings(NEXUS_GpioHandle gpio);

/****************************************
* Module functions
***************/

void NEXUS_GpioModule_GetDefaultSettings(NEXUS_GpioModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.enabledDuringActiveStandby = true;
}

NEXUS_ModuleHandle NEXUS_GpioModule_Init(const NEXUS_GpioModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_GpioModuleSettings defaultSettings;
    unsigned i;

    BDBG_ASSERT(!g_NEXUS_gpioModule);

    if (!pSettings) {
        NEXUS_GpioModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    g_NEXUS_gpioModule = NEXUS_Module_Create("gpio", &moduleSettings);
    if (!g_NEXUS_gpioModule) {
        return NULL;
    }
    NEXUS_LockModule();
    BKNI_Memset(&g_NEXUS_gpio, 0, sizeof(g_NEXUS_gpio));
    g_NEXUS_gpio.settings = *pSettings;
    BLST_S_INIT(&g_NEXUS_gpio.list);

    /* Prior to installing L2 callback, we must ensure that all GPIO interrupts are masked. */
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_MASK_LO, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_STAT_LO, 0xFFFFFFFF, 0xFFFFFFFF);
    #ifdef BCHP_GIO_MASK_HI
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_MASK_HI, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_STAT_HI, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif
    #ifdef BCHP_GIO_MASK_EXT
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_STAT_EXT, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif
    #ifdef BCHP_GIO_AON_MASK_LO
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_AON_MASK_LO, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_AON_STAT_LO, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif
    #ifdef BCHP_GIO_AON_MASK_EXT
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_AON_MASK_EXT, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_AON_STAT_EXT, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif
    #ifdef BCHP_GIO_MASK_EXT_HI
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT_HI, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_STAT_EXT_HI, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif
    #ifdef BCHP_GIO_MASK_EXT2
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT2, 0xFFFFFFFF, 0);
    BREG_Update32(g_pCoreHandles->reg, BCHP_GIO_STAT_EXT2, 0xFFFFFFFF, 0xFFFFFFFF);
    #endif

#ifndef BCHP_INT_ID_gio_irqen
#ifdef BCHP_INT_ID_IRQ0_gio_irqen
#define BCHP_INT_ID_gio_irqen BCHP_INT_ID_IRQ0_gio_irqen
#elif defined(BCHP_INT_ID_IRQ0_gpio_irqen)
#define BCHP_INT_ID_gio_irqen BCHP_INT_ID_IRQ0_gpio_irqen
#else
#define BCHP_INT_ID_gio_irqen BCHP_INT_ID_gio
#endif
#endif
    for(i=0;i<sizeof(g_NEXUS_gpio.intCallbacks)/sizeof(g_NEXUS_gpio.intCallbacks[0]);i++) {
        BINT_Id id = BCHP_INT_ID_gio_irqen;

        #ifdef BCHP_INT_ID_aon_gio_irqen
        if(i==1) {
            id = BCHP_INT_ID_aon_gio_irqen;
        }
        #endif
        g_NEXUS_gpio.intCallbacks[i].refCnt = 0;
        g_NEXUS_gpio.intCallbacks[i].id = id;
        g_NEXUS_gpio.intCallbacks[i].callback = NULL;
    }

    NEXUS_UnlockModule();
    return g_NEXUS_gpioModule;
}

void NEXUS_GpioModule_Uninit(void)
{
    unsigned i;
    NEXUS_LockModule();
    for(i=0;i<sizeof(g_NEXUS_gpio.intCallbacks)/sizeof(g_NEXUS_gpio.intCallbacks[0]);i++) {
        if(g_NEXUS_gpio.intCallbacks[i].callback) {
            BINT_DisableCallback(g_NEXUS_gpio.intCallbacks[i].callback);
            BINT_DestroyCallback(g_NEXUS_gpio.intCallbacks[i].callback);
        }
    }
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_gpioModule);
    g_NEXUS_gpioModule = NULL;
}

/****************************************
* API functions
***************/

void NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType type, NEXUS_GpioSettings *pSettings)
{
    BSTD_UNUSED(type);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_GpioHandle NEXUS_Gpio_OpenAux(unsigned typeAndPin, const NEXUS_GpioSettings *pSettings)
{
    NEXUS_GpioHandle gpio;
    BERR_Code rc;
#ifndef NEXUS_GPIO_REGISTER_ABSTRACTION
    uint32_t address;
#endif
#define NEXUS_GPIO_TYPE(typeAndPin) (typeAndPin >> 16)
#define NEXUS_GPIO_PIN(typeAndPin) (typeAndPin & 0xFFFF)
    NEXUS_GpioType type = NEXUS_GPIO_TYPE(typeAndPin);
    unsigned pin = NEXUS_GPIO_PIN(typeAndPin);

    /* There is no bounds check for type or pin in this generic file.
    When the general eMax was extended from 2 to 3, no existing copies of nexus_gpio_table.c were updated to handle it.
    The worst case is that type>1 is handled as if type==0.
    If more rigorous checking is needed, it must be done in each chip's nexus_gpio_table.c. */

    /* chip-specific sanity check */
    rc = NEXUS_Gpio_P_CheckPinmux(type, pin);
    if ( rc!=NEXUS_SUCCESS ) { rc = BERR_TRACE(rc); goto err_pinmux;}

    gpio = BKNI_Malloc(sizeof(*gpio));
    if(!gpio) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    NEXUS_OBJECT_INIT(NEXUS_Gpio, gpio);

    gpio->pin = pin;
    gpio->type = type;
#if NEXUS_GPIO_REGISTER_ABSTRACTION
    rc = NEXUS_Gpio_P_GetPinData(gpio);
    if ( rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_pindata;}
#else
    rc = NEXUS_Gpio_P_GetPinData(type, pin, &address, &gpio->shift);
    if ( rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_pindata;}

    /* populate the addresses */
    {
    unsigned offset = address - BCHP_GIO_ODEN_LO;
    gpio->addr.iodir=BCHP_GIO_IODIR_LO+offset;
    gpio->addr.data=BCHP_GIO_DATA_LO+offset;
    gpio->addr.oden=BCHP_GIO_ODEN_LO+offset;
    gpio->addr.mask=BCHP_GIO_MASK_LO+offset;
    gpio->addr.ec= BCHP_GIO_EC_LO+offset;
    gpio->addr.ei= BCHP_GIO_EI_LO+offset;
    gpio->addr.level=BCHP_GIO_LEVEL_LO+offset;
    gpio->addr.stat=BCHP_GIO_STAT_LO+offset;
    }
#endif
    gpio->isrCallback = NEXUS_IsrCallback_Create(gpio,NULL);
    if(!gpio->isrCallback) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_callback;}

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&g_NEXUS_gpio.list, gpio, link);
    BKNI_LeaveCriticalSection();

    if (pSettings) {
        rc = NEXUS_Gpio_SetSettings(gpio, pSettings);
        if (rc) {rc=BERR_TRACE(rc); goto err_gpio;}
    }
    else {
        NEXUS_Gpio_P_ReadSettings(gpio);
    }

    return gpio;

err_pindata:
err_callback:
err_gpio:
    NEXUS_Gpio_Close(gpio);
err_alloc:
err_pinmux:
    return NULL;
}

static unsigned NEXUS_Gpio_P_InterruptIndex(NEXUS_GpioHandle gpio)
{
    BSTD_UNUSED(gpio);
    #ifdef BCHP_INT_ID_aon_gio_irqen
    #ifdef BCHP_GIO_AON_MASK_LO
    if(gpio->addr.mask == BCHP_GIO_AON_MASK_LO) {
        return 1;
    }
    #endif
    #ifdef BCHP_GIO_AON_MASK_EXT
    if(gpio->addr.mask == BCHP_GIO_AON_MASK_EXT) {
        return 1;
    }
    #endif
    #endif
    return 0;
}

static NEXUS_Error NEXUS_Gpio_P_AcquireInterrupt(NEXUS_GpioHandle gpio)
{
    unsigned index = NEXUS_Gpio_P_InterruptIndex(gpio);

    BDBG_MSG(("%p:AcquireInterrupt isr:%u %#x:%u", (void*)gpio, index, gpio->addr.mask, gpio->shift));
    if(g_NEXUS_gpio.intCallbacks[index].refCnt==0) {
        BERR_Code rc;
        rc = BINT_CreateCallback(&g_NEXUS_gpio.intCallbacks[index].callback, g_pCoreHandles->bint, g_NEXUS_gpio.intCallbacks[index].id, NEXUS_Gpio_P_isr, NULL, 0);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc); }
        rc = BINT_EnableCallback(g_NEXUS_gpio.intCallbacks[index].callback);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc); }
    }
    g_NEXUS_gpio.intCallbacks[index].refCnt++;

    return NEXUS_SUCCESS;
}

static void NEXUS_Gpio_P_ReleaseInterrupt(NEXUS_GpioHandle gpio)
{
    unsigned index = NEXUS_Gpio_P_InterruptIndex(gpio);
    BDBG_MSG(("%p:ReleaseInterrupt isr:%u %#x:%u", (void*)gpio, index, gpio->addr.mask, gpio->shift));
    if(g_NEXUS_gpio.intCallbacks[index].refCnt>0) {
        g_NEXUS_gpio.intCallbacks[index].refCnt--;
        if(g_NEXUS_gpio.intCallbacks[index].refCnt==0) {
            BINT_DisableCallback(g_NEXUS_gpio.intCallbacks[index].callback);
            BINT_DestroyCallback(g_NEXUS_gpio.intCallbacks[index].callback);
            g_NEXUS_gpio.intCallbacks[index].callback=NULL;
        }
    }
    return;
}

static void NEXUS_Gpio_P_Finalizer(NEXUS_GpioHandle gpio)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);
    if (gpio->isrCallback) {
        NEXUS_IsrCallback_Destroy(gpio->isrCallback);
    }
    if(gpio->settings.interruptMode != NEXUS_GpioInterrupt_eDisabled) {
        NEXUS_Gpio_P_ReleaseInterrupt(gpio);
    }
    BKNI_EnterCriticalSection();
    NEXUS_Gpio_SetInterruptEnabled_isr(gpio, false);
    BLST_S_REMOVE(&g_NEXUS_gpio.list, gpio, NEXUS_Gpio, link);
    BKNI_LeaveCriticalSection();
    NEXUS_OBJECT_DESTROY(NEXUS_Gpio, gpio);
    BKNI_Free(gpio);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Gpio, NEXUS_Gpio_Close);

void NEXUS_Gpio_GetSettings(NEXUS_GpioHandle gpio, NEXUS_GpioSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);
    *pSettings = gpio->settings;
}

static void NEXUS_Gpio_P_SetBit_isr(uint32_t addr, unsigned shift, bool set)
{
    uint32_t mask = 1 << shift;
    uint32_t val = set ? mask: 0;
    BREG_Update32(g_pCoreHandles->reg, addr, mask, val);
}

static unsigned NEXUS_Gpio_P_GetBit_isrsafe(uint32_t addr, unsigned shift)
{
    /* No critical section because its an atomic read. */
    return (BREG_Read32(g_pCoreHandles->reg, addr) >> shift) & 0x1;
}

void NEXUS_Gpio_SetInterruptEnabled_isr(NEXUS_GpioHandle gpio, bool enabled)
{
      NEXUS_Gpio_P_SetBit_isr(gpio->addr.mask, gpio->shift, enabled); /* The MASK register is a misnomer. MASK = 1 is unmasked. */
}

static void NEXUS_Gpio_P_ReadSettings(NEXUS_GpioHandle gpio)
{
    unsigned edge_conf, edge_insensitive, edge_level;

    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);

    if (NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.iodir, gpio->shift)) {
        gpio->settings.mode = NEXUS_GpioMode_eInput;
    }
    else if (NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.oden, gpio->shift)) {
        gpio->settings.mode = NEXUS_GpioMode_eOutputOpenDrain;
    }
    else {
        gpio->settings.mode = NEXUS_GpioMode_eOutputPushPull;
    }

    edge_conf = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.ec, gpio->shift);
    edge_insensitive = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.ei, gpio->shift);
    edge_level = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.level, gpio->shift);

    if (edge_conf == 1 && edge_insensitive == 0 && edge_level == 0) {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eRisingEdge;
    }
    else if (edge_conf == 0 && edge_insensitive == 0 && edge_level == 0) {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;
    }
    else if (edge_conf == 0 && edge_insensitive == 1 && edge_level == 0) {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eEdge;
    }
    else if (edge_conf == 1 && edge_insensitive == 0 && edge_level == 1) {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eHigh;
    }
    else if (edge_conf == 0 && edge_insensitive == 0 && edge_level == 1) {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eLow;
    }
    else {
        gpio->settings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    }

    gpio->settings.value = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.data, gpio->shift);
}

NEXUS_Error NEXUS_Gpio_SetSettings(NEXUS_GpioHandle gpio, const NEXUS_GpioSettings *pSettings)
{
    unsigned edge_conf = 0, edge_insensitive = 0, edge_level = 0, enabled = 1;

    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);

    switch (pSettings->interruptMode){
    case NEXUS_GpioInterrupt_eRisingEdge:
        edge_conf = 1;
        edge_insensitive = 0;
        edge_level = 0;
        break;
    case NEXUS_GpioInterrupt_eFallingEdge:
        edge_conf = 0;
        edge_insensitive = 0;
        edge_level = 0;
        break;
    case NEXUS_GpioInterrupt_eEdge:
        edge_conf = 0;
        edge_insensitive = 1;
        edge_level = 0;
        break;
    case NEXUS_GpioInterrupt_eHigh:
        edge_conf = 1;
        edge_insensitive = 0;
        edge_level = 1;
        break;
    case NEXUS_GpioInterrupt_eLow:
        edge_conf = 0;
        edge_insensitive = 0;
        edge_level = 1;
        break;
    default: /* NEXUS_GpioInterrupt_eDisabled */
        enabled = 0;
        break;
    }

    NEXUS_IsrCallback_Set(gpio->isrCallback, &pSettings->interrupt);

    if(enabled) {
        if(gpio->settings.interruptMode == NEXUS_GpioInterrupt_eDisabled) {
            NEXUS_Error rc;
            rc = NEXUS_Gpio_P_AcquireInterrupt(gpio);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        }
    } else if(gpio->settings.interruptMode != NEXUS_GpioInterrupt_eDisabled) {
            NEXUS_Gpio_P_ReleaseInterrupt(gpio);
    }
    BKNI_EnterCriticalSection();

    /* these calls have been ordered to remove glitches in output */
    if (pSettings->mode != NEXUS_GpioMode_eInput) {
        NEXUS_Gpio_P_SetBit_isr(gpio->addr.data, gpio->shift, pSettings->value);
    }

    NEXUS_Gpio_P_SetBit_isr(gpio->addr.oden, gpio->shift, pSettings->mode == NEXUS_GpioMode_eOutputOpenDrain);

    NEXUS_Gpio_P_SetBit_isr(gpio->addr.iodir, gpio->shift, pSettings->mode == NEXUS_GpioMode_eInput);

    if (!enabled) {
        /* Mask before resetting interrupt condition bits to avoid a re-trigger */
        NEXUS_Gpio_SetInterruptEnabled_isr(gpio, false);
    }

    NEXUS_Gpio_P_SetBit_isr(gpio->addr.ec, gpio->shift, edge_conf);
    NEXUS_Gpio_P_SetBit_isr(gpio->addr.ei, gpio->shift, edge_insensitive);
    NEXUS_Gpio_P_SetBit_isr(gpio->addr.level, gpio->shift, edge_level);

    gpio->settings = *pSettings;

    if (enabled) {
        NEXUS_Gpio_SetInterruptEnabled_isr(gpio, enabled);
    }
    BKNI_LeaveCriticalSection();

    return 0;
}

NEXUS_Error NEXUS_Gpio_GetStatus(NEXUS_GpioHandle gpio, NEXUS_GpioStatus *pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);
    BKNI_EnterCriticalSection();
    pStatus->value = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.data, gpio->shift);
    pStatus->interruptPending = NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.stat, gpio->shift);
    BKNI_LeaveCriticalSection();
    return 0;
}

NEXUS_Error NEXUS_Gpio_ClearInterrupt(NEXUS_GpioHandle gpio)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Gpio, gpio);
    BKNI_EnterCriticalSection();
    BREG_Update32(g_pCoreHandles->reg, gpio->addr.stat, 0xFFFFFFFF, 1 << gpio->shift);
    if ( gpio->settings.interruptMode != NEXUS_GpioInterrupt_eDisabled )
    {
        /* Re-enable interrupts.  May be a masked level interrupt */
        NEXUS_Gpio_SetInterruptEnabled_isr(gpio, true);
    }
    BKNI_LeaveCriticalSection();
    return 0;
}

void NEXUS_Gpio_SetInterruptCallback_priv(NEXUS_GpioHandle gpio, NEXUS_GpioIsrCallback callback_isr, void *context, int param)
{
    BKNI_EnterCriticalSection();
    gpio->directIsrCallback.callback_isr = callback_isr;
    gpio->directIsrCallback.context = context;
    gpio->directIsrCallback.param = param;
    BKNI_LeaveCriticalSection();
}

static bool NEXUS_Gpio_P_Dispatch_isr(NEXUS_GpioHandle gpio)
{
    if (NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.stat, gpio->shift) &
        NEXUS_Gpio_P_GetBit_isrsafe(gpio->addr.mask, gpio->shift))
    {
        /* clear status immediately */
        BREG_Update32(g_pCoreHandles->reg,gpio->addr.stat, 0xFFFFFFFF, 1 << gpio->shift);
        if ( gpio->settings.maskEdgeInterrupts ||
             gpio->settings.interruptMode == NEXUS_GpioInterrupt_eLow ||
             gpio->settings.interruptMode == NEXUS_GpioInterrupt_eHigh )
        {
            /* Mask a level interrupt */
            NEXUS_Gpio_SetInterruptEnabled_isr(gpio, false);
        }
        if(gpio->isrCallback) {
            NEXUS_IsrCallback_Fire_isr(gpio->isrCallback);
        }

        if(gpio->directIsrCallback.callback_isr){
            gpio->directIsrCallback.callback_isr(gpio->directIsrCallback.context, gpio->directIsrCallback.param);
        }

        return true;
    }

    return false;
}

static void NEXUS_Gpio_P_isr(void *context, int param)
{
    NEXUS_GpioHandle gpio;
    BDBG_MSG(("NEXUS_Gpio_P_isr"));

    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

#if 0 /* Useful for debugging flood scenarios */
    BDBG_MSG(("GIO_MASK_LO 0x%08x GIO_STAT_LO 0x%08x", BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_LO), BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_STAT_LO)));
    #ifdef BCHP_GIO_MASK_HI
    BDBG_MSG(("GIO_MASK_HI 0x%08x GIO_STAT_HI 0x%08x", BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_HI), BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_STAT_HI)));
    #endif
    #ifdef BCHP_GIO_MASK_EXT
    BDBG_MSG(("GIO_MASK_EXT 0x%08x GIO_STAT_EXT 0x%08x", BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT), BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_STAT_EXT)));
    #endif
    #ifdef BCHP_GIO_MASK_EXT_HI
    BDBG_MSG(("GIO_MASK_EXT_HI 0x%08x GIO_STAT_EXT_HI 0x%08x", BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT_HI), BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT_HI)));
    #endif
    #ifdef BCHP_GIO_MASK_EXT2
    BDBG_MSG(("GIO_MASK_EXT2 0x%08x GIO_STAT_EXT2 0x%08x", BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT2), BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_MASK_EXT2)));
    #endif
#endif

    for (gpio=BLST_S_FIRST(&g_NEXUS_gpio.list); gpio; gpio = BLST_S_NEXT(gpio, link)) {
        if (NEXUS_Gpio_P_Dispatch_isr(gpio)) {
            BDBG_MSG(("Dispatched GPIO interrupt %u, type %u", gpio->pin, gpio->type));
        }
    }
}

NEXUS_Error NEXUS_Gpio_GetPinMux( unsigned typeAndPin, uint32_t *pAddr, uint32_t *pMask, unsigned *pShift )
{
    NEXUS_GpioType type = NEXUS_GPIO_TYPE(typeAndPin);
    unsigned pin = NEXUS_GPIO_PIN(typeAndPin);
    return NEXUS_Gpio_P_GetPinMux(type, pin, pAddr, pMask, pShift );
}

NEXUS_Error NEXUS_GpioModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc = NEXUS_SUCCESS;

    if (enabled) {
        if(pSettings->mode==NEXUS_StandbyMode_eDeepSleep || pSettings->mode==NEXUS_StandbyMode_ePassive ) {
            if(g_NEXUS_gpio.intCallbacks[0].callback) {
                rc = BINT_DisableCallback(g_NEXUS_gpio.intCallbacks[0].callback);
            }
            g_NEXUS_gpio.standby = true;
        }
    } else {
        if(g_NEXUS_gpio.standby) {
            if(g_NEXUS_gpio.intCallbacks[0].callback) {
                rc = BINT_EnableCallback(g_NEXUS_gpio.intCallbacks[0].callback);
            }
            g_NEXUS_gpio.standby = false;
        }
    }
    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

void NEXUS_Gpio_GetPinData_priv(NEXUS_GpioHandle gpio, NEXUS_GpioPinData *pPinData)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    pPinData->address = 0;
    pPinData->shift = 0;

#if NEXUS_GPIO_REGISTER_ABSTRACTION
    errCode = NEXUS_Gpio_P_GetPinData(gpio);
    if ( errCode!=NEXUS_SUCCESS) { errCode = BERR_TRACE(errCode); goto done;}
    pPinData->address = gpio->addr.oden;
    pPinData->shift = gpio->pin;
#else
    errCode = NEXUS_Gpio_P_GetPinData(gpio->type, gpio->pin, &pPinData->address, &pPinData->shift);
    if ( errCode!=NEXUS_SUCCESS) { errCode = BERR_TRACE(errCode); goto done;}
#endif

done:
    return;
}
