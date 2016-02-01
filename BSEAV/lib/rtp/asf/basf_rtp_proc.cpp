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

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>      /* ntohs */
#include <errno.h>
#include <time.h>           /* rtp timestamp */
#include <sys/time.h>
#include <math.h>

/* local refsw includes */
#include "bstd.h"           
#include "bkni.h"
#include "bdbg.h"
#include "bsettop.h"

/* asf parser includes */
#include "basf_parser.h"
#include "blst_squeue.h"    /* blst_sq_ */
#include "basf_rtp_proc.h"

/* flags indexes for asf rtp members */
#define IDX_SAMPLE      (0x1)
#define IDX_LENGTH      (0x2)
#define IDX_TSTAMP      (0x3)
#define IDX_DURATION    (0x4)
#define IDX_SEQUENCE    (0x5)

/* constant definitions */
#define RTP_PAYLOAD_TYPE_ASF_1  (0x60)          /* known payload type 1*/
#define RTP_PAYLOAD_TYPE_ASF_2  (0x1A)          /* known payload type 2*/
#define RTP_PAYLOAD_VERS_ASF_2  (0x02)          /* known payload version 2 */
#define RTP_PAYLOAD_VERS_ASF_1  (0x01)          /* known payload version 1 */
#define RTP_PAYLOAD_TYPE_UNK    (-1)            /* unknown payload type */

#define BUF_SIZE	            (4096*20/*8*2*/)/* default buffer size */
#define RTP_MARKER(b)           ((b)&(0x80))    /* extract marker bit */          
#define RTP_MAX_SEQ             (0xFFFF)        /* max sequence num */
#define RTP_TIMER_INTERVAL      (10*1000)       /* max timeout interval */
#define RTP_ASF_TS_CLOCKRATE    (1000)          /* clockrate is 1kHZ acc. to klemets-asf-rtp-00 */
#define RTSP_FILE_DUMP          "/mnt/hd/videos/dump.wmv"    /* default file to dump to */

#define PACK_DELIMITER          "----"          /* delimiter to put in between asf packets */
#undef FEED_DELIMITER                           /* disabled by default */

#define B_ASFIP_SIMDROP_UPPER   (171)           /* simulated drop upper bound sequence */
#define B_ASFIP_SIMDROP_LOWER   (39)            /* simulated drop lower bound sequence */
#define RTP_BIT_ON(h,a)         ((h)&(1<<(a)))  /* bit-on mask get value */

/* enable debug in this module */
BDBG_MODULE(basf_rtp_proc);

#define BDBG_MSG_TRACE(x) BDBG_MSG(x)
#ifdef PROFILE
#undef BDBG_MSG
#define BDBG_MSG(x)
#endif

/* describes all possible states */
typedef enum basf_rtp_state {
    basf_rtp_idle        = 0,
    basf_rtp_receiving   = 1,
    basf_rtp_out_of_sync = 2,
    basf_rtp_syncing     = 3,
    basf_rtp_finished    = 4
} basf_rtp_state;

/* describes the received seq number */
typedef struct basf_rtp_seq {
    unsigned int prev;      /* previous sequence number */
    unsigned int cur;       /* current sequence number */
    unsigned int next;      /* next sequence number */
} basf_rtp_seq;

/* generic rtp packet structure */
typedef struct basf_rtp_payload {
    union b1 {                          /* first byte */
        struct pack {
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
            unsigned char csrcc:4;      /* contributing source count */
            unsigned char extension:1;  /* rtp extension */   
            unsigned char padding:1;     /* is padded ? */
            unsigned char version:2;    /* rtp version*/
#else /* BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */
            unsigned char version:2;    /* big endian ordering */
            unsigned char padding:1;     
            unsigned char extension:1;  
            unsigned char csrcc:4;      
#endif
            } pack;
        unsigned char b;                /* source byte */
    } b1;
    union b2 {                          /* second byte */
        struct pack {
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
            unsigned char type:7;       /* payload type ident */
            unsigned char marker:1;     /* marker flag */
#else /* BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */
            unsigned char marker:1;       /* big endian ordering */
            unsigned char type:7;                 
#endif
        } pack;
        unsigned char b;                /* source byte */
    } b2;
    unsigned short seq_num;             /* sequence number */
    unsigned long timestamp;            /* packet timestamp */
    unsigned long ssrc;                 /* synchronization source */
    unsigned int csrc[1];               /* contributing sources */
    unsigned char* data;                /* payload data, not aligned */
} basf_rtp_payload;

/* asf rtp header definition */
typedef struct basf_rtp_hdr {
    union {
        struct {
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
            unsigned char S:1;   /* indicates a sample payload */
            unsigned char L:1;   /* length is used rather than offset */
            unsigned char R:1;   /* relative timestamp on */
            unsigned char D:1;   /* duration on */
            unsigned char I:1;   /* sequence number on */
            unsigned char res:3; /* reserved */
#else /* BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */
            unsigned char res:3;
            unsigned char I:1;
            unsigned char D:1;
            unsigned char R:1;
            unsigned char L:1;
            unsigned char S:1;
#endif
        } flags;
        unsigned long len;  /* use packet length */
        unsigned long off;  /* use packet offset */
    };
    unsigned long tstamp;   /* timestamp value */
    unsigned long duration; /* duration value */
    unsigned long seqn;     /* sequence number value */
} basf_rtp_hdr;

