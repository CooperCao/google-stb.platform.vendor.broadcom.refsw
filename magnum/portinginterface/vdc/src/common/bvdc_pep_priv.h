/***************************************************************************
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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVDC_PEP_PRIV_H__
#define BVDC_PEP_PRIV_H__

#include "bstd.h"
#include "bvdc.h"
#include "bdbg.h"
#include "bvdc_window_priv.h"

#if (BVDC_P_SUPPORT_HIST)
#include "bchp_pep_cmp_0_v0.h"
#endif

#if (BVDC_P_SUPPORT_TNT)
#include "bchp_tnt_cmp_0_v0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_HST);

#if (BVDC_P_SUPPORT_HIST)
#if BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_END
#define BVDC_P_HISTO_TABLE_SIZE (BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_END + 1)
#else
#define BVDC_P_HISTO_TABLE_SIZE 1  /* hush warnings */
#endif
#else
#define BVDC_P_HISTO_TABLE_SIZE 1  /* hush warnings */
#endif

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

    /* Histogram min and max value */
    uint32_t                       ulHistSize;
    BVDC_LumaStatus                stHistoData;
    /* This is a temp histo data read every vsync */
    BVDC_LumaStatus                stTmpHistoData;
    uint32_t                       ulAvgAPL;
    uint32_t                       ulFiltAPL;
    uint32_t                       ulAvgLevelStats[BVDC_LUMA_HISTOGRAM_LEVELS];
    int32_t                        lLastGain;

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

void BVDC_P_Sharpness_Calculate_Gain_Value_isr
    ( const int16_t                sSharpness,
      const int16_t                sMinGain,
      const int16_t                sCenterGain,
      const int16_t                sMaxGain,
      uint32_t                    *ulSharpnessGain );

#if(BVDC_P_SUPPORT_HIST)
void BVDC_P_Histo_UpdateHistoData_isr
    ( BVDC_P_Pep_Handle            hPep );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_PEP_PRIV_H__ */
/* End of File */
