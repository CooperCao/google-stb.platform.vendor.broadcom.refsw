/******************************************************************************
 * (c) 2006-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "blst_squeue.h"

#include "brtp.h"
#include "brtp_packet.h"
#include "brtp_parser_h264.h"
#include "brtp_parser_mpeg4.h"
#include "bbase64.h"
#include "btimestamp.h"
#include "brtp_util.h"
#include "brtp_play.h"

#include "lm.h"

BDBG_MODULE(brtp_play);


#include <pthread.h>
#include <errno.h>

typedef struct b_rtp_play_task {
    pthread_mutex_t mutex;
    pthread_t thread;
} b_rtp_play_task;

static int
b_rtp_play_start(b_rtp_play_task *task, void *(*play_task)(void *cntx), void *cntx)
{
    int rc;

    rc = pthread_mutex_init(&task->mutex, NULL);
    if (rc!=0) {
        BDBG_ERR(("b_rtp_play_start: pthread_mutex_init failed with %d", errno));
        return -1;
    }

    rc = pthread_create(&task->thread, NULL, play_task, cntx);
    if (rc!=0) {
        pthread_mutex_destroy(&task->mutex);
        BDBG_ERR(("b_rtp_play_start: pthread_create failed with %d", errno));
        return -1;
    }
    return 0;
}

static void
b_rtp_play_stop(b_rtp_play_task *task)
{
    int rc;
    void *result;

    rc = pthread_join(task->thread, &result);
    BDBG_WRN(("join completed"));
    if(rc!=0) {
        BDBG_ERR(("b_rtp_play_stop: pthread_join failed with %d", errno));
    }
    pthread_mutex_destroy(&task->mutex);
    return;
}

#define B_PLAY_IDLE_TIMER_PERIOD    100
static void
b_rtp_play_idle(b_rtp_play_task *task)
{
    int rc;
    rc = pthread_mutex_unlock(&task->mutex);
    if (rc!=0) {
        BDBG_ERR(("b_rtp_play_idle: pthread_mutex_unlock returned %d", errno));
    }
    BKNI_Sleep(1);
    rc = pthread_mutex_lock(&task->mutex);
    if (rc!=0) {
        BDBG_ERR(("b_rtp_play_idle: pthread_mutex_lock returned %d", errno));
    }
    return;
}

static void
b_rtp_play_lock(b_rtp_play_task *task)
{
    int rc;
    rc = pthread_mutex_lock(&task->mutex);
    if (rc!=0) {
        BDBG_ERR(("b_rtp_play_lock: pthread_mutex_lock returned %d", errno));
    }
    return;
}

static void
b_rtp_play_unlock(b_rtp_play_task *task)
{
    int rc;
    rc = pthread_mutex_unlock(&task->mutex);
    if (rc!=0) {
        BDBG_ERR(("b_rtp_play_unlock: pthread_mutex_unlock returned %d", errno));
    }
    return;
}

BDBG_OBJECT_ID(brtp_channel);
BDBG_OBJECT_ID(brtp_play);

typedef struct brtp_channel {
    BDBG_OBJECT(brtp_channel)
    brtp_t rtp;
    brtp_parser_t parser;
    lm_stream_t lm_stream;
    brtp_parser_mux_stream_t stream;
    struct brtp_play *play;
    struct {
        bool rtcp_sr:1;
        bool rtp_ts:1;
    } sync;
    brtp_ntp sr_ntp;
    uint32_t sr_timestamp;
    uint32_t rtp_timestamp;
    brtp_parser_mux_stream_cfg stream_cfg;
    const struct brtp_play_stream_config *play_cfg;
    uint8_t rtp_format;
    int sock_in;
    uint8_t cfg[256];
    uint8_t cfg_len;
} brtp_channel;

struct brtp_play {
    BDBG_OBJECT(brtp_play)
    brtp_parser_mux_t mux;
    lm_session_t session;
    lm_timer_t timer;
    brtp_play_config cfg;
    unsigned time_playing;
    struct brtp_channel *video;
    struct brtp_channel *audio;
    struct brtp_channel channels[2];
    lm_state_t lm;
    b_rtp_play_task task;
    char lm_stop;
    lm_timer_t idle_timer;
    enum {b_rtp_play_stopped, b_rtp_play_playing, b_rtp_play_end_of_stream} state;
};


static void
b_rtp_start_parser(struct brtp_channel *chn, uint32_t timestamp_offset)
{
    brtp_session_cfg session_cfg;

    BDBG_ASSERT(chn->play);
    BDBG_ASSERT(chn->play->mux);
    BDBG_ASSERT(chn->rtp);
    BDBG_ASSERT(chn->parser);
    BDBG_ASSERT(chn->stream==NULL);

    BDBG_WRN(("b_rtp_sync_one: starting stream %#lx[0x%02x] offset %u", (unsigned long)chn, chn->stream_cfg.pes_id, timestamp_offset));

    brtp_default_session_cfg(chn->rtp, &session_cfg);
    session_cfg.pt_mask = 0xFF;
    session_cfg.pt_value = chn->rtp_format;
    brtp_start(chn->rtp, chn->parser, &session_cfg);

    chn->stream_cfg.clock_offset = timestamp_offset;
    chn->stream_cfg.sink_stream_cntx = chn;

    chn->stream = chn->parser->start(chn->parser, chn->play->mux, &chn->stream_cfg, &chn->cfg, chn->cfg_len);
    BDBG_ASSERT(chn->stream);
    return;
}

static void
b_rtp_timestamp_pkt(void *cntx, const void *rtp_pkt)
{
    struct brtp_channel *chn = cntx;
    uint32_t timestamp_offset;

    BDBG_OBJECT_ASSERT(chn, brtp_channel);

    if(chn->sync.rtp_ts) {
        return;
    }

    chn->sync.rtp_ts = true;
    chn->rtp_timestamp = B_RTP_TIMESTAMP(rtp_pkt) ;
    if (chn->stream) {
        return;
    }
    if(chn->parser->type->type == brtp_stream_video) {
        timestamp_offset = chn->rtp_timestamp - (uint32_t)(chn->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY);
        BDBG_WRN(("autosync %s video timestamp_offset=%u", chn->parser->type->name, timestamp_offset));
        b_rtp_start_parser(chn, timestamp_offset);
    }
    return;
}

static brtp_enqueue
b_rtp_data(void *rtp, void *pkt, size_t len, void *cookie)
{
    struct brtp_channel *chn = rtp;
    brtp_enqueue status;
    brtp_pkt_info pkt_info[8];
    int ninfo;

    status = brtp_enqueue_pkt(chn->rtp, pkt, len, cookie);

    if(chn->lm_stream) { /* XXX this takes too much CPU cyles for the high bandwidth streams */
        ninfo = brtp_get_pkt_info(chn->rtp, pkt_info, sizeof(pkt_info)/sizeof(*pkt_info));
        if (ninfo>0) {
            lm_stream_update_pkt_info(chn->lm_stream, pkt_info, ninfo);
        }
    }
    return status;
}

