/******************************************************************************
 * (c) 2007-2016 Broadcom Corporation
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

#include "brtp_spf.h"
#include "brtp_packet.h"
#include "btimestamp.h"
#include "bfifo.h"

BDBG_MODULE(brtp_spf);

BLST_SQ_HEAD(b_rtp_queue, brtp_packet);

#if (SPF_SUPPORT==n)
/*
   This change includes both version 9 and version 10 of this file. This was done intentionally
   so that the Android group can build ip_client/ip_streamer along with librtp without using
   a patch file. There is some unknown conflict between version 9 and version 10 that prevents
   MPEG2TS and Google ES from working at the same time. We introduced the SPF_SUPPORT flag to
   allow the user to select which one he wants to work. If SPF_SUPPORT=n, then MPEG2TS streams
   will work; if SPF_SUPPORT=y, then Google ES streams will work, but MPEG2TS streams will fail.
   Yes, I know this is a hacky kludge, and is a very ugly way to resolve this conflict. Some
   time in the future, this conflict will be resolved in a more elegant manner.
*/
/* this is brtp_spf.c@@/main/10 */
BDBG_OBJECT_ID(brtp_t);

#define B_RTP_PKT_FIFO_SIZE     32
BFIFO_HEAD(b_rtp_info_fifo, brtp_pkt_info);

struct brtp {
    BDBG_OBJECT(brtp_t)
    brtp_cfg cfg;
    brtp_stats stats;
    batom_pipe_t pipe_out;
    brtp_session_cfg session_cfg;
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
/* use dbg macro for data flow. To use, uncomment the following */
#define BDBG_MSG_FLOW(x) BDBG_MSG(x)

static int b_rtp_copy_to_user(void *dst, void *src, unsigned size)
{
    /* null implementation */
    BSTD_UNUSED(dst);
    BSTD_UNUSED(src);
    BSTD_UNUSED(size);

    BDBG_ERR(("null b_rtp_copy_to_user() implementation"));

    return -1;
}

static const brtp_cfg b_rtp_default_cfg  = {
    128, /* ~67 ms delay, for 20Mbps stream and 1316 bytes payload per packet */
    60, /* wait_time, ms (a larger wait time will require more skb's to be made available) */
    NULL, /* pointer to the factory */
    b_rtp_copy_to_user
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

    BDBG_OBJECT_ASSERT(rtp, brtp_t);
    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->reordering))) {
        batom_release(pkt->cookie);
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
    rtp->pipe_out = NULL;

    return;
}

static void
b_rtp_atom_user_free(batom_t atom, void *user)
{
    batom_t child_atom;

    BSTD_UNUSED(atom);
    child_atom = *(void**)user;
    batom_release(child_atom);
}

static const batom_user b_rtp_atom_user = {
    b_rtp_atom_user_free,
    sizeof(void **)
};

static void
b_rtp_process_pkt(brtp_t rtp, brtp_packet_t rtp_pkt)
{
    uint32_t word0;
    unsigned sequence;
    unsigned cc;
    unsigned offset;
    unsigned ext_len;
    unsigned long base;
    size_t pkt_len;
    batom_t atom;
    batom_cursor cursor;

    rtp->state.old_cnt = 0; /* reset old_cnt */

    for(;;) {
        word0 = B_RTP_LOAD32(rtp_pkt->data,0);
        sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);

        rtp->state.last_timestamp = B_RTP_PKT_TIMESTAMP(rtp_pkt);
        rtp->state.next_sequence = B_RTP_NEXT_SEQUENCE(sequence);
        rtp->state.last_ssrc = B_RTP_LOAD32(rtp_pkt->data, 8);
        cc = B_RTP_GET_BITS(word0, 4, 7);
        offset = 12+cc*4;
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
        if (rtp->pipe_out==NULL) {
            goto discard;
        }

        batom_cursor_from_atom(&cursor, rtp_pkt->cookie);

        ext_len = 0;
        base = (unsigned long)((uint8_t *)cursor.vec->base)+offset;
        if (B_RTP_GET_BIT(word0, 3)) {
            rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)base, 4); /* copy header extension, reusing the rtp_pkt->data buffer */
            ext_len = (B_RTP_LOAD16(rtp_pkt->data, 2)*4)+4;
        }
        base += ext_len;

        BDBG_ASSERT(rtp->cfg.factory);
        atom = batom_from_range(rtp->cfg.factory,
                                (void*)base,
                                cursor.vec->len-(offset+ext_len),
                                &b_rtp_atom_user,
                                &rtp_pkt->cookie);
                rtp->stats.bytesReceived += cursor.vec->len-(offset+ext_len);
        batom_pipe_push(rtp->pipe_out, atom);
        BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
        goto next;
