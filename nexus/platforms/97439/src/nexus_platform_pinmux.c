/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);

NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
#if NEXUS_USE_7439_DR4
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8 );
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8 , gpio_006 ) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8 , gpio_007 )
           );
     reg |=(
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8 ,gpio_006, 5) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8 ,gpio_007, 5)
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);


    /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, HVD0_OL);
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#elif NEXUS_USE_7439_SFF && NEXUS_USE_3461_FRONTEND_DAUGHTER_CARD
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    BDBG_MSG(("Setting pinmux for BCM9TS_DC connector"));
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21);
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_00) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_01)
           );
    reg |=(
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_00, 1) | /*BSC_M3_SCL*/
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_01, 1)   /*BSC_M3_SDA*/
          );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016)
           );
    reg |=(
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 0) /*GPIO_016*/
          );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_072) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073)
           );
    reg |=(
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071, 1) | /*PKT_CLK0*/
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_072, 1) | /*PKT_DATA0*/
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073, 1)   /*PKT_SYNC0*/
          );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_074) |
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_075)
           );
    reg |=(
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_074, 1) | /*PKT_ERROR0*/
          BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_075, 1)   /*PKT_VALID0*/
          );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);

#endif


    return BERR_SUCCESS;
}
