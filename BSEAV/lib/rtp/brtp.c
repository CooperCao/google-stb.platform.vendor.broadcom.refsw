/******************************************************************************
 * (c) 2006-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "brtp.h"
#include "brtp_packet.h"
#include "brtp_parser.h"
#include "btimestamp.h"
#include "bfifo.h"

BDBG_MODULE(brtp);

BLST_SQ_HEAD(b_rtp_queue, brtp_packet);

BDBG_OBJECT_ID(brtp_t);

#define B_RTP_PKT_FIFO_SIZE     32
BFIFO_HEAD(b_rtp_info_fifo, brtp_pkt_info);

struct brtp {
    BDBG_OBJECT(brtp_t)
    brtp_cfg cfg;
    brtp_parser_t parser;
    brtp_session_cfg session_cfg;
    struct b_rtp_queue waiting;
    struct b_rtp_queue reordering;
    struct b_rtp_queue free;
    struct {
        uint16_t next_sequence; /* next expected sequence number */
        uint32_t last_timestamp; /* timestamp of the last delivered packet */
        uint32_t last_ssrc; /* last known SSRC */
        unsigned old_cnt; /* number of old discarded packets */
        struct b_rtp_info_fifo pkt_info_fifo;
    } state;
    brtp_pkt_info pkt_info[B_RTP_PKT_FIFO_SIZE];
    struct brtp_packet packets[1]; /* variable size array */
};
#define B_RTP_NEXT_SEQUENCE(seq) ((uint16_t)((seq)+1))


static void
b_rtp_pkt_free_noop(void *source_cntx, void *pkt)
{
    BSTD_UNUSED(source_cntx);
    BSTD_UNUSED(pkt);
    return ;
}
static void
b_rtp_noparser_noop(void *noparser_cntx, const void *pkt)
{
    BSTD_UNUSED(noparser_cntx);
    BSTD_UNUSED(pkt);
    return;
}


static const brtp_cfg b_rtp_default_cfg  = {
    128, /* 120 ms delay, for 10Mbps stream and 1400 bytes payload per packet */
    120, /* wait_time, ms */
    NULL, b_rtp_pkt_free_noop,
    NULL, b_rtp_noparser_noop
};

void
brtp_default_cfg(brtp_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    *cfg = b_rtp_default_cfg;
    return;
}

static const brtp_session_cfg b_rtp_default_session_cfg = {
    0, 0, /* pt_mask, pt_value */
    0, 0  /* ssrc_mask, ssrc_value */
};

void
brtp_default_session_cfg(brtp_t rtp, brtp_session_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    BDBG_OBJECT_ASSERT(rtp, brtp_t);
    *cfg = b_rtp_default_session_cfg;
    return;
}


static void
b_rtp_reset(brtp_t rtp)
{
    unsigned i;
    brtp_packet_t pkt;

    /* TODO: debug this, had to comment it out to avoid a crash */
    //BDBG_OBJECT_ASSERT(rtp, brtp_t);
    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->waiting))) {
        rtp->cfg.pkt_free(rtp->cfg.pkt_source, pkt->cookie);
        BLST_SQ_REMOVE_HEAD(&rtp->waiting, queue);
    }
    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->reordering))) {
        rtp->cfg.pkt_free(rtp->cfg.pkt_source, pkt->cookie);
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
    }
    BLST_SQ_INIT(&rtp->free);
    for(i=0;i<rtp->cfg.npackets;i++) {
        BLST_SQ_INSERT_TAIL(&rtp->free, &rtp->packets[i], queue); /* make them sorted in natural way */
    }
    BFIFO_INIT(&rtp->state.pkt_info_fifo, rtp->pkt_info, B_RTP_PKT_FIFO_SIZE);
    rtp->state.next_sequence = 0;
    rtp->state.last_timestamp = 0;
    rtp->state.last_ssrc = 0;
    rtp->state.old_cnt = 0;
    rtp->parser = NULL;
    return;
}

static void
b_rtp_flush_queues(brtp_t rtp)
{
    unsigned i;
    brtp_packet_t pkt;

    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->waiting))) {
        rtp->cfg.pkt_free(rtp->cfg.pkt_source, pkt->cookie);
        BLST_SQ_REMOVE_HEAD(&rtp->waiting, queue);
    }
    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->reordering))) {
        rtp->cfg.pkt_free(rtp->cfg.pkt_source, pkt->cookie);
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
    }
    BLST_SQ_INIT(&rtp->free);
    for(i=0;i<rtp->cfg.npackets;i++) {
        BLST_SQ_INSERT_TAIL(&rtp->free, &rtp->packets[i], queue); /* make them sorted in natural way */
    }
    return;
}

