/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * MPEG-2 PES Parser/Demux library
 *
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg2pes_parser.h"
#include "biobits.h"
#include "bkni.h"

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

BDBG_MODULE(bmpeg2pes_parser);

typedef enum {
	b_mpeg2pes_parser_result_match, 
	b_mpeg2pes_parser_result_nosync, 
	b_mpeg2pes_parser_result_nomatch, 
	b_mpeg2pes_parser_result_error, 
	b_mpeg2pes_parser_result_more
} b_mpeg2pes_parser_result;


bmpeg2ts_parser_action 
b_mpeg2pes_parser_stream_drop(bmpeg2pes_parser_stream *stream, unsigned flags, batom_accum_t src, batom_cursor *payload_start, size_t len)
{
	BSTD_UNUSED(payload_start);
	BSTD_UNUSED(len);
	BSTD_UNUSED(stream);
	BSTD_UNUSED(flags);
	BSTD_UNUSED(src);
	return bmpeg2ts_parser_action_skip;
}


void 
bmpeg2pes_parser_stream_init(bmpeg2pes_parser_stream *stream, uint16_t stream_id)
{
	BDBG_ASSERT(stream);
	stream->stream_id = stream_id;
	stream->flags = 0;
	stream->npackets = 0;
	stream->pts = 0;	
	stream->payload = b_mpeg2pes_parser_stream_drop;
	
	return;
}


int
bmpeg2pes_parser_stream_feed(bmpeg2pes_parser_stream *stream, batom_accum_t src, batom_cursor *cursor)
{
    batom_checkpoint check;
    int result=BMPEG2TS_RESULT_EOS;

    BDBG_ASSERT(cursor);

    /* ISO/IEC 13818-1 */

    while(1){
        uint32_t word;
        unsigned flags = stream->flags;
        int payload_len;

        BATOM_SAVE(cursor, &check);

        word = batom_cursor_uint32_be(cursor);
        if( (word&0xFFFFFF00) != 0x00000100) {
            if(BATOM_IS_EOF(cursor)) {goto do_exit;}
            BDBG_MSG(("b_mpeg2pes_parser_stream_feed: %#lx invalid packet_start_code_prefix 0x%08x!= 0x000001XX", (unsigned long)stream, word));
            goto err_out_of_sync;
        }
        word &= 0xFF;

        payload_len = batom_cursor_uint16_be(cursor);

        switch(bmpeg2pes_decode_stream_id(word)) {
            case bmpeg2pes_stream_id_type_invalid:
                goto err_out_of_sync;
            case bmpeg2pes_stream_id_type_raw:
                result = word;
                goto next_packet;
            default:
                break;
        }

        if(word != stream->stream_id) {
            result = word;
            goto next_packet;
        }

        word = batom_cursor_uint24_be(cursor);
        flags |= word&BMPEG2PES_DATA_ALIGMENT;

        if (B_GET_BIT(word, 15)) {
            uint32_t pts;
            uint32_t pts_word;
            bmpeg2ts_parser_action action;

            pts_word = batom_cursor_byte(cursor);

            pts =
                /* pts_32_30 */(B_GET_BITS(pts_word, 3, 1)<<29);
            pts_word = batom_cursor_uint32_be(cursor);

            pts |=
                /* pts_29_15 */(B_GET_BITS(pts_word, 31, 17)<<14) |
                /* pts_14_1 */B_GET_BITS(pts_word, 15, 2);
            flags |= BMPEG2PES_PTS_VALID;
            stream->pts = pts;

            action = stream->payload(stream, flags, src, cursor, payload_len);
            if (action==bmpeg2ts_parser_action_skip) {
                goto next_packet;
            } else { /* bmpeg2ts_parser_action_hold */
                batom_cursor_rollback(cursor, &check);
                result = BMPEG2TS_RESULT_HOLD;
                goto do_exit;
            }
        }

next_packet:
        batom_cursor_rollback(cursor, &check);
        BATOM_SKIP(cursor, payload_len+6);
        if(BATOM_IS_EOF(cursor)) {
            return result;
        }
        stream->flags = flags;

    }

do_exit:
    return result;

err_out_of_sync:
    batom_cursor_rollback(cursor, &check);
    result = BMPEG2TS_RESULT_SYNC_ERROR;
    goto do_exit;
}

