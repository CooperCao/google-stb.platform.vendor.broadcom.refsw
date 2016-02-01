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

#ifndef __BASF_RTP_PROC_H__
#define __BASF_RTP_PROC_H__

/* local includes */
#include <stdio.h>

/* brcm stb includes */
#include "bsettop.h"    
#include "bsettop_os.h"
#include "basf_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_FOUT            (NULL)          /* default file out */
#define BASF_FALSE              (0)
#define BASF_TRUE               (1)

#define RTP_OFFSET_MARK         (4)             /* offset into asf rtp hdr when m on */
#define RTP_OFFSET_NOMARK       (4)             /* offset into asf rtp hdr when m off */
#define RTP_OFFSET              (12)            /* offset into ip hdr */
#define RTP_PACKET_COUNT        (8)             /* number of packets to hold in the buffer */

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG)
#define BASF_RTP_FOURCC(a,b,c,d)(((uint32_t)d)|(((uint32_t)c)<<8)|  \
                                 (((uint32_t)b)<<16)|(((uint32_t)a)<<24))
#else   /* BSTD_ENDIAN_LITTLE */
#define BASF_RTP_FOURCC(a,b,c,d)(((uint32_t)a)|(((uint32_t)b)<<8)|  \
                                 (((uint32_t)c)<<16)|(((uint32_t)d)<<24))
#endif

/* forward decls for opaque structs */
typedef struct basf_rtp_context* basf_rtp_context_t;
typedef struct basf_rtp_config* basf_rtp_config_t;
typedef struct basf_rtp_output* basf_rtp_output_t;
typedef struct basf_feed_buffer* basf_feed_buffer_t;

/* rtp feed context */
typedef struct basf_rtp_feed_ctx {
    bdecode_t decode;                   /* decoder instance */
    bplaypump_t playpump;               /* playpump instance */
    basf_feed_buffer_t buff;            /* global buffer struct */
} basf_rtp_feed_ctx;

/* rtp feed info */
typedef struct basf_rtp_info {
    unsigned char* chunk;               /* rtp data chunk */
    unsigned size;                      /* size of the chunk */
    unsigned int bitrate;               /* bitrate for this stream */
    bool commit_buffers;                /* do commit */
    unsigned int packet_size;           /* size of an asf data packet */
    unsigned int timestamp;             /* current timestamp */
} basf_rtp_info;

/* possible error codes enumeration */
typedef enum basf_rtp_err {
    basf_rtp_stream_read_failed,        /* stream read failed */
    basf_rtp_err_fail,                  /* no context failure */
    basf_rtp_err_success                /* no context success */
} basf_rtp_err;

/* opaque struct decls */
typedef struct basf_rtsp_session* basf_rtsp_session_t;
typedef struct basf_rtsp_demux* basf_rtsp_demux_t;
typedef struct basf_rtp_packet* basf_rtp_packet_t;
typedef struct basf_rtp_feed_ctx* basf_rtp_feed_ctx_t;

/* enum that defines the type of event we're getting */
typedef enum basf_rtsp_handler_type {
    basf_rtsp_handler_header,   /* header packet handler */
    basf_rtsp_handler_packet    /* rtp packet handler */
} basf_rtsp_handler_type;   

/* defines possible rtp packet types as defined by asf/ip*/
typedef enum basf_rtsp_pktype {
    basf_rtsp_pktype_audio,     /* audio session */
    basf_rtsp_pktype_video,     /* video session */
    basf_rtsp_pktype_app,       /* application session */
    basf_rtsp_pktype_unk        /* no rtp session (header) */
} basf_rtsp_pktype;

/* struct holding handler context information */
typedef struct basf_rtsp_handler_ctx {
    basf_rtsp_pktype pk_type;       /* a/v/app */
    basf_rtsp_handler_type hdl_type;/* header or packet */
} basf_rtsp_handler_ctx;

