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

#include "bdsp_arm_priv_include.h"

BDBG_MODULE(bdsp_arm_priv);
BDBG_OBJECT_ID(BDSP_Arm);
BDBG_OBJECT_ID(BDSP_ArmContext);
BDBG_OBJECT_ID(BDSP_ArmTask);
BDBG_OBJECT_ID(BDSP_ArmStage);
BDBG_OBJECT_ID(BDSP_P_InterTaskBuffer);

BERR_Code BDSP_Arm_P_ValidateVersion(
    BDSP_ArmSettings *pSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BTEE_InstanceStatus Status;
    unsigned bdspArmMinorVer = BDSP_ARM_MINOR_VERSION; /* Fix for "comparison of unsigned expression < 0 is always false" */

    BDBG_ENTER(BDSP_Arm_P_ValidateVersion);
    errCode = BTEE_Instance_GetStatus(pSettings->hBteeInstance,&Status);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Unable to GetStatus from BTEE"));
        errCode = BERR_UNKNOWN;
    }
    else
    {
        if(BDSP_ARM_MAJOR_VERSION != Status.version.major)
        {
            BDBG_ERR(("Incompatible Fw (v%d) and Astra (v%d) major version",BDSP_ARM_MAJOR_VERSION,Status.version.major));
            errCode = BERR_UNKNOWN;
        }
        else if(bdspArmMinorVer > Status.version.minor)
        {
            BDBG_ERR(("Incompatible Fw (v%d) and Astra (v%d) minor version",BDSP_ARM_MINOR_VERSION,Status.version.minor));
            errCode = BERR_UNKNOWN;
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_ValidateVersion);
    return errCode;
}
BERR_Code BDSP_Arm_P_ReleaseFIFO(
    BDSP_Arm *pDevice,
    unsigned dspIndex,
    uint32_t* ui32Fifo,
    unsigned numfifos
)
{
    unsigned i;
    BDBG_MSG(("BDSP_Arm_P_ReleaseFIFO: dspIndex=%d, numfifos = %d, startFifoIndex = %d", dspIndex, numfifos, *ui32Fifo));
    BKNI_AcquireMutex(pDevice->deviceMutex);
    for(i=0; i < numfifos; i++)
    {
        BDBG_MSG(("Freeing Fifo (Soft FIFO) %d", (*ui32Fifo+i-BDSP_FIFO_0_INDEX)));
        pDevice->hardwareStatus.softFifo[dspIndex][(*ui32Fifo+i-BDSP_FIFO_0_INDEX)] = false;
    }
    *ui32Fifo = BDSP_FIFO_INVALID;
    BKNI_ReleaseMutex(pDevice->deviceMutex);
    return BERR_SUCCESS;
}

BERR_Code BDSP_Arm_P_AssignFreeFIFO(
    BDSP_Arm *pDevice,
    unsigned dspIndex,
    uint32_t *pui32Fifo,
    unsigned numfifosreqd
)
{
    BERR_Code errCode=BERR_SUCCESS;
    unsigned count = 0;
    int32_t i =0, start_index = 0;
    BDBG_MSG(("BDSP_Arm_P_AssignFreeFIFO: dspIndex=%d, numfifos reqd= %d", dspIndex, numfifosreqd));
    BKNI_AcquireMutex(pDevice->deviceMutex);
    /* Find free Fifo Ids */
    for (i=0; i < (int32_t)BDSP_ARM_NUM_FIFOS; i++)
    {
        if (false == pDevice->hardwareStatus.softFifo[dspIndex][i])
        {
            count++;
            /* Found enough contiguous RDBs. Remember 1 FIFO has 4 RDBs */
            if(count >= numfifosreqd)
            {
                break;
            }
        }
        else
        {
            count = 0;
        }
    }
    if (i >= (int32_t)BDSP_ARM_NUM_FIFOS)
    {
        BKNI_ReleaseMutex(pDevice->deviceMutex);
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        return errCode;
    }
    else
    {
        start_index =  i - numfifosreqd + 1;
        for(;i >= start_index; i--)
        {
            BDBG_MSG(("Allocating fifo (Soft Fifo) %d", i));
            pDevice->hardwareStatus.softFifo[dspIndex][i] = true;
        }
        /* This is the fifo ID from where RDBs are free */
        *pui32Fifo = BDSP_FIFO_0_INDEX + start_index;
    }
    BKNI_ReleaseMutex(pDevice->deviceMutex);
    return errCode;
}

BERR_Code BDSP_Arm_P_InitDeviceSettings(
    BDSP_Arm *pArm
)
{
    pArm->numDsp       = BDSP_ARM_MAX_DSP;
	pArm->numCorePerDsp= BDSP_ARM_MAX_CORE_PER_DSP;

    return BERR_SUCCESS;
}

static BERR_Code BDSP_Arm_P_TeeInit(
    BDSP_Arm *pDevice
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BTEE_ClientCreateSettings sClientSettings;

    BDBG_ENTER(BDSP_Arm_P_TeeInit);
    pDevice->armDspApp.hBteeInstance = pDevice->deviceSettings.hBteeInstance;
    BTEE_Client_GetDefaultCreateSettings(pDevice->armDspApp.hBteeInstance,&sClientSettings);
    sClientSettings.pEventCallback_isr = BDSP_Arm_P_AstraEventCallback_isr;
    sClientSettings.pCallbackData = (void *)pDevice;
    errCode = BTEE_Client_Create(
            pDevice->armDspApp.hBteeInstance,   /* Instance Handle */
            "ARM_BDSP",                         /* Client Name */
            &sClientSettings,                   /* Client Settings */
            &pDevice->armDspApp.hClient         /* [out] */
            );
    if(BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("failed to open Arm BDSP client"));
        goto end;
    }

end:

    BDBG_LEAVE(BDSP_Arm_P_TeeInit);
    return errCode;
}

BERR_Code BDSP_Arm_P_Initialize(
    void *pDeviceHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

    BDBG_ENTER(BDSP_Arm_P_Initialize);
    /* Assert the function arguments*/
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /*If Firmware authentication is Disabled*/
    if(pDevice->deviceSettings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Arm_P_Initialize should be called only if bFwAuthEnable is true"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    errCode = BDSP_Arm_P_OpenUserApp(pDevice);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_Initialize: Unable to Open the Arm User App"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_CheckDspAlive(pDevice);
    if (errCode!=BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_Initialize: DSP not alive"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_Initialize);
    return errCode;
}

BERR_Code BDSP_Arm_P_OpenUserApp(
    BDSP_Arm *pDevice
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BTEE_ConnectionSettings sConnectionSettings;
    BDSP_ArmDspCommand sCommand;

    BDBG_ENTER(BDSP_Arm_P_OpenUserApp);

    errCode = BTEE_Application_Open(
        pDevice->armDspApp.hClient,
        "ARM_AUDIO_SYSTEM",
        "/init_process.fpexe",
        &pDevice->armDspApp.hApplication);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_OpenUserApp: Failed to start Application {ARM_AUDIO_SYSTEM} on Arm"));
        goto end;
    }

    BTEE_Connection_GetDefaultSettings(
        pDevice->armDspApp.hClient,
        &sConnectionSettings    /* [out] */
        );

    errCode = BTEE_Connection_Open(
        pDevice->armDspApp.hApplication,
        "ARM_AUDIO_SYSTEM",
        &sConnectionSettings,   /* */
        &pDevice->armDspApp.hConnection /* [out] Connection Handle */
        );

    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_OpenUserApp: Failed to open connection {ARM_AUDIO_SYSTEM} on Arm"));
        goto end;
    }

    sCommand.eCommandType = BDSP_ArmDspCommandType_Open;
    sCommand.sOpenCommand.ui64identifier       = BDSP_ARM_SYSTEM_OPEN_COMMAND_ID;
    sCommand.sOpenCommand.host2DspCommandQueueOffset = pDevice->hCmdQueue[0]->QueueAddress.BaseOffset;
    sCommand.sOpenCommand.genericResponseQueueOffset = pDevice->hGenRespQueue[0]->QueueAddress.BaseOffset;

    BDBG_MSG(("host2DspCommandQueueOffset = "BDSP_MSG_FMT, BDSP_MSG_ARG(sCommand.sOpenCommand.host2DspCommandQueueOffset)));
    BDBG_MSG(("genericResponseQueueOffset = "BDSP_MSG_FMT, BDSP_MSG_ARG(sCommand.sOpenCommand.genericResponseQueueOffset)));
    errCode = BTEE_Connection_SendMessage(pDevice->armDspApp.hConnection,
        &sCommand,
        sizeof(BDSP_ArmDspCommand));

    if (BERR_SUCCESS != errCode) {
        BDBG_ERR(("BDSP_Arm_P_OpenUserApp: Failed to Send OPEN USER APP Command"));
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_OpenUserApp);
    return errCode;
}

static void BDSP_Arm_P_CloseUserApp(
    BDSP_Arm *pDevice
)
{
    BDBG_ENTER(BDSP_Arm_P_CloseUserApp);

    BTEE_Connection_Close(pDevice->armDspApp.hConnection);

    /* Close the application */
    BTEE_Application_Close(pDevice->armDspApp.hApplication);

    /* Close the ArmDsp Client */
    BTEE_Client_Destroy(pDevice->armDspApp.hClient);

    BDBG_LEAVE(BDSP_Arm_P_CloseUserApp);
}

BERR_Code BDSP_Arm_P_CheckDspAlive(
    BDSP_Arm *pDevice
)
{
	BERR_Code errCode = BERR_SUCCESS;
    unsigned i=0;

	BDBG_ENTER(BDSP_Arm_P_CheckDspAlive);
    for(i=0; i< pDevice->numDsp;i++)
    {
        errCode = BDSP_Arm_P_ProcessPingCommand(pDevice, i);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_CheckDspAlive: DSP %d not Alive", i));
            goto end;
        }
    }

end:
	BDBG_LEAVE(BDSP_Arm_P_CheckDspAlive);
	return errCode;
}

