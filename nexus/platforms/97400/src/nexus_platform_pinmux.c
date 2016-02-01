/***************************************************************************
*     (c)2011 Broadcom Corporation
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
*   This file contains pinmux initialization for reference boards.
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
#include "bkni.h"
#include "bchp_clk.h"

BDBG_MODULE(nexus_platform_pinmux);

/***************************************************************************
Summary:
    Configure pin muxes for a 97400 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    if (!hReg) {
        BDBG_ERR(("Register handle has not been opened yet."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Writing pinmuxes in order from pg. 14 of the schematic */

    /* GPIO 000..001 set by OS */

    /* GPIO 002 */
    reg = BREG_Read32(hReg,
                      BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002)
            );

    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 0) /* GPIO_002 -> POD_EN */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2,
                 reg);

    /* GPIO 003..006 set by OS */

    /* GPIO 007..013 */
    reg = BREG_Read32(hReg,
                      BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013 )
            );
    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 4 ) | /* UART 1 RX -> tp_in_07 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 4 ) | /* UART 1 TX -> tp_out_08 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 3 ) | /* IRQ1 [3517 IRQ] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 3 ) | /* IRQ2 [Test] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 4 ) | /* UART 2 RX -> tp_in_11 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 4 ) | /* UART 2 TX -> tp_out_12 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 )   /* IRQ3 [POD] */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,
                 reg);

    /* GPIO 014..023 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_014, 4 ) | /* IRQ4 [Slot 2] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_015, 2 ) | /* IRQ5 [Diseqc] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_016, 2 ) | /* IRQ6 [4500 IRQ] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_017, 3 ) | /* AUD_FS_CLK0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_018, 3 ) | /* AUD_FS_CLK1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_019, 3 ) | /* SC_CLK_OUT_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 2 ) | /* IRQ13 [Slot 1a] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021, 5 ) | /* SC_CLK_OUT_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 2 ) | /* IRQ 11 [Slot 0b] */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 2 )   /* IRQ 12 [Slot 0a] */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,
                 reg);

    /* GPIO 024..033 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 1 ) | /* VI0_656_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 2 ) | /* SPI_M_SCK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 2 ) | /* SPI_M_MOSI */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 2 ) | /* SPI_M_MISO */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 2 ) | /* SPI_M_SS0b */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 2 ) | /* PPKT_IN_SYNC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 2 ) | /* PPKT_IN_VALID */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 2 ) | /* PPKT_IN_DATA0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 2 ) | /* PPKT_IN_DATA1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 2 )   /* PPKT_IN_DATA2 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,
                 reg);

    /* GPIO 034..043 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 2 ) | /* PPKT_IN_DATA3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 2 ) | /* PPKT_IN_DATA4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 2 ) | /* PPKT_IN_DATA5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 2 ) | /* PPKT_IN_DATA6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 2 ) | /* PPKT_IN_DATA7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 2 ) | /* PPKT_IN_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, 2 ) | /* RMXP_OUT_VALID */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, 2 ) | /* RMXP_OUT_SYNC */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, 2 ) | /* RMXP_OUT_DATA0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043, 4 )   /* NDS_SC_AUX0 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,
                 reg);

    /* GPIO 044..053 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044, 4 ) | /* NDS_SC_AUX1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045, 4 ) | /* NDS_SC_VCTRL1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046, 4 ) | /* SC_IO_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047, 4 ) | /* SC_CLK_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048, 4 ) | /* SC_RST_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049, 4 ) | /* SC_PRES_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 4 ) | /* SC_VCC_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051, 1 ) | /* PKT_ERROR0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052, 1 ) | /* PKT_ERROR1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 1 )   /* PKT_ERROR2 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,
                 reg);

    /* GPIO 054..064 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054, 1 ) | /* PKT_ERROR3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 2 ) | /* VI0_656_D0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 2 ) | /* VI0_656_D1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 2 ) | /* VI0_656_D2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 2 ) | /* VI0_656_D3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 2 ) | /* VI0_656_D4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 2 ) | /* VI0_656_D5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 2 ) | /* VI0_656_D6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 2 ) | /* VI0_656_D7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063, 1 ) | /* PKT_CLK_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1 )   /* PKT_CLK_1 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,
                 reg);

    /* GPIO 065..078 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065, 1 ) | /* PKT_CLK_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066, 1 ) | /* PKT_CLK_3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067, 1 ) | /* PKT_CLK_4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 1 ) | /* PKT_CLK_5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, 1 ) | /* PKT_DATA0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070, 1 ) | /* PKT_DATA1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071, 1 ) | /* PKT_DATA2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 1 ) | /* PKT_DATA3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1 ) | /* PKT_DATA4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1 ) | /* PKT_DATA5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1 ) | /* PKT_SYNC0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1 ) | /* PKT_SYNC1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077, 1 ) | /* PKT_SYNC2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078, 1 )   /* PKT_SYNC3 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,
                 reg);

    /* GPIO 079..088 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079, 1 ) | /* PKT_SYNC4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1 ) | /* PKT_SYNC5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1 ) | /* SC_IO_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 1 ) | /* SC_CLK_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 1 ) | /* SC_RST_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 1 ) | /* SC_PRES_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 1 ) | /* SC_VCC_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1 ) | /* GP_LED_KD_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1 ) | /* GP_LED_KD_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_088, 1 )   /* GP_LED_KD_2 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,
                 reg);

    /* GPIO 089..098 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1 ) | /* GP_LED_KD_3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1 ) | /* GP_LED_LS_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1 ) | /* GP_LED_LS_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 1 ) | /* GP_LED_LS_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 1 ) | /* GP_LED_LS_3 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 1 ) | /* GP_LED_LS_4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 1 ) | /* GP_LED_LD_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 1 ) | /* GP_LED_LD_1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 1 ) | /* GP_LED_LD_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_098, 1 )   /* GP_LED_LD_3 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,
                 reg);

    /* GPIO 099..109 */
    reg =
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1 ) | /* GP_LED_LD_4 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1 ) | /* GP_LED_LD_5 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1 ) | /* GP_LED_LD_6 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1 ) | /* GP_LED_LD_7 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1 ) | /* SC_EXT_CLK */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1 ) | /* SC_IO_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1 ) | /* SC_CLK_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 1 ) | /* SC_RST_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107, 1 ) | /* SC_PRES_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 1 ) | /* SC_VCC_0 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_109, 3 )   /* NDS_SC_AUX0_2 */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,
                 reg);

    /* GPIO 110..112 SGPIO 00..09 */
    reg = BREG_Read32(hReg,
                      BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);

    reg = reg & BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, byp_sys9_clk);

    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 3 ) | /* NDS_SC_AUX1_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111, 3 ) | /* NDS_SC_VCTRL_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 2 ) | /* IRQ14 [Slot 1b] */
