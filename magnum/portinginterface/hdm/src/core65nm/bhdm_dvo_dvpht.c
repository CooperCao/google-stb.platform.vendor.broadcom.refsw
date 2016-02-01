/***************************************************************************
*	  (c)2004-2010 Broadcom Corporation
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
*  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/


#include "bhdm.h"
#include "bhdm_priv.h"



BDBG_MODULE(BHDM) ;


BERR_Code BHDM_DVO_P_EnableDvoPort(
	BHDM_Handle hHDMI,		/* [in] HDMI handle */
	BHDM_OutputFormat eOutputFormat /* [in] format to use on Output Port */
)
{
	BERR_Code	   rc = BERR_SUCCESS;
	uint32_t Register ;
	uint8_t iByPassHdmiPort ;
	uint8_t ui24BitMode  ;


	BDBG_ENTER(BHDM_DVO_P_EnableDvoPort) ;
	BDBG_ASSERT( hHDMI );

	/*****	Enable DVO Port *****/
	iByPassHdmiPort = 1 ;  /* DVO Port */
	ui24BitMode = 0 ;
	BDBG_WRN(("Enabling DVO Port; HDMI output DISABLED")) ;

	if (eOutputFormat != BHDM_OutputFormat_e12BitDVOMode)
	{
		BDBG_ERR(("12 Bit DVO Only Supported; Enabling for 12 Bit DVO"));
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done;
	}

	/* Enable Pin muxes */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17) ;
	Register &=
		~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_086)  /* dvo0_de */
		|  BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_087));	/* dvo0_vsync */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_086, 3)  /* dvo0_de */
		|  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_087, 4));  /* dvo0_vsync */
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18) ;
	Register &=
		~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088)  /* dvo0_hsync */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_091)   /* dvo0_00 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_092)   /* dvo0_01 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089)   /* dvo0_02 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_090)   /* dvo0_03 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_093)   /* dvo0_04 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_094)   /* dvo0_05 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_095)) ; /* dvo0_06 */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088, 4)    /* dvo0_hsync*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_091, 2)    /* dvo0_00 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_092, 2)    /* dvo0_01*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089, 4)    /* dvo0_02 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_090, 2)    /* dvo0_03 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_093, 2)    /* dvo0_04*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_094, 2)    /* dvo0_05*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_095, 2));	 /* dvo0_06*/
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19) ;
	Register &=
		~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_096) /* dvo0_07 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_097)  /* dvo0_08 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_098)  /* dvo0_09 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_099)  /* dvo0_10 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_100)  /* dvo0_11 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_101)  /* dvo0_clk_p */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_102)) ;  /* dvo0_clk_n */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_096, 2)  /* dvo0_00 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_097, 2)  /* dvo0_01 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_098, 2)  /* dvo0_02 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_099, 2)  /* dvo0_03 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_100, 2)  /* dvo0_04 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_101, 2)  /* dvo0_05 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_102, 2)) ;  /* dvo0_06 */
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19, Register) ;


	/* set the dvo/dvi mode */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_SCHEDULER_CONTROL) ;
	Register &= ~ BCHP_MASK(HDMI_SCHEDULER_CONTROL, HDMI_MODE_REQUEST) ;
	Register |= BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, HDMI_MODE_REQUEST, 0);	/* must be in DVI mode */
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_SCHEDULER_CONTROL, Register) ;

	/* by pass the HDMI port; enable the DVO port */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_ENCODER_CTL) ;
	Register &= ~BCHP_MASK(HDMI_ENCODER_CTL, ENCODER_BYPASS) ;
	Register |= BCHP_FIELD_DATA(HDMI_ENCODER_CTL, ENCODER_BYPASS, iByPassHdmiPort) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_ENCODER_CTL, Register) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL) ;
	Register &= ~(BCHP_MASK(HDMI_MISC_CONTROL, IS_24_BIT_MODE)
				|BCHP_MASK(HDMI_MISC_CONTROL, DVO_ESEL)) ;
	Register |= BCHP_FIELD_DATA(HDMI_MISC_CONTROL, IS_24_BIT_MODE, ui24BitMode)
			| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DVO_ESEL, 1) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL, Register) ;

	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL_1) ;
	Register &= ~(BCHP_MASK(HDMI_MISC_CONTROL_1, DVO_FORMAT_DDR)
				|BCHP_MASK(HDMI_MISC_CONTROL_1, DVO_FORMAT_WIDTH)) ;
	Register |= BCHP_FIELD_DATA(HDMI_MISC_CONTROL_1, DVO_FORMAT_DDR, 1)
			| BCHP_FIELD_DATA(HDMI_MISC_CONTROL_1, DVO_FORMAT_WIDTH, 0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL_1, Register) ;


	/* Set delays */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL) ;
	Register &=
		~(BCHP_MASK(HDMI_MISC_CONTROL, DELAY_SELECTION_PIX_CLK)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_ONE_CTL_PIX_CLK)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_TWO_CTL_PIX_CLK)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_THREE_CTL_PIX_CLK)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_FOUR_CTL_PIX_CLK)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_SELECTION_PIX_DATA)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_ONE_CTL_PIX_DATA)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_TWO_CTL_PIX_DATA)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_THREE_CTL_PIX_DATA)
		| BCHP_MASK(HDMI_MISC_CONTROL, DELAY_FOUR_CTL_PIX_DATA)) ;
	Register |=
		(BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_SELECTION_PIX_CLK, 3)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_ONE_CTL_PIX_CLK, 7)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_TWO_CTL_PIX_CLK, 3)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_THREE_CTL_PIX_CLK, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_FOUR_CTL_PIX_CLK, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_SELECTION_PIX_DATA, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_ONE_CTL_PIX_DATA, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_TWO_CTL_PIX_DATA, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_THREE_CTL_PIX_DATA, 0)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DELAY_FOUR_CTL_PIX_DATA, 0)) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL, Register) ;


done:
	BDBG_LEAVE(BHDM_DVO_P_EnableDvoPort);
	return rc ;

}

