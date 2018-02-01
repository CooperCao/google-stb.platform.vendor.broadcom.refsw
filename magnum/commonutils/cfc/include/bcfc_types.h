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
 ******************************************************************************/
#ifndef BCFC_TYPES_H__
#define BCFC_TYPES_H__

#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Overview: ********************************************************
The cfc enums defined here are similar to avc enums in bavc_type.h. But avc enums
are defined to match industrial standards, and cfc enums are defined in a way so
that it could be conviniently used to do lookup internally
****************************************************************************/

/*
Summary:
    Pixel color format type.

Description:

See Also:

*/
typedef enum
{
    BCFC_ColorFormat_eRGB = 0,   /* non-linear */
    BCFC_ColorFormat_eYCbCr,     /* regular YCbCr, i.e. non-constant luminance */
    BCFC_ColorFormat_eYCbCr_CL,  /* constant luminance.  does bt709 have CL combination ??? */
    BCFC_ColorFormat_eLMS,
    BCFC_ColorFormat_eICtCp,
    BCFC_ColorFormat_eMax,
    BCFC_ColorFormat_eInvalid = BCFC_ColorFormat_eMax

} BCFC_ColorFormat;

/*
Summary:
    Pixel colorimetry type.

Description:
    ColorPrimaries defines RGB in term of CIE XYZ, and MatrixCoefficients
    specifies how RGB is converted to YCbCr. Colorimetry is decided by both
    ColorPrimaries and MatrixCoefficients. It is used to lookup matrices
    internally.

See Also:

*/
typedef enum BCFC_Colorimetry
{
    BCFC_Colorimetry_eBt709,
    BCFC_Colorimetry_eSmpte170M,      /* ntsc / pal digital */
    BCFC_Colorimetry_eBt470_BG,       /* pal analog */
    BCFC_Colorimetry_eBt2020,
    BCFC_Colorimetry_eXvYcc601,
    BCFC_Colorimetry_eXvYcc709,       /* matrix is same as BT709 */
    BCFC_Colorimetry_eFcc,            /* only for input */
    BCFC_Colorimetry_eSmpte240M,      /* only for input */
    BCFC_Colorimetry_eMax,
    BCFC_Colorimetry_eInvalid = BCFC_Colorimetry_eMax
} BCFC_Colorimetry;

/*
Summary:
    Pixel color range

Description:
    Used to specify color component range

See Also:

*/
typedef enum BCFC_ColorRange {

    BCFC_ColorRange_eLimited,
    BCFC_ColorRange_eFull,
    BCFC_ColorRange_eMax
} BCFC_ColorRange;

/*
Summary:
    Video transfer function

Description:
    Used to specify transfer function, such as OETF for input, EOTF for display.

See Also:

*/
typedef enum BCFC_ColorTF {
    BCFC_ColorTF_eBt1886,      /* SDR */
    BCFC_ColorTF_eBt2100Pq,    /* i.e. HDR-PQ, or HDR10 */
    BCFC_ColorTF_eHlg,         /* Hybrid Log-Gamma */
    BCFC_ColorTF_eMax
} BCFC_ColorTF;

/*
Summary:
    Defines pixel sample's bit depth per component.

Description:
    This enum is used to report the pixel sample bit depth per component.
    Conventional video codecs support 8-bit per component video sample.
    while h265 video spec also supports 10-bit video.

See Also:
*/
typedef enum BCFC_ColorDepth
{
    BCFC_ColorDepth_e8Bit = 0,
    BCFC_ColorDepth_e10Bit,
    BCFC_ColorDepth_e12Bit,
    BCFC_ColorDepth_e16Bit,
    BCFC_ColorDepth_ePacked16,
    BCFC_ColorDepth_eMax

} BCFC_ColorDepth;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BCFC_TYPES_H__ */
/* End of file. */
