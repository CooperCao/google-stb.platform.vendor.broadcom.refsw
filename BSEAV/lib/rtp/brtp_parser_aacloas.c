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
 * RTP AAC-LOAS parser library
 *   RFC 6416 module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brtp_parser_aacloas.h"
#include "brtp_packet.h"
#include "biobits.h"
#include "brtp_util.h"

BDBG_MODULE(brtp_parser_aacloas);

BDBG_OBJECT_ID(brtp_parser_aacloas_t);

struct brtp_parser_aacloas {
    struct brtp_parser parent; /* must be the first element */
    BDBG_OBJECT(brtp_parser_aacloas_t)
    brtp_parser_aacloas_stream_cfg stream_cfg;
    unsigned pkt_cnt;
    unsigned pkt_len;
    brtp_parser_aacloas_cfg cfg;

    const uint8_t *meta_base;
    size_t meta_len;

};

typedef struct brtp_parser_aacloas *brtp_parser_aacloas_t;

static const brtp_parser_type b_rtp_parser_aacloas= {
    "AAC-LOAS",
    brtp_stream_audio
};

const brtp_parser_type *brtp_parser_aacloas = &b_rtp_parser_aacloas;



void
brtp_parser_aacloas_default_cfg(brtp_parser_aacloas_cfg *cfg)
{
    *cfg = 0;
    return;
}

static const brtp_parser_aacloas_stream_cfg b_rtp_parser_aacloas_aac_loas_cfg = {
    brtp_parser_aacloas_aac_loas, /* mode */
    13, /* sizelength */
    3, /* indexlength */
    2, /* headerlength */
    {0}, /* config */
    0, /* config_len */
    15 /* profile */
};

void 
brtp_parser_aacloas_default_stream_cfg(brtp_parser_aacloas_mode mode, brtp_parser_aacloas_stream_cfg *cfg)
{
    BDBG_ASSERT(cfg);
    switch(mode) {
    default:
        BDBG_WRN(("brtp_parser_aacloas_default_stream_cfg: not supported mode %u", mode));
        cfg->mode = mode;
        break;
    case brtp_parser_aacloas_aac_loas:
        *cfg = b_rtp_parser_aacloas_aac_loas_cfg;
        break;

    }
    return;
}

static void
b_rtp_parser_aacloas_begin(brtp_parser_aacloas_t parser)
{
    brtp_parser_begin(&parser->parent);
    parser->pkt_cnt = 0;
    parser->pkt_len = 0;
    return;
}

static void
b_rtp_parser_aacloas_commit(brtp_parser_aacloas_t parser)
{
    brtp_parser_commit(&parser->parent);
    return;
}

uint8_t streamMuxConfig[6];
static void 
b_rtp_parser_aacloas_packet(brtp_parser_t parser_, brtp_packet_t pkt, const void *data, size_t len)
{
    brtp_parser_aacloas_t parser = (brtp_parser_aacloas_t)parser_;
    BSTD_UNUSED(len);

    BDBG_OBJECT_ASSERT(parser, brtp_parser_aacloas_t);
    BDBG_ASSERT(parser->parent.stream.mux);
    BDBG_MSG(("Len (%zu) %zu -> Data %x %x %x %x %x %x %x %x\n\n",pkt->len,len,B_RTP_LOAD8(data,0),B_RTP_LOAD8(data,1),B_RTP_LOAD8(data,2),B_RTP_LOAD8(data,3),B_RTP_LOAD8(data,4),B_RTP_LOAD8(data,5),B_RTP_LOAD8(data,6),B_RTP_LOAD8(data,7)));

#if 0
    const uint8_t streamMuxConfig[6] = {0x40, 0x00, 0x24, 0x20, 0x3f, 0xc0}; /*stereo*/
    const uint8_t streamMuxConfig[6] = {0x40, 0x00, 0x27, 0x10, 0x3f, 0xc0}; /*mono*/
#endif

    uint8_t audioMuxElement[6];
    uint32_t syncword;
    uint32_t frameLength=0;
    uint32_t i;

    uint8_t curByte, lastByte=0;
    uint32_t sizeBytes, sizeBits, sizeBitsAlign;

    uint8_t *rtpBuffer;
    uint32_t byteCount = 0;


    /* Convert StreamMuxConfig to AudioMuxConfig for LOAS (insert a 0-bit to indicate to parse StreamMuxConfig each time) */
    for ( i = 0; i < sizeof(audioMuxElement); i++ ){
        if ( i < sizeof(streamMuxConfig) )  {
            audioMuxElement[i] = streamMuxConfig[i] >> 1;
        }
        else    {
            audioMuxElement[i] = 0;
        }
        if ( i > 0 ){
            audioMuxElement[i] |= (streamMuxConfig[i-1] & 0x1) << 7;
        }
    }

    sizeBits = len*8 + (24 + 44 + 1);
    sizeBitsAlign = sizeBits + (8-(sizeBits&7));
    sizeBytes = sizeBitsAlign /8;

    rtpBuffer = (uint8_t *)BKNI_Malloc(sizeBytes*sizeof(char));

    /* 11 bit syncword 0x2b7 */
    syncword=0x02b7<<13;
    rtpBuffer[0] = (syncword>>16) & 0xff;
    rtpBuffer[1] = (syncword>>8) & 0xff;

    /* 13 bit size of audioMuxElement */
    frameLength = len+sizeof(audioMuxElement);
    rtpBuffer[1] |= (frameLength>>8) & 0x1f;
    rtpBuffer[2] = frameLength & 0xff;

    curByte = B_RTP_LOAD8(data,0);

    audioMuxElement[sizeof(audioMuxElement)-1] |= ((curByte&0xe0) >> 5);
    BKNI_Memcpy(&rtpBuffer[3], audioMuxElement, sizeof(audioMuxElement));

    byteCount = sizeof(audioMuxElement)+3;

        /* Shift in remaining bits */
    for ( i = 0; i < len; i++ ){
        lastByte = (curByte&0x1f) << 3;
        if ( i < (len-1) ){
            curByte = B_RTP_LOAD8(data,i+1);
            lastByte |= ((curByte&0xe0) >> 5);
        }
        rtpBuffer[byteCount++] = lastByte;
    }

    b_rtp_parser_aacloas_begin(parser);
    brtp_parser_add_packet(&parser->parent, pkt, rtpBuffer, sizeBytes);
    b_rtp_parser_aacloas_commit(parser);

    BKNI_Free(rtpBuffer);
    return;
}

