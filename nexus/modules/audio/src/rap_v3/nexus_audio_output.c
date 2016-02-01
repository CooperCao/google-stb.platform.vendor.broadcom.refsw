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
#include "priv/nexus_timebase_priv.h"
#if NEXUS_NUM_HDMI_OUTPUTS
#include "nexus_hdmi_output.h"
#include "priv/nexus_hdmi_output_priv.h"
#endif
#if NEXUS_NUM_RFM_OUTPUTS
#include "priv/nexus_rfm_priv.h"
#endif
#include "nexus_vcxo.h"


#ifndef NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 0
#endif

#define NEXUS_AUDIO_MAX_TIMEBASES  (NEXUS_NUM_HDMI_OUTPUTS + NEXUS_NUM_I2S_OUTPUTS + NEXUS_NUM_AUDIO_DUMMY_OUTPUTS + NEXUS_NUM_SPDIF_OUTPUTS + 1) /* [0] is not used */
#define NEXUS_AUDIO_NO_TIMEBASE  0


BDBG_MODULE(nexus_audio_output);

BDBG_OBJECT_ID(NEXUS_AudioOutputData);

typedef struct NEXUS_AudioOutputData
{
    BDBG_OBJECT(NEXUS_AudioOutputData)
    NEXUS_AudioOutputSettings settings;
    NEXUS_AudioInput input;
#if BCHP_CHIP == 3563
    NEXUS_AudioOutputEqualizerSettings eqSettings;
#endif
    NEXUS_AudioOutputSyncSettings syncSettings;
    NEXUS_AudioOutputStatus status;
    BRAP_DestinationHandle destination;
    BAVC_AudioSamplingRate sampleRate;
    BKNI_EventHandle notifyEvent;
    NEXUS_EventCallbackHandle notifyEventCallback; 
    bool settingsChanged;
    bool trickMute;
    bool compressedMute;
    bool syncMute; /* could be moved into syncSettings if sync were refactored to know to mute sinks in addition to sources */
} NEXUS_AudioOutputData;

#if NEXUS_NUM_HDMI_OUTPUTS > 0
static void NEXUS_AudioOutput_P_HdmiSettingsChanged(void *);
static void NEXUS_AudioOutput_P_InitHdmiOutputPort(BRAP_OutputPort port);
static void NEXUS_AudioOutput_P_SetMaiParams(BRAP_OutputPort port, BAVC_AudioSamplingRate sampleRate, unsigned pll);
static void NEXUS_AudioOutput_P_SetHdmiDestination(NEXUS_AudioOutputHandle output, BRAP_DestinationHandle destination);
static void NEXUS_AudioOutput_P_HdmiSampleRate_isr(void *pParam, int param, void *pData);
#endif



#if BCHP_CHIP == 3563
#define NEXUS_HAS_EQUALIZER 1

static int32_t NEXUS_AudioOutput_P_Vol2Magnum(NEXUS_AudioVolumeType type, int32_t volume)
{
    /* The 3563 allows setting of linear or dB values. */
    if ( type == NEXUS_AudioVolumeType_eDecibel )
    {
        /* Values are in 1/100 dB -- same as raptor */
        if ( volume > 0 )
        {
            BDBG_WRN(("dB volume levels > 0 are not supported - setting to 0 dB"));
            return 0;
        }
        else
        {
            if ( volume < NEXUS_AUDIO_VOLUME_DB_MIN )
            {
                return -NEXUS_AUDIO_VOLUME_DB_MIN;
            }
            else
            {
                return -volume;
            }
        }
    }
    else
    {
        /* Values are linear */
        if ( volume > NEXUS_AUDIO_VOLUME_LINEAR_MAX )
        {
            BDBG_WRN(("Clipping audio volume to linear max"));
            return NEXUS_AUDIO_VOLUME_LINEAR_MAX;
        }
        return volume;
    }
}
#else
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
#endif

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
    pSettings->timebase = NEXUS_Timebase_eInvalid;
    pSettings->defaultSampleRate = 48000;
    pSettings->leftVolume = NEXUS_AUDIO_VOLUME_DB_NORMAL;
    pSettings->rightVolume = NEXUS_AUDIO_VOLUME_DB_NORMAL;
    pSettings->pll = g_NEXUS_audioModuleData.moduleSettings.defaultPll;
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
    NEXUS_Error errCode;
    NEXUS_AudioOutputSettings defaultSettings;
    NEXUS_AudioOutputData *pData;
    
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

