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

#include "bxvd_platform.h"
#include "bxvd_priv.h"
#include "bxvd_decoder.h"
#include "bxvd_decoder_timer.h"

#include "bxvd_vdec_info.h"


BDBG_MODULE(BXVD_DECODERTMR);

#if BDBG_DEBUG_BUILD

/*
 * Result will be printed every BXVD_DECODER_TIMER_S_VSYNCS_PER_PRINT vsync's
 * For a value of '0', results are printed once per second.
 */
#define BXVD_DECODER_TIMER_S_VSYNCS_PER_PRINT  0


/*
 * Only compile the local functions if timing data is being collected.
 */

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING


static const char * const sFunctionNameLUT[BXVD_DecoderTimer_P_eFuncMax] =
{
   "DRIsr",
   "UQVet",
   "UQDQT",
   "UQVal",
   "UQAdd",

   "\n\tGetCnt",
   "Peek",
   "GetPic",
   "RelPic"

};

#endif

#if ( BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING || BXVD_DECODER_TIMER_S_ENABLE_CALLBACK_TIMING )

/*
 * Local functions
 */
void BXVD_DecoderTimer_S_GetTime_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * pTimeUsec
   )
{
   if( NULL != hXvdCh->pXvd->hTimer )
   {
      BTMR_ReadTimer_isr( hXvdCh->pXvd->hTimer, pTimeUsec );
   }
   else
   {
      *pTimeUsec = 0;
   }

   return;
}

void BXVD_DecoderTimer_S_SnapshotEndTimeAndUpdateElapse_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_Sample * pTimerSample
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_S_SnapshotEndTimeAndUpdateElapse_isr);

   BXVD_DecoderTimer_S_GetTime_isr( hXvdCh, &(pTimerSample->uiEndTimeUsecs) );

   /* Calculate the elapse time. */
   pTimerSample->uiElapseTimeUsecs = ( pTimerSample->uiEndTimeUsecs - pTimerSample->uiStartTimeUsecs );

   /* Check if the time is a new max elapse time. */
   if ( pTimerSample->uiElapseTimeUsecs > pTimerSample->uiMaxTimeUsecs )
   {
      pTimerSample->uiMaxTimeUsecs = pTimerSample->uiElapseTimeUsecs;
   }

   /* Check if the time is a new min elapse time. */
   if ( 0 == pTimerSample->uiMinTimeUsecs )
   {
      pTimerSample->uiMinTimeUsecs = pTimerSample->uiElapseTimeUsecs;
   }
   else if ( pTimerSample->uiElapseTimeUsecs < pTimerSample->uiMinTimeUsecs )
   {
      pTimerSample->uiMinTimeUsecs = pTimerSample->uiElapseTimeUsecs;
   }

   /* Add the elapse time for this callback to the total. */
   pTimerSample->uiTotalTimeUsecs += pTimerSample->uiElapseTimeUsecs;

   pTimerSample->uiNumSamples++;

   BDBG_LEAVE(BXVD_DecoderTimer_S_SnapshotEndTimeAndUpdateElapse_isr);

   return;
}

void BXVD_DecoderTimer_S_CookResults_isrsafe(
   BXVD_ChannelHandle hXvdCh
   )
{
   BXVD_DecoderTimer_P_Data * pTimerData = &(hXvdCh->stDecoderContext.stLogData.stTimerData);

   uint32_t i;

   BDBG_ENTER(BXVD_DecoderTimer_S_CookResults_isrsafe);

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING

   for ( i = 0; i < BXVD_DecoderTimer_P_eFuncMax; i++ )
   {
      if ( 0 != pTimerData->astFunctions[ i ].uiNumSamples )
      {
         pTimerData->astFunctions[ i ].uiAverageTimeUsecs = pTimerData->astFunctions[ i ].uiTotalTimeUsecs / pTimerData->astFunctions[ i ].uiNumSamples;
      }
   }

#endif

   for ( i=0; i < BXVD_DecoderTimer_P_eCbMax; i++ )
   {
      if ( 0 != pTimerData->astCallbacks[ i ].uiNumSamples )
      {
         pTimerData->astCallbacks[ i ].uiAverageTimeUsecs = pTimerData->astCallbacks[ i ].uiTotalTimeUsecs / pTimerData->astCallbacks[ i ].uiNumSamples;
      }

   }

   BDBG_LEAVE(BXVD_DecoderTimer_S_CookResults_isrsafe);

   return;
}

