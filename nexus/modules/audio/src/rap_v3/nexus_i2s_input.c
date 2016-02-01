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
*   API name: I2sInput
*    Specific APIs related to I2S audio inputs.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"
#include "priv/nexus_timebase_priv.h"

BDBG_MODULE(nexus_i2s_input);

#if NEXUS_NUM_I2S_INPUTS

typedef struct NEXUS_I2sInput
{
    NEXUS_OBJECT(NEXUS_I2sInput);
    bool running;
    bool opened;
    NEXUS_I2sInputSettings settings;
    BRAP_ChannelHandle rapChannel;
    NEXUS_AudioAssociationHandle association;
    NEXUS_AudioInputObject connector;
} NEXUS_I2sInput;

static NEXUS_I2sInput g_i2sInputs[NEXUS_NUM_I2S_INPUTS];
static NEXUS_Error NEXUS_I2sInput_P_ConnectionChange(void *pParam, NEXUS_AudioInput input);

/***************************************************************************
Summary:
    Get default settings for an I2S Input
See Also:

 ***************************************************************************/
void NEXUS_I2sInput_GetDefaultSettings(
    NEXUS_I2sInputSettings *pSettings      /* [out] default settings */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->timebase = NEXUS_Timebase_eInvalid;
    pSettings->sampleRate = 48000;  /* Default is 48kHz */
    pSettings->bitsPerSample = 16;  /* Default 16-bit data */
    pSettings->leftVolume = pSettings->rightVolume = NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
}

/***************************************************************************
Summary:
    Open an I2S input.
See Also:
    NEXUS_I2sInput_Close
 ***************************************************************************/
