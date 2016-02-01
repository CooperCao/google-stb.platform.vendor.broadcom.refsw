/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
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
 * RTP playback module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bsettop.h"
#include "bsettop_pvr_priv.h"
#include "bsettop_playback_rtsp.h"
#include "bstd.h"
#include "brtp_util.h"
#include "brtp_play.h"
#include "brtp_delay.h"
#include "bkni.h"

BDBG_MODULE(playback_rtsp);

#define B_PTS_DELAY_NORM		500

#define B_PTS_DELAY_VIDEO_MIN	300
#define B_PTS_DELAY_VIDEO_MAX	800

#define B_PTS_DELAY_AUDIO_MIN	300
#define B_PTS_DELAY_AUDIO_MAX	1200



typedef struct bplayback_rtsp *bplayback_rtsp_t;

struct brtp_codec {
	bplayback_rtsp_t decode;
	bpts_delay delay;
	bdecoder_info info;
	bool rtcp_sync;
	brtp_ntp ntp_base;
	uint32_t timestamp_base;
	uint32_t decoder_base;
	btimestamp decoder_clock;
	enum {brtp_sync_state_discovery, brtp_sync_state_tracking,  brtp_sync_state_throttling} sync_state;
	const char *name;
	int pts_delta_min;
	int pts_delta_max;
	uint32_t last_pts;
	unsigned no_data_cnt;
};

struct brtp_decode_source {
	struct bplayback_file self;
	bplayback_rtsp_t parent;
	brtp_source_t source;
	char url[256];
};

BDBG_OBJECT_ID(bplayback_rtsp_t);

/* the playback structure */
struct bplayback_rtsp {
	bplayback_iface iface; /* bplayback_iface shall be the first member in the structure */
	BDBG_OBJECT(bplayback_rtsp_t)
	bplaypump_t pump; /* set not NULL if not playing */
	bdecode_t decode;
	bstream_t stream;
	btimestamp decoder_clock;
	bool decoder_sync;
	uint32_t decoder_base;
	struct brtp_codec video;
	struct brtp_codec audio;
	brtp_play_t play;
	struct brtp_decode_source *source;
	bool use_tcp;
};

static const char  brtp_decode_source_id[] = "brtp_decode_source_id";
static void b_playback_rtsp_eos(void *cntx);

static void
b_rtsp_source_close(bplayback_file_t file)
{
	struct brtp_decode_source *source = (struct brtp_decode_source *)file;
	if (source->self.file_kind != brtp_decode_source_id) {
		BDBG_ERR(("b_rtsp_source_close: invalied file %#lx type", (unsigned long)file));
		return;
	}
	if (source->source) {
		/* XXX call brtp_play_disconnect */
		source->source = NULL;
	}
	BKNI_Free(source);
	return ;
}

bplayback_file_t
bplayback_rtsp_open_source(bplayback_t playback, const char *url)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	struct brtp_decode_source *source;
	BSTD_UNUSED(playback);
	BDBG_ASSERT(url);
	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);

	source = BKNI_Malloc(sizeof(*source));
	if (!source) {
		BDBG_ERR(("bplayback_rtsp_open_source: out of memory %d", sizeof(*source)));
		goto err_alloc;
	}
	bplayback_file_init(&source->self);
	BKNI_Snprintf(source->url, sizeof(source->url), "%s", url);
	source->self.file.close = b_rtsp_source_close;
	source->self.file_kind = brtp_decode_source_id;
	source->parent = p;
	source->source = brtp_play_connect(p->play, source->url);
	if (!source->source) {
		goto err_connect;
	}
	return &source->self;

err_connect:
	BKNI_Free(source);
err_alloc:
	return NULL;
}

/* this functions returns true if decoder attached to the stream */
static bool
b_playback_rtsp_check_decode(bplayback_rtsp_t p) 
{
	bstream_status stream_status;
	if (p->decode) {
		return true;
	}

	if (bstream_get_mpeg_parameters(p->stream, &stream_status)!=b_ok) {
		return false;
	}
	p->decode = stream_status.decode;
	return p->decode!=NULL;
}

static void
b_playback_rtsp_params_init(bplayback_params *params, bplayback_t playback)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	BSTD_UNUSED(playback);

	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);

	BKNI_Memset(params, 0, sizeof(*params));
	return;
}

