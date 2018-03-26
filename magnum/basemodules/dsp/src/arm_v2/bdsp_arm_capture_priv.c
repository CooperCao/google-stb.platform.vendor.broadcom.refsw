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

BDBG_MODULE(bdsp_arm_capture);

static BERR_Code BDSP_Arm_P_InitAudioCapture(
    BDSP_ArmContext                     *pArmContext,
    BDSP_ArmCapture                     *pArmCapture,
    const BDSP_AudioCaptureCreateSettings *pCaptureCreateSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
    unsigned MemoryRequired = 0;
    BDSP_MMA_Memory Memory;
    unsigned index = 0;
    BDSP_P_BufferDescriptor *pDescriptor;

    BDBG_ENTER(BDSP_Arm_P_InitAudioCapture);
    pArmCapture->pArmContext = pArmContext;
	pArmCapture->enabled       = false;
	pArmCapture->maxBuffers    = pCaptureCreateSettings->maxChannels;

	/*Decision to use which HEAP either one provided by Application or Device Heap*/
	if (pCaptureCreateSettings->hHeap)
	{
		pArmCapture->hHeap = pCaptureCreateSettings->hHeap;
	}
	else
	{
		pArmCapture->hHeap = pArmContext->pDevice->memHandle;
	}
    MemoryRequired = pCaptureCreateSettings->maxChannels*pCaptureCreateSettings->channelBufferSize;
    errCode = BDSP_MMA_P_AllocateAlignedMemory(pArmCapture->hHeap,
                    MemoryRequired,
                    &Memory,
                    BDSP_MMA_Alignment_4KByte);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_InitAudioCapture: Unable to Allocate memory for Capture buffers!!!!"));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		return errCode;
	}
	BKNI_Memset(Memory.pAddr, 0, MemoryRequired);
	BDSP_MMA_P_FlushCache(Memory,MemoryRequired);

    pArmCapture->captureBuffer = Memory;
    BKNI_Memset((void *)pArmCapture->CapturePointerInfo, 0, (sizeof(BDSP_ArmCapturePointerInfo)*BDSP_AF_P_MAX_CHANNELS));
    for(index=0; index< pArmCapture->maxBuffers; index++)
    {
        pArmCapture->CapturePointerInfo[index].CaptureBufferMemory = Memory;
        pDescriptor = &pArmCapture->CapturePointerInfo[index].captureBufferPtr;
        pDescriptor->pBasePtr = Memory.pAddr;
        pDescriptor->pReadPtr = Memory.pAddr;
        pDescriptor->pWritePtr= Memory.pAddr;
        pDescriptor->pEndPtr  = (void *)((uint8_t *)Memory.pAddr+ pCaptureCreateSettings->channelBufferSize);
        pDescriptor->pWrapPtr = (void *)((uint8_t *)Memory.pAddr+ pCaptureCreateSettings->channelBufferSize);
        Memory.pAddr = (void *)((uint8_t *)Memory.pAddr+ pCaptureCreateSettings->channelBufferSize);
        Memory.offset = Memory.offset +  pCaptureCreateSettings->channelBufferSize;
    }

    BDBG_LEAVE(BDSP_Arm_P_InitAudioCapture);
    return errCode;
}

BERR_Code BDSP_Arm_P_AudioCaptureCreate(
	void                                  *pContextHandle,
	const BDSP_AudioCaptureCreateSettings *pCaptureCreateSettings,
	BDSP_AudioCaptureHandle               *pCaptureHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmContext *pArmContext = (BDSP_ArmContext *)pContextHandle;
	BDSP_ArmCapture *pArmCapture;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureCreate);
	BDBG_OBJECT_ASSERT(pArmContext, BDSP_ArmContext);

	/* Allocate capture structure and add it to the task's capture list */
	pArmCapture = (BDSP_ArmCapture *)BKNI_Malloc(sizeof(BDSP_ArmCapture));
	if(NULL == pArmCapture)
	{
		BDBG_ERR(("BDSP_Arm_P_AudioCaptureCreate: Unable to Allocate memory for Capture!!!!"));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto end;
	}

    /* Update the capture information using the capture create settings */
    BKNI_Memset((void *)pArmCapture, 0, sizeof(BDSP_ArmCapture));
	pArmCapture->capture.pCapHandle = pArmCapture;

	pArmCapture->capture.destroy = BDSP_Arm_P_AudioCaptureDestroy;
	pArmCapture->capture.addToStage = BDSP_Arm_P_AddAudioCaptureToStage;
	pArmCapture->capture.removeFromStage = BDSP_Arm_P_RemoveAudioCaptureFromStage;
	pArmCapture->capture.getBuffer = BDSP_Arm_P_AudioCaptureGetBuffer;
	pArmCapture->capture.consumeData = BDSP_Arm_P_AudioCaptureConsumeData;

    errCode = BDSP_Arm_P_InitAudioCapture(pArmContext, pArmCapture, pCaptureCreateSettings);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AudioCaptureCreate: Unable to create a capture for the Context %p", (void *)pArmContext));
        goto end;
    }
    BDBG_OBJECT_SET(pArmCapture, BDSP_ArmCapture);
	*pCaptureHandle = &pArmCapture->capture;

