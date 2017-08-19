/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bdsp_raaga_priv_include.h"

BDBG_MODULE(bdsp_raaga_int);

static const  BINT_Id DSPGenRespInterruptId[] =
{
#if defined BCHP_INT_ID_HOST_MSG
     BCHP_INT_ID_HOST_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    ,BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_HOST_MSG
#endif
#else
     BCHP_INT_ID_RAAGA_DSP_FW_INTH_HOST_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_HOST_MSG
#endif
#endif
};

static const  BINT_Id DSPSynInterruptId[] =
{
#if defined BCHP_INT_ID_SYNC_MSG
     BCHP_INT_ID_SYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    ,BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_SYNC_MSG
#endif
#else
     BCHP_INT_ID_RAAGA_DSP_FW_INTH_SYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    , BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_SYNC_MSG
#endif
#endif
};

static const  BINT_Id DSPAsynInterruptId[] =
{
#if defined BCHP_INT_ID_ASYNC_MSG
     BCHP_INT_ID_ASYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    ,BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_ASYNC_MSG
#endif
#else
     BCHP_INT_ID_RAAGA_DSP_FW_INTH_ASYNC_MSG
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
	,BCHP_INT_ID_RAAGA_DSP_FW_INTH_1_ASYNC_MSG
#endif
#endif
};

static const  BINT_Id DSPWatchDogInterruptId[] =
{
#if defined BCHP_INT_ID_WATCHDOG_TIMER_ATTN
    BCHP_INT_ID_WATCHDOG_TIMER_ATTN
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    ,BCHP_INT_ID_RAAGA_DSP_INTH_1_WATCHDOG_TIMER_ATTN
#endif
#else
    BCHP_INT_ID_RAAGA_DSP_INTH_WATCHDOG_TIMER_ATTN
#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
    ,BCHP_INT_ID_RAAGA_DSP_INTH_1_WATCHDOG_TIMER_ATTN
#endif
#endif
};

static void  BDSP_Raaga_P_FrameSyncUnlock_isr(
	void *pTaskHandle)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

   BDBG_MSG(("BDSP_Raaga_P_FrameSyncUnlock_isr: FrameSync unlock interrupt occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application FrameSync Unlock callback function */
	if(pRaagaTask->audioInterruptHandlers.unlock.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.unlock.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.unlock.pParam1,
			pRaagaTask->audioInterruptHandlers.unlock.param2);
	}
}

static void  BDSP_Raaga_P_FrameSyncLock_isr(
	void *pTaskHandle)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

   BDBG_MSG(("BDSP_Raaga_P_FrameSyncLock_isr: FrameSync lock interrupt occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application FrameSync Lock callback function */
	if(pRaagaTask->audioInterruptHandlers.lock.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.lock.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.lock.pParam1,
			pRaagaTask->audioInterruptHandlers.lock.param2);
	}
}

static void  BDSP_Raaga_P_SampleRateChange_isr(
	BDSP_RaagaTask *pRaagaTask,
	BDSP_P_SampleRateChangeInfo *pSampleRateChangeInfo

)
{
	BDBG_ENTER(BDSP_Raaga_P_SampleRateChange_isr);
	BDBG_MSG(("BDSP_Raaga_P_SampleRateChange_isr: Received Sample Rate Change for Task (%d)", pRaagaTask->taskParams.taskId));
	BDBG_MSG(("BaseSampleRate = %d \t StreamSampleRate = %d",pSampleRateChangeInfo->ui32BaseSampleRate,pSampleRateChangeInfo->ui32StreamSampleRate));
	if(pRaagaTask->audioInterruptHandlers.sampleRateChange.pCallback_isr)
	{
		pRaagaTask->audioInterruptHandlers.sampleRateChange.pCallback_isr (
					pRaagaTask->audioInterruptHandlers.sampleRateChange.pParam1,
					pRaagaTask->audioInterruptHandlers.sampleRateChange.param2,
					pSampleRateChangeInfo->ui32StreamSampleRate,
					pSampleRateChangeInfo->ui32BaseSampleRate);
	}
	BDBG_LEAVE(BDSP_Raaga_P_SampleRateChange_isr);
}

static void BDSP_Raaga_P_CdbItbOverflow_isr(
	void *pTaskHandle)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
   BDBG_MSG(("BDSP_Raaga_P_CdbItbOverflow_isr: cdbItb Overflow event occured for Task %d",pRaagaTask->taskParams.taskId));
	/* Call the application CDB/ITB Overflow callback function */
	if(pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.cdbItbOverflow.pParam1,
			pRaagaTask->audioInterruptHandlers.cdbItbOverflow.param2);
	}
}

