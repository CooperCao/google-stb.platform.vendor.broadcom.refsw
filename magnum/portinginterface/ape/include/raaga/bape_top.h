/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BAPE_H_
#define BAPE_H_

#include "bchp.h"
#include "bint.h"
#include "bmem.h"
#include "breg_mem.h"
#include "btmr.h"
#include "bimg.h"
#if BAPE_DSP_SUPPORT
#include "bdsp.h"
#else
typedef void *BDSP_Handle;
#endif
#include "bape_types.h"
#include "bape_mixer_input_capture.h"
#include "bape_mixer.h"
#include "bape_decoder.h"
#include "bape_playback.h"
#include "bape_input.h"
#include "bape_output.h"
#include "bape_mux_output.h"
#include "bape_pll.h"
#include "bape_input_capture.h"
#include "bape_output_capture.h"
#include "bape_encoder.h"
#include "bape_dolby_digital_reencode.h"
#include "bape_dolby_volume.h"
#include "bape_processor.h"
#include "bape_tru_surround.h"
#include "bape_tru_volume.h"
#if BAPE_DSP_SUPPORT
#include "bape_custom_processing.h"
#endif
#include "bape_auto_volume_level.h"
#include "bape_3d_surround.h"
#include "bape_equalizer.h"
#include "bape_rf_encoder.h"
#include "bape_echo_canceller.h"
#include "bape_crc.h"
#include "bape_debug.h"

/*=************************ Module Overview ********************************
APE (Audio Processing Engine) is a porting interface (PI) module that
controls the audio subsystem.  This includes the AIO (mixer, inputs, outputs)
as well as the decode processor (DSP or CPU).

Device:
-------
An APE Device is identified by a device handle (BAPE_Handle).  This is the
top-level interface to the audio subsystem.

Decoder:
--------
A decoder is capable of decoding digital audio into PCM samples and also
providing compressed passthrough of input data.  This is identified by
a BAPE_DecoderHandle.  Decoding is done in a decode processor, and the
output can be fed into a mixer.  Optionally, post-processing (BAPE_Processor)
may be inserted between the decoder and the mixer.   Decoders can read
from a RAVE context (BAVC_XptContextMap) or on some systems can read
from an input port (BAPE_InputPort).  The latter is useful for external
inputs that provide compressed data (e.g. SPDIF) or inputs that require
post processing.

Playback:
---------
Playback is used for playing host-generated PCM data.  It is also an
input to a mixer.

InputCapture:
-------------
InputCapture is used to capture PCM input from an input port (BAPE_InputPort)
and feeds data into a mixer.

InputGroup:
-----------
Input groups allow for multiple stereo inputs to be grouped together
and treated as multichannel data.  This is useful if you want to combine
three stereo playback channels to generate 5.1 audio for example.  Grouped
inputs will always start synchronously in the mixer.

Mixer:
------
Mixers are used to mix one or more inputs to one or more outputs.  A
mixer is always required to bind an output to its input(s).   Mixers
provide input volume scaling as well as output volume scaling.  All
inputs to a mixer must run at the same sample rate, and correspondingly
they will be sample rate converted to either a fixed sample rate value
or they can slave to a master input's sampling rate.  It is possible
for an input to be routed to more than one mixer.  For example, the
application may want different input volume levels applied to one
output vs. another one.  For example, you could use two mixers to
send a decode input mixed with a playback input to one output setting
both inputs to half volume.  You could simultaneously route the same
decode input to another output at full volume with no playback
by using two mixers and applying different volume levels to each.
Inputs to a mixer are connected using a generic connector type
(BAPE_Connector), and similarly outputs are connected using
a generic connector type (BAPE_OutputPort).

Outputs:
--------
Several different output types are supported by this module.  They include
audio DACs (BAPE_DacHandle), SPDIF transmitters (BAPE_SpdifOutputHandle),
I2S transmitters (BAPE_I2sOutputHandle) and the MAI interface to HDMI
(BAPE_MaiOutputHandle).  Outputs must always be connected to a single
mixer to receive data, even if they are receiving compressed output.
Output data can also be captured by the host by connecting an
BAPE_OutputCaptureHandle.

Inputs:
-------
External audio inputs are supported by this module.  BAPE_I2sInput
represents an I2S receiver.  In the future, other inputs may be
added as well.

Example Connection Diagrams:
----------------------------
1) Single decoder routed to DAC + SPDIF
BAPE_DecoderHandle (stereo data) -> BAPE_MixerHandle -> BAPE_DacHandle + BAPE_SpdifOutputHandle

2) Decoder + host data mixed to DAC, Decoder data routed directly to SPDIF (PCM)
BAPE_PlaybackHandle --------------\
BAPE_DecoderHandle (stereo data) --> BAPE_MixerHandle1 -> BAPE_DacHandle
                                 \-> BAPE_MixerHandle2 -> BAPE_SpdifOutputHandle

3) Decode to DAC with compressed passthrough to SPDIF

BAPE_DecoderHandle1 (stereo data)     -> BAPE_MixerHandle1 -> BAPE_DacHandle
BAPE_DecoderHandle2 (compressed data) -> BAPE_MixerHandle2 -> BAPE_SpdifOutputHandle

4) Decode to DAC and I2S with DAC delayed to compensate for an external
audio processor on I2S.
BAPE_DecoderHandle (stereo data) --> BAPE_MixerHandle1 (delay=0) -> BAPE_DacHandle
                                 \-> BAPE_MixerHandle2 (delay=n) -> BAPE_I2sOutputHandle

5) I2S stereo input routed to DAC
BAPE_I2sInput -> BAPE_InputCapture -> BAPE_MixerHandle -> BAPE_DacHandle

6) Decode of primary audio mixed with decode of secondary audio (e.g. MP3 sound effect)
BAPE_DecoderHandle1 (stereo data) --> BAPE_MixerHandle -> BAPE_DacHandle
BAPE_DecoderHandle2 (stereo data) -/

***************************************************************************/

