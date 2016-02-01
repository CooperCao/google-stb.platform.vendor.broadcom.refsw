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
*   API name: RfAudioEncoder
*    Specific APIs related to RF Audio Encoding
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_rf_audio_encoder);

/***************************************************************************
Summary:
    Audio encoder object
 ***************************************************************************/
typedef struct NEXUS_RfAudioEncoder
{
    NEXUS_OBJECT(NEXUS_RfAudioEncoder);
    BAPE_RfEncoderHandle encoder;
    NEXUS_AudioInputObject connector;
    NEXUS_AudioInput input;
    char name[11];   /* RF ENCODER */
} NEXUS_RfAudioEncoder;

static BAPE_RfAudioEncoding NEXUS_Audio_P_RfEncodingToMagnum(NEXUS_RfAudioEncoding nexus);
static NEXUS_RfAudioEncoding NEXUS_Audio_P_RfEncodingToNexus(BAPE_RfAudioEncoding magnum);

void NEXUS_RfAudioEncoder_GetDefaultSettings(
    NEXUS_RfAudioEncoderSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->encoding = NEXUS_RfAudioEncoding_eBtsc;
}

NEXUS_RfAudioEncoderHandle NEXUS_RfAudioEncoder_Open( /* attr{destructor=NEXUS_RfAudioEncoder_Close}  */
    const NEXUS_RfAudioEncoderSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_RfAudioEncoderHandle handle;
    BAPE_RfEncoderSettings encoderSettings;
    BAPE_Connector path;
    NEXUS_RfAudioEncoderSettings defaults;
    
    if ( NULL == pSettings )
    {
        NEXUS_RfAudioEncoder_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_RfAudioEncoder));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    NEXUS_OBJECT_INIT(NEXUS_RfAudioEncoder, handle);
          
    BAPE_RfEncoder_GetDefaultSettings(&encoderSettings);
    errCode = BAPE_RfEncoder_Create(NEXUS_AUDIO_DEVICE_HANDLE,
                                  &encoderSettings,
                                  &handle->encoder);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_create;
    }

    BKNI_Snprintf(handle->name, sizeof(handle->name), "RF ENCODER");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eRfEncoder, handle);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_eCompressed;
    BAPE_RfEncoder_GetConnector(handle->encoder, &path);
    handle->connector.port = (size_t)path;

    errCode = NEXUS_RfAudioEncoder_SetSettings(handle, pSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_settings;
    }
        
    /* success */
    return handle;

err_settings:
    NEXUS_RfAudioEncoder_Close(handle);
    return NULL;
        
err_create:
    BDBG_OBJECT_DESTROY(handle, NEXUS_RfAudioEncoder);
    BKNI_Free(handle);
err_malloc:
    return NULL;
}

static void NEXUS_RfAudioEncoder_P_Finalizer(
    NEXUS_RfAudioEncoderHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_RfAudioEncoder, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_RfEncoder_Destroy(handle->encoder);
    NEXUS_OBJECT_DESTROY(NEXUS_RfAudioEncoder, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_RfAudioEncoder, NEXUS_RfAudioEncoder_Close);

void NEXUS_RfAudioEncoder_GetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoderSettings *pSettings    /* [out] Settings */
    )
{
    BAPE_RfEncoderSettings settings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_RfEncoder_GetSettings(handle->encoder, &settings);
    pSettings->encoding = NEXUS_Audio_P_RfEncodingToNexus(settings.encoding);
}

