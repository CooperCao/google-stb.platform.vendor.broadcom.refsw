/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "nexus_dvb_ci_module.h"
#include "priv/nexus_core.h"
#include "bchp_hif_top_ctrl.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_ebi.h"

BDBG_MODULE(nexus_dvb_ebi);

#if !(defined BCHP_EBI_CS_CONFIG_0) && (defined BCHP_EBI_CS_CONFIG_1)
/* the 3556 does not define EBI_CS_CONFIG_0, only 1 */
#define BCHP_EBI_CS_CONFIG_0_mem_io_SHIFT BCHP_EBI_CS_CONFIG_1_mem_io_SHIFT
#define BCHP_EBI_CS_CONFIG_0_ta_wait_SHIFT BCHP_EBI_CS_CONFIG_1_ta_wait_SHIFT
#define BCHP_EBI_CS_CONFIG_0_wait_count_SHIFT BCHP_EBI_CS_CONFIG_1_wait_count_SHIFT
#define BCHP_EBI_CS_CONFIG_0_t_hold_SHIFT BCHP_EBI_CS_CONFIG_1_t_hold_SHIFT
#define BCHP_EBI_CS_CONFIG_0_t_setup_SHIFT BCHP_EBI_CS_CONFIG_1_t_setup_SHIFT
#define BCHP_EBI_CS_CONFIG_0_cs_hold_SHIFT BCHP_EBI_CS_CONFIG_1_cs_hold_SHIFT
#define BCHP_EBI_CS_CONFIG_0_split_cs_SHIFT BCHP_EBI_CS_CONFIG_1_split_cs_SHIFT
#define BCHP_EBI_CS_CONFIG_0_le_SHIFT BCHP_EBI_CS_CONFIG_1_le_SHIFT
#define BCHP_EBI_CS_CONFIG_0_enable_SHIFT BCHP_EBI_CS_CONFIG_1_enable_SHIFT
#define BCHP_EBI_CS_CONFIG_0_we_ctl_SHIFT BCHP_EBI_CS_CONFIG_1_we_ctl_SHIFT
#endif

static void NEXUS_DvbCi_P_SetFpgaTristate(NEXUS_DvbCiHandle handle, bool tristate);
static void NEXUS_DvbCi_P_SetFpgaPcmciaMode(NEXUS_DvbCiHandle handle, bool pcmciaMode);
static void NEXUS_DvbCi_P_ExternalIrq_isr(void *pParam, int interruptNum);
static void NEXUS_DvbCi_P_SetExtIrqPolarity_isr(unsigned irqNum, NEXUS_DvbCiPinState state);
static unsigned NEXUS_DvbCi_P_GetL1FromExtIrq(unsigned irqNum);
static void NEXUS_DvbCi_P_DisableExtIrq(NEXUS_DvbCiHandle handle, unsigned extIrqNum, NEXUS_IsrCallbackHandle *pCallback);
static void NEXUS_DvbCi_P_EnableExtIrq(NEXUS_DvbCiHandle handle, unsigned extIrqNum, NEXUS_IsrCallbackHandle *pCallback, NEXUS_DvbCiPinState pinState, NEXUS_CallbackDesc *pCallbackDesc);

bool NEXUS_DvbCi_P_CardPresent(NEXUS_DvbCiHandle handle)
{
    NEXUS_GpioStatus statusCd1;

    NEXUS_Gpio_GetStatus(handle->openSettings.cd1Pin, &statusCd1);
    if ( NULL != handle->openSettings.cd2Pin )
    {
        NEXUS_GpioStatus statusCd2;

        NEXUS_Gpio_GetStatus(handle->openSettings.cd2Pin, &statusCd2);
        BDBG_MSG(("Checking card presence: CD1=%s, CD2=%s",
                  statusCd1.value==NEXUS_GpioValue_eLow?"low":"high",
                  statusCd2.value==NEXUS_GpioValue_eLow?"low":"high"));
        return (statusCd1.value == NEXUS_GpioValue_eLow) && (statusCd2.value == NEXUS_GpioValue_eLow);
    }
    BDBG_MSG(("Checking card presence: CD1=%s",
              statusCd1.value==NEXUS_GpioValue_eLow?"low":"high"));
    return statusCd1.value == NEXUS_GpioValue_eLow;
}

