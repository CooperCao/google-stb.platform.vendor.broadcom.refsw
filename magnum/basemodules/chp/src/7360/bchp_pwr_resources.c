/***************************************************************************
*     Copyright (c) 2006-2014, Broadcom Corporation*
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



#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#include "bchp_pwr_resources_priv.h"

/* Resource definitions */
const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD,
    BDBG_STRING("AVD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD0,
    BDBG_STRING("AVD0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD0_CLK,
    BDBG_STRING("AVD0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD0_PWR[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD0_PWR,
    BDBG_STRING("AVD0_PWR")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AUD_AIO[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AUD_AIO,
    BDBG_STRING("AUD_AIO")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_RAAGA[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_RAAGA,
    BDBG_STRING("RAAGA")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_RAAGA0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_RAAGA0_CLK,
    BDBG_STRING("RAAGA0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_RAAGA0_DSP[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_RAAGA0_DSP,
    BDBG_STRING("RAAGA0_DSP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_RAAGA0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_RAAGA0_SRAM,
    BDBG_STRING("RAAGA0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC,
    BDBG_STRING("VDC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_BVN[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_BVN,
    BDBG_STRING("BVN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_DAC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_DAC,
    BDBG_STRING("VDC_DAC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_VEC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_VEC,
    BDBG_STRING("VDC_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT,
    BDBG_STRING("XPT")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_PARSER[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_PARSER,
    BDBG_STRING("XPT_PARSER")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_PLAYBACK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_PLAYBACK,
    BDBG_STRING("XPT_PLAYBACK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_RAVE[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_RAVE,
    BDBG_STRING("XPT_RAVE")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_PACKETSUB[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_PACKETSUB,
    BDBG_STRING("XPT_PACKETSUB")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_REMUX,
    BDBG_STRING("XPT_REMUX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_108M[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_108M,
    BDBG_STRING("XPT_108M")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_XMEMIF[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_XMEMIF,
    BDBG_STRING("XPT_XMEMIF")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_SRAM,
    BDBG_STRING("XPT_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_WAKEUP[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_WAKEUP,
    BDBG_STRING("XPT_WAKEUP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_TX[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_TX,
    BDBG_STRING("HDMI_TX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_TX_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_TX_CLK,
    BDBG_STRING("HDMI_TX_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_TX_CEC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_TX_CEC,
    BDBG_STRING("HDMI_TX_CEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC,
    BDBG_STRING("M2MC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC_SRAM,
    BDBG_STRING("M2MC_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HSM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HSM,
    BDBG_STRING("HSM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DMA[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_DMA,
    BDBG_STRING("DMA")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_SMARTCARD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_SMARTCARD,
    BDBG_STRING("SMARTCARD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_SMARTCARD0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_SMARTCARD0,
    BDBG_STRING("SMARTCARD0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_SMARTCARD1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_SMARTCARD1,
    BDBG_STRING("SMARTCARD1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_SOFTMODEM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_SOFTMODEM,
    BDBG_STRING("SOFTMODEM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AUD_PLL0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AUD_PLL0,
    BDBG_STRING("AUD_PLL0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AUD_PLL1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AUD_PLL1,
    BDBG_STRING("AUD_PLL1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_BINT_OPEN[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_BINT_OPEN,
    BDBG_STRING("BINT_OPEN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MAGNUM_CONTROLLED[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED,
    BDBG_STRING("MAGNUM_CONTROLLED")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AVD0_PWR[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_AVD0_PWR,
    BDBG_STRING("HW_AVD0_PWR")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_AIO[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VEC_AIO,
    BDBG_STRING("HW_VEC_AIO")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_RAAGA0_CLK,
    BDBG_STRING("HW_RAAGA0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_RAAGA0_SRAM,
    BDBG_STRING("HW_RAAGA0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN,
    BDBG_STRING("HW_BVN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN_108M[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN_108M,
    BDBG_STRING("HW_BVN_108M")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN_SRAM,
    BDBG_STRING("HW_BVN_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VDC_DAC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VDC_DAC,
    BDBG_STRING("HW_VDC_DAC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VEC_SRAM,
    BDBG_STRING("HW_VEC_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_108M[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_108M,
    BDBG_STRING("HW_XPT_108M")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_XMEMIF[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_XMEMIF,
    BDBG_STRING("HW_XPT_XMEMIF")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_RMX[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_RMX,
    BDBG_STRING("HW_XPT_RMX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_SRAM,
    BDBG_STRING("HW_XPT_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_WAKEUP[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_WAKEUP,
    BDBG_STRING("HW_XPT_WAKEUP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_TX_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HDMI_TX_SRAM,
    BDBG_STRING("HW_HDMI_TX_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_TX_108M[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HDMI_TX_108M,
    BDBG_STRING("HW_HDMI_TX_108M")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_TX_CEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HDMI_TX_CEC,
    BDBG_STRING("HW_HDMI_TX_CEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_GFX_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_GFX_SRAM,
    BDBG_STRING("HW_GFX_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_GFX_108M[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_GFX_108M,
    BDBG_STRING("HW_GFX_108M")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_DMA[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_DMA,
    BDBG_STRING("HW_DMA")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_MDM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_MDM,
    BDBG_STRING("HW_MDM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_AVD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_PLL_AVD_CH1,
    BDBG_STRING("HW_PLL_AVD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_AVD_CH2[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_PLL_AVD_CH2,
    BDBG_STRING("HW_PLL_AVD_CH2")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_AVD_CH3[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_PLL_AVD_CH3,
    BDBG_STRING("HW_PLL_AVD_CH3")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_VCXO[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_PLL_VCXO,
    BDBG_STRING("HW_PLL_VCXO")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AVD0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_AVD0_CLK,
    BDBG_STRING("HW_AVD0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA0_DSP[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_RAAGA0_DSP,
    BDBG_STRING("HW_RAAGA0_DSP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_TX_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HDMI_TX_CLK,
    BDBG_STRING("HW_HDMI_TX_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_M2MC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_M2MC,
    BDBG_STRING("HW_M2MC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SCD0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SCD0,
    BDBG_STRING("HW_SCD0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SCD1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SCD1,
    BDBG_STRING("HW_SCD1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AUD_PLL0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_AUD_PLL0,
    BDBG_STRING("HW_AUD_PLL0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AUD_PLL1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_AUD_PLL1,
    BDBG_STRING("HW_AUD_PLL1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_SCD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_PLL_SCD,
    BDBG_STRING("HW_PLL_SCD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_VCXO_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_PLL_VCXO_CH0,
    BDBG_STRING("HW_PLL_VCXO_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_SCD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_PLL_SCD_CH0,
    BDBG_STRING("HW_PLL_SCD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_SCD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_PLL_SCD_CH1,
    BDBG_STRING("HW_PLL_SCD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_PLL_VCXO_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_PLL_VCXO_CH1,
    BDBG_STRING("HW_PLL_VCXO_CH1")
}};

/* List of resources */
const BCHP_PWR_P_Resource* const BCHP_PWR_P_ResourceList[BCHP_PWR_P_NUM_ALLNODES] = {
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_AVD0,
    BCHP_PWR_P_Resource_AVD0_CLK,
    BCHP_PWR_P_Resource_AVD0_PWR,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_RAAGA0_CLK,
    BCHP_PWR_P_Resource_RAAGA0_DSP,
    BCHP_PWR_P_Resource_RAAGA0_SRAM,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_BVN,
    BCHP_PWR_P_Resource_VDC_DAC,
    BCHP_PWR_P_Resource_VDC_VEC,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_XPT_PARSER,
    BCHP_PWR_P_Resource_XPT_PLAYBACK,
    BCHP_PWR_P_Resource_XPT_RAVE,
    BCHP_PWR_P_Resource_XPT_PACKETSUB,
    BCHP_PWR_P_Resource_XPT_REMUX,
    BCHP_PWR_P_Resource_XPT_108M,
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    BCHP_PWR_P_Resource_XPT_SRAM,
    BCHP_PWR_P_Resource_XPT_WAKEUP,
    BCHP_PWR_P_Resource_HDMI_TX,
    BCHP_PWR_P_Resource_HDMI_TX_CLK,
    BCHP_PWR_P_Resource_HDMI_TX_CEC,
    BCHP_PWR_P_Resource_M2MC,
    BCHP_PWR_P_Resource_M2MC_SRAM,
    BCHP_PWR_P_Resource_HSM,
    BCHP_PWR_P_Resource_DMA,
    BCHP_PWR_P_Resource_SMARTCARD,
    BCHP_PWR_P_Resource_SMARTCARD0,
    BCHP_PWR_P_Resource_SMARTCARD1,
    BCHP_PWR_P_Resource_SOFTMODEM,
    BCHP_PWR_P_Resource_AUD_PLL0,
    BCHP_PWR_P_Resource_AUD_PLL1,
    BCHP_PWR_P_Resource_BINT_OPEN,
    BCHP_PWR_P_Resource_MAGNUM_CONTROLLED,
    BCHP_PWR_P_Resource_HW_AVD0_CLK,
    BCHP_PWR_P_Resource_HW_AVD0_PWR,
    BCHP_PWR_P_Resource_HW_VEC_AIO,
    BCHP_PWR_P_Resource_HW_RAAGA0_CLK,
    BCHP_PWR_P_Resource_HW_RAAGA0_DSP,
    BCHP_PWR_P_Resource_HW_RAAGA0_SRAM,
    BCHP_PWR_P_Resource_HW_HDMI_TX_CLK,
    BCHP_PWR_P_Resource_HW_BVN,
    BCHP_PWR_P_Resource_HW_BVN_108M,
    BCHP_PWR_P_Resource_HW_BVN_SRAM,
    BCHP_PWR_P_Resource_HW_VDC_DAC,
    BCHP_PWR_P_Resource_HW_VEC_SRAM,
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    BCHP_PWR_P_Resource_HW_XPT_RMX,
    BCHP_PWR_P_Resource_HW_XPT_SRAM,
    BCHP_PWR_P_Resource_HW_XPT_WAKEUP,
    BCHP_PWR_P_Resource_HW_HDMI_TX_SRAM,
    BCHP_PWR_P_Resource_HW_HDMI_TX_108M,
    BCHP_PWR_P_Resource_HW_HDMI_TX_CEC,
    BCHP_PWR_P_Resource_HW_M2MC,
    BCHP_PWR_P_Resource_HW_GFX_SRAM,
    BCHP_PWR_P_Resource_HW_GFX_108M,
    BCHP_PWR_P_Resource_HW_DMA,
    BCHP_PWR_P_Resource_HW_SCD0,
    BCHP_PWR_P_Resource_HW_SCD1,
    BCHP_PWR_P_Resource_HW_MDM,
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH1,
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH2,
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH3,
    BCHP_PWR_P_Resource_HW_AUD_PLL0,
    BCHP_PWR_P_Resource_HW_AUD_PLL1,
    BCHP_PWR_P_Resource_HW_PLL_SCD,
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH0,
    BCHP_PWR_P_Resource_HW_PLL_SCD_CH0,
    BCHP_PWR_P_Resource_HW_PLL_SCD_CH1,
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH1,
    BCHP_PWR_P_Resource_HW_PLL_VCXO,
};

/* Coded dependencies */
static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD[] = {
    BCHP_PWR_P_Resource_AVD0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0[] = {
    BCHP_PWR_P_Resource_AVD0_CLK,
    BCHP_PWR_P_Resource_AVD0_PWR,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0_CLK[] = {
    BCHP_PWR_P_Resource_HW_AVD0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0_PWR[] = {
    BCHP_PWR_P_Resource_HW_AVD0_PWR,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_AIO[] = {
    BCHP_PWR_P_Resource_HW_VEC_AIO,
    BCHP_PWR_P_Resource_HW_RAAGA0_CLK,
    BCHP_PWR_P_Resource_HW_RAAGA0_DSP,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA[] = {
    BCHP_PWR_P_Resource_RAAGA0_CLK,
    BCHP_PWR_P_Resource_RAAGA0_DSP,
    BCHP_PWR_P_Resource_RAAGA0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_CLK[] = {
    BCHP_PWR_P_Resource_HW_RAAGA0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_DSP[] = {
    BCHP_PWR_P_Resource_HW_RAAGA0_DSP,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_SRAM[] = {
    BCHP_PWR_P_Resource_HW_RAAGA0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC[] = {
    BCHP_PWR_P_Resource_BVN,
    BCHP_PWR_P_Resource_VDC_DAC,
    BCHP_PWR_P_Resource_VDC_VEC,
    BCHP_PWR_P_Resource_HW_HDMI_TX_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_BVN[] = {
    BCHP_PWR_P_Resource_HW_BVN,
    BCHP_PWR_P_Resource_HW_BVN_108M,
    BCHP_PWR_P_Resource_HW_BVN_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_DAC[] = {
    BCHP_PWR_P_Resource_HW_VDC_DAC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_VEC[] = {
    BCHP_PWR_P_Resource_HW_VEC_AIO,
    BCHP_PWR_P_Resource_HW_VEC_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    BCHP_PWR_P_Resource_HW_XPT_RMX,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PARSER[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PLAYBACK[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_RAVE[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PACKETSUB[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_REMUX[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    BCHP_PWR_P_Resource_HW_XPT_RMX,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_108M[] = {
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_XMEMIF[] = {
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_SRAM[] = {
    BCHP_PWR_P_Resource_HW_XPT_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_WAKEUP[] = {
    BCHP_PWR_P_Resource_HW_XPT_WAKEUP,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_TX[] = {
    BCHP_PWR_P_Resource_HDMI_TX_CLK,
    BCHP_PWR_P_Resource_HDMI_TX_CEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_TX_CLK[] = {
    BCHP_PWR_P_Resource_HW_HDMI_TX_CLK,
    BCHP_PWR_P_Resource_HW_HDMI_TX_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_TX_CEC[] = {
    BCHP_PWR_P_Resource_HW_HDMI_TX_CEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC[] = {
    BCHP_PWR_P_Resource_HW_M2MC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC_SRAM[] = {
    BCHP_PWR_P_Resource_HW_GFX_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HSM[] = {
    BCHP_PWR_P_Resource_DMA,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_DMA[] = {
    BCHP_PWR_P_Resource_HW_DMA,
    BCHP_PWR_P_Resource_HW_XPT_108M,
    BCHP_PWR_P_Resource_HW_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SMARTCARD[] = {
    BCHP_PWR_P_Resource_SMARTCARD0,
    BCHP_PWR_P_Resource_SMARTCARD1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SMARTCARD0[] = {
    BCHP_PWR_P_Resource_HW_SCD0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SMARTCARD1[] = {
    BCHP_PWR_P_Resource_HW_SCD1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SOFTMODEM[] = {
    BCHP_PWR_P_Resource_HW_MDM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_PLL0[] = {
    BCHP_PWR_P_Resource_HW_AUD_PLL0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_PLL1[] = {
    BCHP_PWR_P_Resource_HW_AUD_PLL1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_BINT_OPEN[] = {
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_HDMI_TX,
    BCHP_PWR_P_Resource_M2MC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MAGNUM_CONTROLLED[] = {
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_HDMI_TX,
    BCHP_PWR_P_Resource_SMARTCARD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_AVD0_CLK[] = {
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH1,
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH2,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_RAAGA0_DSP[] = {
    BCHP_PWR_P_Resource_HW_PLL_AVD_CH3,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HDMI_TX_CLK[] = {
    BCHP_PWR_P_Resource_HW_HDMI_TX_108M,
    BCHP_PWR_P_Resource_HW_BVN_108M,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_M2MC[] = {
    BCHP_PWR_P_Resource_HW_GFX_108M,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SCD0[] = {
    BCHP_PWR_P_Resource_HW_PLL_SCD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SCD1[] = {
    BCHP_PWR_P_Resource_HW_PLL_SCD_CH1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_AUD_PLL0[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_AUD_PLL1[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_PLL_SCD[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_PLL_VCXO_CH0[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_PLL_SCD_CH0[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH1,
    BCHP_PWR_P_Resource_HW_PLL_SCD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_PLL_SCD_CH1[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO_CH1,
    BCHP_PWR_P_Resource_HW_PLL_SCD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_PLL_VCXO_CH1[] = {
    BCHP_PWR_P_Resource_HW_PLL_VCXO,
    NULL
};

/* List of coded dependencies */
const BCHP_PWR_P_Resource* const * const BCHP_PWR_P_DependList[BCHP_PWR_P_NUM_ALLNODES] = {
    BCHP_PWR_P_Depend_AVD,
    BCHP_PWR_P_Depend_AVD0,
    BCHP_PWR_P_Depend_AVD0_CLK,
    BCHP_PWR_P_Depend_AVD0_PWR,
    BCHP_PWR_P_Depend_AUD_AIO,
    BCHP_PWR_P_Depend_RAAGA,
    BCHP_PWR_P_Depend_RAAGA0_CLK,
    BCHP_PWR_P_Depend_RAAGA0_DSP,
    BCHP_PWR_P_Depend_RAAGA0_SRAM,
    BCHP_PWR_P_Depend_VDC,
    BCHP_PWR_P_Depend_BVN,
    BCHP_PWR_P_Depend_VDC_DAC,
    BCHP_PWR_P_Depend_VDC_VEC,
    BCHP_PWR_P_Depend_XPT,
    BCHP_PWR_P_Depend_XPT_PARSER,
    BCHP_PWR_P_Depend_XPT_PLAYBACK,
    BCHP_PWR_P_Depend_XPT_RAVE,
    BCHP_PWR_P_Depend_XPT_PACKETSUB,
    BCHP_PWR_P_Depend_XPT_REMUX,
    BCHP_PWR_P_Depend_XPT_108M,
    BCHP_PWR_P_Depend_XPT_XMEMIF,
    BCHP_PWR_P_Depend_XPT_SRAM,
    BCHP_PWR_P_Depend_XPT_WAKEUP,
    BCHP_PWR_P_Depend_HDMI_TX,
    BCHP_PWR_P_Depend_HDMI_TX_CLK,
    BCHP_PWR_P_Depend_HDMI_TX_CEC,
    BCHP_PWR_P_Depend_M2MC,
    BCHP_PWR_P_Depend_M2MC_SRAM,
    BCHP_PWR_P_Depend_HSM,
    BCHP_PWR_P_Depend_DMA,
    BCHP_PWR_P_Depend_SMARTCARD,
    BCHP_PWR_P_Depend_SMARTCARD0,
    BCHP_PWR_P_Depend_SMARTCARD1,
    BCHP_PWR_P_Depend_SOFTMODEM,
    BCHP_PWR_P_Depend_AUD_PLL0,
    BCHP_PWR_P_Depend_AUD_PLL1,
    BCHP_PWR_P_Depend_BINT_OPEN,
    BCHP_PWR_P_Depend_MAGNUM_CONTROLLED,
    BCHP_PWR_P_Depend_HW_AVD0_CLK,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_RAAGA0_DSP,
    NULL,
    BCHP_PWR_P_Depend_HW_HDMI_TX_CLK,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_M2MC,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_SCD0,
    BCHP_PWR_P_Depend_HW_SCD1,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_AUD_PLL0,
    BCHP_PWR_P_Depend_HW_AUD_PLL1,
    BCHP_PWR_P_Depend_HW_PLL_SCD,
    BCHP_PWR_P_Depend_HW_PLL_VCXO_CH0,
    BCHP_PWR_P_Depend_HW_PLL_SCD_CH0,
    BCHP_PWR_P_Depend_HW_PLL_SCD_CH1,
    BCHP_PWR_P_Depend_HW_PLL_VCXO_CH1,
    NULL,
};

#include "bchp_pwr_impl.c"

void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate)
{
    switch(id) {
        case BCHP_PWR_HW_AVD0_CLK:
            BCHP_PWR_P_HW_AVD0_CLK_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AVD0_PWR:
            BCHP_PWR_P_HW_AVD0_PWR_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_AIO:
            BCHP_PWR_P_HW_VEC_AIO_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA0_CLK:
            BCHP_PWR_P_HW_RAAGA0_CLK_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA0_DSP:
            BCHP_PWR_P_HW_RAAGA0_DSP_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA0_SRAM:
            BCHP_PWR_P_HW_RAAGA0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_TX_CLK:
            BCHP_PWR_P_HW_HDMI_TX_CLK_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN:
            BCHP_PWR_P_HW_BVN_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN_108M:
            BCHP_PWR_P_HW_BVN_108M_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN_SRAM:
            BCHP_PWR_P_HW_BVN_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VDC_DAC:
            BCHP_PWR_P_HW_VDC_DAC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_SRAM:
            BCHP_PWR_P_HW_VEC_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_108M:
            BCHP_PWR_P_HW_XPT_108M_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_XMEMIF:
            BCHP_PWR_P_HW_XPT_XMEMIF_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_RMX:
            BCHP_PWR_P_HW_XPT_RMX_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_SRAM:
            BCHP_PWR_P_HW_XPT_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_WAKEUP:
            BCHP_PWR_P_HW_XPT_WAKEUP_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_TX_SRAM:
            BCHP_PWR_P_HW_HDMI_TX_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_TX_108M:
            BCHP_PWR_P_HW_HDMI_TX_108M_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_TX_CEC:
            BCHP_PWR_P_HW_HDMI_TX_CEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_M2MC:
            BCHP_PWR_P_HW_M2MC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_GFX_SRAM:
            BCHP_PWR_P_HW_GFX_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_GFX_108M:
            BCHP_PWR_P_HW_GFX_108M_Control(handle, activate);
            break;
        case BCHP_PWR_HW_DMA:
            BCHP_PWR_P_HW_DMA_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SCD0:
            BCHP_PWR_P_HW_SCD0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SCD1:
            BCHP_PWR_P_HW_SCD1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_MDM:
            BCHP_PWR_P_HW_MDM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_AVD_CH1:
            BCHP_PWR_P_HW_PLL_AVD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_AVD_CH2:
            BCHP_PWR_P_HW_PLL_AVD_CH2_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_AVD_CH3:
            BCHP_PWR_P_HW_PLL_AVD_CH3_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AUD_PLL0:
            BCHP_PWR_P_HW_AUD_PLL0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AUD_PLL1:
            BCHP_PWR_P_HW_AUD_PLL1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_SCD:
            BCHP_PWR_P_HW_PLL_SCD_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_VCXO_CH0:
            BCHP_PWR_P_HW_PLL_VCXO_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_SCD_CH0:
            BCHP_PWR_P_HW_PLL_SCD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_SCD_CH1:
            BCHP_PWR_P_HW_PLL_SCD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_VCXO_CH1:
            BCHP_PWR_P_HW_PLL_VCXO_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_PLL_VCXO:
            BCHP_PWR_P_HW_PLL_VCXO_Control(handle, activate);
            break;
        default:
            BDBG_ASSERT(0);
            break;
    }
}

void BCHP_PWR_P_HW_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, bool activate)
{
    BCHP_PWR_P_HW_ControlId(handle, resource->id, activate);
}
