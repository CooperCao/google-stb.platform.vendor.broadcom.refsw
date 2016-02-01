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
*   API name: TruVolume
*    Specific APIs related to DolbyVolume (formerly Volume IQ) Audio Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_dolby_volume);

static BLST_S_HEAD(StageList, NEXUS_DolbyVolume) g_stageList;
static BLST_S_HEAD(StageList258, NEXUS_DolbyVolume258) g_stageList258;

typedef struct NEXUS_DolbyVolume
{
    NEXUS_OBJECT(NEXUS_DolbyVolume);
    BLST_S_ENTRY(NEXUS_DolbyVolume) node;
    NEXUS_AudioInputObject connector;
    NEXUS_DolbyVolumeSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_DolbyVolume;

typedef struct NEXUS_DolbyVolume258
{
    NEXUS_OBJECT(NEXUS_DolbyVolume258);
    BLST_S_ENTRY(NEXUS_DolbyVolume258) node;
    NEXUS_AudioInputObject connector;
    NEXUS_DolbyVolume258Settings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_DolbyVolume258;

void NEXUS_DolbyVolume_GetDefaultSettings(
    NEXUS_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDolbyVolume,
                                           &g_NEXUS_StageSettings);
    pSettings->enabled = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bDolbyVolEna;
    pSettings->blockSize = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iBlockSize;
    pSettings->numBands = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiBands;
    pSettings->inputReferenceLevel = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiInputReferenceLevel;
    pSettings->outputReferenceLevel = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiOutputReferenceLevel;
    pSettings->calibrationOffset = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iCalibration;
    pSettings->volumeModelerEnabled = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bVolModelerEna;
    pSettings->reset = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bResetNowFlag;
    pSettings->digitalVolumeLevel = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iDigitalVolumeLevel;
    pSettings->analogVolumeLevel = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iAnalogVolumeLevel;
    pSettings->volumeLevelerAmount = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiVolLvlAmount;
    pSettings->volumeLevelerEnabled = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bVolLvlEnable;
    pSettings->midsideProcessingEnabled = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bEnableMidsideProc;
    pSettings->halfModeEnabled = g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bHalfmodeEna;
}

NEXUS_DolbyVolumeHandle NEXUS_DolbyVolume_Open(
    const NEXUS_DolbyVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_DolbyVolume *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_DolbyVolume, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eDolbyVolume, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_eNone;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_DolbyVolume_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDolbyVolume,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_DolbyVolume_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_DolbyVolume_SetSettings(pStage, &pStage->settings);

    return pStage;
}

static void NEXUS_DolbyVolume_P_Finalizer(
    NEXUS_DolbyVolumeHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DolbyVolume, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_DolbyVolume, node);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyVolume, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DolbyVolume, NEXUS_DolbyVolume_Close);


void NEXUS_DolbyVolume_GetSettings(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_DolbyVolume_SetSettings(
    NEXUS_DolbyVolumeHandle handle,
    const NEXUS_DolbyVolumeSettings *pSettings
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);

    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bDolbyVolEna = pSettings->enabled;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iBlockSize = pSettings->blockSize;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiBands = pSettings->numBands;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiInputReferenceLevel = pSettings->inputReferenceLevel;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiOutputReferenceLevel = pSettings->outputReferenceLevel;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iCalibration = pSettings->calibrationOffset;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bVolModelerEna = pSettings->volumeModelerEnabled;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bResetNowFlag = pSettings->reset;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iDigitalVolumeLevel = pSettings->digitalVolumeLevel;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.iAnalogVolumeLevel = pSettings->analogVolumeLevel;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.uiVolLvlAmount = pSettings->volumeLevelerAmount;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bVolLvlEnable = pSettings->volumeLevelerEnabled ;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bEnableMidsideProc = pSettings->midsideProcessingEnabled;
    g_NEXUS_StageSettings.uConfigParams.sDolbyVolConfigParams.bHalfmodeEna = pSettings->halfModeEnabled;

    errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

NEXUS_AudioInput NEXUS_DolbyVolume_GetConnector(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    return &handle->connector;
}

NEXUS_Error NEXUS_DolbyVolume_AddInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
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

NEXUS_Error NEXUS_DolbyVolume_RemoveInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
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

NEXUS_Error NEXUS_DolbyVolume_RemoveAllInputs(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);

    if ( handle->input )
    {
        return NEXUS_DolbyVolume_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DolbyVolume_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    BLST_S_INIT(&g_stageList258);
    return BERR_SUCCESS;
}

void NEXUS_DolbyVolume_P_Uninit(void)
{
    NEXUS_DolbyVolume *pNode;
    NEXUS_DolbyVolume258 *pNode258;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_DolbyVolume_Close(pNode);
    }
    while ( NULL != (pNode258 = BLST_S_FIRST(&g_stageList258)) )
    {
        NEXUS_DolbyVolume258_Close(pNode258);
    }
}

