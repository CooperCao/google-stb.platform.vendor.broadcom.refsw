/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bavc.h"

BDBG_MODULE( BAVC );

/***************************************************************************
 *
 */
/* NTSC-J for Japan */
#define BAVC_P_VIDEO_FORMAT_IS_NTSC_J(fmt) \
	((BFMT_VideoFmt_eNTSC_J == (fmt)) || (BFMT_VideoFmt_e720x482_NTSC_J == (fmt)))

/* NTSC-M for North America */
#define BAVC_P_VIDEO_FORMAT_IS_NTSC_M(fmt) \
	((BFMT_VideoFmt_eNTSC == (fmt)) || (BFMT_VideoFmt_e720x482_NTSC == (fmt)) || \
	 (BFMT_VideoFmt_eNTSC_443 == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_NTSC(fmt) \
	(BAVC_P_VIDEO_FORMAT_IS_NTSC_J(fmt) || BAVC_P_VIDEO_FORMAT_IS_NTSC_M(fmt))

#define BAVC_P_VIDEO_FORMAT_IS_480P(fmt) \
	((BFMT_VideoFmt_e480p == (fmt)) || (BFMT_VideoFmt_e720x483p == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_PAL_BB1D1G(fmt) \
	((BFMT_VideoFmt_ePAL_B == (fmt)) || (BFMT_VideoFmt_ePAL_B1 == (fmt)) || \
	(BFMT_VideoFmt_ePAL_D1 == (fmt)) || (BFMT_VideoFmt_ePAL_G == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_PAL(fmt) \
	(BAVC_P_VIDEO_FORMAT_IS_PAL_BB1D1G(fmt) || (BFMT_VideoFmt_ePAL_H == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_I == (fmt)) || (BFMT_VideoFmt_ePAL_D == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_K == (fmt)) || (BFMT_VideoFmt_ePAL_M == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_N == (fmt)) || (BFMT_VideoFmt_ePAL_NC == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_60 == (fmt)))

#define BAVC_P_DEFAULT_MATRIX_COEFF(fmt, xvycc) \
	( (BAVC_P_VIDEO_FORMAT_IS_NTSC(fmt) || BAVC_P_VIDEO_FORMAT_IS_480P(fmt) || \
	   BAVC_P_VIDEO_FORMAT_IS_PAL(fmt)  || (BFMT_VideoFmt_e576p_50Hz == fmt)) ? \
		  ((xvycc) ? BAVC_MatrixCoefficients_eXvYCC_601 : BAVC_MatrixCoefficients_eSmpte_170M) : \
		  ((xvycc) ? BAVC_MatrixCoefficients_eXvYCC_709 : BAVC_MatrixCoefficients_eItu_R_BT_709) )

BAVC_MatrixCoefficients  BAVC_GetDefaultMatrixCoefficients_isrsafe(
	BFMT_VideoFmt eDisplayFmt,
	bool          bXvYcc )
{
	return BAVC_P_DEFAULT_MATRIX_COEFF(eDisplayFmt, bXvYcc);
}

/***************************************************************************
 *
 */
BAVC_HDMI_DRM_EOTF BAVC_TransferCharacteristicsToEotf_isrsafe(BAVC_TransferCharacteristics eTransChar)
{
    BAVC_HDMI_DRM_EOTF eEotf = BAVC_HDMI_DRM_EOTF_eSDR;

    switch (eTransChar)
    {
        case BAVC_TransferCharacteristics_eSmpte_ST_2084:
            eEotf = BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084;
            break;

        case BAVC_TransferCharacteristics_eArib_STD_B67:
            eEotf = BAVC_HDMI_DRM_EOTF_eFuture;
            break;

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

        default:
            BDBG_WRN(("Unsupported EOTF.  Assuming SDR."));
            break;
    }

    return eEotf;
}


/* End of File */

