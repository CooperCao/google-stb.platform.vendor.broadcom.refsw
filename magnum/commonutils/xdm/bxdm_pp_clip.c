/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"                /* Dbglib */

#include "bxdm_pp.h"
#include "bxdm_pp_priv.h"
#include "bxdm_pp_qm.h"

BDBG_MODULE(BXDM_PPCLIP);       /* Register software module with debug interface */
BDBG_FILE_MODULE(BXDM_PPCLIP);

#define BXDM_PictureProvider_P_CLIPTIME_PTS_WRAP_THRESHOLD (0x80000000)
#define BXDM_PictureProvider_P_CLIPTIME_STC_WRAP_THRESHOLD (0x80000000)

void BXDM_PPCLIP_P_ClipTimeCallbackTriggerHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicture
   )
{
   BXDM_PictureProvider_P_ClipTimeV2 * pstCurrentClip = &( hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip );

   /* Check for clip start */
   if ( ( true == hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable )
        && ( true == pstCurrentClip->stTime.bValid )
        && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eClipStartOnly == pstCurrentClip->stTime.eType )
           )
        && ( true == pstCurrentClip->stStartEvent.bOccurred )
        && ( true == pstCurrentClip->stDisplayEvent.bOccurred )
      )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Trigger Clip Start",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
      hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = false;
      pLocalState->stCallbackInfo.stClipStart.bTriggered = true;
      pLocalState->stCallbackInfo.stClipStart.stClipEventInfo.uiPTS = pstCurrentClip->stStartEvent.uiTime;
      pLocalState->stCallbackInfo.stClipStart.stClipEventInfo.uiClipId = pstCurrentClip->stTime.uiId;

      if ( BXDM_PictureProvider_ClipTimeType_eClipStartOnly == pstCurrentClip->stTime.eType )
      {
         /* Auto disable clip logic in start only mode */
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Auto-Disable Clip Logic (ClipStartOnly)",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
         BKNI_Memset( &hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip,
                      0,
                      sizeof( BXDM_PictureProvider_P_ClipTimeV2 ) );
      }
   }
   else if ( false == pLocalState->stCallbackInfo.stClipStart.bTriggered )
   {
      /* Check for clip stop */
      if ( ( true == hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable )
           && ( true == pstCurrentClip->stTime.bValid )
           && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
                || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
                || ( BXDM_PictureProvider_ClipTimeType_eClipStopOnly == pstCurrentClip->stTime.eType )
              )
           && ( true == pstCurrentClip->stStartEvent.bOccurred )
           && ( true == pstCurrentClip->stDisplayEvent.bOccurred )
           && ( true == pstCurrentClip->stStopEvent.bOccurred )
         )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Trigger Clip Stop",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
         hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = false;
         pLocalState->stCallbackInfo.stClipStop.bTriggered = true;
         pLocalState->stCallbackInfo.stClipStop.stClipEventInfo.uiPTS = pstCurrentClip->stStopEvent.uiTime;
         pLocalState->stCallbackInfo.stClipStop.stClipEventInfo.uiClipId = pstCurrentClip->stTime.uiId;
         /* SWBLURAY-22390: see code block below hXdmPP->stDMState.stChannel.stClipInfo.bClipActive = false;*/
      }

      /* Check for picture marker */
      if ( ( true == hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable )
           && ( true == pstCurrentClip->stTime.bValid )
           && ( true == pstCurrentClip->stMarkerEvent.bOccurred )
           && ( ( false == pstCurrentClip->stMarkerEvent.bDelayedCallback )
                || ( true == pstCurrentClip->stStopEvent.bOccurred )
                /* PR56292: Trigger the marker callback if the first
                 * picture containing the marker is displayed */
                || ( ( BXDM_PictureProvider_TSMResult_ePass == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
                     && ( NULL != pstPicture->pstUnifiedPicture ) /*SW7445-3404: fix for converity 1/11/16 */
                     && ( pstCurrentClip->stMarkerEvent.uiTime == pstPicture->pstUnifiedPicture->stPictureMarker.uiValue ) )
              )
         )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Trigger Marker",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
         hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable = false;
         pLocalState->stCallbackInfo.stPictureMarker.bTriggered = true;
         pLocalState->stCallbackInfo.stPictureMarker.stClipEventInfo.uiPTS = pstCurrentClip->stMarkerEvent.uiTime;
         pLocalState->stCallbackInfo.stPictureMarker.stClipEventInfo.uiClipId = pstCurrentClip->stTime.uiId;
      }

      /* SWBLURAY-22390: The problem was observed when playing the PIP from USB and main from optical disk.
       * The video clips are in sync and transition to the next clip at the same time.   At the end of the
       * PIP clip, two vsync's were not delivered to XDM.  This was probably due to accessing video data on
       * the USB drive.  The failure was as follows;
       * - due to the two missing vsync's, the next to the last picture of the 1st clip triggers the 1st clip stop
       * - the occurrence of the clip stop results in the loading of the start/stop parameters for the 2nd clip
       * - the last picture of the 1st clip is within one picture time of the end of the 2nd clip, this results in
       *   the 2nd clip start and stop callbacks being generated off this picture
       *
       * The fix is to not load the parameters for the next clip until both the stop event has occurred
       * and the marker has been seen.
       */
      if ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
      {
         if ( ( true == pstCurrentClip->stStopEvent.bOccurred )
               && ( true == pstCurrentClip->stMarkerEvent.bOccurred )
            )
         {
            hXdmPP->stDMState.stChannel.stClipInfo.bClipActive = false;
         }
      }
      else
      {
         if ( true == pstCurrentClip->stStopEvent.bOccurred )
         {
            hXdmPP->stDMState.stChannel.stClipInfo.bClipActive = false;
         }
      }

   }

}     /* end of BXDM_PPCLIP_P_ClipTimeCallbackTriggerHandler_isr() */

