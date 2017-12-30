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
*   API name: Post Processor
*    Specific APIs related to Post Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp.h"
#endif

BDBG_MODULE(bape_processor);

BDBG_OBJECT_ID(BAPE_Processor);

#if BAPE_CHIP_HAS_POST_PROCESSING
typedef struct BAPE_Processor
{
    BDBG_OBJECT(BAPE_Processor)
    BAPE_PathNode node;
    BAPE_PostProcessorType type;
    BAPE_ProcessorSettings settings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
    BDSP_Algorithm bdspAlgo;
    unsigned sampleRate;
} BAPE_Processor;

static BERR_Code BAPE_Processor_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_Processor_P_ApplyDspSettings(BAPE_ProcessorHandle handle);
static void BAPE_Processor_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_Processor_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);
static void BAPE_Processor_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);
static BERR_Code BAPE_Processor_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);

void BAPE_Processor_GetDefaultCreateSettings(
    BAPE_ProcessorCreateSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = BAPE_PostProcessorType_eMax;
}

static void BAPE_Processor_P_GetDefaultSettings(
    BAPE_PostProcessorType type,
    BAPE_ProcessorSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    switch ( type )
    {
    case BAPE_PostProcessorType_eKaraokeVocal:
        {
            BDSP_Raaga_Audio_VocalPPConfigParams dspSettings;
            BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eVocalPP, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.karaokeVocal.echo.enabled = false;
            pSettings->settings.karaokeVocal.echo.attenuation = (unsigned)((((int64_t)dspSettings.i32EchoAttn)*100) >> 31);
            pSettings->settings.karaokeVocal.echo.delay = dspSettings.i32EchoDelayInMs;
        }
        break;
    case BAPE_PostProcessorType_eFade:
        {
            BDSP_Raaga_Audio_FadeCtrlConfigParams dspSettings;
            BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eFadeCtrl, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.fade.type = dspSettings.ui32EasingFunctionType;
            pSettings->settings.fade.level = 100;
            pSettings->settings.fade.duration = 100;
        }
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
        {
            BDSP_Raaga_Audio_TsmCorrectionConfigParams dspSettings;
            BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eTsmCorrection, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.advTsm.mode = (BAPE_AdvancedTsmMode)dspSettings.ui32TsmCorrectionMode;
        }
        break;
    case BAPE_PostProcessorType_eAmbisonic:
        {
            BDSP_Raaga_Audio_AmbisonicsConfigParams dspSettings;
            BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAmbisonics, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.ambisonic.ambisonicSource = dspSettings.ui32AmbisonicProcess;
            pSettings->settings.ambisonic.yaw = dspSettings.ui32Yaw;
            pSettings->settings.ambisonic.pitch = dspSettings.ui32Pitch;
            pSettings->settings.ambisonic.roll = dspSettings.ui32Roll;
        }
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", type));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
        break; /* unreachable */
    }
}

static bool BAPE_Processor_P_SupportsMultichannelOutput(BAPE_PostProcessorType type)
{
    switch ( type )
    {
    default:
    case BAPE_PostProcessorType_eKaraokeVocal:
    case BAPE_PostProcessorType_eFade:
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
    case BAPE_PostProcessorType_eAmbisonic:
        return true;
        break; /* unreachable */
    }

    return false;
}

