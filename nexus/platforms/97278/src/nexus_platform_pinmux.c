/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
#include "nexus_types.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"

#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);

#define SV_BOARD_ID 1
#define HB_BOARD_ID 2
#define VMS_BOARD_ID 3

#if NEXUS_HAS_SAGE
static void NEXUS_Platform_P_EnableSageDebugPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    const char *pinmux_env = NEXUS_GetEnv("sage_log");
    if (pinmux_env) {
        int pinmux_env_val = NEXUS_atoi(pinmux_env);
        if (pinmux_env_val != 1) {
            return; /* Only enable pin mux if this is set to 1 */
        }
    }

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Selecting SAGE pin mux for board ID %d",platformStatus.boardId.major));

    switch (platformStatus.boardId.major) {

        default:
        {
            /* VMS and HB boards don't have anything other than UART 0 headers */
            BDBG_MSG(("Unknown or no SAGE UART available on board type %d.",platformStatus.boardId.major));
            break;
        }
        case SV_BOARD_ID:
        {
            /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
             * GPIO_06    : TP_IN_18
             * GPIO_07    : TP_OUT_19
             */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
            /* pinmux for Rx & Tx */
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007)
                );

            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_006_TP_IN_18) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_007_TP_OUT_19)
                );
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

            /* Route SAGE_UART to connector */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
            reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_SCPU) );

            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
            break;
        }
    }

    /* Enable mux inside sys_ctrl to output the uart router to the test port */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

}
#endif

static void NEXUS_Platform_P_EnableHvdUartPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    int hvd_env_val = -1;

    const char *uart_env = NEXUS_GetEnv("hvd_uart");
    if (uart_env) {
        hvd_env_val = NEXUS_atoi(uart_env);
    }
    if ((hvd_env_val != 0) && (hvd_env_val != 1)) {
        return; /* Only enable pin mux if this is set to valid HVD number */
    }

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_WRN(("Selecting HVD%d IL -> UART2, OL -> UART1 pin mux for board ID %d",hvd_env_val,platformStatus.boardId.major));

    switch (platformStatus.boardId.major) {
        default:
        {
            /* HB boards don't have anything other than UART 0 headers */
            BDBG_MSG(("Unknown or no HVD UART available on board type %d.",platformStatus.boardId.major));
            break;
        }
        case SV_BOARD_ID:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039)
                );

            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_038, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_038_TP_IN_17) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_039, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_039_TP_OUT_18)
                );
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007)
                );

            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_006_TP_IN_18) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_007_TP_OUT_19)
                );
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
            /* Activate the test port */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
            reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

            switch (hvd_env_val) {
                case 0:
                {
                    /* Route HVD0_UARTs to connector */
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_HVD0_OL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1_port_8_cpu_sel_HVD0_IL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);

                    break;
                }
                case 1:
                {   /* Route HVD1_UARTs to connector */
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_HVD1_OL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1_port_8_cpu_sel_HVD1_IL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);

                    break;
                }
                /* default unreachable */
            }
        }
        case VMS_BOARD_ID:
        {
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
            reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040));
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_040, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_040_TP_IN_19));
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
            reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_041));
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_041, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_041_TP_OUT_20));
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);
            /* Activate the test port */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
            reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

            switch (hvd_env_val) {
                case 0:
                {
                    /* Route HVD0 OL UART to connector */
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_5_cpu_sel_HVD0_OL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
                    break;
                }
                case 1:
                {   /* Route HVD1 OL UART to connector */
                    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel ) );
                    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_5_cpu_sel_HVD1_OL) );
                    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
                    break;
                }
                /* default unreachable */
            }
        }
    }

}

/***************************************************************************
Summary:
    Configure the pin muxing for the 97278 reference platforms.
Description:
    The core module must be initialised for this to be called.
 ***************************************************************************/

NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Configure the streamer (BCM9TS) input to route into input band 0 for SV */

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Board ID major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));

    if (platformStatus.boardId.major == SV_BOARD_ID) {

        BDBG_MSG(("Configuring pin mux for BCM9TS streamer input to input band 0"));

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
        BDBG_MSG(("Before BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));
        reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_071) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_072)
            );
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_071,
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_071_PKT_CLK0) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_072,
                            BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_072_PKT_DATA0)
            );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);
        BDBG_MSG(("After  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10: %08x",reg));

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
        BDBG_MSG(("Before BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11: %08x",reg));
        reg &= ~(
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_073) |
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_074) |
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_075)
            );
        reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_073,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_073_PKT_SYNC0) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_074,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_074_PKT_ERROR0) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_075,
                                BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_075_PKT_VALID0)
            );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
        BDBG_MSG(("After  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11: %08x",reg));
    }

#if NEXUS_HAS_SAGE
    NEXUS_Platform_P_EnableSageDebugPinmux();
#endif
    NEXUS_Platform_P_EnableHvdUartPinmux();
    return BERR_SUCCESS;
}
