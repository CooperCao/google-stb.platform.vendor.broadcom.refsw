/******************************************************************************
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
******************************************************************************/

#include "bchp.h"
#include "bchp_priv.h"
#include "bdbg.h"
#include "bkni.h"

#include "bchp_clkgen.h"
#include "bchp_hdmi_tx_phy.h"
#include "bchp_aio_misc.h"


BDBG_MODULE(BCHP_PWR_IMPL);

static void BCHP_PWR_P_HW_AVD0_CLK_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;
    BDBG_MSG(("HW_AVD0_CLK: %s", activate?"on":"off"));

	if (activate) {
		/* AVD Core, CPU, SCB, 108M clock */
		mask = (BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_AVD_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_CPU_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_SCB_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_108_CLOCK_ENABLE_MASK);
		BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE, mask, mask);
	}
	else {
		/* AVD Core, CPU, SCB, 108M clock */
		mask = (BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_AVD_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_CPU_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_SCB_CLOCK_ENABLE_MASK |
		BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE_SVD_108_CLOCK_ENABLE_MASK);
		BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_AVD0_TOP_CLOCK_ENABLE, mask, 0);
	}
}

static void BCHP_PWR_P_HW_AVD0_PWR_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;
    
    BSTD_UNUSED(handle);
    BSTD_UNUSED(mask);

    BDBG_MSG(("HW_AVD0_PWR: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_AVD0_TOP_POWER_SWITCH_MEMORY_SVD_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_AVD0_TOP_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_AVD0_TOP_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_AVD0_TOP_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_VEC_AIO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_VEC_AIO: %s", activate?"on":"off"));

    mask = (BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_ALTERNATE_SCB_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_ALTERNATE_216_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_ALTERNATE_108_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_ALTERNATE2_108_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_SCB_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_216_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE_VEC_108_CLOCK_ENABLE_MASK);

    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_ENABLE, mask, activate?mask:0);
	}

static void BCHP_PWR_P_HW_RAAGA0_CLK_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_RAAGA0_CLK: %s", activate?"on":"off"));

    mask = (BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE_RAAGA_SCB_CLOCK_ENABLE_MASK |
            BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE_RAAGA_108_CLOCK_ENABLE_MASK);

    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_RAAGA0_DSP_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_RAAGA0_DSP: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE_RAAGA_DSP_CLOCK_ENABLE_MASK;

    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_RAAGA0_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_RAAGA0_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_RAAGA_DSP_TOP_POWER_SWITCH_MEMORY_RAAGA_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_RAAGA_DSP_TOP_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_HDMI_TX_CLK_Control(BCHP_Handle handle, bool activate)
{
		uint32_t mask, val;

		BDBG_MSG(("HW_HDMI_TX_CLK: %s", activate?"on":"off"));

#if 1
		mask = BCHP_CLKGEN_DVP_HT_ENABLE_DVPHT_CLK_MAX_ENABLE_MASK;
		BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_ENABLE, mask, activate?mask:0);
#endif
#if 0
		mask =	BCHP_CLKGEN_DVP_HT_CLOCK_ENABLE_DVPHT_ALTERNATE_216_CLOCK_ENABLE_MASK;
		BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_CLOCK_ENABLE, mask, activate?mask:0);
#endif
		val = BREG_Read32(handle->regHandle, BCHP_HDMI_TX_PHY_RESET_CTL);
		mask = (BCHP_HDMI_TX_PHY_RESET_CTL_PLL_RESETB_MASK |
				BCHP_HDMI_TX_PHY_RESET_CTL_PLLDIV_RSTB_MASK );
		if (activate) {
			val |= mask;
		}
		else {
			val &= ~mask;
		}
		BREG_Write32(handle->regHandle, BCHP_HDMI_TX_PHY_RESET_CTL, val);
}

static void BCHP_PWR_P_HW_BVN_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_BVN: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_BVN_TOP_ENABLE_BVN_SCB_CLOCK_ENABLE_MASK;
    
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_BVN_TOP_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_BVN_108M_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_BVN_108M: %s", activate?"on":"off"));

     mask = (BCHP_CLKGEN_BVN_TOP_ENABLE_BVN_216_CLK_ENABLE_MASK |
	     BCHP_CLKGEN_BVN_TOP_ENABLE_BVN_108_CLK_ENABLE_MASK);
    
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_BVN_TOP_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_BVN_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_BVN_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_BVN_TOP_POWER_SWITCH_MEMORY_BVN_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_BVN_TOP_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_BVN_TOP_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_BVN_TOP_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_VDC_DAC_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_VDC_DAC: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_DISABLE_DISABLE_VEC_DACADC_CLOCK_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_TOP_CLOCK_DISABLE, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_VEC_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_VEC_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_VEC_AIO_TOP_POWER_SWITCH_MEMORY_A_VEC_POWER_SWITCH_MEMORY_A_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_TOP_POWER_SWITCH_MEMORY_A, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_TOP_POWER_SWITCH_MEMORY_A, mask, 0);
	BKNI_Delay(10);
	} else {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_VEC_AIO_TOP_POWER_SWITCH_MEMORY_A, mask, mask);
    }
}

