/***************************************************************************
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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVDC_PEP_PRIV_H__
#define BVDC_PEP_PRIV_H__

#include "bstd.h"
#include "bvdc.h"
#include "bdbg.h"
#include "bvdc_hist_priv.h"
#include "bvdc_window_priv.h"

#define BVDC_P_SUPPORT_PEP_VER_0                             (0)
#define BVDC_P_SUPPORT_PEP_VER_1                             (1)
#define BVDC_P_SUPPORT_PEP_VER_2                             (2)
#define BVDC_P_SUPPORT_PEP_VER_3                             (3)
#define BVDC_P_SUPPORT_PEP_VER_4                             (4)
#define BVDC_P_SUPPORT_PEP_VER_5                             (5) /*7366Bx, 7364Ax, 7445D0 10 bit introduction*/

#if (BVDC_P_SUPPORT_PEP)
#include "bchp_pep_cmp_0_v0.h"
#endif

#if(BVDC_P_SUPPORT_HIST_VER >= BVDC_P_SUPPORT_HIST_VER_2)
#include "bchp_hist.h"
#endif

#if (BVDC_P_SUPPORT_TAB)
#include "bchp_tab_0.h"
#endif
#if (BVDC_P_SUPPORT_TNT)
#include "bchp_tnt_cmp_0_v0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_HST);

#if (BVDC_P_SUPPORT_PEP)
#define BVDC_P_CAB_TABLE_SIZE   (BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_END + 1)
#define BVDC_P_LAB_TABLE_SIZE   (BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_END + 1)
#else
#define BVDC_P_CAB_TABLE_SIZE   1  /* hush warnings */
#define BVDC_P_LAB_TABLE_SIZE   1  /* hush warnings */
#endif

#if (BVDC_P_SUPPORT_HIST)
#if (BVDC_P_SUPPORT_HIST_VER >= BVDC_P_SUPPORT_HIST_VER_2)
#define BVDC_P_HISTO_TABLE_SIZE (BCHP_HIST_RD_BINi_ARRAY_END + 1)
#else
#define BVDC_P_HISTO_TABLE_SIZE (BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_END + 1)
#endif
#else
#define BVDC_P_HISTO_TABLE_SIZE 1  /* hush warnings */
#endif

#define BVDC_P_PEP_FIX_FRACTIONAL_SHIFT         20

/* values in 10 bits */
#define BVDC_P_PEP_BLACK_LUMA_VALUE             64
#define BVDC_P_PEP_WHITE_LUMA_VALUE             940
#define BVDC_P_PEP_MAX_LUMA_VALUE               1023

/* This defines the number of points originally calculated for the LAB in the Dyn Cont code */
/* Later expanded to the number of entries in the actual hardware LAB table */
#define BVDC_P_PEP_LAB_GEN_SIZE                 64

#define BVDC_P_PEP_MAX_CAB_SETTING_GRANUALITY   4
#define BVDC_P_PEP_CMS_SAT_MIN_RANGE           -140
#define BVDC_P_PEP_CMS_SAT_MAX_RANGE            140
#define BVDC_P_PEP_CMS_HUE_MIN_RANGE           -50
#define BVDC_P_PEP_CMS_HUE_MAX_RANGE            50
#define BVDC_P_PEP_CMS_COLOR_REGION_NUM         6

#define BVDC_P_PEP_ITOFIX(x) \
    (int32_t)((x) << BVDC_P_PEP_FIX_FRACTIONAL_SHIFT)

#define BVDC_P_PEP_CMS_IS_ENABLE(sat, hue) \
    (((sat)->lGreen   != 0 ) || \
     ((sat)->lYellow  != 0 ) || \
     ((sat)->lRed     != 0 ) || \
     ((sat)->lMagenta != 0 ) || \
     ((sat)->lBlue    != 0 ) || \
     ((sat)->lCyan    != 0 ) || \
     ((hue)->lGreen   != 0 ) || \
     ((hue)->lYellow  != 0 ) || \
     ((hue)->lRed     != 0 ) || \
     ((hue)->lMagenta != 0 ) || \
     ((hue)->lBlue    != 0 ) || \
     ((hue)->lCyan    != 0 ))

#define BVDC_P_PEP_CMS_DISABLE(colorBar) \
    {(colorBar)->lGreen   = 0;  \
     (colorBar)->lYellow  = 0;  \
     (colorBar)->lRed     = 0;  \
     (colorBar)->lMagenta = 0;  \
     (colorBar)->lBlue    = 0;  \
     (colorBar)->lCyan    = 0;}

