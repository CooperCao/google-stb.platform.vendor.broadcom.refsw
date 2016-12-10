/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio PI Device Level Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clkgen.h"
#if BAPE_DSP_SUPPORT
#include "bdsp_raaga.h"
#endif

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


BDBG_MODULE(bape);
BDBG_FILE_MODULE(bape_algos);
BDBG_FILE_MODULE(bape_mem);

BDBG_OBJECT_ID(BAPE_Device);
BDBG_OBJECT_ID(BAPE_BufferNode);

#if BCHP_PWR_SUPPORT
static BERR_Code BAPE_P_StandbyAIO(BAPE_Handle handle);
#endif
static BERR_Code BAPE_P_ResetAIO(BAPE_Handle handle);
static BERR_Code BAPE_P_InitFmmSw(BAPE_Handle handle);
static BERR_Code BAPE_P_InitFmmHw(BAPE_Handle handle);
static BERR_Code BAPE_P_InitTimers(BAPE_Handle handle);
static void BAPE_P_DestroyTimers(BAPE_Handle handle);
static unsigned BAPE_P_CalculateBufferSize(const BAPE_Settings * pSettings, unsigned pool, unsigned baseBufferSize);
static unsigned BAPE_P_GetNumberOfBuffers(const BAPE_Settings * pSettings, unsigned pool);

void BAPE_GetDefaultSettings(
    BAPE_Settings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->maxDspTasks = BAPE_CHIP_MAX_DSP_TASKS;
    pSettings->maxArmTasks = BAPE_CHIP_MAX_ARM_TASKS;
    pSettings->rampPcmSamples = true;
    pSettings->maxIndependentDelay = 0;
    pSettings->maxPcmSampleRate = 48000;
    pSettings->numPcmBuffers = BAPE_CHIP_DEFAULT_NUM_PCM_BUFFERS;
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 &&
         BAPE_P_GetDolbyMS12Config() == BAPE_DolbyMs12Config_eA )
    {
        pSettings->numPcmBuffers += 1;
    }
    pSettings->numCompressedBuffers = BAPE_CHIP_DEFAULT_NUM_COMPRESSED_BUFFERS;
    pSettings->numCompressed4xBuffers = BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS;
    pSettings->numCompressed16xBuffers = BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS;
    /*pSettings->numRfEncodedPcmBuffers = 0; Implicit with memset(0) */
}

#if BAPE_DSP_SUPPORT
#define BAPE_P_COMPUTE_RBUF_SIZE(maxDelay)   \
    ((BDSP_AF_P_MAX_BLOCKING_TIME * BDSP_AF_P_MAX_SAMPLING_RATE \
    + BDSP_AF_P_MAX_THRESHOLD + BDSP_AF_P_SAMPLE_PADDING  \
    + (maxDelay) * BDSP_AF_P_MAX_SAMPLING_RATE) \
    * 4)                     /* 4 to make in bytes */
#else
#define BAPE_P_COMPUTE_RBUF_SIZE(maxDelay)   (128*1024)
#endif

#define BAPE_P_BUFFER_POOL_TYPE_TO_STRING(pool) \
    (pool==3) ? "compressed 16x / HBR" : \
    (pool==2) ? "compressed 4x / PCM RF" : \
    (pool==1) ? "compressed" : \
    "PCM"

/***************************************************************************
Summary:
Get an estimate of the memory required by APE
***************************************************************************/
BERR_Code BAPE_GetMemoryEstimate(
    const BAPE_Settings *pSettings,            /* [in] - required */
    BAPE_MemoryEstimate *pEstimate             /* [out] */
    )
{
    unsigned bufferSize, numBuffers, i;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pEstimate);

    BKNI_Memset(pEstimate, 0, sizeof(*pEstimate));

    for ( i=0; i<BAPE_MAX_BUFFER_POOLS; i++ )
    {
        /* There are four classes of buffer pools.  PCM buffers, compressed 1x, compressed 4x/RF, compressed 16x. */
        bufferSize = BAPE_P_CalculateBufferSize(pSettings, i, BAPE_P_COMPUTE_RBUF_SIZE(pSettings->maxIndependentDelay));
        numBuffers = BAPE_P_GetNumberOfBuffers(pSettings, i);

        BDBG_MSG(("MemEst - %u %s buffers (size %u bytes), total %u bytes", numBuffers, BAPE_P_BUFFER_POOL_TYPE_TO_STRING(i), bufferSize, (numBuffers * bufferSize)));

        pEstimate->general += (numBuffers * bufferSize);
    }

    return BERR_SUCCESS;
}

static unsigned BAPE_P_CalculateBufferSize(
    const BAPE_Settings * pSettings,
    unsigned pool,
    unsigned baseBufferSize
    )
{
    unsigned bufferSize;

    /* There are four classes of buffer pools.  PCM buffers, compressed 1x, compressed 4x/RF, compressed 16x. */
    switch ( pool )
    {
    case 0: /* PCM */
        bufferSize = 2*baseBufferSize;      /* One buffer for 32-bit L samples, one for 32-bit R samples */
        if ( pSettings->maxPcmSampleRate > 48000 && pSettings->maxPcmSampleRate <= 96000 )
        {
            bufferSize *= 2;
        }
        else if ( pSettings->maxPcmSampleRate > 96000 )
        {
            bufferSize *= 4;
        }
        break;
    case 1: /* Compressed */
        bufferSize = baseBufferSize;        /* One buffer for 16-bit interleaved L/R sampeles */
        break;
    case 2: /* 4x Compressed */
        bufferSize = 4*baseBufferSize;      /* One buffer for 16-bit interleaved L/R sampeles (4x samplerate) */
        break;
    case 3: /* 16x Compressed */
        #if BAPE_DSP_SUPPORT
        bufferSize = BDSP_AF_P_MAX_16X_BUF_SIZE + (16*(baseBufferSize-BAPE_P_COMPUTE_RBUF_SIZE(0)));    /* MLP requires a 1.5MB buffer to handle bursty delivery.
                                                                                    Additionally, we need an independent delay size 16x a standard buffer (768kHz) */
        #else
        bufferSize = 16*baseBufferSize;
        #endif
        break;
    default:
        bufferSize = 0;
        BDBG_ERR(("Invalid buffer pool type %u", pool));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    return bufferSize;
}

static unsigned BAPE_P_GetNumberOfBuffers(
    const BAPE_Settings * pSettings,
    unsigned pool
    )
{
    unsigned numBuffers;

    switch ( pool )
    {
    case 0: /* PCM */
        numBuffers = pSettings->numPcmBuffers;
        break;
    case 1: /* Compressed */
        numBuffers = pSettings->numCompressedBuffers;
        break;
    case 2: /* 4x Compressed */
        numBuffers = pSettings->numCompressed4xBuffers + pSettings->numRfEncodedPcmBuffers;
        break;
    case 3: /* 16x Compressed */
        numBuffers = pSettings->numCompressed16xBuffers;
        break;
    default:
        numBuffers = 0;
        BDBG_ERR(("Invalid buffer pool type %u", pool));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    return numBuffers;
}

BERR_Code BAPE_Open(
    BAPE_Handle *pHandle,   /* [out] returned handle */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMEM_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    BDSP_Handle dspHandle,
    BDSP_Handle armHandle,
    const BAPE_Settings *pSettings  /* NULL will use default settings */
    )
{
    BAPE_Settings defaultSettings;
    BAPE_Handle handle;
    BERR_Code errCode;
    BAPE_BufferNode *pNode;
    unsigned bufferSize, numBuffers;
    unsigned i, buffer;

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
    handle->chpHandle = chpHandle;
    handle->regHandle = regHandle;
    handle->memHandle = memHandle;
    handle->intHandle = intHandle;
    handle->tmrHandle = tmrHandle;
    handle->dspHandle = dspHandle;
    handle->armHandle = armHandle;
    handle->settings = *pSettings;
    BLST_S_INIT(&handle->mixerList);

    if ( false == pSettings->rampPcmSamples )
    {
        /* Should only be used for test purposes.  */
        BDBG_WRN(("PCM Sample Ramping is disabled in SRC.  This should only be done for test purposes."));
    }

#ifdef BCHP_PWR_RESOURCE_AUD_AIO
    BCHP_PWR_AcquireResource(chpHandle, BCHP_PWR_RESOURCE_AUD_AIO);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
    BCHP_PWR_AcquireResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL0);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
    BCHP_PWR_AcquireResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL1);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
    BCHP_PWR_AcquireResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL2);
