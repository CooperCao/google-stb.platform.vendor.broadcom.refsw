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

#include "bdsp_raaga_priv_include.h"
#include "bchp_memc_arb_0.h"

BDBG_MODULE(bdsp_diag);
static void BDSP_Raaga_P_Device_Memory_details(
	BDSP_Raaga_P_DeviceMemoryInfo *pMemInfo,
	unsigned numDsp
)
{
	unsigned i = 0, size = 0, j = 0, k = 0;
	BDBG_ENTER(BDSP_Raaga_P_Device_Memory_details);
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("\tDEVICE MEMORY FOOTPRINT "));
	BDBG_MSG(("\tREAD ONLY MEMORY \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
		BDSP_MSG_ARG(pMemInfo->sROMemoryPool.Memory.offset),
		BDSP_MSG_ARG(pMemInfo->sROMemoryPool.Memory.offset+ pMemInfo->sROMemoryPool.ui32Size),
		pMemInfo->sROMemoryPool.ui32Size,
		pMemInfo->sROMemoryPool.ui32UsedSize));
	for(i=0; i<numDsp; i++)
	{
		BDBG_MSG(("\tALLOCATION FOR DSP %d ",i));
		BDBG_MSG(("\tKERNEL MEMORY RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
			BDSP_MSG_ARG(pMemInfo->sKernelRWMemoryPool[i].Memory.offset),
			BDSP_MSG_ARG(pMemInfo->sKernelRWMemoryPool[i].Memory.offset+ pMemInfo->sROMemoryPool.ui32Size),
			pMemInfo->sKernelRWMemoryPool[i].ui32Size,
			pMemInfo->sKernelRWMemoryPool[i].ui32UsedSize));
		BDBG_MSG(("-------------------------------------------- "));
		BDBG_MSG(("\tHOST SHARED RW RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
			BDSP_MSG_ARG(pMemInfo->sHostSharedRWMemoryPool[i].Memory.offset),
			BDSP_MSG_ARG(pMemInfo->sHostSharedRWMemoryPool[i].Memory.offset+ pMemInfo->sHostSharedRWMemoryPool[i].ui32Size),
			pMemInfo->sHostSharedRWMemoryPool[i].ui32Size,
			pMemInfo->sHostSharedRWMemoryPool[i].ui32UsedSize));
		BDBG_MSG(("\t \tTARGET PRINT RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->TargetBufferMemory[i].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->TargetBufferMemory[i].Buffer.offset+ pMemInfo->TargetBufferMemory[i].ui32Size),
			pMemInfo->TargetBufferMemory[i].ui32Size));
		BDBG_MSG(("\t \tCACHE HOLE 1 RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->CacheHole1[i].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->CacheHole1[i].Buffer.offset+pMemInfo->CacheHole1[i].ui32Size),
			pMemInfo->CacheHole1[i].ui32Size));
		BDBG_MSG(("\t \tCOMMAND QUEUE RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->cmdQueueParams[i].Memory.offset),
			BDSP_MSG_ARG(pMemInfo->cmdQueueParams[i].Memory.offset+pMemInfo->cmdQueueParams[i].ui32Size),
			pMemInfo->cmdQueueParams[i].ui32Size));
		BDBG_MSG(("\t \tRESPONSE QUEUE RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->genRspQueueParams[i].Memory.offset),
			BDSP_MSG_ARG(pMemInfo->genRspQueueParams[i].Memory.offset+pMemInfo->genRspQueueParams[i].ui32Size),
			pMemInfo->genRspQueueParams[i].ui32Size));
		BDBG_MSG(("\t \tCACHE HOLE 2 RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->CacheHole2[i].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->CacheHole2[i].Buffer.offset+pMemInfo->CacheHole2[i].ui32Size),
			pMemInfo->CacheHole2[i].ui32Size));
		size = BDSP_MAX_POOL_OF_DESCRIPTORS*BDSP_ALIGN_SIZE((BDSP_MAX_DESCRIPTORS_PER_POOL*sizeof(BDSP_AF_P_sCIRCULAR_BUFFER)),BDSP_MAX_HOST_DSP_L2C_SIZE);
		BDBG_MSG(("\t \tDESCRIPTOR RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->DescriptorMemory[i][0].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->DescriptorMemory[i][0].Buffer.offset+ size),
			size));
		BDBG_MSG(("\t \tDEBUG SERVICE RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->DeubgServiceMemory[i].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->DeubgServiceMemory[i].Buffer.offset+ pMemInfo->DeubgServiceMemory[i].ui32Size),
			pMemInfo->DeubgServiceMemory[i].ui32Size));
		for(j=0; j<BDSP_DebugType_eLast;j++)
		{
			BDBG_MSG(("\t \tDEBUG QUEUE[%d] RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", j,BDSP_MSG_ARG(pMemInfo->debugQueueParams[i][j].Memory.offset),
				BDSP_MSG_ARG(pMemInfo->debugQueueParams[i][j].Memory.offset+pMemInfo->debugQueueParams[i][j].ui32Size),
				pMemInfo->debugQueueParams[i][j].ui32Size));
		}
		BDBG_MSG(("-------------------------------------------- "));
		BDBG_MSG(("\tDEVICE ONLY RW RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
			BDSP_MSG_ARG(pMemInfo->sRWMemoryPool[i].Memory.offset),
			BDSP_MSG_ARG(pMemInfo->sRWMemoryPool[i].Memory.offset+ pMemInfo->sRWMemoryPool[i].ui32Size),
			pMemInfo->sRWMemoryPool[i].ui32Size,
			pMemInfo->sRWMemoryPool[i].ui32UsedSize));
		for(j=0; j < BDSP_RAAGA_MAX_CORE_PER_DSP; j++)
		{
			for(k=0; k<BDSP_MAX_NUM_SCHED_LEVELS;k++)
			{
				BDBG_MSG(("\t \tWORKBUFFER[Core %d][ Level %d] RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", j,k,
					BDSP_MSG_ARG(pMemInfo->WorkBufferMemory[i][j][k].Buffer.offset),
					BDSP_MSG_ARG(pMemInfo->WorkBufferMemory[i][j][k].Buffer.offset+pMemInfo->WorkBufferMemory[i][j][k].ui32Size),
					pMemInfo->WorkBufferMemory[i][j][k].ui32Size));
			}
		}
		BDBG_MSG(("\t \tPROCESS SPAWN MEMORY RANGE\t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->ProcessSpawnMemory[i].Buffer.offset),
			BDSP_MSG_ARG(pMemInfo->ProcessSpawnMemory[i].Buffer.offset+pMemInfo->ProcessSpawnMemory[i].ui32Size),
			pMemInfo->ProcessSpawnMemory[i].ui32Size));
		BDBG_MSG(("-------------------------------------------- "));
	}
	BDBG_LEAVE(BDSP_Raaga_P_Device_Memory_details);
}

