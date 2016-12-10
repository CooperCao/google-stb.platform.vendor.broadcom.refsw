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

BDBG_MODULE(nexus_platform_97250);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_P_SupportVideoDecoderCodec(pSettings, NEXUS_VideoCodec_eH265);
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
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
    BSTD_UNUSED(boxMode);

#if NEXUS_HAS_SAGE
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].size = 32*1024*1024;
#endif

    if (g_platformMemory.memoryLayout.memc[0].size <= (512*1024*1024))
    {
        BDBG_WRN(("Using low memory heap configuration"));
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 128*1024*1024;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 64*1024 *1024;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 64*1024*1024;

    }
    else
    {
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 258*1024*1024;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 192*1024*1024;
    }

    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = (0x00400000);

    return;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    const char *board = NULL;
    /* run-time vs. compile time */
#if NEXUS_USE_7250_DGL
    board = "7250 DGL";
#elif defined NEXUS_USE_7250_CWF
    board = "7250 CWF";
#elif defined NEXUS_USE_7250_SV
    board = "7250 SV";
#elif NEXUS_USE_7250_USFF
    board = "7250 USFF";
#elif NEXUS_USE_7250_ACX16
    board = "7250 ACFFX16";
#elif NEXUS_USE_7250_CD2
    board = "7250 CD2";
#elif NEXUS_USE_72501_SFF
    board = "72501 SFF/USFF";
#elif NEXUS_USE_72501_SAT
    board = "72501 SFFSAT";
#endif

    if (board)
    {
        BDBG_WRN(("*** Initializing %s Board ... ***", board));
    }
    else
    {
        BDBG_WRN(("*** Initializing a 7250 Board, but no subtype detected ... ***"));
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
    return;
}