typedef basf_rtp_hdr* basf_rtp_hdr_t;

/* asf payload part */
typedef struct basf_rtp_asf_payload {
    unsigned int size;      /* size of the data */
    unsigned char* data;    /* actual data */
                            /* fill here */ 
} basf_rtp_asf_payload;

/* describes the asf part of the rtp */
typedef struct basf_rtp_packet {
    BLST_SQ_ENTRY(basf_rtp_packet) queue;   /* link to other packets */
    basf_rtp_payload rtp;                   /* rtp payload part of the packet */
    basf_rtp_asf_payload rtp_asf;           /* asf payload part of the packet */    
} basf_rtp_packet;

/* declare a rtp packet queue */
BLST_SQ_HEAD(basf_rtp_queue, basf_rtp_packet);

/* rtp context structure */
typedef struct basf_rtp_context {
    BDBG_OBJECT(basf_rtp_context_t)     /* for debug purposes */
    unsigned char* buff;                /* dummy buffer, not in use (?) */
    unsigned int basf_rtp_state;        /* context parse state */
    basf_rtp_seq seq;                   /* overall sequence state */
    struct basf_rtp_queue waiting;      /* waiting mapped list packets */
    struct basf_rtp_queue free;         /* free mapped queue packets */
    basf_rtp_packet* list;              /* actual list of packets */    
    basf_rtp_config* cfg;               /* pointer to the config */
    unsigned int alloc_count;           /* debug allocs */
} basf_rtp_context;

/* types of file outputs */
typedef enum basf_rtp_fout_type {
    basf_rtp_fout_stdout,
    basf_rtp_fout_stderr,
    basf_rtp_fout_preset
} basf_rtp_fout_type;

/* local decls */
int basf_rtsp_out_dispatch(const basf_rtp_context_t ctx, basf_rtp_info* desc, bool bheader);

/* default config */
basf_rtp_config default_cfg = {{ 
        0,                  /* fd - stdout */
        NULL,               /* asf */
        basf_rtp_output_fd, /* mask */
        stdout,             /* file */
        NULL,               /* fn_mux */
        NULL                /* fn_mux_data */
    },            
    NULL, "", NULL,         /* ctx, link here, demux */
    NULL, 0, 0,             /* rtsp, pack size, pack count */
    basf_rtp_ip_tcp         /* tcp is default */
};

/* return the default configure rtp session */
void
basf_rtsp_default_config (basf_rtp_config_t* cfg)
{
    *cfg = &default_cfg;
}

/* open a dump file instance */
static inline int 
basf_rtsp_client_fopen(const basf_rtp_config_t cfg, const char* file)
{
    int ret = 0;
    BDBG_MSG_TRACE(("opening %s for write", file));

    BDBG_ASSERT((cfg != NULL));
    if (cfg != NULL) {
        cfg->out.fout = fopen(file, "wb");
        BDBG_ASSERT(cfg->out.fout);
        if (!cfg->out.fout)  {
            BDBG_ERR(("could not open %s, reason:\"%s\"", 
                      file, strerror(errno)));                
        }        
    }

    return (ret);
}

/* close the file we're dumping into */
static inline int 
basf_rtsp_client_fclose (const basf_rtp_context_t ctx)
{
    int ret = 0;

    BDBG_ASSERT((ctx) && (ctx->cfg->out.fout));        
    if ((ctx) && (ctx->cfg->out.fout)) {
        fclose((FILE*)ctx->cfg->out.fout);
        ctx->cfg->out.fout = NULL;
    }

    return (ret);
}

/* set out to file descriptor */
void
basf_rtsp_out_set_fd(const basf_rtp_config_t cfg, int fd)
{
    BDBG_ASSERT((cfg != NULL) && (fd > 0));
    if ((cfg) && (fd > 0)) {
        cfg->out.fd = fd;
        cfg->out.mask |= (unsigned int)basf_rtp_output_fd;
    } else { BDBG_WRN(("invalid fd"));
    }
}

/* set output to file out */
void
basf_rtsp_out_set_fout(const basf_rtp_config_t cfg, const FILE* fout)
{
    BDBG_ASSERT((cfg != NULL));
    if (cfg != NULL) {
        cfg->out.mask |= (unsigned int)basf_rtp_output_file;
        if(fout == DEFAULT_FOUT) {
            basf_rtsp_client_fopen(cfg, RTSP_FILE_DUMP);     /* open a dump file instance */
        } else {
            cfg->out.fout = fout;
        }
   } else { 
       BDBG_WRN(("invalid config on set fout"));
   }
}

/* set output to asf demux */
void
basf_rtsp_out_set_asf_demux(const basf_rtp_config_t cfg, const basf_parser_t asf)
{
    BDBG_ASSERT((cfg != NULL) && (asf != NULL));
    if ((cfg != NULL) && (asf != NULL)) {
        cfg->out.asf = asf;
        cfg->out.mask |= (unsigned int)basf_rtp_output_asf;
    } else { BDBG_WRN(("invalid asf"));
    }
}

