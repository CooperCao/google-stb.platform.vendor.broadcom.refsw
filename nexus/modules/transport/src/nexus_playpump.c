/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"

#ifndef BXPT_HAS_MULTICHANNEL_PLAYBACK
#include "bchp_int_id_xpt_pb0.h"
#include "bchp_int_id_xpt_pb1.h"
#if NEXUS_NUM_PLAYPUMPS > 2
#include "bchp_int_id_xpt_pb2.h"
#endif
#if NEXUS_NUM_PLAYPUMPS > 3
#include "bchp_int_id_xpt_pb3.h"
#endif
#if NEXUS_NUM_PLAYPUMPS > 4
#include "bchp_int_id_xpt_pb4.h"
#endif
#if NEXUS_NUM_PLAYPUMPS > 5
#include "bchp_int_id_xpt_pb5.h"
#endif
#if NEXUS_NUM_PLAYPUMPS > 6
#include "bchp_int_id_xpt_pb6.h"
#endif
#if NEXUS_NUM_PLAYPUMPS > 7
#include "bchp_int_id_xpt_pb7.h"
#endif
#endif /* BXPT_HAS_MULTICHANNEL_PLAYBACK */

#include "priv/nexus_timebase_priv.h"
#if NEXUS_NUM_DMA_CHANNELS
#include "nexus_dma.h"
#include "priv/nexus_dma_priv.h"
#endif
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_playpump);
BDBG_FILE_MODULE(nexus_flow_playpump);

#define NEXUS_P_PACKETIZER_BASE (0x100)

static BERR_Code NEXUS_Playpump_P_SetParserBand(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpSettings *pSettings);
static void NEXUS_Playpump_P_InstallRangeErrIntHandler(NEXUS_PlaypumpHandle p);
static void NEXUS_Playpump_P_UninstallRangeErrIntHandler(NEXUS_PlaypumpHandle p);
static NEXUS_Error NEXUS_Playpump_P_SetInterrupts(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpSettings *pSettings);
static NEXUS_Error NEXUS_Playpump_P_SetPause( NEXUS_PlaypumpHandle p);

void NEXUS_Playpump_GetDefaultOpenPidChannelSettings_priv(NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings)
{
    BKNI_Memset(pSettings,0, sizeof(*pSettings));
}

void NEXUS_Playpump_GetDefaultOpenSettings(NEXUS_PlaypumpOpenSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->streamMuxCompatible = false;
    pSettings->fifoSize = B_PVR_PLAYBACK_BUFFER;
    pSettings->alignment = 12; /* 2^12 = 4096  (I/O block size) alignment */
    pSettings->numDescriptors = NEXUS_NUM_PLAYBACK_DESC;
    pSettings->dataNotCpuAccessible = false;
    return;
}

void
NEXUS_Playpump_GetDefaultSettings(NEXUS_PlaypumpSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if NEXUS_NUM_DMA_CHANNELS
    pSettings->securityDmaDataFormat = NEXUS_DmaDataFormat_eMpeg;
#endif
    pSettings->transportType = NEXUS_TransportType_eTs;
    pSettings->originalTransportType = NEXUS_TransportType_eUnknown;
    pSettings->timestamp.type = NEXUS_TransportTimestampType_eNone;
    pSettings->timestamp.timebase = NEXUS_Timebase_eInvalid;
    pSettings->timestamp.pacingMaxError = 1024; /* HW reset value */
    pSettings->mode = NEXUS_PlaypumpMode_eFifo;
    pSettings->playRate = NEXUS_NORMAL_PLAY_SPEED;
    pSettings->allPass = false;
    pSettings->acceptNullPackets = false;
    pSettings->maxDataRate = 108000000;
    pSettings->dataNotCpuAccessible = false;
    NEXUS_CallbackDesc_Init(&pSettings->dataCallback);
    NEXUS_CallbackDesc_Init(&pSettings->errorCallback);
    NEXUS_CallbackDesc_Init(&pSettings->ccError);
    NEXUS_CallbackDesc_Init(&pSettings->teiError);
    return;
}

#if NEXUS_NUM_PLAYPUMPS
#if (BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG)
#define PLAYBACK_HAS_MEMORY(i) (pTransport->settings.clientEnabled.playback[i].rave)
#else
#define PLAYBACK_HAS_MEMORY(i) (true)
#endif
#endif

