/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/

#ifndef BAVC_TYPES_H__
#define BAVC_TYPES_H__

/* This is the only dependency */
#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    These defines describe the scratch registers that hold the current
    settings of the VBI Encoder control registers.

Description:
    Define scratch registers to coordinate programming of VBI encoder
    control registers between BVBI and BVDC. BDC_Open would reset those
    registers with the default setting, i.e. only VBI pass-through is
    enabled; the following BVBI_Encode_ApplyChanges would update the
    settings by read/modify/write those scratch registers; the following
    vec vsync isr would read back the scratch setting and build RUL
    to reprogram the corresponding VBI_ENC_xxx_Control register;

See Also:
    BVDC_Open
    BVBI_Encode_ApplyChanges
    BAVC_VbiPath
***************************************************************************/
/* this is the AVC wrapper of VBI_ENC_656_Ancil_Control register */
#ifdef BCHP_RDC_scratch_i_ARRAY_BASE
#define BAVC_VBI_ENC_BP_CTRL_SCRATCH     \
    (BCHP_RDC_scratch_i_ARRAY_BASE + BCHP_RDC_scratch_i_ARRAY_END * sizeof(uint32_t))
#define BAVC_VBI_ENC_0_CTRL_SCRATCH      \
    (BCHP_RDC_scratch_i_ARRAY_BASE + (BCHP_RDC_scratch_i_ARRAY_END - 1) * sizeof(uint32_t))
#define BAVC_VBI_ENC_1_CTRL_SCRATCH      \
    (BCHP_RDC_scratch_i_ARRAY_BASE + (BCHP_RDC_scratch_i_ARRAY_END - 2) * sizeof(uint32_t))
#define BAVC_VBI_ENC_2_CTRL_SCRATCH      \
    (BCHP_RDC_scratch_i_ARRAY_BASE + (BCHP_RDC_scratch_i_ARRAY_END - 3) * sizeof(uint32_t))
#endif

/***************************************************************************
Summary:
    This definition defines the maximum number of mosaic rectangless in a window.

Description:
    This is for video window mosaic mode usage only.  The XVD/MVD and XPT also
    needs this macro to allocate approriate number of channels.

See Also:
    BVDC_Window_SetMosaic, BVDC_Window_GetMosaicDstRects
***************************************************************************/
#define BAVC_MOSAIC_MAX                      (12)

/***************************************************************************
Summary:
    This enumeration represents the available video input source.

Description:
    BAVC_SourceId is an enumeration which represents the source devices to
    VDC module and VBI. It is needed by BVDC_Source_Create to create a
    source handle, and BVBI_Decode_Create.

    There are two major type of sources, the video source and graphics
    source. For example, Mpeg feeder, vdec and ITU-R-656 input are video
    sources, and graphics feeder is graphics source.

    Right after the pixel data is input to VDC module, it is automatically
    expanded to a 4:4:4:4 AYCrCb or ARGB format with 8 bits per component.
    Then the expanded color components are further processed in the VDC
    module.

    After some intra picture processing, pixels from several source could
    could be finally blended together in a compositor. Before the blending
    is performed, color space conversion would be done automatically to
    pixels from each source to match the main source.

    And before the color space conversion, a video input could involve
    dimension scaling / filtering, luma keying intra picture processing,
    and a graphics input could involve a series of operations in the order
    of color key, alpha pre-multiplying, horizontal up scaling, and gamma
    correction.

    The number of different sources (eMpeg0, eVdec0, and etc) might not be
    available on all chipset.  This will be chip specific configurations.
    Calling BVDC_Source_Create will return an error if the source is not
    available on that particular chip.

    There are two graphics feeders in the 7038 chip. Each of them fetches
    pixel data from the video memory and sends to the video compositors. A
    variety of YCrCb, ARGB and CLUT pixel formats are supported for both SD
    and HD applications. Please refer to BPXL module for the complete list.

    The eVfdn sources are used for graphics sources only. There are
    limitations to its use. The corresponding CAP block cannot be used
    because only 1 VFD can be used in the video path. Note that the use of
    CAP requires the use of its corresponding VFD. Only 1 window can be used
    with a VFD. All current window assignments with a particular VFD will
    remain. For example, VFD_0 will be used with the CMP0_V0 window.

See Also:
    BVDC_Source_Create, BVDC_Source_Destroy, BVDC_Source_Handle
***************************************************************************/
typedef enum BAVC_SourceId
{
    BAVC_SourceId_eMpeg0 = 0,        /* Mpeg feeder 0 */
    BAVC_SourceId_eMpeg1,            /* Mpeg feeder 1 */
    BAVC_SourceId_eMpeg2,            /* Mpeg feeder 2 */
    BAVC_SourceId_eMpeg3,            /* Mpeg feeder 3 */
    BAVC_SourceId_eMpeg4,            /* Mpeg feeder 4 */
    BAVC_SourceId_eMpeg5,            /* Mpeg feeder 5 */
    BAVC_SourceId_eMpegMax = BAVC_SourceId_eMpeg5,
    BAVC_SourceId_eVdec0,            /* Analog video source from vdec0 */
    BAVC_SourceId_eVdec1,            /* Analog video source from vdec1 */
    BAVC_SourceId_e656In0,           /* ITU-R-656 video source.0 */
    BAVC_SourceId_e656In1,           /* ITU-R-656 video source 1 */
    BAVC_SourceId_eGfx0,             /* Gfx feeder 0 */
    BAVC_SourceId_eGfx1,             /* Gfx feeder 1 */
    BAVC_SourceId_eGfx2,             /* Gfx feeder 2 */
    BAVC_SourceId_eGfx3,             /* Gfx feeder 3 */
    BAVC_SourceId_eGfx4,             /* Gfx feeder 4 */
    BAVC_SourceId_eGfx5,             /* Gfx feeder 5 */
    BAVC_SourceId_eGfx6,             /* Gfx feeder 6 */
    BAVC_SourceId_eGfxMax = BAVC_SourceId_eGfx6,
    BAVC_SourceId_eHdDvi0,           /* HD DVI Input 0 */
    BAVC_SourceId_eHdDvi1,           /* HD DVI Input 1 */
    BAVC_SourceId_eDs0,              /* DownStream Input 0 */
    BAVC_SourceId_eVfd0,             /* Video feeder 0 */
    BAVC_SourceId_eVfd1,             /* Video feeder 1 */
    BAVC_SourceId_eVfd2,             /* Video feeder 2 */
    BAVC_SourceId_eVfd3,             /* Video feeder 3 */
    BAVC_SourceId_eVfd4,             /* Video feeder 4 */
    BAVC_SourceId_eVfd5,             /* Video feeder 5 */
    BAVC_SourceId_eVfd6,             /* Video feeder 6 */
    BAVC_SourceId_eVfd7,              /* Video feeder 7 */
    BAVC_SourceId_eVfdMax = BAVC_SourceId_eVfd7,
    BAVC_SourceId_eMax               /* Max source  */

} BAVC_SourceId;

