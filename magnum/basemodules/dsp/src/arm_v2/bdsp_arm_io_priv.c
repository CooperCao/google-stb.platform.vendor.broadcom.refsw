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

BDBG_MODULE(bdsp_arm_io_priv);

/***********************************************************************
Name        :   BDSP_Arm_P_InterTaskBuffer_Create

Type        :   PI Interface

Input       :   pContextHandle  -   Context handle provided by the PI.
				dataType            -   Type of data which will be present in the Intertask buffer.
				pInterTaskBufferHandle  -   Handle of the Intertask buffer returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Allocate and initilaise the memory for the Intertask buffer descriptor structure.
		2)  Allocate the memory for I/O buffer depending on the number of channels.
		3)  Allocate the memory for the I/O Generic buffer.
		4)  Allocate the memory for the I/O and I/O Generic descriptors.
		5)  Populate the I/O and I/O Generic descriptors with the data.
		6)  Intialise the function pointers for destroy and flush which will be used by the PI for further processing.
		7)  Fill the Intertask buffer handle for further use by PI.
***********************************************************************/
BERR_Code BDSP_Arm_P_InterTaskBuffer_Create(
	void 						*pContextHandle,
	BDSP_DataType 				 dataType,
	BDSP_BufferType 			 bufferType,
	BDSP_InterTaskBufferHandle 	*pInterTaskBufferHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
	BDSP_AF_P_DistinctOpType distinctOp;
	BDSP_ArmContext *pArmContext;
	unsigned channels=0, numtoc=0, numMetaData=0, numObjectData=0;
    unsigned memReqd=0, i= 0;
    BDSP_MMA_Memory Memory;
	BDBG_ENTER(BDSP_Arm_P_InterTaskBuffer_Create);
	pArmContext = (BDSP_ArmContext *)pContextHandle;
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

	if(bufferType != BDSP_BufferType_eDRAM)
	{
		BDBG_ERR(("Only DRAM(0) buffer type of InterTask buffer supported in Arm!!!. Buffer type provided = %d", dataType));
		errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto end;
	}

	pInterTaskBuffer = BKNI_Malloc(sizeof(BDSP_P_InterTaskBuffer));
	if ( NULL == pInterTaskBuffer )
	{
		BDBG_ERR(("Unable to Allocate memory for InterTaskBuffer Handle !!!"));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto end;
	}
	BKNI_Memset(pInterTaskBuffer, 0, sizeof(*pInterTaskBuffer));
	BDSP_P_InitInterTaskBuffer(&pInterTaskBuffer->interTaskBuffer, pInterTaskBuffer);

	/* Initialize the inter task buffer apis */
	pInterTaskBuffer->interTaskBuffer.destroy = BDSP_Arm_P_InterTaskBuffer_Destroy;
	pInterTaskBuffer->interTaskBuffer.flush   = BDSP_Arm_P_InterTaskBuffer_Flush;

    numtoc        = BDSP_AF_P_MAX_TOC;
    numMetaData   = BDSP_AF_P_MAX_METADATA;
    numObjectData = BDSP_AF_P_MAX_OBJECTDATA;
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &distinctOp);
	pInterTaskBuffer->numChannels= channels;
	pInterTaskBuffer->distinctOp = distinctOp;
	pInterTaskBuffer->inUse 	 = false;
	pInterTaskBuffer->dataType   = dataType;
	pInterTaskBuffer->srcHandle  = NULL;
	pInterTaskBuffer->dstHandle  = NULL;
	pInterTaskBuffer->srcIndex   = -1;
	pInterTaskBuffer->dstIndex   = -1;
	pInterTaskBuffer->pContext   = pContextHandle;
    pInterTaskBuffer->numTocData   = numtoc;
    pInterTaskBuffer->numMetaData  = numMetaData;
    pInterTaskBuffer->numObjectData= numObjectData;

    BDSP_P_CalculateInterTaskMemory(&memReqd, channels);
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pArmContext->pDevice->memHandle,
                            memReqd,
                            &Memory,
                            BDSP_MMA_Alignment_4KByte);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Allocate memory for inter task io Buffer !"));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto error;
    }

    pInterTaskBuffer->descriptorAllocated = false;
    pInterTaskBuffer->MemoryPool.Memory   = Memory;
    pInterTaskBuffer->MemoryPool.ui32Size = memReqd;
    pInterTaskBuffer->MemoryPool.ui32UsedSize = 0;
    for(i=0; i<channels; i++)
    {
        errCode = BDSP_P_RequestMemory(&pInterTaskBuffer->MemoryPool, BDSP_AF_P_INTERTASK_BUFFER_PER_CHANNEL, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to allocate memory for PCM data"));
            goto end;
        }
        pInterTaskBuffer->PcmDetails[i].BuffersDetails.Memory  = Memory;
        pInterTaskBuffer->PcmDetails[i].BuffersDetails.ui32Size= BDSP_AF_P_INTERTASK_BUFFER_PER_CHANNEL;
    }
    for(i=0; i<numtoc; i++)
    {
        errCode = BDSP_P_RequestMemory(&pInterTaskBuffer->MemoryPool, BDSP_AF_P_TOC_BUFFER_SIZE, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to allocate memory for TOC data"));
            goto end;
        }
        pInterTaskBuffer->TocDetails[i].BuffersDetails.Memory  = Memory;
        pInterTaskBuffer->TocDetails[i].BuffersDetails.ui32Size= BDSP_AF_P_TOC_BUFFER_SIZE;
    }
    for(i=0; i<numMetaData; i++)
    {
        errCode = BDSP_P_RequestMemory(&pInterTaskBuffer->MemoryPool, BDSP_AF_P_METADATA_BUFFER_SIZE, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to allocate memory for Meta data"));
            goto end;
        }
        pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.Memory   = Memory;
        pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.ui32Size = BDSP_AF_P_METADATA_BUFFER_SIZE;
    }
    for(i=0; i<numObjectData; i++)
    {
        errCode = BDSP_P_RequestMemory(&pInterTaskBuffer->MemoryPool, BDSP_AF_P_OBJECTDATA_BUFFER_SIZE, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to allocate memory for Object data"));
            goto end;
        }
        pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.Memory   = Memory;
        pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.ui32Size = BDSP_AF_P_OBJECTDATA_BUFFER_SIZE;
    }

    if(BDSP_BufferType_eRDB == bufferType)
    {
#if 0
        unsigned numFifos = 0;
        uint32_t FifoId   = 0;
        pInterTaskBuffer->ebufferType = BDSP_AF_P_BufferType_eRDB;
        numFifos = channels+numtoc+numMetaData+numObjectData;
        errCode = BDSP_Arm_P_AssignFreeFIFO(pArmContext->pDevice, 0, &FifoId, numFifos);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to allocate RDB FIFO for Intertask buffer"));
            goto error_rdb;
        }
        for(i=0;i<channels;i++)
        {
            pInterTaskBuffer->PcmDetails[i].BuffersDetails.ui32FifoId = FifoId+i;
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->PcmDetails[i].BuffersDetails,
                    pArmContext->pDevice->regHandle,
                    pArmContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->PcmDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Create Queue for PCM buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->PcmDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for PCM buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numtoc;i++)
        {
            pInterTaskBuffer->TocDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->TocDetails[i].BuffersDetails,
                    pArmContext->pDevice->regHandle,
                    pArmContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->TocDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Create Queue for TOC buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->TocDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for TOC buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numMetaData;i++)
        {
            pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+numtoc+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->MetaDataDetails[i].BuffersDetails,
                    pArmContext->pDevice->regHandle,
                    pArmContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Create Queue for MetaData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for MetaData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numObjectData;i++)
        {
            pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+numtoc+numMetaData+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails,
                    pArmContext->pDevice->regHandle,
                    pArmContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Create Queue for ObjectData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for ObjectData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
#endif
    }
    else
    {
        pInterTaskBuffer->ebufferType = BDSP_AF_P_BufferType_eDRAM;
        for(i=0;i<channels;i++)
        {
            BDSP_P_InitBufferDescriptor(&pInterTaskBuffer->PcmDetails[i].BufferPointer,
                        &pInterTaskBuffer->PcmDetails[i].BuffersDetails.Memory,
                        pInterTaskBuffer->PcmDetails[i].BuffersDetails.ui32Size);
        }
        for(i=0;i<numtoc;i++)
        {
            BDSP_P_InitBufferDescriptor(&pInterTaskBuffer->TocDetails[i].BufferPointer,
                        &pInterTaskBuffer->TocDetails[i].BuffersDetails.Memory,
                        pInterTaskBuffer->TocDetails[i].BuffersDetails.ui32Size);
        }
        for(i=0;i<numMetaData;i++)
        {
            BDSP_P_InitBufferDescriptor(&pInterTaskBuffer->MetaDataDetails[i].BufferPointer,
                        &pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.Memory,
                        pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.ui32Size);
        }
        for(i=0;i<numObjectData;i++)
        {
            BDSP_P_InitBufferDescriptor(&pInterTaskBuffer->ObjectDataDetails[i].BufferPointer,
                        &pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.Memory,
                        pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.ui32Size);
        }
    }

	*pInterTaskBufferHandle = &pInterTaskBuffer->interTaskBuffer;
	BDBG_OBJECT_SET(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
    goto end;

#if 0
error_rdb:
    BDSP_MMA_P_FreeMemory(&Memory);
#endif
error:
    BKNI_Free(pInterTaskBuffer);
end:
	BDBG_LEAVE(BDSP_Arm_P_InterTaskBuffer_Create);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_InterTaskBuffer_Destroy

Type        :   PI Interface

Input       :   pInterTaskBufferHandle  -   Handle of the Intertask buffer to be destroyed by the PI.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Validate if the Intertask buffer is not connected to any stage at its input or ouput before destroying.
		2)  Retrieve the I/O buffer address from I/O descriptor and free the memory.
		3)  Retrieve the I/O Generic buffer address from I/O Generic descriptor and free the memory.
		4)  Free the memory of the I/O and I/O Generic descriptors.
		5)  Free the memory allocated to Intertask buffer descriptor structure.
