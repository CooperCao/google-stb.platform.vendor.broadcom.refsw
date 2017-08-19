/******************************************************************************
 * Copyright (C) 2004-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/
#include "bstd.h"
#include "bxvd.h"
#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_pvr.h"

BDBG_MODULE(BXVD_PVR);

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_EnablePause
(
   BXVD_ChannelHandle hXvdCh,
   bool bEnablePause
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_PVR_EnablePause);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_EnablePause_isr(
      hXvdCh,
      bEnablePause
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_EnablePause);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_EnablePause_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool bEnablePause
   )
{
   BERR_Code rc;
   BXDM_Picture_Rate stCurrentPlaybackRate;
   BDBG_ENTER(BXVD_PVR_EnablePause_isr);

   BDBG_ASSERT(hXvdCh);

   rc = BXDM_PictureProvider_GetPlaybackRate_isr(
            hXvdCh->hPictureProvider,
            &stCurrentPlaybackRate);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_EnablePause(%d)", bEnablePause));

   if ( BERR_SUCCESS == rc )
   {
      if( bEnablePause ) {
            /* SW7400-2870: pause if not already paused by a previous call to BXVD_PVR_EnablePause.*/
            if ( false == hXvdCh->bPauseActive  )
            {
               hXvdCh->bPauseActive = true;

               hXvdCh->stSavedPlaybackRate = stCurrentPlaybackRate;
               hXvdCh->bSavedPlaybackRateValid = true;

               stCurrentPlaybackRate.uiNumerator = 0;
               stCurrentPlaybackRate.uiDenominator = 1;

               rc = BXDM_PictureProvider_SetPlaybackRate_isr(
                        hXvdCh->hPictureProvider,
                        &stCurrentPlaybackRate
                        );
            }
      } else {
            /* SW7400-2870: un-pause if previously paused by calling BXVD_PVR_EnablePause. */
            if ( true == hXvdCh->bPauseActive  )
            {
               hXvdCh->bPauseActive = false;

               if ( true == hXvdCh->bSavedPlaybackRateValid )
               {
                  rc = BXDM_PictureProvider_SetPlaybackRate_isr(
                           hXvdCh->hPictureProvider,
                           &hXvdCh->stSavedPlaybackRate
                           );
               }
               else
               {
                  stCurrentPlaybackRate.uiNumerator = 1;
                  stCurrentPlaybackRate.uiDenominator = 1;

                  rc = BXDM_PictureProvider_SetPlaybackRate_isr(
                           hXvdCh->hPictureProvider,
                           &stCurrentPlaybackRate
                           );

                  hXvdCh->stSavedPlaybackRate = stCurrentPlaybackRate;
                  hXvdCh->bSavedPlaybackRateValid = true;
               }
            }
      }
   }

   BDBG_LEAVE(BXVD_PVR_EnablePause_isr);
   return BERR_TRACE(rc);
}

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetPauseStatus
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbPauseStatus
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_GetPauseStatus);

   rc = BXVD_PVR_GetPauseStatus_isr(
      hXvdCh,
      pbPauseStatus
      );

   BDBG_LEAVE(BXVD_PVR_GetPauseStatus);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_GetPauseStatus_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbPauseStatus
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BDBG_ENTER(BXVD_PVR_GetPauseStatus_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbPauseStatus);

   /* SW7400-2870: within XDM, stCurrentPlaybackRate.uiNumerator can be set to '0' by either
    * BXVD_PVR_EnablePause or BXVD_SetPlaybackRate. Use the new flag 'bPauseActive'
    * to determine if the system was paused by calling BXVD_PVR_EnablePause.
    */
   *pbPauseStatus = hXvdCh->bPauseActive;

   BDBG_LEAVE(BXVD_PVR_GetPauseStatus_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
DEPRECATED: SW7400-2870: use BXVD_SetPlaybackRate(isr) instead
***************************************************************************/
BERR_Code BXVD_PVR_SetSlowMotionRate
(
   BXVD_ChannelHandle hXvdCh,
   unsigned long ulRate
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_SetSlowMotionRate);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_SetSlowMotionRate_isr(
      hXvdCh,
      ulRate
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_SetSlowMotionRate);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_SetSlowMotionRate_isr
(
   BXVD_ChannelHandle hXvdCh,
   unsigned long ulRate
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stPlaybackRate;
   BDBG_ENTER(BXVD_PVR_SetSlowMotionRate_isr);

   BDBG_ASSERT(hXvdCh);

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetSlowMotionRate(%ld)", ulRate));

   /* The addition of "uiPvrPlaybackRate" allows the playback rate to be treated roughly
    * as a percentage of normal playback.
    */
   BKNI_Memset( &stPlaybackRate, 0, sizeof( BXDM_Picture_Rate ) );

   stPlaybackRate.uiNumerator = 1;
   if ( ulRate > 1 )
   {
      stPlaybackRate.uiDenominator = ulRate;
   }
   else
   {
      stPlaybackRate.uiDenominator = 1;
   }

   /* SW7601-186: XVD + DMv2 maintained separate pause and slow motion states
    * and put precedence on the paused state.  However, XVD + XDM implements pause
    * and slow motion using the same single state (the playback rate) in XDM.
    *
    * I.e. pause --> playback_rate = 0
    *      slow motion --> playback_rate = slow motion rate
    *
    * The following call sequence exposed an XDM compatibility issue:
    *    1) BXVD_PVR_EnablePause_isr(true) <-- playback_rate = 0
    *    2) BXVD_PVR_SetSlowMotionRate_isr(1) <-- playback_rate = 100% (but should stay at 0)
    *
    * To maintain backwards compatibility with XVD + DMv2, the XDM playback rate
    * is not changed if the system is in a paused state.  The slow motion rate is
    * instead remembered and set when unpaused.
    */
   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_SetPlaybackRate_isr(
               hXvdCh->hPictureProvider,
               &stPlaybackRate
               );
   }

   hXvdCh->stSavedPlaybackRate = stPlaybackRate;
   hXvdCh->bSavedPlaybackRateValid = true;

   BDBG_LEAVE(BXVD_PVR_SetSlowMotionRate);
   return BERR_TRACE( rc );
}

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetSlowMotionRate
(
   BXVD_ChannelHandle hXvdCh,
   unsigned long *pSMRate
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_GetSlowMotionRate);

   rc = BXVD_PVR_GetSlowMotionRate_isr(
      hXvdCh,
      pSMRate
      );

   BDBG_LEAVE(BXVD_PVR_GetSlowMotionRate);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_GetSlowMotionRate_isr
