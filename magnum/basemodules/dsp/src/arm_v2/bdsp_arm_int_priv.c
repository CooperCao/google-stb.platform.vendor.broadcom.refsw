/******************************************************************************
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
 *****************************************************************************/

#include "bdsp_arm_priv_include.h"

BDBG_MODULE(bdsp_arm_int);   /* Register software module with debug interface */

static void  BDSP_Arm_P_FrameSyncUnlock_isr(
	void *pTaskHandle)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

   BDBG_MSG(("BDSP_Arm_P_FrameSyncUnlock_isr: FrameSync unlock interrupt occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application FrameSync Unlock callback function */
	if(pArmTask->audioInterruptHandlers.unlock.pCallback_isr) {
		pArmTask->audioInterruptHandlers.unlock.pCallback_isr (
			pArmTask->audioInterruptHandlers.unlock.pParam1,
			pArmTask->audioInterruptHandlers.unlock.param2);
	}
}

static void  BDSP_Arm_P_FrameSyncLock_isr(
	void *pTaskHandle)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

   BDBG_MSG(("BDSP_Arm_P_FrameSyncLock_isr: FrameSync lock interrupt occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application FrameSync Lock callback function */
	if(pArmTask->audioInterruptHandlers.lock.pCallback_isr) {
		pArmTask->audioInterruptHandlers.lock.pCallback_isr (
			pArmTask->audioInterruptHandlers.lock.pParam1,
			pArmTask->audioInterruptHandlers.lock.param2);
	}
}

static void  BDSP_Arm_P_SampleRateChange_isr(
	BDSP_ArmTask *pArmTask,
	BDSP_P_SampleRateChangeInfo *pSampleRateChangeInfo

)
{
	BDBG_ENTER(BDSP_Arm_P_SampleRateChange_isr);
	BDBG_MSG(("BDSP_Arm_P_SampleRateChange_isr: Received Sample Rate Change for Task (%d)", pArmTask->taskParams.taskId));
	BDBG_MSG(("BaseSampleRate = %d \t StreamSampleRate = %d",pSampleRateChangeInfo->ui32BaseSampleRate,pSampleRateChangeInfo->ui32StreamSampleRate));
	if(pArmTask->audioInterruptHandlers.sampleRateChange.pCallback_isr)
	{
		pArmTask->audioInterruptHandlers.sampleRateChange.pCallback_isr (
					pArmTask->audioInterruptHandlers.sampleRateChange.pParam1,
					pArmTask->audioInterruptHandlers.sampleRateChange.param2,
					pSampleRateChangeInfo->ui32StreamSampleRate,
					pSampleRateChangeInfo->ui32BaseSampleRate);
	}
	BDBG_LEAVE(BDSP_Arm_P_SampleRateChange_isr);
}

static void BDSP_Arm_P_CdbItbOverflow_isr(
	void *pTaskHandle)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
   BDBG_MSG(("BDSP_Arm_P_CdbItbOverflow_isr: cdbItb Overflow event occured for Task %d",pArmTask->taskParams.taskId));
	/* Call the application CDB/ITB Overflow callback function */
	if(pArmTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr) {
		pArmTask->audioInterruptHandlers.cdbItbOverflow.pCallback_isr (
			pArmTask->audioInterruptHandlers.cdbItbOverflow.pParam1,
			pArmTask->audioInterruptHandlers.cdbItbOverflow.param2);
	}
}

static void BDSP_Arm_P_CdbItbUnderflow_isr(
	void *pTaskHandle )
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
   BDBG_MSG(("BDSP_Arm_P_CdbItbUnderflow_isr: cdbItb Underflow event occured for Task %d",pArmTask->taskParams.taskId));
	/* Call the application CDB/ITB underflow callback function */
	if(pArmTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr) {
		pArmTask->audioInterruptHandlers.cdbItbUnderflow.pCallback_isr (
			pArmTask->audioInterruptHandlers.cdbItbUnderflow.pParam1,
			pArmTask->audioInterruptHandlers.cdbItbUnderflow.param2);
	}
}

