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
#include "bchp_aon_pin_ctrl.h"


BDBG_MODULE(nexus_platform_pinmux);


/***************************************************************************
Summary:
    BOLT will set the initial pinmux. This function can overwrite any pin muxes for the 97366 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    /* SW7366-85: Added DVB-CI support */
#if NEXUS_HAS_DVB_CI
#if (BCHP_VER == BCHP_VER_A0)
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_001) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_002) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_003) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_004) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_005)
            );
   reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000, 0) | /* GPIO_000 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_001, 0) | /* GPIO_001 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_002, 0) | /* GPIO_002 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_003, 0) | /* GPIO_003 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_004, 0) | /* GPIO_004 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_005, 0) /* GPIO_005 */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_009) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_010)
            );
   reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006, 0) | /* GPIO_006 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007, 0) | /* GPIO_007 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008, 0) | /* GPIO_008 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_009, 0) | /* GPIO_009 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_010, 0) /* GPIO_010 */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_091) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_092) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_093)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_090, 0) | /* GPIO_090 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_091, 0) | /* GPIO_091 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_092, 0) | /* GPIO_092 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_093, 0) /* GPIO_093 */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_094) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_095) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_096) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_097)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_094, 0) | /* GPIO_094 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_095, 0) | /* GPIO_095 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_096, 0) | /* GPIO_096 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_097, 0) /* GPIO_097 */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);
#elif (BCHP_VER >= BCHP_VER_B0)
   reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
   reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07)
            );
   reg |= (
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07, 0)
           );
   BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

   reg = BREG_Read32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
   reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15)
            );
   reg |= (
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14, 0) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 0)
           );
   BREG_Write32(hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_000) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_001) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_002) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_003) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_004) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_005) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_006) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_007)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_000, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_001, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_002, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_003, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_004, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_005, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_006, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_007, 0)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_008) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_009) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_010) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_011) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_012) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_013) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_014) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_015)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_008, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_009, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_010, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_011, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_012, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_013, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_014, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_015, 1)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_016) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_017)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_016, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_017, 1)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_032) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_034) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_035)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_032, 3) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_034, 3) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_035, 3)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_046) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_047)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_046, 3) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_047, 0)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_048) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_049) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_050) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_051) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_052) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_053) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_054) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_055)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_048, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_049, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_050, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_051, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_052, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_053, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_054, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_055, 0)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_057)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_057, 3)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif
#endif

#if (BCHP_VER == BCHP_VER_A0)
   /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2.
      On 97366 A0 boards
        UART1 --> GPIO (121, 122) and port_1_cpu_sel
        UART2 --> GPIO (112, 114) and port_5_cpu_sel
    */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_121) |
               BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_122)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_121, 2) |
              BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_122, 2)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_112) |
               BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_114)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_112, 8) |
              BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_114, 8)
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);

   /*
    * Other possible values keeping it handy here for quick update: 
    * AUDIO_FP0,AUDIO_FP1,SHVD0_OL,SHVD0_IL, HVD1_OL, HVD1_IL, HVD2_OL, HVD2_IL, VICE20_ARC0, VICE20_ARC1,, VICE21_ARC0, VICE21_ARC1
    */

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel) |
            BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, HVD0_OL); /* outer loop */
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, HVD0_IL); /* inner loop */
   BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
#elif (BCHP_VER >= BCHP_VER_B0)
   /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2.
      On 97366 B0 boards
        UART1 --> GPIO (72, 73) and port_10_cpu_sel
        UART2 --> GPIO (74, 75) and port_11_cpu_sel
    */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
   /* UART 1 --> (GPIO 72, 73) */
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_072) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_073)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_072, 5) /* ALT_TP_IN_13 */ |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_073, 4) /* ALT_TP_OUT_13 */
           );
   /* UART 2 --> (GPIO 74, 75) */
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_074, 4) /* ALT_TP_IN_14 */ |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_075, 4) /* ALT_TP_OUT_14 */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);

   /*
    * Other possible values keeping it handy here for quick update:
    * AUDIO_FP0,AUDIO_FP1,SHVD0_OL,SHVD0_IL, HVD1_OL, HVD1_IL, HVD2_OL, HVD2_IL, VICE20_ARC0, VICE20_ARC1,, VICE21_ARC0, VICE21_ARC1
    */

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_10_cpu_sel) |
            BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_11_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_10_cpu_sel, HVD0_OL); /* outer loop */
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_11_cpu_sel, HVD0_IL); /* inner loop */
   BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_1, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
   /*
    * Only SV boards get this GPIO pin configured, others like BCM97366EXT8 configure these pin for other uses such as MTSIF interface.
    */
#if defined (NEXUS_USE_7366_SV) || defined (NEXUS_USE_7399_SV)
   /*
    * We need to set the GPIO pins for PKT2 so that ASI input is properly routed (SV boards only).
    */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_066) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_067) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_068) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_069) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_070)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_066, 2) | /* PKT_CLK2   */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_067, 2) | /* PKT_DATA2  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_068, 2) | /* PKT_SYNC2  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_069, 2) | /* PKT_ERROR2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_070, 1)   /* PKT_VALID2 */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);
#endif /* SV board ASI Input config */
#endif /* if BCHP_VER >= B0 */

   return BERR_SUCCESS;
}


