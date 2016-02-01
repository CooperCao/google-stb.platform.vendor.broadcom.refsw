/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   Private routines internal to the audio module
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_AUDIO_PRIV_H__
#define NEXUS_AUDIO_PRIV_H__

#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decode_to_memory.h"
#include "nexus_audio_decoder_extra.h"
#include "brap.h"
#include "nexus_platform_features.h"
#include "nexus_audio_init.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_core_audio.h"
#include "priv/nexus_audio_input_priv.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_dts_encode.h"
#include "nexus_ac3_encode.h"
#include "nexus_dolby_volume.h"
#include "nexus_audio_output_group.h"

#include "nexus_audio_dac.h"
#if NEXUS_NUM_I2S_OUTPUTS
#include "nexus_i2s_output.h"
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
#include "nexus_spdif_output.h"
#endif
#if 0 && NEXUS_NUM_HDMI_OUTPUTS
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_NUM_RF_MODS
#include "nexus_rf_mod.h"
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
#include "nexus_audio_playback.h"
#endif

#include "nexus_tru_volume.h"
#include "nexus_audio_encoder.h"
#include "nexus_rf_audio_encoder.h"
#include "nexus_auto_volume_level.h"
#include "nexus_3d_surround.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_audio_capture.h"

typedef struct NEXUS_AudioDecoder
{
    NEXUS_OBJECT(NEXUS_AudioDecoder);
    unsigned index;
    NEXUS_AudioDecoderSettings settings;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_AudioDecoderStartSettings programSettings;
    bool opened;
    bool started;                   /* Has the channel been started by the app? */
    bool running;                   /* Is the channel actually active? (enabled && started) */
    bool trickMute;                 /* decoder muted (stopped) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool trickForceStopped;         /* decoder muted (stopped) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool channelStatusValid;        /* Is the channel status valid yet? */
    bool isDss;
    bool isPlayback;
    bool tsmPermitted;
    bool tsmEnabled;
    bool locked;
    unsigned fakeFramesDecoded; /* dummy count increments while decoding. */
    BRAP_DEC_AudioParams audioParams;
    BRAP_ChannelHandle rapChannel;
    NEXUS_RaveHandle raveContext;
    NEXUS_RaveHandle savedRaveContext;
    BKNI_EventHandle sampleRateEvent;
    BKNI_EventHandle sourceChangedEvent;
    BKNI_EventHandle layerChangeEvent;
    BKNI_EventHandle channelChangeReportEvent;
    NEXUS_EventCallbackHandle layerChangeCallback;
    NEXUS_EventCallbackHandle channelChangeReportEventHandler;
    struct
    {
        NEXUS_StcChannelDecoderConnectionHandle connector;
        unsigned priority;
        BKNI_EventHandle statusChangeEvent;
        NEXUS_EventCallbackHandle statusChangeEventHandler;
    } stc;
    unsigned ptsErrorCount;
    BRAP_DSPCHN_SampleRateChangeInfo sampleRateInfo;
    NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioInputObject connectors[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioDecoderTrickState trickState;
    NEXUS_AudioInputSyncSettings syncSettings;
    bool syncMute;                 /* decoder muted (while running) by request from the sync module (Keep this unconditional for sake of simplicity) */
#if NEXUS_HAS_ASTM
    struct
    {
        bool permitted;
        NEXUS_AudioDecoderAstmSettings settings;
        NEXUS_AudioDecoderAstmStatus status;
    } astm;
#endif
    NEXUS_AudioOutputHandle hdmiOutput;
    bool mono_to_all;
} NEXUS_AudioDecoder;

/***************************************************************************
Summary:
    Start the audio decoder using the current program settings.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Start(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Start the audio decoder using the current program settings.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Stop(
    NEXUS_AudioDecoderHandle handle,
    bool flush
    );

/***************************************************************************
Summary:
    Determine if a decoder is running.
 ***************************************************************************/
bool NEXUS_AudioDecoder_P_IsRunning(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Retrieve the raptor channel for a decoder object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_AudioDecoder_P_GetChannel(
    NEXUS_AudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Set the decoder's mute state for trick modes
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_SetTrickMute(
    NEXUS_AudioDecoderHandle decoder,
    bool mute
    );

/***************************************************************************
Summary:
    Set the decoder's mute state for sync channel
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_SetSyncMute(
    NEXUS_AudioDecoderHandle decoder,
    bool mute
    );

#if NEXUS_NUM_AUDIO_PLAYBACKS
/***************************************************************************
Summary:
    Determine if a playback channel is running.
 ***************************************************************************/
bool NEXUS_AudioPlayback_P_IsRunning(
    NEXUS_AudioPlaybackHandle handle
    );

/***************************************************************************
Summary:
    Retrieve the raptor channel for a playback channel.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_AudioPlayback_P_GetChannel(
    NEXUS_AudioPlaybackHandle handle
    );
#endif

/***************************************************************************
Summary:
Force a particular input to stop

Description:
This function should only be used when the system is shutting down to
avoid clobbering application state.
 ***************************************************************************/
void NEXUS_AudioInput_P_ForceStop(NEXUS_AudioInput input);

/***************************************************************************
Summary:
    Determine if an input chain is running.  All nodes upstream from the
    specified node will be searched.
 ***************************************************************************/
bool NEXUS_AudioInput_P_IsRunning(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Remove all destinations attached to this object and below.  Used when an
    association will become invalid.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_RemoveDestinations(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Determine format of incoming data.  All nodes upstream from the specified
    node will be searched until a format is found.
 ***************************************************************************/
NEXUS_AudioInputFormat NEXUS_AudioInput_P_GetFormat(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Add an input to the specified connector.  Sanity checks in terms of how
    many inputs are supported, etc. should be done by the caller.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_AddInput(
    NEXUS_AudioInput destination,
    NEXUS_AudioInput source);

/***************************************************************************
Summary:
    Remove all inputs to the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_RemoveAllInputs(
    NEXUS_AudioInput destination
    );

/***************************************************************************
Summary:
    Remove a single input from the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_RemoveInput(NEXUS_AudioInput destination, NEXUS_AudioInput source);

/***************************************************************************
Summary:
    Determine if the specified source is an input to the destination
    connector.
 ***************************************************************************/
bool NEXUS_AudioInput_P_IsConnected(
    NEXUS_AudioInput destination,
    NEXUS_AudioInput source
    );

/***************************************************************************
Summary:
    Find the raptor channel handle for the specified connector.  If a mixer
    is present in the stream or there is no actual input connected, the
    result will be NULL.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_AudioInput_P_GetChannel(
    NEXUS_AudioInput input,
    NEXUS_AudioInput *pChannelInput  /* Actual input node with the channel.  Optional, pass NULL if not interested */
    );

/***************************************************************************
Summary:
    Retrieve all outputs downstream from this connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_GetOutputs(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputList *pOutputList,
    bool directOnly                          /* If true, only outputs connected to this channel will be returned.
                                               If false, all outputs downstream will be returned, including those
                                               attached to downstream mixers. */
    );

/***************************************************************************
Summary:
    Set connection-specific data for the binding between the specified
    source and destination connectors.  The data will be copied and stored
    inside the connector object.  It will be lost when the connection is
    broken.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_SetConnectionData(
    NEXUS_AudioInput destination,
    NEXUS_AudioInput source,
    const void *pData,
    size_t dataSize
    );

/***************************************************************************
Summary:
    Retrieve a pointer to the stored connection data between this source
    and destination.  May be NULL if not previously set.
 ***************************************************************************/
const void *NEXUS_AudioInput_P_GetConnectionData(
    NEXUS_AudioInput destination,
    NEXUS_AudioInput source
    );

/***************************************************************************
Summary:
    Connect an audio output to the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_ConnectOutput(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Disconnect an audio output from the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_DisconnectOutput(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Prepare the input chain to start.  Destinations will be attached
    and downstream connections will be made.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStart(
    NEXUS_AudioInput input
    );
/***************************************************************************
Summary:
    Prepare the input chain to start with some processing added beforehand.
    Destinations will be attached to the provided association, and
    downstream connections will be made.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStartWithProcessing(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Invalidate all downstream destinations
    related to this object.  The channel is being closed or being repurposed.
 ***************************************************************************/
void NEXUS_AudioInput_P_InvalidateConnections(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Notify the input chain of a data format change (e.g. sample rate)
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_FormatChanged(
    NEXUS_AudioInput input,
    NEXUS_AudioInputFormatData *pData
    );

/***************************************************************************
Summary:
    Validate if an output has a valid destination at the specified input.
 ***************************************************************************/
bool NEXUS_AudioInput_P_HasOutputDestination(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Return a downstream mixer handle if present
 ***************************************************************************/
NEXUS_AudioMixerHandle NEXUS_AudioInput_P_GetMixer(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Determine if an output must be slaved to a DAC
Description:
    Some outputs (i.e. RFM) are slaves to a DAC.  AudioInput will find the
    proper DAC to bind the output to and provide it in a call to SetSlaveSource
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsDacSlave(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    For slave outputs, set the DAC source
Description:
    Some outputs (i.e. RFM) are slaves to a DAC.  Mixer will find the proper
    DAC to bind the output to and provide it here.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetSlaveSource(
    NEXUS_AudioOutputHandle slaveHandle,
    NEXUS_AudioOutputHandle sourceHandle);

/***************************************************************************
Summary:
    Apply output-specific settings to the raptor structure
Description:
    Mixer will setup the raptor output port settings initially and then
    hand this structure off to the individual outputs to make their
    adjustments.  It requires knowledge of raptor structures, but no
    actual raptor function calls.  This is required for things like LR swap
    because some part of the logic is in a downmix mode and some is in the
    DAC settings.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_AdjustRapSettings(
    NEXUS_AudioOutputHandle handle,
    BRAP_OutputSettings *pSettings    /* [in/out] */
    );

/***************************************************************************
Summary:
    Inform an audio output of a sample rate change
Description:
    Certain outputs such as HDMI require notification of sample rate changes
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SampleRateChange_isr(
    NEXUS_AudioOutputHandle output,
    BAVC_AudioSamplingRate sampleRate
    );

/***************************************************************************
Summary:
    Apply some settings once an output port is connected.
Description:
    Some settings such as volume/mute can not be applied until an output is
    connected.  This routine will be called once
    AudioInput creates the raptor destination for this output in order
    to delay application of the settings until the correct time.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_ApplySettings(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Apply delay settings and other output port configuration
    before an output port is added.
Description:
    Independent delay settings can not be applied after an output port is
    connected.  This routine will be called before NEXUS_AudioDecoder_P_Start()
    adds the output port in order apply the settings at the correct time.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_ApplyConfig(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Null out the independent delay settings.
Description:
    Independent delay settings should be nulled out when a port is removed,
    or they may interfere with setting them on the same or another port.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_ZeroDelay(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Mute or un-mute the decoder for trick operations
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetTrickMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    );
/***************************************************************************
Summary:
    Mute or un-mute an output for sync channel
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetSyncMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    );


/***************************************************************************
Summary:
    Set the output PLL to be used for this output.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetPll(
    NEXUS_AudioOutputHandle output,
    unsigned pll
    );

/***************************************************************************
Summary:
    Enable the HBR mode on the output port
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetHbrMode(
    NEXUS_AudioOutputHandle output,
    bool bEnableHbr
    );

/***************************************************************************
Summary:
    Determine if an output is connected to any inputs
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsConnected(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Unlink from a parent channel
 ***************************************************************************/
void NEXUS_AudioCapture_P_Disconnect(
    NEXUS_AudioCaptureHandle handle
    );


/***************************************************************************
Summary:
    Set the connection to enable capture start/stop.
 ***************************************************************************/
void NEXUS_AudioCapture_P_SetConnection(
    NEXUS_AudioCaptureHandle handle,
    bool connect
    );

/***************************************************************************
Summary:
    Apply output-specific settings to the raptor structure
Description:
    Mixer will setup the raptor output port settings initially and then
    hand this structure off to the individual outputs to make their
    adjustments.  It requires knowledge of raptor structures, but no
    actual raptor function calls.  This is required for things like LR swap
    because some part of the logic is in a downmix mode and some is in the
    DAC settings.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDac_P_AdjustRapSettings(
    NEXUS_AudioDacHandle handle,
    BRAP_OutputSettings *pSettings    /* [in/out] */
    );

#if NEXUS_NUM_SPDIF_OUTPUTS
/***************************************************************************
Summary:
    Apply output-specific settings to the raptor structure
Description:
    Mixer will setup the raptor output port settings initially and then
    hand this structure off to the individual outputs to make their
    adjustments.  It requires knowledge of raptor structures, but no
    actual raptor function calls.  This is required for things like LR swap
    because some part of the logic is in a downmix mode and some is in the
    DAC settings.
 ***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_P_AdjustRapSettings(
    NEXUS_SpdifOutputHandle handle,
    BRAP_OutputSettings *pSettings    /* [in/out] */
    );
#endif

#if NEXUS_NUM_I2S_OUTPUTS
/***************************************************************************
Summary:
    Apply output-specific settings to the raptor structure
Description:
    Mixer will setup the raptor output port settings initially and then
    hand this structure off to the individual outputs to make their
    adjustments.  It requires knowledge of raptor structures, but no
    actual raptor function calls.  This is required for things like LR swap
    because some part of the logic is in a downmix mode and some is in the
    DAC settings.
 ***************************************************************************/
NEXUS_Error NEXUS_I2sOutput_P_AdjustRapSettings(
    NEXUS_I2sOutputHandle handle,
    BRAP_OutputSettings *pSettings    /* [in/out] */
    );
#endif

#if NEXUS_NUM_I2S_INPUTS
/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_I2sInput_P_IsRunning(
    NEXUS_I2sInputHandle handle
    );

/***************************************************************************
Summary:
    Retrieve the raptor channel for an i2s input object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_I2sInput_P_GetChannel(
    NEXUS_I2sInputHandle handle
    );

#endif

#if NEXUS_NUM_ANALOG_AUDIO_INPUTS
BDBG_OBJECT_ID_DECLARE(NEXUS_AnalogAudioInput);

typedef struct NEXUS_AnalogAudioInput
{
    BDBG_OBJECT(NEXUS_AnalogAudioInput)
    NEXUS_AnalogAudioInputSettings settings;
    NEXUS_AudioInputObject connector;
} NEXUS_AnalogAudioInput;
#endif

#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_AnalogAudioDecoder_P_IsRunning(
    NEXUS_AnalogAudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Retrieve the raptor channel for an analog decoder object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_AnalogAudioDecoder_P_GetChannel(
    NEXUS_AnalogAudioDecoderHandle handle
    );
#endif


/***************************************************************************
Summary:
    Init the DAC module -- No uninit is required.
***************************************************************************/
NEXUS_Error NEXUS_AudioDac_P_Init(void);

/***************************************************************************
Summary:
    Init the SPDIF module -- No uninit is required.
***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_P_Init(void);

/***************************************************************************
Summary:
    Init the Decoder module
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Init(bool *pbSupportAlgos);

/***************************************************************************
Summary:
    Un-Init the Decoder module
***************************************************************************/
void NEXUS_AudioDecoder_P_Uninit(void);

/***************************************************************************
Summary:
    Inform an audio output of a sample rate change
Description:
    Certain outputs such as HDMI require notification of sample rate changes
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SampleRateChange_isr(
    NEXUS_AudioOutputHandle output,
    BAVC_AudioSamplingRate sampleRate
    );

void NEXUS_AudioDecoder_P_TrickReset( NEXUS_AudioDecoderHandle decoder);
void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle decoder);
int32_t NEXUS_AudioModule_P_Vol2Magnum(NEXUS_AudioVolumeType type, int32_t volume);

/***************************************************************************
Summary:
    Set audio parameters for an HDMI output
Description:
    Set number of audio channels and compressed / PCM.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetHDMI(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputFormat format
    );

/***************************************************************************
Summary:
    Initialize SRS TruVolume processing.
Description:
    Initialize tracking of SRS TruVolume stages for clean-up at uninit.
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_P_Init(void);

/***************************************************************************
Summary:
    Uninitialize SRS TruVolume processing.
Description:
    Clean-up SRS TruVolume stages.
 ***************************************************************************/
void NEXUS_TruVolume_P_Uninit(void);

/***************************************************************************
Summary:
    Initialize Dolby Volume processing.
Description:
    Initialize tracking of Dolby stages for clean-up at uninit.
 ***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_P_Init(void);

/***************************************************************************
Summary:
    Uninitialize Dolby Volume processing.
Description:
    Clean-up Dolby Volume stages.
 ***************************************************************************/
void NEXUS_DolbyVolume_P_Uninit(void);

NEXUS_Error NEXUS_AudioDecoder_P_ConfigureRave(NEXUS_AudioDecoderHandle handle, NEXUS_RaveHandle rave, const NEXUS_AudioDecoderStartSettings *pProgram, NEXUS_PidChannelStatus *pPidChannelStatus);
#endif /* #ifndef NEXUS_AUDIO_PRIV_H__ */

