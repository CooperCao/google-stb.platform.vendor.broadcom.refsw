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
 * GOP player module for the on the fly PVR
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BOTF_GOP_PLAYER_H
#define BOTF_GOP_PLAYER_H

#include "botf_gop_manager.h"

/* bpvr_gop_manager.h defines bpvr_gop_player type */

#ifdef __cplusplus
extern "C"
{
#endif

#define BOTF_DISPLAY_I (1<<0)
#define BOTF_DISPLAY_P (1<<1)
#define BOTF_DISPLAY_B (1<<2)

typedef enum bpvr_rate_control {
	bpvr_rate_control_frame,  
	bpvr_rate_control_field,
	bpvr_rate_control_pts
} bpvr_rate_control;

typedef struct bpvr_gop_player_mode {
	bool rewind; /* direction rewind or forward */
	bpvr_rate_control rate_control;
	unsigned frame_filter;
	unsigned rate; /* BPVR_RATE_MODULO based, 200 -> 2xplayback, 150 1.5xplayback, 133 1.33xplayback, 400 4xplayback, slow motion is not supported */
	unsigned pts_atom; /* number of PTS ticks (27MHz or 45KHz) which correspond to the display time of one video frame. Used in combination with rate in the PTS controled rate management */
} bpvr_gop_player_mode;

typedef void (*bpvr_gop_done)(void *cntx, bpvr_gop_desc *gop);

struct BOTF_ParserPtrs;

/* this function creates new GOP player */
bpvr_gop_player 
bpvr_gop_player_create(
		bpvr_gop_done done_cb, /* callback to call when GOP is not needed */
		void *cb_cntx,  /* context to be passed into the callback function */        
        void *otf_cntx,
        struct BOTF_ParserPtrs *OPParserPtrs,
        const struct BOTF_ParserPtrs *IPParserPtrs
		);

void
bpvr_gop_player_setmembase(bpvr_gop_player player, int32_t membase);

/* this function feeds complete GOP descriptor into the player */
void
bpvr_gop_player_feed(
		bpvr_gop_player player,  /* instance of the gop player */
		bpvr_gop_desc *gop /* GOP descriptor */
		);

/* this function is used to set playback mode, it shall be called before any gop was sent to the player */
void
bpvr_gop_player_set_mode(
		bpvr_gop_player player,  /* instance of the gop player */
		bpvr_gop_player_mode *mode /* player mode */
		);

/* this function is used to get current playback mode, it shall be caleed after before any gop was sent to the player */
void
bpvr_gop_player_get_mode(
		bpvr_gop_player player,  /* instance of the gop player */
		bpvr_gop_player_mode *mode /* player mode */
		);

/* this function informs GOP player that CDB buffer is full, and if GOP player is waiting for more data to come
 * it shall decode existing GOP and recycle them before new GOP would arrive */
void
bpvr_gop_player_force(
		bpvr_gop_player player  /* instance of the gop player */
		);
	
/* 
* this function is used to reset GOP player into initial state, it should remove all pending data before return 
*/
void 
bpvr_gop_player_reset(
		bpvr_gop_player player  /* instance of the gop player */
	);

/* 
* this function releases resources allocated by the GOP player
*/
void 
bpvr_gop_player_destroy(
		bpvr_gop_player player  /* instance of the gop player */
	);


/* this function activated feed to the mpeg decoder */
void 
bpvr_gop_player_start(
		bpvr_gop_player player /* instance of the gop player */
		);

/* Release the GOPs which are played */
void bpvr_gop_freecompleted(bpvr_gop_player player);

/* Stop gop player */
void bpvr_gop_player_stop(bpvr_gop_player player);

#ifdef __cplusplus
}
#endif


#endif /* BOTF_GOP_PLAYER_H */

