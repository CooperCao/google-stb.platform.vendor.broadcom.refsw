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

#include "btimed_pkt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>


BDBG_MODULE(brtp_test);

typedef struct {
  struct timeval start;  /* start of recording (GMT) */
  uint32_t source;        /* network source (multicast address) */
  uint16_t port;          /* UDP port */
} RD_hdr_t;

typedef struct {
  uint16_t length;    /* length of packet, including this header (may 
                        be smaller than plen if not whole packet recorded) */
  uint16_t plen;      /* actual header+payload length for RTP, 0 for RTCP */
  uint32_t offset;    /* milliseconds since the start of recording */
} RD_packet_t;

static void 
b_pkt_free(void *source_cntx, void *pkt)
{
	BSTD_UNUSED(source_cntx);
	BKNI_Free(pkt);
	return ;
}

struct b_rtp_test {
	struct timeval start;
	bool timed_pkt;
	bool timed_rtp;
	brtp_parser_mux_t mux;
	FILE *fout;
};


static int
b_timed_pkt_write_header(struct b_rtp_test *test, b_pkt_type type, unsigned len)
{
	struct b_pkt_header hdr;
	struct timeval 	now;

	gettimeofday(&now, NULL);
	hdr.type = type;
	hdr.offset = (now.tv_sec - test->start.tv_sec)*1000;
	if (now.tv_usec < test->start.tv_usec) {
		hdr.offset = (hdr.offset - 1000) + (now.tv_usec + 1000000 - test->start.tv_usec)/1000;
	} else {
		hdr.offset = hdr.offset + (now.tv_usec - test->start.tv_usec)/1000;
	}
	hdr.len = len + sizeof(hdr);
	hdr.reserved = 0;
	fwrite(&hdr, sizeof(hdr), 1, test->fout);
	return 0;
}

static void 
b_rtp_save_payload(void *cntx, brtp_io_block *block, brtp_parser_t parser, uint32_t pts)
{
	brtp_io_vec *vec;
	struct b_rtp_test  *test = cntx;
	FILE *fout=test->fout;
	unsigned len;

	BSTD_UNUSED(parser);
	BSTD_UNUSED(pts);

	BDBG_ASSERT(test->fout);
	BDBG_ASSERT(test->mux);

	if (test->timed_pkt) {
		for(len=0,vec = BLST_SQ_FIRST(block);vec;vec = BLST_SQ_NEXT(vec,list)) {
			len += vec->len;
		}
		b_timed_pkt_write_header(test, b_pkt_type_data, len);
	} else if (test->timed_rtp) {
		goto done; /* don't write PES data */
	}
			  
	for(vec = BLST_SQ_FIRST(block);vec;vec = BLST_SQ_NEXT(vec,list)) {
		BDBG_MSG(("ES: %d %#lx", vec->len, vec->data));
		fwrite(vec->data, 1, vec->len, fout);
	}
done:
	brtp_parser_mux_recycle(test->mux, block);
	return;
}

struct brtp_channel {
	struct b_rtp_test *test;
	brtp_parser_mux_t mux; 
	brtp_t rtp;
	brtp_parser_t parser;
	brtp_parser_mux_stream_cfg stream_cfg;
	brtp_parser_mux_stream_t stream;
	lm_stream_t lm_stream;
	brtp_ntp sr_ntp;
	uint32_t sr_timestamp;
	uint32_t rtp_timestamp;
	struct {
		bool rtcp_sr:1;
		bool rtp_ts:1;
	} sync;
	uint8_t rtp_format;
	int sock_in;
	FILE *fin;
	uint8_t cfg[256];
	uint8_t cfg_len;
};

#define B_RTP_N_CHANNELS 2
static struct brtp_channel channels[B_RTP_N_CHANNELS];

struct b_rtp_feeder_file  {
	FILE *fin;
	RD_hdr_t hdr;
	RD_packet_t packet;
	struct timeval tv;
};

static bool
b_rtp_file_read_hdr(struct b_rtp_feeder_file *feeder)
{
	int rc;

	if (!feeder->fin) {
		return false;
	}
	rc = fread(&feeder->packet, sizeof(feeder->packet), 1, feeder->fin);
	if (rc==0) {
		feeder->fin = NULL;
		return false;
	} else if(rc!=1) {
		perror("");exit(1); 
	}
	feeder->packet.length = ntohs(feeder->packet.length);
	feeder->packet.plen = ntohs(feeder->packet.plen);
	feeder->packet.offset = ntohl(feeder->packet.offset);
	feeder->tv.tv_sec = feeder->hdr.start.tv_sec + feeder->packet.offset/1000;
	feeder->tv.tv_usec = feeder->hdr.start.tv_usec + (feeder->packet.offset%1000)*1000;
	if (feeder->tv.tv_usec > 1000000) {
		BDBG_MSG(("%u %u:%u", feeder->packet.offset, feeder->tv.tv_sec, feeder->tv.tv_usec));
		feeder->tv.tv_sec ++;
		feeder->tv.tv_usec -= 1000000;
		BDBG_MSG(("%u %u:%u", feeder->packet.offset, feeder->tv.tv_sec, feeder->tv.tv_usec));
		BDBG_ASSERT(feeder->tv.tv_usec < 1000000);
	}
	return true;
}


