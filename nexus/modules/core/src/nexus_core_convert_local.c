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
*
* API Description:
*
***************************************************************************/

#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_memory.h"
#include "priv/nexus_core_convert_local.h"

BDBG_MODULE(nexus_core_convert_local);

static const struct {
    NEXUS_VideoCodec codec;
    BAVC_VideoCompressionStd std;
} g_Nexus_VideoCodecs[] = {
    {NEXUS_VideoCodec_eH263, BAVC_VideoCompressionStd_eH263},
    {NEXUS_VideoCodec_eH264, BAVC_VideoCompressionStd_eH264},
    {NEXUS_VideoCodec_eH264_Mvc, BAVC_VideoCompressionStd_eMVC},
    {NEXUS_VideoCodec_eH264_Svc, BAVC_VideoCompressionStd_eSVC},
    {NEXUS_VideoCodec_eVc1, BAVC_VideoCompressionStd_eVC1},
    {NEXUS_VideoCodec_eVc1SimpleMain, BAVC_VideoCompressionStd_eVC1SimpleMain},
    {NEXUS_VideoCodec_eMpeg1, BAVC_VideoCompressionStd_eMPEG1},
    {NEXUS_VideoCodec_eMpeg4Part2,BAVC_VideoCompressionStd_eMPEG4Part2},
    {NEXUS_VideoCodec_eDivx311, BAVC_VideoCompressionStd_eMPEG4Part2},
    {NEXUS_VideoCodec_eAvs, BAVC_VideoCompressionStd_eAVS},
    {NEXUS_VideoCodec_eRv40, BAVC_VideoCompressionStd_eRV9}, /* Codec ID used by Real is RV40, Real Video 9 is just a version of software first introduced implementation of RV40 video codec */
    {NEXUS_VideoCodec_eVp6, BAVC_VideoCompressionStd_eVP6},  /* Codec most often used in Flash Video */
    {NEXUS_VideoCodec_eVp7, BAVC_VideoCompressionStd_eVP7},
    {NEXUS_VideoCodec_eVp8, BAVC_VideoCompressionStd_eVP8},  /* WebM video codec */
    {NEXUS_VideoCodec_eVp9, BAVC_VideoCompressionStd_eVP9},  /* WebM video codec */
    {NEXUS_VideoCodec_eMotionJpeg, BAVC_VideoCompressionStd_eMOTION_JPEG},
    {NEXUS_VideoCodec_eSpark, BAVC_VideoCompressionStd_eSPARK},  /* Another codec for Flash Video */
    {NEXUS_VideoCodec_eMpeg2, BAVC_VideoCompressionStd_eMPEG2},
    {NEXUS_VideoCodec_eMpeg2, BAVC_VideoCompressionStd_eMPEG2DTV},
    {NEXUS_VideoCodec_eMpeg2, BAVC_VideoCompressionStd_eMPEG2_DSS_PES},
    {NEXUS_VideoCodec_eH265, BAVC_VideoCompressionStd_eH265}
};

BAVC_VideoCompressionStd
NEXUS_P_VideoCodec_ToMagnum(NEXUS_VideoCodec codec, NEXUS_TransportType transportType)
{
    if(codec!=NEXUS_VideoCodec_eMpeg2) {
        unsigned i;
        for(i=0;i<sizeof(g_Nexus_VideoCodecs)/sizeof(*g_Nexus_VideoCodecs);i++) {
            if(g_Nexus_VideoCodecs[i].codec == codec) {
                return g_Nexus_VideoCodecs[i].std;
            }
        }
    }
    /* MPEG2 varieties */
    switch(transportType) {
    case NEXUS_TransportType_eDssEs:
        return BAVC_VideoCompressionStd_eMPEG2DTV;
    case NEXUS_TransportType_eDssPes:
        return BAVC_VideoCompressionStd_eMPEG2_DSS_PES;
    default:
        return BAVC_VideoCompressionStd_eMPEG2;
    }
}

