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
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#include "bchp_memc_ddr_2.h"
#include "bchp_clkgen.h"
#include "bchp_memc_arb_0.h"
#include "bchp_memc_arb_1.h"

BDBG_MODULE(nexus_platform_97445);

static void nexus_p_modifyMemoryRtsSettings(NEXUS_MemoryRtsSettings *pRtsSettings )
{
    BSTD_UNUSED(pRtsSettings);
#if NEXUS_HAS_VIDEO_DECODER
    switch (pRtsSettings->boxMode) {

    /**** For 7445 ****/
    default:
    case 1:
    case 12:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 5;   /* encode 0 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[3].mfdIndex = 4;   /* encode 1 */
        pRtsSettings->videoDecoder[3].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma for 4K and  */
        pRtsSettings->avd[0].secondaryMemcIndex = 1;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 1;           /* pip */
        pRtsSettings->avd[2].memcIndex = 0;           /* encode 0, encode 1 */
        break;
    case 3:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 3;   /* encode 0 */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 4;   /* encode 1 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[3].mfdIndex = 5;   /* encode 2 */
        pRtsSettings->videoDecoder[3].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma for 4K and  */
        pRtsSettings->avd[0].secondaryMemcIndex = 1;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 0;           /* encode 0 */
        pRtsSettings->avd[2].memcIndex = 0;           /* encode 1, encode 2 */
        break;
     case 7:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* encode 0  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* encode 1 */
        pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* encode 2 */
        pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[3].mfdIndex = 3;   /* encode 3 */
        pRtsSettings->videoDecoder[3].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[4].mfdIndex = 4;   /* encode 4  */
        pRtsSettings->videoDecoder[4].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[5].mfdIndex = 5;   /* encode 5 */
        pRtsSettings->videoDecoder[5].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 0;           /* encode 0,1, Luma */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* encode 0,1, Chroma */
        pRtsSettings->avd[0].splitBufferHevc = false;
        pRtsSettings->avd[1].memcIndex = 1;           /* encode 2,3 */
        pRtsSettings->avd[2].memcIndex = 2;           /* encode 4,5 */
        break;
     case 8:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* decode 0 /encode 0  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* decode 1/encode 1 */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* decode 2/encode 2 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* decode 0 /encode 0, Luma */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* decode 0 /encode 0, Chroma */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 1;           /* decode 1/encode 1  */
        pRtsSettings->avd[2].memcIndex = 1;           /* decode 2/encode 2 */
        break;
     case 9:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* 4K/Multi-PIP triple 1080i/single 1080i  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* Multi-PIP triple 1080i /Main or PIP  */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* 1080p30 xcode1 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[3].mfdIndex = 3;   /* 1080p30 xcode2 */
        pRtsSettings->videoDecoder[3].avdIndex = 2;   /* HVD 2 */

        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma for 4K and  */
        pRtsSettings->avd[0].secondaryMemcIndex = 1;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 2;           /* pip: Up to 1080i@60 decode */
        pRtsSettings->avd[2].memcIndex = 1;           /* Dual 1080i xcode */
        break;
     case 13:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip   */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* encode 0 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma for 4K and  */
        pRtsSettings->avd[0].secondaryMemcIndex = 1;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 1;           /* pip */
        pRtsSettings->avd[2].memcIndex = 0;           /* encode 0 */
        break;
     case 14:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main/decode 0 */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* decode 1 */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* decode 2 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[3].mfdIndex = 3;   /* decode 3 */
        pRtsSettings->videoDecoder[3].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma  */
        pRtsSettings->avd[0].secondaryMemcIndex = 2;  /* main video, Chroma   */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 2;           /* decode 2 */
        pRtsSettings->avd[2].memcIndex = 2;           /* encode 2, encode 3 */
        break;
    case 15:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* Decode 0 */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 0;   /* Decode 1 - MFD is unused. Must be linked to decode 0. */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 1;   /* Decode 2 */
        pRtsSettings->videoDecoder[2].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 1;           /* Decode 0 video */
        pRtsSettings->avd[1].memcIndex = 1;           /* Decode 1 video */
        pRtsSettings->avd[2].memcIndex = 1;           /* Decode 2, video */
        break;
    case 1000:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto HEVC 4Kp@60 */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 3;   /* encode 0 */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* encode 1 */
        pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[3].mfdIndex = 4;   /* encode 3 */
        pRtsSettings->videoDecoder[3].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->videoDecoder[4].mfdIndex = 5;   /* encode 4 */
        pRtsSettings->videoDecoder[4].avdIndex = 2;   /* HVD 2 */
        pRtsSettings->avd[0].memcIndex = 2;           /* main video, Luma for 4K   */
        pRtsSettings->avd[0].secondaryMemcIndex = 1;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 0;           /* 1st and 2nd transcode */
        pRtsSettings->avd[2].memcIndex = 0;           /* 3rd and 4th transcode */
        break;

     /**** For 7252/7448/7449 ****/
    case 2:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto 4K@60 10 bit  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip upto 1080p60  */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

        pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma for 4K   */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 0;           /* pip */
        break;
    case 4:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto 4K@50 10 bit  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* SHVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip upto 1080p50  */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 2;   /* transcode upto 720p25  */
        pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */

        pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma for 4K   */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 0;           /* pip & transcode */
        break;
    case 5:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto 4K@60 10 bit  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* SHVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 2;   /* transcode upto 720p30  */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */

        pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma for 4K   */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        pRtsSettings->avd[1].memcIndex = 0;           /* transcode */
        break;
    case 6:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto HEVC 1080p@60 or AVC1080i60 */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 2;   /* 1st transcode upto 720p60  */
        pRtsSettings->videoDecoder[1].avdIndex = 1;   /* HVD 1 */
        pRtsSettings->videoDecoder[2].mfdIndex = 3;   /* 2nd transcode upto 720p60  */
        pRtsSettings->videoDecoder[2].avdIndex = 1;   /* HVD 1 */

        pRtsSettings->avd[0].memcIndex = 1;           /* main video*/
        pRtsSettings->avd[0].splitBufferHevc = false;
        pRtsSettings->avd[1].memcIndex = 0;           /* transcode */
        break;
    case 10:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto 4K@60 10 bit  */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->videoDecoder[1].mfdIndex = 1;   /* pip upto 1080p60  */
        pRtsSettings->videoDecoder[1].avdIndex = 0;   /* HVD 0 */

        pRtsSettings->avd[0].memcIndex = 1;           /* main video, Luma for 4K   */
        pRtsSettings->avd[0].secondaryMemcIndex = 0;  /* main video, Chroma for 4K  */
        pRtsSettings->avd[0].splitBufferHevc = true;
        break;
    case 1001:
        pRtsSettings->videoDecoder[0].mfdIndex = 0;   /* main, upto HEVC 1080p@60 or AVC1080p60 */
        pRtsSettings->videoDecoder[0].avdIndex = 0;   /* HVD 0 */
        pRtsSettings->avd[0].memcIndex = 0;           /* main video*/
        pRtsSettings->avd[0].splitBufferHevc = false;
        break;

    }
#endif
}

