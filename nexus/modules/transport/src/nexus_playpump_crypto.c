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
 **************************************************************************/
#include "nexus_transport_module.h"

#include "nexus_playpump_impl.h"
#include "bsink_playback.h"
#include "blst_squeue.h"
#include "biobits.h"

/* TODO: refactor NEXUS_ENCRYPTED_DVR_WITH_M2M to only be required for NEXUS_Dma code, not external crypto */
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
#include "nexus_dma.h"
#include "priv/nexus_dma_priv.h"

BDBG_MODULE(nexus_playpump_crypto);

/* NEXUS_PLAYPUMP_EXTERNAL_CRYPTO is defined in nexus/extensions/transport/external_crypto/external_crypto.inc */
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
#include "nexus_playpump_external_crypto.h"
#endif

#define BDBG_MSG_TRACE(x)  /* BDBG_MSG(x) */

#define B_PUMP_CRYPTO_FACTORY_ATOMS  128
#define B_PUMP_CRYPT_MAX_TS_SEGMENTS  8

BDBG_OBJECT_ID(b_pump_crypto_t);

struct b_pump_crypto {
    BDBG_OBJECT(b_pump_crypto_t)
    NEXUS_PlaypumpHandle pump;
    struct bpvr_queue pending; /* entries extracted from the playpump main queue and fed into the pump_packtizer */
    batom_factory_t factory;
    batom_pipe_t parser_pipe; /* input to the ts parser, preserving block boundaries  */
    batom_accum_t parser_acc; /* input to the ts parser, concatenated blocks */
    bsink_playback_t sink;
    struct {
        bool  feed_stall; /* true if feed stalled (i.e. due to lack to resources) */
        size_t ts_bytes; /* number of bytes consumed by bsink_playback */
        size_t recycle_count; /* number of recycled playpump segments */
        batom_pipe_t pipe_clear; /* pipe from the ts parser */
    } ts_feed;
    struct {
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
        void *job; /* custom handle */
        /* Note that I've left the atom/busy/event/callback members in case we want to extend the external crypto to be async. */
#else
        NEXUS_DmaJobHandle job; /* job is only created when crypto started */
        NEXUS_DmaJobBlockSettings blocks[B_PUMP_CRYPT_MAX_TS_SEGMENTS];
#endif
        batom_t atom; /* atom currently handled in DMA engine, NULL if DMA is idle */
        bool busy; /* set to true if DMA is currently pending, on flush atom would go to NULL, but busy would still be set (since we can't cancel DMA), on stop we wait until DMA is completed */
        BKNI_EventHandle event;
        NEXUS_EventCallbackHandle callback;
        size_t packetSize;
        int packetOffset;
    } dma;
};

static void
b_pump_crypto_advance_recycle(b_pump_crypto_t crypto);

static void
b_pump_crypto_atom_free(batom_t atom, void *user)
{
    struct bpvr_queue_item *item = *(struct bpvr_queue_item **)user;
    BDBG_MSG_TRACE(("b_pump_crypto_atom_free>: %#lx %#lx", (unsigned long)atom, (unsigned long)item));
    BSTD_UNUSED(atom);
    BDBG_ASSERT(item->ref_cnt>0);
    item->ref_cnt--;
    BDBG_MSG_TRACE(("b_pump_crypto_atom_free>: %#lx %#lx:%u", (unsigned long)atom, (unsigned long)item, item->ref_cnt));
    return;
}

static const batom_user b_pump_crypto_atom = {
    b_pump_crypto_atom_free,
    sizeof(struct bpvr_queue_item **)
};

static void
b_pump_reset_ts_feed(b_pump_crypto_t crypto)
{
    crypto->ts_feed.ts_bytes = 0;
    crypto->ts_feed.feed_stall = false;
    crypto->ts_feed.recycle_count = 0;
    return;
}

