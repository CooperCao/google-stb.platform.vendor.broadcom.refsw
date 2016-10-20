/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: Encoder
*    Specific APIs related to Audio Encoding
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_encoder);

BDBG_OBJECT_ID(BAPE_Encoder);
typedef struct BAPE_Encoder
{
    BDBG_OBJECT(BAPE_Encoder)
    BAPE_PathNode node;
    BAPE_EncoderSettings settings;
    BAPE_Ac3EncodeSettings ac3Settings;
    BAPE_DtsEncodeSettings dtsSettings;
    BAPE_AacEncodeSettings aacSettings, aacPlusSettings;
    BAPE_MpegEncodeSettings mp3Settings;
    BAPE_G711EncodeSettings g711Settings;
    BAPE_G723_1EncodeSettings g723_1Settings;
    BAPE_G726EncodeSettings g726Settings;
    BAPE_G729EncodeSettings g729Settings;
    BAPE_IlbcEncodeSettings ilbcSettings;
    BAPE_IsacEncodeSettings isacSettings;
    BAPE_OpusEncodeSettings opusSettings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
} BAPE_Encoder;

static const char *BAPE_Encoder_P_GetName(BAVC_AudioCompressionStd codec);
static void BAPE_Encoder_P_InputSampleRateChange_isr(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection, unsigned sampleRate);
static BERR_Code BAPE_Encoder_P_InputFormatChange(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static BERR_Code BAPE_Encoder_P_ApplyDspSettings(BAPE_EncoderHandle handle);
static BERR_Code BAPE_Encoder_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_Encoder_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_Encoder_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_Encoder_P_GetAllDefaultCodecSettings(BAPE_EncoderHandle handle);
static void BAPE_Encoder_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

static void BAPE_Encoder_P_GetDefaultAc3Settings(BAPE_Handle deviceHandle, BAPE_Ac3EncodeSettings *ac3Settings);
static void BAPE_Encoder_P_GetDefaultDtsSettings(BAPE_Handle deviceHandle, BAPE_DtsEncodeSettings *dtsSettings);
static void BAPE_Encoder_P_GetDefaultAacSettings(BAPE_Handle deviceHandle, BAPE_AacEncodeSettings *aacSettings);
static void BAPE_Encoder_P_GetDefaultAacPlusSettings(BAPE_Handle deviceHandle, BAPE_AacEncodeSettings *aacPlusSettings);
static void BAPE_Encoder_P_GetDefaultMp3Settings(BAPE_Handle deviceHandle, BAPE_MpegEncodeSettings *mp3Settings);
static void BAPE_Encoder_P_GetDefaultG711Settings(BAPE_Handle deviceHandle, BAPE_G711EncodeSettings *g711Settings);
static void BAPE_Encoder_P_GetDefaultG726Settings(BAPE_Handle deviceHandle, BAPE_G726EncodeSettings *g726Settings);
static void BAPE_Encoder_P_GetDefaultG729Settings(BAPE_Handle deviceHandle, BAPE_G729EncodeSettings *g729Settings);
static void BAPE_Encoder_P_GetDefaultG723_1Settings(BAPE_Handle deviceHandle, BAPE_G723_1EncodeSettings *g723_1Settings);
static void BAPE_Encoder_P_GetDefaultIlbcSettings(BAPE_Handle deviceHandle, BAPE_IlbcEncodeSettings *ilbcSettings);
static void BAPE_Encoder_P_GetDefaultIsacSettings(BAPE_Handle deviceHandle, BAPE_IsacEncodeSettings *isacSettings);
static void BAPE_Encoder_P_GetDefaultOpusSettings(BAPE_Handle deviceHandle, BAPE_OpusEncodeSettings *opusSettings);


/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void BAPE_Encoder_GetDefaultSettings(
    BAPE_EncoderSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->codec = BAVC_AudioCompressionStd_eAacAdts;
    pSettings->loudnessEquivalenceEnabled = true;
}

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
BERR_Code BAPE_Encoder_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EncoderSettings *pSettings,
    BAPE_EncoderHandle *pHandle             /* [out] */
    )
{
    BDSP_StageCreateSettings stageCreateSettings;
    BAPE_EncoderHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);

    handle = BKNI_Malloc(sizeof(BAPE_Encoder));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_Encoder));
    BDBG_OBJECT_SET(handle, BAPE_Encoder);
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_eEncoder, pSettings->codec, BAPE_ConnectorFormat_eMax, deviceHandle, handle);
    handle->node.pName = BAPE_Encoder_P_GetName(pSettings->codec);
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].pName = "compressed";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].pName = "compressed 4x";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].useBufferPool = true;
    handle->settings.codec = BAVC_AudioCompressionStd_eMax;

    /* Format and capabilities are set in SetSettings per-codec */

    /* Generic Routines */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* Encoder Specifics */
    handle->node.inputSampleRateChange_isr = BAPE_Encoder_P_InputSampleRateChange_isr;
    handle->node.inputFormatChange = BAPE_Encoder_P_InputFormatChange;
    handle->node.allocatePathFromInput = BAPE_Encoder_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_Encoder_P_StopPathFromInput;
    handle->node.freePathFromInput = BAPE_Encoder_P_FreePathFromInput;
    handle->node.removeInput = BAPE_Encoder_P_RemoveInputCallback;

    /* Create Stage Handle */
    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioEncode, &stageCreateSettings);
    /* Filter based on application bavc algo list and ape codec table */
    BAPE_P_PopulateSupportedBDSPAlgos( BDSP_AlgorithmType_eAudioEncode, NULL, 0, 
        (const bool *)stageCreateSettings.algorithmSupported, 
        stageCreateSettings.algorithmSupported);

    /* TODO: Expose supported algorithms option for application? */
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage = handle->hStage;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].hStage = handle->hStage;

    /* Init codec settings */
    BAPE_Encoder_P_GetAllDefaultCodecSettings(handle);
    errCode = BAPE_Encoder_SetSettings(handle, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_settings;
    }

    *pHandle = handle;
    return BERR_SUCCESS;

err_stage_create:
err_settings:
    BAPE_Encoder_Destroy(handle);
    return errCode;
}