typedef enum basf_rtp_output_mask {
    basf_rtp_output_asf = 0x01,  /* direct output to demux */
    basf_rtp_output_file= 0x02,  /* direct output to file */
    basf_rtp_output_fd  = 0x04,  /* direct output to file descr. */
    basf_rtp_output_cust= 0x08   /* direct output to custom func */
} basf_rtp_output_mask;     

/* define maximums here */
typedef enum basf_max_enum {
    MAX_LINK_PROTO = 16,    /* max proto within a link */
    MAX_LINK_PATH  = 240,   /* max path within a link */
    MAX_LINK_LEN   = 256    /* maximum link length allowed */
} basf_max_enum;

/* use mux function */
/* typedef int (*basf_rtp_mux_func)(basf_rtsp_handler_ctx* ctx, void* client_data, 
                                 unsigned char* packet, unsigned size,
                                 unsigned int agg_bitrate, bool commit);
*/
/* use mux function */
typedef int (*basf_rtp_mux_func)(basf_rtsp_handler_ctx* ctx, void* client_data, 
                                 basf_rtp_info* desc);

/* describes the rtp output */
typedef struct basf_rtp_output {    
    int fd;                     /* to a file descriptor */
    basf_parser_t asf;                 /* to the asf demux */    
    unsigned int mask;          /* mask descriptor */
    const FILE* fout;           /* directly to file */
    basf_rtp_mux_func fn_mux;   /* custom mux func */
    void* fn_mux_data;          /* custom mux func */
} basf_rtp_output;

/* describes ip transport type */
typedef enum basf_rtp_ip_type {
    basf_rtp_ip_udp,            /* use udp unicast */
    basf_rtp_ip_tcp             /* use tcp/rtsp interleaved */
} basf_rtp_ip_type;

/* config parameters */
typedef struct basf_rtp_config {
    basf_rtp_output out;        /* output mux */
    basf_rtp_context_t ctx;     /* parent context */
    char link[MAX_LINK_LEN];    /* external rtsp link */
    basf_rtsp_demux_t demux;    /* backref to demux */
    basf_rtsp_session_t rtsp;   /* backref to session */
    int packet_size;            /* size of a packet */
    int packet_count;           /* number of packets */
    basf_rtp_ip_type ip_type;   /* use rtsp/tcp */
} basf_rtp_config;

/* asf feed descriptor */
typedef struct basf_rtp_feed_descriptor {
    unsigned int audio, audio_id;   /* audio stream id and pid */
    unsigned int video, video_id;   /* video stream id and pid */
    int quiet, nowait;              /* outside settings */
    const char* link, *pes, *file, *ts; /* in/out file config */
    const char* msg;
} basf_rtp_feed_descriptor, 
 *basf_rtp_feed_descriptor_t;

typedef struct basf_ip_headers* basf_ip_headers_t;

/* open an rtsp session */
int basf_rtsp_open_session (
    basf_rtsp_session_t* rtsp,      /* [out] */ 
    basf_rtsp_demux_t* demux,       /* [out] */
    basf_rtp_config_t cfg           /* [in] */
    );
/* send rtsp options to wm encoder server */
int basf_rtsp_send_options (
    basf_rtsp_session_t session,    /* [in] */ 
    basf_rtsp_demux_t demux         /* [in] */
    );
/* loop through sessions, do setup and play */
int basf_rtsp_iterate_sessions (
    basf_rtsp_session_t rtsp,       /* [in] */ 
    basf_rtsp_demux_t demux         /* [in] */
    );
/* close the rtsp session */
int basf_rtsp_close_session (
    basf_rtsp_session_t rtsp,       /* [in] */
    basf_rtsp_demux_t demux         /* [in] */
    );
/* extract header structure from the hdr, to get the size of the packet */
int basf_rtsp_get_header (
    basf_rtsp_demux_t demux,        /* [in] */
    basf_ip_headers_t* asf_hdr      /* [out] */
    );