***********************************************************************/
void BDSP_Arm_P_InterTaskBuffer_Destroy(
	void *pInterTaskBufferHandle
)
{
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
    BDSP_ArmContext *pArmContext;
    unsigned numFifos = 0;
    uint32_t FifoId   = 0;

	BDBG_ENTER(BDSP_Arm_P_InterTaskBuffer_Destroy);
	pInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pInterTaskBufferHandle;
	BDBG_OBJECT_ASSERT(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
    pArmContext = (BDSP_ArmContext *)pInterTaskBuffer->pContext;

	if (pInterTaskBuffer->dstHandle || pInterTaskBuffer->srcHandle)
	{
		BDBG_ERR(("BDSP_Arm_P_InterTaskBuffer_Destroy: Cannot destroy inter task buffer when in use. Please disconnect any input/output stages"));
		BDBG_ERR(("Handles connected to intertask buf - dst 0x%p src 0x%p",(void *)pInterTaskBuffer->dstHandle,(void *)pInterTaskBuffer->srcHandle));
		BERR_TRACE(BERR_NOT_AVAILABLE);
		goto end;
	}
    if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
    {
        FifoId = pInterTaskBuffer->PcmDetails[0].BuffersDetails.ui32FifoId;
        numFifos = pInterTaskBuffer->numChannels+pInterTaskBuffer->numTocData+
                   pInterTaskBuffer->numMetaData+pInterTaskBuffer->numObjectData;
        BDSP_Arm_P_ReleaseFIFO(pArmContext->pDevice, 0, &FifoId, numFifos);
    }

    BDSP_MMA_P_FreeMemory(&pInterTaskBuffer->MemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BKNI_Free(pInterTaskBuffer);

end:
	BDBG_LEAVE(BDSP_Arm_P_InterTaskBuffer_Destroy);
}

/***********************************************************************
Name        :   BDSP_Arm_P_InterTaskBuffer_Flush

Type        :   PI Interface

Input       :   pInterTaskBufferHandle  -   Handle of the Intertask buffer to be flushed.

Return      :   None

Functionality   :   To flush any buffer, we need to reset the Read, Write and Wrap pointers.
		1)  For every I/O buffer (number of channels), Reset Read and Write to Start Address and Wrap to End Address.
		2)  For the only I/O Generic buffer, Reset Read and Write to Start Address and Wrap to End Address.
***********************************************************************/
void BDSP_Arm_P_InterTaskBuffer_Flush(
	void *pInterTaskBufferHandle
)
{
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
    BDSP_ArmContext      *pArmContext;
    BDSP_Arm             *pArm;
    unsigned i = 0;
	BDBG_ENTER(BDSP_Arm_P_InterTaskBuffer_Flush);

	pInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pInterTaskBufferHandle;
	BDBG_OBJECT_ASSERT(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
    pArmContext = (BDSP_ArmContext *)pInterTaskBuffer->pContext;
    pArm        = (BDSP_Arm *)pArmContext->pDevice;
    for(i=0;i<pInterTaskBuffer->numChannels;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
#if 0
            BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->PcmDetails[i].hQueueHandle);
#endif
        }
        else
        {
            pInterTaskBuffer->PcmDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->PcmDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pArm->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_Data][i],
                    pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset);
            }
        }
    }
    for(i=0;i<pInterTaskBuffer->numTocData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
#if 0
            BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->TocDetails[i].hQueueHandle);