void BXVD_DecoderTimer_S_ResetTimerData_isr(
   BXVD_DecoderTimer_P_Data * pTimerData
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_S_ResetTimerData_isr);

   BKNI_Memset_isr( pTimerData, 0, sizeof( BXVD_DecoderTimer_P_Data ) );

   BDBG_LEAVE(BXVD_DecoderTimer_S_ResetTimerData_isr);
   return;
}

#endif

/*
 * Public functions for measuring the elapse time of functions.
 * Conditionally compiled out to reduce overhead during normal play.
 */

void BXVD_DecoderTimer_P_SnapshotFunctionStartTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eFunc eFunctionIndex
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_P_SnapshotFunctionStartTime_isr);

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING

   BXVD_DecoderTimer_S_GetTime_isr(
         hXvdCh,
         &(hXvdCh->stDecoderContext.stLogData.stTimerData.astFunctions[ eFunctionIndex ].uiStartTimeUsecs)
         );

#else
   BSTD_UNUSED( hXvdCh );
   BSTD_UNUSED( eFunctionIndex );
#endif

   BDBG_LEAVE(BXVD_DecoderTimer_P_SnapshotFunctionStartTime_isr);

   return;
}

void BXVD_DecoderTimer_P_SnapshotFunctionEndTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eFunc eFunctionIndex
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_P_SnapshotFunctionEndTime_isr);

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING

   BXVD_DecoderTimer_S_SnapshotEndTimeAndUpdateElapse_isr(
         hXvdCh,
         &(hXvdCh->stDecoderContext.stLogData.stTimerData.astFunctions[ eFunctionIndex ])
         );

#else
   BSTD_UNUSED( hXvdCh );
   BSTD_UNUSED( eFunctionIndex );
#endif

   BDBG_LEAVE(BXVD_DecoderTimer_P_SnapshotFunctionEndTime_isr);

   return;
}

/*
 * Public functions for measuring the elapse time of callbacks.
 */

void BXVD_DecoderTimer_P_SnapshotCallbackStartTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eCallbacks eCallbackIndex
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_P_SnapshotCallbackStartTime_isr);

#if BXVD_DECODER_TIMER_S_ENABLE_CALLBACK_TIMING

   BXVD_DecoderTimer_S_GetTime_isr(
         hXvdCh,
         &(hXvdCh->stDecoderContext.stLogData.stTimerData.astCallbacks[ eCallbackIndex ].uiStartTimeUsecs)
         );

#else
   BSTD_UNUSED( hXvdCh );
   BSTD_UNUSED( eCallbackIndex );
#endif


   BDBG_LEAVE(BXVD_DecoderTimer_P_SnapshotCallbackStartTime_isr);

   return;
}

void BXVD_DecoderTimer_P_SnapshotCallbackEndTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eCallbacks eCallbackIndex
   )
{
   BDBG_ENTER(BXVD_DecoderTimer_P_SnapshotCallbackEndTime_isr);

#if BXVD_DECODER_TIMER_S_ENABLE_CALLBACK_TIMING

   BXVD_DecoderTimer_S_SnapshotEndTimeAndUpdateElapse_isr(
         hXvdCh,
         &(hXvdCh->stDecoderContext.stLogData.stTimerData.astCallbacks[ eCallbackIndex ])
         );

#else
   BSTD_UNUSED( hXvdCh );
   BSTD_UNUSED( eCallbackIndex );
#endif


   BDBG_LEAVE(BXVD_DecoderTimer_P_SnapshotCallbackEndTime_isr);

   return;
}

/*
 * Public function for printing the results.
 */

