/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_keypad_module.h"
#include "priv/nexus_core.h"
#include "bkpd.h"
#if !NEXUS_HAS_GPIO
#undef NEXUS_HAS_SPI
#endif
#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#include "nexus_gpio.h"
#endif

BDBG_MODULE(nexus_keypad);

NEXUS_ModuleHandle g_NEXUS_keypadModule;
struct {
    NEXUS_KeypadModuleSettings settings;
} g_NEXUS_keypad;

/****************************************
* Module functions
***************/

void NEXUS_KeypadModule_GetDefaultSettings(NEXUS_KeypadModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
    pSettings->common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
}

NEXUS_ModuleHandle NEXUS_KeypadModule_Init(const NEXUS_KeypadModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_KeypadModuleSettings defaultSettings;

    BDBG_ASSERT(!g_NEXUS_keypadModule);
    
    if (!pSettings) {
        NEXUS_KeypadModule_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(NEXUS_ModulePriority_eLow, &pSettings->common);
    g_NEXUS_keypadModule = NEXUS_Module_Create("keypad", &moduleSettings);

    BKNI_Memset(&g_NEXUS_keypad, 0, sizeof(g_NEXUS_keypad));
    g_NEXUS_keypad.settings = *pSettings;
    return g_NEXUS_keypadModule;
}

void NEXUS_KeypadModule_Uninit()
{
    NEXUS_Module_Destroy(g_NEXUS_keypadModule);
    g_NEXUS_keypadModule = NULL;
}

/****************************************
* API functions
***************/

struct NEXUS_Keypad {
    NEXUS_OBJECT(NEXUS_Keypad);
    NEXUS_KeypadSettings settings;
    BKPD_Handle kpd;
    bool overflow;
    NEXUS_KeypadEvent *queue;
    unsigned rptr, wptr;
    NEXUS_IsrCallbackHandle dataReadyCallback;
    uint32_t lastCode;
    NEXUS_Time lastTime;
#if NEXUS_HAS_SPI
    NEXUS_CallbackHandler spiInterrupt;
    NEXUS_SpiHandle spi;
    NEXUS_GpioHandle gpio;
#endif
};

void NEXUS_Keypad_GetDefaultSettings(NEXUS_KeypadSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->eventQueueSize = 10;
    pSettings->prescale = 0x0055; /* taken from KPD PI default */
    pSettings->debounce = 0x40; /* taken from KPD PI default */
    pSettings->repeatFilterTime = 200;
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);
}

#if NEXUS_HAS_SPI
/* This is the low-level SPI read */
static BERR_Code NEXUS_Keypad_P_ReadOne(NEXUS_SpiHandle spi, uint16_t *data)
{
    uint8_t wData[32], rData[32];
    BERR_Code rc;

    wData[0] = 0;   /* Chip address is zero */
    wData[1] = 0xff;

    rc = NEXUS_Spi_Read(spi, &wData[0], &rData[0], 2);
    if (!rc) {
        *data = rData[1];
    }
    return rc;
}

/* This is the high-level debounced read */
static BERR_Code NEXUS_Keypad_P_Read(NEXUS_SpiHandle spi, uint16_t *pData)
{
    BERR_Code rc;
    uint16_t val[2];

    rc = NEXUS_Keypad_P_ReadOne(spi, &val[0]);
    if (rc) return BERR_TRACE(rc);
    BKNI_Sleep(10); /* debouncing */

    rc = NEXUS_Keypad_P_ReadOne(spi, &val[1]);
    if (rc) return BERR_TRACE(rc);
    BKNI_Sleep(100); /* debouncing */

    if (val[0] == val[1]) {
        *pData = val[0];
        rc = 0;
    }
    else {
        BDBG_MSG(("debounce rejected keys %x, %x", val[0], val[1]));
        rc = -1;
    }

    return rc;
}