/* set custom demux */
void
basf_rtsp_output_set_cust_demux(const basf_rtp_config_t cfg, const basf_rtp_mux_func fn, void* fn_data)
{
    BDBG_ASSERT((cfg != NULL) && (fn != NULL));
    if ((cfg != NULL) && (fn != NULL)) {
        cfg->out.fn_mux = fn;
        cfg->out.fn_mux_data = fn_data;
        cfg->out.mask |= (unsigned int)basf_rtp_output_cust;
    } else { BDBG_WRN(("invalid custom demux"));
    }
}

static inline void
basf_rtsp_packet_print(unsigned char* packet, unsigned short seq_cur, 
                       int count, int is_calc_mark, unsigned packetSize,
                       unsigned short packet_offset)
{    
#if 0
    BDBG_MSG_TRACE(("PKT SEQ: seq:%d [%d]"
           "DATA: %.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X  "
           "%.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X  "
           "size:%d pkt_offset:%u, [%s]\n", seq_cur, count, 
           packet[0], packet[1], packet[2], packet[3], 
           packet[4], packet[5], packet[6], packet[7], 
           packet[8], packet[9], packet[10], packet[11], 
           packet[12], packet[13], packet[14], packet[15],
           packetSize, packet_offset, is_calc_mark ? "m" : "-"));
#else
    BDBG_MSG_TRACE(("pkt seq:%d(%d) size:%d [%s]", 
              seq_cur, count, packetSize, is_calc_mark ? "m" : "-"));
#endif
}

/* handler for asf header data */
int 
basf_rtsp_aux_header_handler(void* clientData, unsigned char* packet, unsigned size)
{
    int feed_len;
    basf_parser_t asf;
    int ret = 0;
    static basf_rtp_info desc;
    basf_rtp_context_t data = NULL;    
    basf_ip_headers_t* asf_hdr = NULL;
    basf_rtsp_demux_t demux = NULL;

    BSTD_UNUSED(asf_hdr);
    if ((data = (basf_rtp_context_t)clientData)) {
        asf = (basf_parser_t)data->cfg->out.asf; BDBG_ASSERT(asf);
        demux = data->cfg->demux; BDBG_ASSERT(demux); 
#ifdef __HAS_LEGACY_HDR__
        basf_rtsp_get_header(demux, &asf_hdr);
        BDBG_MSG_TRACE(("filesize=%ld, pack_count=%ld, pack_size=%d\n",
                  (long)asf_hdr->filesize,
                  (long)asf_hdr->packets_num,   
                  asf_hdr->min_pack_size)); 

        data->cfg->packet_size = asf_hdr->i_min_data_packet_size;
        data->cfg->packet_count = asf_hdr->i_data_packets_count;
#else /* __HAS_LEGACY_HDR__ */
        basf_rtsp_get_packet_info(demux, &data->cfg->packet_size, 
                                  &data->cfg->packet_count);
#endif /* __HAS_LEGACY_HDR__ */
        if (asf || data->cfg->out.mask) {
           desc.chunk = packet; desc.size = size;
           desc.bitrate = 0; desc.commit_buffers = BASF_FALSE;
           feed_len = basf_rtsp_out_dispatch(data, &desc, true);
        }
    } else { BDBG_WRN(("invalid client data"));
        BDBG_ASSERT(data);
    }
    
    return (ret);
}

/* timer instance */
static void 
basf_rtp_input_timer(void *client_data)
{
    b_timer_t rtp_timer; 
    struct timeval ts;
    basf_rtsp_demux_t demux = NULL;

    BDBG_ASSERT(client_data);
    if(client_data) {
        demux = (basf_rtsp_demux_t)client_data;        
        BDBG_ASSERT(demux);
        if(demux) {
            gettimeofday(&ts, NULL);    
            BDBG_MSG_TRACE(("timer at: %u %u\n", ts.tv_sec, ts.tv_usec));
            rtp_timer = b_timer_schedule(RTP_TIMER_INTERVAL, basf_rtp_input_timer, client_data);  
            basf_rtsp_set_rtptimer(demux, rtp_timer);        
        }
    }
}

/* create the playback timer */
static b_timer_t
basf_rtp_create_timer(basf_rtsp_demux_t demux)
{
	b_timer_t rtp_timer = NULL; 

    BDBG_ASSERT(demux);
    if(demux != NULL) {
        rtp_timer = b_timer_schedule(RTP_TIMER_INTERVAL, basf_rtp_input_timer, (void*)demux);
        basf_rtsp_set_rtptimer(demux, rtp_timer);        
    }

    return (rtp_timer);
}

/* cancel the playback timer */
static void
basf_rtp_destroy_timer(basf_rtsp_demux_t demux)
{
    b_timer_t rtp_timer;
    BDBG_ASSERT(demux);

    if(demux) {
        rtp_timer = basf_rtsp_get_rtptimer(demux);
        BDBG_ASSERT(rtp_timer);
        if (rtp_timer) {
            b_timer_cancel(rtp_timer);
            basf_rtsp_set_rtptimer(demux, NULL);        
        }
    }
}

