/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef NEXUS_AUDIO_MODULE_H_
#define NEXUS_AUDIO_MODULE_H_

#include "nexus_base.h"
#include "bape.h"
#include "nexus_audio_thunks.h"
#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_init.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_input.h"
#include "nexus_audio_input_capture.h"
#include "nexus_audio_output.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_mux_output.h"
#include "priv/nexus_audio_input_priv.h"
#include "priv/nexus_audio_output_priv.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_playback.h"
#include "priv/nexus_core.h"
#include "priv/nexus_core_audio.h"
#include "nexus_spdif_output.h"
#include "nexus_i2s_input.h"
#include "nexus_i2s_output.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decode_to_memory.h"
#include "nexus_audio_decoder_extra.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_decoder_primer.h"
#include "bape.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "nexus_ac3_encode.h"
#include "nexus_audio_encoder.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"
#include "nexus_dts_encode.h"
#include "nexus_echo_canceller.h"
#include "nexus_tru_volume.h"
#include "nexus_audio.h"
#include "nexus_audio_module.h"
#include "nexus_audio_debug.h"
#include "priv/nexus_audio_standby_priv.h"
#include "nexus_audio_output_group.h"
#include "nexus_audio_dummy_output.h"
#include "nexus_auto_volume_level.h"
#include "nexus_3d_surround.h"
#include "nexus_audio_equalizer.h"
#include "nexus_audio_return_channel.h"
#include "nexus_rf_audio_encoder.h"
#include "nexus_spdif_input.h"
#include "nexus_audio_dsp.h"
#include "nexus_audio_dsp_private.h"
#if NEXUS_CVOICE
#include "nexus_custom_voice.h"
#endif
#if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
#include "nexus_audio_buffer_capture.h"
#endif
#if NEXUS_AUDIO_RAVE_MONITOR_EXT
#include "nexus_audio_rave_monitor.h"
#endif
#if NEXUS_NUM_DSP_VIDEO_DECODERS
#include "nexus_audio_dsp_video_decoder_module.h"
#endif
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "nexus_audio_dsp_video_encoder_module.h"
#endif
#include "nexus_audio_crc.h"
#include "nexus_audio_processor.h"
#include "blst_queue.h"
#include "priv/nexus_audio_image_priv.h"

/***************************************************************************
Summary:
    BAPE Handle and helper macro
***************************************************************************/
#define NEXUS_AUDIO_DEVICE_HANDLE g_NEXUS_audioModuleData.apeHandle

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME audio
#define NEXUS_MODULE_SELF g_NEXUS_audioModule

extern NEXUS_ModuleHandle g_NEXUS_audioModule;

typedef struct NEXUS_AudioModuleData
{
    BDSP_Handle dspHandle;
    BDSP_Handle armHandle;
    BAPE_Handle apeHandle;
    BAPE_DebugHandle debugHandle;
    NEXUS_AudioModuleSettings settings;
    NEXUS_AudioModuleInternalSettings internalSettings;
    NEXUS_AudioCapabilities capabilities;
    BAPE_Capabilities piCapabilities;
    void *pImageContext[BAPE_DEVICE_TYPE_MAX];
    bool watchdogDeferred;
    unsigned numDsps;
    bool verifyFirmware;   /* true if Firmware verifcation is required for Raaga0 or Raaga1  */
    char dspFirmwareVersionInfo[25];  /* Dsp Release Version */
    char armFirmwareVersionInfo[25];  /* Arm Release Version */
    void *pPakContext;
    BIMG_Interface pakImg;
} NEXUS_AudioModuleData;

extern NEXUS_AudioModuleData g_NEXUS_audioModuleData;

/***************************************************************************
Summary:
    Audio encoder object
 ***************************************************************************/
typedef struct NEXUS_AudioEncoder
{
    NEXUS_OBJECT(NEXUS_AudioEncoder);
    BAPE_EncoderHandle encoder;
    NEXUS_AudioInputObject connector;
    NEXUS_AudioInputHandle input;
    char name[8];   /* ENCODER */
} NEXUS_AudioEncoder;

