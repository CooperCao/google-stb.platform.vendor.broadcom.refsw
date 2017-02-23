/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Media parser library, MPEG1 parser
 * 
 *******************************************************************************/
#include "bstd.h"
#include "biobits.h"
#include "bkni.h"
#include "bmpeg1_parser.h"

BDBG_MODULE(bmpeg1_parser);

BDBG_OBJECT_ID(bmpeg1_parser_t);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x)  */

typedef enum b_mpeg1_parser_state {
	b_mpeg1_parser_state_capture,
	b_mpeg1_parser_state_parsing,
	b_mpeg1_parser_state_parsing_pack,
	b_mpeg1_parser_state_discard,
	b_mpeg1_parser_state_resync
} b_mpeg1_parser_state;

struct bmpeg1_parser {
	BDBG_OBJECT(bmpeg1_parser_t)
	BLST_S_HEAD(b_mpeg1_handlers, bmpeg1_parser_handler) handlers;
	b_mpeg1_parser_state state;
	batom_accum_t acc;
	struct {
		bmpeg1_parser_handler *handler;
		uint16_t stream_id;
		bmedia_packet_header pts;
		size_t size; /* either size of packet to capture or number of bytes to drop */
	} packet;
	uint64_t acc_off;
	bmpeg1_parser_cfg cfg;
};


static bmpeg1_parser_action
b_mpeg1_parser_stream_drop(bmpeg1_parser_stream *stream, unsigned flags, batom_accum_t src, batom_cursor *payload_start, size_t len)
{
	BSTD_UNUSED(payload_start);
	BSTD_UNUSED(len);
	BSTD_UNUSED(stream);
	BSTD_UNUSED(flags);
	BSTD_UNUSED(src);
	return bmpeg1_parser_action_none;
}


void 
bmpeg1_parser_stream_init(bmpeg1_parser_stream *stream, uint16_t stream_id)
{
	BDBG_ASSERT(stream);
	stream->stream_id = stream_id;
	stream->flags = 0;
	stream->pts = 0;	
	stream->payload = b_mpeg1_parser_stream_drop;
	
	return;
}

int
bmpeg1_parser_stream_feed(bmpeg1_parser_stream *stream, batom_accum_t src, batom_cursor *cursor)
{
	batom_checkpoint check;
	int result=BMPEG1_RESULT_EOS;
	uint32_t word;
	int payload_len;

	BDBG_ASSERT(cursor);
	
	while(1){       			       
		BATOM_SAVE(cursor, &check);

		word = batom_cursor_uint32_be(cursor);
		if( (word&0xFFFFFF00) != 0x00000100) {
			BDBG_MSG(("b_mpeg1_parser_stream_feed: %#lx invalid packet_start_code_prefix 0x%08x!= 0x000001XX", (unsigned long)stream, word));
			goto err_out_of_sync;
		}
		word &= 0xFF;		

		if(!b_mpeg1_test_stream_id(word)) {			
			if(word == 0xBA){
				batom_cursor_skip(cursor, (
							   4 /* 0010 */ + 
							   3 /* system_clock_reference [32..30] */ +
							   1 /* marker_bit */ +
							   15 /* system_clock_reference [29..15] */ +
							   1 /* marker_bit */ +
							   15 /* system_clock_reference [14..0] */ +
							   1 /* marker_bit */ +
							   1 /* marker_bit */ +
							   22 /* mux_rate */ +
							   1 /* marker_bit */
							   )/8);
				continue;
			}
			else if(word == 0xBB) {
				payload_len = batom_cursor_uint16_be(cursor);
                if(BATOM_IS_EOF(cursor)) {
					return result;
                }
		        if(batom_cursor_skip(cursor, payload_len)!=(unsigned)payload_len) {
					return result;
				}
				continue;
			}
			else {
				goto err_out_of_sync;
			}
		}

		payload_len = batom_cursor_uint16_be(cursor);

		if(word != stream->stream_id) {
			result = word;
			goto next_packet;
		}

		for(;;) {
			/* drop stuffing bytes */
			word = batom_cursor_next(cursor);
			if ((int)word==BATOM_EOF) {
				return result;
			}
			if (word!=0xFF) {
				break;
			}
		}
		
		/* Because of similarity in PES structure, we will re-use same code for MPEG1 parsing*/
		if(B_GET_BITS(word, 7, 6)==0x01) {					
			batom_cursor_byte(cursor); /* skip STD_buffer */
			word = batom_cursor_byte(cursor);			
		} 
			       
		if((B_GET_BITS(word, 7, 4)&0x0E)==0x02) {
			uint32_t pts;
			uint32_t pts_bits;
			bmpeg1_parser_action action;

			pts_bits = B_GET_BITS(word, 3, 1); /* PTS 32..30 */
			pts = pts_bits<<29;
			pts_bits = batom_cursor_uint32_be(cursor); /* PTS  29..15 14..0 */
			pts |= B_GET_BITS(pts_bits, 31, 17)<<14; /* PTS  29..15  */
			pts |= B_GET_BITS(pts_bits, 15, 2); /* PTS  14..1 */
		
			if(B_GET_BIT(word, 4)) {
				batom_cursor_skip(cursor, 5); /* skip DTS */
			}
			
			stream->pts = pts;
			action = stream->payload(stream, 0, src, cursor, payload_len);
			if (action==bmpeg1_parser_action_none) {					
                goto next_packet;
			} else { 
				batom_cursor_rollback(cursor, &check);
				result = BMPEG1_RESULT_HOLD; 
				goto do_exit;		
			}
		} 					

next_packet:
		batom_cursor_rollback(cursor, &check);
		BATOM_SKIP(cursor, payload_len+6);       

		if(BATOM_IS_EOF(cursor)) {
			return result;
		}	
	}	

do_exit:	
	return result;
	
 err_out_of_sync:

	batom_cursor_rollback(cursor, &check);	
	result = BMPEG1_RESULT_SYNC_ERROR;
	goto do_exit;
}