static void 
b_mpeg2pes_drop_packet (void *packet_cnxt, batom_accum_t src, batom_cursor *payload_start, size_t len, const bmpeg2pes_atom_info *info)
{
	BSTD_UNUSED(packet_cnxt);
	BSTD_UNUSED(info);
	BSTD_UNUSED(src);
	BSTD_UNUSED(info);
	batom_cursor_skip(payload_start, len);
	return;
}

void 
bmpeg2pes_parser_reset(bmpeg2pes_parser *pes)
{
	pes->state = bmpeg2pes_parser_state_sync;
	pes->pkt_len = 0;
	pes->data_len = 0;
	pes->info.pes_id = 0;
	pes->info.flags = 0;
	pes->info.pts = 0;
	pes->info.data_offset = 0;
	pes->hold_enable = false;
	batom_accum_clear(pes->pes_header);
	return;
}

int
bmpeg2pes_parser_init(batom_factory_t factory, bmpeg2pes_parser *pes, uint8_t id)
{
	pes->pes_header = batom_accum_create(factory);
	if(!pes->pes_header) {
		return -1;
	}
	pes->id = id;
	pes->packet_cnxt = NULL;
	pes->packet = b_mpeg2pes_drop_packet;
	bmpeg2pes_parser_reset(pes);
	return 0;
}

bmpeg2pes_stream_id_type 
bmpeg2pes_decode_stream_id(unsigned stream_id)
{
	BDBG_ASSERT(stream_id<0x100);
	switch(stream_id) {
	case 0xBC: /* program_stream_map */
	case 0xBE: /* padding_stream */
	case 0xBF: /* private_stream_2 */ 
	case 0xF0: /* ECM_stream */
	case 0xF1: /* EMM_stream */
	case 0xF2: /* ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A or ISO/IEC 13818- 6_DSMCC_stream */
	case 0xF8: /* ITU-T Rec. H.222.1 type E */ 
	case 0xFF: /* program_stream_directory */
		return bmpeg2pes_stream_id_type_raw;
	default:
		if(stream_id >= 0xBD) {
			return bmpeg2pes_stream_id_type_data;
		} else {
			return bmpeg2pes_stream_id_type_invalid;
		} 
	}
}

static b_mpeg2pes_parser_result
b_mpeg2pes_parser_hdr(bmpeg2pes_parser *pes, batom_cursor *cursor)
{
	/* table 2-17 ISO/IEC 13818-1  */
	if(batom_cursor_reserve(cursor, 9)==9) {
		uint32_t word;
		unsigned flags = pes->info.flags;
		unsigned off;

		word = batom_cursor_uint32_be(cursor);
		if( (word&0xFFFFFF00) != 0x00000100) {
			BDBG_MSG(("b_mpeg2pes_parser_hdr: %#lx invalid packet_start_code_prefix 0x%08x!= 0x000001XX", (unsigned long)pes, word));
			return b_mpeg2pes_parser_result_nosync;
		}
		word &= 0xFF;
		pes->info.pes_id = word;
		if(pes->id!=BMPEG2PES_ID_ANY && pes->id!=word) {
			BDBG_WRN(("b_mpeg2pes_parser_hdr: %#lx unknown pes id %#x(%#x)", (unsigned long)pes, pes->info.pes_id, pes->id));
			return b_mpeg2pes_parser_result_nomatch;
		}
		pes->pkt_len = batom_cursor_uint16_be(cursor);
		switch(bmpeg2pes_decode_stream_id(word)) {
		case bmpeg2pes_stream_id_type_invalid:
			return b_mpeg2pes_parser_result_nosync;
		case bmpeg2pes_stream_id_type_raw:
			pes->data_len = pes->pkt_len;
			batom_cursor_skip(cursor, 6);
			return b_mpeg2pes_parser_result_match;
		default:
			break;
		}
		word = batom_cursor_uint24_be(cursor);
		flags |= word&BMPEG2PES_DATA_ALIGMENT;

		off = 0;
		flags &= ~(BMPEG2PES_PTS_VALID | BMPEG2PES_DTS_VALID);
		if (B_GET_BIT(word, 15)) {
			uint32_t pts;
			uint32_t pts_word;
			pts_word = batom_cursor_byte(cursor);
			pts = 
			 /* pts_32_30 */(B_GET_BITS(pts_word, 3, 1)<<29);
			pts_word = batom_cursor_uint32_be(cursor);

			pts |= 
			 /* pts_29_15 */(B_GET_BITS(pts_word, 31, 17)<<14) |
			 /* pts_14_1 */B_GET_BITS(pts_word, 15, 2);
			flags |= BMPEG2PES_PTS_VALID;
			off = 5;
			pes->info.pts = pts;
		}
        if (B_GET_BIT(word, 14)) {
            uint32_t dts;
            uint32_t dts_word;
            dts_word = batom_cursor_byte(cursor);
            dts =
                /* dts_32_30 */(B_GET_BITS(dts_word, 3, 1)<<29);
            dts_word = batom_cursor_uint32_be(cursor);

            dts |=
                /* dts_29_15 */(B_GET_BITS(dts_word, 31, 17)<<14) |
                /* dts_14_1 */B_GET_BITS(dts_word, 15, 2);
            flags |= BMPEG2PES_DTS_VALID;
            off += 5;
            pes->info.dts = dts;
        }

		word =  word&0xFF; /* PES_header_data_length */
		if(word<off) {
			return b_mpeg2pes_parser_result_error;
		}
		batom_cursor_skip(cursor, word-off);
		if(BATOM_IS_EOF(cursor)) {
			return b_mpeg2pes_parser_result_more;
		}
		pes->info.flags = flags;
		word+=3; /* three bytes extra */
		if(pes->pkt_len>word) {
			pes->data_len = pes->pkt_len - word;
		} else {
			pes->data_len = 0;
		}
		BDBG_MSG_TRACE(("b_mpeg2pes_parser_hdr: %#lx %#x:%#x pes_len:%u", (unsigned long)pes, pes->id, pes->info.pes_id, pes->data_len));
		return b_mpeg2pes_parser_result_match;
	}
	return b_mpeg2pes_parser_result_more;
}
#define B_STR_FLAG(v,name) ((v)?#name " ":"")

