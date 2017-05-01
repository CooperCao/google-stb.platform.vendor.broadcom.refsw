/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * GOP player module for the on the fly PVR
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "blst_squeue.h"

#include "botf_gop_player.h"
#include "botf_priv.h"
#include "botf_platform.h"
#include "botf_itb_feeder.h"

BDBG_MODULE(botf_gop_player);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */
#define BDBG_MSG_BTP(x) /* BDBG_MSG(x) */
#define BDBG_MSG_FRAME(x) /* BDBG_MSG(x) */

#define B_OTF_ENABLE_PICTAG          1

#define B_INVALID_PTS 0

#define B_CMD_BUILD_REFERENCE	2
#define B_CMD_DISPLAY_FWD	    5
#define B_CMD_DISPLAY_BACK	    6
#define B_CMD_DISPLAY_PAST	    7
#define B_CMD_DISPLAY_FUTURE    8
#define B_CMD_PICTURE_TAG	    13

#define B_CMD_NOCMD             0xFF /* No command to inject */


#define B_ITB_TERMINATION   0x70
#ifdef UNIFIED_ITB_SUPPORT
#define B_ITB_BTPENTRYTYPE  0x23
#else
#define B_ITB_BTPENTRYTYPE  0x05
#endif

#define B_BTPCMD_SIZE		0x10
#define B_MAXFRAMEITBSIZE   (5*1024)

#define B_INVALID_CHUNK_NO  (0xFFFFFFFF)


struct bpvr_gop_entry {
	BLST_SQ_ENTRY(bpvr_gop_entry) queue;
	bpvr_gop_desc *gop; /* GOP in the VBV buffer */	
};

typedef struct bpvr_cp_entry {
    BLST_SQ_ENTRY(bpvr_cp_entry) queue;
    const void *itb_cp; /* Check point contains ITB address */   
    const void *cdb_cp; /* Check point contains CDB address */   
}bpvr_cp_entry;

BLST_SQ_HEAD(bpvr_gop_queue, bpvr_gop_entry);
BLST_SQ_HEAD(bpvr_cp_queue, bpvr_cp_entry);

/* sequence to display frame while in trick mode */
struct bpvr_gop_play_seq {    
	uint32_t BTPStartCmd[B_SCV_LEN/4];  /* Command inserted before feeding frame data */
#if B_OTF_ENABLE_PICTAG
    uint32_t BTPPicTagCmd[B_SCV_LEN/4];
#endif
	uint32_t ptsCmd[B_SCV_LEN/4];
    bool bNeedDummyFrame;
    /* Command inserted after feeding frame data 
     * B_CMD_NOCMD will indicate not to inject any command.
     */
    uint32_t BTPEndCmd;
	struct {
		bool pic_tag;
        uint8_t play_index; /* Index of frame in the origianl GOP(display order) */
	    int16_t index;  /* Index of frame in the origianl GOP(decode order) */
	} frames[BOTF_GOP_MAX_FRAMES], filtered_frames[BOTF_GOP_MAX_FRAMES];
	unsigned n_frames;
    unsigned filtered_nframes;
	unsigned cur_frame;
};

/* picture information */
struct bpvr_pic_entry {
	BLST_SQ_ENTRY(bpvr_pic_entry) queue;
	bpvr_pic_info info;
};
BLST_SQ_HEAD(bpvr_pic_queue, bpvr_pic_entry);

struct bpvr_gop_player_impl {
	void *cb_cntx;
    void *otf_cntx;
	bpvr_gop_done gop_done_cb;

	enum {
		bgop_waiting=0, /* waiting for new frame */
		bgop_feed_meta=1,  /* feeding meta data */
		bgop_feed_frame=2,  /* feeding frame */
		bgop_feed_sequence=3, /* feeding sequence */
		bgop_wait_2nd_gop=4, /* waiting for the second GOP */
        bgop_play_gop=5  /* Play the GOP */
	} state;    	
	/*  NOTE, GOP could be only at one queue at the time */
	struct bpvr_gop_queue active; /* GOP's currently being processed */
	struct bpvr_gop_queue complete; /* GOP's already processed and waiting to be consumed */
	struct bpvr_gop_queue free; /* pool of free GOP's */
    struct bpvr_cp_queue cp_complete; /* Checkpoints associated with complete GOPs */
    struct bpvr_cp_queue cp_free; /* Free Checkpoints */
	struct bpvr_gop_play_seq play_seq; /* display frame player sequence */
	struct bpvr_gop_play_seq gop_play; /* plan to play entire gop */
	struct bpvr_gop_entry *prev_gop; /* previous GOP, used in the fast forward */	
	unsigned disp_frames; /* number of frames to display */
	int reference_frame; /* frame number in the reference buffer */
	int past_predictor;  /* frame in the past predictor buffer */
    int future_predictor; /* frame in the future predictor buffer */
	unsigned disp_count, frame_count; /* counters used in the rate calculation */
    unsigned block_count; /* number of blocks sent to the decoder (used for debug purposes) */
	bpvr_gop_player_mode mode;
	
    botf_itb_feeder itb_feeder;
    BOTF_ParserPtrs *OPParserPtrs;
    const BOTF_ParserPtrs *IPParserPtrs;
    bpvr_start_code prevscode_end;
    bool    bprevscodeend_valid;
    bool    play_IFrame; 
    bool    bStop;
    uint32_t chunk_no;
    uint32_t chunk_frame_cntr;  /* Number of frames presented in this chunk */
    uint32_t dispFutureCmd[B_SCV_LEN/4];
    uint32_t dispPastCmd[B_SCV_LEN/4];
    uint32_t dispBackCmd[B_SCV_LEN/4];
	struct bpvr_gop_entry gops_storage[B_PVR_MAX_GOPS]; /* placeholder for the GOP entries */
    bpvr_cp_entry cp_storage[B_PVR_MAX_GOPS]; /* Storage for checkpoints */
};


static void b_pvr_gop_start_play(bpvr_gop_player player);
static void b_pvr_gop_player_buffer(bpvr_gop_player player);
static void bpvr_make_btp_cmd(uint32_t *buf, unsigned command,  unsigned skip_cnt, unsigned display_cnt, unsigned pts);
static void bpvr_make_pts_cmd(uint32_t *buf, unsigned pts);
static void b_pvr_gop_feed_segment(bpvr_gop_player player, const bpvr_itb_segment *segment);

#define B_OTF_STEP_BY_STEP  0
#if B_OTF_STEP_BY_STEP
#include <stdio.h>
#define B_OTF_WAIT_KEY(x) do {BDBG_WRN(x);getchar();getchar();}while(0)
#else
#define B_OTF_WAIT_KEY(x) 
#endif

#if BDBG_DEBUG_BUILD
static const char *
b_pvr_pic_type(unsigned pic_type)
{
    switch(pic_type) {
    case B_FRAME_I: return "I";
    case B_FRAME_P: return "P";
	case B_FRAME_B: return "B";
    default: return "INVAL";
    }
}
#endif

bpvr_gop_player 
bpvr_gop_player_create(
		bpvr_gop_done done_cb,
		void *cb_cntx,
        void *otf_cntx,
        struct BOTF_ParserPtrs *OPParserPtrs,
        const struct BOTF_ParserPtrs *IPParserPtrs
		)
{
	bpvr_gop_player  player;

	player = BKNI_Malloc(sizeof(*player));
	if (!player) { goto err_alloc; }

	BKNI_Memset(player, 0, sizeof(*player));
	player->gop_done_cb = done_cb;
	player->cb_cntx = cb_cntx;
    player->otf_cntx = otf_cntx;
    player->OPParserPtrs = OPParserPtrs;
    player->IPParserPtrs = IPParserPtrs;
    player->itb_feeder = botf_itb_feeder_create(player->IPParserPtrs);
    if(player->itb_feeder==NULL) {  goto err_feeder; }

	bpvr_gop_player_reset(player);

    bpvr_make_btp_cmd(player->dispFutureCmd, B_CMD_DISPLAY_FUTURE,  0, 0, 0);
    bpvr_make_btp_cmd(player->dispPastCmd, B_CMD_DISPLAY_PAST,  0, 0, 0);
    bpvr_make_btp_cmd(player->dispBackCmd, B_CMD_DISPLAY_BACK,  0, 0, 0);

	return player;
err_feeder:
    BKNI_Free(player);
err_alloc:
    return NULL;
}


void 
bpvr_gop_player_start(bpvr_gop_player player)
{
    player->bStop = false;
	return;
}

void bpvr_gop_player_stop(bpvr_gop_player player)
{

    player->bStop = true;
    return;
}