static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
    unsigned boxMode = g_pPreInitState->boxMode;
    unsigned i;
#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#if BCHP_VER >= BCHP_VER_E0
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eVp9] = true;
#endif
    }
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
    switch (boxMode) {

    /**** For 7445 ****/
    default:
    case 1:
    case 12:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        break;
    case 3:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080i;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080i;
        pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080i;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        break;
    case 7:
        for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
            pSettings->videoDecoder[i].colorDepth = 10;
            pSettings->videoDecoder[i].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        }
        break;
    case 8:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[1].colorDepth = 10;
		pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[2].colorDepth = 10;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 9:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[1].colorDepth = 8;
        pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p30hz;
        pSettings->videoDecoder[2].colorDepth = 8;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p30hz;
        pSettings->videoDecoder[3].colorDepth = 8;
        pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080p30hz;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        pSettings->videoDecoder[1].mosaic.maxNumber =3;
        pSettings->videoDecoder[1].mosaic.maxWidth =1920;
        pSettings->videoDecoder[1].mosaic.maxHeight=1088;
        pSettings->videoDecoder[1].mosaic.colorDepth = 8;
        break;
    case 13:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
		pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        pSettings->videoDecoder[1].colorDepth = 8;
        pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[2].colorDepth = 8;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
	case 14:
        pSettings->videoDecoder[0].colorDepth = 8;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
		pSettings->videoDecoder[1].colorDepth = 8;
		pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
		pSettings->videoDecoder[2].colorDepth = 8;
		pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p30hz;
		pSettings->videoDecoder[3].colorDepth = 8;
		pSettings->videoDecoder[3].maxFormat = NEXUS_VideoFormat_e1080p30hz;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 15:
        pSettings->videoDecoder[0].colorDepth = 8;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[1].colorDepth = 8;
        pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[2].colorDepth = 8;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[0].mosaic.maxNumber = 1;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[1].mosaic.maxNumber = 1;
        pSettings->videoDecoder[1].mosaic.maxWidth =1920;
        pSettings->videoDecoder[1].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 1000:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        pSettings->videoDecoder[5].used = false;
        break;

    /**** For 7252/7448/7449 ****/
    case 2:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        pSettings->videoDecoder[2].used = false;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 4:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 8;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 5:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[0].mosaic.maxNumber =3;
        pSettings->videoDecoder[0].mosaic.maxWidth =1920;
        pSettings->videoDecoder[0].mosaic.maxHeight=1088;
        pSettings->videoDecoder[0].mosaic.colorDepth = 10;
        pSettings->videoDecoder[2].used = false;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 6:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[0].used = true;
        pSettings->videoDecoder[1].colorDepth = 10;
        pSettings->videoDecoder[1].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[1].used = true;
        pSettings->videoDecoder[2].colorDepth = 10;
        pSettings->videoDecoder[2].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[2].used = true;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 10:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
        pSettings->videoDecoder[2].used = false;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;
    case 1001:
        pSettings->videoDecoder[0].colorDepth = 10;
        pSettings->videoDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
        pSettings->videoDecoder[0].used = true;
        pSettings->videoDecoder[1].used = false;
        pSettings->videoDecoder[2].used = false;
        pSettings->videoDecoder[3].used = false;
        pSettings->videoDecoder[4].used = false;
        pSettings->videoDecoder[5].used = false;
        break;

    }