static void NEXUS_Keypad_P_DataReady_spi(void *context)
{
    BERR_Code rc;
    NEXUS_KeypadHandle keypad = (NEXUS_KeypadHandle)context;
    NEXUS_KeypadEvent event;
    NEXUS_Time currentTime;

    event.code = 0;

    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);

    rc = NEXUS_Keypad_P_Read(keypad->spi, &event.code);
    /* After reading, re-enable interrupt */
    NEXUS_Gpio_ClearInterrupt(keypad->gpio);
    if (rc) {
        /* no error message. a debounced key will return non-zero, and this is normal */
        goto done;
    }

    NEXUS_Time_Get(&currentTime);

    event.repeat =  (keypad->lastCode == event.code && NEXUS_Time_Diff(&currentTime, &keypad->lastTime) < (int)keypad->settings.repeatFilterTime);
    keypad->lastTime = currentTime;
    keypad->lastCode = event.code;

    BDBG_MSG(("DataReady: %x%s", event.code, event.repeat?" (repeat)":""));

    BKNI_EnterCriticalSection();
    keypad->queue[keypad->wptr++] = event;
    if (keypad->wptr == keypad->settings.eventQueueSize) {
        keypad->wptr = 0;
    }

    /* if we add an element and the queue is now empty, we have overflowed. */
    if (keypad->rptr == keypad->wptr) {
        keypad->overflow = true;
    }

    NEXUS_IsrCallback_Fire_isr(keypad->dataReadyCallback);
    BKNI_LeaveCriticalSection();
done:
    return;
}
#endif

static void NEXUS_Keypad_P_DataReady_isr(void *context, int param, void *unused)
{
    BERR_Code rc;
    NEXUS_KeypadHandle keypad = (NEXUS_KeypadHandle)context;
    NEXUS_KeypadEvent event;
    NEXUS_Time time;

    BSTD_UNUSED(param);
    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);

    rc = BKPD_Read_isr(keypad->kpd, &event.code);
    if (rc) BERR_TRACE(rc);

    NEXUS_Time_Get_isrsafe(&time);
    event.repeat = (keypad->lastCode == event.code) && (unsigned)NEXUS_Time_Diff_isrsafe(&time, &keypad->lastTime) < keypad->settings.repeatFilterTime;
    keypad->lastTime = time;
    keypad->lastCode = event.code;

    BDBG_MSG(("NEXUS_Keypad_P_DataReady_isr %x %s", event.code, event.repeat?"(repeat)":""));

    keypad->queue[keypad->wptr++] = event;
    if (keypad->wptr == keypad->settings.eventQueueSize) {
        keypad->wptr = 0;
    }

    /* if we add an element and the queue is now empty, we have overflowed. */
    if (keypad->rptr == keypad->wptr) {
        keypad->overflow = true;
    }

    NEXUS_IsrCallback_Fire_isr(keypad->dataReadyCallback);
}

NEXUS_KeypadHandle NEXUS_Keypad_Open(unsigned index, const NEXUS_KeypadSettings *pSettings)
{
    NEXUS_KeypadHandle keypad;
    BERR_Code rc;
    NEXUS_KeypadSettings defaultSettings;
    BKPD_Settings kpdSettings;
#if NEXUS_HAS_SPI
    NEXUS_GpioSettings gpioSettings;
    NEXUS_Error err;
#endif
    int i;

    BSTD_UNUSED(index);

    if (!pSettings) {
        NEXUS_Keypad_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    keypad = BKNI_Malloc(sizeof(*keypad));
    if (!keypad) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Keypad, keypad);
    keypad->dataReadyCallback = NEXUS_IsrCallback_Create(keypad, NULL);
    NEXUS_IsrCallback_Set(keypad->dataReadyCallback, &pSettings->dataReady);

    keypad->queue = BKNI_Malloc(sizeof(NEXUS_KeypadEvent) * pSettings->eventQueueSize);
    if (!keypad->queue) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    /* Check for SPI */
    /* SPI keypad requires GPIO Interrupt */
#if NEXUS_HAS_SPI
    if (g_NEXUS_keypad.settings.spi.valid && g_NEXUS_keypad.settings.gpio.valid)
    {
        NEXUS_SpiSettings spiSettings;
        NEXUS_GpioSettings keypadGpioSettings;

        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        keypad->spi = NEXUS_Spi_Open(g_NEXUS_keypad.settings.spi.index, &spiSettings);
        if (!keypad->spi) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }

        NEXUS_Gpio_GetDefaultSettings(g_NEXUS_keypad.settings.gpio.type, &keypadGpioSettings);
        keypad->gpio = NEXUS_Gpio_Open(g_NEXUS_keypad.settings.gpio.type, g_NEXUS_keypad.settings.gpio.pin, &keypadGpioSettings);
        if (!keypad->gpio) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }

        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
        NEXUS_CallbackHandler_Init(keypad->spiInterrupt, NEXUS_Keypad_P_DataReady_spi, keypad);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eHigh;
        NEXUS_CallbackHandler_PrepareCallback(keypad->spiInterrupt, gpioSettings.interrupt);
        err = NEXUS_Gpio_SetSettings(keypad->gpio, &gpioSettings);
        if (err) {
            BDBG_ERR(("Unable to Set GPIO Settings keypad SPI interrupt"));
            goto error;
        }

        goto done;
    }
