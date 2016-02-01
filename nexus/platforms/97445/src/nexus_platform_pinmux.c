/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
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
    Configure pin muxes for the 97445 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/

/* pimux are now set in the bolt, for any change to the default pinmux
   open a Bolt Jira.
*/

#if BCHP_VER >= BCHP_VER_D0
#if NEXUS_USE_7445_SV || NEXUS_USE_7445_C
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t mask, value;

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, TP_IN_19); /* UART PORT 5 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_042, TP_OUT_20); /* UART PORT 5 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_060, GPIO_060); /* UART PORT 5 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_043, TP_IN_21); /* UART PORT 6 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_044, TP_OUT_22); /* UART PORT 6 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_010, GPIO_010); /* UART PORT 6 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_068, GPIO_068); /* UART PORT 7 */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069, GPIO_069); /* UART PORT 7 */

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_050, GPIO_050 ); /* BSC_M4_SCL */
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_051, GPIO_051 );
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_053, GPIO_053 );
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, mask, value);

#if NEXUS_HAS_DVB_CI
    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_034, EBI_ADDR_01);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_035, GPIO_035);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_036, EBI_ADDR_00);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, EBI_ADDR_02);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_086, GPIO_086);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088, GPIO_088);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089, GPIO_089);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_090, GPIO_090);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_093, GPIO_093);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104, GPIO_104);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105, GPIO_105);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20, mask, value);

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_106, GPIO_106);
#endif
    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_5_cpu_sel, HVD0_OL); /* Debug UART 5 set for outer loop */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_6_cpu_sel, HVD0_IL); /* Debug UART 6 set for inner loop */

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_UART_ROUTER_SEL_1, port_7_cpu_sel, HVD0_ILP2); /* Debug UART 7 set for inner loop*/

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable, SYS);

    return BERR_SUCCESS;
}
#elif NEXUS_USE_7252_SV
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
#if NEXUS_HAS_DVB_CI
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t mask, value;
    BREG_Handle hReg = g_pCoreHandles->reg;

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_034, EBI_ADDR_01);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_035, GPIO_035);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_036, EBI_ADDR_00);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, EBI_ADDR_02);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_086, GPIO_086);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088, GPIO_088);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089, GPIO_089);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_090, GPIO_090);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_093, GPIO_093);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19, mask, value);

    BCHP_UPDATE_INIT(mask,value);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104, GPIO_104);
    BCHP_UPDATE_FIELD_ENUM(mask, value, SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105, GPIO_105);
    BREG_Update32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20, mask, value);

    BREG_SET_FIELD_ENUM(hReg, SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_106, GPIO_106);
#endif
    return BERR_SUCCESS;
}

#else
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    return BERR_SUCCESS;
}
#endif
#endif
