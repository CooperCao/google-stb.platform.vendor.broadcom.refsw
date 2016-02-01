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
*   Private routines internal to the audio module
*
* Revision History:
*
* $brcm_Log: $
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
} g_hdmiMapping[NEXUS_NUM_HDMI_OUTPUTS];

static void NEXUS_AudioOutput_P_HdmiSampleRateChange_isr(void *, int, BAVC_AudioSamplingRate);
#endif
#if NEXUS_NUM_RFM_OUTPUTS
#include "priv/nexus_rfm_priv.h"
static struct
{
    NEXUS_AudioOutputHandle output;
    BAPE_RfModHandle handle;
} g_rfmMapping[NEXUS_NUM_RFM_OUTPUTS];
#endif
#include "bchp_vcxo_0_rm.h"
#include "bchp_vcxo_ctl_misc.h"

BDBG_MODULE(nexus_audio_output);

BDBG_OBJECT_ID(NEXUS_AudioOutputData);


/******************************************************************************
The array to represent the value of volume in hex corresponding to the value
in DB. The application inputs the volume in terms of DB and the Corresponding
HEX value is mentioned here. The formula used for the same is:

    HEX = (2^23) * 10^(DB/20)

Note: 23 is the number of bits in the volume control field.

The volume can range from 0-1. 0 in hex corresponds to the 139 DB from the above
Formula. If application enters more than this value, it is forced to 0.
******************************************************************************/
static const int32_t g_db2linear[] =
{
    0x800000,   0x721482,   0x65AC8C,   0x5A9DF7,   0x50C335,
    0x47FACC,   0x4026E7,   0x392CED,   0x32F52C,   0x2D6A86,
    0x287A26,   0x241346,   0x2026F3,   0x1CA7D7,   0x198A13,
    0x16C310,   0x144960,   0x12149A,   0x101D3F,   0xE5CA1,
    0xCCCCC,    0xB6873,    0xA2ADA,    0x90FCB,    0x81385,
    0x732AE,    0x66A4A,    0x5B7B1,    0x51884,    0x48AA7,
    0x40C37,    0x39B87,    0x33718,    0x2DD95,    0x28DCE,
    0x246B4,    0x20756,    0x1CEDC,    0x19C86,    0x16FA9,
    0x147AE,    0x1240B,    0x10449,    0xE7FA,     0xCEC0,
    0xB844,     0xA43A,     0x925E,     0x8273,     0x7443,
    0x679F,     0x5C5A,     0x524F,     0x495B,     0x4161,
    0x3A45,     0x33EF,     0x2E49,     0x2940,     0x24C4,
    0x20C4,     0x1D34,     0x1A07,     0x1732,     0x14AC,
    0x126D,     0x106C,     0xEA3,      0xD0B,      0xBA0,
    0xA5C,      0x93C,      0x83B,      0x755,      0x689,
    0x5D3,      0x531,      0x4A0,      0x420,      0x3AD,
    0x346,      0x2EB,      0x29A,      0x251,      0x211,
    0x1D7,      0x1A4,      0x176,      0x14D,      0x129,
    0xEC,       0xD2,       0xA7,       0x95,       0x84,
    0x76,       0x69,       0x5E,       0x53,       0x4A,
    0x42,       0x3B,       0x34,       0x2F,       0x2A,
    0x25,       0x21,       0x1D,       0x1A,       0x17,
    0x15,       0x12,       0x10,       0xE,        0xD,
    0xB,        0xA,        0x9,        0x8,        0x7,
    0x6,        0x5,        0x5,        0x4,        0x4,
    0x3,        0x3,        0x2,        0x2,        0x2,
    0x2,        0x1,        0x1,        0x1,        0x1,
    0x1,        0x1,        0x1,        0x0
};

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
        linearVol = g_db2linear[volumeIndex];
        /* Grab fractional portion */
        volume = volume % 100;
        /* Linearly interpolate between the two levels */
        linearVol -= ((g_db2linear[volumeIndex]-g_db2linear[volumeIndex+1])*volume)/100;

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

