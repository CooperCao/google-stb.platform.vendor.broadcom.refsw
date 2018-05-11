/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bavc.h"
#include "bkni.h"
#include "bchp_common.h"

BDBG_MODULE( BAVC );

/***************************************************************************
 *
 */
BAVC_MatrixCoefficients  BAVC_GetDefaultMatrixCoefficients_isrsafe(
    BFMT_VideoFmt eDisplayFmt,
    bool          bXvYcc )
{
    return ((BFMT_IS_PAL(eDisplayFmt)  || (BFMT_VideoFmt_e576p_50Hz == eDisplayFmt)) ? BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG :
            (BFMT_IS_NTSC(eDisplayFmt) || BFMT_IS_480P(eDisplayFmt)) ?
            ((bXvYcc) ? BAVC_MatrixCoefficients_eXvYCC_601 : BAVC_MatrixCoefficients_eSmpte_170M) :
            ((bXvYcc) ? BAVC_MatrixCoefficients_eXvYCC_709 : BAVC_MatrixCoefficients_eItu_R_BT_709));
}

/***************************************************************************
 *
 */
BAVC_HDMI_DRM_EOTF BAVC_TransferCharacteristicsToEotf_isrsafe(BAVC_TransferCharacteristics eTransChar, BAVC_TransferCharacteristics ePreferredTransChar)
{
    BAVC_HDMI_DRM_EOTF eEotf = BAVC_HDMI_DRM_EOTF_eMax;

    if ((unsigned)eTransChar == 0) eTransChar = BAVC_TransferCharacteristics_eUnknown;
    switch (ePreferredTransChar)
    {
        case BAVC_TransferCharacteristics_eArib_STD_B67:
            eEotf = BAVC_HDMI_DRM_EOTF_eHLG;
            break;
        default:
            break;
    }

    if (eEotf == BAVC_HDMI_DRM_EOTF_eMax)
    {
        switch (eTransChar)
        {
            case BAVC_TransferCharacteristics_eSmpte_ST_2084:
                eEotf = BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084;
                break;

            case BAVC_TransferCharacteristics_eArib_STD_B67:
                eEotf = BAVC_HDMI_DRM_EOTF_eHLG;
                break;

            default:
                BDBG_WRN(("Unsupported TransChar %d.  Assuming SDR.", eTransChar));
                /* fall through */
            case BAVC_TransferCharacteristics_eUnknown:
            case BAVC_TransferCharacteristics_eItu_R_BT_709:
            case BAVC_TransferCharacteristics_eItu_R_BT_470_2_M:
            case BAVC_TransferCharacteristics_eItu_R_BT_470_2_BG:
            case BAVC_TransferCharacteristics_eSmpte_170M:
            case BAVC_TransferCharacteristics_eSmpte_240M:
            case BAVC_TransferCharacteristics_eLinear:
            case BAVC_TransferCharacteristics_eIec_61966_2_4:
            case BAVC_TransferCharacteristics_eItu_R_BT_2020_10bit:
            case BAVC_TransferCharacteristics_eItu_R_BT_2020_12bit:
                eEotf = BAVC_HDMI_DRM_EOTF_eSDR;
                break;
        }
    }

    return eEotf;
}

void BAVC_GetDefaultStaticHdrMetadata_isrsafe(BAVC_StaticHdrMetadata * pMetadata)
{
    if (pMetadata)
    {
        BKNI_Memset(pMetadata, 0, sizeof(*pMetadata));
        /* -1 means invalid, to match decoder behavior when not present */
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen.ulX = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen.ulY = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue.ulX = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue.ulY = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed.ulX = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed.ulY = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stWhitePoint.ulX = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stColorVolume.stWhitePoint.ulY = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMax = 0xffffffff;
        pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMin = 0xffffffff;
    }
}

