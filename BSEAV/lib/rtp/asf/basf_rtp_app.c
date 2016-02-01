/***************************************************************************
 *     Copyright (c) 2002-2007, Broadcom Corporation
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
 * this is a re-written url processing set of functions, based on the murl.cpp 
 * C++ module in the reference software. It is a minimal implementation used by
 * the drm, to aid in the process of remote license grabbing.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "bsettop.h"
#include "bargs.h"
#include "btune.h"

#define B_N_RESOURCES	        (2)
#define DECODER_FILL_LIMIT      (3.0)
#define DECODER_DEFAULT_FILL    (1.0)
#define MBIT                    (1024*1024)

/* decoder fill rates, to ip bitrates */
#define BASF_RTP_ASF_0_BWRATE   (0*(MBIT))
#define BASF_RTP_ASF_1_BWRATE   (1*(MBIT))
#define BASF_RTP_ASF_2_BWRATE   (2*(MBIT))
#define BASF_RTP_ASF_3_BWRATE   (3*(MBIT))
#define BASF_RTP_ASF_4_BWRATE   (4*(MBIT))
#define BASF_RTP_ASF_5_BWRATE   (5*(MBIT))
#define BASF_RTP_ASF_6_BWRATE   (6*(MBIT))
#define BASF_RTP_ASF_7_BWRATE   (7*(MBIT))

/* local refsw includes */
#include "bstd.h"           
#include "bkni.h"
#include "bdbg.h"

/* asf parser includes */
#include "basf_parser.h"
#include "basf_rtp_proc.h"
#include "blst_squeue.h"    /* blst_sq_ */

BDBG_MODULE(basf_rtp_app);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)
#ifdef PROFILE
#undef BDBG_MSG
#define BDBG_MSG(x)
#endif

/* bitrate to decoder fill limit */
typedef struct bitlimit {
    int64_t bitrate;    /* bitrate in */
    float limit;        /* decoder fill limit */
} bitlimit;

bitlimit limits[] = {   /* bitrate to decoder fill limit */
    {BASF_RTP_ASF_0_BWRATE, (4.4)},
    {BASF_RTP_ASF_1_BWRATE, (4.0)}, {BASF_RTP_ASF_2_BWRATE, (3.6)},
    {BASF_RTP_ASF_3_BWRATE, (3.2)}, {BASF_RTP_ASF_4_BWRATE, (2.8)},
    {BASF_RTP_ASF_5_BWRATE, (2.4)}, {BASF_RTP_ASF_6_BWRATE, (2.0)},
    {BASF_RTP_ASF_7_BWRATE, (1.6)} 
};

/* global rtp buffer descriptor */
typedef struct basf_feed_buffer {
    size_t offset;          /* offset into buffer */
    size_t size;            /* total size */
    float limit;            /* typically 2 */
    void* buffer;           /* data itself*/
} basf_feed_buffer;

/* new arguments on the cmdline */
static struct b_extra_arg {
    const char *link;
    const char *ip_type;
} b_extra_arg = {
	NULL,
    NULL
};

/* new args table */
static const struct bargs bextra_args[] = {
    {"link", bargString, parse_string, "RTSP link", (void **)&b_extra_arg.link, NULL},
    {"ip_type", bargString, parse_string, "IP type(udp/tcp)", (void **)&b_extra_arg.ip_type, NULL},
    BARGS_TERMINATOR
};

/* allowed arg types */
static const bargs *decode_args[] = {
	b_args,
	bstreamer_args,
	bmpeg_args,
	bdisplay_args,
	bplayback_args,
	bdecode_args,
	bwindow_args,
	bextra_args,
	NULL
};

static void
usage(const char *name)
{
	fprintf(stderr, "Settop ASF RTP decode application\n\n");
    fprintf(stderr, "./asfip -video_type <vtype> -audio_type <atype> -mpeg_type <asf> -audio <apid> -video <vpid> -link <url>\n\n");
    fprintf(stderr, "\tvtype - video type -> vc1, vc-1, vc1-sm\n");
    fprintf(stderr, "\tatype - audio type -> wma\n");
    fprintf(stderr, "\tapid  - audio pid  -> audio stream id\n");
    fprintf(stderr, "\tapid  - video pid  -> video stream id\n");
    fprintf(stderr, "\turl   - windows media server publishing point URL\n"
                    "\t\t(eg. rtsp://<IP>:<PORT>/<PublishingPoint>)\n\n");
    fprintf(stderr, "Extended video decode parameters:\n\n");
	barg_usage(name, decode_args, "[filename] [indexname]");
	return;
}

