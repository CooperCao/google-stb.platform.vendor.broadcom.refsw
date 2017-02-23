/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#ifndef BAPE_DECODER_H_
#define BAPE_DECODER_H_

#include "bavc.h"
#include "bape_types.h"
#include "bape_input.h"
#include "bape_codec_types.h"

/***************************************************************************
Summary:
Decoder Handle
***************************************************************************/
typedef struct BAPE_Decoder *BAPE_DecoderHandle;

/***************************************************************************
Summary:
Decoder Open settings
***************************************************************************/
typedef enum BAPE_DecoderType
{
    BAPE_DecoderType_eUniversal,    /* Supports both decode and passthrough operations */
    BAPE_DecoderType_eDecode,       /* Supports only decode operations */
    BAPE_DecoderType_ePassthrough,  /* Supports only passthrough operations */
    BAPE_DecoderType_eDecodeToMemory, /* Supports only decoding to memory for host processing.  You must add and process buffers
                                         via BAPE_Decoder_QueueBuffer/BAPE_Decoder_GetBuffers/BAPE_Decoder_ConsumeBuffers/etc. */
    BAPE_DecoderType_eMax
} BAPE_DecoderType;

/***************************************************************************
Summary:
Decoder Open settings
***************************************************************************/
typedef struct BAPE_DecoderOpenSettings
{
    unsigned dspIndex;                          /* Index of the DSP you would like to use.  Default = 0. */
    size_t ancillaryDataFifoSize;               /* Ancillary data FIFO size in bytes */
    BAVC_AudioCompressionStd *pSupportedCodecs; /* List of audio compression standards supported by this decoder.
                                                   If NULL, all supported codecs will be enabled. */
    unsigned numSupportedCodecs;                /* Number of supported elements in pSupportedCodecs */
    BAPE_DecoderType type;                      /* Decoder Type */
    bool rateControlSupport;                    /* If true, decode rate control will be supported for this decoder. */
    bool karaokeSupported;                      /* If true, karaoke post process will be supported for this decoder. */
    BAPE_MultichannelFormat multichannelFormat; /* Set the multichannel format for this decoder */
} BAPE_DecoderOpenSettings;

/***************************************************************************
Summary:
Get default open settings for an audio decoder
***************************************************************************/
void BAPE_Decoder_GetDefaultOpenSettings(
    BAPE_DecoderOpenSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Open an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DecoderOpenSettings *pSettings,
    BAPE_DecoderHandle *pHandle                 /* [out] */
    );

