/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*    APIs for an audio mixer.  Allows multiple inputs to be connected to outputs.
*
***************************************************************************/
#include "nexus_audio_module.h"
#include "nexus_pid_channel.h"
#include "nexus_timebase.h"    /* Timebase */
#include "priv/nexus_audio_mixer_priv.h"

BDBG_MODULE(nexus_audio_mixer);

typedef struct NEXUS_AudioMixerInputNode
{
    NEXUS_AudioMixerInputSettings inputSettings;
    NEXUS_AudioInputHandle input;
    BLST_Q_ENTRY(NEXUS_AudioMixerInputNode) inputNode;
} NEXUS_AudioMixerInputNode;

typedef struct NEXUS_AudioMixer
{
    NEXUS_OBJECT(NEXUS_AudioMixer);
    bool opened;
    bool started;
    bool explicitlyStarted;
    NEXUS_AudioMixerSettings settings;
    NEXUS_AudioInputObject connector;
    BAPE_MixerHandle dspMixer, imMixer;
    char name[13];   /* INT HW MIXER */
    BLST_Q_HEAD(InputList, NEXUS_AudioMixerInputNode) inputList;
} NEXUS_AudioMixer;

#if NEXUS_NUM_AUDIO_MIXERS
static NEXUS_AudioMixer g_mixers[NEXUS_NUM_AUDIO_MIXERS];
#else
#ifndef NEXUS_NUM_AUDIO_MIXERS
#define NEXUS_NUM_AUDIO_MIXERS 0
#endif
static NEXUS_AudioMixer g_mixers[1];
#endif

#define min(A,B) ((A)<(B)?(A):(B))

