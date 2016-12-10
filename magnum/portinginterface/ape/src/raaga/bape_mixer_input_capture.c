/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#if !B_REFSW_MINIMAL
BDBG_MODULE(bape_mixer_input_capture);
 
BDBG_OBJECT_ID(BAPE_MixerInputCapture);

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
	BDSP_AudioCaptureHandle *pCapture;
	BAPE_MixerInputCapture  *pMxrInputCapture;
	BDSP_AudioCaptureCreateSettings dspSettings;
	BERR_Code errCode;

	
    BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(NULL != pMixerInputCaptureHandle);
    

	/* Malloc handle structure, set object id */
	pMxrInputCapture = (BAPE_MixerInputCapture *)BKNI_Malloc(sizeof(BAPE_MixerInputCapture));
    if ( NULL == pMxrInputCapture )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc_stage;
    }
    
    /* Update the capture information using the capture create settings */
    BKNI_Memset(pMxrInputCapture, 0, sizeof(*pMxrInputCapture));
	BDBG_OBJECT_SET(pMxrInputCapture, BAPE_MixerInputCapture);
	
	
	BDSP_AudioCapture_GetDefaultCreateSettings(&dspSettings);
	
	dspSettings.maxChannels = pSettings->maxChannels;
    dspSettings.channelBufferSize = pSettings->channelBufferSize;
	dspSettings.hHeap = pSettings->hHeap;


	pCapture = &(pMxrInputCapture->hCapture);
	errCode = BDSP_AudioCapture_Create(hApe->dspContext, &dspSettings, pCapture);

	*pMixerInputCaptureHandle = pMxrInputCapture;

	
err_malloc_stage:
	return errCode;	

}


void BAPE_MixerInputCapture_Destroy(
    BAPE_MixerInputCaptureHandle hMixerInputCapture
    )
{

	BDBG_OBJECT_ASSERT(hMixerInputCapture, BAPE_MixerInputCapture);
	BDSP_AudioCapture_Destroy(hMixerInputCapture->hCapture);

	BDBG_OBJECT_DESTROY(hMixerInputCapture, BAPE_MixerInputCapture);
	BKNI_Free(hMixerInputCapture);
}


BERR_Code BAPE_MixerInputCapture_GetBuffer(
    BAPE_MixerInputCaptureHandle hMixerInputCapture,
    BAPE_BufferDescriptor *pBuffers /* [out] */
    )
{
	BERR_Code errCode;
	BDSP_BufferDescriptor dspBufferDescriptor;
	unsigned i;
	
	
	BDBG_ASSERT(NULL != hMixerInputCapture);	
	BDBG_ASSERT(NULL != pBuffers);

	errCode = BDSP_AudioCapture_GetBuffer(hMixerInputCapture->hCapture, &dspBufferDescriptor);

	for(i=0;i<BDSP_AF_P_MAX_CHANNELS;i++)
	{
		pBuffers->buffers[i].pBuffer = dspBufferDescriptor.buffers[i].pBuffer;
		pBuffers->buffers[i].pWrapBuffer = dspBufferDescriptor.buffers[i].pWrapBuffer;
	}
 	pBuffers->bufferSize	= dspBufferDescriptor.bufferSize;
 	pBuffers->interleaved	= dspBufferDescriptor.interleaved;
 	pBuffers->numBuffers	= dspBufferDescriptor.numBuffers;
 	pBuffers->wrapBufferSize = dspBufferDescriptor.wrapBufferSize;

	return errCode;	
}

BERR_Code BAPE_MixerInputCapture_ConsumeData(
    BAPE_MixerInputCaptureHandle hMixerInputCapture,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    )
{
	BERR_Code errCode;
	
	BDBG_ASSERT(NULL != hMixerInputCapture);

	errCode = BDSP_AudioCapture_ConsumeData(hMixerInputCapture->hCapture, numBytes);

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
#endif