void
b_pump_crypto_enqueue(b_pump_crypto_t crypto, struct bpvr_queue_item *item)
{
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BSTD_UNUSED(item);
    BDBG_MSG_TRACE(("b_pump_crypto_enqueue:%#lx queuing up %#lx", (unsigned long)crypto, (unsigned long)item));
    if (item->skip == 0 && item->desc.length == 0) {
        batom_accum_clear(crypto->parser_acc);
        batom_pipe_flush(crypto->parser_pipe);
        b_pump_crypto_advance_recycle(crypto);
    }
    BDBG_ASSERT(BFIFO_WRITE(&crypto->pending)==item);
    BFIFO_WRITE_COMMIT(&crypto->pending, 1);
    return;
}


#define B_MPEG2_TS_SYNC 0x47

static bool
b_pump_crypto_process_mpeg2ts(b_pump_crypto_t crypto)
{
    int byte;
    unsigned len;
    batom_accum_t acc;
    batom_cursor cursor;
    batom_cursor packet_start;
    const batom_vec *vec;

    BDBG_MSG_TRACE(("b_pump_crypto_process_mpeg2ts>: %#lx", (unsigned long)crypto));
    acc = crypto->parser_acc;
    for(;;) {
        batom_t atom = batom_pipe_pop(crypto->parser_pipe);
        if(!atom) { break; }

        len = batom_len(atom);
        BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx processing atom %#lx:%u", (unsigned long)crypto, (unsigned long)atom, len));

        if(len==crypto->dma.packetSize && batom_accum_len(acc)==0) {
            const batom_vec *vec = batom_get_vec(atom, 0);
            BDBG_ASSERT(vec);
            if(vec->len > (4+crypto->dma.packetOffset) &&
              ((uint8_t *)vec->base)[0 + crypto->dma.packetOffset] == B_MPEG2_TS_SYNC &&
            /* transport_scrambling_control */ B_GET_BITS(((uint8_t *)vec->base)[3 + crypto->dma.packetOffset],7,6) == 0x03) { /* there is an injected packet */

                ((uint8_t *)vec->base)[3 + crypto->dma.packetOffset] = B_GET_BITS(((uint8_t *)vec->base)[3 + crypto->dma.packetOffset],5,0) | B_SET_BITS( transport_scrambling_control, 0, 7, 6); /* clear adaptation field */
                ((batom_vec *)vec)->dirty = true;
                NEXUS_FlushCache(vec->base, vec->len);
                batom_pipe_push(crypto->ts_feed.pipe_clear, atom);
                batom_accum_clear(acc); /* clear any accumulated data */
                BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx meta packet at %#lx, bypassing decryption", (unsigned long)crypto, (unsigned long)vec->base));
                continue;
            }
        }
        batom_accum_add_atom(acc, atom);
        batom_release(atom);
        batom_cursor_from_accum(&cursor, acc);
        for(len=0;;len++) {
            BATOM_CLONE(&packet_start, &cursor);
            BATOM_NEXT(byte, &cursor);
            if (crypto->dma.packetOffset) {
                batom_cursor peek_ahead;
                int remaining;
                BATOM_CLONE(&peek_ahead, &cursor);
                remaining = BATOM_RESERVE(&peek_ahead,(unsigned)crypto->dma.packetOffset - 1);
                if (remaining < crypto->dma.packetOffset - 1) {
                    len += crypto->dma.packetOffset - 1;
                    break;
                }
                BATOM_SKIP(&peek_ahead, crypto->dma.packetOffset - 1);
                BATOM_NEXT(byte, &peek_ahead);
            }
            if(byte==B_MPEG2_TS_SYNC) {
                break;
            } else if(byte==BATOM_EOF) {
                batom_accum_trim(acc, &packet_start);
                goto no_sync;
            }
        }
        if(len>0) { /* remove all data prior to 0x47 */
            batom_accum_trim(acc, &packet_start);
            batom_cursor_from_accum(&packet_start, acc);
        }
        BATOM_CLONE(&cursor, &packet_start);
        len = batom_cursor_size(&cursor);
        len -= len%crypto->dma.packetSize; /* get complete packets */
        if(len<=0) {
            continue;
        }
        batom_cursor_skip(&cursor, len);
        atom = batom_accum_extract(acc, &packet_start, &cursor, NULL, NULL);
        if(atom) {
            unsigned nvecs;
            unsigned i;
            NEXUS_Error rc = 0;

            batom_accum_trim(acc, &cursor);
            vec = batom_get_vectors(atom,&nvecs);
            BDBG_ASSERT(nvecs>0);
            if(nvecs>B_PUMP_CRYPT_MAX_TS_SEGMENTS) {
                BDBG_WRN(("b_pump_crypto_process_mpeg2ts:%#lx reached max of transport segments %u(%u)", (unsigned long)crypto, nvecs, B_PUMP_CRYPT_MAX_TS_SEGMENTS));
                nvecs = B_PUMP_CRYPT_MAX_TS_SEGMENTS;
            }
            BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx decrypting %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
            BSTD_UNUSED(i);
            /* The external crypto interface is currently synchronous and in-place.
               Every call to NEXUS_PlaypumpExternalCrypto_Decrypt should contain whole 188 byte packets.
               However, some of those packets may span multiple batom_vec entries any may not be contiguous.
               The external library may need to create a temporary packet buffer if it can only decrypt in contiguous packets. */
            rc = NEXUS_PlaypumpExternalCrypto_Decrypt(crypto->dma.job, vec, nvecs);
            if(rc==NEXUS_SUCCESS) {
                BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx completed %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
                batom_pipe_push(crypto->ts_feed.pipe_clear, atom);
            }
            else {
                BDBG_WRN(("b_pump_crypto_process_mpeg2ts:%#lx can't handle %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
                rc = BERR_TRACE(rc);
                batom_release(atom);
            }
#else
            for(i=0;i<nvecs;i++) {		
                crypto->dma.blocks[i].pSrcAddr = vec[i].base;
                crypto->dma.blocks[i].pDestAddr = vec[i].base;
                crypto->dma.blocks[i].blockSize = vec[i].len;
                crypto->dma.blocks[i].scatterGatherCryptoEnd = false;	
            }
            crypto->dma.blocks[nvecs-1].scatterGatherCryptoEnd = true;
#if (!NEXUS_HAS_XPT_DMA)
            NEXUS_Module_Lock(g_NEXUS_Transport_P_State.moduleSettings.dma);
            rc = NEXUS_DmaJob_ProcessBlocks_priv(crypto->dma.job, crypto->dma.blocks, nvecs, crypto->dma.event);
            NEXUS_Module_Unlock(g_NEXUS_Transport_P_State.moduleSettings.dma);
#else
            rc = NEXUS_DmaJob_ProcessBlocks_priv(crypto->dma.job, crypto->dma.blocks, nvecs, crypto->dma.event);
#endif
            if(rc==NEXUS_SUCCESS) {
                BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx completed %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
                batom_pipe_push(crypto->ts_feed.pipe_clear, atom);
            } else if(rc==NEXUS_DMA_QUEUED) {
                BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx queued %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
                crypto->dma.atom = atom;
                crypto->dma.busy = true;
                break;
            } else {
                BDBG_WRN(("b_pump_crypto_process_mpeg2ts:%#lx can't handle %#lx:%u", (unsigned long)crypto, (unsigned long)vec[0].base, len));
                rc = BERR_TRACE(rc);
                batom_release(atom);
            }
#endif
        } else {
            BDBG_MSG_TRACE(("b_pump_crypto_process_mpeg2ts<: %#lx, no atom", (unsigned long)crypto));
            return false;
        }
no_sync:
        ;
    }
    BDBG_MSG_TRACE(("b_pump_crypto_process_mpeg2ts<: %#lx", (unsigned long)crypto));
    return true;
}


static bool
b_pump_crypto_process(b_pump_crypto_t crypto)
{
    if(!crypto->dma.busy) {
        BDBG_ASSERT(!crypto->dma.atom); /* if DMA is idle, we shouldn't have any atoms queued */
        return b_pump_crypto_process_mpeg2ts(crypto);
    }
    return false;
}

static void
b_pump_crypto_advance_recycle(b_pump_crypto_t crypto)
{
    struct bpvr_queue_item *item;
    unsigned count;

    BDBG_MSG_TRACE(("b_pump_crypto_advance_recycle>: %#lx", (unsigned long)crypto));
    for(count=0;BFIFO_READ_PEEK(&crypto->pending)>0;count++) {
        item = BFIFO_READ(&crypto->pending);
        BDBG_MSG_TRACE(("b_pump_crypto_advance_recycle: %#lx %#lx:%u", (unsigned long)crypto, (unsigned long)item, item->ref_cnt));
        if (item->ref_cnt) { break; }
        BDBG_MSG_TRACE(("b_pump_crypto_advance_recycle: %#lx %#lx recycling %u:%u %#lx:%#lx", (unsigned long)crypto, (unsigned long)item, item->skip, item->desc.length, (unsigned long)BFIFO_READ(&crypto->pump->fifo), (unsigned long)item->desc.addr - item->skip));
        if (!item->sg) {
            /* in S/G mode data fifo isn't used */
            BDBG_ASSERT(BFIFO_READ(&crypto->pump->fifo) == ((uint8_t *)item->desc.addr - item->skip));
            BFIFO_READ_COMMIT(&crypto->pump->fifo, item->skip+item->desc.length);
        }
        BFIFO_READ_COMMIT(&crypto->pending, 1);
        BFIFO_READ_COMMIT(&crypto->pump->activeFifo, 1); /* recycle chunk */
        item->skip = 0;
        item->desc.length = 0;
        item->desc.addr = 0;
    }
    crypto->ts_feed.recycle_count += count;
    BDBG_MSG_TRACE(("b_pump_crypto_advance_recycle<: %#lx %u:%u", (unsigned long)crypto, crypto->ts_feed.recycle_count, count));
    return;
}

static void
b_pump_crypto_advance(b_pump_crypto_t crypto)
{
    BDBG_MSG_TRACE(("b_pump_crypto_advance>: %#lx", (unsigned long)crypto));
    /* 1. recycle packets in the playpump fifo */
    b_pump_crypto_advance_recycle(crypto);
    if(crypto->ts_feed.feed_stall) {
        /* 2. some data was recycled, push crypto further */
        crypto->ts_feed.feed_stall = !b_pump_crypto_process(crypto);
    }
    /* 3. feed data to the playback hardware */
    crypto->ts_feed.ts_bytes += bsink_playback_feed(crypto->sink, crypto->ts_feed.pipe_clear);
    /* 4. do a callback to application */
    if (crypto->ts_feed.recycle_count) {
        BDBG_MSG_TRACE(("b_pump_crypto_advance_feed: %#lx read_callback %u", (unsigned long)crypto, crypto->ts_feed.recycle_count));
        crypto->ts_feed.recycle_count = 0;
        b_playpump_p_do_read_callback(crypto->pump);
    }
    BDBG_MSG_TRACE(("b_pump_crypto_advance<: %#lx", (unsigned long)crypto));
}


bool
b_pump_crypto_feed(b_pump_crypto_t crypto, struct bpvr_queue_item *item, const void *data, size_t len)
{
    batom_t atom;
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BDBG_MSG_TRACE(("b_pump_crypto_feed>: %#lx %#lx (%#lx:%u) %02X %02X %02X %02X", (unsigned long)crypto, (unsigned long)item, (unsigned long)data, len, (unsigned)((uint8_t*)data)[0], (unsigned)((uint8_t*)data)[1], (unsigned)((uint8_t*)data)[2], (unsigned)((uint8_t*)data)[3]));
    atom = batom_from_range(crypto->factory, data, len, &b_pump_crypto_atom, &item);
    if(atom) {
        item->ref_cnt++;
        crypto->ts_feed.ts_bytes = 0;
        batom_pipe_push(crypto->parser_pipe, atom);
        BDBG_MSG_TRACE(("b_pump_crypto_feed:%#lx making atom %#lx:%#lx(%#lx:%u)", (unsigned long)crypto, (unsigned long)item, (unsigned long)atom, (unsigned long)data, (unsigned)len));
        crypto->ts_feed.feed_stall = !b_pump_crypto_process(crypto);
        b_pump_crypto_advance(crypto);
    }
    BDBG_MSG_TRACE(("b_pump_crypto_feed<: %#lx %#lx (%#lx:%u)", (unsigned long)crypto, (unsigned long)item, (unsigned long)data, len));
    return true;
}

static void
b_pump_crypto_dma_callback(void *context)
{
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
    BSTD_UNUSED(context);
    /* This is only needed if the external library can be async */
#else
    b_pump_crypto_t crypto = context;
    NEXUS_Error rc;
    NEXUS_DmaJobStatus jobStatus;
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);

    if(!crypto->dma.job) { /* it's possible that we got stale event, so we need to verify state of DMA before going further */
        return;
    }
    rc = NEXUS_DmaJob_GetStatus(crypto->dma.job, &jobStatus);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);return;}
    if(jobStatus.currentState != NEXUS_DmaJobState_eComplete) {
        BDBG_WRN(("b_pump_crypto_dma_callback: %#lx event but not ready", (unsigned long)context));
        return;
    }
    crypto->dma.busy = false;
    if(crypto->dma.atom) {
        BDBG_MSG_FLOW(("b_pump_crypto_process_mpeg2ts:%#lx completed queued %#lx(%#lx:%u)", (unsigned long)crypto, (unsigned long)crypto->dma.atom, (unsigned long)crypto->dma.blocks[0].pSrcAddr, crypto->dma.blocks[0].blockSize));
        batom_pipe_push(crypto->ts_feed.pipe_clear, crypto->dma.atom);
        crypto->dma.atom = NULL;
        b_pump_crypto_advance(crypto);
    }
    return;
