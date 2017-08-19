/******************************************************************************
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
 *****************************************************************************/


 #include "bdsp_arm_priv_include.h"


 BDBG_MODULE(bdsp_arm_int);   /* Register software module with debug interface */

#if 0  /*SYNCASYNC_MSG CDN TBD*/
static const    BINT_Id ui32ARMDSPSynInterruptId[] =
{
#if defined BCHP_INT_ID_SYNC_MSG
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

static const    BINT_Id ui32ARMDSPAsynInterruptId[] =
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
#endif  /*SYNCASYNC_MSG CDN TBD*/

void BDSP_Arm_P_AckEventCallback_isr(BTEE_ClientEvent event, void *pDeviceHandle)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmTask *pArmTask = NULL;
    int uiTaskIndex = 0;
	BDSP_Arm_P_AckInfo sAckInfo;
	BERR_Code ret = BERR_SUCCESS;
	size_t ui32RcvMsgLen = 0;
	BTEE_ConnectionHandle hConnection = NULL;

	BDBG_ENTER(BDSP_Arm_P_AckEventCallback_isr);

	BSTD_UNUSED(event);

	/*Receive ack msg from Astra */
	ret = BTEE_Client_ReceiveMessage(
		pDevice->armDspApp.hClient, 		 /* Client Handle */
		&hConnection, /* Connection that originated the message */
		(void *)&sAckInfo,					   /* Pointer to buffer for received message */
		sizeof(BDSP_Arm_P_AckInfo),			 /* Length of message buffer in bytes */
		&ui32RcvMsgLen,			   /* Returned message length in bytes */
		0					       /* Timeout in msec.  Pass 0 for no timeout. */
		);

	if (BERR_SUCCESS != ret)
	{
		BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: Failed to receive msg"));
		goto end;
	}

	if(ui32RcvMsgLen != sizeof(BDSP_Arm_P_AckInfo))
	{
		BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: Mismatch in size of msg recieved(%d) and expected size (%d)", (unsigned int)ui32RcvMsgLen, (unsigned int)sizeof(BDSP_Arm_P_AckInfo)));
		goto end;
	}


	if(hConnection == pDevice->armDspApp.hConnection)
	{
		if( BDSP_Arm_P_AckType_eDevice == sAckInfo.eAckType)
		{
			BKNI_SetEvent(pDevice->hDeviceEvent);
		}
		else if( BDSP_Arm_P_AckType_eTask == sAckInfo.eAckType)
		{
			uiTaskIndex = sAckInfo.params.ui32TaskID;
			uiTaskIndex = uiTaskIndex - BDSP_ARM_TASK_ID_START_OFFSET;
			pArmTask = pDevice->taskDetails.pArmTask[uiTaskIndex];
			if(pArmTask == NULL)
			{
				BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: something went wrong and Event callback was called for someother task"));
				goto end;
			}
			if (uiTaskIndex < BDSP_ARM_MAX_FW_TASK_PER_DSP)
			{
				BKNI_SetEvent(pDevice->taskDetails.pArmTask[uiTaskIndex]->hEvent);
			}
			else
			{
				BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: TaskIndex is %d which is greater than the max allowed index %d ",uiTaskIndex,(BDSP_ARM_MAX_FW_TASK_PER_DSP-1)));
				goto end;
			}

			switch(pArmTask->lastEventType)
			{
				case BDSP_START_TASK_COMMAND_ID:
					pArmTask->isStopped=false;
					break;
				case BDSP_STOP_TASK_COMMAND_ID:
					pArmTask->isStopped=true;
					break;
				default:
					break;
			}
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: ACK received from ArmDsp with wrong Ack type (%d)",sAckInfo.eAckType));
			goto end;
		}
	}
	else if(hConnection == pDevice->armDspApp.hHbcConnection)
	{
		if( BDSP_Arm_P_AckType_eDevice == sAckInfo.eAckType)
		{
			if(sAckInfo.params.ui32DeviceCmdResp == BDSP_ARM_HBC_SET_WATCHDOG_RESPONSE_ID)
			{
				BDSP_ArmContext *pArmContext=NULL;

				BDBG_ERR(("Received ArmDspSystem Watchdog"));
				/* set device watchdog flag */
				pDevice->deviceWatchdogFlag = true;

				for ( pArmContext = BLST_S_FIRST(&pDevice->contextList);
					  pArmContext != NULL;
					  pArmContext = BLST_S_NEXT(pArmContext, node) )
				{
					pArmContext->contextWatchdogFlag = true;
					BDBG_ERR(("Arm Watchdog context 0x%p set",(void *)pArmContext));
				}

#ifdef BCHP_RAAGA_DSP_INTH_HOST_SET
				/* Write to Raaga Watchdog Register */
				BDSP_Write32(pDevice->regHandle,BCHP_RAAGA_DSP_INTH_HOST_SET,(0x1<<BCHP_RAAGA_DSP_INTH_HOST_SET_WATCHDOG_TIMER_ATTN_SHIFT));
#else
				BDBG_ERR(("Raaga not available; Hence ArmDspSystem Watchdog cannot recover"));
#endif

			}
			else
			{
				BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: Cmd received from HBC with wrong cmd type (%d)",sAckInfo.params.ui32DeviceCmdResp));
				goto end;
			}
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_AckEventCallback_isr: ACK received from HBC with wrong Ack type (%d)",sAckInfo.eAckType));
			goto end;
		}
	}
	else
	{
		BDBG_ERR(("Received Unknown msg (handl=%p(hA=%p)(hH=%p) AckType=%d, params=%d",(void*)hConnection,(void*)pDevice->armDspApp.hConnection,(void*)pDevice->armDspApp.hHbcConnection,sAckInfo.eAckType,sAckInfo.params.ui32TaskID));
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_AckEventCallback_isr);
}