static void BDSP_Raaga_P_Device_Generic_details(
	BDSP_Raaga  *pDevice
)
{
	unsigned index = 0;
	BERR_Code ret;
	BBOX_Config *pBoxConfig;
	uint32_t blockout = 0, roundrobin = 0, regval =0;
	BDBG_ENTER(BDSP_Raaga_P_Device_Generic_details);
	pBoxConfig = (BBOX_Config *)BKNI_Malloc(sizeof(BBOX_Config));
    if(pBoxConfig == NULL)
    {
        BDBG_ERR(("BDSP_Raaga_P_Device_Generic_details: Unable to allocate memory for BOX Config"));
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		return;
    }
	BKNI_Memset(pBoxConfig, 0 , sizeof(BBOX_Config));
	ret = BBOX_GetConfig(pDevice->boxHandle, pBoxConfig);
	if(BERR_SUCCESS != ret)
	{
		BDBG_ERR(("BDSP_Raaga_P_Device_Generic_details: Error in retrieving the Num DSP from BOX MODE"));
		goto exit;
	}
	BDBG_MSG(("\t BOX ID              = %d", pBoxConfig->stBox.ulBoxId));
	BDBG_MSG(("\t Number of DSPs      = %d", pDevice->numDsp));
	BDBG_MSG(("\t Number of Cores/DSP = %d", pDevice->numCorePerDsp));
	{
		regval = BDSP_ReadReg32(pDevice->regHandle,BCHP_MEMC_ARB_0_CLIENT_INFO_40);
		roundrobin = (regval&BCHP_MEMC_ARB_0_CLIENT_INFO_40_RR_EN_MASK)>>BCHP_MEMC_ARB_0_CLIENT_INFO_40_RR_EN_SHIFT;
		BDBG_MSG(("\t ROUND ROBIN         = %s",DisableEnable[roundrobin]));
		blockout = (regval&BCHP_MEMC_ARB_0_CLIENT_INFO_40_BO_VAL_MASK)>>BCHP_MEMC_ARB_0_CLIENT_INFO_40_BO_VAL_SHIFT;
		BDBG_MSG(("\t BLOCKOUT VALUE      = 0x%x (%dns)",blockout,((blockout*700/0x13f))));
	}
	BDBG_MSG(("\t NUM MEM CONTROLLERS = %d", pBoxConfig->stMemConfig.ulNumMemc));
	BDBG_MSG(("\t MEMORY REFRESH RATE = %dx", pBoxConfig->stMemConfig.eRefreshRate));
	BDBG_MSG(("\t Application Settings"));
	BDBG_MSG(("\t \tFirmware Auntentication = %s", DisableEnable[pDevice->deviceSettings.authenticationEnabled]));
	BDBG_MSG(("\t \tPre Load binaries       = %s", DisableEnable[pDevice->deviceSettings.preloadImages]));
	BDBG_MSG(("\t System Scheduling configuration"));
	BDBG_MSG(("\t \tScheduling levels = %d",pDevice->systemSchedulingInfo.numSchedulingLevels));
	BDBG_MSG(("\t \tPreemption Levels = %d",pDevice->systemSchedulingInfo.numPreemptionLevels));
	for(index=0; index<BDSP_P_TaskType_eLast; index++)
	{
		BDBG_MSG(("\t \t \tTask Type %s:\t%s,\tLEVEL: 0x%x,\tTHRESHOLD: 0x%x",
			TaskType[index], DisableEnable[pDevice->systemSchedulingInfo.sTaskSchedulingInfo[index].supported],
			pDevice->systemSchedulingInfo.sTaskSchedulingInfo[index].schedulingLevel,
			pDevice->systemSchedulingInfo.sTaskSchedulingInfo[index].schedulingThreshold));
	}
exit:
	BDBG_LEAVE(BDSP_Raaga_P_Device_Generic_details);
	BKNI_Free(pBoxConfig);
}

