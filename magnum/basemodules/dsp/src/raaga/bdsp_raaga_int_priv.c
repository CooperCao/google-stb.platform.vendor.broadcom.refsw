/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


 #include "bdsp_raaga_int_priv.h"
#include "bdsp_raaga_priv.h"
#include "bdsp_raaga_types.h"


 BDBG_MODULE(bdsp_raaga_int);   /* Register software module with debug interface */

 /*------------------------- GLOBALS AND EXTERNS ------------------------------*/
 static const  BINT_Id ui32DSPSynInterruptId[] =
 {
#if defined BCHP_INT_ID_SYNC_MSG
     /* old Style RDB */
         BCHP_INT_ID_SYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_SYNC_MSG
#endif
#else
     BCHP_INT_ID_RAAGA_DSP_FW_INTH_SYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_SYNC_MSG
#endif
#endif
 };

 static const  BINT_Id ui32DSPAsynInterruptId[] =
 {
#if defined BCHP_INT_ID_ASYNC_MSG
         BCHP_INT_ID_ASYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_ASYNC_MSG
#endif
#else
     BCHP_INT_ID_RAAGA_DSP_FW_INTH_ASYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_ASYNC_MSG
#endif
#endif
 };

static const  BINT_Id ui32DSPWatchDogInterruptId[] =
{
#if defined BCHP_INT_ID_WATCHDOG_TIMER_ATTN
    BCHP_INT_ID_WATCHDOG_TIMER_ATTN
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_INTH_1_WATCHDOG_TIMER_ATTN
#endif
#else
    BCHP_INT_ID_RAAGA_DSP_INTH_WATCHDOG_TIMER_ATTN
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_INTH_1_WATCHDOG_TIMER_ATTN
#endif
#endif
 };

#define BDSP_RAAGA_DEFAULT_WATCHDOG_COUNT   (0x3FFFFFFF)


 /***************************************************************************
 Description:
     This API will be called when any synchronous interrupt will be raised by
     FW for any task.
 Returns:
     void
 See Also:
     BDSP_P_DSP2HostSyn_isr, BDSP_P_DSP2HostAsyn_isr, BDSP_P_InterruptInstall
 ***************************************************************************/
 static void BDSP_Raaga_P_DSP2HostSyn_isr(
         void    *pDeviceHandle,
         int     iParm2
 )
 {
     BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
     uint32_t    ui32SyncTaskIds = 0;
     uint32_t    ui32DspId = 0, ui32TaskId = 0;
     int  i=0;
     BDSP_RaagaTask *pRaagaTask;
     uint32_t uiTaskIndex=0;

     BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

     BDBG_ENTER(BDSP_Raaga_P_DSP2HostSyn_isr);


     BDBG_MSG (("... Sync Message Posted by DSP ... "));

     ui32DspId = iParm2;

     for(i  = 0; i < BDSP_RAAGA_MAX_FW_TASK_PER_DSP; i++)
     {

        pRaagaTask = pDevice->taskDetails.pRaagaTask[ui32DspId][i];

        if(pRaagaTask == NULL)
        {
            continue;
        }

        if (pRaagaTask->taskId == BDSP_P_INVALID_TASK_ID)
        {
            continue;
        }
         ui32SyncTaskIds = BDSP_Read32_isr(
                             pDevice->regHandle,
                             BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0  + pDevice->dspOffset[ui32DspId]);

         if (ui32SyncTaskIds & (1 << pRaagaTask->taskId))
         {
            ui32TaskId = pRaagaTask->taskId;

            BDSP_Write32_isr(
                         pDevice->regHandle,
                         BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 + pDevice->dspOffset[ui32DspId], ui32SyncTaskIds);
             uiTaskIndex = ui32TaskId - BDSP_RAAGA_TASK_ID_START_OFFSET;

             if (uiTaskIndex < BDSP_RAAGA_MAX_FW_TASK_PER_DSP)
             {
                 BKNI_SetEvent(pDevice->taskDetails.pRaagaTask[ui32DspId][uiTaskIndex]->hEvent);
             }

             switch(pRaagaTask->lastEventType)
             {
                 case BDSP_START_TASK_COMMAND_ID:
                     pRaagaTask->isStopped=false;
                     break;

                 case BDSP_STOP_TASK_COMMAND_ID:
                     pRaagaTask->isStopped=true;
                     break;

                 default:
                     break;
             }
         }
     }

     BDBG_LEAVE(BDSP_Raaga_P_DSP2HostSyn_isr);
     return;
 }

 static void BDSP_Raaga_P_CdbItbOverflow_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_CdbItbOverflow_isr: cdbItb Overflow interrupt occured for Task %d",
                 pRaagaTask->taskId));
     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pParam1,
             pRaagaTask->audioInterruptHandlers.cdbItbOverflow.param2);
     }
 }


 static void BDSP_Raaga_P_CdbItbUnderflow_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_CdbItbUnderflow_isr: cdbItb Underflow interrupt occured for Task %d",
                 pRaagaTask->taskId));
     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pParam1,
             pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.param2);
     }
 }

 static void BDSP_Raaga_P_EncoderOutputOverflow_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_EncoderOutputOverflow_isr: Encoder output Overflow interrupt occured for Task %d",
                 pRaagaTask->taskId));
     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.encoderOutputOverflow.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.encoderOutputOverflow.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.encoderOutputOverflow.pParam1,
             pRaagaTask->audioInterruptHandlers.encoderOutputOverflow.param2);
     }
 }

 static void BDSP_Raaga_P_AncillaryDataPresent_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("BDSP_Raaga_P_AncillaryDataPresent_isr: MPEG Ancillary Data interrupt occured for Task %d",
                 pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.ancillaryData.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.ancillaryData.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.ancillaryData.pParam1,
             pRaagaTask->audioInterruptHandlers.ancillaryData.param2);
    }

 }

  static void BDSP_Raaga_P_DialnormChange_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("BDSP_Raaga_P_DialnormChange_isr: Change in Dialnorm  Event occured for Task %d",
                 pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.dialnormChange.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.dialnormChange.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.dialnormChange.pParam1,
             pRaagaTask->audioInterruptHandlers.dialnormChange.param2);
    }

 }

 static void  BDSP_Raaga_P_DecoderUnlock_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_DecoderUnlock_isr: Decoder unlock interrupt occured for Task %d",
                 pRaagaTask->taskId));

    pRaagaTask->decLocked = false;
     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.unlock.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.unlock.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.unlock.pParam1,
             pRaagaTask->audioInterruptHandlers.unlock.param2);
     }
 }

 static void  BDSP_Raaga_P_DecoderLock_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_DecoderLock_isr: Decoder lock interrupt occured for Task %d",
                 pRaagaTask->taskId));

     pRaagaTask->decLocked = true;

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.lock.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.lock.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.lock.pParam1,
             pRaagaTask->audioInterruptHandlers.lock.param2);
     }
 }




