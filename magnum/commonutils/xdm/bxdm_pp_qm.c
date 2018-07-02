/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_tsm.h"
#include "bxdm_pp_vtsm.h"
#include "bxdm_pp_callback_priv.h"
#include "bxdm_pp_output.h"
#include "bxdm_pp_clip.h"

BDBG_MODULE(BXDM_PPQM);     /* Register software module with debug interface */

static void BXDM_PPQM_S_ValidatePictureContext_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstPrevPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicCntxt
   );

void BXDM_PPQM_P_InvalidatePictureContext_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   )
{
   BDBG_ENTER( BXDM_PPQM_P_InvalidatePictureContext_isr );

   BKNI_Memset( pstPicCntxt, 0, sizeof ( BXDM_PictureProvider_P_Picture_Context ) );

   BDBG_LEAVE( BXDM_PPQM_P_InvalidatePictureContext_isr );

   return;
}

static void BXDM_PPQM_S_Detect32FilmContent_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
)
{
   uint32_t i, uiIndex;
   BXDM_Picture_PullDown eExpected32Pulldown = 0;
   bool bDone = false;
   BXDM_Picture_PullDown ePulldown;

   BSTD_UNUSED(pLocalState);

   /* SW7445-586: use accessor method to retrieve pulldwon */
   BXDM_PPQM_P_GetPicturePulldown_isr( pstPicCntxt, &ePulldown );

   hXdmPP->stDMState.stDecode.ePPBPulldownHistory[hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryIndex] = ePulldown;

   if ( hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryIndex == ( BXDM_PictureProvider_P_MAX_PPB_PULLDOWN_HISTORY - 1 ) )
   {
      hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryIndex = 0;
   }
   else
   {
      hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryIndex++;
   }

   if ( hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryCount < BXDM_PictureProvider_P_MAX_PPB_PULLDOWN_HISTORY )
   {
      hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryCount++;
   }

   /* Check for 3:2 cadence */
   if ( hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryCount < BXDM_PictureProvider_P_MAX_PPB_PULLDOWN_HISTORY )
   {
      uiIndex = 0;
   }
   else
   {
      uiIndex = hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryIndex;
   }

   for (i = 0; (i < hXdmPP->stDMState.stDecode.uiPPBPulldownHistoryCount) && (false == bDone); i++ )
   {
      /* We need to seed the initial values */
      if ( ( i != 0 )
           && ( hXdmPP->stDMState.stDecode.ePPBPulldownHistory[uiIndex] != eExpected32Pulldown ) )
      {
         pstPicCntxt->stPicParms.stDisplay.stStatic.b32FilmSource = false;
         bDone = true;
      }
      else
      {
         switch ( hXdmPP->stDMState.stDecode.ePPBPulldownHistory[uiIndex] )
         {
            case BXDM_Picture_PullDown_eTopBottom:
               eExpected32Pulldown = BXDM_Picture_PullDown_eTopBottomTop;
               break;

            case BXDM_Picture_PullDown_eBottomTop:
               eExpected32Pulldown = BXDM_Picture_PullDown_eBottomTopBottom;
               break;

            case BXDM_Picture_PullDown_eTopBottomTop:
               eExpected32Pulldown = BXDM_Picture_PullDown_eBottomTop;
               pstPicCntxt->stPicParms.stDisplay.stStatic.b32FilmSource = true;
               break;

            case BXDM_Picture_PullDown_eBottomTopBottom:
               eExpected32Pulldown = BXDM_Picture_PullDown_eTopBottom;
               pstPicCntxt->stPicParms.stDisplay.stStatic.b32FilmSource = true;
               break;

            default:
               bDone = true;
               break;
         }
      }

      if ( uiIndex == ( BXDM_PictureProvider_P_MAX_PPB_PULLDOWN_HISTORY - 1 ) )
      {
         uiIndex = 0;
      }
      else
      {
         uiIndex++;
      }
   }
}

