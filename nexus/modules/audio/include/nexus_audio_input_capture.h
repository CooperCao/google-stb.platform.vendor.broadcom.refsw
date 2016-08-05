/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*  
*  This program is the proprietary software of Broadcom and/or its licensors,
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
*   API name: AudioInputCapture
*    Object used to capture data from an external input into the audio subsystem
*
***************************************************************************/
#ifndef NEXUS_AUDIO_INPUT_CAPTURE_H_
#define NEXUS_AUDIO_INPUT_CAPTURE_H_

#include "nexus_audio_capture.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioInputCapture

Header file: nexus_audio_input_capture.h

Module: Audio

Description: Capture data from an external input into the audio subsystem

**************************************/

/***************************************************************************
Summary:
Handle for audio input capture
***************************************************************************/
typedef struct NEXUS_AudioInputCapture *NEXUS_AudioInputCaptureHandle;

/***************************************************************************
Summary:
Open-time settings for AudioInputCapture
***************************************************************************/
typedef struct NEXUS_AudioInputCaptureOpenSettings
{
    size_t fifoSize;                    /* FIFO size in bytes. Set to valid non-zero value to enable
                                           capture to memory functionality.
                                           This value is a total FIFO size to hold
                                           all channels of data interleaved into a single buffer. */
    NEXUS_AudioMultichannelFormat multichannelFormat;   /* This defines the maximum width of data that can be captured.
                                                           Default is NEXUS_AudioMultichannelFormat_eStereo to capture stereo
                                                           data.  5.1 or 7.1 will increase the number of buffers allocated. */
    NEXUS_HeapHandle heap;              /* Optional heap for fifo allocation */
    NEXUS_AudioCaptureFormat format;    /* Captured data format.  Default is 16bit stereo.
                                           This value is ignored for compressed data, and
                                           can not be changed while connected to any inputs. */
    size_t threshold;                   /* FIFO data callback threshold in bytes.  When the amount of data
                                           in the FIFO reaches this level, the dataCallback
                                           routine provided in NEXUS_AudioCaptureStartSettings will be called.
                                           If this value is 0, a default threshold percentage will be used. */
} NEXUS_AudioInputCaptureOpenSettings;

/***************************************************************************
Summary:
Get default Open-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultOpenSettings(
    NEXUS_AudioInputCaptureOpenSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Open an AudioInputCapture handle
***************************************************************************/
NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_Open(
    unsigned index,
    const NEXUS_AudioInputCaptureOpenSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an AudioInputCapture handle
***************************************************************************/
void NEXUS_AudioInputCapture_Close(
    NEXUS_AudioInputCaptureHandle handle
    );

/***************************************************************************
Summary:
Start-time settings for AudioInputCapture
***************************************************************************/
typedef struct NEXUS_AudioInputCaptureStartSettings
{
    NEXUS_AudioInputHandle input;
} NEXUS_AudioInputCaptureStartSettings;

/***************************************************************************
Summary:
Get Default Start-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultStartSettings(
    NEXUS_AudioInputCaptureStartSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Start capturing input data
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_Start(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop capturing input data
***************************************************************************/
void NEXUS_AudioInputCapture_Stop(
    NEXUS_AudioInputCaptureHandle handle
    );

/***************************************************************************
Summary:
Settings for AudioInputCapture
***************************************************************************/
typedef struct NEXUS_AudioInputCaptureSettings
{
    NEXUS_CallbackDesc sourceChanged;   /* This callback fires whenever specifics such as sample rate, layer, or bitrate change.
                                           It also fires when capturing HDMI/SPDIF input if the input data format changes.  */
    NEXUS_CallbackDesc dataCallback;    /* Callback when data becomes available. User should call NEXUS_AudioInputCapture_GetBuffer.
                                           You will not receive another callback until NEXUS_AudioInputCapture_GetBuffer is called. */

    int32_t volumeMatrix[NEXUS_AudioChannel_eMax][NEXUS_AudioChannel_eMax]; /* Volume matrix.  This allows customization of channel volume
                                                                               output.  Default is [Left][Left] = Normal, [Right][Right] = Normal,
                                                                               [Center][Center] = Normal, etc.  All other volumes are zero by default.
                                                                               This will affect stereo and multichannel PCM outputs only,  not compressed. */
    bool muted;     /* This will affect stereo and multichannel PCM outputs only, not compressed. */

    NEXUS_AudioCaptureFormat format;    /* Only used when NEXUS_AudioInputCaptureOpenSettings->fifoSize is > 0.
                                           Can be set at open time. See NEXUS_AudioCaptureOpenSettings for usage. */
    size_t threshold;                   /* Only used when NEXUS_AudioInputCaptureOpenSettings->fifoSize is > 0.
                                           Can be set at open time. See NEXUS_AudioCaptureOpenSettings for usage. */
} NEXUS_AudioInputCaptureSettings;

/***************************************************************************
Summary:
Get current settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set settings for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_SetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureSettings *pSettings
    );

/***************************************************************************
Summary:
Status for AudioInputCapture
***************************************************************************/
typedef struct NEXUS_AudioInputCaptureStatus
{
    bool started;               /* True if capture has been started */
    bool inputSignalPresent;    /* True if the input signal is present (applicable to HDMI/SPDIF inputs only) */
    bool inputSignalValid;      /* True if the input signal is valid and can be captured (applicable to HDMI/SPDIF inputs only) */

    unsigned sampleRate;        /* In Hz */
    NEXUS_AudioCodec codec;     /* Audio stream format (applicable to HDMI/SPDIF inputs only) */
    unsigned numPcmChannels;    /* Number of PCM channels in the stream (applicable to HDMI input only) */
} NEXUS_AudioInputCaptureStatus;

/***************************************************************************
Summary:
Get current status for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_GetStatus(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
Get audio connector to connect InputCapture to other objects
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioInputCapture_GetConnector(
    NEXUS_AudioInputCaptureHandle handle
    );

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer that contains data

Description:
NEXUS_AudioInputCapture_GetBuffer is non-destructive. You can safely call it
multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_GetBuffer(
    NEXUS_AudioInputCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    );

/***************************************************************************
Summary:
Notify AudioInputCapture how much data removed from the buffer.

Description:
You can only call NEXUS_AudioInputCapture_ReadComplete once after a
NEXUS_AudioInputCapture_GetBuffer call.  After calling it, you must call
NEXUS_AudioInputCapture_GetBuffer before reading more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_ReadComplete(
    NEXUS_AudioInputCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_INPUT_CAPTURE_H_ */
