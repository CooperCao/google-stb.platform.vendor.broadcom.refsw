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
 * RTP parser library
 * 
 *******************************************************************************/
#include "bstd.h"
#include "btimestamp.h"
#include <inttypes.h>

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
	BDBG_MSG(("ts64 %"PRIu64 "(%"PRIu64 ")  %s(%u)",ts64, ts, timestamp->state==btimestamp_state_wrap?"wrap":"", timestamp->wrap_count ));
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