/* measure bitrate, calculate jitter */
void
brtp_asf_payload_measure_bitrate(unsigned size, unsigned int rtp_timestamp)
{    
    long delta;                                 /* timestamp diff calculations */
    unsigned long curr_ts, diff_ts;    
    struct timeval now, diff;

    static unsigned long last_ts = 0, sum_diff_ts = 0;
    static long last_delta = 0, jitter = 0;
    static struct timeval last_tv = {0,0}, sum_diff_tv = {0,0};

    BSTD_UNUSED(size);    
    if(last_tv.tv_sec == 0) {                       /* calculate bitrate */
        gettimeofday(&last_tv, NULL);
        last_ts = rtp_timestamp;
    }

    curr_ts = rtp_timestamp;
    diff_ts = (unsigned long)ceil((curr_ts - last_ts)*(1000/(double)RTP_ASF_TS_CLOCKRATE));      
    if(diff_ts >= 1) {                              /* transform to ms */
        gettimeofday(&now, NULL);                   /* get current instance */
        timersub(&now, &last_tv, &diff); 
        sum_diff_ts += diff_ts;                     /* add up to the source diff */
        sum_diff_tv.tv_usec += diff.tv_usec;        /* add up to the receiver diff */
        
        delta = (long)(sum_diff_ts - ceil((sum_diff_tv.tv_usec)/(double)1000));
        if(delta < 0) BDBG_MSG_TRACE(("overflow!"));/* warn when overflow */
        jitter = (long)(jitter + ceil((abs(delta-last_delta)-jitter)/(double)16));
        
        BDBG_MSG_TRACE(("payload arr diff:%2.fms sum:%2.fms strm diff:%dms sum:%dms jitter:%dms", 
                  diff.tv_usec/(float)1000, sum_diff_tv.tv_usec/(float)1000, diff_ts, 
                  sum_diff_ts, jitter));            /* list bitrate */
        last_delta = delta; last_tv = now;
    }
    last_ts = curr_ts;
}


/* look at raw rtp header fields and validate packet */
int 
basf_rtsp_aux_dissect_rtp(basf_rtp_packet* rtp_packet, unsigned char* pack, unsigned size)
{
    int ret = 0;
    basf_rtp_payload* rtp_payload = &rtp_packet->rtp;
    unsigned char* packet = pack;

    BDBG_MSG_TRACE(("rtp packet(%d) %.2X %.2X %.2X %.2X", size, packet[0], packet[1], packet[2], packet[3]));
    
    BDBG_ASSERT(size > 0);
    rtp_payload->b1.b = (*(unsigned char*)packet); packet++;
    rtp_payload->b2.b = (*(unsigned char*)packet); packet++;
    rtp_payload->seq_num = ntohs(*(unsigned short*)packet); packet+=2;
    rtp_payload->timestamp = ntohl(*(unsigned int*)packet); packet+=4;
    rtp_payload->ssrc = ntohl(*(unsigned int*)packet); packet+=4;
    packet += sizeof(rtp_payload->csrc[0])*rtp_payload->b1.pack.csrcc;

#if 1
    if ((RTP_PAYLOAD_VERS_ASF_1 != rtp_payload->b1.pack.version) && 
        (RTP_PAYLOAD_VERS_ASF_2 != rtp_payload->b1.pack.version)) { 
        BDBG_WRN(("unknown asf rtp version (%d)", rtp_payload->b1.pack.version));
        BDBG_ASSERT((RTP_PAYLOAD_VERS_ASF_1 == rtp_payload->b1.pack.version) ||
                    (RTP_PAYLOAD_VERS_ASF_2 == rtp_payload->b1.pack.version));
        ret = (-1);
    } else ret = packet - pack;
    
    if ((RTP_PAYLOAD_TYPE_ASF_1 != rtp_payload->b2.pack.type) && 
        (RTP_PAYLOAD_TYPE_ASF_2 != rtp_payload->b2.pack.type)) {
        BDBG_WRN(("unknown asf rtp type (%d)", rtp_payload->b2.pack.type));
        BDBG_ASSERT((RTP_PAYLOAD_TYPE_ASF_1 == rtp_payload->b2.pack.type) ||
                    (RTP_PAYLOAD_TYPE_ASF_2 == rtp_payload->b2.pack.type));
        ret = (-1);
    } else ret = packet - pack;
#else
    ret = packet-pack;
#endif
    
    brtp_asf_payload_measure_bitrate(size, rtp_payload->timestamp);
    BDBG_MSG_TRACE(("\nversion=%u padding=%u extension=%u csrcc=%u"
            " marker=%u type=%u seq_num=%u timestamp=%ul ssrc=%ul\n",
            rtp_payload->b1.pack.version, rtp_payload->b1.pack.padding, 
            rtp_payload->b1.pack.extension, rtp_payload->b1.pack.csrcc, 
            rtp_payload->b2.pack.marker, rtp_payload->b2.pack.type,
            rtp_payload->seq_num, rtp_payload->timestamp, rtp_payload->ssrc));

    return (ret);
}