#if ((BCHP_CHIP==35230) && (BCHP_VER==C1))
    if (pSettings->pll != NEXUS_AudioOutputPll_e0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif
    

    pData = output->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    /* Store settings */
    pData->settings = *pSettings;
    pData->settingsChanged = true;

    /* Apply volume settings only if we have a destination */
    if ( pData->input && NEXUS_AudioInput_P_GetOutputDestination(pData->input, output) )
    {
        errCode = NEXUS_AudioOutput_P_ApplySettings(output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
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
        BDBG_WRN(("Input %p is already connected to this output", output->pMixerData));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Input %p is running.  Please stop first.", (void *)input));
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

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    pData = output->pMixerData;

    if ( NULL == pData || input != pData->input )
    {
        BDBG_ERR(("Input %p is not connected to this output.", (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Forcing input %p to stop on output %p shutdown", (void *)input, (void *)output));
        NEXUS_AudioInput_P_ForceStop(input);
    }
    
    /* 
       Always fall out of HBR mode on a disconnect.  Also, clear decoder mute flags,
       they will be re-asserted on a new connection if needed.
    */
    (void)NEXUS_AudioOutput_P_SetHbrMode(output, false);
    pData->syncMute = false;
    pData->trickMute = false;
    pData->compressedMute = false;

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

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

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

        if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
        {
            #if NEXUS_NUM_HDMI_OUTPUTS
            NEXUS_Module_Lock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);
            NEXUS_HdmiOutput_SetNotifyAudioEvent_priv(output->pObjectHandle, NULL);
            NEXUS_Module_Unlock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);
            #endif
        }
        if ( pData->notifyEventCallback )
        {
            NEXUS_UnregisterEvent(pData->notifyEventCallback);
        }
        if ( pData->notifyEvent )
        {
            BKNI_DestroyEvent(pData->notifyEvent);
        }

        BDBG_OBJECT_DESTROY(pData, NEXUS_AudioOutputData);
        BKNI_Free(pData);
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
    Determine if an output must be tied to a SPDIF output
Description:
    ARC outputs are slaves to SPDIF output.
 ***************************************************************************/
bool NEXUS_AudioOutput_P_IsSpdifSlave(
    NEXUS_AudioOutputHandle output
    )
{
#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    return (output->objectType == NEXUS_AudioOutputType_eArc) ? true : false;
#else
    BSTD_UNUSED(output);
    return false;
#endif
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

    #if NEXUS_NUM_AUDIO_DACS > 1
	unsigned index;
    #endif

    BDBG_ASSERT(NULL != slaveHandle);
    BDBG_ASSERT(true == NEXUS_AudioOutput_P_IsDacSlave(slaveHandle));

    if ( NULL == g_NEXUS_audioModuleData.moduleSettings.modules.rfm )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( NULL != sourceHandle )
    {
        BDBG_ASSERT(sourceHandle->objectType == NEXUS_AudioOutputType_eDac);
        enabled = true;

        #if NEXUS_NUM_AUDIO_DACS > 1
        /* If we have more than one DAC, make sure RAP selects the proper source to the mux */
    	NEXUS_Module_Lock(g_NEXUS_audioModuleData.moduleSettings.modules.rfm);
    	NEXUS_Rfm_GetIndex_priv(slaveHandle->pObjectHandle, &index);
    	NEXUS_Module_Unlock(g_NEXUS_audioModuleData.moduleSettings.modules.rfm);

		errCode = BRAP_SetRfModSource(g_NEXUS_audioModuleData.hRap, index, sourceHandle->port);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        #endif
    }

    /* Tell RFM to mute or unmute appropriately */
    NEXUS_Module_Lock(g_NEXUS_audioModuleData.moduleSettings.modules.rfm);
    NEXUS_Rfm_GetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
    rfmConnectionSettings.audioEnabled = enabled;
    errCode = NEXUS_Rfm_SetConnectionSettings_priv(slaveHandle->pObjectHandle, &rfmConnectionSettings);
    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.moduleSettings.modules.rfm);

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


/***************************************************************************
Summary:
    For ARC slave outputs, set the SPDIF source
Description:
    ARC outputs are slaves to SPDIF outputs.  Mixer will find the proper
    SPDIF to bind the output to and provide it here.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_SetArcSlaveSource(
    NEXUS_AudioOutputHandle slaveHandle, /* ARC connector */
    NEXUS_AudioOutputHandle sourceHandle)/*SPDIF output connector */
{
#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
    BRAP_CapInputPort   arcOutputPort;    /* select ARC (i.e Hdmi input port) */ 
    BRAP_OutputPort     spdifOutputPort;  /* source of the selected ARC */
    NEXUS_AudioReturnChannelSettings arcSettings;
    unsigned index;
    NEXUS_Error errCode;

    NEXUS_AudioReturnChannel_P_GetIndex(slaveHandle->pObjectHandle, &index);
    switch ( index )
    {
    case 0:
        arcOutputPort = BRAP_CapInputPort_eHdmi;
        break;

    default:
        BDBG_ERR(("Unsupported audio ARC id requested (%d)", index));
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_RETURN_CHANNEL);
        arcOutputPort = BRAP_CapInputPort_eMax;
        break;
    }

    spdifOutputPort = BRAP_OutputPort_eSpdif;  /*TO_DO: will need priv function to map for multiple SPDIF out */

    NEXUS_AudioReturnChannel_P_SetSourceId(slaveHandle->pObjectHandle, spdifOutputPort);

    NEXUS_AudioReturnChannel_GetSettings(slaveHandle->pObjectHandle, &arcSettings);
    if ( arcSettings.muted || (NULL == sourceHandle) )
    {
        spdifOutputPort = BRAP_OutputPort_eMax;
    }

    errCode = BRAP_SetArcSource(g_NEXUS_audioModuleData.hRap,
                                arcOutputPort, spdifOutputPort);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

#else
    /* No ARC support */
    BSTD_UNUSED(slaveHandle);
    BSTD_UNUSED(sourceHandle);
#endif

    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Populate a destination details structure for an output
Description:
    The 3563/7405+ raptor requires a structure for an output, not just an
    enum value.  This is populated by the AudioOutput code per-output type.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_P_GetDestinationDetails(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo,  /* [in/out] */
    BRAP_DstDetails *pPrivateDestInfo  /* [in/out] */
    )
{
    int i,numberOfChannels=0;
    BRAP_OutputPortConfig   opConfig;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(NULL != pDestInfo);

    /* Set destination details to defaults */
    BKNI_Memset(pDestInfo, 0, sizeof(BRAP_DstDetails));

#if !NEXUS_NUM_AUDIO_CAPTURES
    BSTD_UNUSED(primaryChannel);
#endif

    switch ( output->objectType )
    {
#if NEXUS_NUM_AUDIO_CAPTURES
    case NEXUS_AudioOutputType_eCapture:
        pDestInfo->eAudioDst = BRAP_AudioDst_eRingBuffer;
        /* this is a ring-buffer object.  Need to get details from capture */
        return NEXUS_AudioCapture_P_GetDestinationDetails(output->pObjectHandle, format, primaryChannel, pDestInfo);
#endif        
#if NEXUS_HAS_AUDIO_MUX_OUTPUT
    case NEXUS_AudioOutputType_eMux:
        pDestInfo->eAudioDst = BRAP_AudioDst_eRaveBuffer;
        return NEXUS_AudioMuxOutput_P_GetDestinationDetails(output->pObjectHandle, pDestInfo);
#endif
    case NEXUS_AudioOutputType_eGroup:
        for ( i = 0; i < BRAP_OutputChannelPair_eMax; i++ )
        {
            pDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
        }
        pDestInfo->eAudioDst = BRAP_AudioDst_eOutputPort;
        return NEXUS_AudioOutputGroup_P_GetDestinationDetails(output->pObjectHandle, format, primaryChannel, pDestInfo);
#if NEXUS_NUM_HDMI_OUTPUTS
    case NEXUS_AudioOutputType_eHdmi:
        pDestInfo->eAudioDst = BRAP_AudioDst_eOutputPort;
        for ( i = 0; i < BRAP_OutputChannelPair_eMax; i++ )
        {
            pDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
        }
        pDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLR] = output->port;

        BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, output->port, &opConfig);
        opConfig.bCompressed = (format == NEXUS_AudioInputFormat_eCompressed)?true:false;
        /* do not change HBR moode for compressed */
        if(!opConfig.bCompressed)
            opConfig.bHbrEnable = false;
        switch(format)
        {
        case NEXUS_AudioInputFormat_eCompressed:
        case NEXUS_AudioInputFormat_ePcmStereo:
#if BRAP_VER >= 4
            pDestInfo->eAudioMode = BRAP_OutputMode_e2_0; /* dont care for compressed */
            pDestInfo->bLfeOn = false;
#else
            pDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e2_0; /* dont care for compressed */
            pDestInfo->uDstDetails.sOpDetails.bLfeOn = false;
#endif
            BDBG_MSG(("Setting %s output for port %d", opConfig.bCompressed?"compressed":"stereo",output->port));
            opConfig.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = output->port;
            opConfig.uOutputPortSettings.sMaiSettings.eMaiAudioFormat =
                (opConfig.bCompressed)?BRAP_OP_MaiAudioFormat_eSpdifCompressed:BRAP_OP_MaiAudioFormat_eSpdif2Channel;
            numberOfChannels=2; /* number of channels */
            break;
        case NEXUS_AudioInputFormat_ePcm5_1:
#if BRAP_VER >= 4
            pDestInfo->eAudioMode = BRAP_OutputMode_e3_2;
            pDestInfo->bLfeOn = true;
#else
            pDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e3_2;
            pDestInfo->uDstDetails.sOpDetails.bLfeOn = true;
#endif
            BDBG_MSG(("Setting %s output for port %d", "pcm 5.1",output->port));
            opConfig.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = NEXUS_HDMI_MULTI_PORT_0;
            opConfig.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdif6Channel;
            numberOfChannels=6; /* number if channels */

            /* for 5.1 pcm audio we need i2s ports */
            BDBG_ASSERT(pPrivateDestInfo);
            pPrivateDestInfo->eAudioDst = BRAP_AudioDst_eOutputPort;
            for ( i = 0; i < BRAP_OutputChannelPair_eMax; i++ )
            {
                pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
            }
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLR] = NEXUS_HDMI_MULTI_PORT_0;
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLRSurround] = NEXUS_HDMI_MULTI_PORT_1;
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eCentreLF] = NEXUS_HDMI_MULTI_PORT_2;
#if BRAP_VER >= 4
            pPrivateDestInfo->eAudioMode = BRAP_OutputMode_e3_2;
            pPrivateDestInfo->bLfeOn = true;
#else
            pPrivateDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e3_2;
            pPrivateDestInfo->uDstDetails.sOpDetails.bLfeOn = true;
#endif
            break;
        case NEXUS_AudioInputFormat_ePcm7_1:
#if BRAP_VER >= 4
            pDestInfo->eAudioMode = BRAP_OutputMode_e3_4;
            pDestInfo->bLfeOn = true;
#else
            pDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e3_4;
            pDestInfo->uDstDetails.sOpDetails.bLfeOn = true;
#endif
            BDBG_MSG(("Setting %s output for port %d", "pcm 7.1",output->port));
            opConfig.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = NEXUS_HDMI_MULTI_PORT_0;
            opConfig.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdif8Channel;
            numberOfChannels=8; /* number of channels */

            /* for 7.1 pcm audio we need i2s ports */
            BDBG_ASSERT(pPrivateDestInfo);
            pPrivateDestInfo->eAudioDst = BRAP_AudioDst_eOutputPort;
            for ( i = 0; i < BRAP_OutputChannelPair_eMax; i++ )
            {
                pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
            }
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLR] = NEXUS_HDMI_MULTI_PORT_0;
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLRSurround] = NEXUS_HDMI_MULTI_PORT_1;
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eCentreLF] = NEXUS_HDMI_MULTI_PORT_2;
            pPrivateDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLRRear] = NEXUS_HDMI_MULTI_PORT_3;