/***************************************************************************
Summary:
    Close an Audio Encoder stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_Encoder_Destroy(
    BAPE_EncoderHandle handle
    )
{
    bool running;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    running = BAPE_PathNode_P_IsActive(&handle->node);
    BDBG_ASSERT(false == running);
    BDBG_ASSERT(NULL == handle->input);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_Encoder);
    BKNI_Free(handle);
}


/***************************************************************************
Summary:
    Get Settings for an Audio Encoder stage
***************************************************************************/
void BAPE_Encoder_GetSettings(
    BAPE_EncoderHandle handle,
    BAPE_EncoderSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set Settings for an Audio Encoder stage
***************************************************************************/
BERR_Code BAPE_Encoder_SetSettings(
    BAPE_EncoderHandle handle,
    const BAPE_EncoderSettings *pSettings
    )
{
    bool running;
    BERR_Code errCode;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;

    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_ASSERT(NULL != pSettings);

    running = BAPE_PathNode_P_IsActive(&handle->node);
    if ( running && pSettings->codec != handle->settings.codec )
    {
        BDBG_ERR(("Can not change codec while running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( pSettings->codec != handle->settings.codec )
    {        
        errCode = BDSP_Stage_SetAlgorithm(handle->hStage, BAPE_P_GetCodecAudioEncode(pSettings->codec));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    handle->settings = *pSettings;
    BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, pSettings->codec);
    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3Plus:
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eDts:
        /* AC3/DTS can be sent to SPDIF/HDMI */
        format.type = BAPE_DataType_eIec61937;
        break;
    default:
        /* Other codecs can only be sent to MuxOutput */
        format.type = BAPE_DataType_eCompressedRaw;
        break;
    }
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    format.type = BAPE_DataType_eIec61937x4;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], &format);
    if ( errCode ) 
    {
        return BERR_TRACE(errCode);
    }

    /* Voice Conferencing codecs only support mono.  DTS/AC3 support 5.1/2.0.  Others are 2.0 only */
    BAPE_FMT_P_InitCapabilities(&caps, NULL, NULL);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eG711:
    case BAVC_AudioCompressionStd_eG726:
    case BAVC_AudioCompressionStd_eG723_1:
    case BAVC_AudioCompressionStd_eG729:
    case BAVC_AudioCompressionStd_eIlbc:
    case BAVC_AudioCompressionStd_eIsac:
    case BAVC_AudioCompressionStd_eAmrNb:
    case BAVC_AudioCompressionStd_eAmrWb:
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmMono);
        break;
    case BAVC_AudioCompressionStd_eOpus:
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmMono);
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
        break;
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
    case BAVC_AudioCompressionStd_eDts:
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
        /* fall-through */
    default:
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
        break;
    }
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode ) 
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_Encoder_GetDefaultCodecSettings(
    BAPE_Handle deviceHandle,
    BAVC_AudioCompressionStd codec,
    BAPE_EncoderCodecSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    pSettings->codec = codec;
    switch ( codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
        BAPE_Encoder_P_GetDefaultAc3Settings(deviceHandle, &pSettings->codecSettings.ac3);
        break;
    case BAVC_AudioCompressionStd_eDts:
        BAPE_Encoder_P_GetDefaultDtsSettings(deviceHandle, &pSettings->codecSettings.dts);
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        BAPE_Encoder_P_GetDefaultAacSettings(deviceHandle, &pSettings->codecSettings.aac);
        break;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        BAPE_Encoder_P_GetDefaultAacPlusSettings(deviceHandle, &pSettings->codecSettings.aacPlus);
        break;
    case BAVC_AudioCompressionStd_eMpegL3:
        BAPE_Encoder_P_GetDefaultMp3Settings(deviceHandle, &pSettings->codecSettings.mp3);
        break;
    case BAVC_AudioCompressionStd_eG711:
        BAPE_Encoder_P_GetDefaultG711Settings(deviceHandle, &pSettings->codecSettings.g711);
        break;
    case BAVC_AudioCompressionStd_eG726:
        BAPE_Encoder_P_GetDefaultG726Settings(deviceHandle, &pSettings->codecSettings.g726);
        break;
    case BAVC_AudioCompressionStd_eG729:
        BAPE_Encoder_P_GetDefaultG729Settings(deviceHandle, &pSettings->codecSettings.g729);
        break;
    case BAVC_AudioCompressionStd_eG723_1:
        BAPE_Encoder_P_GetDefaultG723_1Settings(deviceHandle, &pSettings->codecSettings.g723_1);
        break;
    case BAVC_AudioCompressionStd_eIlbc:
        BAPE_Encoder_P_GetDefaultIlbcSettings(deviceHandle, &pSettings->codecSettings.ilbc);
        break;
    case BAVC_AudioCompressionStd_eIsac:
        BAPE_Encoder_P_GetDefaultIsacSettings(deviceHandle, &pSettings->codecSettings.isac);
        break;
    case BAVC_AudioCompressionStd_eOpus:
        BAPE_Encoder_P_GetDefaultOpusSettings(deviceHandle, &pSettings->codecSettings.opus);
        break;
    default:
        break;
    }
}

/***************************************************************************
Summary:
    Get Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
void BAPE_Encoder_GetCodecSettings(
    BAPE_EncoderHandle handle,
    BAVC_AudioCompressionStd codec,              /* the codec for which you are retrieving settings. */
    BAPE_EncoderCodecSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_ASSERT(NULL != pSettings);
    
    pSettings->codec = codec;
    switch ( codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
        pSettings->codecSettings.ac3 = handle->ac3Settings;
        break;
    case BAVC_AudioCompressionStd_eDts:
        pSettings->codecSettings.dts = handle->dtsSettings;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        pSettings->codecSettings.aac = handle->aacSettings;
        break;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        pSettings->codecSettings.aac = handle->aacPlusSettings;
        break;
    case BAVC_AudioCompressionStd_eMpegL3:
        pSettings->codecSettings.mp3 = handle->mp3Settings;
        break;
    case BAVC_AudioCompressionStd_eG711:
        pSettings->codecSettings.g711 = handle->g711Settings;
        break;
    case BAVC_AudioCompressionStd_eG723_1:
        pSettings->codecSettings.g723_1 = handle->g723_1Settings;
        break;
    case BAVC_AudioCompressionStd_eG726:
        pSettings->codecSettings.g726 = handle->g726Settings;
        break;
    case BAVC_AudioCompressionStd_eG729:
        pSettings->codecSettings.g729 = handle->g729Settings;
        break;
    case BAVC_AudioCompressionStd_eIlbc:
        pSettings->codecSettings.ilbc = handle->ilbcSettings;
        break;
    case BAVC_AudioCompressionStd_eIsac:
        pSettings->codecSettings.isac = handle->isacSettings;
        break;
    case BAVC_AudioCompressionStd_eOpus:
        pSettings->codecSettings.opus = handle->opusSettings;
        break;
    default:
        break;
    }
}

