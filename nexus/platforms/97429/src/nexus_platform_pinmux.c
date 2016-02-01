/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* Module Description:
*    This file contains pinmux initialization for the 97231 reference board.
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);

static NEXUS_Error NEXUS_Platform_P_InitPinmux_7429(void);
static NEXUS_Error NEXUS_Platform_P_InitPinmux_7428(void);
static NEXUS_Error NEXUS_Platform_P_InitPinmux_7241(void);

NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BCHP_Info chipInfo;
    BCHP_GetInfo(g_pCoreHandles->chp, &chipInfo);

    switch (chipInfo.productId)
    {
        case 0x7429:
        case 0x74295:
            return NEXUS_Platform_P_InitPinmux_7429();
        case 0x7428:
        case 0x74285:
            return NEXUS_Platform_P_InitPinmux_7428();
        case 0x7241:
        case 0x72415:
            return NEXUS_Platform_P_InitPinmux_7241();
        default:
            return NEXUS_INVALID_PARAMETER;
    }
}

/***************************************************************************
Summary:
    Configure pin muxes for the 97428 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
static NEXUS_Error NEXUS_Platform_P_InitPinmux_7428(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_033)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_034)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_035)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_036)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_037)
            );

    reg |=  (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_033, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_034, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_035, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_036, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_037, 2)
            );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047)
            );
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046, 4) |  /* PKT1_VALID  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047, 3) ;   /* PKT1_CLK */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048, 3) |  /* PKT1     */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049, 3) |  /* PKT1     */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050, 4) |  /* PM_GPIO_50    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051, 3) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052, 3) |  /* PKT2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053, 3) |  /* PKT2  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054, 3) |  /* PKT2   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055, 3);   /* PKT2   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056, 3) |  /* PKT2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062, 4) |  /* SC0  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063, 4) ;  /* SC0  */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 4) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070, 4) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071, 4) ;
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    /* Enable UART 1 for NFC */
#ifdef NEXUS_PLATFORM_97428_WIFI
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065, 4) | /* UART_TX1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066, 4) | /* UART_RX1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067, 5) | /* UART_RTS1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068, 5);  /* UART_CTS1 */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);
#endif

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073)
            );
    reg |= ( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 4) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 2)  /* RMX0   */
            );

    /* aon GPIO */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00));

    reg |= BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 );  /* aud spdif o/p */

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00, 1 ) /* MSC_M)_SCL.FPGA*/
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_02 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_03 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_02, 2 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_03, 2 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05, 1 )
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

#if !NEXUS_HAS_DVB_CI && !NEXUS_PLATFORM_97428_WIFI
    /* AVD UART */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065, 6) |  /* 6 = ALT_TP_OUT_02  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066, 6) ;  /* 6 = ALT_TP_IN_02   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);
#endif
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_InitPinmux_7429(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_033)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_034)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_035)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_036)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_037)
            );

    reg |=  (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_033, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_034, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_035, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_036, 2) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_037, 2)
            );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047)
            );
#if NEXUS_PLATFORM_4538_DBS
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046, 0) |  /* GPIO_046 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047, 0) ;  /* GPIO_047 */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046, 4) |  /* PKT1_VALID  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047, 3) ;   /* PKT1_CLK */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048, 3) |  /* PKT1     */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049, 3) |  /* PKT1     */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050, 4) |  /* PM_GPIO_50    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051, 3) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052, 3) |  /* PKT2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053, 3) |  /* PKT2  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054, 3) |  /* PKT2   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055, 3);   /* PKT2   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);
      reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056, 3) |  /* PKT2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062, 0) |  /* GPIO  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063, 0) ;  /* GPIO  */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 4) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070, 4) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071, 4) ;
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_079)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078, 0) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_079, 5) ;  /* NDS-SC0   */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082)   |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083)   |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084)   |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085)   |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)   |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 3) ;
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_092)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_093)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 3) |  /* mtsif */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 3) |  /* mtsif */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 3) ;  /* MTSIF   */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104)
            );

    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 4)|  /* mtsif */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 7)  /* MTSIF */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105, 6);  /* MTSIF */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)
            );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 7) | /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 7) | /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 7)   /* MTSIF */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

    /* aon GPIO */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00));

    reg |= BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 ); /* aud spdif o/p */

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 )
        );

    reg |=(
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00, 1 ) /* MSC_M)_SCL.FPGA*/
       );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_02 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_03 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05 )
        );
    reg |=(
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01, 1 ) |
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_02, 2 ) |
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_03, 2 ) |
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04, 1 ) |
       BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05, 1 )
       );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