static bool 
b_mpeg1_parser_skip(bmpeg1_parser_t parser, size_t len)
{
	parser->state = b_mpeg1_parser_state_discard;
	parser->packet.handler = NULL;
	parser->packet.size = len;
	return true;
}

static bool 
b_mpeg1_parser_capture(bmpeg1_parser_t parser, bmpeg1_parser_handler *handler, unsigned stream_id, size_t len, bool pts_valid, uint32_t pts)
{
	BSTD_UNUSED(handler);
	BSTD_UNUSED(len);
	BDBG_MSG_TRACE(("b_mpeg1_parser_capture: %#lx %#lx 0x%02x:%u", (unsigned long)parser, (unsigned long)handler, stream_id, len ));
	parser->state = b_mpeg1_parser_state_capture;
	parser->packet.handler = handler;
	parser->packet.stream_id = stream_id;
	parser->packet.size = len;
	BMEDIA_PACKET_HEADER_INIT(&parser->packet.pts);
	parser->packet.pts.pts_valid = pts_valid;
	parser->packet.pts.pts = pts;
	return true;
}

static bool 
b_mpeg1_parser_resync(bmpeg1_parser_t parser)
{
	BDBG_MSG(("b_mpeg1_parser_resync: %#lx", (unsigned long)parser));
	parser->state = b_mpeg1_parser_state_resync;
	return true;
}

static bool 
b_mpeg1_parser_parse(bmpeg1_parser_t parser)
{
	BDBG_MSG_TRACE(("b_mpeg1_parser_parse: %#lx", (unsigned long)parser));
	parser->state = b_mpeg1_parser_state_parsing;
	return true;
}

static bool 
b_mpeg1_parser_system_header(bmpeg1_parser_t parser, batom_cursor *cursor)
{
	unsigned header_length;

	BDBG_MSG_TRACE(("b_mpeg1_parser_system_header: %#lx", (unsigned long)parser));
	header_length = batom_cursor_uint16_be(cursor);
	if(!BATOM_IS_EOF(cursor)) {
		return b_mpeg1_parser_skip(parser, header_length);
	}
	return false;
}

bool
b_mpeg1_test_stream_id(unsigned stream_id)
{
	/* Table 1 -- stream_id table */
	switch(stream_id) {
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
		return true;
	default:
		return ((stream_id&0xC0)==0xC0);
	}
}

static bmpeg1_parser_handler *
b_mpeg1_test_stream(bmpeg1_parser_t parser, unsigned stream_id)
{
	bmpeg1_parser_handler *handler;

	for(handler=BLST_S_FIRST(&parser->handlers);handler;handler=BLST_S_NEXT(handler, link)) {
		if(handler->stream_id==(handler->stream_id_mask&stream_id)) {
			return handler;
		}
	}
	return NULL;
}