/***************************************************************************
Summary:
    Set Codec-Specific Settings for an Audio Encoder stage
***************************************************************************/
BERR_Code BAPE_Encoder_SetCodecSettings(
    BAPE_EncoderHandle handle,
    const BAPE_EncoderCodecSettings *pSettings
    )
{
    bool codecsEqual = false;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_ASSERT(NULL != pSettings);
    if ( pSettings->codec == handle->settings.codec )
    {
        codecsEqual = true;
    }

    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
        handle->ac3Settings = pSettings->codecSettings.ac3;
        break;
    case BAVC_AudioCompressionStd_eDts:
        handle->dtsSettings = pSettings->codecSettings.dts;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        handle->aacSettings = pSettings->codecSettings.aac;
        switch ( handle->settings.codec )
        {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
            codecsEqual=true;
            break;
        default:
            break;
        }
        break;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        handle->aacPlusSettings = pSettings->codecSettings.aacPlus;
        switch ( handle->settings.codec )
        {
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            codecsEqual=true;
            break;
        default:
            break;
        }
        break;
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:        
        switch ( handle->settings.codec )
        {
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
            codecsEqual=true;
            break;
        default:
            break;
        }
        break;
    case BAVC_AudioCompressionStd_eMpegL3:
        handle->mp3Settings = pSettings->codecSettings.mp3;
        break;
    case BAVC_AudioCompressionStd_eSbc:
        break;
    case BAVC_AudioCompressionStd_eG711:
        handle->g711Settings = pSettings->codecSettings.g711;
        break;
    case BAVC_AudioCompressionStd_eG726:
        handle->g726Settings = pSettings->codecSettings.g726;
        break;
    case BAVC_AudioCompressionStd_eG723_1:
        handle->g723_1Settings = pSettings->codecSettings.g723_1;
        break;
    case BAVC_AudioCompressionStd_eG729:
        handle->g729Settings = pSettings->codecSettings.g729;
        break;
    case BAVC_AudioCompressionStd_eIlbc:
        handle->ilbcSettings = pSettings->codecSettings.ilbc;
        break;
    case BAVC_AudioCompressionStd_eIsac:
        handle->isacSettings = pSettings->codecSettings.isac;
        break;
    case BAVC_AudioCompressionStd_eOpus:
        handle->opusSettings = pSettings->codecSettings.opus;
    default:
        break;
    }

    if ( codecsEqual )
    {
        errCode = BAPE_Encoder_P_ApplyDspSettings(handle);
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
***************************************************************************/
void BAPE_Encoder_GetConnector(
    BAPE_EncoderHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_ASSERT(NULL != pConnector);

    switch ( format )
    {
    case BAPE_ConnectorFormat_eCompressed:
    case BAPE_ConnectorFormat_eCompressed4x:
        *pConnector = &handle->node.connectors[format];
        break;
    default:
        BDBG_ERR(("Unsupported data path format %u", format));
        *pConnector = NULL;
        break;
    }
}

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_Encoder_AddInput(
    BAPE_EncoderHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Can not have more than one input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_PathNode_P_AddInput(&handle->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_Encoder_RemoveInput(
    BAPE_EncoderHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected", input->pParent->pName, input->pName, (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    errCode = BAPE_PathNode_P_RemoveInput(&handle->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_Encoder_RemoveAllInputs(
    BAPE_EncoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    if ( handle->input )
    {
        return BAPE_Encoder_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static const char *BAPE_Encoder_P_GetName(BAVC_AudioCompressionStd codec)
{
    switch ( codec )
    {
    case BAVC_AudioCompressionStd_eDts:
        return "DTS Encoder";
    case BAVC_AudioCompressionStd_eAc3:
        return "AC3 Encoder";
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        return "AAC Encoder";
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        return "AAC-HE Encoder";
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:
        return "MPEG Encoder";
    case BAVC_AudioCompressionStd_eMpegL3:
        return "MP3 Encoder";
    case BAVC_AudioCompressionStd_eSbc:
        return "SBC Encoder";
    default:
        return "Encoder";
    }
}

static void BAPE_Encoder_P_InputSampleRateChange_isr(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection, unsigned sampleRate)
{
    BAPE_EncoderHandle hEncoder;
    BAVC_AudioCompressionStd codec;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    hEncoder = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hEncoder, BAPE_Encoder);

    codec = hEncoder->settings.codec;

    BDBG_MSG(("InputSampleRateChange codec %s, rate %u", BAPE_P_GetCodecName(codec), sampleRate));
    if ( sampleRate == 48000 )
    {
        switch ( codec )
        {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
            if ( hEncoder->aacSettings.sampleRate == 32000 )
            {
                sampleRate = 32000;
            }
            break;
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            if ( hEncoder->aacSettings.sampleRate == 32000 )
            {
                sampleRate = 32000;
            }
            break;
        default:
            break;
        }
    }
    BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[BAPE_ConnectorFormat_eCompressed], sampleRate);
    BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[BAPE_ConnectorFormat_eCompressed4x], sampleRate*4);
}

static BERR_Code BAPE_Encoder_P_InputFormatChange(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BAPE_FMT_Descriptor outputFormat, outputFormat4x;
    BAVC_AudioCompressionStd codec;
    BAPE_EncoderHandle hEncoder;
    BAPE_DataType dataType;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pNewFormat);

    hEncoder = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hEncoder, BAPE_Encoder);

    BDBG_MSG(("InputFormatChange - new source %s, codec %s new rate %d", BAPE_FMT_P_SourceToString_isrsafe(pNewFormat->source), BAPE_FMT_P_GetSubTypeName_isrsafe(pNewFormat), pNewFormat->sampleRate));

    /* This module will convert the input to stereo.  All other fields are passthrough. */
    BAPE_Connector_P_GetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed], &outputFormat);
    BAPE_Connector_P_GetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed4x], &outputFormat4x);
    dataType = outputFormat.type;
    codec = BAPE_FMT_P_GetAudioCompressionStd_isrsafe(&outputFormat);
    outputFormat = *pNewFormat;
    outputFormat.type = dataType;
    BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&outputFormat, codec);
    if ( pNewFormat->sampleRate == 48000 )
    {
        switch ( codec )
        {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
            if ( hEncoder->aacSettings.sampleRate == 32000 )
            {
                outputFormat.sampleRate = 32000;
            }
            break;
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            if ( hEncoder->aacSettings.sampleRate == 32000 )
            {
                outputFormat.sampleRate = 32000;
            }
            break;
        default:
            break;
        }
    }
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed], &outputFormat);
    outputFormat.type = outputFormat4x.type;
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed4x], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;    
}

