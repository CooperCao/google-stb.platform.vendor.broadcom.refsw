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
 * Module Description: Audio Input Capture Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bape_input_capture);

BDBG_OBJECT_ID(BAPE_InputCapture);

static BERR_Code BAPE_InputCapture_P_AllocatePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

static BERR_Code BAPE_InputCapture_P_FreePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

static BERR_Code BAPE_InputCapture_P_ConfigPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

static BERR_Code BAPE_InputCapture_P_StartPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

static void BAPE_InputCapture_P_StopPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    );

static BERR_Code BAPE_InputCapture_P_EnableInterrupts(BAPE_InputCaptureHandle handle);
static void BAPE_InputCapture_P_DisableInterrupts(BAPE_InputCaptureHandle handle);
static void BAPE_InputCapture_P_ClearInterrupts_isr(BAPE_InputCaptureHandle handle);

void BAPE_InputCapture_GetDefaultOpenSettings(
    BAPE_InputCaptureOpenSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code BAPE_InputCapture_P_InputFormatChange_isr(
    BAPE_PathNode *pNode,
    BAPE_InputPort inputPort
    );

static void BAPE_InputCapture_P_FreeBuffer(BAPE_InputCaptureHandle handle, unsigned idx)
{
    if ( handle->bufferBlock[idx] )
    {
        if ( handle->pBuffers[idx] )
        {
            BMMA_Unlock(handle->bufferBlock[idx], handle->pBuffers[idx]);
            handle->pBuffers[idx] = NULL;
        }
        if ( handle->bufferOffset[idx] )
        {
            BMMA_UnlockOffset(handle->bufferBlock[idx], handle->bufferOffset[idx]);
            handle->bufferOffset[idx] = 0;
        }
        BMMA_Free(handle->bufferBlock[idx]);
        handle->bufferBlock[idx] = NULL;
    }
}

BERR_Code BAPE_InputCapture_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_InputCaptureOpenSettings *pSettings,
    BAPE_InputCaptureHandle *pHandle                    /* [out] */
    )
{
    unsigned i;
    BAPE_InputCaptureOpenSettings defaultSettings;
    BAPE_InputCaptureHandle handle;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( NULL == pSettings )
    {
        BAPE_InputCapture_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    *pHandle = NULL;
    if ( index >= BAPE_CHIP_MAX_INPUT_CAPTURES )
    {
        BDBG_WRN(("This chip can not support more than %u input capture channels", BAPE_CHIP_MAX_DFIFOS));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_index;
    }

    handle = BKNI_Malloc(sizeof(BAPE_InputCapture));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_InputCapture));
    BDBG_OBJECT_SET(handle, BAPE_InputCapture);
    handle->deviceHandle = deviceHandle;
    if ( pSettings->bufferSize && pSettings->numBuffers > 0 )
    {
        handle->hHeap = pSettings->heap ? pSettings->heap : deviceHandle->memHandle;
        handle->bufferSize = pSettings->bufferSize;
        handle->numBuffers = pSettings->numBuffers;
    }

    handle->index = index;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_eInputCapture, 0, 1, deviceHandle, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "InputCapture %u", index);
    handle->node.pName = handle->name;
    handle->node.connectors[0].useBufferPool = true;
    #if BAPE_INPUT_CAPTURE_REQUIRES_SFIFO
    handle->sfifoForFmmOutputs = true;
    #else
    handle->sfifoForFmmOutputs = false;
    #endif

    /* set up capture to memory if requested */
    if ( handle->bufferSize )
    {
        BDBG_MSG(("Memory Capture Mode. Allocating resources"));
        for ( i = 0; i < handle->numBuffers; i++ )
        {
            /* Allocate buffer */
            handle->bufferBlock[i] = BMMA_Alloc(handle->hHeap, handle->bufferSize, 32, NULL);
            if ( NULL == handle->bufferBlock[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                goto err_buffer;
            }

            handle->pBuffers[i] = BMMA_Lock(handle->bufferBlock[i]);
            if ( NULL == handle->pBuffers[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BAPE_InputCapture_P_FreeBuffer(handle, i);
                goto err_buffer;
            }

            handle->bufferOffset[i] = BMMA_LockOffset(handle->bufferBlock[i]);
            if (  0 == handle->bufferOffset[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BAPE_InputCapture_P_FreeBuffer(handle, i);
                goto err_buffer;
            }

            /* Flush once at open to make sure the buffer has been invalidated from the cache. */
            BMMA_FlushCache(handle->bufferBlock[i], handle->pBuffers[i], handle->bufferSize);
        }

        handle->settings.watermark = pSettings->watermarkThreshold > 0 ? pSettings->watermarkThreshold : handle->bufferSize / 2;
        handle->settings.bitsPerSample = 32;
        handle->settings.interleaveData = true;

        BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
        format.source = BAPE_DataSource_eDfifo;
        format.type = BAPE_DataType_ePcmStereo;
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_format;
        }
    }
    else
    {
        BDBG_MSG(("FMM Capture Mode"));
        BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
        if ( handle->sfifoForFmmOutputs )
        {
            /* DFIFO -> SFIFO mode */
            format.source = BAPE_DataSource_eDfifo;
        }
        else
        {
            /* FCI splitter upstream, direct FCI -> FMM */
            format.source = BAPE_DataSource_eFci;
        }
        format.type = BAPE_DataType_ePcmStereo;
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_format;
        }
    }
    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm7_1);
