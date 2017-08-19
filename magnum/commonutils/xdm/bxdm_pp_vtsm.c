/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bdbg.h"                /* Dbglib */

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_qm.h"
#include "bxdm_pp_fp.h"
#include "bxdm_pp_vtsm.h"

BDBG_MODULE(BXDM_PPVTSM);
BDBG_FILE_MODULE(BXDM_PPVTSM);

void BXDM_PPVTSM_P_VirtualStcSet_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   BDBG_ENTER(BXDM_PPVTSM_P_VirtualStcSet_isr);

   /* If we've selected the current picture using TSM, we set the
    * virtual STC (vSTC) to be equal to the actual STC so that we
    * can have smooth transitions from TSM --> VSYNC
    * mode. (e.g. when bVsyncModeOnPcrDiscontinuity is enabled) */

   /* PR52803: We don't want the vSTC to track the STC if it is
    * invalid */
   if (hXdmPP->stDMState.stChannel.stSelectedPicture.bValidated
       && (BXDM_PictureProvider_DisplayMode_eTSM == hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stDynamic.eSelectionMode)
       && (true == hXdmPP->stDMConfig.bSTCValid)
      )
   {
      /* The virutal STC = PCRoffset + STCsnapshot
       * SW7435-657: decouple the SW PCR offset from the virtual STC.  It will be added to the vSTC
       * during the TSM comparison. Previously BXDM_PPQM_P_GetCookedPcrOffset_isr was called here.
       */
      BXDM_PPQM_P_GetHwPcrOffset_isr(
         hXdmPP,
         &(hXdmPP->stDMState.stChannel.stSelectedPicture),
         &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole)
         );

      hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole += pLocalState->uiAdjustedStc;
      hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiFractional = 0;
   }

   BDBG_LEAVE(BXDM_PPVTSM_P_VirtualStcSet_isr);
   return;
}

void BXDM_PPVTSM_P_VirtualStcIncrement_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState
   )
{
   /* Virtual TSM - Setting the virtual STC (vSTC) */
   BDBG_ENTER(BXDM_PPVTSM_P_VirtualStcIncrement_isr);

   /* PR52803: If the STC is validated or we're in VSYNC display mode,
    * then we can exit trick mode transition */
   if ( ( true == hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition ) /* We're in a trick mode transition */
        && ( ( true == hXdmPP->stDMConfig.bSTCValid ) /* The STC is valid */
             || ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pLocalState->eDisplayMode ) /* We're in VSYNC display mode */
             || ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != pLocalState->uiSlowMotionRate ) /* We've entered into another trick mode */
           )
      )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] Exiting trick mode transition (stc: %x)",
                                 hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pLocalState->uiAdjustedStc ));
      hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition = false;
   }

   /* If in CRC mode
    * - use the delta PTS for the selected picture to increment the virtual STC
    * else
    * - use the delta STC to increment the virtual STC
    * (The delta STC is based on the display rate)
    */
   if ( true == hXdmPP->stDMConfig.bCRCMode )
   {
      if ( true == hXdmPP->stDMState.stChannel.stSelectedPicture.bValidated )
      {
         pLocalState->stSTCDelta = hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSDelta;
      }
      else
      {
         pLocalState->stSTCDelta.uiWhole = 0;
         pLocalState->stSTCDelta.uiFractional = 0;
      }
   }

#if 0
   BKNI_Printf("[eClockRate = %d, eMonitorRefreshRate = %d] --> deltaSTC=(%d, %d)\n",
               pLocalState->eClockRate, hXdmPP->stDMState.stDecode.eMonitorRefreshRate,
               pLocalState->stSTCDelta.uiWhole,
               pLocalState->stSTCDelta.uiFractional);