static void  BDSP_Arm_P_FirstPtsReceived_isr(
	void *pTaskHandle , BDSP_P_PtsInfo *pPtsInfo)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDSP_AudioTaskTsmStatus ptsInfo;


   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
   ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
   ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
   ptsInfo.ePtsType = pPtsInfo->ePtsType;
   ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

   BDBG_MSG(("BDSP_Arm_P_FirstPtsReceived_isr: First PTS received event occured for Task %d",pArmTask->taskParams.taskId));
   BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

   /* Call the application first pts received callback function */
   if(pArmTask->audioInterruptHandlers.firstPts.pCallback_isr) {
	pArmTask->audioInterruptHandlers.firstPts.pCallback_isr (
		pArmTask->audioInterruptHandlers.firstPts.pParam1,
		pArmTask->audioInterruptHandlers.firstPts.param2,
		&ptsInfo);
   }
}

static void  BDSP_Arm_P_TsmFail_isr(
     void *pTaskHandle , BDSP_P_PtsInfo*pPtsInfo)
{

    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Arm_P_TsmFail_isr: TSm Fail event occured for Task %d", pArmTask->taskParams.taskId));
	BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

    /* Call the application TSM fail callback function */
    if(pArmTask->audioInterruptHandlers.tsmFail.pCallback_isr) {
     pArmTask->audioInterruptHandlers.tsmFail.pCallback_isr (
         pArmTask->audioInterruptHandlers.tsmFail.pParam1,
         pArmTask->audioInterruptHandlers.tsmFail.param2,
         &ptsInfo);
    }
}

static void  BDSP_Arm_P_AstmPass_isr(
     void *pTaskHandle , BDSP_P_PtsInfo *pPtsInfo)
{

    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_AudioTaskTsmStatus  ptsInfo;

    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    ptsInfo.ui32RunningPts = pPtsInfo->ui32RunningPts;
    ptsInfo.i32PtsToStcPhase = pPtsInfo->i32Pts2StcPhase;
    ptsInfo.ePtsType = pPtsInfo->ePtsType;
    ptsInfo.i32FrameSizeIn45khzTicks = pPtsInfo->i32TSMUpperThreshold;

    BDBG_MSG(("BDSP_Arm_P_AstmPass_isr: ASTM Pass event occured for Task %d",pArmTask->taskParams.taskId));
	BDBG_MSG(("RunningPts = 0x%x \t Pts2StcPhase = %d \t PtsType = %d \t TSMUpperThreshold = 0x%x",pPtsInfo->ui32RunningPts,pPtsInfo->i32Pts2StcPhase,pPtsInfo->ePtsType,pPtsInfo->i32TSMUpperThreshold));

    /* Call the application ASTM TSM Pass callback function */
    if(pArmTask->audioInterruptHandlers.tsmPass.pCallback_isr) {
     pArmTask->audioInterruptHandlers.tsmPass.pCallback_isr (
         pArmTask->audioInterruptHandlers.tsmPass.pParam1,
         pArmTask->audioInterruptHandlers.tsmPass.param2,
         &ptsInfo);
    }
}

static void  BDSP_Arm_P_AcmodeChange_isr(
	void *pTaskHandle , BDSP_P_AcmodeChangeInfo *pAcmodeChangeInfo)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   uint32_t ui32ModeValue;

   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

   BDBG_MSG(("BDSP_Arm_P_AcmodeChange_isr: mode change event occured for Task %d",pArmTask->taskParams.taskId));
   BDBG_MSG(("New ModeValue = %d",pAcmodeChangeInfo->ui32ModeValue));


   /*Prepare the structure for application */
   ui32ModeValue = pAcmodeChangeInfo->ui32ModeValue;

  /* Call the application Acmode change callback function */
  if(pArmTask->audioInterruptHandlers.modeChange.pCallback_isr) {
	pArmTask->audioInterruptHandlers.modeChange.pCallback_isr (
		pArmTask->audioInterruptHandlers.modeChange.pParam1,
		pArmTask->audioInterruptHandlers.modeChange.param2,
		ui32ModeValue);
  }

}