void BXDM_PPCLIP_P_Reset_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   /* Reset clip info structure */
   BKNI_Memset( &hXdmPP->stDMState.stChannel.stClipInfo, 0, sizeof (BXDM_PictureProvider_P_ClipInfo) );
   /* Initialize last timing marker default start value in AVD */
   hXdmPP->stDMState.stChannel.stClipInfo.uiLastMarker = 0xFFFFFFFF;
}

bool BXDM_PPCLIP_P_ClipTimeTSMHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   bool bEvaluateActualPts,
   BXDM_PictureProvider_TSMResult *peTsmState
   )
{
   BXDM_PictureProvider_P_ClipTimeV2 * pstCurrentClip = &( hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip );
   bool bResult = false;
   BXDM_PPFP_P_DataType stPts;

   /* Get Actual PTS value for this picture for ClipStart/ClipStop
    * calculations */
   BXDM_PPQM_P_GetPtsWithFrac_isr( pstPicture,
                               BXDM_PictureProvider_P_PTSIndex_eActual,
                               0,
                               &stPts );

   /* Check for clip start event */
   if ( ( true == pstCurrentClip->stTime.bValid )
        && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eClipStartOnly == pstCurrentClip->stTime.eType )
           )
        && ( false == pstCurrentClip->stStartEvent.bOccurred )
        && ( false == pstPicture->stPicParms.stTSM.stDynamic.bClipTransition )
        && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
        && ( true == bEvaluateActualPts )
        && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bGeneratedClipStop ) /* SWBLURAY-21565: only allow the last picture of a clip to generate one stop event */
        /* PR50077: Handle PTS wrap scenarios for clip start event.
         * We need to handle all of the following start scenarios
         * since both the PTS and clip start times are unsigned values
         * and PTS wraps are allowed:
         *
         *  0) (clip start = 0) - a clip start of 0 indicates the clip
         *   start event should occur on the very first picture of the
         *   clip
         *
         *  1) (pts >= clip start) - this is a normal scenario, where
         *   the first pts value is larger than the clip start time
         *   and we don't want to drop anything.  E.g.
         *    - clip start = 3000
         *    - clip pts values = [4500, 6000, 7500, ...]
         *    - we want to trigger the clip start on pts=4500 and not
         *     drop anything
         *
         *  2) (pts >> clip start) - this is a pts wrap scenario,
         *   where the clip start is very small, but the first pts
         *   values are very large.  In this scenario, we want to drop
         *   all picture until the wrap and then use clip start to
         *   filter out the rest. E.g.
         *     - clip start = 3000
         *     - clip pts values = [-3000, -1500, 0, 1500, 3000, ...]
         *     - we want to drop pictures with pts = [-3000, -1500, 0,
         *      1500] and trigger the clip start on pts=3000
         *
         *  3) (pts < clip start) - this is the typical scenario,
         *   where we want to drop all pts values that are less than
         *   the clip start time. E.g.
         *    - clip start = 3000
         *    - clip pts values = [0, 1500, 3000, ...]
         *    - we want to drop pictures with pts=[0, 1500] and
         *     trigger the clip start on pts=3000
         *
         *  4) (pts << clip start) - this is a pts wrap scenario,
         *   where the clip start is very large, but the first pts
         *   values are very small.  In this scenario, we don't want
         *   to drop anything.  E.g.
         *      - clip start = -3000 (0xFFFFF448)
         *      - clip pts values = [4500, 6000, 7500, ...]
         *    - we want to trigger the clip start on pts=4500 and not
         *     drop anything
         *
         * In the end, we want to trigger the start event if only
         * cases 1) or 4) occur.
         */
        && ( ( ( 0 == pstCurrentClip->stTime.uiStart ) /* Case 0: start time is 0 */
               && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType ) )
             || ( ( stPts.uiWhole >= pstCurrentClip->stTime.uiStart ) /* Case 1: pts is larger than start time, but not too large */
                  && ( ( stPts.uiWhole - pstCurrentClip->stTime.uiStart ) < BXDM_PictureProvider_P_CLIPTIME_PTS_WRAP_THRESHOLD ) )
             || ( ( stPts.uiWhole < pstCurrentClip->stTime.uiStart ) /* Case 4: pts is less than the start time, but much lesser */
                  && ( ( pstCurrentClip->stTime.uiStart - stPts.uiWhole ) > BXDM_PictureProvider_P_CLIPTIME_PTS_WRAP_THRESHOLD ) )
           )
      )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Start Event:: Start Time:%08x  PTS:%08x",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                pstCurrentClip->stTime.uiStart,
                                stPts.uiWhole ));
      pstCurrentClip->stStartEvent.bOccurred = true;

      /* We send the coded PTS if it exists, otherwise, we send the start time */
      if ( true == pstPicture->pstUnifiedPicture->stPTS.bValid )
      {
         pstCurrentClip->stStartEvent.uiTime = stPts.uiWhole;
      }
      else
      {
         pstCurrentClip->stStartEvent.uiTime = pstCurrentClip->stTime.uiStart;
      }
   }

   /* Check for clip start/stop scenario */
   if ( ( true == pstCurrentClip->stTime.bValid )
        && ( false == pstPicture->stPicParms.stTSM.stDynamic.bClipTransition )
        && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
        && ( ( ( true == bEvaluateActualPts )
               && ( BXDM_PictureProvider_DisplayMode_eTSM == pLocalState->eSelectionMode ) ) ||
             ( ( false == bEvaluateActualPts )
               && ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pLocalState->eSelectionMode ) )
           )
      )
   {
      /* Drop pictures that are either before the clip start or after
       * the clip stop */
      if ( ( false == pstCurrentClip->stStartEvent.bOccurred )
           && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
                || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
                || ( BXDM_PictureProvider_ClipTimeType_eClipStartOnly == pstCurrentClip->stTime.eType )
              ) )
      {
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Drop pre-start:: Start Time:%08x  PTS:%08x",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                   pstCurrentClip->stTime.uiStart,
                                   stPts.uiWhole ));
         bResult = true;
         *peTsmState = BXDM_PictureProvider_TSMResult_eDrop;
      }
      else if ( ( true == pstCurrentClip->stStopEvent.bOccurred )
                && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
                     || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
                     || ( BXDM_PictureProvider_ClipTimeType_eClipStopOnly == pstCurrentClip->stTime.eType )
                   ) )
      {
         /* Drop pictures after a stop event has occurred */
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Drop post-stop:: Stop Time:%08x  PTS:%08x",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                   pstCurrentClip->stTime.uiStop,
                                   stPts.uiWhole ));
         bResult = true;
         *peTsmState = BXDM_PictureProvider_TSMResult_eDrop;
      }
   }

   /* Handle dummy PPBs */
   if ( false == pstPicture->pstUnifiedPicture->stBufferInfo.bValid )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Dummy PPB",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
      /* PR47517: If the start event has already occurred, and we're
       * seeing a dummy PPB, we should assume the display event has
       * occurred because a dummy PPB indicates the end of a clip.
       * The display event gates the generation of the clip
       * start/stop/marker interrupts.  This logic is needed in the
       * case where the clip only contains a dummy PPB which actually
       * won't go out to the display. */
      /* SW7601-175: To handle picture-less clips, we need to assume
       * the start and display event have occurred as soon as we see
       * a dummy PPB. */
      if ( ( true == pstCurrentClip->stTime.bValid )
           && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
         )
      {
         pstCurrentClip->stStartEvent.bOccurred = true;
         pstCurrentClip->stDisplayEvent.bOccurred = true;
      }

      bResult = true;
      *peTsmState = BXDM_PictureProvider_TSMResult_eDrop;
   }

   return bResult;
}

