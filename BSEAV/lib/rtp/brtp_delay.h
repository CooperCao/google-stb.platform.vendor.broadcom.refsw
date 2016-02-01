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
#ifndef BRTP_DELAY_H__
#define BRTP_DELAY_H__

#include "bfifo.h"


#ifdef __cplusplus
extern "C"
{
#endif
#define B_RTP_PTS_DELAY_FIFO	128
BFIFO_HEAD(b_pts_delay_fifo, int);
typedef struct bpts_delay {
	struct b_pts_delay_fifo fifo;
	int data[B_RTP_PTS_DELAY_FIFO];
} bpts_delay;

void bpts_delay_init(bpts_delay *delay);
void bpts_delay_add(bpts_delay *delay, int diff);

typedef struct bdelay_statistics {
	double sum; 
	double square_sum;
	int min;
	int max;
	unsigned n;
} bdelay_statistics;


void bdelay_statistics_clear(bdelay_statistics *stats);
void bdelay_statistics_add(bdelay_statistics *stats, const int *data, size_t nitems);
unsigned bdelay_statistics_entries(const bdelay_statistics *stats);
int bdelay_statistics_mean(const bdelay_statistics *stats);
int bdelay_statistics_deviation(const bdelay_statistics *stats);
int bdelay_statistics_min(const bdelay_statistics *stats);
int bdelay_statistics_max(const bdelay_statistics *stats);
void bpts_delay_run_stats(bpts_delay *delay, bdelay_statistics *stats);

typedef struct bdecoder_info {
	int min_delay;
	int max_delay;
} bdecoder_info;

int bdecoder_get_stc_step(const bdecoder_info *info, const bdelay_statistics *stats);

#ifdef __cplusplus
}
#endif

#endif /* BRTP_DELAY_H__ */

