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
 **************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_recpump_impl.h"
#include "priv/nexus_core.h"
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
#include "priv/nexus_dma_priv.h"
#endif
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_recpump);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define NEXUS_P_FLOW_NAME(flow) ((flow)==&(flow)->recpump->data ? "CDB" : "ITB")

/* XPT RAVE is configured with a fixed size for wrapping */
#define NEXUS_RAVE_MEMC_BLOCK_SIZE BXPT_RAVE_WRAP_THRESH

typedef enum NEXUS_Recpump_P_BufferState {
    NEXUS_Recpump_P_BufferState_eApplicationNotified,
    NEXUS_Recpump_P_BufferState_eWaitingApplication,
    NEXUS_Recpump_P_BufferState_eNoData,
    NEXUS_Recpump_P_BufferState_eError,
    NEXUS_Recpump_P_BufferState_eTerminated,
    NEXUS_Recpump_P_BufferState_eMax
} NEXUS_Recpump_P_BufferState;

static NEXUS_Error NEXUS_Recpump_P_Start(NEXUS_RecpumpHandle r);
static NEXUS_Error NEXUS_Recpump_P_StartFlow(struct NEXUS_RecpumpFlow *flow);
static void        NEXUS_Recpump_P_StopFlow(struct NEXUS_RecpumpFlow *flow);
static NEXUS_Recpump_P_BufferState NEXUS_Recpump_P_TestDataReadyCallback(struct NEXUS_RecpumpFlow *flow);
static NEXUS_Error NEXUS_Recpump_P_ApplyTpitStartSettings(NEXUS_RecpumpHandle r);

#define NEXUS_RECPUMP_PID_IS_INDEX(PSETTINGS) \
    ( (((PSETTINGS)->pidType == NEXUS_PidType_eVideo) && ((PSETTINGS)->pidTypeSettings.video.index)) || \
      (((PSETTINGS)->pidType == NEXUS_PidType_eOther) && ((PSETTINGS)->pidTypeSettings.other.index)) \
    )

#define B_MAX(x,y) (((x)>=(y))?(x):(y))

void NEXUS_Recpump_GetDefaultOpenSettings(NEXUS_RecpumpOpenSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* The default bufferSize is based on the least common multiple of a 188 byte transport packet and 4096 atomSize (for O_DIRECT file i/o).
    Then, multiply by 12 to get 12 of these LCM's. That's the default buffer.
    Finally, add 68 in order to anticipate the NEXUS_RAVE_MEMC_BLOCK_SIZE replacement logic in NEXUS_Recpump_P_Start. 256-188 = 68. This is not a crucial step. We could have chosen
    to not add it, but it would have meant that the final buffer would be less than 12 LCM's after the NEXUS_RAVE_MEMC_BLOCK_SIZE replacement logic. */
    pSettings->data.bufferSize = ((188/4)*4096*12) + 68;
    pSettings->data.alignment = 12; /* 4K alignment */
    pSettings->data.dataReadyThreshold = (188/4)*4096; /* threshold is the least common multiple of a 188 byte transport packet and 4096 atomSize */
    pSettings->data.atomSize = 4096; /* typical value for disk direct I/O */

    /* 6*4 is a size of the single 6-word SCT entry. 16 SCT's per atom. index.bufferSize must be multiple of this. */
    pSettings->index.bufferSize = 6*4*16*48;
    pSettings->index.alignment = 7; /* 128-Byte alignment */
    pSettings->index.dataReadyThreshold = 1024;
    pSettings->index.atomSize = 0; /* typically not needed. index write benefits from stdio buffering. */
}

void NEXUS_Recpump_GetDefaultSettings( NEXUS_RecpumpSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->data.dataReady);
    NEXUS_CallbackDesc_Init(&pSettings->data.overflow);
    NEXUS_CallbackDesc_Init(&pSettings->index.dataReady);
    NEXUS_CallbackDesc_Init(&pSettings->index.overflow);
#if BXPT_NUM_TSIO
    NEXUS_CallbackDesc_Init(&pSettings->tsioDmaEnd);
#endif
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
    pSettings->securityDmaDataFormat = NEXUS_DmaDataFormat_eMpeg;
#endif
    pSettings->outputTransportType = NEXUS_TransportType_eTs;
    pSettings->bandHold = NEXUS_RecpumpFlowControl_eAuto;
    pSettings->localTimestamp = false;
    return;
}

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
static void
NEXUS_Recpump_P_DmaCallback(void *context)
{
    NEXUS_RecpumpHandle pump=context;
    BDBG_OBJECT_ASSERT(pump, NEXUS_Recpump);
    if(pump->crypto.tail != pump->crypto.head) { /*  DMA was pending */
        bool pending = pump->data.pending;
        BDBG_MSG(("NEXUS_Recpump_P_DmaCallback:%#lx DMA completed (%#lx:%#lx) %s", (unsigned long)pump, (unsigned long)pump->crypto.tail, (unsigned long)pump->crypto.head, pending?"":"Waiting For Data"));
        pump->crypto.tail = pump->crypto.head;
        if(NEXUS_Recpump_P_TestDataReadyCallback(&pump->data)!=NEXUS_Recpump_P_BufferState_eApplicationNotified && !pending ) {
            BDBG_MSG(("dma complete but there is no data in the record buffer"));
        }
    }
    return;
}
#endif

static void
NEXUS_Recpump_isr(void *flow_, int parm2 )
{
    struct NEXUS_RecpumpFlow *flow = flow_;
    BSTD_UNUSED(parm2);
    BKNI_SetEvent_isr(flow->event);
    BDBG_ASSERT(flow->irqEnabled);
    BINT_DisableCallback_isr(flow->irq);
    flow->irqEnabled = false;
}

static void
NEXUS_Recpump_Overflow_isr(void *flow_, int parm2 )
{
    struct NEXUS_RecpumpFlow *flow = flow_;
    BSTD_UNUSED(parm2);
    NEXUS_IsrCallback_Fire_isr(flow->overflowCallback);
    BINT_DisableCallback_isr(flow->overflow_irq);
}

#if BXPT_NUM_TSIO
static void
NEXUS_Recpump_TsioDmaEnd_isr(void *context, int parm2 )
{
    NEXUS_RecpumpHandle pump=context;
    BSTD_UNUSED(parm2);
    NEXUS_IsrCallback_Fire_isr(pump->tsioDmaEndCallback);
    BINT_DisableCallback_isr(pump->tsioDmaEndIrq);
}
#endif

/*
if the app specifics a buffer, use it.
if the app specifies a heap, then nexus does the alloc.
if neither, xpt does the alloc. */
static NEXUS_Error
NEXUS_Recpump_P_AllocContextMem(BXPT_Rave_AllocCxSettings *allocSettings, const NEXUS_RecpumpOpenSettings *pSettings, bool allowUserAllocation,
    struct NEXUS_RecpumpBufferAlloc *alloc)
{
    NEXUS_Error rc;
    bool useSecureHeap;

    BKNI_Memset(alloc, 0, sizeof(*alloc));

    rc = NEXUS_Rave_P_UseSecureHeap(pSettings->data.heap, pSettings->useSecureHeap, &useSecureHeap);
    if (rc) return BERR_TRACE(rc);

    if (pSettings->indexType == NEXUS_RecpumpIndexType_eEs) {
        allocSettings->RequestedType = BXPT_RaveCx_eAv;
    }
    else if (useSecureHeap) {
        allocSettings->RequestedType = BXPT_RaveCx_eRecordR;
    }
    else {
        allocSettings->RequestedType = BXPT_RaveCx_eRecord;
    }

    if (allowUserAllocation && pSettings->data.buffer) {
        NEXUS_MemoryBlockHandle block;
        unsigned blockOffset;
        NEXUS_Module_Lock(g_NEXUS_Transport_P_State.moduleSettings.core);
        rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv((void*)pSettings->data.buffer, pSettings->data.bufferSize, &block, &blockOffset);
        NEXUS_Module_Unlock(g_NEXUS_Transport_P_State.moduleSettings.core);
        if (rc) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        allocSettings->CdbBlock = NEXUS_MemoryBlock_GetBlock_priv(block);
        allocSettings->CdbBlockOffset = blockOffset;
        alloc->data.block = allocSettings->CdbBlock;
        alloc->data.buffer = BMMA_Lock(alloc->data.block);
        alloc->data.offset = BMMA_LockOffset(alloc->data.block);
    }
    else if (pSettings->data.bufferSize) {
        NEXUS_HeapHandle heap = useSecureHeap ? pTransport->moduleSettings.secureHeap : pSettings->data.heap;
        heap = NEXUS_P_DefaultHeap(heap, NEXUS_DefaultHeapType_eFull);
        if (!heap) heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
        alloc->data.mmaHeap = NEXUS_Heap_GetMmaHandle(heap);
        alloc->data.block = allocSettings->CdbBlock = BMMA_Alloc(alloc->data.mmaHeap, pSettings->data.bufferSize, 1 << pSettings->data.alignment, NULL);
        if (!alloc->data.block) { rc=BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }
        alloc->data.buffer = BMMA_Lock(alloc->data.block);
        alloc->data.offset = BMMA_LockOffset(alloc->data.block);
        BDBG_MSG(("alloc cdb buffer %p", alloc->data.buffer));
    }
    else {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    if (allowUserAllocation && pSettings->index.buffer) {
        NEXUS_MemoryBlockHandle block;
        unsigned blockOffset;
        NEXUS_Module_Lock(g_NEXUS_Transport_P_State.moduleSettings.core);
        rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv((void*)pSettings->index.buffer, pSettings->index.bufferSize, &block, &blockOffset);
        NEXUS_Module_Unlock(g_NEXUS_Transport_P_State.moduleSettings.core);
        if (rc) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        allocSettings->ItbBlock = NEXUS_MemoryBlock_GetBlock_priv(block);
        allocSettings->ItbBlockOffset = blockOffset;
        alloc->index.block = allocSettings->ItbBlock;
        alloc->index.buffer = BMMA_Lock(alloc->index.block);
        alloc->index.offset = BMMA_LockOffset(alloc->index.block);
    }
    else if (pSettings->index.bufferSize && pSettings->indexType != NEXUS_RecpumpIndexType_eNone) {
        NEXUS_HeapHandle heap;
        heap = NEXUS_P_DefaultHeap(pSettings->index.heap, NEXUS_DefaultHeapType_eFull);
        if (!heap) heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
        alloc->index.mmaHeap = NEXUS_Heap_GetMmaHandle(heap);
        alloc->index.block = allocSettings->ItbBlock = BMMA_Alloc(alloc->index.mmaHeap, pSettings->index.bufferSize, 1 << pSettings->index.alignment, NULL);
        if (!alloc->index.block) { rc=BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }
        alloc->index.buffer = BMMA_Lock(alloc->index.block);
        alloc->index.offset = BMMA_LockOffset(alloc->index.block);
        BDBG_MSG(("alloc itb buffer %p", alloc->index.buffer));
    }
    else {
        allocSettings->BufferCfg.Itb.Length = 0;
    }

    return NEXUS_SUCCESS;

error:
    if (alloc->index.block) {
        BMMA_Unlock(alloc->index.block, alloc->index.buffer);
        BMMA_UnlockOffset(alloc->index.block, alloc->index.offset);
        if (alloc->index.mmaHeap) { /* only free if nexus allocated */
            BMMA_Free(alloc->index.block);
        }
    }

    if (alloc->data.block) {
        BMMA_Unlock(alloc->data.block, alloc->data.buffer);
        BMMA_UnlockOffset(alloc->data.block, alloc->data.offset);
        if (alloc->data.mmaHeap) { /* only free if nexus allocated */
            BMMA_Free(alloc->data.block);
        }
    }
    return rc;
}

static void NEXUS_Recpump_P_ControlIrq(struct NEXUS_RecpumpFlow *flow, NEXUS_Recpump_P_BufferState  bufferState)
{
    BKNI_EnterCriticalSection();
    if(flow->irqEnabled) {
       if(bufferState !=  NEXUS_Recpump_P_BufferState_eNoData) {
            BINT_DisableCallback_isr(flow->irq);
            flow->irqEnabled = false;
       }
    } else {
        if(!flow->recpump->dataStopped) { /* only re-enable interrupts if accepting new data */
            if(bufferState == NEXUS_Recpump_P_BufferState_eNoData) {
                BINT_EnableCallback_isr(flow->irq);
                flow->irqEnabled = true;
            }
        }
    }
    BKNI_LeaveCriticalSection();
    return;
}

static void
NEXUS_Recpump_P_RaveEvent(void *flow_)
{
    struct NEXUS_RecpumpFlow *flow = flow_;
    NEXUS_Recpump_P_BufferState  bufferState;
    BDBG_MSG(("NEXUS_Recpump_P_RaveEvent:%#lx %s", (unsigned long)flow, NEXUS_P_FLOW_NAME(flow)));

    /* it's normal that we may get an event even though there's no dataready callback. there may be a rave event set
    while a WriteComplete finishes. no need to test here. */
    bufferState = NEXUS_Recpump_P_TestDataReadyCallback(flow);
    /* there is no promise of atomicity between  NEXUS_Recpump_isr and  NEXUS_Recpump_P_RaveEvent so interrupt could be wery well enabled by now */
    NEXUS_Recpump_P_ControlIrq(flow, bufferState);
    return;
}

NEXUS_RecpumpHandle NEXUS_Recpump_Open(unsigned index, const NEXUS_RecpumpOpenSettings *pSettings)
{
    NEXUS_Error rc = 0;
    BXPT_Rave_AllocCxSettings allocSettings;
    BINT_Id int_id;
    NEXUS_RecpumpHandle r;
    NEXUS_RecpumpOpenSettings settings;

    if (index == NEXUS_ANY_ID) {
        unsigned i;
        for (i=0;i<BXPT_NUM_RAVE_CONTEXTS;i++) {
            if (!pTransport->recpump[i]) {
                index = i;
                break;
            }
        }
        if (i == BXPT_NUM_RAVE_CONTEXTS) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            BDBG_ERR(("no recpump not available"));
            return NULL;
        }
    }