end:
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureCreate);
    return errCode;
}

void BDSP_Arm_P_AudioCaptureDestroy (
	void *pCaptureHandle
)
{
	BDSP_ArmCapture *pArmCapture = (BDSP_ArmCapture *)pCaptureHandle;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureDestroy);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);

	if(NULL != pArmCapture->pArmStage)
	{
		BDBG_ERR(("BDSP_Arm_P_AudioCaptureDestroy: Please call BDSP_AudioTask_RemoveCapture() before calling BDSP_AudioCapture_Destroy()"));
		BDBG_ASSERT(0);
	}

	BDSP_MMA_P_FreeMemory(&pArmCapture->captureBuffer);
	BKNI_Free(pArmCapture);

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureDestroy);
}

static BERR_Code BDSP_Arm_P_PopulateAudioCaptureInfo(
    BDSP_P_ConnectionDetails *pConnectionDetails,
    BDSP_ArmCapture        *pArmCapture,
    const BDSP_StageAudioCaptureSettings *pSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
    unsigned numbuffers = 0, i = 0;
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psOutputBuffer;

    BDBG_ENTER(BDSP_Arm_P_PopulateAudioCaptureInfo);
	if (pConnectionDetails->eValid != BDSP_AF_P_eValid)
	{
		BDBG_ERR(("BDSP_Arm_P_PopulateAudioCaptureInfo: Cannot add capture for an invalid output of the stage"));
		errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
	}

    switch(pConnectionDetails->eConnectionType)
    {
		case BDSP_ConnectionType_eFmmBuffer:
            {
                pArmCapture->eBuffType    = BDSP_AF_P_BufferType_eFMM;
                pArmCapture->StartCapture = false;/** Will only set to true once write pointer is >= start write pointer  **/
                numbuffers = pConnectionDetails->connectionHandle.fmm.fmmDescriptor.numBuffers;
                pArmCapture->numBuffers = numbuffers;
                for(i=0;i<numbuffers;i++)
                {
                    psOutputBuffer = (BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *)&pArmCapture->CapturePointerInfo[i].outputBufferPtr;
                    psOutputBuffer->ui32ReadAddr = (dramaddr_t)pConnectionDetails->connectionHandle.fmm.fmmDescriptor.buffers[i].read;
                    psOutputBuffer->ui32WriteAddr= (dramaddr_t)pConnectionDetails->connectionHandle.fmm.fmmDescriptor.buffers[i].write;
                    psOutputBuffer->ui32BaseAddr = (dramaddr_t)pConnectionDetails->connectionHandle.fmm.fmmDescriptor.buffers[i].base;
                    psOutputBuffer->ui32EndAddr  = (dramaddr_t)pConnectionDetails->connectionHandle.fmm.fmmDescriptor.buffers[i].end;
                    psOutputBuffer->ui32WrapAddr = (dramaddr_t)pConnectionDetails->connectionHandle.fmm.fmmDescriptor.buffers[i].end;
                    pArmCapture->CapturePointerInfo[i].ui32StartWriteAddr = psOutputBuffer->ui32EndAddr + (2*BDSP_SIZE_OF_FMMREG);
                    pArmCapture->CapturePointerInfo[i].shadowRead = BDSP_ReadFMMReg(pArmCapture->pArmContext->pDevice->regHandle,psOutputBuffer->ui32BaseAddr);
                    pArmCapture->CapturePointerInfo[i].lastWrite  = BDSP_ReadFMMReg(pArmCapture->pArmContext->pDevice->regHandle,psOutputBuffer->ui32WriteAddr);
                }
                for(i=0;i<pSettings->numChannelPair;i++)
                {
                    /* Since the allocation in APE was for pair of channels, BDSP deals with individual channels. Hence splitting the buffer*/
                    pArmCapture->CapturePointerInfo[2*i].OutputBufferMemory   = pSettings->channelPairInfo[i].outputBuffer;
                    pArmCapture->CapturePointerInfo[2*i+1].OutputBufferMemory = pSettings->channelPairInfo[i].outputBuffer;
                    pArmCapture->CapturePointerInfo[2*i+1].OutputBufferMemory.pAddr =
                        (void *)((uint8_t *)pArmCapture->CapturePointerInfo[2*i+1].OutputBufferMemory.pAddr + (pSettings->channelPairInfo[i].bufferSize/2));
                    pArmCapture->CapturePointerInfo[2*i+1].OutputBufferMemory.offset =
                        (pArmCapture->CapturePointerInfo[2*i+1].OutputBufferMemory.offset + (pSettings->channelPairInfo[i].bufferSize/2));
                }
            }
            break;
#if 0
        case BDSP_ConnectionType_eRDBBuffer:
            {
                BDSP_ArmQueue *pArmQueue = (BDSP_ArmQueue *)pConnectionDetails->connectionHandle.rdb.pQHandle->pQueueHandle;
                pArmCapture->eBuffType    = BDSP_AF_P_BufferType_eRDB;
                pArmCapture->StartCapture = true;
                numbuffers = pArmQueue->numChannels;
                pArmCapture->numBuffers = numbuffers;
                for(i=0;i<numbuffers;i++)
                {
                    psOutputBuffer = (BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *)&pArmCapture->CapturePointerInfo[i].outputBufferPtr;
                    psOutputBuffer->ui32ReadAddr = pArmQueue->hMsgQueue[i]->QueueAddress.ReadOffset;
                    psOutputBuffer->ui32WriteAddr= pArmQueue->hMsgQueue[i]->QueueAddress.WriteOffset;
                    psOutputBuffer->ui32BaseAddr = pArmQueue->hMsgQueue[i]->QueueAddress.BaseOffset;
                    psOutputBuffer->ui32EndAddr  = pArmQueue->hMsgQueue[i]->QueueAddress.EndOffset;
                    psOutputBuffer->ui32WrapAddr = pArmQueue->hMsgQueue[i]->QueueAddress.EndOffset;
                    pArmCapture->CapturePointerInfo[i].ui32StartWriteAddr = 0;
                    pArmCapture->CapturePointerInfo[i].shadowRead = BDSP_ReadFIFOReg(pArmCapture->pArmContext->pDevice->regHandle,psOutputBuffer->ui32BaseAddr);
                    pArmCapture->CapturePointerInfo[i].lastWrite  = BDSP_ReadFIFOReg(pArmCapture->pArmContext->pDevice->regHandle,psOutputBuffer->ui32WriteAddr);
                }
                /* Initialisation of the Buffer allocated by APE For Queue*/
                for(i=0;i<pArmCapture->numBuffers;i++)
                {
                    pArmCapture->CapturePointerInfo[i].OutputBufferMemory = pSettings->channelPairInfo[i].outputBuffer;
                }
            }
            break;
#endif
        default:
			BDBG_ERR(("BDSP_Arm_P_PopulateAudioCaptureInfo: Output Capture not supported for buffer type %d", pConnectionDetails->eConnectionType));
			errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_PopulateAudioCaptureInfo);
    return errCode;
}

