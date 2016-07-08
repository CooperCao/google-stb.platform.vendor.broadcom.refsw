/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * RTP H263 parser library
 *   RFC 4629 module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brtp_parser_h263.h"
#include "brtp_packet.h"
#include "bbase64.h"

BDBG_MODULE(brtp_parser_h263);

BDBG_OBJECT_ID(brtp_parser_h263_t);

struct brtp_parser_h263 {
    struct brtp_parser parent; /* must be the first element */
    BDBG_OBJECT(brtp_parser_h263_t)
    uint8_t *nal_header;
    brtp_parser_h263_cfg cfg;
    enum {brtp_state_single,  brtp_state_multi} state;
    unsigned multi_len;
    const uint8_t *meta_base;
    size_t meta_len;
    enum {brtp_nal_state_none, brtp_nal_state_idr} nal_state;
};
typedef struct brtp_parser_h263 *brtp_parser_h263_t;

static const brtp_parser_type b_rtp_parser_h263 = {
    "H.263",
    brtp_stream_video
};

const brtp_parser_type *brtp_parser_h263 = &b_rtp_parser_h263;

static bool
b_rtp_parser_h263_data(brtp_parser_h263_t parser, brtp_packet_t packet, const void *data, size_t len)
{
    return brtp_parser_add_packet(&parser->parent, packet, data, len);
}


static void
b_rtp_parser_h263_add_meta(brtp_parser_h263_t parser)
{
    brtp_io_vec *vec;

    if (parser->meta_len) {
        BDBG_MSG(("b_rtp_parser_h263_add_meta:%#lx adding %zu bytes of meda data", (unsigned long)parser, parser->meta_len));
        vec = brtp_parser_next(&parser->parent);
        if (vec) {
            vec->len = parser->meta_len;
            vec->data = parser->meta_base;
            vec->packet = NULL;
        }
    }
    return;
}



static void 
b_rtp_parser_h263_packet(brtp_parser_t parser_, brtp_packet_t pkt, const void *data, size_t len)
{
    brtp_parser_h263_t parser = (brtp_parser_h263_t)parser_;

    uint8_t nal;
    unsigned p_type;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_h263_t);
    BDBG_ASSERT(parser->parent.stream.mux);

    nal = B_RTP_LOAD8(data,0);
    p_type = B_RTP_GET_BIT(nal, 5);

    BDBG_MSG(("Len %zu Data %x %x %x %x %x %x %x %x",len,B_RTP_LOAD8(data,0),B_RTP_LOAD8(data,1),B_RTP_LOAD8(data,2),B_RTP_LOAD8(data,3),B_RTP_LOAD8(data,4),B_RTP_LOAD8(data,5),B_RTP_LOAD8(data,6),B_RTP_LOAD8(data,7)));

    if(p_type == 0x4)
    {
        brtp_parser_begin(&parser->parent);

        ((uint8_t *)data)[0] = 0x00;
        ((uint8_t *)data) [1]= 0x00;

        b_rtp_parser_h263_data(parser, pkt, (uint8_t *)data, len);
        brtp_parser_commit(&parser->parent);
    }else{

        brtp_parser_begin(&parser->parent);
        b_rtp_parser_h263_data(parser, pkt, (uint8_t *)data+2, len-2);
        brtp_parser_commit(&parser->parent);

    }
    return;

}

static void 
b_rtp_parser_h263_discontinuity(brtp_parser_t parser_)
{
    brtp_parser_h263_t parser = (brtp_parser_h263_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_h263_t);
    if (parser->state == brtp_state_multi) { /* accumulating packets */
        BDBG_WRN(("b_rtp_parser_h263_discontinuity: %#lx dropping accumulated data", (unsigned long)parser));
        brtp_parser_abort(&parser->parent);
        parser->state = brtp_state_single;
    }
    return;
}

brtp_parser_h263_cfg b_rtp_parser_h263_default_cfg = {
    4+128,/* meta_len nal_hdr, SPS + PPS */
    NULL    /* meta */
};

void
brtp_parser_h263_default_cfg(brtp_parser_h263_cfg *cfg)
{
    *cfg = b_rtp_parser_h263_default_cfg;
    return;
}



static const char *
b_strchr(const char *str, char ch)
{
    char str_ch;

    while( '\0'!= (str_ch=*str)) {
        if (str_ch==ch) {
            return str;
        }
        str++;
    }
    return NULL;
}

#if 0
static size_t 
b_strlen(const char *str) 
{
    size_t len;
    for(len=0;str[len];len++) { }
    return len;
}
#endif