static void  BDSP_Raaga_P_CrcError_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

     BDBG_MSG(("BDSP_Raaga_P_CrcError_isr: CRC error interrupt occured for Task %d",
                 pRaagaTask->taskId));
 }

  static void  BDSP_Raaga_P_SampleRateChange_isr(
     void *pTaskHandle , BDSP_DSPCHN_P_FwSampleinfo *psFwSampleInfo)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(psFwSampleInfo);

     BDBG_MSG(("BDSP_Raaga_P_SampleRateChange_isr: Sample rate change interrupt occured for Task %d",
                 pRaagaTask->taskId));


     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.sampleRateChange.pCallback_isr) {

         pRaagaTask->audioInterruptHandlers.sampleRateChange.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.sampleRateChange.pParam1,
             pRaagaTask->audioInterruptHandlers.sampleRateChange.param2,
             psFwSampleInfo->sStreamSamplingRate.ui32SamplingRate,
             psFwSampleInfo->sBaseSamplingRate.ui32SamplingRate);
     }
 }


  static void  BDSP_Raaga_P_BitRateChange_isr(
     void *pTaskHandle , BDSP_DSPCHN_P_FwBitRateChangeInfo *pFwBitrateChangeInfo)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioBitRateChangeInfo  bitrateChangeInfo;

    BDSP_RaagaStage *pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

    BSTD_UNUSED(pFwBitrateChangeInfo);
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_MSG(("BDSP_Raaga_P_BitRateChange_isr: Bit rate change interrupt occured for Task %d",
                 pRaagaTask->taskId));

     /*Prepare the structure for application */
     /*bitrateChangeInfo.eAlgorithm = pRaagaTask->settings.pBranchInfo[0]->sFwStgInfo[0].eAlgorithm;*/

     bitrateChangeInfo.eAlgorithm = pRaagaPrimaryStage->algorithm;

     bitrateChangeInfo.ui32BitRate = pFwBitrateChangeInfo->ui32BitRate;
     bitrateChangeInfo.ui32BitRateIndex = pFwBitrateChangeInfo->ui32BitRateIndex;


     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.bitrateChange.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.bitrateChange.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.bitrateChange.pParam1,
             pRaagaTask->audioInterruptHandlers.bitrateChange.param2,
             &bitrateChangeInfo);
     }

 }

  static void  BDSP_Raaga_P_ModeChange_isr(
      void *pTaskHandle , BDSP_DSPCHN_P_FwModeChangeInfo *pFwModeChangeInfo)
  {

     BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
     uint32_t ui32ModeValue;

     BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_MSG(("BDSP_Raaga_P_ModeChange_isr: mode change interrupt occured for Task %d",
               pRaagaTask->taskId));


     /*Prepare the structure for application */
     ui32ModeValue = pFwModeChangeInfo->ui32ModeValue;

    /* Call the application Stream Info Available  callback function */
    if(pRaagaTask->audioInterruptHandlers.modeChange.pCallback_isr) {
      pRaagaTask->audioInterruptHandlers.modeChange.pCallback_isr (
          pRaagaTask->audioInterruptHandlers.modeChange.pParam1,
          pRaagaTask->audioInterruptHandlers.modeChange.param2,
          ui32ModeValue);
    }

  }


 static void  BDSP_Raaga_P_FirstPtsReady_isr(
     void *pTaskHandle , BDSP_Raaga_P_PtsInfo *pPtsInfo)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus ptsInfo;


    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Raaga_P_FirstPtsReady_isr: First PTS ready interrupt occured for Task %d",
             pRaagaTask->taskId));

    /* Call the application Stream Info Available  callback function */
    if(pRaagaTask->audioInterruptHandlers.firstPts.pCallback_isr) {
     pRaagaTask->audioInterruptHandlers.firstPts.pCallback_isr (
         pRaagaTask->audioInterruptHandlers.firstPts.pParam1,
         pRaagaTask->audioInterruptHandlers.firstPts.param2,
         &ptsInfo);
    }
 }