#endif
        }
        else
        {
            pInterTaskBuffer->TocDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->TocDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pArm->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_TOC][i],
                    pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset);
             }
        }
    }
    for(i=0;i<pInterTaskBuffer->numMetaData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
#if 0
            BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle);
#endif
        }
        else
        {
            pInterTaskBuffer->MetaDataDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->MetaDataDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pArm->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_MetaData][i],
                    pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset);
            }
        }
    }
    for(i=0;i<pInterTaskBuffer->numObjectData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
#if 0
            BDSP_Arm_P_InitMsgQueue(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle);
#endif
        }
        else
        {
            pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pArm->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_ObjectData][i],
                    pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset);
            }
        }
    }

	BDBG_LEAVE(BDSP_Arm_P_InterTaskBuffer_Flush);
}

BERR_Code BDSP_Arm_P_AddRaveInput(
    void *pStageHandle,
    const BAVC_XptContextMap *pContextMap,
    unsigned *pInputIndex
)
{
    BERR_Code errCode=BERR_SUCCESS;
    BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
    unsigned ipIndex=0;
    BDSP_P_ConnectionDetails *psStageInput;

    BDBG_ENTER(BDSP_Arm_P_AddRaveInput);
    BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);

    BDBG_MSG(("BDSP_Arm_P_AddRaveInput:Connecting RAVE Input to Stage (%s)", Algorithm2Name[pArmStage->eAlgorithm]));
    BDSP_P_GetFreeInputPortIndex(&(pArmStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
    BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);
    psStageInput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageInput[ipIndex];

    psStageInput->eValid          = BDSP_AF_P_eValid;
    psStageInput->eConnectionType = BDSP_ConnectionType_eRaveBuffer;
    psStageInput->connectionHandle.rave.raveContextMap = *pContextMap;

    pArmStage->totalInputs+=1;
    pArmStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eRaveBuffer]+=1;
    *pInputIndex = ipIndex;

    BDBG_LEAVE(BDSP_Arm_P_AddRaveInput);
    return errCode;
}

