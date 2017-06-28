/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/

#include "nexus_audio_module.h"
#include "priv/nexus_audio_mux_output_priv.h"

BDBG_MODULE(nexus_audio_mux_output);

#define NEXUS_AUDIO_MUX_USE_RAVE 0

#if NEXUS_HAS_AUDIO_MUX_OUTPUT

typedef struct NEXUS_AudioMuxOutput
{
    NEXUS_OBJECT(NEXUS_AudioMuxOutput);
    BLST_S_ENTRY(NEXUS_AudioMuxOutput) node;
    NEXUS_AudioOutputObject connector;
    NEXUS_RaveHandle raveContext;
    BAVC_XptContextMap contextMap;
    BAPE_MuxOutputHandle muxOutput;
    BAVC_Xpt_StcSoftIncRegisters softIncRegisters;
    NEXUS_AudioMuxOutputStartSettings startSettings;
    NEXUS_AudioInputHandle input;
    NEXUS_AudioMuxOutputSettings settings;
    NEXUS_IsrCallbackHandle overflowCallback;
    struct {
        BMMA_Block_Handle mmaBlock;
    } cdb, itb;
    bool started;
    char name[4];   /* MUX */
#define NEXUS_AUDIO_MUX_OUTPUT_BLOCKS 2
    struct {
        BMMA_Block_Handle mmaBlock;
        NEXUS_MemoryBlockHandle block;
    } block[NEXUS_AUDIO_MUX_OUTPUT_BLOCKS];
} NEXUS_AudioMuxOutput;


static BLST_S_HEAD(MuxOutputList, NEXUS_AudioMuxOutput) g_muxOutputList;

/* Static array defining CDB and ITB sizes for various Buffer Configuration Modes */
static const BAVC_CdbItbConfig g_cdbItbCfg =
{
    { 256 * 1024, 4, true }, { 128 * 1024, 4, false }, false
};

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.transport)

static void NEXUS_AudioMuxOutput_P_Overflow_isr(void *pParam1, int param2);
static void NEXUS_AudioMuxOutput_P_MemoryBlock_Free(unsigned index, NEXUS_AudioMuxOutputHandle handle);

/***************************************************************************
Summary:
    Get default settings for an Audio Mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultCreateSettings(
    NEXUS_AudioMuxOutputCreateSettings *pSettings       /* [out] default settings */
    )
{
    BAPE_MuxOutputCreateSettings apeDefaults;

    if ( NULL == pSettings )
    {
        return;
    }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BAPE_MuxOutput_GetDefaultCreateSettings(&apeDefaults);
    pSettings->numDescriptors = apeDefaults.numDescriptors;
}

/***************************************************************************
Summary:
    Open an Audio Mux Ouptut Handle
***************************************************************************/
NEXUS_AudioMuxOutputHandle NEXUS_AudioMuxOutput_Create(     /* attr{destructor=NEXUS_AudioMuxOutput_Destroy}  */
    const NEXUS_AudioMuxOutputCreateSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_AudioCapabilities audioCaps;
    NEXUS_AudioMuxOutputCreateSettings defaults;
    NEXUS_RaveOpenSettings raveSettings;
    NEXUS_RaveStatus raveStatus;
    NEXUS_AudioMuxOutputHandle handle;
    BAPE_MuxOutputCreateSettings createSettings;
    BAPE_MuxOutputInterruptHandlers interrupts;
    NEXUS_HeapHandle nexusHeap;
    BERR_Code errCode;
    size_t cdbLength, itbLength;
    NEXUS_HeapHandle heap;
    BMMA_Heap_Handle mmaHeap;

    if ( NULL == pSettings )
    {
        NEXUS_AudioMuxOutput_GetDefaultCreateSettings(&defaults);
        pSettings = &defaults;
    }
    
    /* Create Handle */
    handle = BKNI_Malloc(sizeof(NEXUS_AudioMuxOutput));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioMuxOutput));
    NEXUS_OBJECT_INIT(NEXUS_AudioMuxOutput, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "MUX");
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eMux, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.pName = handle->name;
    NEXUS_CallbackDesc_Init(&handle->settings.overflow);

    cdbLength = g_cdbItbCfg.Cdb.Length;
    itbLength = g_cdbItbCfg.Itb.Length;

    NEXUS_GetAudioCapabilities(&audioCaps);
    if ( audioCaps.dsp.codecs[NEXUS_AudioCodec_eLpcm1394].encode )
    {
        /* Default CDB size if 1394 LPCM Encoding is supported needs to be much larger for uncompressed data */
        cdbLength = 700*1024;
        itbLength = 350*1024;
    }
    if ( 0 != pSettings->data.fifoSize )
    {
        cdbLength = pSettings->data.fifoSize;
    }
    if ( 0 != pSettings->index.fifoSize )
    {
        itbLength = pSettings->index.fifoSize;
    }
    if ( itbLength < (cdbLength/2) )
    {
        BDBG_WRN(("Index FIFO Size less than recommended - increasing from %lu to %lu bytes", (unsigned long)itbLength, (unsigned long)cdbLength));
        itbLength = cdbLength/2;
    }

