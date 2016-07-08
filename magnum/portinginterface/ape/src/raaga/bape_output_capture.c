/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_output_capture);

BDBG_OBJECT_ID(BAPE_OutputCapture);

typedef struct BAPE_OutputCapture
{
    BDBG_OBJECT(BAPE_OutputCapture)
    BAPE_Handle deviceHandle;
    BMEM_Heap_Handle hHeap;
    unsigned bufferSize;
    BAPE_OutputCaptureSettings settings;
    BAPE_OutputCaptureInterruptHandlers interrupts;
    unsigned index;
    BAPE_OutputPortObject outputPort;
    unsigned sampleRate;
    unsigned numBuffers;
    void *pBuffers[BAPE_Channel_eMax];
    uint32_t bufferOffset[BAPE_Channel_eMax];
    BAPE_DfifoGroupHandle dfifoGroup;
    BAPE_LoopbackGroupHandle loopbackGroup;
#if BAPE_CHIP_MAX_FS > 0
    unsigned fs;
#else
    BAPE_MclkSource mclkSource;
    unsigned pllChannel;
    unsigned mclkFreqToFsRatio;
#endif
    bool enabled;
    char name[7];   /* CAP %d */
} BAPE_OutputCapture;

static void BAPE_OutputCapture_P_ClearInterrupts_isr(BAPE_OutputCaptureHandle handle);

/* Connector callbacks */
static void BAPE_OutputCapture_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
static BERR_Code BAPE_OutputCapture_P_Enable(BAPE_OutputPort output);
static void BAPE_OutputCapture_P_Disable(BAPE_OutputPort output);
#if BAPE_CHIP_MAX_FS > 0
static void BAPE_OutputCapture_P_SetFs(BAPE_OutputPort output, unsigned fsNum);
#else
static void BAPE_OutputCapture_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
#endif

void BAPE_OutputCapture_GetDefaultOpenSettings(
    BAPE_OutputCaptureOpenSettings *pSettings       /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if BAPE_DSP_SUPPORT
    pSettings->bufferSize = BDSP_AF_P_NON_DELAY_RBUF_SIZE*2;
#else
    pSettings->bufferSize = 128*1024;
#endif
    pSettings->numBuffers = 1;
    pSettings->watermarkThreshold = pSettings->bufferSize / 2;
    pSettings->alignment = 8;
}

