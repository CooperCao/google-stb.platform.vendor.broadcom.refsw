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
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_types.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"

BDBG_MODULE(nexus_platform_97342);

/*
 * Platform specific API to assign implicit heap indices for various types of buffers
 */
void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    const NEXUS_PlatformMemory *pMemory = &g_platformMemory; /* g_platformMemory is completely initialized already */

    BSTD_UNUSED(boxMode);

    /*
     * NEXUS_MEMC0_MAIN_HEAP for various data structures in different nexus and magnum modules.
     * Offscreen graphics surfaces can also be created on this heap.
     */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = pMemory->osRegion[0].base >= 512*1024*1024 ? 1:0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    if (pMemory->osRegion[1].base) {
        /* Linux boot command had 3 bmem statements, last bmem= is always to reserve
         * for DOCSIS, not for Nexus heap.
         * NEXUS_MEMC0_GRAPHICS_HEAP would be for creating off screen surfaces
         * by the application with cached mapping only.
         */
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = pMemory->osRegion[1].base >= 512*1024*1024 ? 1:0;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */
    }
}

NEXUS_Error NEXUS_Platform_P_ReadMemcConfig(NEXUS_PlatformMemory *pMemory, NEXUS_PlatformSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BKNI_Memset(pMemory, 0, sizeof(*pMemory));
    pMemory->memc[0].length = 1024*1024*1024; /* bmem will limit */
    return 0;
}

NEXUS_HeapHandle NEXUS_Platform_P_GetFramebufferHeap(unsigned displayIndex)
{
    NEXUS_HeapHandle heapHandle=NULL;
    switch (displayIndex) {
    case NEXUS_OFFSCREEN_SURFACE:
        case NEXUS_SECONDARY_OFFSCREEN_SURFACE:
        case 0: /* HD Display */
        case 1: /* SD Display */
        heapHandle = g_pCoreHandles->heap[0].nexus;
        break;
    default:
        BDBG_MSG(("Invalid display index %d",displayIndex));
        break;
    }
    return heapHandle;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
