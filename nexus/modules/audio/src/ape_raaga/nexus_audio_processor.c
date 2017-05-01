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
*   API name: AudioProcessor
*    Specific APIs related to Audio Post Processing
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_processor);

typedef struct NEXUS_AudioProcessor
{
    NEXUS_OBJECT(NEXUS_AudioProcessor);
    NEXUS_AudioInputObject connectors[NEXUS_AudioConnectorType_eMax];
    NEXUS_AudioPostProcessing type;
    NEXUS_AudioProcessorSettings settings;
    NEXUS_AudioInputHandle input;
    void* apeHandle;
    char name[50];   /* specific to type - up to 50 characters */
} NEXUS_AudioProcessor;

void NEXUS_AudioProcessor_GetDefaultOpenSettings(
    NEXUS_AudioProcessorOpenSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_ProcessorCreateSettings piSettings;
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BAPE_Processor_GetDefaultCreateSettings(&piSettings);
}

static NEXUS_Error NEXUS_AudioProcessor_P_GetPiSettings(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_ProcessorSettings piSettings;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_Processor_GetSettings((BAPE_ProcessorHandle)handle->apeHandle, &piSettings);

    switch ( handle->type )
    {
    case NEXUS_AudioPostProcessing_eKaraokeVocal:
        pSettings->settings.karaokeVocal.echo.enabled = piSettings.settings.karaokeVocal.echo.enabled;
        pSettings->settings.karaokeVocal.echo.attenuation = piSettings.settings.karaokeVocal.echo.attenuation;
        pSettings->settings.karaokeVocal.echo.delay = piSettings.settings.karaokeVocal.echo.delay;
    case NEXUS_AudioPostProcessing_eFade:
        pSettings->settings.fade.type = piSettings.settings.fade.type;
        pSettings->settings.fade.duration = piSettings.settings.fade.duration;
        pSettings->settings.fade.level = piSettings.settings.fade.level;
        break;
    case NEXUS_AudioPostProcessing_eAdvancedTsm:
        pSettings->settings.advancedTsm.mode = piSettings.settings.advTsm.mode;
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        errCode = BERR_NOT_SUPPORTED;
        break;
    }
    pSettings->type = handle->type;

    return BERR_TRACE(errCode);
}

static bool NEXUS_AudioProcessor_P_SupportsMultichannelOutput(NEXUS_AudioPostProcessing type)
{
    switch ( type )
    {
    default:
    case NEXUS_AudioPostProcessing_eKaraokeVocal:
    case NEXUS_AudioPostProcessing_eFade:
        break;
    case NEXUS_AudioPostProcessing_eAdvancedTsm:
        return true;
        break; /* unreachable */
    }
    return false;
}

NEXUS_AudioProcessorHandle NEXUS_AudioProcessor_Open(
    const NEXUS_AudioProcessorOpenSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AudioProcessorOpenSettings defaults;
    NEXUS_AudioProcessorHandle handle;
    BAPE_ProcessorHandle piHandle;
    BAPE_ProcessorCreateSettings piSettings;
    BAPE_Connector connector;
    BERR_Code errCode;
    char * name;

    if ( NULL == pSettings )
    {
        NEXUS_AudioProcessor_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioProcessor));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_AudioProcessor, handle);

    BAPE_Processor_GetDefaultCreateSettings(&piSettings);
    switch ( pSettings->type )
    {
    case NEXUS_AudioPostProcessing_eKaraokeVocal:
        name = "KARAOKEVOCAL";
        piSettings.type = BAPE_PostProcessorType_eKaraokeVocal;
        break;
    case NEXUS_AudioPostProcessing_eFade:
        name = "FADE";
        piSettings.type = BAPE_PostProcessorType_eFade;
        break;
    case NEXUS_AudioPostProcessing_eAdvancedTsm:
        name = "ADVANCEDTSM";
        piSettings.type = BAPE_PostProcessorType_eAdvancedTsm;
        break;
    default:
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", pSettings->type));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_ape_create;
        break; /* unreachable */
    }

    errCode = BAPE_Processor_Create(NEXUS_AUDIO_DEVICE_HANDLE, &piSettings, &piHandle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ape_handle;
    }
    handle->apeHandle = (void*) piHandle;
    handle->type = pSettings->type;
    errCode = NEXUS_AudioProcessor_P_GetPiSettings(handle, &handle->settings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ape_create;
    }

    BKNI_Snprintf(handle->name, sizeof(handle->name), name);
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eStereo], NEXUS_AudioInputType_eAudioProcessor, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eStereo], Open);
    handle->connectors[NEXUS_AudioConnectorType_eStereo].pName = handle->name;
    handle->connectors[NEXUS_AudioConnectorType_eStereo].format = NEXUS_AudioInputFormat_eNone;    /* Inherit from parent */
    BAPE_Processor_GetConnector((BAPE_ProcessorHandle)handle->apeHandle, BAPE_ConnectorFormat_eStereo, &connector);
    handle->connectors[NEXUS_AudioConnectorType_eStereo].port = (size_t)connector;
    if ( NEXUS_AudioProcessor_P_SupportsMultichannelOutput(pSettings->type) )
    {
        NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], NEXUS_AudioInputType_eAudioProcessor, handle);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eMultichannel], Open);
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].pName = handle->name;
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_eNone;    /* Inherit from parent */
        BAPE_Processor_GetConnector((BAPE_ProcessorHandle)handle->apeHandle, BAPE_ConnectorFormat_eMultichannel, &connector);
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port = (size_t)connector;
    }

    /*
    errCode = NEXUS_AudioProcessor_SetSettings(handle, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_settings;
    }
    */

    /* Success */
    return handle;