static int 
b_rtp_file_feed(const struct brtp_channel *channels) 
{
	int rc;
	char line[256];
	struct b_rtp_feeder_file rd[B_RTP_N_CHANNELS];
	unsigned chn_no;
	unsigned nactive;
	brtp_enqueue state;

	for(nactive=0,chn_no=0;chn_no<B_RTP_N_CHANNELS;chn_no++) {
		rd[chn_no].fin = NULL;
		if(!channels[chn_no].parser) {
			continue;
		}
		BDBG_MSG(("open %d", chn_no));
		BDBG_ASSERT(channels[chn_no].fin);
		fgets(line, sizeof(line)-1, channels[chn_no].fin);
		rc = fread(&rd[chn_no].hdr, sizeof(rd[chn_no].hdr), 1, channels[chn_no].fin);

		if(rc!=1) { perror("");exit(1); }
		rd[chn_no].hdr.start.tv_sec = ntohl(rd[chn_no].hdr.start.tv_sec);
		rd[chn_no].hdr.start.tv_usec = ntohl(rd[chn_no].hdr.start.tv_usec);
		rd[chn_no].fin = channels[chn_no].fin;
		nactive++;
		if (!b_rtp_file_read_hdr(&rd[chn_no])) {
			nactive--;
		}
	}
	BDBG_MSG(("sources: %d", nactive));
	while(nactive>0) {
		void *buf;
		struct timeval tv_min;
		unsigned chn_min;

		tv_min.tv_sec=0;
		tv_min.tv_usec=0;
		chn_min=(unsigned)-1;

		for(chn_no=0;chn_no<B_RTP_N_CHANNELS;chn_no++) {
			if(!rd[chn_no].fin) {
				continue;
			}
			BDBG_MSG((":%u->%u:%u", chn_no, (unsigned)rd[chn_no].tv.tv_sec, (unsigned)rd[chn_no].tv.tv_usec));
			if (   (chn_min==(unsigned)-1)
				|| (rd[chn_no].tv.tv_sec < tv_min.tv_sec) 
				|| ((rd[chn_no].tv.tv_sec == tv_min.tv_sec) && (rd[chn_no].tv.tv_usec < tv_min.tv_usec))) {
				chn_min = chn_no;
				tv_min = rd[chn_no].tv;
			}
		}
		chn_no = chn_min;
		BDBG_ASSERT(chn_no!=(unsigned)-1);
		BDBG_MSG(("%u->%u", chn_no, rd[chn_no].packet.offset));
		/* printf("%d:%d\n", packet.plen, packet.offset); */
		buf = BKNI_Malloc(rd[chn_no].packet.plen);
		/* BDBG_ERR(("%p %p", buf, (uint8_t *)buf+packet.plen)); */
		BDBG_ASSERT(buf);
		rc = fread(buf, 1, rd[chn_no].packet.plen, rd[chn_no].fin);
		if (rc!=rd[chn_no].packet.plen) { perror("");exit(1);}
		/* printf("\t\t%02x\n", ((uint8_t *)buf)[12]); */
		fflush(stdout);
		state = brtp_enqueue_pkt(channels[chn_no].rtp, buf, rc, buf);
		if (state!=brtp_enqueue_queued) {
			BKNI_Free(buf);
		}
		if (!b_rtp_file_read_hdr(&rd[chn_no])) {
			nactive--;
		}
	}
	return 0;
}