/***************************************************************************
Summary:
    This enumeration represents the different analog decoder.

Description:
    BAVC_DecoderId is an enumeration which represents the analog decoder
    that is repsonsible for decode analog input.

    BAVC_DecoderId_eVindeco_0 -
        This represent the analog decoder (VINDECO) for VGA or YPrPb input.

    BAVC_DecoderId_eVdec_0 -
        This represent the analog decoder (VDEC) for composite/SVIDEO/SCART/IF
        Demodulator input.

    BAVC_DecoderId_e656In_0 -
        This represent the 656 input. This generally means external analog
        decoder.

    BAVC_DecoderId_eMaxCount -
        Counter. Do not use.

See Also:
    BANV_Source_GetDefaultSettings, BANV_Source_Create, BANV_Source_Destroy,
    BVDC_Source_SetDecoderID, BVDC_Source_GetDecoderID.
***************************************************************************/
typedef enum
{
    BAVC_DecoderId_eVindeco_0 = 0,
    BAVC_DecoderId_eVdec_0,
    BAVC_DecoderId_e656In_0,

    BAVC_DecoderId_eMaxCount

} BAVC_DecoderId;

/***************************************************************************
Summary:
    This enumeration describes the VBI paths within the Video Encoder block

Description:
    Contains the path taken inside the VEC, which is required by the VBI PI.
    BAVC_VbiPath_eVec0 - Represent the Primary VEC VBI encoder.
    BAVC_VbiPath_eVec1 - Represent the Secondary VEC VBI encoder.
    BAVC_VbiPath_eVec2 - Represent the Tertiary VEC VBI encoder.
    BAVC_VbiPath_eBypass0 - Represent the first 656 Bypass VEC VBI encoder.
    BAVC_VbiPath_eBypass1 - Represent the second 656 Bypass VEC VBI encoder.

See Also:
    BVDC_Display_GetVbiPath
***************************************************************************/
typedef enum BAVC_VbiPath
{
    BAVC_VbiPath_eVec0 = 0,            /* Primary VEC path */
    BAVC_VbiPath_eVec1,                /* Secondary VEC path */
    BAVC_VbiPath_eVec2,                /* Tertiary VEC path */
    BAVC_VbiPath_eBypass0,             /* 656 vec */
    BAVC_VbiPath_eBypass1,             /* 656 vec */

    BAVC_VbiPath_eUnknown              /* Unknown path */

} BAVC_VbiPath;

/***************************************************************************
Summary:
    Used to specify scan type.

Description:

See Also:
****************************************************************************/
typedef enum BAVC_ScanType
{
    BAVC_ScanType_eInterlaced = 0,   /* Interlaced */
    BAVC_ScanType_eProgressive       /* Progressive */

} BAVC_ScanType;


/***************************************************************************
Summary:
    Used to specify field polarity.

Description:
    The values are suitable for assignment to an integer bitmask.

See Also:
****************************************************************************/
typedef enum BAVC_Polarity
{
    BAVC_Polarity_eTopField = 0,       /* Top field */
    BAVC_Polarity_eBotField,           /* Bottom field */
    BAVC_Polarity_eFrame               /* Progressive frame */

} BAVC_Polarity;

/***************************************************************************
Summary:
    Used to specify interpolation mode.

Description:

See Also:
****************************************************************************/
typedef enum BAVC_InterpolationMode
{
    BAVC_InterpolationMode_eField = 0,  /* Field */
    BAVC_InterpolationMode_eFrame       /* Frame */

} BAVC_InterpolationMode;

/***************************************************************************
Summary:
    Used to specify Chroma location type.

Description:
    LT1-----C-----LT2
     |             |
     |             |
     A------B------|
     |             |
     |             |
    LB1-----D-----LB2

    Luma sample position:
        Top field:    LT1, LT2
        Bottom field: LB1, LB2

    Chroma sample position:
        BAVC_ChromaLocation_eType0: A
        BAVC_ChromaLocation_eType1: B
        BAVC_ChromaLocation_eType2: LT1
        BAVC_ChromaLocation_eType3: C
        BAVC_ChromaLocation_eType4: LB1
        BAVC_ChromaLocation_eType5: D
See Also:
****************************************************************************/
typedef enum BAVC_ChromaLocation
{
    BAVC_ChromaLocation_eType0 = 0,
    BAVC_ChromaLocation_eType1,
    BAVC_ChromaLocation_eType2,
    BAVC_ChromaLocation_eType3,
    BAVC_ChromaLocation_eType4,
    BAVC_ChromaLocation_eType5

} BAVC_ChromaLocation;

/* Backward compatibility */
#define BAVC_MpegType_eMpeg1     BAVC_ChromaLocation_eType1
#define BAVC_MpegType_eMpeg2     BAVC_ChromaLocation_eType0

/***************************************************************************
Summary:
    Used to specify YCbCr type.

Description:
    The values assigned to these enumerations should be kept in step with
    the ISO 13818-2 specification to minimize effort converting to these
    types.

See Also:
****************************************************************************/
typedef enum BAVC_YCbCrType
{
    BAVC_YCbCrType_e4_2_0 = 1,  /* 4:2:0 */
    BAVC_YCbCrType_e4_2_2    ,  /* 4:2:2 */
    BAVC_YCbCrType_e4_4_4       /* 4:4:4 (unsupported) */

} BAVC_YCbCrType;


