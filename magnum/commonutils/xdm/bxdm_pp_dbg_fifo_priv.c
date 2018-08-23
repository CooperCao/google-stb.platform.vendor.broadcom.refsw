/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_dbg_fifo_priv.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_dbg_common.h"


BDBG_MODULE(BXDM_PPDBG_FIFO_PRIV);


#if BDBG_DEBUG_BUILD

static void BXDM_PPDFIFO_S_GetDefaultMetaData(
   const BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_P_DebugFifo_Entry *pstEntry
   )
{
   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstEntry );

   /* TODO: memset the entire structure? */

   BKNI_Memset( &pstEntry->stMetadata, 0, sizeof( BXDM_DebugFifo_Metadata ));
   pstEntry->stMetadata.uiInstanceID = hXdmPP->stDMConfig.uiInstanceID;
   pstEntry->stMetadata.uiVsyncCount = hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount;

   pstEntry->stMetadata.uiPPBIndex = -1;
   if ( NULL != pstPicture && true == pstPicture->bValidated )
   {
      pstEntry->stMetadata.uiPPBIndex = pstPicture->stPicParms.uiPPBIndex;
   }

   if ( NULL != hXdmPP->hTimer )
   {
      BTMR_ReadTimer_isr( hXdmPP->hTimer, &pstEntry->stMetadata.uiTimestamp );
   }

   return;
}


BERR_Code BXDM_PPDFIFO_P_QueDBG_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_DebugFifo_DebugInfo * pstDebugInfo
   )
{
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pstDebugInfo );

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      goto done;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      goto done;
   }

   BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, NULL, pstEntry );
   pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eDebugInfo;

   pstEntry->data.stDebugInfo = *pstDebugInfo;

   BDBG_Fifo_CommitBuffer( &stToken );

done:

   return BERR_SUCCESS;

}

void BXDM_PPDFIFO_P_QueMFD_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture* pMFDPicture
   )
{
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueMFD_isr );

   BDBG_ASSERT( hXdmPP );
   BDBG_ASSERT( pLocalState );
   BDBG_ASSERT( pMFDPicture );

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      goto done;
   }

   while ( NULL != pMFDPicture )
   {
      pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

      if ( NULL == pstEntry )
      {
         goto done;
      }

      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, &hXdmPP->stDMState.stChannel.stSelectedPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eMFD;

      pstEntry->data.stMFD = *pMFDPicture;
      BDBG_Fifo_CommitBuffer( &stToken );

      pMFDPicture = pMFDPicture->pEnhanced;
   }

done:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueMFD_isr );

   return;

}

/*
 * Routines for dealing with Unified Pictures.
 */