static void 
b_rtp_play_data(void *stream_cntx, brtp_io_block *block, uint32_t pts)
{
	struct brtp_codec *codec  = stream_cntx;
	bresult rc;
	uint32_t clock;
	bplayback_rtsp_t decode = codec->decode;
	brtp_io_vec *vec;
	size_t buffer_size;
	void *buffer=NULL;
	unsigned copied;
	int delta = 0;

	BDBG_OBJECT_ASSERT(decode, bplayback_rtsp_t);

	BSTD_UNUSED(block);

	b_playback_rtsp_check_decode(decode);

	if (!decode->decoder_sync && decode->decode) {
		decode->decoder_sync = true;
		clock = ((B_FIRST_TIMESTAMP_DELAY*1000)-B_PTS_DELAY_NORM)*45;
		decode->decoder_base = clock;
		btimestamp_add(&decode->decoder_clock, clock);
		bdecode_set_stc(decode->decode, clock);
	}

	if (pts && decode->decode) {

		rc = bdecode_get_stc(decode->decode, &clock);
		if (rc==b_ok) {
			delta = brtp_delta_time_full(pts, clock);
			bpts_delay_add(&codec->delay, delta);
			if (codec->pts_delta_max==0 || delta>codec->pts_delta_max) {
				codec->pts_delta_max = delta;
			}
			if (codec->pts_delta_min==0 || delta<codec->pts_delta_min) {
				codec->pts_delta_min = delta;
			}
			codec->last_pts = pts;
		}
	}

	
	for(buffer_size=0, copied=0,vec = BLST_SQ_FIRST(block);vec;vec = BLST_SQ_NEXT(vec,list)) {
		unsigned consumed;
		unsigned to_copy;
		unsigned left;
		for(consumed=0;consumed<vec->len;) {
			if (buffer_size==copied) {
				if (copied>0) {
					if (bplaypump_read_complete(decode->pump, 0, copied)!=b_ok) {
						return;
					}
				}
				for(;;) {
					if (bplaypump_get_buffer(decode->pump, &buffer, &buffer_size)!=b_ok) {
						return;
					}
					if (buffer_size>0) {
						break;
					}
					BDBG_WRN(("overflow"));
					BKNI_Sleep(10);
				}
				copied=0;
			}
			BDBG_ASSERT(buffer);
			BDBG_ASSERT(vec->len>=consumed);
			BDBG_ASSERT(buffer_size>=copied);
			to_copy = vec->len - consumed;
			left = buffer_size-copied;
			if (to_copy>left) {
				to_copy = left;
			}
			BKNI_Memcpy((uint8_t *)buffer + copied, (uint8_t *)vec->data + consumed, to_copy);
			copied += to_copy;
			consumed += to_copy;
		}
	}
	if(copied) {
		if (bplaypump_read_complete(decode->pump, 0, copied)!=b_ok) {
			return;
		}
	}
	if(delta>0 && codec->sync_state == brtp_sync_state_throttling && delta>codec->info.max_delay){
		unsigned delay = (delta-codec->info.max_delay)/45;
		if (delay>100) {delay=100;}
		BDBG_MSG(("sleeping for %d msec", delay));
		BKNI_Sleep(delay);
	}
	return;
}

