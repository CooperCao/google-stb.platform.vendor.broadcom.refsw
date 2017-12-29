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

/***************************************************************************
*
* This file is auto-generated
*
* This file maps the power resource control to register writes.
*
***************************************************************************/

#include "bchp.h"
#include "bchp_priv.h"
#include "bdbg.h"
#include "bkni.h"
#include "bchp_clkgen.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(BCHP_PWR_IMPL);

static void BCHP_PWR_P_DV_HVD_CTRL_CH0_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_HVD_CTRL_CH0_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_HVD_CTRL_CH1_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_HVD_CTRL_CH1_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_HVD_CTRL_CH2_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_HVD_CTRL_CH2_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_HVD_CTRL_CH5_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_HVD_CTRL_CH5_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_HVD_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_NETWORK_CTRL_CH5_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_NETWORK_CTRL_CH5_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_NETWORK_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_NETWORK_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_NETWORK_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_XPT_CTRL_CH0_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_XPT_CTRL_CH0_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_XPT_CTRL_CH1_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_XPT_CTRL_CH1_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_XPT_CTRL_CH2_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_XPT_CTRL_CH2_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_DIV);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, NDIV_INT);
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, PDIV);
    }
}

static void BCHP_PWR_P_DV_XPT_CTRL_CH4_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_XPT_CTRL_CH4_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_DIV);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, NDIV_INT);
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_XPT_PLL_DIV, PDIV);
    }
}

static void BCHP_PWR_P_HW_AIO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AIO: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO);
    mask = (BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO_AIO_ALTERNATE_SCB_CLOCK_ENABLE_AIO_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO_AIO_ALTERNATE_108_CLOCK_ENABLE_AIO_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO, reg);
}

static void BCHP_PWR_P_HW_AIO_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AIO_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = (BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_108_CLOCK_ENABLE_RAAGA0_MASK |
            BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_GISB_CLOCK_ENABLE_RAAGA0_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_AIO_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AIO_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AIO);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AIO_SRAM_PDA_IN_AIO_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AIO, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AIO);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AIO_SRAM_PDA_OUT_AIO_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_AIO_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_AIO_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AIO_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_AIO_SYSTEMPORT_54_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_ASP_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_ASP: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE_ASPT54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE_ASPT_GISB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE_ASPT_SCB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_ASP);
    mask = BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_ASP_ASP_SECTOP_PIPELINE_CLOCK_ENABLE_SECTOP_ASP_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_ASP, reg);
}

static void BCHP_PWR_P_HW_ASP_ASPT_ASP_540_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_ASP_ASPT_ASP_540: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE_ASPT_ASP_540_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_ASP_BVN_DVPHR0_DVPHT0_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_ASP_BVN_DVPHR0_DVPHT0_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE_ASPT_BVB_324_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_ASP_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_ASP_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_ASP_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_ASP);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_ASP_SRAM_PDA_IN_ASP_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_ASP, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_ASP);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_ASP_SRAM_PDA_OUT_ASP_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_ASP_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_BVN_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE_BVNT_648_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE_BVNT54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE_BVNT_SCB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_BVN_MIDDLE_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_BVN_MIDDLE_TOP_INST_CLOCK_ENABLE_BVNT54_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_BVN_MIDDLE_TOP_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_BVN_MVP_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_BVN_MVP_TOP_INST_CLOCK_ENABLE_BVND_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_BVN_MVP_TOP_INST_CLOCK_ENABLE_BVNT_SCB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_BVN_MVP_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_BVN_DVPHR0_DVPHT0_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN_DVPHR0_DVPHT0_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE_BVNT_BVB_324_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE_BVNT_GISB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_BVN_EDGE_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_BVN_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNE);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNE_SRAM_PDA_IN_BVNE_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNE, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNE);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNE_SRAM_PDA_OUT_BVNE_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_BVN_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNM);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNM_SRAM_PDA_IN_BVNM_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNM, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNM);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVNM_SRAM_PDA_OUT_BVNM_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_BVN_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVN);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVN_SRAM_PDA_IN_BVN_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVN, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVN);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_BVN_SRAM_PDA_OUT_BVN_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_BVN_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_DVPHR0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHR0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE_DVPHR_54_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE2);
    mask = BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE2_DVPHR_108_CLOCK_ENABLE2_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE2, reg);
}

static void BCHP_PWR_P_HW_DVPHR0_SECBUS_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHR0_SECBUS: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE_DVPHR_RBUS_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE_DVPHR_648_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE_DVPHR_27_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE_DVPHR_BVB_324_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE0);
    mask = BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE0_DVPHR_108_CLOCK_ENABLE0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_DVP_HR_INST_CLOCK_ENABLE0, reg);
}