static void  BDSP_Raaga_P_TsmFail_isr(
     void *pTaskHandle , BDSP_Raaga_P_PtsInfo*pPtsInfo)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Raaga_P_TsmFail_isr: TSm Fail interrupt occured for Task %d",
             pRaagaTask->taskId));

    /* Call the application Stream Info Available  callback function */
    if(pRaagaTask->audioInterruptHandlers.tsmFail.pCallback_isr) {
     pRaagaTask->audioInterruptHandlers.tsmFail.pCallback_isr (
         pRaagaTask->audioInterruptHandlers.tsmFail.pParam1,
         pRaagaTask->audioInterruptHandlers.tsmFail.param2,
         &ptsInfo);
    }
}

 static void  BDSP_Raaga_P_StartPtsReached_isr(
  void *pTaskHandle)
 {

     BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

     BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_MSG(("BDSP_Raaga_P_StartPtsReached_isr: Start PTS reached interrupt occured for Task %d",
              pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.startPtsReached.pCallback_isr) {
      pRaagaTask->audioInterruptHandlers.startPtsReached.pCallback_isr (
          pRaagaTask->audioInterruptHandlers.startPtsReached.pParam1,
          pRaagaTask->audioInterruptHandlers.startPtsReached.param2);
     }
 }

 static void  BDSP_Raaga_P_StopPtsReached_isr(
  void *pTaskHandle)
 {

     BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

     BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_MSG(("BDSP_Raaga_P_StopPtsReached_isr: Start PTS reached interrupt occured for Task %d",
              pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.startPtsReached.pCallback_isr) {
      pRaagaTask->audioInterruptHandlers.startPtsReached.pCallback_isr (
          pRaagaTask->audioInterruptHandlers.startPtsReached.pParam1,
          pRaagaTask->audioInterruptHandlers.startPtsReached.param2);
     }
 }

 static void  BDSP_Raaga_P_AstmPass_isr(
     void *pTaskHandle , BDSP_Raaga_P_PtsInfo *pPtsInfo)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Raaga_P_AstmPass_isr: ASTM Pass interrupt occured for Task %d",
             pRaagaTask->taskId));

    /* Call the application TSM Pass callback function */
    if(pRaagaTask->audioInterruptHandlers.tsmPass.pCallback_isr) {
     pRaagaTask->audioInterruptHandlers.tsmPass.pCallback_isr (
         pRaagaTask->audioInterruptHandlers.tsmPass.pParam1,
         pRaagaTask->audioInterruptHandlers.tsmPass.param2,
         &ptsInfo);
    }
}

  static void  BDSP_Raaga_P_RampEnable_isr(
     void *pTaskHandle , BDSP_DSPCHN_RampEnableInfo *pRampEnableInfo)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pRampEnableInfo);

    BDBG_MSG(("BDSP_Raaga_P_RampEnable_isr: Ramp Enable interrupt occured for Task %d",
             pRaagaTask->taskId));

    /* Call the application Stream Info Available  callback function */
    if(pRaagaTask->audioInterruptHandlers.rampEnable.pCallback_isr) {
        pRaagaTask->audioInterruptHandlers.rampEnable.pCallback_isr (
        pRaagaTask->audioInterruptHandlers.rampEnable.pParam1,
        pRaagaTask->audioInterruptHandlers.rampEnable.param2);
    }


}

  static void  BDSP_Raaga_P_StreamInfoAvailable_isr(
      void *pTaskHandle )
 {

     BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

     BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_MSG(("BDSP_Raaga_P_StreamInfoAvailable_isr: Stream Information available interrupt occured for Task %d",
              pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.statusReady.pCallback_isr) {
      pRaagaTask->audioInterruptHandlers.statusReady.pCallback_isr (
          pRaagaTask->audioInterruptHandlers.statusReady.pParam1,
          pRaagaTask->audioInterruptHandlers.statusReady.param2);
     }
 }


static void  BDSP_Raaga_P_UnlicensedAlgo_isr(
    void *pTaskHandle,
    BDSP_Raaga_P_UnsupportedAlgoInfo *pUnlicensedAlgoInfo
    )
{
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    BDBG_ERR(("BDSP_P_DSP2HostAsyn_isr: Unlicensed Algo (%u) interrupt occured for Task %d",
            pUnlicensedAlgoInfo->ui32AudioAlgorithm,
            pRaagaTask->taskId));

    /* Call the application Stream Info Available  callback function */
    if(pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr) {
        pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr (
        pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pParam1,
        pRaagaTask->audioInterruptHandlers.unlicensedAlgo.param2);
    }
}

 static void  BDSP_Raaga_P_VencDataDiscarded_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("BDSP_Raaga_P_VencDataDiscarded_isr: Video Encoder data discarded Event occured for Task %d",
                 pRaagaTask->taskId));

     /* Call the application Stream Info Available  callback function */
     if(pRaagaTask->audioInterruptHandlers.vencDataDiscarded.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.vencDataDiscarded.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.vencDataDiscarded.pParam1,
             pRaagaTask->audioInterruptHandlers.vencDataDiscarded.param2);
    }

}

 static void  BDSP_Raaga_P_OnDemandAudioFrameDelivered_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("BDSP_Raaga_P_OnDemandAudioFrameDelivered_isr: OnDemand Audio Output Delivered Event occured for Task %d",
                 pRaagaTask->taskId));

    if(pRaagaTask->audioInterruptHandlers.onDemandAudioFrameDelivered.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.onDemandAudioFrameDelivered.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.onDemandAudioFrameDelivered.pParam1,
             pRaagaTask->audioInterruptHandlers.onDemandAudioFrameDelivered.param2);
    }

}

  static void  BDSP_Raaga_P_TargetVolumeReached_isr(
     void *pTaskHandle , void *pParam2)
 {

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BSTD_UNUSED(pParam2);

    BDBG_MSG(("BDSP_Raaga_P_TargetVolumeReached_isr: Volume level reached  Event occured for Task %d",
                 pRaagaTask->taskId));

    if(pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pParam1,
             pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.param2);
    }

}


 /***************************************************************************
 Description:
     This API will be called when any asynchronous interrupt will be raised by
     FW for any task.
 Returns:
     void
 See Also:
     BDSP_P_DSP2HostSyn_isr, BDSP_P_DSP2HostAsyn_isr, BDSP_P_InterruptInstall
 ***************************************************************************/
 static void BDSP_Raaga_P_DSP2HostAsyn_isr(
            void    *pDeviceHandle,
            int     iParm2
 )
 {
    uint32_t    ui32ASyncTaskIds = 0;
    uint32_t    ui32DspId = 0;
    int i=0;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDSP_RaagaTask *pRaagaTask;
    BDSP_Raaga_P_AsynEventMsg  *pEventMsg = NULL;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
    BDBG_ENTER(BDSP_Raaga_P_DSP2HostAsyn_isr);

    BDBG_MSG (("... Async Message(s) Posted by DSP ... "));

    ui32DspId = iParm2;


    for(i = 0; i < BDSP_RAAGA_MAX_FW_TASK_PER_DSP; i++)
    {
        pRaagaTask = pDevice->taskDetails.pRaagaTask[ui32DspId][i];
        if(pRaagaTask == NULL)
        {
            continue;
        }

        if((pRaagaTask->taskId == BDSP_P_INVALID_TASK_ID))
        {
            continue;
        }

        if((void *)pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr == NULL)
        {
            continue;
        }

        ui32ASyncTaskIds = BDSP_Read32_isr(
                                pDevice->regHandle,
                                BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1  + pDevice->dspOffset[ui32DspId]);


        if (ui32ASyncTaskIds & (1 << pRaagaTask->taskId))
        {
            unsigned int uiNumMsgs = 0, i = 0;

            BDSP_Write32_isr(
                pDevice->regHandle,
                BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1  + pDevice->dspOffset[ui32DspId], (1 << pRaagaTask->taskId));

            pEventMsg = pRaagaTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr;

            /* Read the message in sEventMsg */
            BDSP_Raaga_P_GetAsyncMsg_isr(pRaagaTask->hAsyncMsgQueue,pEventMsg, &uiNumMsgs);

            /* Now check which response came from FW and work according to it*/
            for(i = 0; i < uiNumMsgs; i++)
            {
                if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_FRAME_SYNC_LOCK_LOST_EVENT_ID)
                {
                    BDSP_Raaga_P_DecoderUnlock_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_FRAME_SYNC_LOCK_EVENT_ID)
                {
                    BDSP_Raaga_P_DecoderLock_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_SAMPLING_RATE_CHANGE_EVENT_ID)
                {
                    BDSP_Raaga_P_SampleRateChange_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sFwSampleInfo));
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_BIT_RATE_CHANGE_EVENT_ID)
                {
                    BDSP_Raaga_P_BitRateChange_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sFwBitRateChange));
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_AUDIO_MODE_CHANGE_EVENT_ID)
                {
                    BDSP_Raaga_P_ModeChange_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sFwModeChange));
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_CRC_ERROR_EVENT_ID)
                {
                    BDSP_Raaga_P_CrcError_isr((void *)pRaagaTask,NULL);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID)
                {
                    BDSP_Raaga_P_FirstPtsReady_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sPtsInfo));
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_START_PTS_EVENT_ID)
                {
                    BDSP_Raaga_P_StartPtsReached_isr((void *)pRaagaTask);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_STOP_PTS_EVENT_ID)
                {
                    BDSP_Raaga_P_StopPtsReached_isr((void *)pRaagaTask);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_ASTMTSM_PASS_EVENT_ID)
                {
                    BDSP_Raaga_P_AstmPass_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sPtsInfo));
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_TSM_FAIL_EVENT_ID)
                {
                    BDSP_Raaga_P_TsmFail_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sPtsInfo));  ;
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_RAMP_ENABLE_EVENT_ID)
                {
                    BDSP_Raaga_P_RampEnable_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sRampEnableInfo));  ;
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_STREAM_INFO_AVAIL_EVENT_ID)
                {
                    BDSP_Raaga_P_StreamInfoAvailable_isr((void *)pRaagaTask);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_CDB_ITB_OVERFLOW_EVENT_ID)
                {
                    BDSP_Raaga_P_CdbItbOverflow_isr((void *)pRaagaTask,NULL);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_CDB_ITB_UNDERFLOW_EVENT_ID)
                {
                    BDSP_Raaga_P_CdbItbUnderflow_isr((void *)pRaagaTask,NULL);
                }
                else if (pEventMsg[i].sMsgHeader.ui32EventID  == BDSP_RAAGA_UNLICENSED_ALGO_EVENT_ID)
                {
                    BDSP_Raaga_P_UnlicensedAlgo_isr((void *)pRaagaTask,&(pEventMsg[i].uFWMessageInfo.sUnsupportedAlgoInfo));
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_ENCODER_OVERFLOW_EVENT_ID)
                {
                    BDSP_Raaga_P_EncoderOutputOverflow_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_ANCDATA_EVENT_ID)
                {
                    BDSP_Raaga_P_AncillaryDataPresent_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_CHANGE_IN_DIALNORM_EVENT_ID)
                {
                    BDSP_Raaga_P_DialnormChange_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_VENC_DATA_DISCARDED_EVENT_ID)
                {
                    BDSP_Raaga_P_VencDataDiscarded_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_ONDEMAND_AUDIO_FRAME_DELIVERED_EVENT_ID)
                {
                    BDSP_Raaga_P_OnDemandAudioFrameDelivered_isr((void *)pRaagaTask,NULL);
                }
                else if(pEventMsg[i].sMsgHeader.ui32EventID == BDSP_RAAGA_VOLUME_LEVEL_REACHED_EVENT_ID)
                {
                    BDSP_Raaga_P_TargetVolumeReached_isr((void *)pRaagaTask,NULL);
                }
                else
                {
                    BDBG_WRN (("BDSP_Raaga_P_DSP2HostAsyn_isr :: <<< Not a Valid Async Message. EventID >>> %d",
                        pEventMsg[i].sMsgHeader.ui32EventID));
                }
            }
        }
    } /*    for(ui32DspId =0; ui32DspId < BDSP_RM_P_MAX_DSPS; ++ui32DspId) */

    BDBG_LEAVE(BDSP_Raaga_P_DSP2HostAsyn_isr);
    return;
}




  /***************************************************************************
 Description:
     This API uninstalls the top level interrupt handlers for all the interrups.
 Returns:
     BERR_Code
 See Also:
     BDSP_P_InterruptUnInstall
 ***************************************************************************/
 BERR_Code BDSP_Raaga_P_InterruptUninstall (
     void    *pTaskHandle        /* [in] Raptor Channel handle */
 )
 {
     BERR_Code       ret = BERR_SUCCESS;
     unsigned int    dspIndex;
     BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

     BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

     BDBG_ENTER (BDSP_Raaga_P_InterruptUninstall);

     dspIndex = pRaagaTask->settings.dspIndex;

    /* We will create two callbacks for Asynchronous Interrupt & for
        Synchtonous interrupt on both DAP0 as well as on DS1 */

    if(pRaagaTask->interruptCallbacks.hDSPAsync[dspIndex])
    {
         ret = BINT_DisableCallback(pRaagaTask->interruptCallbacks.hDSPAsync[dspIndex]);
         if (ret!=BERR_SUCCESS)
         {
             ret = BERR_TRACE(ret);
             goto end;
         }
         ret = BINT_DestroyCallback(pRaagaTask->interruptCallbacks.hDSPAsync[dspIndex]);
         if (ret!=BERR_SUCCESS)
         {
             ret = BERR_TRACE(ret);
             goto end;
         }
    }

     if(pRaagaTask->interruptCallbacks.hDSPSync[dspIndex])
     {
         ret = BINT_DisableCallback(pRaagaTask->interruptCallbacks.hDSPSync[dspIndex]);
         if (ret!=BERR_SUCCESS)
         {
             ret = BERR_TRACE(ret);
             goto end;
         }
         ret = BINT_DestroyCallback(pRaagaTask->interruptCallbacks.hDSPSync[dspIndex]);
         if (ret!=BERR_SUCCESS)
         {
             ret = BERR_TRACE(ret);
             goto end;
         }
    }

 end:
     BDBG_LEAVE(BDSP_Raaga_P_InterruptUninstall);
     return ret;
 }


 /***************************************************************************
Description:
    This API installs the top level interrupt handlers for all the interrups.
Returns:
    BERR_Code
See Also:
    BDSP_P_InterruptUnInstall
***************************************************************************/
BERR_Code BDSP_Raaga_P_InterruptInstall (
    void    *pTaskHandle        /* [in] Raptor Channel handle */
)
{
    BERR_Code       ret = BERR_SUCCESS;
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    unsigned int    dspIndex = pRaagaTask->settings.dspIndex;
    BDSP_Raaga *pDevice = pRaagaTask->pContext->pDevice;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    BDBG_ENTER (BDSP_Raaga_P_InterruptInstall);

    /* We will create two callbacks for Asynchronous Interrupt & for
       Synchtonous interrupt on both DAP0 as well as on DS1 */
    ret = BINT_CreateCallback(
                                &pRaagaTask->interruptCallbacks.hDSPAsync[dspIndex],
                                pDevice->intHandle,
                                ui32DSPAsynInterruptId[dspIndex],
                                BDSP_Raaga_P_DSP2HostAsyn_isr,
                                (void*)pDevice,
                                dspIndex
                                );
    if(ret != BERR_SUCCESS )
    {
        BDBG_ERR(("Create Callback failed for DSP = %d INT_ID_ESR_SO1",dspIndex));
        ret = BERR_TRACE(ret);
        goto err_callback;
    }
    ret = BINT_EnableCallback(pRaagaTask->interruptCallbacks.hDSPAsync[dspIndex]);
    if(ret != BERR_SUCCESS )
    {
        BDBG_ERR(("Enable Callback failed for DSP = %d INT_ID_ESR_SO1",dspIndex));
        ret = BERR_TRACE(ret);
        goto err_callback;
    }

    ret = BINT_CreateCallback(
                                &pRaagaTask->interruptCallbacks.hDSPSync[dspIndex],
                                pDevice->intHandle,
                                ui32DSPSynInterruptId[dspIndex],
                                BDSP_Raaga_P_DSP2HostSyn_isr,
                                (void*)pDevice,
                                dspIndex
                                );
    if(ret != BERR_SUCCESS )
    {
        BDBG_ERR(("Create Callback failed for DSP = %d BCHP_INT_ID_ESR_SO0",dspIndex));
        ret = BERR_TRACE(ret);
        goto err_callback;
    }

    ret = BINT_EnableCallback(pRaagaTask->interruptCallbacks.hDSPSync[dspIndex]);
    if(ret != BERR_SUCCESS )
    {
        BDBG_ERR(("Enable Callback failed for DSP = %d BCHP_INT_ID_ESR_SO0",dspIndex));
        ret = BERR_TRACE(ret);
        goto err_callback;
    }


goto end;

err_callback:
    if(ret != BERR_SUCCESS )
    {
        BDSP_Raaga_P_InterruptUninstall(pTaskHandle);
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_InterruptInstall);
    return ret;
}

