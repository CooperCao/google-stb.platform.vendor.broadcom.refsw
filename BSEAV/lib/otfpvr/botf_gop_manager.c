/***************************************************************************
 *  Copyright (C) 2007-2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * GOP manager module for the on the fly PVR
 *
 ***************************************************************************/
#include "bstd.h"
#include "blst_squeue.h"
#include "bkni.h"

#include "botf_gop_manager.h"
#include "botf_gop_player.h"
#include "botf_priv.h"
#include "botf_platform.h"
#include "botf_marker.h"

#undef current

BDBG_MODULE(botf_gop_manager);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

/*
 * GOP manager consumes start codes and constructs the GOP structures. It
 * uses the following state machine. First column shows the possible start
 * codes and First row shows the possible states. Each entry in the contains
 * next state based on the SC received and the action GOP manager takes
--------------------------------------------------------------------------------
|     State-->|Seeking     |Seeking    |Seeking      |Seeking      |Seeking      |
|SC           |SeqHdrGOP   |GOP        |FirstPic     |PCEFirstSlice|SlicePic     |
|------------------------------------------------------------------------------  |
|Picture SC   |SeqHdrGOP   |SeqHdrGOP  |PCEFirstSlice|SeqHdrGOP    |PCEFirstSlice|
|0x00         |None        |None       |gop_create   |close_gop    |None         |
|------------------------------------------------------------------------------  |
|PCE or Slice |SeqHdrGOP   |SeqHdrGOP  |SeqHdrGOP    |SlicePic     |SlicePic     |
|0x1-0xAD,0xAF|None        |None       |None         |init_frame   |None         |
|------------------------------------------------------------------------------  |
|SeqHdr SC    |GOP         |SeqHdrGOP  |SeqHdrGOP    |SeqHdrGOP    |SeqHdrGOP    |
|0xB3         |None        |None       |None         |close_gop    |ClosePic,GOP |
|------------------------------------------------------------------------------  |
|GOP SC       |FirstPic    |FirstPic   |SeqHdrGOP    |SeqHdrGOP    |SeqHdrGOP    |
|0xB8         |None        |None       |None         |close_gop    |ClosePic,GOP |
|------------------------------------------------------------------------------  |
|Marker   0xAE|SeqHdrGOP   |SeqHdrGOP  |SeqHdrGOP    |SeqHdrGOP    |SeqHdrGOP    |
|             |None        |None       |None         |close_gop    |closePic,GOP |
|------------------------------------------------------------------------------  |
|All other SC |SeqHdrGOP   |GOP        |FirstPic     |PCEFirstSlice|SeqHdrGOP    |
|             |None        |None       |None         |None         |ClosePic,GOP |
|-------------------------------------------------------------------------------
*
* PCE is picture coding extension. Parsing PCE is added to gop manager to support
* field pictures. Here are the MPEG rules related to PCE and field pictures
*     .  PCE may not be always prenest in the video stream. It shall only occur
*        immeadiately after a picture header.
*     .  Picture structure in PCE indicates the following
*            picture_structure	Meaning
*                       00	    reserved
*                       01	    Top Field
*                       10	    Bottom Field
*                       11	    Frame picture
*
*     .  There is no defined sequence for top and bottom fields. Top field may occur
*        before or after bottom field.
*     .  Field pictures always occur in pairs.
*
*   Here is how PCE is handled :
*       PCE is ignored if it occurs anywhere other than immediately after picture header.
*       At the time of initializing frame isField is initialized to false and FieldPicture
*       is set to 0
*       when PCE is encountered if this is a field picture then isField is set to true and
*       FieldPicture is incremented.
*       When a picture header is encountered then if isField is set and FieldPicture is 2 then
*       a new frame is initialized.
*/

#define B_MPEG2_PICSTARTCODE        0x00
#define B_MPEG2_MAXSLICESTARTCODE   0xAF
#define B_MPEG2_USERDATASTARTCODE   0xB2
#define B_MPEG2_SEQHEADERCODE       0xB3
#define B_MPEG2_SEQERRORCODE        0xB4
#define B_MPEG2_EXTENSIONSTARTCODE  0xB5
#define B_MPEG2_SEQENDCODE          0xB7
#define B_MPEG2_GOPSTARTCODE        0xB8

#define B_MPEG2_TOPFIELD            1
#define B_MPEG2_BOTTOMFIELD         2
#define B_MPEG2_FRAMEPIC            3

struct bpvr_gop_sequence {
    bpvr_start_code lastscode; /* Pointer to the last scode used by this GOP */
	int	ref_cnt; /* referece count, to mark frames which could be potentially used by the GOP player */
};

struct bpvr_gop {
	BLST_SQ_ENTRY(bpvr_gop) queue;
	bpvr_gop_desc gop; /* GOP in the CDB buffer */
	struct bpvr_gop_sequence *sequence; /* pointer to the GOP meta information (used by recycling) */
	struct bpvr_gop_sequence meta; /* GOP meta information (used by capturing) */
};

enum bgopm_match  {
		bgopm_match_none=0, /* last GOP is not avaliable */
		bgopm_match_last_include, /* include frame matches last_gop */
		bgopm_match_last_exclude /* exclide frame matches last_gop */
};


BLST_SQ_HEAD(bpvr_gop_manager_queue, bpvr_gop);

struct bpvr_gop_signature {
	BLST_SQ_ENTRY(bpvr_gop_signature) queue;
	uint32_t time_code; /* time_code field from the GOP header */
	uint32_t pts;  	   /* first PTS value (format specific) */
	bool	pts_valid; /* qualifier for PTS */
};

/* Contains gop descriptor and each frame size */
struct bpvr_gop_match_data {
    bpvr_gop_desc gop;
    uint32_t frame_size[BOTF_GOP_MAX_FRAMES];
};

typedef enum bgopm_state {
    bgopm_seekingSeqHdrGOP=0, /* Seeking for Seq Header or GOP */
    bgopm_seekingGOP,         /* Seeking for GOP */
    bgopm_seekingFirstPic,    /* Seeking for first picture in GOP */
    bgopm_seekingPCEFirstSlice,/* Seeking for first Slice in a Picture or picture coding extension */
    bgopm_seekingSlicePic,    /* Seeking for either next Picture or slice */
    bgopm_seekingFiller,         /* Seeking Filler sc after marker */
    bgopm_busy,               /* GOP manager is busy */
    bgopm_discardUntilMarker /* Discard data until next marker */
}bgopm_state;


BLST_SQ_HEAD(bpvr_gop_history, bpvr_gop_signature);
#define B_PVR_HISTORY_SIZE	16

#define B_PVR_NFRAMES_MATCH 2

#define B_INVALID_TIME_CODE (unsigned)(-1)
#define B_UNKNOWN_TIME_CODE (unsigned)(-2)


struct bpvr_gop_manager_impl
{
	void *cb_cntx;
	void (*done_cb)(void *cntx, const void *cdb, const void *itb); /* done callback */
	void (*resume_cb)(void *cntx); /* resume callback for the CDB feeder */
	bpvr_gop_player player; /* consumer of the GOP entries */
    bgopm_state state;
    bgopm_state prev_state;

    bpvr_start_code scode_meta; /* contains last Seq Hdr or GOP Start code */
    bpvr_start_code scode_pic;  /* contains last picture start code  */
    bpvr_start_code scode_filler;  /* Contains Filler sc in this chunk */
    const void *cdb_free; /* pointer to the level what could be reused */	
    const void *itb_free; /* pointer to the level what could be reused */	

	unsigned gop_seq;  /* gop number in the marker */
	unsigned chunk_no; /* chunk number as received from the marker */	