static void
b_rtp_process_pkt(brtp_t rtp, brtp_packet_t rtp_pkt)
{
    uint32_t word0;
    unsigned sequence;
    unsigned cc;
    unsigned offset;
    size_t pkt_len;

    for(;;) {
        word0 = B_RTP_LOAD32(rtp_pkt->data,0);
        sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);

        rtp->state.last_timestamp = B_RTP_PKT_TIMESTAMP(rtp_pkt);
        rtp->state.next_sequence = B_RTP_NEXT_SEQUENCE(sequence);
        rtp->state.last_ssrc = B_RTP_LOAD32(rtp_pkt->data, 8);
        cc = B_RTP_GET_BITS(word0, 4, 7);
        offset = 12+cc*4;
        /* Check for header extension */
        if (B_RTP_GET_BIT(word0, 3)) {
            offset += (B_RTP_LOAD16(rtp_pkt->data, offset+2) + 1) * 4;
        }

        BDBG_ASSERT(rtp_pkt->len > offset);
        pkt_len = rtp_pkt->len - offset;
        if (B_RTP_GET_BIT(word0, 2)) {
            unsigned padding = B_RTP_LOAD8(rtp_pkt->data, rtp_pkt->len-1);

            if (pkt_len<=padding) {
                goto discard;
            }
            pkt_len -= padding;
        }
        if(BFIFO_WRITE_PEEK(&rtp->state.pkt_info_fifo)) {
            brtp_pkt_info *info;
            info = BFIFO_WRITE(&rtp->state.pkt_info_fifo);
            info->timestamp = rtp->state.last_timestamp;
            info->ssrc = rtp->state.last_ssrc;
            info->sequence = sequence;
            info->packet_size = pkt_len;
            BFIFO_WRITE_COMMIT(&rtp->state.pkt_info_fifo, 1);
        }
        if (rtp->parser==NULL) {
            goto discard;
        }
        BLST_SQ_INSERT_TAIL(&rtp->waiting, rtp_pkt, queue); /* insert to tail, so we could remove from head */
        rtp->parser->packet(rtp->parser, rtp_pkt, (uint8_t *)rtp_pkt->data + offset, pkt_len);
        goto next;
discard:
        /* there is no parser attached */
        BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
next:
        rtp_pkt = BLST_SQ_FIRST(&rtp->reordering);
        if(rtp_pkt==NULL ||
            rtp->state.next_sequence != B_RTP_PKT_SEQUENCE(rtp_pkt)) {
            break;
        }
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
        BDBG_MSG(("b_rtp_process_pkt: %#lx picking pkt %u from the reordering queue", (unsigned long)rtp, B_RTP_PKT_SEQUENCE(rtp_pkt)));
    }
}


static int
b_rtp_uint16_diff(uint16_t a, uint16_t b)
{
#if 1
    int diff = (int16_t)((int16_t)a-(int16_t)b);
    return diff;
#else
    int diff = a - b;

    if (diff > (1<<15)) {
        return diff-(1<<16);
    } else if (diff < -(1<<15)) {
        return (1<<16)+diff;
    } else {
        return diff;
    }
#endif
}

