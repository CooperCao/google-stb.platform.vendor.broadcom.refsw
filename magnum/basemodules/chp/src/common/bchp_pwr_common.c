/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
* Module Description:
*
***************************************************************************/

#include "bchp_pwr_resources_priv.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_common.h"

#ifdef BCHP_PWR_HW_HDMI_TX0_PHY
#ifdef BCHP_HDMI_TX_PHY_REG_START
#include "bchp_hdmi_tx_phy.h"
#endif
static void BCHP_PWR_P_HW_HDMI_TX0_PHY_Control(BCHP_Handle handle, bool activate)
{
#ifdef BCHP_HDMI_TX_PHY_POWERDOWN_CTL
    uint32_t mask, reg;

    BDBG_MSG(("HW_HDMI_TX0_PHY: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_HDMI_TX_PHY_POWERDOWN_CTL);
    mask = ( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, RNDGEN_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_PWRDN)  |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_LDO_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BIAS_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, LDO_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BG_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PHY_PWRDN));
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_HDMI_TX_PHY_POWERDOWN_CTL, reg);
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(activate);
#endif
}
#endif

#ifdef BCHP_PWR_HW_HDMI_TX1_PHY
#ifdef BCHP_HDMI_TX_PHY_1_REG_START
#include "bchp_hdmi_tx_phy_1.h"
#endif
static void BCHP_PWR_P_HW_HDMI_TX1_PHY_Control(BCHP_Handle handle, bool activate)
{
#ifdef BCHP_HDMI_TX_PHY_1_POWERDOWN_CTL
    uint32_t mask, reg;

    BDBG_MSG(("HW_HDMI_TX_1_PHY: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_HDMI_TX_PHY_1_POWERDOWN_CTL);
    mask = ( BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, RNDGEN_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_PWRDN)  |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PLL_LDO_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BIAS_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, LDO_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, BG_PWRDN) |
             BCHP_MASK(HDMI_TX_PHY_POWERDOWN_CTL, PHY_PWRDN));
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_HDMI_TX_PHY_1_POWERDOWN_CTL, reg) ;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(activate);
#endif
}
#endif

#ifdef BCHP_PWR_HW_HDMI_RX0_PHY
#ifdef BCHP_DVP_HR_REG_START
#include "bchp_dvp_hr.h"
#endif
#ifdef BCHP_HDMI_RX_FE_0_REG_START
#include "bchp_hdmi_rx_fe_0.h"
#endif
static void BCHP_PWR_P_HW_HDMI_RX0_PHY_Control(BCHP_Handle handle, bool activate)
{
#if BCHP_HDMI_RX_FE_0_RESET_CONTROL || BCHP_DVP_HR_POWER_CONTROL
    uint32_t mask, reg;

    BDBG_MSG(("HW_HDMI_RX0_PHY: %s", activate?"on":"off"));

#ifdef BCHP_HDMI_RX_FE_0_RESET_CONTROL
    reg = BREG_Read32(handle->regHandle, BCHP_HDMI_RX_FE_0_RESET_CONTROL);
    mask = ( BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_PLL_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_CHANNEL_CLOCK_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_CHANNEL_0_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_CHANNEL_1_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_CHANNEL_2_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_ANALOG_BIAS_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_RESISTOR_TERMINATION_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_LDO_ILO_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_LDO_SL_POWER_DOWN_MASK |
             BCHP_HDMI_RX_FE_0_RESET_CONTROL_LDO_PLL_POWER_DOWN_MASK );
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_HDMI_RX_FE_0_RESET_CONTROL, reg);
#endif

#if BCHP_DVP_HR_POWER_CONTROL
    reg = BREG_Read32(handle->regHandle, BCHP_DVP_HR_POWER_CONTROL);
    mask = ( BCHP_DVP_HR_POWER_CONTROL_RX_PHY_0_POWER_DOWN_MASK );
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_DVP_HR_POWER_CONTROL, reg);
#endif
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(activate);
#endif
}
#endif

#ifdef BCHP_PWR_HW_AUD_PLL0
#include "bchp_aud_fmm_iop_pll_0.h"
static void BCHP_PWR_P_HW_AUD_PLL0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AUD_PLL0: %s", activate?"on":"off"));

    if(activate) {
        uint32_t reg, cnt=50;

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_PWRON, reg);

        BKNI_Delay(21);

