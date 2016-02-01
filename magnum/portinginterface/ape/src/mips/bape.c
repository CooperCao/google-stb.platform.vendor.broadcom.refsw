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
#include "bchp_aud_fmm_misc.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_ms_ctrl.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"
#if BAPE_IMG_COMPILED
#include "bape_img.h"
#endif

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(bape);

BDBG_OBJECT_ID(BAPE_Device);
BDBG_OBJECT_ID(BAPE_BufferNode);

static BERR_Code BAPE_P_InitFmm(BAPE_Handle handle);
static BERR_Code BAPE_P_InitPll(BAPE_Handle handle);
static BERR_Code BAPE_P_InitMs(BAPE_Handle handle);

void BAPE_GetDefaultSettings(
    BAPE_Settings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    #if BAPE_IMG_COMPILED
    pSettings->pImageInterface = &BAPE_IMG_Interface;
    pSettings->pImageContext = BAPE_IMG_Context;
    #endif
    pSettings->rampPcmSamples = true;
    pSettings->highBitRateEnabled = true;
    pSettings->maxIndependentDelay = 300;
    pSettings->maxPcmSampleRate = 48000;
    pSettings->numCompressedBuffers = BAPE_CHIP_DEFAULT_NUM_COMPRESSED_BUFFERS;
    pSettings->numPcmBuffers = BAPE_CHIP_DEFAULT_NUM_PCM_BUFFERS;
}

BERR_Code BAPE_Open(
    BAPE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMEM_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    const BAPE_Settings *pSettings  /* NULL will use default settings */
    )
{
    BAPE_Settings defaultSettings;
    BAPE_Handle handle;
    BERR_Code errCode;
    BAPE_BufferNode *pNode;
    unsigned bufferSize;
    unsigned i;

    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(NULL != chpHandle);
    BDBG_ASSERT(NULL != regHandle);
    BDBG_ASSERT(NULL != memHandle);
    BDBG_ASSERT(NULL != intHandle);
    BDBG_ASSERT(NULL != tmrHandle);
    
    if ( NULL == pSettings )
    {
        BAPE_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if(
#if BAPE_FILE_IMAGE
            0 &&
#endif
            (pSettings->pImageInterface==NULL || pSettings->pImageContext==NULL)) {
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_settings;
    }
    

    /* Allocate device structure */
    handle = BKNI_Malloc(sizeof(BAPE_Device));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }
    /* Initialize structure */    
    BKNI_Memset(handle, 0, sizeof(BAPE_Device));
    BDBG_OBJECT_SET(handle, BAPE_Device);
    handle->pImageInterface = pSettings->pImageInterface;
    handle->pImageContext = pSettings->pImageContext;
    handle->chpHandle = chpHandle;
    handle->regHandle = regHandle;
    handle->memHandle = memHandle;
    handle->intHandle = intHandle;
    handle->tmrHandle = tmrHandle;
    handle->settings = *pSettings;
    BLST_S_INIT(&handle->pcmBufferList);
    BLST_S_INIT(&handle->compressedBufferList);
    BLST_S_INIT(&handle->inputGroupList);

    if ( false == pSettings->rampPcmSamples )
    {
        /* Should only be used for test purposes.  */
        BDBG_WRN(("PCM Sample Ramping is disabled in SRC.  This should only be done for test purposes."));
    }

#if BCHP_PWR_SUPPORT
    errCode = BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_APE_AIO);
    if ( errCode )
    {
        goto err_pwr;
    }