static void 
invalid_arg(int arg, const char *argv[])
{
	fprintf(stderr, "Invalid argument %s\n", argv[arg]);
	usage(argv[0]);
	exit(1);
}

#include <sys/time.h>
struct bplay_pipe {
	bplaypump_t playpump;
	int fd;
	bool eof;
};

/* build commandline args for the asf feeder */
static int 
basf_rtp_feed_build_command(unsigned int video, unsigned int audio, const char* link, 
                            basf_rtp_feed_descriptor* descr)
{
    int ret = 0;

    descr->video_id = 0x22; descr->audio_id = 0x21;
    descr->video = video; descr->audio = audio;
    descr->link = link; descr->file = "ma20081.wmv";
    descr->pes = "ma20081.pes"; descr->ts = "ma20081.ts";
    descr->msg = NULL; descr->nowait = 1;
    descr->quiet = 1;

    return (ret);
}

/* determine if the report must be displayed */
static inline int 
basf_rtp_must_report(void)
{
    int rep = 0;
    static struct timeval prev = {0, 0};
    struct timeval now;

    gettimeofday(&now, NULL);
	if ((rep=((now.tv_sec-prev.tv_sec)>0))) {     /* has at least 1 sec elapsed ? */
        prev = now;
    }
    return (rep);
}

/* print decode/playpump report */
static void
basf_rtp_feed_report(bdecode_status cur, bdecode_status prev, bplaypump_status status)
{
    static unsigned int ct = 1;

    fprintf(stderr, "[%u] video: %u%u%c pts %lu/%lu/%lu stc %lu/%lu/%lu %d buf %lu/%lu audio: pts %lu/%lu/%lu stc %lu/%lu/%lu %d buf %lu/%lu", 
                ct, cur.source_width, cur.source_height, cur.interlaced?'i':'p',
                cur.video_pts, cur.video_pts-prev.video_pts, cur.video_pts/ct, 
                cur.video_stc, cur.video_stc-prev.video_stc, cur.video_stc/ct, (int)(cur.video_stc - cur.video_pts),
                cur.video_fifo_depth, cur.video_fifo_size,
                cur.audio_pts, cur.audio_pts-prev.audio_pts, cur.audio_pts/ct, 
                cur.audio_stc, cur.audio_stc-prev.audio_stc, cur.audio_stc/ct, (int)(cur.audio_stc - cur.audio_pts),
                cur.audio_fifo_depth, cur.audio_fifo_size);
    fprintf(stderr, " play %lu/%lu/%lu/%lu\n", status.fifo_depth, status.fifo_size, (unsigned long)status.bytes_played,
            (unsigned long)(8*status.bytes_played/ct)); ct++;
}

/* initialize defaults on global buffer struct */
void 
bplay_pipe_init_buffer(basf_feed_buffer_t buff, unsigned int limit) 
{
    BDBG_ASSERT(buff);
    if (buff) {
        buff->offset = 0;           /* start from 0 */
        buff->limit = limit;        /* commit when half of the decode buff is filled */
        buff->buffer = NULL;        /* init buffer is null */
        buff->size = 0xFFFFFFFF;    /* some very large value */
    }
}

/* dynamically calculate the decoder fill limit based on the bitrate */
void
bplaypump_calc_dyn_decodelimit(unsigned int bitrate, float* limit)
{
    int lim_len, index = 0;
    BDBG_ASSERT(limit);

    if (limit) {
        lim_len = sizeof(limits)/sizeof(limits[0]);         /* if smaller than lowest */
        if (bitrate < limits[0].bitrate) {
                *limit = limits[0].limit;
                BDBG_ERR(("\n %ld < %ld, %f\n", (long)bitrate, (long)limits[0].bitrate,
                          limits[0].limit));
                return;
        } else if (bitrate > limits[lim_len-1].bitrate) {  /* if larger than largest */
                *limit = limits[lim_len-1].limit;
                BDBG_ERR(("\n %ld > %ld, %f\n", (long)bitrate, (long)limits[lim_len-1].bitrate,
                          limits[lim_len-1].limit));
                return;
        } 
        for(index = 0; index < lim_len-1; index++) {        /* if in between */
            if ((limits[index].bitrate < bitrate) && 
                (bitrate < limits[index+1].bitrate)) {
                    BDBG_ERR(("\n %ld < %ld < %ld, %f\n", (long)limits[index].bitrate, 
                              (long)bitrate, (long)limits[index+1].bitrate, 
                              limits[index].limit));
                    *limit = limits[index].limit;
                    break;
            }
        }
    }
}

