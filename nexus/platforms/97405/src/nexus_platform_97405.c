/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_platform_priv.h"
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clk.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_platform_97405);

#ifdef NO_OS
#include "nexus_core_module.h"
#endif

static unsigned g_memoryMode;

/* minimum needed on MEMC0 region[0] heap */
#ifndef NEXUS_MEMC0_MAIN_HEAP_SIZE
#define NEXUS_MEMC0_MAIN_HEAP_SIZE (156*1024*1024)
#endif

#if NEXUS_HAS_VIDEO_DECODER
/* If the user supplied video decoder settings not available,
   then use default  Video Buffers based on the chip usage modes */
static void NEXUS_Platform_P_VideoDecoderSettings(unsigned memoryMode ,NEXUS_VideoDecoderModuleSettings *pSettings)
{
    unsigned numDecodes=0;
    unsigned i;

    for(i=0;i<NEXUS_NUM_XVD_DEVICES;i++) {
        switch (memoryMode )
        {
        default:
        case 0: /* 000b - 64b UMA */
            pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_UMA64_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_UMA64_SECURE_HEAP_SIZE4;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_UMA32_PICTURE_HEAP_SIZE;
            numDecodes=NEXUS_NUM_VIDEO_DECODERS_UMA64;
            break;
        case 1: /* 001b - 32b UMA */
            pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_UMA32_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_UMA32_SECURE_HEAP_SIZE;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_UMA32_PICTURE_HEAP_SIZE;
            numDecodes=NEXUS_NUM_VIDEO_DECODERS_UMA32;
            break;
        case 2: /* 010b - 16b UMA */
            pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_UMA16_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_UMA16_SECURE_HEAP_SIZE;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_UMA16_PICTURE_HEAP_SIZE;
            numDecodes=NEXUS_NUM_VIDEO_DECODERS_UMA16;
            /* limit decoder to SD only decode */
            pSettings->maxDecodeFormat = NEXUS_VideoFormat_ePal;
            pSettings->maxStillDecodeFormat = NEXUS_VideoFormat_ePal;
            break;
        case 4: /* 100b - 32b+16b non-UMA */
            pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_NONUMA3216_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_NONUMA3216_SECURE_HEAP_SIZE;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_NONUMA3216_PICTURE_HEAP_SIZE;
            numDecodes=NEXUS_NUM_VIDEO_DECODERS_NONUMA3216;
            if (!pSettings->avdHeapIndex[0]) {
                pSettings->avdHeapIndex[0] = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            }
            break;
        case 5: /* 101b - 16b+16b non-UMA */
            pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_NONUMA1616_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_NONUMA1616_SECURE_HEAP_SIZE;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_NONUMA1616_PICTURE_HEAP_SIZE;
            numDecodes=NEXUS_NUM_VIDEO_DECODERS_NONUMA1616;
            if (!pSettings->avdHeapIndex[0]) {
                pSettings->avdHeapIndex[0] = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
            }
            break;
        }
    }
    if(!pSettings->numDecodes || !pSettings->numStillDecodes || !pSettings->numMosaicDecodes )
    {
        pSettings->numDecodes = numDecodes;
        /* TODO based on the memory modes */
        pSettings->numStillDecodes=NEXUS_NUM_STILL_DECODES;
        pSettings->numMosaicDecodes=NEXUS_NUM_MOSAIC_DECODES;
    }

    return;
}
#endif

#if NEXUS_HAS_DISPLAY
/* If the user supplied video decoder settings not available,
   then use default  Video Buffers based on the chip usage modes */