#endif

    errCode = BAPE_P_InitFmm(handle);
    if ( errCode )
    {
        goto err_fmm;
    }
    errCode = BAPE_P_InitInterrupts(handle);
    if ( errCode )
    {
        goto err_interrupt;
    }

    bufferSize = ((BAPE_CHIP_MAX_PATH_DELAY + pSettings->maxIndependentDelay) * (pSettings->maxPcmSampleRate/1000)) * BAPE_CHIP_BYTES_PER_PCM_SAMPLE_PAIR;
    handle->pcmBufferSize = bufferSize;
    BDBG_MSG(("Allocating %u PCM buffers (size %u bytes)", pSettings->numPcmBuffers, bufferSize));
    for ( i = 0; i < pSettings->numPcmBuffers; i++ )
    {
        pNode = BKNI_Malloc(sizeof(BAPE_BufferNode));
        if ( NULL == pNode )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_buffer;
        }
        pNode->pMemory = BMEM_AllocAligned(memHandle, bufferSize, 8, 0);
        if ( NULL == pNode->pMemory )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BKNI_Free(pNode);
            goto err_buffer;
        }
        BMEM_ConvertAddressToOffset(memHandle, pNode->pMemory, &pNode->offset);
        pNode->bufferSize = bufferSize;
        BDBG_OBJECT_SET(pNode, BAPE_BufferNode);
        BLST_S_INSERT_HEAD(&handle->pcmBufferList, pNode, node);
    }

    bufferSize = ((BAPE_CHIP_MAX_PATH_DELAY + pSettings->maxIndependentDelay) * (pSettings->maxPcmSampleRate/1000)) * BAPE_CHIP_BYTES_PER_COMPRESSED_SAMPLE;
    if ( pSettings->highBitRateEnabled )
    {
        bufferSize *= 4;    /* HBR = 4x default sample rate */
    }
    handle->compressedBufferSize = bufferSize;
    BDBG_MSG(("Allocating %u compressed buffers (size %u bytes)", pSettings->numCompressedBuffers, bufferSize));
    for ( i = 0; i < pSettings->numCompressedBuffers; i++ )
    {
        pNode = BKNI_Malloc(sizeof(BAPE_BufferNode));
        if ( NULL == pNode )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_buffer;
        }
        pNode->pMemory = BMEM_AllocAligned(memHandle, bufferSize, 8, 0);
        if ( NULL == pNode->pMemory )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BKNI_Free(pNode);
            goto err_buffer;
        }
        pNode->bufferSize = bufferSize;        
        BDBG_OBJECT_SET(pNode, BAPE_BufferNode);
        BLST_S_INSERT_HEAD(&handle->compressedBufferList, pNode, node);
        errCode = BMEM_ConvertAddressToOffset(memHandle, pNode->pMemory, &pNode->offset);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_buffer;
        }
    }
    
    BAPE_Dsp_P_GetDefaultSettings(&handle->dspSettings);
    handle->dspSettings.pImageInterface = pSettings->pImageInterface;
    handle->dspSettings.pImageContext = pSettings->pImageContext;
    handle->dspSettings.readReserved = pSettings->readReserved;
    handle->dspSettings.writeReserved = pSettings->writeReserved;
    handle->dspSettings.readCoreRegister = pSettings->readCoreRegister;
    handle->dspSettings.writeCoreRegister = pSettings->writeCoreRegister;
    handle->dspSettings.readCmtControl = pSettings->readCmtControl;
    handle->dspSettings.writeCmtControl = pSettings->writeCmtControl;
    handle->dspSettings.pParam1 = pSettings->pParam1;
    handle->dspSettings.param2 = pSettings->param2;
    handle->dspSettings.heapSize = 4*1024*1024;         /* TODO: Optimize this */
    handle->dspSettings.stackSize = 32*1024;            /* TODO: Optimize this */
    handle->dspSettings.sharedMemorySize = 512*1024;    /* TODO: Optimize this (should be 1K alligned and larger then sizeof(BAPE_DspIpcBlock) */
    errCode = BAPE_Dsp_P_Open(handle, &handle->dspSettings, &handle->dsp);
    if ( errCode )
    {
        goto err_dsp;
    }

    /* Success */
    *pHandle = handle;

    return BERR_SUCCESS;

err_dsp:
err_buffer:
    /* Remove and free all pcm and compressed buffers and nodes */
    while ( (pNode = BLST_S_FIRST(&handle->pcmBufferList)) )
    {
        BLST_S_REMOVE_HEAD(&handle->pcmBufferList, node);
        BMEM_Free(memHandle, pNode->pMemory);
        BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
        BKNI_Free(pNode);
    }
    while ( (pNode = BLST_S_FIRST(&handle->compressedBufferList)) )
    {
        BLST_S_REMOVE_HEAD(&handle->compressedBufferList, node);
        BMEM_Free(memHandle, pNode->pMemory);
        BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
        BKNI_Free(pNode);
    }
    BAPE_P_UninitInterrupts(handle);
err_interrupt:    
err_fmm:
#if BCHP_PWR_SUPPORT
err_pwr:
#endif
    BDBG_OBJECT_DESTROY(handle, BAPE_Device);
    BKNI_Free(handle);
err_handle:
err_settings:
    *pHandle = NULL;
    return errCode;
}

void BAPE_Close(
    BAPE_Handle handle
    )
{
    unsigned i=0;
    BAPE_BufferNode *pNode;
    #if 0 /* TODO */
    BAPE_InputGroupHandle group;
    #endif

    /* Stop all potential mixer inputs first */
#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( handle->decoders[i] )
        {
            BDBG_MSG(("Stopping decoder %p (%d)", handle->decoders[i], i));
            BAPE_Decoder_Stop(handle->decoders[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( handle->inputCaptures[i] )
        {
            BDBG_MSG(("Stopping input capture %p (%d)", handle->inputCapture[i], i));
            BAPE_InputCapture_Stop(handle->inputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( handle->playbacks[i] )
        {
            BDBG_MSG(("Stopping playback %p (%d)", handle->playbacks[i], i));
            BAPE_Playback_Stop(handle->playbacks[i]);
        }
    }
#endif

    /* Close all mixers next */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        if ( handle->mixers[i] )
        {
            BDBG_MSG(("Closing mixer %p (%d)", handle->mixers[i], i));
            BAPE_Mixer_Close(handle->mixers[i]);
        }
    }

#if 0 /* TODO */
    /* Close all input groups next */
    while ( (group = BLST_S_FIRST(&handle->inputGroupList)) )
    {
        BDBG_MSG(("Closing input group %p", group));
        BAPE_InputGroup_Destroy(group);
    }
#endif

    /* Close all inputs */
#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( handle->decoders[i] )
        {
            BDBG_MSG(("Closing decoder %p (%d)", handle->decoders[i], i));
            BAPE_Decoder_Close(handle->decoders[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( handle->inputCaptures[i] )
        {
            BDBG_MSG(("Closing input capture %p (%d)", handle->inputCapture[i], i));
            BAPE_InputCapture_Close(handle->inputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( handle->playbacks[i] )
        {
            BDBG_MSG(("Closing playback %p (%d)", handle->playbacks[i], i));
            BAPE_Playback_Close(handle->playbacks[i]);
        }
    }
#endif

    /* Close all input and output ports */
#if BAPE_CHIP_MAX_DACS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DACS; i++ )
    {
        if ( handle->dacs[i] )
        {
            BDBG_MSG(("Closing DAC %p (%d)", handle->dacs[i], i));
            BAPE_Dac_Close(handle->dacs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_I2S_OUTPUTS; i++ )
    {
        if ( handle->i2sOutputs[i] )
        {
            BDBG_MSG(("Closing I2S Output %p (%d)", handle->i2sOutputs[i], i));
            BAPE_I2sOutput_Close(handle->i2sOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_SPDIF_OUTPUTS; i++ )
    {
        if ( handle->spdifOutputs[i] )
        {
            BDBG_MSG(("Closing SPDIF Output %p (%d)", handle->spdifOutputs[i], i));
            BAPE_SpdifOutput_Close(handle->spdifOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_MAI_OUTPUTS; i++ )
    {
        if ( handle->maiOutputs[i] )
        {
            BDBG_MSG(("Closing MAI Output %p (%d)", handle->maiOutputs[i], i));
            BAPE_MaiOutput_Close(handle->maiOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_OUTPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_OUTPUT_CAPTURES; i++ )
    {
        if ( handle->outputCaptures[i] )
        {
            BDBG_MSG(("Closing Output Capture %p (%d)", handle->outputCaptures[i], i));
            BAPE_OutputCapture_Close(handle->outputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_I2S_INPUTS; i++ )
    {
        if ( handle->i2sInputs[i] )
        {
            BDBG_MSG(("Closing I2S Input %p (%d)", handle->i2sInputs[i], i));
            BAPE_I2sInput_Close(handle->i2sInputs[i]);
        }
    }
#endif

    /* Remove and free all pcm and compressed buffers and nodes */
    while ( (pNode = BLST_S_FIRST(&handle->pcmBufferList)) )
    {
        BLST_S_REMOVE_HEAD(&handle->pcmBufferList, node);
        BMEM_Free(handle->memHandle, pNode->pMemory);
        BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
        BKNI_Free(pNode);
    }
    while ( (pNode = BLST_S_FIRST(&handle->compressedBufferList)) )
    {
        BLST_S_REMOVE_HEAD(&handle->compressedBufferList, node);
        BMEM_Free(handle->memHandle, pNode->pMemory);
        BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
        BKNI_Free(pNode);
    }

    BAPE_Dsp_P_Close(handle->dsp);

    BAPE_P_UninitInterrupts(handle);

#if BCHP_PWR_SUPPORT
    BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_APE_AIO);
#endif

    BDBG_OBJECT_DESTROY(handle, BAPE_Device);
    BKNI_Free(handle);
}

const char *BAPE_P_GetSampleRateString_isrsafe(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:    /* 32K Sample rate */
        return "32 kHz";
    case BAVC_AudioSamplingRate_e44_1k:    /* 44.1K Sample rate */
        return "44.1 kHz";
    case BAVC_AudioSamplingRate_e48k:      /* 48K Sample rate */
        return "48 kHz";
    case BAVC_AudioSamplingRate_e96k:      /* 96K Sample rate */
        return "96 kHz";
    case BAVC_AudioSamplingRate_e16k:      /* 16K Sample rate */
        return "16 kHz";
    case BAVC_AudioSamplingRate_e22_05k:   /* 22.05K Sample rate */
        return "22.05 kHz";
    case BAVC_AudioSamplingRate_e24k:      /* 24K Sample rate */
        return "24 kHz";
    case BAVC_AudioSamplingRate_e64k:      /* 64K Sample rate */
        return "64 kHz";
    case BAVC_AudioSamplingRate_e88_2k:    /* 88.2K Sample rate */
        return "88.2 kHz";
    case BAVC_AudioSamplingRate_e128k:     /* 128K Sample rate */
        return "128 kHz";
    case BAVC_AudioSamplingRate_e176_4k:   /* 176.4K Sample rate */
        return "176.4 kHz";
    case BAVC_AudioSamplingRate_e192k:     /* 192K Sample rate */
        return "192 kHz";
    case BAVC_AudioSamplingRate_e8k:       /* 8K Sample rate */
        return "8 kHz";
    case BAVC_AudioSamplingRate_e12k:      /* 12K Sample rate */
        return "12 kHz";
    case BAVC_AudioSamplingRate_e11_025k:  /* 11.025K Sample rate */
        return "11.025 kHz";
    default:
        return "Unknown";
    }
}

unsigned BAPE_P_GetSampleRateValue_isrsafe(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:    /* 32K Sample rate */
        return 32000;
    case BAVC_AudioSamplingRate_e44_1k:    /* 44.1K Sample rate */
        return 44100;
    case BAVC_AudioSamplingRate_e48k:      /* 48K Sample rate */
        return 48000;
    case BAVC_AudioSamplingRate_e96k:      /* 96K Sample rate */
        return 96000;
    case BAVC_AudioSamplingRate_e16k:      /* 16K Sample rate */
        return 16000;
    case BAVC_AudioSamplingRate_e22_05k:   /* 22.05K Sample rate */
        return 22050;
    case BAVC_AudioSamplingRate_e24k:      /* 24K Sample rate */
        return 24000;
    case BAVC_AudioSamplingRate_e64k:      /* 64K Sample rate */
        return 64000;
    case BAVC_AudioSamplingRate_e88_2k:    /* 88.2K Sample rate */
        return 88200;
    case BAVC_AudioSamplingRate_e128k:     /* 128K Sample rate */
        return 128000;
    case BAVC_AudioSamplingRate_e176_4k:   /* 176.4K Sample rate */
        return 176400;
    case BAVC_AudioSamplingRate_e192k:     /* 192K Sample rate */
        return 192000;
    case BAVC_AudioSamplingRate_e8k:       /* 8K Sample rate */
        return 8000;
    case BAVC_AudioSamplingRate_e12k:      /* 12K Sample rate */
        return 12000;
    case BAVC_AudioSamplingRate_e11_025k:  /* 11.025K Sample rate */
        return 11025;
    default:
        return 0;
    }
}

BAVC_AudioSamplingRate BAPE_P_GetSampleRateEnum_isrsafe(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 32000:
        return BAVC_AudioSamplingRate_e32k;    /* 32K Sample rate */
    case 44100:
        return BAVC_AudioSamplingRate_e44_1k;    /* 44.1K Sample rate */
    case 48000:
        return BAVC_AudioSamplingRate_e48k;      /* 48K Sample rate */
    case 96000:
        return BAVC_AudioSamplingRate_e96k;      /* 96K Sample rate */
    case 16000:
        return BAVC_AudioSamplingRate_e16k;      /* 16K Sample rate */
    case 22050:
        return BAVC_AudioSamplingRate_e22_05k;   /* 22.05K Sample rate */
    case 24000:
        return BAVC_AudioSamplingRate_e24k;      /* 24K Sample rate */
    case 64000:
        return BAVC_AudioSamplingRate_e64k;      /* 64K Sample rate */
    case 88200:
        return BAVC_AudioSamplingRate_e88_2k;    /* 88.2K Sample rate */
    case 128000:
        return BAVC_AudioSamplingRate_e128k;     /* 128K Sample rate */
    case 176400:
        return BAVC_AudioSamplingRate_e176_4k;   /* 176.4K Sample rate */
    case 192000:
        return BAVC_AudioSamplingRate_e192k;     /* 192K Sample rate */
    case 8000:
        return BAVC_AudioSamplingRate_e8k;       /* 8K Sample rate */
    case 12000:
        return BAVC_AudioSamplingRate_e12k;      /* 12K Sample rate */
    case 11025:
        return BAVC_AudioSamplingRate_e11_025k;  /* 11.025K Sample rate */
    default:
        return BAVC_AudioSamplingRate_eUnknown;
    }
}

unsigned BAPE_P_GetSampleRateCstatCode_isrsafe(BAVC_AudioSamplingRate sampleRate)
{
    switch ( sampleRate )
    {
    case BAVC_AudioSamplingRate_e32k:      /* 32K Sample rate */
        return 0x3;
    case BAVC_AudioSamplingRate_e44_1k:    /* 44.1K Sample rate */
        return 0x0;
    case BAVC_AudioSamplingRate_e48k:      /* 48K Sample rate */
        return 0x2;
    case BAVC_AudioSamplingRate_e96k:      /* 96K Sample rate */
        return 0xc;
    case BAVC_AudioSamplingRate_e22_05k:   /* 22.05K Sample rate */
        return 0x4;
    case BAVC_AudioSamplingRate_e24k:      /* 24K Sample rate */
        return 0x6;
    case BAVC_AudioSamplingRate_e88_2k:    /* 88.2K Sample rate */
        return 0x8;
    case BAVC_AudioSamplingRate_e176_4k:   /* 176.4K Sample rate */
        return 0xc;
    case BAVC_AudioSamplingRate_e192k:     /* 192K Sample rate */
        return 0xe;
    default:
        return 0x8; /* not indicated */
    }
}

static BERR_Code BAPE_P_InitFmm(BAPE_Handle handle)
{
    BREG_Handle regHandle;
    uint32_t regVal, regAddr, endAddr;
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    /* Assert toplevel reset */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MISC_RESET, 0);

    BDBG_MSG(("Resetting FMM"));
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MISC_RESET);
    regVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_TOP_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_REGS_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_REGS_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_LOGIC_B, Inactive)) |
              (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_REGS_B, Inactive));
    BREG_Write32(regHandle, BCHP_AUD_FMM_MISC_RESET, regVal);

    /* Clear Ringbuffer registers */
    regAddr = BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
    endAddr = regAddr + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR - BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR) * (BAPE_CHIP_MAX_SOURCE_CHANNELS + BAPE_CHIP_MAX_DESTINATION_CHANNELS));
    #ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FRMSTADDR
    endAddr += ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_FRMSTADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FRMSTADDR) * (BAPE_CHIP_MAX_SOURCE_CHANNELS + BAPE_CHIP_MAX_DESTINATION_CHANNELS));
    #endif
    BDBG_MSG(("Clearing ringbuffer registers from 0x%x to 0x%x", regAddr, endAddr-4));
    while ( regAddr < endAddr )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }
    BDBG_MSG(("Resetting source channel group IDs to default"));
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE;
    for ( i = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_START; i <= BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_END; i++ )
    {
        regVal = BREG_Read32(regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_GRPi, GROUP_ID, i);
        BREG_Write32(regHandle, regAddr, regVal);
        regAddr += (BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_ELEMENT_SIZE/8);
    }
    BDBG_MSG(("Clearing all DP registers"));
    #if 0
    for ( regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER_OUTPUT_ENA; regAddr < BCHP_AUD_FMM_DP_CTRL0_DP_SM; regAddr += 4 )
    {
        BREG_Write32(regHandle, regAddr, 0);
    }
    #endif
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_BASE;
    for ( i = BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_START; i <= BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += (BCHP_AUD_FMM_DP_CTRL0_PING_COEFFICIENTSi_ARRAY_ELEMENT_SIZE/8);
    }
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_BASE;
    for( i=BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_START; i<=BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEPi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, 0x10000);
        regAddr += 4;
    }
    #else
    BREG_Write32(regHandle, BCHP_AUD_FMM_DP_CTRL0_PING_COEFF_RAMP_STEP, 0x10000);
    #endif
    #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE
    regAddr = BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_BASE;
    for ( i = BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_START; i <= BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += (BCHP_AUD_FMM_DP_CTRL0_PONG_COEFFICIENTSi_ARRAY_ELEMENT_SIZE/8);
    }
     #ifdef BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE
        regAddr = BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_BASE;
        for( i=BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_START; i<=BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEPi_ARRAY_END; i++ )
        {
            BREG_Write32(regHandle, regAddr, 0x10000);
            regAddr += 4;
        }
        #else
        BREG_Write32(regHandle, BCHP_AUD_FMM_DP_CTRL0_PONG_COEFF_RAMP_STEP, 0x10000);
     #endif
    #endif
    for ( regAddr = BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL; regAddr < BCHP_AUD_FMM_DP_CTRL0_MIXER00_LT_VOL_LEVEL+(16*BAPE_CHIP_MAX_MIXERS); regAddr += 4 )
    {
        BREG_Write32(regHandle, regAddr, 0);
    }
    #if 0 /* don't mess with soft_coef, it's unused defined BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_BASE */
    for ( regAddr = BCHP_AUD_FMM_DP_CTRL0_SOFT_COEFCi_ARRAY_BASE; regAddr < BCHP_AUD_FMM_DP_CTRL0_SOFT_RESERVED_i_ARRAY_BASE; regAddr += 4 )
    {
        BREG_Write32(regHandle, regAddr, 0);
    }
    #endif
    BDBG_MSG(("Resetting mixer group IDs to default"));
    for ( i = 0; i < BAPE_CHIP_MAX_MIXERS; i++ )
    {
        regAddr = BAPE_P_GetMixerConfigAddress(i);
        regVal = BREG_Read32(regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_MIXER0_CONFIG, MIXER_GROUP_BEGIN, i);
        BREG_Write32(regHandle, regAddr, regVal);
    }
    BDBG_MSG(("Setting IOP Capture group IDs to default"));
    regAddr = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_BASE;
    for ( i = BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_START; i <= BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_END; i++ )
    {
        regVal = BREG_Read32(regHandle, regAddr);
        regVal &= ~BCHP_MASK(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_CTRL_CAP_CFGi, GROUP, i);
        BREG_Write32(regHandle, regAddr, regVal);
        regAddr += BCHP_AUD_FMM_IOP_CTRL_CAP_CFGi_ARRAY_ELEMENT_SIZE/8;
    }
    BDBG_MSG(("Setting RAMP STEP to default"));
    BREG_Write32(regHandle, BCHP_AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, BCHP_FIELD_DATA(AUD_FMM_DP_CTRL0_VOLUME_RAMP_STEP, VOLUME_RAMP_STEP, 0x200));
    /* Initialize PLLs */
    BDBG_MSG(("Initializing PLLs"));
    errCode = BAPE_P_InitPll(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    /* Initialize Micro Sequencer Registers */
    BDBG_MSG(("Initializing MS registers"));
    errCode = BAPE_P_InitMs(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BDBG_MSG(("Initializing SRC registers"));
    errCode = BAPE_P_InitSrc(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_InitMs(BAPE_Handle handle)
{
    unsigned i;
    uint32_t regVal, regAddr, endAddr;
    BREG_Handle regHandle;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    /* Initialize the two address blocks to zero */
    regAddr = BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0;
    endAddr = BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + (BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END*(BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_ELEMENT_SIZE/8));
    while ( regAddr <= endAddr )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }
    regAddr = BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE;
    for ( i = 0; i <= BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, 0);
        regAddr += 4;
    }

    /* Download the firmware */
    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to  3 */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 1 */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 1);

    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 2 
             to enable internal access to instruction memory */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Instruction_memory);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Copy the microcode to the F/W instruction memory */
    regAddr = BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE;
    for ( i=BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_START; i <= BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END; i++ )
    {
        BREG_Write32(regHandle, regAddr, g_BAPE_MS_Firmware[i]);
        regAddr += 4;
    }

    /* Now Reset Pico */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, Configuration_bits);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

    /* Program AUD_FMM_MS_CTRL_USEQ_INST+4: INST to 0 */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_BASE + 4, 0);

    /* Program AUD_FMM_MS_CTRL_USEQ_CTRL: CFG_CTRL to 0 
             to enable internal access to block external access */
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL);
    regVal &= ~BCHP_MASK(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_MS_CTRL_USEQ_CTRL, CFG_CTRL, No_external_access);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_USEQ_CTRL, regVal);