void BXDM_PPDFIFO_P_QueUnifiedPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   )
{
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;
   const BXDM_Picture * pstUnified;
   const BXDM_Picture * pstSelectedUnified;

   bool  bPrintAdditional;

   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BSTD_UNUSED(pLocalState);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueUnifiedPicture_isr );

   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);
   pstSelectedUnified = pstSelectedPicture->pstUnifiedPicture;

   pstUnified = pstPicture->pstUnifiedPicture;

   /* SW7405-4736: now that this routine is called from BXDM_PP_S_SelectPicture, there is a
    * need to verify that the unified picture is still available.  In the scenario where a
    * picture is being redisplayed while subsequent ones are dropped, "pstSelectedUnified"
    * will be valid and "pstUnified" can be NULL.  This can occur when playing a DVD clip
    * where video data continues after the clip stop time.  The picture just prior to the
    * clip stop time will be repeated, the pictures after the clip stop time will be dropped.
    */
   if ( NULL == pstUnified )
   {
      goto Done;
   }

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      goto Done;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      goto Done;
   }

   {
      BXDM_DebugFifo_UnifiedPicture * pUniP = (BXDM_DebugFifo_UnifiedPicture *)&pstEntry->data.stUniPic;

      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, pstPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eUnifiedPicture;

      pUniP->stUnifiedPicture = *pstUnified;

      /* Print once per second or when any of the parameters change. */

      bPrintAdditional = ( hXdmPP->stDMState.stDecode.uiVsyncCountQM >= BXDM_P_VsyncsPerSecondLUT[ hXdmPP->stDMConfig.eMonitorRefreshRate ] );

      if ( pstSelectedUnified && true == pstSelectedPicture->bValidated )
      {
         bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiWidth != pstSelectedUnified->stBufferInfo.stSource.uiWidth ) ;
         bPrintAdditional |= ( pstUnified->stBufferInfo.stSource.uiHeight != pstSelectedUnified->stBufferInfo.stSource.uiHeight ) ;
         bPrintAdditional |= ( pstUnified->stProtocol.eProtocol != pstSelectedUnified->stProtocol.eProtocol ) ;
         bPrintAdditional |= ( pstUnified->stBufferInfo.eBufferFormat != pstSelectedUnified->stBufferInfo.eBufferFormat ) ;
         bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiNumerator != pstSelectedUnified->stFrameRate.stRate.uiNumerator ) ;
         bPrintAdditional |= ( pstUnified->stFrameRate.stRate.uiDenominator != pstSelectedUnified->stFrameRate.stRate.uiDenominator ) ;
         bPrintAdditional |= ( pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD != pstSelectedPicture->stPicParms.stTSM.stStatic.eFrameRateXVD ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.eAspectRatio != pstSelectedUnified->stAspectRatio.eAspectRatio ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioX != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioX ) ;
         bPrintAdditional |= ( pstUnified->stAspectRatio.uiSampleAspectRatioY != pstSelectedUnified->stAspectRatio.uiSampleAspectRatioY ) ;
         bPrintAdditional |= ( hXdmPP->stDMStatus.stCurrentPTS.uiSwPcrOffset != hXdmPP->stDMConfig.uiSoftwarePCROffset );
         bPrintAdditional |= ( hXdmPP->stDMConfig.uiDirtyBits_1 & BXDM_PictureProvider_P_DIRTY_1_DISPLAY_FIELD_MODE );
      }
      else
      {
         /* Results in printing the parameters for the first picture after start decode. */
         bPrintAdditional = true;
      }

      pUniP->bPrintAdditional = bPrintAdditional;

      if ( true == bPrintAdditional )
      {
         if ( 0 !=  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount )
         {
            /* determine Average dPTS (using FRD parameters) ... */
            BXDM_PPFP_P_FixPtDiv_isr(
                  &hXdmPP->stDMState.stDecode.stFRDStats.stDeltaPTSRunningSum,
                  hXdmPP->stDMState.stDecode.stFRDStats.uiDeltaPTSCount,
                  &pUniP->stDeltaPTSAvg
                  );

            /* Convert the fractional part of stDeltaPTSAvg to a base 10 value for display. */
            BXDM_PPFP_P_FixPtBinaryFractionToBase10_isr( &pUniP->stDeltaPTSAvg, 2, &pUniP->uiAverageFractionBase10 );
         }
         else
         {
            pUniP->stDeltaPTSAvg.uiFractional = 0;
            pUniP->stDeltaPTSAvg.uiWhole = 0;
            pUniP->uiAverageFractionBase10 = 0;
         }

         pUniP->eFrameRate = pstPicture->stPicParms.stTSM.stStatic.eFrameRateXVD;
         pUniP->eFrameRateType = pstPicture->stPicParms.stTSM.stStatic.eFrameRateType;
         pUniP->uiSwPcrOffsetUsedForEvaluation = pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;
         pUniP->eDisplayFieldMode = hXdmPP->stDMConfig.eDisplayFieldMode;
         pUniP->ePictureDisplayFieldMode = pstPicture->stPicParms.stDisplay.stDynamic.eDisplayFieldMode;
         pUniP->bForceSingleFieldMode = pstPicture->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode;

      }     /* end of if ( bPrintAdditional )*/


      /* SW7445-586: use accessor method to retrieve pulldown */
      BXDM_PPQM_P_GetPicturePulldown_isr( pstPicture, &pUniP->ePulldown );

      if ( pUniP->ePulldown >= BXDM_Picture_PullDown_eMax ) pUniP->ePulldown = 0;

      switch ( pstPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
      {
         case BXDM_PictureProvider_PictureHandlingMode_eHonorPTS:    pUniP->cSelectionMode = 'h';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS:   pUniP->cSelectionMode = 'i';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eDrop:        pUniP->cSelectionMode = 'd';   break;
         case BXDM_PictureProvider_PictureHandlingMode_eWait:
            pUniP->cSelectionMode = (BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode) ? 'w' : 'v' ;
            break;

         case BXDM_PictureProvider_PictureHandlingMode_eDefault:
         default:
            if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
            {
               pUniP->cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bEvaluatedWithSwStc ) ? 's' : 't';
            }
            else
            {
               pUniP->cSelectionMode = ( pstPicture->stPicParms.stTSM.stDynamic.bSelectedInPrerollMode ) ? 'p' : 'v' ;
            }
            break;
      }

      /* A picture-less picture, it will simply be dropped. */
      if ( false == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
      {
         pUniP->cSelectionMode = 'x';
      }

      /* SWSTB-439: if the error threshold is non-zero, use "uiPercentError" from the picture structure
       * to determine if the picture has an error.  Otherwise use the error flag. */

      if ( 0 != hXdmPP->stDMConfig.uiErrorThreshold )
      {
         pUniP->cErrorOnThisPicture = ( pstUnified->stError.uiPercentError == 0 ) ? ' ' :
                                       ( pstUnified->stError.uiPercentError >= hXdmPP->stDMConfig.uiErrorThreshold ) ? 'e' : 'p';
      }
      else
      {
         pUniP->cErrorOnThisPicture = ( pstUnified->stError.bThisPicture == true )  ? 'E' : ' ';
      }


      pUniP->eSelectionMode = pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode;
      pUniP->eTsmResult = pstPicture->stPicParms.stTSM.stDynamic.eTsmResult;
      pUniP->bAppendedToPreviousPicture = pstPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture;

      BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &pUniP->uiDisplayOffset );

      pUniP->iStcPtsDifferenceActual = pstPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceActual;


      if ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode )
      {
         BXDM_PPQM_P_GetHwPcrOffset_isr( hXdmPP, pstPicture, &pUniP->uiPcrOffset );

         pUniP->uiPcrOffset += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;


         if (( false == hXdmPP->stDMConfig.bUseHardwarePCROffset )
               || ( true == hXdmPP->stDMConfig.bPlayback))
         {
            pUniP->bPcrOffsetValid = true;
            pUniP->bPcrOffsetDiscontinuity = false;
         }
         else
         {
            pUniP->bPcrOffsetValid = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded;
            pUniP->bPcrOffsetDiscontinuity = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded;
         }

         pUniP->iPTSJitterCorrection = pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection;
         pUniP->uiStcSnapshot = pLocalState->uiStcSnapshot;
         pUniP->iStcJitterCorrectionOffset = hXdmPP->stDMState.stDecode.iStcJitterCorrectionOffset;

      }
      else
      {

         BXDM_PPVTSM_P_VirtualStcGet_isr( hXdmPP, &pUniP->uiVirtualStc );

         pUniP->uiVirtualStc += pstPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;

         BXDM_PPQM_P_GetPtsWithFrac_isr(
               pstPicture,
               BXDM_PictureProvider_P_PTSIndex_eVirtual,
               pstPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement,
               &pUniP->stVirtualPts
               );
      }

      BDBG_Fifo_CommitBuffer( &stToken );

   }

