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

BDBG_MODULE(bdsp_raaga_io_priv);

/***********************************************************************
Name        :   BDSP_Raaga_P_InterTaskBuffer_Create

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
BERR_Code BDSP_Raaga_P_InterTaskBuffer_Create(
	void 						*pContextHandle,
	BDSP_DataType 				 dataType,
	BDSP_BufferType 			 bufferType,
	BDSP_InterTaskBufferHandle 	*pInterTaskBufferHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
	BDSP_AF_P_DistinctOpType distinctOp;
	BDSP_RaagaContext *pRaagaContext;
	unsigned channels=0, numtoc=0, numMetaData=0, numObjectData=0;
    unsigned memReqd=0, i= 0;
    BDSP_MMA_Memory Memory;
	BDBG_ENTER(BDSP_Raaga_P_InterTaskBuffer_Create);
	pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

	if((bufferType != BDSP_BufferType_eRDB)&&(bufferType != BDSP_BufferType_eDRAM))
	{
		BDBG_ERR(("Only DRAM(0) and RDB(1) buffer type of InterTask buffer supported in Raaga!!!. Buffer type provided = %d", dataType));
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
	pInterTaskBuffer->interTaskBuffer.destroy = BDSP_Raaga_P_InterTaskBuffer_Destroy;
	pInterTaskBuffer->interTaskBuffer.flush   = BDSP_Raaga_P_InterTaskBuffer_Flush;

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
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pRaagaContext->pDevice->memHandle,
                            memReqd,
                            &Memory,
                            BDSP_MMA_Alignment_4KByte);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Allocate memory for inter task io Buffer !"));
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
            BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to allocate memory for PCM data"));
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
            BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to allocate memory for TOC data"));
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
            BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to allocate memory for Meta data"));
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
            BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to allocate memory for Object data"));
            goto end;
        }
        pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.Memory   = Memory;
        pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.ui32Size = BDSP_AF_P_OBJECTDATA_BUFFER_SIZE;
    }

    if(BDSP_BufferType_eRDB == bufferType)
    {
        unsigned numFifos = 0;
        uint32_t FifoId   = 0;
        pInterTaskBuffer->ebufferType = BDSP_AF_P_BufferType_eRDB;
        numFifos = channels+numtoc+numMetaData+numObjectData;
        errCode = BDSP_Raaga_P_AssignFreeFIFO(pRaagaContext->pDevice, 0, &FifoId, numFifos);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to allocate RDB FIFO for Intertask buffer"));
            goto error_rdb;
        }
        for(i=0;i<channels;i++)
        {
            pInterTaskBuffer->PcmDetails[i].BuffersDetails.ui32FifoId = FifoId+i;
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->PcmDetails[i].BuffersDetails,
                    pRaagaContext->pDevice->regHandle,
                    pRaagaContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->PcmDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Create Queue for PCM buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->PcmDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for PCM buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numtoc;i++)
        {
            pInterTaskBuffer->TocDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->TocDetails[i].BuffersDetails,
                    pRaagaContext->pDevice->regHandle,
                    pRaagaContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->TocDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Create Queue for TOC buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->TocDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for TOC buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numMetaData;i++)
        {
            pInterTaskBuffer->MetaDataDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+numtoc+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->MetaDataDetails[i].BuffersDetails,
                    pRaagaContext->pDevice->regHandle,
                    pRaagaContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Create Queue for MetaData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for MetaData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
        for(i=0;i<numObjectData;i++)
        {
            pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails.ui32FifoId = (FifoId+channels+numtoc+numMetaData+i);
            errCode = BDSP_P_CreateMsgQueue(
                    &pInterTaskBuffer->ObjectDataDetails[i].BuffersDetails,
                    pRaagaContext->pDevice->regHandle,
                    pRaagaContext->pDevice->dspOffset[0],
                    &(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Create Queue for ObjectData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle);
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Create: Unable to Initialise Command Queue for ObjectData buffer(%d)", i));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }
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

error_rdb:
    BDSP_MMA_P_FreeMemory(&Memory);
error:
    BKNI_Free(pInterTaskBuffer);
end:
	BDBG_LEAVE(BDSP_Raaga_P_InterTaskBuffer_Create);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_InterTaskBuffer_Destroy

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
void BDSP_Raaga_P_InterTaskBuffer_Destroy(
	void *pInterTaskBufferHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
    BDSP_RaagaContext *pRaagaContext;
    unsigned numFifos = 0;
    uint32_t FifoId   = 0;

	BDBG_ENTER(BDSP_Raaga_P_InterTaskBuffer_Destroy);
	pInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pInterTaskBufferHandle;
	BDBG_OBJECT_ASSERT(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
    pRaagaContext = (BDSP_RaagaContext *)pInterTaskBuffer->pContext;

	if (pInterTaskBuffer->dstHandle || pInterTaskBuffer->srcHandle)
	{
		BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Destroy: Cannot destroy inter task buffer when in use. Please disconnect any input/output stages"));
		BDBG_ERR(("Handles connected to intertask buf - dst 0x%p src 0x%p",(void *)pInterTaskBuffer->dstHandle,(void *)pInterTaskBuffer->srcHandle));
		BERR_TRACE(BERR_NOT_AVAILABLE);
		goto end;
	}
    if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
    {
        FifoId = pInterTaskBuffer->PcmDetails[0].BuffersDetails.ui32FifoId;
        numFifos = pInterTaskBuffer->numChannels+pInterTaskBuffer->numTocData+
                   pInterTaskBuffer->numMetaData+pInterTaskBuffer->numObjectData;
        errCode = BDSP_Raaga_P_ReleaseFIFO(pRaagaContext->pDevice, 0, &FifoId, numFifos);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_InterTaskBuffer_Destroy: Unable to release fifo %d for RDB Type Intertask buffer !!!!", FifoId));
			errCode = BERR_TRACE(errCode);
			BDBG_ASSERT(0);
		}
    }

    BDSP_MMA_P_FreeMemory(&pInterTaskBuffer->MemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BKNI_Free(pInterTaskBuffer);

end:
	BDBG_LEAVE(BDSP_Raaga_P_InterTaskBuffer_Destroy);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_InterTaskBuffer_Flush

Type        :   PI Interface

Input       :   pInterTaskBufferHandle  -   Handle of the Intertask buffer to be flushed.

Return      :   None

Functionality   :   To flush any buffer, we need to reset the Read, Write and Wrap pointers.
		1)  For every I/O buffer (number of channels), Reset Read and Write to Start Address and Wrap to End Address.
		2)  For the only I/O Generic buffer, Reset Read and Write to Start Address and Wrap to End Address.
***********************************************************************/
void BDSP_Raaga_P_InterTaskBuffer_Flush(
	void *pInterTaskBufferHandle
)
{
	BDSP_P_InterTaskBuffer *pInterTaskBuffer;
    BDSP_RaagaContext      *pRaagaContext;
    BDSP_Raaga             *pRaaga;
    unsigned i = 0;
	BDBG_ENTER(BDSP_Raaga_P_InterTaskBuffer_Flush);

	pInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pInterTaskBufferHandle;
	BDBG_OBJECT_ASSERT(pInterTaskBuffer, BDSP_P_InterTaskBuffer);
    pRaagaContext = (BDSP_RaagaContext *)pInterTaskBuffer->pContext;
    pRaaga        = (BDSP_Raaga *)pRaagaContext->pDevice;
    for(i=0;i<pInterTaskBuffer->numChannels;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
            BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->PcmDetails[i].hQueueHandle);
        }
        else
        {
            pInterTaskBuffer->PcmDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->PcmDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pRaaga->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex][0],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_Data][i],
                    pInterTaskBuffer->PcmDetails[i].BufferPointer.BaseOffset);
            }
        }
    }
    for(i=0;i<pInterTaskBuffer->numTocData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
            BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->TocDetails[i].hQueueHandle);
        }
        else
        {
            pInterTaskBuffer->TocDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->TocDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pRaaga->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex][0],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_TOC][i],
                    pInterTaskBuffer->TocDetails[i].BufferPointer.BaseOffset);
            }
        }
    }
    for(i=0;i<pInterTaskBuffer->numMetaData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
            BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->MetaDataDetails[i].hQueueHandle);
        }
        else
        {
            pInterTaskBuffer->MetaDataDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->MetaDataDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pRaaga->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex][0],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_MetaData][i],
                    pInterTaskBuffer->MetaDataDetails[i].BufferPointer.BaseOffset);
            }
        }
    }
    for(i=0;i<pInterTaskBuffer->numObjectData;i++)
    {
        if(BDSP_AF_P_BufferType_eRDB == pInterTaskBuffer->ebufferType)
        {
            BDSP_Raaga_P_InitMsgQueue(pInterTaskBuffer->ObjectDataDetails[i].hQueueHandle);
        }
        else
        {
            pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.ReadOffset = pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset;
            pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.WriteOffset= pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset;
            if(pInterTaskBuffer->descriptorAllocated)
            {
                BDSP_P_InitDramBuffer(&pRaaga->memInfo.DescriptorMemory[pInterTaskBuffer->dspIndex][0],
                    pInterTaskBuffer->bufferDescriptorAddr[BDSP_AF_P_PortBuffer_Type_ObjectData][i],
                    pInterTaskBuffer->ObjectDataDetails[i].BufferPointer.BaseOffset);
            }
        }
    }

	BDBG_LEAVE(BDSP_Raaga_P_InterTaskBuffer_Flush);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetDefaultCreateQueueSettings

Type        :   PI Interface

Input       :   pContextHandle  -   Handle of the Context for which the Queue needs to created.
				pSettings           -   Settings for creating the Queue.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Clear the memory provided by the PI as part of this function.
***********************************************************************/
BERR_Code BDSP_Raaga_P_GetDefaultCreateQueueSettings(
	void                     *pContextHandle,
	BDSP_QueueCreateSettings *pSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDBG_ENTER(BDSP_Raaga_P_GetDefaultCreateQueueSettings);

	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

	BKNI_Memset(pSettings, 0, sizeof(BDSP_QueueCreateSettings));

	BDBG_LEAVE(BDSP_Raaga_P_GetDefaultCreateQueueSettings);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Queue_Create

Type        :   PI Interface

Input       :   pContextHandle  -   Handle of the Context for which the Queue needs to created.
				dspIndex            -   Index of the DSP.
				pSettings           -   Settings for creating the Queue.
				pQueueHandle        -   Queue Handle returned back to the PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Allocate and Initialise the memory for the Queue Descriptor.
		2)  Depending on whether the Data is CDB or ITB for which the Queue is to be created, assign the FIFO-ID.
		3)  Create the Queue using the internal BDSP function with given settings.
		4)  Intialise all the function pointers  for Flush and Destroy, which will be used by the PI.
***********************************************************************/
BERR_Code BDSP_Raaga_P_Queue_Create(
	void 					 *pContextHandle,
	unsigned 				  dspIndex,
	BDSP_QueueCreateSettings *pSettings,
	BDSP_QueueHandle 		 *pQueueHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaContext  *pRaagaContext;
	BDSP_RaagaQueue    *pQueue;
	BDSP_AF_P_DistinctOpType distinctOp;
	unsigned channels =0, i = 0;
	BDSP_P_MsgQueueParams sMsgQueueParams;

	BDBG_ENTER(BDSP_Raaga_P_Queue_Create);
	pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);

	pQueue   = BKNI_Malloc(sizeof(BDSP_RaagaQueue));
	if ( NULL == pQueue )
	{
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("Unable to Allocate memory for Queue Handle !!!"));
		goto end;
	}

	BKNI_Memset(pQueue, 0, sizeof(*pQueue));
	BDSP_P_InitQueue(&pQueue->Queue, pQueue);

	/* Initialize the Queue apis */
	pQueue->Queue.destroy = BDSP_Raaga_P_Queue_Destroy;
	pQueue->Queue.flush   = BDSP_Raaga_P_Queue_Flush;
	pQueue->Queue.getIoBuffer = BDSP_Raaga_P_Queue_GetIoBuffer;
	pQueue->Queue.getBuffer = BDSP_Raaga_P_Queue_GetBuffer;
	pQueue->Queue.consumeData = BDSP_Raaga_P_Queue_ConsumeData;
	pQueue->Queue.commitData = BDSP_Raaga_P_Queue_CommitData;
	pQueue->Queue.getOpBufferAddr = BDSP_Raaga_P_Queue_GetBufferAddr;

	pQueue->inUse     = false;
	pQueue->srcHandle = NULL;
	pQueue->dstHandle = NULL;
	pQueue->srcIndex  = -1;
	pQueue->dstIndex  = -1;
	pQueue->dspIndex  = dspIndex;
	pQueue->pRaagaContext  = pRaagaContext;
	pQueue->input = pSettings->input;

	BDSP_P_GetDistinctOpTypeAndNumChans(pSettings->dataType, &channels, &distinctOp);
	pQueue->numChannels = channels;
	pQueue->distinctOp  = distinctOp;

    /* copy the Create settings */
    BKNI_Memcpy((void *)&pQueue->createSettings,(void *)pSettings, sizeof(BDSP_QueueCreateSettings));
	for(i = 0; i< pSettings->numBuffers; i++)
	{
		errCode = BDSP_Raaga_P_AssignFreeFIFO(pRaagaContext->pDevice,dspIndex,&(sMsgQueueParams.ui32FifoId),1);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Queue_Create: Unable to find a free FIFO for QUEUE!!!!"));
			errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			BDBG_ASSERT(0);
		}
		sMsgQueueParams.Memory   = pSettings->bufferInfo[i].buffer;
		sMsgQueueParams.ui32Size = pSettings->bufferInfo[i].bufferSize;
		errCode = BDSP_P_CreateMsgQueue(
				&sMsgQueueParams,
				pRaagaContext->pDevice->regHandle,
				pRaagaContext->pDevice->dspOffset[dspIndex],
				&(pQueue->hMsgQueue[i]));
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Queue_Create: Unable to Create Command Queue for buffer(%d)", i));
			errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			BDBG_ASSERT(0);
		}

        errCode = BDSP_Raaga_P_InitMsgQueue(pQueue->hMsgQueue[i]);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Queue_Create: Unable to Initialise Command Queue for buffer(%d)", i));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }
	}

	*pQueueHandle = &pQueue->Queue;
	BDBG_OBJECT_SET(pQueue, BDSP_RaagaQueue);