    uint32_t gop_time_code; /* Last GOP time code */
	struct bpvr_gop *current; /* current GOP, null if state is seeking for SeqHdr or GOP */
    struct bpvr_gop *prev;    /* Previous GOP */
	struct bpvr_gop_manager_queue complete; /* list of complete GOP's */
	struct bpvr_gop_manager_queue free; /* list of free GOP's entries */
	struct bpvr_gop_manager_queue staging; /* list of GOP's from a single chunk in a interleaved mode */
    struct {
        unsigned nallocated;
	    struct bpvr_gop *gop_ptrs[B_PVR_MAX_GOPS]; /* placeholder for the pointer GOP entries */
    } gop_allocator;
	struct bpvr_gop_history history; /* list of GOP's signature, it's used in the interleaved mode to remove duplicated gops */
	struct bpvr_gop_signature history_mem[B_PVR_HISTORY_SIZE]; /* placeholder for the GOP history */
    bpvr_start_code last_scode; /* last scode processed by start code parser */
	BOTF_FeederMode mode; /* feeder mode, it's affects how gops extracted from the CDB data */
	bpvr_stream_type stream; /* stream type used to select source for PTS (BES header or picture header user data) */
	enum bgopm_match last_match; /* type of match scheme to be applied for the last GOP */
	struct bpvr_gop_match_data last_gop; /* template to match last GOP frame */
	uint32_t next_pts; /* PTS for the next parsed frame */
	uint32_t last_pts; /* PTS for the next parsed frame */
    uint32_t next_tag; /* tag for the next parsed frame */
    botf_scv_parser scv_parser;
    const BOTF_ParserPtrs *IPParserPtrs;
    uint8_t pic_type;      /* Last picture type */
	bool next_pts_valid; /* qualifier for the PTS value, sets to off once value is used */
    bool next_tag_valid; /* qualifier for the tag value, sets to off once value is used */
	bool paused; /* true if parsing is paused */
    /* true when only last or first I-frame should be displayed in a chunk
     * For chunk forward mode, last I-frame will be displayed and chunk backward mode
     * first I-frame will be displayed.
     * This flag is included as part of the picture tag BTP commands.
     * This flag is valid only in chunk forward, chunk backward modes. For all the other
     * modes this will be ignored.
     */
    bool disp_only_last_first_iframe;
    /* true when last or first I-frame need to be displayed in a chunk
     * disp_only_last_first_iframe flag displays only last or first I-frame in chunk,
     * where as this flag displays other I-frames in the chunk and ensures to display
     * last or first I-frame.
     * This flag is included as part of the picture tag BTP commands.
     * This flag is valid only in chunk forward, chunk backward modes. For all the other
     * modes this will be ignored.
     */
    bool disp_last_first_iframe;
    bool     bUsePtsAsTag;    /* If true, use PTS as picture tag, otherwise uses the tag specified in stream
                               * using pictag BTP command */
    bool     bIsField;      /* Indicates whether current frame being processed is a field or */
    uint8_t  FieldPicture;  /* Current field picture count */
    uint8_t  PrevScode;     /* Previous scode */

    struct {
        unsigned n;
        bool enable;
    } headless_gop_frame_counter;
};

static void bpvr_gop_manager_recycle(void *cntx, bpvr_gop_desc *gop);
static void b_pvr_gop_init_frame(bpvr_gop_manager manager);

/* Typedef for state consume sc function pointer */
typedef bgopm_state(*bgopm_statefunc)(bpvr_gop_manager manager, const bpvr_start_code *scode);


static bgopm_state bgopm_seekingSeqHdrGOPFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_seekingGOPFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_seekingFirstPicFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_seekingPCEFirstSliceFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_seekingSlicePicFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_seekingFillerFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_busyFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);
static bgopm_state bgopm_discardUntilMarkerFunc(bpvr_gop_manager manager, const bpvr_start_code *scode);


#define B_GOP_STATE_ENTRY(state) {bgopm_##state##Func, bgopm_##state, #state}
static const struct {
    bgopm_statefunc action;
    bgopm_state state;
    const char *name;
} b_gopm_states[] = {
    B_GOP_STATE_ENTRY(seekingSeqHdrGOP),
    B_GOP_STATE_ENTRY(seekingGOP),
    B_GOP_STATE_ENTRY(seekingFirstPic),
    B_GOP_STATE_ENTRY(seekingPCEFirstSlice),
    B_GOP_STATE_ENTRY(seekingSlicePic),
    B_GOP_STATE_ENTRY(seekingFiller),
    B_GOP_STATE_ENTRY(busy),
    B_GOP_STATE_ENTRY(discardUntilMarker)
};

bpvr_gop_manager
bpvr_gop_manager_create(bpvr_gop_manager_cfg *cfg)
{
	bpvr_gop_manager manager;
    unsigned i;

	BDBG_ASSERT(cfg->cdb_done);
	BDBG_ASSERT(cfg->cdb_resume);	
	BDBG_ASSERT(cfg->OPParserPtrs);
    BDBG_ASSERT(cfg->IPParserPtrs);
	
	manager = BKNI_Malloc(sizeof(*manager));
	if (!manager) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		return NULL;
	}
	BKNI_Memset(manager, 0, sizeof(*manager));

    manager->gop_allocator.nallocated = 0;
    BKNI_Memset(manager->gop_allocator.gop_ptrs, 0, sizeof(manager->gop_allocator.gop_ptrs));
    for(i=0;i<8;i++) {
	    struct bpvr_gop *gop_ptr;
        gop_ptr = BKNI_Malloc(sizeof(*gop_ptr));
        if(!gop_ptr) {
            break;
        }
        manager->gop_allocator.gop_ptrs[i] = gop_ptr;
        manager->gop_allocator.nallocated = i+1;
    }

	manager->player = bpvr_gop_player_create(bpvr_gop_manager_recycle, manager, cfg->cdb_cntx, cfg->OPParserPtrs, cfg->IPParserPtrs);
	if (!manager->player) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BKNI_Free(manager);
		return NULL;
	}
	manager->cb_cntx = cfg->cdb_cntx;
	manager->done_cb = cfg->cdb_done;
	manager->resume_cb = cfg->cdb_resume;	
    manager->IPParserPtrs = cfg->IPParserPtrs;
    manager->bUsePtsAsTag = cfg->bUsePtsAsTag;

	bpvr_gop_manager_reset(manager, BOTF_FeederMode_eGop, bpvr_stream_type_mpeg);	

	return manager;
}

void
bpvr_gop_manager_reset(bpvr_gop_manager manager, BOTF_FeederMode mode, bpvr_stream_type stream)
{
	unsigned i;

	manager->state = bgopm_seekingSeqHdrGOP;
	manager->prev_state = bgopm_discardUntilMarker;
    manager->PrevScode = 0;
	manager->current = NULL;	
    manager->prev = NULL;

	manager->paused = false;    	
	manager->gop_seq = 0;
	manager->chunk_no = 0;	
	manager->mode = mode;
	manager->stream = stream;
	manager->last_match = bgopm_match_none;
	manager->next_pts_valid = false;
	manager->next_pts = 0;    	
    manager->last_pts = 0;
    manager->next_tag_valid = false;
    manager->next_tag = 0;
    manager->disp_only_last_first_iframe = false;
    manager->disp_last_first_iframe = false;

#ifdef OTF_CDI_SPECIFIC
	if (mode == BOTF_FeederMode_eChunkForward)
	{
		manager->state = bgopm_seekingFiller;
		manager->next_tag_valid = true;
		manager->disp_only_last_first_iframe = true;
	}
#endif

	BLST_SQ_INIT(&manager->complete);
	BLST_SQ_INIT(&manager->free);
	BLST_SQ_INIT(&manager->staging);
	BLST_SQ_INIT(&manager->history);

    BKNI_Memset(&manager->scode_meta, 0, sizeof(manager->scode_meta));
    BKNI_Memset(&manager->scode_pic, 0, sizeof(manager->scode_pic));
    BKNI_Memset(&manager->scode_filler, 0, sizeof(manager->scode_filler));

	for(i=0;i<manager->gop_allocator.nallocated;i++) {
	    struct bpvr_gop *gop_ptr = manager->gop_allocator.gop_ptrs[i];
        BDBG_ASSERT(gop_ptr);
        BKNI_Memset(&gop_ptr->gop.sc_filler, 0, sizeof(gop_ptr->gop.sc_filler)); /* clear filler */
		BLST_SQ_INSERT_HEAD(&manager->free, gop_ptr, queue);
	}

	for(i=0;i<B_PVR_HISTORY_SIZE;i++) {
		manager->history_mem[i].pts_valid = false;
		manager->history_mem[i].time_code = B_INVALID_TIME_CODE;
		BLST_SQ_INSERT_HEAD(&manager->history, &manager->history_mem[i], queue);
	}
	return;
}


