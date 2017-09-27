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

BDBG_MODULE(bdsp_raaga_command);

BERR_Code BDSP_Raaga_P_ProcessInitCommand(
    BDSP_Raaga *pDevice,
    unsigned    dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga_P_Command sCommand;
    unsigned i= 0;
    unsigned MemoryRequired =0;
	unsigned preemptionLevel = 0;

    BDBG_ENTER(BDSP_Raaga_P_ProcessInitCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessInitCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_INIT;
    sCommand.sCommandHeader.ui32CommandCounter      = 0;
    sCommand.sCommandHeader.ui32TaskID              = 0;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    /* RO  Memory = KERNAL+RDB+ROMFS+Loadable Image*/
    sCommand.uCommand.sInitCommand.sROImageSizeInfo.ui32RORdbVarSize= BDSP_IMG_SYSTEM_RDBVARS_SIZE;
    sCommand.uCommand.sInitCommand.sROImageSizeInfo.ui32RORomfsSize =
					(BDSP_IMG_INIT_ROMFS_SIZE+BDSP_IMG_SYSTEM_CODE_SIZE+BDSP_IMG_SYSTEM_LIB_SIZE);
    sCommand.uCommand.sInitCommand.sROImageSizeInfo.ui32ROLoadableImgSize = (pDevice->memInfo.sROMemoryPool.ui32Size-
                    (BDSP_IMG_SYSTEM_KERNEL_SIZE+BDSP_IMG_SYSTEM_RDBVARS_SIZE+
                     BDSP_IMG_INIT_ROMFS_SIZE+BDSP_IMG_SYSTEM_CODE_SIZE+BDSP_IMG_SYSTEM_LIB_SIZE));

    sCommand.uCommand.sInitCommand.sCustomMMInfo.ui32UserProcessSpawnMemSize = BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE;
	for(preemptionLevel=0; preemptionLevel<BDSP_RAAGA_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
	{
		sCommand.uCommand.sInitCommand.sCustomMMInfo.ui32WorkBufferBlockSizePerLevel[preemptionLevel]=
						pDevice->memInfo.WorkBufferMemory[preemptionLevel].ui32Size;
		MemoryRequired += pDevice->memInfo.WorkBufferMemory[preemptionLevel].ui32Size;
	}
	sCommand.uCommand.sInitCommand.sCustomMMInfo.ui32TotalWorkBufferSize = (pDevice->numCorePerDsp*MemoryRequired);

    BDSP_Raaga_P_CalculateInitMemory(&MemoryRequired);
    sCommand.uCommand.sInitCommand.sRWImageSizeInfo.ui32RWCommonMemSize += MemoryRequired;
    BDSP_Raaga_P_CalculateDebugMemory(&pDevice->deviceSettings, &MemoryRequired);
    sCommand.uCommand.sInitCommand.sRWImageSizeInfo.ui32RWCommonMemSize += MemoryRequired;
    sCommand.uCommand.sInitCommand.sRWImageSizeInfo.ui32RWSystemProcessMemSize= BDSP_IMG_TOPLEVEL_PROCESS_MEM_SIZE;
    sCommand.uCommand.sInitCommand.sRWImageSizeInfo.ui32RWDefaultMMMemSize    = BDSP_IMG_DEFAULT_MM_PROC_HEAP_SIZE;
    sCommand.uCommand.sInitCommand.sRWImageSizeInfo.ui32RWCustomMMMemSize     =
                    ((BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE*BDSP_RAAGA_MAX_NUM_USER_PROCESS)+
                    sCommand.uCommand.sInitCommand.sCustomMMInfo.ui32TotalWorkBufferSize);

    sCommand.uCommand.sInitCommand.sHeapInfo.ui32NumHeap = 3;
    sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[0].HeapBaseAddr = pDevice->deviceSettings.memc[0].baseAddress;
    sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[0].ui64HeapSize = pDevice->deviceSettings.memc[0].size;
    BDBG_MSG(("MEMC%d size : " BDSP_MSG_FMT, 0, BDSP_MSG_ARG(pDevice->deviceSettings.memc[0].size)));
    BDBG_MSG(("MEMC%d size : " BDSP_MSG_FMT, 1, BDSP_MSG_ARG(pDevice->deviceSettings.memc[1].size)));
    BDBG_MSG(("MEMC%d size : " BDSP_MSG_FMT, 2, BDSP_MSG_ARG(pDevice->deviceSettings.memc[2].size)));

    BDBG_MSG(("MEMC%d base address  : " BDSP_MSG_FMT, 0, BDSP_MSG_ARG(pDevice->deviceSettings.memc[0].baseAddress)));

    if(0 != pDevice->deviceSettings.memc[1].size)
    {
        sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[1].HeapBaseAddr = pDevice->deviceSettings.memc[1].baseAddress;
        sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[1].ui64HeapSize = pDevice->deviceSettings.memc[1].size;
        BDBG_MSG(("MEMC%d base address  : " BDSP_MSG_FMT, 1, BDSP_MSG_ARG(pDevice->deviceSettings.memc[1].baseAddress)));
    }
    if(0 != pDevice->deviceSettings.memc[2].size)
    {
        sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[2].HeapBaseAddr = pDevice->deviceSettings.memc[2].baseAddress;
        sCommand.uCommand.sInitCommand.sHeapInfo.sHeapLimits[2].ui64HeapSize = pDevice->deviceSettings.memc[2].size;
        BDBG_MSG(("MEMC%d base address  : " BDSP_MSG_FMT, 2, BDSP_MSG_ARG(pDevice->deviceSettings.memc[2].baseAddress)));
    }

    sCommand.uCommand.sInitCommand.sSchedulingInfo.ui32NumCores            = pDevice->numCorePerDsp;
    sCommand.uCommand.sInitCommand.sSchedulingInfo.ui32NumUserProcess      = BDSP_RAAGA_MAX_NUM_USER_PROCESS;
    sCommand.uCommand.sInitCommand.sSchedulingInfo.ui32NumPreemptionLevels = BDSP_RAAGA_MAX_NUM_PREEMPTION_LEVELS;
    sCommand.uCommand.sInitCommand.sSchedulingInfo.ui32NumSchedulingLevels = BDSP_RAAGA_MAX_NUM_SCHED_LEVELS;

    for(i=0; i< BDSP_RAAGA_MAX_NUM_SCHED_LEVELS;i++)
    {
        sCommand.uCommand.sInitCommand.sSchedulingInfo.ui32PreemptiveThreshold[i] = 0;
    }

    sCommand.uCommand.sInitCommand.sTimerInfo.ui32PeriodicTimerInUs = BDSP_RAAGA_PERIODIC_TIMER;
    sCommand.uCommand.sInitCommand.sTimerInfo.ui32WatchdogTimerinMs = BDSP_RAAGA_WATCHDOG_TIMER;

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[dspindex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessInitCommand: Error in Writing the Command"));
        BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BDSP_Raaga_P_ProcessInitCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessPingCommand(
    BDSP_Raaga *pDevice,
    unsigned    dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessPingCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessPingCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));

    sCommand.sCommandHeader.eCommandID           = BDSP_P_CommandID_PING;
    sCommand.sCommandHeader.ui32CommandCounter      = 0;
    sCommand.sCommandHeader.ui32TaskID              = 0;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_ResetEvent(pDevice->hEvent[dspindex]);
    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[dspindex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPingCommand: Error in Writing the command"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pDevice->hEvent[dspindex], BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPingCommand: PING COMMAND ACK timeout from DSP %d!!!",dspindex));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pDevice->hGenRespQueue[dspindex], (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPingCommand: Unable to read the Response for PING from DSP %d", dspindex));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_PING))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPingCommand: PING ACK not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessPingCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessStartTaskCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_StartTaskCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessStartTaskCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessStartTaskCommand"));
	BDBG_MSG(("\t Scheduling Mode      = %s", SchedulingMode[pPayload->eSchedulingMode]));
	BDBG_MSG(("\t Task Type            = %s", TaskType[pPayload->eTaskType]));
	BDBG_MSG(("\t Scheduling Level     = %d", pPayload->ui32SchedulingLevel));
	BDBG_MSG(("\t Task ID              = %d", pPayload->ui32TaskId));
	BDBG_MSG(("\t Master Task ID       = %d", pPayload->ui32MasterTaskId));
	BDBG_MSG(("\t Sync FIFO ID         = %d", pPayload->ui32SyncQueueFifoId));
	BDBG_MSG(("\t Async FIFO ID        = %d", pPayload->ui32AsyncQueueFifoId));
	BDBG_MSG(("\t Event Mask Enable    = 0x%x", pPayload->ui32EventEnableMask));
	BDBG_MSG(("\t Task   Memory Offset ="BDSP_MSG_FMT" Size = %d ",BDSP_MSG_ARG(pPayload->sTaskMemoryInfo.BaseAddr),pPayload->sTaskMemoryInfo.Size));
	BDBG_MSG(("\t Shared Memory Offset ="BDSP_MSG_FMT" Size = %d ",BDSP_MSG_ARG(pPayload->sSharedMemoryInfo.BaseAddr),pPayload->sSharedMemoryInfo.Size));
	BDBG_MSG(("\t CIT    Memory Offset ="BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sConfigMemoryInfo.BaseAddr),pPayload->sConfigMemoryInfo.Size));
	BDBG_MSG(("\t Primary Stage Memory Offset ="BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sPrimaryStageMemoryInfo.BaseAddr),pPayload->sPrimaryStageMemoryInfo.Size));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_START_TASK;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sStartTask, pPayload, sizeof(BDSP_P_StartTaskCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStartTaskCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStartTaskCommand: START TASK RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStartTaskCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_START_TASK))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStartTaskCommand: START TASK RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
	pRaagaTask->taskParams.isRunning = true;
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessStartTaskCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessStopTaskCommand(
    BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessStopTaskCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessStopTaskCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_STOP_TASK;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_ResetEvent(pRaagaTask->hEvent);
	pRaagaTask->taskParams.isRunning = false;

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStopTaskCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStopTaskCommand: STOP TASK RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStopTaskCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_STOP_TASK))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessStopTaskCommand: STOP TASK RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessStopTaskCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessAlgoReconfigCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_AlgoReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessAlgoReconfigCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessAlgoReconfigCommand"));
	BDBG_MSG(("\t Algorithm = %s",Algorithm2Name[pPayload->eAlgorithm]));
	BDBG_MSG(("\t Stage Total Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sStageMemoryInfo.BaseAddr),pPayload->sStageMemoryInfo.Size));
	BDBG_MSG(("\t Host Config Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sHostConfigMemoryInfo.BaseAddr),pPayload->sHostConfigMemoryInfo.Size));
	BDBG_MSG(("\t F.W. Config Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sFwConfigMemoryInfo.BaseAddr),pPayload->sFwConfigMemoryInfo.Size));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_ALGO_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sAlgoReconfig, pPayload, sizeof(BDSP_P_AlgoReconfigCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAlgoReconfigCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAlgoReconfigCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAlgoReconfigCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_ALGO_RECONFIG))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAlgoReconfigCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessAlgoReconfigCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessTsmReconfigCommand_isr(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_TsmReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessTsmReconfigCommand_isr);
	BDBG_MSG(("BDSP_Raaga_P_ProcessTsmReconfigCommand_isr"));
	BDBG_MSG(("\t Smooth Threshold      = %d",pPayload->sTsmSettings.i32TSMSmoothThreshold));
	BDBG_MSG(("\t Sync Limit Threshold  = %d",pPayload->sTsmSettings.i32TSMSyncLimitThreshold));
	BDBG_MSG(("\t Gross Threshold       = %d",pPayload->sTsmSettings.i32TSMGrossThreshold));
	BDBG_MSG(("\t Discard Threshold     = %d",pPayload->sTsmSettings.i32TSMDiscardThreshold));
	BDBG_MSG(("\t Transaction Threshold = %d",pPayload->sTsmSettings.i32TsmTransitionThreshold));
	BDBG_MSG(("\t STC ADDR              = 0x%x",pPayload->sTsmSettings.ui32STCAddr));
	BDBG_MSG(("\t AV Offset             = %d",pPayload->sTsmSettings.ui32AVOffset));
	BDBG_MSG(("\t SW STC Offset         = %d",pPayload->sTsmSettings.ui32SwSTCOffset));
	BDBG_MSG(("\t Audio Offset          = %d",pPayload->sTsmSettings.ui32AudioOffset));
	BDBG_MSG(("\t Enable Recovery       = %s",DisableEnable[pPayload->sTsmSettings.eEnableTSMErrorRecovery]));
	BDBG_MSG(("\t STC Valid             = %s",TrueFalse[pPayload->sTsmSettings.eSTCValid]));
	BDBG_MSG(("\t Playback On           = %s",TrueFalse[pPayload->sTsmSettings.ePlayBackOn]));
	BDBG_MSG(("\t TSM Enable            = %s",DisableEnable[pPayload->sTsmSettings.eTsmEnable]));
	BDBG_MSG(("\t TSM Log Enable        = %s",DisableEnable[pPayload->sTsmSettings.eTsmLogEnable]));
	BDBG_MSG(("\t ASTM Enable           = %s",DisableEnable[pPayload->sTsmSettings.eAstmEnable]));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_TSM_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sTsmReconfig, pPayload, sizeof(BDSP_P_TsmReconfigCommand));

	errCode = BDSP_Raaga_P_SendCommand_isr(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessTsmReconfigCommand_isr: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessTsmReconfigCommand_isr);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessDataSyncReconfigCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_DataSyncReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessDataSyncReconfigCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessDataSyncReconfigCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_DATASYNC_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sDataSyncReconfig, pPayload, sizeof(BDSP_P_DataSyncReconfigCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

	errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessDataSyncReconfigCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessDataSyncReconfigCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessDataSyncReconfigCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_DATASYNC_RECONFIG))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessDataSyncReconfigCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessDataSyncReconfigCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_EventEnableDisableCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;

    BDBG_ENTER(BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr);
	BDBG_MSG(("BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr"));
	BDBG_MSG(("\t Enable Event Mask = 0x%x",pPayload->ui32EnableEvent));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_EVENT_NOTIFICATION;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sEventEnableDisable, pPayload, sizeof(BDSP_P_EventEnableDisableCommand));
    errCode = BDSP_Raaga_P_SendCommand_isr(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessEventEnableDisableCommand_isr);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessPauseCommand(
    BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessPauseCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessPauseCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_PAUSE;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_ResetEvent(pRaagaTask->hEvent);
    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPauseCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPauseCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPauseCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_PAUSE))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessPauseCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessPauseCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessResumeCommand(
    BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessResumeCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessResumeCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_RESUME;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_ResetEvent(pRaagaTask->hEvent);
    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessResumeCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessResumeCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessResumeCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_RESUME))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessResumeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessResumeCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessAudioGapFillCommand(
    BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;

    BDBG_ENTER(BDSP_Raaga_P_ProcessAudioGapFillCommand);
	BDBG_MSG(("BDSP_Raaga_P_ProcessAudioGapFillCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    pDevice = pRaagaTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_GAP_FILL_ENABLE;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioGapFillCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessAudioGapFillCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessFrameAdvanceCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_FrameAdvanceCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessFrameAdvanceCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));

    pDevice = pRaagaTask->pContext->pDevice;
	BDBG_MSG(("BDSP_Raaga_P_ProcessFrameAdvanceCommand"));
	BDBG_MSG(("\t Duration Of Frame Advance = %d",pPayLoad->ui32DurationOfFrameAdv));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_FRAME_ADVANCE;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sFrameAdvance, pPayLoad, sizeof(BDSP_P_FrameAdvanceCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessFrameAdvanceCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessFrameAdvanceCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessFrameAdvanceCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_FRAME_ADVANCE))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessFrameAdvanceCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessFrameAdvanceCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessAudioOutputFreezeCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_AudioOutputFreezeCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessFreezeCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));

    pDevice = pRaagaTask->pContext->pDevice;
	BDBG_MSG(("BDSP_Raaga_P_ProcessAudioOutputFreezeCommand"));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_OUTPUT_FREEZE;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sAudioOutputFreeze, pPayLoad, sizeof(BDSP_P_AudioOutputFreezeCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputFreezeCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputFreezeCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputFreezeCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_AUDIO_OUTPUT_FREEZE))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputFreezeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessAudioOutputFreezeCommand);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand(
    BDSP_RaagaTask *pRaagaTask,
    BDSP_P_AudioOutputUnFreezeCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga  *pDevice;
    BDSP_Raaga_P_Command sCommand;
    BDSP_Raaga_P_Response sRsp;

    BDBG_ENTER(BDSP_Raaga_P_ProcessFreezeCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_Raaga_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_Raaga_P_Response));

    pDevice = pRaagaTask->pContext->pDevice;
	BDBG_MSG(("BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand"));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_OUTPUT_UNFREEZE;
    sCommand.sCommandHeader.ui32CommandCounter      = pRaagaTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pRaagaTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_Raaga_P_Command);
    sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);

    BKNI_Memcpy((void *)&sCommand.uCommand.sAudioOutputUnFreeze, pPayLoad, sizeof(BDSP_P_AudioOutputUnFreezeCommand));
    BKNI_ResetEvent(pRaagaTask->hEvent);

    errCode = BDSP_Raaga_P_SendCommand(pDevice->hCmdQueue[pRaagaTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand: Error in Writing the command for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pRaagaTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand: RESPONSE timeout for Task (%d)",pRaagaTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_GetResponse(pRaagaTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_Raaga_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand: Unable to read response for Task (%d)",pRaagaTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_AUDIO_OUTPUT_UNFREEZE))
    {
        BDBG_ERR(("BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand);
    return errCode;
}