BERR_Code BDSP_Arm_P_AddAudioCaptureToStage(
	void *pCaptureHandle,
	void *pStageHandle,
	unsigned outputId,
	const BDSP_StageAudioCaptureSettings *pSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmCapture *pArmCapture = (BDSP_ArmCapture *)pCaptureHandle;
    BDSP_ArmStage   *pArmStage   = (BDSP_ArmStage *)pStageHandle;
    unsigned i = 0;

    BDBG_ENTER(BDSP_Arm_P_AddAudioCaptureToStage);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);
	if(outputId >= BDSP_AF_P_MAX_OP_FORKS)
	{
	    BDBG_ERR(("BDSP_Arm_P_AddAudioCaptureToStage: Output ID provided (%d) exceeds the limit (%d)", outputId, (BDSP_AF_P_MAX_OP_FORKS-1)));
		errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto end;
	}

	/* Initialize the read/write pointers to base */
    for(i=0; i<BDSP_AF_P_MAX_CHANNELS; i++)
    {
		pArmCapture->CapturePointerInfo[i].captureBufferPtr.pReadPtr = pArmCapture->CapturePointerInfo[i].captureBufferPtr.pBasePtr;
		pArmCapture->CapturePointerInfo[i].captureBufferPtr.pWritePtr= pArmCapture->CapturePointerInfo[i].captureBufferPtr.pBasePtr;
	}

    errCode = BDSP_Arm_P_PopulateAudioCaptureInfo(&pArmStage->sStageConnectionInfo.sStageOutput[outputId], pArmCapture, pSettings);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AddAudioCaptureToStage: Couldn't derive the Output pointer for the Output Id (%d) for Stage %d (%s)",
            outputId, pArmStage->eAlgorithm, Algorithm2Name[pArmStage->eAlgorithm]));
        goto end;
    }

	/* Enable the capture */
    pArmCapture->pArmStage = pArmStage;
	pArmCapture->stageDestroyed = false;
    pArmCapture->updateRead  = pSettings->updateRead;
	pArmCapture->enabled     = true;

	BKNI_AcquireMutex(pArmCapture->pArmContext->pDevice->deviceMutex);
	BLST_S_INSERT_HEAD(&pArmStage->captureList, pArmCapture, node);
	BKNI_ReleaseMutex(pArmCapture->pArmContext->pDevice->deviceMutex);