static int 
b_rtp_socket_feed(const struct brtp_channel *channels) 
{
	fd_set fds;
	struct timeval 	timeout;
	uint8_t sock_buf[65536];
	void *buf;
	int sock_max = -1;
	unsigned chn_no;
	int rc;
	brtp_enqueue state;

		
	for(chn_no=0;chn_no<B_RTP_N_CHANNELS;chn_no++) {
		if(!channels[chn_no].parser) {
			continue;
		}
		BDBG_ASSERT(channels[chn_no].sock_in>=0);
		if(channels[chn_no].sock_in > sock_max) {
			sock_max = channels[chn_no].sock_in;
		}
	}

	for(;;) {
		FD_ZERO(&fds);
		for(chn_no=0;chn_no<B_RTP_N_CHANNELS;chn_no++) {
			if(!channels[chn_no].parser) {
				continue;
			}
			FD_SET(channels[chn_no].sock_in, &fds);
		}
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		rc = select(sock_max+1, &fds, NULL, NULL, &timeout);
		if(rc==0) {
			fprintf(stderr, "no more data....\n");
			break;
		}
		for(chn_no=0;chn_no<B_RTP_N_CHANNELS;chn_no++) {
			if(!channels[chn_no].parser) {
				continue;
			}
			if(!FD_ISSET(channels[chn_no].sock_in, &fds)) {
				continue;
			}
			BDBG_MSG(("reading from: %u(%d)...", chn_no, channels[chn_no].sock_in));
			rc = read(channels[chn_no].sock_in, sock_buf, sizeof(sock_buf));
			BDBG_MSG(("reading from: %u(%d) -> %d", chn_no, channels[chn_no].sock_in, rc));
			if (rc<0) {
				perror("socket");
				continue;
			} else if (rc==0) { /* no data */
				continue;
			}
			buf = BKNI_Malloc(rc);
			BDBG_ASSERT(buf);
			memcpy(buf, sock_buf,rc);
			state = brtp_enqueue_pkt(channels[chn_no].rtp, buf, rc, buf);
			if (state!=brtp_enqueue_queued) {
				BKNI_Free(buf);
			}
		}
	}
	return 0;
}

static void
b_rtp_start_parser(struct brtp_channel *chn, uint32_t timestamp_offset)
{
	brtp_session_cfg session_cfg;

	BDBG_ASSERT(chn->mux);
	BDBG_ASSERT(chn->rtp);
	BDBG_ASSERT(chn->parser);
	BDBG_ASSERT(chn->test);
	BDBG_ASSERT(chn->stream==NULL);

	BDBG_WRN(("b_rtp_sync_one: starting stream %#lx[0x%02x] offset %u", (unsigned long)chn, chn->stream_cfg.pes_id, timestamp_offset));

	brtp_default_session_cfg(chn->rtp, &session_cfg);
	session_cfg.pt_mask = 0xFF;
	session_cfg.pt_value = chn->rtp_format;
	brtp_start(chn->rtp, chn->parser, &session_cfg);

	chn->stream_cfg.clock_offset = timestamp_offset;

	chn->stream = chn->parser->start(chn->parser, chn->mux, &chn->stream_cfg, &chn->cfg, chn->cfg_len);
	BDBG_ASSERT(chn->stream);
	return;
}

