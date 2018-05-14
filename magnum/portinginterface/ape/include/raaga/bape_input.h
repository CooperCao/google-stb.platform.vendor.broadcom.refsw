/***************************************************************************
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#ifndef BAPE_INPUT_H_
#define BAPE_INPUT_H_

#define BAPE_MAX_DECODERS_PER_INPUT     4

/***************************************************************************
Summary:
I2S Input Handle
***************************************************************************/
typedef struct BAPE_I2sInput *BAPE_I2sInputHandle;

/***************************************************************************
Summary:
I2S Input Settings
***************************************************************************/
typedef struct BAPE_I2sInputSettings
{
    unsigned sampleRate;        /* Sample rate in Hz */
    unsigned bitsPerSample;     /* Values from 1 to 32 are supported. */

    BAPE_I2sJustification   justification;    /* Data Justification*/
    BAPE_I2sDataAlignment   dataAlignment;    /* Data Alignment */
    BAPE_I2sLRClockPolarity lrPolarity;       /* LRCK polarity */
    BAPE_I2sSclkPolarity    sclkPolarity;     /* SCLK polarity */
} BAPE_I2sInputSettings;

/***************************************************************************
Summary:
Get Default I2S Input Settings
***************************************************************************/
void BAPE_I2sInput_GetDefaultSettings(
    BAPE_I2sInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Open an I2S Input
***************************************************************************/
BERR_Code BAPE_I2sInput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_I2sInputSettings *pSettings,
    BAPE_I2sInputHandle *pHandle            /* [out] */
    );

/***************************************************************************
Summary:
Close an I2S Input
***************************************************************************/
void BAPE_I2sInput_Close(
    BAPE_I2sInputHandle handle
    );

