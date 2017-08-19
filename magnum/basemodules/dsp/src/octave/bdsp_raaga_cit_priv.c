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

#include "bdsp_raaga_cit_priv.h"

BDBG_MODULE(bdsp_cit_priv);

void BDSP_Raaga_P_GetInterStagePortIndex(
	BDSP_RaagaStage          *pRaagaStage,
	BDSP_AF_P_DistinctOpType  eDistinctOpType,
	unsigned                 *pIndex
)
{
	int32_t index = 0;
	unsigned firstFreeIndex = BDSP_AF_P_MAX_OP_FORKS;
	BDBG_ENTER(BDSP_Raaga_P_GetInterStagePortIndex);
	for(index=(BDSP_AF_P_MAX_OP_FORKS-1); index >= 0; index--)
	{
		if(pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[index].ePortDataType == eDistinctOpType)
		{
			*pIndex = index;
			goto end;
		}
		if(pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[index].ePortDataType == BDSP_AF_P_DistinctOpType_eMax)
		{
			firstFreeIndex = index;
		}
	}
	if(firstFreeIndex == BDSP_AF_P_MAX_OP_FORKS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GetInterStagePortIndex: Not able to derive the index"));
		BDBG_ASSERT(0);
	}
	else
	{
		*pIndex = firstFreeIndex;
	}
end:
	BDBG_MSG(("InterStage Port Index = %d",*pIndex));
	BDBG_LEAVE(BDSP_Raaga_P_GetInterStagePortIndex);
}


static void BDSP_P_FillSamplingFrequencyLut(
	BDSP_MMA_Memory *pLUTMemory
)
{
    BDSP_AF_P_sOpSamplingFreq sOpSamplingFrequencyMapLut =  {
																{ /*QSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*HSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*FSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*HSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*VHSF */
                                                                  32000,
                                                                  44100,
                                                                  48000
                                                                }
                                                            };

    BDBG_ENTER(BDSP_P_FillSamplingFrequencyLut);

	BDSP_MMA_P_CopyDataToDram(pLUTMemory,
		(void *)&sOpSamplingFrequencyMapLut,
		sizeof(BDSP_AF_P_sOpSamplingFreq));

	BDBG_LEAVE(BDSP_P_FillSamplingFrequencyLut);
	return;
}