/* calculate the packet loss from payload loss */
int
basf_rtp_calc_pkt_payload_loss(unsigned short seq_cur, int loss_len, int payloads_packet, int rel_pos)
{
    int lost_pkts = 0, orig_loss_len = loss_len;

    if ((loss_len == 0) || (payloads_packet == 0)) goto exit;
    if (loss_len > payloads_packet) {
        if (rel_pos != 0) {            
            loss_len -= payloads_packet - rel_pos; lost_pkts++;
            lost_pkts += (loss_len)/(payloads_packet);
            if ((loss_len%payloads_packet) != 0) lost_pkts++;
        } else {            
            lost_pkts += (loss_len)/(payloads_packet);
            if ((loss_len%payloads_packet) != 0) lost_pkts++;            
        }
    } else {
        if (rel_pos + loss_len > payloads_packet) lost_pkts+=2;
        else lost_pkts++;
    }
exit:
    if (lost_pkts)
        BDBG_MSG_TRACE(("CALC pld seq:%d lost:%d plds_pkt:%d rel_pos:%d => lost_pkts:%d",
                  seq_cur, orig_loss_len, payloads_packet, rel_pos, lost_pkts));
    return (lost_pkts);
}

/* calculate header offset based on first byte 
 * */
unsigned int
basf_rtp_calc_hdr_offset (basf_rtp_hdr_t hdr)
{
    unsigned int off = 1;                               /* account for the flag byte itself */
    
    BDBG_MSG_TRACE(("header len = 0x%.2x 0x%.2x", hdr->len, ((unsigned char*)(&hdr->len))[0]));

    if (hdr->flags.S) {BDBG_MSG_TRACE(("[S]"));} off += 0;   /* S - key sample, no influence on offset*/
    if (hdr->flags.L) {BDBG_MSG_TRACE(("[L]"));} off += 3;   /* L - length/offset, 3B, not variable */
    if (hdr->flags.R) {BDBG_MSG_TRACE(("[R]")); off += 4;}  /* R - relative timestamp, 4B */
    if (hdr->flags.D) {BDBG_MSG_TRACE(("[D]")); off += 4;}  /* D - sample duration, 4B */
    if (hdr->flags.I) {BDBG_MSG_TRACE(("[I]")); off += 4;}  /* I - sequence number, 4B */
    
    return (off);
}

unsigned char g_junk[8000];

