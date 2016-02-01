/***************************************************************************
*     (c)2010-2015 Broadcom Corporation
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
#include "bstd.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_97439);

/*
 7439 BX support only.
*/
static void nexus_p_modifyMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings )
{
#if !NEXUS_HAS_VIDEO_DECODER && !NEXUS_HAS_VIDEO_ENCODER
    BSTD_UNUSED(pRtsSettings);
#endif

#if NEXUS_HAS_VIDEO_DECODER
    switch (pRtsSettings->boxMode)
    {
       case 1: /* Main 4Kp60 10bit HEVC, 4Kp60 8-bit VP9, 4Kp60 8bit AVC*/
            pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
            pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = false;
            break;
        case 2: /* Single Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
                   Dual Decode: 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC per source */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[2].mfdIndex = 3;   /* xcode   1*/
            pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */
            pRtsSettings->videoDecoder[3].mfdIndex = 2;   /* xcode   2 */
            pRtsSettings->videoDecoder[3].avdIndex = 1;   /* HVD 1 */


            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
         case 6:
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  MFD 1*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

            /* No XCODE */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video- HVD0 + Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video HDV0 + Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
          case 3: /* Single Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
               Dual Decode: 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC per source
               Support 9 512x288p30 decodes */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  MFD 1*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

            /* No XCODE */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
         case 4: /* Single Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
                    Dual Transcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 3;   /* xcode   1*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
            pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* xcode   2 */
            pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
         case 5: /* Single Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
                    Dual Transcode Quad multi-PIP*/
            pRtsSettings->videoDecoder[0].mfdIndex = 0;  /* Multi-pip*/
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* Pip */
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
            pRtsSettings->videoDecoder[2].mfdIndex = 3;   /* xcode   0*/
            pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
         case 7: /* Main Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
                    Dual Transcode */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;  /* Multi-pip*/
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* Pip */
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
            pRtsSettings->videoDecoder[2].mfdIndex = 3;   /* xcode   0*/
            pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */
            pRtsSettings->videoDecoder[3].mfdIndex = 2;   /* xcode   2 */
            pRtsSettings->videoDecoder[3].avdIndex = 1;   /* HVD 1 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;

        case 9: /* Main Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC +
                   Dual Transcode */
           pRtsSettings->videoDecoder[0].mfdIndex = 0;  /* Multi-pip*/
           pRtsSettings->videoDecoder[0].avdIndex = 0;
           pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* NO Pip */
           pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 1 */
           pRtsSettings->videoDecoder[2].mfdIndex = 3;   /* xcode   0*/
           pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */
           pRtsSettings->videoDecoder[3].mfdIndex = 2;   /* xcode   2 */
           pRtsSettings->videoDecoder[3].avdIndex = 1;   /* HVD 1 */

           pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
           pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
           pRtsSettings->avd[0].splitBufferHevc = true;
           pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
           pRtsSettings->avd[1].splitBufferHevc = false;
           break;

       case 10: /* Main Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC
                  +1080p60 PIP 3+1 Quad Multi-PIP */
           pRtsSettings->videoDecoder[0].mfdIndex = 0;  /* Multi-pip*/
           pRtsSettings->videoDecoder[0].avdIndex = 0;
           pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* NO Pip */
           pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

           pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
           pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
           pRtsSettings->avd[0].splitBufferHevc = true;
           pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
           pRtsSettings->avd[1].splitBufferHevc = false;
           break;
        case 12: /*  Decode: 3840x2160p50 10-bit HEVC, 3840x2160p50 8-bit VP9, or 1920x1080p50 8-bit AVC + PIP
            Dual Decode: 1920x1080p25/50i 8-bit HEVC / VP9 / AVC per source
            1080i50 10 bit + 576i50 Multi-PIP  mode */
         pRtsSettings->videoDecoder[0].mfdIndex = 0;
         pRtsSettings->videoDecoder[0].avdIndex = 0;
         pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  MFD 1*/
         pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

         /* No XCODE */

         pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
         pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
         pRtsSettings->avd[0].splitBufferHevc = true;
         pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
         pRtsSettings->avd[1].splitBufferHevc = false;
         break;

        case 13:
           pRtsSettings->videoDecoder[0].mfdIndex = 0;
           pRtsSettings->videoDecoder[0].avdIndex = 0;

           /* No XCODE */
           pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
           pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
           pRtsSettings->avd[0].splitBufferHevc = false;
           break;
       case 14: /* Single Decode: 3840x2160p60 10-bit HEVC, 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
               Dual Decode: 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC per source
               Support 9 512x288p30 decodes */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  MFD 1*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

            /* No XCODE */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[1].splitBufferHevc = false;

            break;
       case 16: /* Single Decode: 3840x2160p60 10-bit HEVC, */
         pRtsSettings->videoDecoder[0].mfdIndex = 0;
         pRtsSettings->videoDecoder[0].avdIndex = 0;
         pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* xcode 0 */
         pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

         pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
         pRtsSettings->avd[0].splitBufferHevc = true;
         pRtsSettings->avd[1].memcIndex = 0;           /* 3rd Decoder  */
         pRtsSettings->avd[1].splitBufferHevc = false;
         break;

       case 17: /* Triple Decode: 31080p60 10-bit HEVC / 1080p30/60i 8-bit AVC
                 , 3840x2160p60 8-bit VP9, or 1920x1080p60 8-bit AVC + PIP
               Dual Decode: 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC per source
               Support 9 512x288p30 decodes */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  PIP*/
            pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */
            pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* xcode 0 */
            pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].splitBufferHevc = false;
            pRtsSettings->avd[1].memcIndex = 0;           /* 3rd Decoder  */
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;

      case 18: /* dual DDR DDR3-2133 31080p60 10-bit HEVC and 1080p60 8-bit AVC/MPEG2
                 , 3840x2160p25 8-bit VP9,  +
             1920x1080p60/60i 10-bit HEVC / 8-bit VP9/AVC/MPEG2*/
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  second decode*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma  */
            pRtsSettings->avd[0].splitBufferHevc = true;
            pRtsSettings->avd[1].memcIndex = 0;
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
     case 19: /*  Single DDR 2160p25 10-bit HEVC / 1080p25/650i 8-bit AVC/MPEG2
                 , 3840x2160p25 8-bit VP9,  +
            Dual Decode: 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC per source */
            pRtsSettings->videoDecoder[0].mfdIndex = 0;
            pRtsSettings->videoDecoder[0].avdIndex = 0;
            pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  second decode*/
            pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 0 */

            pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
            pRtsSettings->avd[0].splitBufferHevc = false;
            pRtsSettings->avd[1].memcIndex = 0;
            pRtsSettings->avd[1].splitBufferHevc = false;
            break;
    case 20: /*  Single DDR 2160p50 10-bit HEV /2160p50 8-bit VP9/ 1080p50 8-bit AVC/MPEG2.
                 second decode 352x288p50 10bit HEVC or 8bit AVC/VP9/MPEG2 */
           pRtsSettings->videoDecoder[0].mfdIndex = 0;
           pRtsSettings->videoDecoder[0].avdIndex = 0;
           pRtsSettings->videoDecoder[1].mfdIndex = 1;   /*  second decode*/
           pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 0 */

           pRtsSettings->avd[0].memcIndex = 0;           /* main video, Luma  */
           pRtsSettings->avd[0].splitBufferHevc = false;
           pRtsSettings->avd[1].memcIndex = 0;
           pRtsSettings->avd[1].splitBufferHevc = false;
           break;
    default:
       BDBG_ERR((" NEED TO SET CORRECT BOX MODE %d",pRtsSettings->boxMode ));
       break;


    }