static void BDSP_Raaga_P_GetFMMDetails(
	BDSP_AF_P_DistinctOpType eOutputType,
	BDSP_Algorithm			 eAlgorithm,
    BDSP_AF_P_FmmDstFsRate   *pBaseRateMultiplier,
    BDSP_AF_P_FmmContentType *pFMMContentType
)
{
	switch (eOutputType)
	{
		case BDSP_AF_P_DistinctOpType_e7_1_PCM:
		case BDSP_AF_P_DistinctOpType_e5_1_PCM:
		case BDSP_AF_P_DistinctOpType_eStereo_PCM:
		case BDSP_AF_P_DistinctOpType_eMono_PCM:
			*pFMMContentType 	 = BDSP_AF_P_FmmContentType_ePcm;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressed:
		case BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eCompressed;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressed4x:
		/*SW7425-6056: It is based on ui32FMMContentType that the FW decides on a type of zero fill.
		Without this check, Spdif preambles were filled in during a zero fill for BTSC
		SWRAA-162: New FMM Dest type added to address the same*/
			if (eAlgorithm == BDSP_Algorithm_eBtscEncoder)
			{
				*pFMMContentType = BDSP_AF_P_FmmContentType_eAnalogCompressed;
			}else
			{
				*pFMMContentType = BDSP_AF_P_FmmContentType_eCompressed;
			}
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_e4xBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressedHBR:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eCompressed;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_e16xBaseRate;
			break;
		default:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eInvalid;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eInvalid;
			break;
	}

}

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
						psConfig->PPMCfgAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(wrcnt);
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
	unsigned numBuffers
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
				for(i=0;i<numBuffers;i++)
				{
					pCircularBuffer->BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->CDB_Base);
					pCircularBuffer->EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->CDB_End);
					pCircularBuffer->ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->CDB_Read);
					pCircularBuffer->WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->CDB_Valid);
					pCircularBuffer->WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->CDB_Wrap);
					i=1+1;
					pCircularBuffer++;
					pCircularBuffer->BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->ITB_Base);
					pCircularBuffer->EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->ITB_End);
					pCircularBuffer->ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->ITB_Read);
					pCircularBuffer->WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->ITB_Valid);
					pCircularBuffer->WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pRaveContextMap->ITB_Wrap);
					pCircularBuffer++;
				}
			}
			break;
		case BDSP_ConnectionType_eFmmBuffer:
			{
				BDSP_FmmBufferDescriptor *pFMMDescriptor = (BDSP_FmmBufferDescriptor *)pDescriptor;
				for( i=0;i<numBuffers;i++)
				{
					pCircularBuffer->BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].base);
					pCircularBuffer->EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].end);
					pCircularBuffer->ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].read);
					pCircularBuffer->WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].write);
					pCircularBuffer->WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pFMMDescriptor->buffers[i].end);
					pCircularBuffer++;
				}
			}
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

				BDSP_Raaga_P_GetFMMDetails(pRaagaConnectStage->sStageConnectionInfo.eStageOpDataType[index],
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
					pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.numBuffers);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_PopulateGateOpenConfig: Unable to populate the descriptor for the Gate Open buffer"));
					goto end;
				}
				for(i=0;i<pRingbuffer->ui32NumBuffers;i++)
				{
					pRingbuffer->sExtendedBuffer[i].sCircularBuffer= sCircularBuffer[i];
					pRingbuffer->sExtendedBuffer[i].startWriteAddr = pRingbuffer->sExtendedBuffer[i].sCircularBuffer.ReadAddr + (5 * sizeof(dramaddr_t));
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
    BDSP_AF_P_FmmDstFsRate eBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eInvalid;
    BDSP_AF_P_FmmContentType eFMMContentType = BDSP_AF_P_FmmContentType_eInvalid;
	unsigned index =0, independentDelay =0;
	BDSP_AF_P_sTASK_SCHEDULING_CONFIG *pSchedulingConfig;

	BDBG_ENTER(BDSP_Raaga_P_PopulateSchedulingConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pSchedulingConfig = (BDSP_AF_P_sTASK_SCHEDULING_CONFIG *)Memory.pAddr;

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
				BDSP_Raaga_P_GetFMMDetails(pRaagaConnectStage->sStageConnectionInfo.eStageOpDataType[index],
					pRaagaConnectStage->eAlgorithm,
					&eBaseRateMultiplier,
					&eFMMContentType);
				errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
					(void *)&pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor,
					BDSP_ConnectionType_eFmmBuffer,
					&pSchedulingConfig->sExtendedBuffer.sCircularBuffer,
					1);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_PopulateSchedulingConfig: Unable to populate the descriptor for the Scheduling buffer"));
					goto end;
				}
				pSchedulingConfig->sExtendedBuffer.startWriteAddr = pSchedulingConfig->sExtendedBuffer.sCircularBuffer.ReadAddr + (5 * sizeof(dramaddr_t));
				independentDelay = pRaagaConnectStage->sStageConnectionInfo.sStageOutput[index].connectionHandle.fmm.fmmDescriptor.delay;
				goto next_step;
			}
		}
	}
	BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

next_step:
	pSchedulingConfig->sSchedulingInfo.ui32MaxIndependentDelay = pRaagaTask->startSettings.maxIndependentDelay;
	pSchedulingConfig->sSchedulingInfo.eFMMContentType         = eFMMContentType;
	pSchedulingConfig->sSchedulingInfo.eBaseRateMultiplier     = eBaseRateMultiplier;
	pSchedulingConfig->sSchedulingInfo.bFixedSampleRate        = BDSP_AF_P_Boolean_eFalse;
	pSchedulingConfig->sSchedulingInfo.ui32FixedSampleRate     = 0;
	pSchedulingConfig->sSchedulingInfo.ui32BlockingTime        = BDSP_AF_P_BLOCKING_TIME;
	pSchedulingConfig->sSchedulingInfo.ui32IndependentDelay    = independentDelay;

	BDSP_MMA_P_FlushCache(Memory,sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG));

