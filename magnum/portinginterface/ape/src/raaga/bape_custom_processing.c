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
*   API name: CustomProcessing
*    Specific APIs related to custom audio post-processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

#if !B_REFSW_MINIMAL
BDBG_MODULE(bape_custom_processing);

BDBG_OBJECT_ID(BAPE_CustomProcessing);

typedef struct BAPE_CustomProcessing
{
    BDBG_OBJECT(BAPE_CustomProcessing)
    BAPE_PathNode node;
    BAPE_CustomProcessingSettings settings;
    void *pSettings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
} BAPE_CustomProcessing;

static BERR_Code BAPE_CustomProcessing_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static BERR_Code BAPE_CustomProcessing_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_CustomProcessing_P_ApplyDspSettings(BAPE_CustomProcessingHandle handle);
static void BAPE_CustomProcessing_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_CustomProcessing_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

void BAPE_CustomProcessing_GetDefaultSettings(
    BAPE_CustomProcessingSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->algorithm = BDSP_Algorithm_eMax;
    pSettings->outputFormat = BAPE_MultichannelFormat_e2_0;
}

BERR_Code BAPE_CustomProcessing_Create(
    BAPE_Handle deviceHandle,
    const BAPE_CustomProcessingSettings *pSettings,
    BAPE_CustomProcessingHandle *pHandle
    )
{
    BAPE_CustomProcessingHandle handle;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);

    handle = BKNI_Malloc(sizeof(BAPE_CustomProcessing));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_CustomProcessing));
    BDBG_OBJECT_SET(handle, BAPE_CustomProcessing);    
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eCustom, 1, deviceHandle, handle);
    handle->node.pName = "CustomProcessing";

    /* Initialize to stereo.  SetSettings may change this. */
    handle->node.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_format;
    }

    /* We can accept 2.0 input from the DSP */
    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_caps;
    }

    /* Generic Routines */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* CustomProcessing Specifics */
    handle->node.inputFormatChange = BAPE_CustomProcessing_P_InputFormatChange;
    handle->node.allocatePathFromInput = BAPE_CustomProcessing_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_CustomProcessing_P_StopPathFromInput;
    handle->node.removeInput = BAPE_CustomProcessing_P_RemoveInputCallback;

    /* Apply Settings */
    errCode = BAPE_CustomProcessing_SetSettings(handle, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_set_settings;
    }

    *pHandle = handle;
    return BERR_SUCCESS;

err_set_settings:
err_set_caps:
err_set_format:
    BAPE_CustomProcessing_Destroy(handle);
    return errCode;
}


void BAPE_CustomProcessing_Destroy(
    BAPE_CustomProcessingHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);

    if ( handle->hStage != NULL )
    {
        BDSP_Stage_Destroy(handle->hStage);
        handle->hStage = NULL;
    }

    if ( handle->pSettings )
    {
        BKNI_Free(handle->pSettings);
        handle->pSettings = NULL;
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_CustomProcessing);
    BKNI_Free(handle);
}