/* measure bitrate deviation */
void
brtp_asf_packet_measure_bitrate(unsigned size, unsigned int rtp_timestamp)
{
    /* for timestamp diff calculations */
    static unsigned long last_ts = 0;
    unsigned long curr_ts;

    static struct timeval last_tv = {0,0};
    struct timeval now, diff;

    BSTD_UNUSED(size);

    /* calculate bitrate */
    if(last_tv.tv_sec == 0) {
        gettimeofday(&last_tv, NULL);
        last_ts = rtp_timestamp;
    }
    curr_ts = rtp_timestamp;
    if(curr_ts - last_ts != 0) {
        gettimeofday(&now, NULL);
        timersub(&now, &last_tv, &diff); 
        /* list bitrate */
        BDBG_MSG_TRACE(("pkt arrival: %fms stream: %dms", diff.tv_usec/(float)1000, curr_ts-last_ts));
        last_tv = now;
    }
    last_ts = curr_ts;
}

/* extract ip type from commandline argument */
basf_rtp_ip_type 
brtp_asf_get_ip_type(const char* ip_type) 
{
    basf_rtp_ip_type ret = basf_rtp_ip_tcp; /* default is tcp */
    if (!ip_type) goto err;                 /* nothing on the cmdline */
    
    /* udp was selected for ip transport type */
    if (strcasecmp(ip_type, "udp")==0) { 
        ret = basf_rtp_ip_udp; 
    }

    /* look for tcp arg also, we might later change default */
    if (strcasecmp(ip_type, "tcp")==0) { 
        ret = basf_rtp_ip_tcp; 
    }

err:    
    return (ret);
}

/* see if we can fit this in the existing buff, if not, get a new buff */
int
bplay_pipe_peek_buffer(bplaypump_t playpump, basf_feed_buffer* buff, basf_rtp_info* desc)
{
    int ret = 0;

    BDBG_ASSERT(playpump);      /* validate inputs */
    BDBG_ASSERT(buff);
    BDBG_ASSERT(buff->limit);

    /* PT fake commit */
#if 1
    desc->commit_buffers = true;
    goto commit;  
#endif
    
#if 0
    brtp_asf_packet_measure_bitrate(desc->size, desc->timestamp);                       /* measure bitrate deviation */
#endif

    if((!playpump) || (!buff) || (buff->limit == 0)) {
        BDBG_ERR(("**** PKB invalid input in pipe"));
        return (ret) = -1;
    } else if ((buff->limit == DECODER_DEFAULT_FILL) && (desc->bitrate != 0)) {
        bplaypump_calc_dyn_decodelimit(desc->bitrate, &buff->limit);    
    }
commit:
    if (((buff->offset + desc->size >= (float)buff->size/(float)buff->limit) && 
         (buff->size)) || desc->commit_buffers) {                                       /* check validity of offsets */
        if(bplaypump_read_complete(playpump, 0, buff->offset)) {                        /* signal buffer end */
            BDBG_ERR(("**** PKB bplaypump_get_buffer failed"));
        } else {
            BDBG_MSG(("**** PKB RD COMPLETE buff->offset=%d\n", buff->offset));
            buff->offset = 0; buff->size = 0;
        }        
    }
    if((buff->offset == 0) || (buff->offset + desc->size >= buff->size)) {              /* new run, no offset */
        if(bplaypump_get_buffer(playpump, &buff->buffer, &buff->size)) {                /* request a new buffer */
            ret = -1; BDBG_ERR(("**** PKB bplaypump_get_buffer failed"));
        } else if (desc->size >= buff->size) { 
            BDBG_MSG(("**** PKB WRN size (%d) > buff->size (%d)\n", desc->size, buff->size));
            memcpy((unsigned char*)buff->buffer + buff->offset, desc->chunk, buff->size);
            buff->offset += buff->size; desc->chunk += buff->size;
            desc->size -= buff->size;
            goto commit;            
        }
        else { BDBG_MSG(("**** PKB NEW buff->size=%d\n", buff->size));
        }
    } 
    BDBG_MSG(("**** PKB COPY AT offset=%d size=%d\n", buff->offset, desc->size));
    memcpy((unsigned char*)buff->buffer + buff->offset, desc->chunk, desc->size);       /* fill buffer at offset */
    buff->offset += desc->size;

    return (ret); /* != 0 means still space in */
}

