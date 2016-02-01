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
#include "bchp_memc_misc_0.h"
#include "bchp_memc_ddr_0.h"

BDBG_MODULE(nexus_platform_97208);

#ifdef NO_OS
#include "nexus_core_module.h"
#endif

#if NEXUS_HAS_VIDEO_DECODER
/* If the user supplied video decoder settings not available,
   then use default  Video Buffers based on the chip usage modes */
static void NEXUS_Platform_P_VideoDecoderSettings(NEXUS_VideoDecoderModuleSettings *pSettings)
{
    unsigned numDecodes=0;
    unsigned i;

    for(i=0;i<NEXUS_NUM_XVD_DEVICES;i++) {
        if(pSettings->heapSize[i].general || pSettings->heapSize[i].secure || pSettings->heapSize[i].picture )
        {
            BDBG_MSG(("Using User Defined Video Decoder[%u] Heaps", i));
            continue;
        }

          pSettings->heapSize[i].general = NEXUS_VIDEO_DECODER_GENERAL_HEAP_SIZE;
            pSettings->heapSize[i].secure = NEXUS_VIDEO_DECODER_SECURE_HEAP_SIZE;
            pSettings->heapSize[i].picture = NEXUS_VIDEO_DECODER_PICTURE_HEAP_SIZE;


    }
        numDecodes=NEXUS_NUM_VIDEO_DECODERS;
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
static void NEXUS_Platform_P_DisplaySettings(NEXUS_DisplayModuleSettings *pSettings)
{
    if(pSettings->sdBuffers.count || pSettings->hdBuffers.count || pSettings->fullHdBuffers.count)
    {
        BDBG_WRN(("Using User Defined Display Buffers"));
        return;
    }

    pSettings->displayHeapSettings[0].fullHdBuffers.count = NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS;
    pSettings->displayHeapSettings[0].hdBuffers.count = NEXUS_DISPLAY_NUM_HD_BUFFERS;
    pSettings->displayHeapSettings[0].sdBuffers.count = NEXUS_DISPLAY_NUM_SD_BUFFERS;
    pSettings->legacy.numWindowsPerDisplay = 1; /* no PIP */
}
#endif

/*
 * Platform specific API to assign implicit heap indices for various types of buffers
 */
void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    BSTD_UNUSED(boxMode);
    /*
     * Main heap for various data structures in different nexus and magnum modules.
     */
    pSettings->heap[0].memcIndex = 0;
    pSettings->heap[0].subIndex = 0;
    pSettings->heap[0].size = -1;
    pSettings->heap[0].memoryType = NEXUS_MemoryType_eFull;
#if 0
    /*
     * Picture memory for XVD0
     */
    pSettings->heap[1].memcIndex = 1;
    pSettings->heap[1].subIndex = 0;
    pSettings->heap[1].size = -1;
    pSettings->heap[1].memoryType = NEXUS_MemoryType_eDeviceOnly;

    /*
     * Picture memory for XVD1
     */
    pSettings->heap[2].memcIndex = 2;
    pSettings->heap[2].subIndex = 0;
    pSettings->heap[2].size = -1;
    pSettings->heap[2].memoryType = NEXUS_MemoryType_eDeviceOnly;
#endif
    /* adjust module settings based on MEMC config */
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Platform_P_VideoDecoderSettings(&pSettings->videoDecoderModuleSettings);
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_Platform_P_DisplaySettings(&pSettings->displayModuleSettings);
#endif
    return;
}

NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    NEXUS_HeapHandle heapHandle=NULL;
    BSTD_UNUSED(displayIndex);
    heapHandle = g_pCoreHandles->heap[0].nexus; /* default heap for surface creation */
    return heapHandle;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