#if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x4);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x16);
#endif
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_caps;
    }
    /* Init node callbacks */
    handle->node.configPathToOutput = BAPE_InputCapture_P_ConfigPathToOutput;
    handle->node.startPathToOutput = BAPE_InputCapture_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_InputCapture_P_StopPathToOutput;
    handle->node.inputPortFormatChange_isr = BAPE_InputCapture_P_InputFormatChange_isr;
    *pHandle = handle;
    deviceHandle->inputCaptures[index] = handle;
    return BERR_SUCCESS;

err_buffer:
err_format:
err_caps:
    BAPE_InputCapture_Close(handle);
err_malloc:
err_index:
    return errCode;
}

/***************************************************************************
Summary:
Close an input capture channel
***************************************************************************/
void BAPE_InputCapture_Close(
    BAPE_InputCaptureHandle handle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    if ( handle->running )
    {
        BDBG_WRN(("Stopping input capture %p (%d) on close", (void *)handle, handle->index));
        BAPE_InputCapture_Stop(handle);
    }

    /* Disconnect from all mixers, post-processors, and groups */
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[0]);

    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        if ( handle->pBuffers[i] )
        {
            BAPE_InputCapture_P_FreeBuffer(handle, i);
        }
    }

    handle->deviceHandle->inputCaptures[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_InputCapture);
    BKNI_Free(handle);
}