static NEXUS_AudioOutputData *NEXUS_AudioOutput_P_CreateData(NEXUS_AudioOutputHandle output);

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
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_AudioOutputSettings));
    pSettings->defaultSampleRate = 48000;
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

    BDBG_ASSERT(NULL != output);
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
    BDBG_ASSERT(NULL != output);

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

    /* We can apply volume immediately.  Do that here. */
    BAPE_GetOutputVolume((BAPE_MixerOutput)output->port, &volume);
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        volume.volume[i*2] = NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->leftVolume);
        volume.volume[(i*2)+1] = NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->rightVolume);
    }
    volume.muted = pSettings->muted || pData->trickMute;
    errCode = BAPE_SetOutputVolume((BAPE_MixerOutput)output->port, &volume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
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
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputData *pData;

    BDBG_ASSERT(NULL != output);
    BDBG_ASSERT(NULL != input);

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
        BDBG_WRN(("Input %p is already connected to this output", output->pMixerData));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Input %p is running.  Please stop first.", input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = NEXUS_AudioInput_P_ConnectOutput(input, output);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pData->input = input;

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
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputData *pData;

    BDBG_ASSERT(NULL != output);
    BDBG_ASSERT(NULL != input);

    pData = output->pMixerData;

    if ( NULL == pData || input != pData->input )
    {
        BDBG_ERR(("Input %p is not connected to this output.", input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Input %p is running.  Please stop first.", input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
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

    BDBG_ASSERT(NULL != output);

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

        BDBG_OBJECT_DESTROY(pData, NEXUS_AudioOutputData);
        BKNI_Free(pData);

        /* TODO: Handle RFM object handles */
        switch ( output->objectType )
        {
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
                        g_hdmiMapping[i].output = NULL;
                        g_hdmiMapping[i].handle = NULL;
                        break;
                    }
                }
                BDBG_ASSERT(i < NEXUS_NUM_HDMI_OUTPUTS);
            }
            break;
#if NEXUS_NUM_RFM_OUTPUTS
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
    BDBG_ASSERT(NULL != output);
    return (output->objectType == NEXUS_AudioOutputType_eRfm) ? true : false;
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
    #if NEXUS_NUM_RFM_OUTPUTS
    NEXUS_RfmConnectionSettings rfmConnectionSettings;
    bool enabled=false;
    NEXUS_Error errCode;
    BAPE_RfModHandle apeRfmHandle=NULL;
    BAPE_RfModSettings apeRfmSettings;
    int i;

    BDBG_ASSERT(NULL != slaveHandle);
    BDBG_ASSERT(true == NEXUS_AudioOutput_P_IsDacSlave(slaveHandle));

    if ( NULL == g_NEXUS_audioModuleData.settings.modules.rfm )
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
        apeRfmSettings.master = (BAPE_MixerOutput)sourceHandle->port;
        apeRfmSettings.muted = false;
    }
        
    errCode = BAPE_RfMod_SetSettings(apeRfmHandle, &apeRfmSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Tell RFM to mute or unmute appropriately */
    NEXUS_Module_Lock(g_NEXUS_audioModuleData.settings.modules.rfm);
    NEXUS_Rfm_GetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
    rfmConnectionSettings.audioEnabled = enabled;
    errCode = NEXUS_Rfm_SetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.settings.modules.rfm);

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    #else

    /* No RFM */
    BSTD_UNUSED(slaveHandle);
    BSTD_UNUSED(sourceHandle);

    #endif

    return BERR_SUCCESS;
}

