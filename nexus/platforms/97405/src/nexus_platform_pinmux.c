/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
*   This file contains pinmux initialization for the 97405 reference board.
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

BDBG_MODULE(nexus_platform_pinmux);

#define NEXUS_ENABLE_CPU_UART2 0

#if NEXUS_BOARD_7405_MSG
/***************************************************************************
Summary:
    Configure pin muxes for a 7405 MSG reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    const char * enableAvdUarts=NEXUS_GetEnv("enable_avd_uarts");

    BDBG_WRN(("Settings pinmux for 7405-MSG board"));

    /* Writing pinmuxes in order from pg. 8 of the schematic */
    /* GPIO 000..001 set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
     * GPIO3    : UART_RXD_0(2)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000)
            );
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001, 0) |  /* TBD BCM6816_RESET_N */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000, 3);   /* BCM7405_EXT_IRQ */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

   /* GPIO 002..019 are used for MII done in CFE*/
    if(enableAvdUarts)
    {
        BDBG_WRN(("Avd UARTS are enabled, disabling MII"));
        reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
        reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 ) /* | */
            );

        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 5 ) | /* UART 1 RX -> tp_in_07 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 5 ) | /* UART 1 TX -> tp_out_08 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 5 ) |  /* UART 2 RX -> tp_in_11 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 5 ) /* |  UART 2 TX -> tp_out_12 */
            );
        BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,reg);
    }

    /* GPIO 020..023 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 2 ) | /* BCM7405_IRQ13_N,4506_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021, 5 ) | /* SC_CLK_OUT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 2 ) | /* IRQ_11_SLOT0A_N,4506_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 2 )  /*  IRQ_12_SLOT0B_N ,4506_2*/
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,reg);

    /* GPIO 024..033 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033 )
            );

    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 0 ) | /* TP 1242 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 2 ) | /* GP_SPI_M_SCK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 2 ) | /* SPI_M_MSO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 2 ) | /* SPI_M_MISO  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 2 ) | /* SPI_M_SSOb */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 0 ) | /* GPIO29,BCM 5355 RST */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 03 )  /* TP */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,reg);

    /* GPIO 034-043 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 0 ) | /* TP  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 3 ) | /* UART_RXD_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 3 ) | /* UART_TXD_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043, 4 )   /* GP_NDS_SC_AUX0_1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);

    /* GPIO 044-053 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044, 4 ) | /* GP_NDS_SC_AUX1_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045, 4 ) | /* GP_NDS_SC_VCTRL_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047, 0 ) | /* TP  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051, 1 ) | /* GP_PKT_ERROR0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052, 1 ) | /* GP_PKT_ERROR1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 1 )   /* GP_PKT_ERROR2 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);

    /* GPIO 054-064 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054, 1 ) | /* GP_PKT_ERROR3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 1 ) | /* pkt_error4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 1 ) | /* pkt_error5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 1 ) | /* pkt_valid0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 1 ) | /* pkt_valid1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 1 ) | /* pkt_valid2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 1 ) | /* pkt_valid3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 1 ) | /* pkt_valid4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 1 ) | /* pkt_valid5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063, 1 ) | /* GP_PKT_CLK0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1 )   /* GP_PKT_CLK1 */

           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);


    /* GPIO 065-076 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065, 1 ) | /* GP_PKT_CLK2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066, 1 ) | /* GP_PKT_CLK3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067, 1 ) | /* GP_PKT_CLK4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 1 ) | /* GP_PKT_CLK5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, 1 ) | /* GP_PKT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070, 1 ) | /* GP_PKT_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071, 1 ) | /* GP_PKT_DATA2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 1 ) | /* GP_PKT_DATA3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1 ) | /* GP_PKT_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1 ) | /* GP_PKT_DATA5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1 ) | /* GP_PKT_SYNC0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1 )   /* GP_PKT_SYNC1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* GPIO 077-087 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077, 1 ) | /* GP_PKT_SYNC2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078, 1 ) | /* GP_PKT_SYNC3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079, 1 ) | /* GP_PKT_SYNC4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1 ) | /* GP_PKT_SYNC5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 0 ) | /* TP */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 4 ) | /* UART_RXD_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 4 ) | /* UART_TXD_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 0 ) | /* GPIO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 0 ) | /* GPIO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1)  | /* LED_KD_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1)    /* LED_KD_0(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 1) |  /*LED_LD_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 1) |  /*LED_LD_1(1)*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 1) |  /*LED_LD_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 1) |  /*LED_LS_4(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 1) |  /*LED_LS_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 1) |  /*LED_LS_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1) |  /*LED_LS_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1) |  /*LED_LS_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) |  /*LED_KD_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 1)    /*LED_KD_2(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

    /* GPIO 098-108 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)  | /* LED_LD_7 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1) |  /* LED_LD_6 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1) |  /* LED_LD_5 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1) |  /* LED_LD_4 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 1) |  /* LED_LD_3 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1 ) | /* GP_SC_EXT_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1 ) | /* GP_SC_IO_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1 ) | /* GP_SC_CLK_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 1 ) | /* GP_SC_RST_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107, 1 ) | /* GP_SC_PRES_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 1 )   /* GP_SC_VCC_0 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

    /* GPIO 109-112,sgpio_00-sgpio_07 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112 ) |

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109, 3 ) | /* UART_CTS_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 3 ) | /* UART_RTS_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111, 3 ) | /* UART_RXD_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 3 ) | /* UART_TXD_0 */

           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 1 ) |  /* GP_BSC_M0_SCL  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 1 ) |  /* GP_BSC_M0_SDA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02, 1 ) |  /* GP_BSC_M1_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03, 1 ) |  /* GP_BSC_M1_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04, 1 ) |  /* GP_BSC_M2_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05, 1 ) |  /* GP_BSC_M2_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06, 1 ) |  /* GP_BSC_M3_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07, 1 )    /* GP_BSC_M3_SDA */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_IL -> UART2. */
    if(enableAvdUarts)
    {
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel));
        reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
        reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL);
        BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
        reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
    }
    return BERR_SUCCESS;
}
#elif NEXUS_BOARD_7405_IFE
/***************************************************************************
Summary:
    Configure pin muxes for a 97405-IFE reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    const char * enableMII=NEXUS_GetEnv("enable_mii");

    BDBG_WRN(("Settings pinmux for 7405-IFE board"));

    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO 000..001 set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
     * GPIO3    : UART_RXD_0(2)
     * GPIO1    : ENET_LINK(1)
     * GPIO0    : ENET_ACTIVITY(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000)
            );
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000, 1);
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

    /* GPIO 000-019,should be set by OS for MII/ */

    /* GPIO 007..013 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008 ) |
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009 ) | */ /* Done in CFE */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010 ) | */ /* Done in CFE */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 ) /* | */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013 ) */ /* Done in CFE */
            );
    if(enableMII)
    {
        BDBG_WRN(("MII is enabled, disabling AVD UARTS"));
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 1 ) | /* mii_rxd_03 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 1 ) | /* mii_rx_er */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 1 ) | */ /* mii_tx_clk */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 1 ) | */ /* mii_tx_en */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 1 ) |  /* mii_txd_00 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 1 ) /* |  mii_txd_01 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 ) */   /* IRQ3 [POD] */
        );
    }
    else
    {
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 5 ) | /* UART 1 RX -> tp_in_07 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 5 ) | /* UART 1 TX -> tp_out_08 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 3 ) | */ /* TBD IRQ1 [3517 IRQ] */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 3 ) | */ /* IRQ2 [Test] */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 5 ) |  /* UART 2 RX -> tp_in_11 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 5 ) /* |  UART 2 TX -> tp_out_12 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 ) */   /* IRQ3 [POD] */
        );
    }


    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,reg);

    /* GPIO 015 & 020..023 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021 ) /*|
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023 )  */ /* TBD */
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 2 ) | /* GP_IRQ_13_SLOT1A_N */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021, 5 ) | /* SC_CLK_OUT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 2 ) /* |  IRQ_11_SLOT0A_N
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 2 )    IRQ_12_SLOT0B_N */ /* TBD */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,reg);

    /* GPIO 024..033 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033 )
            );

    reg |=(
#if NEXUS_NUM_656_INPUTS /* 8VDB demod can not be used if 656 input is enabled */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 1 ) | /* GP_VI_656_CLK */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 2 ) | /* GP_IRQ_10_3510_N. The GP_VI_656_CLK the jumper wire
                                                                           needs to be disconnected as they are GPIO pin is shared. */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 2 ) | /* GP_SPI_M_SCK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 2 ) | /* SPI_M_MSO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 2 ) | /* SPI_M_MISO  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 2 ) | /* SPI_M_SSOb */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 3 ) | /* pkt_clk2,conflicts GP_PPKT_IN_SYNC */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 3 ) | /* pkt_clk3,conflicts GP_PPKT_IN_VALID */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 3 ) | /* pkt_clk4,conflicts GP_PPKT_IN_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 3 ) | /* pkt_data2,conflicts GP_PPKT_IN_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 3 )   /* pkt_data3,conflicts GP_PPKT_IN_DATA2 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,reg);

    /* GPIO 034-043 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 3 ) | /* pkt_data4,conflicst GP_PPKT_IN_DATA3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 3 ) | /* pkt_sync2,conflicts GP_PPKT_IN_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 3 ) | /* pkt_sync3,conflicts GP_PPKT_IN_DATA5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 3 ) | /* pkt_sync4,conflicts GP_PPKT_IN_DATA6  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 2 ) | /* GP_PPKT_IN_DATA7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 2 ) | /* GP_PPKT_IN_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, 2 ) | /* GP_RMXP_OUT_VALID */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, 2 ) | /* GP_RMXP_OUT_SYNC */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, 2 ) | /* GP_RMXP_OUT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043, 4 )   /* GP_NDS_SC_AUX0_1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);

    /* GPIO 044-053 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044, 4 ) | /* GP_NDS_SC_AUX1_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045, 4 ) | /* GP_NDS_SC_VCTRL_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046, 2 ) | /* GP_RMXP_OUT_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047, 2 ) | /* GP_RMXP_OUT_DATA5  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048, 2 ) | /* GP_RMXP_OUT_DATA6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049, 2 ) | /* GP_RMXP_OUT_DATA7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 2 ) | /* GP_RMXP_OUT_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051, 1 ) | /* GP_PKT_ERROR0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052, 1 ) | /* GP_PKT_ERROR1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 1 )   /* GP_PKT_ERROR2 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);

    /* GPIO 054-064 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064 )
            );
    reg |=(
#if NEXUS_NUM_656_INPUTS /* accept 656 input */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 2 ) | /* GP_VI_656_D0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 2 ) | /* GP_VI_656_D1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 2 ) | /* GP_VI_656_D2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 2 ) | /* GP_VI_656_D3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 2 ) | /* GP_VI_656_D4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 2 ) | /* GP_VI_656_D5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 2 ) | /* GP_VI_656_D6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 1 ) | /* GP_VI_656_D7 */
#else

           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 1 ) | /* pkt_error4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 1 ) | /* pkt_error5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 1 ) | /* pkt_valid0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 1 ) | /* pkt_valid1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 1 ) | /* pkt_valid2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 1 ) | /* pkt_valid3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 1 ) | /* pkt_valid4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 1 ) | /* pkt_valid5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054, 1 ) | /* GP_PKT_ERROR3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063, 1 ) | /* GP_PKT_CLK0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1 )   /* GP_PKT_CLK1 */

           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);

