/***************************************************************************
 *	   Copyright (c) 2007-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * GOP manager module for the on the fly PVR
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BOTF_GOP_MANAGER_H
#define BOTF_GOP_MANAGER_H

#include "botf.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bpvr_start_code {
	const void *cdb; /* pointer to the start code in CDB */    
    const void *itbentryptr; /* ITB entry pointer */
    const void *prevbaseentryptr; /* pointer to prev base entry ITB ptr */
    uint8_t code; /* start code */
    uint8_t index; /* Index of this start code in this entry */
} bpvr_start_code;

/* this structure is used to describe itb data associated with a segment */
typedef struct {    
    bpvr_start_code start_scode; 
    bpvr_start_code end_scode; 
} bpvr_itb_segment;

typedef struct {
	unsigned pic_tag; /* picture tag - sequentially increasing number, used as key for lookup */
	uint32_t pts;  	   /* PTS value (format specific) */
	/* unsigned first_slice_type; not implemented */ /* type of the first slice as per MPEG spec */
	uint8_t frame_no; /* frame number in the GOP */
	uint8_t pic_type; /* picture type as per MPEG spec */

	struct {
	  bool repeat_first_field:1; /* value of corresponding fieild in the mpeg header */
      bool tag:1; /* true if tag information is valid */
	  bool pts:1; /* true if pts information is valid */
	} flags;
} bpvr_pic_info;

/* maximum value of the picture tag */
#define B_PIC_TAG_MAX	0x10000
/* #define B_PIC_TAG_MAX	0x400 */
/* invalid picture tag */
#define B_PIC_TAG_INVALID 0
/* maximum number of GOPS in the CDB buffer */
#define B_PVR_MAX_GOPS 60
/* maximum number of FRAMES in the GOP */
#define BOTF_GOP_MAX_FRAMES	48

#define B_FRAME_I	1
#define B_FRAME_P	2
#define B_FRAME_B	3

typedef struct bpvr_gop_desc {	
	unsigned chunk_no; /* chunk number as sent by the feeder */
	uint32_t time_code; /* time_code field from the GOP header */     
	uint8_t gop_no;  /* GOP number in the chunk */
	uint8_t n_frames; /* number of complete frames in the GOP */
	struct {
		bool complete:1; /* true if GOP is complete */
		bool discontinuity:1; /* true if discontinuity was detected */
        bool disp_iframe:1; /* if true, override decimation and display I-frame in this GOP */
        bool last_gop:1; /* if true, this is the last gop in a chunk */
	} flags;	
    bpvr_itb_segment itb_meta;    
	struct {
		bpvr_pic_info info;		
        bpvr_itb_segment itb_meta; 		
        bpvr_itb_segment itb_slices; /* Slices ITB segment */
	} frames[BOTF_GOP_MAX_FRAMES]; /* frame segments */
    const void *lastcdbptr; /* Last CDB pointer used by the sequence, used for recycling */
    bpvr_start_code sc_filler; /* Filler (userdata) start code */
    uint32_t headless_gop_frame_count;
} bpvr_gop_desc;

typedef enum bpvr_stream_type {
	bpvr_stream_type_mpeg, /* MPEG (transport, program or PES) */
	bpvr_stream_type_dss  /* DSS stream */
} bpvr_stream_type;

typedef struct bpvr_gop_manager_impl *bpvr_gop_manager;

typedef struct botf_scv_parser_impl *botf_scv_parser; 

typedef struct bpvr_gop_player_impl *bpvr_gop_player; /* this is a forward declaration for the gop_player, it needed because gop_player and gop_manager, are mutually recursive */
#ifdef __cplusplus
}
#endif

#include "botf_gop_player.h"

#ifdef __cplusplus
extern "C"
{
#endif
struct BOTF_ParserPtrs;
typedef struct bpvr_gop_manager_cfg {
	void *cdb_cntx;  /* context of the CDB feeder */
	void (*cdb_done)(void *cntx, const void *cdb, const void *itb); /* done callback for the CDB feeder */
	void (*cdb_resume)(void *cntx); /* resume callback for the CDB feeder */
	struct BOTF_ParserPtrs *OPParserPtrs;
    const struct BOTF_ParserPtrs *IPParserPtrs;
    bool     bUsePtsAsTag;              /* If true, use PTS as picture tag, otherwise uses the tag specified in stream
                                         * using pictag BTP command */
} bpvr_gop_manager_cfg;

/* this function is used to initialize the GOP manager configuration structure */
void
bpvr_gop_manager_init_cfg(
		bpvr_gop_manager_cfg *cfg
);

/* this function creats new gop manager */
bpvr_gop_manager
bpvr_gop_manager_create(
		bpvr_gop_manager_cfg *cfg /* configuration of the GOP manager */
		);


/* this function is called when CDB buffer full, it could cause premature release of incomplete GOP's */
void 
bpvr_gop_manager_force(
		bpvr_gop_manager manager, /* instance of the GOP manager */
		const void *cdb_ptr,     /* last occupied byte in the CDB FIFO */
        const void *itb_ptr     /* last occupied byte in the ITB FIFO */
		);

/* this function resets state of the GOP manager */
void
bpvr_gop_manager_reset(
		bpvr_gop_manager manager, /* instance of the GOP manager */
		BOTF_FeederMode mode,  /* new feeder mode */
		bpvr_stream_type stream /* new stream type */
	);

/* this function returns gop_player created by gop_manager */
bpvr_gop_player 
bpvr_gop_manager_player(
		bpvr_gop_manager manager /* instance of the gop manager */
		);

/* this function is used to set PTS for the next frame */
void
bpvr_gop_manager_set_pts(
		bpvr_gop_manager manager,  /* instance of the GOP manager */
		uint32_t pts /* PTS value */
		);

/* this function is used to update the tag */
void
bpvr_gop_manager_set_tag(
		bpvr_gop_manager manager,  /* instance of the GOP manager */
		uint32_t tag /* tag value */
		);

void bpvr_gop_manager_set_disp_only_lfiframe(bpvr_gop_manager manager, uint8_t disp_iframe);
void bpvr_gop_manager_set_disp_lfiframe(bpvr_gop_manager manager, uint8_t disp_iframe);

/* this function releases resources associated with the GOP manager */
void
bpvr_gop_manager_destroy(
		bpvr_gop_manager manager /* instance of the GOP manager */
	);

bool
bpvr_gop_manager_feed(
		bpvr_gop_manager manager, /* instance of the GOP manager */
		const bpvr_start_code *scode /* start code */
		);

void
bpvr_gop_manager_setscvparser(bpvr_gop_manager manager, botf_scv_parser scv_parser);

unsigned long bpvr_gop_manager_get_pts(bpvr_gop_manager manager);

#ifdef __cplusplus
}
#endif


#endif /* BOTF_GOP_MANAGER_H*/

