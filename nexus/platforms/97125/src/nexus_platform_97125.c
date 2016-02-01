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
* API Description:
*   API name: Platform Core
*    7400 core module initialization
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_types.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"

#include "bchp_memc_ddr_0.h"

#if NEXUS_CONFIG_IMAGE
#include "nexus_img_kernel.h"
#endif

#ifdef NO_OS
#include "nexus_core_module.h"
#endif

BDBG_MODULE(nexus_platform_97125);

#ifndef NEXUS_DOCSIS_HEAP
#define NEXUS_DOCSIS_HEAP (NEXUS_MAX_HEAPS-1)
#endif

/*
 * Platform specific API to assign implicit heap indices for various types of buffers
 */
void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    static NEXUS_PlatformMemory platformMemory;
    NEXUS_PlatformMemory *pMemory = &platformMemory;
    int i;

    BSTD_UNUSED(boxMode);

    NEXUS_Platform_P_GetHostMemory(pMemory); /* get bmem regions from OS, un-concatenated. */

    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 0;
    pSettings->heap[NEXUS_DOCSIS_HEAP].size = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 0;


    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (0==pMemory->osRegion[i].length) {
            break;
        }
        if (pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size == 0) {
            /*
             * NEXUS_MEMC0_MAIN_HEAP for various data structures in different nexus and magnum modules.
             * Offscreen graphics surfaces can also be created on this heap.
             */
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = pMemory->osRegion[i].base >= 512*1024*1024 ? 1:0;
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = pMemory->osRegion[i].length;
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;
            BDBG_MSG(("Main heap OS region %d, %u@%x",i,pMemory->osRegion[i].length,pMemory->osRegion[i].base));
        }
        else if (pMemory->osRegion[i].base + pMemory->osRegion[i].length == 256*1024*1024) {
        /* if 2 or more bmem entries, the second is DOCSIS */
            pSettings->heap[NEXUS_DOCSIS_HEAP].size = pMemory->osRegion[i].length;
            BDBG_MSG(("DOCSIS heap OS region %d, %u@%x",i,pMemory->osRegion[i].length,pMemory->osRegion[i].base));
            pSettings->heap[NEXUS_DOCSIS_HEAP].memcIndex = 0;
            pSettings->heap[NEXUS_DOCSIS_HEAP].subIndex = 0; /* always in lower 256MB */
            pSettings->heap[NEXUS_DOCSIS_HEAP].memoryType = NEXUS_MEMORY_TYPE_RESERVED; /* no mapping, no allocations. external use. */
        }
        else {
#ifdef NEXUS_MEMC0_GRAPHICS_HEAP
            /* Linux boot command had 3 bmem statements
             * NEXUS_MEMC0_GRAPHICS_HEAP would be for creating off screen surfaces
             * by the application with cached mapping only.
             */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = pMemory->osRegion[i].base >= 512*1024*1024 ? 1:0;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = pMemory->osRegion[i].base >= 512*1024*1024 ? -1 : (int)pMemory->osRegion[i].length;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */
            BDBG_MSG(("Graphics heap OS region %d, %u@%x",i,pMemory->osRegion[i].length,pMemory->osRegion[i].base));
#endif
        }
    }
    if (pSettings->heap[NEXUS_DOCSIS_HEAP].size == 0) {
        BDBG_WRN(("No DOCSIS heap region specified.  Use a bmem= statement on the kernel boot command line to reserve DOCSIS memory at the top of first 256MB"));
    }
#if NEXUS_HAS_DISPLAY
#if (NEXUS_DISPLAY_NUM_SD_BUFFERS || NEXUS_DISPLAY_NUM_HD_BUFFERS || NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS)
    pSettings->displayModuleSettings.fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS;
    pSettings->displayModuleSettings.hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS;
    pSettings->displayModuleSettings.sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS;
#endif
#endif

#if NEXUS_HAS_VIDEO_DECODER
#if NEXUS_VIDEO_DECODER_GENERAL_HEAP_SIZE
    pSettings->videoDecoderModuleSettings.heapSize[0].general = NEXUS_VIDEO_DECODER_GENERAL_HEAP_SIZE;
    pSettings->videoDecoderModuleSettings.heapSize[0].secure = NEXUS_VIDEO_DECODER_SECURE_HEAP_SIZE;
    pSettings->videoDecoderModuleSettings.heapSize[0].picture = NEXUS_VIDEO_DECODER_PICTURE_HEAP_SIZE;
#endif
#endif
}

NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    if ((g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus) && (NEXUS_OFFSCREEN_SURFACE==displayIndex)) {
        return g_pCoreHandles->heap[NEXUS_MEMC0_GRAPHICS_HEAP].nexus;
    }
    else {
        return g_pCoreHandles->heap[NEXUS_MEMC0_MAIN_HEAP].nexus;
    }
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{

}

