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
#include "bchp_aon_ctrl.h"

BDBG_MODULE(nexus_platform_97278);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH265);
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eVp9);
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
    switch (g_pPreInitState->boxMode) {
        case 1:
        case 1001:
            pSettings->videoDecoder[0].mosaic.maxNumber = 3;
            pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
            pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
            pSettings->videoDecoder[0].mosaic.colorDepth = 10;
            break;
        case 2:
            pSettings->videoDecoder[0].mosaic.maxNumber = 3;
            pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
            pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
            pSettings->videoDecoder[0].mosaic.colorDepth = 10;
            pSettings->videoDecoder[1].mosaic.maxNumber = 3;
            pSettings->videoDecoder[1].mosaic.maxWidth = 1920;
            pSettings->videoDecoder[1].mosaic.maxHeight = 1088;
            pSettings->videoDecoder[1].mosaic.colorDepth = 10;
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
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 148*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 108*1024 *1024;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 64*1024*1024;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType = NEXUS_HEAP_TYPE_GRAPHICS;
    switch(boxMode)
    {
        case 1:
        case 1001:
        {
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 64*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType = NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            break;
        }
        case 2:
        case 1000:
        {
            break;
        }
        default:
        {
            BDBG_ERR(("Box mode %d not supported",boxMode));
            break;
        }
    }

    if (BCHP_GET_FIELD_DATA(BREG_Read32(g_pPreInitState->hReg, BCHP_AON_CTRL_GLOBAL_ADDRESS_MAP_VARIANT_LOCKED),
        AON_CTRL_GLOBAL_ADDRESS_MAP_VARIANT_LOCKED, map_variant_locked)) {
        BDBG_WRN(("v7-64 Memory Map Variant detected."));
        pSettings->heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_HIGH_MEMORY | NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
        pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].memoryType |= NEXUS_MEMORY_TYPE_HIGH_MEMORY | NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_HIGH_MEMORY | NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
        pSettings->heap[NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP].memoryType |= NEXUS_MEMORY_TYPE_HIGH_MEMORY | NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
        pSettings->heap[NEXUS_MEMC1_SECURE_GRAPHICS_HEAP].memoryType |= NEXUS_MEMORY_TYPE_HIGH_MEMORY | NEXUS_MEMORY_TYPE_MANAGED | NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
    }
    return;
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
            board = "HB";
            break;
        default:
            board = "unknown";
            break;
    }

    BDBG_WRN(("Initializing %s platform in %s mode", board, mode));

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
