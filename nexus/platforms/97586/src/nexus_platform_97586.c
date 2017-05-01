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
#include "bchp_clkgen.h"

BDBG_MODULE(nexus_platform_97586);

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
    switch (boxMode)
    {
       default:
       case 1:
           /* Main + PiP */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;

            pSettings->videoDecoder[1].colorDepth = 10;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p;
            break;
       case 2:
           /* Main only, No PiP */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            break;
       case 3:
           /* Main only @ 8 bit, No PiP */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            break;
       case 4:
           /* Main + Xcode */
            pSettings->videoDecoder[0].colorDepth = 8;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;

            pSettings->videoDecoder[1].colorDepth = 8;
            pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p;
            break;
       case 5:
           /* Headless, 1 xcode only */
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
            break;
       case 6:
           /* Main only, No PiP (similar to case 2)*/
            pSettings->videoDecoder[0].colorDepth = 10;
            pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p;
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

#if(NEXUS_HAS_VIDEO_ENCODER)
    switch (boxMode)
    {
       default:
       case 1: /* no xcoding allowed */
            for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
            {
                pSettings->videoEncoder[i].used = false;
                pSettings->videoEncoder[i].maxWidth  = 0;
                pSettings->videoEncoder[i].maxHeight = 0;
                pSettings->videoEncoder[i].interlaced = false;
            }
            break;
       case 2: /* xcoding not allowed in boxmode 2 */
           for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
           {
               pSettings->videoEncoder[i].used = false;
               pSettings->videoEncoder[i].maxWidth  = 640;
               pSettings->videoEncoder[i].maxHeight = 480;
               pSettings->videoEncoder[i].interlaced = false;
           }
           break;
       case 3: /* xcoding not allowed in boxmode 3 */
           for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
           {
               pSettings->videoEncoder[i].used = false;
               pSettings->videoEncoder[i].maxWidth  = 640;
               pSettings->videoEncoder[i].maxHeight = 480;
               pSettings->videoEncoder[i].interlaced = false;
           }
           break;
       case 4: /* xcoding allowed up to 576p */
           for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
           {
               /* 480p30 or 576p25 encode by DSP */
               pSettings->videoEncoder[i].used = (i < 1);
               pSettings->videoEncoder[i].maxWidth  = 720;
               pSettings->videoEncoder[i].maxHeight = 576;
               pSettings->videoEncoder[i].interlaced = false;
           }
           break;
       case 5: /* xcoding allowed up to 720p*/
           for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
           {
               /* 720p30 encode by DSP */
               pSettings->videoEncoder[i].used = (i < 1);
               pSettings->videoEncoder[i].maxWidth  = 1280;
               pSettings->videoEncoder[i].maxHeight = 720;
               pSettings->videoEncoder[i].interlaced = false;
           }
           break;
       case 6: /* xcoding not allowed in boxmode 6 */
           for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++)
           {
               pSettings->videoEncoder[i].used = false;
               pSettings->videoEncoder[i].maxWidth  = 640;
               pSettings->videoEncoder[i].maxHeight = 480;
               pSettings->videoEncoder[i].interlaced = false;
           }
           break;

    }
#endif /* (NEXUS_HAS_VIDEO_ENCODER) */
#if NEXUS_HAS_DISPLAY
    switch(boxMode)
    {
       default:
       case 1: /* upscaled to 4Kp60 */
           pSettings->display[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           break;
       case 2: /* UHD on display 0, 480i on display 1 */
           pSettings->display[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->display[1].maxFormat = NEXUS_VideoFormat_eNtsc;
           break;
       case 3: /* upscaled to 4Kp60, no SD output */
           pSettings->display[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           break;
       case 4: /* upscaled to 4Kp60, no SD output  + Xcode has its own window */
           pSettings->display[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           break;
       case 5: /* headless so no displays */
           pSettings->display[0].window[0].used = false;
           pSettings->display[0].window[1].used = false;
           pSettings->display[1].window[0].used = false;
           pSettings->display[1].window[1].used = false;
           break;
       case 6: /* UHD on display 0, 480i on display 1  No Pip */
           pSettings->display[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->display[1].maxFormat = NEXUS_VideoFormat_eNtsc;
           pSettings->display[0].window[1].used = false;
           pSettings->display[1].window[1].used = false;
           break;
    }
#endif /* NEXUS_HAS_DISPLAY */
}


void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
    pOps->modifyDefaultMemoryRtsSettings = nexus_p_modifyMemoryRtsSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    unsigned managed = NEXUS_MEMORY_TYPE_MANAGED;
    unsigned not_mapped = NEXUS_MEMORY_TYPE_NOT_MAPPED;
#if BMMA_USE_STUB
    managed = 0;
    not_mapped = 0;
#endif

    BSTD_UNUSED(boxMode);

    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
#if(NEXUS_USE_73649_SFF)
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 128*1024*1024;;
#else
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 208*1024*1024;
#endif
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 0; /* TBD once kernel is up */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* dynamically calculated */
    pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = managed | not_mapped;

    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memcIndex = 0;
#if (NEXUS_USE_73649_SFF)
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].subIndex = 0;
#else
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].subIndex = 1;
#endif
#if (NEXUS_USE_73649_SFF)
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 0*1024*1024; /* force shared non-secure video for 512 MB combo */
#else
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */
#endif
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eSecure;

    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
#if(NEXUS_USE_73649_SFF)
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 88*1024*1024; /* on screen graphics for primary display */
#else
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1; /* TBD */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 128*1024*1024; /* on screen graphics for primary display */
#endif
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication;

#if 0 /* Disable till VICE or transcode has use case */
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].subIndex = 0; /* TBD */
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size = (0x00400000);
    pSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].memoryType = managed | NEXUS_MEMORY_TYPE_DRIVER_UNCACHED|NEXUS_MEMORY_TYPE_DRIVER_CACHED|NEXUS_MEMORY_TYPE_APPLICATION_CACHED;
#endif
}

NEXUS_Error NEXUS_Platform_P_GetFramebufferHeapIndex(unsigned displayIndex, unsigned *pHeapIndex)
{
    unsigned boxMode = g_pPreInitState->boxMode;
    switch (boxMode) {
    default:
    case 1:
        switch (displayIndex)
        {
        case NEXUS_OFFSCREEN_SURFACE:
        case NEXUS_SECONDARY_OFFSCREEN_SURFACE:
        case 0: /* HD Display GFD0 */
            *pHeapIndex = NEXUS_MEMC0_GRAPHICS_HEAP;
            break;
        case 1: /* SD Display */
            *pHeapIndex = NEXUS_MEMC0_GRAPHICS_HEAP;
            break;
        default:
            BDBG_ERR(("Invalid display index %d",displayIndex));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
    }
    return 0;
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

#if NEXUS_USE_7586_SV
    board = "7586 SV";
#endif

    if (board)
    {
       BDBG_WRN(("*** Initializing %s Board ...***", board));
    }
    else
    {
       BDBG_WRN(("*** Initializing a 7586 Board, but not subtype detected ...***"));
    }

    NEXUS_Platform_P_InitRaaga();

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
    return;
}