end:
    BDBG_LEAVE(BDSP_Arm_P_AddAudioCaptureToStage);
    return errCode;
}

void BDSP_Arm_P_RemoveAudioCaptureFromStage(
	void *pCaptureHandle,
	void *pStageHandle
)
{
	BDSP_ArmCapture *pArmCapture = (BDSP_ArmCapture *)pCaptureHandle;

    BDBG_ENTER(BDSP_Arm_P_RemoveAudioCaptureFromStage);
	BSTD_UNUSED(pStageHandle);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);

	if(pArmCapture->stageDestroyed != true)
	{
		BKNI_AcquireMutex(pArmCapture->pArmContext->pDevice->deviceMutex);
		BLST_S_REMOVE(&pArmCapture->pArmStage->captureList, pArmCapture, BDSP_ArmCapture, node);
		BKNI_ReleaseMutex(pArmCapture->pArmContext->pDevice->deviceMutex);
	}

	pArmCapture->enabled     = false;
	pArmCapture->pArmStage = NULL;
    pArmCapture->numBuffers  = 0;
    pArmCapture->eBuffType   = BDSP_AF_P_BufferType_eInvalid;

    BDBG_LEAVE(BDSP_Arm_P_RemoveAudioCaptureFromStage);
}

BERR_Code BDSP_Arm_P_AudioCaptureGetBuffer(
	void                  *pCaptureHandle,   /* [in] capture handle */
	BDSP_BufferDescriptor *pBuffers     /* [out] pointer to buffer descriptor */
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmCapture *pArmCapture = (BDSP_ArmCapture *)pCaptureHandle;
    uint8_t *pReadPointer=NULL, *pWritePointer = NULL, *pEndPointer=NULL;
    unsigned chunk1 = 0,chunk2 = 0, i = 0;
	unsigned chunk1_lowest = 0;
    BDSP_MMA_Memory Memory;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureGetBuffer);
    BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);

    pBuffers->interleaved = false;
    pBuffers->numBuffers  = pArmCapture->numBuffers;
    if(pArmCapture->enabled)
    {
        for(i=0;i<pArmCapture->numBuffers;i++)
        {
            Memory = pArmCapture->CapturePointerInfo[i].CaptureBufferMemory;
            pReadPointer = (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pReadPtr;
            pWritePointer= (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pWritePtr;
            pEndPointer  = (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pEndPtr;
            if(pReadPointer > pWritePointer)
            {
                /* Loop Around scenario */
                chunk1 = pEndPointer   - pReadPointer;
                chunk2 = 0;/* Wrap Around condition is not handled */
                /*Wrap will always start from Base */
                pBuffers->buffers[i].wrapBuffer = Memory;
            }
            else
            {
                chunk1 = pWritePointer - pReadPointer;
                chunk2 = 0;
            }
            Memory.pAddr = (void *)pReadPointer;
            pBuffers->buffers[i].buffer = Memory;
			if(i == 0)
			{
				chunk1_lowest = chunk1;
			}
			chunk1_lowest = ((chunk1<chunk1_lowest)?chunk1:chunk1_lowest);
        }
		pBuffers->bufferSize     = chunk1_lowest;
		pBuffers->wrapBufferSize = chunk2;
    }
	else
	{
		pBuffers->bufferSize     = 0;
		pBuffers->wrapBufferSize = 0;
	}

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureGetBuffer);
    return errCode;
}

BERR_Code BDSP_Arm_P_AudioCaptureConsumeData(
	void    *pCaptureHandle, /* [in] capture handle */
	uint32_t numBytes       /* [in] sizes of data read from each intermediate buffer */
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmCapture *pArmCapture = (BDSP_ArmCapture *)pCaptureHandle;
    uint8_t *pReadPointer=NULL, *pBasePointer = NULL, *pEndPointer=NULL;
    unsigned i = 0;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureConsumeData);
	BDBG_OBJECT_ASSERT(pArmCapture, BDSP_ArmCapture);

    if(pArmCapture->enabled)
    {
        for(i=0; i<pArmCapture->numBuffers; i++)
        {
            pReadPointer = (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pReadPtr;
            pBasePointer = (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pBasePtr;
            pEndPointer  = (uint8_t *)pArmCapture->CapturePointerInfo[i].captureBufferPtr.pEndPtr;
            pReadPointer = (pReadPointer+numBytes);
            if(pReadPointer >= pEndPointer)
            {
                /* Handle Wrap Around */
                pArmCapture->CapturePointerInfo[i].captureBufferPtr.pReadPtr =(void *)(pBasePointer + (pEndPointer-pReadPointer));
            }
            else
            {
                pArmCapture->CapturePointerInfo[i].captureBufferPtr.pReadPtr = (void *)pReadPointer;
            }
        }
    }

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureConsumeData);
    return errCode;
}

static void BDSP_Arm_P_AudioCaptureDetectCaptureReady(
    BREG_Handle regHandle,
    dramaddr_t  StartWriteAddr,
    dramaddr_t  WriteAddr,
    bool       *pstartCapture
)
{
    dramaddr_t WritePtr = 0, StartWritePtr = 0;
    BDBG_ENTER(BDSP_Arm_P_AudioCaptureDetectCaptureReady);

    StartWritePtr = BDSP_ReadFMMReg(regHandle, StartWriteAddr);
    WritePtr      = BDSP_ReadFMMReg(regHandle, WriteAddr);
    if(WritePtr >= StartWritePtr)
    {
        *pstartCapture = true;
    }
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureDetectCaptureReady);
}

