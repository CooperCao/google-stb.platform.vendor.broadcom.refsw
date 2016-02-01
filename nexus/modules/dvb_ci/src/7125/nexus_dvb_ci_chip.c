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
#if BCHP_CHIP != 7125
#error This file is not for the current chip.  Please make sure this file is unique per-chip.
#endif

NEXUS_Error NEXUS_DvbCi_P_SetupPinmuxes(NEXUS_DvbCiHandle handle, bool pcmciaMode)
{
    uint32_t regVal;

    /* Only one interface is supported on the 7125 */
    BSTD_UNUSED(handle);

    /* Do this in critical section, these are shared registers */

    BKNI_EnterCriticalSection();

    if ( pcmciaMode )
    {
        /* 
          * CHIP2CC
          *
          * GPIO_60: MOVAL, GPIO_61: MOSTRT, GPIO_70:77: MDO[0:7], GPIO_59: MCLKO
          */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_61_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_60_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_59_MASK);

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);

        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_72_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_71_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_70_MASK);

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);

		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_77_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_76_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_75_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_74_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_73_MASK);

        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);

        /* 
          * CC2CHIP
          *
          * GPIO_79: MISTRT, GPIO_78: MIVAL, GPIO_62:69: MDI[0:7], GPIO_58: MCLKI
          */
        regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);

        regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_64_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_63_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_62_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_58_MASK);
		
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_58, 2)); /* POD_EBI_ADDR_14 */

		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);
		
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);

		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_69_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_68_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_67_MASK|
                    BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_66_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_65_MASK);
		
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);

		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);

		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_79_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_78_MASK);

		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
    }
    else
    {
		/* 
		  * CHIP2CC
		  *
		  * GPIO_60: MOVAL, GPIO_61: MOSTRT, GPIO_70:77: MDO[0:7], GPIO_59: MCLKO
		  */
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_61_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_60_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_59_MASK);
	
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_61, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_60, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_59, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);
	
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_72_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_71_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_70_MASK);
	
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_72, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_71, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_70, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);
	
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_77_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_76_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_75_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_74_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_73_MASK);
	
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_77, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_76, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_75, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_74, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_73, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
	
		/* 
		  * CC2CHIP
		  *
		  * GPIO_79: MISTRT, GPIO_78: MIVAL, GPIO_62:69: MDI[0:7], GPIO_58: MCLKI
		  */
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_64_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_63_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_62_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_gpio_58_MASK);
		
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_64, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_63, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_62, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_58, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, regVal);
		
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_69_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_68_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_67_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_66_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_gpio_65_MASK);
		
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_69, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_68, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_67, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_66, 1)|
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_65, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, regVal);
	
		regVal = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
	
		regVal &= ~(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_79_MASK|
					BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18_gpio_78_MASK);
	
		regVal |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_79, 1)|  
				   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_78, 1)); 
	
		BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, regVal);
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

