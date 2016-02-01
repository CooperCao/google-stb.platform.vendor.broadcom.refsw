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
#include "bchp_clkgen.h"

BDBG_MODULE(nexus_platform_97364);

static void nexus_p_modifyMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    switch (pRtsSettings->boxMode)
    {
        default:
        case 1: /* Main + PiP @ 1080p30 or i60 with 10 bit HEVC, No SD or Xcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
        case 2: /* Main @ 1080p30 or i60 with 10 bit HEVC, With SD enabled, No Xcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
        case 3: /* Main @ 1080p60 8 bit HEVC, With SD disabled, No Xcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
        case 4: /* Main @1080p30 or i60 with 10 bit HEVC, No SD  No Pip, one Xcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* xcode */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
        case 5: /* Single decoder@ 1080p60 10 bit HEVC, Xcode and Headless */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* xcode  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
       case 6: /* Main @ 1080p60 10 bit HEVC, With SD enabled, No Xcode (2166 MHz DDR3)*/
           pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
           pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */

           pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
           pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
           pRtsSettings->avd[0].splitBufferHevc = false;
           break;

    }
#endif /* NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_HAS_VIDEO_ENCODER /* since no VICE no need */
    switch (pRtsSettings->boxMode)
    {
        default:
        case 1: /* TBD  */
            break;
    }
#endif /* NEXUS_HAS_VIDEO_ENCODER */
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
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
    /* based on SW7445-3032 JIRA 1080p30 only decode should default to NEXUS_VideoFromat_e1080i not p */
    switch (boxMode)
    {
       default:
       case 1:
           /* Main + PiP */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;

            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080i;
            break;
       case 2:
           /* Main only, No PiP */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;
            pSettings->videoDecoder[1].used = false;
            break;
       case 3:
           /* Main only @ 8 bit, No PiP */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[1].used = false;
            break;
       case 4:
           /* Main + Xcode */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080i;

            pSettings->videoDecoder[1].colorDepth = 8;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080i;
            break;
       case 5:
           /* Headless, 1 xcode only */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[1].used = false;
            break;
       case 6:
           /* Main only, No PiP (similar to case 2)*/
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[1].used = false;
            break;

    }

#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
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