/***************************************************************************
Summary:
Device Settings
***************************************************************************/
typedef struct BAPE_Settings
{
    unsigned maxDspTasks;               /* Maximum DSP tasks.  One task is required for each decoder and FW Mixer that will run concurrently. */
    unsigned maxArmTasks;               /* Maximum ARM tasks.  One task is required for each decoder and FW Mixer that will run concurrently. */
    unsigned maxIndependentDelay;       /* Max independent delay value in ms */
    unsigned maxPcmSampleRate;          /* Max PCM sample rate in Hz */
    unsigned numPcmBuffers;             /* Number of discrete PCM decoder output buffers required.  This is the
                                           number of outputs that will receive discrete stereo content plus the
                                           number of outputs that will receive discrete multichannel output * the number
                                           of channel pairs involved in multichannel (3 for 5.1, 4 for 7.1).
                                           Independent delay does not affect this number, only different content sources.
                                           These buffers are also used when non-DSP data sources (e.g. Playback or Equalizer) connect
                                           as input to DSP mixers. */
    unsigned numCompressedBuffers;      /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 48kHz.
                                           Independent delay does not affect this number, only different content sources. */
    unsigned numCompressed4xBuffers;    /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 192kHz.
                                           Independent delay does not affect this number, only different content sources. */
    unsigned numCompressed16xBuffers;   /* Number of discrete compressed decoder output buffers required.  This is the
                                           number of outputs that will receive discrete compressed content running at up to 768kHz (HDMI HBR).
                                           Independent delay does not affect this number, only different content sources. */
    unsigned numRfEncodedPcmBuffers;    /* Number of discrete RF encoded PCM buffers required.  This is the
                                           number of outputs that will receive discrete RF encoded PCM content running at up to 192kHz (BTSC).
                                           Independent delay does not affect this number, only different content sources. */
    bool rampPcmSamples;                /* If true (default), PCM samples will be ramped up/down on startup, shutdown, and underflow conditions.
                                          Set to false if you want to disable this feature for testing or verification purposes. */
    BAPE_LoudnessEquivalenceMode loudnessMode;  /* Loudness Equivalence Mode.  Default is BAPE_LoudnessEquivalenceMode_eNone. */
    struct {
        BSTD_DeviceOffset baseAddress; /* Physical base address of the lowest physical address region for each MEMC.
            [0] is always 0 and it is assumed to always exist. For [1] and [2], an address of 0 means the MEMC is not populated.
            APE is unable to access a discontiguous upper memory region, so its base address and size is not needed. */
    } memc[3]; /* for each MEMC */
} BAPE_Settings;

