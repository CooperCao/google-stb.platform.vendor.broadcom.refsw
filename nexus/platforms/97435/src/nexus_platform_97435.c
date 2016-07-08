/***************************************************************************
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
***************************************************************************/
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clkgen.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"

BDBG_MODULE(nexus_platform_97435);

static unsigned g_num_xcodes = 4;
static void NEXUS_Platform_P_GetNumTranscodes(unsigned *pNumTranscodes);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
#else
    BSTD_UNUSED(pSettings);
#endif
    /* This is the first function to be called, and it is always called. */
    /* Get the number of transcodes for which RTS is configured. */
    NEXUS_Platform_P_GetNumTranscodes(&g_num_xcodes);
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    const NEXUS_PlatformMemory *pMemory = &g_platformMemory; /* g_platformMemory is completely initialized already */
    unsigned managed = NEXUS_MEMORY_TYPE_MANAGED;
    unsigned not_mapped = NEXUS_MEMORY_TYPE_NOT_MAPPED;
#if defined(NEXUS_SAGE_SECURE_HEAP) || defined (NEXUS_VIDEO_SECURE_HEAP)
    bool secure_heap = (NEXUS_GetEnv("secure_heap")!=NULL);
#else
    bool secure_heap = false;
#endif
    BSTD_UNUSED(boxMode);

#if BMMA_USE_STUB
    managed = 0;
    not_mapped = 0;
#endif

    /* nexus default heap, also used for SD2 FB */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    /* if there is no bmem in lower 256MB, heap[0] must move to upper memory. we must also shrink NEXUS_MEMC0_GRAPHICS_HEAP */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = pMemory->osRegion[0].base >= 512*1024*1024 ? 1 : 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    if (g_num_xcodes > 1) {
        unsigned i;
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            if (pMemory->osRegion[i].length && pMemory->osRegion[i].memcIndex == 1) break;
        }
        /* Offscreen gfx surfaces & Main FB. */
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = (i<NEXUS_MAX_HEAPS && pMemory->osRegion[i].length > 512*1024*1024) ? 576*1024*1024 : -1; /* VC4 cannot cross 1GB physical address boundary */
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */
        /* BDBG_LOG(( BDBG_UINT64_FMT ", %d", BDBG_UINT64_FMT(pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].offset), pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size)); */
    }
    /* Offscreen gfx surfaces & Main FB. */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = secure_heap ? 144*1024*1024 : -1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */

    if (g_num_xcodes == 1) {
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    }
    else {
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    }

    if (g_num_xcodes == 4) {
        /* used for VCE 2nd core fw and debug buffer */
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 30 * 1024 * 1024;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memoryType = managed | NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;

        /* MEMC0 device heap for picture buffers (AVD1+VDC+VCE0). */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

        /* MEMC1 device heap for picture buffers: SVD0+VDC+VCE1. */
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

        /* used for XVD fw/general heap and VCE secure heap.
         * TODO: separate XVD fw heap from non-CPU accessible heap to optimize mappable size;
         * TODO: security support for VCE secure heap;
         */
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = 98 * 1024 * 1024;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType = managed | NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
    }
    else if (g_num_xcodes == 2) {
        /* used for VCE 2nd core fw and debug buffer */
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 30  * 1024 * 1024;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memoryType = managed | NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;

        /* MEMC0 device heap for picture buffers (AVD1+VDC). */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

        /* MEMC1 device heap for picture buffers: video decoder, encoder and BVN. */
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

        /* used for XVD fw/general heap and VCE secure heap.
         * TODO: separate XVD fw heap from non-CPU accessible heap to optimize mappable size;
         * TODO: security support for VCE secure heap;
         */
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = 80 * 1024 * 1024;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType = NEXUS_MemoryType_eFull;
    }
    else /* (g_num_xcodes == 1) */{
        /* MEMC0 device heap for picture buffers (AVD1+VDC). */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped; /* unmapped */

        /* used for XVD fw/general heap and VCE secure heap.
         * TODO: separate XVD fw heap from non-CPU accessible heap to optimize mappable size;
         * TODO: security support for VCE secure heap;
         */
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = 70 * 1024 * 1024;
        pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType = NEXUS_MemoryType_eFull;
    }

    if (secure_heap) {
#ifdef NEXUS_VIDEO_SECURE_HEAP
        /* video secure heap. */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 126 * 1024 * 1024;
#endif
    }
}

static void NEXUS_Platform_P_GetNumTranscodes(unsigned *pNumTranscodes)
{
    BREG_Handle hReg = g_pPreInitState->hReg;
    uint32_t regValue, blockout;
    /* Read RTS settings to find the number of hardware (ViCE) video encode channels */
    regValue = BREG_Read32(hReg, BCHP_MEMC_ARB_0_CLIENT_INFO_72);
    blockout = BCHP_GET_FIELD_DATA(regValue, MEMC_ARB_0_CLIENT_INFO_72, BO_VAL);
    if (blockout==0x7fff) { /* MCVP0 off */
        *pNumTranscodes = 4;
    }
    else
    {
        regValue = BREG_Read32(hReg, BCHP_MEMC_ARB_0_CLIENT_INFO_20);
        blockout = BCHP_GET_FIELD_DATA(regValue, MEMC_ARB_0_CLIENT_INFO_20, BO_VAL);
        if (blockout==0x7fff) { /* MADR3_WR off */
            BDBG_ERR(("dual transcode RTS is no longer supported.  Please use CFE>cfgrts to select single or quad transcode"));
            *pNumTranscodes = 2;
        }
        else {
            *pNumTranscodes = 1;
        }
    }
}

/* read memoryMode & memSizes[].
this function does not know memc offsets.
this function is needed for memc's that linux doesn't report (e.g. non-UMA 7405 MEMC1)
*/
NEXUS_Error NEXUS_Platform_P_ReadMemcConfig(NEXUS_PlatformMemory *pMemory, NEXUS_PlatformSettings *pSettings)
{
    int rc;

    rc = NEXUS_Platform_P_ReadGenericMemcConfig(pMemory, pSettings);
    if (rc) return BERR_TRACE(rc);

    BDBG_MSG(("Memory indicates: MEMC0: %d MB, MEMC1: %d MB", pMemory->memc[0].length >> 20, pMemory->memc[1].length >> 20));
    BDBG_WRN(("CFE RTS is configured for %u transcodes",g_num_xcodes));
#if NEXUS_PLATFORM_97435_1STB1T
    if (g_num_xcodes>1) {
        BDBG_ERR(("For cfgrts dual or quad transcode, must build Nexus _without_ NEXUS_PLATFORM_97435_1STB1T=y."));
        BKNI_Memset(pMemory, 0, sizeof(*pMemory)); /* force failure */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
#else
    if (g_num_xcodes==1) {
        BDBG_ERR(("For cfgrts single transcode, must build Nexus with NEXUS_PLATFORM_97435_1STB1T=y."));
        BKNI_Memset(pMemory, 0, sizeof(*pMemory));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
#endif
    else {
    return 0;
    }
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
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

#if NEXUS_PLATFORM_P_READ_BOX_MODE
unsigned NEXUS_Platform_P_ReadBoxMode(void)
{
    static bool set = false;
    static unsigned boxmode = 0;
    /* only read once per run */
    if (!set) {
        NEXUS_Platform_P_GetNumTranscodes(&g_num_xcodes);
        boxmode = (g_num_xcodes == 1) ? 2 : 1;
        BDBG_WRN(("Box Mode %d set by cfgrts in CFE.",boxmode));
        set = true;
    }
    return boxmode;
}
#endif
