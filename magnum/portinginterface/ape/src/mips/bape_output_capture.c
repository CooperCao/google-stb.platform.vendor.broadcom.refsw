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
 * Module Description: Audio Output Capture
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
#include "bchp_aud_fmm_iop_ctrl.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_bf_esr1_h.h"
#include "bchp_aud_fmm_bf_esr2_h.h"

BDBG_MODULE(bape_output_capture);

BDBG_OBJECT_ID(BAPE_OutputCapture);

typedef struct BAPE_OutputCapture
{
    BDBG_OBJECT(BAPE_OutputCapture)
    BAPE_Handle deviceHandle;
    unsigned bufferSize, watermark;
    BAPE_OutputCaptureSettings settings;
    unsigned index;
    BAPE_MixerOutputObject connector;
    BAVC_AudioSamplingRate sampleRate;
    void *pBuffer;
    uint32_t bufferOffset;
    unsigned destch, loopback, fs;
    bool enabled;
    BAPE_OutputCaptureInterruptHandlers interrupts;
    char name[7];   /* CAP %d */
} BAPE_OutputCapture;

static void BAPE_OutputCapture_P_Enable(void *pHandle);
static void BAPE_OutputCapture_P_Disable(void *pHandle);
static void BAPE_OutputCapture_P_SetFs(void *pHandle, unsigned fsNum);
static void BAPE_OutputCapture_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase);
static void BAPE_OutputCapture_P_ClearInterrupts_isr(BAPE_OutputCaptureHandle handle);

#ifdef BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG0
#define BAPE_LOOPBACK_CFG_REG_ADDR(index) (BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG0)
#else
#define BAPE_LOOPBACK_CFG_REG_ADDR(index) (BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_BASE+((index*BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_ELEMENT_SIZE)/8))
#endif

#define BAPE_LOOPBACK_FCI(index) ((0x6<<6)|(index))

#if defined(BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0_CAPTURE_MODE_SHIFT) || defined(BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_CAPTURE_MODE_SHIFT)
#define BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE 1
#endif

void BAPE_OutputCapture_GetDefaultOpenSettings(
    BAPE_OutputCaptureOpenSettings *pSettings       /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));    
}

