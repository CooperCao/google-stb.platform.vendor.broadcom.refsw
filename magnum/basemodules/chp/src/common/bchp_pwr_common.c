/***************************************************************************
*     Copyright (c) 2006-2015, Broadcom Corporation*
*     All Rights Reserved*
*     Confidential Property of Broadcom Corporation*
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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

#include "bchp_pwr_resources_priv.h"
#include "bchp_sun_top_ctrl.h"

#ifdef BCHP_PWR_HW_HDMI_TX0_PHY
#include "bchp_hdmi_tx_phy.h"
static void BCHP_PWR_P_HW_HDMI_TX0_PHY_Control(BCHP_Handle handle, bool activate)
{
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
}
#endif

#ifdef BCHP_PWR_HW_HDMI_TX1_PHY
#include "bchp_hdmi_tx_phy_1.h"
static void BCHP_PWR_P_HW_HDMI_TX1_PHY_Control(BCHP_Handle handle, bool activate)
{
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
}
#endif

#ifdef BCHP_PWR_HW_HDMI_RX0_PHY
#include "bchp_dvp_hr.h"
#include "bchp_hdmi_rx_fe_0.h"
static void BCHP_PWR_P_HW_HDMI_RX0_PHY_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HDMI_RX0_PHY: %s", activate?"on":"off"));

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

    reg = BREG_Read32(handle->regHandle, BCHP_DVP_HR_POWER_CONTROL);
    mask = ( BCHP_DVP_HR_POWER_CONTROL_RX_PHY_0_POWER_DOWN_MASK );
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_DVP_HR_POWER_CONTROL, reg);
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
#include "bchp_aud_fmm_iop_out_dac_ctrl_0.h"
#include "bchp_aud_misc.h"
static void BCHP_PWR_P_HW_AUD_DAC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AUD_DAC: %s", activate?"on":"off"));

#ifdef BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK
    if (activate)
    {
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_MISC_CTRL);
        reg &= ~BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK;
        reg |= BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK;
        BREG_Write32(handle->regHandle, BCHP_AUD_MISC_CTRL, reg);
        /* Delay > 50 us */
        BKNI_Delay(100);
    }
#endif

    reg = BREG_Read32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2);
    mask = BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2_STB_pu_MASK ;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_AUD_FMM_IOP_OUT_DAC_CTRL_0_ANALOG_CTRL_REG_2, reg);

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

#ifdef BCHP_AUD_MISC_CTRL_STB_ready4sample_MASK
    {
        unsigned i, count = 100000; /* up to 100 milliseconds */
        for ( i = 0 ; i < count; i++ )
        {
            uint32_t ready = BREG_Read32(handle->regHandle, BCHP_AUD_MISC_CTRL);
            mask = activate ? BCHP_AUD_MISC_CTRL_STB_ready4sample_MASK : BCHP_AUD_MISC_CTRL_STB_ready4pwdn_MASK;
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

#ifdef BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK
    if (!activate)
    {
        reg = BREG_Read32(handle->regHandle, BCHP_AUD_MISC_CTRL);
        reg &= ~BCHP_AUD_MISC_CTRL_STB_power_rail_OK_MASK;
        BREG_Write32(handle->regHandle, BCHP_AUD_MISC_CTRL, reg);
    }
#endif
}
#endif

#ifdef BCHP_PWR_HW_RFM_PHY
#include "bchp_rfm_sysclk.h"
static void BCHP_PWR_P_HW_RFM_PHY_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

#ifdef BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1_otp_option_rfm_disable_MASK
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1);
    if(reg & BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_1_otp_option_rfm_disable_MASK)
    return;
#endif

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
}
#endif