/***************************************************************************
Summary:
    Use to specify what color space conversion matrix should be use by
    HD_DVI.

Description:
    BAVC_CscMode_e709RgbFullRange - Conversion from
       [Full Range RGB BT.709 / xvYCC 709] to [YCbCr BT.709 / xvYCC 709 ]

    BAVC_CscMode_e709RgbLimitedRange -  Conversion from
       [Limited Range RGB BT.709 / xvYCC 709] to [YCbCr BT.709 / xvYCC 709 ]

    BAVC_CscMode_e709YCbCr - Conversion from YCbCr to YCbCr, no conversion needed.

    BAVC_CscMode_e601RgbFullRange - Conversion from
       [Full Range RGB BT.601 / SMPTE 170M / xvYCC 601] to
       [YCbCr BT.601 / SMPTE 170M / xvYCC 601 ]

    BAVC_CscMode_e601RgbLimitedRange - Conversion from
       [Limited Range RGB BT.601 / SMPTE 170M / xvYCC 601] to
       [YCbCr BT.601 / SMPTE 170M / xvYCC 601 ]

    BAVC_CscMode_e601YCbCr - Conversion from YCbCr to YCbCr, no conversion needed.

See Also:
****************************************************************************/
typedef enum
{
    BAVC_CscMode_e709RgbFullRange = 0,
    BAVC_CscMode_e709RgbLimitedRange,
    BAVC_CscMode_e709YCbCr,

    BAVC_CscMode_e601RgbFullRange,
    BAVC_CscMode_e601RgbLimitedRange,
    BAVC_CscMode_e601YCbCr,

    BAVC_CscMode_e2020RgbFullRange,
    BAVC_CscMode_e2020RgbLimitedRange,
    BAVC_CscMode_e2020YCbCr,

    /* Must be last */
    BAVC_CscMode_eMax

} BAVC_CscMode;



/***************************************************************************
Summary:
    Used to specify color space type.  This extracted out of the AVI info
    frame.  Y1Y0.

Description:

See Also:
    BAVC_MatrixCoefficients, BAVC_ColorRange
****************************************************************************/
typedef enum
{
    BAVC_Colorspace_eRGB = 0,
    BAVC_Colorspace_eYCbCr422,
    BAVC_Colorspace_eYCbCr444,
    BAVC_Colorspace_eYCbCr420,
    BAVC_Colorspace_eFuture

} BAVC_Colorspace;

/***************************************************************************
Summary:
    Video color range

Description:
    Used to specify HDMI output color component range.
    Inside BVN, RGB is always full range, and YCbCr is always limited range.

See Also:
    BAVC_MatrixCoefficients, BAVC_ColorSpace
****************************************************************************/
typedef enum BAVC_ColorRange {
    BAVC_ColorRange_eAuto,  /* colorRange will be decided by BVDC */
    BAVC_ColorRange_eLimited,
    BAVC_ColorRange_eFull,
    BAVC_ColorRange_eMax
} BAVC_ColorRange;

/***************************************************************************
Summary:
    Used to specify the possible frame rates.

Description:
    The values assigned to these enumerations should be kept in step with
    the ISO 13818-2 specification to minimize effort converting to these
    types.

See Also:
****************************************************************************/
typedef enum BAVC_FrameRateCode
{
    BAVC_FrameRateCode_eUnknown = 0, /* Unknown */
    BAVC_FrameRateCode_e23_976 = 1,
    BAVC_FrameRateCode_e24,
    BAVC_FrameRateCode_e25,
    BAVC_FrameRateCode_e29_97,
    BAVC_FrameRateCode_e30,
    BAVC_FrameRateCode_e50,
    BAVC_FrameRateCode_e59_94,
    BAVC_FrameRateCode_e60,
    BAVC_FrameRateCode_e14_985,
    BAVC_FrameRateCode_e7_493,
    BAVC_FrameRateCode_e10,
    BAVC_FrameRateCode_e15,
    BAVC_FrameRateCode_e20,
    BAVC_FrameRateCode_e12_5,
    BAVC_FrameRateCode_e100,
    BAVC_FrameRateCode_e119_88,
    BAVC_FrameRateCode_e120,
    BAVC_FrameRateCode_e19_98,
    BAVC_FrameRateCode_e7_5,
    BAVC_FrameRateCode_e12,
    BAVC_FrameRateCode_e11_988,
    BAVC_FrameRateCode_e9_99,
    BAVC_FrameRateCode_eMax          /* Max Enum Value */
} BAVC_FrameRateCode;

/* TODO: to be removed shortly after xvd/synclib/etc adapt to new naming changes. */
#define BAVC_FrameRateCode_e7_943 BAVC_FrameRateCode_e7_493

/***************************************************************************
Summary:
    Used to enumerate the possible color space standards

Description:
    This enum follows the MPEG-2 standard Video-spec except the RGB output
    enum(=3), which is intended for HDMI use only in case of RGB output to
    an HDMI Rx device to indicate the RGB color space interpretation of the
    digital video data.

See Also:
    BAVC_ColorPrimaries, BAVC_TransferCharacteristics,
    BAVC_ColorSpace, BAVC_ColorRange
****************************************************************************/
typedef enum BAVC_MatrixCoefficients
{
    /* Recommendation ITU-R BT.709. Typical video format: ATSC HD, PAL HD, UHD */
    BAVC_MatrixCoefficients_eItu_R_BT_709 = 1,

    /* FCC. Typical video format: Obsolete 1953 NTSC SD */
    BAVC_MatrixCoefficients_eFCC = 4,

    /* Recommendation ITU-R BT.470-2 System B, G. Typical video format: PAL SD */
    BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG = 5,

    /* SMPTE 170M. Typical video format: NTSC SD */
    BAVC_MatrixCoefficients_eSmpte_170M = 6,

    /* SMPTE 240M. Typical video format: Obsolete 1987 ATSC HD */
    BAVC_MatrixCoefficients_eSmpte_240M = 7,

    /* Rec ITU-R BT.2020 non-constant luminance: Typical video format: UHD */
    BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL = 9,

    /* Rec ITU-R BT.2020 constant luminance: Typical video format: UHD */
    BAVC_MatrixCoefficients_eItu_R_BT_2020_CL = 10,

    /* HDMI 1.3 xvYCC709. Typical video format: HD */
    BAVC_MatrixCoefficients_eXvYCC_709 = 11,

    /* HDMI 1.3 xvYCC601. Typical video format: SD */
    BAVC_MatrixCoefficients_eXvYCC_601 = 12,

    /* Unspecified Video: Image characteristics are unknown;
       VDC would handle 'Unknown' case as, i.e.
       if the decoded picture is in HD format(size is larger than
       720x576), then take default HD color matrix; else take default SD color
       matrix.

       Note: it may also be used to indicate the display doesn't output HDMI
       video. */
    BAVC_MatrixCoefficients_eUnknown = 2,

    /* 0, 3,  8 - 255 reserved;
       MVD should handle the error and provide 'Unknown' to VDC. */

    /* to be obsolete */
    BAVC_MatrixCoefficients_eHdmi_RGB = 0,

    /* to be obsolete */
    BAVC_MatrixCoefficients_eDvi_Full_Range_RGB = 3,

    /* to be obsolete */
    BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr = 13

} BAVC_MatrixCoefficients;