void
bpvr_gop_manager_destroy(bpvr_gop_manager manager)
{
    unsigned i;
    for(i=0;i<manager->gop_allocator.nallocated;i++) {
        BKNI_Free(manager->gop_allocator.gop_ptrs[i]);
    }
	BKNI_Free(manager);
	return;
}

void
bpvr_gop_manager_setscvparser(bpvr_gop_manager manager, botf_scv_parser scv_parser)
{
    manager->scv_parser = scv_parser;
}

static void
b_itb_fill_segment(bpvr_gop_manager manager, bpvr_itb_segment *segment, const bpvr_start_code *start_scode, const bpvr_start_code *end_scode)
{
    BSTD_UNUSED(manager);
    segment->start_scode = *start_scode;
    segment->end_scode = *end_scode;
}

void
bpvr_gop_manager_set_pts(bpvr_gop_manager manager, uint32_t pts)
{
    BDBG_MSG(("next_pts: %u", pts)); 
	manager->next_pts_valid = true;
	manager->next_pts = pts;
    manager->last_pts = pts;
#ifdef OTF_CDI_SPECIFIC
	{
		BOTF_Handle lhOtf;
		lhOtf = (BOTF_Handle)(manager->cb_cntx);
		if ( (lhOtf->Settings.PTSValueCbck != NULL)  && (lhOtf->Settings.MonitorCntx != NULL) )
		{
			lhOtf->Settings.PTSValueCbck(lhOtf->Settings.MonitorCntx,
										 manager->next_tag_valid ? manager->next_tag : 0,
										 pts);
		}
	}
#endif
	return;
}

/* Create a new GOP, scode contains information about the first picture start code (end of GOP meta data)*/
void bpvr_gop_manager_set_tag(bpvr_gop_manager manager, uint32_t tag)
{
    BDBG_MSG(("Picture tag from BTP = %#x", tag));
	manager->next_tag_valid = true;
	manager->next_tag = tag;
	return;

}

/* Set the flag for this chunk to indicate if only last or first I-frame should be displayed */
void 
bpvr_gop_manager_set_disp_only_lfiframe(bpvr_gop_manager manager, uint8_t disp_only_last_first_iframe)
{
    BDBG_MSG(("Display only last or first I-frame flag = %d",disp_only_last_first_iframe));
    manager->disp_only_last_first_iframe = disp_only_last_first_iframe;
    return;
}

/* Set the flag for this chunk to ensure that last or first I-frame is displayed */
void 
bpvr_gop_manager_set_disp_lfiframe(bpvr_gop_manager manager, uint8_t disp_last_first_iframe)
{
    BDBG_MSG(("Display last or first I-frame flag = %d",disp_last_first_iframe));
    manager->disp_last_first_iframe = disp_last_first_iframe;
    return;
}

static void
b_pvr_gop_feed_to_player(bpvr_gop_manager manager, struct bpvr_gop *gop)
{
    BDBG_MSG(("feeding gop=%p:%p ref_cnt=%d frames:%u PTS:%u %s %s %s %s",(void *)&gop->gop, (void *)gop, gop->sequence->ref_cnt, gop->gop.n_frames, gop->gop.frames[0].info.pts, gop->gop.flags.complete?"complete":"", gop->gop.flags.discontinuity?"discontinuity":"", gop->gop.flags.disp_iframe?"iframe":"", gop->gop.flags.last_gop?"last":""));
    BLST_SQ_INSERT_TAIL(&manager->complete, gop, queue);
    bpvr_gop_player_feed(manager->player, &gop->gop);
    return;
}

static void
b_pvr_gop_create(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
	struct bpvr_gop *gop;   	

	gop = BLST_SQ_FIRST(&manager->free);
	BDBG_ASSERT(gop);
	BDBG_MSG(("found new gop ITB= %p CDB = %p", (void *)manager->scode_meta.itbentryptr, (void *)manager->scode_meta.cdb));
	BLST_SQ_REMOVE_HEAD(&manager->free, queue);
	gop->gop.n_frames = 0;
    gop->gop.flags.disp_iframe = false;
	gop->gop.flags.complete = false;
	gop->gop.flags.discontinuity = false;
    gop->gop.flags.last_gop = false;
    gop->gop.time_code = manager->gop_time_code;
    /*
     * If we are playing in chunk mode, then the skipped frames before the first sequence header of every
     * chunk will need to be included in the frame_count to get the right speed.
     * We don't add them in for continuous play (sequential or interleaved mode), as all the data is being
     * delivered, so the maximum error is one gops worth of frames, regardless of play time.
     * We don't add them in for gop mode as the data is already aligned
     *
     * Assign the counted frames to the first gop found in the chunk. Since we are discontinuous, it is not
     * significant whether we are playing backwards or forwards.
     */
    if (manager->headless_gop_frame_counter.n && (manager->mode == BOTF_FeederMode_eChunkForward || manager->mode == BOTF_FeederMode_eChunkBackward)) {
        gop->gop.headless_gop_frame_count = manager->headless_gop_frame_counter.n;
        manager->headless_gop_frame_counter.n = 0;
    }
    else {
        gop->gop.headless_gop_frame_count = 0;
    }
    manager->prev = manager->current;
	manager->current = gop;
    b_itb_fill_segment(manager, &gop->gop.itb_meta, &manager->scode_meta, scode);
    BKNI_Memset(&gop->gop.sc_filler, 0, sizeof(gop->gop.sc_filler)); /* clear filler */

	b_pvr_gop_init_frame(manager);

    if(BLST_SQ_FIRST(&manager->free)==NULL && manager->gop_allocator.nallocated<B_PVR_MAX_GOPS) {
	    struct bpvr_gop *gop_ptr;
        gop_ptr = BKNI_Malloc(sizeof(*gop_ptr));
        BDBG_MSG(("allocationg %u'th gop -> %p", manager->gop_allocator.nallocated, (void *)gop_ptr));
        if(gop_ptr) {
            BKNI_Memset(&gop_ptr->gop.sc_filler, 0, sizeof(gop_ptr->gop.sc_filler)); /* clear filler */
            manager->gop_allocator.gop_ptrs[manager->gop_allocator.nallocated] = gop_ptr;
            manager->gop_allocator.nallocated++;
		    BLST_SQ_INSERT_HEAD(&manager->free, gop_ptr, queue);
        }
    }
    if (BLST_SQ_FIRST(&manager->free)==NULL) {
        BDBG_MSG(("pause data acquisition"));
        manager->paused = true;
    }
	return;
}

static bool
b_is_chunks_mode(bpvr_gop_manager manager)
{
	switch(manager->mode) {
	case BOTF_FeederMode_eInterleaved:
	case BOTF_FeederMode_eChunkBackward:
		return true;
	default:
		return false;
	}
}

static void
b_pvr_gop_flush(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
	struct bpvr_gop *gop = manager->current;

	BDBG_ASSERT(gop);
	gop->gop.flags.complete = false;
	gop->meta.ref_cnt = 1;
    gop->meta.lastscode = *scode;
    BDBG_MSG(("flushing gop %p, time_code=%#x %d frames ITB=%p CDB=%p", (void *)gop, (unsigned)gop->gop.time_code, gop->gop.n_frames, (void *)scode->itbentryptr, (void *)scode->cdb));

    if(gop->gop.n_frames>0) {
        if(b_is_chunks_mode(manager)) {
            gop->sequence = NULL;
            BLST_SQ_INSERT_HEAD(&manager->staging, gop, queue);
            manager->current = NULL;
        } else {
            gop->sequence = &gop->meta;
            gop->gop.lastcdbptr = (void *) gop->sequence->lastscode.cdb;
            if (manager->prev != NULL) {
                if (!manager->disp_only_last_first_iframe) {
                    b_pvr_gop_feed_to_player(manager, manager->prev);
                } else {
                    BDBG_MSG(("gop_flush, discarding the GOP because disp last or first I-frame flag is set"));
                    BLST_SQ_INSERT_HEAD(&manager->free, manager->prev, queue);
                }
                manager->prev = NULL;
            }
        }
    } else {
        BDBG_MSG(("useless gop %p", (void *)&gop->gop));
        /* recycle GOP */
        BLST_SQ_INSERT_HEAD(&manager->free, gop, queue);
        manager->current = NULL;
    }
}

