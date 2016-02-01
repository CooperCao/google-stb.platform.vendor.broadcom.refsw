/***************************************************************************
*     (c)2004-2009 Broadcom Corporation
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: DvbCi
*    Specific APIs related DVB Common Interface
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_dvb_ci_module.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_gpio.h"

BDBG_MODULE(nexus_dvb_ci);

/* Sanity check */
#if BCHP_CHIP != 7342
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

static NEXUS_GpioHandle g_gpioA15, g_gpioA16, g_gpioA17, g_gpioA18;
static NEXUS_GpioHandle g_gpioA19, g_gpioA20, g_gpioA21, g_gpioA22;
static NEXUS_GpioHandle g_gpioA23, g_gpioA24, g_gpioA25;
static NEXUS_GpioHandle g_gpioEBI;

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;

    /* Only one interface is supported on the 7325 */
    BSTD_UNUSED(handle);

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();

    if ( pcmciaMode )
    {
        /* From the schematic, the 7325 has limited EBI addressing for the CI card.  A15..A25 are not connected, nor is D8..D15 
           Switch these pins to GPIO for this mode, address pins should be driven low because there are no pull down resistors */
           
        /* Chip to CI */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_071_MASK|          /* MDO_4/D12 -> D12 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_070_MASK|         /* MDO_3/D11 -> D11 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_069_MASK|         /* MDO_2/D10 -> D10 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_068_MASK|         /* MDO_1/D9-> D9 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_067_MASK|         /* MDO_0/D8 -> D8 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_066_MASK|         /* MOSTRT/BVD1 -> BVD1 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_065_MASK);         /* MOVAL/BVD2 -> BVD21 (ignored) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_074_MASK|          /* MDO_7/D15 -> D15 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_073_MASK|          /* MDO_6/D14 -> D14 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_072_MASK);          /* MDO_5/D13 -> D13 (ignored) */
                 
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_083_MASK);   /* MCLKO/VS2 -> VS2  (ignored) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
		
        /* CI to Chip */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_055_MASK|          /* MIVAL/A16 -> A16 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_054_MASK);         /* MISTRT/A17 -> A17 (GPIO - must drive to 0) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_063_MASK|          /* MDI_7/A25 -> A25 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_062_MASK|          /* MDI_6/A24 -> A24 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_061_MASK|          /* MDI_5/A23 -> A23 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_060_MASK|          /* MDI_4/A22 -> A22 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_059_MASK|          /* MDI_3/A21 -> A21 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_058_MASK|          /* MDI_2/A20 -> A20 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_057_MASK|          /* MDI_1/A19 -> A19 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_056_MASK);         /* MDI_0/A18 -> A18 (GPIO - must drive to 0) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_082_MASK);   /* MCLKI/A15 -> A15  (ignored) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
    }
    else
    {
        /* Chip to CI */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_071_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_070_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_069_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_068_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_067_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_066_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_065_MASK);

        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071, 1) |         /* MDO_4/D12 -> MDO_4 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070, 1) |         /* MDO_3/D11 -> MDO_3 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069, 1) |         /* MDO_2/D10 -> MDO_2 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_068, 1) |         /* MDO_1/D9 -> MDO_1 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_067, 1) |         /* MDO_0/D8 -> MDO_0 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_066, 1) |         /* MOVAL/BVD2 -> MOVAL */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_065, 1));          /* MOSTRT/BVD1 -> MOSTRT */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_074_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_073_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_072_MASK);
                    
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_074, 1) |         /* MDO_7/D15 -> MDO_7 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_073, 1) |         /* MDO_6/D14 -> MDO_6 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_072, 1));          /* MDO_5/D13 -> MDO_5 */
                 
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_083_MASK);

        regVal |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_083, 2);       /* MCLKO/VS2 -> MCLKO   */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);

        /* CI to Chip */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_055_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_054_MASK);
		
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_055, 1) |         /* MIVAL/A16 -> MIVAL */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_054, 1));         /* MISTRT/A17  -> MISTRT */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_063_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_062_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_061_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_060_MASK| 
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_059_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_058_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_057_MASK| 
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_056_MASK);
		
        regVal |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063, 1) |         /* MDI_7/A25 -> MDI_7 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062, 1) |         /* MDI_6/A24 -> MDI_6 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_061, 1) |         /* MDI_5/A23 -> MDI_5 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_060, 1) |         /* MDI_4/A22 -> MDI_4 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_059, 1) |         /* MDI_3/A21 -> MDI_3 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_058, 1) |         /* MDI_2/A20 -> MDI_2 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_057, 1) |         /* MDI_1/A19 -> MDI_1 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_056, 1);           /* MDI_0/A18 -> MDI_0 */		

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_082_MASK);

        regVal |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_082, 2);       /* MCLKI/A15 -> MCLKI   */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
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

    g_gpioA15 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 83, &gpioSettings);
    if ( NULL == g_gpioA15 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA16 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 65, &gpioSettings);
    if ( NULL == g_gpioA16 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA17 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 66, &gpioSettings);
    if ( NULL == g_gpioA17 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA18 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 67, &gpioSettings);
    if ( NULL == g_gpioA18 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA19 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 68, &gpioSettings);
    if ( NULL == g_gpioA19 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA20 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 69, &gpioSettings);
    if ( NULL == g_gpioA20 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA21 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 70, &gpioSettings);
    if ( NULL == g_gpioA21 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA22 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 71, &gpioSettings);
    if ( NULL == g_gpioA22 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA23 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 72, &gpioSettings);
    if ( NULL == g_gpioA23 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA24 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 73, &gpioSettings);
    if ( NULL == g_gpioA24 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA25 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 74, &gpioSettings);
    if ( NULL == g_gpioA25 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
	
    g_gpioEBI = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 106, &gpioSettings);
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

