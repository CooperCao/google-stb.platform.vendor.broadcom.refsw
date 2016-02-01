/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: AutoVolumeLevel
*    Specific APIs related to AVL audio processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_auto_volume_level);

static BLST_S_HEAD(StageList, NEXUS_AutoVolumeLevel) g_stageList;

typedef struct NEXUS_AutoVolumeLevel
{
    NEXUS_OBJECT(NEXUS_AutoVolumeLevel);
    BLST_S_ENTRY(NEXUS_AutoVolumeLevel) node;
    NEXUS_AudioInputObject connector;
    NEXUS_AutoVolumeLevelSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_AutoVolumeLevel;

/***************************************************************************
Summary:
    Get default settings for an AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_Open
 ***************************************************************************/
void NEXUS_AutoVolumeLevel_GetDefaultSettings(
    NEXUS_AutoVolumeLevelSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->enabled = true;

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eAvl,
                                           &g_NEXUS_StageSettings);
    pSettings->target = g_NEXUS_StageSettings.uConfigParams.sAvlParams.iTarget;
    pSettings->lowerBound = g_NEXUS_StageSettings.uConfigParams.sAvlParams.iLowerBound;
    pSettings->fixedBoost = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiFixedBoost;
    pSettings->ref = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiRef;
    pSettings->alpha = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiAlpha;
    pSettings->beta = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiBeta;
    pSettings->threshold = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiThreshold;
    pSettings->dtfPercent = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiDtfPcnt;
    pSettings->alpha2 = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiAlpha2;
    pSettings->fastGainRamp = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uisNsfgr;
    pSettings->dtf = g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiDtf;
}

/***************************************************************************
Summary:
    Open a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_Close
 ***************************************************************************/
NEXUS_AutoVolumeLevelHandle NEXUS_AutoVolumeLevel_Open(
    const NEXUS_AutoVolumeLevelSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_AutoVolumeLevel *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_AutoVolumeLevel, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eAutoVolumeLevel, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_eNone;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_AutoVolumeLevel_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eAvl,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_AutoVolumeLevel_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_AutoVolumeLevel_SetSettings(pStage, &pStage->settings);

    return pStage;
}

/***************************************************************************
Summary:
    Close a AVL processing stage
Description:
    Input to the stage must be removed prior to closing.
See Also:
    NEXUS_AutoVolumeLevel_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_AutoVolumeLevel_P_Finalizer(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AutoVolumeLevel, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_AutoVolumeLevel, node);
    NEXUS_OBJECT_DESTROY(NEXUS_AutoVolumeLevel, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AutoVolumeLevel, NEXUS_AutoVolumeLevel_Close);

/***************************************************************************
Summary:
    Get Settings for a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_SetSettings
 ***************************************************************************/
void NEXUS_AutoVolumeLevel_GetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AutoVolumeLevelSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set Settings for a AVL processing stage
See Also:
    NEXUS_AutoVolumeLevel_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_SetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    const NEXUS_AutoVolumeLevelSettings *pSettings
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.bBypass = !handle->settings.enabled;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.iTarget = pSettings->target ;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.iLowerBound = pSettings->lowerBound;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiFixedBoost = pSettings->fixedBoost;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiRef = pSettings->ref;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiAlpha = pSettings->alpha;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiBeta = pSettings->beta;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiThreshold = pSettings->threshold;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiDtfPcnt = pSettings->dtfPercent;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiAlpha2 = pSettings->alpha2;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uisNsfgr = pSettings->fastGainRamp;
    g_NEXUS_StageSettings.uConfigParams.sAvlParams.uiDtf = pSettings->dtf;
    errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for a AVL processing stage
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_AutoVolumeLevel_GetConnector(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    return &handle->connector;
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_RemoveInput
    NEXUS_AutoVolumeLevel_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_AddInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
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
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_AddInput
    NEXUS_AutoVolumeLevel_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
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
    Add an input to this processing stage
See Also:
    NEXUS_AutoVolumeLevel_AddInput
    NEXUS_AutoVolumeLevel_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveAllInputs(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);

    if ( handle->input )
    {
        return NEXUS_AutoVolumeLevel_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AutoVolumeLevel_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
}

void NEXUS_AutoVolumeLevel_P_Uninit(void)
{
    NEXUS_AutoVolumeLevel *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_AutoVolumeLevel_Close(pNode);
    }
}

BRAP_ProcessingStageHandle NEXUS_AutoVolumeLevel_P_GetStageHandle(NEXUS_AutoVolumeLevelHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    return handle->stage;
}