void BAVC_ValidateStaticHdrMetadata_isrsafe(
    BAVC_StaticHdrMetadata * pMetadata)
{
    BAVC_Point * points[BAVC_NUM_COLORIMETRY_POINTS];
    unsigned i = 0;

    /* uses GBRW order as originally specified by CEA/SMPTE */
    points[0] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen;
    points[1] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue;
    points[2] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed;
    points[3] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stWhitePoint;

    for (i = 0; i < BAVC_NUM_COLORIMETRY_POINTS; i++)
    {
        if (points[i]->ulX > 50000)
        {
            points[i]->ulX = 0;
        }

        if (points[i]->ulY  > 50000)
        {
            points[i]->ulY  = 0;
        }
    }

    if (pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMax == 0xffffffff)
    {
        pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMax = 0;
    }
    /* max: already in 0.0001 nits */

    if (pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMin == 0xffffffff)
    {
        pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMin = 0;
    }
    /* min: already in 0.0001 nits */
}

void BAVC_DeserializeStaticHdrMetadata_isrsafe(BAVC_StaticHdrMetadata * pMetadata, size_t len, const uint8_t * bytes)
{
    BAVC_Point * apPoints[BAVC_NUM_COLORIMETRY_POINTS];
    unsigned i;

    if (len != BAVC_TYPE1_STATIC_HDR_METADATA_LEN)
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    /* this maps GBRW -> 0123 per original CEA/SMPTE guidelines */
    apPoints[0] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stGreen;
    apPoints[1] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stBlue;
    apPoints[2] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stPrimaries.stRed;
    apPoints[3] = &pMetadata->stMasteringDisplayColorVolume.stColorVolume.stWhitePoint;

    /* Display Primaries and White Point */
    for (i = 0; i < BAVC_NUM_COLORIMETRY_POINTS; i++)
    {
        apPoints[i]->ulX = bytes[i * 4] | ((uint16_t)bytes[1 + i * 4] << 8);
        apPoints[i]->ulY = bytes[2 + i * 4] | ((uint16_t)bytes[3 + i * 4] << 8);
    }

    /* Max Display Mastering Luminance */
    pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMax = bytes[16] | (bytes[17] & 0x00FF) << 8;

    /* Min Display Mastering Luminance */
    pMetadata->stMasteringDisplayColorVolume.stLuminance.uiMin = bytes[18] | (bytes[19] & 0x00FF) << 8;

    /* Max Content Light Level */
    pMetadata->stContentLightLevel.ulMax = bytes[20] | (bytes[21] & 0x00FF) << 8;

    /* Max Frame Average Light Level */
    pMetadata->stContentLightLevel.ulMaxFrameAvg = bytes[22] | (bytes[23] & 0x00FF) << 8;
}

#if ( BCHP_CHIP != 7425 ) && ( BCHP_CHIP != 7435 ) && ( BCHP_CHIP != 7445 ) && ( BCHP_CHIP != 7439 ) && ( BCHP_CHIP != 7366 )
#define BAVC_VCE_MAX_B_FRAMES_INTERLACED 3
#define BAVC_VCE_MAX_BUFFERS_ORIGINAL_INTERLACED 8
#define BAVC_VCE_MAX_BUFFERS_SHIFTED_INTERLACED 8
#define BAVC_VCE_MAX_BUFFERS_DECIMATED_INTERLACED 10

#define BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE 7
#define BAVC_VCE_MAX_BUFFERS_ORIGINAL_PROGRESSIVE 12
#define BAVC_VCE_MAX_BUFFERS_DECIMATED_PROGRESSIVE 14
#endif