#endif

   /* If the selected picture is not valid (i.e. on start-up) then we
    * always set PTS initialized to FALSE so that we reassign PTS
    * until we have a valid picture displayed */
   if ( !hXdmPP->stDMState.stChannel.stSelectedPicture.bValidated )
   {
      hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized = false;
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] vPTS invalidated: startup",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
   }

   /* Handle decoder trick modes */
   if ( true == hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition )
   {
      /* PR52803: Don't increment vSTC when in a trick mode
       * transition */
      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM].uiWhole = 0;
      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM].uiFractional = 0;
   }
   else if ( pLocalState->uiSlowMotionRate )
   {
      bool bStallStc = ( hXdmPP->stDMState.stDecode.stNonRealTime.bLastStallStcValid
                         && hXdmPP->stDMState.stDecode.stNonRealTime.bLastStallStc );

      /* We are not paused, so we can increment the vSTC on this
       * VSYNC */
      if ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != pLocalState->uiSlowMotionRate )
      {
         /* We're not in normal playback. Multiply the deltaSTC by
          * the playback rate, before adding it to the vSTC */
         BXDM_PPFP_P_FixPtFractionalMul_isr(
            &pLocalState->stSTCDelta,
            pLocalState->uiSlowMotionRate,
            BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE,
            &pLocalState->stSTCDelta);
      }

      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM] = pLocalState->stSTCDelta;

      /* SW7445-2544: if the STC was stalled on the previous vsync, don't increment the
       * virtual STC on this vsync. This showed up as a problem when encoding ES streams
       * in vsync mode. The "correct" solution might be to set pLocalState->stSTCDelta to '0'.
       * For now keep the change local to this routine. 6/24/15 */

      if ( false == bStallStc )
      {
         /* Increment the vSTC by delta STC (which already has taken
          * slow motion into account).
          * SW7425-1264: if the "physical" STC is running in reverse, interpolate the
          * virtual STC in reverse as well. */

         if ( true == pLocalState->bUsingSwStcToRunInReverse )
         {
            BXDM_PPFP_P_FixPtSub_isr(
               &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
               &pLocalState->stSTCDelta,
               &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
               );
         }
         else
         {
            BXDM_PPFP_P_FixPtAdd_isr(
               &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
               &pLocalState->stSTCDelta,
               &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
               );
         }
      }

   }
   else
   {
      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM].uiWhole = 0;
      pLocalState->stEffectiveSTCDelta[BXDM_PictureProvider_DisplayMode_eVirtualTSM].uiFractional = 0;

      /* We're paused, so we only increment the STC if we need to
       * handle a frame advance request */

      /* PR51720: Frame advance is now performed by doing one field
       * advance per vsync until we get to the next frame */
      if ( BXDM_PictureProvider_FrameAdvanceMode_eFrameByField == hXdmPP->stDMConfig.eFrameAdvanceMode )
      {
         /* We ADD to the existing frame advance count (vs setting it)
          * to ensure we handle FRAME advance requests while in the
          * middle of honoring a previous one */
         if ( 0 == hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount )
         {
            hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount += hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.uiNumElements - hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stDisplay.stDynamic.uiSelectedElement;
         }
         else
         {
            BXVD_DBG_WRN(hXdmPP,("%x:[%02x.xxx] Prior Frame Advance was not finished, yet.  Frame may not advance properly on 3:2 content",
                                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
            hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount += hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.uiNumElements;
         }
      }

      if ( hXdmPP->stDMConfig.eFrameAdvanceMode != BXDM_PictureProvider_FrameAdvanceMode_eOff )
      {
         hXdmPP->stDMState.stDecode.stVTSM.eFrameAdvanceMode = hXdmPP->stDMConfig.eFrameAdvanceMode;
         hXdmPP->stDMConfig.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;

         if ( true == hXdmPP->stDMState.stChannel.bPostFlushDecode )
         {
            /* SW7425-2656:  If this is the first picture after a "flush" and the system is paused,
             * we want to hold off displaying the picture until the system is unpaused or a
             * "frame advance" command is issued.  The second case is handled here by clearing
             * "bPostFlushDecode" flag.  Clearing this flag prevents vPTS from being
             * reset in BXDM_PPVTSM_P_VirtualPtsInterpolate_isr.
             */
            hXdmPP->stDMState.stChannel.bPostFlushDecode = false;

            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] clear bPostFlushDecode due to frame advance",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
         }

      }

      /* Handle frame advance */
      switch ( hXdmPP->stDMState.stDecode.stVTSM.eFrameAdvanceMode )
      {
         case BXDM_PictureProvider_FrameAdvanceMode_eField:
            /* We're doing a FIELD advance, so we need to increment
             * the vSTC by one field time, which is deltaPTS */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] Frame Advance: Field",
                                          hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                          BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));

            /* SW7425-1264: if the "physical" STC is running in reverse, interpolate the
             * virtual STC in reverse as well. */

            if ( true == pLocalState->bUsingSwStcToRunInReverse )
            {
               BXDM_PPFP_P_FixPtSub_isr(
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                  &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSDelta),
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                  );
            }
            else
            {
               BXDM_PPFP_P_FixPtAdd_isr(
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                  &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSDelta),
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                  );
            }

            /* Frame advance is a one-shot request, so we reset it to
             * disabled when we've honored the request */
            hXdmPP->stDMState.stDecode.stVTSM.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;

            pLocalState->uiState2Bits |= BXDM_PPDBG_State2_FrameAdvance;
            break;

         case BXDM_PictureProvider_FrameAdvanceMode_eFrame:
            /* We're doing a FRAME advance, so we need to set the
             * vSTC to the vPTS of the NEXT PPB */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] Frame Advance: Frame",
                                       hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                       BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ) ));
            hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC = hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSOfNextPPB[BXDM_PictureProvider_P_PTSIndex_eVirtual];

            /* We need to account for the current field inversion
             * correct offset to ensure the next frame is actually
             * selected by TSM.
             * SW7425-1264: if the "physical" STC is running in reverse,
             * subtract the FIC offset. */

            if ( true == pLocalState->bUsingSwStcToRunInReverse )
            {
               BXDM_PPFP_P_FixPtSub_isr(
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                  &(hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset),
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                  );
            }
            else
            {
               BXDM_PPFP_P_FixPtAdd_isr(
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                  &(hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset),
                  &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                  );
            }

            /* Frame advance is a one-shot request, so we reset it to
             * disabled when we've honored the request */
            hXdmPP->stDMState.stDecode.stVTSM.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;

            pLocalState->uiState2Bits |= BXDM_PPDBG_State2_FrameAdvance;
            break;

         case BXDM_PictureProvider_FrameAdvanceMode_eFrameByField:
            /* We're doing a FRAME advance by fields, so we need to do
             * field advances until we get to the next frame */
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] Frame Advance: Frame by Field[%d]",
                                      hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                      BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                      hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount ));

            if ( hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount > 0 )
            {
               /* SW7425-1264: if the "physical" STC is running in reverse,
                * interpolate the virtual STC in reverse as well. */

               if ( true == pLocalState->bUsingSwStcToRunInReverse )
               {
                  BXDM_PPFP_P_FixPtSub_isr(
                     &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                     &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSDelta),
                     &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                     );
               }
               else
               {
                  BXDM_PPFP_P_FixPtAdd_isr(
                     &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC),
                     &(hXdmPP->stDMState.stChannel.stSelectedPicture.stPicParms.stTSM.stStatic.stPTSDelta),
                     &(hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC)
                     );
               }

               hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount--;
            }

            if ( 0 == hXdmPP->stDMConfig.uiFrameAdvanceByFieldCount )
            {
               hXdmPP->stDMState.stDecode.stVTSM.eFrameAdvanceMode = BXDM_PictureProvider_FrameAdvanceMode_eOff;
            }

            pLocalState->uiState2Bits |= BXDM_PPDBG_State2_FrameAdvance;
            break;

         default:
            break;
      }
   }

   BDBG_LEAVE(BXDM_PPVTSM_P_VirtualStcIncrement_isr);
   return;
}