NEXUS_VideoCodec
NEXUS_P_VideoCodec_FromMagnum(BAVC_VideoCompressionStd std)
{
    unsigned i;
    for(i=0;i<sizeof(g_Nexus_VideoCodecs)/sizeof(*g_Nexus_VideoCodecs);i++) {
        if(g_Nexus_VideoCodecs[i].std == std) {
            return g_Nexus_VideoCodecs[i].codec;
        }
    }
    BDBG_WRN(("unsupported BAVC_VideoCompressionStd:%u", std));
    return NEXUS_VideoCodec_eUnknown;
}

BAVC_AudioCompressionStd NEXUS_P_AudioCodec_ToMagnum(NEXUS_AudioCodec codec)
{
    switch ( codec )
    {
    case NEXUS_AudioCodec_eMpeg:
        /* Really, L1,L2,L3 are equivalent */
        return BAVC_AudioCompressionStd_eMpegL2;
    case NEXUS_AudioCodec_eMp3:
        return BAVC_AudioCompressionStd_eMpegL3;
    case NEXUS_AudioCodec_eAc3:
        return BAVC_AudioCompressionStd_eAc3;
    case NEXUS_AudioCodec_eAc3Plus:
        return BAVC_AudioCompressionStd_eAc3Plus;
    case NEXUS_AudioCodec_eAacAdts:
        return BAVC_AudioCompressionStd_eAacAdts;
    case NEXUS_AudioCodec_eAacLoas:
        return BAVC_AudioCompressionStd_eAacLoas;
    case NEXUS_AudioCodec_eAacPlusLoas:
        return BAVC_AudioCompressionStd_eAacPlusLoas;
    case NEXUS_AudioCodec_eAacPlusAdts:
        return BAVC_AudioCompressionStd_eAacPlusAdts;
    case NEXUS_AudioCodec_eDts:
        return BAVC_AudioCompressionStd_eDts;
    case NEXUS_AudioCodec_eDtsHd:
        return BAVC_AudioCompressionStd_eDtsHd;
    case NEXUS_AudioCodec_eDtsCd:
        return BAVC_AudioCompressionStd_eDtsCd;
    case NEXUS_AudioCodec_eDtsExpress:
        return BAVC_AudioCompressionStd_eDtsExpress;
    case NEXUS_AudioCodec_eWmaStd:
        return BAVC_AudioCompressionStd_eWmaStd;
    case NEXUS_AudioCodec_eWmaStdTs:
        return BAVC_AudioCompressionStd_eWmaStdTs;
    case NEXUS_AudioCodec_eWmaPro:
        return BAVC_AudioCompressionStd_eWmaPro;
    case NEXUS_AudioCodec_ePcm:
        return BAVC_AudioCompressionStd_ePcm;
    case NEXUS_AudioCodec_ePcmWav:
        return BAVC_AudioCompressionStd_ePcmWav;
    case NEXUS_AudioCodec_eLpcmDvd:
        return BAVC_AudioCompressionStd_eLpcmDvd;
    case NEXUS_AudioCodec_eLpcmHdDvd:
        return BAVC_AudioCompressionStd_eLpcmHdDvd;
    case NEXUS_AudioCodec_eLpcmBluRay:
        return BAVC_AudioCompressionStd_eLpcmBd;
    case NEXUS_AudioCodec_eAmrNb:
        return BAVC_AudioCompressionStd_eAmrNb;
    case NEXUS_AudioCodec_eAmrWb:
        return BAVC_AudioCompressionStd_eAmrWb;
    case NEXUS_AudioCodec_eDra:
        return BAVC_AudioCompressionStd_eDra;
    case NEXUS_AudioCodec_eCook:
        return BAVC_AudioCompressionStd_eCook;
    case NEXUS_AudioCodec_eAdpcm:
        return BAVC_AudioCompressionStd_eAdpcm;
    case NEXUS_AudioCodec_eVorbis:
        return BAVC_AudioCompressionStd_eVorbis;
    case NEXUS_AudioCodec_eLpcm1394:
        return BAVC_AudioCompressionStd_eLpcm1394;
    case NEXUS_AudioCodec_eG711:
        return BAVC_AudioCompressionStd_eG711;
    case NEXUS_AudioCodec_eG723_1:
        return BAVC_AudioCompressionStd_eG723_1;
    case NEXUS_AudioCodec_eG726:
        return BAVC_AudioCompressionStd_eG726;
    case NEXUS_AudioCodec_eG729:
        return BAVC_AudioCompressionStd_eG729;
    case NEXUS_AudioCodec_eFlac:
        return BAVC_AudioCompressionStd_eFlac;
    case NEXUS_AudioCodec_eMlp:
        return BAVC_AudioCompressionStd_eMlp;
    case NEXUS_AudioCodec_eApe:
        return BAVC_AudioCompressionStd_eApe;
    case NEXUS_AudioCodec_eIlbc:
        return BAVC_AudioCompressionStd_eIlbc;
    case NEXUS_AudioCodec_eIsac:
        return BAVC_AudioCompressionStd_eIsac;
    case NEXUS_AudioCodec_eOpus:
        return BAVC_AudioCompressionStd_eOpus;
    case NEXUS_AudioCodec_eAls:
        return BAVC_AudioCompressionStd_eAls;
    case NEXUS_AudioCodec_eAc4:
        return BAVC_AudioCompressionStd_eAc4;
    default:
        return BAVC_AudioCompressionStd_eMax;
    }
}