#ifdef BCHP_CLKGEN_PLL_AUDIO0_AUDIO0
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_AUDIO0);
        mask = ( BCHP_CLKGEN_PLL_AUDIO0_AUDIO0_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO0_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_AUDIO0, reg);
#endif
#ifdef BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL);
        mask = ( BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL_PLL_AUDO0_ISO_OUT_SEL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL, reg);
#endif

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_RESET, reg);

        /* Check for PLL lock */
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_LOCK_STATUS);
            if (BCHP_GET_FIELD_DATA(reg, AUD_FMM_IOP_PLL_0_LOCK_STATUS, LOCK))
            break;
        }
        if(!cnt)
            BDBG_ERR(("HW_AUD_PLL0 Timeout"));
    } else {
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_RESET, reg);

#ifdef BCHP_CLKGEN_PLL_AUDIO0_AUDIO0
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_AUDIO0);
        mask = ( BCHP_CLKGEN_PLL_AUDIO0_AUDIO0_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO0_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_AUDIO0, reg);
#endif
#ifdef BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL);
        mask = ( BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL_PLL_AUDO0_ISO_OUT_SEL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_1P8V_TS28HPM_6MX_2MR_FC_X_E_PLLAUDIO0_INST_SEL, reg);
#endif
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_0_PWRON, reg);
    }
}
#endif

#ifdef BCHP_PWR_HW_AUD_PLL1
#include "bchp_aud_fmm_iop_pll_1.h"
static void BCHP_PWR_P_HW_AUD_PLL1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AUD_PLL1: %s", activate?"on":"off"));

    if(activate) {
        uint32_t reg, cnt=50;

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_PWRON, reg);

        BKNI_Delay(21);

#ifdef BCHP_CLKGEN_PLL_AUDIO1_AUDIO1
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_AUDIO1);
        mask = ( BCHP_CLKGEN_PLL_AUDIO1_AUDIO1_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO1_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_AUDIO1, reg);
#endif
#ifdef BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL);
        mask = ( BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL_PLL_AUDO0_ISO_OUT_SEL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL, reg);
#endif

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_RESET, reg);

        /* Check for PLL lock */
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_LOCK_STATUS);
            if (BCHP_GET_FIELD_DATA(reg, AUD_FMM_IOP_PLL_0_LOCK_STATUS, LOCK))
            break;
        }
        if(!cnt)
            BDBG_ERR(("HW_AUD_PLL1 Timeout"));
    } else {
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_RESET, reg);

#ifdef BCHP_CLKGEN_PLL_AUDIO1_AUDIO1
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_AUDIO1);
        mask = ( BCHP_CLKGEN_PLL_AUDIO1_AUDIO1_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO1_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_AUDIO1, reg);
#endif
#ifdef BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL);
        mask = ( BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL_PLL_AUDO0_ISO_OUT_SEL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_ONOFF_ANA_PLL4_RFMOD_1P8V_TS28HPM_6MX_2MR_NP_X_E_PLLAUDIO1_INST_SEL, reg);
#endif
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_1_PWRON, reg);
    }
}
#endif

#ifdef BCHP_PWR_HW_AUD_PLL2
#include "bchp_aud_fmm_iop_pll_2.h"
static void BCHP_PWR_P_HW_AUD_PLL2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AUD_PLL2: %s", activate?"on":"off"));

    if(activate) {
        uint32_t reg, cnt=50;

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_PWRON, reg);

        BKNI_Delay(21);

#ifdef BCHP_CLKGEN_PLL_AUDIO2_AUDIO2
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO2_AUDIO2);
        mask = ( BCHP_CLKGEN_PLL_AUDIO2_AUDIO2_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO2_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO2_AUDIO2, reg);
#endif

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_RESET, reg);

        /* Check for PLL lock */
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_LOCK_STATUS);
            if (BCHP_GET_FIELD_DATA(reg, AUD_FMM_IOP_PLL_0_LOCK_STATUS, LOCK))
            break;
        }
        if(!cnt)
            BDBG_ERR(("HW_AUD_PLL2 Timeout"));
    } else {
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_CONTROL_0);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0_HOLD_CH_ALL_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_CONTROL_0, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_RESET);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETD_MASK |
                 BCHP_AUD_FMM_IOP_PLL_0_RESET_RESETA_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_RESET, reg);

