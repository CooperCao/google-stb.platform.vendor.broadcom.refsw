/***************************************************************************
*     (c)2010-2014 Broadcom Corporation
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
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_97250);

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
    unsigned boxMode = g_pPreInitState->boxMode;
    unsigned i;
#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++)
    {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    }
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;

    switch (boxMode)
    {
        default:
        case 1: /* Main + PiP */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080i;
            break;
        case 2: /* Main only @ 8-bit, no PiP */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[1].used = false; /* Single Decoder */
	    break;
        case 3: /* Main only @ 8-bit, no PiP */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[1].used = false; /* Single Decoder */
            break;
        case 4: /* Main + xcode */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[1].colorDepth = 8;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080i;
            break;
        case 5: /* Headless, 1 xcode */
        case 6: /* Main only, no PiP (similar to case 2) */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[1].used = false; /* Single Decoder */
            break;
        case 7: /* Main + PiP @ 8-bit */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[1].colorDepth = 8;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_eNtsc;
            break;
        case 8: /* Single decoder, multi-PIP (2x) */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[0].mosaic.maxNumber = 2;
            pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
            pSettings->videoDecoder[0].mosaic.maxHeight = 1080;
            pSettings->videoDecoder[1].used = false; /* Single Decoder */
            break;
    }

#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
    /* this is needed since Atlas tries to use the first iframe of 4K stream as thumb-nail */
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#endif /* NEXUS_NUM_STILL_DECODES */

#else
    BSTD_UNUSED(i);
#endif
}


void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}



void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    enum BMEM_SIZE { BMEM_SIZE_LOW, BMEM_SIZE_MED, BMEM_SIZE_HIGH } bmem_size;

    if (g_platformMemory.memc[0].length < (512*1024*1024))
        bmem_size = BMEM_SIZE_LOW;
    else if (g_platformMemory.memc[0].length > (1024*1024*1024))
        bmem_size = BMEM_SIZE_HIGH;
    else
        bmem_size = BMEM_SIZE_MED;

    BSTD_UNUSED(boxMode);

    if (bmem_size == BMEM_SIZE_LOW)
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 128*1024*1024;
    else
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 258*1024*1024;

    if (bmem_size == BMEM_SIZE_LOW)
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 64*1024 *1024;
    else
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024;

    if (bmem_size == BMEM_SIZE_LOW)
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 64*1024*1024;
    else if (bmem_size == BMEM_SIZE_HIGH)
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 384*1024*1024;
    else
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 128*1024*1024;

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

    switch (g_pPreInitState->boxMode) {
    case 1:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD, Video:10-bit HD main + HD PIP, Transcode:None ***", g_pPreInitState->boxMode));
        break;
    case 2:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD+SD, Video:HD/no PIP, Transcode:None ***", g_pPreInitState->boxMode));
        break;
    case 3:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD, Video:HD/no PIP, Transcode:None ***", g_pPreInitState->boxMode));
        break;
    case 4:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD, Video:Dual HD/no PIP, Transcode:SD/576p(Max) ***", g_pPreInitState->boxMode));
        break;
    case 5:
        BDBG_WRN(("*** 97250 BoxMode %d:Headless, Video:10-bit HD/no PIP, Transcode:720p(Max) ***", g_pPreInitState->boxMode));
        break;
    case 6:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD+SD, Video:10-bit HD/no PIP, Transcode:None ***", g_pPreInitState->boxMode));
        break;
    case 7:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD, Video:HD + 480p PIP, Transcode:None ***", g_pPreInitState->boxMode));
        break;
    case 8:
        BDBG_WRN(("*** 97250 BoxMode %d:Display:UHD+SD, Video:Multi-PIP (2x), Transcode:None ***", g_pPreInitState->boxMode));
        break;
    default:
        BDBG_WRN(("*** 97250 No BoxMode specified!"));
        break;
    }

    if (board)
    {
        BDBG_WRN(("*** Initializing %s Board ... ***", board));
    }
    else
    {
        BDBG_WRN(("*** Initializing a 7250 Board, but no subtype detected ... ***"));
    }
    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
    return;
}
