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
*   API name: DolbyVolume
*    Specific APIs related to Dolby Volume Audio Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp.h"
#endif

BDBG_MODULE(bape_dolby_volume);

BDBG_OBJECT_ID(BAPE_DolbyVolume);

#if BAPE_CHIP_HAS_POST_PROCESSING
typedef struct BAPE_DolbyVolume
{
    BDBG_OBJECT(BAPE_DolbyVolume)
    BAPE_Handle deviceHandle;
    BAPE_PathNode node;
    BAPE_DolbyVolumeSettings settings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
} BAPE_DolbyVolume;

static BERR_Code BAPE_DolbyVolume_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DolbyVolume_P_ApplyDspSettings(BAPE_DolbyVolumeHandle handle);
static void BAPE_DolbyVolume_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_DolbyVolume_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

void BAPE_DolbyVolume_GetDefaultSettings(
    BAPE_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BDSP_Raaga_Audio_DV258ConfigParams dspSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDv258, (void *)&dspSettings, sizeof(dspSettings));

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->enabled = (dspSettings.i32DolbyVolumeEnable == 0) ? false : true;
    pSettings->preGain = dspSettings.i32Pregain;
    pSettings->inputReferenceLevel = dspSettings.i32InputReferenceLevel;
    pSettings->outputReferenceLevel = dspSettings.i32OutputReferenceLevel;
    pSettings->calibrationOffset = dspSettings.i32CalibrationOffset;
    pSettings->reset = false;
    pSettings->volumeModelerEnabled = (dspSettings.i32VlmMdlEnable == 0) ? false : true;
    pSettings->digitalVolumeLevel = dspSettings.i32DigitalVolumeLevel;
    pSettings->analogVolumeLevel = dspSettings.i32AnalogVolumeLevel;
    pSettings->volumeLevelerEnabled = (dspSettings.i32LvlEnable == 0) ? false : true;
    pSettings->midsideProcessingEnabled = (dspSettings.i32EnableMidsideProc == 0) ? false : true;
    pSettings->halfModeEnabled = (dspSettings.i32HalfmodeFlag == 0) ? false : true;
    pSettings->volumeLevelerAmount = dspSettings.i32LvlAmount;
    pSettings->limiterEnabled = (dspSettings.i32LimiterEnable == 0) ? false : true;
}

/***************************************************************************
Summary:
    Open an SRS DolbyVolume stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyVolumeSettings *pSettings,
    BAPE_DolbyVolumeHandle *pHandle
    )
{
    BAPE_DolbyVolumeHandle handle;
    BAPE_DolbyVolumeSettings defaults;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    if ( NULL == pSettings )
    {
        pSettings = &defaults;
        BAPE_DolbyVolume_GetDefaultSettings(&defaults);
    }

    handle = BKNI_Malloc(sizeof(BAPE_DolbyVolume));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_DolbyVolume));
    BDBG_OBJECT_SET(handle, BAPE_DolbyVolume);
    handle->deviceHandle = deviceHandle;
    handle->settings = *pSettings;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDolbyVolume, 1, handle->deviceHandle, handle);
    handle->node.pName = "Dolby Volume";

    handle->node.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    /* DV258 works with 2.0 or 5.1 source content from the DSP */
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

    /* DolbyVolume Specifics */
    handle->node.allocatePathFromInput = BAPE_DolbyVolume_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_DolbyVolume_P_StopPathFromInput;
    handle->node.removeInput = BAPE_DolbyVolume_P_RemoveInputCallback;

    *pHandle = handle;
    return BERR_SUCCESS;

err_connector_format:
err_caps:
    BAPE_DolbyVolume_Destroy(handle);
    return errCode;
}

void BAPE_DolbyVolume_Destroy(
    BAPE_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
        handle->hStage = NULL;
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_DolbyVolume);
    BKNI_Free(handle);
}