static BERR_Code BAPE_Encoder_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned output, input;
    BAPE_EncoderHandle handle;
    BDSP_DataType dataType;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);

    /* if we don't have any 4x connections, downgrade to AC3 */
    if ( handle->settings.codec == BAVC_AudioCompressionStd_eAc3Plus )
    {
        if ( BLST_SQ_FIRST(&pNode->connectors[BAPE_ConnectorFormat_eCompressed4x].connectionList) == NULL )
        {
            BDBG_MSG(("Falling back to AC3 only Encode"));
            handle->settings.codec = BAVC_AudioCompressionStd_eAc3;
        }
    }

    dataType = BAPE_DSP_P_GetDataTypeFromConnector(handle->input);
    /* Dolby transcoder only supports pulse or DDRE as it's source.  Check for pulse here, DDRE is handled in bape_dolby_digital_reencode.c */
    if ( handle->settings.codec == BAVC_AudioCompressionStd_eAc3 ||
         handle->settings.codec == BAVC_AudioCompressionStd_eAc3Plus )
    {
        if ( pConnection->pSource->pParent->type == BAPE_PathNodeType_eDecoder )
        {
            BAPE_DecoderHandle decoder = pConnection->pSource->pParent->pHandle;
            switch ( decoder->startSettings.codec )
            {
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacLoas:
            case BAVC_AudioCompressionStd_eAacPlusAdts:
            case BAVC_AudioCompressionStd_eAacPlusLoas:
#if BDSP_MS12_SUPPORT
            case BAVC_AudioCompressionStd_eAc3:
            case BAVC_AudioCompressionStd_eAc3Plus:
#endif
#if BDSP_MS10_SUPPORT || BDSP_MS12_SUPPORT
                dataType = BDSP_DataType_eDolbyTranscodeData;
#endif
                break;
            default:
                BDBG_ERR(("AC3 transcoder can only be used for AAC source material"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
        else
        {
            BDBG_ERR(("AC3 encoding is only supported when directly connected to an AAC decoder."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage, 
                                        dataType,
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_Encoder_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;    
}

static void BAPE_Encoder_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EncoderHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
}

static void BAPE_Encoder_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BAPE_Encoder_P_StopPathFromInput(pNode, pConnection);
}

static BERR_Code BAPE_Encoder_P_ApplyAc3Settings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
#if BDSP_MS12_SUPPORT
    BDSP_Raaga_Audio_DDPEncConfigParams userConfig;
#else
    BDSP_Raaga_Audio_DDTranscodeConfigParams userConfig;
#endif

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

#if BDSP_MS12_SUPPORT
    userConfig.ui32Mode = (handle->settings.codec == BAVC_AudioCompressionStd_eAc3Plus) ? 8 : 9;
    userConfig.ui32DolbyCertificationFlag = (handle->ac3Settings.certificationMode) ? 1 : 0;
    BDBG_ERR(("%s - setting userConfig ui32Mode=%d, ui32DolbyCertificationFlag=%d", __FUNCTION__, userConfig.ui32Mode, userConfig.ui32DolbyCertificationFlag));
#else
    userConfig.eSpdifHeaderEnable = handle->ac3Settings.spdifHeaderEnabled?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable;
#endif

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultAc3Settings(BAPE_Handle deviceHandle, BAPE_Ac3EncodeSettings *ac3Settings)
{
#if BDSP_MS12_SUPPORT
    BDSP_Raaga_Audio_DDPEncConfigParams userConfig;
    BDSP_Algorithm algo = BDSP_Algorithm_eDDPEncode;
#else
    BDSP_Raaga_Audio_DDTranscodeConfigParams userConfig;
    BDSP_Algorithm algo = BDSP_Algorithm_eAc3Encode;
#endif

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, algo) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(algo, &userConfig, sizeof(userConfig)));

#if BDSP_MS12_SUPPORT
    ac3Settings->certificationMode = (userConfig.ui32DolbyCertificationFlag == 1) ? true : false;
#else
    ac3Settings->spdifHeaderEnabled = (userConfig.eSpdifHeaderEnable == BDSP_AF_P_eEnable)?true:false;
#endif
}

static BERR_Code BAPE_Encoder_P_ApplyDtsSettings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_DtsBroadcastEncConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    userConfig.ui32SpdifHeaderEnable = handle->dtsSettings.spdifHeaderEnabled?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable;

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultDtsSettings(BAPE_Handle deviceHandle, BAPE_DtsEncodeSettings *dtsSettings)
{
    BDSP_Raaga_Audio_DtsBroadcastEncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eDtsCoreEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDtsCoreEncode, &userConfig, sizeof(userConfig)));
    dtsSettings->spdifHeaderEnabled = (userConfig.ui32SpdifHeaderEnable == 0)?false:true;
}

static BERR_Code BAPE_Encoder_P_ApplyAacSettings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_AacheEncConfigParams userConfig;
    const BAPE_AacEncodeSettings *pSettings;
    unsigned sbr;
    unsigned format;
    unsigned maxBitRate;

    switch ( handle->settings.codec )
    {
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        pSettings = &handle->aacSettings;
        sbr=0;
        format = (handle->settings.codec == BAVC_AudioCompressionStd_eAacAdts)?1:2;
        maxBitRate = 320000;
        break;
    default:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        pSettings = &handle->aacPlusSettings;
        sbr=1;
        format = (handle->settings.codec == BAVC_AudioCompressionStd_eAacPlusAdts)?1:2;
        maxBitRate = 52000;
        break;
    }

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( pSettings->bitRate < 16000 || pSettings->bitRate > maxBitRate )
    {
        BDBG_ERR(("Bit Rate for codec %u (%s) must be between 16000 and %u bps.", handle->settings.codec, BAPE_P_GetCodecName(handle->settings.codec), maxBitRate));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* TODO: This will be converted to an integer in an upcoming release */
    BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeBitRate, (BDSP_Raaga_Audio_AacEncodeBitRate)pSettings->bitRate);

    if ( pSettings->channelMode == BAPE_ChannelMode_e1_0 ) 
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 1);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeMonoChSelcect, (BDSP_Raaga_Audio_AacEncodeMonoChannelSelect)pSettings->monoMode);
    }
    else if ( pSettings->channelMode == BAPE_ChannelMode_e1_1 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 1);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeMonoChSelcect, BDSP_Raaga_Audio_AacEncodeMono_DualMono);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeMonoChSelcect, BDSP_Raaga_Audio_AacEncodeMono_Mix);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EnableSBR, sbr);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EncodeFormatType, format);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui3248SRC32Enable, pSettings->sampleRate == 32000 ? 1 : 0);
    if ( pSettings->complexity > (unsigned)BDSP_Raaga_Audio_AacEncodeComplexity_High )
    {
        BDBG_WRN(("AAC Complexity setting out of range.  Using highest complexity."));
        BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeComplexity, BDSP_Raaga_Audio_AacEncodeComplexity_High);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeComplexity, pSettings->complexity);
    }

    /* In the case of AAC-LC/AAC-LC+SBR aka AAC+ backed in ADTS set the Encode type to MPEG-2
       due to compatability with certain receivers/TVs. Also this keeps things in line for
       SCTE 193-1 and ISO/IEC 13818-1 */
    switch ( handle->settings.codec )
    {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeAdtsMpegType, BDSP_Raaga_Audio_AacEncodeAdtsMpeg2);
            break;
        default:
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            BAPE_DSP_P_SET_VARIABLE(userConfig, eAacEncodeAdtsMpegType, BDSP_Raaga_Audio_AacEncodeAdtsMpeg4);
            break;
    }


    switch ( handle->node.deviceHandle->settings.loudnessMode )
    {
    case BAPE_LoudnessEquivalenceMode_eAtscA85:
        /* A/85 is consistent with 24dB stereo output from the decoder */
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32InputVolLevel, -24);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32OutputVolLevel, -24);
        break;
    case BAPE_LoudnessEquivalenceMode_eEbuR128:
        /* EBU-R128 is consistent with 23dB stereo output from the decoder */
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32InputVolLevel, -23);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32OutputVolLevel, -23);
        break;
    default:
        /* Use default config from FW */
        break;
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultAacSettings(BAPE_Handle deviceHandle, BAPE_AacEncodeSettings *aacSettings)
{
    BDSP_Raaga_Audio_AacheEncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eAacEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAacEncode, &userConfig, sizeof(userConfig)));
    aacSettings->bitRate = 128000;
    BDBG_CASSERT(BDSP_Raaga_Audio_AacEncodeBitRate_e16kbps == 16000);  /* Just in case they ever change the rate enum */
    if ( userConfig.ui32bEncodeMono )
    {
        if ( userConfig.eAacEncodeMonoChSelcect == BDSP_Raaga_Audio_AacEncodeMono_DualMono )
        {
            aacSettings->channelMode = BAPE_ChannelMode_e1_1;
            aacSettings->monoMode = BAPE_MonoChannelMode_eMix;
        }
        else
        {
            aacSettings->channelMode = BAPE_ChannelMode_e1_0;
            aacSettings->monoMode = (BAPE_MonoChannelMode)userConfig.eAacEncodeMonoChSelcect;
        }        
    }
    else
    {
        aacSettings->channelMode = BAPE_ChannelMode_e2_0;
        aacSettings->monoMode = BAPE_MonoChannelMode_eMix;
    }
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Left==(int)BAPE_MonoChannelMode_eLeft);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Right==(int)BAPE_MonoChannelMode_eRight);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Mix==(int)BAPE_MonoChannelMode_eMix);
    aacSettings->sampleRate = 0;
    aacSettings->complexity = userConfig.eAacEncodeComplexity;
}

