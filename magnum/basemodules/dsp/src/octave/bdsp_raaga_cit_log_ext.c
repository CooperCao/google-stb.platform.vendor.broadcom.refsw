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

#include "bdsp_raaga_cit_log.h"

BDBG_MODULE(bdsp_ext);

static void BDSP_Raaga_P_Analyse_CIT_PortDetails(
	BDSP_AF_P_sIoPort *pPortDetails,
	BDSP_P_FwBuffer   *pDescriptorMemory
)
{
	unsigned i = 0;
	BDBG_MSG(("\t Port Type                = %s",PortType[pPortDetails->ePortType]));
	BDBG_MSG(("\t Distinct Output Type     = %s",DistinctOutputType[pPortDetails->ePortDataType]));
	BDBG_MSG(("\t Num Port Buffer          = %d",pPortDetails->ui32numPortBuffer));
	if(pPortDetails->ui32tocIndex != BDSP_AF_P_TOC_INVALID)
	{
		BDBG_MSG(("\t TOC Index                = %d",pPortDetails->ui32tocIndex));
	}
	if(pPortDetails->ui32numBranchfromPort != BDSP_AF_P_BRANCH_INVALID)
	{
		BDBG_MSG(("\t Branch from Port         = %d",pPortDetails->ui32numBranchfromPort));
	}
	if(pPortDetails->sIOGenericBuffer != 0)
	{
		BDBG_MSG(("\t IO GEN Buffer Descriptor = "BDSP_MSG_FMT,BDSP_MSG_ARG(pPortDetails->sIOGenericBuffer)));
	}
	BDSP_Raaga_P_Analyse_CIT_DataAccess(&pPortDetails->sDataAccessAttributes);
	for(i=0;i<pPortDetails->ui32numPortBuffer;i++)
	{
		unsigned j = 0;
		BDBG_MSG(("\t IO Buffer: Num Buffers   = %d",pPortDetails->sIoBuffer[i].ui32NumBuffer));
		BDBG_MSG(("\t IO Buffer: Buffer Type   = %s",BufferType[pPortDetails->sIoBuffer[i].eBufferType]));
		for(j=0; j<pPortDetails->sIoBuffer[i].ui32NumBuffer; j++)
		{
			BDSP_Raaga_P_Analyse_CIT_BufferDetails(pDescriptorMemory,
				pPortDetails->sIoBuffer[i].sCircularBuffer[j],
				j);
		}
	}
}
void BDSP_Raaga_P_Analyse_CIT_GlobalTask(
	BDSP_RaagaTask                  *pRaagaTask,
	BDSP_AF_P_sGLOBAL_TASK_CONFIG   *psGlobalTaskConfig
)
{
	BDSP_RaagaStage *pRaagaPrimaryStage;
	BDBG_ENTER(BDSP_Raaga_P_Analyse_CIT_GlobalTask);

	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

	BDBG_MSG(("Global Task Configuration"));
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("Number of Stages in Task = %d ",psGlobalTaskConfig->ui32NumStagesInTask));
	BDBG_MSG(("Scratch Size             = %d ",psGlobalTaskConfig->ui32ScratchSize));
	BDBG_MSG(("InterStage Size(PCM only)= %d ",psGlobalTaskConfig->ui32InterStageSize));
	BDBG_MSG(("DataSync Config Offset   = "BDSP_MSG_FMT", Size = %d",
		BDSP_MSG_ARG(psGlobalTaskConfig->sDataSyncUserConfigInfo.BaseAddr), psGlobalTaskConfig->sDataSyncUserConfigInfo.Size));

	BDBG_MSG(("TSM UserConfig Offset    = "BDSP_MSG_FMT", Size = %d",
		BDSP_MSG_ARG(psGlobalTaskConfig->sTsmConfigInfo.BaseAddr), psGlobalTaskConfig->sTsmConfigInfo.Size));
	BDSP_Raaga_P_Analyse_CIT_TSMConfig(pRaagaPrimaryStage->stageMemInfo.sTsmSettings.Buffer);

	BDBG_MSG(("GateOpen Config Offset   = "BDSP_MSG_FMT", Size = %d",
		BDSP_MSG_ARG(psGlobalTaskConfig->sGateOpenConfigInfo.BaseAddr), psGlobalTaskConfig->sGateOpenConfigInfo.Size));
	BDSP_Raaga_P_Analyse_CIT_GateOpenConfig(pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer);

	BDBG_MSG(("Scheduling Config Offset = "BDSP_MSG_FMT", Size = %d",
		BDSP_MSG_ARG(psGlobalTaskConfig->sSchedulingInfo.BaseAddr), psGlobalTaskConfig->sSchedulingInfo.Size));
	BDSP_Raaga_P_Analyse_CIT_SchedulingConfig(pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer);

	BDBG_MSG(("-------------------------------------------- "));
	BDBG_LEAVE(BDSP_Raaga_P_Analyse_CIT_GlobalTask);
}