void 
bpvr_gop_player_reset(bpvr_gop_player player)
{
	unsigned i;

	BLST_SQ_INIT(&player->active);
	BLST_SQ_INIT(&player->free);
	BLST_SQ_INIT(&player->complete);
    BLST_SQ_INIT(&player->cp_free);
    BLST_SQ_INIT(&player->cp_complete);

	player->state = bgop_waiting;	
	player->disp_frames=0;
	player->reference_frame=-1;
	player->past_predictor=-1;
    player->future_predictor=-1;
    player->prev_gop = NULL; 

	player->frame_count = 0;
	player->disp_count = 0;
    player->block_count = 0;


	player->mode.rewind = false;
	player->mode.rate_control = bpvr_rate_control_frame;
	player->mode.frame_filter = BOTF_DISPLAY_I|BOTF_DISPLAY_P|BOTF_DISPLAY_B;
	player->mode.rate = 1 * BOTF_PLAY_RATE_MODULO;
	player->mode.pts_atom  = 1501; /* for NTSC 29.97 FPS */	

    player->play_IFrame = false;

    player->bprevscodeend_valid = false;

    player->chunk_no = B_INVALID_CHUNK_NO;

	for(i=0;i<B_PVR_MAX_GOPS;i++) {
		BLST_SQ_INSERT_HEAD(&player->free, &player->gops_storage[i], queue);
	}
    for(i=0;i<B_PVR_MAX_GOPS;i++) {
        BLST_SQ_INSERT_HEAD(&player->cp_free, &player->cp_storage[i], queue);
    }
	return;
}

void 
bpvr_gop_player_destroy(
		bpvr_gop_player player
	)
{
    botf_itb_feeder_destroy(player->itb_feeder);
	BKNI_Free(player);
	return;
}

static void
bpvr_make_btp_cmd(uint32_t *buf, unsigned command,  unsigned skip_cnt, unsigned display_cnt, unsigned pts)
{
    buf[0] = (B_ITB_BTPENTRYTYPE << 24) | (command << 8) | (skip_cnt);
    buf[1] = display_cnt << 24;
    buf[2] = 0;
    buf[3] = pts;
    return;
}

static void
bpvr_make_pts_cmd(uint32_t *buf, unsigned pts)
{
    buf[0] = (B_SCV_TYPE_PTS<< 24);
    buf[1] = pts;
    buf[2] = 0;
    buf[3] = 0;
    return;
}

/* Injects data to decoder */
static void 
b_pvr_gop_feed_data_(bpvr_gop_player player, const void *buf, unsigned len)
{
    unsigned count = len;
    const uint8_t *src;
    uint8_t *destn;
   
    BDBG_ASSERT(len%B_SCV_LEN==0);
    src = buf;
    if (player->OPParserPtrs->ItbValidPtr == player->OPParserPtrs->ItbReadPtr)  {
        destn = player->OPParserPtrs->ItbValidPtr;
    } else {
        destn = player->OPParserPtrs->ItbValidPtr+1;
    }

    BDBG_MSG_TRACE(("feeding data, buf=%#x len=%#x destn=%#x", buf, len, destn));
    while(count > 0)
    {
        /* Make sure that at least 8 entries are available before calling botf_itb_feeder() as
         * it may write multiple ITB entries in one call.
         */
        if (destn > player->OPParserPtrs->ItbEndPtr - 8*B_SCV_LEN) {                       
            BDBG_MSG(("b_pvr_gop_feed_data: %p setting wrap %p %p", (void *)player, (void *)destn, player->OPParserPtrs->ItbEndPtr));
            player->OPParserPtrs->ItbWrapAroundPtr = destn - 1;
            BOTF_P_SetOPITBWrapPtr(player->otf_cntx);
            destn = player->OPParserPtrs->ItbStartPtr;
        }
        destn = botf_itb_feeder_copy(player->itb_feeder, destn, src);
        src = src + B_SCV_LEN;
        count -= B_SCV_LEN;
    }
    player->OPParserPtrs->ItbValidPtr = destn-1;
    return;
}

static void
b_pvr_gop_feed_one_entry(bpvr_gop_player player, const void *buf)
{
    b_pvr_gop_feed_data_(player, buf, B_SCV_LEN);
    return;
}

static void
b_pvr_gop_feed_copy_entries(bpvr_gop_player player, const void *buf, unsigned len)
{
    const uint8_t *src;
    size_t to_copy;

    for(src=buf,to_copy=0;len>= B_SCV_LEN;len-=B_SCV_LEN) {
        uint32_t word0 = ((const uint32_t *)(src+to_copy))[0];
        if( B_SCV_TYPE(word0)==B_SCV_TYPE_PTS) {
            if(to_copy>0) {
                b_pvr_gop_feed_data_(player, src, to_copy);
            }
            src += to_copy  + B_SCV_LEN;
            to_copy = 0;
            continue;
        }
        to_copy += B_SCV_LEN;
    }
    if(to_copy>0) {
        b_pvr_gop_feed_data_(player, src, to_copy);
    }
    return;
}


static void 
b_pvr_gop_add_termentry(bpvr_gop_player player, const void *end)
{
#ifdef UNIFIED_ITB_SUPPORT
    uint32_t baseaddress[4] = {0x20000000, 0, 0, 0};
#else
    uint32_t baseaddress[4] = {0x01000000, 0, 0, 0};
#endif
    /* XVD firmware always subtracts 3 and asssumes data is valid upto that point */
    static const uint32_t termination[4] = {0x70000000, 0, 0, 0};

    BDBG_MSG_TRACE(("termination=%#x",end));

    baseaddress[1] = botf_mem_paddr(player->IPParserPtrs->mem, end);

    b_pvr_gop_feed_one_entry(player, baseaddress);
    b_pvr_gop_feed_one_entry(player, termination);
    return;
}

static void
b_pvr_gop_feed_filler(bpvr_gop_player player, const struct bpvr_gop_entry *gop_entry)
{
    /* Create a segment which contains only one sc */
    bpvr_itb_segment segment;
    BDBG_ASSERT(gop_entry->gop);
    if(gop_entry->gop->sc_filler.cdb!=NULL) { /* valid entry */
        BDBG_ASSERT(gop_entry->gop->sc_filler.prevbaseentryptr);
        segment.start_scode = segment.end_scode = gop_entry->gop->sc_filler;
        segment.end_scode.itbentryptr = (const uint8_t *)segment.end_scode.itbentryptr + B_SCV_LEN;
        b_pvr_gop_feed_segment(player, &segment);
    } else {
        BDBG_WRN(("b_pvr_gop_feed_filler: %p invalid filler in gop:%p", (void *)player, (void *)gop_entry));
    }
    return;
}

/*
 * Feed only slice 1
 * segment points to frame slices info
 */
static void
b_pvr_gop_feed_slice1(bpvr_gop_player player, const bpvr_itb_segment *segment)
{
    /* Create a segment which contains only slice1 */
    bpvr_itb_segment slice1segment = *segment;
    const void *end_scode_cdb;
    BDBG_ASSERT(segment->start_scode.prevbaseentryptr);
    BDBG_ASSERT(segment->end_scode.prevbaseentryptr);
    slice1segment.end_scode.itbentryptr = (const uint8_t *)slice1segment.start_scode.itbentryptr + B_SCV_LEN;
    end_scode_cdb = (const uint8_t *)slice1segment.end_scode.cdb + 8;
    if ((uint8_t *)end_scode_cdb > (uint8_t *)player->IPParserPtrs->CdbEndPtr) {
        unsigned long end_scode_cdb_offset = 8 - (player->IPParserPtrs->CdbEndPtr - (const uint8_t *)slice1segment.end_scode.cdb);
        end_scode_cdb = player->IPParserPtrs->CdbStartPtr + end_scode_cdb_offset;
    }
    slice1segment.end_scode.cdb = end_scode_cdb;
    b_pvr_gop_feed_segment(player, &slice1segment);
    return;
}