void BAPE_CustomProcessing_GetSettings(
    BAPE_CustomProcessingHandle handle,
    BAPE_CustomProcessingSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


BERR_Code BAPE_CustomProcessing_SetSettings(
    BAPE_CustomProcessingHandle handle,
    const BAPE_CustomProcessingSettings *pSettings
    )
{
    BERR_Code errCode;
    bool running;
    bool algoChanged;
    bool formatChanged;
    BAPE_FMT_Descriptor format;
    BDSP_StageCreateSettings stageCreateSettings;

    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pSettings);

    /* Check if we are running */
    running = BAPE_PathNode_P_IsActive(&handle->node);
    /* Check if the algorithm is changing */
    algoChanged = (pSettings->algorithm == handle->settings.algorithm) ? false : true;

    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    switch ( pSettings->outputFormat )
    {
    case BAPE_MultichannelFormat_e2_0:
        format.type = BAPE_DataType_ePcmStereo;
        break;
    case BAPE_MultichannelFormat_e5_1:
        format.type = BAPE_DataType_ePcm5_1;
        break;
    case BAPE_MultichannelFormat_e7_1:
        format.type = BAPE_DataType_ePcm7_1;
        break;
    default:
        BDBG_ERR(("Invalid multichannel format"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if format changed */
    formatChanged = BAPE_FMT_P_IsEqual_isrsafe(&format, &handle->node.connectors[0].format);

    /* Only allow some changes if we are stopped */
    if ( running )
    {
        if ( algoChanged )
        {
            BDBG_ERR(("Can not change custom processing algorithm while running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        if ( formatChanged )
        {
            BDBG_ERR(("Can not change output format while running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
  
    if ( formatChanged )
    {
        /* Propagate format change first */
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Check if algorithm changed */
    if ( algoChanged )
    {
        BDSP_AlgorithmInfo algoInfo;

        /* Check if this algorithm is supported */
        errCode = BDSP_GetAlgorithmInfo(handle->node.deviceHandle->dspHandle, pSettings->algorithm, &algoInfo);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if ( algoInfo.type != BDSP_AlgorithmType_eAudioProcessing )
        {
            BDBG_ERR(("Audio processing algorithm %u is not an audio processing algorithm", pSettings->algorithm));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        if ( algoInfo.supported == false )
        {
            BDBG_ERR(("Audio processing algorithm %u is not supported", pSettings->algorithm));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* Release old settings buffer if needed */
        if ( handle->pSettings )
        {
            BKNI_Free(handle->pSettings);
            handle->pSettings = NULL;
        }

        /* Alloc new settings buffer */
        if ( pSettings->algorithmSettingsSize > 0 )
        {
            handle->pSettings = BKNI_Malloc(pSettings->algorithmSettingsSize);
            if ( NULL == handle->pSettings )
            {
                return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            }

            /* Retrieve defaults for this algorithm into our buffer */
            errCode = BDSP_Raaga_GetDefaultAlgorithmSettings(pSettings->algorithm, handle->pSettings, pSettings->algorithmSettingsSize);
            if ( errCode )
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }

        if ( handle->hStage != NULL )
        {
            BDSP_Stage_Destroy(handle->hStage);
            handle->hStage = NULL;
            handle->node.connectors[0].hStage = NULL;
        }

        BDSP_Stage_GetDefaultCreateSettings(handle->node.deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[pSettings->algorithm] = true;
        errCode = BDSP_Stage_Create(handle->node.deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
        if ( errCode )
        {
            handle->hStage = NULL;
            return BERR_TRACE(errCode);
        }
        handle->node.connectors[0].hStage = handle->hStage;    
    }

    /* Save settings */
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(BAPE_CustomProcessingSettings));

    return BERR_SUCCESS;
}

void BAPE_CustomProcessing_GetAlgorithmSettings(
    BAPE_CustomProcessingHandle handle,
    void *pSettings,        /* [out] Should be defined as the correct data type for this custom algorithm */
    size_t settingsSize     /* Size of the settings structure in bytes */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(settingsSize > 0);
    BDBG_ASSERT(settingsSize <= handle->settings.algorithmSettingsSize);
    if ( NULL != handle->pSettings )
    {
        BKNI_Memcpy(pSettings, handle->pSettings, settingsSize);
    }
    else
    {
        BKNI_Memset(pSettings, 0, settingsSize);
    }
}

BERR_Code BAPE_CustomProcessing_SetAlgorithmSettings(
    BAPE_CustomProcessingHandle handle,
    const void *pSettings,  /* Should be defined as the correct data type for this custom algorithm */
    size_t settingsSize     /* Size of the settings structure in bytes */
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(settingsSize > 0);
    if ( settingsSize > handle->settings.algorithmSettingsSize )
    {
        BDBG_ERR(("Custom algorithm size is greater than specified in BAPE_CustomProcessingSettings.algorithmSettingsSize"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( NULL != handle->pSettings )
    {
        BKNI_Memcpy(handle->pSettings, pSettings, settingsSize);
    }
    errCode = BAPE_CustomProcessing_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_CustomProcessing_GetConnector(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}


BERR_Code BAPE_CustomProcessing_AddInput(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
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

BERR_Code BAPE_CustomProcessing_RemoveInput(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected", input->pParent->pName, input->pName, (void* )input));
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

BERR_Code BAPE_CustomProcessing_RemoveAllInputs(
    BAPE_CustomProcessingHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    if ( handle->input )
    {
        return BAPE_CustomProcessing_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

BERR_Code BAPE_CustomProcessing_GetAlgorithmStatus(
    BAPE_CustomProcessingHandle handle,
    void *pStatus,      /* [out] Should be defined as the correct data type for this custom algorithm */
    size_t statusSize   /* Size of the status structure in bytes */
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(statusSize > 0);
    errCode = BDSP_Stage_GetStatus(handle->hStage, pStatus, statusSize);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


static BERR_Code BAPE_CustomProcessing_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BAPE_FMT_Descriptor outputFormat;
    BAPE_DataType dataType;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pNewFormat);

    /* Set our output format but pass all other fields through */
    BAPE_Connector_P_GetFormat(&pNode->connectors[0], &outputFormat);
    dataType = outputFormat.type;
    outputFormat = *pNewFormat;
    outputFormat.type = dataType;
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[0], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_CustomProcessing_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_CustomProcessingHandle handle;
    unsigned input, output;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage, 
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_CustomProcessing_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_CustomProcessing_P_ApplyDspSettings(BAPE_CustomProcessingHandle handle)
{
    BERR_Code errCode;

    if ( NULL == handle->pSettings )
    {
        /* NULL means just use the defaults */
        return BERR_SUCCESS;
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, handle->pSettings, handle->settings.algorithmSettingsSize);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_CustomProcessing_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_CustomProcessingHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_CustomProcessing);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
}

static void BAPE_CustomProcessing_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_CustomProcessing_RemoveInput(pNode->pHandle, pConnector);
}
#endif
