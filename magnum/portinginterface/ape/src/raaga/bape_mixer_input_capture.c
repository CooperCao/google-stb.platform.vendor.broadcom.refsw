/***************************************************************************
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
 *
 * Module Description: Audio Mixer Interface
 *
 ***************************************************************************/
 
#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_mixer_input_capture);
 
BDBG_OBJECT_ID(BAPE_MixerInputCapture);

#if BAPE_CHIP_MAX_DSP_MIXERS
void BAPE_MixerInputCapture_GetDefaultCreateSettings(
    BAPE_MixerInputCaptureCreateSettings *pSettings  /* [out] */
    )
{
	BDSP_AudioCaptureCreateSettings dspSettings;
	
	BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

	BDSP_AudioCapture_GetDefaultCreateSettings(&dspSettings);

    pSettings->maxChannels = dspSettings.maxChannels;
    pSettings->channelBufferSize = dspSettings.channelBufferSize;
    pSettings->hHeap = dspSettings.hHeap;
}

BERR_Code BAPE_MixerInputCapture_Create(
    BAPE_Handle hApe,
    const BAPE_MixerInputCaptureCreateSettings *pSettings,
    BAPE_MixerInputCaptureHandle *pMixerInputCaptureHandle   /* [out] */
    )
{
	BAPE_MixerInputCapture  *pMxrInputCapture;
	BDSP_AudioCaptureCreateSettings dspSettings;
    BDSP_ContextHandle dspContext = NULL;
	BERR_Code errCode;
	
    BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(NULL != pMixerInputCaptureHandle);

	/* Malloc handle structure, set object id */
	pMxrInputCapture = (BAPE_MixerInputCapture *)BKNI_Malloc(sizeof(BAPE_MixerInputCapture));
    if ( NULL == pMxrInputCapture )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_cleanup;
    }
    
    /* Update the capture information using the capture create settings */
    BKNI_Memset(pMxrInputCapture, 0, sizeof(*pMxrInputCapture));
	BDBG_OBJECT_SET(pMxrInputCapture, BAPE_MixerInputCapture);
    pMxrInputCapture->input = pSettings->input;

	BDSP_AudioCapture_GetDefaultCreateSettings(&dspSettings);
	
	dspSettings.maxChannels = pSettings->maxChannels;
    dspSettings.channelBufferSize = pSettings->channelBufferSize;
	dspSettings.hHeap = pSettings->hHeap;

    if ( hApe->dspContext )
    {
        dspContext = hApe->dspContext;
    }
    else if ( hApe->armContext )
    {
        dspContext = hApe->armContext;
    }

    if ( !dspContext )
    {
        BDBG_ERR(("No available dsp device"));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_cleanup;
    }

    pMxrInputCapture->hDeviceContext = dspContext;

    errCode = BDSP_AudioCapture_Create(pMxrInputCapture->hDeviceContext, &dspSettings, &(pMxrInputCapture->hCapture));
    if ( errCode ) { BERR_TRACE(errCode); goto err_cleanup; }

    pMxrInputCapture->dspSettings = dspSettings;

    /* set our input if we received one */
    pMxrInputCapture->input = pSettings->input;

    *pMixerInputCaptureHandle = pMxrInputCapture;

    return BERR_SUCCESS;
	
err_cleanup:
    if ( pMxrInputCapture )
    {
        BAPE_MixerInputCapture_Destroy(pMxrInputCapture);
        pMxrInputCapture = NULL;
    }
	return errCode;
}

void BAPE_MixerInputCapture_Destroy(
    BAPE_MixerInputCaptureHandle handle
    )
{
	BDBG_OBJECT_ASSERT(handle, BAPE_MixerInputCapture);
    if ( handle->hCapture )
    {
        BDSP_AudioCapture_Destroy(handle->hCapture);
        handle->hCapture = NULL;
    }

	BDBG_OBJECT_DESTROY(handle, BAPE_MixerInputCapture);
	BKNI_Free(handle);
}

