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
*   API name: Post Processor
*    Specific APIs related to Post Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_processor);

BDBG_OBJECT_ID(BAPE_Processor);

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

void BAPE_Processor_GetDefaultCreateSettings(
    BAPE_ProcessorCreateSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = BAPE_PostProcessorType_eFade;
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
            BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eVocalPP, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.karaokeVocal.echo.enabled = false;
            pSettings->settings.karaokeVocal.echo.attenuation = (unsigned)((((int64_t)dspSettings.i32EchoAttn)*100) >> 31);
            pSettings->settings.karaokeVocal.echo.delay = dspSettings.i32EchoDelayInMs;
        }
        break;
    case BAPE_PostProcessorType_eFade:
        {
            BDSP_Raaga_Audio_FadeCtrlConfigParams dspSettings;
            BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eFadeCtrl, (void *)&dspSettings, sizeof(dspSettings));
            pSettings->settings.fade.type = dspSettings.ui32EasingFunctionType;
            pSettings->settings.fade.level = 100;
            pSettings->settings.fade.duration = 100;
        }
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", type));
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
        break; /* unreachable */
    }
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
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, handle->type, 1, deviceHandle, handle);
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
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_bad_type;
        break; /* unreachable */
    }

    BAPE_Processor_P_GetDefaultSettings(handle->type, &handle->settings);

    handle->node.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

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
    handle->node.connectors[0].hStage = handle->hStage;

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

void BAPE_Processor_P_GetFadeStatus(
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
        BDBG_MSG(("%s: active %u, remaining %lu, level %lu", __FUNCTION__,
                  (unsigned)pStatus->status.fade.active,
                  (unsigned long)pStatus->status.fade.remaining,
                  (unsigned long)pStatus->status.fade.level));
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
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Processor);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
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

    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
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
