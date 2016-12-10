/***************************************************************************
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
 *
 * Module Description: APE Types
 *
 ***************************************************************************/

#ifndef BAPE_TYPES_H_
#define BAPE_TYPES_H_

/***************************************************************************
Summary:
Device Handle
***************************************************************************/
typedef struct BAPE_Device *BAPE_Handle;

/***************************************************************************
Summary:
Mixer Input Representation
***************************************************************************/
typedef struct BAPE_PathConnector *BAPE_Connector;

/***************************************************************************
Summary:
Output Port Representation
***************************************************************************/
typedef struct BAPE_OutputPortObject *BAPE_OutputPort;

/***************************************************************************
Summary:
Input Port Representation
***************************************************************************/
typedef struct BAPE_InputPortObject *BAPE_InputPort;

/***************************************************************************
Summary:
Channel Modes
***************************************************************************/
typedef enum BAPE_ChannelMode
{
    BAPE_ChannelMode_e1_0,
    BAPE_ChannelMode_e1_1,
    BAPE_ChannelMode_e2_0,
    BAPE_ChannelMode_e3_0,
    BAPE_ChannelMode_e2_1,
    BAPE_ChannelMode_e3_1,
    BAPE_ChannelMode_e2_2,
    BAPE_ChannelMode_e3_2,
    BAPE_ChannelMode_e3_4,
    BAPE_ChannelMode_eMax
} BAPE_ChannelMode;

/***************************************************************************
Summary:
Channels
***************************************************************************/
typedef enum BAPE_Channel
{
    BAPE_Channel_eLeft,
    BAPE_Channel_eRight,
    BAPE_Channel_eLeftSurround,
    BAPE_Channel_eRightSurround,
    BAPE_Channel_eCenter,
    BAPE_Channel_eLfe,
    BAPE_Channel_eLeftRear,
    BAPE_Channel_eRightRear,
    BAPE_Channel_eMax
} BAPE_Channel;

/***************************************************************************
Summary:
Channel Pairs
***************************************************************************/
typedef enum BAPE_ChannelPair
{
    BAPE_ChannelPair_eLeftRight,
    BAPE_ChannelPair_eLeftRightSurround,
    BAPE_ChannelPair_eCenterLfe,
    BAPE_ChannelPair_eLeftRightRear,
    BAPE_ChannelPair_eMax
} BAPE_ChannelPair;

/***************************************************************************
Summary:
Stereo Mode
***************************************************************************/
typedef enum BAPE_StereoMode
{
    BAPE_StereoMode_eLeftRight,
    BAPE_StereoMode_eLeftLeft,
    BAPE_StereoMode_eRightRight,
    BAPE_StereoMode_eRightLeft
} BAPE_StereoMode;

/***************************************************************************
Summary:
Dual Mono Modes
***************************************************************************/
typedef enum BAPE_DualMonoMode
{
    BAPE_DualMonoMode_eStereo,
    BAPE_DualMonoMode_eLeft,
    BAPE_DualMonoMode_eRight,
    BAPE_DualMonoMode_eMix,
    BAPE_DualMonoMode_eMax
} BAPE_DualMonoMode;

/***************************************************************************
Summary:
Mono Channel Select
***************************************************************************/
typedef enum BAPE_MonoChannelMode
{
    BAPE_MonoChannelMode_eLeft,
    BAPE_MonoChannelMode_eRight,
    BAPE_MonoChannelMode_eMix,
    BAPE_MonoChannelMode_eMax
} BAPE_MonoChannelMode;

/***************************************************************************
Summary:
SPDIF Burst Types
***************************************************************************/
typedef enum BAPE_SpdifBurstType
{
    BAPE_SpdifBurstType_eNull,
    BAPE_SpdifBurstType_ePause,
    BAPE_SpdifBurstType_eNone,
    BAPE_SpdifBurstType_eMax
} BAPE_SpdifBurstType;