void BAVC_VCE_GetDefaultBufferConfig_isrsafe(
      bool bInterlaced,
      BAVC_VCE_BufferConfig *pstBufferConfig
      )
{
   BKNI_Memset( pstBufferConfig, 0, sizeof( *pstBufferConfig ) );
#if ( BCHP_CHIP != 7425 ) && ( BCHP_CHIP != 7435 ) && ( BCHP_CHIP != 7445 ) && ( BCHP_CHIP != 7439 ) && ( BCHP_CHIP != 7366 )
    pstBufferConfig->eScanType = bInterlaced? BAVC_ScanType_eInterlaced : BAVC_ScanType_eProgressive;
    pstBufferConfig->uiNumberOfBFrames = bInterlaced?
        BAVC_VCE_MAX_B_FRAMES_INTERLACED : BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE;
#else
    BSTD_UNUSED(bInterlaced);
#endif
}

unsigned BAVC_VCE_GetRequiredBufferCount_isrsafe(
      const BAVC_VCE_BufferConfig *pstBufferConfig,
      BAVC_VCE_BufferType eBufferType
      )
{
#if ( BCHP_CHIP != 7425 ) && ( BCHP_CHIP != 7435 ) && ( BCHP_CHIP != 7445 ) && ( BCHP_CHIP != 7439 ) && ( BCHP_CHIP != 7366 ) && !defined(BCHP_RAAGA_AX_MISC_REG_START)
   unsigned uiNumberOfBFrames = pstBufferConfig->uiNumberOfBFrames;

   switch ( pstBufferConfig->eScanType )
   {
   case BAVC_ScanType_eInterlaced:
      if ( uiNumberOfBFrames > BAVC_VCE_MAX_B_FRAMES_INTERLACED )
      {
         uiNumberOfBFrames = BAVC_VCE_MAX_B_FRAMES_INTERLACED;
      }
      switch ( eBufferType )
      {
         /* NOTE: +1 buffer to include 1-picture RUL delay for VIPinVDC. */
         /* TODO: it's possible to optimize allocation between chroma field buffers and shifted chroma frame buffers */
         case BAVC_VCE_BufferType_eOriginal: return (BAVC_VCE_MAX_BUFFERS_ORIGINAL_INTERLACED-(BAVC_VCE_MAX_B_FRAMES_INTERLACED-pstBufferConfig->uiNumberOfBFrames))*2+1;
         case BAVC_VCE_BufferType_eDecimated: return (BAVC_VCE_MAX_BUFFERS_DECIMATED_INTERLACED-(BAVC_VCE_MAX_B_FRAMES_INTERLACED-pstBufferConfig->uiNumberOfBFrames))*2+1;
         case BAVC_VCE_BufferType_eShiftedChroma: return (BAVC_VCE_MAX_BUFFERS_SHIFTED_INTERLACED-(BAVC_VCE_MAX_B_FRAMES_INTERLACED-pstBufferConfig->uiNumberOfBFrames))+1;
         default:
            return 0;
      }
      break;

   case BAVC_ScanType_eProgressive:
      if ( uiNumberOfBFrames > BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE )
      {
         uiNumberOfBFrames = BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE;
      }
      switch ( eBufferType )
      {
         /* NOTE: +1 buffer to include 1-picture RUL delay for VIPinVDC. */
         case BAVC_VCE_BufferType_eOriginal: return (BAVC_VCE_MAX_BUFFERS_ORIGINAL_PROGRESSIVE-(BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE-pstBufferConfig->uiNumberOfBFrames))+1;
         case BAVC_VCE_BufferType_eDecimated: return (BAVC_VCE_MAX_BUFFERS_DECIMATED_PROGRESSIVE-(BAVC_VCE_MAX_B_FRAMES_PROGRESSIVE-pstBufferConfig->uiNumberOfBFrames))+1;
         case BAVC_VCE_BufferType_eShiftedChroma: return 0;
         default:
            return 0;
      }
      break;
   }
#else
   BSTD_UNUSED( pstBufferConfig );
   BSTD_UNUSED( eBufferType );
#endif
   /* RAAGA soft encoder uses 8 capture buffers */
#ifdef BCHP_RAAGA_AX_MISC_REG_START
   return 8;
#else
   return 0;
#endif
}


/* End of File */
