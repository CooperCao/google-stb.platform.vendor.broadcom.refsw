/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
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
*  $brcm_Workfile: $
*  $brcm_Revision: $
*  $brcm_Date: $
*
*  Module Description:
*
*  Revision History:
*
*  $brcm_Log: $
*
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);

/***************************************************************************
Summary:
    Configure pin muxes for the 97584 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0
    * GPIO_00    : MII_RX_DV
    * GPIO_01    : MII_RX_CLK
    * GPIO_02    : MII_TX_CLK
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_00) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_01) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_02)
        );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_00, 2) |  /* MII_RX_DV */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_01, 2) |      /* MII_RX_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_02, 2) ;      /* MII_TX_CLK */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1
    * GPIO_03    : MII_RXD_03
    * GPIO_04    : MII_RXD_02
    * GPIO_05    : MII_RXD_01
    * GPIO_06    : MII_RXD_00
    * GPIO_07    : MII_TXD_03
    * GPIO_08    : MII_TXD_02
    * GPIO_09    : MII_TXD_01
    * GPIO_10    : MII_TXD_00
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_03) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_04) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_05) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_06) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_07) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_08) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_09) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_10)
        );
#if NEXUS_HAS_DVB_CI
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_03, 1) |  /* EBI_ADDR3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_04, 1) |      /* EBI_ADDR4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_05, 1) |      /* EBI_ADDR5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_06, 1) |      /* EBI_ADDR6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_07, 1) |      /* EBI_ADDR7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_08, 1) |      /* EBI_ADDR8 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_09, 1) |      /* EBI_ADDR9*/
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_10, 1) ;      /* EBI_ADDR10 */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_03, 2) |  /* MII_RXD_03 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_04, 2) |      /* MII_RXD_02 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_05, 2) |      /* MII_RXD_01 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_06, 2) |      /* MII_RXD_00 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_07, 2) |      /* MII_TXD_03 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_08, 2) |      /* MII_TXD_02 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_09, 2) |      /* MII_TXD_01*/
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_10, 2) ;      /* MII_TXD_00 */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
    * GPIO_11    : MII_TX_EN
    * GPIO_12    : MII_RX_ER
    * GPIO_13    : MII_TX_ER
    * GPIO_14    : MII_COL
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_11) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_12) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_13) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_14)
        );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_12, 2) |  /* MII_RX_ER */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_13, 2) |      /* MII_TX_ER */
#if NEXUS_HAS_DVB_CI
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_11, 1) |  /* EBI_ADDR11 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_14, 1) |  /* EBI_ADDR14 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_15, 1) |  /* EBI_WAITB */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_16, 1) |  /* EBI_WE1B */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_17, 1) ;  /* EBI_RWB */
#else
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_11, 2) |  /* MII_TX_EN */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_14, 2) ;  /* MII_COL */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

#if NEXUS_HAS_DVB_CI
	/* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6
	* GPIO_46    : POD2CHIP_MCLKI
	* GPIO_47    : POD2CHIP_MDI0
	* GPIO_48    : POD2CHIP_MDI1
	* GPIO_49    : POD2CHIP_MDI2
	* GPIO_50    : POD2CHIP_MDI3
	* GPIO_51    : POD2CHIP_MDI4
	* GPIO_52    : POD2CHIP_MDI5
	*/

	reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);

	reg &= ~(
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_46) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_47) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_48) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_49) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52)
		);

	reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_46, 1) |  /* POD2CHIP_MCLKI */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_47, 1) |      /* POD2CHIP_MDI0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_48, 1) |      /* POD2CHIP_MDI1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_49, 1) |      /* POD2CHIP_MDI2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50, 1) |      /* POD2CHIP_MDI3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51, 1) |      /* POD2CHIP_MDI4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52, 1) ;      /* POD2CHIP_MDI5 */

	BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

	/* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
	* GPIO_53    : POD2CHIP_MDI6
	* GPIO_54    : POD2CHIP_MDI7
	* GPIO_55    : POD2CHIP_MISTRT
	* GPIO_56    : POD2CHIP_MIVAL
	* GPIO_57    : CHIP2POD_MCLKO
	* GPIO_58    : CHIP2POD_MDO0
	* GPIO_59    : CHIP2POD_MDO1
	* GPIO_60    : CHIP2POD_MDO2
	*/

	reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);

	reg &= ~(
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_53) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_54) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_55) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_56) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_57) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60)
		);

	reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_53, 1) |  /* POD2CHIP_MDI6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_54, 1) |      /* POD2CHIP_MDI7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_55, 1) |      /* POD2CHIP_MISTRT */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_56, 1) |      /* POD2CHIP_MIVAL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_57, 1) |      /* CHIP2POD_MCLKO */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58, 1) |      /* CHIP2POD_MDO0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59, 1) |      /* CHIP2POD_MDO1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60, 1) ;      /* CHIP2POD_MDO2 */

	BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

	/* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
	* GPIO_61    : CHIP2POD_MDO3
	* GPIO_62    : CHIP2POD_MDO4
	* GPIO_63    : CHIP2POD_MDO5
	* GPIO_64    : CHIP2POD_MDO6
	* GPIO_65    : CHIP2POD_MDO7
	* GPIO_66    : CHIP2POD_MOSTRT
	* GPIO_67    : CHIP2POD_MOVAL
	* GPIO_68    : EBI_ADDR13
	*/

	reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);

	reg &= ~(
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_61) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_62) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_63) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_64) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_65) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67) |
		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68)
		);

	reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_61, 1) |  /* CHIP2POD_MDO3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_62, 1) |      /* CHIP2POD_MDO4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_63, 1) |      /* CHIP2POD_MDO5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_64, 1) |      /* CHIP2POD_MDO6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_65, 1) |      /* CHIP2POD_MDO7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66, 1) |      /* CHIP2POD_MOSTRT */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67, 1) |      /* CHIP2POD_MOVAL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68, 3) ;      /* POD2CHIP_MICLK */

	BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);