static void NEXUS_Platform_P_DisplaySettings(unsigned memoryMode ,NEXUS_DisplayModuleSettings *pSettings)
{
    BSTD_UNUSED(memoryMode);
    if(pSettings->sdBuffers.count || pSettings->hdBuffers.count || pSettings->fullHdBuffers.count)
    {
        BDBG_WRN(("Using User Defined Display Buffers"));
        return;
    }

    /* override defaults. limit capabilities based on MEMC strapping options. */
    switch (memoryMode )
    {
    default:
    case 0: /* 000b - 64b UMA */
        pSettings->sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_UMA64;
        pSettings->hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_UMA64;
        pSettings->fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_UMA64;
        BDBG_WRN(("%s",NEXUS_UMA64_STR));
        break;
    case 1: /* 001b - 32b UMA */
        pSettings->sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_UMA32;
        pSettings->hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_UMA32;
        pSettings->fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_UMA32;
        pSettings->legacy.numWindowsPerDisplay = NEXUS_NUM_VIDEO_DECODERS_UMA32;
        BDBG_WRN(("%s",NEXUS_UMA32_STR));
        break;
    case 2: /* 010b - 16b UMA */
        pSettings->sdBuffers.count =  NEXUS_DISPLAY_NUM_SD_BUFFERS_UMA16;
        pSettings->hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_UMA16;
        pSettings->fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_UMA16;
        pSettings->legacy.numDisplays = 1; /* no CMP1 */
        pSettings->legacy.numWindowsPerDisplay = 1; /* no PIP */
        BDBG_WRN(("%s",NEXUS_UMA16_STR));
        break;
    case 4: /* 100b - 32b+16b non-UMA */
        pSettings->sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_NONUMA3216;
        pSettings->hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_NONUMA3216;
        pSettings->fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_NONUMA3216;
        BDBG_WRN(("%s",NEXUS_NONUMA3216_STR));
        break;
    case 5: /* 101b - 16b+16b non-UMA */
        pSettings->sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_NONUMA1616;
        pSettings->hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_NONUMA1616;
        pSettings->fullHdBuffers.count =NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_NONUMA1616;
        pSettings->legacy.numWindowsPerDisplay = 1; /* no PIP */
        BDBG_WRN(("%s",NEXUS_NONUMA1616_STR));
        break;
    }
    return;
}
#endif

/*
 * Platform specific API to assign implicit heap indices for various types of buffers

   Following api strictly uses the memory configuration of the kernel
   for 2630+ kernels, boot using
   UMA 64 & UMA 32 assuming 512M memory as in 97405 board
   bmem=192M@64M bmem=256M@512M
   Non-UMA 32+16 Total memory = 256, 64M
   bmem=192M@64M

   for 2618 kernels
   mem=64M no graphics heap is available
   For UMA modes decoder picture buffer always come form NEXUS_MEMC0_MAIN_HEAP
   For non UMA mode decoder picture bufffers always come from NEXUS_MEMC0_PICTURE_BUFFER_HEAP
*/
void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    const NEXUS_PlatformMemory *pMemory = &g_platformMemory; /* g_platformMemory is completely initialized already */

    BSTD_UNUSED(boxMode);

    /* NEXUS_MEMC0_MAIN_HEAP for various data structures in different nexus and magnum modules. */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    if (pMemory->memc[0].region[0].length >= NEXUS_MEMC0_MAIN_HEAP_SIZE) {
        /* graphics heap never created in lower memory, so use it all */
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;
    }
    else {
        /* if no room in lower memory, there must be room in upper memory */
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 1;
        /* use a fixed size so we can share with graphics heap */
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = NEXUS_MEMC0_MAIN_HEAP_SIZE;
    }
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    /* only create a separate graphics heap if there's upper memory. graphics heap features app-only mmap. */
    if (pMemory->memc[0].region[1].length) {
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1; /* the remainder */
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */
    }

    /* non UMA case */
    if (pMemory->memc[1].region[0].length) {
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = -1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = NEXUS_MemoryType_eDeviceOnly; /* unmapped */
    }

    /* adjust module settings based on MEMC config */
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Platform_P_VideoDecoderSettings(g_memoryMode, &pSettings->videoDecoderModuleSettings);
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_Platform_P_DisplaySettings(g_memoryMode, &pSettings->displayModuleSettings);
#endif
}