#if (NEXUS_NUM_656_INPUTS) && (NEXUS_NUM_I2S_OUTPUTS)
#error "Please select either 656 input  OR I2S output , not both"
#endif

    /* GPIO 065-076 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076 )
            );
    reg |=(
#if NEXUS_NUM_I2S_INPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 2 ) | /* i2s_clk0_in */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 2 ) | /* i2s_data0_in */
#elif NEXUS_NUM_I2S_OUTPUTS /* loose PKT 5,streamer input */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 5 ) | /* i2s_clk0_out */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 5 ) | /* i2s_data0_out */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 1 ) | /* GP_PKT_CLK5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1 ) | /* GP_PKT_DATA5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065, 1 ) | /* GP_PKT_CLK2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066, 1 ) | /* GP_PKT_CLK3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067, 1 ) | /* GP_PKT_CLK4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, 1 ) | /* GP_PKT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070, 1 ) | /* GP_PKT_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071, 1 ) | /* GP_PKT_DATA2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 0 ) | /* Input to 7405 BCM3117_GPIO_12/TO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1 ) | /* GP_PKT_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1 ) | /* GP_PKT_SYNC0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1 )   /* GP_PKT_SYNC1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* GPIO 077-087 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)
            );
    reg |=(
#if NEXUS_NUM_I2S_INPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 2 ) | /* i2s_lr0_in */
#elif NEXUS_NUM_I2S_OUTPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 5 ) | /* i2s_lr0_out */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1 ) | /* GP_PKT_SYNC5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077, 1 ) | /* GP_PKT_SYNC2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078, 0 ) | /* 7405 Outout signal BCM3117_GPIO_10/CWD is a Clear Watchdog signal to the 3117,   GP_PKT_SYNC3 is still muxed in via a DN */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079, 1 ) | /* GP_PKT_SYNC4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1 ) | /* GP_SC_IO_1 */
#if NEXUS_FRONTEND_GPIO_INTERRUPT           
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 0 ) | /* 3117_IRQb,interrupt ,GP_SC_CLK_1 is still muxed in via a DNI */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 0 ) | /* 3112_IRQb,interrupt, GP_SC_RST_1 is still muxed in via a DNI */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 3 ) | /* 3117_IRQb,interrupt ,GP_SC_CLK_1 is still muxed in via a DNI */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 3 ) | /* 3112_IRQb,interrupt, GP_SC_RST_1 is still muxed in via a DNI */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 1 ) | /* GP_SC_PRES_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 1 ) | /* GP_SC_VCC_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1)  | /* LED_KD_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1)    /* LED_KD_0(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 1) |  /*LED_LD_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 1) |  /*LED_LD_1(1)*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 1) |  /*LED_LD_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 1) |  /*LED_LS_4(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 1) |  /*LED_LS_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 1) |  /*LED_LS_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1) |  /*LED_LS_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1) |  /*LED_LS_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) |  /*LED_KD_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 1)    /*LED_KD_2(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

    /* GPIO 098-108 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)  | /* LED_LD_7 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1) |  /* LED_LD_6 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1) |  /* LED_LD_5 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1) |  /* LED_LD_4 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 1) |  /* LED_LD_3 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1 ) | /* GP_SC_EXT_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1 ) | /* GP_SC_IO_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1 ) | /* GP_SC_CLK_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 1 ) | /* GP_SC_RST_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107, 1 ) | /* GP_SC_PRES_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 1 )   /* GP_SC_VCC_0 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

    /* GPIO 109-112,sgpio_00-sgpio_07 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110 ) |

            /* on bcm97405 P9 board used with 7405B0 uart 0 uses these lines
               do not change them here */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112 ) | */

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109, 0 ) | /* BCM3117_TC_TX_CLK,7405_SPI_M_SCK, upstream signal,input to 3117 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 0 ) | /* BCM3117_TC_TX_DATA,7405_SPI_M_MOSI, upstream signal,input to 3117 */

           /* on bcm97405 P9 board used with 7405B0 uart 0 uses these lines
              CFE sets them correctly do not change them here */
           /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111, 5 ) | *//* GP_NDS_SC_VCTRL_2 */
           /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 0 ) |  *//* ????GP_POD_VCC_ON */

           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 1 ) |  /* GP_BSC_M0_SCL  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 1 ) |  /* GP_BSC_M0_SDA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02, 1 ) |  /* GP_BSC_M1_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03, 1 ) |  /* GP_BSC_M1_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04, 1 ) |  /* GP_BSC_M2_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05, 1 ) |  /* GP_BSC_M2_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06, 1 ) |  /* GP_BSC_M3_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07, 1 )    /* GP_BSC_M3_SDA */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

    /* Configure the AVD UARTS to debug mode.  Default is AVD0_OL -> UART1, AUD_ZSP -> UART2. */
    if(enableMII==NULL)
    {
        uint32_t port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
        uint32_t port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AUDIO_ZSP);

        if(NEXUS_StrCmp(NEXUS_GetEnv("uart1_source"), "avd0_il")==0)
        {
            BDBG_WRN(("uart1_source=avd0_il"));
            port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_IL);
        } 
        else if(NEXUS_StrCmp(NEXUS_GetEnv("uart1_source"), "aud_zsp")==0)
        {
            BDBG_WRN(("uart1_source=aud_zsp"));
            port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AUDIO_ZSP);
        }

        if(NEXUS_StrCmp(NEXUS_GetEnv("uart2_source"), "avd0_il")==0)
        {
            BDBG_WRN(("uart2_source=avd0_il"));
            port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL);
        }
        else if(NEXUS_StrCmp(NEXUS_GetEnv("uart2_source"), "avd0_ol")==0)
        {
            port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_OL);
        }

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel));
        reg |= port1 | port2;
        BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
        reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
    }
    return BERR_SUCCESS;
}
#else
/***************************************************************************
Summary:
    Configure pin muxes for a 97405 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    const char * enableMII=NEXUS_GetEnv("enable_mii");

    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO 000..001 set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
     * GPIO3    : UART_RXD_0(2)
     * GPIO1    : ENET_LINK(1)
     * GPIO0    : ENET_ACTIVITY(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_003)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000)
            );
    reg |=
#if NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_003, 0) | /* GPIO (VPP2_EN0) */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 0) | /* GPIO (VPP2_EN1) */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_003, 1) | /* mii_rx_en */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 1) | /* mii_rx_clk */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000, 1);
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

    /* GPIO 000-019,should be set by OS for MII/ */

    /* GPIO 007..013 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_004 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_005 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008 ) |
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009 ) | */ /* Done in CFE */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010 ) | */ /* Done in CFE */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 ) /* | */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013 ) */ /* Done in CFE */
            );
    if(enableMII)
    {
        BDBG_WRN(("MII is enabled, disabling AVD UARTS"));
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_004, 1 ) | /* mii_rxd_00 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_005, 1 ) | /* mii_rxd_01 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 1 ) | /* mii_rxd_03 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 1 ) | /* mii_rx_er */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 1 ) | */ /* mii_tx_clk */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 1 ) | */ /* mii_tx_en */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 1 ) |  /* mii_txd_00 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 1 ) /* |  mii_txd_01 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 ) */   /* IRQ3 [POD] */
        );
    }
    else
    {
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_004, 0 ) | /* GPIO (VPP1_EN0) */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_005, 0 ) | /* GPIO (VPP1_EN1) */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_007, 5 ) | /* UART 1 RX -> tp_in_07 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_008, 5 ) | /* UART 1 TX -> tp_out_08 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_009, 3 ) | */ /* TBD IRQ1 [3517 IRQ] */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_010, 3 ) | */ /* IRQ2 [Test] */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 5 ) |  /* UART 2 RX -> tp_in_11 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 5 ) /* |  UART 2 TX -> tp_out_12 */
            /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_013, 4 ) */   /* IRQ3 [POD] */
        );
    }