static void BDSP_Arm_P_AudioCaptureGetReadBufferDetails(
    BREG_Handle                      regHandle,
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
    BDSP_AF_P_BufferType             eBufferType,
    dramaddr_t                       shadowRead,
    dramaddr_t                      *pLastWrite,
    unsigned                        *pSize,
    BDSP_MMA_Memory                 *pReadMemory
)
{
    unsigned depth = 0;
    dramaddr_t BaseAddr, EndAddr, ReadAddr, WriteAddr;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureGetReadBufferDetails);
    switch(eBufferType)
    {
		case BDSP_AF_P_BufferType_eFMM:
			BaseAddr = BDSP_ReadFMMReg(regHandle, pBuffer->ui32BaseAddr);
			EndAddr  = BDSP_ReadFMMReg(regHandle, pBuffer->ui32EndAddr);
			ReadAddr = shadowRead;
			WriteAddr= BDSP_ReadFMMReg(regHandle, pBuffer->ui32WriteAddr);
            if((ReadAddr ^ WriteAddr) == BDSP_FMM_WRAP_MASK)
            {
                /* Buffer is full */
                depth = (EndAddr - BaseAddr) + 1;
            }
            else
            {
				ReadAddr  &= BDSP_FMM_ADDR_MASK; /* Get LSB 39/31 Bits based on Register Width */
				WriteAddr &= BDSP_FMM_ADDR_MASK; /* Get LSB 39/31 Bits based on Register Width*/
                if(ReadAddr > WriteAddr)
                {
                    depth = EndAddr - ReadAddr + 1;
                }
                else
                {
                    depth = WriteAddr - ReadAddr;
                }
            }
            break;
#if 0
        case BDSP_AF_P_BufferType_eRDB:
			BaseAddr = BDSP_ReadFIFOReg(regHandle, pBuffer->ui32BaseAddr);
			EndAddr  = BDSP_ReadFIFOReg(regHandle, pBuffer->ui32EndAddr);
			ReadAddr = shadowRead;
			WriteAddr= BDSP_ReadFIFOReg(regHandle, pBuffer->ui32WriteAddr);
            if(ReadAddr > WriteAddr)
            {
                depth = EndAddr - ReadAddr;
            }
            else
            {
                depth = WriteAddr - ReadAddr;
            }
            break;
#endif
        default:
			BDBG_ERR(("BDSP_Arm_P_AudioCaptureGetReadBufferDetails: Output Capture not supported for buffer type %d [%s]",
                eBufferType, BufferType[eBufferType]));
            BDBG_ASSERT(0);
            break;
    }

    *pSize = depth;
    pReadMemory->pAddr = (void *)((uint8_t *)pReadMemory->pAddr +((shadowRead&BDSP_FMM_ADDR_MASK)-BaseAddr));
    *pLastWrite = WriteAddr;
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureGetReadBufferDetails);
}