static void
b_pvr_gop_feed_segment(bpvr_gop_player player, const bpvr_itb_segment *segment)
{
    const uint8_t *end;
    const uint8_t *start;

    /* In SVP, each ITB entry contains only one start code. */
    if (player->bprevscodeend_valid) {
        /* Check, if this is next to the previous itb entry. If so, don't need to inject termination
         * entry */
        if ((player->prevscode_end.itbentryptr != segment->start_scode.itbentryptr)) {
            BOTF_P_SetOPITBValidPtr(player->otf_cntx);
            b_pvr_gop_add_termentry(player, player->prevscode_end.cdb);
        }
    }
    BDBG_ASSERT(segment->start_scode.prevbaseentryptr);
    /* Feed base address entry */
    b_pvr_gop_feed_one_entry(player, segment->start_scode.prevbaseentryptr);

    start = segment->start_scode.itbentryptr;
    end = segment->end_scode.itbentryptr;

    if (end>start) {
        b_pvr_gop_feed_copy_entries(player, start, end - start);
    } else {
        b_pvr_gop_feed_copy_entries(player, start, (uint8_t *) player->IPParserPtrs->ItbWrapAroundPtr - start + 1);
        b_pvr_gop_feed_copy_entries(player,  player->IPParserPtrs->ItbStartPtr, end - (uint8_t *)player->IPParserPtrs->ItbStartPtr);
    }    

    player->prevscode_end = segment->end_scode;
    player->bprevscodeend_valid = true;


	return;
}

static bool
b_pvr_reserve_feed(bpvr_gop_player player)
{
    int size;

    if (player->bStop) return false;

    /* Calculate the available size in ITB */
    if (player->OPParserPtrs->ItbReadPtr > player->OPParserPtrs->ItbValidPtr) {
        size = player->OPParserPtrs->ItbReadPtr - player->OPParserPtrs->ItbValidPtr - 1;
    } else if (player->OPParserPtrs->ItbReadPtr < player->OPParserPtrs->ItbValidPtr ) {
        size = (player->OPParserPtrs->ItbEndPtr - player->OPParserPtrs->ItbValidPtr + player->OPParserPtrs->ItbReadPtr - player->OPParserPtrs->ItbStartPtr);
    } else {
        size = player->OPParserPtrs->ItbEndPtr + 1 - player->OPParserPtrs->ItbStartPtr;
    }
    /* As a ITB producer, we should consider the following. 
     *      Read points to the first byte not yet consumed by AVD.
     *      valid points to the last byte written.
     *      read = valid means buffer is empty.
     *      valid = read - 1 means buffer is full
     * Currently, I don't know whether ITB consumer(XVD) will take care of setting the read
     * pointer to be equal to valid when the buffer is empty. To avoid confusion,
     * subtract B_SCV_LEN to keep at least one entry distance from valid to read so
     * that valid never will be equal to read - 1.
     */
    size -= B_SCV_LEN;
    if (size < B_MAXFRAMEITBSIZE)
    {
        BDBG_MSG(("Space not available in destn rave, state=%d", player->state));        
		return false;
    }
    return true;
}

static unsigned
bpvr_gop_build_play_seq(bpvr_gop_player player, unsigned frame)
{
	struct bpvr_gop_entry *gop_entry;
	struct bpvr_gop_play_seq *seq=&player->play_seq;
	int i;
	unsigned skip_cnt, disp_cnt=1;
	bpvr_gop_desc *gop;

	gop_entry = BLST_SQ_FIRST(&player->active);
	BDBG_ASSERT(gop_entry);
	gop = gop_entry->gop;

	BDBG_ASSERT(frame<gop->n_frames);

	seq->cur_frame=0;
	seq->bNeedDummyFrame = false;
	switch(gop->frames[frame].info.pic_type) {
	case B_FRAME_I:
    case B_FRAME_P:
        if ((int)frame==player->past_predictor) {
            /* Use display past command to display past predictor */
            seq->n_frames=0;
            disp_cnt = 1;
            seq->BTPEndCmd = B_CMD_DISPLAY_PAST;
            break;
        }
        if ((int)frame==player->future_predictor) {
            /* Use display past command to display Future predictor */
            seq->n_frames=0;
            disp_cnt = 1;
            seq->BTPEndCmd = B_CMD_DISPLAY_FUTURE;
            break;
        }

        player->past_predictor = -1;
        player->future_predictor = -1;
        seq->n_frames=0;
        if ((int)frame==player->reference_frame) {
			seq->frames[seq->n_frames].index = player->reference_frame;
            seq->frames[seq->n_frames].pic_tag = true;
            seq->n_frames++;
        } else {
            for(i=0;i<=(int)frame;i++) {
                if (i>player->reference_frame  && gop->frames[i].info.pic_type!=B_FRAME_B) {
                    seq->frames[seq->n_frames].index = i;
                    seq->frames[seq->n_frames].pic_tag = true;
                    seq->n_frames++;
                }
            }
        }
        disp_cnt=1;
        skip_cnt = seq->n_frames;

        seq->BTPEndCmd = B_CMD_DISPLAY_FUTURE;
        BDBG_MSG_BTP(("BTP: prepate Display Forward %u:%u", skip_cnt, disp_cnt));
        bpvr_make_btp_cmd(seq->BTPStartCmd, B_CMD_DISPLAY_FWD,  skip_cnt, disp_cnt, 0);
        seq->bNeedDummyFrame = true;        

        if (seq->n_frames>=2) {
            player->past_predictor = seq->frames[seq->n_frames-2].index;
            player->future_predictor = seq->frames[seq->n_frames-1].index;
        }
        BDBG_MSG(("new past predictor is %d future predictor is %d", player->past_predictor, player->future_predictor));

        break;

	case B_FRAME_B:
		seq->n_frames=0;
		seq->cur_frame=0;
		player->past_predictor=-1;
        player->future_predictor=-1;
        for(i=0;i<=(int)frame;i++) {
            if (i>player->reference_frame) {
                if(gop->frames[i].info.pic_type!=B_FRAME_B) {
                    seq->frames[seq->n_frames].index = i;
                    seq->frames[seq->n_frames].pic_tag = true;
                    seq->n_frames++;
                }
            }
        }

        if (seq->n_frames>=1) {
            player->future_predictor = seq->frames[seq->n_frames-1].index;
        }

        if(seq->n_frames>=2) {
            player->past_predictor=seq->frames[seq->n_frames-2].index;            
        }

        BDBG_MSG(("new past predictor is %d future predictor is %d", player->past_predictor, player->future_predictor));

        if ( (player->reference_frame < 0 && seq->n_frames<2) || seq->n_frames<1) {
            BDBG_MSG(("detected B frame %d from an open GOP", frame));
            seq->n_frames=0;            
            seq->BTPEndCmd = B_CMD_NOCMD;
            return 0;
        }

        skip_cnt = seq->n_frames;
        if (player->mode.rewind) {
            for(disp_cnt=0,i=(int)frame;i>=0;i--) {
                if(gop->frames[i].info.pic_type==B_FRAME_B) {
                    unsigned j;
                    bool display=false;

                    /* only display B frames what we should display */
                    for(j=0;j<player->gop_play.n_frames;j++) {
                        if(player->gop_play.frames[j].index==i) {
                            display=true;
                            break;
                        }
                    }
                    if (display) {
                        seq->frames[seq->n_frames].index = i;
                        seq->frames[seq->n_frames].pic_tag = true;
                        seq->n_frames++;
                        disp_cnt++;
                    }
                } else {
                    break;
                }
            }
        } else {
            for(disp_cnt=0,i=(int)frame;i<(int)gop->n_frames;i++) {
                if(gop->frames[i].info.pic_type==B_FRAME_B) {
                    unsigned j;
                    bool display=false;

                    /* only display B frames what we should display */
                    for(j=0;j<player->gop_play.n_frames;j++) {
                        if(player->gop_play.frames[j].index==i) {
                            display=true;
                            break;
                        }
                    }
                    if (display) {
                        seq->frames[seq->n_frames].index = i;
                        seq->frames[seq->n_frames].pic_tag = true;
                        seq->n_frames++;
                        disp_cnt++;
                    }
                } else {
                    break;
                }
            }
        }
        BDBG_ASSERT(disp_cnt>0);
        seq->BTPEndCmd = B_CMD_NOCMD;
        BDBG_MSG_BTP(("BTP: prepate Display Forward %u:%u", skip_cnt, disp_cnt));
        bpvr_make_btp_cmd(seq->BTPStartCmd, B_CMD_DISPLAY_FWD, skip_cnt, disp_cnt, 0);
        break;
    default:
        BDBG_WRN(("Invalid frame type %d, ignore",gop_entry->gop->frames[frame].info.pic_type));
        disp_cnt = 1;
        seq->BTPEndCmd = B_CMD_NOCMD;
        seq->n_frames=0;        
        break;
    }
    return disp_cnt;
}