NEXUS_AudioCodec NEXUS_P_AudioCodec_FromMagnum(BAVC_AudioCompressionStd std)
{
    switch ( std )
    {
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:
        return NEXUS_AudioCodec_eMpeg;
    case BAVC_AudioCompressionStd_eMpegL3:
        return NEXUS_AudioCodec_eMp3;
    case BAVC_AudioCompressionStd_eAc3:
        return NEXUS_AudioCodec_eAc3;
    case BAVC_AudioCompressionStd_eAc3Plus:
        return NEXUS_AudioCodec_eAc3Plus;
    case BAVC_AudioCompressionStd_eAacAdts:
        return NEXUS_AudioCodec_eAacAdts;
    case BAVC_AudioCompressionStd_eAacLoas:
        return NEXUS_AudioCodec_eAacLoas;
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        return NEXUS_AudioCodec_eAacPlusLoas;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        return NEXUS_AudioCodec_eAacPlusAdts;
    case BAVC_AudioCompressionStd_eDts:
        return NEXUS_AudioCodec_eDts;
    case BAVC_AudioCompressionStd_eDtshd:
        return NEXUS_AudioCodec_eDtsHd;
    case BAVC_AudioCompressionStd_eDtsLegacy:
        return NEXUS_AudioCodec_eDtsLegacy;
    case BAVC_AudioCompressionStd_eWmaStd:
        return NEXUS_AudioCodec_eWmaStd;
    case BAVC_AudioCompressionStd_eWmaStdTs:
        return NEXUS_AudioCodec_eWmaStdTs;
    case BAVC_AudioCompressionStd_eWmaPro:
        return NEXUS_AudioCodec_eWmaPro;
    case BAVC_AudioCompressionStd_ePcm:
        return NEXUS_AudioCodec_ePcm;
    case BAVC_AudioCompressionStd_ePcmWav:
        return NEXUS_AudioCodec_ePcmWav;
    case BAVC_AudioCompressionStd_eLpcmDvd:
        return NEXUS_AudioCodec_eLpcmDvd;
    case BAVC_AudioCompressionStd_eLpcmHdDvd:
        return NEXUS_AudioCodec_eLpcmHdDvd;
    case BAVC_AudioCompressionStd_eLpcmBd:
        return NEXUS_AudioCodec_eLpcmBluRay;
    case BAVC_AudioCompressionStd_eAmr:
        return NEXUS_AudioCodec_eAmr;
    case BAVC_AudioCompressionStd_eDra:
        return NEXUS_AudioCodec_eDra;
    case BAVC_AudioCompressionStd_eCook:
        return NEXUS_AudioCodec_eCook;
    case BAVC_AudioCompressionStd_eAdpcm:
        return NEXUS_AudioCodec_eAdpcm;
    case BAVC_AudioCompressionStd_eVorbis:
        return NEXUS_AudioCodec_eVorbis;
    case BAVC_AudioCompressionStd_eLpcm1394:
        return NEXUS_AudioCodec_eLpcm1394;
    case BAVC_AudioCompressionStd_eG711:
        return NEXUS_AudioCodec_eG711;
    case BAVC_AudioCompressionStd_eG723_1:
        return NEXUS_AudioCodec_eG723_1;
    case BAVC_AudioCompressionStd_eG726:
        return NEXUS_AudioCodec_eG726;
    case BAVC_AudioCompressionStd_eG729:
        return NEXUS_AudioCodec_eG729;
    case BAVC_AudioCompressionStd_eFlac:
        return NEXUS_AudioCodec_eFlac;
    case BAVC_AudioCompressionStd_eMlp:
        return NEXUS_AudioCodec_eMlp;
    case BAVC_AudioCompressionStd_eApe:
        return NEXUS_AudioCodec_eApe;
    case BAVC_AudioCompressionStd_eIlbc:
        return NEXUS_AudioCodec_eIlbc;
    case BAVC_AudioCompressionStd_eIsac:
        return NEXUS_AudioCodec_eIsac;
    case BAVC_AudioCompressionStd_eOpus:
        return NEXUS_AudioCodec_eOpus;
    case BAVC_AudioCompressionStd_eAls:
        return NEXUS_AudioCodec_eAls;
    case BAVC_AudioCompressionStd_eAc4:
        return NEXUS_AudioCodec_eAc4;
    default:
        return NEXUS_AudioCodec_eMax;
    }
}

