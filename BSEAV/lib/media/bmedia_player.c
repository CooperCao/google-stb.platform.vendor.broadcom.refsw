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
 * BMedia library, player interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bmedia_player.h"
#include "bmedia_player_generic.h"
#include "bmedia_player_es.h"
#include "bmp4_player.h"
#include "bmkv_player.h"
#include "bwav_player.h"
#include "bape_player.h"
#include "bogg_player.h"
#include "bmpeg2ts_player.h"
#include "bmpeg2pes_player.h"
#if B_HAS_AVI
#include "bavi_player.h"
#include "bavi_player_custom.h"
#endif

#if B_HAS_ASF
#include "basf_player.h"
#endif

#if B_HAS_RMFF
#include "brmff_player.h"
#endif

#if B_HAS_NAV_PLAYER
#include "bmedia_player_nav.h"
#endif



BDBG_MODULE(bmedia_player);

#define BDBG_MSG_FLOW(x)   /* BDBG_MSG(x) */

BDBG_OBJECT_ID(bmedia_player_t);

struct bmedia_player {
    BDBG_OBJECT(bmedia_player_t)
    void *player;
    const bmedia_player_methods *methods;

    /* data that is used to account in mapping from PTS value to the position in the stream, it accounts to scaling of PTS'es when doing trickmodes */
    bmedia_player_pos index_offset; /* player position that corresponds to the 0 PTS value, it's needed to account for the PTS wraparound */
    bmedia_time_scale time_scale; /* current scale of PTS values */
    bmedia_player_pos last_position; /* last known good position */
    uint32_t last_pts;
    bstream_mpeg_type stream_format;
    bool last_pts_valid;
    bool last_continous; /* if previous data sent to decoder was not continuous */
    bool last_mode_valid;
    bmedia_player_decoder_mode last_mode; /* saved previous mode */
    bmedia_time_scale max_decoder_rate; /* maximum fast forward rate supported by the decoder */
    bmedia_player_decoder_config decoder_features;
};

void
bmedia_player_init_stream(bmedia_player_stream *stream)
{
    unsigned i;
    BKNI_Memset(stream, 0, sizeof(*stream));
    stream->format = bstream_mpeg_type_es;
    stream->stream.es.audio_codec = baudio_format_unknown;
    stream->stream.es.video_codec = bvideo_codec_unknown;
    stream->stream.mpeg2ts.packet_size = 188;
    stream->stream.noindex.auto_rate = true;
    stream->stream.noindex.bitrate = 1024*1024;
    stream->stream.id.master = 0xE0;
    for(i=0;i<BMEDIA_PLAYER_MAX_TRACKS;i++) {
        stream->stream.id.other[i] = 0xC0+i;
        stream->drop[i] = 0;
        stream->stream.es.other_video[i] = bvideo_codec_unknown;
        stream->stream.es.other_audio[i] = baudio_format_unknown;
    }
    return;
}

static void
b_media_player_error_detected(void *cntx)
{
    BSTD_UNUSED(cntx);
    return;
}

static void
b_media_player_decoder_config_init(bmedia_player_decoder_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    config->max_decoder_rate = 1.2 * BMEDIA_TIME_SCALE_BASE;
    config->host_mode = bmedia_player_host_trick_mode_auto;
    config->mode_modifier = 1;
    config->brcm = true;
    config->video_buffer_size = 1024 * 1024;
    return;
}

void
bmedia_player_init_config(bmedia_player_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    config->error_detected = b_media_player_error_detected;
    config->decoder_features.brcm = true;
    config->timeshifting = false;
    config->prefered_read_size = BIO_BLOCK_SIZE;
    config->max_data_parsed = 1024*1024;
    config->reorder_timestamps = true;
    config->autoselect_player = true;
    config->max_pes_size = 0;
    config->key_frame_distance = 0;
    config->force_seek_step = 0;
    config->format.mp4.fragmentBufferSize = 0;
    b_media_player_decoder_config_init(&config->decoder_features);
    return;
}