/* build sequence to decode B frames from the open GOP */
static unsigned
b_pvr_build_cross_gop(bpvr_gop_player player, bpvr_gop_desc *gop, bpvr_gop_desc *next, int frame)
{
	int i;
	struct bpvr_gop_play_seq *seq=&player->play_seq;
	unsigned skip_cnt, disp_cnt;

	BDBG_MSG(("cross_gop %p:%p frame (%d:%d:%d,%d:%d)", (void *)gop, (void *)next, frame, gop->n_frames, gop->chunk_no, next?next->n_frames:0, next?next->chunk_no:0));
	seq->n_frames=0;
	seq->cur_frame=0;
	if (next==NULL || !next->flags.complete || gop->flags.discontinuity || next->flags.discontinuity || (player->mode.rewind && gop->chunk_no > next->chunk_no) || (!player->mode.rewind && gop->chunk_no < next->chunk_no)) {
		
		/* previous GOP is not completed one, so we have to skip playback of open GOP B frames */
		unsigned n_frames=0;
		if (player->mode.rewind) {
			for(disp_cnt=0,i=frame;i>=0;i--) {
				if(gop->frames[i].info.pic_type==B_FRAME_B) {
					seq->frames[n_frames].index = i;
					seq->frames[n_frames].pic_tag = true;
					n_frames++;
					disp_cnt++;
				} else {
					break;
				}
			}
		} else {
			for(disp_cnt=0,i=frame;i<(int)gop->n_frames;i++) {
				if(gop->frames[i].info.pic_type==B_FRAME_B) {
					seq->frames[n_frames].index = i;
					seq->frames[n_frames].pic_tag = true;
					disp_cnt++;
					n_frames++;
				} else {
					break;
				}
			}
		}
		BDBG_MSG(("aborting playback of %d open GOP B frames", disp_cnt));
		return disp_cnt;
	}

	/* add I and P frames from the next GOP */
	for(i=0;i<(int)next->n_frames;i++) {
		if(next->frames[i].info.pic_type!=B_FRAME_B) {
			seq->frames[seq->n_frames].index = -i - 1;
			seq->frames[seq->n_frames].pic_tag = true;
			seq->n_frames++;
		}
	}
	/* add I frame from the current GOP */
	for(i=0;i<frame;i++) {
		if(gop->frames[i].info.pic_type!=B_FRAME_B) {
			seq->frames[seq->n_frames].index = i;
			seq->frames[seq->n_frames].pic_tag = true;
			seq->n_frames++;
		}
	}
	skip_cnt = seq->n_frames;

	/* add B frames from the current GOP */
	if (player->mode.rewind) {
		for(disp_cnt=0,i=frame;i>=0;i--) {
			if(gop->frames[i].info.pic_type==B_FRAME_B) {
				unsigned j;
				bool display=false;

				/* only display B frames what we should display */
				for(j=0;j<player->gop_play.n_frames;j++) {
					if(player->gop_play.frames[j].index==i) {
						display=true;
						break;
					}
				}
				if (display) {
					seq->frames[seq->n_frames].index = i;
					seq->frames[seq->n_frames].pic_tag = true;
					seq->n_frames++;
					disp_cnt++;
				}
			} else {
				break;
			}
		}
	} else {
		for(disp_cnt=0,i=frame;i<(int)gop->n_frames;i++) {
			if(gop->frames[i].info.pic_type==B_FRAME_B) {
				unsigned j;
				bool display=false;

				/* only display B frames what we should display */
				for(j=0;j<player->gop_play.n_frames;j++) {
					if(player->gop_play.frames[j].index==i) {
						display=true;
						break;
					}
				}
				if (display) {
					seq->frames[seq->n_frames].index = i;
					seq->frames[seq->n_frames].pic_tag = true;
					seq->n_frames++;
					disp_cnt++;
				}
			} else {
				break;
			}
		}
	}
	BDBG_ASSERT(disp_cnt>0);
    seq->BTPEndCmd = B_CMD_NOCMD;
    BDBG_MSG_BTP(("BTP: prepate Display Forward %u:%u", skip_cnt, disp_cnt));
	bpvr_make_btp_cmd(seq->BTPStartCmd, B_CMD_DISPLAY_FWD, skip_cnt, disp_cnt, 0);    
	return disp_cnt;
}

static void b_pvr_gop_next_frame(bpvr_gop_player player);

/* this function build sequence to play an entire gop, basically it rearanges frames in the reverse display order */
static void
b_pvr_gop_build_gop_rew_seq(bpvr_gop_desc *gop, struct bpvr_gop_play_seq *gop_play)
{
	int frame, i, last_ref;

	BKNI_Memset(gop_play->frames,0,sizeof(gop_play->frames));

	gop_play->cur_frame=0;

	if(0 && !gop->flags.complete) {
		/* use I frame only */
		BDBG_MSG(("incomplete GOP %px, using I frame only", (void *)gop));
	  	for(frame=0;frame<(int)gop->n_frames;frame++) {
			if (gop->frames[frame].info.pic_type == B_FRAME_I) {
				gop_play->frames[0].index = frame;
				gop_play->frames[0].pic_tag = true;
				gop_play->n_frames=1;
				return;
			}
		}
		BDBG_WRN(("No I frames found in the GOP %p", (void *)gop));
		gop_play->n_frames=0;
		return;
	}

  	for(frame=gop->n_frames-1,i=1, last_ref=0;frame>0;frame--,i++) {
		switch(gop->frames[frame].info.pic_type) {
		default:
			BDBG_WRN(("Unknown frame type %d",gop->frames[frame].info.pic_type));
			/* fallthrough */
		case B_FRAME_I:
		case B_FRAME_P:
			gop_play->frames[last_ref].index = frame;
			gop_play->frames[last_ref].pic_tag = true;
			last_ref = i;
			break;
		case B_FRAME_B:
			gop_play->frames[i].index = frame;
			gop_play->frames[i].pic_tag = true;
			break;
		}
	}
	gop_play->n_frames=i;
	for(i=0;i<(int)gop_play->n_frames;i++) {
		BDBG_MSG_TRACE(("dispframes %d:%d:%d", i, gop->frames[gop_play->frames[i].index].info.pic_type, gop_play->frames[i].index));
	}
	return;
}

/* this function build sequence to play an entire gop, basically it rearanges frames in the display order */
static void
b_pvr_gop_build_gop_fwd_seq(bpvr_gop_desc *gop, struct bpvr_gop_play_seq *gop_play)
{
	unsigned frame, i, last_ref;

	BKNI_Memset(gop_play->frames,0,sizeof(gop_play->frames));

	gop_play->cur_frame=0;

	if(0 && !gop->flags.complete) {
		/* use I frame only */
		BDBG_MSG(("Incomplete GOP %p, using I frame only", (void *)gop));
	  	for(frame=0;frame<gop->n_frames;frame++) {
			if (gop->frames[frame].info.pic_type == B_FRAME_I) {
				gop_play->frames[0].index = frame;
				gop_play->frames[0].pic_tag = true;
				gop_play->n_frames=1;
				return;
			}
		}
		BDBG_WRN(("No I frames found in the GOP %p", (void *)gop));
		gop_play->n_frames=0;
		return;
	}

  	for(frame=0,i=1, last_ref=0;i<gop->n_frames;i++) {
		switch(gop->frames[i].info.pic_type) {
		default:
			BDBG_WRN(("Unknown frame type %d",gop->frames[frame].info.pic_type));
			continue;
		case B_FRAME_I:
		case B_FRAME_P:
			gop_play->frames[frame].index = last_ref;
			gop_play->frames[frame].pic_tag = true;
			frame++;
			last_ref = i;
			break;
		case B_FRAME_B:
			gop_play->frames[frame].index = i;
			gop_play->frames[frame].pic_tag = true;
			frame++;
			break;
		}
	}
	gop_play->frames[frame].index=last_ref;
	gop_play->frames[frame].pic_tag=true;
	frame++;
	gop_play->n_frames=frame;
	for(i=0;i<gop_play->n_frames;i++) {
		BDBG_MSG_TRACE(("dispframes %d:%d:%d", i, gop->frames[gop_play->frames[i].index].info.pic_type, gop_play->frames[i].index));
	}
	return;
}

