/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*    Specific APIs related to Broadcom AutoVolumeLevel (AVL) Audio Processing
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_auto_volume_level);

typedef struct NEXUS_AutoVolumeLevel
{
    NEXUS_OBJECT(NEXUS_AutoVolumeLevel);
    NEXUS_AudioInputObject connector;
    NEXUS_AutoVolumeLevelSettings settings;
    NEXUS_AudioInput input;
    BAPE_AutoVolumeLevelHandle apeHandle;
    char name[4];   /* AVL */
} NEXUS_AutoVolumeLevel;

void NEXUS_AutoVolumeLevel_GetDefaultSettings(
    NEXUS_AutoVolumeLevelSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_AutoVolumeLevelSettings piSettings;
    BDBG_ASSERT(NULL != pSettings);
    BAPE_AutoVolumeLevel_GetDefaultSettings(&piSettings);
    pSettings->enabled = piSettings.enabled;
    pSettings->loudnessEqualization = piSettings.loudnessEqualization;
    pSettings->target = piSettings.target;
    pSettings->lowerBound = piSettings.lowerBound;
    pSettings->ref = piSettings.ref;
    pSettings->beta = piSettings.beta;
    pSettings->threshold = piSettings.threshold;
    pSettings->dtfPercent = piSettings.dtfPercent;
    pSettings->alpha2 = piSettings.alpha2;
    pSettings->fastGainRamp = piSettings.fastGainRamp;
    pSettings->dtf = piSettings.dtf;
}

NEXUS_AutoVolumeLevelHandle NEXUS_AutoVolumeLevel_Open(
    const NEXUS_AutoVolumeLevelSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AutoVolumeLevelHandle handle;
    BAPE_AutoVolumeLevelSettings defaults;
    BAPE_Connector connector;
    BERR_Code errCode;
    handle = BKNI_Malloc(sizeof(NEXUS_AutoVolumeLevel));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_AutoVolumeLevel, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "AVL");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eAutoVolumeLevel, handle);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_eNone; /* Determined by inputs */
    BAPE_AutoVolumeLevel_GetDefaultSettings(&defaults);
    errCode = BAPE_AutoVolumeLevel_Create(NEXUS_AUDIO_DEVICE_HANDLE, &defaults, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        BDBG_OBJECT_DESTROY(handle, NEXUS_AutoVolumeLevel);
        BKNI_Free(handle);
        return NULL;
    }
    handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    BAPE_AutoVolumeLevel_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;
    if ( NULL == pSettings )
    {
        NEXUS_AutoVolumeLevel_GetDefaultSettings(&handle->settings);
    }
    else
    {
        (void)NEXUS_AutoVolumeLevel_SetSettings(handle, pSettings);
    }

    return handle;
}

static void NEXUS_AutoVolumeLevel_P_Finalizer(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AutoVolumeLevel, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_AutoVolumeLevel_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_AutoVolumeLevel, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AutoVolumeLevel, NEXUS_AutoVolumeLevel_Close);


void NEXUS_AutoVolumeLevel_GetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AutoVolumeLevelSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != pSettings);
    
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(NEXUS_AutoVolumeLevelSettings));
}

NEXUS_Error NEXUS_AutoVolumeLevel_SetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    const NEXUS_AutoVolumeLevelSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_AutoVolumeLevelSettings piSettings;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_AutoVolumeLevel_GetSettings(handle->apeHandle, &piSettings);
    
    piSettings.enabled = pSettings->enabled;
    piSettings.loudnessEqualization = pSettings->loudnessEqualization;
    piSettings.target = pSettings->target;
    piSettings.lowerBound = pSettings->lowerBound;
    piSettings.ref = pSettings->ref;
    piSettings.beta = pSettings->beta;
    piSettings.threshold = pSettings->threshold;
    piSettings.dtfPercent = pSettings->dtfPercent;
    piSettings.alpha2 = pSettings->alpha2;
    piSettings.fastGainRamp = pSettings->fastGainRamp;
    piSettings.dtf = pSettings->dtf;

    errCode = BAPE_AutoVolumeLevel_SetSettings(handle->apeHandle, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(NEXUS_AutoVolumeLevelSettings));
    return BERR_SUCCESS;
}

NEXUS_AudioInput NEXUS_AutoVolumeLevel_GetConnector(
    NEXUS_AutoVolumeLevelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    return &handle->connector;
}

NEXUS_Error NEXUS_AutoVolumeLevel_AddInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != input);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Only one input can be added"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_AutoVolumeLevel_AddInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_AutoVolumeLevel_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AutoVolumeLevel_RemoveInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AutoVolumeLevel);
    BDBG_ASSERT(NULL != handle->input);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input not connected"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_AutoVolumeLevel_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}

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

