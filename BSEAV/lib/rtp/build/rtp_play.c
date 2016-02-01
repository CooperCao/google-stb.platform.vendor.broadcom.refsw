#include "bsettop.h"
#include "bstd.h"
#include "bkni.h"
#include "btimed_pkt.h"
#include "btimestamp.h"
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include "brtp_util.h"
#include "brtp_packet.h"
#include "bfifo.h"
#include "brtp_play.h"
#include "brtp_delay.h"
#include "bsettop_playback_rtsp.h"

BDBG_MODULE(rtp_play);

#define B_PTS_DELAY_NORM		500

#define B_PTS_DELAY_VIDEO_MIN	300
#define B_PTS_DELAY_VIDEO_MAX	800

#define B_PTS_DELAY_AUDIO_MIN	300
#define B_PTS_DELAY_AUDIO_MAX	1200



typedef struct play_state {
	FILE *fin;
	bplaypump_t pump;
	bdecode_t decode;
} play_state;

static int
play_read(play_state *play, size_t len)
{
	void *buffer;
	size_t buffer_size;
	size_t to_read;
	int rc;

	while(len>0) {
		if (bplaypump_get_buffer(play->pump, &buffer, &buffer_size)!=b_ok) {
			perror("bplaypump_get_buffer");
			goto error;
		}
		if(buffer_size==0) {
			BKNI_Sleep(10); /* sleep for 10 secs */
			continue;
		}
		if(buffer_size>len) {
			to_read = len;
		} else {
			to_read = buffer_size;
		}
		rc = fread(buffer, to_read, 1, play->fin);
		if (rc<0) {
			perror("fread");
			goto error;
		}
		if (bplaypump_read_complete(play->pump, 0, to_read)!=b_ok) {
			perror("bplaypump_read_complete");
			goto error;
		}
		len -= to_read;
	}
	return 0;
error:
	return -1;
}

static struct {
	uint32_t pts;
	int delta_min;
	int delta_max;
} last_pts[256]={{0,0,0}};

