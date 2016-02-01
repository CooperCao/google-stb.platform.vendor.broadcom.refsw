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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_encoder);

typedef struct NEXUS_RfAudioEncoder
{
    NEXUS_OBJECT(NEXUS_RfAudioEncoder);
    NEXUS_AudioInputObject connector;
    BRAP_ProcessingStageHandle stage;
    NEXUS_BtscEncodeSettings btscSettings;
    NEXUS_RfAudioEncoderSettings settings;
    NEXUS_AudioInput input;
    NEXUS_RfAudioEncoding encoding;
}NEXUS_RfAudioEncoder;

static BRAP_ProcessingType NEXUS_RfAudioEncoder_P_EncodingToProcessingType(NEXUS_RfAudioEncoding encoding)
{
    switch ( encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        return BRAP_ProcessingType_eBtsc;
    default:
        return BRAP_ProcessingType_eNone;
    }
}

static void NEXUS_RfAudioEncoder_P_GetDefaultBtscSettings(NEXUS_BtscEncodeSettings *pSettings)
{
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eBtsc, &g_NEXUS_StageSettings);
    pSettings->encodingMode = NEXUS_BtscEncodingMode_eStereo;
    pSettings->useDeEmphasizedSourceSignals = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bUseDeEmphasizedSourceSignals;
    pSettings->equivalentMode75us = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bEquivalenMode75MicroSec;
    pSettings->fmDeviationControl = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bOpFreqDevCtrl;
    pSettings->volume = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.uiVolumeLevel;
    pSettings->srdCalibrationFactor = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.ui32SRDCalibrationFactor;
    pSettings->mainChannel.preemphasis = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bMainChannelPreEmphasisOn;
    pSettings->mainChannel.fmDeviationControl = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bSumChanFreqDevCtrl;
    pSettings->diffChannel.clipping = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bClipDiffChannelData;
    pSettings->diffChannel.preemphasis = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bDiffChannelPreEmphasisOn;
    pSettings->diffChannel.fmDeviationControl = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bDiffChanFreqDevCtrl;
    pSettings->testTone.enabled = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.bFreqGeneration;
    pSettings->testTone.frequency = g_NEXUS_StageSettings.uConfigParams.sBTSCParams.ui32Frequency;
}


static void NEXUS_RfAudioEncoder_P_ApplyEncodingSettings(NEXUS_RfAudioEncoderHandle handle, NEXUS_RfAudioEncoding encoding, BRAP_ProcessingStageSettings *pSettings)
{
    switch ( encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:        
        pSettings->uConfigParams.sBTSCParams.bUseDeEmphasizedSourceSignals = handle->btscSettings.useDeEmphasizedSourceSignals;
        pSettings->uConfigParams.sBTSCParams.bEquivalenMode75MicroSec = handle->btscSettings.equivalentMode75us;
        pSettings->uConfigParams.sBTSCParams.bOpFreqDevCtrl = handle->btscSettings.fmDeviationControl;
        pSettings->uConfigParams.sBTSCParams.bBTSCMonoModeOn = (handle->btscSettings.encodingMode==NEXUS_BtscEncodingMode_eMono)?true:false;
        pSettings->uConfigParams.sBTSCParams.uiVolumeLevel = handle->btscSettings.volume;
        pSettings->uConfigParams.sBTSCParams.ui32SRDCalibrationFactor = handle->btscSettings.srdCalibrationFactor;
        pSettings->uConfigParams.sBTSCParams.bMainChannelPreEmphasisOn = handle->btscSettings.mainChannel.preemphasis;
        pSettings->uConfigParams.sBTSCParams.bSumChanFreqDevCtrl = handle->btscSettings.mainChannel.fmDeviationControl;
        pSettings->uConfigParams.sBTSCParams.bClipDiffChannelData = handle->btscSettings.diffChannel.clipping;
        pSettings->uConfigParams.sBTSCParams.bDiffChannelPreEmphasisOn = handle->btscSettings.diffChannel.preemphasis;
        pSettings->uConfigParams.sBTSCParams.bDiffChanFreqDevCtrl = handle->btscSettings.diffChannel.fmDeviationControl;
        pSettings->uConfigParams.sBTSCParams.bFreqGeneration = handle->btscSettings.testTone.enabled;
        pSettings->uConfigParams.sBTSCParams.ui32Frequency = handle->btscSettings.testTone.frequency;
        break;
    default:
        break;
    }
}

/***************************************************************************
Summary:
    Get default settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetDefaultSettings(
    NEXUS_RfAudioEncoderSettings *pSettings   /* [out] default settings */
    )
{
    if ( NULL == pSettings )
    {
        return;
    }
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->encoding = NEXUS_RfAudioEncoding_eBtsc;
}

/***************************************************************************
Summary:
    Open an Audio Encoder stage
***************************************************************************/
NEXUS_RfAudioEncoderHandle NEXUS_RfAudioEncoder_Open( /* attr{destructor=NEXUS_RfAudioEncoder_Close}  */
    const NEXUS_RfAudioEncoderSettings *pSettings
    )
{
    NEXUS_RfAudioEncoderSettings defaults;
    NEXUS_RfAudioEncoderHandle handle;
    BRAP_ProcessingType processingType;
    BERR_Code errCode;

    if ( NULL == pSettings )
    {
        NEXUS_RfAudioEncoder_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    processingType = NEXUS_RfAudioEncoder_P_EncodingToProcessingType(pSettings->encoding);
    if ( processingType == BRAP_ProcessingType_eNone )
    {
        BDBG_ERR(("RF Encoding %u is not supported", pSettings->encoding));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_encoding;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_RfAudioEncoder));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_RfAudioEncoder, handle);
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eRfEncoder, handle);
    handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    handle->encoding = pSettings->encoding;
    NEXUS_RfAudioEncoder_P_GetDefaultBtscSettings(&handle->btscSettings);

    BRAP_GetDefaultProcessingStageSettings(processingType, &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &handle->stage);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_stage;
    }
    
    handle->settings = *pSettings;
    
    return handle;