static void BXDM_PPQM_S_ValidatePictureContext_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstPrevPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicCntxt
   )
{
   BDBG_ENTER( BXDM_PPQM_S_ValidatePictureContext_isr );
   /*
   ** If this context has already been validated, there is nothing to do.
   ** This logic is a precursor to having an array of picture contexts.
   */
   if ( true == pstPicCntxt->bValidated )
   {
      goto AllDone;
   }
   else
   {
      pstPicCntxt->bValidated = true;
   }

   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eValidatePictureHead );

   hXdmPP->stDMState.stDecode.uiPPBIndex++;
   pstPicCntxt->stPicParms.uiPPBIndex = hXdmPP->stDMState.stDecode.uiPPBIndex;

   /* PR50196: Detect 3:2 film content and override source format as
    * needed */
   BXDM_PPQM_S_Detect32FilmContent_isr( hXdmPP, pLocalState, pstPicCntxt );

   /* Set TSM parameters */
   BXDM_PPTSM_P_PtsCalculateParameters_isr( hXdmPP, pLocalState, pstPicCntxt );

   /* By default, set the selection mode for this picture equal to the
    * display mode */
   pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = pLocalState->eSelectionMode;

   /* bSelectedInPrerollMode is only used in the debug messages to indicate why
    * the picture was selected in vsync mode
    */
   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode
        && true == hXdmPP->stDMState.stDecode.bPreRolling )
   {
      pstPicCntxt->stPicParms.stTSM.stDynamic.bSelectedInPrerollMode = true;
   }

   /*
    * If in playback mode, force PCR offset and all the associated
    * flags to '0'.
    *
    * If "live" playback, set the offset and flags based on the
    * data in the PPB and the system state.
    */

   /* Zero out local PCR copy */
   BKNI_Memset(
            &pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM,
            0,
            sizeof( BXDM_PictureProvider_P_PCROffset )
            );

   /* Set local PCR copy to be same as coded copy */
   if ( false == hXdmPP->stDMConfig.bPlayback )
   {
      pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy = pstPicCntxt->pstUnifiedPicture->stPCROffset;
   }

   /* Handle coded PTS related logic */
   if ( true == pstPicCntxt->pstUnifiedPicture->stPTS.bValid )
   {
      /* used to generated the first coded PTS callback */
      pLocalState->bCodedPtsReceived = true;

      /* Snapshot the first coded PTS received, this value will be delivered to
       * the application via the "BXVD_Interrupt_eFirstPTSReady" callback.
       * Previously the value returned by this callback was "stLatestCodedPts".
       * Testing preroll mode highlighted the need for the snapshot; preroll starts in vsync mode,
       * the first picture received will be promoted for display.  When the queue was empty, the PTS
       * value returned by "BXVD_Interrupt_eFirstPTSReady" was from the first picture.  When the queue
       * was not empty, the PTS returned was from the second picture.
       */
      if ( false == hXdmPP->stDMState.stDecode.bFirstCodedPtsReceived )
      {
         hXdmPP->stDMState.stDecode.stFirstCodedPts.ui32RunningPTS = pstPicCntxt->pstUnifiedPicture->stPTS.uiValue;
         hXdmPP->stDMState.stDecode.stFirstCodedPts.ePTSType = BXDM_PictureProvider_PTSType_eCoded;
         hXdmPP->stDMState.stDecode.bFirstCodedPtsReceived = true;
      }

      /* save the PTS */
      hXdmPP->stDMState.stDecode.stLatestCodedPts.ui32RunningPTS = pstPicCntxt->pstUnifiedPicture->stPTS.uiValue;
      hXdmPP->stDMState.stDecode.stLatestCodedPts.ePTSType = BXDM_PictureProvider_PTSType_eCoded;

      /* PR52901: save the PCR info.  An application can use this when it receives
       * the BXVD_Interrupt_eFirstPTSReady callback.  Or when calling BXVD_GetPTS
       * before a picture has been promoted for display.
       */
      hXdmPP->stDMState.stDecode.stLatestCodedPts.uiPCROffset = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue;
      hXdmPP->stDMState.stDecode.stLatestCodedPts.bPcrOffsetValid = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;
      hXdmPP->stDMState.stDecode.stLatestCodedPts.bPcrDiscontinuity = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity;
   }

   /* Override local PCR copy if it is not valid.  Use the previous PCR copy (if valid). */
   if ( ( false == hXdmPP->stDMConfig.bPlayback )
        && ( false == pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid ) )
   {
      /* The PCR is invalid, so we set the PCR for this PPB to be the
       * same as the previous PPB if it is valid, otherwise, we set it
       * to zero */
      if ( true == pstPrevPicCntxt->bValidated )
      {
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue = pstPrevPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue;
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid = pstPrevPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;
      }
      else
      {
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue = 0;
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid = false;
      }
   }

   if ( true == hXdmPP->stDMConfig.bPlayback )
   {
      hXdmPP->stDMState.stDecode.bValidPcrOffsetReceived = true;
   }
   else
   {
      /* SWSTB-68: handling of discontinuous PCR offsets at startup.  If "bValidPcrOffsetReceived" is false,
       * we are at the beginning of a stream.  If the handling mode is _eDrop or _eVsync and the PCR offset
       * is discontinuous, assume that it is invalid. Logic further along will display the picture in
       * vsync mode appropriate. */

      if ( false == hXdmPP->stDMState.stDecode.bValidPcrOffsetReceived )
      {
         bool bIgnoreThisPCR;

         bIgnoreThisPCR = ( BXDM_PictureProvider_PCRDiscontinuityMode_eDrop == hXdmPP->stDMConfig.ePCRDiscontinuityModeAtStartup );
         bIgnoreThisPCR |= ( BXDM_PictureProvider_PCRDiscontinuityMode_eVsync == hXdmPP->stDMConfig.ePCRDiscontinuityModeAtStartup );
         bIgnoreThisPCR &= ( true == pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity );

         if ( false == bIgnoreThisPCR )
         {
      hXdmPP->stDMState.stDecode.bValidPcrOffsetReceived = pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;
         }
      }

      /* PR50623: If the application has specified to NOT use
       * the HW PCR offset set the overloaded flags accordingly.
       * Otherwise load them with the derived values.
       */
      if ( false == hXdmPP->stDMConfig.bUseHardwarePCROffset )
      {
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded = true;
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded = false;
      }
      else
      {
         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bValidOverloaded =
                  pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bValid;

         pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded =
                  pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.bDiscontinuity;
      }
   }

   /* If an error, bump the error count.  */
   if( true == pstPicCntxt->pstUnifiedPicture->stError.bThisPicture )
   {
      hXdmPP->stDMStatus.stCounters.uiDecodeErrorCount++;
      pLocalState->bDecodeError = true;
   }

   /* Check for dummy PPB */
   if ( true == pstPicCntxt->pstUnifiedPicture->stBufferInfo.bValid )
   {
      hXdmPP->stDMState.stDecode.bFirstPPBSeen = true;
   }

   /*
    * Count the number of I-Frames.  Informational only, for use by an application.
    */
   if ( BXDM_Picture_Coding_eI == pstPicCntxt->pstUnifiedPicture->stPictureType.eCoding )
   {
      hXdmPP->stDMStatus.stCounters.uiIFrameCount++;

      if ( true == pstPicCntxt->pstUnifiedPicture->stError.bThisPicture )
      {
         hXdmPP->stDMStatus.stCounters.uiErrorIFrameCount++;
      }
   }

   /* SW7425-1001: check for "bLastPicture" being set, effectively an EOS flag.
    * Currently defined to only be delivered with a "picture-less" picture.
    *
    * If the flag is set for a "picture-less" picture, the picture currently
    * selected for display is the last one of this video.  When the selected picture
    * has been displayed completely, "bLast" should be set in the MFD picture structure.
    * This is done in BXDM_PPOUT_P_CalculateVdcData_isr.
    *
    * It is an error condition if "bLastPicture" is set for a "standard" picture.  The flag
    * will be ignored in this instance.
    */
   if ( true == pstPicCntxt->pstUnifiedPicture->stPictureType.bLastPicture )
   {
      /* "stBufferInfo.bValid" being "false" means that this is a "picture-less" picture.
       * Mark the currently selected picture as the last one.
       */
      if ( false == pstPicCntxt->pstUnifiedPicture->stBufferInfo.bValid )
      {
         pstSelectedPicCntxt->stPicParms.stDisplay.stDynamic.bLastPictureSeen = true;
      }
      else
      {
         BDBG_ERR(("%x:[%02x.%03x]  BXDM_PPQM_S_ValidatePictureContext_isr:: bLastPicture is set for a standard picture.",
                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                       pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF ));
      }
   }


   /*
    * Bump the running count of pictures seen by the video decoder.
    */
   hXdmPP->stDMStatus.stCounters.uiPicturesReceivedCount += pstPicCntxt->pstUnifiedPicture->stStats.uiDeltaPicturesSeen;

   BXDM_PPOUT_P_CalculateStaticVdcData_isr( hXdmPP, pstPicCntxt );

   /* Conditionally execute the PPB parameters callback. */
   pLocalState->bPictureUnderEvaluation = true;
   BXDM_PPCB_P_ExecuteSingleCallback_isr( hXdmPP, pLocalState, BXDM_PictureProvider_Callback_ePictureUnderEvaluation );

   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eValidatePictureHead );