bmpeg2ts_parser_action 
bmpeg2pes_parser_feed(bmpeg2pes_parser *pes, unsigned ts_flags, batom_accum_t src, batom_cursor *payload_start, size_t len)
{
	batom_cursor cursor;
	batom_accum_t pes_header;

	BDBG_ASSERT(pes);
	BDBG_ASSERT(payload_start);
	BDBG_ASSERT(src);
	BDBG_MSG_TRACE(("bmpeg2pes_parser_feed: %#lx %#x:%#x %#lx:%u:%#x %s%s%s%s",(unsigned long)pes, pes->id, pes->info.pes_id, (unsigned long)payload_start, len, ts_flags, B_STR_FLAG(ts_flags&BMPEG2TS_ERROR, ERR), B_STR_FLAG(ts_flags&BMPEG2TS_DISCONTINUITY, DISC),B_STR_FLAG(ts_flags&BMPEG2TS_MARKED_DISCONTINUITY, MDISC), B_STR_FLAG(ts_flags&BMPEG2TS_PAYLOAD_UNIT_START,START)));

	if (pes->hold_enable) {
		return bmpeg2ts_parser_action_hold;
	}
	BDBG_MSG_TRACE(("bmpeg2pes_parser_feed: %#lx %#x:%#x state:%u pes_len:%u data_offset:%u", (unsigned long)pes, pes->id, pes->info.pes_id, pes->state, pes->data_len, pes->info.data_offset));

	/* ISO/IEC 13818-1 */
	/* Table 2-18 -- PES packet */
	if(pes->state == bmpeg2pes_parser_state_data) {
		if(pes->data_len > 0) {
			if(pes->info.data_offset >= pes->data_len) {
				BDBG_MSG_TRACE(("bmpeg2pes_parser_feed: %#lx %#x:%#x end of bounded PES pes_len:%u data_offset:%u", (unsigned long)pes, pes->id, pes->info.pes_id, pes->data_len, pes->info.data_offset));
				pes->state = bmpeg2pes_parser_state_hdr;
				if(pes->info.data_offset > pes->data_len) {
					pes->state = bmpeg2pes_parser_state_sync;
					ts_flags |= BMPEG2PES_DISCONTINUITY;
				}
			}
		} else if(len >= 4) {
			uint32_t scode;
			BATOM_CLONE(&cursor, payload_start);
			scode = BATOM_UINT32_BE(&cursor);
			if( scode == (0x100u | pes->info.pes_id)) {
				BDBG_MSG(("bmpeg2pes_parser_feed: %#lx detected new pes packet %#x len (%u)", (unsigned long)pes, scode, pes->info.data_offset));
				pes->state = bmpeg2pes_parser_state_hdr;
			}
		}
	}
	if(ts_flags&BMPEG2TS_PAYLOAD_UNIT_START) {
		if(pes->state == bmpeg2pes_parser_state_data) {
			BDBG_MSG_TRACE(("bmpeg2pes_parser_feed: %#lx %#x:%#x discontinuty at pes_len:%u data_offset:%u", (unsigned long)pes, pes->id, pes->info.pes_id, pes->data_len, pes->info.data_offset));
			ts_flags |= BMPEG2PES_DISCONTINUITY;
		}
		pes->state = bmpeg2pes_parser_state_hdr;
		pes->info.data_offset = 0;
	} 
	switch(pes->state) {
	case bmpeg2pes_parser_state_data:
		pes->info.flags |= ts_flags;
		if(len>0) {
			pes->packet(pes->packet_cnxt, src , payload_start, len, &pes->info);
		}
		pes->info.flags = 0;
		pes->info.data_offset += len;
		break;
	case bmpeg2pes_parser_state_hdr:
		pes_header = pes->pes_header;
		BATOM_CLONE(&cursor, payload_start);
		batom_cursor_skip(payload_start, len);
		batom_accum_append(pes_header, src, &cursor, payload_start);
		batom_cursor_from_accum(&cursor,pes_header);
		switch(b_mpeg2pes_parser_hdr(pes, &cursor)) {
		case b_mpeg2pes_parser_result_match:
			batom_accum_trim(pes_header, &cursor);
			len = batom_accum_len(pes_header);
			if(len>0) {
				batom_cursor_from_accum(&cursor, pes_header);
				pes->packet(pes->packet_cnxt, pes_header, &cursor, len, &pes->info);
                pes->info.flags = 0;
			}
			batom_accum_clear(pes_header);
			pes->info.data_offset += len;
			pes->state = bmpeg2pes_parser_state_data;
			break;
		case b_mpeg2pes_parser_result_more:
			break;
		case b_mpeg2pes_parser_result_nosync:
		case b_mpeg2pes_parser_result_nomatch:
		case b_mpeg2pes_parser_result_error:
			pes->state = bmpeg2pes_parser_state_sync;
			pes->info.flags = BMPEG2PES_DISCONTINUITY;
			batom_accum_clear(pes_header);
			break;
		}
		break;
	case bmpeg2pes_parser_state_sync:
		batom_cursor_skip(payload_start, len);
		break;
	}
		
	return bmpeg2ts_parser_action_consume;
}


