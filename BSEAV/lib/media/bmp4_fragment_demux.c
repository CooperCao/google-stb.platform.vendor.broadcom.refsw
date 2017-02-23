/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * MP4 container parser library
 *
 *******************************************************************************/
#include "bstd.h"
#include "blst_slist.h"
#include "biobits.h"
#include "bkni.h"
#include "bmp4_util.h"
#include "bmp4_fragment_demux.h"

BDBG_MODULE(bmp4_fragment_demux);

BDBG_OBJECT_ID(bmp4_fragment_demux);
BDBG_OBJECT_ID(bmp4_fragment_stream);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

#define B_MP4_STREAM_VEC_FRAME	0
#define B_MP4_STREAM_VEC_RAI    1
#define B_MP4_STREAM_VEC_EOS	2

typedef enum b_mp4_fragment_demux_state {
    b_mp4_fragment_demux_state_box, /* looking for the box */
    b_mp4_fragment_demux_state_capture, /* capturing box */
    b_mp4_fragment_demux_state_remux /* parse captured box */
} b_mp4_fragment_demux_state;

typedef struct b_mp4_fragment_box {
    uint32_t type;
    int start;
    size_t len;
    size_t header_len;
    size_t next; /* next reading point (inside the segment) */
} b_mp4_fragment_box;

typedef struct b_mp4_fragment_track_state {
    struct {
        uint64_t run_offset;
        uint64_t run_start_time;
    } next, current;
} b_mp4_fragment_track_state;

typedef struct b_mp4_fragment_header {
    uint64_t timescale;
    uint64_t starttime;
    uint32_t fourcc;
} b_mp4_fragment_header;

typedef struct b_mp4_fragment_codec_private {
    batom_t data;
} b_mp4_fragment_codec_private;

typedef struct b_mp4_fragment_track {
    bool next_sample_valid;
    bool fragment_run_valid;
    bool fragment_header_valid;
    bool movie_fragment_valid;
    bool movie_fragment_data_valid;
    bool track_eos;

    unsigned next_sample_no;
    bmp4_track_fragment_header track_fragment_header;
    bmp4_track_fragment_run_header run_header;
    bmp4_track_fragment_run_state run_state;
    bmp4_track_fragment_run_sample next_sample;

    b_mp4_fragment_box root;
    b_mp4_fragment_box movie_fragment;
    b_mp4_fragment_box movie_fragment_data;
    b_mp4_fragment_box track_fragment;
    b_mp4_fragment_box track_fragment_run;
    b_mp4_fragment_box track_fragment_header_box;
    b_mp4_fragment_header fragment_header;
    b_mp4_fragment_codec_private codec_private;
    bmp4_trackextendsbox track_extends;
    batom_cursor track_cursor;
    b_mp4_fragment_track_state state;
    int (*payload_handler)(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len);
} b_mp4_fragment_track;

struct bmp4_fragment_stream {
    BDBG_OBJECT(bmp4_fragment_stream)
    BLST_S_ENTRY(bmp4_fragment_stream) link;
    unsigned stream_id;
    bool busy; /* set to true if currently for this track there is a process of converting fragment to individual frames */
    batom_accum_t data;
    batom_accum_t frame_accum; 
    batom_pipe_t pipe_out;
    b_mp4_fragment_track track;
    bmp4_fragment_stream_cfg cfg;
    const batom_vec *vecs[3];
    struct {
        bmedia_adts_header adts_header;
    } aac;
};

struct bmp4_fragment_demux {
    BDBG_OBJECT(bmp4_fragment_demux)
    batom_accum_t accum;
    bmp4_fragment_demux_cfg config;
    size_t box_size;
    size_t box_header_size;
    uint64_t offset;
    BLST_S_HEAD(b_mp4_fragment_stream, bmp4_fragment_stream) streams;
    b_mp4_fragment_demux_state demux_state;
    unsigned next_fragment_track_ID; /* track ID of the next fragment to parse, it's set to not 0, if frame with the same ID is already parsed  */
    struct {
        void *nal_prefix;
        batom_vec nal_vec;
    } h264;
};

static void 
b_mp4_fragment_box_init(b_mp4_fragment_box *box, uint32_t type) 
{
    box->next = 0;
    box->type = type;
    box->start = -1;
    box->len = 0;
    return;
} 

static void
b_mp4_fragment_box_set(b_mp4_fragment_box *box, unsigned start, size_t len, size_t header_len)
{
    BDBG_MSG_TRACE(("b_mp4_fragment_box_set:%#lx " B_MP4_TYPE_FORMAT " %lu:%u", (unsigned long)box, B_MP4_TYPE_ARG(box->type), (unsigned long)start, (unsigned)len));
    box->header_len = header_len;
    box->start = start;
    box->len = len;
    box->next = 0;
    return;
}

static int
b_mp4_fragment_box_peek(b_mp4_fragment_box *box, const batom_cursor *source, b_mp4_fragment_box *child, bool *end_of_data, bmp4_box *mp4_box, batom_cursor *destination)
{
    size_t box_size;
    size_t position;

    BDBG_ASSERT(box->start>=0);
    BATOM_CLONE(destination, source);
    position = box->start + box->next;
    if(position != batom_cursor_skip(destination, position)) {
        *end_of_data = true;
        return -1;
    }
    *end_of_data = false;
    box_size = bmp4_parse_box(destination, mp4_box);
    position = batom_cursor_pos(destination);
    if(box_size==0 || box_size>mp4_box->size) {
        if(position + BMP4_BOX_MAX_SIZE >= box->len) {
            *end_of_data = true;
        }
        return -1;
    }
    BDBG_MSG(("b_mp4_fragment_box_peek: %#lx:" B_MP4_TYPE_FORMAT " box: " B_MP4_TYPE_FORMAT ":%u (" B_MP4_TYPE_FORMAT ")", (unsigned long)box, B_MP4_TYPE_ARG(box->type), B_MP4_TYPE_ARG(mp4_box->type), (unsigned)mp4_box->size, B_MP4_TYPE_ARG(child->type)));
    if(mp4_box->type==child->type) {
        b_mp4_fragment_box_set(child, position, mp4_box->size-box_size, box_size);
    }
    return 0;
}

static int
b_mp4_fragment_box_next(b_mp4_fragment_box *box, const batom_cursor *source, b_mp4_fragment_box *child, bool *end_of_data, batom_cursor *destination)
{
    for(;;) {
        int rc;
        bmp4_box mp4_box;

        rc = b_mp4_fragment_box_peek(box, source, child, end_of_data, &mp4_box, destination);
        if(rc<0) {
            return rc;
        }
        box->next += mp4_box.size;
        if(mp4_box.type==child->type) {
            break;
        }
    }
    return 0;
}

