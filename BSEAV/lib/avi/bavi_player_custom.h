/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * AVI library, player interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAVI_PLAYER_CUSTOM_H__
#define _BAVI_PLAYER_CUSTOM_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bavi_player_custom *bavi_player_custom_t;

bavi_player_custom_t bavi_player_custom_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
void bavi_player_custom_destroy(bavi_player_custom_t player);
int bavi_player_custom_next(bavi_player_custom_t player, bmedia_player_entry *entry);
void bavi_player_custom_tell(bavi_player_custom_t player, bmedia_player_pos *pos);
void bavi_player_custom_get_status(bavi_player_custom_t player, bmedia_player_status *status);
void bavi_player_custom_set_direction(bavi_player_custom_t player, bmedia_player_step direction);
int bavi_player_custom_seek(bavi_player_custom_t player, bmedia_player_pos pos);

extern const bmedia_player_methods bavi_player_custom_methods;

#ifdef __cplusplus
}
#endif


#endif /* _BAVI_PLAYER_CUSTOM_H__ */