static void 
b_rtp_play_timer(void *cntx)
{
	bplayback_rtsp_t decode = cntx;
	int audio_step=0, video_step=0;
	struct bdelay_statistics stats;
	unsigned video_entries, audio_entries;
	uint32_t clock;
	bdecode_status status;

	BDBG_OBJECT_ASSERT(decode, bplayback_rtsp_t);

	if (!b_playback_rtsp_check_decode(decode)) {
		return;
	}
	bdecode_get_status(decode->decode,&status);

	audio_step = 0;
	video_step = 0;
	bpts_delay_run_stats(&decode->video.delay, &stats);
	video_entries = bdelay_statistics_entries(&stats); 
	if(video_entries>5) {
		BDBG_WRN(("%s:%d min:%d max:%d mean:%d deviation:%d decoder:%d:%d", "video", bdelay_statistics_entries(&stats), bdelay_statistics_min(&stats)/45, bdelay_statistics_max(&stats)/45, bdelay_statistics_mean(&stats)/45, bdelay_statistics_deviation(&stats)/45, brtp_delta_time_full(decode->video.last_pts, status.video_pts)/45, brtp_delta_time_full(status.video_stc, status.video_pts)/45 ));
		video_step = bdecoder_get_stc_step(&decode->video.info, &stats);
		decode->video.no_data_cnt = 0;
	} else {
		decode->video.no_data_cnt++;
	}
	bpts_delay_run_stats(&decode->audio.delay, &stats);
	audio_entries = bdelay_statistics_entries(&stats); 
	if(audio_entries>5) {
		BDBG_WRN(("%s:%d min:%d max:%d mean:%d deviation:%d decoder:%d:%d", "audio", bdelay_statistics_entries(&stats), bdelay_statistics_min(&stats)/45, bdelay_statistics_max(&stats)/45, bdelay_statistics_mean(&stats)/45, bdelay_statistics_deviation(&stats)/45, brtp_delta_time_full(decode->audio.last_pts, status.audio_pts)/45, brtp_delta_time_full(status.audio_stc, status.audio_pts)/45 ));
		audio_step = bdecoder_get_stc_step(&decode->audio.info, &stats);
		decode->audio.no_data_cnt = 0;
	} else {
		decode->audio.no_data_cnt ++;
	}
	if(video_step || audio_step) {
		int step = (video_step + audio_step)/2;

		if (decode->video.sync_state == brtp_sync_state_discovery && video_step > 0 && decode->use_tcp) {
			BDBG_WRN(("use throttled sync for video"));
			decode->video.sync_state = brtp_sync_state_throttling;
		}
		if (decode->audio.sync_state == brtp_sync_state_discovery && audio_step > 0 && decode->use_tcp) {
			BDBG_WRN(("use throttled sync for audio"));
			decode->audio.sync_state = brtp_sync_state_throttling;
		}

		if(decode->video.sync_state == brtp_sync_state_tracking && decode->audio.sync_state == brtp_sync_state_tracking) {
			bdecode_get_stc(decode->decode, &clock);
			BDBG_WRN(("correcting clock by %d (%u->%u)", step, clock, clock+step));
			bdecode_set_stc(decode->decode, clock+step);
		} 
	}
	if (decode->video.no_data_cnt>5 || decode->audio.no_data_cnt>5) {
		BDBG_WRN(("no_data video:%u audio:%u", decode->video.no_data_cnt, decode->audio.no_data_cnt));
	}

	return;
}