static void BAPE_Encoder_P_GetDefaultAacPlusSettings(BAPE_Handle deviceHandle, BAPE_AacEncodeSettings *aacPlusSettings)
{
    BDSP_Raaga_Audio_AacheEncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eAacEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAacEncode, &userConfig, sizeof(userConfig)));
    aacPlusSettings->bitRate = (unsigned)userConfig.eAacEncodeBitRate;
    BDBG_CASSERT(BDSP_Raaga_Audio_AacEncodeBitRate_e16kbps == 16000);  /* Just in case they ever change the rate enum */
    if ( userConfig.ui32bEncodeMono )
    {
        if ( userConfig.eAacEncodeMonoChSelcect == BDSP_Raaga_Audio_AacEncodeMono_DualMono )
        {
            aacPlusSettings->channelMode = BAPE_ChannelMode_e1_1;
            aacPlusSettings->monoMode = BAPE_MonoChannelMode_eMix;
        }
        else
        {
            aacPlusSettings->channelMode = BAPE_ChannelMode_e1_0;
            aacPlusSettings->monoMode = (BAPE_MonoChannelMode)userConfig.eAacEncodeMonoChSelcect;
        }
    }
    else
    {
        aacPlusSettings->channelMode = BAPE_ChannelMode_e2_0;
        aacPlusSettings->monoMode = BAPE_MonoChannelMode_eMix;
    }
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Left==(int)BAPE_MonoChannelMode_eLeft);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Right==(int)BAPE_MonoChannelMode_eRight);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_AacEncodeMono_Mix==(int)BAPE_MonoChannelMode_eMix);
    aacPlusSettings->sampleRate = 0;
    aacPlusSettings->complexity = userConfig.eAacEncodeComplexity;
}

