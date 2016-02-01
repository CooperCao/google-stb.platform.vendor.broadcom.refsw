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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_3d_surround);

#define NEXUS_HAS_3D_SURROUND 1

#if NEXUS_HAS_3D_SURROUND
static BLST_S_HEAD(StageList, NEXUS_3dSurround) g_stageList;
#endif

typedef struct NEXUS_3dSurround
{
    NEXUS_OBJECT(NEXUS_3dSurround);
    BLST_S_ENTRY(NEXUS_3dSurround) node;
    NEXUS_AudioInputObject connector;
    NEXUS_3dSurroundSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_3dSurround;

void NEXUS_3dSurround_GetDefaultSettings(
    NEXUS_3dSurroundSettings *pSettings   /* [out] default settings */
    )
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eBrcm3DSurround,
                                           &g_NEXUS_StageSettings);    
    pSettings->enabled = true;
    pSettings->softLimiterEnabled = g_NEXUS_StageSettings.uConfigParams.sBrcm3DSurroundParams.bSoftLimiterEnable;
#else
    BSTD_UNUSED(pSettings);
#endif
}

NEXUS_3dSurroundHandle NEXUS_3dSurround_Open(
    const NEXUS_3dSurroundSettings *pSettings     /* Pass NULL for default settings */
    )
{
#if NEXUS_HAS_3D_SURROUND
    NEXUS_Error errCode;
    NEXUS_3dSurround *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_3dSurround, pStage);

    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_e3dSurround, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_ePcmStereo;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_3dSurround_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eBrcm3DSurround,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_3dSurround_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_3dSurround_SetSettings(pStage, &pStage->settings);

    return pStage;
#else
    BSTD_UNUSED(pSettings);
    return (NULL);
#endif
}

static void NEXUS_3dSurround_P_Finalizer(
    NEXUS_3dSurroundHandle handle
    )
{
#if NEXUS_HAS_3D_SURROUND
    NEXUS_OBJECT_ASSERT(NEXUS_3dSurround, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_3dSurround, node);
    NEXUS_OBJECT_DESTROY(NEXUS_3dSurround, handle);
    BKNI_Free(handle);
#else
    BSTD_UNUSED(handle);
#endif
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_3dSurround, NEXUS_3dSurround_Close);


void NEXUS_3dSurround_GetSettings(
    NEXUS_3dSurroundHandle handle,
    NEXUS_3dSurroundSettings *pSettings    /* [out] Settings */
    )
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
}


NEXUS_Error NEXUS_3dSurround_SetSettings(
    NEXUS_3dSurroundHandle handle,
    const NEXUS_3dSurroundSettings *pSettings
    )
{
#if NEXUS_HAS_3D_SURROUND
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);

    g_NEXUS_StageSettings.uConfigParams.sBrcm3DSurroundParams.b3DSurroundEnable = handle->settings.enabled;
    g_NEXUS_StageSettings.uConfigParams.sBrcm3DSurroundParams.bSoftLimiterEnable = handle->settings.softLimiterEnabled;

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

NEXUS_AudioInput NEXUS_3dSurround_GetConnector(
    NEXUS_3dSurroundHandle handle
    )
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
    return &handle->connector;
#else
    BSTD_UNUSED(handle);
    return (NULL);
#endif
}

NEXUS_Error NEXUS_3dSurround_AddInput(
    NEXUS_3dSurroundHandle handle,
    NEXUS_AudioInput input
    )
{
#if NEXUS_HAS_3D_SURROUND
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
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

NEXUS_Error NEXUS_3dSurround_RemoveInput(
    NEXUS_3dSurroundHandle handle,
    NEXUS_AudioInput input
    )
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
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

NEXUS_Error NEXUS_3dSurround_RemoveAllInputs(
    NEXUS_3dSurroundHandle handle
    )
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);

    if ( handle->input )
    {
        return NEXUS_3dSurround_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_3dSurround_P_Init(void)
{
#if NEXUS_HAS_3D_SURROUND
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
#else
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void NEXUS_3dSurround_P_Uninit(void)
{
#if NEXUS_HAS_3D_SURROUND
    NEXUS_3dSurround *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_3dSurround_Close(pNode);
    }
#endif
}

BRAP_ProcessingStageHandle NEXUS_3dSurround_P_GetStageHandle(NEXUS_3dSurroundHandle handle)
{
#if NEXUS_HAS_3D_SURROUND
    BDBG_OBJECT_ASSERT(handle, NEXUS_3dSurround);
    return handle->stage;
#else
    BSTD_UNUSED(handle);
    return (NULL);
#endif
}

