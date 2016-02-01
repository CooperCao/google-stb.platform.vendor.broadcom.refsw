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
*   API name: AudioCapture
*    Specific APIs related to PCM audio capture.  This supports capture
*    of data into memory from a decoder or other source.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_audio_capture.h"

BDBG_MODULE(nexus_audio_capture);

static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle);
static void NEXUS_AudioCapture_P_Interrupt_isr(void *pParam, int param);
static void NEXUS_AudioCapture_P_SampleRate_isr(void *pParam, int param, BAVC_AudioSamplingRate sampleRate);
static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle);

/***************************************************************************
Summary:
Get default settings for opening an audio capture channel
***************************************************************************/
void NEXUS_AudioCapture_GetDefaultOpenSettings(
    NEXUS_AudioCaptureOpenSettings *pSettings      /* [out] default settings */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fifoSize = 88*1024*2;
}

#if NEXUS_NUM_AUDIO_CAPTURES
typedef struct NEXUS_AudioCapture
{
    NEXUS_OBJECT(NEXUS_AudioCapture);
    bool opened;
    bool running;
    BAPE_OutputCaptureHandle apeHandle;
    NEXUS_AudioOutputObject connector;
    NEXUS_AudioCaptureSettings settings;
    void *pMemory, *pCachedMemory;
    uint32_t *pBuffer;
    int bufferSize, rptr, wptr, bufferDepth;
    NEXUS_IsrCallbackHandle dataCallback, sampleRateCallback;
    unsigned sampleRate;
} NEXUS_AudioCapture;

static NEXUS_AudioCapture g_captures[NEXUS_NUM_AUDIO_CAPTURES];

NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    NEXUS_AudioCaptureOpenSettings defaults;
    BAPE_OutputCaptureOpenSettings openSettings;
    NEXUS_AudioCaptureHandle handle;
    BAPE_OutputCaptureInterruptHandlers interrupts;
    BAPE_MixerOutput connector;
    BERR_Code errCode;

    if ( index >= NEXUS_NUM_AUDIO_CAPTURES )
    {
        BDBG_ERR(("index out of range."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = &g_captures[index];
    if ( handle->opened )
    {
        BDBG_ERR(("AudioCapture %u already open", index));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    if ( NULL == pSettings )
    {
        NEXUS_AudioCapture_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    BAPE_OutputCapture_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize = pSettings->fifoSize;
    if ( pSettings->threshold )
    {
        openSettings.watermarkThreshold = pSettings->threshold & (~255);
    }

    NEXUS_OBJECT_INIT(NEXUS_AudioCapture, handle);
    
    handle->dataCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->dataCallback )
    {
        (void)BERR_TRACE(BERR_OS_ERROR);
        goto err_data_callback;
    }

    handle->sampleRateCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->sampleRateCallback )
    {
        (void)BERR_TRACE(BERR_OS_ERROR);
        goto err_sample_rate_callback;
    }

    errCode = BAPE_OutputCapture_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &openSettings, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_ape_handle;
    }

    handle->pMemory = BMEM_Heap_Alloc(g_pCoreHandles->heap[0].mem, pSettings->fifoSize);
    if ( NULL == handle->pMemory )
    {
        (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_memory;
    }
    BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, handle->pMemory, &handle->pCachedMemory);
    handle->pBuffer = handle->pCachedMemory;
    handle->bufferSize = pSettings->fifoSize;
    handle->opened = true;
    handle->connector.objectType = NEXUS_AudioOutputType_eCapture;
    handle->connector.pObjectHandle = handle;
    BAPE_OutputCapture_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (uint32_t)connector;
    handle->settings.format = NEXUS_AudioCaptureFormat_e16BitStereo;

    BAPE_OutputCapture_GetInterruptHandlers(handle->apeHandle, &interrupts);
    interrupts.watermark.pCallback_isr = NEXUS_AudioCapture_P_Interrupt_isr;
    interrupts.watermark.pParam1 = handle;
    interrupts.sampleRate.pCallback_isr = NEXUS_AudioCapture_P_SampleRate_isr;
    interrupts.sampleRate.pParam1 = handle;
    errCode = BAPE_OutputCapture_SetInterruptHandlers(handle->apeHandle, &interrupts);
    BDBG_ASSERT(errCode == BERR_SUCCESS);

    return handle;

err_memory:
    BAPE_OutputCapture_Close(handle->apeHandle);
err_ape_handle:
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
err_sample_rate_callback:
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
err_data_callback:
    return NULL;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioCapture, NEXUS_AudioCapture_Close);

static void NEXUS_AudioCapture_P_Finalizer(
    NEXUS_AudioCaptureHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BDBG_ASSERT(handle->opened);
    NEXUS_AudioOutput_RemoveAllInputs(&handle->connector);
    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BAPE_OutputCapture_Close(handle->apeHandle);
    NEXUS_IsrCallback_Destroy(handle->sampleRateCallback);
    NEXUS_IsrCallback_Destroy(handle->dataCallback);
    BMEM_Free(g_pCoreHandles->heap[0].mem, handle->pMemory);
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioCapture));
}