end:
	BDBG_LEAVE(BDSP_Raaga_P_Queue_Create);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Queue_Destroy

Type        :   PI Interface

Input       :   pQueueHandle        -   Queue Handle returned back to the PI.

Return      :   NONE

Functionality   :   Following are the operations performed.
		1)  Chech whether the Queue you intended to destroy is in Use.
		2)  Depending on the number of buffers, release all the FIFOs.
		3)  Destroy the Queue Handle and Free the memory used to store the data structure.
***********************************************************************/
void BDSP_Raaga_P_Queue_Destroy(
	void *pQueueHandle
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaQueue *pQueue;
	unsigned i = 0;
	uint32_t ui32FifoId;

	BDBG_ENTER(BDSP_Raaga_P_Queue_Destroy);
	pQueue = (BDSP_RaagaQueue *)pQueueHandle;
	BDBG_OBJECT_ASSERT(pQueue, BDSP_RaagaQueue);

	if (pQueue->inUse)
	{
		BDBG_ERR(("BDSP_Raaga_P_Queue_Destroy: Cannot destroy Queue when in use. Please disconnect any input/output stages"));
		BDBG_ASSERT(0);
	}

	/* Free the Queue Handle Allocated */
	for(i = 0; i< pQueue->createSettings.numBuffers; i++)
	{
		if(pQueue->hMsgQueue[i])
		{
			ui32FifoId = pQueue->hMsgQueue[i]->ui32FifoId;
			errCode = BDSP_P_DestroyMsgQueue(pQueue->hMsgQueue[i]);
			if(errCode == BERR_SUCCESS)
			{
				errCode = BDSP_Raaga_P_ReleaseFIFO(pQueue->pRaagaContext->pDevice, pQueue->dspIndex, &ui32FifoId, 1);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_Queue_Destroy: Unable to release fifo %d for RDB Queue !!!!",ui32FifoId));
					errCode = BERR_TRACE(errCode);
					BDBG_ASSERT(0);
				}
			}
			else
			{
				BDBG_ERR(("BDSP_Raaga_P_Queue_Destroy: Error in Destroying the Message Queue"));
				errCode = BERR_TRACE(errCode);
				BDBG_ASSERT(0);
			}
		}
	}
	BDBG_OBJECT_DESTROY(pQueue, BDSP_RaagaQueue);
	BKNI_Free(pQueue);

	BDBG_LEAVE(BDSP_Raaga_P_Queue_Destroy);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Queue_Flush

Type        :   PI Interface

Input       :   pQueueHandle        -   Queue Handle returned back to the PI.

Return      :   NONE

Functionality   :   Following are the operations performed.
		1) INIT the Message Queue created for the Queue.
