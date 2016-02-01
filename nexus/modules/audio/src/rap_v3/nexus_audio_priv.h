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
#if NEXUS_NUM_RFM_OUTPUTS
#include "priv/nexus_rfm_priv.h"
#endif
#if NEXUS_NUM_I2S_INPUTS
#include "nexus_i2s_input.h"
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
#include "nexus_rf_audio_decoder.h"
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
#include "nexus_audio_playback.h"
#endif
#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
#include "nexus_analog_audio_decoder.h"
#endif
#if NEXUS_NUM_ANALOG_AUDIO_INPUTS
#include "nexus_analog_audio_input.h"
#endif

#if NEXUS_NUM_HDMI_INPUTS || NEXUS_NUM_SPDIF_INPUTS
#define NEXUS_HAS_DIGITAL_AUDIO_INPUTS 1
#endif

#define NEXUS_GLOBAL_INDEPENDENT_VOLUME 1

#if NEXUS_HAS_AUDIO_POST_PROCESSING
#include "nexus_bbe.h"
#include "nexus_pro_logic_2.h"
#include "nexus_mono_downmix.h"
#include "nexus_xen.h"
#include "nexus_audyssey_abx.h"
#include "nexus_audyssey_adv.h"
#include "nexus_studio_sound.h"
#if NEXUS_CVOICE
#include "nexus_custom_voice.h"
#endif
#if NEXUS_CSURROUND
#include "nexus_custom_surround.h"
#endif
#if NEXUS_CBASS
#include "nexus_custom_bass.h"
#endif
#if NEXUS_CUSTOM_AVL
#include "nexus_custom_avl.h"
#endif
#if NEXUS_CUSTOM_ACF
#include "nexus_custom_acf.h"
#endif
#if NEXUS_CUSTOM_DBE
#include "nexus_custom_dbe.h"
#endif
#endif

#include "nexus_dts_encode.h"
#include "nexus_ac3_encode.h"
#include "nexus_tru_volume.h"
#include "nexus_audio_encoder.h"
#include "nexus_auto_volume_level.h"
#include "nexus_3d_surround.h"
#if BRAP_VER <= 3
#define NEXUS_HAS_MS11_SUPPORT 1
#endif
#include "nexus_rf_audio_encoder.h"
#include "nexus_dolby_volume.h"
#include "nexus_dolby_digital_reencode.h"

#if (BCHP_CHIP == 35230 || BCHP_CHIP == 35125 || BCHP_CHIP == 35233)
#include "nexus_audio_return_channel.h"
#include "nexus_sbc_encode.h"
#include "bana.h"
#endif


#if BRAP_VER >= 4
#include "brap_processing.h"
#if BCHP_CHIP!=7344 && BCHP_CHIP!=7346 && BCHP_CHIP!=7231 && BCHP_CHIP!=7358 && BCHP_CHIP!=7552
#include "brap_spdifrx.h"
#endif
#include "nexus_audio_mux_output.h"

/* Add defines due to PI renamed in RAP */
#define BRAP_PB_GetBufInfo            BRAP_GetInputBufferInfo
#define BRAP_PB_UpdateBufUsg          BRAP_UpdateInputBufferUsage
#define BRAP_PB_BufferWriteDone       BRAP_InputBufferWriteDone
#define BRAP_RingBufferReadDone       BRAP_DestinationBufferReadDone
#define BRAP_RingBufferReadDone_isr   BRAP_DestinationBufferReadDone_isr
#define BRAP_GetRingBufInfo           BRAP_GetDestinationBufferInfo
#define BRAP_GetRingBufInfo_isr       BRAP_GetDestinationBufferInfo_isr
#define BRAP_UpdateRingBufUsg         BRAP_UpdateDestinationBufferUsage
#define BRAP_UpdateRingBufUsg_isr     BRAP_UpdateDestinationBufferUsage_isr
#define BRAP_PcmBufInfo               BRAP_BufferInfo
#define BRAP_PcmBufSzUpdt             BRAP_BufferSizeUpdate

#define BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP  BRAP_MAX_CHANNELS_IN_ASSOCIATION
#endif

#include "nexus_audio_capture.h"

extern BRAP_ProcessingStageSettings g_NEXUS_StageSettings;

/***************************************************************************
Summary:
    Audio association handle.  Used for internal resource tracking.
 ***************************************************************************/
