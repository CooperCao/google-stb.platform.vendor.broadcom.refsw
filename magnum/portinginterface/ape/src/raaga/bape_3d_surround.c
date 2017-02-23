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
*   API name: 3dSurround
*    Specific APIs related to Broadcom 3D Surround Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp_raaga.h"
#endif

BDBG_MODULE(bape_3d_surround);

BDBG_OBJECT_ID(BAPE_3dSurround);

#if BAPE_CHIP_HAS_POST_PROCESSING
typedef struct BAPE_3dSurround
{
    BDBG_OBJECT(BAPE_3dSurround)
    BAPE_PathNode node;
    BAPE_3dSurroundSettings settings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
} BAPE_3dSurround;

static BERR_Code BAPE_3dSurround_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static BERR_Code BAPE_3dSurround_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_3dSurround_P_ApplyDspSettings(BAPE_3dSurroundHandle handle);
static void BAPE_3dSurround_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_3dSurround_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);


void BAPE_3dSurround_GetDefaultSettings(
    BAPE_3dSurroundSettings *pSettings   /* [out] default settings */
    )
{
    BDSP_Raaga_Audio_Brcm3DSurroundConfigParams dspSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eBrcm3DSurround, (void *)&dspSettings, sizeof(dspSettings));

    pSettings->enabled = (dspSettings.i32BRCM3DSurroundEnableFlag == 0) ? false : true;
    BDBG_CASSERT((int)BDSP_Raaga_Audio_eBroadcom3DSurroundMode_LAST == (int)BAPE_3dSurroundSpeakerPosition_eMax);
    switch ( dspSettings.eBroadcom3DSurroundMode )
    {
    default:
    case BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Centre:
        pSettings->speakerPosition = BAPE_3dSurroundSpeakerPosition_eCenter;
        break;
    case BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Wide:
        pSettings->speakerPosition = BAPE_3dSurroundSpeakerPosition_eWide;
        break;
    case BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Extrawide:
        pSettings->speakerPosition = BAPE_3dSurroundSpeakerPosition_eExtraWide;
        break;
    }
}

/***************************************************************************
Summary:
    Open an SRS 3dSurround stage
***************************************************************************/
BERR_Code BAPE_3dSurround_Create(
    BAPE_Handle deviceHandle,
    const BAPE_3dSurroundSettings *pSettings,
    BAPE_3dSurroundHandle *pHandle
    )
{
    BAPE_3dSurroundHandle handle;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BDSP_StageCreateSettings stageCreateSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);


    handle = BKNI_Malloc(sizeof(BAPE_3dSurround));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_3dSurround));
    BDBG_OBJECT_SET(handle, BAPE_3dSurround);
    handle->settings = *pSettings;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_e3dSurround, 1, deviceHandle, handle);
    handle->node.pName = "AVL";

    /* Output format is fixed to PCM stereo */
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_format;
    }
    handle->node.connectors[0].useBufferPool = true;

    /* We can accept either 2.0 or 5.1 input from the DSP */
    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
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

    /* 3dSurround Specifics */
    handle->node.allocatePathFromInput = BAPE_3dSurround_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_3dSurround_P_StopPathFromInput;
    handle->node.removeInput = BAPE_3dSurround_P_RemoveInputCallback;
    handle->node.inputFormatChange = BAPE_3dSurround_P_InputFormatChange;

    /* Create Stage Handle */
    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eBrcm3DSurround] = true;
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
err_set_caps:
err_set_format:
    BAPE_3dSurround_Destroy(handle);
    return errCode;    
}

void BAPE_3dSurround_Destroy(
    BAPE_3dSurroundHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[0]);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_3dSurround);
    BKNI_Free(handle);
}

void BAPE_3dSurround_GetSettings(
    BAPE_3dSurroundHandle handle,
    BAPE_3dSurroundSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_3dSurround_SetSettings(
    BAPE_3dSurroundHandle handle,
    const BAPE_3dSurroundSettings *pSettings
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_3dSurround_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


void BAPE_3dSurround_GetConnector(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}


BERR_Code BAPE_3dSurround_AddInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
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


BERR_Code BAPE_3dSurround_RemoveInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
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


BERR_Code BAPE_3dSurround_RemoveAllInputs(
    BAPE_3dSurroundHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    if ( handle->input )
    {
        return BAPE_3dSurround_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_3dSurround_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BAPE_FMT_Descriptor outputFormat;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pNewFormat);

    /* This module will convert the input to stereo.  All other fields are passthrough. */
    outputFormat = *pNewFormat;
    outputFormat.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[0], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_3dSurround_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_3dSurroundHandle handle;
    unsigned output, input;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage, 
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_3dSurround_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_3dSurround_P_ApplyDspSettings(BAPE_3dSurroundHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_Brcm3DSurroundConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_DSP_P_SET_VARIABLE(userConfig, i32BRCM3DSurroundEnableFlag, handle->settings.enabled?1:0);
    switch ( handle->settings.speakerPosition )
    {
    default:
    case BAPE_3dSurroundSpeakerPosition_eCenter:
        BAPE_DSP_P_SET_VARIABLE(userConfig, eBroadcom3DSurroundMode, BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Centre);
        break;
    case BAPE_3dSurroundSpeakerPosition_eWide:
        BAPE_DSP_P_SET_VARIABLE(userConfig, eBroadcom3DSurroundMode, BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Wide);
        break;
    case BAPE_3dSurroundSpeakerPosition_eExtraWide:
        BAPE_DSP_P_SET_VARIABLE(userConfig, eBroadcom3DSurroundMode, BDSP_Raaga_Audio_eBroadcom3DSurroundMode_Extrawide);
        break;
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_3dSurround_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_3dSurroundHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_3dSurround);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
}

static void BAPE_3dSurround_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_3dSurround_RemoveInput(pNode->pHandle, pConnector);
}

#else
typedef struct BAPE_3dSurround
{
    BDBG_OBJECT(BAPE_3dSurround)
} BAPE_3dSurround;

void BAPE_3dSurround_GetDefaultSettings(
    BAPE_3dSurroundSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_3dSurround_Create(
    BAPE_Handle deviceHandle,
    const BAPE_3dSurroundSettings *pSettings,
    BAPE_3dSurroundHandle *pHandle
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_3dSurround_Destroy(
    BAPE_3dSurroundHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_3dSurround_GetSettings(
    BAPE_3dSurroundHandle handle,
    BAPE_3dSurroundSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_3dSurround_SetSettings(
    BAPE_3dSurroundHandle handle,
    const BAPE_3dSurroundSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


void BAPE_3dSurround_GetConnector(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector *pConnector
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnector);
}


BERR_Code BAPE_3dSurround_AddInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_3dSurround_RemoveInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_3dSurround_RemoveAllInputs(
    BAPE_3dSurroundHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