#if NEXUS_AUDIO_MUX_USE_RAVE
    /* Setup rave buffer */
    BSTD_UNUSED(heap);
    BSTD_UNUSED(mmaHeap);
    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    raveSettings.config = g_cdbItbCfg;
    NEXUS_GetAudioCapabilities(&audioCaps);
    raveSettings.config.Cdb.Length = cdbLength;
    raveSettings.config.Itb.Length = itbLength;
    /* audio mux output CDB/ITB heap assigned here for record type (non-secure) RAVE context. */
    raveSettings.heap = NEXUS_P_DefaultHeap(pSettings->data.heap, NEXUS_DefaultHeapType_eFull);
    BDBG_MSG(("rave setting CDB nexus heap = %p", (void *)raveSettings.heap));
    handle->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    if ( NULL == handle->raveContext )
    {
        (void)BERR_TRACE(BERR_UNKNOWN);
        UNLOCK_TRANSPORT();
        goto err_rave_context;
    }
    errCode = NEXUS_Rave_GetStatus_priv(handle->raveContext, &raveStatus);
    UNLOCK_TRANSPORT();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_rave_status;
    }
    BKNI_Memcpy(&handle->contextMap, &raveStatus.xptContextMap, sizeof(raveStatus.xptContextMap));
#else
    BSTD_UNUSED(raveStatus);
    BSTD_UNUSED(raveSettings);

    heap = NEXUS_P_DefaultHeap(pSettings->data.heap, NEXUS_DefaultHeapType_eFull);
    if (!heap)
    {
        heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    }

    mmaHeap = NEXUS_Heap_GetMmaHandle(heap);

    handle->cdb.mmaBlock = BMMA_Alloc(mmaHeap, cdbLength, 8, NULL);
    if (!handle->cdb.mmaBlock) {BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto err_cdb_alloc;}
    handle->itb.mmaBlock = BMMA_Alloc(mmaHeap, itbLength, 8, NULL);
    if (!handle->itb.mmaBlock) {BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto err_itb_alloc;}
#endif

    handle->overflowCallback = NEXUS_IsrCallback_Create(handle, NULL);
    if ( NULL == handle->overflowCallback )
    {
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto err_overflow_callback;
    }

    BAPE_MuxOutput_GetDefaultCreateSettings(&createSettings);
    createSettings.numDescriptors = pSettings->numDescriptors;
#if NEXUS_AUDIO_MUX_USE_RAVE
    createSettings.pContextMap = &handle->contextMap;
    /* NOTE: NEXUS_Rave assumes blockOffset is zero */
    createSettings.cdb.block = raveStatus.cdbBlock;
    createSettings.cdb.size = raveSettings.config.Cdb.Length;
    createSettings.itb.block = raveStatus.itbBlock;
    createSettings.itb.size = raveSettings.config.Itb.Length;