BERR_Code BAPE_MixerInputCapture_Start(
    BAPE_MixerInputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MixerInputCapture);
    BDBG_MSG(("Configuring BDSP capture for input %p", (void*)handle->input));
    if ( handle->input )
    {
        unsigned i, dspOutputIndex;
        BERR_Code errCode;
        BDSP_StageAudioCaptureSettings capSettings;
        BAPE_PathConnection *pOutputConnection;
        BDSP_ContextHandle pDeviceContext;

        BDBG_OBJECT_ASSERT(handle->input, BAPE_PathConnector);

        if ( handle->input->hStage == NULL )
        {
            BDBG_ERR(("Connector must be associated with a DSP stage"));
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }

        pOutputConnection = BLST_SQ_FIRST(&handle->input->connectionList);
        if ( pOutputConnection == NULL )
        {
            BDBG_ERR(("Must configure and start the path before starting capture"));
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }

        errCode = BDSP_Stage_GetContext(handle->input->hStage, &pDeviceContext);
        if ( errCode ) { return BERR_TRACE(errCode); }

        if ( !pDeviceContext )
        {
            BDBG_ERR(("Invalid device context returned by BDSP"));
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }

        if ( pDeviceContext != handle->hDeviceContext )
        {
            BDSP_AudioCapture_Destroy(handle->hCapture);
            handle->hCapture = NULL;

            handle->hDeviceContext = pDeviceContext;

            errCode = BDSP_AudioCapture_Create(handle->hDeviceContext, &handle->dspSettings, &(handle->hCapture));
            if ( errCode ) { return BERR_TRACE(errCode); }
        }

        BDSP_Stage_GetDefaultAudioCaptureSettings(&capSettings);
        capSettings.numChannelPair = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->input->format);
        BDBG_MSG(("Configuring BDSP capture numChPairs %d", capSettings.numChannelPair));
        for ( i = 0; i < capSettings.numChannelPair; i++ )
        {
            if ( handle->input->pBuffers[i]->pMemory == NULL ||
                 handle->input->pBuffers[i]->block == NULL )
            {
                BDBG_ERR(("No Buffers associated with this connector!!"));
                return BERR_TRACE(BERR_NOT_AVAILABLE);
            }
            capSettings.channelPairInfo[i].outputBuffer.hBlock = handle->input->pBuffers[i]->block;
            capSettings.channelPairInfo[i].outputBuffer.pAddr = handle->input->pBuffers[i]->pMemory;
            capSettings.channelPairInfo[i].outputBuffer.offset = handle->input->pBuffers[i]->offset;
            capSettings.channelPairInfo[i].bufferSize = handle->input->pBuffers[i]->bufferSize;
            BDBG_MSG(("setting BDSP stage capture ch pair %d to hBlock %p, pAddr %p, offset " BDBG_UINT64_FMT, i, (void*)capSettings.channelPairInfo[i].outputBuffer.hBlock, (void*)capSettings.channelPairInfo[i].outputBuffer.pAddr, BDBG_UINT64_ARG(capSettings.channelPairInfo[i].outputBuffer.offset)));
        }

        dspOutputIndex = pOutputConnection->dspOutputIndex;
        errCode = BDSP_Stage_AddAudioCapture(handle->input->hStage, handle->hCapture, dspOutputIndex, &capSettings);
        if ( errCode )
        {
            BDBG_ERR(("Unable to add BDSP audio capture stage"));
            return BERR_TRACE(errCode);
        }

        handle->hStage = handle->input->hStage;
        return BERR_SUCCESS;
    }

    return BERR_NOT_AVAILABLE;
}

void BAPE_MixerInputCapture_Stop(
    BAPE_MixerInputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_MixerInputCapture);

    if ( handle->hStage )
    {
        BDSP_Stage_RemoveAudioCapture(handle->hStage, handle->hCapture);
    }
}