#endif

    errCode = BAPE_P_InitFmmSw(handle);
    if ( errCode )
    {
        goto err_fmm;
    }

    errCode = BAPE_P_InitFmmHw(handle);
    if ( errCode )
    {
        goto err_fmm;
    }

    errCode = BAPE_P_InitInterrupts(handle);
    if ( errCode )
    {
        goto err_interrupt;
    }

    /* Buffer Allocations */
    for ( i = 0; i < BAPE_MAX_BUFFER_POOLS; i++ )
    {
        static const BAPE_DataType validTypes[BAPE_MAX_BUFFER_POOLS][BAPE_DataType_eMax] =
        {
            {BAPE_DataType_ePcmStereo, BAPE_DataType_ePcm5_1, BAPE_DataType_ePcm7_1, BAPE_DataType_eMax},
            {BAPE_DataType_eIec61937, BAPE_DataType_eMax},
            {BAPE_DataType_eIec61937, BAPE_DataType_eIec61937x4, BAPE_DataType_ePcmRf, BAPE_DataType_eMax},
            {BAPE_DataType_eIec61937, BAPE_DataType_eIec61937x4, BAPE_DataType_ePcmRf, BAPE_DataType_eIec61937x16, BAPE_DataType_eMax}
        };
        static const BAPE_DataSource validSources[] =
        {
            BAPE_DataSource_eDspBuffer, BAPE_DataSource_eHostBuffer, BAPE_DataSource_eDfifo, BAPE_DataSource_eFci, BAPE_DataSource_eMax
        };
        BLST_S_INIT(&handle->buffers[i].freeList);
        BLST_S_INIT(&handle->buffers[i].allocatedList);
        BAPE_FMT_P_InitCapabilities(&handle->buffers[i].capabilities, validSources, &validTypes[i][0]);
        bufferSize = BAPE_P_CalculateBufferSize(pSettings, i, BAPE_P_COMPUTE_RBUF_SIZE(pSettings->maxIndependentDelay));
        numBuffers = BAPE_P_GetNumberOfBuffers(pSettings, i);
        BDBG_MSG(("Allocating %u %s buffers (size %u bytes)", numBuffers, BAPE_P_BUFFER_POOL_TYPE_TO_STRING(i), bufferSize));

        handle->buffers[i].bufferSize = bufferSize;
        handle->buffers[i].numFreeBuffers = numBuffers;
        #if BDBG_DEBUG_BUILD
        handle->buffers[i].numBuffers = numBuffers;
        handle->buffers[i].maxUsed = 0;
        #endif

        for ( buffer = 0; buffer < numBuffers; buffer++ )
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
            BDBG_OBJECT_SET(pNode, BAPE_BufferNode);
            BDBG_MSG(("Created Buffer Node %p in pool %u", (void *)pNode, i));
            BLST_S_INSERT_HEAD(&handle->buffers[i].freeList, pNode, node);
            BDBG_ASSERT(pNode == BLST_S_FIRST(&handle->buffers[i].freeList));
            pNode->bufferSize = bufferSize;
            pNode->allocated = false;
            pNode->poolIndex = i;
        }
    }

#if BAPE_DSP_SUPPORT
    if ( handle->dspHandle )
    {
        BDSP_ContextCreateSettings dspContextSettings;
        BDSP_Status dspStatus;

        /* Determine num dsps */
        BDSP_GetStatus(handle->dspHandle, &dspStatus);
        handle->numDsps = dspStatus.numDsp;

        /* Create DSP Context */
        BDSP_Context_GetDefaultCreateSettings(handle->dspHandle, BDSP_ContextType_eAudio, &dspContextSettings);
        dspContextSettings.maxTasks = pSettings->maxDspTasks;
        errCode = BDSP_Context_Create(handle->dspHandle, &dspContextSettings, &handle->dspContext);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_context;
        }

        #if BDBG_DEBUG_BUILD
        /* log supported BDSP algos */
        {
        BDSP_AlgorithmInfo algoInfo;
        BERR_Code errCode;
        unsigned i;

        BDBG_MODULE_MSG(bape_algos, ("*** ENABLED RAAGA DECODE/PT/ENCODE BDSP ALGOS ***"));
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("    (BDSP_Algorithm) | Name"));
        for ( i=0; i<BDSP_Algorithm_eGenericPassthrough; i++ )
        {
            if ( BAPE_DSP_P_AlgorithmSupportedByApe(handle, i) )
            {
                errCode = BDSP_GetAlgorithmInfo(handle->dspHandle, i, &algoInfo);
                if ( !errCode )
                {
                    BDBG_MODULE_MSG(bape_algos, ("    (%02d) | %s ", i, algoInfo.pName));
                }
            }
        }
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("***   ENABLED RAAGA OTHER ALGOS/PROCESSING    ***"));
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("    (BDSP_Algorithm) | Name"));
        for ( i=BDSP_Algorithm_eGenericPassthrough; i<BDSP_Algorithm_eAudioProcessing_EndIdx; i++ )
        {
            if ( BAPE_DSP_P_AlgorithmSupported(handle, i) )
            {
                errCode = BDSP_GetAlgorithmInfo(handle->dspHandle, i, &algoInfo);
                if ( !errCode )
                {
                    BDBG_MODULE_MSG(bape_algos, ("    (%02d) | %s ", i, algoInfo.pName));
                }
            }
        }
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        }
        #endif
    }

    #if BDSP_ARM_AUDIO_SUPPORT
    if ( handle->armHandle )
    {
        BDSP_ContextCreateSettings dspContextSettings;
        BDSP_Status dspStatus;

        /* Determine num dsps */
        BDSP_GetStatus(handle->armHandle, &dspStatus);
        handle->numArms = dspStatus.numDsp;

        /* Create DSP Context */
        BDSP_Context_GetDefaultCreateSettings(handle->armHandle, BDSP_ContextType_eAudio, &dspContextSettings);
        dspContextSettings.maxTasks = pSettings->maxArmTasks;
        errCode = BDSP_Context_Create(handle->armHandle, &dspContextSettings, &handle->armContext);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_arm_context;
        }

        #if BDBG_DEBUG_BUILD
        /* log supported BDSP algos */
        {
        BDSP_AlgorithmInfo algoInfo;
        BERR_Code errCode;
        unsigned i;

        BDBG_MODULE_MSG(bape_algos, ("*** ENABLED ARM DECODE/PT/ENCODE BDSP ALGOS ***"));
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("    (BDSP_Algorithm) | Name"));
        for ( i=0; i<BDSP_Algorithm_eGenericPassthrough; i++ )
        {
            if ( BAPE_DSP_P_AlgorithmSupportedByApe(handle, i) )
            {
                errCode = BDSP_GetAlgorithmInfo(handle->armHandle, i, &algoInfo);
                if ( !errCode )
                {
                    BDBG_MODULE_MSG(bape_algos, ("    (%02d) | %s ", i, algoInfo.pName));
                }
            }
        }
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("***   ENABLED ARM OTHER ALGOS/PROCESSING    ***"));
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        BDBG_MODULE_MSG(bape_algos, ("    (BDSP_Algorithm) | Name"));
        for ( i=BDSP_Algorithm_eGenericPassthrough; i<BDSP_Algorithm_eAudioProcessing_EndIdx; i++ )
        {
            if ( BAPE_DSP_P_AlgorithmSupported(handle, i) )
            {
                errCode = BDSP_GetAlgorithmInfo(handle->armHandle, i, &algoInfo);
                if ( !errCode )
                {
                    BDBG_MODULE_MSG(bape_algos, ("    (%02d) | %s ", i, algoInfo.pName));
                }
            }
        }
        BDBG_MODULE_MSG(bape_algos, ("*** ----------------------------------- ***"));
        }
        #endif
    }
    #endif