NEXUS_PlaypumpHandle
NEXUS_Playpump_Open(unsigned index, const NEXUS_PlaypumpOpenSettings *pSettings)
{
#if NEXUS_NUM_PLAYPUMPS
    NEXUS_PlaypumpHandle p;
    BXPT_Playback_ChannelSettings play_cfg;
    BERR_Code rc;
    BPVRlib_Feed_Settings feed_cfg;
    BMMA_Block_Handle feed_bufBlock = NULL;
    NEXUS_PlaypumpOpenSettings defaultSettings;
    NEXUS_HeapHandle heap;

    if (!pSettings) {
        NEXUS_Playpump_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    if (index == NEXUS_ANY_ID) {
        unsigned i;
        for (i=0;i<NEXUS_NUM_PLAYPUMPS;i++) {
            if (pTransport->playpump[i].playpump==NULL && BXPT_Playback_IsAvailable(pTransport->xpt, i) && PLAYBACK_HAS_MEMORY(i)) {
#if NEXUS_HAS_XPT_DMA && (!BXPT_DMA_HAS_MEMDMA_MCPB)
                if (pTransport->dmaChannel[i].dma) { continue; }
#endif
                index = i;
                break;
            }
        }
        if (i == NEXUS_NUM_PLAYPUMPS) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            BDBG_ERR(("no playpump available"));
            return NULL;
        }
    }

#if NEXUS_HAS_XPT_DMA && (!BXPT_DMA_HAS_MEMDMA_MCPB)
    if (pTransport->dmaChannel[index].dma) {
        BDBG_ERR(("playpump[%d]'s MCPB channel is in use by DMA", index));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
#endif

    if (index >= NEXUS_NUM_PLAYPUMPS) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("playpump[%d] not available", index));
        return NULL;
    }

    if (pTransport->playpump[index].playpump) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("playpump[%d] already open", index));
        return NULL;
    }
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(playpump,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }


    pTransport->playpump[index].playpump = p = BKNI_Malloc(sizeof(*p));
    if (!p) {
        NEXUS_CLIENT_RESOURCES_RELEASE(playpump,Count,NEXUS_ANY_ID);
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Playpump, p);

    p->index = index;
    p->openSettings = *pSettings;
    p->consumerStarted = false;
    NEXUS_Playpump_GetDefaultSettings(&p->settings);
    p->settings.dataNotCpuAccessible = pSettings->dataNotCpuAccessible;
    p->settings.transportType = NEXUS_TransportType_eTs;
    p->settings.timestamp.type = NEXUS_TransportTimestampType_eNone;
    p->settings.timestamp.pacing = false;
    p->settings.playRate = NEXUS_NORMAL_PLAY_SPEED;
    p->dataCallback = NEXUS_TaskCallback_Create(p, NULL);
    if(!p->dataCallback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    p->errorCallback = NEXUS_TaskCallback_Create(p, NULL);
    if(!p->errorCallback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    p->ccErrorCallback = NEXUS_IsrCallback_Create(p, NULL);
    if(!p->ccErrorCallback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    p->teiErrorCallback = NEXUS_IsrCallback_Create(p, NULL);
    if(!p->teiErrorCallback) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    b_pid_map_init(&p->packetizer_map, NEXUS_P_PACKETIZER_BASE);

    BLST_S_INIT(&p->pid_list);

    p->item_mem = BKNI_Malloc(sizeof(*p->item_mem)*pSettings->numDescriptors);
    if(p->item_mem==NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    heap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (!heap) {
        heap = g_pCoreHandles->heap[pTransport->moduleSettings.mainHeapIndex].nexus;
    }
    /* some playpump operations require driver CPU accessible to the playback fifo */
    if ( !pSettings->dataNotCpuAccessible && !NEXUS_P_CpuAccessibleHeap(heap)) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }
    p->heap = heap;

    /* fifoSize == 0 is valid for scatter/gather-only mode */
    if (pSettings->fifoSize) {
        if (pSettings->memory) {
            rc = NEXUS_MemoryBlock_Lock(pSettings->memory, &p->buf);
            if (rc) {rc = BERR_TRACE(rc); goto error;}
            p->bufBlock = NULL;
            p->buf = (uint8_t*)p->buf + pSettings->memoryOffset;
            feed_bufBlock = NEXUS_MemoryBlock_GetBlock_priv(pSettings->memory);
        }
        else {
            p->bufBlock = BMMA_Alloc(NEXUS_Heap_GetMmaHandle(p->heap), pSettings->fifoSize, 1<<pSettings->alignment, NULL);
            if (!p->bufBlock) {
                rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
                goto error;
            }
            p->buf = BMMA_Lock(p->bufBlock); /* map memory to the cached region */
            if (p->buf == NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;}
            feed_bufBlock = p->bufBlock;
        }
    }
    BDBG_MODULE_MSG(nexus_flow_playpump, ("open %p, index %d, fifo %d", (void *)p, p->index, (unsigned)pSettings->fifoSize));

    heap = NEXUS_P_DefaultHeap(pSettings->boundsHeap, NEXUS_DefaultHeapType_eBounds);
    if (heap) {
        NEXUS_MemoryStatus heapStatus;

        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if(heapStatus.addr) {
            p->boundsHeapSize = heapStatus.size;
            p->boundsHeapAddr = NEXUS_OffsetToCachedAddr(heapStatus.offset);
        } else {
            (void )BERR_TRACE(NEXUS_INVALID_PARAMETER);
            p->boundsHeapAddr = NULL;
            p->boundsHeapSize = 0;
        }
    }

    b_playpump_p_reset(p);

    rc = BXPT_Playback_GetChannelDefaultSettings(pTransport->xpt, index, &play_cfg);
    if (rc) {rc=BERR_TRACE(rc);goto error;}

#if NEXUS_HAS_LEGACY_XPT
    play_cfg.AlwaysResumeFromLastDescriptor = true;
#endif

    rc = BXPT_Playback_OpenChannel(pTransport->xpt, &p->xpt_play, index, &play_cfg);
    if (rc) {
        unsigned i;
        for (i=index;i<NEXUS_NUM_PLAYPUMPS;i++) {
            if (pTransport->playpump[i].playpump==NULL && BXPT_Playback_IsAvailable(pTransport->xpt, i)) {
                BDBG_ERR(("playback:%u is not available, try playback:%u", index, i));
                break;
            }
        }
        rc=BERR_TRACE(rc);
        goto error;
    }

    rc = NEXUS_Playpump_P_SetParserBand(p, &p->settings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}

    BPVRlib_Feed_GetDefaultSettings(&feed_cfg);
    feed_cfg.xptHandle = pTransport->xpt;
    feed_cfg.xptPlayHandle = p->xpt_play;
    feed_cfg.bufferBlock = feed_bufBlock;
    feed_cfg.bufferOffset = 0;
    feed_cfg.mmaHeap = NEXUS_Heap_GetMmaHandle(g_pCoreHandles->heap[pTransport->moduleSettings.mainHeapIndex].nexus);
    feed_cfg.intHandle = g_pCoreHandles->bint;
    feed_cfg.numDesc = 32;
    if(p->openSettings.streamMuxCompatible) {
        /*
         * when used for StreamMux use number of descriptors as number of descriptors queued in the HW, otherwise numDesc is just number of SW descriptors,
         * SW->HW descriptors refilled by playpump (media conversion step run inline with refill)
         */
        feed_cfg.numDesc = p->openSettings.numDescriptors;
    }
    feed_cfg.applicationCnxt = p;
    feed_cfg.useExtndedDesc = pSettings->streamMuxCompatible;
    feed_cfg.descAvaliable_isr = NEXUS_P_Playpump_DescAvail_isr;
    feed_cfg.offsetToCachedAddr = NEXUS_OffsetToCachedAddr;
    feed_cfg.flushCache = NEXUS_FlushCache;
    rc = BPVRlib_Feed_Open(&p->play_feed, &feed_cfg);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}

    rc = BKNI_CreateEvent(&p->descEvent);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto error;}

#if B_HAS_PLAYPUMP_IP
    {
        rc = b_playpump_ip_open(&p->ip, p, open_params);
        if (rc) { rc = BERR_TRACE(rc);goto error; }
    }
#endif

    /* init vob remap state */
    p->vob_remap_state.codec = NEXUS_AudioCodec_eMpeg;
    p->vob_remap_state.track = 0;

    p->crypto = b_pump_crypto_create(p);
#if 0 /* if security module not connected, NULL is normal */
    if(!p->crypto) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
#endif

#if B_HAS_MEDIA
    p->use_media = false;
    p->demux = b_pump_demux_create(p);
    if(!p->demux) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
#endif

    return p;
error:
    if (p) {
        if (p->crypto) {
            b_pump_crypto_destroy(p->crypto);
        }
        if (p->play_feed) {
            BPVRlib_Feed_Close(p->play_feed);
        }
        if (p->descEvent) {
            BKNI_DestroyEvent(p->descEvent);
        }
        if (p->xpt_play) {
            BXPT_Playback_CloseChannel(p->xpt_play);
        }
        if (pSettings->memory) {
            NEXUS_MemoryBlock_Unlock(pSettings->memory);
        }
        else if (p->bufBlock) {
            if(p->buf) {
                BMMA_Unlock(p->bufBlock, p->buf);
            }
            BMMA_Free(p->bufBlock);
        }
        if(p->item_mem) {
            BKNI_Free(p->item_mem);
        }
        if(p->errorCallback) {
            NEXUS_TaskCallback_Destroy(p->errorCallback);
        }
        if(p->ccErrorCallback) {
            NEXUS_IsrCallback_Destroy(p->ccErrorCallback);
        }
        if(p->teiErrorCallback) {
            NEXUS_IsrCallback_Destroy(p->teiErrorCallback);
        }
        if(p->dataCallback) {
            NEXUS_TaskCallback_Destroy(p->dataCallback);
        }
        BKNI_Free(p);
        pTransport->playpump[index].playpump = NULL;
    }
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
#endif
    return NULL;
}

static void NEXUS_Playpump_P_Release(NEXUS_PlaypumpHandle p)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Playpump, p);
    NEXUS_CLIENT_RESOURCES_RELEASE(playpump,Count,NEXUS_ANY_ID);
}

static void NEXUS_Playpump_P_Finalizer(NEXUS_PlaypumpHandle p)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Playpump, p);

    if (p->state.running) {
        NEXUS_Playpump_Stop(p);
    }
    BDBG_MODULE_MSG(nexus_flow_playpump, ("close %p", (void *)p));
    NEXUS_Playpump_CloseAllPidChannels(p);
#if B_HAS_PLAYPUMP_IP
    b_playpump_ip_close(&p->ip);
#endif
    if (p->crypto) {
        b_pump_crypto_destroy(p->crypto);
    }
#if B_HAS_MEDIA
    b_pump_demux_destroy(p->demux);
#endif
    p->state.packetizer = b_play_packetizer_none;

    /* force the destroying of the xpt interrupt */
    p->settings.ccError.callback = NULL;
    p->settings.teiError.callback = NULL;
    NEXUS_Playpump_P_SetInterrupts(p, &p->settings);

    BPVRlib_Feed_Close(p->play_feed);
    BKNI_DestroyEvent(p->descEvent);
    BXPT_Playback_CloseChannel(p->xpt_play);
    if (p->openSettings.memory) {
        NEXUS_MemoryBlock_Unlock(p->openSettings.memory);
    }
    else if (p->bufBlock) {
        if(p->buf) {
            BMMA_Unlock(p->bufBlock, p->buf);
        }
        BMMA_Free(p->bufBlock);
    }
    BKNI_Free(p->item_mem);
    p->xpt_play = NULL;
    p->play_feed = NULL;
#if NEXUS_NUM_PLAYPUMPS
    pTransport->playpump[p->index].playpump = NULL;
#endif
#if NEXUS_NUM_DMA_CHANNELS
    if(p->settings.securityDma) {
        NEXUS_OBJECT_RELEASE(p, NEXUS_Dma, p->settings.securityDma);
    }
#endif

    NEXUS_IsrCallback_Destroy(p->ccErrorCallback);
    NEXUS_IsrCallback_Destroy(p->teiErrorCallback);
    NEXUS_TaskCallback_Destroy(p->errorCallback);
    NEXUS_TaskCallback_Destroy(p->dataCallback);
    NEXUS_OBJECT_DESTROY(NEXUS_Playpump, p);
    BKNI_Free(p);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Playpump, NEXUS_Playpump_Close);

void NEXUS_Playpump_GetSettings(NEXUS_PlaypumpHandle p, NEXUS_PlaypumpSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BDBG_ASSERT(pSettings);
    *pSettings = p->settings;
    return;
}

