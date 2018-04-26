/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description: Mixer Interface for "Standard" Mixers
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bape_buffer.h"

BDBG_MODULE(bape_mixer);

#if BAPE_CHIP_MAX_BYPASS_MIXERS > 0
BDBG_FILE_MODULE(bape_fci);
BDBG_FILE_MODULE(bape_mixer_input_coeffs_detail);

/* Local function prototypes */
static void BAPE_BypassMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate);
static void BAPE_BypassMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate);
static BERR_Code BAPE_BypassMixer_P_RemoveAllInputs( BAPE_MixerHandle handle );
static BERR_Code BAPE_BypassMixer_P_RemoveAllOutputs( BAPE_MixerHandle handle );
static bool BAPE_BypassMixer_P_ValidateSettings(BAPE_Handle hApe, const BAPE_MixerSettings *pSettings);

/* Node callbacks */
static BERR_Code BAPE_BypassMixer_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_BypassMixer_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_BypassMixer_P_ConfigPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_BypassMixer_P_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_BypassMixer_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_BypassMixer_P_OutputConnectionAdded(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_BypassMixer_P_OutputConnectionRemoved(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_BypassMixer_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);
static BERR_Code BAPE_BypassMixer_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static void      BAPE_BypassMixer_P_InputMute(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, bool enabled);
static void      BAPE_BypassMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

/* Resource routines */
static BERR_Code BAPE_BypassMixer_P_AllocateResources(BAPE_MixerHandle mixer);
static BERR_Code BAPE_BypassMixer_P_AllocateConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_BypassMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_BypassMixer_P_FreeResources(BAPE_MixerHandle handle);

/* Define the interface struct with all the API callbacks.   It's declared and
 * populated at the bottom of this file.  */
static const BAPE_MixerInterface  bypassMixerInterface;

/*************************************************************************/
BERR_Code BAPE_BypassMixer_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    )
{
    BAPE_MixerHandle handle;
    BAPE_MixerSettings *defaultSettings = NULL;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings ) {
        defaultSettings = BKNI_Malloc(sizeof(BAPE_MixerSettings));
        if ( NULL == defaultSettings ) {
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
        BAPE_Mixer_GetDefaultSettings(defaultSettings);
        pSettings = defaultSettings;
    }

    /* Make sure settings are valid */
    if ( !BAPE_BypassMixer_P_ValidateSettings(deviceHandle, pSettings) ) {
        if (defaultSettings) {
            BKNI_Free(defaultSettings);
        }
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Mixer));
    if ( NULL == handle ) {
        if (defaultSettings) {
            BKNI_Free(defaultSettings);
        }
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Mixer));
    BDBG_OBJECT_SET(handle, BAPE_Mixer);
    handle->mixerType = BAPE_MixerType_eBypass;
    handle->settings = *pSettings;
    handle->interface = &bypassMixerInterface;
    handle->deviceHandle = deviceHandle;
    BLST_S_INSERT_HEAD(&deviceHandle->mixerList, handle, node);
    handle->fs = BAPE_FS_INVALID;
    BAPE_P_InitPathNode(&handle->pathNode, BAPE_PathNodeType_eMixer, handle->settings.type, 1, deviceHandle, handle);
    switch ( handle->explicitFormat )
    {
    case BAPE_MixerFormat_ePcmStereo:
        handle->pathNode.pName = "Bypass 2ch Mixer";
        break;
    case BAPE_MixerFormat_ePcm5_1:
        handle->pathNode.pName = "Bypass 6ch Mixer";
        break;
    case BAPE_MixerFormat_ePcm7_1:
        handle->pathNode.pName = "Bypass 8ch Mixer";
        break;
    default:
        handle->pathNode.pName = "Bypass Mixer";
        break;
    }
    handle->pathNode.connectors[0].useBufferPool = true;

    BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
    format.sampleRate = 0;
    format.source = BAPE_DataSource_eHostBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->pathNode, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eHostBuffer);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm7_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmRf);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x4);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x16);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->pathNode, &caps);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_caps; }

    /* Fill in node connection routines */
    handle->pathNode.allocatePathFromInput = BAPE_BypassMixer_P_AllocatePathFromInput;
    handle->pathNode.freePathFromInput = BAPE_BypassMixer_P_FreePathFromInput;
    handle->pathNode.configPathFromInput = BAPE_BypassMixer_P_ConfigPathFromInput;
    handle->pathNode.startPathFromInput = BAPE_BypassMixer_P_StartPathFromInput;
    handle->pathNode.stopPathFromInput = BAPE_BypassMixer_P_StopPathFromInput;
    handle->pathNode.outputConnectionAdded = BAPE_BypassMixer_P_OutputConnectionAdded;
    handle->pathNode.outputConnectionRemoved = BAPE_BypassMixer_P_OutputConnectionRemoved;
    handle->pathNode.inputSampleRateChange_isr = BAPE_BypassMixer_P_InputSampleRateChange_isr;
    handle->pathNode.inputFormatChange = BAPE_BypassMixer_P_InputFormatChange;
    handle->pathNode.inputMute = BAPE_BypassMixer_P_InputMute;
    handle->pathNode.removeInput = BAPE_BypassMixer_P_RemoveInputCallback;
    handle->stereoMode = BAPE_StereoMode_eLeftRight;

    *pHandle = handle;
    if (defaultSettings) {
        BKNI_Free(defaultSettings);
    }
    return BERR_SUCCESS;

