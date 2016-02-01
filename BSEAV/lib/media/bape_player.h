/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
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
 * APE library, player interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAPE_PLAYER_H__
#define _BAPE_PLAYER_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bape_player *bape_player_t;

bape_player_t bape_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
void bape_player_destroy(bape_player_t player);
int bape_player_next(bape_player_t player, bmedia_player_entry *entry);
void bape_player_tell(bape_player_t player, bmedia_player_pos *pos);
void bape_player_get_status(bape_player_t player, bmedia_player_status *status);
int bape_player_set_direction(bape_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode);
int bape_player_seek(bape_player_t player, bmedia_player_pos pos);

extern const bmedia_player_methods bape_player_methods;

#ifdef __cplusplus
}
#endif


#endif /* _BAPE_PLAYER_H__ */