typedef struct b_mp4_meta_atom_info {
    void *buffer;
    bmp4_fragment_demux_t demux;
} b_mp4_meta_atom_info;

static void 
b_mp4_meta_atom_free(batom_t atom, void *user)
{
    const b_mp4_meta_atom_info *info= (b_mp4_meta_atom_info *)user;

    BSTD_UNUSED(atom);
    BDBG_MSG_TRACE(("b_mp4_meta_atom_free:%#lx %#lx", (unsigned long)atom, (unsigned long)user));
    BDBG_OBJECT_ASSERT(info->demux, bmp4_fragment_demux);
    BDBG_MSG(("b_mp4_meta_atom_free: demux:%#lx block:%#lx", (unsigned long)info->demux, (unsigned long)info->buffer));
    if(info->buffer) {
        info->demux->config.alloc->bmem_free(info->demux->config.alloc, info->buffer);
    }
    return;
}

static const batom_user b_mp4_meta_atom = {
	b_mp4_meta_atom_free,
	sizeof(b_mp4_meta_atom_info)
};



void 
bmp4_fragment_demux_default_cfg(bmp4_fragment_demux_cfg *cfg)
{
    BKNI_Memset(cfg, 0, sizeof(*cfg));
    cfg->burst_frames = 4;
    return;
}

static void
b_mp4_fragment_track_init(b_mp4_fragment_track *track)
{
    BKNI_Memset(track, 0, sizeof(*track));
    b_mp4_fragment_box_init(&track->movie_fragment, BMP4_MOVIE_FRAGMENT);
    b_mp4_fragment_box_init(&track->movie_fragment_data, BMP4_MOVIE_DATA);
    b_mp4_fragment_box_init(&track->track_fragment, BMP4_TRACK_FRAGMENT);
    b_mp4_fragment_box_init(&track->track_fragment_header_box, BMP4_TRACK_FRAGMENT_HEADER);
    b_mp4_fragment_box_init(&track->track_fragment_run, BMP4_TRACK_FRAGMENT_RUN);
    b_mp4_fragment_box_init(&track->root, B_MP4_FRAGMENT_WRAPPER);
    return;
}

static int
b_mp4_fragment_demux_allocate_h264(bmp4_fragment_demux_t demux)
{
    demux->h264.nal_prefix = demux->config.alloc->bmem_alloc(demux->config.alloc, bmedia_nal_vec.len);
    if(!demux->h264.nal_prefix) { return -1; }
    BKNI_Memcpy(demux->h264.nal_prefix, bmedia_nal_vec.base, bmedia_nal_vec.len);
    BATOM_VEC_INIT(&demux->h264.nal_vec, demux->h264.nal_prefix, bmedia_nal_vec.len);
    return 0;
}

static void
b_mp4_fragment_demux_free_h264(bmp4_fragment_demux_t demux)
{
    demux->config.alloc->bmem_free(demux->config.alloc, demux->h264.nal_prefix);
    return;
}