#endif


#if BAPE_CHIP_MAX_PLLS > 0
    errCode = BAPE_P_InitTimers(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_timer;
    }
#endif

    handle->bStandby = false;

    /* Success */
    *pHandle = handle;

    return BERR_SUCCESS;

#if BAPE_CHIP_MAX_PLLS
err_timer:
    BAPE_P_DestroyTimers(handle);
#endif
#if BAPE_DSP_SUPPORT
#if BDSP_ARM_AUDIO_SUPPORT
    if ( handle->armContext )
    {
        BDSP_Context_Destroy(handle->armContext);
        handle->armContext = NULL;
    }
err_arm_context:
#endif
    if ( handle->dspContext )
    {
        BDSP_Context_Destroy(handle->dspContext);
        handle->dspContext = NULL;
    }
err_context:
#endif
err_buffer:
    /* Remove and free all buffers and nodes */
    for ( i = 0; i < BAPE_MAX_BUFFER_POOLS; i++ )
    {
        while ( (pNode = BLST_S_FIRST(&handle->buffers[i].freeList)) )
        {
            BLST_S_REMOVE_HEAD(&handle->buffers[i].freeList, node);
            BDBG_OBJECT_ASSERT(pNode, BAPE_BufferNode);
            BMEM_Free(memHandle, pNode->pMemory);
            BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
            BKNI_Free(pNode);
        }
    }
    BAPE_P_UninitInterrupts(handle);
err_interrupt:
err_fmm:
    BDBG_OBJECT_DESTROY(handle, BAPE_Device);
    BKNI_Free(handle);
err_handle:
    *pHandle = NULL;

#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
    BCHP_PWR_ReleaseResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL0);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
    BCHP_PWR_ReleaseResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL1);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
    BCHP_PWR_ReleaseResource(chpHandle, BCHP_PWR_RESOURCE_AUD_PLL2);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_AIO
    BCHP_PWR_ReleaseResource(chpHandle, BCHP_PWR_RESOURCE_AUD_AIO);
#endif
    return errCode;
}