(
   BXVD_ChannelHandle hXvdCh,
   unsigned long *pSMRate
   )
{
   BERR_Code rc=BERR_SUCCESS;
   BXDM_Picture_Rate stPlaybackRate;
   BDBG_ENTER(BXVD_PVR_GetSlowMotionRate_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pSMRate);

   /* SW7400-2870: if not paused by a call to BXVD_PVR_EnablePause, retrieve
    * the playback rate from XDM.  If paused, return the valued saved in the
    * channel structure.
    */
   if ( false == hXvdCh->bPauseActive )
   {
      rc = BXDM_PictureProvider_GetPlaybackRate_isr(
                  hXvdCh->hPictureProvider,
                  &stPlaybackRate
                  );

      if (  BERR_SUCCESS != rc )
      {
         *pSMRate = 0;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_PVR_GetSlowMotionRate_isr() BXDM_PictureProvider_GetPlaybackRate_isr returned %d", rc ));
         return BERR_TRACE( rc );
      }
   }
   else
   {
      if ( true == hXvdCh->bSavedPlaybackRateValid )
      {
         stPlaybackRate = hXvdCh->stSavedPlaybackRate;
      }
      else
      {
         /* It should be impossible to hit this case.  If "bPauseActive" is true,
          * there should always be a saved playback rate.  Add this warning in the
          * event a logic bug creeps in.
          */
         stPlaybackRate.uiNumerator = 1;
         stPlaybackRate.uiDenominator = 1;
         BXVD_DBG_WRN(hXvdCh, ("BXVD_PVR_GetSlowMotionRate_isr() bPauseActive but no saved playback rate."));
      }
   }

   if ( 0 == stPlaybackRate.uiNumerator )
   {
      *pSMRate = 0;
   }
   else
   {
      *pSMRate = stPlaybackRate.uiDenominator / stPlaybackRate.uiNumerator;
   }

   BDBG_LEAVE(BXVD_PVR_GetSlowMotionRate_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_FrameAdvance
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FrameAdvanceMode eFrameAdvMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_FrameAdvance);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_FrameAdvance_isr(
      hXvdCh,
      eFrameAdvMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_FrameAdvance);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_FrameAdvance_isr
(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FrameAdvanceMode eFrameAdvMode
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_FrameAdvance_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(eFrameAdvMode <= BXVD_PVR_FrameAdvanceMode_eField_by_Field);

   switch ( eFrameAdvMode )
   {
      case BXVD_PVR_FrameAdvanceMode_eFrame_by_Frame:
         rc = BXDM_PictureProvider_SetFrameAdvanceMode_isr(
               hXvdCh->hPictureProvider,
               BXDM_PictureProvider_FrameAdvanceMode_eFrameByField
               );
         break;

      case BXVD_PVR_FrameAdvanceMode_eField_by_Field:
         rc = BXDM_PictureProvider_SetFrameAdvanceMode_isr(
               hXvdCh->hPictureProvider,
               BXDM_PictureProvider_FrameAdvanceMode_eField
               );
         break;

      default:
         BXVD_DBG_ERR(hXvdCh, ("Unknown frame advance mode (%d)", eFrameAdvMode));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_FrameAdvance(%d)", eFrameAdvMode));

   BDBG_LEAVE(BXVD_PVR_FrameAdvance_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_EnableReverseFields(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_EnableReverseFields);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_EnableReverseFields_isr(
      hXvdCh,
      bEnable
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_EnableReverseFields);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_EnableReverseFields_isr(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_EnableReverseFields_isr);

   BDBG_ASSERT(hXvdCh);

   rc = BXDM_PictureProvider_SetReverseFieldsMode_isr(
         hXvdCh->hPictureProvider,
         bEnable
         );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_EnableReverseFields(%d)", bEnable));

   BDBG_LEAVE(BXVD_PVR_EnableReverseFields_isr);
   return BERR_TRACE( rc );
}

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetReverseFieldStatus
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_GetReverseFieldStatus);

   rc = BXVD_PVR_GetReverseFieldStatus_isr(
      hXvdCh,
      pbEnable
      );

   BDBG_LEAVE(BXVD_PVR_GetReverseFieldStatus);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_GetReverseFieldStatus_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_GetReverseFieldStatus_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbEnable);

   rc = BXDM_PictureProvider_GetReverseFieldsMode_isr(
         hXvdCh->hPictureProvider,
         pbEnable
         );

   BDBG_LEAVE(BXVD_PVR_GetReverseFieldStatus_isr);
   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_SetHostSparseMode(
   BXVD_ChannelHandle hXvdCh,
   bool bSparseMode
   )
{
   uint32_t            i;

   BAVC_XptContextMap  XptContextMap;
   BAVC_XptContextMap  aXptContextMap_Extended[BXVD_NUM_EXT_RAVE_CONTEXT];

   bool bCurSparseMode = hXvdCh->stDecoderContext.bHostSparseMode;

   BXDM_PictureProvider_PreserveStateSettings stPreserveStateSettings;
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BXVD_PVR_SetHostSparseMode);
   BDBG_ASSERT(hXvdCh);

   hXvdCh->stDecoderContext.bHostSparseMode = bSparseMode;

   if ((bCurSparseMode != bSparseMode) && (hXvdCh->eDecoderState == BXVD_P_DecoderState_eActive))
   {
      /* SW7445-881: support bHostSparseMode for H265/HEVC  */
      if (((hXvdCh->sDecodeSettings.eVideoCmprStd != BAVC_VideoCompressionStd_eH264)
           && (hXvdCh->sDecodeSettings.eVideoCmprStd != BAVC_VideoCompressionStd_eH265))
          && (bSparseMode == true))
      {
         BXVD_DBG_WRN(hXvdCh, ("Sparse mode enabled on Non H264/H265 stream, Sparse mode now disabled"));
         hXvdCh->stDecoderContext.bHostSparseMode = false;
      }
      else
      {
         BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetHostSparseMode: Stopping and starting decoding"));

         BERR_TRACE(BXDM_PictureProvider_GetDefaultPreserveStateSettings_isrsafe(
            hXvdCh->hPictureProvider,
            &stPreserveStateSettings
            ));

         stPreserveStateSettings.bDisplay = true;
         stPreserveStateSettings.bCounters = true;
         hXvdCh->bPreserveState = true;

         BERR_TRACE(BXDM_PictureProvider_SetPreserveStateSettings_isr(
            hXvdCh->hPictureProvider,
            &stPreserveStateSettings
            ));

         BERR_TRACE(BXVD_StopDecode(hXvdCh));

         /* Reset XPT Rave CDB read register address */
         XptContextMap.CDB_Read = hXvdCh->ulXptCDB_Read;
         hXvdCh->sDecodeSettings.pContextMap = &XptContextMap;

         for (i = 0; i < hXvdCh->sDecodeSettings.uiContextMapExtNum; i++)
         {
            hXvdCh->sDecodeSettings.aContextMapExtended[i] = &aXptContextMap_Extended[i];
            aXptContextMap_Extended[i].CDB_Read = hXvdCh->aulXptCDB_Read_Extended[i];
         }

         rc = BERR_TRACE(BXVD_StartDecode(hXvdCh, &hXvdCh->sDecodeSettings));

         hXvdCh->bPreserveState = false;
      }
   }
   else
   {
      BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetHostSparseMode: NOT stopping/starting decoding.. Letting flush do it!!!"));
   }

   BDBG_LEAVE(BXVD_PVR_SetHostSparseMode);
   return BERR_TRACE(rc);
}

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetHostSparseMode
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbSparseMode
   )
{
   BDBG_ENTER(BXVD_PVR_GetHostSparseMode);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbSparseMode);

   *pbSparseMode = hXvdCh->stDecoderContext.bHostSparseMode;

   BDBG_LEAVE(BXVD_PVR_GetHostSparseMode);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_SetGopTrickMode