AllDone:

   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eValidatePictureTail );

   /* PR52898: Remember the current eDisplayFieldMode setting for this
    * picture to prevent a change to the display field mode affecting
    * the picture after it's been displayed on the screen
    *
    * SW7405-4117: when BXDM_PictureProvider_DisplayFieldMode_eAuto is specified, the stream
    * height is used in conjunction with uiMaxHeightSupportedByDeinterlacer
    * to choose either eSingleField or eBothField slow motion (and preroll).
    *
    * SW7405-5683:  Delivering interlaced content to a display running at <= 30 Hz is not supported by the system.
    * The most common case would be 1080i60 to a 30p display. To get the best possible results, the FIC is disabled
    * and the display mode is forced to eSingleField for interlaced content when the monitor rate is <= 30 Hz.
    */
   if ( true == pstPicCntxt->stPicParms.stDisplay.stDynamic.bForceSingleFieldMode )
   {
      /* SW7405-5683: */
      pstPicCntxt->stPicParms.stDisplay.stDynamic.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eSingleField;
   }
   else if ( BXDM_PictureProvider_DisplayFieldMode_eAuto == hXdmPP->stDMConfig.eDisplayFieldMode )
   {
      bool bPausedOrSlowMotion;

      /* Check for a STC trick mode of pause or slow motion.
       */
      bPausedOrSlowMotion = ( BXDM_PictureProvider_P_STCTrickMode_ePause == pLocalState->eSTCTrickMode );
      bPausedOrSlowMotion |= ( BXDM_PictureProvider_P_STCTrickMode_eSlowMotion == pLocalState->eSTCTrickMode );
      bPausedOrSlowMotion |= ( BXDM_PictureProvider_P_STCTrickMode_eSlowRewind == pLocalState->eSTCTrickMode );

      /* Check for a decoder trick mode of pause or slow motion.  This check encompasses preroll.
       */
      bPausedOrSlowMotion |= ( pLocalState->uiSlowMotionRate < BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE );

      /* Default to eBothField */
      pstPicCntxt->stPicParms.stDisplay.stDynamic.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;

      /* When paused or in slow motion, revert to eSingleField if the deinterlacer cannot support the stream. */
      if ( pstPicCntxt->pstUnifiedPicture->stBufferInfo.stSource.uiHeight > hXdmPP->stDMConfig.uiMaxHeightSupportedByDeinterlacer
            && true == bPausedOrSlowMotion
         )
      {
         pstPicCntxt->stPicParms.stDisplay.stDynamic.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eSingleField;
      }
   }
   else
   {
      pstPicCntxt->stPicParms.stDisplay.stDynamic.eDisplayFieldMode = hXdmPP->stDMConfig.eDisplayFieldMode;
   }

   BXDM_PPCLIP_P_ClipTimeQueueTransitionHandler_isr(
      hXdmPP,
      pLocalState,
      pstPicCntxt
      );

   /* We can enter auto-vsync mode either at the beginning of a stream or during steady state.
    *
    * For steady state, we enter auto-vsync mode if:
    *
    * 1) live mode (PCR is invalid during playback)
    * 2) AND the PCR is discontinuous, i.e. "bDiscontinuityOverloaded" is set.
    *    (Note: if the HW PCR offset is being ignored, discontinuity will
    *     be ignored as well.  Hence the use of "bDiscontinuityOverloaded")
    * 3) AND the STC is valid
    * 4) AND the mode is enabled, ie. "bVsyncModeOnPcrDiscontinuity" is set.
    *
    * SWSTB-68: at startup, we enter auto-vsync mode if:
    * 1) live mode (PCR is invalid during playback)
    * 2) AND the PCR is discontinuous, i.e. "bDiscontinuityOverloaded" is set.
    *    (Note: if the HW PCR offset is being ignored, discontinuity will
    *     be ignored as well.  Hence the use of "bDiscontinuityOverloaded")
    * 3) AND a valid PCR offset has not been received (indicates startup state)
    * 4) AND the mode is "BXDM_PictureProvider_PCRDiscontinuityMode_eVsync"
    */
   {
      bool bAutoVsyncDuringSteadyState;
      bool bAutoVsyncAtStartup;

      bAutoVsyncDuringSteadyState = ( false == hXdmPP->stDMConfig.bPlayback );
      bAutoVsyncDuringSteadyState &= ( true == pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded );
      bAutoVsyncDuringSteadyState &= ( true == hXdmPP->stDMConfig.bSTCValid );
      bAutoVsyncDuringSteadyState &= ( true == hXdmPP->stDMConfig.bVirtualTSMOnPCRDiscontinuity );

      bAutoVsyncAtStartup = ( false == hXdmPP->stDMConfig.bPlayback );
      bAutoVsyncAtStartup &= ( true == pstPicCntxt->stPicParms.stTSM.stStatic.stPCROffsetXDM.bDiscontinuityOverloaded );
      bAutoVsyncAtStartup &= ( false == hXdmPP->stDMState.stDecode.bValidPcrOffsetReceived );
      bAutoVsyncAtStartup &= ( BXDM_PictureProvider_PCRDiscontinuityMode_eVsync == hXdmPP->stDMConfig.ePCRDiscontinuityModeAtStartup );

      if ( true == bAutoVsyncDuringSteadyState || true == bAutoVsyncAtStartup )
      {
         /* SW7335-781: Output warning when forcing picture selection override */
         BXDM_PPDBG_P_PrintSelectionModeOverride_isr("PCR Discontinuity", hXdmPP, pstPicCntxt);
         pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;

      }
   }

   /* If the decode supports drop@decode, check if there is an outstanding drop request.
    *
    * SW7425-3237: The vPTS (virtual PTS) of the current picture is based on "eSelectionMode",
    * the picture selection mode.
    * - If the mode is eTSM, vPTS will be based on the preceding picture's ACTUAL PTS value.
    * - If the mode is eVirtual (vsync), vPTS will be based on the preceding picture's VIRTUAL PTS.
    *
    * With the above in mind, any "eSelectionMode" overrides should occur before the vPTS is set,
    * i.e. before "BXDM_PPVTSM_P_VirtualPtsInterpolate_isr" is called.
    *
    * The failure reported by SW7425-3237 was due to the following code block being located
    * in "BXDM_PPTSM_S_CompareStcAndPts".  The failure was found by doing a "hot swap"; the streamer
    * was playing one stream and then switched to a second stream without executing a Stop/Start decode.
    * This resulted in the coded PTS values taking a large step back in time. In addition, the system was
    * running in vsync mode due to the PCR being discontinuous. The failure came about when the PCR became
    * "continuous", the vPTS for that picture was based the ACTUAL PTS of the preceding picture, however
    * the virtual STC was still running on the old time base.  As a result the picture was "late" as were
    * all subsequent ones.  The virtual STC didn't reset because the system never transitioned out of vsync mode.
    *
    * By moving this code block here, when "uiPendingDropCount" is non-zero, the picture's vPTS will
    * be based on the preceding picture's VIRTUAL PTS.
    */
   if ( ( NULL != hXdmPP->stDMConfig.stDecoderInterface.requestPictureDrop_isr )
        && ( NULL != hXdmPP->stDMConfig.stDecoderInterface.getPictureDropPendingCount_isr )
      )
   {
      /* Snapshot the start time. */
      BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureDropPendingCountIsr );

      hXdmPP->stDMConfig.stDecoderInterface.getPictureDropPendingCount_isr(
               hXdmPP->stDMConfig.pDecoderPrivateContext,
               &hXdmPP->stDMState.stDecode.uiPendingDropCount
               );

      /* Snapshot the end time. */
      BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureDropPendingCountIsr );

      if ( 0 != hXdmPP->stDMState.stDecode.uiPendingDropCount )
      {
         /* SW7335-781: Output warning when forcing picture selection override */
         BXDM_PPDBG_P_PrintSelectionModeOverride_isr("Drop@Decode", hXdmPP, pstPicCntxt);
         /* SW7405-3929: If there is a pending drop@decode, then override selection mode to vTSM mode for this picture */
         pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
      }
   }


   if ( BXDM_PictureProvider_DisplayMode_eVirtualTSM != pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode )
   {
      /* Handle auto-vsync mode for this picture */
      pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = pLocalState->eSelectionMode;

   }

   /* Now that all the necessary parameters have been dereferenced,
    * calculate the appropriate PTS values.
    */
   BXDM_PPTSM_P_PtsInterpolate_isr( hXdmPP, pLocalState, pstPicCntxt, pstPrevPicCntxt );
   BXDM_PPVTSM_P_VirtualPtsInterpolate_isr( hXdmPP, pLocalState, pstPicCntxt, pstPrevPicCntxt, pstSelectedPicCntxt );

   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eValidatePictureTail );

   BDBG_LEAVE( BXDM_PPQM_S_ValidatePictureContext_isr );

   return;

}