/***************************************************************************
Summary:
SPDIF Output Formatter Settings
***************************************************************************/
typedef struct BAPE_SpdifChannelStatus
{
    bool separateLeftRight;     /* Use different channel number bits for left and right channels (Bits 20..21) */
    bool professional;          /* Bit 0 */
    bool copyright;             /* Bit 2 */
    uint8_t formatInformation;  /* Bits 3..5 */
    uint8_t categoryCode;       /* Bits 8..15 */
    uint8_t sourceNumber;       /* Bits 16..19 */
    uint8_t clockAccuracy;      /* bits 28..29 */
    uint8_t cgmsA;              /* bits 40..41 */
} BAPE_SpdifChannelStatus;

/***************************************************************************
Summary:
SCLK (Bit Clock) Rate
***************************************************************************/
typedef enum BAPE_SclkRate
{
    BAPE_SclkRate_e64Fs,
    BAPE_SclkRate_e128Fs,
    BAPE_SclkRate_eMax
} BAPE_SclkRate;

/***************************************************************************
Summary:
MCLK (Master Clock) Rate
***************************************************************************/
typedef enum BAPE_MclkRate
{
    BAPE_MclkRate_eAuto,
    BAPE_MclkRate_e128Fs,
    BAPE_MclkRate_e256Fs,
    BAPE_MclkRate_e384Fs,
    BAPE_MclkRate_e512Fs,
    BAPE_MclkRate_eMax
} BAPE_MclkRate;

/***************************************************************************
Summary:
PLL Indexes
***************************************************************************/
typedef enum BAPE_Pll
{
    BAPE_Pll_e0,
    BAPE_Pll_e1,
    BAPE_Pll_e2,
    BAPE_Pll_eMax
} BAPE_Pll;

/***************************************************************************
Summary:
PLL Indexes
***************************************************************************/
typedef enum BAPE_Nco
{
    BAPE_Nco_e0,
    BAPE_Nco_e1,
    BAPE_Nco_e2,
    BAPE_Nco_e3,
    BAPE_Nco_e4,
    BAPE_Nco_e5,
    BAPE_Nco_e6,
    BAPE_Nco_eMax
} BAPE_Nco;

/***************************************************************************
Summary:
I2S Data Justification
***************************************************************************/
typedef enum BAPE_I2sJustification
{
    BAPE_I2sJustification_eLsbFirst,    /* LSB is at the LRCK transition */
    BAPE_I2sJustification_eMsbFirst,    /* MSB is at the LRCK transition */
    BAPE_I2sJustification_eMax
} BAPE_I2sJustification;

/***************************************************************************
Summary:
I2S Data Alignment
***************************************************************************/
typedef enum BAPE_I2sDataAlignment
{
    BAPE_I2sDataAlignment_eAligned,     /* Data is aligned to LRCK */
    BAPE_I2sDataAlignment_eDelayed,     /* Data is delayed one SCLK from LRCK */
    BAPE_I2sDataAlignment_eMax
} BAPE_I2sDataAlignment;

/***************************************************************************
Summary:
I2S LRCK Polarity
***************************************************************************/
typedef enum BAPE_I2sLRClockPolarity
{
    BAPE_I2sLRClockPolarity_eLeftHigh,  /* Left is high on LRCK */
    BAPE_I2sLRClockPolarity_eLeftLow,   /* Left is low on LRCK */
    BAPE_I2sLRClockPolarity_eMax
} BAPE_I2sLRClockPolarity;

/***************************************************************************
Summary:
I2S SCLK Polarity
***************************************************************************/
typedef enum BAPE_I2sSclkPolarity
{
    BAPE_I2sSclkPolarity_eRising,   /* Rising edge aligned with SDATA */
    BAPE_I2sSclkPolarity_eFalling,   /* Falling edge aligned with SDATA */
    BAPE_I2sSclkPolarity_eMax
} BAPE_I2sSclkPolarity;

/***************************************************************************
Summary:
Multichannel I2S Mode
***************************************************************************/
typedef enum BAPE_I2sMultiMode
{
    BAPE_I2sMultiMode_eMultichannel,    /* Default.  I2S multi will be used 
                                           as a single output capable of stereo
                                           or multichannel. */
    BAPE_I2sMultiMode_eStereo,          /* I2S multi will be used as a set of
                                           discrete stereo outputs that share
                                           a common sample clock and word select. */
    BAPE_I2sMultiMode_eMax
} BAPE_I2sMultiMode;