#if !NEXUS_HAS_DVB_CI
    /* AVD UART */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 5) |  /* 5 = ALT_TP_OUT_07  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 5) ;  /* 5 = ALT_TP_IN_07   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_7_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_7_cpu_sel, SVD0_OL);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#if NEXUS_PLATFORM_4538_DBS
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03, 2) /* EXT_IRQB_1 */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, reg);
#endif

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_InitPinmux_7241(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /*
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0:gpio_00-gpio_07 programmed by CFE/OS
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1:gpio_08-gpio_15 programmed by CFE/OS
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2:gpio_16-gpio_25 programmed by CFE/OS
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3:gpio_26-gpio_33 programmed by CFE/OS
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4:gpio_34-gpio_41 programmed by CFE/OS
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5:gpio_42-gpio_45 programmed by CFE/OS
    */
#if USE_SPI_FRONTEND
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_015)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_015, 2);

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);
#endif

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_023)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016, 1) | /* EBI_WE1B */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_023, 1); /* EBI_CS3B      */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

#if USE_SPI_FRONTEND
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_024)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_024, 2);

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_038)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_039)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_038, 2) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_039, 2);

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);
#endif

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);

    reg &= ~(
#if USE_SPI_FRONTEND
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_040)  |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047)
            );

#if USE_SPI_FRONTEND
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_040, 2) |  /* EBI_ADDR14       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046, 3) |  /* EBI_ADDR14       */
#else
    reg |=      BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_046, 3) |  /* EBI_ADDR14               */
#endif

#if NEXUS_PLATFORM_7241_WIFI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047, 0)   /* GPIO_047    */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_047, 1)   /* POD2CHIP_MDI0    */
#endif
            ;   /* POD2CHIP         */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055)
            );
#if NEXUS_PLATFORM_7241_WIFI
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048, 0) |  /* GPIO_048    */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_048, 1) |  /* POD2CHIP_MDI1    */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_049, 1) |  /* POD2CHIP_MDI2    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_050, 1) |  /* POD2CHIP_MDI3    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_051, 1) |  /* POD2CHIP_MDI4    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_052, 1) |  /* POD2CHIP_MDI5    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_053, 1) |  /* POD2CHIP_MDI6   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_054, 1) |  /* POD2CHIP_MDI7    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_055, 1) ;  /* POD2CHIP_MISTRT  */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056, 1) |  /* POD2CHIP_MIVAL   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_057, 1) |  /* CHIP2POD_MCLKO   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_058, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_059, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_060, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_061, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_062, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_063, 1);  /* POD2CHIP_MDI3   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_064, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_065, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_066, 1) |  /* CHIP2POD_MOSTRT */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_067, 1) |  /* CHIP2POD_MOVAL  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_068, 3) |  /* EBI_ADDR13      */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_069, 3) |  /* EBI_ADDR12      */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_070, 2) |  /* EBI_ADDR2       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_071, 3);   /* EBI_ADDR1       */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_079)
            );

    reg |=
#if !NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 2) |  /* EBI_ADDR0       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_073, 1) |  /* MPOD SDI        */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_074, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_075, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_076, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_078, 1) |  /* RMX0   */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_072, 3) | /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_079, 1) ;  /* NDS-SC0   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_084)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_085)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_086)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_080, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_081, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_082, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_083, 1) |  /* NDS-SC0   */
#if NEXUS_PLATFORM_7241_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 3) ;  /* MTSIF   */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1) ;  /* PKT1   */
#endif

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

    /* gpio_0092-gpio_0093 uart0 set by CFE */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095)
            );
