/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp_raaga.h"
#endif

BDBG_MODULE(bape_echo_canceller);
BDBG_OBJECT_ID(BAPE_EchoCanceller);

#if BAPE_CHIP_HAS_POST_PROCESSING
/* static BDSP_Algorithm BAPE_EchoCanceller_P_GetType(BAPE_EchoCancellerAlgorithm algorithm); */
static BERR_Code BAPE_EchoCanceller_P_ApplyDspSettings(BAPE_EchoCancellerHandle handle);
static void BAPE_EchoCanceller_P_GetDefaultAlgorithmSettings(BAPE_EchoCancellerHandle handle);
static bool BAPE_EchoCanceller_P_IsRunning(BAPE_EchoCancellerHandle handle);
static void BAPE_EchoCanceller_P_InitInterTaskDescriptors(BAPE_EchoCancellerHandle handle);

static BERR_Code BAPE_EchoCanceller_P_Local_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_EchoCanceller_P_Local_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_EchoCanceller_P_Local_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_EchoCanceller_P_Local_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

static BERR_Code BAPE_EchoCanceller_P_Remote_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_EchoCanceller_P_Remote_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_EchoCanceller_P_Remote_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_EchoCanceller_P_Remote_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

void BAPE_EchoCanceller_GetDefaultSettings(
    BAPE_EchoCancellerSettings *pSettings   /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->algorithm = BAPE_EchoCancellerAlgorithm_eSpeex;
}

BERR_Code BAPE_EchoCanceller_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EchoCancellerSettings *pSettings,
    BAPE_EchoCancellerHandle *pHandle               /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_EchoCancellerHandle handle;
    BAPE_EchoCancellerSettings defaults;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BDSP_StageCreateSettings stageCreateSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( NULL == pSettings )
    {
        BAPE_EchoCanceller_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(BAPE_EchoCanceller));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_EchoCanceller));
    BDBG_OBJECT_SET(handle, BAPE_EchoCanceller);
    handle->deviceHandle = deviceHandle;

    /* Initialize node for local connections.  This is the primary node that can attach to outputs. */
    BAPE_P_InitPathNode(&handle->localNode, BAPE_PathNodeType_eEchoCanceller, pSettings->algorithm, 1, deviceHandle, handle);   
    handle->localNode.pName = "Echo Canceller (local)";

    handle->localNode.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->localNode.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmMono;    
    errCode = BAPE_Connector_P_SetFormat(&handle->localNode.connectors[0], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    /* Generic Routines */
    handle->localNode.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->localNode.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->localNode.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->localNode.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->localNode.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* Echo Canceller Specifics */
    handle->localNode.allocatePathFromInput = BAPE_EchoCanceller_P_Local_AllocatePathFromInput;
    handle->localNode.startPathFromInput = BAPE_EchoCanceller_P_Local_StartPathFromInput;
    handle->localNode.stopPathFromInput = BAPE_EchoCanceller_P_Local_StopPathFromInput;
    handle->localNode.removeInput = BAPE_EchoCanceller_P_Local_RemoveInputCallback;

    /* Initialize node for remote inputs.  This node can not connect downstream, and is an inter-task link between a decoder and the ecno-canceller. */
    BAPE_P_InitPathNode(&handle->remoteNode, BAPE_PathNodeType_eEchoCanceller, pSettings->algorithm, 1, deviceHandle, handle);
    handle->remoteNode.pName = "Echo Canceller (remote)";
    handle->remoteNode.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->remoteNode.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmMono;    
    errCode = BAPE_Connector_P_SetFormat(&handle->remoteNode.connectors[0], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->localNode, &caps);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmMono);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->localNode, &caps);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_caps;
    }

    BAPE_PathNode_P_GetInputCapabilities(&handle->remoteNode, &caps);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmMono);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->remoteNode, &caps);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_set_caps;
    }

    /* No generic routines, we can not connect to outputs */

    /* Echo Canceller Specifics */
    handle->remoteNode.allocatePathFromInput = BAPE_EchoCanceller_P_Remote_AllocatePathFromInput;
    handle->remoteNode.startPathFromInput = BAPE_EchoCanceller_P_Remote_StartPathFromInput;
    handle->remoteNode.stopPathFromInput = BAPE_EchoCanceller_P_Remote_StopPathFromInput;
    handle->remoteNode.removeInput = BAPE_EchoCanceller_P_Remote_RemoveInputCallback;

    /* Init algorithm settings */
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(BAPE_EchoCancellerSettings));
    BAPE_EchoCanceller_P_GetDefaultAlgorithmSettings(handle);

    /* Allocate required buffers */
    errCode = BDSP_InterTaskBuffer_Create(deviceHandle->dspContext, BDSP_DataType_ePcmMono, BDSP_BufferType_eDRAM, &handle->hInterTaskBuffer);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_inter_task_buffer;
    }

    BAPE_EchoCanceller_P_InitInterTaskDescriptors(handle);

    /* Create BDSP stage */
    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioEchoCanceller, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eSpeexAec] = true;
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    handle->localNode.connectors[0].hStage = handle->hStage;    

    *pHandle = handle;
    return BERR_SUCCESS;