bmedia_player_t
bmedia_player_create(bfile_io_read_t fd, const bmedia_player_config *config, const bmedia_player_stream *stream)
{
    bmedia_player_t player;
    const bmedia_player_methods *methods;
    bstream_mpeg_type type;
    BERR_Code rc;

    BSTD_UNUSED(rc);
    BDBG_ASSERT(stream);
    BDBG_ASSERT(config);
    BDBG_ASSERT(fd);

    type = stream->format;

    if(stream->without_index) {
        switch(type) {
        case bstream_mpeg_type_es:
            if(stream->stream.es.video_codec == bvideo_codec_unknown &&
                    (stream->stream.es.audio_codec == baudio_format_mp3 || stream->stream.es.audio_codec == baudio_format_mpeg ||
                    stream->stream.es.audio_codec == baudio_format_aac ||
                    stream->stream.es.audio_codec == baudio_format_ac3 ||
                    stream->stream.es.audio_codec == baudio_format_ac3_plus
                    ) &&
                    stream->stream.noindex.auto_rate) {
                methods = &bmedia_player_es;
            }  else {
                methods = &bmedia_player_generic;
            }
            break;
        case bstream_mpeg_type_pes:
        case bstream_mpeg_type_vob:
            if(stream->stream.noindex.auto_rate) {
                methods = &bmpeg2pes_player_methods;
                break;
            }
        case bstream_mpeg_type_ts:
            if(stream->stream.noindex.auto_rate) {
                methods = &bmpeg2ts_player_methods;
                break;
            }
            /* fell through */
        default:
            methods = &bmedia_player_generic;
            break;
        }
    } else {
        switch(type) {
#if B_HAS_AVI
        case bstream_mpeg_type_avi:
            /* try custom player first */
            methods = config->autoselect_player ? &bavi_player_custom_methods : &bavi_player_methods;
            break;
#endif
#if B_HAS_ASF
        case bstream_mpeg_type_asf:
            methods = &basf_player_methods;
            break;
#endif
#if B_HAS_RMFF
        case bstream_mpeg_type_rmff:
            methods = &brmff_player_methods;
            break;
#endif
        case bstream_mpeg_type_mp4:
            methods = &bmp4_player_methods;
            break;
        case bstream_mpeg_type_mkv:
            methods = &bmkv_player_methods;
            break;
        case bstream_mpeg_type_wav:
            methods = &bwav_player_methods;
            break;
        case bstream_mpeg_type_ape:
            methods = &bape_player_methods;
            break;
        case bstream_mpeg_type_ogg:
            methods = &bogg_player_methods;
            break;
        case bstream_mpeg_type_ts:
#if B_HAS_NAV_PLAYER
            methods = &bmedia_player_nav;
            break;
#endif
        default:
            methods = NULL;
            break;
        }
    }
    if(!methods) {
        BDBG_WRN(("Unknown media type %u", (unsigned) type));
        return NULL;
    }
    player = BKNI_Malloc(sizeof(*player));
    if(!player) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(player, bmedia_player_t);
    player->index_offset = 0;
    player->time_scale = BMEDIA_TIME_SCALE_BASE;
    player->last_pts = 0;
    player->last_position = 0;
    player->last_pts_valid = false;
    player->last_continous = false;
    player->last_mode_valid = false;
    player->stream_format = stream->format;
    player->decoder_features = config->decoder_features;
    player->player = methods->create(fd, config, stream);
#if B_HAS_AVI
    if (!player->player && methods == &bavi_player_custom_methods ) {
        methods = &bavi_player_methods;
        player->player = methods->create(fd, config, stream); /* and if it fails than open with regular player */
    }
#endif
    if (!player->player) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_player;
    }
    player->methods = methods;
    return player;

err_player:
    BKNI_Free(player);
err_alloc:
    return NULL;
}

void
bmedia_player_destroy(bmedia_player_t player)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    player->methods->destroy(player->player);
    BDBG_OBJECT_DESTROY(player, bmedia_player_t);
    BKNI_Free(player);
    return;
}

int
bmedia_player_next(bmedia_player_t player, bmedia_player_entry *entry)
{
    int rc;
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_ASSERT(entry);
    bmedia_player_init_entry(entry);
    rc = player->methods->next(player->player, entry);
    BDBG_MSG_FLOW(("bmedia_player_next:%#lx(%d) start:%lu length:%u type:%u timestamp:%u", (unsigned long)player, rc, (unsigned long)entry->length, (unsigned)entry->length, (unsigned)entry->type, (unsigned)entry->timestamp));

    if(entry->length >= BMEDIA_PLAYER_MAX_BLOCK){
        BDBG_WRN(("bmedia_player_next:%#lx truncated block from %u to %u", (unsigned long)player, (unsigned)entry->length, BMEDIA_PLAYER_MAX_BLOCK));
        entry->length = BMEDIA_PLAYER_MAX_BLOCK;
    }

    return rc;
}

