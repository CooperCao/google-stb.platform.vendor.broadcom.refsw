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
*   API name: I2sInput
*    Specific APIs related to I2S audio inputs.
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_i2s_input);

#if NEXUS_NUM_I2S_INPUTS

typedef struct NEXUS_I2sInput
{
    NEXUS_OBJECT(NEXUS_I2sInput);
    unsigned index;
    NEXUS_AudioInputObject connector;
    BAPE_I2sInputHandle input;
    BAPE_InputCaptureHandle inputCapture;
    BAPE_DecoderHandle decoder;
    NEXUS_I2sInputSettings settings;
    bool started;
    char name[12];   /* I2S INPUT %d */
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
    BAPE_I2sInputSettings inputSettings;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BAPE_I2sInput_GetDefaultSettings(&inputSettings);
    pSettings->sampleRate = 48000;
    pSettings->bitsPerSample = inputSettings.bitsPerSample;
    pSettings->lsbAtLRClock = (inputSettings.justification == BAPE_I2sJustification_eLsbFirst)?true:false;
    pSettings->alignedToLRClock = (inputSettings.dataAlignment == BAPE_I2sDataAlignment_eAligned)?true:false;
    pSettings->lrClkPolarity = (inputSettings.lrPolarity == BAPE_I2sLRClockPolarity_eLeftHigh)?true:false;
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
    NEXUS_I2sInputHandle handle; 
    NEXUS_I2sInputSettings defaults;   
    BAPE_I2sInputSettings settings;
    BAPE_InputPort inputPort;
    BAPE_Connector connector;
    BERR_Code errCode;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if ( index >= audioCapabilities.numOutputs.i2s )
    {
        BDBG_ERR(("I2sInput %u not supported on this chipset", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_I2sInput_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_I2sInput));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(handle, 0, sizeof(NEXUS_I2sInput));
    NEXUS_OBJECT_INIT(NEXUS_I2sInput, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "I2S INPUT %u", index);
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eI2s, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connector, Open);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    handle->index = index;
    
    BAPE_I2sInput_GetDefaultSettings(&settings);
    errCode = BAPE_I2sInput_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &settings, &handle->input);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_open;
    }
    if ( g_NEXUS_audioModuleData.settings.routeInputsToDsp )
    {
        BAPE_DecoderOpenSettings decoderSettings;
        BAPE_Decoder_GetDefaultOpenSettings(&decoderSettings);
        errCode = BAPE_Decoder_Open(NEXUS_AUDIO_DEVICE_HANDLE, NEXUS_AUDIO_DECODER_INDEX(NEXUS_AudioInputType_eI2s, index), &decoderSettings, &handle->decoder);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_decoder;
        }
        BAPE_Decoder_GetConnector(handle->decoder, BAPE_ConnectorFormat_eStereo, &connector);
    }
    else
    {
        BAPE_InputCaptureOpenSettings inputCaptureSettings;
        BAPE_InputCapture_GetDefaultOpenSettings(&inputCaptureSettings);
        errCode = BAPE_InputCapture_Open(NEXUS_AUDIO_DEVICE_HANDLE, NEXUS_AUDIO_CAPTURE_INDEX(NEXUS_AudioInputType_eI2s, index), &inputCaptureSettings, &handle->inputCapture);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_input_capture;
        }
        BAPE_InputCapture_GetConnector(handle->inputCapture, &connector);
    }
    handle->connector.port = (size_t)connector;
    BAPE_I2sInput_GetInputPort(handle->input, &inputPort);
    handle->connector.inputPort = (size_t)inputPort;

    errCode = NEXUS_I2sInput_SetSettings(handle, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_settings;
    }

    return handle;

err_settings:
    if ( g_NEXUS_audioModuleData.settings.routeInputsToDsp )
    {
        BAPE_Decoder_Close(handle->decoder);
    }
    else
    {
        BAPE_InputCapture_Close(handle->inputCapture);
    }
err_input_capture:
err_decoder:
    BAPE_I2sInput_Close(handle->input);
err_open:
    BDBG_OBJECT_DESTROY(handle, NEXUS_I2sInput);
    BKNI_Free(handle);