BERR_Code BAPE_OutputCapture_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_OutputCaptureOpenSettings *pSettings,
    BAPE_OutputCaptureHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_OutputCaptureHandle handle;
    BAPE_OutputCaptureOpenSettings defaultSettings;
    void *pMem, *pCachedMem;
    unsigned bufferSize=0, watermark=0;
    unsigned maxChannelPairs, numBuffers=1;
    unsigned i;
    unsigned alignment=8;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( index >= BAPE_CHIP_MAX_OUTPUT_CAPTURES )
    {
        BDBG_ERR(("Unable to open output capture %u.  Max output captures is %u on this platform.", index, BAPE_CHIP_MAX_OUTPUT_CAPTURES));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    if ( NULL == pSettings )
    {
        BAPE_OutputCapture_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* Determine buffer size */
    if ( NULL != pSettings )
    {
        bufferSize = pSettings->bufferSize;
        watermark = pSettings->watermarkThreshold;
        alignment = pSettings->alignment;
        if ( pSettings->numBuffers > BAPE_Channel_eMax )
        {
            BDBG_WRN(("Requested %u buffers but only %u channels are possible.  Allocating %u buffers.",
                      pSettings->numBuffers, BAPE_Channel_eMax, BAPE_Channel_eMax));
            numBuffers = BAPE_Channel_eMax;
        }
        else
        {
            numBuffers = pSettings->numBuffers;
        }
    }
    if ( 0 == bufferSize )
    {
        bufferSize = deviceHandle->buffers[0].bufferSize;
    }
    if ( 0 == watermark )
    {
        watermark = bufferSize/4;
    }

    if ( bufferSize % pSettings->alignment != 0 )
    {
        BDBG_ERR(("Requested bufferSize(%lu) is not compatiable with alignment (%lu)", (unsigned long)bufferSize, (unsigned long)alignment));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Allocate handle */
    handle = BKNI_Malloc(sizeof(BAPE_OutputCapture));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_OutputCapture));

    BDBG_OBJECT_SET(handle, BAPE_OutputCapture);
    handle->deviceHandle = deviceHandle;
    handle->hHeap = pSettings->heap ? pSettings->heap : deviceHandle->memHandle;
    handle->bufferSize = bufferSize;
    handle->settings.watermark = watermark;
    handle->settings.bitsPerSample = 32;
    handle->settings.interleaveData = true;
    handle->index = index;
    BAPE_P_InitOutputPort(&handle->outputPort, BAPE_OutputPortType_eOutputCapture, index, handle);
    maxChannelPairs = BAPE_CHIP_MAX_LOOPBACKS;
    if ( maxChannelPairs > BAPE_CHIP_MAX_DFIFOS )
    {
        maxChannelPairs = BAPE_CHIP_MAX_DFIFOS;
    }
    if ( maxChannelPairs > pSettings->numBuffers )
    {
        maxChannelPairs = pSettings->numBuffers;
    }
    if ( maxChannelPairs > BAPE_ChannelPair_eMax )
    {
        maxChannelPairs = BAPE_ChannelPair_eMax;
    }
    BAPE_FMT_P_EnableSource(&handle->outputPort.capabilities, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_eIec61937x4);
    switch ( maxChannelPairs )
    {
    case 4:
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm7_1);
        /* Fall through */
    case 3:
        BAPE_FMT_P_EnableType(&handle->outputPort.capabilities, BAPE_DataType_ePcm5_1);
        break;
    default:
        break;
    }
    handle->outputPort.enable = BAPE_OutputCapture_P_Enable;
    handle->outputPort.disable = BAPE_OutputCapture_P_Disable;
    handle->outputPort.setTimingParams_isr = BAPE_OutputCapture_P_SetTimingParams_isr;
    handle->sampleRate = 0;
    handle->outputPort.muteInMixer = true;
#if BAPE_CHIP_MAX_FS > 0
    handle->outputPort.fsTiming = true;
    handle->outputPort.setFs = BAPE_OutputCapture_P_SetFs;
    handle->fs = (unsigned)-1;
#else
    handle->outputPort.setMclk_isr = BAPE_OutputCapture_P_SetMclk_isr;
    handle->mclkSource = BAPE_MclkSource_eNone;
    handle->pllChannel = 0;
    handle->mclkFreqToFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;
#endif
    BKNI_Snprintf(handle->name, sizeof(handle->name), "CAP %u", index);
    handle->outputPort.pName = handle->name;

    handle->numBuffers = numBuffers;
    for ( i = 0; i < numBuffers; i++ )
    {
        /* Allocate buffer */
        handle->pBuffers[i] = pMem = BMEM_Heap_AllocAligned(handle->hHeap, bufferSize, alignment, 0);
        if ( NULL == pMem )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_buffer;
        }
    
        errCode = BMEM_Heap_ConvertAddressToOffset(handle->hHeap, pMem, &handle->bufferOffset[i]);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_offset;
        }
    
        errCode = BMEM_Heap_ConvertAddressToCached(handle->hHeap, pMem, &pCachedMem);
        if ( BERR_SUCCESS == errCode )
        {
            /* Flush once at open to make sure the buffer has been invalidated from the cache. */
            BMEM_Heap_FlushCache(handle->hHeap, pCachedMem, bufferSize);
        }
        /* Not fatal if it fails, assume no cache support */
    }

    handle->deviceHandle->outputCaptures[index] = handle;

    *pHandle = handle;
    return BERR_SUCCESS;

err_offset:
err_buffer:
    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        if ( handle->pBuffers[i] )
        {
            BMEM_Heap_Free(handle->hHeap, handle->pBuffers[i]);
        }
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_OutputCapture);
    BKNI_Free(handle);
    return errCode;
}

