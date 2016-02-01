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
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

/* BRCM refsw depends */
#include "berr.h"
#include "bkni.h"
#include "bstd.h"
#include "bdbg.h"

/* asf ip common files */
#include "bsettop.h"
#include "basf_parser.h"
#include "basf_rtp_proc.h"
#include "asfpars.h"
#include "basf_hdr_common.h"

/* livemedia includes */
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

/* wmv version info */
#define WMV_SERVER_OK                   (0)
#define WMV_SERVER_UNSUPPORTED          (1)
#define WMV_SUPPORTED_MAJOR_VERSION     (9)
#define WMV_SUPPORTED_MINOR_VERSION     (1)

/* defaults */
#define BASF_RTP_HDR_OFFSET             (4)
#define BASF_RTSP_HTTP_DEFAULT_PORT     (80)
#define BASF_RTP_DEFAULT_PORT           (19778)
#define BASF_RTP_BUFFER_SIZE            (0x10000)
#define BASF_RTP_WATCHDOG_TIMEOUT       (8*1000)

/* livemedia buffer lengths */
#define BASF_RTP_AUDIOAPP_BUFFER_LEN    (32*4*1024)     /* 100 */ 
#define BASF_RTP_VIDEO_BUFFER_LEN       (2*1024*1024)   /* 2000 */

/* closing states */
#define BASF_RTP_STATE_CLOSING          (~0x00)
#define BASF_RTP_STATE_READING          (0x01)
#define BASF_RTP_STATE_START            (0x00)

/* defines for idle times */
#define BASF_RTP_MCAST_IDLETIME         (120)
#define BASF_RTP_NONMCAST_IDLETIME      (60)

#define BASF_RTSP_TOKEN             "rtsp"          /* string tokens */
#define BASF_RTP_MIME_AV            "X-ASF-PF"      /* mime tokens */
#define BASF_RTP_MIME_APP           "X-WMS-RTX"

#define BASF_RTSP_HDR_DATA          "data:"
#define BASF_RTSP_HDR_PGMPU         "pgmpu:"
#define BASF_RTSP_HDR_BASE64        "base64"
#define BASF_RTSP_HDR_WMSASF_APP    "application"
#define BASF_RTSP_HDR_WMSASF_V1     "vnd.ms.wms-hdr.asfv1"   

#define BASF_RTP_MEDIUM_VIDEO       "video"         /* medium tokens */
#define BASF_RTP_MEDIUM_AUDIO       "audio"
#define BASF_RTP_MEDIUM_APP         "application"   

#define BASF_RTSP_BRCM_PLAYER       "asfip player"  /* player alias */
#define BASF_RTSP_WMV_SUPP_SVR      "WMServer"      /* server alias */

/* bitrate calculation */
#define RATE(val,time) ((float)(val)/((float)(time)->tv_sec+(time)->tv_usec*(0.000001)))

/* calculate rtsp session timeout */
#define BASF_RTSP_TIMEOUT_CALC(session)((session)->rtsp_timeout != 0 ?  \
                                        (session)->rtsp_timeout*(1000): \
                                        BASF_RTP_WATCHDOG_TIMEOUT)
/* debug info */
BDBG_MODULE(basf_rtsp_sess);                              
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

#ifdef PROFILE
    #undef BDBG_MSG
    #define BDBG_MSG(x)
#endif

/* session description context
*/
typedef struct basf_rtsp_session {
    basf_ip_headers headers;        /* asf headers */
    basf_rtsp_demux* demux;         /* backptr to demux */
    basf_rtp_channel** line;        /* asf streams descriptor */
    b_timer_t rtsp_timer,end_timer; /* session timer */
    char* sdp_data;                 /* contents of the sdp session/file */
    char rtsp_path[MAX_LINK_PATH];  /* rtsp server path */
    int rtsp_timeout;               /* rtsp timeout from the server */
    bool idle_data;                 /* do we have idle data ? */
    struct basf_rtsp_livemedia {
        MediaSession* ms;               /* media session instance */
        TaskScheduler* scheduler;       /* scheduler instance */
        BasicUsageEnvironment* env;     /* environment var */
        RTSPClient* rtsp;               /* client instance for rtsp */
        MediaSubsessionIterator* iter;  /* iterator for sub-sessions */
        MediaSubsession* sub;           /* sub-session a/v */    
        char state;                     /* closing state */
    } lm;    
    basf_rtp_config_t cfg;
    int64_t acc_file_size;              /* accumulated file size */
    bool closing, rtcp_sync;            /* signals closure */
    int32_t idle_count;                 /* number of idle times */
    int64_t total_length;
    int stream_count;
    bool multicast;                     /* is this a multicast session ? */
} basf_rtsp_session;

typedef struct basf_rtsp_demux {
    b_timer_t rtp_timer;                /* main rtp timer */
    char slink[MAX_LINK_PROTO];         /* name of the demux */
    char* rtsp_path;                    /* path to server resource */
    unsigned int id;                    /* demux identifier */
    basf_rtp_stream* stream;            /* pointer to output stream */    
    basf_rtsp_session *session;         /* pointer to parent session */
    struct {
        int remote_rtsp_port;   /* outgoing http port here */
        int local_rtp_port;     /* client instance */
        bool using_http;        /* use http for rtsp */
        bool using_tcp;         /* use tcp for rtsp */
        bool using_ks;          /* use kasenna server livemedia options */
    } rtsp_params;
    struct {                        /* aggregate struct to save binary asf hdr */
        unsigned char* p_header;    /* asf header after uudec */
        int p_header_size;          /* asf header size after uudec */
    } asf_dmx_header;               
    basf_rtp_mux_func handler;      /* handler from livemedia and backwards*/
    void* handler_data;             /* backwards handler data */
    basf_props_t props;             /* header properties structure */
    unsigned char* out;             /* output pipe */
    bool error;                     /* do we have err ? */
    struct {
        int pkts_lost, pkts_valid;      /* total lost, valid packets */
        int plds_lost;                  /* total lost payloads */
        int plds_dropped, plds_total;   /* total, dropped packets */
        float plds_pack;                /* average payloads in an asf packet */
    } stats;
    basf_rtp_channel* curr_channel;     /* current channel (needed in rtp handler) */
} basf_rtsp_demux;

/* local forward decls 
*/
int basf_rtsp_destroy_instance (basf_rtsp_demux_t demux);
void basf_rtsp_stream_close (void* client_data);
basf_rtp_stream* basf_rtp_stream_demux_new (basf_rtsp_demux_t demux, char* sdemux, unsigned char* out);
int basf_rtp_stream_read (basf_rtp_stream* s, void* read, int nread);
basf_rtp_err basf_rtsp_parse_hdr (basf_rtsp_demux* demux);
basf_rtp_err basf_rtsp_map_to_demux (basf_rtsp_demux* demux, basf_rtsp_session* session, 
    MediaSubsessionIterator* iter, MediaSubsession* sub);