err_stage:
    BDBG_OBJECT_DESTROY(handle, NEXUS_RfAudioEncoder);
    BKNI_Free(handle);
err_malloc:
err_encoding:
    return NULL;
}


/***************************************************************************
Summary:
    Close an Audio Encoder stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
static void NEXUS_RfAudioEncoder_P_Finalizer(
    NEXUS_RfAudioEncoderHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_RfAudioEncoder, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BRAP_DestroyProcessingStage(handle->stage);
    NEXUS_OBJECT_DESTROY(NEXUS_RfAudioEncoder, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_RfAudioEncoder, NEXUS_RfAudioEncoder_Close);

/***************************************************************************
Summary:
    Get Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoderSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    if ( NULL == pSettings )
    {
        return;
    }
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(NEXUS_RfAudioEncoderSettings));
}


/***************************************************************************
Summary:
    Set Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_SetSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    if ( NULL == pSettings )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(NEXUS_RfAudioEncoderSettings));

    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Get Encoding-Specific Settings for an Audio Encoder stage
***************************************************************************/
void NEXUS_RfAudioEncoder_GetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_RfAudioEncoding encoding, /* the encoding for which you are retrieving settings. */
    NEXUS_RfAudioEncoderEncodingSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);

    if ( NULL == pSettings )
    {
        return;
    }

    pSettings->encoding = encoding;
    switch ( encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        BKNI_Memcpy(&pSettings->encodingSettings.btsc, &handle->btscSettings, sizeof(handle->btscSettings));
        break;
    default:
        break;
    }
}


/***************************************************************************
Summary:
    Set Encoding-Specific Settings for an Audio Encoder stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_SetEncodingSettings(
    NEXUS_RfAudioEncoderHandle handle,
    const NEXUS_RfAudioEncoderEncodingSettings *pSettings
    )
{
    BERR_Code errCode;
    bool hasSettings = true;

    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);

    if ( NULL == pSettings )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch ( pSettings->encoding )
    {
    case NEXUS_RfAudioEncoding_eBtsc:
        BKNI_Memcpy(&handle->btscSettings, &pSettings->encodingSettings.btsc, sizeof(handle->btscSettings));
        break;
    default:
        hasSettings = false;
        break;
    }
    
    if ( hasSettings && pSettings->encoding == handle->encoding )
    {
        BRAP_GetCurrentProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        NEXUS_RfAudioEncoder_P_ApplyEncodingSettings(handle, handle->encoding, &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Get the audio connector for an Audio Encoder stage

Description:
This is used for a direct connection to SPDIF, as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(spdif), NEXUS_RfAudioEncoder_GetConnector(RfAudioEncoder));

***************************************************************************/
NEXUS_AudioInput NEXUS_RfAudioEncoder_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_RfAudioEncoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    return &handle->connector;
}


/***************************************************************************
Summary:
Add an input to this processing stage

Description:
This is used to connect to the audio decoder as follows:

    NEXUS_RfAudioEncoder_AddInput(RfAudioEncoder, NEXUS_AudioDecoder_GetConnector(audioDecoder));

***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_AddInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != input);

    if ( handle->input )
    {
        BDBG_ERR(("An input is already connected to this processing stage"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Connect at input node */
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);

    if ( BERR_SUCCESS == errCode )
    {
        handle->input = input;
    }

    return errCode;
}

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_RemoveInput(
    NEXUS_RfAudioEncoderHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    BDBG_ASSERT(NULL != input);

    if ( handle->input != input )
    {
        BDBG_ERR(("Input %p is not connected to this processing stage", (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Disconnect at input node */
    handle->input = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}


/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_RfAudioEncoder_RemoveAllInputs(
    NEXUS_RfAudioEncoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);

    if ( handle->input )
    {
        return NEXUS_RfAudioEncoder_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

BRAP_ProcessingStageHandle NEXUS_RfAudioEncoder_P_GetStageHandle(NEXUS_RfAudioEncoderHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_RfAudioEncoder);
    
    /* Determine if encoding has changed and update settings if it has */
    if ( handle->encoding != handle->settings.encoding )
    {
        BERR_Code errCode;
        BRAP_ProcessingType processingType;
        processingType = NEXUS_RfAudioEncoder_P_EncodingToProcessingType(handle->settings.encoding);
        BRAP_GetDefaultProcessingStageSettings(processingType, &g_NEXUS_StageSettings);
        NEXUS_RfAudioEncoder_P_ApplyEncodingSettings(handle, handle->settings.encoding, &g_NEXUS_StageSettings);
        errCode = BRAP_SetProcessingStageSettings(handle->stage, &g_NEXUS_StageSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return NULL;
        }
        handle->encoding = handle->settings.encoding;
    }
        
    return handle->stage;
}

