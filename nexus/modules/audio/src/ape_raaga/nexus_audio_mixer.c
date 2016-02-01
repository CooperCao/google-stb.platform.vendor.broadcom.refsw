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
    bool started;
    NEXUS_AudioMixerSettings settings;
    NEXUS_AudioInputObject connector;
    BAPE_MixerHandle dspMixer;
    char name[10];   /* DSP MIXER */
} NEXUS_AudioMixer;

#if NEXUS_NUM_AUDIO_MIXERS
static NEXUS_AudioMixer g_mixers[NEXUS_NUM_AUDIO_MIXERS];
#else
#ifndef NEXUS_NUM_AUDIO_MIXERS
#define NEXUS_NUM_AUDIO_MIXERS 0
#endif
static NEXUS_AudioMixer g_mixers[1];
#endif

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
    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        pSettings->dolby.volumeLimiter.enableIntelligentLoudness = true;
    }
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
    NEXUS_AudioMixerSettings defaults;

    if ( NULL == pSettings )
    {
        NEXUS_AudioMixer_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

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

    pMixer->settings.mixUsingDsp = pSettings->mixUsingDsp;
    BDBG_MSG(("mixUsingDsp = %u",pMixer->settings.mixUsingDsp));

    /* Setup connector */
    if ( pSettings->mixUsingDsp )
    {
        BAPE_MixerSettings mixerSettings;
        BAPE_Connector connector;

        BKNI_Snprintf(pMixer->name, sizeof(pMixer->name), "DSP MIXER");
        NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eDspMixer, pMixer);
        pMixer->connector.pName = pMixer->name;
        BAPE_Mixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.type = BAPE_MixerType_eDsp;
        mixerSettings.dspIndex = pSettings->dspIndex;
        mixerSettings.multiStreamBalance = (pSettings->multiStreamBalance != 0) ? pSettings->multiStreamBalance : pSettings->dolby.multiStreamBalance;
        mixerSettings.certificationMode = pSettings->dolby.certificationMode;
        mixerSettings.enablePostProcessing = pSettings->dolby.enablePostProcessing;
        mixerSettings.volumeLimiter.enableVolumeLimiting = pSettings->dolby.volumeLimiter.enableVolumeLimiting;
        mixerSettings.volumeLimiter.enableIntelligentLoudness = pSettings->dolby.volumeLimiter.enableIntelligentLoudness;
        mixerSettings.volumeLimiter.volumeLimiterAmount = pSettings->dolby.volumeLimiter.volumeLimiterAmount;
        mixerSettings.dialogEnhancer.enableDialogEnhancer = pSettings->dolby.dialogEnhancer.enableDialogEnhancer;
        mixerSettings.dialogEnhancer.dialogEnhancerLevel = pSettings->dolby.dialogEnhancer.dialogEnhancerLevel;
        mixerSettings.dialogEnhancer.contentSuppressionLevel = pSettings->dolby.dialogEnhancer.contentSuppressionLevel;
        mixerSettings.intelligentEqualizer.enabled = pSettings->dolby.intelligentEqualizer.enabled;
        mixerSettings.intelligentEqualizer.numBands = pSettings->dolby.intelligentEqualizer.numBands;
        BKNI_Memcpy(mixerSettings.intelligentEqualizer.band, pSettings->dolby.intelligentEqualizer.band, sizeof(mixerSettings.intelligentEqualizer.band));
        mixerSettings.mixerSampleRate = pSettings->outputSampleRate;
        rc = BAPE_Mixer_Create(NEXUS_AUDIO_DEVICE_HANDLE, &mixerSettings, &pMixer->dspMixer);
        if ( rc )
        {
            rc = BERR_TRACE(rc);
            goto mixer_create_fail;
        }
        BAPE_Mixer_GetConnector(pMixer->dspMixer, &connector);
        pMixer->connector.port = (size_t)connector;
        pMixer->connector.format = NEXUS_AudioInputFormat_eNone; /* Determined by inputs */
    }
    else
    {
        BKNI_Snprintf(pMixer->name, sizeof(pMixer->name), "HW MIXER");
        NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eMixer, pMixer);
        pMixer->connector.pName = pMixer->name;

        pMixer->connector.format = NEXUS_AudioInputFormat_eNone; /* Determined by inputs */
        if ( pSettings->fixedOutputFormatEnabled )
        {
            switch ( pSettings->fixedOutputFormat )
            {
            case NEXUS_AudioMultichannelFormat_e5_1:
                pMixer->connector.format = NEXUS_AudioInputFormat_ePcm5_1;
                break;
            case NEXUS_AudioMultichannelFormat_e7_1:
                pMixer->connector.format = NEXUS_AudioInputFormat_ePcm7_1;
                break;
            case NEXUS_AudioMultichannelFormat_eStereo:
                pMixer->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
                break;
            default:
            case NEXUS_AudioMultichannelFormat_eMax:
                BDBG_ERR(("Fixed Mixer output format %u is not currently supported", pSettings->fixedOutputFormat));
                rc = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto mixer_create_fail;
                break; /* unreachable */
            }
            BDBG_MSG(("Fixed Mixer output format = %u enabled", pSettings->fixedOutputFormat));
        }
        pMixer->settings.fixedOutputFormatEnabled = pSettings->fixedOutputFormatEnabled;
        pMixer->settings.fixedOutputFormat = pSettings->fixedOutputFormat;
    }

    /* Set Settings */
    rc = NEXUS_AudioMixer_SetSettings(pMixer, pSettings);        /* Handles NULL for defaults */
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto mixer_create_fail;
    }

    /* Success */
    return pMixer;