int basf_rtsp_create_instance (basf_rtsp_session_t* rtsp, basf_rtsp_demux_t* demux, 
    basf_rtp_config* cfg);
void basf_rtp_watchdog_timeout(void *client_data);
void basf_rtp_watchdog_endclose(void *client_data);

/* open an rtsp session */
int 
basf_rtsp_open_session(basf_rtsp_session_t* rtsp, basf_rtsp_demux_t* demux, const basf_rtp_config_t cfg)
{
    int ret = 0;

    BDBG_ASSERT(cfg);
    if (cfg) {
        ret = basf_rtsp_create_instance(rtsp, demux, cfg);
    } else { BDBG_WRN(("invalid config in open session"));
    }

    return (ret);
}

/* close the rtsp session */
int 
basf_rtsp_close_session(basf_rtsp_session_t rtsp, basf_rtsp_demux_t demux)
{   
    int ret = 0;

    BDBG_ASSERT(rtsp); 
    BDBG_ASSERT(demux);

    if ((rtsp) && (demux)) {
        ret = basf_rtsp_destroy_instance(demux);
    } else { BDBG_WRN(("invalid rtsp, demux in close session"));
    }

    return (ret);
}

/* returns the path part out of an rtsp link */
static inline int
basf_rtsp_get_path(const char* link, char* path)
{
    int ret = 0, path_len = 0;
    const char* rtsp_token = NULL;

    if ((link) && (path)) {
        rtsp_token = strstr(link, BASF_RTSP_TOKEN);
        if (rtsp_token != NULL) {
            path_len = strlen(link) - strlen(BASF_RTSP_TOKEN);
            strcpy(path, link + strlen(BASF_RTSP_TOKEN) + 3);    /* account for extra :// */
        } else { 
            ret = -1;
        }
    }
    return (ret);
}
/* returns the protocol part out of an rtsp link */
static inline int
basf_rtsp_get_proto(const char* link, char* proto)
{
    int ret = 0;
    const char* rtsp_token = NULL;
    if (link && proto) {
        rtsp_token = strstr(link, BASF_RTSP_TOKEN);
        if (rtsp_token != NULL) {
            strncpy(proto, rtsp_token, strlen(BASF_RTSP_TOKEN));
        } else {
            ret = -1;
        }
    }

    return (ret);
}

/* initialize the statuses in the demux instance */
static void
basf_rtsp_demux_stats_init(basf_rtsp_demux_t demux)
{
    BDBG_ASSERT(demux);
    if (demux) {        
        demux->stats.pkts_lost = 0;
        demux->stats.pkts_valid = 0;
        demux->stats.plds_lost = 0;        
        demux->stats.plds_dropped = 0;
        demux->stats.plds_total = 0;
        demux->stats.plds_pack = 0.0;
    }
}

/* initialize the demux instance */
static int 
basf_rtsp_demux_init(basf_rtsp_demux_t demux, basf_rtsp_session_t rtsp)
{
    int ret = 0;

    BDBG_ASSERT(demux);
    if (demux) {
        (demux)->rtsp_params.local_rtp_port = BASF_RTP_DEFAULT_PORT;
        (demux)->rtsp_params.using_http = 0;
        (demux)->rtsp_params.remote_rtsp_port = BASF_RTSP_HTTP_DEFAULT_PORT;
        (demux)->rtsp_params.using_ks = BASF_FALSE;
        (demux)->rtsp_params.using_tcp = (rtsp->cfg->ip_type==basf_rtp_ip_tcp) ? 1 : 0;    
        (demux)->session = rtsp;        
        (demux)->error = BASF_FALSE;         
        (demux)->handler = NULL;
        (rtsp)->demux = demux;

        basf_rtsp_demux_stats_init(demux);
        basf_rtsp_get_proto(rtsp->cfg->link, demux->slink);
    }

    return (ret);
}

static inline void
basf_rtsp_session_init_headers(basf_rtsp_session_t rtsp)
{
    BDBG_ASSERT(rtsp);
    if (rtsp) {
        (rtsp)->headers.pstream = NULL;
        (rtsp)->headers.pcontent = NULL;
        (rtsp)->headers.pfile = NULL;
        (rtsp)->headers.pbitrate = NULL;
        (rtsp)->headers.pcodec = NULL;
    }
}

/* initialize livemedia session variables 
*/
static inline void 
basf_rtsp_livemedia_init(basf_rtsp_session_t rtsp)
{
    BDBG_ASSERT(rtsp);
    if (rtsp) {
        (rtsp)->lm.env = NULL;
        (rtsp)->lm.ms = NULL;
        (rtsp)->lm.rtsp = NULL;
        (rtsp)->stream_count = 0;
        (rtsp)->line = NULL;
    }
}

/* initialize entire session params */
static inline int 
basf_rtsp_session_init(basf_rtsp_session_t rtsp)
{
    int ret = 0;

    BDBG_ASSERT(rtsp);
    if (rtsp) {
        (rtsp)->sdp_data = NULL;                                /* initialize generics */
        (rtsp)->total_length = 0;
        (rtsp)->idle_data = BASF_TRUE;
        (rtsp)->idle_count = 0;
        (rtsp)->rtsp_timeout = 0;
        (rtsp)->multicast = BASF_FALSE;
        (rtsp)->acc_file_size = 0;
        (rtsp)->closing = BASF_FALSE;        

        ret = basf_rtsp_get_path(rtsp->cfg->link, rtsp->rtsp_path); /* obtain the rtsp path */
        basf_rtsp_livemedia_init(rtsp);                             /* initialize livemedia vars */
        basf_rtsp_session_init_headers(rtsp);                       /* initialize asf headers */
    }

    return (ret);
}

int 
basf_rtsp_demux_create(basf_rtsp_session_t* rtsp, basf_rtsp_demux_t* demux, basf_rtp_config* cfg)
{
    int ret = 0;

    BDBG_ASSERT(rtsp); BDBG_ASSERT(demux);

    if ((rtsp) && (demux)) {
        (*demux) = (basf_rtsp_demux_t)BKNI_Malloc(sizeof(basf_rtsp_demux));
        (*rtsp) = (basf_rtsp_session_t)BKNI_Malloc(sizeof(basf_rtsp_session));
        (*rtsp)->cfg = cfg;

        basf_rtsp_demux_init((*demux), (*rtsp));    /* start demux */
        basf_rtsp_session_init((*rtsp));            /* start session */

    } else { BDBG_WRN (("invalid demux create parameters"));
    }

    return (ret);
}