/* this function is used to finish GOP and feed it to consumer */
static void
b_pvr_gop_close_gop(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop = manager->current;

	BDBG_ASSERT(gop);

	gop->gop.chunk_no = manager->chunk_no;
	gop->gop.gop_no = manager->gop_seq;
	manager->gop_seq++;

    BDBG_MSG(("closing gop %p, time_code=%#x(%u:%u) chunk:%u frames:%u ITB=%p CDB=%p", (void *)gop, (unsigned)gop->gop.time_code, gop->gop.frames[0].info.pts, gop->gop.frames[gop->gop.n_frames-1].info.pts, gop->gop.chunk_no, gop->gop.n_frames,
              gop->gop.frames[gop->gop.n_frames-1].itb_slices.end_scode.itbentryptr,gop->gop.frames[gop->gop.n_frames-1].itb_slices.end_scode.cdb));
	gop->meta.ref_cnt = 1;
	if(b_is_chunks_mode(manager)) {
		gop->sequence = NULL;
		BLST_SQ_INSERT_HEAD(&manager->staging, gop, queue);
        manager->current = NULL;
	} else {
		gop->sequence = &gop->meta;
        gop->gop.lastcdbptr = (void *)gop->sequence->lastscode.cdb;
        if (manager->prev != NULL) {
            if (!manager->disp_only_last_first_iframe) {
                b_pvr_gop_feed_to_player(manager, manager->prev);
            } else {
                BDBG_MSG(("close_gop, discarding the GOP because disp last or first I-frame flag is set"));
                BLST_SQ_INSERT_HEAD(&manager->free, manager->prev, queue);
            }
            manager->prev = NULL;
        }
	}
}

/* initialized all necessary per picture information */
static void
b_pvr_gop_init_frame(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop = manager->current;

	BDBG_ASSERT(gop);

	gop->gop.frames[gop->gop.n_frames].info.flags.pts = false;
    gop->gop.frames[gop->gop.n_frames].info.flags.tag = false;
	gop->gop.frames[gop->gop.n_frames].info.flags.repeat_first_field = false;
    gop->gop.frames[gop->gop.n_frames].info.pic_type =  manager->pic_type;
    gop->gop.frames[gop->gop.n_frames].info.flags.pts = manager->next_pts_valid;
    gop->gop.frames[gop->gop.n_frames].info.pts = manager->next_pts;
    manager->next_pts_valid = false;
    manager->next_pts = 0;

	return;
}

/* stores all necessary per picture information */
static void
b_pvr_gop_finish_frame(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop = manager->current;
	BDBG_ASSERT(gop);

    BDBG_MSG(("Frame#%d Start ITB=%p CDB=%p End ITB=%p CDB=%p",gop->gop.n_frames,
              gop->gop.frames[gop->gop.n_frames].itb_meta.start_scode.itbentryptr, gop->gop.frames[gop->gop.n_frames].itb_meta.start_scode.cdb,
              gop->gop.frames[gop->gop.n_frames].itb_slices.end_scode.itbentryptr, gop->gop.frames[gop->gop.n_frames].itb_slices.end_scode.cdb));

    if (!gop->gop.frames[gop->gop.n_frames].info.flags.tag) {
        if (manager->bUsePtsAsTag) {
            gop->gop.frames[gop->gop.n_frames].info.flags.tag = manager->next_pts_valid;
            gop->gop.frames[gop->gop.n_frames].info.pic_tag = manager->next_pts;
        } else {
            gop->gop.frames[gop->gop.n_frames].info.flags.tag = manager->next_tag_valid;
            gop->gop.frames[gop->gop.n_frames].info.pic_tag = manager->next_tag;
        }
    }
	gop->gop.frames[gop->gop.n_frames].info.frame_no = gop->gop.n_frames;
	gop->gop.n_frames++;
	return;
}

static void
bpvr_gop_store_last_gop(bpvr_gop_manager manager, bpvr_gop_desc *gop)
{
    unsigned i;

    manager->last_gop.gop = *gop;

    for(i=0;i<gop->n_frames;i++) {
        if (gop->frames[i].itb_meta.end_scode.cdb < gop->frames[i].itb_meta.start_scode.cdb)  {
            /* Wrap around */
            manager->last_gop.frame_size[i] = manager->IPParserPtrs->CdbWrapAroundPtr - (unsigned long) gop->frames[i].itb_meta.start_scode.cdb + 1 +
                            (unsigned long) gop->frames[i].itb_meta.end_scode.cdb - manager->IPParserPtrs->CdbStartPtr;
        } else {
            manager->last_gop.frame_size[i] = (uint8_t *) gop->frames[i].itb_meta.end_scode.cdb - (uint8_t *) gop->frames[i].itb_meta.start_scode.cdb;
        }

        if (gop->frames[i].itb_slices.end_scode.cdb < gop->frames[i].itb_slices.start_scode.cdb)  {
            /* Wrap around */
            manager->last_gop.frame_size[i] += manager->IPParserPtrs->CdbWrapAroundPtr - (unsigned long) gop->frames[i].itb_slices.start_scode.cdb + 1 +
                            (unsigned long) gop->frames[i].itb_slices.end_scode.cdb - manager->IPParserPtrs->CdbStartPtr;
        } else {
            manager->last_gop.frame_size[i] += (uint8_t *) gop->frames[i].itb_slices.end_scode.cdb - (uint8_t *) gop->frames[i].itb_slices.start_scode.cdb;
        }
    }
}

static bool
bpvr_last_gop_match(bpvr_gop_manager manager, bpvr_gop_desc *gop)
{
    bpvr_gop_desc *gop1 = &(manager->last_gop.gop);
    bpvr_gop_desc *gop2 = gop;
    unsigned gop1_framelen, gop2_framelen;
    unsigned i;

	BDBG_MSG(("gop match:(%p:%#x:%u:%u:%u) (%p:%#x:%u:%u:%u)", (void *)gop1, (unsigned)gop1->time_code, gop1->n_frames, (unsigned)gop1->flags.complete, gop1->frames[0].info.pts, (void *)gop2, (unsigned)gop2->time_code, gop2->n_frames, gop2->frames[0].info.pts, (unsigned)gop2->flags.complete));
	if (gop1->time_code != B_INVALID_TIME_CODE && gop1->time_code != gop2->time_code) {
		return false;
	}
	if ( (!gop1->flags.complete && gop1->n_frames<=B_PVR_NFRAMES_MATCH) || (!gop2->flags.complete && gop2->n_frames<=B_PVR_NFRAMES_MATCH)) {
		BDBG_MSG(("not matched because of GOP size %u:%u %u:%u", gop1->flags.complete, gop1->flags.complete, gop2->flags.complete, gop2->flags.complete));
		return false;
	}
    for(i=0;i<gop1->n_frames && i<gop2->n_frames;i++) {

        gop1_framelen = manager->last_gop.frame_size[i];

        if (gop2->frames[i].itb_meta.end_scode.cdb < gop2->frames[i].itb_meta.start_scode.cdb) {
            /* Wrap around */
            gop2_framelen = manager->IPParserPtrs->CdbWrapAroundPtr - (unsigned long) gop2->frames[i].itb_meta.start_scode.cdb + 1 +
                            (unsigned long) gop2->frames[i].itb_meta.end_scode.cdb - manager->IPParserPtrs->CdbStartPtr;
        } else {
            gop2_framelen = (uint8_t *) gop2->frames[i].itb_meta.end_scode.cdb - (uint8_t *)gop2->frames[i].itb_meta.start_scode.cdb;
        }

        if (gop2->frames[i].itb_slices.end_scode.cdb < gop2->frames[i].itb_slices.start_scode.cdb)
            /* Wrap around */
            gop2_framelen += manager->IPParserPtrs->CdbWrapAroundPtr - (unsigned long) gop2->frames[i].itb_slices.start_scode.cdb + 1 +
                            (unsigned long) gop2->frames[i].itb_slices.end_scode.cdb - manager->IPParserPtrs->CdbStartPtr;
        else
            gop2_framelen += (uint8_t *) gop2->frames[i].itb_slices.end_scode.cdb - (uint8_t *)gop2->frames[i].itb_slices.start_scode.cdb;

        if (   gop1->frames[i].info.pic_type != gop2->frames[i].info.pic_type
            || gop1->frames[i].info.pts != gop2->frames[i].info.pts
            || gop1->frames[i].info.flags.pts != gop2->frames[i].info.flags.pts
            || (gop1->n_frames > i+1 && gop2->n_frames > i+1 && ( /* size of last frame is not trust worthy */
               gop1_framelen != gop2_framelen)) ) {
            BDBG_MSG(("not matched frame:%u len(%u:%u) pts(%u:%u)", i, gop1_framelen, gop2_framelen, gop1->frames[i].info.pts, gop2->frames[i].info.pts));
            return false;
        }
    }
	return true;
}