static BERR_Code
NEXUS_Playpump_P_SetParserBand(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpSettings *pSettings)
{
    BXPT_Playback_ParserConfig parserConfig;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BDBG_ASSERT(p->xpt_play);

#if B_REFSW_DSS_SUPPORT
    rc = BXPT_DirecTvPlayback_SetParserBandMode( p->xpt_play, NEXUS_IS_DSS_MODE(pSettings->transportType)?
        BXPT_ParserMode_eDirecTv : BXPT_ParserMode_eMpeg); /* XXX this shall precced SetParserConfig, since it resets parser configuration */
    if (rc) return BERR_TRACE(rc);
#endif

    rc = BXPT_Playback_GetParserConfig(p->xpt_play, &parserConfig);
    if (rc) return BERR_TRACE(rc);
    /* for playback, we should just let all data through. if the stream is good, there's no harm.
    if the stream is bad, the decoders must handle this anyway. */
#if NEXUS_PARSER_BAND_CC_CHECK
    parserConfig.ContCountIgnore = !pSettings->continuityCountEnabled;
#else
    if(pSettings->continuityCountEnabled != p->settings.continuityCountEnabled && BLST_S_FIRST(&p->pid_list)!=NULL) {
        BDBG_WRN(("%#lx:continuityCountEnabled wouldn't get applied to aleady opened pids", (unsigned long)p));
    }
#endif
    parserConfig.ErrorInputIgnore = (pSettings->teiError.callback == NULL); /* if we want these errors, then transport cannot ignore */
    parserConfig.AcceptAdapt00 = true;
    parserConfig.AllPass = pSettings->allPass;
    parserConfig.AcceptNulls = pSettings->acceptNullPackets;

#if BXPT_HAS_PCR_PACING
    /* enable PARSER_FORCE_RESTAMP in pcr-pacing mode */
    parserConfig.ForceRestamping = (pSettings->timestamp.pacing && pSettings->timestamp.pcrPacingPid);
#endif
    if (pSettings->timestamp.forceRestamping==true) {
        parserConfig.ForceRestamping = true;
    }

    parserConfig.UsePcrTimeBase = true; /* eInvalid now means get the default timebase and track it */
    if (parserConfig.UsePcrTimeBase) {
        NEXUS_TimebaseHandle timebase = NEXUS_Timebase_Resolve_priv(pSettings->timestamp.timebase);
        if (timebase)
        {
            parserConfig.WhichPcrToUse = timebase->hwIndex;
        }
        else
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    rc = BXPT_Playback_SetParserConfig(p->xpt_play, &parserConfig);
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_Playback_SetBitRate(p->xpt_play, pSettings->maxDataRate);
    if (rc) return BERR_TRACE(rc);

    return 0;
}


NEXUS_Error
NEXUS_Playpump_SetSettings(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpSettings *pSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BDBG_ASSERT(pSettings);

    if (pSettings->playRate == 0) {
        /* playRate == 0 is not pause. It must be non-zero. */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if(p->state.running && (
                p->settings.transportType != pSettings->transportType ||
                p->settings.timestamp.type != pSettings->timestamp.type ||
                p->settings.timestamp.pacing != pSettings->timestamp.pacing ||
                p->settings.securityDma != pSettings->securityDma ||
                p->settings.securityContext != pSettings->securityContext ||
                p->settings.mode != pSettings->mode
                )) {
        BDBG_WRN(("NEXUS_Playpump_SetSettings: %lx can't change settings when started", (unsigned long)p));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_running;
    }

    if (pSettings->securityDma) {
#if NEXUS_HAS_XPT_DMA && !NEXUS_INTEGRATED_M2M_SUPPORT
        BDBG_ERR(("Playpump M2M crypto not supported on this platform. Use NEXUS_KeySlot_AddPidChannel instead."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
#else
        BDBG_WRN(("Playpump M2M crypto has been deprecated. Use NEXUS_KeySlot_AddPidChannel instead."));
#if !NEXUS_HAS_XPT_DMA
        if (!pTransport->moduleSettings.dma) {
            BDBG_ERR(("Transport module does not have dma module handle."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#endif
#endif
    }

    if (pSettings->timestamp.pacingMaxError != p->settings.timestamp.pacingMaxError) {
        rc = BXPT_Playback_SetPacingErrorBound(p->xpt_play, pSettings->timestamp.pacingMaxError);
        if (rc) return BERR_TRACE(rc);
    }

    if (pSettings->timestamp.timebase != p->settings.timestamp.timebase) {
        BXPT_Playback_ChannelSettings cfg;

        rc = BXPT_Playback_GetChannelSettings(p->xpt_play, &cfg);
        if (rc) return BERR_TRACE(rc);

        cfg.UsePcrTimeBase = true; /* eInvalid now means get the default timebase and track it */
        if (cfg.UsePcrTimeBase) {
            NEXUS_TimebaseHandle timebase = NEXUS_Timebase_Resolve_priv(pSettings->timestamp.timebase);
            if (timebase)
            {
                cfg.WhichPcrToUse = timebase->hwIndex;
            }
            else
            {
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }

        rc = BXPT_Playback_SetChannelSettings(p->xpt_play, &cfg);
        if (rc) return BERR_TRACE(rc);
    }

    if(pSettings->transportType != p->settings.transportType) {
        if(BLST_S_FIRST(&p->pid_list)!=NULL) {
            BDBG_WRN(("NEXUS_Playpump_SetSettings: %lx can't change settings when PIDs are attached", (unsigned long)p));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_attached;
        }
#if B_HAS_MEDIA
        rc = b_pump_demux_set_stream_type(p->demux, pSettings->transportType, &p->use_media);
        if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc); goto err_media;}
        BDBG_MSG(("NEXUS_Playpump_SetSettings: %s of stream type %u", p->use_media?"media preprocessing":"native processsing", pSettings->transportType));

        if(!p->use_media) {
            bool supported;

            if (p->crypto) {
                rc = b_pump_crypto_set_stream_type(p->crypto, pSettings->transportType, &supported);
                if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc); goto err_crypto;}
            }
        }
#endif
    }

    rc = NEXUS_Playpump_P_SetParserBand(p, pSettings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_parser_band;}

#if B_HAS_MEDIA
    if(p->settings.playRate != pSettings->playRate) {
        if (p->state.packetizer==b_play_packetizer_media) {
            b_pump_demux_set_rate(p->demux, pSettings->playRate);
        }
    }
#endif
    {
        bool dataNotCpuAccessible = pSettings->dataNotCpuAccessible;
        if(p->openSettings.dataNotCpuAccessible && !dataNotCpuAccessible) {
            BDBG_WRN(("%p:overwrite application supplied NEXUS_PlaypumpSettings.dataNotCpuAccessible", (void *)p));
            dataNotCpuAccessible = true;
        }
#if B_HAS_MEDIA
        if(dataNotCpuAccessible && p->use_media) {
            BDBG_ERR(("SW demux requires CPU accessible memory"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
#endif
        p->settings = *pSettings;
        p->settings.dataNotCpuAccessible = dataNotCpuAccessible;
    }

#if NEXUS_NUM_DMA_CHANNELS
    if (pSettings->securityDma != p->settings.securityDma) {
        if(p->settings.securityDma) {
            NEXUS_OBJECT_RELEASE(p, NEXUS_Dma, p->settings.securityDma);
        }
        if(pSettings->securityDma) {
            NEXUS_OBJECT_ACQUIRE(p, NEXUS_Dma, pSettings->securityDma);
        }
    }
#endif

    NEXUS_TaskCallback_Set(p->dataCallback, &p->settings.dataCallback);
    NEXUS_TaskCallback_Set(p->errorCallback, &p->settings.errorCallback);
    NEXUS_Playpump_P_SetInterrupts(p, &p->settings);

#if B_HAS_MEDIA
err_crypto:
err_media:
#endif
err_parser_band:
err_running:
err_attached:
    return rc;
}

static NEXUS_Error NEXUS_Playpump_P_StartPid(NEXUS_PlaypumpHandle p, NEXUS_P_PlaypumpPidChannel *play_pid)
{
    NEXUS_Error rc;

    /* activate packetizer to all pids that needs packetizer */
    if(play_pid->packetizer.enable) {
        rc = BXPT_Playback_PacketizeStream(p->xpt_play, play_pid->packetizer.context, &play_pid->packetizer.cfg, true);
        if (rc) return BERR_TRACE(rc);
        play_pid->packetizer.active = true;
    }

    return 0;
}

static void NEXUS_Playpump_P_StopPid(NEXUS_PlaypumpHandle p, NEXUS_P_PlaypumpPidChannel *play_pid)
{
    NEXUS_Error rc;

    if(play_pid->packetizer.enable) {
        rc = BXPT_Playback_PacketizeStream(p->xpt_play, play_pid->packetizer.context, &play_pid->packetizer.cfg, false);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        play_pid->packetizer.active = false;
    }
}

static NEXUS_Error NEXUS_Playpump_Start_priv(NEXUS_PlaypumpHandle p, bool muxInput)
{
    BERR_Code rc;
    BXPT_Playback_ChannelSettings cfg;
    NEXUS_TransportType transportType;
    NEXUS_P_PlaypumpPidChannel *play_pid;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);

    BDBG_MODULE_MSG(nexus_flow_playpump, ("start %p, transportType %d", (void *)p, p->settings.transportType));
    if(NEXUS_GetEnv("profile_playpump")) {
        NEXUS_Profile_Start();
    }

    if (p->state.running) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_state; }

    /* If we're in allPass, verify that there is one, and only one, PID channel on the playback. Check that the PID
       channel used is correct too. */
    if (p->settings.allPass)
    {
        unsigned AllPassPidChannel;
        NEXUS_P_PlaypumpPidChannel *play_pid;

        unsigned ChannelCount = 0;
        bool FoundValidChannel = false;

        NEXUS_Playpump_GetAllPassPidChannelIndex( p, &AllPassPidChannel );
        for(play_pid=BLST_S_FIRST(&p->pid_list); play_pid; play_pid=BLST_S_NEXT(play_pid, link))
        {
            ChannelCount++;
            if(play_pid->pid_channel->status.pidChannelIndex == AllPassPidChannel)
            {
                FoundValidChannel = true;
            }
        }

        if( !ChannelCount || 1 < ChannelCount )
        {
            BDBG_ERR(( "Only 1 PID channel is supported in allPass mode." ));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }

        if( !FoundValidChannel )
        {
            BDBG_ERR(( "Incorrect PID channel used for allPass. See NEXUS_Playpump_GetAllPassPidChannelIndex()." ));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
    }

    BKNI_Memset(&p->state, 0, sizeof(p->state)); /* wipe-out all temporary state */
    BKNI_Memset(&cfg, 0, sizeof(cfg));

    rc = NEXUS_Playpump_P_SetParserBand(p, &p->settings);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_parser_band;}

    transportType = p->settings.transportType;
#if B_HAS_MEDIA
    p->state.packetizer = p->use_media ? b_play_packetizer_media:b_play_packetizer_none;
    if (p->state.packetizer==b_play_packetizer_media) {
        transportType = NEXUS_TransportType_eMpeg2Pes;
    } else
#endif
#if NEXUS_NUM_DMA_CHANNELS
    if (p->settings.securityDma && p->crypto) {
        p->state.packetizer = b_play_packetizer_crypto;
    }
#endif

    rc = BXPT_Playback_GetChannelSettings(p->xpt_play, &cfg);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

#if NEXUS_HAS_LEGACY_XPT
    cfg.AlwaysResumeFromLastDescriptor = true;
#endif
#if B_HAS_NATIVE_MPEG1
    /* initialize to the MPEG2 Program Stream Mode */
    cfg.PsMode = BXPT_Playback_PS_Mode_MPEG2;
    cfg.PackHdrConfig = BXPT_Playback_PackHdr_Drop;
#endif

#if B_PACKETIZE_HSX
    if (!(transportType == NEXUS_TransportType_eTs || NEXUS_IS_DSS_MODE(transportType))) {
        cfg.SyncMode = BXPT_PB_SYNC_MPEG_BLIND; /* TS Blind */
        cfg.PacketLength = 184;
    } else
#endif
    switch(transportType) {
    case NEXUS_TransportType_eEs:
        cfg.SyncMode = BXPT_PB_SYNC_BYPASS;
        cfg.PacketLength = 184;
        break;
#if B_HAS_NATIVE_MPEG1
    case NEXUS_TransportType_eMpeg1Ps:
        cfg.PsMode = BXPT_Playback_PS_Mode_MPEG1;
        cfg.PackHdrConfig = BXPT_Playback_PackHdr_Drop;
        /* fallthrough */
#endif
    case NEXUS_TransportType_eVob:
    case NEXUS_TransportType_eMpeg2Pes:
        cfg.SyncMode = BXPT_PB_SYNC_PES; /* TS PES */
        break;

    case NEXUS_TransportType_eTs:
        cfg.PacketLength = 188;
        cfg.SyncMode = BXPT_PB_SYNC_MPEG; /* TS mode. This allows HW to search for the 0x47 sync byte. */
        break;
    case NEXUS_TransportType_eDssPes:
    case NEXUS_TransportType_eDssEs:
#if B_REFSW_DSS_SUPPORT
        cfg.PacketLength = 130;
        #if BXPT_HAS_MULTICHANNEL_PLAYBACK
        cfg.SyncMode = BXPT_PB_SYNC_DSS;
        #else
        cfg.SyncMode = BXPT_PB_SYNC_MPEG_BLIND; /* direct TV mode. Use MPEG blind-sync, per DVT's suggestion. PR 42365 */
        #endif
#else
        BDBG_ERR(("DSS not supported. Compile with B_REFSW_DSS_SUPPORT=y to enable."));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
#endif
        break;
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    case NEXUS_TransportType_eBulk:
        cfg.PacketLength = 192;
        cfg.SyncMode = BXPT_PB_SYNC_BULK; /* TS mode. This allows HW to search for the 0x47 sync byte. */
        break;
#endif
    default:
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    if (p->settings.blindSync) {
        cfg.SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
    }

    /* DisableTimestampParityCheck = false for all except e28_4P_Mod300
       Use32BitTimestamps = true if e32_* */
    switch (p->settings.timestamp.type) {
    case NEXUS_TransportTimestampType_eNone:
        cfg.TimestampEn = false;
        break;
    case NEXUS_TransportTimestampType_e30_2U_Mod300:
        cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Mod300;
        cfg.TimestampEn = true;
        #if BXPT_HAS_32BIT_PB_TIMESTAMPS
        cfg.Use32BitTimestamps = false;
        #endif
        cfg.DisableTimestampParityCheck = true;
        break;
    case NEXUS_TransportTimestampType_e30_2U_Binary:
        cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
        cfg.TimestampEn = true;
        #if BXPT_HAS_32BIT_PB_TIMESTAMPS
        cfg.Use32BitTimestamps = false;
        #endif
        cfg.DisableTimestampParityCheck = true;
        break;
    #if BXPT_HAS_32BIT_PB_TIMESTAMPS
    case NEXUS_TransportTimestampType_e32_Mod300:
        cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Mod300;
        cfg.TimestampEn = true;
        cfg.Use32BitTimestamps = true;
        cfg.DisableTimestampParityCheck = true;
        break;
    case NEXUS_TransportTimestampType_e32_Binary:
        cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
        cfg.TimestampEn = true;
        cfg.Use32BitTimestamps = true;
        cfg.DisableTimestampParityCheck = true;
        break;
    case NEXUS_TransportTimestampType_e28_4P_Mod300:
        cfg.TimestampMode = BXPT_TimestampMode_e28_4P_Mod300;
        cfg.TimestampEn = true;
        cfg.Use32BitTimestamps = false;
        cfg.DisableTimestampParityCheck = false;
        break;
    #endif
    default:
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    if (p->settings.timestamp.pcrPacingPid && p->settings.timestamp.pacing) {
#if BXPT_HAS_PCR_PACING
        cfg.PcrPacingPid = p->settings.timestamp.pcrPacingPid;
        cfg.PcrBasedPacing = true;
        if (cfg.TimestampEn) {
            /* for PCR pacing with TTS stream, TimestampMode is the type of timestamp
               in the PCR, not the timestamp that prepends the transport packet */
            cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Mod300;
        }
#else
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
#endif
    }

    cfg.ResetPacing = p->settings.timestamp.resetPacing;
    cfg.UsePcrTimeBase = true; /* eInvalid now means get the default timebase and track it */
    if (cfg.UsePcrTimeBase) {
        NEXUS_TimebaseHandle timebase = NEXUS_Timebase_Resolve_priv(p->settings.timestamp.timebase);
        if (timebase)
        {
            cfg.WhichPcrToUse = timebase->hwIndex;
        }
        else
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    cfg.PacingOffsetAdjustDisable = p->settings.timestamp.pacingOffsetAdjustDisable;

#if BXPT_HAS_TSMUX
    if(muxInput) {
        cfg.PesBasedPacing = true;
        cfg.Use8WordDesc = true;
        cfg.PesBasedPacing = true;
        cfg.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
        cfg.Use32BitTimestamps = true;
        cfg.TimestampEn = false;
    } else {
        cfg.PesBasedPacing = false;
        cfg.Use8WordDesc = false;
    }
#else
    BSTD_UNUSED(muxInput);
#endif
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    /* nexus layer pacing counter allocation for non-TSmux playpump! TSmux pacing counter would be alloc'd inside PI */
    if (p->settings.timestamp.pacing && !muxInput) {
        p->pacingCounter = BXPT_Playback_AllocPacingCounter(pTransport->xpt);
        if (!p->pacingCounter) {
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto error;
        }
        cfg.PacingCounter = p->pacingCounter;
    }
#endif
    rc = BXPT_Playback_SetChannelSettings(p->xpt_play, &cfg);
    if (rc) {rc=BERR_TRACE(rc);goto error;}

#if B_HAS_VBI
    rc = bstream_p_vbi_open(&p->stream);
    if (rc) goto error;
#endif

    if (p->settings.timestamp.pacing) {
        rc = BXPT_Playback_ConfigPacing(p->xpt_play, BXPT_PacingControl_eStart);
        if (rc) goto error;
        p->state.pacing = true;

        NEXUS_Playpump_P_InstallRangeErrIntHandler(p);
    }

    for(play_pid=BLST_S_FIRST(&p->pid_list);play_pid!=NULL;play_pid=BLST_S_NEXT(play_pid, link)) {
        rc = NEXUS_Playpump_P_StartPid(p, play_pid);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto error; }
    }

#if B_HAS_MEDIA
    if (p->state.packetizer==b_play_packetizer_media) {
        rc = b_pump_demux_start(p->demux);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    } else
#endif
    if (p->state.packetizer==b_play_packetizer_crypto) {
        rc = b_pump_crypto_start(p->crypto);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    } else {
        rc = BPVRlib_Feed_Start(p->play_feed);
        if (rc) {rc=BERR_TRACE(rc);goto error;}
    }

    /* pause playback until first consumer (decode or record) is started. this prevent data from being lost
    if any amount of time between playback start and rave configuration (which is done at decode start). */
    if(!p->consumerStarted) { /* 't pause on Start */
        rc = NEXUS_Playpump_P_SetPause(p);
        if (rc) {rc=BERR_TRACE(rc);goto error_pause;}
    }

    /* We may have a left over event, and we want to ensure that 1 interrupt
    always means 1 free descriptor in b_playpump_p_xpt_event */
    BKNI_ResetEvent(p->descEvent);

    p->playEventHandle = NEXUS_RegisterEvent(p->descEvent, b_playpump_p_xpt_event, p);
    if (!p->playEventHandle) {rc=BERR_TRACE(NEXUS_UNKNOWN); goto error_started;}

    /* Set run state because calling first read */
    p->state.running = true;
    p->state.muxInput = muxInput;

    if(!muxInput) {
        /* Request the first async read, which kickstarts the read cycle. */
        b_playpump_p_do_read_callback(p);

        p->throttle_timer = NEXUS_ScheduleTimer(B_THROTTLE_TIMEOUT, b_playpump_p_throttle_timer, p);  /* schedulle timer after 30 ms */
    }

    return 0;

error_pause:
error_started:
#if B_HAS_MEDIA
    if (p->state.packetizer==b_play_packetizer_media) {
        b_pump_demux_stop(p->demux);
    } else
#endif
    if (p->state.packetizer==b_play_packetizer_crypto) {
        b_pump_crypto_stop(p->crypto);
    } else {
        BPVRlib_Feed_Stop(p->play_feed);
    }
    if (p->state.pacing) {
        NEXUS_Playpump_P_UninstallRangeErrIntHandler(p);

        rc = BXPT_Playback_ConfigPacing(p->xpt_play,BXPT_PacingControl_eStop);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    }
err_parser_band:
err_state:
error:
    BDBG_ASSERT(rc);
    return rc;
}

NEXUS_Error NEXUS_Playpump_Start(NEXUS_PlaypumpHandle p)
{
    if(p->openSettings.streamMuxCompatible) {
        /* Can't use this playpump for as a regular playback */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return NEXUS_Playpump_Start_priv(p, false);
}

NEXUS_Error NEXUS_Playpump_StartMuxInput_priv(NEXUS_PlaypumpHandle playpump)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    return NEXUS_Playpump_Start_priv(playpump, true);
}


void NEXUS_Playpump_Stop(NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;
    NEXUS_P_PlaypumpPidChannel *play_pid;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    if(!p->state.running) {
        BDBG_WRN(("NEXUS_Playpump_Stop: %p playpump already stopped", (void *)p));
        return;
    }
    p->consumerStarted = false; /* don't pause on Start */

    BDBG_MODULE_MSG(nexus_flow_playpump, ("stop %p", (void *)p));
    NEXUS_UnregisterEvent(p->playEventHandle);
    if (p->state.pacing) {
        NEXUS_Playpump_P_UninstallRangeErrIntHandler(p);
        rc = BXPT_Playback_ConfigPacing(p->xpt_play, BXPT_PacingControl_eStop);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("ignored error %#x from BXPT_Playback_ConfigPacing", rc));
        }
        p->state.pacing = false;
    }
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
     if (p->pacingCounter) {
         BXPT_Playback_ChannelSettings chSettings;
         BXPT_Playback_GetChannelSettings(p->xpt_play, &chSettings);
         chSettings.PacingCounter = NULL;
         BXPT_Playback_SetChannelSettings(p->xpt_play, &chSettings);
         BXPT_Playback_FreePacingCounter(p->pacingCounter);
     }
#endif
    rc = NEXUS_Playpump_Flush(p);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    /* after we've flushed, we can state we're not running */
    p->state.running = false;
    p->state.muxInput = false;

    p->settings.playRate = NEXUS_NORMAL_PLAY_SPEED; /* reset a play rate */
#if B_HAS_MEDIA
    if (p->state.packetizer==b_play_packetizer_media) {
        b_pump_demux_stop(p->demux);
    } else
#endif
    if (p->state.packetizer==b_play_packetizer_crypto) {
        b_pump_crypto_stop(p->crypto);
    } else {
        BPVRlib_Feed_Stop(p->play_feed);
    }

    for(play_pid=BLST_S_FIRST(&p->pid_list);play_pid!=NULL;play_pid=BLST_S_NEXT(play_pid, link)) {
        NEXUS_Playpump_P_StopPid(p, play_pid);
    }
    rc = BXPT_Playback_DisablePacketizers(p->xpt_play);
    if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    if (p->throttle_timer) {
        NEXUS_CancelTimer(p->throttle_timer);
        p->throttle_timer = NULL;
    }

    if(NEXUS_GetEnv("profile_playpump")) {
        NEXUS_Profile_Stop("NEXUS_Playpump");
    }

    return;
}

NEXUS_Error NEXUS_Playpump_Flush(NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);

    if (!p->state.running) {
        /* no need to flush if not started. if it is stopped, BPVRlib_Feed_Stop/Start further down
        will have the unintentional effect of restarting playback. */
        return 0;
    }
    BDBG_MODULE_MSG(nexus_flow_playpump, ("flush %p", (void *)p));

#if B_HAS_MEDIA
    if (p->state.packetizer==b_play_packetizer_media) {
        b_pump_demux_flush(p->demux);
    } else
#endif
    if (p->state.packetizer==b_play_packetizer_crypto) {
        b_pump_crypto_flush(p->crypto);
    }
    b_playpump_p_reset(p);

    if (p->state.packetizer!=b_play_packetizer_media) {
        BPVRlib_Feed_Stop(p->play_feed);
        rc = BPVRlib_Feed_Start(p->play_feed);
        if (rc) return BERR_TRACE(rc);
    }

    p->state.last_addr = NULL;
    p->state.last_size = 0;

    /* PI will clear pause, so restore it */
    if (p->paused) {
        rc = NEXUS_Playpump_P_SetPause(p);
        if (rc) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_Playpump_GetBuffer(NEXUS_PlaypumpHandle p, void **buffer, size_t *size)
{
    uint8_t *addr;
    unsigned freeSize, freeDesc;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);

    if (!p->state.running) {
        /* don't print error message, because this is a normal exit from thread processing */
        return NEXUS_UNKNOWN;
    }

    freeSize = BFIFO_WRITE_PEEK(&p->fifo);
    if(freeSize==0) {
        BDBG_MSG_FLOW(("Playback buffer is full, keep waiting"));
        goto keep_waiting;
    }
    freeDesc = BFIFO_WRITE_LEFT(&p->activeFifo);
    if(freeDesc==0) {
        BDBG_MSG_FLOW(("no chunks available, waiting for any"));
        goto keep_waiting;
    }
    BDBG_ASSERT(BFIFO_WRITE_PEEK(&p->pendingFifo));
    BDBG_ASSERT(BFIFO_WRITE(&p->activeFifo)==BFIFO_WRITE(&p->pendingFifo));
    *size = freeSize;
    p->state.last_size = freeSize;
    addr = BFIFO_WRITE(&p->fifo);
    BDBG_ASSERT(addr);

    p->state.last_addr = addr;
    *buffer = p->state.last_addr;
    BDBG_MSG_FLOW(("get_buffer %#lx, %d", (unsigned long)*buffer, *size));
    return 0;

keep_waiting:
    *buffer = NULL;
    *size = 0;
    return 0;

}

NEXUS_Error NEXUS_Playpump_WriteComplete(NEXUS_PlaypumpHandle p, size_t skip, size_t amount_used)
{
    BERR_Code rc;
    unsigned amount_to_commit = skip + amount_used;
    unsigned amount_to_skip = skip;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);

    if (amount_to_commit > p->state.last_size) {
        BDBG_ERR(("GetBuffer returned %u, WriteComplete called with %u,%u(%u)",  (unsigned)p->state.last_size, (unsigned)skip, (unsigned)amount_used, amount_to_commit));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (!p->state.running) {
        /* don't print error message, because this is a normal exit from thread processing */
        return NEXUS_UNKNOWN;
    }

    if (!p->state.last_addr) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    if (skip==0 && amount_used==0) {
        BDBG_MSG(("%#lx loop detected", (unsigned long)p)); /* player sends an empty entry if it's about to loop a stream */
    }

    /* make sure it's in the physical memory so the chip can read it */
    BDBG_MSG_FLOW(("write_complete %#lx:%u %u %#lx", (unsigned long)p->state.last_addr, skip, amount_used, (unsigned long)BFIFO_WRITE(&p->activeFifo)));

    rc = b_playpump_p_add_request(p, amount_to_skip, amount_to_commit - amount_to_skip, NULL);
    if (rc) return BERR_TRACE(rc);

    p->state.last_addr = NULL;
    p->state.last_size = 0;

    return 0;
}

NEXUS_Error NEXUS_Playpump_SubmitScatterGatherDescriptor(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpScatterGatherDescriptor *pDesc, size_t numDescriptors, size_t *pNumConsumed)
{
    BERR_Code rc;
    unsigned nFree;
    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BDBG_ASSERT(pNumConsumed);

    if (!p->state.running) {
        /* don't print error message, because this is a normal exit from thread processing */
        return NEXUS_UNKNOWN;
    }

    *pNumConsumed = 0;

    nFree = BFIFO_WRITE_LEFT(&p->activeFifo);
    if (nFree==0 || numDescriptors==0) {
        return NEXUS_SUCCESS;
    }
    else if (nFree < numDescriptors) {
        numDescriptors = nFree;
    }

    BDBG_MSG_FLOW(("submit_sg %#lx: %#lx", (unsigned long)p->state.last_addr, (unsigned long)BFIFO_WRITE(&p->activeFifo)));

    rc = b_playpump_p_add_request(p, 0, numDescriptors*sizeof(NEXUS_PlaypumpScatterGatherDescriptor), pDesc);
    if (rc) return BERR_TRACE(rc);

    *pNumConsumed = numDescriptors;

    return 0;
}

NEXUS_Error NEXUS_Playpump_GetStatus(NEXUS_PlaypumpHandle p, NEXUS_PlaypumpStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->started = p->state.running;
    pStatus->fifoSize = p->openSettings.fifoSize;
    pStatus->descFifoSize = p->openSettings.numDescriptors;
    pStatus->bytesPlayed = p->state.bytes_played;
    pStatus->bufferBase = p->buf;
    pStatus->fifoDepth = BFIFO_READ_LEFT(&p->fifo);
    pStatus->descFifoDepth = BFIFO_READ_LEFT(&p->activeFifo);
    pStatus->index=p->index;
    pStatus->pacingTsRangeError = p->state.pacingTsRangeError;
    pStatus->syncErrors = 0;
    pStatus->resyncEvents = 0;
    pStatus->streamErrors = 0;
    pStatus->mediaPtsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->mediaPts = 0;

    /* for scatter-gather, use fifoDepth for sum of payload bytes */
    if (pStatus->fifoDepth==0 && pStatus->descFifoDepth!=0) {
        size_t sum = 0;
        struct bpvr_queue fifo = p->activeFifo;
        while (BFIFO_READ_LEFT(&fifo)) {
            struct bpvr_queue_item *item = BFIFO_READ(&fifo);
            sum += item->desc.length;
            BFIFO_READ_COMMIT(&fifo, 1);
        }
        pStatus->fifoDepth = sum;
        if (sum) {
            pStatus->fifoSize = sum;
        }
    }

#if B_HAS_MEDIA
    if(p->state.packetizer==b_play_packetizer_media) {
        b_pump_demux_status(p->demux, pStatus);
    } else
#endif
    if(p->state.packetizer==b_play_packetizer_crypto) {
        b_pump_crypto_status(p->crypto);
    }
    return NEXUS_SUCCESS;
}

void NEXUS_Playpump_P_ConsumerStarted(NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    if (!p->consumerStarted) {
        p->consumerStarted = true; /* don't pause on Start */
        if (p->state.running) {
            rc = NEXUS_Playpump_P_SetPause(p);
            if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);}
        }
    }
    return;
}

static void
NEXUS_Playpump_P_SetPacketizerCfg(BXPT_Playback_PacketizeConfig *pkt_cfg, NEXUS_TransportType type, uint16_t oldpid, uint16_t newpid,
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings)
{
    bool remapped = false;
    BDBG_MSG(("mapping stream id %#x to pid %#x", (unsigned)oldpid, (unsigned)newpid));

    BXPT_Playback_GetPacketizerDefaults(pkt_cfg);
    pkt_cfg->Pid = newpid;
    pkt_cfg->ChannelNum = 0;
    if(type == NEXUS_TransportType_eEs) {
        pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Es;
    } else {
        pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_Sid;
        pkt_cfg->FilterConfig.StreamId = oldpid;
    }
    if (type == NEXUS_TransportType_eVob) {
        if (pSettings->pidType == NEXUS_PidType_eAudio) {
            switch (pSettings->pidTypeSettings.audio.codec) {
            case NEXUS_AudioCodec_eLpcmDvd:
            case NEXUS_AudioCodec_eLpcmHdDvd:
            case NEXUS_AudioCodec_eLpcmBluRay:
                pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_SidSubSid;
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.Id = (oldpid & 0xFF);
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId = 0xA0 | (oldpid>>8);
                remapped = true;
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_SidSubSid;
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.Id = (oldpid & 0xFF);
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId = 0xC0 | (oldpid>>8);
                remapped = true;
                break;
            case NEXUS_AudioCodec_eAc3:
                pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_SidSubSid;
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.Id = (oldpid & 0xFF);
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId = 0x80 | (oldpid>>8);
                remapped = true;
                break;
            case NEXUS_AudioCodec_eDts:
                pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_SidSubSid;
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.Id = (oldpid & 0xFF);
                pkt_cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId = 0x88 | (oldpid>>8);
                remapped = true;
                break;
            default:
                break;
            }
        }
    }
    if((type == NEXUS_TransportType_eVob || type == NEXUS_TransportType_eMpeg2Pes) && oldpid >= 0x100 && !remapped ) {
        pkt_cfg->PacketizerMode = BXPT_Playback_PacketizerMode_Pes_SidSubSid;
        pkt_cfg->FilterConfig.StreamIdAndSubStreamId.Id = (oldpid & 0xFF);
        pkt_cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId =(oldpid>>8);
    }
    return;
}

/* OpenPidChannel and activate packetization if needed */
static NEXUS_Error
NEXUS_Playpump_P_OpenHwPid(NEXUS_PlaypumpHandle p, NEXUS_P_PlaypumpPidChannel *play_pid, unsigned oldpid, NEXUS_TransportType type,
    NEXUS_TransportType originalType, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings, const NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings_priv)
{
    uint16_t newpid = 0;
    BERR_Code rc;
    NEXUS_P_HwPidChannel *hwPidChannel=NULL;
    NEXUS_PidChannelStatus pid_status;

    play_pid->packetizer.enable = false;
    play_pid->packetizer.active = false;
    switch (type) {
    default:
        hwPidChannel = NEXUS_P_HwPidChannel_Open(NULL, p, oldpid, &pSettings->pidSettings, p->settings.continuityCountEnabled);
        if(!hwPidChannel) { rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_pid_channel_ts; }
        hwPidChannel->status.transportType = type;
        hwPidChannel->status.originalTransportType = originalType;
        goto opened;

    case NEXUS_TransportType_eEs:
    case NEXUS_TransportType_eVob:
    case NEXUS_TransportType_eMpeg2Pes:
    case NEXUS_TransportType_eMpeg1Ps:
        break;
    }
    newpid = b_pid_map_alloc(&p->packetizer_map);
    if(newpid==0) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_newpid;
    }

    /* the remainder of this function handles packetization */
    NEXUS_Playpump_P_SetPacketizerCfg(&play_pid->packetizer.cfg, type, oldpid, pSettings_priv?pSettings_priv->tsPid:newpid, pSettings);
    play_pid->packetizer.cfg.preserveCC = pSettings_priv?pSettings_priv->preserveCC:false;

    /* packetizer always generates cc = 0, so we cannot use continuityCountEnabled */
    hwPidChannel = NEXUS_P_HwPidChannel_Open(NULL, p, pSettings_priv?pSettings_priv->tsPid:newpid, &pSettings->pidSettings, false);
    if(!hwPidChannel) { rc = BERR_TRACE(BERR_NOT_SUPPORTED); goto err_pid_channel; }

    hwPidChannel->status.transportType = NEXUS_TransportType_eTs;
    hwPidChannel->status.originalTransportType = originalType;
    hwPidChannel->status.remappedPid = oldpid;
    if(pSettings_priv) {
        hwPidChannel->settingsPriv = *pSettings_priv;
        hwPidChannel->settingsPrivValid = true;
    }

    rc = NEXUS_P_HwPidChannel_GetStatus(hwPidChannel, &pid_status);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_pid_status; }
    play_pid->packetizer.context = newpid - NEXUS_P_PACKETIZER_BASE;
    play_pid->packetizer.cfg.ChannelNum = pid_status.pidChannelIndex;
    BDBG_ASSERT(newpid >= NEXUS_P_PACKETIZER_BASE);

    play_pid->packetizer.enable = true;

opened:
    play_pid->pid_channel = hwPidChannel;
    if (p->state.running) {
        rc = NEXUS_Playpump_P_StartPid(p, play_pid);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_packetize; }
    }

    return 0;

err_packetize:
err_pid_status:
    NEXUS_P_HwPidChannel_CloseAll(hwPidChannel);
err_pid_channel:
    if (newpid) {
        b_pid_map_free(&p->packetizer_map, newpid);
    }
err_newpid:
err_pid_channel_ts:
    play_pid->pid_channel = NULL;
    BDBG_ASSERT(rc);
    return rc;
}