/* this is where we create the scheduler, env. and rtsp client
*/
int 
basf_rtsp_create_instance(basf_rtsp_session_t* prtsp, basf_rtsp_demux_t* pdemux, basf_rtp_config* cfg)
{
    basf_rtsp_session* rtsp; 
    basf_rtsp_demux* demux;
    int http_port = 0, ret = 0;

    basf_rtsp_demux_create(prtsp, pdemux, cfg);    
    rtsp = (*prtsp); demux = (*pdemux);

    /* create lm task scheduler */
    if ((rtsp->lm.scheduler = BasicTaskScheduler::createNew()) == NULL) {
        BDBG_ERR(("task scheduler creation failed"));
        goto error;
    } else {
        BDBG_MSG_TRACE(("task scheduler creation succeeded"));
    }
    /* create lm basic usage environment */
    if (!( rtsp->lm.env = BasicUsageEnvironment::createNew(*rtsp->lm.scheduler))) {
        BDBG_ERR(("usage environment creation failed"));
        goto error;
    } else { 
        BDBG_MSG(("usage environment creation succeeded"));
    }
    /* look for "rtsp" type-links */
    if (demux->stream == NULL && !strcasecmp(demux->slink, "rtsp")) {
        if (demux->rtsp_params.using_http)
            http_port = demux->rtsp_params.remote_rtsp_port;

        /* create a livemedia client for it */
        if ((rtsp->lm.rtsp = RTSPClient::createNew(*rtsp->lm.env, 1, BASF_RTSP_BRCM_PLAYER, http_port)) == NULL) {
            BDBG_ERR(("RTSPClient::createNew failed (%s)", rtsp->lm.env->getResultMsg()));
            goto error;
        } else {
            BDBG_MSG(("created rtsp client\n"));
        }
    }
error:    
    /* error handling here */

    return (ret);
}

/* look at 'server' field and validate - WMServer/9.1.1.3814 */
static int 
basf_rtsp_validate_server(basf_rtsp_session* session)
{
    char* svr_vers, *svr_maj, *svr_min;
    const char vers_delim[] = ".";
    int vmaj = 0, vmin = 0, supp_len = 0;

    char* svr_type = session->lm.rtsp->serverType();
    int supported_server = WMV_SERVER_UNSUPPORTED;
    
    if (svr_type != NULL) {
        supp_len = strlen(BASF_RTSP_WMV_SUPP_SVR);
        if (strncmp(svr_type, BASF_RTSP_WMV_SUPP_SVR, supp_len) == 0) {
            
            svr_vers = svr_type+supp_len+1;                 /* go to the version */
            svr_maj = strtok(svr_vers, vers_delim);         /* get major */
            
            if (svr_maj) vmaj = atoi(svr_maj);
            svr_min = strtok(NULL, vers_delim);             /* get minor */
            
            if (svr_min) vmin = atoi(svr_min);
            if ((vmaj>WMV_SUPPORTED_MAJOR_VERSION) ||        /* either maj > */
               ((vmaj==WMV_SUPPORTED_MAJOR_VERSION) &&      /* or maj =, min > */
                (vmin>=WMV_SUPPORTED_MINOR_VERSION))) {
                    supported_server = WMV_SERVER_OK;
            } else {
                BDBG_ERR(("unsupported server version %d.%d", svr_type));            
                BDBG_ASSERT(supported_server == WMV_SERVER_OK); /* will always assert false */
            }
        }            
        else {
            BDBG_ERR(("unsupported server type is : %s", svr_type));            
            BDBG_ASSERT(supported_server == WMV_SERVER_OK);     /* will always assert false */
        }
    } else { BDBG_WRN(("could not retreive server type"));
    }
    return (supported_server);
}

/* send rtsp options to wm encoder server */
int 
basf_rtsp_send_options(basf_rtsp_session_t session, basf_rtsp_demux_t demux)
{
    unsigned char* sdp;
    char *surl, *soptions;
    int ret = -1;

    BDBG_ASSERT(session); BDBG_ASSERT(demux);
    if ((demux->stream == NULL) && !strcasecmp(demux->slink, "rtsp")) {        

        surl = (char*)BKNI_Malloc(strlen(session->rtsp_path)+8);
        sprintf(surl, "rtsp://%s", session->rtsp_path); /* copy link to buffer */
        soptions = session->lm.rtsp->sendOptionsCmd(surl);
        if (soptions) delete [] soptions;

        sdp = (unsigned char*)session->lm.rtsp->describeURL(surl, NULL, demux->rtsp_params.using_ks);
        if (sdp == NULL) {                               /* handle sdp session here */
            BDBG_ERR(("describeURL failed (%s)", session->lm.env->getResultMsg()));
            BKNI_Free(surl);
            goto error;
        } else {
            if (WMV_SERVER_OK != basf_rtsp_validate_server(session)) {
                BDBG_ASSERT((false)); goto error;
            } else {
                ret = 0;                                /* successful describe */
            }
        }        
        BKNI_Free(surl);                                /* free up the link, print sdp data */
        session->sdp_data = strdup((char*)sdp);       
        delete [] sdp;
        BDBG_MSG_TRACE(("sdp=%s\n", session->sdp_data));
    }
    else if (demux->stream == NULL && !strcasecmp(demux->slink, "sdp")) {
        session->sdp_data = strdup(session->rtsp_path);
        /* TODO: implement getting the sdp data from local file */
    } else {                                            /* or read from an sdp file */
        ret = basf_rtp_stream_read_failed;
        BDBG_ERR(("failed to read sdp from %s, not supported", session->rtsp_path));
        BKNI_Free(session);        
        BDBG_MSG(("sdp=%s\n", session->sdp_data));
        goto error;
    }
    if (!(session->lm.ms = MediaSession::createNew(*session->lm.env, session->sdp_data))) {
        BDBG_ERR(("could not create new session"));
        goto error;
    }
    else { BDBG_MSG(("session creation succeeded"));
    }

error:
    /* error handling here */
    return (ret);
}

static bool
basf_rtp_initialize_subsession(MediaSubsession* sub, basf_rtsp_demux* demux, 
                               unsigned int* buff_len)
{
    bool init = false;
    int client_port;

    BDBG_ASSERT((demux) && (buff_len));
    if ((!demux) || (!buff_len))
        goto error;

    if ((client_port = demux->rtsp_params.local_rtp_port) != -1) {
        sub->setClientPortNum(client_port);
    }
    if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_AUDIO) ||
            !strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_APP))        /* value taken from wmplayer scan */
        *buff_len = BASF_RTP_AUDIOAPP_BUFFER_LEN;
    else if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_VIDEO))      /* set video buffer size */
        *buff_len = BASF_RTP_VIDEO_BUFFER_LEN;
    else {
        BDBG_WRN(("unknown session type"));                             /* session type not recognized */
        goto error;
    }            
    BDBG_MSG_TRACE(("CODECNAME = %s\n", sub->codecName()));
    if (!strcasecmp(sub->codecName(), BASF_RTP_MIME_AV)) 
        init = sub->initiate(BASF_RTP_HDR_OFFSET); 
    else if (!strcasecmp(sub->codecName(), BASF_RTP_MIME_APP))           /* indicate valid subsession */
        init = true;                                        
    else
        init = sub->initiate(0);