void BDSP_Raaga_P_Analyse_CIT_PrimaryStage(
	BDSP_AF_P_sPRIMARYSTAGE_INFO *pPrimaryStageInfo
)
{
	BDBG_ENTER(BDSP_Raaga_P_Analyse_CIT_PrimaryStage);
	BDBG_MSG(("Primary Stage Configuration"));
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("Number Zero fill samples = %d",pPrimaryStageInfo->ui32NumZeroFillSamples));
	BDBG_MSG(("PPM Correction           = %s ",DisableEnable[pPrimaryStageInfo->ePPMCorrectionEnable]));
	BDBG_MSG(("Open Gate at start       = %s ",DisableEnable[pPrimaryStageInfo->eOpenGateAtStart]));
	BDBG_MSG(("Time base for the Task   = %s ",GlobalTimeBase[pPrimaryStageInfo->eTimeBaseType]));
	BDBG_MSG(("IDS Code   Offset        = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPrimaryStageInfo->sIdsCodeInfo.BaseAddr),pPrimaryStageInfo->sIdsCodeInfo.Size));
	BDBG_MSG(("Sampling Freq LUT Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPrimaryStageInfo->sSamplingFrequencyLutInfo.BaseAddr),pPrimaryStageInfo->sSamplingFrequencyLutInfo.Size));
	BDBG_MSG(("DataSync Status Offset   = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPrimaryStageInfo->sDataSyncStatusInfo.BaseAddr),pPrimaryStageInfo->sDataSyncStatusInfo.Size));
	BDBG_MSG(("TSM Status Offset        = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pPrimaryStageInfo->sTsmStatusInfo.BaseAddr),pPrimaryStageInfo->sTsmStatusInfo.Size));
	BDSP_Raaga_P_Analyse_CIT_PPMConfig(&pPrimaryStageInfo->sPPMConfig[0]);

	BDBG_LEAVE(BDSP_Raaga_P_Analyse_CIT_PrimaryStage);
}

void BDSP_Raaga_P_Analyse_CIT_Stage(
	BDSP_AF_P_sSTAGE_CONFIG *pStageConfig,
	BDSP_P_FwBuffer         *pDescriptorMemory
)
{
	unsigned index = 0;
	BDBG_ENTER(BDSP_Raaga_P_Analyse_CIT_Stage);

	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("eCollectResidual    = %s ",DisableEnable[pStageConfig->eCollectResidual]));
	BDBG_MSG(("Stage Memory Offset = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sStageMemoryInfo.BaseAddr),pStageConfig->sStageMemoryInfo.Size));
	BDBG_MSG(("User Config Offset  = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sAlgoUserConfigInfo.BaseAddr),pStageConfig->sAlgoUserConfigInfo.Size));
	BDBG_MSG(("Status Offset       = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sAlgoStatusInfo.BaseAddr),pStageConfig->sStageMemoryInfo.Size));
	BDBG_MSG(("Code   Offset       = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sAlgoCodeInfo.BaseAddr),pStageConfig->sAlgoCodeInfo.Size));
	BDBG_MSG(("Interframe Offset   = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sInterFrameInfo.BaseAddr),pStageConfig->sInterFrameInfo.Size));
	BDBG_MSG(("LUT	Offset         = "BDSP_MSG_FMT" Size = %d",BDSP_MSG_ARG(pStageConfig->sLookUpTableInfo.BaseAddr),pStageConfig->sLookUpTableInfo.Size));
	BDBG_MSG(("Num Input Ports     = %d", pStageConfig->sIOConfig.ui32NumInputs));
	for(index = 0; index<pStageConfig->sIOConfig.ui32NumInputs; index++)
	{
		BDBG_MSG(("--------------------"));
		BDSP_Raaga_P_Analyse_CIT_PortDetails(&pStageConfig->sIOConfig.sInputPort[index], pDescriptorMemory);
		BDBG_MSG(("--------------------"));
	}
	BDBG_MSG(("Num Output Ports    = %d", pStageConfig->sIOConfig.ui32NumOutputs));
	for(index = 0; index<pStageConfig->sIOConfig.ui32NumOutputs; index++)
	{
		BDBG_MSG(("--------------------"));
		BDSP_Raaga_P_Analyse_CIT_PortDetails(&pStageConfig->sIOConfig.sOutputPort[index], pDescriptorMemory);
		BDBG_MSG(("--------------------"));
	}
	BDBG_LEAVE(BDSP_Raaga_P_Analyse_CIT_Stage);
}
