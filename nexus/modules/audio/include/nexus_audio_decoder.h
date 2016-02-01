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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: AudioDecoder
*    API for audio decoder management.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_DECODER_H__
#define NEXUS_AUDIO_DECODER_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_decoder_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioDecoder

Header file: nexus_audio_decoder.h

Module: Audio

Description: Decode compressed audio from a NEXUS_PidChannel

**************************************/

/***************************************************************************
Summary:
Audio decoder type
***************************************************************************/
typedef enum NEXUS_AudioDecoderType
{
    NEXUS_AudioDecoderType_eDecode,
    NEXUS_AudioDecoderType_eAudioDescriptor,
    NEXUS_AudioDecoderType_ePassthrough,
    NEXUS_AudioDecoderType_eDecodeToMemory,  /* Decode to memory mode.  This mode is used to use the audio
                                                DSP as an accelerator that only decodes compressed audio and
                                                returns the decoded samples to the application for processing rather
                                                than performing TSM and sending data into the mixing subsystem.
                                                Interfaces for this mode are in nexus_audio_decode_to_host.h. */
    NEXUS_AudioDecoderType_eMax
} NEXUS_AudioDecoderType;

/***************************************************************************
Summary:
Audio decoder open settings
***************************************************************************/
typedef struct NEXUS_AudioDecoderOpenSettings
{
    NEXUS_AudioDecoderType type;

    unsigned fifoSize;                  /* Audio FIFO size in bytes */
    unsigned ancillaryDataFifoSize;     /* Ancillary Data FIFO size in bytes */

    bool independentDelay;              /* Set to true to enable independent output delay */

    NEXUS_AudioMultichannelFormat multichannelFormat;   /*
        Allocate resources at Open-time so that NEXUS_AudioOutput_AddInput with NEXUS_AudioDecoderConnectorType_eMultichannel can do multichannel output.
        If you set multichannelFormat = NEXUS_AudioMultichannelFormat_eNone, then NEXUS_AudioDecoderConnectorType_eMultichannel will simply result in stereo.
        If you set multichannelFormat to something other than eNone, then NEXUS_AudioDecoderConnectorType_eMultichannel will result in the specified multichannel format.
        This setting has no effect on connector types other than NEXUS_AudioDecoderConnectorType_eMultichannel.
        Default is NEXUS_AudioMultichannelFormat_eNone to save resources. */

    unsigned dspIndex;  /* Index of the DSP you would like to use.  Default = 0. */
    NEXUS_HeapHandle cdbHeap;
    bool karaokeSupported;           /* Set to true to enable Karaoke post processing support */
} NEXUS_AudioDecoderOpenSettings;

/***************************************************************************
Summary:
Get default open settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultOpenSettings(
    NEXUS_AudioDecoderOpenSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an audio decoder of the specified type
***************************************************************************/
NEXUS_AudioDecoderHandle NEXUS_AudioDecoder_Open( /* attr{destructor=NEXUS_AudioDecoder_Close}  */
    unsigned index,
    const NEXUS_AudioDecoderOpenSettings *pSettings   /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an audio decoder of the specified type
***************************************************************************/
void NEXUS_AudioDecoder_Close(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
Get Settings for an audio decoder
***************************************************************************/
void NEXUS_AudioDecoder_GetSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSettings *pSettings   /* [out] Settings */
    );

/***************************************************************************
Summary:
Set Settings for an audio decoder
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSettings *pSettings /* Settings */
    );

/***************************************************************************
Summary:
Initialize an audio decoder program structure
***************************************************************************/
void NEXUS_AudioDecoder_GetDefaultStartSettings(
    NEXUS_AudioDecoderStartSettings *pSettings /* [out] Program Defaults */
    );

/***************************************************************************
Summary:
Start deocding the specified program
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Start(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderStartSettings *pSettings    /* What to start decoding */
    );