#endif /* NEXUS_HAS_VIDEO_DECODER */
#if NEXUS_HAS_VIDEO_ENCODER /* to be completed later*/
    switch (pRtsSettings->boxMode)
    {
        default:
        case 1:
        case 3:
        case 6:
        case 10:
        case 12:
        case 13:
        case 14:
        case 20:
            /* NO ENCODER ALLOWED */
            break;
        case 2: /* 7439 box mode 2 & 4 */
        case 4:
        case 7:
        case 9:
        case 18:
            pRtsSettings->vce[0].memcIndex     = 0;
            pRtsSettings->vce[1].memcIndex     = 0;
            break;
        case 5:
        case 16:
        case 17:
        case 19:
            pRtsSettings->vce[0].memcIndex     = 0;
            break;
    }
#endif /* NEXUS_HAS_VIDEO_ENCODER */
}

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    unsigned boxMode = g_pPreInitState->boxMode;
    unsigned i;
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#if BCHP_VER >= BCHP_VER_B0
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eVp9] = true;
#endif
    }
    /* enable MVC 3D for decoder 0 */
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;

    switch (boxMode)
    {
        default:
       case 1:
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080p60 HEVC */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].used = false;
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;
            break;
        case 2: /* both decoders are 1080p60 HEVC 10 bit capable or single 4K decode, user must disable second decoder */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;

           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p30hz;
           pSettings->videoDecoder[1].colorDepth = 8; /* 8 bit 1080p30 HEVC */

           /* for transcondig decode 1920x1080p30 / 60i 8-bit HEVC / VP9 / AVC*/
           pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p30hz;
           pSettings->videoDecoder[2].colorDepth = 8; /* 8 bit 1080p30  */
           pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080p30hz;
           pSettings->videoDecoder[3].colorDepth = 8; /* 8 bit 1080p30 */
           break;
        case 6:
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p30hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit 1080p30/1080i60 HEVC 8 bit VP9/AVC*/
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;
           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           /* NO Transcoding*/
           break;
        case 3: /* both decoders are 1080p60 HEVC 10 bit capable or single 4K decode, user must disable second decoder */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p30hz;
           pSettings->videoDecoder[1].colorDepth = 8; /* 8 bit 1080i60 HEVC or 9 512x288p30 decodes */
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;
           /* NO Transcoding*/
           break;
        case 4: /* Single UHD 10 bit capable with Dual transcode */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;

           /* for transcondig decode 1920x1080p50 8-bit HEVC / 8 bit VP9 /  1080p25/50i 8 bit AVC/MPEG */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p50hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit Only for HEVC */
           pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p50hz;
           pSettings->videoDecoder[2].colorDepth = 10; /* 10 bit only for HEVC */
           break;
        case 5: /* Single UHD 10 bit capable , Multi Pip, Pip and with single Dtranscode */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit 1080p30 */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p30hz;

           /* for transcondig decode 1920x1080p30 10-bit HEVC / 8 bit VP9 */
           pSettings->videoDecoder[2].colorDepth = 10; /* 10 bit Only for HEVC */
           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           pSettings->videoDecoder[3].used = false;
           break;
        case 7: /* Main UHD 10 bit capable , Pip, Dual  transcode */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p50hz;
           pSettings->videoDecoder[1].colorDepth = 8; /* 8 bit 1080p25 */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p25hz;

           /* for transcondig decode 1920x1080p25 8-bit HEVC / 8 bit VP9/AVC/MPEG2 */
           pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p25hz;
           pSettings->videoDecoder[2].colorDepth = 8; /* 8 bit */
           pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080p25hz;
           pSettings->videoDecoder[3].colorDepth = 8; /* 8 bit */
           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           break;
       case 9: /* Main UHD 10 bit capable , Dual  transcode */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p50hz;
           pSettings->videoDecoder[1].used = false; /* no PIP */

           /* for transcondig decode 1920x1080p50 10-bit HEVC / 1080p25 8 bit VP9/AVC/MPEG2 */
           pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p50hz;
           pSettings->videoDecoder[2].colorDepth = 10; /* 8 bit */
           pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080p50hz;
           pSettings->videoDecoder[3].colorDepth = 10; /* 8 bit */
           break;
       case 10: /* Main UHD 10 bit capable , Pip, Quad-PIP */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit 1080p60 */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;

           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 576;
           pSettings->videoDecoder[0].mosaic.maxWidth = 720;
           break;
       case 12: /* Main UHD 10 bit capable , Pip, Quad-PIP */
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p50hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* max 10 bit HEVC 1080p60 */
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p50hz;
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;

           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           break;
       case 13:
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p50hz;
           pSettings->videoDecoder[1].used = false;
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;

           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           break;
        case 14:
           pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4k HEVC/VP9 */
           pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
           pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
           pSettings->videoDecoder[1].colorDepth = 10; /* Decode 1- 10 bit 1080p60 HEVC 8 bit VP9/AVC*/
           pSettings->videoDecoder[2].used = false;
           pSettings->videoDecoder[3].used = false;
           pSettings->videoDecoder[0].mosaic.maxNumber = 3;
           pSettings->videoDecoder[0].mosaic.maxHeight = 1088;
           pSettings->videoDecoder[0].mosaic.maxWidth = 1920;
           /* NO Transcoding*/
           break;
        case 16:
          pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080p60 HEVC/VP9 */
          pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
          pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz; /* xcode */
          pSettings->videoDecoder[1].colorDepth = 10; /* Decode 1- 10 bit 1080p60 HEVC 8 bit VP9/AVC*/
          pSettings->videoDecoder[2].used = false;
          pSettings->videoDecoder[3].used = false;
          /*1 x Transcode decode 1080p60 */
          break;
       case 17:
          pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080p60 HEVC/VP9 */
          pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
          pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
          pSettings->videoDecoder[1].colorDepth = 10; /* Decode 1- 10 bit 1080p60 HEVC 8 bit VP9/AVC*/

          pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
          pSettings->videoDecoder[2].colorDepth = 10; /* Decode 2- 10 bit 1080p60 HEVC 8 bit VP9/AVC*/
          pSettings->videoDecoder[3].used = false;
          /*1 x Transcode*/
          break;
       case 18:
          pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit  HEVC, 8 bit VP9/AVC/MPEG2 */
          pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
          pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
          pSettings->videoDecoder[1].colorDepth = 10; /* 10 bit  HEVC, 8 bit VP9/AVC/MPEG2 */

          pSettings->videoDecoder[2].used = false;
          pSettings->videoDecoder[3].used = false;
          /*  2 x Transcode*/
          break;
       case 19:
          pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 4Kp25 HEVC/VP9, 2nd 8bit 1080p25 */
          pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p25hz;
          pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p25hz;

          pSettings->videoDecoder[2].used = false;
          pSettings->videoDecoder[3].used = false;
          /*  1 x Transcode*/
          break;
      case 20:
         pSettings->videoDecoder[0].colorDepth = 10; /* 10 bit 1080i50 HEVC/VP9 */
         pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p50hz;
         pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e288p50hz;
         pSettings->videoDecoder[1].colorDepth = 10; /* 352x288p50 10 bit HEVC */

         pSettings->videoDecoder[2].used = false;
         pSettings->videoDecoder[3].used = false;
         break;
    }

