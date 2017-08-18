/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BBOX_VDC_H__
#define BBOX_VDC_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bfmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: This is only used in VDC. See usage below. */
#define BBOX_VDC_DISREGARD                     0x7FFF

/* This needs to be updated if the latest chip has more than this */
#define BBOX_VDC_DEINTERLACER_COUNT            6

/* This is the max number of video windows a display may have. */
#define BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY     (2)

/* This is the max number of graphics windows a display may have. */
#define BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY       (3)

/* This is the max number of windows a display may have. */
#define BBOX_VDC_WINDOW_COUNT_PER_DISPLAY         \
    (BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY + BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY)

/* This needs to be updated if the display ID enum in VDC is updated. */
#define BBOX_VDC_DISPLAY_COUNT                 7

/* This needs to be updated if the number of STG displays in VDC is updated. */
#define BBOX_VDC_STG_DISPLAY_COUNT             6

/* This needs to be updated if the number of HDMI displays in VDC is updated. */
#define BBOX_VDC_HDMI_DISPLAY_COUNT            2

/***************************************************************************
 * capability flags are or-ed during acquiring.
 */
typedef enum
{
    BBOX_Vdc_Resource_eMem0    =     (1<< 0),      /* able to access mem ctrl 0 */
    BBOX_Vdc_Resource_eMem1    =     (1<< 1),      /* able to access mem ctrl 1 */
    BBOX_Vdc_Resource_eMem2    =     (1<< 2),      /* able to access mem ctrl 2 */
    BBOX_Vdc_Resource_eAllSrc  =     (1<< 3),      /* able to use by all sources */
    BBOX_Vdc_Resource_eHd      =     (1<< 4),      /* able to handle HD size */
    BBOX_Vdc_Resource_eMadr0   =     (1<< 5),      /* able to handle transcode 0*/
    BBOX_Vdc_Resource_eMadr1   =     (1<< 6),      /* able to handle transcode 1*/
    BBOX_Vdc_Resource_eMadr2   =     (1<< 7),      /* able to handle transcode 2*/
    BBOX_Vdc_Resource_eMadr3   =     (1<< 8),      /* able to handle transcode 3*/
    BBOX_Vdc_Resource_eMadr4   =     (1<< 9),      /* able to handle transcode 4*/
    BBOX_Vdc_Resource_eMadr5   =     (1<<10),      /* able to handle transcode 5*/
    BBOX_Vdc_Resource_eHdmi0   =     (1<<11),      /* able to handle HDMI output 0 */
    BBOX_Vdc_Resource_eHdmi1   =     (1<<12),      /* able to handle HDMI output 1 */
    BBOX_Vdc_Resource_eInvalid =     (0xffff)      /* cause acquire to fail */

} BBOX_Vdc_Resource;

#define BBOX_FTR_SD       (0)
#define BBOX_FTR_HD       (BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_M0       (BBOX_Vdc_Resource_eMem0)
#define BBOX_FTR_M1       (BBOX_Vdc_Resource_eMem1)
#define BBOX_FTR_M2       (BBOX_Vdc_Resource_eMem2)
#define BBOX_FTR_M01      (BBOX_Vdc_Resource_eMem0 | BBOX_Vdc_Resource_eMem1)
#define BBOX_FTR_HD_M0    (BBOX_Vdc_Resource_eMem0 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_M1    (BBOX_Vdc_Resource_eMem1 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_M2    (BBOX_Vdc_Resource_eMem2 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_MR0   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr0)
#define BBOX_FTR_HD_MR1   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr1)
#define BBOX_FTR_HD_MR2   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr2)
#define BBOX_FTR_HD_MR3   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr3)
#define BBOX_FTR_HD_MR4   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr4)
#define BBOX_FTR_HD_MR5   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr5)

#define BBOX_FTR_INVALID  (BBOX_Vdc_Resource_eInvalid)
#define BBOX_FTR_DISREGARD BBOX_VDC_DISREGARD
#define BBOX_INVALID_NUM_MEMC 0xFFFFFFFF

