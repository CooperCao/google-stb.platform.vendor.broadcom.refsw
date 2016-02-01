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
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_MIXER_INPUT_CAPTURE_H_
#define BAPE_MIXER_INPUT_CAPTURE_H_

#include "bavc.h"

/***************************************************************************
Summary:
Mixer Input Capture Handle
***************************************************************************/
typedef struct BAPE_MixerInputCapture *BAPE_MixerInputCaptureHandle;


/***************************************************************************
Summary:
Mixer Input Capture Create Settings
***************************************************************************/
typedef struct BAPE_MixerInputCaptureCreateSettings
{
    unsigned maxChannels;       /* Maximum number of channels to capture.  1 = mono/compressed.  2 = stereo.  6 = 5.1.  Default = 2. */
    size_t channelBufferSize;   /* Channel buffer size in bytes.  Default is 1536kB. */
    BMEM_Heap_Handle hHeap;     /* Memory Heap to use for allocating buffers.  If NULL, the default heap will be used. */
} BAPE_MixerInputCaptureCreateSettings;

/***************************************************************************
Summary:
Get Default Mixer Input Capture Create Settings
***************************************************************************/
void BAPE_MixerInputCapture_GetDefaultCreateSettings(
    BAPE_MixerInputCaptureCreateSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Create an mixer input capture context 
 
Description: 
An Mixer Input Capture object is used to capture a copy of the DSP data into host 
memory prior to being consumed by intended consumer (e.g. audio mixer hardware). 
For the data to be copied, a thread must call BDSP_ProcessAudioCapture() on a 
background thread at a frequent interval (e.g. 10ms). 
 
To bind a capture handle to a mixer's input, provide it in 
BAPE_MixerAddInputSettings when connecting the input to a mixer.  A 
capture handle may be linked to only a single input on a single mixer. 
***************************************************************************/
BERR_Code BAPE_MixerInputCapture_Create(
    BAPE_Handle hApe,
    const BAPE_MixerInputCaptureCreateSettings *pSettings,
    BAPE_MixerInputCaptureHandle *pMixerInputCaptureHandle   /* [out] */
    );

/***************************************************************************
Summary:
Destroy an mixer input capture context
***************************************************************************/
void BAPE_MixerInputCapture_Destroy(
    BAPE_MixerInputCaptureHandle hMixerInputCapture
    );

/***************************************************************************
Summary:
Get the Buffer Descriptors
***************************************************************************/
BERR_Code BAPE_MixerInputCapture_GetBuffer(
    BAPE_MixerInputCaptureHandle hCapture,
    BAPE_BufferDescriptor *pBuffers /* [out] */
    );

/***************************************************************************
Summary:
Consume data from the capture buffers
***************************************************************************/
BERR_Code BAPE_MixerInputCapture_ConsumeData(
    BAPE_MixerInputCaptureHandle hCapture,
    unsigned numBytes                   /* Number of bytes read from each buffer */
    );

/***************************************************************************
Summary:
Consume data from the capture buffers
***************************************************************************/
BERR_Code	BAPE_ProcessAudioCapture(
			BDSP_Handle device
			);

/***************************************************************************
Summary:
Mixer Input Capture Interrupts
***************************************************************************/
typedef struct BAPE_MixerInputCaptureInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned sampleRate);
        void *pParam1;
        int param2;
    } sampleRate;
} BAPE_MixerInputCaptureInterruptHandlers;

/***************************************************************************
Summary:
Get Mixer Input Capture Interrupts
***************************************************************************/
void BAPE_MixerInputCapture_GetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    BAPE_MixerInputCaptureInterruptHandlers *pInterrupts    /* [out] */
    );

/***************************************************************************
Summary:
Set Mixer Input Capture Interrupts
***************************************************************************/
BERR_Code BAPE_MixerInputCapture_SetInterruptHandlers(
    BAPE_MixerInputCaptureHandle hOutputCapture,
    const BAPE_MixerInputCaptureInterruptHandlers *pInterrupts
    );

#endif /* #ifndef BAPE_MIXER_INPUT_CAPTURE_H_ */
