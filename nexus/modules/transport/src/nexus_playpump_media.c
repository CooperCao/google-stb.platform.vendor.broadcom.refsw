/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bsink_playback.h"
#include "blst_squeue.h"
#include "blst_slist.h"

#if B_HAS_MEDIA
/* saving PES dara to file is controlled in the  BSEAV/lib/utils/bsink_playback.c */

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
BDBG_MODULE(nexus_playpump_media);


BDBG_OBJECT_ID(b_pump_demux_t);
/* Must be 10K for proper WMA audio flushing */
#define B_PUMP_DEMUX_FILLER_SIZE    10240

struct b_pump_demux_alloc_entry {
    BLST_S_ENTRY(b_pump_demux_alloc_entry) link;
    BMMA_Block_Handle block;
    void *addr;
};

struct b_pump_demux_alloc_iface {
    struct balloc_iface iface; /* must be first */
    BLST_S_HEAD(b_pump_demux_alloc_allocations, b_pump_demux_alloc_entry) blocks;
};

struct b_pump_demux {
    BDBG_OBJECT(b_pump_demux_t)
    NEXUS_PlaypumpHandle pump;
    struct bpvr_queue pending; /* entries extracted from the playpump main queue and fed into the pump_packtizer */
    batom_factory_t factory;
    batom_pipe_t pipe_demux; /* input to the filter */
    bsink_playback_t sink;
    struct {
        bool  feed_stall; /* true if feed stalled (i.e. due to lack to resources) */
        bool  flush_queued;  /* if received entry to flush filter (e.g. end of stream mark), actuall flush would happen after completing processing of all queued data */
        unsigned forced_clear_events; /* number calls to bmedia_filter_clear */
        size_t pes_bytes; /* number of bytes consumed by bsink_playback */
        size_t recycle_count; /* number of recycled playpump segments */
        batom_pipe_t pipe_in; /* pipe from the bmedia_filter */
    } pes_feed;
    bmedia_filter_t filter;
    struct {
        void *ptr;
        BMMA_Block_Handle block;
    } eos_filler;
    void *dcrypt_ctx;      /* drm context, may be used by asf and others */
    void *drm_ctx;
    b_pid_map video_map;
    b_pid_map audio_map;
    b_pid_map other_map;
    struct b_pump_demux_alloc_iface alloc_iface;
};

static void *
b_mma_alloc(balloc_iface_t alloc, size_t size)
{
    struct b_pump_demux_alloc_iface *iface = (void *)alloc;
    struct b_pump_demux_alloc_entry *entry;

    entry = BKNI_Malloc(sizeof(*entry));
    if(entry==NULL) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_malloc;}

    entry->block = BMMA_Alloc(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma, size, 0, NULL);
    if(entry->block==NULL) { (void)BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);goto err_bmma_alloc;}

    entry->addr = BMMA_Lock(entry->block);
    if(entry->addr == NULL) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_lock;}

    BLST_S_DICT_ADD(&iface->blocks, entry, b_pump_demux_alloc_entry, addr, link, err_duplicate);

    return entry->addr;

err_duplicate:
    BMMA_Unlock(entry->block, entry->addr);
err_lock:
    BMMA_Free(entry->block);
err_bmma_alloc:
    BKNI_Free(entry);
err_malloc:
    return NULL;
}

static void
b_mma_free(balloc_iface_t alloc, void *ptr)
{
    struct b_pump_demux_alloc_iface *iface = (void *)alloc;
    struct b_pump_demux_alloc_entry *entry;

    BLST_S_DICT_REMOVE(&iface->blocks, entry, ptr, b_pump_demux_alloc_entry, addr, link);
    if(entry==NULL) { (void)BERR_TRACE(NEXUS_UNKNOWN); return; }

    BMMA_Unlock(entry->block, entry->addr);
    BMMA_Free(entry->block);
    BKNI_Free(entry);
    return;
}

#define B_PUMP_DEMUX_FACTORY_ATOMS  128
#define B_PUMP_DEMUX_POOL_BLOCKS    64