static void 
b_rtp_play_sync(void *stream_cntx, const brtp_parser_mux_stream_cfg *stream_cfg, const brtp_ntp *ntp, uint32_t rtcp_timestamp)
{
	struct brtp_codec *codec  = stream_cntx;
	uint32_t clock;
	unsigned ntp_diff, clock_diff;
	uint64_t decoder_diff;
	bplayback_rtsp_t decode = codec->decode;
	bdecode_status status;
	bplaypump_status pump_status;

	BSTD_UNUSED(stream_cfg);

	BDBG_OBJECT_ASSERT(decode, bplayback_rtsp_t);

	if (!decode->decoder_sync) {
		return;
	}
	if (!b_playback_rtsp_check_decode(decode)) {
		return;
	}

	bdecode_get_stc(decode->decode, &clock);
	btimestamp_add(&codec->decoder_clock, clock);

	if(!codec->rtcp_sync) {
		codec->rtcp_sync = true;
		codec->ntp_base = *ntp;
		codec->timestamp_base = rtcp_timestamp;
		codec->decoder_base = clock;
		codec->ntp_base = *ntp;
	}
	decoder_diff = btimestamp_get(&codec->decoder_clock) - codec->decoder_base;
	ntp_diff = brtp_ntp_diff(ntp, &codec->ntp_base);
	clock_diff = (unsigned)(decoder_diff/45);

	if (codec->sync_state == brtp_sync_state_discovery) {
		if (ntp_diff > 10000) {
			int delta = (100*brtp_delta_time_full(ntp_diff, clock_diff))/(int)ntp_diff; /* delta now in percent */


			if (delta > 5 || delta < -5) {
				if (codec->decode->use_tcp) {
					BDBG_WRN(("sync delta %d, use throttled sync for %s", delta, codec->name));
					codec->sync_state = brtp_sync_state_throttling;
				}
			} else {
				BDBG_WRN(("sync delta %d, use tracking sync for %s", delta, codec->name));
				codec->sync_state = brtp_sync_state_tracking;
			}
		}
	}

	bdecode_get_status(decode->decode,&status);
	bplaypump_get_status(decode->pump, &pump_status);
	BDBG_WRN(("rtcp:%s %u:%u:%d video: %ux%u%c %lu(%d) %lu (%d:%d) %lu:(%d:%d) audio: %lu(%d) %lu (%d:%d) %lu:(%d:%d) %lu", \
		codec->name, ntp_diff, clock_diff, brtp_delta_time_full(ntp_diff,clock_diff),\
		status.source_width, status.source_height,status.interlaced?'i':'p', status.video_pts, brtp_delta_time_full(decode->video.last_pts, status.video_pts)/45, status.video_stc, brtp_delta_time_full(status.video_pts,status.video_stc), brtp_delta_time_full(status.video_stc, clock), status.video_fifo_depth, decode->video.pts_delta_max/45, decode->video.pts_delta_min/45, \
		status.audio_pts, brtp_delta_time_full(decode->audio.last_pts, status.audio_pts)/45, status.audio_stc, brtp_delta_time_full(status.audio_pts,status.audio_stc), brtp_delta_time_full(status.audio_stc, clock), status.audio_fifo_depth, decode->audio.pts_delta_max/45, decode->audio.pts_delta_min/45, \
		pump_status.fifo_depth));

	codec->pts_delta_min = 0;
	codec->pts_delta_max = 0;
	codec->last_pts = 0;
	codec->no_data_cnt = 0;

	return;
}


static void
b_rtp_codec_init(struct brtp_codec *codec, struct brtp_play_stream_config *config, bplayback_rtsp_t decode,	const bdecoder_info *info, const char *name)
{
	BDBG_ASSERT(codec);
	BDBG_ASSERT(decode);
	BDBG_ASSERT(info);
	BDBG_ASSERT(name);

	bpts_delay_init(&codec->delay);
	btimestamp_init(&codec->decoder_clock);
	codec->rtcp_sync = false;
	codec->info = *info;
	codec->decode = decode;
	codec->name = name;
	codec->pts_delta_min = 0;
	codec->pts_delta_max  = 0;
	codec->sync_state = brtp_sync_state_discovery;

	config->stream_cntx = codec;
	config->data = b_rtp_play_data;
	config->sync = b_rtp_play_sync;

	return;
}

int
b_playback_rtsp_stop_rtp(bplayback_rtsp_t p)
{
	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	if(p->source->source) {
		return brtp_play_stop(p->play);
	} else {
		return 0;
	}
}

int
b_playback_rtsp_start_rtp(bplayback_rtsp_t p)
{
	int rc;
	brtp_play_config rtp_config;
	bdecoder_info video_info={B_PTS_DELAY_VIDEO_MIN*45, B_PTS_DELAY_VIDEO_MAX*45};
	bdecoder_info audio_info={B_PTS_DELAY_AUDIO_MIN*45, B_PTS_DELAY_AUDIO_MAX*45};

	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	BDBG_ASSERT(p->source);

	p->decoder_sync = false;
	p->use_tcp = true;
	btimestamp_init(&p->decoder_clock);
	brtp_play_default_cfg(p->play, &rtp_config);

	b_rtp_codec_init(&p->video, &rtp_config.video, p, &video_info, "video");
	b_rtp_codec_init(&p->audio, &rtp_config.audio, p, &audio_info, "audio");

	rtp_config.timer = b_rtp_play_timer;
	rtp_config.end_of_stream  = b_playback_rtsp_eos;
	rtp_config.play_cntx = p;
	rtp_config.use_tcp = p->use_tcp; 
	rc = brtp_play_start(p->play, p->source->source, &rtp_config);
	return rc;
}

