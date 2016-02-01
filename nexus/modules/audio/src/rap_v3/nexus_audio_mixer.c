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
*   API name: AudioMixer
*    APIs for an audio mixer.  Allows multiple inputs to be connected to outputs.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_audio_module.h"
#include "nexus_pid_channel.h"
#include "nexus_timebase.h"    /* Timebase */

BDBG_MODULE(nexus_audio_mixer);

typedef struct NEXUS_AudioMixer
{
    NEXUS_OBJECT(NEXUS_AudioMixer);
    bool opened;
    NEXUS_AudioAssociationHandle association;
    NEXUS_AudioMixerSettings settings;
    NEXUS_AudioInputObject connector;
    BRAP_ProcessingStageHandle stage;
    BRAP_ChannelHandle masterChannel;
} NEXUS_AudioMixer;

#define NEXUS_HAS_DSP_MIXING 1

#if NEXUS_NUM_AUDIO_MIXERS
static NEXUS_AudioMixer g_mixers[NEXUS_NUM_AUDIO_MIXERS];

static NEXUS_Error NEXUS_AudioMixer_P_InputStarted(void *pHandle, struct NEXUS_AudioInputObject *pInput);
static NEXUS_Error NEXUS_AudioMixer_P_ConnectionChange(void *pHandle, struct NEXUS_AudioInputObject *pInput);

/***************************************************************************
Summary:
    Get default settings for an audio mixer
See Also:
    NEXUS_AudioMixer_Open
 ***************************************************************************/
