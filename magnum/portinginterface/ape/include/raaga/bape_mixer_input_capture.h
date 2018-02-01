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
Mixer Input Capture Create Settings
***************************************************************************/
typedef struct BAPE_MixerInputCaptureCreateSettings
{
    unsigned maxChannels;       /* Maximum number of channels to capture.  1 = mono/compressed.  2 = stereo.  6 = 5.1.  Default = 2. */
    size_t channelBufferSize;   /* Channel buffer size in bytes.  Default is 1536kB. */
    BMMA_Heap_Handle hHeap;     /* Memory Heap to use for allocating buffers.  If NULL, the default heap will be used. */
    BAPE_Connector input;       /* Optional connector associated with capture point. Default is NULL.
                                   If NULL, BAPE_MixerInputSettings must be used to configure the capture point */
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
Start a mixer input capture context
***************************************************************************/
BERR_Code BAPE_MixerInputCapture_Start(
    BAPE_MixerInputCaptureHandle handle
    );

/***************************************************************************
Summary:
Stop a mixer input capture context
***************************************************************************/
void BAPE_MixerInputCapture_Stop(
    BAPE_MixerInputCaptureHandle handle
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


#if BAPE_DSP_SUPPORT
/***************************************************************************
Summary:
Consume data from the capture buffers
***************************************************************************/
BERR_Code   BAPE_ProcessAudioCapture(
            BDSP_Handle device
            );

#endif /* BAPE_DSP_SUPPORT */
#endif /* #ifndef BAPE_MIXER_INPUT_CAPTURE_H_ */