BERR_Code BAPE_OutputCapture_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_OutputCaptureOpenSettings *pSettings,
    BAPE_OutputCaptureHandle *pHandle             /* [out] */
    )
{
    BERR_Code errCode;
    unsigned loopback, destch;
    BAPE_OutputCaptureHandle handle;
    void *pMem, *pCachedMem;
    unsigned bufferSize=0, watermark=0;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( index >= BAPE_CHIP_MAX_OUTPUT_CAPTURES )
    {
        BDBG_ERR(("Unable to open output capture %u.  Max output captures is %u on this platform.", index, BAPE_CHIP_MAX_OUTPUT_CAPTURES));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    
    /* Find an available loopback "port" */
    for ( loopback=0; loopback < BAPE_CHIP_MAX_LOOPBACKS; loopback++ )
    {
        if ( !deviceHandle->loopbackAllocated[loopback] )
        {
            break;
        }
    }
    if ( loopback >= BAPE_CHIP_MAX_LOOPBACKS )
    {
        BDBG_ERR(("Loopback resources exhaused."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find an available destination channel */
    for ( destch=0; destch < BAPE_CHIP_MAX_DESTINATION_CHANNELS; destch++ )
    {
        if ( !deviceHandle->destinationChannelAllocated[destch] )
        {
            break;
        }
    }
    if ( destch >= BAPE_CHIP_MAX_DESTINATION_CHANNELS )
    {
        BDBG_ERR(("Destination channels exhausted.  Please close any unused InputCapture objects."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Determine buffer size */
    if ( NULL != pSettings )
    {
        bufferSize = pSettings->fifoSize;
        watermark = pSettings->watermarkThreshold;
    }
    if ( 0 == bufferSize )
    {
        bufferSize = deviceHandle->pcmBufferSize;
    }
#if (4 == BAPE_CHIP_BYTES_PER_PCM_SAMPLE_PAIR)
    if ( bufferSize < (deviceHandle->pcmBufferSize*2) )
    {
        bufferSize = (deviceHandle->pcmBufferSize*2);
        BDBG_WRN(("Specified buffer size is too small.  Increasing to %u bytes", bufferSize));
    }
#elif (8 == BAPE_CHIP_BYTES_PER_PCM_SAMPLE_PAIR)
    if ( bufferSize < deviceHandle->pcmBufferSize )
    {
        bufferSize = deviceHandle->pcmBufferSize;
        BDBG_WRN(("Specified buffer size is too small.  Increasing to %u bytes", bufferSize));
    }
#else
#error Unsupported PCM sample width
#endif
    if ( 0 == watermark )
    {
        watermark = bufferSize/4;
    }

    /* Allocate handle */
    handle = BKNI_Malloc(sizeof(BAPE_OutputCapture));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_OutputCapture));

    /* Allocate buffer */
    pMem = BMEM_Heap_AllocAligned(deviceHandle->memHandle, bufferSize, 8, 0);
    if ( NULL == pMem )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_buffer;
    }

    errCode = BMEM_Heap_ConvertAddressToOffset(deviceHandle->memHandle, pMem, &handle->bufferOffset);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_offset;
    }

    errCode = BMEM_Heap_ConvertAddressToCached(deviceHandle->memHandle, pMem, &pCachedMem);
    if ( BERR_SUCCESS == errCode )
    {
        /* Flush once at open to make sure the buffer has been invalidated from the cache. */
        BMEM_Heap_FlushCache(deviceHandle->memHandle, pCachedMem, bufferSize);
    }
    /* Not fatal if it fails, assume no cache support */

    BDBG_OBJECT_SET(handle, BAPE_OutputCapture);
    handle->deviceHandle = deviceHandle;
    handle->bufferSize = bufferSize;
    handle->watermark = watermark;
    handle->settings.bitsPerSample = 32;
    handle->index = index;
    BAPE_P_InitMixerOutput(&handle->connector, handle);
    handle->connector.maxChannelPairs = 1;  /* Capture is stero only.  Create an output group for multichannel if supported */
    handle->connector.fsTiming = true;
    handle->connector.compressedSupported = true;
    handle->connector.streamId[BAPE_ChannelPair_eLeftRight] = index+BAPE_CHIP_MAX_OUTPUT_PORTS; /* Loopback(s) follow OPs in IOP */
    handle->connector.enable = BAPE_OutputCapture_P_Enable;
    handle->connector.disable = BAPE_OutputCapture_P_Disable;
    handle->connector.setFs = BAPE_OutputCapture_P_SetFs;
    handle->connector.setTimingParams_isr = BAPE_OutputCapture_P_SetTimingParams_isr;
    handle->sampleRate = BAVC_AudioSamplingRate_eUnknown;
    handle->pBuffer = pMem;
    handle->destch = destch;
    deviceHandle->destinationChannelAllocated[destch] = true;
    handle->loopback = loopback;
    deviceHandle->loopbackAllocated[loopback] = true;
    handle->fs = (unsigned)-1;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "CAP %u", index); 

    handle->deviceHandle->outputCaptures[index] = handle;

    *pHandle = handle;
    return BERR_SUCCESS;

err_offset:
    BMEM_Heap_Free(deviceHandle->memHandle, pMem);
err_buffer:
    BKNI_Free(handle);
    return errCode;
}

void BAPE_OutputCapture_Close(
    BAPE_OutputCaptureHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BMEM_Heap_Free(handle->deviceHandle->memHandle, handle->pBuffer);
    handle->deviceHandle->loopbackAllocated[handle->loopback] = false;
    handle->deviceHandle->destinationChannelAllocated[handle->destch] = false;
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
#if BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE
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
    uint32_t regAddr, regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    
    regAddr = BAPE_P_DEST_CHANNEL_TO_WRADDR_REG(handle->destch);
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regAddr = BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(handle->destch);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
}

BERR_Code BAPE_OutputCapture_GetBuffer(
    BAPE_OutputCaptureHandle handle,
    void **pBuffer,                     /* [out] Address data should be written to (uncached) */
    unsigned *pBufferSize               /* [out] Contiguous buffer size in bytes */
    )
{   
    uint32_t rd, wr, base, wrap;
    unsigned bufferSize;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferSize);

    if ( !handle->enabled )
    {
        *pBuffer = NULL;
        *pBufferSize = 0;
        return BERR_SUCCESS;
    }

    BKNI_EnterCriticalSection();

    /* Read registers */
    rd = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(handle->destch));
    wr = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_WRADDR_REG(handle->destch));
    base = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_BASEADDR_REG(handle->destch));
    
    /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */    
    wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^ 
        BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

    /* Mask off toggle bits */
    rd = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);

    /* Get base address to read from */
    errCode = BMEM_Heap_ConvertOffsetToAddress(handle->deviceHandle->memHandle, rd, pBuffer);
    if ( errCode )
    {
        BKNI_LeaveCriticalSection();
        return BERR_TRACE(errCode);
    }

    /* Compute size of contiguous space */
    if ( wrap )
    {
        bufferSize = handle->bufferSize - (rd-base);
    }
    else
    {
        bufferSize = (wr - rd);
    }

    *pBufferSize = bufferSize;

    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