#ifdef BCHP_CLKGEN_PLL_AUDIO2_AUDIO2
        reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO2_AUDIO2);
        mask = ( BCHP_CLKGEN_PLL_AUDIO2_AUDIO2_PM_PLLL_LDO_POWERDOWN_PLL_AUDIO2_MASK );
        reg &= ~mask;
        reg |= mask;
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO2_AUDIO2, reg);
#endif

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_LDO_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_LDO_PWRON_LDO_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_LDO_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_BG_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_BG_PWRON_BG_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_BG_PWRON, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_PWRON);
        mask = ( BCHP_AUD_FMM_IOP_PLL_0_PWRON_PWRON_PLL_MASK );
        reg &= ~mask;
        BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_PLL_2_PWRON, reg);
    }
}
#endif

#ifdef BCHP_PWR_HW_AUD_DAC
#ifdef BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_REG_START
#include "bchp_aud_fmm_iop_out_dac_ctrl_0.h"
#endif
#include "bchp_aud_misc.h"

#ifdef BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK
#define BCHP_STB_power_rail_OK_REG    BCHP_AUD_MISC_CTRL
#define BCHP_STB_power_rail_OK_MASK   BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK
#elif BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_power_rail_OK_MASK
#define BCHP_STB_power_rail_OK_REG    BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2
#define BCHP_STB_power_rail_OK_MASK   BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_power_rail_OK_MASK
#endif

#ifdef BCHP_AUD_MISC_CTRL_STB_ready4sample_MASK
#define BCHP_STB_ready4sample_REG     BCHP_AUD_MISC_CTRL
#define BCHP_STB_ready4sample_MASK    BCHP_AUD_MISC_CTRL_STB_ready4sample_MASK
#elif BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3_STB_ready4sample_MASK
#define BCHP_STB_ready4sample_REG     BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3
#define BCHP_STB_ready4sample_MASK    BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3_STB_ready4sample_MASK
#endif

#ifdef BCHP_AUD_MISC_CTRL_STB_ready4pwdn_MASK
#define BCHP_STB_ready4pwdn_REG       BCHP_AUD_MISC_CTRL
#define BCHP_STB_ready4pwdn_MASK      BCHP_AUD_MISC_CTRL_STB_ready4pwdn_MASK
#elif BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3_STB_ready4pwdn_MASK
#define BCHP_STB_ready4pwdn_REG       BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3
#define BCHP_STB_ready4pwdn_MASK      BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_3_STB_ready4pwdn_MASK
#endif

#define BCHP_REG(Reg) BCHP_##Reg

static void BCHP_PWR_P_HW_AUD_DAC_Control(BCHP_Handle handle, bool activate)
{
#if defined(BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_pu_MASK) || defined(BCHP_STB_power_rail_OK_MASK) || defined(BCHP_STB_ready4sample_MASK)
    uint32_t mask, reg;
#else
    BSTD_UNUSED(handle);
#endif

    BDBG_MSG(("HW_AUD_DAC: %s", activate?"on":"off"));

#ifdef BCHP_STB_power_rail_OK_MASK
    if (activate)
    {
        reg = BREG_Read32(handle->regHandle, BCHP_STB_power_rail_OK_REG);
        reg &= ~BCHP_STB_power_rail_OK_MASK;
        reg |= BCHP_STB_power_rail_OK_MASK;
        BREG_Write32(handle->regHandle, BCHP_STB_power_rail_OK_REG, reg);
        /* Delay > 50 us */
        BKNI_Delay(100);
    }
#endif

#ifdef BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_pu_MASK
    reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2);
    mask = BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_pu_MASK ;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2, reg);
#endif

#ifdef BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1_STB_CP_ext_pok_en_MASK
    reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1);
    if (activate) {
        mask = (BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1_STB_CP_ext_pok_MASK |
                BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1_STB_CP_ext_pok_en_MASK);
        reg &= ~mask;
        reg |= mask;
        BKNI_Sleep(20);
    } else {
        reg &= ~BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1_STB_CP_ext_pok_en_MASK;
        BKNI_Sleep(10);
    }
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_1, reg);
#endif

