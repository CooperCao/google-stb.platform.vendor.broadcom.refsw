/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_src_ctrl0.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"

BDBG_MODULE(bape_mixer);

BDBG_OBJECT_ID(BAPE_Mixer);
BDBG_OBJECT_ID(BAPE_MixerInput);
BDBG_OBJECT_ID(BAPE_MixerInputLink);
BDBG_OBJECT_ID(BAPE_MixerOutput);

#define BAPE_MIXER_INPUT_INDEX_INVALID ((unsigned)-1)

static unsigned BAPE_Mixer_P_FindInputIndex(BAPE_MixerHandle mixer, BAPE_MixerInput input);
static void BAPE_Mixer_P_GetDefaultInputVolume(BAPE_MixerInputVolume *pVolume);
static BERR_Code BAPE_Mixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index);
static BERR_Code BAPE_Mixer_P_ApplyOutputVolume(BAPE_MixerHandle mixer, BAPE_MixerOutput output);
static BERR_Code BAPE_Mixer_P_CreateLink(BAPE_MixerHandle mixer, BAPE_MixerInput input, unsigned index);
static void BAPE_Mixer_P_DestroyLink(BAPE_MixerHandle mixer, unsigned index);
static BERR_Code BAPE_Mixer_P_AllocateResources(BAPE_MixerHandle mixer);
static BERR_Code BAPE_Mixer_P_SetupAdaptiveRateControl(BAPE_MixerHandle mixer, BAPE_MixerInputLink *pLink, BAPE_MixerInput input, bool enabled);
static void BAPE_Mixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, BAVC_AudioSamplingRate sampleRate);
static void BAPE_Mixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_MixerInput input, BAVC_AudioSamplingRate inputRate, BAVC_AudioSamplingRate outputRate);
static void BAPE_Mixer_P_LoadOutputCoefs(BAPE_MixerHandle mixer, uint8_t mixerId, uint8_t portId, int32_t left, int32_t right);
static void BAPE_Mixer_P_LoadInputCoefs(BAPE_MixerHandle mixer, uint8_t mixerId, uint8_t portId, int32_t left0, int32_t right0, int32_t left1, int32_t right1);
static void BAPE_Mixer_P_EnableAdaptiveRateControl(BAPE_MixerHandle mixer, BAPE_MixerInput input, unsigned adaptRateId, unsigned sourceChannelId);
static void BAPE_Mixer_P_DisableAdaptiveRateControl(BAPE_MixerHandle mixer, unsigned adaptRateId);
static void BAPE_Mixer_P_SetAdaptiveRateFrequency_isr(BAPE_MixerHandle mixer, BAPE_MixerInput input, BAVC_AudioSamplingRate sampleRate);

void BAPE_Mixer_GetDefaultSettings(
    BAPE_MixerSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->defaultSampleRate = BAVC_AudioSamplingRate_e48k;
}