static bool 
b_mpeg1_parser_packet(bmpeg1_parser_t parser, batom_cursor *cursor, unsigned stream_id)
{
	unsigned packet_length;
	bmpeg1_parser_handler *handler;

	BDBG_MSG_TRACE(("b_mpeg1_parser_packet: %#lx", (unsigned long)parser));

	/* 2.4.3.3. Packet layer */
	packet_length = batom_cursor_uint16_be(cursor);
	if(BATOM_IS_EOF(cursor)) {
		goto rollback;
	}
	handler = b_mpeg1_test_stream(parser, stream_id);
	if(handler && stream_id>0xBC ) {
		batom_cursor start;
		uint32_t pts=0;
		bool pts_valid=false;
		uint32_t pts_bits;
		size_t header_length;
		int byte;

		BATOM_CLONE(&start,cursor);
		/* drop reserved_stream, private_stream_1, padding_stream and private_stream_2 */
		for(;;) {
			/* drop stuffing bytes */
			byte = batom_cursor_next(cursor);
			if (byte==BATOM_EOF) {
				goto rollback;
			}
			if (byte!=0xFF) {
				break;
			}
		}
		if(B_GET_BITS(byte, 7, 6)==0x01) {
			batom_cursor_byte(cursor); /* skip STD_buffer */
			byte = batom_cursor_byte(cursor);
		} 
		if((B_GET_BITS(byte, 7, 4)&0x0E)==0x02) {
			pts_bits = B_GET_BITS(byte, 3, 1); /* PTS 32..30 */
			pts = pts_bits<<29;
			pts_bits = batom_cursor_uint32_be(cursor); /* PTS  29..15 14..0 */
			pts |= B_GET_BITS(pts_bits, 31, 17)<<14; /* PTS  29..15  */
			pts |= B_GET_BITS(pts_bits, 15, 2); /* PTS  14..1 */
			pts_valid = true;
			if(B_GET_BIT(byte, 4)) {
				batom_cursor_skip(cursor, 5); /* skip DTS */
			}
		} 
		if(BATOM_IS_EOF(cursor)) {
			goto rollback;
		}
		header_length = batom_cursor_distance(&start, cursor);
		if(packet_length>=header_length) {
			return b_mpeg1_parser_capture(parser, handler, stream_id, packet_length-header_length, pts_valid, pts);
		}
		/* invalid length */
		return b_mpeg1_parser_resync(parser);
	} else {
		/* drop reserved_stream, private_stream_1, padding_stream and private_stream_2 */
		return b_mpeg1_parser_skip(parser, packet_length);
	}
rollback:
	return false;
}



static bool 
b_mpeg1_parser_pack_scode(bmpeg1_parser_t parser, batom_cursor *cursor, uint32_t prev_start_code)
{
	uint32_t start_code;

	/* 2.4.3.2 Pack Layer */
	if (prev_start_code == 0x01BA) {
		batom_cursor_skip(cursor, (
			4 /* 0010 */ + 
			3 /* system_clock_reference [32..30] */ +
			1 /* marker_bit */ +
			15 /* system_clock_reference [29..15] */ +
			1 /* marker_bit */ +
			15 /* system_clock_reference [14..0] */ +
			1 /* marker_bit */ +
			1 /* marker_bit */ +
			22 /* mux_rate */ +
			1 /* marker_bit */
			)/8);
		start_code = batom_cursor_uint32_be(cursor);
		if(!BATOM_IS_EOF(cursor)) {
			BDBG_MSG_TRACE(("b_mpeg1_parser_pack_scode: %#lx scode: %#x", (unsigned long)parser, start_code));
			if (start_code==0x1BB) {
				return b_mpeg1_parser_system_header(parser, cursor);
			} else if ( (start_code&0xFFFFFF00) == 0x100) {
				unsigned stream_id = start_code & 0xFF;
				if (b_mpeg1_test_stream_id(stream_id)) {
					return b_mpeg1_parser_packet(parser, cursor, stream_id);
				} else {
					return b_mpeg1_parser_resync(parser); 
				}
			} else {
				return b_mpeg1_parser_resync(parser);
			}
		}
		goto rollback;
	} else if (prev_start_code == 0x01B9) {
		/* end of stream */
		return b_mpeg1_parser_parse(parser);
	} else {
		/* invalid start code */
		return b_mpeg1_parser_resync(parser);
	} 
rollback:
	return false;
}

