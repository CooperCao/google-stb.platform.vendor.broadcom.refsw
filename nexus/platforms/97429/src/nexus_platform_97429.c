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
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_arb_0.h"

BDBG_MODULE(nexus_platform_97429);

#define MB (1024 * 1024)

static void nexus_p_modifyDefaultMemoryConfigurationSettings(NEXUS_MemoryConfigurationSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    const NEXUS_PlatformMemory *pMemory = &g_platformMemory; /* g_platformMemory is completely initialized already */
    BSTD_UNUSED(boxMode);
    /* kernel suggested boot options bmem=192M@64M bmem=512M@512M for boards with >750M memory
       bmem=192M@64M bmem=192M@512M for the boards with 512 Mbytes of memory */

    /* nexus default heap, also used for SD2 FB */
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    /* if there is no bmem in lower 256MB, heap[0] must move to upper memory. we must also shrink NEXUS_MEMC0_GRAPHICS_HEAP */
    if (pMemory->osRegion[0].base >= 512*1024*1024) {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 192 * 1024 * 1024;
    }
    else {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;
    }
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* calculated */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = NEXUS_MemoryType_eDeviceOnly; /* unmapped */

    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* cached only */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;

}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
#if (defined BDSP_VP6_SUPPORT || defined BDSP_H264_ENCODE_SUPPORT)
    uint32_t regData;

    regData = BREG_Read32(g_pCoreHandles->reg, BCHP_MEMC_ARB_0_CLIENT_INFO_51);
    regData &= ~(BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_51, RR_EN)|
                 BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_51, BO_VAL));
    regData |= BCHP_FIELD_DATA(MEMC_ARB_0_CLIENT_INFO_51, RR_EN, 1);        /* Enable round-robin */
    regData |= BCHP_FIELD_DATA(MEMC_ARB_0_CLIENT_INFO_51, BO_VAL, 0xd8);    /* 1us blockout */
    BREG_Write32(g_pCoreHandles->reg, BCHP_MEMC_ARB_0_CLIENT_INFO_51, regData);
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