/* new handler for rtp packets */
int 
basf_rtsp_aux_packet_handler(void* clientData, basf_rtp_info* desc)
{
    static int feed_len, plds_pack, last_plds_pack;    
    unsigned char marker_version;
    unsigned char* fill_buf; int fill_size;    
    basf_rtp_info out_desc;                     /* output info descriptor */
    basf_parser_t asf; basf_rtp_packet rtp_packet;

    static unsigned int seq_prev = 0;           /* initializers */    
    unsigned short seq_cur = 0;                 /* current sequence, relative position */    
    
    static int gather_len = 0, packet_len = 0, debug_len = 0;
    static int first_pkt_mark = 0, last_pkt_mark = 0;    
    static int out_of_sync = 0, pkt_out_of_sync = 0;
    static int recover_marker = 0, count = 0, rel_pos = 0;    

    int plds_lost = 0, plds_valid = 0, pkts_lost = 0;   
    int ret = 0, is_calc_mark = 0, pkts_valid = 0;
    int pkt_offset = 0, offset = 0, calc_off;
    
    basf_rtp_context_t data = NULL;    
    static int first_run = 1;

    data = (basf_rtp_context_t)clientData;
    BDBG_ASSERT(data); if(!data) return (ret);
    asf = (basf_parser_t)data->cfg->out.asf;    
    BDBG_ASSERT(asf); if(!data) return (ret);

    pkt_offset = basf_rtsp_aux_dissect_rtp(&rtp_packet, desc->chunk, desc->size);
    marker_version = rtp_packet.rtp.b2.b; 
    seq_cur = rtp_packet.rtp.seq_num;
    
#ifdef __HAS_SIMULATED_DROP__
    {
        static int start_drop = 0;
        if ((seq_cur % B_ASFIP_SIMDROP_UPPER) == 0) {
            start_drop = 1;
            BDBG_ERR(("simulated drop on %d", seq_cur));
            return (0);
        } else if (start_drop > 0) {
            start_drop = (start_drop < B_ASFIP_SIMDROP_LOWER) ? start_drop+1 : 0;
            BDBG_ERR(("simulated drop on %d", seq_cur));
            return (0);
        }
    }
#endif /* __HAS_SIMULATED_DROP__ */
    
    if ((seq_cur - seq_prev) <= 0) BDBG_MSG_TRACE(("REVERSED %d<->%d", seq_prev, seq_cur-1));
    is_calc_mark = RTP_MARKER(marker_version);

    /* calculate dynamic offset based on flags */
    {
        basf_rtp_hdr* hdr = (basf_rtp_hdr*)(desc->chunk + RTP_OFFSET);
        calc_off = basf_rtp_calc_hdr_offset(hdr);
        
        /* log non-4B displacements */
        if ((desc->chunk[0+RTP_OFFSET] != 0) && (desc->chunk[0+RTP_OFFSET] != 0x80)) {
            BDBG_MSG_TRACE(("calc_off=%d desc->chunk [0]=0x%.2X [1]=0x%.2x", 
                        calc_off, desc->chunk[0+RTP_OFFSET], 
                        desc->chunk[1+RTP_OFFSET]));
        }
    }

    /* offset should be dyn calculated rather than fixed */
    offset = ((pkt_offset != -1) ? (pkt_offset) : 
            (RTP_OFFSET)) + calc_off;       /* calculate asf rtp packet offset */
    
    debug_len += desc->size - RTP_OFFSET;   /* measure gathered packet length */
    desc->chunk += offset; count++;         /* rtp offset + asf offset, is it different? */    
    desc->size -= offset;                   /* adjust overall pkt size */
    
    if (recover_marker) {                   
        BDBG_ERR(("RECOVER %d", seq_cur));  /* recovery! */
        out_of_sync = recover_marker = 0;   /* force not out of sync */
        gather_len = packet_len = 0;        /* force malloc, packet len recount */
        pkt_out_of_sync = 0; rel_pos = 0;   /* not out of sync anymore, relative pos reset */
    }

    basf_rtsp_packet_print(desc->chunk, seq_cur, count, is_calc_mark, desc->size, pkt_offset);   
    packet_len += desc->size;    

#if 1 
    if (is_calc_mark) {
        BDBG_MSG_TRACE(("pack_size=%d packet_len=%d debug_len=%d", 
                    desc->packet_size, packet_len, debug_len));
        debug_len = 0;
    }
#endif
    
    if ((out_of_sync = ((seq_prev != 0) && (abs(seq_cur - seq_prev) != 1) && 
                        (seq_prev != RTP_MAX_SEQ)))) {         
        plds_lost = seq_cur-1 - seq_prev;
        if (plds_lost < 0) plds_lost += 0xFFFF;
        BDBG_ERR(("LOST %d<->%d plds_lost=%d (%d)", seq_prev, seq_cur-1, plds_lost, count));
        if (!is_calc_mark) { 
            BDBG_ERR(("DROPPED %d", seq_cur));
            /* if (data->buff) {
                data->alloc_count--;
                BDBG_ERR(("FREE DROP alloc: %d", data->alloc_count));                
                BKNI_Free(data->buff); data->buff = NULL; 
            } */        
            pkt_out_of_sync = 1;            /* out of sync and not marker, exiting */            
            goto exit;                      /* if we're out of sync and this is not a marker, bail out */ 
        } else { 
            pkt_out_of_sync = 0;            /* otherwise bail out and mark new beginning */
            BDBG_ERR(("DROPPED MARK %d", seq_cur));            
            /* if (data->buff) {
                data->alloc_count--;
                BDBG_ERR(("FREE DM alloc: %d", data->alloc_count));                
                BKNI_Free(data->buff); data->buff = NULL; 
            } */        
            gather_len = 0; recover_marker = 1; 
            goto exit; 
        }                               
    } else if((pkt_out_of_sync) && (is_calc_mark)) {
        /* if (data->buff) {            
            data->alloc_count--;
            BDBG_ERR(("FREE OOS alloc: %d", data->alloc_count));
            BKNI_Free(data->buff); data->buff = NULL;             
        } */        
        recover_marker = 1; 
        goto exit;
    }

    if ((gather_len == 0) || (recover_marker)) {                            /* moved to _start */     
        if (data->buff && !first_run) {            
            /* data->alloc_count--; */
            BDBG_MSG_TRACE(("CLEAN OOS alloc: %d", data->alloc_count));
            BKNI_Memset(data->buff, 0, desc->packet_size*RTP_PACKET_COUNT); 
            /* data->buff = NULL; */
        }              
        if(first_run) {
            data->alloc_count++;
            BDBG_WRN(("ALLOC count:%d %d", data->alloc_count, desc->packet_size*RTP_PACKET_COUNT));
            data->buff = (unsigned char*)BKNI_Malloc(desc->packet_size*RTP_PACKET_COUNT);      
            first_run = 0;
        }
    }                                                               
    if (!pkt_out_of_sync && data->buff) {
        BKNI_Memcpy(data->buff + gather_len, desc->chunk, desc->size);      /* only copy if this is in sync */
        gather_len += desc->size; plds_valid = 1;
        rel_pos++;                                                          /* modify relative position */
        if (is_calc_mark || desc->commit_buffers) {
            BDBG_MSG_TRACE(("commiting buffers : packet_size=%d packet_len=%d",
                        data->cfg->packet_size, packet_len));
            if (packet_len < data->cfg->packet_size) {
                fill_size = data->cfg->packet_size - packet_len;
                BDBG_ASSERT(fill_size > 0);
        
                // don't do this
                BDBG_MSG_TRACE(("padding fill_size = %d", fill_size));                
#if 1
                BKNI_Memset(data->buff + gather_len, 0, fill_size);         /* copy filler to gather buffer */     
                gather_len += fill_size;
#endif

                /* PT inferring larger than normal length */
#if 0
                BDBG_ERR(("bef> padding_length_type=%.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X", data->buff[0], data->buff[1], 
                            data->buff[2], data->buff[3], data->buff[4], data->buff[5],
                            data->buff[6], data->buff[7], data->buff[8], data->buff[9],
                            data->buff[10], data->buff[11]));
                
                /* (*(unsigned short*)(data->buff + 5)) = gather_len; */
                (*(unsigned char*)(data->buff + 3)) |= 0x8;    /* set padding length type to word (bits 4-5 LSB) */
                
                BKNI_Memset(g_junk, 0, 8000);
                BKNI_Memcpy(g_junk, data->buff, 7);
                BKNI_Memcpy(g_junk+7, &fill_size, 1);
                BKNI_Memcpy(g_junk+8, data->buff+7, gather_len-fill_size-7);
                BKNI_Memcpy(data->buff, g_junk, 8000);
                
                BDBG_ERR(("aft< padding_length_type=%.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X  %.2X %.2X %.2X %.2X", data->buff[0], data->buff[1], 
                            data->buff[2], data->buff[3], data->buff[4], data->buff[5],
                            data->buff[6], data->buff[7], data->buff[8], data->buff[9],
                            data->buff[10], data->buff[11]));
#endif
            }        
#if 0
                out_desc.chunk = data->buff; 
                out_desc.size = gather_len;        /* fill output structure */
                feed_len = basf_rtsp_out_dispatch(data, &out_desc, false);
                gather_len = 0;
                return (ret);
#endif
            packet_len = 0; pkts_valid = 1;            
        }
    } else BDBG_MSG_TRACE(("DROPPED PKOS %d", seq_cur));

    if (!pkt_out_of_sync && data->buff) {
        if ((gather_len + 0 /* 4 */>= (int)desc->packet_size*RTP_PACKET_COUNT) || (desc->commit_buffers)) {
            
            out_desc.chunk = data->buff; 
            out_desc.size = gather_len;        /* fill output structure */
            out_desc.bitrate = desc->bitrate; 
            out_desc.commit_buffers = desc->commit_buffers;
            out_desc.timestamp = rtp_packet.rtp.timestamp;
            
            if(desc->commit_buffers) { 
                BDBG_ERR(("COMMIT has %d bytes in buffer", out_desc.size));
            }
            
            feed_len = basf_rtsp_out_dispatch(data, &out_desc, false);
            BDBG_MSG_TRACE(("gather_len is %d data: %.2x %.2x %.2x %.2x, feeding\n", gather_len, 
                            data->buff[0], data->buff[1], data->buff[2], data->buff[3]));
            BDBG_MSG_TRACE(("FEED %d (%d+%d>=%d)", gather_len, gather_len, desc->size,
                        desc->packet_size*RTP_PACKET_COUNT));
            gather_len = 0;                                                 /* moved to stop, happens on recycle */
        } else {
            BDBG_MSG_TRACE(("gather_len is %d, adding\n", gather_len)); 
        }
    } else
        BDBG_WRN(("pkt_out_of_sync=%d OR data->buff=%p", pkt_out_of_sync, data->buff));
exit:
    if(out_of_sync) BDBG_MSG_TRACE(("eoos cur=%d prev=%d\n", seq_cur, seq_prev));
    if((is_calc_mark) && (first_pkt_mark == 0)) {                           /* calculate payloads per packet */
        first_pkt_mark = seq_cur; plds_pack = 0;
    } else if ((is_calc_mark) && (first_pkt_mark != 0)) {
        if ((!pkt_out_of_sync) && (!out_of_sync)) {
            last_pkt_mark = seq_cur;
            last_plds_pack = plds_pack = last_pkt_mark - first_pkt_mark;
            first_pkt_mark = last_pkt_mark;
        } else {
            first_pkt_mark = 0; 
            plds_pack = 0;
        }
    } else if (!is_calc_mark) {
        plds_pack = 0;
    }    

    pkts_lost = basf_rtp_calc_pkt_payload_loss(seq_cur, plds_lost,          /* calculate lost packets from lost payloads */
                    last_plds_pack, rel_pos ? rel_pos-1:0);                 /* relative position should be 0-based */
    basf_rtsp_update_stats(data->cfg->demux, pkts_lost, pkts_valid,         /* notify session manager with packet statistics */
                    plds_lost, 0, plds_valid, plds_pack);
    seq_prev = seq_cur;
    if(last_plds_pack != 0) {
        rel_pos = (rel_pos)%last_plds_pack;
    }

    return (ret);
}

