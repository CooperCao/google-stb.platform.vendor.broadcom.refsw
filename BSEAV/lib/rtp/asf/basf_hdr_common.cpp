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

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* local refsw includes */
#include "bstd.h"           
#include "bkni.h"
#include "bdbg.h"
#include "bsettop.h"

#include "basf_parser.h"
#include "basf_rtp_proc.h"
#include "blst_squeue.h"    /* blst_sq_ */

#include "asfpars.h"
#include "basf_hdr_common.h"

BDBG_MODULE(basf_hdr_common);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x) 
#ifdef PROFILE
    #undef BDBG_MSG
    #define BDBG_MSG(x)
#endif

#define BASF_RTP_B64_DECODED_SIZE(size) ((size+3)/4)*3

/* RFC1113 base64 decode */
const int pr2six[256]= {
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
    64,0 , 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
    64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
};

char six2pr[64] = {
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z', 'a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/'
};

/* implementation for a base64 decode, the non-unix way
*/
int basf_rtp_b64_uudecode(unsigned char* dest, char* src, int size)
{
    int sizedecoded, last;
    unsigned char *out;
    char *walk = src;
    
    BDBG_ASSERT(dest); BDBG_ASSERT(src);
    if(!(dest) || !(src)) {
        return BASF_RTP_B64_DECODED_SIZE(size);
    }

    walk = src;
    for(;;) {
        if(pr2six[(int)*walk++] < 64) {        
            continue;
        } else {
            break;
        }
    }
    size = (int)(walk-src-1);
    sizedecoded = BASF_RTP_B64_DECODED_SIZE(size);
    walk = src; out = dest;
    while (size > 0) {
        *(out++) = (unsigned char)(pr2six[(int)*walk] << 2 | pr2six[(int)walk[1]] >> 4);
        *(out++) = (unsigned char)(pr2six[(int)walk[1]] << 4 | pr2six[(int)walk[2]] >> 2);
        *(out++) = (unsigned char)(pr2six[(int)walk[2]] << 6 | pr2six[(int)walk[3]]);
        walk += 4; size -= 4;
    }
    if(size & 03) {
        last = *(walk-2);
        sizedecoded -= (pr2six[last] >= 64) ? 2 : 1;
    }

    return sizedecoded;
}

/* render the contents of the content header */
void
basf_app_render_content_hdr(basf_hdr_content* content_hdr)
{
    if(content_hdr != NULL){
        BDBG_MSG_TRACE(("content-title=%s(%p)\ncontent-author=%s(%p)\ncontent-copyr=%s(%p)\n"
               "content-descr=%s(%p)\ncontent-rating=%s(%p)\n",
               content_hdr->title, content_hdr->title, 
               content_hdr->author, content_hdr->author,
               content_hdr->copyright, content_hdr->copyright,
               content_hdr->description, content_hdr->description,
               content_hdr->rating, content_hdr->rating));
    }
}

/* render the contents of the stream headers */
void
basf_app_render_stream_hdr(basf_hdr_stream_lst* stream_hdr_list)
{
    basf_hdr_stream* stream_hdr;
    for(stream_hdr = BLST_S_FIRST(stream_hdr_list); stream_hdr; 
        stream_hdr = BLST_S_NEXT(stream_hdr,link)) {
            BDBG_MSG_TRACE(("stream-type="));
            switch(stream_hdr->stream_type) {
                case basf_media_stream_video: BDBG_MSG_TRACE(("video\n"));break;
                case basf_media_stream_audio: BDBG_MSG_TRACE(("audio\n"));break;
                case basf_media_stream_unk:   BDBG_MSG_TRACE(("unknown\n"));break;
            }
            BDBG_MSG_TRACE(("stream-no=%d\nstream-reserved=%d\n"
                   "stream-encrypted=(%s)\n", 
                    B_ASFP_FLAGS_GET_STREAM_NO(stream_hdr->flags),
                    B_ASFP_FLAGS_GET_RESERVED(stream_hdr->flags),
                    B_ASFP_FLAGS_GET_ENCRYPTED(stream_hdr->flags) ? 
                        "encrypted" : "not encrypted"));
        }
}