static bool BDSP_Arm_P_AudioCaptureDetectCaptureError(
    BREG_Handle                      regHandle,
    BDSP_AF_P_BufferType             eBufferType,
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
    dramaddr_t                       shadowRead,
    dramaddr_t                       lastWrite
)
{
    bool error = false;
    dramaddr_t WriteAddr;
    BDBG_ENTER(BDSP_Arm_P_AudioCaptureDetectCaptureError);

    switch(eBufferType)
    {
        case BDSP_AF_P_BufferType_eFMM:
            WriteAddr = BDSP_ReadFMMReg(regHandle, pBuffer->ui32WriteAddr);
            WriteAddr  = (WriteAddr & BDSP_FMM_ADDR_MASK); /* use only the LSB 39/31 bits */
            shadowRead = (shadowRead & BDSP_FMM_ADDR_MASK); /* use only the LSB 39/31 bits */
            break;
#if 0
        case BDSP_AF_P_BufferType_eRDB:
            WriteAddr = BDSP_ReadFIFOReg(regHandle, pBuffer->ui32WriteAddr);
            break;
#endif
        default:
            BDBG_ERR(("BDSP_Arm_P_AudioCaptureDetectCaptureError: Output Capture not supported for buffer type %d [%s]",
                eBufferType, BufferType[eBufferType]));
            BDBG_ASSERT(0);
            break;
    }

    /* It is not possible to detect a capture error if shadow read and last write are the same */
    if (shadowRead == lastWrite)
    {
        error = false;
    }
    else
    {
        if (shadowRead>lastWrite)
        {
            /* Write should be between shadow and last write */
            if((WriteAddr>shadowRead)||(WriteAddr<lastWrite))
            {
                error = true;
            }
        }
        else
        {
            /* Write should not be between shadow and last write */
            if((WriteAddr>shadowRead)&&(WriteAddr<lastWrite))
            {
                error = true;
            }
        }
    }

    if(error)
    {
        BDBG_ERR(("shadow = "BDSP_MSG_FMT": write = " BDSP_MSG_FMT ": last wr = "BDSP_MSG_FMT, BDSP_MSG_ARG(shadowRead), BDSP_MSG_ARG(WriteAddr), BDSP_MSG_ARG(lastWrite)));
    }

    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureDetectCaptureError);
    return error;
}

static void BDSP_Arm_P_AudioCaptureGetWriteBufferDetails(
    BDSP_P_BufferDescriptor *pCaptureBufferPtr,
    unsigned                *pSize,
    BDSP_MMA_Memory         *pWriteMemory
)
{
    uint8_t *pWritePtr, *pReadPtr, *pEndPtr;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureGetWriteBufferDetails);
    pEndPtr  = (uint8_t *)pCaptureBufferPtr->pEndPtr;
    pReadPtr = (uint8_t *)pCaptureBufferPtr->pReadPtr;
    pWritePtr= (uint8_t *)pCaptureBufferPtr->pWritePtr;

    *pSize = 0;
    if(pWritePtr >= pReadPtr)
    {
        /* Wrap Around wont be handled, hence write till End */
        *pSize = pEndPtr-pWritePtr;
    }
    else
    {
        *pSize = pReadPtr-pWritePtr;
    }
    pWriteMemory->pAddr = pCaptureBufferPtr->pWritePtr;
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureGetWriteBufferDetails);
}

static void BDSP_Arm_P_AudioCaptureUpdateWriteBufferDetails(
    BDSP_P_BufferDescriptor *pCaptureBufferPtr,
    unsigned                 bytescopied
)
{
    uint8_t *pWritePtr, *pBasePtr, *pEndPtr;
    BDBG_ENTER(BDSP_Arm_P_AudioCaptureUpdateWriteBufferDetails);
    pBasePtr = (uint8_t *)pCaptureBufferPtr->pBasePtr;
    pEndPtr  = (uint8_t *)pCaptureBufferPtr->pEndPtr;
    pWritePtr= (uint8_t *)pCaptureBufferPtr->pWritePtr;
    pWritePtr += bytescopied;

    if(pWritePtr >= pEndPtr)
    {
        pCaptureBufferPtr->pWritePtr = (void *)(pBasePtr + (pWritePtr - pEndPtr));
    }
    else
    {
        pCaptureBufferPtr->pWritePtr = (void *)(pWritePtr);
    }
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureUpdateWriteBufferDetails);
}

static void BDSP_Arm_P_AudioCaptureUpdateReadPointer(
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
    BDSP_AF_P_BufferType             eBufferType,
    dramaddr_t                       ui32ReadAddr,
    BREG_Handle                      hReg
)
{
    BDBG_ENTER(BDSP_Arm_P_AudioCaptureUpdateReadPointer);
    switch (eBufferType)
    {
        case BDSP_AF_P_BufferType_eRAVE:
        case BDSP_AF_P_BufferType_eFMM:
            BDSP_WriteFMMReg(hReg, pBuffer->ui32ReadAddr, ui32ReadAddr);
            break;
#if 0
        case BDSP_AF_P_BufferType_eRDB:
            BDSP_WriteFIFOReg(hReg, pBuffer->ui32ReadAddr, ui32ReadAddr);
            break;
#endif
        case BDSP_AF_P_BufferType_eDRAM:
        default:
            pBuffer->ui32ReadAddr = ui32ReadAddr;
            break;
    }
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureUpdateReadPointer);
}