error:
    BDBG_MSG_TRACE(("RTP subsession '%s/%s' %s\n", sub->mediumName(), sub->codecName(),
                init ? "succeeded":"failed"));
    return (init);
}

/* loop through sessions, do setup and play */
int 
basf_rtsp_iterate_sessions (basf_rtsp_session* rtsp, basf_rtsp_demux* demux)
{
    MediaSubsessionIterator* iter;
    MediaSubsession* sub;
    RTPSource* source = NULL;
    Groupsock* group_sock = NULL;

    int ret = 0;
    static int media_session_count = 0;    
    unsigned int buff_len = 0;
    bool init = false, rtsp_tcp = demux->rtsp_params.using_tcp;

    if (!rtsp->lm.rtsp) {                                        /* initialise each media subsession */
        BDBG_ASSERT(false);
        BDBG_MSG(("rtsp not set on iterate"));
        goto error;
    }
    iter = new MediaSubsessionIterator(*rtsp->lm.ms);
    while((sub = iter->next()) != NULL) {
        
        BDBG_MSG(("iteration medium: %s\n", sub->mediumName()));
        buff_len = 0; media_session_count++;                    /* count media sessions */

        if ((init = basf_rtp_initialize_subsession(sub, demux, &buff_len))) {
            source = sub->rtpSource();
            if (source != NULL) {
                group_sock = source->RTPgs();
                if (group_sock != NULL) {                       /* increase the buffer size */
                    increaseReceiveBufferTo(*rtsp->lm.env, group_sock->socketNum(), buff_len);
                }
            }
            BDBG_MSG_TRACE(("RTP subsession '%s/%s'\n", 
                            sub->mediumName(), sub->codecName()));
            if (rtsp->lm.rtsp) {                                 /* issue the SETUP */
                rtsp->lm.rtsp->setupMediaSubsession(*sub, false, rtsp_tcp, false);   
            } else {                                            /* force multicast for unknown */
                BDBG_WRN(("rtsp->rtsp is invalid for this session"));
                goto error;
            }
            if (!rtsp->multicast) {                
                rtsp->multicast = IsMulticastAddress(           /* for mcast rollover (TODO:) */
                        sub->connectionEndpointAddress());
            }
        }
    }
    if (rtsp->lm.rtsp) {
        if (!rtsp->lm.rtsp->playMediaSession(*rtsp->lm.ms)) {    /* send PLAY command */
            BDBG_ERR(("PLAY failed %s", rtsp->lm.env->getResultMsg()));
            delete iter; goto error;
        }
        
        rtsp->rtsp_timeout = rtsp->lm.rtsp->sessionTimeoutParameter();
        if (rtsp->rtsp_timeout > 0) {                            /* read timeout value for session */
            BDBG_MSG_TRACE(("session timeout of %ds", rtsp->rtsp_timeout));
        }
    }

    /* this should be called from outside this function */
    basf_rtsp_map_to_demux(demux, rtsp, iter, sub);

error:
    /* error handling here */
    return (ret);
}

/* local rtp handler, from here on we just dispatch */
static inline void
basf_rtsp_rtp_handler(basf_rtsp_pktype pktype, basf_rtsp_demux* demux, unsigned char* packet, 
                      unsigned size)
{
    int32_t packets, packet_size;
    int64_t bitrate, file_size;
    basf_rtsp_handler_ctx ctx;

    static basf_rtp_info desc;
    bool final_commit, commit = false;
    bool pkt_limit, file_limit;

    BDBG_ASSERT(demux);
    BDBG_ASSERT(demux->handler);

    if ((demux) && (demux->handler)) {   /* assume sync call, otherwise ctx should be dyn-alloc-ed */
        ctx.pk_type = pktype;
        ctx.hdl_type = basf_rtsp_handler_packet;

        bitrate = basf_rtsp_hdr_get_bitrate(demux->props);
        file_size = basf_rtsp_hdr_get_filesize(demux->props);
        packets = basf_rtsp_hdr_get_pktcount(demux->props);
        packet_size = basf_rtsp_hdr_get_minpksize(demux->props);

        demux->session->acc_file_size += size-(RTP_OFFSET+RTP_OFFSET_MARK);     /* count out the rtp header */
        BDBG_MSG_TRACE(("file_size=%lld, file_size=%lld, data_pkts_count=%ld\n", 
                        (long)demux->session->acc_file_size, 
                        file_size, packets));

        desc.chunk = packet; desc.size = size;
        desc.bitrate = bitrate; desc.commit_buffers = commit;
        desc.packet_size = packet_size;
        demux->handler(&ctx, (void*)demux->handler_data, &desc);        

        if ((pkt_limit = (demux->stats.pkts_valid + demux->stats.pkts_lost >= packets-1))) {
            BDBG_ERR(("packet limit reached: valid:%d + lost:%d >= packets:%d",
                      demux->stats.pkts_valid, demux->stats.pkts_lost, packets));
        }
        if ((file_limit = (demux->session->acc_file_size >= file_size))) {
            BDBG_ERR(("file limit reached: accumulated:%lld >= file_size:%lld",
                      demux->session->acc_file_size, file_size));            
            BDBG_ERR(("packet limit is: valid:%d + lost:%d >= packets:%d",
                      demux->stats.pkts_valid, demux->stats.pkts_lost, packets));
        }
        if ((final_commit = (demux->session->acc_file_size >= file_size -
                            /*RTP_PACKET_COUNT*/1*packet_size))) {
                BDBG_ERR(("final commit reached: accumulated:%lld >= file_size:%lld",
                          demux->session->acc_file_size, file_size));
        }        
        
        /* only manage teardown for UDP */
        if ((pkt_limit || file_limit)  && !demux->rtsp_params.using_tcp ) {  /* TODO: fix this for UDP */
                demux->session->lm.state = BASF_RTP_STATE_CLOSING;
                demux->session->closing = BASF_TRUE;
                BDBG_WRN(("creating watchdog."));
                demux->session->end_timer = b_timer_schedule(BASF_RTSP_TIMEOUT_CALC(demux->session), 
                        basf_rtp_watchdog_endclose, (void*)demux->session);  
        }
    }
}

/* audio channel rtp callback */
void 
basf_aux_rtphandler_audio(void* clientData, unsigned char* packet, unsigned size)
{
    static int count = 0;
    basf_rtp_channel* channel;
    basf_rtsp_demux* demux;

    BDBG_MSG(("PACKET [A] [%d] %s HANDLER: %.2X %.2X %.2X %.2X size:%d\n", 
           count, (char*)clientData, packet[0], packet[1], packet[2], packet[3], size));

    channel = (basf_rtp_channel*)clientData; BDBG_ASSERT(channel);
    demux = channel->demux; BDBG_ASSERT(demux); 
    demux->curr_channel = channel;

    BDBG_ASSERT(demux->handler);
    if (demux && (demux->handler)) {
        basf_rtsp_rtp_handler(basf_rtsp_pktype_audio, demux, packet, size);
    }

    count++;
}