/* Macro for RDC memc index table entry */
#define BBOX_MK_RDC_MEMC_IDX(MemcIdxRdc)   (BBOX_MemcIndex_##MemcIdxRdc)

/* Macro for DRAM refresh rate entry */
#define BBOX_MK_DRAM_REFRESH_RATE(rate)   (BBOX_DramRefreshRate_e##rate)

/* Macro for video window capture memc index table entry */
#define BBOX_MK_VID_WIN_CAP_MEMC_IDX(MemcIdxW0, MemcIdxW1)   \
{ \
    BBOX_MemcIndex_##MemcIdxW0, \
    BBOX_MemcIndex_##MemcIdxW1  \
}

/* Macro for video window deinterlacer memc index table entry */
#define BBOX_MK_VID_WIN_MAD_MEMC_IDX(MemcIdxMad0, MemcIdxMad1)   \
{ \
    BBOX_MemcIndex_##MemcIdxMad0, \
    BBOX_MemcIndex_##MemcIdxMad1  \
}

/* Macro for graphics window memc index table entry.
*  G1 and G2 are not used for now */
#define BBOX_MK_GFD_WIN_MEMC_IDX(MemcIdxG0)   \
{ \
    BBOX_MemcIndex_##MemcIdxG0, \
    BBOX_MemcIndex_Invalid,     \
    BBOX_MemcIndex_Invalid      \
}

/* Macro for cmp hdr memc index table entry */
#define BBOX_MK_CMP_CFC_MEMC_IDX(cmp)   BBOX_MemcIndex_##cmp

/* Macro for gfd hdr memc index table entry */
#define BBOX_MK_GFD_CFC_MEMC_IDX(g0)    BBOX_MemcIndex_##g0

/* Macro for HDMI/DVI output hdr memc index table entry */
#define BBOX_MK_DVI_CFC_MEMC_IDX(dvi) \
{ \
    BBOX_MemcIndex_##dvi,             \
    BBOX_MemcIndex_Invalid            \
}

/* Macro for video and graphics window memc index table entry */
#define BBOX_MK_WIN_MEMC_IDX(MemcIdxW0, MemcIdxW1, MemcIdxMad0, MemcIdxMad1, MemcIdxG0)   \
{ \
    BBOX_MK_VID_WIN_CAP_MEMC_IDX(MemcIdxW0, MemcIdxW1),     \
    BBOX_MK_VID_WIN_MAD_MEMC_IDX(MemcIdxMad0, MemcIdxMad1), \
    BBOX_MK_GFD_WIN_MEMC_IDX(MemcIdxG0),                    \
    BBOX_MK_CMP_CFC_MEMC_IDX(Invalid),                      \
    BBOX_MK_GFD_CFC_MEMC_IDX(Invalid)                       \
}

/* Macro for HDR video and graphics display memc index table entry */
#define BBOX_MK_HDR_MEMC_IDX(MemcIdxW0, MemcIdxW1, MemcIdxMad0, MemcIdxMad1, MemcIdxG0, CfcCmp, CfcG0) \
{ \
    BBOX_MK_VID_WIN_CAP_MEMC_IDX(MemcIdxW0, MemcIdxW1),     \
    BBOX_MK_VID_WIN_MAD_MEMC_IDX(MemcIdxMad0, MemcIdxMad1), \
    BBOX_MK_GFD_WIN_MEMC_IDX(MemcIdxG0),                    \
    BBOX_MK_CMP_CFC_MEMC_IDX(CfcCmp),                       \
    BBOX_MK_GFD_CFC_MEMC_IDX(CfcG0)                         \
}