#endif
}

static  void
b_pump_crypto_no_cache_flush(const void *addr, size_t len)
{
    BSTD_UNUSED(addr);
    BSTD_UNUSED(len);
    return;
}

b_pump_crypto_t
b_pump_crypto_create(NEXUS_PlaypumpHandle pump)
{
    BERR_Code rc;
    b_pump_crypto_t crypto;
    bsink_playback_cfg sink_cfg;

    BDBG_MSG_TRACE(("b_pump_crypto_create>:"));
    crypto = BKNI_Malloc(sizeof(*crypto));
    if(!crypto) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }

    BDBG_OBJECT_INIT(crypto, b_pump_crypto_t);
    crypto->pump = pump;
    crypto->dma.job = NULL;
    crypto->dma.atom = NULL;
    crypto->dma.busy = false;

    b_pump_reset_ts_feed(crypto);
    BFIFO_INIT(&crypto->pending, pump->item_mem, pump->openSettings.numDescriptors);
    crypto->ts_feed.pipe_clear = NULL;
    crypto->factory = batom_factory_create(bkni_alloc, B_PUMP_CRYPTO_FACTORY_ATOMS);
    if(!crypto->factory) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_factory; }

    bsink_playback_default_cfg(&sink_cfg);
    sink_cfg.feed = crypto->pump->play_feed;
    sink_cfg.cache_flush = b_pump_crypto_no_cache_flush; /* since data came straigh from DMA there is no need to flush a cache */
    sink_cfg.addr_to_offset = NEXUS_AddrToOffset;
    crypto->sink = bsink_playback_create(crypto->factory, &sink_cfg);
    if(!crypto->sink) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_sink; }

    crypto->parser_pipe = batom_pipe_create(crypto->factory);
    if(!crypto->parser_pipe) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_parser_pipe; }

    crypto->ts_feed.pipe_clear = batom_pipe_create(crypto->factory);
    if(!crypto->ts_feed.pipe_clear) { rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_pipe_clear; }

    crypto->parser_acc = batom_accum_create(crypto->factory);
    if(!crypto->parser_acc) { rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_parser_acc; }

    rc = BKNI_CreateEvent(&crypto->dma.event);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_dma_event; }

    crypto->dma.callback = NEXUS_RegisterEvent(crypto->dma.event, b_pump_crypto_dma_callback, crypto);
    if(!crypto->dma.callback) { rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_dma_callback; }

    BDBG_MSG_TRACE(("b_pump_crypto_create>: %#lx", (unsigned long)crypto));
    return crypto;