static void
bpvr_gop_history_convert(bpvr_gop_desc *gop, struct bpvr_gop_signature *item)
{
	unsigned i;

	item->time_code = gop->time_code;
	item->pts_valid = false;
    item->pts = 0;
	for(i=0;i<gop->n_frames;i++) {
		if (gop->frames[i].info.flags.pts) {
			item->pts_valid = true;
			item->pts = gop->frames[i].info.pts;
			break;
		}
	}
	return;
}

static void
bpvr_gop_history_add(bpvr_gop_manager manager, bpvr_gop_desc *gop)
{
	struct bpvr_gop_signature *item;

	/* maintain FIFO of entries */
    BDBG_MSG(("adding history gop %p, time_code=%#x(%u:%u) %u frames", (void *)gop, (unsigned)gop->time_code, gop->frames[0].info.pts, gop->frames[gop->n_frames-1].info.pts, gop->n_frames));
	item = BLST_SQ_FIRST(&manager->history);
	BDBG_ASSERT(item);
    BDBG_MSG(("recycling history entry time_code=%#x(%u)", item->time_code, item->pts));
	BLST_SQ_REMOVE_HEAD(&manager->history, queue);
	bpvr_gop_history_convert(gop, item);
	BLST_SQ_INSERT_TAIL(&manager->history, item, queue);
	return;
}

static bool
bpvr_gop_history_match(bpvr_gop_manager manager, bpvr_gop_desc *gop)
{
	struct bpvr_gop_signature *item;
	struct bpvr_gop_signature template;

	bpvr_gop_history_convert(gop, &template);

	if (template.time_code == B_INVALID_TIME_CODE  && !template.pts_valid) {
		return false;
	}
	for(item=BLST_SQ_FIRST(&manager->history);item;item=BLST_SQ_NEXT(item, queue)) {
		if (item->time_code == B_INVALID_TIME_CODE  && !item->pts_valid) {
			continue;
		}
		BDBG_MSG(("history_match: %p %#x:%#x %u %u:%u", (void *)item, item->time_code, template.time_code, item->pts_valid, item->pts,template.pts));
		if (template.time_code == item->time_code && (template.pts_valid == item->pts_valid && template.pts == item->pts)) {
			return true;
		}
	}
	return false;
}

/* this function scans captured GOP's and rearanges them into the display order. */
static bool
bpvr_gop_push_chunk_backward(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop;
	struct bpvr_gop_sequence *sequence;
	bool new_frames=false;

	gop = BLST_SQ_FIRST(&manager->staging);
	if (!gop) {	
		return false;
	}
	manager->state = bgopm_busy; /* state shall be changed by the caller */
	sequence = &gop->meta;
	sequence->ref_cnt = 0;

	/* rearrange GOP's in the reverse (display) order */
	while( NULL != (gop=BLST_SQ_FIRST(&manager->staging))) {
		BLST_SQ_REMOVE_HEAD(&manager->staging, queue);
        sequence->ref_cnt++;
		gop->sequence = sequence;
        gop->gop.lastcdbptr = (void *)sequence->lastscode.cdb;
        if (manager->disp_only_last_first_iframe) {
            if (BLST_SQ_FIRST(&manager->staging) == NULL) {
                /* Feed first GOP, otherwise don't feed */
                gop->sequence = &gop->meta;
                gop->sequence->ref_cnt = 1;
                gop->gop.flags.disp_iframe = true;
                BDBG_MSG(("added (%d) GOP %p",  sequence->ref_cnt, (void *)&gop->gop));
                BDBG_MSG(("push_chunk_backward, Feeding gop to player with disp_iframe flag set"));
                b_pvr_gop_feed_to_player(manager, gop);

            } else {
                BDBG_MSG(("push_chunk_backward, discarding the GOP because disp last or first I-frame flag is set"));
                BLST_SQ_INSERT_HEAD(&manager->free, gop, queue);
            }
        } else {
            if (BLST_SQ_FIRST(&manager->staging) == NULL) {
                /* Indicate to the GOP player that this is the last GOP in this chunk, so that
                 * it can insert the Filler after this GOP.
                 */
                gop->gop.flags.last_gop = true;
                BDBG_ASSERT(manager->scode_filler.prevbaseentryptr);
                gop->gop.sc_filler = manager->scode_filler;
                if (manager->disp_last_first_iframe)
                    gop->gop.flags.disp_iframe = true;
            }
            BDBG_MSG(("added (%d) GOP %p:%p",  sequence->ref_cnt, (void *)&gop->gop, (void *)gop));
            b_pvr_gop_feed_to_player(manager, gop);
        }
		new_frames = true;
	}
	return new_frames;
}

/* this function scans captured GOP's, rearanges them into the display order, removed duplicates/old frames. Returns true if frames were added into the player queue */
static bool
bpvr_gop_push_interleaved(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop,*last_gop=NULL;
    struct bpvr_gop *match_gop=NULL;
	bool skip=true;
	bool seeking=true;
	struct bpvr_gop_sequence *sequence;
	bool new_frames=false;

	gop = BLST_SQ_FIRST(&manager->staging);
	if (!gop) {		
		return false;
	}
	manager->state = bgopm_busy; /* state shall be changed by the caller */
	sequence = &gop->meta;
	sequence->ref_cnt = 0;
	if(manager->last_match == bgopm_match_none) {
		skip = false;
	} else {
        /* First check, if we can find the matching GOP */
        for (gop = BLST_SQ_FIRST(&manager->staging);gop;gop=BLST_SQ_NEXT(gop, queue)) {
			if (bpvr_last_gop_match(manager, &gop->gop)) {
                   match_gop = gop;
                   break;
            }
        }
        if (!match_gop) {
            /* No match found, push all the complete GOPs to player */
            BDBG_WRN(("No overlap found (too small chunk size?), resyncing with stream"));
            manager->last_match = bgopm_match_none;
            skip = false;
        }
    }

	/* we assume that interleaved mode is used for rewind only */
	/* rearrange GOP's in the reverse (display) order */
	while( NULL != (gop=BLST_SQ_FIRST(&manager->staging))) {
		bool add=false;

		BLST_SQ_REMOVE_HEAD(&manager->staging, queue);
		if (!skip) {
			last_gop = gop;
			if ( gop->gop.flags.complete ||
				(sequence->ref_cnt==0 && BLST_SQ_FIRST(&manager->staging)==NULL) /* only GOP in the sequence */ ) {
				add = true;
				if (manager->last_match == bgopm_match_none) {
					add = !bpvr_gop_history_match(manager, &gop->gop);
					BDBG_MSG(("history %s GOP %p",  add?"in":"out", (void *)&gop->gop));
				}
			}
		} else {
			switch(manager->last_match) {
			case bgopm_match_none:
				break;
			case bgopm_match_last_include:
				if (gop == match_gop) {
					skip=false;
				}
				if (gop->gop.flags.complete) {
					add = true;
				}
				break;
			default:
			case bgopm_match_last_exclude:
				if (gop == match_gop) {
					skip=false;
				}
				break;
			}
		}
		BDBG_MSG(("filtered %s(%d) GOP %p",  add?"in":"out", sequence->ref_cnt, (void *)&gop->gop));
		if (add) {
			add = !bpvr_gop_history_match(manager, &gop->gop);
			BDBG_MSG(("history %s GOP %p",  add?"in":"out", (void *)&gop->gop));
		}
		if (add) {
			sequence->ref_cnt++;
			if (seeking) {
				/* we have to use permanent location for the sequence meta information */
				gop->meta = *sequence;
				sequence = &gop->meta;
				seeking = false;
				/* mark very first frame as discontinuity pointer */
				if(manager->last_match == bgopm_match_none) {
					gop->gop.flags.discontinuity = true;
				}
			}
			gop->sequence = sequence;
            gop->gop.lastcdbptr = (void *)sequence->lastscode.cdb;

			bpvr_gop_history_add(manager, &gop->gop);
            if (BLST_SQ_FIRST(&manager->staging) == NULL) {
                gop->gop.flags.last_gop = true;
                BDBG_ASSERT(manager->scode_filler.prevbaseentryptr);
                gop->gop.sc_filler = manager->scode_filler;
            }
            b_pvr_gop_feed_to_player(manager, gop);
			new_frames = true;
		} else {
			BLST_SQ_INSERT_HEAD(&manager->free, gop, queue);
		}
	}
	if(seeking) {
		BDBG_WRN(("resyncing with a stream"));
		manager->last_match = bgopm_match_none;
	} else if(last_gop) {
        bpvr_gop_store_last_gop(manager, &last_gop->gop);		
		if (last_gop->gop.flags.complete) {
			manager->last_match = bgopm_match_last_exclude;
		} else {
			manager->last_match = bgopm_match_last_include;
		}
	}
	return new_frames;
}