static void BCHP_PWR_P_HW_XPT_108M_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_XPT_108M: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_CORE_XPT_CLOCK_ENABLE_XPT_108_CLOCK_ENABLE_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_CLOCK_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_XPT_XMEMIF_Control(BCHP_Handle handle, bool activate)
{
	uint32_t mask;
	 BDBG_MSG(("HW_XPT_XMEMIF: %s", activate?"on":"off"));

	 mask = (BCHP_CLKGEN_CORE_XPT_CLOCK_ENABLE_XPT_216_CLOCK_ENABLE_MASK |
		 BCHP_CLKGEN_CORE_XPT_CLOCK_ENABLE_XPT_SCB_CLOCK_ENABLE_MASK);
	 BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_CLOCK_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_XPT_RMX_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;
    BDBG_MSG(("HW_XPT_RMX: %s", activate?"on":"off"));

    mask = (BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE_DISABLE_XPT_20P25_CLOCK_MASK |
	    BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE_DISABLE_XPT_27_CLOCK_MASK |
	    BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE_DISABLE_XPT_40P5_CLOCK_MASK |
	    BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE_DISABLE_XPT_54_CLOCK_MASK |
	    BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE_DISABLE_XPT_81_CLOCK_MASK);
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_CLOCK_DISABLE, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_XPT_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_XPT_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_CORE_XPT_POWER_SWITCH_MEMORY_XPT_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_CORE_XPT_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_XPT_WAKEUP_Control(BCHP_Handle handle, bool activate)
{
    BSTD_UNUSED(handle);
    BDBG_MSG(("HW_XPT_WAKEUP: %s", activate?"on":"off"));

#if 0 /* Edit the register read/modify/write below */
    BREG_AtomicUpdate32(handle->regHandle, BCHP_REGISTERNAME,
        BCHP_REGISTERNAME_XPT_WAKEUP_MASK,
        activate ? 0 : 0xFFFFFFFFFF);
#endif
    BSTD_UNUSED(activate);
}

static void BCHP_PWR_P_HW_HDMI_TX_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_HDMI_TX_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_DVP_HT_POWER_SWITCH_MEMORY_DVPHT_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_HDMI_TX_108M_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_HDMI_TX_108M: %s", activate?"on":"off"));

    mask = (BCHP_CLKGEN_DVP_HT_CLOCK_ENABLE_DVPHT_216_CLOCK_ENABLE_MASK |
        BCHP_CLKGEN_DVP_HT_CLOCK_ENABLE_DVPHT_108_CLOCK_ENABLE_MASK );
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_DVP_HT_CLOCK_ENABLE, mask, activate?mask:0);
}

static void BCHP_PWR_P_HW_HDMI_TX_CEC_Control(BCHP_Handle handle, bool activate)
{
    BSTD_UNUSED(handle);
    BDBG_MSG(("HW_HDMI_TX_CEC: %s", activate?"on":"off"));

#if 0 /* Edit the register read/modify/write below */
    BREG_AtomicUpdate32(handle->regHandle, BCHP_REGISTERNAME,
        BCHP_REGISTERNAME_HDMI_TX_CEC_MASK,
        activate ? 0 : 0xFFFFFFFFFF);
#endif
    BSTD_UNUSED(activate);
}

static void BCHP_PWR_P_HW_M2MC_Control(BCHP_Handle handle, bool activate)
{
	uint32_t mask;

	 BDBG_MSG(("HW_M2MC: %s", activate?"on":"off"));

	 mask = BCHP_CLKGEN_GRAPHICS_CLOCK_ENABLE_GFX_M2MC_CLOCK_ENABLE_MASK;
	 BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_CLOCK_ENABLE, mask, activate ? mask : 0);

	 mask = BCHP_CLKGEN_GRAPHICS_CLOCK_DISABLE_DISABLE_GFX_M2MC_CORE_CLOCK_MASK;
	 BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_CLOCK_DISABLE, mask, activate ? 0: mask);
}

static void BCHP_PWR_P_HW_GFX_SRAM_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;
	
    BDBG_MSG(("HW_GFX_SRAM: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_GRAPHICS_POWER_SWITCH_MEMORY_GFX_POWER_SWITCH_MEMORY_MASK;

    if(activate) {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_POWER_SWITCH_MEMORY, mask, 2);
	BKNI_Delay(10);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_POWER_SWITCH_MEMORY, mask, 0);
	BKNI_Delay(10);
    } else {
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_POWER_SWITCH_MEMORY, mask, mask);
    }
}

static void BCHP_PWR_P_HW_GFX_108M_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;
		
    BDBG_MSG(("HW_GFX_108M: %s", activate?"on":"off"));

    mask = (BCHP_CLKGEN_GRAPHICS_CLOCK_ENABLE_GFX_108_CLOCK_ENABLE_MASK |
	    BCHP_CLKGEN_GRAPHICS_CLOCK_ENABLE_GFX_SCB_CLOCK_ENABLE_MASK);
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_GRAPHICS_CLOCK_ENABLE, mask, activate ? mask : 0);
}