static bool 
b_mpeg1_parser_pack_start(bmpeg1_parser_t parser, batom_cursor *cursor)
{
	uint32_t start_code;
	batom_checkpoint chk;

	BDBG_MSG_TRACE(("b_mpeg1_parser_pack_start: %#lx", (unsigned long)parser));

	/* 2.4.3.2 Pack Layer */
	BATOM_SAVE(cursor, &chk);
	start_code = batom_cursor_uint32_be(cursor);
	if (!BATOM_IS_EOF(cursor)) {
		if (b_mpeg1_parser_pack_scode(parser, cursor, start_code)) {
			return true;
		}
	}
	batom_cursor_rollback(cursor, &chk);
	return false;
}

static bool 
b_mpeg1_parser_pack_continue(bmpeg1_parser_t parser, batom_cursor *cursor)
{
	uint32_t start_code;
	batom_checkpoint chk;
	unsigned stream_id;

	/* 2.4.3.2 Pack Layer */
	BATOM_SAVE(cursor, &chk);
	start_code = batom_cursor_uint32_be(cursor);
	if (!BATOM_IS_EOF(cursor)) {
		BDBG_MSG_TRACE(("b_mpeg1_parser_pack_continue: %#lx scode: %#x", (unsigned long)parser, start_code));
		if ( (start_code&0xFFFFFF00) == 0x100) {
			stream_id = start_code & 0xFF;
			if(b_mpeg1_test_stream_id(stream_id)) {
				if(b_mpeg1_parser_packet(parser, cursor, stream_id)) {
					return true;
				}
			} else {
				if (b_mpeg1_parser_pack_scode(parser, cursor, start_code)) {
					return true;
				}
			}
		} else {
			batom_cursor_rollback(cursor, &chk);
			return b_mpeg1_parser_resync(parser);
		}
	}
	batom_cursor_rollback(cursor, &chk);
	return false;
}

static bool 
b_mpeg1_parser_doresync(bmpeg1_parser_t parser, batom_cursor *cursor)
{
	int byte;
	batom_checkpoint chk;

	BDBG_CASSERT(BATOM_EOF<0);
	/* search 00 or EOF */
	for(;;) {
		BATOM_SAVE(cursor, &chk);
		byte = batom_cursor_next(cursor);
		if(byte<=0)  {
			break;
		}
	}
	/* found 00 */
	/* look for 00 00 01 prefix */
	if (byte==0x00) {
		byte = batom_cursor_next(cursor);
		if (byte==0x00) {
			byte = batom_cursor_next(cursor);
			if (byte==0x01) {
				/* found start code prefix */
				batom_cursor_rollback(cursor, &chk);
				return b_mpeg1_parser_parse(parser);
			} 
		}
	}
	if (byte==BATOM_EOF) {
		/* reached last byte */
		batom_cursor_rollback(cursor, &chk);
		return false;
	} else {
		return true;
	}
}


size_t
bmpeg1_parser_feed(bmpeg1_parser_t parser, batom_pipe_t pipe)
{
	size_t len=0;
	batom_t atom;
	bmpeg1_parser_action action = bmpeg1_parser_action_none;

	BDBG_OBJECT_ASSERT(parser, bmpeg1_parser_t);
	BDBG_MSG_TRACE(("bmpeg1_parser_feed>:%#lx %#lx %#lu", (unsigned long)parser, (unsigned long)pipe, (unsigned long)0 + batom_accum_len(parser->acc)));
	do {
		bool want_continue;
		atom=batom_pipe_pop(pipe);
		BDBG_MSG(("bmpeg1_parser_feed:%#lx atom:%#lx", (unsigned long)parser, (unsigned long)atom));
		if (atom) {
			batom_accum_add_atom(parser->acc, atom);
			batom_release(atom);
			len += batom_len(atom);
		}
		do {
			size_t acc_len = batom_accum_len(parser->acc);
			batom_cursor cursor;
			want_continue = false;

			BDBG_MSG_TRACE(("bmpeg1_parser_feed:%#lx acc_len:%u", (unsigned long)parser, acc_len));
			batom_cursor_from_accum(&cursor, parser->acc);
			switch(parser->state) {
			case b_mpeg1_parser_state_capture:
				if(acc_len>=parser->packet.size) {
					batom_t atom;
					batom_cursor first;

					BATOM_CLONE(&first,&cursor);
					batom_cursor_skip(&cursor, parser->packet.size);
					atom = batom_accum_extract(parser->acc, &first, &cursor, bmedia_atom, &parser->packet.pts);
					if (atom) {
						BDBG_MSG_TRACE(("bmpeg1_parser_feed: %#lx payload %#lx:%#lx:%u", (unsigned long)parser, (unsigned long)parser->packet.handler, (unsigned long)atom, parser->packet.size));
						if(parser->packet.handler) {
                            action = parser->packet.handler->handler(parser->packet.handler, parser->packet.stream_id, atom);
                        } else {
                            batom_release(atom);
                            action = bmpeg1_parser_action_none;
                        }
                    }
                    parser->packet.handler = NULL;
                    parser->packet.size = 0;
                    want_continue = b_mpeg1_parser_parse(parser);
				}
				break;
			case b_mpeg1_parser_state_parsing_pack:
				want_continue = b_mpeg1_parser_pack_start(parser, &cursor);
				break;
			case b_mpeg1_parser_state_parsing:
				want_continue = b_mpeg1_parser_pack_continue(parser, &cursor);
				break;
			case b_mpeg1_parser_state_discard:
				acc_len = batom_cursor_skip(&cursor, parser->packet.size); 
				BDBG_ASSERT(acc_len<=parser->packet.size);
				parser->packet.size -= acc_len;
				if (parser->packet.size==0) {
					want_continue = b_mpeg1_parser_parse(parser);
				}
				break;
			case b_mpeg1_parser_state_resync:
				want_continue = b_mpeg1_parser_doresync(parser, &cursor);
				break;
			default:
				BDBG_ASSERT(0);
				break;
			}
			parser->acc_off += batom_cursor_pos(&cursor);
			batom_accum_trim(parser->acc, &cursor);
			if(action == bmpeg1_parser_action_return) {
				goto done;
			}
		} while(want_continue);
	} while(atom);
done:
	return len;
}