#else
    createSettings.cdb.block = handle->cdb.mmaBlock;
    createSettings.cdb.size = cdbLength;
    createSettings.itb.block = handle->itb.mmaBlock;
    createSettings.itb.size = itbLength;
    createSettings.useRDB = true;
#endif
    nexusHeap = NEXUS_P_DefaultHeap(pSettings->index.heap, NEXUS_DefaultHeapType_eFull);
    if (!nexusHeap)
    {
        nexusHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    }
    createSettings.heaps.descriptor = NEXUS_Heap_GetMmaHandle(nexusHeap);
    errCode = BAPE_MuxOutput_Create(NEXUS_AUDIO_DEVICE_HANDLE, &createSettings, &handle->muxOutput);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_muxoutput_create;
    }
    BAPE_MuxOutput_GetInterruptHandlers(handle->muxOutput, &interrupts);
    interrupts.overflow.pCallback_isr = NEXUS_AudioMuxOutput_P_Overflow_isr;
    interrupts.overflow.pParam1 = handle;
    errCode = BAPE_MuxOutput_SetInterruptHandlers(handle->muxOutput, &interrupts);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_interrupts;
    }
    
    /* Success */
    BLST_S_INSERT_HEAD(&g_muxOutputList, handle, node);
    return handle;

err_interrupts: 
    BAPE_MuxOutput_Destroy(handle->muxOutput);
err_muxoutput_create:
    NEXUS_IsrCallback_Destroy(handle->overflowCallback);
err_overflow_callback:
#if NEXUS_AUDIO_MUX_USE_RAVE
err_rave_status:

    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
err_rave_context:
#else
    BMMA_Free(handle->itb.mmaBlock);
err_itb_alloc:
    BMMA_Free(handle->cdb.mmaBlock);
err_cdb_alloc:
#endif

    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioMuxOutput, handle);
    BKNI_Free(handle);
err_malloc:
    return NULL;
}


/***************************************************************************
Summary:
    Close an Audio Mux Ouptut Handle
***************************************************************************/
static void NEXUS_AudioMuxOutput_P_Finalizer(
    NEXUS_AudioMuxOutputHandle handle
    )
{
    unsigned i;
    NEXUS_OBJECT_ASSERT(NEXUS_AudioMuxOutput, handle);
    BLST_S_REMOVE(&g_muxOutputList, handle, NEXUS_AudioMuxOutput, node);
    for (i=0;i<NEXUS_AUDIO_MUX_OUTPUT_BLOCKS;i++) {
        NEXUS_AudioMuxOutput_P_MemoryBlock_Free(i, handle);
    }
    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BAPE_MuxOutput_Destroy(handle->muxOutput);
    NEXUS_IsrCallback_Destroy(handle->overflowCallback);
#if NEXUS_AUDIO_MUX_USE_RAVE
    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
#else
    BSTD_UNUSED(i);
    BMMA_Free(handle->cdb.mmaBlock);
    handle->cdb.mmaBlock = NULL;
    BMMA_Free(handle->itb.mmaBlock);
    handle->itb.mmaBlock = NULL;
#endif
    NEXUS_OBJECT_DESTROY(NEXUS_AudioMuxOutput, handle);
    BKNI_Free(handle);
}

static void NEXUS_AudioMuxOutput_P_Release(NEXUS_AudioMuxOutputHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioMuxOutput, NEXUS_AudioMuxOutput_Destroy);