/* this function postprocesses parsed GOP's, return true if data was added into the play queue */
static bool
bpvr_gop_push_chunks(bpvr_gop_manager manager)
{
	switch(manager->mode) {
	case BOTF_FeederMode_eInterleaved:
		return bpvr_gop_push_interleaved(manager);
	case BOTF_FeederMode_eChunkBackward:
		return bpvr_gop_push_chunk_backward(manager);
	default:
		return false;
	}
}

static void
bpvr_gop_manager_recycle_scan(bpvr_gop_manager manager)
{
	struct bpvr_gop *gop;
    const bpvr_start_code *scode = NULL;

	/* rescan queue and release unused GOP's */
	while(NULL != (gop=BLST_SQ_FIRST(&manager->complete))) {
		BDBG_ASSERT(gop->sequence);
        BDBG_MSG(("recycle_scan, gop=%p ref_cnt=%d",(void *)&gop->gop, gop->sequence->ref_cnt));
		if (gop->sequence->ref_cnt>0) {
			break; /* can't release GOP which could be in use */
		}
		BDBG_MSG(("recycling GOP %p", (void *)&gop->gop));
        scode = &(gop->sequence->lastscode);
		BLST_SQ_REMOVE_HEAD(&manager->complete,queue);
		BLST_SQ_INSERT_HEAD(&manager->free, gop, queue);
	}
#if BDBG_DEBUG_BUILD
	{
		unsigned i;
		for(i=0,gop=BLST_SQ_FIRST(&manager->complete);gop;gop=BLST_SQ_NEXT(gop, queue)) {
		    BDBG_MSG_TRACE(("GOP:%u:%#x:%d %#x..%#x %s %s %s %s in the processing queue", i, (unsigned)gop, gop->sequence->ref_cnt, (unsigned)gop->gop.itb_meta.start_scode.cdb, (unsigned)gop->sequence->lastscode.cdb, gop->gop.flags.complete?"complete":"", gop->gop.flags.discontinuity?"discontinuity":"", gop->gop.flags.disp_iframe?"disp_iframe":"",gop->gop.flags.last_gop?"last_gop":""));
			i++;
		}
		BDBG_MSG(("%u GOP's in the processing queue", i));
	}
#endif
	if (scode) {
        const void *cdb_ptr;

        cdb_ptr = scode->cdb;
        BDBG_MSG(("Free CDB = %p itb %p",cdb_ptr, scode->prevbaseentryptr));
		manager->done_cb(manager->cb_cntx, cdb_ptr, scode->prevbaseentryptr);
	}
	if (manager->paused) {
		if (BLST_SQ_FIRST(&manager->free)) {
			BDBG_MSG(("resume data acquisition"));
			manager->paused = false;
			manager->resume_cb(manager->cb_cntx);
		}
	}
	return;
}

static void
bpvr_gop_manager_gettimecode(bpvr_gop_manager manager)
{
    uint8_t cdbdata[4];

    botf_scv_parser_getdatabytes(manager->scv_parser, 0, sizeof(cdbdata), cdbdata);
    manager->gop_time_code = (cdbdata[0] << (25-8)) | (cdbdata[1] << (17-8)) |
        (cdbdata[2] << (9-8)) | (cdbdata[3] >> 7);
}

static uint8_t
bpvr_gop_manager_getpicstructure(bpvr_gop_manager manager)
{
    uint8_t cdbdata[1];

    botf_scv_parser_getdatabytes(manager->scv_parser, 2, sizeof(cdbdata), cdbdata);

    return(cdbdata[0] & 0x3);
}

static void
bpvr_gop_manager_getpictype(bpvr_gop_manager manager)
{
    uint8_t scdata[1];

    botf_scv_parser_getdatabytes(manager->scv_parser, 1, sizeof(scdata), scdata);
    manager->pic_type = (scdata[0] >> 3) & 7;
}

static void
bpvr_gop_manager_updatefreeptrs(bpvr_gop_manager manager, const void *cdb, const void *itb)
{
    if(manager->current==NULL && BLST_SQ_FIRST(&manager->complete)==NULL && BLST_SQ_FIRST(&manager->staging)==NULL)
    {
        manager->cdb_free = cdb;
        manager->itb_free = itb;
    }
    return;
}

static void
bpvr_gop_manager_updatefree_scode(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bpvr_gop_manager_updatefreeptrs(manager, scode->cdb, scode->prevbaseentryptr);
    return;
}

static bgopm_state
bpvr_gop_manager_consumemarkersc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    struct bpvr_gop *gop = manager->current;
    uint8_t data[5];

    BDBG_MSG(("Found marker at ITB=%p CDB=%p", scode->itbentryptr, scode->cdb));
    /* Get the chunk counter */
    botf_scv_parser_getdatabytes(manager->scv_parser, 0, sizeof(data), data);


    if (gop != NULL) {
        if(gop->gop.n_frames>0) {
            gop->gop.flags.complete = false;
            gop->meta.lastscode = *scode;

            BDBG_MSG(("closed(%s) gop", gop->gop.flags.complete?"complete":"incomplete"));
            b_pvr_gop_close_gop(manager);
        } else {
            BDBG_MSG(("useless gop %p", (void *)&gop->gop));
            /* recycle GOP */
            BLST_SQ_INSERT_HEAD(&manager->free, gop, queue);
            manager->current = NULL;
            if(manager->chunk_no==B_MARKER_CHUNK_EOF) {
                bpvr_gop_player_force(manager->player);
            }
        }
    } 

    manager->next_pts_valid = false;
    manager->next_pts = 0;
    manager->chunk_no = data[4] | (data[3] << 8) | (data[1] << 16) | (data[0] << 24);
    BDBG_MSG(("gop_manager chunk_no = %u",manager->chunk_no));

    if(manager->chunk_no==B_MARKER_CHUNK_EOF) {
        bpvr_gop_player_force(manager->player);
        BOTF_P_DetectedEof(manager->cb_cntx);
    }

    return(bgopm_seekingFiller);
}

