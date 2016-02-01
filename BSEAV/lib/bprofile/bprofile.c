/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * 		Report module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bprofile.h"
#include "bsymtable.h"
#include "bprofile_tick.h"

BDBG_MODULE(bprofile);
#define BDBG_MSG_TRACE(x)  /* BDBG_MSG(x) */

#if BPROFILE_CFG_SINGLE_THREAD
#define B_GET_EVENT(e) (((e).addr)&B_PROFILE_EVENT_MASK)
#define B_GET_ADDR(e) ((e).addr)&(~B_PROFILE_EVENT_MASK)
#else
#define B_GET_EVENT(e) (((e).event_0)&B_PROFILE_EVENT_MASK)
#define B_GET_STACK(e) (((e).event_0)&(~B_PROFILE_EVENT_MASK))
#define B_GET_ADDR(e) (e).addr
#endif

typedef struct bprof_res_entry {
	unsigned addr;
	unsigned count;
	unsigned child_count; /* number of calls into the child functions */
	bprofile_sample all;
	bprofile_sample self;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	bprofile_sample preempt;
#endif
} bprof_res_entry;


typedef struct bprof_call_entry {
	unsigned addr;
	const bprofile_sample *enter;
	const bprofile_sample *child;
	unsigned call_count; /* number of function calls at that level */
	bprofile_sample all;
	bprofile_sample self;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	bprofile_sample preempt;
#endif
	unsigned stack;
} bprof_call_entry;

#define BPROF_CALL_MAX_STACK    512
typedef struct bprof_call_stack {
	unsigned nelem;
	bprof_call_entry stack[BPROF_CALL_MAX_STACK];
} bprof_call_stack;

typedef struct bprof_thread_info {
	unsigned stack_max;
	unsigned stack_min;
	const void *thread_key;
} bprof_thread_info;

#define BPROF_CALL_MAX_THREADS 64
typedef struct bprof_threads {
	unsigned nelem;
	bprof_thread_info threads[BPROF_CALL_MAX_THREADS];
} bprof_threads;

BSTD_INLINE void 
bprofile_sample_init(bprofile_sample *sample)
{
	sample->time = 0;
#if BPROFILE_CFG_PERF_COUNTER
	{
		unsigned i;
		for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
			sample->counters[i] = 0;
		}
	}
#endif
	return;
}

/* returns ts1-ts2 in modulo arithmetic */
BSTD_INLINE unsigned 
b_ts_diff(unsigned ts1, unsigned ts2)
{
	return ts1-ts2;
}
#define B_MAX_TS_DIFF	(1u<<31)

BSTD_INLINE void 
bprofile_sample_add_diff(bprofile_sample *acc, const bprofile_sample *a, const bprofile_sample *b)
{
	unsigned diff = b_ts_diff(a->time, b->time);

	if (diff>B_MAX_TS_DIFF) {
		/* ignore out of order entries */
		return;
	}
	acc->time += diff;

#if BPROFILE_CFG_PERF_COUNTER
	{
		unsigned i;
		for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
			acc->counters[i] += bperf_sample_diff(a->counters[i], b->counters[i]) ;
		}
	}
#endif
	return;
}


BSTD_INLINE void 
bprofile_sample_add(bprofile_sample *acc, const bprofile_sample *a)
{
	acc->time += a->time;
#if BPROFILE_CFG_PERF_COUNTER
	{
		unsigned i;
		for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
			acc->counters[i] += a->counters[i];
		}
	}
#endif
	return;
}

BSTD_INLINE void 
b_prof_res_entry_init(bprof_res_entry *entry)
{
	entry->addr = 0;
	entry->count = 0;
	entry->child_count = 0;
	bprofile_sample_init(&entry->all);
	bprofile_sample_init(&entry->self);
#ifndef BPROFILE_CFG_SINGLE_THREAD
	bprofile_sample_init(&entry->preempt);
#endif
}

static B_BIN_SEARCH(b_search_addr, bprof_res_entry, addr) 