static void
NEXUS_Playpump_P_ClosePid(NEXUS_PlaypumpHandle p, NEXUS_P_PlaypumpPidChannel *play_pid)
{
#if B_HAS_MEDIA
    if(p->use_media) {
        if(p->state.running) {
            b_pump_demux_remove_pid(p->demux, play_pid);
        }
        b_pump_demux_close_pid(p->demux, play_pid);
    }
#endif
    if (p->state.running) {
        NEXUS_Playpump_P_StopPid(p, play_pid);
    }

    if(play_pid->packetizer.enable) {
        b_pid_map_free(&p->packetizer_map, play_pid->packetizer.context+NEXUS_P_PACKETIZER_BASE);
    }
    return;
}

void
NEXUS_Playpump_GetDefaultOpenPidChannelSettings(NEXUS_PlaypumpOpenPidChannelSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_PidChannel_GetDefaultSettings(&pSettings->pidSettings);
    pSettings->pidType = NEXUS_PidType_eUnknown;
    pSettings->allowTimestampReordering = true;
    return;
}



static NEXUS_PidChannelHandle
NEXUS_Playpump_P_OpenPidChannel_MuxImpl(NEXUS_PlaypumpHandle p, unsigned pid, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings, const NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings_priv)
{
    BERR_Code rc;
    NEXUS_P_PlaypumpPidChannel *play_pid;
    NEXUS_PlaypumpOpenPidChannelSettings settings;
    NEXUS_TransportType transportType;
    NEXUS_PidChannelHandle pidChannel;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    if(!pSettings) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&settings);
        pSettings = &settings;
    }

    transportType = p->settings.transportType;

    /* detect if user tries to open the same pid (track) twice  */
    for(play_pid=BLST_S_FIRST(&p->pid_list);play_pid;play_pid=BLST_S_NEXT(play_pid,link)) {
        if(play_pid->pid == pid && (pSettings->pidSettings.pidChannelIndex == (unsigned)-1 || pSettings->pidSettings.pidChannelIndex == play_pid->settings.pidSettings.pidChannelIndex) && (play_pid->packetizer.cfg.preserveCC == (pSettings_priv?pSettings_priv->preserveCC:false))) {
            pidChannel = NEXUS_PidChannel_P_Create(play_pid->pid_channel);
            return pidChannel;
        }
    }

    play_pid = BKNI_Malloc(sizeof(*play_pid));
    if(!play_pid) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}

    play_pid->pid = pid;
    play_pid->settings = *pSettings;
