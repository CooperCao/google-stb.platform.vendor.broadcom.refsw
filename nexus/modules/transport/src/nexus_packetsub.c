/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
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
 * Porting interface code for the packet substitution section of the
 * data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "nexus_transport_module.h"

BDBG_MODULE(nexus_packetsub);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#if BXPT_HAS_PACKETSUB

#include "bxpt_packetsub.h"
#include "bchp_int_id_xpt_bus_if.h"

struct NEXUS_PacketSub
{
    NEXUS_OBJECT(NEXUS_PacketSub);
    unsigned index;
    NEXUS_PacketSubOpenSettings openSettings;
    NEXUS_PacketSubSettings settings;
    BXPT_PacketSub_Handle xptPacketSub;
    bool started;
    BMMA_Heap_Handle mmaHeap;
    BMMA_Block_Handle mmaBlock;
    uint8_t *buffer; /* cached ptr */
    unsigned offsetBuffer; /* offset for buffer */
    unsigned wptr, rptr, size; /* fifo of buffer space */

    struct {
        BMMA_Heap_Handle mmaHeap;
        BMMA_Block_Handle mmaBlock;
        BXPT_PacketSub_Descriptor *ptr; /* pointer to array of descriptors */
        unsigned wptr, rptr, size; /* fifo of descriptors */
    } desc;

    unsigned lastGetBufferSize;
    NEXUS_IsrCallbackHandle finished, dataCallback;
    BINT_CallbackHandle psubInt;
};

static void
NEXUS_PacketSub_isr(void *context, int param)
{
    NEXUS_PacketSubHandle packetSub = context;
    BXPT_PacketSub_Descriptor *pDesc, *pCachedDesc;
    BXPT_PacketSub_ChannelStatus status;

    BSTD_UNUSED(param);

    (void)BXPT_PacketSub_GetChannelStatus_isrsafe(packetSub->xptPacketSub, &status);
    (void)BXPT_PacketSub_GetCurrentDescriptorAddress_isrsafe(packetSub->xptPacketSub, &pDesc);
    BDBG_MSG_TRACE(("%p: isr cur desc %p, busy=%d finished=%d", packetSub, pDesc, status.Busy, status.Finished));

#if 0
    /* fifo should not be empty when we get an interrupt */
    BDBG_ASSERT(packetSub->desc.rptr != packetSub->desc.wptr);
#else
    /* TODO: why does this happen? */
    if (packetSub->desc.rptr == packetSub->desc.wptr) {
        return;
    }
#endif

    /* if not finished, then back up one desc to find what's been completed so far */
    if (!status.Finished) {
        if (pDesc == &packetSub->desc.ptr[packetSub->desc.rptr]) {
            /* none completed */
            return;
        }
        if (pDesc == packetSub->desc.ptr) {
            pDesc = &packetSub->desc.ptr[packetSub->desc.size-1];
        }
        else {
            pDesc--;
        }
    }

    /* update buffer and desc rptrs */
    packetSub->desc.rptr = (pDesc - packetSub->desc.ptr) + 1;
    if (packetSub->desc.rptr == packetSub->desc.size) {
        packetSub->desc.rptr = 0;
    }
    
    pCachedDesc = pDesc;
    NEXUS_FlushCache_isr(pCachedDesc, sizeof(*pCachedDesc));
    BDBG_ASSERT(pCachedDesc->BufferStartAddr >= packetSub->offsetBuffer && pCachedDesc->BufferStartAddr < packetSub->offsetBuffer + packetSub->size);
    packetSub->rptr = (pCachedDesc->BufferStartAddr - packetSub->offsetBuffer) + pCachedDesc->BufferLength;
    BDBG_ASSERT(packetSub->rptr <= packetSub->size);
    if (packetSub->rptr == packetSub->size) {
        packetSub->rptr = 0;
    }
    BDBG_MSG(("isr(%p) fifo=%d,%d; desc=%d,%d", (void *)packetSub, packetSub->rptr, packetSub->wptr, packetSub->desc.rptr, packetSub->desc.wptr));

    NEXUS_IsrCallback_Fire_isr(packetSub->dataCallback);
    if (packetSub->rptr == packetSub->wptr) {
        BDBG_ASSERT(packetSub->desc.rptr == packetSub->desc.wptr);
        NEXUS_IsrCallback_Fire_isr(packetSub->finished);
    }
}

