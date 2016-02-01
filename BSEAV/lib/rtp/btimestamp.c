/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
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
 * RTP parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "btimestamp.h"

BDBG_MODULE(btimestamp);

void 
btimestamp_init(btimestamp *timestamp)
{
	timestamp->wrap_count = 0;
	timestamp->last_timestamp =  0;
	timestamp->state = btimestamp_state_init;
	return;
}

void 
btimestamp_add(btimestamp *timestamp, uint32_t ts)
{
	switch(timestamp->state) {
	case btimestamp_state_init:
		timestamp->state = btimestamp_state_normal;
		break;
	case btimestamp_state_normal:
		if ( ts < 0x80000000ul && ts < timestamp->last_timestamp && timestamp->last_timestamp > 0x80000000) {
			/* detect ts wrap */
			timestamp->state = btimestamp_state_wrap;
		}
		break; 
	case btimestamp_state_wrap: 
		if (ts > 0x40000000ul) {
			/* once ts is matured (and wouldn't jump back), clear state and increment wrap_count */
			timestamp->state = btimestamp_state_normal;
			timestamp->wrap_count++;
		}
	}
	timestamp->last_timestamp = ts;
	return;
}

uint64_t 
btimestamp_get(const btimestamp *timestamp) 
{
	unsigned wrap_count;
	uint64_t ts64;
	uint64_t ts = timestamp->last_timestamp;

	wrap_count = timestamp->wrap_count;
	if (timestamp->state == btimestamp_state_wrap && ts < 0x80000000ul) {
		/* increase wrap_count for timestamps's that wrapped */
		wrap_count ++;
	}
	ts64 = (((uint64_t)1)<<32)*wrap_count + ts;
	BDBG_MSG(("ts64 %llu(%llu)  %s(%u)",ts64, ts, timestamp->state==btimestamp_state_wrap?"wrap":"", timestamp->wrap_count ));
	return ts64;
}

void 
brtp_ntp_init(brtp_ntp *ntp, uint32_t ntp_msw, uint32_t ntp_lsw)
{
	ntp->ntp_msw = ntp_msw;
	ntp->ntp_lsw = ntp_lsw;
	return;
}

#define B_IABS(n) (((n)>0)?(n):(-(n)))

#define B_RTP_DIFF_MAX ((int)( (1u<<31)-2000))

int
brtp_ntp_diff(const brtp_ntp *a, const brtp_ntp *b)
{
	int32_t ntp_msw;
	uint32_t ntp_lsw;
	int ntp_msec;

	ntp_msw = a->ntp_msw - b->ntp_msw;
	if (a->ntp_lsw<b->ntp_lsw) {
		ntp_msw --;
	}
	ntp_lsw = a->ntp_lsw - b->ntp_lsw;
	BDBG_MSG(("%u", ntp_lsw));
	BDBG_MSG(("%u", (unsigned)(500*(ntp_lsw/(double)(1u<<31))) ));
	ntp_msec = ((ntp_lsw/((1<<10)/8))*(1000/8))/(1<<22); /* == (ntp_lsw * 1000)/(1<<32) */
	BDBG_MSG(("%u %u %u", ntp_lsw, ((1<<10)/8), (ntp_lsw/((1<<10)/8)) ));
	BDBG_MSG(("%u %u %u", (ntp_lsw/((1<<10)/8))*(1000/8), 1<<22, (ntp_lsw/((1<<10)/8))*(1000/8)/(1<<22) ));
	BDBG_MSG(("%d", ntp_msec));
	if(ntp_msw > B_RTP_DIFF_MAX/1000) {
		return B_RTP_DIFF_MAX;
	} else if(ntp_msw < -(B_RTP_DIFF_MAX/1000)) {
		return -B_RTP_DIFF_MAX;
	} 
	ntp_msec += ntp_msw*1000;
	return ntp_msec;
}

/* this function takes two timestamps and returns difference in half rate */
int
brtp_delta_time(uint32_t a, uint32_t b)
{
	int32_t delta;
#if 1
	delta = ((int32_t)(a-b))/2;
#else
	/* 1. scale */
	a /= 2;
	b /= 2;
	delta = a - b;
	/* 2. Handle wraparond cases */
	if (delta > (1<<30)) {
		return delta-(1<<31);
	} else if ( delta < -(1<<30)) {
		return delta+(1<<31);
	}
	delta = ((int32_t)(a-b))/2;
#endif
	return delta;
}