static void
b_pump_reset_pes_feed(b_pump_demux_t demux)
{
    demux->pes_feed.pes_bytes = 0;
    demux->pes_feed.feed_stall = false;
    demux->pes_feed.flush_queued = false;
    demux->pes_feed.recycle_count = 0;
    demux->pes_feed.forced_clear_events = 0;
    return;
}

static void 
b_pump_demux_stream_error(void *cntx)
{
    b_pump_demux_t demux = cntx;
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    NEXUS_TaskCallback_Fire(demux->pump->errorCallback);
    return;
}


b_pump_demux_t
b_pump_demux_create(NEXUS_PlaypumpHandle pump)
{
    b_pump_demux_t demux;
    bmedia_filter_cfg filter_cfg;
    bsink_playback_cfg sink_cfg;
    BERR_Code rc;

    BDBG_MSG_TRACE(("b_pump_demux_create>:"));
    demux = BKNI_Malloc(sizeof(*demux));
    if(!demux) {
        BDBG_ERR(("b_pump_demux_init: can't allocate %u bytes", (unsigned)sizeof(*demux)));
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(demux, b_pump_demux_t);
    demux->pump = pump;
    demux->alloc_iface.iface.bmem_alloc  = b_mma_alloc;
    demux->alloc_iface.iface.bmem_free = b_mma_free;
    BLST_S_INIT(&demux->alloc_iface.blocks);


    b_pump_reset_pes_feed(demux);
    demux->pes_feed.pipe_in = NULL;
    BFIFO_INIT(&demux->pending, pump->item_mem, pump->openSettings.numDescriptors);
    b_pid_map_init(&demux->video_map, 0xE0);
    b_pid_map_init(&demux->audio_map, 0xC0);
    b_pid_map_init(&demux->other_map, 0xD0); /* Hijack  top 16 audio stream_id's  ITU-T Rec. H.222.0 (ISO/IEC 13818-1) Table 2-22 . Stream_id assignments */
    demux->factory = batom_factory_create(bkni_alloc, B_PUMP_DEMUX_FACTORY_ATOMS);
    if(!demux->factory) {
        goto err_factory;
    }
    bsink_playback_default_cfg(&sink_cfg);
    sink_cfg.feed = demux->pump->play_feed;
    sink_cfg.cache_flush = NEXUS_FlushCache;
    sink_cfg.addr_to_offset = NEXUS_AddrToOffset;
    demux->sink = bsink_playback_create(demux->factory, &sink_cfg);
    if(!demux->sink) {
        goto err_sink;
    }

    demux->pipe_demux = batom_pipe_create(demux->factory);
    if(!demux->pipe_demux) {
        goto err_pipe_demux;
    }
    demux->eos_filler.block = BMMA_Alloc(g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma, B_PUMP_DEMUX_FILLER_SIZE, 0, NULL);
    if(demux->eos_filler.block==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto err_eos_filler_block;
    }
    demux->eos_filler.ptr = BMMA_Lock(demux->eos_filler.block);
    if(demux->eos_filler.ptr==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_eos_filler_ptr;
    }

    bmedia_filter_default_cfg(&filter_cfg);
    filter_cfg.eos_data = demux->eos_filler.ptr;
    filter_cfg.eos_len = B_PUMP_DEMUX_FILLER_SIZE;
    filter_cfg.application_cnxt = demux;
    filter_cfg.stream_error = b_pump_demux_stream_error;

    BKNI_Memset((void *)filter_cfg.eos_data, 0, filter_cfg.eos_len);
    demux->filter = bmedia_filter_create(demux->factory, &demux->alloc_iface.iface, &filter_cfg);

    if(!demux->filter) {
        goto err_filter;
    }

    BDBG_MSG_TRACE(("b_pump_demux_create>: %#lx", (unsigned long)demux));
    return demux;

err_filter:
    BMMA_Unlock(demux->eos_filler.block, demux->eos_filler.ptr);
err_eos_filler_ptr:
    BMMA_Free(demux->eos_filler.block);
err_eos_filler_block:
    batom_pipe_destroy(demux->pipe_demux);
err_pipe_demux:
    bsink_playback_destroy(demux->sink);
err_sink:
    batom_factory_destroy(demux->factory);
err_factory:
    BKNI_Free(demux);
err_alloc:
    return NULL;
}

void
b_pump_demux_destroy(b_pump_demux_t demux)
{
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_destroy>: %#lx", (unsigned long)demux));
    bmedia_filter_destroy(demux->filter);
    bsink_playback_destroy(demux->sink);
    batom_pipe_destroy(demux->pipe_demux);
    batom_factory_destroy(demux->factory);
    BMMA_Unlock(demux->eos_filler.block, demux->eos_filler.ptr);
    BMMA_Free(demux->eos_filler.block);
    BDBG_OBJECT_DESTROY(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_destroy<: %#lx", (unsigned long)demux));
    BKNI_Free(demux);
    return;
}


void 
b_pump_demux_status(b_pump_demux_t demux, NEXUS_PlaypumpStatus *pStatus)
{
    batom_factory_stats factory_stats;
    bmedia_filter_status filter_status;


    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);

    BDBG_MSG(("demux_status: pending:%p:%u  pipe_in:%p pes_bytes:%u %s", (void *)BFIFO_READ(&demux->pending), BFIFO_READ_PEEK(&demux->pending)?BFIFO_READ(&demux->pending)->ref_cnt:0, (void *)(demux->pes_feed.pipe_in?batom_pipe_peek(demux->pes_feed.pipe_in):NULL), (unsigned)demux->pes_feed.pes_bytes, demux->pes_feed.feed_stall?"STALL":""));

    batom_factory_get_stats(demux->factory, &factory_stats);
    BDBG_MSG(("demux_status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u/%u arena:%u/%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.full_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.full_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));

    bmedia_filter_get_status(demux->filter, &filter_status);
    BDBG_MSG(("demux_status: offset:%u acc_length:%u obj_length:%u state:%s errors:%u,%u,%u", (unsigned)filter_status.offset, (unsigned)filter_status.acc_length, (unsigned)filter_status.obj_length, filter_status.state, filter_status.errors.sync_errors, filter_status.errors.resync_events, filter_status.errors.format_errors));

    pStatus->syncErrors += filter_status.errors.sync_errors;
    pStatus->resyncEvents += filter_status.errors.resync_events;
    pStatus->streamErrors += filter_status.errors.format_errors + demux->pes_feed.forced_clear_events;
    pStatus->mediaPtsType = filter_status.last_pts_valid?NEXUS_PtsType_eCoded:NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    pStatus->mediaPts = filter_status.last_pts;

    return;
}


void
b_pump_demux_stop(b_pump_demux_t demux)
{
    NEXUS_P_PlaypumpPidChannel *pid;

    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_stop>: %#lx", (unsigned long)demux));
    BDBG_ASSERT(demux->pump);

    for(pid=BLST_S_FIRST(&demux->pump->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        b_pump_demux_remove_pid(demux, pid);
    }
    bmedia_filter_stop(demux->filter);
    bsink_playback_stop(demux->sink);
    b_pump_reset_pes_feed(demux);
    demux->pes_feed.pipe_in = NULL;
    b_pump_demux_set_rate(demux, demux->pump->settings.playRate); /* reset rate on stop */
    BFIFO_INIT(&demux->pending, demux->pump->item_mem, demux->pump->openSettings.numDescriptors);
    BDBG_MSG_TRACE(("b_pump_demux_stop<: %#lx", (unsigned long)demux));
    return;
}

static void
b_pump_demux_advance_recycle(b_pump_demux_t demux)
{
    struct bpvr_queue_item *item;
    unsigned count;

    BDBG_MSG_TRACE(("b_pump_demux_advance_recycle>: %#lx", (unsigned long)demux));
    for(count=0;BFIFO_READ_PEEK(&demux->pending)>0;count++) {
        item = BFIFO_READ(&demux->pending);
        BDBG_MSG_TRACE(("b_pump_demux_advance_recycle: %#lx %#lx:%u", (unsigned long)demux, (unsigned long)item, item->ref_cnt));
        if (item->ref_cnt) { break; }
        BDBG_MSG_TRACE(("b_pump_demux_advance_recycle: %#lx %#lx recycling %u:%u %#lx:%#lx", (unsigned long)demux, (unsigned long)item, item->skip, item->desc.length, (unsigned long)BFIFO_READ(&demux->pump->fifo), (unsigned long)item->desc.addr - item->skip));
        if(!item->sg) {
        BDBG_ASSERT(BFIFO_READ(&demux->pump->fifo) == ((uint8_t *)item->desc.addr - item->skip));
        BFIFO_READ_COMMIT(&demux->pump->fifo, item->skip+item->desc.length);
        }
        BFIFO_READ_COMMIT(&demux->pending, 1);
        BFIFO_READ_COMMIT(&demux->pump->activeFifo, 1); /* recycle chunk */
        item->skip = 0;
        item->desc.length = 0;
        item->desc.addr = 0;
    }
    demux->pes_feed.recycle_count += count;
    BDBG_MSG_TRACE(("b_pump_demux_advance_recycle<: %#lx %u:%u", (unsigned long)demux, demux->pes_feed.recycle_count, count));
    return;
}

/* should be the same as in playback, basisally if buffer less than B_IO_BLOCK_LIMIT, then playback would wait for playpump to consume it, but playpump(media_filter) could wait for playback to feed more data in order to satisfy its needs */
#define B_IO_BLOCK_SIZE 4096
#define B_IO_BLOCK_LIMIT (4*B_IO_BLOCK_SIZE)

static bool /* this function return true if there any usefull empty space in the playpump FIFO */
b_pump_demux_check_buffer( b_pump_demux_t demux)
{
    size_t data_left;
    uint8_t *addr;
    NEXUS_PlaypumpHandle pump = demux->pump;
    if(BFIFO_WRITE_LEFT(&pump->activeFifo)==0) {
        return false;
    }
    data_left = BFIFO_WRITE_LEFT(&pump->fifo);
    if(data_left > B_IO_BLOCK_LIMIT) {
        return true;
    }
    addr = BFIFO_WRITE(&pump->fifo);
    if((addr+B_IO_BLOCK_LIMIT) >= ((uint8_t *)pump->buf + pump->openSettings.fifoSize)) { /* if data is close to wrap, then application would discard it (and there is might be more data after wrap */
        return true;
    }
    return false;
}

void
b_pump_demux_advance(b_pump_demux_t demux)
{
    BDBG_MSG_TRACE(("b_pump_demux_advance>: %#lx %s %s", (unsigned long)demux, demux->pes_feed.feed_stall?"stall":"", batom_pipe_peek(demux->pipe_demux)==NULL?"idle":""));
    /* 1. recycle packets in the playpump fifo */
    b_pump_demux_advance_recycle(demux);
    if(demux->pes_feed.feed_stall ) {
        /* 2. some data was recycled, push demux further */
        demux->pes_feed.feed_stall = !(bmedia_filter_feed(demux->filter, demux->pipe_demux));
    }
    /* 2. Check if filter was stalled */
    BDBG_MSG_TRACE(("b_pump_demux_advance: %#lx %u %u %u %u %u", (unsigned long)demux, demux->pes_feed.pes_bytes, BFIFO_READ_PEEK(&demux->pump->pendingFifo), BFIFO_WRITE_PEEK(&demux->pump->fifo), BFIFO_WRITE_LEFT(&demux->pump->activeFifo), batom_pipe_peek(demux->pes_feed.pipe_in)));
    if (demux->pes_feed.pes_bytes==0 && BFIFO_READ_PEEK(&demux->pump->pendingFifo)==0 && !b_pump_demux_check_buffer(demux) && batom_pipe_peek(demux->pes_feed.pipe_in)==NULL ) {
        /* if there is no queued PES bytes, no elements in the pending fifo, but application FIFO full, then clogged media filter (e.g. got a frame that is larger then  playback buffer) */
        BDBG_WRN(("b_pump_demux_advance: %#lx congestion detected in the media filter, clearing[%u]", (unsigned long)demux, demux->pes_feed.forced_clear_events));
        bmedia_filter_clear(demux->filter);
        demux->pes_feed.forced_clear_events ++;
        b_pump_demux_stream_error(demux);
    }
    /* 3. feed data to the playback hardware */
    b_pump_demux_advance_recycle(demux);
    demux->pes_feed.pes_bytes += bsink_playback_feed(demux->sink, demux->pes_feed.pipe_in);
    /* 4. verify if flush is queued */
    if(demux->pes_feed.flush_queued) {
        if(!demux->pes_feed.feed_stall && batom_pipe_peek(demux->pes_feed.pipe_in)==NULL && batom_pipe_peek(demux->pipe_demux)==NULL) {
            BDBG_MSG(("b_pump_demux_advance: %#lx loop flush", (unsigned long)demux)); /* player sends an empty entry if it's about to loop a stream */
            demux->pes_feed.flush_queued = false;
            bmedia_filter_flush(demux->filter);
            b_pump_demux_advance_recycle(demux);
        }
    }
    /* 5. do a callback to application */
    if (demux->pes_feed.recycle_count) {
        BDBG_MSG_TRACE(("b_pump_demux_advance_feed: %#lx read_callback %u", (unsigned long)demux, demux->pes_feed.recycle_count));
        demux->pes_feed.recycle_count = 0;
        b_playpump_p_do_read_callback(demux->pump);
    }
    BDBG_MSG_TRACE(("b_pump_demux_advance<: %#lx", (unsigned long)demux));
}

void
b_pump_demux_reclaim(b_pump_demux_t demux)
{
    bsink_playback_status sink_status;
    size_t pes_bytes;

    BDBG_MSG_TRACE(("b_pump_demux_reclaim>: %#lx", (unsigned long)demux));
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);

    if(!demux->pes_feed.pipe_in) {
        /* run-away event */
        return;
    }

    pes_bytes = bsink_playback_feed(demux->sink, demux->pes_feed.pipe_in);
    BDBG_MSG_TRACE(("b_pump_demux_reclaim: %#lx pes_bytes:%u(%u)", (unsigned long)demux, pes_bytes, demux->pes_feed.pes_bytes));
    demux->pes_feed.pes_bytes += pes_bytes;
    b_pump_demux_advance_recycle(demux);
    bsink_playback_get_status(demux->sink, &sink_status);
    if(sink_status.idle) {
        batom_pipe_t pipe = demux->pipe_demux;
        demux->pes_feed.pes_bytes = 0;
        /* we have played all queued up PES entries, try to push accumulated stream data */
        demux->pes_feed.feed_stall = !(bmedia_filter_feed(demux->filter, pipe));
        b_pump_demux_advance(demux);
        if(demux->pes_feed.pes_bytes==0) {
            unsigned i;
            for(i=0;i<16;i++) {
                if(batom_pipe_peek(pipe)==NULL) { 
                    /* no accumulated data, get new data from FIFO  */
                    b_play_next(demux->pump);
                    break;
                } else {
                    /* push accumulated data */
                    BDBG_MSG(("b_pump_demux_reclaim: %#lx feed accumulated data %u", (unsigned long)demux, i));
                    demux->pes_feed.feed_stall = !(bmedia_filter_feed(demux->filter, pipe));
                    b_pump_demux_advance(demux);
                    if(demux->pes_feed.pes_bytes>0) {
                        /* exit if constructed PES data */
                        break;
                    }
                }
            }
        }
    }
    BDBG_MSG_TRACE(("b_pump_demux_reclaim<: %#lx %s pes_bytes:%u", (unsigned long)demux, sink_status.idle?"IDLE":"", demux->pes_feed.pes_bytes));
}


void
b_pump_demux_enqueue(b_pump_demux_t demux, struct bpvr_queue_item *item)
{
    BDBG_MSG_TRACE(("b_pump_demux_enqueue>: %#lx %#lx (%#lx:%u)", (unsigned long)demux, (unsigned long)item));
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    if (item->desc.length==0 && item->skip==0) {
        BDBG_MSG(("b_pump_demux_enqueue: %#lx  loop", (unsigned long)demux)); /* player sends an empty entry if it's about to loop a stream */
        demux->pes_feed.flush_queued = true;
    }
    BDBG_ASSERT(BFIFO_WRITE(&demux->pending)==item);
    BFIFO_WRITE_COMMIT(&demux->pending, 1);
    BDBG_MSG_TRACE(("b_pump_demux_enqueue<: %#lx %#lx (%#lx:%u)", (unsigned long)demux, (unsigned long)item));
    return;
}

static void
b_pump_demux_atom_free(batom_t atom, void *user)
{
    struct bpvr_queue_item *item = *(struct bpvr_queue_item **)user;
    BDBG_MSG_TRACE(("b_pump_demux_atom_free>: %#lx %#lx:%u", (unsigned long)atom, (unsigned long)item, item->ref_cnt));
    BSTD_UNUSED(atom);
    BDBG_ASSERT(item->ref_cnt>0);
    item->ref_cnt--;
    BDBG_MSG_TRACE(("b_pump_demux_atom_free>: %#lx %#lx:%u", (unsigned long)atom, (unsigned long)item, item->ref_cnt));
    return;
}

static const batom_user b_pump_demux_atom = {
    b_pump_demux_atom_free,
    sizeof(struct bpvr_queue_item **)
};

bool
b_pump_demux_feed(b_pump_demux_t demux, struct bpvr_queue_item *item, const void *data, size_t len)
{
    batom_t atom;
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_feed>: %#lx %#lx (%#lx:%u) %02X %02X %02X %02X", (unsigned long)demux, (unsigned long)item, (unsigned long)data, len, (unsigned)((uint8_t*)data)[0], (unsigned)((uint8_t*)data)[1], (unsigned)((uint8_t*)data)[2], (unsigned)((uint8_t*)data)[3]));

    atom = batom_from_range(demux->factory, data, len, &b_pump_demux_atom, &item);
    if(atom) {
        batom_pipe_t pipe = demux->pipe_demux;

        item->ref_cnt++;
        demux->pes_feed.pes_bytes = 0;
        batom_pipe_push(pipe, atom);
        demux->pes_feed.feed_stall = !(bmedia_filter_feed(demux->filter, pipe));
        b_pump_demux_advance(demux);
    }
    BDBG_MSG_TRACE(("b_pump_demux_feed<: %#lx %#lx (%#lx:%u)", (unsigned long)demux, (unsigned long)item, (unsigned long)data, len));
    return true;
}

void
b_pump_demux_flush(b_pump_demux_t demux)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_flush>: %#lx", (unsigned long)demux));
    b_pump_reset_pes_feed(demux);
    bmedia_filter_flush(demux->filter);
    batom_pipe_flush(demux->pipe_demux);
    bsink_playback_stop(demux->sink);
    {
        unsigned count;
        unsigned i;
        struct bpvr_queue_item *item;

        bmedia_filter_flush(demux->filter);
        if (BFIFO_READ_PEEK(&demux->pump->pendingFifo)) {
            item = BFIFO_READ(&demux->pump->pendingFifo);
            if(item->ref_cnt) {
                BDBG_ERR(("b_pump_demux_flush: %p lost buffer [IN-FLIGHT] %p:%u", (void *)demux, (void *)item, item->ref_cnt));
            }
        }
        count = BFIFO_READ_LEFT(&demux->pending);
        item = BFIFO_READ(&demux->pending);
        for(i=0;i<count;i++) {
            if(item->ref_cnt) {
                BDBG_ERR(("b_pump_demux_flush: %p lost buffer [ENQUEUED] %p:%u", (void *)demux, (void *)item, item->ref_cnt));
            }
            item++;
            if(item>=demux->pending.bf_last) {
                item = demux->pending.bf_base;
            }
        }
    }

    b_pump_demux_advance_recycle(demux);
    rc = bsink_playback_start(demux->sink);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    BFIFO_INIT(&demux->pending, demux->pump->item_mem, demux->pump->openSettings.numDescriptors);
    BDBG_MSG_TRACE(("b_pump_demux_flush<: %#lx", (unsigned long)demux));
    return;
}

NEXUS_Error
b_pump_demux_open_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid, unsigned stream_id, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings)
{
    uint16_t pes_pid;
    NEXUS_Error rc;
    b_pid_map *map;

    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pid);
    switch(pSettings->pidType) {
    case NEXUS_PidType_eAudio:
        map = &demux->audio_map;
        break;
    case NEXUS_PidType_eVideo:
        map = &demux->video_map;
        break;
    default:
        map = &demux->other_map;
        break;
    }
    pes_pid = b_pid_map_alloc(map);
    if(pes_pid==0) {rc=BERR_TRACE(BERR_NOT_SUPPORTED);goto err_pes_pid;}
    BDBG_MSG(("b_pump_demux_open_pid:%#lx mapping media stream:%u to PES ID:%#x", (unsigned long)demux, (unsigned)stream_id, (unsigned)pes_pid));

    pid->media_stream = NULL;
    pid->media_pid = pes_pid;
    pid->stream_id = stream_id;
    return NEXUS_SUCCESS;

err_pes_pid:
    return  rc;
}

