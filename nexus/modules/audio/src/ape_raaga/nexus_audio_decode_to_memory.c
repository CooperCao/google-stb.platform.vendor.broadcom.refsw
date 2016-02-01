/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* API Description:
*   Interfaces for returning decoded audio frames to the host
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_decode_to_memory);

static void NEXUS_AudioDecoder_P_FreeDecodeToMemoryNodes(
    NEXUS_AudioDecoderHandle decoder
    );

static void NEXUS_AudioDecoder_P_BufferComplete_isr(
    void *pParam1,
    int param2
    );

NEXUS_Error NEXUS_AudioDecoder_P_InitDecodeToMemory(
    NEXUS_AudioDecoderHandle decoder
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoderDecodeToMemorySettings settings;
    BAPE_DecoderInterruptHandlers interrupts;

    decoder->bufferCompleteCallback = NEXUS_IsrCallback_Create(decoder, NULL);
    if ( NULL == decoder->bufferCompleteCallback )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    NEXUS_CallbackDesc_Init(&decoder->bufferCompleteCallbackDesc);

    NEXUS_AudioDecoder_GetDecodeToMemorySettings(decoder, &settings);
    errCode = NEXUS_AudioDecoder_SetDecodeToMemorySettings(decoder, &settings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_Decoder_GetInterruptHandlers(decoder->channel, &interrupts);
    interrupts.hostBufferReady.pCallback_isr = NEXUS_AudioDecoder_P_BufferComplete_isr;
    interrupts.hostBufferReady.pParam1 = decoder;
    errCode = BAPE_Decoder_SetInterruptHandlers(decoder->channel, &interrupts);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_AudioDecoder_P_UninitDecodeToMemory(
    NEXUS_AudioDecoderHandle decoder
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);

    NEXUS_AudioDecoder_P_FreeDecodeToMemoryNodes(decoder);

    if ( decoder->bufferCompleteCallback )
    {
        NEXUS_IsrCallback_Destroy(decoder->bufferCompleteCallback);
        decoder->bufferCompleteCallback = NULL;
    }
}

void NEXUS_AudioDecoder_GetDecodeToMemorySettings(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderDecodeToMemorySettings *pSettings /* [out] */
    )
{
    BAPE_DecoderDecodeToMemorySettings apeSettings;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Decoder_GetDecodeToMemorySettings(decoder->channel, &apeSettings);
    pSettings->maxBuffers = apeSettings.maxBuffers;
    pSettings->maxSampleRate = apeSettings.maxSampleRate;
    pSettings->bitsPerSample = apeSettings.bitsPerSample;
    pSettings->numPcmChannels = apeSettings.numPcmChannels;
    for ( i = 0; i < (unsigned)NEXUS_AudioChannel_eMax; i++ )
    {
        pSettings->channelLayout[i] = (NEXUS_AudioChannel)apeSettings.channelLayout[i];
    }
    pSettings->bufferComplete = decoder->bufferCompleteCallbackDesc;
}

NEXUS_Error NEXUS_AudioDecoder_SetDecodeToMemorySettings(
    NEXUS_AudioDecoderHandle decoder,
    const NEXUS_AudioDecoderDecodeToMemorySettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned i;
    NEXUS_AudioDecoderBufferNode *pNode;
    BAPE_DecoderDecodeToMemorySettings apeSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    BDBG_ASSERT(NULL != pSettings);

    if ( decoder->started )
    {
        BDBG_ERR(("DecodeToMemory settings can only be modified while stopped."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( 0 == pSettings->maxBuffers )
    {
        BDBG_ERR(("Invalid number of buffers specified"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BAPE_Decoder_GetDecodeToMemorySettings(decoder->channel, &apeSettings);

    if ( NULL == decoder->pBufferNodes || pSettings->maxBuffers != apeSettings.maxBuffers )
    {
        NEXUS_AudioDecoder_P_FreeDecodeToMemoryNodes(decoder);
        decoder->pBufferDescriptors = BKNI_Malloc(sizeof(BAPE_DecoderBufferDescriptor)*pSettings->maxBuffers);
        if ( NULL == decoder->pBufferDescriptors )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        decoder->pBufferNodes = BKNI_Malloc(sizeof(NEXUS_AudioDecoderBufferNode)*pSettings->maxBuffers);
        if ( NULL == decoder->pBufferNodes )
        {
            NEXUS_AudioDecoder_P_FreeDecodeToMemoryNodes(decoder);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_Memset(decoder->pBufferNodes, 0, sizeof(NEXUS_AudioDecoderBufferNode)*pSettings->maxBuffers);
        for ( i = 0; i < pSettings->maxBuffers; i++ )
        {
            pNode = decoder->pBufferNodes + i;
            BLST_Q_INSERT_TAIL(&decoder->freeBuffers, pNode, node);
        }
    }
    apeSettings.maxBuffers = pSettings->maxBuffers;
    apeSettings.maxSampleRate = pSettings->maxSampleRate;
    apeSettings.bitsPerSample = pSettings->bitsPerSample;
    apeSettings.numPcmChannels = pSettings->numPcmChannels;
    for ( i = 0; i < (unsigned)NEXUS_AudioChannel_eMax; i++ )
    {
        apeSettings.channelLayout[i] = (BAPE_Channel)pSettings->channelLayout[i];
    }
    errCode = BAPE_Decoder_SetDecodeToMemorySettings(decoder->channel, &apeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    decoder->maxDecodeBuffers = pSettings->maxBuffers;
    NEXUS_IsrCallback_Set(decoder->bufferCompleteCallback, &pSettings->bufferComplete);
    decoder->bufferCompleteCallbackDesc = pSettings->bufferComplete;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_GetDecodeToMemoryStatus(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderDecodeToMemoryStatus *pStatus /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_DecoderDecodeToMemoryStatus apeStatus;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    BDBG_ASSERT(NULL != pStatus);

    errCode = BAPE_Decoder_GetDecodeToMemoryStatus(decoder->channel, &apeStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    pStatus->bufferSize = apeStatus.bufferSize;
    pStatus->pendingBuffers = apeStatus.pendingBuffers;
    pStatus->completedBuffers = apeStatus.completedBuffers;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_QueueBuffer(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_MemoryBlockHandle buffer,
    unsigned bufferOffset,
    size_t bufferLength
    )
{
    NEXUS_AudioDecoderBufferNode *pNode;
    BAPE_DecoderBufferDescriptor bufferDesc;
    BERR_Code errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);

    if ( bufferLength == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pNode = BLST_Q_FIRST(&decoder->freeBuffers);
    if ( NULL == pNode )
    {
        /* BERR_TRACE omitted intentionally */
        return BERR_NOT_AVAILABLE;
    }
    pNode->block = buffer;
    pNode->blockOffset = bufferOffset;
    errCode = NEXUS_MemoryBlock_LockOffset(buffer, &pNode->memOffset);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_Decoder_InitBufferDescriptor(&bufferDesc);
    bufferDesc.allocatedBytes = bufferLength;
    bufferDesc.filledBytes = 0;
    bufferDesc.memoryOffset = bufferOffset + pNode->memOffset;
    errCode = BAPE_Decoder_QueueBuffer(decoder->channel, &bufferDesc);
    if ( errCode )
    {
        NEXUS_MemoryBlock_UnlockOffset(buffer);
        return BERR_TRACE(errCode);
    }

    BLST_Q_REMOVE_HEAD(&decoder->freeBuffers, node);
    BLST_Q_INSERT_TAIL(&decoder->activeBuffers, pNode, node);

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_GetDecodedFrames(
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_MemoryBlockHandle *pMemoryBlocks,         /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    NEXUS_AudioDecoderFrameStatus *pFrameStatus,    /* attr{nelem=maxFrames;nelem_out=pNumFrames} [out] */
    unsigned maxFrames,
    unsigned *pNumFrames                            /* [out] */
    )
{
    NEXUS_AudioDecoderBufferNode *pNode;
    BERR_Code errCode;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    BDBG_ASSERT(NULL != pMemoryBlocks);
    BDBG_ASSERT(NULL != pFrameStatus);
    BDBG_ASSERT(NULL != pNumFrames);

    *pNumFrames = 0;

    if ( maxFrames == 0 )
    {
        return BERR_SUCCESS;
    }

    if ( maxFrames > decoder->maxDecodeBuffers )
    {
        maxFrames = decoder->maxDecodeBuffers;
    }
    errCode = BAPE_Decoder_GetBuffers(decoder->channel, decoder->pBufferDescriptors, maxFrames, pNumFrames);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    maxFrames = *pNumFrames;
    for ( i = 0; i < maxFrames; i++ )
    {
        for ( pNode = BLST_Q_FIRST(&decoder->activeBuffers);
              NULL != pNode;
              pNode = BLST_Q_NEXT(pNode, node) )
        {
            if ( pNode->blockOffset + pNode->memOffset == decoder->pBufferDescriptors[i].memoryOffset )
            {
                pMemoryBlocks[i] = pNode->block;
                pFrameStatus[i].pts = decoder->pBufferDescriptors[i].ptsInfo.ui32CurrentPTS;
                pFrameStatus[i].ptsType = decoder->pBufferDescriptors[i].ptsInfo.ePTSType == BAVC_PTSType_eCoded ? NEXUS_PtsType_eCoded :
                    decoder->pBufferDescriptors[i].ptsInfo.ePTSType == BAVC_PTSType_eInterpolatedFromValidPTS ? NEXUS_PtsType_eInterpolatedFromValidPTS : NEXUS_PtsType_eInterpolatedFromInvalidPTS;
                pFrameStatus[i].filledBytes = decoder->pBufferDescriptors[i].filledBytes;
                pFrameStatus[i].sampleRate = decoder->pBufferDescriptors[i].sampleRate;
                pFrameStatus[i].bufferOffset = pNode->blockOffset;
                break;
            }
        }
        /* We had better find the buffer or something is horribly wrong */
        BDBG_ASSERT(NULL != pNode);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioDecoder_ConsumeDecodedFrames(
    NEXUS_AudioDecoderHandle decoder,
    size_t numBuffers
    )
{
    NEXUS_AudioDecoderBufferNode *pNode;
    BERR_Code errCode;
    unsigned totalBuffers;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    if ( numBuffers == 0 )
    {
        return BERR_SUCCESS;
    }
    if ( numBuffers > decoder->maxDecodeBuffers )
    {
        BDBG_ERR(("Attempt to return %lu buffers but decoder is configured for max %u", numBuffers, decoder->maxDecodeBuffers));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BAPE_Decoder_GetBuffers(decoder->channel, decoder->pBufferDescriptors, numBuffers, &totalBuffers);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if ( numBuffers > totalBuffers )
    {
        BDBG_ERR(("Attempt to return %lu buffers but only %u are outstanding", numBuffers, totalBuffers));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BAPE_Decoder_ConsumeBuffers(decoder->channel, totalBuffers);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    for ( i = 0; i < totalBuffers; i++ )
    {
        for ( pNode = BLST_Q_FIRST(&decoder->activeBuffers);
              NULL != pNode;
              pNode = BLST_Q_NEXT(pNode, node) )
        {
            if ( pNode->blockOffset + pNode->memOffset == decoder->pBufferDescriptors[i].memoryOffset )
            {
                NEXUS_MemoryBlock_UnlockOffset(pNode->block);
                BLST_Q_REMOVE(&decoder->activeBuffers, pNode, node);
                BLST_Q_INSERT_TAIL(&decoder->freeBuffers, pNode, node);
                break;
            }
        }
        /* We had better find the buffer or something is horribly wrong */
        BDBG_ASSERT(NULL != pNode);
    }

    return BERR_SUCCESS;
}

static void NEXUS_AudioDecoder_P_FreeDecodeToMemoryNodes(
    NEXUS_AudioDecoderHandle decoder
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);

    if ( decoder->pBufferDescriptors )
    {
        BKNI_Free(decoder->pBufferDescriptors);
        decoder->pBufferDescriptors = NULL;
    }
    if ( decoder->pBufferNodes )
    {
        BKNI_Free(decoder->pBufferNodes);
        decoder->pBufferNodes = NULL;
        BLST_Q_INIT(&decoder->freeBuffers);
        BLST_Q_INIT(&decoder->activeBuffers);
    }
    decoder->maxDecodeBuffers = 0;
}

static void NEXUS_AudioDecoder_P_BufferComplete_isr(
    void *pParam1,
    int param2
    )
{
    NEXUS_AudioDecoderHandle decoder = (NEXUS_AudioDecoderHandle)pParam1;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoder, decoder);
    BSTD_UNUSED(param2);

    NEXUS_IsrCallback_Fire_isr(decoder->bufferCompleteCallback);
}
