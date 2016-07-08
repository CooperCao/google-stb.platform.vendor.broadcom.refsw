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
#include "bchp_clkgen.h"

BDBG_MODULE(nexus_platform_97364);

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

#if(NEXUS_USE_73649_SFF) || (NEXUS_USE_7364_SFF)
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 148*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 0*1024*1024; /* force shared non-secure video for 512 MB combo */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 88*1024*1024; /* on screen graphics for primary display */
#else
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 208*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 128*1024*1024; /* on screen graphics for primary display */
#endif
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
}

void NEXUS_Platform_P_InitRaaga(void)
{
    /* 20150209 SW7364-142: run raaga at 648MHz for 1U MS12 config B and 576p encode */
    BREG_AtomicUpdate32(g_pCoreHandles->reg, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0,
        BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_MASK, 0x5 << BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_SHIFT);
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
   const char *board = NULL;
   /* some day this needs to become run-time vs. compile time */
#if NEXUS_USE_7364_SV
    board = "7364 SV";
#elif defined NEXUS_USE_73649_SV
   board = "73649 SV";
#elif NEXUS_USE_73649_SFF
   board = "73649 SFF";
#elif defined NEXUS_USE_7364_SFF
    board = "7364 SFF";
#endif

    if (board)
    {
       BDBG_WRN(("*** Initializing %s Board ...***", board));
    }
    else
    {
       BDBG_WRN(("*** Initializing a 7364 Board, but not subtype detected ...***"));
    }

    NEXUS_Platform_P_InitRaaga();

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}