    if (index >= BXPT_NUM_RAVE_CONTEXTS) {
        BDBG_ERR(("recpump %d not supported", index));
        return NULL;
    }
    if (pTransport->recpump[index]) {
        BDBG_ERR(("recpump %d already open", index));
        return NULL;
    }

    if (!pSettings) {
        NEXUS_Recpump_GetDefaultOpenSettings(&settings);
        pSettings = &settings;
    }

    BDBG_MSG(("NEXUS_Recpump_Open %d: cdb=%u (%d threshold), itb=%u (%d threshold)",
        index,
        (unsigned)pSettings->data.bufferSize, pSettings->data.dataReadyThreshold,
        (unsigned)pSettings->index.bufferSize, pSettings->index.dataReadyThreshold));

    /* Ensure required numbers */
    if (pSettings->data.bufferSize < NEXUS_RAVE_MEMC_BLOCK_SIZE*2) {
        BDBG_ERR(("data.bufferSize %u invalid", (unsigned)pSettings->data.bufferSize));
        return NULL;
    }
    if (pSettings->data.dataReadyThreshold > pSettings->data.bufferSize) {
        BDBG_ERR(("data.dataReadyThreshold %d invalid", pSettings->data.dataReadyThreshold));
        return NULL;
    }
    /* index.bufferSize can be 0. Otherwise, it must be valid. */
    if (pSettings->index.bufferSize) {
        if (pSettings->index.bufferSize < NEXUS_RAVE_MEMC_BLOCK_SIZE*2) {
            BDBG_ERR(("index.bufferSize %u invalid", (unsigned)pSettings->index.bufferSize));
            return NULL;
        }
        if (pSettings->index.dataReadyThreshold > pSettings->index.bufferSize) {
            BDBG_ERR(("index.dataReadyThreshold %d invalid", pSettings->index.dataReadyThreshold));
            return NULL;
        }
    }
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(recpump,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    /* Warn on dangerous numbers */
    if (pSettings->data.dataReadyThreshold * 100 / pSettings->data.bufferSize > 50) {
        BDBG_WRN(("data.dataReadyThreshold is very large. Overflow likely."));
    }
    if (pSettings->index.bufferSize && pSettings->index.dataReadyThreshold * 100 / pSettings->index.bufferSize > 50) {
        BDBG_WRN(("index.dataReadyThreshold is very large. Overflow likely."));
    }

    r = (NEXUS_RecpumpHandle)BKNI_Malloc(sizeof(*r));
    if (!r) {
        NEXUS_CLIENT_RESOURCES_RELEASE(recpump,Count,NEXUS_ANY_ID);
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    pTransport->recpump[index] = r;
    NEXUS_OBJECT_INIT(NEXUS_Recpump, r);
    r->tindex = index;
    r->openSettings = *pSettings;
    r->inputTransportType = NEXUS_TransportType_eTs;
    r->scdIdx = NULL;
    r->scdPidCount = 0;
    r->scdUsedMap = 0;
    r->scdMapMode = -1;
    NEXUS_Recpump_GetDefaultSettings(&r->settings);

    BXPT_Rave_GetDefaultAllocCxSettings(&allocSettings);
    allocSettings.CdbBlock = NULL;
    allocSettings.ItbBlock = NULL;
    allocSettings.BufferCfg.Cdb.Length = pSettings->data.bufferSize;
    allocSettings.BufferCfg.Itb.Length = pSettings->index.bufferSize;
    allocSettings.BufferCfg.Cdb.Alignment = pSettings->data.alignment;
    allocSettings.BufferCfg.Itb.Alignment = pSettings->index.alignment;

    /* The end result we want on little endian or big endian CPU's is ITB in host endianness, CDB as big endian (byte stream).
    To get this, we counter-intuitively program RAVE to output ITB as big endian, CDB in host endianness.
    A little endian CPU will swap on read, so ITB will be seen in host endianness and CDB as big endian.
    A big endian CPU will not swap on read, so ITB will be seen in host endianness and CDB as big endian. The same result. */
    allocSettings.BufferCfg.Itb.LittleEndian = false;
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
    allocSettings.BufferCfg.Cdb.LittleEndian = true;
#else
    allocSettings.BufferCfg.Cdb.LittleEndian = false;
#endif

    if (pSettings->dummyRecpump) {
        /* alloc soft context for ITB-only record. CDB of dummyRecpump will be used as bit bucket. */
        allocSettings.SoftRaveMode = BXPT_RaveSoftMode_eIndexOnlyRecord;
        allocSettings.SoftRaveSrcCx = pSettings->dummyRecpump->rave_rec;
        if (pSettings->data.heap && pSettings->data.bufferSize != 0) {
            /* if CDB is bit bucket, user should not be allocating memory for it. */
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error;
        }
    }
    else {
        allocSettings.SoftRaveSrcCx = NULL;
    }
    rc = NEXUS_Recpump_P_AllocContextMem(&allocSettings, pSettings, true, &r->rave_rec_mem);
    if (rc) { goto error; }

    rc = BXPT_Rave_AllocCx(pTransport->rave[0].channel, &allocSettings, &r->rave_rec);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    /* nullifyVct is only needed in limited uses. please ask for FAE for more usage information.
    at build time, you must export BXPT_VCT_SUPPORT=y.
    set nullifyVct = 0 to disable the feature.
    set nullifyVct = 1 for TVCT mode.
    set nullifyVct = 2 for CVCT mode. */
    if (pSettings->nullifyVct) {
#ifdef BXPT_VCT_SUPPORT
        /* an extra, unused RAVE context is allocated immediately after rave_rec so that RAVE FW has extra CXMEM to work with */
        allocSettings.SoftRaveSrcCx = NULL;
        rc = NEXUS_Recpump_P_AllocContextMem(&allocSettings, pSettings, false, &r->extra_rave_rec_mem);
        if (rc) { goto error; }

        rc = BXPT_Rave_AllocCx(pTransport->rave[0].channel, &allocSettings, &r->rave_rec_extra);
        if (rc) { rc=BERR_TRACE(rc); goto error; }
#else
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
#endif
    }

    /* get bufferBase for data and index buffers */
    {
    BAVC_XptContextMap context;
    BSTD_DeviceOffset offset;
    rc = BXPT_Rave_GetContextRegisters(r->rave_rec, &context);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    r->rave_rec_index = NEXUS_RAVE_INDEX(context.CDB_Read);

    offset = BREG_ReadAddr(g_pCoreHandles->reg, context.CDB_Base);
    r->data.bufferBase = (uint8_t*)r->rave_rec_mem.data.buffer + (offset - r->rave_rec_mem.data.offset); /* convert offset -> cached ptr */
    r->data.flushableBuffer = NEXUS_P_CpuAccessibleAddress(r->data.bufferBase);

    if (r->rave_rec_mem.index.buffer) {
        offset = BREG_ReadAddr(g_pCoreHandles->reg, context.ITB_Base);
        r->index.bufferBase = (uint8_t*)r->rave_rec_mem.index.buffer + (offset - r->rave_rec_mem.index.offset); /* convert offset -> cached ptr */
        r->index.flushableBuffer = NEXUS_P_CpuAccessibleAddress(r->index.bufferBase);
    }
    }

    rc = BKNI_CreateEvent(&r->data.event);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    r->data.eventCallback = NEXUS_RegisterEvent(r->data.event, NEXUS_Recpump_P_RaveEvent, &r->data);
    if (!r->data.eventCallback) { rc=BERR_TRACE(NEXUS_UNKNOWN);goto error;}

    r->data.irqEnabled = false;
#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eCdbMinDepthThresh, &int_id);
#else
    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eCdbUpperThresh, &int_id);
#endif
    rc = BINT_CreateCallback(&r->data.irq, g_pCoreHandles->bint, int_id, NEXUS_Recpump_isr, &r->data, 0);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eCdbOverflow, &int_id);
    rc = BINT_CreateCallback(&r->data.overflow_irq, g_pCoreHandles->bint, int_id, NEXUS_Recpump_Overflow_isr, &r->data, 0);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    rc = BKNI_CreateEvent(&r->index.event);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    r->index.eventCallback = NEXUS_RegisterEvent(r->index.event, NEXUS_Recpump_P_RaveEvent, &r->index);
    if (!r->index.eventCallback) { rc=BERR_TRACE(NEXUS_UNKNOWN);goto error;}

    r->index.irqEnabled = false;
#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eItbMinDepthThresh, &int_id);
#else
    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eItbUpperThresh, &int_id);
#endif
    rc = BINT_CreateCallback(&r->index.irq, g_pCoreHandles->bint, int_id, NEXUS_Recpump_isr, &r->index, 0);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eItbOverflow, &int_id);
    rc = BINT_CreateCallback(&r->index.overflow_irq, g_pCoreHandles->bint, int_id, NEXUS_Recpump_Overflow_isr, &r->index, 0);
    if (rc) { rc=BERR_TRACE(rc); goto error; }

#if BXPT_NUM_TSIO
    BXPT_Rave_GetIntId(r->rave_rec, BXPT_RaveIntName_eTsioDmaEnd, &int_id);
    rc = BINT_CreateCallback(&r->tsioDmaEndIrq, g_pCoreHandles->bint, int_id, NEXUS_Recpump_TsioDmaEnd_isr, (void *) r, 0);
    if (rc) { rc=BERR_TRACE(rc); goto error; }
    r->tsioDmaEndCallback = NEXUS_IsrCallback_Create(r, NULL);
#endif
#if NEXUS_ENCRYPTED_DVR_WITH_M2M && (!NEXUS_HAS_XPT_DMA)
    rc = BKNI_CreateEvent(&r->crypto.event);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    r->crypto.callback = NEXUS_RegisterEvent(r->crypto.event, NEXUS_Recpump_P_DmaCallback, r);
    if(!r->crypto.callback) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
#endif

    r->data.bufferSize = pSettings->data.bufferSize;
    r->data.dataReadyThreshold = pSettings->data.dataReadyThreshold;
    r->data.atomSize = pSettings->data.atomSize;
    /* r->data.atomSize = 0; */
    r->data.overflowCallback = NEXUS_IsrCallback_Create(r, NULL);
    r->data.dataReadyCallback = NEXUS_TaskCallback_Create(r, NULL);

    r->index.bufferSize = pSettings->index.bufferSize;
    r->index.dataReadyThreshold = pSettings->index.dataReadyThreshold;
    r->index.atomSize = pSettings->index.atomSize;
    r->index.overflowCallback = NEXUS_IsrCallback_Create(r, NULL);
    r->index.dataReadyCallback = NEXUS_TaskCallback_Create(r, NULL);

    /* init the callbacks */
    NEXUS_Recpump_SetSettings(r, &r->settings);

    r->dataStopped = false;
    r->data.recpump = r;
    r->index.recpump = r;

    BLST_S_INIT(&r->pid_list);

    rc = NEXUS_RaveErrorCounter_Init_priv(&r->raveErrors, r->rave_rec);
    if (rc) {
        goto error;
    }

    if (r->openSettings.dummyRecpump) {
        NEXUS_OBJECT_ACQUIRE(r, NEXUS_Recpump, r->openSettings.dummyRecpump);
    }

    return r;

error:
    NEXUS_Recpump_Close(r);
    return NULL;
}

static NEXUS_Error NEXUS_Recpump_P_ProvisionTpitIndexer(NEXUS_RecpumpHandle r)
{
    NEXUS_Error rc;
    bool needed = r->settings.tpit.idleEventEnable
        || r->settings.tpit.firstPacketEnable
        || r->settings.tpit.recordEventEnable;
    if (!needed) {
        int i;
        for (i = 0; i < NEXUS_NUM_ECM_TIDS; i++) {
            if (r->settings.tpit.ecmPair[i].enabled) {
                needed = true;
            }
        }
    }
    if (!needed) {
        NEXUS_Recpump_P_PidChannel *pid;
        for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
            if (pid->tpit.enabled) {
                needed = true;
                break;
            }
        }
    }

    if (needed && !r->tpitIdx) {
        rc = BXPT_Rave_AllocIndexer(pTransport->rave[0].channel, BXPT_RaveIdx_eTpit, 1, r->rave_rec, &r->tpitIdx);
        if (rc) return BERR_TRACE(rc);
    }
    else if (!needed && r->tpitIdx) {
        BXPT_Rave_FreeIndexer(r->tpitIdx);
        r->tpitIdx = NULL;
    }
    return NEXUS_SUCCESS;
}

static void
NEXUS_Recpump_P_FreeScdIndexer(NEXUS_RecpumpHandle r)
{
    if(r->scdIdx) {
        NEXUS_Recpump_P_PidChannel *pid;
        BXPT_Rave_FreeIndexer(r->scdIdx);
        r->scdPidCount = 0;
        r->scdIdx = NULL;

        for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
            pid->assignedScd = -1;
        }
        r->scdUsedMap = 0;
        r->scdMapMode = -1;
    }
}

