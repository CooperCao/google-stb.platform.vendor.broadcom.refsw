/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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

#include "bstd.h"
#include "basf_player.h"
#include "bkni.h"
#include "bfifo.h"
#include "bfile_stdio.h"
#include "biobits.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "bsettop.h"
#include "basf_rtp_proc.h"                      /* ASFIP link to rtp/rtsp lib */
#include "basf_stream.h"

BDBG_MODULE(basf_rtp_feed);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)
#ifdef PROFILE
#undef BDBG_MSG
#define BDBG_MSG(x)
#endif

#if 1
#define BUF_SIZE	    (4096*16)
#else
#define BUF_SIZE	    (197*7)
#endif

#define DECODER_TIMEOUT     (0) 
#define LM_CLOSING_TIMEOUT  (1)

static void 
test_buf_recycle(void *application_cnxt, void *cookie)
{
    BSTD_UNUSED(application_cnxt);
	BDBG_WRN(("FREE %#lx", (unsigned long)cookie));
#if 0
	BKNI_Memset(cookie, 0xDE, BUF_SIZE);
#endif
	BKNI_Free(cookie);
	return;
}

struct {
	struct {
		unsigned stream, id, pid;
	} video,audio;
} cfg = {
	{0x2, 0xFD, 0x21},
	{0, 0xCD, 0x22}
};

struct stream_cnxt {
	struct basf_parser_object_handler stream_properties_handler; /* shall be first */
	batom_pipe_t pipe;
    basf_stream_t stream;
	FILE *pes_out;
	FILE *ts_out;
	unsigned pid_cont_counter;
	unsigned ts_pid;
	unsigned frame_cnt;
	bool vc1_simple_main;
	bool vc1_interlaced;
	bool vc1_finterpflag;
	bool vc1_rangered;
	bool vc1_maxbframes;
};

#define B_TS_SIZE_MAX       (188-4)
#define B_TS_SIZE_MAX_AF    (B_TS_SIZE_MAX-2)

static void 
ts2pes_packetizer(struct stream_cnxt *cnxt, const bio_vec *packet, unsigned count)
{
	batom_cursor cursor;
	bio_array array;
	batom_checkpoint chk;
	FILE *fout=cnxt->ts_out;
	size_t len;
	size_t hlen;
	uint8_t header[188];
	uint8_t data[188];
	unsigned pkt_cnt=0;

	batom_cursor_from_iov(&cursor, &array, packet, count);

	for(;;) {
		BATOM_SAVE(&cursor, &chk);
		len=batom_cursor_copy(&cursor, data, B_TS_SIZE_MAX);
		if (len==0) {
			break;
		}
		header[0] = 0x47;
		header[1] = 
			B_SET_BIT( transport_error_indicator, 0, 7) |
			B_SET_BIT( payload_unit_start_indicator, pkt_cnt==0, 6) |
			B_SET_BIT( transport_priority, 0, 5) |
			B_SET_BITS( "pid 12..8", B_GET_BITS(cnxt->ts_pid, 12, 8), 4, 0);
		header[2] = B_GET_BITS(cnxt->ts_pid, 7, 0);
		header[3] = 
			B_SET_BITS(transport_scrambling_control, 0, 7, 6) |
			B_SET_BITS(continuity_counter, B_GET_BITS(cnxt->pid_cont_counter, 3, 0), 3, 0);
			
		if (len==B_TS_SIZE_MAX) {
            hlen = 4;
			header[3] |= B_SET_BITS(adaptation_field_control, 0x1, 5, 4); /* payload only */
		} else {
			unsigned i;
			header[3] |= B_SET_BITS(adaptation_field_control, 0x3, 5, 4); /* adaptation field and payload */
			if (len>B_TS_SIZE_MAX_AF) {
				len = B_TS_SIZE_MAX_AF;
			}
			hlen = (B_TS_SIZE_MAX_AF-len)+1;
			header[4] = hlen;
			header[5] = 
				B_SET_BIT(discontinuity_indicator, 0, 7) |
				B_SET_BIT(random_access_indicator, 0, 6) |
				B_SET_BIT(elementary_stream_priority_indicator, 0, 5) |
				B_SET_BIT(PCR_flag, 0, 4) |
				B_SET_BIT(OPCR_flag, 0, 3) |
				B_SET_BIT(splicing_point_flag, 0, 2) |
				B_SET_BIT(transport_private_data_flag, 0, 1) |
				B_SET_BIT(adaptation_field_extension_flag, 0, 0);
			hlen += 5;
			for(i=6;i<hlen;i++) {
				header[i] = 0xFF;	
			}
			batom_cursor_rollback(&cursor, &chk);
			batom_cursor_copy(&cursor, data, len);
		}
		fwrite(header, 1, hlen, fout);
		BDBG_MSG_TRACE(("ts:%u %u,%u(%u) (%02X %02X):(%02X %02X %02X %02X)", pkt_cnt, hlen, len, hlen+len, header[1], header[3],data[0],data[1],data[2],data[3]));
		fwrite(data, 1, len, fout);
		pkt_cnt++;
		cnxt->pid_cont_counter++;
	}
	return;
}

static void
test_pes_writer(void *stream_cnxt, const bio_vec *packet, unsigned count)
{
	struct stream_cnxt *cnxt = (struct stream_cnxt*)stream_cnxt;
	FILE *fout = cnxt->pes_out;
	unsigned i;
#if 1
	if (cnxt->ts_out) {
		ts2pes_packetizer(cnxt, packet, count);
	}
	fprintf(stderr, "+");fflush(stderr);
	for(i=0;i<count;i++) {
		BDBG_MSG_TRACE(("write:%u:%u %lx:%u", i, count, (unsigned long)packet[i].base, packet[i].len));
		fwrite(packet[i].base, 1, packet[i].len, fout);
	}
#endif
	basf_stream_recycle(cnxt->stream, packet);
	return;
}

struct index_tool {
	FILE *fin;
	FILE *fout;
	FILE *ts_fout;
	basf_parser_t  asf;
	bool index_found;
	basf_simple_index index;
	basf_demux_t demux;
	bool reverse;
	unsigned seek;
	bmedia_time_scale scale;
	bmedia_player_step direction;
	struct stream_cnxt *video;
};

#define BASF_GET_DWORD(p,off) \
			(basf_dword)(((uint8_t *)(p))[(off)+0]) | \
			((basf_dword)(((uint8_t *)(p))[(off)+1])<<8) | \
			((basf_dword)(((uint8_t *)(p))[(off)+2])<<16) | \
			((basf_dword)(((uint8_t *)(p))[(off)+3])<<24) 

#define BASF_GET_WORD(p,off) \
			(basf_word)(((uint8_t *)(p))[(off)+0]) | \
			((basf_word)(((uint8_t *)(p))[(off)+1])<<8) 

static void 
test_simple_index(void *application_cnxt, const basf_simple_index *index)
{
	unsigned i;
	uint8_t buff[64];
	basf_dword pkt_no;
	basf_word pkt_cnt;
	struct index_tool *tool=(struct index_tool *)application_cnxt;
	FILE *fin=tool->fin;

	BDBG_WRN(("b_test_simple_index"));


	fseek(fin, index->offset, SEEK_SET);
	for(i=0;i<index->index_entries_count;i++) {
		fread(buff, 1, 4+2, fin);
		pkt_no = BASF_GET_DWORD(buff,0);
		pkt_cnt = BASF_GET_WORD(buff,4);
		BDBG_MSG(("simple_index %u: %u %u", i, pkt_no, pkt_cnt));
	}
	tool->index = *index;
	tool->index_found = true;
	return;
}

#define B_FIELD_LEN(name,type) sizeof(basf_##type)
#define B_ASF_VIDEO_CODEC(c1,c2,c3,c4) (((uint32_t)c1) | ((uint32_t)c2<<8) | ((uint32_t)c3<<16) | ((uint32_t)c4<<24))
#define B_ASF_VIDEO_WVC3 B_ASF_VIDEO_CODEC('W','M','V','3')

