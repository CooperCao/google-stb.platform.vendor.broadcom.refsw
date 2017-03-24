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
 *****************************************************************************/
#include "bstd.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"
#include "bchp_clkgen.h"

BDBG_MODULE(nexus_platform_97268);

#define MB (1024 * 1024)

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH265);
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eVp9);
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH264_Mvc);

    switch (g_pPreInitState->boxMode) {
    case 1:
    case 2:
        pSettings->videoDecoder[0].mosaic.maxNumber = 4;
        pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
        pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        break;
    /* Supported on B0 only*/
    case 5:
    case 6:
    case 7:
    case 8:
        pSettings->videoDecoder[0].mosaic.maxNumber = 3;
        pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
        pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        pSettings->videoDecoder[1].mosaic.maxNumber = 1;
        pSettings->videoDecoder[1].mosaic.maxWidth = 1920;
        pSettings->videoDecoder[1].mosaic.maxHeight = 1088;
        pSettings->videoDecoder[1].mosaic.colorDepth = 10;
        break;
    case 9:
        pSettings->videoDecoder[0].mosaic.maxNumber = 3;
        pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
        pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        break;
    default:
        break;
    }
#else /* NEXUS_HAS_VIDEO_DECODER */
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 160*MB;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 96*MB;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 64*MB;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType = NEXUS_HEAP_TYPE_GRAPHICS;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    char *board;
    NEXUS_PlatformStatus platformStatus;

#if NEXUS_CPU_ARM64
    const char *mode = "64 bit";
#elif NEXUS_CPU_ARM
    const char *mode = "32 bit compatibility";
#endif

    NEXUS_Platform_GetStatus(&platformStatus);

    switch (platformStatus.boardId.major)
    {
        case 1:
            board = "SV";
            break;
        case 2:
            board = ((platformStatus.boardId.minor == 4)? "LC6L" : "USFF");
            break;
        case 3:
            board = "DV";
            break;
        case 4:
            board = "HB";
            break;
        case 5:
            board = "VMS";
            break;
        default:
            board = "unknown";
            break;
    }

    BDBG_WRN(("Initialising %s platform in %s mode", board, mode));

#if 0
    {
        uint32_t reg;
        unsigned postdiv,vco,pdiv,ndiv;

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_DIV);
        pdiv = (reg >> 10) & 0xf;
        ndiv = reg & 0x1ff;
        vco = 54/pdiv * ndiv;
        BDBG_ERR((" PLL AVX PLL  VCO - %dMHz", vco));

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_0);
        postdiv = (reg >> 1) & 0xff;
        BDBG_ERR((" PLL AVX CTRL CH0 - SID\t\t- div - %2d  - Freq %dMHz ", postdiv, vco / postdiv));

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1);
        postdiv = (reg >> 1) & 0xff;
        BDBG_ERR((" PLL AVX CTRL CH1 - HVD CPU\t- div - %2d  - Freq %dMHz ", postdiv, vco / postdiv));

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2);
        postdiv = (reg >> 1) & 0xff;
        BDBG_ERR((" PLL AVX CTRL CH2 - HVD Core\t- div - %2d  - Freq %dMHz ", postdiv, vco / postdiv));

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3);
        postdiv = (reg >> 1) & 0xff;
        BDBG_ERR((" PLL AVX CTRL CH3 - M2MC\t\t- div - %2d  - Freq %dMHz ", postdiv, vco / postdiv));

        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4);
        postdiv = (reg >> 1) & 0xff;
        BDBG_ERR((" PLL AVX CTRL CH4 - V3D\t\t- div - %2d  - Freq %dMHz ", postdiv, vco / postdiv));
    }
#endif

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