bmp4_fragment_demux_t 
bmp4_fragment_demux_create(const bmp4_fragment_demux_cfg *config)
{
    bmp4_fragment_demux_t demux;
    int rc;

    BDBG_ASSERT(config);
    if(config->factory==NULL || config->alloc==NULL) { (void)BERR_TRACE(BERR_NOT_SUPPORTED); goto err_config; }
    demux = BKNI_Malloc(sizeof(*demux));
    if(demux==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(demux, bmp4_fragment_demux);
    demux->config = *config;
    demux->offset = 0;
    demux->demux_state = b_mp4_fragment_demux_state_box;
    demux->next_fragment_track_ID = 0;
    BLST_S_INIT(&demux->streams);

    demux->accum = batom_accum_create(config->factory);
    if(!demux->accum) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_create_accum;}

    rc = b_mp4_fragment_demux_allocate_h264(demux);
    if(rc!=0) {(void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto err_h264;}


    return demux;
err_h264:
    batom_accum_destroy(demux->accum);
err_create_accum:
    BKNI_Free(demux);
err_alloc:
err_config:
    return NULL;
}

void 
bmp4_fragment_demux_destroy(bmp4_fragment_demux_t demux)
{
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    b_mp4_fragment_demux_free_h264(demux);
    batom_accum_destroy(demux->accum);
    BDBG_OBJECT_DESTROY(demux, bmp4_fragment_demux);
    BKNI_Free(demux);
    return;
}

void 
bmp4_fragment_demux_reset(bmp4_fragment_demux_t demux)
{
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    bmp4_fragment_demux_flush(demux);
    demux->demux_state = b_mp4_fragment_demux_state_box;
    demux->offset = 0;
    return;
}

static int
b_mp4_fragment_track_next(bmp4_fragment_demux_t demux, b_mp4_fragment_track *track, const batom_cursor *source, bool *end_of_fragment)
{
    int rc=0;
    bool end_of_data;

    for(;;) {
        if(track->fragment_header.starttime == (uint64_t)((int64_t)(-1))) {
            if(track->track_eos) {
                track->track_eos = false;
                *end_of_fragment = true;
                rc = -1;
            } else {
                track->track_eos = true;
            }
            break;
            /* end of stream */
        } else if(!track->movie_fragment_valid) {
            bmp4_movie_fragment_header movie_fragment_header;
            b_mp4_fragment_box movie_fragment_header_box;
            bmp4_box mdat;

            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx reading movie_fragment", (unsigned long)track));
            rc = b_mp4_fragment_box_next(&track->root, source, &track->movie_fragment, &end_of_data, &track->track_cursor);
            if(rc<0) {
                if(end_of_data) {
                    *end_of_fragment = true;
                    break;
                } else {
                    break;
                }
            }

            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx reading movie_fragment_header", (unsigned long)track));
            b_mp4_fragment_box_init(&movie_fragment_header_box, BMP4_MOVIE_FRAGMENT_HEADER);
            rc = b_mp4_fragment_box_next(&track->movie_fragment, source, &movie_fragment_header_box, &end_of_data, &track->track_cursor);
            if(rc<0) {
                break;
            }
            if(!bmp4_parse_movie_fragment_header(&track->track_cursor, &movie_fragment_header)) {
                return -1;
            }
            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx reading movie_fragment_data", (unsigned long)track));
            rc = b_mp4_fragment_box_peek(&track->root, source, &track->movie_fragment_data, &end_of_data, &mdat, &track->track_cursor);
            if(rc==0 && mdat.type == track->movie_fragment_data.type) {
                BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx movie_fragment_data at:%u", (unsigned long)track, (unsigned)track->movie_fragment_data.start));
                track->movie_fragment_data_valid = true;
            } else {
                track->movie_fragment_data_valid = false;
            }
            track->state.next.run_offset = track->state.current.run_offset = 0;
            track->movie_fragment_valid = true;
        }
        if(!track->fragment_header_valid) {

            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx reading track_fragment", (unsigned long)track));
            rc = b_mp4_fragment_box_next(&track->movie_fragment, source, &track->track_fragment, &end_of_data, &track->track_cursor);
            if(rc<0) {
                if(end_of_data) {
                    track->movie_fragment_valid = false; /* look for the next fragment */
                    continue;
                } else {
                    break;
                }
            }
            if(demux->config.traf_box) {
                demux->config.traf_box(demux->config.user_context, source, track->track_fragment.len);
            }
            rc = b_mp4_fragment_box_next(&track->track_fragment, source, &track->track_fragment_header_box, &end_of_data, &track->track_cursor);
            if(rc!=0) {
                return -1;
            }
            if(!bmp4_parse_track_fragment_header(&track->track_cursor, &track->track_fragment_header)) {
                return -1;
            }
#if 0
            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx found track %u:%u", (unsigned long)track, track->track_fragment_header.track_ID, track->track_extends.track_ID));
            if(track->track_fragment_header.track_ID != track->track_extends.track_ID) {
                continue; /* advance to the next fragment */
            }
#endif
            track->fragment_header_valid = true;
        }
        if(!track->fragment_run_valid) {
            BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx reading track_fragment_run", (unsigned long)track));
            rc = b_mp4_fragment_box_next(&track->track_fragment, source, &track->track_fragment_run, &end_of_data, &track->track_cursor);
            if(rc<0) {
                if(end_of_data) {
                    track->fragment_header_valid = false; 
                    continue;
                } else {
                    break;
                }
            }
            if(!bmp4_parse_track_fragment_run_header(&track->track_cursor, &track->run_header)) {
                return -1;
            }
            bmp4_init_track_fragment_run_state(&track->run_state);
            track->fragment_run_valid = true;
        }
        if(track->run_state.sample_no >= track->run_header.sample_count) {
            track->state.current = track->state.next;
            track->fragment_run_valid = false;
            continue;
        }

        track->next_sample_no = track->run_state.sample_no;
        if(!bmp4_parse_track_fragment_run_sample(&track->track_cursor, &track->track_fragment_header, &track->run_header, &track->track_extends, &track->run_state, &track->next_sample)) {
            track->fragment_run_valid = false;
            continue;
        }
        BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx track_ID:%u sample.offset:%u %s", (unsigned long)track, track->track_extends.track_ID, (unsigned)track->next_sample.offset, track->run_header.validate.data_offset?"run_header.data_offset":""));
        if(track->run_header.validate.data_offset) {
            track->next_sample.offset += track->run_header.data_offset;
        } else {
            track->next_sample.offset += track->state.current.run_offset;
        }
        track->state.next.run_offset = track->next_sample.offset + track->next_sample.size;
        BDBG_MSG_TRACE(("b_mp4_fragment_track_next:%#lx track_ID:%u run.offset:%u %s %s %u %u", (unsigned long)track, track->track_extends.track_ID, (unsigned)track->next_sample.offset, track->track_fragment_header.validate.base_data_offset?"track_fragment_header.base_data_offset":"", track->movie_fragment_data_valid?"MDAT.offset":"", (unsigned)track->movie_fragment_data.start, (unsigned)(track->movie_fragment.start-track->movie_fragment.header_len)));
        if(track->track_fragment_header.validate.base_data_offset) {
            /* it appears that in the case of individual fragment offset is relative to  location of the base_data_offset field inside the track fragment header */
            track->next_sample.offset += track->track_fragment_header.base_data_offset + track->track_fragment_header_box.start + 4 /* flags */ + 4 /* track_ID */; 
        } else {
            if(track->movie_fragment_data_valid && !track->run_header.validate.data_offset)  {
                track->next_sample.offset += track->movie_fragment_data.start;
            } else {
                track->next_sample.offset += track->movie_fragment.start - track->movie_fragment.header_len;
            }
        }
        track->next_sample.time += track->state.current.run_start_time;
        track->state.next.run_start_time = track->next_sample.time + track->next_sample.duration;
        break;
    }

    BDBG_MSG(("b_mp4_fragment_track_next:%#lx track_ID:%u sample offset:%u time:%u duration:%u size:%u flags:%#x composition_time_offset:%d %s", (unsigned long)track, (unsigned)track->track_extends.track_ID, (unsigned)track->next_sample.offset, (unsigned)track->next_sample.time, track->next_sample.duration, track->next_sample.size, track->next_sample.flags, (int)track->next_sample.composition_time_offset, *end_of_fragment?"EOF":""));

    return rc;
}

static bool
b_mp4_parse_fragment_header(batom_cursor *cursor, b_mp4_fragment_header *header)
{
    header->timescale = batom_cursor_uint64_be(cursor);
    header->starttime = batom_cursor_uint64_be(cursor);
    header->fourcc = batom_cursor_uint32_le(cursor);
    return !BATOM_IS_EOF(cursor);
}

static int
b_mp4_fragment_track_start(b_mp4_fragment_track *track, batom_accum_t accum, size_t header_len, size_t box_len, const batom_cursor *source)
{
    b_mp4_fragment_box box;
    batom_cursor cursor;
    int rc;
    bool end_of_data;
    batom_t track_extends;
    batom_cursor end_of_box;

    BDBG_ASSERT(header_len<box_len);
    b_mp4_fragment_box_set(&track->root, header_len, box_len-header_len, header_len);
    b_mp4_fragment_box_init(&box, B_MP4_FRAGMENT_HEADER);
    rc = b_mp4_fragment_box_next(&track->root, source, &box, &end_of_data, &cursor);
    if(rc<0) { return rc; }
    if(!b_mp4_parse_fragment_header(&cursor, &track->fragment_header)) { return -1; }
    b_mp4_fragment_box_init(&box, B_MP4_FRAGMENT_DATA);
    rc = b_mp4_fragment_box_next(&track->root, source, &box, &end_of_data, &cursor);
    if(rc<0) { return rc; }
    BATOM_CLONE(&end_of_box, &cursor);
    batom_cursor_skip(&end_of_box, box.len);
    track->codec_private.data = batom_accum_extract(accum, &cursor, &end_of_box, NULL, NULL);
    if(!track->codec_private.data) {return -1; }
    b_mp4_fragment_box_init(&box, BMP4_TRACK_EXTENDS);
    rc = b_mp4_fragment_box_next(&track->root, source, &box, &end_of_data, &cursor);
    if(rc<0) { return rc; }
    BATOM_CLONE(&end_of_box, &cursor);
    batom_cursor_skip(&end_of_box, box.len);
    track_extends = batom_accum_extract(accum, &cursor, &end_of_box, NULL, NULL);
    if(!track_extends) {return -1;}
    end_of_data = !bmp4_parse_trackextends(track_extends, &track->track_extends);
    batom_release(track_extends);
    if(end_of_data) { return -1;}

    track->next_sample_valid = false;
    track->fragment_run_valid = false;
    track->fragment_header_valid = false;
    track->movie_fragment_valid = false;
    track->track_eos = false;
    track->movie_fragment_data_valid = false;
    track->state.current.run_offset = 0;
    track->state.current.run_start_time = track->fragment_header.starttime;
    track->state.next.run_offset = 0;
    track->state.next.run_start_time = track->fragment_header.starttime;

    return 0;
}

static void 
b_mp4_fragment_track_clear(b_mp4_fragment_track *track)
{
    if(track->codec_private.data) {
        batom_release(track->codec_private.data);
        track->codec_private.data=NULL;
    }
    return;
}

static void 
b_mp4_fragment_demux_report_error(bmp4_fragment_demux_t demux)
{
    if(demux->config.error) {
        demux->config.error(demux->config.user_context);
    }
    return;
}

static int
b_mp4_fragment_stream_start(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, const batom_cursor *cursor, batom_accum_t accum, size_t box_len)
{
    batom_cursor end_of_box;
    batom_t data;

    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    BDBG_OBJECT_ASSERT(stream, bmp4_fragment_stream);

    BATOM_CLONE(&end_of_box, cursor);
    batom_cursor_skip(&end_of_box, box_len);
    data = batom_accum_extract(accum, cursor, &end_of_box, NULL, NULL);
    if(!data) { return -1; }
    batom_accum_clear(stream->data);
    batom_accum_add_atom(stream->data, data);
    batom_release(data);
    stream->busy = true;
    return 0;
}


static int
b_mp4_payload_handler_h264(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len)
{
    int rc=0;
    size_t left;
    unsigned count;
	bmedia_packet_header hdr;
    batom_t pes_atom;

    BDBG_MSG_TRACE(("b_mp4_payload_handler_h264:%#lx:%#lx:%u payload %u", (unsigned long)demux, (unsigned long)stream, frame_no, (unsigned)payload_len));

    batom_accum_clear(stream->frame_accum);

	BMEDIA_PACKET_HEADER_INIT(&hdr);
    BMEDIA_PES_SET_PTS(&hdr,bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));

    if(frame_no==0) { /* need to allocate data rai and append for pps,sps and  rai */
        stream->vecs[B_MP4_STREAM_VEC_FRAME] = &bmedia_nal_vec;
        stream->vecs[B_MP4_STREAM_VEC_RAI] = &bmedia_rai_h264;
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_eos_h264;

        hdr.header_type = B_MP4_STREAM_VEC_RAI;
        if(stream->track.codec_private.data) {
            if(stream->track.fragment_header.fourcc == BMEDIA_FOURCC('a','v','c','1')) {
                void *h264_meta;
                size_t meta_length = batom_len(stream->track.codec_private.data);
                batom_cursor cursor;
                b_mp4_meta_atom_info meta_info;
                bmedia_h264_meta meta;
                batom_t meta_marker;

                h264_meta = demux->config.alloc->bmem_alloc(demux->config.alloc, meta_length);
                if(h264_meta==NULL) { (void)BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto error; }
                BDBG_MSG(("b_mp4_payload_handler_h264:%#lx:%#lx allocated h264_meta:%#lx", (unsigned long)demux, (unsigned long)stream, (unsigned long)h264_meta));
                batom_cursor_from_atom(&cursor, stream->track.codec_private.data);
                batom_cursor_copy(&cursor, h264_meta, meta_length);
                if(!bmedia_read_h264_meta(&meta, h264_meta, meta_length)) { BDBG_ERR(("b_mp4_payload_handler_h264:%#lx:%#lx invalid avc1 meta data", (unsigned long)demux, (unsigned long)stream));}
                bmedia_copy_h264_meta_with_nal_vec(stream->frame_accum, &meta, &demux->h264.nal_vec);
                meta_info.buffer = h264_meta;
                meta_info.demux = demux;
                meta_marker = batom_from_range(demux->config.factory,demux->h264.nal_prefix, 1, &b_mp4_meta_atom, &meta_info); /* construct a vector that just carries 0 and marker so memory would get freed unsed metadata delivered */
                if(meta_marker==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
                batom_accum_add_atom(stream->frame_accum, meta_marker);
                batom_release(meta_marker);
            } else /* fourcc==BMEDIA_FOURCC('H','2','6','4')  */ {
                batom_accum_add_atom(stream->frame_accum, stream->track.codec_private.data);
            }
        }
        hdr.key_frame = true;
    }

    for(count=0,left=payload_len;left>0;count++) {
        batom_cursor nal_start;
        const size_t lengthSize = 4;
        uint32_t nal_len;
        if(left<=lengthSize) {
            BDBG_MSG_TRACE(("b_mp4_payload_handler_h264:%#lx:%#lx invalid frame, left %u", (unsigned long)demux, (unsigned long)stream, (unsigned)left));
            rc = -1;
            goto error;
        }
        left -= lengthSize;
        nal_len = batom_cursor_vword_be(payload, lengthSize);
        /*  BDBG_MSG(("nal_len:%u", nal_len)); */
        BATOM_CLONE(&nal_start, payload);
        if(nal_len==0 || nal_len > left || nal_len!=batom_cursor_skip(payload, nal_len)) {
            BDBG_MSG_TRACE(("b_mp4_payload_handler_h264:%#lx:%#lx not enough frame data %u:%u", (unsigned long)demux, (unsigned long)stream, (unsigned)nal_len, (unsigned)left));
            rc = -1;
            goto error;
        }
        if(count==0 && frame_no!=0) {
            hdr.header_type = B_MP4_STREAM_VEC_FRAME;
        } else {
            batom_accum_add_range(stream->frame_accum, demux->h264.nal_prefix, bmedia_nal_vec.len);
        }
        batom_accum_append(stream->frame_accum, stream->data, &nal_start, payload);
        left -= nal_len;
    }
    if(frame_no==0) { /* could free codec_private.data */
        if(stream->track.codec_private.data) {
            batom_release(stream->track.codec_private.data);
            stream->track.codec_private.data=NULL;
        }
    }
	pes_atom = batom_from_accum(stream->frame_accum, bmedia_atom, &hdr);	
    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;

error:
    batom_accum_clear(stream->frame_accum);
    return -1;
}

static int
b_mp4_payload_handler_h265(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len)
{
    int rc=0;
    size_t left;
    unsigned count;
    bmedia_packet_header hdr;
    batom_t pes_atom;

    BDBG_MSG_TRACE(("b_mp4_payload_handler_h265:%#lx:%#lx:%u payload %u", (unsigned long)demux, (unsigned long)stream, frame_no, (unsigned)payload_len));

    batom_accum_clear(stream->frame_accum);

    BMEDIA_PACKET_HEADER_INIT(&hdr);
    BMEDIA_PES_SET_PTS(&hdr,bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));

    if(frame_no==0) { /* need to allocate data rai and append for pps,sps and  rai */
        stream->vecs[B_MP4_STREAM_VEC_FRAME] = &bmedia_nal_vec;
        stream->vecs[B_MP4_STREAM_VEC_RAI] = NULL;
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_eos_h265;

        if(stream->track.codec_private.data) {
            if(stream->track.fragment_header.fourcc == BMEDIA_FOURCC('h','v','c','1')) {
                batom_cursor cursor;
                b_mp4_meta_atom_info meta_info;
                bmedia_h265_meta meta;
                batom_t meta_marker;
                unsigned configurationVersion;

                batom_cursor_from_atom(&cursor, stream->track.codec_private.data);
                configurationVersion = batom_cursor_byte(&cursor);
                if(BATOM_IS_EOF(&cursor)) {
                    goto error;
                }
                if(configurationVersion!=1) {
                    BDBG_ERR(("b_mp4_payload_handler_h265: not supported version %u", configurationVersion));
                    goto error;
                }
                if(!bmedia_read_h265_meta(&cursor, &meta, demux->config.alloc)) {
                    BDBG_ERR(("b_mp4_payload_handler_h265:%#lx:%#lx invalid avc1 meta data", (unsigned long)demux, (unsigned long)stream));
                    goto error;
                }
                bmedia_copy_h265_meta(stream->frame_accum, &meta, &demux->h264.nal_vec);
                meta_info.buffer = meta.data;
                meta_info.demux = demux;
                meta_marker = batom_from_range(demux->config.factory,demux->h264.nal_prefix, 1, &b_mp4_meta_atom, &meta_info); /* construct a vector that just carries 0 and marker so memory would get freed once metadata delivered */
                if(meta_marker==NULL) { (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
                batom_accum_add_atom(stream->frame_accum, meta_marker);
                batom_release(meta_marker);
            } else /* fourcc==BMEDIA_FOURCC('h','e','v','1') */ {
                batom_accum_add_atom(stream->frame_accum, stream->track.codec_private.data);
            }
        }
        hdr.key_frame = true;
    }

    for(count=0,left=payload_len;left>0;count++) {
        batom_cursor nal_start;
        const size_t lengthSize = 4;
        uint32_t nal_len;
        if(left<=lengthSize) {
            BDBG_MSG_TRACE(("b_mp4_payload_handler_h265:%#lx:%#lx invalid frame, left %u", (unsigned long)demux, (unsigned long)stream, (unsigned)left));
            rc = -1;
            goto error;
        }
        left -= lengthSize;
        nal_len = batom_cursor_vword_be(payload, lengthSize);
        /*  BDBG_MSG(("nal_len:%u", nal_len)); */
        BATOM_CLONE(&nal_start, payload);
        if(nal_len==0 || nal_len > left || nal_len!=batom_cursor_skip(payload, nal_len)) {
            BDBG_MSG_TRACE(("b_mp4_payload_handler_h265:%#lx:%#lx not enough frame data %u:%u", (unsigned long)demux, (unsigned long)stream, (unsigned)nal_len, (unsigned)left));
            rc = -1;
            goto error;
        }
        if(count==0 && frame_no!=0) {
            hdr.header_type = B_MP4_STREAM_VEC_FRAME;
        } else {
            batom_accum_add_range(stream->frame_accum, demux->h264.nal_prefix, bmedia_nal_vec.len);
        }
        batom_accum_append(stream->frame_accum, stream->data, &nal_start, payload);
        left -= nal_len;
    }
    if(frame_no==0) { /* could free codec_private.data */
        if(stream->track.codec_private.data) {
            batom_release(stream->track.codec_private.data);
            stream->track.codec_private.data=NULL;
        }
    }
    pes_atom = batom_from_accum(stream->frame_accum, bmedia_atom, &hdr);
    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;

error:
    batom_accum_clear(stream->frame_accum);
    return -1;
}


static int
b_mp4_payload_handler_aac(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len)
{
    batom_cursor cursor;
    bmedia_adts_hdr hdr;
    batom_t pes_atom;

    BDBG_MSG_TRACE(("b_mp4_payload_handler_aac:%#lx:%#lx:%u payload %u", (unsigned long)demux, (unsigned long)stream, frame_no, (unsigned)payload_len));
    BSTD_UNUSED(demux);
        
    if(frame_no==0) {
        bmedia_info_aac aac_info;

        batom_cursor_from_atom(&cursor, stream->track.codec_private.data);

        if(!bmedia_info_probe_aac_info(&cursor, &aac_info)) {
            return -1;
        }

        bmedia_adts_header_init_aac(&stream->aac.adts_header,&aac_info);
    }
	BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
    BMEDIA_PES_SET_PTS(&hdr.pes, bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));
    bmedia_adts_hdr_init(&hdr, &stream->aac.adts_header, payload_len);
    BATOM_CLONE(&cursor, payload);
    batom_cursor_skip(&cursor, payload_len);
    pes_atom = batom_accum_extract(stream->data, payload, &cursor, &bmedia_adts_atom, &hdr);

    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;
}

#if 0
typedef int16_t b_mp4_short;
typedef uint32_t b_mp4_dword;
typedef uint64_t b_mp4_reference_time;

typedef struct b_mp4_rect {
    b_mp4_short left;
    b_mp4_short top;
    b_mp4_short right;
    b_mp4_short bottom;
} b_mp4_rect;

typedef struct b_mp4_videoinfoheader {
    b_mp4_rect source;
    b_mp4_rect target;
    b_mp4_dword bitrate;
    b_mp4_dword bitErrorRate;
    b_mp4_reference_time avgTimePerFrame;
    bmedia_bitmapinfo header;
} b_mp4_videoinfoheader;

static bool
b_mp4_read_rect(b_mp4_rect *rect, batom_cursor *c)
{
    rect->left = batom_cursor_uint16_le(c);
    rect->top = batom_cursor_uint16_le(c);
    rect->right = batom_cursor_uint16_le(c);
    rect->bottom = batom_cursor_uint16_le(c);
    if(!BATOM_IS_EOF(c)) {
        BDBG_MSG(("b_mp4_read_rect: %u,%u,%u,%u", rect->left, rect->top, rect->right, rect->bottom));
    }
    return !BATOM_IS_EOF(c);
}

static bool
b_mp4_read_videoinfoheader(b_mp4_videoinfoheader *vf, batom_cursor *c)
{
    if(!b_mp4_read_rect(&vf->source, c)) {
        return false;
    }
    if(!b_mp4_read_rect(&vf->target, c)) {
        return false;
    }
    vf->bitrate = batom_cursor_uint32_le(c);
    vf->bitErrorRate = batom_cursor_uint32_le(c);
    vf->avgTimePerFrame = batom_cursor_uint64_le(c);
    if(BATOM_IS_EOF(c)) {
        return false;
    }
    return bmedia_read_bitmapinfo(&vf->header, c);
}
#endif

static int
b_mp4_payload_handler_generic(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len)
{
    batom_cursor cursor;
	bmedia_packet_header hdr;
    batom_t pes_atom;

    BDBG_MSG_TRACE(("b_mp4_payload_handler_generic:%#lx:%#lx:%u payload %u", (unsigned long)demux, (unsigned long)stream, frame_no, (unsigned)payload_len));
    BSTD_UNUSED(demux);
    
	BMEDIA_PACKET_HEADER_INIT(&hdr);
    BMEDIA_PES_SET_PTS(&hdr, bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));
    hdr.key_frame = frame_no==0;
    BATOM_CLONE(&cursor, payload);
    batom_cursor_skip(&cursor, payload_len);
    pes_atom = batom_accum_extract(stream->data, payload, &cursor, bmedia_atom, &hdr);

    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;
}