/* shifts down elements in the table */
BSTD_INLINE void 
b_profile_move(bprof_res_entry *table, size_t nentries, unsigned pos)
{
	unsigned i;
	for(i=nentries;i>pos;i--) {
		table[i] = table[i-1];
	}
	return;
}


#if BDBG_DEBUG_BUILD
static bool
b_res_validate(const bprof_res_entry *res, size_t size)
{
	unsigned prev_addr;
	size_t i;
	if (size==0) {
		return true;
	}
	prev_addr = res[0].addr;
	for(i=1;i<size;i++) {
		if(prev_addr>=res[i].addr) {
			for(i=0;i<size;i++) {
				BDBG_ERR(("b_res_validate: %u: %#x", i, res[i].addr));
			}
			return false;
		}
	}
	return true;
}
#endif

BSTD_INLINE void
b_profile_entry_swap(bprofile_entry *a, bprofile_entry *b)
{
	bprofile_entry temp;

	temp = *a;
	*a = *b;
	*b = temp;
	return;
}

BSTD_INLINE void
b_res_heap_swap(bprof_res_entry *a, bprof_res_entry *b)
{
	bprof_res_entry temp;

	temp = *a;
	*a = *b;
	*b = temp;
	return;
}


#define B_RES_KEY_GT(res1,res2) ((res1)->self.time > (res2)->self.time)
/* #define B_RES_KEY_GT(res1,res2) ((res1)->all.time > (res2)->all.time) */

static void 
b_res_heap_sift(bprof_res_entry *res, int start, int count)
{
	int root = start;
	int child;

	while( root*2 + 1 < count) {
		child = root*2 + 1;
		if (child < count -1 && B_RES_KEY_GT(&res[child], &res[child+1])) {
			child++;
		}
		if (B_RES_KEY_GT(&res[root], &res[child])) {
			b_res_heap_swap(&res[root], &res[child]);
			root = child;
		} else {
			return;
		}
	}
	return;
}

static void 
b_res_heap_sort(bprof_res_entry *res, size_t size)
{
	int i;

	for(i=(size/2)-1;i>=0;i--) {
		b_res_heap_sift(res, i, size);
	}
	for(i=size-1;i>0;i--) {
		b_res_heap_swap(&res[i], &res[0]);
		b_res_heap_sift(res, 0, i);
	}
	return;
}

#ifndef BPROFILE_CFG_SINGLE_THREAD

static B_BIN_SEARCH(b_search_thread, bprof_thread_info, stack_max) 

#if BDBG_DEBUG_BUILD
static bool
b_thread_validate(const bprof_threads *threads)
{
	unsigned i;

	for(i=1;i<threads->nelem;i++) {
		if(threads->threads[i].stack_max < threads->threads[i].stack_min) {
			BDBG_ERR(("%u: %#x %#x", i, threads->threads[i].stack_min, threads->threads[i].stack_max));
			return false;
		}
		if(threads->threads[i-1].stack_max < threads->threads[i-1].stack_max) {
			BDBG_ERR(("%u: %#x %#x", i, threads->threads[i-1].stack_max, threads->threads[i].stack_max));
			return false;
		}
	}
	return true;
}
#endif