/***************************************************************************
Description:
    The APIs below are the low level interrupt hanlders which are dispatched
    from the top level handlers
    iParm2 = dspIndex
Returns:
    All of them return void
Note:
    This comment is common for all the APIs below
***************************************************************************/

static void BDSP_Raaga_P_Watchdog_isr(void *pDeviceHandle,  int iParm2 )
{
    BDSP_Raaga   *pRaagaDevice = (BDSP_Raaga   *)pDeviceHandle;
    BDSP_RaagaContext *pRaagaContext=NULL;

    BDBG_OBJECT_ASSERT(pRaagaDevice, BDSP_Raaga);

    BDBG_MSG((" watchdog received from dspIndex = %d", iParm2));

    pRaagaDevice->deviceWatchdogFlag = true;

    /* Mask all the external interrupts for this dsp */
    BDSP_Write32(pRaagaDevice->regHandle, \
        BCHP_RAAGA_DSP_ESR_SI_MASK_SET + pRaagaDevice->dspOffset[iParm2], \
        0xffffffff);

    BDSP_Raaga_P_ResetHardware_isr(pDeviceHandle, iParm2);

    for ( pRaagaContext = BLST_S_FIRST(&pRaagaDevice->contextList);
          pRaagaContext != NULL;
          pRaagaContext = BLST_S_NEXT(pRaagaContext, node) )
    {
        pRaagaContext->contextWatchdogFlag = true;
        BDBG_WRN(("Raaga Watchdog Interrupt called for dsp %d. Calling Function callback for  Context handle 0x%p",iParm2, (void *)pRaagaContext));

        /* Call the application callback */
        if(pRaagaContext->interruptHandlers.watchdog.pCallback_isr)
        {

            pRaagaContext->interruptHandlers.watchdog.pCallback_isr (
                pRaagaContext->interruptHandlers.watchdog.pParam1,
                iParm2
            );
        }
    }
}