void NEXUS_AudioMixer_GetDefaultSettings(
    NEXUS_AudioMixerSettings *pSettings    /* [out] Default Settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioMixerSettings));
}

/***************************************************************************
Summary:
    Open an audio mixer
See Also:
    NEXUS_AudioMixer_Close
 ***************************************************************************/
NEXUS_AudioMixerHandle NEXUS_AudioMixer_Open(
    const NEXUS_AudioMixerSettings *pSettings
    )
{
    NEXUS_AudioMixer *pMixer=NULL;
    int i;
    NEXUS_Error rc=0;


    for ( i = 0; i < NEXUS_NUM_AUDIO_MIXERS; i++ )
    {
        pMixer = &g_mixers[i];
        if ( !pMixer->opened )
        {
            /* Clear structure */
            NEXUS_OBJECT_INIT(NEXUS_AudioMixer, pMixer);
            pMixer->opened = true;
            break;
        }
    }

    if ( i >= NEXUS_NUM_AUDIO_MIXERS )
    {
        BDBG_ERR(("Out of mixers"));
        rc=BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    /* Setup connector */
    NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eMixer, pMixer);
    pMixer->connector.format = NEXUS_AudioInputFormat_eNone; /* Determined by inputs */
    pMixer->connector.connectCb = NEXUS_AudioMixer_P_InputStarted;
    pMixer->connector.disconnectCb = NEXUS_AudioMixer_P_ConnectionChange;

    /* Set Settings */
    NEXUS_AudioMixer_SetSettings(pMixer, pSettings);        /* Handles NULL for defaults */

    /* Success */
    return pMixer;
}

/***************************************************************************
Summary:
    Close an audio mixer
See Also:
    NEXUS_AudioMixer_Open
 ***************************************************************************/
static void NEXUS_AudioMixer_P_Finalizer(
    NEXUS_AudioMixerHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioMixer, handle);

    NEXUS_AudioInput_Shutdown(&handle->connector);

    if ( handle->association )
    {
        NEXUS_AudioAssociation_P_Destroy(handle->association);
        handle->association = NULL;
    }

    if ( handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
        handle->stage = NULL;
    }

    NEXUS_OBJECT_CLEAR(NEXUS_AudioMixer, handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioMixer, NEXUS_AudioMixer_Close);

/***************************************************************************
Summary:
    Get current settings for an audio mixer
See Also:
    NEXUS_AudioMixer_SetSettings
 ***************************************************************************/
void NEXUS_AudioMixer_GetSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioMixerSettings *pSettings      /* [out] Mixer Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set settings of an audio mixer
See Also:
    NEXUS_AudioMixer_SetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_SetSettings(
    NEXUS_AudioMixerHandle handle,
    const NEXUS_AudioMixerSettings *pSettings    /* [out] Mixer Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);

    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) )
    {
        BDBG_ERR(("Cannot change mixer settings while running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( NULL == pSettings )
    {
        NEXUS_AudioMixer_GetDefaultSettings(&handle->settings);
    }
    else
    {
        if ( pSettings->mixUsingDsp )
        {
#if NEXUS_HAS_DSP_MIXING
            if ( NULL == handle->stage )
            {
                BERR_Code errCode;
                BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eFwMixer,
                                                       &g_NEXUS_StageSettings);
                errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                                     &g_NEXUS_StageSettings,
                                                     &handle->stage);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
            }
#else
            BDBG_ERR(("DSP mixing not supported on this platform."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
        }

        handle->settings = *pSettings;
    }

    /* Be sure to remove the association so it will be re-created next time with the new settings. */
    if ( NULL != handle->association )
    {
        NEXUS_AudioInput_P_RemoveDestinations(&handle->connector);
        NEXUS_AudioAssociation_P_Destroy(handle->association);
        handle->association = NULL;
    }
    
    /* If we've disabled DSP mixing, destroy the processing stage. */
    if ( handle->stage && !handle->settings.mixUsingDsp )
    {
        BRAP_DestroyProcessingStage(handle->stage);
        handle->stage = NULL;
    }

    /* Mixer settings are only applied at start-time.  Nothing more to do now, it will be handled at next start */

    /* Success */
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Add an audio input to a mixer
See Also:
    NEXUS_AudioMixer_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_AddInput(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != input);

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if I'm running */
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check format */
    if ( NEXUS_AudioInputFormat_eCompressed == NEXUS_AudioInput_P_GetFormat(input) )
    {
        BDBG_ERR(("Can not attach a compressed source to a mixer"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Connect at input node */
    return NEXUS_AudioInput_P_AddInput(&handle->connector, input);
}

/***************************************************************************
Summary:
    Remove all audio inputs from a mixer
See Also:
    NEXUS_AudioMixer_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_RemoveAllInputs(
    NEXUS_AudioMixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);

    /* Check if I'm running */
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if we're using DSP mixing, that must be disabled if we remove the master input. */
    if ( handle->settings.mixUsingDsp )
    {
        BDBG_WRN(("Removing the master input from a DSP mixer.  Disabling DSP mixing."));
        (void)NEXUS_AudioMixer_SetSettings(handle, NULL);
    }

    /* Connect at input node */
    return NEXUS_AudioInput_P_RemoveAllInputs(&handle->connector);
}

/***************************************************************************
Summary:
    Remove an audio input from a mixer
See Also:
    NEXUS_AudioMixer_AddInput
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_RemoveInput(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != input);

    /* Check if I'm running */
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Make sure this input is actually connected to this mixer */
    if ( !NEXUS_AudioInput_P_IsConnected(&handle->connector, input) )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if we're using DSP mixing, that must be disabled if we remove the master input. */
    if ( handle->settings.mixUsingDsp && input == handle->settings.master )
    {
        BDBG_WRN(("Removing the master input from a DSP mixer.  Disabling DSP mixing."));
        (void)NEXUS_AudioMixer_SetSettings(handle, NULL);
    }

    /* Remove Input */
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

static NEXUS_Error NEXUS_AudioMixer_P_InputStarted(void *pHandle, struct NEXUS_AudioInputObject *pInput)
{
    NEXUS_Error errCode;
    NEXUS_AudioMixerHandle mixer = (NEXUS_AudioMixerHandle) pHandle;
    NEXUS_AudioAssociationHandle association = mixer->association;

    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != pInput);

    BDBG_MSG(("Input %p Started", (void *)pInput));

    /* Create a new association if required */
    if ( NULL == association )
    {
        NEXUS_AudioInput upstreamMixerConnector;
        upstreamMixerConnector = NEXUS_AudioInput_P_FindUpstream(&mixer->connector, NEXUS_AudioInputType_eMixer);

        if ( NULL == upstreamMixerConnector )
        {
            BRAP_AssociatedChanSettings associationSettings;
            BDBG_MSG(("No association, creating one"));
            if ( NEXUS_AudioInput_P_GetAssociationSettings(&mixer->connector, &associationSettings) )
            {
                BDBG_ERR(("Unable to get mixer association"));
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
#if NEXUS_HAS_DSP_MIXING
            if ( mixer->settings.mixUsingDsp )
            {
                /* Validate master setting at start time as opposed to SetSettings() time.  Makes life
                   easier and also remains compatible with newer APE implementation of DSP mixer. */
                if ( NULL == mixer->settings.master )
                {
                    BDBG_ERR(("You must specify a master input if mixing with the DSP"));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                /* Make sure the master is a decoder, and is actually connected */
                if ( mixer->settings.master->objectType != NEXUS_AudioInputType_eDecoder )
                {
                    BDBG_ERR(("Only decoders can be master inputs to a mixer."));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                if ( !NEXUS_AudioInput_P_IsConnected(&mixer->connector, mixer->settings.master) )
                {
                    BDBG_ERR(("Input %p is not directly connected to this mixer.  Cannot set it as the master.", (void *)mixer->settings.master));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
                /* Set the master appropriately */
                associationSettings.hMasterChannel = mixer->masterChannel = NEXUS_AudioInput_P_GetChannel(mixer->settings.master, NULL);
                BDBG_ASSERT(NULL != associationSettings.hMasterChannel);    /* Should never happen */
            }
#endif
            mixer->association = NEXUS_AudioAssociation_P_Create(&associationSettings);
            if ( NULL == mixer->association )
            {
                BDBG_ERR(("Unable to create association"));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
        else
        {
            NEXUS_AudioMixerHandle upstreamMixer;
            upstreamMixer = upstreamMixerConnector->pObjectHandle;
            mixer->association = upstreamMixer->association;             
        }      
    }

    /* If this is the first input to start... */
    if ( !NEXUS_AudioInput_P_IsRunning(&mixer->connector) )
    {
        if ( NEXUS_AudioInputType_eDecoder == pInput->objectType &&
             NEXUS_AudioDecoder_P_IsMultiChannelOutputEnabled((NEXUS_AudioDecoderHandle)pInput->pObjectHandle) )
        {
            errCode = NEXUS_AudioInput_P_PrepareToStartWithProcessing(&mixer->connector, mixer->association, NULL, 0, NULL, NULL, true, NEXUS_AudioCodec_eUnknown);
        }
        else
        {
            /* Must add outputs */
            errCode = NEXUS_AudioInput_P_PrepareToStart(&mixer->connector, mixer->association);
        }
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioMixer_P_ConnectionChange(void *pHandle, struct NEXUS_AudioInputObject *pInput)
{
    NEXUS_AudioMixerHandle mixer = (NEXUS_AudioMixerHandle) pHandle;

    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    BSTD_UNUSED(pInput);

    BDBG_MSG(("Mixer %p connection change", (void *)mixer));
    if ( NULL != mixer->association )
    {
        BDBG_MSG(("Mixer has association"));
        if ( !NEXUS_AudioInput_P_IsRunning(&mixer->connector) )
        {
            BDBG_MSG(("No other inputs are running.  Destroying association."));
            NEXUS_AudioAssociation_P_Destroy(mixer->association);
            mixer->association = NULL;
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get the audio input connector for connection to outputs or post-processing
***************************************************************************/
NEXUS_AudioInput NEXUS_AudioMixer_GetConnector(
    NEXUS_AudioMixerHandle mixer
    )
{
    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    return &mixer->connector;
}

BRAP_ProcessingStageHandle NEXUS_AudioMixer_P_GetStageHandle(NEXUS_AudioMixerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    return handle->stage;
}

bool NEXUS_AudioMixer_P_IsSlaveChannel(NEXUS_AudioMixerHandle handle, BRAP_ChannelHandle channel)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);

#if NEXUS_HAS_DSP_MIXING
    if ( handle->settings.mixUsingDsp && channel != handle->masterChannel )
    {
        return true;
    }
#endif
    BSTD_UNUSED(channel);
    return false;
}

bool NEXUS_AudioMixer_P_IsUsingDsp(NEXUS_AudioMixerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    return handle->settings.mixUsingDsp;
}

NEXUS_Error NEXUS_AudioMixer_Start(
    NEXUS_AudioMixerHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioMixer_Stop(
    NEXUS_AudioMixerHandle handle
    )
{
    BSTD_UNUSED(handle);
}


#endif /* #if NEXUS_NUM_AUDIO_MIXERS */