BERR_Code BDSP_Arm_P_AddRaveOutput(
	void *pStageHandle,
	const BAVC_XptContextMap *pContextMap,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	unsigned opIndex=0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Arm_P_AddRaveOutput);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);

	BDBG_MSG(("BDSP_Arm_P_AddRaveOutput:Connecting RAVE Output to Stage (%s)", Algorithm2Name[pArmStage->eAlgorithm]));
	BDSP_P_GetFreeOutputPortIndex(&(pArmStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageOutput[opIndex];

	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eRaveBuffer;
	psStageOutput->connectionHandle.rave.raveContextMap = *pContextMap;

	pArmStage->totalOutputs+=1;
	pArmStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eRaveBuffer]+=1;
	*pOutputIndex = opIndex;

	BDBG_LEAVE(BDSP_Arm_P_AddRaveOutput);
	return errCode;
}

BERR_Code BDSP_Arm_P_AddFmmInput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_FmmBufferDescriptor *pDescriptor,
	unsigned *pInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	unsigned ipIndex=0, channels=0, index =0;
	BDSP_P_ConnectionDetails *psStageInput;
	BDSP_AF_P_DistinctOpType distinctOp;

	BDBG_ENTER(BDSP_Arm_P_AddFmmInput);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDSP_P_GetFreeInputPortIndex(&(pArmStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	psStageInput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageInput[ipIndex];

	BDBG_MSG(("BDSP_Arm_P_AddFmmInput:Connecting FMM Input to Stage (%s) with datatype = %s",
        Algorithm2Name[pArmStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &distinctOp);
	if(channels!=pDescriptor->numBuffers)
	{
		errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("BDSP_Arm_P_AddFmmInput::FMM Input not added!! Channels = %d and buffers in descriptor = %d",channels,pDescriptor->numBuffers));
		goto end;
	}

	psStageInput->eValid          = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eFmmBuffer;
	psStageInput->dataType        = dataType;
	psStageInput->connectionHandle.fmm.fmmDescriptor = *pDescriptor;

	for(index=0;index<BDSP_AF_P_MAX_CHANNEL_PAIR;index++)/*rate controller Initialize per pair of channels*/
	{
		psStageInput->connectionHandle.fmm.rateController[index].wrcnt = BDSP_AF_P_WRCNT_INVALID;
	}

	for(index=0; index<((pDescriptor->numBuffers+1)>>1); index++)/*rate controller per pair of channels*/
	{
		psStageInput->connectionHandle.fmm.rateController[index].wrcnt = pDescriptor->rateControllers[index].wrcnt;
	}

	pArmStage->totalInputs+=1;
	pArmStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eFmmBuffer]+=1;
	*pInputIndex = ipIndex;

	if ((pArmStage->running)&&(!pArmStage->pContext->contextWatchdogFlag))
	{
        errCode = BDSP_Arm_P_ReconfigCit(pArmStage,true,psStageInput,ipIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_AddFmmInput: Error in Reconfig of CIT"));
			goto end;
		}
	}
end:
	BDBG_LEAVE(BDSP_Arm_P_AddFmmInput);
	return errCode;
}

BERR_Code BDSP_Arm_P_AddFmmOutput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_FmmBufferDescriptor *pDescriptor,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	unsigned opIndex=0, channels =0, index = 0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Arm_P_AddFmmOutput);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDSP_P_GetFreeOutputPortIndex(&(pArmStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Arm_P_AddFmmOutput:Connecting FMM output to Stage (%s) with datatype = %s",
        Algorithm2Name[pArmStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pArmStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	if(channels!=pDescriptor->numBuffers)
	{
		errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("BDSP_Arm_P_AddFmmOutput::FMM Output not added!! Channels = %d and buffers in descriptor = %d",channels,pDescriptor->numBuffers));
		goto end;
	}

	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eFmmBuffer;
	psStageOutput->dataType        = dataType;
	psStageOutput->connectionHandle.fmm.fmmDescriptor = *pDescriptor;

	for(index=0;index<BDSP_AF_P_MAX_CHANNEL_PAIR;index++)/*rate controller Initialize per pair of channels*/
	{
		psStageOutput->connectionHandle.fmm.rateController[index].wrcnt = BDSP_AF_P_WRCNT_INVALID;
	}

	for(index=0; index<((pDescriptor->numBuffers+1)>>1); index++)/*rate controller per pair of channels*/
	{
		psStageOutput->connectionHandle.fmm.rateController[index].wrcnt = pDescriptor->rateControllers[index].wrcnt;
	}

	pArmStage->totalOutputs+=1;
	pArmStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eFmmBuffer]+=1;
	*pOutputIndex = opIndex;

end:
	BDBG_LEAVE(BDSP_Arm_P_AddFmmOutput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AddInterTaskBufferInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose input will be Intertask buffer.
				dataType            -   Type of data present in the Intertask buffer.
				pBufferHandle       -   Intertask buffer descriptor provided by the PI.
				pInputIndex     -   Index of the input which is associated for the Intertask buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free input index for the stage which can be used for this Intertask buffer.
		2)  Populate the Source details for the Stage with appropriate values using the Intertask descriptor provided by the PI.
		3)  Populate the Address of the I/O and I/O Generic buffers.
		4)  Increment the number of inputs from Intertask buffer for this stage and total inputs to stage.
		5)  Set the Intertask buffer's InUse variable if some stage is already connected to feed data into the Intertask buffer.
		6)  Fill the input index pointer for the PI for later use.
		7)  If the stage is already running/inside watchdog recovery then perform the CIT reconfiguration.
