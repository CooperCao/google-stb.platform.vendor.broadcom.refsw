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
*   Private routines internal to the audio module
*
***************************************************************************/
#include "nexus_audio_module.h"
#include "priv/nexus_audio_output_priv.h"
#if NEXUS_NUM_HDMI_OUTPUTS
#include "nexus_hdmi_output.h"
#include "priv/nexus_hdmi_output_priv.h"
static struct
{
    NEXUS_AudioOutputHandle output;
    BAPE_MaiOutputHandle handle;
    BKNI_EventHandle settingsChangedEvent;
    NEXUS_EventCallbackHandle settingsChangedCallback;
} g_hdmiMapping[NEXUS_NUM_HDMI_OUTPUTS];

static void NEXUS_AudioOutput_P_HdmiSettingsChanged(void *pOutput);
static void NEXUS_AudioOutput_P_HdmiSampleRateChange_isr(void *, int, unsigned);
#define NEXUS_AudioOutput_P_GetSampleRateEnum NEXUS_AudioOutput_P_GetSampleRateEnum_isr
static BAVC_AudioSamplingRate NEXUS_AudioOutput_P_GetSampleRateEnum_isr(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 32000:
        return BAVC_AudioSamplingRate_e32k;    /* 32K Sample rate */
    case 44100:
        return BAVC_AudioSamplingRate_e44_1k;    /* 44.1K Sample rate */
    case 48000:
        return BAVC_AudioSamplingRate_e48k;      /* 48K Sample rate */
    case 96000:
        return BAVC_AudioSamplingRate_e96k;      /* 96K Sample rate */
    case 16000:
        return BAVC_AudioSamplingRate_e16k;      /* 16K Sample rate */
    case 22050:
        return BAVC_AudioSamplingRate_e22_05k;   /* 22.05K Sample rate */
    case 24000:
        return BAVC_AudioSamplingRate_e24k;      /* 24K Sample rate */
    case 64000:
        return BAVC_AudioSamplingRate_e64k;      /* 64K Sample rate */
    case 88200:
        return BAVC_AudioSamplingRate_e88_2k;    /* 88.2K Sample rate */
    case 128000:
        return BAVC_AudioSamplingRate_e128k;     /* 128K Sample rate */
    case 176400:
        return BAVC_AudioSamplingRate_e176_4k;   /* 176.4K Sample rate */
    case 192000:
        return BAVC_AudioSamplingRate_e192k;     /* 192K Sample rate */
    case 8000:
        return BAVC_AudioSamplingRate_e8k;       /* 8K Sample rate */
    case 12000:
        return BAVC_AudioSamplingRate_e12k;      /* 12K Sample rate */
    case 11025:
        return BAVC_AudioSamplingRate_e11_025k;  /* 11.025K Sample rate */
    default:
        return 0;
    }
}
#endif
#if NEXUS_HAS_RFM
#include "priv/nexus_rfm_priv.h"
static struct
{
    NEXUS_AudioOutputHandle output;
    BAPE_RfModHandle handle;
} g_rfmMapping[NEXUS_NUM_RFM_OUTPUTS];
#endif
#if 0 /* TODO */
#include "bchp_vcxo_0_rm.h"
#include "bchp_vcxo_ctl_misc.h"
#endif

#ifndef NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 0
#endif

#define NEXUS_AUDIO_MAX_TIMEBASES  (NEXUS_NUM_HDMI_OUTPUTS + NEXUS_NUM_I2S_OUTPUTS + NEXUS_NUM_AUDIO_DUMMY_OUTPUTS + NEXUS_NUM_SPDIF_OUTPUTS + 1) /* [0] is not used */
#define NEXUS_AUDIO_NO_TIMEBASE  0

BDBG_MODULE(nexus_audio_output);

BDBG_OBJECT_ID(NEXUS_AudioOutputData);

static int32_t NEXUS_AudioOutput_P_Vol2Magnum(NEXUS_AudioVolumeType type, int32_t volume)
{
    if ( type == NEXUS_AudioVolumeType_eDecibel )
    {
        int32_t linearVol;
        int volumeIndex;
        if ( volume > 0 )
        {
            BDBG_ERR(("Currently, amplification is not supported for dB values.  Clipping to 0 dB"));
            return 0;
        }
        else if ( volume < NEXUS_AUDIO_VOLUME_DB_MIN )
        {
            BDBG_ERR(("Clipping out of range volume to minimum"));
            return NEXUS_AUDIO_VOLUME_LINEAR_MIN;
        }
        else if ( volume == NEXUS_AUDIO_VOLUME_DB_MIN )
        {
            return NEXUS_AUDIO_VOLUME_LINEAR_MIN;
        }

        /* Convert dB attenuation to linear gain.  Start by Flipping the sign */
        volume = -volume;

        /* Grab integer portion */
        volumeIndex = volume/100;
        linearVol = NEXUS_Audio_P_ConvertDbToLinear(volumeIndex);
        /* Grab fractional portion */
        volume = volume % 100;
        /* Linearly interpolate between the two levels */
        linearVol -= ((NEXUS_Audio_P_ConvertDbToLinear(volumeIndex)-NEXUS_Audio_P_ConvertDbToLinear(volumeIndex+1))*volume)/100;

        return linearVol;
    }
    else
    {
        if ( volume < NEXUS_AUDIO_VOLUME_LINEAR_MIN )
        {
            BDBG_ERR(("Clipping out of range volume to minimum"));
            return NEXUS_AUDIO_VOLUME_LINEAR_MIN;
        }
#if 0 /* Permitting amplification */
        /* The destination APIs are linear now.  No conversion required!! */
        else if ( volume > NEXUS_AUDIO_VOLUME_LINEAR_NORMAL )
        {
            BDBG_ERR(("This platform does not support amplification.  Volume will be set to normal"));
            return NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
        }
#endif

        return volume;
    }
}