#define BVDC_P_PEP_CMS_COMPARE_EQ(src, dst) \
    (((src)->lGreen   == (dst)->lGreen  ) && \
     ((src)->lYellow  == (dst)->lYellow ) && \
     ((src)->lRed     == (dst)->lRed    ) && \
     ((src)->lMagenta == (dst)->lMagenta) && \
     ((src)->lBlue    == (dst)->lBlue   ) && \
     ((src)->lCyan    == (dst)->lCyan   ))

#define BVDC_P_PEP_CMS_SAT_WITHIN_RANGE(sat) \
    (((sat)->lGreen   >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lGreen   <= BVDC_P_PEP_CMS_SAT_MAX_RANGE) && \
     ((sat)->lYellow  >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lYellow  <= BVDC_P_PEP_CMS_SAT_MAX_RANGE) && \
     ((sat)->lRed     >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lRed     <= BVDC_P_PEP_CMS_SAT_MAX_RANGE) && \
     ((sat)->lMagenta >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lMagenta <= BVDC_P_PEP_CMS_SAT_MAX_RANGE) && \
     ((sat)->lBlue    >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lBlue    <= BVDC_P_PEP_CMS_SAT_MAX_RANGE) && \
     ((sat)->lCyan    >= BVDC_P_PEP_CMS_SAT_MIN_RANGE) && ((sat)->lCyan    <= BVDC_P_PEP_CMS_SAT_MAX_RANGE))

#define BVDC_P_PEP_CMS_HUE_WITHIN_RANGE(hue) \
    (((hue)->lGreen   >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lGreen   <= BVDC_P_PEP_CMS_HUE_MAX_RANGE) && \
     ((hue)->lYellow  >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lYellow  <= BVDC_P_PEP_CMS_HUE_MAX_RANGE) && \
     ((hue)->lRed     >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lRed     <= BVDC_P_PEP_CMS_HUE_MAX_RANGE) && \
     ((hue)->lMagenta >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lMagenta <= BVDC_P_PEP_CMS_HUE_MAX_RANGE) && \
     ((hue)->lBlue    >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lBlue    <= BVDC_P_PEP_CMS_HUE_MAX_RANGE) && \
     ((hue)->lCyan    >= BVDC_P_PEP_CMS_HUE_MIN_RANGE) && ((hue)->lCyan    <= BVDC_P_PEP_CMS_HUE_MAX_RANGE))

/***************************************************************************
 * PEP private data structures
 ***************************************************************************/
typedef struct BVDC_P_PepContext
{
    BDBG_OBJECT(BVDC_PEP)

    /* private fields. */
    BVDC_P_WindowId                eId;
    BREG_Handle                    hReg;
    bool                           bInitial;
    bool                           bHardStart;
    BVDC_Backlight_CallbackData    stCallbackData;

    bool                           bLoadCabTable;
    bool                           bLoadLabTable;
    bool                           bProcessCab;
    bool                           bProcessHist;
    bool                           bLabTableValid;
    bool                           bLabCtrlPending;

    /* These variables are used in contrast stretch algorithm */
    uint32_t                       aulLastBin[BVDC_P_HISTO_TABLE_SIZE];
    uint32_t                       aulLastLastBin[BVDC_P_HISTO_TABLE_SIZE];
    uint32_t                       aulBin[BVDC_P_HISTO_TABLE_SIZE];

    /* 24.8 notation */
    int32_t                        lFixLastMin;
    int32_t                        lFixLastMax;
    int32_t                        lFixLastMid;
    int32_t                        lFixBrtCur;
    int32_t                        lFixBrtLast;

    /* 16.16 notation */
    int32_t                        lFixEstLuma[BVDC_P_LAB_TABLE_SIZE];
    int32_t                        lFixHist_out[BVDC_P_PEP_LAB_GEN_SIZE];

    int32_t                        alDCTableTemp[BVDC_DC_TABLE_ROWS * BVDC_DC_TABLE_COLS];

    /* These are the output of dynamic contrast stretch algorithm */
    uint32_t                       aulLabTable[BVDC_P_LAB_TABLE_SIZE];

    /* Histogram min and max value */
    uint32_t                       ulHistSize;
    BVDC_LumaStatus                stHistoData;
    /* This is a temp histo data read every vsync */
    BVDC_LumaStatus                stTmpHistoData;
    uint32_t                       ulAvgAPL;
    uint32_t                       ulFiltAPL;
    uint32_t                       ulAvgLevelStats[BVDC_LUMA_HISTOGRAM_LEVELS];
    int32_t                        lLastGain;

    /* sharpness chroma gain, changed by source */
    uint32_t                       ulLumaChromaGain;

    /* These variables are used in CMS algorithm */
    int32_t                        alSatGain[BVDC_P_PEP_CMS_COLOR_REGION_NUM];
    int32_t                        alHueGain[BVDC_P_PEP_CMS_COLOR_REGION_NUM];
    int32_t                        alCr[BVDC_P_CAB_TABLE_SIZE];
    int32_t                        alCb[BVDC_P_CAB_TABLE_SIZE];
    int32_t                        tempCr[BVDC_P_CAB_TABLE_SIZE];
    int32_t                        tempCb[BVDC_P_CAB_TABLE_SIZE];

    uint32_t                       ulPrevHSize;
    uint32_t                       ulPrevVSize;
    BFMT_Orientation               ePrevSrcOrientation;
    BFMT_Orientation               ePrevDispOrientation;

} BVDC_P_PepContext;