/* application channel rtp callback */
void 
basf_aux_rtphandler_app(void* clientData, unsigned char* packet, unsigned size)
{
    static int count = 0;
    basf_rtsp_demux* demux;
    basf_rtp_channel* channel;

    BDBG_MSG(("PACKET [P] [%d] %s HANDLER: %.2X %.2X %.2X %.2X size:%d\n", 
           count, (char*)clientData, packet[0], packet[1], packet[2], packet[3], size));

    channel = (basf_rtp_channel*)clientData; BDBG_ASSERT(channel);    
    demux = channel->demux; BDBG_ASSERT(demux); 
    demux->curr_channel = channel;

    BDBG_ASSERT(demux->handler);
    if (demux && (demux->handler)) {
        basf_rtsp_rtp_handler(basf_rtsp_pktype_app, demux, packet, size);
    }
    count++;
}

/* video channel rtp callback */
void 
basf_aux_rtphandler_video(void* clientData, unsigned char* packet, unsigned size)
{
    static int count = 0;
    basf_rtsp_demux* demux;
    basf_rtp_channel* channel;

    BDBG_MSG(("PACKET [V] [%d] HANDLER: %.2X %.2X %.2X %.2X size:%d\n", 
           count, packet[0], packet[1], packet[2], packet[3], size));

    channel = (basf_rtp_channel*)clientData; BDBG_ASSERT(channel);    
    demux = channel->demux; BDBG_ASSERT(demux); 
    demux->curr_channel = channel;
    BDBG_ASSERT(demux->handler);
    if (demux && (demux->handler)) {
        basf_rtsp_rtp_handler(basf_rtsp_pktype_video, demux, packet, size);
    }
    count++;
}

/* install rtp channel callback through here */
int
basf_rtsp_sethandler(basf_rtsp_demux_t demux, basf_rtp_mux_func handler, void* client_data)
{
    BDBG_ASSERT(demux); 
    BDBG_ASSERT(handler);

    if ((demux) && (handler)) {
        demux->handler_data = client_data;                /* client data goes backwards */
        demux->handler = (basf_rtp_mux_func)handler;      /* backwards handler */
    }

    return (0);
}

/* clean a given session, reclaim the environment, free vars */
void
basf_rtsp_clean_session(basf_rtsp_session* session)
{
    if (session) {
        if (session->lm.ms) {        /* close media session */
            Medium::close(session->lm.ms);
            session->lm.ms = NULL;
        }
        if (session->lm.rtsp) {      /* close rtsp client */
            Medium::close(session->lm.rtsp);
            session->lm.rtsp = NULL;
        }
        if (session->lm.env) {       /* reclaim initial environment */
            session->lm.env->reclaim();
        }
        if (session->lm.scheduler) { /* delete original scheduler instance */
            delete session->lm.scheduler;
            session->lm.scheduler = NULL;
        }
        if (session->sdp_data) {     /* free up sdp data */
            BKNI_Free(session->sdp_data);
            session->sdp_data = NULL;
        }
        BKNI_Free(session);         /* free up session */
   }
}

/* create and initialize a channel */
int
basf_rtsp_channel_init(basf_rtp_channel** channel, basf_rtsp_demux* demux)
{
    int ret = 0;
    BDBG_ASSERT(channel);

    if (channel) {
        (*channel) = (basf_rtp_channel*)BKNI_Malloc(sizeof(basf_rtp_channel));
        (*channel)->buff_len = BASF_RTP_BUFFER_SIZE;
        (*channel)->buff = (uint8_t*)BKNI_Malloc(BASF_RTP_BUFFER_SIZE);
        (*channel)->demux = demux;
        (*channel)->block = 0;
        (*channel)->calc_pts = 0;
        (*channel)->asf_channel = BASF_FALSE;
        (*channel)->rtcp_sync = BASF_FALSE;
    } else {
        ret = (-1);
        BDBG_ERR(("could not create new channel"));
    }

    return (ret);
}

/* looks at the sdp description and maps the sessions to the demux instance */
basf_rtp_err 
basf_rtsp_map_to_demux(basf_rtsp_demux* demux, basf_rtsp_session* 
                       session, MediaSubsessionIterator *iter, 
                       MediaSubsession *sub) 
{
    basf_rtp_err ret = basf_rtp_err_success;
    basf_rtp_channel* channel; 

    /* start enumerating sessions */
    iter->reset();
    while ((sub = iter->next()) != NULL) {
        BDBG_MSG(("POST ITERATION: %s\n", sub->codecName()));
        if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_APP)) continue;
        if (sub->readSource() == NULL) continue;

        /* initialize the a/v/app channels */
        if (-1 == basf_rtsp_channel_init(&channel, demux)) {
            /* always assert, to signal err */
            BDBG_ASSERT(false);
            break;
        }
        /* MS asf audio codec */
        if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_AUDIO)) {
            channel->aux_data = "a";
            sub->rtpSource()->setAuxilliaryReadHandler(basf_aux_rtphandler_audio, (void*)channel);
            channel->num_channels = sub->numChannels();
            channel->rate = sub->rtpTimestampFrequency();
            if (!strcasecmp(sub->codecName(), BASF_RTP_MIME_AV))
                channel->asf_channel = BASF_TRUE;
        }
        /* MS application codec */
        if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_APP)) {
            channel->aux_data = "p";
            sub->rtpSource()->setAuxilliaryReadHandler(basf_aux_rtphandler_app, (void*)channel);
            if (!strcasecmp(sub->codecName(), BASF_RTP_MIME_APP))
                channel->asf_channel = BASF_TRUE;
        }        
        /* MS ASF video codec */
        if (!strcasecmp(sub->mediumName(), BASF_RTP_MEDIUM_VIDEO)) {
            channel->aux_data = "v";
            sub->rtpSource()->setAuxilliaryReadHandler(basf_aux_rtphandler_video, (void*)channel);
            if (!strcasecmp(sub->codecName(), BASF_RTP_MIME_AV))
                channel->asf_channel = BASF_TRUE;
        }

        /* if we get an RTCP bye, close the stream */
        if (sub->rtcpInstance() != NULL)
            sub->rtcpInstance()->setByeHandler(basf_rtsp_stream_close, channel);

        /* re-read the sinks and allocate a new line */
        if (session && channel && channel->asf_channel) {
            channel->lm.frame_sink = sub->readSource();
            channel->lm.rtp_sink  = sub->rtpSource();
            session->line = (basf_rtp_channel**)realloc(session->line, sizeof(basf_rtp_channel)*
                                                        (session->stream_count+1));
            session->line[session->stream_count++] = channel;
        } else BKNI_Free(channel);
    }

    /* get rid of the iterator */
    delete iter;

    /* parse the header */
    if (basf_rtsp_parse_hdr(demux) != basf_rtp_err_success) {
        BDBG_ERR(("cannot find a usable asf header"));
        BDBG_ASSERT(false);        
        goto error;
    } else { basf_rtsp_aggbitrate_asfhdr(demux->props, 1); /* print it for debug */
        BDBG_MSG_TRACE(("hdr in map_to_demux"));
    }

    /* calculate total time, for later use, report it */
    session->total_length = (int64_t)(session->lm.ms->playEndTime()*1000000);
    if (session->total_length < 0) session->total_length = 0;
    else if (session->total_length > 0) {
        BDBG_MSG_TRACE(("total length for stream is: %d", session->total_length));
    }

    /* close session if no streams found */
    if (session->stream_count <= 0) {
        BDBG_ERR(("no codec supported, aborting"));
        goto error;
    }
    return (ret);