(
   BXVD_ChannelHandle hXvdCh,
   bool bEnableTrickMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_SetGopTrickMode);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_SetGopTrickMode_isr(
      hXvdCh,
      bEnableTrickMode
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_SetGopTrickMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_SetGopTrickMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool bEnableTrickMode
   )
{
   BDBG_ENTER(BXVD_PVR_SetGopTrickMode_isr);

   BDBG_ASSERT(hXvdCh);

   hXvdCh->stDecoderContext.bReversePlayback = bEnableTrickMode;

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetGopTrickMode(%d)", bEnableTrickMode));

   BDBG_LEAVE(BXVD_PVR_SetGopTrickMode_isr);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetGopTrickMode
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnableTrickMode
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_GetGopTrickMode);

   rc = BXVD_PVR_GetGopTrickMode_isr(
      hXvdCh,
      pbEnableTrickMode
      );

   BDBG_LEAVE(BXVD_PVR_GetGopTrickMode);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_GetGopTrickMode_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnableTrickMode
   )
{
   BDBG_ENTER(BXVD_PVR_GetGopTrickMode_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbEnableTrickMode);

   *pbEnableTrickMode = hXvdCh->stDecoderContext.bReversePlayback;

   BDBG_LEAVE(BXVD_PVR_GetGopTrickMode_isr);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_SetAutoValidateStcOnPause
(
   BXVD_ChannelHandle hXvdCh,
   bool bAutoValidateStcOnPause
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_SetAutoValidateStcOnPause);

   BKNI_EnterCriticalSection();
   rc = BXVD_PVR_SetAutoValidateStcOnPause_isr(
      hXvdCh,
      bAutoValidateStcOnPause
      );
   BKNI_LeaveCriticalSection();

   BDBG_LEAVE(BXVD_PVR_SetAutoValidateStcOnPause);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_SetAutoValidateStcOnPause_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool bAutoValidateStcOnPause
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_SetAutoValidateStcOnPause_isr);

   BDBG_ASSERT(hXvdCh);

   rc = BXDM_PictureProvider_SetAutoValidateStcOnPauseMode_isr(
         hXvdCh->hPictureProvider,
         bAutoValidateStcOnPause
         );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetAutoValidateStcOnPause(%d)", bAutoValidateStcOnPause));

   BDBG_LEAVE(BXVD_PVR_SetAutoValidateStcOnPause_isr);
   return BERR_TRACE(rc);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetAutoValidateStcOnPause
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbAutoValidateStcOnPause
   )
{
   BERR_Code rc;

   BDBG_ENTER(BXVD_PVR_GetAutoValidateStcOnPause);

   rc = BXVD_PVR_GetAutoValidateStcOnPause_isr(
      hXvdCh,
      pbAutoValidateStcOnPause
      );

   BDBG_LEAVE(BXVD_PVR_GetAutoValidateStcOnPause);
   return BERR_TRACE(rc);
}