#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].colorDepth = 10;
    pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
    /* this is needed since Atlas tries to use the first iframe of 4K stream as thumb-nail */
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eVp9] = true;
#endif /* NEXUS_NUM_STILL_DECODES */
#else
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
    pOps->modifyDefaultMemoryRtsSettings = nexus_p_modifyMemoryRtsSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 142*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 26*1024*1024;

    switch(boxMode)
    {
         default:
         /* 1 MEMC */
         case 1:
            /* use the rest of available memc0 main heap memory */
            if (g_platformMemory.memc[0].length > 1 * 1024 * 1024)
             pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 192*1024*1024; /* for trellis usage */
            /* If the platform does not have more then 1GB then customer has to tweak these values
               to conform with their 3D app usage */
         case 12:
         case 13:
         case 17:
         case 19:
         case 20:
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 124*1024*1024;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
           /* All boxmodes below have 2 memc's*/
         case 2:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 10:
         case 14:
         case 16:
         case 18:
            /* GFD 0, 1,2,3 */
           pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 0;
           /* Fall through */
         case 3: /* Cases that use default MEMC0 Graphics heap for GFD1 */
           pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256*1024*1024; /* primary graphics heap and secondary graphics frame buff */
           pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
           break;
    }

    return;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
   const char *board = NULL;
   /* TODO: some day this needs to become run-time vs. compile time. read product ID */
    board = "7439 B0 Based ";

#if defined NEXUS_USE_7252S_VMS_SFF
    board = "7252S VMS SFF";
#elif defined NEXUS_USE_3390_VMS
	board = "93390 VMS";
#elif defined NEXUS_USE_7439_SFF
    board = "SFF board";
#elif defined NEXUS_USE_7439_SV
    board = "SV board";
#elif defined NEXUS_USE_7252S_SAT
    board = "SAT board";
#endif

    if (board)
    {
       BDBG_WRN(("*** Initializing %s Board ...***", board));
    }
    else
    {
       BDBG_WRN(("*** Initializing a 7439 B0 Board, but not subtype detected ...***"));
    }

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