NEXUS_Error NEXUS_RfAudioEncoder_SetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderSettings *pSettings
    )
{
    BAPE_RfEncoderSettings settings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_RfEncoder_GetSettings(handle->encoder, &settings);
    settings.encoding = NEXUS_Audio_P_RfEncodingToMagnum(pSettings->encoding);
    if ( settings.encoding == BAPE_RfAudioEncoding_eMax )
    {
        BDBG_ERR(("Unsupported RF audio encoding mode"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_RfEncoder_SetSettings(handle->encoder, &settings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}


void NEXUS_RfAudioEncoder_GetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoding encoding,
    NEXUS_RfAudioEncoderEncodingSettings *pSettings    /* [out] Settings */
    )
{
    BAPE_RfAudioEncoding magnumEncoding;
    BAPE_RfEncoderEncodingSettings encodingSettings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != pSettings);
    pSettings->encoding = encoding;
    magnumEncoding = NEXUS_Audio_P_RfEncodingToMagnum(encoding);
    BAPE_RfEncoder_GetEncodingSettings(handle->encoder, magnumEncoding, &encodingSettings);
    switch ( encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        pSettings->encodingSettings.btsc.useDeEmphasizedSourceSignals = encodingSettings.encodingSettings.btsc.useDeEmphasizedSourceSignals;
        pSettings->encodingSettings.btsc.equivalentMode75us = encodingSettings.encodingSettings.btsc.equivalentMode75us;
        pSettings->encodingSettings.btsc.fmDeviationControl = encodingSettings.encodingSettings.btsc.fmDeviationControl;
        pSettings->encodingSettings.btsc.rightChannelMute = encodingSettings.encodingSettings.btsc.rightChannelMute;
        pSettings->encodingSettings.btsc.volume = encodingSettings.encodingSettings.btsc.volume;
        pSettings->encodingSettings.btsc.srdCalibrationFactor = encodingSettings.encodingSettings.btsc.srdCalibrationFactor;
        pSettings->encodingSettings.btsc.encodingMode = encodingSettings.encodingSettings.btsc.encodingMode == BAPE_BtscEncodingMode_eMono ? NEXUS_BtscEncodingMode_eMono : NEXUS_BtscEncodingMode_eStereo;
        pSettings->encodingSettings.btsc.mainChannel.preemphasis = encodingSettings.encodingSettings.btsc.mainChannel.preemphasis;
        pSettings->encodingSettings.btsc.mainChannel.fmDeviationControl = encodingSettings.encodingSettings.btsc.mainChannel.fmDeviationControl;
        pSettings->encodingSettings.btsc.diffChannel.clipping = encodingSettings.encodingSettings.btsc.diffChannel.clipping;
        pSettings->encodingSettings.btsc.diffChannel.preemphasis = encodingSettings.encodingSettings.btsc.diffChannel.preemphasis;
        pSettings->encodingSettings.btsc.diffChannel.fmDeviationControl = encodingSettings.encodingSettings.btsc.diffChannel.fmDeviationControl;
        pSettings->encodingSettings.btsc.testTone.enabled = encodingSettings.encodingSettings.btsc.testTone.enabled;
        pSettings->encodingSettings.btsc.testTone.frequency = encodingSettings.encodingSettings.btsc.testTone.frequency;
        break;
    default:
        break;
    }
}

NEXUS_Error NEXUS_RfAudioEncoder_SetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderEncodingSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_RfAudioEncoding magnumEncoding;
    BAPE_RfEncoderEncodingSettings encodingSettings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != pSettings);
    magnumEncoding = NEXUS_Audio_P_RfEncodingToMagnum(pSettings->encoding);
    BAPE_RfEncoder_GetEncodingSettings(handle->encoder, magnumEncoding, &encodingSettings);
    switch ( pSettings->encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        encodingSettings.encodingSettings.btsc.useDeEmphasizedSourceSignals = pSettings->encodingSettings.btsc.useDeEmphasizedSourceSignals;
        encodingSettings.encodingSettings.btsc.equivalentMode75us = pSettings->encodingSettings.btsc.equivalentMode75us;
        encodingSettings.encodingSettings.btsc.fmDeviationControl = pSettings->encodingSettings.btsc.fmDeviationControl;
        encodingSettings.encodingSettings.btsc.rightChannelMute = pSettings->encodingSettings.btsc.rightChannelMute;
        encodingSettings.encodingSettings.btsc.volume = pSettings->encodingSettings.btsc.volume;
        encodingSettings.encodingSettings.btsc.srdCalibrationFactor = pSettings->encodingSettings.btsc.srdCalibrationFactor;
        encodingSettings.encodingSettings.btsc.encodingMode = pSettings->encodingSettings.btsc.encodingMode == NEXUS_BtscEncodingMode_eMono ? BAPE_BtscEncodingMode_eMono : BAPE_BtscEncodingMode_eStereo;
        encodingSettings.encodingSettings.btsc.mainChannel.preemphasis = pSettings->encodingSettings.btsc.mainChannel.preemphasis;
        encodingSettings.encodingSettings.btsc.mainChannel.fmDeviationControl = pSettings->encodingSettings.btsc.mainChannel.fmDeviationControl;
        encodingSettings.encodingSettings.btsc.diffChannel.clipping = pSettings->encodingSettings.btsc.diffChannel.clipping;
        encodingSettings.encodingSettings.btsc.diffChannel.preemphasis = pSettings->encodingSettings.btsc.diffChannel.preemphasis;
        encodingSettings.encodingSettings.btsc.diffChannel.fmDeviationControl = pSettings->encodingSettings.btsc.diffChannel.fmDeviationControl;
        encodingSettings.encodingSettings.btsc.testTone.enabled = pSettings->encodingSettings.btsc.testTone.enabled;
        encodingSettings.encodingSettings.btsc.testTone.frequency = pSettings->encodingSettings.btsc.testTone.frequency;
        break;
    default:
        BDBG_ERR(("RF Audio Encoding %u not supported", pSettings->encoding));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_RfEncoder_SetEncodingSettings(handle->encoder, &encodingSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

NEXUS_AudioInput NEXUS_RfAudioEncoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_RfAudioEncoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    return &handle->connector;
}

NEXUS_Error NEXUS_RfAudioEncoder_AddInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != input);

    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_RfEncoder_AddInput(handle->encoder, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_RfEncoder_RemoveInput(handle->encoder, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }

    handle->input = input;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_RfAudioEncoder_RemoveInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != input);
    if ( input != handle->input )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    (void)BAPE_RfEncoder_RemoveInput(handle->encoder, (BAPE_Connector)input->port);
    handle->input = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

NEXUS_Error NEXUS_RfAudioEncoder_RemoveAllInputs(
    NEXUS_RfAudioEncoderHandle handle
    )
{
    if ( handle->input ) 
    {
        return NEXUS_RfAudioEncoder_RemoveInput(handle, handle->input);
    }
    return NEXUS_SUCCESS;
}

static BAPE_RfAudioEncoding NEXUS_Audio_P_RfEncodingToMagnum(NEXUS_RfAudioEncoding nexus)
{
    switch ( nexus )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        return BAPE_RfAudioEncoding_eBtsc;
    default:
        return BAPE_RfAudioEncoding_eMax;
    }
}

static NEXUS_RfAudioEncoding NEXUS_Audio_P_RfEncodingToNexus(BAPE_RfAudioEncoding magnum)
{
    switch ( magnum )
    {
    case BAPE_RfAudioEncoding_eBtsc:
        return NEXUS_RfAudioEncoding_eBtsc;
    default:
        return NEXUS_RfAudioEncoding_eMax;
    }
}
