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

BDBG_MODULE(bdsp_arm_command);

BERR_Code BDSP_Arm_P_ProcessInitCommand(
    BDSP_Arm   *pDevice,
    unsigned    dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_P_Command sCommand;
    unsigned i= 0;
    unsigned MemoryRequired  = 0;
    unsigned preemptionLevel = 0;

    BDBG_ENTER(BDSP_Arm_P_ProcessInitCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessInitCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_INIT;
    sCommand.sCommandHeader.ui32CommandCounter      = 0;
    sCommand.sCommandHeader.ui32TaskID              = 0;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle, BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    sCommand.uCommand.sArmInitCommand.sCommonMemory.sRODescriptor.ui64StartAddress = pDevice->memInfo.sROMemoryPool.Memory.offset;
    sCommand.uCommand.sArmInitCommand.sCommonMemory.sRODescriptor.ui64Size         = pDevice->memInfo.sROMemoryPool.ui32Size;
    sCommand.uCommand.sArmInitCommand.sCommonMemory.sRWDescriptor.ui64StartAddress = pDevice->memInfo.sRWMemoryPool[dspindex].Memory.offset;
    sCommand.uCommand.sArmInitCommand.sCommonMemory.sRWDescriptor.ui64Size         = pDevice->memInfo.sRWMemoryPool[dspindex].ui32Size;
    sCommand.uCommand.sArmInitCommand.sCommonMemory.sIoMemoryDescriptor.ui64StartAddress = pDevice->memInfo.sIOMemoryPool[dspindex].Memory.offset;
    sCommand.uCommand.sArmInitCommand.sCommonMemory.sIoMemoryDescriptor.ui64Size         = pDevice->memInfo.sIOMemoryPool[dspindex].ui32Size;

    sCommand.uCommand.sArmInitCommand.sCustomMMInfo.ui32UserProcessSpawnMemSize = 0; /* Not Used in ARM*/
    sCommand.uCommand.sArmInitCommand.sCustomMMInfo.ui64ProcessSpawnMemStartAddr= 0; /* Future Use*/
    for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
    {
        sCommand.uCommand.sArmInitCommand.sCustomMMInfo.ui32WorkBufferBlockSizePerLevel[preemptionLevel]=
                        pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size;
        MemoryRequired += pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size;
    }
    sCommand.uCommand.sArmInitCommand.sCustomMMInfo.ui32TotalWorkBufferSize = (pDevice->numCorePerDsp*MemoryRequired);
    sCommand.uCommand.sArmInitCommand.sCustomMMInfo.ui64WorkBufferStartAddr =
                        pDevice->memInfo.WorkBufferMemory[dspindex][0].Buffer.offset;

    sCommand.uCommand.sArmInitCommand.sSchedulingInfo.ui32NumCores            = 1;
    sCommand.uCommand.sArmInitCommand.sSchedulingInfo.ui32NumUserProcess      = BDSP_MAX_NUM_USER_PROCESS;
    sCommand.uCommand.sArmInitCommand.sSchedulingInfo.ui32NumPreemptionLevels = BDSP_MAX_NUM_PREEMPTION_LEVELS;
    sCommand.uCommand.sArmInitCommand.sSchedulingInfo.ui32NumSchedulingLevels = BDSP_MAX_NUM_SCHED_LEVELS;

    for(i=0; i< BDSP_MAX_NUM_SCHED_LEVELS;i++)
    {
        sCommand.uCommand.sArmInitCommand.sSchedulingInfo.ui32PreemptiveThreshold[i] = 0;
    }

    sCommand.uCommand.sArmInitCommand.sTimerInfo.ui32PeriodicTimerInUs = BDSP_PERIODIC_TIMER;
    sCommand.uCommand.sArmInitCommand.sTimerInfo.ui32WatchdogTimerinMs = BDSP_WATCHDOG_TIMER;

    sCommand.uCommand.sArmInitCommand.ui64NumRegRegions = 2;
    sCommand.uCommand.sArmInitCommand.RegisterRegions[0].ui64StartAddress = 0xF0C00000;
    sCommand.uCommand.sArmInitCommand.RegisterRegions[0].ui64Size         = 0x00100000;
    sCommand.uCommand.sArmInitCommand.RegisterRegions[1].ui64StartAddress = 0xF0A00000;
    sCommand.uCommand.sArmInitCommand.RegisterRegions[1].ui64Size         = 0x00080000;
    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[dspindex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
         BDBG_ERR(("BDSP_Arm_P_ProcessInitCommand: Error in Writing the Command"));
         BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BDSP_Arm_P_ProcessInitCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessPingCommand(
    BDSP_Arm *pDevice,
    unsigned  dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_P_Command  sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessPingCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessPingCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_PING;
    sCommand.sCommandHeader.ui32CommandCounter      = 0;
    sCommand.sCommandHeader.ui32TaskID              = 0;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_ResetEvent(pDevice->hEvent[dspindex]);
    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[dspindex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPingCommand: Error in Writing the command"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pDevice->hEvent[dspindex], BDSP_EVENT_TIMEOUT_IN_MS*20);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPingCommand: PING COMMAND ACK timeout from DSP %d!!!",dspindex));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pDevice->hGenRespQueue[dspindex], (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPingCommand: Unable to read the Response for PING from DSP %d", dspindex));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_PING))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPingCommand: PING ACK not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessPingCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessStartTaskCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_StartTaskCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessStartTaskCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessStartTaskCommand"));
    BDBG_MSG(("\t Scheduling Mode      = %s", FirmwareSchedulingType[pPayload->eSchedulingMode]));
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

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_START_TASK;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sStartTask, pPayload, sizeof(BDSP_P_StartTaskCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStartTaskCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStartTaskCommand: START TASK RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStartTaskCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_START_TASK))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStartTaskCommand: START TASK RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
    pArmTask->taskParams.isRunning = true;
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessStartTaskCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessStopTaskCommand(
    BDSP_ArmTask *pArmTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessStopTaskCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessStopTaskCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_STOP_TASK;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_ResetEvent(pArmTask->hEvent);
    pArmTask->taskParams.isRunning = false;

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStopTaskCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStopTaskCommand: STOP TASK RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStopTaskCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_STOP_TASK))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessStopTaskCommand: STOP TASK RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessStopTaskCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessCITReConfigCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_CitReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessCITReConfigCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessCITReConfigCommand"));
    BDBG_MSG(("\t CIT Config Memory Offset   ="BDSP_MSG_FMT" Size = %d ",BDSP_MSG_ARG(pPayload->sFwConfigMemoryInfo.BaseAddr),pPayload->sFwConfigMemoryInfo.Size));
    BDBG_MSG(("\t CIT ReConfig Memory Offset ="BDSP_MSG_FMT" Size = %d ",BDSP_MSG_ARG(pPayload->sHostConfigMemoryInfo.BaseAddr),pPayload->sHostConfigMemoryInfo.Size));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_CIT_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sCitReconfigure, pPayload, sizeof(BDSP_P_CitReconfigCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessCITReConfigCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessCITReConfigCommand: CIT RECONFIG timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessCITReConfigCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_CIT_RECONFIG))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessCITReConfigCommand: CIT RECONFIG RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessCITReConfigCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessAlgoReconfigCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_AlgoReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessAlgoReconfigCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessAlgoReconfigCommand"));
    BDBG_MSG(("\t Algorithm = %s",Algorithm2Name[pPayload->eAlgorithm]));
    BDBG_MSG(("\t Stage Total Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sStageMemoryInfo.BaseAddr),pPayload->sStageMemoryInfo.Size));
    BDBG_MSG(("\t Host Config Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sHostConfigMemoryInfo.BaseAddr),pPayload->sHostConfigMemoryInfo.Size));
    BDBG_MSG(("\t F.W. Config Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPayload->sFwConfigMemoryInfo.BaseAddr),pPayload->sFwConfigMemoryInfo.Size));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_ALGO_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sAlgoReconfig, pPayload, sizeof(BDSP_P_AlgoReconfigCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAlgoReconfigCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAlgoReconfigCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAlgoReconfigCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_ALGO_RECONFIG))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAlgoReconfigCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessAlgoReconfigCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessTsmReconfigCommand_isr(
    BDSP_ArmTask *pArmTask,
    BDSP_P_TsmReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessTsmReconfigCommand_isr);
    BDBG_MSG(("BDSP_Arm_P_ProcessTsmReconfigCommand_isr"));
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

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_TSM_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sTsmReconfig, pPayload, sizeof(BDSP_P_TsmReconfigCommand));

    errCode = BDSP_Arm_P_SendCommand_isr(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessTsmReconfigCommand_isr: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessTsmReconfigCommand_isr);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessDataSyncReconfigCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_DataSyncReconfigCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessDataSyncReconfigCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessDataSyncReconfigCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_DATASYNC_RECONFIG;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sDataSyncReconfig, pPayload, sizeof(BDSP_P_DataSyncReconfigCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessDataSyncReconfigCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessDataSyncReconfigCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessDataSyncReconfigCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_DATASYNC_RECONFIG))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessDataSyncReconfigCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessDataSyncReconfigCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessEventEnableDisableCommand_isr(
    BDSP_ArmTask *pArmTask,
    BDSP_P_EventEnableDisableCommand *pPayload
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;

    BDBG_ENTER(BDSP_Arm_P_ProcessEventEnableDisableCommand_isr);
    BDBG_MSG(("BDSP_Arm_P_ProcessEventEnableDisableCommand_isr"));
    BDBG_MSG(("\t Enable Event Mask = 0x%x",pPayload->ui32EnableEvent));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_EVENT_NOTIFICATION;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sEventEnableDisable, pPayload, sizeof(BDSP_P_EventEnableDisableCommand));
    errCode = BDSP_Arm_P_SendCommand_isr(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessEventEnableDisableCommand_isr: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessEventEnableDisableCommand_isr);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessPauseCommand(
    BDSP_ArmTask *pArmTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessPauseCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessPauseCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_PAUSE;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_ResetEvent(pArmTask->hEvent);
    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPauseCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPauseCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPauseCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_PAUSE))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessPauseCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessPauseCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessResumeCommand(
    BDSP_ArmTask *pArmTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessResumeCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessResumeCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_RESUME;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_ResetEvent(pArmTask->hEvent);
    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessResumeCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessResumeCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessResumeCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_RESUME))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessResumeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessResumeCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessAudioGapFillCommand(
    BDSP_ArmTask *pArmTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;

    BDBG_ENTER(BDSP_Arm_P_ProcessAudioGapFillCommand);
    BDBG_MSG(("BDSP_Arm_P_ProcessAudioGapFillCommand"));

    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    pDevice = pArmTask->pContext->pDevice;

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_GAP_FILL_ENABLE;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eNone;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioGapFillCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessAudioGapFillCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessFrameAdvanceCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_FrameAdvanceCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessFrameAdvanceCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));

    pDevice = pArmTask->pContext->pDevice;
    BDBG_MSG(("BDSP_Arm_P_ProcessFrameAdvanceCommand"));
    BDBG_MSG(("\t Duration Of Frame Advance = %d",pPayLoad->ui32DurationOfFrameAdv));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_FRAME_ADVANCE;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sFrameAdvance, pPayLoad, sizeof(BDSP_P_FrameAdvanceCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessFrameAdvanceCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessFrameAdvanceCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessFrameAdvanceCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_FRAME_ADVANCE))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessFrameAdvanceCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessFrameAdvanceCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessAudioOutputFreezeCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_AudioOutputFreezeCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessAudioOutputFreezeCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));

    pDevice = pArmTask->pContext->pDevice;
    BDBG_MSG(("BDSP_Arm_P_ProcessAudioOutputFreezeCommand"));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_OUTPUT_FREEZE;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sAudioOutputFreeze, pPayLoad, sizeof(BDSP_P_AudioOutputFreezeCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputFreezeCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputFreezeCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputFreezeCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_AUDIO_OUTPUT_FREEZE))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputFreezeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessAudioOutputFreezeCommand);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand(
    BDSP_ArmTask *pArmTask,
    BDSP_P_AudioOutputUnFreezeCommand *pPayLoad
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm  *pDevice;
    BDSP_P_Command sCommand;
    BDSP_P_Response sRsp;

    BDBG_ENTER(BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand);
    BKNI_Memset(&sCommand, 0, sizeof(BDSP_P_Command));
    BKNI_Memset(&sRsp, 0, sizeof(BDSP_P_Response));

    pDevice = pArmTask->pContext->pDevice;
    BDBG_MSG(("BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand"));

    sCommand.sCommandHeader.eCommandID              = BDSP_P_CommandID_AUDIO_OUTPUT_UNFREEZE;
    sCommand.sCommandHeader.ui32CommandCounter      = pArmTask->taskParams.commandCounter++;
    sCommand.sCommandHeader.ui32TaskID              = pArmTask->taskParams.taskId;
    sCommand.sCommandHeader.eResponseType           = BDSP_P_ResponseType_eRequired;
    sCommand.sCommandHeader.ui32CommandSizeInBytes  = sizeof(BDSP_P_Command);
    /*sCommand.sCommandHeader.ui32CommandTimeStamp    = BDSP_ReadReg32(pDevice->regHandle,BCHP_RAAGA_DSP_TIMERS_TSM_TIMER_VALUE);*/

    BKNI_Memcpy((void *)&sCommand.uCommand.sAudioOutputUnFreeze, pPayLoad, sizeof(BDSP_P_AudioOutputUnFreezeCommand));
    BKNI_ResetEvent(pArmTask->hEvent);

    errCode = BDSP_Arm_P_SendCommand(pDevice->hCmdQueue[pArmTask->createSettings.dspIndex], &sCommand);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand: Error in Writing the command for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    errCode = BKNI_WaitForEvent(pArmTask->hEvent, BDSP_EVENT_TIMEOUT_IN_MS);
    if (BERR_TIMEOUT == errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand: RESPONSE timeout for Task (%d)",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GetResponse(pArmTask->hSyncQueue, (void *)&sRsp, sizeof(BDSP_P_Response));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand: Unable to read response for Task (%d)",pArmTask->taskParams.taskId));
        goto end;
    }

    if((sRsp.sResponseHeader.eStatus != BERR_SUCCESS)||
       (sRsp.sResponseHeader.eCommandID != BDSP_P_CommandID_AUDIO_OUTPUT_UNFREEZE))
    {
        BDBG_ERR(("BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand: RESPONSE not received successfully!!, Status = %d, Response for CMD ID = %d",
            sRsp.sResponseHeader.eStatus, sRsp.sResponseHeader.eCommandID));
        errCode = BERR_TRACE(sRsp.sResponseHeader.eStatus);
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand);
    return errCode;
}
