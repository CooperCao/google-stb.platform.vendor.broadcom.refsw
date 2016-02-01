/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * MPEG-2 TS Parser/Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg2psi_parser.h"
#include "tspsi_validate.h"
#include "bkni.h"

BDBG_MODULE(bmpeg2psi_parser);


#define B_STR_FLAG(v,name) ((v)?#name " ":"")

static void 
b_mpeg2psi_parser_section_drop(void *section_cnxt, batom_t atom)
{
	BSTD_UNUSED(section_cnxt);
	batom_release(atom);
	return;
}

void 
bmpeg2psi_parser_flush(bmpeg2psi_parser *psi)
{
	psi->section_length = 0;
	batom_accum_clear(psi->accum);
	return;
}

void
bmpeg2psi_parser_shutdown(bmpeg2psi_parser *psi)
{
	batom_accum_destroy(psi->accum);
}

int 
bmpeg2psi_parser_init(batom_factory_t factory, bmpeg2psi_parser *psi)
{
	psi->accum = batom_accum_create(factory);
	if(!psi->accum) {
		return -1;
	}
	psi->section_length = 0;
	psi->hold_enable = false;
	psi->section_cnxt = NULL;
	psi->section = b_mpeg2psi_parser_section_drop;
	BKNI_Memset(&psi->filter, 0xFF, sizeof(psi->filter));
	return 0;
}

bmpeg2ts_parser_action 
bmpeg2psi_parser_feed(bmpeg2psi_parser *psi, unsigned ts_flags, batom_accum_t src, batom_cursor *payload_start, size_t len)
{
	batom_accum_t accum;
	batom_cursor cursor;
	batom_cursor cursor2;

	BDBG_ASSERT(psi);
	BDBG_ASSERT(payload_start);
	BDBG_ASSERT(src);

	BDBG_MSG(("bmpeg2psi_parser_feed: %p %p:%u:%#x %s%s%s%s",(void *)psi, (void *)payload_start, (unsigned)len, ts_flags, B_STR_FLAG(ts_flags&BMPEG2TS_ERROR, ERR), B_STR_FLAG(ts_flags&BMPEG2TS_DISCONTINUITY, DISC),B_STR_FLAG(ts_flags&BMPEG2TS_MARKED_DISCONTINUITY, MDISC), B_STR_FLAG(ts_flags&BMPEG2TS_PAYLOAD_UNIT_START,START)));

	if(psi->hold_enable) {
		return bmpeg2ts_parser_action_hold;
	}
	accum = psi->accum;
	BATOM_CLONE(&cursor, payload_start);
	batom_cursor_skip(payload_start, len);
	if(ts_flags&BMPEG2TS_PAYLOAD_UNIT_START) {
		size_t pointer;

		batom_accum_clear(accum);
		if(sizeof(psi->data)>len) {
			goto discard;
		}
		pointer = batom_cursor_byte(&cursor); /* read pointer */
		if(pointer+sizeof(psi->data)>len) {
			goto discard;
		}
		len -= pointer;
		batom_cursor_skip(&cursor, pointer); /* skip pointer */
		BATOM_CLONE(&cursor2, &cursor);
		batom_cursor_copy(&cursor2, psi->data, sizeof(psi->data));
		psi->section_length = (((size_t)(psi->data[1]&0x0F))<<8) | psi->data[2];
		psi->section_left = psi->section_length+3;
		BDBG_MSG(("bmpeg2psi_parser_feed: %p found section:%u", (void *)psi, (unsigned)psi->section_length));
		if(!TS_Filter_Compare(psi->data, psi->filter.mask, psi->filter.excl, psi->filter.coef, sizeof(psi->data))) {
			BDBG_MSG(("bmpeg2psi_parser_feed: %p section:%u discarded", (void *)psi, (unsigned)psi->section_length));
			goto discard;
		} 
	} else if(ts_flags & (BMPEG2TS_MARKED_DISCONTINUITY|BMPEG2TS_DISCONTINUITY|BMPEG2TS_ERROR)) {
		goto discard;
	}
	if(psi->section_length) {
		if(len>=psi->section_left) { /* last packet */
			batom_t atom;
			BATOM_CLONE(&cursor2, &cursor);
			batom_cursor_skip(&cursor2, psi->section_left);
			batom_accum_append(accum, src, &cursor, &cursor2);
			atom = batom_from_accum(accum, NULL, NULL);
			BDBG_MSG(("bmpeg2psi_parser_feed: %p extracted section:%p(%u:%u)", (void *)psi, (void *)atom, atom?(unsigned)batom_len(atom):0, (unsigned)psi->section_length));
			psi->section_length = 0;
			if(atom) {
				psi->section(psi->section_cnxt, atom);
			}
		} else {
			psi->section_left -= len;
			batom_accum_append(accum, src, &cursor, payload_start);
		}
	}
	return bmpeg2ts_parser_action_consume;
		
discard:
	bmpeg2psi_parser_flush(psi);
	return bmpeg2ts_parser_action_consume;
}