#if B_HAS_MEDIA
    if(p->use_media) {
        transportType = NEXUS_TransportType_eMpeg2Pes;
        rc  = b_pump_demux_open_pid(p->demux, play_pid, pid, pSettings);
        if(rc!=NEXUS_SUCCESS) {
            goto err_media_open;
        }
        pid = play_pid->media_pid;
        if (p->state.running) {
            rc = b_pump_demux_add_pid(p->demux, play_pid);
            if(rc!=NEXUS_SUCCESS) {
                goto err_media_add;
            }
        }
    }
#endif
    rc = NEXUS_Playpump_P_OpenHwPid(p, play_pid, pid, transportType,
            p->settings.originalTransportType==NEXUS_TransportType_eUnknown?p->settings.transportType:p->settings.originalTransportType,
            pSettings, pSettings_priv);
    if (rc) { rc = BERR_TRACE(rc); goto err_pid_channel;}
    BLST_S_DICT_ADD(&p->pid_list, play_pid, NEXUS_P_PlaypumpPidChannel, pid_channel, link, err_duplicate);
    pidChannel = NEXUS_PidChannel_P_Create(play_pid->pid_channel);
    if(pidChannel==NULL) {
        NEXUS_Playpump_P_ClosePid(p, play_pid);
	return NULL;
    }

    return pidChannel;