/***************************************************************************
Summary:
Get the NEXUS_AudioOutputHandle connector to make upstream connection
    
Description:
NEXUS_AudioMuxOutput can be connected to NEXUS_AudioEncoder (for transcode mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
        
Or it can be connected to NEXUS_AudioDecoder (for passthrough mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioDecoder_GetConnector(audioDecoder));
        
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioMuxOutput_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioMuxOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    return &handle->connector;
}


/***************************************************************************
Summary:
Get default start-time settings for a mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultStartSettings(
    NEXUS_AudioMuxOutputStartSettings *pSettings  /* [out] default settings */
    )
{
    if ( NULL == pSettings )
    {
        return;
    }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
Start capturing data.  

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioMuxOutput_Stop
NEXUS_AudioMuxOutput_GetConnector
***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_Start(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputStartSettings *pSettings
    )
{
    BAPE_MuxOutputStartSettings startSettings;
    NEXUS_AudioMuxOutputStartSettings defaults;
    NEXUS_StcChannelNonRealtimeSettings stcNrtSettings;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    if ( NULL == pSettings )
    {
        NEXUS_AudioMuxOutput_GetDefaultStartSettings(&defaults);
        pSettings = &defaults;
    }

    handle->startSettings = *pSettings;

    BAPE_MuxOutput_GetDefaultStartSettings(&startSettings);
    if ( NULL != pSettings->stcChannel )
    {
        unsigned stcChannelIndex;
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetIndex_priv(pSettings->stcChannel, &stcChannelIndex);
        if ( pSettings->nonRealTime )
        {

            NEXUS_StcChannel_GetSoftIncrementRegOffset_priv(pSettings->stcChannel, &handle->softIncRegisters);
            startSettings.pNonRealTimeIncrement = &handle->softIncRegisters;

            /* set STC to freerunning mode on stop */
            NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv(&stcNrtSettings);
            stcNrtSettings.triggerMode = NEXUS_StcChannelTriggerMode_eSoftIncrement;
            errCode = NEXUS_StcChannel_SetNonRealtimeConfig_priv(pSettings->stcChannel, &stcNrtSettings, true);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                UNLOCK_TRANSPORT();
                goto err_nrt_config;
            }
        }
        UNLOCK_TRANSPORT();
        startSettings.stcIndex = stcChannelIndex;
    }
    startSettings.presentationDelay = pSettings->presentationDelay;
#if NEXUS_AUDIO_MUX_USE_RAVE
    LOCK_TRANSPORT();
    /* Reset RAVE pointers prior to start */
    NEXUS_Rave_Flush_priv(handle->raveContext);
    UNLOCK_TRANSPORT();
#endif
    errCode = BAPE_MuxOutput_Start(handle->muxOutput, &startSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start;
    }

    handle->started = true;

    return BERR_SUCCESS;

err_start:
    if ( NULL != pSettings->stcChannel )
    {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv(&stcNrtSettings);
        stcNrtSettings.triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
        (void)NEXUS_StcChannel_SetNonRealtimeConfig_priv(pSettings->stcChannel, &stcNrtSettings, false);
        UNLOCK_TRANSPORT();
    }
err_nrt_config:
    return errCode;
}

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioMuxOutput_Stop(
    NEXUS_AudioMuxOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);

    if ( !handle->started )
    {
        return;
    }

    BAPE_MuxOutput_Stop(handle->muxOutput);
    handle->started = false;

    if ( NULL != handle->startSettings.stcChannel )
    {
        NEXUS_StcChannelNonRealtimeSettings stcNrtSettings;
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv(&stcNrtSettings);
        stcNrtSettings.triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
        (void)NEXUS_StcChannel_SetNonRealtimeConfig_priv(handle->startSettings.stcChannel, &stcNrtSettings, false);
        UNLOCK_TRANSPORT();
    }
}

static void NEXUS_AudioMuxOutput_P_MemoryBlock_Free(unsigned index, NEXUS_AudioMuxOutputHandle handle)
{
    if (handle->block[index].block) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, handle->block[index].block, Release);
        NEXUS_MemoryBlock_Free(handle->block[index].block);
        handle->block[index].block = NULL;
        handle->block[index].mmaBlock = NULL;
    }
}