static NEXUS_Error NEXUS_AudioMixer_P_ApplyMixerVolume(NEXUS_AudioMixerHandle handle, NEXUS_AudioInputHandle input, const BAPE_MixerInputVolume *pInputVolume, const NEXUS_AudioMixerInputNode *pInputNode);

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
    BAPE_MixerSettings mixerSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioMixerSettings));
    BAPE_Mixer_GetDefaultSettings(&mixerSettings);
    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        pSettings->dolby.volumeLimiter.enableIntelligentLoudness = true;
    }
    pSettings->loopbackMixerEnabled = mixerSettings.loopbackMixerEnabled;
    BDBG_CASSERT(NEXUS_AudioChannel_eMax == (NEXUS_AudioChannel)BAPE_Channel_eMax);
    BKNI_Memcpy(pSettings->loopbackVolumeMatrix, mixerSettings.loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax);
    BKNI_Memcpy(pSettings->outputVolume, mixerSettings.outputVolume.volume, sizeof(uint32_t)*NEXUS_AudioChannel_eMax);
    pSettings->outputMuted = mixerSettings.outputVolume.muted;
    pSettings->mainDecodeFade.level = mixerSettings.fade.mainDecodeFade.level;
    pSettings->mainDecodeFade.type = mixerSettings.fade.mainDecodeFade.type;
    pSettings->mainDecodeFade.duration = mixerSettings.fade.mainDecodeFade.duration;
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
    NEXUS_AudioMixerSettings * defaults = NULL;
    int mixerCount;

    mixerCount = min(g_NEXUS_audioModuleData.capabilities.numMixers, NEXUS_NUM_AUDIO_MIXERS);

    for ( i = 0; i < mixerCount; i++ )
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

    if ( i >= mixerCount )
    {
        BDBG_ERR(("Out of mixers"));
        rc=BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        defaults = BKNI_Malloc(sizeof(NEXUS_AudioMixerSettings));
        if ( !defaults )
        {
            BDBG_ERR(("malloc of mixer settings failed"));
            rc=BERR_TRACE(BERR_NOT_SUPPORTED);
            return NULL;
        }
        NEXUS_AudioMixer_GetDefaultSettings(defaults);
        pSettings = defaults;
    }

    pMixer->settings.mixUsingDsp = pSettings->mixUsingDsp;
    pMixer->settings.intermediate = pSettings->intermediate;
    BDBG_MSG(("mixUsingDsp = %u, intermediate = %u",pMixer->settings.mixUsingDsp,pMixer->settings.intermediate));

    /* Setup connector */
    if ( pSettings->mixUsingDsp )
    {
        BAPE_MixerSettings mixerSettings;
        BAPE_Connector connector;

        BKNI_Snprintf(pMixer->name, sizeof(pMixer->name), "DSP MIXER");
        NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eDspMixer, pMixer);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &pMixer->connector, Open);
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
        mixerSettings.loopbackMixerEnabled = pSettings->loopbackMixerEnabled;
        BKNI_Memcpy(mixerSettings.loopbackVolumeMatrix, pSettings->loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax);
        BKNI_Memcpy(mixerSettings.outputVolume.volume, pSettings->outputVolume, sizeof(uint32_t)*NEXUS_AudioChannel_eMax);
        mixerSettings.outputVolume.muted = pSettings->outputMuted;
        mixerSettings.fade.continuousFading = pSettings->dolby.fade.continuousFading;
        mixerSettings.fade.mainDecodeFade.level = pSettings->mainDecodeFade.level;
        mixerSettings.fade.mainDecodeFade.type = pSettings->mainDecodeFade.type;
        mixerSettings.fade.mainDecodeFade.duration = pSettings->mainDecodeFade.duration;
        if ( NEXUS_GetEnv("audio_mixer_zereos_disabled") ) {
            mixerSettings.mixerEnableZeros = false;
        }

        rc = BAPE_Mixer_Create(NEXUS_AUDIO_DEVICE_HANDLE, &mixerSettings, &pMixer->dspMixer);
        if ( rc )
        {
            rc = BERR_TRACE(rc);
            goto mixer_create_fail;
        }
        BAPE_Mixer_GetConnector(pMixer->dspMixer, &connector);
        pMixer->connector.port = (size_t)connector;
        pMixer->connector.format = NEXUS_AudioInputFormat_eNone; /* Determined by inputs */
        BLST_Q_INIT(&pMixer->inputList);
    }
    else if ( pSettings->intermediate ) /* Fixed "intermediate" HW mixer */
    {
        BAPE_MixerSettings mixerSettings;
        BAPE_Connector connector;

        BKNI_Snprintf(pMixer->name, sizeof(pMixer->name), "INT HW MIXER");
        NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eIntermediateMixer, pMixer);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &pMixer->connector, Open);
        pMixer->connector.pName = pMixer->name;
        BAPE_Mixer_GetDefaultSettings(&mixerSettings);
        mixerSettings.type = BAPE_MixerType_eStandard;
        mixerSettings.format = BAPE_MixerFormat_eAuto;
        if ( pSettings->fixedOutputFormatEnabled )
        {
            switch ( pSettings->fixedOutputFormat )
            {
            case NEXUS_AudioMultichannelFormat_e7_1:
                mixerSettings.format = BAPE_MixerFormat_ePcm7_1;
                break;
            case NEXUS_AudioMultichannelFormat_e5_1:
                mixerSettings.format = BAPE_MixerFormat_ePcm5_1;
                break;
            case NEXUS_AudioMultichannelFormat_eStereo:
                mixerSettings.format = BAPE_MixerFormat_ePcmStereo;
                break;
            default:
            case NEXUS_AudioMultichannelFormat_eMax:
                mixerSettings.format = BAPE_MixerFormat_eAuto;
                break;
            }
        }
        BDBG_MSG(("Intermediate Mixer - create bape mixer, format %lu", (unsigned long)mixerSettings.format));
        mixerSettings.mixerSampleRate = pSettings->outputSampleRate;
        rc = BAPE_Mixer_Create(NEXUS_AUDIO_DEVICE_HANDLE, &mixerSettings, &pMixer->imMixer);
        if ( rc )
        {
            rc = BERR_TRACE(rc);
            goto mixer_create_fail;
        }
        BAPE_Mixer_GetConnector(pMixer->imMixer, &connector);
        pMixer->connector.port = (size_t)connector;
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
        BLST_Q_INIT(&pMixer->inputList);
    }
    else /* Automatic Output Mixer */
    {
        BKNI_Snprintf(pMixer->name, sizeof(pMixer->name), "HW MIXER");
        NEXUS_AUDIO_INPUT_INIT(&pMixer->connector, NEXUS_AudioInputType_eMixer, pMixer);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &pMixer->connector, Open);
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

    if ( defaults )
    {
        BKNI_Free(defaults);
    }

    /* Success */
    return pMixer;