/***************************************************************************
Description:
    This API Installs callbacks for device level interrupts.
Returns:
    BERR_Code
See Also:
    BDSP_P_DeviceLevelInterruptUnInstall
***************************************************************************/
BERR_Code BDSP_Raaga_P_ContextInterruptInstall (
    void    *pContextHandle
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t    ui32DspIndex;
    BDSP_RaagaContext   *pRaagaContext =(BDSP_RaagaContext *)pContextHandle;

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

    if(BLST_S_EMPTY(&(pRaagaContext->pDevice->contextList))) /*callback should be created for first context*/
    {
        for(ui32DspIndex = 0; ui32DspIndex < pRaagaContext->pDevice->numDsp; ui32DspIndex++)
        {
            ret = BINT_CreateCallback(
                        &pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex],
                        pRaagaContext->pDevice->intHandle,
                        ui32DSPWatchDogInterruptId[ui32DspIndex],
                        BDSP_Raaga_P_Watchdog_isr,
                        (void*)pRaagaContext->pDevice,
                        ui32DspIndex
                        );

            if (ret!=BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret);
                goto end;
            }
        }

    }
end:
    return ret;
}


/***************************************************************************
Description:
    This API Uninstalls callbacks for device level interrupts.
Returns:
    BERR_Code
See Also:
    BDSP_P_DeviceLevelInterruptUnInstall
***************************************************************************/
BERR_Code BDSP_Raaga_P_ContextInterruptUninstall (
    void    *pContextHandle
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t    ui32DspIndex;
    BDSP_RaagaContext   *pRaagaContext =(BDSP_RaagaContext *)pContextHandle;

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

    if(BLST_S_EMPTY(&(pRaagaContext->pDevice->contextList))) /*callback  should be destroyed for last context*/
    {
        for(ui32DspIndex = 0; ui32DspIndex < pRaagaContext->pDevice->numDsp; ui32DspIndex++)
        {
            if(pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex])
            {
                 ret = BINT_DisableCallback(pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex]);
                 if (ret!=BERR_SUCCESS)
                 {
                     ret = BERR_TRACE(ret);
                     goto end;
                 }
                 ret = BINT_DestroyCallback(pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex]);
                 if (ret!=BERR_SUCCESS)
                 {
                     ret = BERR_TRACE(ret);
                     goto end;
                 }
                 pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex] = NULL;
            }
        }
    }

