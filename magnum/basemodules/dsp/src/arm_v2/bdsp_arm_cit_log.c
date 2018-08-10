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

BDBG_MODULE(bdsp_cit);

void BDSP_Arm_P_Analyse_CIT(
	BDSP_ArmTask  *pArmTask,
	bool           CitReconfig
)
{
	unsigned NumStages = 0;
	unsigned index = 0;
	BDSP_AF_P_sTASK_CONFIG *pTaskConfig;
	BDSP_AF_P_sSTAGE_CONFIG *pStageConfig;

	BDBG_ENTER(BDSP_Arm_P_Analyse_CIT);

	if(CitReconfig)
	{
	    BDBG_MSG(("============================================ "));
		BDBG_MSG(("\t RE-CONFIG CIT FOR TASK %d ",pArmTask->taskParams.taskId));
	    BDBG_MSG(("============================================ "));
	}
	else
	{
	    BDBG_MSG(("============================================ "));
		BDBG_MSG(("\t\t   CIT FOR TASK %d ",pArmTask->taskParams.taskId));
	    BDBG_MSG(("============================================ "));
	}

    pTaskConfig = (BDSP_AF_P_sTASK_CONFIG *)pArmTask->taskMemInfo.sHostCITMemory.Buffer.pAddr;
	BDBG_MSG(("Running on DSP: %d ",pArmTask->createSettings.dspIndex));
	BDBG_MSG(("Running on CORE: %d ",pArmTask->taskParams.coreIndex));
	BDBG_MSG(("Context: %s ",ContextType[pArmTask->pContext->settings.contextType]));
	BDBG_MSG(("Scheduling Mode: %s ",SchedulingMode[pArmTask->startSettings.schedulingMode]));
	if(BDSP_TaskSchedulingMode_eSlave == pArmTask->startSettings.schedulingMode)
	{
		BDBG_MSG(("Master Task ID: %d ",pArmTask->taskParams.masterTaskId));
	}
	BDBG_MSG(("Scheduling Type: %s ",SchedulingType[pArmTask->startSettings.realtimeMode]));
	BDBG_MSG(("Delay Mode: %s ",DelayMode[pArmTask->startSettings.audioTaskDelayMode]));
	BDBG_MSG(("Zero Phase Correction: %s ",DisableEnable[pArmTask->startSettings.eZeroPhaseCorrEnable]));
	BDBG_MSG(("Gate Open Required: %s",DisableEnable[pArmTask->startSettings.gateOpenReqd]));
	BDBG_MSG(("Open Gate At Start: %s",DisableEnable[pArmTask->startSettings.openGateAtStart]));
	BDBG_MSG(("No of Dependent tasks for which Gate Open is required: %d",pArmTask->startSettings.DependentTaskInfo.numTasks));
	BDBG_MSG(("-------------------------------------------- "));

	/* Global Task Configuration */
	BDSP_Arm_P_Analyse_CIT_GlobalTask(pArmTask, &pTaskConfig->sGlobalTaskConfig);

	NumStages = pTaskConfig->sGlobalTaskConfig.ui32NumStagesInTask;
	BDBG_MSG(("Num of Stages: %d ",NumStages));
	BDBG_MSG(("-------------------------------------------- "));
    for(index=0; index<NumStages; index++)
    {
        pStageConfig = &pTaskConfig->sStageConfig[index];
        BDBG_MSG(("Configuration for Stage: %d ",index));
		BDBG_MSG(("Algorithm		  = %s ",Algorithm2Name[pStageConfig->eAlgorithm]));
		BDBG_MSG(("Stage Id 		  = %d ",pStageConfig->ui32StageId));
		if(index == 0)
		{
			BDSP_Arm_P_Analyse_CIT_PrimaryStage(&pTaskConfig->sPrimaryStageInfo);
		}
		BDSP_Arm_P_Analyse_CIT_Stage(pStageConfig, &pArmTask->pContext->pDevice->memInfo.DescriptorMemory[pArmTask->createSettings.dspIndex][0]);
		BDBG_MSG(("-------------------------------------------- "));
    }
	BDBG_MSG(("============================================ "));
	BDBG_LEAVE(BDSP_Arm_P_Analyse_CIT);
}