#if NEXUS_PLATFORM_7231_FBTSFF || NEXUS_PLATFORM_7231_EUSFF
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1) |  /* Legacy mode transport PKT1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1) |  /* Legacy mode transport PKT1 */
#elif NEXUS_PLATFORM_7241_DCSFBTSFF
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 3) |  /* MTSIF   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 3) |  /* MTSIF   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 3) |  /* MTSIF */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_088, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 2) |  /* PKT3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 2) |  /* PKT3   */
#endif
#if NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 4) |  /* POD   */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 3) |  /* POD   */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 4) ;  /* POD   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);

    /* gpio_0106 set by OS/CFE */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_096)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_097)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104)
            );

    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_096, 4) |  /* POD   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_097, 4) |  /* POD2CHIP_MCLKI   */
#if NEXUS_PLATFORM_7241_T2SFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 2) |  /* EXT_IRQ2 for 3461 & hmirx_nxp   */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 1) |  /* GP98_SF_HOLDb/(GP98_BCM3128_IRQb/GP98_DCARD1_IRQb)   */
#endif
#if !NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_099, 1) |  /* GP99_SF_WPb/(GP99_POD_IRQb) */
#endif
#if NEXUS_PLATFORM_7241_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 4)|  /* MTSIF*/
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1)|  /* PKT 2 */
#endif
#if NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1)  /* PKT3_SYNC */
#elif NEXUS_PLATFORM_7241_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 7)  /* PKT3_SYNC */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1)  /* PKT3_SYNC */
#endif
            );

    if(NEXUS_StrCmp(NEXUS_GetEnv("hddvi_width"), "30")==0) {
        BDBG_WRN(("enabling 30 bit hd-dvi"));
        reg |= ( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 2)|  /* HD_DVI0_0 */
                 BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 2)  /* HD_DVI0_10 */
                );
    }
    else {
        reg |= ( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)|  /* PKT 2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1)  /* PKT 2 */
            );
    }
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105));
#if NEXUS_PLATFORM_7241_DCSFBTSFF
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105, 6);  /* MTSIF */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105, 1);  /* PKT2_ERR */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* gpio_107,gpio_108,gpio_109,gpio_110,gpio_111,gpio_112,gpio_113,gpio_114
       set by CFE/OS for USB and ENET */
    /*  gpio_122 -  gpio_126 is TDB AON set by OS???? */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117)  |
#if NEXUS_HAS_DVB_CI
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_122)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_123)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_124)
#else
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)
#endif
            );

    reg |= (
#if NEXUS_PLATFORM_7231_FBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 1) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 1) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 1)   /* PKT_4 */
#elif NEXUS_PLATFORM_7241_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 7) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 7) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 7)   /* PKT_4 */
#elif NEXUS_PLATFORM_7231_EUSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 5) | /* TBD *** PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 5) | /* TBD *** PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 5)   /* HD_DVI0_11 */
#elif NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 1) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 4) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 4) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_122, 0) | /* GPIO_122 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_123, 0) | /* GPIO_123 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_124, 0)  /* GPIO_124 */
#elif NEXUS_PLATFORM_7241_T2SFF
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 0)    /* PKT_4 */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 1) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 0) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 4)   /* PKT_4 */
#endif
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

    /*  gpio_127 -  gpio_130 is TDB AON set by OS???? */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_128)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_129)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_130)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_131)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132)
            );


    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_128, 2) |   /* i2s input */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_129, 2) |   /* i2s input */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_130, 2) |   /* i2s input */
