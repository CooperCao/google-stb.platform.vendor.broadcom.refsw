/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * media navigational indexer
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BTIME_INDEXER_H__
#define _BTIME_INDEXER_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct btime_indexer *btime_indexer_t;

typedef struct btime_indexer_position {
	uint64_t offset; /* interpolated position */
	bmedia_player_pos time; /* interpolated time */
	unsigned byterate; /* interpolated byterate bytes/sec */
	struct {
		bmedia_player_pos time; 
		uint32_t timestamp;
		uint64_t offset;
	} prev, next;  /* data that was used for interpolation */
} btime_indexer_position;

typedef struct btime_indexer_status {
	unsigned nentries;
	unsigned byterate;
    bool byterate_valid; /* if set to false then byterate is not calculated based on the stream, but from other factors */
	bmedia_player_pos duration;
    bmedia_player_pos position; /* last position */
} btime_indexer_status;

typedef enum btime_indexer_direction {
    btime_indexer_direction_forward,
    btime_indexer_direction_backward
} btime_indexer_direction;

typedef struct btime_indexer_location {
    uint32_t timestamp;
    btime_indexer_direction direction;
    uint64_t offset;
} btime_indexer_location;

typedef struct btime_indexer_byterate_limit {
    unsigned low;
    unsigned high;
    unsigned default_;
} btime_indexer_byterate_limit;

/* create index every 60 seconds (one minute) */
#define BINDEX_TIME_STEP	(60*BMEDIA_PLAYER_POS_SCALE)
/* when seeking create index entry every 10 minutes */
#define BINDEX_SEEK_STEP    (10*BINDEX_TIME_STEP)

btime_indexer_t btime_indexer_create(void);
void btime_indexer_destroy(btime_indexer_t index);
int btime_indexer_add(btime_indexer_t index, uint32_t timestamp, uint64_t off, bmedia_player_pos *position);
void btime_indexer_seek(btime_indexer_t index, bmedia_player_pos time);
bool btime_indexer_position_by_time(btime_indexer_t index, bmedia_player_pos time, btime_indexer_position *position);
void btime_indexer_get_status(btime_indexer_t index, btime_indexer_status *status);
void btime_indexer_dump(btime_indexer_t index);
void btime_indexer_get_byterate_limit(btime_indexer_t index, btime_indexer_byterate_limit *limit);
void btime_indexer_set_byterate_limit(btime_indexer_t index, const btime_indexer_byterate_limit *limit);
int btime_indexer_get_time_by_location(btime_indexer_t index, const btime_indexer_location *location, bmedia_player_pos *pos);

#ifdef __cplusplus
}
#endif


#endif /* _BTIME_INDEXER_H__ */