static void
b_profile_thread_entry(bprof_threads *threads,  const bprofile_sys_iface *sys_iface, const bprofile_entry *entry)
{
	int pos;
	const void *key;
	unsigned i;
	unsigned long stack = B_GET_STACK(*entry);

	pos = b_search_thread(threads->threads, threads->nelem, stack);
	if (pos > 0) {
		return;
	}
	if (pos<0) {
		pos = -(pos+1);
		BDBG_ASSERT(pos>=0);
	}
	if((unsigned)pos<threads->nelem && threads->threads[pos].stack_min <= stack && stack <= threads->threads[pos].stack_max) {
		return;
	}
	/* else search with external function */
	key = sys_iface->thread_from_stack((unsigned long*)stack);
	if (!key) {
		char buf[16];
		BDBG_WRN(("unknown thread for stack %#x[%s]", stack, sys_iface->get_name(entry->addr, buf, sizeof(buf))));
		return; /* invalid stack entry */
	}
	for(i=0;i<threads->nelem;i++) {
		if(threads->threads[i].thread_key == key) {
			/* bingo found key update entry now */
			if (stack < threads->threads[i].stack_min) {
				threads->threads[i].stack_min = stack;
			}  else if (stack > threads->threads[i].stack_max) {
				threads->threads[i].stack_max = stack;
			} else {
				BDBG_ASSERT(false); /* shall have found it before */
			}
			BDBG_ASSERT(b_thread_validate(threads));
			return;
		}
	}
	if (threads->nelem>=BPROF_CALL_MAX_THREADS) {
		/* reached max number of threads */
		return;
	}
	for(i=threads->nelem;i>(unsigned)pos;i--) {
		threads->threads[i] = threads->threads[i-1];
	}
	threads->threads[pos].stack_max = stack;
	threads->threads[pos].stack_min = stack;
	threads->threads[pos].thread_key = key;
	threads->nelem++;
	BDBG_ASSERT(b_thread_validate(threads));
	return;
}
#endif

static void 
b_prof_perf_cnt_head_1(void)
{
#if BPROFILE_CFG_PERF_COUNTER
	unsigned i;
	const bperf_counter_mode *mode = bperf_get_mode();

	if (!mode) {
		return;
	}
	for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
		if (i==0 && mode==&bperf_counter_instructions) {
			BKNI_Printf("%8s ", "issue rate");
		} else {
			BKNI_Printf("%16s ", mode->counter_names[i]);
		}
	}
#endif
}
static void 
b_prof_perf_cnt_head_2(void)
{
#if BPROFILE_CFG_PERF_COUNTER
	unsigned i;
	const bperf_counter_mode *mode = bperf_get_mode();

	if (!mode) {
		return;
	}
	for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
		if (i==0 && mode==&bperf_counter_instructions) {
			BKNI_Printf("%8s ", "%");
		} else {
			BKNI_Printf("%16s ", "total/function");
		}
	}
#endif
}

BSTD_INLINE void 
b_prof_perf_cnt_line(const bprof_res_entry *res)
{
#if BPROFILE_CFG_PERF_COUNTER
	unsigned i;
	const bperf_counter_mode *mode = bperf_get_mode();

	BDBG_ASSERT(res);
	if (!mode) {
		return;
	}
	for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
		unsigned rate;
		const bprofile_sample *sample=&res->self;

		if (i==0 && mode==&bperf_counter_instructions) {
			rate = sample->time/1000;
			if(rate>0) {
				rate = sample->counters[i]/rate;
			} 
			BKNI_Printf("%6u.%1u ", rate/10, rate%10);
			continue;
		}
		if (res->count>0) {
			rate = (10*sample->counters[i])/res->count;
		} else {
			rate = 0;
		}
		BKNI_Printf("%9u/%5u.%1u ",  sample->counters[i], rate/10, rate%10);
	}
#endif
	BSTD_UNUSED(res);
}

static int 
b_prof_get_pos(bprof_res_entry *res, size_t *size, unsigned cur_addr, const bprofile_sys_iface *sys_iface)
{
	int pos = b_search_addr(res, *size, cur_addr);
	/* BDBG_WRN(("i=%u size=%u pos=%d addr=%#x", i, size, pos, table[i].addr)); */
	if (pos<0) {
		/* BDBG_WRN(("insert")); */
		pos = -(pos+1);
		BDBG_ASSERT(pos>=0);
		if (*size>=sys_iface->maxentries) {
			BDBG_LOG(("function table overflowed (%u) %#x", sys_iface->maxentries, cur_addr));
			return -1; /* skip this entry */
		}
		b_profile_move(res, *size, pos);
		b_prof_res_entry_init(&res[pos]);
		res[pos].addr = cur_addr;
		(*size)++;
		BDBG_ASSERT(b_res_validate(res, *size));
	}
	BDBG_ASSERT(res[pos].addr == cur_addr);
	return pos;
}

