/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"
#include "bkni.h"

#include "bchp_sun_top_ctrl.h"

#include "bsagelib_priv.h"

#if BCHP_CHIP==7439 || BCHP_CHIP==74371
#include "bchp_aon_pin_ctrl.h"
#endif

BDBG_MODULE(BSAGElib_pinmux);

BERR_Code BSAGElib_P_Init_Serial(BSAGElib_Handle hSAGElib)
{
    uint32_t reg;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_P_Init_Serial);
    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    /* UART configurations are platform specific, not just chip specific. SAGE
       pin muxing for new chips should be set up in nexus_platform_pinmux.c,
       and compile conditionally on value of NEXUS_HAS_SAGE  */

    /* reg may not be referenced for newer chip ports */
    BSTD_UNUSED(reg);

    /* Set up GPIO */
#if BCHP_CHIP==7435
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
     * GPIO_067    : UART_RXD_1(4)
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_067));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_067, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_067_UART_RXD_1);  /* UART_RXD_1 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
     * GPIO_068    : UART_TXD_1 (4)
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_068));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_068, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_068_UART_TXD_1);  /* UART_TXD_1 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);
#endif
#if BCHP_CHIP==7584 || BCHP_CHIP==75845
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     * GPIO_89    : UART_TX1 (1)
     * GPIO_90    : UART_RX1 (1)
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_89) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90)
        );
    reg |=
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_89, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_89_UART_TX1) |  /* UART_TX1 */
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_90_UART_RX1);  /* UART_RX1 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif

#if BCHP_CHIP==7445
#if defined(NEXUS_USE_7252_VMS_SFF) || defined(NEXUS_USE_7445_VMS_SFF) || defined(NEXUS_USE_7445_C) || defined(NEXUS_USE_7252_C)
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     * GPIO_07    : TP_OUT_19
     * GPIO_06    : TP_IN_18
     * using UART ROUTER SEL 0 : port 1 cpu sel : SCPU
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007 )
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_006_TP_IN_18) | /* TP_IN_18 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_007_TP_OUT_19) );/* TP_OUT_19 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);
    /* Route SAGE UART to TP_IN_18/TP_OUT_19 */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_SCPU) ); /* SCPU */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
#else /* VMS_SFF */
#if (BCHP_VER==BCHP_VER_D0) || (BCHP_VER==BCHP_VER_E0)
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     * GPIO_10    : TP_IN_22 (4)
     * GPIO_11    : TP_IN_22 (4)
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_010 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_011 )
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_010, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_010_TP_IN_22) | /* TP_IN_22 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_011, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_011_TP_OUT_23) );/* TP_OUT_23 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);
#else /* BCHP_REV */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
     * GPIO_10    : TP_IN_22 (4)
     * GPIO_11    : TP_IN_22 (4)
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_010 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_011 )
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_010, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_010_TP_IN_22) | /* TP_IN_22 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_011, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_011_TP_OUT_23) );/* TP_OUT_23 */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);
#endif /* BCHP_REV */
    /* Route SAGE UART to TP_IN_22/TP_OUT_23 */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_2_cpu_sel_SCPU) ); /* SCPU */
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
#endif /* VMS_SFF */

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#endif /* BCHP_CHIP */
#if BCHP_CHIP==7439
#if BCHP_VER==BCHP_VER_A0
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
     * GPIO_16    : UART_RXD_2
     * GPIO_17    : UART_TXD_2
     */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17 )
        );
    reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_gpio_16_UART_RXD_2) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_gpio_17_UART_TXD_2) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

    /* Route UART to UART_RXD_2/UART_TXD_2 */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_8_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1_port_8_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#elif BCHP_VER==BCHP_VER_B0
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     * GPIO_006    : TP_IN_18
     * GPIO_007    : TP_OUT_19
     */
#if defined(NEXUS_USE_7439_SV) && defined(NEXUS_USE_7439_DR4)
    /* pinmux for Rx */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_042)
        );

    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_042, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_042_TP_OUT_20)
        );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* pinmux for Tx */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041)
        );

    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_041_TP_IN_19)
        );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    /* Route SAGE_UART to connector */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_5_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
#else
    /* pinmux for Rx */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007)
        );

    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_006_TP_IN_18) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_007_TP_OUT_19)
        );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

    /* Route SAGE_UART to connector */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
#endif

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#endif
#endif

#if BCHP_CHIP==7366
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074)
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_074_ALT_TP_IN_14) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_075_ALT_TP_OUT_15) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);


    /* Route UART to UART_RXD_2/UART_TXD_2 */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_11_cpu_sel) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_11_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1_port_11_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#endif

#if BCHP_CHIP==7364
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff, gpio_112) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff, gpio_114)
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff, gpio_112, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_114_ALT_TP_IN_02) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff, gpio_114, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_onoff_gpio_112_ALT_TP_OUT_02) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);


    /* Route UART to UART_RXD_2/UART_TXD_2 */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_2_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#endif

#if BCHP_CHIP==7250
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3
     * GPIO_058    : TP_IN_00
     */
    /* pinmux for Rx */
    reg = BREG_Read32(hSAGElib->core_handles.hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3_onoff, gpio_058));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3_onoff, gpio_058, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_onoff_gpio_058_TP_IN_00);
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4
     * GPIO_059    : TP_OUT_00
     */
    /* pinmux for Tx */
    reg = BREG_Read32(hSAGElib->core_handles.hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff, gpio_059));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff, gpio_059, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_onoff_gpio_059_TP_OUT_00);
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

    /* Route SAGE_UART to connector */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_1_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#endif

#if BCHP_CHIP==74371
#if defined(NEXUS_USE_74371_XID)
    /* configure UART2 as sage uart */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_104 ) |
        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_105 )
        );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_104, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_104_TP_IN_29) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_105, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_105_ALT_TP_OUT_06) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);

    /* Route SAGE_UART to connector */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_5_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);
#else /* SV */
     /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
      * AON_GPIO_12    : TP_IN_10
      * AON_GPIO_13    : ALT_TP_OUT_00
      */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &= ~(
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) |
        BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 )
        );
    reg |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_TP_IN_10) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_ALT_TP_OUT_00) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

    /* Route SAGE_UART to connector */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1);
    reg &=~( BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_7_cpu_sel ) );
    reg |= (BCHP_FIELD_DATA(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_7_cpu_sel, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1_port_7_cpu_sel_SCPU) );
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);
#endif
    /* enable mux inside sys_ctrl to ouput the uart router to the test port */
    reg = BREG_Read32(hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg = BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
    BREG_Write32 (hSAGElib->core_handles.hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#endif

    BDBG_LEAVE(BSAGElib_P_Init_Serial);
    return rc;
}