NEXUS_Error
b_pump_demux_add_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid)
{
    NEXUS_Error rc;
    bmedia_stream_cfg stream_cfg;

    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_ASSERT(pid);
    BDBG_MSG(("b_pump_demux_add_pid: %#lx mapping stream %u to pid %#x", (unsigned long)demux, pid->stream_id, pid->media_pid));
    bmedia_stream_default_cfg(&stream_cfg);
    stream_cfg.reorder_timestamps = pid->settings.allowTimestampReordering;
    pid->media_stream = bmedia_filter_stream_open(demux->filter, pid->stream_id, pid->media_pid, &stream_cfg);
    if(!pid->media_stream) {rc=BERR_TRACE(BERR_NOT_SUPPORTED);goto err_media_stream;}
    return NEXUS_SUCCESS;

err_media_stream:
    return rc;
}

void
b_pump_demux_close_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid)
{
    unsigned media_pid;
    b_pid_map *map;
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_ASSERT(pid);

    media_pid = pid->media_pid;
    if(media_pid>=demux->video_map.base_pid) {
        map = &demux->video_map;
    } else if (media_pid>=demux->other_map.base_pid) {
        map = &demux->other_map;
    } else {
        map = &demux->audio_map;
    }
    b_pid_map_free(map, media_pid);
    pid->media_pid = 0;
    pid->stream_id = 0;
    return;
}