static BERR_Code BAPE_Encoder_P_ApplyMp3Settings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_Mpeg1L3EncConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( handle->mp3Settings.bitRate < 32000 || handle->mp3Settings.bitRate > 320000 )
    {
        BDBG_ERR(("Bit Rate for codec %u (%s) must be between 32000 and 320000 bps.", handle->settings.codec, BAPE_P_GetCodecName(handle->settings.codec)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Make the bitrate an appropriate value */
    if ( handle->mp3Settings.bitRate < 40000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e32kbps);
    }
    else if ( handle->mp3Settings.bitRate < 48000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e40kbps);
    }
    else if ( handle->mp3Settings.bitRate < 56000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e48kbps);
    }
    else if ( handle->mp3Settings.bitRate < 64000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e56kbps);
    }
    else if ( handle->mp3Settings.bitRate < 80000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e64kbps);
    }
    else if ( handle->mp3Settings.bitRate < 96000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e80kbps);
    }
    else if ( handle->mp3Settings.bitRate < 112000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e96kbps);
    }
    else if ( handle->mp3Settings.bitRate < 128000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e112kbps);
    }
    else if ( handle->mp3Settings.bitRate < 160000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e128kbps);
    }
    else if ( handle->mp3Settings.bitRate < 192000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e160kbps);
    }
    else if ( handle->mp3Settings.bitRate < 224000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e192kbps);
    }
    else if ( handle->mp3Settings.bitRate < 256000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e224kbps);
    }
    else if ( handle->mp3Settings.bitRate < 320000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e256kbps);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeBitRate, BDSP_Raaga_Audio_Mp3EncodeBitRate_e320kbps);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, ePrivateBit, handle->mp3Settings.privateBit?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable);
    BAPE_DSP_P_SET_VARIABLE(userConfig, eCopyright, handle->mp3Settings.copyrightBit?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable);
    BAPE_DSP_P_SET_VARIABLE(userConfig, eOriginal, handle->mp3Settings.originalBit?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable);
    BAPE_DSP_P_SET_VARIABLE(userConfig, eEmphasisType, (BDSP_Raaga_Audio_Mp3EncodeEmphasisType)handle->mp3Settings.emphasisMode);
    if ( handle->mp3Settings.channelMode == BAPE_ChannelMode_e1_0 ) 
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 1);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeMonoChannelSelect, (BDSP_Raaga_Audio_Mp3EncodeMonoChannelSelect)handle->mp3Settings.monoMode);
    }
    else if ( handle->mp3Settings.channelMode == BAPE_ChannelMode_e1_1 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 1);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeMonoChannelSelect, BDSP_Raaga_Audio_Mp3EncodeMono_DualMono);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32bEncodeMono, 0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eMp3EncodeMonoChannelSelect, BDSP_Raaga_Audio_Mp3EncodeMono_Mix);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultMp3Settings(BAPE_Handle deviceHandle, BAPE_MpegEncodeSettings *mp3Settings)
{
    BDSP_Raaga_Audio_Mpeg1L3EncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eMpegAudioEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eMpegAudioEncode, &userConfig, sizeof(userConfig)));
    mp3Settings->bitRate = (unsigned)userConfig.eMp3EncodeBitRate * 1000;
    BDBG_CASSERT(BDSP_Raaga_Audio_Mp3EncodeBitRate_e32kbps == 32);  /* Just in case they ever change the rate enum */
    mp3Settings->privateBit = (userConfig.ePrivateBit == BDSP_AF_P_eEnable)?true:false;
    mp3Settings->copyrightBit = (userConfig.eCopyright == BDSP_AF_P_eEnable)?true:false;
    mp3Settings->originalBit = (userConfig.eOriginal == BDSP_AF_P_eEnable)?true:false;
    mp3Settings->emphasisMode = (BAPE_MpegEmphasisMode)userConfig.eEmphasisType;
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eNone==(int)BAPE_MpegEmphasisMode_eNone);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeEmphasisType_e50_15uSeconds==(int)BAPE_MpegEmphasisMode_e50_15ms);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eReserved==(int)BAPE_MpegEmphasisMode_eReserved);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeEmphasisType_eCCITTJ_17==(int)BAPE_MpegEmphasisMode_eCcit_J17);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeMono_Left==(int)BAPE_MonoChannelMode_eLeft);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeMono_Right==(int)BAPE_MonoChannelMode_eRight);
    BDBG_CASSERT((int)BDSP_Raaga_Audio_Mp3EncodeMono_Mix==(int)BAPE_MonoChannelMode_eMix);

    if ( userConfig.ui32bEncodeMono )
    {
        mp3Settings->channelMode = (userConfig.eMp3EncodeMonoChannelSelect == BDSP_Raaga_Audio_Mp3EncodeMono_DualMono)? BAPE_ChannelMode_e1_1:BAPE_ChannelMode_e1_0;
        mp3Settings->monoMode = (userConfig.eMp3EncodeMonoChannelSelect == BDSP_Raaga_Audio_Mp3EncodeMono_DualMono)?BAPE_MonoChannelMode_eMix:(BAPE_MonoChannelMode)userConfig.eMp3EncodeMonoChannelSelect;
    }
    else
    {
        mp3Settings->channelMode = BAPE_ChannelMode_e2_0;
        mp3Settings->monoMode = BAPE_MonoChannelMode_eMix;
    }
}

static BERR_Code BAPE_Encoder_P_ApplyG711G726Settings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_G711_G726EncConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( handle->settings.codec == BAVC_AudioCompressionStd_eG711 )
    {
        /* G.711 */
        BAPE_DSP_P_SET_VARIABLE(userConfig, eCompressionType, (handle->g711Settings.compressionMode == BAPE_G711G726CompressionMode_eUlaw)?BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726:BDSP_Raaga_Audio_eCompressionType_aLaw_disableG726);
        BAPE_DSP_P_SET_VARIABLE(userConfig, eBitRate, BDSP_Raaga_Audio_eBitRate_64kbps);
    }
    else
    {
        /* G.726 */
        BAPE_DSP_P_SET_VARIABLE(userConfig, eCompressionType, (handle->g726Settings.compressionMode == BAPE_G711G726CompressionMode_eUlaw)?BDSP_Raaga_Audio_eCompressionType_uLaw_G726:BDSP_Raaga_Audio_eCompressionType_aLaw_G726);
        if ( handle->g726Settings.bitRate <= 16*1024 )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig, eBitRate, BDSP_Raaga_Audio_eBitRate_16kbps);
        }
        else if ( handle->g726Settings.bitRate <= 24*1024 )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig, eBitRate, BDSP_Raaga_Audio_eBitRate_24kbps);
        }
        else if ( handle->g726Settings.bitRate <= 32*1024 )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig, eBitRate, BDSP_Raaga_Audio_eBitRate_32kbps);
        }
        else
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig, eBitRate, BDSP_Raaga_Audio_eBitRate_40kbps);
        }
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultG711Settings(BAPE_Handle deviceHandle, BAPE_G711EncodeSettings *g711Settings)
{
    BDSP_Raaga_Audio_G711_G726EncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eG711Encode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eG711Encode, &userConfig, sizeof(userConfig)));

    switch ( userConfig.eCompressionType )
    {
    case BDSP_Raaga_Audio_eCompressionType_uLaw_G726:
    case BDSP_Raaga_Audio_eCompressionType_uLaw_ext:
    case BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726:
        g711Settings->compressionMode = BAPE_G711G726CompressionMode_eUlaw;
        break;
    default:
        g711Settings->compressionMode = BAPE_G711G726CompressionMode_eAlaw;
        break;
    }
}