static basf_parser_handler_action
stream_properties_object_extract_type(struct basf_parser_object_handler *handler, batom_t atom)
{
	struct stream_cnxt *stream= (struct stream_cnxt *)(void *)handler;
	basf_guid stream_type;
	basf_dword flags;
    batom_cursor cursor;
    
	unsigned stream_number;
	basf_dword bi_compression;

	basf_read_guid(&cursor, &stream_type);
    batom_cursor_from_atom(&cursor, atom);
    
	if (basf_compare_guid(&stream_type, &basf_video_media_guid)!=0) {
		return basf_parser_handler_action_none;
	}
	batom_cursor_skip(&cursor, 
			  B_FIELD_LEN( error_correction_type , guid)
			+ B_FIELD_LEN( time_offset, qword)
			+ B_FIELD_LEN( type_specific_length, dword)
			+ B_FIELD_LEN( error_correction_length, dword) 
			);
	flags = basf_read_word(&cursor);
	stream_number = B_GET_BITS(flags,6,0);
	batom_cursor_skip(&cursor, B_FIELD_LEN( Reserved, dword));

	/* video stream */
	batom_cursor_skip(&cursor,
		  B_FIELD_LEN( encoded_image_height, dword)
		+ B_FIELD_LEN( encoded_image_height, dword)
		+ B_FIELD_LEN( ReservedFlags, byte)
		+ B_FIELD_LEN( format_data_size, word)
		+ B_FIELD_LEN( bi_size, dword) 
		+ B_FIELD_LEN( bi_width, dword) 
		+ B_FIELD_LEN( bi_height, dword)
		+ B_FIELD_LEN( bi_planes, word) 
		+ B_FIELD_LEN( bi_bit_count, word));
	bi_compression = basf_read_dword(&cursor);

	batom_cursor_skip(&cursor,
		  B_FIELD_LEN( bi_size_image, dword)
		+ B_FIELD_LEN( bi_xpels_per_meter, dword)
		+ B_FIELD_LEN( bi_ypels_per_meter, dword)
		+ B_FIELD_LEN( bi_clr_used,  dword)
		+ B_FIELD_LEN( bi_clr_important, dword)
		);

	BDBG_MSG(("video_stream: %u %c%c%c%c", stream_number, bi_compression&0xFF, (bi_compression>>8)&0xFF, (bi_compression>>16)&0xFF, (bi_compression>>24)&0xFF));
	switch(bi_compression) {
	case B_ASF_VIDEO_WVC3: /* VC-1 Simple/Main Profile */
		{
			unsigned profile;
			unsigned frmrtq_postproc;
			unsigned bitrtq_postproc;
			unsigned loopfilter;
			unsigned multires;
			unsigned fastuvmc;
			unsigned extended_mv;
			unsigned dquant;
			unsigned vstransform;
			unsigned overlap;
			unsigned syncmarker;
			unsigned rangered;
			unsigned maxbframes;
			unsigned quantizer;
			unsigned finterpflag;
			batom_bitstream bs;

			batom_bitstream_init(&bs, &cursor);

			profile = batom_bitstream_bits(&bs, 4);
			BDBG_MSG(("header: %#x", bs.cache));

			frmrtq_postproc = batom_bitstream_bits(&bs, 3);
			bitrtq_postproc = batom_bitstream_bits(&bs, 5);
			loopfilter = batom_bitstream_bit(&bs);
			batom_bitstream_drop(&bs); /* Reserved3 */
			multires = batom_bitstream_bit(&bs);
			batom_bitstream_drop(&bs); /* Reserved4 */
			fastuvmc = batom_bitstream_bit(&bs);
			extended_mv = batom_bitstream_bit(&bs);
			dquant = batom_bitstream_bits(&bs, 2);
			vstransform = batom_bitstream_bit(&bs);
			batom_bitstream_drop(&bs); /* Reserved5 */
			overlap = batom_bitstream_bit(&bs);
			syncmarker = batom_bitstream_bit(&bs);
			rangered = batom_bitstream_bit(&bs);
			maxbframes = batom_bitstream_bits(&bs, 3);
			quantizer = batom_bitstream_bits(&bs, 2);
			finterpflag = batom_bitstream_bit(&bs);
			batom_bitstream_drop(&bs); /* Reserved6 */

			/* bio_bitstream_dump(&bs); */
			BDBG_MSG(("profile: %u frmrtq_postproc: %u bitrtq_postproc: %u loopfilter: %u", profile, frmrtq_postproc, bitrtq_postproc, loopfilter));
			BDBG_MSG(("rangered: %d maxbframes: %d quantizer: %d finterpflag: %d", rangered, maxbframes, quantizer, finterpflag));
			stream->vc1_finterpflag = finterpflag;
			stream->vc1_rangered = rangered;
			stream->vc1_maxbframes = maxbframes;
			stream->vc1_simple_main = true;
		}
		break;
		default:
			stream->vc1_simple_main = false;
			break;

	}

	return basf_parser_handler_action_none;
}