BERR_Code BAPE_Mixer_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    )
{
    BAPE_MixerHandle handle;
    BAPE_MixerSettings defaultSettings;
    unsigned i, j;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BAPE_Mixer_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Make sure a valid default rate has been set. */
    switch ( pSettings->defaultSampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:
    case BAVC_AudioSamplingRate_e44_1k:
    case BAVC_AudioSamplingRate_e48k:
        break;
    default:
        BDBG_ERR(("Mixers only support default sample rates of 32k, 44.1k or 48kHz."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( index >= BAPE_CHIP_MAX_MIXERS )
    {
        BDBG_ERR(("This chip only supports %u mixers.  Cannot open mixer %u", BAPE_CHIP_MAX_MIXERS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->mixers[index] )
    {
        BDBG_ERR(("Mixer %u already opened.", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Mixer));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Mixer));
    BDBG_OBJECT_SET(handle, BAPE_Mixer);
    handle->settings = *pSettings;
    handle->index = index;
    handle->deviceHandle = deviceHandle;
    deviceHandle->mixers[index] = handle;
    handle->sampleRate = BAVC_AudioSamplingRate_eUnknown;
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        for ( j = 0; j < BAPE_ChannelPair_eMax; j++ )
        {
            handle->dpMixers[j][i] = 0xff;
        }
    }
    handle->fs = BAPE_FS_INVALID;

    *pHandle = handle;

    return BERR_SUCCESS;
}

void BAPE_Mixer_Close(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    
    /* Make sure no inputs are running. */
    if ( handle->running )
    {
        BDBG_ERR(("Mixer %p (%d) is running.  Cannot close.", handle, handle->index));
        BDBG_ASSERT(false == handle->running);
        return;
    }

    /* Remove all inputs */
    BAPE_Mixer_RemoveAllInputs(handle);

    /* Remove all outputs */
    BAPE_Mixer_RemoveAllOutputs(handle);

    /* Unlink from device */
    handle->deviceHandle->mixers[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_Mixer);
    BKNI_Free(handle);
}

BERR_Code BAPE_Mixer_AddInput(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input,
    bool makeMaster                /* If true, this will be the master input for sample rate purposes */
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    if ( handle->running )
    {
        BDBG_ERR(("Cannot change inputs while a mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

#if BDBG_DEBUG_BUILD
    /* Make sure the same input isn't added multiple times */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            BDBG_ERR(("Cannot add the same input multiple times to a single mixer."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
#endif

    /* Find empty slot */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == NULL )
        {
            /* SW7408-257: release any previously-allocated resources for previously-added
            inputs first before connecting a link to another input, because otherwise
            the allocated resources for this mixer will not be enough to cover the new
            input. Also, need to release before connecting the new input, or the
            release process will try to release the new input's resources, too, and assert. */
            BAPE_Mixer_P_ReleaseResources(handle);

            errCode = BAPE_Mixer_P_CreateLink(handle, input, i);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
            handle->inputs[i] = input;
            BDBG_MSG(("Adding input %s to mixer %u as %s", input->pName, handle->index, makeMaster?"master":"slave"));
            if ( makeMaster )
            {
                if ( handle->master )
                {
                    BDBG_WRN(("Replacing current master input with new input"));
                }
                handle->master = input;
            }
            BAPE_Mixer_P_GetDefaultInputVolume(&handle->inputVolume[i]);
            return BERR_SUCCESS;
        }
    }

    /* Mixer has no available slots. */
    BDBG_ERR(("Mixer can not accept any more inputs.  Maximum inputs per mixer is %u.", BAPE_CHIP_MAX_MIXER_INPUTS));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_Mixer_RemoveInput(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    if ( handle->running )
    {
        BDBG_ERR(("Cannot change inputs while a mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find slot containing this input */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            BAPE_Mixer_P_ReleaseResources(handle);
            BAPE_Mixer_P_DestroyLink(handle, i);
            handle->inputs[i] = NULL;
            if ( handle->master == input )
            {
                BDBG_WRN(("Removing master input %p", input));
                handle->master = NULL;
            }
            return BERR_SUCCESS;
        }
    }

    /* Input not found. */
    BDBG_ERR(("Input %p is not connected to mixer %p", input, handle));
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

BERR_Code BAPE_Mixer_RemoveAllInputs(
    BAPE_MixerHandle handle
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] )
        {
            errCode = BAPE_Mixer_RemoveInput(handle, handle->inputs[i]);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Mixer_AddOutput(
    BAPE_MixerHandle handle,
    BAPE_MixerOutput output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);

    if ( output->mixer )
    {
        BDBG_ERR(("Output %p is already connected to mixer %p", output, output->mixer));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->running )
    {
        /* TODO: Add output on the fly -- mute, bind, program sample rate, start, unmute */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        unsigned i;
        /* Bind mixer and output.  Remainder is done at start time. */
        output->mixer = handle;
        BLST_S_INSERT_HEAD(&handle->outputList, output, node);
        for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
        {
            output->mixerId[i] = BAPE_MIXER_ID_INVALID;
        }
        output->dummyMixerId = BAPE_MIXER_ID_INVALID;
        BAPE_Mixer_P_ReleaseResources(handle);

        BDBG_MSG(("Added output %p '%s' to mixer %u [mixer rate %s]", output, output->pName, handle->index, BAPE_P_GetSampleRateString_isrsafe(handle->sampleRate)));
        if ( output->setTimingParams_isr && handle->sampleRate != BAVC_AudioSamplingRate_eUnknown )
        {
            output->setTimingParams_isr(output->pHandle, handle->sampleRate, handle->settings.outputTimebase);
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Mixer_RemoveOutput(
    BAPE_MixerHandle handle,
    BAPE_MixerOutput output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);

    if ( output->mixer != handle )
    {
        BDBG_ERR(("Output %p is not connected to mixer %p"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->running )
    {
        /* TODO: remove output on the fly -- mute, stop, unbind, unmute */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        /* Unbind mixer and output. */
        BAPE_Mixer_P_ReleaseResources(handle);
        output->mixer = NULL;
        BLST_S_REMOVE(&handle->outputList, output, BAPE_MixerOutputObject, node);
        BDBG_MSG(("Removed output %p '%s' from mixer %u [mixer rate %s]", output, output->pName, handle->index, BAPE_P_GetSampleRateString_isrsafe(handle->sampleRate)));
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Mixer_RemoveAllOutputs(
    BAPE_MixerHandle handle
    )
{
    BAPE_MixerOutput output;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    while ( (output=BLST_S_FIRST(&handle->outputList)) )
    {
        errCode = BAPE_Mixer_RemoveOutput(handle, output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static unsigned BAPE_Mixer_P_FindInputIndex(BAPE_MixerHandle handle, BAPE_MixerInput input)
{
    unsigned i;

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            return i;
        }
    }

    return BAPE_MIXER_INPUT_INDEX_INVALID;
}

BERR_Code BAPE_Mixer_GetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input,
    BAPE_MixerInputVolume *pVolume      /* [out] */
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", input, handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pVolume = handle->inputVolume[i];

    return BERR_SUCCESS;
}

BERR_Code BAPE_Mixer_SetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_MixerInput input,
    const BAPE_MixerInputVolume *pVolume
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", input, handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle->inputVolume[i] = *pVolume;
    /* Apply volume if input is running */
    if ( handle->running )
    {
        errCode = BAPE_Mixer_P_ApplyInputVolume(handle, i);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;    
}

BERR_Code BAPE_GetOutputVolume(
    BAPE_MixerOutput output,
    BAPE_OutputVolume *pVolume      /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
    BDBG_ASSERT(NULL != pVolume);
    
    *pVolume = output->volume;
    return BERR_SUCCESS;
}

BERR_Code BAPE_SetOutputVolume(
    BAPE_MixerOutput output,
    const BAPE_OutputVolume *pVolume
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
    BDBG_ASSERT(NULL != pVolume);

    output->volume = *pVolume;
    if ( output->mixer && output->mixer->running )
    {
        errCode = BAPE_Mixer_P_ApplyOutputVolume(output->mixer, output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_GetOutputDelayStatus(
    BAPE_MixerOutput output,
    BAPE_OutputDelayStatus *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
    BDBG_ASSERT(NULL != pStatus);

    pStatus->pathDelay = BAPE_CHIP_MAX_PATH_DELAY;
    pStatus->additionalDelay = (output->mixer)?output->mixer->settings.outputDelay:0;    
}   

void BAPE_GetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    uint32_t *pRampStep                 /* All mixers output output volume is changed by this amount
                                           every Fs while ramping.  Specified in 4.23 format. 
                                           Ignored for compressed data. */
    )
{
    uint32_t regVal;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pRampStep);
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP);
    *pRampStep = BCHP_GET_FIELD_DATA(regVal, AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP);
}

void BAPE_SetOutputVolumeRampStep(
    BAPE_Handle deviceHandle,
    const uint32_t rampStep             /* All mixers output output volume is changed by this amount
                                           every Fs while ramping.  Specified in 4.23 format. 
                                           Ignored for compressed data. */
    )
{
    uint32_t regVal;
    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    regVal = BREG_Read32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, rampStep);
    BREG_Write32(deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, regVal);
}

static void BAPE_Mixer_P_GetDefaultInputVolume(BAPE_MixerInputVolume *pVolume)
{
    unsigned i;
    BKNI_Memset(pVolume, 0, sizeof(*pVolume));
    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        pVolume->coefficients[i][i] = BAPE_VOLUME_NORMAL;
    }
}

static void BAPE_Mixer_P_LoadInputCoefs(BAPE_MixerHandle mixer, uint8_t mixerId, uint8_t portId, int32_t left0, int32_t right0, int32_t left1, int32_t right1)
{
    uint32_t regVal, regAddr, regOffset;
    uint32_t mixerNum, portNum;
    
    mixerNum = mixerId;
    portNum = portId;

    #ifdef BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF
    /* On chips without a ping/pong setup - program the mixer to freeze the coefficients while updating */
    regVal = BREG_Read32(mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF);
    regVal &= ~(1<<mixerNum);
    BREG_Write32(mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF, regVal);
    #endif

    /* -- From the RDB for 7408: (Same for other chips except for number of mixers, inputs, and coefficients)
    Lout = Left_Coef_0 * Lin + Right_Coef_0 * Rin;;;;  Rout = Left_Coef_1 * Lin + Right_Coef_1 * Rin.
    For input M of mixer N (where M ranges from 0 to 3 and N are ranges from 0 to 5), the index to the scaling coefficient array is at follow:
    Left_Coef_0 = coef[N*16 + M*4 + 0], Right_Coef_0 = coef[N*16 + M*4 + 1];;;;  Left_Coef_1 = coef[N*16 + M*4 + 2], Right_Coef_1 = coef[N*16 + M*4 + 3],
    For example, the scaling coefficients for input 0 of mixer 0 are
    Left_Coef_0 = coef[0], Right_Coef_0 = coef[1];;;;  Left_Coef_1 = coef[2], Right_Coef_1 = coef[3].
    The scaling coefficients for input 3 of mixer 1 are
    Left_Coef_0 = coef[28], Right_Coef_0 = coef[29];;;;  Left_Coef_1 = coef[30], Right_Coef_1 = coef[31].
    */
    
    regVal = 0; /* If this channel pair is not valid, it must be muted in the mixer */
                
    /* Compute left coefficient 0 offset */
    regOffset = (mixerNum * 4 * BAPE_CHIP_MAX_MIXER_INPUTS) + (portNum * 4);
    BDBG_ASSERT(regOffset < (BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_END-3));   /* Sanity check */
    regAddr = (regOffset * 4) + BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE;
    regVal = left0 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BDBG_CASSERT(BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE > BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE);
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
    #endif

    regAddr += 4;   /* Jump to Right_Coef_0 */
    regVal = right0 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
    #endif
    
    regAddr += 4;   /* Jump to Left_Coef_1 */
    regVal = left1 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
    #endif

    regAddr += 4;   /* Jump to Right_Coef_1 */
    regVal = right1 & BCHP_MASK(AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi, COEFFICIENTS);
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    /* Write pong coef also */
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr + (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE-BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE), regVal);
    #endif

    #ifdef BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF
    /* On chips without a ping/pong setup - program the mixer to load the coefficients after updating */
    regVal = BREG_Read32(mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF);
    regVal |= (1<<mixerNum);
    BREG_Write32(mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_USE_NEW_SCALING_COEFF, regVal);
    #endif
}

static BERR_Code BAPE_Mixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index)
{
    unsigned i, j;
    int32_t left0, right0, left1, right1;
    BAPE_MixerInputLink *pLink = mixer->pInputLinks[index];

    BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        for ( j = 0; j < BAPE_CHIP_MAX_MIXERS; j++ )
        {
            if ( mixer->dpMixers[i][j] != 0xff )
            {
                uint32_t mixerNum = mixer->dpMixers[i][j];
                uint32_t portNum = index;

                if ( pLink->input->numChannelPairs > i && !mixer->inputMuted[index] && !mixer->inputVolume[index].muted )
                {
                    left0 = mixer->inputVolume[index].coefficients[2*i][2*i];
                    right0 = mixer->inputVolume[index].coefficients[(2*i)+1][2*i];
                    left1 = mixer->inputVolume[index].coefficients[2*i][(2*i)+1];
                    right1 = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                }
                else
                {
                    left0 = right0 = left1 = right1 = 0;
                }

                BAPE_Mixer_P_LoadInputCoefs(mixer, mixerNum, portNum, left0, right0, left1, right1);
            }
        }
    }

    return BERR_SUCCESS;
}

static void BAPE_Mixer_P_LoadOutputCoefs(BAPE_MixerHandle mixer, uint8_t mixerId, uint8_t portId, int32_t left, int32_t right)
{
    uint32_t regAddr, regVal;
    const uint32_t rightRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;
    const uint32_t mixerRegOffset = BCHP_AUD_FMM_DP_CTRL0_MIXER10_LT_VOL_LEVEL-BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;
    
    if ( 1 == portId )
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER01_LT_VOL_LEVEL;
    }
    else
    {
        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL;
    }

    left = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL, left) &
            BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);
    right = BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL, right) &
            BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);
    
    regAddr += mixerRegOffset * ((uint32_t) mixerId);
    regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL, MIXER_OUTPUT_LEFT_VOLUME_LEVEL);            
    regVal |= left;
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);

    regAddr += rightRegOffset;
    regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER00_RT_VOL_LEVEL, MIXER_OUTPUT_RIGHT_VOLUME_LEVEL);
    regVal |= right;
    BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
}

static BERR_Code BAPE_Mixer_P_ApplyOutputVolume(BAPE_MixerHandle mixer, BAPE_MixerOutput output)
{
    unsigned i;
    uint32_t regVal, regAddr, mask;

    BDBG_ASSERT(mixer == output->mixer);

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        /* Convert mixer id to left volume register */
        if ( output->mixerId[i] != BAPE_MIXER_ID_INVALID )
        {
            uint32_t left, right;

            if ( mixer->compressed )
            {
                /* output scaling makes no sense for compressed outputs.  set to full scale. */
                left = right = BAPE_VOLUME_NORMAL;
            }
            else
            {
                left = output->volume.volume[i*2];
                right = output->volume.volume[(i*2)+1];
            }

            BDBG_MSG(("Channel Pair %u: Left Output Scaling 0x%x Right Output Scaling 0x%x", i, left, right));
            BAPE_Mixer_P_LoadOutputCoefs(mixer, BAPE_MIXER_ID_TO_MIXER(output->mixerId[i]), BAPE_MIXER_ID_TO_PORT(output->mixerId[i]), left, right);

            /* Mute in mixer if need be */
            regAddr = BAPE_P_GetMixerConfigAddress(BAPE_MIXER_ID_TO_MIXER(output->mixerId[i]));
            regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
            if ( BAPE_MIXER_ID_TO_PORT(output->mixerId[i]) )
            {
                mask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT1);
            }
            else
            {
                mask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT0);
            }
            if ( mixer->compressed )
            {
                mask |= BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA);
            }
            regVal &= ~mask;
            if ( output->muteInMixer && output->volume.muted )
            {
                regVal |= mask;
            }
            BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);

            if ( i == 0 && output->dummyMixerId != BAPE_MIXER_ID_INVALID )
            {
                BAPE_Mixer_P_LoadOutputCoefs(mixer, BAPE_MIXER_ID_TO_MIXER(output->dummyMixerId), BAPE_MIXER_ID_TO_PORT(output->dummyMixerId), left, right);

                /* Mute in mixer if need be */
                regAddr = BAPE_P_GetMixerConfigAddress(BAPE_MIXER_ID_TO_MIXER(output->dummyMixerId));
                regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
                if ( BAPE_MIXER_ID_TO_PORT(output->dummyMixerId) )
                {
                    mask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT1);
                }
                else
                {
                    mask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_MUTE_ENA_OUTPUT0);
                }
                if ( mixer->compressed )
                {
                    mask |= BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, VOLUME_ENA);
                }
                regVal &= ~mask;
                if ( output->muteInMixer && output->volume.muted )
                {
                    regVal |= mask;
                }
                BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
            }
        }
    }

    if ( output->setMute )
    {
        bool mute = output->volume.muted;

        /* If this mixer is providing compressed data and the input is muted, mute the output as well. */
        if ( mixer->compressed && mixer->inputMuted[0] )
        {
            mute = true;
        }

        /* Call the output's mute handler */
        BDBG_MSG(("Setting output mute to %u", output->volume.muted));
        output->setMute(output->pHandle, mute, false);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Mixer_P_CreateLink(BAPE_MixerHandle mixer, BAPE_MixerInput input, unsigned index)
{
    BAPE_MixerInputLink *pLink;
    unsigned i;

    pLink = BKNI_Malloc(sizeof(BAPE_MixerInputLink));
    if ( NULL == pLink )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    
    BKNI_Memset(pLink, 0, sizeof(BAPE_MixerInputLink));
    BDBG_OBJECT_SET(pLink, BAPE_MixerInputLink);
    pLink->input = input;
    pLink->mixer = mixer;
    mixer->pInputLinks[index] = pLink;
    mixer->inputMuted[index] = false;

    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
    {
        pLink->sourceChannelId[i] = 
            pLink->srcId[i] = 
            pLink->playbackId[i] = 
            pLink->adaptRateId[i] = 0xff;
    }

    BLST_S_INSERT_HEAD(&input->mixerList, pLink, node);

    return BERR_SUCCESS;
}

static void BAPE_Mixer_P_DestroyLink(BAPE_MixerHandle mixer, unsigned index)
{
    BAPE_MixerInputLink *pLink, *pPrev=NULL;

    BAPE_MixerInput input = mixer->inputs[index];

    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pPrev=pLink, pLink = BLST_S_NEXT(pLink, node) )
    {
        BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);

        if ( pLink->mixer == mixer )
        {
            /* We've found our match. Remove it. */
            if ( NULL == pPrev )
            {
                BLST_S_REMOVE_HEAD(&input->mixerList, node);
            }
            else
            {
                BLST_S_REMOVE_NEXT(&input->mixerList, pPrev, node);
            }
            BDBG_OBJECT_DESTROY(pLink, BAPE_MixerInputLink);
            BKNI_Free(pLink);
            return;
        }
    }
    mixer->pInputLinks[index] = NULL;
}