/*******************************************************************************
 **
 ** end "private" display queue management routines.
 **
 *******************************************************************************/


/*******************************************************************************
 **
 ** Routines for manipulating picture context.
 **
 *******************************************************************************/


void BXDM_PPQM_P_GetHwPcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   )
{
   if ( true == hXdmPP->stDMConfig.bUseHardwarePCROffset )
   {
      *puiPcrOffset = pstPicture->stPicParms.stTSM.stStatic.stPCROffsetXDM.stPCROffsetCopy.uiValue;
   }
   else
   {
      *puiPcrOffset = 0;
   }
   return;
}

void BXDM_PPQM_P_GetSoftwarePcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   )
{
   /* SW7635-51: when a picture is selected for display, uiSoftwarePCROffset is bound
    * to the picture and "bSwPcrOffsetValid" is set true.  Prior to selecting a picture,
    * "uiSoftwarePCROffset" will be false.
    */
   if ( true == pstPicture->stPicParms.stDisplay.stDynamic.bSwPcrOffsetValid )
   {
      *puiPcrOffset = pstPicture->stPicParms.stDisplay.stDynamic.uiSoftwarePCROffset;
   }
   else
   {
      *puiPcrOffset = hXdmPP->stDMConfig.uiSoftwarePCROffset;
   }

   return;
}

/* PR50623: return the "cooked" PCR offset, i.e. the sum
 * of the HW and SW PCR offsets.
 */
void BXDM_PPQM_P_GetCookedPcrOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPcrOffset
   )
{
   uint32_t uiSoftwarePcrOffset;
   uint32_t uiHwPcrOffset;

   /* Return the sum of the SW and HW PCR offsets. */

   BXDM_PPQM_P_GetSoftwarePcrOffset_isr( hXdmPP, pstPicture, &uiSoftwarePcrOffset );
   BXDM_PPQM_P_GetHwPcrOffset_isr( hXdmPP, pstPicture, &uiHwPcrOffset );

   *puiPcrOffset = uiSoftwarePcrOffset + uiHwPcrOffset;

   return;
}

/* SW7425-2255: return the appropriate display offset.
 */
void BXDM_PPQM_P_GetPtsOffset_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   uint32_t * puiPtsOffset
   )
{
   /* SW7425-2255: when a picture is selected for display, uiPTSOffset is bound
    * to the picture and "bPtsOffsetValid" is set true.  Prior to selecting a picture,
    * "bPtsOffsetValid" will be false.
    */
   if ( true == pstPicture->stPicParms.stDisplay.stDynamic.bPtsOffsetValid )
   {
      *puiPtsOffset = pstPicture->stPicParms.stDisplay.stDynamic.uiPtsOffset;
   }
   else
   {
      *puiPtsOffset = hXdmPP->stDMConfig.uiPTSOffset;
   }

   return;
}


/*
** Public functions.
*/
void BXDM_PPQM_P_GetPredictedPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t * puiPts
   )
{
   BDBG_ASSERT(puiPts);

   if ( false == pstPicture->bValidated )
   {
      *puiPts = 0;
   }
   else
   {
      *puiPts = pstPicture->stPicParms.stTSM.stStatic.stPTSOfNextPPB[ePTSIndex].uiWhole;
   }

   return;

}

void BXDM_PPQM_P_GetPredictedPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   BXDM_PPFP_P_DataType *pstPTS
   )
{
   BDBG_ASSERT(pstPTS);

   if ( false == pstPicture->bValidated )
   {
      pstPTS->uiWhole = 0;
      pstPTS->uiFractional = 0;
   }
   else
   {
      *pstPTS = pstPicture->stPicParms.stTSM.stStatic.stPTSOfNextPPB[ePTSIndex];
   }

   return;

}

void BXDM_PPQM_P_SetPredictedPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   BXDM_PPFP_P_DataType stPTS
   )
{

   pstPicture->stPicParms.stTSM.stStatic.stPTSOfNextPPB[ePTSIndex] = stPTS;

   return;
}

void BXDM_PPQM_P_GetPtsType_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_PTSType * pePtsType
   )
{

   if ( false == pstPicture->bValidated )
   {
      *pePtsType = BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS;
   }
   else
   {
      *pePtsType = pstPicture->stPicParms.stTSM.stStatic.ePtsType;
   }

   return;
}