static void BCHP_PWR_P_HW_DMA_Control(BCHP_Handle handle, bool activate)
{
		uint32_t mask;

		BDBG_MSG(("HW_DMA: %s", activate?"on":"off"));
#if 0   
		mask = BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE_SEC_ALTERNATE_SCB_CLOCK_ENABLE_MASK;

		BREG_AtomicUpdate32(handle->regHandle,	BCHP_CLKGEN_SECTOP_INST_CLOCK_ENABLE, mask, activate?mask:0);
#else
		BSTD_UNUSED(handle);
		BSTD_UNUSED(activate);
		BSTD_UNUSED(mask);
#endif	
}

static void BCHP_PWR_P_HW_SCD0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_SCD0: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE_DISABLE_SC0_CLOCK_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_SCD1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_SCD1: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE_DISABLE_SC1_CLOCK_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_MDM_Control(BCHP_Handle handle, bool activate)
{
    BSTD_UNUSED(handle);
	BDBG_MSG(("HW_MDM: %s", activate?"on":"off"));

#if 0 /* Edit the register read/modify/write below */
    BREG_AtomicUpdate32(handle->regHandle, BCHP_REGISTERNAME,
        BCHP_REGISTERNAME_MDM_MASK,
        activate ? 0 : 0xFFFFFFFFFF);
#endif
    BSTD_UNUSED(activate);
}

static void BCHP_PWR_P_HW_PLL_AVD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_AVD_CH1: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_1, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_AVD_CH2_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_AVD_CH2: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_2_CLOCK_DIS_CH2_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_2, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_AVD_CH3_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_AVD_CH3: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_3, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_AUD_PLL0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_AUD_PLL0: %s", activate?"on":"off"));

    if(activate) {
	mask = (BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET, mask, 0);

	mask = BCHP_CLKGEN_PLL_AUDIO0_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_PLL_PWRDN, mask, 0);
    } else {
	mask = BCHP_CLKGEN_PLL_AUDIO0_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_PLL_PWRDN, mask, mask);

	mask = (BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO0_PLL_RESET, mask, mask);
    }
}

static void BCHP_PWR_P_HW_AUD_PLL1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_AUD_PLL1: %s", activate?"on":"off"));

    if(activate) {
	mask = (BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET, mask, 0);

	mask = BCHP_CLKGEN_PLL_AUDIO1_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_PLL_PWRDN, mask, 0);
    } else {
	mask = BCHP_CLKGEN_PLL_AUDIO1_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_PLL_PWRDN, mask, mask);

	mask = (BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_AUDIO1_PLL_RESET, mask, mask);
    }
}

static void BCHP_PWR_P_HW_PLL_SCD_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_SCD_PLL: %s", activate?"on":"off"));
    
    if(activate) {
	mask = (BCHP_CLKGEN_PLL_SC_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_SC_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_RESET, mask, 0);

	mask = BCHP_CLKGEN_PLL_SC_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_PWRDN, mask, 0);	
    } else {
	mask = BCHP_CLKGEN_PLL_SC_PLL_PWRDN_PWRDN_PLL_MASK;
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_PWRDN, mask, mask);

	mask = (BCHP_CLKGEN_PLL_SC_PLL_RESET_RESETD_MASK |
		BCHP_CLKGEN_PLL_SC_PLL_RESET_RESETA_MASK);
	BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_RESET, mask, mask);
    }
}

static void BCHP_PWR_P_HW_PLL_VCXO_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_VCXO_CH0: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_0, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_SCD_CH0_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_SCD_CH0: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_SC_PLL_CHANNEL_CTRL_CH_0_CLOCK_DIS_CH0_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_CHANNEL_CTRL_CH_0, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_SCD_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_SCD_CH1: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_SC_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_SC_PLL_CHANNEL_CTRL_CH_1, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_VCXO_CH1_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_VCXO_CH1: %s", activate?"on":"off"));

    mask = BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK;
    BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_1, mask, activate?0:mask);
}

static void BCHP_PWR_P_HW_PLL_VCXO_Control(BCHP_Handle handle, bool activate)
{
    uint32_t mask;

    BDBG_MSG(("HW_PLL_VCXO: %s", activate?"on":"off"));

    if(activate) {
        mask = (BCHP_CLKGEN_PLL_VCXO_PLL_RESET_RESETD_MASK |
                BCHP_CLKGEN_PLL_VCXO_PLL_RESET_RESETA_MASK);
        BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_RESET, mask, 0);

        mask = BCHP_CLKGEN_PLL_VCXO_PLL_PWRDN_PWRDN_PLL_MASK;
        BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_PWRDN, mask, 0);
    } else {
        mask = BCHP_CLKGEN_PLL_VCXO_PLL_PWRDN_PWRDN_PLL_MASK;
        BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_PWRDN, mask, mask);

        mask = (BCHP_CLKGEN_PLL_VCXO_PLL_RESET_RESETD_MASK |
                BCHP_CLKGEN_PLL_VCXO_PLL_RESET_RESETA_MASK);
        BREG_AtomicUpdate32(handle->regHandle, BCHP_CLKGEN_PLL_VCXO_PLL_RESET, mask, mask);
    }
}
