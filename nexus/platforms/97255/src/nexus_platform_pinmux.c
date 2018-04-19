/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#define DV_BOARD_ID 2
#define HB_BOARD_ID 6
#define TWO_L_BOARD_ID 12

#if NEXUS_HAS_SAGE
static void NEXUS_Platform_P_EnableSageDebugPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    if (!NEXUS_Platform_P_EnableSageLog()) {
        return;
    }

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Selecting SAGE pin Mux for board ID %d",platformStatus.boardId.major));

    switch (platformStatus.boardId.major) {

        default:
        {
            /* USFF boards don't have anything other than UART 0 headers */
            BDBG_MSG(("Unknown or no SAGE UART available on board type %d.",platformStatus.boardId.major));
            break;
        }
        case SV_BOARD_ID:
        {
            if (platformStatus.chipId == 0x73574) {
                /* GPIO_040 UART_TXD_0  */
                reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
                reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_040));
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_040, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_040_UART_TXD_0));
                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);
                /* GPIO_041 UART_RXD_0  */
                reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
                reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041));
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_041, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_041_UART_RXD_0));
                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

                /* AON_GPIO_12 UART_RXD_1 */
                /* AON_GPIO_13 UART_TXD_1 */
                reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                reg &= ~(BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12) |
                         BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13));
                reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_ALT_TP_IN_00) |
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_ALT_TP_OUT_00));
                BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

                /*Route SAGE to UART1 for now */
                reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_SCPU) );
                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

                /* AON_GPIO_16 UART_RXD_2 */
                /* AON_GPIO_17 UART_TXD_2 */


                /* finally explicitly set the test port for UART 1 and 2 */
                 reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
                 reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
                 reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
                 BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
            }
            /* 72550 and 72554 SVs are different designs. 7255[01] uses a 7250 SV but the schematics require some mapping */
            if (platformStatus.chipId < 0x72553) {

                /* 7250 GPIO 58/59 == 7255 GPIO 13/14 for UART 1 */
                /* 7250 GPIO 96/97 == 7255 GPIO 38/39 for UART 2 */

                /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
                 * GPIO_013    : TP_IN_00
                 * GPIO_014    : TP_OUT_00
                 */
                /* pinmux for Tx & Rx */
                reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
                reg &= ~(
                    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013) |
                    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014)
                    );
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_013_TP_IN_00) |
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_014_TP_OUT_00));

                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

                /* Route SAGE_UART to connector */
                reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_SCPU) );
                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

            } else { /* 72553/4 */

                /* UART 1 - Will require R3203 and R3204 moved to B-C position */

                /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
                 * GPIO_12     : ALT_TP_IN_04
                 * GPIO_13     : ALT_TP_OUT_04
                 */
                reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
                /* pinmux for Rx & Tx */
                reg &= ~(
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12) |
                    BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13)
                    );

                reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_ALT_TP_IN_00) |
                        BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_ALT_TP_OUT_00)
                    );
                BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

                /* Route SAGE_UART to connector */
                reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
                reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_0_cpu_sel ) );
                reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_0_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_SCPU) );

                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
            }

            break;
        }
        case HB_BOARD_ID:
        case DV_BOARD_ID:
        case TWO_L_BOARD_ID:
        {
            /* UART 1 */

            /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
             * GPIO_12     : ALT_TP_IN_04
             * GPIO_13     : ALT_TP_OUT_04
             */
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
            /* pinmux for Rx & Tx */
            reg &= ~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13)
                );

            reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_ALT_TP_IN_00) |
                    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_ALT_TP_OUT_00)
                );
            BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

            /* Route SAGE_UART to connector */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
            reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_0_cpu_sel ) );
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_0_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_SCPU) );

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

/***************************************************************************
Summary:
    Configure the pin muxing for the 97255 reference platforms.
Description:
    The core module must be initialised for this to be called.
 ***************************************************************************/

NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Board ID major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));

#if NEXUS_HAS_SAGE
    NEXUS_Platform_P_EnableSageDebugPinmux();
#endif

    return BERR_SUCCESS;
}