/***************************************************************************
Summary:
    Open an Processor stage
***************************************************************************/
BERR_Code BAPE_Processor_Create(
    BAPE_Handle deviceHandle,
    const BAPE_ProcessorCreateSettings *pSettings,
    BAPE_ProcessorHandle *pHandle
    )
{
    BAPE_ProcessorHandle handle;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BDSP_StageCreateSettings stageCreateSettings;
    BDSP_Algorithm bdspAlgo;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( NULL == pSettings )
    {
        BDBG_ERR(("settings cannot be NULL for BAPE_Processor_Create"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Processor));
    if ( NULL == handle )
    {
        BDBG_ERR(("Could not allocate %lu bytes for BAPE_Processor handle", (unsigned long)sizeof(BAPE_Processor)));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Processor));
    BDBG_OBJECT_SET(handle, BAPE_Processor);
    handle->type = pSettings->type;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, handle->type, 2, deviceHandle, handle);
    switch ( handle->type )
    {
    case BAPE_PostProcessorType_eKaraokeVocal:
        handle->node.pName = "KaraokeVocal";
        bdspAlgo = BDSP_Algorithm_eVocalPP;
        break;
    case BAPE_PostProcessorType_eFade:
        handle->node.pName = "Fade";
        bdspAlgo = BDSP_Algorithm_eFadeCtrl;
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
        handle->node.pName = "AdvancedTsm";
        bdspAlgo = BDSP_Algorithm_eTsmCorrection;
        break;
    case BAPE_PostProcessorType_eAmbisonic:
        handle->node.pName = "Ambisonic";
        bdspAlgo = BDSP_Algorithm_eAmbisonics;
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_bad_type;
        break; /* unreachable */
    }

    BAPE_Processor_P_GetDefaultSettings(handle->type, &handle->settings);

    handle->node.connectors[BAPE_ConnectorFormat_eStereo].pName = "stereo";
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].useBufferPool = true;
    BAPE_FMT_P_InitDescriptor(&format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    if ( BAPE_Processor_P_SupportsMultichannelOutput(handle->type) )
    {
        handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].pName = "multichannel";
        handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].useBufferPool = true;
        BAPE_FMT_P_InitDescriptor(&format);
        format.source = BAPE_DataSource_eDspBuffer;
        format.type = BAPE_DataType_ePcm5_1;
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], &format);
        if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }
    }

    /* Processor works with 2.0 or 5.1 source content from the DSP */
    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_caps; }

    /* Generic Routines */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* Processor Specifics */
    handle->node.allocatePathFromInput = BAPE_Processor_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_Processor_P_StopPathFromInput;
    handle->node.removeInput = BAPE_Processor_P_RemoveInputCallback;
    handle->node.inputSampleRateChange_isr = BAPE_Processor_P_InputSampleRateChange_isr;
    handle->node.inputFormatChange = BAPE_Processor_P_InputFormatChange;


    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[bdspAlgo] = true;
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    handle->bdspAlgo = bdspAlgo;
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].hStage = handle->hStage;
    handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].hStage = handle->hStage;

    *pHandle = handle;

    return BERR_SUCCESS;

err_bad_type:
err_stage_create:
err_connector_format:
err_caps:
    BAPE_Processor_Destroy(handle);
    return errCode;
}

void BAPE_Processor_Destroy(
    BAPE_ProcessorHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_Processor);
    BKNI_Free(handle);
}