/* this function modifies play sequence according to the current mode */
static void
b_pvr_gop_filter_seq(bpvr_gop_player player)
{
	struct bpvr_gop_entry *gop_entry;
	bpvr_gop_desc *gop;
	struct bpvr_gop_play_seq *gop_play = &player->gop_play;
	unsigned i, frame, filtered_index;
	unsigned mask;	
    unsigned start_frame;
    unsigned frame_count, disp_count;
    bool found_Iframe;

	gop_entry = BLST_SQ_FIRST(&player->active);
	BDBG_ASSERT(gop_entry);

	gop = gop_entry->gop;
    /*
     * if we are playing discontinuous, then the manager may have added some extra frames in to be accounted for before
     * playing this gop
     */
    if (gop->headless_gop_frame_count) {
        player->frame_count += gop->headless_gop_frame_count * BOTF_PLAY_RATE_MODULO;
        gop->headless_gop_frame_count = 0;
    }
	/* Filter out the frames to filtered_frames array based on the filter(I, IP or IPB) */
	for(frame=0,i=0;i<gop_play->n_frames;i++) {
        /* assign display order index */
        gop_play->frames[i].play_index = i;
		switch(gop->frames[gop_play->frames[i].index].info.pic_type) {
		case B_FRAME_I: mask = BOTF_DISPLAY_I;break;
		case B_FRAME_P: mask = BOTF_DISPLAY_P;break;
		default: case B_FRAME_B: mask = BOTF_DISPLAY_B;break;
		}	
		if ( (mask&player->mode.frame_filter) == mask) {
			gop_play->filtered_frames[frame++] = gop_play->frames[i];            
		}
	}
	gop_play->filtered_nframes = frame;
	if (frame==0) {
		BDBG_MSG(("skiping GOP %p because of frame filter", (void *)gop));
		return;
	}
	for(i=0;i<gop_play->filtered_nframes;i++) {
		BDBG_MSG_TRACE(("filtered frames %d:%d:%d", i, gop->frames[gop_play->filtered_frames[i].index].info.pic_type, gop_play->filtered_frames[i].index));
	}

	switch(player->mode.rate_control) {
	default:
	case bpvr_rate_control_frame:
        if ( player->frame_count == 0 || gop->chunk_no == B_MARKER_CHUNK_EOF) {
            player->play_IFrame = true;
        }

        if (player->play_IFrame) {
            /* If this is the first GOP we are playing or previous GOP wasn't complete, make sure to play I-Frame 
             * to oprimize decoder bandwidth.             
             */

            /* set frame_count and display_count to 0 so that I frame will not be filtered out */
            player->frame_count = 0;
            player->disp_count = 0;
            if (player->mode.rewind) {
                /* Move the start position until we align to I-frame */
                for(start_frame = 0; start_frame < gop_play->n_frames; start_frame++) {
                    frame_count = 0;
                    disp_count = 0;         
                    found_Iframe = false;
                    for (i=start_frame; i<gop_play->n_frames; i++) {
                        if ((frame_count >= disp_count)){
                            disp_count += player->mode.rate;
                            if (gop->frames[gop_play->frames[i].index].info.pic_type == B_FRAME_I) {
                                found_Iframe = true;
                                break;                            
                            }                                
                        }
                        frame_count += BOTF_PLAY_RATE_MODULO;            
                    }
                    if (found_Iframe) 
                        break;                            
                }
            } else {
                /* In forward case, just start from I-frame */
                start_frame = 0;
                for(start_frame = 0; start_frame < gop_play->n_frames; start_frame++) {                    
                    if (gop->frames[gop_play->frames[start_frame].index].info.pic_type == B_FRAME_I) {
                        break;
                    }                   
                }
            }            
        } else {
            start_frame = 0;
        }

        for(i=start_frame,frame=0,filtered_index=0;i<gop->n_frames;i++) {
            BDBG_MSG_TRACE(("filtered_index=%u i=%u frame_count=%u disp_count=%u",filtered_index,i,player->frame_count,player->disp_count));
            if (player->frame_count > player->frame_count + BOTF_PLAY_RATE_MODULO) {
                /* cheap correction of the overlfow problem */
                player->frame_count = 0;
                player->disp_count = 0;
            }			
            if (player->frame_count >= player->disp_count && !gop->flags.disp_iframe) {
                /*
                 * Following logic need to make sure that if there is no filtered frame where we are 
                 * supposed to display a frame, it will display previous filtered frame.
                 * For example, if the original GOP is 
                 * I0  B1  B2  P3  B4  B5  P6  B7  B8  P9  B10 B11 P12 B12 B14 (decode order) 
                 * B1  B2  I0  B4  B5  P3  B7  B8  P6  B10 B11 P9  B13 B14 P12 (display order, contents of gop_play )
                 *         I0          P3          P6          P9          P12 (filtered frames with IP filter)
                 * B1      I0      B5      B7      P6      B11     B13     P12 (rate 200)
                 * I0      I0      I0      P3      P6      P6      P9      P12 (atual display)
                 */
                if (filtered_index < gop_play->filtered_nframes-1) {
                    while( (gop_play->filtered_frames[filtered_index+1].play_index <= i) &&
                           (filtered_index < gop_play->filtered_nframes-1)) {
                        filtered_index++;
                    }
                }
                gop_play->frames[frame++] = gop_play->filtered_frames[filtered_index];
                BDBG_MSG_TRACE(("displayed frames, play index=%d", gop_play->filtered_frames[filtered_index].play_index));
                player->disp_count += player->mode.rate;
            }
            player->frame_count += BOTF_PLAY_RATE_MODULO;
        }
        break;
    }	
    /* There are no frames to display and display I frame flag is set by GOP player */
    if (frame==0 && gop->flags.disp_iframe) {
        for (filtered_index=0;filtered_index<gop_play->filtered_nframes;filtered_index++) {
            if (gop->frames[gop_play->filtered_frames[filtered_index].index].info.pic_type == B_FRAME_I) {
                gop_play->frames[frame++] = gop_play->filtered_frames[filtered_index];
            }
        }
    }
    gop_play->n_frames = frame;
    if (frame==0) {
        BDBG_MSG(("skiping GOP %p because of rate control", (void *)gop));
        return;
    }
    BDBG_MSG(("disp_count %u frame_count %u frames %d", player->disp_count, player->frame_count, gop_play->n_frames));

    /* If the current is not complete, then make sure to play I-frame
     * in next GOP as a means to protect decoder bandwidth.
     * Do not do this for I frame mode as there is no decoder bandwidth saving,
     * and the frame_count/display_count pacing is compromised by the resync code
     */
    if (!gop->flags.complete && player->mode.frame_filter != BOTF_DISPLAY_I)    
        player->play_IFrame = true;              
    else
        player->play_IFrame = false;
    for(i=0;i<gop_play->n_frames;i++) {
        BDBG_MSG(("dispframes[filt/rate] %d:%d:%d", i, gop->frames[gop_play->frames[i].index].info.pic_type, gop_play->frames[i].index));
    }
    player->chunk_frame_cntr += gop_play->n_frames;
	return;
}