BRAP_ProcessingStageHandle NEXUS_DolbyVolume_P_GetStageHandle(NEXUS_DolbyVolumeHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    return handle->stage;
}


void NEXUS_DolbyVolume258_GetDefaultSettings(
    NEXUS_DolbyVolume258Settings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if NEXUS_HAS_MS11_SUPPORT
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDV258,
                                           &g_NEXUS_StageSettings);
    pSettings->enabled = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bDolbyVolEna;
    pSettings->preGain = g_NEXUS_StageSettings.uConfigParams.sDV258Params.iPreGain;
    pSettings->inputReferenceLevel = g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiInputReferenceLevel;
    pSettings->outputReferenceLevel = g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiOutputReferenceLevel;
    pSettings->calibrationOffset = g_NEXUS_StageSettings.uConfigParams.sDV258Params.iCalibrationOffset;
    pSettings->volumeModelerEnabled = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bVolModelerEna;
    pSettings->reset = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bResetNowFlag;
    pSettings->digitalVolumeLevel = g_NEXUS_StageSettings.uConfigParams.sDV258Params.iDigitalVolumeLevel;
    pSettings->analogVolumeLevel = g_NEXUS_StageSettings.uConfigParams.sDV258Params.iAnalogVolumeLevel;
    pSettings->volumeLevelerAmount = g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiVolLvlAmount;
    pSettings->volumeLevelerEnabled = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bVolLvlEnable;
    pSettings->midsideProcessingEnabled = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bEnableMidsideProc;
    pSettings->halfModeEnabled = g_NEXUS_StageSettings.uConfigParams.sDV258Params.bHalfmodeEna;
#endif
}

NEXUS_DolbyVolume258Handle NEXUS_DolbyVolume258_Open(
    const NEXUS_DolbyVolume258Settings *pSettings     /* Pass NULL for default settings */
    )
{
#if NEXUS_HAS_MS11_SUPPORT
    NEXUS_Error errCode;
    NEXUS_DolbyVolume258 *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_DolbyVolume258, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eDolbyVolume258, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_eNone;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_DolbyVolume258_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDV258,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_DolbyVolume258_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList258, pStage, node);

    /* Apply settings */
    (void)NEXUS_DolbyVolume258_SetSettings(pStage, &pStage->settings);

    return pStage;
#else
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(pSettings);
    return NULL;
#endif
}

static void NEXUS_DolbyVolume258_P_Finalizer(
    NEXUS_DolbyVolume258Handle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DolbyVolume258, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
    }

    BLST_S_REMOVE(&g_stageList258, handle, NEXUS_DolbyVolume258, node);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyVolume258, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DolbyVolume258, NEXUS_DolbyVolume258_Close);


void NEXUS_DolbyVolume258_GetSettings(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_DolbyVolume258Settings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_DolbyVolume258_SetSettings(
    NEXUS_DolbyVolume258Handle handle,
    const NEXUS_DolbyVolume258Settings *pSettings
    )
{
#if NEXUS_HAS_MS11_SUPPORT
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);

    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bDolbyVolEna = pSettings->enabled;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.iPreGain = pSettings->preGain;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiInputReferenceLevel = pSettings->inputReferenceLevel;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiOutputReferenceLevel = pSettings->outputReferenceLevel;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.iCalibrationOffset = pSettings->calibrationOffset;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bVolModelerEna = pSettings->volumeModelerEnabled;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bResetNowFlag = pSettings->reset;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.iDigitalVolumeLevel = pSettings->digitalVolumeLevel;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.iAnalogVolumeLevel = pSettings->analogVolumeLevel;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.uiVolLvlAmount = pSettings->volumeLevelerAmount;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bVolLvlEnable = pSettings->volumeLevelerEnabled ;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bEnableMidsideProc = pSettings->midsideProcessingEnabled;
    g_NEXUS_StageSettings.uConfigParams.sDV258Params.bHalfmodeEna = pSettings->halfModeEnabled;

    errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_AudioInput NEXUS_DolbyVolume258_GetConnector(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    return &handle->connector;
}

NEXUS_Error NEXUS_DolbyVolume258_AddInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
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

NEXUS_Error NEXUS_DolbyVolume258_RemoveInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
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

NEXUS_Error NEXUS_DolbyVolume258_RemoveAllInputs(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);

    if ( handle->input )
    {
        return NEXUS_DolbyVolume258_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

BRAP_ProcessingStageHandle NEXUS_DolbyVolume258_P_GetStageHandle(NEXUS_DolbyVolume258Handle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    return handle->stage;
}

