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

#ifndef __BASF_HDR_COMMON_H__
#define __BASF_HDR_COMMON_H__

#include "liveMedia.hh"
#include "basf_rtp_proc.h"

/* calculate the payload length based on the first byte in it */
#define BASF_RTP_PAYLOAD_LEN(p) (p) ? ((p[0] & 0x3) + 1) << 2 : 0;

typedef struct basf_rtp_stream {
        /* TODO: handle ASF stream characteristics here
         *     */
} basf_rtp_stream;

/* structure describing an asf stream instance 
*/
typedef struct basf_rtp_channel {
    struct {                            /* livemedia aggregate structure */
        RTPSource* rtp_sink;            /* rtp source */
        FramedSource* frame_sink;       /* framed source sink */    
        AuxHandlerFunc* rtp_handler;    /* auxiliary handler function */
    } lm;
    basf_rtsp_demux_t demux;            /* pointer to parent */
    bool asf_channel, rtcp_sync;        /* channel flags */
    unsigned char* buff;                /* rtp buffer */
    unsigned int buff_len;              /* rtp buffer length */
    unsigned int num_channels;
    int64_t calc_pts, rate;             /* bitrate and current pts */
    const char* aux_data;               /* auxiliary callback data */
    unsigned char block;                /* naive block var */
} basf_rtp_channel;

/* structure aggregating asf headers
*/
typedef struct basf_ip_headers {
    basf_hdr_stream_lst* pstream;
    basf_hdr_content* pcontent;
    basf_hdr_file* pfile;
    basf_hdr_bitrate* pbitrate;
    basf_hdr_codec_lst* pcodec;
} basf_ip_headers;


/* open a new parser instance, used to crunch the sdp asf header
*/
basf_props_t basf_rtsp_header_parse_open (
    basf_ip_headers_t hdr, 
    uint8_t* header, 
    int header_len
    );
/* close current parser instance
*/
void basf_rtsp_header_parse_close (
    basf_props_t props                  /* [in] */
    );
/* get total packet count from the file header 
*/
int32_t basf_rtsp_hdr_get_pktcount(
    basf_props_t headers                /* [in] */
    );
/* get the total filesize from the file header 
*/
int64_t basf_rtsp_hdr_get_filesize (
    basf_props_t headers                /* [in] */
    );
/* get the aggregate bitrate header
*/
int64_t basf_rtsp_hdr_get_bitrate (
    basf_props_t headers                /* [in] */
    );
/* get the minimum packet size from the file header 
*/
int32_t basf_rtsp_hdr_get_minpksize (
    basf_props_t headers                /* [in] */
    );
/* print asf description header and aggregate bitrates 
*/
int64_t basf_rtsp_aggbitrate_asfhdr (
    basf_props_t headers,               /* [in] */
    int print                           /* [in] */
    );
/* implementation for a base64 decode, the non-unix way
*/
int basf_rtp_b64_uudecode (
    unsigned char *dest,                /* [out] */
    char *src,                          /* [in] */
    int size                            /* [in] */
    );

typedef void (*basf_rtp_fn_feed) (void);

#endif /* __BASF_HDR_COMMON_H__ */

