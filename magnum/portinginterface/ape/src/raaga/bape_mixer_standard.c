/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

BDBG_MODULE(bape_mixer);
BDBG_FILE_MODULE(bape_mixer_get_mclk_source);
BDBG_FILE_MODULE(bape_fci);
BDBG_FILE_MODULE(bape_mixer_input_coeffs_detail);

#define BAPE_MIXER_INPUT_INDEX_INVALID ((unsigned)-1)

/* Local function prototypes */
static void BAPE_StandardMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate);
static void BAPE_StandardMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate);
static BERR_Code BAPE_StandardMixer_P_RemoveAllInputs( BAPE_MixerHandle handle );
static BERR_Code BAPE_StandardMixer_P_RemoveAllOutputs( BAPE_MixerHandle handle );
static bool BAPE_StandardMixer_P_ValidateSettings(BAPE_Handle hApe, const BAPE_MixerSettings *pSettings);
static uint32_t BAPE_StandardMixer_P_ApplyAdditionalOutputVolume(BAPE_OutputPort output, int index);

/* Node callbacks */
static BERR_Code BAPE_StandardMixer_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_StandardMixer_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_StandardMixer_P_ConfigPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_StandardMixer_P_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_StandardMixer_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_StandardMixer_P_OutputConnectionAdded(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_StandardMixer_P_OutputConnectionRemoved(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_StandardMixer_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);
static BERR_Code BAPE_StandardMixer_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static void      BAPE_StandardMixer_P_InputMute(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, bool enabled);
static void      BAPE_StandardMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

/* Resource routines */
static BERR_Code BAPE_StandardMixer_P_AllocateResources(BAPE_MixerHandle mixer);
static BERR_Code BAPE_StandardMixer_P_AllocateConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_StandardMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_StandardMixer_P_FreeResources(BAPE_MixerHandle handle);
static BERR_Code BAPE_StandardMixer_P_AllocateMixerGroups(BAPE_MixerHandle handle);

/* Define the interface struct with all the API callbacks.   It's declared and
 * populated at the bottom of this file.  */
static const BAPE_MixerInterface  standardMixerInterface;


/*************************************************************************/
BERR_Code BAPE_StandardMixer_P_Create(
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
    if ( !BAPE_StandardMixer_P_ValidateSettings(deviceHandle, pSettings) ) {
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
    handle->explicitFormat = pSettings->format != BAPE_MixerFormat_eAuto ? pSettings->format : BAPE_MixerFormat_eMax;
    handle->settings = *pSettings;
    handle->interface = &standardMixerInterface;
    handle->deviceHandle = deviceHandle;
    BLST_S_INSERT_HEAD(&deviceHandle->mixerList, handle, node);
    handle->fs = BAPE_FS_INVALID;
    BAPE_P_InitPathNode(&handle->pathNode, BAPE_PathNodeType_eMixer, handle->settings.type, 1, deviceHandle, handle);
    handle->pathNode.subtype = BAPE_MixerType_eStandard;
        switch ( handle->explicitFormat )
        {
        case BAPE_MixerFormat_ePcmStereo:
            handle->pathNode.pName = "Explicit 2ch Mixer";
            break;
        case BAPE_MixerFormat_ePcm5_1:
            handle->pathNode.pName = "Explicit 6ch Mixer";
            break;
        case BAPE_MixerFormat_ePcm7_1:
            handle->pathNode.pName = "Explicit 8ch Mixer";
            break;
        default:
            handle->pathNode.pName = "Standard Mixer";
            break;
        }
    handle->pathNode.connectors[0].useBufferPool = true;

    BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
    format.sampleRate = 0;
    format.source = BAPE_DataSource_eFci;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->pathNode, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eHostBuffer);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDfifo);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eFci);
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
    handle->pathNode.allocatePathFromInput = BAPE_StandardMixer_P_AllocatePathFromInput;
    handle->pathNode.freePathFromInput = BAPE_StandardMixer_P_FreePathFromInput;
    handle->pathNode.configPathFromInput = BAPE_StandardMixer_P_ConfigPathFromInput;
    handle->pathNode.startPathFromInput = BAPE_StandardMixer_P_StartPathFromInput;
    handle->pathNode.stopPathFromInput = BAPE_StandardMixer_P_StopPathFromInput;
    handle->pathNode.outputConnectionAdded = BAPE_StandardMixer_P_OutputConnectionAdded;
    handle->pathNode.outputConnectionRemoved = BAPE_StandardMixer_P_OutputConnectionRemoved;
    handle->pathNode.inputSampleRateChange_isr = BAPE_StandardMixer_P_InputSampleRateChange_isr;
    handle->pathNode.inputFormatChange = BAPE_StandardMixer_P_InputFormatChange;
    handle->pathNode.inputMute = BAPE_StandardMixer_P_InputMute;
    handle->pathNode.removeInput = BAPE_StandardMixer_P_RemoveInputCallback;
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
static void BAPE_StandardMixer_P_Destroy(
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
    BAPE_StandardMixer_P_RemoveAllInputs(handle);

    /* Remove all outputs */
    BAPE_StandardMixer_P_RemoveAllOutputs(handle);

    /* Break any downstream connections */
    BAPE_Connector_P_RemoveAllConnections(&handle->pathNode.connectors[0]);

    /* Unlink from device */
    BLST_S_REMOVE(&handle->deviceHandle->mixerList, handle, BAPE_Mixer, node);
    BDBG_OBJECT_DESTROY(handle, BAPE_Mixer);
    BKNI_Free(handle);
}

/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_Start(BAPE_MixerHandle handle)
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
    errCode = BAPE_StandardMixer_P_AllocateResources(handle);
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
    BAPE_StandardMixer_P_FreeResources(handle);
    return errCode;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_Stop(BAPE_MixerHandle handle)
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
        BAPE_StandardMixer_P_FreeResources(handle);
    }
    handle->startedExplicitly = false;
}