void BXDM_PPVTSM_P_VirtualPtsInterpolate_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_PictureProvider_P_Picture_Context* pstPrevPicture,
   BXDM_PictureProvider_P_Picture_Context* pstSelectedPicture
   )
{
   BXDM_PPFP_P_DataType stVirtualPTSTemp;
   BXDM_PPFP_P_DataType stVirtualPTSOfNextPPB;
   uint32_t i;

   BDBG_ENTER(BXDM_PPVTSM_P_VirtualPtsInterpolate_isr);

   BSTD_UNUSED( pLocalState );

   /* Virtual TSM */

   /* Re-initialize vPTS if the currently selected picture did not
    * pass TSM */

   /* PR52803: We don't want to do a vPTS reset if we're in a trick mode transition to prevent the next
    * picture from being displayed too early if entering back into a trick mode, e.g. pause->play->pause.
    * SW7445-491: we also don't want to reset it if the TMS result callback requested a "drop" of
    * the previous picture.
    * SWSTB-6219: "stDynamic.bForceDisplay": also don't reset the vPTS on the second picture if the
    * first picture was displayed due to the channel change mode being "first picture preview". */

   if ( ( BXDM_PictureProvider_TSMResult_ePass != pstPrevPicture->stPicParms.stTSM.stDynamic.eTsmResult )
        && ( false == hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition )
        && ( BXDM_PictureProvider_PictureHandlingMode_eDrop != pstPrevPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
        &&  ( false == pstPrevPicture->stPicParms.stDisplay.stDynamic.bForceDisplay )
      )
   {
      hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized = false;
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.%03x] vPTS invalidated: TSM result: %d",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                pstPrevPicture->stPicParms.stTSM.stDynamic.eTsmResult ));
   }

   /* We interpolate the vPTS from the previous PPB */
   if ( (pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode == BXDM_PictureProvider_DisplayMode_eTSM)
        && ( false == pstPrevPicture->bValidated ) )
   {
      /* If we're in TSM mode and the previous picture is invalid, we
       * set the vPTS of the next PPB based on the CURRENT picture's
       * ACTUAL interpolated PTS.  This causes the vPTS to track
       * actual PTS when in TSM mode so that we can have a seamless
       * transition from TSM --> VSYNC mode */
      BXDM_PPQM_P_GetPredictedPtsWithFrac_isr(pstPicture,
                                          BXDM_PictureProvider_P_PTSIndex_eActual,
                                          &stVirtualPTSOfNextPPB);

      /* We need to add the JRC jitter offset so that the vPTS tracks the de-jittered PTS values */
      stVirtualPTSOfNextPPB.uiWhole += pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection;

      /* Since we're setting the vPTS based on an actual PTS, we
       * indicate that the vPTS has been initialized */
      hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized= true;

      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.%03x] initialize vPTS TSM mode: vPTS:%08x includes jitter offset of %08x",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                stVirtualPTSOfNextPPB.uiWhole,
                                pstPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection ));
   }
   else
   {
      if (pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode == BXDM_PictureProvider_DisplayMode_eTSM)
      {
         /* If we're in TSM mode, we set the vPTS of this picture based on the
          * previous picture's ACTUAL interpolated PPB.
          * SW7445-491: if the TMS result callback requested a "drop" of the previous picture,
          * set the vPTS of this picture based on the selected picture.  This results in the
          * vPTS advancing by a picture time as opposed to two picture times.  */
         if ( BXDM_PictureProvider_PictureHandlingMode_eDrop == pstPrevPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
         {
            BXDM_PPQM_P_GetPredictedPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eActual, &stVirtualPTSTemp );
         }
         else
         {
            BXDM_PPQM_P_GetPredictedPtsWithFrac_isr( pstPrevPicture, BXDM_PictureProvider_P_PTSIndex_eActual, &stVirtualPTSTemp );
         }

        /* We need to add the JRC jitter offset so that the vPTS tracks the de-jittered PTS values */
         stVirtualPTSTemp.uiWhole += pstPrevPicture->stPicParms.stTSM.stStatic.iPTSJitterCorrection;

         hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized = true;
      }
      else
      {
         /* If we're in VSYNC mode, we set the vPTS of this picture based on the
          * previous picture's VIRTUAL interpolated PPB.
          * SW7445-491: if the TMS result callback requested a "drop" of the previous picture,
          * set the vPTS of this picture based on the selected picture.  This results in the
          * vPTS advancing by a picture time as opposed to two picture times.  */
         if ( BXDM_PictureProvider_PictureHandlingMode_eDrop == pstPrevPicture->stPicParms.stTSM.stDynamic.ePictureHandlingMode )
         {
            BXDM_PPQM_P_GetPredictedPtsWithFrac_isr( pstSelectedPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, &stVirtualPTSTemp );
         }
         else
         {
            BXDM_PPQM_P_GetPredictedPtsWithFrac_isr( pstPrevPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, &stVirtualPTSTemp );
         }

         /* Handle vPTS skew */
         /* Detect and correct for PTS skew in vTSM mode.  Skew can occur
          * when the delivery queue runs dry for one or more vsyncs and
          * the interpolated PTS for the NEXT PPB is no longer accurate.
          * The queue can run dry for multiple reasons...errors in the
          * stream, a stream wrap, etc. In such a case, without this
          * logic, subsequent PPBs would always have been evaluated as
          * LATE because the interpolated vPTS would be far behind the
          * vSTC */

         /* PR52803: We don't want to do vPTS skew detection when in a
          * trick mode transition to prevent the next picture from
          * being displayed too early if entering back into a trick
          * mode.  E.g. pause->play->pause */
         if ( ( hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized )
              && ( false == hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition )
            )
         {
            uint32_t uiStcPtsDifference = 0;
            /* PR57049: We use the dStcPts when last displayed (vs the evaluated) one
             * because the evaluated one has been updated already before we started
             * evaluating this picture */
            if ( pstSelectedPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceDisplayed > 0 )
            {
               uiStcPtsDifference = (uint32_t) pstSelectedPicture->stPicParms.stTSM.stDynamic.iStcPtsDifferenceDisplayed;
            }

            if ( uiStcPtsDifference >= ( pstSelectedPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiWhole +
                                         ( pstSelectedPicture->stPicParms.stTSM.stStatic.stPTSDelta.uiFractional ? 1 : 0 ) ) )
            {
               hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized = false;
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.%03x] vPTS invalidated: skew Detected, i.e. the DQ ran dry uiStcPtsDifference:%08x)",
                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                         uiStcPtsDifference ));
            }
         }

         /* We need to override the predicted virtualPTS for this PPB */
         if ( false == hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized)
         {
            /* We've just entered into VSYNC mode, so we need to set the
             * vPTS to a default starting value equal to the starting
             * vSTC value.  Also, When in vTSM mode, make sure we don't
             * assign a stale PTS value for this PPB, so we need to
             * assign this PPB the larger of the current STC or the
             * interpolated PTS */

            stVirtualPTSTemp = hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC;

            /* SW7445-3432: uiSoftwarePCROffset will be added to the virtual stc in
             * BXDM_PPTSM_S_CompareStcAndPts_isr.  Hence the need to include it here
             * when initializing the virtual PTS. */

            stVirtualPTSTemp.uiWhole += hXdmPP->stDMConfig.uiSoftwarePCROffset;

            /* SW7425-2656:  If this is the first picture after a "flush" and the system is paused,
             * we don't want to display this picture until the system is unpaused or a "frame advance"
             * command is issued.  This is achieved by setting the vPTS to be larger than the vSTC.
             * The value doesn't matter since the vPTS will be reset again once the system is unpaused.
             *
             * "hold last picture" is implicit with the "flush"; by forcing this picture to be a
             * little early, the picture that was being displayed will continue to be displayed.
             *
             * In contrast to the behavior after a "flush", if the system is paused prior to calling BXVD_StartDecode
             * in the normal manner, the first picture will be displayed.
             */
            if ( ( true == hXdmPP->stDMState.stChannel.bPostFlushDecode )
                 && ( 0 == pLocalState->uiSlowMotionRate )
                 && ( 1 == pstPicture->stPicParms.uiPPBIndex )
               )
            {
               /* The amount added doesn't matter. */
               stVirtualPTSTemp.uiWhole += 10;

               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.%03x] initialize vPTS for 1st picture with system paused: vStc:%08x vPTS:%08x",
                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                         hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole,
                                         stVirtualPTSTemp.uiWhole ));
            }
            else
            {
               uint32_t uiPtsOffset;
               uint32_t uiVsyncOffset;

               /* SW7425-2255: once a picture has been selected for display, use the
                * offset bound to the picture.
                */
               BXDM_PPQM_P_GetPtsOffset_isr( hXdmPP, pstPicture, &uiPtsOffset );

               /* Subtract the PTS offset */
               stVirtualPTSTemp.uiWhole -= uiPtsOffset;

               /* Subtract the field inversion correction offset */
               BXDM_PPFP_P_FixPtSub_isr(
                  &stVirtualPTSTemp,
                  &hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset,
                  &stVirtualPTSTemp);

               /* SW7439-353:  This issue showed up when the system was running in vsync mode.
                * The stream being played was 29.97 Hz interlaced content.  The monitor refresh rate was
                * changed between the first and second picture, i.e. it was changed form 60 Hz to 59.94.
                * This caused the second picture to just miss passing the TSM test by one vsync. Due to
                * the interaction with the FIC (field inversion correct) logic, the system ended up in a
                * state where two pictures passed on every fourth vsync instead of one picture passing
                * on every second vsync. Essentially every other picture was dropped. The FIC was constantly
                * switching between first and second slot.
                *
                * The fix is to move the vPTS ahead in time a little bit to avoid being right on the edge of
                * the pass window. This is similar to the effect of the STC jitter correction logic.
                */
               uiVsyncOffset = ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pLocalState->eDisplayMode ) ? 10 : 0;
               stVirtualPTSTemp.uiWhole -= uiVsyncOffset;

               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.%03x] initialize vPTS : vPTS:%08x = vStc:%08x - PTSOffset:%08x - fieldInv:%08x - uiVsyncOffset:%d + uiSoftwarePCROffset:%d",
                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                         stVirtualPTSTemp.uiWhole,
                                         hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole,
                                         uiPtsOffset,
                                         hXdmPP->stDMState.stDecode.stFieldInversionCorrectionPTSOffset.uiWhole,
                                         uiVsyncOffset,
                                         hXdmPP->stDMConfig.uiSoftwarePCROffset ));

            }

            hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized = true;

         }  /* end of if ( false == hXdmPP->stDMState.stDecode.stVTSM.bVirtualPTSInitialized ) */

      }

      /* Set the PTS for each element in the PPB */
      stVirtualPTSOfNextPPB = stVirtualPTSTemp;
      for (i = 0; i < pstPicture->stPicParms.stTSM.stStatic.uiNumElements; i++)
      {

         BXDM_PPQM_P_SetPtsWithFrac_isr(pstPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, i, stVirtualPTSTemp);

#if 0
         BKNI_Printf("vPTS[%d]: %08x\n", i, stVirtualPTSTemp.uiWhole);
#endif
         /* SW7425-1264: if the "physical" STC is running in reverse, interpolate the
          * virtual PTS's in reverse as well. */

         if ( true == pLocalState->bUsingSwStcToRunInReverse )
         {
            BXDM_PPFP_P_FixPtSub_isr(
               &stVirtualPTSTemp,
               &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
               &stVirtualPTSTemp
               );

            /* Calculate the next PPB's virtual PTS */
            BXDM_PPFP_P_FixPtSub_isr(
               &stVirtualPTSOfNextPPB,
               &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
               &stVirtualPTSOfNextPPB
               );
         }
         else
         {
            BXDM_PPFP_P_FixPtAdd_isr(
               &stVirtualPTSTemp,
               &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
               &stVirtualPTSTemp
               );

            /* Calculate the next PPB's virtual PTS */
            BXDM_PPFP_P_FixPtAdd_isr(
               &stVirtualPTSOfNextPPB,
               &(pstPicture->stPicParms.stTSM.stStatic.stPTSDelta),
               &stVirtualPTSOfNextPPB
               );
         }

      }