static void 
b_rtp_parser_aacloas_discontinuity(brtp_parser_t parser_)
{
    brtp_parser_aacloas_t parser = (brtp_parser_aacloas_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_aacloas_t);
    BDBG_ASSERT(parser->parent.stream.mux);
    brtp_parser_abort(&parser->parent);
    b_rtp_parser_aacloas_begin(parser);
    return;
}


static brtp_parser_mux_stream_t
b_rtp_parser_aacloas_start(brtp_parser_t parser_, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *aacloas_cfg_, size_t aacloas_cfg_len)
{
    unsigned int i;
    brtp_parser_aacloas_t parser = (brtp_parser_aacloas_t)parser_;
    const brtp_parser_aacloas_stream_cfg *aacloas_cfg = aacloas_cfg_;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_aacloas_t);
    BDBG_ASSERT(parser->parent.stream.mux==NULL);
    BDBG_ASSERT(cfg);
    BDBG_ASSERT(aacloas_cfg);
    BDBG_ASSERT(sizeof(*aacloas_cfg)==aacloas_cfg_len);

    parser->stream_cfg = *aacloas_cfg;
    brtp_parser_mux_attach(mux, parser_, cfg);

    for(i=0; i<sizeof(streamMuxConfig); i++)
    {
        streamMuxConfig[i] =  aacloas_cfg->config[i];
    }

    return  &parser->parent.stream;

}

void 
b_rtp_parser_aacloas_stop(brtp_parser_t parser_)
{
    brtp_parser_aacloas_t parser = (brtp_parser_aacloas_t)parser_;
    BDBG_OBJECT_ASSERT(parser, brtp_parser_aacloas_t);
    brtp_parser_stop(&parser->parent);
    return;
}

static void
b_rtp_parser_aacloas_destroy(brtp_parser_t parser_)
{
    brtp_parser_aacloas_t parser = (brtp_parser_aacloas_t)parser_;

    BDBG_OBJECT_ASSERT(parser, brtp_parser_aacloas_t);
    b_rtp_parser_aacloas_stop(parser_);

    BDBG_OBJECT_DESTROY(parser, brtp_parser_aacloas_t);
    BKNI_Free(parser);
    return;
}

brtp_parser_t 
brtp_parser_aacloas_create(const brtp_parser_aacloas_cfg *cfg)
{
    brtp_parser_aacloas_t parser;

    BDBG_ASSERT(cfg);
    parser = BKNI_Malloc(sizeof(*parser));
    if (!parser) {
        BDBG_ERR(("brtp_parser_aacloas_create: out of memory"));
        return NULL;
    }
    BDBG_OBJECT_INIT(parser, brtp_parser_aacloas_t);
    brtp_parser_init(&parser->parent);
    parser->parent.packet = b_rtp_parser_aacloas_packet;
    parser->parent.discontinuity = b_rtp_parser_aacloas_discontinuity;
    parser->parent.start = b_rtp_parser_aacloas_start;
    parser->parent.stop = b_rtp_parser_aacloas_stop;
    parser->parent.destroy = b_rtp_parser_aacloas_destroy;
    parser->parent.type = brtp_parser_aacloas;
    parser->cfg = *cfg;

    BDBG_ERR(("brtp_parser_aacloas_create: done"));

    return &parser->parent;
}