/***************************************************************************
Summary:
Close an audio decoder
***************************************************************************/
void BAPE_Decoder_Close(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Decoder Mixing Modes
***************************************************************************/
typedef enum BAPE_DecoderMixingMode
{
    BAPE_DecoderMixingMode_eDescription,  /* In this mixing mode, channels will be mixed using
                                             volume coefficients contained in the secondary audio
                                             program.  This is typically used in audio services for
                                             the visually impaired (as described in DTG D-Book section 4.5),
                                             where a voice over track can be mixed with the primary
                                             audio track. */
    BAPE_DecoderMixingMode_eSoundEffects, /* In this mixing mode, channels will be mixed using static
                                             coefficients in the mixer as opposed to stream-based
                                             coefficients. */
    BAPE_DecoderMixingMode_eApplicationAudio, /* In this mixing mode, channels will be mixed using static
                                             coefficients in the mixer as opposed to stream-based
                                             coefficients. */
    BAPE_DecoderMixingMode_eStandalone,   /* In this mixing mode, decodes will remain independent.
                                             Currently only supported for Dolby MS12 configurations. */
    BAPE_DecoderMixingMode_eMax
} BAPE_DecoderMixingMode;

/***************************************************************************
Summary:
Start-time settings for an audio decoder
***************************************************************************/
typedef struct BAPE_DecoderStartSettings
{
    BAVC_AudioCompressionStd codec;
    BAVC_StreamType streamType;         /* Required for TSM computation */

    unsigned stcIndex;                  /* Which STC index should be used */

    const BAVC_XptContextMap *pContextMap;  /* What RAVE context should be read while decoding.
                                               If decoding from an input port, pass NULL for this
                                               setting */
    BAPE_InputPort inputPort;               /* What input port to read data from.  If decoding from
                                               RAVE, pass NULL for this setting */

    bool targetSyncEnabled;             /* If true, normal frame sync operation will be used (default).  This flag can be set to false
                                           for certification applications that require the last frame of the input buffer to be consumed
                                           without the audio decoder finding the start of the successive frame.  */

    bool ppmCorrection;                 /* If true, PPM correction will be performed for 2ms lipsync
                                           precision on PCM outputs.  Not available with any compressed
                                           or multichannel data outputs into hardware. */

    bool decodeRateControl;             /* If true, decoder rate control is enabled for trick mode
                                           operations.  Not availble with ppmCorrection or if any
                                           compressed or multichannel data outputs into hardware. */

    bool karaokeModeEnabled;            /* If true, karaoke postprocess is enabled. */

    bool nonRealTime;                   /* Normal operation for decoding is real time, if this is set to 'true' then decoding is used as a
                                           source for non-realtime transcode operations */

    bool forceCompleteFirstFrame;       /* If true, the first frame will always be entirely rendered to the output and not partially truncated
                                           for TSM computations.  This should be disabled for normal operation, but may be required for some
                                           bit-exact certification testing that requires all data to be rendered even with TSM enabled. */

    BAPE_DspDelayMode delayMode;        /* DSP Delay mode for this task.  Default uses a fixed path delay irrespective of source codec.
                                           Low delay mode provides lower delay depending on the input codec, but has restrictions on
                                           running only a single DSP task at a time as well as limits on the post-processing that can
                                           be performed */

    BAPE_DecoderMixingMode mixingMode;  /* Mixing mode to be used when multiple decoders are mixed */
    unsigned maxOutputRate;             /* Max output rate we support from the decoder -
                                           Valid values are 48000 or 96000. Units are Hz. Default is 48000.
                                           Content above the maxOutputRate will be downsampled within its rate family.
                                           For example - if the content is 88200Hz and maxOutputRate = 48000, decoder
                                           will output 44100Hz for that content.  Similarly, if the content is
                                           176,400Hz and maxOutputRate = 96000, decoder will output 88200Hz. */
} BAPE_DecoderStartSettings;

/***************************************************************************
Summary:
Get default start settings for an audio decoder
***************************************************************************/
void BAPE_Decoder_GetDefaultStartSettings(
    BAPE_DecoderStartSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Start an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Start(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop an audio decoder
***************************************************************************/
void BAPE_Decoder_Stop(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Pause an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Pause(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Resume an audio decoder from a paused state
***************************************************************************/
BERR_Code BAPE_Decoder_Resume(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Advance an audio decoder in ms units.  Must be paused first.
***************************************************************************/
BERR_Code BAPE_Decoder_Advance(
    BAPE_DecoderHandle handle,
    unsigned milliseconds           /* Milliseconds to advance */
    );

/***************************************************************************
Summary:
Freeze decoder path to output.
***************************************************************************/
BERR_Code BAPE_Decoder_Freeze(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
UnFreeze decoder path to output.
***************************************************************************/
BERR_Code BAPE_Decoder_UnFreeze(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Prepare the decoder to flush (called prior to flushing RAVE context)
***************************************************************************/
BERR_Code BAPE_Decoder_DisableForFlush(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Flush the decoder's buffers (called after flushing RAVE context)
***************************************************************************/
BERR_Code BAPE_Decoder_Flush(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Audio Decoder TSM Settings
***************************************************************************/
typedef struct BAPE_DecoderTsmSettings
{
    bool tsmEnabled;
    bool astmEnabled;
    bool playback;
    uint32_t ptsOffset;     /* PTS offset value in 45 kHz ticks.  Will be internally converted to
                               27MHz if required. */
    uint32_t stcOffset;     /* SW STC Offset in 45kHz ticks.  Will be internaly converted to
                               27 MHz if required.*/
    /* All thresholds below are programmed in milliseconds. */
    struct
    {
        unsigned discard;           /* Threshold (ms) beyond which frames will be discarded */
        unsigned grossAdjustment;   /* Threshold (ms) beyond which gross adjustments will be performed */
        unsigned smoothTrack;       /* Threshold (ms) beyond which smooth tracking (ppm) adjustments will be performed */
        unsigned syncLimit;         /* Sets the sync limit for audio master mode (in ms).  Set to 0 to disable audio master mode */
    } thresholds;
} BAPE_DecoderTsmSettings;

/***************************************************************************
Summary:
Get Audio Decoder TSM Settings
***************************************************************************/
void BAPE_Decoder_GetTsmSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Decoder TSM Settings in isr context
***************************************************************************/
void BAPE_Decoder_GetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    );

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    );

/***************************************************************************
Summary:
Indicate a playback STC is valid to the decoder in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetStcValid_isr(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Audio Decoder TSM Status
***************************************************************************/
typedef struct BAPE_DecoderTsmStatus
{
    BAVC_PTSInfo ptsInfo;
    int32_t ptsStcDifference;   /* PTS - STC value */
    unsigned lastFrameLength;   /* In ms */
} BAPE_DecoderTsmStatus;

/***************************************************************************
Summary:
Get Audio Decoder TSM Status
***************************************************************************/
BERR_Code BAPE_Decoder_GetTsmStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Decoder TSM Status in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_GetTsmStatus_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
Normal Playback Rate
***************************************************************************/
#define BAPE_NORMAL_DECODE_RATE (100)

/***************************************************************************
Summary:
Audio Decoder Karaoke Settings that are changeable on the fly
***************************************************************************/
typedef struct BAPE_DecoderKaraokeSettings
{
    unsigned vocalSuppressionLevel;     /*Amount of vocal suppression to be applied to the music program, specified in percentage reduction (attenuation).
                                            Valid values are 0,75,85,90,95,100. 0 = Disabled, 90 = Default.*/
    unsigned vocalSuppressionFrequency; /* This refers to the sampling frequency up to which the vocal suppression is applied to.
                                            The vocalist have a typical range of 200Hz to 8Khz. Default=4688Hz */
    unsigned outputMakeupBoost;         /* Amount to scale (boost) the processed output to compensate for audio level reduction caused by Karaoke processing.
                                            Specified in dB. Valid values are 0,1,2,3.  3 = Default. */
}BAPE_DecoderKaraokeSettings;

/***************************************************************************
Summary:
Get default karaoke settings for an audio decoder
***************************************************************************/
void BAPE_Decoder_GetDefaultKaraokeSettings(
    BAPE_DecoderKaraokeSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Audio Decoder Settings that are changeable on the fly
***************************************************************************/
typedef struct BAPE_DecoderSettings
{
    BAPE_MultichannelFormat multichannelFormat;     /* Controls whether the decoder outputs 2.0, 5.1, or 7.1 data on the multichannel path.
                                                       This is not changeable on the fly. */
    BAPE_ChannelMode outputMode;                    /* Can not be set to a wider value than multichannelFormat (e.g. 3_4 is not possible
                                                       when multichannel format is 5.1 */
    bool outputLfe;

    bool loudnessEquivalenceEnabled;                /* This value has been depracated.  Loudness equivalence is enabled via BAPE_Settings.loudnessMode. */

    bool ancillaryDataEnabled;                      /* If true, ancillary data parsing will be enabled.  Default is false. */

    BAPE_DualMonoMode dualMonoMode;

    unsigned decodeRate;                /* Playback Rate for trick modes.  Use BAPE_NORMAL_DECODE_RATE
                                           for normal operation.  Ranges from BAPE_NORMAL_DECODE_RATE/2
                                           to BAPE_NORMAL_DECODE_RATE*2.  ppmCorrection is not available
                                           if the rate is non-standard.  This is ignored unless
                                           BAPE_DecoderStartSettings.decodeRateControl is true */

    BAPE_DecoderKaraokeSettings karaokeSettings;    /* Settings for configuring the karaoke post process */
} BAPE_DecoderSettings;

/***************************************************************************
Summary:
Get Audio Decoder Settings
***************************************************************************/
void BAPE_Decoder_GetSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderSettings *pSettings
    );

/***************************************************************************
Summary:
Audio Decoder Status
***************************************************************************/
typedef struct BAPE_DecoderStatus
{
    BAVC_AudioCompressionStd codec;
    BAPE_DecoderTsmStatus tsmStatus;
    unsigned sampleRate;
    unsigned framesDecoded;
    unsigned frameErrors;
    unsigned dummyFrames;
    unsigned cdbUnderFlowCount;
    unsigned mode;
    bool running;
    bool halted;
    bool valid;
    union
    {
        BAPE_MpegStatus   mpeg;
        BAPE_Ac3Status    ac3;
        BAPE_Ac4Status    ac4;
        BAPE_AacStatus    aac;
        BAPE_WmaStatus    wma;
        BAPE_WmaProStatus wmaPro;
        BAPE_DtsStatus    dts;
        BAPE_PcmWavStatus pcmWav;
        BAPE_AmrStatus    amr;
        BAPE_DraStatus    dra;
        BAPE_CookStatus   cook;
        BAPE_AlsStatus    als;
    } codecStatus;
} BAPE_DecoderStatus;

typedef struct BAPE_DecoderAc4PresentationInfo
{
    unsigned index;                                             /* Index of this Presentation */
    char id[BAPE_AC4_PRESENTATION_ID_LENGTH];                   /* Unique Identifier for this Presentation */
    BAPE_Ac4AssociateType associateType;                        /* Describes what the associated program contains */
    char name[BAPE_AC4_PRESENTATION_NAME_LENGTH];               /* Name/Title of the Presentation */
    char language[BAPE_AC4_LANGUAGE_NAME_LENGTH];               /* Language of the Presentation */
} BAPE_DecoderAc4PresentationInfo;

/***************************************************************************
Summary:
Audio Presentation Info
***************************************************************************/
typedef struct BAPE_DecoderPresentationInfo
{
    BAVC_AudioCompressionStd codec;
    union
    {
        BAPE_DecoderAc4PresentationInfo ac4;
    } info;
} BAPE_DecoderPresentationInfo;

/***************************************************************************
Summary:
Get Audio Decoder Status
***************************************************************************/
void BAPE_Decoder_GetStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderStatus *pStatus     /* [out] */
    );

/***************************************************************************
Summary:
Get Presentation info by index. Call BAPE_Decoder_GetStatus first to get
the number of presentations, etc.
***************************************************************************/
void BAPE_Decoder_GetPresentationInfo(
    BAPE_DecoderHandle handle,
    unsigned presentationIndex,
    BAPE_DecoderPresentationInfo *pInfo     /* [out] */
    );

/***************************************************************************
Summary:
Audio Decoder Codec-Specific Settings
***************************************************************************/
typedef struct BAPE_DecoderCodecSettings
{
    BAVC_AudioCompressionStd codec;
    union
    {
        BAPE_Ac3Settings    ac3;
        BAPE_Ac3Settings    ac3Plus;
        BAPE_Ac4Settings    ac4;
        BAPE_MpegSettings   mpeg;
        BAPE_MpegSettings   mp3;
        BAPE_AacSettings    aac;       /* Applies to both ADTS/LOAS */
        BAPE_AacSettings    aacPlus;   /* Applies to both ADTS/LOAS */
        BAPE_WmaProSettings wmaPro;
        BAPE_DtsSettings    dts;       /* Applies to DTS, DTS-HD, DTS-Legacy */
        BAPE_AdpcmSettings  adpcm;
        BAPE_IlbcSettings   ilbc;
        BAPE_IsacSettings   isac;
        BAPE_AlsSettings    als;       /* MPEG-4 Audio Lossless Coding  */
    } codecSettings;
} BAPE_DecoderCodecSettings;

/***************************************************************************
Summary:
Get Audio Decoder Codec-Specific Settings
***************************************************************************/
void BAPE_Decoder_GetCodecSettings(
    BAPE_DecoderHandle handle,
    BAVC_AudioCompressionStd codec,
    BAPE_DecoderCodecSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder Codec-Specific Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetCodecSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    );

/***************************************************************************
Summary:
Get connector for decoder output data
***************************************************************************/
void BAPE_Decoder_GetConnector(
    BAPE_DecoderHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Decoder Interrupt Handlers
***************************************************************************/
typedef struct BAPE_DecoderInterruptHandlers
{
    /* Interrupt fires when first PTS is received */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } firstPts;
    /* Interrupt fires when TSM Fail (PTS Error) occurs */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } tsmFail;
    /* Interrupt fires when TSM transitions from fail -> pass in ASTM mode */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } tsmPass;
    /* Interrupt fires when the decoder receives the first or any new sample rate in the stream */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned sampleRate);
        void *pParam1;
        int param2;
    } sampleRateChange;
    /* Interrupt fires when the decoder receives the first or any new sample rate in the fmm */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned sampleRate);
        void *pParam1;
        int param2;
    } fmmSampleRateChange;
    /* Interrupt fires when the decoder achieves frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } lock;
    /* Interrupt fires when the decoder loses frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } unlock;
    /* Interrupt fires when the decoder status is ready for the host to read.
       Typically, this occurs after the first frame is successfully decoded. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } statusReady;
    /* Interrupt fires with stream channel mode (acmod) changes.
       Call BAPE_Decoder_GetStatus() from task context to determine
       latest status. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } modeChange;
    /* Interrupt fires with stream bitrate changes. Call BAPE_Decoder_GetStatus()
       from task context to determine latest status. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } bitrateChange;
    /* This interrupt fires when the CDB and/or ITB overflow */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } cdbItbOverflow;
    /* This interrupt fires when the CDB and/or ITB underflows during an active playback condition */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } cdbItbUnderflow;
    /* This interrupt fires when the active input has changed to an incompatible format and been halted.
       The application must call BAPE_Decoder_Stop() and BAPE_Decoder_Start() to resume processing. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } inputHalted;
    /* This interrupt fires when ancillary data has been added to the FIFO.  The application
       should call BAPE_Decoder_GetAncillaryDataBuffer() and BAPE_Decoder_ConsumeAncillaryData()
       to retrieve the data */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } ancillaryData;
    /* This interrupt fires when the Dialog Normalization values have changed.  The Application
       should call BAPE_Decoder_GetStatus() */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } dialnormChange;
    /* This interrupt fires when a new host buffer is ready in DecodeToMemory mode */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } hostBufferReady;
} BAPE_DecoderInterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BAPE_Decoder_GetInterruptHandlers(
    BAPE_DecoderHandle handle,
    BAPE_DecoderInterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers

Description:
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BAPE_Decoder_SetInterruptHandlers(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderInterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Get Default CDB/ITB configuration for decoding
***************************************************************************/
void BAPE_Decoder_GetDefaultCdbItbConfig(
    BAPE_DecoderHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    );

/***************************************************************************
Summary:
Treat input underflows as gaps in non-realtime mode.

Description:
In non-realtime mode, underflow conditions are not typically treated as a
gap in data and the decoder will stall waiting for more data.  This can
lead to deadlock scenarios if there is an actual gap in the data.  When
this routine is called, the decoder will enter a gap-fill mode and will
treat input underflows as missing frames until valid input data is received.

This routine can only be called if the decoder is started and running
in non-realtime mode.
***************************************************************************/
BERR_Code BAPE_Decoder_EnterUnderflowMode(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Ancillary Data Header
***************************************************************************/
typedef struct BAPE_AncillaryDataHeader
{
    unsigned blockSize;         /* Block size including this header and any padding */
    unsigned payloadSize;       /* Payload size in bytes */
    unsigned payloadSizeBits;   /* Payload size in bits */
    unsigned frameNumber;       /* Frame number */
} BAPE_AncillaryDataHeader;

/***************************************************************************
Summary:
Get Decoder Ancillary Data
***************************************************************************/
BERR_Code BAPE_Decoder_GetAncillaryDataBuffer(
    BAPE_DecoderHandle hDecoder,
    void **pBuffer, /* [out] pointer to ancillary data buffer */
    size_t *pSize   /* [out] size of data buffer in bytes */
    );

/***************************************************************************
Summary:
Consume Decoder Ancillary Data
***************************************************************************/
BERR_Code BAPE_Decoder_ConsumeAncillaryData(
    BAPE_DecoderHandle hDecoder,
    size_t numBytes
    );

/***************************************************************************
Summary:
Flush Decoder Ancillary Data
***************************************************************************/
void BAPE_Decoder_FlushAncillaryData(
    BAPE_DecoderHandle hDecoder
    );

/***************************************************************************
Summary:
Get Decoder Path Delay
***************************************************************************/
BERR_Code BAPE_Decoder_GetPathDelay_isr(
    BAPE_DecoderHandle hDecoder,
    unsigned *pDelay    /* [out] in ms */
    );

/***************************************************************************
Summary:
DecodeToMemory settings
***************************************************************************/
typedef struct BAPE_DecoderDecodeToMemorySettings
{
    unsigned maxBuffers;            /* Maximum number of buffers that can be queued at any time */
    unsigned maxSampleRate;         /* Maximum sample rate in Hz (default 48000) */
    unsigned bitsPerSample;         /* Sample size in bits - 8/16/32 are supported (default 16) */
    unsigned numPcmChannels;        /* Number of PCM channels (default 2 for stereo) */
    BAPE_Channel channelLayout[BAPE_Channel_eMax];  /* Layout of channels in memory */
} BAPE_DecoderDecodeToMemorySettings;

/***************************************************************************
Summary:
Get DecodeToMemory settings
***************************************************************************/
void BAPE_Decoder_GetDecodeToMemorySettings(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderDecodeToMemorySettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set DecodeToMemory settings

Description:
Set the decode to host settings for a decoder.  This can only be changed
while the decoder is stopped.
***************************************************************************/
BERR_Code BAPE_Decoder_SetDecodeToMemorySettings(
    BAPE_DecoderHandle hDecoder,
    const BAPE_DecoderDecodeToMemorySettings *pSettings
    );

/***************************************************************************
Summary:
DecodeToMemory status
***************************************************************************/
typedef struct BAPE_DecoderDecodeToMemoryStatus
{
    unsigned bufferSize;        /* Buffer size per frame in bytes */
    unsigned pendingBuffers;    /* Number of buffers queued for decoder output */
    unsigned completedBuffers;  /* Number of completed buffers waiting for host */
} BAPE_DecoderDecodeToMemoryStatus;

/***************************************************************************
Summary:
Get DecodeToMemory status
***************************************************************************/
BERR_Code BAPE_Decoder_GetDecodeToMemoryStatus(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderDecodeToMemoryStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
DecodeToMemory buffer descriptor
***************************************************************************/
typedef struct BAPE_DecoderBufferDescriptor
{
    size_t allocatedBytes;      /* Allocated buffer size in bytes */
    uint64_t memoryOffset;      /* Offset to memory */
    size_t filledBytes;         /* Amount filled by the decoder in bytes (may be zero) */
    unsigned sampleRate;        /* Sample rate in Hz */
    BAVC_PTSInfo ptsInfo;       /* PTS information (written by decoder)*/
} BAPE_DecoderBufferDescriptor;

/***************************************************************************
Summary:
Initialize DecodeToMemory buffer descriptor
***************************************************************************/
void BAPE_Decoder_InitBufferDescriptor(
    BAPE_DecoderBufferDescriptor *pDescriptor /* [out] */
    );

/***************************************************************************
Summary:
Queue a buffer for DecodeToMemory operation
***************************************************************************/
BERR_Code BAPE_Decoder_QueueBuffer(
    BAPE_DecoderHandle hDecoder,
    const BAPE_DecoderBufferDescriptor *pDescriptor
    );

/***************************************************************************
Summary:
Get completed buffers

Description:
This will return the completed frames from the audio decoder.  This is non-
destructive and the app must call BAPE_Decoder_ConsumeBuffers
to remove them from the queue.
***************************************************************************/
BERR_Code BAPE_Decoder_GetBuffers(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderBufferDescriptor *pBuffers, /* [out] */
    unsigned maxBuffers,
    unsigned *pNumBuffers/* [out] */
    );

/***************************************************************************
Summary:
Consume buffers returned from BAPE_Decoder_GetBuffers

Description:
This removes one or more completed buffers from the decoder's queue and
transfers ownership to the host.
***************************************************************************/
BERR_Code BAPE_Decoder_ConsumeBuffers(
    BAPE_DecoderHandle hDecoder,
    unsigned numBuffers
    );

#endif