static NEXUS_Error NEXUS_Recpump_P_ProvisionScdIndexer(NEXUS_RecpumpHandle r)
{
    NEXUS_Error rc;
    unsigned scdNo;
    NEXUS_Recpump_P_PidChannel *pid;
    unsigned requiredScds = 0;
    int newMapMode = -1;

    /* 1. Count the indexed pids, and detect the (new) pid mapping mode needed */

    for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if (NEXUS_RECPUMP_PID_IS_INDEX(&pid->settings)) {
            requiredScds++;
            if (newMapMode >= 0) {
                bool requiredMode = pid->settings.pidType == NEXUS_PidType_eVideo && pid->settings.pidTypeSettings.video.pid == 0x1fff;
                if (requiredMode != newMapMode) {
                    BDBG_ERR(("Mixed video indexing modes not supported"));
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
            }
            else {
                newMapMode = pid->settings.pidType == NEXUS_PidType_eVideo && pid->settings.pidTypeSettings.video.pid == 0x1fff;
            }
        }
    }

    BDBG_MSG(("requiredScds %d newMapMode %d", requiredScds, newMapMode));

    /* 2. Allocate SCD indexer for desired number of pids (we only increase  number of SCD) */

    if(requiredScds>r->scdPidCount) {
        if (r->actuallyStarted && r->scdUsedMap) {
            BDBG_WRN(("reallocating indexers after start: event loss likely"));
        }
        NEXUS_Recpump_P_FreeScdIndexer(r);
        rc = BXPT_Rave_AllocIndexer(pTransport->rave[0].channel, BXPT_RaveIdx_eScd, requiredScds, r->rave_rec, &r->scdIdx);
        if (rc) { return BERR_TRACE(rc); }
        r->scdPidCount = requiredScds;
    }
    /* What is SetPidChannelSettings turns off the last index? NEXUS_Recpump_P_FreeScdIndexer won't be called until we remove the last pidchannel. Is that ok? */

    /* 3. For each indexed pid, make allocation state match requirement  */

    for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        BXPT_Rave_IndexerSettings indx_cfg;
        BXPT_Rave_ScdEntry ScdConfig;
        const NEXUS_RecpumpPidChannelSettings *pSettings = &pid->settings;

        if (!pid->pidChn) continue;

        if (!NEXUS_RECPUMP_PID_IS_INDEX(&pid->settings)) {
            if (pid->assignedScd >= 0) {
                /* Detector assigned, but not needed. Disable this detector */
                BKNI_Memset(&ScdConfig, 0, sizeof(ScdConfig));
                /* use previous (not new) mapMode to control disable */
                if (r->scdMapMode == 0) {
                    BXPT_Rave_SetScdUsingPid(r->scdIdx, pid->assignedScd, 0x1fff, &ScdConfig);
                }
                else if (r->scdMapMode == 1) {
                    ScdConfig.PidChannel = 0x1fff;
                    BXPT_Rave_SetScdEntry(r->scdIdx, pid->assignedScd, &ScdConfig);
                }
                /* then free it */
                r->scdUsedMap &= ~(1 << pid->assignedScd);
                pid->assignedScd = -1;
            }
            continue;
        }

        if (pid->assignedScd < 0) {
            /* No detector assigned, but needed. Search bitmap for free detector */
            unsigned mask;
            for (scdNo = 0, mask = 1; scdNo < r->scdPidCount; scdNo++, mask <<= 1) {
                if ((r->scdUsedMap & mask) == 0) {
                    break;
                }
            }
            BDBG_ASSERT(scdNo < r->scdPidCount);
        }
        else {
            scdNo = pid->assignedScd;
        }

        /* scdNo is the free or existing detector - reprogram it */

        BKNI_Memset(&ScdConfig, 0, sizeof(ScdConfig));

        if (pSettings->pidType == NEXUS_PidType_eVideo && pSettings->pidTypeSettings.video.index) {

            if (!r->rave_rec_mem.index.buffer) {
                BDBG_ERR(("No index buffer allocated"));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }

            /* Set Index Configuration */
            BDBG_ASSERT(r->scdIdx);
            if(scdNo==0) {
                bool svcVideo = false;
                /* We are only capturing Index Data on the main video PID*/
                rc = BXPT_Rave_GetIndexerConfig( r->scdIdx, &indx_cfg);
                if (rc) {return BERR_TRACE(rc);}


                switch (pSettings->pidTypeSettings.video.codec) {
                case NEXUS_VideoCodec_eH264_Mvc:
                case NEXUS_VideoCodec_eH264_Svc:
                    svcVideo = true;
                    /* fallthrough */
                case NEXUS_VideoCodec_eH265:
                case NEXUS_VideoCodec_eH264:
                case NEXUS_VideoCodec_eH263:
                case NEXUS_VideoCodec_eVc1:
                case NEXUS_VideoCodec_eVc1SimpleMain:
                    /* 8 bytes of post-SC data is the max that can be captured.
                    AVC requires 59 bits, so we need them all. */
                    indx_cfg.Cfg.Scd.ScRange[0].RangeHi = 0x7F;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeLo = 0x00;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeIsASlice = false;
                    break;
                case NEXUS_VideoCodec_eMpeg2:
                    indx_cfg.Cfg.Scd.ScRange[0].RangeHi = 0xB7;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeLo = 0xB3;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeIsASlice = false;
#if BXPT_HAS_AVS
    /* for AVS-capable chips, we must explicitly enable SC 0x00 for MPEG */
                    indx_cfg.Cfg.Scd.ScRange[1].RangeHi = 0x01; /* must allow 0x01 for HITS */
                    indx_cfg.Cfg.Scd.ScRange[1].RangeLo = 0x00;
                    indx_cfg.Cfg.Scd.ScRange[1].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[1].RangeIsASlice = false;
#endif
                    break;
                case NEXUS_VideoCodec_eAvs:
                    /* sequence start and end */
                    indx_cfg.Cfg.Scd.ScRange[0].RangeHi = 0xB1;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeLo = 0xB0;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[0].RangeIsASlice = false;
                    /* I picture */
                    indx_cfg.Cfg.Scd.ScRange[1].RangeHi = 0xB3;
                    indx_cfg.Cfg.Scd.ScRange[1].RangeLo = 0xB3;
                    indx_cfg.Cfg.Scd.ScRange[1].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[1].RangeIsASlice = false;
                    /* P and B pictures */
                    indx_cfg.Cfg.Scd.ScRange[2].RangeHi = 0xB6;
                    indx_cfg.Cfg.Scd.ScRange[2].RangeLo = 0xB6;
                    indx_cfg.Cfg.Scd.ScRange[2].RangeEnable = true;
                    indx_cfg.Cfg.Scd.ScRange[2].RangeIsASlice = false;
                    break;
                default:
                    BDBG_ERR(("Video Format not supported"));
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                }
#if BXPT_HAS_SVC_MVC
                indx_cfg.Cfg.Scd.SvcMvcMode = svcVideo;
#endif
                rc = BXPT_Rave_SetIndexerConfig(r->scdIdx, &indx_cfg);
                if (rc) {return BERR_TRACE(rc);}
            }

            ScdConfig.PidChannel = pid->pidChn->hwPidChannel->status.pidChannelIndex;
            ScdConfig.EsCount = 9; /* EsCount 9 means 1 start code + 8 bytes of payload after the start code, which is the maximum allowed by 6 word SCT */
            ScdConfig.ExtractPts = true;
            if (pSettings->pidTypeSettings.video.pid != 0x1fff)
            {
                BDBG_WRN(("Generating index using pid 0x%x (%u), %u", pSettings->pidTypeSettings.video.pid, pSettings->pidTypeSettings.video.pid, scdNo));
                rc = BXPT_Rave_SetScdUsingPid(r->scdIdx, scdNo, pSettings->pidTypeSettings.video.pid, &ScdConfig);
                if (rc) {return BERR_TRACE(rc);}
            }
            else
            {
                /* TODO: verify not allPass */
                BDBG_MSG(("Generating index using pid channel %d",ScdConfig.PidChannel));
                rc = BXPT_Rave_SetScdEntry(r->scdIdx, scdNo, &ScdConfig);
                if (rc) {return BERR_TRACE(rc);}
            }
        }
        else if (pSettings->pidType == NEXUS_PidType_eOther && pSettings->pidTypeSettings.other.index) {

            if (!r->rave_rec_mem.index.buffer) {
                BDBG_ERR(("No index buffer allocated"));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }

            if(scdNo==0) {
                unsigned i;
                /* Set Index Configuration */
                (void)BXPT_Rave_GetIndexerConfig( r->scdIdx, &indx_cfg);
                BDBG_CASSERT(NEXUS_NUM_STARTCODE_RANGES <= BXPT_MAX_STARTCODE_RANGES);
                for (i=0;i<NEXUS_NUM_STARTCODE_RANGES;i++) {
                    indx_cfg.Cfg.Scd.ScRange[i].RangeHi = pSettings->pidTypeSettings.other.startCodeRange[i].high;
                    indx_cfg.Cfg.Scd.ScRange[i].RangeLo = pSettings->pidTypeSettings.other.startCodeRange[i].low;
                    indx_cfg.Cfg.Scd.ScRange[i].RangeEnable = (pSettings->pidTypeSettings.other.startCodeRange[i].high >= pSettings->pidTypeSettings.other.startCodeRange[i].low);
                    indx_cfg.Cfg.Scd.ScRange[i].RangeIsASlice = false;
                }
                rc = BXPT_Rave_SetIndexerConfig(r->scdIdx, &indx_cfg);
                if (rc) {return BERR_TRACE(rc);}
            }

            ScdConfig.PidChannel = pid->pidChn->hwPidChannel->status.pidChannelIndex;
            ScdConfig.EsCount = 2; /* no payload needed, min 2 required */
            ScdConfig.ExtractPts = true;
            rc = BXPT_Rave_SetScdEntry(r->scdIdx, scdNo, &ScdConfig);
            if (rc) {return BERR_TRACE(rc);}
        }

        /* complete allocation of detector */
        pid->assignedScd = scdNo;
        r->scdUsedMap |= 1 << scdNo;
    }

    /* stash the new mapMode */
    r->scdMapMode = newMapMode;
    return NEXUS_SUCCESS;
}

static void NEXUS_Recpump_P_Release(NEXUS_RecpumpHandle r)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Recpump, r);
    NEXUS_CLIENT_RESOURCES_RELEASE(recpump,Count,NEXUS_ANY_ID);
}

static void NEXUS_Recpump_P_Finalizer(NEXUS_RecpumpHandle r)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Recpump, r);
    if (r->started) {
        NEXUS_Recpump_Stop(r);
    }
    NEXUS_Recpump_RemoveAllPidChannels(r);

    if (r->openSettings.dummyRecpump) {
        NEXUS_OBJECT_RELEASE(r, NEXUS_Recpump, r->openSettings.dummyRecpump);
    }

    if (r->rave_rec_mem.data.buffer) {
        BMMA_Unlock(r->rave_rec_mem.data.block, r->rave_rec_mem.data.buffer);
        BMMA_UnlockOffset(r->rave_rec_mem.data.block, r->rave_rec_mem.data.offset);
    }
    if (r->rave_rec_mem.index.buffer) {
        BMMA_Unlock(r->rave_rec_mem.index.block, r->rave_rec_mem.index.buffer);
        BMMA_UnlockOffset(r->rave_rec_mem.index.block, r->rave_rec_mem.index.offset);
    }
    if (r->extra_rave_rec_mem.data.buffer) {
        BMMA_Unlock(r->extra_rave_rec_mem.data.block, r->extra_rave_rec_mem.data.buffer);
        BMMA_UnlockOffset(r->extra_rave_rec_mem.data.block, r->extra_rave_rec_mem.data.offset);
    }
    if (r->extra_rave_rec_mem.index.buffer) {
        BMMA_Unlock(r->extra_rave_rec_mem.index.block, r->extra_rave_rec_mem.index.buffer);
        BMMA_UnlockOffset(r->extra_rave_rec_mem.index.block, r->extra_rave_rec_mem.index.offset);
    }

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
#if (!NEXUS_HAS_XPT_DMA)
    if(r->crypto.callback) {
        NEXUS_UnregisterEvent(r->crypto.callback);
    }
    if(r->crypto.event) {
        BKNI_DestroyEvent(r->crypto.event);
    }
#endif
    if(r->settings.securityDma) {
        NEXUS_OBJECT_RELEASE(r, NEXUS_Dma, r->settings.securityDma);
    }
#endif
#if BXPT_NUM_TSIO
    if (r->tsioDmaEndIrq) {
        BINT_DestroyCallback(r->tsioDmaEndIrq);
    }
    if (r->tsioDmaEndCallback) {
        NEXUS_IsrCallback_Destroy(r->tsioDmaEndCallback);
    }
#endif
    if (r->index.overflow_irq) {
        BINT_DestroyCallback(r->index.overflow_irq);
    }
    if (r->data.overflow_irq) {
        BINT_DestroyCallback(r->data.overflow_irq);
    }
    if (r->index.irq) {
        BINT_DestroyCallback(r->index.irq);
    }
    if(r->index.eventCallback) {
        NEXUS_UnregisterEvent(r->index.eventCallback);
    }
    if (r->index.event) {
        BKNI_DestroyEvent(r->index.event);
    }
    if (r->data.irq) {
        BINT_DestroyCallback(r->data.irq);
    }
    if(r->data.eventCallback) {
        NEXUS_UnregisterEvent(r->data.eventCallback);
    }
    if (r->data.event) {
        BKNI_DestroyEvent(r->data.event);
    }
    if (r->tpitIdx) {
        BXPT_Rave_FreeIndexer(r->tpitIdx);
    }
    NEXUS_Recpump_P_FreeScdIndexer(r);
    NEXUS_RaveErrorCounter_Uninit_priv(&r->raveErrors);
    if (r->extra_rave_rec) {
        BXPT_Rave_FreeContext(r->extra_rave_rec);
    }
    if (r->rave_rec) {
        BXPT_Rave_FreeContext(r->rave_rec);
    }

    /* free blocks after BXPT_Rave_FreeContext */
    if (r->rave_rec_mem.data.mmaHeap && r->rave_rec_mem.data.block) {
        BMMA_Free(r->rave_rec_mem.data.block);
    }
    if (r->rave_rec_mem.index.mmaHeap && r->rave_rec_mem.index.block) {
        BMMA_Free(r->rave_rec_mem.index.block);
    }
    if (r->extra_rave_rec_mem.data.mmaHeap && r->extra_rave_rec_mem.data.block) {
        BMMA_Free(r->extra_rave_rec_mem.data.block);
    }
    if (r->extra_rave_rec_mem.index.mmaHeap && r->extra_rave_rec_mem.index.block) {
        BMMA_Free(r->extra_rave_rec_mem.index.block);
    }

    if (r->data.overflowCallback) {
        NEXUS_IsrCallback_Destroy(r->data.overflowCallback);
    }
    if (r->data.dataReadyCallback) {
        NEXUS_TaskCallback_Destroy(r->data.dataReadyCallback);
    }
    if (r->index.overflowCallback) {
        NEXUS_IsrCallback_Destroy(r->index.overflowCallback);
    }
    if (r->index.dataReadyCallback) {
        NEXUS_TaskCallback_Destroy(r->index.dataReadyCallback);
    }
    r->rave_rec = NULL;
    pTransport->recpump[r->tindex] = NULL;
    NEXUS_OBJECT_DESTROY(NEXUS_Recpump, r);
    BKNI_Free(r);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Recpump, NEXUS_Recpump_Close);