static BERR_Code BDSP_Arm_P_InitAtTaskCreate(
	BDSP_ArmTask 			*pArmTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice = (BDSP_Arm*)pArmTask->pContext->pDevice;
	unsigned dspIndex =0;
	BDBG_ENTER(BDSP_Arm_P_InitAtTaskCreate);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	dspIndex = pArmTask->createSettings.dspIndex;
	pArmTask->taskParams.isRunning 	   = false;
	pArmTask->taskParams.paused	 	   = false;
	pArmTask->taskParams.frozen	 	   = false;
	pArmTask->taskParams.commandCounter  = 0;
	pArmTask->taskParams.lastCommand     = BDSP_P_CommandID_INVALID;
	pArmTask->taskParams.masterTaskId    = BDSP_P_INVALID_TASK_ID;
	BKNI_AcquireMutex(pDevice->deviceMutex);
	pArmTask->taskParams.taskId          = BDSP_P_GetFreeTaskId(&pDevice->taskDetails[dspIndex]);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	errCode = BKNI_CreateEvent(&pArmTask->hEvent);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("Unable to create event for TASK %d",pArmTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		BDBG_ASSERT(0);
	}
	BKNI_ResetEvent(pArmTask->hEvent);

	if(pArmTask->taskParams.taskId == BDSP_P_INVALID_TASK_ID)
	{
		BDBG_ERR(("BDSP_Arm_P_InitAtTaskCreate: Cannot create task as already Max(%d) tasks created on DSP",BDSP_MAX_FW_TASK_PER_DSP));
		BDBG_ASSERT(0);
	}

	BKNI_Memset(&pArmTask->audioInterruptHandlers, 0, sizeof(pArmTask->audioInterruptHandlers));

	BDBG_LEAVE(BDSP_Arm_P_InitAtTaskCreate);
	return errCode;
}

static void BDSP_Arm_P_UnInitAtTaskDestroy(
	BDSP_ArmTask 			*pArmTask
)
{
	BDSP_Arm *pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;

	BDBG_ENTER(BDSP_Arm_P_UnInitAtTaskDestroy);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	BKNI_Memset(&pArmTask->audioInterruptHandlers, 0, sizeof(pArmTask->audioInterruptHandlers));
	BKNI_DestroyEvent(pArmTask->hEvent);
	BKNI_AcquireMutex(pDevice->deviceMutex);
	BDSP_P_ReleaseTaskId(&pDevice->taskDetails[pArmTask->createSettings.dspIndex], &pArmTask->taskParams.taskId);
	BLST_S_REMOVE(&pArmTask->pContext->taskList,pArmTask,BDSP_ArmTask,node);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_LEAVE(BDSP_Arm_P_UnInitAtTaskDestroy);
}

static BERR_Code BDSP_Arm_P_InitAtStartTask(
	BDSP_ArmTask 			*pArmTask,
	BDSP_TaskStartSettings  *pStartSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmTask    *pMasterArmTask;
    BDSP_ArmContext *pArmContext;
	BDSP_Arm        *pDevice;
	BDSP_ArmStage   *pArmPrimaryStage;
	unsigned stageIndex = 0;

	BDBG_ENTER(BDSP_Arm_P_InitAtStartTask);
    pArmContext = (BDSP_ArmContext *)pArmTask->pContext;
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
	pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	if(pDevice->taskDetails[pArmTask->createSettings.dspIndex].numActiveTasks >= BDSP_MAX_NUM_TASKS)
	{
		BDBG_ERR(("BDSP_Arm_P_InitAtStartTask: Max tasks(%d) already running on DSP(%d), cannot start task (%d)",BDSP_MAX_NUM_TASKS,
			pArmTask->createSettings.dspIndex, pArmTask->taskParams.taskId));
		BDBG_ASSERT(0);
	}

	errCode = BDSP_P_CopyStartTaskSettings(pArmContext->settings.contextType, &pArmTask->startSettings, pStartSettings);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Arm_P_InitAtStartTask: Start Settings couldn't be copied for Task %d",pArmTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	pArmTask->taskParams.masterTaskId = BDSP_P_INVALID_TASK_ID;
	if ((pStartSettings->schedulingMode== BDSP_TaskSchedulingMode_eSlave) && (pStartSettings->masterTask !=NULL))
	{
		pMasterArmTask = (BDSP_ArmTask *)(pStartSettings->masterTask->pTaskHandle);
		BDBG_OBJECT_ASSERT(pMasterArmTask, BDSP_ArmTask);
		pArmTask->taskParams.masterTaskId = pMasterArmTask->taskParams.taskId;
	}

	BKNI_AcquireMutex(pDevice->deviceMutex);
	pDevice->taskDetails[pArmTask->createSettings.dspIndex].numActiveTasks++;
	pDevice->taskDetails[pArmTask->createSettings.dspIndex].pTask[pArmTask->taskParams.taskId] = (void *)pArmTask;
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	pArmPrimaryStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
	BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pArmConnectStage)
	BSTD_UNUSED(macroBrId);
	BSTD_UNUSED(macroStId);
	{
		pArmConnectStage->pArmTask = pArmTask;
		pArmConnectStage->stageID    = stageIndex;
		pArmConnectStage->running    = true;
		stageIndex++;
	}
	BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pArmConnectStage)

end:
	BDBG_LEAVE(BDSP_Arm_P_InitAtStartTask);
	return errCode;
}

static void BDSP_Arm_P_UnInitAtStopTask(
	BDSP_ArmTask    *pArmTask
)
{
	BDSP_Arm      *pDevice;
	BDSP_ArmStage *pArmPrimaryStage;

	BDBG_ENTER(BDSP_Arm_P_UnInitAtStopTask);
	pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	pArmPrimaryStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
	BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pArmConnectStage)
	BSTD_UNUSED(macroBrId);
	BSTD_UNUSED(macroStId);
	{
		pArmConnectStage->pArmTask = NULL;
		pArmConnectStage->stageID    = 0;
		pArmConnectStage->running    = false;
	}
	BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pArmConnectStage)

	pArmTask->taskParams.isRunning     = false;
	pArmTask->taskParams.lastCommand   = BDSP_P_CommandID_STOP_TASK;
	pArmTask->taskParams.masterTaskId  = BDSP_P_INVALID_TASK_ID;
	pArmTask->taskParams.commandCounter= 0;

	BKNI_AcquireMutex(pArmTask->pContext->pDevice->deviceMutex);
	pDevice->taskDetails[pArmTask->createSettings.dspIndex].numActiveTasks--;
	pDevice->taskDetails[pArmTask->createSettings.dspIndex].pTask[pArmTask->taskParams.taskId] = NULL;
	BKNI_ReleaseMutex(pArmTask->pContext->pDevice->deviceMutex);

	BDSP_P_DeleteStartTaskSettings(&pArmTask->startSettings);

	BDBG_LEAVE(BDSP_Arm_P_UnInitAtStopTask);
}

static void BDSP_Arm_P_InitDevice(
    BDSP_Arm *pDevice
)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned dspindex=0, index=0;

    BDBG_ENTER(BDSP_Arm_P_InitDevice);

    errCode = BKNI_CreateMutex(&(pDevice->deviceMutex));
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Unable to Allocate the Mutex for Device"));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BDBG_ASSERT(0);
    }

    for (dspindex=0; dspindex<pDevice->numDsp; dspindex++)
    {
        errCode = BKNI_CreateEvent(&(pDevice->hEvent[dspindex]));
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("Unable to create event for DSP %d",dspindex));
            errCode = BERR_TRACE(errCode);
            BDBG_ASSERT(0);
        }
        BKNI_ResetEvent(pDevice->hEvent[dspindex]);

        for (index=0;index<BDSP_ARM_NUM_FIFOS;index++)
        {
            pDevice->hardwareStatus.softFifo[dspindex][index] = false;
        }

		for (index=0;index<BDSP_MAX_DESCRIPTORS;index++)
		{
			pDevice->hardwareStatus.descriptor[dspindex][index] = false;
		}

        for (index=0 ; index< BDSP_MAX_FW_TASK_PER_DSP; index++)
        {
            pDevice->taskDetails[dspindex].taskId[index] = false;
            pDevice->taskDetails[dspindex].pTask[index]  = NULL;
            pDevice->taskDetails[dspindex].numActiveTasks= 0;
        }
    }

	errCode = BDSP_P_PopulateSystemSchedulingDeatils(&pDevice->systemSchedulingInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_InitDevice: Unable to Program the System Scheduling Information"));
		errCode = BERR_TRACE(errCode);
		BDBG_ASSERT(0);
	}

    BDBG_LEAVE(BDSP_Arm_P_InitDevice);
}

static void BDSP_Arm_P_DeInitDevice(
    BDSP_Arm *pDevice
)
{
    unsigned dspindex=0, index=0;

    BDBG_ENTER(BDSP_Arm_P_DeInitDevice);

	BKNI_DestroyMutex(pDevice->deviceMutex);
	for (dspindex=0; dspindex<pDevice->numDsp; dspindex++)
	{
	    BKNI_DestroyEvent(pDevice->hEvent[dspindex]);
		for (index=0;index<BDSP_ARM_NUM_FIFOS;index++)
		{
            pDevice->hardwareStatus.softFifo[dspindex][index] = false;
		}
		for (index=0;index<BDSP_MAX_DESCRIPTORS;index++)
		{
			pDevice->hardwareStatus.descriptor[dspindex][index] = false;
		}
		for (index=0 ; index< BDSP_MAX_FW_TASK_PER_DSP; index++)
		{
			pDevice->taskDetails[dspindex].taskId[index] = false;
			pDevice->taskDetails[dspindex].pTask[index]  = NULL;
			pDevice->taskDetails[dspindex].numActiveTasks= 0;
		}
	}
    BDBG_LEAVE(BDSP_Arm_P_DeInitDevice);
}

static BERR_Code BDSP_Arm_P_ProgramRDB(
    BDSP_Arm *pDevice,
    unsigned  dspIndex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    dramaddr_t *pAddr = NULL, *pCfgAddr= NULL;
    dramaddr_t offset = 0;
    BDBG_ENTER(BDSP_Arm_P_ProgramRDB);

    pAddr = (dramaddr_t *)pDevice->memInfo.sIOMemoryPool[dspIndex].Memory.pAddr;

    /* Program the size of the FMM Register access */
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_REGISTER_WIDTH] = BDSP_SIZE_OF_FMMREG;

    /*Program the FIFO ID of Host2DSP command Queue Fifo ID */
    offset = pDevice->memInfo.CfgRegisters[dspIndex].Buffer.offset;
    pCfgAddr = (dramaddr_t *)pDevice->memInfo.CfgRegisters[dspIndex].Buffer.pAddr;
    *pCfgAddr= pDevice->hCmdQueue[dspIndex]->ui32FifoId;
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID] = offset;
    pCfgAddr++;

    /*Program the FIFO ID of Generic Response command Queue Fifo ID*/
    *pCfgAddr = pDevice->hGenRespQueue[dspIndex]->ui32FifoId;
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPRESPONSE_FIFO_ID] = offset+sizeof(dramaddr_t);

    /*Program the FIFO address*/
    offset = pDevice->memInfo.softFifo[dspIndex].Buffer.offset;
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_BASE_ADDR] = offset;
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_END_ADDR]  = offset+sizeof(dramaddr_t);
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_WRITE_ADDR]= offset+(2*sizeof(dramaddr_t));
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_READ_ADDR] = offset+(3*sizeof(dramaddr_t));
    pAddr[BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_1_BASE_ADDR] = offset+(4*sizeof(dramaddr_t));

    BDSP_MMA_P_FlushCache(pDevice->memInfo.sIOMemoryPool[dspIndex].Memory, BDSP_ARM_IMG_SYSTEM_IO_SIZE);

    BDBG_LEAVE(BDSP_Arm_P_ProgramRDB);
    return errCode;
}