err_dma_callback:
    BKNI_DestroyEvent(crypto->dma.event);
err_dma_event:
    batom_accum_destroy(crypto->parser_acc);
err_parser_acc:
    batom_pipe_destroy(crypto->ts_feed.pipe_clear);
err_pipe_clear:
    batom_pipe_destroy(crypto->parser_pipe);
err_parser_pipe:
    bsink_playback_destroy(crypto->sink);
err_sink:
    batom_factory_destroy(crypto->factory);
err_factory:
    BKNI_Free(crypto);
err_alloc:
    return NULL;
}

void
b_pump_crypto_destroy(b_pump_crypto_t crypto)
{
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BDBG_MSG_TRACE(("b_pump_crypto_destroy>: %#lx", (unsigned long)crypto));
    bsink_playback_destroy(crypto->sink);
    NEXUS_UnregisterEvent(crypto->dma.callback);
    BKNI_DestroyEvent(crypto->dma.event);
    batom_accum_destroy(crypto->parser_acc);
    batom_pipe_destroy(crypto->parser_pipe);
    batom_pipe_destroy(crypto->ts_feed.pipe_clear);
    batom_factory_destroy(crypto->factory);
    BDBG_OBJECT_DESTROY(crypto, b_pump_crypto_t);
    BKNI_Free(crypto);
    BDBG_MSG_TRACE(("b_pump_crypto_destroy<: %#lx", (unsigned long)crypto));
    return;
}