***********************************************************************/
void BDSP_Raaga_P_Queue_Flush(
	void *pQueueHandle
)
{
	BDSP_RaagaQueue *pQueue;
	unsigned i;

	BDBG_ENTER(BDSP_Raaga_P_Queue_Flush);

	pQueue = (BDSP_RaagaQueue *)pQueueHandle;
	BDBG_OBJECT_ASSERT(pQueue, BDSP_RaagaQueue);

	if (pQueue->inUse)
	{
		BDBG_ERR(("BDSP_Raaga_P_Queue_Flush: Cannot flush Queue when in use"));
		BDBG_ASSERT(0);
	}

	for (i = 0; i < pQueue->createSettings.numBuffers; i++)
	{
		/* Reset the Message Queue  */
		BDSP_Raaga_P_InitMsgQueue(pQueue->hMsgQueue[i]);
	}

	BDBG_LEAVE(BDSP_Raaga_P_Queue_Flush);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddRaveInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which input must be fed from RAVE Buffer.
				pContextMap     -   Context map of the XPT.
				pInputIndex     -   Index of the input for this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the free input index for the stage which can be used for this RAVE buffer.
		2)  Treat the CDB and ITB buffers as two buffers for same input Index.
		3)  Populate the Source details for the Stage with appropriate values using the XPT descriptor, each for CDB and ITB buffer.
		4)  Populate the Address of the I/O buffers, each for CDB and ITB buffer.
		5)  Increment the number of inputs for this stage (both CDB and ITB are treated as 1 input) and
			also the number of RAVE buffers (as 1 for both CDB and ITB) in the eco-system.
		6)  Fill the Input index pointer for the PI for later use (only 1 as both CDB and ITB are treated as same input).
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddRaveInput(
	void *pStageHandle,
	const BAVC_XptContextMap *pContextMap,
	unsigned *pInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned ipIndex=0;
	BDSP_P_ConnectionDetails *psStageInput;

	BDBG_ENTER(BDSP_Raaga_P_AddRaveInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);

	BDBG_MSG(("BDSP_Raaga_P_AddRaveInput:Connecting RAVE Input to Stage (%s)", Algorithm2Name[pRaagaStage->eAlgorithm]));
	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);
	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[ipIndex];

	psStageInput->eValid          = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eRaveBuffer;
	psStageInput->connectionHandle.rave.raveContextMap = *pContextMap;

	pRaagaStage->totalInputs+=1;
	pRaagaStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eRaveBuffer]+=1;
	*pInputIndex = ipIndex;

	BDBG_LEAVE(BDSP_Raaga_P_AddRaveInput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddRaveOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which input must be fed from RAVE Buffer.
				pContextMap     -   Context map of the XPT.
				pInputIndex     -   Index of the input for this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the free input index for the stage which can be used for this RAVE buffer.
		2)  Treat the CDB and ITB buffers as two buffers for same input Index.
		3)  Populate the Source details for the Stage with appropriate values using the XPT descriptor, each for CDB and ITB buffer.
		4)  Populate the Address of the I/O buffers, each for CDB and ITB buffer.
		5)  Increment the number of inputs for this stage (both CDB and ITB are treated as 1 input) and
			also the number of RAVE buffers (as 1 for both CDB and ITB) in the eco-system.
		6)  Fill the Input index pointer for the PI for later use (only 1 as both CDB and ITB are treated as same input).
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddRaveOutput(
	void *pStageHandle,
	const BAVC_XptContextMap *pContextMap,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned opIndex=0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_AddRaveOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);

	BDBG_MSG(("BDSP_Raaga_P_AddRaveOutput:Connecting RAVE Output to Stage (%s)", Algorithm2Name[pRaagaStage->eAlgorithm]));
	BDSP_P_GetFreeOutputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[opIndex];

	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eRaveBuffer;
	psStageOutput->connectionHandle.rave.raveContextMap = *pContextMap;

	pRaagaStage->totalOutputs+=1;
	pRaagaStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eRaveBuffer]+=1;
	*pOutputIndex = opIndex;

	BDBG_LEAVE(BDSP_Raaga_P_AddRaveOutput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddFmmInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which FMM buffer needs to be connected at input.
				dataType            -   Type of data present in the FMM buffer.
				pDescriptor     -   FMM buffer descriptor provided by the PI.
				pInputIndex     -   Index of the input which is associated for the FMM buffer.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free input index for the stage which can be used for this FMM buffer.
		2)  Depending on the datatype, figure out the number of channels and input type.
		3)  Populate the Source details for the Stage with appropriate values using the FMM descriptor provided by the PI.
		4)  Increment the number of inputs for this stage and also the number of FMM buffers in the eco-system.
		5)  Fill the input index pointer for the PI for later use.
		6)  IF the stage is already running/ watchdog recovery then perform the CIT reconfigure.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddFmmInput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_FmmBufferDescriptor *pDescriptor,
	unsigned *pInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned ipIndex=0, channels=0, index =0;
	BDSP_P_ConnectionDetails *psStageInput;
	BDSP_AF_P_DistinctOpType distinctOp;

	BDBG_ENTER(BDSP_Raaga_P_AddFmmInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[ipIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddFmmInput:Connecting FMM Input to Stage (%s) with datatype = %s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &distinctOp);
	if(channels!=pDescriptor->numBuffers)
	{
		errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("BDSP_Raaga_P_AddFmmInput::FMM Input not added!! Channels = %d and buffers in descriptor = %d",channels,pDescriptor->numBuffers));
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

	pRaagaStage->totalInputs+=1;
	pRaagaStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eFmmBuffer]+=1;
	*pInputIndex = ipIndex;

	if ((pRaagaStage->running)&&(!pRaagaStage->pContext->contextWatchdogFlag))
	{
		errCode = BDSP_Raaga_P_ReconfigCit(pRaagaStage,true,psStageInput,ipIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_AddFmmInput: Error in Reconfig of CIT"));
			goto end;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_AddFmmInput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddFmmOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be fed to FMM Buffer.
				dataType            -   Type of data that will fed into FMM buffer.
				pDescriptor     -   Descriptor for the FMM buffer.
				pOutputIndex        -   Index of the Ouput from this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output index for the stage which can be used for this FMM buffer.
		2)  Depending on the datatype, figure out the number of channels and output type.
		3)  Populate the Dstination details for the Stage with appropriate values using the FMM descriptor provided by the PI.
		4)  Populate the Address of the I/O buffers and rate control data.
		5)  Increment the number of ouputs from this stage and also the number of FMM buffers in the eco-system.
		6)  Fill the Ouput index pointer for the PI for later use.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddFmmOutput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_FmmBufferDescriptor *pDescriptor,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned opIndex=0, channels =0, index = 0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_AddFmmOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDSP_P_GetFreeOutputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddFmmOutput:Connecting FMM output to Stage (%s) with datatype = %s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pRaagaStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	if(channels!=pDescriptor->numBuffers)
	{
		errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("BDSP_Raaga_P_AddFmmOutput::FMM Output not added!! Channels = %d and buffers in descriptor = %d",channels,pDescriptor->numBuffers));
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

	pRaagaStage->totalOutputs+=1;
	pRaagaStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eFmmBuffer]+=1;
	*pOutputIndex = opIndex;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AddFmmOutput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddInterTaskBufferInput

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
BERR_Code BDSP_Raaga_P_AddInterTaskBufferInput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_InterTaskBuffer *pBufferHandle,
	unsigned *pInputIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_P_InterTaskBuffer *pRaagaInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
	unsigned ipIndex;
	BDSP_P_ConnectionDetails *psStageInput;

	BDBG_ENTER(BDSP_Raaga_P_AddInterTaskBufferInput);

	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BDBG_ASSERT(NULL != pInputIndex);
	BDBG_ASSERT(dataType == pRaagaInterTaskBuffer->dataType);

	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[ipIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddInterTaskBufferInput:Connecting Intertask Input to Stage %s with datatype=%s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[dataType]));

	psStageInput->eValid          = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eInterTaskBuffer;
	psStageInput->dataType        = dataType;
	psStageInput->connectionHandle.interTask.hInterTask = (BDSP_InterTaskBufferHandle)&pRaagaInterTaskBuffer->interTaskBuffer;;

	pRaagaStage->totalInputs+=1;
	pRaagaStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eInterTaskBuffer]+=1;

	pRaagaInterTaskBuffer->dstIndex  = ipIndex;
	pRaagaInterTaskBuffer->dstHandle = pStageHandle;
	*pInputIndex = ipIndex;

	if (pRaagaInterTaskBuffer->srcHandle)
	{
		pRaagaInterTaskBuffer->inUse = true;
	}

	if ((pRaagaStage->running)&&(!pRaagaStage->pContext->contextWatchdogFlag))
	{
		errCode = BDSP_Raaga_P_ReconfigCit(pRaagaStage,true,psStageInput,ipIndex);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_AddInterTaskBufferInput: Error in Reconfig of CIT"));
			goto end;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_AddInterTaskBufferInput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddInterTaskBufferOutput

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
BERR_Code BDSP_Raaga_P_AddInterTaskBufferOutput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_InterTaskBuffer *pBufferHandle,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_P_InterTaskBuffer *pRaagaInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pBufferHandle;
	unsigned opIndex;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_AddInterTaskBufferOutput);

	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaInterTaskBuffer, BDSP_P_InterTaskBuffer);
	BDBG_ASSERT(NULL != pOutputIndex);
	BDBG_ASSERT(dataType == pRaagaInterTaskBuffer->dataType);

	BDSP_P_GetFreeOutputPortIndex(&((pRaagaStage->sStageConnectionInfo.sStageOutput[0])), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddInterTaskBufferOutput:Connecting Intertask Output to Stage %s with datatype=%s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[dataType]));

	psStageOutput->eValid		  = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType= BDSP_ConnectionType_eInterTaskBuffer;
	psStageOutput->dataType		  = dataType;
	psStageOutput->connectionHandle.interTask.hInterTask = (BDSP_InterTaskBufferHandle)&pRaagaInterTaskBuffer->interTaskBuffer;;

	pRaagaStage->totalOutputs+=1;
	pRaagaStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eInterTaskBuffer]+=1;
	pRaagaStage->sStageConnectionInfo.eStageOpDataType[opIndex]=pRaagaInterTaskBuffer->distinctOp;

	pRaagaInterTaskBuffer->srcIndex  = opIndex;
	pRaagaInterTaskBuffer->srcHandle = pStageHandle;
	*pOutputIndex = opIndex;

	if (pRaagaInterTaskBuffer->dstHandle)
	{
		pRaagaInterTaskBuffer->inUse = true;
	}

	BDBG_LEAVE(BDSP_Raaga_P_AddInterTaskBufferOutput);
	return errCode;
}