void NEXUS_PacketSub_GetDefaultOpenSettings( NEXUS_PacketSubOpenSettings *pOpenSettings )
{
    BKNI_Memset(pOpenSettings, 0, sizeof(*pOpenSettings));
    pOpenSettings->fifoSize = 32 * 1024;
    pOpenSettings->numDescriptors = 4; /* default to a small number */
}

static bool g_packetsub_open[BXPT_NUM_PACKETSUBS];
NEXUS_PacketSubHandle NEXUS_PacketSub_Open( unsigned index, const NEXUS_PacketSubOpenSettings *pOpenSettings )
{
    NEXUS_PacketSubHandle packetSub;
    BERR_Code rc;
    BXPT_PacketSub_ChannelSettings channelSettings;
    unsigned int_id;
    NEXUS_PacketSubOpenSettings defaultOpenSettings;
    NEXUS_HeapHandle heap;

    if (!pOpenSettings) {
        NEXUS_PacketSub_GetDefaultOpenSettings(&defaultOpenSettings);
        pOpenSettings = &defaultOpenSettings;
    }
    
    if (index == NEXUS_ANY_ID) {
        for (index=0;index<BXPT_NUM_PACKETSUBS;index++) {
            if (!g_packetsub_open[index]) break;
        }
    }
    if (index >= BXPT_NUM_PACKETSUBS || g_packetsub_open[index]) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
    if (pOpenSettings->fifoSize < 4) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pOpenSettings->numDescriptors == 0) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    packetSub = BKNI_Malloc(sizeof(*packetSub));
    if (!packetSub) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_PacketSub, packetSub);
    packetSub->index = index;
    g_packetsub_open[index] = true;

    packetSub->finished = NEXUS_IsrCallback_Create(packetSub, NULL);
    if (!packetSub->finished) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    packetSub->dataCallback = NEXUS_IsrCallback_Create(packetSub, NULL);
    if (!packetSub->dataCallback) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    heap = NEXUS_P_DefaultHeap(pOpenSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (!heap) heap = g_pCoreHandles->heap[pTransport->settings.mainHeapIndex].nexus;
    packetSub->mmaHeap = NEXUS_Heap_GetMmaHandle(heap);

    /* alloc fifo */
    packetSub->size = pOpenSettings->fifoSize;
    packetSub->mmaBlock = BMMA_Alloc(packetSub->mmaHeap, packetSub->size, 4, NULL);
    if (!packetSub->mmaBlock) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto error;
    }
    packetSub->buffer = BMMA_Lock(packetSub->mmaBlock);
    packetSub->offsetBuffer = BMMA_LockOffset(packetSub->mmaBlock);

    /* alloc descriptors */
    packetSub->desc.size = pOpenSettings->numDescriptors + 1; /* alloc one extra for simple fifo logic */
    packetSub->desc.mmaHeap = g_pCoreHandles->heap[pTransport->settings.mainHeapIndex].mma;
    packetSub->desc.mmaBlock = BMMA_Alloc(packetSub->desc.mmaHeap, packetSub->desc.size * sizeof(BXPT_PacketSub_Descriptor), 16, NULL);
    if (!packetSub->desc.mmaBlock) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto error;
    }
    packetSub->desc.ptr = BMMA_Lock(packetSub->desc.mmaBlock);
    BDBG_MSG_TRACE(("%p: desc fifo %#x, size %#x * %d", packetSub, packetSub->desc.ptr, sizeof(BXPT_PacketSub_Descriptor), packetSub->desc.size));

    rc = BXPT_PacketSub_GetChannelDefaultSettings(pTransport->xpt, index, &channelSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    channelSettings.descBlock = packetSub->desc.mmaBlock;

    /* set default settings to match HW/PI default */
    packetSub->settings.outputRate = channelSettings.OutputRate;

    rc = BXPT_PacketSub_OpenChannel(pTransport->xpt, &packetSub->xptPacketSub, index, &channelSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BXPT_PacketSub_GetEobIntId( packetSub->xptPacketSub, &int_id );
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    packetSub->openSettings = *pOpenSettings;

    rc = BINT_CreateCallback(&packetSub->psubInt, g_pCoreHandles->bint, int_id, NEXUS_PacketSub_isr, packetSub, 0);
    if (rc) {rc = BERR_TRACE(rc); goto error;}
    rc = BINT_EnableCallback(packetSub->psubInt);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return packetSub;

error:
    NEXUS_PacketSub_Close(packetSub);
    return NULL;
}

static void NEXUS_PacketSub_P_Finalizer( NEXUS_PacketSubHandle packetSub )
{
    NEXUS_OBJECT_ASSERT(NEXUS_PacketSub, packetSub);

    g_packetsub_open[packetSub->index] = false;
    if (packetSub->started) {
        NEXUS_PacketSub_Stop(packetSub);
    }
    if (packetSub->psubInt) {
        BINT_DestroyCallback(packetSub->psubInt);
    }
    if (packetSub->xptPacketSub) {
        BXPT_PacketSub_CloseChannel(packetSub->xptPacketSub);
    }
    if (packetSub->desc.mmaBlock) {
        BMMA_Unlock(packetSub->desc.mmaBlock, packetSub->desc.ptr);
        BMMA_Free(packetSub->desc.mmaBlock);
    }
    if (packetSub->mmaBlock) {
        BMMA_Unlock(packetSub->mmaBlock, packetSub->buffer);
        BMMA_UnlockOffset(packetSub->mmaBlock, packetSub->offsetBuffer);
        BMMA_Free(packetSub->mmaBlock);
    }
    if (packetSub->finished) {
        NEXUS_IsrCallback_Destroy(packetSub->finished);
    }
    if (packetSub->dataCallback) {
        NEXUS_IsrCallback_Destroy(packetSub->dataCallback);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_PacketSub, packetSub);
    BKNI_Free(packetSub);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_PacketSub, NEXUS_PacketSub_Close);

NEXUS_Error NEXUS_PacketSub_Start( NEXUS_PacketSubHandle packetSub )
{
    BERR_Code rc;
    NEXUS_P_HwPidChannel *pidChannel;

    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    if (packetSub->started) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (!packetSub->settings.pidChannel) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pidChannel = packetSub->settings.pidChannel->hwPidChannel;

    #if !BXPT_HAS_PB_PACKETSUB
    if (pidChannel->status.playback) {
        /* PSUB HW does not support playback */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    #endif

    rc = BXPT_PacketSub_SetPidChanNum(packetSub->xptPacketSub, pidChannel->status.pidChannelIndex, nexus_p_xpt_parser_band(pidChannel));
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_PacketSub_StartChannel(packetSub->xptPacketSub);
    if (rc) return BERR_TRACE(rc);

    packetSub->started = true;

    return 0;
}

static void nexus_p_packetsub_flush_fifo(NEXUS_PacketSubHandle packetSub)
{
    BKNI_EnterCriticalSection();
    packetSub->rptr = packetSub->wptr = 0;
    packetSub->desc.rptr = packetSub->desc.wptr = 0;
    BKNI_LeaveCriticalSection();
    packetSub->lastGetBufferSize = 0;
}

void NEXUS_PacketSub_Stop( NEXUS_PacketSubHandle packetSub )
{
    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    if (packetSub->started) {
        (void)BXPT_PacketSub_StopChannel(packetSub->xptPacketSub);
        packetSub->started = false;
        
        nexus_p_packetsub_flush_fifo(packetSub);
    }
}

NEXUS_Error NEXUS_PacketSub_SetSettings( NEXUS_PacketSubHandle packetSub, const NEXUS_PacketSubSettings *pSettings )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    NEXUS_IsrCallback_Set(packetSub->finished, &pSettings->finished);
    NEXUS_IsrCallback_Set(packetSub->dataCallback, &pSettings->dataCallback);

    if (packetSub->started) {
        /* some settings can't be changed after start */
        if (pSettings->pidChannel != packetSub->settings.pidChannel) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        if (pSettings->loop != packetSub->settings.loop) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    rc = BXPT_PacketSub_SetFullRateOutput(packetSub->xptPacketSub, pSettings->fullRateOutput);
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_PacketSub_SetOutputRate(packetSub->xptPacketSub, pSettings->outputRate);
    if (rc) return BERR_TRACE(rc);

#if BXPT_HAS_PACKETSUB_FORCED_INSERTION
    rc = BXPT_PacketSub_SetForcedInsertion( packetSub->xptPacketSub, pSettings->forcedInsertion );
    if (rc ) return BERR_TRACE( rc );
#endif

    packetSub->settings = *pSettings;
    return 0;
}

void NEXUS_PacketSub_GetSettings( NEXUS_PacketSubHandle packetSub, NEXUS_PacketSubSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);
    *pSettings = packetSub->settings;
}

NEXUS_Error NEXUS_PacketSub_SetPause( NEXUS_PacketSubHandle packetSub, bool paused )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    rc = BXPT_PacketSub_PauseChannel(packetSub->xptPacketSub, paused);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

/* === data; --- available
   |===W---R===|
   |---R===W---|
   |R======W---|
*/
/* continuous space available at wptr */
#define BFIFO_WRITE_AVAILABLE(wptr, rptr, size, min) ((wptr)<(rptr)?(rptr)-(wptr)-(min):(rptr)?(size)-(wptr):(size)-(wptr)-(min))

NEXUS_Error NEXUS_PacketSub_GetBuffer( NEXUS_PacketSubHandle packetSub, void **pBuffer, unsigned *pSize )
{
    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    /* there must be buffer space and a spare desciptor */
    BKNI_EnterCriticalSection();
    if (!BFIFO_WRITE_AVAILABLE(packetSub->desc.wptr, packetSub->desc.rptr, packetSub->desc.size, 1) ||
        !BFIFO_WRITE_AVAILABLE(packetSub->wptr, packetSub->rptr, packetSub->size, 4)) {
        *pBuffer = NULL;
        *pSize = 0;
    }
    else {
        *pBuffer = &packetSub->buffer[packetSub->wptr];
        *pSize = BFIFO_WRITE_AVAILABLE(packetSub->wptr, packetSub->rptr, packetSub->size, 4);
    }
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("GetBuffer(%p) fifo=%d,%d; desc=%d,%d; size=%d", (void *)packetSub, packetSub->rptr, packetSub->wptr, packetSub->desc.rptr, packetSub->desc.wptr, *pSize));

    packetSub->lastGetBufferSize = *pSize;

    return 0;
}