/***********************************************************************
Name        :   BDSP_Arm_P_ProcessWatchdogInterrupt

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle provided by the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   On occurance of an watchdog, call the Arm Open function.
                On completion, clear the watchdog flag of the Device handle and Context handle
***********************************************************************/
BERR_Code BDSP_Arm_P_ProcessWatchdogInterrupt(
    void *pContextHandle)
{
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BERR_Code err=BERR_SUCCESS;
    BDSP_Arm  *pDevice= pArmContext->pDevice;

    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

    BKNI_EnterCriticalSection();
    /* BDSP_Arm_P_ProcessWatchdogInterrupt() should not be called from within
    Critical sections. To ensure this, added these few lines of code.
    Do nothing here. If BDSP_Arm_P_ProcessWatchdogInterrupt is called incorrectly from
    a critical section, this piece of code will cause it to deadlock and exit */
    BKNI_LeaveCriticalSection();

    BKNI_AcquireMutex(pDevice->watchdogMutex);
    BDBG_WRN(("BDSP_Arm_P_ProcessWatchdogInterrupt called"));


    if(pDevice->deviceWatchdogFlag == true)
    {

		BDSP_MAP_Table_Entry MapTable[BDSP_ARM_MAX_ALLOC_DEVICE];
		uint32_t ui32NumEntries = 0;

		BKNI_Memset(MapTable,0,(BDSP_ARM_MAX_ALLOC_DEVICE*sizeof(BDSP_MAP_Table_Entry)));
		BDSP_Arm_P_RetrieveEntriesToUnMap(&(pDevice->sDeviceMapTable[0]), &MapTable[0], &ui32NumEntries, BDSP_ARM_MAX_ALLOC_DEVICE);
		err = BDSP_Arm_P_SendUnMapCommand(pDevice, &MapTable[0], ui32NumEntries);
		if (BERR_SUCCESS != err)
		{
			BDBG_ERR(("BDSP_Arm_P_ProcessWatchdogInterrupt: Send ARM UNMAP Command failed!!!!"));
		}

		err = BDSP_Arm_P_ArmHbcClose(pDevice);
		if (BERR_SUCCESS != err)
		{
		  BDBG_ERR(("BDSP_Arm_P_ProcessWatchdogInterrupt: BDSP_Arm_P_ArmHbcClose  failed!!!!"));
		}

		err = BDSP_Arm_P_ArmDspClose(pDevice);
		if (BERR_SUCCESS != err)
		{
		  BDBG_ERR(("BDSP_Arm_P_ProcessWatchdogInterrupt: BDSP_Arm_P_ArmDspClose  failed!!!!"));
		}

        BDSP_Arm_P_Open((void *)pDevice);

        pArmContext->pDevice->deviceWatchdogFlag = false;
    }

    pArmContext->contextWatchdogFlag = false;

    BKNI_ReleaseMutex(pArmContext->pDevice->watchdogMutex);

    return err;
}