bgopm_state bgopm_seekingSeqHdrGOPFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingSeqHdrGOP;
    unsigned sc = scode->code;

    bpvr_gop_manager_updatefree_scode(manager, scode);

    switch(sc) {
    case B_MPEG2_PICSTARTCODE:
        /*
         * count this frame if our skipped frame counter is enabled
         */
        if (manager->headless_gop_frame_counter.enable) {
            manager->headless_gop_frame_counter.n++;
        }
        break;
    case B_MPEG2_GOPSTARTCODE:
        manager->scode_meta = *scode;
        bpvr_gop_manager_gettimecode(manager);
        /*
         * found the start of a gop, so stop counting skipped frames
         */
        manager->headless_gop_frame_counter.enable = false;
        next_state = bgopm_seekingFirstPic;
        break;
    case B_MPEG2_SEQHEADERCODE:
        /*
         * found a sequence header, so stop counting skipped frames
         */
        manager->headless_gop_frame_counter.enable = false;
        manager->scode_meta = *scode;
        next_state = bgopm_seekingGOP;
        break;

    case B_MARKER_SC:
        next_state = bpvr_gop_manager_consumemarkersc(manager, scode);
        break;
    default:
        if (sc > B_MPEG2_PICSTARTCODE && sc <= B_MPEG2_MAXSLICESTARTCODE) {
            /* Ignore slice start codes */
        }
        break;
    }
    return next_state;
}

bgopm_state bgopm_seekingGOPFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingGOP;
    unsigned sc = scode->code;

    switch(sc) {
    case B_MPEG2_PICSTARTCODE:
        /* if there is no GOP header after sequence header */
        if(manager->prev_state == bgopm_seekingSeqHdrGOP || manager->prev_state == bgopm_seekingSlicePic) {
            manager->gop_time_code = B_UNKNOWN_TIME_CODE; /* there is no time code */
            next_state = bgopm_seekingFirstPicFunc(manager, scode); /* short circuit state-machine */
        } else {
            bpvr_gop_manager_updatefree_scode(manager, scode);
            next_state = bgopm_seekingSeqHdrGOP;
        }
        break;
    case B_MPEG2_GOPSTARTCODE:
        bpvr_gop_manager_gettimecode(manager);
        next_state = bgopm_seekingFirstPic;
        break;
    case B_MPEG2_SEQHEADERCODE:
        bpvr_gop_manager_updatefree_scode(manager, scode);
        manager->scode_meta = *scode;
        break;
    case B_MARKER_SC:
        next_state = bpvr_gop_manager_consumemarkersc(manager, scode);
        break;

    default:
        if (sc > B_MPEG2_PICSTARTCODE && sc <= B_MPEG2_MAXSLICESTARTCODE) {
            bpvr_gop_manager_updatefree_scode(manager, scode);
            next_state = bgopm_seekingSeqHdrGOP;
        }
        break;
    }
    return next_state;
}

bgopm_state bgopm_seekingFirstPicFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingFirstPic;
    unsigned sc = scode->code;

    switch(sc) {
    case B_MPEG2_PICSTARTCODE:
        bpvr_gop_manager_getpictype(manager);
        b_pvr_gop_create(manager, scode);
        manager->scode_pic = *scode;
        manager->bIsField = false;
        manager->FieldPicture = 0;
        next_state = bgopm_seekingPCEFirstSlice;
        break;
    case B_MPEG2_GOPSTARTCODE:
        bpvr_gop_manager_updatefree_scode(manager, scode);
        manager->scode_meta = *scode;
        break;
    case B_MPEG2_SEQHEADERCODE:
        bpvr_gop_manager_updatefree_scode(manager, scode);
        manager->scode_meta = *scode;
        next_state = bgopm_seekingGOP;
        break;
    case B_MARKER_SC:
        next_state = bpvr_gop_manager_consumemarkersc(manager, scode);
        break;
    default:
        if (sc > B_MPEG2_PICSTARTCODE && sc <= B_MPEG2_MAXSLICESTARTCODE) {
            bpvr_gop_manager_updatefree_scode(manager, scode);
            next_state = bgopm_seekingSeqHdrGOP;
        }
        break;
    }
    return next_state;
}

bgopm_state bgopm_seekingPCEFirstSliceFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingPCEFirstSlice;
    struct bpvr_gop *gop = manager->current;
    unsigned sc = scode->code;
    uint8_t picstruct;

    switch(sc) {
    case B_MPEG2_PICSTARTCODE:
        b_pvr_gop_flush(manager, scode);
        next_state = bgopm_seekingSeqHdrGOP;
        break;
    case B_MPEG2_GOPSTARTCODE:
        b_pvr_gop_flush(manager, scode);
        next_state = bgopm_seekingFirstPic;
        manager->scode_meta = *scode;
        break;
    case B_MPEG2_SEQHEADERCODE:
        b_pvr_gop_flush(manager, scode);
        manager->scode_meta = *scode;
        next_state = bgopm_seekingGOP;
        break;

    case B_MARKER_SC:
        next_state = bpvr_gop_manager_consumemarkersc(manager, scode);
        break;

    default:
        if (sc > B_MPEG2_PICSTARTCODE && sc <= B_MPEG2_MAXSLICESTARTCODE) {
            if (!manager->bIsField ||(manager->bIsField && (manager->FieldPicture == 1))) {
                b_itb_fill_segment(manager, &(gop->gop.frames[gop->gop.n_frames].itb_meta), &(manager->scode_pic), scode);
                manager->scode_pic = *scode;
            }
            next_state  = bgopm_seekingSlicePic;
        } else if ((sc == B_MPEG2_EXTENSIONSTARTCODE) && (manager->PrevScode == B_MPEG2_PICSTARTCODE)) {
            /* Check if this is a frame or field */
            picstruct = bpvr_gop_manager_getpicstructure(manager);
            if (picstruct == B_MPEG2_TOPFIELD || picstruct == B_MPEG2_BOTTOMFIELD) {
                manager->bIsField = true;
                manager->FieldPicture++;
            }
        }
        break;
    }
    return next_state;
}

bgopm_state bgopm_seekingSlicePicFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingSlicePic;
    struct bpvr_gop *gop = manager->current;
    unsigned sc = scode->code;

    switch(sc) {
    case B_MPEG2_PICSTARTCODE:
        if (manager->bIsField && manager->FieldPicture < 2) {
            /* This is a second field picture */
            next_state  = bgopm_seekingPCEFirstSlice;
            break;
        }
        BDBG_MSG(("found picture payload"));
        b_itb_fill_segment(manager, &(gop->gop.frames[gop->gop.n_frames].itb_slices), &(manager->scode_pic), scode);
        b_pvr_gop_finish_frame(manager);
        if (gop->gop.n_frames>=BOTF_GOP_MAX_FRAMES) {
            BDBG_WRN(("detected too large gop, %d frames, aborting it", gop->gop.n_frames));
            BDBG_MSG(("incomplete gop %p %d frames at %p", (void *)&gop->gop, gop->gop.n_frames, scode->itbentryptr));
            gop->gop.flags.complete = false;
            gop->meta.lastscode = *scode;
            b_pvr_gop_close_gop(manager);
            next_state = bgopm_seekingSeqHdrGOP;
            return next_state;
        }
        manager->bIsField = false;
        manager->FieldPicture = 0;
        bpvr_gop_manager_getpictype(manager);
        b_pvr_gop_init_frame(manager);
        manager->scode_pic = *scode;
        next_state  = bgopm_seekingPCEFirstSlice;
        break;
    case B_MPEG2_GOPSTARTCODE:
        b_itb_fill_segment(manager, &(gop->gop.frames[gop->gop.n_frames].itb_slices), &(manager->scode_pic), scode);
        b_pvr_gop_finish_frame(manager);
        gop->gop.flags.complete = true;
        if (manager->mode == BOTF_FeederMode_eChunkForward || manager->mode == BOTF_FeederMode_eChunkBackward) {
            gop->gop.flags.discontinuity = true ; /* for non continous modes all gops marked as discontinuity (it causes in display path to remove all open GOP B frames */
        }
        gop->meta.lastscode = *scode;
        manager->scode_meta = *scode;
        BDBG_MSG(("completed gop %p %#x %u:%u frames at %p", (void *)&gop->gop, (unsigned)gop->gop.time_code, gop->gop.frames[0].info.pts, gop->gop.n_frames, scode->itbentryptr));
        b_pvr_gop_close_gop(manager);
        bpvr_gop_manager_gettimecode(manager);
        next_state  = bgopm_seekingFirstPic;
        break;
    case B_MPEG2_SEQHEADERCODE:
        b_itb_fill_segment(manager, &(gop->gop.frames[gop->gop.n_frames].itb_slices), &(manager->scode_pic), scode);
        b_pvr_gop_finish_frame(manager);
        gop->gop.flags.complete = true;
        if (manager->mode == BOTF_FeederMode_eChunkForward || manager->mode == BOTF_FeederMode_eChunkBackward) {
            gop->gop.flags.discontinuity = true ; /* for non continous modes all gops marked as discontinuity (it causes in display path to remove all open GOP B frames */
        }
        gop->meta.lastscode = *scode;
        manager->scode_meta = *scode;
        BDBG_MSG(("completed gop %p %#x %u:%u frames at %p", (void *)&gop->gop, (unsigned)gop->gop.time_code, gop->gop.frames[0].info.pts, gop->gop.n_frames, scode->itbentryptr));
        b_pvr_gop_close_gop(manager);
        next_state  = bgopm_seekingGOP;
        break;
    case B_MARKER_SC:
        next_state = bpvr_gop_manager_consumemarkersc(manager, scode);
        break;
    default:
        if (sc > B_MPEG2_PICSTARTCODE && sc <= B_MPEG2_MAXSLICESTARTCODE){
            /* Ignore slice start codes */
        }
        else {
            b_pvr_gop_flush(manager, scode);
            next_state = bgopm_seekingSeqHdrGOP;
        }
        break;
    }
    return next_state;
}