/***************************************************************************
 * PEP private functions
 ***************************************************************************/
BERR_Code BVDC_P_Pep_Create
    ( BVDC_P_Pep_Handle           *phPep,
      const BVDC_P_WindowId        eWinId,
      const BREG_Handle            hReg );

void BVDC_P_Pep_Destroy
    ( BVDC_P_Pep_Handle            hPep );

void BVDC_P_Pep_Init
    ( const BVDC_P_Pep_Handle      hPep );

void BVDC_P_Pep_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList,
      bool                         bInitial );

void BVDC_P_Pep_SetInfo_isr
    ( BVDC_P_Pep_Handle            hPep,
      BVDC_P_PictureNode          *pPicture );

void BVDC_P_Tab_BuildRul_isr
    ( const BVDC_Window_Handle     hWindow,
      BVDC_P_ListInfo             *pList );

void BVDC_P_Pep_DynamicContrast_isr
    ( const BVDC_ContrastStretch  *pCS,
      BVDC_P_PepContext           *pPep,
      uint32_t                    *pulLabTable );

BERR_Code BVDC_P_Pep_ComposeCabTable
    ( const uint32_t               ulFleshtone,
      const uint32_t               ulGreenBoost,
      const uint32_t               ulBlueBoost,
      uint32_t                    *pulCabTable );

void BVDC_P_Pep_Cms
    ( BVDC_P_PepContext           *pPep,
      const BVDC_ColorBar         *pSatGain,
      const BVDC_ColorBar         *pHueGain,
      bool                         bIsHd,
      uint32_t                    *pulCabTable );

void BVDC_P_Sharpness_Calculate_Peak_Values
    ( const int16_t                sSharpness,
      uint32_t                    *ulPeakSetting,
      uint32_t                    *ulPeakScale );

void BVDC_P_Sharpness_Calculate_Gain_Value_isr
    ( const int16_t                sSharpness,
      const int16_t                sMinGain,
      const int16_t                sCenterGain,
      const int16_t                sMaxGain,
      uint32_t                    *ulSharpnessGain );

void BVDC_P_Pep_GetLumaStatus
    ( const BVDC_Window_Handle     hWindow,
      BVDC_LumaStatus             *pLumaStatus );

void BVDC_P_Pep_GetRadialTable
    ( uint32_t                     ulColorId,
      bool                         bIsHd,
      const uint16_t             **ppRadialTable );

void BVDC_P_Pep_GetAngleTable
    ( uint32_t                     ulColorId,
      bool                         bIsHd,
      const uint16_t             **ppAngleTable );

void BVDC_P_Pep_GetHueAngleTable
    ( uint32_t                     ulColorId,
      bool                         bIsHd,
      const uint16_t             **ppHueAngleTable );

#if(BVDC_P_SUPPORT_HIST && BVDC_P_SUPPORT_HIST_VER == BVDC_P_SUPPORT_HIST_VER_1)
void BVDC_P_Histo_UpdateHistoData_isr
    ( BVDC_P_Pep_Handle            hPep );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_PEP_PRIV_H__ */
/* End of File */
