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

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "berr.h"
#include "bxdm_pp_fp.h"
#include "bxdm_pp_avg.h"

BDBG_MODULE(BXDM_PPAVG);

typedef struct BXDM_PPAVG_P_Context
{
   BXDM_PPAVG_P_Settings stSettings;

   uint32_t uiSampleCount;
   uint32_t uiSampleIndex;
   BXDM_PPFP_P_DataType *astSample;
   BXDM_PPFP_P_DataType stCumulativeSum;
} BXDM_PPAVG_P_Context;

static const BXDM_PPAVG_P_Settings s_stAvgDefaultSettings =
{
   120, /* Samples */
};

BERR_Code
BXDM_PPAVG_P_GetDefaultSettings(
   BXDM_PPAVG_P_Settings *pstAvgSettings
   )
{
   BDBG_ENTER(BXDM_PPAVG_P_GetDefaultSettings);

   BDBG_ASSERT(pstAvgSettings);

   *pstAvgSettings = s_stAvgDefaultSettings;

   BDBG_LEAVE(BXDM_PPAVG_P_GetDefaultSettings);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
BXDM_PPAVG_P_Create(
   BXDM_PPAVG_P_Handle *phAvg,
   const BXDM_PPAVG_P_Settings *pAvgSettings
   )
{
   BXDM_PPAVG_P_Context *pAvgHandle = NULL;

   BDBG_ENTER(BXDM_PPAVG_P_Create);

   BDBG_ASSERT(phAvg);
   BDBG_ASSERT(pAvgSettings);
   BDBG_ASSERT(pAvgSettings->uiNumSamples);

   /* Set handle to NULL in case the allocation fails */
   *phAvg = NULL;

   /* Allocate AVG Handle */
   pAvgHandle = (BXDM_PPAVG_P_Context *) BKNI_Malloc(sizeof(BXDM_PPAVG_P_Context));
   if (!pAvgHandle)
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* Zero out the newly allocated context */
   BKNI_Memset((void*)pAvgHandle, 0, sizeof(BXDM_PPAVG_P_Context));

   /* Allocate value queue */
   pAvgHandle->stSettings = *pAvgSettings;

   pAvgHandle->astSample = (BXDM_PPFP_P_DataType *) BKNI_Malloc(sizeof(BXDM_PPFP_P_DataType) * pAvgHandle->stSettings.uiNumSamples);
   if (!pAvgHandle->astSample)
   {
      BXDM_PPAVG_P_Destroy(pAvgHandle);
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
   BKNI_Memset((void*)pAvgHandle->astSample, 0, (sizeof(BXDM_PPFP_P_DataType) * pAvgHandle->stSettings.uiNumSamples));

   *phAvg = pAvgHandle;

   BDBG_LEAVE(BXDM_PPAVG_P_Create);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
BXDM_PPAVG_P_Destroy(
   BXDM_PPAVG_P_Handle hAvg
   )
{
   BDBG_ENTER(BXDM_PPAVG_P_Destroy);

   BDBG_ASSERT(hAvg);

   if ( hAvg->astSample )
   {
      BKNI_Free(hAvg->astSample);
      hAvg->astSample = NULL;
   }

   BKNI_Free(hAvg);

   BDBG_LEAVE(BXDM_PPAVG_P_Destroy);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
BXDM_PPAVG_P_AddValueFixedPoint_isrsafe(
   BXDM_PPAVG_P_Handle hAvg,
   const BXDM_PPFP_P_DataType *pstValue
   )
{
   BDBG_ENTER(BXDM_PPAVG_P_AddValueFixedPoint_isrsafe);

   BDBG_ASSERT(hAvg);
   BDBG_ASSERT(pstValue);

   /* Compute Cumulative Sum */
   if ( hAvg->uiSampleCount < hAvg->stSettings.uiNumSamples )
   {
      /* Increment Sample Count */
      hAvg->uiSampleCount++;
   }
   else
   {
      /* Subtract oldest value from running sum */
      BXDM_PPFP_P_FixPtSub_isr(
               &hAvg->stCumulativeSum,
               &hAvg->astSample[hAvg->uiSampleIndex],
               &hAvg->stCumulativeSum);
   }

   hAvg->astSample[hAvg->uiSampleIndex] = *pstValue;

   /* Update running sum */
   BXDM_PPFP_P_FixPtAdd_isr(
            &hAvg->stCumulativeSum,
            &hAvg->astSample[hAvg->uiSampleIndex],
            &hAvg->stCumulativeSum);

   /* Increment error index */
   hAvg->uiSampleIndex++;
   if ( hAvg->uiSampleIndex >= hAvg->stSettings.uiNumSamples )
   {
      hAvg->uiSampleIndex = 0;
   }

   BDBG_LEAVE(BXDM_PPAVG_P_AddValueFixedPoint_isrsafe);

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BXDM_PPAVG_P_AddValueInteger_isrsafe(
   BXDM_PPAVG_P_Handle hAvg,
   int32_t iValue
   )
{
   BERR_Code rc;
   BXDM_PPFP_P_DataType stValue;

   BDBG_ENTER(BXDM_PPAVG_P_AddValueInteger_isrsafe);

   BDBG_ASSERT(hAvg);

   stValue.uiWhole = (uint32_t) iValue;
   stValue.uiFractional = 0;

   rc = BXDM_PPAVG_P_AddValueFixedPoint_isrsafe(
      hAvg,
      &stValue
      );

   BDBG_LEAVE(BXDM_PPAVG_P_AddValueInteger_isrsafe);

   return BERR_TRACE( rc );
}

BERR_Code
BXDM_PPAVG_P_Reset_isrsafe(
   BXDM_PPAVG_P_Handle hAvg
   )
{
   BDBG_ENTER(BXDM_PPAVG_P_Reset_isrsafe);

   BDBG_ASSERT(hAvg);

   hAvg->uiSampleCount = 0;
   hAvg->uiSampleIndex = 0;
   BKNI_Memset((void*)hAvg->astSample, 0, (sizeof(BXDM_PPFP_P_DataType) * hAvg->stSettings.uiNumSamples));

   hAvg->stCumulativeSum.uiWhole = 0;
   hAvg->stCumulativeSum.uiFractional = 0;

   BDBG_LEAVE(BXDM_PPAVG_P_Reset_isrsafe);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
BXDM_PPAVG_P_GetAverage_isrsafe(
   BXDM_PPAVG_P_Handle hAvg,
   BXDM_PPFP_P_DataType *pstAverage,
   uint32_t *puiNumSamples
   )
{
   BDBG_ENTER( BXDM_PPAVG_P_GetAverage_isrsafe );

   BDBG_ASSERT( hAvg );
   BDBG_ASSERT( pstAverage );

   if ( NULL != puiNumSamples )
   {
      *puiNumSamples = hAvg->uiSampleCount;
   }

   if ( 0 == hAvg->uiSampleCount )
   {
      pstAverage->uiWhole = 0;
      pstAverage->uiFractional = 0;
   }
   else
   {
      bool bNegative = false;

      /* Compute Average */
      *pstAverage = hAvg->stCumulativeSum;

      if ( (int32_t) pstAverage->uiWhole < 0 )
      {
         bNegative = true;
         pstAverage->uiWhole = (uint32_t) (- ((int32_t) pstAverage->uiWhole));
      }

      BXDM_PPFP_P_FixPtDiv_isr(
         pstAverage,
         hAvg->uiSampleCount,
         pstAverage);

      if ( true == bNegative )
      {
         pstAverage->uiWhole = (uint32_t) (- ((int32_t) pstAverage->uiWhole));
      }
   }

   BDBG_LEAVE( BXDM_PPAVG_P_GetAverage_isrsafe );

   return BERR_TRACE( BERR_SUCCESS );
}
