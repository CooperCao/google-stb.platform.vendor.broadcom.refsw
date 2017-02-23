/******************************************************************************
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
 *****************************************************************************/
#ifndef NEXUS_SIMPLE_AUDIO_DECODER_H__
#define NEXUS_SIMPLE_AUDIO_DECODER_H__

#include "nexus_types.h"
#include "nexus_simple_stc_channel.h"
#include "nexus_simple_decoder_types.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder_types.h"
#include "nexus_audio_processor.h"
#endif
#include "nexus_core_compat.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
SimpleAudioDecoder is an already-opened, pre-connected audio decoder.
It is used in multi-process systems where garbage collection is required.

Dual output configurations (i.e. TV1 and TV2) are not supported.
**/

/**
Summary:
**/
typedef struct NEXUS_SimpleAudioDecoder *NEXUS_SimpleAudioDecoderHandle;

/**
Summary:
**/
NEXUS_SimpleAudioDecoderHandle NEXUS_SimpleAudioDecoder_Acquire( /* attr{release=NEXUS_SimpleAudioDecoder_Release}  */
    unsigned index
    );

/**
Summary:
**/
void NEXUS_SimpleAudioDecoder_Release(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary: Decoder Selector
**/
typedef enum NEXUS_SimpleAudioDecoderSelector {
    NEXUS_SimpleAudioDecoderSelector_ePrimary,
    NEXUS_SimpleAudioDecoderSelector_eSecondary,
    NEXUS_SimpleAudioDecoderSelector_eDescription,
    NEXUS_SimpleAudioDecoderSelector_eMax
} NEXUS_SimpleAudioDecoderSelector;


typedef struct NEXUS_SimpleAudioDecoderStatus
{
    NEXUS_AudioDecoderStatus status[NEXUS_SimpleAudioDecoderSelector_eMax];
} NEXUS_SimpleAudioDecoderStatus;

/**
Summary:
**/
typedef struct NEXUS_SimpleAudioDecoderProcessorSettings
{
    struct {
        bool connected;                     /* if true, Fade processing will be inserted in the processing chain
                                               and be available for use. if false, AudioFadeSettings will be ignored. */
        NEXUS_AudioFadeSettings settings;
    } fade;
    struct {
        bool connected;                     /* if true, KaraokeVocal processing will be inserted in the processing chain
                                               and be available for use. if false, KaraokeVocalSettings will be ignored. */
        NEXUS_KaraokeVocalSettings settings;
    } karaokeVocal;
} NEXUS_SimpleAudioDecoderProcessorSettings;

/**
Summary:
**/
typedef struct NEXUS_SimpleAudioDecoderStartSettings
{
    NEXUS_AudioDecoderStartSettings primary;   /* Required pid and codec for audio decode and (if secondary settings are not specified)
                                                  for passthrough. */
    NEXUS_AudioDecoderStartSettings secondary; /* Optional second pid and codec for passthrough decoder. if secondary.pidChannel == NULL,
                                                  primary start settings will be used for passthrough decoder */
    NEXUS_AudioDecoderStartSettings description; /* optional pid and codec for audio description */
    struct {
        bool pcm, compressed; /* start primers for pcm and/or compressed if regular decoder not available. */
    } primer;

    bool master; /* when using ePersistent decoder type with DSP mixer downstream, we need the app to tell us if we are the master. */

    struct {
        bool enabled;                   /* If true, decoder will be used to passthrough application-provided IEC61937 data via GetPassthroughBuffer()
                                           and PassthroughWriteComplete().  No decoding will be performed. */
        unsigned sampleRate;            /* Sample Rate in Hz */
        NEXUS_CallbackDesc dataCallback;
    } passthroughBuffer;
} NEXUS_SimpleAudioDecoderStartSettings;

/**
Summary:
Run-time settings

Description:
Changing some of these settings after start may require a stop/start. Nexus will handle this automatically.

pcmOutput will result in an internal restart.
**/
typedef struct NEXUS_SimpleAudioDecoderSettings
{
    NEXUS_AudioDecoderSettings primary;
    NEXUS_AudioDecoderSettings secondary;
    NEXUS_AudioDecoderSettings description;
    NEXUS_SimpleAudioDecoderProcessorSettings processorSettings[NEXUS_SimpleAudioDecoderSelector_eMax];
} NEXUS_SimpleAudioDecoderSettings;

/**
Summary:
Set or clear SimpleStcChannel for lipsync.

For advanced lipsync, including SyncChannel and Astm, all lipsynced decoders must be set before any is started.

Description:
If you set and start audio and video separately, you will get a glitch when both audio and video have started
and SyncChannel makes an adjustment. Another option is to setNEXUS_SimpleStcChannelSettings.sync = false and
get only basic TSM, off by up to 100 msec, with no glitch.
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_SetStcChannel(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleStcChannelHandle stcChannel /* attr{null_allowed=y} use NULL to clear. */
    );