NEXUS_Error NEXUS_Platform_P_ReadMemcConfig(NEXUS_PlatformMemory *pMemory, NEXUS_PlatformSettings *pSettings)
{
    BREG_Handle hReg = g_pPreInitState->hReg;
    uint32_t regValue;
    unsigned memSizes[2];

    BSTD_UNUSED(pSettings);

    regValue = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);

    BDBG_MSG(("SUN_TOP_CTRL.STRAP_VALUE_0 = 0x%08x", regValue));

    g_memoryMode = BCHP_GET_FIELD_DATA(regValue, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr_configuration);

    /* each chip is 16 bit wide, depeding on the number width of the bus, determine number of chips */
    switch (g_memoryMode)
    {
    default:
    case 0: /* 000b - 64b UMA */
        memSizes[0] = 4;
        memSizes[1] = 0;
        break;
    case 1: /* 001b - 32b UMA */
        memSizes[0] = 2;
        memSizes[1] = 0;
        break;
    case 2: /* 010b - 16b UMA */
        memSizes[0] = 1;
        memSizes[1] = 0;
        break;
    case 4: /* 100b - 32b+16b non-UMA */
        memSizes[0] = 2;
        memSizes[1] = 1;
        break;
    case 5: /* 101b - 16b+16b non-UMA */
        memSizes[0] = 1;
        memSizes[1] = 1;
        break;
    }

    /* Now, we have the number of 16-bit parts per bank and we can calculate the total bank size based on 16-bit part size */
    switch ( BCHP_GET_FIELD_DATA(regValue, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr0_device_config) )
    {
    case 0:
        memSizes[0] *= 16;
        break;
    case 1:
        memSizes[0] *= 32;
        break;
    default:
    case 2:
        memSizes[0] *= 64;
        break;
    case 3:
        memSizes[0] *= 128;
        break;
    }
    switch ( BCHP_GET_FIELD_DATA(regValue, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr1_device_config) )
    {
    case 0:
        memSizes[1] *= 16;
        break;
    case 1:
        memSizes[1] *= 32;
        break;
    default:
    case 2:
        memSizes[1] *= 64;
        break;
    case 3:
        memSizes[1] *= 128;
        break;
    }
    /* Convert to MB current values are size divided by 16b (2 bytes) */
    memSizes[0] *= ((1024*1024)*2);
    memSizes[1] *= ((1024*1024)*2);

    /* keeping artificial 256 MB limit for platforms near the end of their SW release schedule */
    if (memSizes[0] > 256 * 1024 * 1024) {
        memSizes[0] = 256 * 1024 * 1024;
    }

#ifdef NEXUS_SECURITY_VIDEO_VERIFICATION_LEGACY_65NM
    /* Reserve some memory for video FW */
    BDBG_WRN(("MEMC0 was indicating %d MB but it is forced to 254 MB", memSizes[0] >> 20));
    memSizes[0] = XVD_PHYSICAL_ADDRESS;
#endif


    BDBG_MSG(("Memory indicates: MEMC0: %d MB, MEMC1: %d MB (memory mode %d)", memSizes[0] >> 20, memSizes[1] >> 20, g_memoryMode));

    /* populate memc[] */
    pMemory->memc[0].length = memSizes[0];
    pMemory->memc[1].length = memSizes[1];

    return 0;
}

NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    BSTD_UNUSED(displayIndex);
    if (g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus) {
        return g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus;
    }
    else {
        return g_pCoreHandles->heap[NEXUS_MEMC0_MAIN_HEAP].nexus;
    }
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    uint32_t regValue;

    /* Change the Clock to 81 Mhz(1) for UHF not default 27 Mhz(0)*/
    regValue = BREG_Read32(g_pCoreHandles->reg, BCHP_CLK_MISC);
    regValue &= ~BCHP_MASK(CLK_MISC, UHFR_CLK_SEL);
    regValue |= (BCHP_FIELD_DATA(CLK_MISC, UHFR_CLK_SEL, 1));
    BREG_Write32(g_pCoreHandles->reg, BCHP_CLK_MISC , regValue);

#ifdef NO_OS
    NO_OS_MemoryTest(g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, g_pCoreHandles->heap[0],
        g_pCoreHandles->heap[1], g_pCoreHandles->heap[2], memSize0, memSize1, 0);
#endif

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