BERR_Code BAPE_OutputCapture_ConsumeData(
    BAPE_OutputCaptureHandle handle,
    unsigned numBytes                   /* Number of bytes read from the buffer */
    )
{
    uint32_t rd, wr, base, wrap, rdaddr, wraddr, wrapBit;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( !handle->enabled )
    {
        return BERR_SUCCESS;
    }

    BKNI_EnterCriticalSection();

    /* Read registers */
    rd = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(handle->destch));
    wr = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_WRADDR_REG(handle->destch));
    base = BREG_Read32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_BASEADDR_REG(handle->destch));

    /* Same toggle bit means no wrap.  Opposite toggle bits means wrap. */    
    wrap = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ^ 
        BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP);

    /* Mask off toggle bits */
    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);

    /* Check for potential overflow */
    if ( wrap )
    {
        if ( ((rdaddr-base)+numBytes) > handle->bufferSize )
        {
            BKNI_LeaveCriticalSection();
            BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [wrap]"));
            BDBG_ERR(("rd %#x wr %#x base %#x size %#x numBytes %#x", rd, wr, base, handle->bufferSize, numBytes));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        if ( (rdaddr+numBytes) > wraddr )
        {
            BKNI_LeaveCriticalSection();
            BDBG_ERR(("Invalid number of bytes provided to BAPE_OutputCapture_ConsumeData [no wrap]"));
            BDBG_ERR(("rd %#x wr %#x base %#x size %#x numBytes %#x", rd, wr, base, handle->bufferSize, numBytes));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* Update read pointer */
    rdaddr += numBytes;
    wrapBit = rd & BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
    if ( (rdaddr-base) >= handle->bufferSize )
    {
        wrapBit ^= BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP);
        rdaddr = base;
    }
    rdaddr |= wrapBit;
    BREG_Write32_isr(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(handle->destch), rdaddr);
    BAPE_OutputCapture_P_ClearInterrupts_isr(handle);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void BAPE_OutputCapture_GetConnector(
    BAPE_OutputCaptureHandle handle,
    BAPE_MixerOutput *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->connector;
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
        /* Install interrupt handlers */
        errCode = BAPE_P_SetDestinationChannelFullmarkInterrupt(handle->deviceHandle, handle->destch, 
                                                                handle->interrupts.watermark.pCallback_isr,
                                                                handle->interrupts.watermark.pParam1,
                                                                handle->interrupts.watermark.param2);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        errCode = BAPE_P_SetDestinationChannelOverflowInterrupt(handle->deviceHandle, handle->destch, 
                                                                handle->interrupts.overflow.pCallback_isr,
                                                                handle->interrupts.overflow.pParam1,
                                                                handle->interrupts.overflow.param2);
        if ( errCode )
        {
            BAPE_P_SetDestinationChannelFullmarkInterrupt(handle->deviceHandle, handle->destch, NULL, NULL, 0);
            return BERR_TRACE(errCode);
        }
    }

    handle->interrupts = *pInterrupts;
    return BERR_SUCCESS;
}

static void BAPE_OutputCapture_P_Enable(void *pHandle)
{
    uint32_t regAddr, regVal, regAnd, regOr;
    BERR_Code errCode;
    BAPE_OutputCaptureHandle handle = (BAPE_OutputCaptureHandle)pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    BDBG_ASSERT(false == handle->enabled);

    /* Setup and enable the destination channel first, then IOP */

    /* RBUF setup */
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(handle->destch), handle->bufferOffset);
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_WRADDR_REG(handle->destch), handle->bufferOffset);
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_BASEADDR_REG(handle->destch), handle->bufferOffset);
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_ENDADDR_REG(handle->destch), handle->bufferOffset+handle->bufferSize-1);
    BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_DEST_CHANNEL_TO_FREEFULL_REG(handle->destch), handle->watermark);

    /* Install interrupt handlers */
    if ( handle->interrupts.watermark.pCallback_isr )
    {
        errCode = BAPE_P_SetDestinationChannelFullmarkInterrupt(handle->deviceHandle, handle->destch, 
                                                                handle->interrupts.watermark.pCallback_isr,
                                                                handle->interrupts.watermark.pParam1,
                                                                handle->interrupts.watermark.param2);
        BDBG_ASSERT(errCode == BERR_SUCCESS);
    }
    if ( handle->interrupts.overflow.pCallback_isr )
    {
        errCode = BAPE_P_SetDestinationChannelOverflowInterrupt(handle->deviceHandle, handle->destch, 
                                                                handle->interrupts.overflow.pCallback_isr,
                                                                handle->interrupts.overflow.pParam1,
                                                                handle->interrupts.overflow.param2);
        BDBG_ASSERT(errCode == BERR_SUCCESS);
    }

    /* Setup DESTCH */