static void  BDSP_Arm_P_BitrateChange_isr(
	void *pTaskHandle , BDSP_P_BitrateChangeInfo *pBitrateChangeInfo)
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDSP_ArmStage *pArmPrimaryStage;
   BDSP_AudioBitRateChangeInfo	bitrateChangeInfo;

   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
   pArmPrimaryStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
   BDBG_OBJECT_ASSERT(pArmPrimaryStage, BDSP_ArmStage);

   BDBG_MSG(("BDSP_Arm_P_BitRateChange_isr: Bit rate change event occured for Task %d",pArmTask->taskParams.taskId));
   BDBG_MSG(("Algorithm = %d \t BitRate = %d \t BitRateIndex = %d ",pArmPrimaryStage->eAlgorithm,pBitrateChangeInfo->ui32BitRate, pBitrateChangeInfo->ui32BitRateIndex));

	/*Prepare the structure for application */
	bitrateChangeInfo.eAlgorithm = pArmPrimaryStage->eAlgorithm;
	bitrateChangeInfo.ui32BitRate = pBitrateChangeInfo->ui32BitRate;
	bitrateChangeInfo.ui32BitRateIndex = pBitrateChangeInfo->ui32BitRateIndex;


	/* Call the application Bitrate change callback function */
	if(pArmTask->audioInterruptHandlers.bitrateChange.pCallback_isr) {
		pArmTask->audioInterruptHandlers.bitrateChange.pCallback_isr (
			pArmTask->audioInterruptHandlers.bitrateChange.pParam1,
			pArmTask->audioInterruptHandlers.bitrateChange.param2,
			&bitrateChangeInfo);
	}

}