/***************************************************************************
Summary:
    Used to specify the possible color primaries.

Description:
    This enum follows the MPEG-2 standard Video-spec.

See Also:
    BAVC_MatrixCoefficients, BAVC_TransferCharacteristics.
****************************************************************************/
typedef enum BAVC_ColorPrimaries
{
    /* Recommendation ITU-R BT.709;
       (ATSC HD or PAL HD) */
    BAVC_ColorPrimaries_eItu_R_BT_709 = 1,

    /* Recommendation ITU-R BT.470-2 System M;
       (NTSC SD 1953, not the same as PAL SD nor SMPTE170) */
    BAVC_ColorPrimaries_eItu_R_BT_470_2_M = 4,

    /* Recommendation ITU-R BT.470-2 System B, G;
       (PAL SD, similar to SMPTE170) */
    BAVC_ColorPrimaries_eItu_R_BT_470_2_BG = 5,

    /* SMPTE 170M; (NTSC SD) */
    BAVC_ColorPrimaries_eSmpte_170M = 6,

    /* SMPTE 240M (1987);
       (ATSC HD; same as SMPTE170) */
    BAVC_ColorPrimaries_eSmpte_240M = 7,

    /* Generic file
       AVC specification ??? */
    BAVC_ColorPrimaries_eGenericFilm = 8,

    /* Rec. ITU-R BT. 2020
       (UHDTV) */
    BAVC_ColorPrimaries_eItu_R_BT_2020 = 9,

    /* Unspecified Video: Image characteristics are unknown;
       VDC would handle 'Unknown' case as follows, i.e.
       if the decoded picture is in HD format(size is larger than
       720x576), then take default HD color matrix; else take default SD color
       matrix. */
    BAVC_ColorPrimaries_eUnknown = 2

    /* 0 - forbidden;
       MVD should handle the error and provide 'Unknown' to VDC. */

    /* 3, 8 - 255 reserved;
       MVD should handle the error and provide 'Unknown' to VDC. */

} BAVC_ColorPrimaries;

/***************************************************************************
Summary:
    Used to specify the possible transfer characteristics.

Description:
    This enum follows the MPEG-2 standard Video-spec.

See Also:
    BAVC_MatrixCoefficients, BAVC_ColorPrimaries.
****************************************************************************/
typedef enum BAVC_TransferCharacteristics
{
    /* Recommendation ITU-R BT.709;
       (ATSC HD or PAL HD) */
    BAVC_TransferCharacteristics_eItu_R_BT_709 = 1,

    /* FCC, or Recommendation ITU-R BT.470-2 System M;
       (NTSC SD 1953, assumed display gamma 2.2) */
    BAVC_TransferCharacteristics_eItu_R_BT_470_2_M = 4,

    /* Recommendation ITU-R BT.470-2 System B, G;
       (PAL SD, assumed display gamma 2.8) */
    BAVC_TransferCharacteristics_eItu_R_BT_470_2_BG = 5,

    /* SMPTE 170M; (NTSC SD) */
    BAVC_TransferCharacteristics_eSmpte_170M = 6,

    /* SMPTE 240M (1987); (ATSC HD) */
    BAVC_TransferCharacteristics_eSmpte_240M = 7,

    /* Linear Transfer Characteristics */
    BAVC_TransferCharacteristics_eLinear = 8,

    /* Recommendation ITU-T H.262, H.264; (IEC 61966-2-4 gamma, xvYCC) */
    BAVC_TransferCharacteristics_eIec_61966_2_4 = 11,

    /* Recommendation ITU-R BT.2020 10 bit system;
       (Ultra HD) */
    BAVC_TransferCharacteristics_eItu_R_BT_2020_10bit = 14,

    /* Recommendation ITU-R BT.709 12 bit ssytem;
       (Ultra HD) */
    BAVC_TransferCharacteristics_eItu_R_BT_2020_12bit = 15,

    /* SMPTE ST 2084 */
    BAVC_TransferCharacteristics_eSmpte_ST_2084 = 16,

    /* ARIB STD-B67 */
    BAVC_TransferCharacteristics_eArib_STD_B67 = 18,

    /* Unspecified Video: Image characteristics are unknown;
       VDC would handle 'Unknown' case as follows, i.e.
       if the decoded picture is in HD format(size is larger than
       720x576), then take default HD color matrix; else take default
       SD color. */
    BAVC_TransferCharacteristics_eUnknown = 2

    /* 0 - forbidden;
       MVD should handle the error and provide 'Unknown' to VDC. */

    /* 3, 9 - 255 reserved;
       MVD should handle the error and provide 'Unknown' to VDC. */
} BAVC_TransferCharacteristics;

/***************************************************************************
Summary:
    Used to specify the source state.

Description:
    When source picture call back function is used to pull the video/graphics
    field/frame produced by an upstream module to BVDC for display, the
    current state of the used BVDC source sub-source is passed from BVDC to
    user as a parameter of the call back function, in order to synchronize
    the upstream module and BVDC.

See Also:
    BVDC_Source_PictureCallback_isr, BVDC_Source_InstallPictureCallback
****************************************************************************/
typedef enum BAVC_SourceState
{
    BAVC_SourceState_eActive_Create, /* The source was just created. */
    BAVC_SourceState_eActive_Sync,   /* BVDC has recieved a sync pulse. */
    BAVC_SourceState_eActive_Start,  /* BVDC has recieved the first frame after the sync pulse. */
    BAVC_SourceState_eActive_Reset,  /* We need to restart the syncronization process. */
    BAVC_SourceState_eActive,        /* Simply active. Normal case */
    BAVC_SourceState_eActive_Last,   /* The source is about to be destroyed. The next callback will be the last. */
    BAVC_SourceState_eDestroy        /* The source has been destroyed and is no longer active, This callback is the last. */

} BAVC_SourceState;


/***************************************************************************
Summary:
    Defines the timebase.

Description:
    This enum defines the timebase or pcr to PLLs, RateMangers
    and audio/video decoders/DACs.

See Also:
****************************************************************************/
typedef enum BAVC_Timebase
{
    BAVC_Timebase_e0 = 0,   /* TimeBase/PCR 0 */
    BAVC_Timebase_e1,       /* TimeBase/PCR 1 */
    BAVC_Timebase_e2,       /* TimeBase/PCR 2 */
    BAVC_Timebase_e3,       /* TimeBase/PCR 3 */
    BAVC_Timebase_e4,       /* TimeBase/PCR 4 */
    BAVC_Timebase_e5,       /* TimeBase/PCR 5 */
    BAVC_Timebase_e6,       /* TimeBase/PCR 6 */
    BAVC_Timebase_e7,       /* TimeBase/PCR 7 */
    BAVC_Timebase_e8,       /* TimeBase/PCR 8 */
    BAVC_Timebase_e9,       /* TimeBase/PCR 9 */
    BAVC_Timebase_e10,      /* TimeBase/PCR 10 */
    BAVC_Timebase_e11,      /* TimeBase/PCR 11 */
    BAVC_Timebase_e12,      /* TimeBase/PCR 12 */
    BAVC_Timebase_e13,      /* TimeBase/PCR 13 */
    BAVC_Timebase_eMax      /* TimeBase/PCR max */

}BAVC_Timebase;


