/******************************************************************************
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
 ******************************************************************************/
#ifndef BVDC_CFC_PRIV_H__
#define BVDC_CFC_PRIV_H__

#include "bcfc.h"
#include "bvdc.h"
#include "bvdc_common_priv.h"
#if BVDC_P_DBV_SUPPORT
#include "bvdc_cfc_dbv_priv.h"
#endif
#if BVDC_P_TCH_SUPPORT
#include "bvdc_cfc_tch_priv.h"
#endif
#include "bmth_fix.h"
#include "bmth_fix_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BVDC_P_CFC_NEED_BLEND_MATRIX(hCompositor) \
    ((hCompositor)->abBlenderUsed[0] && \
     (hCompositor)->stCfcCapability[0].stBits.bCscBlendOut && \
     BCFC_IS_HDR10((hCompositor)->stOutColorSpaceExt.stColorSpace.eColorTF))

/* -------------------------------------------------------------------
 * default for TCH display brightness
 */
#define BVDC_P_TCH_DEFAULT_HDR_BRIGHTNESS 1000
#define BVDC_P_TCH_DEFAULT_SDR_BRIGHTNESS  100

/* -------------------------------------------------------------------
 * macros that avoid to change auto-generated table files
 */
#define BVDC_P_LR_XY_F_BITS       BCFC_P_LR_XY_F_BITS
#define BVDC_P_MAKE_LR_ADJ        BCFC_MAKE_LR_ADJ
#define BVDC_P_TfConvRamLuts      BCFC_TfConvRamLuts
#define BVDC_P_CfcLutLoadListInfo BCFC_LutLoadListInfo
#define BVDC_P_RamLut             BCFC_RamLut
#define BVDC_P_CfcLRangeAdj       BCFC_LRangeAdj
#define BVDC_P_LRangeAdjTable     BCFC_LRangeAdjTable

/* --------------------------------------------------------------------
 * cfc meta data from decoded video content for dbv or tch
 */
typedef union BVDC_P_CfcMetaData
{
    BAVC_HdrMetadataType eType;

    /* DBV support */
  #if BVDC_P_DBV_SUPPORT
    BVDC_P_DBV_Input stDbvInput;
  #endif
  #if BVDC_P_TCH_SUPPORT
    BVDC_P_TCH_Input stTchInput;
  #endif
} BVDC_P_CfcMetaData;

/* --------------------------------------------------------------------
 * PWL table for dbv/tch background color nl2l&l2nl adjustment:
 */
#define BVDC_P_PWL_XY_I_BITS          (1)
#define BVDC_P_PWL_XY_F_BITS         (24)
#define BVDC_P_PWL_SLP_M_I_BITS       (0)
#define BVDC_P_PWL_SLP_M_F_BITS      (24)
#define BVDC_P_PWL_SLP_E_I_BITS       (4)
#define BVDC_P_PWL_SLP_E_F_BITS       (0)

#define BVDC_P_MAKE_PWL_XY(x, y) \
    {BMTH_FIX_SIGNED_FTOFIX(x, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS), \
     BMTH_FIX_SIGNED_FTOFIX(y, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)}

#define BVDC_P_MAKE_PWL_SLOPE(m,e) \
    {BMTH_FIX_SIGNED_FTOFIX(m, BVDC_P_PWL_SLP_M_I_BITS, BVDC_P_PWL_SLP_M_F_BITS), \
     BMTH_FIX_SIGNED_FTOFIX(e, BVDC_P_PWL_SLP_E_I_BITS, BVDC_P_PWL_SLP_E_F_BITS)}