discard:
        BDBG_MSG(("b_rtp_process_pkt: %#lx discarding pkt", (unsigned long)rtp));
        batom_release(rtp_pkt->cookie);
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

        rtp->stats.packetsOutOfSequence++;
    sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);
    diff = b_rtp_uint16_diff(sequence,rtp->state.next_sequence);
    if (diff<0) { /* old packet ingore it */
        if (rtp->state.old_cnt<100) {
            BDBG_MSG(("b_rtp_reorder_pkt: %#lx(%u) discarding old packet %u:%u %d",(unsigned long)rtp, rtp->state.old_cnt, sequence, rtp->state.next_sequence, diff));
            rtp->state.old_cnt++;
            goto discard;
        } else { /* don't discard more than 100 packets in the row */
            BDBG_MSG(("b_rtp_reorder_pkt: %#lx resyncing sequence packet %u:%u %d",(unsigned long)rtp, sequence, rtp->state.next_sequence, diff));
            b_rtp_process_pkt(rtp, rtp_pkt);
            rtp->state.old_cnt = 0; /* reset old_cnt */
            goto done;
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
    /* TODO: fudge clock_rate for now */
    if (diff > (int)(rtp->cfg.wait_time*90000)/(1000*2) /* last packet stayed too late */
        || diff < 0 ||
        (diff==0 && rtp->state.last_timestamp==0)) {
        p = BLST_SQ_FIRST(&rtp->reordering);
        BDBG_ASSERT(p);
        BDBG_MSG(("b_rtp_reorder_pkt: %#lx applying pkt %u->%u (%d,%d)", (unsigned long)rtp, rtp->state.next_sequence, B_RTP_PKT_SEQUENCE(p), b_rtp_uint16_diff(B_RTP_PKT_SEQUENCE(p), rtp->state.next_sequence), diff));
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
        b_rtp_process_pkt(rtp, p);
    }
done:
    return;
discard:
        rtp->stats.packetsDiscarded++;
    BDBG_MSG(("b_rtp_reorder_pkt: %#lx discarding pkt", (unsigned long)rtp_pkt));
    batom_release(rtp_pkt->cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    return;
}

static brtp_enqueue
brtp_enqueue_pkt(brtp_t rtp, const void *pkt, size_t pkt_len, void *cookie)
{
    uint32_t word0, word1;
    unsigned cc;
    unsigned sequence;
    brtp_packet_t rtp_pkt;

    BDBG_OBJECT_ASSERT(rtp, brtp_t);

    rtp_pkt = BLST_SQ_FIRST(&rtp->free);
    if (!rtp_pkt) {
        while(NULL!=(rtp_pkt=BLST_SQ_FIRST(&rtp->reordering))) {
            batom_release(rtp_pkt->cookie);
            BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
            BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
        }
        rtp_pkt = BLST_SQ_FIRST(&rtp->free);
        rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)pkt, 12);
        word0 = B_RTP_LOAD32(rtp_pkt->data,0);
        sequence = B_RTP_GET_BITS(word0, 16, 31);
        rtp->state.next_sequence = sequence;
        BDBG_MSG(("brtp_enqueue_pkt: %#lx overflow for pkt %#lx", (unsigned long)rtp, (unsigned long)pkt));
    }
    BLST_SQ_REMOVE_HEAD(&rtp->free, queue);

        rtp->stats.packetsReceived++;
    /* there is shall be a complete header */
    if (pkt_len <= 12) { goto invalid_pkt; }

    /* copy RTP header to a 12-byte buffer (rtp_pkt->data), then use B_RTP_LOAD32 to access fields */
    rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)pkt, 12);

    word0 = B_RTP_LOAD32(rtp_pkt->data,0);
    word1 = B_RTP_LOAD32(rtp_pkt->data,4);

    /* version shall be equal to 2 */
    if (B_RTP_GET_BITS(word0, 0, 1) != 2) { goto invalid_pkt; }
    cc = B_RTP_GET_BITS(word0, 4, 7);
    /* now we know complete header size */
    if (pkt_len <= 12+cc*4) { goto invalid_pkt; }

    if(rtp->pipe_out==NULL) {
        BDBG_MSG(("brtp_enqueue_pkt: %#lx %#lx premature pkt %u ssrc:%#x", (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_LOAD32(rtp_pkt->data, 8)));
        goto discard_pkt;
    }

    BDBG_MSG_FLOW(("%#x brtp_enqueue_pkt: %#lx packet:%#lx:%u type:%u ssrc:%#x ts:%u seq:%u(%u)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), B_RTP_LOAD32(rtp_pkt->data, 8), word1, (unsigned)B_RTP_GET_BITS(word0, 16, 31), (unsigned)rtp->state.next_sequence));
    if (
        ((B_RTP_GET_BITS(word0, 9, 15) & rtp->session_cfg.pt_mask) !=  rtp->session_cfg.pt_value) ||
        (B_RTP_LOAD32(rtp_pkt->data, 8) & rtp->session_cfg.ssrc_mask) != rtp->session_cfg.ssrc_value) {
        BDBG_MSG(("%#x brtp_enqueue_pkt: %#lx packet:%#lx:%u filtered out type:%u(%u) ssrc:%#x(%#x)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), rtp->session_cfg.pt_value, B_RTP_LOAD32(rtp_pkt->data, 8), rtp->session_cfg.ssrc_value));
        goto discard_pkt;
    }

    rtp_pkt->len = pkt_len;
    rtp_pkt->cookie = cookie;

    sequence = B_RTP_GET_BITS(word0, 16, 31);
    if (sequence == rtp->state.next_sequence ||  /* match */
            (rtp->state.next_sequence == 0 && BLST_SQ_FIRST(&rtp->reordering)==NULL) /* first packet, uninitialized next_sequence */
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
        rtp->stats.packetsDiscarded++;
    batom_release(cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    BDBG_MSG(("brtp_enqueue_pkt: %#lx discarding pkt %#lx:%u",  (unsigned long)rtp, (unsigned long)pkt, (unsigned)pkt_len));

    return brtp_enqueue_discard;
    return brtp_enqueue_overflow;
invalid_pkt:
    batom_release(cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    BDBG_MSG(("brtp_enqueue_pkt: %#lx discarding pkt %#lx:%u",  (unsigned long)rtp, (unsigned long)pkt, (unsigned)pkt_len));
    return brtp_enqueue_invalid;
}

brtp_enqueue
brtp_feed(brtp_t rtp, batom_pipe_t pipe_in) {
    int i;
    batom_t atom;
    batom_cursor cursor;
    brtp_enqueue enqueue_result = brtp_enqueue_queued;

    for(i=0;;) {
        if(batom_pipe_peek(pipe_in)) {
            atom = batom_pipe_pop(pipe_in);
            BDBG_ASSERT(atom);
            batom_cursor_from_atom(&cursor, atom);
#if 0
            {
                uint32_t udata = *(void**)batom_userdata(atom);
                BDBG_MSG_FLOW(("brtp_push() pipe1, pop: (0x%x) (0x%x)", atom, udata));
            }
#endif
            enqueue_result = brtp_enqueue_pkt(rtp, (const void *)cursor.vec->base, /*size_t pkt_len*/cursor.vec->len, /*void *cookie*/atom);
            if(enqueue_result != brtp_enqueue_queued) { BDBG_MSG_FLOW(("brtp_enqueue_pkt() returned: %d", enqueue_result)); }
        }
        else {
            /* pipe is empty */
            break;
        }
    }

    return enqueue_result;
}

void brtp_start(brtp_t rtp, batom_pipe_t pipe, const brtp_session_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_t);

    BDBG_ASSERT(pipe);
    BDBG_ASSERT(cfg);

        BKNI_Memset(&rtp->stats, 0, sizeof(struct brtp_stats));
    rtp->session_cfg = *cfg;
    rtp->pipe_out = pipe;
    return;
}

void
brtp_stop(brtp_t rtp)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_t);
    b_rtp_reset(rtp);
}

brtp_t
brtp_create(const brtp_cfg *cfg)
{
    brtp_t rtp;

    BDBG_ASSERT(cfg);

    rtp = BKNI_Malloc(sizeof(*rtp)+cfg->npackets*sizeof(struct brtp_packet));
    if (!rtp) {
        BDBG_ERR(("out of memory in brtp_create"));
        return NULL;
    }
    BDBG_OBJECT_INIT(rtp, brtp_t);

    {
        unsigned i;
        uint8_t *hdr_buf = BKNI_Malloc(sizeof(uint8_t)*12*cfg->npackets);
        if (!hdr_buf) {
            BDBG_ERR(("out of memory in brtp_create"));
            BKNI_Free(rtp);
            return NULL;
        }

        for(i=0;i<cfg->npackets;i++) {
            rtp->packets[i].data = hdr_buf+(i*12);
        }

        if (i==0) {
            if (hdr_buf)
                BKNI_Free(hdr_buf);
        }
    }

    rtp->cfg = *cfg;
    rtp->session_cfg = b_rtp_default_session_cfg;
    rtp->pipe_out = NULL;
    BLST_SQ_INIT(&rtp->reordering);
    b_rtp_reset(rtp);
    return rtp;
}

void
brtp_destroy(brtp_t rtp)
{
    BKNI_Free((void*)rtp->packets[0].data);
    BDBG_OBJECT_DESTROY(rtp, brtp_t);
    BKNI_Free((void*)rtp);
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

int brtp_get_stats(brtp_t rtp, brtp_stats *stats)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_t);
    if (stats == NULL) return -1;
        *stats = rtp->stats;
        return 0;
}
#else
#include "brtp.h"
/* this is brtp_spf.c@@/main/9 */
BDBG_OBJECT_ID(brtp_spf_t);

#define B_RTP_PKT_FIFO_SIZE     32
BFIFO_HEAD(b_rtp_info_fifo, brtp_spf_pkt_info);

struct brtp_spf {
    BDBG_OBJECT(brtp_spf_t)
    brtp_spf_cfg cfg;
    brtp_stats stats;
    batom_pipe_t pipe_out;
    brtp_spf_session_cfg session_cfg;
    struct b_rtp_queue reordering;
    struct b_rtp_queue free;
    struct {
        uint16_t next_sequence; /* next expected sequence number */
        uint32_t last_timestamp; /* timestamp of the last delivered packet */
        uint32_t last_ssrc; /* last known SSRC */
        unsigned old_cnt; /* number of old discarded packets */
        struct b_rtp_info_fifo pkt_info_fifo;
    } state;
    brtp_spf_pkt_info pkt_info[B_RTP_PKT_FIFO_SIZE];
    struct brtp_packet packets[1]; /* variable size array */
};

#define B_RTP_NEXT_SEQUENCE(seq) ((uint16_t)((seq)+1))
/* use dbg macro for data flow. To use, uncomment the following */
#define BDBG_MSG_FLOW(x) /* BDBG_MSG(x) */

static int b_rtp_copy_to_user(void *dst, void *src, unsigned size)
{
    /* null implementation */
    BSTD_UNUSED(dst);
    BSTD_UNUSED(src);
    BSTD_UNUSED(size);

    BDBG_ERR(("null b_rtp_copy_to_user() implementation"));

    return -1;
}

static const brtp_spf_cfg b_rtp_default_cfg  = {
    128, /* ~67 ms delay, for 20Mbps stream and 1316 bytes payload per packet */
    60, /* wait_time, ms (a larger wait time will require more skb's to be made available) */
    NULL, /* pointer to the factory */
    b_rtp_copy_to_user
};

void
brtp_spf_default_cfg(brtp_spf_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    *cfg = b_rtp_default_cfg;
    return;
}

static const brtp_spf_session_cfg b_rtp_default_session_cfg = {
    0, 0, /* pt_mask, pt_value */
    0, 0  /* ssrc_mask, ssrc_value */
};

void
brtp_spf_default_session_cfg(brtp_spf_t rtp, brtp_spf_session_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);
    *cfg = b_rtp_default_session_cfg;
    return;
}


static void
b_rtp_reset(brtp_spf_t rtp)
{
    unsigned i;
    brtp_packet_t pkt;

    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);
    while(NULL!=(pkt=BLST_SQ_FIRST(&rtp->reordering))) {
        batom_release(pkt->cookie);
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
    rtp->pipe_out = NULL;

    return;
}

static void
b_rtp_atom_user_free(batom_t atom, void *user)
{
    batom_t child_atom;

    BSTD_UNUSED(atom);
    child_atom = *(void**)user;
    batom_release(child_atom);
}

static const batom_user b_rtp_atom_user = {
    b_rtp_atom_user_free,
    sizeof(void **)
};

static void
b_rtp_process_pkt(brtp_spf_t rtp, brtp_packet_t rtp_pkt)
{
    uint32_t word0;
    unsigned sequence;
    unsigned cc;
    unsigned offset;
    unsigned ext_len;
    unsigned long base;
    size_t pkt_len;
    batom_t atom;
    batom_cursor cursor;

    rtp->state.old_cnt = 0; /* reset old_cnt */

    for(;;) {
        word0 = B_RTP_LOAD32(rtp_pkt->data,0);
        sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);

        rtp->state.last_timestamp = B_RTP_PKT_TIMESTAMP(rtp_pkt);
        rtp->state.next_sequence = B_RTP_NEXT_SEQUENCE(sequence);
        rtp->state.last_ssrc = B_RTP_LOAD32(rtp_pkt->data, 8);
        cc = B_RTP_GET_BITS(word0, 4, 7);
        offset = 12+cc*4;
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
            brtp_spf_pkt_info *info;
            info = BFIFO_WRITE(&rtp->state.pkt_info_fifo);
            info->timestamp = rtp->state.last_timestamp;
            info->ssrc = rtp->state.last_ssrc;
            info->sequence = sequence;
            info->packet_size = pkt_len;
            BFIFO_WRITE_COMMIT(&rtp->state.pkt_info_fifo, 1);
        }
        if (rtp->pipe_out==NULL) {
            goto discard;
        }

        batom_cursor_from_atom(&cursor, rtp_pkt->cookie);

        ext_len = 0;
        base = (unsigned long)((uint8_t *)cursor.vec->base)+offset;
        if (B_RTP_GET_BIT(word0, 3)) {
            rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)base, 4); /* copy header extension, reusing the rtp_pkt->data buffer */
            ext_len = (B_RTP_LOAD16(rtp_pkt->data, 2)*4)+4;
        }
        base += ext_len;

        BDBG_ASSERT(rtp->cfg.factory);
        atom = batom_from_range(rtp->cfg.factory,
                                (void*)base,
                                cursor.vec->len-(offset+ext_len),
                                &b_rtp_atom_user,
                                &rtp_pkt->cookie);
        batom_pipe_push(rtp->pipe_out, atom);
        BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
        goto next;