void
b_pump_crypto_status(b_pump_crypto_t crypto)
{
    batom_factory_stats factory_stats;

    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);

    BDBG_MSG(("crypto_status: pending:%p:%u  pipe_in:%p ts_bytes:%u %s DMA:%u",
        (void *)BFIFO_READ(&crypto->pending), BFIFO_READ_PEEK(&crypto->pending)?BFIFO_READ(&crypto->pending)->ref_cnt:0,
        (void *)batom_pipe_peek(crypto->ts_feed.pipe_clear),
        (unsigned)crypto->ts_feed.ts_bytes, crypto->ts_feed.feed_stall?"STALL":"",
        crypto->dma.atom?(unsigned)batom_len(crypto->dma.atom):0 ));

    batom_factory_get_stats(crypto->factory, &factory_stats);
    BDBG_MSG(("crypto_status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u/%u arena:%u/%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.full_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.full_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));

    return;
}


void
b_pump_crypto_stop(b_pump_crypto_t crypto)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BDBG_MSG_TRACE(("b_pump_crypto_stop>: %#lx", (unsigned long)crypto));
    BDBG_ASSERT(crypto->pump);

    if(crypto->dma.atom) {
        batom_release(crypto->dma.atom);
        crypto->dma.atom = NULL;
    }

    batom_accum_clear(crypto->parser_acc);
    batom_pipe_flush(crypto->parser_pipe);
    batom_pipe_flush(crypto->ts_feed.pipe_clear);
    bsink_playback_stop(crypto->sink);
    b_pump_reset_ts_feed(crypto);
    BFIFO_INIT(&crypto->pending, crypto->pump->item_mem, crypto->pump->openSettings.numDescriptors);
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
    BSTD_UNUSED(rc);
    NEXUS_PlaypumpExternalCrypto_Stop(crypto->dma.job);
    crypto->dma.job = NULL;
#else
    if(crypto->dma.busy) {
        unsigned i;
        NEXUS_DmaJobStatus jobStatus;

        for(i=0;i<100;i++) {
            rc = NEXUS_DmaJob_GetStatus(crypto->dma.job, &jobStatus);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);break;}
            if(jobStatus.currentState == NEXUS_DmaJobState_eComplete) {
                break;
            }
            BDBG_MSG(("b_pump_crypto_stop: %#lx waiting for DMA to complete %u", (unsigned long)crypto, i));
            BKNI_Sleep(1);
        }
        crypto->dma.busy = false;
    }
    NEXUS_DmaJob_Destroy(crypto->dma.job);
    crypto->dma.job = NULL;
