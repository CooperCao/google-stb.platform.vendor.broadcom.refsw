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
#include "nexus_spi.h"
#include "nexus_gpio.h"

BDBG_MODULE(nexus_keypad);

NEXUS_ModuleHandle g_NEXUS_keypadModule;
struct {
    NEXUS_KeypadModuleSettings settings; /* unused */
} g_NEXUS_keypad;

#if BCHP_CHIP == 3563
#define KEYPAD_GPIO 24
#elif BCHP_CHIP == 7325
#define KEYPAD_GPIO 31
#elif (BCHP_CHIP == 7335) || (BCHP_CHIP==7336)
#define KEYPAD_GPIO 17
#elif BCHP_CHIP == 3548 || BCHP_CHIP == 3556
#define KEYPAD_GPIO 2
#else
#error not supported
#endif

/****************************************
* Module functions
***************/

void NEXUS_KeypadModule_GetDefaultSettings(NEXUS_KeypadModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_KeypadModule_Init(const NEXUS_KeypadModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;

    BDBG_ASSERT(!g_NEXUS_keypadModule);
    BSTD_UNUSED(pSettings);

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLowActiveStandby;
    g_NEXUS_keypadModule = NEXUS_Module_Create("keypad", &moduleSettings);
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

BDBG_OBJECT_ID(NEXUS_Keypad);

struct NEXUS_Keypad {
    BDBG_OBJECT(NEXUS_Keypad)
    NEXUS_KeypadSettings settings;
    NEXUS_SpiHandle spi;
    NEXUS_GpioHandle gpio;
    bool overflow;
    NEXUS_KeypadEvent *queue;
    unsigned rptr, wptr;
    NEXUS_TaskCallbackHandle dataReadyCallback;
    NEXUS_Time lastTime;
    uint32_t lastCode;
};

void NEXUS_Keypad_GetDefaultSettings(NEXUS_KeypadSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->eventQueueSize = 10;
    pSettings->prescale = 0x0055; /* taken from KPD PI default */
    pSettings->debounce = 0x40; /* taken from KPD PI default */
    pSettings->repeatFilterTime = 250; /* milliseconds */
    NEXUS_CallbackDesc_Init(&pSettings->dataReady);
}

/* This is the low-level SPI read */
static BERR_Code NEXUS_Keypad_P_ReadOne(NEXUS_KeypadHandle keypad, uint16_t *data)
{
    uint8_t wData[32], rData[32];
    BERR_Code rc;

    wData[0] = 0;   /* Chip address is zero */
    wData[1] = 0xff;

    rc = NEXUS_Spi_Read(keypad->spi, &wData[0], &rData[0], 2);
    if (!rc) {
        *data = rData[1];
    }
    return rc;
}

/* This is the high-level debounced read */
static BERR_Code NEXUS_Keypad_P_Read(NEXUS_KeypadHandle keypad, uint16_t *pData)
{
    BERR_Code rc;
    uint16_t val[2];

    rc = NEXUS_Keypad_P_ReadOne(keypad, &val[0]);
    if (rc) return BERR_TRACE(rc);
    BKNI_Sleep(10); /* debouncing */

    rc = NEXUS_Keypad_P_ReadOne(keypad, &val[1]);
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

/* This is a callback from the Gpio module, so we must manually lock. */
static void NEXUS_Keypad_P_DataReady_nosync(void *context, int param)
{
    BERR_Code rc;
    NEXUS_KeypadHandle keypad = (NEXUS_KeypadHandle)context;
    NEXUS_KeypadEvent event;
    NEXUS_Time currentTime;

    event.code = 0;

    NEXUS_LockModule();
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);
    BSTD_UNUSED(param);

    rc = NEXUS_Keypad_P_Read(keypad, &event.code);
    /* After reading, re-enable interrupt */
    NEXUS_Gpio_ClearInterrupt(keypad->gpio);
    if (rc) {
        /* no error message. a debounced key will return non-zero, and this is normal */
        goto done;
    }

    NEXUS_Time_Get(&currentTime);

    event.repeat = (keypad->lastCode == event.code && NEXUS_Time_Diff(&currentTime, &keypad->lastTime) < (int)keypad->settings.repeatFilterTime);
    keypad->lastTime = currentTime;
    keypad->lastCode = event.code;

    BDBG_MSG(("DataReady: %x%s", event.code, event.repeat?" (repeat)":""));

    keypad->queue[keypad->wptr++] = event;
    if (keypad->wptr == keypad->settings.eventQueueSize) {
        keypad->wptr = 0;
    }

    /* if we add an element and the queue is now empty, we have overflowed. */
    if (keypad->rptr == keypad->wptr) {
        keypad->overflow = true;
    }

    NEXUS_TaskCallback_Fire(keypad->dataReadyCallback);
done:
    NEXUS_UnlockModule();
}

NEXUS_KeypadHandle NEXUS_Keypad_Open(unsigned index, const NEXUS_KeypadSettings *pSettings)
{
    NEXUS_KeypadHandle keypad;
    NEXUS_KeypadSettings defaultSettings;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_SpiSettings spiSettings;

    BSTD_UNUSED(index);

    if (!pSettings) {
        NEXUS_Keypad_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    keypad = BKNI_Malloc(sizeof(*keypad));
    BKNI_Memset(keypad, 0, sizeof(*keypad));
    BDBG_OBJECT_SET(keypad, NEXUS_Keypad);

    /* 3563 keypad requires SPI channel 1 */
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = false;
    keypad->spi = NEXUS_Spi_Open(1, &spiSettings);
    if (!keypad->spi) {
        BDBG_ERR(("Unable to open SPI 1 for keypad"));
        goto error;
    }

    keypad->queue = BKNI_Malloc(sizeof(NEXUS_KeypadEvent) * pSettings->eventQueueSize);
    if ( NULL == keypad->queue )
    {
        BDBG_ERR(("Out of memory"));
        goto error;
    }

    keypad->dataReadyCallback = NEXUS_TaskCallback_Create(keypad, NULL);
    NEXUS_TaskCallback_Set(keypad->dataReadyCallback, &pSettings->dataReady);
    keypad->settings = *pSettings;

    /* SPI keypad requires GPIO Interrupt */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eHigh;
    gpioSettings.interrupt.callback = NEXUS_Keypad_P_DataReady_nosync;
    gpioSettings.interrupt.context = keypad;
    keypad->gpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, KEYPAD_GPIO, &gpioSettings);
    if (!keypad->gpio) {
        BDBG_ERR(("Unable to open GPIO %d for keypad SPI interrupt", KEYPAD_GPIO));
        goto error;
    }

    return keypad;
error:
    NEXUS_Keypad_Close(keypad);
    return NULL;
}

void NEXUS_Keypad_Close(NEXUS_KeypadHandle keypad)
{
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);

    if (keypad->spi) {
        NEXUS_Spi_Close(keypad->spi);
    }
    if (keypad->gpio) {
        NEXUS_Gpio_Close(keypad->gpio);
    }
    if (keypad->queue) {
        BKNI_Free(keypad->queue);
    }
    if (keypad->dataReadyCallback) {
        NEXUS_TaskCallback_Destroy(keypad->dataReadyCallback);
    }

    BDBG_OBJECT_UNSET(keypad, NEXUS_Keypad);
    BKNI_Free(keypad);
}

NEXUS_Error NEXUS_Keypad_GetEvents(NEXUS_KeypadHandle keypad, NEXUS_KeypadEvent *pEvents, size_t numEvents, size_t *pNumEventsRead, bool *pOverflow)
{
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);

    *pNumEventsRead = 0;
    while (numEvents && keypad->rptr != keypad->wptr) {
        pEvents[(*pNumEventsRead)++] = keypad->queue[keypad->rptr++];
        numEvents--;
        if (keypad->rptr == keypad->settings.eventQueueSize) {
            keypad->rptr = 0;
        }
    }

    *pOverflow = keypad->overflow;
    keypad->overflow = false;

    return 0;
}

void NEXUS_Keypad_FlushEvents(NEXUS_KeypadHandle keypad)
{
    BDBG_OBJECT_ASSERT(keypad, NEXUS_Keypad);
    keypad->rptr = keypad->wptr = 0;
}
