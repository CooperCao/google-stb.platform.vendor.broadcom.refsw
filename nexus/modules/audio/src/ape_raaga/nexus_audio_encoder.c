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
* API Description:
*   API name: DtsEncode
*    Specific APIs related to DTS Audio Encoding
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_encoder);

/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetDefaultSettings(
    NEXUS_AudioEncoderSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_EncoderSettings encoderSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_Encoder_GetDefaultSettings(&encoderSettings);
    pSettings->codec = NEXUS_Audio_P_MagnumToCodec(encoderSettings.codec);
    pSettings->loudnessEquivalenceEnabled = encoderSettings.loudnessEquivalenceEnabled;
}

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
NEXUS_AudioEncoderHandle NEXUS_AudioEncoder_Open( /* attr{destructor=NEXUS_AudioEncoder_Close}  */
    const NEXUS_AudioEncoderSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioEncoderHandle handle;
    BAPE_EncoderSettings encoderSettings;
    BAPE_Connector path;
    NEXUS_AudioEncoderSettings defaults;
    
    if ( NULL == pSettings )
    {
        NEXUS_AudioEncoder_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioEncoder));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    NEXUS_OBJECT_INIT(NEXUS_AudioEncoder, handle);
    
    BAPE_Encoder_GetDefaultSettings(&encoderSettings);
    encoderSettings.codec = NEXUS_Audio_P_CodecToMagnum(pSettings->codec);
    errCode = BAPE_Encoder_Create(NEXUS_AUDIO_DEVICE_HANDLE,
                                  &encoderSettings,
                                  &handle->encoder);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_create;
    }

    BKNI_Snprintf(handle->name, sizeof(handle->name), "ENCODER");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eEncoder, handle);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_eCompressed;
    BAPE_Encoder_GetConnector(handle->encoder, BAPE_ConnectorFormat_eCompressed, &path);
    handle->connector.port = (size_t)path;
    
    /* success */
    return handle;

err_create:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEncoder, handle);
    BKNI_Free(handle);
err_malloc:
    return NULL;
}

/***************************************************************************
Summary:
    Close an DTS Encode stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
static void NEXUS_AudioEncoder_P_Finalizer(
    NEXUS_AudioEncoderHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEncoder, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_Encoder_Destroy(handle->encoder);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEncoder, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEncoder, NEXUS_AudioEncoder_Close);

/***************************************************************************
Summary:
    Get Settings for an DTS Encode stage
***************************************************************************/
void NEXUS_AudioEncoder_GetSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioEncoderSettings *pSettings    /* [out] Settings */
    )
{
    BAPE_EncoderSettings settings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Encoder_GetSettings(handle->encoder, &settings);
    pSettings->codec = NEXUS_Audio_P_MagnumToCodec(settings.codec);
    pSettings->loudnessEquivalenceEnabled = settings.loudnessEquivalenceEnabled;
}