#endif
    BDBG_MSG_TRACE(("b_pump_crypto_stop<: %#lx", (unsigned long)crypto));
    return;
}

void
b_pump_crypto_reclaim(b_pump_crypto_t crypto)
{
    bsink_playback_status sink_status;
    size_t ts_bytes;

    if(!crypto->ts_feed.pipe_clear) {
        /* run-away event */
        return;
    }

    BDBG_MSG_TRACE(("b_pump_crypto_reclaim>: %#lx", (unsigned long)crypto));
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    ts_bytes = bsink_playback_feed(crypto->sink, crypto->ts_feed.pipe_clear);
    BDBG_MSG_TRACE(("b_pump_crypto_reclaim: %#lx ts_bytes:%u(%u)", (unsigned long)crypto, ts_bytes, crypto->ts_feed.ts_bytes));
    crypto->ts_feed.ts_bytes += ts_bytes;
    b_pump_crypto_advance_recycle(crypto);
    bsink_playback_get_status(crypto->sink, &sink_status);
    if(sink_status.idle) {
        crypto->ts_feed.ts_bytes = 0;
        /* we have played all queued up entries */
        b_play_next(crypto->pump);
    }
    BDBG_MSG_TRACE(("b_pump_crypto_reclaim<: %#lx %s ts_bytes:%u", (unsigned long)crypto, sink_status.idle?"IDLE":"", crypto->ts_feed.ts_bytes));
}

