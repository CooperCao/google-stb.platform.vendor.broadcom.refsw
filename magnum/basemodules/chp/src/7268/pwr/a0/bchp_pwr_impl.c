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

static void BCHP_PWR_P_DV_AVX_CTRL_CH0_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_AVX_CTRL_CH0_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV, *prediv);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT, *mult);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_DV_AVX_CTRL_CH1_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_AVX_CTRL_CH1_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT);
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT, *mult);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV, *prediv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_DV_AVX_CTRL_CH2_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_AVX_CTRL_CH2_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV, *prediv);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT, *mult);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_DV_AVX_CTRL_CH3_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_AVX_CTRL_CH3_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3, MDIV_CH3);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3, MDIV_CH3, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV, *prediv);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT, *mult);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_DV_AVX_CTRL_CH4_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_AVX_CTRL_CH4_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV);
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, PDIV, *prediv);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_AVX_PLL_DIV, NDIV_INT, *mult);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_DV_CPU_CTRL_CH2_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_CPU_CTRL_CH2_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV, PDIV);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV_4K);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV_4K, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_CPU_CTRL_CH3_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_CPU_CTRL_CH3_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3, MDIV_CH3);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3, MDIV_CH3, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV, PDIV);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV_4K);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV_4K, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_CPU_CTRL_CH4_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_CPU_CTRL_CH4_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV, PDIV);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV_4K);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV_4K, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_CPU_CTRL_CH5_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_CPU_CTRL_CH5_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV);
    if(!set) {
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV, PDIV);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_DIV_4K);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_CPU_PLL_DIV_4K, NDIV_INT);
    }
}

static void BCHP_PWR_P_DV_RAAGA_CTRL_CH0_div_Control(BCHP_Handle handle, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{
    uint32_t reg;

    BDBG_MSG(("DV_RAAGA_CTRL_CH0_div: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0);
    if(!set) {
        *postdiv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, *postdiv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0, reg);
    }
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_DIV);
    if(!set) {
        *mult = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_DIV, NDIV_INT);
        *prediv = BCHP_GET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_DIV, PDIV);
    } else {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_DIV, NDIV_INT, *mult);
        BCHP_SET_FIELD_DATA(reg, CLKGEN_PLL_RAAGA_PLL_DIV, PDIV, *prediv);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_DIV, reg);
    }
}

static void BCHP_PWR_P_HW_AIO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AIO: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO);
    mask = (BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO_ALTERNATE_108_CLOCK_ENABLE_AIO_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO_SYSTEM_ALTERNATE_SCB_CLOCK_ENABLE_AIO_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_AIO, reg);
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

static void BCHP_PWR_P_HW_AVX_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AVX_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_AVX_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AVX_CH_CTRL_CH_1_POST_DIV_HOLD_CH1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1);
    mask = (BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK |
            BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1_POST_DIVIDER_HOLD_CH1_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1, reg);
}

static void BCHP_PWR_P_HW_AVX_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AVX_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK |
            BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_AVX_CH_CTRL_CH_3_POST_DIV_HOLD_CH3_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AVX_CH_CTRL_CH_3_POST_DIV_HOLD_CH3: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3);
    mask = (BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_HOLD_CH3_MASK |
            BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3, reg);
}

static void BCHP_PWR_P_HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4);
    mask = (BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4_POST_DIVIDER_HOLD_CH4_MASK |
            BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4_CLOCK_DIS_CH4_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4, reg);
}

static void BCHP_PWR_P_HW_BVN_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE_BVNT_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE_BVNT54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE_BVNT_BVB_648_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_BVN_DVPHR0_DVPHT0_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN_DVPHR0_DVPHT0_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE_BVNT_GISB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE_BVNT_BVB_324_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_BVN_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_BVN_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_BVN_SRAM: %s", activate?"on":"off"));

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

static void BCHP_PWR_P_HW_CPU_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CPU_CH_CTRL_CH_2_POST_DIV_HOLD_CH2: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2);
    mask = (BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2_POST_DIVIDER_HOLD_CH2_MASK |
            BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_2, reg);
}

static void BCHP_PWR_P_HW_CPU_CH_CTRL_CH_3_POST_DIV_HOLD_CH3_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CPU_CH_CTRL_CH_3_POST_DIV_HOLD_CH3: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3);
    mask = (BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK |
            BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_HOLD_CH3_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_3, reg);
}

static void BCHP_PWR_P_HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4);
    mask = (BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4_POST_DIVIDER_HOLD_CH4_MASK |
            BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4_CLOCK_DIS_CH4_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_4, reg);
}