void BAPE_Processor_GetSettings(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_Processor_SetSettings(
    BAPE_ProcessorHandle handle,
    const BAPE_ProcessorSettings *pSettings
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_Processor_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Processor_P_GetFadeStatus(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorStatus *pStatus    /* [out] Status */
    )
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_FadeCtrlPPStatusInfo status;

    errCode = BDSP_Stage_GetStatus(handle->hStage, &status, sizeof(status));
    if ( errCode )
    {
        BERR_TRACE(errCode);
        return;
    }

    if ( status.ui32StatusValid == 0 )
    {
        pStatus->valid = true;
        pStatus->status.fade.active = (status.ui32FadeActiveStatus==1) ? true : false;
        pStatus->status.fade.remaining = status.ui32RemainingDuration;
        pStatus->status.fade.level = status.ui32CurrentVolumeLevel;
        BDBG_MSG(("%s: active %u, remaining %lu, level %lu", BSTD_FUNCTION,
                  (unsigned)pStatus->status.fade.active,
                  (unsigned long)pStatus->status.fade.remaining,
                  (unsigned long)pStatus->status.fade.level));
    }
}

static void BAPE_Processor_P_GetAdvancedTsmStatus(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorStatus *pStatus    /* [out] Status */
    )
{
    BERR_Code errCode;
    BDSP_Raaga_TsmCorrectionPPStatus status;

    errCode = BDSP_Stage_GetStatus(handle->hStage, &status, sizeof(status));
    if ( errCode )
    {
        BERR_TRACE(errCode);
        return;
    }

    if ( status.ui32StatusValid == 1 )
    {
        pStatus->valid = true;
        pStatus->status.advTsm.mode = handle->settings.settings.advTsm.mode;
        pStatus->status.advTsm.pts = status.ui32PTS;
        pStatus->status.advTsm.ptsValid = (status.ui32PTSValid==1) ? true : false;
        switch (status.ui32PTSType) {
        case 0:
            pStatus->status.advTsm.ptsType = BAVC_PTSType_eCoded;
            break;
        case 1:
            pStatus->status.advTsm.ptsType = BAVC_PTSType_eInterpolatedFromValidPTS;
            break;
        case 2:
            pStatus->status.advTsm.ptsType = BAVC_PTSType_eInterpolatedFromInvalidPTS;
            break;
        default:
            pStatus->status.advTsm.ptsType = BAVC_PTSType_eMax;
            break;
        }
        pStatus->status.advTsm.correction = status.i32TimeInMsecAdjusted;
        BDBG_MSG(("%s: pts %u, valid %u, type %d, correction %d", BSTD_FUNCTION,
                  (unsigned)pStatus->status.advTsm.pts,
                  (unsigned)pStatus->status.advTsm.ptsValid,
                  (int)pStatus->status.advTsm.ptsType,
                  pStatus->status.advTsm.correction));
    }
}

void BAPE_Processor_GetStatus(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorStatus *pStatus    /* [out] Status */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    switch ( handle->type )
    {
    case BAPE_PostProcessorType_eKaraokeVocal:
    case BAPE_PostProcessorType_eAmbisonic:
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
        BAPE_Processor_P_GetAdvancedTsmStatus(handle, pStatus);
        break;
    case BAPE_PostProcessorType_eFade:
        BAPE_Processor_P_GetFadeStatus(handle, pStatus);
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }
}

void BAPE_Processor_GetConnector(
    BAPE_ProcessorHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(NULL != pConnector);

    *pConnector = NULL;
    switch ( format )
    {
    case BAPE_ConnectorFormat_eStereo:
        switch ( handle->type )
        {
        case BAPE_PostProcessorType_eKaraokeVocal:
        case BAPE_PostProcessorType_eFade:
        case BAPE_PostProcessorType_eAdvancedTsm:
        case BAPE_PostProcessorType_eAmbisonic:
            *pConnector = &handle->node.connectors[BAPE_ConnectorFormat_eStereo];
            break;
        default:
            break;
        }
        break;
    case BAPE_ConnectorFormat_eMultichannel:
        switch ( handle->type )
        {
        case BAPE_PostProcessorType_eAdvancedTsm:
        case BAPE_PostProcessorType_eAmbisonic:
            *pConnector = &handle->node.connectors[BAPE_ConnectorFormat_eMultichannel];
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if ( pConnector == NULL )
    {
        BDBG_ERR(("BAPE_Processor type %d does not support connector format %d", handle->type, format));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}


BERR_Code BAPE_Processor_AddInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
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


BERR_Code BAPE_Processor_RemoveInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
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


BERR_Code BAPE_Processor_RemoveAllInputs(
    BAPE_ProcessorHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    if ( handle->input )
    {
        return BAPE_Processor_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BDSP_DataType BAPE_Processor_P_DetermineOutputFormat(BAPE_ProcessorHandle handle)
{
    BAPE_Connector connector = NULL;
    switch ( handle->type )
    {
    default:
        break;
    case BAPE_PostProcessorType_eKaraokeVocal:
    case BAPE_PostProcessorType_eFade:
        if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]) > 0 )
        {
            connector = &handle->node.connectors[BAPE_ConnectorFormat_eStereo];
        }
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
        if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]) > 0 && BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel]) > 0 )
        {
            BDBG_ERR(("Advanced Tsm Processor does not support simultaneous stereo and multichannel consumers"));
        }
        else if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel]) > 0 )
        {
            connector = &handle->node.connectors[BAPE_ConnectorFormat_eMultichannel];
        }
        else if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]) > 0 )
        {
            connector = &handle->node.connectors[BAPE_ConnectorFormat_eStereo];
        }
        break;
    case BAPE_PostProcessorType_eAmbisonic:
        if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel]) > 0 )
        {
            connector = &handle->node.connectors[BAPE_ConnectorFormat_eMultichannel];
        }
        else if ( BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]) > 0 )
        {
            connector = &handle->node.connectors[BAPE_ConnectorFormat_eStereo];
        }
        break;
    }

    if ( connector )
    {
        return BAPE_DSP_P_GetDataTypeFromConnector(connector);
    }

    return BDSP_DataType_eMax;
}