/* main rtp handler */
int
basf_rtsp_aux_rtphandler(basf_rtsp_handler_ctx* ctx, void* clientData, 
                         basf_rtp_info* desc)
{
    int ret = 0;

    BDBG_ASSERT(ctx); BDBG_ASSERT(desc);
    if(!ctx) return (ret) = -1;

    switch(ctx->hdl_type) {
        case basf_rtsp_handler_header:
            basf_rtsp_aux_header_handler(clientData, desc->chunk, desc->size);
            break;
        case basf_rtsp_handler_packet:
            basf_rtsp_aux_packet_handler(clientData, desc);
            break;
    }

    return (ret);
}

/* dump asf to a file */
int 
basf_rtsp_client_write(basf_rtp_context_t ctx, unsigned char* buff, int len)
{
    int ret = 0;
#ifdef __HAS_FEED_DELIMITER__
    const char* delim = PACK_DELIMITER;
#endif

    BDBG_ASSERT((ctx) && (ctx->cfg));
    if ((ctx) && (ctx->cfg)) {
        fwrite(buff, 1, len, (FILE*)ctx->cfg->out.fout);
#ifdef __HAS_FEED_DELIMITER__
        fwrite(delim, 1, strlen(delim), (FILE*)ctx->cfg->out.fout);
#endif
    }

    return (ret);
}

