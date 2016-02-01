/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef bxdm_pp_FRD_H__
#define bxdm_pp_FRD_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

#define BXDM_PPFRD_P_MAX_DELTAPTS_SAMPLES 180
#define BXDM_PPFRD_P_MAX_NUMELEMENTS_SAMPLES 60

typedef struct BXDM_PPFRD_P_Stats
{
   BXDM_PPFP_P_DataType astDeltaPTS[BXDM_PPFRD_P_MAX_DELTAPTS_SAMPLES]; /* Array of deltaPTS values */
   BXDM_PPFP_P_DataType stDeltaPTSRunningSum; /* Running sum of deltaPTS values in array */
   uint32_t uiDeltaPTSCount; /* Number of valid deltaPTS values in the array */
   uint32_t uiDeltaPTSIndex; /* Index of next deltaPTS entry */

   uint32_t auiNumElements[BXDM_PPFRD_P_MAX_NUMELEMENTS_SAMPLES]; /* Array of numElements values */
   uint32_t uiNumElementsRunningSum; /* Running sum of numElements */
   uint32_t uiNumPicturesCount; /* Number of valid numElements values in the array */
   uint32_t uiNumElementsIndex; /* Index of next numElements entry */

   uint32_t uiLastPTS;
   bool bLastPTSValid;
   uint32_t uiPicturesSinceLastValidPTS;

   BAVC_FrameRateCode eLastReportedStableFrameRate;
   BAVC_FrameRateCode eLastCalculatedFrameRate;
   uint32_t uiNumPicturesCalculatedFrameRateWasStable;
} BXDM_PPFRD_P_Stats;

BERR_Code BXDM_PPFRD_P_AddPTS_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   uint32_t uiPTS,
   bool bPTSValid,
   uint32_t uiNumElements
   );

BERR_Code BXDM_PPFRD_P_GetFrameRate_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_ClockRate eClockRate,
   BXDM_PictureProvider_FrameRateDetectionMode eFRDMode,
   BAVC_FrameRateCode *peFrameRate
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_FRD_H__ */