void 
bmpeg2pes_parser_flush(bmpeg2pes_parser *pes)
{
	batom_accum_t pes_header;

	BDBG_ASSERT(pes);
	pes_header = pes->pes_header;
	if(batom_accum_len(pes_header)) {
		batom_accum_clear(pes_header);
		pes->info.flags |= BMPEG2PES_DISCONTINUITY;
	}
	pes->state = bmpeg2pes_parser_state_sync;
	return;
}

void 
bmpeg2pes_parser_shutdown(bmpeg2pes_parser *pes)
{
	BDBG_ASSERT(pes);
	batom_accum_destroy(pes->pes_header);
	return;
}

BDBG_OBJECT_ID(bmpeg2pes_demux_t);
struct bmpeg2pes_demux {
	BDBG_OBJECT(bmpeg2pes_demux_t)
	enum {b_mpeg2pes_demux_state_data, b_mpeg2pes_demux_state_hdr, b_mpeg2pes_demux_state_sync, b_mpeg2pes_demux_state_resync} state;
	unsigned packet_len;
	unsigned packet_off;
	batom_accum_t accum;
	bmpeg2pes_parser parser;
	bmpeg2pes_demux_status status;
};

void 
bmpeg2pes_demux_default_config(bmpeg2pes_demux_config *config)
{
	config->packet_cnxt = NULL;
	config->packet = b_mpeg2pes_drop_packet;
	return;
}

