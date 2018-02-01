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
#define DV_BOARD_ID 2
#define HB_BOARD_ID 6

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
            /* 7260 and 72603 SVs are different designs. 7260 uses a 7250 SV but the schematics require some mapping */
            if (platformStatus.chipId == 0x7260) {

                /* 7250 GPIO 58/59 == 7260 GPIO 13/14 for UART 1 */
                /* 7250 GPIO 96/97 == 7260 GPIO 38/39 for UART 2 */

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

            } else { /* 72603 and 72604 */

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
    Configure the pin muxing for the 97260 reference platforms.
Description:
    The core module must be initialised for this to be called.
 ***************************************************************************/

NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
#if NEXUS_HAS_DVB_CI
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
#endif
    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Board ID major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));

#if NEXUS_HAS_SAGE
    NEXUS_Platform_P_EnableSageDebugPinmux();
#endif
#if NEXUS_HAS_DVB_CI
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_007) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_008)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_006_GPIO_006) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_007_GPIO_007) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_008, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_008_GPIO_008)
        );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_009) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_009, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_009_GPIO_009) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_013_GPIO_013) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_014_GPIO_014) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2_gpio_015_GPIO_015)
        );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_038) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_039)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_033, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_033_GPIO_033) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_038, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_038_GPIO_038) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_039, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_039_GPIO_039)
        );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);

            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042)
                );

            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_042, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_042_GPIO_042)
                );
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_054) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_054, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_054_EBI_ADDR_13) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_056, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_056_EBI_ADDR_01)
        );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_057, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_057_EBI_ADDR_00) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_058, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_058_EBI_ADDR_02)
        );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069)
            );

    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_069, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_069_EBI_ADDR_12)
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);
#endif

    return BERR_SUCCESS;
}