void BAPE_MixerInput_P_Shutdown(
    BAPE_MixerInput input
    )
{
    BAPE_MixerInputLink *pLink;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    while ( (pLink = BLST_S_FIRST(&input->mixerList)) )
    {
        BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);

        if ( pLink->mixer->running )
        {
            BDBG_ERR(("Cannot close mixer input %p while still connected to a running mixer.", input));
            BDBG_ASSERT(!pLink->mixer->running);
            return;
        }

        /* Remove this input from the mixer */
        BAPE_Mixer_RemoveInput(pLink->mixer, input);
    }
}

void BAPE_MixerInput_P_GetSourceChannels(
    BAPE_MixerInput input,
    BAPE_ChannelPair channelPair,
    uint8_t sourceChannels[BAPE_CHIP_MAX_SOURCE_CHANNELS],
    unsigned *pNumChannels
    )
{
    unsigned numChannels=0;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(NULL != pNumChannels);
    BDBG_ASSERT(channelPair < BAPE_ChannelPair_eMax);

    /* Sanity check */
    BDBG_ASSERT(input->numChannelPairs > channelPair);

    if ( input->group )
    {
        /* TODO: Handle group */
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        BAPE_MixerInputLink *pLink;

        for ( pLink = BLST_S_FIRST(&input->mixerList);
              pLink != NULL;
              pLink = BLST_S_NEXT(pLink, node) )
        {
            sourceChannels[numChannels] = pLink->sourceChannelId[channelPair];
            if ( sourceChannels[numChannels] != 0xff )
            {
                numChannels++;
            }
        }
    }

    *pNumChannels = numChannels;
}