void BAPE_OutputCapture_Close(
    BAPE_OutputCaptureHandle handle
    )
{
    unsigned i;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(handle->enabled == false);
    BDBG_ASSERT(handle->outputPort.mixer == NULL);
    BDBG_ASSERT(NULL == handle->dfifoGroup);
    BDBG_ASSERT(NULL == handle->loopbackGroup);
    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        if ( handle->pBuffers[i] )
        {
            BMEM_Heap_Free(handle->hHeap, handle->pBuffers[i]);
        }
    }
    handle->deviceHandle->outputCaptures[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_OutputCapture);
    BKNI_Free(handle);
}

void BAPE_OutputCapture_GetSettings(
    BAPE_OutputCaptureHandle handle,
    BAPE_OutputCaptureSettings *pSettings       /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_OutputCapture_SetSettings(
    BAPE_OutputCaptureHandle handle,
    const BAPE_OutputCaptureSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
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

void BAPE_OutputCapture_Flush(
    BAPE_OutputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BKNI_EnterCriticalSection();
    BAPE_OutputCapture_Flush_isr(handle);
    BKNI_LeaveCriticalSection();
}

void BAPE_OutputCapture_Flush_isr(
    BAPE_OutputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    
    if ( handle->dfifoGroup )
    {
        BAPE_DfifoGroup_P_Flush_isr(handle->dfifoGroup);
        BAPE_OutputCapture_P_ClearInterrupts_isr(handle);
    }
}

BERR_Code BAPE_OutputCapture_GetBuffer(
    BAPE_OutputCaptureHandle handle,
    BAPE_BufferDescriptor *pBuffers         /* [out] */
    )
{   
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pBuffers);

    BKNI_Memset(pBuffers, 0, sizeof(*pBuffers));
    pBuffers->interleaved = handle->settings.interleaveData;
    if ( !handle->enabled )
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

BERR_Code BAPE_OutputCapture_ConsumeData(
    BAPE_OutputCaptureHandle handle,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( !handle->enabled )
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(NULL != handle->dfifoGroup);
    BKNI_EnterCriticalSection();
    errCode = BAPE_DfifoGroup_P_CommitData_isr(handle->dfifoGroup, numBytes);
    BAPE_OutputCapture_P_ClearInterrupts_isr(handle);
    BKNI_LeaveCriticalSection();

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_OutputCapture_GetOutputPort(
    BAPE_OutputCaptureHandle handle,
    BAPE_OutputPort *pOutputPort
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pOutputPort);
    *pOutputPort = &handle->outputPort;
}

void BAPE_OutputCapture_GetInterruptHandlers(
    BAPE_OutputCaptureHandle handle,
    BAPE_OutputCaptureInterruptHandlers *pInterrupts    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_OutputCapture_SetInterruptHandlers(
    BAPE_OutputCaptureHandle handle,
    const BAPE_OutputCaptureInterruptHandlers *pInterrupts
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pInterrupts);

    if ( handle->enabled )
    {
        BDBG_ASSERT(NULL != handle->dfifoGroup);
        /* Install interrupt handlers */
        errCode = BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup,
                                                         pInterrupts->watermark.pCallback_isr,
                                                         pInterrupts->watermark.pParam1,
                                                         pInterrupts->watermark.param2);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        errCode = BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup,
                                                         pInterrupts->overflow.pCallback_isr,
                                                         pInterrupts->overflow.pParam1,
                                                         pInterrupts->overflow.param2);
        if ( errCode )
        {
            BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup, NULL, NULL, 0);
            return BERR_TRACE(errCode);
        }
    }

    handle->interrupts = *pInterrupts;
    return BERR_SUCCESS;
}

