/*
 * LOGDUMP_DECODE system definitions
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
 * $Id$
 */


extern int armcycle_per_msec;
#define	TIME_IN_SECONDS(Arm_Cycle)	(((double)Arm_Cycle)/(armcycle_per_msec))/1000

#define MSEC_IN_SECOND 1000

#define MAX_FILE_NAME_SIZE 1024

typedef struct eventdump_info {
	bool first_dump_info;
	FILE *statistic_file;
	FILE *log_file;
} eventdump_info_t;

typedef struct eventdump_header {
	char dump_file_name[MAX_FILE_NAME_SIZE];
	void (*dump_callback)(eventdump_info_t*, uint32*, uint32, uint32);
} eventdump_header_t;

void logdump_ampdu_dump(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_channel_switch(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_rate_cnt(eventdump_info_t *trace_info, uint32 *data,
	uint32 arm_time_cycle, uint32 data_size);
void logdump_mgt_rate_cnt(eventdump_info_t *trace_info, uint32 *data,
	uint32 arm_time_cycle, uint32 data_size);
void logdump_btcx_stats(eventdump_info_t *trace_info, uint32 *data,
	uint32 arm_time_cycle, uint32 data_size);
void logdump_ecounters_ipcstats(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_counters(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_powerstats_phy(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_powerstats_scan(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_powerstats_awdl(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_powerstats_v2(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void ecounters_trigger_reason(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void logdump_wl_LQM(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);
void leaky_ap_stat(eventdump_info_t *trace_info,
	uint32 *data, uint32 arm_time_cycle, uint32 data_size);

extern int register_eventlog_dump_processing_callback(char *output_file_name,
	uint16 tag, void (*callback_function)(eventdump_info_t*,
	uint32*, uint32, uint32));

extern void register_dump_processing_callbacks(void);