void BXDM_PPCLIP_P_ClipTimeTSMTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BXDM_PictureProvider_P_Picture_Context* pstPicture,
   BXDM_PictureProvider_TSMResult *peTsmState
   )
{
   BXDM_PictureProvider_P_ClipTimeV2 * pstCurrentClip = &( hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip );

   /* Handle ClipStop */
   if ( ( true == pstCurrentClip->stTime.bValid ) /* We're in a valid clip */
        && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eEnableBoth == pstCurrentClip->stTime.eType )
             || ( BXDM_PictureProvider_ClipTimeType_eClipStopOnly == pstCurrentClip->stTime.eType )
           )
        && ( true == pstCurrentClip->stStartEvent.bOccurred ) /* A clip has started */
        && ( true == pstCurrentClip->stDisplayEvent.bOccurred )
        && ( false == pstCurrentClip->stStopEvent.bOccurred ) /* The clip has not already stopped */
        && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bGeneratedClipStop ) ) /* SWBLURAY-21565: only allow the last picture of a clip to generate one stop event */
   {
      /* Handle "faking" clip stop time for End-of-clip (EOC) Mode (stop time not provided by "app")  */
      if ( ( 0xFFFFFFFF == pstCurrentClip->stTime.uiStop )
           && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
            && ( 0 != pLocalState->uiSlowMotionRate )                      /* We're not in a pause state */
            && ( true == pstCurrentClip->stMarkerEvent.bOccurred ))        /* The marker has been seen */
         {
         BXDM_PictureProvider_P_Picture_Context* pstPrevPicEvaluated = &(hXdmPP->stDMState.stChannel.stPreviousPictureUnderEvaluation);
         uint32_t uiPredPTS;

         /* Use previous picture under evaluation to predict the end, since the selected picture may not be the end if
            intervening frames are dropped */
         BXDM_PPQM_P_GetPredictedPts_isr(pstPrevPicEvaluated,
                                     BXDM_PictureProvider_P_PTSIndex_eActual,
                                     &uiPredPTS );

         /* Set clip stop time to be the predicted PTS of the next picture from the currently selected picture (i.e. the
            expected start time of the next clip) - 1 */
         pstCurrentClip->stTime.uiStop = uiPredPTS - 1;
         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Stop Time Override: 0xFFFFFFFF -> 0x%08x",
                                      hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                      BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                      pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                      pstCurrentClip->stTime.uiStop ));
      }
      /* Handle Clipstop in conditions other than EOC mode
         NOTE: The above override will result in a fall-through to the following code
         (non-EOC mode, or non-DVD mode) */
      if ( ( 0xFFFFFFFF != pstCurrentClip->stTime.uiStop )
            || ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility != pstCurrentClip->stTime.eType ))
      {
         if ( ( BXDM_PictureProvider_DisplayMode_eTSM == pLocalState->eDisplayMode )
              || ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility != pstCurrentClip->stTime.eType )
            )
         {
            /* Handle ClipStop xTSM modes based on actual PTS
             * values */

            /* PR51946: We need to account for STC/PTS jitter, so
             * we offset the stop time by the jitter threshold */
            BXDM_PPFP_P_DataType stPts;
            BXDM_PPFP_P_DataType stEffectivePTS;

            /* Handle ClipStop in vTSM mode */
            /* PR50077: Handle PTS wrap scenarios for clip stop
             * event (vTSM). We need to handle all of the following
             * stop scenarios since both the PTS and clip stop
             * times are unsigned values and PTS wraps are allowed:
             *
             *  1) (pts >= clip stop) - this is a normal scenario,
             *   where the pts value is larger than the clip stop
             *   time and we want to trigger the stop event right
             *   away. E.g.
             *    - clip stop = 3000
             *    - clip pts values = [4500, 6000, 7500, ...]
             *    - we want to trigger the clip stop event on pts >= 4500
             *
             *  2) (pts >> clip stop) - this is a pts wrap
             *   scenario, where the clip stop is very small, but
             *   the pts values are very large.  In this scenario,
             *   we want trigger the clip stop event only after the
             *   pts wraps. E.g.
             *     - clip stop = 3000
             *     - clip pts values = [-3000, -1500, 0, 1500, 3000, ...]
             *     - we want to trigger the clip stop event on
             *      pts >= 3000, and not on -3000.
             *
             *  3) (pts < clip stop) - this is the typical
             *   scenario, where the pts is less than the clip
             *   stop, so we don't want to do anything until the
             *   pts matures in case #1. E.g.
             *    - clip stop = 3000
             *    - clip pts values = [0, 1500, 3000, ...]
             *    - we wait for the stc to mature where we will
             *     trigger the clip stop event on pts >= 3000
             *
             *  4) (pts << clip stop) - this is a pts wrap
             *   scenario, where the clip stop is very large, but
             *   the pts values are very small.  In this scenario,
             *   we want to trigger the clip stop event right
             *   away. E.g.
             *      - clip stop = -3000 (0xFFFFF448)
             *      - clip pts values = [4500, 6000, 7500, ...]
             *    - we want to trigger the clip stop on pts >= 4500
             *
             * In the end, we want to trigger the stop event if
             * only cases 1) or 4) occur.
             */

            /* Get Actual PTS value for this picture for ClipStart/ClipStop
             * calculations */
            BXDM_PPQM_P_GetPtsWithFrac_isr( pstPicture,
                                        BXDM_PictureProvider_P_PTSIndex_eActual,
                                        0,
                                        &stPts );

            BXDM_PPFP_P_FixPtAdd_isr(
               &stPts,
               &pstPicture->stPicParms.stDisplay.stDynamic.stDisplayDuration,
               &stEffectivePTS
               );

            if ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
            {
               /* PR56062: We want to trigger the stop event now if the
                * stop time will be reached before the next vsync */
               BXDM_PPFP_P_FixPtAdd_isr(
                  &stEffectivePTS,
                  &pLocalState->stSTCDelta,
                  &stEffectivePTS
                  );

               /* PR56062: We don't want to trigger the stop event if
                * the next picture will trigger it exactly, so we subtract
                * 1 from the expected display duration. */
               if ( pLocalState->stSTCDelta.uiWhole != 0 )
               {
                  stEffectivePTS.uiWhole -= 1;
               }
            }

            /* SW7601-179: Round the effective PTS up, so that we don't miss
             * a stop event if we're off by a fractional */
            if ( 0 != stEffectivePTS.uiFractional )
            {
               stEffectivePTS.uiWhole++;
               stEffectivePTS.uiFractional = 0;
            }

            if ( ( ( true == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
                   || ( BXDM_PictureProvider_TSMResult_eWait != *peTsmState ) )
                 && ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility != pstCurrentClip->stTime.eType )
                      || ( false == pstPicture->stPicParms.stTSM.stDynamic.bClipTransition ) )
                 && ( ( ( stEffectivePTS.uiWhole >= pstCurrentClip->stTime.uiStop ) /* Case 1: PTS is greater than or equal to the stop time, but not too large */
                        && ( ( stEffectivePTS.uiWhole - pstCurrentClip->stTime.uiStop ) < BXDM_PictureProvider_P_CLIPTIME_PTS_WRAP_THRESHOLD ) )
                      || ( ( stEffectivePTS.uiWhole < pstCurrentClip->stTime.uiStop ) /* Case 4: PTS is less than the stop time, but much lesser */
                           && ( ( pstCurrentClip->stTime.uiStop - stEffectivePTS.uiWhole ) > BXDM_PictureProvider_P_CLIPTIME_PTS_WRAP_THRESHOLD ) ) ) )
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Stop Event (TSM/vTSM):: Stop Time:%08x PTS:%08x OrigPTS:%08x Duration:%08x",
                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                         pstCurrentClip->stTime.uiStop,
                                         stEffectivePTS.uiWhole,
                                         stPts.uiWhole,
                                         pstPicture->stPicParms.stDisplay.stDynamic.stDisplayDuration.uiWhole ));
               pstCurrentClip->stStopEvent.bOccurred = true;
               pstCurrentClip->stStopEvent.uiTime = pstCurrentClip->stTime.uiStop;
               /* SWBLURAY-21565: only allow the last picture of a clip to generate one stop event */
               pstPicture->stPicParms.stDisplay.stDynamic.bGeneratedClipStop = true;
            }
         }
         else
         {
            /* Handle ClipStop in VSYNC mode */
            if ( ( true == pstCurrentClip->stMarkerEvent.bOccurred ) /* The marker event has occurred */
                 && ( 0 != pLocalState->uiSlowMotionRate ) ) /* We're not in a pause state */
            {
               BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Stop Event (VSYNC) Stop Time %08x",
                                         hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                         BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                         pstPicture->stPicParms.uiPPBIndex & 0xFFF,
                                         pstCurrentClip->stTime.uiStop ));
               pstCurrentClip->stStopEvent.bOccurred = true;
               pstCurrentClip->stStopEvent.uiTime = pstCurrentClip->stTime.uiStop;
               /* SWBLURAY-21565: only allow the last picture of a clip to generate one stop event */
               pstPicture->stPicParms.stDisplay.stDynamic.bGeneratedClipStop = true;
            }
         }
      } /* End: Handle clip stop in Non-EOC or Non-DVD mode */

      /* PR50381: The DVD-VR VR100 title 11 has the following
       * properties:
       *  - many single picture clips in a row
       *  - each picture has the same PTS value
       *
       * These properties coupled with the application setting the
       * clip stop time to EOC Marker mode (0xFFFFFFFF) was resulting
       * in DM displaying the next picture of the clip without
       * generating a ClipStart event for the next clip.
       *
       * Since the PTS values of each picture are identical, the next
       * clip's picture's TSM result is always ePass, so we end up
       * displaying the picture from the next clip.  We were not
       * generating a ClipStart event because the clip time transition
       * handling occurs when we first look at the picture in the
       * queue (before TSM is performed).  It was never expected that
       * a ClipStop event would occur at the same time as a picture
       * passing.
       *
       * To prevent this issue, we always override a ePass TSM result
       * to eWait when the stop event first occurs to allow the clip
       * time transition handler to execute
       */
      if ( ( true == pstCurrentClip->stStopEvent.bOccurred )
           && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
           && ( BXDM_PictureProvider_TSMResult_ePass == *peTsmState ) )
      {
         *peTsmState = BXDM_PictureProvider_TSMResult_eWait;
      }
   } /* End: Handle Clip Stop */

   /* Override TSM state if PTSError occurs during playback of a
    * clip */
   if ( ( true == hXdmPP->stDMConfig.bPlayback )
        && ( true == pstCurrentClip->stTime.bValid )
        && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
        && ( true == pstCurrentClip->stStartEvent.bOccurred )
      )
   {
      /* We're in the middle of playing back a clip */
      if ( ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
           && ( ( BXDM_PictureProvider_TSMResult_eTooLate == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult )
                || ( BXDM_PictureProvider_TSMResult_eTooEarly == pstPicture->stPicParms.stTSM.stDynamic.eTsmResult ) )
           && ( BXDM_PictureProvider_DisplayMode_eTSM == pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode ) )
      {
         /* SW7335-781: Output warning when forcing picture selection override */
         BXDM_PPDBG_P_PrintSelectionModeOverride_isr("PTS Error Override", hXdmPP, pstPicture);

         /* To ensure a smooth seamless clip transition, we set the
          * selection mode for this picture to be VSYNC so that we end up
          * using vTSM mode to evaluate it  */
         pstPicture->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;

         /* We will want to generate a PTS error, but only when this
          * picture is actually displayed */
         hXdmPP->stDMState.stChannel.stClipInfo.bForcePTSError = true;
      }

      /* We handle the bForcePTSError flag */
      if ( true == hXdmPP->stDMState.stChannel.stClipInfo.bForcePTSError )
      {
         /* We need to send the next coded PTS */
         if ( ( BXDM_PictureProvider_TSMResult_ePass == *peTsmState )
              && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bPPBRepeated )
              && ( NULL != pstPicture->pstUnifiedPicture ) /* SW7445-3404: coverity defect C92940. */
              && ( true == pstPicture->pstUnifiedPicture->stPTS.bValid )
            )
         {
            /* We are in a TSM Pass and have a coded PTS, so this
             * picture will be displayed next, so we force the PTS
             * error */
            pLocalState->bForcePtsError = true;
            hXdmPP->stDMState.stChannel.stClipInfo.bForcePTSError = false;

            hXdmPP->stDMConfig.bSTCValid = false;
            /* To trigger the config debug message. */
            hXdmPP->stDMConfig.uiDirtyBits_1 |= BXDM_PictureProvider_P_DIRTY_1_STC_VALID;

            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Trigger Force PTS Error",
                                      hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                      BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                      pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
         }
      }
   }

   if ( ( BXDM_PictureProvider_TSMResult_ePass == *peTsmState )
        && ( true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.bValid )
        && ( true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stStartEvent.bOccurred )
        && ( false == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stDisplayEvent.bOccurred )
        && ( false == pstPicture->stPicParms.stTSM.stDynamic.bClipTransition )
        && ( false == pstPicture->stPicParms.stDisplay.stDynamic.bGeneratedClipStop )  /* SWBLURAY-21565: only allow the last picture of a clip to generate one stop event */
      )
   {
      /* We set the display event to true because we've achieved TSM
       * pass and we'll start displaying the next picture */
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Display Event",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicture->stPicParms.uiPPBIndex & 0xFFF ));
      hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stDisplayEvent.bOccurred = true;
   }
}