static void
b_rtp_sync_one(void *cntx, uint32_t ntp_msw, uint32_t ntp_lsw, uint32_t timestamp)
{
    struct brtp_channel *chn = cntx;
    struct brtp_channel *video=NULL;
    struct brtp_channel *audio=NULL;
    int32_t ntp_delta;
    int32_t ts_delta;
    int32_t audio_ts_delta;
    int32_t rtp_ts;
    unsigned timestamp_offset;

    BDBG_OBJECT_ASSERT(chn, brtp_channel);
    BDBG_OBJECT_ASSERT(chn->play, brtp_play);
    BDBG_ASSERT(chn->play->mux);
    BDBG_ASSERT(chn->rtp);
    BDBG_ASSERT(chn->parser);
    brtp_ntp_init(&chn->sr_ntp, ntp_msw, ntp_lsw);
    chn->sr_timestamp = timestamp;
    chn->play_cfg->sync(chn->play_cfg->stream_cntx, &chn->stream_cfg, &chn->sr_ntp, timestamp);
    chn->sync.rtcp_sr = true;
    audio = chn->play->audio;
    video = chn->play->video;
    if (video->stream==NULL) {
        BDBG_WRN(("%#lx don't start %s video until rtp timestamp is known", (unsigned long)chn, chn->parser->type->name));
        return;
    }
    if(audio->stream) {
        return; /* already started */
    }
    BDBG_ASSERT(audio);
    BDBG_ASSERT(video);
    if (!video->sync.rtcp_sr) {
        BDBG_WRN(("%#lx don't start %s audio until %s video synchronized", (unsigned long)chn, audio->parser->type->name, video->parser->type->name));
        return;
    }
    if (!audio->sync.rtcp_sr) {
        BDBG_WRN(("%#lx don't start %s audio until rtcp_sr arrived", (unsigned long)chn, audio->parser->type->name, video->parser->type->name));
        return;
    }
    /* this is an audio stream and it's time to calculate delay between audio and video */
    ntp_delta = brtp_ntp_diff(&video->sr_ntp, &audio->sr_ntp);
    ts_delta = (brtp_delta_time(video->rtp_timestamp, video->sr_timestamp)*(int64_t)2000)/(int)video->stream_cfg.clock_rate;
    if(audio->sync.rtp_ts) {
        audio_ts_delta = (brtp_delta_time(audio->rtp_timestamp, audio->sr_timestamp)*(int64_t)2000)/(int)audio->stream_cfg.clock_rate; /* 2000*(delta_ts/2)/clock_rate, get RTP TS delta in ms */
    } else {
        audio_ts_delta = 0;
    }
    rtp_ts = ((ntp_delta+ts_delta)*((int64_t)audio->stream_cfg.clock_rate))/1000; /* calculate rtp_ts that shall bring audio back to 0 */
    timestamp_offset = (uint32_t)(audio->sr_timestamp + rtp_ts - (int)(audio->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY)) ;
    BDBG_WRN(("delta:(%d) ntp:%d ts:%d(%d), rtp_ts=%d(%d) timestamp_offset=%u (audio ts:%d)", ntp_delta+ts_delta, ntp_delta, ts_delta, audio_ts_delta, rtp_ts, audio->sr_timestamp, timestamp_offset, (brtp_delta_time(audio->rtp_timestamp, audio->sr_timestamp)*125)/((int)audio->stream_cfg.clock_rate/16)));
    b_rtp_start_parser(audio, timestamp_offset);
    return;
}