#if !B_REFSW_MINIMAL
/***********************************************************************
Name        :   BDSP_Raaga_P_AddQueueInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose input will be from a Queue/RDB Buffer.
				pQueueHandle        -   Handle of the Queue provided by the PI.
				pInputIndex         -   Index of the input for this stage provided back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free input index for the stage which can be used for this Queue.
		2)  Populate the Source details for the Stage with appropriate values using the Queue descriptor provided by the PI.
			The type of input will be treated as RDB type each for CDB and ITB. Separate buffers are allocated for CDB and ITB
			and not treated as individual/seperate inputs.
		3)  Populate the Address of the I/O buffer using the FIFO address filled during Create Queue. There is no I/O Generic buffer.
		4)  Increment the number of inputs from RDB buffer for this stage and total inputs to the stage.
		5)  Fill the input index pointer for the PI for later use.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddQueueInput(
	void     *pStageHandle,
	void     *pQueueHandle,
	unsigned *pInputIndex /* [out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
	unsigned ipIndex;
	BDSP_P_ConnectionDetails *psStageInput;

	BDBG_ENTER(BDSP_Raaga_P_AddQueueInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);
	BDBG_ASSERT(NULL != pInputIndex);

	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[ipIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddQueueInput:Connecting Queue Input to Stage %s with datatype=%s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[pRaagaQueue->createSettings.dataType]));

	psStageInput->eValid		  = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eRDBBuffer;
	psStageInput->dataType		  = pRaagaQueue->createSettings.dataType;
	psStageInput->connectionHandle.rdb.pQHandle = &pRaagaQueue->Queue;

	pRaagaStage->totalInputs+=1;
	pRaagaStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eRDBBuffer]+=1;

	pRaagaQueue->dstIndex  = ipIndex;
	pRaagaQueue->dstHandle = pStageHandle;
	*pInputIndex = ipIndex;

	if (pRaagaQueue->srcHandle)
	{
		pRaagaQueue->inUse = true;
	}

	BDBG_LEAVE(BDSP_Raaga_P_AddQueueInput);
	return errCode;
}
#endif /*!B_REFSW_MINIMAL*/