#endif

    rc = BKPD_GetDefaultSettings(&kpdSettings, g_pCoreHandles->chp);
    if (rc) {BERR_TRACE(rc);goto error;}

    kpdSettings.prescale = pSettings->prescale;
    kpdSettings.debounce = pSettings->debounce;
    kpdSettings.keyMask = pSettings->keyMask;
    kpdSettings.intMode = true;

    rc = BKPD_Open(&keypad->kpd, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &kpdSettings);
    if (rc) {BERR_TRACE(rc);goto error;}

    (void)BKPD_InstallInterruptCallback(keypad->kpd, NEXUS_Keypad_P_DataReady_isr, keypad, 0);

    for (i=0;i<4;i++)
    {
        uint16_t code;

        rc = BKPD_Read(keypad->kpd, &code);
        if (rc) {BERR_TRACE(rc);goto error;}
        if (code != 0)
        {   /* this pattern occurs when no keypad is connected */
            goto done;
        }
    }
    BDBG_WRN(("no keypad detected"));
    goto error;

done:
    keypad->settings = *pSettings;

    return keypad;
error:
    NEXUS_Keypad_Close(keypad);
    return NULL;
}

static void NEXUS_Keypad_P_Finalizer(NEXUS_KeypadHandle keypad)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Keypad, keypad);

    if (keypad->kpd) {
        BKPD_Close(keypad->kpd);
    }

#if NEXUS_HAS_SPI
    if (keypad->spi) {
        NEXUS_Spi_Close(keypad->spi);
    }
    if (keypad->gpio) {
        NEXUS_GpioSettings gpioSettings;

        NEXUS_Gpio_GetSettings(keypad->gpio, &gpioSettings);
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(keypad->gpio, &gpioSettings);
        NEXUS_CallbackHandler_Shutdown(keypad->spiInterrupt);

        NEXUS_Gpio_Close(keypad->gpio);
    }
#endif

    if (keypad->queue) {
        BKNI_Free(keypad->queue);
    }
    if (keypad->dataReadyCallback) {
        NEXUS_IsrCallback_Destroy(keypad->dataReadyCallback);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Keypad, keypad);
    BKNI_Free(keypad);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Keypad, NEXUS_Keypad_Close);

NEXUS_Error NEXUS_Keypad_GetEvents(NEXUS_KeypadHandle keypad, NEXUS_KeypadEvent *pEvents, size_t numEvents, size_t *pNumEventsRead, bool *pOverflow)
{
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);

    if (numEvents && !pEvents) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    *pNumEventsRead = 0;

    BKNI_EnterCriticalSection();
    while (numEvents && keypad->rptr != keypad->wptr) {
        pEvents[(*pNumEventsRead)++] = keypad->queue[keypad->rptr++];
        numEvents--;
        if (keypad->rptr == keypad->settings.eventQueueSize) {
            keypad->rptr = 0;
        }
    }
    *pOverflow = keypad->overflow;
    keypad->overflow = false;
    BKNI_LeaveCriticalSection();

    return 0;
}

void NEXUS_Keypad_FlushEvents(NEXUS_KeypadHandle keypad)
{
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);
    BKNI_EnterCriticalSection();
    keypad->rptr = keypad->wptr = 0;
    keypad->overflow = false;
    BKNI_LeaveCriticalSection();
}