#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0;
    regAnd = ~(BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, BUFFER_PAIR_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, DESTFIFO_SIZE_DOUBLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PLAY_FROM_CAPTURE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_TO_SOURCEFIFO)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, NOT_PAUSE_WHEN_FULL)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, FCI_CAP_ID)|
            #if BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_MODE)|
            #endif
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID));
    regOr = BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE, 1) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, DESTFIFO_SIZE_DOUBLE, 1) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, FCI_CAP_ID, BAPE_LOOPBACK_FCI(handle->loopback)) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID, 1);
    #if BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE
    if ( handle->settings.bitsPerSample == 16 )
    {
        regOr |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_MODE, 1);
    }
    #endif
#else
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + ((handle->destch *BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_ELEMENT_SIZE)/8);
    regAnd = ~(BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, BUFFER_PAIR_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PLAY_FROM_CAPTURE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_TO_SOURCEFIFO)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, NOT_PAUSE_WHEN_FULL)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID)|
            #if BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE)|
            #endif
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID));
    regOr = BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE, 1) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, DESTFIFO_SIZE_DOUBLE, 1) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, FCI_CAP_ID, BAPE_LOOPBACK_FCI(handle->loopback)) |
            BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID, 1);
    #if BAPE_DESTCH_SUPPORTS_16BIT_CAPTURE
    if ( handle->settings.bitsPerSample == 16 )
    {
        regOr |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_MODE, 1);
    }
    #endif
#endif

    /* This register is shared with the sample rate change interrupt.  Protect against re-entrancy. */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= regAnd;
    regVal |= regOr;
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
    #ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRL0, 1);
    #else
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ((handle->destch *BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_ELEMENT_SIZE)/8);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, 1);
    #endif
    BKNI_LeaveCriticalSection();

    /* Set bit resolution correctly */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_FCI_CFGi_ARRAY_BASE + (4*handle->connector.streamId[0]);
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_CTRL_FCI_CFGi, STREAM_BIT_RESOLUTION, Res_16_Bit);
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Enable loopback in IOP */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_ELEMENT_SIZE * handle->loopback)/8);
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, ENA)|
                BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, ENA, 1) |
              BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP, handle->loopback);
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Done */
    handle->enabled = true;
}

