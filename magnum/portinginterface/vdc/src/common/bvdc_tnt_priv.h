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
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef BVDC_TNT_PRIV_H__
#define BVDC_TNT_PRIV_H__

#include "bstd.h"
#include "bdbg.h"
#include "bvdc_tnt.h"

#include "bvdc_common_priv.h"

#ifdef __cplusplus
     extern "C" {
#endif

#if (BVDC_P_SUPPORT_TNT_VER == 5)            /* TNT2 HW base */

#define BVDC_P_CHROMA_SHARPNESS_FORMAT_POINTS       2 /* Cr/Cb or Hue/Sat */
#define BVDC_P_MAX_CORING_THRESHOLD_SCALE_FACTORS   3

typedef struct
{
    uint32_t    aulRegionConfig[BVDC_MAX_CHROMA_SHARPNESS_REGIONS * BVDC_P_CHROMA_SHARPNESS_FORMAT_POINTS];
    uint32_t    aulPwl[BVDC_MAX_CHROMA_SHARPNESS_REGIONS * BVDC_MAX_CHROMA_SHARPNESS_PWL * BVDC_MAX_CHROMA_SHARPNESS_PWL_POINTS];
    uint32_t    aulPwlInput[BVDC_MAX_CHROMA_SHARPNESS_REGIONS];
    uint32_t    aulGainAdj[BVDC_MAX_CHROMA_SHARPNESS_REGIONS];
    uint32_t    aulColorOffset[BVDC_MAX_CHROMA_SHARPNESS_REGIONS];
} BVDC_P_ChromaSharpnessRegionSettings;


typedef struct
{
    uint32_t                             aulLumaPeakingGain[BVDC_MAX_LUMA_PEAKING_FREQ_BANDS];
    uint32_t                             aulLoBandScaleFactor[BVDC_P_MAX_CORING_THRESHOLD_SCALE_FACTORS];
    uint32_t                             aulHiBandScaleFactor[BVDC_P_MAX_CORING_THRESHOLD_SCALE_FACTORS];
    uint32_t                             aulLtiScaleFactor[BVDC_P_MAX_CORING_THRESHOLD_SCALE_FACTORS];
    bool                                 bChromaRegionCorrectionEnable;
    bool                                 abChromaSharpnessRegionEnable[BVDC_MAX_CHROMA_SHARPNESS_REGIONS];
    BVDC_P_ChromaSharpnessRegionSettings stChromaSharpnessRegionConfig;
} BVDC_P_SharpnessData;

BERR_Code BVDC_P_Tnt_ValidateSharpnessSettings
    ( const BVDC_SharpnessSettings       *pstSettings );

void BVDC_P_Tnt_StoreSharpnessSettings
    ( BVDC_Window_Handle                  hWindow,
      const BVDC_SharpnessSettings       *pstSettings );

BERR_Code BVDC_P_Tnt_InterpolateSharpness
    ( BVDC_Window_Handle                  hWindow,
      const int16_t                       sSharpness);


#endif /* (BVDC_P_SUPPORT_TNT_VER == 5) */

void BVDC_P_Tnt_BuildInit_isr
    ( BVDC_Window_Handle                  hWindow,
      BVDC_P_ListInfo                    *pList );

void BVDC_P_Tnt_BuildRul_isr
    ( BVDC_Window_Handle                  hWindow,
      BVDC_P_ListInfo                    *pList );

void BVDC_P_Tnt_BuildVysncRul_isr
    ( BVDC_Window_Handle                  hWindow,
      BVDC_P_ListInfo                    *pList );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_TNT_PRIV_H__*/

/* End of file. */