static int
b_mp4_payload_handler_wma_pro(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, unsigned frame_no, uint32_t timestamp, batom_cursor *payload, size_t payload_len)
{
    batom_cursor cursor;
    bmedia_bcma_hdr hdr;
    batom_t pes_atom;

    BDBG_MSG_TRACE(("b_mp4_payload_handler_wma_pro:%#lx:%#lx:%u payload %u", (unsigned long)demux, (unsigned long)stream, frame_no, (unsigned)payload_len));
    BSTD_UNUSED(demux);

    batom_accum_clear(stream->frame_accum);
    
    if(frame_no==0) {
        batom_cursor_from_atom(&cursor, stream->track.codec_private.data);
        stream->vecs[B_MP4_STREAM_VEC_FRAME] = &bmedia_frame_bcma;
    }
	BMEDIA_PACKET_HEADER_INIT(&hdr.pes);
    BMEDIA_PES_SET_PTS(&hdr.pes, bmedia_time2pts(timestamp, BMEDIA_TIME_SCALE_BASE));
	hdr.pes.header_type = B_MP4_STREAM_VEC_FRAME;
	hdr.pes.header_off = 4;
    hdr.pes.key_frame = true;
    bmedia_bcma_hdr_init(&hdr, payload_len);
    batom_accum_add_atom(stream->frame_accum, stream->track.codec_private.data);
    BATOM_CLONE(&cursor, payload);
    batom_cursor_skip(&cursor, payload_len);
    batom_accum_append(stream->frame_accum, stream->data, payload, &cursor);
    pes_atom = batom_from_accum(stream->frame_accum, &bmedia_bcma_atom, &hdr);

    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;
}