#if !B_REFSW_MINIMAL
void BAPE_InputCapture_GetSettings(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureSettings *pSettings       /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_InputCapture_SetSettings(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pSettings);
#if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE
    if ( pSettings->bitsPerSample != 32 && pSettings->bitsPerSample != 16)
    {
        BDBG_ERR(("Only 16-bit and 32-bit capture samples are supported on this chipset."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#else
    if ( pSettings->bitsPerSample != 32 )
    {
        BDBG_ERR(("Only 32-bit capture samples are supported on this chipset."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

void BAPE_InputCapture_Flush(
    BAPE_InputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BKNI_EnterCriticalSection();
    BAPE_InputCapture_Flush_isr(handle);
    BKNI_LeaveCriticalSection();
}
#endif

void BAPE_InputCapture_Flush_isr(
    BAPE_InputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    if ( handle->dfifoGroup )
    {
        BAPE_DfifoGroup_P_Flush_isr(handle->dfifoGroup);
        BAPE_InputCapture_P_ClearInterrupts_isr(handle);
    }
}

/***************************************************************************
Summary:
Get Default InputCapture Start Settings
***************************************************************************/
void BAPE_InputCapture_GetDefaultStartSettings(
    BAPE_InputCaptureStartSettings *pSettings       /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
Start InputCapture
***************************************************************************/
BERR_Code BAPE_InputCapture_Start(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureStartSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_BufferNode *pBuffer;
    BAPE_DfifoGroupCreateSettings dfifoCreateSettings;
    BAPE_DfifoGroupSettings dfifoSettings;
    BAPE_FMT_Descriptor format;
    unsigned numBuffersRequired, i, step, numChannelPairs;
    bool captureToMemory = false;
    bool dfifoRequired = false;
    bool sfifoDownstream = false;
    BAPE_PathNode *pNode;
    unsigned numFound;

    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pSettings);

    /* If we're already running, return error */
    if ( handle->running )
    {
        BDBG_ERR(("InputCapture %p (%d) is already running.  Can't start.", (void *)handle, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check for input */
    if ( NULL == pSettings->input )
    {
        BDBG_ERR(("No input provided"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_OBJECT_ASSERT(pSettings->input, BAPE_InputPort);

    /* clear state variables*/
    handle->sfifoRequired = false;

    if ( handle->bufferSize > 0 )
    {
        captureToMemory = true;
    }

    /* Don't reference InputPort format fields until after this.  */
    errCode = BAPE_InputPort_P_AttachConsumer(pSettings->input, &handle->node, &handle->inputPortFormat);
    if ( errCode )
    {
        goto err_attach;
    }

    BDBG_MSG(("InputCapture %u attached to input %s using format %s [%u Hz]", handle->index, pSettings->input->pName, BAPE_FMT_P_GetTypeName_isrsafe(&handle->inputPortFormat), handle->inputPortFormat.sampleRate));
    numBuffersRequired = i = step = numChannelPairs = 0;
    BKNI_Memcpy(&handle->startSettings, pSettings, sizeof(handle->startSettings));
    BDBG_MSG(("InputCapture %p (%d) Starting", (void *)handle, handle->index));

    /* Need to set master connection and FCIs before configuring downstream */
    {
        BAPE_PathConnection *pConnection;
        for ( pConnection = BLST_SQ_FIRST(&handle->node.connectors[0].connectionList);
              NULL != pConnection;
              pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
        {
            BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
            BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);
            errCode = BAPE_InputCapture_P_AllocatePathFromInput(&handle->node, pConnection);
            if ( errCode )
            {
                goto err_allocate_path;
            }
        }
    }

    if ( handle->sfifoForFmmOutputs && handle->pMasterConnection )
    {
        sfifoDownstream = handle->sfifoRequired;
    }
    BDBG_MSG(("  captureToMemory %d sfifoDownstream %d", captureToMemory, sfifoDownstream));
    dfifoRequired = captureToMemory || sfifoDownstream;

    /* Setup output format */
    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format = handle->inputPortFormat;
    if ( captureToMemory || sfifoDownstream )
    {
        format.source = BAPE_DataSource_eDfifo;
    }
    else
    {
        format.source = BAPE_DataSource_eFci;
    }
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_format;
    }

    /* Build downstream network */
    BDBG_MSG(("InputCapture %p (%d) Acquire Downstream", (void *)handle, handle->index));
    errCode = BAPE_PathNode_P_AcquirePathResources(&handle->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_acquire_resources;
    }

    /* Prepare network to start */
    BDBG_MSG(("InputCapture %p (%d) Configure Downstream", (void *)handle, handle->index));
    errCode = BAPE_PathNode_P_ConfigurePathResources(&handle->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_configure_resources;
    }

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->inputPortFormat);

    /* Check for DSP mixer downstream to compare buffer requirements */
    BAPE_PathNode_P_FindConsumersBySubtype(&handle->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);

    if (numFound != 0 && (handle->inputPortFormat.type != BAPE_DataType_ePcmMono && handle->inputPortFormat.type != BAPE_DataType_ePcmStereo) ) {
        BDBG_WRN(("Unable to start Input Capture due to DSP Mixer attached and input format %s", BAPE_FMT_P_GetTypeName_isrsafe(&handle->inputPortFormat)));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_format_mismatch;
    }

    /* capture to memory disables capture to FMM outputs */
    if ( captureToMemory )
    {
        /* Make sure we have enough buffers to satisfy this request */
        numBuffersRequired = numChannelPairs;
        if ( handle->settings.interleaveData )
        {
            step = 2;
        }
        else
        {
            numBuffersRequired *= 2;
            step = 1;
        }
        if ( numBuffersRequired > handle->numBuffers )
        {
            BDBG_ERR(("To support %u channels %s requires %u buffers but only %u are allocated for OutputCapture %u",
                      numChannelPairs, (handle->settings.interleaveData)?"interleaved":"non-interleaved",
                      numBuffersRequired, handle->numBuffers, handle->index));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    if ( dfifoRequired )
    {
        BAPE_DfifoGroup_P_GetDefaultCreateSettings(&dfifoCreateSettings);
        numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->inputPortFormat);
        BDBG_MSG(("Input requires %u channel pairs", numChannelPairs));
        dfifoCreateSettings.numChannelPairs = numChannelPairs;
        errCode = BAPE_DfifoGroup_P_Create(handle->deviceHandle, &dfifoCreateSettings, &handle->dfifoGroup);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_dfifo_alloc;
        }
        /* Configure DFIFOs after all outputs are configured so we know which is the master */
        BAPE_DfifoGroup_P_GetSettings(handle->dfifoGroup, &dfifoSettings);

        dfifoSettings.highPriority = (handle->inputPortFormat.sampleRate >= 96000)?true:false;
        if ( !captureToMemory )
        {
            dfifoSettings.linkedSfifo = handle->pMasterConnection->sfifoGroup;
        }

        if ( captureToMemory ) /* capture to memory */
        {
            BDBG_MSG(("Setting up Dfifo to capture to memory"));
            dfifoSettings.interleaveData = handle->settings.interleaveData;
            dfifoSettings.dataWidth = handle->settings.bitsPerSample;
            for ( i = 0; i < numBuffersRequired; i++ )
            {
                unsigned dfifoBufIndex = i*step;
                dfifoSettings.bufferInfo[dfifoBufIndex].base = handle->bufferOffset[i];
                dfifoSettings.bufferInfo[dfifoBufIndex].length = handle->bufferSize;
                dfifoSettings.bufferInfo[dfifoBufIndex].watermark = handle->settings.watermark;
            }
        }
        else if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->inputPortFormat) ) /* capture PCM to FMM */
        {
            dfifoSettings.dataWidth = 32;
            dfifoSettings.interleaveData = false;
            for ( i = 0; i < numChannelPairs; i++ )
            {
                uint64_t length;
                unsigned bufferNum = i*2;
                pBuffer = handle->node.connectors[0].pBuffers[i];
                length = pBuffer->bufferSize/2;
                dfifoSettings.bufferInfo[bufferNum].base = pBuffer->offset;
                dfifoSettings.bufferInfo[bufferNum+1].base = pBuffer->offset + length;
                dfifoSettings.bufferInfo[bufferNum].length = length;
                dfifoSettings.bufferInfo[bufferNum+1].length = length;
            }
        }
        else /* capture compressed to FMM */
        {
            dfifoSettings.dataWidth = 16;
            dfifoSettings.interleaveData = true;
            pBuffer = handle->node.connectors[0].pBuffers[0];
            dfifoSettings.bufferInfo[0].block = pBuffer->block;
            dfifoSettings.bufferInfo[0].pBuffer = pBuffer->pMemory;
            dfifoSettings.bufferInfo[0].base = pBuffer->offset;
            dfifoSettings.bufferInfo[0].length = pBuffer->bufferSize;
        }
        handle->interleave = dfifoSettings.interleaveData;
        BAPE_InputPort_P_GetFciIds(pSettings->input, &dfifoSettings.input);
        errCode = BAPE_DfifoGroup_P_SetSettings(handle->dfifoGroup, &dfifoSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_apply_dfifo_settings;
        }
    }

    if ( NULL == handle->pMasterConnection && handle->bufferSize == 0 )
    {
        BDBG_ERR(("No outputs have been connected to this input capture."));
        goto err_no_master;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(handle->deviceHandle);
    #endif

    BDBG_ERR(("InputCapture %p (%d) Start Downstream", (void *)handle, handle->index));
    /* Start */
    errCode = BAPE_PathNode_P_StartPaths(&handle->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start_paths;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&handle->node);
    #endif

    /* Enable Interrupts */
    BAPE_InputCapture_P_EnableInterrupts(handle);

    if ( dfifoRequired )
    {
        BDBG_MSG(("InputCapture %p (%d) Start Dfifo", (void *)handle, handle->index));
        /* Start DFIFO */
        errCode = BAPE_DfifoGroup_P_Start(handle->dfifoGroup, false);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_start_dfifo;
        }
    }

    BDBG_MSG(("InputCapture %p (%d) Start Input", (void *)handle, handle->index));
    /* Start Input via callback */
    BDBG_ASSERT(NULL != pSettings->input->enable);
    pSettings->input->enable(pSettings->input);

    handle->running = true;
    handle->halted = false;
    return BERR_SUCCESS;

    err_start_dfifo:
        BAPE_PathNode_P_StopPaths(&handle->node);
    err_start_paths:
    err_apply_dfifo_settings:
    err_no_master:
        if (handle->dfifoGroup) {
            /* Release resources */
            BAPE_DfifoGroup_P_Destroy(handle->dfifoGroup);
            handle->dfifoGroup = NULL;
        }
    err_dfifo_alloc:
    err_configure_resources:
    err_format_mismatch:
        BAPE_PathNode_P_ReleasePathResources(&handle->node);
    err_acquire_resources:
    err_format:
        {
            BAPE_PathConnection *pConnection;
            for ( pConnection = BLST_SQ_FIRST(&handle->node.connectors[0].connectionList);
                  NULL != pConnection;
                  pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
            {
                BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
                BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);
                BAPE_InputCapture_P_FreePathFromInput(&handle->node, pConnection);
            }
        }
    err_allocate_path:
        (void)BAPE_InputPort_P_DetachConsumer(pSettings->input, &handle->node);
    err_attach:
        return errCode;
}

/***************************************************************************
Summary:
Stop InputCapture
***************************************************************************/
void BAPE_InputCapture_Stop(
    BAPE_InputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    if ( !handle->running )
    {
        BDBG_WRN(("Input Capture %u already stopped", handle->index));
        return;
    }

    /* Stop input */
    BDBG_ASSERT(NULL != handle->startSettings.input);
    BDBG_ASSERT(NULL != handle->startSettings.input->disable);
    handle->startSettings.input->disable(handle->startSettings.input);

    /* Stop DFIFOs */
    if ( handle->dfifoGroup )
    {
        BAPE_DfifoGroup_P_Stop(handle->dfifoGroup);
    }

    /* Disable Interrupts */
    BAPE_InputCapture_P_DisableInterrupts(handle);

    if ( handle->fciSpOutput )
    {
        BAPE_FciSplitterOutputGroup_P_Destroy(handle->fciSpOutput);
        handle->fciSpOutput = NULL;
    }

    BDBG_MSG(("Stop Paths"));
    /* Stop Paths Downstream */
    BAPE_PathNode_P_StopPaths(&handle->node);

    BDBG_MSG(("Detach Consumer from Input"));
    (void)BAPE_InputPort_P_DetachConsumer(handle->startSettings.input, &handle->node);

    /* Release resources */
    if ( handle->dfifoGroup )
    {
        BAPE_DfifoGroup_P_Destroy(handle->dfifoGroup);
        handle->dfifoGroup = NULL;
    }

    if ( !handle->sfifoForFmmOutputs )
    {
        BAPE_PathConnection *pConnection;
        for ( pConnection = BLST_SQ_FIRST(&handle->node.connectors[0].connectionList);
              NULL != pConnection;
              pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
        {
            BERR_Code errCode;
            BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
            BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);
            errCode = BAPE_InputCapture_P_FreePathFromInput(&handle->node, pConnection);
            if ( errCode )
            {
                BDBG_ERR(("Unable to free path from input"));
            }
        }
    }

    handle->pMasterConnection = NULL;
    handle->running = false;
}

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_InputCapture_GetConnector(
    BAPE_InputCaptureHandle handle,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}

/***************************************************************************
Summary:
Get InputCapture status
***************************************************************************/
void BAPE_InputCapture_GetStatus(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureStatus *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_InputCaptureStatus));

    if (  handle->running )
    {
        pStatus->running = true;
        pStatus->halted = handle->halted || handle->startSettings.input->halted;
    }
}

static BERR_Code BAPE_InputCapture_P_AllocatePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BERR_Code errCode;
    BAPE_InputCaptureHandle handle;
    BAPE_PathNode *pSink;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    pSink = pConnection->pSink;

    BDBG_MSG(("%s", __FUNCTION__));
    if ( NULL == handle->pMasterConnection )
    {
        handle->pMasterConnection = pConnection;
        if ( handle->sfifoForFmmOutputs )
        {
            if ( (pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer) )
            {
                BDBG_MSG(("  sfifo required"));
                handle->sfifoRequired = true;
            }
        }
        else
        {
            if ( handle->startSettings.input )
            {
                /* using FCI Splitter */
                if ( handle->startSettings.input->fciSpGroup )
                {
                    BAPE_FciSplitterGroupCreateSettings fciSpCreateSettings;
                    BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(&fciSpCreateSettings);
                    if ( handle->fciSpOutput )
                    {
                        BDBG_MSG(("Freeing FCI Splitter output group"));
                        BAPE_FciSplitterOutputGroup_P_Destroy(handle->fciSpOutput);
                        handle->fciSpOutput = NULL;
                    }
                    fciSpCreateSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->startSettings.input->format);
                    errCode = BAPE_FciSplitterOutputGroup_P_Create(handle->startSettings.input->fciSpGroup, &fciSpCreateSettings, &handle->fciSpOutput);
                    if ( errCode != BERR_SUCCESS )
                    {
                        BDBG_ERR(("Unable to allocate FCI Splitter Output Group, numChannelPairs %lu", (unsigned long)fciSpCreateSettings.numChannelPairs));
                        return BERR_TRACE(errCode);
                    }
                    BAPE_FciSplitterOutputGroup_P_GetOutputFciIds(handle->fciSpOutput, &handle->pMasterConnection->inputFciGroup);
                }
                else /* direct connection */
                {
                    BAPE_InputPort_P_GetFciIds(handle->startSettings.input, &handle->pMasterConnection->inputFciGroup);
                }
                #if BDBG_DEBUG_BUILD
                {
                    unsigned i;
                    for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->startSettings.input->format); i++ )
                    {
                        BDBG_MSG(("Attaching FCI input - fci id[%lu] %lx", (unsigned long)i, (unsigned long)handle->pMasterConnection->inputFciGroup.ids[i]));
                    }
                }
                #endif
            }
            else
            {
                BDBG_ERR(("no input port specified!!"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
    }
    else
    {
        if ( handle->fciSpOutput )
        {
            BDBG_ERR(("Multiple outputs from an FCI Splitting Input Capture are not currently supported. Use a separate input capture for each downstream path on this chip."));
            BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        /* anything to do for dfifo -> sfifo case? */
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_InputCapture_P_FreePathFromInput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_InputCaptureHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    BDBG_MSG(("%s", __FUNCTION__));
    if ( NULL != handle->pMasterConnection )
    {
        unsigned i;
        for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->format); i++ )
        {
            BDBG_MSG(("Clearing FCI input - fci id[%lu] %lx -> 3ff", (unsigned long)i, (unsigned long)handle->pMasterConnection->inputFciGroup.ids[i]));
            handle->pMasterConnection->inputFciGroup.ids[i] = 0x3ff;
        }

        handle->pMasterConnection = NULL;
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_InputCapture_P_ConfigPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_PathConnector *pSource;
    BAPE_InputCaptureHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    pSource = pConnection->pSource;

    BDBG_MSG(("%s", __FUNCTION__));

    if ( handle->sfifoRequired )
    {
        BAPE_SfifoGroupSettings sfifoSettings;
        unsigned numChannelPairs;
        unsigned i;

        BDBG_MSG(("  SFIFO required"));
        if ( NULL == handle->pMasterConnection )
        {
            handle->pMasterConnection = pConnection;
        }
        handle->sfifoRequired = true;

        BAPE_SfifoGroup_P_GetSettings(pConnection->sfifoGroup, &sfifoSettings);
        sfifoSettings.highPriority = (handle->inputPortFormat.sampleRate >= 96000)?true:false;
        sfifoSettings.signedData = true;
        sfifoSettings.stereoData = true;

        if ( pConnection != handle->pMasterConnection )
        {
            BDBG_MSG(("Linking playback source channel group %p to master %p", (void *)pConnection->sfifoGroup, (void *)handle->pMasterConnection->sfifoGroup));
            sfifoSettings.master = handle->pMasterConnection->sfifoGroup;
        }
        else
        {
            sfifoSettings.master = NULL;
        }

        numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->inputPortFormat);

        if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->inputPortFormat) )
        {
            sfifoSettings.interleaveData = false;
            sfifoSettings.dataWidth = 32;
            sfifoSettings.sampleRepeatEnabled = true;
            /* Setup buffers from pool */
            for ( i = 0; i < numChannelPairs; i++ )
            {
                unsigned bufferId = 2*i;
                BAPE_BufferNode *pBuffer = pSource->pBuffers[i];
                BDBG_ASSERT(NULL != pBuffer);
                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset;
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize/2;
                sfifoSettings.bufferInfo[bufferId].wrpoint = sfifoSettings.bufferInfo[bufferId].base+(pBuffer->bufferSize/2)-1;
                bufferId++;
                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset+(pBuffer->bufferSize/2);
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize/2;
                sfifoSettings.bufferInfo[bufferId].wrpoint = sfifoSettings.bufferInfo[bufferId].base+(pBuffer->bufferSize/2)-1;
            }
        }
        else
        {
            sfifoSettings.interleaveData = true;
            sfifoSettings.dataWidth = 16;
            sfifoSettings.sampleRepeatEnabled = false;
            /* Setup buffers from pool */
            for ( i = 0; i < numChannelPairs; i++ )
            {
                unsigned bufferId = 2*i;
                BAPE_BufferNode *pBuffer = pSource->pBuffers[i];
                BDBG_ASSERT(NULL != pBuffer);
                sfifoSettings.bufferInfo[bufferId].block = pBuffer->block;
                sfifoSettings.bufferInfo[bufferId].pBuffer = pBuffer->pMemory;
                sfifoSettings.bufferInfo[bufferId].base = pBuffer->offset;
                sfifoSettings.bufferInfo[bufferId].length = pBuffer->bufferSize;
                sfifoSettings.bufferInfo[bufferId].wrpoint = pBuffer->offset+pBuffer->bufferSize-1;
                bufferId++;
                sfifoSettings.bufferInfo[bufferId].block = NULL;
                sfifoSettings.bufferInfo[bufferId].pBuffer = NULL;
                sfifoSettings.bufferInfo[bufferId].base = 0;
                sfifoSettings.bufferInfo[bufferId].length = 0;
                sfifoSettings.bufferInfo[bufferId].wrpoint = 0;
            }
        }
        BAPE_SfifoGroup_P_SetSettings(pConnection->sfifoGroup, &sfifoSettings);
    }
    /* Other nodes don't require anything done here */

    return BERR_SUCCESS;
}

static BERR_Code BAPE_InputCapture_P_StartPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_PathNode *pSink;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    pSink = pConnection->pSink;
    BDBG_MSG(("StartPathToOutput - sink %p, type %d", (void *)pSink, pSink->type));

    if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
    {
        /* FW Mixer is a special case. Nothing to do */
    }
    else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
    {
        BERR_Code errCode;

        if ( pConnection->sfifoGroup )
        {
            /* TODO: Sure would be nice to use PLAY_RUN instead of WRPOINT */
            errCode = BAPE_SfifoGroup_P_Start(pConnection->sfifoGroup, false);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
            if ( pSink->type == BAPE_PathNodeType_eMixer )
            {
                BAPE_StandardMixer_P_SfifoStarted(pSink->pHandle, pConnection);
            }
        }
    }

    return BERR_SUCCESS;
}

static void BAPE_InputCapture_P_StopPathToOutput(
    BAPE_PathNode *pNode,
    BAPE_PathConnection *pConnection
    )
{
    BAPE_PathNode *pSink;

    BAPE_InputCaptureHandle handle = pNode->pHandle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    /* Stop slaves first */
    for ( pConnection = BLST_SQ_FIRST(&pNode->connectors[0].connectionList);
        NULL != pConnection;
        pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        if ( pConnection != handle->pMasterConnection )
        {
            if ( pConnection->sfifoGroup )
            {
                BAPE_SfifoGroup_P_Stop(pConnection->sfifoGroup);
            }
        }
    }

    if ( handle->pMasterConnection )
    {
        pSink = handle->pMasterConnection->pSink;

        if ( pSink->type == BAPE_PathNodeType_eMixer && pSink->subtype == BAPE_MixerType_eDsp )
        {
            /* FW Mixer is a special case. Nothing to do */
        }
        else if ( pSink->type == BAPE_PathNodeType_eMixer || pSink->type == BAPE_PathNodeType_eEqualizer )
        {
            if ( handle->pMasterConnection->sfifoGroup )
            {
                BAPE_SfifoGroup_P_Stop(handle->pMasterConnection->sfifoGroup);
            }
        }
    }

    if ( handle->fciSpOutput )
    {
        BAPE_FciSplitterOutputGroup_P_Destroy(handle->fciSpOutput);
        handle->fciSpOutput = NULL;
    }
}

static BERR_Code BAPE_InputCapture_P_InputFormatChange_isr(
    BAPE_PathNode *pNode,
    BAPE_InputPort inputPort
    )
{
    BAPE_InputCaptureHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BKNI_ASSERT_ISR_CONTEXT();
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    BDBG_MSG(("%s", __FUNCTION__));

    /* On the fly format changes are not possible */
    if ( inputPort->format.type != handle->inputPortFormat.type )
    {
        BDBG_MSG(("Input data format has changed (%s->%s).  Halting capture.", BAPE_FMT_P_GetTypeName_isrsafe(&handle->inputPortFormat), BAPE_FMT_P_GetTypeName_isrsafe(&inputPort->format)));
        if ( handle->interrupts.inputHalted.pCallback_isr )
        {
            handle->interrupts.inputHalted.pCallback_isr(handle->interrupts.inputHalted.pParam1, handle->interrupts.inputHalted.param2);
        }
        /* Intentionally omitted BERR_TRACE */
        handle->halted = true;
        return BERR_NOT_SUPPORTED;
    }

    if ( inputPort->format.sampleRate != handle->inputPortFormat.sampleRate )
    {
        /* Sample rate has changed.  Propagate this. -- TODO if HQ SRC is required this can't be done - also may introduce buffering issues. */
        handle->inputPortFormat.sampleRate = inputPort->format.sampleRate;
        BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[0], handle->inputPortFormat.sampleRate);
        if ( handle->interrupts.sampleRate.pCallback_isr )
        {
            handle->interrupts.sampleRate.pCallback_isr(handle->interrupts.sampleRate.pParam1, handle->interrupts.sampleRate.param2, handle->inputPortFormat.sampleRate);
        }
    }
    return BERR_SUCCESS;
}

BERR_Code BAPE_InputCapture_GetBuffer(
    BAPE_InputCaptureHandle handle,
    BAPE_BufferDescriptor *pBuffers         /* [out] */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != pBuffers);

    BKNI_Memset(pBuffers, 0, sizeof(*pBuffers));
    pBuffers->interleaved = handle->interleave;
    if ( !handle->running )
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(NULL != handle->dfifoGroup);
    errCode = BAPE_DfifoGroup_P_GetBuffer(handle->dfifoGroup, pBuffers);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_InputCapture_ConsumeData(
    BAPE_InputCaptureHandle handle,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    if ( !handle->running )
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(NULL != handle->dfifoGroup);
    BKNI_EnterCriticalSection();
    errCode = BAPE_DfifoGroup_P_CommitData_isr(handle->dfifoGroup, numBytes);
    BAPE_InputCapture_P_ClearInterrupts_isr(handle);
    BKNI_LeaveCriticalSection();

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_InputCapture_GetInterruptHandlers(
    BAPE_InputCaptureHandle handle,
    BAPE_InputCaptureInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != handle);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_InputCapture_SetInterruptHandlers(
    BAPE_InputCaptureHandle handle,
    const BAPE_InputCaptureInterruptHandlers *pInterrupts
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != handle);
    BKNI_EnterCriticalSection();
    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    if ( handle->running )
    {
        errCode = BAPE_InputCapture_P_EnableInterrupts(handle);
        if ( errCode != BERR_SUCCESS )
        {
            return BERR_TRACE(errCode);
        }
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_InputCapture_P_EnableInterrupts(
    BAPE_InputCaptureHandle handle
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != handle);

    /* Install interrupt handlers */
    if ( handle->dfifoGroup )
    {
        if ( handle->interrupts.watermark.pCallback_isr )
        {
            BDBG_MSG(("Register Watermark Interrupt"));
            errCode = BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup,
                                                             handle->interrupts.watermark.pCallback_isr,
                                                             handle->interrupts.watermark.pParam1,
                                                             handle->interrupts.watermark.param2);
            if ( errCode )
            {
                BERR_TRACE(errCode);
            }
        }
        if ( handle->interrupts.overflow.pCallback_isr )
        {
            BDBG_MSG(("Register Overflow Interrupt"));
            errCode = BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup,
                                                             handle->interrupts.overflow.pCallback_isr,
                                                             handle->interrupts.overflow.pParam1,
                                                             handle->interrupts.overflow.param2);
            if ( errCode )
            {
                BERR_TRACE(errCode);
            }
        }
    }
    return BERR_SUCCESS;
}

static void BAPE_InputCapture_P_DisableInterrupts(
    BAPE_InputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);
    BDBG_ASSERT(NULL != handle);

    /* Clear interrupts */
    BDBG_MSG(("Disable Interrupts"));
    if ( handle->dfifoGroup )
    {
        BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup, NULL, NULL, 0);
        BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup, NULL, NULL, 0);
    }
}

static void BAPE_InputCapture_P_ClearInterrupts_isr(BAPE_InputCaptureHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_InputCapture);

    if ( handle->running )
    {
        BDBG_MSG(("Clear Interrupts"));
        if ( handle->dfifoGroup )
        {
            BAPE_DfifoGroup_P_RearmFullmarkInterrupt_isr(handle->dfifoGroup);
            BAPE_DfifoGroup_P_RearmOverflowInterrupt_isr(handle->dfifoGroup);
        }
    }
}
