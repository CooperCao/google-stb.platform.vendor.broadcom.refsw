/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_OUTPUT_H_
#define BAPE_OUTPUT_H_

/***************************************************************************
Summary:
Audio DAC Handle
***************************************************************************/
typedef struct BAPE_Dac *BAPE_DacHandle;

/***************************************************************************
Summary:
Audio DAC Mute Modes
***************************************************************************/
typedef enum BAPE_DacMuteType
{
    BAPE_DacMuteType_eDrive0,
    BAPE_DacMuteType_eDriveNegative1,
    BAPE_DacMuteType_eCustom,
    BAPE_DacMuteType_eMax
} BAPE_DacMuteType;

/***************************************************************************
Summary:
Audio DAC Settings
***************************************************************************/
typedef struct BAPE_DacSettings
{
    BAPE_StereoMode stereoMode;         /* Mode to handle stereo mixer ouptut */
    BAPE_DacMuteType muteType;          /* Mute mode. */
    uint16_t customLeftValue;           /* 16-bit left sample, used only with custom mute type */
    uint16_t customRightValue;          /* 16-bit right sample, used only with custom mute type */
    int fineAdjustment;                 /* fine adjustment of DAC output level - 28nm and newer chips only.
                                           valid values are -100 to 100. 0 means no adjustment, positive values boost the level,
                                           negative values cut the level. Integer steps are 0.005dBV. Total range is +/-0.5dBV */
    struct
    {
        bool enabled;           /* If true, test tone output is enabled with the settings below.  If false, the tone output is disabled. */
        int32_t samples[64];    /* Test tone samples.  The samples are 20-bit signed data. */
        bool zeroOnLeft;        /* If true, the left channel will output zeroes */
        bool zeroOnRight;       /* If true, the right channel will output zeroes */
        bool sharedSamples;     /* If true (default), samples 0..63 will be output on both Left and Right.
                                   If false, samples 0..31 will output on Left and 32..63 on Right. */
        unsigned numSamplesLeft;            /* Number of samples to play on the left channel before repeating (1..64) */
        unsigned numSamplesRight;           /* Number of samples to play on the right channel before repeating (1..64) */
        unsigned sampleRate;                /* Sampling frequency of the samples */
    } testTone;
} BAPE_DacSettings;

/***************************************************************************
Summary:
Get Default Audio DAC Settings
***************************************************************************/
void BAPE_Dac_GetDefaultSettings(
    BAPE_DacSettings *pSettings
    );

/***************************************************************************
Summary:
Open an audio DAC
***************************************************************************/
BERR_Code BAPE_Dac_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DacSettings *pSettings,
    BAPE_DacHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close an audio DAC
***************************************************************************/
void BAPE_Dac_Close(
    BAPE_DacHandle handle
    );

