/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

/* Define GOLd_BOARD to setup SHVD UARTS on GOLD BOARD, default is silver board */
#define GOLD_BOARD 0

BDBG_MODULE(nexus_platform_pinmux);


/***************************************************************************
Summary:
    Configure pin muxes for the 97425 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* need special routing */
    /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);

    /*
     * Other possible values keeping it handy here for quick update:
     * AUDIO_FP0,AUDIO_FP1,SHVD0_OL,SHVD0_IL, HVD1_OL, HVD1_IL, HVD2_OL, HVD2_IL, VICE20_ARC0, VICE20_ARC1,, VICE21_ARC0, VICE21_ARC1
     */
#if NEXUS_HAS_DVB_CI
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_034) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_035) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_036) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_034, 3) | /* EBI_ADDR1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_035, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_036, 3) | /* EBI_ADDR0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, 3)   /* EBI_ADDR2 */
        );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_086) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_086, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089, 0)
        );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_093)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_090, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_093, 0)
        );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105, 0)
        );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_106)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_106, 0)
        );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21, reg);
#endif
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, HVD0_IL);
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, HVD0_OL);

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#if NEXUS_USE_7252S_VMS_SFF
    /* bit needs to be set on VMS board */
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_NO_SCAN_0,0x0001B6FE);
#endif

    return BERR_SUCCESS;
}