static NEXUS_AudioOutputData *NEXUS_AudioOutput_P_CreateData(NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioOutputData *pData;
    if ( NULL == output->pMixerData )
    {
        output->pMixerData = pData = BKNI_Malloc(sizeof(NEXUS_AudioOutputData));
        if ( pData )
        {
            BKNI_Memset(pData, 0, sizeof(NEXUS_AudioOutputData));
            BDBG_OBJECT_SET(pData, NEXUS_AudioOutputData);
            pData->settings.defaultSampleRate = 48000;
            pData->sampleRate = BAVC_AudioSamplingRate_eUnknown;
            switch ( output->objectType )
            {
            #if NEXUS_NUM_HDMI_OUTPUTS  /* TODO */
            case NEXUS_AudioOutputType_eHdmi:
                #if NEXUS_NUM_HDMI_OUTPUTS != 1
                #error More than one HDMI output is not supported
                #endif
                BDBG_ASSERT(g_hdmiMapping[0].output == NULL);
                {
                    BERR_Code errCode;
                    BAPE_MixerOutput mixerOutput;
                    BAPE_MaiOutputInterruptHandlers interrupts;

                    BDBG_MSG(("Creating MAI handle"));
                    errCode = BAPE_MaiOutput_Open(NEXUS_AUDIO_DEVICE_HANDLE, 0, NULL, &g_hdmiMapping[0].handle);
                    if ( errCode )
                    {
                        BDBG_ERR(("Unable to open MAI Output"));
                        BKNI_Free(output->pMixerData);
                        output->pMixerData = NULL;
                        return NULL;
                    }

                    BAPE_MaiOutput_GetInterruptHandlers(g_hdmiMapping[0].handle, &interrupts);
                    interrupts.sampleRate.pCallback_isr = NEXUS_AudioOutput_P_HdmiSampleRateChange_isr;
                    interrupts.sampleRate.pParam1 = output;
                    interrupts.sampleRate.param2 = (int)g_hdmiMapping[0].handle;
                    errCode = BAPE_MaiOutput_SetInterruptHandlers(g_hdmiMapping[0].handle, &interrupts);
                    if ( errCode )
                    {
                        BDBG_ERR(("Unable to register sample rate interrupt"));
                        BAPE_MaiOutput_Close(g_hdmiMapping[0].handle);
                        BKNI_Free(output->pMixerData);
                        output->pMixerData = NULL;
                        return NULL;
                    }                    
                    g_hdmiMapping[0].output = output;
                    BAPE_MaiOutput_GetConnector(g_hdmiMapping[0].handle, &mixerOutput);
                    output->port = (uint32_t)mixerOutput;
                }
                break;
            #endif
            #if NEXUS_NUM_RFM_OUTPUTS
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

/***************************************************************************
Summary:
    Inform an audio output of a sample rate change
Description:
    Certain outputs such as HDMI require notification of sample rate changes
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SampleRateChange_isr(
    NEXUS_AudioOutputHandle output,
    BAVC_AudioSamplingRate sampleRate
    )
{
    BDBG_ASSERT(NULL != output);

    #if NEXUS_NUM_HDMI_OUTPUTS
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
    {
        NEXUS_AudioOutputData *pData = output->pMixerData;
        pData->sampleRate = sampleRate;
        if ( sampleRate == BAVC_AudioSamplingRate_eUnknown )
        {
            /* We're reverting to default */
            sampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pData->settings.defaultSampleRate);
        }
    }
    #else
    BSTD_UNUSED(output);
    BSTD_UNUSED(sampleRate);
    #endif

    return BERR_SUCCESS;
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

NEXUS_Error NEXUS_AudioOutput_P_SetTrickMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    )
{
    BAPE_OutputVolume volume;
    NEXUS_AudioOutputData *pData;
    NEXUS_Error errCode;

    if ( NEXUS_AudioOutput_P_IsDacSlave(output) )
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

    BDBG_MSG(("Setting output mute to %d for trick", pData->settings.muted || mute));
    BAPE_GetOutputVolume((BAPE_MixerOutput)output->port, &volume);
    volume.muted = pData->settings.muted || mute;
    errCode = BAPE_SetOutputVolume((BAPE_MixerOutput)output->port, &volume);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pData->trickMute = mute;
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
static void NEXUS_AudioOutput_P_HdmiSampleRateChange_isr(void *pParam1, int param2, BAVC_AudioSamplingRate rate)
{
    NEXUS_AudioOutputHandle output = pParam1;
    BSTD_UNUSED(param2);
    BDBG_MSG(("HDMI Sample Rate Change %u", rate));
    NEXUS_AudioOutput_P_SampleRateChange_isr(output, rate);
    NEXUS_HdmiOutput_AudioSampleRateChange_isr(output->pObjectHandle, rate);
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
    pSettings->defaultSampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pData->settings.defaultSampleRate);
    pSettings->outputPll = pData->settings.pll;
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
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
    {
        NEXUS_Module_Lock(g_NEXUS_audioModuleData.settings.modules.hdmiOutput);
        {
            BAVC_AudioSamplingRate sampleRate = BAVC_AudioSamplingRate_e48k;
            NEXUS_AudioOutputData *pData = output->pMixerData;
            unsigned numberOfChannels = format == NEXUS_AudioInputFormat_ePcm5_1?6:2;
            if ( pData )
            {
                if (pData->sampleRate != BAVC_AudioSamplingRate_eUnknown)
                {
                    sampleRate = pData->sampleRate;
                }
                else
                {
                    pData->sampleRate = sampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pData->settings.defaultSampleRate);
                }
            }
            /* Eventually, audio decoder will need to set the proper compressed format */
            NEXUS_HdmiOutput_SetAudioParams_priv(output->pObjectHandle, BAVC_AudioBits_e16, sampleRate,
                                                 format == NEXUS_AudioInputFormat_eCompressed?BAVC_AudioFormat_eAC3:BAVC_AudioFormat_ePCM,numberOfChannels);
        }
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.settings.modules.hdmiOutput);
    }
}

void NEXUS_AudioOutput_GetDefaultEnabledOutputs(
    NEXUS_AudioOutputEnabledOutputs *pOutputs)
{
    BKNI_Memset(pOutputs, 0, sizeof(NEXUS_AudioOutputEnabledOutputs));
}


NEXUS_Error NEXUS_AudioOutput_CreateClockConfig(
    const NEXUS_AudioOutputEnabledOutputs *outputs,
    NEXUS_AudioOutputClockConfig *config)
{ 
    BSTD_UNUSED(outputs);
    BSTD_UNUSED(config);
    return BERR_NOT_SUPPORTED;
}

