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
#if BCHP_CHIP != 7325
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

static NEXUS_GpioHandle g_gpioA14, g_gpioA15, g_gpioA16, g_gpioA17, g_gpioA18;
static NEXUS_GpioHandle g_gpioA19, g_gpioA20, g_gpioA21, g_gpioA22;
static NEXUS_GpioHandle g_gpioA23, g_gpioA24, g_gpioA25;

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;
    bool ppkt = (handle->openSettings.tsMode == NEXUS_DvbCiTransportMode_ePpkt)?true:false;

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();

    if ( pcmciaMode )
    {
        /* From the schematic, the 7325 has limited EBI addressing for the CI card.  A15..A25 are not connected, nor is D8..D15 
           Switch these pins to GPIO for this mode, address pins should be driven low because there are no pull down resistors */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_05_MASK|          /* MDO_1/A24 -> A24 (ignored) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_04_MASK);         /* MDO_0/A25 -> A25 (ignored) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);

        regVal &=~(((ppkt==true)? BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_15_MASK:
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_14_MASK)|         /* MCLKO/A15-> A15 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_13_MASK|          /* MOVAL/A16 -> A16 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_12_MASK|          /* MOSTRT/A17 -> A17 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_11_MASK|          /* MDO_7/A18 -> A18 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_10_MASK|          /* MDO_6/A19 -> A19 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_09_MASK|          /* MDO_5/A20 -> A20 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_08_MASK|          /* MDO_4/A21 -> A21 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_07_MASK|          /* MDO_3/A22 -> A22 (GPIO - must drive to 0) */
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_06_MASK);         /* MDO_2/A23 -> A23 (GPIO - must drive to 0) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

        regVal &= ~((ppkt==true)?BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_19_MASK:
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_18_MASK);         /* MCLKI/A14 -> A14 (GPIO - must drive to 0) */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, regVal);
    }
    else
    {
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_05_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_04_MASK);

        regVal |=   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_05, (ppkt==true)?2:1) |         /* MDO_1/A24 -> MDO_1 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_04, (ppkt==true)?2:1);         /* MDO_0/A25 -> MDO_0 */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);

        regVal &= ~(((ppkt==true)? BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_15_MASK:
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_14_MASK)|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_13_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_12_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_11_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_10_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_09_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_08_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_07_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_gpio_06_MASK);

        regVal |= ((ppkt==true)?BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_15, (ppkt==true)?2:1): 
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_14, (ppkt==true)?2:1)) |         /* MCLKO/A15 -> MCLKO */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_13, (ppkt==true)?2:1) |         /* MOVAL/A16 -> MOVAL */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_12, (ppkt==true)?2:1) |         /* MOSTRT/A17 -> MOSTRT */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_11, (ppkt==true)?2:1) |         /* MDO_7/A18 -> MDO_7 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_10, (ppkt==true)?2:1) |         /* MDO_6/A19 -> MDO_6 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_09, (ppkt==true)?2:1) |         /* MDO_5/A20 -> MDO_5 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08, (ppkt==true)?2:1) |         /* MDO_4/A21 -> MDO_4 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07, (ppkt==true)?2:1) |         /* MDO_3/A22 -> MDO_3 */
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06, (ppkt==true)?2:1);         /* MDO_2/A23-> MDO_2 */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

        regVal &= ~(((ppkt==true)?BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_19_MASK:
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_gpio_18_MASK));

        regVal |= ((ppkt==true)?BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_19, (ppkt==true)?2:1):
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_18, (ppkt==true)?2:1));         /* MCLKI/A14 -> MCLKI */

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, regVal);
    }

    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DvbCi_P_InitChip(NEXUS_DvbCiHandle handle)
{
    NEXUS_GpioSettings gpioSettings;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    bool ppkt = (handle->openSettings.tsMode == NEXUS_DvbCiTransportMode_ePpkt)?true:false;

    /* All pins will be setup to drive 0 on unused address lines */

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eLow;

    if(ppkt == true)
    {
        g_gpioA14 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 19, &gpioSettings);
    }
    else
    {
        g_gpioA14 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 18, &gpioSettings);
    }
    if ( NULL == g_gpioA14 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}


    if(ppkt == true)
    {
        g_gpioA15 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 15, &gpioSettings);
    }
    else
    {
        g_gpioA15 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 14, &gpioSettings);
    }
    if ( NULL == g_gpioA15 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}

    g_gpioA16 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 13, &gpioSettings);
    if ( NULL == g_gpioA16 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA17 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 12, &gpioSettings);
    if ( NULL == g_gpioA17 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA18 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 4, &gpioSettings);
    if ( NULL == g_gpioA18 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA19 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 5, &gpioSettings);
    if ( NULL == g_gpioA19 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA20 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 6, &gpioSettings);
    if ( NULL == g_gpioA20 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA21 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 7, &gpioSettings);
    if ( NULL == g_gpioA21 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA22 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 8, &gpioSettings);
    if ( NULL == g_gpioA22 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA23 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 9, &gpioSettings);
    if ( NULL == g_gpioA23 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA24 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 10, &gpioSettings);
    if ( NULL == g_gpioA24 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
    g_gpioA25 = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 11, &gpioSettings);
    if ( NULL == g_gpioA25 ) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error;}
        
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
    if ( g_gpioA14 ) {NEXUS_Gpio_Close(g_gpioA15); g_gpioA14 = NULL;}

    return NEXUS_SUCCESS;
}