/* render the contents of the file header 
*/
void
basf_app_render_file_hdr(basf_hdr_file* file_hdr)
{
    if(file_hdr != NULL) {
        BDBG_MSG_TRACE(("file-size=%llu\nfile-creation_date=%llu\n"
               "file-datapack_count=%llu\nfile-play_duration=%llu\n"
               "file-send_duration=%llu\nfile-preroll=%llu\n"
               "file-flags=%u\nfile-min_dt_pack_size=%u\n"
               "file-max_dt_pack_size=%u\nfile-max_bitrate=%u\n",
               file_hdr->size, file_hdr->creation_date,
               file_hdr->datapack_count, file_hdr->play_duration,
               file_hdr->send_duration, file_hdr->preroll,
               file_hdr->flags, file_hdr->min_dt_pack_size,
               file_hdr->max_dt_pack_size, file_hdr->max_bitrate));    
    }
}

/* get the total filesize from the file header 
*/
int64_t 
basf_rtsp_hdr_get_filesize(basf_props_t props)
{
    int64_t file_size = 0;
    basf_hdr_file* pfile = NULL;

    BDBG_ASSERT(props);
    basf_get_file_hdr(props, &pfile);                       /* get 'FILE' header */

    BDBG_ASSERT(pfile);
    file_size = pfile->size;

    return file_size;
}

/* get the minimum packet size from the file header 
*/
int32_t 
basf_rtsp_hdr_get_minpksize(basf_props_t props)
{
    int64_t min_pkt_size = 0;
    basf_hdr_file* pfile = NULL;

    BDBG_ASSERT(props);
    basf_get_file_hdr(props, &pfile);                       /* get 'FILE' header */

    BDBG_ASSERT(pfile);
    min_pkt_size = pfile->min_dt_pack_size;

    return min_pkt_size;
}

/* get total packet count from the file header 
*/
int32_t 
basf_rtsp_hdr_get_pktcount(basf_props_t props)
{
    int packet_count;
    basf_hdr_file* pfile = NULL;

    BDBG_ASSERT(props);
    basf_get_file_hdr(props, &pfile);                       /* get 'FILE' header */

    BDBG_ASSERT(pfile);
    packet_count = pfile->datapack_count;

    return packet_count;
}

/* render the contents of the bitrate header 
*/
void
basf_app_render_bitrate_hdr(basf_hdr_bitrate* bitrate_hdr)
{
    unsigned int index;
    if(bitrate_hdr != NULL) {
        BDBG_MSG_TRACE(("bitrate-count=%lu\n"));
        for(index = 0; index < bitrate_hdr->count; index++) {
               BDBG_MSG_TRACE(("bitrate-stream_id=%lu\nbitrate-stream_rate=%lu\n",
                      bitrate_hdr->stream_id[index], 
                      bitrate_hdr->stream_bitrate[index]));    
        }
    }
}

/* calculate and return aggregate bitrate 
*/
int64_t
basf_app_calc_agg_bitrate(basf_hdr_bitrate* bitrate_hdr)
{
    unsigned int index;
    int64_t agg_bitrate = 0;
    if(bitrate_hdr != NULL) {
        for(index = 0; index < bitrate_hdr->count; index++) {
            agg_bitrate += bitrate_hdr->stream_bitrate[index];    
        }
    }
    return agg_bitrate;
}

/* get the aggregate bitrate header
*/
int64_t
basf_rtsp_hdr_get_bitrate(basf_props_t props)
{
    int64_t bitrate = 0;
    basf_hdr_bitrate* pbitrate = NULL;

    BDBG_ASSERT(props);
    basf_get_bitrate_hdr(props, &pbitrate);                 /* get 'BITRATE' header */

    BDBG_ASSERT(pbitrate);
    bitrate = basf_app_calc_agg_bitrate(pbitrate);          /* and calculate aggregated bitrate */
  
    return bitrate;
}