void 
bmpeg1_parser_default_cfg(bmpeg1_parser_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->unused = 0;
	return;
}

void 
bmpeg1_parser_destroy(bmpeg1_parser_t parser)
{
	BDBG_OBJECT_ASSERT(parser, bmpeg1_parser_t);
	batom_accum_destroy(parser->acc);
	BDBG_OBJECT_DESTROY(parser, bmpeg1_parser_t);
	BKNI_Free(parser);
	return;
}

void 
bmpeg1_parser_remove_handler(bmpeg1_parser_t parser, bmpeg1_parser_handler *handler)
{
	BDBG_OBJECT_ASSERT(parser, bmpeg1_parser_t);
	BDBG_ASSERT(handler);
	BLST_S_REMOVE(&parser->handlers, handler, bmpeg1_parser_handler, link);
    if(parser->packet.handler == handler) {
        parser->packet.handler = NULL; /* clear currently active handler */
    }
	return;
}

void 
bmpeg1_parser_install_handler(bmpeg1_parser_t parser, bmpeg1_parser_handler *handler, uint16_t stream_id, uint16_t stream_id_mask, bmpeg1_parser_handler_cb packet_handler)
{
	BDBG_OBJECT_ASSERT(parser, bmpeg1_parser_t);
	BDBG_ASSERT(handler);
	handler->stream_id_mask  = stream_id_mask;
	handler->stream_id = stream_id;
	handler->handler = packet_handler;
	BLST_S_INSERT_HEAD(&parser->handlers, handler, link);
	return;
}

void
bmpeg1_parser_reset(bmpeg1_parser_t parser)
{
	BDBG_OBJECT_ASSERT(parser, bmpeg1_parser_t);
	batom_accum_clear(parser->acc);
	parser->state = b_mpeg1_parser_state_parsing_pack;
	parser->packet.handler = NULL;
	parser->packet.size = 0;
	parser->acc_off = 0;
	return;
}

bmpeg1_parser_t
bmpeg1_parser_create(batom_factory_t factory, const bmpeg1_parser_cfg *cfg)
{
	bmpeg1_parser_t parser;

	BDBG_ASSERT(cfg);
	BDBG_ASSERT(factory);

	parser = BKNI_Malloc(sizeof(*parser));
	if (!parser) { 
		BDBG_ERR(("bmpeg1_parser_create: out of memory %u", (unsigned)sizeof(*parser)));
		goto err_alloc_parser;
	}
	BDBG_OBJECT_INIT(parser, bmpeg1_parser_t);
	parser->cfg = *cfg;
	BLST_S_INIT(&parser->handlers);
	parser->acc = batom_accum_create(factory);
	if(!parser->acc) {
		goto err_alloc_accum;
	}
	bmpeg1_parser_reset(parser);
	return parser;

err_alloc_accum:
	BKNI_Free(parser);
err_alloc_parser:
	return NULL;
}