static BERR_Code BDSP_Arm_P_DownloadRuntimeAlgorithm(
		BDSP_ArmTask	*pArmTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmPrimaryStage;

	BDBG_ENTER(BDSP_Arm_P_DownloadRuntimeAlgorithm);
	pArmPrimaryStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
    BDBG_OBJECT_ASSERT(pArmPrimaryStage, BDSP_ArmStage);

    BDBG_ERR(("BDSP_Arm_P_DownloadRuntimeAlgorithm: Runtime Code Download not implemented"));
    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        /*errCode = BDSP_Arm_P_DownloadAlgorithm(pArmTask->pContext->pDevice, pStageIterator->eAlgorithm);*/
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode);
			BDBG_ERR(("BDSP_Arm_P_DownloadRuntimeAlgorithm: Error in downloading Algorithm(%d)",pStageIterator->eAlgorithm));
            goto end;
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Arm_P_DownloadRuntimeAlgorithm);
	return errCode;
}

static BERR_Code BDSP_Arm_P_ReleaseRuntimeAlgorithm(
		BDSP_ArmTask	*pArmTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmPrimaryStage;

	BDBG_ENTER(BDSP_Arm_P_ReleaseRuntimeAlgorithm);
	pArmPrimaryStage = (BDSP_ArmStage *)pArmTask->startSettings.primaryStage->pStageHandle;
    BDBG_OBJECT_ASSERT(pArmPrimaryStage, BDSP_ArmStage);

    BDBG_ERR(("BDSP_Arm_P_ReleaseRuntimeAlgorithm: Runtime Code Download not implemented"));
    BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        /*errCode = BDSP_Arm_P_ReleaseAlgorithm(pArmTask->pContext->pDevice, pStageIterator->eAlgorithm);*/
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode);
			BDBG_ERR(("BDSP_Arm_P_ReleaseRuntimeAlgorithm: Error in Releasing Algorithm(%d)",pStageIterator->eAlgorithm));
            goto end;
        }
    }
    BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseRuntimeAlgorithm);
	return errCode;
}

static BERR_Code BDSP_Arm_P_InitInterframe(
	BDSP_ArmStage *pPrimaryArmStage
)
{
	BERR_Code   errCode = BERR_SUCCESS;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	BDSP_Arm	*pDevice;

	BDBG_ENTER(BDSP_Arm_P_InitInterframe);
	pDevice = (BDSP_Arm *)pPrimaryArmStage->pContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pPrimaryArmStage, pStageIterator)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		BDSP_MMA_Memory Memory;
		pAlgoInfo = BDSP_P_LookupAlgorithmInfo(pStageIterator->eAlgorithm);
		Memory    = pStageIterator->stageMemInfo.sInterframe.Buffer;
		errCode = BDSP_P_InterframeRunLengthDecode(
			Memory.pAddr, /*Destination*/
			pDevice->codeInfo.imgInfo[BDSP_ARM_IMG_ID_IFRAME(pStageIterator->eAlgorithm)].Buffer.pAddr,/*Encoded Interframe Image Address*/
			pDevice->codeInfo.imgInfo[BDSP_ARM_IMG_ID_IFRAME(pStageIterator->eAlgorithm)].ui32Size,/*Encoded Interframe Image Size */
			pStageIterator->stageMemInfo.sInterframe.ui32Size);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_InitInterframe: Error for Algorithm(%d) %s",pStageIterator->eAlgorithm, pAlgoInfo->pName));
			goto end;
		}
		BDSP_MMA_P_FlushCache(Memory, pStageIterator->stageMemInfo.sInterframe.ui32Size);
	}
	BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Arm_P_InitInterframe);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Open

Type        :   BDSP Internal

Input       :   pArm - Handle which needs to be opened/created

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
***********************************************************************/
BERR_Code BDSP_Arm_P_Open(
    BDSP_Arm *pDevice
)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned  dspindex=0, MemoryRequired=0;

    BDBG_ENTER(BDSP_Arm_P_Open);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	BDBG_MSG(("BDSP_Arm_P_Open: Authentication %s", DisableEnable[pDevice->deviceSettings.authenticationEnabled]));

    BDSP_P_ValidateCodeDownloadSettings(&pDevice->deviceSettings.maxAlgorithms[0]);

    if(pDevice->hardwareStatus.deviceWatchdogFlag == false)
    {
        if((pDevice->deviceSettings.authenticationEnabled == true)||
           (pDevice->deviceSettings.preloadImages == true))
              pDevice->codeInfo.preloadImages = true;

        BDSP_Arm_P_InitDevice(pDevice);

        errCode = BDSP_Arm_P_TeeInit(pDevice);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to instantiate TEE Client"));
            BDBG_ASSERT(0);
        }

        errCode = BDSP_Arm_P_AssignAlgoSize(pDevice->deviceSettings.pImageInterface,
                pDevice->deviceSettings.pImageContext,
                &pDevice->codeInfo.imgInfo[0]);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to Assign algorithm sizes for Images"));
            BDBG_ASSERT(0);
        }

        BDSP_Arm_P_CalculateDeviceROMemory(pDevice, &MemoryRequired);
        errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
                                MemoryRequired,
                                &(pDevice->memInfo.sROMemoryPool.Memory),
                                BDSP_MMA_Alignment_4KByte);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to Allocate Read Only Memory for Arm"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }
        BDBG_MSG(("BDSP_Arm_P_Open: RO Memory Required = %d",MemoryRequired));
        pDevice->memInfo.sROMemoryPool.ui32Size     = MemoryRequired;
        pDevice->memInfo.sROMemoryPool.ui32UsedSize = 0;

        for(dspindex=0; dspindex<pDevice->numDsp; dspindex++)
        {
            BDSP_Arm_P_CalculateDeviceIOMemory(pDevice, &MemoryRequired);
            errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
                                    MemoryRequired,
                                    &(pDevice->memInfo.sIOMemoryPool[dspindex].Memory),
                                    BDSP_MMA_Alignment_4KByte);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_Open: Unable to Allocate IO Memory for Arm"));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
            BDBG_MSG(("BDSP_Arm_P_Open: IO Memory Required = %d",MemoryRequired));
            pDevice->memInfo.sIOMemoryPool[dspindex].ui32Size     = MemoryRequired;
            pDevice->memInfo.sIOMemoryPool[dspindex].ui32UsedSize = 0;

            BDSP_Arm_P_CalculateDeviceRWMemory(pDevice, dspindex, &MemoryRequired);
            errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
                                     MemoryRequired,
                                     &(pDevice->memInfo.sRWMemoryPool[dspindex].Memory),
                                     BDSP_MMA_Alignment_4KByte);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_Open: Unable to Allocate Read Write Memory for Arm"));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
            BDBG_MSG(("BDSP_Arm_P_Open: RW Memory Required for Dsp(%d)= %d",dspindex, MemoryRequired));
            pDevice->memInfo.sRWMemoryPool[dspindex].ui32Size     = MemoryRequired;
            pDevice->memInfo.sRWMemoryPool[dspindex].ui32UsedSize = 0;
        }
        errCode = BDSP_Arm_P_DownloadCode((void *)pDevice);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to complete the Code download"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }
    }
    for(dspindex=0; dspindex<pDevice->numDsp; dspindex++)
    {
        if(pDevice->hardwareStatus.deviceWatchdogFlag == false)
        {
            errCode = BDSP_Arm_P_AssignDeviceRWMemory(pDevice, dspindex);
            if(errCode != BERR_SUCCESS)
            {
                 BDBG_ERR(("BDSP_Arm_P_Open: Unable to Assign Read Write Memory for Arm DSP %d", dspindex));
                 BDBG_ASSERT(0);
            }

            errCode = BDSP_P_CreateMsgQueue(
                &pDevice->memInfo.cmdQueueParams[dspindex],
                pDevice->regHandle,
                0,
                &(pDevice->hCmdQueue[dspindex]));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_Open: Unable to Create Command Queue for Arm DSP %d", dspindex));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_P_CreateMsgQueue(
                &pDevice->memInfo.genRspQueueParams[dspindex],
                pDevice->regHandle,
                0,
                &(pDevice->hGenRespQueue[dspindex]));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_Open: Unable to Create Generic Response Queue for Arm DSP %d", dspindex));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }

        errCode = BDSP_Arm_P_InitMsgQueue(pDevice->hCmdQueue[dspindex], pDevice->memInfo.softFifo[dspindex].Buffer);
        if(errCode != BERR_SUCCESS)
        {
         BDBG_ERR(("BDSP_Arm_P_Open: Unable to Initialise Command Queue for Arm DSP %d", dspindex));
         errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         BDBG_ASSERT(0);
        }

        errCode = BDSP_Arm_P_InitMsgQueue(pDevice->hGenRespQueue[dspindex], pDevice->memInfo.softFifo[dspindex].Buffer);
        if(errCode != BERR_SUCCESS)
        {
         BDBG_ERR(("BDSP_Arm_P_Open: Unable to Initialise Generic Response Queue for Arm DSP %d", dspindex));
         errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
         BDBG_ASSERT(0);
        }

		errCode = BDSP_Arm_P_DeviceInterruptInstall((void *)pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to Install Device Interrupt callback for Arm DSP %d", dspindex));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            BDBG_ASSERT(0);
        }

        errCode = BDSP_Arm_P_ProgramRDB(pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to Program the Soft RDB for Arm DSP %d", dspindex));
            BDBG_ASSERT(0);
        }

        errCode = BDSP_Arm_P_ProcessInitCommand(pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Open: Unable to Process the INIT Command for Arm DSP %d", dspindex));
            BDBG_ASSERT(0);
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_Open);
    return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Close

Type        :   BDSP Internal

Input       :   pArm - Handle which needs to be opened/created

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
***********************************************************************/
void BDSP_Arm_P_Close(
    void *pDeviceHandle
)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    unsigned dspIndex=0;
    BERR_Code errCode=BERR_SUCCESS;
    BDSP_ArmContext *pArmContext;

    BDBG_ENTER(BDSP_Arm_P_Close);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    /* Destroy any contexts left open */
    while((pArmContext = BLST_S_FIRST(&pDevice->contextList)))
    {
        BDSP_Context_Destroy(&pArmContext->context);
    }

    /* Free up any interrupt handle left open */
    /*while( (pArmExtInterrput = BLST_S_FIRST(&pDevice->interruptList)) )
    {
        BDSP_FreeExternalInterrupt(&pArmExtInterrput->extInterrupt);
    }*/

	BDSP_Arm_P_CloseUserApp(pDevice);

    for(dspIndex = 0; dspIndex< pDevice->numDsp; dspIndex++)
    {
        errCode = BDSP_P_DestroyMsgQueue(pDevice->hCmdQueue[dspIndex]);
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_Close: CMD queue destroy failed for DSP %d!!!", dspIndex));
            errCode = BERR_TRACE(errCode);
        }

        errCode = BDSP_P_DestroyMsgQueue(pDevice->hGenRespQueue[dspIndex]);
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_Close: Generic RSP queue destroy failed for DSP %d!!!", dspIndex));
            errCode = BERR_TRACE(errCode);
        }

        errCode = BDSP_Arm_P_ReleaseDeviceRWMemory(pDevice, dspIndex);
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_Close: Unable to Release RW memory for DSP %d!!!", dspIndex));
            errCode = BERR_TRACE(errCode);
        }

        errCode = BDSP_Arm_P_DeviceInterruptUninstall((void *)pDevice, dspIndex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_Close: Unable to Un-Install Interrupt callbacks for DSP %d", dspIndex));
            errCode = BERR_TRACE(errCode);
        }

        BDSP_MMA_P_FreeMemory(&(pDevice->memInfo.sRWMemoryPool[dspIndex].Memory));
    }

    BDSP_MMA_P_FreeMemory(&(pDevice->memInfo.sROMemoryPool.Memory));

    BDSP_Arm_P_DeInitDevice(pDevice);

    /* Invalidate and free the device structure */
    BDBG_OBJECT_DESTROY(pDevice, BDSP_Arm);
    BKNI_Free(pDevice);

    BDBG_LEAVE(BDSP_Arm_P_Close);
}