/***********************************************************************
Name        :   BDSP_Raaga_P_AddQueueOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage whose output will be to a Queue/RDB Buffer.
				pQueueHandle        -   Handle of the Queue provided by the PI.
				pOutputIndex        -   Index of the output from this stage provided back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output index for the stage which can be used for this Queue.
		2)  Populate the Destination details for the Stage with appropriate values using the Queue descriptor provided by the PI.
			The type of output will be treated as RDB type each for CDB and ITB. Separate buffers are allocated for CDB and ITB
			and not treated as individual/seperate outputs.
		3)  Populate the Address of the I/O buffer using the FIFO address filled during Create Queue. There is no I/O Generic buffer.
		4)  Increment the number of outputs to RDB buffer for this stage and total outputs for the stage.
		5)  Fill the output index pointer for the PI for later use.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddQueueOutput(
	void     *pStageHandle,
	void     *pQueueHandle,
	unsigned *pOutputIndex /* [out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
	unsigned opIndex = 0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_AddQueueOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);
	BDBG_ASSERT(NULL != pOutputIndex);

	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);

	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddQueueOutput:Connecting Queue Output to Stage %s with datatype=%s",
        Algorithm2Name[pRaagaStage->eAlgorithm], DataType[pRaagaQueue->createSettings.dataType]));

	psStageOutput->eValid		  = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType= BDSP_ConnectionType_eRDBBuffer;
	psStageOutput->dataType		  = pRaagaQueue->createSettings.dataType;
	psStageOutput->connectionHandle.rdb.pQHandle = &pRaagaQueue->Queue;

	pRaagaStage->totalOutputs+=1;
	pRaagaStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eRDBBuffer]+=1;
	pRaagaStage->sStageConnectionInfo.eStageOpDataType[opIndex]=pRaagaQueue->distinctOp;

	pRaagaQueue->srcIndex  = opIndex;
	pRaagaQueue->srcHandle = pStageHandle;
	*pOutputIndex = opIndex;

	if(pRaagaQueue->dstHandle)
	{
		pRaagaQueue->inUse = true;
	}

	BDBG_LEAVE(BDSP_Raaga_P_AddQueueOutput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddOutputStage

Type        :   PI Interface

Input       :   pSrcStageHandle -   Handle of the Source Stage for which output must be fed to another Stage.
				dataType            -   Type of Data which is output of the Stage.
				pDstStageHandle -   Handle of the Destination Stage which must be connected at Output.
				pSourceOutputIndex  -   Index of the Ouput for the Source Stage.
				pDestinationInputIndex  -   Index of the Input for the Destination Stage.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output and index for the Source and Destination Stage respectively.
		2)  Interconnect the Stages but populating the Output and Input structures of Source Stage and Destination Stage using a InterStage Buffer.
		3)  Return the Source Output index and Destination Intput Index.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddOutputStage(
	void *pSrcStageHandle,
	BDSP_DataType dataType,
	void *pDstStageHandle,
	unsigned *pSourceOutputIndex,
	unsigned *pDestinationInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaSrcStage = (BDSP_RaagaStage *)pSrcStageHandle;
	BDSP_RaagaStage *pRaagaDstStage = (BDSP_RaagaStage *)pDstStageHandle;
	unsigned opIndex=0, ipIndex =0, channels =0, interStagePortIndex = 0;
	BDSP_P_ConnectionDetails *psStageOutput, *psStageInput;
	BDSP_P_InterStagePortInfo *psInterStagePortInfo;
	const BDSP_P_AlgorithmInfo *psAlgoInfo;

	BDBG_ENTER(BDSP_Raaga_P_AddOutputStage);
	BDBG_OBJECT_ASSERT(pRaagaSrcStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaDstStage, BDSP_RaagaStage);

	psAlgoInfo = BDSP_P_LookupAlgorithmInfo(pRaagaSrcStage->eAlgorithm);

	BDSP_P_GetFreeOutputPortIndex(&(pRaagaSrcStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	BDSP_P_GetFreeOutputPortIndex(&(pRaagaDstStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	BDBG_MSG(("BDSP_Raaga_P_AddOutputStage: Connecting Stage(%s) to Stage(%s) with datatype =%s",
		Algorithm2Name[pRaagaSrcStage->eAlgorithm], Algorithm2Name[pRaagaDstStage->eAlgorithm], DataType[dataType]));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaSrcStage->sStageConnectionInfo.sStageOutput[opIndex];
	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eStage;
	psStageOutput->dataType        = dataType;
	psStageOutput->connectionHandle.stage.hStage = &(pRaagaDstStage->stage);
	/*Data Access Population for Source Stage Output*/
	BDSP_P_GetInterStagePortIndex(&pRaagaSrcStage->sStageConnectionInfo,
	    pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex],
	    &interStagePortIndex);
	psInterStagePortInfo = &pRaagaSrcStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
	psInterStagePortInfo->ePortDataType = pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex];
	psInterStagePortInfo->branchFromPort++;
	psInterStagePortInfo->dataAccessAttributes.eDataAccessType    = BDSP_AF_P_Port_eChannelInterleavedPCM;
	psInterStagePortInfo->dataAccessAttributes.ui32bytesPerSample = 4;
	psInterStagePortInfo->dataAccessAttributes.ui32maxChannelSize = psAlgoInfo->samplesPerChannel*4;
	psInterStagePortInfo->dataAccessAttributes.ui32numChannels	  = psAlgoInfo->maxChannelsSupported;

	pRaagaSrcStage->totalOutputs+=1;
	pRaagaSrcStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eStage]+=1;
	*pSourceOutputIndex = opIndex;

	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaDstStage->sStageConnectionInfo.sStageInput[ipIndex];
	psStageInput->eValid		   = BDSP_AF_P_eValid;
	psStageInput->eConnectionType  = BDSP_ConnectionType_eStage;
	psStageInput->dataType         = dataType;
	psStageInput->connectionHandle.stage.hStage = &(pRaagaSrcStage->stage);

	pRaagaDstStage->totalInputs+=1;
	pRaagaDstStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eStage]+=1;
	*pDestinationInputIndex = ipIndex;

	BDBG_LEAVE(BDSP_Raaga_P_AddOutputStage);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage from which the input must be removed.
				inputIndex      -   The index of the input for the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Retrieve the Connection type used at the input.
		2)  Make sure the Stage is not running if the input is FMM or Intertask buffer, when attempt is made to remove the input.
		3)  If the Stage is running or in Wtachdog recovery, then CIT Reconfigure needs to be performed.
		4)  If the connection type is Intertask task buffer, remove the destination handle of the intertask buffer.
		5)  If source handle of the intertask buffer is NULL, then inUse variable is set to false.
			Now the Intertask buffer can be destroyed by PI later.
		6)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
		7)  Disconnect the Input but resetting Stage Input structure.