BSTD_INLINE bool 
b_thread_match(const bprof_thread_info *thread,  unsigned cur_stack)
{
	return cur_stack >= thread->stack_min && cur_stack <= thread->stack_max;
}

typedef struct b_profile_cntx {
	bprofile_entry *table;
	size_t nentries;
	const bprofile_sys_iface *sys_iface;
	bprof_call_stack call_stack;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	bprof_threads threads;
#endif
	unsigned total_time;
	char buf[32];
	char buf2[32];
} b_profile_cntx;
typedef size_t (*b_report_method)(b_profile_cntx *cntx);


static b_profile_cntx *
b_profile_cntx_init( bprofile_entry *table, size_t nentries, const bprofile_sys_iface *sys_iface)
{
	b_profile_cntx *cntx = BKNI_Malloc(sizeof(*cntx));
	if (!cntx) {
		return NULL;
	}
	cntx->table = table;
	cntx->nentries = nentries;
	cntx->sys_iface = sys_iface;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	cntx->threads.nelem = 0;
#endif
	return cntx;
}

static void
b_profile_cntx_shutdown(b_profile_cntx *cntx)
{
	BKNI_Free(cntx);
}

static size_t
b_profile_report(b_report_method report, bprofile_entry *table, size_t nentries, const bprofile_sys_iface *sys_iface)
{
	b_profile_cntx *cntx;
	size_t result=0;
	unsigned diff;
	size_t i;
	unsigned total_time;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	size_t j;
	const bprof_thread_info *thread;
	bprof_threads *threads;
#endif

	BDBG_ASSERT(sys_iface);
	BDBG_ASSERT(table);

	BDBG_MSG(("profile report from %u samples. Options: %u entries limit, %u entries per report %s %s %s %s", nentries, sys_iface->maxentries, sys_iface->split_threads, sys_iface->split_threads?"'split threads'":"", sys_iface->substract_overhead ?"'substract overhead'":"", sys_iface->call_count?"'call count'":"", sys_iface->preempt_time?"'preempt time'":""));
	if(nentries<=0) {
		BKNI_Printf("no profiler data\n");
		goto no_data;
	}
	cntx = b_profile_cntx_init(table, nentries, sys_iface);
	if (!cntx) {
		goto err_cntx;
	}
#ifndef BPROFILE_CFG_SINGLE_THREAD
	threads = &cntx->threads;
	/* 1. Allocate thread entries */
	for(i=0;i<nentries;i++) {
		b_profile_thread_entry(threads, sys_iface, &table[i]);
	}
	/* 2. Locate out of order entries and fix them */
	for(thread=&threads->threads[0];thread<&threads->threads[threads->nelem];thread++) {
		/* first locate first entry that belongs to the current thread */
		for(i=0;i<nentries;i++) {
			if (b_thread_match(thread, B_GET_STACK(table[i]))) {
				break;
			}
		}
		for(;i<nentries;) {
			if (b_thread_match(thread, B_GET_STACK(table[i]))) {
				i++; /* advance to the next entry */
				continue;
			}
			/* 'i' is entry from the another thread, and 'i-1' shall point to the current thread */
			BDBG_ASSERT(i>0);
			BDBG_ASSERT(b_thread_match(thread, B_GET_STACK(table[i-1])));
			diff=b_ts_diff(table[i].sample.time, table[i-1].sample.time);
			if (diff>B_MAX_TS_DIFF) {
				/* out of order sample, move last entry of the current thread all way to the next entry for the current thread */
				BDBG_MSG_TRACE(("reordering start %u [%u]%s->[%u]%s", i, table[i-1].sample.time, sys_iface->get_name(table[i-1].addr, buf, sizeof(buf)), table[i].sample.time, sys_iface->get_name(table[i].addr, buf2, sizeof(buf2))));
				j=i;
				for(;i<nentries;i++) { 
					if (b_thread_match(thread, B_GET_STACK(table[i]))) {
						break;
					}
					b_profile_entry_swap(&table[i], &table[i-1]); /* push current entry forward */
				}
				BDBG_MSG_TRACE(("reordering stop %u [%u]%s->[%u]%s", i, table[i-1].sample.time, sys_iface->get_name(table[i-1].addr, buf, sizeof(buf)), table[i].sample.time, sys_iface->get_name(table[i].addr, buf2, sizeof(buf2))));
				BDBG_MSG_TRACE(("moved %u[%u..%u] out of order thread entries", i-j,j,i));
			} else {
				/* no discontinuity, just skip entries from other threads */
				for(;i<nentries;i++) { 
					if (b_thread_match(thread, B_GET_STACK(table[i]))) {
						break;
					}
				}
			}
		}
	}
#endif
	for(total_time=0,i=1;i<nentries;i++) {
		diff=b_ts_diff(table[i].sample.time, table[i-1].sample.time);
		if (diff>B_MAX_TS_DIFF) {
			BDBG_WRN(("out of order entry %u [%u]%s->[%u]%s", i, table[i-1].sample.time, sys_iface->get_name(table[i-1].addr, cntx->buf, sizeof(cntx->buf)), table[i].sample.time, sys_iface->get_name(table[i].addr, cntx->buf2, sizeof(cntx->buf2))));
			continue;
		}
		total_time+=diff;
	}
	cntx->total_time = total_time;
	result = report(cntx);
	
	b_profile_cntx_shutdown(cntx);
err_cntx:
no_data:
	return result;
}

