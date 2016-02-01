/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
	BERR_Code      rc = BERR_SUCCESS;
	uint32_t Register ;
	uint8_t iByPassHdmiPort ;
	uint8_t ui24BitMode  ;


	BDBG_ENTER(BHDM_DVO_P_EnableDvoPort) ;
	BDBG_ASSERT( hHDMI );

	/*****  Enable DVO Port *****/
	iByPassHdmiPort = 1 ;  /* DVO Port */
	ui24BitMode = 0 ;
	BDBG_WRN(("Enabling DVO Port; HDMI output DISABLED")) ;

	if (eOutputFormat != BHDM_OutputFormat_e12BitDVOMode)
	{
		BDBG_ERR(("12 Bit DVO Only Supported; Enabling for 12 Bit DVO"));
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done;
	}


	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10) ;
	Register &=
		~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)  /* dvo0_de */
		|  BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087));  /* dvo0_clk_n */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 3)  /* dvo0_de */
		|  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 3));  /* dvo0_clk_n */
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11) ;
	Register &=
		~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)	/* dvo0_00 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089)  /* dvo0_01 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090)  /* dvo0_02 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091)  /* dvo0_03 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094)  /* dvo0_04 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095)  /* dvo0_05 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096)  /* dvo0_06 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097)  /* dvo0_07 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093)  /* dvo0_hsync */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092)) ; /* dvo0_vsync */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 3)  /* dvo0_00 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 3)  /* dvo0_01 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 3)  /* dvo0_02 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 3)  /* dvo0_03 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 3)  /* dvo0_04 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 3)  /* dvo0_05 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 3)  /* dvo0_06 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 3)  /* dvo0_07 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 3)  /* dvo0_hsync */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 3)) ; /* dvo0_vsync */
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, Register) ;


	Register = BREG_Read32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12) ;
	Register &=
		~ (BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098)  /* dvo0_08 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099)   /* dvo0_09 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100)   /* dvo0_10 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101)   /* dvo0_11 */
		| BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102)) ; /* dvo0_clk_p */
	Register |=
		(BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 3)  /* dvo0_08*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 3)	/* dvo0_09 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 3)	 /* dvo0_10*/
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 3)	 /* dvo0_11 */
		| BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 3)) ; /* dvo0_clk_p */
	BREG_Write32(hHDMI->hRegister, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, Register) ;

	/* DVO Timings Adjusments per simulation */
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_DVO_TIMING_ADJUST_D, 0xF7AA4435) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_DVO_TIMING_ADJUST_C,  0x8888C5AA) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_DVO_TIMING_ADJUST_B,  0x88888888) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_DVO_TIMING_ADJUST_A,  0x00063633) ;


	/* set the dvo/dvi mode */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL) ;
	Register &= ~(
		  BCHP_MASK(HDMI_MISC_CONTROL, IS_24_BIT_MODE)
		|BCHP_MASK(HDMI_MISC_CONTROL, DVO_ESEL)) ;
	Register |=
		  BCHP_FIELD_DATA(HDMI_MISC_CONTROL, IS_24_BIT_MODE, ui24BitMode)
		| BCHP_FIELD_DATA(HDMI_MISC_CONTROL, DVO_ESEL, 0) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_MISC_CONTROL, Register) ;


	/* by pass the HDMI port; enable the DVO port */
	Register = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_ENCODER_CTL) ;
	Register &= ~BCHP_MASK(HDMI_ENCODER_CTL, ENCODER_BYPASS) ;
	Register |= BCHP_FIELD_DATA(HDMI_ENCODER_CTL, ENCODER_BYPASS, iByPassHdmiPort) ;
	BREG_Write32(hHDMI->hRegister, BCHP_HDMI_ENCODER_CTL, Register) ;

done:
	BDBG_LEAVE(BHDM_DVO_P_EnableDvoPort);
	return rc ;

}

