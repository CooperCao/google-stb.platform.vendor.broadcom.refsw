/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
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
 * MKV library, player interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMKV_PLAYER_H__
#define _BMKV_PLAYER_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmkv_player *bmkv_player_t;

#define B_MKV_AUX_TAG_PAYLOAD   0
#define B_MKV_AUX_TAG_CODEC_PRIVATE 1
#define B_MKV_AUX_TAG_DURATION  2
#define B_MKV_AUX_TAG_ATTACHMENT_LINK   3

bmkv_player_t bmkv_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
void bmkv_player_destroy(bmkv_player_t player);
int bmkv_player_next(bmkv_player_t player, bmedia_player_entry *entry);
void bmkv_player_tell(bmkv_player_t player, bmedia_player_pos *pos);
void bmkv_player_get_status(bmkv_player_t player, bmedia_player_status *status);
int bmkv_player_set_direction(bmkv_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale);
int bmkv_player_seek(bmkv_player_t player, bmedia_player_pos pos);

extern const bmedia_player_methods bmkv_player_methods;

#ifdef __cplusplus
}
#endif


#endif /* _BMKV_PLAYER_H__ */