/***************************************************************************
Summary:
    Used to specify stripe width.

Description:
    The values defines stripe width of the decoder.

See Also:
****************************************************************************/
typedef enum BAVC_StripeWidth
{
    BAVC_StripeWidth_e64Byte = 0,
    BAVC_StripeWidth_e128Byte,
    BAVC_StripeWidth_e256Byte

} BAVC_StripeWidth;

/***************************************************************************
Summary:
    The picture coding type

Description:
    This enum value follows MPEG picture coding type for I, P or B picture.
    The AVC decoder fw sets picture coding type according to the slice type
    of the 1st slice in picture. THE VC-1 and MPEG4 decoder fw sets it according
    to the codec syntax definition for the picture coding type.
    For the User data the Picture Header it can be associated with different
    MPEG coding types like I, P or B frames. This type is enumerated here.

See Also:

****************************************************************************/
typedef enum BAVC_PictureCoding
{
    BAVC_PictureCoding_eUnknown=0,    /* Picture Coding Type Unknown */
    BAVC_PictureCoding_eI,             /* Picture Coding Type I */
    BAVC_PictureCoding_eP,             /* Picture Coding Type P */
    BAVC_PictureCoding_eB,             /* Picture Coding Type B */
    BAVC_PictureCoding_eMax

} BAVC_PictureCoding;

/* The following is to be backward compatible. However, it should be deprecated. */
#define BAVC_USERDATA_PictureCoding                     BAVC_PictureCoding
#define BAVC_USERDATA_PictureCoding_eUnknown  BAVC_PictureCoding_eUnknown
#define BAVC_USERDATA_PictureCoding_eI               BAVC_PictureCoding_eI
#define BAVC_USERDATA_PictureCoding_eP               BAVC_PictureCoding_eP
#define BAVC_USERDATA_PictureCoding_eB               BAVC_PictureCoding_eB
#define BAVC_USERDATA_PictureCoding_eMax           BAVC_PictureCoding_eMax


/***************************************************************************
Summary:
    The picture coding type

Description:
    BAR data.

See Also:

****************************************************************************/
typedef enum BAVC_BarDataType
{
    BAVC_BarDataType_eInvalid = 0,     /* No data */
    BAVC_BarDataType_eTopBottom,       /* Bar data defines top/bottom */
    BAVC_BarDataType_eLeftRight        /* Bar data defines left/right */

} BAVC_BarDataType;

/***************************************************************************
Summary:
    A structure contains information about the current display state, such
    as current refresh rate, pixel clock rate, and timebase used.

Description:
    This structure is used to pass a information about a display information
    to application that needs to know about the display information.   For
    example HDMI module needs to know the pixel clock rate, so it can match
    its output pixel clock rate as well.

See Also:
    BVDC_CallbackFunc_isr,
    BVDC_Display_InstallRateChangeCallback,
    BVDC_Window_SetMasterFrameRate.
****************************************************************************/
typedef struct BAVC_VdcDisplay_Info
{
    uint64_t                 ulPixelClkRate;     /* defines in bfmt.h (Clock Rate Mask see BFMT_PXL_*) */
    uint32_t                 ulPixelClockRate;   /* defines in bfmt.h (1/BFMT_FREQ_FACTOR Mhz) */
    uint32_t                 ulVertRefreshRate;  /* defines in bfmt.h (1/BFMT_FREQ_FACTOR Hz) */

} BAVC_VdcDisplay_Info;

/***************************************************************************
Summary:
    Defines audio sampling rate

Description:
    This enum defines audio sampling rate.

See Also:
****************************************************************************/
typedef enum BAVC_AudioSamplingRate
{
    BAVC_AudioSamplingRate_eUnknown,  /* Unknown sample rate */
    BAVC_AudioSamplingRate_e8k,       /* 8K Sample rate */
    BAVC_AudioSamplingRate_e11_025k,  /* 11.025K Sample rate */
    BAVC_AudioSamplingRate_e12k,      /* 12K Sample rate */
    BAVC_AudioSamplingRate_e16k,      /* 16K Sample rate */
    BAVC_AudioSamplingRate_e22_05k,   /* 22.05K Sample rate */
    BAVC_AudioSamplingRate_e24k,      /* 24K Sample rate */
    BAVC_AudioSamplingRate_e32k,      /* 32K Sample rate */
    BAVC_AudioSamplingRate_e44_1k,    /* 44.1K Sample rate */
    BAVC_AudioSamplingRate_e48k,      /* 48K Sample rate */
    BAVC_AudioSamplingRate_e64k,      /* 64K Sample rate */
    BAVC_AudioSamplingRate_e88_2k,    /* 88.2K Sample rate */
    BAVC_AudioSamplingRate_e96k,      /* 96K Sample rate */
    BAVC_AudioSamplingRate_e128k,     /* 128K Sample rate */
    BAVC_AudioSamplingRate_e176_4k,   /* 176.4K Sample rate */
    BAVC_AudioSamplingRate_e192k,     /* 192K Sample rate */
    BAVC_AudioSamplingRate_eMax
}BAVC_AudioSamplingRate;


/***************************************************************************
Summary:
    Defines supported number of Audio bits

Description:
    This enum is used to query HDMI monitors for supported audio bits

See Also:
****************************************************************************/
typedef enum BAVC_AudioBits
{
    BAVC_AudioBits_e16=0,
    BAVC_AudioBits_e20,
    BAVC_AudioBits_e24,
    BAVC_AudioBits_eMax

} BAVC_AudioBits ;


/***************************************************************************
Summary:
    Defines supported Audio Formats

Description:
    This enum is used to query HDMI monitors for supported Audio Formats

See Also:
****************************************************************************/
typedef enum BAVC_AudioFormat
{
    BAVC_AudioFormat_ePCM=0,
    BAVC_AudioFormat_eAC3,
    BAVC_AudioFormat_eMPEG1,
    BAVC_AudioFormat_eMP3,
    BAVC_AudioFormat_eMPEG2,
    BAVC_AudioFormat_eAAC,
    BAVC_AudioFormat_eDTS,
    BAVC_AudioFormat_eAVS,
    BAVC_AudioFormat_eATRAC,
    BAVC_AudioFormat_eOneBit,
    BAVC_AudioFormat_eDDPlus,
    BAVC_AudioFormat_eDTSHD,
    BAVC_AudioFormat_eMATMLP,
    BAVC_AudioFormat_eDST,
    BAVC_AudioFormat_eWMAPro,

    BAVC_AudioFormat_eMaxCount

} BAVC_AudioFormat ;

