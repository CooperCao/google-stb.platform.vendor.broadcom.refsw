/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

BDBG_MODULE(nexus_platform_97255);

#define MB (1024 * 1024)

typedef struct NEXUS_Platform_P_PllChannelConfig {
    uint32_t postdiv;
    uint32_t vco;
    uint32_t pdiv;
    uint32_t ndiv;
    uint32_t freq;
} NEXUS_Platform_P_PllChannelConfig;

static void NEXUS_Platform_P_ReadPllChannel(const char* pllName, const char* pllChannelName, uint32_t pllChannel, uint32_t pllDiv, NEXUS_Platform_P_PllChannelConfig *config)
{
    uint32_t reg;

    reg = BREG_Read32(g_pCoreHandles->reg, pllDiv);
    config->pdiv = (reg >> 10) & 0xf;
    config->ndiv = reg & 0x1ff;
    config->vco = 54/config->pdiv * config->ndiv;

    reg = BREG_Read32(g_pCoreHandles->reg, pllChannel);
    config->postdiv = (reg >> 1) & 0xff;
    config->freq = config->vco / config->postdiv;
    BDBG_MSG(("%s - %s\t- div - %2d  - Freq %dMHz ", pllName, pllChannelName, config->postdiv, config->freq));
}

static void NEXUS_P_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH264_Mvc);

    pSettings->videoDecoder[0].mosaic.maxNumber = 0;
#else /* NEXUS_HAS_VIDEO_DECODER */
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = NEXUS_P_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    /* Main driver heap */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 128 * MB;
    pSettings->heap[NEXUS_ARR_HEAP].size = 2 * MB;

    if (g_platformMemory.memoryLayout.memc[0].size <= 512 * MB) {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size /= 2;
    }

    /* Compressed buffer heap */
    switch(boxMode)
    {
        /* Box 4 and 5 are UHD modes so we need a larger CRR region. */
        case 4:
        case 5:
        {
            pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 64 * MB;
            break;
        }
        /* Box 3 allows 2 1080p decodes. */
        case 3:
        {
            pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 48 * MB;
            break;
        }
        /* Single 1080p decode. */
        default:
        {
            pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 32 * MB;
        }

    }

    /* Graphics memory */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 32 * MB;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType = NEXUS_HEAP_TYPE_GRAPHICS;

    /* On platforms with more memory increase the graphics heap to give 3D core more memory.
    *  2GB systems 256MB for GFX, 1GB use 128MB. */
    if (g_platformMemory.memoryLayout.memc[0].size >= 1024 * MB) {
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = g_platformMemory.memoryLayout.memc[0].size /8;
    }
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    char *board;
    NEXUS_PlatformStatus platformStatus;
    NEXUS_Platform_P_PllChannelConfig config;

#if NEXUS_CPU_ARM64
    const char *mode = "64 bit";
#else /* NEXUS_CPU_ARM */
    const char *mode = "32 bit compatability";
#endif

    NEXUS_Platform_GetStatus(&platformStatus);

    switch (platformStatus.boardId.major)
    {
        case 1:
            board = "SV";
            break;
        case 2:
            board = "DV";
            break;
        case 3:
            board = "USFF";
            break;
        case 4:
            board = "PCK";
            break;
        case 6:
            board = "HB";
            break;
       case 12:
            board = "2L";
            break;
        default:
            board = "unknown";
            break;
    }

    BDBG_WRN(("Initialising %s platform in %s mode", board, mode));

    /* Check the selected box mode is compatible with the chip type. */
    switch (platformStatus.chipId) {
        case 0x72554:
            /* 72554 supports all box modes, but not all box modes can be used with all pmaps, check SCB freq. */

            NEXUS_Platform_P_ReadPllChannel("PLL SYS0 PLL", "SCB" ,BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_2, BCHP_CLKGEN_PLL_SYS0_PLL_DIV, &config);

            if (((config.freq) < 388) && (platformStatus.boxMode == 4)) {
                BDBG_ERR(("Incorrect combination of box mode and pmap!"));
                BDBG_ERR(("To use box mode 4 select pmap 5. Or if you need pmap 6 for lower power, use box mode 5."));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }

            break;
        case 0x72553:
        {
            if (platformStatus.boxMode > 3 ) {
                BDBG_ERR(("Only box modes 1 - 3 are supported on %x",platformStatus.chipId));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            break;
        }
        case 0x73574:
        {
            /* for future reference */
            break;
        }
        case 0x7255:
        default:
        {
            if (platformStatus.boxMode > 2 ) {
                BDBG_ERR(("Only box modes 1 & 2 are supported on %x",platformStatus.chipId));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
    }

#if 0
    /* Debug AV Clock configuration, need to also up the debug level to see console prints */
    NEXUS_Platform_P_ReadPllChannel("PLL AVX PLL", "HVD CPU" ,BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_1,BCHP_CLKGEN_PLL_AVX_PLL_DIV, &config);
    NEXUS_Platform_P_ReadPllChannel("PLL AVX PLL", "HVD Core" ,BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_2,BCHP_CLKGEN_PLL_AVX_PLL_DIV, &config);
    NEXUS_Platform_P_ReadPllChannel("PLL AVX PLL", "M2MC" ,BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_3,BCHP_CLKGEN_PLL_AVX_PLL_DIV, &config);
    NEXUS_Platform_P_ReadPllChannel("PLL AVX PLL", "V3D" ,BCHP_CLKGEN_PLL_AVX_PLL_CHANNEL_CTRL_CH_4,BCHP_CLKGEN_PLL_AVX_PLL_DIV, &config);
#endif

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}

#if BDBG_DEBUG_BUILD
/* UART IDs for UUI, defined in BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0 */
const struct NEXUS_Platform_P_UartId NEXUS_Platform_P_UartIds[] =
{
    {0,"NO_CPU"},
    {3,"HVD0_OL"},
    {4,"HVD0_IL"},
    {11,"AVS_TOP"},
    {12,"LEAP"},
    {14,"DPFE"},
    {15,"SCPU"},
    {0,NULL}
};
#endif
