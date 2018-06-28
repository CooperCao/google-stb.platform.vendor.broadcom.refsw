 /******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/***************************************************************************
*
* This file is auto-generated
*
* This file contains a coded representation of power resource
* nodes and their dependencies.
*
***************************************************************************/

#include "bchp_pwr.h"
#include "bchp_pwr_resources.h"
#include "bchp_pwr_resources_priv.h"

/* Resource definitions */
const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AIO_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AIO_CLK,
    BDBG_STRING("AIO_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AIO_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AIO_SRAM,
    BDBG_STRING("AIO_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_ASP[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_ASP,
    BDBG_STRING("ASP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_ASP_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_ASP_SRAM,
    BDBG_STRING("ASP_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AUD_AIO[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AUD_AIO,
    BDBG_STRING("AUD_AIO")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD1,
    BDBG_STRING("AVD1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD1_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD1_CLK,
    BDBG_STRING("AVD1_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_AVD1_PWR[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_AVD1_PWR,
    BDBG_STRING("AVD1_PWR")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_BINT_OPEN[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_BINT_OPEN,
    BDBG_STRING("BINT_OPEN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_BVN[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_BVN,
    BDBG_STRING("BVN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DMA[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_DMA,
    BDBG_STRING("DMA")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_GRAPHICS3D[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_GRAPHICS3D,
    BDBG_STRING("GRAPHICS3D")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_GRAPHICS3D_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_GRAPHICS3D_SRAM,
    BDBG_STRING("GRAPHICS3D_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_RX0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_RX0_CLK,
    BDBG_STRING("HDMI_RX0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_RX0_PHY[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_RX0_PHY,
    BDBG_STRING("HDMI_RX0_PHY")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_RX0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_RX0_SRAM,
    BDBG_STRING("HDMI_RX0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_TX0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_TX0_CLK,
    BDBG_STRING("HDMI_TX0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HDMI_TX0_PHY[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HDMI_TX0_PHY,
    BDBG_STRING("HDMI_TX0_PHY")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HSM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_HSM,
    BDBG_STRING("HSM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC,
    BDBG_STRING("M2MC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC0,
    BDBG_STRING("M2MC0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC0_SRAM,
    BDBG_STRING("M2MC0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC1,
    BDBG_STRING("M2MC1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_M2MC_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_M2MC_SRAM,
    BDBG_STRING("M2MC_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MAGNUM_CONTROLLED[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED,
    BDBG_STRING("MAGNUM_CONTROLLED")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MMM2MC0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_MMM2MC0_SRAM,
    BDBG_STRING("MMM2MC0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MM_M2MC0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_MM_M2MC0,
    BDBG_STRING("MM_M2MC0")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_SECURE_ACCESS[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_SECURE_ACCESS,
    BDBG_STRING("SECURE_ACCESS")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC,
    BDBG_STRING("VDC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_656_OUT[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_656_OUT,
    BDBG_STRING("VDC_656_OUT")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_DAC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_DAC,
    BDBG_STRING("VDC_DAC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_HDMI_RX_CLK0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0,
    BDBG_STRING("VDC_HDMI_RX_CLK0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_HDMI_TX_PHY0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0,
    BDBG_STRING("VDC_HDMI_TX_PHY0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_STG0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_STG0,
    BDBG_STRING("VDC_STG0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_STG1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_STG1,
    BDBG_STRING("VDC_STG1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VDC_VEC[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VDC_VEC,
    BDBG_STRING("VDC_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VICE[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VICE,
    BDBG_STRING("VICE")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VICE0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VICE0,
    BDBG_STRING("VICE0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_VICE0_CLK[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_VICE0_CLK,
    BDBG_STRING("VICE0_CLK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT,
    BDBG_STRING("XPT")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_CORE[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_CORE,
    BDBG_STRING("XPT_CORE")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_PACKETSUB[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_PACKETSUB,
    BDBG_STRING("XPT_PACKETSUB")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_REMUX,
    BDBG_STRING("XPT_REMUX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_SRAM,
    BDBG_STRING("XPT_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_TSIO[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_TSIO,
    BDBG_STRING("XPT_TSIO")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_WAKEUP[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_WAKEUP,
    BDBG_STRING("XPT_WAKEUP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_XPT_XMEMIF[] = {{
    BCHP_PWR_P_ResourceType_eNonLeaf,
    BCHP_PWR_RESOURCE_XPT_XMEMIF,
    BDBG_STRING("XPT_XMEMIF")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AIO[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_AIO,
    BDBG_STRING("HW_AIO")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AIO_RAAGA0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_AIO_RAAGA0,
    BDBG_STRING("HW_AIO_RAAGA0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AIO_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_AIO_SRAM,
    BDBG_STRING("HW_AIO_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_AIO_VEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_AIO_VEC,
    BDBG_STRING("HW_AIO_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ASP[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_ASP,
    BDBG_STRING("HW_ASP")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BDBG_STRING("HW_ASP_BVN_DVPHR0_DVPHT0_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ASP_HVD0_HVD1[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_ASP_HVD0_HVD1,
    BDBG_STRING("HW_ASP_HVD0_HVD1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ASP_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_ASP_SRAM,
    BDBG_STRING("HW_ASP_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN,
    BDBG_STRING("HW_BVN")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN_DVPHR0_DVPHT0_VEC,
    BDBG_STRING("HW_BVN_DVPHR0_DVPHT0_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_BVN_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_BVN_SRAM,
    BDBG_STRING("HW_BVN_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_DVPHR0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_DVPHR0,
    BDBG_STRING("HW_DVPHR0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_DVPHR0_SECBUS[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_DVPHR0_SECBUS,
    BDBG_STRING("HW_DVPHR0_SECBUS")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_DVPHR0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_DVPHR0_SRAM,
    BDBG_STRING("HW_DVPHR0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_DVPHT0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_DVPHT0,
    BDBG_STRING("HW_DVPHT0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD0,
    BDBG_STRING("HW_HVD0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD0_SECBUS[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD0_SECBUS,
    BDBG_STRING("HW_HVD0_SECBUS")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD0_SRAM,
    BDBG_STRING("HW_HVD0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD1[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD1,
    BDBG_STRING("HW_HVD1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD1_SECBUS[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD1_SECBUS,
    BDBG_STRING("HW_HVD1_SECBUS")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD1_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVD1_SRAM,
    BDBG_STRING("HW_HVD1_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVDP0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVDP0_SRAM,
    BDBG_STRING("HW_HVDP0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVDP1_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVDP1_SRAM,
    BDBG_STRING("HW_HVDP1_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVDS0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVDS0_SRAM,
    BDBG_STRING("HW_HVDS0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVDS1_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_HVDS1_SRAM,
    BDBG_STRING("HW_HVDS1_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ITU656_VEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_ITU656_VEC,
    BDBG_STRING("HW_ITU656_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_M2MC0_M2MC1[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_M2MC0_M2MC1,
    BDBG_STRING("HW_M2MC0_M2MC1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_M2MC0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_M2MC0_SRAM,
    BDBG_STRING("HW_M2MC0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_MMM2MC0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_MMM2MC0_SRAM,
    BDBG_STRING("HW_MMM2MC0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_MM_M2MC0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_MM_M2MC0,
    BDBG_STRING("HW_MM_M2MC0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA0[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_RAAGA0,
    BDBG_STRING("HW_RAAGA0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA0_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_RAAGA0_SRAM,
    BDBG_STRING("HW_RAAGA0_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC0_LDO_PWRON_LDO_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_SC0_LDO_PWRON_LDO_PWRON_PLL,
    BDBG_STRING("HW_SC0_LDO_PWRON_LDO_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC1_LDO_PWRON_LDO_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_SC1_LDO_PWRON_LDO_PWRON_PLL,
    BDBG_STRING("HW_SC1_LDO_PWRON_LDO_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SECBUS_VICE20[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_SECBUS_VICE20,
    BDBG_STRING("HW_SECBUS_VICE20")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SECBUS_XPT_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_SECBUS_XPT_XPT_REMUX,
    BDBG_STRING("HW_SECBUS_XPT_XPT_REMUX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_V3D[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_V3D,
    BDBG_STRING("HW_V3D")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_V3D_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_V3D_SRAM,
    BDBG_STRING("HW_V3D_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL,
    BDBG_STRING("HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL,
    BDBG_STRING("HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VDAC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VDAC,
    BDBG_STRING("HW_VDAC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VEC,
    BDBG_STRING("HW_VEC")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_SRAM[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VEC_SRAM,
    BDBG_STRING("HW_VEC_SRAM")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VICE20[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_VICE20,
    BDBG_STRING("HW_VICE20")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_REMUX,
    BDBG_STRING("HW_XPT_REMUX")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_XPT_CORE_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_XPT_CORE_XPT_REMUX,
    BDBG_STRING("HW_XPT_XPT_CORE_XPT_REMUX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_XPT_REMUX[] = {{
    BCHP_PWR_P_ResourceType_eLeaf,
    BCHP_PWR_HW_XPT_XPT_REMUX,
    BDBG_STRING("HW_XPT_XPT_REMUX")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_ASP_ASPT_ASP_540[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_ASP_ASPT_ASP_540,
    BDBG_STRING("HW_ASP_ASPT_ASP_540")
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

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_RX0_PHY[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HDMI_RX0_PHY,
    BDBG_STRING("HW_HDMI_RX0_PHY")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HDMI_TX0_PHY[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HDMI_TX0_PHY,
    BDBG_STRING("HW_HDMI_TX0_PHY")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD0_HVD0_CORE[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD0_HVD0_CORE,
    BDBG_STRING("HW_HVD0_HVD0_CORE")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD0_HVD0_CPU[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD0_HVD0_CPU,
    BDBG_STRING("HW_HVD0_HVD0_CPU")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD1_HVD1_CORE[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD1_HVD1_CORE,
    BDBG_STRING("HW_HVD1_HVD1_CORE")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD1_HVD1_CPU[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD1_HVD1_CPU,
    BDBG_STRING("HW_HVD1_HVD1_CPU")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BDBG_STRING("HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BDBG_STRING("HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BDBG_STRING("HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BDBG_STRING("HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BDBG_STRING("HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BDBG_STRING("HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC0_PWRON_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC0_PWRON_PWRON_PLL,
    BDBG_STRING("HW_SC0_PWRON_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC0_RESET_RESETD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC0_RESET_RESETD,
    BDBG_STRING("HW_SC0_RESET_RESETD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC1_PWRON_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC1_PWRON_PWRON_PLL,
    BDBG_STRING("HW_SC1_PWRON_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SC1_RESET_RESETD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SC1_RESET_RESETD,
    BDBG_STRING("HW_SC1_RESET_RESETD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SYS_CTRL_SC0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SYS_CTRL_SC0,
    BDBG_STRING("HW_SYS_CTRL_SC0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_SYS_CTRL_SC1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_SYS_CTRL_SC1,
    BDBG_STRING("HW_SYS_CTRL_SC1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_V3D_V3D[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_V3D_V3D,
    BDBG_STRING("HW_V3D_V3D")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BDBG_STRING("HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BDBG_STRING("HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_PWRON_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO0_PWRON_PWRON_PLL,
    BDBG_STRING("HW_VCXO0_PWRON_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO0_RESET_RESETD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO0_RESET_RESETD,
    BDBG_STRING("HW_VCXO0_RESET_RESETD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BDBG_STRING("HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BDBG_STRING("HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_PWRON_PWRON_PLL[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO1_PWRON_PWRON_PLL,
    BDBG_STRING("HW_VCXO1_PWRON_PWRON_PLL")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VCXO1_RESET_RESETD[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VCXO1_RESET_RESETD,
    BDBG_STRING("HW_VCXO1_RESET_RESETD")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0,
    BDBG_STRING("HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0,
    BDBG_STRING("HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_VEC_ITU656_0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_VEC_AIO_GFX_VEC_ITU656_0,
    BDBG_STRING("HW_VEC_AIO_GFX_VEC_ITU656_0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BDBG_STRING("HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BDBG_STRING("HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BDBG_STRING("HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4[] = {{
    BCHP_PWR_P_ResourceType_eNonLeafHw,
    BCHP_PWR_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4,
    BDBG_STRING("HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK[] = {{
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK,
    BDBG_STRING("MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK[] = {{
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK,
    BDBG_STRING("MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK[] = {{
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK,
    BDBG_STRING("MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK[] = {{
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK,
    BDBG_STRING("MX_SMARTCARD_MUX_SELECT_SC0_CLOCK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK[] = {{
    BCHP_PWR_P_ResourceType_eMux,
    BCHP_PWR_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK,
    BDBG_STRING("MX_SMARTCARD_MUX_SELECT_SC1_CLOCK")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_HVD_CTRL_CH0_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_HVD_CTRL_CH0_div,
    BDBG_STRING("DV_HVD_CTRL_CH0_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_HVD_CTRL_CH1_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_HVD_CTRL_CH1_div,
    BDBG_STRING("DV_HVD_CTRL_CH1_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_HVD_CTRL_CH2_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_HVD_CTRL_CH2_div,
    BDBG_STRING("DV_HVD_CTRL_CH2_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_HVD_CTRL_CH5_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_HVD_CTRL_CH5_div,
    BDBG_STRING("DV_HVD_CTRL_CH5_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_NETWORK_CTRL_CH5_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_NETWORK_CTRL_CH5_div,
    BDBG_STRING("DV_NETWORK_CTRL_CH5_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_XPT_CTRL_CH0_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_XPT_CTRL_CH0_div,
    BDBG_STRING("DV_XPT_CTRL_CH0_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_XPT_CTRL_CH1_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_XPT_CTRL_CH1_div,
    BDBG_STRING("DV_XPT_CTRL_CH1_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_XPT_CTRL_CH2_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_XPT_CTRL_CH2_div,
    BDBG_STRING("DV_XPT_CTRL_CH2_div")
}};

const BCHP_PWR_P_Resource BCHP_PWR_P_Resource_DV_XPT_CTRL_CH4_div[] = {{
    BCHP_PWR_P_ResourceType_eDiv,
    BCHP_PWR_DV_XPT_CTRL_CH4_div,
    BDBG_STRING("DV_XPT_CTRL_CH4_div")
}};

/* List of resources */
const BCHP_PWR_P_Resource* const BCHP_PWR_P_ResourceList[BCHP_PWR_P_NUM_ALLNODES] = {
    BCHP_PWR_P_Resource_AIO_CLK,
    BCHP_PWR_P_Resource_AIO_SRAM,
    BCHP_PWR_P_Resource_ASP,
    BCHP_PWR_P_Resource_ASP_SRAM,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_AUD_PLL0,
    BCHP_PWR_P_Resource_AUD_PLL1,
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_AVD0,
    BCHP_PWR_P_Resource_AVD0_CLK,
    BCHP_PWR_P_Resource_AVD0_PWR,
    BCHP_PWR_P_Resource_AVD1,
    BCHP_PWR_P_Resource_AVD1_CLK,
    BCHP_PWR_P_Resource_AVD1_PWR,
    BCHP_PWR_P_Resource_BINT_OPEN,
    BCHP_PWR_P_Resource_BVN,
    BCHP_PWR_P_Resource_DMA,
    BCHP_PWR_P_Resource_GRAPHICS3D,
    BCHP_PWR_P_Resource_GRAPHICS3D_SRAM,
    BCHP_PWR_P_Resource_HDMI_RX0_CLK,
    BCHP_PWR_P_Resource_HDMI_RX0_PHY,
    BCHP_PWR_P_Resource_HDMI_RX0_SRAM,
    BCHP_PWR_P_Resource_HDMI_TX0_CLK,
    BCHP_PWR_P_Resource_HDMI_TX0_PHY,
    BCHP_PWR_P_Resource_HSM,
    BCHP_PWR_P_Resource_M2MC,
    BCHP_PWR_P_Resource_M2MC0,
    BCHP_PWR_P_Resource_M2MC0_SRAM,
    BCHP_PWR_P_Resource_M2MC1,
    BCHP_PWR_P_Resource_M2MC_SRAM,
    BCHP_PWR_P_Resource_MAGNUM_CONTROLLED,
    BCHP_PWR_P_Resource_MMM2MC0_SRAM,
    BCHP_PWR_P_Resource_MM_M2MC0,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_RAAGA0_CLK,
    BCHP_PWR_P_Resource_RAAGA0_DSP,
    BCHP_PWR_P_Resource_RAAGA0_SRAM,
    BCHP_PWR_P_Resource_SECURE_ACCESS,
    BCHP_PWR_P_Resource_SMARTCARD0,
    BCHP_PWR_P_Resource_SMARTCARD1,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_VDC_656_OUT,
    BCHP_PWR_P_Resource_VDC_DAC,
    BCHP_PWR_P_Resource_VDC_HDMI_RX_CLK0,
    BCHP_PWR_P_Resource_VDC_HDMI_TX_PHY0,
    BCHP_PWR_P_Resource_VDC_STG0,
    BCHP_PWR_P_Resource_VDC_STG1,
    BCHP_PWR_P_Resource_VDC_VEC,
    BCHP_PWR_P_Resource_VICE,
    BCHP_PWR_P_Resource_VICE0,
    BCHP_PWR_P_Resource_VICE0_CLK,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_XPT_CORE,
    BCHP_PWR_P_Resource_XPT_PACKETSUB,
    BCHP_PWR_P_Resource_XPT_PARSER,
    BCHP_PWR_P_Resource_XPT_PLAYBACK,
    BCHP_PWR_P_Resource_XPT_RAVE,
    BCHP_PWR_P_Resource_XPT_REMUX,
    BCHP_PWR_P_Resource_XPT_SRAM,
    BCHP_PWR_P_Resource_XPT_TSIO,
    BCHP_PWR_P_Resource_XPT_WAKEUP,
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    BCHP_PWR_P_Resource_HW_AIO,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO_SRAM,
    BCHP_PWR_P_Resource_HW_AIO_VEC,
    BCHP_PWR_P_Resource_HW_ASP,
    BCHP_PWR_P_Resource_HW_ASP_ASPT_ASP_540,
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_ASP_HVD0_HVD1,
    BCHP_PWR_P_Resource_HW_ASP_SRAM,
    BCHP_PWR_P_Resource_HW_AUD_PLL0,
    BCHP_PWR_P_Resource_HW_AUD_PLL1,
    BCHP_PWR_P_Resource_HW_BVN,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_SRAM,
    BCHP_PWR_P_Resource_HW_DVPHR0,
    BCHP_PWR_P_Resource_HW_DVPHR0_SECBUS,
    BCHP_PWR_P_Resource_HW_DVPHR0_SRAM,
    BCHP_PWR_P_Resource_HW_DVPHT0,
    BCHP_PWR_P_Resource_HW_HDMI_RX0_PHY,
    BCHP_PWR_P_Resource_HW_HDMI_TX0_PHY,
    BCHP_PWR_P_Resource_HW_HVD0,
    BCHP_PWR_P_Resource_HW_HVD0_HVD0_CORE,
    BCHP_PWR_P_Resource_HW_HVD0_HVD0_CPU,
    BCHP_PWR_P_Resource_HW_HVD0_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD0_SRAM,
    BCHP_PWR_P_Resource_HW_HVD1,
    BCHP_PWR_P_Resource_HW_HVD1_HVD1_CORE,
    BCHP_PWR_P_Resource_HW_HVD1_HVD1_CPU,
    BCHP_PWR_P_Resource_HW_HVD1_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD1_SRAM,
    BCHP_PWR_P_Resource_HW_HVDP0_SRAM,
    BCHP_PWR_P_Resource_HW_HVDP1_SRAM,
    BCHP_PWR_P_Resource_HW_HVDS0_SRAM,
    BCHP_PWR_P_Resource_HW_HVDS1_SRAM,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BCHP_PWR_P_Resource_HW_ITU656_VEC,
    BCHP_PWR_P_Resource_HW_M2MC0_M2MC1,
    BCHP_PWR_P_Resource_HW_M2MC0_SRAM,
    BCHP_PWR_P_Resource_HW_MMM2MC0_SRAM,
    BCHP_PWR_P_Resource_HW_MM_M2MC0,
    BCHP_PWR_P_Resource_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BCHP_PWR_P_Resource_HW_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA0_SRAM,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BCHP_PWR_P_Resource_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_SC0_LDO_PWRON_LDO_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_SC0_PWRON_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_SC0_RESET_RESETD,
    BCHP_PWR_P_Resource_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_SC1_LDO_PWRON_LDO_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_SC1_PWRON_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_SC1_RESET_RESETD,
    BCHP_PWR_P_Resource_HW_SECBUS_VICE20,
    BCHP_PWR_P_Resource_HW_SECBUS_XPT_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_SYS_CTRL_SC0,
    BCHP_PWR_P_Resource_HW_SYS_CTRL_SC1,
    BCHP_PWR_P_Resource_HW_V3D,
    BCHP_PWR_P_Resource_HW_V3D_SRAM,
    BCHP_PWR_P_Resource_HW_V3D_V3D,
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_VCXO0_PWRON_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_VCXO0_RESET_RESETD,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_VCXO1_PWRON_PWRON_PLL,
    BCHP_PWR_P_Resource_HW_VCXO1_RESET_RESETD,
    BCHP_PWR_P_Resource_HW_VDAC,
    BCHP_PWR_P_Resource_HW_VEC,
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0,
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0,
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_VEC_ITU656_0,
    BCHP_PWR_P_Resource_HW_VEC_SRAM,
    BCHP_PWR_P_Resource_HW_VICE20,
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4,
    BCHP_PWR_P_Resource_HW_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_XPT_SRAM,
    BCHP_PWR_P_Resource_HW_XPT_WAKEUP,
    BCHP_PWR_P_Resource_HW_XPT_XPT_CORE_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_XPT_XPT_REMUX,
    BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Resource_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK,
    BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK,
    BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK,
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH0_div,
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH1_div,
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH2_div,
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH5_div,
    BCHP_PWR_P_Resource_DV_NETWORK_CTRL_CH5_div,
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH0_div,
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH1_div,
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH2_div,
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH4_div,
};

/* Coded dependencies */
static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AIO_CLK[] = {
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO,
    BCHP_PWR_P_Resource_HW_AIO_VEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AIO_SRAM[] = {
    BCHP_PWR_P_Resource_HW_AIO_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_ASP[] = {
    BCHP_PWR_P_Resource_HW_ASP,
    BCHP_PWR_P_Resource_HW_ASP_ASPT_ASP_540,
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_ASP_HVD0_HVD1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_ASP_SRAM[] = {
    BCHP_PWR_P_Resource_HW_ASP_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_AIO[] = {
    BCHP_PWR_P_Resource_AIO_CLK,
    BCHP_PWR_P_Resource_AIO_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_PLL0[] = {
    BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_AUD_PLL0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AUD_PLL1[] = {
    BCHP_PWR_P_Resource_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_AUD_PLL1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD[] = {
    BCHP_PWR_P_Resource_AVD0,
    BCHP_PWR_P_Resource_AVD1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0[] = {
    BCHP_PWR_P_Resource_AVD0_CLK,
    BCHP_PWR_P_Resource_AVD0_PWR,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0_CLK[] = {
    BCHP_PWR_P_Resource_HW_HVD0_HVD0_CPU,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD0_PWR[] = {
    BCHP_PWR_P_Resource_HW_HVD0_SRAM,
    BCHP_PWR_P_Resource_HW_HVDP0_SRAM,
    BCHP_PWR_P_Resource_HW_HVDS0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD1[] = {
    BCHP_PWR_P_Resource_AVD1_CLK,
    BCHP_PWR_P_Resource_AVD1_PWR,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD1_CLK[] = {
    BCHP_PWR_P_Resource_HW_HVD1_HVD1_CPU,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_AVD1_PWR[] = {
    BCHP_PWR_P_Resource_HW_HVD1_SRAM,
    BCHP_PWR_P_Resource_HW_HVDP1_SRAM,
    BCHP_PWR_P_Resource_HW_HVDS1_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_BINT_OPEN[] = {
    BCHP_PWR_P_Resource_ASP,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_GRAPHICS3D,
    BCHP_PWR_P_Resource_HDMI_RX0_CLK,
    BCHP_PWR_P_Resource_HDMI_TX0_CLK,
    BCHP_PWR_P_Resource_M2MC,
    BCHP_PWR_P_Resource_MM_M2MC0,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_SMARTCARD0,
    BCHP_PWR_P_Resource_SMARTCARD1,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_VICE,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_XPT_CORE,
    BCHP_PWR_P_Resource_XPT_TSIO,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_BVN[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_DMA[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_GRAPHICS3D[] = {
    BCHP_PWR_P_Resource_HW_V3D_V3D,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_GRAPHICS3D_SRAM[] = {
    BCHP_PWR_P_Resource_HW_V3D_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_RX0_CLK[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_DVPHR0_SECBUS,
    BCHP_PWR_P_Resource_HW_DVPHR0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_RX0_PHY[] = {
    BCHP_PWR_P_Resource_HW_HDMI_RX0_PHY,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_RX0_SRAM[] = {
    BCHP_PWR_P_Resource_HW_DVPHR0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_TX0_CLK[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_DVPHT0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HDMI_TX0_PHY[] = {
    BCHP_PWR_P_Resource_HW_HDMI_TX0_PHY,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HSM[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC[] = {
    BCHP_PWR_P_Resource_M2MC0,
    BCHP_PWR_P_Resource_M2MC1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC0[] = {
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0,
    BCHP_PWR_P_Resource_HW_M2MC0_M2MC1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC0_SRAM[] = {
    BCHP_PWR_P_Resource_HW_M2MC0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC1[] = {
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0,
    BCHP_PWR_P_Resource_HW_M2MC0_M2MC1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_M2MC_SRAM[] = {
    BCHP_PWR_P_Resource_M2MC0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MAGNUM_CONTROLLED[] = {
    BCHP_PWR_P_Resource_ASP,
    BCHP_PWR_P_Resource_ASP_SRAM,
    BCHP_PWR_P_Resource_AUD_AIO,
    BCHP_PWR_P_Resource_AUD_PLL0,
    BCHP_PWR_P_Resource_AUD_PLL1,
    BCHP_PWR_P_Resource_AVD,
    BCHP_PWR_P_Resource_GRAPHICS3D,
    BCHP_PWR_P_Resource_GRAPHICS3D_SRAM,
    BCHP_PWR_P_Resource_HDMI_RX0_CLK,
    BCHP_PWR_P_Resource_HDMI_RX0_PHY,
    BCHP_PWR_P_Resource_HDMI_RX0_SRAM,
    BCHP_PWR_P_Resource_HDMI_TX0_CLK,
    BCHP_PWR_P_Resource_HDMI_TX0_PHY,
    BCHP_PWR_P_Resource_M2MC,
    BCHP_PWR_P_Resource_MMM2MC0_SRAM,
    BCHP_PWR_P_Resource_MM_M2MC0,
    BCHP_PWR_P_Resource_RAAGA,
    BCHP_PWR_P_Resource_SMARTCARD0,
    BCHP_PWR_P_Resource_SMARTCARD1,
    BCHP_PWR_P_Resource_VDC,
    BCHP_PWR_P_Resource_VDC_HDMI_RX_CLK0,
    BCHP_PWR_P_Resource_VDC_HDMI_TX_PHY0,
    BCHP_PWR_P_Resource_VDC_STG0,
    BCHP_PWR_P_Resource_VDC_STG1,
    BCHP_PWR_P_Resource_VICE,
    BCHP_PWR_P_Resource_XPT,
    BCHP_PWR_P_Resource_XPT_CORE,
    BCHP_PWR_P_Resource_XPT_SRAM,
    BCHP_PWR_P_Resource_XPT_TSIO,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MMM2MC0_SRAM[] = {
    BCHP_PWR_P_Resource_HW_MMM2MC0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MM_M2MC0[] = {
    BCHP_PWR_P_Resource_HW_MM_M2MC0,
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA[] = {
    BCHP_PWR_P_Resource_RAAGA0_CLK,
    BCHP_PWR_P_Resource_RAAGA0_DSP,
    BCHP_PWR_P_Resource_RAAGA0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_CLK[] = {
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_DSP[] = {
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_RAAGA0_SRAM[] = {
    BCHP_PWR_P_Resource_HW_RAAGA0_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SECURE_ACCESS[] = {
    BCHP_PWR_P_Resource_HW_SECBUS_XPT_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_DVPHR0_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD0_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD1_SECBUS,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_SECBUS_VICE20,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SMARTCARD0[] = {
    BCHP_PWR_P_Resource_HW_SYS_CTRL_SC0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_SMARTCARD1[] = {
    BCHP_PWR_P_Resource_HW_SYS_CTRL_SC1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC[] = {
    BCHP_PWR_P_Resource_BVN,
    BCHP_PWR_P_Resource_VDC_656_OUT,
    BCHP_PWR_P_Resource_VDC_DAC,
    BCHP_PWR_P_Resource_VDC_VEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_656_OUT[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_VEC_AIO_GFX_VEC_ITU656_0,
    BCHP_PWR_P_Resource_HW_ITU656_VEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_DAC[] = {
    BCHP_PWR_P_Resource_HW_VDAC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_HDMI_RX_CLK0[] = {
    BCHP_PWR_P_Resource_HDMI_RX0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_HDMI_TX_PHY0[] = {
    BCHP_PWR_P_Resource_HDMI_TX0_PHY,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_STG0[] = {
    BCHP_PWR_P_Resource_VICE0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_STG1[] = {
    BCHP_PWR_P_Resource_VICE0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VDC_VEC[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_AIO_VEC,
    BCHP_PWR_P_Resource_HW_ITU656_VEC,
    BCHP_PWR_P_Resource_HW_VEC,
    BCHP_PWR_P_Resource_HW_VEC_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VICE[] = {
    BCHP_PWR_P_Resource_VICE0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VICE0[] = {
    BCHP_PWR_P_Resource_VICE0_CLK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_VICE0_CLK[] = {
    BCHP_PWR_P_Resource_HW_VICE20,
    BCHP_PWR_P_Resource_HW_SECBUS_VICE20,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT[] = {
    BCHP_PWR_P_Resource_XPT_PACKETSUB,
    BCHP_PWR_P_Resource_XPT_PARSER,
    BCHP_PWR_P_Resource_XPT_PLAYBACK,
    BCHP_PWR_P_Resource_XPT_RAVE,
    BCHP_PWR_P_Resource_XPT_REMUX,
    BCHP_PWR_P_Resource_XPT_WAKEUP,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_CORE[] = {
    BCHP_PWR_P_Resource_HW_XPT_XPT_CORE_XPT_REMUX,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PACKETSUB[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PARSER[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_PLAYBACK[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_RAVE[] = {
    BCHP_PWR_P_Resource_XPT_XMEMIF,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_REMUX[] = {
    BCHP_PWR_P_Resource_HW_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_XPT_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_XPT_XPT_CORE_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_SECBUS_XPT_XPT_REMUX,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_SRAM[] = {
    BCHP_PWR_P_Resource_HW_XPT_SRAM,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_TSIO[] = {
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_WAKEUP[] = {
    BCHP_PWR_P_Resource_HW_XPT_WAKEUP,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_XPT_XMEMIF[] = {
    BCHP_PWR_P_Resource_HW_XPT_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_XPT_XPT_CORE_XPT_REMUX,
    BCHP_PWR_P_Resource_HW_SECBUS_XPT_XPT_REMUX,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_ASP_ASPT_ASP_540[] = {
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_AUD_PLL0[] = {
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO,
    BCHP_PWR_P_Resource_HW_AIO_VEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_AUD_PLL1[] = {
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_AIO,
    BCHP_PWR_P_Resource_HW_AIO_VEC,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HDMI_RX0_PHY[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_DVPHR0_SECBUS,
    BCHP_PWR_P_Resource_HW_DVPHR0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HDMI_TX0_PHY[] = {
    BCHP_PWR_P_Resource_HW_ASP_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_BVN_DVPHR0_DVPHT0_VEC,
    BCHP_PWR_P_Resource_HW_DVPHT0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD0_HVD0_CORE[] = {
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD0_HVD0_CPU[] = {
    BCHP_PWR_P_Resource_HW_HVD0,
    BCHP_PWR_P_Resource_HW_ASP_HVD0_HVD1,
    BCHP_PWR_P_Resource_HW_HVD0_HVD0_CORE,
    BCHP_PWR_P_Resource_HW_HVD0_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD1_HVD1_CORE[] = {
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD1_HVD1_CPU[] = {
    BCHP_PWR_P_Resource_HW_ASP_HVD0_HVD1,
    BCHP_PWR_P_Resource_HW_HVD1,
    BCHP_PWR_P_Resource_HW_HVD1_HVD1_CORE,
    BCHP_PWR_P_Resource_HW_HVD1_SECBUS,
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH0_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH1_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH2_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5[] = {
    BCHP_PWR_P_Resource_DV_HVD_CTRL_CH5_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5[] = {
    BCHP_PWR_P_Resource_DV_NETWORK_CTRL_CH5_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0[] = {
    BCHP_PWR_P_Resource_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0[] = {
    BCHP_PWR_P_Resource_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BCHP_PWR_P_Resource_HW_AIO_RAAGA0,
    BCHP_PWR_P_Resource_HW_RAAGA0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_HW_SC0_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC0_PWRON_PWRON_PLL[] = {
    BCHP_PWR_P_Resource_HW_SC0_LDO_PWRON_LDO_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC0_RESET_RESETD[] = {
    BCHP_PWR_P_Resource_HW_SC0_PWRON_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_HW_SC1_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC1_PWRON_PWRON_PLL[] = {
    BCHP_PWR_P_Resource_HW_SC1_LDO_PWRON_LDO_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SC1_RESET_RESETD[] = {
    BCHP_PWR_P_Resource_HW_SC1_PWRON_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SYS_CTRL_SC0[] = {
    BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_SYS_CTRL_SC1[] = {
    BCHP_PWR_P_Resource_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_V3D_V3D[] = {
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    BCHP_PWR_P_Resource_HW_V3D,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO0_PWRON_PWRON_PLL[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO0_RESET_RESETD[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_PWRON_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_HW_VCXO1_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {
    BCHP_PWR_P_Resource_HW_VCXO1_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {
    BCHP_PWR_P_Resource_HW_VCXO1_RESET_RESETD,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO1_PWRON_PWRON_PLL[] = {
    BCHP_PWR_P_Resource_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VCXO1_RESET_RESETD[] = {
    BCHP_PWR_P_Resource_HW_VCXO1_PWRON_PWRON_PLL,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0[] = {
    BCHP_PWR_P_Resource_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0[] = {
    BCHP_PWR_P_Resource_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_VEC_ITU656_0[] = {
    BCHP_PWR_P_Resource_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0[] = {
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH0_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1[] = {
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH1_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2[] = {
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH2_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4[] = {
    BCHP_PWR_P_Resource_DV_XPT_CTRL_CH4_div,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK[] = {
    BCHP_PWR_P_Resource_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Resource_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK[] = {
    BCHP_PWR_P_Resource_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

static const BCHP_PWR_P_Resource* const BCHP_PWR_P_Depend_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK[] = {
    BCHP_PWR_P_Resource_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL
};

/* List of coded dependencies */
const BCHP_PWR_P_Resource* const * const BCHP_PWR_P_DependList[BCHP_PWR_P_NUM_ALLNODES] = {
    BCHP_PWR_P_Depend_AIO_CLK,
    BCHP_PWR_P_Depend_AIO_SRAM,
    BCHP_PWR_P_Depend_ASP,
    BCHP_PWR_P_Depend_ASP_SRAM,
    BCHP_PWR_P_Depend_AUD_AIO,
    BCHP_PWR_P_Depend_AUD_PLL0,
    BCHP_PWR_P_Depend_AUD_PLL1,
    BCHP_PWR_P_Depend_AVD,
    BCHP_PWR_P_Depend_AVD0,
    BCHP_PWR_P_Depend_AVD0_CLK,
    BCHP_PWR_P_Depend_AVD0_PWR,
    BCHP_PWR_P_Depend_AVD1,
    BCHP_PWR_P_Depend_AVD1_CLK,
    BCHP_PWR_P_Depend_AVD1_PWR,
    BCHP_PWR_P_Depend_BINT_OPEN,
    BCHP_PWR_P_Depend_BVN,
    BCHP_PWR_P_Depend_DMA,
    BCHP_PWR_P_Depend_GRAPHICS3D,
    BCHP_PWR_P_Depend_GRAPHICS3D_SRAM,
    BCHP_PWR_P_Depend_HDMI_RX0_CLK,
    BCHP_PWR_P_Depend_HDMI_RX0_PHY,
    BCHP_PWR_P_Depend_HDMI_RX0_SRAM,
    BCHP_PWR_P_Depend_HDMI_TX0_CLK,
    BCHP_PWR_P_Depend_HDMI_TX0_PHY,
    BCHP_PWR_P_Depend_HSM,
    BCHP_PWR_P_Depend_M2MC,
    BCHP_PWR_P_Depend_M2MC0,
    BCHP_PWR_P_Depend_M2MC0_SRAM,
    BCHP_PWR_P_Depend_M2MC1,
    BCHP_PWR_P_Depend_M2MC_SRAM,
    BCHP_PWR_P_Depend_MAGNUM_CONTROLLED,
    BCHP_PWR_P_Depend_MMM2MC0_SRAM,
    BCHP_PWR_P_Depend_MM_M2MC0,
    BCHP_PWR_P_Depend_RAAGA,
    BCHP_PWR_P_Depend_RAAGA0_CLK,
    BCHP_PWR_P_Depend_RAAGA0_DSP,
    BCHP_PWR_P_Depend_RAAGA0_SRAM,
    BCHP_PWR_P_Depend_SECURE_ACCESS,
    BCHP_PWR_P_Depend_SMARTCARD0,
    BCHP_PWR_P_Depend_SMARTCARD1,
    BCHP_PWR_P_Depend_VDC,
    BCHP_PWR_P_Depend_VDC_656_OUT,
    BCHP_PWR_P_Depend_VDC_DAC,
    BCHP_PWR_P_Depend_VDC_HDMI_RX_CLK0,
    BCHP_PWR_P_Depend_VDC_HDMI_TX_PHY0,
    BCHP_PWR_P_Depend_VDC_STG0,
    BCHP_PWR_P_Depend_VDC_STG1,
    BCHP_PWR_P_Depend_VDC_VEC,
    BCHP_PWR_P_Depend_VICE,
    BCHP_PWR_P_Depend_VICE0,
    BCHP_PWR_P_Depend_VICE0_CLK,
    BCHP_PWR_P_Depend_XPT,
    BCHP_PWR_P_Depend_XPT_CORE,
    BCHP_PWR_P_Depend_XPT_PACKETSUB,
    BCHP_PWR_P_Depend_XPT_PARSER,
    BCHP_PWR_P_Depend_XPT_PLAYBACK,
    BCHP_PWR_P_Depend_XPT_RAVE,
    BCHP_PWR_P_Depend_XPT_REMUX,
    BCHP_PWR_P_Depend_XPT_SRAM,
    BCHP_PWR_P_Depend_XPT_TSIO,
    BCHP_PWR_P_Depend_XPT_WAKEUP,
    BCHP_PWR_P_Depend_XPT_XMEMIF,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_ASP_ASPT_ASP_540,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_AUD_PLL0,
    BCHP_PWR_P_Depend_HW_AUD_PLL1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_HDMI_RX0_PHY,
    BCHP_PWR_P_Depend_HW_HDMI_TX0_PHY,
    NULL,
    BCHP_PWR_P_Depend_HW_HVD0_HVD0_CORE,
    BCHP_PWR_P_Depend_HW_HVD0_HVD0_CPU,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_HVD1_HVD1_CORE,
    BCHP_PWR_P_Depend_HW_HVD1_HVD1_CPU,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Depend_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0,
    BCHP_PWR_P_Depend_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0,
    BCHP_PWR_P_Depend_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL,
    BCHP_PWR_P_Depend_HW_SC0_PWRON_PWRON_PLL,
    BCHP_PWR_P_Depend_HW_SC0_RESET_RESETD,
    BCHP_PWR_P_Depend_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    NULL,
    BCHP_PWR_P_Depend_HW_SC1_PWRON_PWRON_PLL,
    BCHP_PWR_P_Depend_HW_SC1_RESET_RESETD,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_SYS_CTRL_SC0,
    BCHP_PWR_P_Depend_HW_SYS_CTRL_SC1,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_V3D_V3D,
    BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Depend_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL,
    BCHP_PWR_P_Depend_HW_VCXO0_PWRON_PWRON_PLL,
    BCHP_PWR_P_Depend_HW_VCXO0_RESET_RESETD,
    BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Depend_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    NULL,
    BCHP_PWR_P_Depend_HW_VCXO1_PWRON_PWRON_PLL,
    BCHP_PWR_P_Depend_HW_VCXO1_RESET_RESETD,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0,
    BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0,
    BCHP_PWR_P_Depend_HW_VEC_AIO_GFX_VEC_ITU656_0,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0,
    BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1,
    BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2,
    BCHP_PWR_P_Depend_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    BCHP_PWR_P_Depend_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Depend_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK,
    BCHP_PWR_P_Depend_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK,
    BCHP_PWR_P_Depend_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK,
    BCHP_PWR_P_Depend_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

#include "bchp_pwr_impl.c"
#include "bchp_pwr_common.c"

void BCHP_PWR_P_HW_ControlId(BCHP_Handle handle, unsigned id, bool activate)
{
    switch(id) {
        case BCHP_PWR_HW_AIO:
            BCHP_PWR_P_HW_AIO_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AIO_RAAGA0:
            BCHP_PWR_P_HW_AIO_RAAGA0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AIO_SRAM:
            BCHP_PWR_P_HW_AIO_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AIO_VEC:
            BCHP_PWR_P_HW_AIO_VEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ASP:
            BCHP_PWR_P_HW_ASP_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ASP_ASPT_ASP_540:
            BCHP_PWR_P_HW_ASP_ASPT_ASP_540_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ASP_BVN_DVPHR0_DVPHT0_VEC:
            BCHP_PWR_P_HW_ASP_BVN_DVPHR0_DVPHT0_VEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ASP_HVD0_HVD1:
            BCHP_PWR_P_HW_ASP_HVD0_HVD1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ASP_SRAM:
            BCHP_PWR_P_HW_ASP_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AUD_PLL0:
            BCHP_PWR_P_HW_AUD_PLL0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_AUD_PLL1:
            BCHP_PWR_P_HW_AUD_PLL1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN:
            BCHP_PWR_P_HW_BVN_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN_DVPHR0_DVPHT0_VEC:
            BCHP_PWR_P_HW_BVN_DVPHR0_DVPHT0_VEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_BVN_SRAM:
            BCHP_PWR_P_HW_BVN_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_DVPHR0:
            BCHP_PWR_P_HW_DVPHR0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_DVPHR0_SECBUS:
            BCHP_PWR_P_HW_DVPHR0_SECBUS_Control(handle, activate);
            break;
        case BCHP_PWR_HW_DVPHR0_SRAM:
            BCHP_PWR_P_HW_DVPHR0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_DVPHT0:
            BCHP_PWR_P_HW_DVPHT0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_RX0_PHY:
            BCHP_PWR_P_HW_HDMI_RX0_PHY_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HDMI_TX0_PHY:
            BCHP_PWR_P_HW_HDMI_TX0_PHY_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD0:
            BCHP_PWR_P_HW_HVD0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD0_HVD0_CORE:
            BCHP_PWR_P_HW_HVD0_HVD0_CORE_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD0_HVD0_CPU:
            BCHP_PWR_P_HW_HVD0_HVD0_CPU_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD0_SECBUS:
            BCHP_PWR_P_HW_HVD0_SECBUS_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD0_SRAM:
            BCHP_PWR_P_HW_HVD0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD1:
            BCHP_PWR_P_HW_HVD1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD1_HVD1_CORE:
            BCHP_PWR_P_HW_HVD1_HVD1_CORE_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD1_HVD1_CPU:
            BCHP_PWR_P_HW_HVD1_HVD1_CPU_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD1_SECBUS:
            BCHP_PWR_P_HW_HVD1_SECBUS_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD1_SRAM:
            BCHP_PWR_P_HW_HVD1_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVDP0_SRAM:
            BCHP_PWR_P_HW_HVDP0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVDP1_SRAM:
            BCHP_PWR_P_HW_HVDP1_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVDS0_SRAM:
            BCHP_PWR_P_HW_HVDS0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVDS1_SRAM:
            BCHP_PWR_P_HW_HVDS1_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_HVD_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1:
            BCHP_PWR_P_HW_HVD_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2:
            BCHP_PWR_P_HW_HVD_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(handle, activate);
            break;
        case BCHP_PWR_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5:
            BCHP_PWR_P_HW_HVD_CH_CTRL_CH_5_POST_DIV_HOLD_CH5_Control(handle, activate);
            break;
        case BCHP_PWR_HW_ITU656_VEC:
            BCHP_PWR_P_HW_ITU656_VEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_M2MC0_M2MC1:
            BCHP_PWR_P_HW_M2MC0_M2MC1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_M2MC0_SRAM:
            BCHP_PWR_P_HW_M2MC0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_MMM2MC0_SRAM:
            BCHP_PWR_P_HW_MMM2MC0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_MM_M2MC0:
            BCHP_PWR_P_HW_MM_M2MC0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5:
            BCHP_PWR_P_HW_NETWORK_CH_CTRL_CH_5_POST_DIV_HOLD_CH5_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA0:
            BCHP_PWR_P_HW_RAAGA0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA0_SRAM:
            BCHP_PWR_P_HW_RAAGA0_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0:
            BCHP_PWR_P_HW_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0:
            BCHP_PWR_P_HW_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_SC0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC0_LDO_PWRON_LDO_PWRON_PLL:
            BCHP_PWR_P_HW_SC0_LDO_PWRON_LDO_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC0_PWRON_PWRON_PLL:
            BCHP_PWR_P_HW_SC0_PWRON_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC0_RESET_RESETD:
            BCHP_PWR_P_HW_SC0_RESET_RESETD_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_SC1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC1_LDO_PWRON_LDO_PWRON_PLL:
            BCHP_PWR_P_HW_SC1_LDO_PWRON_LDO_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC1_PWRON_PWRON_PLL:
            BCHP_PWR_P_HW_SC1_PWRON_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SC1_RESET_RESETD:
            BCHP_PWR_P_HW_SC1_RESET_RESETD_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SECBUS_VICE20:
            BCHP_PWR_P_HW_SECBUS_VICE20_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SECBUS_XPT_XPT_REMUX:
            BCHP_PWR_P_HW_SECBUS_XPT_XPT_REMUX_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SYS_CTRL_SC0:
            BCHP_PWR_P_HW_SYS_CTRL_SC0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_SYS_CTRL_SC1:
            BCHP_PWR_P_HW_SYS_CTRL_SC1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_V3D:
            BCHP_PWR_P_HW_V3D_Control(handle, activate);
            break;
        case BCHP_PWR_HW_V3D_SRAM:
            BCHP_PWR_P_HW_V3D_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_V3D_V3D:
            BCHP_PWR_P_HW_V3D_V3D_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1:
            BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2:
            BCHP_PWR_P_HW_VCXO0_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL:
            BCHP_PWR_P_HW_VCXO0_LDO_PWRON_LDO_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_PWRON_PWRON_PLL:
            BCHP_PWR_P_HW_VCXO0_PWRON_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO0_RESET_RESETD:
            BCHP_PWR_P_HW_VCXO0_RESET_RESETD_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1:
            BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2:
            BCHP_PWR_P_HW_VCXO1_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL:
            BCHP_PWR_P_HW_VCXO1_LDO_PWRON_LDO_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_PWRON_PWRON_PLL:
            BCHP_PWR_P_HW_VCXO1_PWRON_PWRON_PLL_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VCXO1_RESET_RESETD:
            BCHP_PWR_P_HW_VCXO1_RESET_RESETD_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VDAC:
            BCHP_PWR_P_HW_VDAC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC:
            BCHP_PWR_P_HW_VEC_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0:
            BCHP_PWR_P_HW_VEC_AIO_GFX_M2MC0_GFX_M2MC0_M2MC0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0:
            BCHP_PWR_P_HW_VEC_AIO_GFX_MM_M2MC0_GFX_MM_M2MC0_MM_M2MC0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_AIO_GFX_VEC_ITU656_0:
            BCHP_PWR_P_HW_VEC_AIO_GFX_VEC_ITU656_0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VEC_SRAM:
            BCHP_PWR_P_HW_VEC_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_VICE20:
            BCHP_PWR_P_HW_VICE20_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0:
            BCHP_PWR_P_HW_XPT_CH_CTRL_CH_0_POST_DIV_HOLD_CH0_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1:
            BCHP_PWR_P_HW_XPT_CH_CTRL_CH_1_POST_DIV_HOLD_CH1_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2:
            BCHP_PWR_P_HW_XPT_CH_CTRL_CH_2_POST_DIV_HOLD_CH2_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4:
            BCHP_PWR_P_HW_XPT_CH_CTRL_CH_4_POST_DIV_HOLD_CH4_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_REMUX:
            BCHP_PWR_P_HW_XPT_REMUX_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_SRAM:
            BCHP_PWR_P_HW_XPT_SRAM_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_WAKEUP:
            BCHP_PWR_P_HW_XPT_WAKEUP_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_XPT_CORE_XPT_REMUX:
            BCHP_PWR_P_HW_XPT_XPT_CORE_XPT_REMUX_Control(handle, activate);
            break;
        case BCHP_PWR_HW_XPT_XPT_REMUX:
            BCHP_PWR_P_HW_XPT_XPT_REMUX_Control(handle, activate);
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

void BCHP_PWR_P_MUX_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mux, bool set)
{

    switch(resource->id) {
        case BCHP_PWR_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK:
            BCHP_PWR_P_MX_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK_Control(handle, mux, set);
            break;
        case BCHP_PWR_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK:
            BCHP_PWR_P_MX_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK_Control(handle, mux, set);
            break;
        case BCHP_PWR_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK:
            BCHP_PWR_P_MX_ITU656_0_MUX_SELECT_VEC_ITU656_0_CLOCK_Control(handle, mux, set);
            break;
        case BCHP_PWR_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK:
            BCHP_PWR_P_MX_SMARTCARD_MUX_SELECT_SC0_CLOCK_Control(handle, mux, set);
            break;
        case BCHP_PWR_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK:
            BCHP_PWR_P_MX_SMARTCARD_MUX_SELECT_SC1_CLOCK_Control(handle, mux, set);
            break;
        default:
            BDBG_ASSERT(0);
            break;
    }
}

void BCHP_PWR_P_DIV_Control(BCHP_Handle handle, const BCHP_PWR_P_Resource *resource, unsigned *mult, unsigned *prediv, unsigned *postdiv, bool set)
{

    switch(resource->id) {
        case BCHP_PWR_DV_HVD_CTRL_CH0_div:
            BCHP_PWR_P_DV_HVD_CTRL_CH0_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_HVD_CTRL_CH1_div:
            BCHP_PWR_P_DV_HVD_CTRL_CH1_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_HVD_CTRL_CH2_div:
            BCHP_PWR_P_DV_HVD_CTRL_CH2_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_HVD_CTRL_CH5_div:
            BCHP_PWR_P_DV_HVD_CTRL_CH5_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_NETWORK_CTRL_CH5_div:
            BCHP_PWR_P_DV_NETWORK_CTRL_CH5_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_XPT_CTRL_CH0_div:
            BCHP_PWR_P_DV_XPT_CTRL_CH0_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_XPT_CTRL_CH1_div:
            BCHP_PWR_P_DV_XPT_CTRL_CH1_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_XPT_CTRL_CH2_div:
            BCHP_PWR_P_DV_XPT_CTRL_CH2_div_Control(handle, mult, prediv, postdiv, set);
            break;
        case BCHP_PWR_DV_XPT_CTRL_CH4_div:
            BCHP_PWR_P_DV_XPT_CTRL_CH4_div_Control(handle, mult, prediv, postdiv, set);
            break;
        default:
            BDBG_ASSERT(0);
            break;
    }
}

const BCHP_PmapSettings BCHP_PWR_P_DefaultPMapSettings[BCHP_PWR_NUM_P_PMAPSETTINGS] = {
    PMAP(CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, 8),
    PMAP(CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, 6),
    PMAP(CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, 7),
    PMAP(CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5, 6),
    PMAP(CLKGEN_PLL_NETWORK_PLL_CHANNEL_CTRL_CH_5, MDIV_CH5, 3),
    PMAP(CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_0, MDIV_CH0, 6),
    PMAP(CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, 8),
    PMAP(CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2, 15),
    PMAP(CLKGEN_PLL_XPT_PLL_CHANNEL_CTRL_CH_4, MDIV_CH4, 7),
    PMAP(CLKGEN_PLL_XPT_PLL_DIV, NDIV_INT, 120),
    PMAP(CLKGEN_PLL_XPT_PLL_DIV, PDIV, 2),
};

DIVTABLE(DV_HVD_CTRL_CH0_div, -1, -1, 0);
DIVTABLE(DV_HVD_CTRL_CH1_div, -1, -1, 1);
DIVTABLE(DV_HVD_CTRL_CH2_div, -1, -1, 2);
DIVTABLE(DV_HVD_CTRL_CH5_div, -1, -1, 3);
DIVTABLE(DV_NETWORK_CTRL_CH5_div, -1, -1, 4);
DIVTABLE(DV_XPT_CTRL_CH0_div, 9, 10, 5);
DIVTABLE(DV_XPT_CTRL_CH1_div, 9, 10, 6);
DIVTABLE(DV_XPT_CTRL_CH2_div, 9, 10, 7);
DIVTABLE(DV_XPT_CTRL_CH4_div, 9, 10, 8);

const BCHP_PWR_P_FreqMap BCHP_PWR_P_FreqMapList[BCHP_PWR_P_NUM_DIVS] = {
    FREQMAP(DV_HVD_CTRL_CH0_div),
    FREQMAP(DV_HVD_CTRL_CH1_div),
    FREQMAP(DV_HVD_CTRL_CH2_div),
    FREQMAP(DV_HVD_CTRL_CH5_div),
    FREQMAP(DV_NETWORK_CTRL_CH5_div),
    FREQMAP(DV_XPT_CTRL_CH0_div),
    FREQMAP(DV_XPT_CTRL_CH1_div),
    FREQMAP(DV_XPT_CTRL_CH2_div),
    FREQMAP(DV_XPT_CTRL_CH4_div),
};