/***************************************************************************
Summary:
    A structure contains information about the current audio state, such
    as Sample Rate, etc.

Description:
    This structure is used to pass a information about audio to the
    application that needs to know about the audio.  For example, the HDMI
    module needs to know the Audio Sample Rate, so it can properly adjust
    its audio parameters for the current rate.

See Also:
    BAUD_EnableSampleRateChangeCb
****************************************************************************/
typedef struct BAVC_Audio_Info
{
    BAVC_AudioSamplingRate   eAudioSamplingRate ;

} BAVC_Audio_Info ;


/***************************************************************************
Summary:
    The User Data Type

Description:
    The User data can come in the Sequence Header, the GOP Header
    or the Picture Header. This type is enumerated here.

See Also:

****************************************************************************/
typedef enum BAVC_USERDATA_Type
{
    BAVC_USERDATA_Type_eSeq=1,       /* User data coming in the Sequence Header */
    BAVC_USERDATA_Type_eGOP,         /* User data coming in the Gop Header */
    BAVC_USERDATA_Type_ePicture,     /* User Data coming in Picture Header */
    BAVC_USERDATA_Type_eSEI,         /* User Data in H.264 */
    BAVC_USERDATA_Type_eEntryPoint, /* VC1 user data coming in entry point layer */
    BAVC_USERDATA_Type_eField,       /* VC1 user data coming in picture field layer */
    BAVC_USERDATA_Type_eFrame,       /* VC1 user data coming in picture frame layer */
    BAVC_USERDATA_Type_eSlice,       /* VC1 user data coming in slice layer */
    BAVC_USERDATA_Type_eMax
} BAVC_USERDATA_Type;




/***************************************************************************
Summary:
    User Data Info structure common to MVD (Mini-Titan) and XVD (AVC decoder).

Description:
    Interface structure between application and microcode. Microcode
    initializes the fields of this structure to inform application
    about the current status of user data.

See Also:

****************************************************************************/
typedef struct BAVC_USERDATA_info
{
    void            *pUserDataBuffer;   /* Pointer to the pre-allocated buffer */
    uint32_t        ui32UserDataBufSize;    /* Total buffer size */
    BAVC_USERDATA_Type eUserDataType ;  /* Type of the User Data */
    bool            bTopFieldFirst ;    /* MPEG Syntax flag TopFieldFirst */
    bool            bRepeatFirstField ; /* MPEG Syntax flag RepeatFirstField */
    BAVC_PictureCoding ePicCodingType ; /* I/P/B Picture Coding Type */
    uint32_t        ui32PTS ;           /* PTS value of the associated frame */
    bool            bPTSValid ;         /* PTS value of the associated frame */
    bool            bErrorBufferOverflow ;  /* To indicate buffer overflow */
    BAVC_Polarity   eSourcePolarity ;   /* Source Polarity if data found in
                                           Picture Header */
    uint32_t        ui32PicCodExt[2] ;  /* Array of 2 32 bit variable for
                                            storing Picture Coding Extension
                                            All bits from the MPEG stream are
                                            stored as they come in the earliest
                                            going to the highest bit in the word
                                            In word-0 bits 31-30 are unused
                                            and in word-1 bits 31-20 are unused */
    uint32_t        ulDecodePictureId;  /* Decoded picture id for userdata transcode */

} BAVC_USERDATA_info;

/***************************************************************************
Summary:
    Enum for specifying the PTS type.

Description:
    This can be used by the SerialPCRLib to decide whether the PTS can
    be used to jam the STC or not. For example, a coded PTS might be given
    preference to a interpolated PTS.

See Also:

****************************************************************************/
typedef enum BAVC_PTSType
{
    BAVC_PTSType_eCoded,
    BAVC_PTSType_eInterpolatedFromValidPTS,
    BAVC_PTSType_eHostProgrammedPTS,
    BAVC_PTSType_eInterpolatedFromInvalidPTS,
    BAVC_PTSType_eMax
} BAVC_PTSType;

/***************************************************************************
Summary:
    This data structure is passed with the callbacks associated with
    the FirsPTSReady and FirsTSMPassed interrupts. The XXX_GetPTS
    funxtion also might use this structure.

Description:
    This structure consists of the 32 bit PTS value itself and a tag
    associated with it that tells whether this is a coded PTS, an
    Interpolated PTS from a previous Coded PTS, or PTS interpoleted from
    a junk PTS value.

****************************************************************************/
typedef struct BAVC_PTSInfo
{
    uint32_t        ui32CurrentPTS; /* Current PTS value */
    BAVC_PTSType    ePTSType;           /* The PTS type tag */

    /* needed for PCRLib */
    uint32_t uiDecodedFrameCount;
    uint32_t uiDroppedFrameCount;

} BAVC_PTSInfo;

/***************************************************************************
Summary:
    Enum for specifying the HDR metadata type.

Description:
    This can be used by the VDC to decide how to program HDR hw based on the stream
    HDR metadata.

See Also:
    BAVC_HdrMetadata
****************************************************************************/
typedef enum BAVC_HdrMetadataType
{
    BAVC_HdrMetadataType_eUnknown=0,
    BAVC_HdrMetadataType_eDrpu,
    BAVC_HdrMetadataType_eTch_Cvri,
    BAVC_HdrMetadataType_eTch_Cri,
    BAVC_HdrMetadataType_eTch_Slhdr,
    BAVC_HdrMetadataType_eMax

} BAVC_HdrMetadataType;

/***************************************************************************
Summary:
    This data structure is passed with the mpeg data ready callback.

Description:
    This structure consists of the HDR metadata type, size and pointer.

See Also:
    BAVC_MFD_Picture
****************************************************************************/
#define BAVC_HDR_METADATA_SIZE_MAX           2048

typedef struct BAVC_HdrMetadata
{
    BAVC_HdrMetadataType    eType;
    uint32_t                ulSize;
    void                   *pData; /* should be NULL if eType is eUnknown */

} BAVC_HdrMetadata;