static void BCHP_PWR_P_HW_DVPHR0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHR0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_DVPHR);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_DVPHR_SRAM_PDA_IN_DVPHR_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_DVPHR, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_DVPHR);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_DVPHR_SRAM_PDA_OUT_DVPHR_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_DVPHR0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_DVPHT0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHT0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE_DVPHT_BVB_324_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE_DVPHT_IIC_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE_DVPHT_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE_DVPHT_648_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE_DVPHT_54_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_DVP_HT_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_108_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_HVD0_CORE_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_HVD0_CORE: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_CORE_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_HVD0_CPU_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_HVD0_CPU: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_CPU_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_SECBUS_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_SECBUS: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE_HVD0_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD0_SRAM_PDA_IN_HVD0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD0_SRAM_PDA_OUT_HVD0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVD0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVD1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_54_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD1_HVD1_CORE_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD1_HVD1_CORE: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_CORE_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD1_HVD1_CPU_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD1_HVD1_CPU: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_CPU_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD1_SECBUS_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD1_SECBUS: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE_HVD1_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_HVD1_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD1_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD1_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD1);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD1_SRAM_PDA_IN_HVD1_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD1, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD1);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD1_SRAM_PDA_OUT_HVD1_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVD1_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVDP0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVDP0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP0_SRAM_PDA_IN_AVDP0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP0_SRAM_PDA_OUT_AVDP0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDP0_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP0_SRAM_PDA_IN_HVDP0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP0_SRAM_PDA_OUT_HVDP0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDP0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVDP1_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVDP1_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP1);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP1_SRAM_PDA_IN_AVDP1_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP1, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP1);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_AVDP1_SRAM_PDA_OUT_AVDP1_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDP1_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP1);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP1_SRAM_PDA_IN_HVDP1_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP1, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP1);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP1_SRAM_PDA_OUT_HVDP1_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDP1_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVDS0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVDS0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS0_SRAM_PDA_IN_HVDS0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS0_SRAM_PDA_OUT_HVDS0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDS0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVDS1_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVDS1_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS1);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS1_SRAM_PDA_IN_HVDS1_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS1, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS1);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS1_SRAM_PDA_OUT_HVDS1_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVDS1_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1);
    mask = (BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK |
            BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1_POST_DIVIDER_HOLD_CH1_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1, reg);
}

static void BCHP_PWR_P_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK |
            BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5);
    mask = (BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5_POST_DIVIDER_HOLD_CH5_MASK |
            BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5_CLOCK_DIS_CH5_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5, reg);
}

static void BCHP_PWR_P_HW_ITU656_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_ITU656_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC);
    mask = BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_BVB_216_CLOCK_ENABLE_VEC_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC, reg);
}

static void BCHP_PWR_P_HW_M2MC0_M2MC1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_M2MC0_M2MC1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0);
    mask = (BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_GFX_M2MC0_GISB_CLOCK_ENABLE_M2MC0_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_GFX_M2MC0_SCB_CLOCK_ENABLE_M2MC0_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0, reg);
}

static void BCHP_PWR_P_HW_M2MC0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_M2MC0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_M2MC0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_M2MC0_SRAM_PDA_IN_M2MC0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_M2MC0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_M2MC0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_M2MC0_SRAM_PDA_OUT_M2MC0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_M2MC0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_MMM2MC0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_MMM2MC0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_MMM2MC0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_MMM2MC0_SRAM_PDA_IN_MMM2MC0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_MMM2MC0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_MMM2MC0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_MMM2MC0_SRAM_PDA_OUT_MMM2MC0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_MMM2MC0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_MM_M2MC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_MM_M2MC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC1);
    mask = (BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC1_GFX_M2MC1_SCB_CLOCK_ENABLE_M2MC1_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC1_GFX_M2MC1_GISB_CLOCK_ENABLE_M2MC1_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC1, reg);
}

static void BCHP_PWR_P_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5);
    mask = (BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5_CLOCK_DIS_CH5_MASK |
            BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5_POST_DIVIDER_HOLD_CH5_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5, reg);
}

static void BCHP_PWR_P_HW_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = (BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_54_CLOCK_ENABLE_RAAGA0_MASK |
            BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_SCB_CLOCK_ENABLE_RAAGA0_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_RAAGA0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_RAAGA0);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_RAAGA0_SRAM_PDA_IN_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_RAAGA0, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_RAAGA0);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_RAAGA0_SRAM_PDA_OUT_RAAGA0_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_RAAGA0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0);
    mask = BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0_DSP_CLOCK_ENABLE_AIO_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_DSP_CLOCK_ENABLE_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_SC0_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_SC0_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_SC0_LDO_PWRON_LDO_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC0_LDO_PWRON_LDO_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_LDO_PWRON);
    mask = BCHP_CLKGEN_PLL_SC0_PLL_LDO_PWRON_LDO_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_LDO_PWRON, reg);
}