#ifdef BCHP_STB_ready4sample_MASK
    {
        unsigned i, count = 100000; /* up to 100 milliseconds */
        for ( i = 0 ; i < count; i++ )
        {
            uint32_t ready = BREG_Read32(handle->regHandle, BCHP_STB_ready4pwdn_REG);
            mask = activate ? BCHP_STB_ready4sample_MASK : BCHP_STB_ready4pwdn_MASK;
            if ( (ready & mask) > 0 )
            {
                break;
            }
            BKNI_Delay(1); /* 1us */
        }
        if (count == 0)
        {
            BDBG_WRN(("TIMEOUT powering %s DAC", activate ? "UP" : "DOWN"));
        }
    }
#endif

#ifdef BCHP_STB_power_rail_OK_MASK
    if (!activate)
    {
        reg = BREG_Read32(handle->regHandle, BCHP_STB_power_rail_OK_REG);
        reg &= ~BCHP_STB_power_rail_OK_MASK;
        BREG_Write32(handle->regHandle, BCHP_STB_power_rail_OK_REG, reg);
    }
#endif
}
#endif

#ifdef BCHP_PWR_HW_RFM_PHY
#ifdef BCHP_RFM_SYSCLK_REG_START
#include "bchp_rfm_sysclk.h"
#endif
static void BCHP_PWR_P_HW_RFM_PHY_Control(BCHP_Handle handle, bool activate)
{
    bool rfmCapable;
#ifdef BCHP_RFM_SYSCLK_CLKCTL
    uint32_t mask, reg;

#ifdef BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1_otp_option_rfm_disable_MASK
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1);
    if(reg & BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1_otp_option_rfm_disable_MASK)
    return;
#endif

    if((BCHP_GetFeature(handle, BCHP_Feature_eRfmCapable, (void*) &rfmCapable) != BERR_SUCCESS) ||
       !rfmCapable) {
        BDBG_MSG(("RFM is not supported"));
        return;
    }

    BDBG_MSG(("HW_RFM_PHY: %s", activate?"on":"off"));

    BSTD_UNUSED(mask);

    if(activate) {
        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg &= ~BCHP_RFM_SYSCLK_CLKCTL_RFMCLK_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg &= ~BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg &= ~BCHP_RFM_SYSCLK_CLKCTL_LDO_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_DPLL_MISC1);
        reg &= ~( BCHP_RFM_SYSCLK_DPLL_MISC1_ARESET_MASK |
                  BCHP_RFM_SYSCLK_DPLL_MISC1_DRESET_MASK |
                  BCHP_RFM_SYSCLK_DPLL_MISC1_PWRDN_MASK );
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_DPLL_MISC1, reg);

        BKNI_Delay(50); /* wait 50us for RFM PLL to lock */

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg &= ~BCHP_RFM_SYSCLK_CLKCTL_CLK_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_DACCTL);
        reg &= ~BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_DACCTL, reg);
    } else {
        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_DACCTL);
        reg |= BCHP_RFM_SYSCLK_DACCTL_DAC_PWRDN_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_DACCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg |= BCHP_RFM_SYSCLK_CLKCTL_CLK_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_DPLL_MISC1);
        reg |= ( BCHP_RFM_SYSCLK_DPLL_MISC1_ARESET_MASK |
                 BCHP_RFM_SYSCLK_DPLL_MISC1_DRESET_MASK |
                 BCHP_RFM_SYSCLK_DPLL_MISC1_PWRDN_MASK );
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_DPLL_MISC1, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg |= BCHP_RFM_SYSCLK_CLKCTL_LDO_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg |= BCHP_RFM_SYSCLK_CLKCTL_BGCORE_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);

        reg = BREG_Read32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL);
        reg |= BCHP_RFM_SYSCLK_CLKCTL_RFMCLK_OFF_MASK;
        BREG_Write32(handle->regHandle, BCHP_RFM_SYSCLK_CLKCTL, reg);
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(activate);
    BSTD_UNUSED(rfmCapable);
#endif
}
#endif