***********************************************************************/
BERR_Code BDSP_Arm_P_AddInterTaskBufferInput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_InterTaskBuffer *pBufferHandle,
	unsigned *pInputIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	BDSP_P_InterTaskBuffer *pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
	unsigned ipIndex;
	BDSP_P_ConnectionDetails *psStageInput;

	BDBG_ENTER(BDSP_Arm_P_AddInterTaskBufferInput);

	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_OBJECT_ASSERT(pArmInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BDBG_ASSERT(NULL != pInputIndex);
	BDBG_ASSERT(dataType == pArmInterTaskBuffer->dataType);

	BDSP_P_GetFreeInputPortIndex(&(pArmStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	psStageInput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageInput[ipIndex];

	BDBG_MSG(("BDSP_Arm_P_AddInterTaskBufferInput:Connecting Intertask Input to Stage %s with datatype=%s",
        Algorithm2Name[pArmStage->eAlgorithm], DataType[dataType]));

	psStageInput->eValid          = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eInterTaskBuffer;
	psStageInput->dataType        = dataType;
	psStageInput->connectionHandle.interTask.hInterTask = (BDSP_InterTaskBufferHandle)&pArmInterTaskBuffer->interTaskBuffer;;

	pArmStage->totalInputs+=1;
	pArmStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eInterTaskBuffer]+=1;

	pArmInterTaskBuffer->dstIndex  = ipIndex;
	pArmInterTaskBuffer->dstHandle = pStageHandle;
	*pInputIndex = ipIndex;

	if (pArmInterTaskBuffer->srcHandle)
	{
		pArmInterTaskBuffer->inUse = true;
	}

	if ((pArmStage->running)&&(!pArmStage->pContext->contextWatchdogFlag))
	{
		errCode = BDSP_Arm_P_ReconfigCit(pArmStage,true,psStageInput,ipIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Arm_P_AddInterTaskBufferInput: Error in Reconfig of CIT"));
			goto end;
		}
	}
end:
	BDBG_LEAVE(BDSP_Arm_P_AddInterTaskBufferInput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Arm_P_AddInterTaskBufferOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose output will be Intertask buffer.
				dataType            -   Type of data present in the Intertask buffer.
				pBufferHandle       -   Intertask buffer descriptor provided by the PI.
				pOutputIndex        -   Index of the output which is associated for the Intertask buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output index for the stage which can be used for this Intertask buffer.
		2)  Populate the Destination details for the Stage with appropriate values using the Intertask descriptor provided by the PI.
		3)  Populate the Address of the I/O and I/O Generic buffers.
		4)  Increment the number of outputs to Intertask buffer from this stage and total outputs from this stage.
		5)  Set the Intertask buffer's InUse variable if some stage is already connected to consume data from this  Intertask buffer.
		6)  Fill the input index pointer for the PI for later use.