static void BCHP_PWR_P_HW_CPU_CH_CTRL_CH_5_POST_DIV_HOLD_CH5_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CPU_CH_CTRL_CH_5_POST_DIV_HOLD_CH5: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5);
    mask = (BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5_CLOCK_DIS_CH5_MASK |
            BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5_POST_DIVIDER_HOLD_CH5_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_CPU_PLL_CHANNEL_CTRL_CH_5, reg);
}

static void BCHP_PWR_P_HW_CTRL_AVX_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_AVX_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_AVX_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_RAAGA_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_RAAGA_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_big_blast_RAAGA_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_SC0_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_SC0_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_SC0_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_SC1_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_SC1_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_SC1_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_V3D_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_V3D_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_V3D_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_VCXO0_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_VCXO0_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_VCXO0_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_CTRL_VCXO1_PWRDN_REQ_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_CTRL_VCXO1_PWRDN_REQ: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL);
    mask = BCHP_CLKGEN_CLKGEN_INST_CTRL_bit_blast_VCXO1_PWRDN_PLL_REQ_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_CLKGEN_INST_CTRL, reg);
    if(activate)
        BKNI_Delay(55);
}

static void BCHP_PWR_P_HW_DVPHR0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHR0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_DISABLE_DISABLE_DVPHR_ALWAYSON_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_DISABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE_DVPHR_HD_DVI_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE_DVPHR_BVB_324_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE_DVPHR_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE_DVPHR_BVB_648_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE0);
    mask = BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE0_DVPHR_108_CLOCK_ENABLE0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE0, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE2);
    mask = BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE2_DVPHR_108_CLOCK_ENABLE2_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE2, reg);
}

static void BCHP_PWR_P_HW_DVPHR0_SECBUS_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_DVPHR0_SECBUS: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE_DVPHR_RBUS_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_HR_INST_CLOCK_ENABLE, reg);
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

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_DISABLE_DISABLE_DVPMHT_IIC_MASTER_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_DISABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE_DVPHT_648_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE_DVPHT_BVB_324_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE_DVPMHT_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE_DVPMHT_54_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_DVP_MHT_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_108_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_SCB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_SECBUS_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_SECBUS: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_GISB_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_SID_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_SID: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_SCB_COM_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_HVD0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_HVD0_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP_SRAM_PDA_IN_HVDP_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDP_SRAM_PDA_OUT_HVDP_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVD0_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS_SRAM_PDA_IN_HVDS_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVDS_SRAM_PDA_OUT_HVDS_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVD0_SRAM Timeout"));
    }
    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD_SRAM_PDA_IN_HVD_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_HVD_SRAM_PDA_OUT_HVD_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_HVD0_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_M2MC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_M2MC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0);
    mask = (BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_SYSTEM_GISB_CLOCK_ENABLE_M2MC0_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_SYSTEM_SCB_CLOCK_ENABLE_M2MC0_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0, reg);
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

static void BCHP_PWR_P_HW_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_SCB_CLOCK_ENABLE_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_RAAGA0_AIO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA0_AIO: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = (BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_54_CLOCK_ENABLE_RAAGA0_MASK |
            BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_108_CLOCK_ENABLE_RAAGA0_MASK |
            BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_SYSTEM_GISB_CLOCK_ENABLE_RAAGA0_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
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

static void BCHP_PWR_P_HW_RAAGA_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_RAAGA_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK |
            BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0, reg);
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

static void BCHP_PWR_P_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK |
            BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_SC1_PLL_CHANNEL_CTRL_CH_0, reg);
}

static void BCHP_PWR_P_HW_SID_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SID: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_GISB_SID_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_SCB_SID_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_SID_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_SID_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_SID);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_SID_SRAM_PDA_IN_SID_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_SID, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_SID);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_SID_SRAM_PDA_OUT_SID_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_SID_SRAM Timeout"));
    }
}

static void BCHP_PWR_P_HW_STB_CORE_XPT_HIF_XPT_CORE_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_CORE_XPT_HIF_XPT_CORE: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE_XPT_CORE_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_HVDP0_HVDP0_CORE_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_HVDP0_HVDP0_CORE: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_CORE_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_HVDP0_HVDP0_CPU_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_HVDP0_HVDP0_CPU: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_CPU_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_HVDP0_HVDP0_SID_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_HVDP0_HVDP0_SID: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE_HVDP0_SID_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_HVDS0_HVDS0_CORE_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_HVDS0_HVDS0_CORE: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_CORE_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_HVDS0_HVDS0_CPU_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_HVDS0_HVDS0_CPU: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE_HVDS0_CPU_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0);
    mask = BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0_DSP_CLOCK_ENABLE_AIO_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_AIO_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_STB_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0);
    mask = BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0_DSP_CLOCK_ENABLE_RAAGA0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_CLOCK_ENABLE_RAAGA0, reg);
}

