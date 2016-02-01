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
*   API name: Ac3Encode
*    Specific APIs related to SRS-XT audio processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_ac3_encode);

static BLST_S_HEAD(StageList, NEXUS_Ac3Encode) g_stageList;

typedef struct NEXUS_Ac3Encode
{
    NEXUS_OBJECT(NEXUS_Ac3Encode);
    BLST_S_ENTRY(NEXUS_Ac3Encode) node;
    NEXUS_AudioInputObject connector;
    NEXUS_Ac3EncodeSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_Ac3Encode;

#if BCHP_CHIP!=7400
#define NEXUS_HAS_AC3_ENCODE 1
#endif

void NEXUS_Ac3Encode_GetDefaultSettings(
    NEXUS_Ac3EncodeSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    /* Taken from PI */
    pSettings->spdifHeaderEnabled = true;
}

NEXUS_Ac3EncodeHandle NEXUS_Ac3Encode_Open(
    const NEXUS_Ac3EncodeSettings *pSettings     /* Pass NULL for default settings */
    )
{
#if NEXUS_HAS_AC3_ENCODE
    NEXUS_Error errCode;
    NEXUS_Ac3Encode *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Ac3Encode, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eAc3Encode, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_eCompressed;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_Ac3Encode_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eEncodeAc3,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_Ac3Encode_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_Ac3Encode_SetSettings(pStage, &pStage->settings);

    return pStage;
#else
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
#endif
}

static void NEXUS_Ac3Encode_P_Finalizer(
    NEXUS_Ac3EncodeHandle handle
    )
{
#if NEXUS_HAS_AC3_ENCODE

    NEXUS_OBJECT_ASSERT(NEXUS_Ac3Encode, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
        BLST_S_REMOVE(&g_stageList, handle, NEXUS_Ac3Encode, node);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Ac3Encode, handle);
    BKNI_Free(handle);
#else
    BSTD_UNUSED(handle);
#endif
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Ac3Encode, NEXUS_Ac3Encode_Close);

void NEXUS_Ac3Encode_GetSettings(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_Ac3EncodeSettings *pSettings    /* [out] Settings */
    )
{
#if NEXUS_HAS_AC3_ENCODE
    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}

NEXUS_Error NEXUS_Ac3Encode_SetSettings(
    NEXUS_Ac3EncodeHandle handle,
    const NEXUS_Ac3EncodeSettings *pSettings
    )
{
#if NEXUS_HAS_AC3_ENCODE
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eEncodeAc3, &g_NEXUS_StageSettings);
    g_NEXUS_StageSettings.uConfigParams.sAc3ENCParams.bSpdifHeaderEnable = handle->settings.spdifHeaderEnabled;
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

NEXUS_AudioInput NEXUS_Ac3Encode_GetConnector(
    NEXUS_Ac3EncodeHandle handle
    )
{
#if NEXUS_HAS_AC3_ENCODE
    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
    return &handle->connector;
#else
    BSTD_UNUSED(handle);
    return NULL;
#endif
}

NEXUS_Error NEXUS_Ac3Encode_AddInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInput input
    )
{
#if NEXUS_HAS_AC3_ENCODE
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
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
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Ac3Encode_RemoveInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInput input
    )
{
#if NEXUS_HAS_AC3_ENCODE
    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
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
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Ac3Encode_RemoveAllInputs(
    NEXUS_Ac3EncodeHandle handle
    )
{
#if NEXUS_HAS_AC3_ENCODE
    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);

    if ( handle->input )
    {
        return NEXUS_Ac3Encode_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Ac3Encode_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
}

void NEXUS_Ac3Encode_P_Uninit(void)
{
    NEXUS_Ac3Encode *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_Ac3Encode_Close(pNode);
    }
}

BRAP_ProcessingStageHandle NEXUS_Ac3Encode_P_GetStageHandle(NEXUS_Ac3EncodeHandle handle, NEXUS_AudioCodec codec)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Ac3Encode);
    BSTD_UNUSED(codec);

    return handle->stage;
}