err_malloc:
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
    NEXUS_OBJECT_ASSERT(NEXUS_I2sInput, handle);
    if ( handle->started )
    {
        NEXUS_I2sInput_Stop(handle);
    }
    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }
    if ( handle->inputCapture )
    {
        BAPE_InputCapture_Close(handle->inputCapture);
    }
    if ( handle->decoder )
    {
        BAPE_Decoder_Close(handle->decoder);
    }
    BAPE_I2sInput_Close(handle->input);
    NEXUS_OBJECT_DESTROY(NEXUS_I2sInput, handle);
    BKNI_Free(handle);
}

static void NEXUS_I2sInput_P_Release(NEXUS_I2sInputHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connector, Close);
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
    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(*pSettings));
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
    BERR_Code errCode;
    BAPE_I2sInputSettings inputSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_I2sInput);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_I2sInput_GetSettings(handle->input, &inputSettings);
    inputSettings.sampleRate = pSettings->sampleRate;
    inputSettings.bitsPerSample = pSettings->bitsPerSample;
    inputSettings.justification = (pSettings->lsbAtLRClock)?BAPE_I2sJustification_eLsbFirst:BAPE_I2sJustification_eMsbFirst;
    inputSettings.dataAlignment = (pSettings->alignedToLRClock)?BAPE_I2sDataAlignment_eAligned:BAPE_I2sDataAlignment_eDelayed;
    inputSettings.lrPolarity = (pSettings->lrClkPolarity)?BAPE_I2sLRClockPolarity_eLeftHigh:BAPE_I2sLRClockPolarity_eLeftLow;
    errCode = BAPE_I2sInput_SetSettings(handle->input, &inputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Save other settings for now */
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(*pSettings));

    return BERR_SUCCESS;
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
    BERR_Code errCode;
    BAPE_InputPort inputPort;

    BAPE_I2sInput_GetInputPort(handle->input, &inputPort);
    errCode = NEXUS_AudioInput_P_PrepareToStart(&handle->connector);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if ( handle->inputCapture )
    {
        BAPE_InputCaptureStartSettings captureStartSettings;

        BAPE_InputCapture_GetDefaultStartSettings(&captureStartSettings);
        captureStartSettings.input = inputPort;
        errCode = BAPE_InputCapture_Start(handle->inputCapture, &captureStartSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        BAPE_DecoderStartSettings decoderStartSettings;

        BAPE_Decoder_GetDefaultStartSettings(&decoderStartSettings);
        decoderStartSettings.inputPort = inputPort;
        decoderStartSettings.codec = BAVC_AudioCompressionStd_ePcm;
        decoderStartSettings.streamType = BAVC_StreamType_ePes;
        decoderStartSettings.decodeRateControl = false;
        decoderStartSettings.ppmCorrection = false;
        decoderStartSettings.delayMode = BAPE_DspDelayMode_eLowVariable;
        errCode = BAPE_Decoder_Start(handle->decoder, &decoderStartSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }        
    }
    
    handle->started = true;
    return BERR_SUCCESS;
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
    if ( !handle->started )
    {
        return;
    }
    if ( handle->inputCapture )
    {
        BAPE_InputCapture_Stop(handle->inputCapture);
    }
    else
    {
        BAPE_Decoder_Stop(handle->decoder);
    }
    handle->started = false;
}

/***************************************************************************
Summary:
    Get an audio connector for use with downstream components
See Also:

 ***************************************************************************/
NEXUS_AudioInputHandle NEXUS_I2sInput_GetConnector(
    NEXUS_I2sInputHandle handle
    )
{
    return (&handle->connector);
}

/***************************************************************************
Summary:
    Determine if a capture channel is running.
 ***************************************************************************/
bool NEXUS_I2sInput_P_IsRunning(
    NEXUS_I2sInputHandle handle
    )
{
    return handle->started;
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

static void NEXUS_I2sInput_P_Release(NEXUS_I2sInputHandle handle)
{
    BSTD_UNUSED(handle);
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
NEXUS_AudioInputHandle NEXUS_I2sInput_GetConnector(
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

#endif  /* #if NEXUS_NUM_I2S_INPUTS */

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_I2sInput, NEXUS_I2sInput_Close);