static void
b_rtp_timestamp_pkt(void *cntx, const void *rtp_pkt)
{
	struct brtp_channel *chn = cntx;
	uint32_t timestamp_offset;
	
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

static void
b_rtp_sync_one(void *cntx, uint32_t ntp_msw, uint32_t ntp_lsw, uint32_t timestamp_offset)
{
	struct brtp_channel *chn = cntx;
	struct brtp_channel *video=NULL;
	struct brtp_channel *audio=NULL;
	int ntp_delta;
	int ts_delta;
	uint32_t rtp_ts;

	BDBG_ASSERT(chn->mux);
	BDBG_ASSERT(chn->rtp);
	BDBG_ASSERT(chn->parser);
	BDBG_ASSERT(chn->test);
	if (chn->test->timed_pkt || chn->test->timed_rtp) {
		struct b_pkt_rtcp_sr sr;
		sr.ntp_msw = ntp_msw;
		sr.ntp_lsw = ntp_lsw;
		sr.timestamp = timestamp_offset;
		sr.clock_rate = chn->stream_cfg.clock_rate;
		b_timed_pkt_write_header(chn->test, b_pkt_type_rtcp_sr, sizeof(sr));
		fwrite(&sr, sizeof(sr), 1, chn->test->fout);
		fflush(chn->test->fout);
	}
	brtp_ntp_init(&chn->sr_ntp, ntp_msw, ntp_lsw);
	chn->sr_timestamp = timestamp_offset;
	chn->sync.rtcp_sr = true;
	audio = &channels[blm_media_audio]; 
	video=&channels[blm_media_video]; 
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
	ts_delta = (brtp_delta_time(video->rtp_timestamp, video->sr_timestamp)*125)/((int)video->stream_cfg.clock_rate/16);
	rtp_ts = ((ntp_delta+ts_delta)*((int)audio->stream_cfg.clock_rate/100))/10; /* calculate rtp_ts that shall bring audio back to 0 */
	timestamp_offset = audio->sr_timestamp + rtp_ts  - (uint32_t)(audio->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY) ;
	BDBG_WRN(("delta:(%d) ntp:%d ts:%d, rtp_ts=%u(%u) timestamp_offset=%u (audio ts:%d)", ntp_delta+ts_delta, ntp_delta, ts_delta, rtp_ts, audio->sr_timestamp, timestamp_offset, (brtp_delta_time(audio->rtp_timestamp, audio->sr_timestamp)*125)/((int)audio->stream_cfg.clock_rate/16)));
	b_rtp_start_parser(audio, timestamp_offset);
	return;
}


static brtp_enqueue 
b_rtp_data(void *rtp, void *pkt, size_t len, void *cookie)
{
	struct brtp_channel *chn = rtp;
	brtp_enqueue status;
	brtp_pkt_info pkt_info[8];
	int ninfo;
	
	if (chn->test->timed_rtp) {
		b_timed_pkt_write_header(chn->test, b_pkt_type_rtp_data, len);
		fwrite(pkt, len, 1, chn->test->fout);
	}
	status = brtp_enqueue_pkt(chn->rtp, pkt, len, cookie);

	if(chn->lm_stream) {
		ninfo = brtp_get_pkt_info(chn->rtp, pkt_info, sizeof(pkt_info)/sizeof(*pkt_info));
		if (ninfo>0) {
			lm_stream_update_pkt_info(chn->lm_stream, pkt_info, ninfo);
		}
	}
	return status;
}

static int
b_rtp_connect_one(lm_session_t session, brtp_parser_mux_t mux, blm_media media, struct brtp_channel *channels)
{
	const lm_stream_desc *desc;
	struct brtp_channel *chn = &channels[media];
	brtp_parser_h264_cfg h264_cfg;
	brtp_parser_mpeg4_cfg mpeg4_cfg;
	brtp_cfg cfg;
	lm_stream_connect_cfg connect_cfg;

	desc = lm_get_stream_desc(session, media);
	BDBG_ASSERT(desc);
	if (chn->test->timed_rtp) {
		b_timed_pkt_write_header(chn->test, b_pkt_type_rtsp_stream_desc, sizeof(*desc));
		fwrite(desc, sizeof(*desc), 1, chn->test->fout);
	}
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
	cfg.pkt_free = b_pkt_free;
	chn->mux = mux;
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
	connect_cfg.stream_cntx = chn;
	chn->lm_stream = lm_session_stream(session, media, &connect_cfg);
	BDBG_ASSERT(chn->lm_stream);
	return 0;
}

static void
b_rtp_play_url(brtp_parser_mux_t mux, struct brtp_channel *channels, const char *url)
{
	lm_state_t lm;
	lm_session_t session;

	lm = lm_init();
	session = lm_connect(lm, url);
	BDBG_ASSERT(session);

	b_rtp_connect_one(session, mux, blm_media_video, channels);
	b_rtp_connect_one(session, mux, blm_media_audio, channels);

	lm_session_play(session);
	lm_run(lm, NULL); /* doesn't return */
	lm_session_close(session);
	lm_shutdown(lm);

	exit(0);
}


struct b_time_state {
	struct b_pkt_rtcp_sr base;
	btimestamp ts_sr;
	uint32_t offset;
};

static uint32_t last_pts[256]={0};

static struct {
	uint32_t timestamp;
	uint32_t first_timestamp;
	uint32_t clock_rate;
} stream_data[256] = {{0,0,1}};

static void
do_time(const char *file)
{
	FILE *fin;
	struct b_pkt_header hdr;
	struct b_pkt_rtcp_sr rtcp_sr;
	int rc;
	unsigned len;
	struct b_time_state state[2];
	struct b_pkt_rtcp_sr *begin;
	unsigned chn;
	unsigned i;
	uint32_t ntp_msw, ntp_lsw;
	uint32_t ntp_msec;
	uint32_t ts_diff;
	uint32_t ntp_ts_diff;
	uint8_t pes_header[32];
	uint8_t rtp_header[32];
	struct lm_stream_desc desc;
	uint8_t id;


	for(i=0; i<sizeof(state)/sizeof(*state);i++) {
		state[i].base.clock_rate = 0;
		btimestamp_init(&state[i].ts_sr);
	}

	fin = fopen(file,"rb");
	if (!fin) {
		perror(file);
		exit(1);
	}
	while(!feof(fin)) {
		rc = fread(&hdr, sizeof(hdr), 1, fin);
		if (rc!=1) {
			perror(file);
			goto error;
		}
		BDBG_MSG(("%u %u %u", hdr.type, hdr.len, hdr.offset));
		BDBG_ASSERT(hdr.len>=sizeof(hdr));
		len = hdr.len - sizeof(hdr);
		switch(hdr.type) {
		case b_pkt_type_rtcp_sr:
			BDBG_ASSERT(len=sizeof(rtcp_sr));
			rc = fread(&rtcp_sr, sizeof(rtcp_sr), 1, fin);
			if (rc!=1) {
				perror(file);
				goto error;
			}
			for(begin=NULL,i=0; i<sizeof(state)/sizeof(*state);i++) {
				if (state[i].base.clock_rate==rtcp_sr.clock_rate) {
					begin = &state[i].base;
					break;
				} else if (state[i].base.clock_rate==0) {
					begin = &state[i].base;
					if(i==0) {
						*begin = rtcp_sr;
						for(chn=i; chn<sizeof(state)/sizeof(*state);chn++) {
							state[chn].base.ntp_lsw = rtcp_sr.ntp_lsw;
							state[chn].base.ntp_msw = rtcp_sr.ntp_msw;
							state[chn].offset = hdr.offset;
						}
					} else {
						begin->clock_rate = rtcp_sr.clock_rate;
						begin->timestamp = rtcp_sr.timestamp - (hdr.offset - state[0].offset)*(rtcp_sr.clock_rate/1000);
					}
					break;
				}
			}
			if(rtcp_sr.clock_rate==90000) {
				/* id = 0xE0; */
				id = 96;
			} else {
				/* id = 0xC0; */
				id = 97;
			}
			if (stream_data[id].first_timestamp==0) {
				stream_data[id].first_timestamp = stream_data[id].timestamp;
			}
			BDBG_ASSERT(begin);
			chn = i;
			BDBG_MSG(("ntp: %u:%u timestamp:%u rate:%u", rtcp_sr.ntp_msw, rtcp_sr.ntp_lsw, rtcp_sr.timestamp, rtcp_sr.clock_rate));
			ntp_msw = rtcp_sr.ntp_msw-begin->ntp_msw;
			if (rtcp_sr.ntp_lsw<begin->ntp_lsw) {
				ntp_msw--;
			}
			ntp_lsw = rtcp_sr.ntp_lsw - begin->ntp_lsw;
			/* ntp_msec = (uint32_t) (((uint64_t)ntp_lsw) * 1000)/(((uint64_t)1)<<32); */
			ntp_msec = ((ntp_lsw/((1<<10)/8))*(1000/8))/(1<<22); /* == (ntp_lsw * 1000)/(1<<32) */
			/* BDBG_ERR(("%d", ntp_msec)); */
			ntp_msec += ntp_msw*1000;
			btimestamp_add(&state[chn].ts_sr, rtcp_sr.timestamp);
			ts_diff = ((uint64_t)(btimestamp_get(&state[chn].ts_sr)-begin->timestamp)*1000)/rtcp_sr.clock_rate;
			ntp_ts_diff = (unsigned)((brtp_delta_time_full(stream_data[id].timestamp, stream_data[id].first_timestamp)*(int64_t)1000)/rtcp_sr.clock_rate);
			BDBG_WRN(("%u ntp: %u timestamp:%u rtp:%u rate:%u [%d,%d,%d]", hdr.offset - state[chn].offset, ntp_msec, (unsigned)ts_diff, ntp_ts_diff, rtcp_sr.clock_rate, ntp_msec-(hdr.offset - state[chn].offset), ntp_msec-ts_diff, ntp_msec-ntp_ts_diff ));
			if ((int)(ntp_msec-ts_diff) > 1000) {
				getchar();
			}
			/* getchar(); */
			break;
		case b_pkt_type_data:
			fread(pes_header, sizeof(pes_header), 1, fin);
			fseek(fin, -sizeof(pes_header), SEEK_CUR);
			fseek(fin, len, SEEK_CUR);
			if(B_GET_BITS(pes_header[7], 7, 6)&2) {
				uint32_t pts_32_30 = B_GET_BITS(pes_header[9], 3, 1);
				uint32_t pts_29_22 = pes_header[10];
				uint32_t pts_21_15 = B_GET_BITS(pes_header[11], 7, 1);
				uint32_t pts_14_7 = pes_header[12];
				uint32_t pts_6_0 = B_GET_BITS(pes_header[13], 7, 2);
				uint32_t pts;

				id = pes_header[3];

				pts = ((uint32_t)pts_32_30<<29) | (pts_29_22<<21) | (pts_21_15<<14) | (pts_14_7<<6)|pts_6_0;
				if (1||id==0xc0) {
					BDBG_MSG(("PTS: id %02x %u %d", id, pts, brtp_delta_time_full(pts,last_pts[id])/45));
					last_pts[id] = pts;
				}
			}
			break;
		case b_pkt_type_rtp_data:
			fread(rtp_header, sizeof(rtp_header), 1, fin);
			fseek(fin, -sizeof(rtp_header), SEEK_CUR);
			fseek(fin, len, SEEK_CUR);
			{
				uint32_t timestamp = B_RTP_TIMESTAMP(rtp_header);
				uint8_t pt = B_RTP_PT(rtp_header);
				int delta = brtp_delta_time_full(timestamp, stream_data[pt].timestamp);
				if (delta) {
					BDBG_MSG(("%u %u (%d:%dms)", pt, timestamp, delta, (delta*(int64_t)1000)/stream_data[pt].clock_rate));
					stream_data[pt].timestamp = timestamp;
				}
			}
			break;
		case b_pkt_type_rtsp_stream_desc:
			fread(&desc, sizeof(desc), 1, fin);
			stream_data[desc.rtp_format].clock_rate = desc.clock_rate;
			break;
		default:
			BDBG_ASSERT(0);
			fseek(fin, len, SEEK_CUR);
			break;
		}
	}
error:
	fclose(fin);
	exit(0);
}

static void
b_rtp_play_url2(const char *url)
{
	brtp_play_config config;
	brtp_play_t play;
	brtp_source_t source;
	unsigned i;
	unsigned j;

	for(j=0;j<2;j++) {
		play = brtp_play_create();
		BDBG_ASSERT(play);
		brtp_play_default_cfg(play, &config);
		config.use_tcp = true;
		for(i=0;i<5;i++) {
			source = brtp_play_connect(play, url);
			if (!source) {
				break;
			}
			brtp_play_start(play, source, &config);
			BDBG_WRN(("playing ...%u/%u", j, i));
			BKNI_Sleep(5*1000); /* sleep for 5 seconds */
			BDBG_WRN(("stoping play"));
			brtp_play_stop(play);
		}
		BDBG_WRN(("destroyng play"));
		brtp_play_destroy(play);
	}

	return;
}

static void
usage(const char *name, const char *opt)
{
	if (opt) {
		fprintf(stderr, "Unknown option %s\n", opt);
	}
	fprintf(stderr,
			"%s: RTP tool\n"
			"Usage: %s [options] <input> [ -- [options] <input2>] <out>\n"
			"where options:\n",
			name, name);
	fprintf(stderr, 
			"-help - this help\n"
			"-play <url> - play RTSP style URL\n"
			"-timed_rtp - trigger saving of timed RTP data with RTCP SR reports\n"
			"-timed_pkt - trigger saving of timed data among with RTCP SR reports\n"
			"-time - analyze previousely recorded stream\n"
		   );
	fprintf(stderr,
			"-pes_id id - PES stream ID (0 for ES stream)\n"
			"-clock_offset <offset> - stream timestamp that corresponds to the begining of the stream\n"
			"-clock_rate <rate> - timestamp clock rate for the source\n"
			"-type <type> - stream type (h264, aac)\n"
			"-msg <module> - set message level debug output for the module\n"
		  );
	fprintf(stderr,
			"H.264 configuration:\n"
			"-sprop <str> - sprop parameters set\n");
	fprintf(stderr,
			"AAC configuration:\n"
			"-rate <rate> - sampling rate\n"
			"-config <config> - config parameters set\n");
	fprintf(stderr, "<input> <out>: \n"
			"\t - stdin/stdout\n"
			"\t :<port> - UDP port\n"
			"\t filename - file\n");
	exit(0);
}

#if 0
static void 
timestamp_test(void)
{
	uint32_t ts = 2090386473;
	uint32_t offset = 2094561251;
	uint32_t rate = 90000;



	BDBG_ERR(("%u %u %d",ts,offset,(int)(offset-ts)/(rate/1000)));
	ts = 1628716434;
	offset = 1637235264;
	rate = 48000;
	BDBG_ERR(("%u %u %d",ts,offset,(int)(offset-ts)/(rate/1000)));

	exit(0);
}
#endif

int 
main(int argc, const char *argv[])
{
	FILE *fout=NULL;
	int rc;
	brtp_cfg cfg;
	brtp_session_cfg session_cfg;
	brtp_parser_h264_cfg h264_cfg;
	brtp_parser_mpeg4_cfg mpeg4_cfg;
	brtp_parser_mpeg4_stream_cfg mpeg4_stream_cfg;
	brtp_parser_mux_t mux;
	brtp_parser_mux_cfg mux_cfg;
	struct b_rtp_test test;
	const char *sprop = "J01AHqkYFAe2ANQYBBrbegbAUF73wEA=,KM4JyA==";
	const char *type = "h264";
	int file_arg,arg;
	const char *file_in = NULL;
	const char *file_out = "h264.pes";
	unsigned chn_no;
	const char *config = "1210";
	const char *play_url = NULL;


	BKNI_Init();
	BDBG_Init();


#if 0
	timestamp_test();
	{
		const char in[] = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz";
		uint8_t out[64] = {0};
		bbase64_decode(in, sizeof(in), out, sizeof(out));
		BDBG_MSG(("%s -> %s", in, out));
	}
#endif

	for(chn_no=0;chn_no<sizeof(channels)/sizeof(*channels);chn_no++) {
		BKNI_Memset(&channels[chn_no], 0, sizeof(channels[chn_no]));
		brtp_parser_mux_stream_default_cfg(&channels[chn_no].stream_cfg);
		channels[chn_no].test = &test;
		channels[chn_no].mux = NULL;
		channels[chn_no].rtp = NULL;
		channels[chn_no].parser = NULL;
		channels[chn_no].stream = NULL;
		channels[chn_no].lm_stream = NULL;
		channels[chn_no].sock_in = -1;
		channels[chn_no].fin = NULL;
		channels[chn_no].cfg_len = 0;
		channels[chn_no].sync.rtp_ts = false;
		channels[chn_no].sync.rtcp_sr = false;
	}
	brtp_parser_mux_default_cfg(&mux_cfg);
	channels[blm_media_video].stream_cfg.pes_id = 0xE0;
	channels[blm_media_audio].stream_cfg.pes_id = 0xC0;

	test.mux = NULL;
	test.fout = NULL;
	test.timed_pkt = false;
	test.timed_rtp = false;
	gettimeofday(&test.start, NULL);

	for(chn_no=0,arg=1; argc>arg && chn_no<sizeof(channels)/sizeof(*channels); chn_no++) {
		for(file_in=NULL,file_arg=0; argc>arg; arg++ ) {
			if (!strcmp("-help",argv[arg])) {
				usage(argv[0], NULL);
			} else if (!strcmp("-timed_rtp",argv[arg])) {
				test.timed_rtp = true;
			} else if (!strcmp("-timed_pkt",argv[arg])) {
				test.timed_pkt = true;
			} else if (!strcmp("--",argv[arg])) {
				arg++;
				break;
			} else if (!strcmp("-play",argv[arg]) && argc>arg+1) {
				arg++;
				file_arg++;
				play_url=argv[arg];
			} else if (!strcmp("-play2",argv[arg]) && argc>arg+1) {
				arg++;
				play_url=argv[arg];
				b_rtp_play_url2(play_url);
				return 0;
			} else if (!strcmp("-time",argv[arg]) && argc>arg+1) {
				arg++;
				do_time(argv[arg]);
			} else if (!strcmp("-sprop",argv[arg]) && argc>arg+1) {
				arg++;
				sprop = argv[arg];
			} else if (!strcmp("-msg",argv[arg]) && argc>arg+1) {
				arg++;
				BDBG_SetModuleLevel(argv[arg], BDBG_eMsg); 
			} else if (!strcmp("-type",argv[arg]) && argc>arg+1) {
				arg++;
				type = argv[arg];
			} else if (!strcmp("-config",argv[arg]) && argc>arg+1) {
				arg++;
				config = argv[arg];
			} else if (!strcmp("-pes_id",argv[arg]) && argc>arg+1) {
				arg++;
				channels[chn_no].stream_cfg.pes_id = strtol(argv[arg], NULL, 0);
			} else if (!strcmp("-clock_offset",argv[arg]) && argc>arg+1) {
				arg++;
				channels[chn_no].stream_cfg.clock_offset = strtol(argv[arg], NULL, 0);
			} else if (!strcmp("-clock_rate",argv[arg]) && argc>arg+1) {
				arg++;
				channels[chn_no].stream_cfg.clock_rate = strtol(argv[arg], NULL, 0);
			} else if (argv[arg][0]=='-' && argv[arg][1]!='\0') {
				usage(argv[0], argv[arg]);
				/* doesn't reaches here */
			} else if (*argv[arg]!='\0') {
				switch(file_arg) {
				case 0: file_in = argv[arg];break;
				case 1: file_out = argv[arg];break;
				default:
					usage(argv[0], argv[arg]);
					/* doesn't reaches here */
				}
				file_arg++;
			} else {
				usage(argv[0], argv[arg]);
				/* doesn't reaches here */
			}
		}
		
		BDBG_WRN(("reading source[%u] %s pes_id: %#x from %s", chn_no, type, channels[chn_no].stream_cfg.pes_id, play_url?play_url:file_in));
		/* open a source */

		if (play_url) {
			continue; /* skip it */
		}
		if (file_in==NULL) {
			fprintf(stderr, "input sourcer shall be set");
			usage(argv[0],NULL);
		} else if (!strcmp(file_in,"-")) {
			channels[chn_no].fin = stdin;
		} else if (*file_in==':') {
			struct sockaddr_in saddr;
			unsigned port = atol(file_in+1);
			int on=1;
	
			channels[chn_no].sock_in = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (channels[chn_no].sock_in<0) {
				perror("socket");
				return 1;
			}
			memset(&saddr, 0, sizeof saddr);
			BDBG_MSG(("port: %d", port));
			saddr.sin_port = htons(port);
			saddr.sin_family = AF_INET;
			saddr.sin_addr.s_addr = INADDR_ANY ;
			rc = bind(channels[chn_no].sock_in, (struct sockaddr *)&saddr, sizeof saddr);
			if (rc!=0) {
				perror("bind");
				return 1;
			}
			rc = setsockopt(channels[chn_no].sock_in, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
			if (rc!=0) {
				perror("setsockopt");
				exit(1);
			}
		} else {
			channels[chn_no].fin = fopen(file_in,"rb");
			if (!channels[chn_no].fin) {
				perror(file_in);
				return 1;
			}
		}


		if(!strcmp(type,"h264")) {
			brtp_parser_h264_default_cfg(&h264_cfg);
			h264_cfg.meta = BKNI_Malloc(h264_cfg.meta_len);
			channels[chn_no].parser = brtp_parser_h264_create(&h264_cfg);
			channels[chn_no].stream_cfg.clock_rate = 90000; /* default */
			channels[chn_no].cfg_len = strlen(sprop)+1;
			/* BDBG_ASSERT(channels[chn_no].cfg_len <= sizeof(channels[chn_no].cfg)); */
			BKNI_Memcpy(channels[chn_no].cfg, sprop, channels[chn_no].cfg_len);
		} else if (!strcmp(type,"aac")) {
			brtp_parser_mpeg4_default_cfg(&mpeg4_cfg);
			channels[chn_no].parser = brtp_parser_mpeg4_create(&mpeg4_cfg);
			brtp_parser_mpeg4_default_stream_cfg(brtp_parser_mpeg4_aac_hbr, &mpeg4_stream_cfg);
			mpeg4_stream_cfg.config_len = bhex_decode(config, strlen(config), mpeg4_stream_cfg.config, sizeof(mpeg4_stream_cfg.config));
			BDBG_ASSERT(mpeg4_stream_cfg.config_len == 2);
			channels[chn_no].cfg_len = sizeof(mpeg4_stream_cfg);
			/* BDBG_ASSERT(channels[chn_no].cfg_len <= sizeof(channels[chn_no].cfg)); */
			BKNI_Memcpy(channels[chn_no].cfg, &mpeg4_stream_cfg, channels[chn_no].cfg_len);
		} else {
			fprintf(stderr, "unknown stream type '%s'\n",  type);
			return 1;
		}
		BDBG_ASSERT(channels[chn_no].parser);

		brtp_default_cfg(&cfg);
		cfg.pkt_free = b_pkt_free;
		channels[chn_no].rtp = brtp_create(&cfg);
		BDBG_ASSERT(channels[chn_no].rtp);
	}

	if (!strcmp(file_out,"-")) {
		fout = stdout;
	} else {
		fout = fopen(file_out, "wb");
		if (!fout) {
			perror(file_out);
			return 1;
		}
	}

	
	test.fout = fout;
	BDBG_ASSERT(test.fout);


	mux_cfg.meta = BKNI_Malloc(mux_cfg.meta_len);
	mux_cfg.data_sink = b_rtp_save_payload;
	mux_cfg.data_sink_cntx = &test;
	mux_cfg.mode = channels[0].stream_cfg.pes_id==0?brtp_mux_mode_es:brtp_mux_mode_pes;
	mux = brtp_parser_mux_create(&mux_cfg);
	test.mux = mux;

	if (play_url) {
		b_rtp_play_url(mux, channels, play_url);
	}

	for(chn_no=0;chn_no<sizeof(channels)/sizeof(*channels);chn_no++) {
		if (channels[chn_no].parser) {
			brtp_default_session_cfg(channels[chn_no].rtp, &session_cfg);
			brtp_start(channels[chn_no].rtp, channels[chn_no].parser, &session_cfg);

			channels[chn_no].stream = channels[chn_no].parser->start(channels[chn_no].parser, mux, &channels[chn_no].stream_cfg, channels[chn_no].cfg, channels[chn_no].cfg_len);
			BDBG_ASSERT(channels[chn_no].stream);
		}
	}

	chn_no = 0;
	if(channels[0].sock_in>=0) {
		b_rtp_socket_feed(channels);
	} else {
		b_rtp_file_feed(channels);
	}
	for(chn_no=0;chn_no<sizeof(channels)/sizeof(*channels);chn_no++) {
		if (channels[chn_no].parser) {
			channels[chn_no].parser->stop(channels[chn_no].parser);
			brtp_stop(channels[chn_no].rtp);
			brtp_destroy(channels[chn_no].rtp);
			channels[chn_no].parser->destroy(channels[chn_no].parser);
			if (channels[chn_no].sock_in != -1) {
				close(channels[chn_no].sock_in);
			} else if (channels[chn_no].fin!=stdin) {
				fclose(channels[chn_no].fin);
			}
		}
	}

	brtp_parser_mux_destroy(mux);

	if (fout!=stdout) {
		fclose(stdout);
	}
	return 0;
}