/***************************************************************************
Summary:
Normal volume (0dB)
***************************************************************************/
#define BAPE_VOLUME_NORMAL (0x800000)

/***************************************************************************
Summary:
Minimum volume (mute)
***************************************************************************/
#define BAPE_VOLUME_MIN    (0)

/***************************************************************************
Summary:
Buffer Descriptor
***************************************************************************/
typedef struct BAPE_BufferDescriptor
{
    bool interleaved;               /* If true, every other channel will have valid pointers,
                                       e.g. L for L/R, Ls for Ls/Rs, etc.  */
    unsigned numBuffers;            /* Number of buffers.  For mono/interleaved stereo this is 1.  For
                                       non-interleaved stereo, it's 2.  For non-interleaved 7.1 it's 8. */
    struct
    {
        void *pBuffer;              /* Buffer base address prior to wraparound */
        void *pWrapBuffer;          /* Buffer address after wraparound (NULL if no wrap has occurred) */
    } buffers[BAPE_Channel_eMax];

    unsigned bufferSize;            /* Buffer size before wraparound in bytes */
    unsigned wrapBufferSize;        /* Buffer size after wraparound in bytes */        
} BAPE_BufferDescriptor;

/***************************************************************************
Summary:
Multichannel Formats
***************************************************************************/
typedef enum BAPE_MultichannelFormat
{
    BAPE_MultichannelFormat_e2_0,
    BAPE_MultichannelFormat_e5_1,
    BAPE_MultichannelFormat_e7_1,
    BAPE_MultichannelFormat_eMax
} BAPE_MultichannelFormat;

/***************************************************************************
Summary:
Data Path Types 
 
Description: 
Some object types expose multiple data path connectors.  This will specify 
the data type of the connector. 
***************************************************************************/
typedef enum BAPE_ConnectorFormat
{
    BAPE_ConnectorFormat_eStereo,           /* Stereo PCM */
    BAPE_ConnectorFormat_eMultichannel,     /* Multichannel PCM */
    BAPE_ConnectorFormat_eCompressed,       /* Compressed IEC-61937 audio up to 48 kHz for SPDIF/HDMI applications.  */
    BAPE_ConnectorFormat_eCompressed4x,     /* Compressed IEC-61937 audio for HDMI up to 192kHz.  Typically used for AC3+ and DTS-HD HRA audio formats. */
    BAPE_ConnectorFormat_eCompressed16x,    /* Compressed IEC-61937 audio for HDMI HBR packets up to 768kHz.  Typically used for MAT/MLP (Dolby TrueHD) 
                                               and DTS-HD MA audio formats.  Available in HDMI 1.3 and later. */
    BAPE_ConnectorFormat_eMono,             /* Mono PCM, typically only used for voice conferencing applications. */
    BAPE_ConnectorFormat_eMax
} BAPE_ConnectorFormat;

/***************************************************************************
Summary:
DSP Delay Mode
 
Description: 
Determines the DSP's delay mode for an operation.  Default uses a
fixed path delay, but low delay mode provides lower delay depending on the
codec in use.  There are also usage restrictions for low delay mode in
terms of what post-processing can be performed as well as the number of
concurrent DSP tasks in use.
***************************************************************************/
typedef enum BAPE_DspDelayMode
{
    BAPE_DspDelayMode_eDefault,     /* Fixed path delay */
    BAPE_DspDelayMode_eLowFixed,    /* Low, Fixed path delay */
    BAPE_DspDelayMode_eLowVariable, /* Variable low path delay, latency varies by codec */
    BAPE_DspDelayMode_eMax
} BAPE_DspDelayMode;

/***************************************************************************
Summary:
RF Audio Encodings
**************************************************************************/
typedef enum BAPE_RfAudioEncoding
{
    BAPE_RfAudioEncoding_eBtsc,
    BAPE_RfAudioEncoding_eMax
} BAPE_RfAudioEncoding;