#if NEXUS_NUM_STILL_DECODES
    pSettings->stillDecoder[0].used = true;
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;

    if (boxMode==1 || boxMode==3 || boxMode==9 || boxMode==12 || boxMode==13 || boxMode==1000 || boxMode==2 || boxMode==4 || boxMode==5 || boxMode==10) {
        pSettings->stillDecoder[0].colorDepth = 10;
        pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
    }
    else if (boxMode==6 || boxMode==1001){
                pSettings->stillDecoder[0].colorDepth = 10;
				pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
    }
    else if (boxMode==15){
                pSettings->stillDecoder[0].colorDepth = 8;
				pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
    }
	else if(boxMode==14) {
				pSettings->stillDecoder[0].colorDepth = 8;
				pSettings->stillDecoder[0].maxFormat = NEXUS_VideoFormat_e1080p60hz;
	}
    else { /*boxMode 7 and boxMode 8 (Headless)*/
        pSettings->stillDecoder[0].used = false;
    }
#endif /* NEXUS_NUM_STILL_DECODES */
#endif /* NEXUS_HAS_VIDEO_DECODER */
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
    pOps->modifyDefaultMemoryRtsSettings = nexus_p_modifyMemoryRtsSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    switch(boxMode)
    {
    /**** For 7445 ****/
    case 1:
    case 3:
    case 7:
    case 8:
    case 9:
    case 12:
    case 13:
    case 14:
    case 15:
    case 1000:
    default:
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size =  208*1024*1024; /*decoder FW+general,xpt playback,audio other general purpose */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 168*1024 *1024; /* CABACs(28)for 3 decoders + RAVE CDB(6+15) */
        pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = (0x00400000*2);
        pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].size = 5*1024*1024;  /* RDC heap plus margin for possible VCE fw/debug/output descriptors heap */
        break;

    /**** For 7252/7448/7449 ****/
    case 2:
    case 4:
    case 5:
    case 6:
    case 10:
    case 1001:
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size =  192*1024*1024; /*decoder FW+general,xpt playback,audio other general purpose */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 124*1024 *1024; /* CABACs(28)for 3 decoders + RAVE CDB(6+15) */
        pSettings->heap[NEXUS_SAGE_SECURE_HEAP].memcIndex = 1;
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 4*1024*1024;  /* RDC heap plus margin */
        break;
    }

    switch(boxMode)
    {
    /**** For 7445 ****/
    case 3:
    case 12:
    case 1000:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 512*1024*1024; /*gfd 0 on memc 1 */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 16*1024*1024;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            break;
    case 7:
    case 8:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 16*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 256*1024*1024; /* gfd 0 on memc 2*/
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
	case 9:
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 512*1024*1024; /*gfd 0 on memc 2 */
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 16*1024*1024; /*gfd 1 on memc 1 */
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            break;
    case 14:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 200*1024*1024; /*gfd 0/4/5/6 on memc 0 */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 15:
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 512*1024*1024; /*gfd 0 on memc 2 */
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
	case 13:
    default:
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 0;
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].size = 512*1024*1024; /* gfd on memc 2*/
            pSettings->heap[NEXUS_MEMC2_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;

    /**** For 7252/7448/7449 ****/
    case 2:
    case 4:
    case 6:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 164*1024*1024; /*192+124+148 heap should be < 528 */
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 384*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 5:
    case 10:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 0;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 512*1024*1024;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
    case 1001:
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 160*1024*1024;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 0;
            break;
    }

    if(boxMode == 7) {
        pSettings->heap[NEXUS_MEMC2_DRIVER_HEAP].size = 16*1024*1024;
    }

    if(boxMode == 10) {
        pSettings->heap[NEXUS_MEMC1_DRIVER_HEAP].size = 0;  /* Saving 4M by making RDC heap plus margin to 0 as RDC takes memory from MEMC0_MAIN_HEAP and no VCE descriptors needed for box mode #10 */
    }
    return;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    switch (g_pPreInitState->boxMode) {

    /**** For 7445 ****/
    default:
    case 1:
            BDBG_WRN(("*** 97445 BoxMode 1:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080p60->720p30(Max)***"));
        break;
    case 3:
            BDBG_WRN(("*** 97445 BoxMode 3:Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Triple up to 1080p30(Max)***"));
        break;
    case 7:
            BDBG_WRN(("*** 97445 BoxMode 7:Display:None, Video:no Main/no PIP Headless,Transcode:Six up to 720p30(Max)***"));
        break;
    case 8:
            BDBG_WRN(("*** 97445 BoxMode 8:Display:None, Video:no Main/no PIP Headless,Transcode:Triple up to 1080p30(Max)***"));
        break;
    case 9:
            BDBG_WRN(("*** 97445 BoxMode 9:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual up to 1080p30(Max)***"));
        break;
    case 12:
            BDBG_WRN(("*** 97445 BoxMode 12:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:Dual 1080i60->720p30(Max)***"));
        break;
    case 13:
            BDBG_WRN(("*** 97445 BoxMode 13:Display:UHD/SD, Video:UHD Main/HD PIP,Transcode:single up to 1080p60***"));
        break;
	case 14:
            BDBG_WRN(("*** 97445 BoxMode 14:Display:HD Output, Video:HD Main/no PIP,Transcode:Triple (One up to 1080p60 (Max) and Dual up to 1080p30(Max))***"));
        break;
    case 15:
            BDBG_WRN(("*** 97445 BoxMode 15:Display:UHD Output, Video:Dual HD main/HD PIP,Transcode:Single HDMI input transcoding up to 1080p60(Max)***"));
        break;
    case 1000:
            BDBG_WRN(("*** 97445 TEMP BoxMode 1000:Display:UHD/SD, Video:UHD Main/no PIP,Transcode:Quad up to 1080p30(Max)***"));
        break;

    /**** For 7252/7448/7449 ****/
    case 2:
#if NEXUS_USE_7449_VMS_SFF
            BDBG_WRN(("*** 97449D0 BoxMode 2:Display:UHD/SD, Video:UHD Main/HD PIP***"));
#else
            BDBG_WRN(("*** 97252D0 BoxMode 2:Display:UHD/SD, Video:UHD Main/HD PIP***"));
#endif
        break;
    case 4:
            BDBG_WRN(("*** 97252D0 BoxMode 4:Display:UHD/SD, Video:UHD Main/HD PIP, Transcode:1080i50->720p25(Max)***"));
        break;
    case 5:
            BDBG_WRN(("*** 97252D0 BoxMode 5:Display:UHD/SD, Video:UHD Main/HD NO-PIP, Transcode:1080i60->720p30(Max)***"));
        break;
    case 6:
            BDBG_WRN(("*** 97252D0 BoxMode 6:Display:UHD/SD, Video:HD Main/HD NO-PIP, Dual Transcode:1080p60->720p60(Max)***"));
        break;
    case 10:
            BDBG_WRN(("*** 97252D0 BoxMode 10:Display:UHD/SD, Video:UHD Main/(HD Main + HD PIP) ***"));
        break;
    case 1001:
            BDBG_WRN(("*** 97252D0 Temp BoxMode 1001:Display:UHD/SD, Video:HD Main/HD NO-PIP,NO Transcode ***"));
        break;

    }