err_stage_create:
err_inter_task_buffer:
err_set_caps:
err_connector_format:
    BDBG_OBJECT_DESTROY(handle, BAPE_EchoCanceller);
    BKNI_Free(handle);
err_handle:
    return errCode;
}

void BAPE_EchoCanceller_Destroy(
    BAPE_EchoCancellerHandle handle
    )
{
    bool running;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    running = handle->localStarted || handle->remoteStarted;
    BDBG_ASSERT(false == running);
    BDBG_ASSERT(NULL == handle->localInput);
    BDBG_ASSERT(NULL == handle->remoteInput);

    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDSP_InterTaskBuffer_Destroy(handle->hInterTaskBuffer);
    BDBG_OBJECT_DESTROY(handle, BAPE_EchoCanceller);
    BKNI_Free(handle);
}

void BAPE_EchoCanceller_GetSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_EchoCanceller_SetSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerSettings *pSettings
    )
{
    bool running;

    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);

    running = BAPE_EchoCanceller_P_IsRunning(handle);
    if ( running && pSettings->algorithm != handle->settings.algorithm )
    {
        BDBG_ERR(("Can not change algorithm while running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

void BAPE_EchoCanceller_GetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerAlgorithm algorithm,
    BAPE_EchoCancellerAlgorithmSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);

    pSettings->algorithm = algorithm;
    switch ( algorithm )
    {
    case BAPE_EchoCancellerAlgorithm_eSpeex:
        pSettings->algorithmSettings.speex = handle->speexSettings;
        break;
    default:
        break;
    }
}

BERR_Code BAPE_EchoCanceller_SetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerAlgorithmSettings *pSettings
    )
{
    bool running, updateDsp=false;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);

    running = BAPE_EchoCanceller_P_IsRunning(handle);
    if ( handle->settings.algorithm == pSettings->algorithm )
    {
        updateDsp = true;
    }

    switch ( pSettings->algorithm )
    {
    case BAPE_EchoCancellerAlgorithm_eSpeex:
        handle->speexSettings = pSettings->algorithmSettings.speex;
        break;
    default:
        updateDsp = false;
        break;
    }

    if ( running && updateDsp )
    {
        errCode = BAPE_EchoCanceller_P_ApplyDspSettings(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_EchoCanceller_GetConnector(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector *pConnector   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->localNode.connectors[0];
}

BERR_Code BAPE_EchoCanceller_AddLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( NULL != handle->localInput )
    {
        BDBG_ERR(("Can not have more than one local input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("%s: node %p, input %p", __FUNCTION__, (void *)&handle->localNode, (void *)input));
    errCode = BAPE_PathNode_P_AddInput(&handle->localNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->localInput = input;
    return BERR_SUCCESS;
}

BERR_Code BAPE_EchoCanceller_RemoveLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != handle->localInput )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected as the local input", input->pParent->pName, input->pName, (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("%s: node %p, input %p", __FUNCTION__, (void *)&handle->localNode, (void *)input));
    errCode = BAPE_PathNode_P_RemoveInput(&handle->localNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->localInput = NULL;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Add a remote (far-end) input to the echo canceller
***************************************************************************/
BERR_Code BAPE_EchoCanceller_AddRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( NULL != handle->remoteInput )
    {
        BDBG_ERR(("Can not have more than one remote input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("%s: node %p, input %p", __FUNCTION__, (void *)&handle->remoteNode, (void *)input));
    errCode = BAPE_PathNode_P_AddInput(&handle->remoteNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->remoteInput = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove a remote (far-end) input from this processing stage
***************************************************************************/
BERR_Code BAPE_EchoCanceller_RemoveRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != handle->remoteInput )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected as the remote input", input->pParent->pName, input->pName, (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("%s: node %p, input %p", __FUNCTION__, (void *)&handle->remoteNode, (void *)input));
    errCode = BAPE_PathNode_P_RemoveInput(&handle->remoteNode, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->remoteInput = NULL;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_EchoCanceller_RemoveAllInputs(
    BAPE_EchoCancellerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);
    BDBG_MSG(("%s: localInput %p, remoteInput %p", __FUNCTION__, (void *)handle->localInput, (void *)handle->remoteInput));
    if ( handle->localInput )
    {
        (void)BAPE_EchoCanceller_RemoveLocalInput(handle, handle->localInput);
    }
    if ( handle->remoteInput )
    {
        (void)BAPE_EchoCanceller_RemoveRemoteInput(handle, handle->remoteInput);
    }
    return BERR_SUCCESS;
}

/*
static BDSP_Algorithm BAPE_EchoCanceller_P_GetType(BAPE_EchoCancellerAlgorithm algorithm)
{
    switch ( algorithm )
    {
    case BAPE_EchoCancellerAlgorithm_eSpeex:
        return BDSP_Algorithm_eSpeexAec;
    default:
        return BDSP_Algorithm_eMax;
    }    
}
*/

static BERR_Code BAPE_EchoCanceller_P_ApplySpeexSettings(BAPE_EchoCancellerHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_SpeexAECConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    userConfig.ui32GainResolution = handle->speexSettings.gainMode == BAPE_SpeexEchoCancellerGainMode_eBark?0:1;

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_EchoCanceller_P_ApplyDspSettings(BAPE_EchoCancellerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    switch ( handle->settings.algorithm )
    {
    case BAPE_EchoCancellerAlgorithm_eSpeex:
        return BAPE_EchoCanceller_P_ApplySpeexSettings(handle);
    default:
        return BERR_SUCCESS;
    }
}

static BERR_Code BAPE_EchoCanceller_P_Local_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BERR_Code errCode;
    unsigned input, output;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;

    /* More Sanity */
    BDBG_ASSERT(pNode == &handle->localNode);
    BDBG_ASSERT(pConnection->pSource == handle->localInput);

    /* First, make sure we have both inputs known before attempting to start. */
    if ( NULL == handle->remoteInput )
    {
        BDBG_ERR(("Both the local and remote inputs must be connected to the DSP in order to start an echo canceller."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* When the first input starts, initialize the IO Generic Buffer RD/WR pointers to defaults. */
    if ( false == handle->localStarted && false == handle->remoteStarted )
    {
        BAPE_EchoCanceller_P_InitInterTaskDescriptors(handle);
    }

    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->localInput),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }


    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));
    errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hStage, 
                                                 BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), 
                                                 handle->hInterTaskBuffer, 
                                                 &pConnection->dspInputIndex);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_EchoCanceller_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->dspStageInput = input;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_EchoCanceller_P_Remote_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;

    /* More Sanity */
    BDBG_ASSERT(pNode == &handle->remoteNode);
    BDBG_ASSERT(pConnection->pSource == handle->remoteInput);

    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));

    /* When the first input starts, initialize the IO Generic Buffer RD/WR pointers to defaults. */
    if ( false == handle->localStarted && false == handle->remoteStarted )
    {
        BAPE_EchoCanceller_P_InitInterTaskDescriptors(handle);
    }

    /* Remote inputs connect as an inter-task connection.  The local task will drive the processing. */
    pConnection->hInterTaskBuffer = handle->hInterTaskBuffer;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_EchoCanceller_P_Local_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    BDBG_ASSERT(pNode == &handle->localNode);
    BDBG_ASSERT(pConnection->pSource == handle->localInput);

    /* First, make sure we have both inputs known before attempting to start. */
    if ( NULL == handle->remoteInput )
    {
        BDBG_ERR(("Both the local and remote inputs must be connected to the DSP in order to start an echo canceller."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));
    BDBG_ASSERT(false == handle->localStarted);
    handle->localStarted = true;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_EchoCanceller_P_Remote_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    BDBG_ASSERT(pNode == &handle->remoteNode);
    BDBG_ASSERT(pConnection->pSource == handle->remoteInput);

    /* First, make sure we have both inputs known before attempting to start. */
    if ( NULL == handle->localInput )
    {
        BDBG_ERR(("Both the local and remote inputs must be connected to the DSP in order to start an echo canceller."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));
    BDBG_ASSERT(false == handle->remoteStarted);
    handle->remoteStarted = true;

    return BERR_SUCCESS;
}

static void BAPE_EchoCanceller_P_Local_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    BDBG_ASSERT(pNode == &handle->localNode);
    BDBG_ASSERT(pConnection->pSource == handle->localInput);

    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));
    BDBG_ASSERT(handle->localStarted);
    handle->localStarted = false;

    BDSP_Stage_RemoveAllOutputs(handle->hStage);
    BDSP_Stage_RemoveInput(handle->hStage, handle->dspStageInput);
    BDSP_Stage_RemoveInput(handle->hStage, pConnection->dspInputIndex);
}

static void BAPE_EchoCanceller_P_Remote_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_EchoCancellerHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_EchoCanceller);

    BDBG_ASSERT(pNode == &handle->remoteNode);
    BDBG_ASSERT(pConnection->pSource == handle->remoteInput);

    BDBG_MSG(("%s: node %p, connection %p", __FUNCTION__, (void *)pNode, (void *)pConnection));
    BDBG_ASSERT(handle->remoteStarted);
    handle->remoteStarted = false;
}