#if BRAP_VER >= 4
            pPrivateDestInfo->eAudioMode = BRAP_OutputMode_e3_4;
            pPrivateDestInfo->bLfeOn = true;
#else
            pPrivateDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e3_4;
            pPrivateDestInfo->uDstDetails.sOpDetails.bLfeOn = true;
#endif
            break;
        default:
            BDBG_ERR(("Unknown format %d",format));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        BDBG_MSG(("MAI format set to %d,compressed %d number of channnels %d",
                  opConfig.uOutputPortSettings.sMaiSettings.eMaiAudioFormat,opConfig.bCompressed,numberOfChannels));

        NEXUS_Module_Lock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);
        {
            BAVC_AudioSamplingRate sampleRate = BAVC_AudioSamplingRate_e48k;
            NEXUS_AudioOutputData *pData = output->pMixerData;
            if ( pData )
            {
                sampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pData->settings.defaultSampleRate);
            }
            /* Eventually, audio decoder will need to set the proper compressed format */
            NEXUS_HdmiOutput_SetAudioParams_priv(output->pObjectHandle, BAVC_AudioBits_e16, sampleRate,
                                                 opConfig.bCompressed?BAVC_AudioFormat_eAC3:BAVC_AudioFormat_ePCM,numberOfChannels);
        }
        NEXUS_Module_Unlock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);

        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &opConfig);
        if (errCode) return BERR_TRACE(errCode);
        break;
#endif
    default:
        /* The settings here are identical for compressed or stereo */
        pDestInfo->eAudioDst = BRAP_AudioDst_eOutputPort;
        for ( i = 0; i < BRAP_OutputChannelPair_eMax; i++ )
        {
            pDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
        }
        pDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLR] = output->port;