void BXDM_PPQM_P_GetPtsTypeUnfiltered_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_PTSType * pePtsType
   )
{
   *pePtsType = pstPicture->stPicParms.stTSM.stStatic.ePtsType;

   return;
}


void BXDM_PPQM_P_SetPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   BXDM_PPFP_P_DataType stPTS
   )
{
   BDBG_ASSERT( uiIndex < BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB );

   /* A runtime check to keep Coverity happy. */
   if ( uiIndex >= BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB )
   {
      uiIndex = BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB - 1;
   }

   pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiIndex ].stPTS[ePTSIndex] = stPTS;

   return;
}

void BXDM_PPQM_P_GetPtsWithFrac_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   BXDM_PPFP_P_DataType *pstPTS
   )
{
   BDBG_ASSERT( uiIndex < BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB );

   /* A runtime check to keep Coverity happy. */
   if ( uiIndex >= BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB )
   {
      uiIndex = BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB - 1;
   }

   if ( false == pstPicture->bValidated )
   {
      pstPTS->uiWhole = 0;
      pstPTS->uiFractional = 0;
   }
   else
   {
      *pstPTS = pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiIndex ].stPTS[ePTSIndex];
   }

   return;
}


void BXDM_PPQM_P_GetPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   uint32_t * puiPts
   )
{
   BDBG_ASSERT( uiIndex < BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB );

   /* A runtime check to keep Coverity happy. */
   if ( uiIndex >= BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB )
   {
      uiIndex = BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB - 1;
   }


   if ( false == pstPicture->bValidated )
   {
      *puiPts = 0;
   }
   else
   {
      *puiPts = pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiIndex ].stPTS[ePTSIndex].uiWhole;
   }

   return;
}

void BXDM_PPQM_P_GetPtsUnfiltered_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t    uiIndex,
   uint32_t * puiPts
   )
{
   BDBG_ASSERT( uiIndex < BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB );

   /* A runtime check to keep Coverity happy. */
   if ( uiIndex >= BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB )
   {
      uiIndex = BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB - 1;
   }

   *puiPts = pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiIndex ].stPTS[ePTSIndex].uiWhole;

   return;
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXDM_PPQM_P_GetLastPts_isr(
   BXDM_PictureProvider_P_Picture_Context * pstPicture,
   BXDM_PictureProvider_P_PTSIndex ePTSIndex,
   uint32_t * puiPts
   )
{
   uint32_t    uiIndex;

   if ( false == pstPicture->bValidated )
   {
      *puiPts = 0;
   }
   else
   {
      /* This range checking probably isn't required. */

      if ( BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB <= pstPicture->stPicParms.stTSM.stStatic.uiNumElements )
      {
         uiIndex = ( BXDM_PictureProvider_P_MAX_ELEMENTS_PER_PPB - 1 );
      }
      else if ( 0 >= pstPicture->stPicParms.stTSM.stStatic.uiNumElements )
      {
         uiIndex = 0;
      }
      else
      {
         uiIndex = ( pstPicture->stPicParms.stTSM.stStatic.uiNumElements  - 1 );
      }

      BXDM_PPQM_P_GetPts_isr(
                     pstPicture,
                     ePTSIndex,
                     uiIndex,
                     puiPts
                     );

   }

   return;

}
#endif

void BXDM_PPQM_P_SetElementPolarity_isr(
   BXDM_PictureProvider_P_Picture_Context *pstPicture,
   uint32_t uiElementIndex,
   BAVC_Polarity ePolarity)
{

   pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiElementIndex ].ePolarity = ePolarity;

   return;
}

void BXDM_PPQM_P_GetElementPolarity_isr(
   BXDM_PictureProvider_P_Picture_Context *pstPicture,
   uint32_t uiElementIndex,
   BAVC_Polarity *pePolarity)
{

   BDBG_ASSERT(pePolarity);

   *pePolarity = pstPicture->stPicParms.stTSM.stStatic.astElmts[ uiElementIndex ].ePolarity;

   return;
}

/* SW7445-586: initial H265/HEVC interlaced code.
 * SW7445-1638: for H265/HEVC interlaced, each field is stored in a separate picture buffer.
 * A stream may be encoded so that fields can be paired up. In this instance, two
 * linked Unified Pictures will be received. Otherwise, a single Unified Picture will
 * be received which describes a single field.
 *
 * When fields have been paired, the linked Unified Pictures need to be treated as
 * one logical picture. The pulldowns of the separate fields will be combine to
 * derive the pulldown of the logical picture. When only a single field, the pulldown
 * of that field will be used.
 *
 * In addition, for the case of a TBT or BTB picture, the third field will be delivered
 * in a separate Unified Picture as a single field. This third field is really a duplicate
 * of the first field. This field will be "appended" to the original picture in
 * "BXDM_PPQM_P_AppendField_isr()". This will cause the pulldown to morph from TB->TBT or BT->BTB.
 * This new pulldown will be stored in "eSynthesizedPulldown". This field is actually dropped,
 * the morphing of the pulldown causes the first two fields to be displayed longer.
 */
void BXDM_PPQM_P_GetPicturePulldown_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   BXDM_Picture_PullDown * pePulldown
   )
{
   BDBG_ASSERT(pstPicture);
   BDBG_ASSERT(pePulldown);

   if ( pstPicture->stPicParms.stDisplay.stDynamic.eSynthesizedPulldown )
   {
      /* A 3 field (TBT or BTB) split interlaced picture was received.
       * The pulldown of the initial picture needed to be modified,
       * use the modified one.*/
      *pePulldown = pstPicture->stPicParms.stDisplay.stDynamic.eSynthesizedPulldown;
   }
   else if ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstPicture->pstUnifiedPicture->stBufferInfo.eBufferFormat
        &&  NULL != pstPicture->pstUnifiedPicture->pNextPicture )
   {
      /* A linked pair of fields has been received. Use their separate pulldowns
       * to derive one logical pulldown. */
      BXDM_Picture_PullDown ePulldownField_1, ePulldownField_2;
      BXDM_Picture * pNextPicture = pstPicture->pstUnifiedPicture->pNextPicture;

      ePulldownField_1 = pstPicture->pstUnifiedPicture->stBufferInfo.ePulldown;
      ePulldownField_2 = pNextPicture->stBufferInfo.ePulldown;

      if ( BXDM_Picture_PullDown_eTop == ePulldownField_1
           && BXDM_Picture_PullDown_eBottom == ePulldownField_2 )
      {
         *pePulldown = BXDM_Picture_PullDown_eTopBottom;
      }
      else if ( BXDM_Picture_PullDown_eBottom == ePulldownField_1
                 && BXDM_Picture_PullDown_eTop == ePulldownField_2 )
      {
         *pePulldown = BXDM_Picture_PullDown_eBottomTop;
      }
      else
      {
         /* The upstream logic should prevent this situation from ever happening.
          * But just in case, use the pulldown of the "base" picture if something
          * has gone amiss. */
         *pePulldown = pstPicture->pstUnifiedPicture->stBufferInfo.ePulldown;
      }

   }
   else
   {
      /* Get here for all standard content (single picture buffer) or split
       * interlace with a single field. */
      *pePulldown = pstPicture->pstUnifiedPicture->stBufferInfo.ePulldown;
   }

   return;
}