/***************************************************************************
Summary:
Get I2S Input Settings
***************************************************************************/
void BAPE_I2sInput_GetSettings(
    BAPE_I2sInputHandle handle,
    BAPE_I2sInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Set I2S Input Settings
***************************************************************************/
BERR_Code BAPE_I2sInput_SetSettings(
    BAPE_I2sInputHandle handle,
    const BAPE_I2sInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get I2S Input Port Connector
***************************************************************************/
void BAPE_I2sInput_GetInputPort(
    BAPE_I2sInputHandle handle,
    BAPE_InputPort *pPort
    );

/***************************************************************************
Summary:
SPDIF Input Handle
***************************************************************************/
typedef struct BAPE_SpdifInput *BAPE_SpdifInputHandle;

/***************************************************************************
Summary:
SPDIF Input Error handling modes
***************************************************************************/
typedef enum BAPE_SpdifInputErrorInsertion
{
    BAPE_SpdifInputErrorInsertion_eNone,        /* No values inserted */
    BAPE_SpdifInputErrorInsertion_eZero,        /* Zeroes are inserted */
    BAPE_SpdifInputErrorInsertion_eRepeat,      /* Repeat the previous sample */
    BAPE_SpdifInputErrorInsertion_eMax
} BAPE_SpdifInputErrorInsertion;

/***************************************************************************
Summary:
SPDIF Input Settings
***************************************************************************/
typedef struct BAPE_SpdifInputSettings
{
    bool ignoreValidity;            /* If true, the validity bit will be ignored */
    bool ignorePcmParity;           /* If true, PCM parity errors will be ignored */
    bool ignoreCompressedParity;    /* If true, compressed parity errors will be ignored */
    BAPE_SpdifInputErrorInsertion errorInsertion;   /* Action to take on validity or parity errors*/
    BAVC_Timebase stcIndex;         /* STC counter to use for PTS insertion into PES packets */
} BAPE_SpdifInputSettings;

/***************************************************************************
Summary:
Get Default SPDIF Input Settings
***************************************************************************/
void BAPE_SpdifInput_GetDefaultSettings(
    BAPE_SpdifInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Open an SPDIF Input
***************************************************************************/
BERR_Code BAPE_SpdifInput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_SpdifInputSettings *pSettings,
    BAPE_SpdifInputHandle *pHandle              /* [out] */
    );

/***************************************************************************
Summary:
Close an SPDIF Input
***************************************************************************/
void BAPE_SpdifInput_Close(
    BAPE_SpdifInputHandle handle
    );

/***************************************************************************
Summary:
Get SPDIF Input Settings
***************************************************************************/
void BAPE_SpdifInput_GetSettings(
    BAPE_SpdifInputHandle handle,
    BAPE_SpdifInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Set SPDIF Input Settings
***************************************************************************/
BERR_Code BAPE_SpdifInput_SetSettings(
    BAPE_SpdifInputHandle handle,
    const BAPE_SpdifInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get SPDIF Input Port Connector
***************************************************************************/
void BAPE_SpdifInput_GetInputPort(
    BAPE_SpdifInputHandle handle,
    BAPE_InputPort *pPort
    );

/***************************************************************************
Summary:
SPDIF Format Detection Settings
***************************************************************************/
typedef struct BAPE_SpdifInputFormatDetectionSettings
{
    bool enabled;       /* Enable input format detection */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } formatChangeInterrupt;
} BAPE_SpdifInputFormatDetectionSettings;

/***************************************************************************
Summary:
Get SPDIF Format Detection Settings
***************************************************************************/
void BAPE_SpdifInput_GetFormatDetectionSettings(
    BAPE_SpdifInputHandle handle,
    BAPE_SpdifInputFormatDetectionSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set SPDIF Format Detection Settings
***************************************************************************/
BERR_Code BAPE_SpdifInput_SetFormatDetectionSettings(
    BAPE_SpdifInputHandle handle,
    const BAPE_SpdifInputFormatDetectionSettings *pSettings
    );

/***************************************************************************
Summary:
SPDIF Preamble C Data
***************************************************************************/
typedef struct BAPE_SpdifInputPreambleC
{
    BAVC_AudioCompressionStd codec;
    bool errors;                    /* If true, payload may contain errors */
    uint8_t bistreamNumber;         /* Bitstream Number */
    uint8_t typeDependentInfo;      /* Data Type Dependent Information */
} BAPE_SpdifInputPreambleC;

/***************************************************************************
Summary:
SPDIF Format Detection Status
***************************************************************************/
typedef struct BAPE_SpdifInputFormatDetectionStatus
{
    BAVC_AudioCompressionStd codec;
    unsigned numPcmChannels;    /* Total number of PCM audio channels, 0 for comnpressed streams */
    unsigned sampleRate;        /* Sample rate of the incoming stream */
    bool signalPresent;         /* If true, a signal is present */
    bool compressed;            /* If true, stream is compressed.  If false, stream is linear PCM */
    bool goodBiphase;       /* Incoming stream has good subframe preambles and biphase encoding transitions */
    bool leftValidity;      /* If true, validity bit is set for left channel data */
    bool rightValidity;     /* If true, validity bit is set for right channel data */

    bool      pcValid;      /* If true, preambleC is valid for compressed streams */

    bool detectionEnabled;      /* If true, detection is enabled */
} BAPE_SpdifInputFormatDetectionStatus;

/***************************************************************************
Summary:
Get SPDIF Format Detection Status
***************************************************************************/
BERR_Code BAPE_SpdifInput_GetFormatDetectionStatus(
    BAPE_SpdifInputHandle handle,
    BAPE_SpdifInputFormatDetectionStatus *pStatus
    );

/***************************************************************************
Summary:
HDMI Input Handle
***************************************************************************/
typedef struct BAPE_MaiInput *BAPE_MaiInputHandle;

/***************************************************************************
Summary:
HDMI Input Settings
***************************************************************************/
typedef struct BAPE_MaiInputSettings
{
    bool ignoreValidity;            /* If true, the validity bit will be ignored */
    bool ignorePcmParity;           /* If true, PCM parity errors will be ignored */
    bool ignoreCompressedParity;    /* If true, compressed parity errors will be ignored */
    bool hardwareMutingEnabled;     /* If true, HW auto mute will be enabled, if supported by the HW */
    BAPE_SpdifInputErrorInsertion errorInsertion;   /* Action to take on validity or parity errors*/
    unsigned stcIndex;              /* STC counter to use for PTS insertion into PES packets */
} BAPE_MaiInputSettings;

/***************************************************************************
Summary:
Get Default HDMI Input Settings
***************************************************************************/
void BAPE_MaiInput_GetDefaultSettings(
    BAPE_MaiInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Open an HDMI Input
***************************************************************************/
BERR_Code BAPE_MaiInput_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MaiInputSettings *pSettings,
    BAPE_MaiInputHandle *pHandle              /* [out] */
    );

/***************************************************************************
Summary:
Close an HDMI Input
***************************************************************************/
void BAPE_MaiInput_Close(
    BAPE_MaiInputHandle handle
    );

/***************************************************************************
Summary:
Get HDMI Input Settings
***************************************************************************/
void BAPE_MaiInput_GetSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputSettings *pSettings        /* [out] */
    );

/***************************************************************************
Summary:
Set HDMI Input Settings
***************************************************************************/
BERR_Code BAPE_MaiInput_SetSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get HDMI Input Port Connector
***************************************************************************/
void BAPE_MaiInput_GetInputPort(
    BAPE_MaiInputHandle handle,
    BAPE_InputPort *pPort
    );

/***************************************************************************
Summary:
HDMI Format Detection Settings
***************************************************************************/
typedef struct BAPE_MaiInputFormatDetectionSettings
{
    bool enabled;       /* Enable input format detection */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } formatChangeIntInput;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } formatChangeIntDecode[BAPE_MAX_DECODERS_PER_INPUT];
} BAPE_MaiInputFormatDetectionSettings;

/***************************************************************************
Summary:
Get HDMI Format Detection Settings
***************************************************************************/
void BAPE_MaiInput_GetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set HDMI Format Detection Settings
***************************************************************************/
BERR_Code BAPE_MaiInput_SetFormatDetectionSettings(
    BAPE_MaiInputHandle handle,
    const BAPE_MaiInputFormatDetectionSettings *pSettings
    );

/***************************************************************************
Summary:
HDMI Input Format
***************************************************************************/
typedef enum BAPE_MaiInputFormat
{
    BAPE_MaiInputFormat_eIdle,
    BAPE_MaiInputFormat_eMono,
    BAPE_MaiInputFormat_ePcmStereo,
    BAPE_MaiInputFormat_ePcm3Channel,
    BAPE_MaiInputFormat_ePcm5_1,
    BAPE_MaiInputFormat_eSpdifPcmStereo,
    BAPE_MaiInputFormat_eSpdifPcm6Channel,
    BAPE_MaiInputFormat_eSpdifPcm8Channel,
    BAPE_MaiInputFormat_eHbrCompressed,
    BAPE_MaiInputFormat_eHdmiOneBit,
    BAPE_MaiInputFormat_eHdmiDirectStreamTransfer,
    BAPE_MaiInputFormat_eHdmiPcmStereo,
    BAPE_MaiInputFormat_eHdmiNonLinearPcm,
    BAPE_MaiInputFormat_eSpdifLinearPcm,
    BAPE_MaiInputFormat_eSpdifNonLinearPcm,
    BAPE_MaiInputFormat_eMax
} BAPE_MaiInputFormat;

/***************************************************************************
Summary:
HDMI Format Detection Status
***************************************************************************/
typedef struct BAPE_MaiInputFormatDetectionStatus
{
    BAVC_AudioCompressionStd codec;
    unsigned numPcmChannels;    /* Total number of PCM audio channels, 0 for comnpressed streams */
    unsigned sampleRate;        /* Sample rate of the incoming stream */
    unsigned sampleWidth;       /* Sample width in bits */
    bool signalPresent;         /* If true, a signal is present */
    bool compressed;            /* If true, stream is compressed.  If false, stream is linear PCM */
    bool hbr;                   /* If true, stream is compressed.  If false, stream is linear PCM */
    bool pcValid;               /* If true, preambleC is valid for compressed streams */
    bool detectionEnabled;      /* If true, detection is enabled */
} BAPE_MaiInputFormatDetectionStatus;

/***************************************************************************
Summary:
Get HDMI Format Detection Status
***************************************************************************/
BERR_Code BAPE_MaiInput_GetFormatDetectionStatus(
    BAPE_MaiInputHandle handle,
    BAPE_MaiInputFormatDetectionStatus *pStatus
    );

#endif /* #ifndef BAPE_INPUT_H_ */
