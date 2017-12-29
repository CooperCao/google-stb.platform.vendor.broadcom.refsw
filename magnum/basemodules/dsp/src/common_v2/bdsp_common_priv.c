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

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_common_priv);

BERR_Code BDSP_P_PopulateSystemSchedulingDeatils(
	BDSP_P_SystemSchedulingInfo *pSystemSchedulingInfo
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned index = 0, level = 0, threshold = 0, numpreemptionLevels = 0;
	BDBG_ENTER(BDSP_P_PopulateSystemSchedulingDeatils);

	BKNI_Memset((void *)pSystemSchedulingInfo, 0, sizeof(BDSP_P_SystemSchedulingInfo));
	for(index=0; index<BDSP_P_TaskType_eLast; index++)
	{
		pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingLevel = 0xFF;
		pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingThreshold = 0xFF;
	}

	/* Depending on BOX mode/device settings, derive the supported task types*/
	pSystemSchedulingInfo->sTaskSchedulingInfo[BDSP_P_TaskType_eInterruptBased].supported = false;
	pSystemSchedulingInfo->sTaskSchedulingInfo[BDSP_P_TaskType_eRealtime].supported       = true;
	pSystemSchedulingInfo->sTaskSchedulingInfo[BDSP_P_TaskType_eAssuredRate].supported    = false;
	pSystemSchedulingInfo->sTaskSchedulingInfo[BDSP_P_TaskType_eOnDemand].supported       = false;
	pSystemSchedulingInfo->sTaskSchedulingInfo[BDSP_P_TaskType_eAFAP].supported           = true;

	for(index=0; index<BDSP_P_TaskType_eLast; index++)
	{
		if(pSystemSchedulingInfo->sTaskSchedulingInfo[index].supported)
		{
			pSystemSchedulingInfo->numSchedulingLevels++;
			pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingLevel = level;
			level++;
		}
	}

	/* TO DO: Need to expand this feature */
	for(index=0; index<BDSP_P_TaskType_eLast; index++)
	{
		if(pSystemSchedulingInfo->sTaskSchedulingInfo[index].supported)
		{
			/* We support only RT and AFAP modes and have kept them at same levels (RT won't preempt NRT) */
			pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingThreshold = threshold;
			numpreemptionLevels = 1;
		}
	}

	pSystemSchedulingInfo->numPreemptionLevels = numpreemptionLevels;
	BDBG_MSG(("Num Scheduling levels = %d, Num Preemption Levels = %d", pSystemSchedulingInfo->numSchedulingLevels, pSystemSchedulingInfo->numPreemptionLevels));
	for(index=0; index<BDSP_P_TaskType_eLast; index++)
	{
		BDBG_MSG(("TASK TYPE[%s]:\t%s,\tLEVEL: 0x%x,\tTHRESHOLD: 0x%x",TaskType[index], DisableEnable[pSystemSchedulingInfo->sTaskSchedulingInfo[index].supported],
			pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingLevel, pSystemSchedulingInfo->sTaskSchedulingInfo[index].schedulingThreshold));
	}
	BDBG_LEAVE(BDSP_P_PopulateSystemSchedulingDeatils);
	return errCode;
}
void BDSP_P_ValidateCodeDownloadSettings(
    unsigned *pmaxAlgorithms
)
{
    unsigned i=0;

    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        if (pmaxAlgorithms[i] > BDSP_MAX_DOWNLOAD_BUFFERS )
        {
            BDBG_ERR(("Maximum number of Algorithms for Algorithm type(%d) is %d which is exceeding downloading limits(%d)",
                i,pmaxAlgorithms[i],BDSP_MAX_DOWNLOAD_BUFFERS));
            BDBG_ASSERT(0);
        }
    }
}