static int 
play_start(const char *file, bplaypump_t pump, bdecode_t decode)
{
	int rc;
	FILE *fin;
	struct b_pkt_header hdr;
	struct b_pkt_rtcp_sr rtcp_sr;
	unsigned len;
	play_state play;
	struct timeval 	start;
	struct timeval 	now;
	unsigned delta;
	uint32_t clock;
	btimestamp decoder_clock;
	uint32_t decoder_base=0;
	uint64_t decoder_diff;
	brtp_ntp ntp_base;
	brtp_ntp ntp;
	bool sync = false;
	unsigned ntp_diff, clock_diff;
	bdecode_status status;
	bplaypump_status pump_status;
	bool clock_sent=false;
	uint8_t pes_header[16];
	struct bdelay_statistics stats;
	bpts_delay delay_video, delay_audio;
	bdecoder_info video_info={B_PTS_DELAY_VIDEO_MIN*45, B_PTS_DELAY_VIDEO_MAX*45};
	bdecoder_info audio_info={B_PTS_DELAY_AUDIO_MIN*45, B_PTS_DELAY_AUDIO_MAX*45};
	int audio_step, video_step;
	unsigned last_sync = 0;


	bpts_delay_init(&delay_video);
	bpts_delay_init(&delay_audio);
	gettimeofday(&start, NULL);
	btimestamp_init(&decoder_clock);


	fin = fopen(file, "rb");
	if (!fin) {
		perror(file);
		return -1;
	}
	play.fin = fin;
	play.pump = pump;
	play.decode = decode;
	/*
	bdecode_get_clock(decode, &decoder_base);
	btimestamp_add(&decoder_clock, decoder_base);
	*/

	while(!feof(fin)) {
		rc = fread(&hdr, sizeof(hdr), 1, fin);
		if (rc!=1) {
			perror(file);
			goto error;
		}
		BDBG_ASSERT(hdr.len>=sizeof(hdr));
		gettimeofday(&now, NULL);
		delta = (now.tv_sec - start.tv_sec)*1000 + (now.tv_usec - start.tv_usec)/1000;
		if (hdr.offset > delta) {
			BKNI_Sleep(hdr.offset-delta);
		}
		len = hdr.len - sizeof(hdr);

		if (sync && delta > last_sync+1000) {
			last_sync = delta;
			audio_step = 0;
			video_step = 0;
			bpts_delay_run_stats(&delay_video, &stats);
			if(bdelay_statistics_entries(&stats)>10) {
				BDBG_WRN(("%s:%d min:%d max:%d mean:%d deviation:%d", "video", bdelay_statistics_entries(&stats), bdelay_statistics_min(&stats)/45, bdelay_statistics_max(&stats)/45, bdelay_statistics_mean(&stats)/45, bdelay_statistics_deviation(&stats)/45));
				video_step = bdecoder_get_stc_step(&video_info, &stats);
			}
			bpts_delay_run_stats(&delay_audio, &stats);
			if(bdelay_statistics_entries(&stats)>10) {
				BDBG_WRN(("%s:%d min:%d max:%d mean:%d deviation:%d", "audio", bdelay_statistics_entries(&stats), bdelay_statistics_min(&stats)/45, bdelay_statistics_max(&stats)/45, bdelay_statistics_mean(&stats)/45, bdelay_statistics_deviation(&stats)/45));
				audio_step = bdecoder_get_stc_step(&audio_info, &stats);
			}
			if(video_step || audio_step) {
				int step = (video_step + audio_step)/2;
				bdecode_get_stc(decode, &clock);
				BDBG_WRN(("correcting clock by %d (%u->%u)", step, clock, clock+step));
				bdecode_set_stc(decode, clock+step);
			}
		}
		BDBG_MSG(("packet(%d):%d", hdr.type, len));
		switch(hdr.type) {
		case b_pkt_type_rtcp_sr:
			rc = fread(&rtcp_sr, sizeof(rtcp_sr), 1, fin);
			if (rc!=1) {
				perror(file);
				goto error;
			}
			if (!sync) {
				sync = true;
				last_sync = delta;
				bdecode_get_stc(decode, &clock);
				decoder_base = clock;
				btimestamp_add(&decoder_clock, clock);
				brtp_ntp_init(&ntp_base, rtcp_sr.ntp_msw, rtcp_sr.ntp_lsw);
			} else {
				bdecode_get_stc(decode, &clock);
				btimestamp_add(&decoder_clock, clock);
			}
			brtp_ntp_init(&ntp, rtcp_sr.ntp_msw, rtcp_sr.ntp_lsw);
			decoder_diff = btimestamp_get(&decoder_clock) - decoder_base;
			bdecode_get_status(decode,&status);
			bplaypump_get_status(pump, &pump_status);
			ntp_diff = brtp_ntp_diff(&ntp, &ntp_base);
			/* bdecode_set_stc(decode, ntp_diff*45); */
			clock_diff = (unsigned)(decoder_diff/45);
			BDBG_WRN(("%u:%u:%d video: %ux%u%c %lu %lu (%d:%d) %lu:(%d:%d) audio: %lu %lu (%d:%d) %lu:(%d:%d) %lu", \
					ntp_diff, clock_diff, brtp_delta_time_full(ntp_diff,clock_diff),\
					status.source_width, status.source_height,status.interlaced?'i':'p', status.video_pts, status.video_stc, brtp_delta_time_full(status.video_pts,status.video_stc), brtp_delta_time_full(status.video_stc, clock), status.video_fifo_depth, last_pts[0xE0].delta_max/45, last_pts[0xE0].delta_min/45, \
					status.audio_pts, status.audio_stc, brtp_delta_time_full(status.audio_pts,status.audio_stc), brtp_delta_time_full(status.audio_stc, clock), status.audio_fifo_depth, last_pts[0xC0].delta_max/45, last_pts[0xC0].delta_min/45, \
					pump_status.fifo_depth));


			last_pts[0xC0].delta_min = 0;
			last_pts[0xC0].delta_max = 0;
			last_pts[0xE0].delta_min = 0;
			last_pts[0xE0].delta_max = 0;
			break;
		case b_pkt_type_data:
			if (!clock_sent) {
				clock_sent = true;
				clock = ((B_FIRST_TIMESTAMP_DELAY*1000)-B_PTS_DELAY_NORM)*45;
				decoder_base = clock;
				bdecode_set_stc(decode, clock);
			}
			fread(pes_header, sizeof(pes_header), 1, fin);
			fseek(fin, -sizeof(pes_header), SEEK_CUR);
			if(B_GET_BITS(pes_header[7], 7, 6)&2) {
				uint32_t pts_32_30 = B_GET_BITS(pes_header[9], 3, 1);
				uint32_t pts_29_22 = pes_header[10];
				uint32_t pts_21_15 = B_GET_BITS(pes_header[11], 7, 1);
				uint32_t pts_14_7 = pes_header[12];
				uint32_t pts_6_0 = B_GET_BITS(pes_header[13], 7, 2);
				uint32_t pts;
				uint8_t id = pes_header[3];
				pts = ((uint32_t)pts_32_30<<29) | (pts_29_22<<21) | (pts_21_15<<14) | (pts_14_7<<6)|pts_6_0;
				if (1||id==0xc0) {
					int delta;
					bdecode_get_stc(decode, &clock);
					delta = brtp_delta_time_full(pts,clock);
					BDBG_MSG(("PTS: id %02x %u %d (%d)", id, pts, brtp_delta_time_full(pts,last_pts[id].pts)/45, delta));
					last_pts[id].pts = pts;
					if (last_pts[id].delta_min == 0 || delta<last_pts[id].delta_min) {
						last_pts[id].delta_min = delta;
					}
					if (last_pts[id].delta_max == 0 || delta>last_pts[id].delta_max) {
						last_pts[id].delta_max = delta;
					}
					if(id==0xe0) {
						bpts_delay_add(&delay_video, delta);
					} else {
						bpts_delay_add(&delay_audio, delta);
					}
				}
			}
			play_read(&play, len);
			break;
		default:
			fseek(fin, len, SEEK_CUR);
			break;
		}
	}
error:
	fclose(fin);
	return -1;
}