void BAPE_Close(
    BAPE_Handle handle
    )
{
    unsigned i=0;
    BAPE_BufferNode *pNode;
    BAPE_MixerHandle mixer;
    BAPE_MuxOutputHandle muxOutput;

    /* Stop all potential mixer inputs first */
#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( handle->decoders[i] )
        {
            BDBG_MSG(("Stopping decoder %p (%d)", (void *)handle->decoders[i], i));
            BAPE_Decoder_Stop(handle->decoders[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( handle->inputCaptures[i] )
        {
            BDBG_MSG(("Stopping input capture %p (%d)", (void *)handle->inputCaptures[i], i));
            BAPE_InputCapture_Stop(handle->inputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( handle->playbacks[i] )
        {
            BDBG_MSG(("Stopping playback %p (%d)", (void *)handle->playbacks[i], i));
            BAPE_Playback_Stop(handle->playbacks[i]);
        }
    }
#endif

    /* Close all mixers next */
    while ( (mixer=BLST_S_FIRST(&handle->mixerList)) )
    {
        BDBG_MSG(("Destroying mixer %p)", (void *)mixer));
        BAPE_Mixer_Destroy(mixer);
    }

    /* Close all MuxOutputs */
    while ( (muxOutput=BLST_S_FIRST(&handle->muxOutputList)) )
    {
        BDBG_MSG(("Destroying muxOutput %p", (void *)muxOutput));
        #if BAPE_DSP_SUPPORT
        BAPE_MuxOutput_Destroy(muxOutput);
        #endif
    }

    /* Close all inputs */
#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( handle->decoders[i] )
        {
            BDBG_MSG(("Closing decoder %p (%d)", (void *)handle->decoders[i], i));
            BAPE_Decoder_Close(handle->decoders[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( handle->inputCaptures[i] )
        {
            BDBG_MSG(("Closing input capture %p (%d)", (void *)handle->inputCaptures[i], i));
            BAPE_InputCapture_Close(handle->inputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( handle->playbacks[i] )
        {
            BDBG_MSG(("Closing playback %p (%d)", (void *)handle->playbacks[i], i));
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
            BDBG_MSG(("Closing DAC %p (%d)", (void *)handle->dacs[i], i));
            BAPE_Dac_Close(handle->dacs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_I2S_OUTPUTS; i++ )
    {
        if ( handle->i2sOutputs[i] )
        {
            BDBG_MSG(("Closing I2S Output %p (%d)", (void *)handle->i2sOutputs[i], i));
            BAPE_I2sOutput_Close(handle->i2sOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS; i++ )
    {
        if ( handle->i2sMultiOutputs[i] )
        {
            BDBG_MSG(("Closing I2S Multi Output %p (%d)", (void *)handle->i2sMultiOutputs[i], i));
            BAPE_I2sMultiOutput_Close(handle->i2sMultiOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_SPDIF_OUTPUTS; i++ )
    {
        if ( handle->spdifOutputs[i] )
        {
            BDBG_MSG(("Closing SPDIF Output %p (%d)", (void *)handle->spdifOutputs[i], i));
            BAPE_SpdifOutput_Close(handle->spdifOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_MAI_OUTPUTS; i++ )
    {
        if ( handle->maiOutputs[i] )
        {
            BDBG_MSG(("Closing MAI Output %p (%d)", (void *)handle->maiOutputs[i], i));
            BAPE_MaiOutput_Close(handle->maiOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_OUTPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_OUTPUT_CAPTURES; i++ )
    {
        if ( handle->outputCaptures[i] )
        {
            BDBG_MSG(("Closing Output Capture %p (%d)", (void *)handle->outputCaptures[i], i));
            BAPE_OutputCapture_Close(handle->outputCaptures[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_DUMMYSINKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DUMMYSINKS; i++ )
    {
        if ( handle->dummyOutputs[i] )
        {
            BDBG_MSG(("Closing Dummy Output %p (%d)", (void *)handle->dummyOutputs[i], i));
            BAPE_DummyOutput_Close(handle->dummyOutputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_I2S_INPUTS; i++ )
    {
        if ( handle->i2sInputs[i] )
        {
            BDBG_MSG(("Closing I2S Input %p (%d)", (void *)handle->i2sInputs[i], i));
            BAPE_I2sInput_Close(handle->i2sInputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_SPDIF_INPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_SPDIF_INPUTS; i++ )
    {
        if ( handle->spdifInputs[i] )
        {
            BDBG_MSG(("Closing SPDIF Input %p (%d)", (void *)handle->spdifInputs[i], i));
            BAPE_SpdifInput_Close(handle->spdifInputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_MAI_INPUTS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_MAI_INPUTS; i++ )
    {
        if ( handle->maiInputs[i] )
        {
            BDBG_MSG(("Closing MAI Input %p (%d)", (void *)handle->maiInputs[i], i));
            BAPE_MaiInput_Close(handle->maiInputs[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS; i++ )
    {
        if ( handle->audioReturnChannels[i] )
        {
            BDBG_MSG(("Closing Audio Return Channel %p (%d)", (void *)handle->audioReturnChannels[i], i));
            BAPE_AudioReturnChannel_Close(handle->audioReturnChannels[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_RFMODS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_RFMODS; i++ )
    {
        if ( handle->rfmods[i] )
        {
            BDBG_MSG(("Closing RFMOD %p (%d)", (void *)handle->rfmods[i], i));
            BAPE_RfMod_Close(handle->rfmods[i]);
        }
    }
#endif
#if BAPE_CHIP_MAX_CRCS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_CRCS; i++ )
    {
        if ( handle->crcs[i] )
        {
            BDBG_MSG(("Closing CRC %p (%d)", (void *)handle->crcs[i], i));
            BAPE_Crc_Close(handle->crcs[i]);
        }
    }
#endif

#if BAPE_DSP_SUPPORT
#if BDSP_ARM_AUDIO_SUPPORT
    if ( handle->armContext )
    {
        BDSP_Context_Destroy(handle->armContext);
        handle->armContext = NULL;
    }
#endif
    if ( handle->dspContext )
    {
        BDSP_Context_Destroy(handle->dspContext);
        handle->dspContext = NULL;
    }
#endif

#if BDBG_DEBUG_BUILD
    /* Log Buffer Usage information */
    BDBG_MODULE_MSG(bape_mem, ("*** ---------------------------------------- ***"));
    BDBG_MODULE_MSG(bape_mem, ("***          BAPE BUFFER USAGE INFO          ***"));
    BDBG_MODULE_MSG(bape_mem, ("*** ---------------------------------------- ***"));

    for ( i = 0; i < BAPE_MAX_BUFFER_POOLS; i++ )
    {
        switch ( i )
        {
        case 0:
            BDBG_MODULE_MSG(bape_mem, ("***            PCM:  allocated %d, max used %d ***",
                handle->buffers[i].numBuffers,
                handle->buffers[i].maxUsed));
            break;
        case 1:
            BDBG_MODULE_MSG(bape_mem, ("***     COMPRESSED:  allocated %d, max used %d ***",
                handle->buffers[i].numBuffers,
                handle->buffers[i].maxUsed));
            break;
        case 2:
            BDBG_MODULE_MSG(bape_mem, ("***  COMPRESSED 4x:  allocated %d, max used %d ***",
                handle->buffers[i].numBuffers,
                handle->buffers[i].maxUsed));
            break;
        case 3:
            BDBG_MODULE_MSG(bape_mem, ("*** COMPRESSED 16x:  allocated %d, max used %d ***",
                handle->buffers[i].numBuffers,
                handle->buffers[i].maxUsed));
             break;
        }
    }
    BDBG_MODULE_MSG(bape_mem, ("*** ---------------------------------------- ***"));
#endif

#if BAPE_CHIP_MAX_PLLS > 0
    BAPE_P_DestroyTimers(handle);
#endif

    for ( i = 0; i < BAPE_MAX_BUFFER_POOLS; i++ )
    {
        while ( (pNode = BLST_S_FIRST(&handle->buffers[i].freeList)) )
        {
            BDBG_MSG(("Destroy Free Buffer node %p", (void *)pNode));
            BLST_S_REMOVE_HEAD(&handle->buffers[i].freeList, node);
            BDBG_OBJECT_ASSERT(pNode, BAPE_BufferNode);
            BMEM_Free(handle->memHandle, pNode->pMemory);
            BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
            BKNI_Free(pNode);
        }
        while ( (pNode = BLST_S_FIRST(&handle->buffers[i].allocatedList)) )
        {
            BDBG_MSG(("Destroy Allocated Buffer node %p", (void *)pNode));
            BLST_S_REMOVE_HEAD(&handle->buffers[i].allocatedList, node);
            BDBG_OBJECT_ASSERT(pNode, BAPE_BufferNode);
            BMEM_Free(handle->memHandle, pNode->pMemory);
            BDBG_OBJECT_DESTROY(pNode, BAPE_BufferNode);
            BKNI_Free(pNode);
        }
    }

    BAPE_P_UninitIopSw(handle);
    BAPE_P_UninitSrcSw(handle);
    BAPE_P_UninitDpSw(handle);
    BAPE_P_UninitBfSw(handle);
    BAPE_P_UninitInterrupts(handle);

#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
    BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL0);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
    BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL1);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
    BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL2);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_AIO
    BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_AIO);
#endif

    BDBG_OBJECT_DESTROY(handle, BAPE_Device);
    BKNI_Free(handle);
}

/* Get Channel Status code for a given sample rate */
unsigned BAPE_P_GetSampleRateCstatCode_isr(unsigned sampleRate)
{
    switch ( sampleRate )
    {
    case 32000:      /* 32K Sample rate */
        return 0x3;
    case 44100:    /* 44.1K Sample rate */
        return 0x0;
    case 48000:      /* 48K Sample rate */
        return 0x2;
    case 96000:      /* 96K Sample rate */
        return 0xa;
    case 22050:   /* 22.05K Sample rate */
        return 0x4;
    case 24000:      /* 24K Sample rate */
        return 0x6;
    case 88200:    /* 88.2K Sample rate */
        return 0x8;
    case 176400:   /* 176.4K Sample rate */
        return 0xc;
    case 192000:     /* 192K Sample rate */
        return 0xe;
    case 768000:     /* 768K Sample rate */
        return 0x9;
    default:
        return 0x1; /* not indicated */
    }
}

/**************************************************************************/

static BERR_Code BAPE_P_InitFmmSw(BAPE_Handle handle)
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    errCode = BAPE_P_InitBfSw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_init_bf;
    }

    errCode = BAPE_P_InitDpSw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_init_dp;
    }

    BDBG_MSG(("Initializing SRC registers"));
    errCode = BAPE_P_InitSrcSw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_init_src;
    }

    errCode = BAPE_P_InitIopSw(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_init_iop;
    }

    return BERR_SUCCESS;

err_init_iop:
    BAPE_P_UninitSrcSw(handle);
err_init_src:
    BAPE_P_UninitDpSw(handle);
err_init_dp:
    BAPE_P_UninitBfSw(handle);
err_init_bf:
    return BERR_TRACE(errCode);
}


static BERR_Code BAPE_P_InitFmmHw(BAPE_Handle handle)
{
    BREG_Handle regHandle;
    uint32_t regVal;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    regHandle = handle->regHandle;

    errCode = BAPE_P_ResetAIO(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    BDBG_MSG(("Resetting FMM"));
#ifdef BCHP_AUD_FMM_MISC_RESET
    /* Assert toplevel reset */
    BREG_Write32(regHandle, BCHP_AUD_FMM_MISC_RESET, 0);
    regVal = BREG_Read32(regHandle, BCHP_AUD_FMM_MISC_RESET);
    regVal |=
            (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_TOP_LOGIC_B, Inactive))
    #ifdef BCHP_AUD_FMM_MISC_RESET_RESET_SPDIFRX_LOGIC_B_Inactive
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_HDMIRX_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_HDMIRX_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_REGS_B, Inactive))
    #else
        #ifdef BCHP_AUD_FMM_MISC_RESET_RESET_SPDIFRX_0_LOGIC_B_Inactive
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_0_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_0_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_1_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SPDIFRX_1_REGS_B, Inactive))
        #endif
    #endif

          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_REGS_B, Inactive))
    #ifdef BCHP_AUD_FMM_MISC_RESET_RESET_ADC_CIC_REGS_B_Inactive
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_ADC_CIC_REGS_B, Inactive))
    #endif
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_TOP_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_OP_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_PROC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_MS_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_SRC_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_DP_REGS_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_LOGIC_B, Inactive))
          | (BCHP_FIELD_ENUM (AUD_FMM_MISC_RESET, RESET_BF_REGS_B, Inactive));
    BREG_Write32(regHandle, BCHP_AUD_FMM_MISC_RESET, regVal);

    /* Powerup the FMM Modules */
    regVal = BREG_Read32(regHandle, BCHP_AIO_MISC_PWRDOWN);

#if BCHP_AIO_MISC_PWRDOWN_ADC_RESET_Reset
    /* Don't bother with DAC-related fields... they're handled by BAPE_P_InitDacs */
    regVal &= ~((BCHP_MASK (AIO_MISC_PWRDOWN, ADC_IDDQ_PWRUP)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, ADC_REF_PWRUP)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, ADC_R_PWRUP)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, ADC_L_PWRUP)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, ADC_RESET)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, ADC_PWRUP_CORE)) |
                (BCHP_MASK (AIO_MISC_PWRDOWN, SPDIF_RX0)));

    regVal |= ((BCHP_FIELD_DATA (AIO_MISC_PWRDOWN, ADC_IDDQ_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AIO_MISC_PWRDOWN, ADC_REF_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AIO_MISC_PWRDOWN, ADC_R_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AIO_MISC_PWRDOWN, ADC_L_PWRUP, 1)) |
               (BCHP_FIELD_ENUM (AIO_MISC_PWRDOWN, ADC_RESET, Normal)) |
               (BCHP_FIELD_ENUM (AIO_MISC_PWRDOWN, ADC_PWRUP_CORE, Normal)) |
               (BCHP_FIELD_ENUM (AIO_MISC_PWRDOWN, SPDIF_RX0, Normal)));
#endif

#if BCHP_AIO_MISC_PWRDOWN_SPDIF_RX_Powerdown
    regVal &= ~(BCHP_MASK (AIO_MISC_PWRDOWN, SPDIF_RX));
    regVal |= (BCHP_FIELD_ENUM (AIO_MISC_PWRDOWN, SPDIF_RX, Normal));
#endif

    BREG_Write32(regHandle, BCHP_AIO_MISC_PWRDOWN, regVal);
#else
    /* Newer 7429-style chips have a single-bit reset for the entire audio block */
    regVal = BCHP_FIELD_ENUM(AUD_MISC_INIT, AUDIO_INIT, Init);
    BREG_Write32(regHandle, BCHP_AUD_MISC_INIT, regVal);
    (void)BREG_Read32(regHandle, BCHP_AUD_MISC_INIT);
    regVal = BCHP_FIELD_ENUM(AUD_MISC_INIT, AUDIO_INIT, Inactive);
    BREG_Write32(regHandle, BCHP_AUD_MISC_INIT, regVal);

    /* Powerup the FMM Modules */
    regVal = BREG_Read32(regHandle, BCHP_AUD_MISC_PWRDOWN);

    #if BCHP_AUD_MISC_PWRDOWN_ADC_RESET_Reset
    /* Don't bother with DAC-related fields... they're handled by BAPE_P_InitDacs */
    regVal &= ~((BCHP_MASK (AUD_MISC_PWRDOWN, ADC_IDDQ_PWRUP)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, ADC_REF_PWRUP)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, ADC_R_PWRUP)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, ADC_L_PWRUP)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, ADC_RESET)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, ADC_PWRUP_CORE)) |
                (BCHP_MASK (AUD_MISC_PWRDOWN, SPDIF_RX0)));

    regVal |= ((BCHP_FIELD_DATA (AUD_MISC_PWRDOWN, ADC_IDDQ_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AUD_MISC_PWRDOWN, ADC_REF_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AUD_MISC_PWRDOWN, ADC_R_PWRUP, 1)) |
               (BCHP_FIELD_DATA (AUD_MISC_PWRDOWN, ADC_L_PWRUP, 1)) |
               (BCHP_FIELD_ENUM (AUD_MISC_PWRDOWN, ADC_RESET, Normal)) |
               (BCHP_FIELD_ENUM (AUD_MISC_PWRDOWN, ADC_PWRUP_CORE, Normal)) |
               (BCHP_FIELD_ENUM (AUD_MISC_PWRDOWN, SPDIF_RX0, Normal)));
    #endif

    #if BCHP_AUD_MISC_PWRDOWN_SPDIF_RX_Powerdown
    regVal &= ~(BCHP_MASK (AUD_MISC_PWRDOWN, SPDIF_RX));
    regVal |= (BCHP_FIELD_ENUM (AUD_MISC_PWRDOWN, SPDIF_RX, Normal));
    #endif

    BREG_Write32(regHandle, BCHP_AUD_MISC_PWRDOWN, regVal);
#endif

    errCode = BAPE_P_InitDacHw(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_P_InitBfHw(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_P_InitDpHw(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_P_InitSrcHw(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_P_InitIopHw(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_InitTimers(BAPE_Handle handle)
{
    BTMR_TimerSettings timerSettings;
    int i;
    BERR_Code errCode;
    for(i = 0; i < BAPE_CHIP_MAX_PLLS; i++)
    {
       BTMR_GetDefaultTimerSettings(&timerSettings);
       timerSettings.type = BTMR_Type_eCountDown;
       timerSettings.cb_isr = BAPE_P_VerifyPllCallback_isr;
       timerSettings.pParm1 = (void *)handle;
       timerSettings.parm2 = i;
       errCode = BTMR_CreateTimer(handle->tmrHandle, &handle->pllTimer[i], &timerSettings);
       if ( errCode ) return BERR_TRACE(errCode);
   }
   return BERR_SUCCESS;
}
static void BAPE_P_DestroyTimers(BAPE_Handle handle)
{
    int i;
    for(i = 0; i < BAPE_CHIP_MAX_PLLS; i++)
    {
        if (handle->pllTimer[i] != NULL)
        {
            BTMR_DestroyTimer (handle->pllTimer[i]);
            handle->pllTimer[i] = NULL;
        }
    }
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

#if BAPE_DSP_SUPPORT
    if ( NULL != handle->dspContext )
    {
        BERR_Code errCode;
        BDSP_ContextInterruptHandlers contextInterrupts;

        BDSP_Context_GetInterruptHandlers(handle->dspContext, &contextInterrupts);
        contextInterrupts.watchdog.pCallback_isr = pInterrupts->watchdog.pCallback_isr;
        contextInterrupts.watchdog.pParam1 = pInterrupts->watchdog.pParam1;
        contextInterrupts.watchdog.param2 = pInterrupts->watchdog.param2;
        errCode = BDSP_Context_SetInterruptHandlers(handle->dspContext, &contextInterrupts);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
#endif

    handle->interrupts = *pInterrupts;

    return BERR_SUCCESS;
}

BERR_Code BAPE_ProcessWatchdogInterruptStop(
    BAPE_Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

#if BAPE_DSP_SUPPORT
    if ( handle->dspContext || handle->armContext )
    {
        BERR_Code errCode;
        unsigned i, numFound;
        BAPE_PathNode *pNode;
        BAPE_MixerHandle hMixer;

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
            }
            else
            {
                handle->decoderWatchdogInfo[i].state = BAPE_DecoderState_eMax;
            }
        }
        /* If we have playback/inputcapture feeding DSP mixer, we need to stop that as well */
        for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++)
        {
            if ( handle->playbacks[i] && handle->playbacks[i]->running )
            {
                BAPE_PathNode_P_FindConsumersBySubtype(&handle->playbacks[i]->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);
                if ( numFound > 0 )
                {
                    handle->playbackWatchdogInfo[i].restartRequired = true;
                    handle->playbackWatchdogInfo[i].startSettings = handle->playbacks[i]->startSettings;
                    BAPE_Playback_Stop(handle->playbacks[i]);
                }
            }
        }
        for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++)
        {
            if ( handle->inputCaptures[i] && handle->inputCaptures[i]->running )
            {
                BAPE_PathNode_P_FindConsumersBySubtype(&handle->inputCaptures[i]->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);
                if ( numFound > 0 )
                {
                    handle->inputCaptureWatchdogInfo[i].restartRequired = true;
                    handle->inputCaptureWatchdogInfo[i].startSettings = handle->inputCaptures[i]->startSettings;
                    BAPE_InputCapture_Stop(handle->inputCaptures[i]);
                }
            }
        }
        /* If we have any explicitly started DSP mixers, stop them now */
        for ( hMixer = BLST_S_FIRST(&handle->mixerList);
              hMixer != NULL;
              hMixer = BLST_S_NEXT(hMixer, node) )
        {
            if ( (hMixer->settings.type == BAPE_MixerType_eDsp || hMixer->settings.type == BAPE_MixerType_eStandard) && hMixer->startedExplicitly )
            {
                hMixer->restartPending = true;
                BAPE_Mixer_Stop(hMixer);
            }
        }

        /* Reboot ARM Audio context */
        if ( handle->armContext )
        {
            errCode = BDSP_Context_ProcessWatchdogInterrupt(handle->armContext);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        /* Reboot the DSP */
        if ( handle->dspContext )
        {
            errCode = BDSP_Context_ProcessWatchdogInterrupt(handle->dspContext);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_ProcessWatchdogInterruptResume(
    BAPE_Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

#if BAPE_DSP_SUPPORT
    if ( handle->dspContext )
    {
        BERR_Code errCode;
        unsigned i;
        BAPE_MixerHandle hMixer;

        /* If we have any explicitly started DSP mixers, stop them now */
        for ( hMixer = BLST_S_FIRST(&handle->mixerList);
              hMixer != NULL;
              hMixer = BLST_S_NEXT(hMixer, node) )
        {
            if ( hMixer->restartPending )
            {
                hMixer->restartPending = false;
                errCode = BAPE_Mixer_Start(hMixer);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                }
            }
        }
        /* If we have playback/inputcapture feeding DSP mixer, we need to stop that as well */
        for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++)
        {
            if ( handle->playbackWatchdogInfo[i].restartRequired  )
            {
                handle->playbackWatchdogInfo[i].restartRequired = false;
                errCode = BAPE_Playback_Start(handle->playbacks[i], &handle->playbackWatchdogInfo[i].startSettings);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                }
            }
        }
        for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++)
        {
            if ( handle->inputCaptureWatchdogInfo[i].restartRequired )
            {
                handle->inputCaptureWatchdogInfo[i].restartRequired = false;
                errCode = BAPE_InputCapture_Start(handle->inputCaptures[i], &handle->inputCaptureWatchdogInfo[i].startSettings);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                }
            }
        }
        /* Reset all decoder state */
        for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
        {
            if ( handle->decoders[i] )
            {
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
#endif

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_ResetAIO(BAPE_Handle handle)
{
    #ifdef BCHP_SUN_TOP_CTRL_SW_RESET
        /* Note, older SW_RESET registers DO need read-modify-write (atomic update) */
        BREG_AtomicUpdate32_isr(regHandle, BCHP_SUN_TOP_CTRL_SW_RESET,
                0,                                                  /* Clear these bits */
                BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset));   /* Set these bits   */

        BREG_AtomicUpdate32_isr(regHandle, BCHP_SUN_TOP_CTRL_SW_RESET,
                BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset),    /* Clear these bits */
                0);                                                 /* Set these bits   */
    #else
        /* Note, newer SW_INIT set/clear registers DON'T need read-modify-write. */
        /* Put AIO into reset */
        BAPE_Reg_P_UpdateField(handle, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1);

        /* Now clear the reset. */
        BAPE_Reg_P_UpdateField(handle, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1);
    #endif

    return BERR_SUCCESS;
}

#ifdef BCHP_PWR_SUPPORT
static BERR_Code BAPE_P_StandbyAIO(BAPE_Handle handle)
{
#if BCHP_AUD_MISC_INIT_ACK /* 28nm */
    int timeout;
    unsigned ack;

    /* Suspend SCB clients */
    BAPE_Reg_P_UpdateEnum(handle, BCHP_AUD_MISC_INIT, AUD_MISC_INIT, SCB_CLIENT0_INIT, Init);
    timeout = 100;
    ack = BAPE_Reg_P_ReadField(handle, BCHP_AUD_MISC_INIT_ACK, AUD_MISC_INIT_ACK, SCB_CLIENT0_INIT_ACK);
    while ( timeout > 0 && !ack )
    {
        BKNI_Delay(1000); /* 1 millisecond */
        ack = BAPE_Reg_P_ReadField(handle, BCHP_AUD_MISC_INIT_ACK, AUD_MISC_INIT_ACK, SCB_CLIENT0_INIT_ACK);
        timeout--;
    }

    if ( timeout == 0 )
    {
        BDBG_ERR(("ERROR - %s - timed out waiting for SCB_CLIENT0_INIT_ACK", __FUNCTION__));
        return BERR_TRACE(BERR_TIMEOUT);
    }

    /* Put AIO into reset */
    BAPE_Reg_P_UpdateField(handle, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1);

    /* Now clear the reset. */
    BAPE_Reg_P_UpdateField(handle, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1);
#else
    BSTD_UNUSED(handle);
#endif

    return BERR_SUCCESS;
}

static BERR_Code BAPE_P_StandbyFmmHw(BAPE_Handle handle)
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    /* Since all channels are stopped, we can relese the unused "path resources"
     * and that should release everything... That way, when we resume from standby,
     * then all of the resources will be reallocated (causing the corresponding
     * hardware to be reconfigured to its proper state).
     */
    BAPE_P_ReleaseUnusedPathResources(handle);

    /* Disable and destroy the global APE interrupts... We'll recreate them at resume time. */
    BAPE_P_UninitInterrupts(handle);

    errCode = BAPE_P_StandbyAIO(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    /* Now tell a few more things to get ready to power down into standby mode. */
    errCode = BAPE_SpdifInput_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_MaiInput_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_MaiOutput_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_SpdifOutput_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_Dac_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_I2sOutput_P_PrepareForStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    return BERR_SUCCESS;
}
#endif /* BCHP_PWR_SUPPORT*/


#ifdef BCHP_PWR_SUPPORT
static BERR_Code BAPE_P_ResumeFmmHw(BAPE_Handle handle)
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    errCode = BAPE_Nco_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_Pll_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_LoopbackGroup_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_DummysinkGroup_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

#if !B_REFSW_MINIMAL
    errCode = BAPE_AudioReturnChannel_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);
#endif

    errCode = BAPE_Dac_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_DummyOutput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

#if !B_REFSW_MINIMAL
    errCode = BAPE_I2sMultiOutput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);
#endif

    errCode = BAPE_I2sOutput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_MaiOutput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_RfMod_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_SpdifOutput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_SpdifInput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    errCode = BAPE_MaiInput_P_ResumeFromStandby(handle);
    if ( errCode ) return BERR_TRACE(errCode);

    return BERR_SUCCESS;
}
#endif /* BCHP_PWR_SUPPORT*/

BERR_Code BAPE_Standby(
    BAPE_Handle handle,                 /* [in] AP device handle */
    BAPE_StandbySettings *pSettings     /* [in] standby settings */
)
{
#ifdef BCHP_PWR_SUPPORT
    unsigned i;
#endif

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    BSTD_UNUSED(pSettings);

#ifdef BCHP_PWR_SUPPORT
    /* check that all channels have been stopped */
#if BAPE_CHIP_MAX_DECODERS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( handle->decoders[i] && handle->decoders[i]->state != BAPE_DecoderState_eStopped )
        {
            BDBG_ERR(("Decoder %p (%d) is not stopped", (void *)handle->decoders[i], i));
            return BERR_UNKNOWN;
        }
    }
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    for ( i = 0; i < BAPE_CHIP_MAX_INPUT_CAPTURES; i++ )
    {
        if ( handle->inputCaptures[i] && handle->inputCaptures[i]->running == true )
        {
            BDBG_ERR(("Input capture %p (%d) is not stopped", (void *)handle->inputCaptures[i], i));
            return BERR_UNKNOWN;
        }
    }
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    for ( i = 0; i < BAPE_CHIP_MAX_PLAYBACKS; i++ )
    {
        if ( handle->playbacks[i] && handle->playbacks[i]->running == true)
        {
            BDBG_ERR(("Playback %p (%d) is not stopped", (void *)handle->playbacks[i], i));
            return BERR_UNKNOWN;
        }
    }
#endif
#if BAPE_CHIP_MAX_PLLS > 0
    BAPE_P_DestroyTimers(handle);
#endif

    /* if we reach here, then no channels are active. we can power down */
    if (!handle->bStandby)
    {
        BERR_Code   errCode;

        handle->bStandby = true;

        /* Prepare to go into standby. */
        errCode = BAPE_P_StandbyFmmHw(handle);
        if ( errCode ) return BERR_TRACE(errCode);

#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
        BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL0);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
        BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL1);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
        BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL2);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_AIO
        BCHP_PWR_ReleaseResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_AIO);
#endif

    }
#endif

    return BERR_SUCCESS;
}

BERR_Code BAPE_Resume(
    BAPE_Handle handle  /* [in] APE device handle */
)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

#ifdef BCHP_PWR_SUPPORT
    if (handle->bStandby) {
        handle->bStandby = false;

#ifdef BCHP_PWR_RESOURCE_AUD_AIO
        BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_AIO);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL0
        BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL0);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL1
        BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL1);