error:
    basf_rtsp_clean_session(session);    
    return (ret);
}

int 
basf_rtsp_destroy_instance(basf_rtsp_demux* p_this)
{
    int i, ret = 0;
    basf_rtp_channel *channel;
    basf_rtsp_demux* demux = (basf_rtsp_demux*)p_this;
    basf_rtsp_session *session = demux->session;

    if (demux->props) {
        basf_rtsp_header_parse_close(demux->props);     
    }
    for(i = 0; i < session->stream_count; i++) {
        channel = session->line[i];
        if (channel->buff) BKNI_Free(channel->buff);
        if (channel) BKNI_Free(channel);
    }

    if (session->stream_count) BKNI_Free(session->line);
    if (session->lm.rtsp && session->lm.ms) {
        session->lm.rtsp->teardownMediaSession(*session->lm.ms);
    }

#ifdef __HAVE_CLOSE_MS_ON_TEARDOWN__

    if (session->lm.ms) Medium::close(session->lm.ms);

#endif /* __HAVE_CLOSE_MS_ON_TEARDOWN__ */

    if (session->lm.rtsp) Medium::close(session->lm.rtsp);  
    if (session->lm.env) session->lm.env->reclaim();
    if (session->lm.scheduler) delete session->lm.scheduler;

    if (session->sdp_data) BKNI_Free(session->sdp_data);
    if (session) BKNI_Free(session);   /* free malloc-ed */
    if (demux) BKNI_Free(demux);

    return (ret);
}

/* auxiliary read function, called from LM read callback */
void 
basf_rtsp_stream_read(void *client_data, unsigned int size, unsigned int trunc_bytes, 
                      struct timeval pts, unsigned int duration)
{
    void* local_buff;
    uint64_t calc_pts;
    unsigned int min_pktsize, copy_amt;
    
    basf_rtp_channel* channel = (basf_rtp_channel*)client_data;
    basf_rtsp_demux* demux = channel->demux;
    basf_rtsp_session* session = demux->session;

    static int total_size = 0;
    static struct timeval prev = {0,0};
    static struct timeval diff_result = {0,0};
    struct timeval now, diff, diff_total;

    /* calculate bitrate */
    if (prev.tv_sec == 0) {
        gettimeofday(&prev, NULL);
    }
    total_size += size;
    gettimeofday(&now, NULL);

    timersub(&now, &prev, &diff); 
    BKNI_Memcpy(&diff_total, &diff_result, sizeof(struct timeval));
    timeradd(&diff_total, &diff, &diff_result);

    /* list bitrate */
    BDBG_MSG_TRACE(("%.2f Mb/sec on %d in %ds %du [%.2f Mb/sec] total=%d %ds %du\n",
            RATE(size, &diff)/(float)(1024*1024/8),
            size, diff.tv_sec, diff.tv_usec, 
            RATE(total_size, &diff_result)/(float)(1024*1024/8),
            total_size, diff_result.tv_sec, diff_result.tv_usec));
    prev = now;
    
    /* calculate pts for debug */
    BSTD_UNUSED(duration); 
    calc_pts = (uint64_t)pts.tv_sec*(1000000)+(uint64_t)pts.tv_usec;
    calc_pts &= 0x00ffffff;
    BDBG_MSG(("rd size=%d pts=%lld\n", size, (uint64_t)pts.tv_sec*(1000000)+pts.tv_usec));

    if (trunc_bytes > 0 && channel->buff_len < BASF_RTP_VIDEO_BUFFER_LEN) {
        BDBG_MSG(("lost %d bytes", trunc_bytes));
        BDBG_MSG(("increasing buffer size to %d", channel->buff_len * 2));
        
        channel->buff_len *= 2;                 /* double buffer and realloc */
        BDBG_ASSERT(false);
        BDBG_WRN(("realloc-ing to :%d\n", channel->buff_len));
        if ((local_buff = realloc(channel->buff, channel->buff_len)) == NULL)
            BDBG_WRN(("realloc failed"));
        else
            channel->buff = (uint8_t*)local_buff;
    }
    if (size > channel->buff_len) {              /* signal any buffer overflow */
        BDBG_MSG_TRACE(("buffer overflow"));
    }
    if (channel->asf_channel) {                  /* signal packet size to use for depacketization */
        min_pktsize = basf_rtsp_hdr_get_minpksize(demux->props);
        BDBG_ASSERT((copy_amt = ((min_pktsize < size) ? min_pktsize : size)) < (unsigned int)channel->buff_len);
        BDBG_MSG(("copy buffer must handle : %d", copy_amt));
    }
    
    /* reset read variables and set channel pts */
    if (session->lm.state != BASF_RTP_STATE_CLOSING) {
        session->lm.state = BASF_RTP_STATE_READING;
    }

    channel->block = 0; 
    demux->session->idle_data = BASF_FALSE;
    demux->session->idle_count = 0;
    demux->error = BASF_FALSE;

    if (calc_pts > 0 && channel) {
        channel->calc_pts = calc_pts;
    }
}

/* closes stream instance */
void 
basf_rtsp_stream_close(void *client_data)
{
    basf_rtp_channel* channel = (basf_rtp_channel*)client_data;
    basf_rtsp_demux* demux = channel->demux;
    basf_rtsp_session* session = demux->session;

    session->lm.state = BASF_RTP_STATE_CLOSING;
    demux->error = BASF_TRUE;
}

/* debug output information about the header data */
static void
basf_rtsp_print_header(basf_rtsp_demux_t demux)
{
    unsigned char* p_header = demux->asf_dmx_header.p_header;
    int p_header_size = demux->asf_dmx_header.p_header_size;

    BDBG_MSG(("ASF HDR INFO:"
           "%02x %02x %02x %02x last: %02x %02x %02x %02x\n",           
           p_header[0], p_header[1], p_header[2], p_header[3], 
           p_header[p_header_size-4], p_header[p_header_size-3], 
           p_header[p_header_size-2], p_header[p_header_size-1]));
}