end:
	BDBG_LEAVE(BDSP_Raaga_P_PopulateSchedulingConfig);
	return errCode;
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
	unsigned validIndex = 0 , i =0, j=0;
    BDSP_AF_P_DistinctOpType distinctOpType;
	BDSP_P_ConnectionDetails *pStageConnectionDetails;
	BDSP_P_InterStagePortInfo *pInterStagePortInfo;
	BDSP_AF_P_sIoPort *pIOPort;
	BDSP_MMA_Memory descriptorMemory[BDSP_AF_P_MAX_CHANNELS];
	BDSP_AF_P_sCIRCULAR_BUFFER *pDescriptor[BDSP_AF_P_MAX_CHANNELS];
	unsigned DistinctFMMOutputCount[BDSP_AF_P_DistinctOpType_eMax] = {0};

	BDBG_ENTER(BDSP_Raaga_P_PopulateIOConfiguration);
	pDevice = (BDSP_Raaga*)pRaagaStage->pContext->pDevice;
	BDBG_OBJECT_ASSERT(pDevice , BDSP_Raaga);
	dspIndex = pRaagaStage->pRaagaTask->createSettings.dspIndex;

	for(inputIndex=0; inputIndex<BDSP_AF_P_MAX_IP_FORKS; inputIndex++)
	{
		pStageConnectionDetails = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[inputIndex];
		if(pStageConnectionDetails->eValid)
		{
			BDSP_RaagaStage *pSrcStage;
			pIOPort = (BDSP_AF_P_sIoPort *)&pStageIOConfig->sInputPort[validIndex];
			BKNI_Memset((void *)pIOPort,0, sizeof(BDSP_AF_P_sIoPort));
			BKNI_Memset((void *)&descriptorMemory[0],0, (sizeof(BDSP_MMA_Memory)*BDSP_AF_P_MAX_CHANNELS));
			switch(pStageConnectionDetails->eConnectionType)
			{
				case BDSP_ConnectionType_eStage:
					pSrcStage = (BDSP_RaagaStage *)pStageConnectionDetails->connectionHandle.stage.hStage->pStageHandle;
					BDSP_P_GetDistinctOpTypeAndNumChans(pStageConnectionDetails->dataType, &channels, &distinctOpType);
					pIOPort->ePortType     = BDSP_AF_P_PortType_eInterStage;
					pIOPort->ui32numPortBuffer    = BDSP_AF_P_NumPortBuffers_Four;
					pIOPort->sIOGenericBuffer     = 0;
					pIOPort->ePortDataType        = distinctOpType;
					BDSP_Raaga_P_GetInterStagePortIndex(pSrcStage, distinctOpType,&interStagePortIndex);
					pInterStagePortInfo = &pSrcStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
					for(j=0;j<pIOPort->ui32numPortBuffer;j++)
					{
						/* Descriptors are reused from the source Stage's output connection handle */
						BDSP_P_ConnectionDetails *psSrcStageOutput;
						pIOPort->sIoBuffer[j].eBufferType = BDSP_AF_P_BufferType_eLinear;
						pIOPort->sIoBuffer[j].ui32NumBuffer = 1;
						for(outputIndex=0;outputIndex<BDSP_AF_P_MAX_OP_FORKS;outputIndex++)
						{
							psSrcStageOutput = (BDSP_P_ConnectionDetails *)&pSrcStage->sStageConnectionInfo.sStageOutput[outputIndex];
							if((psSrcStageOutput->eConnectionType == BDSP_ConnectionType_eStage)&&(psSrcStageOutput->eValid==BDSP_AF_P_eValid))
							{
								for(i=0;i<pIOPort->sIoBuffer[j].ui32NumBuffer;i++)
								{
									/* We will always have one buffer per port buffer, if required make the descriptor into 2D array */
									pIOPort->sIoBuffer[j].sCircularBuffer[i] = pInterStagePortInfo->bufferDescriptorAddr[j];
								}
							}
						}
					}
					pIOPort->sDataAccessAttributes = pInterStagePortInfo->dataAccessAttributes;
					pIOPort->ui32numBranchfromPort = pInterStagePortInfo->branchFromPort;
					pIOPort->ui32tocIndex          = pInterStagePortInfo->tocIndex;
					break;
				case BDSP_ConnectionType_eRaveBuffer:
					pIOPort->ePortType	   = BDSP_AF_P_PortType_eRAVE;
					pIOPort->ePortDataType = BDSP_AF_P_DistinctOpType_eCdb;
					pIOPort->ui32numPortBuffer	  = BDSP_AF_P_NumPortBuffers_One;
					pIOPort->ui32numBranchfromPort= BDSP_AF_P_BRANCH_INVALID;
					pIOPort->ui32tocIndex		  = BDSP_AF_P_TOC_INVALID;
					pIOPort->sIOGenericBuffer	  = 0;
					for(j=0;j<pIOPort->ui32numPortBuffer;j++)
					{
						pIOPort->sIoBuffer[j].eBufferType = BDSP_AF_P_BufferType_eRAVE;
						pIOPort->sIoBuffer[j].ui32NumBuffer = 2;
						BDSP_Raaga_P_AssignDescriptor((void *)pDevice,
							dspIndex,
							&descriptorMemory[0],
							pIOPort->sIoBuffer[j].ui32NumBuffer);
						for(i=0;i<pIOPort->sIoBuffer[j].ui32NumBuffer;i++)
						{
							pDescriptor[i] = (BDSP_AF_P_sCIRCULAR_BUFFER *)descriptorMemory[i].pAddr;
						}
						errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
							(void *)&pStageConnectionDetails->connectionHandle.rave.raveContextMap,
							BDSP_ConnectionType_eRaveBuffer,
							pDescriptor[0],
							pIOPort->sIoBuffer[j].ui32NumBuffer);
						if(errCode != BERR_SUCCESS)
						{
							BDBG_ERR(("Input RAVE Port Buffer Descriptor Not configured properly"));
						}
						for(i=0;i<pIOPort->sIoBuffer[j].ui32NumBuffer;i++)
						{
							pIOPort->sIoBuffer[j].sCircularBuffer[i] = descriptorMemory[i].offset;
							BDSP_MMA_P_FlushCache(descriptorMemory[i], sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
						}
					}
					break;
				default:
					BDBG_ERR(("Connecting Invalid Buffer at input index = %d", inputIndex));
					break;
			}
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
			BKNI_Memset((void *)&descriptorMemory[0],0, (sizeof(BDSP_MMA_Memory)*BDSP_AF_P_MAX_CHANNELS));
			switch(pStageConnectionDetails->eConnectionType)
			{
				case BDSP_ConnectionType_eStage:
					pIOPort->ePortType     = BDSP_AF_P_PortType_eInterStage;
					pIOPort->ePortDataType = pRaagaStage->sStageConnectionInfo.eStageOpDataType[outputIndex];
					pIOPort->ui32numPortBuffer    = BDSP_AF_P_NumPortBuffers_Four;
					pIOPort->sIOGenericBuffer     = 0;
					BDSP_Raaga_P_GetInterStagePortIndex(pRaagaStage,pIOPort->ePortDataType,&interStagePortIndex);
					pInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
					for(j=0;j<pIOPort->ui32numPortBuffer;j++)
					{
						pIOPort->sIoBuffer[j].eBufferType = BDSP_AF_P_BufferType_eLinear;
						pIOPort->sIoBuffer[j].ui32NumBuffer = 1;
						for(i=0;i<pIOPort->sIoBuffer[j].ui32NumBuffer;i++)
						{
							BDSP_Raaga_P_AssignDescriptor((void *)pDevice,
								dspIndex,
								&descriptorMemory[0],
								pIOPort->sIoBuffer[0].ui32NumBuffer);
							pIOPort->sIoBuffer[j].sCircularBuffer[i] = descriptorMemory[i].offset;
							pInterStagePortInfo->bufferDescriptorAddr[j] = pIOPort->sIoBuffer[j].sCircularBuffer[i];
						}
					}
					pIOPort->sDataAccessAttributes = pInterStagePortInfo->dataAccessAttributes;
					pIOPort->ui32numBranchfromPort = pInterStagePortInfo->branchFromPort;
					if(pInterStagePortInfo->tocIndex == BDSP_AF_P_TOC_INVALID)
					{
						pInterStagePortInfo->tocIndex = *ptocIndex++;
					}
					pIOPort->ui32tocIndex = pInterStagePortInfo->tocIndex;
					break;
				case BDSP_ConnectionType_eFmmBuffer:
					pIOPort->ePortType     = BDSP_AF_P_PortType_eFMM;
					pIOPort->ePortDataType = pRaagaStage->sStageConnectionInfo.eStageOpDataType[outputIndex];
					pIOPort->ui32numPortBuffer    = BDSP_AF_P_NumPortBuffers_One;
					pIOPort->ui32numBranchfromPort= BDSP_AF_P_BRANCH_INVALID;
					pIOPort->ui32tocIndex         = BDSP_AF_P_TOC_INVALID;
					pIOPort->sIOGenericBuffer     = 0;

					for(j=0;j<pIOPort->ui32numPortBuffer;j++)
					{
						if(0 != DistinctFMMOutputCount[pIOPort->ePortDataType])
						{
							/*As long as same output is going to different FMM port, we can make the second one as slave in a Stage*/
							pIOPort->sIoBuffer[j].eBufferType = BDSP_AF_P_BufferType_eFMMSlave;
						}
						else
						{
							pIOPort->sIoBuffer[j].eBufferType = BDSP_AF_P_BufferType_eFMM;
						}
						pIOPort->sIoBuffer[j].ui32NumBuffer = pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor.numBuffers;
						BDSP_Raaga_P_AssignDescriptor((void *)pDevice,
							dspIndex,
							&descriptorMemory[0],
							pIOPort->sIoBuffer[j].ui32NumBuffer);
						for(i=0;i<pIOPort->sIoBuffer[0].ui32NumBuffer;i++)
						{
							pDescriptor[i] = (BDSP_AF_P_sCIRCULAR_BUFFER *)descriptorMemory[i].pAddr;
						}
						errCode = BDSP_Raaga_P_PopulatePortBufferDescriptors(
							(void *)&pStageConnectionDetails->connectionHandle.fmm.fmmDescriptor,
							BDSP_ConnectionType_eFmmBuffer,
							pDescriptor[0],
							pIOPort->sIoBuffer[j].ui32NumBuffer);
						if(errCode != BERR_SUCCESS)
						{
							BDBG_ERR(("Output FMM Port Buffer Descriptor Not configured properly"));
						}
						for(i=0;i<pIOPort->sIoBuffer[j].ui32NumBuffer;i++)
						{
							pIOPort->sIoBuffer[j].sCircularBuffer[i] = descriptorMemory[i].offset;
							BDSP_MMA_P_FlushCache(descriptorMemory[i], sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
						}
					}
					DistinctFMMOutputCount[pIOPort->ePortDataType]++;
					break;
				default:
					BDBG_ERR(("Connecting Invalid Buffer at output index = %d", outputIndex));
					break;
			}
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
	const BDSP_P_AlgorithmInfo *psAlgoInfo;

	BDBG_ENTER(BDSP_Raaga_P_FillGlobalTaskConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		psAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->eAlgorithm);
		if(psAlgoInfo->scratchBufferSize > scratchBufferSize)
			scratchBufferSize = psAlgoInfo->scratchBufferSize;
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
	BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo;
	unsigned tocIndex = 0;

	BDBG_ENTER(BDSP_Raaga_P_FillStageConfig);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	pCodeInfo  = &pRaagaTask->pContext->pDevice->codeInfo;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		psAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->eAlgorithm);

		pStageConfig->ui32StageId      = pRaagaConnectStage->stageID;
		pStageConfig->eAlgorithm       = pRaagaConnectStage->eAlgorithm;
		pStageConfig->eCollectResidual = BDSP_AF_P_eDisable;
		pStageConfig->sStageMemoryInfo.BaseAddr    = pRaagaConnectStage->stageMemInfo.sMemoryPool.Memory.offset;
		pStageConfig->sStageMemoryInfo.Size        = pRaagaConnectStage->stageMemInfo.sMemoryPool.ui32Size;
		pStageConfig->sAlgoUserConfigInfo.BaseAddr = pRaagaConnectStage->stageMemInfo.sAlgoUserConfig.Buffer.offset;
		pStageConfig->sAlgoUserConfigInfo.Size     = psAlgoInfo->algoUserConfigSize;
		pStageConfig->sAlgoStatusInfo.BaseAddr 	   = pRaagaConnectStage->stageMemInfo.sAlgoStatus.Buffer.offset;
		pStageConfig->sAlgoStatusInfo.Size     	   = psAlgoInfo->algoStatusBufferSize;
		pStageConfig->sAlgoCodeInfo.BaseAddr       = pCodeInfo->imgInfo[BDSP_IMG_ID_CODE(pRaagaConnectStage->eAlgorithm)].Buffer.offset;
		pStageConfig->sAlgoCodeInfo.Size		   = pCodeInfo->imgInfo[BDSP_IMG_ID_CODE(pRaagaConnectStage->eAlgorithm)].ui32Size;/*Use the size of binary downloaded*/
		pStageConfig->sInterFrameInfo.BaseAddr 	   = pRaagaConnectStage->stageMemInfo.sInterframe.Buffer.offset;
		pStageConfig->sInterFrameInfo.Size     	   = psAlgoInfo->interFrameSize;
		pStageConfig->sLookUpTableInfo.BaseAddr    = pCodeInfo->imgInfo[BDSP_IMG_ID_TABLE(pRaagaConnectStage->eAlgorithm)].Buffer.offset;
		pStageConfig->sLookUpTableInfo.Size		   = pCodeInfo->imgInfo[BDSP_IMG_ID_TABLE(pRaagaConnectStage->eAlgorithm)].ui32Size;/*Use the size of binary downloaded*/

		errCode = BDSP_Raaga_P_PopulateIOConfiguration(pRaagaConnectStage, &pStageConfig->sIOConfig, &tocIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_FillStageConfig: Error in Configuring the Stage IO for Stage(%d) %s",pRaagaConnectStage->eAlgorithm, psAlgoInfo->pName));
			BDBG_ASSERT(0);
		}

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
	psAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaPrimaryStage->eAlgorithm);
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

	BDSP_Raaga_P_Analyse_CIT(pRaagaTask, false);

    BDBG_LEAVE(BDSP_Raaga_P_GenCit);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_CleanupDescriptors(
	BDSP_RaagaStage 	*pRaagaStage,
	BDSP_AF_P_sIOConfig *psIOConfig
)
{
    BERR_Code errCode = BERR_SUCCESS;
	unsigned index=0,i=0,j=0, interStagePortIndex=0;
	BDSP_P_InterStagePortInfo *psInterStagePortInfo;
	dramaddr_t decriptorArray[BDSP_AF_P_MAX_CHANNELS];
	BDSP_AF_P_sIoPort *psIoPort;

	BDBG_ENTER(BDSP_Raaga_P_CleanupDescriptors);
	for(index=0; index<psIOConfig->ui32NumInputs; index++)
	{
		psIoPort = (BDSP_AF_P_sIoPort *)&psIOConfig->sInputPort[index];
		switch(psIoPort->ePortType)
		{
			case BDSP_AF_P_PortType_eRAVE:
				for(j=0;j<psIoPort->ui32numPortBuffer;j++)
				{
					for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
					{
						decriptorArray[i] = psIoPort->sIoBuffer[j].sCircularBuffer[i];
						psIoPort->sIoBuffer[j].sCircularBuffer[i]=0;
					}
					errCode = BDSP_Raaga_P_ReleaseDescriptor(
						(void *)pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						&decriptorArray[0],
						psIoPort->sIoBuffer[j].ui32NumBuffer);
					if(errCode != BERR_SUCCESS)
					{
						BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
						goto end;
					}
				}
				break;
			case BDSP_AF_P_PortType_eInterStage:
				BDBG_MSG(("Descriptor is owned by the source stage as it is connected at input"));
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
				for(j=0;j<psIoPort->ui32numPortBuffer;j++)
				{
					for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
					{
						decriptorArray[i] = psIoPort->sIoBuffer[j].sCircularBuffer[i];
						psIoPort->sIoBuffer[j].sCircularBuffer[i]=0;
					}
					errCode = BDSP_Raaga_P_ReleaseDescriptor(
						(void *)pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						&decriptorArray[0],
						psIoPort->sIoBuffer[j].ui32NumBuffer);
					if(errCode != BERR_SUCCESS)
					{
						BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
						goto end;
					}
				}
				break;
			case BDSP_AF_P_PortType_eInterStage:
				BDSP_Raaga_P_GetInterStagePortIndex(pRaagaStage,psIoPort->ePortDataType,&interStagePortIndex);
				psInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
				psInterStagePortInfo->tocIndex = BDSP_AF_P_TOC_INVALID;
				for(j=0;j<psIoPort->ui32numPortBuffer;j++)
				{
					for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
					{
						decriptorArray[i] = psIoPort->sIoBuffer[j].sCircularBuffer[i];
						psIoPort->sIoBuffer[j].sCircularBuffer[i]=0;
					}
					errCode = BDSP_Raaga_P_ReleaseDescriptor(
						(void *)pRaagaStage->pContext->pDevice,
						pRaagaStage->pRaagaTask->createSettings.dspIndex,
						&decriptorArray[0],
						psIoPort->sIoBuffer[j].ui32NumBuffer);
					if(errCode != BERR_SUCCESS)
					{
						BDBG_ERR(("BDSP_Raaga_P_CleanupDescriptors: Error in Cleanup of Descriptor"));
						goto end;
					}
					psInterStagePortInfo->bufferDescriptorAddr[j] = 0;
				}
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
	pTaskConfig = (BDSP_AF_P_sTASK_CONFIG *)pRaagaTask->taskMemInfo.sCITMemory.Buffer.pAddr;
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
