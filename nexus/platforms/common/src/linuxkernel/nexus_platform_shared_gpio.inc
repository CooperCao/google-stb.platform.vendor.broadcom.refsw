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
 * This "macro" expects the following is available before including
 * this file into your C file:
 *
 * static void NEXUS_Platform_P_GetSharedGpioSubmoduleInitSettings(b_shared_gpio_module_init_settings * gio_settings);
 */
#include "nexus_gpio_init.h"
#include "../nexus_platform_shared_gpio_common.h"
#include "b_shared_gpio.h"

static NEXUS_Platform_P_SharedGpioState g_NEXUS_Platform_P_SharedGpioState;

void NEXUS_Platform_P_InitSharedGpioSubmodule(void)
{
    b_shared_gpio_module_init_settings gio_settings;
    b_shared_gpio_capabilities gio_caps;

    NEXUS_Platform_P_GetSharedGpioSubmoduleInitSettings(&gio_settings);

    b_shared_gpio_init_submodule(&gio_settings);
    b_shared_gpio_get_capabilities(&gio_caps);
    g_NEXUS_Platform_P_SharedGpioState.featureSupported = gio_caps.feature_supported == 1;
    BDBG_WRN(("shared gpio %s", g_NEXUS_Platform_P_SharedGpioState.featureSupported ? "enabled" : "disabled"));
}

void NEXUS_Platform_P_UninitSharedGpioSubmodule(void)
{
    b_shared_gpio_uninit_submodule();
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_Init(const NEXUS_GpioModuleOsSharedBankInitSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    b_shared_gpio_init_banks_settings init_banks;
    unsigned i;

    BDBG_CASSERT(NEXUS_GPIO_NUM_BANKS == B_SHARED_GPIO_MAX_BANKS);

    for (i = 0; i < B_SHARED_GPIO_MAX_BANKS; i++)
    {
        init_banks.bank_base_addresses[i] = pSettings->bankBaseAddresses[i];
        init_banks.bank_is_aon[i] = pSettings->aon[i];
    }

    b_shared_gpio_init_banks(&init_banks);

    g_NEXUS_Platform_P_SharedGpioState.initSettings = *pSettings;

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_OpenPin(const NEXUS_GpioModuleOsSharedBankPinOpenSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    b_shared_gpio_pin_desc pin;
    b_shared_gpio_irq_type irq_type;

    pin.bank_base_address = pSettings->bankBaseAddress;
    pin.shift = pSettings->shift;
    irq_type = NEXUS_Platform_P_SharedGpio_NexusInterruptTypeToDriver(pSettings->interruptType);

    u_rc = b_shared_gpio_open_pin(&pin, irq_type);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static void NEXUS_Platform_P_SharedGpio_ClosePin(NEXUS_GpioModuleOsSharedBankPinHandle pin)
{
    b_shared_gpio_pin_desc pinDesc;

    NEXUS_Platform_P_SharedGpio_GetPinDesc_isrsafe(pin, &pinDesc);

    b_shared_gpio_close_pin(&pinDesc);
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_GetInterruptStatus_isr(NEXUS_GpioModuleOsSharedBankInterruptStatus * pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    b_shared_gpio_int_status status;

    BKNI_Memset(&status, 0, sizeof(status));
    b_shared_gpio_get_int_status(&status);
    NEXUS_Platform_P_SharedGpio_DriverInterruptStatusToNexus_isr(&status, pStatus);

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_ClearInterrupt_isr(NEXUS_GpioModuleOsSharedBankPinHandle pin)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    b_shared_gpio_pin_desc pinDesc;

    NEXUS_Platform_P_SharedGpio_GetPinDesc_isrsafe(pin, &pinDesc);

    u_rc = b_shared_gpio_clear_int(&pinDesc);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_SetInterruptMask_isr(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool disable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    b_shared_gpio_pin_desc pinDesc;

    NEXUS_Platform_P_SharedGpio_GetPinDesc_isrsafe(pin, &pinDesc);

    u_rc = b_shared_gpio_set_int_mask(&pinDesc, disable ? 1 : 0);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_SetStandby(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool enable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    b_shared_gpio_pin_desc pinDesc;

    NEXUS_Platform_P_SharedGpio_GetPinDesc_isrsafe(pin, &pinDesc);

    u_rc = b_shared_gpio_set_standby(&pinDesc, enable ? 1 : 0);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

#define NEXUS_PLATFORM_P_SHARED_GPIO_STATE() (&g_NEXUS_Platform_P_SharedGpioState)
#include "../nexus_platform_shared_gpio_common.inc"
