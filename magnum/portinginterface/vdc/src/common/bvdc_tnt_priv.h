/***************************************************************************
 *	   Copyright (c) 2003-2012, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 *
 * Revision History:
 *
 * $brcm_Log: $
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