/***************************************************************************
Summary:
    Set Settings for an DTS Encode stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderSettings *pSettings
    )
{
    BAPE_EncoderSettings settings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_Encoder_GetSettings(handle->encoder, &settings);
    settings.codec = NEXUS_Audio_P_CodecToMagnum(pSettings->codec);
    settings.loudnessEquivalenceEnabled = pSettings->loudnessEquivalenceEnabled;
    errCode = BAPE_Encoder_SetSettings(handle->encoder, &settings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Get Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioCodec codec, /* the codec for which you are retrieving settings. */
    NEXUS_AudioEncoderCodecSettings *pSettings    /* [out] Settings */
    )
{
    BAVC_AudioCompressionStd magnumCodec;
    BAPE_EncoderCodecSettings codecSettings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != pSettings);
    pSettings->codec = codec;
    magnumCodec = NEXUS_Audio_P_CodecToMagnum(codec);
    BAPE_Encoder_GetCodecSettings(handle->encoder, magnumCodec, &codecSettings);
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        pSettings->codecSettings.ac3.spdifHeaderEnabled = codecSettings.codecSettings.ac3.spdifHeaderEnabled;
        break;
    case NEXUS_AudioCodec_eDts:
        pSettings->codecSettings.dts.spdifHeaderEnabled = codecSettings.codecSettings.dts.spdifHeaderEnabled;
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        pSettings->codecSettings.aac.bitRate = codecSettings.codecSettings.aac.bitRate;
        pSettings->codecSettings.aac.outputMode = (codecSettings.codecSettings.aac.channelMode == BAPE_ChannelMode_e1_0)?NEXUS_AudioMode_e1_0:(codecSettings.codecSettings.aac.channelMode == BAPE_ChannelMode_e1_1)?NEXUS_AudioMode_e1_1:NEXUS_AudioMode_e2_0;
        pSettings->codecSettings.aac.monoMode = (NEXUS_AudioMonoChannelMode)codecSettings.codecSettings.aac.monoMode;
        pSettings->codecSettings.aac.sampleRate = codecSettings.codecSettings.aac.sampleRate;
        pSettings->codecSettings.aac.complexity = (NEXUS_AacEncodeComplexity)codecSettings.codecSettings.aac.complexity;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        pSettings->codecSettings.aacPlus.bitRate = codecSettings.codecSettings.aacPlus.bitRate;
        pSettings->codecSettings.aacPlus.outputMode = (codecSettings.codecSettings.aacPlus.channelMode == BAPE_ChannelMode_e1_0)?NEXUS_AudioMode_e1_0:(codecSettings.codecSettings.aacPlus.channelMode == BAPE_ChannelMode_e1_1)?NEXUS_AudioMode_e1_1:NEXUS_AudioMode_e2_0;
        pSettings->codecSettings.aacPlus.monoMode = (NEXUS_AudioMonoChannelMode)codecSettings.codecSettings.aacPlus.monoMode;
        BDBG_CASSERT((int)NEXUS_AudioMonoChannelMode_eLeft==(int)BAPE_MonoChannelMode_eLeft);
        BDBG_CASSERT((int)NEXUS_AudioMonoChannelMode_eRight==(int)BAPE_MonoChannelMode_eRight);
        BDBG_CASSERT((int)NEXUS_AudioMonoChannelMode_eMix==(int)BAPE_MonoChannelMode_eMix);
        pSettings->codecSettings.aacPlus.sampleRate = codecSettings.codecSettings.aacPlus.sampleRate;
        BDBG_CASSERT((int)NEXUS_AacEncodeComplexity_eLowest == (int)BAPE_AacEncodeComplexity_eLowest);
        BDBG_CASSERT((int)NEXUS_AacEncodeComplexity_eMediumLow == (int)BAPE_AacEncodeComplexity_eMediumLow);
        BDBG_CASSERT((int)NEXUS_AacEncodeComplexity_eMediumHigh == (int)BAPE_AacEncodeComplexity_eMediumHigh);
        BDBG_CASSERT((int)NEXUS_AacEncodeComplexity_eHighest == (int)BAPE_AacEncodeComplexity_eHighest);
        BDBG_CASSERT((int)NEXUS_AacEncodeComplexity_eMax == (int)BAPE_AacEncodeComplexity_eMax);
        pSettings->codecSettings.aacPlus.complexity = (NEXUS_AacEncodeComplexity)codecSettings.codecSettings.aacPlus.complexity;
        break;
    case NEXUS_AudioCodec_eMp3:
        pSettings->codecSettings.mp3.bitRate = codecSettings.codecSettings.mp3.bitRate;
        pSettings->codecSettings.mp3.privateBit = codecSettings.codecSettings.mp3.privateBit;
        pSettings->codecSettings.mp3.copyrightBit = codecSettings.codecSettings.mp3.copyrightBit;
        pSettings->codecSettings.mp3.originalBit = codecSettings.codecSettings.mp3.originalBit;
        pSettings->codecSettings.mp3.emphasis = (NEXUS_AudioMpegEmphasis)codecSettings.codecSettings.mp3.emphasisMode;
        BDBG_CASSERT((int)NEXUS_AudioMpegEmphasis_eMax==(int)BAPE_MpegEmphasisMode_eMax);
        pSettings->codecSettings.mp3.outputMode = (codecSettings.codecSettings.mp3.channelMode == BAPE_ChannelMode_e1_0)?NEXUS_AudioMode_e1_0:(codecSettings.codecSettings.mp3.channelMode == BAPE_ChannelMode_e1_1)?NEXUS_AudioMode_e1_1:NEXUS_AudioMode_e2_0;
        pSettings->codecSettings.mp3.monoMode = (NEXUS_AudioMonoChannelMode)codecSettings.codecSettings.mp3.monoMode;
        break;
    case NEXUS_AudioCodec_eWmaStd:
        pSettings->codecSettings.wmaStd.bitRate = codecSettings.codecSettings.wmaStd.bitRate;
        pSettings->codecSettings.wmaStd.monoEncoding = (codecSettings.codecSettings.wmaStd.channelMode == BAPE_ChannelMode_e1_0)?true:false;
        break;
    case NEXUS_AudioCodec_eG711:
        pSettings->codecSettings.g711.compressionMode = (NEXUS_G711G726CompressionMode)codecSettings.codecSettings.g711.compressionMode;
        BDBG_CASSERT(NEXUS_G711G726CompressionMode_eUlaw == (int)BAPE_G711G726CompressionMode_eUlaw);
        BDBG_CASSERT(NEXUS_G711G726CompressionMode_eAlaw == (int)BAPE_G711G726CompressionMode_eAlaw);
        BDBG_CASSERT(NEXUS_G711G726CompressionMode_eMax == (int)BAPE_G711G726CompressionMode_eMax);
        break;
    case NEXUS_AudioCodec_eG726:
        pSettings->codecSettings.g726.compressionMode = (NEXUS_G711G726CompressionMode)codecSettings.codecSettings.g726.compressionMode;
        pSettings->codecSettings.g726.bitRate = codecSettings.codecSettings.g726.bitRate;
        break;
    case NEXUS_AudioCodec_eG729:
        pSettings->codecSettings.g729.dtxEnabled = codecSettings.codecSettings.g729.dtxEnabled;
        pSettings->codecSettings.g729.bitRate = codecSettings.codecSettings.g729.bitRate;
        break;
    case NEXUS_AudioCodec_eG723_1:
        pSettings->codecSettings.g723_1.vadEnabled = codecSettings.codecSettings.g723_1.vadEnabled;
        pSettings->codecSettings.g723_1.hpfEnabled = codecSettings.codecSettings.g723_1.hpfEnabled;
        pSettings->codecSettings.g723_1.bitRate = codecSettings.codecSettings.g723_1.bitRate;
        break;
    case NEXUS_AudioCodec_eIlbc:
        pSettings->codecSettings.ilbc.frameLength = codecSettings.codecSettings.ilbc.frameLength;
        break;
    case NEXUS_AudioCodec_eOpus:
        pSettings->codecSettings.opus.bitRate = codecSettings.codecSettings.opus.bitRate;
        pSettings->codecSettings.opus.frameSize = codecSettings.codecSettings.opus.frameSize;
        BDBG_CASSERT((int)NEXUS_OpusEncodeMode_eSilk == (int)BAPE_OpusEncodeMode_eSilk);
        BDBG_CASSERT((int)NEXUS_OpusEncodeMode_eHybrid == (int)BAPE_OpusEncodeMode_eHybrid);
        BDBG_CASSERT((int)NEXUS_OpusEncodeMode_eCELT == (int)BAPE_OpusEncodeMode_eCELT);
        BDBG_CASSERT((int)NEXUS_OpusEncodeMode_eMax == (int)BAPE_OpusEncodeMode_eMax);
        pSettings->codecSettings.opus.encodeMode = codecSettings.codecSettings.opus.encodeMode;
        BDBG_CASSERT((int)NEXUS_OpusBitRateType_eCBR == (int)BAPE_OpusBitRateType_eCBR);
        BDBG_CASSERT((int)NEXUS_OpusBitRateType_eVBR == (int)BAPE_OpusBitRateType_eVBR);
        BDBG_CASSERT((int)NEXUS_OpusBitRateType_eCVBR == (int)BAPE_OpusBitRateType_eCVBR);
        BDBG_CASSERT((int)NEXUS_OpusBitRateType_eMax == (int)BAPE_OpusBitRateType_eMax);
        pSettings->codecSettings.opus.bitRateType = codecSettings.codecSettings.opus.bitRateType;
        pSettings->codecSettings.opus.complexity = codecSettings.codecSettings.opus.complexity;
        break;
    default:
        break;
    }
}


