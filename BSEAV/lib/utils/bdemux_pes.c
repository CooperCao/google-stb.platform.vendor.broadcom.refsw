/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * MPEG-2 Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bdemux_pes.h"
#include "biobits.h"
#include "bkni.h"

BDBG_MODULE(bdemux_pes);

BDBG_OBJECT_ID(bdemux_pes);

typedef enum {bdemux_pes_result_match, bdemux_pes_result_nomatch, bdemux_pes_result_more} bdemux_pes_result;
void 
bdemux_pes_init(bdemux_pes *pes, uint8_t id)
{
	BDBG_OBJECT_INIT(pes, bdemux_pes);
	pes->state = bdemux_pes_state_sync;
	pes->pkt_len = 0;
	pes->pkt_offset = 0;
	pes->id = id;
	pes->ts_flags = 0;
	pes->hold_enable = false;
	pes->pes_packets = 0;
	return;
}

static bdemux_pes_result
b_demux_parse_pes_hdr(bdemux_pes *pes, const uint8_t *pes_header, size_t len)
{
	uint32_t pts=0;
	unsigned flags = 0;

	if( (pes_header[0]|pes_header[1])!=0x00 || pes_header[2]!=0x01 ) { /*  packet_start_code_prefix */
		BDBG_WRN(("bdemux_ts_data: %#lx invalid packet_start_code_prefix 0x%02x%02x%02x != 0x000001", (unsigned long)pes, pes_header[0], pes_header[1], pes_header[2]));
		return bdemux_pes_result_nomatch;
	}
	if(pes->id && pes->id!=pes_header[3]) {
		BDBG_WRN(("bdemux_ts_data: %#lx unknown pes id %#x(%#x)", (unsigned long)pes, pes_header[3], pes->id));
		return bdemux_pes_result_nomatch;
	}
	pes->pkt_len = B_TS_LOAD16(pes_header,4);

	if(B_GET_BIT(pes_header[6], 2)) {
		flags = BDEMUX_PES_DATA_ALIGMENT;
	}
	if (B_GET_BITS(pes_header[7], 7, 6) & 2) {
		if (len<14) {
			return bdemux_pes_result_more;
		}
		pts = 
		 /* pts_32_30 */((uint32_t)B_GET_BITS(pes_header[9], 3, 1)<<29) |
		 /* pts_29_22 */((uint32_t)pes_header[10]<<21) |
		 /* pts_21_15 */((uint32_t)B_GET_BITS(pes_header[11], 7, 1)<<14) |
		 /* pts_14_7 */((uint32_t)pes_header[12]<<6) |
		 /* pts_6_0 */B_GET_BITS(pes_header[13], 7, 2);
		flags |= BDEMUX_PES_PTS;
	}
	pes->pes_packets++;
	bdemux_pes_header(pes, flags, pts);
	pes->pkt_data_offset = 9+pes_header[8]; /* PES_header_data_length */
	return bdemux_pes_result_match;
}


#define B_STR_FLAG(v,name) ((v)?#name " ":"")
bdemux_ts_action
bdemux_ts_data(bdemux_ts *ts, unsigned flags, const uint8_t *data, size_t len)
{
	bdemux_pes *pes = (bdemux_pes*)ts;

	BDBG_OBJECT_ASSERT(pes, bdemux_pes);
	BDBG_ASSERT(data);
	BDBG_ASSERT(len>0);
	BDBG_MSG(("bdemux_ts_data: %#lx %#lx:%u:%#x %s%s%s%s",(unsigned long)ts, (unsigned long)data, len, flags, B_STR_FLAG(flags&BDEMUX_TS_ERROR, ERR), B_STR_FLAG(flags&BDEMUX_TS_DISCONTINUITY, DISC),B_STR_FLAG(flags&BDEMUX_TS_MARKED_DISCONTINUITY, MDISC), B_STR_FLAG(flags&BDEMUX_TS_PAYLOAD_UNIT_START,START)));

	if (pes->hold_enable) {
		return bdemux_ts_action_hold;
	}

	/* ISO/IEC 13818-1 */
	/* Table 2-18 -- PES packet */
	pes->ts_flags |= flags;
	if(flags&BDEMUX_TS_PAYLOAD_UNIT_START) {
		switch(b_demux_parse_pes_hdr(pes, data, len)) {
		case bdemux_pes_result_nomatch:
			pes->state = bdemux_pes_state_sync;
			return bdemux_ts_action_consume;
		case bdemux_pes_result_more:
			BDBG_WRN(("bdemux_ts_data:%#lx partial pes header %u", (unsigned long)pes, len));
			BDBG_ASSERT(len<sizeof(pes->pes_header));
			BKNI_Memcpy(pes->pes_header, data, len);
			pes->pkt_offset = len;
			pes->state = bdemux_pes_state_hdr;
			return bdemux_ts_action_consume;
		case bdemux_pes_result_match:
			break;
		}
		pes->state = bdemux_pes_state_data;
		if(len>pes->pkt_data_offset) {
			pes->pkt_offset = pes->pkt_data_offset;
			len-=pes->pkt_data_offset;
			data+=pes->pkt_data_offset;
		} else {
			pes->pkt_offset = len;
			len = 0;
		}
	}
	if(pes->state == bdemux_pes_state_hdr) {
		size_t to_copy;
		BDBG_ASSERT(pes->pkt_offset<sizeof(pes->pes_header));
		to_copy = sizeof(pes->pes_header) - pes->pkt_offset;
		if (to_copy>len) { to_copy=len;}
		BDBG_ASSERT(pes->pkt_offset+to_copy<=sizeof(pes->pes_header));
		BKNI_Memcpy(pes->pes_header+pes->pkt_offset, data, len);
		switch(b_demux_parse_pes_hdr(pes, data, len)) {
		case bdemux_pes_result_nomatch:
			pes->state = bdemux_pes_state_sync;
			return bdemux_ts_action_consume;
		case bdemux_pes_result_more:
			BDBG_ASSERT(to_copy==len);
			/* we have already copied out data */
			return bdemux_ts_action_consume;
		case bdemux_pes_result_match:
			break;
		}
		pes->state = bdemux_pes_state_data;
		if(pes->pkt_offset+len>pes->pkt_data_offset) {
			len-=pes->pkt_data_offset - pes->pkt_offset;
			data+=pes->pkt_data_offset - pes->pkt_offset;
			pes->pkt_offset = pes->pkt_data_offset;
		} else {
			pes->pkt_offset += len;
			len = 0;
		}
	}
	if(pes->state == bdemux_pes_state_data) {
		bdemux_pes_data(pes, data, len, pes->ts_flags);
		pes->ts_flags = 0; /* clear out flags */
		pes->pkt_offset += len;
	}
	return bdemux_ts_action_consume;
}