err_duplicate:
    BDBG_ERR(("NEXUS_Playpump_OpenPidChannel: %#lx detected duplicate pid %#lx", (unsigned long)p, (unsigned long)play_pid->pid_channel));
err_pid_channel:
#if B_HAS_MEDIA
    if(p->use_media && p->state.running) {
        b_pump_demux_remove_pid(p->demux, play_pid);
    }
err_media_add:
    if(p->use_media) {
        b_pump_demux_close_pid(p->demux, play_pid);
    }
err_media_open:
#endif
    BKNI_Free(play_pid);
err_alloc:
    return NULL;
}

NEXUS_PidChannelHandle
NEXUS_Playpump_OpenPidChannel(NEXUS_PlaypumpHandle p, unsigned pid, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = NEXUS_Playpump_P_OpenPidChannel_MuxImpl(p, pid, pSettings, NULL);
    if (pidChannel) {
        NEXUS_OBJECT_REGISTER(NEXUS_PidChannel, pidChannel, Open);
    }
    return pidChannel;
}

NEXUS_PidChannelHandle
NEXUS_Playpump_OpenPidChannel_priv( NEXUS_PlaypumpHandle p, unsigned src_pid, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings, const NEXUS_Playpump_OpenPidChannelSettings_priv *pSettings_priv)
{
    NEXUS_ASSERT_MODULE();
    return NEXUS_Playpump_P_OpenPidChannel_MuxImpl(p, src_pid, pSettings, pSettings_priv);
}