static int
b_mp4_fragment_stream_track_eos(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream)
{
	bmedia_packet_header hdr;
    batom_t pes_atom;
    uint32_t fourcc = stream->track.fragment_header.fourcc;

    BDBG_MSG_TRACE(("b_mp4_fragment_stream_track_eos:%#lx:%#lx", (unsigned long)demux, (unsigned long)stream));
    if(stream->track.payload_handler == b_mp4_payload_handler_h264) {
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_eos_h264;
    } else if(stream->track.payload_handler == b_mp4_payload_handler_h265) {
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_eos_h265;
    } else if( fourcc==BMEDIA_FOURCC('W','V','C','1')) {
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_eos_vc1;
    } else {
        stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_null_vec;
    }
	BMEDIA_PACKET_HEADER_INIT(&hdr);
    hdr.header_type = B_MP4_STREAM_VEC_EOS | B_MEDIA_PACKET_FLAG_EOS;
    pes_atom = batom_empty(demux->config.factory, bmedia_atom, &hdr);
    if(pes_atom) {
        batom_pipe_push(stream->pipe_out, pes_atom);
    }
    return 0;
}
static int
b_mp4_fragment_stream_process_frame(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream, uint32_t timestamp)
{
    int rc;
    batom_cursor frame;

    BSTD_UNUSED(demux);
    batom_cursor_from_accum(&frame, stream->data);
    if(stream->track.track_eos) {
        return b_mp4_fragment_stream_track_eos(demux, stream);
    }
    if(stream->track.next_sample.offset != batom_cursor_skip(&frame, stream->track.next_sample.offset)) {
        BDBG_MSG_TRACE(("b_mp4_fragment_stream_next:%#lx:%#lx frame out of bounds %u", (unsigned long)demux, (unsigned long)stream, (unsigned)stream->track.next_sample.offset));
        rc = -1;
        goto error;
    }
    if(stream->track.next_sample.size != batom_cursor_reserve(&frame, stream->track.next_sample.size)) {
        BDBG_MSG_TRACE(("b_mp4_fragment_stream_next:%#lx:%#lx not enough data %u", (unsigned long)demux, (unsigned long)stream, (unsigned)stream->track.next_sample.size));
        rc = -1;
        goto error;
    }
    /* batom_cursor_dump(&frame, "frame"); */
    if(demux->config.sample) {
        demux->config.sample(demux->config.user_context, stream->track.track_extends.track_ID, stream->track.next_sample_no, &frame, stream->track.next_sample.size);
    }
    rc = stream->track.payload_handler(demux, stream, stream->track.next_sample_no, timestamp, &frame, stream->track.next_sample.size);
    /* rc = -1; */
error:
    return rc;
}