static void BDSP_Raaga_P_CdbItbUnderflow_isr(
	void *pTaskHandle )
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
   BDBG_MSG(("BDSP_Raaga_P_CdbItbUnderflow_isr: cdbItb Underflow event occured for Task %d",pRaagaTask->taskParams.taskId));
	/* Call the application CDB/ITB underflow callback function */
	if(pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.pParam1,
			pRaagaTask->audioInterruptHandlers.cdbItbUnderflow.param2);
	}
}

static void  BDSP_Raaga_P_FirstPtsReceived_isr(
	void *pTaskHandle , BDSP_P_PtsInfo *pPtsInfo)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDSP_AudioTaskTsmStatus ptsInfo;


   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
   ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
   ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
   ptsInfo.ePtsType = pPtsInfo->ePtsType;
   ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

   BDBG_MSG(("BDSP_Raaga_P_FirstPtsReceived_isr: First PTS received event occured for Task %d",pRaagaTask->taskParams.taskId));
   BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

   /* Call the application first pts received callback function */
   if(pRaagaTask->audioInterruptHandlers.firstPts.pCallback_isr) {
	pRaagaTask->audioInterruptHandlers.firstPts.pCallback_isr (
		pRaagaTask->audioInterruptHandlers.firstPts.pParam1,
		pRaagaTask->audioInterruptHandlers.firstPts.param2,
		&ptsInfo);
   }
}


static void  BDSP_Raaga_P_TsmFail_isr(
     void *pTaskHandle , BDSP_P_PtsInfo*pPtsInfo)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Raaga_P_TsmFail_isr: TSm Fail event occured for Task %d", pRaagaTask->taskParams.taskId));
	BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

    /* Call the application TSM fail callback function */
    if(pRaagaTask->audioInterruptHandlers.tsmFail.pCallback_isr) {
     pRaagaTask->audioInterruptHandlers.tsmFail.pCallback_isr (
         pRaagaTask->audioInterruptHandlers.tsmFail.pParam1,
         pRaagaTask->audioInterruptHandlers.tsmFail.param2,
         &ptsInfo);
    }
}

static void  BDSP_Raaga_P_AstmPass_isr(
     void *pTaskHandle , BDSP_P_PtsInfo *pPtsInfo)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Raaga_P_AstmPass_isr: ASTM Pass event occured for Task %d",pRaagaTask->taskParams.taskId));
	BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

    /* Call the application ASTM TSM Pass callback function */
    if(pRaagaTask->audioInterruptHandlers.tsmPass.pCallback_isr) {
     pRaagaTask->audioInterruptHandlers.tsmPass.pCallback_isr (
         pRaagaTask->audioInterruptHandlers.tsmPass.pParam1,
         pRaagaTask->audioInterruptHandlers.tsmPass.param2,
         &ptsInfo);
    }
}

static void  BDSP_Raaga_P_AcmodeChange_isr(
	void *pTaskHandle , BDSP_P_AcmodeChangeInfo *pAcmodeChangeInfo)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   uint32_t ui32ModeValue;

   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

   BDBG_MSG(("BDSP_Raaga_P_AcmodeChange_isr: mode change event occured for Task %d",pRaagaTask->taskParams.taskId));
   BDBG_MSG(("New ModeValue = %d",pAcmodeChangeInfo->ui32ModeValue));


   /*Prepare the structure for application */
   ui32ModeValue = pAcmodeChangeInfo->ui32ModeValue;

  /* Call the application Acmode change callback function */
  if(pRaagaTask->audioInterruptHandlers.modeChange.pCallback_isr) {
	pRaagaTask->audioInterruptHandlers.modeChange.pCallback_isr (
		pRaagaTask->audioInterruptHandlers.modeChange.pParam1,
		pRaagaTask->audioInterruptHandlers.modeChange.param2,
		ui32ModeValue);
  }

}

static void  BDSP_Raaga_P_BitrateChange_isr(
	void *pTaskHandle , BDSP_P_BitrateChangeInfo *pBitrateChangeInfo)
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDSP_AudioBitRateChangeInfo	bitrateChangeInfo;

   BDSP_RaagaStage *pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

   BDBG_MSG(("BDSP_Raaga_P_BitRateChange_isr: Bit rate change event occured for Task %d",pRaagaTask->taskParams.taskId));
   BDBG_MSG(("Algorithm = %d \t BitRate = %d \t BitRateIndex = %d ",pRaagaPrimaryStage->eAlgorithm,pBitrateChangeInfo->ui32BitRate, pBitrateChangeInfo->ui32BitRateIndex));

	/*Prepare the structure for application */
	bitrateChangeInfo.eAlgorithm = pRaagaPrimaryStage->eAlgorithm;
	bitrateChangeInfo.ui32BitRate = pBitrateChangeInfo->ui32BitRate;
	bitrateChangeInfo.ui32BitRateIndex = pBitrateChangeInfo->ui32BitRateIndex;


	/* Call the application Bitrate change callback function */
	if(pRaagaTask->audioInterruptHandlers.bitrateChange.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.bitrateChange.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.bitrateChange.pParam1,
			pRaagaTask->audioInterruptHandlers.bitrateChange.param2,
			&bitrateChangeInfo);
	}

}