void NEXUS_Recpump_GetSettings(NEXUS_RecpumpHandle r, NEXUS_RecpumpSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    *pSettings = r->settings;
}

static NEXUS_Error NEXUS_Recpump_P_CheckStartTpitIndexAndFlow(NEXUS_RecpumpHandle r)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (r->actuallyStarted && !r->indexing && r->tpitIdx) {
        /* start Index data flow */
        rc = NEXUS_Recpump_P_ApplyTpitStartSettings(r);
        if (rc) {rc=BERR_TRACE(rc);goto err_applystart; }

        rc = NEXUS_Recpump_P_StartFlow(&r->index);
        if (rc) {rc=BERR_TRACE(rc);goto err_startflow; }

        /* keep track of indexing with a separate variable. the indexPidChannel might be removed before we stop the flow. */
        r->indexing = true;
    }

err_applystart:
err_startflow:
    return rc;
}

NEXUS_Error NEXUS_Recpump_SetSettings(NEXUS_RecpumpHandle r, const NEXUS_RecpumpSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);

    if(pSettings->securityContext && !pSettings->securityDma) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (pSettings->securityDma) {
#if NEXUS_HAS_XPT_DMA && !NEXUS_INTEGRATED_M2M_SUPPORT
        BDBG_ERR(("Recpump M2M crypto not supported on this platform. Use NEXUS_KeySlot_AddPidChannel instead."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
#else
        BDBG_WRN(("Recpump M2M crypto has been deprecated. Use NEXUS_KeySlot_AddPidChannel instead."));
#if !NEXUS_HAS_XPT_DMA
        if (!pTransport->moduleSettings.dma) {
            BDBG_ERR(("Transport module does not have dma module handle."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#endif
#endif
    }
    if(r->started &&
        (r->settings.securityDma != pSettings->securityDma ||
        r->settings.securityContext != pSettings->securityContext)) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (r->settings.dropBtpPackets != pSettings->dropBtpPackets) {
        BXPT_Rave_ContextSettings cxSettings;
        BXPT_Rave_GetContextConfig(r->rave_rec, &cxSettings);
        cxSettings.EnableBppSearch = pSettings->dropBtpPackets;
        BXPT_Rave_SetContextConfig(r->rave_rec, &cxSettings);
    }

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
    if (pSettings->securityDma != r->settings.securityDma) {
        if(r->settings.securityDma) {
            NEXUS_OBJECT_RELEASE(r, NEXUS_Dma, r->settings.securityDma);
        }
        if(pSettings->securityDma) {
            NEXUS_OBJECT_ACQUIRE(r, NEXUS_Dma, pSettings->securityDma);
        }
    }
#endif

    r->settings = *pSettings;
    NEXUS_Recpump_P_ProvisionTpitIndexer(r);

    NEXUS_IsrCallback_Set(r->data.overflowCallback, &r->settings.data.overflow);
    NEXUS_TaskCallback_Set(r->data.dataReadyCallback, &r->settings.data.dataReady);
    NEXUS_IsrCallback_Set(r->index.overflowCallback, &r->settings.index.overflow);
    NEXUS_TaskCallback_Set(r->index.dataReadyCallback, &r->settings.index.dataReady);
#if BXPT_NUM_TSIO
    NEXUS_IsrCallback_Set( r->tsioDmaEndCallback, &r->settings.tsioDmaEnd );
#endif

    rc = NEXUS_Recpump_P_CheckStartTpitIndexAndFlow(r);

    return rc;
}

static NEXUS_Error NEXUS_Recpump_P_StartPid(NEXUS_RecpumpHandle r, NEXUS_Recpump_P_PidChannel *pid)
{
    NEXUS_Error rc = 0;
    BSTD_UNUSED(r);
    NEXUS_PidChannel_ConsumerStarted(pid->pidChn);
    return rc;
}

static NEXUS_Error NEXUS_Recpump_P_UpdateScdIndexing(NEXUS_RecpumpHandle r, NEXUS_Recpump_P_PidChannel *pid)
{
    NEXUS_Error rc = NEXUS_Recpump_P_ProvisionScdIndexer(r);
    if (rc == NEXUS_SUCCESS) {
        if (!r->indexing && r->scdUsedMap) {
            /* start Index data flow */
            rc = NEXUS_Recpump_P_StartFlow(&r->index);
            if (rc == NEXUS_SUCCESS) {
                r->indexing = true;
            }
        }
    }
    if (rc == NEXUS_SUCCESS) {
        (void)NEXUS_Recpump_P_StartPid(r, pid);
    }
    return rc;
}

static void NEXUS_Recpump_P_GetThresholds(bool bandHold, unsigned dataReadyThreshold, unsigned bufferSize, uint16_t *upperThreshold, uint16_t *lowerThreshold, uint32_t *minDepthThreshold)
{
    /* The XPT threshold values are in units of NEXUS_RAVE_THRESHOLD_UNITS bytes. */
    if(bandHold) {
#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
        unsigned threshold = (bufferSize *9)/10;
#else
        unsigned threshold = (bufferSize *3)/4;
#endif
        dataReadyThreshold = dataReadyThreshold + (NEXUS_RAVE_THRESHOLD_UNITS - 1);
        dataReadyThreshold -= dataReadyThreshold%NEXUS_RAVE_THRESHOLD_UNITS;
        if(threshold<dataReadyThreshold) {
            threshold = dataReadyThreshold;
        }
        *upperThreshold = threshold / NEXUS_RAVE_THRESHOLD_UNITS;
        *lowerThreshold = threshold / NEXUS_RAVE_THRESHOLD_UNITS;
        if(*lowerThreshold>*upperThreshold) {
            *lowerThreshold = *upperThreshold - 1;
        }

    } else {
        *lowerThreshold = 0;
#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
        *upperThreshold = 0;
#else
        *upperThreshold = dataReadyThreshold / NEXUS_RAVE_THRESHOLD_UNITS;
#endif
    }
    *minDepthThreshold = dataReadyThreshold; /* dataReadyCallback if BXPT_HAS_RAVE_MIN_DEPTH_INTR, else ignored */
}

static NEXUS_PlaypumpHandle nexus_p_get_playpump(const NEXUS_P_HwPidChannel *hwPidChannel)
{
    if (hwPidChannel->status.playback) {
        return pTransport->playpump[hwPidChannel->status.playbackIndex].playpump;
    }
    else {
        return NULL;
    }
}

static unsigned calc_percentage(unsigned n, unsigned d)
{
    return d ? n * 100 / d : 0;
}

static NEXUS_Error NEXUS_Recpump_P_Start(NEXUS_RecpumpHandle r)
{
    BERR_Code rc;
    BXPT_Rave_RecordSettings rec_cfg;
    const NEXUS_RecpumpSettings *pSettings = &r->settings;
    unsigned packetSize, atom;
    NEXUS_Recpump_P_PidChannel *pid;
    unsigned scdNo;
    bool bandHold;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BDBG_ASSERT(r->started && !r->actuallyStarted);

    if (!pSettings->data.dataReady.callback) {
        BDBG_ERR(("dataReady.callback is required"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    r->rave_state = Ready;

    rc = BXPT_Rave_ResetContext(r->rave_rec);
    if (rc) {return BERR_TRACE(rc);}

    /* disable context before we set various RAVE config */
    rc = BXPT_Rave_DisableContext(r->rave_rec);
    if (rc) {return BERR_TRACE(rc);}

#ifdef BXPT_VCT_SUPPORT
    if (r->openSettings.nullifyVct) {
        rc = BXPT_Rave_NullifyVCT(r->rave_rec, true, r->openSettings.nullifyVct == 1 ? BXPT_RaveVct_Tvct : BXPT_RaveVct_Cvct);
        if (rc) {return BERR_TRACE(rc);}
    }
#endif

    rc = BXPT_Rave_GetRecordConfig(r->rave_rec, &rec_cfg);
    if (rc) {return BERR_TRACE(rc);}

    switch (r->inputTransportType) {
    case NEXUS_TransportType_eTs:
    case NEXUS_TransportType_eDssEs:
    case NEXUS_TransportType_eDssPes:
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    case NEXUS_TransportType_eBulk:
#endif
        /* all outputs possible */
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        if (r->settings.outputTransportType != NEXUS_TransportType_eMpeg2Pes &&
            r->settings.outputTransportType != NEXUS_TransportType_eEs)
        {
            BDBG_ERR(("cannot convert from transportType %d to %d", r->inputTransportType, r->settings.outputTransportType));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
    case NEXUS_TransportType_eEs:
        if (r->settings.outputTransportType != NEXUS_TransportType_eEs &&
            r->settings.outputTransportType != NEXUS_TransportType_eTs )
        {
            BDBG_ERR(("cannot convert from transportType %d to %d", r->inputTransportType, r->settings.outputTransportType));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
    default:
        BDBG_ERR(("transport input format %d not supported", r->inputTransportType));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    rec_cfg.MpegMode = !NEXUS_IS_DSS_MODE(r->inputTransportType);
    rec_cfg.CountRecordedPackets = pSettings->tpit.countRecordedPackets;

    /* if BXPT_HAS_RAVE_MIN_DEPTH_INTR is defined, we have separate interrupts for dataReady and bandhold.
    if not, then the following applies... */
    /* if you turn on band hold based on playback, then the CDB_UPPER_THRESHOLD interrupt will hold playback.
    however, recpump already uses CDB_UPPER_THRESHOLD for the dataready interrupt. you cannot have the band hold and
    dataready on the same condition, otherwise you can't establish an I/O pipeline (i.e. new data coming in while you are
    writing out previous data) . what's needed is a second upper threshold interrupt: one for data ready and another
    for band hold. that said, if you want to set band hold, enable this code, set the dataReadyThreshold to be a much higher
    percentage (like 80%), then have your app poll recpump so that you can have an I/O pipeline. */
    switch(pSettings->bandHold) {
    case NEXUS_RecpumpFlowControl_eEnable:
        bandHold = true;
        break;
    case NEXUS_RecpumpFlowControl_eDisable:
        bandHold = false;
        break;
    case NEXUS_RecpumpFlowControl_eAuto:
        bandHold = false;
#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
        for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
            BDBG_ASSERT(pid->pidChn);
            if (pid->pidChn->hwPidChannel->status.playback) {
                bandHold = true;
                break;
            }
        }
#endif
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rec_cfg.BandHoldEn = bandHold;

#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    rec_cfg.BulkMode = false;   /* Default to false. Only _eBulk should use true */
#endif
    switch (r->settings.outputTransportType) {
    case NEXUS_TransportType_eTs:
    case NEXUS_TransportType_eDssEs:
    case NEXUS_TransportType_eDssPes:
        /* if input is DSS, a default output type of eTs is understood to be DSS. */
        rec_cfg.OutputFormat = BAVC_StreamType_eTsMpeg; /* this applies to DSS too */
        packetSize = NEXUS_IS_DSS_MODE(r->inputTransportType)?130:188;
        if (pSettings->timestampType != NEXUS_TransportTimestampType_eNone) {
            packetSize += 4;
        }
        break;
    case NEXUS_TransportType_eMpeg2Pes:
        /* the current impl does not use STREAM_ID_HI/LO for PES id filtering */
        rec_cfg.OutputFormat = BAVC_StreamType_ePes;
        packetSize = 0; /* there is no packet alignment for PES record. app can use M2M DMA to assemble byte aligned data into I/O sized (e.g. 4K) chunks. */
        if (pSettings->timestampType != NEXUS_TransportTimestampType_eNone) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
    case NEXUS_TransportType_eEs:
        rec_cfg.OutputFormat = BAVC_StreamType_eEs;
        packetSize = 0; /* there is no packet alignment for ES record */
        if (pSettings->timestampType != NEXUS_TransportTimestampType_eNone) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        break;
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    case NEXUS_TransportType_eBulk:
        rec_cfg.OutputFormat = BAVC_StreamType_eTsMpeg; /* this applies to DSS too */
        packetSize = NEXUS_IS_DSS_MODE(r->inputTransportType)?130:188;
        if (pSettings->timestampType != NEXUS_TransportTimestampType_eNone) {
            packetSize += 4;
        }
        rec_cfg.BulkMode = true;
        break;
#endif
    default:
        BDBG_ERR(("transport output format not supported"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }


    if (r->openSettings.data.atomSize % 64 != 0) {
        BDBG_ERR(("data.atomSize not supported. Recpump can be extended to support other atomSizes if required."));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    /* find "atom" which is a multiple of data.atomSize and packetSize */
    switch (packetSize) {
    case 188: atom = r->openSettings.data.atomSize * packetSize / 4; break;
    case 130: atom = r->openSettings.data.atomSize * packetSize / 2; break;
    case 192: atom = r->openSettings.data.atomSize * packetSize / 64; break;
    case 134: atom = r->openSettings.data.atomSize * packetSize / 2; break;
    default:  atom = 0; break; /* no alignment. should only occur for packetSize = 0. */
    }

    /* Remove the last transport packet and replace with one whole XC packet. */
    if (pSettings->data.useBufferSize) {
        rec_cfg.UseCdbSize = pSettings->data.useBufferSize;
    } else {
        rec_cfg.UseCdbSize = r->openSettings.data.bufferSize;
    }
    if (atom) {
        rec_cfg.UseCdbSize -= rec_cfg.UseCdbSize % atom;
    }
    if (packetSize) {
        /* Remove the last transport packet and replace it with a whole NEXUS_RAVE_MEMC_BLOCK_SIZE, then remove an atom or packetSize in order
        to get be under the actual bufferSize. This is required for correct RAVE wraparound logic.
        Note that this logic allows a single recpump buffer to be used for any packet size and any atom size. It's just that some combinations
        result in more truncation at the end of the buffer. The only way to avoid any truncation is to only use one packet size, one atom size,
        and to anticipate the NEXUS_RAVE_MEMC_BLOCK_SIZE replacement in your application (e.g. bufferSize = bufferSize - 188 + 256). */
        rec_cfg.UseCdbSize -= packetSize;
        rec_cfg.UseCdbSize += NEXUS_RAVE_MEMC_BLOCK_SIZE;
        /* be sure we stick within the memory we're given */
        while (rec_cfg.UseCdbSize > r->openSettings.data.bufferSize) {
            rec_cfg.UseCdbSize -= atom?atom:packetSize;
        }
        BDBG_ASSERT(rec_cfg.UseCdbSize <= r->openSettings.data.bufferSize);
    }

    NEXUS_Recpump_P_GetThresholds(bandHold, r->openSettings.data.dataReadyThreshold, rec_cfg.UseCdbSize, &rec_cfg.CdbUpperThreshold, &rec_cfg.CdbLowerThreshold,
        &rec_cfg.CdbMinDepthThreshold);
    NEXUS_Recpump_P_GetThresholds(bandHold, r->openSettings.index.dataReadyThreshold, r->openSettings.index.bufferSize, &rec_cfg.ItbUpperThreshold, &rec_cfg.ItbLowerThreshold,
        &rec_cfg.ItbMinDepthThreshold);

    BDBG_MSG(("Start: CDB alloc=%u use=%d (bandhold %u%%, dataready %u%%), ITB alloc=%u (bandhold %u%%, dataready %u%%)",
        (unsigned)r->openSettings.data.bufferSize,
        rec_cfg.UseCdbSize,
        calc_percentage(rec_cfg.CdbUpperThreshold*NEXUS_RAVE_THRESHOLD_UNITS,rec_cfg.UseCdbSize),
        calc_percentage(rec_cfg.CdbMinDepthThreshold,rec_cfg.UseCdbSize),
        (unsigned)r->openSettings.index.bufferSize,
        calc_percentage(rec_cfg.ItbUpperThreshold*NEXUS_RAVE_THRESHOLD_UNITS,r->openSettings.index.bufferSize),
        calc_percentage(rec_cfg.ItbMinDepthThreshold,r->openSettings.index.bufferSize)));

    switch (pSettings->timestampType) {
    case NEXUS_TransportTimestampType_eNone:
        rec_cfg.UseTimeStamps = false;
        break;
    case NEXUS_TransportTimestampType_e30_2U_Mod300:
        rec_cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Mod300;
        rec_cfg.UseTimeStamps = true;
        rec_cfg.DisableTimestampParityCheck = false;
        break;
    case NEXUS_TransportTimestampType_e30_2U_Binary:
        rec_cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
        rec_cfg.UseTimeStamps = true;
        rec_cfg.DisableTimestampParityCheck = false;
        break;
    case NEXUS_TransportTimestampType_e32_Mod300:
        rec_cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Mod300;
        rec_cfg.UseTimeStamps = true;
        rec_cfg.DisableTimestampParityCheck = true;
        break;
    case NEXUS_TransportTimestampType_e32_Binary:
        rec_cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
        rec_cfg.UseTimeStamps = true;
        rec_cfg.DisableTimestampParityCheck = true;
        break;
    case NEXUS_TransportTimestampType_e28_4P_Mod300:
        rec_cfg.TimestampMode = BXPT_TimestampMode_e28_4P_Mod300;
        rec_cfg.UseTimeStamps = true;
        rec_cfg.DisableTimestampParityCheck = true;
        break;
    default:
        BDBG_ERR(("Invalid timestamp mode"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* 1. Count number of pids that require indexing */
    for(scdNo=0,pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        NEXUS_PlaypumpHandle playpump;
        if (NEXUS_RECPUMP_PID_IS_INDEX(&pid->settings)) {
            scdNo++;
        }
        playpump = nexus_p_get_playpump(pid->pidChn->hwPidChannel);
        if (playpump) {
            NEXUS_PlaypumpOpenSettings openSettings;
            NEXUS_Playpump_P_GetOpenSettings(playpump, &openSettings);
            if (openSettings.streamMuxCompatible) {
                rec_cfg.EnableSingleChannelMuxCapableIndexer = true;
            }
        }
    }

    /* Timestamps should be passed through as-is to the recpump when the playpump and record timestamp formats are the same. */
    if( pSettings->timestampType != NEXUS_TransportTimestampType_eNone && r->playback )
    {
        /* NEXUS_Recpump_AddPidChannel() ensures that PID channels are all from live or all are from a playpump.
        So, we just need to check the first channel, if there is one. */
        NEXUS_Recpump_P_PidChannel *playPid = BLST_S_FIRST( &r->pid_list );

        if( playPid )
        {
            NEXUS_PlaypumpHandle playpump = nexus_p_get_playpump(playPid->pidChn->hwPidChannel);
            if (playpump) {
                NEXUS_PlaypumpSettings playpumpSettings;
                NEXUS_Playpump_GetSettings(playpump, &playpumpSettings );
                /* BXPT_TimestampMode_e28_4P_Mod300 maps to REC_TIMESTAMP_MODE 0, which uses internal hw structures
                to determine the timestamp type used in the stream. If that matches the type requested for the record,
                the stream timestamp is used without modification. */
                /* Also need to disable timestamp parity checking. */
                if( pSettings->timestampType == playpumpSettings.timestamp.type ) {
                    rec_cfg.TimestampMode = BXPT_TimestampMode_e28_4P_Mod300;
                    rec_cfg.DisableTimestampParityCheck = true;
                }
            }
        }
    }

#if BXPT_HAS_ATS
    /* Adjust recorded timestamps to stay locked to the stream's PCRs. In other words,
    the delta between PTSs on PCR carrying packets will match the delta between the
    PCRs. */
    if(r->settings.adjustTimestampUsingPcrs)
    {
        if( NULL == r->settings.pcrPidChannel )
        {
            BDBG_ERR(("NULL pcrPidChannel for timestamp adjustments."));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        rec_cfg.PcrPidChannelIndex = r->settings.pcrPidChannel->hwPidChannel->status.pidChannelIndex;
    }
    rec_cfg.LockTimestampsToPcrs = r->settings.adjustTimestampUsingPcrs;
#else /* #if BXPT_HAS_ATS */
    if(r->settings.adjustTimestampUsingPcrs) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif /* #if BXPT_HAS_ATS */
#if BXPT_HAS_LOCAL_ATS
    if(r->settings.localTimestamp) {
        rec_cfg.GenerateLocalAts = true;
        rec_cfg.LocalAtsFormat = 0;
    } else {
        rec_cfg.GenerateLocalAts = false;
    }
#else /* #if BXPT_HAS_LOCAL_ATS */
    if(r->settings.localTimestamp) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif /* #if BXPT_HAS_LOCAL_ATS */

    if (r->bipPidChannelNum) {
        /* these are cleared automatically on RAVE flush */
        rec_cfg.bipIndexing = true;
        rec_cfg.bipPidChannel = r->bipPidChannelNum;
    }

    rc = BXPT_Rave_SetRecordConfig(r->rave_rec, &rec_cfg);
    if (rc) {return BERR_TRACE(rc);}

    rc = NEXUS_Recpump_P_ProvisionScdIndexer(r);
    if (rc) {return BERR_TRACE(rc);}

    if (r->tpitIdx) {
        rc = NEXUS_Recpump_P_ApplyTpitStartSettings(r);
        if (rc) {return BERR_TRACE(rc);}
    }

    NEXUS_RaveErrorCounter_Reset_priv(&r->raveErrors);
    rc = BXPT_Rave_EnableContext(r->rave_rec);
    if (rc) {rc=BERR_TRACE(rc);goto err_rave;}

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
    if(!r->settings.securityContext) {
        r->cryptoActive = false;
    } else {
        NEXUS_DmaJobSettings jobSettings;
        unsigned i;

        if (packetSize) {
            r->data.dataReadyThreshold -= r->data.dataReadyThreshold%packetSize; /* convert dataReadyThreshold to multiplier of transport packet size */
        }
#if (!NEXUS_HAS_XPT_DMA)
        if(!g_NEXUS_Transport_P_State.moduleSettings.dma) { rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_dma;}
#endif

        r->crypto.head = r->crypto.tail = NULL;
        NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
        jobSettings.numBlocks = sizeof(r->crypto.blocks)/sizeof(*r->crypto.blocks);
        jobSettings.dataFormat = r->settings.securityDmaDataFormat;
        jobSettings.keySlot = r->settings.securityContext;
        r->crypto.job = NEXUS_DmaJob_Create(r->settings.securityDma, &jobSettings);
        if(!r->crypto.job) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_dma;}
        for(i=0;i<sizeof(r->crypto.blocks)/sizeof(*r->crypto.blocks);i++) { /* initialize dma blocks */
            NEXUS_DmaJob_GetDefaultBlockSettings(&r->crypto.blocks[i]);
            r->crypto.blocks[i].cached = false;
        }
        r->crypto.blocks[0].resetCrypto = true;
        r->crypto.blocks[0].scatterGatherCryptoStart = true;
        r->crypto.blocks[1].scatterGatherCryptoEnd = true;
        r->crypto.packetSize = packetSize;
        r->cryptoActive = true;
    }
#endif
#if BXPT_SW7425_4528_WORKAROUND
    {
        BXPT_RAVE_P_PesRemap remap;
        BXPT_RAVE_P_GetPesRemapping(r->rave_rec, &remap);
        remap.Enable = false;
        remap.NewIdFor0xC0 = 0xC0;
        remap.NewIdFor0xC1 = 0xC1;
        remap.NewIdFor0xC2 = 0xC2;
        remap.NewIdFor0xC3 = 0xC3;
        remap.NewIdFor0xC4 = 0xC4;
        remap.NewIdFor0xC5 = 0xC5;
        for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link))
        {
            const NEXUS_P_HwPidChannel *hwPidChannel = pid->pidChn->hwPidChannel;
            if(hwPidChannel->settingsPrivValid && hwPidChannel->settingsPriv.remapping)
            {
                switch(hwPidChannel->status.remappedPid)
                {
                    case 0xC0: remap.NewIdFor0xC0 = hwPidChannel->settingsPriv.remappedPesId;break;
                    case 0xC1: remap.NewIdFor0xC1 = hwPidChannel->settingsPriv.remappedPesId;break;
                    case 0xC2: remap.NewIdFor0xC2 = hwPidChannel->settingsPriv.remappedPesId;break;
                    case 0xC3: remap.NewIdFor0xC3 = hwPidChannel->settingsPriv.remappedPesId;break;
                    case 0xC4: remap.NewIdFor0xC4 = hwPidChannel->settingsPriv.remappedPesId;break;
                    case 0xC5: remap.NewIdFor0xC5 = hwPidChannel->settingsPriv.remappedPesId;break;
                    default: BDBG_ERR(("remapping for %#x not supported", hwPidChannel->status.remappedPid));break;
                }
                remap.Enable = true;
            }
        }
        BXPT_RAVE_P_SetPesRemapping(r->rave_rec, &remap);
    }
#endif

    for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if (pid->pidChn) {
            NEXUS_Recpump_P_StartPid(r, pid);
        }
    }

    /* start MPEG data flow */
    BDBG_MSG(("Starting data flow"));
    rc = NEXUS_Recpump_P_StartFlow(&r->data);
    if (rc) {rc=BERR_TRACE(rc);goto err_record_data; }

    if ((r->scdIdx && scdNo > 0) || r->tpitIdx) {
        /* start Index data flow */
        BDBG_MSG(("Starting Index flow"));
        rc = NEXUS_Recpump_P_StartFlow(&r->index);
        if (rc) {rc=BERR_TRACE(rc);goto err_record_scd; }

        /* keep track of indexing with a separate variable. the indexPidChannel might be removed before we stop the flow. */
        r->indexing = true;
    }
    else {
        r->indexing = false;
    }

#if BXPT_NUM_TSIO
    BKNI_EnterCriticalSection();
    rc = BINT_EnableCallback_isr(r->tsioDmaEndIrq);
    if (rc) {rc=BERR_TRACE(rc);goto err_record_scd; }
    BKNI_LeaveCriticalSection();
#endif
    r->actuallyStarted = true;
    return 0;

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
err_dma:
    NEXUS_Recpump_P_StopFlow(&r->index);
#endif
err_record_scd:
    NEXUS_Recpump_P_StopFlow(&r->data);
err_record_data:
    BXPT_Rave_DisableContext(r->rave_rec);
err_rave:
    BDBG_ASSERT(rc);
    return rc;
}

NEXUS_Error NEXUS_Recpump_Start(NEXUS_RecpumpHandle r)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    if (r->started) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    BDBG_ASSERT(!r->started);
    BDBG_ASSERT(!r->actuallyStarted);

    r->started = true;
    if (BLST_S_FIRST(&r->pid_list) != NULL) {
        rc = NEXUS_Recpump_P_Start(r);
    }
    else {
        /* defer */
        rc = 0;
    }

    /* mark started only if successful */
    if (rc) {
        r->started = false;
    } else {
        r->dataStopped = false;
    }
    return rc;
}

static void update_rave_counter(NEXUS_RecpumpHandle r, uint32_t elapsedTime)
{
    uint64_t hiBits;
    hiBits = (r->elapsedRaveTime >> 32);
    if ( (uint32_t)(r->elapsedRaveTime & (uint64_t)0xFFFFFFFF) > elapsedTime ) { /* wrap */
        hiBits++;
    }
    r->elapsedRaveTime = (hiBits<<32) | (uint64_t)elapsedTime;
}

void NEXUS_Recpump_StopData(NEXUS_RecpumpHandle r)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    if (!r->started)  {
        BDBG_ERR(("Recpump %d not started", r->tindex));
        return;
    }
    if(r->dataStopped) {
        BDBG_ERR(("Recpump %d already stopped", r->tindex));
        return;
    }
    r->dataStopped = true;

    if (r->rave_state != Terminated) {
        r->rave_state = Done;
    }

    if(r->actuallyStarted)
    {
        /* capture stats before context is disabled */
        BXPT_Rave_RecordStats stats;
        rc = BXPT_Rave_GetRecordStats(r->rave_rec, &stats);
        if (rc==BERR_SUCCESS) {
            update_rave_counter(r, stats.ElapsedTime);
        }

        rc = BXPT_Rave_DisableContext(r->rave_rec);
        if (rc) { BDBG_ERR(("Error from BXPT_Rave_DisableContext, ignored")); }

        NEXUS_Recpump_P_StopFlow(&r->data);
        if (r->indexing) {
            NEXUS_Recpump_P_StopFlow(&r->index);
        }
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
        if(r->cryptoActive) {
#if NEXUS_HAS_XPT_DMA
            if (r->crypto.tail != r->crypto.head) { /*  DMA idle */
                rc = NEXUS_DmaJob_P_Wait(r->crypto.job);
                if (rc) BERR_TRACE(rc); /* keep going */
            }
#else
            unsigned i;
            for(i=0;i<100;i++) {
                NEXUS_DmaJobStatus jobStatus;
                if(r->crypto.tail == r->crypto.head) { /*  DMA idle */
                    break;
                }
                rc = NEXUS_DmaJob_GetStatus(r->crypto.job, &jobStatus);
                if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);break;}
                if(jobStatus.currentState == NEXUS_DmaJobState_eComplete) {
                    break;
                }
                BDBG_MSG(("NEXUS_Recpump_StopData: %#lx waiting for DMA to complete %u", (unsigned long)r, i));
                BKNI_Sleep(1);
            }
#endif
            NEXUS_DmaJob_Destroy(r->crypto.job);
            r->crypto.job = NULL;
            r->cryptoActive = false;
        }
#endif
    }

    return;
}

void NEXUS_Recpump_Stop(NEXUS_RecpumpHandle r)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    r->rave_state = Terminated;
    if(!r->dataStopped) {
        NEXUS_Recpump_StopData(r);
    }

    (void)BXPT_Rave_DisableContext(r->rave_rec);
    (void)BXPT_Rave_FlushContext(r->rave_rec);
    BINT_ClearCallback(r->data.overflow_irq);
    BINT_ClearCallback(r->index.overflow_irq);
    r->actuallyStarted = false;
    r->started = false;

    NEXUS_CancelCallbacks(r);

    return;
}

void NEXUS_Recpump_GetDefaultAddPidChannelSettings(NEXUS_RecpumpPidChannelSettings *pSettings)
{
    unsigned i;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->pidType = NEXUS_PidType_eUnknown;
    pSettings->pidTypeSettings.video.pid = 0x1fff;
    for (i=0;i<NEXUS_NUM_STARTCODE_RANGES;i++) {
        if (i == 0) {
            /* all start codes */
            pSettings->pidTypeSettings.other.startCodeRange[i].low  = 0x00;
            pSettings->pidTypeSettings.other.startCodeRange[i].high = 0xFF;
        }
        else {
            /* disabled (if low > high) */
            pSettings->pidTypeSettings.other.startCodeRange[i].low  = 0x01;
            pSettings->pidTypeSettings.other.startCodeRange[i].high = 0x00;
        }
    }
}

/* returns true if record has another NEXUS_PidChannelHandle for the same HW pidChannelIndex */
static bool nexus_recpump_p_already_has_pidch(NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_Recpump_P_PidChannel *pid;
    for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if (pid->pidChn != pidChannel && pid->pidChn->hwPidChannel->status.pidChannelIndex == pidChannel->hwPidChannel->status.pidChannelIndex) {
            return true;
        }
    }
    return false;
}

NEXUS_Error NEXUS_Recpump_AddPidChannel(NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel, const NEXUS_RecpumpPidChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_RecpumpPidChannelSettings settings;
    NEXUS_TransportType transportType;
    NEXUS_Recpump_P_PidChannel *pid;
    NEXUS_P_HwPidChannel *hwPidChannel;
    struct NEXUS_Rave_P_ErrorCounter_Link* raveLink;
    bool first;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    if (!pSettings) {
        NEXUS_Recpump_GetDefaultAddPidChannelSettings(&settings);
        pSettings = &settings;
    }
    hwPidChannel = pidChannel->hwPidChannel;

    first = !BLST_S_FIRST(&r->pid_list);
    if (!first) {
        /* check if already playback or not */
        if (hwPidChannel->status.playback != r->playback) {
            BDBG_ERR(("You cannot mix playback and non-playback pid channels to the same recpump"));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    else {
        r->playback = hwPidChannel->status.playback;
    }

    pid = BKNI_Malloc(sizeof(*pid));
    if ( NULL == pid )
    {
       return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset((void*)pid, 0x0, sizeof(*pid));
    pid->pidChn = pidChannel;
    BLST_S_DICT_ADD(&r->pid_list, pid, NEXUS_Recpump_P_PidChannel, pidChn, link, err_duplicate);
    pid->settings = *(NEXUS_RecpumpPidChannelSettings *)pSettings;
    pid->assignedScd = -1;
    NEXUS_OBJECT_ACQUIRE(r, NEXUS_PidChannel, pidChannel);

    if (hwPidChannel->status.playback) {
        NEXUS_PlaypumpSettings playpumpSettings;
        NEXUS_PlaypumpHandle playpump = nexus_p_get_playpump(hwPidChannel);
        if (!playpump) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
        transportType = playpumpSettings.transportType;
        switch(transportType) {
        case NEXUS_TransportType_eMpeg2Pes:
            transportType = NEXUS_TransportType_eTs;
            break;
        default:
            break;
        }
    }
    else {
        NEXUS_ParserBandSettings parserBandSettings;
        NEXUS_ParserBand_P_GetSettings(hwPidChannel->parserBand, &parserBandSettings);
        transportType = parserBandSettings.transportType;
    }
    if (first) {
        r->inputTransportType = transportType;
    }
    else if (r->inputTransportType != transportType) {
        BDBG_ERR(("transport type for all pid channels must match"));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    if (!nexus_recpump_p_already_has_pidch(r, pidChannel)) {
        rc = BXPT_Rave_AddPidChannel(r->rave_rec, hwPidChannel->status.pidChannelIndex, pSettings->useRPipe);
        if (rc) {
            rc = BERR_TRACE(rc);
            goto error;
        }
    }

    if (r->started) {
        if (!r->actuallyStarted) {
            rc = NEXUS_Recpump_P_Start(r);
        }
        else {
            rc = NEXUS_Recpump_P_UpdateScdIndexing(r, pid);
        }
    }

    /* if the last step failed, we have to undo the bookkeeping */
    if (rc) {
        if (!nexus_recpump_p_already_has_pidch(r, pidChannel)) {
            BXPT_Rave_RemovePidChannel(r->rave_rec, hwPidChannel->status.pidChannelIndex);
        }
        goto error;
    }

    /* alloc a rave error counter container and add to this pid channel */
    raveLink = BKNI_Malloc(sizeof(*raveLink));
    if (raveLink==NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); /* keep going */
    }
    else {
        raveLink->counter = &r->raveErrors;
        BLST_S_DICT_ADD(&hwPidChannel->raveCounters, raveLink, NEXUS_Rave_P_ErrorCounter_Link, counter, pidchannel_link, err_duplicate_rave);
    }

    hwPidChannel->destinations |= NEXUS_PIDCHANNEL_P_DESTINATION_RAVE;

    return NEXUS_SUCCESS;

error:
    NEXUS_OBJECT_RELEASE(r, NEXUS_PidChannel, pidChannel);
    BLST_S_DICT_REMOVE(&r->pid_list, pid, pidChannel, NEXUS_Recpump_P_PidChannel, pidChn, link);
    pid->pidChn = NULL;
    BKNI_Free(pid);
    BDBG_ASSERT(rc);
    return rc;
err_duplicate:
    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    BKNI_Free(pid);
    return rc;
err_duplicate_rave:
    BKNI_Free(raveLink);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Recpump_GetPidChannelSettings(NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel, NEXUS_RecpumpPidChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Recpump_P_PidChannel *pid;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    BLST_S_DICT_FIND(&r->pid_list, pid, pidChannel, pidChn, link);
    if(pid==NULL) {
        BDBG_WRN(("NEXUS_Recpump_GetPidChannelSettings: %#lx can't find pid:%#lx", (unsigned long)r, (unsigned long)pidChannel));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    *pSettings = pid->settings;

    return rc;
}

NEXUS_Error NEXUS_Recpump_SetPidChannelSettings(NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel, const NEXUS_RecpumpPidChannelSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Recpump_P_PidChannel *pid;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    BLST_S_DICT_FIND(&r->pid_list, pid, pidChannel, pidChn, link);
    if(pid==NULL) {
        BDBG_WRN(("NEXUS_Recpump_SetPidChannelSettings: %#lx can't find pid:%#lx", (unsigned long)r, (unsigned long)pidChannel));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* probably should ban various transitions here */
    if (pid->settings.useRPipe != pSettings->useRPipe) {
        BDBG_WRN(("NEXUS_Recpump_SetPidChannelSettings: %#lx can't change useRPipe for pid:%#lx", (unsigned long)r, (unsigned long)pidChannel));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    pid->settings = *pSettings;

    if (r->actuallyStarted) {
        rc = NEXUS_Recpump_P_UpdateScdIndexing(r, pid);
    }

    return rc;
}

NEXUS_Error NEXUS_Recpump_RemovePidChannel(NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_Recpump_P_PidChannel *pid;
    struct NEXUS_Rave_P_ErrorCounter_Link* raveLink;
    NEXUS_P_HwPidChannel *hwPidChannel;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    hwPidChannel = pidChannel->hwPidChannel;
    BLST_S_DICT_FIND(&r->pid_list, pid, pidChannel, pidChn, link);
    if(pid==NULL) {
        BDBG_WRN(("NEXUS_Recpump_RemovePidChannel: %#lx can't find pid:%#lx", (unsigned long)r, (unsigned long)pidChannel));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* if the param was valid, the remove must succeed */

    (void)NEXUS_Recpump_SetTpitFilter(r, pidChannel, NULL);

    if (!nexus_recpump_p_already_has_pidch(r, pidChannel)) {
        (void)BXPT_Rave_RemovePidChannel(r->rave_rec, hwPidChannel->status.pidChannelIndex);
    }

    BLST_S_DICT_REMOVE(&hwPidChannel->raveCounters, raveLink, &r->raveErrors, NEXUS_Rave_P_ErrorCounter_Link, counter, pidchannel_link);
    if (raveLink) {
        BKNI_Free(raveLink);
    }
    else {
        BERR_TRACE(NEXUS_INVALID_PARAMETER); /* keep going */
    }
    hwPidChannel->destinations &= ~(NEXUS_PIDCHANNEL_P_DESTINATION_RAVE);

    /* must finish using pidChannel before releasing */
    NEXUS_OBJECT_RELEASE(r, NEXUS_PidChannel, pidChannel);
    BLST_S_DICT_REMOVE(&r->pid_list, pid, pidChannel, NEXUS_Recpump_P_PidChannel, pidChn, link);
    BDBG_ASSERT(pid);

    pid->pidChn = NULL;
    if (r->scdIdx && !BLST_S_FIRST(&r->pid_list)) { /* when all pids were removed free indexer */
        NEXUS_Recpump_P_FreeScdIndexer(r);
    }

    BKNI_Free(pid);

    return 0;
}

void NEXUS_Recpump_RemoveAllPidChannels(NEXUS_RecpumpHandle r)
{
    NEXUS_Recpump_P_PidChannel *pid;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);

    while(NULL!=(pid=BLST_S_FIRST(&r->pid_list))) {
        NEXUS_Recpump_RemovePidChannel(r, pid->pidChn);
    }
}

static NEXUS_Error
NEXUS_Recpump_P_GetBuffer(struct NEXUS_RecpumpFlow *flow, const void **buffer, size_t *bufferSize, const void **buffer2, size_t *bufferSize2, bool *pBypassThresholdTest)
{
    BERR_Code rc;
    BXPT_Rave_ContextPtrs ptrs;
    const BXPT_Rave_DataPtrs *ptr;
    unsigned size;
    bool bypassThresholdTest = false;
    NEXUS_RecpumpHandle pump;
    uint8_t *dataPtr;

    pump = flow->recpump;

    if (!pump->actuallyStarted)  {
        /* don't print error message, because this is a normal exit from thread processing */
        return NEXUS_UNKNOWN;
    }
    rc = BXPT_Rave_CheckBuffer(pump->rave_rec, &ptrs);
    if (rc) return BERR_TRACE(rc);

    if (flow==&pump->data) {
        ptr = &ptrs.Cdb;
    }
    else {
        ptr = &ptrs.Itb;
    }
    BDBG_MSG_TRACE(("RAVE %s depth %u %u, size %u", NEXUS_P_FLOW_NAME(flow),
            (unsigned long)(ptr->ByteCount), (unsigned long)ptr->WrapByteCount,
            (unsigned)flow->bufferSize));

    /* if the depth is greater than the size, then underlying FW or SW must be in a bad state.
       Log the event and flush buffer. Upper level software needs to decide whether to continue or terminate. */
    if ((unsigned long)(ptr->ByteCount + ptr->WrapByteCount) > flow->bufferSize) {
        BDBG_ERR(("Invalid RAVE %s depth %u, size %u", NEXUS_P_FLOW_NAME(flow),
            (unsigned)(ptr->ByteCount + ptr->WrapByteCount),
            (unsigned)flow->bufferSize));
        (void)BXPT_Rave_DisableContext(pump->rave_rec);
        (void)BXPT_Rave_FlushContext(pump->rave_rec);
        if ( bufferSize )  *bufferSize = 0;
        if ( bufferSize2 ) *bufferSize2 = 0;
        (void)BXPT_Rave_EnableContext(pump->rave_rec);

        *pBypassThresholdTest = false;
        return 0;
    }

    if (flow==&pump->data) {
        size = ptr->ByteCount;
        dataPtr = ptr->DataPtr;
        if(size<=0) {
            goto done;
        } else {
            bypassThresholdTest = ptr->WrapByteCount>0;
        }
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
        if(pump->cryptoActive) {
            uint8_t *tail = pump->crypto.tail;
            if(pump->crypto.head == tail) { /* DMA was completed, check if there is new data */
                unsigned nvecs;
                if(tail==NULL) { /* after start we don't know start of CDB buffer */
                    tail = pump->crypto.head=pump->crypto.tail = dataPtr;
                }
                if(ptr->WrapByteCount==0) {
                    if(((uint8_t *)dataPtr + size) >= tail) {
                        /*      |---------|++++++++|~~~~~~~~~~~~|-------------|
                         *   FifoStart  dataPtr  tail     (dataPtr+size) FifoEnd(Wrap) */
                        size  = (dataPtr + size) - tail;
                    } else { /* we just passed through FIFO wrap pointer, reset head */
                        /*      |~~~~~~~~~~~~~~~~~|---------------------------|
                         *   dataPtr/FifoStart (dataPtr+size)         tail/FifoEnd(Wrap) */
                        tail = pump->crypto.tail = dataPtr;
                    }
                    nvecs = 1;
                    if (pump->crypto.packetSize) {
                        size -= size%pump->crypto.packetSize;
                    }
                    pump->crypto.head = tail + size;
                } else {
                    unsigned partialPacket;

                    if(dataPtr > tail)  {
                        /* all data prior to wrap is encrypted */
                        if( ((uint8_t *)ptr->WrapDataPtr + ptr->WrapByteCount) >= tail) {
                            /*     |++++++++++++++++++++|~~~~~~~~~~~~|----------------|+++++++++++++++|
                             * FifoStart/WrapDataPtr  tail WrapDataPtr+WrapByteCount dataPtr   FifoEnd(Wrap)/dataPtr+size */
                            nvecs = 1;
                            size  = ((uint8_t *)ptr->WrapDataPtr + ptr->WrapByteCount) - tail; /* number of bytes prior to wrap */
                            if (pump->crypto.packetSize) {
                                size -= size%pump->crypto.packetSize; /* if there is no wrap, then don't cross packet boundary */
                            }
                            pump->crypto.head = tail + size;
                        } else {
                            return BERR_TRACE(NEXUS_UNKNOWN); /* somehow rave pointer raced ahead of DMA pointer*/
                        }
                    } else if((dataPtr + size) >= tail) {
                        /*     |~~~~~~~~~~~~~~~~~~~~~~~|-------------------|++++++++++|~~~~~~~~|
                        * FifoStart/WrapDataPtr  WrapDataPtr+WrapByteCount dataPtr  tail  FifoEnd(Wrap)/dataPtr+size */
                        size = (dataPtr + size) - tail; /* number of bytes prior to wrap */
                        if(size>0) { /* if there is unencrypted data prior to wrap */
                            if (pump->crypto.packetSize) {
                                partialPacket = (ptr->WrapByteCount + size)%pump->crypto.packetSize; /* size of partial packet */
                            }
                            else {
                                partialPacket = 0;
                            }
                            if(ptr->WrapByteCount > partialPacket) { /* if after write there is complete packet after wrap */
                                nvecs = 2;
                                pump->crypto.head = (uint8_t*)ptr->WrapDataPtr + (ptr->WrapByteCount - partialPacket);
                                pump->crypto.blocks[1].blockSize = ptr->WrapByteCount - partialPacket;
                                pump->crypto.blocks[1].pSrcAddr = ptr->WrapDataPtr;
                                pump->crypto.blocks[1].pDestAddr = ptr->WrapDataPtr;
                            } else { /* Can't use data after wrap yet */
                                nvecs = 1;
                                if (pump->crypto.packetSize) {
                                    size -= size%pump->crypto.packetSize;
                                }
                                pump->crypto.head = tail + size;
                            }
                        } else {
                            /* there was wrap and only unencrypted data after wrap */
                            /*     |~~~~~~~~~~~~~~~~~~~~~~~|-------------------|++++++++++|
                            * FifoStart/WrapDataPtr  WrapDataPtr+WrapByteCount dataPtr  tail/FifoEnd(Wrap)/dataPtr+size */
                            nvecs = 1;
                            size = ptr->WrapByteCount;
                            if (pump->crypto.packetSize) {
                                size -= size%pump->crypto.packetSize;
                            }
                            tail = pump->crypto.tail = ptr->WrapDataPtr;
                            pump->crypto.head = tail + size;
                        }
                    } else {
                        return BERR_TRACE(NEXUS_UNKNOWN); /* somehow rave pointer raced ahead of DMA pointer*/
                    }
                }
                /* BDBG_MSG(("DMA: %u %u %u (%u %u %u(%p:%p:%u)", tail != pump->crypto.head,nvecs, size>flow->dataReadyThreshold, !flow->pending, tail<(dataPtr+flow->dataReadyThreshold), (tail+size)>=(dataPtr+flow->dataReadyThreshold), (tail+size), (dataPtr+flow->dataReadyThreshold), size)); */

                if(tail != pump->crypto.head && /* there is new data available */
                   (nvecs>1 ||  /* wrap */
                    pump->dataStopped ||
                    size>flow->dataReadyThreshold || /* or DMA block large enough */
                    (!flow->pending && tail<(dataPtr+flow->dataReadyThreshold) && (tail+size)>=(dataPtr+flow->dataReadyThreshold))) /* application waits for data and completed DMA would make threshold test to pass */
                        ) {
                    pump->crypto.blocks[0].blockSize = size;
                    pump->crypto.blocks[0].pSrcAddr = tail;
                    pump->crypto.blocks[0].pDestAddr = tail;
                    pump->crypto.blocks[0].scatterGatherCryptoEnd = nvecs==2?false:true;

#if (!NEXUS_HAS_XPT_DMA)
                    NEXUS_Module_Lock(g_NEXUS_Transport_P_State.moduleSettings.dma);
                    rc = NEXUS_DmaJob_ProcessBlocks_priv(pump->crypto.job, pump->crypto.blocks, nvecs, pump->crypto.event);
                    NEXUS_Module_Unlock(g_NEXUS_Transport_P_State.moduleSettings.dma);
#else
                    rc = NEXUS_DmaJob_ProcessBlocks_priv(pump->crypto.job, pump->crypto.blocks, nvecs, NEXUS_Recpump_P_DmaCallback, pump);
#endif
                    BDBG_MSG(("NEXUS_Recpump_P_GetBuffer:%#lx DMA(%#lx:%u) %u->%u", (unsigned long)pump, (unsigned long)pump->crypto.tail, nvecs, size, rc));
                    if(rc==NEXUS_SUCCESS) {
                        tail = pump->crypto.tail = pump->crypto.head;
                    } else if (rc!=NEXUS_DMA_QUEUED) {
                        pump->crypto.head = tail; /* revert changes */
                        return BERR_TRACE(rc);
                    }
                } else {
                    pump->crypto.head = tail;
                }
            }

            if(tail >= dataPtr) {
                size = tail - dataPtr;
                bypassThresholdTest = false;
            } else {
                size = ptr->ByteCount;
                bypassThresholdTest = true;
            }
        }
#endif
    } else {
        size = ptr->ByteCount;
        dataPtr = ptr->DataPtr;
        if (size<=0) {
            goto done;
        } else {
            /* on a wrap around, we must always return whatever data is available */
            bypassThresholdTest = ptr->WrapByteCount>0;
        }
    }

done:
    if (buffer2) {
        /* if you want to peek around the wrap around, don't apply any atomSize truncation. just give
        both buffer and buffer2 exactly as is.
        if crypto is active, don't peek around. */
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
        if (!pump->cryptoActive && ptr->WrapByteCount>0) {
#else
        if (ptr->WrapByteCount>0) {
#endif
            *buffer2 = ptr->WrapDataPtr;
            *bufferSize2 = ptr->WrapByteCount;
        }
        else {
            /* the user is requesting peek around, but there is none or it is wrapped with
            dma logic and can't be provided. */
            *buffer2 = NULL;
            *bufferSize2 = 0;
        }
    }
    else {
        /* apply atomSize limitations before setting buffer/bufferSize */
        if (pump->rave_state != Done) {
            if (size >= flow->atomSize || bypassThresholdTest) {
                /* don't modify size & dataPtr */
            } else {
                /* If we're not at wrap around, and we're not done, then GetBuffer shouldn't report any data which is
                less than the threshold. Let the app wait for the interrupt. */
                size = 0;
                dataPtr = NULL;
            }
        } else {
            /* If we're done, we send everything out. The last piece of data can be < atomSize. */
            if (flow->atomSize && size > flow->atomSize) {
                size -= (size % flow->atomSize);
            }
        }
    }
    if ( pump->rave_state == Done ) {
        /* if writeAllTimeout is set ALL data in buffer will be written - even data < 4k */
        bypassThresholdTest = true;
    }
    *bufferSize = size;
    *buffer = dataPtr;
    *pBypassThresholdTest = bypassThresholdTest;

    BDBG_MSG(("GetBuffer[%s] %u (actual %u, threshold %u)", NEXUS_P_FLOW_NAME(flow), (unsigned)*bufferSize, (unsigned)size, flow->dataReadyThreshold));

    return NEXUS_SUCCESS;
}


static NEXUS_Error
NEXUS_Recpump_P_WriteComplete(struct NEXUS_RecpumpFlow *flow, size_t amount_written)
{
    BERR_Code rc;
    NEXUS_Recpump_P_BufferState  bufferState;

    if (!flow->recpump->actuallyStarted)  {
        /* don't print error message, because this is a normal exit from thread processing */
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (amount_written > flow->lastGetBuffer) {
        BDBG_WRN(("Cannot WriteComplete %u when last GetBuffer was %u", (unsigned)amount_written, flow->lastGetBuffer));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    BDBG_MSG(("WriteComplete[%s] %u", NEXUS_P_FLOW_NAME(flow), (unsigned)amount_written));
    flow->bytesRecorded += amount_written;

    if (flow == &flow->recpump->index) {
        /* Itb read pointer is updated here */
        rc = BXPT_Rave_UpdateReadOffset(flow->recpump->rave_rec, 0 /* CDB count */, amount_written);
    }
    else {
        rc = BXPT_Rave_UpdateReadOffset(flow->recpump->rave_rec, amount_written, 0 /* ITB count */);
    }
    if (rc) {return BERR_TRACE(rc);}

    flow->pending = false;
    flow->lastGetBuffer -= amount_written;

    if (flow == &flow->recpump->data || flow->recpump->indexing) {
        bufferState = NEXUS_Recpump_P_TestDataReadyCallback(flow);
        NEXUS_Recpump_P_ControlIrq(flow, bufferState);
    }
    BINT_EnableCallback(flow->overflow_irq);
#if BXPT_NUM_TSIO
    BINT_EnableCallback(flow->recpump->tsioDmaEndIrq);
#endif
    return NEXUS_SUCCESS;
}

/* this function handles calls from application, it checks rave buffer, saves last returned size and flushes cache */
static NEXUS_Error
NEXUS_Recpump_P_GetBufferApi(struct NEXUS_RecpumpFlow *flow, const void **buffer, size_t *bufferSize, const void **buffer2, size_t *bufferSize2)
{
    NEXUS_Error rc;
    bool wrapped;
    rc = NEXUS_Recpump_P_GetBuffer(flow, buffer, bufferSize, buffer2, bufferSize2, &wrapped);
    if(rc==NEXUS_SUCCESS) {
        flow->lastGetBuffer = *bufferSize;

        if(*bufferSize>0) {
            if (flow->flushableBuffer) {
                NEXUS_FlushCache(*buffer, *bufferSize);
            }
        } else {
            *buffer = NULL;
        }

        if (bufferSize2) {
            flow->lastGetBuffer += *bufferSize2;
            if(*bufferSize2>0) {
                if (flow->flushableBuffer) {
                    NEXUS_FlushCache(*buffer2, *bufferSize2);
                }
            } else {
                *buffer2 = NULL;
            }
        }
    }
    return rc;
}


NEXUS_Error NEXUS_Recpump_GetDataBuffer(NEXUS_RecpumpHandle r, const void **pBuffer, size_t *pAmount)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    return NEXUS_Recpump_P_GetBufferApi(&r->data, pBuffer, pAmount, NULL, NULL);
}

NEXUS_Error NEXUS_Recpump_GetDataBufferWithWrap( NEXUS_RecpumpHandle r, const void **pBuffer, size_t *pAmount, const void **pBuffer2, size_t *pAmount2 )
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    return NEXUS_Recpump_P_GetBufferApi(&r->data, pBuffer, pAmount, pBuffer2, pAmount2);
}

NEXUS_Error NEXUS_Recpump_DataReadComplete(NEXUS_RecpumpHandle r, size_t amount)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    return NEXUS_Recpump_P_WriteComplete(&r->data, amount);
}

NEXUS_Error NEXUS_Recpump_GetIndexBuffer(NEXUS_RecpumpHandle r, const void **pBuffer, size_t *pAmount)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    if (r->indexing) {
        return NEXUS_Recpump_P_GetBufferApi(&r->index, pBuffer, pAmount, NULL, NULL);
    }
    else {
        *pBuffer = NULL;
        *pAmount = 0;
        return 0;
    }
}

NEXUS_Error NEXUS_Recpump_GetIndexBufferWithWrap( NEXUS_RecpumpHandle r, const void **pBuffer, size_t *pAmount, const void **pBuffer2, size_t *pAmount2)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    if (r->indexing) {
        return NEXUS_Recpump_P_GetBufferApi(&r->index, pBuffer, pAmount, pBuffer2, pAmount2);
    }
    else {
        *pBuffer = NULL;
        *pAmount = 0;
        *pBuffer2 = NULL;
        *pAmount2 = 0;
        return 0;
    }
}

NEXUS_Error NEXUS_Recpump_IndexReadComplete(NEXUS_RecpumpHandle r, size_t amount)
{
    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    return NEXUS_Recpump_P_WriteComplete(&r->index, amount);
}

NEXUS_Error NEXUS_Recpump_GetStatus(NEXUS_RecpumpHandle r, NEXUS_RecpumpStatus *pStatus)
{
    BERR_Code rc;
    BXPT_Rave_ContextPtrs ptrs;
    BXPT_Rave_DataPtrs *ptr;
    BXPT_Rave_RecordStats stats;
    NEXUS_Recpump_P_PidChannel *pid;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    pStatus->started = r->started;
    pStatus->openSettings = r->openSettings;
    pStatus->rave.index = r->rave_rec_index;

    rc = BXPT_Rave_CheckBuffer(r->rave_rec, &ptrs);
    if (rc) return BERR_TRACE(rc);
    rc = BXPT_Rave_GetRecordStats(r->rave_rec, &stats);
    if (rc) return BERR_TRACE(rc);

    ptr = &ptrs.Cdb;
    pStatus->data.bytesRecorded = r->data.bytesRecorded;
    pStatus->data.bufferBase = r->data.bufferBase;
    pStatus->data.fifoDepth = (unsigned long)(ptr->ByteCount + ptr->WrapByteCount);
    pStatus->data.fifoSize =  r->data.bufferSize;
    if (pStatus->data.fifoDepth > pStatus->data.fifoSize) {
        BDBG_ERR(("Invalid RAVE CDB depth %u, size %u", (unsigned)pStatus->data.fifoDepth, (unsigned)pStatus->data.fifoSize));
        pStatus->data.fifoDepth = 0;
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    ptr = &ptrs.Itb;
    pStatus->index.bytesRecorded = r->index.bytesRecorded;
    pStatus->index.bufferBase = r->index.bufferBase;
    pStatus->index.fifoDepth = (unsigned long)(ptr->ByteCount + ptr->WrapByteCount);
    pStatus->index.fifoSize = r->index.bufferSize;
    if (pStatus->index.fifoDepth > pStatus->index.fifoSize) {
        BDBG_ERR(("Invalid RAVE ITB depth %u, size %u", (unsigned)pStatus->index.fifoDepth, (unsigned)pStatus->index.fifoSize));
        pStatus->index.fifoDepth = 0;
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    for(pid=BLST_S_FIRST(&r->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if (NEXUS_RECPUMP_PID_IS_INDEX(&pid->settings)) {
            pStatus->hasIndex = true;
            break;
        }
    }

    if ((stats.ElapsedTime || (!r->started && r->elapsedRaveTime))
        && r->data.bytesRecorded)
    {
        uint64_t bitCount = r->data.bytesRecorded * 8;
        uint64_t elapsedMicroSeconds;

        if (r->started) {
            update_rave_counter(r, stats.ElapsedTime);
        }

        elapsedMicroSeconds = (r->elapsedRaveTime * 1000000) / 793652;
        pStatus->bitrate = (unsigned) ((bitCount * 1000000) / elapsedMicroSeconds);
    }

    return 0;
}

static NEXUS_Recpump_P_BufferState
NEXUS_Recpump_P_TestDataReadyCallback(struct NEXUS_RecpumpFlow *flow)
{
    const void *buffer, *buffer2;
    bool bypassThresholdTest = false;
    size_t size = 0, size2 = 0;

    if (flow->recpump->rave_state == Terminated) {
        return NEXUS_Recpump_P_BufferState_eTerminated;
    }

    if (NEXUS_Recpump_P_GetBuffer(flow, &buffer, &size, &buffer2, &size2, &bypassThresholdTest)!=NEXUS_SUCCESS) {
        return NEXUS_Recpump_P_BufferState_eError;
    }

    /* If we haven't exceeded the threshold, we should not fire the dataReady callback. Excessive callbacks lead to poor performance.
    However, we must always fire the callback at the wraparound so the application doesn't get stuck if relying on this callback.
    Must apply NEXUS_RAVE_THRESHOLD_UNITS adjustment before comparing with flow->dataReadyThreshold because XPT is one-based, not zero-based. */
    if ((size == 0 || size+NEXUS_RAVE_THRESHOLD_UNITS < flow->dataReadyThreshold) && !bypassThresholdTest) {
        return NEXUS_Recpump_P_BufferState_eNoData;
    }

    if (flow->pending) {
        return NEXUS_Recpump_P_BufferState_eWaitingApplication;
    }

    flow->pending = true;
    BDBG_MSG(("dataReady[%s] size=%u", NEXUS_P_FLOW_NAME(flow), (unsigned)size));

    NEXUS_TaskCallback_Fire(flow->dataReadyCallback);
    return NEXUS_Recpump_P_BufferState_eApplicationNotified;
}


static NEXUS_Error
NEXUS_Recpump_P_StartFlow(struct NEXUS_RecpumpFlow *flow)
{
    BERR_Code rc;
    BDBG_ASSERT(!flow->irqEnabled);
    BKNI_EnterCriticalSection();
    rc = BINT_EnableCallback_isr(flow->irq);
    flow->irqEnabled = rc==BERR_SUCCESS;
    (void)BINT_EnableCallback_isr(flow->overflow_irq);
    BKNI_LeaveCriticalSection();
    if (rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
    flow->pending = false;
    flow->bytesRecorded = 0;
    flow->recpump->elapsedRaveTime = 0;
    return NEXUS_SUCCESS;
error:
    return rc;
}

static void
NEXUS_Recpump_P_StopFlow(struct NEXUS_RecpumpFlow *flow)
{
    BKNI_EnterCriticalSection();
    if(flow->irqEnabled) {
        BINT_DisableCallback_isr(flow->irq);
        flow->irqEnabled = false;
    }
    BINT_DisableCallback_isr(flow->overflow_irq);
    BKNI_LeaveCriticalSection();
    /* check that there is enough data in the buffer and issue callback */
    NEXUS_Recpump_P_TestDataReadyCallback(flow);
}

void NEXUS_Recpump_GetDefaultTpitFilter( NEXUS_RecpumpTpitFilter *pFilter )
{
    BKNI_Memset(pFilter, 0, sizeof(*pFilter));
    pFilter->mpegMode = true;
}

static bool allpass_pidchannel(struct NEXUS_P_HwPidChannel *hwPidChannel)
{
    if (hwPidChannel->playpump) {
        NEXUS_PlaypumpSettings settings;
        NEXUS_Playpump_GetSettings(hwPidChannel->playpump, &settings);
        return settings.allPass;
    }
    else {
        return hwPidChannel->parserBand->settings.allPass;
    }
}

NEXUS_Error NEXUS_Recpump_SetTpitFilter( NEXUS_RecpumpHandle r, NEXUS_PidChannelHandle pidChannel, const NEXUS_RecpumpTpitFilter *pFilter )
{
    BERR_Code rc;
    NEXUS_Recpump_P_PidChannel *pid;
    NEXUS_Recpump_P_PidChannel *pid1;

    BDBG_OBJECT_ASSERT(r, NEXUS_Recpump);
    BDBG_OBJECT_ASSERT(pidChannel, NEXUS_PidChannel);
    BDBG_CASSERT(sizeof(BXPT_Rave_TpitEntry) == sizeof(NEXUS_RecpumpTpitFilter));

    BLST_S_DICT_FIND(&r->pid_list, pid, pidChannel, pidChn, link);
    if(pid==NULL) {
        BDBG_WRN(("NEXUS_Recpump_SetTpitFilter: %#lx can't find pid:%#lx", (unsigned long)r, (unsigned long)pidChannel));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_pid;
    }


    if (pFilter) {
        /* validation */
        if (pFilter->mpegMode != !NEXUS_IS_DSS_MODE(r->inputTransportType)) {
            BDBG_ERR(("NEXUS_RecpumpTpitFilter.mpegMode does not match parser band's transportType"));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_validate;
        }

        /* recpump init */
        if (!pid->tpit.enabled) {
            unsigned availableIndex;

            /* enable a new filter. this requires finding an available tpit filter index. */
            pid->tpit.enabled = true;

            rc = NEXUS_Recpump_P_ProvisionTpitIndexer(r);
            if (rc) {rc=BERR_TRACE(rc); goto err_alloc;}

            for (availableIndex=0;availableIndex<BXPT_NUM_TPIT_PIDS;availableIndex++) {
                bool taken = false;

                for(pid1=NULL, pid1=BLST_S_FIRST(&r->pid_list);pid1;pid1=BLST_S_NEXT(pid1, link)) {
                     if (pid1->pidChn != pidChannel && pid1->tpit.enabled) {
                        if (pid1->tpit.index == availableIndex) {
                            BDBG_ASSERT(availableIndex <= BXPT_NUM_TPIT_PIDS);
                            taken = true;
                            break;
                        }
                    }
                }
                if (!taken) break;
            }
            if (availableIndex == BXPT_NUM_TPIT_PIDS) {
                rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
                goto err_findindex;
            }

            pid->tpit.index = availableIndex;
        }

        pid->tpit.filter = *pFilter;
        if (!allpass_pidchannel(pidChannel->hwPidChannel)) {
            /* force the pid, but only if not allpass. also, use the remapped pid, but not for packetization. */
            if (pidChannel->hwPidChannel->settings.remap.enabled) {
                pid->tpit.filter.pid = pidChannel->hwPidChannel->status.remappedPid;
            }
            else {
                pid->tpit.filter.pid = pidChannel->hwPidChannel->status.pid;
            }
        }

        /*BDBG_MSG(("setting tpit filter slot %d: tpit index %d", i, pid->tpit.index));*/
        rc = BXPT_Rave_SetTpitFilter(r->tpitIdx, pid->tpit.index, (const BXPT_Rave_TpitEntry *)&pid->tpit.filter);
        if (rc) return BERR_TRACE(rc);

        /* start Index data flow */
        BDBG_MSG(("Starting Index flow late"));
        rc = NEXUS_Recpump_P_CheckStartTpitIndexAndFlow(r);

        if (rc != NEXUS_SUCCESS) {
            goto err_startflow;
        }
    }
    else {
        /* disable */
        if (pid->tpit.enabled) {
            BKNI_Memset(&pid->tpit.filter, 0, sizeof(pid->tpit.filter));

            /*BDBG_MSG(("unsetting tpit filter slot %d: tpit index %d", i, pid->tpit.index));*/
            (void)BXPT_Rave_SetTpitFilter(r->tpitIdx, pid->tpit.index, (const BXPT_Rave_TpitEntry *)&pid->tpit.filter);
            /* don't fail on the disable. we can't mess up the bookkeeping. */

            pid->tpit.enabled = false;
            pid->tpit.index = 0xFFFFFFFF; /* don't care */
        }

        NEXUS_Recpump_P_ProvisionTpitIndexer(r);
    }

    return 0;
err_startflow:
err_findindex:
err_alloc:
    pid->tpit.enabled = false;
    NEXUS_Recpump_P_ProvisionTpitIndexer(r);
err_validate:
err_pid:
    return rc;
}

static NEXUS_Error NEXUS_Recpump_P_ApplyTpitStartSettings(NEXUS_RecpumpHandle r)
{
    unsigned i;
    const NEXUS_RecpumpSettings *pSettings = &r->settings;
    BXPT_Rave_IndexerSettings indx_cfg;
    NEXUS_Error rc;

    (void)BXPT_Rave_GetIndexerConfig(r->tpitIdx, &indx_cfg);
    indx_cfg.Cfg.Tpit.FirstPacketEnable = pSettings->tpit.firstPacketEnable;
    indx_cfg.Cfg.Tpit.StorePcrMsb = pSettings->tpit.storePcrMsb;
    indx_cfg.Cfg.Tpit.IdleEventEnable = pSettings->tpit.idleEventEnable;
    indx_cfg.Cfg.Tpit.RecordEventEnable = pSettings->tpit.recordEventEnable;
    indx_cfg.Cfg.Tpit.CorruptionByte = pSettings->tpit.corruption.data;
    indx_cfg.Cfg.Tpit.CorruptionStart = pSettings->tpit.corruption.start;
    indx_cfg.Cfg.Tpit.CorruptionEnd = pSettings->tpit.corruption.end;
    rc = BXPT_Rave_SetIndexerConfig(r->tpitIdx, &indx_cfg);
    if (rc) {return BERR_TRACE(rc);}

    for (i=0;i<NEXUS_NUM_ECM_TIDS;i++) {
        if (pSettings->tpit.ecmPair[i].enabled) {
            rc = BXPT_Rave_SetTpitEcms(r->tpitIdx, i+1 /* PI is one-based */, pSettings->tpit.ecmPair[i].evenTid, pSettings->tpit.ecmPair[i].oddTid);
            if (rc) {return BERR_TRACE(rc);}
        }
        /* TODO: no disable */
    }
    return NEXUS_SUCCESS;
}