static void
b_mp4_fragment_stream_flush(bmp4_fragment_stream_t stream)
{
    batom_accum_clear(stream->frame_accum);
    batom_accum_clear(stream->data);
    stream->busy = false;
    b_mp4_fragment_track_clear(&stream->track);
    return;
}

static int
b_mp4_fragment_set_handler(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream)
{
    int rc=0;
    uint32_t fourcc = stream->track.fragment_header.fourcc;

    if(fourcc==BMEDIA_FOURCC('a','v','c','1') || fourcc==BMEDIA_FOURCC('H','2','6','4') ) {
        stream->track.payload_handler = b_mp4_payload_handler_h264;
    } else if(fourcc==BMEDIA_FOURCC('h','v','c','1') || fourcc==BMEDIA_FOURCC('h','e','v','1')) {
        stream->track.payload_handler = b_mp4_payload_handler_h265;
    } else if(fourcc==BMEDIA_FOURCC('m','p','4','a') || fourcc==BMEDIA_FOURCC('A','A','C','L')) {
        stream->track.payload_handler = b_mp4_payload_handler_aac;
    } else if(fourcc==BMEDIA_FOURCC('W','V','C','1')) {
        stream->track.payload_handler = b_mp4_payload_handler_generic;
    } else if(fourcc==BMEDIA_FOURCC('W','M','A','P')) {
        stream->track.payload_handler = b_mp4_payload_handler_wma_pro;
    } else if(fourcc==BMEDIA_FOURCC('a','c','-','3') ||  fourcc==BMEDIA_FOURCC('e','c','-','3')) {
        stream->track.payload_handler = b_mp4_payload_handler_generic;
    } else {
        BDBG_WRN(("b_mp4_fragment_set_handler:%#lx %#lx  unknown codec " BMEDIA_FOURCC_FORMAT " for track %u", (unsigned long)demux, (unsigned long)stream, BMEDIA_FOURCC_ARG(fourcc), stream->stream_id));
        rc=-1;
    }
    return rc;
}