/* start the demux instance */
int basf_rtsp_run_demux (
    basf_rtsp_demux_t demux         /* [in] */
    );
/* install rtp channel callback through here */
int basf_rtsp_sethandler (
    basf_rtsp_demux_t demux,        /* [in] */
    basf_rtp_mux_func handler,      /* [in] */
    void* client_data               /* [in] */
    );
/* get packet size and count for the rtp processor */
void basf_rtsp_get_packet_info (
    basf_rtsp_demux_t demux,        /* [in] */
    int* packet_size,               /* [out] */
    int* packet_count               /* [out] */
    );
/* set out to file descriptor */
void basf_rtsp_out_set_fd (
    const basf_rtp_config_t cfg,    /* [in] */
    int fd                          /* [in] */
    );
/* set output to file out */
void basf_rtsp_out_set_fout (
    const basf_rtp_config_t cfg,    /* [in] */
    const FILE* fout                /* [in] */
    );
/* set output to asf demux */
void basf_rtsp_out_set_asf_demux (
    const basf_rtp_config_t cfg,    /* [in] */
    const basf_parser_t asf         /* [in] */
    );
/* set custom demux */
void basf_rtsp_output_set_cust_demux (
    const basf_rtp_config_t cfg,    /* [in] */
    const basf_rtp_mux_func fn,     /* [in] */
    void* fn_data                   /* [in] */
    );
/* configure the rtp session */
void basf_rtsp_default_config (
    basf_rtp_config_t* cfg          /* [out] */
    );
/* configure the external link */
void basf_rtsp_config_link (
    basf_rtp_config_t cfg,          /* [in] */
    const char* link                /* [in] */
    );
void basf_rtsp_config_ip_type (
    basf_rtp_config_t cfg,          /* [in] */
    basf_rtp_ip_type type           /* [in] */
    );
/* start rtsp session */
basf_rtp_context_t basf_rtsp_client_open (    
    basf_rtsp_session_t* rtsp,      /* [out] */
    basf_rtsp_demux_t* demux,       /* [out] */
    const basf_rtp_config_t cfg     /* [in] */
    );
/* close the session, free stuff here */
int basf_rtsp_client_close (
    basf_rtp_context_t ctx          /* [in] */    
    );
/* dump asf to a file */
int basf_rtsp_client_write (
    basf_rtp_context_t ctx,         /* [in] */    
    unsigned char* buff,            /* [in] */    
    int len                         /* [in] */    
    );
/* rtp packet handler */
int basf_rtsp_aux_rtphandler (
    basf_rtsp_handler_type type,    /* [in] */    
    void* clientData,               /* [in] */    
    unsigned char* packet,          /* [in] */    
    unsigned packetSize             /* [in] */    
    );
/* set accessor function to rtp timer */
void basf_rtsp_set_rtptimer (
    basf_rtsp_demux_t demux,        /* [in] */
    b_timer_t rtp_timer             /* [in] */
    );
/* get accessor function to rtp timer */
b_timer_t basf_rtsp_get_rtptimer (
    basf_rtsp_demux_t demux         /* [in] */
    );
/* entry point for the rtp feeder */
int basf_rtp_feed_entrypoint (
    basf_rtp_mux_func fn,           /* [in] */
    basf_rtp_feed_ctx_t feed_ctx,   /* [in] */
    basf_rtp_feed_descriptor_t desc,/* [in] */
    basf_rtp_ip_type rtp_ip_type    /* [in] */
    );
/* update the rtp session stats */
void basf_rtsp_update_stats (
    basf_rtsp_demux_t demux,    /* [in] */
    int pkts_lost,              /* [in] */
    int pkts_valid,             /* [in] */
    int plds_lost,              /* [in] */
    int plds_dropped,           /* [in] */
    int plds_valid,             /* [in] */
    int plds_pack               /* [in] */
    ); 
    
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BASF_RTP_PROC_H__ */