/***************************************************************************
Summary:
    Defines Video compression standards

Description:
    This enum is used to set the video compression standard use in XVD PI.

See Also:
****************************************************************************/
typedef enum BAVC_VideoCompressionStd
{
    BAVC_VideoCompressionStd_eH264,           /* H.264 */
    BAVC_VideoCompressionStd_eMPEG2,          /* MPEG-2 */
    BAVC_VideoCompressionStd_eH261,           /* H.261 */
    BAVC_VideoCompressionStd_eH263,           /* H.263 */
    BAVC_VideoCompressionStd_eVC1,            /* VC1 Advanced profile */
    BAVC_VideoCompressionStd_eMPEG1,          /* MPEG-1 */
    BAVC_VideoCompressionStd_eMPEG2DTV,       /* MPEG-2 DirecTV DSS ES */
    BAVC_VideoCompressionStd_eVC1SimpleMain,  /* VC1 Simple & Main profile */
    BAVC_VideoCompressionStd_eMPEG4Part2,     /* MPEG 4, Part 2. */
    BAVC_VideoCompressionStd_eAVS,            /* AVS Jinzhun profile. */
    BAVC_VideoCompressionStd_eMPEG2_DSS_PES,  /* MPEG-2 DirecTV DSS PES */
    BAVC_VideoCompressionStd_eSVC,            /* Scalable Video Codec */
    BAVC_VideoCompressionStd_eSVC_BL,         /* Scalable Video Codec Base Layer */
    BAVC_VideoCompressionStd_eMVC,            /* MVC Multi View Coding */
    BAVC_VideoCompressionStd_eVP6,            /* VP6 */
    BAVC_VideoCompressionStd_eVP7,            /* VP7 */
    BAVC_VideoCompressionStd_eVP8,            /* VP8 */
    BAVC_VideoCompressionStd_eRV9,            /* Real Video 9 */
    BAVC_VideoCompressionStd_eSPARK,          /* Sorenson Spark */
    BAVC_VideoCompressionStd_eMOTION_JPEG,    /* Motion Jpeg */
    BAVC_VideoCompressionStd_eH265,           /* H.265 */
    BAVC_VideoCompressionStd_eVP9,            /* VP9 */
    BAVC_VideoCompressionStd_eAVS2,           /* AVS2  */
    BAVC_VideoCompressionStd_eMax

} BAVC_VideoCompressionStd;

/***************************************************************************
Summary:
    Defines Video compression profiles

Description:
    This enum is used to report the video compression profile by XVD PI.
    Also used to specify the video compression profile to VCE PI

    This enum is defined by AVD FW team

See Also:
****************************************************************************/
typedef enum BAVC_VideoCompressionProfile
{
    BAVC_VideoCompressionProfile_eUnknown = 0,
    BAVC_VideoCompressionProfile_eSimple,
    BAVC_VideoCompressionProfile_eMain,
    BAVC_VideoCompressionProfile_eHigh,
    BAVC_VideoCompressionProfile_eAdvanced,
    BAVC_VideoCompressionProfile_eJizhun,
    BAVC_VideoCompressionProfile_eSnrScalable,
    BAVC_VideoCompressionProfile_eSpatiallyScalable,
    BAVC_VideoCompressionProfile_eAdvancedSimple,
    BAVC_VideoCompressionProfile_eBaseline,
    BAVC_VideoCompressionProfile_eMultiHighProfile,
    BAVC_VideoCompressionProfile_eStereoHighProfile,
    BAVC_VideoCompressionProfile_eMain10,
    BAVC_VideoCompressionProfile_e0,
    BAVC_VideoCompressionProfile_e2,
    BAVC_VideoCompressionProfile_eGuango,

    BAVC_VideoCompressionProfile_eMax

} BAVC_VideoCompressionProfile;

/***************************************************************************
Summary:
    Defines Video compression levels

Description:
    This enum is used to report the video compression level by XVD PI.
    Also used to specify the video compression level to VCE PI

    This enum is defined by AVD FW team

See Also:
****************************************************************************/
typedef enum BAVC_VideoCompressionLevel
{
    BAVC_VideoCompressionLevel_eUnknown = 0,
    BAVC_VideoCompressionLevel_e00,
    BAVC_VideoCompressionLevel_e10,
    BAVC_VideoCompressionLevel_e1B,
    BAVC_VideoCompressionLevel_e11,
    BAVC_VideoCompressionLevel_e12,
    BAVC_VideoCompressionLevel_e13,
    BAVC_VideoCompressionLevel_e20,
    BAVC_VideoCompressionLevel_e21,
    BAVC_VideoCompressionLevel_e22,
    BAVC_VideoCompressionLevel_e30,
    BAVC_VideoCompressionLevel_e31,
    BAVC_VideoCompressionLevel_e32,
    BAVC_VideoCompressionLevel_e40,
    BAVC_VideoCompressionLevel_e41,
    BAVC_VideoCompressionLevel_e42,
    BAVC_VideoCompressionLevel_e50,
    BAVC_VideoCompressionLevel_e51,
    BAVC_VideoCompressionLevel_e60,
    BAVC_VideoCompressionLevel_e62,
    BAVC_VideoCompressionLevel_eLow,
    BAVC_VideoCompressionLevel_eMain,
    BAVC_VideoCompressionLevel_eHigh,
    BAVC_VideoCompressionLevel_eHigh1440,
    BAVC_VideoCompressionLevel_eL0,
    BAVC_VideoCompressionLevel_eL1,
    BAVC_VideoCompressionLevel_eL2,
    BAVC_VideoCompressionLevel_eL3,
    BAVC_VideoCompressionLevel_e80,
    BAVC_VideoCompressionLevel_e52,

    BAVC_VideoCompressionLevel_eMax

} BAVC_VideoCompressionLevel;

/***************************************************************************
Summary:
    Defines Video decompressed picture buffer format

Description:
    This enum is used to report the video decompressed picture buffer format from XVD.
    Note, all legacy video codecs have frame picture buffer storage out of decoder. HEVC
    or H265 adds a new decompressed picture buffer format in fields pair for interlaced
    content.

See Also:
****************************************************************************/
typedef enum BAVC_DecodedPictureBuffer
{
    BAVC_DecodedPictureBuffer_eFrame = 0,
    BAVC_DecodedPictureBuffer_eFieldsPair,

    BAVC_DecodedPictureBuffer_eMax

} BAVC_DecodedPictureBuffer;