void BDSP_Arm_P_GetDefaultContextSettings(
	void 					   *pDeviceHandle,
    BDSP_ContextType 			contextType,
    BDSP_ContextCreateSettings *pSettings
)
{
	BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;

	BDBG_ENTER(BDSP_Arm_P_GetDefaultContextSettings);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if (contextType == BDSP_ContextType_eAudio)
    {
        pSettings->maxTasks    		 = BDSP_MAX_FW_TASK_PER_AUDIO_CTXT;
        pSettings->contextType 		 = contextType;
        pSettings->maxBranch   		 = BDSP_MAX_BRANCH;
        pSettings->maxStagePerBranch = BDSP_MAX_STAGE_PER_BRANCH;
    }
	else if(contextType == BDSP_ContextType_eVideoEncode)
	{
		/* Added to beat the context create in playback application.
		     Load appropriate values when Video encode is supported*/
        pSettings->maxTasks 		 = 1;
        pSettings->contextType		 = contextType;
        pSettings->maxBranch 		 = 1;
        pSettings->maxStagePerBranch = 1;
	}
    else
    {
        BDBG_ERR(("BDSP_Arm_P_GetDefaultContextSettings: Trying to create a Context(%d) other that Audio on RAAGA which is not supported!!!!!",contextType));
    }

	BDBG_LEAVE(BDSP_Arm_P_GetDefaultContextSettings);
}

BERR_Code BDSP_Arm_P_CreateContext(
	void 							 *pDeviceHandle,
	const BDSP_ContextCreateSettings *pSettings,
	BDSP_ContextHandle 				 *pContextHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
	BDSP_ArmContext *pArmContext;

	BDBG_ENTER(BDSP_Arm_P_CreateContext);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	*pContextHandle = NULL;
	pArmContext = (BDSP_ArmContext *)BKNI_Malloc(sizeof(BDSP_ArmContext));
	if(NULL == pArmContext)
	{
		BDBG_ERR(("BDSP_Arm_P_CreateContext: Unable to allocate Memory for Creating the Context"));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto end;
	}
	BKNI_Memset(pArmContext, 0, sizeof(*pArmContext));
	BDBG_OBJECT_SET(pArmContext, BDSP_ArmContext);
	pArmContext->pDevice  = pDevice;
	pArmContext->settings = *pSettings;

	BDSP_P_InitContext(&pArmContext->context, pArmContext);

	pArmContext->context.destroy = BDSP_Arm_P_DestroyContext;
	pArmContext->context.getDefaultTaskSettings = BDSP_Arm_P_GetDefaultTaskSettings;
	pArmContext->context.createTask  = BDSP_Arm_P_CreateTask;
	pArmContext->context.getDefaultStageCreateSettings = BDSP_Arm_P_GetDefaultStageCreateSettings;
	pArmContext->context.createStage = BDSP_Arm_P_CreateStage;

	pArmContext->context.createInterTaskBuffer = BDSP_Arm_P_InterTaskBuffer_Create;
	pArmContext->context.getDefaultQueueSettings = /*BDSP_Arm_P_GetDefaultCreateQueueSettings*/NULL;
	pArmContext->context.createQueue = /*BDSP_Arm_P_Queue_Create*/NULL;

	pArmContext->context.getInterruptHandlers = BDSP_Arm_P_GetContextInterruptHandlers;
	pArmContext->context.setInterruptHandlers= BDSP_Arm_P_SetContextInterruptHandlers;
	pArmContext->context.processWatchdogInterrupt= BDSP_Arm_P_ProcessContextWatchdogInterrupt;
	pArmContext->context.createCapture = NULL;

	BKNI_AcquireMutex(pDevice->deviceMutex);
	BLST_S_INSERT_HEAD(&pDevice->contextList, pArmContext, node);
	BLST_S_INIT(&pArmContext->taskList);
	*pContextHandle= &pArmContext->context;
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	goto end;

end:
	BDBG_LEAVE( BDSP_Arm_P_CreateContext );
	return errCode;
}

void BDSP_Arm_P_DestroyContext(
	void *pContextHandle
)
{
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
	BDSP_Arm *pDevice;
	BDSP_ArmTask *pArmTask;

	BDBG_ENTER(BDSP_Arm_P_DestroyContext);
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
	pDevice = (BDSP_Arm *)pArmContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

	while((pArmTask = BLST_S_FIRST(&pArmContext->taskList)))
	{
		BDSP_Arm_P_DestroyTask((void *)pArmTask);
	}

	BKNI_AcquireMutex(pDevice->deviceMutex);
	BLST_S_REMOVE(&pDevice->contextList, pArmContext, BDSP_ArmContext, node);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_OBJECT_DESTROY(pArmContext, BDSP_ArmContext);
	BKNI_Free(pArmContext);
	BDBG_LEAVE(BDSP_Arm_P_DestroyContext);
}

void BDSP_Arm_P_GetDefaultTaskSettings(
	void *pContextHandle,
	BDSP_TaskCreateSettings *pSettings
)
{
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;

	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
	BKNI_Memset(pSettings, 0, sizeof(*pSettings));
	pSettings->masterTask = false;
}

BERR_Code BDSP_Arm_P_CreateTask(
	void *pContextHandle,
	const BDSP_TaskCreateSettings *pSettings,
	BDSP_TaskHandle *pTaskHandle
)
{
    BERR_Code   errCode = BERR_SUCCESS;
    BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
    BDSP_ArmTask *pArmTask;
    BDSP_Arm  *pDevice;
    unsigned MemoryRequired=0;

    BDBG_ENTER(BDSP_Arm_P_CreateTask);
    BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
    pDevice = (BDSP_Arm *)pArmContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    *pTaskHandle = NULL;

    pArmTask = (BDSP_ArmTask *)BKNI_Malloc(sizeof(BDSP_ArmTask));
    if(NULL == pArmTask)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to allocate Memory for Creating the Task"));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ASSERT(0);
    }
    BKNI_Memset(pArmTask, 0, sizeof(BDSP_ArmTask));
    BDBG_OBJECT_SET(pArmTask, BDSP_ArmTask);
    pArmTask->pContext = pArmContext;
    pArmTask->createSettings = *pSettings;

    BDSP_P_InitTask(&pArmTask->task, pArmTask);

    pArmTask->task.destroy = BDSP_Arm_P_DestroyTask;
    pArmTask->task.getDefaultTaskStartSettings = BDSP_Arm_P_GetDefaultTaskStartSettings;
    pArmTask->task.start = BDSP_Arm_P_StartTask;
    pArmTask->task.stop = BDSP_Arm_P_StopTask;
    pArmTask->task.retreiveGateOpenSettings = NULL;
    if (pArmContext->settings.contextType == BDSP_ContextType_eAudio)
    {
        pArmTask->task.pause  = BDSP_Arm_P_Pause;
        pArmTask->task.resume = BDSP_Arm_P_Resume;
        pArmTask->task.advance= BDSP_Arm_P_Advance;
        pArmTask->task.getAudioInterruptHandlers_isr = BDSP_Arm_P_GetTaskInterruptHandlers_isr;
        pArmTask->task.setAudioInterruptHandlers_isr = BDSP_Arm_P_SetTaskInterruptHandlers_isr;
        pArmTask->task.audioGapFill = BDSP_Arm_P_AudioGapFill;
        pArmTask->task.freeze   = BDSP_Arm_P_Freeze;
        pArmTask->task.unfreeze = BDSP_Arm_P_UnFreeze;
    }
    else
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Trying to create a Task other than Audio"));
    }

    errCode = BDSP_Arm_P_InitAtTaskCreate(pArmTask);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to Initialise Parameters for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BDBG_ASSERT(0);
    }

    BDSP_Arm_P_CalculateTaskMemory(&MemoryRequired);
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
                            MemoryRequired,
                            &(pArmTask->taskMemInfo.sMemoryPool.Memory),
                            BDSP_MMA_Alignment_4KByte);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to Allocate Memory for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BDBG_ASSERT(0);
    }
    pArmTask->taskMemInfo.sMemoryPool.ui32Size     = MemoryRequired;
    pArmTask->taskMemInfo.sMemoryPool.ui32UsedSize = 0;

    errCode = BDSP_Arm_P_AssignTaskMemory((void *)pArmTask);
    if (errCode != BERR_SUCCESS )
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to assign Task memory for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ASSERT(0);
    }

    errCode = BDSP_P_CreateMsgQueue(
            &pArmTask->taskMemInfo.syncQueueParams,
            pDevice->regHandle,
            0,
            &pArmTask->hSyncQueue);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to Create Sync Resp Queue for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BDBG_ASSERT(0);
    }

    errCode = BDSP_P_CreateMsgQueue(
            &pArmTask->taskMemInfo.asyncQueueParams,
            pDevice->regHandle,
            0,
            &pArmTask->hAsyncQueue);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to Create ASync Resp Queue for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        BDBG_ASSERT(0);
    }

