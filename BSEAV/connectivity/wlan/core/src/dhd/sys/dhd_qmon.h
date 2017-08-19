/*
 * Queue monitoring.
 *
 * The feature allows monitoring the DHD queue utilization to get the percentage of a time period
 * where the number of pending packets is above a configurable theshold.
 * Right now, this is used by a server application, interfacing a Miracast Video Encoder, and
 * doing IOVAR "qtime_percent" at regular interval. Based on IOVAR "qtime_percent" results,
 * the server indicates to the Video Encoder if its bitrate can be increased or must be decreased.
 * Currently, this works only with P2P interfaces and with PROP_TXSTATUS. There is no need to handle
 * concurrent access to the fieds because the existing concurrent accesses are protected
 * by the PROP_TXSTATUS's lock.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_qmon.h 309265 2012-01-19 02:50:46Z $
 *
 */
#ifndef _dhd_qmon_h_
#define _dhd_qmon_h_


typedef struct dhd_qmon_s {
	uint32	transitq_count;
	uint32  queued_time_thres;
	uint64  queued_time_cumul;
	uint64  queued_time_cumul_last;
	uint64  queued_time_last;
	uint64  queued_time_last_io;
} dhd_qmon_t;


extern void dhd_qmon_reset(dhd_qmon_t* entry);
extern void dhd_qmon_tx(dhd_qmon_t* entry);
extern void dhd_qmon_txcomplete(dhd_qmon_t* entry);
extern int dhd_qmon_getpercent(dhd_pub_t *dhdp);
extern int dhd_qmon_thres(dhd_pub_t *dhdp, int set, int setval);


#endif	/* _dhd_qmon_h_ */