NEXUS_I2sInputHandle NEXUS_I2sInput_Open(
    unsigned index,
    const NEXUS_I2sInputSettings *pSettings
    )
{
    NEXUS_Error errCode;
    NEXUS_I2sInput *pInput;
    BRAP_InputPortConfig inputConfig;    

    if ( index >= NEXUS_NUM_I2S_INPUTS )
    {
        BDBG_ERR(("I2S Input %u not supported on this chipset", index));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    pInput = &g_i2sInputs[index];
    if ( pInput->opened  )
    {
        BDBG_ERR(("I2S Input %u already open", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }


    errCode = BRAP_GetDefaultInputConfig(g_NEXUS_audioModuleData.hRap, BRAP_CapInputPort_eExtI2s0, &inputConfig);
    if ( errCode )
    {   
        BDBG_ERR(("Get Default I2S Input Config failed"));
        errCode = BERR_TRACE(errCode);
        return NULL;
    }

    inputConfig.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;
#if NEXUS_NUM_I2S_INPUTS > 1
#error Currently we only support one I2S Input on the version 2 raptor chips
#endif
    inputConfig.eCapPort = BRAP_CapInputPort_eExtI2s0; /* TODO: Support > 1 -- Enum has no values for this */
    /* These are only default values.  Proper values are set at start time */
    inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
    inputConfig.eSampleRate = BAVC_AudioSamplingRate_e48k;

    errCode = BRAP_SetInputConfig(g_NEXUS_audioModuleData.hRap, &inputConfig);
    if ( errCode )
    {
        BDBG_ERR(("Set input port config failed"));
        errCode = BERR_TRACE(errCode);
        return NULL;
    }

    /* Initialize object */
    NEXUS_OBJECT_INIT(NEXUS_I2sInput, pInput);
    NEXUS_AUDIO_INPUT_INIT(&pInput->connector, NEXUS_AudioInputType_eI2s, pInput);
    pInput->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    pInput->connector.disconnectCb = NEXUS_I2sInput_P_ConnectionChange;

    /* Done, mark channel as open */
    pInput->opened = true;

    /* Update Settings - handles NULL case */
    NEXUS_I2sInput_SetSettings(pInput, pSettings);

    return pInput;
}

/***************************************************************************
Summary:
    Close an I2S Input
See Also:
    NEXUS_I2sInput_Open
 ***************************************************************************/
static void NEXUS_I2sInput_P_Finalizer(
    NEXUS_I2sInputHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_I2sInput, handle);

    if ( handle->running )
    {
        BDBG_ERR(("Automatically stopping I2sInput %p on close", (void *)handle));
        NEXUS_I2sInput_Stop(handle);
    }

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_I2sInput, handle);
    BKNI_Memset(handle, 0, sizeof(NEXUS_I2sInput));
}

/***************************************************************************
Summary:
    Get settings for an I2S input
See Also:
    NEXUS_I2sInput_SetSettings
 ***************************************************************************/
void NEXUS_I2sInput_GetSettings(
    NEXUS_I2sInputHandle handle,
    NEXUS_I2sInputSettings *pSettings  /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set settings for an I2S input
See Also:
    NEXUS_I2sInput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_I2sInput_SetSettings(
    NEXUS_I2sInputHandle handle,
    const NEXUS_I2sInputSettings *pSettings    /* [in] Settings */
    )
{
    NEXUS_Error errCode = BERR_SUCCESS;;

    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    if ( NULL == pSettings )
    {
        NEXUS_I2sInput_GetDefaultSettings(&handle->settings);
    }
    else
    {
        handle->settings = *pSettings;
    }

    /* Timebase and SR will be applied on next start call */

    if ( handle->rapChannel )
    {
        /* Apply volume to raptor if channel exists */
        errCode = NEXUS_AudioCaptureChannel_P_SetVolume(handle->rapChannel, handle->settings.leftVolume,
                                                        handle->settings.rightVolume, handle->settings.muted);
    }

    return errCode;
}

/***************************************************************************
Summary:
    Start capturing data from an I2S Input
See Also:
    NEXUS_I2sInput_Stop
 ***************************************************************************/
NEXUS_Error NEXUS_I2sInput_Start(
    NEXUS_I2sInputHandle handle
    )
{
    NEXUS_Error errCode;
    BRAP_InputPortConfig inputConfig;
    BRAP_ChannelParams *pChannelParams;

    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);

    if ( handle->running )
    {
        BDBG_ERR(("I2sInput %p is already running.  Please stop before starting again.", (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( NULL == handle->rapChannel )
    {
        handle->rapChannel = NEXUS_AudioCaptureChannel_P_Acquire();
        if ( NULL == handle->rapChannel )
        {
            BDBG_ERR(("Unable to allocate capture channel.  Please stop other audio inputs first."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        NEXUS_I2sInput_SetSettings(handle, &handle->settings);
    }
    if ( NULL == handle->association )
    {
        /* Look for outputs */
        NEXUS_AudioOutputList outputList;
        NEXUS_AudioInput_P_GetOutputs(&handle->connector, &outputList, true);
        if ( outputList.outputs[0] )
        {
            BRAP_AssociatedChanSettings associationSettings;
            BKNI_Memset(&associationSettings, 0, sizeof(associationSettings));
            associationSettings.sChDetail[0].hRapCh = handle->rapChannel;
            handle->association = NEXUS_AudioAssociation_P_Create(&associationSettings);
            if ( NULL == handle->association )
            {
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
    }

    NEXUS_AudioInput_P_PrepareToStart(&handle->connector, handle->association);

    BRAP_GetCurrentInputConfig(g_NEXUS_audioModuleData.hRap, BRAP_CapInputPort_eExtI2s0, &inputConfig);
    inputConfig.eSampleRate = NEXUS_AudioModule_P_SampleRate2Avc(handle->settings.sampleRate);
    inputConfig.sCapPortParams.uCapPortParams.sInputI2sParams.bLsbAtLRClk = handle->settings.lsbAtLRClock;
    inputConfig.sCapPortParams.uCapPortParams.sInputI2sParams.bAlignedToLRClk = handle->settings.alignedToLRClock;
    inputConfig.sCapPortParams.uCapPortParams.sInputI2sParams.bLRClkPolarity = handle->settings.lrClkPolarity;
    switch ( handle->settings.bitsPerSample )
    {
    default:
        BDBG_WRN(("Invalid bits per sample value (%d), defaulting to 16", handle->settings.bitsPerSample));
        /* Fall through */
    case 16:
        inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
        break;
    case 18:
        inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e18;
        break;
    case 20:
        inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e20;
        break;
    case 24:
        inputConfig.eInputBitsPerSample = BRAP_InputBitsPerSample_e24;
        break;
    }
    errCode = BRAP_SetInputConfig(g_NEXUS_audioModuleData.hRap, &inputConfig);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pChannelParams = BKNI_Malloc(sizeof(*pChannelParams));
    if ( NULL == pChannelParams )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BRAP_GetDefaultChannelParams(handle->rapChannel, pChannelParams);
    pChannelParams->eCapMode = BRAP_CaptureMode_eFullDuplex;
    pChannelParams->eCapInputPort = BRAP_CapInputPort_eExtI2s0;
    
    {
        unsigned timebaseIndex;
        errCode = NEXUS_Timebase_GetIndex(handle->settings.timebase, &timebaseIndex);
        if (errCode) return BERR_TRACE(errCode);
        pChannelParams->eTimebase = BAVC_Timebase_e0 + timebaseIndex;
    }
    pChannelParams->eInputSR = inputConfig.eSampleRate;

#if !NEXUS_GLOBAL_INDEPENDENT_VOLUME
    /***************************************************************************/
    /* This is required to avoid glitches in captured audio due to massive     */
    /* timebase differences between the capture buffer and the output ports    */
    /***************************************************************************/
    pChannelParams->bIndOpVolCtrl = false;      
#endif

    errCode = BRAP_StartChannel(handle->rapChannel, pChannelParams);
    BKNI_Free(pChannelParams);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Apply volume to raptor if channel exists */
    errCode = NEXUS_AudioCaptureChannel_P_SetVolume(handle->rapChannel, handle->settings.leftVolume,
                                                    handle->settings.rightVolume, handle->settings.muted);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->running = true;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Stop capturing data from an I2S Input
See Also:
    NEXUS_I2sInput_Start
 ***************************************************************************/
void NEXUS_I2sInput_Stop(
    NEXUS_I2sInputHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);

    if ( false == handle->running )
    {
        BDBG_ERR(("I2sInput %p is not running - can't stop", (void *)handle));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    errCode = BRAP_StopChannel(handle->rapChannel);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return;
    }

    handle->running = false;
}

/***************************************************************************
Summary:
    Get an audio connector for use with downstream components
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_I2sInput_GetConnector(
    NEXUS_I2sInputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    return &handle->connector;
}

/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_I2sInput_P_IsRunning(
    NEXUS_I2sInputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    return handle->running;
}

/***************************************************************************
Summary:
    Retrieve the raptor channel for an i2s input object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_I2sInput_P_GetChannel(
    NEXUS_I2sInputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    if ( NULL == handle->rapChannel )
    {
        handle->rapChannel = NEXUS_AudioCaptureChannel_P_Acquire();
        if ( NULL == handle->rapChannel )
        {
            BDBG_ERR(("Unable to allocate capture channel.  Please stop other audio inputs first."));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    return handle->rapChannel;
}

static NEXUS_Error NEXUS_I2sInput_P_ConnectionChange(void *pParam, NEXUS_AudioInput input)
{
    NEXUS_I2sInput *pDecoder = pParam;
    BSTD_UNUSED(input);
    BDBG_ASSERT(NULL != pDecoder);

    BDBG_MSG(("Audio I2S input %p connection change", pParam));
    if ( pDecoder->association )
    {
        BDBG_MSG(("Destroying association on connection change"));
        NEXUS_AudioAssociation_P_Destroy(pDecoder->association);
        pDecoder->association = NULL;
    }
    if ( pDecoder->rapChannel )
    {
        NEXUS_AudioCaptureChannel_P_Release(pDecoder->rapChannel);
        pDecoder->rapChannel = NULL;
    }
    return NEXUS_SUCCESS;
}
#else /* #if NEXUS_NUM_I2S_INPUTS */

typedef struct NEXUS_I2sInput
{
    NEXUS_OBJECT(NEXUS_I2sInput);
} NEXUS_I2sInput;


/***************************************************************************
Summary:
    Get default settings for an I2S Input
See Also:

 ***************************************************************************/
void NEXUS_I2sInput_GetDefaultSettings(
    NEXUS_I2sInputSettings *pSettings      /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Open an I2S input.
See Also:
    NEXUS_I2sInput_Close
 ***************************************************************************/
NEXUS_I2sInputHandle NEXUS_I2sInput_Open(
    unsigned index,
    const NEXUS_I2sInputSettings *pSettings
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
    Close an I2S Input
See Also:
    NEXUS_I2sInput_Open
 ***************************************************************************/
static void NEXUS_I2sInput_P_Finalizer(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

/***************************************************************************
Summary:
    Get settings for an I2S input
See Also:
    NEXUS_I2sInput_SetSettings
 ***************************************************************************/
void NEXUS_I2sInput_GetSettings(
    NEXUS_I2sInputHandle handle,
    NEXUS_I2sInputSettings *pSettings  /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

/***************************************************************************
Summary:
    Set settings for an I2S input
See Also:
    NEXUS_I2sInput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_I2sInput_SetSettings(
    NEXUS_I2sInputHandle handle,
    const NEXUS_I2sInputSettings *pSettings    /* [in] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);;
}

/***************************************************************************
Summary:
    Start capturing data from an I2S Input
See Also:
    NEXUS_I2sInput_Stop
 ***************************************************************************/
NEXUS_Error NEXUS_I2sInput_Start(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
    Stop capturing data from an I2S Input
See Also:
    NEXUS_I2sInput_Stop
 ***************************************************************************/
void NEXUS_I2sInput_Stop(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return ;
}

/***************************************************************************
Summary:
    Get an audio connector for use with downstream components
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_I2sInput_GetConnector(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_I2sInput_P_IsRunning(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return false;
}

/***************************************************************************
Summary:
    Retrieve the raptor channel for an i2s input object.
 ***************************************************************************/
BRAP_ChannelHandle NEXUS_I2sInput_P_GetChannel(
    NEXUS_I2sInputHandle handle
    )
{
    BSTD_UNUSED(handle);
    BDBG_WRN(("I2S Input not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}
#endif  /* #if NEXUS_NUM_I2S_INPUTS */

NEXUS_OBJECT_CLASS_MAKE(NEXUS_I2sInput, NEXUS_I2sInput_Close);

