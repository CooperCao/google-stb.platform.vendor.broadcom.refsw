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
*   This file contains pinmux initialization for the 97468 reference board.
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

BDBG_MODULE(nexus_platform_pinmux);

/***************************************************************************
Summary:
	Configure pin muxes for a 97468 reference platform
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
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data6 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data5 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data4 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data3 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data2 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data1  ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data0  ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, aud0_spdif  )
            );
    reg |=
	    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data6, 0 ) |  /*(d) NAND_DATA6 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data5, 0 ) | /*(d) NAND_DATA5 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data4, 0 ) | /*(d) NAND_DATA4 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data3, 0 ) | /*(d) NAND_DATA3 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data2, 0 ) | /*(d) NAND_DATA2 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data1, 0 ) | /*(d) NAND_DATA1 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, nand_data0, 0 ) | /*(d) NAND_DATA0 [p.21] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, aud0_spdif, 0 ) /*(d) AUD0_SPDIF [p.15] */
    ;
    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0,
                 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_miso ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_sck ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_rbb ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_web ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_cle ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_reb ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_ale ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_data7 )
			);
	reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_miso,  0 ) |   /*(d) SF_MISO [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, sf_sck,   0 ) |   /*(d) SF_SCK [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_rbb, 0 ) |  /*(d) NAND_RBB [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_web, 0 ) |  /*(d) NAND_WEB [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_cle, 0 ) |  /*(d) NAND_CLE [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_reb, 0 ) |  /*(d) NAND_REB [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_ale, 0 ) |  /*(d) NAND_ALE [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, nand_data7, 0 )  /*(d) NAND_DATA7 [p.21] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_00 ) |  
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, enet_link ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, enet_activity ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, usb0_pwron ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs2b ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs1b ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs0b ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, sf_mosi )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_00,       1 ) |   /* Enable CLK27 to the FPGA. Bcm97468.Bcm7468.SUN_TOP_CTRL.PIN_MUX_CTRL_2.gpio_00=1 */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, enet_link,     0 ) |  /* (d)LNKLEDb [p.26] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, enet_activity, 0 ) |  /*(d)ENET_ACTIVITY /ACTLEDb [p.26] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, usb0_pwron,    0 ) | /*(d)USB0_PWRON_N [p.26] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs2b,      0 ) |  /*(d)HIF_CS2b [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs1b,      0 ) | /*(d) HIF_CS1b [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, hif_cs0b,      0 ) | /*(d) HIF_CS0b [p.21] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, sf_mosi,       0 )   /*(d)SF_MOSI [p.21] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_05 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_04 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_03 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_02 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_01 )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08, 1 ) |   /*SDIO_DAT2  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07, 1 ) |   /*SDIO_DAT1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06, 1 ) |  /*SDIO_DAT0  [p.10]  */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_05, 1 ) |  /*SDIO_CLK  [p.10]  */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_04, 0 ) |  /*(d) GPIO_04 [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_03, 1 ) | /*SDIO_WPROT[p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_02, 1 ) | /*SDIO_CMD[p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_01, 1 )    /*SDIO_DAT3  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_15  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_14  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_13  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_12  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_11  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_10  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_09  )
			);
	reg |=
	    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16, 2  ) |    /*MII_RX_CLK  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_15, 4  ) |  /*TP_OUT_01 suggested for AVD_OL/MII_TX_EN  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_14, 4  ) | /* TP_IN_01 suggested for AVD_OL/ MII_COL  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_13, 2  ) | /*MII_TX_ER  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_12, 2  ) | /*MII_RX_ER  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_11, 2  ) | /*MII_RX_EN  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_10, 0  ) | /*GPIO_10  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_09, 1  )    /*SDIO_PRES  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_24  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_23  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_22  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_21  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_20  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_19  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_18  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_17  )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_24, 2  ) | /*MII_TXD_01  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_23, 2  ) | /*MII_TXD_02  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_22, 2  ) | /*MII_TXD_03  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_21, 2  ) | /*MII_RXD_00  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_20, 2  ) | /*MII_RXD_01  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_19, 2  ) | /*MII_RXD_02  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_18, 2  ) | /*MII_RXD_03  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_17, 2  )    /*MII_TX_CLK  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_32  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_31  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_30  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_29  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_28  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_27  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_26  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_25  )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_32, 1  ) | /*GP_PKT_DATA0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_31, 1  ) | /*GP_PKT_CLK0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_30, 1  ) | /*GP_UART_RXD_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_29, 1  ) | /*GP_UART_TXD_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_28, 2  ) | /*MII_MDC  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_27, 2  ) | /*MII_MDIO  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_26, 2  ) | /*MII_CRS  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_25 , 2 )    /*MII_TXD_00  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_40   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_39   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_38   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_37   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_36   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_35   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_34   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_33   )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_40, 2  ) | /*GP_SC_PRES_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_39, 2  ) | /*GP_SC_IO_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_38, 2  ) | /*GP_SC_RST_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_37, 2  ) | /*GP_SC_CLK_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_36, 2  ) | /*GP_SC_VCC_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_35, 1  ) | /*GP_PKT_VALID0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_34, 1  ) | /*GP_PKT_ERROR0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_33, 1  )   /*GP_PKT_SYNC0  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_48 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_47 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_46 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_45 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_44 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_43 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_42 ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_41 )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_48, 1 ) | /*FP_LED_LD_1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_47, 1 ) | /*FP_LED_LD_0  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_46, 1 ) | /*PKT_ERROR1 use for daughter cards/GP_SC_EXT_CLK  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_45, 3 ) | /*GP_IRQ12_SLOT0B_N  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_44, 3 ) | /*GP_IRQ11_SLOT0A_N  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_43, 1 ) | /*GP_NDS_SC_AUX1_1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_42, 1 ) | /*GP_NDS_SC_AUX0_1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_41, 1 )   /*GP_NDS_SC_VCTRL_1  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
				 reg);

	reg = BREG_Read32(hReg,
					  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
	reg &= ~(
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_52   ) |  
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_51   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_50   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_49   ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_03  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_02  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_01  ) |
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_00  )
			);
	reg |=
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_52, 1   ) |  /*GP_PKT_SYNC1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_51, 1   ) | /*GP_PKT_DATA1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_50, 1   ) | /*GP_PKT_CLK1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_49, 1   ) | /*GP_PKT_VALID1  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_03, 1  ) | /*GP_BSC_M1_SDA  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_02, 1  ) | /*GP_BSC_M1_SCL  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_01, 1  ) | /*GP_BSC_M0_SDA  [p.10] */
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_00, 1  )   /*GP_BSC_M0_SCL  [p.10] */
	;
	BREG_Write32(hReg,
				 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
				 reg);


    /* Configure the AVD UART to debug mode.  AVD0_OL -> UART1. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
   /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL); */

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,
                          BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
    
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}

