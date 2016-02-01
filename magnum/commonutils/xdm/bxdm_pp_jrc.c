/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "berr.h"
#include "bxdm_pp_jrc.h"
#include "bxdm_pp_fix33.h"

#if BXDM_PPJRC_P_DUMP
#include <stdio.h>
#endif

BDBG_MODULE(BXDM_PPJRC);

/* The jitter correction (JRC) module corrects jitter in incoming values
 * by using a linear best fit algorithm.  The module uses the expected delta
 * to facilitate the fitting.  An estimated PTS value is calculated using the
 * initial PTS sample.  An average bias is calculated by computing the difference
 * between the estimated PTS and the actual PTS.  The average bias is then added
 * to the estimated PTS and used as the corrected PTS.
 *
 * Mathematically, the algorithm can be described as:
 * (Based on work by Richard Lee - rrlee@broadcom.com)
 *
 * D == precise differences between PTS (a.k.a frame time)
 * P[n] == nth delivered PTS (+/- 1 ms)
 * T[n] == nth interpolated PTS
 *      = P[0] + n*D
 * E[n] == nth bias between actual and interpolated PTS
 *      = P[n] - T[n]
 * B[n] == nth average bias
 *      = avg(E[n] ... E[0])
 * C[n] == nth correct PTS
 *      = T[n] + B[n]
 */

typedef struct BXDM_PPJRC_P_Context
{
   BXDM_PPJRC_P_Settings stSettings;

   bool bSeeded;

   uint32_t uiBiasCount;
   uint32_t uiBiasIndex;
   BXDM_PP_Fix33_t *afixBias;
   BXDM_PP_Fix33_t fixCumulativeBias;

   BXDM_PP_Fix33_t fixPreviousActualValueP;
   BXDM_PP_Fix33_t fixPreviousCorrectedValueC;
   BXDM_PP_Fix33_t fixDeltaStep;
   uint32_t uiStepCount;

   BXDM_PP_Fix33_t fixExpectedValueT;

   uint32_t uiTotalCount;
#if BXDM_PPJRC_P_DUMP
   FILE *fDump;
#endif
} BXDM_PPJRC_P_Context;

static const BXDM_PPJRC_P_Settings s_stJrcDefaultSettings =
{
 120, /* Samples */
 1,  /* Lower Threshold */
 45  /* Upper Threshold */
};

BERR_Code BXDM_PPJRC_P_GetDefaultSettings(
   BXDM_PPJRC_P_Settings *pstJrcSettings
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_GetDefaultSettings);

   BDBG_ASSERT(pstJrcSettings);

   *pstJrcSettings = s_stJrcDefaultSettings;

   BDBG_LEAVE(BXDM_PPJRC_P_GetDefaultSettings);

   return BERR_TRACE(BERR_SUCCESS);
}

/* Create the XVD JRC Handle */
BERR_Code BXDM_PPJRC_P_Create(
   BXDM_PPJRC_P_Handle *phJrc,
   const BXDM_PPJRC_P_Settings *pJrcSettings
   )
{
   BXDM_PPJRC_P_Context *pJrcHandle = NULL;

   BDBG_ENTER(BXDM_PPJRC_P_Create);

   BDBG_ASSERT(phJrc);
   BDBG_ASSERT(pJrcSettings);
   BDBG_ASSERT(pJrcSettings->uiQueueDepth);

   /* Set handle to NULL in case the allocation fails */
   *phJrc = NULL;

   /* Allocate JRC Handle */
   pJrcHandle = (BXDM_PPJRC_P_Context *) BKNI_Malloc(sizeof(BXDM_PPJRC_P_Context));
   if (!pJrcHandle)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pJrcHandle, 0, sizeof(BXDM_PPJRC_P_Context));

   /* Allocate value queue */
   pJrcHandle->stSettings = *pJrcSettings;

   pJrcHandle->afixBias = (BXDM_PP_Fix33_t *) BKNI_Malloc(sizeof(BXDM_PP_Fix33_t) * pJrcHandle->stSettings.uiQueueDepth);
   if (!pJrcHandle->afixBias)
   {
      BXDM_PPJRC_P_Destroy(pJrcHandle);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
   BKNI_Memset((void*)pJrcHandle->afixBias, 0, (sizeof(BXDM_PP_Fix33_t) * pJrcHandle->stSettings.uiQueueDepth));

   *phJrc = pJrcHandle;

   BDBG_LEAVE(BXDM_PPJRC_P_Create);

   return BERR_TRACE(BERR_SUCCESS);
}