end:
    BDBG_LEAVE (BDSP_P_DeviceLevelInterruptInstall);
    return ret;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetInterruptHandlers

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle of the task for which settings are required.
                pInterrupts     -   Pointer returned to the PI.

Return      :   None

Functionality   :   Return the Interrupt Handler of the Context back to the PI.
***********************************************************************/

void BDSP_Raaga_P_GetInterruptHandlers(
    void *pContextHandle,
    BDSP_ContextInterruptHandlers *pInterrupts)
{
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_ASSERT(pInterrupts);

    *pInterrupts = pRaagaContext->interruptHandlers;

}

void BDSP_Raaga_P_RestoreWatchdogTimer(
                        void        *pDeviceHandle)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    unsigned dspIndex;

    for(dspIndex = 0; dspIndex < pDevice->numDsp; dspIndex++)
    {
        BDSP_Raaga_P_EnableDspWatchdogTimer( pDeviceHandle, dspIndex, pDevice->dpmInfo.bWatchdogTimerStatus);
    }
}

void BDSP_Raaga_P_RestoreDspWatchdogTimer(
                        void        *pDeviceHandle,
                        uint32_t    dspIndex)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDSP_Raaga_P_EnableDspWatchdogTimer( pDeviceHandle, dspIndex, pDevice->dpmInfo.bWatchdogTimerStatus);
}

void BDSP_Raaga_P_EnableDspWatchdogTimer (
                        void        *pDeviceHandle,
                        uint32_t    dspIndex,
                        bool        bEnable
                        )
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    uint32_t regVal = 0;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_MSG(("%s watchdog: DSP %d", bEnable?"Enable":"Disable", dspIndex));

    if (bEnable)
    {
        /* Program default watchdog count */
        regVal = (regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT)))
                        | BCHP_FIELD_DATA(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT, BDSP_RAAGA_DEFAULT_WATCHDOG_COUNT);
        /* Disable auto reload of count */
        regVal = regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, CM));
        /* Enable timer bit */
        regVal = regVal | BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, ET);
    }
    else {
        /* Program default watchdog count */
        regVal = (regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT)))
                        | BCHP_FIELD_DATA(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, COUNT, BDSP_RAAGA_DEFAULT_WATCHDOG_COUNT);
        /* Disable timer bit */
        regVal = regVal & ~(BCHP_MASK(RAAGA_DSP_TIMERS_WATCHDOG_TIMER, ET));
    }
    BDBG_ASSERT((dspIndex + 1) <= BDSP_RAAGA_MAX_DSP);
    BDSP_Write32(pDevice->regHandle, BCHP_RAAGA_DSP_TIMERS_WATCHDOG_TIMER + pDevice->dspOffset[dspIndex], regVal);

}


void BDSP_Raaga_P_EnableWatchdogTimer (
                        void    *pDeviceHandle,
                        bool    bEnable
                        )
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    uint32_t dspIndex=0;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    for(dspIndex = 0; dspIndex < pDevice->numDsp; dspIndex++)
    {
        BDSP_Raaga_P_EnableDspWatchdogTimer( pDeviceHandle, dspIndex, bEnable);
    }
}