bmpeg2pes_demux_t
bmpeg2pes_demux_create(batom_factory_t factory, const bmpeg2pes_demux_config *config)
{
	bmpeg2pes_demux_t demux;
	int rc;

	BDBG_ASSERT(factory);
	BDBG_ASSERT(config);

	demux = BKNI_Malloc(sizeof(*demux));
	if(!demux) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(demux, bmpeg2pes_demux_t);
	demux->accum = batom_accum_create(factory);
	if(!demux->accum) {
		goto err_accum;
	}
	rc = bmpeg2pes_parser_init(factory, &demux->parser, BMPEG2PES_ID_ANY);
	if(rc!=0) {
		goto err_parser;
	}
	demux->parser.packet_cnxt= config->packet_cnxt;
	demux->parser.packet = config->packet;
	bmpeg2pes_demux_reset(demux);
	return demux;

err_parser:
	batom_accum_destroy(demux->accum);
err_accum:
	BKNI_Free(demux);
err_alloc:
	return NULL;
}

void 
bmpeg2pes_demux_reset(bmpeg2pes_demux_t demux)
{
	BDBG_OBJECT_ASSERT(demux, bmpeg2pes_demux_t);
	demux->state = b_mpeg2pes_demux_state_hdr;
	demux->packet_len = 0;
	demux->packet_off = 0;
	demux->status.nresyncs = 0;
	demux->status.data_offset=0;
	batom_accum_clear(demux->accum);
	bmpeg2pes_parser_reset(&demux->parser);
	return;
}

void 
bmpeg2pes_demux_seek(bmpeg2pes_demux_t demux, uint64_t offset)
{
	BDBG_OBJECT_ASSERT(demux, bmpeg2pes_demux_t);
	
	bmpeg2pes_demux_reset(demux);
	demux->status.data_offset = offset;
	return;
}

void 
bmpeg2pes_demux_destroy(bmpeg2pes_demux_t demux)
{
	BDBG_OBJECT_ASSERT(demux, bmpeg2pes_demux_t);
	batom_accum_destroy(demux->accum);
	bmpeg2pes_parser_shutdown(&demux->parser);
	BDBG_OBJECT_DESTROY(demux, bmpeg2pes_demux_t);
	BKNI_Free(demux);
	return;
}

static bmpeg2pes_stream_id_type
b_mpeg2pes_demux_get_scode(batom_cursor *cursor)
{
	uint32_t scode=0xFFFFFFFFul;
	int byte;

	for(;;) {
		BATOM_NEXT(byte, cursor);
		if(byte!=BATOM_EOF) {
			scode = (scode << 8)|byte;
			if((scode&0xFFFFFF00ul)==0x100) {
				BDBG_MSG_TRACE(("b_mpeg2pes_demux_get_scode: scode:%#x", scode));
				return scode&0xFF;
			}
		} else {
			return 0;
		}
	}
}

void 
bmpeg2pes_demux_get_status(bmpeg2pes_demux_t demux, bmpeg2pes_demux_status *status)
{
	BDBG_OBJECT_ASSERT(demux, bmpeg2pes_demux_t);
	BDBG_ASSERT(status);
	*status = demux->status;
	return;
}