void BDSP_Raaga_P_Device_Diagnostics(
	BDSP_Raaga  *pDevice
)
{
	BDBG_ENTER(BDSP_Raaga_P_Device_Diagnostics);
	BDBG_MSG(("============================================ "));
	BDBG_MSG(("\t HOST DIAGNOSTIC INFORMATION "));
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("\t SYSTEM INFORMATION "));
	/*BDBG_MSG(("\t LICENSE DETAILS T.B.D"));*/
	BDSP_Raaga_P_Device_Generic_details(pDevice);
	BDBG_MSG(("-------------------------------------------- "));
	BDSP_Raaga_P_Device_Memory_details(&pDevice->memInfo, pDevice->numDsp);
	BDBG_MSG(("============================================ "));
	BDBG_LEAVE(BDSP_Raaga_P_Device_Diagnostics);
}

void BDSP_Raaga_P_Stage_PortDiagnostics(
	BDSP_RaagaStage  *pRaagaStage
)
{
	unsigned inputIndex =0, outputIndex =0, index = 0, size = 0;
	dramaddr_t base = 0, end = 0;
	BDSP_P_ConnectionDetails *pStageConnectionDetails;
    BDSP_P_InterTaskBuffer    *pInterTaskBuffer;
	BDSP_RaagaQueue           *pQueueBuffer;
	BAVC_XptContextMap        *pRaveContextMap;
	BDSP_FmmBufferDescriptor  *pFMMDescriptor;
	BDBG_ENTER(BDSP_Raaga_P_Stage_PortDiagnostics);
    /* Port Memory Details for Input*/
	for(inputIndex=0; inputIndex<BDSP_AF_P_MAX_IP_FORKS; inputIndex++)
	{
		pStageConnectionDetails = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[inputIndex];
		if(pStageConnectionDetails->eValid)
		{
			switch(pStageConnectionDetails->eConnectionType)
			{
                case BDSP_ConnectionType_eInterTaskBuffer:
                    BDBG_MSG(("\t INTERTASK PORT AT INPUT MEMORY DETAILS"));
                    pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
					BDBG_MSG(("\t \tCOMPLETE BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",BDSP_MSG_ARG(pInterTaskBuffer->MemoryPool.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->MemoryPool.Memory.offset+ pInterTaskBuffer->MemoryPool.ui32Size),
						pInterTaskBuffer->MemoryPool.ui32Size));
					for(index = 0; index< pInterTaskBuffer->numChannels; index++)
					{
						BDBG_MSG(("\t \tPCM [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->PcmDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->PcmDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->PcmDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->PcmDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numMetaData; index++)
					{
						BDBG_MSG(("\t \tMETA[%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numTocData; index++)
					{
						BDBG_MSG(("\t \tTOC [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->TocDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->TocDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->TocDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->TocDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numObjectData; index++)
					{
						BDBG_MSG(("\t \tOBJ [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.ui32Size));
					}
					BDBG_MSG(("-------------------------------------------- "));
                    break;
				case BDSP_ConnectionType_eRaveBuffer:
					BDBG_MSG(("\t RAVE PORT AT INPUT MEMORY DETAILS"));
					pRaveContextMap = (BAVC_XptContextMap *)&pStageConnectionDetails->connectionHandle.rave.raveContextMap;
					base = BDSP_ReadReg64(pRaagaStage->pContext->pDevice->regHandle,pRaveContextMap->CDB_Base);
					end  = BDSP_ReadReg64(pRaagaStage->pContext->pDevice->regHandle,pRaveContextMap->CDB_End);
					size = end - base + 1;
					BDBG_MSG(("\t \tCDB BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",	BDSP_MSG_ARG(base),
					BDSP_MSG_ARG(end),
					size));
					base = BDSP_ReadReg64(pRaagaStage->pContext->pDevice->regHandle,pRaveContextMap->ITB_Base);
					end  = BDSP_ReadReg64(pRaagaStage->pContext->pDevice->regHandle,pRaveContextMap->ITB_End);
					size = end - base + 1;
					BDBG_MSG(("\t \tITB BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",	BDSP_MSG_ARG(base),
					BDSP_MSG_ARG(end),
					size));
					BDBG_MSG(("-------------------------------------------- "));
					break;
				case BDSP_ConnectionType_eFmmBuffer:
					BDBG_MSG(("\t FMM PORT AT INPUT MEMORY DETAILS"));
					pFMMDescriptor = (BDSP_FmmBufferDescriptor *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor;
					for(index = 0; index< pFMMDescriptor->numBuffers; index++)
					{
						base = BDSP_ReadFMMReg(pRaagaStage->pContext->pDevice->regHandle,pFMMDescriptor->buffers[index].base);
						end  = BDSP_ReadFMMReg(pRaagaStage->pContext->pDevice->regHandle,pFMMDescriptor->buffers[index].end);
						size = end - base + 1;
						BDBG_MSG(("\t \tFMM [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
								BDSP_MSG_ARG(base),
								BDSP_MSG_ARG(end),
								size));
					}
					BDBG_MSG(("-------------------------------------------- "));
					break;
                case BDSP_ConnectionType_eRDBBuffer:
                    pQueueBuffer = (BDSP_RaagaQueue *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle;
					if(BDSP_AF_P_DistinctOpType_eDescriptorQueue == pQueueBuffer->distinctOp)
					{
	                    BDBG_MSG(("\t ANDROID PORT AT OUTPUT MEMORY DETAILS"));
					}
					else
					{
	                    BDBG_MSG(("\t RDB PORT AT OUTPUT MEMORY DETAILS"));
					}
					for(index = 0; index< pQueueBuffer->numChannels; index++)
					{
						BDBG_MSG(("\t \tQUEUE[%d] BUFFER  RANGE ["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pQueueBuffer->createSettings.bufferInfo[index].buffer.offset),
						BDSP_MSG_ARG(pQueueBuffer->createSettings.bufferInfo[index].buffer.offset+ pQueueBuffer->createSettings.bufferInfo[index].bufferSize),
						pQueueBuffer->createSettings.bufferInfo[index].bufferSize));
					}
					BDBG_MSG(("-------------------------------------------- "));
                    break;
				default:
					break;
			}
		}
    }

    /* Port Memory Details for Output*/
	for(outputIndex=0; outputIndex<BDSP_AF_P_MAX_OP_FORKS; outputIndex++)
	{
		pStageConnectionDetails = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[outputIndex];
		if(pStageConnectionDetails->eValid)
		{
			switch(pStageConnectionDetails->eConnectionType)
			{
                case BDSP_ConnectionType_eInterTaskBuffer:
                    BDBG_MSG(("\t INTERTASK PORT AT OUTPUT MEMORY DETAILS"));
                    pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
					BDBG_MSG(("\t \tCOMPLETE BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",BDSP_MSG_ARG(pInterTaskBuffer->MemoryPool.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->MemoryPool.Memory.offset+ pInterTaskBuffer->MemoryPool.ui32Size),
						pInterTaskBuffer->MemoryPool.ui32Size));
					for(index = 0; index< pInterTaskBuffer->numChannels; index++)
					{
						BDBG_MSG(("\t \tPCM [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->PcmDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->PcmDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->PcmDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->PcmDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numMetaData; index++)
					{
						BDBG_MSG(("\t \tMETA[%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->MetaDataDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numTocData; index++)
					{
						BDBG_MSG(("\t \tTOC [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->TocDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->TocDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->TocDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->TocDetails[index].BuffersDetails.ui32Size));
					}
					for(index = 0; index< pInterTaskBuffer->numObjectData; index++)
					{
						BDBG_MSG(("\t \tOBJ [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.Memory.offset),
						BDSP_MSG_ARG(pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.Memory.offset+ pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.ui32Size),
						pInterTaskBuffer->ObjectDataDetails[index].BuffersDetails.ui32Size));
					}
					BDBG_MSG(("-------------------------------------------- "));
                    break;
				case BDSP_ConnectionType_eFmmBuffer:
					BDBG_MSG(("\t FMM PORT AT OUTPUT MEMORY DETAILS"));
					pFMMDescriptor = (BDSP_FmmBufferDescriptor *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor;
					for(index = 0; index< pFMMDescriptor->numBuffers; index++)
					{
						base = BDSP_ReadFMMReg(pRaagaStage->pContext->pDevice->regHandle,pFMMDescriptor->buffers[index].base);
						end  = BDSP_ReadFMMReg(pRaagaStage->pContext->pDevice->regHandle,pFMMDescriptor->buffers[index].end);
						size = end - base + 1;
						BDBG_MSG(("\t \tFMM [%d] BUFFER  RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
								BDSP_MSG_ARG(base),
								BDSP_MSG_ARG(end),
								size));
					}
					BDBG_MSG(("-------------------------------------------- "));
					break;
                case BDSP_ConnectionType_eRDBBuffer:
                    pQueueBuffer = (BDSP_RaagaQueue *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle;
					if(BDSP_AF_P_DistinctOpType_eDescriptorQueue == pQueueBuffer->distinctOp)
					{
	                    BDBG_MSG(("\t ANDROID PORT AT OUTPUT MEMORY DETAILS"));
					}
					else
					{
	                    BDBG_MSG(("\t RDB PORT AT OUTPUT MEMORY DETAILS"));
					}
					for(index = 0; index< pQueueBuffer->numChannels; index++)
					{
						BDBG_MSG(("\t \t QUEUE[%d] BUFFER  RANGE ["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size = %d",index,
						BDSP_MSG_ARG(pQueueBuffer->createSettings.bufferInfo[index].buffer.offset),
						BDSP_MSG_ARG(pQueueBuffer->createSettings.bufferInfo[index].buffer.offset+ pQueueBuffer->createSettings.bufferInfo[index].bufferSize),
						pQueueBuffer->createSettings.bufferInfo[index].bufferSize));
					}
					BDBG_MSG(("-------------------------------------------- "));
					break;
				default:
					break;
			}
		}
    }
	BDBG_LEAVE(BDSP_Raaga_P_Stage_PortDiagnostics);
}