static bool 
test_parser_meta(void *stream_cntx, const void *meta, size_t meta_len)
{
	struct stream_cnxt *stream = (struct stream_cnxt *)stream_cntx;

	if (meta_len<6+4) { return false;}
	stream->vc1_interlaced = ((const uint8_t *)meta)[5+4] & 0x40;
	BDBG_MSG(("video %s", stream->vc1_interlaced?"Interlaced":"Progressive"));
	return true;
}

enum frameType {frameProgressive='P',frameInterlaced='F',fieldInterlaced='I'};

static bool 
test_parser_filter_frame_type(void *stream_cntx, const struct basf_payload_info *info, batom_cursor *first_payload)
{
	basf_dword sc;
	batom_checkpoint chk;
	basf_byte ptype = 0;
	enum frameType type=frameProgressive;
	const char *pic = "";
	struct stream_cnxt *stream = (struct stream_cnxt *)stream_cntx;


	BATOM_SAVE(first_payload, &chk);
	if (stream->vc1_simple_main) {
		batom_bitstream bs;

		batom_bitstream_init(&bs, first_payload);
		type = frameProgressive;
		ptype = 0;
		if (stream->vc1_finterpflag) {
			batom_bitstream_drop(&bs); /* INTERPFRM */
		}
		batom_bitstream_drop_bits(&bs, 2); /* FRMCNT */
		if (stream->vc1_rangered) {
			batom_bitstream_drop(&bs); /* RANGEREDFRM */
		}
		if (batom_bitstream_bit(&bs)) {
			pic = "P";
		} else if (stream->vc1_maxbframes==0) {
			pic = "I";
		} else if (batom_bitstream_bit(&bs)) {
			pic = "I";
		} else {
			pic = "B";
		}
		goto done;
	}
	sc = basf_read_dword(first_payload);
	if ((sc&0x00FFFFFF) ==  0x00010000) { /* some start code */
		fprintf(stderr, "s");
		if (sc!=0xd010000u) {
			int temp;
			/* not a picture scartcode, try to load next one*/
			while(sc!=0x0000010d) {
				BATOM_NEXT(temp, first_payload);
				if (temp==BATOM_EOF) {
					goto skip;
				}
				sc = (sc << 8)|(basf_dword)temp;
				if ( (sc&0xFFFFFF00u) == 0x100) {
					fprintf(stderr, "s");
				}
			}
		}
		ptype = basf_read_byte(first_payload);
	} else {
		/* not a scartcode, get first byte */
		ptype = sc&0xFF;
	}
	if(BATOM_IS_EOF(first_payload)) { goto done; }
	BDBG_MSG(("sc %#x ptype %#x", sc, ptype));
	if (stream->vc1_interlaced) {
		if( (ptype&0x80) == 0) { type = frameProgressive;ptype<<=1;}
		else if( (ptype&0xC0) == 0x80) { type = frameInterlaced;ptype<<=2;}
		else {type = fieldInterlaced;ptype<<=2;}
	} else {
		type = frameProgressive;
	}
	switch(type) {
	default:
	case frameInterlaced:
	case frameProgressive:
		if ((ptype&0x80) == 0) { pic = "P"; }
		else if ((ptype&0xC0) == 0x80) { pic = "B"; }
		else if ((ptype&0xE0) == 0xC0) { pic = "I"; }
		else if ((ptype&0xF0) == 0xC0) { pic = "BI"; }
		else {pic = "Skipped";}
		break;
	case fieldInterlaced:
		switch(ptype&0xE0) {
		default:
		case (0<<5): pic = "I:I"; break;
		case (1<<5): pic = "I:P"; break;
		case (2<<5): pic = "P:I"; break;
		case (3<<5): pic = "P:P"; break;
		case (4<<5): pic = "B:B"; break;
		case (5<<5): pic = "B:BI"; break;
		case (6<<5): pic = "BI:B"; break;
		case (7<<5): pic = "BI:BI"; break;
		}
		break;
	}
done:
	fprintf(stderr, "%s%s", pic, info->key_frame?"(K)":"");
	BDBG_MSG(("%ld type %c ptype=%s(%02x)", info->pres_time, type, pic, ptype));
skip:
	batom_cursor_rollback(first_payload, &chk);
	return true;
}