***********************************************************************/
BERR_Code BDSP_Arm_P_AddInterTaskBufferOutput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_InterTaskBuffer *pBufferHandle,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	BDSP_P_InterTaskBuffer *pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
	unsigned opIndex;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Arm_P_AddInterTaskBufferOutput);

	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_OBJECT_ASSERT(pArmInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BDBG_ASSERT(NULL != pOutputIndex);
	BDBG_ASSERT(dataType == pArmInterTaskBuffer->dataType);

	BDSP_P_GetFreeOutputPortIndex(&((pArmStage->sStageConnectionInfo.sStageOutput[0])), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

	psStageOutput = (BDSP_P_ConnectionDetails *)&pArmStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Arm_P_AddInterTaskBufferOutput:Connecting Intertask Output to Stage %s with datatype=%s",
        Algorithm2Name[pArmStage->eAlgorithm], DataType[dataType]));

	psStageOutput->eValid		  = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType= BDSP_ConnectionType_eInterTaskBuffer;
	psStageOutput->dataType		  = dataType;
	psStageOutput->connectionHandle.interTask.hInterTask = (BDSP_InterTaskBufferHandle)&pArmInterTaskBuffer->interTaskBuffer;;

	pArmStage->totalOutputs+=1;
	pArmStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eInterTaskBuffer]+=1;
	pArmStage->sStageConnectionInfo.eStageOpDataType[opIndex]=pArmInterTaskBuffer->distinctOp;

	pArmInterTaskBuffer->srcIndex  = opIndex;
	pArmInterTaskBuffer->srcHandle = pStageHandle;
	*pOutputIndex = opIndex;

	if (pArmInterTaskBuffer->dstHandle)
	{
		pArmInterTaskBuffer->inUse = true;
	}

	BDBG_LEAVE(BDSP_Arm_P_AddInterTaskBufferOutput);
	return errCode;
}

