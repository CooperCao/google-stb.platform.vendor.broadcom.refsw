/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Mixer Interface
 *
 * Revision History:
 *
 *
 ***************************************************************************/

 
#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"



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