static brtp_parser_mux_stream_t
b_rtp_parser_h263_start(brtp_parser_t parser_, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *sprop, size_t sprop_len)
{
    brtp_parser_h263_t parser = (brtp_parser_h263_t)parser_;
    const char *sprop1;
    uint8_t *meta = (uint8_t *)parser->cfg.meta + 3;
    size_t meta_left = parser->cfg.meta_len - 3;
    int data_len;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_h263_t);
    BDBG_ASSERT(sprop);
    BDBG_ASSERT(cfg);
    brtp_parser_mux_attach(mux, parser_, cfg);
    parser->meta_base = meta;
    brtp_parser_begin(&parser->parent);
    sprop1 = b_strchr(sprop, ',');
    if (sprop1 && meta_left>3) {
        meta[0] = 0;
        meta[1] = 0;
        meta[2] = 1;
        meta+=3;
        meta_left-=3;
        data_len = bbase64_decode(sprop, sprop1-(const char*)sprop, meta, meta_left);
        BDBG_MSG(("sprop1 '%s' len %i", (const char *)sprop, data_len));
        if (data_len>0) {
            meta+=data_len;
            meta_left-=data_len;
        } else {
            BDBG_WRN(("corrupted meta information[1]"));
        }
        sprop_len -= 1+sprop1 - (const char *)sprop;
        sprop = sprop1+1; /* skip comma */
    } else {
        BDBG_WRN(("incomplete meta information[1]"));
    }

    if (meta_left>3) {
        meta[0] = 0;
        meta[1] = 0;
        meta[2] = 1;
        meta+=3;
        meta_left-=3;
        data_len = bbase64_decode(sprop, sprop_len, meta, meta_left);
        BDBG_MSG(("sprop2'%s' len %i",  (const char *)sprop, data_len));
        if (data_len>0) {
            meta+=data_len;
            meta_left-=data_len;
        } else {
            BDBG_WRN(("corrupted meta information[2]"));
        }
    } else {
        BDBG_WRN(("incomplete meta information[2]"));
    }
    parser->meta_len = meta - parser->meta_base;
    parser->nal_state = brtp_nal_state_none;
    BDBG_MSG(("meta %zu bytes", parser->meta_len));

    if(parser->meta_len) {
        brtp_parser_begin(&parser->parent);
        b_rtp_parser_h263_add_meta(parser);
        brtp_parser_commit(&parser->parent);
    }
    parser->state = brtp_state_single;
    parser->multi_len = 0;
    return  &parser->parent.stream;
}

static void 
b_rtp_parser_h263_stop(brtp_parser_t parser_)
{
    brtp_parser_h263_t parser = (brtp_parser_h263_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_h263_t);
    brtp_parser_stop(&parser->parent);
    parser->state = brtp_state_single;
    return;
}

static void
b_rtp_parser_h263_destroy(brtp_parser_t parser_)
{
    brtp_parser_h263_t parser = (brtp_parser_h263_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_h263_t);
    b_rtp_parser_h263_stop(parser_);
    BDBG_OBJECT_DESTROY(parser, brtp_parser_h263_t);
    BKNI_Free(parser);
    return;
}

brtp_parser_t 
brtp_parser_h263_create(const brtp_parser_h263_cfg *cfg)
{
    brtp_parser_h263_t parser;

    BDBG_CASSERT(B_RTP_GET_BIT((uint8_t)0x80, 0));
    BDBG_CASSERT(B_RTP_GET_BIT((uint8_t)0x40, 0)==0);
    BDBG_CASSERT(B_RTP_GET_BITS((uint8_t)0x80, 0,1)==2);

    BDBG_ASSERT(cfg);
    BDBG_ASSERT(cfg->meta);
    parser = BKNI_Malloc(sizeof(*parser));
    if (!parser) {
        BDBG_ERR(("out of memory in brtp_parser_h263_create"));
        return NULL;
    }
    BDBG_OBJECT_INIT(parser, brtp_parser_h263_t);
    brtp_parser_init(&parser->parent);
    parser->parent.packet = b_rtp_parser_h263_packet;
    parser->parent.discontinuity = b_rtp_parser_h263_discontinuity;
    parser->parent.start = b_rtp_parser_h263_start;
    parser->parent.stop = b_rtp_parser_h263_stop;
    parser->parent.destroy = b_rtp_parser_h263_destroy;
    parser->parent.type = brtp_parser_h263;
    parser->cfg = *cfg;
    parser->nal_header = cfg->meta;
    parser->nal_header[0] = 0;
    parser->nal_header[1] = 0;
    parser->nal_header[2] = 1;
    parser->state = brtp_state_single;
    return &parser->parent;
}