void
bmedia_player_get_status(bmedia_player_t player, bmedia_player_status *status)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    status->index_error_cnt = 0;
    status->data_error_cnt = 0;
    status->position = BMEDIA_PLAYER_INVALID;
    player->methods->get_status(player->player, status);
    return;
}


#if B_HAS_NAV_PLAYER

int
bmedia_player_lookup_pts( bmedia_player_t player, bmedia_player_pos pos, uint32_t *p_pts )
{
    if(player->methods == &bmedia_player_nav) {
        return bmedia_player_nav_lookup_pts(player->player, pos, p_pts);
    }
    else {
        *p_pts = bmedia_time2pts(pos, player->time_scale);
        return 0;
    }
}

void
bmedia_player_tell(bmedia_player_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_MSG_FLOW(("bmedia_player_tell>: %#lx", (unsigned long)player));
    BDBG_ASSERT(pos);
    if(player->methods == &bmedia_player_nav || player->methods == &bmpeg2ts_player_methods || player->methods == &bmpeg2pes_player_methods ||
        (player->methods == &bmedia_player_generic && !(player->stream_format==bstream_mpeg_type_wav || player->stream_format==bstream_mpeg_type_avi || player->stream_format==bstream_mpeg_type_asf))) {
        player->methods->tell(player->player, pos);
    } else {
        if(player->last_pts_valid) {
            *pos = player->index_offset + bmedia_pts2time(player->last_pts, player->time_scale);
            player->last_position = *pos;
            BDBG_MSG_FLOW(("bmedia_player_tell: %#lx offset:%lu time:%ld(pts:%lu,scale:%ld) -> %lu", (unsigned long)player, player->index_offset, bmedia_pts2time(player->last_pts, player->time_scale), player->last_pts, player->time_scale, *pos));
        } else {
            *pos = player->last_position;
        }
    }
    BDBG_MSG_FLOW(("bmedia_player_tell<: %#lx %u", (unsigned long)player, (unsigned)*pos));
    return;
}


#define B_FLAG(p,v) ((p)->v?#v:"")