void BXVD_DecoderTimer_P_PrintResults_isr(
   BXVD_ChannelHandle hXvdCh
   )
{
   uint32_t                   uiVsyncThreshold;
   BXVD_DecoderTimer_P_Data * pTimerData;

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING
   char           szTemp[BXVD_DECODER_TIMER_P_MAX_STR_LEN];
   uint32_t       uiOffset=0;
   uint32_t       i;
   bool           bPrintFunctionTimes = false;
#endif

   BDBG_ENTER(BXVD_DecoderTimer_P_PrintResults_isr);

   /*
    * Only compile in the body of this function if timing data is being collected.
    */

#if ( BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING || BXVD_DECODER_TIMER_S_ENABLE_CALLBACK_TIMING )

   uiVsyncThreshold = BXVD_DECODER_TIMER_S_VSYNCS_PER_PRINT;

   if ( 0 == uiVsyncThreshold )
   {
      uiVsyncThreshold = hXvdCh->stDecoderContext.stLogData.uiVsyncsPerSecond;
   }

   pTimerData = &(hXvdCh->stDecoderContext.stLogData.stTimerData);

   if ( hXvdCh->stDecoderContext.stLogData.uiCallbackCount >= uiVsyncThreshold )
   {
      BXVD_DecoderTimer_S_CookResults_isrsafe( hXvdCh );

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING

      for ( i = 0; i < BXVD_DecoderTimer_P_eFuncMax; i++ )
      {
         if ( 0 != pTimerData->astFunctions[i].uiNumSamples )
         {
            uiOffset += BKNI_Snprintf(
                           szTemp + uiOffset,
                           BXVD_DECODER_TIMER_P_MAX_STR_LEN - uiOffset,
                           "%s: %u/%u/%u(%d) ",
                           sFunctionNameLUT[i],
                           pTimerData->astFunctions[i].uiAverageTimeUsecs,
                           pTimerData->astFunctions[i].uiMinTimeUsecs,
                           pTimerData->astFunctions[i].uiMaxTimeUsecs,
                           pTimerData->astFunctions[i].uiNumSamples
                           );

            bPrintFunctionTimes = true;
         }
      }

      if ( true == bPrintFunctionTimes )
      {
         BDBG_MSG(("%s", szTemp));
      }

#endif

      /* BKNI_Snprintf is very slow.  Since there are only two callback functions
       * to time, use discrete print statements instead of building up a string.
       */

      if ( 0 != pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiAverageTimeUsecs
           && 0 == pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiAverageTimeUsecs
         )

      {
         /* only the userdata callback has been executed. */
         BDBG_MSG(("UserData %u/%u/%u(%d) ",
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiAverageTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiMinTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiMaxTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiNumSamples
                     ));
      }
      else if ( 0 == pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiAverageTimeUsecs
           && 0 != pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiAverageTimeUsecs
         )

      {
         /* only the PPB received callback has been executed. */
         BDBG_MSG(("PPBRec %u/%u/%u(%d) ",
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiAverageTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiMinTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiMaxTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiNumSamples
                     ));
      }
      else if ( 0 != pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiAverageTimeUsecs
           && 0 != pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiAverageTimeUsecs
         )

      {
         /* both callbacks have been executed. */
         BDBG_MSG(("UserData %u/%u/%u(%d) PPBRec %u/%u/%u(%d) ",
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiAverageTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiMinTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiMaxTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbUserData ].uiNumSamples,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiAverageTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiMinTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiMaxTimeUsecs,
                     pTimerData->astCallbacks[ BXVD_DecoderTimer_P_eCbPPBReceived ].uiNumSamples
                     ));
      }

      BXVD_DecoderTimer_S_ResetTimerData_isr( &(hXvdCh->stDecoderContext.stLogData.stTimerData) );
   }

#else

   BSTD_UNUSED( hXvdCh );
   BSTD_UNUSED( uiVsyncThreshold );
   BSTD_UNUSED( pTimerData );

#endif

   BDBG_LEAVE(BXVD_DecoderTimer_P_PrintResults_isr);

   return;
}

#endif   /* if debug build */

