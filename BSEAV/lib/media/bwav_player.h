/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
 * WAV library, player interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BWAV_PLAYER_H__
#define _BWAV_PLAYER_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bwav_player *bwav_player_t;

bwav_player_t bwav_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
void bwav_player_destroy(bwav_player_t player);
int bwav_player_next(bwav_player_t player, bmedia_player_entry *entry);
void bwav_player_tell(bwav_player_t player, bmedia_player_pos *pos);
void bwav_player_get_status(bwav_player_t player, bmedia_player_status *status);
int bwav_player_set_direction(bwav_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode);
int bwav_player_seek(bwav_player_t player, bmedia_player_pos pos);

extern const bmedia_player_methods bwav_player_methods;

#ifdef __cplusplus
}
#endif


#endif /* _BWAV_PLAYER_H__ */