BERR_Code BXVD_PVR_GetAutoValidateStcOnPause_isr
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbAutoValidateStcOnPause
   )
{
   BERR_Code rc;
   BDBG_ENTER(BXVD_PVR_GetAutoValidateStcOnPause_isr);

   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbAutoValidateStcOnPause);

   rc = BXDM_PictureProvider_GetAutoValidateStcOnPauseMode_isr(
         hXvdCh->hPictureProvider,
         pbAutoValidateStcOnPause
         );

   BDBG_LEAVE(BXVD_PVR_GetAutoValidateStcOnPause_isr);
   return BERR_TRACE( rc );
}
#endif

/***************************************************************************
   SW7425-2270:
   The application will call SetIgnoreNRTUnderflow when it determines that an
   NRT underflow is actually a gap in the content.
   see expanded comment in bxvd_pvr.h
****************************************************************************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_SetIgnoreNRTUnderflow(
   BXVD_ChannelHandle hXvdCh,
   bool bIgnoreNRTUnderflow
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_PVR_SetIgnoreNRTUnderflow );

   BKNI_EnterCriticalSection();

   rc = BXVD_PVR_SetIgnoreNRTUnderflow_isr( hXvdCh, bIgnoreNRTUnderflow );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_PVR_SetIgnoreNRTUnderflow );

   return BERR_TRACE( rc );
}
#endif

BERR_Code BXVD_PVR_SetIgnoreNRTUnderflow_isr(
   BXVD_ChannelHandle hXvdCh,
   bool bIgnoreNRTUnderflow
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_PVR_SetIgnoreNRTUnderflow_isr );

   BDBG_ASSERT( hXvdCh );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetIgnoreNRTUnderflow_isr(
            hXvdCh->hPictureProvider,
            bIgnoreNRTUnderflow
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetIgnoreNRTUnderflow::  bIgnoreNRTUnderflow:%d",
                  bIgnoreNRTUnderflow
                  ));


   BDBG_LEAVE( BXVD_PVR_SetIgnoreNRTUnderflow_isr );

   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetIgnoreNRTUnderflow(
   BXVD_ChannelHandle hXvdCh,
   bool * pbIgnoreNRTUnderflow
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_PVR_GetIgnoreNRTUnderflow );

   BKNI_EnterCriticalSection();

   rc = BXVD_PVR_GetIgnoreNRTUnderflow_isr( hXvdCh, pbIgnoreNRTUnderflow );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_PVR_GetIgnoreNRTUnderflow );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_PVR_GetIgnoreNRTUnderflow_isr(
   BXVD_ChannelHandle hXvdCh,
   bool * pbIgnoreNRTUnderflow
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_PVR_GetIgnoreNRTUnderflow_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pbIgnoreNRTUnderflow );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetIgnoreNRTUnderflow_isr(
            hXvdCh->hPictureProvider,
            pbIgnoreNRTUnderflow
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_GetIgnoreNRTUnderflow::  bIgnoreNRTUnderflow:%d",
                  *pbIgnoreNRTUnderflow
                  ));

   BDBG_LEAVE( BXVD_PVR_GetIgnoreNRTUnderflow_isr );

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
   SW7425-3358: support for FNRT.
****************************************************************************/