#if NEXUS_ENABLE_CPU_UART2 || NEXUS_HAS_SOFT_AUDIO
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012 )
        );
    reg |= (
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_011, 2 ) |  /* UART 2 RX -> uart_rxd_2 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_012, 2 ) /* |  UART 2 TX -> uart_txd_2 */
        );
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,reg);

    /* GPIO 015 & 020..023 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_020, 2 ) | /* GP_IRQ_13_SLOT1A_N */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_021, 5 ) | /* SC_CLK_OUT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_022, 2 ) | /* IRQ_11_SLOT0A_N */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_023, 2 )  /*  IRQ_12_SLOT0B_N */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,reg);

    /* GPIO 024..033 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033 )
            );

    reg |=(
#if NEXUS_NUM_656_INPUTS /* 8VDB demod can not be used if 656 input is enabled */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 1 ) | /* GP_VI_656_CLK */
#else
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 0 ) | /* GPIO (CD2#) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 3 ) | /* EBI_ADDR_01 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_024, 2 ) | /* GP_IRQ_10_3510_N. The GP_VI_656_CLK the jumper wire
                                                                           needs to be disconnected as they are GPIO pin is shared. */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_025, 2 ) | /* GP_SPI_M_SCK */
#endif
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_026, 2 ) | /* SPI_M_MSO */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 3 ) | /* EBI_ADDR_00  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 3 ) | /* EBI_ADDR_02 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 2 ) | /* PPKT_SYNC (MOSTRT) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 2 ) | /* PPKT_VALID (MOVAL) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 2 ) | /* PPKT_DATA0 (MDO0) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 2 ) | /* PPKT_DATA1 (MDO1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 2 )   /* PPKT_DATA2 (MDO2) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_027, 2 ) | /* SPI_M_MISO  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_028, 2 ) | /* SPI_M_SSOb */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_029, 3 ) | /* pkt_clk2,conflicts GP_PPKT_IN_SYNC */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_030, 3 ) | /* pkt_clk3,conflicts GP_PPKT_IN_VALID */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_031, 3 ) | /* pkt_clk4,conflicts GP_PPKT_IN_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_032, 3 ) | /* pkt_data2,conflicts GP_PPKT_IN_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, 3 )   /* pkt_data3,conflicts GP_PPKT_IN_DATA2 */
#endif
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,reg);

    /* GPIO 034-043 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043 )
            );
    reg |=(
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 2 ) | /* PPKT_DATA3 (MDO3) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 2 ) | /* PPKT_DATA4 (MDO4) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 2 ) | /* PPKT_DATA5 (MDO5) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 2 ) | /* PPKT_DATA6 (MDO6)  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 2 ) | /* PPKT_DATA7 (MDO7) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 4 ) | /* EBI_ADDR13 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_034, 3 ) | /* pkt_data4,conflicst GP_PPKT_IN_DATA3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_035, 3 ) | /* pkt_sync2,conflicts GP_PPKT_IN_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_036, 3 ) | /* pkt_sync3,conflicts GP_PPKT_IN_DATA5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_037, 3 ) | /* pkt_sync4,conflicts GP_PPKT_IN_DATA6  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, 2 ) | /* GP_PPKT_IN_DATA7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, 2 ) | /* GP_PPKT_IN_CLK */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, 2 ) | /* GP_RMXP_OUT_VALID */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, 2 ) | /* GP_RMXP_OUT_SYNC */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, 2 ) | /* GP_RMXP_OUT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_043, 4 )   /* GP_NDS_SC_AUX0_1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);

    /* GPIO 044-053 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_044, 4 ) | /* GP_NDS_SC_AUX1_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_045, 4 ) | /* GP_NDS_SC_VCTRL_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_046, 2 ) | /* GP_RMXP_OUT_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_047, 2 ) | /* GP_RMXP_OUT_DATA5  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_048, 2 ) | /* GP_RMXP_OUT_DATA6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_049, 2 ) | /* GP_RMXP_OUT_DATA7 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 3 ) | /* EBI_ADDR12 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_050, 2 ) | /* GP_RMXP_OUT_CLK */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_051, 1 ) | /* GP_PKT_ERROR0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_052, 1 ) | /* GP_PKT_ERROR1 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 4 )   /* EBI_ADDR14 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_053, 1 )   /* GP_PKT_ERROR2 */