int
bmedia_player_set_direction(bmedia_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    int rc;
    bmedia_time_scale player_time_scale = time_scale;
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_ASSERT(mode);

    BDBG_MSG(("bmedia_player_set_direction>: %#lx %d %d", (unsigned long)player, (int)direction, (int)time_scale));

    rc = 0;
    BKNI_Memset(mode, 0, sizeof(*mode));
    mode->tsm = true;
    mode->continuous = true;
    mode->time_scale = time_scale;
    mode->display_frames = bmedia_player_decoder_frames_all;
    mode->reordering_mode = bmedia_player_reordering_mode_none;

    if(player->methods == &bmedia_player_nav) {
        rc = player->methods->set_direction(player->player, direction, player_time_scale, mode);
        goto done;
    }
    if(time_scale >= 0 && (time_scale <= BMEDIA_TIME_SCALE_BASE || (player->decoder_features.stc && time_scale <= player->decoder_features.max_decoder_rate))) {
        /* normal decode with slow motion  or fast forward */
        mode->discontinuity = player->time_scale > player->decoder_features.max_decoder_rate || player->time_scale<0;
        player_time_scale = BMEDIA_TIME_SCALE_BASE;
        direction = 0;
    } else {
        if(player->methods == &bmedia_player_generic) {
            BDBG_ERR(("unable to do host trick mode without an index"));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
        mode->continuous = false;
        mode->host_paced = true;
        mode->discontinuity = true;
    }
    if(mode->discontinuity) {
        if(player->time_scale == time_scale) {
            mode->discontinuity = false;
        }
    } else if(player->time_scale != time_scale) {
        mode->discontinuity = !(mode->continuous && player->last_continous);
    }
    if(mode->discontinuity) {
        bmedia_player_pos pos = BMEDIA_PLAYER_INVALID;

        if(player->methods != &bmpeg2ts_player_methods && player->methods != &bmpeg2pes_player_methods) {
            if(time_scale >= 0 && time_scale <= player->decoder_features.max_decoder_rate) {
                mode->host_paced = true;
                direction = 0;
            }
        }

        if(player->last_pts_valid) { /* get current position */
            bmedia_player_tell(player, &pos);
        }
        BDBG_MSG(("bmedia_player_set_direction: %#lx media_player %d %d", (unsigned long)player, (int)direction, (int)player_time_scale));

        mode->time_scale = time_scale;
        rc = player->methods->set_direction(player->player, direction, player_time_scale, mode);
        if(player_time_scale==BMEDIA_TIME_SCALE_BASE) {
            mode->time_scale = time_scale;
        }
        if(rc!=0) {rc=BERR_TRACE(rc);goto error;}
        if(pos != BMEDIA_PLAYER_INVALID) {
            bmedia_player_seek(player, pos); /* reseek player to the current position */
        }
        player->last_mode_valid = true;
        player->last_mode = *mode;
    } else if(player->last_mode_valid) {
        *mode = player->last_mode;
        mode->time_scale = time_scale; /* for STC trick, need to reapply the time_scale */
        mode->discontinuity = false;
    }

done:
    player->last_continous = mode->continuous;
    player->time_scale = player_time_scale;
error:
    BDBG_MSG(("bmedia_player_set_direction<: %p %s %d %d -> %d %s %s %s %s %s", (void *)player, rc!=0?"ERROR":"", (int)direction, (int)time_scale, (int)mode->time_scale, B_FLAG(mode,discontinuity), B_FLAG(mode, brcm), B_FLAG(mode, dqt), B_FLAG(mode, tsm), B_FLAG(mode, continuous)));
    return rc;
}

#define b_iabs(x) ((x)<0? -(x):(x))

void
bmedia_player_update_position(bmedia_player_t player, uint32_t pts)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);

    BDBG_MSG_FLOW(("bmedia_player_update_position>: %#lx PTS:%u", (unsigned long)player, (unsigned)pts));
    if (player->methods == &bmedia_player_generic) {
        /* if there's no index, don't update the index offset */
        player->last_pts = pts;
        player->last_pts_valid = true;
        return;
    } else if(player->methods == &bmedia_player_nav) {
        bmedia_player_nav_update_position(player->player, pts);
        return;
    } else if(player->methods == &bmpeg2ts_player_methods) {
        player->last_pts_valid = true;
        bmpeg2ts_player_update_position(player->player, pts);
        return;
    } else if(player->methods == &bmpeg2pes_player_methods) {
        player->last_pts_valid = true;
        bmpeg2pes_player_update_position(player->player, pts);
        return;
    }

    BDBG_MSG_FLOW(("bmedia_player_update_position>: %#lx PTS:%u %s%u %u POS:%u", (unsigned long)player, pts, player->last_pts_valid?"":"INVALID:", player->last_pts, pts - player->last_pts, player->index_offset + bmedia_pts2time(pts, player->time_scale)));
    if (!player->last_pts_valid || pts >= player->last_pts) { /* there was no wrap in the pts values */
        goto done;
    }

    BDBG_MSG(("bmedia_player_update_position: %#lx pts_wrap %u %u %u %u", (unsigned long)player, pts, player->last_pts, pts - player->last_pts, (uint32_t)(BMEDIA_PTS_MODULO-1)-player->last_pts));
    if (
        ((uint32_t)(BMEDIA_PTS_MODULO-1) -  player->last_pts) <  /* distance between last PTS and wrap pointer */
        (3*(BMEDIA_UPDATE_POSITION_INTERVAL*45)) /* 3 times of polling timer */
       ) { /* real wrap */
        uint32_t delta;

        delta = bmedia_pts2time((uint32_t)(BMEDIA_PTS_MODULO-1), b_iabs(player->time_scale));
        BDBG_MSG_FLOW(("bmedia_player_update_position: %#lx media index_offset %lu %s%lu", (unsigned long)player, (unsigned long)player->index_offset, player->time_scale<0?"-":"", (unsigned long)delta));
        /* there was a wrap of pts */
        if (player->time_scale >= 0) {
            player->index_offset += delta;
        } else {
            player->index_offset -= delta;
        }
    } else {
        BDBG_WRN(("bmedia_player_update_position: %#lx out of order PTS: %u %u %u %u", (unsigned long)player, pts, player->last_pts, pts - player->last_pts, (uint32_t)(BMEDIA_PTS_MODULO-1)-player->last_pts));
    }

done:
    BDBG_MSG_FLOW(("bmedia_player_update_position<: %#lx POS:%u", (unsigned long)player, player->index_offset + bmedia_pts2time(pts, player->time_scale)));
    player->last_pts = pts;
    player->last_pts_valid = true;
    return;
}