void
b_pump_demux_remove_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid)
{
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_ASSERT(pid);

    if(!pid->media_stream) {
        return;
    }
    bmedia_filter_stream_close(demux->filter, pid->media_stream);
    pid->media_stream = NULL;
    return;
}

NEXUS_Error
b_pump_demux_set_stream_type(b_pump_demux_t demux, NEXUS_TransportType transportType, bool *supported)
{
    bstream_mpeg_type mediaType;
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);

    BDBG_ASSERT(supported);
    *supported = false;
    if(demux->pes_feed.pipe_in) {
        bmedia_filter_stop(demux->filter);
        demux->pes_feed.pipe_in = NULL;
    }
    switch(transportType) {
    case NEXUS_TransportType_eMkv:
    case NEXUS_TransportType_eMp4:
    case NEXUS_TransportType_eApe:
        /* player does conversion of those to PES  */
        mediaType = bstream_mpeg_type_pes;
        break;
#if !B_HAS_NATIVE_MPEG1
    case NEXUS_TransportType_eMpeg1Ps:
        mediaType = bstream_mpeg_type_mpeg1;
        break;
#endif
    case NEXUS_TransportType_eAvi:
        mediaType = bstream_mpeg_type_avi;
        break;
    case NEXUS_TransportType_eAsf:
        mediaType = bstream_mpeg_type_asf;
        break;
    case NEXUS_TransportType_eWav:
        mediaType = bstream_mpeg_type_wav;
        break;
    case NEXUS_TransportType_eAiff:
        mediaType = bstream_mpeg_type_aiff;
        break;
    case NEXUS_TransportType_eMp4Fragment:
        mediaType = bstream_mpeg_type_mp4_fragment;
        break;
    case NEXUS_TransportType_eRmff:
        mediaType = bstream_mpeg_type_rmff;
        break;
    case NEXUS_TransportType_eFlv:
        mediaType = bstream_mpeg_type_flv;
        break;
    case NEXUS_TransportType_eOgg:
        mediaType = bstream_mpeg_type_ogg;
        break;
    case NEXUS_TransportType_eFlac:
        mediaType = bstream_mpeg_type_flac;
        break;
    case NEXUS_TransportType_eAmr:
        mediaType = bstream_mpeg_type_amr;
        break;
    default:
        return NEXUS_SUCCESS;
    }
    demux->pes_feed.pipe_in = bmedia_filter_start(demux->filter, mediaType);
    if(!demux->pes_feed.pipe_in) { return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    *supported = true;
    return NEXUS_SUCCESS;
}