mixer_create_fail:
    if ( defaults )
    {
        BKNI_Free(defaults);
        defaults = NULL;
    }
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

    NEXUS_OBJECT_DESTROY(NEXUS_AudioMixer, handle);

    /* Done */
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioMixer));
}

static void NEXUS_AudioMixer_P_Release(NEXUS_AudioMixerHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioMixer, NEXUS_AudioMixer_Close);

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
    NEXUS_AudioMixerDolbySettings * pOldDolbySettings = NULL;
    NEXUS_AudioInputHandle oldMaster, newMaster;
    bool loopbackVolMatrixChanged = false;
    bool outputVolumeChanged = false;
    bool mainDecodeFadeChanged = false;
    unsigned oldSampleRate;
    int oldMultiStreamBalance;
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->settings.mixUsingDsp != pSettings->mixUsingDsp ||
         handle->settings.intermediate != pSettings->intermediate )
    {
        BDBG_ERR(("mixUsingDsp/intermediate can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.fixedOutputFormatEnabled != pSettings->fixedOutputFormatEnabled )
    {
        BDBG_ERR(("fixedOutputFormatEnabled can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.fixedOutputFormat != pSettings->fixedOutputFormat )
    {
        BDBG_ERR(("fixedOutputFormat can only be set when a mixer is opened.  It cannot be changed on-the-fly."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pOldDolbySettings = BKNI_Malloc(sizeof(NEXUS_AudioMixerDolbySettings));
    if ( pOldDolbySettings == NULL )
    {
        BDBG_ERR(("Unable to allocate memory for NEXUS_AudioMixerDolbySettings"));
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    oldMaster = handle->settings.master;
    newMaster = pSettings->master;
    oldSampleRate = handle->settings.outputSampleRate;
    BKNI_Memcpy(pOldDolbySettings, &(handle->settings.dolby), sizeof(NEXUS_AudioMixerDolbySettings));
    oldMultiStreamBalance = handle->settings.multiStreamBalance;

    outputVolumeChanged = handle->settings.intermediate && (0 != BKNI_Memcmp(handle->settings.outputVolume, pSettings->outputVolume, sizeof(uint32_t)*NEXUS_AudioChannel_eMax) || handle->settings.outputMuted != pSettings->outputMuted);
    loopbackVolMatrixChanged = handle->settings.mixUsingDsp && (0 != BKNI_Memcmp(handle->settings.loopbackVolumeMatrix, pSettings->loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax));
    mainDecodeFadeChanged = handle->settings.mixUsingDsp && BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 &&
        (handle->settings.mainDecodeFade.level != pSettings->mainDecodeFade.level || handle->settings.dolby.fade.continuousFading != pSettings->dolby.fade.continuousFading);

    handle->settings = *pSettings;

    if ( oldMaster != newMaster )
    {
        if ( NULL != newMaster )
        {
            NEXUS_AudioMixerInputSettings inputSettings;
            if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
            {
                errCode = NEXUS_AudioMixer_GetInputSettings(handle, newMaster, &inputSettings);
                if ( errCode )
                {
                    BDBG_ERR(("%s Unable to get master inputs Input Volume", BSTD_FUNCTION));
                }
            }

            errCode = NEXUS_AudioMixer_RemoveInput(handle, newMaster);
            if ( errCode )
            {
                BDBG_ERR(("Master input %p is not connected to mixer %p", (void *)pSettings->master, (void *)handle));
                handle->settings.master = NULL;
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_cleanup;
            }
            errCode = NEXUS_AudioMixer_AddInput(handle, newMaster);
            if ( errCode )
            {
                handle->settings.master = NULL;
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_cleanup;
            }

            if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
            {
                errCode = NEXUS_AudioMixer_SetInputSettings(handle, newMaster, &inputSettings);
                if ( errCode )
                {
                    BDBG_ERR(("%s Unable to set master inputs Input Volume", BSTD_FUNCTION));
                }
            }
        }
    }

    if ( pSettings->mixUsingDsp )
    {
        if ( (BKNI_Memcmp(pOldDolbySettings, &pSettings->dolby, sizeof(NEXUS_AudioMixerDolbySettings)) != 0) ||
             oldSampleRate != pSettings->outputSampleRate ||
             oldMultiStreamBalance != pSettings->multiStreamBalance ||
             loopbackVolMatrixChanged || mainDecodeFadeChanged )
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
            mixerSettings.fade.continuousFading = pSettings->dolby.fade.continuousFading;
            mixerSettings.fade.mainDecodeFade.level = pSettings->mainDecodeFade.level;
            mixerSettings.fade.mainDecodeFade.type = pSettings->mainDecodeFade.type;
            mixerSettings.fade.mainDecodeFade.duration = pSettings->mainDecodeFade.duration;
            mixerSettings.mixerSampleRate = pSettings->outputSampleRate;
            BKNI_Memcpy(mixerSettings.loopbackVolumeMatrix, pSettings->loopbackVolumeMatrix, sizeof(int32_t)*NEXUS_AudioChannel_eMax*NEXUS_AudioChannel_eMax);
            errCode = BAPE_Mixer_SetSettings(handle->dspMixer, &mixerSettings);
            if ( errCode )
            {
                BKNI_Memcpy(&(handle->settings.dolby), pOldDolbySettings, sizeof(NEXUS_AudioMixerDolbySettings));
                BERR_TRACE(errCode);
                goto err_cleanup;
            }
        }
    }
    else if ( pSettings->intermediate )
    {
        if ( oldSampleRate != pSettings->outputSampleRate || outputVolumeChanged )
        {
            BAPE_MixerSettings mixerSettings;
            BAPE_Mixer_GetSettings(handle->imMixer, &mixerSettings);
            mixerSettings.mixerSampleRate = pSettings->outputSampleRate;
            BKNI_Memcpy(mixerSettings.outputVolume.volume, pSettings->outputVolume, sizeof(uint32_t)*NEXUS_AudioChannel_eMax);
            mixerSettings.outputVolume.muted = pSettings->outputMuted;
            BDBG_MSG(("mixer output volume %x, muted %d", mixerSettings.outputVolume.volume[0], mixerSettings.outputVolume.muted));
            errCode = BAPE_Mixer_SetSettings(handle->imMixer, &mixerSettings);
            if ( errCode )
            {
                BKNI_Memcpy(&(handle->settings.dolby), pOldDolbySettings, sizeof(NEXUS_AudioMixerDolbySettings));
                BERR_TRACE(errCode);
                goto err_cleanup;
            }
        }
    }

    if ( pOldDolbySettings )
    {
        BKNI_Free(pOldDolbySettings);
        pOldDolbySettings = NULL;
    }

    /* Success */
    return BERR_SUCCESS;

err_cleanup:
    if ( pOldDolbySettings )
    {
        BKNI_Free(pOldDolbySettings);
        pOldDolbySettings = NULL;
    }
    return errCode;
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

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;

        /* Only prepare the path layout if no inputs have already started*/
        if ( !NEXUS_AudioInput_P_IsRunning(&handle->connector) )
        {
            errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        errCode = BAPE_Mixer_Start(mixer);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        handle->started = true;
        handle->explicitlyStarted = true;
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
            BERR_TRACE(errCode);
            NEXUS_AudioInput_P_ExplictlyStopFMMMixers(&handle->connector);
            return errCode;
        }

        handle->started = true;
        handle->explicitlyStarted = true;
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

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;

        BAPE_Mixer_Stop(mixer);
        handle->started = false;
        handle->explicitlyStarted = false;
    }
    else
    {
        NEXUS_AudioInput_P_ExplictlyStopFMMMixers(&handle->connector);
        handle->started = false;
        handle->explicitlyStarted = false;
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
    NEXUS_AudioInputHandle input
    )
{
    BERR_Code errCode;
    NEXUS_AudioMixerInputNode *pInputNode;


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

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BAPE_MixerAddInputSettings addInputSettings;
        BAPE_MixerInputVolume inputVolume;
        BAPE_MixerHandle mixer;
        BAPE_MixerInputVolume piVolume;
        BAPE_MixerInputSettings piSettings;
        bool master;
        int i;

        master = (input == handle->settings.master)?true:false;
        mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;

        BAPE_Mixer_GetDefaultAddInputSettings(&addInputSettings);
        addInputSettings.sampleRateMaster = master;
        errCode = BAPE_Mixer_AddInput(mixer, (BAPE_Connector)input->port, &addInputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        pInputNode = BKNI_Malloc(sizeof(NEXUS_AudioMixerInputNode));
        if ( NULL == pInputNode )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            BAPE_Mixer_RemoveInput(mixer, (BAPE_Connector)input->port);
            return errCode;
        }
        BKNI_Memset(pInputNode, 0, sizeof(NEXUS_AudioMixerInputNode));
        pInputNode->input = input;

        /* populate Input Volume defaults from APE */
        BAPE_Mixer_GetInputVolume(mixer, (BAPE_Connector)input->port, &piVolume);
        for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
        {
            pInputNode->inputSettings.volumeMatrix[i][i] = (int32_t)((int64_t)NEXUS_AUDIO_VOLUME_LINEAR_NORMAL * (int64_t)piVolume.coefficients[i][i]/BAPE_VOLUME_NORMAL);
        }
        pInputNode->inputSettings.muted = piVolume.muted;
        pInputNode->inputSettings.fade.level = piVolume.fade.level;
        pInputNode->inputSettings.fade.duration = piVolume.fade.duration;
        pInputNode->inputSettings.fade.type = piVolume.fade.type;
        if ( handle->settings.intermediate )
        {
            BAPE_Mixer_GetInputSettings(mixer, (BAPE_Connector)input->port, &piSettings);
            pInputNode->inputSettings.sampleRateConversionEnabled = piSettings.srcEnabled;
        }
        BLST_Q_INSERT_TAIL(&handle->inputList, pInputNode, inputNode);

        /* Because APE mixers default to unity with add input keep nexus in line
           Proper volume should be set afterwards */
        NEXUS_AudioInput_P_GetDefaultVolume(&inputVolume);
        /*NEXUS_AudioInput_P_GetVolume(input, &inputVolume);*/

        errCode = NEXUS_AudioMixer_P_SetInputVolume(handle, input, &inputVolume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Connect at input node */
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
        {
            BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
            BAPE_Mixer_RemoveInput(mixer, (BAPE_Connector)input->port);
            pInputNode = BLST_Q_FIRST(&handle->inputList);
            while ( NULL != pInputNode && pInputNode->input != input)
            {
                pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
            }
            if (pInputNode != NULL)
            {
                BLST_Q_REMOVE(&handle->inputList, pInputNode, inputNode);
                BKNI_Free(pInputNode);
            }
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
    NEXUS_AudioMixerInputNode *pInputNode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
        (void)BAPE_Mixer_RemoveAllInputs(mixer);
        while ( NULL != (pInputNode = BLST_Q_FIRST(&handle->inputList)) )
        {
            BLST_Q_REMOVE(&handle->inputList, pInputNode, inputNode);
            BKNI_Free(pInputNode);
        }
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
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_AudioMixerInputNode *pInputNode;

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

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
        (void)BAPE_Mixer_RemoveInput(mixer, (BAPE_Connector)input->port);

        pInputNode = BLST_Q_FIRST(&handle->inputList);
        while ( NULL != pInputNode && pInputNode->input != input)
        {
            pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
        }
        if (pInputNode != NULL)
        {
            BLST_Q_REMOVE(&handle->inputList, pInputNode, inputNode);
            BKNI_Free(pInputNode);
        }
    }

    /* Remove Input */
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

/***************************************************************************
Summary:
Get the audio input connector for connection to outputs or post-processing
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AudioMixer_GetConnector(
    NEXUS_AudioMixerHandle mixer
    )
{
    BDBG_OBJECT_ASSERT(mixer, NEXUS_AudioMixer);
    return &mixer->connector;
}


/***************************************************************************
Summary:
    Acquire mixer input volume into a mixer object in nexus
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    NEXUS_AudioMixerInputSettings *pSettings
    )
{
    if ( pSettings == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        NEXUS_AudioMixerInputNode *pInputNode;

        pInputNode = BLST_Q_FIRST(&handle->inputList);
        while ( NULL != pInputNode && pInputNode->input != input)
        {
            pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
        }
        if (pInputNode != NULL)
        {
            *pSettings = pInputNode->inputSettings;
        }
        else
        {
            BDBG_ERR(("%s: Could not locate input(%p) in mixer(%p)",BSTD_FUNCTION, (void *)input, (void *)handle));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        BDBG_ERR(("%s: Only supported for DSP or Intermediate Mixers",BSTD_FUNCTION));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Propagate mixer input volume into a mixer object in nexus
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_SetInputSettings(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    const NEXUS_AudioMixerInputSettings *pSettings
    )
{
    if ( pSettings == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        NEXUS_AudioMixerInputNode *pInputNode;
        BAPE_MixerInputVolume inputVolume;
        NEXUS_Error rc;

        pInputNode = BLST_Q_FIRST(&handle->inputList);
        while ( NULL != pInputNode && pInputNode->input != input)
        {
            pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
        }
        if (pInputNode != NULL)
        {
            BAPE_MixerInputSettings inputSettings;

            pInputNode->inputSettings = *pSettings;
            NEXUS_AudioInput_P_GetVolume(input, &inputVolume);
            inputVolume.fade.level = pSettings->fade.level;
            inputVolume.fade.duration = pSettings->fade.duration;
            inputVolume.fade.type = pSettings->fade.type;
            rc = NEXUS_AudioMixer_P_ApplyMixerVolume(handle, input, &inputVolume, pInputNode);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }

            if ( handle->settings.intermediate )
            {
                BAPE_MixerHandle mixer = handle->imMixer;
                rc = BAPE_Mixer_GetInputSettings(mixer, (BAPE_Connector)input->port, &inputSettings);
                if ( rc )
                {
                    return BERR_TRACE(rc);
                }
                inputSettings.srcEnabled = pSettings->sampleRateConversionEnabled;
                rc = BAPE_Mixer_SetInputSettings(mixer, (BAPE_Connector)input->port, &inputSettings);
                if ( rc )
                {
                    return BERR_TRACE(rc);
                }
            }
        }
        else
        {
            BDBG_ERR(("%s: Could not locate input(%p) in mixer(%p)", BSTD_FUNCTION, (void *)input, (void *)handle));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        BDBG_ERR(("%s: Only supported for DSP or Intermediate Mixers", BSTD_FUNCTION));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get Status of Mixer Input
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetInputStatus(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    NEXUS_AudioMixerInputStatus *pStatus
    )
{
    if ( pStatus == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        NEXUS_AudioMixerInputNode *pInputNode;

        pInputNode = BLST_Q_FIRST(&handle->inputList);
        while ( NULL != pInputNode && pInputNode->input != input)
        {
            pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
        }
        if (pInputNode != NULL)
        {
            BERR_Code errCode;
            BAPE_MixerInputStatus piStatus;
            BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
            errCode = BAPE_Mixer_GetInputStatus(mixer, (BAPE_Connector)input->port, &piStatus);
            if ( errCode == BERR_NOT_AVAILABLE )
            {
                return BERR_NOT_AVAILABLE;
            }
            else if ( errCode != BERR_SUCCESS )
            {
                return BERR_TRACE(errCode);
            }

            pStatus->fade.active = piStatus.fade.active;
            pStatus->fade.level = piStatus.fade.level;
            pStatus->fade.remaining = piStatus.fade.remaining;
        }
        else
        {
            BDBG_ERR(("%s: Could not locate input(%p) in mixer(%p)", BSTD_FUNCTION, (void *)input, (void *)handle));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        BDBG_ERR(("%s: Only supported for DSP or Intermediate Mixers", BSTD_FUNCTION));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get Status of Mixer
***************************************************************************/
NEXUS_Error NEXUS_AudioMixer_GetStatus(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioMixerStatus *pStatus
    )
{
    if ( pStatus == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        BERR_Code errCode;
        BAPE_MixerStatus piStatus;
        BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
        errCode = BAPE_Mixer_GetStatus(mixer, &piStatus);
        if ( errCode == BERR_NOT_AVAILABLE )
        {
            return BERR_NOT_AVAILABLE;
        }
        else if ( errCode != BERR_SUCCESS )
        {
            return BERR_TRACE(errCode);
        }

        pStatus->mainDecodeFade.active = piStatus.mainDecodeFade.active;
        pStatus->mainDecodeFade.level = piStatus.mainDecodeFade.level;
        pStatus->mainDecodeFade.remaining = piStatus.mainDecodeFade.remaining;
    }
    else
    {
        BDBG_ERR(("%s: Only supported for DSP or Intermediate Mixers", BSTD_FUNCTION));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

bool NEXUS_AudioMixer_IsInputMaster_priv(
        NEXUS_AudioMixerHandle handle,
        NEXUS_AudioInputHandle input
        )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    return handle->settings.master == input ? true : false;
}

NEXUS_Error NEXUS_AudioMixer_P_SetInputVolume(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    const BAPE_MixerInputVolume *pInputVolume
    )
{
    if ( handle->settings.mixUsingDsp || handle->settings.intermediate )
    {
        NEXUS_Error rc;
        NEXUS_AudioMixerInputNode *pInputNode;

        pInputNode = BLST_Q_FIRST(&handle->inputList);
        while ( NULL != pInputNode && pInputNode->input != input)
        {
            pInputNode = BLST_Q_NEXT(pInputNode, inputNode);
        }
        if (pInputNode != NULL)
        {
            rc = NEXUS_AudioMixer_P_ApplyMixerVolume(handle, input, pInputVolume, pInputNode);
            if ( rc )
            {
                return BERR_TRACE(rc);
            }
        }
        else
        {
            BDBG_ERR(("%s: Could not locate input(%p) in mixer(%p)", BSTD_FUNCTION, (void *)input, (void *)handle));
            return BERR_INVALID_PARAMETER;
        }
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioMixer_P_ApplyMixerVolume(
    NEXUS_AudioMixerHandle handle,
    NEXUS_AudioInputHandle input,
    const BAPE_MixerInputVolume *pInputVolume,
    const NEXUS_AudioMixerInputNode *pInputNode
    )
{
    BERR_Code errCode;
    BAPE_MixerInputVolume combinedVolume;
    BAPE_MixerHandle mixer = handle->settings.mixUsingDsp?handle->dspMixer:handle->imMixer;
    int i,j;

    for ( i = 0; i < NEXUS_AudioChannel_eMax; i++ )
    {
        for ( j = 0; j < NEXUS_AudioChannel_eMax; j++ )
        {
            combinedVolume.coefficients[i][j] =
                (int32_t)(((uint64_t)pInputVolume->coefficients[i][j]  * (uint64_t)pInputNode->inputSettings.volumeMatrix[i][j]) / 0x800000);
        }
    }

    combinedVolume.muted = pInputVolume->muted | pInputNode->inputSettings.muted;
    combinedVolume.fade.level = pInputNode->inputSettings.fade.level;
    combinedVolume.fade.duration = pInputNode->inputSettings.fade.duration;
    combinedVolume.fade.type = pInputNode->inputSettings.fade.type;
    errCode = BAPE_Mixer_SetInputVolume(mixer, (BAPE_Connector)input->port, &combinedVolume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
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

/***************************************************************************
Summary:
    Check mixer state - mixer can be started even if inputs are not started
 ***************************************************************************/
bool NEXUS_AudioMixer_P_IsExplicitlyStarted(
    NEXUS_AudioMixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMixer);
    return handle->explicitlyStarted;
}

NEXUS_AudioMixerHandle
NEXUS_AudioMixer_P_GetMixerByIndex(
    unsigned index
    )
{
    BDBG_ASSERT(index < g_NEXUS_audioModuleData.capabilities.numMixers);
    return g_mixers[index].opened?&g_mixers[index]:NULL;
}
