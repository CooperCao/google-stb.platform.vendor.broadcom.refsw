/***************************************************************************
 *     (c)2006-2013 Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "nexus_types.h"
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_echo_canceller);

typedef struct NEXUS_EchoCanceller
{
    NEXUS_OBJECT(NEXUS_EchoCanceller);
    BAPE_EchoCancellerHandle apeHandle;
    NEXUS_EchoCancellerSettings settings;
    NEXUS_AudioInputObject connector;
    NEXUS_AudioInput localInput, remoteInput;
    char name[15];   /* ECHO CANCELLER */
} NEXUS_EchoCanceller;

void NEXUS_EchoCanceller_GetDefaultSettings(
    NEXUS_EchoCancellerSettings *pSettings   /* [out] */
    )
{
    BAPE_EchoCancellerSettings apeSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_EchoCanceller_GetDefaultSettings(&apeSettings);
    pSettings->algorithm = (NEXUS_EchoCancellerAlgorithm)apeSettings.algorithm;
}

NEXUS_EchoCancellerHandle NEXUS_EchoCanceller_Create(
    const NEXUS_EchoCancellerSettings *pSettings
    )
{
    NEXUS_EchoCancellerSettings defaults;
    NEXUS_EchoCancellerHandle handle;
    BAPE_EchoCancellerSettings apeSettings;
    BAPE_Connector connector;
    BERR_Code errCode;

    if ( NULL == pSettings )
    {
        NEXUS_EchoCanceller_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_EchoCanceller));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }

    BKNI_Memset(handle, 0, sizeof(NEXUS_EchoCanceller));
    NEXUS_OBJECT_INIT(NEXUS_EchoCanceller, handle);
    
    BAPE_EchoCanceller_GetDefaultSettings(&apeSettings);
    errCode = BAPE_EchoCanceller_Create(NEXUS_AUDIO_DEVICE_HANDLE, &apeSettings, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_ape_handle;
    }

    BKNI_Snprintf(handle->name, sizeof(handle->name), "ECHO CANCELLER");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eEchoCanceller, handle);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_ePcmMono;
    BAPE_EchoCanceller_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;
    
    errCode = NEXUS_EchoCanceller_SetSettings(handle, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_settings;
    }

    return handle;

err_settings:
    BAPE_EchoCanceller_Destroy(handle->apeHandle);
err_ape_handle:
    BDBG_OBJECT_DESTROY(handle, NEXUS_EchoCanceller);
    BKNI_Free(handle);
err_handle:
    return NULL;
}

static void NEXUS_EchoCanceller_P_Finalizer(
    NEXUS_EchoCancellerHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_EchoCanceller, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_EchoCanceller_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_EchoCanceller, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_EchoCanceller, NEXUS_EchoCanceller_Destroy);

void NEXUS_EchoCanceller_GetSettings(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_EchoCancellerSettings *pSettings    /* [out] Settings */
    )
{
    BAPE_EchoCancellerSettings apeSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_EchoCanceller_GetSettings(handle->apeHandle, &apeSettings);
    pSettings->algorithm = (NEXUS_EchoCancellerAlgorithm)apeSettings.algorithm;
}

NEXUS_Error NEXUS_EchoCanceller_SetSettings(
    NEXUS_EchoCancellerHandle handle,
    const NEXUS_EchoCancellerSettings *pSettings
    )
{
    BAPE_EchoCancellerSettings apeSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_EchoCanceller_GetSettings(handle->apeHandle, &apeSettings);
    apeSettings.algorithm = (BAPE_EchoCancellerAlgorithm)pSettings->algorithm;
    errCode = BAPE_EchoCanceller_SetSettings(handle->apeHandle, &apeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

void NEXUS_EchoCanceller_GetAlgorithmSettings(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_EchoCancellerAlgorithm algorithm,
    NEXUS_EchoCancellerAlgorithmSettings *pSettings  /* [out] */
    )
{
    BAPE_EchoCancellerAlgorithmSettings apeSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);

    BAPE_EchoCanceller_GetAlgorithmSettings(handle->apeHandle, (BAPE_EchoCancellerAlgorithm)algorithm, &apeSettings);
    pSettings->algorithm = algorithm;
    switch ( algorithm )
    {
    case NEXUS_EchoCancellerAlgorithm_eSpeex:
        pSettings->algorithmSettings.speex.gainMode = (NEXUS_SpeexEchoCancellerGainMode)apeSettings.algorithmSettings.speex.gainMode;
        break;
    default:
        break;
    }
}

NEXUS_Error NEXUS_EchoCanceller_SetAlgorithmSettings(
    NEXUS_EchoCancellerHandle handle,
    const NEXUS_EchoCancellerAlgorithmSettings *pSettings
    )
{
    BAPE_EchoCancellerAlgorithmSettings apeSettings;
    BERR_Code errCode;
        
    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);

    BAPE_EchoCanceller_GetAlgorithmSettings(handle->apeHandle, (BAPE_EchoCancellerAlgorithm)pSettings->algorithm, &apeSettings);
    switch ( pSettings->algorithm )
    {
    case NEXUS_EchoCancellerAlgorithm_eSpeex:
        apeSettings.algorithmSettings.speex.gainMode = (BAPE_SpeexEchoCancellerGainMode)pSettings->algorithmSettings.speex.gainMode;
        break;
    default:
        return BERR_SUCCESS;
    }

    errCode = BAPE_EchoCanceller_SetAlgorithmSettings(handle->apeHandle, &apeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

NEXUS_AudioInput NEXUS_EchoCanceller_GetConnector(
    NEXUS_EchoCancellerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    return (&handle->connector);
}

NEXUS_Error NEXUS_EchoCanceller_AddLocalInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    if ( handle->localInput )
    {
        BDBG_ERR(("Another local input is already connected to this echo canceller"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_EchoCanceller_AddLocalInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_EchoCanceller_RemoveLocalInput(handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }

    handle->localInput = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove a local (near-end) input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveLocalInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    if ( handle->localInput != input )
    {
        BDBG_ERR(("Input %p is not the local input for this echo canceller", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    (void)BAPE_EchoCanceller_RemoveLocalInput(handle->apeHandle, (BAPE_Connector)input->port);
    handle->localInput = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

/***************************************************************************
Summary:
Add a remote (far-end) input to the echo canceller
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_AddRemoteInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    if ( handle->remoteInput )
    {
        BDBG_ERR(("Another remote input is already connected to this echo canceller"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_EchoCanceller_AddRemoteInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_EchoCanceller_RemoveRemoteInput(handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }

    handle->remoteInput = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Remove a remote (far-end) input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveRemoteInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    if ( handle->remoteInput != input )
    {
        BDBG_ERR(("Input %p is not the remote input for this echo canceller", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    (void)BAPE_EchoCanceller_RemoveRemoteInput(handle->apeHandle, (BAPE_Connector)input->port);
    handle->remoteInput = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveAllInputs(
    NEXUS_EchoCancellerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_EchoCanceller);
    if ( handle->remoteInput )
    {
        (void)NEXUS_EchoCanceller_RemoveRemoteInput(handle, handle->remoteInput);
    }
    if ( handle->localInput )
    {
        (void)NEXUS_EchoCanceller_RemoveLocalInput(handle, handle->localInput);
    }
    return BERR_SUCCESS;
}