#if (BCHP_VER<BCHP_VER_E0)
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 0 ) | /* BSC_M0_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 0 ) | /* BSC_M0_SDA */
#else
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 1 ) | /* BSC_M0_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 1 ) | /* BSC_M0_SDA */
#endif
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02, 1 ) | /* BSC_M1_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03, 1 ) | /* BSC_M1_SDA */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04, 1 ) | /* BSC_M2_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05, 1 ) | /* BSC_M2_SDA */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06, 1 ) | /* BSC_M3_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07, 1 ) | /* BSC_M3_SDA */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_08, 1 ) | /* BSC_M4_SCL */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_09, 1 )   /* BSC_M4_SDA */
    ;

    BREG_Write32(hReg,
                 BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,
                 reg);

    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_OL -> UART2. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD1_OL);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL, reg);
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    /* Enable the UPG watchdog timer's chip reset feature */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_RESET_CTRL);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_RESET_CTRL,watchdog_reset_enable,1);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_RESET_CTRL, reg);

    /* Change the Clock to 81 Mhz(1) for UHF not default 27 Mhz(0)*/
    reg = BREG_Read32(hReg, BCHP_CLK_MISC);
    reg &= ~BCHP_MASK(CLK_MISC, UHFR_CLK_SEL);
    reg |= (BCHP_FIELD_DATA(CLK_MISC, UHFR_CLK_SEL, 1));
    BREG_Write32(hReg, BCHP_CLK_MISC , reg);

    return NEXUS_SUCCESS;
}