/* SW7445-586: for paired H265/HEVC interlaced pictures, the XVD decoder
 * sends along a picture once it has a matching set (TB or BT).  It does
 * not wait to see if the next field is associated with the preceeding
 * fields, i.e. a TBT or BTB picture. This third field will be sent as a
 * single field Unified Picture with the
 * BXDM_Picture_BufferHandlingMode_eSiRepeat flag set.
 *
 * VDC only wants to deal with the picture buffers associated with the
 * TB/BT picture.  This third field will simply be used to extend the
 * display of the original picture by one field time.  It does this by
 * morping the pull down from from TB->TBT or BT->BTB.  This third
 * field will not be sent to VDC.
 */
void BXDM_PPQM_P_AppendField_isr(
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicture,
   BXDM_PictureProvider_P_Picture_Context * pstCurrentPicture
   )
{
   bool bBindPicture;
   bool bFieldAppended = false;

   BDBG_ASSERT(pLocalState);
   BDBG_ASSERT(pstSelectedPicture);
   BDBG_ASSERT(pstCurrentPicture);

   /* Check that:
    * - the "selected" picture has been validated (needed for startup)
    * - the "selected" picture format is split interlaced
    * - the "picture under evaluation" picture format is split interlaced (seems redundant, given that it is a repeated picture)
    *
    * Note: this logic assumes that the calling routine checked that the
    * BXDM_Picture_BufferHandlingMode_eSiRepeat flag was set on the "next" picture.
    */
   bBindPicture = ( true == pstSelectedPicture->bValidated );
   bBindPicture &= ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstSelectedPicture->pstUnifiedPicture->stBufferInfo.eBufferFormat );
   bBindPicture &= ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstCurrentPicture->pstUnifiedPicture->stBufferInfo.eBufferFormat );

   if ( true == bBindPicture )
   {
      BXDM_Picture_PullDown ePulldownSelected, ePulldownCurrent;
      uint32_t i;

      BXDM_PPQM_P_GetPicturePulldown_isr( pstSelectedPicture, &ePulldownSelected );
      BXDM_PPQM_P_GetPicturePulldown_isr( pstCurrentPicture, &ePulldownCurrent );

      /* Append the field to the preceeding picture if a "T" follows at "TB" or "B" follows a "BT".
       * To append, set the number of elements and the polarity of each element.
       *
       * TODO: what if the preceeding is not true?  Should the picture just be dropped?
       */
      if ( BXDM_Picture_PullDown_eTopBottom == ePulldownSelected
           && BXDM_Picture_PullDown_eTop == ePulldownCurrent )
      {
         pstSelectedPicture->stPicParms.stDisplay.stDynamic.eSynthesizedPulldown = BXDM_Picture_PullDown_eTopBottomTop;
         pstSelectedPicture->stPicParms.stTSM.stStatic.uiNumElements = 3;
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 0, BAVC_Polarity_eTopField );
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 1, BAVC_Polarity_eBotField );
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 2, BAVC_Polarity_eTopField );
         bFieldAppended = true;
      }
      else if ( BXDM_Picture_PullDown_eBottomTop == ePulldownSelected
                  && BXDM_Picture_PullDown_eBottom == ePulldownCurrent )
      {
         pstSelectedPicture->stPicParms.stDisplay.stDynamic.eSynthesizedPulldown = BXDM_Picture_PullDown_eBottomTopBottom;
         pstSelectedPicture->stPicParms.stTSM.stStatic.uiNumElements = 3;
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 0, BAVC_Polarity_eBotField );
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 1, BAVC_Polarity_eTopField );
         BXDM_PPQM_P_SetElementPolarity_isr( pstSelectedPicture, 2, BAVC_Polarity_eBotField );
         bFieldAppended = true;
      }

      /* If appending the field
       * - select the 3rd element of the picture, this is the one that just passed TSM
       * - recalculate both the actual and virtual PTS values.
       */
      if ( true == bFieldAppended )
      {
         BXDM_PPFP_P_DataType stPtsActual, stPstVirtual;
         BXDM_PPFP_P_DataType * pstDelta = &(pstSelectedPicture->stPicParms.stTSM.stStatic.stPTSDelta);

         pstCurrentPicture->stPicParms.stDisplay.stDynamic.bAppendedToPreviousPicture = true;

         pstSelectedPicture->stPicParms.stDisplay.stDynamic.uiSelectedElement = 2;

         BXDM_PPQM_P_GetPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eActual, 0, &stPtsActual );
         BXDM_PPQM_P_GetPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, 0, &stPstVirtual );

         for( i=0; i < pstSelectedPicture->stPicParms.stTSM.stStatic.uiNumElements; i++ )
         {
            BXDM_PPQM_P_SetPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eActual, i, stPtsActual );
            BXDM_PPQM_P_SetPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, i, stPstVirtual );

            /* SW7425-4389: interpolate the PTS values in reverse when the
             * SW STC is running in reverse. */
            if ( true == pLocalState->bUsingSwStcToRunInReverse )
            {
               BXDM_PPFP_P_FixPtSub_isrsafe( &stPtsActual, pstDelta, &stPtsActual );
               BXDM_PPFP_P_FixPtSub_isrsafe( &stPstVirtual, pstDelta, &stPstVirtual );
            }
            else
            {
               BXDM_PPFP_P_FixPtAdd_isrsafe( &stPtsActual, pstDelta, &stPtsActual );
               BXDM_PPFP_P_FixPtAdd_isrsafe( &stPstVirtual, pstDelta, &stPstVirtual );
            }
         }

         BXDM_PPQM_P_SetPredictedPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eActual, stPtsActual );
         BXDM_PPQM_P_SetPredictedPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, stPstVirtual );

      }/* end of if ( true = bFieldAppended )*/

   }

   return;
}


/* SW7445-586: for H265/HEVC interlaced, two linked Unified Pictures may be received.
 * Each Unified Picture will describe a single field.  These two fields need to be
 * treated as one logical picture. BXDM_PPQM_P_GetPictureTopBuffer_isr and
 * BXDM_PPQM_P_GetPictureBottomBuffer_isr are used to retrieve the appropriate picture.
 */