void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BDBG_ASSERT(handle->opened);
    BDBG_ASSERT(NULL != pSettings);
    switch ( pSettings->format )
    {
    default:
        break;
    case NEXUS_AudioCaptureFormat_e24BitStereo:
        BDBG_ERR(("24-bit capture not supported on this chipset"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case NEXUS_AudioCaptureFormat_e24Bit5_1:
        BDBG_ERR(("Multichannel capture not supported on this chipset"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);

    if ( handle->running )
    {
        BDBG_ERR(("Already running"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Setup callback */
    NEXUS_IsrCallback_Set(handle->dataCallback, pSettings?&pSettings->dataCallback:NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, pSettings?&pSettings->sampleRateCallback:NULL);

    /* Setup internal buffer and start */
    BKNI_EnterCriticalSection();
    NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
    handle->running = true;
    handle->wptr = handle->rptr = handle->bufferDepth = 0;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);

    BDBG_MSG(("Stopping"));

    BKNI_EnterCriticalSection();
    handle->running = false;
    BKNI_LeaveCriticalSection();

    NEXUS_IsrCallback_Set(handle->dataCallback, NULL);
    NEXUS_IsrCallback_Set(handle->sampleRateCallback, NULL);
}

NEXUS_Error NEXUS_AudioCapture_GetBuffer(
    NEXUS_AudioCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BDBG_ASSERT(NULL != ppBuffer);
    BDBG_ASSERT(NULL != pSize);

    /* Defaults for error returns */
    *ppBuffer = NULL;
    *pSize = 0;

    if ( !handle->running )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("Before Conversion rptr %d wptr %d depth %d/%d", handle->rptr, handle->wptr, handle->bufferDepth, handle->bufferSize));

    switch ( handle->settings.format )
    {
    case NEXUS_AudioCaptureFormat_e24BitStereo:
        NEXUS_AudioCapture_P_ConvertStereo24(handle);
        break;
    case NEXUS_AudioCaptureFormat_e16BitStereo:
        NEXUS_AudioCapture_P_ConvertStereo16(handle);
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    *ppBuffer = &handle->pBuffer[handle->rptr];
    if ( handle->bufferDepth == 0 )
    {
        /* No data */
        *pSize = 0;
    }
    else if ( handle->rptr < handle->wptr )
    {
        /* no wraparound */
        *pSize = (handle->wptr - handle->rptr)*4;
    }
    else
    {
        /* Buffer has wrapped, give contiguous size to end of buffer */
        *pSize = handle->bufferSize - (4*handle->rptr);
    }

    BDBG_MSG(("rptr %d wptr %d size %d words (%d bytes)", handle->rptr, handle->wptr, (*pSize)/4, *pSize));

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioCapture_WriteComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    int newRptr;

    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);

    if ( !handle->running || NULL == handle )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( amountWritten % 4 )
    {
        BDBG_ERR(("The buffer must be updated in increments of four bytes"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    newRptr = handle->rptr + (amountWritten/4);

    if ( newRptr == (handle->bufferSize/4) )
    {
        newRptr = 0;
    }
    else if ( newRptr > (handle->bufferSize/4) )
    {
        BDBG_ERR(("Overflow - invalid number of bytes passed"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Sanity check error cases where the read pointer passes the write pointer */
    if ( newRptr != handle->wptr )
    {
        if ( (handle->rptr < handle->wptr) &&
             (newRptr > handle->wptr) )
        {
            BDBG_ERR(("Overflow - invalid number of bytes passed"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        else if ( (handle->rptr > handle->wptr) &&
                  (newRptr < handle->wptr) &&
                  newRptr != 0 )
        {
            BDBG_ERR(("Overflow - invalid number of bytes passed"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    BDBG_MSG(("Write Complete - %d bytes written, moving rptr from %d to %d (size %d)", amountWritten, handle->rptr, newRptr, handle->bufferSize/4));

    /* Update read pointer of our buffer */
    handle->rptr = newRptr;
    handle->bufferDepth -= amountWritten;

    return BERR_SUCCESS;
}

NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    return &handle->connector;
}

static size_t NEXUS_AudioCapture_P_GetContiguousSpace(NEXUS_AudioCaptureHandle handle)
{
    if ( handle->bufferDepth == handle->bufferSize )
    {
        return 0;
    }
    else if ( handle->wptr < handle->rptr )
    {
        /* No Wrap */
        return handle->bufferSize - handle->bufferDepth;
    }
    else
    {
        /* Wrap - return contiguous amount */
        return handle->bufferSize - (handle->wptr*4);
    }
}

static void NEXUS_AudioCapture_P_AdvanceBuffer(NEXUS_AudioCaptureHandle handle, size_t bytes)
{
    handle->bufferDepth += bytes;
    handle->wptr += (bytes)/4;
    if ( handle->wptr >= (handle->bufferSize/4) )
    {
        handle->wptr = 0;
    }
}

static void NEXUS_AudioCapture_P_ConvertStereo24(NEXUS_AudioCaptureHandle handle)
{
    BERR_Code errCode;
    void *pBuffer, *pCachedBuffer;
    uint32_t *pSource;
    size_t bufferSize, copied;

    for ( ;; )
    {
        errCode = BAPE_OutputCapture_GetBuffer(handle->apeHandle, &pBuffer, &bufferSize);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, pBuffer, &pCachedBuffer);
        pSource = pCachedBuffer;
        copied = 0;
        while ( bufferSize >= 4 )
        {
            size_t available, bytesToCopy;
            if ( handle->bufferDepth > (handle->bufferSize-3) )
            {
                /* Our buffer is full. */
                return;
            }
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available >= bufferSize )
            {
                bytesToCopy = bufferSize;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy > 0 )
            {
                handle->pBuffer[handle->wptr] = (*pSource++) & 0xffffff00;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 4;
                bufferSize -= 4;
                bytesToCopy -= 4;
            }
        }
        BMEM_Heap_FlushCache(g_pCoreHandles->heap[0].mem, pCachedBuffer, copied);
        errCode = BAPE_OutputCapture_ConsumeData(handle->apeHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

static void NEXUS_AudioCapture_P_ConvertStereo16(NEXUS_AudioCaptureHandle handle)
{
    BERR_Code errCode;
    void *pBuffer, *pCachedBuffer;
    uint32_t *pSource;
    size_t bufferSize, copied;

    for ( ;; )
    {
        errCode = BAPE_OutputCapture_GetBuffer(handle->apeHandle, &pBuffer, &bufferSize);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
        if ( bufferSize == 0 )
        {
            /* Done */
            return;
        }
        BMEM_Heap_ConvertAddressToCached(g_pCoreHandles->heap[0].mem, pBuffer, &pCachedBuffer);
        pSource = pCachedBuffer;
        copied = 0;
        while ( bufferSize >= 8 )
        {
            size_t available, bytesToCopy;
            if ( handle->bufferDepth > (handle->bufferSize-3) )
            {
                /* Our buffer is full. */
                return;
            }
            available = NEXUS_AudioCapture_P_GetContiguousSpace(handle);
            if ( available >= bufferSize/2 )
            {
                bytesToCopy = bufferSize/2;
            }
            else
            {
                bytesToCopy = available;
            }
            while ( bytesToCopy > 0 )
            {
                uint32_t sample;
                sample = (*pSource++)>>16;          /* Left */
                sample |= (*pSource++)&0xffff0000;  /* Right */
                handle->pBuffer[handle->wptr] = sample;
                NEXUS_AudioCapture_P_AdvanceBuffer(handle, 4);
                copied += 8;
                bufferSize -= 8;
                bytesToCopy -= 4;
            }
        }
        BMEM_Heap_FlushCache(g_pCoreHandles->heap[0].mem, pCachedBuffer, copied);
        errCode = BAPE_OutputCapture_ConsumeData(handle->apeHandle, copied);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

static void NEXUS_AudioCapture_P_Interrupt_isr(void *pParam, int param)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BSTD_UNUSED(param);

    BDBG_MSG(("Capture Interrupt"));

    if ( handle->running )
    {
        /* Propagate directly to app.  They will read the data out as part of GetBuffer */
        NEXUS_IsrCallback_Fire_isr(handle->dataCallback);
    }
    else
    {
        NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(handle);
    }    
}

static void NEXUS_AudioCapture_P_SampleRate_isr(void *pParam, int param, BAVC_AudioSamplingRate sampleRate)
{
    NEXUS_AudioCaptureHandle handle;

    handle = pParam;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BSTD_UNUSED(param);

    BDBG_MSG(("Sample Rate Interrupt (%u)", sampleRate));

    handle->sampleRate = NEXUS_AudioModule_P_Avc2SampleRate(sampleRate);
    NEXUS_IsrCallback_Fire_isr(handle->sampleRateCallback);
}

static void NEXUS_AudioCapture_P_FlushDeviceBuffer_isr(NEXUS_AudioCaptureHandle handle)
{
    BAPE_OutputCapture_Flush_isr(handle->apeHandle);
}

NEXUS_Error NEXUS_AudioCapture_GetStatus(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureStatus *pStatus   /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioCapture, handle);
    BDBG_ASSERT(NULL != pStatus);
    BKNI_Memset(pStatus, 0, sizeof(NEXUS_AudioCaptureStatus));
    pStatus->started = handle->running;
    pStatus->sampleRate = handle->sampleRate;
    return BERR_SUCCESS;
}
#else
NEXUS_AudioCaptureHandle NEXUS_AudioCapture_Open(     /* attr{destructor=NEXUS_AudioCapture_Close}  */
    unsigned index,
    const NEXUS_AudioCaptureOpenSettings *pSettings    /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_AudioCapture_Close(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void NEXUS_AudioCapture_GetSettings(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioCapture_SetSettings(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioCapture_GetDefaultStartSettings(
    NEXUS_AudioCaptureStartSettings *pSettings  /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioCapture_Start(
    NEXUS_AudioCaptureHandle handle,
    const NEXUS_AudioCaptureStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioCapture_Stop(
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_Error NEXUS_AudioCapture_GetBuffer(
    NEXUS_AudioCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(ppBuffer);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioCapture_WriteComplete(
    NEXUS_AudioCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioOutputHandle NEXUS_AudioCapture_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_AudioCapture_GetStatus(
    NEXUS_AudioCaptureHandle handle,
    NEXUS_AudioCaptureStatus *pStatus   /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