BERR_Code BAPE_MixerInputCapture_GetBuffer(
    BAPE_MixerInputCaptureHandle handle,
    BAPE_BufferDescriptor *pBuffers /* [out] */
    )
{
	BERR_Code errCode;
	BDSP_BufferDescriptor dspBufferDescriptor;
	unsigned i;

    BDBG_ASSERT(NULL != handle);
	BDBG_ASSERT(NULL != pBuffers);

	errCode = BDSP_AudioCapture_GetBuffer(handle->hCapture, &dspBufferDescriptor);

	for(i=0;i<BDSP_AF_P_MAX_CHANNELS;i++)
	{
		pBuffers->buffers[i].block = dspBufferDescriptor.buffers[i].buffer.hBlock;
        pBuffers->buffers[i].pBuffer = dspBufferDescriptor.buffers[i].buffer.pAddr;
		pBuffers->buffers[i].pWrapBuffer = dspBufferDescriptor.buffers[i].wrapBuffer.pAddr;
	}
 	pBuffers->bufferSize	= dspBufferDescriptor.bufferSize;
 	pBuffers->interleaved	= dspBufferDescriptor.interleaved;
 	pBuffers->numBuffers	= dspBufferDescriptor.numBuffers;
 	pBuffers->wrapBufferSize = dspBufferDescriptor.wrapBufferSize;

	return errCode;	
}

BERR_Code BAPE_MixerInputCapture_ConsumeData(
    BAPE_MixerInputCaptureHandle handle,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    )
{
	BERR_Code errCode;
	
	BDBG_ASSERT(NULL != handle);
	errCode = BDSP_AudioCapture_ConsumeData(handle->hCapture, numBytes);
	return errCode;	
}

BERR_Code	BAPE_ProcessAudioCapture(BDSP_Handle device)
{
	BERR_Code errCode;

	errCode = BDSP_ProcessAudioCapture(device);
	return errCode;	
}


void BAPE_MixerInputCapture_GetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    BAPE_MixerInputCaptureInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BDBG_ASSERT(NULL != hOutputCapture);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    *pInterrupts = hOutputCapture->interrupts;
    BKNI_LeaveCriticalSection();
}

BERR_Code BAPE_MixerInputCapture_SetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    const BAPE_MixerInputCaptureInterruptHandlers *pInterrupts
    )
{
    BDBG_ASSERT(NULL != hOutputCapture);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    hOutputCapture->interrupts = *pInterrupts;   
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}
#else /* #if BAPE_CHIP_MAX_DSP_MIXERS */
void BAPE_MixerInputCapture_GetDefaultCreateSettings(
    BAPE_MixerInputCaptureCreateSettings *pSettings  /* [out] */
    )
{
	BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_MixerInputCapture_Create(
    BAPE_Handle hApe,
    const BAPE_MixerInputCaptureCreateSettings *pSettings,
    BAPE_MixerInputCaptureHandle *pMixerInputCaptureHandle   /* [out] */
    )
{
	BSTD_UNUSED(hApe);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pMixerInputCaptureHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MixerInputCapture_Destroy(
    BAPE_MixerInputCaptureHandle handle
    )
{
	BSTD_UNUSED(handle);
}

BERR_Code BAPE_MixerInputCapture_Start(
    BAPE_MixerInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MixerInputCapture_Stop(
    BAPE_MixerInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

BERR_Code BAPE_MixerInputCapture_GetBuffer(
    BAPE_MixerInputCaptureHandle handle,
    BAPE_BufferDescriptor *pBuffers /* [out] */
    )
{
	BSTD_UNUSED(handle);
    BSTD_UNUSED(pBuffers);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_MixerInputCapture_ConsumeData(
    BAPE_MixerInputCaptureHandle handle,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    )
{
	BSTD_UNUSED(handle);
    BSTD_UNUSED(numBytes);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_MixerInputCapture_GetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    BAPE_MixerInputCaptureInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BSTD_UNUSED(hOutputCapture);
    BSTD_UNUSED(pInterrupts);;
}

BERR_Code BAPE_MixerInputCapture_SetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    const BAPE_MixerInputCaptureInterruptHandlers *pInterrupts
    )
{
    BSTD_UNUSED(hOutputCapture);
    BSTD_UNUSED(pInterrupts);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif /* #if BAPE_CHIP_MAX_DSP_MIXERS */