static void
b_rtp_bye(void *cntx)
{
    struct brtp_channel *chn = cntx;
    BDBG_OBJECT_ASSERT(chn, brtp_channel);
    BDBG_OBJECT_ASSERT(chn->play, brtp_play);
    BDBG_WRN(("bye %#lx", (unsigned long)chn));
    return;
}


static int
b_rtp_connect_one(struct brtp_play *play, blm_media media)
{
    const lm_stream_desc *desc;
    struct brtp_channel *chn = &play->channels[media];
    brtp_parser_h264_cfg h264_cfg;
    brtp_parser_mpeg4_cfg mpeg4_cfg;
    brtp_cfg cfg;
    lm_stream_connect_cfg connect_cfg;
    BDBG_OBJECT_ASSERT(play, brtp_play);

    desc = lm_get_stream_desc(play->session, media);
    BDBG_ASSERT(desc);
    brtp_default_cfg(&cfg);
    switch(desc->stream_type) {
    case blm_stream_h264:
        BDBG_ASSERT(media==blm_media_video);
        brtp_parser_h264_default_cfg(&h264_cfg);
        h264_cfg.meta = BKNI_Malloc(h264_cfg.meta_len);
        chn->parser = brtp_parser_h264_create(&h264_cfg);
        break;
    case blm_stream_aac_hbr:
        BDBG_ASSERT(media==blm_media_audio);
        brtp_parser_mpeg4_default_cfg(&mpeg4_cfg);
        chn->parser = brtp_parser_mpeg4_create(&mpeg4_cfg);
        break;
    default:
        BDBG_ERR(("b_rtp_play_one: unknown stream type %d", desc->stream_type));
        return -1;
    }
    BDBG_ASSERT(chn->parser);
    cfg.noparser_cntx = chn;
    cfg.noparser = b_rtp_timestamp_pkt;
    cfg.pkt_free = lm_pkt_free;
    chn->rtp = brtp_create(&cfg);
    chn->cfg_len = desc->cfg_len;
    chn->stream_cfg.clock_rate = desc->clock_rate;
    chn->rtp_format = desc->rtp_format;
    BKNI_Memcpy(chn->cfg, desc->cfg, desc->cfg_len);
    BDBG_ASSERT(chn->rtp);
    lm_stream_default_cfg(&connect_cfg);
    connect_cfg.rtp = chn;
    connect_cfg.rtp_data = b_rtp_data;
    connect_cfg.rtcp_sr = b_rtp_sync_one;
    connect_cfg.rtcp_bye = b_rtp_bye;
    connect_cfg.use_tcp = play->cfg.use_tcp;
    connect_cfg.stream_cntx = chn;
    chn->lm_stream = lm_session_stream(play->session, media, &connect_cfg);
    BDBG_ASSERT(chn->lm_stream);
    return 0;
}

static void
b_rtp_payload(void *cntx, brtp_io_block *block, brtp_parser_t parser, uint32_t pts)
{
    struct brtp_play  *play = cntx;
    struct brtp_channel *chn = parser->stream.cfg.sink_stream_cntx;
    BDBG_OBJECT_ASSERT(play, brtp_play);
    BDBG_OBJECT_ASSERT(chn, brtp_channel);
    chn->play_cfg->data(chn->play_cfg->stream_cntx, block, pts);
    brtp_parser_mux_recycle(play->mux, block);
    return;
}