#define BVDC_P_MAKE_NL_PWL(num_pts,                              \
                           x0, y0, mantissa0, exp0,               \
                           x1, y1, mantissa1, exp1,               \
                           x2, y2, mantissa2, exp2,               \
                           x3, y3, mantissa3, exp3,               \
                           x4, y4, mantissa4, exp4,               \
                           x5, y5, mantissa5, exp5,               \
                           x6, y6, mantissa6, exp6,               \
                           x7, y7, mantissa7, exp7)               \
{                                                                 \
    num_pts,                                                      \
    {                                                             \
        BVDC_P_MAKE_PWL_XY(x0, y0),                               \
        BVDC_P_MAKE_PWL_XY(x1, y1),                               \
        BVDC_P_MAKE_PWL_XY(x2, y2),                               \
        BVDC_P_MAKE_PWL_XY(x3, y3),                               \
        BVDC_P_MAKE_PWL_XY(x4, y4),                               \
        BVDC_P_MAKE_PWL_XY(x5, y5),                               \
        BVDC_P_MAKE_PWL_XY(x6, y6),                               \
        BVDC_P_MAKE_PWL_XY(x7, y7)                                \
    },                                                            \
    {                                                             \
        BVDC_P_MAKE_PWL_SLOPE(mantissa0, exp0),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa1, exp1),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa2, exp2),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa3, exp3),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa4, exp4),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa5, exp5),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa6, exp6),                   \
        BVDC_P_MAKE_PWL_SLOPE(mantissa7, exp7)                    \
    }                                                             \
}

typedef struct BVDC_P_NL_PwlSegments
{
    unsigned num;/* number of points/segments */
    struct {
        uint32_t x;/* U1.24 */
        uint32_t y;/* U1.24 */
    } point[BCFC_LR_ADJ_PTS];
    struct
    {
        int32_t man;/* S0.24 */
        int32_t exp;/* S4.0 */
    } slope[BCFC_LR_ADJ_PTS];
} BVDC_P_NL_PwlSegments;

/* --------------------------------------------------------------------*/
/* integer to fixed */
#define BVDC_P_CFC_ITOFIX(x) \
    BMTH_FIX_SIGNED_ITOFIX(x, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS)

#define BVDC_P_CFC_FIX_MAX_BITS       (63)
#define BVDC_P_CFC_FIX_FRACTION_BITS  (BCFC_CSC_SW_CX_F_BITS)
#define BVDC_P_CFC_FIX_INT_BITS       (BVDC_P_CFC_FIX_MAX_BITS - BVDC_P_CFC_FIX_FRACTION_BITS)

#define BVDC_P_MAKE_CFC_CX_FR_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS)
#define BVDC_P_MAKE_CFC_CO_FR_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift, BCFC_CSC_SW_CO_I_BITS, BCFC_CSC_SW_CO_F_BITS)

#define BVDC_P_MAKE_CFC_CX_TO_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift)
#define BVDC_P_MAKE_CFC_CO_TO_USR(m, usr_shift) \
    BMTH_FIX_SIGNED_CONVERT_isrsafe(m, BCFC_CSC_SW_CO_I_BITS, BCFC_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_MAX_BITS - usr_shift, usr_shift)

/* --------------------------------------------------------------------
 * Color clamp in DVI CFC
 */
typedef struct
{
    uint16_t                    ulYMin;
    uint16_t                    ulYMax;
    uint16_t                    ulCbMin;
    uint16_t                    ulCbMax;
    uint16_t                    ulCrMin;
    uint16_t                    ulCrMax;
} BVDC_P_CscClamp;

/* Configure a CFC according to its input and output color space
 *
 */
void BVDC_P_Cfc_UpdateCfg_isr
    ( BCFC_Context        *pCfc,
      bool                 bMosaicMode,
      bool                 bForceDirty);

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)

void BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(
    const BCFC_LRangeAdj         *pLRangeAdj,
    uint32_t                      ulStartReg,
    BVDC_P_ListInfo              *pList);

/* this function could be called to build CMP/GFD/VEC LRange adjust
 */
void BVDC_P_Cfc_BuildRulForLRAdj_isr(
    const BCFC_LRangeAdj         *pLRangeAdj,
    bool                          bDispAdaptation,
    int16_t                       sHdrPeakBrightness,
    uint32_t                      ulStartReg,
    BVDC_P_ListInfo              *pList);

