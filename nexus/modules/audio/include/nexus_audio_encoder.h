/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_AUDIO_ENCODER_H__
#define NEXUS_AUDIO_ENCODER_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_decoder_types.h"
#include "nexus_ac3_encode.h"
#include "nexus_dts_encode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Internal development notes:

Previously, nexus supported separate interfaces per encodable codec type: NEXUS_Ac3Encode, NEXUS_DtsEncode.
Now we will support a generic API w/ a codec enum.

There is no Start/Stop. Instead, the whole pipeline must be stopped at the source (e.g. NEXUS_AudioDecoder_Stop), then
the app can make changes to the filter graph including AudioEncoder, then the whole pipeline is started at the source (e.g. NEXUS_AudioDecoder_Start).
**/

/**
Summary:
Handle for Audio Encoder stage
**/
typedef struct NEXUS_AudioEncoder *NEXUS_AudioEncoderHandle;

/***************************************************************************
Summary:
Audio Encoder Settings
    
Description:
Delay mode is set in AudioDecoder
***************************************************************************/
typedef struct NEXUS_AudioEncoderSettings
{
    NEXUS_AudioCodec codec;     /* The codec into which audio is currently being encoded.
                                   Can only be changed while the data flow is stopped. */

    bool loudnessEquivalenceEnabled;    /* This value has been depracated.  Loudness equivalence is enabled via NEXUS_AudioModuleSettings.loudnessMode. */
} NEXUS_AudioEncoderSettings;