void BAPE_DolbyVolume_GetSettings(
    BAPE_DolbyVolumeHandle handle,
    BAPE_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_DolbyVolume_SetSettings(
    BAPE_DolbyVolumeHandle handle,
    const BAPE_DolbyVolumeSettings *pSettings
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_DolbyVolume_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


void BAPE_DolbyVolume_GetConnector(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}


BERR_Code BAPE_DolbyVolume_AddInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
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


BERR_Code BAPE_DolbyVolume_RemoveInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
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


BERR_Code BAPE_DolbyVolume_RemoveAllInputs(
    BAPE_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    if ( handle->input )
    {
        return BAPE_DolbyVolume_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_DolbyVolume_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned input, output;
    BAPE_DolbyVolumeHandle handle;
    BDSP_StageCreateSettings stageCreateSettings;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);

    /* inherit the device context from the parent if possible */
    if ( pConnection->pSource->pParent->deviceContext != NULL )
    {
        pNode->deviceContext = pConnection->pSource->pParent->deviceContext;
    }
    else
    {
        if ( handle->deviceHandle->dspContext != NULL )
        {
            pNode->deviceContext = handle->deviceHandle->dspContext;
        }
        else if ( handle->deviceHandle->armContext != NULL )
        {
            pNode->deviceContext = handle->deviceHandle->armContext;
        }
        else
        {
            BDBG_ERR(("No available device context"));
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }
    }

    BDSP_Stage_GetDefaultCreateSettings(pNode->deviceContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eDv258] = true;
    errCode = BDSP_Stage_Create(pNode->deviceContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->node.connectors[0].hStage = handle->hStage;

    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage, 
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_DolbyVolume_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_DolbyVolume_P_ApplyDspSettings(BAPE_DolbyVolumeHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_DV258ConfigParams userConfig;

    if ( handle->hStage )
    {
        errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        BAPE_DSP_P_SET_VARIABLE(userConfig, i32DolbyVolumeEnable, handle->settings.enabled?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32Pregain, handle->settings.preGain);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32InputReferenceLevel, handle->settings.inputReferenceLevel);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32OutputReferenceLevel, handle->settings.outputReferenceLevel);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32CalibrationOffset, handle->settings.calibrationOffset);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32ResetNowFlag, handle->settings.reset?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32VlmMdlEnable, handle->settings.volumeModelerEnabled?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32DigitalVolumeLevel, handle->settings.digitalVolumeLevel);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32AnalogVolumeLevel, handle->settings.analogVolumeLevel);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32EnableMidsideProc, handle->settings.midsideProcessingEnabled?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32HalfmodeFlag, handle->settings.halfModeEnabled?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32LvlEnable, handle->settings.volumeLevelerEnabled?1:0);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32LvlAmount, handle->settings.volumeLevelerAmount);
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32LimiterEnable, handle->settings.limiterEnabled?1:0);

        errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static void BAPE_DolbyVolume_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_DolbyVolumeHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyVolume);
    if ( handle->hStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hStage);
        BDSP_Stage_RemoveAllOutputs(handle->hStage);
        BDSP_Stage_Destroy(handle->hStage);
        handle->hStage = NULL;
        handle->node.connectors[0].hStage = NULL;
        handle->node.deviceContext = NULL;
    }
}

static void BAPE_DolbyVolume_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_DolbyVolume_RemoveInput(pNode->pHandle, pConnector);
}
#else
typedef struct BAPE_DolbyVolume
{
    BDBG_OBJECT(BAPE_DolbyVolume)
} BAPE_DolbyVolume;

void BAPE_DolbyVolume_GetDefaultSettings(
    BAPE_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open an SRS DolbyVolume stage
***************************************************************************/
BERR_Code BAPE_DolbyVolume_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyVolumeSettings *pSettings,
    BAPE_DolbyVolumeHandle *pHandle
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_DolbyVolume_Destroy(
    BAPE_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_DolbyVolume_GetSettings(
    BAPE_DolbyVolumeHandle handle,
    BAPE_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_DolbyVolume_SetSettings(
    BAPE_DolbyVolumeHandle handle,
    const BAPE_DolbyVolumeSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


void BAPE_DolbyVolume_GetConnector(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector *pConnector
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnector);
}


BERR_Code BAPE_DolbyVolume_AddInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_DolbyVolume_RemoveInput(
    BAPE_DolbyVolumeHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_DolbyVolume_RemoveAllInputs(
    BAPE_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