BERR_Code BXVD_PVR_SetFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_PVR_SetFNRTSettings );

   BKNI_EnterCriticalSection();

   rc = BXVD_PVR_SetFNRTSettings_isr( hXvdCh, pstFNRTSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_PVR_SetFNRTSettings );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_PVR_SetFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_PVR_SetFNRTSettings_isr );

   BDBG_ASSERT( hXvdCh );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_SetFNRTSettings_isr(
            hXvdCh->hPictureProvider,
            pstFNRTSettings
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_SetFNRTSettings::  pstFNRTSettings:0x%0*lx",
                         BXVD_P_DIGITS_IN_LONG, (long)pstFNRTSettings));


   BDBG_LEAVE( BXVD_PVR_SetFNRTSettings_isr );

   return BERR_TRACE( rc );
}

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_PVR_GetFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_PVR_GetFNRTSettings );

   BKNI_EnterCriticalSection();

   rc = BXVD_PVR_GetFNRTSettings_isr( hXvdCh, pstFNRTSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_PVR_GetFNRTSettings );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_PVR_GetFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_PVR_GetFNRTSettings_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pstFNRTSettings );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetFNRTSettings_isr(
            hXvdCh->hPictureProvider,
            pstFNRTSettings
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_GetFNRTSettings::  pstFNRTSettings:0x%0*lx",
                  BXVD_P_DIGITS_IN_LONG, (long)pstFNRTSettings));

   BDBG_LEAVE( BXVD_PVR_GetFNRTSettings_isr );

   return BERR_TRACE( rc );
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