static BERR_Code BAPE_Processor_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned input, output;
    BAPE_ProcessorHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);


    if ( BAPE_Processor_P_DetermineOutputFormat(handle) == BDSP_DataType_eMax )
    {
        BDBG_ERR(("No Valid outputs attached"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    BDBG_MSG(("Adding output stage with BDSP_DataType of %d", BAPE_FMT_P_GetDspDataType_isrsafe(&pConnection->pSource->format)));
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BAPE_FMT_P_GetDspDataType_isrsafe(&pConnection->pSource->format),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_Processor_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        BERR_TRACE(errCode);
        goto err_applysettings;
    }

    return BERR_SUCCESS;

err_applysettings:
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
    return errCode;
}

static BERR_Code BAPE_Processor_P_ApplyKaraokeVocalSettings(BAPE_ProcessorHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_VocalPPConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Applying Karaoke Vocal settings - enabled %d, attenuation %d, delay %d.",
              handle->settings.settings.karaokeVocal.echo.enabled,
              handle->settings.settings.karaokeVocal.echo.attenuation,
              handle->settings.settings.karaokeVocal.echo.delay
              ));

    BAPE_DSP_P_SET_VARIABLE(userConfig, i32EchoEnable, handle->settings.settings.karaokeVocal.echo.enabled?1:0);
    BAPE_DSP_P_SET_VARIABLE(userConfig, i32EchoAttn, (unsigned) ((((int64_t)handle->settings.settings.karaokeVocal.echo.attenuation)<<31) / 100) );
    BAPE_DSP_P_SET_VARIABLE(userConfig, i32EchoDelayInMs, handle->settings.settings.karaokeVocal.echo.delay);

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Processor_P_ApplyFadeSettings(BAPE_ProcessorHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_FadeCtrlConfigParams userConfig;
    unsigned samplerate;
    uint32_t previousLevel;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    previousLevel = userConfig.ui32VolumeTargetLevel;

    samplerate = handle->sampleRate == 0 ? 48000 : handle->sampleRate;

    BDBG_MSG(("Applying fade %s from %x to %x for Processor module %p.",
              (previousLevel > userConfig.ui32VolumeTargetLevel) ? "DOWN" : "UP",
              previousLevel,
              userConfig.ui32VolumeTargetLevel,
              (void *)handle));
    BDBG_MSG(("  type %u, duration %u, new level %u",
              handle->settings.settings.fade.type,
              handle->settings.settings.fade.duration,
              handle->settings.settings.fade.level));

    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EasingFunctionType, handle->settings.settings.fade.type);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32VolumeTargetLevel, (unsigned) ((((int64_t)handle->settings.settings.fade.level)<<31) / 100) );
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32DurationInSamples, handle->settings.settings.fade.duration * samplerate / 1000);

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Processor_P_ApplyAdvancedTsmSettings(BAPE_ProcessorHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_TsmCorrectionConfigParams userConfig;
    unsigned currentTsmMode;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    currentTsmMode = userConfig.ui32TsmCorrectionMode;

    BDBG_MSG(("Applying AdvancedTsm settings for Processor module %p.", (void *)handle));
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32TsmCorrectionMode, (unsigned)BAPE_DSP_P_VALIDATE_VARIABLE_UPPER((unsigned)handle->settings.settings.advTsm.mode, (unsigned)BAPE_AdvancedTsmMode_ePpm));

    if ( currentTsmMode != userConfig.ui32TsmCorrectionMode )
    {
        errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Processor_P_ApplyAmbisonicSettings(BAPE_ProcessorHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_AmbisonicsConfigParams userConfig, curUserConfig;
    bool hasStereoConsumers;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BKNI_Memcpy(&curUserConfig, &userConfig, sizeof(curUserConfig));

    /* we already validated in AllocatePath that we only have one output type */
    hasStereoConsumers = BAPE_Connector_P_GetNumConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]) > 0;

    BDBG_MSG(("Applying Ambisonic settings for Processor module %p.", (void *)handle));
    BDBG_MSG(("  ambisonicSource %u, yaw %u, pitch %u, roll %u",
              handle->settings.settings.ambisonic.ambisonicSource,
              handle->settings.settings.ambisonic.yaw,
              handle->settings.settings.ambisonic.pitch,
              handle->settings.settings.ambisonic.roll));

    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32AmbisonicProcess, handle->settings.settings.ambisonic.ambisonicSource ? 1 : 0);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Yaw, handle->settings.settings.ambisonic.yaw);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Pitch, handle->settings.settings.ambisonic.pitch);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32Roll, handle->settings.settings.ambisonic.roll);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32BinauralRendering, hasStereoConsumers? 1:0);

    if ( BKNI_Memcmp(&curUserConfig, &userConfig, sizeof(curUserConfig)) != 0 )
    {
        errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Processor_P_ApplyDspSettings(BAPE_ProcessorHandle handle)
{
    BERR_Code errCode = BERR_SUCCESS;
    switch ( handle->type )
    {
    case BAPE_PostProcessorType_eKaraokeVocal:
        errCode = BAPE_Processor_P_ApplyKaraokeVocalSettings(handle);
        break;
    case BAPE_PostProcessorType_eFade:
        errCode = BAPE_Processor_P_ApplyFadeSettings(handle);
        break;
    case BAPE_PostProcessorType_eAdvancedTsm:
        errCode = BAPE_Processor_P_ApplyAdvancedTsmSettings(handle);
        break;
    case BAPE_PostProcessorType_eAmbisonic:
        errCode = BAPE_Processor_P_ApplyAmbisonicSettings(handle);
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        break;
    }

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return errCode;
}

static void BAPE_Processor_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_ProcessorHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);

    /* Clear the BDSP status */
    errCode = BDSP_Stage_SetAlgorithm(handle->hStage, handle->bdspAlgo);
    if ( errCode )
    {
        BERR_TRACE(errCode);
    }
}