void BDSP_Raaga_P_EnableAndSaveStateWatchdogTimer (
                        void    *pDeviceHandle,
                        bool    bEnable
                        )
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDSP_Raaga_P_EnableWatchdogTimer( pDeviceHandle, bEnable);

    /* Save the status */
    pDevice->dpmInfo.bWatchdogTimerStatus = bEnable;

}

/***********************************************************************
Name        :   BDSP_Raaga_P_SetInterruptHandlers

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle of the task for which settings are required.
                pInterrupts     -   Handle of the Interrupt from the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
    1)  Enable the Watchdog timer for Raaga.
    2)  For each DSP, enable the call back registered as part if Device handle.
    3)  Store the interrupt handler provided by the PI as part of the Context handle.
***********************************************************************/

BERR_Code BDSP_Raaga_P_SetInterruptHandlers(
    void *pContextHandle,
    const BDSP_ContextInterruptHandlers *pInterrupts)
{
    BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    uint32_t            ui32DspIndex;
    BERR_Code err=BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_ASSERT(pInterrupts);


    if(pInterrupts->watchdog.pCallback_isr != NULL)
    {
        BDSP_Raaga_P_EnableAndSaveStateWatchdogTimer((void *)pRaagaContext->pDevice,true);

        for(ui32DspIndex = 0; ui32DspIndex < pRaagaContext->pDevice->numDsp; ui32DspIndex++)
        {
            if(pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex])
            {
                err = BINT_EnableCallback (pRaagaContext->pDevice->hWatchdogCallback[ui32DspIndex]);
                if(err!=BERR_SUCCESS)
                {
                    return BERR_TRACE(err);
                }
            }
        }
    }

    pRaagaContext->interruptHandlers = *pInterrupts;

    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_ProcessWatchdogInterrupt

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle provided by the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   On occurance of an watchdog, call the Raaga Open function.
                On completion, clear the watchdog flag of the Device handle and Context handle
***********************************************************************/

BERR_Code BDSP_Raaga_P_ProcessWatchdogInterrupt(
    void *pContextHandle)
{
    BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    BERR_Code err=BERR_SUCCESS;
    unsigned dspIndex;
    BDSP_Raaga  *pDevice= pRaagaContext->pDevice;

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

    BKNI_EnterCriticalSection();
    /* BDSP_Raaga_P_ProcessWatchdogInterrupt() should not be called from within
    Critical sections. To ensure this, added these few lines of code.
    Do nothing here. If BDSP_Raaga_P_ProcessWatchdogInterrupt is called incorrectly from
    a critical section, this piece of code will cause it to deadlock and exit */
    BKNI_LeaveCriticalSection();

    BKNI_AcquireMutex(pDevice->watchdogMutex);
    BDBG_WRN(("BDSP_Raaga_P_ProcessWatchdogInterrupt called"));


    if(pDevice->deviceWatchdogFlag == true)
    {
        BDSP_Raaga_P_Reset((void *)pDevice);

        BDSP_Raaga_P_Open((void *)pDevice);
        err = BDSP_Raaga_P_Boot((void *)pDevice);

        if (err!=BERR_SUCCESS)
        {
            err= BERR_TRACE(err);
            goto err_boot;
        }

        for( dspIndex =0 ; dspIndex < pDevice->numDsp ; dspIndex++)
        {
            BDSP_Raaga_P_RestoreDspWatchdogTimer((void *)pDevice,dspIndex);
        }

        pRaagaContext->pDevice->deviceWatchdogFlag = false;
    }

    pRaagaContext->contextWatchdogFlag = false;

err_boot:
    BKNI_ReleaseMutex(pRaagaContext->pDevice->watchdogMutex);

    return err;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetAudioInterruptHandlers_isr

Type        :   PI Interface

Input       :   pTaskHandle     -   Handle of the Task for which interrupt handle must be retreived.
                pHandlers       -   Pointer of the interrupt handle returned back to PI.

Return      :   None

Functionality   :   Following are the operations performed.
        1)  Return the audio interrupt handle to the PI.
***********************************************************************/

void BDSP_Raaga_P_GetAudioInterruptHandlers_isr(
    void *pTaskHandle,
    BDSP_AudioInterruptHandlers *pHandlers)
{
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BDBG_ASSERT(pHandlers);

    *pHandlers = pRaagaTask->audioInterruptHandlers;

}

/***********************************************************************
Name        :   BDSP_Raaga_P_SetAudioInterruptHandlers_isr

Type        :   PI Interface

Input       :   pTaskHandle     -   Handle of the Task for which interrupt call back must be set.
                pHandlers       -   Pointer of the interrupt handle provided by PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Depending on all the Interrupt call back functions installed, set the event enabled masks for the task.
        2)  Send the Event notification command to the FW.
        3)  Update the audio interrupt handler for the task to the handle provided by PI.
***********************************************************************/

BERR_Code BDSP_Raaga_P_SetAudioInterruptHandlers_isr(
    void *pTaskHandle,
    const BDSP_AudioInterruptHandlers *pHandlers)
{
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pRaagaTask->pContext;
    BDSP_Raaga  *pDevice= pRaagaContext->pDevice;
    BERR_Code err=BERR_SUCCESS;
    BDSP_Raaga_P_Command     sFwCommand;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_ASSERT(pHandlers);

/*First PTS Interrupt*/
    if((pHandlers->firstPts.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eFirstPTS_Received);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eFirstPTS_Received);
    }

/*Sample Rate Change Interrupt*/
    if((pHandlers->sampleRateChange.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eSampleRateChange);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eSampleRateChange);
    }

/*TSM Fail Interrupt*/
    if((pHandlers->tsmFail.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eTsmFail);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eTsmFail);
    }

