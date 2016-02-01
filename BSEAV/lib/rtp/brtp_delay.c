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
 * RTP delay and statistics module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bfifo.h"
#include "brtp_delay.h"
#include <math.h>

BDBG_MODULE(brtp_delay);

void 
bpts_delay_init(bpts_delay *delay)
{
	BFIFO_INIT(&delay->fifo, delay->data, B_RTP_PTS_DELAY_FIFO);
	return;
}

void 
bpts_delay_add(bpts_delay *delay, int diff)
{
	int *entry;

	if(BFIFO_WRITE_PEEK(&delay->fifo)==0) {
		BDBG_ASSERT(BFIFO_READ_PEEK(&delay->fifo)>0);
		BFIFO_READ_COMMIT(&delay->fifo, 1); /* consume one entry in case of overflow */
	}
	BDBG_ASSERT(BFIFO_WRITE_PEEK(&delay->fifo)>0);
	entry = BFIFO_WRITE(&delay->fifo);
	*entry = diff;
	BFIFO_WRITE_COMMIT(&delay->fifo, 1);
	return;
}

void
bdelay_statistics_clear(bdelay_statistics *stats)
{
	stats->n = 0;
	stats->square_sum = 0.0;
	stats->sum = 0;
	stats->min = INT32_MAX;
	stats->max = INT32_MIN;
	return;
}

void
bdelay_statistics_add(bdelay_statistics *stats, const int *data, size_t nitems)
{
	double sum = stats->sum;
	double square_sum = stats->square_sum;
	int min = stats->min;
	int max = stats->max;
	unsigned i;
	int d;

	for(i=0;i<nitems;i++) {
		d = data[i];
		sum += (float)d;
		square_sum += (float)d*(float)d;
		if (min > d) {
			min = d;
		}
		if (max < d) {
			max = d;
		}
	}
	stats->sum = sum;
	stats->square_sum = square_sum;
	stats->min = min;
	stats->max = max;
	stats->n += nitems;
	
	return;
}

unsigned
bdelay_statistics_entries(const bdelay_statistics *stats)
{
	return stats->n;
}

int
bdelay_statistics_mean(const bdelay_statistics *stats)
{
	if (stats->n==0) {
		BDBG_ERR(("bdelay_statistics_mean: no data"));
		return 0;
	} else {
		double mean = stats->sum/stats->n;

		return (int)(floor(mean + 0.5));
	}
}

int
bdelay_statistics_deviation(const bdelay_statistics *stats)
{
	if (stats->n==0) {
		BDBG_ERR(("bdelay_statistics_deviation: no data"));
		return 0;
	} else {
		double n = stats->n; 
		double mean = stats->sum/n;
		double deviation = stats->square_sum/n - mean*mean;

		if (deviation>0) {
			deviation = sqrt(deviation);
		} else {
			deviation = 0;
		}
		return (int)(floor(deviation + 0.5));
	}
}

int
bdelay_statistics_min(const bdelay_statistics *stats)
{
	return stats->min;
}

int
bdelay_statistics_max(const bdelay_statistics *stats)
{
	return stats->max;
}

void
bpts_delay_run_stats(bpts_delay *delay, bdelay_statistics *stats)
{
	unsigned i;
	unsigned cnt;

	bdelay_statistics_clear(stats);
	for(i=2;i>0;i++) {
		cnt = BFIFO_READ_PEEK(&delay->fifo);
		if (cnt<=0) {
			break;
		}
		bdelay_statistics_add(stats, BFIFO_READ(&delay->fifo), cnt);
		BFIFO_READ_COMMIT(&delay->fifo, cnt);
	}
	return;
}

int
bdecoder_get_stc_step(const bdecoder_info *info, const bdelay_statistics *stats)
{
	int mean;
	int deviation;
	int delta=0;

	mean = bdelay_statistics_mean(stats);
	delta = mean - info->min_delay;
	if (delta >= 0) {
		delta = mean - info->max_delay;
		if (delta <=0) {
			return 0; /* no correction needed */
		}
		BDBG_MSG(("overflow %d %d %d", delta, mean, info->min_delay));
	} else {
		BDBG_MSG(("underflow %d %d %d", delta, mean, info->min_delay));
	}
	deviation = bdelay_statistics_deviation(stats);
	if ( (delta > 0 && deviation>delta) || (delta < 0 && deviation>-delta)) {
		return 0; /* to much noise */
	}
	if (delta < 0) {
		delta = mean - (info->max_delay + info->min_delay)/2;
	} else {
		delta = mean - (info->max_delay + info->min_delay)/2;
	}
	BDBG_MSG(("proposed stc step %d", delta));
	return delta;
}

