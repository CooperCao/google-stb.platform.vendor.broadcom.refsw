/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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

#include "nexus_dvb_ci_module.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_gpio.h"

BDBG_MODULE(nexus_dvb_ci);

/* Sanity check */
#if BCHP_CHIP != 7364
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

static NEXUS_GpioHandle g_gpioA15, g_gpioA16, g_gpioA17, g_gpioA18;
static NEXUS_GpioHandle g_gpioA19, g_gpioA20, g_gpioA21, g_gpioA22;
static NEXUS_GpioHandle g_gpioA23, g_gpioA24, g_gpioA25;
static NEXUS_GpioHandle g_gpioEBI;

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;
    BREG_Handle hReg = g_pCoreHandles->reg;

    BSTD_UNUSED(handle);

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();
    if ( pcmciaMode )
    {
        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000)
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_001) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_002) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_003) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_004) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_005) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008)
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_009) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_010) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_012) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_013) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016)
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_017) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_018) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_019) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_020) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_021) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_022) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_023)
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, regVal);
    }
    else
    { /*dvb-ci mode*/
        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000)
                );
        regVal |=
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000, CHIP2CI_MDO_0);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
        regVal &= ~(

                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_001) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_002) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_003) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_004) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_005) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008)
                );
        regVal |=
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_001, CHIP2CI_MDO_1) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_002, CHIP2CI_MDO_2) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_003, CHIP2CI_MDO_3) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_004, CHIP2CI_MDO_4) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_005, CHIP2CI_MDO_5) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006, CHIP2CI_MDO_6) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007, CHIP2CI_MDO_7) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008, CHIP2CI_MOSTRT);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_009) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_010) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_012) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_013) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016)
                );
        regVal |=
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_009, CHIP2CI_MOVAL) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_010, CHIP2CI_MCLKO) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_012, CI2CHIP_MISTRT) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_013, CI2CHIP_MIVAL) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014, CI2CHIP_MCLKI) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016, CI2CHIP_MDI_0);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, regVal);

        regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
        regVal &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_017) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_018) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_019) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_020) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_021) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_022) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_023)
                );
        regVal |=
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_017, CI2CHIP_MDI_1) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_018, CI2CHIP_MDI_2) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_019, CI2CHIP_MDI_3) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_020, CI2CHIP_MDI_4) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_021, CI2CHIP_MDI_5) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_022, CI2CHIP_MDI_6) |
                BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_023, CI2CHIP_MDI_7);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, regVal);
    }
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DvbCi_P_InitChip(NEXUS_DvbCiHandle handle)
{
    NEXUS_GpioSettings gpioSettings;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    /* All pins will be setup to drive 0 on unused address lines */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eLow;

    g_gpioA15 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 10, &gpioSettings);
    if ( NULL == g_gpioA15 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA16 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 9, &gpioSettings);
    if ( NULL == g_gpioA16 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA17 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 8, &gpioSettings);
    if ( NULL == g_gpioA17 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA18 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 0, &gpioSettings);
    if ( NULL == g_gpioA18 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA19 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 1, &gpioSettings);
    if ( NULL == g_gpioA19 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA20 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 2, &gpioSettings);
    if ( NULL == g_gpioA20 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA21 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 3, &gpioSettings);
    if ( NULL == g_gpioA21 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA22 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 4, &gpioSettings);
    if ( NULL == g_gpioA22 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA23 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 5, &gpioSettings);
    if ( NULL == g_gpioA23 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA24 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 6, &gpioSettings);
    if ( NULL == g_gpioA24 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA25 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 7, &gpioSettings);
    if ( NULL == g_gpioA25 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}

    g_gpioEBI = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 104, &gpioSettings);
    if ( NULL == g_gpioEBI ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}

    NEXUS_Gpio_SetSettings(g_gpioEBI, &gpioSettings);

    return NEXUS_SUCCESS;
error:
    (void)NEXUS_DvbCi_P_UninitChip(handle);
    return errCode;
}

NEXUS_Error NEXUS_DvbCi_P_UninitChip(NEXUS_DvbCiHandle handle)
{
    BSTD_UNUSED(handle);

    if ( g_gpioA25 ) {NEXUS_Gpio_Close(g_gpioA25); g_gpioA25 = NULL;}
    if ( g_gpioA24 ) {NEXUS_Gpio_Close(g_gpioA24); g_gpioA24 = NULL;}
    if ( g_gpioA23 ) {NEXUS_Gpio_Close(g_gpioA23); g_gpioA23 = NULL;}
    if ( g_gpioA22 ) {NEXUS_Gpio_Close(g_gpioA22); g_gpioA22 = NULL;}
    if ( g_gpioA21 ) {NEXUS_Gpio_Close(g_gpioA21); g_gpioA21 = NULL;}
    if ( g_gpioA20 ) {NEXUS_Gpio_Close(g_gpioA20); g_gpioA20 = NULL;}
    if ( g_gpioA19 ) {NEXUS_Gpio_Close(g_gpioA19); g_gpioA19 = NULL;}
    if ( g_gpioA18 ) {NEXUS_Gpio_Close(g_gpioA18); g_gpioA18 = NULL;}
    if ( g_gpioA17 ) {NEXUS_Gpio_Close(g_gpioA17); g_gpioA17 = NULL;}
    if ( g_gpioA16 ) {NEXUS_Gpio_Close(g_gpioA16); g_gpioA16 = NULL;}
    if ( g_gpioA15 ) {NEXUS_Gpio_Close(g_gpioA15); g_gpioA15 = NULL;}

    if ( g_gpioEBI ) {NEXUS_Gpio_Close(g_gpioEBI); g_gpioEBI = NULL;}

    return NEXUS_SUCCESS;
}