/* render the contents of the codec headers */
void
basf_app_render_codec_hdr(basf_hdr_codec_lst* codec_hdr_list)
{
    uint32_t ct = 0;
    basf_hdr_codec_indiv* codec_hdr;

    for(codec_hdr = BLST_S_FIRST(codec_hdr_list); codec_hdr; 
        codec_hdr = BLST_S_NEXT(codec_hdr,link)) {
            BDBG_MSG_TRACE(("codec-type="));
            switch(codec_hdr->type) {
                case asf_cdt_video:   BDBG_MSG_TRACE(("video\n")); break;
                case asf_cdt_audio:   BDBG_MSG_TRACE(("audio\n")); break;
                case asf_cdt_unknown: BDBG_MSG_TRACE(("unknown\n")); break;
            }            
            BDBG_MSG_TRACE(("codec-name=%s(%p)\ncodec-descr=%s(%p)\n",
                   codec_hdr->name, codec_hdr->name, 
                   codec_hdr->description, codec_hdr->description));
            BDBG_MSG_TRACE(("codec-info="));
            for(ct = 0; ct < codec_hdr->info_len; ct++)     /* print codec info (FOURCC) */
                BDBG_MSG_TRACE(("'%.2x'", codec_hdr->info[ct]));            
        }
}

/* print asf description header and aggregate bitrates 
*/
int64_t
basf_rtsp_aggbitrate_asfhdr (basf_props_t headers, int print)
{   
    int ret = 0;

    BSTD_UNUSED(headers);
    BSTD_UNUSED(print);

#if 0
    unsigned int i;
    BDBG_MSG_TRACE(("ASF HEADER: \nfilesize=%ld packets_num=%ld min_pack_size=%d",
            (long)headers.filesize,
            (long)headers.packets_num,
            headers.min_pack_size));
    BDBG_MSG_TRACE(("max possible streams is : %d", sizeof(headers)/sizeof(basf_rtp_stream_t)));
    for(i = 0; i < sizeof(asfh.stream)/sizeof(basf_rtp_stream_t); i++) {
        if ((BASF_RTP_STREAM_AUDIO == headers.s[i].type) || 
            (BASF_RTP_STREAM_AUDIO == headers.s[i].type)) {
                BDBG_MSG_TRACE(("*%s* [%d].type=%d [%d].on=%d [%d].bitrate=%d", 
                      (headers.s[i].type == BASF_RTP_STREAM_AUDIO) ? "audio":"video",
                      i, headers.s[i].type, i, headers.s[i].on,
                      i, headers.s[i].bitrate));
                if(!print) {
                    headers.agg_bitrate += headers.s[i].bitrate;
                }
        }
    }
#endif 
    return (ret);
}

/* close current parser instance
*/
void
basf_rtsp_header_parse_close(basf_props_t props)
{
    BDBG_ASSERT(props);
    if(props) {
        basf_close_asf(props);                                  /* close asfa */            
    }
}

/* open a new parser instance, used to crunch the sdp asf header
*/
basf_props_t
basf_rtsp_header_parse_open(basf_ip_headers_t headers, uint8_t* header, int header_len)
{
    basf_parser_t asf;
    basf_props_t props = NULL;    

    BDBG_ASSERT(header && header_len > 0);
    if (header && header_len > 0) {
        if((props = basf_open_asf_chunk(header, header_len+1, basf_hdr_ctx_gen, &asf)) != NULL) {
            basf_scan_asf_chunk(props);                           
            basf_get_stream_list_hdr(props, &headers->pstream);      /* get 'STREAM' header(s) */
            basf_app_render_stream_hdr(headers->pstream);    

            basf_get_content_hdr(props, &headers->pcontent);         /* get 'CONTENT' header */
            basf_app_render_content_hdr(headers->pcontent);        

            basf_get_file_hdr(props, &headers->pfile);               /* get 'FILE' header */
            basf_app_render_file_hdr(headers->pfile);        

            basf_get_bitrate_hdr(props, &headers->pbitrate);         /* get 'BITRATE' header */
            basf_app_render_bitrate_hdr(headers->pbitrate);        

            basf_get_codec_hdr(props, &headers->pcodec);             /* get 'CODECS' header */
            basf_app_render_codec_hdr(headers->pcodec);        
        } else {
            BDBG_ERR(("open_asf_chunk failed"));
        }
    } else {
        BDBG_ERR(("invalid inputs in parse_open"));
    }

    return (props);
}