#endif

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
    * gpio_74    : CLK_OBSRV2(3)
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74)
        );
#if !NEXUS_HAS_DVB_CI
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74, 3) ;  /* CLK_OBSRV2 */
#else
	reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_69, 3) |  /* EBI_ADDR12 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_70, 2) |      /* MPOD_SCTL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_71, 2) |      /* MPOD_SCLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_72, 2) |      /* MPOD_M_SDO */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_73, 1) ;      /* MPOD_M_SDI */
#endif

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
    * GPIO_79    : SC0_VCC
    * GPIO_80    : SC0_CLK
    * GPIO_81    : SC0_RST
    * GPIO_82    : SC0_IO
    * GPIO_83    : SC0_PRES
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_79) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_80) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_81) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83)
        );

    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_79, 1) |      /* SC0_VCC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_80, 1) |      /* SC0_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_81, 1) |      /* SC0_RST */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82, 1) |      /* SC0_IO */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83, 1);       /* SC0_PRES */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
    * GPIO_87    : UART_TX0
    * GPIO_88    : UART_RX0
    * GPIO_89    : UART_TX1
    * GPIO_90    : UART_TX1
    * GPIO_91    : ALT_TP_OUT_02
    * GPIO_92    : ALT_TP_IN_02
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_87) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_88) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_89) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_92)
        );

    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_87, 2) |   /* UART_TX0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_88, 2) |      /* UART_RX0 */
#if NEXUS_HAS_DVB_CI
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_89, 4) |      /* POD_EBI_RDB */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 4) |      /* POD_EBI_WE0B */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 4) |      /* POD_EBI_DSB */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_92, 4) ;      /* GPIO */
#else
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_89, 1) |      /* UART_TX1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 1) |      /* UART_TX1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 3) |      /* ALT_TP_OUT_02 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_92, 3) ;      /* ALT_TP_IN_02 */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
    * GPIO_93    : SF_HOLDB
    * GPIO_94    : SF_WPB
    * GPIO_96    : PKT2_CLK
    * GPIO_97     : PKT2_DATA
    * GPIO_98     : PKT2_SYNC
    * GPIO_99     : PKT2_VALID
    * GPIO_100    : PKT2_ERROR
    * GPIO_101    : default
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_93) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_94) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_96) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_97) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100)
        );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_93, 1) | /* SF_HOLDB */
#if NEXUS_HAS_DVB_CI
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_94, 1);     /* SF_WPB */
#else
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_94, 1) |     /* SF_WPB */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_96, 1) |     /* PKT2_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_97, 1) |     /* PKT2_DATA */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98, 1) |     /* PKT2_SYNC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99, 1) |     /* PKT2_VALID */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1);     /* PKT2_ERROR */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
    * GPIO_105    : ENET_ACTIVITY
    * GPIO_107    : PKT4_CLK
    * GPIO_108    : PKT4_DATA
    * GPIO_109    : PKT4_SYNC
    * GPIO_110    : MII_CRS
    * others      : default
    */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_107) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_108) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110)
        );

    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105, 1) |   /* ENET_ACTIVITY */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_107, 2) |      /* PKT4_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_108, 2) |      /* PKT4_DATA */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109, 2) |      /* PKT4_SYNC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 1) ;      /* MII_CRS */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
    * GPIO_123    : RGMII_MDC
    * GPIO_124    : RGMII_MDIO
    * others      : default
    */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);

    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_123) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_124)
        );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_123, 1) |      /* RGMII_MDC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_124, 1) ;      /* RGMII_MDIO */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0
    * AON_IR_IN0     : AON_IR_IN0(0)
    * AON_S3_STANDBYB: AON_S3_STANDBYB(0)
    * AON_HDMI_HTPLG : AON_HDMI_HTPLG(0)
    * AON_GPIO_00    : AUD_SPDIF(1)
    * AON_GPIO_01    : ENET_LINK(1)
    * AON_GPIO_03    : LED_OUT(1)
    * AON_GPIO_04    : AON_GPIO_04(0)
    * AON_GPIO_05    : I2S_DATA0_OUT(3)
    */

    reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);

    reg &=~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_ir_in0 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_s3_standbyb ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_hdmi_htplg ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05 )
        );

    reg |=(
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_ir_in0, 0 ) |      /* AON_IR_IN0 */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_s3_standbyb, 0 ) | /* AON_S3_STANDBYB */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_hdmi_htplg, 0 ) |  /* AON_HDMI_HTPLG */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 ) |     /* AUD_SPDIF */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 1 ) |     /* ENET_LINK */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 1 ) |     /* LED_OUT */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 0 ) |     /* I2S_CLK0_OUT */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 3 )       /* I2S_DATA0_OUT */
        );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
    * AON_GPIO_07    : UART_RX2(4)
    * AON_GPIO_09    : UART_TX2(4)
    */

    reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);

    reg &=~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 )
        );

    reg |=(
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 4 ) |  /* UART_RX2 */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 4 )    /* UART_TX2 */
        );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3
    * sgpio_00       : BSC_M0_SCL(1)
    * sgpio_01       : BSC_M0_SDA(1)
    */

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);

    reg &=~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01)
        );

    reg |=(
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00,1 ) |      /* BSC_M0_SCL */
        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01,1 )        /* BSC_M0_SDA */
        );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1 */
    /* UART2 is reserved for LEAP */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}