BERR_Code BAPE_MixerInput_P_PrepareToStart(
    BAPE_MixerInput input,
    bool adaptiveRateControl
    )
{
    BAPE_MixerInputLink *pLink;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    BDBG_MSG(("Preparing to start mixer input %s", input->pName));

    if ( input->group )
    {
        /* TODO: Handle group */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Make sure we have at least one connection first */
    if ( NULL == BLST_S_FIRST(&input->mixerList) )
    {
        BDBG_ERR(("Input %s (%p) is not connected to any mixers.  Can't start.", input->pName, input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* For each mixer, allocate resources */
    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( !pLink->mixer->ready )
        {
            errCode = BAPE_Mixer_P_AllocateResources(pLink->mixer);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        errCode = BAPE_Mixer_P_SetupAdaptiveRateControl(pLink->mixer, pLink, input, adaptiveRateControl);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_MixerInput_P_Start(
    BAPE_MixerInput input,
    BAVC_AudioSamplingRate sampleRate
    )
{
    BAPE_MixerInputLink *pLink;
    unsigned i,j;
    uint32_t regVal, bitmask, regAddr;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    BDBG_MSG(("MixerInput %s starting", input->pName));

    if ( input->group )
    {
        /* TODO: Handle group */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    /* First, setup the sample rate since it may affect output programming. */
    BKNI_EnterCriticalSection();
    BAPE_MixerInput_P_SetSampleRate_isr(input, sampleRate);
    BKNI_LeaveCriticalSection();

    /* For each set of components... */
    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        BAPE_MixerOutput output;

        if ( pLink->mixer->running == 0 )
        {
            /* Enable all the mixer outputs */
            for ( output = BLST_S_FIRST(&pLink->mixer->outputList); 
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
                BDBG_ASSERT(NULL != output->enable);
                
                BDBG_MSG(("Enabling output %s (%p)", output->pName, output));                
                output->enable(output->pHandle);

                for ( i = 0; i < input->numChannelPairs; i++ )
                {
                    /* Start mixer output */
                    if ( 0 == BAPE_MIXER_ID_TO_PORT(output->mixerId[i]) )
                    {
                        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT0_ENA);
                    }
                    else
                    {
                        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT1_ENA);
                    }

                    BDBG_MSG(("Enabling mixer %d output port %d", BAPE_MIXER_ID_TO_MIXER(output->mixerId[i]), BAPE_MIXER_ID_TO_PORT(output->mixerId[i])));
                    bitmask <<= BAPE_MIXER_ID_TO_MIXER(output->mixerId[i]);
                    regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA);
                    regVal |= bitmask;
                    BREG_Write32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, regVal);
                }
                if ( output->dummyMixerId != BAPE_MIXER_ID_INVALID )
                {
                    /* Start mixer output */
                    if ( 0 == BAPE_MIXER_ID_TO_PORT(output->dummyMixerId) )
                    {
                        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT0_ENA);
                    }
                    else
                    {
                        bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT1_ENA);
                    }

                    BDBG_MSG(("Enabling mixer %d output port %d", BAPE_MIXER_ID_TO_MIXER(output->dummyMixerId), BAPE_MIXER_ID_TO_PORT(output->dummyMixerId)));
                    bitmask <<= BAPE_MIXER_ID_TO_MIXER(output->dummyMixerId);
                    regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA);
                    regVal |= bitmask;
                    BREG_Write32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, regVal);
                }
            }
        }
        pLink->mixer->running++;


        /* Enable SRC's and Mixers in the same order as RAP (for each channel pair, enable SRC then Mixer) */
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            uint32_t src = (pLink->input->mustBranchAtSrc)? pLink->input->srcId[i] : pLink->srcId[i];

            BDBG_MSG(("Enabling SRC %u", src));
            bitmask = BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, ENA0) << src;
            regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
            regVal |= bitmask;
            BREG_Write32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal);

            BDBG_MSG(("%u mixer groups", pLink->mixer->numMixerGroups));
            for ( j = 0; j < pLink->mixer->numMixerGroups; j++ )
            {
                int k;
                /* We need to enable the input that matches this connector */
                for ( k = 0; k < BAPE_CHIP_MAX_MIXER_INPUTS; k++ )
                {
                    if ( pLink->mixer->inputs[k] == input )
                    {
                        /* Got it.  Enable the input corresponding to this. */
                        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG + 
                            ((BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT10_CONFIG-BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG) * pLink->mixer->dpMixers[i][j]);
                        regAddr += (k/2)*4; /* Input config registers are setup in pairs. */
                        if ( 0 == (k%2) )
                        {
                            /* Even.  Write the first enable. */
                            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_ENA);
                        }
                        else
                        {
                            /* Odd.  Write the second enable. */
                            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_ENA);
                        }
                        BDBG_MSG(("Enabling mixer %u input port %u", pLink->mixer->dpMixers[i][j], k));
                        regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
                        regVal |= bitmask;
                        BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);
                    }
                } /* For Each Input To This DP Mixer */
            }   /* For each DP mixer in the APE_Mixer */            
        } /* For Each Channel Pair */

        /* Enable Source Channels */
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            uint32_t sourceChannel = (pLink->input->mustBranchAtSrc)? pLink->input->sourceChannelId[i] : pLink->sourceChannelId[i];

            BDBG_MSG(("Enabling Source Channel %u", sourceChannel));
            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*sourceChannel);
            regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
            regVal |= BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE);
            BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);
        }

        /* For inputs other than decoders, just start the source channel now */
        if ( input->type != BAPE_MixerInputType_eDecoder )
        {
            for ( i = 0; i < input->numChannelPairs; i++ )
            {
                uint32_t sourceChannel = (pLink->input->mustBranchAtSrc)? pLink->input->sourceChannelId[i] : pLink->sourceChannelId[i];

                BDBG_MSG(("Enabling PLAY_RUN for Source Channel %u", sourceChannel));
                regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*sourceChannel);
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
                regVal |= BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN);
                BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);
            }
        }
    }

    BDBG_MSG(("MixerInput %s started", input->pName));

    return BERR_SUCCESS;
}

void BAPE_MixerInput_P_Stop(
    BAPE_MixerInput input
    )
{
    BAPE_MixerInputLink *pLink;
    unsigned i, j, k, timeout;
    uint32_t regVal, bitmask, regAddr;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    BDBG_MSG(("MixerInput %s stopping", input->pName));

    if ( input->group )
    {
        /* TODO: Handle group */
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    /* For each set of components... */
    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        BAPE_MixerOutput output;

        /* Enable all the mixer inputs and cascade outputs  */
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            uint32_t sourceChannel;

            if ( pLink->input->mustBranchAtSrc )
            {
                sourceChannel = pLink->input->sourceChannelId[i];
            }
            else
            {
                sourceChannel = pLink->sourceChannelId[i];
            }
            /* For inputs other than decoders, stop the source channel here.  If the watchdog has fired, we also must clear PLAY_RUN */
            if ( input->type != BAPE_MixerInputType_eDecoder || pLink->mixer->deviceHandle->dsp->watchdogAsserted )
            {
                BDBG_MSG(("Disabling PLAY_RUN for Source Channel %u", sourceChannel));
                regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + (4*sourceChannel);
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
                regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CTRLi, PLAY_RUN);
                BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);
            }

            /* Wait for group flow to stop */
            for ( timeout = 1000; timeout > 0; timeout-- )
            {
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
                if ( 0 == (regVal & (1<<sourceChannel)) )
                {
                    break;
                }
                BKNI_Sleep(1);
            }
            if ( 0 == timeout )
            {
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_FLOWON);
                BDBG_WRN(("Timeout waiting for source channel %u flow to stop [0x%08x]", sourceChannel, regVal));
            }


            /* Disable Source Channels */
            BDBG_MSG(("Disabling Source Channel %u", sourceChannel));
            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*sourceChannel);
            regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE);
            BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);

            /* Wait for the source channel to stop */            
            for ( timeout = 1000; timeout > 0; timeout-- )
            {
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_ENABLE);
                if ( 0 == (regVal & (1<<sourceChannel)) )
                {
                    break;
                }
                BKNI_Sleep(1);
            }
            if ( 0 == timeout )
            {
                regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_SOURCECH_GROUP_ENABLE);
                BDBG_WRN(("Timeout waiting for source channel %u enable status to clear [0x%08x]", sourceChannel, regVal));
            }
        }

        /* Disable the mixer input into the mixers */
        for ( j = 0; j < BAPE_CHIP_MAX_MIXER_INPUTS; j++ )
        {
            if ( pLink->mixer->inputs[j] == input )
            {
                for ( i = 0; i < input->numChannelPairs; i++ )
                {
                    for ( k = 0; k < pLink->mixer->numMixerGroups; k++ )
                    {
                        /* Got it.  Enable the input corresponding to this. */
                        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG + 
                            ((BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT10_CONFIG-BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG) * pLink->mixer->dpMixers[i][k]);
                        regAddr += (j/2)*4; /* Input config registers are setup in pairs. */
                        if ( 0 == (j%2) )
                        {
                            /* Even.  Write the first enable. */
                            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_ENA);
                        }
                        else
                        {
                            /* Odd.  Write the second enable. */
                            bitmask = BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_ENA);
                        }
                        BDBG_MSG(("Disabling mixer %u input port %u", pLink->mixer->dpMixers[i][k], j));
                        regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, regAddr);
                        regVal &= ~bitmask;
                        BREG_Write32(pLink->mixer->deviceHandle->regHandle, regAddr, regVal);
                    }
                }
                break;
            }
        }

        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            uint32_t src;
            if ( pLink->input->mustBranchAtSrc )
            {
                src = pLink->input->srcId[i];
            }
            else
            {
                src = pLink->srcId[i];
            }
            /* Disable SRCs */
            BDBG_MSG(("Disabling SRC %u", src));
            bitmask = BCHP_MASK(AUD_FMM_SRC_CTRL0_STRM_ENA, ENA0) << src;
            regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA);
            regVal &= ~bitmask;
            BREG_Write32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_SRC_CTRL0_STRM_ENA, regVal);
        }

		if (pLink->mixer->running > 0)
		{
	        BDBG_MSG(("Mixer run count was %u now %u", pLink->mixer->running, pLink->mixer->running-1));
	        pLink->mixer->running--;
	    }
	    else
	    {
	        BDBG_MSG(("Mixer run count was already 0"));
	    }
	    
        /* If there are other running inputs, stop here. */
        if ( pLink->mixer->running == 0 )
        {
            /* We're the last running input.  Stop the mixers and outputs now. */
            BDBG_MSG(("Mixer %p (%d) has no more running inputs.  Stopping all mixers and outputs.", pLink->mixer, pLink->mixer->index));
            for ( i = 0; i < input->numChannelPairs; i++ )
            {
                /* Stop all mixer outputs */
                for ( j = 0; j < pLink->mixer->numMixerGroups; j++ )
                {
                    BDBG_MSG(("Disabling all outputs on mixer %u", pLink->mixer->dpMixers[i][j]));
                    bitmask = (BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT1_ENA) |
                               BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, MIXER0_OUTPUT0_ENA)) << pLink->mixer->dpMixers[i][j];
                    regVal = BREG_Read32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA);
                    regVal &= ~bitmask;
                    BREG_Write32(pLink->mixer->deviceHandle->regHandle, BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA, regVal);
                }            
            }
            for ( output = BLST_S_FIRST(&pLink->mixer->outputList); 
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                BDBG_MSG(("Disabling output %s (%p)", output->pName, output));
                
                BDBG_ASSERT(NULL != output->disable);
                output->disable(output->pHandle);    
            }
        }

        BAPE_Mixer_P_SetupAdaptiveRateControl(pLink->mixer, pLink, input, false);
    }


    BDBG_MSG(("MixerInput %s stopped", input->pName));        
}