static void BAPE_OutputCapture_P_Disable(void *pHandle)
{
    uint32_t regAddr, regVal, regAnd;
    BAPE_OutputCaptureHandle handle = (BAPE_OutputCaptureHandle)pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    /*BDBG_ASSERT(handle->enabled);*/

    /* Stop the IOP first then the destination channel */

    /* Stop IOP */
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE + ((BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_ELEMENT_SIZE * handle->loopback)/8);
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, ENA));
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

    /* Stop DESTCH */
    #ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
    BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRL0, 0);
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0;
    regAnd = ~(BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, CAPTURE_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_SEQ_ID_VALID));
    #else
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_BASE + ((handle->destch *BCHP_AUD_FMM_BF_CTRL_DESTCH_CTRLi_ARRAY_ELEMENT_SIZE)/8);
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, 0);
    regAddr = BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_BASE + ((handle->destch *BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_ELEMENT_SIZE)/8);
    regAnd = ~(BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, CAPTURE_ENABLE)|
               BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_SEQ_ID_VALID));
    #endif

    /* This register is shared with the sample rate change interrupt.  Protect against re-entrancy. */
    BKNI_EnterCriticalSection();
    regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);
    regVal &= regAnd;
    BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
    BKNI_LeaveCriticalSection();

    /* Clear interrupts */
    BAPE_P_SetDestinationChannelFullmarkInterrupt(handle->deviceHandle, handle->destch, NULL, NULL, 0);
    BAPE_P_SetDestinationChannelOverflowInterrupt(handle->deviceHandle, handle->destch, NULL, NULL, 0);

    handle->enabled = false;
}

static void BAPE_OutputCapture_P_SetFs(void *pHandle, unsigned fsNum)
{
    uint32_t regAddr, regVal;
    BAPE_OutputCaptureHandle handle = (BAPE_OutputCaptureHandle)pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    handle->fs = fsNum;    

    regAddr = BAPE_LOOPBACK_CFG_REG_ADDR(handle->loopback);
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    #ifdef BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG0
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFG0, TMG_SRC_SEL);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFG0, TMG_SRC_SEL, fsNum);
    #else
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, TMG_SRC_SEL);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_LOOPBACK_CFGi, TMG_SRC_SEL, fsNum);
    #endif
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
}

static void BAPE_OutputCapture_P_SetTimingParams_isr(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase)
{
    BAPE_OutputCaptureHandle handle = (BAPE_OutputCaptureHandle)pHandle;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);
    BSTD_UNUSED(timebase);

    if ( handle->sampleRate != sampleRate )
    {
        uint32_t priority, regAddr, regVal;

        handle->sampleRate = sampleRate;
        if ( handle->interrupts.sampleRate.pCallback_isr )
        {
            handle->interrupts.sampleRate.pCallback_isr(handle->interrupts.sampleRate.pParam1,
                                                        handle->interrupts.sampleRate.param2,
                                                        sampleRate);
        }

/* 20110310 bandrews - this currently writes to the wrong address, so I am
removing it per Jon's advice until when / if we figure out how to use it */
#if 0
        switch ( handle->sampleRate )
        {
        case BAVC_AudioSamplingRate_e96k:
        case BAVC_AudioSamplingRate_e128k:
        case BAVC_AudioSamplingRate_e176_4k:
        case BAVC_AudioSamplingRate_e192k:
            priority = 1;
            break;
        default:
            priority = 0;
            break;
        }

        regAddr = BAPE_LOOPBACK_CFG_REG_ADDR(handle->loopback);

        /* Update priority bit based on sample rate (>= 96k requires high priority) */
        regVal = BREG_Read32_isr(handle->deviceHandle->regHandle, regAddr);     
        #ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_ID_HIGH);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFG0, PROCESS_ID_HIGH, priority);
        #else
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_DESTCH_CFGi, PROCESS_ID_HIGH, priority);
        #endif
        BREG_Write32_isr(handle->deviceHandle->regHandle, regAddr, regVal);
#else
		BSTD_UNUSED(priority);
		BSTD_UNUSED(regAddr);
		BSTD_UNUSED(regVal);
#endif
    }
}

static void BAPE_OutputCapture_P_ClearInterrupts_isr(BAPE_OutputCaptureHandle handle)
{
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_OutputCapture);

    if ( handle->enabled )
    {
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_ESR1_H_STATUS_CLEAR, 1<<handle->destch);        
        regVal = (1<<BCHP_SHIFT(AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, REARM_FULLMARK))<<handle->destch;
        BREG_Write32_isr(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, regVal);
    }
}