static void BDSP_Arm_P_AudioCaptureUpdateShadowRead(
    BREG_Handle                      regHandle,
    BDSP_AF_P_BufferType             eBufferType,
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *pBuffer,
    dramaddr_t                      *pShadowRead,
    unsigned                         bytesRead
)
{
    dramaddr_t shadowReadAddr = 0;
    dramaddr_t BaseAddr, EndAddr;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureUpdateShadowRead);
    shadowReadAddr = *pShadowRead + bytesRead;
    switch(eBufferType)
    {
        case BDSP_AF_P_BufferType_eFMM:
            BaseAddr = BDSP_ReadFMMReg(regHandle, pBuffer->ui32BaseAddr);
            EndAddr  = BDSP_ReadFMMReg(regHandle, pBuffer->ui32EndAddr);
            if((shadowReadAddr & BDSP_FMM_ADDR_MASK) > EndAddr)
            {
                shadowReadAddr = BaseAddr+((shadowReadAddr-EndAddr)-1);
				/* Flip bit 40/32 on a wrap based on register size*/
				shadowReadAddr ^= BDSP_FMM_WRAP_MASK;
            }
            break;
#if 0
        case BDSP_AF_P_BufferType_eRDB:
            BaseAddr = BDSP_ReadFIFOReg(regHandle, pBuffer->ui32BaseAddr);
            EndAddr  = BDSP_ReadFIFOReg(regHandle, pBuffer->ui32EndAddr);
			if (shadowReadAddr >= EndAddr)
			{
				shadowReadAddr = BaseAddr + (shadowReadAddr-EndAddr);
			}
            break;
#endif
        default:
            BDBG_ERR(("BDSP_Arm_P_AudioCaptureUpdateShadowRead: Output Capture not supported for buffer type %d [%s]",
                eBufferType, BufferType[eBufferType]));
            BDBG_ASSERT(0);
            break;
    }
    *pShadowRead = shadowReadAddr;
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureUpdateShadowRead);
}

static BERR_Code BDSP_Arm_P_AudioCaptureProcessing(
    BDSP_ArmCapture *pArmCapture
)
{
	BERR_Code errCode = BERR_SUCCESS;
    bool err_status = false;
    unsigned WriteSize = 0, ReadSize = 0, bytesToCopy = 0;
    BDSP_MMA_Memory ReadMemory, WriteMemory;
    unsigned i = 0;

    BDBG_ENTER(BDSP_Arm_P_AudioCaptureProcessing);
    BKNI_AcquireMutex(pArmCapture->pArmContext->pDevice->deviceMutex);
    if(pArmCapture->eBuffType == BDSP_AF_P_BufferType_eFMM && pArmCapture->StartCapture == false)
    {
        /* Wait for the first channel (L) to have its start write pointer moved as pointers are moved in reverse order*/
        BDSP_Arm_P_AudioCaptureDetectCaptureReady(
            pArmCapture->pArmContext->pDevice->regHandle,
            pArmCapture->CapturePointerInfo[0].ui32StartWriteAddr,
            pArmCapture->CapturePointerInfo[0].outputBufferPtr.ui32WriteAddr,
            &pArmCapture->StartCapture);
		if(true == pArmCapture->StartCapture)
		{
			BDBG_MSG(("BDSP_Arm_P_AudioCaptureProcessing: Initilaise the Shadow Read and Last write to Start Write"));
			for (i = 0; i < pArmCapture->numBuffers; i++)
			{
				 pArmCapture->CapturePointerInfo[i].shadowRead = BDSP_ReadFMMReg(
					 pArmCapture->pArmContext->pDevice->regHandle,
					 pArmCapture->CapturePointerInfo[i].ui32StartWriteAddr);
				 pArmCapture->CapturePointerInfo[i].lastWrite = pArmCapture->CapturePointerInfo[i].shadowRead;
			}
		}
    }

    if(true == pArmCapture->StartCapture)
    {
        for(i=0; i<pArmCapture->numBuffers; i++)
        {
            BDBG_MSG(("CAPTURE PASS for Channel %d", i));
            /* Detect Capture Error*/
            err_status = BDSP_Arm_P_AudioCaptureDetectCaptureError(
                pArmCapture->pArmContext->pDevice->regHandle,
                pArmCapture->eBuffType,
                &pArmCapture->CapturePointerInfo[i].outputBufferPtr,
                pArmCapture->CapturePointerInfo[i].shadowRead,
                pArmCapture->CapturePointerInfo[i].lastWrite);
            if(err_status == true)
            {
                BDBG_ERR(("!!! BDSP_Arm_P_AudioCaptureProcessing: Capture error detected for buffer [%d] in capture %p !!!!!", i, (void *)pArmCapture));
                continue;
            }

            /* Get Depth and Read Pointer of Read Buffer and update the last write*/
            ReadMemory  = pArmCapture->CapturePointerInfo[i].OutputBufferMemory;
            BDSP_Arm_P_AudioCaptureGetReadBufferDetails(
                pArmCapture->pArmContext->pDevice->regHandle,
                &pArmCapture->CapturePointerInfo[i].outputBufferPtr,
                pArmCapture->eBuffType,
                pArmCapture->CapturePointerInfo[i].shadowRead,
                &pArmCapture->CapturePointerInfo[i].lastWrite,
                &ReadSize,
                &ReadMemory);

            /* Get Depth and Write Pointer of Write Buffer */
            WriteMemory = pArmCapture->CapturePointerInfo[i].CaptureBufferMemory;
            BDSP_Arm_P_AudioCaptureGetWriteBufferDetails(
                &pArmCapture->CapturePointerInfo[i].captureBufferPtr,
                &WriteSize,
                &WriteMemory);

            /* Copy the minimum */
            bytesToCopy = (WriteSize<ReadSize)?WriteSize:ReadSize;

            /* Copy Data */
            BDSP_MMA_P_FlushCache(ReadMemory, bytesToCopy);
            BKNI_Memcpy(WriteMemory.pAddr, ReadMemory.pAddr, bytesToCopy);
            BDSP_MMA_P_FlushCache(WriteMemory, bytesToCopy);

            /* Update Write buffer Pointers */
            BDSP_Arm_P_AudioCaptureUpdateWriteBufferDetails(&pArmCapture->CapturePointerInfo[i].captureBufferPtr, bytesToCopy);

            /* Update ShadowRead */
            BDSP_Arm_P_AudioCaptureUpdateShadowRead(
                pArmCapture->pArmContext->pDevice->regHandle,
                pArmCapture->eBuffType,
                &pArmCapture->CapturePointerInfo[i].outputBufferPtr,
                &pArmCapture->CapturePointerInfo[i].shadowRead,
                bytesToCopy);

            /* Update ReadPointer if set to Applcation*/
            if (pArmCapture->updateRead)
            {
                BDSP_Arm_P_AudioCaptureUpdateReadPointer(
                    &pArmCapture->CapturePointerInfo[i].outputBufferPtr,
                    pArmCapture->eBuffType,
                    pArmCapture->CapturePointerInfo[i].shadowRead,
                    pArmCapture->pArmContext->pDevice->regHandle);
            }
        }
    }
	BKNI_ReleaseMutex(pArmCapture->pArmContext->pDevice->deviceMutex);
    BDBG_LEAVE(BDSP_Arm_P_AudioCaptureProcessing);
    return errCode;
}