static BAPE_MclkSource BAPE_Mixer_P_GetMclkSource(BAPE_MixerHandle mixer)
{
    unsigned mixerIndex;
    BAPE_MixerOutput output;
    bool needMclk = false;
    bool pllRequired = false;

    /* Do we even need an MCLK source? */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
        if ( output->setMclk_isr || output->fsTiming )
        {
            needMclk = true;
            break;
        }
    }

    if ( false == needMclk )
    {
        /* No outputs need MCLK */
        return BAPE_MclkSource_eNone;
    }

    /* Check local outputs first */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
        if ( output->mclkOutput != BAPE_MclkSource_eNone )
        {
            return output->mclkOutput;
        }
        else if ( output->pllRequired )
        {
            pllRequired = true;
        }
    }

    /* If an output desires a PLL, skip search for other MCLK source. */
    if ( pllRequired )
    {
        return BAPE_MclkSource_ePll0 + mixer->settings.outputPll;
    }

    /* Check all open mixers with matching inputs and settings  */
    for ( mixerIndex = 0; mixerIndex < BAPE_CHIP_MAX_MIXERS; mixerIndex++ )
    {
        int i, j;
        bool matchInput=true, matchSettings=false;
        BAPE_MixerHandle thisMixer;

        thisMixer = mixer->deviceHandle->mixers[mixerIndex];
        if ( NULL == thisMixer || thisMixer == mixer )
        {
            continue;
        }

        /* Determine if we have a matching timing or not */
        if ( mixer->master )
        {
            /* Must have the same master channel, timebase, and default sample rate */
            if ( thisMixer->master && 
                 (thisMixer->master->pHandle == mixer->master->pHandle) &&
                 (thisMixer->settings.outputTimebase == mixer->settings.outputTimebase) &&
                 (thisMixer->settings.defaultSampleRate == thisMixer->settings.defaultSampleRate) )
            {
                matchSettings = true;
            }
        }
        else
        {
            /* No master, just default timing. */
            if ( NULL == thisMixer->master &&
                 (thisMixer->settings.outputTimebase == mixer->settings.outputTimebase) &&
                 (thisMixer->settings.defaultSampleRate == mixer->settings.defaultSampleRate) )
            {
                matchSettings = true;
            }
        }

        /* Must match all inputs */
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( mixer->inputs[i] )
            {
                for ( j = 0; j < BAPE_CHIP_MAX_MIXER_INPUTS; j++ )
                {
                    if ( thisMixer->inputs[j] &&
                         (thisMixer->inputs[j]->pHandle == mixer->inputs[i]->pHandle) )
                    {
                        break;
                    }
                }
                if ( j >= BAPE_CHIP_MAX_MIXER_INPUTS )
                {
                    matchInput = false;
                    break;
                }
            }
            if ( thisMixer->inputs[i] )
            {
                for ( j = 0; j < BAPE_CHIP_MAX_MIXER_INPUTS; j++ )
                {
                    if ( mixer->inputs[j] &&
                         (mixer->inputs[j]->pHandle == thisMixer->inputs[i]->pHandle) )
                    {
                        break;
                    }
                }
                if ( j >= BAPE_CHIP_MAX_MIXER_INPUTS )
                {
                    matchInput = false;
                    break;
                }
            }
        }

        /* If we've matched settings and input(s), get look for an mclk source */
        if ( matchSettings && matchInput )
        {
            for ( output = BLST_S_FIRST(&thisMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
                if ( output->mclkOutput != BAPE_MclkSource_eNone )
                {
                    BDBG_MSG(("Found associated MCLK source %u", output->mclkOutput));
                    return output->mclkOutput;
                }
            }
        }
    }

    /* If we get here, we haven't found one.  Return the PLL. */
    return BAPE_MclkSource_ePll0 + mixer->settings.outputPll;
}

