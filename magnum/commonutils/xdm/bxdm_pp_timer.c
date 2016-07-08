/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bxdm_pp_timer.h"

BDBG_MODULE(BXDM_PPTMR);

/* SW7445-2997: added 12.5 -> 20 Hz display rates. */
const uint32_t BXDM_PPTMR_lutVsyncsPersSecond[BXDM_PictureProvider_MonitorRefreshRate_eMax]=
{
   60,   /* BXDM_PictureProvider_MonitorRefreshRate_eUnknown */
   7,   /* BXDM_PictureProvider_MonitorRefreshRate_e7_493Hz */
   7,   /* BXDM_PictureProvider_MonitorRefreshRate_e7_5Hz */
   10,   /* BXDM_PictureProvider_MonitorRefreshRate_e9_99Hz */
   10,   /* BXDM_PictureProvider_MonitorRefreshRate_e10Hz */
   12,   /* BXDM_PictureProvider_MonitorRefreshRate_e11_988Hz */
   12,   /* BXDM_PictureProvider_MonitorRefreshRate_e12Hz */
   12,   /* BXDM_PictureProvider_MonitorRefreshRate_e12_5Hz */
   15,   /* BXDM_PictureProvider_MonitorRefreshRate_e14_985Hz */
   15,   /* BXDM_PictureProvider_MonitorRefreshRate_e15Hz */
   20,   /* BXDM_PictureProvider_MonitorRefreshRate_e19_98Hz */
   20,   /* BXDM_PictureProvider_MonitorRefreshRate_e20Hz */
   24,   /* BXDM_PictureProvider_MonitorRefreshRate_e23_97 */
   24,   /* BXDM_PictureProvider_MonitorRefreshRate_e24 */
   25,   /* BXDM_PictureProvider_MonitorRefreshRate_e25 */
   30,   /* BXDM_PictureProvider_MonitorRefreshRate_e29_97 */
   30,   /* BXDM_PictureProvider_MonitorRefreshRate_e30 */
   48,   /* BXDM_PictureProvider_MonitorRefreshRate_e48 */
   50,   /* BXDM_PictureProvider_MonitorRefreshRate_e50 */
   60,   /* BXDM_PictureProvider_MonitorRefreshRate_e59_94 */
   60,   /* BXDM_PictureProvider_MonitorRefreshRate_e60 */
  100,   /* BXDM_PictureProvider_MonitorRefreshRate_e100 */
  120,   /* BXDM_PictureProvider_MonitorRefreshRate_e119_88 */
  120,   /* BXDM_PictureProvider_MonitorRefreshRate_e120 */
}; /* end of BXDM_PPTMR_lutVsyncsPersSecond */

#if BDBG_DEBUG_BUILD

#if ( BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING || BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING )

/*
 * Result will be printed every BXDM_PPTMR_S_VSYNCS_PER_PRINT vsync's
 * For a value of '0', results are printed once per second.
 */
#define BXDM_PPTMR_S_VSYNCS_PER_PRINT  0

/*
 * Local function prototypes
 */
void BXDM_PPTMR_S_CookResults_isr( BXDM_PictureProvider_Handle hXdmPP );
void BXDM_PPTMR_S_ResetTimerData_isr( BXDM_PPTIMER_P_Data * pTimerData );

/*
 * Private functions
 */

void BXDM_PPTMR_S_GetTime_isrsafe(
   BXDM_PictureProvider_Handle hXdmPP,
   uint32_t * pTimeUsec
   )
{
   if( NULL != hXdmPP->hTimer )
   {
      BTMR_ReadTimer_isr( hXdmPP->hTimer, pTimeUsec );
   }
   else
   {
      *pTimeUsec = 0;
   }

   return;
}


void BXDM_PPTMR_S_SnapshotEndTimeAndUpdateElapse_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPTIMER_P_Sample * pTimerSample
   )
{
   BDBG_ENTER(BXDM_PPTMR_S_SnapshotEndTimeAndUpdateElapse_isr);

   BXDM_PPTMR_S_GetTime_isrsafe( hXdmPP, &(pTimerSample->uiEndTimeUsecs) );

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

   BDBG_LEAVE(BXDM_PPTMR_S_SnapshotEndTimeAndUpdateElapse_isr);

   return;
}