#if 0
    errCode = BDSP_Arm_P_TaskInterruptInstall((void *)pArmTask);
    if ( BERR_SUCCESS!= errCode )
    {
        BDBG_ERR(("BDSP_Arm_P_CreateTask: Unable to Install Interrupt for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        BDBG_ASSERT(0);
    }
#else
    BDBG_ERR(("BDSP_Arm_P_CreateTask: Installation of Interrupt for Task, not completed"));
#endif

    BKNI_AcquireMutex(pDevice->deviceMutex);
    BLST_S_INSERT_HEAD(&pArmContext->taskList, pArmTask, node);
    BKNI_ReleaseMutex(pDevice->deviceMutex);

    *pTaskHandle = &pArmTask->task;

    BDBG_LEAVE(BDSP_Arm_P_CreateTask);
    return errCode;
}


void BDSP_Arm_P_DestroyTask(
	void *pTaskHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask =(BDSP_ArmTask *)pTaskHandle;

    BDBG_ENTER(BDSP_Arm_P_DestroyTask);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

    if(pArmTask->taskParams.isRunning == true)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: Task (%d) is still running, Stopping it by force",pArmTask->taskParams.taskId));
        BDSP_Arm_P_StopTask(pTaskHandle);
    }

#if 0
    errCode = BDSP_Arm_P_TaskInterruptUninstall(pTaskHandle);
    if ( BERR_SUCCESS!=errCode )
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: Unable to Un-Install Interrupt for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        BDBG_ASSERT(0);
    }
#else
    BDBG_ERR(("BDSP_Arm_P_DestroyTask: Un-Installation of Interrupt for Task, not completed"));
#endif
    errCode = BDSP_P_DestroyMsgQueue(pArmTask->hSyncQueue);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: SYNC queue destroy failed for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
    }

    errCode = BDSP_P_DestroyMsgQueue(pArmTask->hAsyncQueue);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: ASYNC queue destroy failed for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
    }

    errCode = BDSP_Arm_P_ReleaseTaskMemory((void *)pArmTask);
    if ( BERR_SUCCESS !=errCode )
    {
        BDBG_ERR(("BDSP_Arm_P_DestroyTask: Unable to Release Memory for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
    }
    BDSP_MMA_P_FreeMemory(&pArmTask->taskMemInfo.sMemoryPool.Memory);

    BDSP_Arm_P_UnInitAtTaskDestroy(pArmTask);

    BDBG_OBJECT_DESTROY(pArmTask, BDSP_ArmTask);
    BKNI_Free(pArmTask);

    BDBG_LEAVE(BDSP_Arm_P_DestroyTask);
}


void BDSP_Arm_P_GetDefaultTaskStartSettings(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pSettings    /* [out] */
)
{

	BDSP_ArmTask *pArmTask;
	pArmTask = (BDSP_ArmTask *)pTaskHandle;

	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
	BDBG_ASSERT(NULL != pSettings);

	BKNI_Memset((void *)pSettings,0,sizeof(BDSP_TaskStartSettings));

	pSettings->primaryStage   = NULL;
	pSettings->schedulingMode = BDSP_TaskSchedulingMode_eStandalone;
	pSettings->realtimeMode   = BDSP_TaskRealtimeMode_eRealTime;
	pSettings->masterTask     = NULL;
	pSettings->audioTaskDelayMode = BDSP_AudioTaskDelayMode_eDefault;
	pSettings->timeBaseType       = BDSP_AF_P_TimeBaseType_e45Khz;
	pSettings->ppmCorrection      = false;
	pSettings->openGateAtStart    = false;
	pSettings->stcIncrementConfig.enableStcTrigger = false;
	pSettings->extInterruptConfig.enableInterrupts = false;
	pSettings->eZeroPhaseCorrEnable=true; /*Enable Zero Phase Correction in default settings*/
	pSettings->pSampleRateMap      = NULL;
	pSettings->maxIndependentDelay = BDSP_MAX_INDEPENDENT_DELAY_IN_MS;
	pSettings->gateOpenReqd        = true;
}

BERR_Code BDSP_Arm_P_StartTask(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pStartSettings
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
    BDSP_ArmStage *pPrimaryStage;
    BDSP_Arm      *pDevice;
    unsigned dspIndex =0;
    BDSP_P_StartTaskCommand sPayload;

    BDBG_ENTER(BDSP_Arm_P_StartTask);
    BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
    dspIndex = pArmTask->createSettings.dspIndex;
    pPrimaryStage = (BDSP_ArmStage *)pStartSettings->primaryStage->pStageHandle;
    BDBG_OBJECT_ASSERT(pPrimaryStage, BDSP_ArmStage);
    pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);

    BKNI_Memset(&sPayload,0,sizeof(BDSP_P_StartTaskCommand));

    BDBG_MSG(("Start task (%d) on DSP %d",pArmTask->taskParams.taskId, dspIndex));

    errCode = BDSP_Arm_P_InitAtStartTask(pArmTask, pStartSettings);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Unable to Initialise the Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_InitMsgQueue(pArmTask->hSyncQueue, pDevice->memInfo.softFifo[dspIndex].Buffer);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Sync Queue Init failed for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_InitMsgQueue(pArmTask->hAsyncQueue, pDevice->memInfo.softFifo[dspIndex].Buffer);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: ASync Queue Init failed for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    if(pArmTask->pContext->pDevice->codeInfo.preloadImages == false)
    {
        errCode = BDSP_Arm_P_DownloadRuntimeAlgorithm(pArmTask);
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("BDSP_Arm_P_StartTask: Error in Runtime Algorithm Download for Task %d",pArmTask->taskParams.taskId));
            errCode = BERR_TRACE(errCode);
            goto end;
        }
    }

    errCode = BDSP_Arm_P_InitInterframe(pPrimaryStage);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Decode the Interframe for all stage for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Arm_P_GenCit(pArmTask);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Error in Forming CIT Network for Task %d",pArmTask->taskParams.taskId));
        errCode =BERR_TRACE(errCode);
        goto end;
    }

	errCode = BDSP_P_PopulateSchedulingInfo(
			&pArmTask->startSettings,
			pArmTask->pContext->settings.contextType,
			&pArmTask->pContext->pDevice->systemSchedulingInfo,
			&sPayload);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Unable to populate Scheduling Info for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto end;
    }
    sPayload.ui32TaskId          = pArmTask->taskParams.taskId;
    sPayload.ui32MasterTaskId    = pArmTask->taskParams.masterTaskId;
    sPayload.ui32SyncQueueFifoId = pArmTask->hSyncQueue->ui32FifoId;
    sPayload.ui32AsyncQueueFifoId= pArmTask->hAsyncQueue->ui32FifoId;
    sPayload.ui32EventEnableMask = pArmTask->taskParams.eventEnabledMask;
    sPayload.sTaskMemoryInfo.BaseAddr   = pArmTask->taskMemInfo.sMemoryPool.Memory.offset;
    sPayload.sTaskMemoryInfo.Size       = pArmTask->taskMemInfo.sMemoryPool.ui32Size;
    sPayload.sSharedMemoryInfo.BaseAddr = pArmTask->taskMemInfo.sMPSharedMemory.Buffer.offset;
    sPayload.sSharedMemoryInfo.Size     = pArmTask->taskMemInfo.sMPSharedMemory.ui32Size;
    sPayload.sConfigMemoryInfo.BaseAddr = pArmTask->taskMemInfo.sCITMemory.Buffer.offset;
    sPayload.sConfigMemoryInfo.Size     = pArmTask->taskMemInfo.sCITMemory.ui32Size;
    sPayload.sPrimaryStageMemoryInfo.BaseAddr   = pPrimaryStage->stageMemInfo.sMemoryPool.Memory.offset;
    sPayload.sPrimaryStageMemoryInfo.Size       = pPrimaryStage->stageMemInfo.sMemoryPool.ui32Size;

    errCode = BDSP_Arm_P_ProcessStartTaskCommand(pArmTask, &sPayload);
    if (BERR_SUCCESS != errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_StartTask: Error in Start Task command processing for Task %d",pArmTask->taskParams.taskId));
        errCode = BERR_TRACE(errCode);
        goto err_start_task;
    }

    pArmTask->taskParams.isRunning   = true;
    pArmTask->taskParams.lastCommand = BDSP_P_CommandID_START_TASK;
    goto end;

err_start_task:
    BDSP_Arm_P_UnInitAtStopTask(pArmTask);
end:
    BDBG_LEAVE(BDSP_Arm_P_StartTask);
    return errCode;
}

