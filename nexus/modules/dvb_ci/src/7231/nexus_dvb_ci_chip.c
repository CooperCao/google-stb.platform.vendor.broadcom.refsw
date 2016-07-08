/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* API Description:
*   API name: DvbCi
*    Specific APIs related DVB Common Interface
*
***************************************************************************/

#include "nexus_dvb_ci_module.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hif_top_ctrl.h"

BDBG_MODULE(nexus_dvb_ci);

/* Sanity check */
#if BCHP_CHIP != 7231
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

static NEXUS_GpioHandle g_gpioA15, g_gpioA16, g_gpioA17, g_gpioA18;
static NEXUS_GpioHandle g_gpioA19, g_gpioA20, g_gpioA21, g_gpioA22;
static NEXUS_GpioHandle g_gpioA23, g_gpioA24, g_gpioA25;
static NEXUS_GpioHandle g_gpioEBI;

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;
    uint8_t dirtyEnableEBI = 0;
    BREG_Handle hReg = g_pCoreHandles->reg;
	
    BSTD_UNUSED(handle);

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();

    if ( pcmciaMode )
    {
    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_47) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_48) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_49)
            );

    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, regVal);

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52) |    
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_53) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_54) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_55) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_56) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_57)
            );
            
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, regVal);

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60) |    
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_61) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_62) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_63) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_64) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_65)
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, regVal);   
    
    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67)
            );
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, regVal);  

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    regVal &= ~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_97));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, regVal);
    }
    else
    { /*dvb-ci mode*/
    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_47) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_48) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_49)
            );
    regVal |= 
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_47, POD2CHIP_MDI0) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_48, POD2CHIP_MDI1) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_49, POD2CHIP_MDI2);            
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, regVal);
    
    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52) |    
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_53) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_54) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_55) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_56) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_57)
            );
    regVal |= 
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50, POD2CHIP_MDI3) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51, POD2CHIP_MDI4) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52, POD2CHIP_MDI5) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_53, POD2CHIP_MDI6) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_54, POD2CHIP_MDI7) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_55, POD2CHIP_MISTRT) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_56, POD2CHIP_MIVAL) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_57, CHIP2POD_MCLKO);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, regVal);

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60) |    
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_61) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_62) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_63) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_64) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_65)
            );
    regVal |= 
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58, CHIP2POD_MDO0) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59, CHIP2POD_MDO1) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60, CHIP2POD_MDO2) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_61, CHIP2POD_MDO3) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_62, CHIP2POD_MDO4) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_63, CHIP2POD_MDO5) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_64, CHIP2POD_MDO6) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_65, CHIP2POD_MDO7);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, regVal); 

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    regVal &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67)
            );
    regVal |= 
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66, CHIP2POD_MOSTRT) |
           BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67, CHIP2POD_MOVAL);            
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, regVal);  

    regVal = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    regVal &= ~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_97));
    regVal |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_97, POD2CHIP_MCLKI);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, regVal);
    }

    /*This bit must be set active by MIPS boot code to fully use the EBI. 
      Software should wait 500nS after setting this bit before using the EBI.*/
    regVal = BREG_Read32(hReg, BCHP_HIF_TOP_CTRL_HIF_CTRL);
    if((regVal & BCHP_HIF_TOP_CTRL_HIF_CTRL_ENABLE_FULL_USE_OF_EBI_MASK) == 0)
    {
        BDBG_ERR(("ENABLE full use of EBI"));
        dirtyEnableEBI = 1;
        regVal &= ~(BCHP_HIF_TOP_CTRL_HIF_CTRL_ENABLE_FULL_USE_OF_EBI_MASK);
        regVal |= BCHP_FIELD_DATA(HIF_TOP_CTRL_HIF_CTRL, ENABLE_FULL_USE_OF_EBI, 1);
        BREG_Write32(hReg, BCHP_HIF_TOP_CTRL_HIF_CTRL, regVal);
    }    
    BKNI_LeaveCriticalSection();
    if(dirtyEnableEBI)
    {
        BKNI_Sleep(1);
    }
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

    g_gpioA15 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 57, &gpioSettings);
    if ( NULL == g_gpioA15 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA16 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 67, &gpioSettings);
    if ( NULL == g_gpioA16 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA17 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 66, &gpioSettings);
    if ( NULL == g_gpioA17 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA18 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 58, &gpioSettings);
    if ( NULL == g_gpioA18 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA19 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 59, &gpioSettings);
    if ( NULL == g_gpioA19 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA20 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 60, &gpioSettings);
    if ( NULL == g_gpioA20 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA21 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 61, &gpioSettings);
    if ( NULL == g_gpioA21 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA22 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 62, &gpioSettings);
    if ( NULL == g_gpioA22 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA23 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 63, &gpioSettings);
    if ( NULL == g_gpioA23 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA24 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 64, &gpioSettings);
    if ( NULL == g_gpioA24 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA25 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 65, &gpioSettings);
    if ( NULL == g_gpioA25 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
        
    g_gpioEBI = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 124, &gpioSettings);
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