NEXUS_Error
b_pump_demux_start(b_pump_demux_t demux)
{
    NEXUS_Error rc = 0;
    NEXUS_P_PlaypumpPidChannel *pid;
    bool supported;

    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_MSG_TRACE(("b_pump_demux_start>: %#lx", (unsigned long)demux));

#if NEXUS_ENCRYPTED_DVR_WITH_M2M
    if(demux->pump->settings.securityContext==NULL) {
       demux->dcrypt_ctx = NULL;
    }
#endif

    if(!demux->pes_feed.pipe_in) {
        rc = b_pump_demux_set_stream_type(demux, demux->pump->settings.transportType, &supported);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_stream;}
        BDBG_ASSERT(supported);
    }
    for(pid=BLST_S_FIRST(&demux->pump->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        BDBG_ASSERT(pid->media_stream == NULL);
        rc = b_pump_demux_add_pid(demux, pid);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_pid;}
    }
    rc = bsink_playback_start(demux->sink);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_sink_start; }

    BDBG_MSG_TRACE(("b_pump_demux_start<: %#lx", (unsigned long)demux));
    return NEXUS_SUCCESS;

err_sink_start:
err_pid:
    for(pid=BLST_S_FIRST(&demux->pump->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if(pid->media_stream) {
            b_pump_demux_remove_pid(demux, pid);
        }
    }
err_stream:
    return rc;
}