void BXDM_PPCLIP_P_ClipTimeQueueTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState,
   BXDM_PictureProvider_P_Picture_Context * pstPicCntxt
   )
{
   BXDM_PictureProvider_P_ClipTimeV2 * pstCurrentClip = &( hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip );

   BSTD_UNUSED(pLocalState);

   if ( (true == pstPicCntxt->pstUnifiedPicture->stPTS.bValid)
        && ( true == hXdmPP->stDMState.stChannel.stClipInfo.bClipTransition ) )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Exit Clip Transition Mode",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF ));

      /* We turn off the clip transition mode when we first get a coded PTS */
      hXdmPP->stDMState.stChannel.stClipInfo.bClipTransition = false;
   }

   /* SWBLURAY-23188: The failure was a 24p stream going to a 24Hz display, the 60Hz output was fine.
    * It just so happened that the last picture of the current clip was being dropped.  Then due to the
    * pipelining of the logic, the first picture of the next clip was dropped as well.  For both pictures,
    * the TSM result was overridden to "drop" from "pass".  The TSM result was feeding back into the vTSM
    * logic and resetting the vPTS.  The vPTS was set to the coded PTS of the second picture in the new clip.
    * However the vSTC was still based on the old time base.  This resulted in all the pictures in the new
    * clip being late, hence they were dropped.
    *
    * The solution is to load the parameters for the next clip sooner.  This loading is gated by the variable
    * "bClipActive".  Now the new clip will be loaded ("bClipActive" set to true) when:
    * - a DVD clip
    * - AND the stop event has occurred
    * - AND not a picture less clip, i.e. not a dummy PPB
    * - AND a new maker has been seen
    *
    * For now this is only being done for the DVD clips.  For the other modes "bClipActive" is cleared
    * in " BXDM_PPCLIP_P_ClipTimeCallbackTriggerHandler_isr".
    */
   if ( ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == pstCurrentClip->stTime.eType )
         && ( true == pstCurrentClip->stStopEvent.bOccurred )
         && ( true == pstPicCntxt->pstUnifiedPicture->stBufferInfo.bValid )
         && ( hXdmPP->stDMState.stChannel.stClipInfo.uiLastMarker != pstPicCntxt->pstUnifiedPicture->stPictureMarker.uiValue )
      )
   {
      hXdmPP->stDMState.stChannel.stClipInfo.bClipActive = false;
   }

   /* Handle a clip transition */
   if ( true == hXdmPP->stDMConfig.stClipTimeSettings.bValid )
   {
      if ( BXDM_PictureProvider_ClipTimeType_eDisable == hXdmPP->stDMConfig.stClipTimeSettings.eType )
      {
         /* We want to disable any clipping that may be active, so we
          * invalidate the local current clip info */
         BKNI_Memset( &hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip,
                      0,
                      sizeof( BXDM_PictureProvider_P_ClipTimeV2 ) );

         /* We disable all clip related callbacks */
         hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = false;
         hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = false;
         hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable = false;

         /* We invalidate the public clip info */
         BKNI_Memset( &hXdmPP->stDMConfig.stClipTimeSettings,
                      0,
                      sizeof( BXDM_PictureProvider_ClipTimeSettings ) );
      }
      else if ( ( false == hXdmPP->stDMState.stChannel.stClipInfo.bClipActive )
                || ( BXDM_PictureProvider_ClipTimeType_eClipStopOnly == hXdmPP->stDMConfig.stClipTimeSettings.eType ) )
      {
         /* If the next clip and current clip are both valid, then we are
          * in a seamless clip transition, so we need to override an
          * invalid PTS of this picture to be equal to the next clip's
          * start time to prevent it from being dropped */
         if ( ( true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.bValid )
              && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.eType )
              && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == hXdmPP->stDMConfig.stClipTimeSettings.eType )
              && ( false == pstPicCntxt->pstUnifiedPicture->stPTS.bValid )
            )
         {
            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Enter Clip Transition",
                                      hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                      BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                      pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF ));

            BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Clip Transition PTS Override",
                                      hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                      BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                      pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF ));
            /* Override PTS of this picture if it is not coded, so
             * that the clip start will return the proper PTS.  We
             * assume the PTS of the first picture of the next clip
             * is the same as the start time for seamless cases */
            hXdmPP->stDMState.stChannel.stClipInfo.bClipTransition = true;
         }

         /* We copy the next clip info to the current clip info */

         /* We invalidate the local current clip info */
         BKNI_Memset( &hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip,
                      0,
                      sizeof( BXDM_PictureProvider_P_ClipTimeV2 ) );

         hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime = hXdmPP->stDMConfig.stClipTimeSettings;

         /* We invalidate the public clip info */
         BKNI_Memset( &hXdmPP->stDMConfig.stClipTimeSettings,
                      0,
                      sizeof( BXDM_PictureProvider_ClipTimeSettings ) );

         BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] New Clip (%d: %08x -> %08x)",
                                   hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                   BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                   pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF,
                                   hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.eType,
                                   hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.uiStart,
                                   hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.uiStop ));

         /* We re-enable the clip callbacks */
         switch ( hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.eType )
         {
            case BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility:
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = true;
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = true;
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable = true;
               break;

            case BXDM_PictureProvider_ClipTimeType_eEnableBoth:
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = true;
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = true;
               break;

            case BXDM_PictureProvider_ClipTimeType_eClipStartOnly:
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStart].bEnable = true;
               break;

            case BXDM_PictureProvider_ClipTimeType_eClipStopOnly:
               hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_eClipStop].bEnable = true;

               /* We assume the start event has already occurred */
               hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stStartEvent.bOccurred = true;
               hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stDisplayEvent.bOccurred = true;
               break;

            default:
               break;
         }

         /* We set bClipActive to true */
         hXdmPP->stDMState.stChannel.stClipInfo.bClipActive = true;
      }
   }

   /* Handle Auto-Vsync Mode Cases */
   if (true == hXdmPP->stDMState.stChannel.stClipInfo.bClipTransition)
   {
      pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
      pstPicCntxt->stPicParms.stTSM.stDynamic.bClipTransition = true;
   }

   /* PR51920: If the STC is valid, then we can turn off the trick
    * mode transition bit. */
   if ( ( true == hXdmPP->stDMState.stChannel.stClipInfo.bTrickModeTransition )
        && ( ( true == hXdmPP->stDMConfig.bSTCValid ) /* The STC is valid */
             || ( BXDM_PictureProvider_DisplayMode_eVirtualTSM == pLocalState->eDisplayMode ) /* We're in VSYNC display mode */
             || ( BXDM_PICTUREPROVIDER_NORMAL_PLAYBACK_RATE != pLocalState->uiSlowMotionRate ) /* We've entered into another trick mode */
           )
      )
   {
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Exiting trick mode transition (stc: %x)",
                                 hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                 BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                 pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF,
                                 pLocalState->uiAdjustedStc ));
      hXdmPP->stDMState.stChannel.stClipInfo.bTrickModeTransition = false;
   }

   /* If the STC becomes invalid in the middle of playing back a clip,
    * then we go into VSYNC mode */
   if ( true == hXdmPP->stDMConfig.bPlayback
        && false == hXdmPP->stDMConfig.bSTCValid
        && true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.bValid
        && ( BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stTime.eType )
        && false == hXdmPP->stDMState.stChannel.stClipInfo.bTrickModeTransition
        && true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stStartEvent.bOccurred
        && true == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stDisplayEvent.bOccurred
        && false == hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stStopEvent.bOccurred
      )
   {
      /* SW7335-781: Output warning when forcing picture selection override */
      BXDM_PPDBG_P_PrintSelectionModeOverride_isr("Clip STC Invalid", hXdmPP, pstPicCntxt);
      pstPicCntxt->stPicParms.stTSM.stDynamic.eSelectionMode = BXDM_PictureProvider_DisplayMode_eVirtualTSM;
   }

   /* Handle Picture Marker */
   /* PR52247: We need to handle the marker event AFTER the clip
    * transition code to handle picture-less clips (i.e. only contains
    * a single dummy PPB).  Picture-less clips will trigger a
    * transition, start, stop, and marker event all on the same
    * vsync */
   if ( hXdmPP->stDMState.stChannel.stClipInfo.uiLastMarker != pstPicCntxt->pstUnifiedPicture->stPictureMarker.uiValue )
   {
      /* We set the marker event */
      BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.%03x] Marker Event Old:%08x  New:%08x",
                                hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                                BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                                pstPicCntxt->stPicParms.uiPPBIndex & 0xFFF,
                                hXdmPP->stDMState.stChannel.stClipInfo.uiLastMarker,
                                pstPicCntxt->pstUnifiedPicture->stPictureMarker.uiValue ));
      hXdmPP->stCallbacks[BXDM_PictureProvider_Callback_ePictureMarker].bEnable = true;
      hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stMarkerEvent.bOccurred = true;
      hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stMarkerEvent.uiTime = pstPicCntxt->pstUnifiedPicture->stPictureMarker.uiValue;
      hXdmPP->stDMState.stChannel.stClipInfo.stCurrentClip.stMarkerEvent.bDelayedCallback = ( true == pstPicCntxt->pstUnifiedPicture->stBufferInfo.bValid );

      /* We save the new marker */
      hXdmPP->stDMState.stChannel.stClipInfo.uiLastMarker = pstPicCntxt->pstUnifiedPicture->stPictureMarker.uiValue;

      /* TODO: handle multiple markers */
   }
}

void BXDM_PPCLIP_P_ClipTimeTrickModeTransitionHandler_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState * pLocalState
   )
{
   BSTD_UNUSED(pLocalState);

   BXDM_MODULE_MSG_isr(( hXdmPP, BXDM_Debug_MsgType_eCLIP, "%x:[%02x.xxx] Entering trick mode transition (stc: %x)",
                              hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                              BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                              pLocalState->uiAdjustedStc ));

   /* PR51920: We set the trick mode transition bit so that we don't
    * go into auto-vsync mode which gets triggered if the STC is
    * invalidated during the middle of a clip. */
   hXdmPP->stDMState.stChannel.stClipInfo.bTrickModeTransition = true;
}