BERR_Code BDSP_Arm_P_StopTask(
	void *pTaskHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Arm_P_StopTask);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if(pArmTask->pContext->contextWatchdogFlag == false)
	{
		errCode = BDSP_Arm_P_ProcessStopTaskCommand(pArmTask);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Arm_P_StopTask: Error in Stop Task command processing for Task %d",pArmTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

	errCode = BDSP_Arm_P_CleanupCit(pArmTask);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Arm_P_StopTask: Error in Cleanup of CIT for Task %d",pArmTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	if(pArmTask->pContext->pDevice->codeInfo.preloadImages == false)
	{
		errCode = BDSP_Arm_P_ReleaseRuntimeAlgorithm(pArmTask);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Arm_P_StopTask: Error in Releasing Algorithms for Task %d",pArmTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}
	BDSP_Arm_P_UnInitAtStopTask(pArmTask);

end:
	BDBG_LEAVE(BDSP_Arm_P_StopTask);
	return errCode;
}

void BDSP_Arm_P_GetDefaultStageCreateSettings(
	BDSP_AlgorithmType algoType,
	BDSP_StageCreateSettings *pSettings /* [out] */
)
{
	unsigned i;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;

	BDBG_ENTER(BDSP_Arm_P_GetDefaultStageCreateSettings);
	BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(algoType < BDSP_AlgorithmType_eMax);

	pSettings->algoType = algoType;
	for (i = 0; i < BDSP_Algorithm_eMax; i++)
	{
		pAlgoSupportInfo = BDSP_Arm_P_LookupAlgorithmSupportInfo(i);
		pAlgoInfo = BDSP_P_LookupAlgorithmInfo(i);
		if (algoType == pAlgoInfo->type)
		{
			pSettings->algorithmSupported[i] = pAlgoSupportInfo->supported;
		}
		else
		{
			pSettings->algorithmSupported[i] = false;
		}
	}

	switch (algoType)
	{
		case BDSP_AlgorithmType_eAudioDecode:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 3;
			break;
		case BDSP_AlgorithmType_eAudioPassthrough:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioEncode:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioMixer:
			pSettings->maxInputs = 3;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioEchoCanceller:
			pSettings->maxInputs = 2;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioProcessing:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 2;
			break;
		default:
			BDBG_ERR(("AlgoType is not yet supported in the system"));
			break;
	}
	BDBG_LEAVE(BDSP_Arm_P_GetDefaultStageCreateSettings);
}

BERR_Code BDSP_Arm_P_CreateStage(
	void *pContextHandle,
	const BDSP_StageCreateSettings *pSettings,
	BDSP_StageHandle *pStageHandle /* [out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage;
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
	BDSP_Arm	*pDevice;
	unsigned    MemoryRequired =0, i=0;

	BDBG_ENTER(BDSP_Arm_P_CreateStage);
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);
	pDevice = (BDSP_Arm *)pArmContext->pDevice;
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
	pArmStage = BKNI_Malloc(sizeof(BDSP_ArmStage));
	if ( NULL == pArmStage )
	{
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ASSERT(0);
	}
	BKNI_Memset(pArmStage, 0, sizeof(*pArmStage));
	BDBG_OBJECT_SET(pArmStage, BDSP_ArmStage);

	pArmStage->eAlgorithm = BDSP_Algorithm_eMax;
	pArmStage->settings   = *pSettings;
	pArmStage->pContext   = pArmContext;
	pArmStage->pArmTask   = NULL;
	pArmStage->running    = false;
	pArmStage->stageID    = 0;

	BDSP_P_InitStage(&pArmStage->stage, pArmStage);

	/* Initialize the stage apis */
	pArmStage->stage.destroy = BDSP_Arm_P_DestroyStage;
	pArmStage->stage.setAlgorithm = BDSP_Arm_P_SetAlgorithm;
	pArmStage->stage.getStageSettings = BDSP_Arm_P_GetStageSettings;
	pArmStage->stage.setStageSettings = BDSP_Arm_P_SetStageSettings;
	pArmStage->stage.getStageStatus = BDSP_Arm_P_GetStageStatus;

	pArmStage->stage.getTsmSettings_isr = BDSP_Arm_P_GetTsmSettings_isr;
	pArmStage->stage.setTsmSettings_isr = BDSP_Arm_P_SetTsmSettings_isr;
	pArmStage->stage.getTsmStatus_isr = BDSP_Arm_P_GetTsmStatus_isr;

	pArmStage->stage.getDatasyncSettings = BDSP_Arm_P_GetDatasyncSettings;
	pArmStage->stage.getDatasyncSettings_isr = BDSP_Arm_P_GetDatasyncSettings_isr;
	pArmStage->stage.setDatasyncSettings = BDSP_Arm_P_SetDatasyncSettings;
	pArmStage->stage.getDatasyncStatus_isr = BDSP_Arm_P_GetDatasyncStatus_isr;
    pArmStage->stage.getAudioDelay_isrsafe = BDSP_Arm_P_GetAudioDelay_isrsafe;

	pArmStage->stage.addOutputStage = BDSP_Arm_P_AddOutputStage;

	pArmStage->stage.addFmmOutput = BDSP_Arm_P_AddFmmOutput;
	pArmStage->stage.addFmmInput = BDSP_Arm_P_AddFmmInput;

	pArmStage->stage.addRaveOutput = BDSP_Arm_P_AddRaveOutput;
	pArmStage->stage.addRaveInput = BDSP_Arm_P_AddRaveInput;

	pArmStage->stage.addInterTaskBufferOutput = BDSP_Arm_P_AddInterTaskBufferOutput;
	pArmStage->stage.addInterTaskBufferInput = BDSP_Arm_P_AddInterTaskBufferInput;
#if 0
	pArmStage->stage.addQueueOutput = BDSP_Arm_P_AddQueueOutput;
#if !B_REFSW_MINIMAL
	pArmStage->stage.addQueueInput = BDSP_Arm_P_AddQueueInput;
#endif /*!B_REFSW_MINIMAL*/
#endif
	pArmStage->stage.removeAllOutputs = BDSP_Arm_P_RemoveAllOutputs;
#if !B_REFSW_MINIMAL
	pArmStage->stage.removeOutput = BDSP_Arm_P_RemoveOutput;
#endif /*!B_REFSW_MINIMAL*/

	pArmStage->stage.removeInput = BDSP_Arm_P_RemoveInput;
	pArmStage->stage.removeAllInputs = BDSP_Arm_P_RemoveAllInputs;

	if (BDSP_ContextType_eVideoEncode == pArmContext->settings.contextType)
	{
		pArmStage->stage.getVideoEncodeDatasyncSettings = NULL;
		pArmStage->stage.setVideoEncodeDatasyncSettings = NULL;
	}

	BDSP_Arm_P_CalculateStageMemory(&MemoryRequired, pSettings->algoType, false, NULL);
	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							MemoryRequired,
							&(pArmStage->stageMemInfo.sMemoryPool.Memory),
							BDSP_MMA_Alignment_4KByte);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_CreateStage: Unable to Allocate Memory for Stage %d",pArmStage->eAlgorithm));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}
	pArmStage->stageMemInfo.sMemoryPool.ui32Size 	 = MemoryRequired;
	pArmStage->stageMemInfo.sMemoryPool.ui32UsedSize = 0;

	for (i=0; i<BDSP_Algorithm_eMax; i++)
	{
		if (pSettings->algorithmSupported[i])
		{
			errCode = BDSP_Arm_P_SetAlgorithm((void *)pArmStage, i);
			break;
		}
	}
	if (errCode != BERR_SUCCESS)
	{
		errCode = BERR_TRACE(errCode);
		goto err_set_algorithm;
	}

	*pStageHandle=&pArmStage->stage;
	goto end;

err_set_algorithm:
	BDSP_MMA_P_FreeMemory(&pArmStage->stageMemInfo.sMemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pArmStage, BDSP_ArmStage);
	BKNI_Free(pArmStage);
end:
	BDBG_LEAVE(BDSP_Arm_P_CreateStage);
	return errCode;
}

void BDSP_Arm_P_DestroyStage(
	void *pStageHandle
)
{
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	if(pArmStage->running)
	{
		BDBG_ERR(("BDSP_Arm_P_DestroyStage: Trying to Destroy a Running stage(%d) on Task (%d)",
			pArmStage->eAlgorithm, pArmStage->pArmTask->taskParams.taskId));
		BDBG_ASSERT(0);
	}

	BDSP_MMA_P_FreeMemory(&pArmStage->stageMemInfo.sMemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pArmStage, BDSP_ArmStage);
	BKNI_Free(pArmStage);
}

/***********************************************************************
Name        :   BDSP_Arm_P_GetAlgorithmInfo

Type        :   PI Interface

Input       :   algorithm -The algorithm for the which the data is requested by PI.
				pInfo - Pointer where the specific data releated to algorithm is returned back to the PI.

Return      :   None

Functionality   :   Returns the following data back to the PI.
	1)  Name of the algorithm.
	2)  Whether the algorithm is supported or not.
	3)  To which category the algothm belongs to.
	4)  The size of the User Configuration required by the algorithm.
	5)  The size of the Status Buffer required by the algorithm.
***********************************************************************/
void BDSP_Arm_P_GetAlgorithmInfo(
	BDSP_Algorithm algorithm,
	BDSP_AlgorithmInfo *pInfo /* [out] */
)
{
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;

	BDBG_ENTER(BDSP_Arm_P_GetAlgorithmInfo);
	BDBG_ASSERT(pInfo);

	pAlgoSupportInfo = BDSP_Arm_P_LookupAlgorithmSupportInfo(algorithm);
	pAlgoInfo = BDSP_P_LookupAlgorithmInfo(algorithm);
	pInfo->supported = pAlgoSupportInfo->supported;
	pInfo->pName     = pAlgoSupportInfo->pName;
	if(pAlgoSupportInfo->supported)
	{
		pInfo->type         = pAlgoInfo->type;
		pInfo->settingsSize = pAlgoInfo->algoUserConfigSize;
		pInfo->statusSize   = pAlgoInfo->algoStatusBufferSize;
	}
	else
	{
		pInfo->type         = BDSP_AlgorithmType_eMax;
		pInfo->settingsSize = 0;
		pInfo->statusSize   = 0;
	}

	BDBG_LEAVE(BDSP_Arm_P_GetAlgorithmInfo);
}