static void
b_playback_rtsp_eos(void *cntx)
{
	bplayback_rtsp_t p= cntx;

	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	BDBG_ASSERT(p->source);
	BDBG_WRN(("end of stream"));
	b_playback_rtsp_stop_rtp(p);
	BDBG_WRN(("restarting"));
	p->source->source = brtp_play_connect(p->play, p->source->url);
	if (!p->source->source) {
		goto err_connect;
	}

	b_playback_rtsp_start_rtp(p);
err_connect:

	return;
}

static bstream_t 
b_playback_rtsp_start(bplayback_t playback, bplaypump_t playpump, const bstream_mpeg *mpeg_in, bplayback_file_t file, const bplayback_params *params)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	struct brtp_decode_source *source = (struct brtp_decode_source *)file;
	bstream_mpeg mpeg;
	bplaypump_params playpump_params;
	int rc;

	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	BDBG_ASSERT(source);
	BSTD_UNUSED(mpeg_in);
	BSTD_UNUSED(params);

	if (p->pump) {
		BDBG_ERR(("b_playback_rtsp_start: %lx already playing", (unsigned long)p));
		goto err_already_started;
	}
	if (source->self.file_kind != brtp_decode_source_id || source->source==NULL || source->parent != p) {
		BDBG_ERR(("b_playback_rtsp_start: invalied file %#lx type", (unsigned long)file));
		goto err_invalid_file;
	}
	p->source = source;
	bstream_mpeg_init(&mpeg);
	mpeg.video[0].pid = 0xE0;
	mpeg.video[0].format = bvideo_codec_h264;
	mpeg.audio[0].pid = 0xC0;
	mpeg.audio[0].format = baudio_format_aac;
	mpeg.mpeg_type = bstream_mpeg_type_pes;

	p->pump = playpump;
	bplaypump_params_init(&playpump_params, playpump);
	playpump_params.constant_rate = true;

	p->stream = bplaypump_start(playpump, &mpeg, &playpump_params);
	if (!p->stream) {
		goto err_pump_start;
	}
	p->decode = NULL;

	rc = b_playback_rtsp_start_rtp(p);
	if(rc!=0) {
		goto err_rtp_start;
	}

	return p->stream;
err_rtp_start:
	bplaypump_stop(p->pump);
	p->stream = NULL;
err_pump_start:
	p->play = NULL;
	p->source = NULL;
err_invalid_file:
err_already_started:
	return NULL;
}

static bresult 
b_playback_rtsp_stop(bplayback_t playback)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	if(!p->pump) {
		return berr_invalid_state;
	}
	b_playback_rtsp_stop_rtp(p);
	bplaypump_stop(p->pump);
	p->pump = NULL;
	p->stream = NULL;
	p->source = NULL;
	return b_ok;
}

static bresult 
b_playback_rtsp_get_status(bplayback_t playback, bplayback_status *status)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	BKNI_Memset(status, 0, sizeof(*status));
	return b_ok;
}

static void 
b_playback_rtsp_close(bplayback_t playback)
{
	bplayback_rtsp_t p = (bplayback_rtsp_t)playback;
	BDBG_OBJECT_ASSERT(p, bplayback_rtsp_t);
	if (p->pump) {
		b_playback_rtsp_stop(playback);
	}
	brtp_play_destroy(p->play);
	BDBG_OBJECT_DESTROY(p, bplayback_rtsp_t);
	BKNI_Free(p);
	return;
}


bplayback_t 
bplayback_rtsp_open(void)
{
	bplayback_rtsp_t p;

	p = BKNI_Malloc(sizeof(*p));
	if (!p) {
		BDBG_ERR(("bplayback_rtsp_ope: out of memory  %d", sizeof(*p)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(p, bplayback_rtsp_t);
	bplayback_iface_init(&p->iface);
	p->play = brtp_play_create();
	if (!p->play) {
		goto err_brtp_play;
	}
	p->iface.params_init = b_playback_rtsp_params_init;
	p->iface.start = b_playback_rtsp_start;
	p->iface.stop = b_playback_rtsp_stop;
	p->iface.get_status = b_playback_rtsp_get_status;
	p->iface.close = b_playback_rtsp_close;
	p->pump = NULL; /* not playing */
	p->source = NULL;
	p->stream = NULL;
	p->use_tcp = true; /* XXX make it an option */
	return &p->iface;

err_brtp_play:
	BKNI_Free(p);
err_alloc:
	return NULL;
}



