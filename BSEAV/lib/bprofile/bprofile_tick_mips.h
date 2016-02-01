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
 * 		MIPS Data acquisition module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
/* MIPS32 performance sampling routine */

#ifndef b_gettick
#if defined(__mips__)
BSTD_INLINE unsigned __attribute__((no_instrument_function))
b_gettick(void)
{ 
	unsigned b_mips_tick;
	__asm__ __volatile__(
			".set\tpush\n\t"
			".set\tmips32\n\t"
			"mfc0\t%0, $9, 0\n\t"
			".set\tpop\n\t"
			: "=r" (b_mips_tick));
	return b_mips_tick;				
}
#else
#error Not supported
#endif
#endif

BSTD_INLINE void __attribute__((no_instrument_function))
b_sample(bprofile_sample *sample)
{
        unsigned time;
#if BPROFILE_CFG_PERF_COUNTER
        unsigned data[BPROFILE_CFG_PERF_COUNTER];
#endif
        time = b_gettick();

#if BPROFILE_CFG_PERF_COUNTER
#if BPROFILE_CFG_PERF_COUNTER <= BPERF_N_COUNTERS
		data[0] = b_perf_read_one(0);
#if BPROFILE_CFG_PERF_COUNTER > 1
		data[1] = b_perf_read_one(1);
#endif
#if BPROFILE_CFG_PERF_COUNTER > 2
		data[2] = b_perf_read_one(2);
#endif
#if BPROFILE_CFG_PERF_COUNTER > 3
		data[3] = b_perf_read_one(3);
#endif
#else /* BPERF_N_COUNTERS <= BPROFILE_CFG_PERF_COUNTER */
#error "Not supported"
#endif
#endif /* BPROFILE_CFG_PERF_COUNTER */
		sample->time = time;
#if BPROFILE_CFG_PERF_COUNTER
		sample->counters[0] = data[0];
# if BPROFILE_CFG_PERF_COUNTER > 1
		sample->counters[1] = data[1];
# endif
# if BPROFILE_CFG_PERF_COUNTER > 2
		sample->counters[2] = data[2];
# endif
# if BPROFILE_CFG_PERF_COUNTER > 2
		sample->counters[3] = data[3];
# endif
#endif
		return;
}