err_caps:
err_format:
    if (defaultSettings) {
        BKNI_Free(defaultSettings);
    }
    BAPE_Mixer_Destroy(handle);

    return errCode;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_Destroy(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Make sure no inputs are running. */
    if ( handle->running )
    {
        BDBG_ERR(("Mixer %p (%d) is running.  Cannot close.", (void *)handle, handle->index));
        BDBG_ASSERT(false == handle->running);
        return;
    }

    /* Remove all inputs */
    BAPE_BypassMixer_P_RemoveAllInputs(handle);

    /* Remove all outputs */
    BAPE_BypassMixer_P_RemoveAllOutputs(handle);

    /* Break any downstream connections */
    BAPE_Connector_P_RemoveAllConnections(&handle->pathNode.connectors[0]);

    /* Unlink from device */
    BLST_S_REMOVE(&handle->deviceHandle->mixerList, handle, BAPE_Mixer, node);
    BDBG_OBJECT_DESTROY(handle, BAPE_Mixer);
    BKNI_Free(handle);
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_Start(BAPE_MixerHandle handle)
{
    BERR_Code errCode;
    unsigned sampleRate;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( handle->running > 0 )
    {
        BDBG_ERR(("Mixer is already running and cannot be explicitly started."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->startedExplicitly )
    {
        BDBG_ERR(("Mixer %p (%d) has already been explicity started.  Can't start again.", (void *)handle, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->settings.format == BAPE_MixerFormat_eAuto || handle->settings.format >= BAPE_MixerFormat_eMax )
    {
        BDBG_ERR(("Mixer cannot be explicitly started with format %d.", handle->settings.format));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Need to Allocate our own resources and push our output FCIs downstream before configuring downstream components */
    errCode = BAPE_BypassMixer_P_AllocateResources(handle);
    if ( errCode )
    {
        goto err_alloc;
    }

    /* Prepare subnodes */
    errCode = BAPE_PathNode_P_AcquirePathResources(&handle->pathNode);
    if ( errCode )
    {
        BDBG_ERR(("Mixer %p (%d) Path resources could not be explicity acquired.", (void *)handle, handle->index));
        BERR_TRACE(errCode);
        goto err_acquire_resources;
    }

    sampleRate = (handle->master) ? (handle->master->format.sampleRate) : handle->settings.defaultSampleRate;
    BKNI_EnterCriticalSection();
    BAPE_Connector_P_SetSampleRate_isr(&handle->pathNode.connectors[0], (sampleRate != 0) ? sampleRate : handle->settings.defaultSampleRate);
    BKNI_LeaveCriticalSection();

    /* Link the path resources */
    errCode = BAPE_PathNode_P_ConfigurePathResources(&handle->pathNode);
    if ( errCode )
    {
        BDBG_ERR(("Mixer %p (%d) Path resources could not be explicity configured.", (void *)handle, handle->index));
        BERR_TRACE(errCode);
        goto err_config_path;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(handle->deviceHandle);
    #endif

    handle->startedExplicitly = true;
    handle->explicitFormat = handle->settings.format;
    errCode = BAPE_PathNode_P_StartPaths(&handle->pathNode);
    if ( errCode )
    {
        BDBG_ERR(("Mixer %p (%d) Path could not be explicity started.", (void *)handle, handle->index));
        BERR_TRACE(errCode);
        goto err_start_path;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&handle->pathNode);
    #endif

    return BERR_SUCCESS;

err_start_path:
    handle->startedExplicitly = false;
    handle->explicitFormat = BAPE_MixerFormat_eMax;
err_config_path:
    BAPE_PathNode_P_ReleasePathResources(&handle->pathNode);
err_acquire_resources:
err_alloc:
    BAPE_BypassMixer_P_FreeResources(handle);
    return errCode;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_Stop(BAPE_MixerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( !handle->startedExplicitly )
    {
        BDBG_ERR(("Mixer %p (%d) was not started with BAPE_Mixer_Start(). Can't stop with BAPE_Mixer_Stop.", (void *)handle, handle->index));
        return;
    }

    if ( handle->running > 0 )
    {
        BDBG_ERR(("Mixer %p (%d) has running inputs.  Please stop all inputs before calling BAPE_Mixer_Stop", (void *)handle, handle->index));
        return;
    }

    BAPE_PathNode_P_StopPaths(&handle->pathNode);

    if ( handle->running == 0 )
    {
        BAPE_PathNode_P_ReleasePathResources(&handle->pathNode);
        BAPE_BypassMixer_P_FreeResources(handle);
    }
    handle->startedExplicitly = false;
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_AddInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerAddInputSettings *pSettings     /* Optional, pass NULL for default settings */
    )
{
    unsigned i, maxInputs;
    bool srcEnabled;
    BERR_Code errCode;
    BAPE_MixerAddInputSettings defaultSettings;
    bool makeMaster;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( NULL == pSettings )
    {
        pSettings = &defaultSettings;
        BAPE_Mixer_GetDefaultAddInputSettings(&defaultSettings);
    }

    makeMaster = pSettings->sampleRateMaster;

    if ( handle->running && handle->startedExplicitly )
    {
        /* allow input to be added on the fly */
        BDBG_MSG(("Adding input %s to explicitly started Standard Mixer %p", input->pName, (void *)handle));
    }
    else if ( handle->running )
    {
        BDBG_ERR(("Cannot change inputs while a mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Make sure the same input isn't added multiple times */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            BDBG_WRN(("Cannot add the same input multiple times to a single mixer."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    /* TBD7211 - customize for FW capabilities */
    srcEnabled = false;
    maxInputs = BAPE_CHIP_MAX_BYPASS_MIXER_INPUTS;

    /* Find empty slot */
    for ( i = 0; i < maxInputs; i++ )
    {
        if ( handle->inputs[i] == NULL )
        {
            handle->inputs[i] = input;
            BDBG_MSG(("Adding input %s to mixer %u as %s", input->pName, handle->index, makeMaster?"master":"slave"));
            if ( makeMaster )
            {
                if ( handle->master )
                {
                    BDBG_MSG(("Replacing current master input with new input"));
                }
                handle->master = input;
            }

            /* init input volume settings */
            BAPE_Mixer_P_GetDefaultInputVolume(&handle->inputVolume[i]);

            /* init input settings */
            handle->inputSettings[i].srcEnabled = srcEnabled;

            errCode = BAPE_PathNode_P_AddInput(&handle->pathNode, input);
            if ( errCode )
            {
                handle->inputs[i] = NULL;
                return BERR_TRACE(errCode);
            }
            if( pSettings->capture )
            {
                handle->inputSettings[i].capture = pSettings->capture;
            }

            /* TODO: validate capture is not hooked to another mixer/input and store the link in the capture handle */
            return BERR_SUCCESS;
        }
    }

    /* Mixer has no available slots. */
    BDBG_ERR(("Mixer can not accept any more inputs.  Maximum inputs per mixer is %u.", maxInputs));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_RemoveInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( handle->running && !handle->startedExplicitly)
    {
        BDBG_ERR(("Cannot change inputs while a \"Auto Mode\" mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find slot containing this input */
    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i < BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        errCode = BAPE_PathNode_P_RemoveInput(&handle->pathNode, input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        handle->inputs[i] = NULL;
        /* TODO: Invalidate linkage between capture and mixer in capture handle */
        handle->inputCaptures[i] = NULL;
        if ( handle->master == input )
        {
            BDBG_MSG(("Removing master input %p", (void *)input));
            handle->master = NULL;
        }
        return BERR_SUCCESS;
    }

    /* Input not found. */
    BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_RemoveAllInputs(
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
            errCode = BAPE_BypassMixer_P_RemoveInput(handle, handle->inputs[i]);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_AddOutput(
    BAPE_MixerHandle handle,
    BAPE_OutputPort output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    if ( output->mixer )
    {
        BDBG_ERR(("Output %p is already connected to mixer %p", (void *)output, (void *)output->mixer));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->running )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        BAPE_OutputPort op;
        unsigned maxOutputs;

        maxOutputs = BAPE_CHIP_MAX_BYPASS_MIXER_OUTPUTS;

        for ( op = BLST_S_FIRST(&handle->outputList);
              op != NULL;
              op = BLST_S_NEXT(op, node) )
        {
            if ( op == output )
            {
                BDBG_ERR(("Output already added to mixer %p", (void*)handle));
                return BERR_TRACE(BERR_UNKNOWN);
            }
        }

        if ( handle->numOutputs >= maxOutputs )
        {
            BDBG_ERR(("Max num outputs (%d) already added for mixer %p", maxOutputs, (void*)handle));
            return BERR_TRACE(BERR_UNKNOWN);
        }

        /* Bind mixer and output.  Remainder is done at start time. */
        output->mixer = handle;
        BLST_S_INSERT_HEAD(&handle->outputList, output, node);
        handle->numOutputs++;
        output->sourceMixerGroup = NULL;
        output->sourceMixerOutputIndex = 0;
        BAPE_BypassMixer_P_FreeResources(handle);

        BDBG_MSG(("Added output %p '%s' to mixer %u [mixer rate %u]", (void *)output, output->pName, handle->index, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle)));
        BKNI_EnterCriticalSection();
        if ( output->setTimingParams_isr && BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle) != 0 )
        {
            output->setTimingParams_isr(output, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle), handle->settings.outputTimebase);
        }
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_RemoveOutput(
    BAPE_MixerHandle handle,
    BAPE_OutputPort output
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    if ( output->mixer != handle )
    {
        BDBG_ERR(("Output %p(%s) is not connected to mixer %p", (void *)output, output->pName, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->running )
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        /* Release mixer-level resources and rebuild on the next start */
        BAPE_BypassMixer_P_FreeResources(handle);
        output->mixer = NULL;
        BLST_S_REMOVE(&handle->outputList, output, BAPE_OutputPortObject, node);
        BDBG_ASSERT(handle->numOutputs > 0);
        handle->numOutputs--;
        BDBG_MSG(("Removed output %p '%s' from mixer %u [mixer rate %u]", (void *)output, output->pName, handle->index, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle)));
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_RemoveAllOutputs(
    BAPE_MixerHandle handle
    )
{
    BAPE_OutputPort output;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    while ( (output=BLST_S_FIRST(&handle->outputList)) )
    {
        errCode = BAPE_BypassMixer_P_RemoveOutput(handle, output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_GetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    BAPE_MixerInputVolume *pVolume      /* [out] */
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pVolume = handle->inputVolume[i];

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index)
{
    BAPE_Connector source = mixer->inputs[index];

    BDBG_OBJECT_ASSERT(source, BAPE_PathConnector);

    BSTD_UNUSED(source);
    BSTD_UNUSED(mixer);
    BSTD_UNUSED(index);

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_SetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerInputVolume *pVolume
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle->inputVolume[i] = *pVolume;
    #if BDBG_DEBUG_BUILD
    {
        unsigned j;

        BDBG_MODULE_MSG(bape_mixer_input_coeffs_detail, ("Input %d -> BapeMixer %p, muted %d, coeffRamp %x, coeff matrix:", i, (void *)handle, pVolume->muted, pVolume->coefficientRamp));
        for ( j = 0; j < BAPE_Channel_eMax; j++ )
        {
            BDBG_MODULE_MSG(bape_mixer_input_coeffs_detail, ("%6x %6x %6x %6x %6x %6x %6x %6x",
                   pVolume->coefficients[j][0],
                   pVolume->coefficients[j][1],
                   pVolume->coefficients[j][2],
                   pVolume->coefficients[j][3],
                   pVolume->coefficients[j][4],
                   pVolume->coefficients[j][5],
                   pVolume->coefficients[j][6],
                   pVolume->coefficients[j][7]
                   ));
        }

    }
    #endif
    /* Apply volume if input is running */
    if ( handle->running )
    {
        errCode = BAPE_BypassMixer_P_ApplyInputVolume(handle, i);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_GetInputSettings(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    BAPE_MixerInputSettings *pSettings      /* [out] */
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pSettings);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pSettings = handle->inputSettings[i];

    return BERR_SUCCESS;
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_SetInputSettings(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerInputSettings *pSettings
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pSettings);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle->inputSettings[i] = *pSettings;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_BypassMixer_P_ApplyStereoMode(BAPE_MixerHandle handle, BAPE_StereoMode stereoMode)
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( handle->stereoMode != stereoMode )
    {
        handle->stereoMode = stereoMode;
        if (handle->running)
        {
            int i;
            for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
            {
                if ( handle->inputs[i] != NULL )
                {
                    int index;
                    index = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, handle->inputs[i]);
                    errCode = BAPE_BypassMixer_P_ApplyInputVolume(handle, index);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                }
            }
        }
    }
    return BERR_SUCCESS;
}

#if 0 /* TBD7211 - handle additional volume for Loudness Equivalence */
static uint32_t BAPE_BypassMixer_P_ApplyAdditionalOutputVolume(BAPE_OutputPort output, int index)
{
    /* CURRENT VOLUME[i] * 2^23 * 10^(OUTPUT ADDITIONAL GAIN /20) / 2^23
       Pre-calculated 2^23 * 10^(OUTPUT ADDITIONAL GAIN /20):
       +4dB - 0xCADDC7
       +3dB - 0xB4CE07
       -7dB - 0x392CED
       -8dB - 0x32F52C
       2^23 - 0x800000
       */

    switch (output->additionalGain)
    {
    case 4:
        return (uint32_t)(((uint64_t)output->volume.volume[index]  * 0xCADDC7) / 0x800000);
    case 3:
        return (uint32_t)(((uint64_t)output->volume.volume[index]  * 0xB4CE07) / 0x800000);
    case -7:
        return (uint32_t)(((uint64_t)output->volume.volume[index]  * 0x392CED) / 0x800000);
    case -8:
        return (uint32_t)(((uint64_t)output->volume.volume[index]  * 0x32F52C) / 0x800000);
    default:
        return output->volume.volume[index];
    }

}
#endif

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_ApplyOutputVolume(BAPE_MixerHandle mixer, BAPE_OutputPort output)
{
    bool handleOutputMuteFirst = true;

    BDBG_ASSERT(mixer == output->mixer);

    /* unmute output after mixer mute settings have been applied */
    if ( output->setMute && !handleOutputMuteFirst )
    {
        /* Call the output's mute handler */
        BDBG_MSG(("Setting output mute to %u", output->volume.muted));
        output->setMute(output, output->volume.muted, false);
    }

    return BERR_SUCCESS;
}

static bool BAPE_BypassMixer_P_ValidateSettings(BAPE_Handle hApe, const BAPE_MixerSettings *pSettings)
{
    switch ( pSettings->defaultSampleRate )
    {
    case 16000:
    case 22050:
    case 24000:
    case 32000:
    case 44100:
    case 48000:
        break;
    default:
        BDBG_ERR(("Mixers only support default sample rates of 32k, 44.1k or 48kHz, and their half samplerates."));
        return false;
    }

    if ( pSettings->outputDelay > hApe->settings.maxIndependentDelay )
    {
        BDBG_ERR(("Output delay value (%u ms) is larger than the maximum (%u ms).  Adjust BAPE_Settings.maxIndependentDelay if a larger value is required.", pSettings->outputDelay, hApe->settings.maxIndependentDelay));
        return false;
    }

    /* Settings are valid */
    return true;
}

static BERR_Code BAPE_BypassMixer_P_SetSettings(
    BAPE_MixerHandle hMixer,
    const BAPE_MixerSettings *pSettings
    )
{
    bool timebaseChanged;
    bool volumeChanged;

    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BDBG_ASSERT(NULL != pSettings);

    if ( !BAPE_BypassMixer_P_ValidateSettings(hMixer->deviceHandle, pSettings) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( hMixer->running )
    {
        /* Determine if the output volume settings have changed */
        volumeChanged = BKNI_Memcmp(&hMixer->settings.outputVolume, &pSettings->outputVolume, sizeof(BAPE_OutputVolume));

        /* If mixer has downstream connections, apply volume changes here */
        if ( volumeChanged )
        {
            BAPE_OutputPort output;
            BAPE_PathConnection *pOutputConnection;
            unsigned mixerIndex = 0;
            unsigned numOutputConnections = 0;

            /* outputs to mixers are 1:1 -- iterate past them to the downstream connections */
            for ( output = BLST_S_FIRST(&hMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                BDBG_ASSERT(NULL != output->sourceMixerGroup);
                mixerIndex++;
            }

            /* count downstream connections */
            for ( pOutputConnection = BLST_SQ_FIRST(&hMixer->pathNode.connectors[0].connectionList);
                  pOutputConnection != NULL;
                  pOutputConnection = BLST_SQ_NEXT(pOutputConnection, downstreamNode) )
            {
                numOutputConnections++;
            }
        }

        /* Store new settings */
        hMixer->settings = *pSettings;
    }
    else
    {
        /* Give up our resources if allocated */
        BAPE_BypassMixer_P_FreeResources(hMixer);

        /* Determine if the timebase has changed */
        timebaseChanged = (hMixer->settings.outputTimebase != pSettings->outputTimebase) ? true : false;
        /* Determine if the output volume settings have changed */
        volumeChanged = BKNI_Memcmp(&hMixer->settings.outputVolume, &pSettings->outputVolume, sizeof(BAPE_OutputVolume));

        /* Store new settings, they will be applied next time the mixer tries to start */
        hMixer->settings = *pSettings;

        /* Refresh output timebase if needed */
        if ( timebaseChanged && BAPE_Mixer_P_GetOutputSampleRate_isrsafe(hMixer) != 0 )
        {
            BAPE_OutputPort output;

            BKNI_EnterCriticalSection();
            for ( output = BLST_S_FIRST(&hMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                if ( output->setTimingParams_isr )
                {
                    output->setTimingParams_isr(output, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(hMixer), pSettings->outputTimebase);
                }
            }
            BKNI_LeaveCriticalSection();
        }
    }

    return BERR_SUCCESS;
}

/*************************************************************************/
/* Allocate mixer resources */
static BERR_Code BAPE_BypassMixer_P_AllocatePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_MixerHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Grab mixer-level resources */
    errCode = BAPE_BypassMixer_P_AllocateResources(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Grab resources for this connection */
    errCode = BAPE_BypassMixer_P_AllocateConnectionResources(handle, pConnection);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
/* Release mixer resources */
static void BAPE_BypassMixer_P_FreePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_MixerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    BDBG_MSG(("Free Path From Input %s %s", pConnection->pSource->pParent->pName, pConnection->pSource->pName));

    /* Release resources for this connection */
    BAPE_BypassMixer_P_FreeConnectionResources(handle, pConnection);
    if (!handle->running && !handle->startedExplicitly)
    {
        /* Free mixer-level resources if needed  */
        BAPE_BypassMixer_P_FreeResources(handle);
    }
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_ConfigPathFromInput(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection
    )
{
    unsigned inputNum;

    BAPE_MixerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Setup linkage from conection into mixer */
    inputNum = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    BDBG_MSG(("pConnection %p, bufferGroup %p", (void*)pConnection, (void*)pConnection->bufferGroupHandle));
    if ( pConnection->bufferGroupHandle )
    {
        /* TBD7211 - anything to do here? - this will only work for 1:1 bypass.
           As soon as we have multiple inputs or outputs, this will not work. */
        handle->bufferGroupHandle = pConnection->bufferGroupHandle;
    }

    /* Done.  Refresh input scaling now */
    BAPE_BypassMixer_P_ApplyInputVolume(handle, inputNum);

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_StartPathFromInput(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection
    )
{
    BAPE_MixerHandle handle;
    BAPE_Connector input;
    BAPE_OutputPort output;
    unsigned inputIndex, numOutputConnections;
    BERR_Code errCode;
    BAPE_PathConnection *pOutputConnection;
    bool volumeControlEnabled;
    const BAPE_FMT_Descriptor *pBfd;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    input = pConnection->pSource;
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    BDBG_MSG(("Input %s -> %s starting", input->pParent->pName, input->pName));

    /* First, setup the sample rate since it may affect output programming. */
    BKNI_EnterCriticalSection();
    BAPE_BypassMixer_P_InputSampleRateChange_isr(pNode, pConnection, pConnection->pSource->format.sampleRate);
    BKNI_LeaveCriticalSection();

    pBfd = BAPE_Mixer_P_GetOutputFormat_isrsafe(handle);
    volumeControlEnabled = BAPE_FMT_P_IsLinearPcm_isrsafe(pBfd);

    /* Start the global mixer components */
    if ( handle->running == 0 )
    {
        /* Enable all the mixer outputs */
        for ( output = BLST_S_FIRST(&handle->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
            BDBG_ASSERT(NULL != output->enable);

            BDBG_MSG(("Enabling output %s (%p)", output->pName, (void *)output));
            errCode = output->enable(output);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }

            /* Apply output volume after enabling outputs (many mute in the output itself) */
            BAPE_BypassMixer_P_ApplyOutputVolume(handle, output);
        }

        /* count output connections */
        numOutputConnections=0;
        for ( pOutputConnection = BLST_SQ_FIRST(&handle->pathNode.connectors[0].connectionList);
              pOutputConnection != NULL;
              pOutputConnection = BLST_SQ_NEXT(pOutputConnection, downstreamNode) )
        {
            numOutputConnections++;
        }
    }

    /* Enable Mixer Inputs Next */
    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Now that mixer is running, Apply output volume changes and mutes */
    if ( handle->running == 0 )
    {
        for ( output = BLST_S_FIRST(&handle->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( volumeControlEnabled )
            {
                /* we are headed to compressed mode.  try to apply the volume ramp before
                   we disable the volume scaling on this mixer */
                BAPE_BypassMixer_P_ApplyOutputVolume(handle, output);
            }

            if ( !volumeControlEnabled )
            {
                /* we are headed to PCM mode, apply volume coeffs after enabling the volume
                   control above.  This is necessary to ensure we receive valid status
                   and wait for ramping to complete. */
                BAPE_BypassMixer_P_ApplyOutputVolume(handle, output);
            }
        }
    }

    BDBG_MSG(("Mixer Input %s [%u] started", input->pName, inputIndex));
    handle->running++;
    handle->inputRunning[inputIndex] = true;

    return BERR_SUCCESS;

err_output:
    return errCode;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_StopPathFromInput(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection
    )
{
    BAPE_MixerHandle handle;
    BAPE_Connector input;
    BAPE_OutputPort output;
    unsigned mixerIndex, inputIndex, numOutputConnections;
    BAPE_PathConnection *pOutputConnection;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    input = pConnection->pSource;
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    /* Determine the input index for this connection */
    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return;
    }

    /* If this input was not started (e.g. error in StartPath) return out */
    if ( !handle->inputRunning[inputIndex] )
    {
        BDBG_MSG(("Input %u was not started.  Nothing to stop.", inputIndex));
        return;
    }
    handle->inputRunning[inputIndex] = false;

    #if BAPE_DSP_SUPPORT
    if ( handle->inputCaptures[inputIndex] )
    {
        BDSP_Stage_RemoveAudioCapture(pConnection->pSource->hStage, handle->inputCaptures[inputIndex]->hCapture);
    }
    #endif

    BDBG_ASSERT(handle->running > 0);
    BDBG_MSG(("Mixer run count was %u now %u", handle->running, handle->running-1));
    handle->running--;
    /* If there are other running inputs, stop outputs here. */
    if ( handle->running == 0 )
    {
        /* We're the last running input.  Stop the mixers and outputs now. */
        BDBG_MSG(("Mixer %p (%d) has no more running inputs.  Stopping all mixer outputs.", (void *)handle, handle->index));

        mixerIndex = 0;
        for ( output = BLST_S_FIRST(&handle->outputList);
            output != NULL;
            output = BLST_S_NEXT(output, node) )
        {
            BDBG_MSG(("Disabling output %s (%p)", output->pName, (void *)output));

            BDBG_ASSERT(NULL != output->disable);
            output->disable(output);
            mixerIndex++;
        }
        numOutputConnections=0;
        for ( pOutputConnection = BLST_SQ_FIRST(&handle->pathNode.connectors[0].connectionList);
              pOutputConnection != NULL;
              pOutputConnection = BLST_SQ_NEXT(pOutputConnection, downstreamNode) )
        {
            numOutputConnections++;
        }
    }
}

static BERR_Code BAPE_BypassMixer_P_OutputConnectionAdded(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;

    BSTD_UNUSED(pConnection);
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( handle->running > 0 )
    {
        BDBG_ERR(("Cannot add connections while a mixer is running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Update number of output connections */
    handle->numOutputConnections++;
    /* Release any saved resources and re-allocate on the next start call */
    BAPE_BypassMixer_P_FreeResources(handle);

    return BERR_SUCCESS;
}

static void BAPE_BypassMixer_P_OutputConnectionRemoved(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;

    BSTD_UNUSED(pConnection);
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Make sure we're not running.  Should never happen. */
    BDBG_ASSERT(handle->running == 0);

    /* Release any saved resources and re-allocate on the next start call.  This will also remove any output connection resources. */
    BAPE_BypassMixer_P_FreeResources(handle);

    /* Decrement number of output connections */
    BDBG_ASSERT(handle->numOutputConnections > 0);
    handle->numOutputConnections--;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_InputSampleRateChange_isr(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection,
    unsigned newSampleRate
    )
{
    BAPE_MixerHandle handle;
    unsigned sampleRate, i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BKNI_ASSERT_ISR_CONTEXT();

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    sampleRate = newSampleRate;

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    if ( handle->inputCaptures[i] && NULL != handle->inputCaptures[i]->interrupts.sampleRate.pCallback_isr )
    {
        handle->inputCaptures[i]->interrupts.sampleRate.pCallback_isr(handle->inputCaptures[i]->interrupts.sampleRate.pParam1,
                                                                    handle->inputCaptures[i]->interrupts.sampleRate.param2,
                                                                    newSampleRate);
    }

    /* If we are setting the same rate, ignore it */
    if ( pConnection->format.sampleRate == sampleRate && sampleRate != 0 )
    {
        BDBG_MSG(("Input %s %s sample rate unchanged (%u)", pConnection->pSource->pParent->pName, pConnection->pSource->pName, pConnection->format.sampleRate));
        return;
    }

    BDBG_MSG(("Input %s %s sample rate changed from %u to %u (master=%d)", pConnection->pSource->pParent->pName, pConnection->pSource->pName, pConnection->format.sampleRate, sampleRate, pConnection->pSource == handle->master));

    /* Mark new sample rate in connection */
    pConnection->format.sampleRate = sampleRate;

    /* Handle fixed rate output */
    if ( handle->settings.mixerSampleRate )
    {
        /* Set mixer sample rate. */
        BAPE_BypassMixer_P_SetSampleRate_isr(handle, handle->settings.mixerSampleRate);
    }
    /* Handle the sample rate master */
    else if ( handle->master == pConnection->pSource )
    {
        if ( sampleRate != 0 )
        {
            unsigned mixerRate=sampleRate;
            if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&pConnection->pSource->format) )
            {
                switch ( sampleRate )
                {
                case 8000:
                case 16000:
                    BDBG_MSG(("Using output rate of 32000 for mixer"));
                    mixerRate = 32000;
                    break;
                case 11025:
                case 22050:
                    BDBG_MSG(("Using output rate of 44100 for mixer"));
                    mixerRate = 44100;
                    break;
                case 12000:
                case 24000:
                    BDBG_MSG(("Using output rate of 48000 for mixer"));
                    mixerRate = 48000;
                    break;
                case 32000:
                case 44100:
                case 48000:
                case 64000:
                case 88200:
                case 96000:
                case 176400:
                case 192000:
                    BDBG_MSG(("Mixer input rate is supported.  Using it directly."));
                    mixerRate = sampleRate;
                    break;
                default:
                    BDBG_WRN(("Nonstandard PCM rate (%u) from input %s %s, defaulting to %u output rate", sampleRate, pConnection->pSource->pParent->pName, pConnection->pSource->pName, handle->settings.defaultSampleRate));
                    mixerRate = handle->settings.defaultSampleRate;
                    break;
                }
            }
            /* Set mixer sample rate. */
            BAPE_BypassMixer_P_SetSampleRate_isr(handle, mixerRate);
        }
        else if ( BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle) == 0 )
        {
            /* Make sure there is a valid sample rate */
            BAPE_BypassMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }
    else
    {
        /* Make sure there is a valid sample rate */
        if ( BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle) == 0 )
        {
            BAPE_BypassMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }

    /* Update SRCs accordingly. */
    BAPE_BypassMixer_P_SetInputSRC_isr(handle, pConnection->pSource, sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(handle));
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_InputFormatChange(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection,
    const BAPE_FMT_Descriptor *pNewFormat
    )
{
    BAPE_MixerHandle handle;
    BAPE_DataType dataType;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Handle a format change by releasing all connection resources.
       They will get re-allocated on the next start sequence for the correct format.
    */
    BDBG_MSG(("Input Format Changed for input %s %s [%s->%s]", pConnection->pSource->pParent->pName, pConnection->pSource->pName,
              BAPE_FMT_P_GetTypeName_isrsafe(&pConnection->format), BAPE_FMT_P_GetTypeName_isrsafe(pNewFormat)));

    /* Determine if we need to release resources or not */
    if ( pConnection->format.type != pNewFormat->type || pConnection->format.ppmCorrection != pNewFormat->ppmCorrection )
    {
        BDBG_MSG(("Resource requirements changed.  Freeing connection resources."));
        BAPE_BypassMixer_P_FreeConnectionResources(handle, pConnection);

        /* Some changes are not supported while running.  Check these now. */
        if ( handle->running > 0 )
        {
            if ( BAPE_FMT_P_IsLinearPcm_isrsafe(pNewFormat) != BAPE_FMT_P_IsLinearPcm_isrsafe(BAPE_Mixer_P_GetOutputFormat_isrsafe(handle)) )
            {
                BDBG_ERR(("Can not change from compressed to PCM or vice-versa while other inputs are running."));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            /* TODO: Technically, this is possible if we permit downmixing in the mixer.  Currently not required for STB use cases. */
            if ( BAPE_FMT_P_GetNumChannelPairs_isrsafe(pNewFormat) > BAPE_FMT_P_GetNumChannelPairs_isrsafe(BAPE_Mixer_P_GetOutputFormat_isrsafe(handle)) )
            {
                BDBG_ERR(("Input is wider than the current mixer data format.  Cannot change this while other inputs are running."));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
    }

    /* Re-evaluate mixer output format */
    errCode = BAPE_Mixer_P_DetermineOutputDataType(handle, &dataType);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if ( dataType != BAPE_Mixer_P_GetOutputDataType_isrsafe(handle) )
    {
        BAPE_FMT_Descriptor format;

        /* If the data format has changed while stopped, re-evaluate the output data format. */
        BAPE_BypassMixer_P_FreeResources(handle);

        BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
        format.type = dataType;
        /* Update output format and propagate it */
        errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* If this is the master input, propagate data type to output data type if required. */
    if ( pConnection->pSource == handle->master )
    {
        switch ( pNewFormat->type )
        {
        case BAPE_DataType_eIec61937:
        case BAPE_DataType_eIec61937x4:
        case BAPE_DataType_eIec61937x16:
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&handle->pathNode.connectors[0].format, BAPE_FMT_P_GetAudioCompressionStd_isrsafe(pNewFormat));
            break;
        case BAPE_DataType_ePcmRf:
            BAPE_FMT_P_SetRfAudioEncoding(&handle->pathNode.connectors[0].format, BAPE_FMT_P_GetRfAudioEncoding_isrsafe(pNewFormat));
            break;
        default:
            break;
        }
    }
    BKNI_EnterCriticalSection();
    BAPE_BypassMixer_P_InputSampleRateChange_isr(pNode, pConnection, pNewFormat->sampleRate);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}


/*************************************************************************/
static void BAPE_BypassMixer_P_InputMute(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection,
    bool enabled
    )
{
    BAPE_MixerHandle handle;
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Determine mixer input index */
    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    /* Should never happen */
    if ( i >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(i < BAPE_CHIP_MAX_MIXER_INPUTS);
        return;
    }

    /* See if mute state is changing */
    if ( handle->inputMuted[i] != enabled )
    {
        /* Update mute state and re-apply input volume */
        handle->inputMuted[i] = enabled;
        BAPE_BypassMixer_P_ApplyInputVolume(handle, i);
    }
}

/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_AllocateResources(BAPE_MixerHandle mixer)
{
    BAPE_OutputPort output;
    unsigned numOutputs=0, numOutputConnections=0;
    BERR_Code errCode;
    BAPE_PathConnection *pConnection;
    BAPE_DataType outputDataType;
    BAPE_FMT_Descriptor newFormat;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);

    if ( mixer->resourcesAllocated )
    {
        BDBG_MSG(("Mixer %p (%d) already has resources allocated", (void *)mixer, mixer->index));
        return BERR_SUCCESS;
    }

    output = BLST_S_FIRST(&mixer->outputList);
    if ( NULL == output && 0 == mixer->numOutputConnections )
    {
        BDBG_ERR(("No outputs or nodes connected to mixer %p (%d), cannot allocate resources.", (void *)mixer, mixer->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Determine output format based on widest input format */
    errCode = BAPE_Mixer_P_DetermineOutputDataType(mixer, &outputDataType);
    if ( errCode )
    {
        BDBG_ERR(("Unable to determine mixer ouptut data format."));
        return BERR_TRACE(errCode);
    }

    BAPE_Connector_P_GetFormat(&mixer->pathNode.connectors[0], &newFormat);
    newFormat.type = outputDataType;

    /* Make sure all outputs are capable of receiving this data format */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
        if ( !BAPE_FMT_P_FormatSupported_isrsafe(&output->capabilities, &newFormat) )
        {
            BDBG_ERR(("Output %s can not accept a data type of %s or source type %s", output->pName, BAPE_FMT_P_GetTypeName_isrsafe(&newFormat), BAPE_FMT_P_GetSourceName_isrsafe(&newFormat)));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        numOutputs++;
    }

    for ( pConnection = BLST_SQ_FIRST(&mixer->pathNode.connectors[0].connectionList);
          pConnection != NULL;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        numOutputConnections++;
    }

    /* Update output format and propagate it */
    errCode = BAPE_Connector_P_SetFormat(&mixer->pathNode.connectors[0], &newFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Mixer %p (%d) [%s]: Allocating mixers for %u outputs, %u outputConnections, using data format %s.",
              (void *)mixer, mixer->index, mixer->pathNode.pName, numOutputs, numOutputConnections, BAPE_FMT_P_GetTypeName_isrsafe(&newFormat)));

    /* Refresh output scaling */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_MSG(("Refreshing output scaling for output %p", (void *)output));
        BAPE_BypassMixer_P_ApplyOutputVolume(mixer, output);
    }

    mixer->resourcesAllocated = true;
    return BERR_SUCCESS;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);

    /* Only do this if something actually changed */
    BDBG_MSG(("Set Mixer Output Rate to %u (was %u)", sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(mixer)));
    if ( BAPE_Mixer_P_GetOutputSampleRate_isrsafe(mixer) != sampleRate )
    {
        BAPE_OutputPort output;

        if ( sampleRate == 0 )

        BDBG_MSG(("Changing mixer %p (%d) sample rate to %u [was %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(mixer)));

        (void)BAPE_Connector_P_SetSampleRate_isr(&mixer->pathNode.connectors[0], sampleRate);

        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( mixer->inputs[i] )
            {
                BAPE_BypassMixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->format.sampleRate, sampleRate);
            }
        }

        /* For each output... */
        for ( output = BLST_S_FIRST(&mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( output->setTimingParams_isr )
            {
                output->setTimingParams_isr(output, sampleRate, mixer->settings.outputTimebase);
            }
        }

        #if BAPE_CHIP_MAX_PLLS > 0
        if ( BAPE_MCLKSOURCE_IS_PLL(mixer->mclkSource) && sampleRate != 0 )
        {
            BERR_Code errCode;
            /* Update the PLL */
            errCode = BAPE_P_UpdatePll_isr(mixer->deviceHandle, mixer->settings.outputPll);
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
        #endif

        #if BAPE_CHIP_MAX_NCOS > 0
        if ( BAPE_MCLKSOURCE_IS_NCO(mixer->mclkSource) && sampleRate != 0 )
        {
            BERR_Code errCode;
            /* Update the NCO */
            errCode = BAPE_P_UpdateNco_isr(mixer->deviceHandle, mixer->settings.outputNco);
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
        #endif

    }
    else
    {
        BDBG_MSG(("NOT Changing mixer %p (%d) sample rate to %u [currently %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isrsafe(mixer)));
    }
}

/*************************************************************************/
static void BAPE_BypassMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate)
{
    BAPE_PathConnection *pLink=NULL;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    /* Find connection node for this object */
    for ( pLink = BLST_SQ_FIRST(&input->connectionList);
          NULL != pLink;
          pLink = BLST_SQ_NEXT(pLink, downstreamNode) )
    {
        if ( pLink->pSink == &mixer->pathNode )
            break;
    }
    BDBG_ASSERT(NULL != pLink);

    BSTD_UNUSED(inputRate);
    BSTD_UNUSED(outputRate);
}


/*************************************************************************/
static BERR_Code BAPE_BypassMixer_P_AllocateConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    unsigned i;
    BERR_Code errCode;
    bool srcRequired=false, buffersOnly=false;
    BAPE_BufferInterfaceType bufferInterfaceType;

    BAPE_Connector input;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(&handle->pathNode == pConnection->pSink);

    input = pConnection->pSource;

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    BDBG_ASSERT(i != BAPE_MIXER_INPUT_INDEX_INVALID);

    buffersOnly = true;
    bufferInterfaceType = BAPE_BufferInterfaceType_eDram;
    #if 0 /* TBD7211 - customize for FW capabilities */
    {
        switch ( pConnection->pSource->format.source )
        {
        case BAPE_DataSource_eDfifo:
        case BAPE_DataSource_eDspBuffer:
        case BAPE_DataSource_eHostBuffer:
            /* Omit for HBR sources, only one sample rate is valid for HBR */
            if ( pConnection->pSource->format.type != BAPE_DataType_eIec61937x16 )
            {
                srcRequired = handle->inputSettings[i].srcEnabled;
            }
            break;
        case BAPE_DataSource_eFci:
            if ( pConnection->pSource->pParent->type == BAPE_PathNodeType_eInputCapture )
            {
                srcRequired = handle->inputSettings[i].srcEnabled;
            }
            else
            {
                /* TODO: Many cases could require an SRC - omit now to save HW resources for EQ */
                return BERR_SUCCESS;
            }
            break;
        default:
            BDBG_ERR(("Unrecognized source type %u", pConnection->pSource->format.source));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    #endif

    BDBG_MSG(("Mixer %p Allocating resources for input %s %s", (void *)handle, input->pParent->pName, input->pName));

    /* Check if we've already allocated resources */
    if ( pConnection->resourcesAllocated )
    {
        if ( (pConnection->format.type == input->format.type) &&
             (pConnection->format.ppmCorrection == input->format.ppmCorrection) )
        {
            BDBG_MSG(("Connection %s -> %s %s already has the required resources", input->pParent->pName, pConnection->pSink->pName, input->pName));
            buffersOnly = true;
        }
        else
        {
            /* Connection format has changed */
            BDBG_MSG(("Connection %s -> %s %s format changed.  Releasing resources.", input->pParent->pName, pConnection->pSink->pName, input->pName));
            BAPE_BypassMixer_P_FreeConnectionResources(handle, pConnection);
        }
    }

    if ( input->useBufferPool )
    {
        /* This is safe to call multiple times, it only allocates if need be */
        errCode = BAPE_P_AllocateInputBuffers(handle->deviceHandle, input, bufferInterfaceType);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    if ( buffersOnly )
    {
        /* We don't need to re-add the remaining resources */
        return BERR_SUCCESS;
    }

    /* TBD7211 - allocate SRC (FW) resources*/
    BSTD_UNUSED(srcRequired);

    pConnection->format = input->format;
    pConnection->resourcesAllocated = true;

    return BERR_SUCCESS;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    BDBG_MSG(("Freeing connection resources for input %s %s", pConnection->pSource->pParent->pName, pConnection->pSource->pName));

    if ( pConnection->pSource->useBufferPool )
    {
        BAPE_P_FreeInputBuffers(handle->deviceHandle, pConnection->pSource);
    }
    pConnection->resourcesAllocated = false;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_FreeResources(BAPE_MixerHandle handle)
{
    BAPE_PathConnection *pConnection;

    BDBG_MSG(("Mixer %d (%p) Free Resources", handle->index, (void *)handle));

    if ( handle->running )
    {
        BDBG_ERR(("Can't release resources while mixer %p (%d) is running", (void *)handle, handle->index));
        BDBG_ASSERT(!handle->running);
        return;
    }

    if ( !handle->resourcesAllocated )
    {
        /* Nothing to do */
        return;
    }

#if BAPE_CHIP_MAX_PLLS > 0
    if ( BAPE_MCLKSOURCE_IS_PLL(handle->mclkSource))
    {
        BAPE_P_DetachMixerFromPll_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_ePll0);
    }
#endif

#if BAPE_CHIP_MAX_NCOS > 0
    if ( BAPE_MCLKSOURCE_IS_NCO(handle->mclkSource))
    {
        BAPE_P_DetachMixerFromNco_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_eNco0);
    }
#endif
        handle->mclkSource = BAPE_MclkSource_eNone;

    if ( handle->fs != BAPE_FS_INVALID )
    {
        BAPE_P_FreeFs(handle->deviceHandle, handle->fs);
        handle->fs = BAPE_FS_INVALID;
    }

    /* Invalidate Downstream connections as well */
    for ( pConnection = BLST_SQ_FIRST(&handle->pathNode.connectors[0].connectionList);
          pConnection != NULL;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        BAPE_FciIdGroup_Init(&pConnection->inputFciGroup);
    }

    handle->resourcesAllocated = false;
}

/*************************************************************************/
static void BAPE_BypassMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_BypassMixer_P_RemoveInput(pNode->pHandle, pConnector);
}

/*************************************************************************/
/* Declare the interface struct containing the API callbacks */

#define NOT_SUPPORTED(x) NULL
/* If any of the APIs are not supported by a particular type of mixer,
 * just add a placeholder like this:
 *   NOT_SUPPORTED(BAPE_BypassMixer_P_RemoveAllInputs)
 */

static const BAPE_MixerInterface  bypassMixerInterface  = {
    BAPE_BypassMixer_P_Destroy,            /*       (*destroy)           */
    BAPE_BypassMixer_P_Start,              /*       (*start)             */
    BAPE_BypassMixer_P_Stop,               /*       (*stop)              */
    BAPE_BypassMixer_P_AddInput,           /*       (*addInput)          */
    BAPE_BypassMixer_P_RemoveInput,        /*       (*removeInput)       */
    BAPE_BypassMixer_P_RemoveAllInputs,    /*       (*removeAllInputs)   */
    BAPE_BypassMixer_P_AddOutput,          /*       (*addOutput)         */
    BAPE_BypassMixer_P_RemoveOutput,       /*       (*removeOutput)      */
    BAPE_BypassMixer_P_RemoveAllOutputs,   /*       (*removeAllOutputs)  */
    BAPE_BypassMixer_P_GetInputVolume,     /*       (*getInputVolume)    */
    BAPE_BypassMixer_P_SetInputVolume,     /*       (*setInputVolume)    */
    BAPE_BypassMixer_P_GetInputSettings,   /*       (*getInputSettings)  */
    BAPE_BypassMixer_P_SetInputSettings,   /*       (*setInputSettings)  */
    BAPE_BypassMixer_P_ApplyOutputVolume,  /*       (*applyOutputVolume) */
    BAPE_BypassMixer_P_SetSettings,        /*       (*setSettings)       */
    BAPE_BypassMixer_P_ApplyStereoMode,    /*       (*applyStereoMode)   */
    NOT_SUPPORTED(BAPE_BypassMixer_P_GetInputStatus),
    NOT_SUPPORTED(BAPE_BypassMixer_P_GetStatus)
};

#endif /* BAPE_CHIP_MAX_BYPASS_MIXERS */