void
b_pump_crypto_flush(b_pump_crypto_t crypto)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BDBG_MSG_TRACE(("b_pump_crypto_flush>: %#lx", (unsigned long)crypto));
    if(crypto->dma.atom) {
        batom_release(crypto->dma.atom);
        crypto->dma.atom = NULL;
    }
    /* don't reset crypto->dma.busy */
    b_pump_reset_ts_feed(crypto);
    batom_accum_clear(crypto->parser_acc);
    batom_pipe_flush(crypto->parser_pipe);
    batom_pipe_flush(crypto->ts_feed.pipe_clear);
    bsink_playback_stop(crypto->sink);
    b_pump_crypto_advance_recycle(crypto);
    rc = bsink_playback_start(crypto->sink);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);}
    BFIFO_INIT(&crypto->pending, crypto->pump->item_mem, crypto->pump->openSettings.numDescriptors);
    BDBG_MSG_TRACE(("b_pump_crypto_flush<: %#lx", (unsigned long)crypto));
    return;
}

NEXUS_Error
b_pump_crypto_set_stream_type(b_pump_crypto_t crypto, NEXUS_TransportType transportType, bool *supported)
{
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);

    BDBG_ASSERT(supported);
    switch(transportType) {
    case NEXUS_TransportType_eTs:
        *supported = true;
        break;
    default:
        *supported = false;
        break;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error
b_pump_crypto_start(b_pump_crypto_t crypto)
{
    NEXUS_Error rc;
#if !NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
    unsigned i;
    NEXUS_DmaJobSettings jobSettings;
#endif

    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    BDBG_MSG_TRACE(("b_pump_crypto_start>: %#lx", (unsigned long)crypto));

    BDBG_ASSERT(crypto->dma.job==NULL); /* shall not call start without stop */

#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
    crypto->dma.job = NEXUS_PlaypumpExternalCrypto_Start(crypto->pump->settings.securityContext);
    if(!crypto->dma.job) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_dma;}
#else
#if (!NEXUS_HAS_XPT_DMA)
    if(!g_NEXUS_Transport_P_State.moduleSettings.dma) { rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_dma;}
#endif
    if(!crypto->pump->settings.securityDma) { rc = BERR_TRACE(BERR_NOT_SUPPORTED);goto err_dma;}

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    jobSettings.numBlocks = B_PUMP_CRYPT_MAX_TS_SEGMENTS;
    jobSettings.dataFormat = crypto->pump->settings.securityDmaDataFormat;
    jobSettings.keySlot = crypto->pump->settings.securityContext;
    crypto->dma.job = NEXUS_DmaJob_Create(crypto->pump->settings.securityDma, &jobSettings);
    if(!crypto->dma.job) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_dma;}

    for(i=0;i<B_PUMP_CRYPT_MAX_TS_SEGMENTS;i++) { /* initialize dma blocks */
        NEXUS_DmaJob_GetDefaultBlockSettings(&crypto->dma.blocks[i]);
    }
    crypto->dma.blocks[0].resetCrypto = true;
    crypto->dma.blocks[0].scatterGatherCryptoStart = true;