static NEXUS_MemoryBlockHandle NEXUS_AudioMuxOutput_P_MemoryBlockFromMma(NEXUS_AudioMuxOutputHandle handle, unsigned index, BMMA_Block_Handle mmaBlock)
{
    if (handle->block[index].mmaBlock == mmaBlock) {
        return handle->block[index].block;
    }
    NEXUS_AudioMuxOutput_P_MemoryBlock_Free(index, handle);
    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.core);
    handle->block[index].block = NEXUS_MemoryBlock_FromMma_priv(mmaBlock);
    NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.core);
    if (handle->block[index].block) {
        handle->block[index].mmaBlock = mmaBlock;
    }
    NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, handle->block[index].block, Acquire);
    return handle->block[index].block;
}

/**
Summary:
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetBufferStatus_priv(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputStatus *pStatus /* [out] */
    )
{
   BERR_Code errCode;
   BAVC_AudioBufferStatus bufferStatus;

   BKNI_Memset(&bufferStatus, 0, sizeof(bufferStatus));

   errCode = BAPE_MuxOutput_GetBufferStatus(handle->muxOutput, &bufferStatus);
   if ( errCode )
   {
       return BERR_TRACE(errCode);
   }

   pStatus->bufferBlock = NEXUS_AudioMuxOutput_P_MemoryBlockFromMma( handle, 0, bufferStatus.stCommon.hFrameBufferBlock );
   pStatus->metadataBufferBlock = NEXUS_AudioMuxOutput_P_MemoryBlockFromMma( handle, 1, bufferStatus.stCommon.hMetadataBufferBlock );

   return BERR_SUCCESS;
}