BSTD_INLINE unsigned 
b_profile_adjust_time(unsigned time, unsigned overhead)
{
	if (time>overhead) {
		return time-overhead;
	} else {
		return 0;
	}
}

static void
b_profile_report_flat_print(b_profile_cntx *cntx, bprof_res_entry *res, size_t nelem, unsigned total_time)
{
	const bprofile_sys_iface *sys_iface=cntx->sys_iface;
	size_t i;
	b_tick2ms_init();

	if(sys_iface->substract_overhead) {
		bprofile_probe_info probe_info;

		bprofile_get_info(&probe_info);

		for(i=0;i<nelem;i++) {
			bprof_res_entry *cur = &res[i];
			unsigned overhead = (probe_info.overhead.time * cur->count)/2; 
			if (overhead > cur->self.time) {
				BDBG_MSG(("overhead estimation: %s %ux(%u) %u", sys_iface->get_name(res[i].addr, cntx->buf, sizeof(cntx->buf)), res[i].count, overhead, res[i].self.time));
			}
			cur->self.time = b_profile_adjust_time(res[i].self.time, overhead); /* self time takes only one penalty */
			overhead += probe_info.overhead.time*cur->child_count; /* all time has additional parent's penalty for each call to a children */
			cur->all.time = b_profile_adjust_time(cur->all.time, overhead); 
#if BPROFILE_CFG_PERF_COUNTER
			{
				unsigned j;
				for(j=0;j<BPROFILE_CFG_PERF_COUNTER;j++) {

					overhead = (probe_info.overhead.counters[j]* cur->count)/2; 
					cur->self.counters[j] = b_profile_adjust_time(cur->self.counters[j], overhead); /* self takes only one penalty */
					overhead += probe_info.overhead.counters[j]*cur->child_count; /* all has additional parent's penalty for each call to a child */
					cur->all.counters[j] = b_profile_adjust_time(cur->all.counters[j], overhead); 
				}
			}
#endif
		}
	}

	/* sort table based on time */
	b_res_heap_sort(res, nelem);
	total_time /= 1000; /* scale to make percentage */
	if(total_time==0) {
		total_time = 1;
	}
	BKNI_Printf("%5c %10s %8s ", '%', "cumulative", "self");
#ifndef BPROFILE_CFG_SINGLE_THREAD
	if(sys_iface->preempt_time) {
		BKNI_Printf("%8s ", "preempt");
	}
#endif
	b_prof_perf_cnt_head_1();
	if(sys_iface->call_count) {
		BKNI_Printf("%8s ", "function");
	}
	BKNI_Printf("%8s %s\n", "", "");
	BKNI_Printf("%5s %10s %8s ", "time", "msec", "msec");
#ifndef BPROFILE_CFG_SINGLE_THREAD
	if(sys_iface->preempt_time) {
		BKNI_Printf("%8s ", "msec");
	}
#endif
	b_prof_perf_cnt_head_2();
	if(sys_iface->call_count) {
		BKNI_Printf("%8s ", "calls");
	}
	BKNI_Printf("%8s %s\n", "calls", "name");
	for(i=0;i<nelem;i++) {
		unsigned time_all = b_tick2_100us(res[i].all.time);
		unsigned time_self = b_tick2_100us(res[i].self.time); 
		unsigned percent = res[i].self.time/total_time; 
#define B_100US_FORMAT(n) "%"#n"u.%1u"
#define B_100US_DATA(v) v/10, v%10
		if (i>sys_iface->show_entries) {
			break;
		}

		BKNI_Printf("%3u.%1u" B_100US_FORMAT(8) " " B_100US_FORMAT(6) " " , percent/10, percent%10, B_100US_DATA(time_all), B_100US_DATA(time_self));
#ifndef BPROFILE_CFG_SINGLE_THREAD
		if(sys_iface->preempt_time) {
			unsigned time_preempt = b_tick2_100us(res[i].preempt.time); 
			BKNI_Printf(B_100US_FORMAT(6) " " , B_100US_DATA(time_preempt));
		}
#endif
		b_prof_perf_cnt_line(&res[i]);
		if(sys_iface->call_count) {
			BKNI_Printf("%8u ", res[i].child_count);
		}

		BKNI_Printf("%8u %s\n", res[i].count, sys_iface->get_name(res[i].addr, cntx->buf, sizeof(cntx->buf)));
	}

}