/* Filler is injected immeadiately after marker sc */
bgopm_state bgopm_seekingFillerFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state next_state = bgopm_seekingSeqHdrGOP;
    struct bpvr_gop *gop = manager->current;

    BDBG_MSG_TRACE(("bgopm_seekingFillerFunc: %#x", (unsigned)manager));
    /* Filler sc is used to push the last frame to display. Adding Filler SC
     * makes XVD to release the last frame CDB/ITB
     */
    BDBG_ASSERT(scode->prevbaseentryptr);
    manager->scode_filler = *scode;

    /* In chunk forward mode, feed the last gop in chunk here.
     * If we are here in sequnetial mode, that means OTF is supposed
     * to display last or first I-frame based on the command received.
     */
    if (manager->mode == BOTF_FeederMode_eChunkForward || manager->mode == BOTF_FeederMode_eSequential) {
        gop = NULL;
        if (manager->current != NULL) {
            gop = manager->current;
            manager->current = NULL;
        } else {
            if (manager->prev != NULL) {
                gop = manager->prev;
                manager->prev = NULL;
            }
        }
        if (gop != NULL) {
            if (manager->disp_only_last_first_iframe || manager->disp_last_first_iframe) {
                BDBG_MSG(("Feeding gop to player with disp_iframe flag set"));
                gop->gop.flags.disp_iframe = true;
            }
            gop->gop.sc_filler = manager->scode_filler;
            b_pvr_gop_feed_to_player(manager, gop);
        }
    }

    bpvr_gop_push_chunks(manager);
    bpvr_gop_manager_recycle_scan(manager);

    /*
     * transitioning from a marker packet to a sequence header
     * reset and enable the counting of skipped frames at the head of a chunk
     */
    manager->headless_gop_frame_counter.n = 0;
    manager->headless_gop_frame_counter.enable = true;
    return next_state;
}

bgopm_state bgopm_busyFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    BSTD_UNUSED(manager);
    BSTD_UNUSED(scode);
    return bgopm_busy;
}

bgopm_state bgopm_discardUntilMarkerFunc(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    struct bpvr_gop *gop;

    if (scode->code == B_MARKER_SC) {
        BDBG_MSG(("Discarded data until %p", scode->cdb));

        gop=BLST_SQ_LAST(&manager->complete);
        if (gop != NULL) {
            gop->sequence->lastscode = *scode;
        } else {
            manager->cdb_free = scode->cdb;
            manager->itb_free = scode->prevbaseentryptr;
        }
        return bgopm_seekingSeqHdrGOP;
    } else {
        return bgopm_discardUntilMarker;
    }
}

bool
bpvr_gop_manager_feed(bpvr_gop_manager manager, const bpvr_start_code *scode)
{
    bgopm_state new_state;
	BDBG_ASSERT(manager->paused==false); /* we shall never be called while in the pause state */

    manager->itb_free = NULL;
    manager->cdb_free = NULL;


    /* Call the function based on the current state */
    BDBG_ASSERT(manager->state < sizeof(b_gopm_states)/sizeof(*b_gopm_states));
    BDBG_ASSERT(manager->state == b_gopm_states[manager->state].state); /* verify that order of states in array matches enum value */
    BDBG_MSG_TRACE(("state:%s(%u) prev:%s(%u) startcode:%#x", b_gopm_states[manager->state].name, manager->state, b_gopm_states[manager->prev_state].name, manager->prev_state, scode->code));

    new_state = b_gopm_states[manager->state].action(manager, scode);
    if(manager->state!=new_state) {
        manager->prev_state = manager->state;
        manager->state = new_state;
    }

    manager->PrevScode = scode->code;

	if (manager->cdb_free || manager->itb_free) {
		manager->done_cb(manager->cb_cntx, manager->cdb_free, manager->itb_free);
	}
	
	return manager->paused;
}

static void
bpvr_gop_manager_recycle(void *cntx, bpvr_gop_desc *gop_desc)
{
	bpvr_gop_manager manager = cntx;
	struct bpvr_gop *gop;

	BDBG_MSG(("freeing GOP %p", (void *)gop_desc));

	for(gop=BLST_SQ_FIRST(&manager->complete);gop;gop=BLST_SQ_NEXT(gop, queue)) {
		if (gop_desc == &gop->gop) {
			gop->sequence->ref_cnt--;
			BDBG_MSG(("[+]recycling GOP %p %p %d", (void *)&gop->gop, (void *)gop->sequence, gop->sequence->ref_cnt));
			if(manager->state != bgopm_busy) {
					bpvr_gop_manager_recycle_scan(manager);
			} else {
				BDBG_MSG(("capturer busy, rescan delayed"));
			}
			return;
		}
	}
	BDBG_WRN(("unknown gop %p", (void *)gop_desc));
	return;
}

void
bpvr_gop_manager_force(bpvr_gop_manager manager, const void *cdb_ptr, const void *scv_ptr)
{   	
    BSTD_UNUSED(cdb_ptr);
    BSTD_UNUSED(scv_ptr);

	BDBG_MSG(("bpvr_gop_manager_force"));
    if (manager->mode == BOTF_FeederMode_eInterleaved || manager->mode == BOTF_FeederMode_eChunkBackward) {
        /* We came here because input buffers are full and decoder stopped consuming data.
         * In chunkbackward or interleaved mode, OTF needs atleast two chunks to be in the
         * input CDB to properly perform trickmodes. If the application feeds more data per
         * chunk, OTF can get stuck. To get out of this, OTF consumes the data whatever it hasn't
         * consumed and discards the data until next marker */
        if (manager->state == bgopm_seekingPCEFirstSlice || manager->state == bgopm_seekingSlicePic) {
            botf_scv_parser_getlastscode(manager->scv_parser, &(manager->last_scode));
            b_pvr_gop_flush(manager, &(manager->last_scode));
            BDBG_MSG(("Discard data from %p until next marker", manager->last_scode.cdb));
        }
        bpvr_gop_push_chunks(manager);
        manager->state = bgopm_discardUntilMarker;
    }
    bpvr_gop_player_force(manager->player);
}

bpvr_gop_player
bpvr_gop_manager_player( bpvr_gop_manager manager)
{
	return manager->player;
}

void
bpvr_gop_manager_init_cfg(bpvr_gop_manager_cfg *cfg)
{
	BKNI_Memset(cfg, 0, sizeof(*cfg));
	return;
}

unsigned long bpvr_gop_manager_get_pts(bpvr_gop_manager manager)
{
    return manager->last_pts;
}