/**
Summary:
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputStatus *pStatus /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_MuxOutputStatus muxStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);

    if ( NULL == pStatus )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    errCode = NEXUS_AudioMuxOutput_GetBufferStatus_priv(handle, pStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BAPE_MuxOutput_GetStatus(handle->muxOutput, &muxStatus);
    if ( BERR_SUCCESS == errCode )
    {
        pStatus->numFrames = muxStatus.numFrames;
        pStatus->numErrorFrames = muxStatus.numErrorFrames;
        pStatus->data.fifoDepth = muxStatus.data.fifoDepth;
        pStatus->data.fifoSize = muxStatus.data.fifoSize;
        pStatus->index.fifoDepth = muxStatus.index.fifoDepth;
        pStatus->index.fifoSize = muxStatus.index.fifoSize;
        pStatus->numDroppedFrames = muxStatus.numDroppedOverflowFrames;
    }

    return BERR_SUCCESS;
}

/**
Summary:
The mux manager (or other consumer) will call this API to get encoded frames from the NEXUS_AudioMuxOutput
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetBuffer(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputFrame **pBuffer, /* [out] pointer to NEXUS_AudioMuxOutputFrame structs */
    size_t *pSize,  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer */
    const NEXUS_AudioMuxOutputFrame **pBuffer2, /* [out] pointer to NEXUS_AudioMuxOutputFrame structs after wrap around */
    size_t *pSize2  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer2 */
    )
{
    BERR_Code errCode;
    const BAVC_AudioBufferDescriptor *pDesc1, *pDesc2;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    NEXUS_ASSERT_FIELD(NEXUS_AudioMuxOutputFrame, pts, BAVC_AudioBufferDescriptor, stCommon.uiPTS);
    NEXUS_ASSERT_FIELD(NEXUS_AudioMuxOutputFrame, shr, BAVC_AudioBufferDescriptor, stCommon.iSHR);
    NEXUS_ASSERT_FIELD(NEXUS_AudioMuxOutputFrame, offset, BAVC_AudioBufferDescriptor, stCommon.uiOffset);
    NEXUS_ASSERT_FIELD(NEXUS_AudioMuxOutputFrame, rawDataOffset, BAVC_AudioBufferDescriptor, uiRawDataOffset);
    NEXUS_ASSERT_STRUCTURE(NEXUS_AudioMuxOutputFrame, BAVC_AudioBufferDescriptor);
    BDBG_CASSERT(sizeof(NEXUS_AudioMuxOutputFrame) == sizeof(BAVC_AudioBufferDescriptor));
    BDBG_CASSERT(sizeof(NEXUS_AudioMetadataDescriptor) == sizeof(BAVC_AudioMetadataDescriptor));

    if ( NULL == pBuffer || NULL == pSize || NULL == pBuffer2 || NULL == pSize2 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    errCode = BAPE_MuxOutput_GetBufferDescriptors(handle->muxOutput, &pDesc1, pSize, &pDesc2, pSize2);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    *pBuffer = (const NEXUS_AudioMuxOutputFrame *)pDesc1;
    *pBuffer2 = (const NEXUS_AudioMuxOutputFrame *)pDesc2;

    BDBG_MSG(("GetBuffer: Returning %lu descriptors (%lu+%lu)", (unsigned long)*pSize+*pSize2, (unsigned long)*pSize, (unsigned long)*pSize2));

    return BERR_SUCCESS;
}

/**
Summary:
Report how much data returned by last NEXUS_AudioMuxOutput_GetBuffer call was consumed
**/
NEXUS_Error NEXUS_AudioMuxOutput_ReadComplete(
    NEXUS_AudioMuxOutputHandle handle,
    unsigned framesCompleted /* must be <= pSize+pSize2 returned by last NEXUS_AudioMuxOutput_GetBuffer call. */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    
    BDBG_MSG(("ReadComplete: Completed %u frames", framesCompleted));

    errCode = BAPE_MuxOutput_ConsumeBufferDescriptors(handle->muxOutput, framesCompleted);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioMuxOutput_GetDelayStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioMuxOutputDelayStatus *pStatus    /* [out] */
    )
{
    BERR_Code errCode;
    BAVC_AudioCompressionStd avcCodec;
    BAPE_MuxOutputDelayStatus delayStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    BDBG_ASSERT(NULL != pStatus);

    avcCodec = NEXUS_Audio_P_CodecToMagnum(codec);
    errCode = BAPE_MuxOutput_GetDelayStatus(handle->muxOutput, avcCodec, &delayStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    pStatus->endToEndDelay = delayStatus.endToEndDelay;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary: 
    Link audio mux output to a particular node
 ***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_P_AddInput(
    NEXUS_AudioMuxOutputHandle handle, 
    NEXUS_AudioInputHandle input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);

    if (input == NULL || handle->input != NULL)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);  
    }   

    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_MuxOutput_AddInput(handle->muxOutput, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary: 
    Unlink audio mux output from a particular node
 ***************************************************************************/
void NEXUS_AudioMuxOutput_P_RemoveInput(
    NEXUS_AudioMuxOutputHandle handle, 
    NEXUS_AudioInputHandle input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioMuxOutput);
    BDBG_ASSERT(handle->input == input);

    (void)BAPE_MuxOutput_RemoveAllInputs(handle->muxOutput);
    handle->input = NULL;
}

void NEXUS_AudioMuxOutput_P_WatchdogReset(void)
{
    NEXUS_AudioMuxOutputHandle hMuxOutput;

    for ( hMuxOutput = BLST_S_FIRST(&g_muxOutputList);
          hMuxOutput != NULL;
          hMuxOutput = BLST_S_NEXT(hMuxOutput, node) )
    {
        if ( hMuxOutput->started )
        {
            NEXUS_AudioMuxOutput_Stop(hMuxOutput);
            (void)NEXUS_AudioMuxOutput_Start(hMuxOutput, &hMuxOutput->startSettings);
        }
    }
}

void NEXUS_AudioMuxOutput_GetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputSettings *pSettings /* [out] */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioMuxOutput, handle);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_AudioMuxOutput_SetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputSettings *pSettings
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioMuxOutput, handle);
    NEXUS_IsrCallback_Set(handle->overflowCallback, &pSettings->overflow);
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

static void NEXUS_AudioMuxOutput_P_Overflow_isr(void *pParam1, int param2)
{
    NEXUS_AudioMuxOutputHandle handle = pParam1;
    BSTD_UNUSED(param2);
    NEXUS_OBJECT_ASSERT(NEXUS_AudioMuxOutput, handle);
    NEXUS_IsrCallback_Fire_isr(handle->overflowCallback);
}