static void
b_rtp_play_skip_data(void *stream_cntx, brtp_io_block *block, uint32_t pts)
{
    BSTD_UNUSED(stream_cntx);
    BSTD_UNUSED(block);
    BSTD_UNUSED(pts);
    return;
}

static void
b_rtp_play_skip_sync(void *stream_cntx, const brtp_parser_mux_stream_cfg *stream_cfg, const brtp_ntp *ntp, uint32_t rtcp_timestamp)
{
    BSTD_UNUSED(stream_cntx);
    BSTD_UNUSED(stream_cfg);
    BSTD_UNUSED(ntp);
    BSTD_UNUSED(rtcp_timestamp);
    return;
}

static void
b_rtp_play_skip_timer(void *play_cntx)
{
    BSTD_UNUSED(play_cntx);
    return;
}

static void
b_rtp_play_skip_end_of_stream(void *play_cntx)
{
    BSTD_UNUSED(play_cntx);
    return;
}

#define B_PLAY_TIMER_PERIOD 1000
static void
b_rtp_play_timer(void *cntx)
{
    struct brtp_play  *play = cntx;
    BDBG_OBJECT_ASSERT(play, brtp_play);
    play->cfg.timer(play->cfg.play_cntx);
    play->timer = lm_timer_schedulle(play->lm, B_PLAY_TIMER_PERIOD, b_rtp_play_timer, play);
    if(play->audio || play->video) {
        lm_session_status st;
        lm_session_get_status(play->session, &st);
        play->time_playing++; /* XXX account for time drift, use gettimeofday or Co */
        if (st.end_time > 0 && play->state != b_rtp_play_end_of_stream && play->time_playing>st.end_time) {
            BDBG_WRN(("end_of_play"));
            play->state = b_rtp_play_end_of_stream;
            b_rtp_play_unlock(&play->task); /* temporarily release a lock for call-out */
            play->cfg.end_of_stream(play->cfg.play_cntx);
            b_rtp_play_lock(&play->task);
        }
    }
    return;
}

static const brtp_play_config b_rtp_play_default_config = {
    {NULL, b_rtp_play_skip_data, b_rtp_play_skip_sync},
    {NULL, b_rtp_play_skip_data, b_rtp_play_skip_sync},
    NULL,
    b_rtp_play_skip_timer,
    b_rtp_play_skip_end_of_stream,
    false /* use_tcp */
};

static void
b_rtp_play_idle_timer(void *cntx)
{
    struct brtp_play  *play = cntx;
    BDBG_OBJECT_ASSERT(play, brtp_play);
    b_rtp_play_idle(&play->task);
    play->idle_timer = lm_timer_schedulle(play->lm, B_PLAY_IDLE_TIMER_PERIOD, b_rtp_play_idle_timer, play);
    return;
}

static void *
b_rtp_play_worker(void *cntx)
{
    brtp_play_t play = cntx;

    BDBG_OBJECT_ASSERT(play, brtp_play);
    b_rtp_play_lock(&play->task);
    b_rtp_play_idle_timer(play); /* start timer */
    play->lm_stop = 0;
    lm_run(play->lm, &play->lm_stop);
    b_rtp_play_unlock(&play->task);
    BDBG_WRN(("worker completed"));
    return NULL;
}

