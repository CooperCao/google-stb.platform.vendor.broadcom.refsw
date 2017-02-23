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
*   API name: DolbyVolume
*    Specific APIs related to Dolby Volume Audio Processing
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_dolby_volume);

struct NEXUS_DolbyVolume {
    NEXUS_OBJECT(NEXUS_DolbyVolume);
};

void NEXUS_DolbyVolume_GetDefaultSettings(
    NEXUS_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_DolbyVolumeHandle NEXUS_DolbyVolume_Open(
    const NEXUS_DolbyVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_DolbyVolume_P_Finalizer(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DolbyVolume, NEXUS_DolbyVolume_Close); 

void NEXUS_DolbyVolume_GetSettings(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_DolbyVolume_SetSettings(
    NEXUS_DolbyVolumeHandle handle,
    const NEXUS_DolbyVolumeSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioInputHandle NEXUS_DolbyVolume_GetConnector(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_DolbyVolume_AddInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume_RemoveInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume_RemoveAllInputs(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

typedef struct NEXUS_DolbyVolume258
{
    NEXUS_OBJECT(NEXUS_DolbyVolume258);
    NEXUS_AudioInputObject connector;
    NEXUS_DolbyVolume258Settings settings;
    NEXUS_AudioInputHandle input;
    BAPE_DolbyVolumeHandle apeHandle;
    char name[13];   /* DOLBY VOLUME */
} NEXUS_DolbyVolume258;

void NEXUS_DolbyVolume258_GetDefaultSettings(
    NEXUS_DolbyVolume258Settings *pSettings   /* [out] default settings */
    )
{
    BAPE_DolbyVolumeSettings piSettings;
    BDBG_ASSERT(NULL != pSettings);
    BAPE_DolbyVolume_GetDefaultSettings(&piSettings);
    pSettings->enabled = piSettings.enabled;
    pSettings->preGain = piSettings.preGain;
    pSettings->inputReferenceLevel = piSettings.inputReferenceLevel;
    pSettings->outputReferenceLevel = piSettings.outputReferenceLevel;
    pSettings->calibrationOffset = piSettings.calibrationOffset;
    pSettings->reset = piSettings.reset;
    pSettings->volumeModelerEnabled = piSettings.volumeModelerEnabled;
    pSettings->digitalVolumeLevel = piSettings.digitalVolumeLevel;
    pSettings->analogVolumeLevel = piSettings.analogVolumeLevel;
    pSettings->midsideProcessingEnabled = piSettings.midsideProcessingEnabled;
    pSettings->halfModeEnabled = piSettings.halfModeEnabled;
    pSettings->volumeLevelerEnabled = piSettings.volumeLevelerEnabled;
    pSettings->volumeLevelerAmount = piSettings.volumeLevelerAmount;
    pSettings->limiterEnabled = piSettings.limiterEnabled;
}

NEXUS_DolbyVolume258Handle NEXUS_DolbyVolume258_Open(
    const NEXUS_DolbyVolume258Settings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_DolbyVolume258Settings defaults;
    NEXUS_DolbyVolume258Handle handle;
    BAPE_Connector connector;
    BERR_Code errCode;

    if ( NULL == pSettings )
    {
        pSettings = &defaults;
        NEXUS_DolbyVolume258_GetDefaultSettings(&defaults);
    }

    handle = BKNI_Malloc(sizeof(NEXUS_DolbyVolume258));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_DolbyVolume258, handle);

    errCode = BAPE_DolbyVolume_Create(NEXUS_AUDIO_DEVICE_HANDLE, NULL, &handle->apeHandle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ape_handle;
    }

    BKNI_Snprintf(handle->name, sizeof(handle->name), "DOLBY VOLUME");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eDolbyVolume258, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connector, Open);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_eNone;    /* Inherit from parent */
    BAPE_DolbyVolume_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;

    errCode = NEXUS_DolbyVolume258_SetSettings(handle, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_settings;
    }
    
    /* Success */
    return handle;

err_settings:
    BAPE_DolbyVolume_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyVolume258, handle);
err_ape_handle:
    BKNI_Free(handle);
err_malloc:
    return NULL;
}

static void NEXUS_DolbyVolume258_P_Finalizer(
    NEXUS_DolbyVolume258Handle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_DolbyVolume258, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_DolbyVolume_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyVolume258, handle);
    BKNI_Free(handle);
}

static void NEXUS_DolbyVolume258_P_Release(NEXUS_DolbyVolume258Handle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_DolbyVolume258, NEXUS_DolbyVolume258_Close);

void NEXUS_DolbyVolume258_GetSettings(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_DolbyVolume258Settings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_DolbyVolume258_SetSettings(
    NEXUS_DolbyVolume258Handle handle,
    const NEXUS_DolbyVolume258Settings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_DolbyVolumeSettings piSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);

    BAPE_DolbyVolume_GetSettings(handle->apeHandle, &piSettings);
    piSettings.enabled = pSettings->enabled;
    piSettings.preGain = pSettings->preGain;
    piSettings.inputReferenceLevel = pSettings->inputReferenceLevel;
    piSettings.outputReferenceLevel = pSettings->outputReferenceLevel;
    piSettings.calibrationOffset = pSettings->calibrationOffset;
    piSettings.reset = pSettings->reset;
    piSettings.volumeModelerEnabled = pSettings->volumeModelerEnabled;
    piSettings.digitalVolumeLevel = pSettings->digitalVolumeLevel;
    piSettings.analogVolumeLevel = pSettings->analogVolumeLevel;
    piSettings.midsideProcessingEnabled = pSettings->midsideProcessingEnabled;
    piSettings.halfModeEnabled = pSettings->halfModeEnabled;
    piSettings.volumeLevelerEnabled = pSettings->volumeLevelerEnabled;
    piSettings.volumeLevelerAmount = pSettings->volumeLevelerAmount;
    piSettings.limiterEnabled = pSettings->limiterEnabled;

    errCode = BAPE_DolbyVolume_SetSettings(handle->apeHandle, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

NEXUS_AudioInputHandle NEXUS_DolbyVolume258_GetConnector(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    return &handle->connector;
}

NEXUS_Error NEXUS_DolbyVolume258_AddInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    BDBG_ASSERT(NULL != input);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Only one input can be added"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_DolbyVolume_AddInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_DolbyVolume_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DolbyVolume258_RemoveInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume258);
    BDBG_ASSERT(NULL != handle->input);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input not connected"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_DolbyVolume_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
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