static void
b_rtp_reorder_pkt(brtp_t rtp, brtp_packet_t rtp_pkt)
{
    int diff;
    unsigned sequence;
    brtp_packet_t prev,p;

    sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);
    diff = b_rtp_uint16_diff(sequence,rtp->state.next_sequence);
    if (diff<0) { /* old packet ingore it */
        if (rtp->state.old_cnt<100) {
            BDBG_WRN(("b_rtp_reorder_pkt: %#lx(%u) discarding old packet %u:%u %d",(unsigned long)rtp, rtp->state.old_cnt, sequence, rtp->state.next_sequence, diff));
            rtp->state.old_cnt++;
            goto discard;
        } else { /* don't discard more than 100 packets in the row */
            BDBG_WRN(("b_rtp_reorder_pkt: %#lx resyncing sequence packet %u:%u %d",(unsigned long)rtp, sequence, rtp->state.next_sequence, diff));
            rtp->parser->discontinuity(rtp->parser);
            b_rtp_process_pkt(rtp, rtp_pkt);
        }
    }
    rtp->state.old_cnt = 0; /* reset old_cnt */
    /* insert packet into the sorted list. The earliest packet must be on the head of the list.*/
    for(prev=NULL,p=BLST_SQ_FIRST(&rtp->reordering);p;p=BLST_SQ_NEXT(p, queue)) {
        diff = b_rtp_uint16_diff(sequence, B_RTP_PKT_SEQUENCE(p));
        if (diff==0) {
            BDBG_MSG(("b_rtp_reorder_pkt: %#lx duplicate %u",(unsigned long)rtp, sequence));
            goto discard;
        }
        if (diff < 0) {
            break;
        }
        prev = p;
    }
    if (prev==NULL) {
        BDBG_MSG(("b_rtp_reorder_pkt: %#lx queuing first pkt %u", (unsigned long)rtp, B_RTP_PKT_SEQUENCE(rtp_pkt)));
        BLST_SQ_INSERT_HEAD(&rtp->reordering, rtp_pkt, queue);
    } else {
        BDBG_MSG(("b_rtp_reorder_pkt: %#lx queuing pkt %u after %u", (unsigned long)rtp, B_RTP_PKT_SEQUENCE(rtp_pkt), B_RTP_PKT_SEQUENCE(prev)));
        BLST_SQ_INSERT_AFTER(&rtp->reordering, prev, rtp_pkt, queue);
    }
    p = BLST_SQ_LAST(&rtp->reordering);
    BDBG_ASSERT(p);
    diff = brtp_delta_time(B_RTP_PKT_TIMESTAMP(p), rtp->state.last_timestamp);
    if (diff > (int)(rtp->cfg.wait_time*rtp->parser->stream.cfg.clock_rate)/(1000*2) /* last packet stayed too late */
        /* | diff < 0  */ /* discontinuity */) {
        p = BLST_SQ_FIRST(&rtp->reordering);
        BDBG_ASSERT(p);
        BDBG_WRN(("b_rtp_reorder_pkt: %#lx applying pkt %u->%u (%d,%d)", (unsigned long)rtp, rtp->state.next_sequence, B_RTP_PKT_SEQUENCE(p), b_rtp_uint16_diff(B_RTP_PKT_SEQUENCE(p), rtp->state.next_sequence), diff));
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
        rtp->parser->discontinuity(rtp->parser);
        b_rtp_process_pkt(rtp, p);
    }
    return;
discard:
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    return;
}

static void
b_rtp_scan_recycle(brtp_t rtp)
{
    brtp_packet_t rtp_pkt;

    while( NULL != (rtp_pkt = BLST_SQ_FIRST(&rtp->waiting))) {
        if (rtp_pkt->ref_cnt) {
            break;
        }
        BDBG_MSG(("b_rtp_scan_recycle: %#lx recycling %#lx", (unsigned long)rtp, (unsigned long)rtp_pkt));
        rtp->cfg.pkt_free(rtp->cfg.pkt_source, rtp_pkt->cookie);
        BLST_SQ_REMOVE_HEAD(&rtp->waiting, queue);
        BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    }
    return;
}

