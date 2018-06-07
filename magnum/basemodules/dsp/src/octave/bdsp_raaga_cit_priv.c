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

#include "bdsp_raaga_cit_priv.h"

BDBG_MODULE(bdsp_cit_priv);

static BERR_Code BDSP_Raaga_P_PopulateHwPPMConfig(
	BDSP_RaagaStage *pRaagaPrimaryStage,
	BDSP_AF_P_sHW_PPM_CONFIG *psHWPPMConfig
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned PPMCount = 0, index=0, portindex = 0, rateIndex = 0;
	BDSP_AF_P_sHW_PPM_CONFIG *psConfig = psHWPPMConfig;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_PopulateHwPPMConfig);

    /*Initialization*/
    for(index =0; index<BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS;index++)
    {
        psConfig->ePPMChannel = BDSP_AF_P_eDisable;
        psConfig->PPMCfgAddr  = 0;
		psConfig++;
    }

	psConfig = psHWPPMConfig;
    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        for(portindex=0;portindex<BDSP_AF_P_MAX_OP_FORKS;portindex++)
        {
			psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaConnectStage->sStageConnectionInfo.sStageOutput[portindex];
            if(psStageOutput->eConnectionType == BDSP_ConnectionType_eFmmBuffer &&psStageOutput->eValid==BDSP_AF_P_eValid)
            {
                for(rateIndex =0; rateIndex<BDSP_AF_P_MAX_CHANNEL_PAIR; rateIndex++)
                {
					unsigned wrcnt = psStageOutput->connectionHandle.fmm.rateController[rateIndex].wrcnt;
					if(wrcnt != BDSP_AF_P_WRCNT_INVALID)
					{
						if(PPMCount >= BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS)
						{
							BDBG_ERR(("BDSP_Raaga_P_PopulateHwPPMConfig:Programming for more than %d Adaptive Blocks",BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS));
			                errCode = BERR_LEAKED_RESOURCE;
                            goto end;
						}
						psConfig->ePPMChannel = BDSP_AF_P_eEnable;
						psConfig->PPMCfgAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(wrcnt);
						PPMCount++;
						psConfig++;
					}
                }
			}
		}
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

end:
	BDBG_LEAVE(BDSP_Raaga_P_PopulateHwPPMConfig);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PopulatePortBufferDescriptors(
	void *pDescriptor,
	BDSP_ConnectionType eConnectionType,
	BDSP_AF_P_sCIRCULAR_BUFFER *pCircularBuffer,
	unsigned numBuffers,
	unsigned typeOfBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned i=0;

	BDBG_ENTER(BDSP_Raaga_P_PopulatePortBufferDescriptors);
	switch(eConnectionType)
	{
		case BDSP_ConnectionType_eRaveBuffer:
			{
				BAVC_XptContextMap *pRaveContextMap = (BAVC_XptContextMap *)pDescriptor;
                if (typeOfBuffer > 0)
                {
                    BDBG_ERR(("Rave Doesn't support TOC/META/Object Data type"));
                    BDBG_ASSERT(0);
                }
				for(i=0;i<numBuffers;i++)
				{
					pCircularBuffer->BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->CDB_Base);
					pCircularBuffer->EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->CDB_End);
					pCircularBuffer->ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->CDB_Read);
					pCircularBuffer->WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->CDB_Valid);
					pCircularBuffer->WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->CDB_Wrap);
					i=i+1;
					pCircularBuffer++;
					pCircularBuffer->BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->ITB_Base);
					pCircularBuffer->EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->ITB_End);
					pCircularBuffer->ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->ITB_Read);
					pCircularBuffer->WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->ITB_Valid);
					pCircularBuffer->WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaveContextMap->ITB_Wrap);
					pCircularBuffer++;
				}
			}
			break;
		case BDSP_ConnectionType_eFmmBuffer:
			{
				BDSP_FmmBufferDescriptor *pFMMDescriptor = (BDSP_FmmBufferDescriptor *)pDescriptor;
                if (typeOfBuffer > 0)
                {
                    BDBG_ERR(("FMM Doesn't support TOC/META/Object Data type"));
                    BDBG_ASSERT(0);
                }
				for( i=0;i<numBuffers;i++)
				{
					pCircularBuffer->BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].base);
					pCircularBuffer->EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].end);
					pCircularBuffer->ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].read);
					pCircularBuffer->WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].write);
					pCircularBuffer->WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].end);
					pCircularBuffer++;
				}
			}
			break;
        case BDSP_ConnectionType_eRDBBuffer:
            {
                BDSP_RaagaQueue *pQueue = (BDSP_RaagaQueue *)pDescriptor;
                if (typeOfBuffer > 0)
                {
                    BDBG_ERR(("Queue/RDB buffer doesn't support TOC/META/Object Data type"));
                    BDBG_ASSERT(0);
                }
                for( i=0;i<numBuffers;i++)
                {
                    pCircularBuffer->BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pQueue->hMsgQueue[i]->QueueAddress.BaseOffset);
                    pCircularBuffer->EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pQueue->hMsgQueue[i]->QueueAddress.EndOffset);
                    pCircularBuffer->ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pQueue->hMsgQueue[i]->QueueAddress.ReadOffset);
                    pCircularBuffer->WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pQueue->hMsgQueue[i]->QueueAddress.WriteOffset);
                    pCircularBuffer->WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pQueue->hMsgQueue[i]->QueueAddress.EndOffset);
                    pCircularBuffer++;
                }
            }
            break;
		case BDSP_ConnectionType_eInterTaskBuffer:
			{
                BDSP_P_InterTaskBuffer *pIntertaskBuffer = (BDSP_P_InterTaskBuffer*)pDescriptor;
                BDSP_P_InterTaskBufferDetails *pBufferDetails;
                switch(typeOfBuffer)
                {
                    case BDSP_AF_P_PortBuffer_Type_Data:
                        pBufferDetails = &pIntertaskBuffer->PcmDetails[0];
                        break;
                    case BDSP_AF_P_PortBuffer_Type_TOC:
                        pBufferDetails = &pIntertaskBuffer->TocDetails[0];
                        break;
                    case BDSP_AF_P_PortBuffer_Type_MetaData:
                        pBufferDetails = &pIntertaskBuffer->MetaDataDetails[0];
                        break;
                    case BDSP_AF_P_PortBuffer_Type_ObjectData:
                        pBufferDetails = &pIntertaskBuffer->ObjectDataDetails[0];
                        break;
                    default:
                        BDBG_ERR(("BDSP_Raaga_P_PopulatePortBufferDescriptors: Invalid Port buffer type"));
                        BDBG_ASSERT(0);
                        break;
                }
                for( i=0;i<numBuffers;i++)
                {
                    if(BDSP_AF_P_BufferType_eDRAM == pIntertaskBuffer->ebufferType)
                    {
                        pCircularBuffer->BaseAddr = pBufferDetails->BufferPointer.BaseOffset;
                        pCircularBuffer->EndAddr  = pBufferDetails->BufferPointer.EndOffset;
                        pCircularBuffer->ReadAddr = pBufferDetails->BufferPointer.BaseOffset;
                        pCircularBuffer->WriteAddr= pBufferDetails->BufferPointer.BaseOffset;
                        pCircularBuffer->WrapAddr = pBufferDetails->BufferPointer.EndOffset;
                    }
                    else
                    {
                        /* RDB*/
                        pCircularBuffer->BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pBufferDetails->hQueueHandle->QueueAddress.BaseOffset);
                        pCircularBuffer->EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pBufferDetails->hQueueHandle->QueueAddress.EndOffset);
                        pCircularBuffer->ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pBufferDetails->hQueueHandle->QueueAddress.ReadOffset);
                        pCircularBuffer->WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pBufferDetails->hQueueHandle->QueueAddress.WriteOffset);
                        pCircularBuffer->WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pBufferDetails->hQueueHandle->QueueAddress.EndOffset);
                    }
                    pCircularBuffer++;
                    pBufferDetails++;
                }
			}
			break;
		case BDSP_ConnectionType_eStage:
            break;
		default:
			BDBG_ERR(("BDSP_Raaga_P_PopulatePortBufferDescriptors: Wrong Connection type"));
			break;
	}
	BDBG_LEAVE(BDSP_Raaga_P_PopulatePortBufferDescriptors);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PopulateGateOpenConfig(
	BDSP_RaagaTask *pRaagaTask,
	BDSP_MMA_Memory Memory
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	BDSP_AF_P_sTASK_GATEOPEN_CONFIG *pGateOpenConfig;
	BDSP_AF_P_sRING_BUFFER_INFO *pRingbuffer;
	unsigned NumPorts =0 , index =0, i =0;
    BDSP_AF_P_FmmDstFsRate eBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eInvalid;
    BDSP_AF_P_FmmContentType eFMMContentType = BDSP_AF_P_FmmContentType_eInvalid;
	BDSP_AF_P_sCIRCULAR_BUFFER sCircularBuffer[BDSP_AF_P_MAX_CHANNELS];

	BDBG_ENTER(BDSP_Raaga_P_PopulateGateOpenConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pGateOpenConfig = (BDSP_AF_P_sTASK_GATEOPEN_CONFIG *)Memory.pAddr;
	pRingbuffer = &pGateOpenConfig->sRingBufferInfo[0];

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroBrId);
	BSTD_UNUSED(macroStId);
	{
		for(index=0; index<BDSP_AF_P_MAX_OP_FORKS; index++)
		{
			if((pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eConnectionType == BDSP_ConnectionType_eFmmBuffer) &&
				(pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eValid == BDSP_AF_P_eValid))
			{

				BDSP_P_GetFMMDetails(pRaagaConnectStage->sStageConnectionInfo.eStageOpDataType[index],
					pRaagaConnectStage->eAlgorithm,
					&eBaseRateMultiplier,
					&eFMMContentType);
				pRingbuffer->bFixedSampleRate    = BDSP_AF_P_Boolean_eFalse;
				pRingbuffer->ui32FixedSampleRate = 0;
				pRingbuffer->eBufferType         = BDSP_AF_P_BufferType_eFMM;
				pRingbuffer->eFMMContentType     = eFMMContentType;
				pRingbuffer->eBaseRateMultiplier = eBaseRateMultiplier;
				pRingbuffer->ui32IndependentDelay= pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.delay;
				pRingbuffer->ui32NumBuffers      = pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.numBuffers;
				errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
					(void *)&pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor,
					BDSP_ConnectionType_eFmmBuffer,
					&sCircularBuffer[0],
					pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.numBuffers,
					BDSP_AF_P_PortBuffer_Type_Data);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_PopulateGateOpenConfig: Unable to populate the descriptor for the Gate Open buffer"));
					goto end;
				}
				for(i=0;i<pRingbuffer->ui32NumBuffers;i++)
				{
					pRingbuffer->sExtendedBuffer[i].sCircularBuffer= sCircularBuffer[i];
					pRingbuffer->sExtendedBuffer[i].startWriteAddr = pRingbuffer->sExtendedBuffer[i].sCircularBuffer.ReadAddr + (5*BDSP_SIZE_OF_FMMREG);
				}
				NumPorts++;
				pRingbuffer++;
			}
		}
	}
	BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

	pGateOpenConfig->ui32NumPorts            = NumPorts;
	pGateOpenConfig->ui32MaxIndependentDelay = pRaagaTask->startSettings.maxIndependentDelay;
	pGateOpenConfig->ui32BlockingTime        = BDSP_AF_P_BLOCKING_TIME;
	BDSP_MMA_P_FlushCache(Memory,sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG));