void NEXUS_Heap_ToString(const NEXUS_MemoryStatus *pStatus, char *buf, unsigned buf_size )
{
#ifdef BDBG_DEBUG_BUILD /* used in BDBG_ERR and BDBG_MSG */
    unsigned n = 0;
    if (pStatus->heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) {
        n+= BKNI_Snprintf(buf, buf_size, "PICBUF%u", pStatus->memcIndex);
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_MAIN) {
        n+= BKNI_Snprintf(buf, buf_size, "MAIN");
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION) {
        n+= BKNI_Snprintf(buf, buf_size, "SAGE");
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION) {
        n+= BKNI_Snprintf(buf, buf_size, "CRR");
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_EXPORT_REGION) {
        n+= BKNI_Snprintf(buf, buf_size, "XRR");
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_GRAPHICS) {
        n+= BKNI_Snprintf(buf, buf_size, "GFX");
    }
    else if (pStatus->heapType & NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS) {
        n+= BKNI_Snprintf(buf, buf_size, "2ND_GFX");
    }
    else {
        n+= BKNI_Snprintf(buf, buf_size, "DRIVER%u", pStatus->memcIndex);
    }
    if (pStatus->memoryType & NEXUS_MEMORY_TYPE_SECURE && buf_size > n) {
        if (pStatus->memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF) {
           n+= BKNI_Snprintf(&buf[n], buf_size-n, " (SECURE, RUNTIME: OPEN)");
        } else {
           n+= BKNI_Snprintf(&buf[n], buf_size-n, " (SECURE)");
        }
    }
#else /* BDBG_DEBUG_BUILD */
    BSTD_UNUSED(pStatus);
    BSTD_UNUSED(buf);
    BSTD_UNUSED(buf_size);
#endif
}
