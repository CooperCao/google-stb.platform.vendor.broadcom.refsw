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

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_encoder);

typedef struct NEXUS_AudioEncoder
{
    NEXUS_OBJECT(NEXUS_AudioEncoder);
    NEXUS_AudioInputObject connector;
    BRAP_ProcessingStageHandle stage;
    NEXUS_Ac3EncodeSettings ac3Settings;
    NEXUS_DtsEncodeSettings dtsSettings;
    NEXUS_AacEncodeSettings aacSettings, aacPlusSettings;
    NEXUS_AudioEncoderSettings settings;
    NEXUS_AudioInput input;
    NEXUS_AudioCodec codec;
}NEXUS_AudioEncoder;

static void NEXUS_AudioEncoder_P_GetDefaultAacSettings(NEXUS_AacEncodeSettings *pSettings)
{
#if BRAP_VER >= 4
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eEncodeAacHe, &g_NEXUS_StageSettings);
    pSettings->outputMode = g_NEXUS_StageSettings.uConfigParams.sAacHeENCParams.bEncodeMono?NEXUS_AacEncodeOutputMode_e1_0:NEXUS_AacEncodeOutputMode_e2_0;
    pSettings->bitRate = g_NEXUS_StageSettings.uConfigParams.sAacHeENCParams.ui32BitRate;
#else
    BSTD_UNUSED(pSettings);
#endif    
}


static void NEXUS_AudioEncoder_P_ApplyCodecSettings(NEXUS_AudioEncoderHandle handle, NEXUS_AudioCodec codec, BRAP_ProcessingStageSettings *pSettings)
{
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        pSettings->uConfigParams.sAc3ENCParams.bSpdifHeaderEnable = handle->ac3Settings.spdifHeaderEnabled;
        break;
    case NEXUS_AudioCodec_eDts:
        pSettings->uConfigParams.sDTSENCParams.bSpdifHeaderEnable = handle->dtsSettings.spdifHeaderEnabled;
        break;
#if BRAP_VER >= 4
    case NEXUS_AudioCodec_eAacAdts:
    case NEXUS_AudioCodec_eAacLoas:
        pSettings->uConfigParams.sAacHeENCParams.eEncodeFormatType = (codec == NEXUS_AudioCodec_eAacAdts)?BRAP_DSPCHN_AacheEncFormat_eAdts:BRAP_DSPCHN_AacheEncFormat_eLoas;
        pSettings->uConfigParams.sAacHeENCParams.bEnableSBR = false;
        pSettings->uConfigParams.sAacHeENCParams.bEncodeMono = handle->aacSettings.outputMode == NEXUS_AacEncodeOutputMode_e1_0?true:false;
        pSettings->uConfigParams.sAacHeENCParams.ui32BitRate = handle->aacSettings.bitRate;
        break;
    case NEXUS_AudioCodec_eAacPlusAdts:
    case NEXUS_AudioCodec_eAacPlusLoas:
        pSettings->uConfigParams.sAacHeENCParams.eEncodeFormatType = (codec == NEXUS_AudioCodec_eAacPlusAdts)?BRAP_DSPCHN_AacheEncFormat_eAdts:BRAP_DSPCHN_AacheEncFormat_eLoas;
        pSettings->uConfigParams.sAacHeENCParams.bEnableSBR = true;
        pSettings->uConfigParams.sAacHeENCParams.bEncodeMono = handle->aacPlusSettings.outputMode == NEXUS_AacEncodeOutputMode_e1_0?true:false;
        pSettings->uConfigParams.sAacHeENCParams.ui32BitRate = handle->aacPlusSettings.bitRate;
        break;
#endif
    default:
        break;
    }
}

/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetDefaultSettings(
    NEXUS_AudioEncoderSettings *pSettings   /* [out] default settings */
    )
{
    if ( NULL == pSettings )
    {
        return;
    }
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = NEXUS_AudioCodec_eAc3;
}

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
NEXUS_AudioEncoderHandle NEXUS_AudioEncoder_Open( /* attr{destructor=NEXUS_AudioEncoder_Close}  */
    const NEXUS_AudioEncoderSettings *pSettings
    )
{
    NEXUS_AudioEncoderSettings defaults;
    NEXUS_AudioEncoderHandle handle;
    BRAP_ProcessingType processingType;
    BERR_Code errCode;

    if ( NULL == pSettings )
    {
        NEXUS_AudioEncoder_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    processingType = NEXUS_Audio_P_CodecToProcessingType(pSettings->codec);
    if ( processingType == BRAP_ProcessingType_eNone )
    {
        BDBG_ERR(("Codec %u is not supported for encode", pSettings->codec));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_codec;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioEncoder));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_AudioEncoder, handle);
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eEncoder, handle);
    handle->connector.format = NEXUS_AudioInputFormat_eCompressed;
    handle->codec = pSettings->codec;
    NEXUS_Ac3Encode_GetDefaultSettings(&handle->ac3Settings);
    NEXUS_DtsEncode_GetDefaultSettings(&handle->dtsSettings);
    NEXUS_AudioEncoder_P_GetDefaultAacSettings(&handle->aacSettings);
    NEXUS_AudioEncoder_P_GetDefaultAacSettings(&handle->aacPlusSettings);

    BRAP_GetDefaultProcessingStageSettings(processingType, &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &handle->stage);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_stage;
    }
    
    handle->settings = *pSettings;
    
    return handle;