end:
	BDBG_LEAVE(BDSP_Raaga_P_PopulateGateOpenConfig);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PopulateSchedulingConfig(
	BDSP_RaagaTask *pRaagaTask,
	BDSP_MMA_Memory Memory
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_AF_P_FmmDstFsRate eBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eMax;
    BDSP_AF_P_FmmContentType eFMMContentType = BDSP_AF_P_FmmContentType_eMax;
	unsigned index =0, independentDelay =0;
	BDSP_AF_P_sTASK_SCHEDULING_CONFIG *pSchedulingConfig;

	BDBG_ENTER(BDSP_Raaga_P_PopulateSchedulingConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pSchedulingConfig = (BDSP_AF_P_sTASK_SCHEDULING_CONFIG *)Memory.pAddr;

    if(pRaagaTask->startSettings.schedulingMode != BDSP_TaskSchedulingMode_eSlave)
    {
        BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
        BSTD_UNUSED(macroBrId);
        BSTD_UNUSED(macroStId);
        {
            for(index=0; index<BDSP_AF_P_MAX_OP_FORKS; index++)
            {
                if((pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eConnectionType == BDSP_ConnectionType_eFmmBuffer) &&
                    (pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eValid == BDSP_AF_P_eValid))
                {
                    pSchedulingConfig->eBufferType = BDSP_AF_P_BufferType_eFMM;
                    BDSP_P_GetFMMDetails(pRaagaConnectStage->sStageConnectionInfo.eStageOpDataType[index],
                        pRaagaConnectStage->eAlgorithm,
                        &eBaseRateMultiplier,
                        &eFMMContentType);
                    errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
                        (void *)&pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor,
                        BDSP_ConnectionType_eFmmBuffer,
                        &pSchedulingConfig->sExtendedBuffer.sCircularBuffer,
                        1,
                        BDSP_AF_P_PortBuffer_Type_Data);
                    if(errCode != BERR_SUCCESS)
                    {
                        BDBG_ERR(("BDSP_Raaga_P_PopulateSchedulingConfig: Unable to populate the descriptor for the Scheduling buffer"));
                        goto end;
                    }
                    pSchedulingConfig->sExtendedBuffer.startWriteAddr = pSchedulingConfig->sExtendedBuffer.sCircularBuffer.ReadAddr + (5*BDSP_SIZE_OF_FMMREG);
                    independentDelay = pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.delay;
                    goto next_step_1;
                }
            }
        }
        BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

next_step_1:
        pSchedulingConfig->sSchedulingInfo.ui32MaxIndependentDelay = pRaagaTask->startSettings.maxIndependentDelay;
        pSchedulingConfig->sSchedulingInfo.eFMMContentType         = eFMMContentType;
        pSchedulingConfig->sSchedulingInfo.eBaseRateMultiplier     = eBaseRateMultiplier;
        pSchedulingConfig->sSchedulingInfo.bFixedSampleRate        = BDSP_AF_P_Boolean_eFalse;
        pSchedulingConfig->sSchedulingInfo.ui32FixedSampleRate     = 0;
        pSchedulingConfig->sSchedulingInfo.ui32BlockingTime        = BDSP_AF_P_BLOCKING_TIME;
        pSchedulingConfig->sSchedulingInfo.ui32IndependentDelay    = independentDelay;
    }
    else
    {
        BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
        BSTD_UNUSED(macroBrId);
        BSTD_UNUSED(macroStId);
        {
            for(index=0; index<BDSP_AF_P_MAX_OP_FORKS; index++)
            {
                if((pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eConnectionType == BDSP_ConnectionType_eInterTaskBuffer) &&
                    (pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].eValid == BDSP_AF_P_eValid))
                {
                    pSchedulingConfig->eBufferType = BDSP_AF_P_BufferType_eDRAM;
                    {
                        BDSP_P_InterTaskBuffer *pInterTaskBuffer;
                        dramaddr_t BaseOffset = 0;
                        pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
                        BaseOffset = pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_Data][0];

                        pSchedulingConfig->sExtendedBuffer.sCircularBuffer.BaseAddr  = BaseOffset;
                        pSchedulingConfig->sExtendedBuffer.sCircularBuffer.EndAddr   = BaseOffset + (1*sizeof(dramaddr_t));
                        pSchedulingConfig->sExtendedBuffer.sCircularBuffer.ReadAddr  = BaseOffset + (2*sizeof(dramaddr_t));
                        pSchedulingConfig->sExtendedBuffer.sCircularBuffer.WriteAddr = BaseOffset + (3*sizeof(dramaddr_t));
                        pSchedulingConfig->sExtendedBuffer.sCircularBuffer.WrapAddr  = BaseOffset + (4*sizeof(dramaddr_t));
                    }
                    independentDelay = 0;
                    goto next_step_2;
                }
            }
        }
        BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