/**** For 7445 ****/
#if NEXUS_USE_7445_SV
    BDBG_WRN(("*** Initializing 7445 SV Board ...***"));
#endif
#if NEXUS_USE_7445_VMS_SFF
    BDBG_WRN(("***Initializing 7445 VMS_SFF Board ...***"));
#endif
#if NEXUS_USE_7445_C
    BDBG_WRN(("***Initializing 7445 C Board ...***"));
#endif
#if NEXUS_USE_7445_DBS
    BDBG_WRN(("***Initializing 7445 DBS Board ...***"));
#endif
#if NEXUS_USE_7445_LCC
    BDBG_WRN(("***Initializing 7445 LCC Board ...***"));
#endif
#if NEXUS_USE_7445_AUTO
    BDBG_WRN(("***Initializing 7445 AUTO Board ...***"));
#endif


/**** For 7252/7448/7449 ****/
#if NEXUS_USE_7252_SV
    BDBG_WRN(("*** Initializing 7252 SV Board ...***"));
#elif NEXUS_USE_7252_VMS_SFF
    BDBG_WRN(("***Initializing 7252 VMS_SFF Board ...***"));
#elif NEXUS_USE_7252_C
    BDBG_WRN(("***Initializing 7252 C Board ...***"));
#elif NEXUS_USE_7252_LCC
    BDBG_WRN(("***Initializing 7252 LCC Board ...***"));
#elif NEXUS_USE_7449_VMS_SFF
    BDBG_WRN(("*** Initializing 7449 VMS_SFF Board ...***"));
#endif

    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
