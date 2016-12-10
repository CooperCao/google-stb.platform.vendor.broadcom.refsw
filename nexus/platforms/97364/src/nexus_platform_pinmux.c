/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
 * Module Description:
 *
 ***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

#if NEXUS_TRANSPORT_EXTENSION_TSIO
#include "bchp_sca.h"
#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif
#endif


BDBG_MODULE(nexus_platform_pinmux);


/***************************************************************************
Summary:
    BOLT will set the initial pinmux. This function can overwrite any pin muxes for the 97364 reference platform
    Currently copied from 97366.
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* UART0 --> GPIO (118, 117) */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_118) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_117)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_118, 1) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_117, 1)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

   /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2.
      On 97366 boards
        UART1 --> GPIO (083, 086) and port_1_cpu_sel
        UART2 --> GPIO (112, 114) and port_2_cpu_sel
    */

   /* UART 1 --> GPIO ( 083, 086) */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_083) |
#if NEXUS_TRANSPORT_EXTENSION_TSIO
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_084) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_086)
            );
#if NEXUS_TRANSPORT_EXTENSION_TSIO
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_083, 6) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_084, 0) | /* set as GPIO for TSIO RST_0   */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_086, 0)   /* set as GPIO for TSIO SC1_VCC */
           );
#else
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_083, 6) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_086, 6)
           );
#endif
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

   /* UART 2 --> GPIO ( 112, 114) */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_112) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_114)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_112, 8) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_114, 8)
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);

   /*
    * Other possible values keeping it handy here for quick update: 
    * AUDIO_FP0,AUDIO_FP1,SHVD0_OL,SHVD0_IL, HVD1_OL, HVD1_IL, HVD2_OL, HVD2_IL, VICE20_ARC0, VICE20_ARC1,, VICE21_ARC0, VICE21_ARC1
    */

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel) |
            BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, HVD0_OL); /* outer loop */
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, HVD0_IL); /* inner loop */
   BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0,reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);
   /*
    * We need to set the GPIO pins for PKT0 so that ASI input is properly routed
    */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016) | /* PKT0_DATA  */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014) | /* PKT0_CLK   */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_013) | /* PKT0_VALID */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_012)   /* PKT0_SYNC  */
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016, 3) | /* PKT0_DATA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014, 3) | /* PKT0_CLK   */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_013, 3) | /* PKT0_VALID */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_012, 3)   /* PKT0_SYNC  */
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_020)  /* PKT0_ERROR  */
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_020, 3)  /* PKT0_ERROR  */
          );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

#if NEXUS_HAS_DVB_CI
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_024)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_024, 1) /*  EBI_ADDR_0 */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_025) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_026) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_027) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_028) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_029) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_030) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_031) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_032)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_025, 1) | /*  EBI_ADDR_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_026, 1) | /*  EBI_ADDR_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_027, 1) | /*  EBI_ADDR_3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_028, 1) | /*  EBI_ADDR_4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_029, 1) | /*  EBI_ADDR_5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_030, 1) | /*  EBI_ADDR_6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_031, 1) | /*  EBI_ADDR_7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_032, 1) /*  EBI_ADDR_8 */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_033) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_034) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_035) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_036) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_037) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_038) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_039) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_033, 1) | /* EBI_ADDR_9 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_034, 1) | /* EBI_ADDR_10 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_035, 1) | /* EBI_ADDR_11 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_036, 1) | /* EBI_ADDR_12 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_037, 1) | /* EBI_ADDR_13 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_038, 1) | /* EBI_ADDR_14 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_039, 1) | /* EBI_RWB */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040, 1) /* EBI_WAITB */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_042) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_043) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_044) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_045) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_046) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_047) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_048)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_042, 1) | /* EBI_WE1B */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_043, 1) | /* EBI_DATA8 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_044, 1) | /* EBI_DATA9 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_045, 1) | /* EBI_DATA10 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_046, 1) | /* EBI_DATA11 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_047, 1) | /* EBI_DATA12 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_048, 1) /* EBI_DATA13 */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_049) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_050)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_049, 1) | /* EBI_DATA14 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_050, 1) /* EBI_DATA15 */
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_098) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_099)
            );

   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_098, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_099, 0)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_100) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_103) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_104) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_105)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_100, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_102, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_103, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_104, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_105, 0)
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#else
#if NEXUS_TRANSPORT_EXTENSION_TSIO
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040)  /* TSIO_VCTRL as gpio  */
           );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040, 0)  /* TSIO_VCTRL gpio */
          );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);

   BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_SMARTCARD);
   /* For gpio to work need to disable AFE_CMD_2 - set to zero, at reset = 1 */
   reg = BREG_Read32(hReg, BCHP_SCA_AFE_CMD_2);
   reg &= !(
            BCHP_MASK( SCA_AFE_CMD_2, power_dn ) |
            BCHP_MASK( SCA_AFE_CMD_2, bp_modeb )
           );
   BREG_Write32(hReg, BCHP_SCA_AFE_CMD_2, reg);
   BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_SMARTCARD);

#endif

#endif

   return BERR_SUCCESS;
}