static BERR_Code BAPE_OutputCapture_P_Enable(BAPE_OutputPort output)
{
    BERR_Code errCode;
    BAPE_DfifoGroupSettings dfifoSettings;
    BAPE_LoopbackGroupSettings loopbackSettings;
    BAPE_OutputCaptureHandle handle;
    BAPE_LoopbackGroupCreateSettings loopbackCreateSettings;
    BAPE_DfifoGroupCreateSettings dfifoCreateSettings;
    unsigned numBuffersRequired, i, step, numChannelPairs;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    BDBG_ASSERT(false == handle->enabled);

    /* Make sure we have enough buffers to satisfy this request */
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(BAPE_Mixer_P_GetOutputFormat(output->mixer));
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

    /* Allocate needed loopback resources */
    BAPE_LoopbackGroup_P_GetDefaultCreateSettings(&loopbackCreateSettings);
    loopbackCreateSettings.numChannelPairs = numChannelPairs;
    errCode = BAPE_LoopbackGroup_P_Create(handle->deviceHandle, &loopbackCreateSettings, &handle->loopbackGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_loopback_alloc;
    }

    /* Allocate needed DFIFO resources */
    BAPE_DfifoGroup_P_GetDefaultCreateSettings(&dfifoCreateSettings);
    dfifoCreateSettings.numChannelPairs = numChannelPairs;
    errCode = BAPE_DfifoGroup_P_Create(handle->deviceHandle, &dfifoCreateSettings, &handle->dfifoGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dfifo_alloc;
    }

    /* Setup and enable the DFIFO first, then Loopback */
    BAPE_DfifoGroup_P_GetSettings(handle->dfifoGroup, &dfifoSettings);
    dfifoSettings.highPriority = (BAPE_Mixer_P_GetOutputSampleRate(output->mixer)) >= 96000 ? true : false;
    BAPE_LoopbackGroup_P_GetCaptureFciIds(handle->loopbackGroup, &dfifoSettings.input);
    dfifoSettings.interleaveData = handle->settings.interleaveData;
    dfifoSettings.dataWidth = handle->settings.bitsPerSample;
    for ( i = 0; i < numBuffersRequired; i++ )
    {
        unsigned dfifoBufIndex = i*step;
        dfifoSettings.bufferInfo[dfifoBufIndex].base = handle->bufferOffset[i];
        dfifoSettings.bufferInfo[dfifoBufIndex].length = handle->bufferSize;
        dfifoSettings.bufferInfo[dfifoBufIndex].watermark = handle->settings.watermark;
    }
    errCode = BAPE_DfifoGroup_P_SetSettings(handle->dfifoGroup, &dfifoSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dfifo_settings;
    }

    /* Configure Loopback */
    BKNI_EnterCriticalSection();
    BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
#if BAPE_CHIP_MAX_FS > 0
    loopbackSettings.fs = handle->fs;
#else
    loopbackSettings.mclkSource = handle->mclkSource;
    loopbackSettings.pllChannel = handle->pllChannel;
    loopbackSettings.mclkFreqToFsRatio = handle->mclkFreqToFsRatio;
#endif
    loopbackSettings.input = handle->outputPort.sourceMixerFci;
    loopbackSettings.resolution = handle->settings.bitsPerSample > 24 ? 24 : handle->settings.bitsPerSample;
    errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_loopback_settings;
    }

    /* Install interrupt handlers */
    if ( handle->interrupts.watermark.pCallback_isr )
    {
        errCode = BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup,
                                                         handle->interrupts.watermark.pCallback_isr,
                                                         handle->interrupts.watermark.pParam1,
                                                         handle->interrupts.watermark.param2);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_fullmark;
        }
    }
    if ( handle->interrupts.overflow.pCallback_isr )
    {
        errCode = BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup,
                                                         handle->interrupts.overflow.pCallback_isr,
                                                         handle->interrupts.overflow.pParam1,
                                                         handle->interrupts.overflow.param2);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_overflow;
        }
    }

    /* Enable DFIFO */
    errCode = BAPE_DfifoGroup_P_Start(handle->dfifoGroup, false);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_dfifo_start;
    }

    /* Enable Loopback */
    errCode = BAPE_LoopbackGroup_P_Start(handle->loopbackGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_loopback_start;
    }

    /* Done */
    handle->enabled = true;
    return BERR_SUCCESS;

err_loopback_start:
    BAPE_DfifoGroup_P_Stop(handle->dfifoGroup);