int
bmedia_player_seek(bmedia_player_t player, bmedia_player_pos pos)
{
    int rc;
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_MSG_FLOW(("bmedia_player_seek>: %#lx %lu",  (unsigned long)player, pos));

    switch (player->stream_format) {
    case bstream_mpeg_type_avi:
	case bstream_mpeg_type_asf:
        if ((player->methods == &bmedia_player_generic) && (pos != 0)){
			BDBG_ERR(("seeking in stream format %d without index is not supported", player->stream_format));
			return -1;
        }
        break;
    default:
        break;
    }

    rc = player->methods->seek(player->player, pos);
    player->last_pts_valid = false;
    if(player->methods == &bmedia_player_nav) {
        /* no-op */
    } else if(player->methods == &bmpeg2ts_player_methods || player->methods == &bmpeg2pes_player_methods) {
        /* no op */
    } else {
        /* set stream position for PTS 0 */
        uint32_t delta;

        delta = bmedia_pts2time((uint32_t)(BMEDIA_PTS_MODULO-1), b_iabs(player->time_scale));
        if(delta==0)  { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto done; }
        BDBG_MSG_FLOW(("bmedia_player_seek: %#lx %ld %lu %lu (%lu->%lu)",  (unsigned long)player, player->time_scale, pos, delta, player->index_offset, (pos/delta)*delta));
        player->index_offset = (pos/delta)*delta;
        player->last_pts = bmedia_time2pts(pos, player->time_scale); /* save the expected PTS value, it's latter used in bmedia_player_update_position, to detect wrap in PTS values */
        if(rc==0) {
            player->last_position = pos;
        }
    }
done:
    BDBG_MSG_FLOW(("bmedia_player_seek<: %#lx %lu %d",  (unsigned long)player, pos, rc));
    return rc;
}

void
bmedia_player_get_decoder_config (bmedia_player_t player, bmedia_player_decoder_config *config)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_ASSERT(config);
    *config  = player->decoder_features;
    if(player->methods == &bmedia_player_nav) {
        bmedia_player_nav_get_decoder_config(player->player, config);
    }
    return;
}


int
bmedia_player_set_decoder_config (bmedia_player_t player, const bmedia_player_decoder_config *config)
{
    int rc=0;

    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BDBG_ASSERT(config);

    if(player->methods == &bmedia_player_nav) {
        rc = bmedia_player_nav_set_decoder_config(player->player, config);
    } else{
        if(config->host_mode!=bmedia_player_host_trick_mode_auto) {
            BDBG_ERR(("bmedia_player_set_decoder_config:%#lx mode %u not supported", (unsigned long)player, (unsigned)config->host_mode));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        player->decoder_features = *config;
    }

    return rc;
}

#else /* B_HAS_NAV_PLAYER */

int
bmedia_player_lookup_pts( bmedia_player_t player, bmedia_player_pos pos, uint32_t *p_pts )
{
    *p_pts = bmedia_time2pts(pos, player->time_scale);
    return 0;
}

int
bmedia_player_set_direction(bmedia_player_t player, bmedia_player_step direction, bmedia_time_scale time_scale, bmedia_player_decoder_mode *mode)
{
    int rc;
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    BSTD_UNUSED(mode);
    rc = player->methods->set_direction(player->player, direction, time_scale, mode);
    if(rc!=0) {rc=BERR_TRACE(rc);}
    return rc;
}

int
bmedia_player_seek(bmedia_player_t player, bmedia_player_pos pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    return player->methods->seek(player->player, pos);
}

void
bmedia_player_tell(bmedia_player_t player, bmedia_player_pos *pos)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);
    player->methods->tell(player->player, pos);
    return;
}

#define b_iabs(x) ((x)<0? -(x):(x))