unsigned BDSP_P_GetFreeTaskId(
    BDSP_P_TaskInfo *pTaskInfo
)
{
    unsigned taskId =0;
    for (taskId = 0 ; taskId < BDSP_MAX_FW_TASK_PER_DSP; taskId++)
    {
        if (pTaskInfo->taskId[taskId] == false)
        {
            pTaskInfo->taskId[taskId] = true;
            return taskId;
        }
    }
    return BDSP_P_INVALID_TASK_ID;
}

void BDSP_P_ReleaseTaskId(
    BDSP_P_TaskInfo *pTaskInfo,
    unsigned        *taskId
)
{
    pTaskInfo->taskId[*taskId] = false;
    *taskId = BDSP_P_INVALID_TASK_ID;
}

void BDSP_P_GetDistinctOpTypeAndNumChans(
    BDSP_DataType dataType, /* [in] */
    unsigned *numChans, /* [out] */
    BDSP_AF_P_DistinctOpType *distinctOp /* [out] */
)
{
    switch(dataType)
    {
        case BDSP_DataType_ePcmMono:
            *distinctOp = BDSP_AF_P_DistinctOpType_eMono_PCM;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937x16:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressedHBR;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937x4:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed4x;
            *numChans = 1;
            break;
        case BDSP_DataType_eCompressedRaw:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcmRf:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed4x;
            *numChans = 1;
            break;
        case BDSP_DataType_eRave:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcm5_1:
            *distinctOp = BDSP_AF_P_DistinctOpType_e5_1_PCM;
            *numChans = 6;
            break;
        case BDSP_DataType_eDolbyTranscodeData:
            *distinctOp = BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut;
            *numChans = 6;
#if (BDSP_MS12_SUPPORT == 65)
            *numChans = 8;
#endif
            break;
        case BDSP_DataType_ePcm7_1:
            *distinctOp = BDSP_AF_P_DistinctOpType_e7_1_PCM;
            *numChans = 8;
            break;
        case BDSP_DataType_eRdbCdb:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_eRdbItb:
            *distinctOp = BDSP_AF_P_DistinctOpType_eItb;
            *numChans = 1;
            break;
        case BDSP_DataType_eRdbAnc:
            *distinctOp = BDSP_AF_P_DistinctOpType_eAncillaryData;
            *numChans = 1;
            break;
        case BDSP_DataType_eRDBPool:
            *distinctOp = BDSP_AF_P_DistinctOpType_eDescriptorQueue;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcmStereo:
        default:
            *distinctOp = BDSP_AF_P_DistinctOpType_eStereo_PCM;
            *numChans = 2;
            break;
    }

    return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetFreeInputPortIndex

Type        :   BDSP Internal

Input       :   psStageInput  -   Pointer of the StageInput details for which we need to return the Input port index
                   index              -   Index returned back.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the first free input index for the stage and return the same to the caller.
***********************************************************************/
void BDSP_P_GetFreeInputPortIndex(
    BDSP_P_ConnectionDetails *psStageInput,
    unsigned *index
)
{
    unsigned i;
    BDSP_P_ConnectionDetails *psStageInputDetails = psStageInput;

    for (i = 0; i < BDSP_AF_P_MAX_IP_FORKS; i++)
    {
        if (psStageInputDetails->eValid != BDSP_AF_P_eValid)
        {
            break;
        }
        psStageInputDetails++;
    }
    *index = i;
}


/***********************************************************************
Name        :   BDSP_P_GetFreeOutputPortIndex

Type        :   BDSP Internal

Input       :   psStageOutput   -   Pointer of the StageOutput details for which we need to return the Output port index
                   index                  -   Index returned back.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the first free output index for the stage and return the same to the caller.
***********************************************************************/
void BDSP_P_GetFreeOutputPortIndex(
    BDSP_P_ConnectionDetails *psStageOutput,
    unsigned *index
)
{
    unsigned i;
    BDSP_P_ConnectionDetails *psStageDstDetails = psStageOutput;

    for (i = 0; i < BDSP_AF_P_MAX_OP_FORKS; i++)
    {
        if (psStageDstDetails->eValid != BDSP_AF_P_eValid)
        {
            break;
        }
        psStageDstDetails++;
    }
    *index = i;
}

/***********************************************************************
Name        :   BDSP_DSP_P_InterframeRunLengthDecode

Type        :   PI Interface

Input       :   pSrc                    -   Pointer to Source Array which is encoded using Run Length Algorithm
                pDst                    -   Pointer to Memory Location in DRAM where the uncompressed array needs to be copied.
                ui32IfBuffEncodedSize   -   Size of encoded Inter-Frame array
                ui32AllocatedBufferSize -   Size of Memory Allocated in Dram.

Return      :   BERR_Code

Functionality   :   Following are the operations performed.
        1)  Uncompress the Inter-Frame Array using the Run Length Decode Algorithm
        2)  Confirm that enough size has been allocated in DRAM by BDSP.
        3)  Copy the original uncompressed opcodes into DRAM
***********************************************************************/
BERR_Code BDSP_P_InterframeRunLengthDecode(
	void *pDst,
	void *pSrc,
	uint32_t ui32EncodedSize,
	uint32_t ui32AllocatedBufferSize
)
{
    BERR_Code   rc = BERR_SUCCESS;
    uint32_t ui32IfBuffActualSize = 0;
    uint32_t i =0, j =0;
    uint32_t * pTempSrc = (uint32_t *) pSrc;
    uint32_t * pTempDest = (uint32_t *) pDst;
    uint32_t last_index;

    BDBG_ENTER(BDSP_P_InterframeRunLengthDecode);
    BDBG_ASSERT(( ui32EncodedSize % 8 == 0)); /* Any array that has been encoded using the run length encode has to be in multiples of 8. Otherwise something has gone wrong */

    /*for (i = 0; i < (ui32EncodedSize >> 3); i++)
    {
        ui32IfBuffActualSize += pTempSrc[2*i + 1];
    }*/

    ui32IfBuffActualSize += pTempSrc[2];

    /* Size has already been allocated in DRAM. Confirm whether it is big enough. */
    if (ui32AllocatedBufferSize < ui32IfBuffActualSize)
    {
        BDBG_ERR(("Allocated memory (%d) for interframe buffer is less than required (%d)",
                    ui32AllocatedBufferSize,
                    ui32IfBuffActualSize
                 ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    last_index = pTempSrc[1] >> 2;

    /* Then start copying in chunks element by element */
    for (i = 4; i < last_index; i+=2)
    {
        for (j = 0; j < pTempSrc[i]; j++)
        {
            *pTempDest++ = pTempSrc[i+1];
        }
    }
    BDBG_LEAVE(BDSP_P_InterframeRunLengthDecode);
    return rc;
}

void BDSP_P_GetInterStagePortIndex(
	BDSP_P_StageConnectionInfo *pStageConnectionInfo,
	BDSP_AF_P_DistinctOpType    eDistinctOpType,
	unsigned                   *pIndex
)
{
	int32_t index = 0;
	unsigned firstFreeIndex = BDSP_AF_P_MAX_OP_FORKS;
	BDBG_ENTER(BDSP_P_GetInterStagePortIndex);
	for(index=(BDSP_AF_P_MAX_OP_FORKS-1); index >= 0; index--)
	{
		if(pStageConnectionInfo->sInterStagePortInfo[index].ePortDataType == eDistinctOpType)
		{
			*pIndex = index;
			goto end;
		}
		if(pStageConnectionInfo->sInterStagePortInfo[index].ePortDataType == BDSP_AF_P_DistinctOpType_eMax)
		{
			firstFreeIndex = index;
		}
	}
	if(firstFreeIndex == BDSP_AF_P_MAX_OP_FORKS)
	{
		BDBG_ERR(("BDSP_P_GetInterStagePortIndex: Not able to derive the index"));
		BDBG_ASSERT(0);
	}
	else
	{
		*pIndex = firstFreeIndex;
	}
end:
	BDBG_MSG(("InterStage Port Index = %d",*pIndex));
	BDBG_LEAVE(BDSP_P_GetInterStagePortIndex);
}

void BDSP_P_GetInterTaskPortIndex(
	BDSP_Stage                 *pStage,
	BDSP_P_StageConnectionInfo *pStageConnectionInfo,
	BDSP_AF_P_DistinctOpType    eDistinctOpType,
	unsigned                   *pIndex
)
{
	int32_t index = 0;
    BDSP_P_InterTaskBuffer *pIntertaskBuffer;
	BDBG_ENTER(BDSP_P_GetInterTaskPortIndex);
	for(index = 0; index < BDSP_AF_P_MAX_OP_FORKS; index++)
	{
	    if(pStageConnectionInfo->sStageOutput[index].eConnectionType == BDSP_ConnectionType_eInterTaskBuffer)
        {
            pIntertaskBuffer = (BDSP_P_InterTaskBuffer *)pStageConnectionInfo->sStageOutput[index].connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
            if((pIntertaskBuffer->distinctOp == eDistinctOpType)&&(pIntertaskBuffer->srcHandle == pStage))
            {
                *pIndex = index;
                goto end;
            }
        }
	}
    if(index >= BDSP_AF_P_MAX_OP_FORKS)
    {
		BDBG_ERR(("BDSP_P_GetInterTaskPortIndex: Not able to derive the index"));
		BDBG_ASSERT(0);
    }
end:
	BDBG_MSG(("InterTask Port Index = %d",*pIndex));
	BDBG_LEAVE(BDSP_P_GetInterTaskPortIndex);
}

#if !B_REFSW_MINIMAL
BERR_Code BDSP_P_GetDefaultDatasyncSettings(
        void *pDeviceHandle,
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
)
{
    BDBG_ENTER(BDSP_P_GetDefaultDatasyncSettings);
    if(sizeof(BDSP_AudioTaskDatasyncSettings) != settingsBufferSize)
    {
        BDBG_ERR(("BDSP_P_GetDefaultDatasyncSettings: settingsBufferSize (%lu) is not equal to Config size (%lu) of DataSync ",
            (unsigned long)settingsBufferSize,(unsigned long)sizeof(BDSP_AudioTaskDatasyncSettings)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy((void *)(volatile void *)pSettingsBuffer,(void *)&BDSP_sDefaultFrameSyncSettings,settingsBufferSize);

    BSTD_UNUSED(pDeviceHandle);
    BDBG_LEAVE(BDSP_P_GetDefaultDatasyncSettings);
    return BERR_SUCCESS;
}
#endif /*!B_REFSW_MINIMAL*/

BERR_Code BDSP_P_GetDefaultTsmSettings(
        void *pDeviceHandle,
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
)
{
    BDBG_ENTER(BDSP_P_GetDefaultTsmSettings);
    if(sizeof(BDSP_AudioTaskTsmSettings) != settingsBufferSize)
    {
        BDBG_ERR(("BDSP_P_GetDefaultTsmSettings: settingsBufferSize (%lu) is not equal to Config size (%lu) of DataSync ",
            (unsigned long)settingsBufferSize,(unsigned long)sizeof(BDSP_AudioTaskTsmSettings)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy((void *)(volatile void *)pSettingsBuffer,(void *)&BDSP_sDefaultTSMSettings,settingsBufferSize);

    BSTD_UNUSED(pDeviceHandle);
    BDBG_LEAVE(BDSP_P_GetDefaultTsmSettings);
    return BERR_SUCCESS;
}

BERR_Code BDSP_P_CopyStartTaskSettings(
    BDSP_ContextType        contextType,
	BDSP_TaskStartSettings *pBDSPStartSettings,
	BDSP_TaskStartSettings *pAPPStartSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_P_CopyStartTaskSettings);

    BKNI_Memcpy((void *)pBDSPStartSettings, (void *)pAPPStartSettings, sizeof(BDSP_TaskStartSettings));

	/*Pointers provided by PI are allocated by them respectively and should not hold them*/
	pBDSPStartSettings->pSampleRateMap      = NULL;
	pBDSPStartSettings->psVDecoderIPBuffCfg = NULL;
	pBDSPStartSettings->psVEncoderIPConfig  = NULL;

	if(BDSP_ContextType_eVideo == contextType )
	{
		if(NULL == pAPPStartSettings->psVDecoderIPBuffCfg)
		{
			BDBG_ERR(("BDSP_P_CopyStartTaskSettings: Input Config structure not provided for Video Decode Task by PI"));
			errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			return errCode;
		}
		pBDSPStartSettings->psVDecoderIPBuffCfg = (BDSP_sVDecoderIPBuffCfg *)BKNI_Malloc(sizeof(BDSP_sVDecoderIPBuffCfg));
		BKNI_Memcpy((void*)pBDSPStartSettings->psVDecoderIPBuffCfg,
			(void *)pAPPStartSettings->psVDecoderIPBuffCfg,
			sizeof(BDSP_sVDecoderIPBuffCfg));
		goto end;
	}

	if(BDSP_ContextType_eVideoEncode == contextType )
	{
		if(NULL == pAPPStartSettings->psVEncoderIPConfig)
		{
			BDBG_ERR(("BDSP_P_CopyStartTaskSettings: Input Config structure not provided for Video Encode Task by PI"));
			errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			return errCode;
		}
		pBDSPStartSettings->psVEncoderIPConfig = (BDSP_sVEncoderIPConfig *)BKNI_Malloc(sizeof(BDSP_sVEncoderIPConfig));
		BKNI_Memcpy((void*)pBDSPStartSettings->psVEncoderIPConfig,
			(void *)pAPPStartSettings->psVEncoderIPConfig,
			sizeof(BDSP_sVEncoderIPConfig));
		goto end;
	}
	if(BDSP_ContextType_eAudio == contextType )
	{
		/* If APE doesnt provide the Sample Rate Map table then BDSP will internally fill it. No error handling required*/
		if(NULL != pAPPStartSettings->pSampleRateMap)
		{
			pBDSPStartSettings->pSampleRateMap = (BDSP_AF_P_sOpSamplingFreq *)BKNI_Malloc(sizeof(BDSP_AF_P_sOpSamplingFreq));
			BKNI_Memcpy((void*)pBDSPStartSettings->pSampleRateMap,
				(void *)pAPPStartSettings->pSampleRateMap,
				sizeof(BDSP_AF_P_sOpSamplingFreq));
		}
	}

end:
	BDBG_LEAVE(BDSP_P_CopyStartTaskSettings);
	return errCode;
}

void BDSP_P_DeleteStartTaskSettings(
	BDSP_TaskStartSettings *pStartSettings
)
{
	BDBG_ENTER(BDSP_P_DeleteStartTaskSettings);

	/* Release the Memory used to store the input Configuration*/
	if(pStartSettings->pSampleRateMap)
		BKNI_Free(pStartSettings->pSampleRateMap);
	if(pStartSettings->psVDecoderIPBuffCfg)
		BKNI_Free(pStartSettings->psVDecoderIPBuffCfg);
	if(pStartSettings->psVEncoderIPConfig)
		BKNI_Free(pStartSettings->psVEncoderIPConfig);

	BKNI_Memset((void *)pStartSettings, 0, sizeof(BDSP_TaskStartSettings));
	BDBG_LEAVE(BDSP_P_DeleteStartTaskSettings);
}

BERR_Code BDSP_P_PopulateSchedulingInfo(
    BDSP_TaskStartSettings  	*pTaskStartSettings,
	BDSP_ContextType         	 contextType,
	BDSP_P_SystemSchedulingInfo *pSystemSchedulingLevel,
	BDSP_P_StartTaskCommand 	*psCommand
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_P_PopulateSchedulingInfo);

	switch(pTaskStartSettings->schedulingMode)
	{
		case BDSP_TaskSchedulingMode_eStandalone:
		case BDSP_TaskSchedulingMode_eMaster:
			psCommand->eSchedulingMode = BDSP_P_SchedulingMode_eMaster;
			break;
		case BDSP_TaskSchedulingMode_eSlave:
			psCommand->eSchedulingMode = BDSP_P_SchedulingMode_eSlave;
			break;
		default:
			BDBG_ERR(("BDSP_P_PopulateSchedulingInfo: Invalid Scheduling mode being Set"));
			errCode = BERR_INVALID_PARAMETER;
			goto end;
	}
	switch(pTaskStartSettings->realtimeMode)
	{
		case BDSP_TaskRealtimeMode_eRealTime:
			if((contextType == BDSP_ContextType_eVideoEncode)||
                (pTaskStartSettings->audioTaskDelayMode == BDSP_AudioTaskDelayMode_WD_eLow))
			{
				psCommand->eTaskType = BDSP_P_TaskType_eInterruptBased;
				psCommand->ui32SchedulingLevel =
					pSystemSchedulingLevel->sTaskSchedulingInfo[BDSP_P_TaskType_eInterruptBased].schedulingLevel;
			}
			else
			{
				psCommand->eTaskType = BDSP_P_TaskType_eRealtime;
				psCommand->ui32SchedulingLevel =
					pSystemSchedulingLevel->sTaskSchedulingInfo[BDSP_P_TaskType_eRealtime].schedulingLevel;
			}
			break;
		case BDSP_TaskRealtimeMode_eNonRealTime:
			psCommand->eTaskType = BDSP_P_TaskType_eAFAP;
			psCommand->ui32SchedulingLevel =
				pSystemSchedulingLevel->sTaskSchedulingInfo[BDSP_P_TaskType_eAFAP].schedulingLevel;
			break;
		case BDSP_TaskRealtimeMode_eOnDemand:
			psCommand->eTaskType = BDSP_P_TaskType_eOnDemand;
			psCommand->ui32SchedulingLevel =
				pSystemSchedulingLevel->sTaskSchedulingInfo[BDSP_P_TaskType_eOnDemand].schedulingLevel;
			break;
		default:
			BDBG_ERR(("BDSP_P_PopulateSchedulingInfo: Invalid Tasktype being Set"));
			errCode = BERR_INVALID_PARAMETER;
			goto end;
	}

end:
	BDBG_LEAVE(BDSP_P_PopulateSchedulingInfo);
	return errCode;
}

void BDSP_P_InitBufferDescriptor(
    BDSP_P_BufferPointer    *pBufferPointer,
    BDSP_MMA_Memory         *pMemory,
    unsigned                 size
)
{
    BDBG_ENTER(BDSP_P_InitBufferDescriptor);
    pBufferPointer->BaseOffset = pMemory->offset;
    pBufferPointer->ReadOffset = pMemory->offset;
    pBufferPointer->WriteOffset= pMemory->offset;
    pBufferPointer->EndOffset  = pMemory->offset+size;
    BDBG_LEAVE(BDSP_P_InitBufferDescriptor);
}

void BDSP_P_InitDramBuffer(
    BDSP_P_FwBuffer   *pDescriptorMemory,
    dramaddr_t         bufferDescriptorAddr,
    dramaddr_t         BaseOffset
)
{
    BDSP_AF_P_sCIRCULAR_BUFFER *pCircularBuffer;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_P_InitDramBuffer);
    pCircularBuffer = (BDSP_AF_P_sCIRCULAR_BUFFER *)BDSP_MMA_P_GetVirtualAddressfromOffset(
                            pDescriptorMemory,
                            bufferDescriptorAddr);
    pCircularBuffer->ReadAddr = BaseOffset;
    pCircularBuffer->WriteAddr= BaseOffset;
    Memory       = pDescriptorMemory->Buffer;

    Memory.pAddr = (void *)pCircularBuffer;
    Memory.offset= bufferDescriptorAddr;
    BDSP_MMA_P_FlushCache(Memory, sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));

    BDBG_LEAVE(BDSP_P_InitDramBuffer);
}
