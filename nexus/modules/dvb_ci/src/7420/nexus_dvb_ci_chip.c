/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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

BDBG_MODULE(nexus_dvb_ci);

/* Sanity check */
#if BCHP_CHIP != 7420
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;

    /* Only one interface is supported on the 7405 */
    BSTD_UNUSED(handle);

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();

    if ( pcmciaMode )
    {
        /* 
          * CHIP2POD
          *
          * GPIO_40: valid, GPIO_41: start, GPIO_42:49: MDI[0:7], GPIO_18: MCLKI
          */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_047_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_046_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_045_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_044_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_043_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_042_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_041_MASK|                
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_040_MASK);
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_048_MASK);
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_018_MASK);
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, regVal);

        /* 
          * POD2CHIP
          *
          * GPIO_29: start, GPIO_30: valid, GPIO_31:38: MDO[0:7], GPIO_16: MCLKO
          */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_031_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_030_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_029_MASK);		
	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, regVal);
		
       regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
       regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_038_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_037_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_036_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_035_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_034_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_033_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_032_MASK);		
	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, regVal);

       regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
       regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_016_MASK);
	regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 4)); /* A14 */
       BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, regVal);
    }
    else
    {
        /* 
          * CHIP2POD
          *
          * GPIO_40: valid, GPIO_41: start, GPIO_42:49: MDI[0:7], GPIO_18: MCLKI
          */

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_047_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_046_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_045_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_044_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_043_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_042_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_041_MASK|                
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_040_MASK);
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_047, 1)|  
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_046, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_045, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 1)); 
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_049_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_048_MASK);
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049, 1)|  
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048, 1)); 
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_018_MASK);
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018, 1)); /* MCLKI */
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, regVal);

        /* 
          * POD2CHIP
          *
          * GPIO_29: start, GPIO_30: valid, GPIO_31:38: MDO[0:7], GPIO_16: MCLKO
          */

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_031_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_030_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_029_MASK);		
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_031, 1)|  
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_030, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_029, 1)); 
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, regVal);

	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, regVal);
		
       regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
       regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_038_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_037_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_036_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_035_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_034_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_033_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_032_MASK);	
        regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_038, 1)|  
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_037, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 1)|
                   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032, 1)); 
	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, regVal);

       regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
       regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_016_MASK);
	regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 1)); /* MCLKO */
       BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, regVal);
    }

    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DvbCi_P_InitChip(NEXUS_DvbCiHandle handle)
{
    BSTD_UNUSED(handle);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_DvbCi_P_UninitChip(NEXUS_DvbCiHandle handle)
{
    BSTD_UNUSED(handle);
    return NEXUS_SUCCESS;
}