#if 0 /*SYNCASYNC_MSG CDN TBD*/
/***************************************************************************
Description:
    This API will be called when any synchronous interrupt will be raised by
    FW for any task.
Returns:
    void
See Also:
    BDSP_P_DSP2HostSyn_isr, BDSP_P_DSP2HostAsyn_isr, BDSP_P_InterruptInstall
***************************************************************************/
static void BDSP_Arm_P_DSP2HostSyn_isr(
        void    *pDeviceHandle,
        int     iParm2
)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    uint32_t    ui32DspId = 0, ui32TaskId = 0;
    uint32_t    ui32SyncTaskIds = 0;
    uint32_t    uiTaskIndex=0;
    int i = 0;
    BDSP_ArmTask *pArmTask;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BDBG_ENTER(BDSP_Arm_P_DSP2HostSyn_isr);
    BDBG_MSG (("... Sync Message Posted by ARM DSP ... "));

    ui32DspId = iParm2;
    for(i  = 0; i < BDSP_ARM_MAX_FW_TASK_PER_DSP; i++)
    {
        pArmTask = pDevice->taskDetails.pArmTask[i];

        if(pArmTask == NULL)
        {
           continue;
        }

        if (pArmTask->taskId == BDSP_P_INVALID_TASK_ID)
        {
           continue;
        }
#if 0 /* TO DO */
        ui32SyncTaskIds = BDSP_Read32_isr(
                            pDevice->regHandle,
                            BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_0  + pDevice->dspOffset[ui32DspId]);
#else
        BDBG_ERR(("BDSP_Arm_P_DSP2HostSyn_isr: Need to figure out which register to read from"));
#endif

        if (ui32SyncTaskIds & (1 << pArmTask->taskId))
        {
           ui32TaskId = pArmTask->taskId;
#if 0 /* TO DO */
           BDSP_Write32_isr(
                        pDevice->regHandle,
                        BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_0 + pDevice->dspOffset[ui32DspId], ui32SyncTaskIds);
#else
            BDBG_ERR(("BDSP_Arm_P_DSP2HostSyn_isr: Need to figure out which register to write to"));
#endif
            uiTaskIndex = ui32TaskId - BDSP_ARM_TASK_ID_START_OFFSET;

            if (uiTaskIndex < BDSP_ARM_MAX_FW_TASK_PER_DSP)
            {
                BKNI_SetEvent(pDevice->taskDetails.pArmTask[uiTaskIndex]->hEvent);
            }

            switch(pArmTask->lastEventType)
            {
                case BDSP_START_TASK_COMMAND_ID:
                    pArmTask->isStopped=false;
                    break;

                case BDSP_STOP_TASK_COMMAND_ID:
                    pArmTask->isStopped=true;
                    break;

                default:
                    break;
            }
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_DSP2HostSyn_isr);
    return;
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
static void BDSP_Arm_P_DSP2HostAsyn_isr(
        void    *pDeviceHandle,
        int     iParm2
)
{
    uint32_t    ui32ASyncTaskIds = 0;
    uint32_t    ui32DspId = 0;
    int i=0;
    BDSP_Arm     *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDSP_ArmTask *pArmTask;
    BDSP_Arm_P_AsynEventMsg  *pEventMsg = NULL;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    BDBG_ENTER(BDSP_Arm_P_DSP2HostAsyn_isr);

    BDBG_MSG (("... Async Message(s) Posted by ARM DSP ... "));

    ui32DspId = iParm2;

    for(i = 0; i < BDSP_ARM_MAX_FW_TASK_PER_DSP; i++)
    {
        pArmTask = pDevice->taskDetails.pArmTask[i];
        if(pArmTask == NULL)
        {
            continue;
        }

        if((pArmTask->taskId == BDSP_P_INVALID_TASK_ID))
        {
            continue;
        }

        if((void *)pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr == NULL)
        {
            continue;
        }

#if 0 /* TO DO */
        ui32ASyncTaskIds = BDSP_Read32_isr(
                                pDevice->regHandle,
                                BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_STATUS_1  + pDevice->dspOffset[ui32DspId]);
#else
        BDBG_ERR(("BDSP_Arm_P_DSP2HostAsyn_isr: Need to figure out which register to read from"));
#endif

        if (ui32ASyncTaskIds & (1 << pArmTask->taskId))
        {
            unsigned int uiNumMsgs = 0, i = 0;

#if 0 /* TO DO */
            BDSP_Write32_isr(
                pDevice->regHandle,
                BCHP_RAAGA_DSP_PERI_SW_MSG_BITS_CLEAR_1  + pDevice->dspOffset[ui32DspId], (1 << pRaagaTask->taskId));
#else
            BDBG_ERR(("BDSP_Arm_P_DSP2HostAsyn_isr: Need to figure out which register to write to"));
#endif

            pEventMsg = (void *)pArmTask->taskMemGrants.sTaskQueue.sAsyncMsgBufmem.pBaseAddr;

            /* Read the message in sEventMsg */
            BDSP_Arm_P_GetAsyncMsg_isr(pArmTask->hAsyncMsgQueue,pEventMsg, &uiNumMsgs);

            /* Now check which response came from FW and work according to it*/
            for(i = 0; i < uiNumMsgs; i++)
            {
#if 0
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
#else
                BDBG_ERR(("BDSP_Arm_P_DSP2HostAsyn_isr: No ASYNC MESSAGE HANDLING ADDED"));
#endif
            }
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_DSP2HostAsyn_isr);
    return;
}
#endif /*CDN_TBD*/