/***************************************************************************
Summary:
    Get default settings for an audio output
See Also:
    NEXUS_AudioOutput_Open
 ***************************************************************************/
void NEXUS_AudioOutput_GetDefaultSettings(
    NEXUS_AudioOutputSettings *pSettings      /* [out] Settings */
    )
{
    BAPE_MixerSettings mixerSettings;

    BAPE_Mixer_GetDefaultSettings(&mixerSettings);

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioOutputSettings));
    pSettings->defaultSampleRate = 48000;
    pSettings->pll = g_NEXUS_audioModuleData.settings.defaultPll;
    pSettings->nco = mixerSettings.outputNco;
    pSettings->leftVolume = NEXUS_AUDIO_VOLUME_DB_NORMAL;
    pSettings->rightVolume = NEXUS_AUDIO_VOLUME_DB_NORMAL;
    pSettings->autoConfigureVcxo = true;
}

/***************************************************************************
Summary:
    Get settings of an audio output
See Also:
    NEXUS_AudioOutput_SetSettings
 ***************************************************************************/
void NEXUS_AudioOutput_GetSettings(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioOutputSettings *pSettings    /* [out] Current Settings */
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(NULL != pSettings);

    if ( NULL == output->pMixerData )
    {
        NEXUS_AudioOutput_GetDefaultSettings(pSettings);
        return;
    }

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    *pSettings = pData->settings;

    /* Success */
}

#if NEXUS_NUM_HDMI_OUTPUTS > 0
NEXUS_Error NEXUS_AudioOutput_P_SetHDMISettings(
    NEXUS_AudioOutputHandle output,
    const NEXUS_AudioOutputSettings *pSettings
    )
{
    NEXUS_Error errCode;
    int i;

    /* Check for an update HDMI (MAI) settings changes */

    for ( i = 0; i < NEXUS_NUM_HDMI_OUTPUTS; i++ )
    {
        if ( g_hdmiMapping[i].output == output )
        {
            BAPE_MaiOutputSettings curMaiOutputSettings, maiOutputSettings;
            NEXUS_HdmiOutputSettings hdmiSettings;
            NEXUS_HdmiOutputStatus hdmiStatus;
            BAPE_MaiOutput_GetSettings(g_hdmiMapping[i].handle, &maiOutputSettings);
            BKNI_Memcpy(&curMaiOutputSettings, &maiOutputSettings, sizeof(maiOutputSettings));

            switch ( pSettings->channelMode )
            {
            default:
            case NEXUS_AudioChannelMode_eStereo:
                maiOutputSettings.stereoMode = BAPE_StereoMode_eLeftRight;
                break;
            case NEXUS_AudioChannelMode_eLeft:
                maiOutputSettings.stereoMode = BAPE_StereoMode_eLeftLeft;
                break;
            case NEXUS_AudioChannelMode_eRight:
                maiOutputSettings.stereoMode = BAPE_StereoMode_eRightRight;
                break;
            case NEXUS_AudioChannelMode_eSwapped:
                maiOutputSettings.stereoMode = BAPE_StereoMode_eRightLeft;
                break;
            }

            /* Remaining settings need to be pulled from the HDMI output */
            NEXUS_HdmiOutput_GetSettings(output->pObjectHandle, &hdmiSettings);
            NEXUS_HdmiOutput_GetStatus(output->pObjectHandle, &hdmiStatus);

            if (hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eAc3])
            {
                maiOutputSettings.loudnessType = BAPE_OutputLoudnessType_Passive;
            }
            else
            {
                maiOutputSettings.loudnessType = BAPE_OutputLoudnessType_Active;
            }

            if (hdmiSettings.loudnessDeviceMode == NEXUS_AudioLoudnessDeviceMode_eActive)
            {
                maiOutputSettings.loudnessType = BAPE_OutputLoudnessType_Active;
            }
            else if (hdmiSettings.loudnessDeviceMode == NEXUS_AudioLoudnessDeviceMode_ePassive)
            {
                maiOutputSettings.loudnessType = BAPE_OutputLoudnessType_Passive;
            }
            maiOutputSettings.channelStatus.professional = hdmiSettings.audioChannelStatusInfo.professionalMode;
            maiOutputSettings.channelStatus.copyright = hdmiSettings.audioChannelStatusInfo.swCopyRight;
            maiOutputSettings.channelStatus.categoryCode = hdmiSettings.audioChannelStatusInfo.categoryCode;
            maiOutputSettings.channelStatus.clockAccuracy = hdmiSettings.audioChannelStatusInfo.clockAccuracy;
            maiOutputSettings.channelStatus.separateLeftRight = hdmiSettings.audioChannelStatusInfo.separateLRChanNum;
            maiOutputSettings.ditherEnabled = hdmiSettings.audioDitherEnabled;
            maiOutputSettings.underflowBurst = hdmiSettings.audioBurstType;
            maiOutputSettings.burstPadding = hdmiSettings.audioBurstPadding;

            if ( BKNI_Memcmp(&curMaiOutputSettings, &maiOutputSettings, sizeof(maiOutputSettings)) != 0 )
            {
                errCode = BAPE_MaiOutput_SetSettings(g_hdmiMapping[i].handle, &maiOutputSettings);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
            }
            break;
        }
    }

    return BERR_SUCCESS;
}
#endif


/***************************************************************************
Summary:
    Set settings of an audio output
See Also:
    NEXUS_AudioOutput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_SetSettings(
    NEXUS_AudioOutputHandle output,
    const NEXUS_AudioOutputSettings *pSettings
    )
{
    BAPE_OutputVolume volume;
    NEXUS_Error errCode;
    NEXUS_AudioOutputSettings defaultSettings;
    NEXUS_AudioOutputData *pData;
    int i;
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    if ( NULL == pSettings )
    {
        NEXUS_AudioOutput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if ( NULL == output->pMixerData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

#if NEXUS_NUM_HDMI_OUTPUTS > 0
    /* refresh prior to setting volume / mute settings, in case burst type changed */
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
    {
        NEXUS_AudioOutput_P_SetHDMISettings(output, pSettings);
    }
