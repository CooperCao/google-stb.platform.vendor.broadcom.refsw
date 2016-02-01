/***************************************************************************
*     (c)2010-2013 Broadcom Corporation
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
#if(BCHP_VER >= BCHP_VER_B0)
#include "bchp_memc_ddr23_aphy_ac_0.h"
#include "bchp_memc_ddr23_aphy_ac_1.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#endif

BDBG_MODULE(nexus_platform_97420);

#ifdef NO_OS
#include "nexus_core_module.h"
#endif

#if NEXUS_HAS_VIDEO_DECODER
/* If the user supplied video decoder settings not available,
   then use default  Video Buffers based on the chip usage modes */
static void NEXUS_Platform_P_VideoDecoderSettings(NEXUS_VideoDecoderModuleSettings *pSettings)
{

     if(pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].general || pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].secure
        || pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].picture ){
        BDBG_MSG(("Using User Defined Video Decoder[%u] Heaps", NEXUS_MEMC0_MAIN_HEAP));
     }
     else
     {
        pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].general = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_GENERAL_HEAP_SIZE;
        pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].secure = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_SECURE_HEAP_SIZE;
        pSettings->heapSize[NEXUS_MEMC0_MAIN_HEAP].picture = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_PICTURE_HEAP_SIZE;
        pSettings->avdHeapIndex[NEXUS_MEMC0_MAIN_HEAP] = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->mfdMapping[NEXUS_MEMC0_MAIN_HEAP] = 0;

     }


     if(pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].general || pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].secure
        || pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].picture ){
        BDBG_MSG(("Using User Defined Video Decoder[%u] Heaps", NEXUS_MEMC0_MAIN_HEAP));
     }
     else
     {
        pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].general = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_GENERAL_HEAP_SIZE;
        pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].secure = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_SECURE_HEAP_SIZE;
        pSettings->heapSize[NEXUS_MEMC1_MAIN_HEAP].picture = NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_PICTURE_HEAP_SIZE;
        pSettings->avdHeapIndex[NEXUS_MEMC1_MAIN_HEAP] = NEXUS_MEMC1_MAIN_HEAP;
        pSettings->mfdMapping[NEXUS_MEMC1_MAIN_HEAP] = 1;

     }
    return;
}
#endif


#if NEXUS_HAS_DISPLAY

/* If the user supplied video decoder settings not available,
 * then use default  Video Buffers based on the chip usage modes
 */

static void NEXUS_Platform_P_DisplaySettings(NEXUS_DisplayModuleSettings *pSettings)
{
    unsigned i;
    NEXUS_DisplayHeapSettings displayHeapSettings;
    bool displayMemConfigSet=false;

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        displayHeapSettings = pSettings->displayHeapSettings[i];
        if(displayHeapSettings.fullHdBuffers.count || displayHeapSettings.hdBuffers.count ||
           displayHeapSettings.sdBuffers.count || displayHeapSettings.fullHdBuffers.pipCount ||
           displayHeapSettings.hdBuffers.pipCount || displayHeapSettings.sdBuffers.pipCount){
            displayMemConfigSet=true;
            break;
        }
    }

    if(displayMemConfigSet){
        BDBG_WRN(("Using User Defined Display Buffers"));
        return;
    }
    else{
        pSettings->primaryDisplayHeapIndex = NEXUS_MEMC1_MAIN_HEAP;
        for(i=0;i<NEXUS_MAX_HEAPS;i++ ) {
            switch(i)
            {
            case NEXUS_MEMC0_MAIN_HEAP:
                pSettings->displayHeapSettings[i].fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_ON_MEMC0;
                pSettings->displayHeapSettings[i].hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_ON_MEMC0;
                pSettings->displayHeapSettings[i].sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_ON_MEMC0;
                pSettings->displayHeapSettings[i].fullHdBuffers.pipCount = NEXUS_DISPLAY_NUM_FULLHD_PIP_BUFFERS_ON_MEMC0;
                pSettings->displayHeapSettings[i].hdBuffers.pipCount = NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_ON_MEMC0;
                pSettings->displayHeapSettings[i].sdBuffers.pipCount = NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_ON_MEMC0;
                break;
            case NEXUS_MEMC1_MAIN_HEAP:
                pSettings->displayHeapSettings[i].fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_ON_MEMC1;
                pSettings->displayHeapSettings[i].hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS_ON_MEMC1;
                pSettings->displayHeapSettings[i].sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS_ON_MEMC1;
                pSettings->displayHeapSettings[i].fullHdBuffers.pipCount = NEXUS_DISPLAY_NUM_FULLHD_PIP_BUFFERS_ON_MEMC1;
                pSettings->displayHeapSettings[i].hdBuffers.pipCount = NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_ON_MEMC1;
                pSettings->displayHeapSettings[i].sdBuffers.pipCount = NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_ON_MEMC1;
                break;
            default:
                pSettings->displayHeapSettings[i].fullHdBuffers.count = 0;
                pSettings->displayHeapSettings[i].hdBuffers.count = 0;
                pSettings->displayHeapSettings[i].sdBuffers.count = 0;
                pSettings->displayHeapSettings[i].fullHdBuffers.pipCount = 0;
                pSettings->displayHeapSettings[i].hdBuffers.pipCount = 0;
                pSettings->displayHeapSettings[i].sdBuffers.pipCount = 0;
               break;
             }
          pSettings->displayHeapSettings[i].fullHdBuffers.format = NEXUS_VideoFormat_e1080p;
          pSettings->displayHeapSettings[i].fullHdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
          pSettings->displayHeapSettings[i].hdBuffers.format = NEXUS_VideoFormat_e1080i;
          pSettings->displayHeapSettings[i].hdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
          pSettings->displayHeapSettings[i].sdBuffers.format = NEXUS_VideoFormat_ePalG;
          pSettings->displayHeapSettings[i].sdBuffers.pixelFormat = NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8;
          if(g_pCoreHandles->heap[i].nexus){
              NEXUS_Heap_SetDisplayHeapSettings(g_pCoreHandles->heap[i].nexus, &pSettings->displayHeapSettings[i]);
          }
        }


       /*
        * Based on RTS settings for BCM97420
        */
        pSettings->videoWindowHeapIndex[0][0] = NEXUS_MEMC1_MAIN_HEAP;
        pSettings->videoWindowHeapIndex[0][1] = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->videoWindowHeapIndex[1][0] = NEXUS_MEMC1_MAIN_HEAP;
        pSettings->videoWindowHeapIndex[1][1] = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->videoWindowHeapIndex[2][0] = NEXUS_MEMC0_MAIN_HEAP;
        pSettings->videoWindowHeapIndex[2][1] = NEXUS_MEMC0_MAIN_HEAP;
    }
    return;
}
#endif