***********************************************************************/
void BDSP_Raaga_P_RemoveInput(
	void *pStageHandle,
	unsigned inputIndex
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageInputDetails;

	BDBG_ENTER(BDSP_Raaga_P_RemoveInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(inputIndex < BDSP_AF_P_MAX_IP_FORKS);
	if(pRaagaStage->totalInputs == 0)
	{
		BDBG_ERR(("BDSP_Raaga_P_RemoveInput: Invalid index(%d), Number of inputs to stage is already Zero", inputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pRaagaStage->sStageConnectionInfo;
	pStageInputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageInput[inputIndex];
	connectionType = pStageInputDetails->eConnectionType;
	if((pRaagaStage->running)&&((connectionType != BDSP_ConnectionType_eInterTaskBuffer)&&(connectionType != BDSP_ConnectionType_eFmmBuffer)))
	{
		BDBG_ERR(("Cannot remove inputs when the stage is running"));
		return;
	}

	if((pRaagaStage->running)&&(!pRaagaStage->pContext->contextWatchdogFlag))
    {
        errCode = BDSP_Raaga_P_ReconfigCit(pRaagaStage,false,pStageInputDetails,inputIndex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_RemoveInput: Error in Reconfig of CIT"));
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
	pRaagaStage->totalInputs--;

end:
	BDBG_LEAVE(BDSP_Raaga_P_RemoveInput);
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveAllInputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all inputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Recursively remove the all the inputs connected to the stage.
***********************************************************************/
void BDSP_Raaga_P_RemoveAllInputs(
	void *pStageHandle
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned i, numInputs;

	BDBG_ENTER(BDSP_Raaga_P_RemoveAllInputs);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	numInputs = pRaagaStage->totalInputs;

	for (i = 0; i < numInputs; i++)
	{
		BDSP_Raaga_P_RemoveInput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pRaagaStage->totalInputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pRaagaStage->sStageConnectionInfo.numInputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Raaga_P_RemoveAllInputs);
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be removed.
				outputIndex     -   The index of the Output in the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Make sure the Stage is not running, when attempt is made to remove the output.
		2)  Retrieve the Connection type used at the output.
		3)  If the connection type is Intertask task buffer, remove the source handle of the intertask buffer.
		4)  If destination handle of the intertask buffer is NULL, then inUse variable is set to false.
			Now the Intertask buffer can be destroyed by PI later.
		5)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
		6)  Disconnect the Output but resetting Stage Output structure.
		7)  If the Output is RAVE buffer then disconnection needs to be done for both CDB and ITB buffer.
***********************************************************************/
void BDSP_Raaga_P_RemoveOutput(
	void *pStageHandle,
	unsigned outputIndex
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageOutputDetails;

	BDBG_ENTER(BDSP_Raaga_P_RemoveOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(outputIndex < BDSP_AF_P_MAX_OP_FORKS);

	if(pRaagaStage->totalOutputs == 0)
	{
		BDBG_ERR(("BDSP_Raaga_P_RemoveOutput: Invalid index(%d), Number of outputs to stage is already Zero", outputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pRaagaStage->sStageConnectionInfo;
	pStageOutputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageOutput[outputIndex];
	connectionType = pStageOutputDetails->eConnectionType;
	if(pRaagaStage->running)
	{
		BDBG_ERR(("Cannot remove output when the stage is running"));
		return;
	}

	if(BDSP_ConnectionType_eStage == connectionType)
	{
		BDSP_P_InterStagePortInfo *psInterStagePortInfo;
		unsigned interStagePortIndex = 0;
		BDSP_P_GetInterStagePortIndex(&pRaagaStage->sStageConnectionInfo,
            pStageConnectionInfo->eStageOpDataType[outputIndex],
            &interStagePortIndex);
		psInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
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
		BDSP_P_InterTaskBuffer *pRaagaInterTaskBuffer;

		/* Get the inter task buffer handle */
		pRaagaInterTaskBuffer = (BDSP_P_InterTaskBuffer *)pStageOutputDetails->connectionHandle.interTask.hInterTask->pInterTaskBufferHandle;

		pRaagaInterTaskBuffer->srcHandle = NULL;
		pRaagaInterTaskBuffer->srcIndex = -1;

		if (NULL == pRaagaInterTaskBuffer->dstHandle)
		{
			pRaagaInterTaskBuffer->inUse = false;
			pRaagaInterTaskBuffer->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
		}
	}
	else if(BDSP_ConnectionType_eRDBBuffer== connectionType)
	{
		BDSP_RaagaQueue *pRaagaQueue;

		/* Get the inter task buffer handle */
		pRaagaQueue = (BDSP_RaagaQueue *)pStageOutputDetails->connectionHandle.rdb.pQHandle->pQueueHandle;

		pRaagaQueue->srcHandle = NULL;
		pRaagaQueue->srcIndex = -1;

		if (NULL == pRaagaQueue->dstHandle)
		{
			pRaagaQueue->inUse = false;
			pRaagaQueue->distinctOp = BDSP_AF_P_DistinctOpType_eMax;
		}
	}

	pStageConnectionInfo->numOutputs[connectionType]--;
	pStageConnectionInfo->eStageOpDataType[outputIndex] = BDSP_AF_P_DistinctOpType_eMax;

	pStageOutputDetails->eValid = BDSP_AF_P_eInvalid;
	pStageOutputDetails->eConnectionType = BDSP_ConnectionType_eMax;
	BKNI_Memset(&pStageOutputDetails->connectionHandle, 0, sizeof(pStageOutputDetails->connectionHandle));
	pRaagaStage->totalOutputs--;

	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveAllOutputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all outputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Recursively remove the all the outputs connected to the stage.
		2)  Special handling is done for RAVE buffer as both the outputs CDB and ITB buffers are removed in one instance.
***********************************************************************/
void BDSP_Raaga_P_RemoveAllOutputs(
	void *pStageHandle
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned i, numOutputs;

	BDBG_ENTER(BDSP_Raaga_P_RemoveAllOutputs);

	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	numOutputs = pRaagaStage->totalOutputs;

	for (i = 0; i < numOutputs; i++)
	{
		BDSP_Raaga_P_RemoveOutput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pRaagaStage->totalOutputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pRaagaStage->sStageConnectionInfo.numOutputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Raaga_P_RemoveAllOutputs);
}

void BDSP_Raaga_P_Queue_GetIoBuffer(
	void *pQueueHandle,
	BDSP_AF_P_sIO_BUFFER *pBuffer /*[out]*/
)
{
    BDSP_RaagaQueue *pRaagaQueue;
    unsigned i=0;
    BDBG_ENTER(BDSP_Raaga_P_Queue_GetIoBuffer);

    pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
    BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);

    pBuffer->ui32NumBuffers = pRaagaQueue->numChannels;
    pBuffer->eBufferType = BDSP_AF_P_BufferType_eRDB;

    /* Pass the RDB Registers used for maintaing the Read/Write pointers*/
    for (i=0; i < pRaagaQueue->numChannels; i++)
    {
        pBuffer->sCircBuffer[i].ui32BaseAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->hMsgQueue[i]->QueueAddress.BaseOffset);
        pBuffer->sCircBuffer[i].ui32EndAddr  = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->hMsgQueue[i]->QueueAddress.EndOffset);
        pBuffer->sCircBuffer[i].ui32ReadAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->hMsgQueue[i]->QueueAddress.ReadOffset);
        pBuffer->sCircBuffer[i].ui32WriteAddr= BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->hMsgQueue[i]->QueueAddress.WriteOffset);
        pBuffer->sCircBuffer[i].ui32WrapAddr = BDSP_REGSET_PHY_ADDR_FOR_DSP(pRaagaQueue->hMsgQueue[i]->QueueAddress.EndOffset);
    }
    BDBG_LEAVE(BDSP_Raaga_P_Queue_GetIoBuffer);
}

BERR_Code BDSP_Raaga_P_Queue_GetBuffer(
	void                  *pQueueHandle,
	BDSP_BufferDescriptor *pDescriptor /*[out] */
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaQueue *pRaagaQueue= (BDSP_RaagaQueue *)pQueueHandle;
	dramaddr_t BaseOffset=0, EndOffset=0, ReadOffset=0, WriteOffset=0;
	uint32_t ui32Size, ui32WrapSize = 0;

    BDBG_ENTER(BDSP_Raaga_P_Queue_GetBuffer);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);

    BaseOffset  = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.BaseOffset);
    EndOffset   = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.EndOffset);
    WriteOffset = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.WriteOffset);
    ReadOffset  = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.ReadOffset);

	if(pRaagaQueue->input)
	{
		if (ReadOffset > WriteOffset)
		{
			ui32Size     = ReadOffset - WriteOffset;
			ui32WrapSize = 0;
		}
		else
		{
			ui32Size     = EndOffset - WriteOffset;
			ui32WrapSize = ReadOffset- BaseOffset;
		}
        pDescriptor->buffers[0].buffer = pRaagaQueue->hMsgQueue[0]->Memory;
        pDescriptor->buffers[0].buffer.pAddr = (void *)((uint8_t *)pDescriptor->buffers[0].buffer.pAddr +
											(WriteOffset - BaseOffset));
	}
	else
	{
		if (WriteOffset >= ReadOffset)
		{
			ui32Size     = WriteOffset - ReadOffset;
			ui32WrapSize = 0;
		}
		else
		{
			ui32Size     = EndOffset  - ReadOffset;
			ui32WrapSize = WriteOffset- BaseOffset;
		}
        pDescriptor->buffers[0].buffer = pRaagaQueue->hMsgQueue[0]->Memory;
        pDescriptor->buffers[0].buffer.pAddr = (void *)((uint8_t *)pDescriptor->buffers[0].buffer.pAddr +
											(ReadOffset - BaseOffset));
	}
	pDescriptor->numBuffers  = 1;
	pDescriptor->interleaved = false;
	pDescriptor->bufferSize  = ui32Size;
	pDescriptor->wrapBufferSize = ui32WrapSize;

	if (ui32WrapSize)
	{
        pDescriptor->buffers[0].wrapBuffer = pRaagaQueue->hMsgQueue[0]->Memory;
	}
	else
	{
        pDescriptor->buffers[0].wrapBuffer.pAddr = NULL;
	}
	BDBG_LEAVE(BDSP_Raaga_P_Queue_GetBuffer);
	return errCode;
}