#if BRAP_VER >= 4
        pDestInfo->eAudioMode = BRAP_OutputMode_e2_0;
        pDestInfo->bLfeOn = false;
#if ((BCHP_CHIP == 35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233))
        if ( NEXUS_AudioOutputType_eI2s == output->objectType &&
             BRAP_OutputPort_eI2s0 == output->port && 
             NEXUS_AudioInputFormat_ePcm5_1 == format )
        {
            /* for multi-channel I2S out */
            pDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eLRSurround] = BRAP_OutputPort_eI2s1;
            pDestInfo->uDstDetails.sOpDetails.eOutput[BRAP_OutputChannelPair_eCentreLF] = BRAP_OutputPort_eI2s2;
            pDestInfo->eAudioMode = BRAP_OutputMode_e3_2;
        }
#endif
#else
        pDestInfo->uDstDetails.sOpDetails.eAudioMode = BRAP_OutputMode_e2_0;
        pDestInfo->uDstDetails.sOpDetails.bLfeOn = false;
#endif
        BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, output->port, &opConfig);
        opConfig.bCompressed = (format == NEXUS_AudioInputFormat_eCompressed)?true:false;
        BDBG_MSG(("Setting %s output for port %d", opConfig.bCompressed?"compressed":"stereo",output->port));
        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &opConfig);
        if (errCode) return BERR_TRACE(errCode);
        break;
    }

#if 0 == NEXUS_NUM_HDMI_OUTPUTS
    BSTD_UNUSED(pPrivateDestInfo);
    BSTD_UNUSED(numberOfChannels);
#endif

    return BERR_SUCCESS;
}

#if BCHP_CHIP == 3563
/***************************************************************************
Summary:
    Get settings of an audio output
See Also:
    NEXUS_AudioOutput_SetSettings
 ***************************************************************************/
void NEXUS_AudioOutput_GetEqualizerSettings(
    NEXUS_AudioOutputHandle output,
    NEXUS_AudioOutputEqualizerSettings *pSettings    /* [out] Current Settings */
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(NULL != pSettings);

    if ( NULL == output->pMixerData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            NEXUS_Error errCode;
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return;
        }
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    }

    pData = output->pMixerData;
    *pSettings = pData->eqSettings;
}

/***************************************************************************
Summary:
    Set settings of an audio output
See Also:
    NEXUS_AudioOutput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioOutput_SetEqualizerSettings(
    NEXUS_AudioOutputHandle output,
    const NEXUS_AudioOutputEqualizerSettings *pSettings
    )
{
    NEXUS_AudioOutputData *pData;
#if NEXUS_HAS_EQUALIZER
    NEXUS_AudioOutputEqualizerMode hardwareMode = NEXUS_AudioOutputEqualizerMode_eDisabled;
    NEXUS_Error errCode;
#endif

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(NULL != pSettings);

    if ( NULL == output->pMixerData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    pData = output->pMixerData;

#if NEXUS_HAS_EQUALIZER
    switch ( output->objectType )
    {
    case NEXUS_AudioOutputType_eDac:
        hardwareMode = g_NEXUS_audioModuleData.moduleSettings.dacEqualizerMode;
        break;
    case NEXUS_AudioOutputType_eI2s:
        hardwareMode = g_NEXUS_audioModuleData.moduleSettings.i2sEqualizerMode;
        break;
    default:
        break;
    }

    if ( hardwareMode == NEXUS_AudioOutputEqualizerMode_eDisabled )
    {
        if ( pSettings->mode != hardwareMode )
        {
            BDBG_WRN(("Equalizer is not enabled for this output.  Ignoring equalizer settings"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        return BERR_SUCCESS;
    }

    if ( pSettings->mode != NEXUS_AudioOutputEqualizerMode_eDisabled && pSettings->mode != hardwareMode )
    {
        BDBG_WRN(("Requested equalizer mode does not match the one provided at NEXUS_AudioModule_Init().  Please check settings."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
    pData->eqSettings = *pSettings;

    if ( hardwareMode == NEXUS_AudioOutputEqualizerMode_eTone )
    {
        BRAP_Equalizer_ToneControl toneParams;
        if ( pSettings->mode == NEXUS_AudioOutputEqualizerMode_eDisabled )
        {
            BRAP_GetDefaultToneControlParams(g_NEXUS_audioModuleData.hRap, &toneParams);
        }
        else
        {
            toneParams.iBassGain = pSettings->modeSettings.tone.bass/10;        /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
            toneParams.iTrebleGain = pSettings->modeSettings.tone.treble/10;    /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
        }
        errCode = BRAP_SetEqualizerToneControl(g_NEXUS_audioModuleData.hRap, output->port, &toneParams);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else if ( hardwareMode == NEXUS_AudioOutputEqualizerMode_eFiveBand )
    {
        BRAP_Equalizer_Params eqParams;
        if ( pSettings->mode == NEXUS_AudioOutputEqualizerMode_eDisabled )
        {
            BRAP_GetDefaultEqualizerParams(g_NEXUS_audioModuleData.hRap, &eqParams);
        }
        else
        {
            eqParams.i100HzGain = pSettings->modeSettings.fiveBand.gain100Hz/10;    /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
            eqParams.i300HzGain = pSettings->modeSettings.fiveBand.gain300Hz/10;    /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
            eqParams.i1000HzGain = pSettings->modeSettings.fiveBand.gain1000Hz/10;  /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
            eqParams.i3000HzGain = pSettings->modeSettings.fiveBand.gain3000Hz/10;  /* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
            eqParams.i10000HzGain = pSettings->modeSettings.fiveBand.gain10000Hz/10;/* Raptor takes this in 1/10 dB instead of the usual 1/100 dB */
        }
        errCode = BRAP_SetEqualizerParams(g_NEXUS_audioModuleData.hRap, output->port, &eqParams);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        /* Custom mode will have parameters set elsewhere */
    }

    errCode = BRAP_EnableEqualizerLoudness(g_NEXUS_audioModuleData.hRap, output->port, pSettings->loudnessEnabled);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
