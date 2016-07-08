/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*   API name: AudioMixer
*    APIs for an audio mixer.  Allows inputs to be connected to outputs
*    with the capability to mix inputs and control volume/equalization.
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
    NEXUS_AudioMixerSettings settings;
    NEXUS_AudioInputObject connector;
} NEXUS_AudioMixer;

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

    if ( NULL == pSettings )
    {
        NEXUS_AudioMixer_GetDefaultSettings(&handle->settings);
    }
    else
    {
        if ( pSettings->mixUsingDsp )
        {
            BDBG_ERR(("DSP mixing not supported on this platform."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        handle->settings = *pSettings;
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

    /* Remove Input */
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

static NEXUS_Error NEXUS_AudioMixer_P_InputStarted(void *pHandle, struct NEXUS_AudioInputObject *pInput)
{
    NEXUS_Error errCode;
    NEXUS_AudioMixerHandle mixer = (NEXUS_AudioMixerHandle) pHandle;

    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != pInput);

    BDBG_MSG(("Input %p Started", (void *)pInput));


    /* If this is the first input to start... */
    if ( !NEXUS_AudioInput_P_IsRunning(&mixer->connector) )
    {
        /* Must add outputs */
        errCode = NEXUS_AudioInput_P_PrepareToStart(&mixer->connector);
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

    return BERR_SUCCESS;
}
NEXUS_AudioInput NEXUS_AudioMixer_GetConnector(
    NEXUS_AudioMixerHandle mixer
    )
{
    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    return &mixer->connector;
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

NEXUS_Error NEXUS_AudioMixer_GetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInput input,
    NEXUS_AudioMixerInputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioMixer_SetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInput input,
    const NEXUS_AudioMixerInputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif /* #if NEXUS_NUM_AUDIO_MIXERS */