#endif

    if ( !(NEXUS_AudioOutput_P_IsDacSlave(output) || NEXUS_AudioOutput_P_IsSpdifSlave(output)) )
    {
        /* We can apply volume immediately.  Do that here. */
        BAPE_GetOutputVolume((BAPE_OutputPort)output->port, &volume);
        for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
        {
            volume.volume[i*2] = NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->leftVolume);
            volume.volume[(i*2)+1] = NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->rightVolume);
        }
        volume.muted = pSettings->muted || pData->compressedMute;
        errCode = BAPE_SetOutputVolume((BAPE_OutputPort)output->port, &volume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    /* Apply channel mode if required */
    if ( output->setChannelMode )
    {
        errCode = output->setChannelMode(output->pObjectHandle, pSettings->channelMode);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* catch invalid delay settings */
    if ( g_NEXUS_audioModuleData.settings.independentDelay &&
        (pSettings->additionalDelay > g_NEXUS_audioModuleData.settings.maxIndependentDelay) )
    {
        BDBG_ERR(("additionalDelay value %d is greater than maxIndependentDelay %d.  Please check NEXUS_AudioModuleSettings.maxIndependentDelay",
            pSettings->additionalDelay,
            g_NEXUS_audioModuleData.settings.maxIndependentDelay));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else if (!g_NEXUS_audioModuleData.settings.independentDelay &&
        pSettings->additionalDelay > 0)
    {
        BDBG_ERR(("additionalDelay value %d was set but maxIndependentDelay was not enabled.  Please check NEXUS_AudioModuleSettings.maxIndependentDelay",
                  pSettings->additionalDelay));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Store settings */
    pData->settings = *pSettings;

    /* Applying remaining settings should be deferred until the next start call if we're running. */
    if ( pData->input )
    {
        NEXUS_AudioInput_P_OutputSettingsChanged(pData->input, output);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Add an input to this output
See Also:
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_AddInput(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    if ( NULL == output->pMixerData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if ( pData->input != NULL )
    {
        BDBG_WRN(("Input %p is already connected to this output", (void *)output->pMixerData));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Input %p(%s) is running.  Please stop first.", (void *)input, input->pName));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = NEXUS_AudioInput_P_ConnectOutput(input, output);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pData->input = input;

#if BDBG_DEBUG_BUILD
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi || output->objectType == NEXUS_AudioOutputType_eSpdif )
    {
        NEXUS_AudioDebug_AddOutput((void *)output->port);

    }
#endif

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Add an input to this output
See Also:
    NEXUS_AudioOutput_AddInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_RemoveInput(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    pData = output->pMixerData;

    if ( NULL == pData || input != pData->input )
    {
        BDBG_ERR(("Input %p (%s) is not connected to this output %p (%s).", (void *)input, input->pName, (void *)output, output->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Forcing input %p(%s) to stop on output %p (%s) shutdown", (void *)input, input->pName, (void *)output, output->pName));
        NEXUS_AudioInput_P_ForceStop(input);
    }

#if BDBG_DEBUG_BUILD
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi || output->objectType == NEXUS_AudioOutputType_eSpdif )
    {
        NEXUS_AudioDebug_RemoveOutput((void *)output->port);
    }
#endif

    errCode = NEXUS_AudioInput_P_DisconnectOutput(input, output);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pData->input = NULL;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Add an input to this output
See Also:
    NEXUS_AudioOutput_AddInput
    NEXUS_AudioOutput_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_RemoveAllInputs(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_ASSERT(NULL != output);

    pData = output->pMixerData;
    if ( pData && NULL != pData->input )
    {
        NEXUS_Error errCode;

        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        errCode = NEXUS_AudioOutput_RemoveInput(output, pData->input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;;
}

/***************************************************************************
Summary:
    Shutdown this output handle
Description:
    This routine should be called before the specific output object
    (e.g. AudioDac) is closed.
See Also:
    NEXUS_AudioOutput_AddInput
    NEXUS_AudioOutput_RemoveInput
 ***************************************************************************/
void NEXUS_AudioOutput_Shutdown(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( pData )
    {
        NEXUS_Error rc;

        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

        rc = NEXUS_AudioOutput_RemoveAllInputs(output);
        if (rc) {
            /* we cannot complete the shutdown */
            return;
        }
        if ( pData->equalizer )
        {
            rc = NEXUS_AudioOutput_SetEqualizer(output, NULL);
            if (rc) {
                /* we cannot complete the shutdown */
                return;
            }
        }

        BDBG_OBJECT_DESTROY(pData, NEXUS_AudioOutputData);
        BKNI_Free(pData);

        /* TODO: Handle RFM object handles */
        switch ( output->objectType )
        {
#if NEXUS_NUM_HDMI_OUTPUTS
        case NEXUS_AudioOutputType_eHdmi:
            {
                int i;
                for ( i = 0; i < NEXUS_NUM_HDMI_OUTPUTS; i++ )
                {
                    if ( g_hdmiMapping[i].output == output )
                    {
                        output->port = 0;
                        BDBG_MSG(("Closing MAI output"));
                        BAPE_MaiOutput_Close(g_hdmiMapping[i].handle);

                        NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);
                        NEXUS_HdmiOutput_SetNotifyAudioEvent_priv(output->pObjectHandle, NULL);
                        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);

                        NEXUS_UnregisterEvent(g_hdmiMapping[i].settingsChangedCallback);
                        BKNI_DestroyEvent(g_hdmiMapping[i].settingsChangedEvent);
                        BKNI_Memset(&g_hdmiMapping[i], 0, sizeof(g_hdmiMapping[i]));
                        break;
                    }
                }
                BDBG_ASSERT(i < NEXUS_NUM_HDMI_OUTPUTS);
            }
            break;
#endif
#if NEXUS_HAS_RFM
        case NEXUS_AudioOutputType_eRfm:
            {
                int i;
                for ( i = 0; i < NEXUS_NUM_RFM_OUTPUTS; i++ )
                {
                    if ( g_rfmMapping[i].output == output )
                    {
                        output->port = 0;
                        BDBG_MSG(("Closing RFM output"));
                        BAPE_RfMod_Close(g_rfmMapping[i].handle);
                        g_rfmMapping[i].output = NULL;
                        g_rfmMapping[i].handle = NULL;
                        break;
                    }
                }
                BDBG_ASSERT(i < NEXUS_NUM_RFM_OUTPUTS);
            }
            break;
#endif
        default:
            break;
        }
    }
    output->pMixerData = NULL;

    return ;
}

/***************************************************************************
Summary:
    Determine if an output must be tied to a DAC
Description:
    Some outputs (i.e. RFM) are slaves to a DAC.
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsDacSlave(
    NEXUS_AudioOutputHandle output
    )
{
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    return (output->objectType == NEXUS_AudioOutputType_eRfm) ? true : false;
}

/***************************************************************************
Summary:
    Determine if an output must be tied to a SPDIF port
Description:
    Some outputs (i.e. ARC) are slaves to a SPDIF port.
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsSpdifSlave(
    NEXUS_AudioOutputHandle output
    )
{
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    return (output->objectType == NEXUS_AudioOutputType_eArc) ? true : false;
}

/***************************************************************************
Summary:
    For slave outputs, set the DAC source
Description:
    Some outputs (i.e. RFM) are slaves to a DAC.  Mixer will find the proper
    DAC to bind the output to and provide it here.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetSlaveSource(
    NEXUS_AudioOutputHandle slaveHandle,
    NEXUS_AudioOutputHandle sourceHandle)
{
    BDBG_OBJECT_ASSERT(slaveHandle, NEXUS_AudioOutput);
    BSTD_UNUSED(sourceHandle);  /* In case neither of the cases below are defined */

    if ( NEXUS_AudioOutput_P_IsDacSlave(slaveHandle) )
    {
        #if NEXUS_HAS_RFM
        NEXUS_RfmConnectionSettings rfmConnectionSettings;
        bool enabled=false;
        NEXUS_Error errCode;
        BAPE_RfModHandle apeRfmHandle=NULL;
        BAPE_RfModSettings apeRfmSettings;
        int i;

        if ( NULL == g_NEXUS_audioModuleData.internalSettings.modules.rfm )
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        for ( i = 0; i < NEXUS_NUM_RFM_OUTPUTS; i++ )
        {
            if ( g_rfmMapping[i].output == slaveHandle )
            {
                apeRfmHandle = g_rfmMapping[i].handle;
                break;
            }
        }

        BDBG_ASSERT(NULL != apeRfmHandle);

        BAPE_RfMod_GetSettings(apeRfmHandle, &apeRfmSettings);

        if ( NULL == sourceHandle )
        {
            apeRfmSettings.master = NULL;
            apeRfmSettings.muted = true;
        }
        else
        {
            BDBG_ASSERT(sourceHandle->objectType == NEXUS_AudioOutputType_eDac);
            enabled = true;
            apeRfmSettings.master = (BAPE_OutputPort)sourceHandle->port;
            apeRfmSettings.muted = false;
        }

        errCode = BAPE_RfMod_SetSettings(apeRfmHandle, &apeRfmSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Tell RFM to mute or unmute appropriately */
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.rfm);
        NEXUS_Rfm_GetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
        rfmConnectionSettings.audioEnabled = enabled;
        errCode = NEXUS_Rfm_SetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.rfm);

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        #endif
    }
    else if ( NEXUS_AudioOutput_P_IsSpdifSlave(slaveHandle) )
    {
        #if NEXUS_NUM_AUDIO_RETURN_CHANNEL
            NEXUS_Error errCode;

            errCode = NEXUS_AudioReturnChannel_P_SetMaster( slaveHandle, sourceHandle );

            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        #endif
    }

    return BERR_SUCCESS;
}

NEXUS_AudioOutputData *NEXUS_AudioOutput_P_CreateData(NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioOutputData *pData;

    if ( NULL == output->pMixerData )
    {
        output->pMixerData = pData = BKNI_Malloc(sizeof(NEXUS_AudioOutputData));
        if ( pData )
        {
            BKNI_Memset(pData, 0, sizeof(NEXUS_AudioOutputData));
            BDBG_OBJECT_SET(pData, NEXUS_AudioOutputData);
            NEXUS_AudioOutput_GetDefaultSettings( &pData->settings);
            pData->sampleRate = BAVC_AudioSamplingRate_eUnknown;
            switch ( output->objectType )
            {
            #if NEXUS_NUM_HDMI_OUTPUTS  /* TODO */
            case NEXUS_AudioOutputType_eHdmi:
            {
                NEXUS_HdmiOutputStatus hdmiStatus;
                NEXUS_HdmiOutput_GetStatus(output->pObjectHandle, &hdmiStatus);
                    BDBG_ASSERT(g_hdmiMapping[hdmiStatus.index].output == NULL);
                    {
                        BERR_Code errCode;
                        BAPE_OutputPort mixerOutput;
                        BAPE_MaiOutputInterruptHandlers interrupts;

                        BDBG_MSG(("Creating MAI handle"));
                        errCode = BAPE_MaiOutput_Open(NEXUS_AUDIO_DEVICE_HANDLE, hdmiStatus.index, NULL, &g_hdmiMapping[hdmiStatus.index].handle);
                        if ( errCode )
                        {
                            BDBG_ERR(("Unable to open MAI Output"));
                            BKNI_Free(output->pMixerData);
                            output->pMixerData = NULL;
                            return NULL;
                        }

                        errCode = BKNI_CreateEvent(&g_hdmiMapping[hdmiStatus.index].settingsChangedEvent);
                        if ( errCode )
                        {
                            errCode = BERR_TRACE(errCode);
                            BAPE_MaiOutput_Close(g_hdmiMapping[hdmiStatus.index].handle);
                            BKNI_Free(output->pMixerData);
                            output->pMixerData = NULL;
                            return NULL;
                        }

                        g_hdmiMapping[hdmiStatus.index].settingsChangedCallback = NEXUS_RegisterEvent(g_hdmiMapping[hdmiStatus.index].settingsChangedEvent, NEXUS_AudioOutput_P_HdmiSettingsChanged, output);
                        if ( NULL == g_hdmiMapping[hdmiStatus.index].settingsChangedCallback )
                        {
                            errCode = BERR_TRACE(errCode);
                            BKNI_DestroyEvent(g_hdmiMapping[hdmiStatus.index].settingsChangedEvent);
                            BAPE_MaiOutput_Close(g_hdmiMapping[hdmiStatus.index].handle);
                            BKNI_Free(output->pMixerData);
                            output->pMixerData = NULL;
                            return NULL;
                        }

                        BAPE_MaiOutput_GetInterruptHandlers(g_hdmiMapping[hdmiStatus.index].handle, &interrupts);
                        interrupts.sampleRate.pCallback_isr = NEXUS_AudioOutput_P_HdmiSampleRateChange_isr;
                        interrupts.sampleRate.pParam1 = output;
                        interrupts.sampleRate.param2 = 0;
                        errCode = BAPE_MaiOutput_SetInterruptHandlers(g_hdmiMapping[hdmiStatus.index].handle, &interrupts);
                        if ( errCode )
                        {
                            BDBG_ERR(("Unable to register sample rate interrupt"));
                            NEXUS_UnregisterEvent(g_hdmiMapping[hdmiStatus.index].settingsChangedCallback);
                            BKNI_DestroyEvent(g_hdmiMapping[hdmiStatus.index].settingsChangedEvent);
                            BAPE_MaiOutput_Close(g_hdmiMapping[hdmiStatus.index].handle);
                            BKNI_Free(output->pMixerData);
                            output->pMixerData = NULL;
                            return NULL;
                        }
                        g_hdmiMapping[hdmiStatus.index].output = output;
                        BAPE_MaiOutput_GetOutputPort(g_hdmiMapping[hdmiStatus.index].handle, &mixerOutput);
                        output->port = (size_t)mixerOutput;

                        NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);
                        NEXUS_HdmiOutput_SetNotifyAudioEvent_priv(output->pObjectHandle, g_hdmiMapping[hdmiStatus.index].settingsChangedEvent);
                        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);
                        NEXUS_AudioOutput_P_HdmiSettingsChanged(output);
                    }
            }
                break;
            #endif
            #if NEXUS_HAS_RFM
            case NEXUS_AudioOutputType_eRfm:
                #if NEXUS_NUM_RFM_OUTPUTS != 1
                #error More than one RFM output is not supported
                #endif
                BDBG_ASSERT(g_rfmMapping[0].output == NULL);
                {
                    BERR_Code errCode;
                    BDBG_MSG(("Creating RFM handle"));
                    errCode = BAPE_RfMod_Open(NEXUS_AUDIO_DEVICE_HANDLE, 0, NULL, &g_rfmMapping[0].handle);
                    if ( errCode )
                    {
                        BDBG_ERR(("Unable to open MAI Output"));
                        BKNI_Free(output->pMixerData);
                        output->pMixerData = NULL;
                        return NULL;
                    }
                    g_rfmMapping[0].output = output;
                    output->port = 0;
                }
                break;
            #endif
            default:
                break;
            }
        }
        else
        {
            NEXUS_Error errCode;
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
    }

    return output->pMixerData;
}

void NEXUS_AudioOutput_GetSyncSettings_priv( NEXUS_AudioOutputHandle audioOutput, NEXUS_AudioOutputSyncSettings *pSyncSettings )
{
    NEXUS_AudioOutputData *pData = audioOutput->pMixerData;
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    *pSyncSettings = pData->syncSettings;
}

NEXUS_Error NEXUS_AudioOutput_SetSyncSettings_priv( NEXUS_AudioOutputHandle audioOutput, const NEXUS_AudioOutputSyncSettings *pSyncSettings )
{
    NEXUS_AudioOutputData *pData = audioOutput->pMixerData;

    NEXUS_ASSERT_MODULE();
    if (!pData) return 0; /* TODO */

    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    pData->syncSettings = *pSyncSettings;

    BKNI_EnterCriticalSection();
    /* fire right away. always "on". */
    if (pSyncSettings->startCallback_isr) {
        (*pSyncSettings->startCallback_isr)(pSyncSettings->callbackContext, 0);
    }
    BKNI_LeaveCriticalSection();

    /* set delay + additionalDelay */
    if ( !g_NEXUS_audioModuleData.settings.independentDelay &&
         (pData->settings.additionalDelay || pData->syncSettings.delay) )
    {
        BDBG_ERR(("Independent output delay is not enabled, ignoring delay.  Please check NEXUS_AudioModuleSettings.independentDelay"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Applying remaining settings should be deferred until the next start call if we're running. */
    if ( pData->input )
    {
        NEXUS_AudioInput_P_OutputSettingsChanged(pData->input, audioOutput);
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioOutput_GetSyncStatus_isr( NEXUS_AudioOutputHandle audioOutput, NEXUS_AudioOutputSyncStatus *pSyncStatus )
{

    BDBG_ASSERT(NULL != audioOutput);
    BDBG_ASSERT(NULL != pSyncStatus);

    /* Default to PCM */
    pSyncStatus->compressed = false;

    /* Look for connected input.  If found, determine format from AudioInput. */
    if ( NULL != audioOutput->pMixerData )
    {
        NEXUS_AudioOutputData *pData;

        pData = audioOutput->pMixerData;
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        if ( pData->input != NULL )
        {
            NEXUS_AudioInputFormat format = NEXUS_AudioInput_P_GetFormat(pData->input);

            if ( format == NEXUS_AudioInputFormat_eCompressed )
            {
                pSyncStatus->compressed = true;
            }
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioOutput_GetStatus( NEXUS_AudioOutputHandle audioOutput, NEXUS_AudioOutputStatus *pStatus )
{
    NEXUS_AudioOutputData *pData = audioOutput->pMixerData;
    if (!pData) {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    }
    else {
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        *pStatus = pData->status;
    }
    return 0;
}

NEXUS_Error NEXUS_AudioOutput_P_SetCompressedMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    )
{
    BAPE_OutputVolume volume;
    NEXUS_AudioOutputData *pData;
    NEXUS_Error errCode;

    if ( NEXUS_AudioOutput_P_IsDacSlave(output) ||
         NEXUS_AudioOutput_P_IsSpdifSlave(output) )
    {
        return NEXUS_SUCCESS;
    }

    if ( NULL == output->pMixerData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if (output->port != 0)
    {
        BAPE_GetOutputVolume((BAPE_OutputPort)output->port, &volume);
        volume.muted = pData->settings.muted || mute;
        errCode = BAPE_SetOutputVolume((BAPE_OutputPort)output->port, &volume);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        BDBG_MSG(("audio output %s %s", output->pName, mute ? "muted" : "unmuted"));
    }

    pData->compressedMute = mute;
    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Determine if an output is connected to any inputs
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsConnected(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_ASSERT(NULL != output);

    pData = output->pMixerData;
    if ( NULL != pData && NULL != pData->input )
    {
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        return true;
    }

    return false;
}

#if NEXUS_NUM_HDMI_OUTPUTS
static void NEXUS_AudioOutput_P_HdmiSampleRateChange_isr(void *pParam1, int param2, unsigned sampleRate)
{
    BAVC_AudioSamplingRate avcRate;
    NEXUS_AudioOutputHandle output;
    NEXUS_AudioOutputData *pData;

    BSTD_UNUSED(param2);


    avcRate = NEXUS_AudioOutput_P_GetSampleRateEnum_isr(sampleRate);
    output=(NEXUS_AudioOutputHandle)pParam1;
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_MSG(("%s - HDMI Sample Rate Change %u, output %p", __FUNCTION__, sampleRate, (void *)output));
    pData = output->pMixerData;
    BDBG_ASSERT(NULL != pData);
    pData->sampleRate = avcRate;

    NEXUS_HdmiOutput_AudioSampleRateChange_isr(output->pObjectHandle, avcRate);
}

static void NEXUS_AudioOutput_P_HdmiSettingsChanged(void *pOutput)
{
    NEXUS_AudioOutputHandle output = (NEXUS_AudioOutput)pOutput;
    NEXUS_AudioOutputSettings outputSettings;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(output->objectType == NEXUS_AudioOutputType_eHdmi);

    BDBG_MSG(("HDMI audio settings have changed.  Updating."));

    /* this will force the settings to be applied to the PI */
    NEXUS_AudioOutput_GetSettings(output, &outputSettings);
    NEXUS_AudioOutput_SetSettings(output, &outputSettings);
}
#endif

void NEXUS_AudioOutput_P_GetMixerSettings(NEXUS_AudioOutputHandle output, BAPE_MixerSettings *pSettings /* [out] */)
{
    NEXUS_AudioOutputData *pData;
    NEXUS_Error errCode;
    unsigned timebaseIndex;

    BDBG_ASSERT(NULL != output);

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    BAPE_Mixer_GetDefaultSettings(pSettings);
    pSettings->defaultSampleRate = pData->settings.defaultSampleRate;
    pSettings->outputPll = pData->settings.pll;
    pSettings->outputNco = pData->settings.nco;
    errCode = NEXUS_Timebase_GetIndex(pData->settings.timebase, &timebaseIndex);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        timebaseIndex = 0;
    }
    pSettings->outputTimebase = timebaseIndex;
    if ( NULL == NEXUS_GetEnv("no_independent_delay") )
    {
        pSettings->outputDelay = pData->settings.additionalDelay;
    }
}

void NEXUS_AudioOutput_P_SetOutputFormat(NEXUS_AudioOutputHandle output, NEXUS_AudioInputFormat format)
{
    BDBG_ASSERT(NULL != output);
#if NEXUS_NUM_HDMI_OUTPUTS
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
    {
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);
        {
            BAVC_AudioSamplingRate sampleRate = BAVC_AudioSamplingRate_e48k;
            NEXUS_AudioOutputData *pData = output->pMixerData;
            unsigned numberOfChannels;

            switch ( format )
            {
            case NEXUS_AudioInputFormat_ePcm5_1:
                numberOfChannels = 6;
                break;
            case NEXUS_AudioInputFormat_ePcm7_1:
                numberOfChannels = 8;
                break;
            default:
                numberOfChannels = 2;
                break;
            }

            if ( pData )
            {
                if ( pData->sampleRate == BAVC_AudioSamplingRate_eUnknown )
                {
                sampleRate = NEXUS_AudioOutput_P_GetSampleRateEnum(pData->settings.defaultSampleRate);
                }
                else
                {
                    sampleRate = pData->sampleRate;
                }
            }
            BDBG_MSG(("Set HDMI audio format to %s - %u channels", format==NEXUS_AudioInputFormat_eCompressed?"Compressed":"PCM",numberOfChannels));
            /* Eventually, audio decoder will need to set the proper compressed format */
            NEXUS_HdmiOutput_SetAudioParams_priv(output->pObjectHandle, BAVC_AudioBits_e16, sampleRate,
                                                 format == NEXUS_AudioInputFormat_eCompressed?BAVC_AudioFormat_eAC3:BAVC_AudioFormat_ePCM,numberOfChannels);
        }
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.hdmiOutput);
    }
#else
    BSTD_UNUSED(output);
    BSTD_UNUSED(format);
#endif
}

bool NEXUS_AudioOutput_P_IsRunning(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL == pData )
    {
        /* No connection can have been made, so we can't be running */
        return false;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    if ( NULL == pData->input )
    {
        /* There is no input, can't be running */
        return false;
    }

    /* We're running if our input is running. */
    return NEXUS_AudioInput_P_IsRunning(pData->input);
}

void NEXUS_AudioOutput_GetDefaultEnabledOutputs(
    NEXUS_AudioOutputEnabledOutputs *pOutputs)
{
    BKNI_Memset(pOutputs, 0, sizeof(NEXUS_AudioOutputEnabledOutputs));
}

NEXUS_Error NEXUS_AudioOutput_P_ValidateClockConfig(
    unsigned outputFamily,
    NEXUS_AudioOutputClockSource *config,
    NEXUS_AudioOutputClockSource *suggestions,
    NEXUS_AudioCapabilities *capabilities,
    unsigned *allocatedPll,
    unsigned *allocatedNco,
    bool supportsPll,
    bool supportsNco,
    char *output)
{    

    if (outputFamily == NEXUS_AUDIO_NO_TIMEBASE)
    {
        return BERR_SUCCESS;
    }

    if (outputFamily >= NEXUS_AUDIO_MAX_TIMEBASES)
    {        
        BDBG_ERR(("%s output %s outputFamily(%d) exceeds NEXUS_AUDIO_MAX_TIMEBASES (%d)",__FUNCTION__,output,outputFamily,NEXUS_AUDIO_MAX_TIMEBASES));
        return BERR_TRACE(BERR_INVALID_PARAMETER);         
    }

    if (supportsNco && outputFamily && suggestions[outputFamily].nco == NEXUS_AudioOutputNco_eMax &&
        suggestions[outputFamily].pll == NEXUS_AudioOutputPll_eMax && *allocatedNco < capabilities->numNcos)
    {
        config->nco = NEXUS_AudioOutputNco_e0+*allocatedNco;
        suggestions[outputFamily].nco = NEXUS_AudioOutputNco_e0+*allocatedNco;
        *allocatedNco += 1;
        return BERR_SUCCESS;
    }
    else if (supportsNco && outputFamily && suggestions[outputFamily].nco != NEXUS_AudioOutputNco_eMax)
    {
        config->nco = suggestions[outputFamily].nco;
        return BERR_SUCCESS;
    }
    else if (supportsPll && outputFamily && suggestions[outputFamily].pll == NEXUS_AudioOutputPll_eMax &&
        suggestions[outputFamily].nco == NEXUS_AudioOutputNco_eMax && *allocatedPll < capabilities->numPlls)
    {
        config->pll = NEXUS_AudioOutputPll_e0+*allocatedPll;
        suggestions[outputFamily].pll = NEXUS_AudioOutputPll_e0+*allocatedPll;
        *allocatedPll += 1; 
        return BERR_SUCCESS;
    } 
    else if (supportsPll && outputFamily && suggestions[outputFamily].pll != NEXUS_AudioOutputPll_eMax)
    {
        config->pll = suggestions[outputFamily].pll;
        return BERR_SUCCESS;
    }

    BDBG_ERR(("%s: could not allocate a resource for %s:",__FUNCTION__,output));
    BDBG_ERR(("output supports nco %d - output supports pll %d", supportsNco, supportsPll));
    BDBG_ERR(("NCOs allocated %d - PLL's allocated %d", *allocatedNco, *allocatedPll));
    BDBG_ERR(("System Supports NCOs %d PLL %d Outputs %d", capabilities->numNcos, capabilities->numPlls, NEXUS_AUDIO_MAX_TIMEBASES));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}    


NEXUS_Error NEXUS_AudioOutput_CreateClockConfig(
    const NEXUS_AudioOutputEnabledOutputs *outputs,
    NEXUS_AudioOutputClockConfig *config)
{     
     NEXUS_AudioCapabilities capabilities;
     unsigned totalResources = 0;
     unsigned totalRequests = 0;
     unsigned requestedPll = 0;
     unsigned requestedSpdif = 0;
     unsigned requestedI2S = 0;
     unsigned requestedMultiI2S = 0;
     unsigned requstedDummy = 0;
     unsigned requestedHDMI = 0;
     unsigned allocatedPll = 0;
     unsigned allocatedNco = 0;
     unsigned i;
     char outputName[32];
     NEXUS_Error errCode = BERR_SUCCESS;
     NEXUS_AudioOutputClockSource suggestions[NEXUS_AUDIO_MAX_TIMEBASES];

     #if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
        BDBG_CASSERT((int)NEXUS_MAX_AUDIO_DUMMY_OUTPUTS>=(int)NEXUS_NUM_AUDIO_DUMMY_OUTPUTS);
     #endif
     #if NEXUS_NUM_SPDIF_OUTPUTS
        BDBG_CASSERT((int)NEXUS_MAX_AUDIO_SPDIF_OUTPUTS>=(int)NEXUS_NUM_SPDIF_OUTPUTS);
     #endif
     #if NEXUS_NUM_DAC_OUTPUTS
        BDBG_CASSERT((int)NEXUS_MAX_AUDIO_DAC_OUTPUTS>=(int)NEXUS_NUM_DAC_OUTPUTS);
     #endif
     #if NEXUS_NUM_I2S_OUTPUTS
        BDBG_CASSERT((int)NEXUS_MAX_AUDIO_I2S_OUTPUTS>=(int)NEXUS_NUM_I2S_OUTPUTS);
     #endif
     #if NEXUS_NUM_HDMI_OUTPUTS
        BDBG_CASSERT((int)NEXUS_MAX_AUDIO_HDMI_OUTPUTS>=(int)NEXUS_NUM_HDMI_OUTPUTS);
     #endif

     NEXUS_GetAudioCapabilities(&capabilities);

     totalResources = capabilities.numNcos + capabilities.numPlls;
     for (i=0; i<NEXUS_AUDIO_MAX_TIMEBASES; i++)
     {
         suggestions[i].pll = NEXUS_AudioOutputPll_eMax;
         suggestions[i].nco = NEXUS_AudioOutputNco_eMax;
     }

#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS > 0
     for(i=0;i<NEXUS_NUM_AUDIO_DUMMY_OUTPUTS;i++)
     {
         requstedDummy += outputs->audioDummy[i]?1:0;
         config->audioDummy[i].pll=NEXUS_AudioOutputPll_eMax;
         config->audioDummy[i].nco=NEXUS_AudioOutputNco_eMax;

     }
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS > 0
     for(i=0;i<NEXUS_NUM_SPDIF_OUTPUTS;i++)
     {
         requestedSpdif += outputs->spdif[i]?1:0;
         config->spdif[i].pll=NEXUS_AudioOutputPll_eMax;
         config->spdif[i].nco=NEXUS_AudioOutputNco_eMax;
     }
#endif
#if NEXUS_NUM_I2S_OUTPUTS > 0
     for(i=0;i<NEXUS_NUM_I2S_OUTPUTS;i++)
     {
         requestedI2S += outputs->i2s[i]?1:0;         
         config->i2s[i].pll=NEXUS_AudioOutputPll_eMax;
         config->i2s[i].nco=NEXUS_AudioOutputNco_eMax;
     }      
#endif
#if NEXUS_NUM_I2S_MULTI_OUTPUTS > 0
     for(i=0;i<NEXUS_NUM_I2S_MULTI_OUTPUTS;i++)
     {
         requestedMultiI2S += outputs->i2sMulti[i]?1:0;             
         config->i2sMulti[i].pll=NEXUS_AudioOutputPll_eMax;
         config->i2sMulti[i].nco=NEXUS_AudioOutputNco_eMax;
     }        
#endif
#if NEXUS_NUM_HDMI_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_HDMI_OUTPUTS;i++)
     {
         requestedHDMI += outputs->hdmi[i]?1:0;
         config->hdmi[i].pll=NEXUS_AudioOutputPll_eMax;
         config->hdmi[i].nco=NEXUS_AudioOutputNco_eMax;
     }
#endif
        totalRequests = requestedHDMI + requestedMultiI2S + requestedI2S + requestedSpdif + requstedDummy;
        requestedPll = requestedMultiI2S + requestedI2S + requestedSpdif;
        BDBG_MSG(("Resources available %d", totalResources));
        BDBG_MSG(("Requested HDMI(%d) Spdif(%d) I2S (%d) MultiI2S (%d) Dummy(%d)",
            requestedHDMI, requestedSpdif, requestedI2S, requestedMultiI2S, requstedDummy));

#if NEXUS_NUM_SPDIF_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_SPDIF_OUTPUTS;i++)
        {
            BKNI_Snprintf (outputName, sizeof(outputName), "SPDIF %u", i);
            errCode |= NEXUS_AudioOutput_P_ValidateClockConfig(
                outputs->spdif[i],
                &config->spdif[i],
                &suggestions[0],
                &capabilities,
                &allocatedPll,
                &allocatedNco,
                true,
                false,
                outputName);
        }
#endif
            
#if NEXUS_NUM_I2S_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_I2S_OUTPUTS;i++)
        {
            BKNI_Snprintf (outputName, sizeof(outputName), "I2S %u", i);
            errCode |= NEXUS_AudioOutput_P_ValidateClockConfig(
                outputs->i2s[i],
                &config->i2s[i],
                &suggestions[0],
                &capabilities,
                &allocatedPll,
                &allocatedNco,
                true,
                false,
                outputName);
        }
#endif
#if NEXUS_NUM_I2S_MULTI_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_I2S_MULTI_OUTPUTS;i++)
        {
            BKNI_Snprintf (outputName, sizeof(outputName), "MULTI I2S %u", i);
            errCode |= NEXUS_AudioOutput_P_ValidateClockConfig(
                outputs->i2sMulti[i],
                &config->i2sMulti[i],
                &suggestions[0],
                &capabilities,
                &allocatedPll,
                &allocatedNco,
                true,
                false,
                outputName);
        }
#endif
#if  NEXUS_NUM_HDMI_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_HDMI_OUTPUTS;i++)
        {
            BKNI_Snprintf (outputName, sizeof(outputName), "HDMI %u", i);
            errCode |= NEXUS_AudioOutput_P_ValidateClockConfig(
                outputs->hdmi[i],
                &config->hdmi[i],
                &suggestions[0],
                &capabilities,
                &allocatedPll,
                &allocatedNco,
                true,
                true,
                outputName);

        }
#endif
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS > 0
        for(i=0;i<NEXUS_NUM_AUDIO_DUMMY_OUTPUTS;i++)
        {
            BKNI_Snprintf (outputName, sizeof(outputName), "DUMMY %u", i);
            errCode |= NEXUS_AudioOutput_P_ValidateClockConfig(
                outputs->audioDummy[i],
                &config->audioDummy[i],
                &suggestions[0],
                &capabilities,
                &allocatedPll,
                &allocatedNco,
                true,
                true,
                outputName);
        }
#endif

    return errCode;
}