static void BCHP_PWR_P_HW_STB_SECSECXPT_XPT_SECPIPELINE_SECXPT_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_SECSECXPT_XPT_SECPIPELINE_SECXPT: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT);
    mask = BCHP_CLKGEN_STB_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT_XPT_SECTOP_PIPELINE_CLOCK_ENABLE_SECTOP_XPT_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_SECTOP_INST_CLOCK_ENABLE_SECTOP_XPT, reg);
}

static void BCHP_PWR_P_HW_STB_SYS_CTRL_SC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_SYS_CTRL_SC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE_DISABLE_SC0_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_STB_SYS_CTRL_SC1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_SYS_CTRL_SC1: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE_DISABLE_SC1_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_SYS_CTRL_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_STB_V3D_V3D_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_V3D_V3D: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE);
    mask = BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE_V3D_CLOCK_ENABLE_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_STB_VEC_AIO_GFX_M2MC0_SYSTEM_M2MC0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_VEC_AIO_GFX_M2MC0_SYSTEM_M2MC0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0);
    mask = BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0_SYSTEM_CLOCK_ENABLE_M2MC0_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_M2MC0, reg);
}

static void BCHP_PWR_P_HW_STB_VEC_AIO_GFX_VEC_ITU656_0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_STB_VEC_AIO_GFX_VEC_ITU656_0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE_DISABLE_VEC_ITU656_0_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_V3D_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_V3D: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE_V3D_54_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE_V3D_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE_V3D_GISB_2ND_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE_V3D_GISB_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PLL_V3D_PLL_CHANNEL_CTRL_CH_0);
    mask = (BCHP_CLKGEN_PLL_V3D_PLL_CHANNEL_CTRL_CH_0_POST_DIVIDER_HOLD_CH0_MASK |
            BCHP_CLKGEN_PLL_V3D_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PLL_V3D_PLL_CHANNEL_CTRL_CH_0, reg);
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

static void BCHP_PWR_P_HW_VDAC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VDAC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_ANA_SDAC_TS28HPM_6MX_2MR_FC_H_E_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_ANA_SDAC_TS28HPM_6MX_2MR_FC_H_E_INST_CLOCK_DISABLE_DISABLE_ADC_4P5_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_ANA_SDAC_TS28HPM_6MX_2MR_FC_H_E_INST_CLOCK_DISABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE);
    mask = BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE_DISABLE_QDAC_DACADC_CLOCK_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_DISABLE, reg);
    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC);
    mask = (BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC_SYSTEM_BVB_216_CLOCK_ENABLE_VDAC_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC_QDAC_BVB_216_CLOCK_ENABLE_VDAC_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VDAC, reg);
}

static void BCHP_PWR_P_HW_VEC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC);
    mask = (BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_SYSTEM_SCB_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_SYSTEM_108_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_SYSTEM_BVB_324_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_SYSTEM_BVB_648_CLOCK_ENABLE_VEC_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_SYSTEM_BVB_216_CLOCK_ENABLE_VEC_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC, reg);
}

static void BCHP_PWR_P_HW_VEC_AIO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VEC_AIO: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_AIO);
    mask = BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_AIO_SYSTEM_54_CLOCK_ENABLE_VEC_AIO_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VEC_AIO, reg);
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

static void BCHP_PWR_P_HW_VIP_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VIP: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VIP);
    mask = (BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VIP_SYSTEM_SCB_CLOCK_ENABLE_VIP_MASK |
            BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VIP_SYSTEM_BVB_216_CLOCK_ENABLE_VIP_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_CLOCK_ENABLE_VIP, reg);
}

static void BCHP_PWR_P_HW_VIP_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_VIP_SRAM: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VIP);
    mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VIP_SRAM_PDA_IN_VIP_MASK;
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VIP, reg);
    {
        uint32_t val=0, cnt=50;
        while(cnt--) {
            BKNI_Delay(10);
            reg = BREG_Read32(handle->regHandle, BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VIP);
            mask = BCHP_SUN_TOP_CTRL_SRAM_POWER_GATE_VIP_SRAM_PDA_OUT_VEC_MASK;
            reg &= mask;
            val |= activate?0:mask;
            if (val == reg)
                break;
        }
        if(!cnt)
            BDBG_ERR(("HW_VIP_SRAM Timeout"));
    }
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

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE);
    mask = (BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE_DISABLE_XPT_40P5_CLOCK_MASK |
            BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE_DISABLE_XPT_20P25_CLOCK_MASK |
            BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE_DISABLE_XPT_54_CLOCK_MASK |
            BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE_DISABLE_XPT_81_CLOCK_MASK |
            BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE_DISABLE_XPT_27_CLOCK_MASK);
    reg &= ~mask;
    reg |= activate?0:mask;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_DISABLE, reg);
}