/*TSM Pass Interrupt*/
    if((pHandlers->tsmPass.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eAstmTsmPass);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eAstmTsmPass);
    }

/*Lock Interrupt*/
    if((pHandlers->lock.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eFrameSyncLock);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eFrameSyncLock);
    }

/*Unlock Interrupt*/
    if((pHandlers->unlock.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eFrameSyncLockLost);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eFrameSyncLockLost);
    }

/*Status Interrupt*/
    if((pHandlers->statusReady.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eStreamInfoAvail);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eStreamInfoAvail);
    }

    /*Mode Change Interrupt*/
    if((pHandlers->modeChange.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eAudioModeChange);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eAudioModeChange);
    }

    /*Bit rate Change Interrupt*/
    if((pHandlers->bitrateChange.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eBitRateChange);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eBitRateChange);
    }

    /*CDB/ITB overflow Interrupt*/
    if((pHandlers->cdbItbOverflow.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eCdbItbOverflow);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eCdbItbOverflow);
    }

    /*CDB/ITB underflow Interrupt*/
    if((pHandlers->cdbItbUnderflow.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eCdbItbUnderflow);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eCdbItbUnderflow);
    }

    /*Start PTS reached Interrupt*/
    if((pHandlers->startPtsReached.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eStartOnPTS);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eStartOnPTS);
    }

    /*Stop PTS reached Interrupt*/
    if((pHandlers->stopPtsReached.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eStopOnPTS);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eStopOnPTS);
    }

    /*Unlicensed Algo Interrupt*/
    if((pHandlers->unlicensedAlgo.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eUnlicensedAlgo);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eUnlicensedAlgo);
    }

    /*Ramp Enable Interrupt*/
    if((pHandlers->rampEnable.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eRampEnable);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eRampEnable);
    }

    /*Encoder output overflow Interrupt*/
    if((pHandlers->encoderOutputOverflow.pCallback_isr == NULL))
    {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eEncoderOverflow);
    }
    else
    {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eEncoderOverflow);
    }

     /* Ancillary Data Interrupt */
     if((pHandlers->ancillaryData.pCallback_isr == NULL))
     {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eAncData);
     }
     else
     {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eAncData);
     }

     /* Dialnorm Change Interrupt */
     if((pHandlers->dialnormChange.pCallback_isr == NULL))
     {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eChangeInDialnorm);
     }
     else
     {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eChangeInDialnorm);
     }

     if((pHandlers->onDemandAudioFrameDelivered.pCallback_isr == NULL))
     {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eOnDemandAudioFrameDelivered);
     }
     else
     {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eOnDemandAudioFrameDelivered);
     }

     /* Volume Level Reached Interrupt */
     if((pHandlers->targetVolumeLevelReached.pCallback_isr == NULL))
     {
        /*Send the command to DSP to mask the interrupt*/
        pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eTargetVolumeLevelReached);
     }
     else
     {
        /*Send the command to DSP to unmask the interrupt*/
        pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eTargetVolumeLevelReached);
     }

    /* Video Encode Data Discard Interrupt */
    if((pHandlers->vencDataDiscarded.pCallback_isr == NULL))
    {
       /*Send the command to DSP to mask the interrupt*/
       pRaagaTask->eventEnabledMask &= ~(BDSP_Raaga_P_EventIdMask_eVencDataDiscarded);
    }
    else
    {
       /*Send the command to DSP to unmask the interrupt*/
       pRaagaTask->eventEnabledMask |= (BDSP_Raaga_P_EventIdMask_eVencDataDiscarded);
    }

    if((pRaagaTask->isStopped == false))
    {
        /*  Prepare message structure for FW to write in message queue */
        sFwCommand.sCommandHeader.ui32CommandID = BDSP_EVENT_NOTIFICATION_COMMAND_ID;
        sFwCommand.sCommandHeader.ui32CommandSizeInBytes = sizeof(BDSP_Raaga_P_Command);
        sFwCommand.sCommandHeader.eResponseType = BDSP_P_ResponseType_eNone;
        sFwCommand.sCommandHeader.ui32TaskID = pRaagaTask->taskId;
        sFwCommand.sCommandHeader.ui32CommandCounter = pRaagaTask->commandCounter++;;
        sFwCommand.sCommandHeader.ui32CommandTimeStamp =  BREG_Read32(pDevice->regHandle,
                                                                      BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);
        sFwCommand.uCommand.sEnableDisableEvent.ui32EnableEvent =
            pRaagaTask->eventEnabledMask ;

        /* Write in Message queue */
        err = BDSP_Raaga_P_SendCommand_isr(pDevice->hCmdQueue[pRaagaTask->settings.dspIndex], &sFwCommand,(void *)pRaagaTask);
        if(err != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_SetAudioInterruptHandlers_isr: Send command failed"));
            err = BERR_TRACE(err);
            goto end;
        }
    }
    pRaagaTask->audioInterruptHandlers = *pHandlers;
end:
    return err;
}


/**********************************************************
Function: BDSP_Raaga_P_RestoreInterrupts_isr

Description :  Isr version of  BDSP_Raaga_P_RestoreInterrupts
                Restores the interrupts to their state
                before reset

**********************************************************/
uint32_t BDSP_Raaga_P_RestoreInterrupts_isr(
    void                *pDeviceHandle,
    uint32_t    uiDspIndex)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BERR_Code err = BERR_SUCCESS;
    uint32_t L2Reg ;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
    /* Dsp Sync and Async interrupt registers use the same L2 register so using only one
    of the name of the interrupts to derive the L2 register name*/
    L2Reg = BCHP_INT_ID_GET_REG(ui32DSPSynInterruptId[uiDspIndex]);
    BINT_ApplyL2State_isr(pDevice->intHandle,L2Reg );

    L2Reg = BCHP_INT_ID_GET_REG(ui32DSPWatchDogInterruptId[uiDspIndex]);
    BINT_ApplyL2State_isr(pDevice->intHandle,L2Reg );

    return err;

}