static void BAPE_Encoder_P_GetDefaultG726Settings(BAPE_Handle deviceHandle, BAPE_G726EncodeSettings *g726Settings)
{
    BDSP_Raaga_Audio_G711_G726EncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eG726Encode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eG726Encode, &userConfig, sizeof(userConfig)));

    switch ( userConfig.eCompressionType )
    {
    case BDSP_Raaga_Audio_eCompressionType_uLaw_G726:
    case BDSP_Raaga_Audio_eCompressionType_uLaw_ext:
    case BDSP_Raaga_Audio_eCompressionType_uLaw_disableG726:
        g726Settings->compressionMode = BAPE_G711G726CompressionMode_eUlaw;
        break;
    default:
        g726Settings->compressionMode = BAPE_G711G726CompressionMode_eAlaw;
        break;
    }

    switch ( userConfig.eBitRate )
    {
    case BDSP_Raaga_Audio_eBitRate_16kbps:
        g726Settings->bitRate = 16000;
        break;
    case BDSP_Raaga_Audio_eBitRate_24kbps:
        g726Settings->bitRate = 24000;
        break;
    default:
    case BDSP_Raaga_Audio_eBitRate_32kbps:
        g726Settings->bitRate = 24000;
        break;
    case BDSP_Raaga_Audio_eBitRate_40kbps:
        g726Settings->bitRate = 24000;
        break;
    }
}


static BERR_Code BAPE_Encoder_P_ApplyG729Settings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_G729EncoderUserConfig userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32DtxEnable, handle->g729Settings.dtxEnabled ? 1 : 0);
    if ( handle->g729Settings.bitRate <= 6554 ) /* 6.4 * 1024 */
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Bitrate, 0);
    }
    else if ( handle->g729Settings.bitRate <= 8*1024 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Bitrate, 1);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Bitrate, 2);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultG729Settings(BAPE_Handle deviceHandle, BAPE_G729EncodeSettings *g729Settings)
{
    BDSP_Raaga_Audio_G729EncoderUserConfig userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eG729Encode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eG729Encode, &userConfig, sizeof(userConfig)));

    g729Settings->dtxEnabled = userConfig.ui32DtxEnable ? true : false;
    switch ( userConfig.ui32Bitrate )
    {
    default:
    case 0:
        g729Settings->bitRate = 6400;
        break;
    case 1:
        g729Settings->bitRate = 8000;
        break;
    case 2:
        g729Settings->bitRate = 11800;
        break;
    }
}

static BERR_Code BAPE_Encoder_P_ApplyG723_1Settings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_G723EncoderUserConfig userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, UseVx, handle->g723_1Settings.vadEnabled ? 1 : 0);
    BAPE_DSP_P_SET_VARIABLE(userConfig, UseHp, handle->g723_1Settings.hpfEnabled ? 1 : 0);
    if ( handle->g729Settings.bitRate < 6300 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, WrkRate, 0);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, WrkRate, 1);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultG723_1Settings(BAPE_Handle deviceHandle, BAPE_G723_1EncodeSettings *g723_1Settings)
{
    BDSP_Raaga_Audio_G723EncoderUserConfig userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eG723_1Encode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eG723_1Encode, &userConfig, sizeof(userConfig)));

    g723_1Settings->vadEnabled = userConfig.UseVx ? true : false;
    g723_1Settings->hpfEnabled = userConfig.UseHp ? true : false;
    switch ( userConfig.WrkRate )
    {
    default:
    case 0:
        g723_1Settings->bitRate = 6300;
        break;
    case 1:
        g723_1Settings->bitRate = 5300;
        break;
    }
}

static BERR_Code BAPE_Encoder_P_ApplyIlbcSettings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_ILBCConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, mode, (handle->ilbcSettings.frameLength == 30) ? 30 : 20);

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultIlbcSettings(BAPE_Handle deviceHandle, BAPE_IlbcEncodeSettings *ilbcSettings){
    BDSP_Raaga_Audio_ILBCConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eiLBCEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eiLBCEncode, &userConfig, sizeof(userConfig)));

    ilbcSettings->frameLength = userConfig.mode;
}

static BERR_Code BAPE_Encoder_P_ApplyIsacSettings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_ISACConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, ui16frameLen, (handle->isacSettings.frameLength == 60) ? 60 : 30);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui16CodingMode, (handle->isacSettings.codingMode == BAPE_IsacCodingMode_eIndependent) ? 1 : 0);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui16nbTest, (handle->isacSettings.bandMode == BAPE_IsacBandMode_eNarrow) ? 1 : 0);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui16fixedFL, (handle->isacSettings.fixedFrameLength == true) ? 1 : 0);
    if ( handle->isacSettings.bitrate < 10000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16rateBPS, 10000);
    }
    else if ( handle->isacSettings.bitrate > 32000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16rateBPS, 32000);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16rateBPS, handle->isacSettings.bitrate);
    }

    if ( handle->isacSettings.bottleneck < 10000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16bottleneck, 10000);
    }
    else if ( handle->isacSettings.bottleneck > 32000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16bottleneck, 32000);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16bottleneck, handle->isacSettings.bottleneck);
    }

    if ( handle->isacSettings.payload < 100 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadSize, 100);
    }
    else if ( handle->isacSettings.payload > 400 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadSize, 400);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadSize, handle->isacSettings.payload);
    }

    if ( handle->isacSettings.framerate < 32000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadRate, 32000);
    }
    else if ( handle->isacSettings.framerate > 53400 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadRate, 53400);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui16payloadRate, handle->isacSettings.framerate);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultIsacSettings(BAPE_Handle deviceHandle, BAPE_IsacEncodeSettings *isacSettings)
{
    BDSP_Raaga_Audio_ISACConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eiSACEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eiSACEncode, &userConfig, sizeof(userConfig)));

    isacSettings->frameLength = userConfig.ui16frameLen;
    isacSettings->codingMode = (userConfig.ui16CodingMode == 1) ? BAPE_IsacCodingMode_eIndependent : BAPE_IsacCodingMode_eAdaptive;
    isacSettings->bandMode = (userConfig.ui16nbTest == 1) ? BAPE_IsacBandMode_eNarrow : BAPE_IsacBandMode_eWide;
    isacSettings->fixedFrameLength = (userConfig.ui16fixedFL == 1) ? true : false;
    isacSettings->bitrate = userConfig.ui16rateBPS;
    isacSettings->bottleneck = userConfig.ui16bottleneck;
    isacSettings->payload = userConfig.ui16payloadSize;
    isacSettings->framerate = userConfig.ui16payloadRate;
}