static void BCHP_PWR_P_HW_SC0_PWRON_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC0_PWRON_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_PWRON);
    mask = BCHP_CLKGEN_PLL_SC0_PLL_PWRON_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_PWRON, reg);
}

static void BCHP_PWR_P_HW_SC0_RESET_RESETD_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC0_RESET_RESETD: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_RESET);
    mask = (BCHP_CLKGEN_PLL_SC0_PLL_RESET_RESETA_MASK |
            BCHP_CLKGEN_PLL_SC0_PLL_RESET_RESETD_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC0_PLL_RESET, reg);
    if(activate)
        BKNI_Delay(30);
}

static void BCHP_PWR_P_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_SC1_LDO_PWRON_LDO_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC1_LDO_PWRON_LDO_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_LDO_PWRON);
    mask = BCHP_CLKGEN_PLL_SC1_PLL_LDO_PWRON_LDO_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_LDO_PWRON, reg);
}

static void BCHP_PWR_P_HW_SC1_PWRON_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC1_PWRON_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_PWRON);
    mask = BCHP_CLKGEN_PLL_SC1_PLL_PWRON_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_PWRON, reg);
}

static void BCHP_PWR_P_HW_SC1_RESET_RESETD_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC1_RESET_RESETD: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_RESET);
    mask = (BCHP_CLKGEN_PLL_SC1_PLL_RESET_RESETD_MASK |
            BCHP_CLKGEN_PLL_SC1_PLL_RESET_RESETA_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_RESET, reg);
    if(activate)
        BKNI_Delay(30);
}

static void BCHP_PWR_P_HW_SECBUS_VICE20_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SECBUS_VICE20: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_SECBUS_XPT_XPT_REMUX_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SECBUS_XPT_XPT_REMUX: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE_XPT_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_SYS_CTRL_SC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SYS_CTRL_SC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE_DISABLE_SC0_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_SYS_CTRL_SC1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SYS_CTRL_SC1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE_DISABLE_SC1_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_V3D_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_V3D: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE_V3D_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE_V3D_GISB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE_V3D_SCB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_V3D_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_V3D_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_V3D);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_V3D_SRAM_PDA_IN_V3D_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_V3D, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_V3D);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_V3D_SRAM_PDA_OUT_V3D_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_V3D_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_V3D_V3D_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_V3D_V3D: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE_V3D_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_V3D_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_1);
    mask = (BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_1_POST_DIVIDER_HOLD_CH1_MASK |
            BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_1, reg);
}

static void BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK |
            BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_LDO_PWRON);
    mask = BCHP_CLKGEN_PLL_VCXO0_PLL_LDO_PWRON_LDO_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_LDO_PWRON, reg);
}

static void BCHP_PWR_P_HW_VCXO0_PWRON_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_PWRON_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_PWRON);
    mask = BCHP_CLKGEN_PLL_VCXO0_PLL_PWRON_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_PWRON, reg);
    if(activate)
        BKNI_Delay(5);
}

static void BCHP_PWR_P_HW_VCXO0_RESET_RESETD_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO0_RESET_RESETD: %s", activate?"on":"off"));

    BSTD_UNUSED(reg);

    mask = (BCHP_CLKGEN_PLL_VCXO0_PLL_RESET_RESETA_MASK |
            BCHP_CLKGEN_PLL_VCXO0_PLL_RESET_RESETD_MASK);
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO0_PLL_RESET, mask, activate?0:mask);
    if(activate)
        BKNI_Delay(30);
}

static void BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_1);
    mask = (BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_1_POST_DIVIDER_HOLD_CH1_MASK |
            BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_1, reg);
}

static void BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK |
            BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_LDO_PWRON);
    mask = BCHP_CLKGEN_PLL_VCXO1_PLL_LDO_PWRON_LDO_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_LDO_PWRON, reg);
}

static void BCHP_PWR_P_HW_VCXO1_PWRON_PWRON_PLL_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_PWRON_PWRON_PLL: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_PWRON);
    mask = BCHP_CLKGEN_PLL_VCXO1_PLL_PWRON_PWRON_PLL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_PWRON, reg);
    if(activate)
        BKNI_Delay(5);
}