static void BAPE_Processor_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_Processor_RemoveInput(pNode->pHandle, pConnector);
}

static void BAPE_Processor_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate)
{
    BAPE_ProcessorHandle handle;
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BKNI_ASSERT_ISR_CONTEXT();

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);

    handle->sampleRate = newSampleRate;

    BDBG_MSG(("Input Samplrate is now %u", handle->sampleRate));

    /* propagate the sample rate change to our output connecters */
    for ( i = 0; i < pNode->numConnectors; i++ )
    {
        BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[i], handle->sampleRate);
    }

    /* error checking?? */
}

static BERR_Code BAPE_Processor_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BAPE_ProcessorHandle handle;
    BAPE_FMT_Descriptor outputFormat, curOutputFormat;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pNewFormat);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);

    BAPE_Connector_P_GetFormat(&pNode->connectors[BAPE_ConnectorFormat_eStereo], &curOutputFormat);
    outputFormat = *pNewFormat;
    outputFormat.type = curOutputFormat.type;
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eStereo], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BAPE_Connector_P_GetFormat(&pNode->connectors[BAPE_ConnectorFormat_eMultichannel], &curOutputFormat);
    outputFormat = *pNewFormat;
    outputFormat.type = curOutputFormat.type;
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eMultichannel], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

#else
typedef struct BAPE_Processor
{
    BDBG_OBJECT(BAPE_Processor)
} BAPE_Processor;

void BAPE_Processor_GetDefaultCreateSettings(
    BAPE_ProcessorCreateSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_Processor_Create(
    BAPE_Handle deviceHandle,
    const BAPE_ProcessorCreateSettings *pSettings,
    BAPE_ProcessorHandle *pHandle
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Processor_Destroy(
    BAPE_ProcessorHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_Processor_GetSettings(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_Processor_SetSettings(
    BAPE_ProcessorHandle handle,
    const BAPE_ProcessorSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Processor_GetStatus(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorStatus *pStatus    /* [out] Status */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
}

void BAPE_Processor_GetConnector(
    BAPE_ProcessorHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(format);
    BSTD_UNUSED(pConnector);
}

BERR_Code BAPE_Processor_AddInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_Processor_RemoveInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_Processor_RemoveAllInputs(
    BAPE_ProcessorHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