static void BAPE_EchoCanceller_P_GetDefaultSpeexSettings(BAPE_EchoCancellerHandle handle)
{
    BDSP_Raaga_Audio_SpeexAECConfigParams userConfig;

    BERR_TRACE(BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eSpeexAec, &userConfig, sizeof(userConfig)));

    handle->speexSettings.gainMode = (userConfig.ui32GainResolution == 0)?BAPE_SpeexEchoCancellerGainMode_eBark:BAPE_SpeexEchoCancellerGainMode_eLinear;
}

static void BAPE_EchoCanceller_P_GetDefaultAlgorithmSettings(BAPE_EchoCancellerHandle handle)
{
    BAPE_EchoCanceller_P_GetDefaultSpeexSettings(handle);
}

static void BAPE_EchoCanceller_P_Local_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_EchoCanceller_RemoveLocalInput(pNode->pHandle, pConnector);
}

static void BAPE_EchoCanceller_P_Remote_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_EchoCanceller_RemoveRemoteInput(pNode->pHandle, pConnector);
}

static bool BAPE_EchoCanceller_P_IsRunning(BAPE_EchoCancellerHandle handle)
{
    return handle->localStarted;
}

static void BAPE_EchoCanceller_P_InitInterTaskDescriptors(BAPE_EchoCancellerHandle handle)
{
    BDBG_MSG(("%s: handle %p", __FUNCTION__, (void *)handle));
    BDSP_InterTaskBuffer_Flush(handle->hInterTaskBuffer);
}
#else
void BAPE_EchoCanceller_GetDefaultSettings(
    BAPE_EchoCancellerSettings *pSettings   /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_EchoCanceller_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EchoCancellerSettings *pSettings,
    BAPE_EchoCancellerHandle *pHandle               /* [out] */
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_EchoCanceller_Destroy(
    BAPE_EchoCancellerHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_EchoCanceller_GetSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_EchoCanceller_SetSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_EchoCanceller_GetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerAlgorithm algorithm,
    BAPE_EchoCancellerAlgorithmSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(algorithm);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_EchoCanceller_SetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerAlgorithmSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_EchoCanceller_GetConnector(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector *pConnector   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnector);
}

BERR_Code BAPE_EchoCanceller_AddLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_EchoCanceller_RemoveLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
        BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_EchoCanceller_AddRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_EchoCanceller_RemoveRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_EchoCanceller_RemoveAllInputs(
    BAPE_EchoCancellerHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
