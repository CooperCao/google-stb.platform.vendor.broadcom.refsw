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
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_clkgen.h"
#include "bchp_75845.h"
#include "bchp_gio_aon.h"

#if (defined BDSP_VP6_SUPPORT || defined BDSP_H264_ENCODE_SUPPORT)
#include "bchp_memc_arb_0.h"
#endif

BDBG_MODULE(nexus_platform_975845);

static void nexus_p_modifyDefaultMemoryConfigurationSettings(NEXUS_MemoryConfigurationSettings *pSettings)
{
#if NEXUS_HAS_VIDEO_DECODER
    int i;
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++)
    {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    }
    pSettings->videoDecoder[0].colorDepth = 10;
    pSettings->videoDecoder[1].colorDepth = 10;
#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#endif
#else
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    /* kernel suggested boot options bmem=192M@64M bmem=512M@512M for boards with >750M memory
       bmem=192M@64M bmem=256M@512M for the boards with 512 Mbytes of memory */
    NEXUS_PlatformMemory *pMemory = &g_platformMemory;
    bool osRegion1Available = (pMemory->osRegion[1].base && pMemory->osRegion[1].length) ? true : false;

#if defined(NEXUS_SAGE_SECURE_HEAP) || defined (NEXUS_VIDEO_SECURE_HEAP)
    bool secure_heap = (NEXUS_GetEnv("secure_heap")!=NULL);
#else
    bool secure_heap = false;
#endif

    BSTD_UNUSED(boxMode);

    if (secure_heap) {
        if (!osRegion1Available) {
            BDBG_ERR(("not enough space for running secure heaps."));
        }
    }

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

    if (secure_heap) {
#ifdef NEXUS_VIDEO_SECURE_HEAP
        /* video secure heap. */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 128 * 1024 * 1024;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eSecure;
#endif
    }

    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = (secure_heap ||osRegion1Available) ? 1 : 0;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* calculated */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = NEXUS_MemoryType_eDeviceOnly; /* unmapped */

    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1; /* if !osRegion1Available, this heap will not be created */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].optional = true;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    uint32_t regData;
    uint32_t val;
    BCHP_Info chpInfo;

    BCHP_GetInfo(g_pCoreHandles->chp, &chpInfo);

#if (defined BDSP_VP6_SUPPORT || defined BDSP_H264_ENCODE_SUPPORT)
    regData = BREG_Read32(g_pCoreHandles->reg, BCHP_MEMC_ARB_0_CLIENT_INFO_51);
    if (BCHP_GET_FIELD_DATA(regData, MEMC_ARB_0_CLIENT_INFO_51, RR_EN) != 1 ||
        BCHP_GET_FIELD_DATA(regData, MEMC_ARB_0_CLIENT_INFO_51, BO_VAL) != 0x11e)
    {
        BDBG_WRN(("/******************************************************************************/"));
        BDBG_WRN(("MEMC0 RTS SETINGS ARE INCORRECT FOR RAAGA TRANSCODE. PLEASE UPDATE TO LATEST CFE"));
        BDBG_WRN(("/******************************************************************************/"));
    }
#else
    BSTD_UNUSED(regData);
#endif

    /* for 7583v10, configure aon_gpio_06 as output high */
    if (chpInfo.productId == BCHP_BCM75835)
    {
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_AON_IODIR_LO);
        val &= ~(1 << 6);
        BREG_Write32(g_pCoreHandles->reg, BCHP_GIO_AON_IODIR_LO, val);

        val = BREG_Read32(g_pCoreHandles->reg, BCHP_GIO_AON_DATA_LO);
        if (((val >> 6) & 0x1) == 0)
        {
            val |= (1 << 6);
            BREG_Write32(g_pCoreHandles->reg, BCHP_GIO_AON_DATA_LO, val);
        }
    }
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
