/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/

#ifndef NEXUS_AUDIO_RF_ENCODER_H__
#define NEXUS_AUDIO_RF_ENCODER_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for an RF Audio Encoder
**/
typedef struct NEXUS_RfAudioEncoder *NEXUS_RfAudioEncoderHandle;

/***************************************************************************
Summary:
Audio Encoder Settings
    
Description:
Delay mode is set in AudioDecoder
***************************************************************************/
typedef struct NEXUS_RfAudioEncoderSettings
{
    NEXUS_RfAudioEncoding encoding;     /* The encoding format to use.  Can only be changed while the data flow is stopped. */
} NEXUS_RfAudioEncoderSettings;

/***************************************************************************
Summary:
    Get default settings for an RF Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetDefaultSettings(
    NEXUS_RfAudioEncoderSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an RF Audio Encoder stage
***************************************************************************/
NEXUS_RfAudioEncoderHandle NEXUS_RfAudioEncoder_Open( /* attr{destructor=NEXUS_RfAudioEncoder_Close}  */
    const NEXUS_RfAudioEncoderSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close an RF Audio Encoder stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_RfAudioEncoder_Close(
    NEXUS_RfAudioEncoderHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an RF Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoderSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an RF Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_SetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderSettings *pSettings
    );

/***************************************************************************
Summary:
BTSC RF Encoding Modes
***************************************************************************/
typedef enum NEXUS_BtscEncodingMode
{
    NEXUS_BtscEncodingMode_eStereo, /* Default will output L+R and L-R channels with the pilot tone enabled */
    NEXUS_BtscEncodingMode_eMono,   /* Mono will output L+R only */
    NEXUS_BtscEncodingMode_eMax
} NEXUS_BtscEncodingMode;
 
/***************************************************************************
Summary:
BTSC RF Encoding Settings

Description:
The parameters below are used for calibration and tuning of the BTSC
encoder.
***************************************************************************/
typedef struct NEXUS_BtscEncodeSettings
{
    bool useDeEmphasizedSourceSignals;  /* Default = true. */
    bool equivalentMode75us;            /* Default = false.  When enabled, L+R and L-R will go through preephasis and be rendered out. */
    bool fmDeviationControl;            /* Default = true.  Used to Control FM Deviation of Final Output to 50Khz */
    bool rightChannelMute;              /* Default = false.  Used to test stereo separation */

    NEXUS_BtscEncodingMode encodingMode;    /* Default = stereo.  */

    int32_t volume;                     /* Default = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL.  Ranges from NEXUS_AUDIO_VOLUME_LINEAR_NORMAL to NEXUS_AUDIO_VOLUME_LINEAR_MIN. */
    uint32_t srdCalibrationFactor;      /* Stereo Reference Decoder Calibration Factor.  Default = 0x12BE76C9.  Specified in 4.28 format */

    struct
    {
        bool preemphasis;               /* Default = true. */
        bool fmDeviationControl;        /* Default = true. */
    } mainChannel;    /* Settings for the main (L+R) channel */
    
    struct
    {
        bool clipping;                  /* Default = true. */
        bool preemphasis;               /* Default = true. */
        bool fmDeviationControl;        /* Default = true. */
    } diffChannel;    /* Settings for the difference (L-R) channel */
       
    struct
    {
        bool enabled;                   /* If true, a test tone is enabled */
        bool frequency;                 /* Test tone frequency in Hz.  If 0, a sweep will be output */
    } testTone;
    
} NEXUS_BtscEncodeSettings;

/***************************************************************************
Summary:
Encoding-Specific Settings for an RF Audio Encoder
***************************************************************************/
typedef struct NEXUS_RfAudioEncoderEncodingSettings
{
    NEXUS_RfAudioEncoding encoding;         /* this is used for the encodingSettings lookup */
    union
    {
        NEXUS_BtscEncodeSettings btsc;
    } encodingSettings;
} NEXUS_RfAudioEncoderEncodingSettings;

/***************************************************************************
Summary:
    Get Encoding-Specific Settings for an RF Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoding encoding,                 /* the encoding for which you are retrieving settings. */
    NEXUS_RfAudioEncoderEncodingSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Encoding-Specific Settings for an RF Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_SetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderEncodingSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an RF Audio Encoder

Description:
    This is used for a direct connection to a DAC, as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(dac), NEXUS_RfAudioEncoder_GetConnector(rfEncoder));

***************************************************************************/
NEXUS_AudioInputHandle NEXUS_RfAudioEncoder_GetConnector(
    NEXUS_RfAudioEncoderHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage

Description:
This is used to connect to the audio decoder as follows:

    NEXUS_RfAudioEncoder_AddInput(audioEncoder, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_AddInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_RemoveInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_RemoveAllInputs(
    NEXUS_RfAudioEncoderHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_RF_ENCODER_H__ */