static void BCHP_PWR_P_HW_XPT_WAKEUP_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_WAKEUP: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PM_CLOCK_Async_ALIVE_SEL);
    mask = BCHP_CLKGEN_PM_CLOCK_Async_ALIVE_SEL_CLOCK_Async_CG_XPT_HIF_MASK;
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PM_CLOCK_Async_ALIVE_SEL, reg);

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_PM_PLL_ALIVE_SEL);
    mask = ( BCHP_CLKGEN_PM_PLL_ALIVE_SEL_PLL_SYS0_MASK );
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_PM_PLL_ALIVE_SEL, reg);
}

static void BCHP_PWR_P_HW_XPT_XPT_REMUX_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask, reg;

    BDBG_MSG(("HW_XPT_XPT_REMUX: %s", activate?"on":"off"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE);
    mask = (BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE_XPT_108_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE_XPT_HIF_54_CLOCK_ENABLE_MASK);
    reg &= ~mask;
    reg |= activate?mask:0;
    BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_CORE_XPT_HIF_INST_CLOCK_ENABLE, reg);
}

static void BCHP_PWR_P_MX_HVDP0_SELECT_HVDP0_CORE_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_HVDP0_SELECT_HVDP0_CORE_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_CORE_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_CORE_CLOCK_SELECT);
    }
}

static void BCHP_PWR_P_MX_HVDP0_SELECT_HVDP0_CPU_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_HVDP0_SELECT_HVDP0_CPU_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_CPU_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_CPU_CLOCK_SELECT);
    }
}

static void BCHP_PWR_P_MX_HVDP0_SELECT_HVDP0_SID_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_HVDP0_SELECT_HVDP0_SID_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_SID_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_HVDP0_TOP_INST_CLOCK_SELECT, HVDP0_SID_CLOCK_SELECT);
    }
}

static void BCHP_PWR_P_MX_HVDS0_SELECT_HVDS0_CORE_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_HVDS0_SELECT_HVDS0_CORE_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, HVDS0_CORE_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, HVDS0_CORE_CLOCK_SELECT);
    }
}

static void BCHP_PWR_P_MX_HVDS0_SELECT_HVDS0_CPU_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_HVDS0_SELECT_HVDS0_CPU_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, HVDS0_CPU_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_HVDS0_TOP_INST_CLOCK_SELECT, HVDS0_CPU_CLOCK_SELECT);
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

static void BCHP_PWR_P_MX_RAAGA_DSP_0_RAAGA0_DSP_SELECT_RAAGA0_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_RAAGA_DSP_0_RAAGA0_DSP_SELECT_RAAGA0: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_RAAGA0);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_RAAGA_DSP_TOP_0_INST_RAAGA0, DSP_CLOCK_SELECT_RAAGA0, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_RAAGA_DSP_TOP_0_INST_RAAGA0, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_RAAGA_DSP_TOP_0_INST_RAAGA0, DSP_CLOCK_SELECT_RAAGA0);
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

static void BCHP_PWR_P_MX_V3D_SELECT_V3D_CORE_CLOCK_SELECT_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_V3D_SELECT_V3D_CORE_CLOCK_SELECT: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_SELECT);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_V3D_TOP_INST_CLOCK_SELECT, V3D_CORE_CLOCK_SELECT, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_V3D_TOP_INST_CLOCK_SELECT, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_V3D_TOP_INST_CLOCK_SELECT, V3D_CORE_CLOCK_SELECT);
    }
}

static void BCHP_PWR_P_MX_VEC_AIO_GFX_M2MC0_SELECT_M2MC0_Control(BCHP_Handle handle, unsigned *mux, bool set)
{
    uint32_t reg;

    BDBG_MSG(("MX_VEC_AIO_GFX_M2MC0_SELECT_M2MC0: %s", set?"write":"read"));

    reg = BREG_Read32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_M2MC0);
    if(set) {
        BCHP_SET_FIELD_DATA(reg, CLKGEN_STB_VEC_AIO_GFX_TOP_INST_M2MC0, CLOCK_SELECT_M2MC0, *mux);
        BREG_Write32(handle->regHandle, BCHP_CLKGEN_STB_VEC_AIO_GFX_TOP_INST_M2MC0, reg);
    } else {
        *mux = BCHP_GET_FIELD_DATA(reg, CLKGEN_STB_VEC_AIO_GFX_TOP_INST_M2MC0, CLOCK_SELECT_M2MC0);
    }
}