static void
b_pvr_gop_next_seq(bpvr_gop_player player)
{
	struct bpvr_gop_entry *gop_entry;
	int frame;
    uint32_t cmd;

	gop_entry = BLST_SQ_FIRST(&player->active);
	BDBG_ASSERT(gop_entry);	
    while(player->play_seq.cur_frame<player->play_seq.n_frames) {        
        frame  = player->play_seq.frames[player->play_seq.cur_frame].index;
        BDBG_MSG_TRACE(("feeding (%d,%d) frame %d(%d) for GOP %#x", player->play_seq.cur_frame, player->play_seq.n_frames, frame, player->gop_play.cur_frame, gop_entry->gop));
        if (frame>=0) {			
			if (!b_pvr_reserve_feed(player)) { /* not enough space */
				BDBG_MSG(("b_pvr_gop_next_seq, Not enough space in feeder"));
				return;
			}
            BDBG_MSG_TRACE(("feeding frame(meta) %d for GOP %#x", frame, gop_entry->gop));            
            if (player->play_seq.cur_frame==0) {
                BDBG_MSG_TRACE(("feeding BTP command %#x frame %d for GOP %#x", player->play_seq.BTPStartCmd, frame, gop_entry->gop));
                BDBG_MSG_BTP(("BTP: feeding Start Cmd"));
                b_pvr_gop_feed_one_entry(player, player->play_seq.BTPStartCmd);
            }
#if B_OTF_ENABLE_PICTAG
            if (player->play_seq.frames[player->play_seq.cur_frame].pic_tag) {
                BDBG_MSG_BTP(("BTP: Picture Tag %#x", (unsigned)gop_entry->gop->frames[frame].info.pic_tag));
                bpvr_make_btp_cmd(player->play_seq.BTPPicTagCmd,B_CMD_PICTURE_TAG, 0, 0, gop_entry->gop->frames[frame].info.pic_tag);
                BDBG_MSG_TRACE(("feeding BTP command(tag) %#x(%#x)", player->play_seq.BTPPicTagCmd,gop_entry->gop->frames[frame].info.pic_tag ));
                b_pvr_gop_feed_one_entry(player, player->play_seq.BTPPicTagCmd);
            }
#endif
            if(gop_entry->gop->frames[frame].info.flags.pts) {
                bpvr_make_pts_cmd(player->play_seq.ptsCmd, gop_entry->gop->frames[frame].info.pts);
                BDBG_MSG_TRACE(("feeding PTS %#x(%u:%#x)", player->play_seq.ptsCmd, frame, gop_entry->gop->frames[frame].info.pts));
                b_pvr_gop_feed_one_entry(player, player->play_seq.ptsCmd);
            }
            BDBG_MSG_TRACE(("feeding frame(data) %d[%s] for GOP %#x", frame, b_pvr_pic_type(gop_entry->gop->frames[frame].info.pic_type), gop_entry->gop));
            BDBG_MSG_FRAME(("feeding frame(data) %d[%s] for GOP %#x", frame, b_pvr_pic_type(gop_entry->gop->frames[frame].info.pic_type), gop_entry->gop));
            b_pvr_gop_feed_segment(player, &(gop_entry->gop->frames[frame].itb_meta));  
            b_pvr_gop_feed_segment(player, &(gop_entry->gop->frames[frame].itb_slices));
        } else {
            /* negative frame id is used to indicate previous(next) GOP */
            struct bpvr_gop_entry *gop_next;

            if(player->mode.rewind) {
                gop_next = BLST_SQ_NEXT(gop_entry,queue);
                BDBG_ASSERT(gop_next);
            } else {
                gop_next = player->prev_gop;
                BDBG_ASSERT(gop_next);
            }

            frame = -frame - 1;			
			if (!b_pvr_reserve_feed(player)) { /* not enough space */
				BDBG_MSG(("b_pvr_gop_next_seq, Not enough space in feeder"));
				return;
			}            
            BDBG_MSG_TRACE(("feeding [prev]frame(meta) %d(%d) for GOP %#x", frame, player->gop_play.cur_frame, gop_next->gop));            
            if (player->play_seq.cur_frame==0) {
                BDBG_MSG_TRACE(("feeding BTP command %#x frame %d for GOP %#x", player->play_seq.BTPStartCmd, frame, gop_entry->gop));
                BDBG_MSG_BTP(("BTP: feeding Start Cmd"));
                b_pvr_gop_feed_one_entry(player, player->play_seq.BTPStartCmd);
            }
#if B_OTF_ENABLE_PICTAG
            if (player->play_seq.frames[player->play_seq.cur_frame].pic_tag) {
                BDBG_MSG_BTP(("BTP: Picture Tag %#x", (unsigned)gop_entry->gop->frames[frame].info.pic_tag));
                bpvr_make_btp_cmd(player->play_seq.BTPPicTagCmd,B_CMD_PICTURE_TAG, 0, 0, gop_next->gop->frames[frame].info.pic_tag);
                BDBG_MSG_TRACE(("feeding BTP command(tag) %#x(%#x)", player->play_seq.BTPPicTagCmd, gop_entry->gop->frames[frame].info.pic_tag));
                b_pvr_gop_feed_one_entry(player, player->play_seq.BTPPicTagCmd);
            }
#endif

            BDBG_MSG_TRACE(("feeding [prev]frame(data) %d(%d) for GOP %#x", frame, player->gop_play.cur_frame, gop_next->gop));
            b_pvr_gop_feed_segment(player, &(gop_next->gop->frames[frame].itb_meta));  
            b_pvr_gop_feed_segment(player, &(gop_next->gop->frames[frame].itb_slices));			
        }        
        player->play_seq.cur_frame++;
        /* B_OTF_WAIT_KEY(("Decode Frame completed")); */
    }
    	
    /* Feed the cmd at the end of play sequence */
    cmd = player->play_seq.BTPEndCmd;
    switch(cmd) {
    case B_CMD_NOCMD:
        break;
    default:
        break;
    case B_CMD_DISPLAY_FUTURE:
        BDBG_MSG_BTP(("BTP: display Future"));
        b_pvr_gop_feed_one_entry(player, player->dispFutureCmd);
        BDBG_MSG_FRAME(("feeding slice frame(data) %u[%s] for GOP %#x", 0, b_pvr_pic_type(gop_entry->gop->frames[0].info.pic_type), (unsigned)gop_entry->gop));
        b_pvr_gop_feed_segment(player, &gop_entry->gop->frames[0].itb_meta);
        b_pvr_gop_feed_slice1(player, &gop_entry->gop->frames[0].itb_slices);      
        /* This I-frame must be displayed, it may be last piece of data we feed to xvd.
         * XVD need another sc after this to push out I-frame to display.
         */
        if (gop_entry->gop->flags.disp_iframe) {
            b_pvr_gop_feed_filler(player, gop_entry);
            b_pvr_gop_feed_filler(player, gop_entry);
            BOTF_P_SetOPITBValidPtr(player->otf_cntx);
        }
        break;
    case B_CMD_DISPLAY_PAST:
        BDBG_MSG_BTP(("BTP: display Past"));
        b_pvr_gop_feed_one_entry(player, player->dispPastCmd);
        BDBG_MSG_FRAME(("feeding slice frame(data) %u[%s] for GOP %#x", 0, b_pvr_pic_type(gop_entry->gop->frames[0].info.pic_type), (unsigned)gop_entry->gop));
        b_pvr_gop_feed_segment(player, &gop_entry->gop->frames[0].itb_meta);
        b_pvr_gop_feed_slice1(player, &gop_entry->gop->frames[0].itb_slices);
        /* This I-frame must be displayed, it may be last piece of data we feed to xvd.
         * XVD need another sc after this to push out I-frame to display.
         */
        if (gop_entry->gop->flags.disp_iframe) {
            b_pvr_gop_feed_filler(player, gop_entry);
            b_pvr_gop_feed_filler(player, gop_entry);
            BOTF_P_SetOPITBValidPtr(player->otf_cntx);
        }
        break;
    }
    
    player->gop_play.cur_frame += player->disp_frames;
    player->state = bgop_feed_frame;
    b_pvr_gop_next_frame(player);

	return;
}

static void
b_pvr_gop_recycle(bpvr_gop_player player, struct bpvr_gop_entry *gop_entry)
{	
    bpvr_cp_entry *cp_entry;

    cp_entry = BLST_SQ_FIRST(&player->cp_free);
    BDBG_ASSERT(cp_entry);

    /* Each checkpoint is associated with one GOP in complete queue */
	BLST_SQ_REMOVE_HEAD(&player->cp_free, queue);    

    /* XVD updates the ITB read pointer in 128 byte chunks 
     * This means, even after consuming a GOP XVD may not advance the ITB read
     * pointer beyond the GOP. 
     * The following code sets the ITB check point to be 128 bytes before where 
     * we really need to set the check point. We will use this in combination of
     * CDB read pointer to make sure that XVD consumed the complete GOP or not.
     */
    cp_entry->itb_cp = player->OPParserPtrs->ItbValidPtr - 128;    
    if ((uint8_t *) cp_entry->itb_cp < player->OPParserPtrs->ItbStartPtr) {        
        cp_entry->itb_cp = player->OPParserPtrs->ItbWrapAroundPtr - 128 +
                            (player->OPParserPtrs->ItbValidPtr - player->OPParserPtrs->ItbStartPtr);        
    }
    cp_entry->cdb_cp = gop_entry->gop->lastcdbptr;
    BDBG_MSG(("CP Insert itb_cp=%p cdb_cp=%p", cp_entry->itb_cp, cp_entry->cdb_cp));
    BLST_SQ_INSERT_TAIL(&player->cp_complete, cp_entry, queue);
    BLST_SQ_INSERT_TAIL(&player->complete, gop_entry, queue);
    /* For last gop in a chunk insert filler SC */
    BDBG_MSG(("b_pvr_gop_recycle: %u %p", gop_entry->gop->flags.last_gop, (void *)BLST_SQ_FIRST(&player->active)));
    if (gop_entry->gop->flags.last_gop) {
        BDBG_MSG(("b_pvr_gop_recycle: %p last gop, adding filler", (void *)gop_entry));
        b_pvr_gop_feed_filler(player, gop_entry);        
       /* XVD updates ITB read pointer in chunks of 128 bytes (8 ITB entries) 
        * Insert another filler, so that OTF can see the ITB
        * read pointer when xvd really consumed the last ITB entry
        * Each filler should take up 4 ITB entries
        */
        b_pvr_gop_feed_filler(player, gop_entry);
        BOTF_P_SetOPITBValidPtr(player->otf_cntx);
    }

	return;
}

