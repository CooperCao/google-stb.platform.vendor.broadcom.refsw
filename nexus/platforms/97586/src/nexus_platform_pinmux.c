/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
    BOLT will set the initial pinmux. This function can overwrite any pin muxes for the 97586 reference platform
    Currently copied from 97364.
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* UART0 --> GPIO (000, 001) , refer to : SUN_TOP_CTRL_PIN_MUX_CTRL_3 */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_001) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_000)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_001, 2) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_000, 2)
           );
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

#if 0

   /* UART 1 --> GPIO ( 002, 003) refer to : SUN_TOP_CTRL_PIN_MUX_CTRL_4 */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_002) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_003)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_002, 2) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_003, 2)
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

   /* UART 2 --> GPIO ( 004, 005) refer to : SUN_TOP_CTRL_PIN_MUX_CTRL_4 */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
   reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_005) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_004)
            );
   reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_004, 2) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_005, 2)
           );

   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);
#endif


#if 0
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
#endif
   return BERR_SUCCESS;
}