/* called when a new header is available */
static int
basf_rtsp_notify_header(basf_rtsp_demux_t demux)
{
    basf_rtsp_handler_ctx ctx;
    static basf_rtp_info desc;
    int ret = 0;
    int64_t bitrate = 0;

    BDBG_ASSERT(demux);
    BDBG_ASSERT(demux->handler);

    if ((demux) && (demux->handler)) {       /* assume sync call, otherwise ctx should be dyn-alloc-ed */
        ctx.pk_type = basf_rtsp_pktype_unk;
        ctx.hdl_type = basf_rtsp_handler_header;

        desc.chunk = demux->asf_dmx_header.p_header;
        desc.size = demux->asf_dmx_header.p_header_size;
        desc.bitrate = bitrate;
        desc.commit_buffers = BASF_FALSE;   /* dot not commit headers */
        demux->session->acc_file_size += desc.size;
        demux->handler(&ctx, (void*)demux->handler_data, &desc);
        basf_rtsp_print_header(demux);
    }

    return (ret);
}

/* looks up the sdp headers and determines if this is the right kind of session
*/
basf_rtp_err
basf_rtsp_lookup_hdr(char* sdp_data, char** out_ptr)
{
    char* ptr = NULL;
    int miss = 0;
    basf_rtp_err ret = basf_rtp_err_fail;

    BDBG_ASSERT(sdp_data); BDBG_ASSERT(out_ptr);
    if ((sdp_data != NULL) && (out_ptr != NULL)) {
        if ((ptr = strcasestr(sdp_data, BASF_RTSP_HDR_PGMPU)) != NULL) {
        if ((ptr = strcasestr(ptr, BASF_RTSP_HDR_DATA)) != NULL) {
        if ((ptr = strcasestr(ptr, BASF_RTSP_HDR_WMSASF_APP)) != NULL) {
        if ((ptr = strcasestr(ptr, BASF_RTSP_HDR_WMSASF_V1)) != NULL) {
        if ((ptr = strcasestr(ptr, BASF_RTSP_HDR_BASE64)) != NULL) {
            ptr += strlen(BASF_RTSP_HDR_BASE64); 
            if ((ptr) && (ptr[0] == ',')) {
                *out_ptr = ++ptr;                
            } else { miss = 1; goto exit; }
        } else { miss = 2; goto exit; }
        } else { miss = 3; goto exit; }
        } else { miss = 4; goto exit; }
        } else { miss = 5; goto exit; }
    }}

exit:
    if (miss != 0) {
        BDBG_WRN(("returned asf header missing section #%d", miss));
    }

    return (ret);
}

/* parse the asf header and pass it back to rtp handler 
*/
basf_rtp_err
basf_rtsp_parse_hdr(basf_rtsp_demux *demux)
{
    int header_size;
    char *hdr_end, *asfptr = NULL;
    unsigned char* header_bin = NULL;
    
    basf_rtsp_session* session = demux->session;
    basf_rtsp_lookup_hdr(session->sdp_data, &asfptr);
    if (asfptr == NULL) {                   /* parse the asf header */
        BDBG_WRN(("parse_hdr could NOT find marker"));
        return basf_rtp_err_fail;
    } else { BDBG_MSG(("parse_hdr found marker: %s\n", asfptr)); }

    asfptr  = strdup(asfptr);             
    hdr_end = strchr(asfptr, '\n');
    while(hdr_end > asfptr && (*hdr_end == '\n' || *hdr_end == '\r'))
        *hdr_end-- = '\0';

    BDBG_MSG(("asfptr %s\n", asfptr));
    if (asfptr >= hdr_end) {
        return basf_rtp_err_fail;
    }
    header_size = hdr_end - asfptr;     
    header_bin = (unsigned char*)BKNI_Malloc(hdr_end - asfptr);
    if ((header_size = basf_rtp_b64_uudecode(header_bin, asfptr, header_size)) <= 0) {
        BKNI_Free(asfptr);  
        return basf_rtp_err_fail;          /* could not uudecode */
    } else {
        demux->asf_dmx_header.p_header = header_bin;                            /* we save the header here */
        demux->asf_dmx_header.p_header_size = header_size;
        demux->props = basf_rtsp_header_parse_open(&demux->session->headers, 
                                                   header_bin, header_size);    /* parse asf headers*/                                                                               
        basf_rtsp_notify_header(demux);                                         /* finish off by notifying rtp of header */
                                                                                /* parse it to get packet size */
    }

    BKNI_Free(asfptr);
    return basf_rtp_err_success;
}

/* get packet size and count for the rtp processor, to determine eof
   and buffering policy
*/
void
basf_rtsp_get_packet_info(basf_rtsp_demux_t demux, int* packet_size, int* packet_count)
{
    BDBG_ASSERT(packet_size);
    BDBG_ASSERT(packet_count);

    if (packet_count)
        *packet_count = basf_rtsp_hdr_get_pktcount(demux->props);
    if (packet_size)
        *packet_size = basf_rtsp_hdr_get_minpksize(demux->props);
}

#ifdef __HAS_LEGACY_HDR__
/* extract header structure from the hdr, to get the size of the packet */
int 
basf_rtsp_get_header(basf_rtsp_demux_t demux, asf_header_t** asf_hdr)
{
    int ret = 0;  

    BDBG_ASSERT(demux);
    BDBG_ASSERT(asf_hdr);

    if ((demux) && (demux->session) && (asf_hdr)) {
        BDBG_ERR(("hdr in get_header"));
        basf_rtsp_aggbitrate_asfhdr(demux->session->headers, 0);
        *asf_hdr = &demux->session->asf_hdr;
    } else { BDBG_WRN(("could not get header, invalid params"));
        ret = -1;
    }
    
    return (ret);
}
#endif /* __HAS_LEGACY_HDR__ */

void 
basf_rtp_watchdog_endclose(void *client_data)
{
    basf_rtsp_session* session = (basf_rtsp_session*)client_data;
    session->idle_count++;

    BDBG_ERR(("watchdog END CLOSE func, ti=%d", session->idle_count));
    session->cfg->demux->error = BASF_TRUE;
    session->lm.state = BASF_RTP_STATE_CLOSING;
}

