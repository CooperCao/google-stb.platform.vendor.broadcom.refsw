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
#include "nexus_gpio_init.h"
#include "../nexus_platform_shared_gpio_common.h"
#include "bcm_driver.h"

static NEXUS_Platform_P_SharedGpioState g_NEXUS_Platform_P_SharedGpioState;

void NEXUS_Platform_P_InitSharedGpioSubmodule(void)
{
    int u_rc;
    bcmdriver_os_config os_cfg;

    BKNI_Memset(&os_cfg, 0, sizeof(os_cfg));
    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_GET_OS_CONFIG, &os_cfg);
    if (u_rc) {
        BDBG_WRN(("Nexus requires a new bcmdriver.ko ioctl API BRCM_IOCTL_GET_OS_CONFIG. Are you running with an old bcmdriver.ko?"));
    }
    g_NEXUS_Platform_P_SharedGpioState.featureSupported = os_cfg.shared_gpio_supported == 1;
    BDBG_WRN(("shared gpio %s", g_NEXUS_Platform_P_SharedGpioState.featureSupported ? "enabled" : "disabled"));
}

void NEXUS_Platform_P_UninitSharedGpioSubmodule(void)
{
    /* do nothing */
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_Init(const NEXUS_GpioModuleOsSharedBankInitSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;
    unsigned i;

    BDBG_CASSERT(NEXUS_GPIO_NUM_BANKS == B_SHARED_GPIO_MAX_BANKS);

    control.command = bcmdriver_shared_gpio_command_init_banks;
    for (i = 0; i < B_SHARED_GPIO_MAX_BANKS; i++)
    {
        control.data.init_banks.bank_base_addresses[i] = pSettings->bankBaseAddresses[i];
        control.data.init_banks.bank_is_aon[i] = pSettings->aon[i];
    }

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

    g_NEXUS_Platform_P_SharedGpioState.initSettings = *pSettings;

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_OpenPin(const NEXUS_GpioModuleOsSharedBankPinOpenSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_open_pin;
    control.data.open_pin.pin.bank_base_address = pSettings->bankBaseAddress;
    control.data.open_pin.pin.shift = pSettings->shift;
    control.data.open_pin.irq_type = NEXUS_Platform_P_SharedGpio_NexusInterruptTypeToDriver(pSettings->interruptType);

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static void NEXUS_Platform_P_SharedGpio_ClosePin(NEXUS_GpioModuleOsSharedBankPinHandle pin)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_close_pin;
    NEXUS_Platform_P_SharedGpio_GetPinDesc(pin, &control.data.close_pin.pin);

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_GetInterruptStatus_isr(NEXUS_GpioModuleOsSharedBankInterruptStatus * pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_get_int_status;
    BKNI_Memset(&control.data.get_int_status, 0, sizeof(control.data.get_int_status));

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

    NEXUS_Platform_P_SharedGpio_DriverInterruptStatusToNexus_isr(&control.data.get_int_status, pStatus);

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_ClearInterrupt_isr(NEXUS_GpioModuleOsSharedBankPinHandle pin)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_clear_int;
    NEXUS_Platform_P_SharedGpio_GetPinDesc(pin, &control.data.clear_int.pin);

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_SetInterruptMask_isr(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool disable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_set_int_mask;
    NEXUS_Platform_P_SharedGpio_GetPinDesc(pin, &control.data.set_int_mask.pin);
    control.data.set_int_mask.disable = disable ? 1 : 0;

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SharedGpio_SetStandby(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool enable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_shared_gpio_control control;

    control.command = bcmdriver_shared_gpio_command_set_standby;
    NEXUS_Platform_P_SharedGpio_GetPinDesc(pin, &control.data.set_standby.pin);
    control.data.set_standby.enable = enable ? 1 : 0;

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SHARED_GPIO_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto err; }

err:
    return rc;
}

#define NEXUS_PLATFORM_P_SHARED_GPIO_STATE() (&g_NEXUS_Platform_P_SharedGpioState)
#include "../nexus_platform_shared_gpio_common.inc"
