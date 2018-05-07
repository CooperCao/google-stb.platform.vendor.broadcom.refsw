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
*
* Module Description:
*
******************************************************************************/
#ifndef BBOX_VDC_PRIV_H__
#define BBOX_VDC_PRIV_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bavc_types.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_P_SRC_IS_MPEG(source_id) \
     (BAVC_SourceId_eMpegMax>=(source_id))

#define BBOX_P_SRC_IS_GFX(source_id) \
    ((BAVC_SourceId_eGfx0<=(source_id)) && \
     (BAVC_SourceId_eGfxMax>=(source_id)))

#define BBOX_P_SRC_IS_HDDVI(source_id) \
    ((BAVC_SourceId_eHdDvi0==(source_id)) || \
     (BAVC_SourceId_eHdDvi1==(source_id)))

#define BBOX_P_SRC_IS_VDEC(source_id) \
    ((BAVC_SourceId_eVdec0<=(source_id)) && \
     (BAVC_SourceId_eVdec1>=(source_id)))

#define BBOX_P_SRC_IS_656IN(source_id) \
    ((BAVC_SourceId_e656In0<=(source_id)) && \
     (BAVC_SourceId_e656In1>=(source_id)))

#define BBOX_P_SRC_IS_DS0(source_id) \
    (BAVC_SourceId_eDs0==(source_id))

#define BBOX_P_SRC_IS_VFD(source_id) \
    ((BAVC_SourceId_eVfd0<=(source_id)) && \
     (BAVC_SourceId_eVfdMax>=(source_id)))

#define BBOX_P_VDC_MTG_DISABLE              0
#define BBOX_P_VDC_MTG_ENABLE               1

void BBOX_P_Vdc_SetSourceLimits
    ( BBOX_Vdc_Source_Capabilities *pSourceCap,
      BAVC_SourceId                 eSourceId,
      uint32_t                      ulMtg,
      uint32_t                      ulWidth,
      uint32_t                      ulHeight,
      BBOX_Vdc_Colorspace           eColorSpace,
      BBOX_Vdc_Bpp                  eBpp,
      bool                          bCompressed,
      BBOX_Vdc_SourceRateLimit      eRate,
      BBOX_Vdc_SourceClass          eClass );

void BBOX_P_Vdc_SetDisplayLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BFMT_VideoFmt                  eMaxVideoFmt,
      BFMT_VideoFmt                  eMaxHdmiDisplayFmt,
      uint32_t                       ulStgId,
      uint32_t                       ulEncoderCoreId,
      uint32_t                       ulEncoderChannel,
      BBOX_Vdc_MosaicModeClass       eMosaicClass );

void BBOX_P_Vdc_SetWindowLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BBOX_Vdc_WindowId              eWinId,
      uint32_t                       ulMad,
      bool                           bSrcSideDeinterlace,
      BBOX_Vdc_Resource_Capture      eCap,
      BBOX_Vdc_Resource_Feeder       eVfd,
      BBOX_Vdc_Resource_Scaler       eScl,
      uint32_t                       ulWinWidthFraction,
      uint32_t                       ulWinHeightFraction,
      BBOX_Vdc_SclCapBias            eSclCapBias,
      BBOX_Vdc_WindowClass           eClass );

void BBOX_P_Vdc_SetDeinterlacerLimits
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap,
      BBOX_Vdc_DeinterlacerId             eId,
      uint32_t                            ulWidth,
      uint32_t                            ulHeight,
      uint32_t                            ulHsclThreshold );

void BBOX_P_Vdc_SetXcodeLimits
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap,
      uint32_t                     ulNumXcodeCapVfd,
      uint32_t                     ulNumXcodeGfd );

void BBOX_P_Vdc_ResetSourceLimits
    ( BBOX_Vdc_Source_Capabilities *pSourceCap,
      BAVC_SourceId                 eSourceId );

void BBOX_P_Vdc_ResetDisplayLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId );

void BBOX_P_Vdc_ResetWindowLimits
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap,
      BBOX_Vdc_DisplayId             eDisplayId,
      BBOX_Vdc_WindowId              eWinId );

void BBOX_P_Vdc_ResetDeinterlacerLimits
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap,
      BBOX_Vdc_DeinterlacerId             eId );

void BBOX_P_Vdc_ResetXcodeLimits
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap );

#define BBOX_P_VDC_SET_LEGACY_SRC_LIMIT( capabilities, src )  BBOX_P_Vdc_SetSourceLimits( \
    capabilities, \
    src, \
    BBOX_P_VDC_MTG_DISABLE, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_Vdc_Colorspace_eDisregard, \
    BBOX_Vdc_Bpp_eDisregard, \
    false, \
    BBOX_Vdc_SourceRateLimit_e60Hz, \
    BBOX_Vdc_SourceClass_eLegacy)

#define BBOX_P_VDC_SET_LEGACY_DISPLAY_LIMIT( capabilities, display, mosaicClass ) BBOX_P_Vdc_SetDisplayLimits( \
    capabilities, \
    display, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_FTR_INVALID, \
    BBOX_FTR_INVALID, \
    BBOX_FTR_INVALID, \
    BBOX_Vdc_MosaicModeClass_e##mosaicClass )

#define BBOX_P_VDC_SET_LEGACY_WINDOW_LIMIT( capabilities, display, window )  BBOX_P_Vdc_SetWindowLimits( \
    capabilities, \
    display, \
    window, \
    BBOX_VDC_DISREGARD, \
    false, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_Vdc_SclCapBias_eDisregard, \
    BBOX_Vdc_WindowClass_eLegacy)

#define BBOX_P_VDC_SET_LEGACY_DEINTERLACER_LIMIT( capabilities, id )   BBOX_P_Vdc_SetDeinterlacerLimits( \
    capabilities, \
    id, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD )

#define BBOX_P_VDC_SET_LEGACY_XCODE_LIMIT( capabilities  )  BBOX_P_Vdc_SetXcodeLimits( \
    capabilities, \
    BBOX_VDC_DISREGARD, \
    BBOX_VDC_DISREGARD )

#define BBOX_P_VDC_SET_SRC_CLASS_LIMIT( capabilities, src, mtg, width, height, colorspace, bpp, compressed, rate, eClass ) BBOX_P_Vdc_SetSourceLimits( \
    capabilities, \
    BAVC_SourceId_e##src, \
    BBOX_P_VDC_##mtg, \
    width, \
    height, \
    BBOX_Vdc_Colorspace_e##colorspace, \
    BBOX_Vdc_Bpp_e##bpp, \
    compressed, \
    BBOX_Vdc_SourceRateLimit_e##rate, \
    BBOX_Vdc_SourceClass_e##eClass )

#define BBOX_P_VDC_SET_SRC_LIMIT( capabilities, src, mtg, width, height, colorspace, bpp, compressed ) \
    BBOX_P_VDC_SET_SRC_CLASS_LIMIT( capabilities, src, mtg, width, height, colorspace, bpp, compressed, 60Hz, Legacy )

#define BBOX_P_VDC_SET_DISPLAY_LIMIT( capabilities, id, fmt, hdmiFmt, stgId, encId, encChan, mosaicClass )  BBOX_P_Vdc_SetDisplayLimits( \
    capabilities, \
    BBOX_Vdc_Display_e##id, \
    BFMT_VideoFmt_e##fmt, \
    BFMT_VideoFmt_e##hdmiFmt, \
    BBOX_Vdc_StgCoreId_e##stgId, \
    BBOX_Vdc_EncoderCoreId_e##encId, \
    BBOX_Vdc_EncoderChannelId_e##encChan, \
    BBOX_Vdc_MosaicModeClass_e##mosaicClass )