static void  BDSP_Arm_P_StreamInfoAvailable_isr(
	 void *pTaskHandle )
{

	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	BDBG_MSG(("BDSP_Arm_P_StreamInfoAvailable_isr: Stream Information available event occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application Stream Info Available  callback function */
	if(pArmTask->audioInterruptHandlers.statusReady.pCallback_isr) {
	 pArmTask->audioInterruptHandlers.statusReady.pCallback_isr (
		 pArmTask->audioInterruptHandlers.statusReady.pParam1,
		 pArmTask->audioInterruptHandlers.statusReady.param2);
	}
}

static void  BDSP_Arm_P_UnlicensedAlgo_isr(
	 void *pTaskHandle,
	 BDSP_P_UnlicensedAlgoInfo *pUnlicensedAlgoInfo)
{
	 BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

	 BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	 BDBG_ERR(("BDSP_Arm_P_UnlicensedAlgo_isr: Unlicensed Algo (%u) event occured for Task %d", pUnlicensedAlgoInfo->ui32AudioAlgorithm,pArmTask->taskParams.taskId));

	 /* Call the application UnlicensedAlgo	callback function */
	 if(pArmTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr) {
		 pArmTask->audioInterruptHandlers.unlicensedAlgo.pCallback_isr (
		 pArmTask->audioInterruptHandlers.unlicensedAlgo.pParam1,
		 pArmTask->audioInterruptHandlers.unlicensedAlgo.param2);
	 }
}

static void BDSP_Arm_P_AncillaryDataAvailable_isr(
	void *pTaskHandle )
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

   BDBG_MSG(("BDSP_Arm_P_AncillaryDataAvailable_isr: MPEG Ancillary Data event occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application Ancillary data present callback function */
	if(pArmTask->audioInterruptHandlers.ancillaryData.pCallback_isr) {
		pArmTask->audioInterruptHandlers.ancillaryData.pCallback_isr (
			pArmTask->audioInterruptHandlers.ancillaryData.pParam1,
			pArmTask->audioInterruptHandlers.ancillaryData.param2);
   }

}

static void BDSP_Arm_P_DialnormChange_isr(
	void *pTaskHandle )
{

   BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
   BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);


   BDBG_MSG(("BDSP_Arm_P_DialnormChange_isr: Change in Dialnorm event occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application Dialnorm change callback function */
	if(pArmTask->audioInterruptHandlers.dialnormChange.pCallback_isr) {
		pArmTask->audioInterruptHandlers.dialnormChange.pCallback_isr (
			pArmTask->audioInterruptHandlers.dialnormChange.pParam1,
			pArmTask->audioInterruptHandlers.dialnormChange.param2);
   }

}

static void  BDSP_Arm_P_VolumeLevelReached_isr(
     void *pTaskHandle)
{

    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    BDBG_MSG(("BDSP_Arm_P_VolumeLevelReached_isr: Volume level reached event occured for Task %d",pArmTask->taskParams.taskId));

	/* Call the application volume level reached callback function */
    if(pArmTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr) {
         pArmTask->audioInterruptHandlers.targetVolumeLevelReached.pCallback_isr (
             pArmTask->audioInterruptHandlers.targetVolumeLevelReached.pParam1,
             pArmTask->audioInterruptHandlers.targetVolumeLevelReached.param2);
    }
}

static void BDSP_Arm_P_Dsp2TaskAsyn_isr(
    BDSP_Arm     *pDevice,
    BDSP_ArmTask *pArmTask
)
{
    BDSP_P_AsynMsg  *pEventMsg = NULL;
    unsigned uiNumMsgs = 0, i = 0;

	BDBG_ENTER(BDSP_Arm_P_Dsp2TaskAsyn_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	BDBG_MSG(("ASYNC MSG Received from Task %d", pArmTask->taskParams.taskId));
    pEventMsg = (BDSP_P_AsynMsg *)pArmTask->taskMemInfo.hostAsyncQueue.pAddr;

    /* Read the message in sEventMsg */
    BDSP_Arm_P_GetAsyncMsg_isr(pArmTask->hAsyncQueue, pEventMsg, &uiNumMsgs);

    /* Now check which response came from FW and work according to it*/
    for(i = 0; i < uiNumMsgs; i++)
    {
        if(pEventMsg[i].sAsynHeader.eEventID== BDSP_P_EventID_CDB_ITB_UNDERFLOW)
        {
           BDSP_Arm_P_CdbItbUnderflow_isr((void *)pArmTask);
        }
        else if(pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_CDB_ITB_OVERFLOW)
        {
           BDSP_Arm_P_CdbItbOverflow_isr((void *)pArmTask);
        }
        else if(pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_SAMPLING_RATE_CHANGE)
        {
           BDSP_Arm_P_SampleRateChange_isr((void *)pArmTask,&(pEventMsg[i].uInfo.sSampleRateChangeInfo));
        }
        else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FRAME_SYNC_LOCK)
        {
           BDSP_Arm_P_FrameSyncLock_isr((void *)pArmTask);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FRAME_SYNC_UNLOCK)
        {
           BDSP_Arm_P_FrameSyncUnlock_isr((void *)pArmTask);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_FIRST_PTS_RECEIVED)
        {
           BDSP_Arm_P_FirstPtsReceived_isr((void *)pArmTask, &pEventMsg[i].uInfo.sPtsInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID == BDSP_P_EventID_TSM_FAIL)
        {
           BDSP_Arm_P_TsmFail_isr((void *)pArmTask, &pEventMsg[i].uInfo.sPtsInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ASTM_TSM_PASS)
        {
           BDSP_Arm_P_AstmPass_isr((void *)pArmTask, &pEventMsg[i].uInfo.sPtsInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ACMODE_CHANGE)
        {
           BDSP_Arm_P_AcmodeChange_isr((void *)pArmTask, &pEventMsg[i].uInfo.sAcmodeChangeInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_BIT_RATE_CHANGE)
        {
           BDSP_Arm_P_BitrateChange_isr((void *)pArmTask, &pEventMsg[i].uInfo.sBitrateChangeInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_STREAM_INFO_AVAILABLE)
        {
           BDSP_Arm_P_StreamInfoAvailable_isr((void *)pArmTask);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_UNLICENSED_ALGO)
        {
           BDSP_Arm_P_UnlicensedAlgo_isr((void *)pArmTask, &pEventMsg[i].uInfo.sUnlicensedAlgoInfo);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE)
        {
           BDSP_Arm_P_AncillaryDataAvailable_isr((void *)pArmTask);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_DIAL_NORM_CHANGE)
        {
           BDSP_Arm_P_DialnormChange_isr((void *)pArmTask);
        }
        else if (pEventMsg[i].sAsynHeader.eEventID  == BDSP_P_EventID_VOLUME_LEVEL_REACHED)
        {
           BDSP_Arm_P_VolumeLevelReached_isr((void *)pArmTask);
        }
        else
        {
           BDBG_WRN (("BDSP_Arm_P_DSP2HostAsyn_isr :: <<< Async Message is Not Valid or Not Supported. EventID >>> %d",
               pEventMsg[i].sAsynHeader.eEventID));
        }
    }

	BDBG_LEAVE(BDSP_Arm_P_Dsp2TaskAsyn_isr);
}

static void BDSP_Arm_P_Dsp2TaskSyn_isr(
    BDSP_Arm     *pDevice,
    BDSP_ArmTask *pArmTask
)
{

	BDBG_ENTER(BDSP_Arm_P_Dsp2TaskSyn_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	BDBG_MSG(("SYNC MSG Received from Task %d", pArmTask->taskParams.taskId));

    BKNI_SetEvent(pArmTask->hEvent);

	BDBG_LEAVE(BDSP_Arm_P_Dsp2TaskSyn_isr);
}

static void BDSP_Arm_P_Dsp2HostGenResp_isr(
    BDSP_Arm     *pDevice,
    unsigned      dspIndex
)
{
	BDBG_ENTER(BDSP_Arm_P_Dsp2HostGenResp_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	BDBG_MSG(("... Generic Response Message Posted by DSP %d... ", dspIndex));

	BKNI_SetEvent(pDevice->hEvent[dspIndex]);

	BDBG_LEAVE(BDSP_Arm_P_Dsp2HostGenResp_isr);
	return;
}

void BDSP_Arm_P_AstraEventCallback_isr(
    BTEE_ClientEvent event,
    void            *pDeviceHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmDspAck sAckInfo;
    BDSP_ArmTask *pArmTask = NULL;
    uint32_t    ui32TaskId = 0;
	uint32_t    ui32DeviceId = 0;
    size_t ui32RcvMsgLen = 0;
    BTEE_ConnectionHandle hConnection = NULL;

    BDBG_ENTER(BDSP_Arm_P_AstraEventCallback_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	BSTD_UNUSED(event);
	BDBG_MSG(("BDSP_Arm_P_AstraEventCallback_isr: EventCallback received"));

	errCode = BTEE_Client_ReceiveMessage(
		pDevice->armDspApp.hClient, 		 /* Client Handle */
		&hConnection, /* Connection that originated the message */
		(void *)&sAckInfo,					   /* Pointer to buffer for received message */
		sizeof(BDSP_ArmDspAck),			 /* Length of message buffer in bytes */
		&ui32RcvMsgLen,			   /* Returned message length in bytes */
		0					       /* Timeout in msec.  Pass 0 for no timeout. */
		);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: Failed to receive msg"));
		goto end;
	}
	/* Check if the msg is intentended from armdsp */
	if(hConnection != pDevice->armDspApp.hConnection)
	{
		BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: Received Unknown msg (handl=%p(hA=%p) AckType=%d, params=%d",
			(void*)hConnection,(void*)pDevice->armDspApp.hConnection,sAckInfo.eAckType,
			((BDSP_ArmDspAck_Type_eDevice == sAckInfo.eAckType)?sAckInfo.ui32DspIndex:sAckInfo.ui32TaskID)));
	}
	if(ui32RcvMsgLen != sizeof(BDSP_ArmDspAck))
	{
		BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: Mismatch in size of msg recieved(%d) and expected size (%d)", (unsigned int)ui32RcvMsgLen, (unsigned int)sizeof(BDSP_ArmDspAck)));
		goto end;
	}

	if(BDSP_ArmDspAck_Type_eDevice == sAckInfo.eAckType)
	{
	    BDSP_Arm_P_Dsp2HostGenResp_isr(pDevice,sAckInfo.ui32DspIndex);
	}
	else if(BDSP_ArmDspAck_Type_eTask == sAckInfo.eAckType)
	{
		ui32TaskId = sAckInfo.ui32TaskID;
		ui32DeviceId = sAckInfo.ui32DspIndex;
		pArmTask = pDevice->taskDetails[ui32DeviceId].pTask[ui32TaskId];
		if(NULL == pArmTask)
		{
			BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: something went wrong and Event callback was called for someother task"));
			goto end;
		}
		if(BDSP_MAX_FW_TASK_PER_DSP <= ui32TaskId)
		{
			BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: TaskIndex is %d which is greater than the max allowed index %d ",ui32TaskId,(BDSP_MAX_FW_TASK_PER_DSP-1)));
			goto end;
		}
		/* Handling Response to command */
		if(BDSP_ArmDspResp_Type_eCmdAck == sAckInfo.ui32RespType)
		{
			BDSP_Arm_P_Dsp2TaskSyn_isr(pDevice, pArmTask);
		}
		/* Handling of Async events */
		else if(BDSP_ArmDspResp_Type_eEvent == sAckInfo.ui32RespType)
		{
			/* Async msg handler */
			BDSP_Arm_P_Dsp2TaskAsyn_isr(pDevice, pArmTask);
		}
	}
	else
	{
		BDBG_ERR(("BDSP_Arm_P_AstraEventCallback_isr: ACK received from ArmDsp with wrong Ack type (%d)",sAckInfo.eAckType));
		goto end;
	}

end:
    BDBG_LEAVE(BDSP_Arm_P_AstraEventCallback_isr);
}

#if 0
static void BDSP_Arm_P_Watchdog_isr(
    void *pDeviceHandle,
    int   iParm2
)
{
	BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
	BDSP_ArmContext *pArmContext=NULL;
	unsigned dspIndex;

	BDBG_ENTER(BDSP_Arm_P_Watchdog_isr);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	BDBG_MSG(("Watchdog received from dspIndex = %d", iParm2));
    BDBG_ERR(("Something similar to Reset hardware has to be implemented here"));
    BSTD_UNUSED(dspIndex);

	pDevice->hardwareStatus.deviceWatchdogFlag = true;

	for(pArmContext = BLST_S_FIRST(&pDevice->contextList);
		pArmContext != NULL;
		pArmContext = BLST_S_NEXT(pArmContext, node))
	{
		pArmContext->contextWatchdogFlag = true;
		BDBG_MSG(("Calling Function callback for Context(type %d) handle 0x%p",pArmContext->settings.contextType,(void *)pArmContext));
		/* Call the application callback */
		if(pArmContext->interruptHandlers.watchdog.pCallback_isr)
		{
			pArmContext->interruptHandlers.watchdog.pCallback_isr(
				pArmContext->interruptHandlers.watchdog.pParam1,
				iParm2
			);
		}
	}
	BDBG_LEAVE(BDSP_Arm_P_Watchdog_isr);
}
#endif

static BERR_Code BDSP_Arm_P_WatchdogRecoverySequence(
	BDSP_Arm  *pDevice
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Arm_P_WatchdogRecoverySequence);

	errCode = BDSP_Arm_P_Open(pDevice);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_WatchdogRecoverySequence: Error in Opening and processing Open for Arm"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_OpenUserApp(pDevice);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_WatchdogRecoverySequence: Unable to Open the Arm User App"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_CheckDspAlive(pDevice);
	if (errCode!=BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_WatchdogRecoverySequence:  DSP not alive"));
		errCode= BERR_TRACE(errCode);
		goto end;
	}

	pDevice->hardwareStatus.deviceWatchdogFlag = false;

end:
	BDBG_LEAVE(BDSP_Arm_P_WatchdogRecoverySequence);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_ProcessContextWatchdogInterrupt

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle provided by the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   On occurance of an watchdog, call the Arm Open function.
                On completion, clear the watchdog flag of the Device handle and Context handle
***********************************************************************/
BERR_Code BDSP_Arm_P_ProcessContextWatchdogInterrupt(
    void *pContextHandle
)
{
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Arm  *pDevice;

	BDBG_ENTER(BDSP_Arm_P_ProcessContextWatchdogInterrupt);
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    pDevice= (BDSP_Arm  *)pArmContext->pDevice;
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_EnterCriticalSection();
	/* This recovery function should not be called in Interrupt Sequence, hence putting a condition check*/
    BKNI_LeaveCriticalSection();

    BDBG_MSG(("BDSP_Arm_P_ProcessContextWatchdogInterrupt called for Context(type %d) %p",pArmContext->settings.contextType, pContextHandle));
	if(pDevice->hardwareStatus.deviceWatchdogFlag == true)
	{
		errCode = BDSP_Arm_P_WatchdogRecoverySequence(pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_ProcessContextWatchdogInterrupt: Error in Watchdog recovery sequence"));
			goto end;
		}
	}
	pArmContext->contextWatchdogFlag = false;

end:
	BDBG_LEAVE(BDSP_Arm_P_ProcessContextWatchdogInterrupt);
	return errCode;
}

BERR_Code BDSP_Arm_P_DeviceInterruptInstall (
	void		*pDeviceHandle,
	unsigned    dspIndex
)
{
	BERR_Code	errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	BDBG_ENTER (BDSP_Arm_P_DeviceInterruptUninstall);

   /* Kept for future Implementation if required */
   BSTD_UNUSED(dspIndex);

	BDBG_LEAVE(BDSP_Arm_P_DeviceInterruptInstall);
	return errCode;
}

/***************************************************************************
Description:
	This API uninstalls the top level interrupt handlers for all the interrups.
Returns:
	BERR_Code
See Also:
***************************************************************************/
BERR_Code BDSP_Arm_P_DeviceInterruptUninstall (
	void		*pDeviceHandle,
	unsigned     dspIndex
)
{
	BERR_Code	errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	BDBG_ENTER (BDSP_Arm_P_DeviceInterruptUninstall);

    /* Kept for future Implementation if required */
    BSTD_UNUSED(dspIndex);
	BDBG_LEAVE(BDSP_Arm_P_DeviceInterruptUninstall);
	return errCode;
}

void BDSP_Arm_P_GetTaskInterruptHandlers_isr(
    void *pTaskHandle,
    BDSP_AudioInterruptHandlers *pHandlers
)
{
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Arm_P_GetTaskInterruptHandlers_isr);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
    BDBG_ASSERT(pHandlers);

    *pHandlers = pArmTask->audioInterruptHandlers;

	BDBG_LEAVE(BDSP_Arm_P_GetTaskInterruptHandlers_isr);
}

BERR_Code BDSP_Arm_P_SetTaskInterruptHandlers_isr(
    void *pTaskHandle,
    const BDSP_AudioInterruptHandlers *pHandlers
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Arm_P_SetTaskInterruptHandlers_isr);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	/*Sample Rate Change Interrupt*/
	if((pHandlers->sampleRateChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_SAMPLING_RATE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_SAMPLING_RATE_CHANGE);
	}

	/*Frame Sync Lock Interrupt*/
	if((pHandlers->lock.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_LOCK);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_LOCK);
	}

	/*Frame Sync UnLock Interrupt*/
	if((pHandlers->unlock.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_UNLOCK);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FRAME_SYNC_UNLOCK);
	}

	/*CDB ITB  Overflow Interrupt*/
	if((pHandlers->cdbItbOverflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_OVERFLOW);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_OVERFLOW);
	}

	/*CDB ITB  Underflow Interrupt*/
	if((pHandlers->cdbItbUnderflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_UNDERFLOW);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_CDB_ITB_UNDERFLOW);
	}

	/*First PTS Interrupt*/
	if((pHandlers->firstPts.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FIRST_PTS_RECEIVED);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_FIRST_PTS_RECEIVED);
	}

	/*TSM FAIL Interrupt*/
	if((pHandlers->tsmFail.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_TSM_FAIL);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_TSM_FAIL);
	}

	/*TSM PASS Interrupt*/
	if((pHandlers->tsmPass.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ASTM_TSM_PASS);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ASTM_TSM_PASS);
	}

	/*ACMODE Interrupt*/
	if((pHandlers->modeChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ACMODE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ACMODE_CHANGE);
	}

	/*BIT RATE CHANGE Interrupt*/
	if((pHandlers->bitrateChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_BIT_RATE_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_BIT_RATE_CHANGE);
	}

	/*STREAM INFO Interrupt*/
	if((pHandlers->statusReady.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_STREAM_INFO_AVAILABLE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_STREAM_INFO_AVAILABLE);
	}

	/*UNLICENSED INFO Interrupt*/
	if((pHandlers->unlicensedAlgo.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_UNLICENSED_ALGO);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_UNLICENSED_ALGO);
	}

	/*ANCILLARY INFO INFO Interrupt*/
	if((pHandlers->ancillaryData.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE);
	}

	/*DIAL NORM Interrupt*/
	if((pHandlers->dialnormChange.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_DIAL_NORM_CHANGE);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_DIAL_NORM_CHANGE);
	}

	/*VOLUME LEVEL REACHED Interrupt*/
	if((pHandlers->targetVolumeLevelReached.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_VOLUME_LEVEL_REACHED);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_VOLUME_LEVEL_REACHED);
	}

	/*ENCODER OVERFLOW Interrupt*/
	if((pHandlers->encoderOutputOverflow.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ENCODER_OVERFLOW_EVENT);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ENCODER_OVERFLOW_EVENT);
	}

	/*ON DEMAND AUDIO FRAME DELIVERED Interrupt*/
	if((pHandlers->onDemandAudioFrameDelivered.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ON_DEMAND_AUDIO_FRAME_DELIVERED);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_ON_DEMAND_AUDIO_FRAME_DELIVERED);
	}

	/*VIDEO ENCODER DISCARD Interrupt*/
	if((pHandlers->vencDataDiscarded.pCallback_isr == NULL))
	{
		BDSP_P_CLEAR_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_VIDEO_ENCODER_DATA_DISCARD);
	}
	else
	{
		BDSP_P_SET_BIT(pArmTask->taskParams.eventEnabledMask, BDSP_P_EventID_VIDEO_ENCODER_DATA_DISCARD);
	}

	if(pArmTask->taskParams.isRunning == true)
	{
		BDSP_P_EventEnableDisableCommand Payload;
		BKNI_Memset(&Payload, 0, sizeof(BDSP_P_EventEnableDisableCommand));
		Payload.ui32EnableEvent = pArmTask->taskParams.eventEnabledMask;

		errCode = BDSP_Arm_P_ProcessEventEnableDisableCommand_isr(pArmTask,&Payload);
	    if(errCode != BERR_SUCCESS)
	    {
	        BDBG_ERR(("BDSP_Arm_P_SetTaskInterruptHandlers_isr: Send command failed"));
	        errCode = BERR_TRACE(errCode);
	        goto end;
	    }
	}
	pArmTask->audioInterruptHandlers = *pHandlers;

end:
	BDBG_LEAVE(BDSP_Arm_P_SetTaskInterruptHandlers_isr);
	return errCode;
}

void BDSP_Arm_P_GetContextInterruptHandlers(
    void *pContextHandle,
    BDSP_ContextInterruptHandlers *pInterrupts
)
{
    BDSP_ArmContext   *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    BDBG_ASSERT(pInterrupts);

    *pInterrupts = pArmContext->interruptHandlers;
}

/***********************************************************************
Name        :   BDSP_Arm_P_SetInterruptHandlers

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle of the task for which settings are required.
                pInterrupts     -   Handle of the Interrupt from the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
    1)  Store the interrupt handler provided by the PI as part of the Context handle.
***********************************************************************/
BERR_Code BDSP_Arm_P_SetContextInterruptHandlers(
    void *pContextHandle,
    const BDSP_ContextInterruptHandlers *pInterrupts
)
{
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    BDBG_ASSERT(pInterrupts);

    if(pInterrupts->watchdog.pCallback_isr != NULL)
    {
       /* Do nothing, Enable and Disbale of Watchdog has been moved to Device interrupt install*/
	   pArmContext->interruptHandlers = *pInterrupts;
	}

    return errCode;
}
