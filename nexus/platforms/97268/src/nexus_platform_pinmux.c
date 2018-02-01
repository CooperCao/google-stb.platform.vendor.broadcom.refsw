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

/***************************************************************************
Summary:
    Configure the pin muxing for the 97268 reference platforms.
Description:
    The core module must be initialised for this to be called.
 ***************************************************************************/

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
    BDBG_MSG(("Selecting SAGE pin mux for board ID %d",platformStatus.boardId.major));

    switch (platformStatus.boardId.major) {

        default:
        {
            /* USFF, VMS and HB boards don't have anything other than UART 0 headers */
            BDBG_MSG(("Unknown or no SAGE UART available on board type %d.",platformStatus.boardId.major));
            break;
        }
        case 1:  /* SV board */
        {
            /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1
             * GPIO_05    : ALT_TP_IN_04
             * GPIO_06    : ALT_TP_OUT_04
             */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
            /* pinmux for Rx & Tx */
            reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_005) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006)
                );

            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_005, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_005_ALT_TP_IN_04) |
                    BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_006_ALT_TP_OUT_04)
                );
            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);

            /* Route SAGE_UART to connector */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
            reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel ) );
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_2_cpu_sel_SCPU) );

            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
            break;
        }

        case 3: /* DV board */
        {
            /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
             * GPIO_12     : ALT_TP_IN_04
             * GPIO_13     : ALT_TP_OUT_04
             */
            reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
            /* pinmux for Rx & Tx */
            reg &= ~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13)
                );

            reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_TP_IN_05) |
                    BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_TP_OUT_05)
                );
            BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

            /* Route SAGE_UART to connector */
            reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
            reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_3_cpu_sel ) );
            reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_3_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_3_cpu_sel_SCPU) );

            BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
            break;
        }
    }

    /* enable mux inside sys_ctrl to output the uart router to the test port */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

}
#endif


NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    NEXUS_PlatformStatus platformStatus;
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Configure the streamer (BCM9TS) input to route into input band 3 for SV */

    NEXUS_Platform_GetStatus(&platformStatus);
    BDBG_MSG(("Board ID major: %d, minor: %d", platformStatus.boardId.major, platformStatus.boardId.minor));

    if (platformStatus.boardId.major == 1 /* SV */ ) {

        BDBG_MSG(("Configuring pin mux for BCM9TS streamer input to input band 3"));

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
        BDBG_MSG(("Before BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2: %08x",reg));
        reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016)
            );
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_013, 5) | /* PKT_CLK3  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_014, 5) | /* PKT_DATA3  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_015, 5) | /* PKT_SYNC3  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_016, 5)   /* PKT_VALID3  */
            );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
        BDBG_MSG(("After  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2: %08x",reg));

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
        BDBG_MSG(("Before BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3: %08x",reg));
        reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_017)
            );
        reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_017, 5) /* PKT_ERROR3  */
            );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);
        BDBG_MSG(("After  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3: %08x",reg));
    } else if (platformStatus.boardId.major == 3 /* DV */ ) {

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
    }
#if NEXUS_HAS_SAGE
    NEXUS_Platform_P_EnableSageDebugPinmux();
#endif

    return BERR_SUCCESS;
}