/* Build LUT RUL for RAM NL2L/L2NL/LMR loading and RDC RUL for usage control
 */
void BVDC_P_Cfc_BuildRulForRamLut_isr
    ( const BCFC_RamLut          *pRamLut,
      uint32_t                    ulStartReg,
      uint32_t                    ulLutId,
      BCFC_LutLoadListInfo       *pLutList,
      BVDC_P_ListInfo            *pList);

/* Build RDC RUL for ram LUT loading
 */
void BVDC_P_Cfc_BuildRulForLutLoading_isr
    ( BCFC_LutLoadListInfo       *pLutList,
      uint32_t                    ulAddrReg, /* *_LUT_DESC_ADDR */
      uint32_t                    ulCfgReg, /* *_LUT_DESC_CFG */
      BVDC_P_ListInfo            *pList);

#if BVDC_P_DBV_SUPPORT
void BVDC_P_Dbv_UpdateVideoInputColorSpace_isr(
    BCFC_ColorSpace              *pColorSpace,
    const BAVC_MVD_Field         *pMvdFieldData );
void BVDC_P_Dbv_UpdateGfxInputColorSpace_isr(
    BVDC_Compositor_Handle        hCompositor,
    const BCFC_ColorSpace      *pColorSpace );
bool BVDC_P_Display_DbvGetInfoFrame_isr(
    BVDC_Display_Handle           hDisplay,
    BAVC_HDMI_DRMInfoFrameType1  *pInfoFrame);
#endif

#if BVDC_P_TCH_SUPPORT
void BVDC_P_Tch_UpdateVideoInputColorSpace_isr(
    BVDC_Compositor_Handle        hCompositor,
    BCFC_ColorSpace              *pColorSpace,
    const BAVC_MVD_Field         *pMvdFieldData );
#endif

#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

void BVDC_P_Cfc_FromMatrix_isr
    ( BCFC_Context                    *pCfc,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift );

void BVDC_P_Cfc_ToMatrix_isr
    ( int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BCFC_Csc3x4               *pCsc,
      uint32_t                         ulShift );

void BVDC_P_Cfc_GetCfcToApplyAttenuationRGB_isr
    ( BCFC_Colorimetry                 eColorimetry,
      const BCFC_Csc3x4              **ppYCbCrToRGB,
      const BCFC_Csc3x4              **ppRGBToYCbCr );

BERR_Code BVDC_P_Cfc_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      BCFC_Csc3x4                     *pCscCoeffs );
uint32_t BVDC_P_Compositor_Update_Canvas_Background_isrsafe
    ( BVDC_Compositor_Handle           hCompositor,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue);

void BVDC_P_Cfc_ApplyContrast_isr
    ( int16_t                          sContrast,
      BCFC_Csc3x4                     *pCscCoeffs );

void BVDC_P_Cfc_ApplyBrightness_isr
    ( int16_t                          sBrightness,
      BCFC_Csc3x4                     *pCscCoeffs );

void BVDC_P_Cfc_ApplySaturationAndHue_isr
    ( int16_t                          sSaturation,
      int16_t                          sHue,
      BCFC_Csc3x4                     *pCscCoeffs );

void BVDC_P_Cfc_ApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BCFC_Csc3x4                     *pCscCoeffs,
      const BCFC_Csc3x4               *pYCbCrToRGB,
      const BCFC_Csc3x4               *pRGBToYCbCr,
      bool                             bUserCsc,
      void                            *pTmpBuf );

void BVDC_P_Cfc_DvoApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BCFC_Csc3x4                     *pCscCoeffs );

void BVDC_P_Cfc_ApplyYCbCrColor_isr
    ( BCFC_Csc3x4                     *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2,
      void                            *pTmpBuf );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_CFC_PRIV_H__ */
/* End of file. */