/* watchdog signalling rtp timeout */
void 
basf_rtp_watchdog_timeout(void *client_data)
{    
    int32_t packets;
    int64_t file_size;

    basf_rtsp_session* session = (basf_rtsp_session*)client_data;
    session->idle_count++;

    BDBG_ERR(("watchdog func, ti=%d", session->idle_count));

    session->cfg->demux->error = BASF_TRUE;
    session->lm.state = BASF_RTP_STATE_CLOSING;
    
    file_size = basf_rtsp_hdr_get_filesize(session->demux->props);
    BDBG_WRN(("watchdog accumulated file_size=%ld, total file_size=%ld", 
                (long)session->acc_file_size, (long)file_size));

    packets = basf_rtsp_hdr_get_pktcount(session->demux->props);
    BDBG_WRN(("watchdog packet limit is: valid:%d + lost:%d >= packets:%d",
              session->demux->stats.pkts_valid, session->demux->stats.pkts_lost, packets));
    
    /* reschedule the watchdog */
    session->rtsp_timer = b_timer_schedule(BASF_RTSP_TIMEOUT_CALC(session), basf_rtp_watchdog_timeout,
                                           (void*)session);  
}

/* create the playback timer */
static b_timer_t
basf_rtsp_create_watchdog(basf_rtsp_session* session)
{
	b_timer_t rtsp_timer = NULL; 

    BDBG_ASSERT(session);
    if (session != NULL) {
        rtsp_timer = b_timer_schedule(BASF_RTSP_TIMEOUT_CALC(session), 
                                      basf_rtp_watchdog_timeout, 
                                      (void*)session);

        session->rtsp_timer = rtsp_timer;        
    }

    return (rtsp_timer);
}

/* cancel the playback timer */
static void
basf_rtsp_destroy_watchdog(basf_rtsp_session* session)
{
    BDBG_ASSERT(session);
    if (session) {
        BDBG_ASSERT(session->rtsp_timer);
        if (session->rtsp_timer) {
            b_timer_cancel(session->rtsp_timer);
            session->rtsp_timer = NULL;        
        }
    }
}

/* determine if we need to close the session gere */
static int
basf_rtsp_calc_closing(basf_rtsp_session *session)
{
    int ret = 0;

    if (session->closing == BASF_TRUE) {
        BDBG_WRN(("reached stream end of file, closing"));
        return (-1);
    } else if ((session->multicast) && (session->idle_data) && 
              (session->idle_count > BASF_RTP_MCAST_IDLETIME)) {
        BDBG_WRN(("multicast data timeout, aborting"));
        BDBG_ASSERT(false);
        return (-1);
    } else if (!(session->multicast) && (session->idle_data) && 
              (session->idle_count > BASF_RTP_NONMCAST_IDLETIME)) {        
        BDBG_WRN(("non-multicast data timeout, aborting"));
        BDBG_ASSERT(false);
        return (-1);
    }

    return (ret);
}

/* set accessor function to rtp timer */
void 
basf_rtsp_set_rtptimer(basf_rtsp_demux_t demux, b_timer_t rtp_timer)
{
    BDBG_ASSERT(demux);
    if (demux) {
        demux->rtp_timer = rtp_timer;
    }
}

/* get accessor function to rtp timer */
b_timer_t
basf_rtsp_get_rtptimer(basf_rtsp_demux_t demux)
{
    b_timer_t rtp_timer = NULL;

    BDBG_ASSERT(demux);
    if (demux) { 
        rtp_timer = demux->rtp_timer;
    }
    
    return (rtp_timer);
}

/* start the demux instance */
int 
basf_rtsp_run_demux(basf_rtsp_demux_t demux)
{
    TaskToken task;
    basf_rtp_channel *channel;
    struct timeval now, diff;
    
    static struct timeval prev = {0,0};
    int index = 0;
    char* param_value = NULL;
    basf_rtsp_session *session = demux->session;

    /* see if we need to stop the session */
    if (basf_rtsp_calc_closing(session) == -1) {        
        BDBG_WRN(("final closing session."));
        demux->error = BASF_TRUE;
        goto error;
    } else {
        BDBG_MSG_TRACE(("session->lm.state==%d", session->lm.state));
    }

    for(index = 0; index < session->stream_count; index++) {
        channel = session->line[index];
        if (channel->block == 0) {
            channel->block = 1;     /* naive blocking */    
            channel->lm.frame_sink->getNextFrame(channel->buff, channel->buff_len,
                basf_rtsp_stream_read, channel, basf_rtsp_stream_close, 
                channel);
        }
    }

    
    if (basf_rtsp_create_watchdog(session)) {                             /* create a watchdog */   
        if (session->lm.state != BASF_RTP_STATE_CLOSING) {
            session->lm.state = BASF_RTP_STATE_START;    
                session->lm.scheduler->doEventLoop(&session->lm.state);         /* do the read */            
                
                if ((prev.tv_sec == 0) && (prev.tv_usec == 0)) {
                    gettimeofday(&prev, NULL);
                }
    
                gettimeofday(&now, NULL);
                timersub(&now, &prev, &diff); 
               
                /* windows media uses a dummy rtsp GET_PARAMETER to maintain keep-alive */ 
                if ((diff.tv_sec > BASF_RTSP_TIMEOUT_CALC(session)/(1000))) {
                    if (session->lm.rtsp != NULL) {
                        prev = now;
                        session->lm.rtsp->getMediaSessionParameter(*session->lm.ms, 
                                NULL, param_value);
                    }
                }                
                
                /* flag the channel if we're doing RTCP synchro */
                for(index = 0; index < session->stream_count; index++) {                        
                    channel = session->line[index];
                    if (!channel->rtcp_sync && channel->lm.rtp_sink && 
                            channel->lm.rtp_sink->hasBeenSynchronizedUsingRTCP())
                        session->rtcp_sync = BASF_TRUE;
                        BDBG_MSG(("channel->rtpSource->hasBeenSynchronizedUsingRTCP()"));
                }
            } 
            basf_rtsp_destroy_watchdog(session);                         /* unschedule the task */
    }
error:
    return (demux->error==BASF_TRUE) ? (0):(1);
}

/* updates packet statistics */
void 
basf_rtsp_update_stats (basf_rtsp_demux_t demux, int pkts_lost, int pkts_valid, int plds_lost,
                        int plds_dropped, int plds_valid, int plds_pack)
{
    BDBG_ASSERT(demux);

    if (demux) {

        demux->stats.pkts_lost += pkts_lost;
        demux->stats.pkts_valid += pkts_valid;
        demux->stats.plds_lost += plds_lost;        
        demux->stats.plds_dropped += plds_dropped;
        demux->stats.plds_total += plds_valid;
        
        if (plds_pack != 0)
            demux->stats.plds_pack = (demux->stats.plds_pack + plds_pack)/(2.0);    
            if (pkts_lost)
                BDBG_MSG_TRACE(("pkts lost:%d valid:%d plds lost:%d total:%d plds/pack avg:%2f now:%d\n", 
                                demux->stats.pkts_lost, demux->stats.pkts_valid,
                                demux->stats.plds_lost, demux->stats.plds_total, 
                                demux->stats.plds_pack, plds_pack));
    }
}