bool
b_pump_demux_is_empty(b_pump_demux_t demux)
{
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);

    return demux->pes_feed.pes_bytes == 0;
}


void
b_pump_demux_descriptor(b_pump_demux_t demux, const NEXUS_PlaypumpSegment *desc)
{
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_ASSERT(desc);
    bmedia_filter_seek(demux->filter, desc->offset);
    if(desc->timestamp!=0) {
        unsigned i;
        for(i=0; i<sizeof(desc->timestamp_delta)/sizeof(desc->timestamp_delta[0]); i++){
            if(desc->timestamp_delta[i].stream_id) {
                bmedia_filter_set_offset(demux->filter, desc->timestamp + desc->timestamp_delta[i].timestamp_delta, desc->timestamp_delta[i].stream_id);
            }
        }
    }
    return;
}

void
b_pump_demux_set_rate(b_pump_demux_t demux, int rate)
{
    BDBG_OBJECT_ASSERT(demux, b_pump_demux_t);
    BDBG_CASSERT( (256 / (NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE) == (256 * BMEDIA_TIME_SCALE_BASE)/NEXUS_NORMAL_PLAY_SPEED));
    bmedia_filter_set_timescale(demux->filter, rate/(NEXUS_NORMAL_PLAY_SPEED/BMEDIA_TIME_SCALE_BASE));
    bmedia_filter_set_keyframeonly(demux->filter, rate!=NEXUS_NORMAL_PLAY_SPEED);
}

bool 
b_pump_demux_is_congested(b_pump_demux_t demux)
{
    return demux->pes_feed.flush_queued;
}

#endif /* B_HAS_MEDIA */