discard:
        BDBG_MSG(("b_rtp_process_pkt: %#lx discarding pkt", (unsigned long)rtp));
        batom_release(rtp_pkt->cookie);
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
b_rtp_reorder_pkt(brtp_spf_t rtp, brtp_packet_t rtp_pkt)
{
    int diff;
    unsigned sequence;
    brtp_packet_t prev,p;

    sequence = B_RTP_PKT_SEQUENCE(rtp_pkt);
    diff = b_rtp_uint16_diff(sequence,rtp->state.next_sequence);
    if (diff<0) { /* old packet ingore it */
        if (rtp->state.old_cnt<100) {
            BDBG_MSG(("b_rtp_reorder_pkt: %#lx(%u) discarding old packet %u:%u %d",(unsigned long)rtp, rtp->state.old_cnt, sequence, rtp->state.next_sequence, diff));
            rtp->state.old_cnt++;
            goto discard;
        } else { /* don't discard more than 100 packets in the row */
            BDBG_MSG(("b_rtp_reorder_pkt: %#lx resyncing sequence packet %u:%u %d",(unsigned long)rtp, sequence, rtp->state.next_sequence, diff));
            b_rtp_process_pkt(rtp, rtp_pkt);
            rtp->state.old_cnt = 0; /* reset old_cnt */
            goto done;
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
    /* TODO: fudge clock_rate for now */
    if (diff > (int)(rtp->cfg.wait_time*90000)/(1000*2) /* last packet stayed too late */
        || diff < 0 ||
        (diff==0 && rtp->state.last_timestamp==0)) {
        p = BLST_SQ_FIRST(&rtp->reordering);
        BDBG_ASSERT(p);
        BDBG_MSG(("b_rtp_reorder_pkt: %#lx applying pkt %u->%u (%d,%d)", (unsigned long)rtp, rtp->state.next_sequence, B_RTP_PKT_SEQUENCE(p), b_rtp_uint16_diff(B_RTP_PKT_SEQUENCE(p), rtp->state.next_sequence), diff));
        BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
        b_rtp_process_pkt(rtp, p);
    }
done:
    return;
discard:
    BDBG_MSG(("b_rtp_reorder_pkt: %#lx discarding pkt", (unsigned long)rtp_pkt));
    batom_release(rtp_pkt->cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    return;
}

static brtp_spf_enqueue
brtp_spf_enqueue_pkt(brtp_spf_t rtp, const void *pkt, size_t pkt_len, void *cookie)
{
    uint32_t word0, word1;
    unsigned cc;
    unsigned sequence;
    brtp_packet_t rtp_pkt;

    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);

    rtp_pkt = BLST_SQ_FIRST(&rtp->free);
    if (!rtp_pkt) {
        while(NULL!=(rtp_pkt=BLST_SQ_FIRST(&rtp->reordering))) {
            batom_release(rtp_pkt->cookie);
            BLST_SQ_REMOVE_HEAD(&rtp->reordering, queue);
            BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
        }
        rtp_pkt = BLST_SQ_FIRST(&rtp->free);
        rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)pkt, 12);
        word0 = B_RTP_LOAD32(rtp_pkt->data,0);
        sequence = B_RTP_GET_BITS(word0, 16, 31);
        rtp->state.next_sequence = sequence;
        BDBG_MSG(("brtp_spf_enqueue_pkt: %#lx overflow for pkt %#lx", (unsigned long)rtp, (unsigned long)pkt));
    }
    BLST_SQ_REMOVE_HEAD(&rtp->free, queue);

    /* there is shall be a complete header */
    if (pkt_len <= 12) { goto invalid_pkt; }

    /* copy RTP header to a 12-byte buffer (rtp_pkt->data), then use B_RTP_LOAD32 to access fields */
    rtp->cfg.copy_to_user((void*)rtp_pkt->data, (void*)pkt, 12);

    word0 = B_RTP_LOAD32(rtp_pkt->data,0);
    word1 = B_RTP_LOAD32(rtp_pkt->data,4);

    /* version shall be equal to 2 */
    if (B_RTP_GET_BITS(word0, 0, 1) != 2) { goto invalid_pkt; }
    cc = B_RTP_GET_BITS(word0, 4, 7);
    /* now we know complete header size */
    if (pkt_len <= 12+cc*4) { goto invalid_pkt; }

    if(rtp->pipe_out==NULL) {
        BDBG_MSG(("brtp_spf_enqueue_pkt: %#lx %#lx premature pkt %u ssrc:%#x", (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_LOAD32(rtp_pkt->data, 8)));
        goto discard_pkt;
    }

    BDBG_MSG_FLOW(("%#x brtp_spf_enqueue_pkt: %#lx packet:%#lx:%u type:%u ssrc:%#x ts:%u seq:%u(%u)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), B_RTP_LOAD32(rtp_pkt->data, 8), word1, (unsigned)B_RTP_GET_BITS(word0, 16, 31), (unsigned)rtp->state.next_sequence));
    if (
        ((B_RTP_GET_BITS(word0, 9, 15) & rtp->session_cfg.pt_mask) !=  rtp->session_cfg.pt_value) ||
        (B_RTP_LOAD32(rtp_pkt->data, 8) & rtp->session_cfg.ssrc_mask) != rtp->session_cfg.ssrc_value) {
        BDBG_MSG(("%#x brtp_spf_enqueue_pkt: %#lx packet:%#lx:%u filtered out type:%u(%u) ssrc:%#x(%#x)", word0, (unsigned long)rtp, (unsigned long)pkt, pkt_len, B_RTP_GET_BITS(word0, 9, 15), rtp->session_cfg.pt_value, B_RTP_LOAD32(rtp_pkt->data, 8), rtp->session_cfg.ssrc_value));
        goto discard_pkt;
    }

    rtp_pkt->len = pkt_len;
    rtp_pkt->cookie = cookie;

    sequence = B_RTP_GET_BITS(word0, 16, 31);
    if (sequence == rtp->state.next_sequence ||  /* match */
            (rtp->state.next_sequence == 0 && BLST_SQ_FIRST(&rtp->reordering)==NULL) /* first packet, uninitialized next_sequence */
            ) {
        /* fast path */
        b_rtp_process_pkt(rtp, rtp_pkt);
    } else {
        BDBG_MSG(("brtp_spf_enqueue_pkt: %#lx reordering %u %u", (unsigned long)rtp, sequence, rtp->state.next_sequence));
        /* add into the reordering queue */
        b_rtp_reorder_pkt(rtp, rtp_pkt);
    }
    return brtp_spf_enqueue_queued;

discard_pkt:
    batom_release(cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    BDBG_MSG(("brtp_spf_enqueue_pkt: %#lx discarding pkt %#lx:%u",  (unsigned long)rtp, (unsigned long)pkt, (unsigned)pkt_len));

    return brtp_spf_enqueue_discard;
    return brtp_spf_enqueue_overflow;
invalid_pkt:
    batom_release(cookie);
    BLST_SQ_INSERT_HEAD(&rtp->free, rtp_pkt, queue);
    BDBG_MSG(("brtp_spf_enqueue_pkt: %#lx discarding pkt %#lx:%u",  (unsigned long)rtp, (unsigned long)pkt, (unsigned)pkt_len));
    return brtp_spf_enqueue_invalid;
}

brtp_spf_enqueue
brtp_spf_feed(brtp_spf_t rtp, batom_pipe_t pipe_in) {
    int i;
    batom_t atom;
    batom_cursor cursor;
    brtp_spf_enqueue enqueue_result = brtp_spf_enqueue_queued;

    for(i=0;;) {
        if(batom_pipe_peek(pipe_in)) {
            atom = batom_pipe_pop(pipe_in);
            BDBG_ASSERT(atom);
            batom_cursor_from_atom(&cursor, atom);
#if 0
            {
                uint32_t udata = *(void**)batom_userdata(atom);
                BDBG_MSG_FLOW(("brtp_spf_push() pipe1, pop: (0x%x) (0x%x)", atom, udata));
            }
#endif
            enqueue_result = brtp_spf_enqueue_pkt(rtp, (const void *)cursor.vec->base, /*size_t pkt_len*/cursor.vec->len, /*void *cookie*/atom);
            if(enqueue_result != brtp_spf_enqueue_queued) { BDBG_MSG_FLOW(("brtp_spf_enqueue_pkt() returned: %d", enqueue_result)); }
        }
        else {
            /* pipe is empty */
            break;
        }
    }

    return enqueue_result;
}

void brtp_spf_start(brtp_spf_t rtp, batom_pipe_t pipe, const brtp_spf_session_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);

    BDBG_ASSERT(pipe);
    BDBG_ASSERT(cfg);

    rtp->session_cfg = *cfg;
    rtp->pipe_out = pipe;
    return;
}

void
brtp_spf_stop(brtp_spf_t rtp)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);
    b_rtp_reset(rtp);
}

brtp_spf_t
brtp_spf_create(const brtp_spf_cfg *cfg)
{
    brtp_spf_t rtp;

    BDBG_ASSERT(cfg);

    rtp = BKNI_Malloc(sizeof(*rtp)+cfg->npackets*sizeof(struct brtp_packet));
    if (!rtp) {
        BDBG_ERR(("out of memory in brtp_spf_create"));
        return NULL;
    }
    BDBG_OBJECT_INIT(rtp, brtp_spf_t);

    {
        unsigned i;
        uint8_t *hdr_buf = BKNI_Malloc(sizeof(uint8_t)*12*cfg->npackets);
        if (!hdr_buf) {
            BDBG_ERR(("out of memory in brtp_spf_create"));
            BKNI_Free(rtp);
            return NULL;
        }

        for(i=0;i<cfg->npackets;i++) {
            rtp->packets[i].data = hdr_buf+(i*12);
        }

        if (i==0) {
            if (hdr_buf)
                BKNI_Free(hdr_buf);
        }
    }

    rtp->cfg = *cfg;
    rtp->session_cfg = b_rtp_default_session_cfg;
    rtp->pipe_out = NULL;
    BLST_SQ_INIT(&rtp->reordering);
    b_rtp_reset(rtp);
    return rtp;
}

void
brtp_spf_destroy(brtp_spf_t rtp)
{
    BKNI_Free((void*)rtp->packets[0].data);
    BDBG_OBJECT_DESTROY(rtp, brtp_spf_t);
    BKNI_Free((void*)rtp);
    return;
}

int
brtp_spf_get_pkt_info(brtp_spf_t rtp, brtp_spf_pkt_info *info, size_t max_entries)
{
    unsigned to_read;

    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);

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

int brtp_get_stats(brtp_spf_t rtp, brtp_stats *stats)
{
    BDBG_OBJECT_ASSERT(rtp, brtp_spf_t);
    if (stats == NULL) return -1;
        *stats = rtp->stats;
        return 0;
}
#endif