/***************************************************************************
Summary:
    Set Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetCodecSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderCodecSettings *pSettings
    )
{
    BAVC_AudioCompressionStd magnumCodec;
    BAPE_EncoderCodecSettings codecSettings;
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != pSettings);
    magnumCodec = NEXUS_Audio_P_CodecToMagnum(pSettings->codec);
    BAPE_Encoder_GetCodecSettings(handle->encoder, magnumCodec, &codecSettings);
    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        codecSettings.codecSettings.ac3.spdifHeaderEnabled = pSettings->codecSettings.ac3.spdifHeaderEnabled;
        break;
    case NEXUS_AudioCodec_eDts:
        codecSettings.codecSettings.dts.spdifHeaderEnabled = pSettings->codecSettings.dts.spdifHeaderEnabled;
        break;
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        codecSettings.codecSettings.aac.bitRate = pSettings->codecSettings.aac.bitRate;
        codecSettings.codecSettings.aac.channelMode = (pSettings->codecSettings.aac.outputMode == NEXUS_AudioMode_e1_0)?BAPE_ChannelMode_e1_0:(pSettings->codecSettings.aac.outputMode == NEXUS_AudioMode_e1_1)?BAPE_ChannelMode_e1_1:BAPE_ChannelMode_e2_0;
        codecSettings.codecSettings.aac.monoMode = (BAPE_MonoChannelMode)pSettings->codecSettings.aac.monoMode;
        codecSettings.codecSettings.aac.sampleRate = pSettings->codecSettings.aac.sampleRate;
        codecSettings.codecSettings.aac.complexity = (BAPE_AacEncodeComplexity)pSettings->codecSettings.aac.complexity;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        codecSettings.codecSettings.aacPlus.bitRate = pSettings->codecSettings.aacPlus.bitRate;
        codecSettings.codecSettings.aacPlus.channelMode = (pSettings->codecSettings.aacPlus.outputMode == NEXUS_AudioMode_e1_0)?BAPE_ChannelMode_e1_0:(pSettings->codecSettings.aacPlus.outputMode == NEXUS_AudioMode_e1_1)?BAPE_ChannelMode_e1_1:BAPE_ChannelMode_e2_0;
        codecSettings.codecSettings.aacPlus.monoMode = (BAPE_MonoChannelMode)pSettings->codecSettings.aacPlus.monoMode;
        codecSettings.codecSettings.aacPlus.sampleRate = pSettings->codecSettings.aacPlus.sampleRate;
        codecSettings.codecSettings.aacPlus.complexity = (BAPE_AacEncodeComplexity)pSettings->codecSettings.aacPlus.complexity;
        break;
    case NEXUS_AudioCodec_eMp3:
        codecSettings.codecSettings.mp3.bitRate = pSettings->codecSettings.mp3.bitRate;
        codecSettings.codecSettings.mp3.privateBit = pSettings->codecSettings.mp3.privateBit;
        codecSettings.codecSettings.mp3.copyrightBit = pSettings->codecSettings.mp3.copyrightBit;
        codecSettings.codecSettings.mp3.originalBit = pSettings->codecSettings.mp3.originalBit;
        codecSettings.codecSettings.mp3.emphasisMode = (BAPE_MpegEmphasisMode)pSettings->codecSettings.mp3.emphasis;
        codecSettings.codecSettings.mp3.channelMode = (pSettings->codecSettings.mp3.outputMode == NEXUS_AudioMode_e1_0)?BAPE_ChannelMode_e1_0:(pSettings->codecSettings.mp3.outputMode == NEXUS_AudioMode_e1_1)?BAPE_ChannelMode_e1_1:BAPE_ChannelMode_e2_0;
        codecSettings.codecSettings.mp3.monoMode = (BAPE_MonoChannelMode)pSettings->codecSettings.mp3.monoMode;
        break;
    case NEXUS_AudioCodec_eWmaStd:
        codecSettings.codecSettings.wmaStd.bitRate = pSettings->codecSettings.wmaStd.bitRate;
        codecSettings.codecSettings.wmaStd.channelMode = pSettings->codecSettings.wmaStd.monoEncoding ? BAPE_ChannelMode_e1_0 : BAPE_ChannelMode_e2_0;
        break;
    case NEXUS_AudioCodec_eG711:
        codecSettings.codecSettings.g711.compressionMode = (BAPE_G711G726CompressionMode)pSettings->codecSettings.g711.compressionMode;
        break;
    case NEXUS_AudioCodec_eG726:
        codecSettings.codecSettings.g726.compressionMode = (BAPE_G711G726CompressionMode)pSettings->codecSettings.g726.compressionMode;
        codecSettings.codecSettings.g726.bitRate = pSettings->codecSettings.g726.bitRate;
        break;
    case NEXUS_AudioCodec_eG729:
        codecSettings.codecSettings.g729.dtxEnabled = pSettings->codecSettings.g729.dtxEnabled;
        codecSettings.codecSettings.g729.bitRate = pSettings->codecSettings.g729.bitRate;
        break;
    case NEXUS_AudioCodec_eG723_1:
        codecSettings.codecSettings.g723_1.vadEnabled = pSettings->codecSettings.g723_1.vadEnabled;
        codecSettings.codecSettings.g723_1.hpfEnabled = pSettings->codecSettings.g723_1.hpfEnabled;
        codecSettings.codecSettings.g723_1.bitRate = pSettings->codecSettings.g723_1.bitRate;
        break;
    case NEXUS_AudioCodec_eIlbc:
        codecSettings.codecSettings.ilbc.frameLength = pSettings->codecSettings.ilbc.frameLength;
        break;
    case NEXUS_AudioCodec_eOpus:
        codecSettings.codecSettings.opus.bitRate = pSettings->codecSettings.opus.bitRate;
        codecSettings.codecSettings.opus.frameSize = pSettings->codecSettings.opus.frameSize;
        codecSettings.codecSettings.opus.encodeMode = pSettings->codecSettings.opus.encodeMode;
        codecSettings.codecSettings.opus.bitRateType = pSettings->codecSettings.opus.bitRateType;
        codecSettings.codecSettings.opus.complexity = pSettings->codecSettings.opus.complexity;
        break;
    default:
        return BERR_SUCCESS;
    }
    errCode = BAPE_Encoder_SetCodecSettings(handle->encoder, &codecSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for an Audio Encoder stage

Description:
This is used for a direct connection to SPDIF, as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(spdif), NEXUS_AudioEncoder_GetConnector(audioEncoder));

***************************************************************************/
NEXUS_AudioInput NEXUS_AudioEncoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_AudioEncoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    return &handle->connector;
}

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_AddInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != input);

    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_Encoder_AddInput(handle->encoder, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_Encoder_RemoveInput(handle->encoder, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }

    handle->input = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != input);
    if ( input != handle->input )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    (void)BAPE_Encoder_RemoveInput(handle->encoder, (BAPE_Connector)input->port);
    handle->input = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_RemoveAllInputs(
    NEXUS_AudioEncoderHandle handle
    )
{
    if ( handle->input ) 
    {
        return NEXUS_AudioEncoder_RemoveInput(handle, handle->input);
    }
    return NEXUS_SUCCESS;
}