#endif
#ifdef BCHP_PWR_RESOURCE_AUD_PLL2
        BCHP_PWR_AcquireResource(handle->chpHandle, BCHP_PWR_RESOURCE_AUD_PLL2);
#endif

        {
            BERR_Code   errCode;

            /* Put the FMM hardware into the "initial" state... the state that
             * BAPE_Open() leaves things in.
             */
            errCode = BAPE_P_InitFmmHw(handle);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Recreate and enable the APE global interrupts. */
            errCode = BAPE_P_InitInterrupts(handle);
            if ( errCode ) return BERR_TRACE(errCode);

            /* Now bring inputs and outputs into their "open" (but "unstarted") state. */
            errCode = BAPE_P_ResumeFmmHw(handle);
            if ( errCode ) return BERR_TRACE(errCode);
#if BAPE_CHIP_MAX_PLLS > 0
            errCode = BAPE_P_InitTimers(handle);
            if ( errCode ) return BERR_TRACE(errCode);
#endif
        }
    }
#endif

    return BERR_SUCCESS;
}

void BAPE_GetCapabilities(
    BAPE_Handle hApe,
    BAPE_Capabilities *pCaps        /* [out] */
    )
{
#if BAPE_DSP_SUPPORT
    BDSP_Handle hDsp;
    unsigned i;
#endif

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(NULL != pCaps);

    BKNI_Memset(pCaps, 0, sizeof(BAPE_Capabilities));

    #ifdef BAPE_CHIP_MAX_I2S_INPUTS
    pCaps->numInputs.i2s = BAPE_CHIP_MAX_I2S_INPUTS;
    #endif
    #ifdef BAPE_CHIP_MAX_MAI_INPUTS
    pCaps->numInputs.mai = BAPE_CHIP_MAX_MAI_INPUTS;
    #endif
    #ifdef BAPE_CHIP_MAX_SPDIF_INPUTS
    pCaps->numInputs.spdif = BAPE_CHIP_MAX_SPDIF_INPUTS;
    #endif

    #ifdef BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS
    pCaps->numOutputs.audioReturnChannel = BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS;
    #endif
    #ifdef BAPE_CHIP_MAX_OUTPUT_CAPTURES
    pCaps->numOutputs.capture = BAPE_CHIP_MAX_OUTPUT_CAPTURES;
    #endif
    #ifdef BAPE_CHIP_MAX_DACS
    pCaps->numOutputs.dac = BAPE_CHIP_MAX_DACS;
    #endif
    #ifdef BAPE_CHIP_MAX_DUMMYSINKS
    pCaps->numOutputs.dummy = BAPE_CHIP_MAX_DUMMYSINKS;
    #endif
    #ifdef BAPE_CHIP_MAX_I2S_OUTPUTS
    pCaps->numOutputs.i2s = BAPE_CHIP_MAX_I2S_OUTPUTS;
    #endif
    #ifdef BAPE_CHIP_MAX_LOOPBACKS
    pCaps->numOutputs.loopback = BAPE_CHIP_MAX_LOOPBACKS;
    #endif
    #ifdef BAPE_CHIP_MAX_MAI_OUTPUTS
    pCaps->numOutputs.mai = BAPE_CHIP_MAX_MAI_OUTPUTS;
    #endif
    #ifdef BAPE_CHIP_MAX_RFMODS
    pCaps->numOutputs.rfmod = BAPE_CHIP_MAX_RFMODS;
    #endif
    #ifdef BAPE_CHIP_MAX_SPDIF_OUTPUTS
    pCaps->numOutputs.spdif = BAPE_CHIP_MAX_SPDIF_OUTPUTS;
    #endif

    #ifdef BAPE_CHIP_MAX_DECODERS
    pCaps->numDecoders = BAPE_CHIP_MAX_DECODERS;
    #endif
    #ifdef BAPE_CHIP_MAX_PLAYBACKS
    pCaps->numPlaybacks = BAPE_CHIP_MAX_PLAYBACKS;
    #endif
    #ifdef BAPE_CHIP_MAX_INPUT_CAPTURES
    pCaps->numInputCaptures = BAPE_CHIP_MAX_INPUT_CAPTURES;
    #endif
    #ifdef BAPE_CHIP_MAX_VCXOS
    pCaps->numVcxos = BAPE_CHIP_MAX_VCXOS;
    #endif
    #ifdef BAPE_CHIP_MAX_PLLS
    pCaps->numPlls = BAPE_CHIP_MAX_PLLS;
    #endif
    #ifdef BAPE_CHIP_MAX_NCOS
    pCaps->numNcos = BAPE_CHIP_MAX_NCOS;
    #endif
    #ifdef BAPE_CHIP_MAX_CRCS
    pCaps->numCrcs = BAPE_CHIP_MAX_CRCS;
    #endif
    #ifdef BAPE_CHIP_MAX_STCS
    pCaps->numStcs = BAPE_CHIP_MAX_STCS;
    #endif

#if BAPE_DSP_SUPPORT
    hDsp = hApe->dspHandle;
    if ( hDsp )
    {
        BERR_Code errCode;
        BDSP_Status dspStatus;
        const BAPE_CodecAttributes *pAttributes;
        BDSP_AlgorithmInfo algorithmInfo;

        BDSP_GetStatus(hDsp, &dspStatus);
        pCaps->numDsps = dspStatus.numDsp;
        BKNI_Snprintf(pCaps->dsp.versionInfo, sizeof(pCaps->dsp.versionInfo),
                      "%dp%dp%dp%d",dspStatus.firmwareVersion.majorVersion,
                      dspStatus.firmwareVersion.minorVersion,
                      dspStatus.firmwareVersion.branchVersion,
                      dspStatus.firmwareVersion.branchSubVersion);

        for ( i = 0; i < BAVC_AudioCompressionStd_eMax; i++ )
        {
            pAttributes = BAPE_P_GetCodecAttributes_isrsafe(i);
            BDBG_ASSERT(NULL != pAttributes);
            if ( pAttributes->decodeAlgorithm != BDSP_Algorithm_eMax )
            {
                errCode = BDSP_GetAlgorithmInfo(hDsp, pAttributes->decodeAlgorithm, &algorithmInfo);
                if ( BERR_SUCCESS == errCode )
                {
                    if ( algorithmInfo.supported )
                    {
                        pCaps->dsp.codecs[i].decode = true;
                    }
                }
            }
            if ( pAttributes->encodeAlgorithm != BDSP_Algorithm_eMax )
            {
                errCode = BDSP_GetAlgorithmInfo(hDsp, pAttributes->encodeAlgorithm, &algorithmInfo);
                if ( BERR_SUCCESS == errCode )
                {
                    if ( algorithmInfo.supported )
                    {
                        pCaps->dsp.codecs[i].encode = true;
                        pCaps->dsp.encoder = true;
                    }
                }
            }
            if ( pAttributes->passthroughAlgorithm != BDSP_Algorithm_eMax )
            {
                errCode = BDSP_GetAlgorithmInfo(hDsp, pAttributes->passthroughAlgorithm, &algorithmInfo);
                if ( BERR_SUCCESS == errCode )
                {
                    if ( algorithmInfo.supported )
                    {
                        pCaps->dsp.codecs[i].passthrough = true;
                    }
                }
            }
        }

        for ( i = 0; i < BDSP_Algorithm_eMax; i++ )
        {
            errCode = BDSP_GetAlgorithmInfo(hDsp, i, &algorithmInfo);
            if ( BERR_SUCCESS == errCode && true == algorithmInfo.supported )
            {
                switch ( i )
                {
                case BDSP_Algorithm_eBrcmAvl:
                    pCaps->dsp.autoVolumeLevel = true;
                    break;
                case BDSP_Algorithm_eDsola:
                    pCaps->dsp.decodeRateControl = true;
                    break;
                case BDSP_Algorithm_eSrsTruVolume:
                    pCaps->dsp.truVolume = true;
                    break;
                case BDSP_Algorithm_eBrcm3DSurround:
                    pCaps->dsp._3dSurround = true;
                    break;
                case BDSP_Algorithm_eMixer:
                    pCaps->dsp.mixer = true;
                    break;
                case BDSP_Algorithm_eDDPEncode:
                case BDSP_Algorithm_eDdre:
                    pCaps->dsp.dolbyDigitalReencode = true;
                    break;
                case BDSP_Algorithm_eDpcmr:
                case BDSP_Algorithm_eDv258:
                    pCaps->dsp.dolbyVolume = true;
                    break;
                case BDSP_Algorithm_eGenCdbItb:
                    pCaps->dsp.muxOutput = true;
                    break;
                case BDSP_Algorithm_eBtscEncoder:
                    pCaps->dsp.rfEncoder.supported = true;
                    pCaps->dsp.rfEncoder.encodings[BAPE_RfAudioEncoding_eBtsc] = true;
                    break;
                case BDSP_Algorithm_eSpeexAec:
                    pCaps->dsp.echoCanceller.supported = true;
                    pCaps->dsp.echoCanceller.algorithms[BAPE_EchoCancellerAlgorithm_eSpeex] = true;
                    break;
                case BDSP_Algorithm_eKaraoke:
                    pCaps->dsp.karaoke = true;
                    break;
                case BDSP_Algorithm_eVocalPP:
                    pCaps->dsp.processing[BAPE_PostProcessorType_eKaraokeVocal] = true;
                    break;
                case BDSP_Algorithm_eFadeCtrl:
                    pCaps->dsp.processing[BAPE_PostProcessorType_eFade] = true;
                    break;
                default:
                    break;
                }
            }
        }

        /* special cases */
        {
            /* disable DDP encode in MS12 config C */
            if ( pCaps->dsp.codecs[BAVC_AudioCompressionStd_eAc3Plus].encode && BAPE_P_GetDolbyMS12Config() == BAPE_DolbyMs12Config_eC )
            {
                pCaps->dsp.codecs[BAVC_AudioCompressionStd_eAc3Plus].encode = false;
            }

            if ( pCaps->dsp.codecs[BAVC_AudioCompressionStd_eAc3].encode &&
                 (BAPE_P_GetDolbyMSVersion_isrsafe() == BAPE_DolbyMSVersion_eMS12 || BAPE_P_GetDolbyMSVersion_isrsafe() == BAPE_DolbyMSVersion_eMS11) )
            {
                pCaps->dsp.codecs[BAVC_AudioCompressionStd_eAc3].encode = false;
            }
        }
    }

    #if BAPE_CHIP_SRC_TYPE_IS_IIR
    pCaps->equalizer.supported = true;
    for ( i = 0; i < BAPE_EqualizerStageType_eMax; i++ )
    {
        pCaps->equalizer.types[i] = true;
    }
    #endif
#endif
}

