/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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

BDBG_MODULE(bape_playback);

BDBG_OBJECT_ID(BAPE_Playback);

typedef struct BAPE_Playback
{
    BDBG_OBJECT(BAPE_Playback)
    BAPE_Handle deviceHandle;
    unsigned index;
    BAPE_MixerInputObject connector;
    BAPE_MixerInputLink *pMaster;
    void *pBuffer;
    unsigned bufferSize;
    unsigned threshold;
    unsigned bufferDepth;   /* Required for pre-start buffer fills (used in looparound mode) */
    BAPE_PlaybackInterruptHandlers interrupts;
    bool loopEnabled;
    bool running;
    char name[12]; /* Playback %d */
} BAPE_Playback;

static void BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle handle);

void BAPE_Playback_GetDefaultOpenSettings(
    BAPE_PlaybackOpenSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->fifoSize = 128*1024;
    pSettings->watermarkThreshold = 64*1024;
}

BERR_Code BAPE_Playback_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_PlaybackOpenSettings *pSettings,
    BAPE_PlaybackHandle *pHandle                    /* [out] */
    )
{
    BAPE_PlaybackHandle handle;
    BAPE_PlaybackOpenSettings defaultSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);
    
    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BAPE_Playback_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if ( index >= BAPE_CHIP_MAX_PLAYBACKS )
    {
        BDBG_ERR(("This chip only supports %u playbacks.  Cannot open playback %u", BAPE_CHIP_MAX_PLAYBACKS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->playbacks[index] )
    {
        BDBG_ERR(("Playback %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Playback));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_Playback));
    handle->deviceHandle = deviceHandle;
    handle->pBuffer = BMEM_AllocAligned(deviceHandle->memHandle, pSettings->fifoSize, 5, 0);
    if ( NULL == handle->pBuffer )
    {
        BKNI_Free(handle);
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    handle->bufferSize = pSettings->fifoSize;
    handle->threshold = pSettings->watermarkThreshold;
    handle->index = index;
    BAPE_P_InitMixerInput(&handle->connector, BAPE_MixerInputType_ePlayback, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "Playback %u", index);
    handle->connector.pName = handle->name;
    handle->connector.numChannelPairs = 1;  /* Stereo only */
    BDBG_OBJECT_SET(handle, BAPE_Playback);
    *pHandle = handle;
    deviceHandle->playbacks[index] = handle;
    return BERR_SUCCESS;
}

void BAPE_Playback_Close(
    BAPE_PlaybackHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    if ( handle->running )
    {
        BDBG_WRN(("Stopping playback %p (%d) on close", handle, handle->index));
        BAPE_Playback_Stop(handle);
    }

    /* Disconnect from all mixers, post-processors, and groups */
    BAPE_MixerInput_P_Shutdown(&handle->connector);

    BMEM_Free(handle->deviceHandle->memHandle, handle->pBuffer);
    handle->deviceHandle->playbacks[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_Playback);
    BKNI_Free(handle);
}

void BAPE_Playback_GetDefaultStartSettings(
    BAPE_PlaybackStartSettings *pSettings       /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->sampleRate = BAVC_AudioSamplingRate_e44_1k;
    pSettings->bitsPerSample = 16;
    pSettings->isStereo = true;
    pSettings->isSigned = true;
    pSettings->reverseEndian = false;
    pSettings->loopEnabled = false;
    pSettings->startThreshold = 0;
}

BERR_Code BAPE_Playback_Start(
    BAPE_PlaybackHandle handle,
    const BAPE_PlaybackStartSettings *pSettings
    )
{
    uint32_t sourceChannelId;
    BERR_Code errCode;
    uint32_t regAddr, regVal;
    uint32_t priority, resolution, offset;
    BAPE_MixerInputLink *pLink, *pMaster=NULL;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    BDBG_ASSERT(NULL != pSettings);

    switch ( pSettings->sampleRate )
    {
    case BAVC_AudioSamplingRate_e96k:
    case BAVC_AudioSamplingRate_e128k:
    case BAVC_AudioSamplingRate_e176_4k:
    case BAVC_AudioSamplingRate_e192k:
        priority = 1;
        break;
    case BAVC_AudioSamplingRate_eUnknown:
        BDBG_ERR(("Invalid sampling rate"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    default:
        priority = 0;
        break;
    }

    switch ( pSettings->bitsPerSample )
    {
    case 8:
    case 16:
    case 32:
        resolution=pSettings->bitsPerSample;
        break;
    default:
        BDBG_ERR(("Unsupported bits per sample.  Only 8/16/32 are supported."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    
    /* If we're already running, return error */
    if ( handle->running )
    {
        BDBG_ERR(("Playback %p (%d) is already running.  Can't start.", handle, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Playback %p (%d) Starting", handle, handle->index));

    /* Tell the mixer to allocate resources if not already done */
    errCode = BAPE_MixerInput_P_PrepareToStart(&handle->connector, false);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    for ( pLink = BLST_S_FIRST(&handle->connector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        sourceChannelId = pLink->sourceChannelId[0];
        if ( sourceChannelId != 0xff )
        {
            if ( NULL == pMaster )
            {
                pMaster = pLink;
            }
            if ( sourceChannelId >= BAPE_CHIP_MAX_SOURCE_CHANNELS )
            {
                BDBG_ERR(("Invalid source channel ID %u", sourceChannelId));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            BDBG_MSG(("Playback %p (%d) Using Source Channel %d", handle, handle->index, sourceChannelId));

            /* Set the source channel configuration */
            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*sourceChannelId);
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            regVal &= ~(
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, STCSYNC_ENABLE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE)|
                        BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE)
                        );
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID, 1);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH, priority);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN, pSettings->reverseEndian?1:0);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, resolution);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 1);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE, handle->deviceHandle->settings.rampPcmSamples?1:0);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY, pSettings->loopEnabled?1:0);
            if ( pSettings->startThreshold > 0 )
            {
                if ( pSettings->loopEnabled )
                {
                    BDBG_WRN(("Playback start threshold is not compatible with the loop setting.  Disabling startThreshold."));
                }
                else
                {
                    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION, 1);
                }
            }
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 1);
            regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, pSettings->isStereo?0:1);
            if ( pMaster && pMaster != pLink )
            {
                BDBG_MSG(("Linking playback source channel %u to master %u", sourceChannelId, pMaster->sourceChannelId[0]));
                regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID, pMaster->sourceChannelId[0]);
                regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 1);
            }
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

            /* Setup the ringbuffer registers */
            errCode = BMEM_ConvertAddressToOffset(handle->deviceHandle->memHandle, handle->pBuffer, &offset);
            BDBG_ASSERT(BERR_SUCCESS == errCode);
            regAddr = BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId);
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, offset);
            regAddr += 4;   /* Write is next */
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, offset+handle->bufferDepth);
            regAddr += 4;   /* Base is next */
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, offset);
            regAddr += 4;   /* End is next */
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, offset + handle->bufferSize - 1);
            regAddr += 4;   /* Freefull is next */
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, handle->threshold);
            regAddr += 4;   /* WRPOINT is last */
            if ( pSettings->startThreshold > handle->bufferSize )
            {
                BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                regVal = offset + handle->bufferSize - 1;
            }
            else
            {
                regVal = offset + pSettings->startThreshold;
            }
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);

            regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + (4*sourceChannelId);
            regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
            regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB);
            if ( !pSettings->isSigned )
            {
                regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB, 1);
            }
            BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
        }
    }

    /* Install the interrupt handler */
    if ( pMaster && handle->interrupts.watermark.pCallback_isr )
    {
        errCode = BAPE_P_SetSourceChannelFreemarkInterrupt(handle->deviceHandle,
                                                           pMaster->sourceChannelId[0],
                                                           handle->interrupts.watermark.pCallback_isr,
                                                           handle->interrupts.watermark.pParam1,
                                                           handle->interrupts.watermark.param2);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }    

    /* Done, start the hardware. */
    errCode = BAPE_MixerInput_P_Start(&handle->connector, pSettings->sampleRate);
    if ( errCode )
    {
        if ( pMaster )
        {
            /* Remove the interrupt handler */
            (void)BAPE_P_SetSourceChannelFreemarkInterrupt(handle->deviceHandle,
                                                           pMaster->sourceChannelId[0],
                                                           NULL, NULL, 0);
        }
        return BERR_TRACE(errCode);
    }

    /* Success */
    BAPE_Playback_P_ReArmWatermark(handle);
    BDBG_MSG(("Playback %p (%d) Start Successful", handle, handle->index));
    handle->running = true;
    handle->pMaster = pMaster;
    handle->loopEnabled = pSettings->loopEnabled;
    return BERR_SUCCESS;
}