BERR_Code BDSP_Arm_P_SetAlgorithm(
	void *pStageHandle,
	BDSP_Algorithm algorithm
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pCurrAlgoInfo, *pAlgoInfo;
	bool valid = false;
	unsigned index = 0;

	BDBG_ENTER(BDSP_Arm_P_SetAlgorithm);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_MSG(("Setting Algo (%d) for Stage (%p)", algorithm, (void *)pArmStage));

	pAlgoInfo = BDSP_P_LookupAlgorithmInfo(algorithm);
	pCurrAlgoInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);
	/*  Return error if the stage is running.*/
	if (pArmStage->running)
	{
		BDBG_ERR(("Cannot set algorithm when the stage is running : stage handle = 0x%p : Current algo = %s",
				   pStageHandle, pCurrAlgoInfo->pName));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	if (pArmStage->totalOutputs)
	{
		BDBG_ERR(("Stage has non-zero (%d) output connections at set algorithm", pArmStage->totalOutputs));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if (pArmStage->totalInputs)
	{
		BDBG_ERR(("Stage has non-zero (%d) input connections at set algorithm", pArmStage->totalInputs));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if(!pArmStage->settings.algorithmSupported[algorithm])
	{
		BDBG_ERR((" algorithm %s (%d) being passed in %s which was not enabled during CreateStage call ",
					pAlgoInfo->pName,algorithm, BSTD_FUNCTION));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/*  Return error if there is an attempt to change the stage type */
	if((pCurrAlgoInfo->type == BDSP_AlgorithmType_eAudioDecode)||
		(pCurrAlgoInfo->type == BDSP_AlgorithmType_eAudioPassthrough))
	{
		switch (pAlgoInfo->type)
		{
			case BDSP_AlgorithmType_eAudioDecode:
			case BDSP_AlgorithmType_eAudioPassthrough:
				valid = true;
				break;
			default:
				valid = false;
				break;
		}
	}
	else if((pCurrAlgoInfo->type == BDSP_AlgorithmType_eMax)&&
			 (pAlgoInfo->type != BDSP_AlgorithmType_eMax))
	{
		valid = true;
	}
	else
	{
		valid = (pArmStage->settings.algoType == pAlgoInfo->type) ? true : false;
	}
	if(!valid)
	{
		BDBG_ERR(("Cannot change the algo type of the stage from %d (%s) to %d (%s)",
					pCurrAlgoInfo->type, pCurrAlgoInfo->pName, pAlgoInfo->type, pAlgoInfo->pName));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	pArmStage->eAlgorithm = pAlgoInfo->algorithm;
	if(pArmStage->stageMemInfo.sMemoryPool.ui32UsedSize != 0)
	{
		errCode = BDSP_Arm_P_ReleaseStageMemory((void *)pArmStage);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_SetAlgorithm: Unable to Release memory for Stage(%p)", (void *)pArmStage));
			goto end;
		}
	}
	errCode = BDSP_Arm_P_AssignStageMemory((void *)pArmStage);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_SetAlgorithm: Unable to Assign memory for algorithm(%d) %s", algorithm, pAlgoInfo->pName));
		goto end;
	}

	/* Reset the Algorithm User Config */
	if( pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size)
	{
		BKNI_Memcpy(pArmStage->stageMemInfo.sAlgoUserConfig.Buffer.pAddr,
			(void *)pAlgoInfo->pDefaultUserConfig,
			pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size);
		BDSP_MMA_P_FlushCache(pArmStage->stageMemInfo.sAlgoUserConfig.Buffer, pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size);

		BKNI_Memcpy(pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer.pAddr,
			(void *)pAlgoInfo->pDefaultUserConfig,
			pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size);
		BDSP_MMA_P_FlushCache(pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer, pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size);
	}

	/* Reset the Algorithm Status buffer */
	if( pArmStage->stageMemInfo.sAlgoStatus.ui32Size)
	{
		BKNI_Memset(pArmStage->stageMemInfo.sAlgoStatus.Buffer.pAddr, 0xFF, pArmStage->stageMemInfo.sAlgoStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pArmStage->stageMemInfo.sAlgoStatus.Buffer, pArmStage->stageMemInfo.sAlgoStatus.ui32Size);
	}

	/* Reset the IDS Status buffer */
	if( pArmStage->stageMemInfo.sIdsStatus.ui32Size)
	{
		BKNI_Memset(pArmStage->stageMemInfo.sIdsStatus.Buffer.pAddr, 0xFF, pArmStage->stageMemInfo.sIdsStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pArmStage->stageMemInfo.sIdsStatus.Buffer, pArmStage->stageMemInfo.sIdsStatus.ui32Size);
	}

	/* Reset the TSM Status buffer */
	if( pArmStage->stageMemInfo.sTsmStatus.ui32Size)
	{
		BKNI_Memset(pArmStage->stageMemInfo.sTsmStatus.Buffer.pAddr, 0xFF, pArmStage->stageMemInfo.sTsmStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pArmStage->stageMemInfo.sTsmStatus.Buffer, pArmStage->stageMemInfo.sTsmStatus.ui32Size);
	}

	/* Load the Default TSM and DataSync settings for the Stage*/
	BDSP_MMA_P_CopyDataToDram(&pArmStage->stageMemInfo.sDataSyncSettings.Buffer,
		(void *)&BDSP_sDefaultFrameSyncSettings ,
		sizeof(BDSP_AudioTaskDatasyncSettings));

	BDSP_MMA_P_CopyDataToDram(&pArmStage->stageMemInfo.sTsmSettings.Buffer,
		(void *)&BDSP_sDefaultTSMSettings ,
		sizeof(BDSP_AudioTaskTsmSettings));

	/*Reset the InterStage Port Information*/
	for(index=0; index< BDSP_AF_P_MAX_OP_FORKS; index++)
	{
		pArmStage->sStageConnectionInfo.sInterStagePortInfo[index].branchFromPort = 0;
		pArmStage->sStageConnectionInfo.sInterStagePortInfo[index].tocIndex       = BDSP_AF_P_TOC_INVALID;
		pArmStage->sStageConnectionInfo.sInterStagePortInfo[index].ePortDataType  = BDSP_AF_P_DistinctOpType_eMax;
	}
end:
	BDBG_LEAVE(BDSP_Arm_P_SetAlgorithm);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetAudioDelay_isrsafe(
    BDSP_CTB_Input   *pCtbInput,
    void             *pStageHandle,
    BDSP_CTB_Output  *pCTBOutput
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);

    pCTBOutput->ui32Threshold = BDSP_AF_P_MAX_THRESHOLD+BDSP_AF_P_SAMPLE_PADDING;

    if(pCtbInput->realtimeMode != BDSP_TaskRealtimeMode_eNonRealTime)
    {
        if(pCtbInput->audioTaskDelayMode == BDSP_AudioTaskDelayMode_eDefault)
        {
            pCTBOutput->ui32AudOffset = BDSP_AF_P_MAX_AUD_OFFSET;
            pCTBOutput->ui32BlockTime = BDSP_AF_P_BLOCKING_TIME;
        }
        else
        {
            /* Low and lowest Delay mode are considered same */
            /*2*BT worst case decode time; AudOffset~DT= input buffer wait time */
            pCTBOutput->ui32AudOffset = BDSP_AF_P_MAX_AUD_OFFSET_LOW_DELAY;
            pCTBOutput->ui32BlockTime = BDSP_AF_P_BLOCKING_TIME_LOW_DELAY;
        }
    }
    else
    {
        pCTBOutput->ui32AudOffset = 0;
        pCTBOutput->ui32BlockTime = 0;
    }

	BSTD_UNUSED(pCtbInput);
    return errCode;
}

BERR_Code BDSP_Arm_P_GetStageSettings(
	void *pStageHandle,
	void *pSettingsBuffer,
	size_t settingsSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Arm_P_GetStageSettings);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);

    pInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);
    if (settingsSize != pInfo->algoUserConfigSize )
    {
        BDBG_ERR(("BDSP_Arm_P_GetStageSettings: Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsSize, (unsigned long)pInfo->algoUserConfigSize, pArmStage->eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

	BDSP_MMA_P_CopyDataFromDram(pSettingsBuffer, &pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer,settingsSize);

	BDBG_LEAVE(BDSP_Arm_P_GetStageSettings);
	return errCode;
}

BERR_Code BDSP_Arm_P_SetStageSettings(
	void *pStageHandle,
	const void *pSettingsBuffer,
	size_t settingsSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Arm_P_SetStageSettings);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);
	BDBG_ASSERT(pArmStage->eAlgorithm < BDSP_Algorithm_eMax);

    pInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);
    if (settingsSize != pInfo->algoUserConfigSize )
    {
        BDBG_ERR(("BDSP_Arm_P_SetStageSettings: Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsSize, (unsigned long)pInfo->algoUserConfigSize, pArmStage->eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

	BDSP_MMA_P_CopyDataToDram(&pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer, (void *)pSettingsBuffer, settingsSize);
	if(pArmStage->running)
	{
		BDSP_P_AlgoReconfigCommand sPayload;
		BDSP_ArmTask *pArmTask;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_AlgoReconfigCommand));
		pArmTask = pArmStage->pArmTask;

		sPayload.eAlgorithm = pArmStage->eAlgorithm;
		sPayload.sStageMemoryInfo.BaseAddr		= pArmStage->stageMemInfo.sMemoryPool.Memory.offset;
		sPayload.sStageMemoryInfo.Size     		= pArmStage->stageMemInfo.sMemoryPool.ui32Size;
		sPayload.sHostConfigMemoryInfo.BaseAddr = pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer.offset;
		sPayload.sHostConfigMemoryInfo.Size     = pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size;
		sPayload.sFwConfigMemoryInfo.BaseAddr 	= pArmStage->stageMemInfo.sAlgoUserConfig.Buffer.offset;
		sPayload.sFwConfigMemoryInfo.Size     	= pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size;

		errCode = BDSP_Arm_P_ProcessAlgoReconfigCommand(pArmTask, &sPayload);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Arm_P_SetStageSettings: Error in AlgoConfig Change command processing of Stage(%d) %s for Task %d",
				pArmStage->eAlgorithm, pInfo->pName, pArmTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}
	else
	{
		/* If the Stage is not running then copy into algo user config too to maintain consistency*/
		BDSP_MMA_P_CopyDataToDram(&pArmStage->stageMemInfo.sAlgoUserConfig.Buffer, (void *)pSettingsBuffer, settingsSize);
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_SetStageSettings);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetDatasyncSettings(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Arm_P_GetDatasyncSettings);
	BDBG_ASSERT(pStageHandle);
	BDBG_ASSERT(pSettingsBuffer);

	BKNI_EnterCriticalSection();
	errCode = BDSP_Arm_P_GetDatasyncSettings_isr(pStageHandle, pSettingsBuffer);
	BKNI_LeaveCriticalSection();
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_GetDatasyncSettings: Error in retreiving the settings"));
	}

	BDBG_LEAVE(BDSP_Arm_P_GetDatasyncSettings);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetDatasyncSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

	BDBG_ENTER(BDSP_Arm_P_GetDatasyncSettings_isr);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataFromDram_isr((void *)pSettingsBuffer, &pArmStage->stageMemInfo.sDataSyncSettings.Buffer, sizeof(BDSP_AudioTaskDatasyncSettings));

	BDBG_LEAVE(BDSP_Arm_P_GetDatasyncSettings_isr);
	return errCode;
}

BERR_Code BDSP_Arm_P_SetDatasyncSettings(
	void *pStageHandle,
	const BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

	BDBG_ENTER(BDSP_Arm_P_SetDatasyncSettings);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataToDram(&pArmStage->stageMemInfo.sDataSyncSettings.Buffer, (void *)pSettingsBuffer, sizeof(BDSP_AudioTaskDatasyncSettings));
	if(pArmStage->running)
	{
		BDSP_ArmTask  *pArmTask = pArmStage->pArmTask;
		BDSP_P_DataSyncReconfigCommand sPayload;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_DataSyncReconfigCommand));
		BKNI_Memcpy(&sPayload.sDataSyncSettings, pSettingsBuffer, sizeof(BDSP_AudioTaskDatasyncSettings));

		errCode = BDSP_Arm_P_ProcessDataSyncReconfigCommand(pArmTask, &sPayload);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Arm_P_SetDatasyncSettings: Error in DataSync ReConfig command processing for Task %d", pArmTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_SetDatasyncSettings);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetTsmSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmSettings  *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

	BDBG_ENTER(BDSP_Arm_P_GetTsmSettings_isr);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataFromDram_isr((void *)pSettingsBuffer, &pArmStage->stageMemInfo.sTsmSettings.Buffer, sizeof(BDSP_AudioTaskTsmSettings));

	BDBG_LEAVE(BDSP_Arm_P_GetTsmSettings_isr);
	return errCode;
}