/***************************************************************************
Summary:
Stop deocding the current program
***************************************************************************/
void NEXUS_AudioDecoder_Stop(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
Stop decoding the current program without flushing
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Suspend(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
Start decoding from the suspended state
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Resume(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
Discards all data accumulated in the decoder buffer
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_Flush(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Get codec-specific decoder settings
***************************************************************************/
void NEXUS_AudioDecoder_GetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioDecoderCodecSettings *pSettings  /* [out] settings for specified codec */
    );

/***************************************************************************
Summary:
    Set codec-specific decoder settings
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetCodecSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderCodecSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the current audio decoder status
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderStatus *pStatus   /* [out] current status */
    );

/***************************************************************************
Summary:
    Get raw channel status information from the decoder

Description:
    When the decoder is connected to a digital input, this routine can
    return the raw channel status bit information from the input device.
    Currently, this applies to HDMI or SPDIF inputs only.  This routine
    will return an error if not connected to a digital input.

See Also:
    NEXUS_SpdifOutput_SetRawChannelStatus
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetRawChannelStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioRawChannelStatus *pStatus   /* [out] current status */
    );

/***************************************************************************
Summary:
Audio decoder connection type 
 
Description: 
These are legacy macros for backward compatibility.  Please refer to 
NEXUS_AudioConnectorType defined in nexus_audio_types.h. 
 
See Also: 
NEXUS_AudioConnectorType 
***************************************************************************/
#define NEXUS_AudioDecoderConnectorType                 NEXUS_AudioConnectorType
#define NEXUS_AudioDecoderConnectorType_eStereo         NEXUS_AudioConnectorType_eStereo       /* two channel PCM audio */
#define NEXUS_AudioDecoderConnectorType_eMultichannel   NEXUS_AudioConnectorType_eMultichannel /* multichannel PCM audio 
                                                            (e.g. 5.1 or 7.1). see NEXUS_AudioDecoderOpenSettings.multichannelFormat for
                                                            required open-time settings to enable multichannel output. */
#define NEXUS_AudioDecoderConnectorType_eCompressed     NEXUS_AudioConnectorType_eCompressed   /* compressed audio (may be stereo or multichannel) */
#define NEXUS_AudioDecoderConnectorType_eMono           NEXUS_AudioConnectorType_eMono         /* single-channel PCM audio */
#define NEXUS_AudioDecoderConnectorType_eMax            NEXUS_AudioConnectorType_eMax

/***************************************************************************
Summary:
    Get an audio connector for use in the audio mixer
***************************************************************************/
NEXUS_AudioInput NEXUS_AudioDecoder_GetConnector( 
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioConnectorType type
    );

/***************************************************************************
Summary:
Discover if a codec is supported.

Description:
Codecs may not be supported because of lack of hardware support, compile-time options, or
run-time options like module memory settings.
***************************************************************************/
void NEXUS_AudioDecoder_IsCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    );

/***************************************************************************
Summary:
Discover if a codec is supported for passthrough.

Description:
Codecs may not be supported because of lack of hardware support, compile-time options, or
run-time options like module memory settings.
***************************************************************************/
void NEXUS_AudioDecoder_IsPassthroughCodecSupported(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioCodec codec,
    bool *pSupported
    );

/***************************************************************************
Summary:
Get ancillary data from a digital stream.

Description:
Ancillary data for audio streams can contain additional per-frame metadata, 
including information about the encoder used and track information.  This 
interface allows the application to capture and process that data. The format 
of the data is stream-dependent.

The format of the data stream is NEXUS_AudioAncillaryDataHeader, followed by 
a whole ancillary data packet.  Ringbuffer wraps are handled internally, so 
NEXUS_AudioDecoder_GetAncillaryDataBuffer will always return whole packets 
including headers.

You must specify the NEXUS_AudioDecoderSettings.ancillaryDataEnabled to enable 
data flow to this interface.

NEXUS_AudioDecoder_GetUserDataBuffer can be called multiple times in a row and is non-destructive.

See Also:
NEXUS_AudioDecoder_AncillaryDataReadComplete
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetAncillaryDataBuffer(
    NEXUS_AudioDecoderHandle handle,
    void **pBuffer,   /* [out] attr{memory=cached} pointer to ancillary data */
    size_t *pSize     /* [out] number of bytes of userdata */
    );

/***************************************************************************
Summary:
Tell the audio decoder how much data was consumed from the last NEXUS_AudioDecoder_GetAncillaryDataBuffer call.

Description:
NEXUS_AudioDecoder_AncillaryDataReadComplete can only be called once 
after NEXUS_AudioDecoder_GetAncillaryDataBuffer. After calling 
NEXUS_AudioDecoder_AncillaryDataReadComplete, you must call 
NEXUS_AudioDecoder_GetAncillaryDataBuffer again before processing any 
more data.

See Also:
NEXUS_AudioDecoder_GetAncillaryDataBuffer
***************************************************************************/
void NEXUS_AudioDecoder_AncillaryDataReadComplete(
    NEXUS_AudioDecoderHandle handle,
    size_t size   /* number of bytes of userdata consumed by the application */
    );

/***************************************************************************
Summary:
Discard all ancillary data which is currently queued.
***************************************************************************/
void NEXUS_AudioDecoder_FlushAncillaryData(
    NEXUS_AudioDecoderHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif

