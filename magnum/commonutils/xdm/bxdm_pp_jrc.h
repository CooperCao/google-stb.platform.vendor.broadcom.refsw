/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef bxdm_pp_JRC_H_
#define bxdm_pp_JRC_H_

#include "bxdm_pp_fp.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

typedef struct BXDM_PPJRC_P_Context *BXDM_PPJRC_P_Handle;

typedef struct BXDM_PPJRC_P_Settings
{
   uint32_t uiQueueDepth;
   uint32_t uiJitterLowerThreshold;
   uint32_t uiJitterUpperThreshold;
} BXDM_PPJRC_P_Settings;

BERR_Code BXDM_PPJRC_P_GetDefaultSettings(
   BXDM_PPJRC_P_Settings *pstJrcSettings
   );

/* Create the XVD JRC Handle */
BERR_Code BXDM_PPJRC_P_Create(
   BXDM_PPJRC_P_Handle *phJrc,
   const BXDM_PPJRC_P_Settings *pJrcSettings
   );

/* Destroy the XVD JRC Handle */
BERR_Code BXDM_PPJRC_P_Destroy(
   BXDM_PPJRC_P_Handle hJrc
   );

BERR_Code BXDM_PPJRC_P_Reset_isrsafe(
   BXDM_PPJRC_P_Handle hJrc
   );

BERR_Code BXDM_PPJRC_P_AddValue_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiOriginalValue,
   const BXDM_PPFP_P_DataType *pstExpectedDelta,
   uint32_t  uiStepCount,
   uint32_t *puiJitterCorrectedValue
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PPJRC_P_GetLowerThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t *puiLowerThreshold
   );
#endif

BERR_Code BXDM_PPJRC_P_SetLowerThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiLowerThreshold
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PPJRC_P_GetUpperThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t *puiUpperThreshold
   );
#endif

BERR_Code BXDM_PPJRC_P_SetUpperThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiUpperThreshold
   );

#ifdef __cplusplus
}
#endif

#endif /* bxdm_pp_JRC_H_ */