NEXUS_Error NEXUS_DvbCi_P_SetupInterface(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t configAddr, baseAddr;
    uint32_t configValue;
#if (BCHP_CHIP == 7278)
    uint64_t baseValue;
#else
    uint32_t baseValue;
#endif

    switch ( handle->openSettings.ebiChipSelect )
    {
    #ifdef BCHP_EBI_CS_CONFIG_0
    case 0:
        baseAddr = BCHP_EBI_CS_BASE_0;
        configAddr = BCHP_EBI_CS_CONFIG_0;
        break;
    #endif
    #ifdef BCHP_EBI_CS_CONFIG_1
    case 1:
        baseAddr = BCHP_EBI_CS_BASE_1;
        configAddr = BCHP_EBI_CS_CONFIG_1;
        break;
    #endif
    #ifdef BCHP_EBI_CS_CONFIG_2
    case 2:
        baseAddr = BCHP_EBI_CS_BASE_2;
        configAddr = BCHP_EBI_CS_CONFIG_2;
        break;
    #endif
    #ifdef BCHP_EBI_CS_CONFIG_3
    case 3:
        baseAddr = BCHP_EBI_CS_BASE_3;
        configAddr = BCHP_EBI_CS_CONFIG_3;
        break;
    #endif
    #ifdef BCHP_EBI_CS_CONFIG_4
    case 4:
        baseAddr = BCHP_EBI_CS_BASE_4;
        configAddr = BCHP_EBI_CS_CONFIG_4;
        break;
    #endif
    #ifdef BCHP_EBI_CS_CONFIG_5
    case 5:
        baseAddr = BCHP_EBI_CS_BASE_5;
        configAddr = BCHP_EBI_CS_CONFIG_5;
        break;
    #endif
    default:
        BDBG_ERR(("Invalid EBI Chip Select %u", handle->openSettings.ebiChipSelect));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check for invalid addresses */
    if ( handle->openSettings.deviceOffset & ~BCHP_EBI_CS_BASE_0_base_addr_MASK )
    {
        BDBG_ERR(("Invalid base address specified."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( pcmciaMode )
    {
        BDBG_MSG(("Configuring EBI CS for MEM space"));
        /* Setup for 1MB of attribute memory */
        baseValue = handle->openSettings.deviceOffset | BCHP_FIELD_ENUM(EBI_CS_BASE_0, size, SIZE_1MB);
        configValue = BCHP_FIELD_DATA(EBI_CS_CONFIG_0, mem_io, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, ta_wait, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, wait_count, 0x18) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, t_hold, 7) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, t_setup, 4) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, cs_hold, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, split_cs, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, enable, 1);
    }
    else
    {
        BDBG_MSG(("Configuring EBI CS for IO space"));
        /* Setup for 8KB of IO space (smallest possible size) */
        baseValue = handle->openSettings.deviceOffset | BCHP_FIELD_ENUM(EBI_CS_BASE_0, size, SIZE_8KB);
        configValue = BCHP_FIELD_DATA(EBI_CS_CONFIG_0, mem_io, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, ta_wait, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, wait_count, 0x18) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, t_hold, 7) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, t_setup, 4) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, cs_hold, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, split_cs, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, we_ctl, 1) |
                      BCHP_FIELD_DATA(EBI_CS_CONFIG_0, enable, 1);
    }

    if ( handle->openSettings.littleEndian )
    {
        configValue |= BCHP_FIELD_DATA(EBI_CS_CONFIG_0, le, 1);
    }
#if (BCHP_CHIP == 7278)
    BREG_Write64(g_pCoreHandles->reg, baseAddr, baseValue);
#else
    BREG_Write32(g_pCoreHandles->reg, baseAddr, baseValue);