#if !NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_131, 1) |  /* TBD Schematics say GPIO_131/CARD_2/SDIO0_LED/VEC_HSYNC---> GP123_POD_VS1  */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132, 3)   /* HD_DVIO */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);


    /* gpio_139 - gpio_147 MII set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_133)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_134)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_139)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_140)
            );

    reg |= (
#if NEXUS_HAS_DVB_CI
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_133, 2) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135, 2) |  /* HD_DVIO */
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_133, 4) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135, 4) |  /* HD_DVIO */
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_134, 3) |   /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136, 5) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138, 4) |  /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_139, 0) |  /* tbd gpio */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_140, 3)   /* HD_DVI */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

    /* set by OS GP149_MDC_ENET */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_141)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_142)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_143) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_144) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_145) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_146) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_144) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148)
             );

    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_141, 3) |  /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_142, 4) |   /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_143, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_144, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_145, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_146, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_147, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148, 3)     /*  */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_00) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_01) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_02) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03)
             );

    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_00, 1) |   /* BSC_M3_SCL */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_01, 1) |   /* BSC_M3_SDA */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_02, 1) |   /* BSC_M4_SCL */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03, 1)     /* BSC_M4_SDA */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05)
            );

      reg |=(
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 ) | /* aud spdif o/p */
#if NEXUS_PLATFORM_7241_DCSFBTSFF
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 4)  | /* 3128_IRQ*/
#elif NEXUS_PLATFORM_7241_WIFI
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 0)  | /* AON_GPIO_4 */
#else
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 7)  | /* HD_DVI0_17 */
#endif

#if NEXUS_PLATFORM_7241_WIFI
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 0)   /* AON_GPIO_5 */
#else
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 7)    /* HD_DVI0_16 */
#endif
            );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
     *
    */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 )
            );

    reg |=(
#if NEXUS_PLATFORM_7241_WIFI
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 0 ) |  /* AON_GPIO_6 */
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 7 ) |  /* HD_DVI0_15 */
#endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 6 ) |  /* HD_DVI0_14 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 6 ) |  /* HD_DVI0_13 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 6 ) |  /* HD_DVI0_12 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 6 ) |  /* HD_DVI0_9 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 6 )    /* HD_DVI0_8 */
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 7 ) |  /* HD_DVI */
#if NEXUS_FRONTEND_4506
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 5 ) |  /* SPI_M_SS0B */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 5 ) |  /* SPI_M_SCK */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 5 ) |  /* SPI_M_MOSI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 5 ) |  /* SPI_M_MISO */
#else
#if NEXUS_PLATFORM_97428_WIFI
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 0 ) |  /* AON_GPIO_16 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 0 ) |  /* AON_GPIO_16 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 0 ) |  /* AON_GPIO_17 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 0 ) |  /* AON_GPIO_18 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 0 ) |  /* AON_GPIO_15 */
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 7 ) |   /* HD_DVI */
#endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 7 ) |  /* HD_DVI */
#endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20, 7 ) |   /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00, 1 )    /* BSC_M0_SCL */
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01, 1 ) | /* BSC_M0_SDA */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_04, 1 ) | /* (HDMI_Tx) BSC_M2_SCL */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_05, 1 )   /* (HDMI_Tx) BSC_M2_SDA */
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

#if 1
#if !NEXUS_HAS_DVB_CI
    /* AVD UARTS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_094, 4) |  /* 4 = ALT_TP_OUT_0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_095, 5) ;  /* 5 = ALT_TP_IN_0   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);
    /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, SVD0_IL);  */
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#endif

#if NEXUS_FRONTEND_4506
   /* Reprogram the pinmux for a 4506 daughtercard in the first daughtercard slot */

   /* First tuner is PKT1 */
   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_087, 1) ;  /* PKT1_CLK */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_089, 1) ;  /* PKT1_SYNC */
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_090, 1) ;  /* PKT1_VALID */
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_091, 1) ;  /* PKT1_ERROR */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);

   /* Second tuner is PKT2 */
   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1) ;  /* PKT2_CLK */
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1) ;  /* PKT2_DATA */
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1) ;  /* PKT2_SYNC */
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1) ;  /* PKT2_VALID */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_105, 1) ;  /* PKT2_ERROR */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

#if 0
   /* SPI */
   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_077, 2) ;  /* SPI_M_SS1B */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

   reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
   reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19 )
           );

   reg |=(
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 5 ) |  /* SPI_M_SS0B */
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 5 ) |  /* SPI_M_SCK */
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 5 ) |  /* SPI_M_MOSI */
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 5 )    /* SPI_M_MISO */
          );
   BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);
#endif

   /* external interrupt */
   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_098, 2) ;  /* EXT_IRQB_2 */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

   reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
   reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)
           );
   reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 0) ;  /* 0 */
   BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);
#endif

    return BERR_SUCCESS;
}