/***************************************************************************
Summary:
    Defines video sample's bit depth per component.

Description:
    This enum is used to report the video sample bit depth. Conventional video codecs support
    8-bit per component video sample, while h265 video spec also supports 10-bit video.

See Also:
****************************************************************************/
typedef enum BAVC_VideoBitDepth
{
    BAVC_VideoBitDepth_e8Bit = 8,
    BAVC_VideoBitDepth_e9Bit = 9,
    BAVC_VideoBitDepth_e10Bit = 10,

    BAVC_VideoBitDepth_eMax

} BAVC_VideoBitDepth;

/***************************************************************************
Summary:
    Defines h264/5 spec of pic_struct syntax for picture cadence.

Description:

See Also:
****************************************************************************/
typedef enum BAVC_PicStruct
{
    BAVC_PicStruct_eFrame = 0,
    BAVC_PicStruct_eTopField,
    BAVC_PicStruct_eBotField,
    BAVC_PicStruct_eTopFirst,
    BAVC_PicStruct_eBotFirst,
    BAVC_PicStruct_eTopBotTopRepeat,
    BAVC_PicStruct_eBotTopBotRepeat,
    BAVC_PicStruct_eFrameDoubling,
    BAVC_PicStruct_eFrameTripling,
    BAVC_PicStruct_eReserved
} BAVC_PicStruct;

/***************************************************************************
Summary:
    Defines picture cadence type.

Description:
    If eUnlocked, video picture cadence is unlocked; else, it reports 3:2 or 2:2 locked cadence.

See Also:
****************************************************************************/
typedef enum BAVC_CadenceType
{
    BAVC_CadenceType_eUnlocked = 0,
    BAVC_CadenceType_e3_2,
    BAVC_CadenceType_e2_2,
    BAVC_CadenceType_eMax
} BAVC_CadenceType;

/***************************************************************************
Summary:
    Defines picture cadence phase.

Description:
    If video picture cadence type is 3:2 or 2:2 locked type, this enum reports the cadence phase
    for the current picture.

    Example cadence scenarios:

       VSYNC    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |

    Interlaced:
       POLARITY | T | B | T | B | T | B | T | B | T | B |

       TYPE = 3:2
       PHASE    | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | --> [T0 B1 T2] [B3 T4] [B5 T6 B7] [T8 B9]
       PHASE    | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | --> T0 B1] [T2 B3] [T4 B5 T6] [B7 T8] [B9
       PHASE    | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | --> T0] [B1 T2] [B3 T4 B5] [T6 B7] [T8 B9
       PHASE    | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | --> [T0 B1] [T2 B3 T4] [B5 T6] [B7 T8 B9]
       PHASE    | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | --> T0] [B1 T2 B3] [T4 B5] [T6 B7 T8] [B9

       TYPE = 2:2
       PHASE    | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | --> [T0 B1] [T2 B3] [T4 B5] [T6 B7] [T8 B9]
       PHASE    | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | --> T0] [B1 T2] [B3 T4] [B5 T6] [B7 T8] [B9


     Progressive:
        POLARITY | F | F | F | F | F | F | F | F | F | F |

        TYPE = 3:2
        PHASE    | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | --> [F0 F1 F2] [F3 F4] [F5 F6 F7] [F8 F9]
        PHASE    | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | --> F0 F1] [F2 F3] [F4 F5] [F6 F7 F8] [F9
        PHASE    | 2 | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | --> F0] [F1 F2] [F3 F4 F5] [F6 F7] [F8 F9
        PHASE    | 3 | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | --> [F0 F1] [F2 F3 F4] [F5 F6] [F7 F8 F9]
        PHASE    | 4 | 0 | 1 | 2 | 3 | 4 | 0 | 1 | 2 | 3 | --> F0] [F1 F2 F3] [F4 F5] [F6 F7 F8] [F9

        TYPE = 2:2
        PHASE    | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | --> [F0 F1] [F2 F3] [F4 F5] [F6 F7] [F8 F9]
        PHASE    | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | --> F0] [F1 F2] [F3 F4] [F5 F6] [F7 F8] [F9

See Also:
****************************************************************************/
typedef enum BAVC_CadencePhase
{
    BAVC_CadencePhase_e0 = 0, /* 1st picture of cadence 3 of 3:2 type or cadence 2 of 2:2 type */
    BAVC_CadencePhase_e1, /* 2nd picture of cadence 3 of 3:2 type or cadence 2 of 2:2 type */
    BAVC_CadencePhase_e2, /* 3rd picture (repeat) of cadence 3 of 3:2 type */
    BAVC_CadencePhase_e3, /* 1st picture of the cadence 2 of 3:2 type */
    BAVC_CadencePhase_e4, /* 2nd picture of the cadence 2 of 3:2 type */
    BAVC_CadencePhase_eMax
} BAVC_CadencePhase;

/***************************************************************************
Summary:
    Defines point coordinates .

Description:
    This structure consists of the 32 bit X and Y.

See Also:
****************************************************************************/

typedef struct BAVC_Point
{
    uint32_t ulX;
    uint32_t ulY;
} BAVC_Point;

typedef struct BAVC_Dimensions
{
    unsigned ulWidth;
    unsigned ulHeight;
} BAVC_Dimensions;

typedef struct BAVC_Bounds
{
    unsigned uiMin;
    unsigned uiMax;
} BAVC_Bounds;

typedef struct BAVC_Color_Volume
{
    /* units: 0 to 50000 represents 0.0 to 1.0 */
    struct
    {
        BAVC_Point stRed;
        BAVC_Point stGreen;
        BAVC_Point stBlue;
    } stPrimaries;
    BAVC_Point stWhitePoint;
} BAVC_Color_Volume;

/* SVP Core ID */
#define BAVC_CoreId_eNOT_MAP BAVC_CoreId_eInvalid
typedef enum BAVC_CoreId {
#define BCHP_P_MEMC_DEFINE_SVP_HWBLOCK(svp_block, access) BAVC_CoreId_e##svp_block,
#include "memc/bchp_memc_svp_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_SVP_HWBLOCK
    BAVC_CoreId_eInvalid,
    BAVC_CoreId_eMax = BAVC_CoreId_eInvalid
} BAVC_CoreId;

/* SVP Core Access Type */
typedef enum BAVC_Access {
    BAVC_Access_eRO = 0,
    BAVC_Access_eWO,
    BAVC_Access_eRW,
    BAVC_Access_eInvalid,
    BAVC_Access_eMax = BAVC_Access_eInvalid

} BAVC_Access;

/* Core list */
typedef struct
{
    uint8_t aeCores[BAVC_CoreId_eMax]; /* valid cores [0 to MIN(usNum, BAVC_CoreId_eMax) -1] */
} BAVC_CoreList;

#ifdef __cplusplus
}
#endif

#endif /* BAVC_TYPES_H__ */

/* end of file */
