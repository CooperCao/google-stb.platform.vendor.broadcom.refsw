/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: AudioDecoder
*    API for audio decoder management.
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
    bool spliceEnabled;    /* Enables the dynamic splicing feature used in ad insertion.
                              When this flag is set, audio decoder open creates only SW rave context.
                              HW rave context creation is deferred to audio decoder start */
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
    Get the Program status for specified program index. This API only applies
    to codecs that support multiple programs in a single PID. When applicable,
    use NEXUS_AudioDecoder_GetStatus() to retrieve the number of presentations.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetPresentationStatus(
    NEXUS_AudioDecoderHandle handle,
    unsigned presentationIndex,
    NEXUS_AudioDecoderPresentationStatus *pStatus
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
NEXUS_AudioInputHandle NEXUS_AudioDecoder_GetConnector(
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

/***************************************************************************
Summary:
Settings for configuring splice mode
Description:
This structure defines a point in the stream at which it will be stopped or started
so the alternative stream can be substituted. The point is defined by the PTS value
and PTS threshold value. Each incoming iPTS is evaluated according to this formula:
pts <= iPTS <= (pts + ptsThreshold). If iPTS satisfies this condition, the stream will be
stopped if mode is eStopAtPts or started if mode is eStartAtPts. Additionally provided
callback will be invoked so the application can provide the replacement stream or perform
cleanup.
The pts and ptsThreshold are in measured 45000Hz units for mpeg and h264 streams. In some
special cases the pts units may be different. The provided pts and ptsThreshold values
are used directly so it is responsibility of the caller to provide correct values as
used by the system.
****************************************************************************/
typedef struct NEXUS_AudioDecoderSpliceSettings
{
    NEXUS_DecoderSpliceMode mode; /* Desired splice mode. If mode is eDisabled the rest of the fields are ignored. */
    uint32_t pts;                      /* Pts value to start or stop decoding depending on the mode */
    uint32_t ptsThreshold;             /* Pts threshold, defines a window of pts to pts+threshold during which splice mode is activated */
    NEXUS_CallbackDesc splicePoint;    /* Callback to call whenever pts falls within a splice window */
} NEXUS_AudioDecoderSpliceSettings;

/***************************************************************************
Summary:
Status of splice operation
Description:
Structure contains the curent splice state that is tracked by video decoder and the last
pts which caused splice transition coming from rave.
****************************************************************************/
typedef struct NEXUS_AudioDecoderSpliceStatus
{
    NEXUS_DecoderSpliceState state;
    uint32_t pts;
} NEXUS_AudioDecoderSpliceStatus;

/***************************************************************************
Summary:
Set desired splice settings
Description:
Configure desired splice settings for content splicing.
If mode is eDisabled, no content splicing will be performed. This is the default state.
If mode is eStopAtPts, decoder will stop decoding at a specified pts. Application then must
follow the procedure to start a replacement stream. This mode is used to switch from the main
stream to the replacement stream.
If mode is eStartAtPts, decoder will start decoding from a specified pts. This mode is used
to return to the main stream.
See Also:
NEXUS_AudioDecoder_GetSpliceSettings
****************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SetSpliceSettings(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSpliceSettings *pSettings
    );

/***************************************************************************
Summary:
Retrieve currently active splice settings.
See Also:
NEXUS_AudioDecoder_SetSpliceSettings
****************************************************************************/
void NEXUS_AudioDecoder_GetSpliceSettings(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSpliceSettings *pSettings
    );

/***************************************************************************
Summary:
Retrieve current status of splice operation
Description:
This function can be called during insertionPoint callback to query current status of
splice operation.
****************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_GetSpliceStatus(
    NEXUS_AudioDecoderHandle handle,
    NEXUS_AudioDecoderSpliceStatus *pStatus
    );

/***************************************************************************
Summary:
Stop data flow to the decoder.
Description:
This function is used to stop data flow to the decoder in case of live decode. Diring playback
we simply can stop playing one stream and play another stream instead. In case of live decode
we must stop live stream from reaching decoder using this function. Content will be still received
by tuners and processed by transport module but will not reach the decoder.
See Also:
NEXUS_AudioDecoder_SpliceStartFlow
****************************************************************************/
void NEXUS_AudioDecoder_SpliceStopFlow(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
Settings for resuming of data flow to the decoder
****************************************************************************/
typedef struct NEXUS_AudioDecoderSpliceStartFlowSettings
{
    NEXUS_PidChannelHandle pidChannel; /* pidChannel to use as a source of data from transport */
} NEXUS_AudioDecoderSpliceStartFlowSettings;

/***************************************************************************
Summary:
Initialize NEXUS_AudioDecoderSpliceStartFlowSettings structure to default values.
****************************************************************************/
void NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings(
    NEXUS_AudioDecoderSpliceStartFlowSettings *pSettings
    );

/***************************************************************************
Summary:
Resume data flow which was stopped by NEXUS_AudioDecoder_SpliceStopFlow call.
Description:
This function is used to resume data flow, previously stopped by NEXUS_AudioDecoder_SpliceStopFlow
api. It allows to select source of the data by providing the correct pidChannel in the settings.
To decode data from live stream, pid channel associated with live stream should be provided,
to decode data from disk, pid channel associated with playback should be provided in settings.
See Also:
NEXUS_AudioDecoder_SpliceStopFlow
NEXUS_AudioDecoderSpliceStartFlowSettings
NEXUS_AudioDecoder_GetDefaultSpliceStartFlowSettings
****************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_SpliceStartFlow(
    NEXUS_AudioDecoderHandle handle,
    const NEXUS_AudioDecoderSpliceStartFlowSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