#define BAPE_CBIT_BUFFER_SIZE 12
    /* Initializing cBits */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE,  0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 4, 0x200);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + BAPE_CBIT_BUFFER_SIZE * 4 + 4, 0x200);        

    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 2*BAPE_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BAPE_CBIT_BUFFER_SIZE *4,   0x4);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + 3*BAPE_CBIT_BUFFER_SIZE * 4 + 4,   0x200);        

    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0,   0x50);
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1,   0x68);

    /*Initializing ramp amount*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0,  0x0);           
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_1,  0x0);           

    /*Initializing Stream CTRL*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_0, 0x112);                   
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_STREAM_CTRL_1, 0x112);       

    /*Initializing burst*/
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_BURST_0, 0x0);       
    BREG_Write32(regHandle, BCHP_AUD_FMM_MS_CTRL_FW_BURST_1, 0x0);       

    /* Initializing SPDIF cfg */
    BREG_Write32(regHandle, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0, 0xb);

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_InitPll(BAPE_Handle handle)
{
    int i;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_PLLS; i++ )
    {
        BAPE_PllSettings pllSettings;
        pllSettings.freeRun = false;
        pllSettings.vcxo = 0;
        errCode = BAPE_Pll_SetSettings(handle, i, &pllSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return errCode;
}

void BAPE_GetInterruptHandlers(
    BAPE_Handle handle,
    BAPE_InterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_SetInterruptHandlers(
    BAPE_Handle handle,
    const BAPE_InterruptHandlers *pInterrupts
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();
    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}

BERR_Code BAPE_ProcessWatchdogInterrupt(
    BAPE_Handle handle
    )
{
#if BAPE_CHIP_MAX_DECODERS
    int i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    if ( handle->dsp )
    {
        BERR_Code errCode;

        /* Stop all running decoders */
        for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
        {
            if ( handle->decoders[i] )
            {
                handle->decoderWatchdogInfo[i].state = handle->decoders[i]->state;
                if ( handle->decoderWatchdogInfo[i].state != BAPE_DecoderState_eStopped )
                {
                    handle->decoderWatchdogInfo[i].startSettings = handle->decoders[i]->startSettings;
                    BAPE_Decoder_Stop(handle->decoders[i]);
                }
                BAPE_DspContext_P_Destroy(handle->decoders[i]->dspContext);
                handle->decoders[i]->dspContext = NULL;
            }
            else
            {
                handle->decoderWatchdogInfo[i].state = BAPE_DecoderState_eMax;
            }
        }

        /* Reboot the DSP */
        errCode = BAPE_Dsp_P_ProcessWatchdog(handle->dsp);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }
        BAPE_Dsp_P_Close(handle->dsp);
        errCode = BAPE_Dsp_P_Open(handle, &handle->dspSettings, &handle->dsp);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        
        /* Reset all decoder state */
        for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
        {
            if ( handle->decoders[i] )
            {
                errCode = BAPE_DspContext_P_Create(handle->dsp, handle->decoders[i], &handle->decoders[i]->dspContext);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
                if ( handle->decoderWatchdogInfo[i].state != BAPE_DecoderState_eStopped )
                {
                    /* Restart Decoder */
                    errCode = BAPE_Decoder_Start(handle->decoders[i], &handle->decoderWatchdogInfo[i].startSettings);
                    if ( errCode )
                    {
                        BDBG_ERR(("Error restarting decoder %d", i));
                        errCode = BERR_TRACE(errCode);
                    }
                    
                    if ( handle->decoderWatchdogInfo[i].state == BAPE_DecoderState_ePaused )
                    {
                        errCode = BAPE_Decoder_Pause(handle->decoders[i]);
                        if ( errCode )
                        {
                            BDBG_ERR(("Error re-pausing decoder %d", i));
                            errCode = BERR_TRACE(errCode);
                        }
                    }
                }
            }
        }
    }
#else
    BSTD_UNUSED(handle);
#endif

    return BERR_SUCCESS;
}

uint32_t BAPE_P_GetMixerConfigAddress(unsigned mixerId)
{
    switch ( mixerId )
    {
    #if BCHP_CHIP == 7408
    case 5:
        /* 7408 made this discontiguous for some reason... */
        return BCHP_AUD_FMM_DP_CTRL0_MIXER5_CONFIG;
    #endif
    default:
        BDBG_ASSERT(mixerId < BAPE_CHIP_MAX_MIXERS);
        return BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG + ((BCHP_AUD_FMM_DP_CTRL0_MIXER1_CONFIG-BCHP_AUD_FMM_DP_CTRL0_MIXER0_CONFIG)*mixerId);
    }
}

void BAPE_GetDecoderPathDelay(
    BAPE_Handle handle,
    unsigned *pDelay    /* [out] in ms */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(NULL != pDelay);
    *pDelay = BAPE_CHIP_MAX_PATH_DELAY;
}

BERR_Code BAPE_Standby(BAPE_Handle handle)
{
#if BCHP_PWR_SUPPORT
    BERR_Code rc;
    
    rc = BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_APE_AIO);

    return rc;
#else
    BSTD_UNUSED(handle);
    return BERR_SUCCESS;
#endif
}

BERR_Code BAPE_Resume(BAPE_Handle handle)
{
#if BCHP_PWR_SUPPORT
    BERR_Code rc;
    
    rc = BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_APE_AIO);

    return rc;
#else
    BSTD_UNUSED(handle);
    return BERR_SUCCESS;
#endif
}