mixer_create_fail:
    pMixer->opened = false;
    NEXUS_OBJECT_DESTROY(NEXUS_AudioMixer, pMixer);
    return NULL;
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

    if ( handle->dspMixer )
    {
        BAPE_Mixer_Destroy(handle->dspMixer);
    }

    /* Done */
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioMixer));
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
    NEXUS_AudioMixerDolbySettings oldDolbySettings;
    NEXUS_AudioInput oldMaster, newMaster;
    unsigned oldSampleRate;
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->settings.mixUsingDsp != pSettings->mixUsingDsp )
    {
        BDBG_ERR(("mixUsingDsp can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.fixedOutputFormatEnabled != pSettings->fixedOutputFormatEnabled )
    {
        BDBG_ERR(("fixedOutputFormatEnabled can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.fixedOutputFormat != pSettings->fixedOutputFormat )
    {
        BDBG_ERR(("fixedOutputFormat can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    oldMaster = handle->settings.master;
    newMaster = pSettings->master;
    oldSampleRate = handle->settings.outputSampleRate;
    oldDolbySettings = handle->settings.dolby;

    handle->settings = *pSettings;

    if ( oldMaster != newMaster )
    {
        if ( NULL != newMaster )
        {
            errCode = NEXUS_AudioMixer_RemoveInput(handle, newMaster);
            if ( errCode )
            {
                BDBG_ERR(("Master input %p is not connected to mixer %p", (void *)pSettings->master, (void *)handle));
                handle->settings.master = NULL;
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            errCode = NEXUS_AudioMixer_AddInput(handle, newMaster);
            if ( errCode )
            {
                handle->settings.master = NULL;
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }

    if ( pSettings->mixUsingDsp )
    {
        if ( (BKNI_Memcmp(&oldDolbySettings, &pSettings->dolby, sizeof(oldDolbySettings)) != 0) || oldSampleRate != pSettings->outputSampleRate )
        {
            BAPE_MixerSettings mixerSettings;
            BAPE_Mixer_GetSettings(handle->dspMixer, &mixerSettings);
            mixerSettings.multiStreamBalance = (pSettings->multiStreamBalance != 0) ? pSettings->multiStreamBalance : pSettings->dolby.multiStreamBalance;
            mixerSettings.certificationMode = pSettings->dolby.certificationMode;
            mixerSettings.enablePostProcessing = pSettings->dolby.enablePostProcessing;
            mixerSettings.volumeLimiter.enableVolumeLimiting = pSettings->dolby.volumeLimiter.enableVolumeLimiting;
            mixerSettings.volumeLimiter.enableIntelligentLoudness = pSettings->dolby.volumeLimiter.enableIntelligentLoudness;
            mixerSettings.volumeLimiter.volumeLimiterAmount = pSettings->dolby.volumeLimiter.volumeLimiterAmount;
            mixerSettings.dialogEnhancer.enableDialogEnhancer = pSettings->dolby.dialogEnhancer.enableDialogEnhancer;
            mixerSettings.dialogEnhancer.dialogEnhancerLevel = pSettings->dolby.dialogEnhancer.dialogEnhancerLevel;
            mixerSettings.dialogEnhancer.contentSuppressionLevel = pSettings->dolby.dialogEnhancer.contentSuppressionLevel;
            mixerSettings.intelligentEqualizer.enabled = pSettings->dolby.intelligentEqualizer.enabled;
            mixerSettings.intelligentEqualizer.numBands = pSettings->dolby.intelligentEqualizer.numBands;
            BKNI_Memcpy(mixerSettings.intelligentEqualizer.band, pSettings->dolby.intelligentEqualizer.band, sizeof(mixerSettings.intelligentEqualizer.band));
            mixerSettings.mixerSampleRate = pSettings->outputSampleRate;
            errCode = BAPE_Mixer_SetSettings(handle->dspMixer, &mixerSettings);
            if ( errCode )
            {
                handle->settings.dolby = oldDolbySettings;
                return BERR_TRACE(errCode);
            }
        }
    }

    /* Success */
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioMixer_Start(
    NEXUS_AudioMixerHandle handle
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);

    if ( handle->started )
    {
        BDBG_ERR(("Already started."));
    }
    
    if ( handle->settings.mixUsingDsp )
    {
        /* Only prepare the path layout if no inputs have already started*/
        if ( !NEXUS_AudioInput_P_IsRunning(&handle->connector) )
        {
            errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        errCode = BAPE_Mixer_Start(handle->dspMixer);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    
        handle->started = true;    
        return BERR_SUCCESS;
    }
    else if (handle->settings.fixedOutputFormatEnabled)
    {
        if ( !NEXUS_AudioInput_P_IsRunning(&handle->connector) )
        {
            errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        errCode = NEXUS_AudioInput_P_ExplictlyStartFMMMixers(&handle->connector);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        handle->started = true;
        return BERR_SUCCESS;
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }        
}

void NEXUS_AudioMixer_Stop(
    NEXUS_AudioMixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    
    if ( false == handle->started )
    {
        return;
    }
    
    if ( handle->settings.mixUsingDsp )
    {
        BAPE_Mixer_Stop(handle->dspMixer);
        handle->started = false;
    }
    else
    {
        NEXUS_AudioInput_P_ExplictlyStopFMMMixers(&handle->connector);
        handle->started = false;
        return;
    }            
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
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != input);

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

#if 0
    /* Check if I'm running */
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) || handle->started )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif

    if ( handle->settings.mixUsingDsp )
    {
        BAPE_MixerAddInputSettings addInputSettings;
        BAPE_MixerInputVolume inputVolume;
        bool master = (input == handle->settings.master)?true:false;

        BAPE_Mixer_GetDefaultAddInputSettings(&addInputSettings);
        addInputSettings.sampleRateMaster = master;
        errCode = BAPE_Mixer_AddInput(handle->dspMixer, (BAPE_Connector)input->port, &addInputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        /* Match input volume in mixer */
        NEXUS_AudioInput_P_GetVolume(input, &inputVolume);
        errCode = BAPE_Mixer_SetInputVolume(handle->dspMixer, (BAPE_Connector)input->port, &inputVolume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Connect at input node */
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        if ( handle->settings.mixUsingDsp )
        {
            BAPE_Mixer_RemoveInput(handle->dspMixer, (BAPE_Connector)input->port);
        }
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
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
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) || handle->started )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.mixUsingDsp )
    {
        (void)BAPE_Mixer_RemoveAllInputs(handle->dspMixer);
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
#if 0
    /* Check if I'm running */
    if ( NEXUS_AudioInput_P_IsRunning(&handle->connector) || handle->started )
    {
        BDBG_ERR(("Mixer %p is running.  Stop all inputs first.", (void *)handle));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
#endif

    /* Make sure this input is actually connected to this mixer */
    if ( !NEXUS_AudioInput_P_IsConnected(&handle->connector, input) )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.mixUsingDsp )
    {
        (void)BAPE_Mixer_RemoveInput(handle->dspMixer, (BAPE_Connector)input->port);
    }

    /* Remove Input */
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
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

/***************************************************************************
Summary: 
    Propagate mixer input volume into a mixer object in nexus 
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_P_SetInputVolume(
    NEXUS_AudioMixerHandle handle, 
    NEXUS_AudioInput input,
    const BAPE_MixerInputVolume *pInputVolume
    )
{
    if ( handle->dspMixer )
    {
        BERR_Code errCode;

        errCode = BAPE_Mixer_SetInputVolume(handle->dspMixer, (BAPE_Connector)input->port, pInputVolume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary: 
    Check mixer state - mixer can be started even if inputs are not started 
 ***************************************************************************/
bool NEXUS_AudioMixer_P_IsStarted(
    NEXUS_AudioMixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    return handle->started;
}

NEXUS_AudioMixerHandle
NEXUS_AudioMixer_P_GetMixerByIndex(
    unsigned index
    )
{
    BDBG_ASSERT(index < NEXUS_NUM_AUDIO_MIXERS);
    return g_mixers[index].opened?&g_mixers[index]:NULL;
}