BERR_Code BDSP_Arm_P_ProcessAudioCapture(
	void *pDevice /* [in] device handle */
)
{
	BERR_Code errCode = BERR_SUCCESS;
    BDSP_Arm *pArmDevice = (BDSP_Arm *)pDevice;
    BDSP_ArmContext *pArmContext = NULL;
    BDSP_ArmTask    *pArmTask    = NULL;
	BDSP_ArmCapture *pArmCapture = NULL;

    BDBG_ENTER(BDSP_Arm_P_ProcessAudioCapture);
	BDBG_OBJECT_ASSERT(pArmDevice, BDSP_Arm);

    /* Get all enabled Captures for all Stages of all tasks of all contexts to derive the captures for processing*/
	for(pArmContext = BLST_S_FIRST(&pArmDevice->contextList);
		pArmContext != NULL;
		pArmContext = BLST_S_NEXT(pArmContext, node))
	{
        for(pArmTask = BLST_S_FIRST(&pArmContext->taskList);
            pArmTask != NULL;
            pArmTask = BLST_S_NEXT(pArmTask, node))
        {
            if (pArmTask->taskParams.isRunning)
            {
				BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(pArmTask->startSettings.primaryStage->pStageHandle, pStageIterator)
				BSTD_UNUSED(macroStId);
				BSTD_UNUSED(macroBrId);
				{
                    for(pArmCapture = BLST_S_FIRST(&pStageIterator->captureList);
                        pArmCapture != NULL;
                        pArmCapture = BLST_S_NEXT(pArmCapture, node))
                    {
                        if(pArmCapture->enabled)
                        {
                            errCode = BDSP_Arm_P_AudioCaptureProcessing(pArmCapture);
                            if(errCode != BERR_SUCCESS)
                            {
                                BDBG_ERR(("BDSP_Arm_P_ProcessAudioCapture: Error in Capture for the Stage %d [%s], but continuing to next capture",
                                    pStageIterator->eAlgorithm, Algorithm2Name[pStageIterator->eAlgorithm]));
                            }
                        }
                    }
                }
				BDSP_ARM_STAGE_TRAVERSE_LOOP_END(pStageIterator)
            }
        }
    }
    BDBG_LEAVE(BDSP_Arm_P_ProcessAudioCapture);
    return errCode;
}