err_dfifo_start:
    BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup, NULL, NULL, 0);
err_overflow:
    BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup, NULL, NULL, 0);
err_fullmark:
err_loopback_settings:
err_dfifo_settings:
    BAPE_DfifoGroup_P_Destroy(handle->dfifoGroup);
    handle->dfifoGroup = NULL;
err_dfifo_alloc:
    BAPE_LoopbackGroup_P_Destroy(handle->loopbackGroup);
    handle->loopbackGroup = NULL;
err_loopback_alloc:
    return errCode;
}

static void BAPE_OutputCapture_P_Disable(BAPE_OutputPort output)
{
    BAPE_OutputCaptureHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( handle->loopbackGroup )
    {
        /* Disable Loopback */
        BAPE_LoopbackGroup_P_Stop(handle->loopbackGroup);

        if ( handle->dfifoGroup )
        {
            /* Disable DFIFO */
            BAPE_DfifoGroup_P_Stop(handle->dfifoGroup);

            /* Clear interrupts */
            BAPE_DfifoGroup_P_SetFullmarkInterrupt(handle->dfifoGroup, NULL, NULL, 0);
            BAPE_DfifoGroup_P_SetOverflowInterrupt(handle->dfifoGroup, NULL, NULL, 0);

            BAPE_DfifoGroup_P_Destroy(handle->dfifoGroup);
            handle->dfifoGroup = NULL;
        }

        BAPE_LoopbackGroup_P_Destroy(handle->loopbackGroup);
        handle->loopbackGroup = NULL;
    }

    handle->enabled = false;
}

#if BAPE_CHIP_MAX_FS > 0
static void BAPE_OutputCapture_P_SetFs(BAPE_OutputPort output, unsigned fsNum)
{
    BAPE_OutputCaptureHandle handle;
    BAPE_LoopbackGroupSettings loopbackSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    handle->fs = fsNum;

    /* Update timing source in loopback */
    if ( handle->loopbackGroup )
    {
        BKNI_EnterCriticalSection();
        BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        loopbackSettings.fs = fsNum;
        errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        BKNI_LeaveCriticalSection();
        BDBG_ASSERT(errCode == BERR_SUCCESS);
    }
}
#else
static void BAPE_OutputCapture_P_SetMclk_isr(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BAPE_OutputCaptureHandle handle;
    BAPE_LoopbackGroupSettings loopbackSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( handle->loopbackGroup )
    {
        /* Set Input Parameters */
        BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        loopbackSettings.mclkSource = mclkSource;
        loopbackSettings.pllChannel = pllChannel;
        loopbackSettings.mclkFreqToFsRatio = mclkFreqToFsRatio;
        errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        BDBG_ASSERT(BERR_SUCCESS == errCode);
    }

    handle->mclkSource = mclkSource;
    handle->pllChannel = pllChannel;
    handle->mclkFreqToFsRatio = mclkFreqToFsRatio;
}
#endif

static void BAPE_OutputCapture_P_SetTimingParams_isr(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase)
{
    BAPE_OutputCaptureHandle handle;

    BDBG_OBJECT_ASSERT(output, BAPE_OutputPort);

    handle = output->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BSTD_UNUSED(timebase);

    if ( handle->sampleRate != sampleRate )
    {
        handle->sampleRate = sampleRate;
        if ( handle->interrupts.sampleRate.pCallback_isr )
        {
            handle->interrupts.sampleRate.pCallback_isr(handle->interrupts.sampleRate.pParam1,
                                                        handle->interrupts.sampleRate.param2,
                                                        sampleRate);
        }
    }
}

static void BAPE_OutputCapture_P_ClearInterrupts_isr(BAPE_OutputCaptureHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( handle->enabled )
    {
        BDBG_ASSERT(NULL != handle->dfifoGroup);
        BAPE_DfifoGroup_P_RearmFullmarkInterrupt_isr(handle->dfifoGroup);
        BAPE_DfifoGroup_P_RearmOverflowInterrupt_isr(handle->dfifoGroup);
    }
}