static void
b_pvr_gop_next_gop(bpvr_gop_player player)
{
	struct bpvr_gop_entry *gop_entry = BLST_SQ_FIRST(&player->active);

	BDBG_ASSERT(gop_entry);

	BDBG_MSG(("done gop %p", (void *)gop_entry->gop));
	BLST_SQ_REMOVE_HEAD(&player->active, queue);
	if(player->mode.rewind) {
		b_pvr_gop_recycle(player, gop_entry);
	} else {
		if (player->prev_gop) {
			b_pvr_gop_recycle(player, player->prev_gop);
		}
		player->prev_gop = gop_entry;
	}
	if (BLST_SQ_FIRST(&player->active)) {
        player->state = bgop_play_gop;		
		return;
	} else {
		player->state = bgop_waiting;
		BDBG_MSG(("waiting for new data"));
		return;
	}
}

static void
b_pvr_gop_next_frame(bpvr_gop_player player)
{    

	BDBG_ASSERT(BLST_SQ_FIRST(&player->active));
	if (player->gop_play.cur_frame < player->gop_play.n_frames) {
		BDBG_MSG(("displaying frame %u:%u out of %u(%u) for GOP %p(%p) %u", player->gop_play.cur_frame, BLST_SQ_FIRST(&player->active)->gop->frames[player->gop_play.frames[player->gop_play.cur_frame].index].info.pts, player->gop_play.n_frames, BLST_SQ_FIRST(&player->active)->gop->n_frames, (void *)BLST_SQ_FIRST(&player->active), (void *)BLST_SQ_FIRST(&player->active)->gop, BLST_SQ_FIRST(&player->active)->gop->frames[0].info.pts));
        B_OTF_WAIT_KEY(("displaying frame[%u] %u:%u(%d) out of %u(%u) for GOP %#x(%#x) %u", player->block_count, player->gop_play.cur_frame, BLST_SQ_FIRST(&player->active)->gop->frames[player->gop_play.frames[player->gop_play.cur_frame].index].info.pts, player->gop_play.frames[player->gop_play.cur_frame].index, player->gop_play.n_frames, BLST_SQ_FIRST(&player->active)->gop->n_frames, (unsigned)BLST_SQ_FIRST(&player->active), (unsigned)BLST_SQ_FIRST(&player->active)->gop, BLST_SQ_FIRST(&player->active)->gop->frames[0].info.pts));
        botf_itb_feeder_checkpoint(player->itb_feeder);
		player->disp_frames = bpvr_gop_build_play_seq(player, player->gop_play.frames[player->gop_play.cur_frame].index);
        player->block_count += player->disp_frames;
		if (player->disp_frames==0) {
			struct bpvr_gop_entry *gop_entry;
	    	gop_entry = BLST_SQ_FIRST(&player->active);
			BDBG_ASSERT(gop_entry);

			if (player->mode.rewind && gop_entry->gop->chunk_no!=B_MARKER_CHUNK_EOF) {
				struct bpvr_gop_entry *gop_next;

				gop_next = BLST_SQ_NEXT(gop_entry, queue);
				if (gop_next==NULL) {
                    BDBG_MSG(("Waiting for the second gop to arrive"));
					player->state = bgop_wait_2nd_gop;
					return;
				} else {
					player->disp_frames = b_pvr_build_cross_gop(player, gop_entry->gop, gop_next->gop, player->gop_play.frames[player->gop_play.cur_frame].index);
				}
			} else {
				player->disp_frames = b_pvr_build_cross_gop(player, gop_entry->gop, player->prev_gop?player->prev_gop->gop:NULL, player->gop_play.frames[player->gop_play.cur_frame].index);
			}
		}
		BDBG_MSG(("displaying %d frames from %d(%d,pic tag %d) for GOP %p(%p)", player->disp_frames, player->gop_play.cur_frame, player->gop_play.frames[player->gop_play.cur_frame].index, BLST_SQ_FIRST(&player->active)?(int)(BLST_SQ_FIRST(&player->active)->gop->frames[player->gop_play.frames[player->gop_play.cur_frame].index].info.pic_tag):-1, (void *)BLST_SQ_FIRST(&player->active), (void *)BLST_SQ_FIRST(&player->active)->gop));
		player->state = bgop_feed_sequence;
		b_pvr_gop_next_seq(player);
		return;
	} else {
		/* We are done with this GOP */
        player->state = bgop_play_gop;
		return;
	}
}

static void
b_pvr_gop_meta(bpvr_gop_player player)
{
	struct bpvr_gop_entry *gop_entry;    

    gop_entry = BLST_SQ_FIRST(&player->active);
    player->reference_frame = -1;
    player->past_predictor = -1;
    player->future_predictor = -1;
    BDBG_ASSERT(gop_entry);
	BDBG_MSG(("trying to feed meta data for GOP %p", (void *)gop_entry));
	player->state = bgop_feed_meta;

	if (b_pvr_reserve_feed(player)) {
		BDBG_MSG(("feeding meta data for GOP %p", (void *)gop_entry));
		b_pvr_gop_feed_segment(player, &gop_entry->gop->itb_meta);

		/* Build reference frame, if needed */
		if ((player->gop_play.n_frames > 1)&& 
		    (gop_entry->gop->frames[0].info.pic_type == B_FRAME_I)) {			
            struct bpvr_gop_play_seq *seq=&player->play_seq;

            BDBG_MSG(("feeding reference frame(meta) %d for GOP %p", 0, (void *)gop_entry->gop));
			/* BDBG_MSG(("buidling reference frame for GOP %#x", gop_entry->gop)); */
            BDBG_MSG_BTP(("BTP: Build Reference"));
            bpvr_make_btp_cmd(seq->BTPStartCmd, B_CMD_BUILD_REFERENCE, 0, 0, 0);
            BDBG_MSG(("feeding BUD packet(decode) %p", (void *)seq->BTPStartCmd));
            b_pvr_gop_feed_one_entry(player, seq->BTPStartCmd);
#if B_OTF_ENABLE_PICTAG
			BDBG_MSG(("feeding BUD packet(tag) %p PTS %#x", (void *)seq->BTPPicTagCmd, gop_entry->gop->frames[0].info.pts));
            BDBG_MSG_BTP(("BTP: Picture Tag %#x", (unsigned)gop_entry->gop->frames[0].info.pic_tag));
			bpvr_make_btp_cmd(seq->BTPPicTagCmd, B_CMD_PICTURE_TAG, 0, 0, gop_entry->gop->frames[0].info.pic_tag);
            b_pvr_gop_feed_one_entry(player, seq->BTPPicTagCmd);
#endif
			BDBG_MSG(("feeding reference frame(data) %u[%s] for GOP %p", 0, b_pvr_pic_type(gop_entry->gop->frames[0].info.pic_type), (void *)gop_entry->gop));
			BDBG_MSG_FRAME(("feeding reference frame(data) %u[%s] for GOP %p", 0, b_pvr_pic_type(gop_entry->gop->frames[0].info.pic_type), (void *)gop_entry->gop));
            b_pvr_gop_feed_segment(player, &(gop_entry->gop->frames[0].itb_meta));
			b_pvr_gop_feed_segment(player, &gop_entry->gop->frames[0].itb_slices);
			player->reference_frame = 0;
		}		
		player->state = bgop_feed_frame;
		b_pvr_gop_next_frame(player);
	}
	else
	{
		BDBG_MSG(("b_pvr_gop_meta, Not enough space in feeder"));
	}
}

/* Continue playing GOPs until we play all the pending GOPs or we run out of ITB space */
static void 
b_pvr_gop_start_play(bpvr_gop_player player)
{
	struct bpvr_gop_entry *gop_entry; 	

    while(player->state == bgop_play_gop)
    {
        gop_entry = BLST_SQ_FIRST(&player->active);

        BDBG_ASSERT(gop_entry);	

        BDBG_MSG(("start playing GOP %p %u:%u:%u", (void *)&gop_entry->gop, gop_entry->gop->n_frames, gop_entry->gop->gop_no, gop_entry->gop->chunk_no));

        if (player->mode.rewind)
            b_pvr_gop_build_gop_rew_seq(gop_entry->gop, &player->gop_play);
        else
            b_pvr_gop_build_gop_fwd_seq(gop_entry->gop, &player->gop_play);

        b_pvr_gop_filter_seq(player);


        player->reference_frame = -1;
        player->past_predictor = -1;	

        if (player->gop_play.n_frames == 0) { /* no frames to be played in this GOP */
            b_pvr_gop_next_gop(player);
            break;
        }   

        /* Play the GOP */
        b_pvr_gop_meta(player);

        /* Move to the next GOP */
        if (player->state == bgop_play_gop) {
            b_pvr_gop_next_gop(player);
        }        
    }
}