static void
b_mp4_fragment_demux_parse_advance(bmp4_fragment_demux_t demux, size_t to_advance)
{
    batom_cursor cursor;
    batom_cursor_from_accum(&cursor, demux->accum);
    batom_cursor_skip(&cursor, to_advance);
    batom_accum_trim(demux->accum, &cursor);
    return;
}

static size_t
b_mp4_fragment_demux_parse_data(bmp4_fragment_demux_t demux, batom_pipe_t pipe, bmp4_fragment_stream_t  idle_stream)
{
    size_t len;
    batom_cursor cursor;
    bmp4_fragment_stream_t stream;

    for(len=0;idle_stream;) {
        batom_t atom=batom_pipe_pop(pipe);
        BDBG_MSG_TRACE(("bmp4_fragment_demux_feed:%#lx atom:%#lx len:%u state:%u", (unsigned long)demux, (unsigned long)atom, len, demux->demux_state)); 
        if(!atom) {
            break;
        }
        len += batom_len(atom);
	    batom_accum_add_atom(demux->accum, atom);
        batom_release(atom);
        for(;;) {
            if(demux->demux_state == b_mp4_fragment_demux_state_box) {
		        bmp4_box box;	
                size_t box_header_size;

                BDBG_MSG(("bmp4_fragment_demux_feed:%#lx looking for box", (unsigned long)demux));
                batom_cursor_from_accum(&cursor, demux->accum);
                box_header_size = bmp4_parse_box(&cursor, &box);
                if(box_header_size==0) {
                    break;
                }
                if(box.type != B_MP4_FRAGMENT_WRAPPER) {
                    b_mp4_fragment_demux_parse_advance(demux, 1);
                    continue;
                }
                demux->box_size = box.size;
                demux->box_header_size = box_header_size;
                demux->demux_state = b_mp4_fragment_demux_state_capture;
            } else if(demux->demux_state == b_mp4_fragment_demux_state_capture) {
                size_t accum_len = batom_accum_len(demux->accum);
                BDBG_MSG(("bmp4_fragment_demux_feed:%p capturing data %u:%u:%u", (void *)demux, (unsigned)accum_len, (unsigned)demux->box_size, (unsigned)len));
                if(accum_len >= demux->box_size) {
                    int rc;
                    BDBG_ASSERT(idle_stream);
                    batom_cursor_from_accum(&cursor, demux->accum);
                    rc=b_mp4_fragment_track_start(&idle_stream->track, demux->accum, demux->box_header_size, demux->box_size, &cursor);
                    if(rc<0) {
                        b_mp4_fragment_demux_report_error(demux);
                        break;
                    }
                    for(stream=BLST_S_FIRST(&demux->streams);stream!=NULL;stream=BLST_S_NEXT(stream, link)) {
                        if(stream->stream_id == idle_stream->track.track_extends.track_ID) {
                            BDBG_MSG(("bmp4_fragment_demux_feed:%#lx captured box for track:%#lx(%u) %s", (unsigned long)demux, (unsigned long)stream, stream->stream_id, stream->busy?"BUSY":""));
                            break;
                        }
                    }
                    if(stream) {
                        if(!stream->busy) {
                            if(stream!=idle_stream) {
                                stream->track = idle_stream->track; /* copy track state, it should be safe, no pointers there */
                                b_mp4_fragment_track_init(&idle_stream->track); /* clear remnants of copied data */
                            } 
                            rc = b_mp4_fragment_stream_start(demux, stream, &cursor, demux->accum, demux->box_size);
                            if(rc!=0) {
                                b_mp4_fragment_track_clear(&idle_stream->track); /* recycle data */
                                idle_stream = NULL;
                                goto stream_done;
                            }
                            idle_stream = NULL;
                            rc = b_mp4_fragment_set_handler(demux, stream);
                            if(rc!=0) {
                                b_mp4_fragment_stream_flush(stream);
                                goto stream_done;
                            }
stream_done:
                            demux->demux_state = b_mp4_fragment_demux_state_box;
                            b_mp4_fragment_demux_parse_advance(demux, demux->box_size);
                        } else {
                            b_mp4_fragment_track_clear(&idle_stream->track); /* recycle data */
                            demux->next_fragment_track_ID = idle_stream->track.track_extends.track_ID;
                            demux->demux_state = b_mp4_fragment_demux_state_remux; /* need to parse captured blocks */
                        } 
                    } else {
                        BDBG_MSG(("bmp4_fragment_demux_feed:%#lx discarding fragment %u", (unsigned long)demux, idle_stream->track.track_extends.track_ID));
                        b_mp4_fragment_demux_parse_advance(demux, demux->box_size);
                        b_mp4_fragment_track_clear(&idle_stream->track); /* recycle data */
                        demux->demux_state = b_mp4_fragment_demux_state_box;
                    }
                    if(idle_stream) {
                        b_mp4_fragment_track_clear(&idle_stream->track); /* recycle data */
                    }
                }
                break;
            } else {
                (void)BERR_TRACE(BERR_NOT_SUPPORTED);
                break;
            }
        }
    }
    return len;
}