err_stage:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEncoder, handle);
    BKNI_Free(handle);
err_malloc:
err_codec:
    return NULL;
}


/***************************************************************************
Summary:
    Close an Audio Encoder stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
static void NEXUS_AudioEncoder_P_Finalizer(
    NEXUS_AudioEncoderHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioEncoder, handle);
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BRAP_DestroyProcessingStage(handle->stage);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioEncoder, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioEncoder, NEXUS_AudioEncoder_Close);

/***************************************************************************
Summary:
    Get Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_AudioEncoder_GetSettings(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioEncoderSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    if ( NULL == pSettings )
    {
        return;
    }
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(NEXUS_AudioEncoderSettings));
}


/***************************************************************************
Summary:
    Set Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_SetSettings(
    NEXUS_AudioEncoderHandle handle,
    const NEXUS_AudioEncoderSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    if ( NULL == pSettings )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(NEXUS_AudioEncoderSettings));

    return BERR_SUCCESS;
}


void NEXUS_AudioEncoder_GetDefaultCodecSettings(
    NEXUS_AudioCodec codec, /* the codec for which you are retrieving settings. */
    NEXUS_AudioEncoderCodecSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(codec);
    BSTD_UNUSED(pSettings);
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
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);

    if ( NULL == pSettings )
    {
        return;
    }

    pSettings->codec = codec;
    switch ( codec )
    {
    case NEXUS_AudioCodec_eAc3:
        BKNI_Memcpy(&pSettings->codecSettings.ac3, &handle->ac3Settings, sizeof(handle->ac3Settings));
        break;
    case NEXUS_AudioCodec_eDts:
        BKNI_Memcpy(&pSettings->codecSettings.dts, &handle->dtsSettings, sizeof(handle->dtsSettings));
        break;
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacAdts:
        BKNI_Memcpy(&pSettings->codecSettings.aac, &handle->aacSettings, sizeof(handle->aacSettings));
        break;
    case NEXUS_AudioCodec_eAacPlusLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
        BKNI_Memcpy(&pSettings->codecSettings.aacPlus, &handle->aacPlusSettings, sizeof(handle->aacPlusSettings));
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
    BERR_Code errCode;
    bool hasSettings = true;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);

    if ( NULL == pSettings )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->codec )
    {
    case NEXUS_AudioCodec_eAc3:
        BKNI_Memcpy(&handle->ac3Settings, &pSettings->codecSettings.ac3, sizeof(handle->ac3Settings));
        break;
    case NEXUS_AudioCodec_eDts:
        BKNI_Memcpy(&handle->dtsSettings, &pSettings->codecSettings.dts, sizeof(handle->dtsSettings));
        break;
    case NEXUS_AudioCodec_eAacLoas:
    case NEXUS_AudioCodec_eAacAdts:
        BKNI_Memcpy(&handle->aacSettings, &pSettings->codecSettings.aac, sizeof(handle->aacSettings));
        break;
    case NEXUS_AudioCodec_eAacPlusLoas:
    case NEXUS_AudioCodec_eAacPlusAdts:
        BKNI_Memcpy(&handle->aacPlusSettings, &pSettings->codecSettings.aacPlus, sizeof(handle->aacPlusSettings));
        break;
    default:
        hasSettings = false;
        break;
    }
    
    if ( hasSettings && pSettings->codec == handle->codec )
    {
        BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        NEXUS_AudioEncoder_P_ApplyCodecSettings(handle, handle->codec, &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
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

Description:
This is used to connect to the audio decoder as follows:

    NEXUS_AudioEncoder_AddInput(audioEncoder, NEXUS_AudioDecoder_GetConnector(audioDecoder));

***************************************************************************/
NEXUS_Error NEXUS_AudioEncoder_AddInput(
    NEXUS_AudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    BDBG_ASSERT(NULL != input);

    if ( handle->input )
    {
        BDBG_ERR(("An input is already connected to this processing stage"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Connect at input node */
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);

    if ( BERR_SUCCESS == errCode )
    {
        handle->input = input;
    }

    return errCode;
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

    if ( handle->input != input )
    {
        BDBG_ERR(("Input %p is not connected to this processing stage", (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Disconnect at input node */
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
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);

    if ( handle->input )
    {
        return NEXUS_AudioEncoder_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

BRAP_ProcessingStageHandle NEXUS_AudioEncoder_P_GetStageHandle(NEXUS_AudioEncoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioEncoder);
    
    /* Determine if codec has changed and update settings if it has */
    if ( handle->codec != handle->settings.codec )
    {
        BERR_Code errCode;
        BRAP_ProcessingType processingType;
        processingType = NEXUS_Audio_P_CodecToProcessingType(handle->settings.codec);
        BRAP_GetDefaultProcessingStageSettings(processingType, &g_NEXUS_StageSettings);
        NEXUS_AudioEncoder_P_ApplyCodecSettings(handle, handle->settings.codec, &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return NULL;
        }
        handle->codec = handle->settings.codec;
    }
        
    return handle->stage;
}