static bool 
test_key_filter(void *stream_cntx, const struct basf_payload_info *info, batom_cursor *first_payload)
{
	struct stream_cnxt *stream = (struct stream_cnxt *)stream_cntx;

	if (stream->frame_cnt>0 && info->key_frame && test_parser_filter_frame_type(stream_cntx, info, first_payload)) {
		stream->frame_cnt--;
		return true;
	}
	return false;
}

static const basf_guid 
	codec_list_guid = 
	{{0x86, 0xD1, 0x52, 0x40, 0x31, 0x1D, 0x11, 0xD0, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}},
	language_list_guid = 
	{{0x7C, 0x43, 0x46, 0xA9, 0xEF, 0xE0, 0x4B, 0xFC, 0xB2, 0x29, 0x39, 0x3E, 0xDE, 0x41, 0x5C, 0x85}},
	metadata_guid =
	{{0xC5, 0xF8, 0xCB, 0xEA, 0x5B, 0xAF, 0x48, 0x77, 0x84, 0x67, 0xAA, 0x8C, 0x44, 0xFA, 0x4C, 0xCA}},
    stream_properties_guid = 
	{{0xB7, 0xDC, 0x07, 0x91, 0xA9, 0xB7, 0x11, 0xCF, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
  	file_properties_guid = 
	{{0x8C, 0xAB, 0xDC, 0xA1, 0xA9, 0x47, 0x11, 0xCF, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}},
	extended_content_description_guid = 
	{{0xD2, 0xD0, 0xA4, 0x40, 0xE3, 0x07, 0x11, 0xD2, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}};

static unsigned
b_asf_read_wstr(batom_cursor *cursor, char *str, size_t len, size_t asf_len )
{
	unsigned i,j;

	len--;

	for(i=0;i<asf_len && i<len;i++) {
		str[i] = basf_read_byte(cursor);
	}
	batom_cursor_skip(cursor, asf_len-i);
	for(j=2;j<i;j+=2) {
		str[j/2]=str[j];
	}
	str[j/2]='\0';
	return j/2;
}

static unsigned
b_asf_read_wstr_word(batom_cursor *cursor, char *str, size_t len)
{
	return b_asf_read_wstr(cursor, str, len, 2*basf_read_word(cursor));
}

static unsigned
b_asf_read_wstr_byte(batom_cursor *cursor, char *str, size_t len)
{
	return b_asf_read_wstr(cursor, str, len, basf_read_byte(cursor));
}

static void
b_asf_skip_data(batom_cursor *cursor)
{
	basf_word asf_len;
	asf_len = basf_read_word(cursor);
	batom_cursor_skip(cursor, asf_len);
	return;
}

static basf_parser_handler_action
extended_content_description_object(struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;
	basf_word content_descriptor_count;
	size_t start;
	unsigned i;

	BSTD_UNUSED(handler);
    batom_cursor_from_atom(&cursor, atom);
    
	BDBG_MSG(("extended_content_description_object"));
	start = batom_cursor_pos(&cursor);
	content_descriptor_count = basf_read_word(&cursor);
	for(i=0;i<content_descriptor_count;i++) {
		char descriptor_name[128];
		basf_word value_type;
		basf_word value_len;
		
		b_asf_read_wstr(&cursor, descriptor_name, sizeof(descriptor_name), basf_read_word(&cursor));
		value_type = basf_read_word(&cursor);
		value_len = basf_read_word(&cursor);
		batom_cursor_skip(&cursor, value_len);
		BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
		fprintf(stderr, "description: %u %s\n", i, descriptor_name);
	}
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	BDBG_ASSERT((batom_cursor_pos(&cursor)-start)<=batom_len(atom));
	return  basf_parser_handler_action_none;
}

static basf_parser_handler_action
file_properties_object(struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;
	basf_dword flags;
#define B_ASF_Broadcast_Flag B_GET_BIT(flags,0)
#define B_ASF_Seekable_Flag B_GET_BIT(flags,1)

	BSTD_UNUSED(handler);
	BSTD_UNUSED(batom_len(atom));
    batom_cursor_from_atom(&cursor, atom);
    
	batom_cursor_skip(&cursor, 
			sizeof(basf_guid) +
			sizeof(basf_qword) +
			sizeof(basf_qword) +
			sizeof(basf_qword) +
			sizeof(basf_qword) +
			sizeof(basf_qword) +
			sizeof(basf_qword)
			);
	flags = basf_read_dword(&cursor);
	if (B_ASF_Broadcast_Flag) { fprintf(stderr, " Broadcast\n"); }
	if (B_ASF_Seekable_Flag) { fprintf(stderr, " Seekable\n"); }
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	return  basf_parser_handler_action_none;
}

static basf_parser_handler_action
stream_properties_object(struct basf_parser_object_handler *handler, batom_t atom)
{
    batom_cursor cursor;
    batom_t atom_copy = NULL;   /* !!! */
    
	size_t start;
	basf_guid stream_type;
	basf_guid error_correction_type;
	basf_qword time_offset;
	basf_dword type_specific_length;
	basf_dword error_correction_length;
	basf_word flags;
	batom_cursor copy;

	BDBG_MSG(("stream_properties_object"));
    batom_cursor_from_atom(&cursor, atom);

    batom_cursor_clone(&copy, &cursor);
    stream_properties_object_extract_type(handler, atom_copy);

	start = batom_cursor_pos(&cursor);
	basf_read_guid(&cursor, &stream_type);
	basf_read_guid(&cursor, &error_correction_type);
	time_offset = basf_read_qword(&cursor);
	type_specific_length = basf_read_dword(&cursor);
	error_correction_length = basf_read_dword(&cursor);
	flags = basf_read_word(&cursor);
	BDBG_MSG(("properties stream: %u",B_GET_BITS(flags,6,0)));

	batom_cursor_skip(&cursor, sizeof(basf_dword));
	batom_cursor_skip(&cursor, type_specific_length);
	batom_cursor_skip(&cursor, error_correction_length);
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	BDBG_ASSERT((batom_cursor_pos(&cursor)-start)<=batom_len(atom));
	return basf_parser_handler_action_none;
}

static basf_parser_handler_action
metadata_object(struct basf_parser_object_handler *handler, batom_t atom)
{
	batom_cursor cursor;
    basf_word description_records_count;
	size_t start;
	unsigned i;

	BSTD_UNUSED(handler);
    batom_cursor_from_atom(&cursor, atom);
    
	BDBG_MSG(("metadata_object"));
	start = batom_cursor_pos(&cursor);
	description_records_count=basf_read_word(&cursor);
	for(i=0;i<description_records_count;i++) {
		basf_word stream_number;
		basf_word name_length;
		basf_word data_type;
		basf_dword data_length;
		char name[128];

		batom_cursor_skip(&cursor, sizeof(basf_word));
		stream_number = basf_read_word(&cursor);
		name_length = basf_read_word(&cursor);
		data_type = basf_read_word(&cursor);
		data_length = basf_read_dword(&cursor);
		b_asf_read_wstr(&cursor, name, sizeof(name), name_length);
		batom_cursor_skip(&cursor, data_length);
		BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
		fprintf(stderr, "metadata: %u stream:%u %s\n", i, stream_number, name);
	}
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	BDBG_ASSERT((batom_cursor_pos(&cursor)-start)<=batom_len(atom));
	return basf_parser_handler_action_none;
}

static basf_parser_handler_action
language_list_object(struct basf_parser_object_handler *handler, batom_t atom)
{
	basf_word language_ID_records_count;
    batom_cursor cursor;

	unsigned i;
	char lang[128];
	size_t start;

	BSTD_UNUSED(handler);
    batom_cursor_from_atom(&cursor, atom);
	
    BDBG_MSG(("language_list_object"));
	start = batom_cursor_pos(&cursor);
	language_ID_records_count=basf_read_word(&cursor);
	for(i=0;i<language_ID_records_count;i++) {
		b_asf_read_wstr_byte(&cursor, lang, sizeof(lang));
		BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
		fprintf(stderr, "Language: %u %s\n", i, lang);
	}
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	BDBG_ASSERT((batom_cursor_pos(&cursor)-start)<=batom_len(atom));
	return basf_parser_handler_action_none;
}

static basf_parser_handler_action
codec_list_object(struct basf_parser_object_handler *handler, batom_t atom)
{
	batom_cursor cursor;
    basf_dword codec_entries_count;
	basf_dword type;

	unsigned i;
	const char *stype;
	char name[128];
	char description[128];
	size_t start;
	
	BSTD_UNUSED(handler);
	BDBG_MSG(("Codec list object"));
    batom_cursor_from_atom(&cursor, atom);
    
	start = batom_cursor_pos(&cursor);
	batom_cursor_skip(&cursor, sizeof(basf_guid));
	codec_entries_count = basf_read_dword(&cursor);
	for(i=0;i<codec_entries_count;i++) {
		type = basf_read_word(&cursor);
		b_asf_read_wstr_word(&cursor, name, sizeof(name));
		b_asf_read_wstr_word(&cursor, description, sizeof(description));
		b_asf_skip_data(&cursor);
		switch(type) {
		case 1: stype="Video";break;
		case 2: stype="Audio";break;
		default: stype="Unknown";break;
		}
		BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
		fprintf(stderr, "%u: %s codec: %s %s\n", i, stype, name, description);
	}
	BDBG_ASSERT(!BATOM_IS_EOF(&cursor));
	BDBG_ASSERT((batom_cursor_pos(&cursor)-start)<=batom_len(atom));
	return basf_parser_handler_action_none;
}

/* recycled main function from the asf test package */
int 
basf_rtp_feed_entrypoint(basf_rtp_mux_func fn_mux, basf_rtp_feed_ctx_t fn_mux_data, 
                         basf_rtp_feed_descriptor* descr, basf_rtp_ip_type rtp_ip_type)
{
    FILE *fout, *ts_fout, *fin;
    
    basf_parser_t asf;
    batom_factory_t asf_factory = NULL;     /* !!! */   /* ALL THESE NEED TO BE INITIALIZED */
    balloc_iface_t asf_alloc_iface = NULL;  /* !!! */
    batom_pipe_t asf_pipe = NULL;           /* !!! */
    basf_demux_t demux = NULL;              /* !!! */
    
	basf_parser_cfg asf_cfg;
	basf_demux_cfg demux_cfg;
	basf_stream_cfg stream_cfg;

    basf_parser_object_handler codec_list_handler; 
	basf_parser_object_handler language_list_handler; 
	basf_parser_object_handler metadata_handler; 
	basf_parser_object_handler file_properties_handler;
	basf_parser_object_handler extended_content_description_handler;
	struct stream_cnxt audio, video;
    BERR_Code err;

    bool quiet = false, nowait = false;        
    basf_rtsp_session_t rtsp = NULL;           
    basf_rtsp_demux_t rtsp_demux = NULL;       
    basf_rtp_config_t rcfg = NULL;             
    basf_rtp_context_t rctx = NULL;            

    cfg.video.id = descr->video_id;
    cfg.audio.id = descr->audio_id;
    cfg.video.stream = descr->video;
    cfg.audio.stream = descr->audio;
    if(descr->msg) err = BDBG_SetModuleLevel(descr->msg, BDBG_eMsg); 

	if (!quiet) {
		fprintf(stderr, "ASFIP player: input %s video: %u audio: %u output: %s %s TS %s\n", descr->file, cfg.video.stream, 
                cfg.audio.stream, cfg.video.id==0?"ES":(cfg.video.id==1?"RAW":"PES"), descr->pes, descr->ts);
	}

    err = BDBG_SetModuleLevel("basf_parser", BDBG_eErr); 
	err = BDBG_SetModuleLevel("basf_stream", BDBG_eErr); 
	err = BDBG_SetModuleLevel("basf_player", BDBG_eErr); 
	err = BDBG_SetModuleLevel("basf_rtp_proc", BDBG_eErr);

    fin = fopen(descr->file,"wb");
	if (!fin) {
		perror(descr->file);return 1;
	}
    fout = fopen(descr->pes,"wb");
	if (!fout) {
		perror(descr->pes);return 1;
	} 
	ts_fout = fopen(descr->ts,"wb");
	if (!fout) {
		perror(descr->ts);return 1;
	} 

	basf_parser_default_cfg(&asf_cfg);
	asf_factory = batom_factory_create(bkni_alloc, 16);
	if(!asf_factory) {
		return -1;
	}
    
    asf_pipe = batom_pipe_create(asf_factory);
	BDBG_ASSERT(asf_pipe);
    
    asf = basf_parser_create(asf_factory, &asf_cfg);
    basf_demux_default_cfg(&demux_cfg);
    demux = basf_demux_create(asf, asf_factory, bkni_alloc, &demux_cfg);
    BDBG_ASSERT(demux);
    
    sleep(nowait ? 0 : DECODER_TIMEOUT);    /* cause an RTSP/RTP pause, to allow decoder to launch */
    basf_rtsp_default_config(&rcfg);        /* configure default */
    basf_rtsp_out_set_asf_demux(rcfg, asf); /* set output to go to a live demux instance */    

#if 0
    basf_rtsp_out_set_fout(rcfg, NULL);   /* set output to stdout for use with decode | */
#endif

    basf_rtsp_config_ip_type(rcfg, rtp_ip_type);                /* set transport type - tcp/udp */
    basf_rtsp_output_set_cust_demux(rcfg, fn_mux, fn_mux_data); /* set custom demux handler */ 
    basf_rtsp_config_link(rcfg, descr->link);                   /* set the rtsp link */
    rctx = basf_rtsp_client_open(&rtsp, &rtsp_demux, rcfg);     /* open rtp asf ip instance */
    if ((rtsp==NULL) || (rtsp_demux==NULL) || (rctx==NULL)) {   /* out, if we don't have a stream */
        goto exit; 
    } 

    basf_parser_install_object_handler(asf, &video.stream_properties_handler, &stream_properties_guid, stream_properties_object);
	video.stream  = audio.stream = NULL;
	if (cfg.video.stream) {
		
        basf_stream_initialize(&stream_cfg);
		video.pes_out = fout;
		video.ts_out = ts_fout;
		video.pid_cont_counter = 0;
		video.ts_pid = cfg.video.pid;
		video.pipe = batom_pipe_create(asf_factory);

		BDBG_ASSERT(video.pipe);   
		video.stream  = basf_stream_attach(demux, video.pipe, cfg.video.stream, &stream_cfg);
	} 
	if (cfg.audio.stream) {
		basf_stream_initialize(&stream_cfg);
		audio.pes_out = fout;
		audio.ts_out = ts_fout;
		audio.pid_cont_counter = 0;
		audio.ts_pid = cfg.audio.pid;
   		audio.pipe = batom_pipe_create(asf_factory);
        
		BDBG_ASSERT(audio.pipe);
        audio.stream  = basf_stream_attach(demux, audio.pipe, cfg.audio.stream, &stream_cfg);
	} 

	if(!quiet) {
		basf_parser_install_object_handler(asf, &codec_list_handler, &codec_list_guid, codec_list_object);
		basf_parser_install_object_handler(asf, &language_list_handler, &language_list_guid, language_list_object);
		basf_parser_install_object_handler(asf, &metadata_handler, &metadata_guid, metadata_object);
		basf_parser_install_object_handler(asf, &file_properties_handler, &file_properties_guid, file_properties_object);
		basf_parser_install_object_handler(asf, &extended_content_description_handler, &extended_content_description_guid, extended_content_description_object);
	}

    for(;;) {
        if(!basf_rtsp_run_demux(rtsp_demux)) {          /* run the main loop here */
            sleep(LM_CLOSING_TIMEOUT);                  /* give it some time to consume decode buffer */
            basf_rtsp_client_close(rctx);               /* close rtsp session */
            basf_rtsp_close_session(rtsp, rtsp_demux);            
            break;
        }
    }
    
    if(video.stream) {
		basf_stream_detach(demux, video.stream);
		video.stream = NULL;
	}
	if (audio.stream) {
		basf_stream_detach(demux, audio.stream);
		audio.stream = NULL;
	}

    if (!quiet) {
		basf_parser_remove_object_handler(asf, &codec_list_handler);
		basf_parser_remove_object_handler(asf, &language_list_handler);
		basf_parser_remove_object_handler(asf, &metadata_handler);
		basf_parser_remove_object_handler(asf, &file_properties_handler);
		basf_parser_remove_object_handler(asf, &extended_content_description_handler);
	}
	basf_parser_remove_object_handler(asf, &video.stream_properties_handler);

exit:
    basf_demux_destroy(demux);
    basf_parser_destroy(asf);    

    fclose(fin);
    fclose(fout);

    return 0;
}