void BAPE_Playback_Stop(
    BAPE_PlaybackHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    
    if ( !handle->running )
    {
        BDBG_MSG(("Playback %p (%d) already stopped.", handle, handle->index));
    }

    /* Stop the hardware */
    BAPE_MixerInput_P_Stop(&handle->connector);
    
    /* Remove the interrupt handler */    
    if ( handle->pMaster && handle->interrupts.watermark.pCallback_isr )
    {
        (void)BAPE_P_SetSourceChannelFreemarkInterrupt(handle->deviceHandle,
                                                       handle->pMaster->sourceChannelId[0],
                                                       NULL, NULL, 0);
    }

    /* Clear the started flag and flush the buffer */
    handle->running = false;
    handle->bufferDepth = 0;
    handle->pMaster = NULL;
}

void BAPE_Playback_Flush(
    BAPE_PlaybackHandle handle
    )
{
    uint32_t sourceChannelId;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);

    if ( handle->running )
    {
        BDBG_ASSERT(NULL != handle->pMaster);
        sourceChannelId = handle->pMaster->sourceChannelId[BAPE_ChannelPair_eLeftRight];
        /* Flush by making write pointer equal to read pointer */
        BREG_Write32(handle->deviceHandle->regHandle,
                     BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(sourceChannelId),
                     BREG_Read32(handle->deviceHandle->regHandle,
                                 BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId)));
    }

    handle->bufferDepth = 0;
}

BERR_Code BAPE_Playback_GetBuffer(
    BAPE_PlaybackHandle handle,
    void **pBuffer,                     /* [out] Address data should be written to (uncached) */
    unsigned *pBufferSize               /* [out] Contiguous buffer size in bytes */
    )
{   
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferSize);

    BDBG_MSG(("Playback %p (%d) GetBuffer", handle, handle->index));

    if ( handle->running && !handle->loopEnabled )
    {
        uint32_t rd,wr,base,sourceChannelId,rdaddr,wraddr,padding=1024,wrapSize=0;

        BDBG_ASSERT(NULL != handle->pMaster);
        sourceChannelId = handle->pMaster->sourceChannelId[BAPE_ChannelPair_eLeftRight];

        rd = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId));
        wr = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(sourceChannelId));
        base = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_BASEADDR_REG(sourceChannelId));

        BDBG_MSG(("PB GetBuffer: RDADDR 0x%x WRADDR 0x%x BASEADDR 0x%x Size %d (0x%x)", rd, wr, base, handle->bufferSize));

        rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);

        errCode = BMEM_ConvertOffsetToAddress(handle->deviceHandle->memHandle, wraddr, pBuffer);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        if ( wraddr > rdaddr )
        {
            *pBufferSize = handle->bufferSize - (wraddr-base);
            wrapSize = rdaddr-base;
        }
        else if ( wraddr < rdaddr )
        {
            *pBufferSize = rdaddr-wraddr;
        }
        else    /* equal */
        {
            if ( BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
                 BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP) )
            {
                /* Toggle bits are equal, buffer is empty. */
                *pBufferSize = handle->bufferSize - (wraddr-base);
                wrapSize = handle->bufferSize - *pBufferSize;
            }
            else
            {
                /* Toggle bit mismatch, buffer is full. */
                *pBufferSize = 0;
            }
        }

        /* Don't allow entire buffer to be filled, need gap between rd/wr for master/slave setups */
        if ( wrapSize > padding )
        {
            wrapSize -= padding;
            padding = 0;
        }
        else
        {
            padding -= wrapSize;
            wrapSize = 0;
        }
        if ( *pBufferSize > padding )
        {
            *pBufferSize = *pBufferSize - padding;
            padding = 0;
        }
        else
        {
            *pBufferSize = 0;
        }
        BDBG_MSG(("GET %u/%u", *pBufferSize, handle->bufferSize));
    }
    else
    {
        *pBufferSize = handle->bufferSize - handle->bufferDepth;
        if ( *pBufferSize )
        {
            *pBuffer = (void *)((unsigned)handle->pBuffer+handle->bufferDepth);
        }
        else
        {
            *pBuffer = NULL;
        }
    }

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return errCode;
}

BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    
    BDBG_MSG(("COMMIT %u", numBytes));
        
    if ( handle->running )
    {
        uint32_t rd,wr,base,sourceChannelId,rdaddr,wraddr;

        BDBG_ASSERT(NULL != handle->pMaster);
        sourceChannelId = handle->pMaster->sourceChannelId[BAPE_ChannelPair_eLeftRight];

        rd = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId));
        wr = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(sourceChannelId));
        base = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_BASEADDR_REG(sourceChannelId));

        BDBG_MSG(("PB Commit: RDADDR 0x%x WRADDR 0x%x BASEADDR 0x%x Size %d (0x%x)", rd, wr, base, handle->bufferSize));

        rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR);
        wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);

        /* Make sure the write pointer doesn't try and pass read */
        if ( rdaddr > wraddr )
        {
            if ( wraddr + numBytes > rdaddr )
            {
                BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            else if ( wraddr + numBytes == rdaddr )
            {
                if ( BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
                     BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP) )
                {
                    /* If the toggle bit is the same we will overflow. */
                    BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
        }
        else    /* rdaddr <= wraddr */
        {
            if ( wraddr + numBytes > (base + handle->bufferSize) )
            {
                BDBG_ERR(("Playback: Attempt to write more data than available in the buffer."));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }

        /* The request is legal.  Update the write pointer. */
        wraddr += numBytes;
        wr &= ~BCHP_MASK(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR);
        BDBG_MSG(("wraddr 0x%x handle->bufferSize 0x%x base 0x%x", wraddr, handle->bufferSize, base));
        if ( wraddr == handle->bufferSize + base )
        {
            BDBG_MSG(("Inverting toggle bit - was 0x%x now 0x%x", wr, wr ^ BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1)));
            wr ^= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP, 1);   /* flip the toggle bit */
            wraddr = base;
        }
        wr |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR, wraddr);
        BREG_Write32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(sourceChannelId), wr);
        BDBG_MSG(("Wrote 0x%x to WRADDR", wr));
    }
    else
    {
        if ( numBytes + handle->bufferDepth > handle->bufferSize )
        {
            BDBG_ERR(("Invalid number of bytes passed."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        handle->bufferDepth += numBytes;
    }

    BAPE_Playback_P_ReArmWatermark(handle);

    return BERR_SUCCESS;
}

void BAPE_Playback_GetStatus(
    BAPE_PlaybackHandle handle,
    BAPE_PlaybackStatus *pStatus    /* [out] */
    )
{
    uint32_t rd,wr,base,sourceChannelId,rdaddr,wraddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_PlaybackStatus));
    pStatus->fifoSize = handle->bufferSize;

    if (!handle->running)
    {
        return;
    } 

    BDBG_ASSERT(handle->pMaster);
    sourceChannelId = handle->pMaster->sourceChannelId[BAPE_ChannelPair_eLeftRight];

    rd = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId));
    wr = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(sourceChannelId));
    base = BREG_Read32(handle->deviceHandle->regHandle, BAPE_P_SOURCE_CHANNEL_TO_BASEADDR_REG(sourceChannelId));

    rdaddr = BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR) - base;
    wraddr = BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR) - base;

    if ( rdaddr > wraddr )
    {
        pStatus->queuedBytes = handle->bufferSize - rdaddr + wraddr;
    }
    else if ( wraddr > rdaddr )
    {
        pStatus->queuedBytes = wraddr - rdaddr;
    }
    else
    {
        if ( BCHP_GET_FIELD_DATA(rd, AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, RINGBUF_RDADDR_WRAP) ==
             BCHP_GET_FIELD_DATA(wr, AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, RINGBUF_WRADDR_WRAP) )
        {
            /* Toggle bit is the same, we're empty. */
            pStatus->queuedBytes = 0;
        }
        else
        {
            /* Toggle bit mismatch, we're full */
            pStatus->queuedBytes = handle->bufferSize;
        }
    }
}

void BAPE_Playback_GetConnector(
    BAPE_PlaybackHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    *pConnector = &handle->connector;
}

void BAPE_Playback_GetInterruptHandlers(
    BAPE_PlaybackHandle handle,
    BAPE_PlaybackInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_Playback_SetInterruptHandlers(
    BAPE_PlaybackHandle handle,
    const BAPE_PlaybackInterruptHandlers *pInterrupts
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);
    BDBG_ASSERT(NULL != pInterrupts);

    if ( handle->running )
    {
        errCode = BAPE_P_SetSourceChannelFreemarkInterrupt(handle->deviceHandle,
                                                           handle->connector.srcId[BAPE_ChannelPair_eLeftRight],
                                                           pInterrupts->watermark.pCallback_isr,
                                                           pInterrupts->watermark.pParam1,
                                                           pInterrupts->watermark.param2);        
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* These aren't actually used by the interrupt directly, so no need for critical section here. */
    handle->interrupts = *pInterrupts;

    return BERR_SUCCESS;
}

static void BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle handle)
{
    uint32_t regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);

    if ( handle->running )
    {
        BDBG_ASSERT(NULL != handle->pMaster);
        regVal = 1<<handle->pMaster->sourceChannelId[0];
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_AUD_FMM_BF_CTRL_REARM_FREEFULL_MARK, regVal);
    }
}