#if BAPE_DSP_SUPPORT
void BAPE_P_PopulateSupportedBDSPAlgos(
    BDSP_AlgorithmType type, /* [in] */
    const BAVC_AudioCompressionStd * pSupportedCodecs, /* [in] */
    unsigned numSupportedCodecs, /* [in] */
    const bool * inAlgorithmSupported, /* [in] */
    bool * outAlgorithmSupported /* [out] */
    )
{
    unsigned i;
    BDSP_Algorithm algo;
    bool algoSupported[BDSP_Algorithm_eMax];

    if ( pSupportedCodecs == NULL )
    {
        numSupportedCodecs = BAVC_AudioCompressionStd_eMax;
    }

    BKNI_Memset(algoSupported, 0, sizeof(algoSupported));
    for ( i = 0; i < numSupportedCodecs; i++ )
    {
        BAVC_AudioCompressionStd bavcCodec = (pSupportedCodecs != NULL) ? pSupportedCodecs[i] : i;

        /* Look for a valid entry in the APE table */
        switch ( type )
        {
            case BDSP_AlgorithmType_eAudioDecode:
                algo = BAPE_P_GetCodecAudioDecode(bavcCodec);
                break;
            case BDSP_AlgorithmType_eAudioPassthrough:
                algo = BAPE_P_GetCodecAudioPassthrough(bavcCodec);
                break;
            case BDSP_AlgorithmType_eAudioEncode:
                algo = BAPE_P_GetCodecAudioEncode(bavcCodec);
                break;
            default:
                algo = BDSP_Algorithm_eMax;
        }
        if ( algo != BDSP_Algorithm_eMax )
        {
            /* Now that we see APE supports it, look for a valid entry in the BDSP table */
            if ( inAlgorithmSupported[algo] )
            {
                BDBG_MSG(("%s - %s BAVC codec %d is supported, BDSP algo %d", __FUNCTION__,
                    (type==BDSP_AlgorithmType_eAudioDecode) ? "Decode" :
                    (type==BDSP_AlgorithmType_eAudioPassthrough) ? "Passthrough" :
                    (type==BDSP_AlgorithmType_eAudioEncode) ? "Encode" :
                    "Invalid",
                    i, algo));
                algoSupported[algo] = true;
            }
        }
        else
        {
            BDBG_MSG(("%s - requested BAVC codec %d is not enabled by APE/BDSP", __FUNCTION__, i));
        }
    }

    for ( i = 0; i < BDSP_Algorithm_eMax; i++ )
    {
        outAlgorithmSupported[i] = algoSupported[i];
    }
}