#else
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}
#endif

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
            NEXUS_AudioOutput_GetDefaultSettings(&pData->settings);
            pData->sampleRate = BAVC_AudioSamplingRate_eUnknown;
            switch ( output->objectType )
            {
#if BCHP_CHIP == 3563
            case NEXUS_AudioOutputType_eDac:
                pData->eqSettings.mode = g_NEXUS_audioModuleData.moduleSettings.dacEqualizerMode;
                break;
            case NEXUS_AudioOutputType_eI2s:
                pData->eqSettings.mode = g_NEXUS_audioModuleData.moduleSettings.i2sEqualizerMode;
                break;
#endif
            #if NEXUS_NUM_HDMI_OUTPUTS
            case NEXUS_AudioOutputType_eHdmi:
                NEXUS_AudioOutput_P_InitHdmiOutput();
                {
                    BERR_Code errCode;
                    BRAP_OutputPortConfig outputSettings;

                    errCode = BKNI_CreateEvent(&pData->notifyEvent);
                    if ( errCode )
                    {
                        (void)BERR_TRACE(errCode);
                        BDBG_OBJECT_DESTROY(pData, NEXUS_AudioOutputData);
                        BKNI_Free(pData);
                        return NULL;
                    }
                    pData->notifyEventCallback = NEXUS_RegisterEvent(pData->notifyEvent, NEXUS_AudioOutput_P_HdmiSettingsChanged, output);
                    if ( NULL == pData->notifyEventCallback )
                    {
                        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                        BKNI_DestroyEvent(pData->notifyEvent);
                        BDBG_OBJECT_DESTROY(pData, NEXUS_AudioOutputData);
                        BKNI_Free(pData);
                        return NULL;
                    }

                    NEXUS_Module_Lock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);
                    NEXUS_HdmiOutput_SetNotifyAudioEvent_priv(output->pObjectHandle, pData->notifyEvent);
                    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.moduleSettings.modules.hdmiOutput);

                    errCode = BRAP_GetDefaultOutputConfig(g_NEXUS_audioModuleData.hRap, BRAP_OutputPort_eI2s4, &outputSettings);
                    /* This should never fail */
                    BDBG_ASSERT(BERR_SUCCESS == errCode);

                    #if NEXUS_NUM_I2S_OUTPUTS > 0 && NEXUS_HDMI_MULTI_ON_I2S
                    /* Fill up output settings */
                    outputSettings.eOutputPort = BRAP_OutputPort_eI2s4;
                    outputSettings.bCompressed = false;
                    outputSettings.eOutputSR = BAVC_AudioSamplingRate_e48k;
                    outputSettings.uiOutputBitsPerSample = 24;
                    outputSettings.uOutputPortSettings.sI2sSettings.bAlignedToLRClk = false;
                    outputSettings.uOutputPortSettings.sI2sSettings.ePll = g_NEXUS_audioModuleData.moduleSettings.defaultPll;
                    errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &outputSettings);
                    /* This should never fail */
                    BDBG_ASSERT(BERR_SUCCESS == errCode);
                    #endif
                }
                break;
            #endif
            default:
                #if BCHP_CHIP == 3563
                pData->eqSettings.mode = NEXUS_AudioOutputEqualizerMode_eDisabled;
                #endif
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
    BRAP_OutputPortConfig outputSettings;
    NEXUS_Error errCode;

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
    if ( g_NEXUS_audioModuleData.moduleSettings.independentDelay )
    {
        /* set independent delay */
        errCode = BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, audioOutput->port, &outputSettings);
        if ( errCode ) return BERR_TRACE(errCode);

#if BRAP_VER >= 4
        outputSettings.uiDelay = pData->status.delay = pData->settings.additionalDelay + pData->syncSettings.delay;
#else
        outputSettings.iDelay = pData->status.delay = pData->settings.additionalDelay + pData->syncSettings.delay;
