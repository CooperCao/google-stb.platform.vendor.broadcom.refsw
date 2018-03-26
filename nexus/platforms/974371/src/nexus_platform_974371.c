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
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_974371);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
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
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 204*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 132*1024*1024; /* on screen graphics for primary display */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
   const char *board = NULL;
   /* some day this needs to become run-time vs. compile time
      74371 only has one board currently 74371 XID */
   board = "74371 XID";

    if (board)
    {
       BDBG_WRN(("*** Initializing %s Board ...***", board));
    }
    else
    {
       BDBG_WRN(("*** Initializing a 74371 Board, but not subtype detected ...***"));
    }
    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
    return;
}