typedef struct NEXUS_AudioDecoderBufferNode
{
    BLST_Q_ENTRY(NEXUS_AudioDecoderBufferNode) node;
    NEXUS_MemoryBlockHandle block;
    unsigned blockOffset;
    NEXUS_Addr memOffset;
} NEXUS_AudioDecoderBufferNode;

/***************************************************************************
Summary:
Input Port Status Information
 ***************************************************************************/
typedef struct NEXUS_AudioInputPortStatus
{
    bool signalPresent;         /* If true, a signal is present */
    bool compressed;            /* If true, stream is compressed.  If false, stream is linear PCM */
    bool hbr;                   /* If true, stream is HBR compressed. */
    NEXUS_AudioCodec codec;     /* Stream Codec */
    unsigned numPcmChannels;    /* Number of PCM channels if codec == NEXUS_AudioCodec_ePcm */
    unsigned sampleRate;        /* Sample Rate in Hz */
} NEXUS_AudioInputPortStatus;

typedef struct {
    NEXUS_RaveHandle source, destination;
} NEXUS_AudioDecoder_RaveConfig;
/***************************************************************************
Summary:
    Audio decoder object
 ***************************************************************************/
typedef struct NEXUS_AudioDecoder
{
    NEXUS_OBJECT(NEXUS_AudioDecoder);
    unsigned index;
    NEXUS_AudioDecoderOpenSettings openSettings;
    NEXUS_AudioDecoderSettings settings;
    NEXUS_AudioDecoderStartSettings programSettings;
    NEXUS_AudioDecoderSpliceSettings spliceSettings;
    NEXUS_AudioDecoderSpliceStatus spliceStatus;
    bool spliceFlowStopped;         /* Was SpliceFlowStop function called? */
    bool started;                   /* Has the channel been started by the app? */
    bool running;                   /* Is the channel actually active? (enabled && started) */
    bool trickMute;                 /* decoder muted (while running) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool trickForceStopped;         /* decoder muted (stopped) by request from the trick modes operations (Keep this unconditional for sake of simplicity) */
    bool isDss;
    bool isPlayback;
    bool tsmPermitted;
    bool tsmEnabled;
    bool isDescriptorDecoder;
    bool locked;
    bool suspended;
    BAPE_DecoderHandle channel;
    BAPE_DecoderStartSettings apeStartSettings;
    bool raveCxtConfigured;
    NEXUS_RaveStatus raveStatus;
    NEXUS_RaveHandle raveContext;
    NEXUS_RaveHandle savedRaveContext;
    NEXUS_AudioDecoderPrimerHandle primer;
    BKNI_EventHandle sampleRateEvent;
    BKNI_EventHandle channelChangeReportEvent;
    BKNI_EventHandle inputFormatChangeEvent;
    NEXUS_EventCallbackHandle sampleRateCallback;
    NEXUS_EventCallbackHandle channelChangeReportEventHandler;
    NEXUS_EventCallbackHandle inputFormatChangeEventHandler;
    NEXUS_IsrCallbackHandle spliceCallback;
    NEXUS_IsrCallbackHandle lockCallback;
    NEXUS_IsrCallbackHandle firstPtsCallback;
    NEXUS_IsrCallbackHandle ptsErrorCallback;
    NEXUS_IsrCallbackHandle fifoOverflowCallback;
    NEXUS_IsrCallbackHandle fifoUnderflowCallback;
    NEXUS_IsrCallbackHandle streamStatusCallback;
    NEXUS_IsrCallbackHandle ancillaryDataCallback;
    NEXUS_IsrCallbackHandle dialnormChangedCallback;
    NEXUS_CallbackDesc bufferCompleteCallbackDesc;
    NEXUS_IsrCallbackHandle bufferCompleteCallback;
    NEXUS_AudioDecoderBufferNode *pBufferNodes;
    BAPE_DecoderBufferDescriptor *pBufferDescriptors;
    unsigned maxDecodeBuffers;
    BLST_Q_HEAD(FreeBufferList, NEXUS_AudioDecoderBufferNode) freeBuffers;
    BLST_Q_HEAD(ActiveBufferList, NEXUS_AudioDecoderBufferNode) activeBuffers;
    struct
    {
        NEXUS_StcChannelDecoderConnectionHandle connector;
        unsigned priority;
        BKNI_EventHandle statusChangeEvent;
        NEXUS_EventCallbackHandle statusChangeEventHandler;
        bool master;
    } stc;
    unsigned ptsErrorCount;
    size_t lastAncillarySize;
    NEXUS_AudioInputPortStatus inputPortStatus;
    NEXUS_AudioInputFormat compressedFormat;
    NEXUS_AudioOutputList outputLists[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioInputObject connectors[NEXUS_AudioDecoderConnectorType_eMax];
    NEXUS_AudioDecoderTrickState trickState;
    struct
    {
        NEXUS_AudioInputSyncSettings settings;
        bool started;
        bool mute;
        bool startMuted;
    } sync;
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
    NEXUS_IsrCallbackHandle sourceChangeAppCallback;
    NEXUS_AudioDecoderHandle descriptorParent;
    struct
    {
        unsigned staticCount;
        uint32_t lastCdbValidPointer;
        uint32_t lastCdbReadPointer;
        NEXUS_TimerHandle timer;
        NEXUS_StcChannelDecoderFifoWatchdogStatus status;
    } fifoWatchdog;
    unsigned numFifoOverflows;
    unsigned numFifoUnderflows;
    char name[11];   /* DECODER %d */
    struct
    {
        BAPE_MixerInputVolume current;
        BAPE_MixerInputVolume next;
    } volume;
    #if NEXUS_AUDIO_RAVE_MONITOR_EXT
    struct {
        NEXUS_AudioRaveMonitorHandle monitor;
        NEXUS_AudioRaveMonitorStartSettings startSettings;
    } raveMonitor;
    #endif
    BAPE_DecoderStatus apeStatus;
} NEXUS_AudioDecoder;

/***************************************************************************
Summary:
    Make sure a NEXUS_AudioInputHandle is suitable for DSP processing
 ***************************************************************************/
#define NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input) \
    do {\
        if ( NULL == (BAPE_Connector)(input)->port ) {\
            if ( (input)->objectType == NEXUS_AudioInputType_eMixer ) {\
                BDBG_ERR(("DSP processing of mixed content requires mixUsingDsp=true in NEXUS_AudioMixerSettings."));\
            } else {\
                BDBG_ERR(("This input type does not support DSP processing."));\
            }\
            return BERR_TRACE(BERR_INVALID_PARAMETER);\
        }\
    } while (0)

/***************************************************************************
Summary:
    Initialize the audio input module
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_Init(void);

/***************************************************************************
Summary:
    Un-Initialize the audio input module
 ***************************************************************************/
void NEXUS_AudioInput_P_Uninit(void);

/***************************************************************************
Summary:
    Determine if an input chain is running.  All nodes upstream from the
    specified node will be searched.
 ***************************************************************************/
bool NEXUS_AudioInput_P_IsRunning(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Determine format of incoming data.  All nodes upstream from the specified
    node will be searched until a format is found.
 ***************************************************************************/
NEXUS_AudioInputFormat NEXUS_AudioInput_P_GetFormat_isrsafe(
    NEXUS_AudioInputHandle input
    );

#define NEXUS_AudioInput_P_GetFormat(input) NEXUS_AudioInput_P_GetFormat_isrsafe(input)

/***************************************************************************
Summary:
    Add an input to the specified connector.  Sanity checks in terms of how
    many inputs are supported, etc. should be done by the caller.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_AddInput(
    NEXUS_AudioInputHandle destination,
    NEXUS_AudioInputHandle source);

/***************************************************************************
Summary:
    Remove all inputs to the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_RemoveAllInputs(
    NEXUS_AudioInputHandle destination
    );

/***************************************************************************
Summary:
    Remove a single input from the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_RemoveInput(NEXUS_AudioInputHandle destination, NEXUS_AudioInputHandle source);

/***************************************************************************
Summary:
    Determine if the specified source is an input to the destination
    connector.
 ***************************************************************************/
bool NEXUS_AudioInput_P_IsConnected(
    NEXUS_AudioInputHandle destination,
    NEXUS_AudioInputHandle source
    );

/***************************************************************************
Summary:
    Retrieve all outputs downstream from this connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_GetOutputs(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioOutputList *pOutputList,
    bool directOnly                         /* If true, only outputs connected to this channel will be returned.
                                               If false, all outputs downstream will be returned, including those
                                               attached to downstream mixers.
                                            */
    );

/***************************************************************************
Summary:
    Connect an audio output to the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_ConnectOutput(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Disconnect an audio output from the specified connector.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_DisconnectOutput(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Prepare the input chain to start, building downstream connections.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStart(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Returns the first object downstream from the current object that matches
    the specified type.  This is a depth-first search, not breadth-first.
 ***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioInput_P_FindByType(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioInputType type
    );

/***************************************************************************
Summary:
    Called when AudioOutputSettings are changed so that mixers can be updated
    accordingly.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_OutputSettingsChanged(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Get the default unity volume
 ***************************************************************************/
void NEXUS_AudioInput_P_GetDefaultVolume(
    BAPE_MixerInputVolume *pInputVolume    /* [out] */
    );

/***************************************************************************
Summary:
    Get the mixer input volume for a particular input
 ***************************************************************************/
void NEXUS_AudioInput_P_GetVolume(
    NEXUS_AudioInputHandle input,
    BAPE_MixerInputVolume *pInputVolume    /* [out] */
    );

/***************************************************************************
Summary:
    Set the mixer input volume for a particular input
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_SetVolume(
    NEXUS_AudioInputHandle input,
    const BAPE_MixerInputVolume *pInputVolume
    );

/***************************************************************************
Summary:
    Propagate mixer input volume into a mixer object in nexus
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_P_SetInputVolume(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    const BAPE_MixerInputVolume *pInputVolume
    );

/***************************************************************************
Summary:
    Check mixer state - mixer can be started even if inputs are not started
 ***************************************************************************/
bool NEXUS_AudioMixer_P_IsStarted(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
    Check mixer state - mixer can be started even if inputs are not started
 ***************************************************************************/
bool NEXUS_AudioMixer_P_IsExplicitlyStarted(
    NEXUS_AudioMixerHandle handle
    );

/***************************************************************************
Summary:
    Get an external input port handle
 ***************************************************************************/
BAPE_InputPort NEXUS_AudioInput_P_GetInputPort(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Determine if this input supports dynamic format changes
 ***************************************************************************/
bool NEXUS_AudioInput_P_SupportsFormatChanges(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Enable/Disable interrupt for dynamic format changes
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_SetFormatChangeInterrupt(
    NEXUS_AudioInputHandle input, NEXUS_AudioInputType clientType,
    void (*pCallback_isr)(void *, int),
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Set stc index for compressed audio DSP use
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_SetStcIndex(
    NEXUS_AudioInputHandle input,
    unsigned stcIndex
    );
/***************************************************************************
Summary:
Get Input Status Information

Description:
Retrieves input status information.  This is currently only supported for
HDMI or SPDIF inputs.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_GetInputPortStatus(
    NEXUS_AudioInputHandle input,
    NEXUS_AudioInputPortStatus *pStatus     /* [out] */
    );

/***************************************************************************
Summary:
Force a particular input to stop

Description:
This function should only be used when the system is shutting down to
avoid clobbering application state.
 ***************************************************************************/
void NEXUS_AudioInput_P_ForceStop(NEXUS_AudioInputHandle input);

/***************************************************************************
Summary:
    NEXUS audio output private data.
 ***************************************************************************/
BDBG_OBJECT_ID_DECLARE(NEXUS_AudioOutputData);
typedef struct NEXUS_AudioOutputData
{
    BDBG_OBJECT(NEXUS_AudioOutputData)
    NEXUS_AudioOutputSettings settings;
    NEXUS_AudioInputHandle input;
    NEXUS_AudioOutputSyncSettings syncSettings;
    NEXUS_AudioOutputStatus status;
    NEXUS_AudioEqualizerHandle equalizer;
    BAVC_AudioSamplingRate sampleRate;
    bool settingsChanged;
    bool compressedMute;
} NEXUS_AudioOutputData;

/***************************************************************************
Summary:
    Is an output a DAC slave?
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsDacSlave(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Is an output a SPDIF slave (e.g. ARC)?
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsSpdifSlave(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Set a DAC slave source.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetSlaveSource(
    NEXUS_AudioOutputHandle slaveHandle,
    NEXUS_AudioOutputHandle sourceHandle
    );

/***************************************************************************
Summary:
    Power Up the DAC
 ***************************************************************************/
void NEXUS_AudioDac_P_PowerUp (
    void *pHandle
    );

/***************************************************************************
Summary:
    Power Down the DAC
 ***************************************************************************/
void NEXUS_AudioDac_P_PowerDown (
    void *pHandle
    );

/***************************************************************************
Summary:
    Power Up SPDIF
 ***************************************************************************/
void NEXUS_SpdifOutput_P_PowerUp (
    void *pHandle
    );

/***************************************************************************
Summary:
    Power Down SPDIF
 ***************************************************************************/
void NEXUS_SpdifOutput_P_PowerDown (
    void *pHandle
    );

/***************************************************************************
Summary:
    Get mixer settings required for an output
 ***************************************************************************/
void NEXUS_AudioOutput_P_GetMixerSettings(
    NEXUS_AudioOutputHandle output,
    BAPE_MixerSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set output format
 ***************************************************************************/
void NEXUS_AudioOutput_P_SetOutputFormat(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputFormat format
    );

/***************************************************************************
Summary:
    Determine if an audio output is active
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsRunning(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Create output-private data structure
 ***************************************************************************/
NEXUS_AudioOutputData *NEXUS_AudioOutput_P_CreateData(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Print the output timebase. Used for debug.
 ***************************************************************************/
void NEXUS_AudioOutput_P_VerifyTimebase(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Get the equalizer handle for an output
 ***************************************************************************/
NEXUS_AudioEqualizerHandle NEXUS_AudioOutput_P_GetEqualizer(
    NEXUS_AudioOutputHandle output
    );

/***************************************************************************
Summary:
    Set the Compressed mute value for an output
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetCompressedMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    );

/***************************************************************************
Summary:
    Get the equalizer handle for an output
 ***************************************************************************/
void NEXUS_AudioEqualizer_P_GetStages(
    NEXUS_AudioEqualizerHandle handle,
    BAPE_EqualizerStageHandle **pStages,
    unsigned *pNumStages
    );

/***************************************************************************
Summary:
    Link audio mux output to a particular node
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_P_AddInput(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Unlink audio mux output from a particular node
 ***************************************************************************/
void NEXUS_AudioMuxOutput_P_RemoveInput(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Reset MuxOutput nodes on a watchdog event
 ***************************************************************************/
void NEXUS_AudioMuxOutput_P_WatchdogReset(void);

/***************************************************************************
Summary:
    Set the Spdif master for a slaved Audio Return Channel output.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioReturnChannel_P_SetMaster(
    NEXUS_AudioOutputHandle slaveHandle,      /* handle of ARC (slave) output device */
    NEXUS_AudioOutputHandle sourceHandle      /* handle of SPDIF master */
    );

/***************************************************************************
Summary:
    Initialize the decoder module
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Init(void);

/***************************************************************************
Summary:
    Un-Initialize the decoder module
 ***************************************************************************/
void NEXUS_AudioDecoder_P_Uninit(void);

/***************************************************************************
Summary:
    Is a decoder running?
 ***************************************************************************/
bool NEXUS_AudioDecoder_P_IsRunning(NEXUS_AudioDecoderHandle handle);

/***************************************************************************
Summary:
    Set TSM state for the decoder
 ***************************************************************************/
void NEXUS_AudioDecoder_P_SetTsm(NEXUS_AudioDecoderHandle handle);

/***************************************************************************
Summary:
    Start the decoder
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Start(NEXUS_AudioDecoderHandle handle);

/***************************************************************************
Summary:
    Stop the decoder
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_Stop(NEXUS_AudioDecoderHandle handle, bool flush);

/***************************************************************************
Summary:
    Set trick mode mute
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_SetTrickMute(NEXUS_AudioDecoderHandle decoder, bool muted);

/***************************************************************************
Summary:
    Reset decoder trick state
 ***************************************************************************/
void NEXUS_AudioDecoder_P_TrickReset( NEXUS_AudioDecoderHandle decoder);

/***************************************************************************
Summary:
    Reset audio DSP(s)
 ***************************************************************************/
void NEXUS_AudioDecoder_P_Reset(void);

/***************************************************************************
Summary:
    Load audio PAK
 ***************************************************************************/
void NEXUS_AudioDecoder_P_LoadPak(void);

/***************************************************************************
Summary:
    DecodeToMemory Initializer/Finalizer
 ***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_InitDecodeToMemory(NEXUS_AudioDecoderHandle decoder);
void NEXUS_AudioDecoder_P_UninitDecodeToMemory(NEXUS_AudioDecoderHandle decoder);

/***************************************************************************
Summary:
    Is a playback running?
 ***************************************************************************/
bool NEXUS_AudioPlayback_P_IsRunning(NEXUS_AudioPlaybackHandle handle);

/***************************************************************************
Summary:
    Convert TruVolume settings to magnum
 ***************************************************************************/
void NEXUS_TruVolume_P_ConvertSettings(const NEXUS_TruVolumeSettings *pNexus, BAPE_TruVolumeSettings *pMagnum);

/***************************************************************************
Summary:
    Is an I2sInput running?
 ***************************************************************************/
bool NEXUS_I2sInput_P_IsRunning(NEXUS_I2sInputHandle handle);

/***************************************************************************
Summary:
    Is an InputCapture running?
 ***************************************************************************/
bool NEXUS_AudioInputCapture_P_IsRunning(NEXUS_AudioInputCaptureHandle handle);

#if NEXUS_NUM_RF_AUDIO_DECODERS > 0
BDBG_OBJECT_ID_DECLARE(NEXUS_RfAudioDecoder);
typedef struct NEXUS_RfAudioDecoder
{
    BDBG_OBJECT(NEXUS_RfAudioDecoder)
    bool started;
    bool running;
    bool opened;
    unsigned outputScaling;
    NEXUS_RfAudioDecoderSettings settings;
    BAPE_DecoderHandle decoder;
    BAPE_InputCaptureHandle inputCapture;
    BAPE_RfInputHandle input;
    NEXUS_AudioInputObject connector;
    NEXUS_RfAudioDecoderMode currentMode;
    void *pStatusData;
    BKNI_EventHandle statusEvent, stopEvent, stopAckEvent;
    NEXUS_TaskCallbackHandle statusChangedCallback;
    NEXUS_TaskCallbackHandle standardDetectedCallback;
    BANA_Handle hAna;
    bool runMagShiftThread;
    NEXUS_ThreadHandle magShiftThread;
    BKNI_EventHandle magShiftChangeEvent;
    bool runAudioClipThread;
    NEXUS_ThreadHandle audioClipThread;
    BKNI_EventHandle audioClipEvent;
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
#endif

#if NEXUS_NUM_ANALOG_AUDIO_INPUTS
BDBG_OBJECT_ID_DECLARE(NEXUS_AnalogAudioInput);
/***************************************************************************
Summary:
    Analog Audio Input
 ***************************************************************************/
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
#endif

NEXUS_Error NEXUS_SpdifInput_P_GetInputPortStatus(
    NEXUS_SpdifInputHandle input,
    NEXUS_AudioInputPortStatus *pStatus     /* [out] */
    );

NEXUS_Error NEXUS_SpdifInput_P_SetFormatChangeInterrupt(
    NEXUS_SpdifInputHandle handle,
    void (*pCallback_isr)(void *, int),
    void *pParam1,
    int param2
    );

NEXUS_Error NEXUS_SpdifInput_P_SetStcIndex(
    NEXUS_SpdifInputHandle handle,
    unsigned stcIndex
    );

/***************************************************************************
Summary:
    Get the APE CRC handle
 ***************************************************************************/
BAPE_CrcHandle NEXUS_AudioCrc_P_GetPIHandle(
    NEXUS_AudioCrcHandle handle
    );

/***************************************************************************
Summary:
    Get the current input settings.
    If no inputs are connected, an Error will be returned.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_P_GetCurrentInputSettings(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcInputSettings * inputSettings /* [out] */
    );

/***************************************************************************
Summary:
    Gets the mixer handle for a specific index.
    If the mixer is not opened NULL will be returned.
***************************************************************************/
NEXUS_AudioMixerHandle NEXUS_AudioMixer_P_GetMixerByIndex(
    unsigned index
    );

/***************************************************************************
Summary:
    Gets the playback handle for a specific index.
    If the playback is not opened NULL will be returned.
***************************************************************************/

NEXUS_AudioPlaybackHandle NEXUS_AudioPlayback_P_GetPlaybackByIndex(
    unsigned index
    );

/***************************************************************************
Summary:
    Gets the input capture handle for a specific index.
    If the input capture is not opened NULL will be returned.
***************************************************************************/

NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(
    unsigned index
    );

/***************************************************************************
Summary:
    Converts NEXUS_AudioCodec to string
***************************************************************************/
const char * NEXUS_AudioCodecToString(
    NEXUS_AudioCodec codec
    );

/***************************************************************************
Summary:
    Converts NEXUS_AudioInputFormat to string
***************************************************************************/
const char * NEXUS_AudioInputFormatToString(
    NEXUS_AudioInputFormat inputformat
    );

/***************************************************************************
Summary:
    Iterate the connector's downstream list to locate a mixer.
    Return NULL if no mixer is found.
 ***************************************************************************/
NEXUS_AudioMixerHandle NEXUS_AudioInput_P_LocateMixer(
    NEXUS_AudioInputHandle source,
    NEXUS_AudioMixerHandle hLastMixerHandle
    );

/***************************************************************************
Summary:
Traverses connections to see if there is a DSP Mixer attached to the decoder.
***************************************************************************/
NEXUS_Error NEXUS_AudioDecoder_P_IsDspMixerAttached(
    NEXUS_AudioDecoderHandle handle,
    bool *pDSPMixerAttached
    );

/***************************************************************************
Summary:
    Iterate the connector's downstream list the next downstream object.
 ***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioInput_P_LocateDownstream(
    NEXUS_AudioInputHandle source,
    NEXUS_AudioInputHandle hLastAudioInput
    );

/***************************************************************************
Summary:
    Iterate the objects output list.
 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioInput_P_LocateOutput(
    NEXUS_AudioInputHandle source,
    NEXUS_AudioOutputHandle hLastAudioOutput
    );

/***************************************************************************
Summary:
    Starts all FMM mixers attached to an input
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_ExplictlyStartFMMMixers(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Stop all FMM mixers attached to an input
 ***************************************************************************/
void NEXUS_AudioInput_P_ExplictlyStopFMMMixers(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Set Default Input Volume
 ***************************************************************************/
void NEXUS_AudioInput_P_GetDefaultInputVolume(
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
    Converts dB index to Linear Volume
 ***************************************************************************/
int32_t NEXUS_Audio_P_ConvertDbToLinear(
    int index
    );

#define NEXUS_AUDIO_CAPTURE_INDEX_BASE (0)
#ifdef NEXUS_NUM_AUDIO_INPUT_CAPTURES
#define NEXUS_I2S_CAPTURE_INDEX_BASE (NEXUS_NUM_AUDIO_INPUT_CAPTURES)
#else
#define NEXUS_I2S_CAPTURE_INDEX_BASE (0)
#endif
#define NEXUS_ADC_CAPTURE_INDEX_BASE (0)
#define NEXUS_RF_CAPTURE_INDEX_BASE (0)
#define NEXUS_AUDIO_CAPTURE_INDEX(typ, idx) (((typ)==NEXUS_AudioInputType_eInputCapture)?NEXUS_AUDIO_CAPTURE_INDEX_BASE+(idx):\
                                             ((typ)==NEXUS_AudioInputType_eI2s)?(NEXUS_I2S_CAPTURE_INDEX_BASE-(int)idx-1)>=0?(NEXUS_I2S_CAPTURE_INDEX_BASE-idx-1):0:\
                                             ((typ)==NEXUS_AudioInputType_eAnalogDecoder)?NEXUS_ADC_CAPTURE_INDEX_BASE+(idx):\
                                             ((typ)==NEXUS_AudioInputType_eRfDecoder)?NEXUS_RF_CAPTURE_INDEX_BASE+(idx):\
                                             (BDBG_ASSERT(0),0))

/***************************************************************************
Summary:
    Audio Decoder Indexes
 ***************************************************************************/
#define NEXUS_AUDIO_DECODER_INDEX_BASE (0)
#define NEXUS_I2S_DECODER_INDEX_BASE (NEXUS_NUM_AUDIO_DECODERS)
#define NEXUS_ADC_DECODER_INDEX_BASE (0)
#define NEXUS_RF_DECODER_INDEX_BASE (0)
#define NEXUS_AUDIO_DECODER_INDEX(typ, idx) (((typ)==NEXUS_AudioInputType_eDecoder)?NEXUS_AUDIO_DECODER_INDEX_BASE+(idx):\
                                             ((typ)==NEXUS_AudioInputType_eI2s)?NEXUS_I2S_DECODER_INDEX_BASE+(idx):\
                                             ((typ)==NEXUS_AudioInputType_eAnalogDecoder)?NEXUS_ADC_DECODER_INDEX_BASE+(idx):\
                                             ((typ)==NEXUS_AudioInputType_eRfDecoder)?NEXUS_RF_DECODER_INDEX_BASE+(idx):\
                                             (BDBG_ASSERT(0),0))

#define NEXUS_AUDIO_DECODER_P_MAX_DSOLA_RATE  (2 * NEXUS_NORMAL_DECODE_RATE) /* 2x */

/***************************************************************************
Summary:
    Convert between nexus/avc codecs
 ***************************************************************************/
NEXUS_AudioCodec NEXUS_Audio_P_MagnumToCodec(BAVC_AudioCompressionStd codec);
BAVC_AudioCompressionStd NEXUS_Audio_P_CodecToMagnum(NEXUS_AudioCodec codec);
NEXUS_Error NEXUS_AudioDecoder_P_ConfigureRave(NEXUS_RaveHandle rave, const NEXUS_AudioDecoderStartSettings *pProgram, const NEXUS_PidChannelStatus * pPidStatus, bool isPlayback);

void NEXUS_AudioDecoderPrimer_P_DecodeStopped(NEXUS_AudioDecoderPrimerHandle primer);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_3dSurround);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Ac3Encode);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioCapture);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioCrc);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioDecoderPrimer);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DolbyDigitalReencode);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioEncoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioEqualizer);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioEqualizerStage);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioInputCapture);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioMixer);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioMuxOutput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioOutputGroup);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioPlayback);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioProcessor);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioReturnChannel);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AutoVolumeLevel);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DolbyVolume);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DolbyVolume258);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DtsEncode);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_EchoCanceller);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_I2sInput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_RfAudioEncoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SpdifInput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_TruVolume);
#if NEXUS_CVOICE
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_CustomVoice);
#endif
#if NEXUS_AUDIO_BUFFER_CAPTURE_EXT
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioBufferCapture);
#endif
#if NEXUS_AUDIO_RAVE_MONITOR_EXT
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AudioRaveMonitor);
#endif

#endif /* #ifndef NEXUS_AUDIO_MODULE_H_ */