#define BBOX_P_VDC_SET_WINDOW_CLASS_LIMIT( capabilities, displayId, winId, mad, srcSideDeinterlace, cap, vfd, scl, width, height, sclCapBias, eClass )  BBOX_P_Vdc_SetWindowLimits( \
    capabilities, \
    BBOX_Vdc_Display_e##displayId, \
    BBOX_Vdc_Window_e##winId, \
    BBOX_FTR_##mad, \
    srcSideDeinterlace, \
    BBOX_Vdc_Resource_Capture_e##cap, \
    BBOX_Vdc_Resource_Feeder_e##vfd, \
    BBOX_Vdc_Resource_Scaler_e##scl, \
    width, \
    height, \
    BBOX_Vdc_SclCapBias_e##sclCapBias, \
    BBOX_Vdc_WindowClass_e##eClass)

#define BBOX_P_VDC_SET_WINDOW_LIMIT( capabilities, displayId, winId, mad, srcSideDeinterlace, cap, vfd, scl, width, height, sclCapBias ) \
    BBOX_P_VDC_SET_WINDOW_CLASS_LIMIT( capabilities, displayId, winId, mad, srcSideDeinterlace, cap, vfd, scl, width, height, sclCapBias, Legacy )

#define BBOX_P_VDC_SET_DEINTERLACER_LIMIT( capabilities, id, width, height, hsclThreshold )  BBOX_P_Vdc_SetDeinterlacerLimits( \
    capabilities, \
    BBOX_Vdc_Deinterlacer_e##id, \
    width, \
    height, \
    hsclThreshold )

#define BBOX_P_VDC_SET_XCODE_LIMIT( capabilities, numCapVfd, numGfd )  BBOX_P_Vdc_SetXcodeLimits( \
    capabilities, \
    numCapVfd, \
    numGfd )

#define BBOX_P_VDC_RESET_SRC_LIMIT( capabilities, src ) BBOX_P_Vdc_ResetSourceLimits( \
    capabilities, \
    BAVC_SourceId_e##src )

#define BBOX_P_VDC_RESET_DISPLAY_LIMIT( capabilities, id )  BBOX_P_Vdc_ResetDisplayLimits( \
    capabilities, \
    BBOX_Vdc_Display_e##id )

#define BBOX_P_VDC_RESET_WINDOW_LIMIT( capabilities, displayId, winId )  BBOX_P_Vdc_ResetWindowLimits( \
    capabilities, \
    BBOX_Vdc_Display_e##displayId, \
    BBOX_Vdc_Window_e##winId )

#define BBOX_P_VDC_RESET_DEINTERLACER_LIMIT( capabilities, id )  BBOX_P_Vdc_ResetDeinterlacerLimits( \
    capabilities, \
    BBOX_Vdc_Deinterlacer_e##id )

#define BBOX_P_VDC_RESET_XCODE_LIMIT( capabilities )  BBOX_P_Vdc_ResetXcodeLimits( \
    capabilities )


void BBOX_P_Vdc_SetDefaultCapabilities
    ( BBOX_Vdc_Capabilities *pBoxVdc );

void BBOX_P_Vdc_SetCapabilities
    ( uint32_t               ulBoxId,
      BBOX_Vdc_Capabilities *pBoxVdc );

void BBOX_P_Vdc_SetSourceCapabilities
    ( uint32_t                      ulBoxId,
      BBOX_Vdc_Source_Capabilities *pSourceCap );

void BBOX_P_Vdc_SetDisplayCapabilities
    ( uint32_t                       ulBoxId,
      BBOX_Vdc_Display_Capabilities *pDisplayCap );

void BBOX_P_Vdc_SetDeinterlacerCapabilities
    ( uint32_t                            ulBoxId,
      BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap );

void BBOX_P_Vdc_SetXcodeCapabilities
    ( uint32_t                     ulBoxId,
      BBOX_Vdc_Xcode_Capabilities *pXcodeCap );

BERR_Code BBOX_P_GetRtsConfig
    ( const uint32_t         ulBoxId,
      BBOX_Rts              *pBoxRts );



#ifdef __cplusplus
}
#endif

#endif/* BBOX_VDC_PRIV_H__ */