next_step_2:
        pSchedulingConfig->sSchedulingInfo.ui32MaxIndependentDelay = 0;
        pSchedulingConfig->sSchedulingInfo.eFMMContentType         = eFMMContentType;
        pSchedulingConfig->sSchedulingInfo.eBaseRateMultiplier     = eBaseRateMultiplier;
        pSchedulingConfig->sSchedulingInfo.bFixedSampleRate        = BDSP_AF_P_Boolean_eFalse;
        pSchedulingConfig->sSchedulingInfo.ui32FixedSampleRate     = 0;
        pSchedulingConfig->sSchedulingInfo.ui32BlockingTime        = 0;
        pSchedulingConfig->sSchedulingInfo.ui32IndependentDelay    = independentDelay;

    }

	BDSP_MMA_P_FlushCache(Memory,sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG));

end:
	BDBG_LEAVE(BDSP_Raaga_P_PopulateSchedulingConfig);
	return errCode;
}

static void BDSP_Raaga_P_CreatePortBufferDetails(
    BDSP_Raaga          *pDevice,
    unsigned             dspIndex,
    BDSP_ConnectionType  eConnectionType,
    void                *pConnectionDetails,
    BDSP_P_PortDetails  *pPortDetials
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned i=0, j=0, numbuffers=0, usedIndex=0;
	BDSP_MMA_Memory descriptorMemory[BDSP_MAX_DESCRIPTORS_PER_POOL];
	BDSP_AF_P_sCIRCULAR_BUFFER *pDescriptor[BDSP_MAX_DESCRIPTORS_PER_POOL];
	BDBG_ENTER(BDSP_Raaga_P_CreatePortBufferDetails);

	numbuffers = pPortDetials->numDataBuffers+
					pPortDetials->numTocBuffers+
					pPortDetials->numMetaDataBuffers+
					pPortDetials->numObjectDataBuffers;
	BKNI_Memset((void *)&descriptorMemory[0],0,(sizeof(BDSP_MMA_Memory)*BDSP_MAX_DESCRIPTORS_PER_POOL));
	BDSP_Raaga_P_AssignDescriptor((void *)pDevice,
								dspIndex,
								&descriptorMemory[0],
								numbuffers);
	for(i=0;i<numbuffers;i++)
	{
		pDescriptor[i] = (BDSP_AF_P_sCIRCULAR_BUFFER *)descriptorMemory[i].pAddr;
	}
    for(i=0; i<pPortDetials->numPortBuffers; i++)
    {
        switch(i)
        {
            case BDSP_AF_P_PortBuffer_Type_Data: /* Data Buffer*/
                numbuffers = pPortDetials->numDataBuffers;
                break;
            case BDSP_AF_P_PortBuffer_Type_TOC: /* TOC Buffer*/
                numbuffers = pPortDetials->numTocBuffers;
                break;
            case BDSP_AF_P_PortBuffer_Type_MetaData: /* MetaData Buffer*/
                numbuffers = pPortDetials->numMetaDataBuffers;
                break;
            case BDSP_AF_P_PortBuffer_Type_ObjectData: /* ObjectData Buffer*/
                numbuffers = pPortDetials->numObjectDataBuffers;
                break;
            default:
                BDBG_ERR(("BDSP_Raaga_P_CreatePortBufferDetails: Invalid Port buffer type"));
                BDBG_ASSERT(0);
                break;
        }
		errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
									pConnectionDetails,
									eConnectionType,
									pDescriptor[usedIndex],
									numbuffers,
									i);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_CreatePortBufferDetails: Port Buffer Descriptor Not configured properly"));
			BDBG_ASSERT(0);
		}
		for(j=0;j<numbuffers;j++)
		{
			pPortDetials->IoBuffer[i][j]= descriptorMemory[j+usedIndex].offset;
			BDSP_MMA_P_FlushCache(descriptorMemory[j+usedIndex], sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
		}
		usedIndex = usedIndex+numbuffers;
    }
	BDBG_LEAVE(BDSP_Raaga_P_CreatePortBufferDetails);
}