static size_t 
b_profile_report_flat(b_profile_cntx *cntx)
{
	const bprofile_sys_iface *sys_iface=cntx->sys_iface;
	bprof_res_entry *res = BKNI_Malloc(sizeof(*res)*sys_iface->maxentries);
	size_t i;
	bprof_call_stack *call_stack = &cntx->call_stack;
	size_t nentries = cntx->nentries;
	const bprofile_entry *table = cntx->table;
	const bprofile_sample *cur_sample;
	bprof_call_entry *cur;
	int pos;
	size_t size=0;
	const bprof_thread_info *thread;
#ifndef BPROFILE_CFG_SINGLE_THREAD
	size_t j;
	const bprof_threads *threads = &cntx->threads;
#endif
	b_tick2ms_init();


	BKNI_Printf("total %u msec\n", b_tick2ms(cntx->total_time));
#if BPROFILE_CFG_SINGLE_THREAD
	BSTD_UNUSED(thread);
	{
#else
	for(thread=&threads->threads[0];thread<&threads->threads[threads->nelem];thread++) {
		unsigned thread_time = 0;
		unsigned diff;
		unsigned prev_time;
		BDBG_MSG(("[%#x] %#x:%#x %s", thread, thread->stack_min, thread->stack_max, sys_iface->thread_name(thread->thread_key)));
#endif
		/* this loop keeps result table sorted by function address, this achieves
		 * O(log2(N)) lookup complexivity */
		call_stack->nelem = 0;
		i=0;
#ifndef BPROFILE_CFG_SINGLE_THREAD
		/* skip other threads */
		for(;i<nentries;i++) {
			if (b_thread_match(thread, B_GET_STACK(table[i]))) {
				break;
			}
		}
		prev_time = table[i].sample.time;
#endif
		for(;i<nentries;i++) {
			bprof_call_entry *parent;
			unsigned cur_addr;

#ifndef BPROFILE_CFG_SINGLE_THREAD
			/* BDBG_MSG(("entry %u %s", i, sys_iface->get_name(table[i].addr, buf, sizeof(buf))));  */
			diff=b_ts_diff(table[i].sample.time, prev_time);
			if (diff<B_MAX_TS_DIFF) {
				thread_time+=diff;
			}
			prev_time = table[i].sample.time;
			if (!b_thread_match(thread, B_GET_STACK(table[i]))) {

				/* search thread match */
				for(j=i+1;j<nentries;j++) {
					if (b_thread_match(thread, B_GET_STACK(table[j]))) {
						break;
					}
				}
				BDBG_MSG_TRACE(("filtered out entries %u..%u", i, j));
				if(j>=nentries) {
					i=j; /* reached out of table */
					continue;
				}
				if (call_stack->nelem>0) {
					/* account for the thread preemption */
					parent = &call_stack->stack[call_stack->nelem-1];
					bprofile_sample_add_diff(&parent->preempt, &table[j].sample, &table[i].sample);
					bprofile_sample_add_diff(&parent->self, &table[i].sample, parent->child);
					parent->child = &table[j].sample;
					BDBG_MSG_TRACE(("filtered out %u[%u] entries %s %u[%#x:%u]...%s %u[%#x:%u] -> %u", j-i, call_stack->nelem, sys_iface->get_name(parent->addr, buf, sizeof(buf)), i, B_GET_STACK(table[i]), table[i].sample.time, sys_iface->get_name(table[j].addr, buf2, sizeof(buf2)), j, B_GET_STACK(table[j]), table[j].sample.time, parent->preempt.time));
				}
				i=j;
				prev_time = table[i].sample.time;
			}
#endif
			cur_addr = B_GET_ADDR(table[i]);

			cur_sample = &table[i].sample;
			switch(B_GET_EVENT(table[i])) {
			default:
				BDBG_ASSERT(0);
				break;
			case B_PROFILE_EVENT_ENTER:
				BDBG_ASSERT(call_stack->nelem < BPROF_CALL_MAX_STACK);
				/* initialize call-stack entry */
				cur = &call_stack->stack[call_stack->nelem];
				cur->addr = cur_addr;
				cur->enter = cur_sample;
				cur->child = cur_sample;
				cur->call_count = 0;
				bprofile_sample_init(&cur->self);
				bprofile_sample_init(&cur->all);
#ifndef BPROFILE_CFG_SINGLE_THREAD
				cur->stack = B_GET_STACK(table[i]);
				bprofile_sample_init(&cur->preempt);
#endif
				if(call_stack->nelem>0) {
					/* update parent's self_time */
					parent = &call_stack->stack[call_stack->nelem-1];
					parent->call_count++;
					bprofile_sample_add_diff(&parent->self, cur_sample, parent->child);
				}
				call_stack->nelem++;
				break;
			case B_PROFILE_EVENT_EXIT: 

				pos  = b_prof_get_pos(res, &size, cur_addr, sys_iface);
				if (pos<0) {
					continue;
				}
				BDBG_ASSERT(res[pos].addr == cur_addr);
				if (call_stack->nelem>0) {
					/* remove element from the top of stack */
					call_stack->nelem--;
					cur = &call_stack->stack[call_stack->nelem];
					if(cur->addr != cur_addr) {
						unsigned drop;

						for(drop=0;drop<=call_stack->nelem;drop++) {
							if(call_stack->stack[call_stack->nelem-drop].addr == cur_addr) {
								break;
							}
						}
						BDBG_MSG(("out of sync:%u(%u) entry %u %s:%s pop %u", drop, call_stack->nelem, i, sys_iface->get_name(cur->addr, cntx->buf, sizeof(cntx->buf)), sys_iface->get_name(table[i].addr, cntx->buf2, sizeof(cntx->buf2)), i ));
						if (drop>call_stack->nelem) {
							call_stack->nelem++; /* unpaired exit, ignore it */
							break;
						}
					}
					res[pos].count++;
					res[pos].child_count += cur->call_count; 
					bprofile_sample_add_diff(&cur->all, cur_sample, cur->enter);
					bprofile_sample_add(&res[pos].all, &cur->all);
					bprofile_sample_add_diff(&cur->self, cur_sample, cur->child);
					bprofile_sample_add(&res[pos].self, &cur->self);
					/* BDBG_WRN(("%u/%u", cur->preempt.time, call_stack->nelem)); */
#ifndef BPROFILE_CFG_SINGLE_THREAD
					bprofile_sample_add(&res[pos].preempt, &cur->preempt);
#endif
					if (call_stack->nelem>0) {
						/* update parent's child exit time */
						parent = &call_stack->stack[call_stack->nelem-1];
						parent->child = cur_sample;
					}
				}

				break;
			}
		}
		/* handle functions that didn't exit yet */
		for(i=0;i<call_stack->nelem;i++) {
			cur = &call_stack->stack[i];
			pos = b_prof_get_pos(res, &size, cur->addr, sys_iface);
			if (pos<0) {
				continue;
			}
			BDBG_ASSERT(res[pos].addr == cur->addr);
			res[pos].count++;
			bprofile_sample_add_diff(&cur->all, cur->child, cur->enter);
			bprofile_sample_add(&res[pos].all, &cur->all);
			bprofile_sample_add(&res[pos].self, &cur->self);
#ifndef BPROFILE_CFG_SINGLE_THREAD
			bprofile_sample_add(&res[pos].preempt, &cur->preempt);
#endif
		}
#ifndef BPROFILE_CFG_SINGLE_THREAD
		if(sys_iface->split_threads && size > 0) {
			unsigned load = cntx->total_time/1000;
			if (load>0) {
				load = thread_time/load;
			}
			/* print out per thread results */
			BKNI_Printf("thread: '%s' %3u.%1u%% self %u msec\n", sys_iface->thread_name(thread->thread_key),  load/10, load%10, b_tick2ms(thread_time));
			b_profile_report_flat_print(cntx, res, size, thread_time);
			size = 0;
		}
#endif
	}
#ifndef BPROFILE_CFG_SINGLE_THREAD
	if(!sys_iface->split_threads) 
#endif
	{
		b_profile_report_flat_print(cntx, res, size, cntx->total_time);
	}

	BKNI_Free(res);
	return size;
}
	
size_t 
bprofile_report_flat(bprofile_entry *table, size_t nentries, const bprofile_sys_iface *sys_iface)
{
	return b_profile_report(b_profile_report_flat, table, nentries, sys_iface);
}

static const char *
b_profile_no_get_name(unsigned long addr, char *buf, size_t buf_len)
{
	BKNI_Snprintf(buf, buf_len, "%08lx", (unsigned long)addr);
	return buf;
}

static bprofile_thread_id
b_profile_no_thread_from_stack(const unsigned long *stack)
{
	BSTD_UNUSED(stack);
	return (bprofile_thread_id)(unsigned long)b_profile_no_thread_from_stack;
}

static const char *
b_profile_no_thread_name(bprofile_thread_id thread)
{
	BSTD_UNUSED(thread);
	return "";
}

void 
bprofile_sys_iface_init(bprofile_sys_iface *sys_iface)
{
	BDBG_ASSERT(sys_iface);
	sys_iface->get_name = b_profile_no_get_name;
	sys_iface->thread_from_stack = b_profile_no_thread_from_stack;
	sys_iface->thread_name = b_profile_no_thread_name;
	sys_iface->maxentries = 256;
	sys_iface->show_entries = 256;
	sys_iface->split_threads = false;
	sys_iface->substract_overhead = false;
	sys_iface->call_count = false;
	sys_iface->preempt_time = false;
	return;
}
