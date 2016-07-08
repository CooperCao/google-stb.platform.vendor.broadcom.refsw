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
 * NEXUS_PLATFORM_P_SHARED_GPIO_STATE() - returns a pointer to an instance of NEXUS_PlatformSharedGpioState
 */
#include "nexus_gpio_init.h"
#include "b_shared_gpio_types.h"

bool NEXUS_Platform_P_SharedGpioSupported(void)
{
    return NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->featureSupported;
}

struct NEXUS_GpioModuleOsSharedBankPin
{
    NEXUS_GpioModuleOsSharedBankPinOpenSettings openSettings;
};

b_shared_gpio_irq_type NEXUS_Platform_P_SharedGpio_NexusInterruptTypeToDriver(NEXUS_GpioInterrupt nexusType)
{
    b_shared_gpio_irq_type driver_type = b_shared_gpio_irq_type_max;

    switch (nexusType)
    {
        case NEXUS_GpioInterrupt_eDisabled:      /* No interrupt */
            driver_type = b_shared_gpio_irq_type_disabled;
            break;
        case NEXUS_GpioInterrupt_eRisingEdge:    /* Interrupt on a 0->1 transition */
            driver_type = b_shared_gpio_irq_type_rising_edge;
            break;
        case NEXUS_GpioInterrupt_eFallingEdge:   /* Interrupt on a 1->0 transition */
            driver_type = b_shared_gpio_irq_type_falling_edge;
            break;
        case NEXUS_GpioInterrupt_eEdge:          /* Interrupt on both a 0->1 and a 1->0 transition */
            driver_type = b_shared_gpio_irq_type_edge;
            break;
        case NEXUS_GpioInterrupt_eLow:           /* Interrupt on a 0 value */
            driver_type = b_shared_gpio_irq_type_low;
            break;
        case NEXUS_GpioInterrupt_eHigh:          /* Interrupt on a 1 value */
            driver_type = b_shared_gpio_irq_type_high;
            break;
        default:
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            break;
    }

    return driver_type;
}

void NEXUS_Platform_P_SharedGpio_GetPinDesc(NEXUS_GpioModuleOsSharedBankPinHandle pin, b_shared_gpio_pin_desc * pinDesc)
{
    pinDesc->bank_base_address = pin->openSettings.bankBaseAddress;
    pinDesc->shift = pin->openSettings.shift;
}

void NEXUS_Platform_P_SharedGpio_DriverInterruptStatusToNexus_isr(const b_shared_gpio_int_status * pDriver, NEXUS_GpioModuleOsSharedBankInterruptStatus * pNexus)
{
    unsigned i;

    for (i = 0; i < NEXUS_GPIO_NUM_BANKS; i++)
    {
        pNexus->status[i] = pDriver->bank_status[i];
    }
}

static void NEXUS_Platform_P_SharedGpio_GetCapabilities(NEXUS_GpioModuleOsSharedBankCapabilities * pCapabilities)
{
    if (!NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->capsValid)
    {
        NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->caps.osManaged = NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->featureSupported;
        NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->capsValid = true;
    }

    *pCapabilities = NEXUS_PLATFORM_P_SHARED_GPIO_STATE()->caps;
}

static void NEXUS_Platform_P_SharedGpio_Close(NEXUS_GpioModuleOsSharedBankPinHandle pin)
{
    NEXUS_Platform_P_SharedGpio_ClosePin(pin);
    BKNI_Free(pin);
}

static NEXUS_GpioModuleOsSharedBankPinHandle NEXUS_Platform_P_SharedGpio_Open(const NEXUS_GpioModuleOsSharedBankPinOpenSettings * pSettings)
{
    NEXUS_GpioModuleOsSharedBankPinHandle pin = NULL;

    pin = BKNI_Malloc(sizeof(struct NEXUS_GpioModuleOsSharedBankPin));
    if (!pin) goto err_alloc;
    BKNI_Memset(pin, 0, sizeof(*pin));

    pin->openSettings = *pSettings;

    if (NEXUS_Platform_P_SharedGpio_OpenPin(pSettings)) goto err_os;

    return pin;

err_os:
    if (pin)
    {
        NEXUS_Platform_P_SharedGpio_Close(pin);
        pin = NULL;
    }

err_alloc:
    return pin;
}

void NEXUS_Platform_P_GetGpioModuleOsSharedBankSettings(NEXUS_GpioModuleOsSharedBankSettings * pSettings)
{
    if (pSettings)
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
        pSettings->init = &NEXUS_Platform_P_SharedGpio_Init;
        pSettings->getCapabilities = &NEXUS_Platform_P_SharedGpio_GetCapabilities;
        pSettings->getInterruptStatus_isr = &NEXUS_Platform_P_SharedGpio_GetInterruptStatus_isr;
        pSettings->openPin = &NEXUS_Platform_P_SharedGpio_Open;
        pSettings->closePin = &NEXUS_Platform_P_SharedGpio_Close;
        pSettings->clearPinInterrupt_isr = &NEXUS_Platform_P_SharedGpio_ClearInterrupt_isr;
        pSettings->setPinInterruptMask_isr = &NEXUS_Platform_P_SharedGpio_SetInterruptMask_isr;
        pSettings->setStandby = &NEXUS_Platform_P_SharedGpio_SetStandby;
    }
}
