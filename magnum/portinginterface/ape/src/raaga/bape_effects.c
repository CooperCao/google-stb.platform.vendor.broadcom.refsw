/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: Effects
*    Specific APIs related to Effects Post Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bdsp.h"

BDBG_MODULE(bape_effects);

BDBG_OBJECT_ID(BAPE_Effects);

typedef struct BAPE_Effects
{
    BDBG_OBJECT(BAPE_Effects)
    BAPE_PathNode node;
    BAPE_EffectsSettings settings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
    unsigned sampleRate;
} BAPE_Effects;

static BERR_Code BAPE_Effects_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_Effects_P_ApplyDspSettings(BAPE_EffectsHandle handle);
static void BAPE_Effects_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_Effects_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);
static void BAPE_Effects_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);

void BAPE_Effects_GetDefaultSettings(
    BAPE_EffectsSettings *pSettings   /* [out] default settings */
    )
{
    BDSP_Raaga_Audio_FadeCtrlConfigParams dspSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eFadeCtrl, (void *)&dspSettings, sizeof(dspSettings));

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fade.type = dspSettings.ui32EasingFunctionType;
    pSettings->fade.level = 100;
    pSettings->fade.duration = 100;
}

/***************************************************************************
Summary:
    Open an SRS Effects stage
***************************************************************************/
BERR_Code BAPE_Effects_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EffectsSettings *pSettings,
    BAPE_EffectsHandle *pHandle
    )
{
    BAPE_EffectsHandle handle;
    BAPE_EffectsSettings defaults;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BDSP_StageCreateSettings stageCreateSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( NULL == pSettings )
    {
        pSettings = &defaults;
        BAPE_Effects_GetDefaultSettings(&defaults);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Effects));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_Effects));
    BDBG_OBJECT_SET(handle, BAPE_Effects);
    handle->settings = *pSettings;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eEffects, 1, deviceHandle, handle);
    handle->node.pName = "Effects";

    handle->node.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    /* Effects work with 2.0 or 5.1 source content from the DSP */
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

    /* Effects Specifics */
    handle->node.allocatePathFromInput = BAPE_Effects_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_Effects_P_StopPathFromInput;
    handle->node.removeInput = BAPE_Effects_P_RemoveInputCallback;
    handle->node.inputSampleRateChange_isr = BAPE_Effects_P_InputSampleRateChange_isr;

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eFadeCtrl] = true;
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    handle->node.connectors[0].hStage = handle->hStage;

    *pHandle = handle;
    return BERR_SUCCESS;

err_stage_create:
err_connector_format:
err_caps:
    BAPE_Effects_Destroy(handle);
    return errCode;
}

void BAPE_Effects_Destroy(
    BAPE_EffectsHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_Effects);
    BKNI_Free(handle);
}

void BAPE_Effects_GetSettings(
    BAPE_EffectsHandle handle,
    BAPE_EffectsSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_Effects_SetSettings(
    BAPE_EffectsHandle handle,
    const BAPE_EffectsSettings *pSettings
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_Effects_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


void BAPE_Effects_GetConnector(
    BAPE_EffectsHandle handle,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}


BERR_Code BAPE_Effects_AddInput(
    BAPE_EffectsHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
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


BERR_Code BAPE_Effects_RemoveInput(
    BAPE_EffectsHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
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


BERR_Code BAPE_Effects_RemoveAllInputs(
    BAPE_EffectsHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    if ( handle->input )
    {
        return BAPE_Effects_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_Effects_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned input, output;
    BAPE_EffectsHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_Effects_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Effects_P_ApplyDspSettings(BAPE_EffectsHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_FadeCtrlConfigParams userConfig;
    unsigned samplerate;
    uint32_t previousLevel;

    /* TBD - check fade status here and wait if needed until previous fade completes
    BDBG_WRN(("Previous fade is still in progress, waiting for completion before issuing new fade."));
    */

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    previousLevel = userConfig.ui32VolumeTargetLevel;

    samplerate = handle->sampleRate == 0 ? 48000 : handle->sampleRate;
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32EasingFunctionType, handle->settings.fade.type);
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32VolumeTargetLevel, (unsigned) ((((int64_t)handle->settings.fade.level)<<31) / 100) );
    BAPE_DSP_P_SET_VARIABLE(userConfig, ui32DurationInSamples, handle->settings.fade.duration * samplerate / 1000);

    BDBG_MSG(("Applying fade %s from %x to %x for effects module %p.",
              (previousLevel > userConfig.ui32VolumeTargetLevel) ? "DOWN" : "UP",
              previousLevel,
              userConfig.ui32VolumeTargetLevel,
              (void *)handle));
    BDBG_MSG(("  type %u, duration %u, new level %u",
              handle->settings.fade.type,
              handle->settings.fade.duration,
              handle->settings.fade.level));

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_Effects_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EffectsHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
}

static void BAPE_Effects_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_Effects_RemoveInput(pNode->pHandle, pConnector);
}

static void BAPE_Effects_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate)
{
    BAPE_EffectsHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BKNI_ASSERT_ISR_CONTEXT();

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Effects);

    handle->sampleRate = newSampleRate;

    BDBG_MSG(("Input Samplrate is now %u", handle->sampleRate));

    /* error checking?? */
}