static void  BDSP_Raaga_P_StreamInfoAvailable_isr(
	 void *pTaskHandle )
{

	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	BDBG_MSG(("BDSP_Raaga_P_StreamInfoAvailable_isr: Stream Information available event occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application Stream Info Available  callback function */
	if(pRaagaTask->audioInterruptHandlers.statusReady.pCallback_isr) {
	 pRaagaTask->audioInterruptHandlers.statusReady.pCallback_isr (
		 pRaagaTask->audioInterruptHandlers.statusReady.pParam1,
		 pRaagaTask->audioInterruptHandlers.statusReady.param2);
	}
}

static void  BDSP_Raaga_P_UnlicensedAlgo_isr(
	 void *pTaskHandle,
	 BDSP_Raaga_P_UnlicensedAlgoInfo *pUnlicensedAlgoInfo)
{
	 BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	 BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	 BDBG_ERR(("BDSP_Raaga_P_UnlicensedAlgo_isr: Unlicensed Algo (%u) event occured for Task %d", pUnlicensedAlgoInfo->ui32AudioAlgorithm,pRaagaTask->taskParams.taskId));

	 /* Call the application UnlicensedAlgo	callback function */
	 if(pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr) {
		 pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr (
		 pRaagaTask->audioInterruptHandlers.unlicensedAlgo.pParam1,
		 pRaagaTask->audioInterruptHandlers.unlicensedAlgo.param2);
	 }
}

static void BDSP_Raaga_P_AncillaryDataAvailable_isr(
	void *pTaskHandle )
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

   BDBG_MSG(("BDSP_Raaga_P_AncillaryDataAvailable_isr: MPEG Ancillary Data event occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application Ancillary data present callback function */
	if(pRaagaTask->audioInterruptHandlers.ancillaryData.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.ancillaryData.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.ancillaryData.pParam1,
			pRaagaTask->audioInterruptHandlers.ancillaryData.param2);
   }

}

static void BDSP_Raaga_P_DialnormChange_isr(
	void *pTaskHandle )
{

   BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);


   BDBG_MSG(("BDSP_Raaga_P_DialnormChange_isr: Change in Dialnorm event occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application Dialnorm change callback function */
	if(pRaagaTask->audioInterruptHandlers.dialnormChange.pCallback_isr) {
		pRaagaTask->audioInterruptHandlers.dialnormChange.pCallback_isr (
			pRaagaTask->audioInterruptHandlers.dialnormChange.pParam1,
			pRaagaTask->audioInterruptHandlers.dialnormChange.param2);
   }

}

static void  BDSP_Raaga_P_VolumeLevelReached_isr(
     void *pTaskHandle)
{

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    BDBG_MSG(("BDSP_Raaga_P_VolumeLevelReached_isr: Volume level reached event occured for Task %d",pRaagaTask->taskParams.taskId));

	/* Call the application volume level reached callback function */
    if(pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr) {
         pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr (
             pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.pParam1,
             pRaagaTask->audioInterruptHandlers.targetVolumeLevelReached.param2);
    }

}


static void BDSP_Raaga_P_Dsp2TaskAsyn_isr(
	void    *pDeviceHandle,
	int	   iParm2
)
{
    BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
    BDSP_RaagaTask *pRaagaTask;
    uint32_t    ui32DspId = 0;
    uint32_t    ui32ASyncTaskIds = 0;
    BDSP_Raaga_P_AsynMsg  *pEventMsg = NULL;
	int  i=0;

	BDBG_ENTER(BDSP_Raaga_P_Dsp2TaskAsyn_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	ui32DspId = iParm2;
    BDBG_MSG (("... Async Message(s) Posted by DSP %d... ", ui32DspId));

    for(i = 0; i < BDSP_RAAGA_MAX_FW_TASK_PER_DSP; i++)
    {
		pRaagaTask = (BDSP_RaagaTask *)pDevice->taskDetails[ui32DspId].pTask[i];
		if(pRaagaTask == NULL)
		{
			continue;
		}
		if (pRaagaTask->taskParams.taskId == BDSP_P_INVALID_TASK_ID)
		{
			continue;
		}

		ui32ASyncTaskIds = BDSP_ReadReg32_isr(pDevice->regHandle,
					BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1 + pDevice->dspOffset[ui32DspId]);

		if (ui32ASyncTaskIds & (1 << pRaagaTask->taskParams.taskId))
		{
			unsigned int uiNumMsgs = 0, i = 0;
			BDSP_WriteReg32_isr(pDevice->regHandle,
			BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1 + pDevice->dspOffset[ui32DspId],
			(1 << pRaagaTask->taskParams.taskId));

			BDBG_MSG(("ASYNC MSG Received from Task %d", pRaagaTask->taskParams.taskId));

			pEventMsg = (BDSP_Raaga_P_AsynMsg *)pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr;

			/* Read the message in sEventMsg */
			BDSP_Raaga_P_GetAsyncMsg_isr(pRaagaTask->hAsyncQueue, pEventMsg, &uiNumMsgs);

			/* Now check which response came from FW and work according to it*/
			for(i = 0; i < uiNumMsgs; i++)
			{
				if(pEventMsg[i].sAsynHeader.eEventID== BDSP_P_EventID_CDB_ITB_UNDERFLOW)
				{
				   BDSP_Raaga_P_CdbItbUnderflow_isr((void *)pRaagaTask);
				}
				else if(pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_CDB_ITB_OVERFLOW)
				{
				   BDSP_Raaga_P_CdbItbOverflow_isr((void *)pRaagaTask);
				}
				else if(pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_SAMPLING_RATE_CHANGE)
				{
				   BDSP_Raaga_P_SampleRateChange_isr((void *)pRaagaTask,&(pEventMsg[i].uInfo.sSampleRateChangeInfo));
				}
				else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FRAME_SYNC_LOCK)
				{
				   BDSP_Raaga_P_FrameSyncLock_isr((void *)pRaagaTask);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FRAME_SYNC_UNLOCK)
				{
				   BDSP_Raaga_P_FrameSyncUnlock_isr((void *)pRaagaTask);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FIRST_PTS_RECEIVED)
				{
				   BDSP_Raaga_P_FirstPtsReceived_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sPtsInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_TSM_FAIL)
				{
				   BDSP_Raaga_P_TsmFail_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sPtsInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ASTM_TSM_PASS)
				{
				   BDSP_Raaga_P_AstmPass_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sPtsInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ACMODE_CHANGE)
				{
				   BDSP_Raaga_P_AcmodeChange_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sAcmodeChangeInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_BIT_RATE_CHANGE)
				{
				   BDSP_Raaga_P_BitrateChange_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sBitrateChangeInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_STREAM_INFO_AVAILABLE)
				{
				   BDSP_Raaga_P_StreamInfoAvailable_isr((void *)pRaagaTask);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_UNLICENSED_ALGO)
				{
				   BDSP_Raaga_P_UnlicensedAlgo_isr((void *)pRaagaTask, &pEventMsg[i].uInfo.sUnlicensedAlgoInfo);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE)
				{
				   BDSP_Raaga_P_AncillaryDataAvailable_isr((void *)pRaagaTask);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_DIAL_NORM_CHANGE)
				{
				   BDSP_Raaga_P_DialnormChange_isr((void *)pRaagaTask);
				}
				else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_VOLUME_LEVEL_REACHED)
				{
				   BDSP_Raaga_P_VolumeLevelReached_isr((void *)pRaagaTask);
				}
				else
				{
				   BDBG_WRN (("BDSP_Raaga_P_DSP2HostAsyn_isr :: <<< Async Message is Not Valid or Not Supported. EventID >>> %d",
					   pEventMsg[i].sAsynHeader.eEventID));
				}
			}

		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_Dsp2TaskAsyn_isr);
}

/***************************************************************************
Description:
 This API will be called when any synchronous interrupt will be raised by
 FW for any task.
Returns:
 void
See Also:
***************************************************************************/
static void BDSP_Raaga_P_Dsp2TaskSyn_isr(
     void    *pDeviceHandle,
     int     iParm2
)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BDSP_RaagaTask *pRaagaTask;
	uint32_t    ui32SyncTaskIds = 0;
	uint32_t    ui32DspId = 0;
	int  i=0;

	BDBG_ENTER(BDSP_Raaga_P_Dsp2TaskSyn_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	ui32DspId = iParm2;
	BDBG_MSG(("... Sync Message Posted by DSP %d... ", ui32DspId));

	for(i  = 0; i < BDSP_RAAGA_MAX_FW_TASK_PER_DSP; i++)
	{
		pRaagaTask = (BDSP_RaagaTask *)pDevice->taskDetails[ui32DspId].pTask[i];
		if(pRaagaTask == NULL)
		{
			continue;
		}
		if (pRaagaTask->taskParams.taskId == BDSP_P_INVALID_TASK_ID)
		{
			continue;
		}
		ui32SyncTaskIds = BDSP_ReadReg32_isr(pDevice->regHandle,
					BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0 + pDevice->dspOffset[ui32DspId]);

		if (ui32SyncTaskIds & (1 << pRaagaTask->taskParams.taskId))
		{
			BDSP_WriteReg32_isr(pDevice->regHandle,
				BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 + pDevice->dspOffset[ui32DspId],
				ui32SyncTaskIds);
			if (pRaagaTask->taskParams.taskId < BDSP_RAAGA_MAX_FW_TASK_PER_DSP)
			{
				BKNI_SetEvent(pRaagaTask->hEvent);
			}
		}
	}

	BDBG_LEAVE(BDSP_Raaga_P_Dsp2TaskSyn_isr);
	return;
}

static void BDSP_Raaga_P_Dsp2HostGenResp_isr(
	  void	  *pDeviceHandle,
	  int	  iParm2
)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	uint32_t	 ui32DspId = 0;

	BDBG_ENTER(BDSP_Raaga_P_Dsp2HostGenResp_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	ui32DspId = iParm2;
	BDBG_MSG(("... Generic Response Message Posted by DSP %d... ", ui32DspId));

	BKNI_SetEvent(pDevice->hEvent[ui32DspId]);

	BDBG_LEAVE(BDSP_Raaga_P_Dsp2HostGenResp_isr);
	return;
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
static void BDSP_Raaga_P_Watchdog_isr(void *pDeviceHandle,  int iParm2)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BDSP_RaagaContext *pRaagaContext=NULL;
	unsigned dspIndex;

	BDBG_ENTER(BDSP_Raaga_P_Watchdog_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	BDBG_MSG(("Watchdog received from dspIndex = %d", iParm2));

	/* Mask all the external interrupts for this dsp */
	BDSP_WriteReg32_isr(pDevice->regHandle,
	 BCHP_RAAGA_DSP_ESR_SI_MASK_SET + pDevice->dspOffset[iParm2],
	 0xffffffff);

	for(dspIndex = 0; dspIndex < pDevice->numDsp; dspIndex++)
	{
		errCode = BDSP_Raaga_P_ResetHardware_isr(pDevice, dspIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Watchdog_isr: Error in Reset for DSP %d",dspIndex));
		}
	}

	pDevice->hardwareStatus.deviceWatchdogFlag = true;

	for(pRaagaContext = BLST_S_FIRST(&pDevice->contextList);
		pRaagaContext != NULL;
		pRaagaContext = BLST_S_NEXT(pRaagaContext, node))
	{
		pRaagaContext->contextWatchdogFlag = true;
		BDBG_MSG(("Calling Function callback for Context(type %d) handle 0x%p",pRaagaContext->settings.contextType,(void *)pRaagaContext));
		/* Call the application callback */
		if(pRaagaContext->interruptHandlers.watchdog.pCallback_isr)
		{
			pRaagaContext->interruptHandlers.watchdog.pCallback_isr(
				pRaagaContext->interruptHandlers.watchdog.pParam1,
				iParm2
			);
		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_Watchdog_isr);
}

static BERR_Code BDSP_Raaga_P_WatchdogRecoverySequence(
	BDSP_Raaga  *pDevice
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_WatchdogRecoverySequence);

	BDSP_Raaga_P_Open(pDevice);

	errCode = BDSP_Raaga_P_Boot(pDevice);
	if (errCode!=BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_WatchdogRecoverySequence: Error in Booting Raaga"));
		errCode= BERR_TRACE(errCode);
		goto end;
	}

	pDevice->hardwareStatus.deviceWatchdogFlag = false;

end:
	BDBG_LEAVE(BDSP_Raaga_P_WatchdogRecoverySequence);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_ProcessContextWatchdogInterrupt

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle provided by the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   On occurance of an watchdog, call the Raaga Open function.
                On completion, clear the watchdog flag of the Device handle and Context handle
***********************************************************************/
BERR_Code BDSP_Raaga_P_ProcessContextWatchdogInterrupt(
    void *pContextHandle
)
{
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga  *pDevice= pRaagaContext->pDevice;

	BDBG_ENTER(BDSP_Raaga_P_ProcessContextWatchdogInterrupt);
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BKNI_EnterCriticalSection();
	/* This recovery function should not be called in Interrupt Sequence, hence putting a condition check*/
    BKNI_LeaveCriticalSection();

    BDBG_MSG(("BDSP_Raaga_P_ProcessContextWatchdogInterrupt called for Context(type %d) %p",pRaagaContext->settings.contextType, pContextHandle));
	if(pDevice->hardwareStatus.deviceWatchdogFlag == true)
	{
		errCode = BDSP_Raaga_P_WatchdogRecoverySequence(pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_ProcessContextWatchdogInterrupt: Error in Watchdog recovery sequence"));
			goto end;
		}
	}
	pRaagaContext->contextWatchdogFlag = false;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ProcessContextWatchdogInterrupt);
	return errCode;
}

/***************************************************************************
Description:
    This API installs the top level interrupt handlers for all the interrups.
Returns:
    BERR_Code
See Also:
***************************************************************************/
BERR_Code BDSP_Raaga_P_TaskInterruptInstall (
    void    *pTaskHandle
)
{
	BERR_Code       errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	unsigned int    dspIndex = pRaagaTask->createSettings.dspIndex;
	BDSP_Raaga      *pDevice = pRaagaTask->pContext->pDevice;

	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	BDBG_ENTER (BDSP_Raaga_P_TaskInterruptInstall);

	/* We will create two callbacks for Asynchronous Interrupt & for
	Synchtonous interrupt on both DAP0 as well as on DS1 */
	errCode = BINT_CreateCallback(
	                        &pRaagaTask->interruptCallbacks.hDSPAsync,
	                        pDevice->intHandle,
	                        DSPAsynInterruptId[dspIndex],
	                        BDSP_Raaga_P_Dsp2TaskAsyn_isr,
	                        (void*)pDevice,
	                        dspIndex
	                        );
	if(errCode != BERR_SUCCESS )
	{
		BDBG_ERR(("Create Callback failed for DSP = %d INT_ID_ESR_SO1",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}
	errCode = BINT_EnableCallback(pRaagaTask->interruptCallbacks.hDSPAsync);
	if(errCode != BERR_SUCCESS )
	{
		BDBG_ERR(("Enable Callback failed for DSP = %d INT_ID_ESR_SO1",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}

	errCode = BINT_CreateCallback(
	                        &pRaagaTask->interruptCallbacks.hDSPSync,
	                        pDevice->intHandle,
	                        DSPSynInterruptId[dspIndex],
	                        BDSP_Raaga_P_Dsp2TaskSyn_isr,
	                        (void*)pDevice,
	                        dspIndex
	                        );
	if(errCode != BERR_SUCCESS )
	{
		BDBG_ERR(("Create Callback failed for DSP = %d BCHP_INT_ID_ESR_SO0",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}

	errCode = BINT_EnableCallback(pRaagaTask->interruptCallbacks.hDSPSync);
	if(errCode != BERR_SUCCESS )
	{
		BDBG_ERR(("Enable Callback failed for DSP = %d BCHP_INT_ID_ESR_SO0",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}

	goto end;
err_callback:
	if(errCode != BERR_SUCCESS )
	{
		BDSP_Raaga_P_TaskInterruptUninstall(pTaskHandle);
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_TaskInterruptInstall);
	return errCode;
}

/***************************************************************************
Description:
	This API uninstalls the top level interrupt handlers for all the interrups.
Returns:
	BERR_Code
See Also:
***************************************************************************/
BERR_Code BDSP_Raaga_P_TaskInterruptUninstall (
	void	*pTaskHandle
)
{
	BERR_Code		errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	BDBG_ENTER (BDSP_Raaga_P_TaskInterruptUninstall);

   /* We will create two callbacks for Asynchronous Interrupt & for
	   Synchtonous interrupt on both DAP0 as well as on DS1 */

   if(pRaagaTask->interruptCallbacks.hDSPAsync)
   {
		errCode = BINT_DisableCallback(pRaagaTask->interruptCallbacks.hDSPAsync);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_TaskInterruptUninstall: Error in Disabling Async Interrupt for Task"));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
		errCode = BINT_DestroyCallback(pRaagaTask->interruptCallbacks.hDSPAsync);
		if (errCode !=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_TaskInterruptUninstall: Error in Destroying Async Interrupt for Task"));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
   }

	if(pRaagaTask->interruptCallbacks.hDSPSync)
	{
		errCode = BINT_DisableCallback(pRaagaTask->interruptCallbacks.hDSPSync);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_TaskInterruptUninstall: Error in Disabling Sync Interrupt for Task"));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
		errCode = BINT_DestroyCallback(pRaagaTask->interruptCallbacks.hDSPSync);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_TaskInterruptUninstall: Error in Destroying Sync Interrupt for Task"));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
   }

end:
	BDBG_LEAVE(BDSP_Raaga_P_TaskInterruptUninstall);
	return errCode;
}

BERR_Code BDSP_Raaga_P_DeviceInterruptInstall (
	void		*pDeviceHandle,
	unsigned    dspIndex
)
{
	BERR_Code	errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	BDBG_ENTER (BDSP_Raaga_P_DeviceInterruptUninstall);

	errCode = BINT_CreateCallback(
	                        &pDevice->interruptCallbacks[dspIndex].hWatchdogCallback,
	                        pDevice->intHandle,
	                        DSPWatchDogInterruptId[dspIndex],
	                        BDSP_Raaga_P_Watchdog_isr,
	                        (void*)pDevice,
	                        dspIndex
	                        );
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Create Watchdog Callback failed for DSP = %d",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}
	errCode = BINT_EnableCallback(pDevice->interruptCallbacks[dspIndex].hWatchdogCallback);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Enable Watchdog Callback failed for DSP = %d",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}

	BDSP_Raaga_P_EnableDspWatchdogTimer(pDevice, dspIndex, true);
	pDevice->hardwareStatus.bWatchdogTimerStatus = true;

	errCode = BINT_CreateCallback(
	                        &pDevice->interruptCallbacks[dspIndex].hGenResp,
	                        pDevice->intHandle,
	                        DSPGenRespInterruptId[dspIndex],
	                        BDSP_Raaga_P_Dsp2HostGenResp_isr,
	                        (void*)pDevice,
	                        dspIndex
	                        );
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Create Generic Response Callback failed for DSP = %d",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}
	errCode = BINT_EnableCallback(pDevice->interruptCallbacks[dspIndex].hGenResp);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Enable Generic Response Callback failed for DSP = %d",dspIndex));
		errCode = BERR_TRACE(errCode);
		goto err_callback;
	}

	goto end;
err_callback:
	if(errCode != BERR_SUCCESS )
	{
		BDSP_Raaga_P_DeviceInterruptUninstall(pDeviceHandle, dspIndex);
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_DeviceInterruptInstall);
	return errCode;
}

/***************************************************************************
Description:
	This API uninstalls the top level interrupt handlers for all the interrups.
Returns:
	BERR_Code
See Also:
***************************************************************************/
BERR_Code BDSP_Raaga_P_DeviceInterruptUninstall (
	void		*pDeviceHandle,
	unsigned     dspIndex
)
{
	BERR_Code	errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	BDBG_ENTER (BDSP_Raaga_P_DeviceInterruptUninstall);

	if(pDevice->interruptCallbacks[dspIndex].hWatchdogCallback)
	{
		errCode = BINT_DisableCallback(pDevice->interruptCallbacks[dspIndex].hWatchdogCallback);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DeviceInterruptUninstall: Error in Disabling Watchdog Interrupt for Device %d", dspIndex));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
		errCode = BINT_DestroyCallback(pDevice->interruptCallbacks[dspIndex].hWatchdogCallback);
		if (errCode !=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DeviceInterruptUninstall: Error in Destroying Watchdog Interrupt for Device %d", dspIndex));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

	BDSP_Raaga_P_EnableDspWatchdogTimer(pDevice, dspIndex, false);
	pDevice->hardwareStatus.bWatchdogTimerStatus = false;

	if(pDevice->interruptCallbacks[dspIndex].hGenResp)
	{
		errCode = BINT_DisableCallback(pDevice->interruptCallbacks[dspIndex].hGenResp);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DeviceInterruptUninstall: Error in Disabling Gen Resp Interrupt for Device %d", dspIndex));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
		errCode = BINT_DestroyCallback(pDevice->interruptCallbacks[dspIndex].hGenResp);
		if (errCode !=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DeviceInterruptUninstall: Error in Destroying Gen Resp Interrupt for Device %d", dspIndex));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_DeviceInterruptUninstall);
	return errCode;
}

void BDSP_Raaga_P_GetTaskInterruptHandlers_isr(
    void *pTaskHandle,
    BDSP_AudioInterruptHandlers *pHandlers
)
{
    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Raaga_P_GetTaskInterruptHandlers_isr);
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    BDBG_ASSERT(pHandlers);

    *pHandlers = pRaagaTask->audioInterruptHandlers;

	BDBG_LEAVE(BDSP_Raaga_P_GetTaskInterruptHandlers_isr);
}

BERR_Code BDSP_Raaga_P_SetTaskInterruptHandlers_isr(
    void *pTaskHandle,
    const BDSP_AudioInterruptHandlers *pHandlers
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Raaga_P_SetTaskInterruptHandlers_isr);

	/*Sample Rate Change Interrupt*/
	if((pHandlers->sampleRateChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_SAMPLING_RATE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_SAMPLING_RATE_CHANGE);
	}

	/*Frame Sync Lock Interrupt*/
	if((pHandlers->lock.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_LOCK);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_LOCK);
	}

	/*Frame Sync UnLock Interrupt*/
	if((pHandlers->unlock.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_UNLOCK);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_UNLOCK);
	}

	/*CDB ITB  Overflow Interrupt*/
	if((pHandlers->cdbItbOverflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_OVERFLOW);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_OVERFLOW);
	}

	/*CDB ITB  Underflow Interrupt*/
	if((pHandlers->cdbItbUnderflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_UNDERFLOW);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_UNDERFLOW);
	}

	/*First PTS Interrupt*/
	if((pHandlers->firstPts.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FIRST_PTS_RECEIVED);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_FIRST_PTS_RECEIVED);
	}

	/*TSM FAIL Interrupt*/
	if((pHandlers->tsmFail.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_TSM_FAIL);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_TSM_FAIL);
	}

	/*TSM PASS Interrupt*/
	if((pHandlers->tsmPass.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ASTM_TSM_PASS);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ASTM_TSM_PASS);
	}

	/*ACMODE Interrupt*/
	if((pHandlers->modeChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ACMODE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ACMODE_CHANGE);
	}

	/*BIT RATE CHANGE Interrupt*/
	if((pHandlers->bitrateChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_BIT_RATE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_BIT_RATE_CHANGE);
	}

	/*STREAM INFO Interrupt*/
	if((pHandlers->statusReady.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_STREAM_INFO_AVAILABLE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_STREAM_INFO_AVAILABLE);
	}

	/*UNLICENSED INFO Interrupt*/
	if((pHandlers->unlicensedAlgo.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_UNLICENSED_ALGO);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_UNLICENSED_ALGO);
	}

	/*ANCILLARY INFO INFO Interrupt*/
	if((pHandlers->ancillaryData.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE);
	}

	/*DIAL NORM Interrupt*/
	if((pHandlers->dialnormChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_DIAL_NORM_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_DIAL_NORM_CHANGE);
	}

	/*VOLUME LEVEL REACHED Interrupt*/
	if((pHandlers->targetVolumeLevelReached.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_VOLUME_LEVEL_REACHED);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_VOLUME_LEVEL_REACHED);
	}

	/*ENCODER OVERFLOW Interrupt*/
	if((pHandlers->encoderOutputOverflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ENCODER_OVERFLOW_EVENT);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ENCODER_OVERFLOW_EVENT);
	}

	/*ON DEMAND AUDIO FRAME DELIVERED Interrupt*/
	if((pHandlers->onDemandAudioFrameDelivered.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ON_DEMAND_AUDIO_FRAME_DELIVERED);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_ON_DEMAND_AUDIO_FRAME_DELIVERED);
	}

	/*VIDEO ENCODER DISCARD Interrupt*/
	if((pHandlers->vencDataDiscarded.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_VIDEO_ENCODER_DATA_DISCARD);
	}
	else
	{
		BDSP_P_SET_BIT(pRaagaTask->taskParams.eventEnabledMask, BDSP_P_EventID_VIDEO_ENCODER_DATA_DISCARD);
	}

	if(pRaagaTask->taskParams.isRunning == true)
	{
		BDSP_P_EventEnableDisableCommand Payload;
		BKNI_Memset(&Payload, 0, sizeof(BDSP_P_EventEnableDisableCommand));
		Payload.ui32EnableEvent = pRaagaTask->taskParams.eventEnabledMask;

		errCode = BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr(pRaagaTask,&Payload);
	    if(errCode != BERR_SUCCESS)
	    {
	        BDBG_ERR(("BDSP_Raaga_P_SetTaskInterruptHandlers_isr: Send command failed"));
	        errCode = BERR_TRACE(errCode);
	        goto end;
	    }
	}
	pRaagaTask->audioInterruptHandlers = *pHandlers;

end:
	BDBG_LEAVE(BDSP_Raaga_P_SetTaskInterruptHandlers_isr);
	return errCode;
}

void BDSP_Raaga_P_GetContextInterruptHandlers(
    void *pContextHandle,
    BDSP_ContextInterruptHandlers *pInterrupts
)
{
    BDSP_RaagaContext   *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_ASSERT(pInterrupts);

    *pInterrupts = pRaagaContext->interruptHandlers;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_SetInterruptHandlers

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle of the task for which settings are required.
                pInterrupts     -   Handle of the Interrupt from the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
    1)  Store the interrupt handler provided by the PI as part of the Context handle.
***********************************************************************/
BERR_Code BDSP_Raaga_P_SetContextInterruptHandlers(
    void *pContextHandle,
    const BDSP_ContextInterruptHandlers *pInterrupts
)
{
    BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
    BDBG_ASSERT(pInterrupts);

    if(pInterrupts->watchdog.pCallback_isr != NULL)
    {
       /* Do nothing, Enable and Disbale of Watchdog has been moved to Device interrupt install*/
	   pRaagaContext->interruptHandlers = *pInterrupts;
	}

    return errCode;
}

/**********************************************************
Function: BDSP_Raaga_P_RestoreInterrupts_isr

Description :	 Isr version of  BDSP_Raaga_P_RestoreInterrupts
		  Restores the interrupts to their state
		  before reset

**********************************************************/
BERR_Code BDSP_Raaga_P_RestoreInterrupts_isr(
	void		*pDeviceHandle,
	uint32_t	 uiDspIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	uint32_t L2Reg ;

	BDBG_ENTER(BDSP_Raaga_P_RestoreInterrupts_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	/* Dsp Sync and Async interrupt registers use the same L2 register so using only one
	of the name of the interrupts to derive the L2 register name*/

	L2Reg = BCHP_INT_ID_GET_REG(DSPSynInterruptId[uiDspIndex]);
	BINT_ApplyL2State_isr(pDevice->intHandle,L2Reg );

	L2Reg = BCHP_INT_ID_GET_REG(DSPWatchDogInterruptId[uiDspIndex]);
	BINT_ApplyL2State_isr(pDevice->intHandle,L2Reg );

	BDBG_LEAVE(BDSP_Raaga_P_RestoreInterrupts_isr);
	return errCode;
}
