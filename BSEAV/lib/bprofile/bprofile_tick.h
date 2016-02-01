/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
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
 * 		Data acquisition module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bperf_counter.h"
#if (defined(LINUX) || defined(__linux__)) && !defined(__KERNEL__)
#define b_get_stack(stack) do{unsigned sp; *(stack)=(unsigned long)&sp;} while(0)

#if B_PERF_BMIPS4380 || B_PERF_BMIPS5000
/* perfomance counters are memory mapped and could be accessed from the user space */

/* in linux user space we can't access cycle counter from C0 */
/* so use 4-th performance counter, and it shall be mapped to the MIPS cycle counter */
#if B_PERF_BMIPS4380 
#define b_gettick() (~(b_perf_read_one(3)))
#elif B_PERF_BMIPS5000
#define b_gettick() b_perf_read_one(4)
#else
#error "Not supported"
#endif
#include "bprofile_tick_mips.h"
/* this variable shall be initialized to number of ticks in 100us */ 
extern unsigned  b_ticks_100us;
#define b_tick2ms_init() unsigned clock__=b_ticks_100us
#define b_tick2_100us(tick) ((tick)/(clock__))

#else /* B_PERF_BMIPS4380 */
#include <sys/time.h>
BSTD_INLINE unsigned __attribute__((no_instrument_function))
b_gettick(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec+((unsigned)tv.tv_sec)*1000000;
}

BSTD_INLINE void __attribute__((no_instrument_function))
b_sample(bprofile_sample *sample)
{
	sample->time = b_gettick();
	return ;
}
#define b_tick2ms_init()
#define b_tick2_100us(tick) ((tick)/100)

#endif /* B_PERF_BMIPS4380  */


#elif defined(CONFIG_UCOS)
#include "bprofile_tick_mips.h"
#define b_get_stack(stack) do{unsigned sp; *(stack)=(unsigned long)&sp;} while(0)

#define b_tick2ms_init() unsigned clock__=(g_running_clock.clock_freq/10000);
#define b_tick2_100us(tick) ((tick)/(clock__))
#elif defined(LINUX) && defined(__KERNEL__) 
#include "bperf_counter.h"
#if B_PERF_BMIPS3300
/* linux kernel adjusts MIPS C0 counter, therefore it's not monotonic counter and not good source for interval measurements */
/* for BCM MIPS300 we could use 4-th performance counter, and it shall be mapped to the MIPS cycle counter */
/*#define b_gettick(void) (~(b_perf_read_one(3))) */
#endif
#include "bprofile_tick_mips.h"
#include "batomic.h"
/* in the Linux there is no dedicated task for the interrupt handler, therefore special trick is required to mark the interrupt context */
extern batomic_t b_in_isr; /* this is mask, where the most significant bit is cleared/set in the entry/exit point of the ISR routine */
BSTD_INLINE void __attribute__((no_instrument_function))
b_get_stack(unsigned long *stack) 
{
	unsigned sp; 
	*(stack)=((unsigned long)batomic_get(&b_in_isr))&(unsigned long)&sp;
}

/* this variable shall be initialized to number of ticks in 100us */ 
extern unsigned  b_ticks_100us;
#define b_tick2ms_init() unsigned clock__=b_ticks_100us
#define b_tick2_100us(tick) ((tick)/(clock__))
#else
#warning "Not supported"
#define b_get_stack(stack) do{unsigned sp; *(stack)=(unsigned long)&sp;} while(0)
#define b_sample(s) (void)
#define b_tick2ms_init() (void)
#define b_tick2_100us(tick) (tick)
#endif

#define b_tick2ms(n) (b_tick2_100us(n)/10)


