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
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr23_shim_addr_cntl_1.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#include "bchp_clkgen.h"

#if defined BDSP_VP6_SUPPORT || defined BDSP_H264_ENCODE_SUPPORT
#include "bchp_memc_arb_0.h"
#endif

BDBG_MODULE(nexus_platform_97425);

static void nexus_p_modifyMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    pRtsSettings->videoDecoder[0].mfdIndex = 0;
    pRtsSettings->videoDecoder[0].avdIndex = 0;
    pRtsSettings->videoDecoder[1].mfdIndex = 1;
    pRtsSettings->videoDecoder[1].avdIndex = 1;
    pRtsSettings->videoDecoder[2].mfdIndex = 2;
    pRtsSettings->videoDecoder[2].avdIndex = 1;
    pRtsSettings->avd[0].memcIndex = 1;
    pRtsSettings->avd[1].memcIndex = 0;
#else
    BSTD_UNUSED(pRtsSettings);
#endif
}

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
#else
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryRtsSettings = nexus_p_modifyMemoryRtsSettings;
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    const NEXUS_PlatformMemory *pMemory = &g_platformMemory; /* g_platformMemory is completely initialized already */
    unsigned managed = NEXUS_MEMORY_TYPE_MANAGED;
    unsigned not_mapped = NEXUS_MEMORY_TYPE_NOT_MAPPED;

    /* bmem=192M@64M bmem=458M@512M */
    BSTD_UNUSED(boxMode);

#if BMMA_USE_STUB
    managed = 0;
    not_mapped = 0;
#endif

    /* heap[0] is nexus default heap, also used for SD2 FB */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    if (pMemory->osRegion[0].base >= 512*1024*1024) {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 192 * 1024 * 1024;
    }
    else {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;
    }
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    /* heap[1] Offscreen gfx surfaces & Main FB for displays 0 and 1. */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */

    /* heap[2] Gfx surface for displays 2 and 3. */
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].subIndex = 0;
#if NEXUS_BASE_OS_ucos_ii
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256 * 1024 * 1024;
#else
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = pMemory->memoryLayout.memc[1].region[0].size > 512*1024*1024 ? 768 * 1024 * 1024 : -1;
#endif
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */

    /* heap[3] used for VCE fw and debug buffer */
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 4 * 1024 * 1024;
    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memoryType = NEXUS_MemoryType_eFull;

    /* heap[4] MEMC0 device heap for picture buffers (AVD1+VDC). */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

    /* heap[5] MEMC1 device heap for picture buffers: video decoder, encoder and BVN. */
    pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
    pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

    /* heap[6] used for XVD fw/general heap and VCE secure heap.
     * TODO: separate XVD fw heap from non-CPU accessible heap to optimize mappable size;
     * TODO: security support for VCE secure heap;
     */
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = 78 * 1024 * 1024;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType =  managed | NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;

    pSettings->i2c[0].clock.type = NEXUS_GpioType_eAonSpecial;
    pSettings->i2c[0].clock.gpio = 0;
    pSettings->i2c[0].data.type = NEXUS_GpioType_eAonSpecial;
    pSettings->i2c[0].data.gpio = 1;
}

/***************************************************************************
Summary:
    Based on the RTS settings for each platform, framebuffer for each display
    could be placed on any heaps. This API shall return the heap handle
    for each frame buffer.
See Also:
    NEXUS_Platform_P_GetFramebufferHeap
 ***************************************************************************/
NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    NEXUS_HeapHandle heapHandle=NULL;
    switch (displayIndex) {
    case 0: /* HD Display */
    case 1: /* SD1 Display */
    case NEXUS_OFFSCREEN_SURFACE:
        heapHandle = g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus;
        break;
    case 2: /* SD2 Display aka 3rd Display*/
    case 3: /* SD3 Display aka 4th Display on MEMC1 */
    case NEXUS_SECONDARY_OFFSCREEN_SURFACE:
        heapHandle = g_pCoreHandles->heap[NEXUS_MEMC1_GRAPHICS_HEAP].nexus;
        break;
    default:
        break;
    }
    return heapHandle;
}


NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
	uint32_t regData;

    /* do work after Core module is up. */
#if (BCHP_VER == BCHP_VER_A1)
	/* See SW7425-413. Remove this when B0 arrives; the CFE standard ViCE2 and Raaga clock settings should work for B0. */
	/* set the vice2 clock to 347MHz and raaga clock to 405MHz */
	regData = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_DIV);
	regData &= ~(BCHP_MASK(CLKGEN_PLL_RAAGA_PLL_DIV, PDIV));
	regData &= ~(BCHP_MASK(CLKGEN_PLL_RAAGA_PLL_DIV, NDIV_INT));
	regData |= BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_DIV, PDIV, 0x2);
	/*	regData |= (BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_DIV, NDIV_INT, 0x90); */
	regData |= BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_DIV, NDIV_INT, 0x5A);
	BREG_Write32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_DIV, regData);

	BREG_AtomicUpdate32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0,
		BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_MASK, 0x7 << BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_SHIFT);

	BREG_AtomicUpdate32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_1,
		BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_1_MDIV_CH1_MASK, 0x6 << BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_1_MDIV_CH1_SHIFT);

	regData = BREG_Read32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_GAIN);
	regData &= ~(BCHP_MASK(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_PROPORTIONAL_IN_PHASE));
	regData &= ~(BCHP_MASK(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_INTEGRATOR_IN_PHASE));
	regData &= ~(BCHP_MASK(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_IN_FREQ));
	regData |= BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_PROPORTIONAL_IN_PHASE, 0x7);
	regData |= BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_INTEGRATOR_IN_PHASE, 0x3);
	regData |= BCHP_FIELD_DATA(CLKGEN_PLL_RAAGA_PLL_GAIN, LOOP_GAIN_IN_FREQ, 0x2);
	BREG_Write32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_GAIN, regData);
#else
	BSTD_UNUSED(regData);
#endif

#if BCHP_VER >= BCHP_VER_B0 && (defined BDSP_VP6_SUPPORT || defined BDSP_H264_ENCODE_SUPPORT)
    regData = BREG_Read32(g_pCoreHandles->reg, BCHP_MEMC_ARB_0_CLIENT_INFO_51);
    regData &= ~(BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_51, RR_EN)|
                 BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_51, BO_VAL));
    regData |= BCHP_FIELD_DATA(MEMC_ARB_0_CLIENT_INFO_51, RR_EN, 1);        /* Enable round-robin */
    regData |= BCHP_FIELD_DATA(MEMC_ARB_0_CLIENT_INFO_51, BO_VAL, 0xd8);    /* 1us blockout */
    BREG_Write32(g_pCoreHandles->reg, BCHP_MEMC_ARB_0_CLIENT_INFO_51, regData);
#endif
    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
    return;
}

#if NEXUS_USE_7425_SV_BOARD
extern NEXUS_Error NEXUS_Platform_P_InitExternalRfm(const NEXUS_PlatformConfiguration *pConfig)
{
    NEXUS_GpioHandle channelGpio;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_GpioStatus gpioStatus;
    bool channel4 = false;
    static uint8_t channel3Data[4] = {0x80, 0x00, 0x1e, 0xa3};
    static uint8_t channel4Data[4] = {0x80, 0x00, 0x21, 0xa3};
    uint8_t *pData;

    /* Make sure that platform initialized the i2c controllers */
    if ( pConfig->i2c[NEXUS_I2C_CHANNEL_EXT_RFM])
    {
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    channelGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 8, &gpioSettings);
    if ( channelGpio )
    {
        NEXUS_Gpio_GetStatus(channelGpio, &gpioStatus);
        channel4 = gpioStatus.value == NEXUS_GpioValue_eLow ? false : true;
        NEXUS_Gpio_Close(channelGpio);
    }

    BDBG_WRN(("Initializing external RFM for channel %u", channel4?4:3));
    pData = channel4?channel4Data:channel3Data;

    NEXUS_I2c_WriteNoAddr(pConfig->i2c[NEXUS_I2C_CHANNEL_EXT_RFM], 0x65, pData, 4);
    }

    return NEXUS_SUCCESS;
}
#endif