typedef struct NEXUS_AudioAssociation *NEXUS_AudioAssociationHandle;

/***************************************************************************
Summary:
    Audio decoder object
 ***************************************************************************/
typedef struct NEXUS_AudioDecoder
{
    NEXUS_OBJECT(NEXUS_AudioDecoder);
    unsigned index;
    NEXUS_AudioDecoderSettings settings;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_AudioDecoderStartSettings programSettings;
    bool opened;
    bool started;                   /* Has the channel been started by the app? */
    bool suspended;
    bool running;                   /* Is the channel actually active? (enabled && started) */
    bool trickMute;                 /* decoder muted (while running) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool trickForceStopped;         /* decoder muted (stopped) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool isDss;
    bool isPlayback;
    bool tsmPermitted;
    bool tsmEnabled;
    bool isDescriptorDecoder;
    bool locked;
    unsigned numFifoOverflows;  /* Total number of FIFO overflows since start was called. */
    unsigned numFifoUnderflows; /* Total number of FIFO underflows since start was called. */
    BRAP_ChannelParams audioParams;
    BRAP_ChannelHandle rapChannel;
    NEXUS_AudioAssociationHandle association;
    NEXUS_RaveHandle raveContext;
    NEXUS_RaveHandle savedRaveContext;
    BKNI_EventHandle sampleRateEvent;
    BKNI_EventHandle sourceChangedEvent;
    BKNI_EventHandle layerChangeEvent;
    BKNI_EventHandle channelChangeReportEvent;
    NEXUS_EventCallbackHandle layerChangeCallback;
    NEXUS_EventCallbackHandle sourceChangeCallback;
    NEXUS_EventCallbackHandle sampleRateCallback;
    NEXUS_EventCallbackHandle channelChangeReportEventHandler;
    NEXUS_IsrCallbackHandle lockCallback;
    NEXUS_IsrCallbackHandle firstPtsCallback;
    NEXUS_IsrCallbackHandle ptsErrorCallback;
    NEXUS_IsrCallbackHandle overflowCallback;
    NEXUS_IsrCallbackHandle underflowCallback;
    NEXUS_TaskCallbackHandle streamInfoCallback;
    NEXUS_TimerHandle statusCheckTimer;
#if BRAP_VER < 4
    BRAP_GainControl fullVolume, muteVolume;
#endif
    struct
    {
        NEXUS_StcChannelDecoderConnectionHandle connector;
        unsigned priority;
        BKNI_EventHandle statusChangeEvent;
        NEXUS_EventCallbackHandle statusChangeEventHandler;
    } stc;
    unsigned ptsErrorCount;
    BRAP_DSPCHN_SampleRateChangeInfo sampleRateInfo;
    NEXUS_AudioInputFormat compressedFormat;
    NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioInputObject connectors[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioDecoderTrickState trickState;
#if NEXUS_HAS_SYNC_CHANNEL
    struct
    {
        NEXUS_AudioInputSyncSettings settings;
        bool mute;                 /* decoder muted (while running) by request from the sync module (Keep this unconditional for sake of simplicity) */
        bool startMuted;
    } sync;
#endif
#if NEXUS_HAS_ASTM
    struct
    {
        bool permitted;
        NEXUS_AudioDecoderAstmSettings settings;
        NEXUS_AudioDecoderAstmStatus status;
    } astm;
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    struct
    {
        NEXUS_AudioDecoderSimpleSettings settings;
    } simple;
#endif
    NEXUS_TaskCallbackHandle sourceChangeAppCallback;
    BRAP_ProcessingStageHandle converter;
    NEXUS_AudioDecoderHandle descriptorParent;
#if BRAP_VER < 4
    BRAP_AudioDescriptorFadeHandle descriptorTable;
#endif
    BRAP_ProcessingStageHandle descriptorProcessor;
    NEXUS_AudioDecoderHandle passthroughParent;
    bool dualChannel;   /* one vs. two-channel passthrough operation.  If true, compressed outputs are actually attached
                           to the passthrough decoder association instead of the primary.  If false, compressed outputs are
                           attached to the primary channel as expected. */

    bool enableStereoDownmixPath;
    struct
    {
        unsigned staticCount;
        uint32_t lastCdbValidPointer;
        uint32_t lastCdbReadPointer;
        NEXUS_TimerHandle timer;
        NEXUS_StcChannelDecoderFifoWatchdogStatus status;
    } fifoWatchdog;
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
    Set the decoder's compressed outputs mute state
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_SetCompressedMute(
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
    bool directOnly                         /* If true, only outputs connected to this channel will be returned.
                                               If false, all outputs downstream will be returned, including those
                                               attached to downstream mixers.
                                            */
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
    Prepare the input chain to start.  Destinations will be attached to
    the provided association, and downstream connections will be made.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStart(
    NEXUS_AudioInput input,
    NEXUS_AudioAssociationHandle association
    );

/***************************************************************************
Summary:
    Prepare the input chain to start with some processing added beforehand.
    Destinations will be attached to the provided association, and
    downstream connections will be made.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStartWithProcessing(
    NEXUS_AudioInput input,
    NEXUS_AudioAssociationHandle association,
    BRAP_ProcessingStageHandle *pStages,
    int numStages,
    BRAP_ProcessingStageHandle descriptorStage,
    BRAP_ChannelHandle descriptorPair,
    bool downmix,
    NEXUS_AudioCodec codec
    );

/***************************************************************************
Summary:
    Invalidate all downstream destinations and any mixer associations
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
BRAP_DestinationHandle NEXUS_AudioInput_P_GetOutputDestination(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Get association settings for an object.  This is used by mixer to
    to determine all upstream raptor channels.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_GetAssociationSettings(
    NEXUS_AudioInput input,
    BRAP_AssociatedChanSettings *pSettings
    );

/***************************************************************************
Summary:
    Returns the first object downstream from the current object that matches
    the specified type.  This is a depth-first search, not breadth-first.
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInput_P_FindByType(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    );

/***************************************************************************
Summary:
    Returns the first object upstream from the current object that matches
    the specified type.  This is a depth-first search, not breadth-first.
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInput_P_FindUpstream(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    );

/***************************************************************************
Summary:
    Search for an object directly connected to this node.
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInput_P_FindDirect(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    );

/***************************************************************************
Summary:
    Get number of direct connections to a node
 ***************************************************************************/
unsigned NEXUS_AudioInput_P_GetNumDirectConnections(
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
    Return a downstream mixer handle if present
 ***************************************************************************/
BRAP_DestinationHandle NEXUS_AudioInput_P_GetOutputDestination(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle
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
    Determine if an output must be slaved to a SPDIF output
Description:
    Audio Return Channel are slaves to SPDIF outputs.  AudioInput will find the
    proper SPDIF to bind the output to and provide it in a call to SetSlaveSource
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsSpdifSlave(
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
    For Audio Return Channel slave outputs, set the SPDIF source
Description:
    ARC outputs are slaves to SPDIF outputs.  Mixer will find the proper
    SPDIF to bind the output to and provide it here.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetArcSlaveSource(
    NEXUS_AudioOutputHandle slaveHandle,
    NEXUS_AudioOutputHandle sourceHandle);


/***************************************************************************
Summary:
    Populate a destination details structure for an output
Description:
    The 3563/7405+ raptor requires a structure for an output, not just an
    enum value.  This is populated by the AudioOutput code per-output type.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_GetDestinationDetails(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo,        /* [in/out] */
    BRAP_DstDetails *pPrivateDestInfo  /* [in/out] */
    );

/***************************************************************************
Summary:
    Apply some settings once an output is connected to an association.
Description:
    Some settings such as volume/mute can not be applied until an output is
    connected to an association.  This routine will be called once
    AudioInput creates the raptor destination for this output in order
    to delay application of the settings until the correct time.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_ApplySettings(
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
    Mute or un-mute the compressesed outputs for decoder mute operations
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetCompressedMute(
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
    Get the input connector feeding this object
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioOutput_P_GetInput(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Retrieve the destination handle for an output
 ***************************************************************************/
BRAP_DestinationHandle NEXUS_AudioOutput_P_GetDestination(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Notify an output that it's destination has been added or removed
 ***************************************************************************/
void NEXUS_AudioOutput_P_SetDestination(
    NEXUS_AudioOutputHandle output,
    BRAP_DestinationHandle destination
    );

#if NEXUS_NUM_HDMI_OUTPUTS > 0
#if BCHP_CHIP == 7420 || BCHP_CHIP == 7342 || BCHP_CHIP == 7340 || BCHP_CHIP == 7125 || BCHP_CHIP == 7468 || BCHP_CHIP == 7550 || BRAP_VER >= 4
#define NEXUS_HDMI_MULTI_ON_MAI 1
#define NEXUS_HDMI_MULTI_PORT_0 BRAP_OutputPort_eMaiMulti0
#define NEXUS_HDMI_MULTI_PORT_1 BRAP_OutputPort_eMaiMulti1
#define NEXUS_HDMI_MULTI_PORT_2 BRAP_OutputPort_eMaiMulti2
#define NEXUS_HDMI_MULTI_PORT_3 BRAP_OutputPort_eMaiMulti3
#elif BCHP_CHIP == 7405 || BCHP_CHIP == 7325 || BCHP_CHIP == 7335
#define NEXUS_HDMI_MULTI_ON_I2S 1
#define NEXUS_HDMI_MULTI_PORT_0 BRAP_OutputPort_eI2s0
#define NEXUS_HDMI_MULTI_PORT_1 BRAP_OutputPort_eI2s1
#define NEXUS_HDMI_MULTI_PORT_2 BRAP_OutputPort_eI2s2
#define NEXUS_HDMI_MULTI_PORT_3 BRAP_OutputPort_eI2s3
#else
#error Need to define HDMI multichannel format for this chip.
#endif

/***************************************************************************
Summary:
    Initialize HDMI output settings at module startup
 ***************************************************************************/
void NEXUS_AudioOutput_P_InitHdmiOutput(void);
#endif

/***************************************************************************
Summary:
    Get the processing stage required to generate a particular output's audio
    data.  Used typically for MUX output, also possibly for BTSC on some DTA
    chips.
 ***************************************************************************/
BRAP_ProcessingStageHandle NEXUS_AudioOutput_P_GetProcessingStage(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Get Destination Details for an audio output group
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutputGroup_P_GetDestinationDetails(
    NEXUS_AudioOutputGroupHandle handle,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo
    );

/***************************************************************************
Summary:
    Set Destination for an output group
 ***************************************************************************/
void NEXUS_AudioOutputGroup_P_SetDestination(
    NEXUS_AudioOutputGroupHandle handle,
    BRAP_DestinationHandle destination
    );

#if NEXUS_HAS_AUDIO_MUX_OUTPUT
/***************************************************************************
Summary:
    Get Destination Details for an audio mux output
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_P_GetDestinationDetails(
    NEXUS_AudioMuxOutputHandle handle,
    BRAP_DstDetails *pDestInfo
    );

/***************************************************************************
Summary:
    Get the processing stage required for an audio mux output
 ***************************************************************************/
BRAP_ProcessingStageHandle NEXUS_AudioMuxOutput_P_GetProcessingStage(
    NEXUS_AudioMuxOutputHandle handle
    );

/***************************************************************************
Summary:
    Set the destination handle for an audio mux output
 ***************************************************************************/
void NEXUS_AudioMuxOutput_P_SetDestination(
    NEXUS_AudioMuxOutputHandle handle,
    BRAP_DestinationHandle destination
    );

#endif

/***************************************************************************
Summary:
    Get Destination Details for an audio capture object
 ***************************************************************************/
NEXUS_Error NEXUS_AudioCapture_P_GetDestinationDetails(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo
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
    Set the destination to enable capture start/stop.
 ***************************************************************************/
void NEXUS_AudioCapture_P_SetDestination(
    NEXUS_AudioCaptureHandle handle,
    BRAP_DestinationHandle destination
    );

/***************************************************************************
Summary:
    Create an audio association.  Will internally reference count multiple
    requests for the same linkage.
 ***************************************************************************/
NEXUS_AudioAssociationHandle NEXUS_AudioAssociation_P_Create(
    BRAP_AssociatedChanSettings *pSettings
    );

/***************************************************************************
Summary:
    Destroy an audio association.
 ***************************************************************************/
void NEXUS_AudioAssociation_P_Destroy(
    NEXUS_AudioAssociationHandle handle
    );

/***************************************************************************
Summary:
    Add a nexus audio output to an association
 ***************************************************************************/
NEXUS_Error NEXUS_AudioAssociation_P_AddOutput(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioInputFormat format,
    NEXUS_AudioOutputHandle output,
    bool downmix
    );

/***************************************************************************
Summary:
    Remove a nexus audio output from an association
 ***************************************************************************/
void NEXUS_AudioAssociation_P_RemoveOutput(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Get the destination handle for an output (used for post-processing)
 ***************************************************************************/
BRAP_DestinationHandle NEXUS_AudioAssociation_P_GetOutputDestination(
    NEXUS_AudioAssociationHandle handle,
    NEXUS_AudioOutputHandle output,
    BRAP_DestinationHandle *pPrivDestination
    );

#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
/***************************************************************************
Summary:
    Determine the Audio Return Channel index (mapped to Hdmi input port).
 ***************************************************************************/
void NEXUS_AudioReturnChannel_P_GetIndex(
    NEXUS_AudioReturnChannelHandle handle,
    unsigned *pIndex );

/***************************************************************************
Summary: Set the Audio Return Channel source (a RAP SPDIF output port identifier) stored
                in the ARC handle
 ***************************************************************************/
void NEXUS_AudioReturnChannel_P_SetSourceId(
    NEXUS_AudioReturnChannelHandle handle,
    BRAP_OutputPort      spdifOutputPort
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

#if NEXUS_NUM_RF_AUDIO_DECODERS
#if BCHP_CHIP == 3556
#define NEXUS_HAS_AASD 1
#define NEXUS_CONNECT_RFAUDIO_AND_FRONTEND 1
#endif
#if BCHP_CHIP == 35230 || BCHP_CHIP == 35125 || BCHP_CHIP == 35126 || BCHP_CHIP == 35233
#if NEXUS_HAS_FRONTEND
#define NEXUS_CONNECT_RFAUDIO_AND_FRONTEND 1
#endif
#endif

#if NEXUS_CONNECT_RFAUDIO_AND_FRONTEND
#include "nexus_frontend.h"
#endif

BDBG_OBJECT_ID_DECLARE(NEXUS_RfAudioDecoder);
typedef struct NEXUS_RfAudioDecoder
{
    BDBG_OBJECT(NEXUS_RfAudioDecoder)
    bool started;
    bool running;
    bool opened;
    unsigned outputScaling;
    NEXUS_RfAudioDecoderSettings settings;
    BRAP_ChannelHandle rapChannel;
    NEXUS_AudioAssociationHandle association;
    NEXUS_AudioInputObject connector;
    NEXUS_RfAudioDecoderMode currentMode;
    void *pStatusData;
    #if NEXUS_HAS_AASD
    bool isSecam;
    bool carrier1Present;
    bool a2Present;
    bool nicamPresent;
    int aasdIndex;
    NEXUS_VideoFormat ifdVideoFormat;
    NEXUS_IfdAudioMode ifdAudioMode;
    enum {RFAUDIO_WAIT_LOCK, RFAUDIO_CARRIER1_SCAN, RFAUDIO_CARRIER2_SCAN, RFAUDIO_LOCKED} currentState;
    unsigned nicamErrors;
    #endif
    BKNI_EventHandle statusEvent, stopEvent, stopAckEvent;
    NEXUS_TaskCallbackHandle statusChangedCallback;
#if (BCHP_CHIP == 35230 || BCHP_CHIP == 35125 || BCHP_CHIP == 35233)
    NEXUS_TaskCallbackHandle standardDetectedCallback;
    BANA_Handle hAna;
    bool runMagShiftThread;
    NEXUS_ThreadHandle magShiftThread;
    BKNI_EventHandle magShiftChangeEvent;
    bool runAudioClipThread;
    NEXUS_ThreadHandle audioClipThread;
    BKNI_EventHandle audioClipEvent;
#endif
} NEXUS_RfAudioDecoder;

/***************************************************************************
Summary:
    Init the RF audio decoder sub-module
 ***************************************************************************/
NEXUS_Error NEXUS_RfAudioDecoder_P_Init(void);

/***************************************************************************
Summary:
    Un-Init the RF audio decoder sub-module
 ***************************************************************************/
void NEXUS_RfAudioDecoder_P_Uninit(void);

/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_RfAudioDecoder_P_IsRunning(
    NEXUS_RfAudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Retrieve the raptor channel for an rf decoder object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_RfAudioDecoder_P_GetChannel(
    NEXUS_RfAudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Initialize RF Audio decoder status
 ***************************************************************************/
NEXUS_Error NEXUS_RfAudioDecoder_P_InitStatus(
    NEXUS_RfAudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Uninitialize RF Audio decoder status
 ***************************************************************************/
void NEXUS_RfAudioDecoder_P_UninitStatus(
    NEXUS_RfAudioDecoderHandle handle
    );

/***************************************************************************
Summary:
    Reset A2 Primary Carrier Status on channel change
 ***************************************************************************/
void NEXUS_RfAudioDecoder_P_ResetStatus(
    NEXUS_RfAudioDecoderHandle handle,
    bool retune
    );

#if NEXUS_HAS_AASD
BRAP_RfAudioStandard NEXUS_RfAudioDecoder_P_GetAasdRapMode(
    NEXUS_RfAudioDecoderHandle handle,
    int index
    );
#endif
#endif

#if NEXUS_NUM_AUDIO_CAPTURE_CHANNELS

/***************************************************************************
Summary:
    Init the capture channel manager.
***************************************************************************/
NEXUS_Error NEXUS_AudioCaptureChannel_P_Init(void);

/***************************************************************************
Summary:
    Uninit the capture channel manager.
***************************************************************************/
NEXUS_Error NEXUS_AudioCaptureChannel_P_Uninit(void);

/***************************************************************************
Summary:
    Acquire a channel from the resource manager.
***************************************************************************/
#define NEXUS_AudioCaptureChannel_P_Acquire() NEXUS_AudioCaptureChannel_P_Acquire_tagged(__FILE__,__LINE__)
BRAP_ChannelHandle NEXUS_AudioCaptureChannel_P_Acquire_tagged(const char *file, int line);

/***************************************************************************
Summary:
    Release an acquired channel back to the resource manager.
***************************************************************************/
#define NEXUS_AudioCaptureChannel_P_Release(h) NEXUS_AudioCaptureChannel_P_Release_tagged((h), __FILE__,__LINE__)
void NEXUS_AudioCaptureChannel_P_Release_tagged(BRAP_ChannelHandle handle, const char *file, int line);
#endif

/***************************************************************************
Summary:
    Set volume of a capture channel
***************************************************************************/
NEXUS_Error NEXUS_AudioCaptureChannel_P_SetVolume(BRAP_ChannelHandle handle, int32_t left, int32_t right, bool mute);

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
NEXUS_Error NEXUS_AudioDecoder_P_Init(void);

/***************************************************************************
Summary:
    Un-Init the Decoder module
***************************************************************************/
void NEXUS_AudioDecoder_P_Uninit(void);

/***************************************************************************
Summary:
    Post-processing interfaces
***************************************************************************/
#if NEXUS_HAS_AUDIO_POST_PROCESSING
BRAP_ProcessingStageHandle NEXUS_Bbe_P_GetStageHandle(NEXUS_BbeHandle handle);
BRAP_ProcessingStageHandle NEXUS_Xen_P_GetStageHandle(NEXUS_XenHandle handle);
BRAP_ProcessingStageHandle NEXUS_ProLogic2_P_GetStageHandle(NEXUS_ProLogic2Handle handle);
BRAP_ProcessingStageHandle NEXUS_MonoDownmix_P_GetStageHandle(NEXUS_MonoDownmixHandle handle);
BRAP_ProcessingStageHandle NEXUS_AudysseyAbx_P_GetStageHandle(NEXUS_AudysseyAbxHandle handle);
BRAP_ProcessingStageHandle NEXUS_AudysseyAdv_P_GetStageHandle(NEXUS_AudysseyAdvHandle handle);
BRAP_ProcessingStageHandle NEXUS_StudioSound_P_GetStageHandle(
    NEXUS_StudioSoundHandle handle,
    BRAP_ProcessingStageHandle *pCsTd,
    BRAP_ProcessingStageHandle *pTsHd,
    BRAP_ProcessingStageHandle *pEqHl
    );
#if NEXUS_CVOICE
BRAP_ProcessingStageHandle NEXUS_CustomVoice_P_GetStageHandle(NEXUS_CustomVoiceHandle handle);
#endif
#if NEXUS_CSURROUND
BRAP_ProcessingStageHandle NEXUS_CustomSurround_P_GetStageHandle(NEXUS_CustomSurroundHandle handle);
#endif
#if NEXUS_CBASS
BRAP_ProcessingStageHandle NEXUS_CustomBass_P_GetStageHandle(NEXUS_CustomBassHandle handle);
#endif
#if NEXUS_CUSTOM_AVL
BRAP_ProcessingStageHandle NEXUS_CustomAvl_P_GetStageHandle(NEXUS_CustomAvlHandle handle);
#endif
#if NEXUS_CUSTOM_ACF
BRAP_ProcessingStageHandle NEXUS_CustomAcousticFilter_P_GetStageHandle(NEXUS_CustomAcousticFilterHandle handle);
#endif
#if NEXUS_CUSTOM_DBE
BRAP_ProcessingStageHandle NEXUS_CustomDbe_P_GetStageHandle(NEXUS_CustomDbeHandle handle);
#endif
#endif  /* HAS_AUDIO_POST_PROCSESING */

BRAP_ProcessingStageHandle NEXUS_DtsEncode_P_GetStageHandle(NEXUS_DtsEncodeHandle handle);
BRAP_ProcessingStageHandle NEXUS_Ac3Encode_P_GetStageHandle(NEXUS_Ac3EncodeHandle handle, NEXUS_AudioCodec codec);
BRAP_ProcessingStageHandle NEXUS_TruVolume_P_GetStageHandle(NEXUS_TruVolumeHandle handle);
BRAP_ProcessingStageHandle NEXUS_AudioEncoder_P_GetStageHandle(NEXUS_AudioEncoderHandle handle);
BRAP_ProcessingStageHandle NEXUS_3dSurround_P_GetStageHandle(NEXUS_3dSurroundHandle handle);
BRAP_ProcessingStageHandle NEXUS_AutoVolumeLevel_P_GetStageHandle(NEXUS_AutoVolumeLevelHandle handle);

NEXUS_Error NEXUS_TruVolume_P_SetSettings(
    BRAP_ProcessingStageHandle stage,
    const NEXUS_TruVolumeSettings *pSettings,
    bool bIsStudioSound,
    void * pProcessingSettings
    );
#if NEXUS_NUM_AUDIO_MIXERS
BRAP_ProcessingStageHandle NEXUS_AudioMixer_P_GetStageHandle(NEXUS_AudioMixerHandle handle);
bool NEXUS_AudioMixer_P_IsSlaveChannel(NEXUS_AudioMixerHandle handle, BRAP_ChannelHandle channel);
bool NEXUS_AudioMixer_P_IsUsingDsp(NEXUS_AudioMixerHandle handle);
#endif

#if BCHP_CHIP != 3548 && BCHP_CHIP != 3556 && BRAP_VER < 4
BRAP_ProcessingStageHandle NEXUS_DolbyVolume_P_GetStageHandle(NEXUS_DolbyVolumeHandle handle);
BRAP_ProcessingStageHandle NEXUS_DolbyVolume258_P_GetStageHandle(NEXUS_DolbyVolume258Handle handle);
BRAP_ProcessingStageHandle NEXUS_DolbyDigitalReencode_P_GetStageHandle(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input,
    BRAP_ProcessingStageHandle *pEncoder
    );
BRAP_ProcessingStageHandle NEXUS_RfAudioEncoder_P_GetStageHandle(NEXUS_RfAudioEncoderHandle handle);
#endif

void NEXUS_AudioDecoder_P_TrickReset(NEXUS_AudioDecoderHandle decoder);
void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle decoder);
bool NEXUS_AudioDecoder_P_IsMultiChannelOutputEnabled(NEXUS_AudioDecoderHandle handle);

/* Codec conversion routines */
BRAP_DSPCHN_AudioType NEXUS_Audio_P_CodecToAudioType(NEXUS_AudioCodec codec);
NEXUS_AudioCodec NEXUS_Audio_P_AudioTypeToCodec(BRAP_DSPCHN_AudioType codec);
BRAP_ProcessingType NEXUS_Audio_P_CodecToProcessingType(NEXUS_AudioCodec codec);

NEXUS_Error NEXUS_AudioDecoder_P_ConfigureRave(NEXUS_AudioDecoderHandle handle, NEXUS_RaveHandle rave, const NEXUS_AudioDecoderStartSettings *pProgram, NEXUS_PidChannelStatus *pPidChannelStatus);

#endif /* #ifndef NEXUS_AUDIO_PRIV_H__ */