static BERR_Code BAPE_Mixer_P_AllocateResources(BAPE_MixerHandle mixer)
{
    unsigned i,j;
    BAPE_MixerOutput output;
    unsigned numChannelPairs=0;
    unsigned numOutputs=0;
    bool compressed = false;
    bool fsRequired = false;
    BERR_Code errCode=BERR_SUCCESS;
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);

    if ( mixer->ready )
    {
        BDBG_MSG(("Mixer %p (%d) already has resources allocated", mixer, mixer->index));
        return BERR_SUCCESS;
    }

    output = BLST_S_FIRST(&mixer->outputList);
    if ( NULL == output )
    {
        BDBG_ERR(("No outputs connected to mixer %p (%d), cannot allocate resources.", mixer, mixer->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* First determine number of channel pairs required */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] )
        {
            j = mixer->inputs[i]->numChannelPairs;
            if ( numChannelPairs == 0 && mixer->inputs[i]->compressed )
            {
                compressed = true;
            }
            else if ( compressed && !mixer->inputs[i]->compressed )
            {
                BDBG_ERR(("Cannot mix compressed and uncompressed data.  Unable to start mixer %p (%d)", mixer, mixer->index));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            if ( j > numChannelPairs )
            {
                numChannelPairs = j;
            }
        }
    }

    /* Make sure we actually found something worthwhile */
    if ( numChannelPairs == 0 )
    {
        BDBG_ERR(("No input data for mixer %p (%d).  Cannot start.", mixer, mixer->index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Make sure all outputs are capable of receiving this data format */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_MixerOutput);
        if ( output->maxChannelPairs < numChannelPairs )
        {
            BDBG_ERR(("Mixer %p (%d) cannot route %d channel pairs to output %p.  Cannot start.", mixer, mixer->index, numChannelPairs, output));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        numOutputs++;
    }

    mixer->numChannelPairs = numChannelPairs;
    mixer->compressed = compressed;
    mixer->numMixerGroups = numOutputs;

    BDBG_MSG(("Mixer %p (%d): Allocating mixers for %u outputs with %u channel pairs.", 
              mixer, mixer->index, numOutputs*numChannelPairs, numOutputs, numChannelPairs));

    /* This allocates the mixers and number of playbacks required to cascade, programming the cascade mixer inputs accordingly. */
    errCode = BAPE_P_AllocateMixers(mixer, numOutputs, numChannelPairs);
    if ( errCode )
    {
        BDBG_ERR(("Unable to allocate %u hardware mixers.  Please reduce the number of outputs and/or the number of output channels.", numOutputs * numChannelPairs));
        errCode = BERR_TRACE(errCode);
        goto err_mixers;
    }
    
    /* Now, determine the input resources needed.  For inputs that branch at SRC we allocate resources at the input level,
       for inputs that use multiple source channels, we allocate resources at the input link. */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] )
        {
            BDBG_ASSERT(false == mixer->pInputLinks[i]->active);
            if ( mixer->inputs[i]->useBufferPool )
            {
                if ( 0 == mixer->inputs[i]->activeLinks )
                {
                    BDBG_MSG(("Allocating buffers for input %s", mixer->inputs[i]->pName));
                    errCode = BAPE_P_AllocateInputBuffers(mixer->deviceHandle, mixer->inputs[i]);
                    if ( errCode )
                    {
                        errCode = BERR_TRACE(errCode);
                        goto err_input_resources;
                    }
                }
            }
            if ( mixer->inputs[i]->mustBranchAtSrc )
            {
                if ( 0 == mixer->inputs[i]->activeLinks )
                {
                    BDBG_MSG(("Mixer %p (%d) Allocating resources for input %p (%s) [branch at SRC]", mixer, mixer->index, mixer->inputs[i], mixer->inputs[i]->pName));
                    errCode = BAPE_P_AllocateInputResources(mixer->deviceHandle, mixer->inputs[i]);
                    if ( errCode )
                    {
                        errCode = BERR_TRACE(errCode);
                        goto err_input_resources;
                    }
                }
            }
            else
            {
                BDBG_MSG(("Mixer %p (%d) Allocating resources for input %p (%s) [No branch at SRC]", mixer, mixer->index, mixer->inputs[i], mixer->inputs[i]->pName));
                errCode = BAPE_P_AllocateInputLinkResources(mixer->deviceHandle, mixer->pInputLinks[i]);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_input_resources;
                }
            }
            mixer->pInputLinks[i]->active = true;
            mixer->inputs[i]->activeLinks++;
        }
    }

    /* Now, we have the resources we need.  Setup the linkage correctly. */   
    i=0;
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        /* Assign mixers to each output (cascade already setup) */
        for ( j = 0; j < numChannelPairs; j++ )
        {
            output->mixerId[j] = BAPE_MIXER_ID_CREATE(mixer->dpMixers[j][i], 0);   /* 0 is always for outputs, 1 always for dummy */
            regAddr = BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (4*output->streamId[j]);
            regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID, BAPE_MIXER_ID_TO_FCI(output->mixerId[j]));
            BDBG_MSG(("Linking output stream %u to FCI 0x%x", output->streamId[j], BAPE_MIXER_ID_TO_FCI(output->mixerId[j])));
            BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
        }
        for ( ; j < BAPE_ChannelPair_eMax; j++ )
        {
            output->mixerId[j] = BAPE_MIXER_ID_INVALID;
        }
        if ( output->dummyStreamId != BAPE_STREAM_ID_INVALID )
        {
            output->dummyMixerId = BAPE_MIXER_ID_CREATE(mixer->dpMixers[BAPE_ChannelPair_eLeftRight][i], 1);   /* 0 is always for outputs, 1 always for dummy */
            regAddr = BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (4*output->dummyStreamId);
            regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, ID);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_FCI_CFGi, ID, BAPE_MIXER_ID_TO_FCI(output->dummyMixerId));
            BDBG_MSG(("Linking dummy output stream %u to FCI 0x%x", output->dummyStreamId, BAPE_MIXER_ID_TO_FCI(output->dummyMixerId)));
            BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
        }
        else
        {
            output->dummyMixerId = BAPE_MIXER_ID_INVALID;
        }
        i++;
    }
    
    /* Link mixer inputs correctly */    
    for ( i = 0; i < numChannelPairs; i++ )
    {
        for ( j = 0; j < BAPE_CHIP_MAX_MIXER_INPUTS; j++ )
        {
            if ( mixer->inputs[j] )
            {
                uint32_t pbId;
                unsigned mixerIndex;

                /* Get playback ID based on input type */
                if ( mixer->inputs[j]->mustBranchAtSrc )
                    pbId = mixer->inputs[j]->playbackId[i];
                else
                    pbId = mixer->pInputLinks[j]->playbackId[i];

                for ( mixerIndex = 0; mixerIndex < BAPE_CHIP_MAX_MIXERS; mixerIndex++ )
                {
                    if ( mixer->dpMixers[i][mixerIndex] != 0xff )
                    {
                        BDBG_MSG(("Binding mixer %u input %u to playback ID %u", mixer->dpMixers[i][mixerIndex], j, pbId));
                        /* Correlate mixer ID to register base */
                        regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG + (((uint32_t)mixer->dpMixers[i][mixerIndex])*(BCHP_AUD_FMM_DP_CTRL0_MIXER1_INPUT10_CONFIG - BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG));
                        /* Add offset for input number */
                        regAddr += 4*(j/2);
                        regVal = BREG_Read32(mixer->deviceHandle->regHandle, regAddr);
                        if ( (j % 2) == 0 )
                        {
                            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER);
                            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT0_PB_NUMBER, pbId);
                        }
                        else
                        {
                            regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER);
                            regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_INPUT10_CONFIG, MIXER_INPUT1_PB_NUMBER, pbId);
                        }
                        BREG_Write32(mixer->deviceHandle->regHandle, regAddr, regVal);
                    }
                }
            }
        }
    }

    /* Done.  Refresh input and output volumes now */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] )
        {
            BDBG_MSG(("Refreshing input scaling for input %u", i));
            BAPE_Mixer_P_ApplyInputVolume(mixer, i);
        }
    }

    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_MSG(("Refreshing output scaling for output %p", output));
        BAPE_Mixer_P_ApplyOutputVolume(mixer, output);
    }
    
    mixer->mclkSource = BAPE_Mixer_P_GetMclkSource(mixer);
    if ( mixer->mclkSource >= BAPE_MclkSource_ePll0 && mixer->mclkSource <= BAPE_MclkSource_ePll2 )
    {
        BAPE_P_AttachMixerToPll(mixer, mixer->mclkSource - BAPE_MclkSource_ePll0);
    }
    else
    {
        BKNI_EnterCriticalSection();
        /* Set outputs to non-PLL MCLK if required */
        for ( output = BLST_S_FIRST(&mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( output->setMclk_isr )
            {
                output->setMclk_isr(output->pHandle, mixer->mclkSource, 2 /* DAC is 256Fs */);
            }
        }
        BKNI_LeaveCriticalSection();
    }

    /* Determine if we need an Fs or not */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        if ( output->fsTiming )
        {
            fsRequired = true;
            break;
        }
    }

    /* Try and grab an Fs */
    if ( fsRequired )
    {
        mixer->fs = BAPE_P_AllocateFs(mixer->deviceHandle);
        if ( mixer->fs == BAPE_FS_INVALID )
        {
            BDBG_ERR(("Unable to allocate fs timing source"));
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_input_resources;
        }
        if ( mixer->mclkSource >= BAPE_MclkSource_ePll0 && mixer->mclkSource <= BAPE_MclkSource_ePll2 )
        {
            /* Do nothing.  The pll set freq will set the fs timing. */
        }
        else
        {
            /* Set the FS to use DAC timing */
            BKNI_EnterCriticalSection();
            BAPE_P_SetFsTiming_isr(mixer->deviceHandle, mixer->fs, mixer->mclkSource, 2 /* DAC is 256Fs */);
            BKNI_LeaveCriticalSection();
        }   
        /* Bind each output to the fs */     
        for ( output = BLST_S_FIRST(&mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( output->fsTiming )
            {
                BDBG_ASSERT(NULL != output->setFs);
                output->setFs(output->pHandle, mixer->fs);
            }
        }
    }

    mixer->ready = true;
    return BERR_SUCCESS;

err_input_resources:
    if ( mixer->fs != BAPE_FS_INVALID )
    {
        BAPE_P_FreeFs(mixer->deviceHandle, mixer->fs);
        mixer->fs = BAPE_FS_INVALID;
    }

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] && mixer->pInputLinks[i]->active )
        {
            mixer->pInputLinks[i]->active = false;
            BDBG_ASSERT(mixer->inputs[i]->activeLinks > 0);
            mixer->inputs[i]->activeLinks--;
            if ( 0 == mixer->inputs[i]->activeLinks )
            {
                if ( mixer->inputs[i]->useBufferPool )
                {
                    BAPE_P_FreeInputBuffers(mixer->deviceHandle, mixer->inputs[i]);
                }
                if ( mixer->inputs[i]->mustBranchAtSrc )
                {
                    BAPE_P_FreeInputResources(mixer->deviceHandle, mixer->inputs[i]);
                }
                else
                {
                    BAPE_P_FreeInputLinkResources(mixer->deviceHandle, mixer->pInputLinks[i]);
                }
            }
        }
    }
    
    BAPE_P_FreeMixers(mixer);
    