#endif
    BREG_Write32(g_pCoreHandles->reg, configAddr, configValue);

    /* Config transport stream pinmuxes -- this is chip-specific, so it's handled in a separate file. */
    NEXUS_DvbCi_P_SetupPinmuxes(handle, pcmciaMode);

    /* Enable/Disable EBI bus driver if necessary */
    if ( handle->openSettings.ebiEnablePin )
    {
        NEXUS_GpioSettings gpioSettings;
        NEXUS_Gpio_GetSettings(handle->openSettings.ebiEnablePin, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
#if (BCHP_CHIP == 7435) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7366) || (BCHP_CHIP == 7445) || (BCHP_CHIP == 7252) || (BCHP_CHIP == 7364) || (BCHP_CHIP == 7439) ||(BCHP_CHIP == 7260) \
    ||(BCHP_CHIP == 7268) || (BCHP_CHIP == 7271) || (BCHP_CHIP == 7278)
        gpioSettings.value = handle->openSettings.ebiEnableValue;
#else
        gpioSettings.value = (pcmciaMode) ? handle->openSettings.ebiEnableValue : !handle->openSettings.ebiEnableValue;
#endif
        NEXUS_Gpio_SetSettings(handle->openSettings.ebiEnablePin, &gpioSettings);
    }
    handle->pcmciaMode = pcmciaMode;

    if ( handle->openSettings.fpgaI2c )
    {
        NEXUS_DvbCi_P_SetFpgaPcmciaMode(handle, pcmciaMode);
    }

    return BERR_SUCCESS;
}