NEXUS_Error
NEXUS_Playpump_ClosePidChannel(NEXUS_PlaypumpHandle p, NEXUS_PidChannelHandle pidChannel)
{
    BSTD_UNUSED(p);
    NEXUS_PidChannel_Close(pidChannel);
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_Playpump_P_HwPidChannel_Disconnect(NEXUS_PlaypumpHandle p, NEXUS_P_HwPidChannel *pidChannel)
{
    NEXUS_Error rc;
    NEXUS_P_PlaypumpPidChannel *play_pid;

    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    BLST_S_DICT_FIND(&p->pid_list, play_pid, pidChannel, pid_channel, link);
    if(play_pid==NULL) {
        BDBG_WRN(("NEXUS_Playpump_ClosePidChannel: %#lx can't find pid:%#lx", (unsigned long)p, (unsigned long)pidChannel));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_not_found;
    }
    BLST_S_DICT_REMOVE(&p->pid_list, play_pid, pidChannel, NEXUS_P_PlaypumpPidChannel, pid_channel, link);
    BDBG_ASSERT(play_pid);
    NEXUS_Playpump_P_ClosePid(p, play_pid);
    BKNI_Free(play_pid);
    return NEXUS_SUCCESS;

err_not_found:
    return rc;
}

void
NEXUS_Playpump_CloseAllPidChannels(NEXUS_PlaypumpHandle p)
{
    NEXUS_P_PlaypumpPidChannel *next_play_pid;
    BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
    for(next_play_pid=BLST_S_FIRST(&p->pid_list);next_play_pid;) {
        NEXUS_P_PlaypumpPidChannel *play_pid = next_play_pid;
        next_play_pid = BLST_S_NEXT(play_pid, link);
        NEXUS_P_HwPidChannel_CloseAll(play_pid->pid_channel); /* this may or may not delete next_play_pid */
    }
    return;
}

static NEXUS_Error NEXUS_Playpump_P_SetPause( NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;
    if (p->paused || !p->consumerStarted) {
        rc = BXPT_Playback_Pause(p->xpt_play);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        rc = BXPT_Playback_Resume(p->xpt_play);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

NEXUS_Error NEXUS_Playpump_SetPause( NEXUS_PlaypumpHandle p, bool paused )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (p->paused != paused) {
        p->paused = paused;
        rc = NEXUS_Playpump_P_SetPause(p);
        if (rc) {
            p->paused = !p->paused;
        }
    }
    return rc;
}

NEXUS_Error NEXUS_Playpump_SuspendPacing( NEXUS_PlaypumpHandle p, bool suspended )
{
    BERR_Code rc;
    rc = BXPT_Playback_ConfigPacing(p->xpt_play, suspended?BXPT_PacingControl_eStop:BXPT_PacingControl_eStart);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

static void NEXUS_Playpump_P_PacingErr_isr( void *playpump, int param2 )
{
    NEXUS_PlaypumpHandle p = (NEXUS_PlaypumpHandle)playpump;
    BSTD_UNUSED(param2);
    p->state.pacingTsRangeError++;
    return;
}

static void NEXUS_Playpump_P_InstallRangeErrIntHandler(NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;
    rc = BINT_CreateCallback(&p->pacingErrIntCallback, g_pCoreHandles->bint, BXPT_Playback_GetIntId(p->xpt_play, BXPT_PbInt_eTsRangeErr),
        NEXUS_Playpump_P_PacingErr_isr, ( void * ) p, 0 );
    if (!rc) {
        rc = BINT_EnableCallback(p->pacingErrIntCallback);
    }
    if (rc) {rc = BERR_TRACE(rc);}
}

static void NEXUS_Playpump_P_UninstallRangeErrIntHandler(NEXUS_PlaypumpHandle p)
{
    (void)BINT_DestroyCallback(p->pacingErrIntCallback);
}

static void NEXUS_Playpump_P_CCError_isr(void *context, int param)
{
    NEXUS_PlaypumpHandle p = context;
    BSTD_UNUSED(param);
    NEXUS_ParserBand_P_CountCcErrors_isr();
    NEXUS_IsrCallback_Fire_isr(p->ccErrorCallback);
}

static void NEXUS_Playpump_P_TeiError_isr(void *context, int param)
{
    NEXUS_PlaypumpHandle p = context;
    BSTD_UNUSED(param);
    NEXUS_IsrCallback_Fire_isr(p->teiErrorCallback);
}

static NEXUS_Error NEXUS_Playpump_P_SetInterrupts(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpSettings *pSettings)
{
    BERR_Code rc;

    /* only enable certain interrupts when their callbacks are desired. this helps typical system performance. */
    if (pSettings->ccError.callback) {
        if (!p->ccErrorInt) {
            BDBG_MSG(("create playpump %d cc callback", p->index));
#if NEXUS_PARSER_BAND_CC_CHECK
            rc = BINT_CreateCallback(&p->ccErrorInt, g_pCoreHandles->bint,
                BXPT_Playback_GetIntId(p->xpt_play, BCHP_XPT_PB0_INTR_PARSER_CONTINUITY_ERROR_SHIFT),
                NEXUS_Playpump_P_CCError_isr, p, 0);
#else
            rc = BINT_CreateCallback(&p->ccErrorInt, g_pCoreHandles->bint,
                                     BXPT_INT_ID_PBP_CONTINUITY_ERROR( p->index ),
                                     NEXUS_Playpump_P_CCError_isr, p, 0);
#endif
            if (rc) return BERR_TRACE(rc);
            rc = BINT_EnableCallback(p->ccErrorInt);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (p->ccErrorInt) {
        (void)BINT_DestroyCallback(p->ccErrorInt);
        p->ccErrorInt = NULL;
    }

    /* only enable certain interrupts when their callbacks are desired. this helps typical system performance. */
    if (pSettings->teiError.callback) {
        if (!p->teiErrorInt) {
            BDBG_MSG(("create playpump %d tei callback", p->index));
            rc = BINT_CreateCallback(&p->teiErrorInt, g_pCoreHandles->bint,
                BXPT_Playback_GetIntId(p->xpt_play, BXPT_PbInt_eTeiError),
                NEXUS_Playpump_P_TeiError_isr, p, 0);
            if (rc) return BERR_TRACE(rc);
            rc = BINT_EnableCallback(p->teiErrorInt);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (p->teiErrorInt) {
        (void)BINT_DestroyCallback(p->teiErrorInt);
        p->teiErrorInt = NULL;
    }

    NEXUS_IsrCallback_Set(p->ccErrorCallback, &pSettings->ccError);
    NEXUS_IsrCallback_Set(p->teiErrorCallback, &pSettings->teiError);

    return 0;
}

NEXUS_Error NEXUS_Playpump_GetAllPassPidChannelIndex(
    NEXUS_PlaypumpHandle playpump,
    unsigned *pHwPidChannel
    )
{
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    BDBG_ASSERT(pHwPidChannel);

    *pHwPidChannel = BXPT_GET_PLAYBACK_ALLPASS_CHNL( playpump->index );
    return 0;
}

void NEXUS_Playpump_IsTransportTypeSupported( NEXUS_PlaypumpHandle playpump, NEXUS_TransportType transportType, bool *pIsSupported )
{
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    switch (transportType) {
    case NEXUS_TransportType_eEs:
    case NEXUS_TransportType_eTs:
    case NEXUS_TransportType_eMpeg2Pes:
    case NEXUS_TransportType_eVob:
    case NEXUS_TransportType_eMpeg1Ps:
#if B_REFSW_DSS_SUPPORT
    case NEXUS_TransportType_eDssEs:
    case NEXUS_TransportType_eDssPes:
#endif
#if B_HAS_ASF
    case NEXUS_TransportType_eAsf:
#endif
#if B_HAS_AVI
    case NEXUS_TransportType_eAvi:
#endif
#if B_HAS_FLV
    case NEXUS_TransportType_eFlv:
#endif
#if B_HAS_RMFF
    case NEXUS_TransportType_eRmff:
#endif
    case NEXUS_TransportType_eWav:
    case NEXUS_TransportType_eMp4Fragment:
    case NEXUS_TransportType_eOgg:
    case NEXUS_TransportType_eFlac:
    case NEXUS_TransportType_eAiff:
    case NEXUS_TransportType_eApe:
    case NEXUS_TransportType_eAmr:
        *pIsSupported = true;
        break;
    default:
        /* includes eMp4 and eMkv */
        *pIsSupported = false;
        break;
    }
}

NEXUS_Error NEXUS_Playpump_GetMpodLtsid(NEXUS_PlaypumpHandle p,  unsigned *pLtsid)
{
     BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
     BDBG_ASSERT(pLtsid);
     BXPT_Playback_GetLTSID(p->xpt_play, pLtsid);
     return 0;
}

void NEXUS_Playpump_P_GetOpenSettings(NEXUS_PlaypumpHandle p, NEXUS_PlaypumpOpenSettings *pOpenSettings)
{
     BDBG_OBJECT_ASSERT(p, NEXUS_Playpump);
     *pOpenSettings = p->openSettings;
}
