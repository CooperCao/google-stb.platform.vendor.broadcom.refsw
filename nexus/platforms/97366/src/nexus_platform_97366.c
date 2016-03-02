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
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_97366);

static void nexus_p_modifyMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    switch (pRtsSettings->boxMode)
    {
        default:
        case 1: /* Main + PiP @ 1080p60 with HEVC*/
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* xcode   */
            pRtsSettings->videoDecoder[2].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            break;
        case 2: /* 7366 Box mode 2 - Single Main no Pip 2 encode*/
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;
            pRtsSettings->videoDecoder[1].avdIndex = 0;
            pRtsSettings->videoDecoder[2].mfdIndex = 2;
            pRtsSettings->videoDecoder[2].avdIndex = 0;

            /* PiP is out but we have both encoders on MEMC0 */
            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            break;
        case 3: /* 7366 Box mode 3 - single 4K decode no pip */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            /* Box mode 3.1 - allow pip when not decoding 4Kx2K */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

            /* Box mode 3 reverse video capture MEMC assignment */
            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma for 4K and  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma for 4K  */
            pRtsSettings->avd[0].splitBufferHevc = true;
            break;
        case 4: /* 7366 Box mode 4 - Dual transcode @ 1080p30/i60 8-bit AVC, NO DISPLAY */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* xcode 0 mfd 1 */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* xcode 1 mfd 2 */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            break;
        case 5: /* 7336 Box mode 5 - Single 4k or Main + Pip limited to 1080p30 AVC or p60 HEVC */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            /* Allow pip only when not decoding 4K */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 1;            /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;   /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            break;
        case 6: /* 7366 Box mode 6 - Dual transcode @ 1080p30/i60 8-bit AVC, NO DISPLAY  + 32-bit DDR3-1866 */
           pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* xcode 0 mfd 1 */
           pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
           pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* xcode 1 mfd 2 */
           pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

           pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
           pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
           pRtsSettings->avd[0].splitBufferHevc = true;
           break;


    }
#else
    BSTD_UNUSED(pRtsSettings);
#endif /* NEXUS_HAS_VIDEO_DECODER */
}

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
    unsigned boxMode = g_pPreInitState->boxMode;
    unsigned i;
#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++)
    {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    }
    pSettings->videoDecoder[2].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;

    switch (boxMode)
    {
        default:
        case 1:
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            pSettings->videoDecoder[2].colorDepth = 10;
            pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            break;
        case 2:
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            pSettings->videoDecoder[2].colorDepth = 10;
            pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            break;
        case 3: /* 7366B0  4k decode only mode*/
            pSettings->videoDecoder[0].colorDepth = 10; /* TBD: how to diff between AVD and HEVC decoders */
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
            /* box-mode 3.1 allows for PIP @ 1080p60*/
            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
            pSettings->videoDecoder[2].used = false;
            break;
        case 4: /* both decoders are 1080p60 HEVC 10 bit capable or single 4K decode, user must disable second decoder */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080p60 HEVC */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit 1080p60 HEVC */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
           pSettings->videoDecoder[2].used = false;
           break;
        case 5: /* 7366B0  4k decode + Main & pip with high temp refresh DDR 2133 MHz */
           pSettings->videoDecoder[0].colorDepth = 10;
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           /* box-mode 5 could allows for PIP @ 1080p60 when not decoding 4K */
           pSettings->videoDecoder[1].colorDepth = 10;
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
           pSettings->videoDecoder[2].used = false;
           break;
        case 6: /* Single decode at 4K in decoder 0, or dual decode max at 1080p60 10 bit but not at the same time */
          pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080p60 HEVC */
          pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
          pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit 1080p60 HEVC */
          pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
          pSettings->videoDecoder[2].used = false;
          break;
    }

#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
    /* this is needed since Atlas tries to use the first iframe of 4K stream as thumb-nail */
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#endif /* NEXUS_NUM_STILL_DECODES */

#else
    BSTD_UNUSED(boxMode);
    BSTD_UNUSED(i);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
    pOps->modifyDefaultMemoryRtsSettings = nexus_p_modifyMemoryRtsSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
#if defined NEXUS_USE_7399_SFF || defined NEXUS_USE_7399_SV
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 192*1024*1024;
#else
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 208*1024*1024;
#endif

    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */

#if defined NEXUS_USE_7399_SFF || defined NEXUS_USE_7399_SV
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 64*1024*1024;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
#else
    switch(boxMode)
    {
       case 6:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 16*1024*1024; /* on screen graphics for primary display */

            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256*1024*1024; /* primary graphics heap and secondary graphics frame buff */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;

       case 5:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 1; /* TODO */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 16*1024*1024; /* on screen graphics for primary display */

            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256*1024*1024; /* primary graphics heap and secondary graphics frame buff */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;

       default: /* all other cases we end up with memory controller 1 for GFD0 and GFD1*/
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 16*1024*1024; /* on screen graphics for primary display */

            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256*1024*1024; /* primary graphics heap and secondary graphics frame buff */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    }

    pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = (0x00400000);
    pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memcIndex = 1;
#endif
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
   const char *board = NULL;
   /* some day this needs to become run-time vs. compile time */
#if NEXUS_USE_7366_SV
    board = "7366 SV";
#elif NEXUS_USE_7366_SFF
    board = "7366 SFF";
#elif NEXUS_USE_7399_SV
    board = "7399 SV";
#elif NEXUS_USE_7399_SFF
    board = "7399 SFF";
#endif

    if (board)
    {
       BDBG_WRN(("*** Initializing %s Board ...***", board));
    }
    else
    {
       BDBG_WRN(("*** Initializing a 7366 Board, but not subtype detected ...***"));
    }

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}


