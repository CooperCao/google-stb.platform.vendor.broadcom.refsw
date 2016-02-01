/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * RTP MPEG4-Part2 Audio Video parser library
 *   RFC 3016 module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brtp_parser_mpeg4part2.h"
#include "brtp_packet.h"
#include "bbase64.h"
#include "brtp_util.h"

BDBG_MODULE(brtp_parser_mpeg4part2);

BDBG_OBJECT_ID(brtp_parser_mpeg4part2_t);

struct brtp_parser_mpeg4part2 {
	struct brtp_parser parent; /* must be the first element */
	BDBG_OBJECT(brtp_parser_mpeg4part2_t)
	uint8_t *nal_header;
	brtp_parser_mpeg4part2_cfg cfg;
	enum {brtp_state_single,  brtp_state_multi} state;
	unsigned multi_len;
	const uint8_t *meta_base;
	size_t meta_len;
	enum {brtp_nal_state_none, brtp_nal_state_idr} nal_state;
};
typedef struct brtp_parser_mpeg4part2 *brtp_parser_mpeg4part2_t;

static const brtp_parser_type b_rtp_parser_mpeg4part2 = {
	"MP4P2",
	brtp_stream_video
};

const brtp_parser_type *brtp_parser_mpeg4part2 = &b_rtp_parser_mpeg4part2;

static bool
b_rtp_parser_mpeg4part2_data(brtp_parser_mpeg4part2_t parser, brtp_packet_t packet, const void *data, size_t len)
{
	return brtp_parser_add_packet(&parser->parent, packet, data, len);
}

static void
b_rtp_parser_mpeg4part2_add_meta(brtp_parser_mpeg4part2_t parser)
{
	brtp_io_vec *vec;

	if (parser->meta_len) {
		BDBG_MSG(("b_rtp_parser_mpeg4part2_add_meta:%#lx adding %u bytes of meda data", (unsigned long)parser, parser->meta_len));
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
b_rtp_parser_mpeg4part2_packet(brtp_parser_t parser_, brtp_packet_t pkt, const void *data, size_t len)
{
	brtp_parser_mpeg4part2_t parser = (brtp_parser_mpeg4part2_t)parser_;
		
	BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4part2_t);
	BDBG_ASSERT(parser->parent.stream.mux);

	BDBG_MSG(("Len %d Data %x %x %x %x %x %x %x\n\n",len,B_RTP_LOAD8(data,0),B_RTP_LOAD8(data,1),B_RTP_LOAD8(data,2),B_RTP_LOAD8(data,3),B_RTP_LOAD8(data,4),B_RTP_LOAD8(data,5),B_RTP_LOAD8(data,6),B_RTP_LOAD8(data,7)));

	brtp_parser_begin(&parser->parent);
	b_rtp_parser_mpeg4part2_data(parser, pkt, (uint8_t *)data, len);
	brtp_parser_commit(&parser->parent);

  	return;

}

static void 
b_rtp_parser_mpeg4part2_discontinuity(brtp_parser_t parser_)
{
	brtp_parser_mpeg4part2_t parser = (brtp_parser_mpeg4part2_t)parser_;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4part2_t);
	if (parser->state == brtp_state_multi) { /* accumulating packets */
		BDBG_WRN(("b_rtp_parser_mpeg4part2_discontinuity: %#lx dropping accumulated data", (unsigned long)parser));
		brtp_parser_abort(&parser->parent);
		parser->state = brtp_state_single;
	}
	return;
}

brtp_parser_mpeg4part2_cfg b_rtp_parser_mpeg4part2_default_cfg = {
	4+128,/* meta_len nal_hdr, SPS + PPS */
	NULL 	/* meta */
};

void
brtp_parser_mpeg4part2_default_cfg(brtp_parser_mpeg4part2_cfg *cfg)
{
	*cfg = b_rtp_parser_mpeg4part2_default_cfg;
	return;
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
b_rtp_parser_mpeg4part2_start(brtp_parser_t parser_, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *sprop, size_t sprop_len)
{
	brtp_parser_mpeg4part2_t parser = (brtp_parser_mpeg4part2_t)parser_;
	uint8_t *meta = (uint8_t *)parser->cfg.meta;
	size_t meta_left = parser->cfg.meta_len;
       int ret;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4part2_t);
	BDBG_ASSERT(cfg);

	BSTD_UNUSED(sprop);
	BSTD_UNUSED(sprop_len);
	
	meta_left = parser->cfg.meta_len/2;
	ret = bhex_decode(parser->cfg.meta, parser->cfg.meta_len,meta,meta_left);

/*	int i;
	for(i=0; i<41; i++)
		BDBG_MSG(("meta[%d] = %x\n\n",i , meta[i]));
*/ 

	brtp_parser_mux_attach(mux, parser_, cfg);
	parser->meta_base = meta;
	brtp_parser_begin(&parser->parent);

       meta += 41;
	parser->meta_len = meta - parser->meta_base;
	parser->nal_state = brtp_nal_state_none;
	BDBG_MSG(("meta %d bytes", parser->meta_len));

	if(parser->meta_len) {
		brtp_parser_begin(&parser->parent);
		b_rtp_parser_mpeg4part2_add_meta(parser);
		brtp_parser_commit(&parser->parent);
	}
	parser->state = brtp_state_single;
	parser->multi_len = 0;
	return  &parser->parent.stream;
}

static void 
b_rtp_parser_mpeg4part2_stop(brtp_parser_t parser_)
{
	brtp_parser_mpeg4part2_t parser = (brtp_parser_mpeg4part2_t)parser_;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4part2_t);
	brtp_parser_stop(&parser->parent);
	parser->state = brtp_state_single;
	return;
}

static void
b_rtp_parser_mpeg4part2_destroy(brtp_parser_t parser_)
{
	brtp_parser_mpeg4part2_t parser = (brtp_parser_mpeg4part2_t)parser_;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_mpeg4part2_t);
	b_rtp_parser_mpeg4part2_stop(parser_);
	BDBG_OBJECT_DESTROY(parser, brtp_parser_mpeg4part2_t);
	BKNI_Free(parser);
	return;
}

brtp_parser_t 
brtp_parser_mpeg4part2_create(const brtp_parser_mpeg4part2_cfg *cfg)
{
	brtp_parser_mpeg4part2_t parser;

	BDBG_CASSERT(B_RTP_GET_BIT((uint8_t)0x80, 0));
	BDBG_CASSERT(B_RTP_GET_BIT((uint8_t)0x40, 0)==0);
	BDBG_CASSERT(B_RTP_GET_BITS((uint8_t)0x80, 0,1)==2);

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(cfg->meta);
	parser = BKNI_Malloc(sizeof(*parser));

	if (!parser) {
		BDBG_ERR(("out of memory in brtp_parser_mpeg4part2_create"));
		return NULL;
	}
	BDBG_OBJECT_INIT(parser, brtp_parser_mpeg4part2_t);
	brtp_parser_init(&parser->parent);
	parser->parent.packet = b_rtp_parser_mpeg4part2_packet;
	parser->parent.discontinuity = b_rtp_parser_mpeg4part2_discontinuity;
	parser->parent.start = b_rtp_parser_mpeg4part2_start;
	parser->parent.stop = b_rtp_parser_mpeg4part2_stop;
	parser->parent.destroy = b_rtp_parser_mpeg4part2_destroy;
	parser->parent.type = brtp_parser_mpeg4part2;
	parser->cfg = *cfg;
	parser->state = brtp_state_single;

	return &parser->parent;
}