NEXUS_Error NEXUS_PacketSub_WriteCompleteWithSkip( NEXUS_PacketSubHandle packetSub, unsigned skip, unsigned amount )
{
    NEXUS_Error rc;
    BXPT_PacketSub_Descriptor *pDesc;
    BMMA_DeviceOffset bufferOffset;

    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);

    if (skip + amount > packetSub->lastGetBufferSize) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    
    if (skip) {
        BKNI_EnterCriticalSection();
        packetSub->wptr += skip;
        if (packetSub->wptr == packetSub->size) {
            packetSub->wptr = 0;
        }
        BKNI_LeaveCriticalSection();
    }
    
    if (!amount) {
        goto done;
    }
    
    /* flush data in cached fifo before submitting to HW */
    NEXUS_FlushCache(&packetSub->buffer[packetSub->wptr], amount);

    /* prepare descriptor for this submisssion */
    pDesc = &packetSub->desc.ptr[packetSub->desc.wptr];
    bufferOffset = packetSub->offsetBuffer + (unsigned)(&packetSub->buffer[packetSub->wptr] - packetSub->buffer);
    rc = BXPT_PacketSub_CreateDesc(packetSub->xptPacketSub, pDesc, bufferOffset, amount, true,
        packetSub->settings.loop ? pDesc : NULL);
    if (rc) return BERR_TRACE(rc);

    BKNI_EnterCriticalSection();
    if (++packetSub->desc.wptr == packetSub->desc.size) {
        packetSub->desc.wptr = 0;
    }
    packetSub->wptr += amount;
    if (packetSub->wptr == packetSub->size) {
        packetSub->wptr = 0;
    }
    BKNI_LeaveCriticalSection();

    /* must add to HW after incrementing wptr */
    rc = BXPT_PacketSub_AddDescriptors(packetSub->xptPacketSub, pDesc, pDesc);
    if (rc) return BERR_TRACE(rc);
    BDBG_MSG_TRACE(("%p: submit desc %p", packetSub, pDesc));

