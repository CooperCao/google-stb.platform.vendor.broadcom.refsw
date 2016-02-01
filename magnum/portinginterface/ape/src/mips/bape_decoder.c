/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bape_decoder);

BDBG_OBJECT_ID(BAPE_Decoder);

void BAPE_Decoder_GetDefaultOpenSettings(
    BAPE_DecoderOpenSettings *pSettings     /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->independentDelay = true;
}

BERR_Code BAPE_Decoder_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DecoderOpenSettings *pSettings, 
    BAPE_DecoderHandle *pHandle                 /* [out] */
    )
{
    BAPE_DecoderOpenSettings defaults;
    BAPE_DecoderHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( NULL == pSettings )
    {
        BAPE_Decoder_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    if ( index >= BAPE_CHIP_MAX_DECODERS )
    {
        BDBG_ERR(("This chip only supports %u decoders.  Cannot open decoder %u", BAPE_CHIP_MAX_DECODERS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->decoders[index] )
    {
        BDBG_ERR(("Decoder %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Decoder));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_Decoder));
    handle->deviceHandle = deviceHandle;
    handle->independentDelay = pSettings->independentDelay;
    handle->index = index;
    BKNI_Snprintf(handle->stereoName, sizeof(handle->stereoName), "Decoder %u S", index);
    BKNI_Snprintf(handle->multichannelName, sizeof(handle->stereoName), "Decoder %u M", index);
    BKNI_Snprintf(handle->compressedName, sizeof(handle->compressedName), "Decoder %u C", index);
    BAPE_P_InitMixerInput(&handle->stereoConnector, BAPE_MixerInputType_eDecoder, handle);
    handle->stereoConnector.pName = handle->stereoName;
    handle->stereoConnector.numChannelPairs = 1;
    handle->stereoConnector.useBufferPool = true;
    BAPE_P_InitMixerInput(&handle->multichannelConnector, BAPE_MixerInputType_eDecoder, handle);
    handle->multichannelConnector.pName = handle->multichannelName;
    handle->multichannelConnector.numChannelPairs = 3;    /* We only support 5.1 at the moment */
    handle->multichannelConnector.useBufferPool = true;
    BAPE_P_InitMixerInput(&handle->compressedConnector, BAPE_MixerInputType_eDecoder, handle);        
    handle->compressedConnector.pName = handle->compressedName;
    handle->compressedConnector.numChannelPairs = 1;
    handle->compressedConnector.compressed = true;
    handle->compressedConnector.useBufferPool = true;
    handle->state = BAPE_DecoderState_eStopped;
    handle->settings.dualMonoMode = BAPE_DualMonoMode_eStereo;
    handle->settings.outputMode = BAPE_ChannelMode_e2_0;
    handle->settings.outputLfe = false;
    BAPE_Decoder_GetDefaultStartSettings(&handle->startSettings);
    handle->tsmSettings.tsmEnabled = false;
    handle->tsmSettings.thresholds.discard = 3000;
    handle->tsmSettings.thresholds.grossAdjustment = 8;
    /* Zero for AC3/AC3+ settings is correct */
    handle->ac3Settings.codec = BAVC_AudioCompressionStd_eAc3;
    handle->ac3PlusSettings.codec = BAVC_AudioCompressionStd_eAc3Plus;
    handle->ac3Settings.codecSettings.ac3.drcMode = handle->ac3PlusSettings.codecSettings.ac3Plus.drcMode = BAPE_Ac3DrcMode_eRf;
    handle->ac3Settings.codecSettings.ac3.drcScaleHi = handle->ac3Settings.codecSettings.ac3.drcScaleLow =
        handle->ac3Settings.codecSettings.ac3Plus.drcScaleHi = handle->ac3Settings.codecSettings.ac3Plus.drcScaleLow = 100;
    handle->aacSettings.codec = BAVC_AudioCompressionStd_eAac;
    handle->aacPlusSettings.codec = BAVC_AudioCompressionStd_eAacPlus;
    /* TODO: AAC values */
    BDBG_OBJECT_SET(handle, BAPE_Decoder);

    errCode = BAPE_DspContext_P_Create(handle->deviceHandle->dsp, handle, &handle->dspContext);
    if ( errCode )
    {
        BDBG_OBJECT_DESTROY(handle, BAPE_Decoder);
        BKNI_Free(handle);
        return BERR_TRACE(errCode);
    }

    /* Success */
    *pHandle = handle;
    deviceHandle->decoders[index] = handle;
    return BERR_SUCCESS;
}

void BAPE_Decoder_Close(
    BAPE_DecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        BDBG_WRN(("Implicitly stopping decoder %u on shutdown.", handle->index));
        BAPE_Decoder_Stop(handle);
    }

    /* Disconnect from all mixers, post-processors, and groups */
    BAPE_MixerInput_P_Shutdown(&handle->stereoConnector);
    BAPE_MixerInput_P_Shutdown(&handle->multichannelConnector);
    BAPE_MixerInput_P_Shutdown(&handle->compressedConnector);

    /* Cleanup */
    BAPE_DspContext_P_Destroy(handle->dspContext);
    handle->deviceHandle->decoders[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_Decoder);
    BKNI_Free(handle);
}

void BAPE_Decoder_GetDefaultStartSettings(
    BAPE_DecoderStartSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = BAVC_AudioCompressionStd_eMax;
    pSettings->streamType = BAVC_StreamType_eTsMpeg;
    pSettings->ppmCorrection = true;
}

BERR_Code BAPE_Decoder_Start(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderStartSettings *pSettings
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    BDBG_MSG(("BAPE_Decoder_Start(%#x) [index %u]", handle, handle->index));

    if ( (NULL == pSettings->pContextMap && NULL == pSettings->inputPort) ||
         (NULL != pSettings->pContextMap && NULL != pSettings->inputPort) )
    {
        BDBG_ERR(("Must specify either an AVC context map or an input port to decode."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( pSettings->inputPort )
    {
        BDBG_ERR(("Decoding from an input port is not yet supported."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        BDBG_ERR(("Already running, cannot start"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle->stereoOutputs = BAPE_MixerInput_P_GetNumOutputs(&handle->stereoConnector) > 0 ? true : false;
    handle->multichannelOutputs = BAPE_MixerInput_P_GetNumOutputs(&handle->multichannelConnector) > 0 ? true : false;
    handle->compressedOutputs = BAPE_MixerInput_P_GetNumOutputs(&handle->compressedConnector) > 0 ? true : false;
    handle->startSettings = *pSettings;

    if ( (handle->compressedOutputs > 0) && (0 == (handle->stereoOutputs+handle->multichannelOutputs)) )
    {
        handle->passthrough = true;
    }
    else
    {
        handle->passthrough = false;
    }

    /* Prepare all branches to start */
    if ( handle->stereoOutputs )
    {
        bool ppmCorrection = pSettings->ppmCorrection;
        /* Don't enable PPM correction if we have compressed or multichannel outputs connected */
        if ( handle->multichannelOutputs || handle->compressedOutputs )
        {
            ppmCorrection = false;
        }
        errCode = BAPE_MixerInput_P_PrepareToStart(&handle->stereoConnector, ppmCorrection);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stereo_start_prep;
        }
    }
    if ( handle->multichannelOutputs )
    {
        errCode = BAPE_MixerInput_P_PrepareToStart(&handle->multichannelConnector, false);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_mc_start_prep;
        }
    }
    if ( handle->compressedOutputs )
    {
        errCode = BAPE_MixerInput_P_PrepareToStart(&handle->compressedConnector, false);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_cmp_start_prep;
        }
    }

    /* Prepare the DSP task */
    errCode = BAPE_DspContext_P_PrepareTask(handle->dspContext);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dsp_prep;
    }

    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        errCode = BAPE_DspContext_P_SetCodecSettings(handle->dspContext, &handle->ac3Settings);
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        errCode = BAPE_DspContext_P_SetCodecSettings(handle->dspContext, &handle->ac3PlusSettings);
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        errCode = BAPE_DspContext_P_SetCodecSettings(handle->dspContext, &handle->aacSettings);
        break;
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        errCode = BAPE_DspContext_P_SetCodecSettings(handle->dspContext, &handle->aacPlusSettings);
        break;
    default:
        break;
    }
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    /* Prepare all branches to start */
    if ( handle->stereoOutputs )
    {
        errCode = BAPE_MixerInput_P_Start(&handle->stereoConnector, BAVC_AudioSamplingRate_eUnknown);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_start_stereo;
        }
    }
    if ( handle->multichannelOutputs )
    {
        errCode = BAPE_MixerInput_P_Start(&handle->multichannelConnector, BAVC_AudioSamplingRate_eUnknown);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_start_multichannel;
        }
    }
    if ( handle->compressedOutputs )
    {
        errCode = BAPE_MixerInput_P_Start(&handle->compressedConnector, BAVC_AudioSamplingRate_eUnknown);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_start_compressed;
        }
    }
    
    /* Start the DSP task */
    errCode = BAPE_DspContext_P_StartTask(handle->dspContext);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_start_task;
    }

    /* Success */
    handle->state = BAPE_DecoderState_eStarted;
    return BERR_SUCCESS;

err_start_task:
    if ( handle->compressedOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->compressedConnector);
    }
err_start_compressed:
    if ( handle->multichannelOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->multichannelConnector);
    }
err_start_multichannel:
    if ( handle->stereoOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->stereoConnector);
    }
err_start_stereo:
err_codec_settings:
    BAPE_DspContext_P_CleanupTask(handle->dspContext);
err_dsp_prep:/* these stops reverse the PPM resource allocation */
err_cmp_start_prep:
    if ( handle->compressedOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->compressedConnector);
    }
err_mc_start_prep:
    if ( handle->multichannelOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->multichannelConnector);
    }