#else /* #if NEXUS_HAS_AUDIO_MUX_OUTPUT */

typedef struct NEXUS_AudioMuxOutput
{
    NEXUS_OBJECT(NEXUS_AudioMuxOutput);
} NEXUS_AudioMuxOutput;


/* Stubs */

/***************************************************************************
Summary:
    Get default settings for an Audio Mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultCreateSettings(
    NEXUS_AudioMuxOutputCreateSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open an Audio Mux Ouptut Handle
***************************************************************************/
NEXUS_AudioMuxOutputHandle NEXUS_AudioMuxOutput_Create(   /* attr{destructor=NEXUS_AudioMuxOutput_Destroy}  */
    const NEXUS_AudioMuxOutputCreateSettings *pSettings /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}


/***************************************************************************
Summary:
    Close an Audio Mux Ouptut Handle
***************************************************************************/
static void NEXUS_AudioMuxOutput_P_Finalizer(
    NEXUS_AudioMuxOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

static void NEXUS_AudioMuxOutput_P_Release(NEXUS_AudioMuxOutputHandle handle)
{
    BSTD_UNUSED(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioMuxOutput, NEXUS_AudioMuxOutput_Destroy);

/***************************************************************************
Summary:
Get the NEXUS_AudioOutputHandle connector to make upstream connection
    
Description:
NEXUS_AudioMuxOutput can be connected to NEXUS_AudioEncoder (for transcode mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
        
Or it can be connected to NEXUS_AudioDecoder (for passthrough mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioDecoder_GetConnector(audioDecoder));
        
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioMuxOutput_GetConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_AudioMuxOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}


/***************************************************************************
Summary:
Get default start-time settings for a mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultStartSettings(
    NEXUS_AudioMuxOutputStartSettings *pSettings  /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Start capturing data.  

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioMuxOutput_Stop
NEXUS_AudioMuxOutput_GetConnector
***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_Start(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioMuxOutput_Stop(
    NEXUS_AudioMuxOutputHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/**
Summary:
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetBufferStatus_priv(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputStatus *pStatus /* [out] */
    )
{
   BSTD_UNUSED(handle);
   BSTD_UNUSED(pStatus);
   return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**
Summary:
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputStatus *pStatus /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**
Summary:
The mux manager (or other consumer) will call this API to get encoded frames from the NEXUS_AudioMuxOutput
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetBuffer(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputFrame **pBuffer, /* [out] pointer to NEXUS_AudioMuxOutputFrame structs */
    size_t *pSize, /* [out] size of pBuffer in bytes (not number of NEXUS_AudioMuxOutputFrame structs) */
    const NEXUS_AudioMuxOutputFrame **pBuffer2, /* [out] pointer to NEXUS_AudioMuxOutputFrame structs after wrap around */
    size_t *pSize2 /* [out] size of pBuffer2 in bytes (not number of NEXUS_AudioMuxOutputFrame structs) */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pSize2);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/**
Summary:
Report how much data returned by last NEXUS_AudioMuxOutput_GetBuffer call was consumed
**/
NEXUS_Error NEXUS_AudioMuxOutput_ReadComplete(
    NEXUS_AudioMuxOutputHandle handle,
    unsigned framesCompleted /* must be <= pSize+pSize2 returned by last NEXUS_AudioMuxOutput_GetBuffer call. */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(framesCompleted);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioMuxOutput_GetDelayStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioMuxOutputDelayStatus *pStatus    /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(codec);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioMuxOutput_P_WatchdogReset(void)
{
    return;
}

void NEXUS_AudioMuxOutput_GetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioMuxOutput_SetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioMuxOutput_P_AddInput(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioMuxOutput_P_RemoveInput(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return;
}

#endif /* #if NEXUS_HAS_AUDIO_MUX_OUTPUT */
