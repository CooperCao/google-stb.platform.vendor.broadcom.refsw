/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
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
 * Embeddeble profiler library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BPROFILE_H__
#define __BPROFILE_H__

/* #define	BPROFILE_CFG_SINGLE_THREAD	1  */

/* #define	BPROFILE_CFG_PERF_COUNTER	1  */

#if BPROFILE_CFG_PERF_COUNTER
#include  "bperf_counter.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bprofile_sample{
	unsigned time;
#if BPROFILE_CFG_PERF_COUNTER
	unsigned counters[BPROFILE_CFG_PERF_COUNTER];
#endif
}bprofile_sample;

typedef struct bprofile_entry {
#ifndef BPROFILE_CFG_SINGLE_THREAD
	unsigned long event_0;
#endif
	unsigned long addr;
	bprofile_sample sample;
} bprofile_entry;

#define B_PROFILE_EVENT_ENTER	0
#define B_PROFILE_EVENT_EXIT	1
#define B_PROFILE_EVENT_MASK 0x03

typedef struct bprofile_probe_info {
 	bprofile_sample overhead; /* overhewad associated with bprofile samping */
} bprofile_probe_info;

typedef const void *bprofile_thread_id;
typedef struct bprofile_sys_iface {
	/* this is a pointer to a function that is used to convert address to the symbol */
	const char *(*get_name)(unsigned long addr, char *buf, size_t buf_len);
	/* this is a pointer to a function that returns opaque handler id from the  stack pointer, NULL means that stack pointer is invalid */
	bprofile_thread_id (*thread_from_stack)(const unsigned long *stack);
	const char *(*thread_name)(bprofile_thread_id thread);
	size_t maxentries; /* maximum number of allocated entries in the profile report */
	size_t show_entries; /* maximum number of printed entries */
	bool split_threads; /* print separate report for each thread */
	bool substract_overhead; /* substract overhead that was cuased by sampling routines */
	bool call_count; /* print number of function calls */
	bool preempt_time; /* print preemption time */
}bprofile_sys_iface;

/* this function activates acquisiton of profiling data */
void bprofile_start(bprofile_entry *table, size_t nelem);
/* this function stops profiling */
int bprofile_stop(void);
/* this function generates profiler report from the acquired profiling data */
size_t bprofile_report_flat(bprofile_entry *table, size_t nentries, const bprofile_sys_iface *sys_iface);
void bprofile_sys_iface_init(bprofile_sys_iface *sys_iface);

/* this funnction is used to calibrate profiler routines, it returns costs of profiling in ticks */
unsigned bprofile_calibrate(bprofile_entry *table, size_t nelem);
/* this function returns current number of accumulated samples */
int bprofile_poll(void); 

void bprofile_get_info(bprofile_probe_info *info);

#ifdef __cplusplus
}
#endif

#endif /* __BPROFILE_H__ */