#if 0
void BAPE_P_PopulateSupportedBAVCAlgos(
    BAPE_Handle deviceHandle, /* [in] */
    BDSP_AlgorithmType algoType, /* [in] */
    bool * outSupportedCodecs /* [out] */
    )
{
    unsigned i;
    BDSP_Algorithm algo;
    BDSP_StageCreateSettings stageCreateSettings;

    BKNI_Memset(outSupportedCodecs, 0, sizeof(outSupportedCodecs));

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, algoType, &stageCreateSettings);
    for ( i = 0; i < BAVC_AudioCompressionStd_eMax; i++ )
    {
        /* Look for a valid entry in the APE table */
        switch ( algoType )
        {
            case BDSP_AlgorithmType_eAudioDecode:
                algo = BAPE_P_GetCodecAudioDecode(i);
                break;
            case BDSP_AlgorithmType_eAudioPassthrough:
                algo = BAPE_P_GetCodecAudioPassthrough(i);
                break;
            case BDSP_AlgorithmType_eAudioEncode:
                algo = BAPE_P_GetCodecAudioEncode(i);
                break;
            default:
                algo = BDSP_Algorithm_eMax;
        }
        if ( algo != BDSP_Algorithm_eMax )
        {
            /* Now that we see APE supports it, look for a valid entry in the BDSP table */
            if ( stageCreateSettings.algorithmSupported[algo] )
            {
                BDBG_MSG(("%s - %s BAVC codec %d is supported", __FUNCTION__,
                    (algoType==BDSP_AlgorithmType_eAudioDecode) ? "Decode" :
                    (algoType==BDSP_AlgorithmType_eAudioPassthrough) ? "Passthrough" :
                    (algoType==BDSP_AlgorithmType_eAudioEncode) ? "Encode" :
                    "Invalid",
                    i));
                outSupportedCodecs[i] = true;
            }
        }
        else
        {
            BDBG_MSG(("%s - requested BAVC codec %d is not enabled by APE/BDSP", __FUNCTION__, i));
        }
    }
}
#endif

/***************************************************************************
Summary:
Get Audio Algo attributes
***************************************************************************/
BDSP_Algorithm BAPE_GetCodecAudioDecode (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_GetCodecAudioDecode(codec);
}

BDSP_Algorithm BAPE_GetCodecAudioPassthrough (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_GetCodecAudioPassthrough(codec);
}

BDSP_Algorithm BAPE_GetCodecAudioEncode (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_GetCodecAudioEncode(codec);
}

bool BAPE_CodecRequiresSrc (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_CodecRequiresSrc(codec);
}

bool BAPE_CodecSupportsCompressed4x (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_CodecSupportsCompressed4x(codec);
}

bool BAPE_CodecSupportsCompressed16x (
    BAVC_AudioCompressionStd codec
    )
{
    return BAPE_P_CodecSupportsCompressed16x(codec);
}

BAPE_DolbyMSVersion BAPE_GetDolbyMSVersion (void)
{
    return BAPE_P_GetDolbyMSVersion();
}
#endif