/***************************************************************************
Summary:
Get default settings for an audio processor
***************************************************************************/
void BAPE_GetDefaultSettings(
    BAPE_Settings *pSettings    /* [out] */
    );


/***************************************************************************
Summary:
Memory Estimate Results
***************************************************************************/
typedef struct BAPE_MemoryEstimate
{
    unsigned general;                   /* General Memory from System Heap - Decoder output buffers, etc */
} BAPE_MemoryEstimate;

/***************************************************************************
Summary:
Get an estimate of the memory required by APE
***************************************************************************/
BERR_Code BAPE_GetMemoryEstimate(
    const BAPE_Settings *pSettings,            /* [in] - required */
    BAPE_MemoryEstimate *pEstimate             /* [out] */
    );

/***************************************************************************
Summary:
Get default settings for an audio processor
***************************************************************************/
void BAPE_GetDefaultSettings(
    BAPE_Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open an audio processor
***************************************************************************/
BERR_Code BAPE_Open(
    BAPE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMEM_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    BDSP_Handle dspHandle,
    BDSP_Handle armHandle,
    const BAPE_Settings *pSettings  /* NULL will use default settings */
    );

/***************************************************************************
Summary:
Close an audio processor
***************************************************************************/
void BAPE_Close(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Device Level Interrupts
***************************************************************************/
typedef struct BAPE_InterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } watchdog;
} BAPE_InterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BAPE_GetInterruptHandlers(
    BAPE_Handle handle,
    BAPE_InterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers

Description:
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BAPE_SetInterruptHandlers(
    BAPE_Handle handle,
    const BAPE_InterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Process a watchdog interrupt stop
***************************************************************************/
BERR_Code BAPE_ProcessWatchdogInterrupt(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Process a watchdog interrupt stop
***************************************************************************/
BERR_Code BAPE_ProcessWatchdogInterruptStop(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Process a watchdog interrupt resume
***************************************************************************/
BERR_Code BAPE_ProcessWatchdogInterruptResume(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Returns single null terminated line from the debug log
***************************************************************************/
BERR_Code BAPE_GetDebugLog(
    BAPE_Handle handle,
    char *buf,
    size_t buf_len
    );

/***************************************************************************
Summary:
Standby settings. Currently unused.
****************************************************************************/
typedef struct BAPE_StandbySettings
{
    bool dummy; /* placeholder to avoid compiler warning */
} BAPE_StandbySettings;

/***************************************************************************
Summary:
Enter Standby mode
***************************************************************************/
BERR_Code BAPE_Standby(
    BAPE_Handle handle,             /* [in]APE device handle */
    BAPE_StandbySettings *pSettings /* [in] standby settings */
    );

/***************************************************************************
Summary:
Resume from standby mode
***************************************************************************/
BERR_Code BAPE_Resume(
    BAPE_Handle handle      /* [in] APE device handle */
    );

/***************************************************************************
Summary:
Audio HW/FW Capabilities
***************************************************************************/
typedef struct BAPE_Capabilities
{
    struct
    {
        unsigned i2s;                   /* Number of I2S inputs */
        unsigned mai;                   /* Number of MAI inputs */
        unsigned spdif;                 /* Number of SPDIF inputs */
    } numInputs;

    struct
    {
        unsigned audioReturnChannel;    /* Number of ARC outputs */
        unsigned capture;               /* Maximum number of OutputCapture objects */
        unsigned dac;                   /* Number of DAC outputs */
        unsigned dummy;                 /* Maximum number of DummyOutput objects */
        unsigned i2s;                   /* Number of I2S outputs */
        unsigned loopback;              /* Maximum number of Loopback objects */
        unsigned mai;                   /* Number of MAI outputs */
        unsigned rfmod;                 /* Number of RF Mod outputs */
        unsigned spdif;                 /* Number of SPDIF outputs */
    } numOutputs;

    unsigned numDecoders;               /* Maximum number of Decoders */
    unsigned numPlaybacks;              /* Maximum number of Playbacks */
    unsigned numInputCaptures;          /* Maximum number of InputCaptures */
    unsigned numVcxos;                  /* Number of VCXO PLLs that feed PLLs */
    unsigned numPlls;                   /* Number of output PLL clocks */
    unsigned numNcos;                   /* Number of output NCO clocks */
    unsigned numCrcs;                   /* Number of CRC objects */
    unsigned numStcs;                   /* Number of STC objects */

    unsigned numDsps;                   /* Number of audio DSPs */
    struct
    {
        struct
        {
            bool decode;                /* True if this codec can be decoded */
            bool passthrough;           /* True if this codec can be decoded */
            bool encode;                /* True if this codec can be encoded */
        } codecs[BAVC_AudioCompressionStd_eMax];

        bool audysseyAbx;               /* True if Audyssey ABX is supported */
        bool audysseyAdv;               /* True if Audyssey ADV is supported */
        bool autoVolumeLevel;           /* True if AutoVolumeLevel is supported */
        bool _3dSurround;               /* True if 3dSurround is supported */
        bool decodeRateControl;         /* True if DSOLA audio rate control is supported */
        bool dolbyDigitalReencode;      /* True if Dolby MS11 is supported */
        bool dolbyVolume;               /* True if DolbyVolume processing is supported */
        bool karaoke;                   /* True if Karaoke post processing is supported */

        struct
        {
            bool supported;             /* True if echo cancellation is supported */
            bool algorithms[BAPE_EchoCancellerAlgorithm_eMax];  /* Each supported algorithm will be set to true */
        } echoCanceller;

        bool encoder;                   /* True if encoder is supported */
        bool mixer;                     /* True if DSP mixers are supported */
        bool muxOutput;                 /* True if MuxOutput is supported for transcoding */

        struct
        {
            bool supported;             /* True if RF encoder is supported */
            bool encodings[BAPE_RfAudioEncoding_eMax];  /* Each supported encoding will be set to true */
        } rfEncoder;

        bool studioSound;               /* True if SRS StudioSound processing is supported */
        bool truSurroundHd;             /* True if SRS TruSurround HD processing is supported */
        bool truVolume;                 /* True if SRS TruVolume processing is supported */
        bool processing[BAPE_PostProcessorType_eMax];  /* True if specificed type is supported by BAPE_Processor */
        char versionInfo[25];           /* Raaga Release Version */
    } dsp;

    struct
    {
        bool supported;                 /* True if Equalizer support is available */
        bool types[BAPE_EqualizerStageType_eMax];   /* Each supported equalizer type will be set to true */
    } equalizer;

} BAPE_Capabilities;

/***************************************************************************
Summary:
Get Audio HW/FW Capabilities
***************************************************************************/
void BAPE_GetCapabilities(
    BAPE_Handle hApe,
    BAPE_Capabilities *pCaps        /* [out] */
    );

#if BAPE_DSP_SUPPORT
/***************************************************************************
Summary:
Get Audio Algo attributes
***************************************************************************/
BDSP_Algorithm BAPE_GetCodecAudioDecode (
    BAVC_AudioCompressionStd codec
    );

BDSP_Algorithm BAPE_GetCodecAudioPassthrough (
    BAVC_AudioCompressionStd codec
    );

BDSP_Algorithm BAPE_GetCodecAudioEncode (
    BAVC_AudioCompressionStd codec
    );

bool BAPE_CodecRequiresSrc (
    BAVC_AudioCompressionStd codec
    );

bool BAPE_CodecSupportsCompressed4x (
    BAVC_AudioCompressionStd codec
    );

bool BAPE_CodecSupportsCompressed16x (
    BAVC_AudioCompressionStd codec
    );

BAPE_DolbyMSVersion BAPE_GetDolbyMSVersion (void);
#endif

#endif