done:
    BDBG_MSG(("WriteComplete(%p) fifo=%d,%d; desc=%d,%d; skip=%d, amount=%d", (void *)packetSub, packetSub->rptr, packetSub->wptr, packetSub->desc.rptr, packetSub->desc.wptr, skip, amount));

    packetSub->lastGetBufferSize = 0;

    return 0;
}

void NEXUS_PacketSub_Flush( NEXUS_PacketSubHandle packetSub )
{
    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);
    if (packetSub->started) {
        NEXUS_Error rc;
        
        (void)BXPT_PacketSub_StopChannel(packetSub->xptPacketSub);
        rc = BXPT_PacketSub_StartChannel(packetSub->xptPacketSub);
        if (rc) {rc = BERR_TRACE(rc);}
        
        nexus_p_packetsub_flush_fifo(packetSub);
    }
}

NEXUS_Error NEXUS_PacketSub_GetStatus( NEXUS_PacketSubHandle packetSub, NEXUS_PacketSubStatus *pStatus )
{
    BXPT_PacketSub_ChannelStatus chStatus;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(packetSub, NEXUS_PacketSub);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BXPT_PacketSub_GetChannelStatus(packetSub->xptPacketSub, &chStatus);
    if (rc) return BERR_TRACE(rc);
    pStatus->finished = chStatus.Finished;
    pStatus->busy = chStatus.Busy;

    return 0;
}