#endif
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);

    /* GPIO 054-064 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064 )
            );
    reg |=(
#if NEXUS_NUM_656_INPUTS /* accept 656 input */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 2 ) | /* GP_VI_656_D0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 2 ) | /* GP_VI_656_D1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 2 ) | /* GP_VI_656_D2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 2 ) | /* GP_VI_656_D3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 2 ) | /* GP_VI_656_D4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 2 ) | /* GP_VI_656_D5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 2 ) | /* GP_VI_656_D6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 1 ) | /* GP_VI_656_D7 */
#else

           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_055, 1 ) | /* pkt_error4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_056, 1 ) | /* pkt_error5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, 1 ) | /* pkt_valid0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, 1 ) | /* pkt_valid1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_059, 1 ) | /* pkt_valid2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_060, 1 ) | /* pkt_valid3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_061, 1 ) | /* pkt_valid4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_062, 1 ) | /* pkt_valid5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_054, 1 ) | /* GP_PKT_ERROR3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_063, 1 ) | /* GP_PKT_CLK0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1 )   /* GP_PKT_CLK1 */

           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);

#if (NEXUS_NUM_656_INPUTS) && (NEXUS_NUM_I2S_OUTPUTS)
#error "Please select either 656 input  OR I2S output , not both"
#endif

    /* GPIO 065-076 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076 )
            );
    reg |=(
#if NEXUS_NUM_I2S_INPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 2 ) | /* i2s_clk0_in */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 2 ) | /* i2s_data0_in */
#elif NEXUS_NUM_I2S_OUTPUTS /* loose PKT 5,streamer input */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 5 ) | /* i2s_clk0_out */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 5 ) | /* i2s_data0_out */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_068, 1 ) | /* GP_PKT_CLK5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1 ) | /* GP_PKT_DATA5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_065, 1 ) | /* GP_PKT_CLK2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_066, 1 ) | /* GP_PKT_CLK3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_067, 1 ) | /* GP_PKT_CLK4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, 1 ) | /* GP_PKT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_070, 1 ) | /* GP_PKT_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_071, 1 ) | /* GP_PKT_DATA2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 1 ) | /* GP_PKT_DATA3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1 ) | /* GP_PKT_DATA4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1 ) | /* GP_PKT_SYNC0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1 )   /* GP_PKT_SYNC1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* GPIO 077-087 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)
            );
    reg |=(
#if NEXUS_NUM_I2S_INPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 2 ) | /* i2s_lr0_in */
#elif NEXUS_NUM_I2S_OUTPUTS
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 5 ) | /* i2s_lr0_out */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1 ) | /* GP_PKT_SYNC5 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_077, 1 ) | /* GP_PKT_SYNC2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_078, 1 ) | /* GP_PKT_SYNC3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_079, 1 ) | /* GP_PKT_SYNC4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1 ) | /* GP_SC_IO_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 1 ) | /* GP_SC_CLK_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 1 ) | /* GP_SC_RST_1 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 0 ) | /* GPIO (READY/IREQ#) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 0 ) | /* GPIO (CD1#) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 1 ) | /* GP_SC_PRES_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 1 ) | /* GP_SC_VCC_1 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1)  | /* LED_KD_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1)    /* LED_KD_0(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_097, 1) |  /*LED_LD_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_096, 1) |  /*LED_LD_1(1)*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 1) |  /*LED_LD_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 1) |  /*LED_LS_4(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093, 1) |  /*LED_LS_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092, 1) |  /*LED_LS_2(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1) |  /*LED_LS_1(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1) |  /*LED_LS_0(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) |  /*LED_KD_3(1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 1)    /*LED_KD_2(1) */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

    /* GPIO 098-108 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)  | /* LED_LD_7 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1) |  /* LED_LD_6 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_100, 1) |  /* LED_LD_5 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1) |  /* LED_LD_4 (1) */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 1) |  /* LED_LD_3 (1) */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 0 ) | /* GPIO (VS1#) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1 ) | /* GP_SC_EXT_CLK */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1 ) | /* GP_SC_IO_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1 ) | /* GP_SC_CLK_0 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 0 ) | /* GPIO (VS2#) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_106, 1 ) | /* GP_SC_RST_0 */
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_107, 1 ) | /* GP_SC_PRES_0 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 0 )   /* GPIO (RESET) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_108, 1 )   /* GP_SC_VCC_0 */
#endif
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

    /* GPIO 109-112,sgpio_00-sgpio_07 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110 ) |

            /* on bcm97405 P9 board used with 7405B0 uart 0 uses these lines
               do not change them here */
            /* BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112 ) | */

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_109, 5 ) | /* GP_NDS_SC_AUX0_2 */
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 0 ) | /* GPIO (VCC_ON) */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_110, 5 ) | /* GP_NDS_SC_AUX1_2 */
#endif
           /* on bcm97405 P9 board used with 7405B0 uart 0 uses these lines
              CFE sets them correctly do not change them here */
           /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_111, 5 ) | *//* GP_NDS_SC_VCTRL_2 */
           /* BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_112, 0 ) |  *//* ????GP_POD_VCC_ON */

           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_00, 1 ) |  /* GP_BSC_M0_SCL  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_01, 1 ) |  /* GP_BSC_M0_SDA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_02, 1 ) |  /* GP_BSC_M1_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_03, 1 ) |  /* GP_BSC_M1_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_04, 1 ) |  /* GP_BSC_M2_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_05, 1 ) |  /* GP_BSC_M2_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_06, 1 ) |  /* GP_BSC_M3_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, sgpio_07, 1 )    /* GP_BSC_M3_SDA */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

    /* Configure the AVD UARTS to debug mode.  Default is AVD0_OL -> UART1, AUD_ZSP -> UART2. */
    if (enableMII==NULL)
    {
        uint32_t port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
#if NEXUS_ENABLE_CPU_UART2
        uint32_t port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, NO_CPU);
#else
        uint32_t port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AUDIO_ZSP);
#endif

        if(NEXUS_StrCmp(NEXUS_GetEnv("uart1_source"), "avd0_il")==0)
        {
            BDBG_WRN(("uart1_source=avd0_il"));
            port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_IL);
        } 
        else if(NEXUS_StrCmp(NEXUS_GetEnv("uart1_source"), "aud_zsp")==0)
        {
            BDBG_WRN(("uart1_source=aud_zsp"));
            port1 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AUDIO_ZSP);
        }

        if(NEXUS_StrCmp(NEXUS_GetEnv("uart2_source"), "avd0_il")==0)
        {
            BDBG_WRN(("uart2_source=avd0_il"));
            port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL);
        }
        else if(NEXUS_StrCmp(NEXUS_GetEnv("uart2_source"), "avd0_ol")==0)
        {
            port2 = BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_OL);
        }

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel));
        reg |= port1 | port2;
        BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
        reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
    }
    return BERR_SUCCESS;
}
#endif

