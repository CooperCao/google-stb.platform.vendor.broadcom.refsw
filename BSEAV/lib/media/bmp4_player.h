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
 * MP4 library, player interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMP4_PLAYER_H__
#define _BMP4_PLAYER_H__

#include "bmedia_player.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmp4_player *bmp4_player_t;

bmp4_player_t bmp4_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream);
void bmp4_player_destroy(bmp4_player_t player);
int bmp4_player_next(bmp4_player_t player, bmedia_player_entry *entry);
void bmp4_player_tell(bmp4_player_t player, bmedia_player_pos *pos);
void bmp4_player_get_status(bmp4_player_t player, bmedia_player_status *status);
int bmp4_player_set_direction(bmp4_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale);
int bmp4_player_seek(bmp4_player_t player, bmedia_player_pos pos);

extern const bmedia_player_methods bmp4_player_methods;

typedef struct bmp4_player_drm_info {
    enum {bmp4_player_drm_info_frame, bmp4_player_drm_info_track_fragment} type;
    union {
        struct {
            off_t offset;
            size_t length;
            bfile_buffer_t buffer;
        } track_fragment;
        struct {
            unsigned sample_no;
        } frame;
    } data;
} bmp4_player_drm_info;

#ifdef __cplusplus
}
#endif


#endif /* _BMP4_PLAYER_H__ */