/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_AddInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerAddInputSettings *pSettings     /* Optional, pass NULL for default settings */
    )
{
    unsigned i;
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
            handle->inputSettings[i].srcEnabled = true;

            errCode = BAPE_PathNode_P_AddInput(&handle->pathNode, input);
            if ( errCode )
            {
                handle->inputs[i] = NULL;
                return BERR_TRACE(errCode);
            }
            if( pSettings->capture )
            {
                handle->inputCaptures[i] = pSettings->capture;
            }

            /* TODO: validate capture is not hooked to another mixer/input and store the link in the capture handle */
            return BERR_SUCCESS;
        }
    }

    /* Mixer has no available slots. */
    BDBG_ERR(("Mixer can not accept any more inputs.  Maximum inputs per mixer is %u.", BAPE_CHIP_MAX_MIXER_INPUTS));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_RemoveInput(
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
static BERR_Code BAPE_StandardMixer_P_RemoveAllInputs(
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
            errCode = BAPE_StandardMixer_P_RemoveInput(handle, handle->inputs[i]);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_AddOutput(
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
        /* Bind mixer and output.  Remainder is done at start time. */
        output->mixer = handle;
        BLST_S_INSERT_HEAD(&handle->outputList, output, node);
        handle->numOutputs++;
        output->sourceMixerGroup = NULL;
        output->sourceMixerOutputIndex = 0;
        BAPE_StandardMixer_P_FreeResources(handle);

        BDBG_MSG(("Added output %p '%s' to mixer %u [mixer rate %u]", (void *)output, output->pName, handle->index, BAPE_Mixer_P_GetOutputSampleRate(handle)));
        BKNI_EnterCriticalSection();
        if ( output->setTimingParams_isr && BAPE_Mixer_P_GetOutputSampleRate_isr(handle) != 0 )
        {
            output->setTimingParams_isr(output, BAPE_Mixer_P_GetOutputSampleRate_isr(handle), handle->settings.outputTimebase);
        }
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_RemoveOutput(
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
        BAPE_StandardMixer_P_FreeResources(handle);
        output->mixer = NULL;
        BLST_S_REMOVE(&handle->outputList, output, BAPE_OutputPortObject, node);
        BDBG_ASSERT(handle->numOutputs > 0);
        handle->numOutputs--;
        BDBG_MSG(("Removed output %p '%s' from mixer %u [mixer rate %u]", (void *)output, output->pName, handle->index, BAPE_Mixer_P_GetOutputSampleRate(handle)));
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_RemoveAllOutputs(
    BAPE_MixerHandle handle
    )
{
    BAPE_OutputPort output;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    while ( (output=BLST_S_FIRST(&handle->outputList)) )
    {
        errCode = BAPE_StandardMixer_P_RemoveOutput(handle, output);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_GetInputVolume(
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
static BERR_Code BAPE_StandardMixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index)
{
    BAPE_MixerGroupInputSettings mixerInputSettings;
    BAPE_Connector source = mixer->inputs[index];
    unsigned mixerNum;
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(source, BAPE_PathConnector);

    for ( mixerNum = 0; mixerNum < BAPE_CHIP_MAX_MIXERS; mixerNum++ )
    {
        BAPE_MixerGroupHandle mixerGroup = mixer->mixerGroups[mixerNum];
        if ( NULL != mixerGroup )
        {
            BAPE_MixerGroup_P_GetInputSettings(mixerGroup, index, &mixerInputSettings);
            for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
            {
                if ( !BAPE_FMT_P_IsLinearPcm_isrsafe(&source->format) && !mixer->inputMuted[index] )
                {
                    /* Non-PCM inputs must be either full-scale or 0 coefficients. */
                    int32_t coef = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&source->format) > i ? BAPE_VOLUME_NORMAL : 0;
                    mixerInputSettings.coefficients[i][0][0] = coef;
                    mixerInputSettings.coefficients[i][1][0] = 0;
                    mixerInputSettings.coefficients[i][0][1] = 0;
                    mixerInputSettings.coefficients[i][1][1] = coef;
                }
                else if ( BAPE_FMT_P_GetNumChannelPairs_isrsafe(&source->format) > i &&
                          !mixer->inputMuted[index] &&
                          !mixer->inputVolume[index].muted )
                {
                    /* PCM inputs can have variable volume */
                    if ( mixer->stereoMode == BAPE_StereoMode_eRightLeft ) /* Right/Left */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[2 * i][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[2 * i][2 * i];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[(2*i)+1][2 * i];
                    }
                    else if ( mixer->stereoMode == BAPE_StereoMode_eLeftLeft ) /* Left/Left */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[2 * i][2 * i];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[(2*i)+1][2*i];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[2 * i][2 * i];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[(2*i)+1][2*i];
                    }
                    else if ( mixer->stereoMode == BAPE_StereoMode_eRightRight ) /* Right/Right */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[2*i][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[2*i][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                    }
                    else /* Left/Right */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[2 * i][2 * i];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[(2*i)+1][2*i];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[2*i][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];

                    }
                }
                else if ( mixer->explicitFormat != BAPE_MixerFormat_eMax && BAPE_Mixer_P_MixerFormatToNumChannels(mixer->explicitFormat) > i &&
                          !mixer->inputMuted[index] &&
                          !mixer->inputVolume[index].muted )
                {
                    /* Allow 2 -> Multichannel Upmixing */
                    if ( mixer->stereoMode == BAPE_StereoMode_eRightLeft ) /* Right/Left */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[0][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[1][(2*i)+1];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[0][2*i];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[1][2*i];
                    }
                    else if ( mixer->stereoMode == BAPE_StereoMode_eLeftLeft ) /* Left/Left */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[0][2*i];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[1][2*i];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[0][2*i];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[1][2*i];
                    }
                    else if ( mixer->stereoMode == BAPE_StereoMode_eRightRight ) /* Right/Right */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[0][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[1][(2*i)+1];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[0][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[1][(2*i)+1];
                    }
                    else /* Left/Right */
                    {
                        mixerInputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[0][2*i];
                        mixerInputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[1][2*i];
                        mixerInputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[0][(2*i)+1];
                        mixerInputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[1][(2*i)+1];
                    }
                }
                else
                {
                    /* Mute for PCM or compressed inputs is identical */
                    mixerInputSettings.coefficients[i][0][0] = 0;
                    mixerInputSettings.coefficients[i][1][0] = 0;
                    mixerInputSettings.coefficients[i][0][1] = 0;
                    mixerInputSettings.coefficients[i][1][1] = 0;
                }
                BDBG_MODULE_MSG(bape_mixer_input_coeffs_detail, ("Save In %d -> Mxr %2d Coeffs: [%2d] %6x %6x %6x %6x", index, BAPE_MixerGroup_P_GetDpMixerId(mixerGroup, i), i,
                          mixerInputSettings.coefficients[i][0][0],
                          mixerInputSettings.coefficients[i][1][0],
                          mixerInputSettings.coefficients[i][0][1],
                          mixerInputSettings.coefficients[i][1][1]));
            }
            errCode = BAPE_MixerGroup_P_SetInputSettings(mixerGroup, index, &mixerInputSettings);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_SetInputVolume(
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
        errCode = BAPE_StandardMixer_P_ApplyInputVolume(handle, i);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_GetInputSettings(
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
static BERR_Code BAPE_StandardMixer_P_SetInputSettings(
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

static BERR_Code BAPE_StandardMixer_P_ApplyStereoMode(BAPE_MixerHandle handle, BAPE_StereoMode stereoMode)
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
                    errCode = BAPE_StandardMixer_P_ApplyInputVolume(handle, index);
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

static uint32_t BAPE_StandardMixer_P_ApplyAdditionalOutputVolume(BAPE_OutputPort output, int index)
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

/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_ApplyOutputVolume(BAPE_MixerHandle mixer, BAPE_OutputPort output)
{
    bool pcm;
    bool handleOutputMuteFirst = true;
    unsigned i;
    BERR_Code errCode;
    BAPE_MixerGroupOutputSettings outputSettings;

    BDBG_ASSERT(mixer == output->mixer);
    BDBG_ASSERT(NULL != output->sourceMixerGroup);

    BAPE_MixerGroup_P_GetOutputSettings(output->sourceMixerGroup, output->sourceMixerOutputIndex, &outputSettings);

    pcm = BAPE_FMT_P_IsLinearPcm_isrsafe(BAPE_Mixer_P_GetOutputFormat(mixer));

    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        if (output->additionalGain == 0)
        {
            outputSettings.coefficients[i] = (pcm) ? output->volume.volume[i] : BAPE_VOLUME_NORMAL;
        }
        else
        {
            outputSettings.coefficients[i] = (pcm) ? BAPE_StandardMixer_P_ApplyAdditionalOutputVolume(output, i): BAPE_VOLUME_NORMAL;
        }
    }

    if ( output->muteInMixer )
    {
        outputSettings.muted = output->volume.muted;
        outputSettings.volumeRampDisabled = !pcm;  /* Disable volume ramp if input is compressed for a pseudo-compressed-mute */
    }
    else
    {
        outputSettings.muted = false;
        outputSettings.volumeRampDisabled = false;
    }

    if ( (output->volume.muted && pcm) || (!output->volume.muted && !pcm) )
    {
        handleOutputMuteFirst = false;
    }

    /* mute output before mixer mute settings have been applied */
    if ( output->setMute && handleOutputMuteFirst )
    {
        /* Call the output's mute handler */
        BDBG_MSG(("Setting output mute to %u", output->volume.muted));
        output->setMute(output, output->volume.muted, false);
    }

    errCode = BAPE_MixerGroup_P_SetOutputSettings(output->sourceMixerGroup ,output->sourceMixerOutputIndex, &outputSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* unmute output after mixer mute settings have been applied */
    if ( output->setMute && !handleOutputMuteFirst )
    {
        /* Call the output's mute handler */
        BDBG_MSG(("Setting output mute to %u", output->volume.muted));
        output->setMute(output, output->volume.muted, false);
    }

    return BERR_SUCCESS;
}

static bool BAPE_StandardMixer_P_ValidateSettings(BAPE_Handle hApe, const BAPE_MixerSettings *pSettings)
{
    switch ( pSettings->defaultSampleRate )
    {
    case 32000:
    case 44100:
    case 48000:
        break;
    default:
        BDBG_ERR(("Mixers only support default sample rates of 32k, 44.1k or 48kHz."));
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

static BERR_Code BAPE_StandardMixer_P_SetSettings(
    BAPE_MixerHandle hMixer,
    const BAPE_MixerSettings *pSettings
    )
{
    bool timebaseChanged;
    bool volumeChanged;

    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BDBG_ASSERT(NULL != pSettings);

    if ( !BAPE_StandardMixer_P_ValidateSettings(hMixer->deviceHandle, pSettings) )
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
            BERR_Code errCode;
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

            /* downstream connections are cascaded -- Apply output volume scaling */
            for (; mixerIndex < hMixer->numMixerGroups; mixerIndex++ )
            {
                errCode = BAPE_MixerGroup_P_ApplyOutputVolume(hMixer->mixerGroups[mixerIndex], &pSettings->outputVolume, BAPE_Mixer_P_GetOutputFormat(hMixer), 0);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
                numOutputConnections--;
                if ( numOutputConnections > 0 )
                {
                    errCode = BAPE_MixerGroup_P_ApplyOutputVolume(hMixer->mixerGroups[mixerIndex], &pSettings->outputVolume, BAPE_Mixer_P_GetOutputFormat(hMixer), 1);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                    numOutputConnections--;
                }
            }
        }

        /* Store new settings */
        hMixer->settings = *pSettings;
    }
    else
    {
        /* Give up our resources if allocated */
        BAPE_StandardMixer_P_FreeResources(hMixer);

        /* Determine if the timebase has changed */
        timebaseChanged = (hMixer->settings.outputTimebase != pSettings->outputTimebase) ? true : false;
        /* Determine if the output volume settings have changed */
        volumeChanged = BKNI_Memcmp(&hMixer->settings.outputVolume, &pSettings->outputVolume, sizeof(BAPE_OutputVolume));

        /* Store new settings, they will be applied next time the mixer tries to start */
        hMixer->settings = *pSettings;

        /* Refresh output timebase if needed */
        if ( timebaseChanged && BAPE_Mixer_P_GetOutputSampleRate(hMixer) != 0 )
        {
            BAPE_OutputPort output;

            BKNI_EnterCriticalSection();
            for ( output = BLST_S_FIRST(&hMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                if ( output->setTimingParams_isr )
                {
                    output->setTimingParams_isr(output, BAPE_Mixer_P_GetOutputSampleRate(hMixer), pSettings->outputTimebase);
                }
            }
            BKNI_LeaveCriticalSection();
        }
    }

    return BERR_SUCCESS;
}

/*************************************************************************/
/* Allocate mixer resources */
static BERR_Code BAPE_StandardMixer_P_AllocatePathFromInput(
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
    errCode = BAPE_StandardMixer_P_AllocateResources(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Grab resources for this connection */
    errCode = BAPE_StandardMixer_P_AllocateConnectionResources(handle, pConnection);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
/* Release mixer resources */
static void BAPE_StandardMixer_P_FreePathFromInput(
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
    BAPE_StandardMixer_P_FreeConnectionResources(handle, pConnection);
    if (!handle->running && !handle->startedExplicitly)
    {
        /* Free mixer-level resources if needed  */
        BAPE_StandardMixer_P_FreeResources(handle);
    }
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_ConfigPathFromInput(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection
    )
{
    unsigned inputNum;
    unsigned mixerIndex;
    BERR_Code errCode;

    BAPE_MixerHandle handle;
    BAPE_MixerGroupInputSettings mixerInputSettings;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Setup linkage from conection into mixer */
    inputNum = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
    {
        BAPE_MixerGroupHandle mixerGroup = handle->mixerGroups[mixerIndex];
        /* Link Input -> Mixer */
        BDBG_MODULE_MSG(bape_fci, ("DP Mixer %u(%p) input %lu, mixer group index %lu", BAPE_MixerGroup_P_GetDpMixerId(mixerGroup, mixerIndex), (void *)handle, (unsigned long)inputNum, (unsigned long)mixerIndex));
        BAPE_MixerGroup_P_GetInputSettings(mixerGroup, inputNum, &mixerInputSettings);
        if ( pConnection->srcGroup )
        {
            BDBG_MSG(("Linking mixer to associated SRC"));
            BDBG_MODULE_MSG(bape_fci, ("  linking to upstream SRC group %p", (void*)pConnection->srcGroup));
            BAPE_SrcGroup_P_GetOutputFciIds(pConnection->srcGroup, &mixerInputSettings.input);
        }
        else if ( pConnection->sfifoGroup )
        {
            BDBG_MSG(("Linking mixer to associated SFIFO"));
            BDBG_MODULE_MSG(bape_fci, ("  linking to upstream SFIFO group %p", (void*)pConnection->sfifoGroup));
            BAPE_SfifoGroup_P_GetOutputFciIds_isrsafe(pConnection->sfifoGroup, &mixerInputSettings.input);
        }
        else
        {
            BDBG_MSG(("Linking mixer to associated FCI"));
            BDBG_MODULE_MSG(bape_fci, ("  linking to upstream FCI group"));
            mixerInputSettings.input = pConnection->inputFciGroup;
        }
        #if BDBG_DEBUG_BUILD
        {
            unsigned i;
            for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->format); i++ )
            {
                BDBG_MODULE_MSG(bape_fci, ("    ch[%lu] fci id %lx",(unsigned long)i, (unsigned long)mixerInputSettings.input.ids[i]));
            }
        }
        #endif
        errCode = BAPE_MixerGroup_P_SetInputSettings(mixerGroup, inputNum, &mixerInputSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Done.  Refresh input scaling now */
    BAPE_StandardMixer_P_ApplyInputVolume(handle, inputNum);

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_StartPathFromInput(
    struct BAPE_PathNode *pNode,
    struct BAPE_PathConnection *pConnection
    )
{
    BAPE_MixerHandle handle;
    BAPE_Connector input;
    BAPE_OutputPort output;
    unsigned mixerIndex, inputIndex, numOutputConnections;
    BERR_Code errCode;
    BAPE_PathConnection *pOutputConnection;
    bool highPriority;
    bool volumeControlEnabled;
    BAPE_MixerGroupSettings mixerGroupSettings;
    const BAPE_FMT_Descriptor *pBfd;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    input = pConnection->pSource;
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    BDBG_MSG(("Input %s -> %s starting", input->pParent->pName, input->pName));

    /* First, setup the sample rate since it may affect output programming. */
    BKNI_EnterCriticalSection();
    BAPE_StandardMixer_P_InputSampleRateChange_isr(pNode, pConnection, pConnection->pSource->format.sampleRate);
    BKNI_LeaveCriticalSection();

    pBfd = BAPE_Mixer_P_GetOutputFormat(handle);
    highPriority = BAPE_FMT_P_IsHBR_isrsafe(pBfd);
    volumeControlEnabled = BAPE_FMT_P_IsLinearPcm_isrsafe(pBfd);

    /* Start the global mixer components */
    if ( handle->running == 0 )
    {
        mixerIndex = 0;
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

            /* Check for and Start CRCs */
            if ( output->crc != NULL )
            {
                errCode = BAPE_Crc_P_Start(output->crc);
                if ( errCode )
                {
                    BDBG_ERR(("Failed to Start Crc on output - %s", output->pName));
                }
            }

            /* Apply output volume after enabling outputs (many mute in the output itself) */
            BAPE_StandardMixer_P_ApplyOutputVolume(handle, output);
        }

        /* set mixer priority */
        for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
        {
            BAPE_MixerGroup_P_GetSettings(handle->mixerGroups[mixerIndex], &mixerGroupSettings);
            mixerGroupSettings.highPriority = highPriority;
            errCode = BAPE_MixerGroup_P_SetSettings(handle->mixerGroups[mixerIndex], &mixerGroupSettings);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }
        }

        /* count output connections */
        numOutputConnections=0;
        for ( pOutputConnection = BLST_SQ_FIRST(&handle->pathNode.connectors[0].connectionList);
              pOutputConnection != NULL;
              pOutputConnection = BLST_SQ_NEXT(pOutputConnection, downstreamNode) )
        {
            numOutputConnections++;
        }

        /* Start Mixer Outputs */
        mixerIndex=0;
        /* outputs to mixers are 1:1 */
        for ( output = BLST_S_FIRST(&handle->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            /* Start mixer output(s) */
            BDBG_ASSERT(NULL != output->sourceMixerGroup);
            errCode = BAPE_MixerGroup_P_StartOutput(output->sourceMixerGroup, output->sourceMixerOutputIndex);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }
            /* This had best not fail */
            BDBG_ASSERT(errCode == BERR_SUCCESS);
            mixerIndex++;
        }

        /* output connection mixers are cascaded */
        for ( ; mixerIndex < handle->numMixerGroups; mixerIndex++ )
        {
            errCode = BAPE_MixerGroup_P_ApplyOutputVolume(handle->mixerGroups[mixerIndex], &handle->settings.outputVolume, BAPE_Mixer_P_GetOutputFormat(handle), 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }
            errCode = BAPE_MixerGroup_P_StartOutput(handle->mixerGroups[mixerIndex], 0);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }
            numOutputConnections--;
            if ( numOutputConnections > 0 )
            {
                errCode = BAPE_MixerGroup_P_ApplyOutputVolume(handle->mixerGroups[mixerIndex], &handle->settings.outputVolume, BAPE_Mixer_P_GetOutputFormat(handle), 1);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_output;
                }
                errCode = BAPE_MixerGroup_P_StartOutput(handle->mixerGroups[mixerIndex], 1);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_output;
                }
                numOutputConnections--;
            }
        }
    }

    /* Enable Mixer Inputs Next */
    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
    {
        errCode = BAPE_MixerGroup_P_StartInput(handle->mixerGroups[mixerIndex], inputIndex);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_input;
        }
        /* This had best not fail */
        BDBG_ASSERT(errCode == BERR_SUCCESS);
    }

    /* Enable SRC's next */
    if ( pConnection->srcGroup )
    {
        errCode = BAPE_SrcGroup_P_Start(pConnection->srcGroup);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_src;
        }
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
                BAPE_StandardMixer_P_ApplyOutputVolume(handle, output);
            }

            BAPE_MixerGroup_P_GetSettings(output->sourceMixerGroup, &mixerGroupSettings);
            mixerGroupSettings.volumeControlEnabled = volumeControlEnabled;
            errCode = BAPE_MixerGroup_P_SetSettings(output->sourceMixerGroup, &mixerGroupSettings);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_output;
            }

            if ( !volumeControlEnabled )
            {
                /* we are headed to PCM mode, apply volume coeffs after enabling the volume
                   control above.  This is necessary to ensure we receive valid status
                   and wait for ramping to complete. */
                BAPE_StandardMixer_P_ApplyOutputVolume(handle, output);
            }
        }
    }

    /* Check for and start CRCs attached to inputs */
    if ( inputIndex != BAPE_MIXER_INPUT_INDEX_INVALID &&
         handle->crcs[inputIndex] != NULL )
    {
        if ( pConnection->sfifoGroup )
        {
            errCode = BAPE_Crc_P_Start(handle->crcs[inputIndex]);
            if ( errCode )
            {
                BDBG_ERR(("Failed to Start Crc on input %d", inputIndex));
            }
        }
        else
        {
            BDBG_ERR(("ERROR, invalid mode - Mixers with no Sfifo are not currently supported"));
        }
    }

    BDBG_MSG(("Mixer Input %s [%u] started", input->pName, inputIndex));
    handle->running++;
    handle->inputRunning[inputIndex] = true;

    return BERR_SUCCESS;

err_src:
    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
    for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
    {
        BAPE_MixerGroup_P_StopInput(handle->mixerGroups[mixerIndex], inputIndex);
    }
err_input:
err_output:
    if ( handle->running == 0 )
    {
        mixerIndex = 0;
        /* Disable all the mixer outputs */
        for ( output = BLST_S_FIRST(&handle->outputList);
            output != NULL;
            output = BLST_S_NEXT(output, node) )
        {
            BAPE_MixerGroup_P_StopOutput(output->sourceMixerGroup, output->sourceMixerOutputIndex);

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

        for ( ; mixerIndex < handle->numMixerGroups; mixerIndex++ )
        {
            BAPE_MixerGroup_P_StopOutput(handle->mixerGroups[mixerIndex], 0);
            numOutputConnections--;
            if ( numOutputConnections > 0 )
            {
                BAPE_MixerGroup_P_StopOutput(handle->mixerGroups[mixerIndex], 1);
            }
            numOutputConnections--;
        }
    }
    return errCode;
}

void BAPE_StandardMixer_P_SfifoStarted(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
#if BAPE_DSP_SUPPORT
    unsigned inputIndex;
    BERR_Code errCode;

    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);
    if ( inputIndex >= BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        BDBG_ASSERT(inputIndex < BAPE_CHIP_MAX_MIXER_INPUTS);
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    if ( handle->inputCaptures[inputIndex] )
    {
        unsigned i;
        BDSP_StageAudioCaptureSettings capSettings;
        BDSP_Stage_GetDefaultAudioCaptureSettings(&capSettings);
        capSettings.numChannelPair = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->format);
        for ( i = 0; i < capSettings.numChannelPair; i++ )
        {
            capSettings.channelPairInfo[i].outputBuffer.hBlock = pConnection->pSource->pBuffers[i]->block;
            capSettings.channelPairInfo[i].outputBuffer.pAddr = pConnection->pSource->pBuffers[i]->pMemory;
            capSettings.channelPairInfo[i].outputBuffer.offset = pConnection->pSource->pBuffers[i]->offset;
            capSettings.channelPairInfo[i].bufferSize = pConnection->pSource->pBuffers[i]->bufferSize;
            BDBG_MSG(("setting BDSP stage capture ch pair %d to hBlock %p, pAddr %p, offset " BDBG_UINT64_FMT, i, (void*)capSettings.channelPairInfo[i].outputBuffer.hBlock, (void*)capSettings.channelPairInfo[i].outputBuffer.pAddr, BDBG_UINT64_ARG(capSettings.channelPairInfo[i].outputBuffer.offset)));
        }
        errCode = BDSP_Stage_AddAudioCapture(pConnection->pSource->hStage, handle->inputCaptures[inputIndex]->hCapture, pConnection->dspOutputIndex, &capSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            return;
        }
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnection);
#endif
}


/*************************************************************************/
static void BAPE_StandardMixer_P_StopPathFromInput(
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

    for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
    {
        /* wait for ramping to complete on this mixer group */
        if ( BAPE_MixerGroup_P_WaitForRamping(handle->mixerGroups[mixerIndex]) == BERR_TIMEOUT )
        {
            BDBG_ERR(("WARNING - %s - Vol Ramp timed out...", BSTD_FUNCTION));
        }
    }

    /* Stop Sfifo CRCs */
    if ( inputIndex != BAPE_MIXER_INPUT_INDEX_INVALID &&
         handle->crcs[inputIndex] != NULL )
    {
        BAPE_Crc_P_Stop(handle->crcs[inputIndex]);
    }

    /* Disable SRC */
    if ( pConnection->srcGroup )
    {
        BAPE_SrcGroup_P_Stop(pConnection->srcGroup);
    }

    /* Disable input to each mixer */
    for ( mixerIndex = 0; mixerIndex < handle->numMixerGroups; mixerIndex++ )
    {
        BAPE_MixerGroup_P_StopInput(handle->mixerGroups[mixerIndex], inputIndex);
    }

    BDBG_ASSERT(handle->running > 0);
    BDBG_MSG(("Mixer run count was %u now %u", handle->running, handle->running-1));
    handle->running--;
    /* If there are other running inputs, stop outputs here. */
    if ( handle->running == 0 )
    {
        /* We're the last running input.  Stop the mixers and outputs now. */
        BDBG_MSG(("Mixer %p (%d) has no more running inputs.  Stopping all mixer outputs.", (void *)handle, handle->index));

        for ( output = BLST_S_FIRST(&handle->outputList);
            output != NULL;
            output = BLST_S_NEXT(output, node) )
        {
            BDBG_ASSERT(NULL != output->sourceMixerGroup);
            BAPE_MixerGroup_P_StopOutput(output->sourceMixerGroup, output->sourceMixerOutputIndex);
        }


        mixerIndex = 0;
        for ( output = BLST_S_FIRST(&handle->outputList);
            output != NULL;
            output = BLST_S_NEXT(output, node) )
        {
            if ( output->crc != NULL )
            {
                BAPE_Crc_P_Stop(output->crc);
            }

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
        for ( ; mixerIndex < handle->numMixerGroups; mixerIndex++ )
        {
            BAPE_MixerGroup_P_StopOutput(handle->mixerGroups[mixerIndex], 0);
            numOutputConnections--;
            if ( numOutputConnections > 0 )
            {
                BAPE_MixerGroup_P_StopOutput(handle->mixerGroups[mixerIndex], 1);
            }
            numOutputConnections--;
        }
    }
}

static BERR_Code BAPE_StandardMixer_P_OutputConnectionAdded(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
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
    BAPE_StandardMixer_P_FreeResources(handle);

    return BERR_SUCCESS;
}

static void BAPE_StandardMixer_P_OutputConnectionRemoved(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;

    BSTD_UNUSED(pConnection);
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Make sure we're not running.  Should never happen. */
    BDBG_ASSERT(handle->running == 0);

    /* Release any saved resources and re-allocate on the next start call.  This will also remove any output connection resources. */
    BAPE_StandardMixer_P_FreeResources(handle);

    /* Decrement number of output connections */
    BDBG_ASSERT(handle->numOutputConnections > 0);
    handle->numOutputConnections--;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_InputSampleRateChange_isr(
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
        BAPE_StandardMixer_P_SetSampleRate_isr(handle, handle->settings.mixerSampleRate);
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
            BAPE_StandardMixer_P_SetSampleRate_isr(handle, mixerRate);
        }
        else if ( BAPE_Mixer_P_GetOutputSampleRate(handle) == 0 )
        {
            /* Make sure there is a valid sample rate */
            BAPE_StandardMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }
    else
    {
        /* Make sure there is a valid sample rate */
        if ( BAPE_Mixer_P_GetOutputSampleRate(handle) == 0 )
        {
            BAPE_StandardMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }

    /* Update SRCs accordingly. */
    BAPE_StandardMixer_P_SetInputSRC_isr(handle, pConnection->pSource, sampleRate, BAPE_Mixer_P_GetOutputSampleRate(handle));

    if ( pConnection->format.ppmCorrection && NULL != pConnection->sfifoGroup )
    {
        /* Update adaptive rate controllers */
        BAPE_SfifoGroup_P_SetSampleRate_isr(pConnection->sfifoGroup, sampleRate);
    }
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_InputFormatChange(
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
        BAPE_StandardMixer_P_FreeConnectionResources(handle, pConnection);

        /* Some changes are not supported while running.  Check these now. */
        if ( handle->running > 0 )
        {
            if ( BAPE_FMT_P_IsLinearPcm_isrsafe(pNewFormat) != BAPE_FMT_P_IsLinearPcm_isrsafe(BAPE_Mixer_P_GetOutputFormat(handle)) )
            {
                BDBG_ERR(("Can not change from compressed to PCM or vice-versa while other inputs are running."));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }

            /* TODO: Technically, this is possible if we permit downmixing in the mixer.  Currently not required for STB use cases. */
            if ( BAPE_FMT_P_GetNumChannelPairs_isrsafe(pNewFormat) > BAPE_FMT_P_GetNumChannelPairs_isrsafe(BAPE_Mixer_P_GetOutputFormat(handle)) )
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
    if ( dataType != BAPE_Mixer_P_GetOutputDataType(handle) )
    {
        BAPE_FMT_Descriptor format;

        /* If the data format has changed while stopped, re-evaluate the output data format. */
        BAPE_StandardMixer_P_FreeResources(handle);

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
    BAPE_StandardMixer_P_InputSampleRateChange_isr(pNode, pConnection, pNewFormat->sampleRate);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}


/*************************************************************************/
static void BAPE_StandardMixer_P_InputMute(
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
        BAPE_StandardMixer_P_ApplyInputVolume(handle, i);
    }
}


/*************************************************************************/
static BAPE_MclkSource BAPE_StandardMixer_P_GetMclkSource(BAPE_MixerHandle mixer)
{
    BAPE_OutputPort output;
    bool needMclk = false;
    bool pllRequired = false;
    bool ncoRequired = false;
    BAPE_MixerHandle thisMixer;

    /* Do we even need an MCLK source? */
    BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Beginning MCLK selection", (void *)mixer ));

    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
        if ( output->setMclk_isr || output->fsTiming )
        {
            needMclk = true;
            break;
        }
    }

    if ( false == needMclk )
    {
        /* No outputs need MCLK (all are DACs) */
        BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Done (no outputs need MCLK) Returning %s",
                                                     (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(BAPE_MclkSource_eNone )));
        return BAPE_MclkSource_eNone;
    }

    /* Check local outputs first */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
        if ( output->mclkOutput != BAPE_MclkSource_eNone )
        {
            BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Done (output provides MCLK) Returning %s",
                                                         (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(output->mclkOutput) ));
            return output->mclkOutput;  /* the mixer has a DAC output, use the DAC's rate manager */
        }
        if ( output->pllRequired )
        {
            pllRequired = true;
        }
        if ( output->builtinNco )
        {
            ncoRequired = true;
        }
    }

    /* At this point, the mixer has no outputs generating a usable clock source,
     * now see if there are any outputs that request a PLL (like Spdif or I2S), if so,
     * then try use the PLL specified in settings.outputPll.  If we have an output that
     * requires an NCO (e.g. DAC on 7429) then skip this.
     */
    if ( ncoRequired )
    {
        BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Outputs require an NCO clock", (void *)mixer ));
    }
    else if ( pllRequired )
    {
        BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Outputs desire a PLL clock", (void *)mixer ));
        /* If the outputPll mixer setting is valid, use it. */
        if ( (signed)mixer->settings.outputPll >= BAPE_Pll_e0 && mixer->settings.outputPll < BAPE_Pll_eMax )
        {
            unsigned pllIndex = mixer->settings.outputPll - BAPE_Pll_e0;
            if ( pllIndex < BAPE_CHIP_MAX_PLLS )
            {
                BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Done (valid PLL specified) Returning %s", (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(pllIndex + BAPE_MclkSource_ePll0)));
                return pllIndex + BAPE_MclkSource_ePll0;
            }
        }
        BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Valid PLL was not specified, continuing...", (void *)mixer ));
    }

    /* Now use an NCO if one was specified. */
    #if BAPE_CHIP_MAX_NCOS > 0
        if ( (signed)mixer->settings.outputNco >= BAPE_Nco_e0 && mixer->settings.outputNco < BAPE_Nco_eMax )
        {
            unsigned ncoIndex = mixer->settings.outputNco - BAPE_Nco_e0;
            if ( ncoIndex < BAPE_CHIP_MAX_NCOS )
            {
                BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Done (valid NCO specified) Returning %s", (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(ncoIndex + BAPE_MclkSource_eNco0) ));
                return ncoIndex + BAPE_MclkSource_eNco0;
            }
        }
    #endif /* BAPE_CHIP_MAX_NCOS */

    if ( ncoRequired )
    {
        BDBG_ERR(("Mixer %p requires an NCO but one has not been specified.", (void *)mixer));
        return BAPE_MclkSource_eMax;
    }
    else
    {
        BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Valid NCO was not specified, continuing...", (void *)mixer ));
    }

    BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Peeking for DAC on another mixer", (void *)mixer ));
    /* Check all open mixers with matching inputs and settings  */
    for ( thisMixer = BLST_S_FIRST(&mixer->deviceHandle->mixerList);
          thisMixer != NULL;
          thisMixer = BLST_S_NEXT(thisMixer, node) )
    {
        int i, j;
        bool matchInput=true, matchSettings=false;

        if ( thisMixer == mixer )
        {
            continue;
        }

        /* Determine if we have a matching timing or not */
        if ( mixer->master )
        {
            /* Must have the same master channel, timebase, and default sample rate */
            if ( thisMixer->master &&
                 (thisMixer->master->pParent == mixer->master->pParent) &&
                 (thisMixer->settings.outputTimebase == mixer->settings.outputTimebase) &&
                 (thisMixer->settings.defaultSampleRate == mixer->settings.defaultSampleRate) &&
                 (thisMixer->settings.defaultSampleRate == mixer->settings.mixerSampleRate) )
            {
                matchSettings = true;
            }
        }
        else
        {
            /* No master, just default timing. */
            if ( NULL == thisMixer->master &&
                 (thisMixer->settings.outputTimebase == mixer->settings.outputTimebase) &&
                 (thisMixer->settings.defaultSampleRate == mixer->settings.defaultSampleRate) &&
                 (thisMixer->settings.defaultSampleRate == mixer->settings.mixerSampleRate) )
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
                         (thisMixer->inputs[j]->pParent == mixer->inputs[i]->pParent) )
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
                         (mixer->inputs[j]->pParent == thisMixer->inputs[i]->pParent) )
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

        /* If we've matched settings and input(s), look for an mclk source */
        if ( matchSettings && matchInput )
        {
            for ( output = BLST_S_FIRST(&thisMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);
                if ( output->mclkOutput != BAPE_MclkSource_eNone )
                {
                    BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p (Found a DAC RM on another mixer) Returning %s",
                                                                 (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(output->mclkOutput)));
                    BDBG_MSG(("Found associated MCLK source %s", BAPE_Mixer_P_MclkSourceToText_isrsafe(output->mclkOutput)));
                    return output->mclkOutput;
                }
            }
        }
    }
    BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Couldn't find usable DAC on another mixer", (void *)mixer ));


    /* If we get here, we haven't found one.  Return the PLL if
     * one was specified, otherwise just fail.
     */
    if ( (signed)mixer->settings.outputPll >= BAPE_Pll_e0 && mixer->settings.outputPll < BAPE_Pll_eMax )
    {
        unsigned pllIndex = mixer->settings.outputPll - BAPE_Pll_e0;
        if ( pllIndex < BAPE_CHIP_MAX_PLLS )
        {
            BDBG_MODULE_MSG(bape_mixer_get_mclk_source, ("Mixer:%p Done (last resort, using specified PLL) Returning %s", (void *)mixer, BAPE_Mixer_P_MclkSourceToText_isrsafe(pllIndex + BAPE_MclkSource_ePll0)));
            return pllIndex + BAPE_MclkSource_ePll0;
        }
    }

    BDBG_ERR(("Can't find MCLK source for mixer:%p, giving up.", (void *)mixer ));

    return BERR_TRACE(BAPE_MclkSource_eMax);    /* this indicates failure */
}

/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_AllocateResources(BAPE_MixerHandle mixer)
{
    unsigned i,j;
    BAPE_OutputPort output;
    BAPE_MclkSource mclkSource;
    unsigned numOutputs=0, numOutputConnections=0;
    bool fsRequired = false;
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
            BDBG_ERR(("Output %s can not accept a data type of %s", output->pName, BAPE_FMT_P_GetTypeName_isrsafe(&newFormat)));
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

    /* This allocates the required number of mixers */
    errCode = BAPE_StandardMixer_P_AllocateMixerGroups(mixer);
    if ( errCode )
    {
        BDBG_ERR(("Unable to allocate sufficient hardware mixers.  Please reduce the number of outputs and/or the number of output channels."));
        errCode = BERR_TRACE(errCode);
        goto err_mixers;
    }

    /* Now, we have the resources we need.  Setup the linkage correctly. 1:1 mixers:outputs*/
    i=0;
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        /* First output always to master */
        output->sourceMixerGroup = mixer->mixerGroups[i];
        output->sourceMixerOutputIndex = 0;
        BAPE_MixerGroup_P_GetOutputFciIds(mixer->mixerGroups[i], 0, &output->sourceMixerFci);
        #if BDBG_DEBUG_BUILD
        {
            unsigned z;
            if ( i == 0 )
            {
                BDBG_MODULE_MSG(bape_fci, ("Push mixer %p(%s) to outputs",(void *)mixer, mixer->pathNode.pName));
            }
            for ( z=0; z<BAPE_FMT_P_GetNumChannelPairs_isrsafe(&newFormat); z++ )
            {
                BDBG_MODULE_MSG(bape_fci, ("  pushed mixer output fci[%u] = %x to output %s", z, output->sourceMixerFci.ids[z], output->pName));
            }
        }
        #endif
        i++;
    }

    /* Setup Downstream connections as well, cascading mixers */
    j=0;
    for ( pConnection = BLST_SQ_FIRST(&mixer->pathNode.connectors[0].connectionList);
          pConnection != NULL;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        unsigned outputIdx;

        if ( (mixer->numOutputConnections - j) == 1 && j > 0 )
        {
            outputIdx = 1;
        }
        else
        {
            outputIdx = 0;
        }
        BDBG_ASSERT(mixer->numOutputConnections <= 2);
        /* TODO - connect mixer[n] -> mixer[n+1], for more than two consumers...
        if ( j > 0 && outputIdx == 0 )
        {
        }
        */
        BAPE_MixerGroup_P_GetOutputFciIds(mixer->mixerGroups[i + (j - outputIdx)], outputIdx, &pConnection->inputFciGroup);
        #if BDBG_DEBUG_BUILD
        {
            unsigned z;
            if ( j == 0 )
            {
                BDBG_MODULE_MSG(bape_fci, ("Push mixer %p(%s) to other connections",(void *)mixer, mixer->pathNode.pName));
            }
            for ( z=0; z<BAPE_FMT_P_GetNumChannelPairs_isrsafe(&newFormat); z++ )
            {
                BDBG_MODULE_MSG(bape_fci, ("  pushed mixer output fci[%u] = %x to %s", z, pConnection->inputFciGroup.ids[z], pConnection->pSink ? pConnection->pSink->pName : "NO SINK?"));
            }
        }
        #endif
        j++;
    }
    i += j;

    /* Refresh output scaling */
    for ( output = BLST_S_FIRST(&mixer->outputList);
          output != NULL;
          output = BLST_S_NEXT(output, node) )
    {
        BDBG_MSG(("Refreshing output scaling for output %p", (void *)output));
        BAPE_StandardMixer_P_ApplyOutputVolume(mixer, output);
    }

    mclkSource = BAPE_StandardMixer_P_GetMclkSource(mixer);
    if ( mclkSource >= BAPE_MclkSource_eMax )
    {
        BDBG_ERR(("Unable to find MclkSource for mixer."));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_mixer_mclksource;
    }

    mixer->mclkSource = mclkSource;
    if (BAPE_MCLKSOURCE_IS_PLL(mclkSource))
    {
        #if BAPE_CHIP_MAX_PLLS > 0
        BAPE_P_AttachMixerToPll(mixer, mixer->mclkSource - BAPE_MclkSource_ePll0);
        #endif
    }
    else if (BAPE_MCLKSOURCE_IS_NCO(mclkSource))
    {
        #if BAPE_CHIP_MAX_NCOS > 0
            BAPE_P_AttachMixerToNco(mixer, mixer->mclkSource - BAPE_MclkSource_eNco0);
        #endif
    }
    else  /* Must be a DAC or none. */
    {
        BKNI_EnterCriticalSection();
        /* Set outputs to DAC rate manager MCLK if required */
        for ( output = BLST_S_FIRST(&mixer->outputList);
              output != NULL;
              output = BLST_S_NEXT(output, node) )
        {
            if ( output->setMclk_isr )
            {
                output->setMclk_isr(output, mixer->mclkSource, 0, 256 /* DAC is 256*Fs */); /* Actually, at Fs > 48 KHz the DAC runs at 64*Fs */
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
            goto err_alloc_fs;
        }
        if ( BAPE_MCLKSOURCE_IS_PLL(mclkSource) )
        {
            BKNI_EnterCriticalSection();
            errCode = BAPE_P_UpdatePll_isr(mixer->deviceHandle, mixer->settings.outputPll);
            BKNI_LeaveCriticalSection();
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_alloc_fs;
            }
        }
        #if BAPE_CHIP_MAX_NCOS > 0
        else if ( BAPE_MCLKSOURCE_IS_NCO(mclkSource) )
        {
            /* Update the NCO */
            BKNI_EnterCriticalSection();
            errCode = BAPE_P_UpdateNco_isr(mixer->deviceHandle, mixer->settings.outputNco);
            BKNI_LeaveCriticalSection();
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_alloc_fs;
            }
        }
        #endif
        else
        {
            /* Set the FS to use DAC timing */
            BKNI_EnterCriticalSection();
            BAPE_P_SetFsTiming_isr(mixer->deviceHandle,
                                   mixer->fs,
                                   mixer->mclkSource,
                                   0,       /* pllChan doesn't apply */
                                   256 );   /* DAC is 256Fs */
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
                output->setFs(output, mixer->fs);
            }
        }
    }

    mixer->resourcesAllocated = true;
    return BERR_SUCCESS;

err_alloc_fs:
    if ( mixer->fs != BAPE_FS_INVALID )
    {
        BAPE_P_FreeFs(mixer->deviceHandle, mixer->fs);
        mixer->fs = BAPE_FS_INVALID;
    }

err_mixer_mclksource:
    BAPE_StandardMixer_P_FreeResources(mixer);

err_mixers:
    return errCode;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);

    /* Only do this if something actually changed */
    BDBG_MSG(("Set Mixer Output Rate to %u (was %u)", sampleRate, BAPE_Mixer_P_GetOutputSampleRate(mixer)));
    if ( BAPE_Mixer_P_GetOutputSampleRate(mixer) != sampleRate )
    {
        BAPE_OutputPort output;

        if ( sampleRate == 0 )

        BDBG_MSG(("Changing mixer %p (%d) sample rate to %u [was %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate(mixer)));

        (void)BAPE_Connector_P_SetSampleRate_isr(&mixer->pathNode.connectors[0], sampleRate);

        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( mixer->inputs[i] )
            {
                BAPE_StandardMixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->format.sampleRate, sampleRate);
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
        BDBG_MSG(("NOT Changing mixer %p (%d) sample rate to %u [currently %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate(mixer)));
    }

    for ( i = 0; i < mixer->numMixerGroups; i++ )
    {
        if ( mixer->mixerGroups[i] != NULL )
        {
            /* used to calculate micro seconds per ramp step */
            BAPE_MixerGroup_P_SetRampSamplerate_isr( mixer->mixerGroups[i], sampleRate );
        }
    }
}

/*************************************************************************/
static void BAPE_StandardMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate)
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

    if ( pLink->srcGroup )
    {
        BDBG_MSG(("Setting input %s SRC to %u->%u", input->pName, inputRate, outputRate));
        BAPE_SrcGroup_P_SetSampleRate_isr(pLink->srcGroup, inputRate, outputRate);
    }
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_AllocateConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    unsigned i;
    BERR_Code errCode;
    bool sfifoRequired=false, srcRequired=false, buffersOnly=false;

    BAPE_Connector input;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(&handle->pathNode == pConnection->pSink);

    input = pConnection->pSource;

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    BDBG_ASSERT(i != BAPE_MIXER_INPUT_INDEX_INVALID);

    switch ( pConnection->pSource->format.source )
    {
    case BAPE_DataSource_eDfifo:
    case BAPE_DataSource_eDspBuffer:
    case BAPE_DataSource_eHostBuffer:
        /* SFIFO is required - also always allocate SRC since it handles startup ramp and input mute */
        sfifoRequired = true;
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

    BDBG_MSG(("Mixer %p Allocating resources for input %s %s", (void *)handle, input->pParent->pName, input->pName));

    /* Check if we've already allocated resources */
    if ( pConnection->resourcesAllocated )
    {
        if ( (pConnection->format.type == input->format.type) &&
             (pConnection->format.ppmCorrection == input->format.ppmCorrection) )
        {
            BDBG_MSG(("Connection %s -> %s %s already has the required resources", input->pParent->pName, pConnection->pSink->pName, input->pName));
            if ( sfifoRequired )
            {
                BDBG_ASSERT(NULL != pConnection->sfifoGroup);
            }
            if ( srcRequired )
            {
                BDBG_ASSERT(NULL != pConnection->srcGroup);
            }

            buffersOnly = true;
        }
        else
        {
            /* Connection format has changed */
            BDBG_MSG(("Connection %s -> %s %s format changed.  Releasing resources.", input->pParent->pName, pConnection->pSink->pName, input->pName));
            BAPE_StandardMixer_P_FreeConnectionResources(handle, pConnection);
        }
    }

    if ( input->useBufferPool )
    {
        /* This is safe to call multiple times, it only allocates if need be */
        errCode = BAPE_P_AllocateInputBuffers(handle->deviceHandle, input);
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

    BDBG_ASSERT(NULL == pConnection->sfifoGroup);
    BDBG_ASSERT(NULL == pConnection->srcGroup);

    if ( sfifoRequired )
    {
        BAPE_SfifoGroupCreateSettings sfifoCreateSettings;
        BAPE_SfifoGroup_P_GetDefaultCreateSettings(&sfifoCreateSettings);
        sfifoCreateSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&input->format);
        sfifoCreateSettings.ppmCorrection = input->format.ppmCorrection;
        errCode = BAPE_SfifoGroup_P_Create(handle->deviceHandle, &sfifoCreateSettings, &pConnection->sfifoGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_alloc_sfifo;
        }
    }

    if ( srcRequired )
    {
        BAPE_SrcGroupSettings srcSettings;
        BAPE_SrcGroupCreateSettings srcCreateSettings;
        BAPE_SrcGroup_P_GetDefaultCreateSettings(&srcCreateSettings);
        srcCreateSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&input->format);
        errCode = BAPE_SrcGroup_P_Create(handle->deviceHandle, &srcCreateSettings, &pConnection->srcGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_alloc_src;
        }
        /* Set SRC Linkage */
        BDBG_MODULE_MSG(bape_fci, ("Linking input %s -> SRC %p (numChPairs %lu)", pConnection->pSource->pParent->pName, (void*)pConnection->srcGroup, (unsigned long)srcCreateSettings.numChannelPairs));
        BAPE_SrcGroup_P_GetSettings(pConnection->srcGroup, &srcSettings);
        if ( pConnection->sfifoGroup )
        {
            /* Input From Sfifo */
            BDBG_MODULE_MSG(bape_fci, ("  SFIFO -> SRC"));
            BAPE_SfifoGroup_P_GetOutputFciIds_isrsafe(pConnection->sfifoGroup, &srcSettings.input);
        }
        else
        {
            /* Input from other FCI source */
            BDBG_MODULE_MSG(bape_fci, ("  FCI -> SRC"));
            srcSettings.input = pConnection->inputFciGroup;
        }
        #if BDBG_DEBUG_BUILD
        {
            unsigned i;
            for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&input->format); i++ )
            {
                BDBG_MODULE_MSG(bape_fci, ("    ch[%lu] fci id %lx",(unsigned long)i, (unsigned long)srcSettings.input.ids[i]));
            }
        }
        #endif
        srcSettings.rampEnabled = BAPE_FMT_P_RampingValid_isrsafe(&input->format);
        srcSettings.startupRampEnabled = srcSettings.rampEnabled;
        errCode = BAPE_SrcGroup_P_SetSettings(pConnection->srcGroup, &srcSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_src_settings;
        }
        BKNI_EnterCriticalSection();
        BAPE_SrcGroup_P_SetSampleRate_isr(pConnection->srcGroup, pConnection->format.sampleRate, BAPE_Mixer_P_GetOutputSampleRate(handle));
        BKNI_LeaveCriticalSection();
    }

    pConnection->format = input->format;
    pConnection->resourcesAllocated = true;

    return BERR_SUCCESS;

    err_src_settings:
    if ( pConnection->srcGroup )
    {
        BAPE_SrcGroup_P_Destroy(pConnection->srcGroup);
        pConnection->srcGroup = NULL;
    }
    err_alloc_src:
    if ( pConnection->sfifoGroup )
    {
        BAPE_SfifoGroup_P_Destroy(pConnection->sfifoGroup);
        pConnection->sfifoGroup = NULL;
    }
    err_alloc_sfifo:
    if ( pConnection->pSource->useBufferPool )
    {
        BAPE_P_FreeInputBuffers(handle->deviceHandle, pConnection->pSource);
    }

    return errCode;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    BDBG_MSG(("Freeing connection resources for input %s %s", pConnection->pSource->pParent->pName, pConnection->pSource->pName));

    if ( pConnection->pSource->useBufferPool )
    {
        BAPE_P_FreeInputBuffers(handle->deviceHandle, pConnection->pSource);
    }
    if ( pConnection->srcGroup )
    {
        BAPE_SrcGroup_P_Destroy(pConnection->srcGroup);
        pConnection->srcGroup = NULL;
    }
    if ( pConnection->sfifoGroup )
    {
        BAPE_SfifoGroup_P_Destroy(pConnection->sfifoGroup);
        pConnection->sfifoGroup = NULL;
    }
    pConnection->resourcesAllocated = false;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_FreeResources(BAPE_MixerHandle handle)
{
    unsigned i;
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

    for ( i = 0; i < handle->numMixerGroups; i++ )
    {
        BDBG_ASSERT(NULL != handle->mixerGroups[i]);
        BAPE_MixerGroup_P_Destroy(handle->mixerGroups[i]);
        handle->mixerGroups[i] = NULL;
    }
    handle->numMixerGroups = 0;

    handle->resourcesAllocated = false;
}