/* playpump rtp handler function */
static int 
bplay_pipe_run(basf_rtsp_handler_ctx* ctx, void* client_data, basf_rtp_info* desc)
{
    bdecode_status cur;                 /* status holders */
    static bdecode_status prev;
    bplaypump_status status;
    bplaypump_t playpump = NULL;        /* playback related */
    bdecode_t decode = NULL;

    basf_feed_buffer_t buff = NULL;
    basf_rtp_feed_ctx_t feed_ctx = (basf_rtp_feed_ctx_t)client_data;

    BSTD_UNUSED(ctx);
    BDBG_ASSERT(feed_ctx);

    if(feed_ctx) {
        playpump = feed_ctx->playpump; 	/* recover playpump */
        BDBG_ASSERT(playpump); 
        decode = feed_ctx->decode;      /* recover decode instance */
        BDBG_ASSERT(decode);  
        buff = feed_ctx->buff;          /* recover play buffer */
        BDBG_ASSERT(buff);  
        bdecode_get_status(decode, &prev); 
    }
    if(desc->size <= 0) {
        BDBG_ERR(("PR invalid chunk size: %d", desc->size));
        return (-1);
    }
    /* send to final decode buffer and report on playback */
    if((bplay_pipe_peek_buffer(playpump, buff, desc) != -1) && (buff)) {
        if (basf_rtp_must_report()) {                   /* only report if needed */
            bdecode_get_status(decode, &cur);            
            bplaypump_get_status(playpump, &status);    /* get new status */
            basf_rtp_feed_report(cur, prev, status);                
            prev = cur;
        } 
    }
   
    return (0);
}

