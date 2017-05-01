/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Embeddable profiler library
 *  Data acquisition module
 *
 *******************************************************************************/
#include "bstd.h"
#include "bprofile.h"
#include "bkni.h"
#include "batomic.h"
#include "btrc.h"
#include "bprofile_tick.h"



BDBG_MODULE(bprofile);

#define DONT_PROFILE __attribute__((no_instrument_function))

struct bprofile_state {
	/* bprofile_entry *next; */
	batomic_t next;
	batomic_t last;
	const bprofile_entry *first;
	bprofile_probe_info info;
};

#define B_PROFILE_SEED	1

static struct bprofile_state b_profile_state = {
	BATOMIC_INIT(B_PROFILE_SEED),
	BATOMIC_INIT(B_PROFILE_SEED),
	(void *)B_PROFILE_SEED,
	{
		{0 /* time */
#if BPROFILE_CFG_PERF_COUNTER
		,{0
#if BPROFILE_CFG_PERF_COUNTER > 1
		,0
#endif
#if BPROFILE_CFG_PERF_COUNTER > 2
		,0
#endif
#if BPROFILE_CFG_PERF_COUNTER > 3
		,0
#endif
		}
#endif
	}}
};

static void DONT_PROFILE
b__profile_start(bprofile_entry *table, size_t nelem) 
{
	if (nelem>4) {
		BKNI_Memset(table, 0, sizeof(*table)*nelem);
		b_profile_state.first = table;
		batomic_set(&b_profile_state.next, ((unsigned long)table)-sizeof(*table));
		batomic_set(&b_profile_state.last, ((unsigned long)table)+(nelem-4)*sizeof(*table));
	}
	return;
}


int DONT_PROFILE
bprofile_stop(void) 
{
	bprofile_entry  *entry = (bprofile_entry *)batomic_get(&b_profile_state.next); 
	/* printf("stop first:%p last:%p next:%p %d\n", b_profile_state.first, b_profile_state.last,  b_profile_state.next, b_profile_state.next - b_profile_state.first); */
	batomic_set(&b_profile_state.last, 1);
	return (entry+1) - b_profile_state.first;
}

int DONT_PROFILE 
bprofile_poll(void)
{
	bprofile_entry  *entry = (bprofile_entry *)batomic_get(&b_profile_state.next); 
	return (entry+1) - b_profile_state.first;
}

static unsigned DONT_PROFILE
b_profile_dummy_0(unsigned a, unsigned b)
{
	return a+b;
}

static unsigned DONT_PROFILE
b_profile_dummy_1(unsigned a, unsigned (*dummy)(unsigned, unsigned))
{
	return a+dummy(a,a);
}

static unsigned 
b_profile_dummy_2(unsigned a, unsigned (*dummy)(unsigned, unsigned))
{
	return a+dummy(a,a);
}


BTRC_MODULE(bprofile_nosample,ENABLE);
BTRC_MODULE(bprofile_sample,ENABLE);

BSTD_INLINE void 
b_profile_sample_diff(bprofile_sample *a, const bprofile_sample *b)
{
#if BPROFILE_CFG_PERF_COUNTER
	unsigned i;
	for(i=0;i<BPROFILE_CFG_PERF_COUNTER;i++) {
		a->counters[i] = bperf_sample_diff(a->counters[i], b->counters[i]) ;
	}
#endif
	a->time = a->time - b->time;
	return;
}