brtp_play_t
brtp_play_create(void)
{
    brtp_play_t  play;
    int rc;

    play = BKNI_Malloc(sizeof(*play));
    if (!play) {
        BDBG_ERR(("brtp_play_create: can't alloced %d bytes of memory", sizeof(*play)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(play, brtp_play);
    play->state = b_rtp_play_stopped;
    play->lm = lm_init();
    if (play->lm==NULL) {
        goto err_lm;
    }
    rc = b_rtp_play_start(&play->task, b_rtp_play_worker, play);
    if (rc!=0) {
        goto err_task;
    }
    return play;
err_task:
    lm_shutdown(play->lm);
err_lm:
    BDBG_OBJECT_DESTROY(play, brtp_play);
    BKNI_Free(play);
err_alloc:
    return NULL;
}

void
brtp_play_default_cfg(brtp_play_t play, brtp_play_config *config)
{
    BSTD_UNUSED(play);

    BDBG_OBJECT_ASSERT(play, brtp_play);

    *config = b_rtp_play_default_config;
    return;
}

brtp_source_t
brtp_play_connect(brtp_play_t play, const char *url)
{
    lm_session_t session;

    BDBG_OBJECT_ASSERT(play, brtp_play);
    /* don't need to obtain a lock */

    session = lm_connect(play->lm, url);

    return (brtp_source_t)session;
}

int
brtp_play_start(brtp_play_t play, brtp_source_t source, const brtp_play_config *config)
{
    brtp_parser_mux_cfg mux_cfg;
    unsigned i;
    int rc;

    BDBG_OBJECT_ASSERT(play, brtp_play);
    BDBG_ASSERT(source);
    b_rtp_play_lock(&play->task);

    for(i=0;i<sizeof(play->channels)/sizeof(*play->channels);i++) {
        struct brtp_channel *ch = &play->channels[i];
        BDBG_OBJECT_INIT(ch, brtp_channel);
        ch->rtp = NULL;
        ch->parser = NULL;
        ch->lm_stream = NULL;
        ch->stream = NULL;
        ch->play = play;
        ch->sync.rtcp_sr = false;
        ch->sync.rtp_ts = false;
    }
    play->cfg = *config;
    play->video = &play->channels[blm_media_video];
    play->audio = &play->channels[blm_media_audio];
    play->video->stream_cfg.pes_id = 0xE0;
    play->audio->stream_cfg.pes_id = 0xC0;
    play->video->play_cfg = &play->cfg.video;
    play->audio->play_cfg = &play->cfg.audio;
    play->session = NULL;
    play->mux = NULL;
    play->timer = NULL;
    play->time_playing = 0;

    brtp_parser_mux_default_cfg(&mux_cfg);
    mux_cfg.meta = BKNI_Malloc(mux_cfg.meta_len);
    mux_cfg.data_sink = b_rtp_payload;
    mux_cfg.data_sink_cntx = play;
    mux_cfg.mode = brtp_mux_mode_pes;
    play->mux = brtp_parser_mux_create(&mux_cfg);

    play->session = (lm_session_t)source;
    if (!play->session) {
        goto no_session;
    }
    play->state = b_rtp_play_playing;
    b_rtp_connect_one(play, blm_media_video);
    b_rtp_connect_one(play, blm_media_audio);

    play->timer = lm_timer_schedulle(play->lm, B_PLAY_TIMER_PERIOD, b_rtp_play_timer, play);
    rc = lm_session_play(play->session);
    if (rc!=0) {
        goto no_play;
    }

    b_rtp_play_unlock(&play->task);

    return 0;
no_play:
    lm_session_close(play->session);

no_session:
    play->state = b_rtp_play_stopped;
    b_rtp_play_unlock(&play->task);
    return -1;
}

int
brtp_play_stop(brtp_play_t play)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(play, brtp_play);
    BDBG_WRN(("brtp_play_stop: %#lx", (unsigned long)play));
    b_rtp_play_lock(&play->task);
    if(!play->session) {
        goto done;
    }
    lm_timer_cancel(play->lm, play->timer);
    lm_session_close(play->session);
    play->session = NULL;

    for(i=0;i<sizeof(play->channels)/sizeof(*play->channels);i++) {
        struct brtp_channel *ch = &play->channels[i];
        BDBG_OBJECT_ASSERT(ch, brtp_channel);
        if(ch->parser) {
            ch->parser->stop(ch->parser);
            ch->parser->destroy(ch->parser);
            ch->parser = NULL;
        }
        if(ch->rtp) {
            brtp_stop(ch->rtp);
            brtp_destroy(ch->rtp);
            ch->rtp = NULL;
        }
    }
done:
    brtp_parser_mux_reset(play->mux);
    play->state = b_rtp_play_stopped;
    BDBG_OBJECT_DESTROY(&play->channels[blm_media_video], brtp_channel);
    BDBG_OBJECT_DESTROY(&play->channels[blm_media_audio], brtp_channel);
    b_rtp_play_unlock(&play->task);
    return 0;
}

void
brtp_play_destroy(brtp_play_t play)
{
    BDBG_OBJECT_ASSERT(play, brtp_play);

    BDBG_ERR(("brtp_play_destroy 1"));
    b_rtp_play_lock(&play->task);
    play->lm_stop = 1;
    BDBG_ERR(("brtp_play_destroy 2"));

    b_rtp_play_unlock(&play->task);
    BDBG_ERR(("brtp_play_destroy 3"));
    b_rtp_play_stop(&play->task);
    BDBG_ERR(("brtp_play_destroy 4"));
    lm_shutdown(play->lm);
    BDBG_OBJECT_DESTROY(play, brtp_play);
    BKNI_Free(play);
    return;
}