err_mixers:
    return errCode;
}

static BERR_Code BAPE_Mixer_P_SetupAdaptiveRateControl(BAPE_MixerHandle mixer, BAPE_MixerInputLink *pLink, BAPE_MixerInput input, bool enabled)
{
    unsigned i;

    if ( input->mustBranchAtSrc )
    {
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            if ( enabled )
            {
                int j;
                if ( input->adaptRateId[i] == 0xff )
                {
                    for ( j = 0; j < BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS; j++ )
                    {
                        if ( false == mixer->deviceHandle->adaptRateAllocated[j] )
                        {
                            mixer->deviceHandle->adaptRateAllocated[j] = true;
                            input->adaptRateId[i] = j;
                            break;
                        }
                    }
                    if ( j >= BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS )
                    {
                        BDBG_ERR(("Insufficient adaptive rate controllers to handle request.  Please disable ppmCorrection on some decoders to reduce the number in use."));
                        return BERR_TRACE(BERR_NOT_SUPPORTED);
                    }
                }
                BDBG_ASSERT(0xff != input->sourceChannelId[i]);
                BAPE_Mixer_P_EnableAdaptiveRateControl(mixer, input, input->adaptRateId[i], input->sourceChannelId[i]);
            }
            else
            {
                if ( input->adaptRateId[i] != 0xff )
                {
                    BDBG_ASSERT(mixer->deviceHandle->adaptRateAllocated[input->adaptRateId[i]]);
                    BAPE_Mixer_P_DisableAdaptiveRateControl(mixer, input->adaptRateId[i]);
                    mixer->deviceHandle->adaptRateAllocated[input->adaptRateId[i]] = false;
                    input->adaptRateId[i] = 0xff;
                }
            }
        }
    }
    else
    {
        /* Input forks at multiple source channels */
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            if ( enabled )
            {
                int j;
                if ( pLink->adaptRateId[i] == 0xff )
                {
                    for ( j = 0; j < BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS; j++ )
                    {
                        if ( false == mixer->deviceHandle->adaptRateAllocated[j] )
                        {
                            BDBG_MSG(("Allocated adaptive rate controller %u", j));
                            mixer->deviceHandle->adaptRateAllocated[j] = true;
                            pLink->adaptRateId[i] = j;
                            break;
                        }
                    }
                    if ( j >= BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS )
                    {
                        BDBG_ERR(("Insufficient adaptive rate controllers to handle request.  Please disable ppmCorrection on some decoders to reduce the number in use."));
                        return BERR_TRACE(BERR_NOT_SUPPORTED);
                    }
                }
                BDBG_ASSERT(0xff != pLink->sourceChannelId[i]);
                BDBG_MSG(("Enabling adaptive rate controller %u", pLink->adaptRateId[i]));
                BAPE_Mixer_P_EnableAdaptiveRateControl(mixer, input, pLink->adaptRateId[i], pLink->sourceChannelId[i]);
            }
            else
            {
                if ( pLink->adaptRateId[i] != 0xff )
                {
                    BDBG_ASSERT(mixer->deviceHandle->adaptRateAllocated[pLink->adaptRateId[i]]);
                    BAPE_Mixer_P_DisableAdaptiveRateControl(mixer, pLink->adaptRateId[i]);
                    mixer->deviceHandle->adaptRateAllocated[pLink->adaptRateId[i]] = false;
                    BDBG_MSG(("Releasing adaptive rate controller %u", pLink->adaptRateId[i]));
                    pLink->adaptRateId[i] = 0xff;
                }
            }
        }
    }
    return BERR_SUCCESS;
}

#if 0
void BAPE_Mixer_P_ReleaseResources(BAPE_MixerHandle mixer)
#else
void BAPE_Mixer_P_ReleaseResources_impl(BAPE_MixerHandle mixer, const char *pFilename, unsigned line)
#endif
{
    unsigned i;

    BDBG_MSG(("Mixer %d (%#x) Release Resources (%s:%d)", mixer->index, mixer, pFilename, line));

    if ( mixer->running )
    {
        BDBG_ERR(("Can't release resources while mixer %p (%d) is running", mixer, mixer->index));
        BDBG_ASSERT(!mixer->running);
        return;
    }

    if ( !mixer->ready )
    {
        /* Nothing to do */
        return;
    }

    if ( mixer->mclkSource >= BAPE_MclkSource_ePll0 && mixer->mclkSource <= BAPE_MclkSource_ePll2 )
    {
        BAPE_P_DetachMixerFromPll(mixer, mixer->mclkSource - BAPE_MclkSource_ePll0);
        mixer->mclkSource = BAPE_MclkSource_eNone;
    }

    if ( mixer->fs != BAPE_FS_INVALID )
    {
        BAPE_P_FreeFs(mixer->deviceHandle, mixer->fs);
        mixer->fs = BAPE_FS_INVALID;
    }

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] )
        {
            BDBG_ASSERT(mixer->pInputLinks[i]->active);
            mixer->pInputLinks[i]->active = false;
            BDBG_ASSERT(mixer->inputs[i]->activeLinks > 0);
            mixer->inputs[i]->activeLinks--;
            if ( 0 == mixer->inputs[i]->activeLinks )
            {
                if ( mixer->inputs[i]->useBufferPool )
                {
                    BAPE_P_FreeInputBuffers(mixer->deviceHandle, mixer->inputs[i]);
                }
                if ( mixer->inputs[i]->mustBranchAtSrc )
                {
                    BAPE_P_FreeInputResources(mixer->deviceHandle, mixer->inputs[i]);
                }
            }
            if ( !mixer->inputs[i]->mustBranchAtSrc )
            {
                BAPE_P_FreeInputLinkResources(mixer->deviceHandle, mixer->pInputLinks[i]);
            }
        }
    }

    BAPE_P_FreeMixers(mixer);
    mixer->ready = false;
}

void BAPE_MixerInput_P_SetSampleRate_isr(BAPE_MixerInput input, BAVC_AudioSamplingRate sampleRate)
{
    BAPE_MixerInputLink *pLink;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    input->sampleRate = sampleRate;

    BDBG_MSG(("Input %p '%s' sample rate change to %s", input, input->pName, BAPE_P_GetSampleRateString_isrsafe(sampleRate)));

    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);
        BDBG_ASSERT(NULL != pLink->mixer);
        if ( pLink->mixer->master == input )
        {
            if ( sampleRate != BAVC_AudioSamplingRate_eUnknown )
            {
                /* Set mixer sample rate. */
                BAPE_Mixer_P_SetSampleRate_isr(pLink->mixer, sampleRate);
            }
            else if ( pLink->mixer->sampleRate == BAVC_AudioSamplingRate_eUnknown )
            {
                /* Make sure there is a valid sample rate */
                BAPE_Mixer_P_SetSampleRate_isr(pLink->mixer, pLink->mixer->settings.defaultSampleRate);
            }
        }
        else
        {
            /* Make sure there is a valid sample rate */
            if ( pLink->mixer->sampleRate == BAVC_AudioSamplingRate_eUnknown )
            {
                BAPE_Mixer_P_SetSampleRate_isr(pLink->mixer, pLink->mixer->settings.defaultSampleRate);
            }
        }

        /* Update SRCs accordingly. */
        BAPE_Mixer_P_SetInputSRC_isr(pLink->mixer, input, sampleRate, pLink->mixer->sampleRate);

        /* Update adaptive rate controllers */
        BAPE_Mixer_P_SetAdaptiveRateFrequency_isr(pLink->mixer, input, sampleRate);
    }
}

