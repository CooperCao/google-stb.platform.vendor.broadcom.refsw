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
*   This file contains pinmux initialization for the 97408 reference board.
*
* Revision History:
 *
 * $brcm_Log: $
 * 
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_irq0.h"

BDBG_MODULE(nexus_platform_pinmux);

/***************************************************************************
Summary:
	Configure pin muxes for a 97408 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
	BREG_Handle hReg = g_pCoreHandles->reg;
	uint32_t reg;
	
	BDBG_ASSERT(NULL != hReg);
	
	/* Writing pinmux registers in numerical order.  For reference, and as a check,
	   explicitly set all pinmuxes here, even those with default setting. (d) 
	   Conditioned out if set elsewhere. */
	
	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_7 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_6 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_5 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_4 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_3 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_2 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_1 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_0 ) 	
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_7, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_6, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_5, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_4, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_3, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_2, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_1, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data_0, 0 ) 
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0,
		     reg);
	
	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_mosi ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_miso ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_sck ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_rbb ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_web ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_reb ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_ale ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_cle )
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_mosi,  0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_miso,  0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_sck,   0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_rbb, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_web, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_reb, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_ale, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_cle, 0 )
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_04 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_03 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_02 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_01 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_00 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs2b ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs1b ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs0b ) 
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_04,       1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_03,       1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_02,       1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_01,       1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_00,       0 ) |	  
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs2b,      1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs1b,      0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs0b,      0 ) 
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_12 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_11 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_10 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_09 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06 ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_05 ) 
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_12, 1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_11, 1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_10, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_09, 0 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08, 1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07, 1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06, 3 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_05, 3 ) 
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_20  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_19  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_18  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_17  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_15  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_14  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_13  )
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_20, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_19, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_18, 2  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_17, 2  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_15, 2  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_14, 0  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_13, 0  )
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_28  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_27  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_26  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_25  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_24  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_23  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_22  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_21  )
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_28, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_27, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_26, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_25, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_24, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_23, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_22, 1  ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_21, 1  )
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_36  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_35  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_34  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_33  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_32  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_31  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_30  ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_29 )
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_36, 0 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_35, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_34, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_33, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_32, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_31, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_30, 1 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_29, 1 )   
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
	reg &= ~(
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_03    ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_02    ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_01    ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_00    ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_40     ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_39     ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_38     ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_37     )
		 );
	reg |=
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_03, 1    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_02, 1    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_01, 1    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, sgpio_00, 1    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_40,  0    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_39,  0    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_38,  0    ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_37,  0    )   
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,
		     reg);

	reg = BREG_Read32(hReg,
			  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
	reg &= ~(		 
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, bsc_s_sda ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, bsc_s_scl ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, ir_in     ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, sgpio_05   ) |
		 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, sgpio_04   )
		 );
	reg |=	  
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, bsc_s_sda, 0 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, bsc_s_scl, 0 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, ir_in,     0 ) | 
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, sgpio_05,   1 ) |
	  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, sgpio_04,   1 )  
	  ;
	BREG_Write32(hReg,
		     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
		     reg);
	       	
	/* Configure the AVD UART to debug mode.  AVD0_OL -> UART1. */
	reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    
	reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
	reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
	reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL);

	BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
	reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
	reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,
			       BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
    
	BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    /* enable interrupts for UARTC, they are  mapped to L1 interrupt UPG_UART2_CPU_INTR  and used by TP1 (soft audio) */
    reg = BREG_Read32(hReg,  BCHP_IRQ0_IRQEN);
    reg |= BCHP_FIELD_DATA(IRQ0_IRQEN, uartc_irqen, 1);
    BREG_Write32(hReg,  BCHP_IRQ0_IRQEN, reg);

	return BERR_SUCCESS;
}