/***************************************************************************
Summary:
Get DAC Settings
***************************************************************************/
void BAPE_Dac_GetSettings(
    BAPE_DacHandle handle,
    BAPE_DacSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set DAC Settings
***************************************************************************/
BERR_Code BAPE_Dac_SetSettings(
    BAPE_DacHandle handle,
    const BAPE_DacSettings *pSettings
    );

/***************************************************************************
Summary:
Get mixer output connector for a DAC
***************************************************************************/
void BAPE_Dac_GetOutputPort(
    BAPE_DacHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
Power Up the DAC
***************************************************************************/
void BAPE_Dac_PowerUp(
    BAPE_DacHandle handle
    );

/***************************************************************************
Summary:
Power Down the DAC
***************************************************************************/
void BAPE_Dac_PowerDown(
    BAPE_DacHandle handle
    );

/***************************************************************************
Summary:
SPDIF Output Handle
***************************************************************************/
typedef struct BAPE_SpdifOutput *BAPE_SpdifOutputHandle;

/***************************************************************************
Summary:
SPDIF Output Settings
***************************************************************************/
typedef struct BAPE_SpdifOutputSettings
{
    BAPE_StereoMode stereoMode;   /* Mode to handle stereo mixer ouptut.  Ignored with compressed input. */
    bool ditherEnabled;           /* If true, a dither signal will be sent
                                     out when there is no PCM data.  Every
                                     192 samples, a zero PCM sample will be overwritten
                                     with one.  Non-zero or compressed samples are ignored.  */
    bool limitTo16Bits;           /* If true, PCM data will be limited to 16 bits */
    BAPE_SpdifBurstType underflowBurst; /* A burst of the specified type will be sent 
                                           if the input underflows.  */

    bool useRawChannelStatus;     /* If true, values from rawChannelStatus will be used instead of channelStatus */
    BAPE_SpdifChannelStatus channelStatus;
    uint8_t rawChannelStatus[24]; /* Programs the channel status directly, sample rate and other values will not be set internally */
    bool allow4xCompressed;       /* Allow support for 61937x4 compressed mode */
} BAPE_SpdifOutputSettings;

/***************************************************************************
Summary:
Get Default SPDIF Output Settings
***************************************************************************/
void BAPE_SpdifOutput_GetDefaultSettings(
    BAPE_SpdifOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Open a SPDIF Output
***************************************************************************/
BERR_Code BAPE_SpdifOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_SpdifOutputSettings *pSettings,
    BAPE_SpdifOutputHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close a SPDIF Output
***************************************************************************/
void BAPE_SpdifOutput_Close(
    BAPE_SpdifOutputHandle handle
    );

/***************************************************************************
Summary:
Get SPDIF Output Settings
***************************************************************************/
void BAPE_SpdifOutput_GetSettings(
    BAPE_SpdifOutputHandle handle,
    BAPE_SpdifOutputSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set SPDIF Output Settings
***************************************************************************/
BERR_Code BAPE_SpdifOutput_SetSettings(
    BAPE_SpdifOutputHandle handle,
    const BAPE_SpdifOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get mixer output connector for SPDIF
***************************************************************************/
void BAPE_SpdifOutput_GetOutputPort(
    BAPE_SpdifOutputHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
MAI (HDMI Output) Handle
***************************************************************************/
typedef struct BAPE_MaiOutput *BAPE_MaiOutputHandle;

/***************************************************************************
Summary:
Loudness Output Mode
***************************************************************************/
typedef enum BAPE_OutputLoudnessType
{
    BAPE_OutputLoudnessType_Active,     /* Output is considered active and will be output at -23/24dB */
    BAPE_OutputLoudnessType_Passive,    /* Output is considered passive and will be output at -31dB */
    BAPE_OutputLoudnessType_Max
} BAPE_OutputLoudnessType;

/***************************************************************************
Summary:
MAI (HDMI Output) Settings
***************************************************************************/
typedef struct BAPE_MaiOutputSettings
{
    BAPE_StereoMode stereoMode; /* Mode to handle stereo mixer ouptut.  Ignored with compressed or multichannel input. */
    bool ditherEnabled;     /* If true, a dither signal will be sent 
                               out when there is no PCM data.  Every 
                               192 samples, a zero PCM sample will be overwritten
                               with one.  Non-zero or compressed samples are ignored.  */
    BAPE_SpdifBurstType underflowBurst; /* A burst of the specified type will be sent 
                                           if the input underflows.  */

    unsigned burstPadding;  /* Choose to pad to the end of pause bursts with additional words.  
                               Valid values are either 0 or 2.  Default is 0 */

    bool useRawChannelStatus;           /* If true, values from rawChannelStatus will be used instead of channelStatus */
    BAPE_SpdifChannelStatus channelStatus;
    BAPE_OutputLoudnessType loudnessType;     /* Specifies if HDMI should be configured as an active or passive output */
    uint8_t rawChannelStatus[24];       /* Programs the channel status directly, sample rate and other values will not be set internally */
    bool lowLatencyPassthroughEnabled;  /* On chips that support Low Latency Passthrough, it will be enabled by default,
                                           but can be disabled by controlling API. */
    bool hardwareMutingEnabled;         /* Allow the HW to manage automatic mute functionality when upstream changes
                                           occur from HDMI Rx source - requires lowLatencyPassthroughEnabled to also
                                           be set to true */
} BAPE_MaiOutputSettings;

/***************************************************************************
Summary:
Get Default MAI (HDMI Output) Settings
***************************************************************************/
void BAPE_MaiOutput_GetDefaultSettings(
    BAPE_MaiOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Open a MAI Output (HDMI)
***************************************************************************/
BERR_Code BAPE_MaiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MaiOutputSettings *pSettings,
    BAPE_MaiOutputHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close a MAI Output (HDMI)
***************************************************************************/
void BAPE_MaiOutput_Close(
    BAPE_MaiOutputHandle handle
    );

/***************************************************************************
Summary:
Get MAI Output (HDMI) Settings
***************************************************************************/
void BAPE_MaiOutput_GetSettings(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set MAI Output (HDMI) Settings
***************************************************************************/
BERR_Code BAPE_MaiOutput_SetSettings(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get mixer output connector for MAI Output (HDMI)
***************************************************************************/
void BAPE_MaiOutput_GetOutputPort(
    BAPE_MaiOutputHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
HDMI output interrupts
***************************************************************************/
typedef struct BAPE_MaiOutputInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned sampleRate);
        void *pParam1;
        int param2;        
    } sampleRate;
} BAPE_MaiOutputInterruptHandlers;

/***************************************************************************
Summary:
Get HDMI output interrupts
***************************************************************************/
void BAPE_MaiOutput_GetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    BAPE_MaiOutputInterruptHandlers *pInterrupts    /* [out] */
    );

/***************************************************************************
Summary:
Set HDMI output interrupts
***************************************************************************/
BERR_Code BAPE_MaiOutput_SetInterruptHandlers(
    BAPE_MaiOutputHandle handle,
    const BAPE_MaiOutputInterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
I2S Output Handle
***************************************************************************/
typedef struct BAPE_I2sOutput *BAPE_I2sOutputHandle;

/***************************************************************************
Summary:
I2S Output Settings
***************************************************************************/
typedef struct BAPE_I2sOutputSettings
{
    BAPE_StereoMode stereoMode; /* Mode to handle stereo mixer ouptut. */

    BAPE_I2sJustification   justification;    /* Data Justification*/
    BAPE_I2sDataAlignment   dataAlignment;    /* Data Alignment */
    BAPE_I2sLRClockPolarity lrPolarity;       /* LRCK polarity */
    BAPE_I2sSclkPolarity    sclkPolarity;     /* SCLK polarity */

    BAPE_SclkRate           sclkRate;         /* Bit clock Rate in multiple of Fs */
    bool                    allowCompressed; /* Enable support for 61937 and 61937x4 compressed modes */
} BAPE_I2sOutputSettings;

/***************************************************************************
Summary:
Get Default I2S Output Settings
***************************************************************************/
void BAPE_I2sOutput_GetDefaultSettings(
    BAPE_I2sOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Open an I2S Output
***************************************************************************/
BERR_Code BAPE_I2sOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sOutputSettings *pSettings,
    BAPE_I2sOutputHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close an I2S Output
***************************************************************************/
void BAPE_I2sOutput_Close(
    BAPE_I2sOutputHandle handle
    );

/***************************************************************************
Summary:
Get I2S Output Settings
***************************************************************************/
void BAPE_I2sOutput_GetSettings(
    BAPE_I2sOutputHandle handle,
    BAPE_I2sOutputSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set I2S Output Settings
***************************************************************************/
BERR_Code BAPE_I2sOutput_SetSettings(
    BAPE_I2sOutputHandle handle,
    const BAPE_I2sOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get mixer output connector for I2S
***************************************************************************/
void BAPE_I2sOutput_GetOutputPort(
    BAPE_I2sOutputHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
Multichannel I2S Output Handle
***************************************************************************/
typedef struct BAPE_I2sMultiOutput *BAPE_I2sMultiOutputHandle;

/***************************************************************************
Summary:
Multichannel I2S Output Settings
***************************************************************************/
typedef struct BAPE_I2sMultiOutputSettings
{
    BAPE_I2sMultiMode mode; /* Mode of operation, multichannel or stereo.
                               Must be set prior to BAPE_I2sMultiOutput_Open() 
                               and cannot be changed on the fly. */

    BAPE_I2sOutputSettings i2sSettings; /* I2S Settings */
} BAPE_I2sMultiOutputSettings;

/***************************************************************************
Summary:
Get Default Multichannel I2S Output Settings
***************************************************************************/
void BAPE_I2sMultiOutput_GetDefaultSettings(
    BAPE_I2sMultiOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Open a Multichannel I2S Output
***************************************************************************/
BERR_Code BAPE_I2sMultiOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sMultiOutputSettings *pSettings,
    BAPE_I2sMultiOutputHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close a Multichannel I2S Output
***************************************************************************/
void BAPE_I2sMultiOutput_Close(
    BAPE_I2sMultiOutputHandle handle
    );

/***************************************************************************
Summary:
Get Multichannel I2S Output Settings
***************************************************************************/
void BAPE_I2sMultiOutput_GetSettings(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_I2sMultiOutputSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Multichannel I2S Output Settings
***************************************************************************/
BERR_Code BAPE_I2sMultiOutput_SetSettings(
    BAPE_I2sMultiOutputHandle handle,
    const BAPE_I2sMultiOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get mixer output connector for Multichannel I2S or the first stereo pair 
if set to stereo mode. 
***************************************************************************/
void BAPE_I2sMultiOutput_GetOutputPort(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
Get mixer output connector for an individual stereo pair on Multichannel I2S. 
Only supported if BAPE_I2sMultiOutputSettings.mode is set to 
BAPE_I2sMultiMode_eStereo. 
***************************************************************************/
void BAPE_I2sMultiOutput_GetStereoOutputPort(
    BAPE_I2sMultiOutputHandle handle,
    BAPE_ChannelPair channelPair,
    BAPE_OutputPort *pConnector        /* [out] */
    );

/***************************************************************************
Summary:
RF Modulator Handle
***************************************************************************/
typedef struct BAPE_RfMod *BAPE_RfModHandle;

/***************************************************************************
Summary:
RF Modulator Settings
***************************************************************************/
typedef struct BAPE_RfModSettings
{
    BAPE_OutputPort master;    /* The RF Modulator must slave to a DAC output */
    bool muted;                 /* If true, the RF Modulator will be muted */
} BAPE_RfModSettings;

/***************************************************************************
Summary:
Get Default RF Modulator Settings
***************************************************************************/
void BAPE_RfMod_GetDefaultSettings(
    BAPE_RfModSettings *pSettings
    );

/***************************************************************************
Summary:
Open an RF Modulator
***************************************************************************/
BERR_Code BAPE_RfMod_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_RfModSettings *pSettings,
    BAPE_RfModHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close an RF Modulator
***************************************************************************/
void BAPE_RfMod_Close(
    BAPE_RfModHandle handle
    );

/***************************************************************************
Summary:
Get RF Modulator Settings
***************************************************************************/
void BAPE_RfMod_GetSettings(
    BAPE_RfModHandle handle,
    BAPE_RfModSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set RF Modulator Settings
***************************************************************************/
BERR_Code BAPE_RfMod_SetSettings(
    BAPE_RfModHandle handle,
    const BAPE_RfModSettings *pSettings
    );

/***************************************************************************
Summary:
Audio Return Channel Handle
***************************************************************************/
typedef struct BAPE_AudioReturnChannel *BAPE_AudioReturnChannelHandle;

/***************************************************************************
Summary:
Audio Return Channel Settings
***************************************************************************/
typedef struct BAPE_AudioReturnChannelSettings
{
    BAPE_OutputPort master;    /* The Audio Return Channel must slave to a Spdif output */
    bool muted;                 /* If true, the Audio Return Channel will be muted */
} BAPE_AudioReturnChannelSettings;

/***************************************************************************
Summary:
Get Default Audio Return Channel Settings
***************************************************************************/
void BAPE_AudioReturnChannel_GetDefaultSettings(
    BAPE_AudioReturnChannelSettings *pSettings
    );

/***************************************************************************
Summary:
Open an Audio Return Channel
***************************************************************************/
BERR_Code BAPE_AudioReturnChannel_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_AudioReturnChannelSettings *pSettings,
    BAPE_AudioReturnChannelHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close an Audio Return Channel
***************************************************************************/
void BAPE_AudioReturnChannel_Close(
    BAPE_AudioReturnChannelHandle handle
    );

/***************************************************************************
Summary:
Get Audio Return Channel Settings
***************************************************************************/
void BAPE_AudioReturnChannel_GetSettings(
    BAPE_AudioReturnChannelHandle handle,
    BAPE_AudioReturnChannelSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Return Channel Settings
***************************************************************************/
BERR_Code BAPE_AudioReturnChannel_SetSettings(
    BAPE_AudioReturnChannelHandle handle,
    const BAPE_AudioReturnChannelSettings *pSettings
    );

/***************************************************************************
Summary:
Dummy Output Handle
***************************************************************************/
typedef struct BAPE_DummyOutput *BAPE_DummyOutputHandle;

/***************************************************************************
Summary:
Dummy Output Settings
***************************************************************************/
typedef struct BAPE_DummyOutputOpenSettings
{
    BAPE_MultichannelFormat maxMultichannelFormat;   /* Set to the worst case multichannel format
                                                        you would like to handle.  Controls the
                                                        number of HW resources allocated. */
} BAPE_DummyOutputOpenSettings;

/***************************************************************************
Summary:
Get Default Dummy Output Settings
***************************************************************************/
void BAPE_DummyOutput_GetDefaultOpenSettings(
    BAPE_DummyOutputOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Open a Dummy Output
***************************************************************************/
BERR_Code BAPE_DummyOutput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DummyOutputOpenSettings *pSettings,
    BAPE_DummyOutputHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
Close a Dummy Output
***************************************************************************/
void BAPE_DummyOutput_Close(
    BAPE_DummyOutputHandle handle
    );

/***************************************************************************
Summary:
Get mixer output connector for a Dummy Output
***************************************************************************/
void BAPE_DummyOutput_GetOutputPort(
    BAPE_DummyOutputHandle handle,
    BAPE_OutputPort *pConnector        /* [out] */
    );

#endif /* #ifndef BAPE_OUTPUT_H_ */