static unsigned DONT_PROFILE
b_profile_calibrate(bprofile_entry *table)
{
	unsigned i;
	bprofile_sample sample0,sample1,sample2;
	unsigned (*dummy)(unsigned, unsigned (*)(unsigned, unsigned)); /* use the function pointers that function call isn't inlined by an accident */
	const unsigned nloops = 10 * 1000;
	unsigned time_100us;
	b_tick2ms_init();

	BSTD_UNUSED(table);

	dummy = b_profile_dummy_1;
    BDBG_MSG(("b_profile_calibrate: %#lx capturing trace ...", (unsigned long)table));
	BTRC_TRACE(bprofile_nosample, START);
    BDBG_MSG(("b_profile_calibrate: %#lx capturing sample ...", (unsigned long)table));
	b_sample(&sample0);
	for(i=nloops;i>0;i--) {
		dummy(0, b_profile_dummy_0);
	}
	b_sample(&sample1);
	BTRC_TRACE(bprofile_nosample, STOP);
    BDBG_MSG(("b_profile_calibrate: %#lx calibrating ...", (unsigned long)table));
	b_profile_sample_diff(&sample1, &sample0);
	dummy = b_profile_dummy_2;
	BTRC_TRACE(bprofile_sample, START);
	b_sample(&sample0);
	for(i=nloops;i>0;i--) {
		dummy(0, b_profile_dummy_0);
	}
	b_sample(&sample2);
	BTRC_TRACE(bprofile_sample, STOP);
	b_profile_sample_diff(&sample2, &sample0);
	if (sample0.time >= sample1.time) {
		sample0.time = sample2.time - sample1.time;
	} else {
		sample0.time = 0;
	}
	time_100us = b_tick2_100us(sample0.time);
		
	BDBG_LOG(("profiler overhead %u.%02u us per function call (%u ticks)", time_100us/100, time_100us%100, sample0.time/nloops));
#if BPROFILE_CFG_PERF_COUNTER
	{
		const bperf_counter_mode *mode = bperf_get_mode();

		for(i=0;mode && i<BPROFILE_CFG_PERF_COUNTER;i++) {
			if(sample2.counters[i] >= sample1.counters[i]) {
				sample0.counters[i] = sample2.counters[i] - sample1.counters[i];
			} else {
				sample0.counters[i] = 0;
			}
			b_profile_state.info.overhead.counters[i] = sample0.counters[i]/nloops;
			BDBG_LOG(("profiler %s overhead %u per function call", mode->counter_names[i], b_profile_state.info.overhead.counters[i]));
		}
	}
#endif

	b_profile_state.info.overhead.time = sample2.time/nloops;
	return b_profile_state.info.overhead.time;
}

unsigned 
bprofile_calibrate(bprofile_entry *table, size_t nelem)
{
    unsigned result;
    b__profile_start(table, nelem);
    result = b_profile_calibrate(table);
    bprofile_stop();
    return result;
}

void 
bprofile_get_info(bprofile_probe_info *info)
{
	*info = b_profile_state.info;
	return;
}

void DONT_PROFILE
bprofile_start(bprofile_entry *table, size_t nelem) 
{
	b__profile_start(table, nelem);
	return;
}


BSTD_INLINE void DONT_PROFILE
b_addentry(unsigned type, void *func)
{
	unsigned long event;
#if BPROFILE_CFG_SINGLE_THREAD
	bprofile_entry  *entry = (bprofile_entry *)(batomic_get(&b_profile_state.next)+sizeof(*entry));
	if ( (unsigned long)entry < (unsigned long)batomic_get(&b_profile_state.last)) {
		event = (unsigned long)func;
#if 0
		event &= &(~B_PROFILE_EVENT_MASK); /* function address has to be 32 bit alligned */
#endif
		event |= type;
		entry->addr = event;
		batomic_set(&b_profile_state.next, (long)entry);
		b_sample(&entry->sample);
	}
#else
	if ( (unsigned)batomic_get(&b_profile_state.next) < (unsigned)batomic_get(&b_profile_state.last)) {
		bprofile_entry  *entry = (bprofile_entry *)batomic_add_return(&b_profile_state.next,sizeof(*entry));
		b_get_stack(&event);
#if 0
		event &= (~B_PROFILE_EVENT_MASK); /* stack pointer has to be 32 bit alligned */
#endif
		event= event | type;
		entry->event_0 = event; 
		entry->addr = (unsigned long)func;
		b_sample(&entry->sample); 
	}
#endif
	return;
}


void DONT_PROFILE
__cyg_profile_func_enter (void *func,  void *caller) 
{
	BSTD_UNUSED(caller);
	b_addentry(B_PROFILE_EVENT_ENTER, func);
}

void DONT_PROFILE
__cyg_profile_func_exit (void *func, void *caller) 
{
	BSTD_UNUSED(caller);
	b_addentry(B_PROFILE_EVENT_EXIT, func);
}