/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetDefaultSettings(
    NEXUS_AudioEncoderSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
NEXUS_AudioEncoderHandle NEXUS_AudioEncoder_Open( /* attr{destructor=NEXUS_AudioEncoder_Close}  */
    const NEXUS_AudioEncoderSettings *pSettings
    );

/***************************************************************************
Summary:
    Close an Audio Encoder stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_AudioEncoder_Close(
    NEXUS_AudioEncoderHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioEncoderSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderSettings *pSettings
    );

/***************************************************************************
Summary:
AAC Encode Output Mode
***************************************************************************/
#define NEXUS_AacEncodeOutputMode       NEXUS_AudioMode
#define NEXUS_AacEncodeOutputMode_e2_0  NEXUS_AudioMode_e2_0    /* Stereo */
#define NEXUS_AacEncodeOutputMode_e1_0  NEXUS_AudioMode_e1_0    /* Mono */
#define NEXUS_AacEncodeOutputMode_e1_1  NEXUS_AudioMode_e1_1    /* Dual Mono */

/***************************************************************************
Summary:
AAC Encode Complexity 
 
Description: 
AAC Encode algorithm complexity.  Default is the lowest complexity, 
which uses the fewest DSP cycles.  To achieve a higher quality encode at a 
given bitrate, this can be set to higher values at the cost of increased 
DSP usage.
***************************************************************************/
typedef enum NEXUS_AacEncodeComplexity
{
    NEXUS_AacEncodeComplexity_eLowest,       /* Lowest complexity. */
    NEXUS_AacEncodeComplexity_eMediumLow,
    NEXUS_AacEncodeComplexity_eMediumHigh,
    NEXUS_AacEncodeComplexity_eHighest,      /* Highest complexity. */
    NEXUS_AacEncodeComplexity_eMax   
} NEXUS_AacEncodeComplexity;

/***************************************************************************
Summary:
AAC Encode Parameters
***************************************************************************/
typedef struct NEXUS_AacEncodeSettings
{
    NEXUS_AacEncodeOutputMode       outputMode; /* Output channel mode */
    NEXUS_AudioMonoChannelMode      monoMode;   /* Set the desired mono channel output mode.  This field is
                                                   only honored if outputMode == NEXUS_AudioMode_e1_0 */
    unsigned                        bitRate;    /* Bitrate in bps */
    unsigned                        sampleRate; /* Output sample rate.  If 0 (default), the stream sample rate will be 
                                                   preserved.  Otherwise, if set to 32000 48 kHz streams will be
                                                   converted to 32 kHz while other sample rates will be preserved. */
    NEXUS_AacEncodeComplexity       complexity; /* Encode algorithm complexity.  0 (default) is the lowest complexity,
                                                   this will use the fewest DSP cycles.  To achieve a higher quality
                                                   encode at a given bitrate, this can be set to higher values up to
                                                   a maximum of 3, but higher values will increase the DSP usage. */
} NEXUS_AacEncodeSettings;

/***************************************************************************
Summary:
MPEG audio encode parameters
***************************************************************************/
typedef struct NEXUS_AudioMpegEncodeSettings
{
    unsigned bitRate;                               /* Output bitrate of the encoder in bps. Ranges from 32000 to 320000. */

    bool privateBit;                                /* If true, the private bit will be asserted in the header */
    bool copyrightBit;                              /* If true, the copyright bit will be asserted in the header */
    bool originalBit;                               /* If true, the original bit will be asserted in the header */
    NEXUS_AudioMpegEmphasis         emphasis;       /* De-Emphasis mode */
    NEXUS_AudioMode                 outputMode;     /* Set the desired channel output mode */
    NEXUS_AudioMonoChannelMode      monoMode;       /* Set the desired mono channel output mode.  This field is
                                                       only honored if outputMode == NEXUS_AudioMode_e1_0 */
}NEXUS_AudioMpegEncodeSettings;

/***************************************************************************
Summary:
WMA Standard audio encode parameters
***************************************************************************/
typedef struct NEXUS_AudioWmaStdEncodeSettings
{
    unsigned bitRate;                   /* Output bitrate of the encoder in bps. Default is 192000. Lower rates can
                                           cause very high DSP usage. */
    bool monoEncoding;                  /* If true, encode as mono.  If false (default), encode as stereo.  */
}NEXUS_AudioWmaStdEncodeSettings;

/***************************************************************************
Summary:
G.711/G.726 Compression Modes
***************************************************************************/
typedef enum NEXUS_G711G726CompressionMode
{
    NEXUS_G711G726CompressionMode_eUlaw,     /* u-Law Compression.  Typically used in North America and Japan. */
    NEXUS_G711G726CompressionMode_eAlaw,     /* a-Law Compression.  Typically used in Europe and elsewhere. */
    NEXUS_G711G726CompressionMode_eMax
} NEXUS_G711G726CompressionMode;

/***************************************************************************
Summary:
G.711 Encode Settings
***************************************************************************/
typedef struct NEXUS_G711EncodeSettings
{
    NEXUS_G711G726CompressionMode compressionMode;   /* Select compression mode. */
} NEXUS_G711EncodeSettings;

/***************************************************************************
Summary:
G.726 Encode Settings
***************************************************************************/
typedef struct NEXUS_G726EncodeSettings
{
    NEXUS_G711G726CompressionMode compressionMode;   /* Select compression mode. */

    unsigned bitRate;                               /* Output bitrate of the encoder in bps.  Valid values are 16000, 24000, 32000, and 40000 for G.726 */
} NEXUS_G726EncodeSettings;

/***************************************************************************
Summary:
G.729 Encode Settings
***************************************************************************/
typedef struct NEXUS_G729EncodeSettings
{
    bool dtxEnabled;    /* If true, DTX (Discontinuous Transmission) is enabled to conserve bandwidth during silence. */

    unsigned bitRate;   /* Output bitrate of the encoder in bps.  Valid values are 6400, 8000, and 11800 for G.729 */
} NEXUS_G729EncodeSettings;

/***************************************************************************
Summary:
G.723.1 Encode Settings
***************************************************************************/
typedef struct NEXUS_G723_1EncodeSettings
{
    bool vadEnabled;    /* If true, VAD (Voice Activity Detection) is enabled to conserve bandwidth during silence. */
    bool hpfEnabled;    /* If true, a high-pass filter will be enabled. */

    unsigned bitRate;   /* Output bitrate of the encoder in bps.  Valid values are 6300 and 5300 for G.723.1 */
} NEXUS_G723_1EncodeSettings;

/***************************************************************************
Summary:
iLbc Encode Settings
***************************************************************************/
typedef struct NEXUS_IlbcEncodeSettings
{
    unsigned frameLength;   /* in milliseconds.  valid values are 20, 30 */
} NEXUS_IlbcEncodeSettings;

/***************************************************************************
Summary:
Opus Encode Modes
***************************************************************************/
typedef enum NEXUS_OpusEncodeMode
{
    NEXUS_OpusEncodeMode_eSilk,             /* Silk Compression Format.*/
    NEXUS_OpusEncodeMode_eHybrid,           /* Hybrid of the two compression types (Silk and CELT) */
    NEXUS_OpusEncodeMode_eCELT,             /* Constrained Energy Lapped Transform Compression Format */
    NEXUS_OpusEncodeMode_eMax
} NEXUS_OpusEncodeMode;


/***************************************************************************
Summary:
Opus Bit Rate Types
***************************************************************************/
typedef enum NEXUS_OpusBitRateType
{
    NEXUS_OpusBitRateType_eCBR,             /* Constant Bit Rate */
    NEXUS_OpusBitRateType_eVBR,             /* Variable Bit Rate */
    NEXUS_OpusBitRateType_eCVBR,            /* Constrained Variable Bit Rate */
    NEXUS_OpusBitRateType_eMax
} NEXUS_OpusBitRateType;

/***************************************************************************
Summary:
Opus Encode Settings
***************************************************************************/
typedef struct NEXUS_OpusEncodeSettings
{

    unsigned                bitRate;            /* Bit rate of the encoded file Supports 6kbps to 510kbps Default is 48kbps. */
    unsigned                frameSize;          /* Size of the frame in samples. Supports 120,240,480,960,1920,2880. Default is 960. */
    NEXUS_OpusEncodeMode    encodeMode;         /* Mode of the encoder (Silk Only, Hybrid, CELT Only). Default is Hybrid. */
    NEXUS_OpusBitRateType   bitRateType;        /* Type of bit rate to encode with (Constant Bit Rate, Variable Bit Rate, Constrained Variable Bit Rate). Default: 1 VBR Supports 0-CBR,1-VBR,2-CVBR. Default is VBR. */
    unsigned                complexity;         /* Computational Complexity for the encoder. Supports 1 - 10. Default is 10. */
} NEXUS_OpusEncodeSettings;

/***************************************************************************
Summary:
Codec-Specific Settings for an audio encoder
***************************************************************************/
typedef struct NEXUS_AudioEncoderCodecSettings
{
    NEXUS_AudioCodec codec; /* this is used for the codecSettings lookup */
    union
    {
        NEXUS_Ac3EncodeSettings ac3;
        NEXUS_DtsEncodeSettings dts;
        NEXUS_AacEncodeSettings aac;
        NEXUS_AacEncodeSettings aacPlus;
        NEXUS_AudioMpegEncodeSettings mp3;
        NEXUS_AudioWmaStdEncodeSettings wmaStd;
        NEXUS_G711EncodeSettings g711;
        NEXUS_G726EncodeSettings g726;
        NEXUS_G729EncodeSettings g729;
        NEXUS_G723_1EncodeSettings g723_1;
        NEXUS_IlbcEncodeSettings ilbc;
        NEXUS_OpusEncodeSettings opus;
    } codecSettings;
} NEXUS_AudioEncoderCodecSettings;

/***************************************************************************
Summary:
    Get Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioCodec codec, /* the codec for which you are retrieving settings. */
    NEXUS_AudioEncoderCodecSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderCodecSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an Audio Encoder stage

Description:
This is used for a direct connection to SPDIF, as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(spdif), NEXUS_AudioEncoder_GetConnector(audioEncoder));

***************************************************************************/
NEXUS_AudioInput NEXUS_AudioEncoder_GetConnector( 
    NEXUS_AudioEncoderHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage

Description:
This is used to connect to the audio decoder as follows:

    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_AudioDecoder_GetConnector(audioDecoder));

***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_AddInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveAllInputs(
    NEXUS_AudioEncoderHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_ENCODER_H__ */