void NEXUS_DvbCi_P_SetVpp(NEXUS_DvbCiHandle handle, NEXUS_DvbCiVpp vpp)
{
    NEXUS_GpioSettings gpioSettings;

    /* If 3.3v and 5 are the only options, use 3.3v for removed */
    if ( vpp == NEXUS_DvbCiVpp_eTristate && NULL == handle->openSettings.vpp0EnablePin1 )
    {
        vpp = NEXUS_DvbCiVpp_e3v;
    }

    if( NULL != handle->openSettings.vpp0EnablePin0 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vpp0EnablePin0, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vpp )
        {
        default:
        case NEXUS_DvbCiVpp_e3v:
            gpioSettings.value = (handle->openSettings.vpp03vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_e5v:
            gpioSettings.value = (handle->openSettings.vpp05vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_eTristate:
            gpioSettings.value = (handle->openSettings.vpp0tristateValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vpp0EnablePin0, &gpioSettings);
    }

    if ( NULL != handle->openSettings.vpp0EnablePin1 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vpp0EnablePin1, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vpp )
        {
        default:
        case NEXUS_DvbCiVpp_e3v:
            gpioSettings.value = (handle->openSettings.vpp03vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_e5v:
            gpioSettings.value = (handle->openSettings.vpp05vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_eTristate:
            gpioSettings.value = (handle->openSettings.vpp0tristateValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vpp0EnablePin1, &gpioSettings);
    }

    if ( NULL != handle->openSettings.vpp1EnablePin0 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vpp1EnablePin0, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vpp )
        {
        default:
        case NEXUS_DvbCiVpp_e3v:
            gpioSettings.value = (handle->openSettings.vpp13vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_e5v:
            gpioSettings.value = (handle->openSettings.vpp15vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_eTristate:
            gpioSettings.value = (handle->openSettings.vpp1tristateValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vpp1EnablePin0, &gpioSettings);
    }

    if ( NULL != handle->openSettings.vpp1EnablePin1 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vpp1EnablePin1, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vpp )
        {
        default:
        case NEXUS_DvbCiVpp_e3v:
            gpioSettings.value = (handle->openSettings.vpp13vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_e5v:
            gpioSettings.value = (handle->openSettings.vpp15vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVpp_eTristate:
            gpioSettings.value = (handle->openSettings.vpp1tristateValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vpp1EnablePin1, &gpioSettings);
    }
}

void NEXUS_DvbCi_P_SetVcc(NEXUS_DvbCiHandle handle, NEXUS_DvbCiVcc vcc)
{
    NEXUS_GpioSettings gpioSettings;

    /* If 3.3v and 5 are the only options, use 3.3v for removed */
    if ( vcc == NEXUS_DvbCiVcc_eTristate && NULL == handle->openSettings.vccEnablePin1 )
    {
        vcc = NEXUS_DvbCiVcc_e3v;
    }

    if ( NULL != handle->openSettings.vccEnablePin0 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vccEnablePin0, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vcc )
        {
        default:
        case NEXUS_DvbCiVcc_e3v:
            gpioSettings.value = (handle->openSettings.vcc3vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVcc_e5v:
            gpioSettings.value = (handle->openSettings.vcc5vValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVcc_eTristate:
            gpioSettings.value = (handle->openSettings.vcctristateValue & 1)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vccEnablePin0, &gpioSettings);
    }

    if ( NULL != handle->openSettings.vccEnablePin1 )
    {
        NEXUS_Gpio_GetSettings(handle->openSettings.vccEnablePin1, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        switch ( vcc )
        {
        default:
        case NEXUS_DvbCiVcc_e3v:
            gpioSettings.value = (handle->openSettings.vcc3vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVcc_e5v:
            gpioSettings.value = (handle->openSettings.vcc5vValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        case NEXUS_DvbCiVcc_eTristate:
            gpioSettings.value = (handle->openSettings.vcctristateValue & 2)?NEXUS_GpioValue_eHigh:NEXUS_GpioValue_eLow;
            break;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.vccEnablePin1, &gpioSettings);
    }
}

NEXUS_Error NEXUS_DvbCi_P_GetVoltageSense(NEXUS_DvbCiHandle handle, NEXUS_DvbCiPinState *pVs1State, NEXUS_DvbCiPinState *pVs2State)
{
    BDBG_ASSERT(NULL != pVs1State);
    BDBG_ASSERT(NULL != pVs2State);

    if ( handle->openSettings.vs1Pin )
    {
        /* Probe VS# pins to determine if 5V or 3V is required.  5V is only required if both vs1 and vs2 read high */
        NEXUS_GpioStatus status;
        NEXUS_Gpio_GetStatus(handle->openSettings.vs1Pin, &status);
        *pVs1State = (status.value)?NEXUS_DvbCiPinState_eHigh:NEXUS_DvbCiPinState_eLow;
        if ( handle->openSettings.vs2Pin )
        {
            NEXUS_Gpio_GetStatus(handle->openSettings.vs2Pin, &status);
            *pVs2State = (status.value)?NEXUS_DvbCiPinState_eHigh:NEXUS_DvbCiPinState_eLow;
        }
        else
        {
            *pVs2State = NEXUS_DvbCiPinState_eUnknown;
        }
        return BERR_SUCCESS;
    }
    else
    {
        return BERR_NOT_SUPPORTED;
    }
}

void NEXUS_DvbCi_P_EnableReadyInterrupt(NEXUS_DvbCiHandle handle, bool enable, NEXUS_CallbackDesc *pCallback)
{
    if ( handle->openSettings.readyPin )
    {
        NEXUS_GpioSettings gpioSettings;
        /* Setup level interrupt to catch ready being asserted */
        NEXUS_Gpio_GetSettings(handle->openSettings.readyPin, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        if ( enable )
        {
            BDBG_WRN(("Enabling READY interrupt"));
            BDBG_ASSERT(NULL != pCallback);
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eHigh;
            gpioSettings.interrupt = *pCallback;
        }
        else
        {
            BDBG_WRN(("Disabling READY/IREQ# interrupt"));
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
            gpioSettings.interrupt.callback = NULL;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.readyPin, &gpioSettings);
        if ( enable )
            NEXUS_Gpio_ClearInterrupt(handle->openSettings.readyPin);
    }
    else
    {
        if ( enable )
        {
            NEXUS_DvbCi_P_EnableExtIrq(handle, handle->openSettings.readyInterrupt, &handle->ireqIrqCallback, NEXUS_DvbCiPinState_eHigh, pCallback);
        }
        else
        {
            NEXUS_DvbCi_P_DisableExtIrq(handle, handle->openSettings.readyInterrupt, &handle->ireqIrqCallback);
        }
    }
}

void NEXUS_DvbCi_P_EnableIreqInterrupt(NEXUS_DvbCiHandle handle, bool enable, NEXUS_CallbackDesc *pCallback)
{
    if ( handle->openSettings.readyPin )
    {
        NEXUS_GpioSettings gpioSettings;

        /* Setup level interrupt to catch ready being asserted */
        NEXUS_Gpio_GetSettings(handle->openSettings.readyPin, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        if ( enable )
        {
            BDBG_MSG(("Enabling IREQ# interrupt"));
            BDBG_ASSERT(NULL != pCallback);
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            gpioSettings.interrupt = *pCallback;
        }
        else
        {
            BDBG_MSG(("Disabling READY/IREQ# interrupt"));
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
            gpioSettings.interrupt.callback = NULL;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.readyPin, &gpioSettings);
        if ( enable )
            NEXUS_Gpio_ClearInterrupt(handle->openSettings.readyPin);
    }
    else
    {
        if ( enable )
        {
            NEXUS_DvbCi_P_EnableExtIrq(handle, handle->openSettings.readyInterrupt, &handle->ireqIrqCallback, NEXUS_DvbCiPinState_eLow, pCallback);
        }
        else
        {
            NEXUS_DvbCi_P_DisableExtIrq(handle, handle->openSettings.readyInterrupt, &handle->ireqIrqCallback);
        }
    }
}

void NEXUS_DvbCi_P_EnableCardDetectInterrupt(NEXUS_DvbCiHandle handle, bool enable, bool insertion, NEXUS_CallbackDesc *pCallback)
{
    NEXUS_GpioSettings gpioSettings;

    if ( handle->openSettings.cd1Pin )
    {
        /* Set polarity of interrupt pins correctly */
        NEXUS_Gpio_GetSettings(handle->openSettings.cd1Pin, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        if ( enable )
        {
            BDBG_ASSERT(NULL != pCallback);
            if ( insertion )
            {
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            }
            else
            {
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eHigh;
            }
            gpioSettings.interrupt = *pCallback;
        }
        else
        {
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.cd1Pin, &gpioSettings);
        if ( enable )
        {
            NEXUS_Gpio_ClearInterrupt(handle->openSettings.cd1Pin);
        }
    }
    else
    {
        if ( enable )
        {
            NEXUS_DvbCi_P_EnableExtIrq(handle, handle->openSettings.cd1Interrupt, &handle->cd1IrqCallback,
                                       insertion?NEXUS_DvbCiPinState_eLow:NEXUS_DvbCiPinState_eHigh, pCallback);
        }
        else
        {
            NEXUS_DvbCi_P_DisableExtIrq(handle, handle->openSettings.cd1Interrupt, &handle->cd1IrqCallback);
        }
    }

    if ( handle->openSettings.cd2Pin )
    {
        /* Set polarity of interrupt pins correctly */
        NEXUS_Gpio_GetSettings(handle->openSettings.cd2Pin, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        if ( enable )
        {
            BDBG_ASSERT(NULL != pCallback);
            if ( insertion )
            {
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            }
            else
            {
                gpioSettings.interruptMode = NEXUS_GpioInterrupt_eHigh;
            }
            gpioSettings.interrupt = *pCallback;
        }
        else
        {
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        }
        NEXUS_Gpio_SetSettings(handle->openSettings.cd2Pin, &gpioSettings);
        if ( enable )
        {
            NEXUS_Gpio_ClearInterrupt(handle->openSettings.cd2Pin);
        }
    }
    else if ( handle->openSettings.cd2Interrupt != 0xFFFFFFFF )
    {
        if ( enable )
        {
            NEXUS_DvbCi_P_EnableExtIrq(handle, handle->openSettings.cd2Interrupt, &handle->cd2IrqCallback,
                                       insertion?NEXUS_DvbCiPinState_eLow:NEXUS_DvbCiPinState_eHigh, pCallback);
        }
        else
        {
            NEXUS_DvbCi_P_DisableExtIrq(handle, handle->openSettings.cd2Interrupt, &handle->cd2IrqCallback);
        }
    }
}

void NEXUS_DvbCi_P_WriteByte(NEXUS_DvbCiHandle handle, uint16_t address, uint8_t value)
{
    bool status;

    status = NEXUS_DvbCi_P_CardPresent(handle);

    if ( status )
    {
        ((uint8_t *)handle->pBaseAddress)[address] = value;
    }
}

uint8_t NEXUS_DvbCi_P_ReadByte(NEXUS_DvbCiHandle handle, uint16_t address)
{
    bool status;
    uint8_t value=0;

    status = NEXUS_DvbCi_P_CardPresent(handle);

    if ( status )
    {
        value = ((uint8_t *)handle->pBaseAddress)[address];
    }

    return value;
}

void NEXUS_DvbCi_P_SetResetState(NEXUS_DvbCiHandle handle, NEXUS_DvbCiPinState state)
{
    NEXUS_GpioSettings gpioSettings;

    if ( handle->openSettings.fpgaI2c )
    {
        NEXUS_DvbCi_P_SetFpgaTristate(handle, (state == NEXUS_DvbCiPinState_eTristate)?true:false);
    }

    NEXUS_Gpio_GetSettings(handle->openSettings.resetPin, &gpioSettings);
    switch ( state )
    {
    default:
        BDBG_ERR(("Unsupported pin state.  Tristating reset signal."));
        /* Fall through */
    case NEXUS_DvbCiPinState_eTristate:
        gpioSettings.mode = NEXUS_GpioMode_eInput;
        break;
    case NEXUS_DvbCiPinState_eLow:
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = 0;
        break;
    case NEXUS_DvbCiPinState_eHigh:
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = 1;
        break;
    }
    (void)NEXUS_Gpio_SetSettings(handle->openSettings.resetPin, &gpioSettings);
}

static void NEXUS_DvbCi_P_SetFpgaState(NEXUS_DvbCiHandle handle)
{
    if ( handle->openSettings.fpgaI2c )
    {
        uint8_t data;
        if ( handle->fpgaTristate )
        {
            data = 0;
        }
        else if ( handle->fpgaPcmciaMode )
        {
            data = 1;
        }
        else
        {
            data = 3;
        }
        NEXUS_I2c_Write(handle->openSettings.fpgaI2c, handle->openSettings.fpgaI2cAddr, 0, &data, 1);
    }
}

static void NEXUS_DvbCi_P_SetFpgaTristate(NEXUS_DvbCiHandle handle, bool tristate)
{
    handle->fpgaTristate = tristate;
    NEXUS_DvbCi_P_SetFpgaState(handle);
}

static void NEXUS_DvbCi_P_SetFpgaPcmciaMode(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    handle->fpgaPcmciaMode = pcmciaMode;
    NEXUS_DvbCi_P_SetFpgaState(handle);
}

static void NEXUS_DvbCi_P_ExternalIrq_isr(void *pParam, int interruptNum)
{
    /* Interrupts must be re-enabled later */
    NEXUS_Core_DisableInterrupt_isr(interruptNum);
    NEXUS_IsrCallback_Fire_isr(pParam);
}

static void NEXUS_DvbCi_P_SetExtIrqPolarity_isr(unsigned irqNum, NEXUS_DvbCiPinState state)
{
    uint32_t reg = BREG_Read32_isr(g_pCoreHandles->reg, BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL);
    uint32_t mask=0;

    switch ( irqNum )
    {
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_MASK
    case 0: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_0_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_MASK
    case 1: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_1_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_MASK
    case 2: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_2_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_MASK
    case 3: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_3_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_MASK
    case 4: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_4_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_MASK
    case 5: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_MASK
    case 6: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_6_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_MASK
    case 7: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_7_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_MASK
    case 8: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_8_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_MASK
    case 9: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_9_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_MASK
    case 10: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_MASK
    case 11: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_11_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_MASK
    case 12: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_12_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_MASK
    case 13: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_13_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_MASK
    case 14: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_14_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_15_level_MASK
    case 15: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_15_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_16_level_MASK
    case 16: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_16_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_17_level_MASK
    case 17: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_17_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_18_level_MASK
    case 18: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_18_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_19_level_MASK
    case 19: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_19_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_20_level_MASK
    case 20: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_20_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_21_level_MASK
    case 21: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_21_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_22_level_MASK
    case 22: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_22_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_23_level_MASK
    case 23: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_23_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_24_level_MASK
    case 24: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_24_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_25_level_MASK
    case 25: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_25_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_26_level_MASK
    case 26: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_26_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_27_level_MASK
    case 27: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_27_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_28_level_MASK
    case 28: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_28_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_29_level_MASK
    case 29: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_29_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_30_level_MASK
    case 30: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_30_level_MASK; break;
    #endif
    #ifdef BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_31_level_MASK
    case 31: mask = BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_31_level_MASK; break;
    #endif
    default:
        BDBG_ERR(("Invalid external interrupt %d", irqNum));
        break;
    }

    if ( state == NEXUS_DvbCiPinState_eHigh )
    {
        reg |= mask;
    }
    else
    {
        reg &= ~mask;
    }

    BREG_Write32_isr(g_pCoreHandles->reg, BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL, reg);
}

static unsigned NEXUS_DvbCi_P_GetL1FromExtIrq(unsigned irqNum)
{
    /* Ugh, this mapping is very difficult to make generic from chip to chip */
    switch ( irqNum )
    {
#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT
    case 0: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT
    case 0: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT
    case 0: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_0_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT
    case 1: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT
    case 1: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT
    case 1: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_1_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT
    case 2: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT
    case 2: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT
    case 2: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_2_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT
    case 3: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT
    case 3: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT
    case 3: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_3_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT
    case 4: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT
    case 4: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT
    case 4: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_4_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT
    case 5: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT
    case 5: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT
    case 5: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_5_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT
    case 6: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT
    case 6: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT
    case 6: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_6_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT
    case 7: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT
    case 7: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT
    case 7: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_7_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT
    case 8: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT
    case 8: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT
    case 8: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_8_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT
    case 9: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT
    case 9: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT
    case 9: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_9_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT
    case 10: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT
    case 10: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT
    case 10: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_10_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT
    case 11: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT
    case 11: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT
    case 11: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_11_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT
    case 12: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT
    case 12: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT
    case 12: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_12_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT
    case 13: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT
    case 13: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT
    case 13: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_13_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT
    case 14: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT
    case 14: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT
    case 14: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_14_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT
    case 15: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT
    case 15: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT
    case 15: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_15_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT
    case 16: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT
    case 16: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT
    case 16: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_16_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT
    case 17: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT
    case 17: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT
    case 17: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_17_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT
    case 18: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT
    case 18: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT
    case 18: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_18_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT
    case 19: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT
    case 19: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT
    case 19: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_19_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT
    case 20: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT
    case 20: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT
    case 20: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_20_CPU_INTR_SHIFT+ 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT
    case 21: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT
    case 21: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT
    case 21: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_21_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT
    case 22: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT
    case 22: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT
    case 22: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_22_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT
    case 23: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT
    case 23: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT
    case 23: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_23_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT
    case 24: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT
    case 24: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT
    case 24: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_24_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT
    case 25: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT
    case 25: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT
    case 25: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_25_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT
    case 26: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT
    case 26: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT
    case 26: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_26_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT
    case 27: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT
    case 27: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT
    case 27: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_27_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT
    case 28: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT
    case 28: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT
    case 28: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_28_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT
    case 29: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT
    case 29: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT
    case 29: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_29_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT
    case 30: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT
    case 30: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT
    case 30: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_30_CPU_INTR_SHIFT + 64;
#endif

#if   defined BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT
    case 31: return BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT
    case 31: return BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT + 32;
#elif defined BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT
    case 31: return BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_EXT_IRQ_31_CPU_INTR_SHIFT + 64;
#endif

    default:
        BDBG_ERR(("Invalid EXT IRQ number %d", irqNum));
        return 0xffffffffUL;
    }
}

static void NEXUS_DvbCi_P_DisableExtIrq(NEXUS_DvbCiHandle handle, unsigned extIrqNum, NEXUS_IsrCallbackHandle *pCallback)
{
    int l1Num = NEXUS_DvbCi_P_GetL1FromExtIrq(extIrqNum);
    if ( *pCallback != NULL )
    {
        BKNI_EnterCriticalSection();
        NEXUS_Core_DisableInterrupt_isr(l1Num);
        BKNI_LeaveCriticalSection();
        NEXUS_Core_DisconnectInterrupt(l1Num);
        NEXUS_IsrCallback_Destroy(*pCallback);
        *pCallback = NULL;
    }
    BSTD_UNUSED(handle);
}

static void NEXUS_DvbCi_P_EnableExtIrq(NEXUS_DvbCiHandle handle, unsigned extIrqNum, NEXUS_IsrCallbackHandle *pCallback, NEXUS_DvbCiPinState pinState, NEXUS_CallbackDesc *pCallbackDesc)
{
    int l1Num = NEXUS_DvbCi_P_GetL1FromExtIrq(extIrqNum);
    if ( *pCallback != NULL )
    {
        NEXUS_DvbCi_P_DisableExtIrq(handle, extIrqNum, pCallback);
    }
    *pCallback = NEXUS_IsrCallback_Create(handle, NULL);
    BDBG_ASSERT(NULL != *pCallback);
    NEXUS_IsrCallback_Set(*pCallback, pCallbackDesc);
    NEXUS_Core_ConnectInterrupt(l1Num, NEXUS_DvbCi_P_ExternalIrq_isr, *pCallback, l1Num);
    BKNI_EnterCriticalSection();
    /* Set Interrupt Polarity */
    NEXUS_DvbCi_P_SetExtIrqPolarity_isr(extIrqNum, pinState);
    /* Enable L1 */
    NEXUS_Core_EnableInterrupt_isr(l1Num);
    BKNI_LeaveCriticalSection();
}