/* Destroy the XVD JRC Handle */
BERR_Code BXDM_PPJRC_P_Destroy(
   BXDM_PPJRC_P_Handle hJrc
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_Destroy);

   BDBG_ASSERT(hJrc);

#if BXDM_P_PPJRC_DUMP
   if ( NULL != hJrc->fDump )
   {
      fclose(hJrc->fDump);
      hJrc->fDump = NULL;
   }
#endif

   if ( hJrc->afixBias )
   {
      BKNI_Free(hJrc->afixBias);
      hJrc->afixBias = NULL;
   }

   BKNI_Free(hJrc);

   BDBG_LEAVE(BXDM_PPJRC_P_Destroy);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXDM_PPJRC_P_Reset_isrsafe(
   BXDM_PPJRC_P_Handle hJrc
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_Reset_isrsafe);

   BDBG_ASSERT(hJrc);

   hJrc->bSeeded = false;

   hJrc->uiBiasCount = 0;
   hJrc->uiBiasIndex = 0;
   BKNI_Memset((void*)hJrc->afixBias, 0, (sizeof(BXDM_PP_Fix33_t) * hJrc->stSettings.uiQueueDepth));
   hJrc->fixCumulativeBias = 0;

   hJrc->uiTotalCount = 0;

   BDBG_LEAVE(BXDM_PPJRC_P_Reset_isrsafe);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code BXDM_PPJRC_P_AddValue_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiCurrentValue,
   const BXDM_PPFP_P_DataType *pstExpectedDelta,
   uint32_t uiStepCount,
   uint32_t *puiJitterCorrectedValue
   )
{
   BXDM_PP_Fix33_t fixExpectedDeltaD;

   BXDM_PP_Fix33_t fixActualValueP;
   BXDM_PP_Fix33_t fixCorrectedValueC;
   BXDM_PP_Fix33_t fixCorrection;

   BXDM_PP_Fix33_t fixBiasE = 0;
   BXDM_PP_Fix33_t fixAverageBiasB = 0;

   BDBG_ENTER(BXDM_PPJRC_P_AddValue_isrsafe);
   BDBG_ASSERT(hJrc);
   BDBG_ASSERT(pstExpectedDelta);

   {
      BXDM_PP_Fix33_t fixExpectedDeltaStep = BXDM_PP_Fix33_from_mixedfraction(
            pstExpectedDelta->uiWhole,
            pstExpectedDelta->uiFractional,
            BXDM_PictureProvider_P_FixedPoint_FractionalOverflow
            );

      if ( fixExpectedDeltaStep != hJrc->fixDeltaStep )
      {
         BDBG_MSG(("RESET - Delta Changed (0x%lu --> 0x%lu)",
                   (unsigned long)hJrc->fixDeltaStep,
                   (unsigned long)fixExpectedDeltaStep
                  ));
         BXDM_PPJRC_P_Reset_isrsafe(hJrc);
         hJrc->fixDeltaStep = fixExpectedDeltaStep;
      }
   }

   /* Store original value as fixed point (P[n]) */
   fixActualValueP = BXDM_PP_Fix33_from_uint32(uiCurrentValue);

   /* Compute Delta Value Expected (D = DeltaStep * StepCount)*/
   fixExpectedDeltaD= BXDM_PP_Fix33_mulu(hJrc->fixDeltaStep, hJrc->uiStepCount);

   /* Compute interpolated PTS (T[n] = P[0] + n*D) */
   if ( false == hJrc->bSeeded )
   {
      hJrc->fixExpectedValueT = fixActualValueP;
      hJrc->bSeeded = true;
   }
   else
   {
      hJrc->fixExpectedValueT = BXDM_PP_Fix33_add( hJrc->fixExpectedValueT, fixExpectedDeltaD );
   }

   /* Compute the error bias (E[n] = P[n] - T[n]) */
   fixBiasE = BXDM_PP_Fix33_sub(fixActualValueP, hJrc->fixExpectedValueT);

   if ( !( ( BXDM_PP_Fix33_to_int32(fixBiasE) <= (int32_t) hJrc->stSettings.uiJitterUpperThreshold )
             && ( BXDM_PP_Fix33_to_int32(fixBiasE) >= -(int32_t)hJrc->stSettings.uiJitterUpperThreshold ) )
           )
   {
      BXDM_PPJRC_P_Reset_isrsafe(hJrc);
      BDBG_MSG(("RESET[0] - Beyond Jitter Threshold (%d)", BXDM_PP_Fix33_to_int32(fixBiasE)));
      fixCorrectedValueC = fixActualValueP;
   }
   else
   {
      fixCorrectedValueC = hJrc->fixExpectedValueT;
   }

   /* Compute the average bias (B[n] = avg(E[n] ... E[0]))*/
   if ( hJrc->uiBiasCount < hJrc->stSettings.uiQueueDepth )
   {
      /* Increment Bias Count */
      hJrc->uiBiasCount++;
   }
   else
   {
      /* Subtract oldest value from running sum */
      hJrc->fixCumulativeBias = BXDM_PP_Fix33_sub(hJrc->fixCumulativeBias, hJrc->afixBias[hJrc->uiBiasIndex]);
   }

   hJrc->afixBias[hJrc->uiBiasIndex] = fixBiasE ;

   /* Update running sum */
   hJrc->fixCumulativeBias = BXDM_PP_Fix33_add(hJrc->fixCumulativeBias, hJrc->afixBias[hJrc->uiBiasIndex]);

   /* Increment Bias index */
   hJrc->uiBiasIndex++;
   if ( hJrc->uiBiasIndex >= hJrc->stSettings.uiQueueDepth )
   {
      hJrc->uiBiasIndex = 0;
   }

   /* Adjust corrected value by the Average Bias */
   if ( hJrc->uiBiasCount > 0 )
   {
      fixAverageBiasB = BXDM_PP_Fix33_divu(hJrc->fixCumulativeBias, hJrc->uiBiasCount);
      fixCorrectedValueC = BXDM_PP_Fix33_add(fixCorrectedValueC, fixAverageBiasB);
   }

   /* Compute the overall correction (CorrectedValue - ActualValue) */
   fixCorrection = BXDM_PP_Fix33_sub(fixCorrectedValueC, fixActualValueP);

   if ( ( BXDM_PP_Fix33_to_int32(fixCorrection) <= (int32_t) hJrc->stSettings.uiJitterLowerThreshold )
        && ( BXDM_PP_Fix33_to_int32(fixCorrection) >= -(int32_t) hJrc->stSettings.uiJitterLowerThreshold )
      )
   {
      fixCorrectedValueC = fixActualValueP;
      fixCorrection = 0;
   }
   else if ( !( ( BXDM_PP_Fix33_to_int32(fixCorrection) <= (int32_t) hJrc->stSettings.uiJitterUpperThreshold )
                && ( BXDM_PP_Fix33_to_int32(fixCorrection) >= -(int32_t)hJrc->stSettings.uiJitterUpperThreshold ) )
           )
   {
      BXDM_PPJRC_P_Reset_isrsafe(hJrc);
      BDBG_MSG(("RESET[1] - Beyond Jitter Threshold (%d 0x%lu)", BXDM_PP_Fix33_to_int32(fixCorrection), (unsigned long)fixCorrection));
      fixCorrectedValueC = fixActualValueP;
      fixAverageBiasB = 0;
      fixCorrection = 0;
   }

   hJrc->uiTotalCount++;

   {
#if BDBG_DEBUG_BUILD
      BXDM_PP_Fix33_t fixActualDelta = BXDM_PP_Fix33_sub(fixActualValueP, hJrc->fixPreviousActualValueP);
      BXDM_PP_Fix33_t fixActualDeltaError = BXDM_PP_Fix33_sub(fixActualDelta, fixExpectedDeltaD);
      BXDM_PP_Fix33_t fixCorrectedDelta = BXDM_PP_Fix33_sub(fixCorrectedValueC, hJrc->fixPreviousCorrectedValueC);
#endif

      BDBG_MSG(("%03d: (Pn:%08x (dPn:%08x edPn:%08x err:%3d)) (Tn:%08x En:%3d) (Bn:%3d) --> (Cn:%08x (dCn:%08x (%3d))",
               hJrc->uiTotalCount % 1000,
               BXDM_PP_Fix33_to_uint32(fixActualValueP),
               BXDM_PP_Fix33_to_uint32(fixActualDelta),
               BXDM_PP_Fix33_to_uint32(fixExpectedDeltaD),
               BXDM_PP_Fix33_to_int32(fixActualDeltaError),
               BXDM_PP_Fix33_to_uint32(hJrc->fixExpectedValueT),
               BXDM_PP_Fix33_to_int32(fixBiasE),
               BXDM_PP_Fix33_to_int32(fixAverageBiasB),
               BXDM_PP_Fix33_to_uint32(fixCorrectedValueC),
               BXDM_PP_Fix33_to_uint32(fixCorrectedDelta),
               BXDM_PP_Fix33_to_int32(fixCorrection)
               ));

#if BXDM_PPJRC_P_DUMP
      {
         if ( NULL == hJrc->fDump )
         {
            hJrc->fDump = fopen("BXDM_PPJRC.csv", "wb");

            if ( NULL != hJrc->fDump )
            {
               fprintf(hJrc->fDump,"Pn,dPn,edPn,err,Tn,En,Bn,Cn,dCn,corr\n");
            }
         }

         if ( NULL != hJrc->fDump )
         {
            fprintf(hJrc->fDump, "%u,%u,%u,%d,%u,%u,%d,%u,%u,%d\n",
                     BXDM_PP_Fix33_to_uint32(fixActualValueP),
                     BXDM_PP_Fix33_to_uint32(fixActualDelta),
                     BXDM_PP_Fix33_to_uint32(fixExpectedDeltaD),
                     BXDM_PP_Fix33_to_int32(fixActualDeltaError),
                     BXDM_PP_Fix33_to_uint32(hJrc->fixExpectedValueT),
                     BXDM_PP_Fix33_to_int32(fixBiasE),
                     BXDM_PP_Fix33_to_int32(fixAverageBiasB),
                     BXDM_PP_Fix33_to_uint32(fixCorrectedValueC),
                     BXDM_PP_Fix33_to_uint32(fixCorrectedDelta),
                     BXDM_PP_Fix33_to_int32(fixCorrection)
                    );
         }
      }
#endif
   }

   hJrc->fixPreviousCorrectedValueC = fixCorrectedValueC;
   hJrc->fixPreviousActualValueP = fixActualValueP;
   hJrc->uiStepCount = uiStepCount;
   *puiJitterCorrectedValue = BXDM_PP_Fix33_to_uint32(fixCorrectedValueC);

   BDBG_LEAVE(BXDM_PPJRC_P_AddValue_isrsafe);

   return BERR_TRACE(BERR_SUCCESS);
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PPJRC_P_GetLowerThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t *puiLowerThreshold
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_GetLowerThreshold_isrsafe);
   BDBG_ASSERT(hJrc);
   BDBG_ASSERT(puiLowerThreshold);

   *puiLowerThreshold = hJrc->stSettings.uiJitterLowerThreshold;

   BDBG_LEAVE(BXDM_PPJRC_P_GetLowerThreshold_isrsafe);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif

BERR_Code BXDM_PPJRC_P_SetLowerThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiLowerThreshold
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_SetLowerThreshold_isrsafe);
   BDBG_ASSERT(hJrc);

   hJrc->stSettings.uiJitterLowerThreshold = uiLowerThreshold;

   BDBG_LEAVE(BXDM_PPJRC_P_SetLowerThreshold_isrsafe);
   return BERR_TRACE(BERR_SUCCESS);
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXDM_PPJRC_P_GetUpperThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t *puiUpperThreshold
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_GetUpperThreshold_isrsafe);
   BDBG_ASSERT(hJrc);
   BDBG_ASSERT(puiUpperThreshold);

   *puiUpperThreshold = hJrc->stSettings.uiJitterUpperThreshold;

   BDBG_LEAVE(BXDM_PPJRC_P_GetUpperThreshold_isrsafe);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif

BERR_Code BXDM_PPJRC_P_SetUpperThreshold_isrsafe(
   BXDM_PPJRC_P_Handle hJrc,
   uint32_t uiUpperThreshold
   )
{
   BDBG_ENTER(BXDM_PPJRC_P_SetUpperThreshold_isrsafe);
   BDBG_ASSERT(hJrc);

   hJrc->stSettings.uiJitterUpperThreshold = uiUpperThreshold;

   BDBG_LEAVE(BXDM_PPJRC_P_SetUpperThreshold_isrsafe);
   return BERR_TRACE(BERR_SUCCESS);
}