err_ape_create:
    BAPE_Processor_Destroy((BAPE_ProcessorHandle)handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioProcessor, handle);
err_ape_handle:
    BKNI_Free(handle);
err_malloc:
    return NULL;
}

static void NEXUS_AudioProcessor_P_Finalizer(
    NEXUS_AudioProcessorHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioProcessor, handle);
    NEXUS_AudioInput_Shutdown(&handle->connectors[NEXUS_AudioConnectorType_eStereo]);
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port )
    {
        NEXUS_AudioInput_Shutdown(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel]);
    }
    BAPE_Processor_Destroy((BAPE_ProcessorHandle)handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioProcessor, handle);
    BKNI_Free(handle);
}

static void NEXUS_AudioProcessor_P_Release(NEXUS_AudioProcessorHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eStereo], Close);
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port )
    {
        NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eMultichannel], Close);
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioProcessor, NEXUS_AudioProcessor_Close);

void NEXUS_AudioProcessor_GetSettings(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    BDBG_ASSERT(pSettings != NULL);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_AudioProcessor_SetSettings(
    NEXUS_AudioProcessorHandle handle,
    const NEXUS_AudioProcessorSettings *pSettings
    )
{
    BAPE_ProcessorSettings piSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);

    BAPE_Processor_GetSettings((BAPE_ProcessorHandle)handle->apeHandle, &piSettings);
    switch ( handle->type )
    {
    case NEXUS_AudioPostProcessing_eKaraokeVocal:
        piSettings.settings.karaokeVocal.echo.enabled = pSettings->settings.karaokeVocal.echo.enabled;
        piSettings.settings.karaokeVocal.echo.attenuation = pSettings->settings.karaokeVocal.echo.attenuation;
        piSettings.settings.karaokeVocal.echo.delay = pSettings->settings.karaokeVocal.echo.delay;
    case NEXUS_AudioPostProcessing_eFade:
        piSettings.settings.fade.type = pSettings->settings.fade.type;
        piSettings.settings.fade.duration = pSettings->settings.fade.duration;
        piSettings.settings.fade.level = pSettings->settings.fade.level;
        break;
    case NEXUS_AudioPostProcessing_eAdvancedTsm:
        piSettings.settings.advTsm.mode = pSettings->settings.advancedTsm.mode;
        break;
    default:
        /* it should be impossible to get here - would indicate a bug in _Open() */
        BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
        break; /* unreachable */
    }
    errCode = BAPE_Processor_SetSettings((BAPE_ProcessorHandle)handle->apeHandle, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    /* make sure the app never changes the type */
    handle->settings.type = handle->type;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Get Status for a AudioProcessor stage
***************************************************************************/
void NEXUS_AudioProcessor_GetStatus(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorStatus *pStatus    /* [out] Status */
    )
{
    BAPE_ProcessorStatus piStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    BDBG_ASSERT(pStatus != NULL);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->type = NEXUS_AudioPostProcessing_eMax;
    BAPE_Processor_GetStatus((BAPE_ProcessorHandle)handle->apeHandle, &piStatus);
    if ( piStatus.valid )
    {
        pStatus->type = handle->type;
        switch ( handle->type )
        {
        case NEXUS_AudioPostProcessing_eKaraokeVocal:
            break;
        case NEXUS_AudioPostProcessing_eAdvancedTsm:
            BDBG_CASSERT((int)NEXUS_AudioAdvancedTsmMode_eMax == (int)BAPE_AdvancedTsmMode_eMax);
            pStatus->status.advancedTsm.mode = (NEXUS_AudioAdvancedTsmMode)piStatus.status.advTsm.mode;
            if ( piStatus.status.advTsm.ptsValid )
            {
                pStatus->status.advancedTsm.pts = piStatus.status.advTsm.pts;
                switch ( piStatus.status.advTsm.ptsType )
                {
                default:
                case BAPE_PtsType_eOriginal:
                    pStatus->status.advancedTsm.ptsType = NEXUS_PtsType_eCoded;
                    break;
                case BAPE_PtsType_eInterpolated:
                    pStatus->status.advancedTsm.ptsType = NEXUS_PtsType_eInterpolatedFromValidPTS;
                    break;
                case BAPE_PtsType_eInterpolatedFromInvalid:
                    pStatus->status.advancedTsm.ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
                    break;
                }
            }
            else
            {
                pStatus->status.advancedTsm.pts = 0;
                pStatus->status.advancedTsm.ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
            }
            pStatus->status.advancedTsm.correction = piStatus.status.advTsm.correction;
            break;
        case NEXUS_AudioPostProcessing_eFade:
            pStatus->status.fade.active = piStatus.status.fade.active;
            pStatus->status.fade.level = piStatus.status.fade.level;
            pStatus->status.fade.remaining = piStatus.status.fade.remaining;
            break;
        default:
            /* it should be impossible to get here - would indicate a bug in _Open() */
            BDBG_ERR(("type %d is not currently supported by NEXUS_AudioProcessor", handle->type));
            BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
        }
    }
}

NEXUS_AudioInputHandle NEXUS_AudioProcessor_GetConnectorByType(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioConnectorType type
    )
{
    NEXUS_AudioInputHandle input = NULL;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    switch ( type )
    {
    case NEXUS_AudioConnectorType_eStereo:
        switch ( handle->type )
        {
        case NEXUS_AudioPostProcessing_eKaraokeVocal:
        case NEXUS_AudioPostProcessing_eAdvancedTsm:
        case NEXUS_AudioPostProcessing_eFade:
            input = &handle->connectors[NEXUS_AudioConnectorType_eStereo];
            break;
        default:
            break;
        }
        break;
    case NEXUS_AudioConnectorType_eMultichannel:
        switch ( handle->type )
        {
        case NEXUS_AudioPostProcessing_eAdvancedTsm:
            input = &handle->connectors[NEXUS_AudioConnectorType_eMultichannel];
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if ( input == NULL )
    {
        BDBG_ERR(("NEXUS_AudioProcessor type %d does not support connector format %d", handle->type, type));
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return input;
}

NEXUS_AudioInputHandle NEXUS_AudioProcessor_GetConnector(
    NEXUS_AudioProcessorHandle handle
    )
{
    return NEXUS_AudioProcessor_GetConnectorByType(handle, NEXUS_AudioConnectorType_eStereo);
}

NEXUS_Error NEXUS_AudioProcessor_AddInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    BDBG_ASSERT(NULL != input);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Only one input can be added"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);

    errCode = BAPE_Processor_AddInput((BAPE_ProcessorHandle)handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
    if ( errCode )
    {
        (void)BAPE_Processor_RemoveInput((BAPE_ProcessorHandle)handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port )
    {
        errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], input);
        if ( errCode )
        {
            NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
            (void)BAPE_Processor_RemoveInput((BAPE_ProcessorHandle)handle->apeHandle, (BAPE_Connector)input->port);
            return BERR_TRACE(errCode);
        }
    }
    handle->input = input;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioProcessor_RemoveInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    BDBG_ASSERT(NULL != handle->input);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input not connected"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BAPE_Processor_RemoveInput((BAPE_ProcessorHandle)handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if ( handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port )
    {
        errCode = NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioProcessor_RemoveAllInputs(
    NEXUS_AudioProcessorHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioProcessor);
    if ( handle->input )
    {
        return NEXUS_AudioProcessor_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

/* Stubs */
#if 0
struct NEXUS_AudioProcessor {
    NEXUS_OBJECT(NEXUS_AudioProcessor);
};

void NEXUS_AudioProcessor_GetDefaultOpenSettings(
    NEXUS_AudioProcessorOpenSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_AudioProcessorHandle NEXUS_AudioProcessor_Open(
    const NEXUS_AudioProcessorOpenSettings *pSettings     /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_AudioProcessor_P_Finalizer(
    NEXUS_AudioProcessorHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioProcessor, NEXUS_AudioProcessor_Close);

void NEXUS_AudioProcessor_GetSettings(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioProcessor_SetSettings(
    NEXUS_AudioProcessorHandle handle,
    const NEXUS_AudioProcessorSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioProcessor_GetStatus(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioProcessorStatus *pStatus    /* [out] Status */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
}

NEXUS_AudioInputHandle NEXUS_AudioProcessor_GetConnector(
    NEXUS_AudioProcessorHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_AudioProcessor_AddInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioProcessor_RemoveInput(
    NEXUS_AudioProcessorHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioProcessor_RemoveAllInputs(
    NEXUS_AudioProcessorHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioProcessor_P_Init(void)
{
    return BERR_SUCCESS;
}

void NEXUS_AudioProcessor_P_Uninit(void)
{
    return;
}
#endif