void BAPE_MixerInput_P_SetMute(
    BAPE_MixerInput input,
    bool muted
    )
{
    BAPE_MixerInputLink *pLink;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    BDBG_MSG(("Input %p '%s' mute set to %u", input, input->pName, muted));

    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        unsigned i;

        BDBG_OBJECT_ASSERT(pLink, BAPE_MixerInputLink);
        BDBG_ASSERT(NULL != pLink->mixer);

        /* Determine mixer input index */
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( pLink->mixer->inputs[i] == input )
            {
                break;
            }
        }

        /* Should never happen */
        BDBG_ASSERT(i < BAPE_CHIP_MAX_MIXER_INPUTS);

        /* See if mute state is changing */
        if ( pLink->mixer->inputMuted[i] != muted )
        {
            /* Update mute state and re-apply input volume */
            pLink->mixer->inputMuted[i] = muted;
            BAPE_Mixer_P_ApplyInputVolume(pLink->mixer, i);

#if 0
            /* TODO: Need to work out why SPDIF/HDMI mute stops requesting data before this approach will work correctly */

            /* Set output mute state if compressed */
            if ( pLink->mixer->compressed )
            {
                BAPE_MixerOutput output;

                /* Refresh each output's mute state */
                for ( output = BLST_S_FIRST(&pLink->mixer->outputList);
                      output != NULL;
                      output = BLST_S_NEXT(output, node) )
                {
                    BDBG_MSG(("Refreshing output scaling for output %p because of input mute", output));
                    BAPE_Mixer_P_ApplyOutputVolume(pLink->mixer, output);
                }
            }
#endif
        }
    }
}

static void BAPE_Mixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, BAVC_AudioSamplingRate sampleRate)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    /* Only do this if something actually changed */
    if ( mixer->sampleRate != sampleRate )
    {
        unsigned i;
        BAPE_MixerOutput output;
        BERR_Code errCode;

        BDBG_MSG(("Changing mixer %p (%d) sample rate to %s [was %s]", mixer, mixer->index, BAPE_P_GetSampleRateString_isrsafe(sampleRate),
                  BAPE_P_GetSampleRateString_isrsafe(mixer->sampleRate)));
        mixer->sampleRate = sampleRate;
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( mixer->inputs[i] )
            {
                BAPE_Mixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->sampleRate, sampleRate);
            }
        }

        /* For each output... */
        for ( output = BLST_S_FIRST(&mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( output->setTimingParams_isr )
            {
                output->setTimingParams_isr(output->pHandle, sampleRate, mixer->settings.outputTimebase);
            }
        }

        if ( sampleRate != BAVC_AudioSamplingRate_eUnknown )
        {
            /* Update the PLL */
            errCode = BAPE_P_UpdatePll_isr(mixer->deviceHandle, mixer->settings.outputPll);
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
    }
    else
    {
        BDBG_MSG(("NOT Changing mixer %p (%d) sample rate to %s [currently %s]", mixer, mixer->index, BAPE_P_GetSampleRateString_isrsafe(sampleRate),
                  BAPE_P_GetSampleRateString_isrsafe(mixer->sampleRate)));
    }
}

static void BAPE_Mixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_MixerInput input, BAVC_AudioSamplingRate inputRate, BAVC_AudioSamplingRate outputRate)
{
    BAPE_MixerInputLink *pLink=NULL;
    unsigned i;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);
    BDBG_ASSERT(inputRate <= BAVC_AudioSamplingRate_eUnknown);
    BDBG_ASSERT(outputRate <= BAVC_AudioSamplingRate_eUnknown);

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( mixer->inputs[i] == input )
        {
            pLink = mixer->pInputLinks[i];
            break;
        }
    }
    BDBG_ASSERT(NULL != pLink);

    BDBG_MSG(("Setting input %s SRC to %s->%s", input->pName, BAPE_P_GetSampleRateString_isrsafe(inputRate), BAPE_P_GetSampleRateString_isrsafe(outputRate)));

    for ( i = 0; i < input->numChannelPairs; i++ )
    {
        if ( input->mustBranchAtSrc )
        {
            BAPE_P_ProgramSRC_isr(mixer->deviceHandle, input->srcId[i], inputRate, outputRate);
        }
        else
        {
            BAPE_P_ProgramSRC_isr(mixer->deviceHandle, pLink->srcId[i], inputRate, outputRate);
        }
    }
}

unsigned BAPE_MixerInput_P_GetNumOutputs(
    BAPE_MixerInput input
    )
{
    BAPE_MixerInputLink *pLink;
    unsigned num=0;

    BDBG_OBJECT_ASSERT(input, BAPE_MixerInput);

    for ( pLink = BLST_S_FIRST(&input->mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        BAPE_MixerOutput output;

        for ( output = BLST_S_FIRST(&pLink->mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            num++;
        }
    }

    return num;
}

static void BAPE_Mixer_P_EnableAdaptiveRateControl(BAPE_MixerHandle mixer, BAPE_MixerInput input, unsigned adaptRateId, unsigned sourceChannelId)
{
    uint32_t regVal, regAddr;

    BDBG_ASSERT(adaptRateId < BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS);
    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + 4*adaptRateId;

    /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(mixer->deviceHandle->regHandle, regAddr);
    regVal = regVal & ~(BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, TRIWINDOW_WIDTH_SEL)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_CAP_SEL)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE));
    #ifndef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD);
    #endif
    
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_SFIFO_SEL, sourceChannelId);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, TRIWINDOW_WIDTH_SEL, 8);
    switch ( input->sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 0);
        break;
    default:
    case BAVC_AudioSamplingRate_e48k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 1);
        break;
    case BAVC_AudioSamplingRate_e96k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 2);
        break;
    case BAVC_AudioSamplingRate_e192k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 3);
        break;
    case BAVC_AudioSamplingRate_e44_1k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 4);
        break;
    case BAVC_AudioSamplingRate_e88_2k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 5);
        break;
    case BAVC_AudioSamplingRate_e176_4k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 6);
        break;
    }
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_CAP_SEL, 4+mixer->settings.outputTimebase);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE, 1);
    #ifndef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_THRESHOLD, 0xffff);
    #endif
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, regVal);

    #ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0
    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0 + 4*adaptRateId;
    regVal = BREG_Read32_isr(mixer->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0, ADAPTIVE_RATE_THRESHOLD);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_THRESHOLD_0, ADAPTIVE_RATE_THRESHOLD, 0xffff);
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, regVal);
    #endif

    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + 4*adaptRateId;
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, 0);

    BKNI_LeaveCriticalSection();
}

static void BAPE_Mixer_P_DisableAdaptiveRateControl(BAPE_MixerHandle mixer, unsigned adaptRateId)
{
    uint32_t regVal, regAddr;

    BDBG_ASSERT(adaptRateId < BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS);
    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + 4*adaptRateId;

    /* This register must be manipulated in critical section, it is programmed at sample rate changes as well */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(mixer->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPTIVE_RATE_ENABLE);
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, regVal);

    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + 4*adaptRateId;
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, 0);

    BKNI_LeaveCriticalSection();
}

static void BAPE_Mixer_P_SetAdaptiveRateControllerFrequency_isr(BAPE_MixerHandle mixer, unsigned adaptRateId, BAVC_AudioSamplingRate sampleRate)
{
    uint32_t regVal, regAddr;

    BDBG_ASSERT(adaptRateId < BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS);
    regAddr = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG + 4*adaptRateId;

    regVal = BREG_Read32_isr(mixer->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE);
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 0);
        break;
    default:
    case BAVC_AudioSamplingRate_e48k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 1);
        break;
    case BAVC_AudioSamplingRate_e96k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 2);
        break;
    case BAVC_AudioSamplingRate_e192k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 3);
        break;
    case BAVC_AudioSamplingRate_e44_1k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 4);
        break;
    case BAVC_AudioSamplingRate_e88_2k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 5);
        break;
    case BAVC_AudioSamplingRate_e176_4k:
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_ADAPTRATE_0_CFG, ADAPT_SAMPLINGRATE, 6);
        break;
    }
    BREG_Write32_isr(mixer->deviceHandle->regHandle, regAddr, regVal);
}

static void BAPE_Mixer_P_SetAdaptiveRateFrequency_isr(BAPE_MixerHandle mixer, BAPE_MixerInput input, BAVC_AudioSamplingRate sampleRate)
{
    unsigned i;

    if ( input->mustBranchAtSrc )
    {
        for ( i = 0; i < input->numChannelPairs; i++ )
        {
            if ( input->adaptRateId[i] != 0xff )
            {
                BAPE_Mixer_P_SetAdaptiveRateControllerFrequency_isr(mixer, input->adaptRateId[i], sampleRate);
            }
        }
    }
    else
    {
        BAPE_MixerInputLink *pLink;

        for ( pLink = BLST_S_FIRST(&input->mixerList);
              pLink != NULL;
              pLink = BLST_S_NEXT(pLink, node) )
        {
            for ( i = 0; i < input->numChannelPairs; i++ )
            {
                if ( pLink->adaptRateId[i] != 0xff )
                {
                    BAPE_Mixer_P_SetAdaptiveRateControllerFrequency_isr(mixer, pLink->adaptRateId[i], sampleRate);
                }
            }
        }
    }
}