typedef enum
{
    BBOX_Vdc_Resource_Feeder_eMfd0 = 0,
    BBOX_Vdc_Resource_Feeder_eMfd1,
    BBOX_Vdc_Resource_Feeder_eMfd2,
    BBOX_Vdc_Resource_Feeder_eMfd3,
    BBOX_Vdc_Resource_Feeder_eMfd4,
    BBOX_Vdc_Resource_Feeder_eMfd5,
    BBOX_Vdc_Resource_Feeder_eVfd0,
    BBOX_Vdc_Resource_Feeder_eVfd1,
    BBOX_Vdc_Resource_Feeder_eVfd2,
    BBOX_Vdc_Resource_Feeder_eVfd3,
    BBOX_Vdc_Resource_Feeder_eVfd4,
    BBOX_Vdc_Resource_Feeder_eVfd5,
    BBOX_Vdc_Resource_Feeder_eVfd6,
    BBOX_Vdc_Resource_Feeder_eVfd7,
    BBOX_Vdc_Resource_Feeder_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_Resource_Feeder_eUnknown = BBOX_Vdc_Resource_eInvalid
} BBOX_Vdc_Resource_Feeder;

typedef enum
{
    BBOX_Vdc_Resource_Capture_eCap0 = 0,
    BBOX_Vdc_Resource_Capture_eCap1,
    BBOX_Vdc_Resource_Capture_eCap2,
    BBOX_Vdc_Resource_Capture_eCap3,
    BBOX_Vdc_Resource_Capture_eCap4,
    BBOX_Vdc_Resource_Capture_eCap5,
    BBOX_Vdc_Resource_Capture_eCap6,
    BBOX_Vdc_Resource_Capture_eCap7,
    BBOX_Vdc_Resource_Capture_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_Resource_Capture_eUnknown = BBOX_Vdc_Resource_eInvalid
} BBOX_Vdc_Resource_Capture;

typedef enum
{
    BBOX_Vdc_Resource_Scaler_eScl0 = 0,
    BBOX_Vdc_Resource_Scaler_eScl1,
    BBOX_Vdc_Resource_Scaler_eScl2,
    BBOX_Vdc_Resource_Scaler_eScl3,
    BBOX_Vdc_Resource_Scaler_eScl4,
    BBOX_Vdc_Resource_Scaler_eScl5,
    BBOX_Vdc_Resource_Scaler_eScl6,
    BBOX_Vdc_Resource_Scaler_eScl7,
    BBOX_Vdc_Resource_Scaler_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_Resource_Scaler_eUnknown = BBOX_Vdc_Resource_eInvalid
} BBOX_Vdc_Resource_Scaler;


typedef enum
{
    BBOX_Vdc_SclCapBias_eAuto = 0,
    BBOX_Vdc_SclCapBias_eSclBeforeCap,
    BBOX_Vdc_SclCapBias_eSclAfterCap,
    BBOX_Vdc_SclCapBias_eAutoDisable,                    /* Disables CAP when possible and selects SCL placement based on bandwidth */
    BBOX_Vdc_SclCapBias_eAutoDisable1080p,               /* Disables CAP when possible for 'format > 1080p' and selects SCL placement based on bandwidth */
    BBOX_Vdc_SclCapBias_eDisregard = BBOX_VDC_DISREGARD  /* VDC default is selected */
} BBOX_Vdc_SclCapBias;