void BXDM_PPTMR_S_CookResults_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   BXDM_PPTIMER_P_Data * pTimerData = &(hXdmPP->stDMState.stDecode.stTimerData);

   uint32_t i;

   BDBG_ENTER(BXDM_PPTMR_S_CookResults_isr);

#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING

   for ( i = 0; i < BXDM_PPTIMER_P_Function_eMax; i++ )
   {
      if ( 0 != pTimerData->astFunctions[ i ].uiNumSamples )
      {
         pTimerData->astFunctions[ i ].uiAverageTimeUsecs = pTimerData->astFunctions[ i ].uiTotalTimeUsecs / pTimerData->astFunctions[ i ].uiNumSamples;
      }
   }

#endif

   for ( i=0; i < BXDM_PictureProvider_Callback_eMax; i++ )
   {
      if ( 0 != pTimerData->astCallbacks[ i ].uiNumSamples )
      {
         pTimerData->astCallbacks[ i ].uiAverageTimeUsecs = pTimerData->astCallbacks[ i ].uiTotalTimeUsecs / pTimerData->astCallbacks[ i ].uiNumSamples;
      }

   }

   BDBG_LEAVE(BXDM_PPTMR_S_CookResults_isr);

   return;
}

#endif

/*
 * Public functions for measuring the elapse time of functions.
 * Conditionally compiled out to reduce overhead during normal play.
 */
void BXDM_PPTMR_P_SnapshotFunctionStartTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPTIMER_P_Function eFunctionIndex
   )
{
   BDBG_ENTER(BXDM_PPTMR_P_SnapshotFunctionStartTime_isr);

#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING

   BXDM_PPTMR_S_GetTime_isrsafe(
         hXdmPP,
         &(hXdmPP->stDMState.stDecode.stTimerData.astFunctions[ eFunctionIndex ].uiStartTimeUsecs)
         );

#else
   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( eFunctionIndex );
#endif

   BDBG_LEAVE(BXDM_PPTMR_P_SnapshotFunctionStartTime_isr);

   return;
}

void BXDM_PPTMR_P_SnapshotFunctionEndTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPTIMER_P_Function eFunctionIndex
   )
{
   BDBG_ENTER(BXDM_PPTMR_P_SnapshotFunctionEndTime_isr);

#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING

   BXDM_PPTMR_S_SnapshotEndTimeAndUpdateElapse_isr(
         hXdmPP,
         &(hXdmPP->stDMState.stDecode.stTimerData.astFunctions[ eFunctionIndex ])
         );

#else
   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( eFunctionIndex );
#endif

   BDBG_LEAVE(BXDM_PPTMR_P_SnapshotFunctionEndTime_isr);

   return;
}

/*
 * Public functions for measuring the elapse time of callbacks.
 */

void BXDM_PPTMR_P_SnapshotCallbackStartTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_Callback eCallbackIndex
   )
{
   BDBG_ENTER(BXDM_PPTMR_P_SnapshotCallbackStartTime_isr);

#if BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING

   BXDM_PPTMR_S_GetTime_isrsafe(
         hXdmPP,
         &(hXdmPP->stDMState.stDecode.stTimerData.astCallbacks[ eCallbackIndex ].uiStartTimeUsecs)
         );

#else
   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( eCallbackIndex );
#endif


   BDBG_LEAVE(BXDM_PPTMR_P_SnapshotCallbackStartTime_isr);

   return;
}

void BXDM_PPTMR_P_SnapshotCallbackEndTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_Callback eCallbackIndex
   )
{
   BDBG_ENTER(BXDM_PPTMR_P_SnapshotCallbackEndTime_isr);

#if BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING

   BXDM_PPTMR_S_SnapshotEndTimeAndUpdateElapse_isr(
         hXdmPP,
         &(hXdmPP->stDMState.stDecode.stTimerData.astCallbacks[ eCallbackIndex ])
         );

#else
   BSTD_UNUSED( hXdmPP );
   BSTD_UNUSED( eCallbackIndex );
#endif


   BDBG_LEAVE(BXDM_PPTMR_P_SnapshotCallbackEndTime_isr);

   return;
}

/*
 * Public function for printing the results.
 */
#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING

static const char* const sFunctionNameLUT[BXDM_PPTIMER_P_Function_eMax] =
{
   "Main",
   "Init",
   "DecIsr",
   "Select",
   "Pic1",
   "Pic2",
   "Pic3",
   "ValPCHead",
   "ValPCTail",
   "Eval",
   "VDC",
   "Update",
   "CallBck",
   "Post",
   "GetCnt",
   "PeekPic",
   "GetPic",
   "RelPic",
   "DrP",
   "DrR",
};

#endif

#if BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING

static const char* const sCallbackNameLUT[BXDM_PictureProvider_Callback_eMax] =
{
   "StcPtsOff",   /* BXDM_PictureProvider_Callback_eStcPtsOffset */
   "FirstPic",    /* BXDM_PictureProvider_Callback_eFirstPTSReady */
   "PTSCoded",    /* BXDM_PictureProvider_Callback_eFirstCodedPTSReady */
   "PTSPass",     /* BXDM_PictureProvider_Callback_eFirstPTSPassed */
   "PTSError",    /* BXDM_PictureProvider_Callback_ePTSError */
   "IFrame",      /* BXDM_PictureProvider_Callback_eIFrame */
   "PicParms",    /* BXDM_PictureProvider_Callback_ePictureParameters */
   "ASTM",        /* BXDM_PictureProvider_Callback_eTSMPassInASTMMode */
   "ClpStr",      /* BXDM_PictureProvider_Callback_eClipStart */
   "ClpStp",      /* BXDM_PictureProvider_Callback_eClipStop */
   "Marker",      /* BXDM_PictureProvider_Callback_ePictureMarker */
   "ReqStc",      /* BXDM_PictureProvider_Callback_eRequestSTC */
   "PPBParms",    /* BXDM_PictureProvider_Callback_ePictureUnderEvaluation */
   "TSM",         /* BXDM_PictureProvider_Callback_eTSMResult */
   "ExtData",     /* BXDM_PictureProvider_Callback_ePictureExtensionData */
   "DecErr",      /* BXDM_PictureProvider_Callback_eDecodeError */
   "CnkDone"      /* BXDM_PictureProvider_Callback_eChunkDone */
};

#endif