int
main(int argc, const char *argv[])
{
    bresult rc;    
    bstream_mpeg mpeg;
    bstream_t stream[B_N_RESOURCES];
    bdisplay_t display=NULL;
    bdecode_t decode[B_N_RESOURCES];
    bdecode_window_t window[B_N_RESOURCES];
    bplaypump_t playpump[B_N_RESOURCES];
    int last_arg;
    const char *fname;
    const char *indxname;
    char strbuf[256];
    int display_index, window_index, decode_index, playback_index;
    int channel, cur_arg;
    bool next_channel;
    bool digital;
    BERR_Code err;

    basf_rtp_feed_ctx feed_ctx;                                     /* feeder context */
    basf_rtp_feed_descriptor feed_descr;                            /* feeder descriptor */
    basf_feed_buffer pk_buff;                                       /* global rtp buffer stateholder */
   
    int ts_pes_out = 0;                                             /* number of feeder args */
    struct bplay_pipe pipe = {NULL,-1,false};
    bplaypump_open_params* params = NULL;
    playpump[0] = NULL;

    BKNI_Init();
    BDBG_Init(); 

    err = BDBG_SetLevel(BDBG_eErr);
    err = BDBG_SetModuleLevel("basf_rtp_test", BDBG_eErr); 
    err = BDBG_SetModuleLevel("basf_ip", BDBG_eErr);
    
    if (argc<2) {
        usage(argv[0]);
        return 1;
    }
    cur_arg=1;channel=0;
    do { 
        next_channel = false;
        stream[channel] = NULL;	
        fname=NULL;
        indxname=NULL;
        display_index=window_index=decode_index=playback_index=channel;
        barg_clear(decode_args);
        last_arg = barg_parse(argv, decode_args, cur_arg, argc);
        if (last_arg<0) {
            invalid_arg(-last_arg, argv);
        } else if (last_arg==0) { /* there is should be at least one extra parameter */
            /* do nothing */
        } else {
            if (strcmp(argv[last_arg],"--")==0) {
                next_channel = true;
                last_arg++;
            } else {
                fname = argv[last_arg++];
                if (argc>last_arg && strcmp(argv[last_arg],"--")!=0) {
                    indxname = argv[last_arg++];
                } else {
                    strcpy(strbuf, fname);
                    strcat(strbuf, ".ndx");
                    indxname = strbuf;
                }
                if (last_arg < argc && strcmp(argv[last_arg],"--")==0) {
                    next_channel = true;
                    last_arg++;
                } 
            }
        }
        if (channel==0) {
            rc = bsettop_init(BSETTOP_VERSION); /* initialize global stuff */
            if (rc!=b_ok) {
                fprintf(stderr, "Initialization error\n");
                return 1;
            }
        }
        btune_mpeg_init(&mpeg);
        digital = mpeg.video[0].pid || mpeg.audio[0].pid || bmpeg_arg.mpeg_type;

        /* open decode. if decode engine not set use the same channel as tuner channel */
        if (bdecode_arg.channel)
            decode_index = *bdecode_arg.channel;
        else if (btuner_arg.channel)
            decode_index = *btuner_arg.channel;
        else
            decode_index = channel;
        decode[channel] = bdecode_open(B_ID(decode_index));
        if (!decode[channel]) {
            fprintf(stderr, "Decode %d not avaliable\n", decode_index);
            return 1;
        }

        /* play from a pipe */
        if (digital) {
            bplaypump_params playpump_params;
            bplaypump_open_params playpump_open_params;
            
            bplaypump_get_open_params(B_ID(playback_index), &playpump_open_params);
            playpump_open_params.buffer_size *= 2;
            playpump_open_params.num_descriptors *= 2;
          
#if B_HAS_DRM
            memset(&mpeg.encryption.key, 0, 4);
#endif /* B_HAS_DRM */
            
            playpump[channel] = bplaypump_open(B_ID(playback_index), &playpump_open_params/* params */);
            if (!playpump[channel]) {
                fprintf(stderr, "Can't open playpump\n");
                return 1;
            }
            bplaypump_params_init(&playpump_params, playpump[channel]);
            playpump_params.is_playback_ip = true;
            /* playpump_params.use_live_playback_mode = true;  */
            stream[channel] = bplaypump_start(playpump[channel], &mpeg, &playpump_params);
            if (!stream[channel]) {
                fprintf(stderr, "Can't start playback\n");
                return 1;
            }
            pipe.playpump = playpump[channel];
            pipe.fd = 0;
        }

        if (display==NULL) {
            /* open display, if display not set use the same channel number as tuner */
            if (bdisplay_arg.display) {
                display_index = *bdisplay_arg.display;
            } else {
                display_index = decode_index;
            }
            display = bdisplay_open(B_ID(display_index));
            if (!display) {
                fprintf(stderr, "Display %d not available\n", display_index);
                return 1;
            }

            if (bdisplay_arg.composite || bdisplay_arg.svideo || bdisplay_arg.component || bdisplay_arg.dvi || 
                    bdisplay_arg.format || bdisplay_arg.aspect_ratio || bdisplay_arg.content_mode) {
                bdisplay_settings settings;

                bdisplay_get(display, &settings);
                if (bdisplay_arg.component || bdisplay_arg.composite || bdisplay_arg.dvi) {
                    /* remove all existent outputs */
                    settings.component = NULL;
                    settings.composite = NULL;
                    settings.svideo = NULL;
                    settings.dvi = NULL;
                    settings.rf = NULL;
                }
                if (bdisplay_arg.composite) {
                    boutput_composite_t composite=NULL;
                    composite = boutput_composite_open(B_ID(0)); /* open first composite output */
                    if (!composite) {
                        fprintf(stderr, "Composite output not available\n");
                        return 1;
                    }
                    settings.composite = composite;
                }
                if (bdisplay_arg.svideo) {
                    boutput_svideo_t svideo=NULL;
                    svideo = boutput_svideo_open(B_ID(0));
                    if (!svideo) {
                        fprintf(stderr, "S-Video output not available\n");
                        return 1;
                    }
                    settings.svideo = svideo;
                } 
                if (bdisplay_arg.component) {
                    boutput_component_t component=NULL;
                    component = boutput_component_open(B_ID(0));
                    if (!component) {
                        fprintf(stderr, "Component output not available\n");
                        return 1;
                    }
                    settings.component = component;
                }
                if (bdisplay_arg.dvi) {
                    boutput_dvi_t dvi=NULL;
                    boutput_dvi_settings dvi_settings;
                    dvi = boutput_dvi_open(B_ID(0));
                    if (!dvi) {
                        fprintf(stderr, "DVI output not available\n");
                        return 1;
                    }
                    boutput_dvi_get(dvi, &dvi_settings);
                    dvi_settings.hdcp = *bdisplay_arg.dvi;
                    boutput_dvi_set(dvi, &dvi_settings);
                    settings.dvi = dvi;
                }
                if (bdisplay_arg.format) {
                    bvideo_format_settings s;
                    settings.format = *bdisplay_arg.format;

                    bvideo_get_format_settings(settings.format, &s);
                    if (s.height > 576 || !s.interlaced) {                        
                        settings.svideo = NULL;             /* this is HD */
                        settings.composite = NULL;
                        settings.rf = NULL;
                    }
                }
                if (bdisplay_arg.aspect_ratio) {
                    settings.aspect_ratio = *bdisplay_arg.aspect_ratio;
                }
                if (bdisplay_arg.content_mode) {
                    settings.content_mode = *bdisplay_arg.content_mode;
                }
                rc = bdisplay_set(display, &settings);
                if (rc!=b_ok) {
                    fprintf(stderr, "Requested display configuration not supported\n");
                    return 1;
                }
            }	
        }
        if (bwindow_arg.index) {
            window_index = *bwindow_arg.index;
        } else {
            window_index = decode_index;
        }
        window[channel] = bdecode_window_open(B_ID(window_index), display);
        if (!window[channel]) {
            fprintf(stderr, "Decode window %d not available\n", window_index);
            return 1;
        }

        if (!stream[channel]) {
            fprintf(stderr, "Stream is not available\n");
            return 1;
        }

        /* display program */
        rc = bdecode_start(decode[channel], stream[channel], window[channel]); /* start decode of the stream from the stream using specified decode engine displaying on the specified  window */
        if (rc!=b_ok) {
            fprintf(stderr, "Can't start decode\n");
            return 1;
        }

        if (bwindow_arg.window_rect ||
                bwindow_arg.x || bwindow_arg.y || bwindow_arg.width || bwindow_arg.height ||
                bwindow_arg.clip_x || bwindow_arg.clip_y || bwindow_arg.clip_width || bwindow_arg.clip_height
           ) {
            bdecode_window_settings settings;
            rc = bdecode_window_get(window[channel], &settings);
            if (rc==b_ok) {
                if (bwindow_arg.window_rect) {
                    sscanf(bwindow_arg.window_rect, "%d,%d,%u,%u",
                            &settings.position.x,
                            &settings.position.y,
                            &settings.position.width,
                            &settings.position.height);
                } else {
                    if (bwindow_arg.x) settings.position.x = *bwindow_arg.x;
                    if (bwindow_arg.y) settings.position.y = *bwindow_arg.y;
                    if (bwindow_arg.width) settings.position.width = *bwindow_arg.width;
                    if (bwindow_arg.height) settings.position.height = *bwindow_arg.height;
                }
                if (bwindow_arg.clip_x) settings.cliprect.x = *bwindow_arg.clip_x;
                if (bwindow_arg.clip_y) settings.cliprect.y = *bwindow_arg.clip_y;
                if (bwindow_arg.clip_width) settings.cliprect.width = *bwindow_arg.clip_width;
                if (bwindow_arg.clip_height) settings.cliprect.height = *bwindow_arg.clip_height;

                printf("decode_window %d,%d,%d,%d[%d,%d,%d,%d]\n",
                        settings.position.x,settings.position.y,settings.position.width,settings.position.height,
                        settings.cliprect.x,settings.cliprect.y,settings.cliprect.width,settings.cliprect.height
                      );
                rc = bdecode_window_set(window[channel],  &settings);
            }
            if (rc!=b_ok) {
                fprintf(stderr, "Can't set decode window parameters\n");
            }
        }
        cur_arg = last_arg;
        channel++;
    } while(next_channel);

    /* call asf feed function */
    if(!basf_rtp_feed_build_command(ts_pes_out ? mpeg.video[0].pid : 0, ts_pes_out ? mpeg.audio[0].pid : 0, 
                                    b_extra_arg.link, &feed_descr)) {
        feed_ctx.decode = decode[0];
        feed_ctx.playpump = playpump[0];
        feed_ctx.buff = &pk_buff;
        bplay_pipe_init_buffer(feed_ctx.buff, DECODER_DEFAULT_FILL);   /* half the decode buffer */
        basf_rtp_feed_entrypoint(bplay_pipe_run, (void*)&feed_ctx, &feed_descr, brtp_asf_get_ip_type(b_extra_arg.ip_type));
    } else { fprintf(stderr, "could not build rtp feed cmdline");
    }

    while(channel-->0) {
        bstream_close(stream[channel]);
        bdecode_window_close(window[channel]);
        bdecode_close(decode[channel]);
    }

    bdisplay_close(display);
 
    return 0;
}