BERR_Code BDSP_Arm_P_AddOutputStage(
	void *pSrcStageHandle,
	BDSP_DataType dataType,
	void *pDstStageHandle,
	unsigned *pSourceOutputIndex,
	unsigned *pDestinationInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_ArmStage *pArmSrcStage = (BDSP_ArmStage *)pSrcStageHandle;
	BDSP_ArmStage *pArmDstStage = (BDSP_ArmStage *)pDstStageHandle;
	unsigned opIndex=0, ipIndex =0, channels =0, interStagePortIndex = 0;
	BDSP_P_ConnectionDetails *psStageOutput, *psStageInput;
	BDSP_P_InterStagePortInfo *psInterStagePortInfo;
	const BDSP_P_AlgorithmInfo *psAlgoInfo;

	BDBG_ENTER(BDSP_Arm_P_AddOutputStage);
	BDBG_OBJECT_ASSERT(pArmSrcStage, BDSP_ArmStage);
	BDBG_OBJECT_ASSERT(pArmDstStage, BDSP_ArmStage);

	psAlgoInfo = BDSP_P_LookupAlgorithmInfo(pArmSrcStage->eAlgorithm);

	BDSP_P_GetFreeOutputPortIndex(&(pArmSrcStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	BDSP_P_GetFreeOutputPortIndex(&(pArmDstStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	BDBG_MSG(("BDSP_Arm_P_AddOutputStage: Connecting Stage(%s) to Stage(%s) with datatype =%s",
		Algorithm2Name[pArmSrcStage->eAlgorithm], Algorithm2Name[pArmDstStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pArmSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pArmSrcStage->sStageConnectionInfo.sStageOutput[opIndex];
	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eStage;
	psStageOutput->dataType        = dataType;
	psStageOutput->connectionHandle.stage.hStage = &(pArmDstStage->stage);
	/*Data Access Population for Source Stage Output*/
	BDSP_P_GetInterStagePortIndex(&pArmSrcStage->sStageConnectionInfo,
	    pArmSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex],
	    &interStagePortIndex);
	psInterStagePortInfo = &pArmSrcStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
	psInterStagePortInfo->ePortDataType = pArmSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex];
	psInterStagePortInfo->branchFromPort++;
	psInterStagePortInfo->dataAccessAttributes.eDataAccessType    = BDSP_AF_P_Port_eChannelInterleavedPCM;
	psInterStagePortInfo->dataAccessAttributes.ui32bytesPerSample = 4;
	psInterStagePortInfo->dataAccessAttributes.ui32maxChannelSize = psAlgoInfo->samplesPerChannel*4;
	psInterStagePortInfo->dataAccessAttributes.ui32numChannels	  = psAlgoInfo->maxChannelsSupported;

	pArmSrcStage->totalOutputs+=1;
	pArmSrcStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eStage]+=1;
	*pSourceOutputIndex = opIndex;

	psStageInput = (BDSP_P_ConnectionDetails *)&pArmDstStage->sStageConnectionInfo.sStageInput[ipIndex];
	psStageInput->eValid		   = BDSP_AF_P_eValid;
	psStageInput->eConnectionType  = BDSP_ConnectionType_eStage;
	psStageInput->dataType         = dataType;
	psStageInput->connectionHandle.stage.hStage = &(pArmSrcStage->stage);

	pArmDstStage->totalInputs+=1;
	pArmDstStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eStage]+=1;
	*pDestinationInputIndex = ipIndex;

	BDBG_LEAVE(BDSP_Arm_P_AddOutputStage);
	return errCode;
}

void BDSP_Arm_P_RemoveInput(
	void *pStageHandle,
	unsigned inputIndex
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageInputDetails;

	BDBG_ENTER(BDSP_Arm_P_RemoveInput);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(inputIndex < BDSP_AF_P_MAX_IP_FORKS);
	if(pArmStage->totalInputs == 0)
	{
		BDBG_ERR(("BDSP_Arm_P_RemoveInput: Invalid index(%d), Number of inputs to stage is already Zero", inputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pArmStage->sStageConnectionInfo;
	pStageInputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageInput[inputIndex];
	connectionType = pStageInputDetails->eConnectionType;
	if((pArmStage->running)&&((connectionType != BDSP_ConnectionType_eInterTaskBuffer)&&(connectionType != BDSP_ConnectionType_eFmmBuffer)))
	{
		BDBG_ERR(("Cannot remove inputs when the stage is running"));
		return;
	}

	if(pArmStage->running)
    {
        errCode = BDSP_Arm_P_ReconfigCit(pArmStage,false,pStageInputDetails,inputIndex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_RemoveInput: Error in Reconfig of CIT"));
            goto end;
        }
    }

    if(connectionType == BDSP_ConnectionType_eInterTaskBuffer)
    {
        BDSP_P_InterTaskBuffer *pInterTaskbuffer = (BDSP_P_InterTaskBuffer *)pStageInputDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;
        pInterTaskbuffer->dstIndex = -1;
        pInterTaskbuffer->dstHandle= NULL;
        if(NULL == pInterTaskbuffer->srcHandle)
        {
            pInterTaskbuffer->inUse = false;
            pInterTaskbuffer->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
        }
    }

	pStageConnectionInfo->numInputs[connectionType]--;
	pStageInputDetails->eValid = BDSP_AF_P_eInvalid;
	pStageInputDetails->eConnectionType = BDSP_ConnectionType_eMax;
	BKNI_Memset(&pStageInputDetails->connectionHandle, 0, sizeof(pStageInputDetails->connectionHandle));
	pArmStage->totalInputs--;

end:
	BDBG_LEAVE(BDSP_Arm_P_RemoveInput);
	return;
}

void BDSP_Arm_P_RemoveAllInputs(
	void *pStageHandle
)
{
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	unsigned i, numInputs;

	BDBG_ENTER(BDSP_Arm_P_RemoveAllInputs);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	numInputs = pArmStage->totalInputs;

	for (i = 0; i < numInputs; i++)
	{
		BDSP_Arm_P_RemoveInput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pArmStage->totalInputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pArmStage->sStageConnectionInfo.numInputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Arm_P_RemoveAllInputs);
	return;
}

void BDSP_Arm_P_RemoveOutput(
	void *pStageHandle,
	unsigned outputIndex
)
{
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageOutputDetails;

	BDBG_ENTER(BDSP_Arm_P_RemoveOutput);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	BDBG_ASSERT(outputIndex < BDSP_AF_P_MAX_OP_FORKS);

	if(pArmStage->totalOutputs == 0)
	{
		BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Invalid index(%d), Number of outputs to stage is already Zero", outputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pArmStage->sStageConnectionInfo;
	pStageOutputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageOutput[outputIndex];
	connectionType = pStageOutputDetails->eConnectionType;
	if(pArmStage->running)
	{
		BDBG_ERR(("Cannot remove output when the stage is running"));
		return;
	}

	if(BDSP_ConnectionType_eStage == connectionType)
	{
		BDSP_P_InterStagePortInfo *psInterStagePortInfo;
		unsigned interStagePortIndex = 0;
		BDSP_P_GetInterStagePortIndex(&pArmStage->sStageConnectionInfo,
            pStageConnectionInfo->eStageOpDataType[outputIndex],
            &interStagePortIndex);
		psInterStagePortInfo = &pArmStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
		psInterStagePortInfo->branchFromPort--;
		if(psInterStagePortInfo->branchFromPort == 0)
		{
			psInterStagePortInfo->ePortDataType = BDSP_AF_P_DistinctOpType_eMax;
			psInterStagePortInfo->tocIndex      = BDSP_AF_P_TOC_INVALID;
			BKNI_Memset(&psInterStagePortInfo->bufferDescriptorAddr[0][0], 0, (sizeof(dramaddr_t)*BDSP_AF_P_MAX_PORT_BUFFERS));
			BKNI_Memset(&psInterStagePortInfo->dataAccessAttributes, 0, sizeof(BDSP_AF_P_Port_sDataAccessAttributes));
		}
	}
	else if(BDSP_ConnectionType_eInterTaskBuffer == connectionType)
	{
		BDSP_P_InterTaskBuffer *pArmInterTaskBuffer;

		/* Get the inter task buffer handle */
		pArmInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pStageOutputDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;

		pArmInterTaskBuffer->srcHandle = NULL;
		pArmInterTaskBuffer->srcIndex = -1;

		if (NULL == pArmInterTaskBuffer->dstHandle)
		{
			pArmInterTaskBuffer->inUse = false;
			pArmInterTaskBuffer->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
		}
	}
	else if(BDSP_ConnectionType_eRDBBuffer== connectionType)
	{
	    BDBG_ERR(("BDSP_Arm_P_RemoveOutput: Queue support is not available in ARM"));
#if 0
		BDSP_ArmQueue *pArmQueue;

		/* Get the inter task buffer handle */
		pArmQueue = (BDSP_ArmQueue *)pStageOutputDetails->connectionHandle.rdb.pQHandle->pQueueHandle;

		pArmQueue->srcHandle = NULL;
		pArmQueue->srcIndex = -1;

		if (NULL == pArmQueue->dstHandle)
		{
			pArmQueue->inUse = false;
			pArmQueue->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
		}
#endif
	}

	pStageConnectionInfo->numOutputs[connectionType]--;
	pStageConnectionInfo->eStageOpDataType[outputIndex] = BDSP_AF_P_DistinctOpType_eMax;

	pStageOutputDetails->eValid = BDSP_AF_P_eInvalid;
	pStageOutputDetails->eConnectionType = BDSP_ConnectionType_eMax;
	BKNI_Memset(&pStageOutputDetails->connectionHandle, 0, sizeof(pStageOutputDetails->connectionHandle));
	pArmStage->totalOutputs--;

	return;
}

void BDSP_Arm_P_RemoveAllOutputs(
	void *pStageHandle)
{
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStageHandle;
	unsigned i, numOutputs;

	BDBG_ENTER(BDSP_Arm_P_RemoveAllOutputs);

	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	numOutputs = pArmStage->totalOutputs;

	for (i = 0; i < numOutputs; i++)
	{
		BDSP_Arm_P_RemoveOutput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pArmStage->totalOutputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pArmStage->sStageConnectionInfo.numOutputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Arm_P_RemoveAllOutputs);
}
