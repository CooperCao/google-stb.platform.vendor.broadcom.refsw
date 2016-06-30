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
BAVC_MatrixCoefficients  BAVC_GetDefaultMatrixCoefficients_isrsafe(
    BFMT_VideoFmt eDisplayFmt,
    bool          bXvYcc )
{
    return ((BFMT_IS_NTSC(eDisplayFmt) || BFMT_IS_480P(eDisplayFmt) ||
             BFMT_IS_PAL(eDisplayFmt) || (BFMT_VideoFmt_e576p_50Hz == eDisplayFmt)) ?
            ((bXvYcc) ? BAVC_MatrixCoefficients_eXvYCC_601 : BAVC_MatrixCoefficients_eSmpte_170M) :
            ((bXvYcc) ? BAVC_MatrixCoefficients_eXvYCC_709 : BAVC_MatrixCoefficients_eItu_R_BT_709));
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
            BDBG_MSG(("Unsupported TransChar %d.  Assuming SDR.", eTransChar));
            break;
    }

    return eEotf;
}


/* End of File */