size_t 
bmpeg2pes_demux_feed(bmpeg2pes_demux_t pes, batom_pipe_t pipe)
{
	size_t feed_len=0;
	batom_t atom;
	batom_accum_t accum;
	uint32_t word;
	BDBG_OBJECT_ASSERT(pes, bmpeg2pes_demux_t);

	atom = batom_pipe_pop(pipe);
	accum = pes->accum;
	do {
		batom_cursor cursor;
		batom_cursor start;
		size_t len;

		BDBG_MSG(("bmpeg2pes_demux_feed:%p atom:%p:%u", (void *)pes, (void *)atom, atom?(unsigned)batom_len(atom):0));
		if(atom) {
			batom_accum_add_atom(accum, atom);
			feed_len += batom_len(atom);
			batom_release(atom);
		}
		for(;;) {
			bmpeg2pes_stream_id_type id_type;
			batom_cursor_from_accum(&cursor, accum);			
			BDBG_MSG_TRACE(("bmpeg2pes_demux_feed:%#lx state:%u accum:%u packet_len:%u packet_off:%u", (unsigned long)pes, (unsigned long)pes->state, batom_accum_len(accum), pes->packet_len, pes->packet_off));
			BATOM_CLONE(&start, &cursor);
			switch(pes->state) {
			case b_mpeg2pes_demux_state_data:
				if(pes->packet_len>0) {
					BDBG_ASSERT(pes->packet_off < pes->packet_len);
					len = batom_cursor_skip(&cursor, pes->packet_len-pes->packet_off); 
					BDBG_ASSERT(pes->packet_off<=pes->packet_len);
				} else {
					if(pes->packet_off==0) {
						batom_cursor_skip(&cursor, sizeof(uint32_t)+sizeof(uint16_t));
					}
					/* search for PES start code or end of data */
					for(;;) {
						word = b_mpeg2pes_demux_get_scode(&cursor);
						len = batom_cursor_pos(&cursor);
						if(word < 0xB9) { /* GM : system start codes are above 0xB9 */
							if(BATOM_IS_EOF(&cursor))  {
                                if(len>4) {
                                    break;
                                }
								goto next;
							}
						} else {
							BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
							pes->state = b_mpeg2pes_demux_state_hdr;
							break;
						} 
					}
					BDBG_ASSERT(len>=4);
					len -= 4;
				}				
				if(len>0) {
					bmpeg2ts_parser_action  action;
					action = bmpeg2pes_parser_feed(&pes->parser, pes->packet_off==0?BMPEG2TS_PAYLOAD_UNIT_START:0, accum, &start, len);
					pes->packet_off+=len;
					if(pes->packet_off==pes->packet_len) {
						pes->state = b_mpeg2pes_demux_state_hdr;
					}
					pes->status.data_offset += len;
					batom_accum_trim(accum, &start);
					if(action==bmpeg2ts_parser_action_hold) {
						goto exit;
					}
				} else {
					goto next;
				}
				break;
			case b_mpeg2pes_demux_state_hdr:
				word = batom_cursor_uint32_be(&cursor);
				BDBG_MSG_TRACE(("bmpeg2pes_demux_feed:%#lx pes_scode:%#x", (unsigned long)pes, word));
				pes->packet_off = 0;				
				pes->packet_len = batom_cursor_uint16_be(&cursor);
				
				if(BATOM_IS_EOF(&cursor)) {
					goto next;
				}
				if( (word&0xFFFFFF00) == 0x100) {
					word &= 0xFF;
					if(bmpeg2pes_decode_stream_id(word)!=bmpeg2pes_stream_id_type_invalid) {
						pes->state = b_mpeg2pes_demux_state_data;
						if(pes->packet_len!=0) {
							pes->packet_len+=sizeof(uint32_t) /* scode */ +sizeof(uint16_t) /* packet_len */;
						}
						BDBG_MSG_TRACE(("bmpeg2pes_demux_feed:%#lx packet_len:%u", (unsigned long)pes, pes->packet_len));
					} else /* if(word >= 0xB0) */ {
						pes->state = b_mpeg2pes_demux_state_sync;
					}
				} else if((word>>24)==0) {
					pes->state = b_mpeg2pes_demux_state_sync;
				} else {
					pes->state = b_mpeg2pes_demux_state_resync;
				}
				break;
			case b_mpeg2pes_demux_state_resync:
				pes->status.nresyncs ++;
				BDBG_MSG(("bmpeg2pes_demux_feed:%#lx resync:%u", (unsigned long)pes, pes->status.nresyncs));
			case b_mpeg2pes_demux_state_sync:
				for(;;) {
					id_type = bmpeg2pes_decode_stream_id(b_mpeg2pes_demux_get_scode(&cursor));
					len = batom_cursor_pos(&cursor);
					if(id_type!=bmpeg2pes_stream_id_type_invalid) {
						BDBG_MSG_TRACE(("bmpeg2pes_demux_feed:%#lx sync_off:%u", (unsigned long)pes, len));
						pes->state = b_mpeg2pes_demux_state_hdr;
						break;
					}
					if(BATOM_IS_EOF(&cursor))  {
						len++;
						if(len>4) { /* we need to read at least 4 bytes */
							break;
						} else {
							goto next;
						}
					}
				}
				BDBG_ASSERT(len>=4);
				len -= 4;
				batom_cursor_skip(&start, len);
				pes->status.data_offset += len;
				batom_accum_trim(accum, &start);
				break;
			}
		}
next:
		;
	} while(NULL!=(atom=batom_pipe_pop(pipe)));
exit:
	return feed_len;
}