/**
Summary: Get Default Start Settings
**/
void NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(
    NEXUS_SimpleAudioDecoderStartSettings *pSettings /* [out] */
    );

/**
Summary: Start
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_Start(
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_SimpleAudioDecoderStartSettings *pSettings
    );

/**
Summary: Stop
**/
void NEXUS_SimpleAudioDecoder_Stop(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary: Get Settings
**/
void NEXUS_SimpleAudioDecoder_GetSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSettings *pSettings /* [out] */
    );

/**
Summary: Set Settings
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_SetSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_SimpleAudioDecoderSettings *pSettings
    );

/**
Summary: Get Status
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_GetStatus(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioDecoderStatus *pStatus   /* [out] Note that the regular NEXUS_AudioDecoder structure is used */
    );

/**
Summary: Get Status
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_GetCombinedStatus(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderStatus *pStatus
    );


/***************************************************************************
Summary:
    Get the Presentation info for specified presentation. This call is not available for all codecs.
    Use NEXUS_SimpleAudioDecoder_GetStatus() to retrieve the number of presentations, etc
***************************************************************************/
NEXUS_Error NEXUS_SimpleAudioDecoder_GetPresentationStatus(
    NEXUS_SimpleAudioDecoderHandle handle,
    unsigned presentationIndex,
    NEXUS_AudioDecoderPresentationStatus *pStatus /* [out] Note that the regular NEXUS_AudioDecoder structure is used */
    );

/**
Summary: Get Processor Status
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_GetProcessorStatus(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioPostProcessing type,
    NEXUS_AudioProcessorStatus *pStatus
    );

/**
Summary: Flush
**/
void NEXUS_SimpleAudioDecoder_Flush(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary: Get Trick State
**/
void NEXUS_SimpleAudioDecoder_GetTrickState(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioDecoderTrickState *pStatus   /* [out] Note that the regular NEXUS_AudioDecoder structure is used */
    );

/**
Summary: Set Trick State
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_SetTrickState(
    NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_AudioDecoderTrickState *pStatus   /* Note that the regular NEXUS_AudioDecoder structure is used */
    );

/**
Summary: Advance
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_Advance(
    NEXUS_SimpleAudioDecoderHandle handle,
    uint32_t pts
    );

/**
Summary: Get codec-specific decoder settings
**/
void NEXUS_SimpleAudioDecoder_GetCodecSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioCodec codec,
    NEXUS_AudioDecoderCodecSettings *pSettings  /* [out] settings for specified codec */
    );

/**
Summary: Set codec-specific decoder settings
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_SetCodecSettings(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    const NEXUS_AudioDecoderCodecSettings *pSettings
    );

/**
Summary: Suspend
Perform internal decoder stop so that outputs can be reconfigured.

Returns NEXUS_SUCCESS if started decoder was stopped. Returns NEXUS_SIMPLE_DECODER_NOT_ENABLED if decoder was not started so could not be stopped.
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_Suspend(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary: Resume
Perform internal decoder start after Suspend.

Returns NEXUS_SUCCESS if start successful or if not suspended. Returns not NEXUS_SUCCESS if could not be started.
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_Resume(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary: Start HdmiInput interface
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_StartHdmiInput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_SimpleAudioDecoderStartSettings *pStartSettings /* attr{null_allowed=y} */
    );

/**
Summary: Stop HdmiInput interface
**/
void NEXUS_SimpleAudioDecoder_StopHdmiInput(
    NEXUS_SimpleAudioDecoderHandle handle
    );

/**
Summary:
Application Passthrough buffer management
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_GetPassthroughBuffer(
    NEXUS_SimpleAudioDecoderHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for passthrough data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_PassthroughWriteComplete(
    NEXUS_SimpleAudioDecoderHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    );

/**
Summary:
Application output management for Standalone Simple Audio Decoders
**/
NEXUS_Error NEXUS_SimpleAudioDecoder_AddOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType
    );

NEXUS_Error NEXUS_SimpleAudioDecoder_RemoveOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output
    );

#ifdef __cplusplus
}
#endif

#endif
