/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: AudioCapture
*    Specific APIs related to PCM audio capture.  This supports capture
*    of data into memory from a decoder or other source.
*
***************************************************************************/
#ifndef NEXUS_AUDIO_CAPTURE_H__
#define NEXUS_AUDIO_CAPTURE_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

/*=************************************
Interface: AudioCapture

Header file: nexus_audio_capture.h

Module: Audio

Description: Capture audio PCM data

**************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for audio capture
***************************************************************************/
typedef struct NEXUS_AudioCapture *NEXUS_AudioCaptureHandle;

/***************************************************************************
Summary:
Enumeration for capture data format

Description:
Captured data will always be stored in host endian mode.  

For 16-bit data:
Address Byte   0  1  2  3

               -----------
Little-Endian |R0|R1|L0|L1|
               -----------

               -----------
Big-Endian    |L1|L0|R1|R0|
               -----------

For 24-bit Data:
Address Byte   0  1  2  3    4  5  6  7

               -----------  -----------
Little-Endian |L0|L1|L2|00||R0|R1|R2|00|
               -----------  -----------

               -----------  -----------
Big-Endian    |L2|L1|L0|00||R2|R1|R0|00|
               -----------  -----------
***************************************************************************/
typedef enum NEXUS_AudioCaptureFormat
{
    NEXUS_AudioCaptureFormat_e16BitStereo,    /* Stereo, 16 bits per sample interleaved into a 32-bit word. */                                                
    NEXUS_AudioCaptureFormat_e24BitStereo,    /* Stereo, 24 bits per sample.  The data is aligned to 32-bits, 
                                                 left-justified.  Left and right channels will interleave 
                                                 one sample per 32-bit word.  */
    NEXUS_AudioCaptureFormat_e16BitMonoLeft,  /* Mono, 16 bits per sample interleaved into a 32-bit word. Left channel samples only. */
    NEXUS_AudioCaptureFormat_e16BitMonoRight, /* Mono, 16 bits per sample interleaved into a 32-bit word. Right channel samples only. */
    NEXUS_AudioCaptureFormat_e16BitMono,      /* Mono, 16 bits per sample interleaved into a 32-bit word. Left and Right channels mixed. */
    NEXUS_AudioCaptureFormat_e24Bit5_1,       /* 5.1 Multichannel, 24 bits per sample.  The data is aligned to 32-bits, 
                                                 left-justified.  Channels will interleave 
                                                 one sample per 32-bit word, ordered L,R,Ls,Rs,C,LFE.  */
    NEXUS_AudioCaptureFormat_e24Bit7_1,       /* 7.1 Multichannel, 24 bits per sample.  The data is aligned to 32-bits,
                                                 left-justified.  Channels will interleave
                                                 one sample per 32-bit word, ordered L,R,Ls,Rs,C,LFE,Lrs,Rrs.  */
    NEXUS_AudioCaptureFormat_eCompressed,     /* Compressed IEC-61937 audio up to 48 kHz for SPDIF/HDMI applications.  */
    NEXUS_AudioCaptureFormat_eMax
} NEXUS_AudioCaptureFormat;

/***************************************************************************
Summary:
Audio Capture Channel Open Settings

Description:
Audio Capture Channel Open Settings

See Also:
NEXUS_AudioCaptureStartSettings
***************************************************************************/
typedef struct NEXUS_AudioCaptureOpenSettings
{
    size_t fifoSize;                    /* FIFO size in bytes.  This value is a total FIFO size to hold
                                           all channels of data interleaved into a single buffer.
                                           If not set, a default size will be used. */    
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
} NEXUS_AudioCaptureOpenSettings;

/***************************************************************************
Summary:
Get default settings for opening an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an audio capture channel

Description:
The captured data will be formatted with the settings provided in 
NEXUS_AudioCaptureSettings.  The sample rate of the data is determined by 
the sample rate of the input data.  For example, if the decoder is outputting 
48kHz, the data will be the same sample rate.  If the decoder is not active, 
the output will be 48kHz.  Data will automatically start flowing once the
source device starts and stop when the source device stops.
***************************************************************************/
NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_Close(
    NEXUS_AudioCaptureHandle handle
    );

/***************************************************************************
Summary:
Audio Capture Channel Settings

Description:
Audio Capture Channel Settings.  Please note, some members may only be
changed while the capture channel is disconnected from all inputs.
***************************************************************************/
typedef struct NEXUS_AudioCaptureSettings
{
    NEXUS_AudioCaptureFormat format;    /* Can be set at open time. See NEXUS_AudioCaptureOpenSettings for usage. */
    size_t threshold;                   /* Can be set at open time. See NEXUS_AudioCaptureOpenSettings for usage. */
} NEXUS_AudioCaptureSettings;

/***************************************************************************
Summary:
Get Audio Capture Channel Settings
***************************************************************************/
void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Capture Channel Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    );

/***************************************************************************
Summary:
Audio Capture Channel Start Settings

Description:
Audio Capture Channel Start Settings

See Also:
NEXUS_AudioCaptureStartSettings
***************************************************************************/
typedef struct NEXUS_AudioCaptureStartSettings
{
    NEXUS_CallbackDesc dataCallback;    /* Callback when data becomes available. User should call NEXUS_AudioCapture_GetBuffer. 
                                           You will not receive another callback until NEXUS_AudioCapture_GetBuffer is called. */
    NEXUS_CallbackDesc sampleRateCallback;    /* Callback when sample rate changes.  Call NEXUS_AudioCapture_GetStatus() to get the
                                                 current sample rate */
} NEXUS_AudioCaptureStartSettings;

/***************************************************************************
Summary:
Get current setting of the audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    );

/***************************************************************************
Summary:
Start capturing data.  

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioCapture_Stop
NEXUS_AudioCapture_GetConnector
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    );

/***************************************************************************
Summary:
Get a pointer and size for the next location in the buffer that contains data

Description:
NEXUS_AudioCapture_GetBuffer is non-destructive. You can safely call it 
multiple times.
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_GetBuffer(
    NEXUS_AudioCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped 
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    );

/***************************************************************************
Summary:
Notify AudioCapture how much data removed from the buffer.

Description:
You can only call NEXUS_AudioCapture_ReadComplete once after a 
NEXUS_AudioCapture_GetBuffer call.  After calling it, you must call 
NEXUS_AudioCapture_GetBuffer before reading more data.
***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_ReadComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    );

/* backward compatibility */
#define NEXUS_AudioCapture_WriteComplete NEXUS_AudioCapture_ReadComplete

/***************************************************************************
Summary:
Get an audio connector to attach this output to a source.

Description:
Capture may be connected to a stereo source device or a compressed source
device, depending on the data format desired.
**************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector(
    NEXUS_AudioCaptureHandle handle
    );

/***************************************************************************
Summary:
Audio Capture Status 
**************************************************************************/
typedef struct NEXUS_AudioCaptureStatus
{
    bool started;           /* True if NEXUS_AudioCapture_Start() has been called. */
    unsigned sampleRate;    /* Current sample rate in Hz */
    NEXUS_EndianMode endian; /* Endian mode of the system */
} NEXUS_AudioCaptureStatus;

/***************************************************************************
Summary:
Get Current Audio Capture Status 
**************************************************************************/
NEXUS_Error NEXUS_AudioCapture_GetStatus(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureStatus *pStatus   /* [out] */
    );

#ifdef __cplusplus
}
#endif
#endif /* #ifndef NEXUS_AUDIO_CAPTURE_H__ */