Done:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueUnifiedPicture_isr );

   return;

}  /* end of BXDM_PPDFIFO_P_QueUnifiedPicture_isr */



void BXDM_PPDFIFO_P_QueDMConfig_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   bool bLastCall
   )
{
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueDMConfig_isr );

   BSTD_UNUSED(pLocalState);
   BSTD_UNUSED(hXdmPP);

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      goto done;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      goto done;
   }

   BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, NULL, pstEntry );
   pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eConfig;

   pstEntry->data.stConfigInfo.bLastCall = bLastCall;
   pstEntry->data.stConfigInfo.stConfig = hXdmPP->stDMConfig;

   BDBG_Fifo_CommitBuffer( &stToken );

done:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueDMConfig_isr );

   return;

}  /* end of BXDM_PPDFIFO_P_PrintDMConfig */


/*
 * SWSTB-1380: routines for managing the debug fifo's
 */

BERR_Code BXDM_PPDFIFO_P_Fifo_Create(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BERR_Code rc = BERR_SUCCESS;

   bool bAllocationError = false;

   BDBG_Fifo_CreateSettings stCreateSettings;

   BDBG_ENTER( BXDM_PPDFIFO_P_Fifo_Create );

   pstDebugFifo->uiElementSize = sizeof( BXDM_P_DebugFifo_Entry );
   pstDebugFifo->uiFifoSize = BXDM_P_MAX_DEBUG_FIFO_COUNT * sizeof( BXDM_P_DebugFifo_Entry );

   /*
    * Allocate the buffer for the debug fifo.
    */
   pstDebugFifo->hBMMABlock = BMMA_Alloc(
      pstDebugFifo->hBMMAHeap,
      pstDebugFifo->uiFifoSize,
      1024,                               /* what was done for VCE, i.e. BVCE_P_DEFAULT_ALIGNMENT */
      NULL
      );

   if ( NULL == pstDebugFifo->hBMMABlock )
   {
      BDBG_ERR(("%s:: BMMA_Alloc failure", BSTD_FUNCTION ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   /*
    * Get the physical address.
    */
   pstDebugFifo->bmmaOffset = BMMA_LockOffset( pstDebugFifo->hBMMABlock );

   /* will this be needed?
     * hBuffer->bOffsetValid = true; */

   if ( 0 == pstDebugFifo->bmmaOffset )
   {
      BDBG_ERR(("%s:: BMMA_LockOffset failure", BSTD_FUNCTION ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   /*
    * Get the virtual address.
    */
   pstDebugFifo->pBuffer = BMMA_Lock( pstDebugFifo->hBMMABlock );

   if ( 0 == pstDebugFifo->pBuffer )
   {
      BDBG_ERR(("%s:: BMMA_Lock failure", BSTD_FUNCTION ));
      bAllocationError = true;
      rc = BERR_OUT_OF_DEVICE_MEMORY;
      goto error;
   }

   pstDebugFifo->bBufferValid = true;

   /*
    * Create the debug FIFO
    */
   BDBG_Fifo_GetDefaultCreateSettings( &stCreateSettings );

   stCreateSettings.elementSize = pstDebugFifo->uiElementSize;
   stCreateSettings.bufferSize = pstDebugFifo->uiFifoSize;
   stCreateSettings.buffer = pstDebugFifo->pBuffer;

   rc = BDBG_Fifo_Create( &(pstDebugFifo->hDebugFifo), &stCreateSettings );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("%s:: BDBG_Fifo_Create failure: rc:%08x", BSTD_FUNCTION, rc ));
      bAllocationError = true;
   }

#if 0
   BDBG_MSG(("%s:: ", BSTD_FUNCTION ));

   BDBG_MSG(("    pstDebugFifo->hBMMAHeap: %08x", (unsigned)pstDebugFifo->hBMMAHeap ));
   BDBG_MSG(("    pstDebugFifo->hDebugFifo: %08x", (unsigned)pstDebugFifo->hDebugFifo ));
   BDBG_MSG(("    pstDebugFifo->pBuffer: %08x", (unsigned)pstDebugFifo->pBuffer ));
   BDBG_MSG(("    pstDebugFifo->bmmaOffset: %08x", (unsigned)pstDebugFifo->bmmaOffset ));
   BDBG_MSG(("    pstDebugFifo->hBMMABlock: %08x", (unsigned)pstDebugFifo->hBMMABlock ));

   BDBG_MSG(("    pstDebugFifo->uiElementSize: %d",         pstDebugFifo->uiElementSize ));
   BDBG_MSG(("    pstDebugFifo->uiFifoSize: %d /1024 = %d", pstDebugFifo->uiFifoSize, pstDebugFifo->uiFifoSize/1024 ));
   BDBG_MSG(("    BXDM_P_DebugFifo_Entry size: %d",         sizeof(BXDM_P_DebugFifo_Entry) ));
   BDBG_MSG(("    BAVC_MFD_Picture size: %d",               sizeof(BAVC_MFD_Picture) ));
   BDBG_MSG(("    BXDM_DebugFifo_UnifiedPicture size: %d",  sizeof(BXDM_DebugFifo_UnifiedPicture) ));
   BDBG_MSG(("    BXDM_DebugFifo_String size: %d",          sizeof(BXDM_DebugFifo_String) ));
   BDBG_MSG(("    BXDM_DebugFifo_DebugInfo size: %d",       sizeof(BXDM_DebugFifo_DebugInfo) ));
   BDBG_MSG(("    BXDM_PictureProvider_P_Config size: %d",  sizeof(BXDM_PictureProvider_P_Config) ));
#endif

error:

   if ( true == bAllocationError )
   {
      BXDM_PPDFIFO_P_Fifo_Destroy( hXdmPP );
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Fifo_Create );

   return BERR_TRACE( rc );

}

BERR_Code BXDM_PPDFIFO_P_Fifo_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BXDM_PPDFIFO_P_Fifo_Destroy );

   /* Release the debug fifo. */

   if ( NULL == pstDebugFifo )
   {
      BDBG_ERR(("%s  pstDebugFifo is NULL", BSTD_FUNCTION ));
      goto error;
   }
#if 0
   BDBG_MSG(("%s::", BSTD_FUNCTION ));
   BDBG_MSG(("    pstDebugFifo->hBMMAHeap: %08x", (unsigned)pstDebugFifo->hBMMAHeap ));
   BDBG_MSG(("    hDebugFifo: %08x", (unsigned)pstDebugFifo->hDebugFifo ));
   BDBG_MSG(("    pBuffer: %08x", (unsigned)pstDebugFifo->pBuffer ));
   BDBG_MSG(("    bmmaOffset: %08x", (unsigned)pstDebugFifo->bmmaOffset ));
   BDBG_MSG(("    hBMMABlock: %08x", (unsigned)pstDebugFifo->hBMMABlock ));
#endif

   if ( NULL != pstDebugFifo->hDebugFifo )
   {
      BDBG_Fifo_Handle tempDebugFifo = pstDebugFifo->hDebugFifo;

      /* SWSTB-8169: use tempDebugFifo to avoid race condition at shutdown. */
      pstDebugFifo->hDebugFifo = NULL;

      BDBG_Fifo_Destroy( tempDebugFifo );
   }

   if ( 0 != pstDebugFifo->pBuffer )
   {
      BMMA_Unlock( pstDebugFifo->hBMMABlock, pstDebugFifo->pBuffer );
      pstDebugFifo->pBuffer = NULL;
      pstDebugFifo->bBufferValid = false;
   }

   if ( 0 != pstDebugFifo->bmmaOffset )
   {
      BMMA_UnlockOffset( pstDebugFifo->hBMMABlock, pstDebugFifo->bmmaOffset );
      pstDebugFifo->bmmaOffset = 0;
   }

   if ( NULL != pstDebugFifo->hBMMABlock )
   {
      BMMA_Free( pstDebugFifo->hBMMABlock );
      pstDebugFifo->hBMMABlock = NULL;
   }

error:

   BDBG_LEAVE( BXDM_PPDFIFO_P_Fifo_Destroy );

   return BERR_TRACE( rc );

}

BERR_Code BXDM_PPDFIFO_P_Reader_Create(
   BXDM_PictureProvider_Handle hXdmPP
   )
{

   BERR_Code rc = BERR_SUCCESS;
   bool bAllocationError = false;
   BXDM_Debug_ReaderInfo * pstReaderInfo;

   BDBG_ENTER( BXDM_PPDFIFO_P_Reader_Create );

   BDBG_ASSERT( hXdmPP );

   pstReaderInfo = &hXdmPP->stDMConfig.stDebugReader;

   rc = BXDM_PictureProvider_GetDebugFifo( hXdmPP, &(pstReaderInfo->stFifoInfo) );

   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("%s:: BXDM_PictureProvider_GetDebugFifo failure: rc:%08x", BSTD_FUNCTION, rc ));
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pEntry = BKNI_Malloc( pstReaderInfo->stFifoInfo.uiElementSize );

   if ( NULL == pstReaderInfo->pEntry )
   {
      BDBG_ERR(("%s:: BKNI_Malloc failure", BSTD_FUNCTION ));
      rc = BERR_OUT_OF_SYSTEM_MEMORY;
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pDebugFifo = BMMA_Lock( pstReaderInfo->stFifoInfo.hBlock );

   if ( 0 == pstReaderInfo->pDebugFifo )
   {
      BDBG_ERR(("%s:: BMMA_Lock failure:", BSTD_FUNCTION ));
      rc = BERR_OUT_OF_SYSTEM_MEMORY;
      bAllocationError = true;
      goto error;
   }

   pstReaderInfo->pDebugFifo = (uint8_t *)pstReaderInfo->pDebugFifo + pstReaderInfo->stFifoInfo.uiOffset;

   rc = BDBG_FifoReader_Create(&(pstReaderInfo->hDebugReader), pstReaderInfo->pDebugFifo);

   if (  BERR_SUCCESS != rc  )
   {
      BDBG_ERR(("%s:: BDBG_FifoReader_Create failure: rc:%08x", BSTD_FUNCTION, rc ));
      bAllocationError = true;
      goto error;
   }
#if 0
   BDBG_MSG(("%s::", BSTD_FUNCTION ));
   BDBG_MSG(("    pstReaderInfo->hDebugReader: %08x", (unsigned)pstReaderInfo->hDebugReader ));
   BDBG_MSG(("    pstReaderInfo->pEntry: %08x", (unsigned)pstReaderInfo->pEntry ));
   BDBG_MSG(("    pstReaderInfo->pDebugFifo: %08x", (unsigned)pstReaderInfo->pDebugFifo ));
#endif

error:

   if ( true == bAllocationError )
   {
      BXDM_PPDFIFO_P_Reader_Destroy( hXdmPP );
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Reader_Create );

   return BERR_TRACE( rc );

}


BERR_Code BXDM_PPDFIFO_P_Reader_Destroy(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BXDM_Debug_ReaderInfo * pstReaderInfo;

   BDBG_ASSERT( hXdmPP );

   BDBG_ENTER( BXDM_PPDFIFO_P_Reader_Destroy );

   pstReaderInfo = &hXdmPP->stDMConfig.stDebugReader;

#if 0
   BDBG_MSG(("%s::", BSTD_FUNCTION ));
   BDBG_MSG(("    pstReaderInfo->hDebugReader: %08x", (unsigned)pstReaderInfo->hDebugReader ));
   BDBG_MSG(("    pstReaderInfo->pEntry: %08x", (unsigned)pstReaderInfo->pEntry ));
   BDBG_MSG(("    pstReaderInfo->pDebugFifo: %08x", (unsigned)pstReaderInfo->pDebugFifo ));
#endif

   if ( NULL != pstReaderInfo->pDebugFifo )
   {
      /* TODO: should this come into play, i.e. is a subtract needed?
       * pstReaderInfo->pDebugFifo = (uint8_t *)pstReaderInfo->pDebugFifo + pstReaderInfo->stFifoInfo.uiOffset;
       */
      BMMA_Unlock( pstReaderInfo->stFifoInfo.hBlock, pstReaderInfo->pDebugFifo );
      pstReaderInfo->pDebugFifo = NULL;
   }

   if ( NULL != pstReaderInfo->pEntry )
   {
      BKNI_Free( pstReaderInfo->pEntry );
      pstReaderInfo->pEntry = NULL;
   }

   if ( NULL != pstReaderInfo->hDebugReader )
   {
      BDBG_FifoReader_Destroy( pstReaderInfo->hDebugReader );
      pstReaderInfo->hDebugReader = NULL;
   }

   BDBG_LEAVE( BXDM_PPDFIFO_P_Reader_Destroy );

   return BERR_TRACE( rc );

}

#endif

void BXDM_PPDFIFO_P_QueString_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   const BXDM_Debug_MsgType eMessageType,
   bool bNeedFormat,
   char * format,
   ...
   )
{
#if BDBG_DEBUG_BUILD
   BXDM_P_DebugFifo_Entry *pstEntry;
   BDBG_Fifo_Token stToken;
   BXDM_PictureProvider_P_DebugFifo * pstDebugFifo = &(hXdmPP->stDMConfig.stDebugFifo);

   BDBG_ENTER( BXDM_PPDFIFO_P_QueString_isr );

   BDBG_ASSERT( hXdmPP );

   if ( NULL == pstDebugFifo->hDebugFifo )
   {
      goto done;
   }

   pstEntry = (BXDM_P_DebugFifo_Entry *) BDBG_Fifo_GetBuffer( pstDebugFifo->hDebugFifo, &stToken );

   if ( NULL == pstEntry )
   {
      goto done;
   }

   {
      BXDM_PictureProvider_P_Picture_Context * pstPicture = NULL;
      va_list argList;


      BXDM_PPDFIFO_S_GetDefaultMetaData( hXdmPP, pstPicture, pstEntry );
      pstEntry->stMetadata.eType = BXDM_DebugFifo_EntryType_eString;

      pstEntry->data.stString.eType = ( eMessageType < BXDM_Debug_MsgType_eMax ) ? eMessageType : BXDM_Debug_MsgType_eUnKnown ;

      if ( true == bNeedFormat )
      {
         va_start( argList, format );
         BKNI_Vsnprintf(pstEntry->data.stString.szString, BXDM_P_MAX_DEBUG_FIFO_STRING_LEN, format, argList );
         va_end(argList);
      }
      else
      {
         va_start( argList, format );
         BKNI_Memcpy(pstEntry->data.stString.szString, va_arg(argList, char *), BXDM_P_MAX_DEBUG_FIFO_STRING_LEN);
         va_end(argList);
      }

      BDBG_Fifo_CommitBuffer( &stToken );
   }

done:

   BDBG_LEAVE( BXDM_PPDFIFO_P_QueString_isr );

#else
   BSTD_UNUSED(hXdmPP);
   BSTD_UNUSED(eMessageType);
   BSTD_UNUSED(bNeedFormat);
   BSTD_UNUSED(format);
#endif

   return;
}