/***************************************************************************
Summary:
Loudness Equivalence Modes
**************************************************************************/
typedef enum BAPE_LoudnessEquivalenceMode
{
    BAPE_LoudnessEquivalenceMode_eNone,         /* Default, no loudness equivalence */
    BAPE_LoudnessEquivalenceMode_eAtscA85,      /* ATSC A/85.  This standardizes all decoders to output 
                                                   Stereo and Multichannel PCM at -20dB.  Compressed data is output at -31dB. 
                                                   The application must set the volume for outputs accordingly so
                                                   that PCM stereo is output to "active" outputs such as DAC at -23 dB 
                                                   and PCM is sent to "passive" outputs such as SPDIF at -31dB.  This
                                                   can be done by calling BAPE_SetOutputVolume() for active outputs
                                                   with the value 0x5A9DF7 (-3dB) and passive outputs with the
                                                   value 0x241346 (-11dB). */
    BAPE_LoudnessEquivalenceMode_eEbuR128,      /* EBU-R128.  This standardizes Dolby decoders to output 
                                                   Stereo and Multichannel PCM at -20dB.  Non-Dolby decoders will
                                                   output Stereo and Multichannel PCM at -23dB.  All encoders and 
                                                   passthrough configurations will output compressed at -31dB. 
                                                   The application must set the volume for outputs accordingly so
                                                   that PCM stereo is output to "active" outputs such as DAC at -23 dB 
                                                   and PCM is sent to "passive" outputs such as SPDIF at -31dB.  This
                                                   can be done by calling BAPE_SetOutputVolume() for active outputs
                                                   with the value 0x5A9DF7 (-3dB) and passive outputs with the
                                                   value 0x241346 (-11dB) for Dolby codecs.  For non-Dolby codecs, use 
                                                   the value 0x800000 (-0dB) for active outputs and 0x32F52C (-8dB) for
                                                   passive outputs.  */
    BAPE_LoudnessEquivalenceMode_eMax
} BAPE_LoudnessEquivalenceMode;

/***************************************************************************
Summary:
Dolby MS Version
**************************************************************************/
typedef enum BAPE_DolbyMSVersion
{
    BAPE_DolbyMSVersion_eNone,
    BAPE_DolbyMSVersion_eMS10,
    BAPE_DolbyMSVersion_eMS11,
    BAPE_DolbyMSVersion_eMS12,
    BAPE_DolbyMSVersion_eMax
} BAPE_DolbyMSVersion;

/***************************************************************************
Summary:
Dolby MS12 Config
**************************************************************************/
typedef enum BAPE_DolbyMs12Config {
    BAPE_DolbyMs12Config_eNone,
    BAPE_DolbyMs12Config_eA,
    BAPE_DolbyMs12Config_eB,
    BAPE_DolbyMs12Config_eC
} BAPE_DolbyMs12Config;

/***************************************************************************
Summary:
Post Processor Type
***************************************************************************/
typedef enum BAPE_PostProcessorType
{
    BAPE_PostProcessorType_eDdre,
    BAPE_PostProcessorType_eDolbyVolume,
    BAPE_PostProcessorType_eTruSurround,
    BAPE_PostProcessorType_eTruVolume,
    BAPE_PostProcessorType_e3dSurround,
    BAPE_PostProcessorType_eDolbyDigitalReencoder,
    BAPE_PostProcessorType_eAutoVolumeLevel,
    BAPE_PostProcessorType_eCustom,
    BAPE_PostProcessorType_eFade,
    BAPE_PostProcessorType_eKaraokeVocal,
    BAPE_PostProcessorType_eMax
} BAPE_PostProcessorType;

/***************************************************************************
Summary:
Fade Settings
***************************************************************************/
typedef struct BAPE_FadeSettings
{
    unsigned level;             /* Percentage representing the volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
    unsigned duration;          /* duration in milliseconds it will take to change
                                   to a new level. Valid values are 3 - 1000 */
    unsigned type;              /* specifies the type of fade -
                                   0- Linear (Default), 1-Cubic-In, 2-Cubic-Out. */
} BAPE_FadeSettings;

/***************************************************************************
Summary:
Fade Status
***************************************************************************/
typedef struct BAPE_FadeStatus
{
    /* Fade Effect Status */
    bool active;
    unsigned remaining;         /* MilliSeconds remaining in the current active fade */
    unsigned level;             /* Percentage representing the current volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
} BAPE_FadeStatus;

#endif /* #ifndef BAPE_TYPES_H_ */