#if 0
      BKNI_Printf("vPTSPredicted: %d\n", stVirtualPTSOfNextPPB.uiWhole);
#endif
   }

   BXDM_PPQM_P_SetPredictedPtsWithFrac_isr(pstPicture, BXDM_PictureProvider_P_PTSIndex_eVirtual, stVirtualPTSOfNextPPB);

   BDBG_LEAVE(BXDM_PPVTSM_P_VirtualPtsInterpolate_isr);
   return;
} /* end of BXDM_PPVTSM_P_VirtualPtsInterpolate_isr() */

void BXDM_PPVTSM_P_VirtualStcGet_isr(
   const BXDM_PictureProvider_Handle hXdmPP,
   uint32_t* puiStc
   )
{
   bool bInterpolateInReverse;

   *puiStc = hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiWhole;

   /* SW7425-1264: only round the virtual STC up if the "physical" STC is
    * running in the forward direction.
    */
   bInterpolateInReverse = ( true == hXdmPP->stDMConfig.stClockOverride.bEnableClockOverride );
   bInterpolateInReverse &= ( hXdmPP->stDMConfig.stClockOverride.iStcDelta < 0 );

   if ( 0 != hXdmPP->stDMState.stDecode.stVTSM.stVirtualSTC.uiFractional
        && false == bInterpolateInReverse
      )

   {
      (*puiStc)++;
   }

   return;
} /* end of BXDM_PPVTSM_P_VirtualStcGet_isr() */

void BXDM_PPVTSM_P_ClipTimeTrickModeTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState
   )
{
   BSTD_UNUSED(pLocalState);

   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eVTSM, "%x:[%02x.xxx] Entering trick mode transition (stc: %x)",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pLocalState->uiAdjustedStc ));
   hXdmPP->stDMState.stDecode.stVTSM.bTrickModeTransition = true;
}