/*************************************************************************/
static void BAPE_StandardMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_StandardMixer_P_RemoveInput(pNode->pHandle, pConnector);
}


/*************************************************************************/
static BERR_Code BAPE_StandardMixer_P_AllocateMixerGroups(BAPE_MixerHandle handle)
{
    BERR_Code errCode=BERR_SUCCESS;
    BAPE_MixerGroupCreateSettings createSettings;
    unsigned i, numOutputs;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

#if BDBG_DEBUG_BUILD
    /* Sanity check, make sure all resources are marked invalid in the mixer */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        BDBG_ASSERT(NULL == handle->mixerGroups[i]);
    }
    BDBG_ASSERT(handle->numMixerGroups == 0);
#endif

    BAPE_MixerGroup_P_GetDefaultCreateSettings(&createSettings);
    createSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(BAPE_Mixer_P_GetOutputFormat(handle));
    /* Allocate 1:1 mixers:outputs and cascade the output connections for other consumers */
    numOutputs = handle->numOutputs;
    numOutputs += (handle->numOutputConnections > 1) ? (handle->numOutputConnections-1) : handle->numOutputConnections;
    BDBG_MSG(("Mixer has %d outputs, %d outputConnections, numChPairs %d, explicitFormat %d", numOutputs, handle->numOutputConnections, createSettings.numChannelPairs, handle->explicitFormat));
    for ( i = 0; i < numOutputs; i++ )
    {
        errCode = BAPE_MixerGroup_P_Create(handle->deviceHandle, &createSettings, &handle->mixerGroups[i]);
        BDBG_MSG(("  Created Mixer group %p, numChPairs %d, explicitFormat %d", (void*)handle->mixerGroups[i], createSettings.numChannelPairs, handle->explicitFormat));
        if ( errCode )
        {
            goto err_create_group;
        }
        handle->numMixerGroups++;
    }
    BDBG_MSG(("  Created %d output mixer groups for parent mixer %p", handle->numMixerGroups, (void*)handle));

    return BERR_SUCCESS;

    err_create_group:
    for ( i = 0; i < handle->numMixerGroups; i++ )
    {
        BAPE_MixerGroup_P_Destroy(handle->mixerGroups[i]);
        handle->mixerGroups[i] = NULL;
    }
    handle->numMixerGroups = 0;

    return errCode;
}

