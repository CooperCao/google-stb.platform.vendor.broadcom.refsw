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
#include "bdemux_ts.h"
#include "biobits.h"
BDBG_MODULE(bdemux_ts);


BDBG_OBJECT_ID(bdemux_ts);
void 
bdemux_ts_init(bdemux_ts *ts, uint16_t pid)
{
	BDBG_ASSERT(ts);
	BDBG_OBJECT_INIT(ts, bdemux_ts);
	ts->pid = pid;
	ts->continuity_counter = 0;
	ts->flags = 0;

	/* verify B_GET_BITS */
	BDBG_CASSERT(B_GET_BITS(0xDE,7,4)==0xD); 
	BDBG_CASSERT(B_GET_BITS(0xDE,3,0)==0xE); 
	BDBG_CASSERT(B_GET_BITS(0xDE,3,3)==0x1); 
	BDBG_CASSERT(B_GET_BITS(0xDE,1,1)==0x1); 
	BDBG_CASSERT(B_GET_BITS(0xDE,0,0)==0x0); 
	BDBG_CASSERT(B_GET_BITS(0xDE,6,0)==0x5E); 
	BDBG_CASSERT(!B_GET_BIT(0xDE,0)); 
	BDBG_CASSERT(B_GET_BIT(0xDE,1)); 
	BDBG_CASSERT(B_GET_BIT(0xDE,7)); 
	BDBG_CASSERT(!B_GET_BIT(0xDE,5)); 

	BDBG_CASSERT(B_SET_BIT( _ , 1, 0) == 1);
	BDBG_CASSERT(B_SET_BIT( _ , 2, 0) == 1);
	BDBG_CASSERT(B_SET_BIT( _ , 1, 1) == 2);
	BDBG_CASSERT(B_SET_BIT( _ , 1, 31) == (1u<<31));
	BDBG_CASSERT(B_SET_BITS( _ , 1, 0, 0) == 1);
	BDBG_CASSERT(B_SET_BITS( _ , 1, 1, 0) == 1);
	BDBG_CASSERT(B_SET_BITS( _ , 1, 1, 1) == 2);
	BDBG_CASSERT(B_SET_BITS( _ , 0x55, 7, 1) == 0xAA);
	return;
}

#define B_NEXT_CONT_COUNTER(c) (((c)+1)&0x0F)

int
bdemux_ts_feed(bdemux_ts *ts, const uint8_t *data, size_t len)
{
	size_t left;
	unsigned ts_continuity_counter;

	BDBG_OBJECT_ASSERT(ts, bdemux_ts);
	BDBG_ASSERT(data);
	BDBG_ASSERT(((unsigned long)data&3)==0); /* check that data is 32 bit alligned */
	BDBG_ASSERT(len>0);
	BDBG_ASSERT((len%B_TS_PKT_LEN)==0);

	/* ISO/IEC 13818-1 */

	ts_continuity_counter = ts->continuity_counter;
	for(left=len;left>=B_TS_PKT_LEN;left-=B_TS_PKT_LEN,data+=B_TS_PKT_LEN) {
		uint32_t word;
		unsigned pid;
		unsigned adaptation_field_control;
		unsigned continuity_counter;
		unsigned off; /* offset into the transport packet */
		unsigned flags = ts->flags;

		if (data[0]!=B_TS_SYNC) {
			goto err_out_of_sync;
		}
		BDBG_ASSERT(data[0]==B_TS_SYNC);

		/* Table 2-3 -- ITU-T Rec. H.222.0 | ISO/IEC 13818 transport packet */
		word = B_TS_LOAD16(data, 1);
		pid = B_GET_BITS(word, 12, 0);
		if (pid!=ts->pid) {
			BDBG_MSG(("bdemux_ts_feed: %#lx wrong pid %#x(%#x)", (unsigned long)ts, (unsigned)pid, (unsigned)ts->pid));
			goto wrong_pid;
		}
		if (B_GET_BIT(word, 15) /* transport_error_indicator */) {
			goto ts_error;
		}
		if (B_GET_BIT(word, 14) /* transport_error_indicator */) {
			flags |= BDEMUX_TS_PAYLOAD_UNIT_START;
		}
		word = B_TS_LOAD8(data, 3);
		continuity_counter = B_GET_BITS(word, 3, 0);
		adaptation_field_control = B_GET_BITS(word, 5, 4);
		if (continuity_counter != ts_continuity_counter) {
			goto cc_error;
		}
after_cc:
		off = 4;
		if (adaptation_field_control==0x02 || adaptation_field_control==0x03) {
			goto adaptation_payload;
		}
after_adaptation_payload:
		if (adaptation_field_control==0x01) {
#if 1
			bdemux_ts_action action = bdemux_ts_data(ts, flags, data+off, B_TS_PKT_LEN-off);
#else
			bdemux_ts_action action = bdemux_ts_action_consume;
#endif
			flags = 0; /* clear flags */
			if (action==bdemux_ts_action_hold) {
				break;
			}
		}
next_cc:
		ts_continuity_counter = B_NEXT_CONT_COUNTER(continuity_counter);
done:
		ts->flags = flags; /* save flags */
wrong_pid:
		;
		continue;
ts_error:
		BDBG_WRN(("bdemux_ts_feed: %#lx transport_error_indicator", (unsigned long)ts));
		flags |= BDEMUX_TS_ERROR;
		goto done;
cc_error:
		if (!(adaptation_field_control==0x00 || adaptation_field_control==0x02)) {
			if(B_NEXT_CONT_COUNTER(continuity_counter)==ts_continuity_counter) {
				BDBG_WRN(("bdemux_ts_feed: %#lx continuity counter duplicate %#x(%#x)", (unsigned long)ts, (unsigned)continuity_counter, (unsigned)ts->continuity_counter));
				goto done;
			}
			BDBG_MSG(("bdemux_ts_feed: %#lx continuity counter error %#x(%#x)", (unsigned long)ts, (unsigned)continuity_counter, (unsigned)ts->continuity_counter));
			flags |= BDEMUX_TS_DISCONTINUITY;
		}
		goto after_cc;
adaptation_payload:
		{
			unsigned adaptation_field_length = B_TS_LOAD8(data, 4);
			if (adaptation_field_length>0) {
				word = B_TS_LOAD8(data, 5);
				if (B_GET_BIT(word, 7) /* discontinuity_indicator  */) {
					flags |= BDEMUX_TS_MARKED_DISCONTINUITY;
				}
			}
			off = 5 + adaptation_field_length;
			if (off>B_TS_PKT_LEN) {
				BDBG_WRN(("bdemux_ts_feed: %#lx invalid adaptation_field_length=%u", (unsigned long)ts, adaptation_field_length));
				goto next_cc;
			}
			adaptation_field_control &= ~2; /* clear bit 1, e.g. 3 would become 1 */
		}
		goto after_adaptation_payload;
	}
	ts->continuity_counter = ts_continuity_counter;
	BDBG_ASSERT(len>=left);
	return len-left;
err_out_of_sync:
	BDBG_ERR(("out of_sync %02x:%02x:%02x:%02x(%#lx) at %u:%u", data[0], data[1], data[2], data[3], (unsigned long)data, len-left, len));
	ts->continuity_counter = ts_continuity_counter;
	if(len==left) {
		return -B_TS_PKT_LEN;
	}
	return -(len-left);
}