BERR_Code BDSP_Raaga_P_Queue_CommitData(
	void  *pQueueHandle,
	size_t bytesWritten
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
    dramaddr_t BaseOffset=0, EndOffset=0, WriteOffset=0, TempOffset=0;

	BDBG_ENTER(BDSP_Raaga_P_Queue_CommitData);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);

	if(!pRaagaQueue->input)
	{
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

    BaseOffset  = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.BaseOffset);
    EndOffset   = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.EndOffset);
    WriteOffset = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.WriteOffset);
    TempOffset  = WriteOffset+bytesWritten;
    if(TempOffset >= EndOffset)
    {
        TempOffset = BaseOffset + (TempOffset - EndOffset);
    }

    BDSP_WriteFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.WriteOffset, TempOffset);
    pRaagaQueue->hMsgQueue[0]->Address.WriteOffset = TempOffset;

	BDBG_LEAVE(BDSP_Raaga_P_Queue_CommitData);
	return errCode;
}

BERR_Code BDSP_Raaga_P_Queue_ConsumeData(
	void  *pQueueHandle,
	size_t readBytes
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
    dramaddr_t ReadOffset=0, EndOffset=0, BaseOffset=0, TempOffset=0;

	BDBG_ENTER(BDSP_Raaga_P_Queue_ConsumeData);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);

	if(pRaagaQueue->input)
	{
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

    BaseOffset = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.BaseOffset);
    EndOffset  = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.EndOffset);
    ReadOffset = BDSP_ReadFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.ReadOffset);
    TempOffset = ReadOffset+readBytes;
    if(TempOffset >= EndOffset)
    {
        TempOffset = BaseOffset + (TempOffset - EndOffset);
    }

    BDSP_WriteFIFOReg(pRaagaQueue->hMsgQueue[0]->hRegister, pRaagaQueue->hMsgQueue[0]->QueueAddress.ReadOffset, TempOffset);
    pRaagaQueue->hMsgQueue[0]->Address.ReadOffset = TempOffset;

	BDBG_LEAVE(BDSP_Raaga_P_Queue_ConsumeData);
	return errCode;
}

BERR_Code BDSP_Raaga_P_Queue_GetBufferAddr(
    void    *pQueueHandle,
    unsigned numbuffers,
    void    *pBuffer /*[out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaQueue *pRaagaQueue = (BDSP_RaagaQueue *)pQueueHandle;
	BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuffer = (BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *)pBuffer;
	unsigned i;

	BDBG_ENTER(BDSP_Raaga_P_Queue_GetBufferAddr);
	BDBG_OBJECT_ASSERT(pRaagaQueue, BDSP_RaagaQueue);
	BDBG_ASSERT(psCircBuffer);
	BDBG_ASSERT(numbuffers == pRaagaQueue->numChannels);
	for(i=0; i<numbuffers; i++)
	{
		psCircBuffer->ui32BaseAddr  = pRaagaQueue->hMsgQueue[i]->QueueAddress.BaseOffset;
		psCircBuffer->ui32EndAddr   = pRaagaQueue->hMsgQueue[i]->QueueAddress.EndOffset;
		psCircBuffer->ui32ReadAddr  = pRaagaQueue->hMsgQueue[i]->QueueAddress.ReadOffset;
		psCircBuffer->ui32WriteAddr = pRaagaQueue->hMsgQueue[i]->QueueAddress.WriteOffset;
		psCircBuffer->ui32WrapAddr  = pRaagaQueue->hMsgQueue[i]->QueueAddress.EndOffset;
		psCircBuffer++;
	}

	BDBG_LEAVE(BDSP_Raaga_P_Queue_GetBufferAddr);
	return errCode;
}