static void BDSP_Raaga_P_PopulatePortDetails(
    BDSP_AF_P_sIoPort   *pIOPort,
    BDSP_P_PortDetails  *pPortDetials
)
{
	int i=0, j=0;
    unsigned numbuffers=0;
	BDBG_ENTER(BDSP_Raaga_P_PopulatePortDetails);

	pIOPort->ePortType	   		  = pPortDetials->ePortType;
	pIOPort->ePortDataType 		  = pPortDetials->distinctOpType;
	pIOPort->ui32numPortBuffer	  = pPortDetials->numPortBuffers;
	pIOPort->ui32numBranchfromPort= pPortDetials->numBranchFromPort;
	pIOPort->ui32tocIndex		  = pPortDetials->tocIndex;
	pIOPort->sIOGenericBuffer	  = 0;
	if(pPortDetials->psDataAccesAttributes != NULL)
	{
		BKNI_Memcpy((void *)&pIOPort->sDataAccessAttributes, (void *)pPortDetials->psDataAccesAttributes, sizeof(BDSP_AF_P_Port_sDataAccessAttributes));
	}
	for(i=0;i<(int)pPortDetials->numPortBuffers;i++)
	{
        switch(i)
        {
            case 0: /* Data Buffer*/
                numbuffers = pPortDetials->numDataBuffers;
                break;
            case 1: /* TOC Buffer*/
                numbuffers = pPortDetials->numTocBuffers;
                break;
            case 2: /* MetaData Buffer*/
                numbuffers = pPortDetials->numMetaDataBuffers;
                break;
            case 3: /* ObjectData Buffer*/
                numbuffers = pPortDetials->numObjectDataBuffers;
                break;
            default:
                BDBG_ERR(("BDSP_Raaga_P_PopulatePortDetails: Invalid Port buffer type"));
                BDBG_ASSERT(0);
                break;
        }

		pIOPort->sIoBuffer[i].eBufferType   = pPortDetials->eBufferType;
		pIOPort->sIoBuffer[i].ui32NumBuffer = numbuffers;
		for(j=0;j<(int)numbuffers;j++)
		{
			pIOPort->sIoBuffer[i].sCircularBuffer[j] = pPortDetials->IoBuffer[i][j];
		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_PopulatePortDetails);
}

static BERR_Code BDSP_Raaga_P_PopulateIOConfiguration(
	BDSP_RaagaStage     *pRaagaStage,
	BDSP_AF_P_sIOConfig *pStageIOConfig,
	unsigned            *ptocIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice;
	unsigned dspIndex =0;
	unsigned inputIndex =0, outputIndex =0, interStagePortIndex = 0, channels = 0;
	unsigned validIndex = 0;
	BDSP_P_ConnectionDetails *pStageConnectionDetails;
	BDSP_P_InterStagePortInfo *pInterStagePortInfo;
    BDSP_P_InterTaskBuffer    *pInterTaskBuffer;
	BDSP_RaagaQueue           *pQueueBuffer;
    BDSP_AF_P_sIoPort *pIOPort = NULL;
	unsigned DistinctFMMOutputCount[BDSP_AF_P_DistinctOpType_eMax] = {0};
    BDSP_P_PortDetails sPortDetails;

	BDBG_ENTER(BDSP_Raaga_P_PopulateIOConfiguration);
	pDevice = (BDSP_Raaga*)pRaagaStage->pContext->pDevice;
	BDBG_OBJECT_ASSERT(pDevice , BDSP_Raaga);
	dspIndex = pRaagaStage->pRaagaTask->createSettings.dspIndex;

    /* Reset the Port Deatils for Input*/
	for(inputIndex=0; inputIndex<BDSP_AF_P_MAX_IP_FORKS; inputIndex++)
	{
        pIOPort = (BDSP_AF_P_sIoPort *)&pStageIOConfig->sInputPort[inputIndex];
        pIOPort->ePortType     = BDSP_AF_P_PortType_eInvalid;
        pIOPort->ePortDataType = BDSP_AF_P_BufferType_eInvalid;
    }

    /* Reset the Port Deatils for Output*/
	for(outputIndex=0; outputIndex<BDSP_AF_P_MAX_OP_FORKS; outputIndex++)
	{
        pIOPort = (BDSP_AF_P_sIoPort *)&pStageIOConfig->sOutputPort[outputIndex];
        pIOPort->ePortType     = BDSP_AF_P_PortType_eInvalid;
        pIOPort->ePortDataType = BDSP_AF_P_BufferType_eInvalid;
    }

	for(inputIndex=0; inputIndex<BDSP_AF_P_MAX_IP_FORKS; inputIndex++)
	{
		pStageConnectionDetails = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[inputIndex];
		if(pStageConnectionDetails->eValid)
		{
			BDSP_RaagaStage *pSrcStage;
			pIOPort = (BDSP_AF_P_sIoPort *)&pStageIOConfig->sInputPort[validIndex];
			BKNI_Memset((void *)pIOPort,0, sizeof(BDSP_AF_P_sIoPort));
			BKNI_Memset((void *)&sPortDetails,0, sizeof(BDSP_P_PortDetails));
			switch(pStageConnectionDetails->eConnectionType)
			{
				case BDSP_ConnectionType_eStage:
					BDBG_MSG(("Connecting Stage at Input"));
					pSrcStage = (BDSP_RaagaStage *)pStageConnectionDetails->connectionHandle.stage.hStage->pStageHandle;
                    BDSP_P_GetDistinctOpTypeAndNumChans(pStageConnectionDetails->dataType, &channels, &sPortDetails.distinctOpType);
					BDSP_P_GetInterStagePortIndex(&pSrcStage->sStageConnectionInfo,
                        sPortDetails.distinctOpType,
                        &interStagePortIndex);
					pInterStagePortInfo = &pSrcStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_Four;
					sPortDetails.numDataBuffers       = 1;
                    sPortDetails.numTocBuffers        = 1;
                    sPortDetails.numMetaDataBuffers   = 1;
                    sPortDetails.numObjectDataBuffers = 1;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eInterStage;
					sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eLinear;
					sPortDetails.tocIndex             = pInterStagePortInfo->tocIndex;
					sPortDetails.numBranchFromPort    = pInterStagePortInfo->branchFromPort;
					sPortDetails.psDataAccesAttributes= &pInterStagePortInfo->dataAccessAttributes;
                    /* Descriptors are reused from the source Stage's output connection handle */
                    BKNI_Memcpy((void *)&sPortDetails.IoBuffer[0][0],
                        (void *)&pInterStagePortInfo->bufferDescriptorAddr[0][0],
                        (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
					break;
                case BDSP_ConnectionType_eInterTaskBuffer:
                    BDBG_MSG(("Connecting InterTask at Input"));
                    pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
                    sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_Four;
                    sPortDetails.numDataBuffers       = pInterTaskBuffer->numChannels;
                    sPortDetails.numTocBuffers        = pInterTaskBuffer->numTocData;
                    sPortDetails.numMetaDataBuffers   = pInterTaskBuffer->numMetaData;
                    sPortDetails.numObjectDataBuffers = pInterTaskBuffer->numObjectData;
                    sPortDetails.distinctOpType       = pInterTaskBuffer->distinctOp;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eInterTask;
                    sPortDetails.eBufferType          = pInterTaskBuffer->ebufferType;
                    sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
                    sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
                    sPortDetails.psDataAccesAttributes= NULL;
                    if(pInterTaskBuffer->descriptorAllocated)
                    {
                        /* If descriptors already allocated, then reuse*/
                        BKNI_Memcpy((void *)&sPortDetails.IoBuffer[0][0],
                            (void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                    }
                    else
                    {
                        BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                                dspIndex,
                                BDSP_ConnectionType_eInterTaskBuffer,
                                (void *)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle,
                                &sPortDetails);
                        BKNI_Memcpy((void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (void *)&sPortDetails.IoBuffer[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                        pInterTaskBuffer->descriptorAllocated = true;
                        pInterTaskBuffer->dspIndex            = dspIndex;
                    }
                    break;
				case BDSP_ConnectionType_eRaveBuffer:
					BDBG_MSG(("Connecting RAVE at Input"));
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
					sPortDetails.numDataBuffers       = 2;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
					sPortDetails.distinctOpType       = BDSP_AF_P_DistinctOpType_eCdb;
					sPortDetails.ePortType            = BDSP_AF_P_PortType_eRAVE;
					sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eRAVE;
					sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
					sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
					sPortDetails.psDataAccesAttributes= NULL;
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eRaveBuffer,
                            (void *)&pStageConnectionDetails->connectionHandle.rave.raveContextMap,
                            &sPortDetails);
					break;
				case BDSP_ConnectionType_eFmmBuffer:
					BDBG_MSG(("Connecting FMM at Input"));
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
					sPortDetails.numDataBuffers       = pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor.numBuffers;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
                    BDSP_P_GetDistinctOpTypeAndNumChans(pStageConnectionDetails->dataType, &channels, &sPortDetails.distinctOpType);
					sPortDetails.ePortType            = BDSP_AF_P_PortType_eFMM;
					sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eFMM;
					sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
					sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
					sPortDetails.psDataAccesAttributes= NULL;
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eFmmBuffer,
                            (void *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor,
                            &sPortDetails);
					break;
                case BDSP_ConnectionType_eRDBBuffer:
                    BDBG_MSG(("Connecting RDB at Input"));
                    pQueueBuffer = (BDSP_RaagaQueue *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle;
                    sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
                    sPortDetails.numDataBuffers       = pQueueBuffer->createSettings.numBuffers;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
                    sPortDetails.distinctOpType       = pQueueBuffer->distinctOp;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eRDB;
                    sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eRDB;
                    sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
                    sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
                    sPortDetails.psDataAccesAttributes= NULL;
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eRDBBuffer,
                            (void *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle,
                            &sPortDetails);
                    break;
				default:
					BDBG_ERR(("Connecting Invalid Buffer at input index = %d", inputIndex));
					break;
			}
			BDSP_Raaga_P_PopulatePortDetails(pIOPort,&sPortDetails);
			validIndex++;
		}
	}
	pStageIOConfig->ui32NumInputs = validIndex;

	validIndex = 0;
	for(outputIndex=0; outputIndex<BDSP_AF_P_MAX_OP_FORKS; outputIndex++)
	{
		pStageConnectionDetails = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[outputIndex];
		if(pStageConnectionDetails->eValid)
		{
			pIOPort = (BDSP_AF_P_sIoPort *)&pStageIOConfig->sOutputPort[validIndex];
			BKNI_Memset((void *)pIOPort,0, sizeof(BDSP_AF_P_sIoPort));
			BKNI_Memset((void *)&sPortDetails,0, sizeof(BDSP_P_PortDetails));
			switch(pStageConnectionDetails->eConnectionType)
			{
				case BDSP_ConnectionType_eStage:
					BDBG_MSG(("Connecting Stage at Output"));
					BDSP_P_GetInterStagePortIndex(&pRaagaStage->sStageConnectionInfo,
                        pRaagaStage->sStageConnectionInfo.eStageOpDataType[outputIndex],
                        &interStagePortIndex);
					pInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
					if(pInterStagePortInfo->tocIndex == BDSP_AF_P_TOC_INVALID)
					{
						pInterStagePortInfo->tocIndex = (*ptocIndex)++;
					}
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_Four;
					sPortDetails.numDataBuffers       = 1;
                    sPortDetails.numTocBuffers        = 1;
                    sPortDetails.numMetaDataBuffers   = 1;
                    sPortDetails.numObjectDataBuffers = 1;
					sPortDetails.distinctOpType       = pRaagaStage->sStageConnectionInfo.eStageOpDataType[outputIndex];
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eInterStage;
					sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eLinear;
					sPortDetails.tocIndex             = pInterStagePortInfo->tocIndex;
					sPortDetails.numBranchFromPort    = pInterStagePortInfo->branchFromPort;
					sPortDetails.psDataAccesAttributes= &pInterStagePortInfo->dataAccessAttributes;
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eStage,
                            NULL,
                            &sPortDetails);
                    BKNI_Memcpy((void *)&pInterStagePortInfo->bufferDescriptorAddr[0][0],
                        (void *)&sPortDetails.IoBuffer[0][0],
                        (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
					break;
                case BDSP_ConnectionType_eInterTaskBuffer:
                    BDBG_MSG(("Connecting InterTask at Output"));
                    pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
                    sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_Four;
                    sPortDetails.numDataBuffers       = pInterTaskBuffer->numChannels;
                    sPortDetails.numTocBuffers        = pInterTaskBuffer->numTocData;
                    sPortDetails.numMetaDataBuffers   = pInterTaskBuffer->numMetaData;
                    sPortDetails.numObjectDataBuffers = pInterTaskBuffer->numObjectData;
                    sPortDetails.distinctOpType       = pInterTaskBuffer->distinctOp;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eInterTask;
                    sPortDetails.eBufferType          = pInterTaskBuffer->ebufferType;
                    sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
                    sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
                    sPortDetails.psDataAccesAttributes= NULL;
                    if(pInterTaskBuffer->descriptorAllocated)
                    {
                        /* If descriptors already allocated, then reuse*/
                        BKNI_Memcpy((void *)&sPortDetails.IoBuffer[0][0],
                            (void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                    }
                    else
                    {
                        BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                                dspIndex,
                                BDSP_ConnectionType_eInterTaskBuffer,
                                (void *)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle,
                                &sPortDetails);
                        BKNI_Memcpy((void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (void *)&sPortDetails.IoBuffer[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                        pInterTaskBuffer->descriptorAllocated = true;
                        pInterTaskBuffer->dspIndex            = dspIndex;
                    }
                    break;
				case BDSP_ConnectionType_eFmmBuffer:
					BDBG_MSG(("Connecting FMM at Output"));
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
					sPortDetails.numDataBuffers       = pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor.numBuffers;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
                    sPortDetails.distinctOpType       = pRaagaStage->sStageConnectionInfo.eStageOpDataType[outputIndex];
					sPortDetails.ePortType            = BDSP_AF_P_PortType_eFMM;
					sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
					sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
					sPortDetails.psDataAccesAttributes= NULL;
                    if(0 != DistinctFMMOutputCount[sPortDetails.distinctOpType])
					{
						/*As long as same output is going to different FMM port, we can make the second one as slave in a Stage*/
						sPortDetails.eBufferType = BDSP_AF_P_BufferType_eFMMSlave;
					}
					else
					{
						sPortDetails.eBufferType = BDSP_AF_P_BufferType_eFMM;
					}
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eFmmBuffer,
                            (void *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor,
                            &sPortDetails);
					DistinctFMMOutputCount[sPortDetails.distinctOpType]++;
					break;
                case BDSP_ConnectionType_eRDBBuffer:
                    BDBG_MSG(("Connecting RDB at Output"));
                    pQueueBuffer = (BDSP_RaagaQueue *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle;
                    sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
                    sPortDetails.numDataBuffers       = pQueueBuffer->createSettings.numBuffers;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
                    sPortDetails.distinctOpType       = pQueueBuffer->distinctOp;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eRDB;
                    sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eRDB;
                    sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
                    sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
                    sPortDetails.psDataAccesAttributes= NULL;
                    BDSP_Raaga_P_CreatePortBufferDetails(pDevice,
                            dspIndex,
                            BDSP_ConnectionType_eRDBBuffer,
                            (void *)pStageConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle,
                            &sPortDetails);
                    break;
				default:
					BDBG_ERR(("Connecting Invalid Buffer at output index = %d", outputIndex));
					break;
			}
            BDSP_Raaga_P_PopulatePortDetails(pIOPort,&sPortDetails);
			validIndex++;
		}
	}
	pStageIOConfig->ui32NumOutputs = validIndex;

	BDBG_LEAVE(BDSP_Raaga_P_PopulateIOConfiguration);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_FillGlobalTaskConfig(
	BDSP_RaagaTask 				  *pRaagaTask,
	BDSP_AF_P_sGLOBAL_TASK_CONFIG *pGlobalTaskConfig
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	unsigned numStages = 0, scratchBufferSize = 0, interStageSize = 0;
	const BDSP_P_AlgorithmCodeInfo *psAlgoCodeInfo;
	const BDSP_P_AlgorithmInfo     *psAlgoInfo;

	BDBG_ENTER(BDSP_Raaga_P_FillGlobalTaskConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		psAlgoCodeInfo = BDSP_Raaga_P_LookupAlgorithmCodeInfo(pRaagaConnectStage->eAlgorithm);
        psAlgoInfo     = BDSP_P_LookupAlgorithmInfo(pRaagaConnectStage->eAlgorithm);
		if(psAlgoCodeInfo->scratchBufferSize> scratchBufferSize)
			scratchBufferSize = psAlgoCodeInfo->scratchBufferSize;
		if((psAlgoInfo->maxChannelsSupported*psAlgoInfo->samplesPerChannel*4) > interStageSize)
			interStageSize = (psAlgoInfo->maxChannelsSupported*psAlgoInfo->samplesPerChannel*4);
		numStages++;
	}
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

	pGlobalTaskConfig->ui32NumStagesInTask = numStages;
	pGlobalTaskConfig->ui32ScratchSize     = scratchBufferSize;
	pGlobalTaskConfig->ui32InterStageSize  = interStageSize;
	pGlobalTaskConfig->sDataSyncUserConfigInfo.BaseAddr = pRaagaPrimaryStage->stageMemInfo.sDataSyncSettings.Buffer.offset;
	pGlobalTaskConfig->sDataSyncUserConfigInfo.Size     = pRaagaPrimaryStage->stageMemInfo.sDataSyncSettings.ui32Size;
	pGlobalTaskConfig->sTsmConfigInfo.BaseAddr 		  	= pRaagaPrimaryStage->stageMemInfo.sTsmSettings.Buffer.offset;
	pGlobalTaskConfig->sTsmConfigInfo.Size     	 	  	= pRaagaPrimaryStage->stageMemInfo.sTsmSettings.ui32Size;
	pGlobalTaskConfig->sSchedulingInfo.BaseAddr 		= pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer.offset;
	pGlobalTaskConfig->sSchedulingInfo.Size     	 	= pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size;
	pGlobalTaskConfig->sGateOpenConfigInfo.BaseAddr 	= pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer.offset;
	pGlobalTaskConfig->sGateOpenConfigInfo.Size     	= pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size;
	pGlobalTaskConfig->sStcTriggerInfo.BaseAddr 	    = pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.Buffer.offset;
	pGlobalTaskConfig->sStcTriggerInfo.Size     	    = pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size;

	errCode = BDSP_Raaga_P_PopulateSchedulingConfig(pRaagaTask, pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_FillGlobalTaskConfig: Not able to populate the Scheduling configuration"));
		goto end;
	}

	errCode = BDSP_Raaga_P_PopulateGateOpenConfig(pRaagaTask, pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_FillGlobalTaskConfig: Not able to populate the Gate Open configuration"));
		goto end;
	}

    errCode = BDSP_P_PopulateStcTriggerConfig(&pRaagaTask->startSettings, pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillGlobalTaskConfig: Not able to populate the STC Trigger configuration"));
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_P_FillGlobalTaskConfig);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_FillStageConfig(
	BDSP_RaagaTask 			*pRaagaTask,
	BDSP_AF_P_sSTAGE_CONFIG *pStageConfig
)
{
    BERR_Code errCode = BERR_SUCCESS ;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	const BDSP_P_AlgorithmInfo *psAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo *psAlgoCodeInfo;
    BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo;
    BDSP_AF_P_EnableDisable eCollectResidual = BDSP_AF_P_eEnable;
	unsigned tocIndex = 0;

	BDBG_ENTER(BDSP_Raaga_P_FillStageConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pCodeInfo  = &pRaagaTask->pContext->pDevice->codeInfo;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		psAlgoInfo = BDSP_P_LookupAlgorithmInfo(pRaagaConnectStage->eAlgorithm);
        psAlgoCodeInfo = BDSP_Raaga_P_LookupAlgorithmCodeInfo(pRaagaConnectStage->eAlgorithm);

		pStageConfig->ui32StageId      = pRaagaConnectStage->stageID;
		pStageConfig->eAlgorithm       = pRaagaConnectStage->eAlgorithm;
		pStageConfig->eCollectResidual = eCollectResidual;
		pStageConfig->sStageMemoryInfo.BaseAddr    = pRaagaConnectStage->stageMemInfo.sMemoryPool.Memory.offset;
		pStageConfig->sStageMemoryInfo.Size        = pRaagaConnectStage->stageMemInfo.sMemoryPool.ui32Size;
		pStageConfig->sAlgoUserConfigInfo.BaseAddr = pRaagaConnectStage->stageMemInfo.sAlgoUserConfig.Buffer.offset;
		pStageConfig->sAlgoUserConfigInfo.Size     = psAlgoInfo->algoUserConfigSize;
		pStageConfig->sAlgoStatusInfo.BaseAddr 	   = pRaagaConnectStage->stageMemInfo.sAlgoStatus.Buffer.offset;
		pStageConfig->sAlgoStatusInfo.Size     	   = psAlgoInfo->algoStatusBufferSize;
		pStageConfig->sAlgoCodeInfo.BaseAddr       = pCodeInfo->imgInfo[BDSP_IMG_ID_CODE(pRaagaConnectStage->eAlgorithm)].Buffer.offset;
		pStageConfig->sAlgoCodeInfo.Size		   = pCodeInfo->imgInfo[BDSP_IMG_ID_CODE(pRaagaConnectStage->eAlgorithm)].ui32Size;/*Use the size of binary downloaded*/
		pStageConfig->sInterFrameInfo.BaseAddr 	   = pRaagaConnectStage->stageMemInfo.sInterframe.Buffer.offset;
		pStageConfig->sInterFrameInfo.Size     	   = psAlgoCodeInfo->interFrameSize;
		pStageConfig->sLookUpTableInfo.BaseAddr    = pCodeInfo->imgInfo[BDSP_IMG_ID_TABLE(pRaagaConnectStage->eAlgorithm)].Buffer.offset;
		pStageConfig->sLookUpTableInfo.Size		   = pCodeInfo->imgInfo[BDSP_IMG_ID_TABLE(pRaagaConnectStage->eAlgorithm)].ui32Size;/*Use the size of binary downloaded*/

		errCode = BDSP_Raaga_P_PopulateIOConfiguration(pRaagaConnectStage, &pStageConfig->sIOConfig, &tocIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_FillStageConfig: Error in Configuring the Stage IO for Stage(%d) %s",pRaagaConnectStage->eAlgorithm, psAlgoInfo->pName));
			BDBG_ASSERT(0);
		}

        BDSP_P_UpdateResidueCollection(&pRaagaConnectStage->sStageConnectionInfo.sStageOutput[0], &eCollectResidual);

		pStageConfig++;
	}
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

    BDBG_LEAVE(BDSP_Raaga_P_FillStageConfig);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_FillPrimaryStageInfo(
		BDSP_RaagaTask				 *pRaagaTask,
		BDSP_AF_P_sPRIMARYSTAGE_INFO *pPrimaryStageInfo
)
{
    BERR_Code errCode = BERR_SUCCESS ;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	const BDSP_P_AlgorithmInfo *psAlgoInfo;
	BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo;

	BDBG_ENTER(BDSP_Raaga_P_FillPrimaryStageInfo);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	psAlgoInfo = BDSP_P_LookupAlgorithmInfo(pRaagaPrimaryStage->eAlgorithm);
	pCodeInfo  = &pRaagaTask->pContext->pDevice->codeInfo;
	switch(pRaagaPrimaryStage->eAlgorithm)
    {
        case BDSP_Algorithm_eMpegAudioDecode:
        case BDSP_Algorithm_eMpegAudioPassthrough:
            pPrimaryStageInfo->ui32NumZeroFillSamples = 13824;
            break;
        case BDSP_Algorithm_eAc3Decode:
        case BDSP_Algorithm_eAc3Passthrough:
        case BDSP_Algorithm_eUdcPassthrough:
            pPrimaryStageInfo->ui32NumZeroFillSamples = 18432;
            break;
        default:
            pPrimaryStageInfo->ui32NumZeroFillSamples = 0;
            break;
    }
	switch(psAlgoInfo->type)
	{
		default:
		case BDSP_AlgorithmType_eAudioMixer:
		case BDSP_AlgorithmType_eAudioDecode:
			pPrimaryStageInfo->ePPMCorrectionEnable = pRaagaTask->startSettings.ppmCorrection;
			break;
		case BDSP_AlgorithmType_eAudioPassthrough:
		case BDSP_AlgorithmType_eAudioProcessing:
		case BDSP_AlgorithmType_eAudioEchoCanceller:
			pPrimaryStageInfo->ePPMCorrectionEnable = BDSP_AF_P_eDisable;
			break;
	}

	if(pRaagaTask->startSettings.pSampleRateMap)
	{
		BDSP_MMA_P_CopyDataToDram(&pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer,
			(void *)pRaagaTask->startSettings.pSampleRateMap,
			sizeof(BDSP_AF_P_sOpSamplingFreq));
	}
	else
	{
		BDBG_MSG(("BDSP_Raaga_P_FillPrimaryStageInfo: Default SAMPLERATE MAP programmed by BDSP"));
		BDSP_P_FillSamplingFrequencyLut(&pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer);
	}
	pPrimaryStageInfo->eOpenGateAtStart = pRaagaTask->startSettings.openGateAtStart;
	pPrimaryStageInfo->eTimeBaseType    = pRaagaTask->startSettings.timeBaseType;

	pPrimaryStageInfo->sTsmStatusInfo.BaseAddr 	    = pRaagaPrimaryStage->stageMemInfo.sTsmStatus.Buffer.offset;
	pPrimaryStageInfo->sTsmStatusInfo.Size          = psAlgoInfo->tsmStatusBufferSize;
	pPrimaryStageInfo->sDataSyncStatusInfo.BaseAddr = pRaagaPrimaryStage->stageMemInfo.sIdsStatus.Buffer.offset;
	pPrimaryStageInfo->sDataSyncStatusInfo.Size     = psAlgoInfo->idsStatusBufferSize;
	pPrimaryStageInfo->sIdsCodeInfo.BaseAddr 		= pCodeInfo->imgInfo[BDSP_IMG_ID_IDS(pRaagaPrimaryStage->eAlgorithm)].Buffer.offset;
	pPrimaryStageInfo->sIdsCodeInfo.Size    		= pCodeInfo->imgInfo[BDSP_IMG_ID_IDS(pRaagaPrimaryStage->eAlgorithm)].ui32Size;
	pPrimaryStageInfo->sSamplingFrequencyLutInfo.BaseAddr = pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer.offset;
	pPrimaryStageInfo->sSamplingFrequencyLutInfo.Size     = pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size;

	errCode = BDSP_Raaga_P_PopulateHwPPMConfig(pRaagaPrimaryStage, &pPrimaryStageInfo->sPPMConfig[0]);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_FillPrimaryStageInfo: Error in Populating the PPM Configuration"));
		goto end;
	}
end:
    BDBG_LEAVE(BDSP_Raaga_P_FillPrimaryStageInfo);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GenCit(
	BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_AF_P_sTASK_CONFIG *pTaskConfig;

	BDBG_ENTER(BDSP_Raaga_P_GenCit);
	pTaskConfig = (BDSP_AF_P_sTASK_CONFIG *)pRaagaTask->taskMemInfo.sCITMemory.Buffer.pAddr;

	errCode = BDSP_Raaga_P_FillPrimaryStageInfo(pRaagaTask, &pTaskConfig->sPrimaryStageInfo);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GenCit: Error in populating the Primary Stage Info"));
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_FillStageConfig(pRaagaTask, &pTaskConfig->sStageConfig[0]);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GenCit: Error in populating the Stage Config"));
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_FillGlobalTaskConfig(pRaagaTask, &pTaskConfig->sGlobalTaskConfig);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GenCit: Error in populating the Global Task Info"));
		BDBG_ASSERT(0);
	}

	BDSP_MMA_P_FlushCache(pRaagaTask->taskMemInfo.sCITMemory.Buffer,sizeof(BDSP_AF_P_sTASK_CONFIG));

	/* Copy the CIT structure to CIT Reconfig structure for runtime reconfiguration*/
	BDSP_MMA_P_CopyDataToDram(&pRaagaTask->taskMemInfo.sHostCITMemory.Buffer, pRaagaTask->taskMemInfo.sCITMemory.Buffer.pAddr, sizeof(BDSP_AF_P_sTASK_CONFIG));

	BDSP_Raaga_P_Analyse_CIT(pRaagaTask, false);

    BDBG_LEAVE(BDSP_Raaga_P_GenCit);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReconfigCit(
	BDSP_RaagaStage 		 *pRaagaStage,
	bool					  bInputAdded,
	BDSP_P_ConnectionDetails *pStageConnectionDetails,
	unsigned                  index
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask;
	BDSP_AF_P_sTASK_CONFIG *pTaskConfig;
	BDSP_P_CitReconfigCommand Payload;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	unsigned stageIndex = 0, channels = 0;
    BDSP_P_PortDetails sPortDetails;
	BDSP_AF_P_sIoPort *pIOPort;
	BDSP_AF_P_sIOConfig *pStageIOConfig;
    BDSP_P_InterTaskBuffer *pInterTaskBuffer;

	BDBG_ENTER(BDSP_Raaga_P_ReconfigCit);
	pRaagaTask = pRaagaStage->pRaagaTask;
	pTaskConfig = (BDSP_AF_P_sTASK_CONFIG *)pRaagaTask->taskMemInfo.sHostCITMemory.Buffer.pAddr;
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		if((pRaagaConnectStage->stageID == pRaagaStage->stageID)&&
		   (pRaagaConnectStage->eAlgorithm == pRaagaStage->eAlgorithm))
		{
			pStageIOConfig = &pTaskConfig->sStageConfig[stageIndex].sIOConfig;
			if(bInputAdded)
			{
				pIOPort = &pStageIOConfig->sInputPort[index];
				pStageIOConfig->ui32NumInputs++;
                BKNI_Memset((void *)pIOPort,0, sizeof(BDSP_AF_P_sIoPort));
                BKNI_Memset((void *)&sPortDetails,0, sizeof(BDSP_P_PortDetails));
			}
			else
			{
			    /* Just reset the Port details and decrement the number of inputs*/
				pIOPort = &pStageIOConfig->sInputPort[index];
				pStageIOConfig->ui32NumInputs--;
                BKNI_Memset((void *)pIOPort,0, sizeof(BDSP_AF_P_sIoPort));
                pIOPort->ePortType     = BDSP_AF_P_PortType_eInvalid;
                pIOPort->ePortDataType = BDSP_AF_P_BufferType_eInvalid;
                break;
			}
			switch(pStageConnectionDetails->eConnectionType)
			{
				case BDSP_ConnectionType_eFmmBuffer:
					BDBG_MSG(("Reconfig CIT: Connecting FMM at Input"));
					sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_One;
					sPortDetails.numDataBuffers       = pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor.numBuffers;
                    sPortDetails.numTocBuffers        = 0;
                    sPortDetails.numMetaDataBuffers   = 0;
                    sPortDetails.numObjectDataBuffers = 0;
                    BDSP_P_GetDistinctOpTypeAndNumChans(pStageConnectionDetails->dataType, &channels, &sPortDetails.distinctOpType);
					sPortDetails.ePortType            = BDSP_AF_P_PortType_eFMM;
					sPortDetails.eBufferType          = BDSP_AF_P_BufferType_eFMM;
					sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
					sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
					sPortDetails.psDataAccesAttributes= NULL;
                    BDSP_Raaga_P_CreatePortBufferDetails(pRaagaTask->pContext->pDevice,
                            pRaagaTask->createSettings.dspIndex,
                            BDSP_ConnectionType_eFmmBuffer,
                            (void *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor,
                            &sPortDetails);
					break;
                case BDSP_ConnectionType_eInterTaskBuffer:
					BDBG_MSG(("Reconfig CIT: Connecting InterTask at Input"));
                    pInterTaskBuffer = (BDSP_P_InterTaskBuffer*)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
                    sPortDetails.numPortBuffers       = BDSP_AF_P_NumPortBuffers_Four;
                    sPortDetails.numDataBuffers       = pInterTaskBuffer->numChannels;
                    sPortDetails.numTocBuffers        = pInterTaskBuffer->numTocData;
                    sPortDetails.numMetaDataBuffers   = pInterTaskBuffer->numMetaData;
                    sPortDetails.numObjectDataBuffers = pInterTaskBuffer->numObjectData;
                    sPortDetails.distinctOpType       = pInterTaskBuffer->distinctOp;
                    sPortDetails.ePortType            = BDSP_AF_P_PortType_eInterTask;
                    sPortDetails.eBufferType          = pInterTaskBuffer->ebufferType;
                    sPortDetails.tocIndex             = BDSP_AF_P_TOC_INVALID;
                    sPortDetails.numBranchFromPort    = BDSP_AF_P_BRANCH_INVALID;
                    sPortDetails.psDataAccesAttributes= NULL;
                    if(pInterTaskBuffer->descriptorAllocated)
                    {
                        /* If descriptors already allocated, then reuse*/
                        BKNI_Memcpy((void *)&sPortDetails.IoBuffer[0][0],
                            (void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                    }
                    else
                    {
                        BDSP_Raaga_P_CreatePortBufferDetails(pRaagaTask->pContext->pDevice,
                                pRaagaTask->createSettings.dspIndex,
                                BDSP_ConnectionType_eInterTaskBuffer,
                                (void *)pStageConnectionDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle,
                                &sPortDetails);
                        BKNI_Memcpy((void *)&pInterTaskBuffer->bufferDescriptorAddr[0][0],
                            (void *)&sPortDetails.IoBuffer[0][0],
                            (BDSP_AF_P_MAX_PORT_BUFFERS*BDSP_AF_P_MAX_CHANNELS*sizeof(dramaddr_t)));
                        pInterTaskBuffer->descriptorAllocated = true;
                        pInterTaskBuffer->dspIndex            = pRaagaTask->createSettings.dspIndex;
                    }
                    break;
				default:
					BDBG_ERR(("BDSP_Raaga_P_ReconfigCit: CIT Reconfigure for the Connection type(%d) requested not supported",pStageConnectionDetails->eConnectionType));
					BDBG_ASSERT(0);
					break;
			}
            BDSP_Raaga_P_PopulatePortDetails(pIOPort,&sPortDetails);
			break;
		}
		stageIndex++;
	}
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

	BDSP_MMA_P_FlushCache(pRaagaTask->taskMemInfo.sHostCITMemory.Buffer,sizeof(BDSP_AF_P_sTASK_CONFIG));
	BDSP_Raaga_P_Analyse_CIT(pRaagaTask, true);

	Payload.sFwConfigMemoryInfo.BaseAddr   = pRaagaTask->taskMemInfo.sCITMemory.Buffer.offset;
	Payload.sFwConfigMemoryInfo.Size       = pRaagaTask->taskMemInfo.sCITMemory.ui32Size;
	Payload.sHostConfigMemoryInfo.BaseAddr = pRaagaTask->taskMemInfo.sHostCITMemory.Buffer.offset;
	Payload.sHostConfigMemoryInfo.Size     = pRaagaTask->taskMemInfo.sHostCITMemory.ui32Size;

	errCode = BDSP_Raaga_P_ProcessCITReConfigCommand(pRaagaTask, &Payload);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReconfigCit: Error in Reconfig CIT command processing for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

end:
    BDBG_LEAVE(BDSP_Raaga_P_ReconfigCit);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_CleanupDescriptors(
	BDSP_RaagaStage 	*pRaagaStage,
	BDSP_AF_P_sIOConfig *psIOConfig
)
{
    BERR_Code errCode = BERR_SUCCESS;
	unsigned index=0,i=0,j=0, interStagePortIndex=0, interTaskPortIndex = 0;
	BDSP_P_InterStagePortInfo *psInterStagePortInfo;
    BDSP_P_InterTaskBuffer    *pInterTaskBuffer;
	BDSP_AF_P_sIoPort *psIoPort;

	BDBG_ENTER(BDSP_Raaga_P_CleanupDescriptors);
	for(index=0; index<psIOConfig->ui32NumInputs; index++)
	{
		psIoPort = (BDSP_AF_P_sIoPort *)&psIOConfig->sInputPort[index];
		switch(psIoPort->ePortType)
		{
			case BDSP_AF_P_PortType_eFMM:
            case BDSP_AF_P_PortType_eRAVE:
            case BDSP_AF_P_PortType_eRDB:
				errCode = BDSP_Raaga_P_ReleasePortDescriptors(pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						psIoPort);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
					goto end;
				}
				break;
			case BDSP_AF_P_PortType_eInterStage:
				BDBG_MSG(("Descriptor is owned by the source stage as it is connected at input"));
				break;
            case BDSP_AF_P_PortType_eInterTask:
                BDBG_MSG(("Descriptor is cleaned up the source task's cleanup"));
                break;
			default:
				BDBG_ERR(("Cleanup descriptor at Input, Port type not supported"));
				break;
		}
	}

	for(index=0; index<psIOConfig->ui32NumOutputs; index++)
	{
		psIoPort = (BDSP_AF_P_sIoPort *)&psIOConfig->sOutputPort[index];
		switch(psIoPort->ePortType)
		{
			case BDSP_AF_P_PortType_eFMM:
            case BDSP_AF_P_PortType_eRAVE:
            case BDSP_AF_P_PortType_eRDB:
				errCode = BDSP_Raaga_P_ReleasePortDescriptors(pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						psIoPort);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
					goto end;
				}
				break;
			case BDSP_AF_P_PortType_eInterStage:
				BDSP_P_GetInterStagePortIndex(&pRaagaStage->sStageConnectionInfo,
                    psIoPort->ePortDataType,
                    &interStagePortIndex);
				psInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
				psInterStagePortInfo->tocIndex = BDSP_AF_P_TOC_INVALID;
				errCode = BDSP_Raaga_P_ReleasePortDescriptors(pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						psIoPort);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
					goto end;
				}
				for(j=0;j<psIoPort->ui32numPortBuffer;j++)
				{
					for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
					{
						psInterStagePortInfo->bufferDescriptorAddr[j][i] = 0;
					}
				}
				break;
            case BDSP_AF_P_PortType_eInterTask:
                BDSP_P_GetInterTaskPortIndex((void *)pRaagaStage,
                    &pRaagaStage->sStageConnectionInfo,
                    psIoPort->ePortDataType,
                    &interTaskPortIndex);
                pInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pRaagaStage->sStageConnectionInfo.sStageOutput[interTaskPortIndex].connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
				errCode = BDSP_Raaga_P_ReleasePortDescriptors(pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						psIoPort);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
					goto end;
				}
				for(j=0;j<psIoPort->ui32numPortBuffer;j++)
				{
					for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
					{
						pInterTaskBuffer->bufferDescriptorAddr[j][i]=0;
					}
				}
				pInterTaskBuffer->descriptorAllocated=false;
                break;
			default:
				BDBG_ERR(("Cleanup descriptor at Output, Port type not supported"));
				break;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_CleanupDescriptors);
	return errCode;
}

BERR_Code BDSP_Raaga_P_CleanupCit(
	BDSP_RaagaTask *pRaagaTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_AF_P_sTASK_CONFIG *pTaskConfig;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	BDSP_AF_P_sSTAGE_CONFIG *pStageConfig;

	BDBG_ENTER(BDSP_Raaga_P_CleanupCit);
	pTaskConfig = (BDSP_AF_P_sTASK_CONFIG *)pRaagaTask->taskMemInfo.sHostCITMemory.Buffer.pAddr;
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pStageConfig = &pTaskConfig->sStageConfig[0];

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		errCode = BDSP_Raaga_P_CleanupDescriptors(pRaagaConnectStage, &pStageConfig->sIOConfig);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_CleanupCit: Error in Cleaup Descriptor for Algorithm(%d)",pRaagaConnectStage->eAlgorithm));
			goto end;
		}
		pStageConfig++;
	}
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

end:
	BDBG_LEAVE(BDSP_Raaga_P_CleanupCit);
	return errCode;
}