#else /* BXPT_HAS_PACKETSUB */
struct NEXUS_PacketSub
{
    NEXUS_OBJECT(NEXUS_PacketSub);    
};

void NEXUS_PacketSub_GetDefaultOpenSettings( NEXUS_PacketSubOpenSettings *pOpenSettings )
{
    BSTD_UNUSED(pOpenSettings);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_PacketSubHandle NEXUS_PacketSub_Open( unsigned index, const NEXUS_PacketSubOpenSettings *pOpenSettings )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pOpenSettings);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_PacketSub_P_Finalizer( NEXUS_PacketSubHandle packetSub )
{
    BSTD_UNUSED(packetSub);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_PacketSub, NEXUS_PacketSub_Close);

NEXUS_Error NEXUS_PacketSub_Start( NEXUS_PacketSubHandle packetSub )
{
    BSTD_UNUSED(packetSub);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_PacketSub_Stop( NEXUS_PacketSubHandle packetSub )
{
    BSTD_UNUSED(packetSub);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_PacketSub_SetSettings( NEXUS_PacketSubHandle packetSub, const NEXUS_PacketSubSettings *pSettings )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_PacketSub_GetSettings( NEXUS_PacketSubHandle packetSub, NEXUS_PacketSubSettings *pSettings )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_PacketSub_SetPause( NEXUS_PacketSubHandle packetSub, bool paused )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(paused);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

NEXUS_Error NEXUS_PacketSub_GetBuffer( NEXUS_PacketSubHandle packetSub, void **pBuffer, unsigned *pSize )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

NEXUS_Error NEXUS_PacketSub_WriteCompleteWithSkip( NEXUS_PacketSubHandle packetSub, unsigned skip, unsigned amount )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(skip);
    BSTD_UNUSED(amount);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return 0;
}

void NEXUS_PacketSub_Flush( NEXUS_PacketSubHandle packetSub )
{
    BSTD_UNUSED(packetSub);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_PacketSub_GetStatus( NEXUS_PacketSubHandle packetSub, NEXUS_PacketSubStatus *pStatus )
{
    BSTD_UNUSED(packetSub);
    BSTD_UNUSED(pStatus);
    BDBG_WRN(("Packetsub not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif /* BXPT_HAS_PACKETSUB */

NEXUS_Error NEXUS_PacketSub_WriteComplete( NEXUS_PacketSubHandle packetSub, unsigned amount )
{
    return NEXUS_PacketSub_WriteCompleteWithSkip(packetSub, 0, amount);
}
