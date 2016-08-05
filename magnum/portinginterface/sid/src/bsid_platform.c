/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
******************************************************************************/

/*
    BSID Platform specific functionality
*/
#include "bstd.h"
#include "bdbg.h"

#include "bchp_sid.h"   /* for access to status register */

#include "bchp_sid_arc.h"
#include "bchp_sid_arc_core.h"
#include "bchp_sid_arc_dbg.h"

#include "bsid_fw_api.h"
#include "bsid_dbg.h"
#include "bsid_platform.h"

BDBG_MODULE(BSID_PLATFORM);

/******************************************************************************
* Function name: BSID_P_ArcSoftwareReset
*   -
* Comments:
*   -
******************************************************************************/
void BSID_P_ArcSoftwareReset(BREG_Handle hReg)
{
   /* NOTE: All chips SID core can be reset via GR Bridge SW INIT.
      But the location of the GR Bridge registers can vary
      For chips that support SW Reset via Sundry SW INIT, we use that for consistency */
#if  ((BCHP_CHIP==7425)  || \
      (BCHP_CHIP==7344)  || \
      (BCHP_CHIP==7346)  || \
      (BCHP_CHIP==73465))
    /* These chips do not support Reset via Sundry, only via GFX GR Bridge */
    BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, SID_CLK_108_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, SID_CLK_108_SW_INIT, DEASSERT));
#elif ((BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7435)  || \
       (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==75845) || \
       (BCHP_CHIP==7429)  || \
       (BCHP_CHIP==74295) || \
       (BCHP_CHIP==7445)  || \
       (BCHP_CHIP==7145)  || \
       (BCHP_CHIP==7366)  || \
       (BCHP_CHIP==7364)  || \
       (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==7271)  || \
       (BCHP_CHIP==7260)  || \
       (BCHP_CHIP==7268)  || \
       (BCHP_CHIP==7439)  || \
       (BCHP_CHIP==7586)  || \
       (BCHP_CHIP==74371))
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init, 1));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_1_CLEAR, sid_sw_init, 1));
#endif
}

/******************************************************************************
* Function name: BSID_P_ChipEnable
*
* Comments:
*
******************************************************************************/
void BSID_P_ChipEnable(BREG_Handle hReg, uint32_t uiBasePhysAddr)
{
    BSID_P_EnableUart(hReg);

    /* wake up sid arc */
    BREG_Write32(hReg, BCHP_SID_ARC_CPU_INST_BASE, uiBasePhysAddr);
    BREG_Write32(hReg, BCHP_SID_ARC_CPU_END_OF_CODE, BSID_FW_ARC_END_OF_CODE);
    BREG_Write32(hReg, BCHP_SID_ARC_DBG_CPU_DBG, 1);
    /* NOTE: The following is actually writing to the ARC Status register (which contains the PC) */
    BREG_Write32(hReg, BCHP_SID_ARC_CORE_CPU_PC, BSID_FW_ARC_RESET_ADDRESS);
    BREG_Write32(hReg, BCHP_SID_ARC_DBG_CPU_DBG, 0);
}

/******************************************************************************
* Function name: BSID_P_DisableArc
*
* Comments:
*
******************************************************************************/
void BSID_P_HaltArc(BREG_Handle hReg)
{
    BREG_Write32(hReg, BCHP_SID_ARC_DBG_CPU_DBG, 1);
    BREG_Write32(hReg, BCHP_SID_ARC_CORE_CPU_PC, BSID_FW_ARC_HALT);
}

/******************************************************************************
* Function name: BSID_P_DisableWatchdog
*
* Comments:
*
******************************************************************************/
/* NOTE: The FW will enable the watchdog - this is to allow us to disable it
   for a "restart" */
void BSID_P_DisableWatchdog(BREG_Handle hReg)
{
    BSTD_UNUSED(hReg);
}

/******************************************************************************
* Function name: BSID_P_ReadSIDStatus_isr
*
* Comments:
*
******************************************************************************/
/* Read SID HW status and Arc PC for debug info upon watchdog */
void BSID_P_ReadSIDStatus_isr(BREG_Handle hReg, uint32_t *puiSidStatus, uint32_t *puiArcPC)
{
   BREG_Write32(hReg, BCHP_SID_ARC_DBG_CPU_DBG, 1);
   *puiArcPC = BREG_Read32(hReg, BCHP_SID_ARC_CORE_CPU_PC);
   BREG_Write32(hReg, BCHP_SID_ARC_DBG_CPU_DBG, 0);

   *puiSidStatus = BREG_Read32(hReg, BCHP_SID_STATUS);
}

/******************************************************************************
* Function name: BSID_P_PlatformEnableUart
*   -
* Comments:
*   -
******************************************************************************/
#ifdef BSID_P_DEBUG_ENABLE_ARC_UART
void BSID_P_PlatformEnableUart(BREG_Handle hReg)
{
/* 7440 */
/* FIXME: 7440 is obsolete */
#if (BCHP_CHIP == 7440)

    uint32_t ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_44));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, ui32_reg | (3<<BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_44_SHIFT));

#if (BCHP_VER == BCHP_VER_A0)
    ui32_reg =  BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_60));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, ui32_reg | (4<<BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_60_SHIFT) );
#else
    /* everything except A0 */
    ui32_reg =  BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, sgpio_08));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, ui32_reg | (4<<BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_sgpio_08_SHIFT) );
#endif

    ui32_reg =  BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    ui32_reg &= ~0xF00;
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL, ui32_reg | (BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_3_cpu_sel_SID << BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_3_cpu_sel_SHIFT));


#elif ((BCHP_CHIP==7425)  || \
       (BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7344)  || \
       (BCHP_CHIP==7346)  || \
       (BCHP_CHIP==73465))

#if (BCHP_CHIP==7425)
    uint32_t ui32_reg;

    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_6_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_5_cpu_sel));
    ui32_reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_6_cpu_sel, SID);
    ui32_reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_5_cpu_sel, SID);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,ui32_reg);

    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    ui32_reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, ui32_reg);
#endif

#if 0 /* 35230C0 as of 082111 */
    /* rxd */
    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, rdb));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, (ui32_reg | (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_rdb_TP_IN_26 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3_rdb_SHIFT)));

    /* txd */
    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, tdb));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, (ui32_reg | (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_tdb_TP_OUT_00 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4_tdb_SHIFT)));

    /* uart selector: tp 1 selected */
    ui32_reg =  BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    ui32_reg &= ~BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL,port_1_cpu_sel);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL, (ui32_reg | (BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_1_cpu_sel_SID << BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_1_cpu_sel_SHIFT)));

    /* enable SUNDRY block from the test port ctlr */
    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, (ui32_reg | BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN)));
#endif

#if 0
    /* rxd */
    uint32_t ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_46));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, ui32_reg | (6<<BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_46_SHIFT));

    /* txd */
    ui32_reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    ui32_reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_43));
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, ui32_reg | (6<<BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_43_SHIFT));

    /* uart selector: tp 1 selected */
    ui32_reg =  BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    ui32_reg &= ~0xF0;
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL, ui32_reg | (BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_1_cpu_sel_SID << BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_port_1_cpu_sel_SHIFT));
#endif /* if 0 */

#endif /* not 7440 A0 */
/* FIXME: What about everything else? Is UART enabled by default for those chips? */
}
#endif /* ifdef BSID_P_DEBUG_ENABLE_ARC_UART */


/*
  End of File
*/