static void BCHP_PWR_P_HW_VCXO1_RESET_RESETD_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VCXO1_RESET_RESETD: %s", activate?"on":"off"));

    BSTD_UNUSED(reg);

    mask = (BCHP_CLKGEN_PLL_VCXO1_PLL_RESET_RESETA_MASK |
            BCHP_CLKGEN_PLL_VCXO1_PLL_RESET_RESETD_MASK);
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO1_PLL_RESET, mask, activate?0:mask);
    if(activate)
        BKNI_Delay(30);
}

static void BCHP_PWR_P_HW_VDAC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VDAC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC);
    mask = (BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC_VEC_BVB_216_CLOCK_ENABLE_VDAC_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC_VEC_GISB_CLOCK_ENABLE_VDAC_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC, reg);
}

static void BCHP_PWR_P_HW_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC);
    mask = (BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_BVB_BUS1_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_GISB_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_BVB_648_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_BVB_324_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_VEC_SCB_CLOCK_ENABLE_VEC_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC, reg);
}

static void BCHP_PWR_P_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0);
    mask = BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_GFX_M2MC0_CLOCK_ENABLE_M2MC0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0, reg);
}

static void BCHP_PWR_P_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_MM_M2MC0);
    mask = BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_MM_M2MC0_GFX_MM_M2MC0_CLOCK_ENABLE_MM_M2MC0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_MM_M2MC0, reg);
}

static void BCHP_PWR_P_HW_VEC_AIO_GFX_VEC_ITU656_0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC_AIO_GFX_VEC_ITU656_0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE_DISABLE_VEC_ITU656_0_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_VEC_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VEC);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VEC_SRAM_PDA_IN_VEC_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VEC, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VEC);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VEC_SRAM_PDA_OUT_VEC_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_VEC_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_VICE20_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VICE20: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_BVB_BUS1_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE_VICE_0_CORE_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_VEC_STG_CLK_CTRL);
    mask = BCHP_CLKGEN_VICE_0_INST_VEC_STG_CLK_CTRL_VICE_0_VEC_STG_CLK_CTRL_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_VICE_0_INST_VEC_STG_CLK_CTRL, reg);
}

static void BCHP_PWR_P_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK |
            BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1);
    mask = (BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK |
            BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1_POST_DIVIDER_HOLD_CH1_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1, reg);
}

static void BCHP_PWR_P_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK |
            BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4);
    mask = (BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4_CLOCK_DIS_CH4_MASK |
            BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4_POST_DIVIDER_HOLD_CH4_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4, reg);
}

static void BCHP_PWR_P_HW_XPT_REMUX_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_REMUX: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE);
    mask = (BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE_DISABLE_XPT_81_CLOCK_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE_DISABLE_XPT_40P5_CLOCK_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE_DISABLE_XPT_54_CLOCK_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE_DISABLE_XPT_27_CLOCK_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE_DISABLE_XPT_20P25_CLOCK_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_XPT_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_XPT);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_XPT_SRAM_PDA_IN_XPT_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_XPT, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_XPT);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_XPT_SRAM_PDA_OUT_XPT_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_XPT_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_XPT_XPT_REMUX_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_XPT_REMUX: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE_XPT_CORE_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE_XPT_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE_XPT_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE_XPT_54_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT);
    mask = BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT_XPT_SECTOP_PIPELINE_CLOCK_ENABLE_SECTOP_XPT_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT, reg);
}

static void BCHP_PWR_P_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_INTERNAL_MUX_SELECT, AUDIO0_OSCREF_CMOS_CLOCK, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_INTERNAL_MUX_SELECT, AUDIO0_OSCREF_CMOS_CLOCK);
    }
}

static void BCHP_PWR_P_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_INTERNAL_MUX_SELECT, AUDIO1_OSCREF_CMOS_CLOCK, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_INTERNAL_MUX_SELECT, AUDIO1_OSCREF_CMOS_CLOCK);
    }
}

static void BCHP_PWR_P_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_ITU656_0_MUX_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_ITU656_0_MUX_SELECT, VEC_ITU656_0_CLOCK, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_ITU656_0_MUX_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_ITU656_0_MUX_SELECT, VEC_ITU656_0_CLOCK);
    }
}

static void BCHP_PWR_P_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_SMARTCARD_MUX_SELECT_SC0_CLOCK: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SMARTCARD_MUX_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_SMARTCARD_MUX_SELECT, SC0_CLOCK, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_SMARTCARD_MUX_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_SMARTCARD_MUX_SELECT, SC0_CLOCK);
    }
}

static void BCHP_PWR_P_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_SMARTCARD_MUX_SELECT_SC1_CLOCK: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_SMARTCARD_MUX_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_SMARTCARD_MUX_SELECT, SC1_CLOCK, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_SMARTCARD_MUX_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_SMARTCARD_MUX_SELECT, SC1_CLOCK);
    }
}