static BERR_Code BAPE_Encoder_P_ApplyOpusSettings(BAPE_EncoderHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_OpusEncConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( handle->opusSettings.bitRate < 6000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32BitRate, 10000);
    }
    else if ( handle->opusSettings.bitRate > 510000 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32BitRate, 510000);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32BitRate, handle->opusSettings.bitRate);
    }

    if ( handle->opusSettings.frameSize  <= 120 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 120);
    }
    else if ( handle->opusSettings.frameSize <= 240 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 240);
    }
    else if ( handle->opusSettings.frameSize <= 480 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 480);
    }
    else if ( handle->opusSettings.frameSize <= 960 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 960);
    }
    else if ( handle->opusSettings.frameSize <= 960 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 960);
    }
    else if ( handle->opusSettings.frameSize <= 1920 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 1920);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32FrameSize, 2880);
    }

    if ( handle->opusSettings.encodeMode == BAPE_OpusEncodeMode_eSilk )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EncodeMode, 0);
    }
    else if ( handle->opusSettings.encodeMode == BAPE_OpusEncodeMode_eCELT )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EncodeMode, 2);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EncodeMode, 1);
    }

    if ( handle->opusSettings.bitRateType == BAPE_OpusBitRateType_eCBR )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32VBREnabled, 0);
    }
    else if ( handle->opusSettings.bitRateType == BAPE_OpusBitRateType_eCVBR )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32VBREnabled, 2);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32VBREnabled, 1);
    }

    if ( handle->opusSettings.complexity  > 10 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Complexity, 10);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Complexity, handle->opusSettings.complexity);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Encoder_P_GetDefaultOpusSettings(BAPE_Handle deviceHandle, BAPE_OpusEncodeSettings *opusSettings)
{
    BDSP_Raaga_Audio_OpusEncConfigParams userConfig;

    if ( !BAPE_DSP_P_AlgorithmSupported(deviceHandle, BDSP_Algorithm_eOpusEncode) )
        return;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eOpusEncode, &userConfig, sizeof(userConfig)));

    opusSettings->bitRate = userConfig.ui32BitRate;
    opusSettings->frameSize = userConfig.ui32FrameSize;
    opusSettings->complexity = userConfig.ui32Complexity;

    if (userConfig.ui32EncodeMode == 0)
    {
        opusSettings->encodeMode = BAPE_OpusEncodeMode_eSilk;
    }
    else if (userConfig.ui32EncodeMode == 2)
    {
        opusSettings->encodeMode = BAPE_OpusEncodeMode_eCELT;
    }
    else
    {
        opusSettings->encodeMode = BAPE_OpusEncodeMode_eHybrid;
    }

    if (userConfig.ui32VBREnabled == 0)
    {
        opusSettings->bitRateType = BAPE_OpusBitRateType_eCBR;
    }
    else if (userConfig.ui32VBREnabled == 2)
    {
        opusSettings->bitRateType = BAPE_OpusBitRateType_eCVBR;
    }
    else
    {
        opusSettings->bitRateType = BAPE_OpusBitRateType_eVBR;
    }
}


static BERR_Code BAPE_Encoder_P_ApplyDspSettings(BAPE_EncoderHandle handle)
{
    BDSP_Algorithm encodeType;
    BDBG_OBJECT_ASSERT(handle, BAPE_Encoder);
    encodeType = BAPE_P_GetCodecAudioEncode(handle->settings.codec);
    switch ( encodeType )
    {
    case BDSP_Algorithm_eAc3Encode:
    case BDSP_Algorithm_eDDPEncode:
        return BAPE_Encoder_P_ApplyAc3Settings(handle);
    case BDSP_Algorithm_eDtsCoreEncode:
        return BAPE_Encoder_P_ApplyDtsSettings(handle);
    case BDSP_Algorithm_eAacEncode:
        return BAPE_Encoder_P_ApplyAacSettings(handle);
    case BDSP_Algorithm_eMpegAudioEncode:
        return BAPE_Encoder_P_ApplyMp3Settings(handle);
    case BDSP_Algorithm_eG711Encode:
    case BDSP_Algorithm_eG726Encode:
        return BAPE_Encoder_P_ApplyG711G726Settings(handle);
    case BDSP_Algorithm_eG729Encode:
        return BAPE_Encoder_P_ApplyG729Settings(handle);
    case BDSP_Algorithm_eG723_1Encode:
        return BAPE_Encoder_P_ApplyG723_1Settings(handle);
    case BDSP_Algorithm_eiLBCEncode:
        return BAPE_Encoder_P_ApplyIlbcSettings(handle);
    case BDSP_Algorithm_eiSACEncode:
        return BAPE_Encoder_P_ApplyIsacSettings(handle);
    case BDSP_Algorithm_eOpusEncode:
        return BAPE_Encoder_P_ApplyOpusSettings(handle);
    default:
        return BERR_SUCCESS;
    }
}

static void BAPE_Encoder_P_GetAllDefaultCodecSettings(BAPE_EncoderHandle handle)
{
    BAPE_Encoder_P_GetDefaultAc3Settings(handle->node.deviceHandle, &handle->ac3Settings);
    BAPE_Encoder_P_GetDefaultDtsSettings(handle->node.deviceHandle, &handle->dtsSettings);
    BAPE_Encoder_P_GetDefaultAacSettings(handle->node.deviceHandle, &handle->aacSettings);
    BAPE_Encoder_P_GetDefaultAacPlusSettings(handle->node.deviceHandle, &handle->aacPlusSettings);
    BAPE_Encoder_P_GetDefaultMp3Settings(handle->node.deviceHandle, &handle->mp3Settings);
    BAPE_Encoder_P_GetDefaultG711Settings(handle->node.deviceHandle, &handle->g711Settings);
    BAPE_Encoder_P_GetDefaultG726Settings(handle->node.deviceHandle, &handle->g726Settings);
    BAPE_Encoder_P_GetDefaultG729Settings(handle->node.deviceHandle, &handle->g729Settings);
    BAPE_Encoder_P_GetDefaultG723_1Settings(handle->node.deviceHandle, &handle->g723_1Settings);
    BAPE_Encoder_P_GetDefaultIlbcSettings(handle->node.deviceHandle, &handle->ilbcSettings);
    BAPE_Encoder_P_GetDefaultIsacSettings(handle->node.deviceHandle, &handle->isacSettings);
    BAPE_Encoder_P_GetDefaultOpusSettings(handle->node.deviceHandle, &handle->opusSettings);
}

static void BAPE_Encoder_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_Encoder_RemoveInput(pNode->pHandle, pConnector);
}