err_stereo_start_prep:
    if ( handle->stereoOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->stereoConnector);
    }

    return errCode;
}

void BAPE_Decoder_Stop(
    BAPE_DecoderHandle handle
    )
{
    bool unmute = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Stop(%#x) [index %u]", handle, handle->index));

    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_WRN(("Decoder %u Already Stopped.", handle->index));
        return;
    case BAPE_DecoderState_ePaused:
    case BAPE_DecoderState_eDisabledPaused:
        unmute = true;
        break;
    default:
        break;
    }

    handle->state = BAPE_DecoderState_eStopped;
    BAPE_DspContext_P_StopTask(handle->dspContext);
    if ( handle->compressedOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->compressedConnector);
    }
    if ( handle->multichannelOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->multichannelConnector);
    }
    if ( handle->stereoOutputs )
    {
        BAPE_MixerInput_P_Stop(&handle->stereoConnector);
    }
    BAPE_DspContext_P_CleanupTask(handle->dspContext);

    if ( unmute )
    {
        BAPE_MixerInput_P_SetMute(&handle->compressedConnector, false);
        BAPE_MixerInput_P_SetMute(&handle->multichannelConnector, false);
        BAPE_MixerInput_P_SetMute(&handle->stereoConnector, false);
    }
}

BERR_Code BAPE_Decoder_Pause(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Pause(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot pause.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        break;
    case BAPE_DecoderState_ePaused:
        BDBG_WRN(("Decoder %u already paused.", handle->index));
        return BERR_SUCCESS;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Pause.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_MixerInput_P_SetMute(&handle->stereoConnector, true);
    BAPE_MixerInput_P_SetMute(&handle->multichannelConnector, true);
    BAPE_MixerInput_P_SetMute(&handle->compressedConnector, true);

    errCode = BAPE_DspContext_P_PauseTask(handle->dspContext);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->state = BAPE_DecoderState_ePaused;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Resume(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Resume(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot resume.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        BDBG_WRN(("Decoder %u already running.", handle->index));
        return BERR_SUCCESS;
    case BAPE_DecoderState_ePaused:
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Resume.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BAPE_DspContext_P_ResumeTask(handle->dspContext);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BAPE_MixerInput_P_SetMute(&handle->stereoConnector, false);
    BAPE_MixerInput_P_SetMute(&handle->multichannelConnector, false);
    BAPE_MixerInput_P_SetMute(&handle->compressedConnector, false);

    handle->state = BAPE_DecoderState_eStarted;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Advance(
    BAPE_DecoderHandle handle,
    unsigned milliseconds           /* Milliseconds to advance */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Advance(%#x, %u) [index %u]", handle, milliseconds, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        BDBG_WRN(("Decoder %u running, can't advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_ePaused:
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BAPE_DspContext_P_AdvanceTask(handle->dspContext, milliseconds);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_DisableForFlush(
    BAPE_DecoderHandle handle
    )
{
    BAPE_DecoderState newState = BAPE_DecoderState_eMax;

    BDBG_MSG(("BAPE_Decoder_DisableForFlush(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot disable for flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        newState = BAPE_DecoderState_eDisabled;
        break;
    case BAPE_DecoderState_ePaused:
        newState = BAPE_DecoderState_eDisabledPaused;
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        /* No change */
        return BERR_SUCCESS;
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_DspContext_P_StopTask(handle->dspContext);
    handle->state = newState;

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Flush(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;
    bool paused = false;

    BDBG_MSG(("BAPE_Decoder_Flush(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
    case BAPE_DecoderState_ePaused:
        BDBG_ERR(("Decoder %u is not disabled, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eDisabled:
        break;
    case BAPE_DecoderState_eDisabledPaused:
        paused = true;
        break;
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* This is a bit brute force, but guarantees all the FW state is reset correctly. */
    BAPE_Decoder_Stop(handle);
    errCode = BAPE_Decoder_Start(handle, &handle->startSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if ( paused )
    {
        errCode = BAPE_Decoder_Pause(handle);
        if ( errCode )
        {
            /* Should never happen, but just for completeness */
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetTsmSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    )
{
    BKNI_EnterCriticalSection();
    BAPE_Decoder_GetTsmSettings_isr(handle, pSettings);
    BKNI_LeaveCriticalSection();
}

void BAPE_Decoder_GetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->tsmSettings;
}

BERR_Code BAPE_Decoder_SetTsmSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    )
{
    BERR_Code errCode;
    BKNI_EnterCriticalSection();
    errCode = BAPE_Decoder_SetTsmSettings_isr(handle, pSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        errCode = BAPE_DspContext_P_SetTsmSettings_isr(handle->dspContext, pSettings);
    }
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->tsmSettings = *pSettings;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_GetTsmStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    )
{
    BERR_Code errCode;
    BKNI_EnterCriticalSection();
    errCode = BAPE_Decoder_GetTsmStatus_isr(handle, pStatus);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_GetTsmStatus_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pStatus);

    errCode = BAPE_DspContext_P_GetTsmStatus_isr(handle->dspContext, pStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

void BAPE_Decoder_GetSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

BERR_Code BAPE_Decoder_SetSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        /* Pass settings to decoder */
        errCode = BAPE_DspContext_P_SetSettings(handle->dspContext, pSettings);
    }
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

void BAPE_Decoder_GetStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderStatus *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_DecoderStatus));
    if ( handle->state == BAPE_DecoderState_eStopped )
    {
        pStatus->codec = BAVC_AudioCompressionStd_eMax;
        pStatus->sampleRate = BAVC_AudioSamplingRate_eUnknown;
    }
    else
    {
        pStatus->codec = handle->startSettings.codec;
        BAPE_DspContext_P_GetStatus(handle->dspContext, pStatus);
    }
}

void BAPE_Decoder_GetCodecSettings(
    BAPE_DecoderHandle handle,
    BAVC_AudioCompressionStd codec,
    BAPE_DecoderCodecSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    switch ( codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        *pSettings = handle->ac3Settings;
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        *pSettings = handle->ac3PlusSettings;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        *pSettings = handle->aacSettings;
        break;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        *pSettings = handle->aacPlusSettings;
        break;
    default:
        break;
    }
    pSettings->codec = codec;
}

BERR_Code BAPE_Decoder_SetCodecSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    
    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        errCode = BAPE_DspContext_P_SetCodecSettings(handle->dspContext, pSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        handle->ac3Settings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        handle->ac3PlusSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        handle->aacSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        handle->aacPlusSettings = *pSettings;
        break;
    default:
        break;
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetStereoPcmConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->stereoConnector;
}

void BAPE_Decoder_GetMultichannelPcmConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->multichannelConnector;
}

void BAPE_Decoder_GetCompressedConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->compressedConnector;
}

void BAPE_Decoder_GetInterruptHandlers(
    BAPE_DecoderHandle handle,
    BAPE_DecoderInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_Decoder_SetInterruptHandlers(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderInterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}

void BAPE_Decoder_P_SetSampleRate_isr(BAPE_DecoderHandle handle, BAVC_AudioSamplingRate sampleRate)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    /* TODO: Reprogam adaptive rate controllers, handle post-processing. */

    BAPE_MixerInput_P_SetSampleRate_isr(&handle->stereoConnector, sampleRate);
    BAPE_MixerInput_P_SetSampleRate_isr(&handle->multichannelConnector, sampleRate);
    BAPE_MixerInput_P_SetSampleRate_isr(&handle->compressedConnector, sampleRate);
}

void BAPE_Decoder_GetDefaultCdbItbConfig(
    BAPE_DecoderHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(handle);
    BKNI_Memset(pConfig, 0, sizeof(BAVC_CdbItbConfig));
    pConfig->Cdb.Length = 256*1024;
    pConfig->Cdb.Alignment = 6; /* cache line on 4380 */
    pConfig->Itb.Length = 128*1024;
    pConfig->Itb.Alignment = 6; /* cache line on 4380 */
    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    pConfig->Cdb.LittleEndian = true;
    #endif
}

BERR_Code BAPE_Decoder_SetStcValid_isr(
    BAPE_DecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        return BAPE_DspContext_P_SetStcValid_isr(handle->dspContext);
    }
    return BERR_SUCCESS;
}