brtp_enqueue
brtp_enqueue_pkt(brtp_t rtp, const void *pkt, size_t pkt_len, void *cookie)
{
    uint32_t word0;
    unsigned cc;
    unsigned sequence;
    brtp_packet_t rtp_pkt;

    BDBG_OBJECT_ASSERT(rtp, brtp_t);

    /* there is shall be a complete header */
    if (pkt_len <= 12) { goto invalid_pkt; }
    word0 = B_RTP_LOAD32(pkt,0);
    /* version shall be equal to 2 */
    if (B_RTP_GET_BITS(word0, 0, 1) != 2) { goto invalid_pkt; }
    cc = B_RTP_GET_BITS(word0, 4, 7);
    /* now we know complete header size */
    if (pkt_len <= 12+cc*4) { goto invalid_pkt; }

    if(rtp->parser==NULL) {
        BDBG_MSG(("brtp_enqueue_pkt: %#lx %#lx premature pkt %u ssrc:%#x", (unsigned long)rtp, (unsigned long)pkt, pkt_len));
        /* no parser attached */
        rtp->cfg.noparser(rtp->cfg.noparser_cntx, pkt);
        if(rtp->parser==NULL)
        {
            BDBG_MSG(("WTF !! Still not Started?"));
            return brtp_enqueue_discard;
        }
    }

    BDBG_MSG(("%#x brtp_enqueue_pkt: %#lx packet:%#lx:%u type:%u ssrc:%#x ts:%u seq:%u(%u)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), B_RTP_LOAD32(pkt, 8), B_RTP_TIMESTAMP(pkt), (unsigned)B_RTP_GET_BITS(word0, 16, 31), (unsigned)rtp->state.next_sequence));
    if (
        ((B_RTP_GET_BITS(word0, 9, 15) & rtp->session_cfg.pt_mask) !=  rtp->session_cfg.pt_value) ||
        (B_RTP_LOAD32(pkt, 8) & rtp->session_cfg.ssrc_mask) != rtp->session_cfg.ssrc_value) {
        BDBG_WRN(("%#x brtp_enqueue_pkt: %#lx packet:%#lx:%u filtered out type:%u(%u) ssrc:%#x(%#x)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), rtp->session_cfg.pt_value, B_RTP_LOAD32(pkt, 8), rtp->session_cfg.ssrc_value));
        goto discard_pkt;
    }

    BDBG_ASSERT(rtp->parser->stream.mux);

    b_rtp_scan_recycle(rtp);

    rtp_pkt = BLST_SQ_FIRST(&rtp->free);
    if (!rtp_pkt) {
        BDBG_WRN(("brtp_enqueue_pkt: %#lx overflow for pkt %#lx", (unsigned long)rtp, (unsigned long)pkt));
        b_rtp_flush_queues(rtp);
        rtp_pkt = BLST_SQ_FIRST(&rtp->free);
    }
    BLST_SQ_REMOVE_HEAD(&rtp->free, queue);
    rtp_pkt->data = pkt;
    rtp_pkt->len = pkt_len;
    rtp_pkt->ref_cnt = 0;
    rtp_pkt->cookie = cookie;

    sequence = B_RTP_GET_BITS(word0, 16, 31);
    if (sequence == rtp->state.next_sequence ||  /* match */
            (rtp->state.next_sequence == 0 && BLST_SQ_FIRST(&rtp->waiting)==NULL && BLST_SQ_FIRST(&rtp->reordering)==NULL) /* first packet, uninitialized next_sequence */
            ) {
        /* fast path */
        b_rtp_process_pkt(rtp, rtp_pkt);
    } else {
        BDBG_MSG(("brtp_enqueue_pkt: %#lx reordering %u %u", (unsigned long)rtp, sequence, rtp->state.next_sequence));
        /* add into the reordering queue */
        b_rtp_reorder_pkt(rtp, rtp_pkt);
    }
    return brtp_enqueue_queued;

discard_pkt:
    return brtp_enqueue_discard;
    return brtp_enqueue_overflow;
invalid_pkt:
    BDBG_WRN(("brtp_enqueue_pkt: %#lx discarding pkt %#lx:%u",  (unsigned long)rtp, (unsigned long)pkt, (unsigned)pkt_len));
    /* recycle packet */
    /* rtp->cfg.pkt_free(rtp->cfg.pkt_source, cookie); */
    return brtp_enqueue_invalid;
}

void
brtp_start(brtp_t rtp, brtp_parser_t parser, const brtp_session_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_t);

    BDBG_ASSERT(parser);
    BDBG_ASSERT(cfg);

    rtp->session_cfg = *cfg;
    rtp->parser = parser;
    return;
}

void
brtp_stop(brtp_t rtp)
{
    /* TODO: debug this, had to comment it out to avoid a crash */
    //BDBG_OBJECT_ASSERT(rtp, brtp_t);
    b_rtp_reset(rtp);
}

static int
b_rtp_ntp_diff_test(uint32_t ntp_msw1, uint32_t ntp_lsw1, uint32_t ntp_msw2, uint32_t ntp_lsw2)
{
    brtp_ntp ntp1, ntp2;

    brtp_ntp_init(&ntp1, ntp_msw1, ntp_lsw1);
    brtp_ntp_init(&ntp2, ntp_msw2, ntp_lsw2);
    return brtp_ntp_diff(&ntp1, &ntp2);
}

brtp_t
brtp_create(const brtp_cfg *cfg)
{
    brtp_t rtp;

    BDBG_ASSERT(cfg);

    BDBG_CASSERT(B_RTP_GET_BIT((1<<31),0));
    BDBG_CASSERT(B_RTP_GET_BIT((1<<30),0)==0);

    BDBG_CASSERT(B_RTP_GET_BITS((1<<31),0,0)==1);
    BDBG_CASSERT(B_RTP_GET_BITS((1<<31),0,1)==2);
    BDBG_CASSERT(B_RTP_GET_BITS(1,31,31)==1);
    BDBG_CASSERT(B_RTP_GET_BITS(1,26,31)==1);

    BDBG_ASSERT(b_rtp_uint16_diff(1, 0) == 1);
    BDBG_ASSERT(b_rtp_uint16_diff(0, 1) == -1);
    BDBG_ASSERT(b_rtp_uint16_diff(0, 16384) == -16384);
    BDBG_ASSERT(b_rtp_uint16_diff(16384, 0) == 16384);
    BDBG_ASSERT(b_rtp_uint16_diff(16384, 16384) == 0);
    BDBG_ASSERT(b_rtp_uint16_diff(-1, 0) == -1);
    BDBG_ASSERT(b_rtp_uint16_diff(-1024, 1024) == -2048);
    BDBG_ASSERT(b_rtp_uint16_diff(1024, -1024) == 2048);
    BDBG_ASSERT(b_rtp_uint16_diff(0, -1) == 1);
    BDBG_ASSERT(b_rtp_uint16_diff(-1024, -2048) == 1024);
    BDBG_ASSERT(b_rtp_uint16_diff(-2048, -1024) == -1024);

    BDBG_ASSERT(brtp_delta_time(2, 0) == 1);
    BDBG_ASSERT(brtp_delta_time(0, 2) == -1);
    BDBG_ASSERT(brtp_delta_time(0, 16384) == -8192);
    BDBG_ASSERT(brtp_delta_time(16384, 0) == 8192);
    BDBG_ASSERT(brtp_delta_time(16384, 16384) == 0);
    BDBG_ASSERT(brtp_delta_time(-2, 0) == -1);
    BDBG_ASSERT(brtp_delta_time(-1024, 1024) == -1024);
    BDBG_ASSERT(brtp_delta_time(1024, -1024) == 1024);
    BDBG_ASSERT(brtp_delta_time(0, -2) == 1);
    BDBG_ASSERT(brtp_delta_time(16349, 4294746617U) == 118514);
    BDBG_ASSERT(brtp_delta_time(4294746617U, 16349) == -118514);
    BDBG_ASSERT(brtp_delta_time(16349, 0) == 8174);
    BDBG_ASSERT(brtp_delta_time(0, 16349) == -8174);

#if 0
    BDBG_ERR(("%.5f\n", 0.001*(1u<<31)));
    BDBG_ERR(("%d",b_rtp_ntp_diff_test(0, 4*2147483, 0, 0)));
    BDBG_ERR(("%d",b_rtp_ntp_diff_test(0, 0xFFFFFFFFul, 0, 0)));
    BDBG_ERR(("%d",b_rtp_ntp_diff_test(0, 0, 0, 0xFFFFFFFFul)));
#endif

    b_rtp_ntp_diff_test(0, 0, 0, 0);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0, 0, 0) == 0);
    BDBG_ASSERT(b_rtp_ntp_diff_test(1, 0, 0, 0) == 1000);
    BDBG_ASSERT(b_rtp_ntp_diff_test(1, 0, 2, 0) == -1000);
    BDBG_ASSERT(b_rtp_ntp_diff_test(1000+(1u<<31), 0, 2000+(1u<<31), 0) == -1000*1000);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0xFFFFFFFFul, 0, 0) == 999);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0x7FFFFFFFul, 0, 0) == 499);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0x3FFFFFFFul, 0, 0) == 249);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0, 0, 0x7FFFFFFFul) == -500);
    BDBG_ASSERT(b_rtp_ntp_diff_test(0, 0, 0, 0x3FFFFFFFul) == -250);


    rtp = BKNI_Malloc(sizeof(*rtp)+cfg->npackets*sizeof(struct brtp_packet));
    if (!rtp) {
        BDBG_ERR(("out of memory in brtp_create"));
        return NULL;
    }
    BDBG_OBJECT_INIT(rtp, brtp_t);
    rtp->cfg = *cfg;
    rtp->session_cfg = b_rtp_default_session_cfg;
    rtp->parser = NULL;
    BLST_SQ_INIT(&rtp->waiting);
    BLST_SQ_INIT(&rtp->reordering);
    b_rtp_reset(rtp);
    return rtp;
}

void
brtp_destroy(brtp_t rtp)
{
    BDBG_OBJECT_DESTROY(rtp, brtp_t);
    BKNI_Free(rtp);
    return;
}

int
brtp_get_pkt_info(brtp_t rtp, brtp_pkt_info *info, size_t max_entries)
{
    unsigned to_read;

    BDBG_OBJECT_ASSERT(rtp, brtp_t);

    to_read = BFIFO_READ_PEEK(&rtp->state.pkt_info_fifo);
    if (to_read==0) {
        return 0;
    }
    if(to_read>max_entries) {
        to_read = max_entries;
    }
    BKNI_Memcpy(info, BFIFO_WRITE(&rtp->state.pkt_info_fifo), to_read*sizeof(*info));
    BFIFO_READ_COMMIT(&rtp->state.pkt_info_fifo, to_read);
    return (int) to_read;
}