/* start rtsp session */
basf_rtp_context_t 
basf_rtsp_client_open(basf_rtsp_session_t* rtsp, basf_rtsp_demux_t* demux, const basf_rtp_config_t cfg)
{
    basf_rtp_context_t ctx = NULL;
    ctx = (basf_rtp_context_t)BKNI_Malloc(sizeof(basf_rtp_context));

    BDBG_ASSERT(ctx); 
    if ((ctx) && (cfg)) { 
        ctx->cfg = cfg; cfg->ctx = ctx; 
    }

    if(0 == basf_rtsp_open_session(rtsp, demux, cfg)) {
        BDBG_ASSERT(*rtsp); BDBG_ASSERT(*demux);        
        if ((*rtsp) && (*demux)) {           
            basf_rtp_create_timer(*demux);                  /* create rtp main timer */
            cfg->demux = (*demux);                          /* cross-assign references */
            cfg->rtsp = (*rtsp);                                                        
            if(!basf_rtsp_send_options(*rtsp, *demux)) {    /* only go further if options are ok */
                basf_rtsp_sethandler(*demux, basf_rtsp_aux_rtphandler, (void*)ctx);
                basf_rtsp_iterate_sessions(*rtsp, *demux);
            } else {
                BKNI_Free(ctx); ctx = NULL;
                basf_rtsp_close_session(*rtsp, *demux);
                *rtsp = NULL; *demux = NULL;
            }
        }
    } else BDBG_WRN(("could not open rtsp session"));
    
	return (ctx);
}

/* close the session, free stuff here */
int 
basf_rtsp_client_close(const basf_rtp_context_t ctx)
{
    int ret = 0;

    BDBG_ASSERT((ctx) && (ctx->cfg));
    BDBG_ASSERT(ctx->cfg->out.asf);
    
    if(ctx != NULL) {
       if(ctx->cfg != NULL) {
           BDBG_ASSERT(ctx->cfg->demux);
           basf_rtp_destroy_timer(ctx->cfg->demux);
       }
       if(ctx->cfg->out.mask & basf_rtp_output_file) {
            basf_rtsp_client_fclose(ctx);
            BKNI_Free(ctx);
       }
    }

    return (ret);
}

int
basf_rtsp_out_dispatch(const basf_rtp_context_t ctx, basf_rtp_info* desc, bool bheader)
{
    int feed_len = 0;
    basf_rtp_config_t cfg;
    
    BDBG_ASSERT(ctx); 
    BDBG_ASSERT((desc->chunk) && (desc->size != 0));
    
    basf_rtp_output* out;
    unsigned char* data = desc->chunk;      /* save ptr to buffer as it came in */
    unsigned size = desc->size;

    BSTD_UNUSED(out);    
    if((ctx) && (desc->chunk)  && (desc->size != 0)) {
        cfg = ctx->cfg;
        if((cfg != NULL) && (cfg->out.mask != 0)) {
            if(cfg->out.mask & (unsigned int)basf_rtp_output_file) {
                feed_len = basf_rtsp_client_write(ctx, desc->chunk, desc->size);/* output to file */
            }
            if (cfg->out.mask & (unsigned int)basf_rtp_output_fd) {             /* output to pipe */
                /* TODO: implement output to fd */
            }
#if 1
            if ((cfg->out.mask & (unsigned int)basf_rtp_output_cust)){           /* output to custom function */
                if(cfg->out.fn_mux) cfg->out.fn_mux(NULL, cfg->out.fn_mux_data, 
                                                    desc);                      /* for now */
            }
#endif

#ifdef __HAS_HDR_HANDLE_DISPATCH__
            if((cfg->out.mask & (unsigned int)basf_rtp_output_asf) && bheader) {
                feed_len = basf_feed(cfg->out.asf, data, size, data);           /* output to asf demux */
            }            
#endif /* __HAS_HDR_HANDLE_DISPATCH__ */
        }
    }

    return (feed_len);
}

/* set the link here */
void 
basf_rtsp_config_link (basf_rtp_config_t cfg, const char* link)
{
    int len;

    BDBG_ASSERT((cfg) && (link));
    if((len = strlen(link)) <= MAX_LINK_LEN) {
        strncpy(cfg->link, link, len);
    } else {
        BDBG_WRN(("link was not set, out of bounds"));
    }
}

/* set the transport type, tcp or udp */
void 
basf_rtsp_config_ip_type (basf_rtp_config_t cfg, basf_rtp_ip_type ip_type)
{
    BDBG_ASSERT((cfg));

    switch(ip_type) {
        case basf_rtp_ip_udp:
            BDBG_MSG_TRACE(("setting IP TYPE : UDP"));
            break;
        case basf_rtp_ip_tcp: 
            BDBG_MSG_TRACE(("setting IP TYPE : TCP"));
            break;
        default:
            BDBG_WRN(("unknown IP type: %d", ip_type));
            break;
    }
    
    cfg->ip_type = ip_type;
}