void
bmedia_player_update_position(bmedia_player_t player, uint32_t pts)
{
    BDBG_OBJECT_ASSERT(player, bmedia_player_t);


    
    BDBG_MSG_FLOW(("bmedia_player_update_position>: %#lx PTS:%u", (unsigned long)player, (unsigned)pts));
    if (player->methods == &bmedia_player_generic) {
        
    
        /* if there's no index, don't update the index offset */
        player->last_pts = pts;
        player->last_pts_valid = true;
        return;
    } else if(player->methods == &bmpeg2ts_player_methods) {
    
        player->last_pts_valid = true;
        bmpeg2ts_player_update_position(player->player, pts);
    
        return;
    } else if(player->methods == &bmpeg2pes_player_methods) {
        

        player->last_pts_valid = true;
        bmpeg2pes_player_update_position(player->player, pts);
        return;
    }

    BDBG_MSG_FLOW(("bmedia_player_update_position>: %#lx PTS:%u %s%u %u POS:%u", (unsigned long)player, pts, player->last_pts_valid?"":"INVALID:", player->last_pts, pts - player->last_pts, player->index_offset + bmedia_pts2time(pts, player->time_scale)));
    if (!player->last_pts_valid || pts >= player->last_pts) { /* there was no wrap in the pts values */
        goto done;
    }

    BDBG_MSG(("bmedia_player_update_position: %#lx pts_wrap %u %u %u %u", (unsigned long)player, pts, player->last_pts, pts - player->last_pts, (uint32_t)(BMEDIA_PTS_MODULO-1)-player->last_pts));
    if (
        ((uint32_t)(BMEDIA_PTS_MODULO-1) -  player->last_pts) <  /* distance between last PTS and wrap pointer */
        (3*(BMEDIA_UPDATE_POSITION_INTERVAL*45)) /* 3 times of polling timer */
       ) { /* real wrap */
        uint32_t delta;


        delta = bmedia_pts2time((uint32_t)(BMEDIA_PTS_MODULO-1), b_iabs(player->time_scale));
        BDBG_MSG_FLOW(("bmedia_player_update_position: %#lx media index_offset %lu %s%lu", (unsigned long)player, (unsigned long)player->index_offset, player->time_scale<0?"-":"", (unsigned long)delta));
        /* there was a wrap of pts */
        if (player->time_scale >= 0) {
            player->index_offset += delta;
        } else {
            player->index_offset -= delta;
        }
    } else {
        BDBG_WRN(("bmedia_player_update_position: %#lx out of order PTS: %u %u %u %u", (unsigned long)player, pts, player->last_pts, pts - player->last_pts, (uint32_t)(BMEDIA_PTS_MODULO-1)-player->last_pts));
    }

done:
    BDBG_MSG_FLOW(("bmedia_player_update_position<: %#lx POS:%u", (unsigned long)player, player->index_offset + bmedia_pts2time(pts, player->time_scale)));
    player->last_pts = pts;
    player->last_pts_valid = true;
    return;
}


void
bmedia_player_get_decoder_config (bmedia_player_t player, bmedia_player_decoder_config *config)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(config);
    return;
}


int
bmedia_player_set_decoder_config (bmedia_player_t player, const bmedia_player_decoder_config *config)
{
    BSTD_UNUSED(player);
    BSTD_UNUSED(config);
    return 0;
}
#endif /* B_HAS_NAV_PLAYER */


bool
bmedia_player_stream_test(const bmedia_player_stream *stream, unsigned sub_stream)
{
    unsigned i;
    BDBG_ASSERT(stream);
    if (sub_stream == stream->master) {
        return true;
    }
    for(i=0;i<BMEDIA_PLAYER_MAX_TRACKS;i++) {
        if(sub_stream == stream->other[i]) {
            return true;
        }
    }
    return false;
}

uint8_t
bmedia_player_stream_get_id(const bmedia_player_stream *stream, unsigned sub_stream)
{
    unsigned i;
    BDBG_ASSERT(stream);
    if (sub_stream == stream->master) {
        return stream->stream.id.master;
    }
    for(i=0;i<BMEDIA_PLAYER_MAX_TRACKS;i++) {
        if(sub_stream == stream->other[i]) {
            return stream->stream.id.other[i];
        }
    }
    return 0;
}

int
bmedia_player_pos_delta(bmedia_player_pos future, bmedia_player_pos past)
{
    int delta = (int)future-(int)past;
    return delta;
}

bool
bmedia_player_pos_in_range(bmedia_player_pos future, bmedia_player_pos past, bmedia_player_pos range)
{
    int delta = (int)future-(int)past;
    if( (delta>=0 && (bmedia_player_pos)delta<=range) || (delta<0 && (bmedia_player_pos)(-delta)<=range)) {
        return true;
    }
    return false;
}

void
bmedia_player_init_status(bmedia_player_status *status)
{
    status->position = 0;
    status->bounds.first = 0;
    status->bounds.last = 0;
    status->direction = 0;
    status->index_error_cnt = 0;
    status->data_error_cnt = 0;
    status->format = bstream_mpeg_type_unknown;
    return;
}

void
bmedia_player_init_entry(bmedia_player_entry *entry)
{
    BKNI_Memset(entry, 0, sizeof(*entry));
    entry->type = bmedia_player_entry_type_no_data;
    entry->content = bmedia_player_entry_content_unknown;
    return;
}

void bmedia_player_init_sync_entry(bmedia_player_sync_entry *entry)
{
    BKNI_Memset(entry, 0, sizeof(*entry));
    return;
}