#endif

    rc = bsink_playback_start(crypto->sink);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_sink_start; }

    crypto->dma.packetSize = NEXUS_IS_DSS_MODE(crypto->pump->settings.transportType)?130:188;
    crypto->dma.packetOffset = 0;
    if (crypto->pump->settings.timestamp.type != NEXUS_TransportTimestampType_eNone) {
        crypto->dma.packetSize += 4;
        crypto->dma.packetOffset = 4;
    }

    BDBG_MSG_TRACE(("b_pump_crypto_start<: %#lx", (unsigned long)crypto));
    return NEXUS_SUCCESS;

err_sink_start:
#if NEXUS_PLAYPUMP_EXTERNAL_CRYPTO
    NEXUS_PlaypumpExternalCrypto_Stop(crypto->dma.job);
#else
    NEXUS_DmaJob_Destroy(crypto->dma.job);
#endif
    crypto->dma.job = NULL;
err_dma:
    return rc;
}

bool
b_pump_crypto_is_empty(b_pump_crypto_t crypto)
{
    BDBG_OBJECT_ASSERT(crypto, b_pump_crypto_t);
    return crypto->dma.atom == NULL && crypto->ts_feed.ts_bytes == 0;
}

#else
void b_pump_crypto_enqueue(b_pump_crypto_t crypto, struct bpvr_queue_item *item)
{
    BSTD_UNUSED(crypto);
    BSTD_UNUSED(item);
}

bool b_pump_crypto_feed(b_pump_crypto_t crypto, struct bpvr_queue_item *item, const void *data, size_t len)
{
    BSTD_UNUSED(crypto);
    BSTD_UNUSED(item);
    BSTD_UNUSED(data);
    BSTD_UNUSED(len);
    return false;
}

bool b_pump_crypto_is_empty(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
    return false;
}

NEXUS_Error b_pump_crypto_start(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error b_pump_crypto_set_stream_type(b_pump_crypto_t crypto, NEXUS_TransportType transportType, bool *supported)
{
    BSTD_UNUSED(crypto);
    BSTD_UNUSED(transportType);
    BSTD_UNUSED(supported);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void b_pump_crypto_flush(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
}

void b_pump_crypto_reclaim(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
}

void b_pump_crypto_stop(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
}

void b_pump_crypto_status(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
}

void b_pump_crypto_destroy(b_pump_crypto_t crypto)
{
    BSTD_UNUSED(crypto);
}

b_pump_crypto_t b_pump_crypto_create(NEXUS_PlaypumpHandle pump)
{
    BSTD_UNUSED(pump);
    return NULL;
}

#endif