void BXDM_PPQM_P_GetPictureTopBuffer_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   const BXDM_Picture_BufferInfo ** pstBufferInfo
   )
{
   BDBG_ASSERT(pstPicture);
   BDBG_ASSERT(pstBufferInfo);

   if ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstPicture->pstUnifiedPicture->stBufferInfo.eBufferFormat
        &&  NULL != pstPicture->pstUnifiedPicture->pNextPicture )
   {
      /* A linked pair of field buffers. Use the pulldown of the separate fields to
       * retrieve the top buffer. */
      BXDM_Picture * pNextPicture = pstPicture->pstUnifiedPicture->pNextPicture;

      if ( BXDM_Picture_PullDown_eTop == pstPicture->pstUnifiedPicture->stBufferInfo.ePulldown )
      {
         *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
      }
      else if ( BXDM_Picture_PullDown_eTop == pNextPicture->stBufferInfo.ePulldown )
      {
         *pstBufferInfo = &(pNextPicture->stBufferInfo);
      }
      else
      {
         /* The upstream logic should prevent this situation from ever happening.
          * But just in case, return the "base" picture if something has gone amiss. */
         *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
      }
   }
   else
   {
      /* Get here for all standard content (single picture buffer) or split
       * interlace with a single field. */
      *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
   }

   return;

}

void BXDM_PPQM_P_GetPictureBottomBuffer_isr(
   const BXDM_PictureProvider_P_Picture_Context *pstPicture,
   const BXDM_Picture_BufferInfo ** pstBufferInfo
   )
{
   BDBG_ASSERT(pstPicture);
   BDBG_ASSERT(pstBufferInfo);

   if ( BXDM_Picture_BufferFormat_eSplitInterlaced == pstPicture->pstUnifiedPicture->stBufferInfo.eBufferFormat
        &&  NULL != pstPicture->pstUnifiedPicture->pNextPicture )
   {
      /* A linked pair of field buffers. Use the pulldown of the separate fields to
       * retrieve the bottom buffer. */
      BXDM_Picture * pNextPicture = pstPicture->pstUnifiedPicture->pNextPicture;

      if ( BXDM_Picture_PullDown_eBottom == pstPicture->pstUnifiedPicture->stBufferInfo.ePulldown )
      {
         *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
      }
      else if ( BXDM_Picture_PullDown_eBottom == pNextPicture->stBufferInfo.ePulldown )
      {
         *pstBufferInfo = &(pNextPicture->stBufferInfo);
      }
      else
      {
         /* The upstream logic should prevent this situation from ever happening.
          * But just in case, return the "base" picture if something has gone amiss. */
         *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
      }
   }
   else
   {
      /* Get here for all standard content (single picture buffer) or split
       * interlace with a single field. */
      *pstBufferInfo = &(pstPicture->pstUnifiedPicture->stBufferInfo);
   }

   return;

}


/*******************************************************************************
 **
 ** End routines for manipulating picture context.
 **
 *******************************************************************************/


/*******************************************************************************
 **
 ** "public" display queue management routines.
 **
 *******************************************************************************/
bool BXDM_PPQM_P_PeekAtNextPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstPrevPicCntxt,
   BXDM_PictureProvider_P_Picture_Context * pstSelectedPicCntxt
   )
{
   uint32_t uiPictureCount = 0;

   BDBG_ENTER( BXDM_PPQM_P_PeekAtNextPicture_isr );

   /* Snapshot the start time. */
   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureCountIsr );

   hXdmPP->stDMConfig.stDecoderInterface.getPictureCount_isr(
            hXdmPP->stDMConfig.pDecoderPrivateContext,
            &uiPictureCount
            );

   /* Snapshot the end time. */
   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP,BXDM_PPTIMER_P_Function_eDecoderGetPictureCountIsr );

   if ( 0 != uiPictureCount )
   {
      /* Used to conditionally generated the "first PTS ready" callback.
       * Which is really the "first PPB received" callback.
       */
      pLocalState->bFirstPPBReceived = true;

      /* Snapshot the start time. */
      BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderPeekAtPictureIsr );

      hXdmPP->stDMConfig.stDecoderInterface.peekAtPicture_isr(
               hXdmPP->stDMConfig.pDecoderPrivateContext,
               0,
               &pstPicCntxt->pstUnifiedPicture
               );

      /* Snapshot the end time. */
      BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderPeekAtPictureIsr );

      /* Update the dynamic context associate with this PPB. */
      BXDM_PPQM_S_ValidatePictureContext_isr( hXdmPP, pLocalState, pstPicCntxt, pstPrevPicCntxt, pstSelectedPicCntxt );
   }

   BDBG_LEAVE( BXDM_PPQM_P_PeekAtNextPicture_isr );

   return ( 0 != uiPictureCount );
}

void BXDM_PPQM_P_GetNextPicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   )
{
   uint32_t uiPictureCount = 0;

   BDBG_ENTER( BXDM_PPQM_P_GetNextPicture_isr );
   BSTD_UNUSED(pLocalState);

   /* This routine assumes that there is data in the queue.
    * The following check is to help with debug.
    */

   /* Snapshot the start time. */
   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureCountIsr );

   hXdmPP->stDMConfig.stDecoderInterface.getPictureCount_isr(
            hXdmPP->stDMConfig.pDecoderPrivateContext,
            &uiPictureCount
            );

   /* Snapshot the end time. */
   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetPictureCountIsr );

   BDBG_ASSERT( 0 != uiPictureCount );

   /* Snapshot the start time. */
   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetNextPictureIsr );

   hXdmPP->stDMConfig.stDecoderInterface.getNextPicture_isr(
            hXdmPP->stDMConfig.pDecoderPrivateContext,
            &pstPicCntxt->pstUnifiedPicture
            );

   /* Snapshot the end time. */
   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderGetNextPictureIsr );

   /*  Since the PPB has been "removed" from the delivery queue, update the "PPBReceived" count.  */
   hXdmPP->stDMStatus.stCounters.uiPicturesDecodedCount++;

   BXDM_PPDBG_P_PrintUnifiedPicture_isr( hXdmPP, pLocalState, pstPicCntxt );

   BDBG_LEAVE( BXDM_PPQM_P_GetNextPicture_isr );

   return;

}   /* end of BXDM_PPQM_P_GetNextPicture_isr() */