static void 
b_pvr_gop_player_buffer(bpvr_gop_player player)
{
    BDBG_MSG(("b_pvr_gop_player_buffer: %p %u", (void *)player, player->state));
	switch(player->state)
	{
	default:
	case bgop_waiting: 
        break;
	case bgop_wait_2nd_gop: 
        break;
	case bgop_feed_meta: 
        b_pvr_gop_meta(player);
        if (player->state == bgop_play_gop) {
            b_pvr_gop_next_gop(player);
        }
        b_pvr_gop_start_play(player);
        break;
	case bgop_feed_frame: 
        b_pvr_gop_next_frame(player);
        if (player->state == bgop_play_gop) {
            b_pvr_gop_next_gop(player);
        }
        b_pvr_gop_start_play(player);
        break;
	case bgop_feed_sequence: 
        b_pvr_gop_next_seq(player);
        if (player->state == bgop_play_gop) {
            b_pvr_gop_next_gop(player);
        }
        b_pvr_gop_start_play(player);
        break;
    case bgop_play_gop:
        b_pvr_gop_start_play(player);
        break;
	}
}

void 
bpvr_gop_freecompleted(bpvr_gop_player player)
{	
	struct bpvr_gop_entry *gop_entry;
    bpvr_cp_entry *cp_entry = BLST_SQ_FIRST(&player->cp_complete);
    bool itb_cpdone, cdb_cpdone;
    const void *itb_read, *itb_valid;       
    bool bfree = false;
    

    itb_read =  player->OPParserPtrs->ItbReadPtr;
    itb_valid = player->OPParserPtrs->ItbValidPtr;
    
    BDBG_MSG(("freecompleted:  itb_read=%p itb_valid=%p",itb_read, itb_valid));
    if (cp_entry == NULL)  {
        return;
    }

    do {        

        itb_cpdone = false;

        /* Check if checkpoint is between read and valid */
        if (itb_read < itb_valid) {
            if ( (cp_entry->itb_cp < itb_read) || cp_entry->itb_cp > itb_valid) {
              itb_cpdone = true;
            }
            BDBG_MSG(("freecompleted: ITB  READ:%p CP:%p VALID:%p WRAP:%p %s", itb_read, cp_entry->itb_cp, itb_valid, player->OPParserPtrs->ItbWrapAroundPtr, itb_cpdone?"DONE":""));
        } else {
            if ( (cp_entry->itb_cp < itb_read) && (cp_entry->itb_cp > itb_valid)) {
                itb_cpdone = true;
            }
            BDBG_MSG(("freecompleted: ITB VALID:%p CP:%p READ:%p WRAP:%p %s", itb_valid, cp_entry->itb_cp, itb_read, player->OPParserPtrs->ItbWrapAroundPtr, itb_cpdone?"DONE":""));
        }
        if (1 || itb_cpdone) {    
            const void *cdb_read, *cdb_valid;
            cdb_cpdone = false;            
            cdb_read = player->OPParserPtrs->CdbReadPtr;
            cdb_valid = player->OPParserPtrs->CdbValidPtr;
            if (cdb_read < cdb_valid) {
                if ( (cp_entry->cdb_cp < cdb_read) || cp_entry->cdb_cp > cdb_valid) {
                    cdb_cpdone = true;
                } 
                BDBG_MSG(("freecompleted: CDB  READ:%p CP:%p VALID:%p WRAP:%p %s", cdb_read, cp_entry->cdb_cp, cdb_valid, player->OPParserPtrs->CdbWrapAroundPtr, cdb_cpdone?"DONE":""));
            } else {
                if ( (cp_entry->cdb_cp < cdb_read) && (cp_entry->cdb_cp > cdb_valid)) {
                    cdb_cpdone = true;
                } 
                BDBG_MSG(("freecompleted: CDB VALID:%p CP:%p  READ:%p WRAP:%p %s", cdb_valid, cp_entry->cdb_cp, cdb_read, player->OPParserPtrs->CdbWrapAroundPtr, cdb_cpdone?"DONE":""));
            }
        }

        if (itb_cpdone && cdb_cpdone) {  			
            gop_entry = BLST_SQ_FIRST(&player->complete);
            BDBG_ASSERT(gop_entry);
            BLST_SQ_REMOVE_HEAD(&player->complete, queue);
            player->gop_done_cb(player->cb_cntx, gop_entry->gop);
            BLST_SQ_INSERT_TAIL(&player->free, gop_entry, queue);
            BLST_SQ_REMOVE_HEAD(&player->cp_complete, queue);
            BLST_SQ_INSERT_TAIL(&player->cp_free, cp_entry, queue);
            cp_entry = BLST_SQ_FIRST(&player->cp_complete);
            
            /* Space available in destn ITB */
            bfree = true;            
        }
    } while (itb_cpdone && cdb_cpdone && cp_entry );

    if (bfree) {
        /* Space available in destn ITB, so try to feed some more frames */
        b_pvr_gop_player_buffer(player);
    } else {
        if(cdb_cpdone && b_pvr_reserve_feed(player)) {
            b_pvr_gop_player_buffer(player);
        }
    }


	return;
}

void
bpvr_gop_player_feed(
		bpvr_gop_player player,  /* instance of the gop player */
		bpvr_gop_desc *gop /* GOP descriptor */
		)
{
	struct bpvr_gop_entry *gop_entry;


	BDBG_MSG(("new gop %p(%u,%u) %u frames", (void *)gop, gop->chunk_no, gop->gop_no, gop->n_frames));

    if (player->chunk_no != gop->chunk_no) {
        if (player->chunk_no != B_INVALID_CHUNK_NO ) {
            BOTF_P_FrameCountCallBack(player->otf_cntx, player->chunk_no, player->chunk_frame_cntr);
        }
        player->chunk_no = gop->chunk_no;
        player->chunk_frame_cntr = 0;
    }
	gop_entry = BLST_SQ_FIRST(&player->free);
	BDBG_ASSERT(gop_entry);
	BLST_SQ_REMOVE_HEAD(&player->free, queue);
	gop_entry->gop = gop;
	BLST_SQ_INSERT_TAIL(&player->active, gop_entry, queue);
	switch(player->state) {
	case bgop_waiting:
		BDBG_MSG(("starting playback"));
        player->state = bgop_play_gop;
		b_pvr_gop_start_play(player);
		break;
    case bgop_play_gop:
		b_pvr_gop_start_play(player);
		break;
	case bgop_wait_2nd_gop:
		{
			struct bpvr_gop_entry *gop_entry, *gop_next;

			BDBG_MSG(("continue playback"));

			gop_entry = BLST_SQ_FIRST(&player->active);
			BDBG_ASSERT(gop_entry);
			gop_next = BLST_SQ_NEXT(gop_entry, queue);
			BDBG_ASSERT(gop_next);
			player->disp_frames = b_pvr_build_cross_gop(player, gop_entry->gop, gop_next->gop, player->gop_play.frames[player->gop_play.cur_frame].index);
			player->state = bgop_feed_sequence;
			b_pvr_gop_next_seq(player);
            /* Move to the next GOP */
            b_pvr_gop_next_gop(player);
            b_pvr_gop_start_play(player);
			break;
		}
	default:
		break;
	}
	return;
}

void 
bpvr_gop_player_force(bpvr_gop_player player)
{
	switch(player->state) {
	case bgop_waiting:
		if((NULL != player->prev_gop)
			&&(NULL == BLST_SQ_FIRST(&player->complete))){
			b_pvr_gop_recycle(player, player->prev_gop);
			player->prev_gop = NULL;
			BDBG_MSG(("force prev gop"));
		}		
		BDBG_MSG(("force while in the waiting state"));
		break;
	case bgop_wait_2nd_gop:
		BDBG_MSG(("forcing gop to be flushed out"));
		b_pvr_gop_next_gop(player);
		break;
	default:
        BDBG_MSG(("force more data"));
        b_pvr_gop_player_buffer(player);
        break;
    }
    B_OTF_WAIT_KEY(("Player Force"));

}

void
bpvr_gop_player_get_mode( bpvr_gop_player player,  bpvr_gop_player_mode *mode)
{
	*mode = player->mode;
	return;
}

void
bpvr_gop_player_set_mode(bpvr_gop_player player, bpvr_gop_player_mode *mode )
{
	player->mode = *mode;
	return;
}