typedef enum
{
    BBOX_Vdc_Bpp_e8bit,
    BBOX_Vdc_Bpp_e10bit,
    BBOX_Vdc_Bpp_e12bit,
    BBOX_Vdc_Bpp_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_Bpp;

typedef enum
{
    BBOX_Vdc_Colorspace_eRGB = 0,
    BBOX_Vdc_Colorspace_eYCbCr422,
    BBOX_Vdc_Colorspace_eYCbCr444,
    BBOX_Vdc_Colorspace_eYCbCr420,
    BBOX_Vdc_Colorspace_eFuture,
    BBOX_Vdc_Colorspace_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_Colorspace;

/* Mosaic mode class:
 *  BBOX_Vdc_MosaicModeClass_eClass0: mosaic coverage for SD
 *  BBOX_Vdc_MosaicModeClass_eClass1: coverage for 1080p
 *  BBOX_Vdc_MosaicModeClass_eClass2: coverage for 4k
 *  BBOX_Vdc_MosaicModeClass_eClass3: coverage for 4k - 135%
 *  BBOX_Vdc_MosaicModeClass_eClass4: coverage for 4k LPDDR4
 *
 * TODO: fake PIP
 * Note: Enum names may change in the future
 */
typedef enum
{
    BBOX_Vdc_MosaicModeClass_eClass0 = 0,
    BBOX_Vdc_MosaicModeClass_eClass1,
    BBOX_Vdc_MosaicModeClass_eClass2,
    BBOX_Vdc_MosaicModeClass_eClass3,
    BBOX_Vdc_MosaicModeClass_eClass4,
    BBOX_Vdc_MosaicModeClass_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_MosaicModeClass;

typedef enum
{
    BBOX_Vdc_Display_eDisplay0 = 0,
    BBOX_Vdc_Display_eDisplay1,
    BBOX_Vdc_Display_eDisplay2,
    BBOX_Vdc_Display_eDisplay3,
    BBOX_Vdc_Display_eDisplay4,
    BBOX_Vdc_Display_eDisplay5,
    BBOX_Vdc_Display_eDisplay6,
    BBOX_Vdc_Display_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_DisplayId;

typedef enum
{
    BBOX_Vdc_StgCoreId_e0,
    BBOX_Vdc_StgCoreId_e1,
    BBOX_Vdc_StgCoreId_e2,
    BBOX_Vdc_StgCoreId_e3,
    BBOX_Vdc_StgCoreId_e4,
    BBOX_Vdc_StgCoreId_e5,
    BBOX_Vdc_StgCoreId_eInvalid = BBOX_FTR_INVALID
} BBOX_Vdc_StgCoreId;

typedef enum
{
    BBOX_Vdc_EncoderCoreId_e0,
    BBOX_Vdc_EncoderCoreId_e1,
    BBOX_Vdc_EncoderCoreId_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_EncoderCoreId_eInvalid = BBOX_FTR_INVALID
} BBOX_Vdc_EncoderCoreId;

typedef enum
{
    BBOX_Vdc_EncoderChannelId_e0,
    BBOX_Vdc_EncoderChannelId_e1,
    BBOX_Vdc_EncoderChannelId_e2,
    BBOX_Vdc_EncoderChannelId_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_EncoderChannelId_eInvalid = BBOX_FTR_INVALID
} BBOX_Vdc_EncoderChannelId;

typedef enum
{
    BBOX_Vdc_Window_eVideo0 = 0,
    BBOX_Vdc_Window_eVideo1,
    BBOX_Vdc_Window_eGfx0,
    BBOX_Vdc_Window_eGfx1,
    BBOX_Vdc_Window_eGfx2,
    BBOX_Vdc_Window_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_WindowId;

typedef enum
{
    BBOX_Vdc_Deinterlacer_eDeinterlacer0 = 0,
    BBOX_Vdc_Deinterlacer_eDeinterlacer1,
    BBOX_Vdc_Deinterlacer_eDeinterlacer2,
    BBOX_Vdc_Deinterlacer_eDeinterlacer3,
    BBOX_Vdc_Deinterlacer_eDeinterlacer4,
    BBOX_Vdc_Deinterlacer_eDeinterlacer5,
    BBOX_Vdc_Deinterlacer_eDisregard = BBOX_VDC_DISREGARD,
    BBOX_Vdc_Deinterlacer_eInvalid = BBOX_FTR_INVALID
} BBOX_Vdc_DeinterlacerId;

typedef struct
{
    uint32_t                      ulMad;
    bool                          bSrcSideDeinterlacer; /* deinterlacer and SCL are decoupled if true */
    BBOX_Vdc_Resource_Capture     eCap;
    BBOX_Vdc_Resource_Feeder      eVfd;
    BBOX_Vdc_Resource_Scaler      eScl;
} BBOX_Vdc_ResourceFeature;

typedef struct
{
    uint32_t ulWidth;
    uint32_t ulHeight;
} BBOX_Vdc_PictureSizeLimits;

typedef struct
{
    uint32_t ulWidthFraction;       /* this is a fraction of the full-screen width. */
    uint32_t ulHeightFraction;      /* this is a fraction of the full-screen height */
} BBOX_Vdc_WindowSizeLimits;

/***************************************************************************
Summary:
    This structure describes box capabilities for VDC sources. Only GFX and
    HD-DVI sources are supported by BOX's VDC sub-module. MPEG feeders
    would be in BOX's XVD sub-module.

Description:
    bAvailable   - specifies if source is available
    bMtgCapable  - only applies to Mpeg Feeders (MFD) and specifies if MFD
                   is MTG-capable
    bCompressed  - applies to GFD only
    stSizeLimits - specifies the source's frame buffer size
    eColorSpace - specifies source's colorspace
    eBpp - specifies number of bits per pixel
See Also:
***************************************************************************/
typedef struct
{
    bool                       bAvailable;
    bool                       bMtgCapable;
    bool                       bCompressed;
    BBOX_Vdc_PictureSizeLimits stSizeLimits;
    BBOX_Vdc_Colorspace        eColorSpace;
    BBOX_Vdc_Bpp               eBpp;
} BBOX_Vdc_Source_Capabilities;

/***************************************************************************
Summary:
    This structure describes box capabilities for VDC windows.

Description:
    bAvailable   - specifies if window is available
    stSizeLimits - specifies the window size based on a fraction of the
                   full-screen size. For example, 1/2 of full-screen height
                   and 1/2 of full-screen width. To use, simply divide the
                   fullscreen size by this number to determine the
                   maximum size for the given window.
    stAvailableResource - indicates shared resource that is available
    bTntAvailable - indicates if TNT is available for the given window
    bAnrAvailable - indicates if a ANR is available for the given window

See Also:
***************************************************************************/
typedef struct
{
    bool                       bAvailable;
    BBOX_Vdc_WindowSizeLimits  stSizeLimits;
    BBOX_Vdc_ResourceFeature   stResource;
    BBOX_Vdc_SclCapBias        eSclCapBias;
} BBOX_Vdc_Window_Capabilities;

/***************************************************************************
Summary:
    This structure describes the matching of a STG display and an encoder that
    is used with the display

Description:
    bAvailable             - specifies if a STG display and its associated encoder are
                             available
    ulStgId                - the paired STG display
    ulEncoderCoreId        - the paired encoder
    ulEncoderChannel       - the encoder channel to use

See Also:
***************************************************************************/
typedef struct
{
    bool                       bAvailable;
    uint32_t                   ulStgId;
    uint32_t                   ulEncoderCoreId;
    uint32_t                   ulEncoderChannel;
} BBOX_Vdc_StgEncoderPair;

/***************************************************************************
Summary:
    This structure describes box capabilities for VDC displays.

Description:
    bAvailable   - specifies if window is available
    stWindow     - This specifies the capabilities available for a given window
                   of a given display. There are 2 windows per display.
    stStgEnc     - specifies the STG display and encoder pair to the given display
    eMaxVideoFmt - specifies the maximum BVN frame buffer resolution
    eMaxHdmiTxVideoFmt - specifies the maximum display resolution for HDMI display.
                         This may have the same value as eMaxVideoFmt. If different,
                         this indicates that the HDMI display can be scaled up to the
                         specified eMaxHdmiTxVideoFmt. For example, if eMaxVideoFmt
                         is 1080p and eMaxHdmiTxVideoFmt is 3840x2160p, the HDMI
                         display format can be scaled up to 3840x2160p.

See Also:
***************************************************************************/
typedef struct BBOX_Vdc_Display_Capabilities
{
    bool                         bAvailable;
    BBOX_Vdc_Window_Capabilities astWindow[BBOX_VDC_WINDOW_COUNT_PER_DISPLAY];
    BBOX_Vdc_StgEncoderPair      stStgEnc;
    BFMT_VideoFmt                eMaxVideoFmt;
    BBOX_Vdc_MosaicModeClass     eMosaicModeClass;
    BFMT_VideoFmt                eMaxHdmiDisplayFmt;
} BBOX_Vdc_Display_Capabilities;

/***************************************************************************
Summary:
    This structure describes box capabilities for deinterlacers.

Description:
    bAvailable      - Specifies if deinterlacer is available
    stPictureLimits - Indicates if a deinterlacer is limited by the
                      given height and width. BBOX_VDC_DISREGARD means
                      the limits  used are imposed by VDC on a per
                      deinterlacer basis.
    ulHsclThreshold - Indicates the horizontal size threshold that will
                      trigger the use of the HSCL before deinterlacing.
                      In other words, a source with a horizontal size
                      greater or equal to this threshold will be horintally
                      scaled by the HSCL first prior to deinterlacing.
                      BBOX_VDC_DISREGARD  means there is no threshold and
                      therefore no HSCL is used.
See Also:
***************************************************************************/
typedef struct BBOX_Vdc_Deinterlacer_Capabilities
{
    bool                        bAvailable;
    BBOX_Vdc_PictureSizeLimits  stPictureLimits;
    uint32_t                    ulHsclThreshold;
} BBOX_Vdc_Deinterlacer_Capabilities;

/***************************************************************************
Summary:
    This structure describes box capabilities for transcoding.

Description:
    ulNumXcodeCapVfd - Indicates the number of available CAP/VFD for transcode.
                       0 means there is no bandwidth allocated for capture on
                       any transcode path. BBOX_VDC_DISREGARD means all
                       available CAP/VFD can be used.

    ulNumXcodeGfd    - Indicates the number of available GFD for transcode. 0
                       means there is gfx support on any transcode path.
                       BBOX_VDC_DISREGARD means all available GFDs can be used.
See Also:
***************************************************************************/
typedef struct BBOX_Vdc_Xcode_Capabilities
{
    uint32_t ulNumXcodeCapVfd;
    uint32_t ulNumXcodeGfd;
} BBOX_Vdc_Xcode_Capabilities;

/***************************************************************************
Summary:
    List of VDC's exposed capabilities
****************************************************************************/
typedef struct BBOX_Vdc_Capabilities
{
    /* This specifies the capabilities available for a given deinterlacer.
       An element of the array corresponds to a deinterlacer, eg., element 0
       corresponds to deinterlacer 0. */
    BBOX_Vdc_Deinterlacer_Capabilities astDeinterlacer[BBOX_VDC_DEINTERLACER_COUNT];

    /* This specifies the capabilities available for transcoding. */
    BBOX_Vdc_Xcode_Capabilities stXcode;

    /* This specifies the capabilities available for a given display. Each element
       of the array maps directly to a VDC display ID enum. */
    BBOX_Vdc_Display_Capabilities astDisplay[BBOX_VDC_DISPLAY_COUNT];

    /* This specifies the capabilities available for a given source. Note that
       only GFX and HD-DVI have imposed VDC BOX limits. The rest are
       disregarded by VDC. */
    BBOX_Vdc_Source_Capabilities astSource[BAVC_SourceId_eMax];

} BBOX_Vdc_Capabilities;

/***************************************************************************
Summary:
    This structure describes memc index settings for VDC.

Description:

See Also:
    BBOX_MemConfig, BBOX_GetMemConfig, BBOX_Vdc_Display_MemcIndexSettings
****************************************************************************/
typedef struct BBOX_Vdc_MemcIndexSettings
{
    uint32_t     ulRdcMemcIndex;

    /* Memc Index for hdmi display CFC LUT */
    uint32_t     aulHdmiDisplayCfcMemcIndex[BBOX_VDC_HDMI_DISPLAY_COUNT];

    struct {
        /* Memc Index for video windows */
        uint32_t aulVidWinCapMemcIndex[BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY];

        /* Memc Index for deinterlacer on video windows */
        uint32_t aulVidWinMadMemcIndex[BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY];

        /* Memc Index for graphics windows */
        uint32_t aulGfdWinMemcIndex[BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY];

        /* Memc Index for compositor CFC LUT */
        uint32_t ulCmpCfcMemcIndex;

        /* Memc Index for graphics windows CFC LUT */
        uint32_t ulGfdCfcMemcIndex;

    } astDisplay[BBOX_VDC_DISPLAY_COUNT];

} BBOX_Vdc_MemcIndexSettings;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_VDC_H__ */

/* end of file */
