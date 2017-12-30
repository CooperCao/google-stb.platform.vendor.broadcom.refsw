/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
***************************************************************************/
#ifndef BAPE_DOLBY_DIGITAL_REENCODE_H__
#define BAPE_DOLBY_DIGITAL_REENCODE_H__

#include "bape.h"
#include "bape_encoder.h"
#include "bape_codec_types.h"

/***************************************************************************
Summary:
Dolby Digital Reencode Handle
***************************************************************************/
typedef struct BAPE_DolbyDigitalReencode *BAPE_DolbyDigitalReencodeHandle;

/***************************************************************************
Summary:
Dolby Digital Reencode Profile
***************************************************************************/
typedef enum BAPE_DolbyDigitalReencodeProfile
{
    BAPE_DolbyDigitalReencodeProfile_eNoCompression,
    BAPE_DolbyDigitalReencodeProfile_eFilmStandardCompression,
    BAPE_DolbyDigitalReencodeProfile_eFilmLightCompression,
    BAPE_DolbyDigitalReencodeProfile_eMusicStandardCompression,
    BAPE_DolbyDigitalReencodeProfile_eMusicLightCompression,
    BAPE_DolbyDigitalReencodeProfile_eSpeechCompression,
    BAPE_DolbyDigitalReencodeProfile_eMax
} BAPE_DolbyDigitalReencodeProfile;

/***************************************************************************
Summary:
Dolby Digital Reencode DRC modes
***************************************************************************/
typedef enum BAPE_DolbyDigitalReencodeDrcMode
{
    BAPE_DolbyDigitalReencodeDrcMode_eLine,
    BAPE_DolbyDigitalReencodeDrcMode_eRf,
    BAPE_DolbyDigitalReencodeDrcMode_eMax
} BAPE_DolbyDigitalReencodeDrcMode;

/***************************************************************************
Summary:
Dolby Digital Reencode Stereo Downmix modes
***************************************************************************/
typedef enum BAPE_DolbyDigitalReencodeStereoMode
{
    BAPE_DolbyDigitalReencodeStereoMode_eLtRt,
    BAPE_DolbyDigitalReencodeStereoMode_eLoRo,
    BAPE_DolbyDigitalReencodeStereoMode_eArib,
    BAPE_DolbyDigitalReencodeStereoMode_eMax
} BAPE_DolbyDigitalReencodeStereoMode;

/***************************************************************************
Summary:
Dolby Digital Reencode Settings
***************************************************************************/
typedef struct BAPE_DolbyDigitalReencodeSettings
{
    BAPE_Ac3EncodeSettings encodeSettings;  /* Settings for the AC3/Dolby Digital encoder */

    bool loudnessEquivalenceEnabled;        /* This value has been depracated.  Loudness equivalence is enabled via BAPE_Settings.loudnessMode. */

    bool externalPcmMode;                   /* If true, operate with input from non-MS decoders.
                                               Should be false if any input is from a dolby MS decoder (AC3/AC3+/AAC). */
    BAPE_MultichannelFormat multichannelFormat; /* Controls whether the DDRE functions with 5.1 or 7.1 pcm input/output on the multichannel path.
                                                   This is not changeable on the fly. */
    BAPE_ChannelMode stereoOutputMode;      /* Configure the channel mode for the stereo port. e2_0, e1_0 are valid values. e2_0 is the default. */
    bool fixedEncoderFormat; /* When true, content will be upmixed or downmixed to the
                                multichannelFormat specified. This control only applies when MS12 is enabled. */


    /* The following settings are only applied if externalPcmMode = true */
    BAPE_DolbyDigitalReencodeProfile profile;           /* Compression Profile */
    BAPE_Ac3CenterMixLevel          centerMixLevel;     /* cmixlev - Center Mix Level */
    BAPE_Ac3SurroundMixLevel        surroundMixLevel;   /* surmixlev - Surround Mix Level */
    BAPE_Ac3DolbySurround           dolbySurround;      /* dsurmod - AC3 Dolby Surround Mode */
    unsigned                        dialogLevel;        /* Dialog level of incoming PCM in dB.  Ranges from 0..31. */

    BAPE_DolbyDigitalReencodeDrcMode drcMode;           /* DRC (Dynamic Range Compression) Mode */
    BAPE_DolbyDigitalReencodeDrcMode drcModeDownmix;    /* DRC (Dynamic Range Compression) Mode for stereo downmixed data*/
    uint16_t                         drcScaleHi;        /* In %, ranges from 0..100 */
    uint16_t                         drcScaleLow;       /* In %, ranges from 0..100 */
    uint16_t                         drcScaleHiDownmix; /* In %, ranges from 0..100 */
    uint16_t                         drcScaleLowDownmix;/* In %, ranges from 0..100 */

    BAPE_DolbyDigitalReencodeStereoMode stereoMode;     /* Stereo Downmix Mode */

    BAPE_DualMonoMode                   dualMonoMode;   /* Dual Mono Processing Mode */

    /* MDA configuration */
    bool encoderMixerRequired;
    bool encoderTaskRequired;
    unsigned encoderDeviceIndex;                        /* Only used if encoderTaskRequired = true. */
} BAPE_DolbyDigitalReencodeSettings;

/***************************************************************************
Summary:
    Get default settings for a Dolby Digital Reencoder
***************************************************************************/
void BAPE_DolbyDigitalReencode_GetDefaultSettings(
    BAPE_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a Dolby Digital Reencoder
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings,
    BAPE_DolbyDigitalReencodeHandle *pHandle    /* [out] */
    );

/***************************************************************************
Summary:
    Close a Dolby Digital Reencoder
***************************************************************************/
void BAPE_DolbyDigitalReencode_Destroy(
    BAPE_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a Dolby Digital Reencoder
***************************************************************************/
void BAPE_DolbyDigitalReencode_GetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Set Settings for a Dolby Digital Reencoder
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_SetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a Dolby Digital Reencoder
***************************************************************************/
void BAPE_DolbyDigitalReencode_GetConnector(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_AddInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_RemoveInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_RemoveAllInputs(
    BAPE_DolbyDigitalReencodeHandle handle
    );

#endif /* #ifndef BAPE_DOLBY_DIGITAL_REENCODE_H__ */