BERR_Code BDSP_Arm_P_SetTsmSettings_isr(
	void *pStageHandle,
	const BDSP_AudioTaskTsmSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;

	BDBG_ENTER(BDSP_Arm_P_SetTsmSettings_isr);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataToDram_isr(&pArmStage->stageMemInfo.sTsmSettings.Buffer, (void *)pSettingsBuffer, sizeof(BDSP_AudioTaskTsmSettings));
	BDBG_MSG(("TSM RE-CONFIG"));
	BDSP_P_Analyse_CIT_TSMConfig_isr(pArmStage->stageMemInfo.sTsmSettings.Buffer);
	if(pArmStage->running)
	{
		BDSP_ArmTask  *pArmTask = pArmStage->pArmTask;
		BDSP_P_TsmReconfigCommand sPayload;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_TsmReconfigCommand));
		BKNI_Memcpy(&sPayload.sTsmSettings, pSettingsBuffer, sizeof(BDSP_AudioTaskTsmSettings));

		if(pArmTask->taskParams.isRunning == true)
		{
		errCode = BDSP_Arm_P_ProcessTsmReconfigCommand_isr(pArmTask, &sPayload);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Arm_P_SetTsmSettings_isr: Error in TSM Re-Config command processing for Task %d", pArmTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
        }
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_SetTsmSettings_isr);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetStageStatus(
	void *pStageHandle,
	void *pStatusBuffer,
	size_t statusSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	uint32_t *pStatusValid;

	BDBG_ENTER(BDSP_Arm_P_GetStageStatus);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pStatusBuffer);

    pAlgoInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);
    if (statusSize != pAlgoInfo->algoStatusBufferSize)
    {
        BDBG_ERR(("Status buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)statusSize, (unsigned long)pAlgoInfo->algoStatusBufferSize, pArmStage->eAlgorithm, pAlgoInfo->pName));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto end;
    }

	BDSP_MMA_P_CopyDataFromDram(pStatusBuffer, &pArmStage->stageMemInfo.sAlgoStatus.Buffer,statusSize);

    if (pAlgoInfo->statusValidOffset != 0xffffffff)
    {
        pStatusValid = (void *)((uint8_t *)pStatusBuffer + pAlgoInfo->statusValidOffset);
        if ( 0 != *pStatusValid )
        {
            BDBG_MSG(("Status buffer for algorithm %d (%s) marked invalid",pArmStage->eAlgorithm, pAlgoInfo->pName));
            errCode = BDSP_ERR_BAD_DEVICE_STATE;   /* BERR_TRACE intentionally omitted */
        }
    }

end:
	BDBG_LEAVE(BDSP_Arm_P_GetStageStatus);
	return errCode;
}

BERR_Code BDSP_Arm_P_GetDatasyncStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncStatus *pStatusBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Arm_P_GetDatasyncStatus_isr);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pStatusBuffer);

    pInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);

	BDSP_MMA_P_CopyDataFromDram_isr(pStatusBuffer, &pArmStage->stageMemInfo.sIdsStatus.Buffer, pInfo->idsStatusBufferSize);

	if (0 != pStatusBuffer->ui32StatusValid)
	{
		BDBG_MSG(("BDSP_Arm_P_GetDatasyncStatus_isr: Datasync Status buffer is not in valid status"));
		errCode = BDSP_ERR_BAD_DEVICE_STATE;
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_GetDatasyncStatus_isr);
	return errCode;
}

void BDSP_Arm_P_GetStatus(
    void *pDeviceHandle,
    BDSP_Status *pStatus
)
{
    BDSP_Arm *pDevice = (BDSP_Arm *)pDeviceHandle;
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Arm);
    pStatus->numDsp = pDevice->numDsp;
    pStatus->numWatchdogEvents=0;
    pStatus->firmwareVersion.majorVersion = BDSP_ARM_MAJOR_VERSION;
    pStatus->firmwareVersion.minorVersion = BDSP_ARM_MINOR_VERSION;
    pStatus->firmwareVersion.branchVersion = 0;
    pStatus->firmwareVersion.branchSubVersion = 0;
    return;
}

BERR_Code BDSP_Arm_P_GetTsmStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmStatus *pStatusBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Arm_P_GetTsmStatus_isr);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(pStatusBuffer);

    pInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);

	BDSP_MMA_P_CopyDataFromDram_isr(pStatusBuffer, &pArmStage->stageMemInfo.sTsmStatus.Buffer, pInfo->tsmStatusBufferSize);
	if (0 != pStatusBuffer->ui32StatusValid)
	{
		BDBG_MSG(("BDSP_Arm_P_GetTsmStatus_isr: TSM Status buffer is not in valid status"));
		errCode = BDSP_ERR_BAD_DEVICE_STATE;
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_GetTsmStatus_isr);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Pause

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Paused.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and not already paused.
		2)  Send the Pause Command to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Arm_P_Pause(
	void *pTaskHandle
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Arm_P_Pause);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_Pause: Task(%d) is not started yet. Ignoring the Pause command",pArmTask->taskParams.taskId));
	}
	else
	{
		if (pArmTask->taskParams.paused == true)
		{
			BDBG_WRN(("TBDSP_Arm_P_Pause: Task(%d) is already in Pause state. Ignoring the Pause command",pArmTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Arm_P_ProcessPauseCommand(pArmTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_Pause: PAUSE of Task (%d) failed", pArmTask->taskParams.taskId));
				goto end;
			}
			pArmTask->taskParams.paused = true;
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_Pause);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Resume

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Resumed.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in Paused state.
		2)  Send the Resume Command to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Arm_P_Resume(
	void *pTaskHandle
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Arm_P_Resume);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_Resume: Task(%d) is not started yet. Ignoring the Resume command",pArmTask->taskParams.taskId));
	}
	else
	{
		if (pArmTask->taskParams.paused == false)
		{
			BDBG_WRN(("BDSP_Arm_P_Resume: Task(%d) is already in Resume state. Ignoring the Resume command",pArmTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Arm_P_ProcessResumeCommand(pArmTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_Resume: PAUSE of Task (%d) failed",pArmTask->taskParams.taskId));
				goto end;
			}
			pArmTask->taskParams.paused = false;
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_Resume);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AudioGapFill

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be zero Gap fill.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		This command puts FW in zero fill mode. This is needed to handle NRT Xcode case. In NRT xcode, decoder
		has to trigger STC to move it forward and if there is no data in the input CDB, it is supposed to do nothing
		so if there is an audio gap, audio stalls and does not trigger STC which in turn stall video also by virtue of
		AV_WINDOW. This command will be called by upper layer when it detects such kind of gap in stream and
		this will make sure aduio fills zeroes and STC moves forward avoiding the deadlock.
***********************************************************************/
BERR_Code BDSP_Arm_P_AudioGapFill(
	void *pTaskHandle
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Arm_P_AudioGapFill);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_AudioGapFill: Task(%d) is not started yet. Ignoring the Audio Gap Fill command",pArmTask->taskParams.taskId));
	}
	else
	{
		if (pArmTask->taskParams.paused == true)
		{
			BDBG_WRN(("BDSP_Arm_P_AudioGapFill: Task(%d) is already in Paused state. Ignoring the Audio Gap Fill command",pArmTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Arm_P_ProcessAudioGapFillCommand(pArmTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_AudioGapFill: Audio Gap Fill of Task (%d) failed",pArmTask->taskParams.taskId));
				goto end;
			}
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_AudioGapFill);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Advance

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Advanced.
		     timeInMs      -   Time provided to advance in milli-seconds.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in Paused state.
		2)  Send the Advance Command to the FW with time and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Arm_P_Advance(
	void *pTaskHandle,
	unsigned timeInMs
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_FrameAdvanceCommand PayLoad;

	BDBG_ENTER(BDSP_Arm_P_Advance);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_Advance: Task(%d) is not started yet. Ignoring the Advance command",pArmTask->taskParams.taskId));
	}
	else
	{
		if (pArmTask->taskParams.paused == false)
		{
			BDBG_WRN(("BDSP_Arm_P_Advance: Task (%d) is not in Pause state. Ignoring the Advance command",pArmTask->taskParams.taskId));
		}
		else
		{
			PayLoad.ui32DurationOfFrameAdv = timeInMs * 45;
			errCode = BDSP_Arm_P_ProcessFrameAdvanceCommand(pArmTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_Advance: FRAME ADVANCE of Task (%d) failed",pArmTask->taskParams.taskId));
			}
		}
	}

	BDBG_LEAVE(BDSP_Arm_P_Advance);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_Freeze

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Freezed.
				pSettings       -   Setting to be used to Freeze

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and not in frozen state.
		2)  Retreive the STC address ( lower 32 bit) and provide it to FW for internal processing of STC snapshotting.
		3)  Send the Freeze Command with dummy FMM port details to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Arm_P_Freeze(
	void *pTaskHandle,
	const BDSP_AudioTaskFreezeSettings *pSettings
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_AudioOutputFreezeCommand PayLoad;

	BDBG_ENTER(BDSP_Arm_P_Freeze);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_Freeze: Task(%d) is not started yet. Ignoring the Audio Freeze command",pArmTask->taskParams.taskId));
	}
	else
	{
		if(pArmTask->taskParams.frozen == true)
		{
			BDBG_WRN(("BDSP_Arm_P_Freeze: Task (%d) is already in Frozen state. Ignoring the Audio Freeze command",pArmTask->taskParams.taskId));
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_Freeze: Implementation is not yet complete"));
			BDBG_MSG(("FMM OUTPUT ADDR = 0x%x",pSettings->fmmOutputAddress));
			BDBG_MSG(("FMM OUTPUT MASK = 0x%x",pSettings->fmmOutputMask));
			BDBG_MSG(("FMM OUTPUT VAL  = 0x%x",pSettings->fmmOutputValue));
			errCode = BDSP_Arm_P_ProcessAudioOutputFreezeCommand(pArmTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_Freeze: AUDIO FREEZE of Task (%d) failed",pArmTask->taskParams.taskId));
				goto end;
			}
			pArmTask->taskParams.frozen = true;
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_Freeze);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_UnFreeze

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Freezed.
				pSettings       -   Setting to be used to Un-Freeze

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in frozen state.
		2)  Retreive the STC address ( lower 32 bit) and provide it to FW for internal processing of STC snapshotting.
		3)  Send the UnFreeze Command with dummy FMM port details to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Arm_P_UnFreeze(
	void *pTaskHandle,
	const BDSP_AudioTaskUnFreezeSettings *pSettings
)
{
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_AudioOutputUnFreezeCommand PayLoad;

	BDBG_ENTER(BDSP_Arm_P_UnFreeze);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	if (pArmTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Arm_P_UnFreeze: Task(%d) is not started yet. Ignoring the Audio UnFreeze command",pArmTask->taskParams.taskId));
	}
	else
	{
		if(pArmTask->taskParams.frozen == false)
		{
			BDBG_WRN(("BDSP_Arm_P_UnFreeze: Task (%d) is already in running state. Ignoring the Audio UnFreeze command",pArmTask->taskParams.taskId));
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_UnFreeze: Implementation is not yet complete"));
			BDBG_MSG(("FMM OUTPUT ADDR = 0x%x",pSettings->fmmOutputAddress));
			BDBG_MSG(("FMM OUTPUT MASK = 0x%x",pSettings->fmmOutputMask));
			BDBG_MSG(("FMM OUTPUT VAL  = 0x%x",pSettings->fmmOutputValue));
			BDBG_MSG(("NUM BUFFERS     = %d",pSettings->ui32NumBuffers));
			errCode = BDSP_Arm_P_ProcessAudioOutputUnFreezeCommand(pArmTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Arm_P_UnFreeze: AUDIO UNFREEZE of Task (%d) failed",pArmTask->taskParams.taskId));
				goto end;
			}
			pArmTask->taskParams.frozen = false;
		}
	}

end:
	BDBG_LEAVE(BDSP_Arm_P_UnFreeze);
	return errCode;
}