#endif
        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &outputSettings);
        if ( errCode ) return BERR_TRACE(errCode);
    }
    else if ( pData->settings.additionalDelay || pData->syncSettings.delay )
    {
        BDBG_ERR(("Independent output delay is not enabled, ignoring delay.  Please check NEXUS_AudioModuleSettings.independentDelay"));
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

#if NEXUS_NUM_HDMI_OUTPUTS > 0
static void NEXUS_AudioOutput_P_HdmiSampleRate_isr(void *pParam, int param, void *pData)
{
    const BRAP_DSPCHN_DestinationSampleRateChangeInfo *pInfo = pData;
    NEXUS_HdmiOutputHandle hHdmi;
    NEXUS_AudioOutputHandle output;
    NEXUS_AudioOutputData *pOutputData;

    hHdmi = pParam;
    output = (NEXUS_AudioOutput)param;
    pOutputData = output->pMixerData;
    pOutputData->sampleRate = pInfo->eSamplingRate;

    NEXUS_HdmiOutput_AudioSampleRateChange_isr(hHdmi, pInfo->eSamplingRate);
}

static void NEXUS_AudioOutput_P_SetHdmiDestination(NEXUS_AudioOutputHandle output, BRAP_DestinationHandle destination)
{
    NEXUS_HdmiOutputHandle hHdmi;
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    hHdmi = output->pObjectHandle;
    if ( destination )
    {
        /* Creating */
        BRAP_InstallDestinationLevelAppInterruptCb(destination,
                                                   BRAP_DestinationInterrupt_eSampleRateChange,
                                                   NEXUS_AudioOutput_P_HdmiSampleRate_isr, hHdmi, (int)output);
    }
    else
    {
        /* Destroying */
        destination = NEXUS_AudioOutput_P_GetDestination(output);
        if ( destination )
        {
            BRAP_RemoveDestinationLevelAppInterruptCb(destination, BRAP_DestinationInterrupt_eSampleRateChange);
        }
    }
}
#endif

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

static NEXUS_Error NEXUS_AudioOutput_P_SetMute(
    NEXUS_AudioOutputData *pData
)
{
    NEXUS_Error errCode;
    bool oldMute, newMute;

    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    if (pData->destination == NULL)
    {
        return NEXUS_SUCCESS;
    }

    errCode = BRAP_GetDestinationMute(pData->destination, &oldMute);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    newMute = pData->settings.muted || pData->trickMute || pData->syncMute || pData->compressedMute;

    if ( oldMute != newMute )
    {
        BDBG_MSG(("Setting output mute to %d", newMute));

        errCode = BRAP_SetDestinationMute(pData->destination, newMute);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioOutput_P_ApplySettings(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputData *pData;
    NEXUS_AudioOutputSettings *pSettings;
    unsigned timebaseIndex = 0;
    BRAP_OutputPortConfig outputSettings, oldSettings;
    bool canSetVolume=true;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    pData = output->pMixerData;
    if ( NULL == pData )
    {
        /* Nothing to do */
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);

    BDBG_MSG(("Applying audio output settings"));

    pSettings = &pData->settings;
    if ( !pData->settingsChanged )
    {
        return BERR_SUCCESS;    /* Nothing to do */
    }
    pData->settingsChanged = false;

    if ( NEXUS_AudioOutput_P_IsDacSlave(output) ||
         NEXUS_AudioOutput_P_IsSpdifSlave(output) ||
         output->objectType == NEXUS_AudioOutputType_eMux )
    {
        /* TODO: Apply RFM mute here? */
        return BERR_SUCCESS;
    }

    /* We can mute compressed and PCM ports */
	errCode = NEXUS_AudioOutput_P_SetMute(pData);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( output->objectType != NEXUS_AudioOutputType_eCapture )
    {
        BRAP_OP_ChannelOrder channelOrder;

        /* Apply remaining settings to raptor based on compressed vs. pcm */
        errCode = BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, output->port, &outputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        oldSettings = outputSettings;

        if ( outputSettings.bCompressed )
        {
            canSetVolume = false;
        }

        switch ( pSettings->channelMode )
        {
        case NEXUS_AudioChannelMode_eStereo:
            channelOrder = BRAP_OP_ChannelOrder_eNormal;
            break;
        case NEXUS_AudioChannelMode_eLeft:
            channelOrder = BRAP_OP_ChannelOrder_eBothLeft;
            break;
        case NEXUS_AudioChannelMode_eRight:
            channelOrder = BRAP_OP_ChannelOrder_eBothRight;
            break;
        case NEXUS_AudioChannelMode_eSwapped:
            channelOrder = BRAP_OP_ChannelOrder_eSwap;
            break;
        default:
            BDBG_ERR(("Invalid Channel Order"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        switch ( output->objectType )
        {
        case NEXUS_AudioOutputType_eDac:
            outputSettings.uOutputPortSettings.sDacSettings.eChannelOrder = channelOrder;
            break;
        case NEXUS_AudioOutputType_eI2s:
            outputSettings.uOutputPortSettings.sI2sSettings.eChannelOrder = channelOrder;
            outputSettings.uOutputPortSettings.sI2sSettings.ePll = pSettings->pll;
            break;
#if NEXUS_NUM_HDMI_OUTPUTS
        case NEXUS_AudioOutputType_eHdmi:
            outputSettings.uOutputPortSettings.sMaiSettings.eChannelOrder = channelOrder;
            #if BCHP_CHIP != 7550
            outputSettings.uOutputPortSettings.sMaiSettings.ePll = pSettings->pll;
            #endif
            {
                NEXUS_HdmiOutputSettings hdmiSettings;
                NEXUS_HdmiOutput_GetSettings(output->pObjectHandle, &hdmiSettings);
                outputSettings.sSpdifChanStatusParams.bProfessionalMode = hdmiSettings.audioChannelStatusInfo.professionalMode;
                outputSettings.sSpdifChanStatusParams.bSwCopyRight = hdmiSettings.audioChannelStatusInfo.swCopyRight;
                outputSettings.sSpdifChanStatusParams.ui16CategoryCode = hdmiSettings.audioChannelStatusInfo.categoryCode;
                outputSettings.sSpdifChanStatusParams.ui16ClockAccuracy = hdmiSettings.audioChannelStatusInfo.clockAccuracy;
                outputSettings.sSpdifChanStatusParams.bSeparateLRChanNum = hdmiSettings.audioChannelStatusInfo.separateLRChanNum;
                #if BCHP_CHIP == 7550
                outputSettings.uOutputPortSettings.sMaiSettings.eMClkRate = BRAP_OP_MClkRate_e256Fs;
                #endif
                outputSettings.sSpdiffmSettings.bEnableDither = hdmiSettings.audioDitherEnabled;
            }
            break;
#endif
        case NEXUS_AudioOutputType_eSpdif:
            outputSettings.uOutputPortSettings.sSpdifSettings.eChannelOrder = channelOrder;
            outputSettings.uOutputPortSettings.sSpdifSettings.ePll = pSettings->pll;
            break;
        default:
            break;
        }

        if ( g_NEXUS_audioModuleData.moduleSettings.independentDelay )
        {
            /* set independent delay */
            BDBG_MSG(("Setting independent delay to %d", pSettings->additionalDelay + pData->syncSettings.delay));

#if BRAP_VER >= 4
            outputSettings.uiDelay = pData->status.delay = pSettings->additionalDelay + pData->syncSettings.delay;
#else
            outputSettings.iDelay = pData->status.delay = pSettings->additionalDelay + pData->syncSettings.delay;
#endif
        }
        else if ( pSettings->additionalDelay )
        {
            BDBG_ERR(("Independent output delay is not enabled, ignoring delay.  Please check NEXUS_AudioModuleSettings.independentDelay"));
        }

        errCode = NEXUS_Timebase_GetIndex(pSettings->timebase, &timebaseIndex);
        if (errCode) return BERR_TRACE(errCode);

#if BRAP_VER < 4
        /* update the timebase and sample rate */
        outputSettings.eOutputTimebase = BAVC_Timebase_e0 + timebaseIndex;
#endif
        outputSettings.eOutputSR = NEXUS_AudioModule_P_SampleRate2Avc(pSettings->defaultSampleRate);

        /* Only apply the settings if something has actually changed.  Otherwise, the port may mute. */
        if ( BKNI_Memcmp(&oldSettings, &outputSettings, sizeof(outputSettings)) )
        {
            errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap,
                                           &outputSettings);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
            #if NEXUS_NUM_HDMI_OUTPUTS > 0
            if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
            {
                /* HDMI needs to also handle the multichannel I2S output PLL and sample rate */
                BAVC_AudioSamplingRate avcSampleRate;

                avcSampleRate = NEXUS_AudioModule_P_SampleRate2Avc(pSettings->defaultSampleRate);
                NEXUS_AudioOutput_P_SetMaiParams(NEXUS_HDMI_MULTI_PORT_0, avcSampleRate, pSettings->pll);
                NEXUS_AudioOutput_P_SetMaiParams(NEXUS_HDMI_MULTI_PORT_1, avcSampleRate, pSettings->pll);
                NEXUS_AudioOutput_P_SetMaiParams(NEXUS_HDMI_MULTI_PORT_2, avcSampleRate, pSettings->pll);
                NEXUS_AudioOutput_P_SetMaiParams(NEXUS_HDMI_MULTI_PORT_3, avcSampleRate, pSettings->pll);
            }
            #endif
        }
    }

    /* Volume must be applied to only PCM outputs */
    if ( canSetVolume )
    {
#ifndef NEXUS_CAS
        BDBG_MSG(("Setting destination volume (port %d) to 0x%x 0x%x",
                  output->port,
                  NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->leftVolume),
                  NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->rightVolume)));
        errCode = BRAP_SetDestinationVolume(pData->destination,
                                            NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->leftVolume),
                                            NEXUS_AudioOutput_P_Vol2Magnum(pSettings->volumeType, pSettings->rightVolume));
#endif /* NEXUS_CAS */

        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

	errCode = NEXUS_AudioOutput_P_SetMute(pData);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    switch ( output->objectType )
    {
    case NEXUS_AudioOutputType_eDac:
    case NEXUS_AudioOutputType_eRfm:
        break;
    default:
        /* Check for VCXO Settings */
        if ( pSettings->autoConfigureVcxo )
        {
            /* Determine PLL linkage */
            NEXUS_AudioOutputPllSettings pllSettings;
            NEXUS_AudioOutputPll_GetSettings(pSettings->pll, &pllSettings);
            if ( pllSettings.mode == NEXUS_AudioOutputPllMode_eVcxo )
            {
                NEXUS_VcxoSettings vcxoSettings;
                NEXUS_Vcxo_GetSettings(pllSettings.modeSettings.vcxo.vcxo, &vcxoSettings);
                errCode = NEXUS_Timebase_GetIndex(pSettings->timebase, &timebaseIndex);
                if (errCode) return BERR_TRACE(errCode);
                if ( vcxoSettings.timebase != (int)timebaseIndex )
                {
                    BDBG_WRN(("Changing VCXO-PLL %u timebase from %u to %u", pllSettings.modeSettings.vcxo.vcxo, vcxoSettings.timebase, timebaseIndex));
                    vcxoSettings.timebase = timebaseIndex;
                    errCode = NEXUS_Vcxo_SetSettings(pllSettings.modeSettings.vcxo.vcxo, &vcxoSettings);
                    if ( errCode )
                    {
                        /* Report and continue */
                        (void)BERR_TRACE(errCode);
                    }
                }
            }
        }
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioOutput_P_SetHbrMode(
    NEXUS_AudioOutputHandle output,
    bool bEnableHbr
    )
{
    NEXUS_Error errCode;

    BRAP_OutputPortConfig outputSettings;
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    if ( output->objectType == NEXUS_AudioOutputType_eHdmi ||
         output->objectType == NEXUS_AudioOutputType_eSpdif )
    {
        errCode = BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap,
                                              output->port,
                                              &outputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        outputSettings.bHbrEnable = bEnableHbr;

        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap,
                                       &outputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else if ( bEnableHbr )
    {
        BDBG_ERR(("Can not enable HBR mode for outputs other than HDMI or SPDIF."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}



NEXUS_Error NEXUS_AudioOutput_P_SetSyncMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    )
{
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

    pData->syncMute = mute;

    if (pData->input && NEXUS_AudioInput_P_GetOutputDestination(pData->input, output))
	{
	    errCode = NEXUS_AudioOutput_P_SetMute(pData);
	    if ( errCode )
	    {
	        return BERR_TRACE(errCode);
	    }
	}

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_AudioOutput_P_SetTrickMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    )
{
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

    pData->trickMute = mute;

    errCode = NEXUS_AudioOutput_P_SetMute(pData);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_AudioOutput_P_SetCompressedMute(
    NEXUS_AudioOutputHandle output,
    bool mute
    )
{
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

    pData->compressedMute = mute;

    errCode = NEXUS_AudioOutput_P_SetMute(pData);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

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

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL != pData && NULL != pData->input )
    {
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        return true;
    }

    return false;
}

NEXUS_AudioInput NEXUS_AudioOutput_P_GetInput(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL != pData )
    {
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        return pData->input;
    }

    return NULL;
}

/***************************************************************************
Summary:
    Retrieve the destination handle for an output
 ***************************************************************************/
BRAP_DestinationHandle NEXUS_AudioOutput_P_GetDestination(
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL != pData && NULL != pData->input )
    {
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioOutputData);
        return pData->destination;
    }

    return NULL;
}

/***************************************************************************
Summary:
    Notify an output that it's destination has been added or removed
 ***************************************************************************/
void NEXUS_AudioOutput_P_SetDestination(
    NEXUS_AudioOutputHandle output,
    BRAP_DestinationHandle destination
    )
{
    NEXUS_AudioOutputData *pData;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);

    pData = output->pMixerData;
    if ( NULL == pData )
    {
        pData = NEXUS_AudioOutput_P_CreateData(output);
        if ( NULL == pData )
        {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return;
        }
    }

#if NEXUS_NUM_HDMI_OUTPUTS
    if ( output->objectType == NEXUS_AudioOutputType_eHdmi )
    {
        NEXUS_AudioOutput_P_SetHdmiDestination(output, destination);
    }
#endif

    pData->destination = destination;
    pData->settingsChanged = true;

    switch ( output->objectType )
    {
#if NEXUS_NUM_AUDIO_CAPTURES
    case NEXUS_AudioOutputType_eCapture:
        NEXUS_AudioCapture_P_SetDestination(output->pObjectHandle, destination);
        break;
#endif        
#if NEXUS_HAS_AUDIO_MUX_OUTPUT
    case NEXUS_AudioOutputType_eMux:
        NEXUS_AudioMuxOutput_P_SetDestination(output->pObjectHandle, destination);
        break;
#endif
    case NEXUS_AudioOutputType_eGroup:
        NEXUS_AudioOutputGroup_P_SetDestination(output->pObjectHandle, destination);
        break;
    default:
        /* No other output types need to know */
        break;
    }
}

BRAP_ProcessingStageHandle NEXUS_AudioOutput_P_GetProcessingStage(
    NEXUS_AudioOutputHandle output
    )
{
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    switch ( output->objectType )
    {
#if NEXUS_HAS_AUDIO_MUX_OUTPUT
    case NEXUS_AudioOutputType_eMux:
        return NEXUS_AudioMuxOutput_P_GetProcessingStage(output->pObjectHandle);
#endif
    default:
        /* No processing required */
        return NULL;
    }
}


#if NEXUS_NUM_HDMI_OUTPUTS > 0
static void NEXUS_AudioOutput_P_HdmiSettingsChanged(void *pOutput)
{
    NEXUS_AudioOutputHandle output = (NEXUS_AudioOutput)pOutput;
    NEXUS_AudioOutputSettings outputSettings;

    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(output->objectType == NEXUS_AudioOutputType_eHdmi);
    
    BDBG_MSG(("HDMI audio settings have changed.  Updating."));

    /* this will force the settings to be applied to raptor */
    NEXUS_AudioOutput_GetSettings(output, &outputSettings);
    NEXUS_AudioOutput_SetSettings(output, &outputSettings);
}

void NEXUS_AudioOutput_P_InitHdmiOutput(void)
{
    BERR_Code errCode;
    BRAP_OutputPortConfig outputSettings;

    /* Must manually "open" hdmi inside the audio module.  This is as good a place as any to do that, it will be done only once. */
    errCode = BRAP_GetDefaultOutputConfig(g_NEXUS_audioModuleData.hRap, BRAP_OutputPort_eMai, &outputSettings);
    /* This should never fail */
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    /* Fill up output settings -- default to PCM */
    outputSettings.eOutputPort = BRAP_OutputPort_eMai;
#if BCHP_CHIP == 7550
    outputSettings.uOutputPortSettings.sMaiSettings.eMClkRate = BRAP_OP_MClkRate_e256Fs;
#else
    outputSettings.uOutputPortSettings.sMaiSettings.ePll = g_NEXUS_audioModuleData.moduleSettings.defaultPll;
#endif
    outputSettings.uOutputPortSettings.sMaiSettings.eMaiMuxSelector = BRAP_OutputPort_eMai;
    outputSettings.uOutputPortSettings.sMaiSettings.eMaiAudioFormat = BRAP_OP_MaiAudioFormat_eSpdif6Channel;
    errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &outputSettings);
    /* This should never fail */
    BDBG_ASSERT(BERR_SUCCESS == errCode);

    NEXUS_AudioOutput_P_InitHdmiOutputPort(NEXUS_HDMI_MULTI_PORT_0);
    NEXUS_AudioOutput_P_InitHdmiOutputPort(NEXUS_HDMI_MULTI_PORT_1);
    NEXUS_AudioOutput_P_InitHdmiOutputPort(NEXUS_HDMI_MULTI_PORT_2);
    NEXUS_AudioOutput_P_InitHdmiOutputPort(NEXUS_HDMI_MULTI_PORT_3);
}

static void NEXUS_AudioOutput_P_InitHdmiOutputPort(BRAP_OutputPort port)
{
    BERR_Code errCode;
    BRAP_OutputPortConfig outputSettings;

    errCode = BRAP_GetDefaultOutputConfig(g_NEXUS_audioModuleData.hRap, port, &outputSettings);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    /* Fill up output settings */
    outputSettings.eOutputPort = port;
    outputSettings.bCompressed = false;
    outputSettings.eOutputSR = BAVC_AudioSamplingRate_e48k;
    outputSettings.uiOutputBitsPerSample = 24;
#if BCHP_CHIP == 7550
    outputSettings.uOutputPortSettings.sMaiMultiSettings.eMClkRate = BRAP_OP_MClkRate_e256Fs;
#else
#if NEXUS_HDMI_MULTI_ON_I2S
    outputSettings.uOutputPortSettings.sI2sSettings.bAlignedToLRClk = false;
    outputSettings.uOutputPortSettings.sI2sSettings.ePll = g_NEXUS_audioModuleData.moduleSettings.defaultPll;
#else
    outputSettings.uOutputPortSettings.sMaiMultiSettings.ePll = g_NEXUS_audioModuleData.moduleSettings.defaultPll;
#endif    
#endif
    errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &outputSettings);
    /* This should never fail */
    BDBG_ASSERT(BERR_SUCCESS == errCode);
}

static void NEXUS_AudioOutput_P_SetMaiParams(BRAP_OutputPort port, BAVC_AudioSamplingRate sampleRate, unsigned pll)
{
    BERR_Code errCode;
    BRAP_OutputPortConfig outputSettings;

    errCode = BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, port, &outputSettings);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
    /* Fill up output settings */
    outputSettings.eOutputSR = sampleRate;

#if BCHP_CHIP == 7550
    outputSettings.uOutputPortSettings.sMaiMultiSettings.eMClkRate = BRAP_OP_MClkRate_e256Fs;
    BSTD_UNUSED(pll);
#else
#if NEXUS_HDMI_MULTI_ON_I2S
    outputSettings.uOutputPortSettings.sI2sSettings.ePll = pll;
#else
    outputSettings.uOutputPortSettings.sMaiMultiSettings.ePll = pll;
#endif    
#endif
    errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &outputSettings);
    /* This should never fail */
    BDBG_ASSERT(BERR_SUCCESS == errCode);
}
#endif

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
                &suggestions[i],
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
                &suggestions[i],
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
                &suggestions[i],
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
                &suggestions[i],
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
                &suggestions[i],
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