/*
 * Platform specific API to assign implicit heap indices for various types of buffers
 */
void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    BSTD_UNUSED(boxMode);
    /*
     * NEXUS_MEMC0_MAIN_HEAP for various data structures in different nexus and magnum modules.
     * Note: This heap would be used for frame buffers on MEMC0 controller also if needed.
     * Offscreen graphics surfaces can also be created on this heap.
     */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 180 * 1024 * 1024;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    /*
     * NEXUS_MEMC1_MAIN_HEAP would be for video buffers and frame buffers on MEMC1. Offscreen
     * graphics surfaces can also be created on this heap.
     */
    pSettings->heap[NEXUS_MEMC1_MAIN_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_MAIN_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC1_MAIN_HEAP].size = 96*1024*1024;
    pSettings->heap[NEXUS_MEMC1_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    /*
     * NEXUS_MEMC0_GRAPHICS_HEAP would be for creating off screen surfaces
     * by the application with cached mapping only.
     */
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memcIndex = 1;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Platform_P_VideoDecoderSettings(&pSettings->videoDecoderModuleSettings);
#endif
    return;
}


NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    NEXUS_HeapHandle heapHandle=NULL;
    switch (displayIndex) {
    case 0: /* HD Display */
        heapHandle = g_pCoreHandles->heap[NEXUS_MEMC1_MAIN_HEAP].nexus;
        break;
    case 1: /*SD1 Display */
        heapHandle = g_pCoreHandles->heap[NEXUS_MEMC1_MAIN_HEAP].nexus;
        break;
    case 2: /*SD2 Display aka 3rd Display*/
      heapHandle = g_pCoreHandles->heap[NEXUS_MEMC0_MAIN_HEAP].nexus;
      break;
    case NEXUS_OFFSCREEN_SURFACE:
        case NEXUS_SECONDARY_OFFSCREEN_SURFACE:
      heapHandle = g_pCoreHandles->heap[NEXUS_MEMC1_GRAPHICS_HEAP].nexus;
      break;
    default:
      BDBG_MSG(("Invalid display index %d",displayIndex));
    }
    return heapHandle;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
#ifdef NO_OS
    NO_OS_MemoryTest(g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, g_pCoreHandles->heap[0],
        g_pCoreHandles->heap[1], g_pCoreHandles->heap[2], memSize0, memSize1, 0);
#endif

    /* these are called after the Core module is up. */
#if NEXUS_HAS_DISPLAY
    NEXUS_Platform_P_DisplaySettings(&g_NEXUS_platformSettings.displayModuleSettings);
#endif

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