static int
b_mp4_fragment_demux_remux_tracks(bmp4_fragment_demux_t demux)
{
    bmp4_fragment_stream_t  stream;
    bmp4_fragment_stream_t  min_stream;
    uint32_t min_time;
    unsigned i;

    for(i=0;i<demux->config.burst_frames;) {
        int rc;
        for(min_time=0, min_stream=NULL,stream=BLST_S_FIRST(&demux->streams);stream!=NULL;stream=BLST_S_NEXT(stream, link)) {
            uint32_t time;
            batom_cursor cursor;
            if(!stream->busy) {
                continue;
            }

            batom_cursor_from_accum(&cursor, stream->data);
            if(!stream->track.next_sample_valid) {
                bool end_of_fragment=false;
                rc = b_mp4_fragment_track_next(demux, &stream->track, &cursor, &end_of_fragment);
                if(rc!=0) {
                    if(end_of_fragment) {
                        BDBG_MSG(("b_mp4_fragment_demux_remux_tracks:%#lx reached end of track %#lx:%u", (unsigned long)demux, (unsigned long)stream, stream->stream_id));
                        b_mp4_fragment_stream_flush(stream);
                        if(demux->next_fragment_track_ID == stream->track.track_extends.track_ID) {
                            demux->next_fragment_track_ID = 0;
                            demux->demux_state = b_mp4_fragment_demux_state_box; /* need to capture more data */
                        }
                    }
                    continue;
                }
                stream->track.next_sample_valid=true;
            }
            time = (1000*(stream->track.next_sample.time + (int32_t)stream->track.next_sample.composition_time_offset)) / stream->track.fragment_header.timescale;
            if(!min_stream || min_time>time) {
                min_stream = stream;
                min_time = time;
            } 
        }
        if(!min_stream) {
            break;
        }
        BDBG_MSG(("b_mp4_fragment_demux_remux_tracks:%#lx selected stream (%u:%u) %u:" BMEDIA_FOURCC_FORMAT " %#lx:%u", (unsigned long)demux, i, demux->config.burst_frames, min_stream->track.track_extends.track_ID, BMEDIA_FOURCC_ARG(min_stream->track.fragment_header.fourcc), (unsigned long)min_stream, min_time));
        rc=b_mp4_fragment_stream_process_frame(demux, min_stream, min_time);
        if(rc==0) {
            i++;
        }
        min_stream->track.next_sample_valid=false;
    }
    return 0;
}


size_t 
bmp4_fragment_demux_feed(bmp4_fragment_demux_t demux, batom_pipe_t pipe)
{
    size_t len=0;
    bmp4_fragment_stream_t  idle_stream;
    bmp4_fragment_stream_t  stream;

    BDBG_MSG_TRACE(("bmp4_fragment_demux_feed>:%#lx %#lx", (unsigned long)demux, (unsigned long)pipe));
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    BDBG_ASSERT(pipe);

    if(demux->demux_state != b_mp4_fragment_demux_state_remux) {
        for(stream=BLST_S_FIRST(&demux->streams);stream!=NULL;stream=BLST_S_NEXT(stream, link)) {
            if(!stream->busy) {
                break;
            }
        }
        idle_stream = stream;
        if(idle_stream!=NULL || BLST_S_FIRST(&demux->streams)==NULL ) {
            len = b_mp4_fragment_demux_parse_data(demux, pipe, idle_stream);
        }
    }
    b_mp4_fragment_demux_remux_tracks(demux);
    BDBG_MSG_TRACE(("bmp4_fragment_demux_feed<: %#lx processed %u bytes state:%u", (unsigned long)demux, len, demux->demux_state));
    return len;
}

int 
bmp4_fragment_demux_seek(bmp4_fragment_demux_t demux, uint64_t off)
{
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    if(off!=demux->offset) {
        bmp4_fragment_demux_flush(demux);
        demux->offset = off;
    }
    return 0;
}


void bmp4_fragment_demux_flush(bmp4_fragment_demux_t demux)
{
    bmp4_fragment_stream_t  stream;
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    batom_accum_clear(demux->accum);

    for(stream=BLST_S_FIRST(&demux->streams);stream!=NULL;stream=BLST_S_NEXT(stream, link)) {
        b_mp4_fragment_stream_flush(stream);
    }
    demux->next_fragment_track_ID = 0;
    demux->demux_state = b_mp4_fragment_demux_state_box; 

    return;
}

void 
bmp4_fragment_demux_get_default_stream_cfg(bmp4_fragment_demux_t demux, bmp4_fragment_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    BSTD_UNUSED(demux); 
	BDBG_ASSERT(cfg);
    BKNI_Memset(cfg, 0, sizeof(*cfg));
    return;
}

void 
bmp4_fragment_get_stream_cfg(bmp4_fragment_stream_t stream, bmedia_pes_stream_cfg *cfg)
{
    BDBG_OBJECT_ASSERT(stream, bmp4_fragment_stream);

	bmedia_pes_default_stream_cfg(cfg);
    cfg->nvecs = sizeof(stream->vecs)/sizeof(stream->vecs[0]);
    cfg->vecs = stream->vecs;
    return;
}


bmp4_fragment_stream_t 
bmp4_fragment_stream_create(bmp4_fragment_demux_t demux, const bmp4_fragment_stream_cfg *cfg, unsigned stream_id, batom_pipe_t pipe_out)
{
    bmp4_fragment_stream_t  stream;
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(pipe_out);

    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(stream, bmp4_fragment_stream);
    stream->busy = false;
    stream->pipe_out = pipe_out;
    stream->stream_id = stream_id;
	stream->vecs[B_MP4_STREAM_VEC_FRAME] = &bmedia_null_vec;
	stream->vecs[B_MP4_STREAM_VEC_RAI] = &bmedia_null_vec;
	stream->vecs[B_MP4_STREAM_VEC_EOS] = &bmedia_null_vec;

    stream->frame_accum = batom_accum_create(demux->config.factory);
    if(!stream->frame_accum) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_frame_accum;}
    stream->data = batom_accum_create(demux->config.factory);
    if(!stream->data) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_data;}
    b_mp4_fragment_track_init(&stream->track);
    BLST_S_INSERT_HEAD(&demux->streams, stream, link);
    return stream;

err_data:
    batom_accum_destroy(stream->frame_accum);
err_frame_accum:
    BKNI_Free(stream);
err_alloc:
    return NULL;
}

void
bmp4_fragment_stream_destroy(bmp4_fragment_demux_t demux, bmp4_fragment_stream_t stream)
{
    BDBG_OBJECT_ASSERT(demux, bmp4_fragment_demux);
    BDBG_OBJECT_ASSERT(stream, bmp4_fragment_stream);
    BLST_S_REMOVE(&demux->streams, stream, bmp4_fragment_stream, link);
    batom_accum_destroy(stream->data);
    batom_accum_destroy(stream->frame_accum);
    BDBG_OBJECT_DESTROY(stream, bmp4_fragment_stream);
    BKNI_Free(stream);
    return;
}