/*************************************************************************/
/* Declare the interface struct containing the API callbacks */

#define NOT_SUPPORTED(x) NULL
/* If any of the APIs are not supported by a particular type of mixer,
 * just add a placeholder like this:
 *   NOT_SUPPORTED(BAPE_StandardMixer_P_RemoveAllInputs)
 */

static const BAPE_MixerInterface  standardMixerInterface  = {
    BAPE_StandardMixer_P_Destroy,                /*       (*destroy)           */
    BAPE_StandardMixer_P_Start,                  /*       (*start)             */
    BAPE_StandardMixer_P_Stop,                   /*       (*stop)              */
    BAPE_StandardMixer_P_AddInput,               /*       (*addInput)          */
    BAPE_StandardMixer_P_RemoveInput,            /*       (*removeInput)       */
    BAPE_StandardMixer_P_RemoveAllInputs,        /*       (*removeAllInputs)   */
    BAPE_StandardMixer_P_AddOutput,              /*       (*addOutput)         */
    BAPE_StandardMixer_P_RemoveOutput,           /*       (*removeOutput)      */
    BAPE_StandardMixer_P_RemoveAllOutputs,       /*       (*removeAllOutputs)  */
    BAPE_StandardMixer_P_GetInputVolume,         /*       (*getInputVolume)    */
    BAPE_StandardMixer_P_SetInputVolume,         /*       (*setInputVolume)    */
    BAPE_StandardMixer_P_GetInputSettings,       /*       (*getInputSettings)  */
    BAPE_StandardMixer_P_SetInputSettings,       /*       (*setInputSettings)    */
    BAPE_StandardMixer_P_ApplyOutputVolume,      /*       (*applyOutputVolume) */
    BAPE_StandardMixer_P_SetSettings,            /*       (*setSettings) */
    BAPE_StandardMixer_P_ApplyStereoMode,        /*       (*applyStereoMode) */
    NOT_SUPPORTED(BAPE_StandardMixer_P_GetInputStatus),
    NOT_SUPPORTED(BAPE_StandardMixer_P_GetStatus)
};