void BXDM_PPQM_P_ReleasePicture_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   )
{
   BDBG_ENTER( BXDM_PPQM_P_ReleasePicture_isr );

   /* Snapshot the start time. */
   BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderReleasePictureIsr );

   if ( NULL != pstPicCntxt->pstUnifiedPicture )
   {
      hXdmPP->stDMConfig.stDecoderInterface.releasePicture_isr(
               hXdmPP->stDMConfig.pDecoderPrivateContext,
               pstPicCntxt->pstUnifiedPicture,
               NULL
               );
   }

   /* Snapshot the end time. */
   BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, BXDM_PPTIMER_P_Function_eDecoderReleasePictureIsr );

   /* Bump the PPB released count. */
   hXdmPP->stDMStatus.stCounters.uiPicturesReleasedCount++;

   /* Invalidate context */
   pstPicCntxt->pstUnifiedPicture = NULL;
   pstPicCntxt->bValidated = false;

   BDBG_LEAVE( BXDM_PPQM_P_ReleasePicture_isr );

   return;

}

void BXDM_PPQM_P_ReleasePictureExt_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   )
{
   BDBG_ENTER( BXDM_PPQM_P_ReleasePictureExt_isr );

   /* SW7405-4804: Typically the extension data callback will be executed when a
    * picture is delivered to VDC via the "BXDM_PPCB_P_ExecuteCallbacks_isr" function.
    * However this does not happen for pictures that are dropped.  This case is
    * now handled by calling "BXDM_PPCB_P_ExecuteSingleCallbackExt_isr" whenever a
    * picture is released.  The flag "bExtensionDataDelivered" ensures that the
    * callback is only executed once per picture.
    */
   BXDM_PPCB_P_ExecuteSingleCallbackExt_isr(
               hXdmPP,
               pLocalState,
               pstPicCntxt,
               BXDM_PictureProvider_Callback_ePictureExtensionData
               );

   BXDM_PPQM_P_ReleasePicture_isr( hXdmPP, pstPicCntxt );

   BDBG_LEAVE( BXDM_PPQM_P_ReleasePictureExt_isr );

   return;
}

void BXDM_PPQM_P_PromotePictureToDisplayStatus_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstNewPicture
   )
{
   BXDM_PictureProvider_P_Picture_Context *  pstSelectedPicture;

   BDBG_ENTER( BXDM_PPQM_P_PromotePictureToDisplayStatus_isr );

   pstSelectedPicture = &(hXdmPP->stDMState.stChannel.stSelectedPicture);

   /* copy the context on the new picture */

   *pstSelectedPicture = *pstNewPicture;

   /*
    * The following logic is esentially cloned from DM V1.
    * "bThisIsTheFirstPPB" will only be true the first time a picture
    * is promoted to display status.
    * "bFirstPPBHasBeenDisplayed" will remain true once a picture is
    * promoted to display status.
    */
   if ( false == hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed )
   {
      hXdmPP->stDMState.stDecode.bThisIsTheFirstPPB = true;
   }
   else
   {
      hXdmPP->stDMState.stDecode.bThisIsTheFirstPPB = false;
   }

   /* SW7445-2645: need to tag the first picture which passes the TSM test
    * to conditionally apply the STC jitter offset.
    * The "bThisIsTheFirstPPB" flag gets set even in vsync mode, i.e.
    * for first-picture-preview and preroll mode.
    * This state info may be overkill, but might prove useful in the future.
    */
   switch ( hXdmPP->stDMState.stDecode.eTsmPassState )
   {
      case BXDM_PictureProvider_P_TsmPassState_eNoneHavePassed :
         if ( true == pLocalState->bTSMPass )
         {
            hXdmPP->stDMState.stDecode.eTsmPassState = BXDM_PictureProvider_P_TsmPassState_eFirstToPass;
         }
         break;

      case BXDM_PictureProvider_P_TsmPassState_eFirstToPass:
         hXdmPP->stDMState.stDecode.eTsmPassState = BXDM_PictureProvider_P_TsmPassState_ePostFirstPass;
         break;

      case BXDM_PictureProvider_P_TsmPassState_ePostFirstPass:
      default:
         hXdmPP->stDMState.stDecode.eTsmPassState = BXDM_PictureProvider_P_TsmPassState_ePostFirstPass;
         break;
   }

   hXdmPP->stDMState.stDecode.bFirstPPBHasBeenDisplayed = true;

   /* If the picture is the first I frame, generate the BXVD_Interrupt_eIFrame callback */

   BXDM_PPCB_P_EvaluateCriteria_IFrame_isr( hXdmPP, pLocalState, pstSelectedPicture );

   /* Since we've promoted a new PPB, we need to reset the PPB and
    * Element repeated flags to false */
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated = false;
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.bElementRepeated = false;

   /* Remember the SPIM setting for this picture to handle cases where
    * SPIM is changed from "both" to "default" during a trick mode
    * transition (e.g. pause -> play */
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.eSourceFormatOverride = hXdmPP->stDMConfig.eSourceFormatOverride;

   /* SW7635-51: snapshot the STC offset (uiSoftwarePCROffset) when a picture is selected for display.
    * SW7435-657: bind the intermediate value to the picture.
    */
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.uiSoftwarePCROffset = pstSelectedPicture->stPicParms.stTSM.stDynamic.uiSwPcrOffsetUsedForEvaluation;;
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.bSwPcrOffsetValid = true;

   /* SW7425-2255: snapshot the display offset (uiPTSOffset) when a picture is selected for display.
    * SW7435-657: bind the intermediate value to the picture.
    */
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.uiPtsOffset = pstSelectedPicture->stPicParms.stTSM.stDynamic.uiPtsOffsetUsedForEvaluation;
   pstSelectedPicture->stPicParms.stDisplay.stDynamic.bPtsOffsetValid = true;

   /* If the PPB contained a coded PTS, save it for the application. */

   if ( true == pstSelectedPicture->pstUnifiedPicture->stPTS.bValid )
   {
      hXdmPP->stDMStatus.stCodedPTS.ui32RunningPTS = pstSelectedPicture->pstUnifiedPicture->stPTS.uiValue;
      hXdmPP->stDMStatus.stCodedPTS.ePTSType = BXDM_PictureProvider_PTSType_eCoded;
   }

   /* Update the public copy with the current GOP time code. */

   hXdmPP->stDMStatus.stGOPTimeCode = pstSelectedPicture->pstUnifiedPicture->stGOPTimeCode;

   BDBG_LEAVE( BXDM_PPQM_P_PromotePictureToDisplayStatus_isr );

   return;
}

BXDM_PictureProvider_P_Picture_Context * BXDM_PPQM_P_GetFirstPictureContext_isr( BXDM_PictureProvider_Handle hXdmPP )
{
   return &(hXdmPP->stDMState.stChannel.stPictureUnderEvaluation);
}

/*******************************************************************************
 **
 ** end "public" display queue management routines.
 **
 *******************************************************************************/