struct brtp_decode;
static int rtp_start_play(struct brtp_decode *rtp_decode);

struct brtp_codec {
	struct brtp_decode *decode;
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

BDBG_OBJECT_ID(brtp_decode);

struct brtp_decode {
	BDBG_OBJECT(brtp_decode)
	bplaypump_t pump;
	bdecode_t decode;
	btimestamp decoder_clock;
	bool decoder_sync;
	uint32_t decoder_base;
	struct brtp_codec video;
	struct brtp_codec audio;
	brtp_play_t play;
	bool use_tcp;
	char url[256];
};

static void 
b_rtp_play_data(void *stream_cntx, brtp_io_block *block, uint32_t pts)
{
	struct brtp_codec *codec  = stream_cntx;
	bresult rc;
	uint32_t clock;
	struct brtp_decode *decode = codec->decode;
	brtp_io_vec *vec;
	size_t buffer_size;
	void *buffer=NULL;
	unsigned copied;
	int delta = 0;

	BSTD_UNUSED(block);

	if (!decode->decoder_sync) {
		decode->decoder_sync = true;
		clock = ((B_FIRST_TIMESTAMP_DELAY*1000)-B_PTS_DELAY_NORM)*45;
		decode->decoder_base = clock;
		btimestamp_add(&decode->decoder_clock, clock);
		bdecode_set_stc(decode->decode, clock);
	}

	if (pts) {

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
	struct brtp_decode *decode = cntx;
	int audio_step=0, video_step=0;
	struct bdelay_statistics stats;
	unsigned video_entries, audio_entries;
	uint32_t clock;
	bdecode_status status;

	BDBG_OBJECT_ASSERT(decode, brtp_decode);

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
	struct brtp_decode *decode = codec->decode;
	bdecode_status status;
	bplaypump_status pump_status;

	BSTD_UNUSED(stream_cfg);

	if (!decode->decoder_sync) {
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
b_rtp_codec_init(struct brtp_codec *codec, struct brtp_play_stream_config *config, struct brtp_decode *decode,	const bdecoder_info *info, const char *name)
	
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

static int
rtp_stop_play(struct brtp_decode *decode)
{
	BDBG_OBJECT_ASSERT(decode, brtp_decode);
	return brtp_play_stop(decode->play);
}

static void
b_rtp_play_end_of_stream(void *cntx)
{
	struct brtp_decode *decode = cntx;

	BDBG_OBJECT_ASSERT(decode, brtp_decode);
	BDBG_WRN(("end of stream"));
	rtp_stop_play(decode);
	BDBG_WRN(("restarting"));
	rtp_start_play(decode);
	return;
}

static int
rtp_start_play(struct brtp_decode *rtp_decode)
{
	brtp_play_config config;
	brtp_source_t source;

	bdecoder_info video_info={B_PTS_DELAY_VIDEO_MIN*45, B_PTS_DELAY_VIDEO_MAX*45};
	bdecoder_info audio_info={B_PTS_DELAY_AUDIO_MIN*45, B_PTS_DELAY_AUDIO_MAX*45};

	BDBG_OBJECT_ASSERT(rtp_decode, brtp_decode);

	rtp_decode->decoder_sync = false;
	rtp_decode->use_tcp = true;
	btimestamp_init(&rtp_decode->decoder_clock);
	brtp_play_default_cfg(rtp_decode->play, &config);

	b_rtp_codec_init(&rtp_decode->video, &config.video, rtp_decode, &video_info, "video");
	b_rtp_codec_init(&rtp_decode->audio, &config.audio, rtp_decode, &audio_info, "audio");

	config.timer = b_rtp_play_timer;
	config.end_of_stream  = b_rtp_play_end_of_stream;
	config.play_cntx = rtp_decode;
	config.use_tcp = rtp_decode->use_tcp; 

	source = brtp_play_connect(rtp_decode->play, rtp_decode->url);
	if(!source) {
		return -1;
	}

	return brtp_play_start(rtp_decode->play, source, &config);
}

static int 
rtp_start(const char *url, bplaypump_t pump, bdecode_t decode)
{
	struct brtp_decode rtp_decode_;
	struct brtp_decode *rtp_decode = &rtp_decode_;
	int rc;

	BDBG_ASSERT(url);

	BKNI_Memset(rtp_decode, 0, sizeof(*rtp_decode));
	BDBG_OBJECT_SET(rtp_decode, brtp_decode);
	rtp_decode->pump = pump;
	rtp_decode->decode = decode;
	rtp_decode->play = brtp_play_create();
	BKNI_Snprintf(rtp_decode->url, sizeof(rtp_decode->url), "%s", url);
	rc = rtp_start_play(rtp_decode);
	getchar();
	BDBG_WRN(("stopping..."));
	rtp_stop_play(rtp_decode);
	BDBG_WRN(("destroyyng..."));
	brtp_play_destroy(rtp_decode->play);
	BDBG_WRN(("done"));
	return rc;
}


int main(int argc, const char *argv[])
{
	int rc;
	const char *file=argv[1];

	BSTD_UNUSED(argc);
	rc = bsettop_init(BSETTOP_VERSION); 
	if (rc!=b_ok) {return 1;}
	{
	bdecode_t decode = bdecode_open(B_ID(0));
	bdisplay_t display = bdisplay_open(B_ID(0));
	bdecode_window_t window = bdecode_window_open(B_ID(0), display);
	bplaypump_t playpump = bplaypump_open(B_ID(0), NULL);
	bstream_mpeg mpeg;
	bstream_t stream;
	bplaypump_params playpump_params;

	bstream_mpeg_init(&mpeg);
	mpeg.video[0].pid = 0xE0;
	mpeg.video[0].format = bvideo_codec_h264;
	mpeg.audio[0].pid = 0xC0;
	mpeg.audio[0].format = baudio_format_aac;
	mpeg.mpeg_type = bstream_mpeg_type_pes;

	if (argv[1]==NULL) {
		fprintf(stderr,"file or RTSP url expected\n");
		goto error;
	}
	if (file[0]=='r' && file[1]=='t' && file[2]=='s' && file[3]=='p' && file[4]==':') {
#if 0
		bplaypump_params_init(&playpump_params, playpump);
		playpump_params.constant_rate = true; 
		/* mpeg.pcr_pid = 0x1FFF; */
		stream = bplaypump_start(playpump, &mpeg, &playpump_params);
		if (!stream) {goto error;}

		if (bdecode_start(decode, stream, window)!=b_ok) {goto error; }

		rtp_start(file, playpump, decode);
#else
		bplayback_t  playback = bplayback_rtsp_open();
		bplayback_file_t source;
		bplayback_params playback_params;
		BDBG_ASSERT(playback);

		source = bplayback_rtsp_open_source(playback, file);
		if (!source) { goto error; }
		bplayback_params_init(&playback_params, playback);
		stream = bplayback_start(playback, playpump, &mpeg, source, &playback_params);
		if (!stream) { goto error; }
		if (bdecode_start(decode, stream, window)!=b_ok) {goto error; }
		getchar(); /* press enter to stop playback */

		bdecode_stop(decode);
		bplayback_stop(playback);
		bplayback_file_close(source);
#endif
	} else {
		bplaypump_params_init(&playpump_params, playpump);
		playpump_params.constant_rate = true; 
		/* mpeg.pcr_pid = 0x1FFF; */
		stream = bplaypump_start(playpump, &mpeg, &playpump_params);
		if (!stream) {goto error;}

		if (bdecode_start(decode, stream, window)!=b_ok) {goto error; }
		play_start(file, playpump, decode);
	}

	}
error:
	bsettop_uninit();
	return 0;
}