BERR_Code BXVD_PVR_GetDefaultFNRTSettings(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc;

   BDBG_ENTER( BXVD_PVR_GetDefaultFNRTSettings );

   BKNI_EnterCriticalSection();

   rc = BXVD_PVR_GetDefaultFNRTSettings_isr( hXvdCh, pstFNRTSettings );

   BKNI_LeaveCriticalSection();

   BDBG_LEAVE( BXVD_PVR_GetDefaultFNRTSettings );

   return BERR_TRACE( rc );
}

BERR_Code BXVD_PVR_GetDefaultFNRTSettings_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PVR_FNRTSettings * pstFNRTSettings
   )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BXVD_PVR_GetDefaultFNRTSettings_isr );

   BDBG_ASSERT( hXvdCh );
   BDBG_ASSERT( pstFNRTSettings );

   /* Check handle type for correctness */
   if (hXvdCh->eHandleType != BXVD_P_HandleType_XvdChannel)
   {
      BDBG_ERR(("Invalid handle type passed to function"));
      return BERR_TRACE(BXVD_ERR_INVALID_HANDLE);
   }

   rc = BXDM_PictureProvider_GetDefaultFNRTSettings_isr(
            hXvdCh->hPictureProvider,
            pstFNRTSettings
            );

   BXVD_DBG_MSG(hXvdCh, ("BXVD_PVR_GetDefaultFNRTSettings::  pstFNRTSettings:0x%0*lx",
                         BXVD_P_DIGITS_IN_LONG, (long)pstFNRTSettings));

   BDBG_LEAVE( BXVD_PVR_GetDefaultFNRTSettings_isr );

   return BERR_TRACE( rc );
}


/*******************/
/* Deprecated APIs */
/*******************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_SetBUDMode
(
   BXVD_ChannelHandle hXvdCh,
   bool bBudMode
   )
{
   BDBG_ENTER(BXVD_PVR_SetBUDMode);
   BDBG_ASSERT(hXvdCh);

   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(bBudMode);

   BXVD_DBG_WRN(hXvdCh, ("BXVD_PVR_SetBUDMode() is DEPRECATED. Video Decoder auto-detects BUD mode"));

   BDBG_LEAVE(BXVD_PVR_SetBudMode);
   return BERR_TRACE(BERR_SUCCESS);
}

/***************************************************************************
***************************************************************************/
BERR_Code BXVD_PVR_GetBUDMode
(
   BXVD_ChannelHandle hXvdCh,
   bool *pbBudMode
   )
{
   BDBG_ENTER(BXVD_PVR_GetBUDMode);
   BDBG_ASSERT(hXvdCh);
   BDBG_ASSERT(pbBudMode);

   BSTD_UNUSED(hXvdCh);
   BSTD_UNUSED(pbBudMode);

   BDBG_LEAVE(BXVD_PVR_GetBUDMode);
   return BERR_TRACE(BERR_SUCCESS);
}
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/* End of File */