void BXDM_PPTMR_P_PrintResults_isr(
   BXDM_PictureProvider_Handle hXdmPP
   )
{
   char           szTemp[BXDM_PP_TIMER_P_MAX_STR_LEN];
   uint32_t       uiOffset=0;
   uint32_t       i;
   uint32_t       uiVsyncThreshold;

   BXDM_PPTIMER_P_Data *         pTimerData;
   BXDM_PictureProvider_MonitorRefreshRate  eMonitorRefreshRate;

   bool           bPrintFunctionTimes = false;

   BDBG_ENTER(BXDM_PPTMR_P_PrintResults_isr);

#if ( BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING || BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING )

   pTimerData = &(hXdmPP->stDMState.stDecode.stTimerData);

   /* Bump the vsync count to know when to print the results. */

   pTimerData->uiVsyncCnt++;

   /* Calculate the "print" threshold.  */

   uiVsyncThreshold = BXDM_PPTMR_S_VSYNCS_PER_PRINT;

   if ( 0 == uiVsyncThreshold )
   {

      if ( hXdmPP->stDMConfig.eMonitorRefreshRate >= BXDM_PictureProvider_MonitorRefreshRate_eMax )
      {
         eMonitorRefreshRate = BXDM_PictureProvider_MonitorRefreshRate_e60Hz;
      }
      else
      {
         eMonitorRefreshRate = hXdmPP->stDMConfig.eMonitorRefreshRate;
      }

      uiVsyncThreshold = BXDM_PPTMR_lutVsyncsPersSecond[ eMonitorRefreshRate ];
   }


   if ( pTimerData->uiVsyncCnt >= uiVsyncThreshold )
   {
      BXDM_PPTMR_S_CookResults_isr( hXdmPP );

      BKNI_Memset(szTemp,0,sizeof(szTemp));

#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING

      for ( i = 0; i < BXDM_PPTIMER_P_Function_eMax; i++ )
      {
         if ( 0 != pTimerData->astFunctions[i].uiNumSamples )
         {
            uiOffset += BKNI_Snprintf(
                           szTemp + uiOffset,
                           BXDM_PP_TIMER_P_MAX_STR_LEN - uiOffset,
                           "%s: %u/%u/%u(%d) ",
                           sFunctionNameLUT[i],
                           pTimerData->astFunctions[i].uiAverageTimeUsecs,
                           pTimerData->astFunctions[i].uiMinTimeUsecs,
                           pTimerData->astFunctions[i].uiMaxTimeUsecs,
                           pTimerData->astFunctions[i].uiNumSamples
                           );

            /* SW7445-572: if the size of the concatenated string is getting close
             * to the size of "szTemp", print the string now to avoid truncating
             * the debug message.  With the smaller BXDM_PP_TIMER_P_MAX_STR_LEN (256),
             * this frequently happens for the function timing messages; it seldom
             * happens for the callback timing messages. */

            if ( uiOffset > BXDM_PP_TIMER_P_PRINT_THRESHOLD )
            {
               BDBG_MSG(("%x:[%02x.xxx] %s",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                     szTemp ));

               uiOffset = 0;
               BKNI_Memset(szTemp,0,sizeof(szTemp));
               bPrintFunctionTimes = false;
            }
            else
            {
               bPrintFunctionTimes = true;
            }

         }
      }

      if ( true == bPrintFunctionTimes )
      {
         BDBG_MSG(("%x:[%02x.xxx] %s",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                     szTemp ));
         uiOffset = 0;
         BKNI_Memset(szTemp,0,sizeof(szTemp));
      }

#endif

#if BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING

      for ( i=0; i < BXDM_PictureProvider_Callback_eMax; i++ )
      {
         if ( 0 != pTimerData->astCallbacks[ i ].uiAverageTimeUsecs )
         {
            uiOffset += BKNI_Snprintf(
                           szTemp + uiOffset,
                           BXDM_PP_TIMER_P_MAX_STR_LEN - uiOffset,
                           "%s: %u/%u/%u(%d) ",
                           sCallbackNameLUT[i],
                           pTimerData->astCallbacks[i].uiAverageTimeUsecs,
                           pTimerData->astCallbacks[i].uiMinTimeUsecs,
                           pTimerData->astCallbacks[i].uiMaxTimeUsecs,
                           pTimerData->astCallbacks[i].uiNumSamples
                           );

            /* SW7445-572: if the size of the concatenated string is getting close
             * to the size of "szTemp", print the string now to avoid truncating
             * the debug message.  With the smaller BXDM_PP_TIMER_P_MAX_STR_LEN (256),
             * this frequently happens for the function timing messages; it seldom
             * happens for the callback timing messages. */

            if ( uiOffset > BXDM_PP_TIMER_P_PRINT_THRESHOLD )
            {
               BDBG_MSG(("%x:[%02x.xxx] %s",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                     szTemp ));
               uiOffset = 0;
               BKNI_Memset(szTemp,0,sizeof(szTemp));
               bPrintFunctionTimes = false;

            }
            else
            {
               bPrintFunctionTimes = true;
            }

         }
      }

      if ( true == bPrintFunctionTimes )
      {
         BDBG_MSG(("%x:[%02x.xxx] %s",
                     hXdmPP->stDMState.stDecode.stDebug.uiVsyncCount,
                     BXDM_PPDBG_FORMAT_INSTANCE_ID( hXdmPP ),
                     szTemp ));
      }
#endif

      BXDM_PPTMR_S_ResetTimerData_isr( &(hXdmPP->stDMState.stDecode.stTimerData) );
   }

#else

   BSTD_UNUSED( szTemp );
   BSTD_UNUSED( uiOffset );
   BSTD_UNUSED( i );
   BSTD_UNUSED( uiVsyncThreshold );
   BSTD_UNUSED( pTimerData );
   BSTD_UNUSED( eMonitorRefreshRate );
   BSTD_UNUSED( hXdmPP );

#endif

   BDBG_LEAVE(BXDM_PPTMR_P_PrintResults_isr);

   return;
}

void BXDM_PPTMR_S_ResetTimerData_isr(
   BXDM_PPTIMER_P_Data * pTimerData
   )
{
   BDBG_ENTER(BXDM_PPTMR_S_ResetTimerData_isr);

   BKNI_Memset_isr( pTimerData, 0, sizeof( BXDM_PPTIMER_P_Data ) );

   BDBG_LEAVE(BXDM_PPTMR_S_ResetTimerData_isr);
   return;
}

#endif   /* if debug build */

