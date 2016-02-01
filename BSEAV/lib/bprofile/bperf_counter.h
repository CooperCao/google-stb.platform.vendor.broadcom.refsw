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
 * Perfomance counter module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BPERF_COUNTER_H__
#define __BPERF_COUNTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if BCHP_CHIP==7403 || BCHP_CHIP==7401 || BCHP_CHIP==3560 || BCHP_CHIP==3563 || BCHP_CHIP==3543 || BCHP_CHIP==7002 || BCHP_CHIP==7208 || BCHP_CHIP==7356 || BCHP_CHIP==7468 || BCHP_CHIP==7550 || BCHP_CHIP==7552
#define B_PERF_BMIPS3300 1
#elif BCHP_CHIP==7400 || BCHP_CHIP==7405 || BCHP_CHIP==7335 || BCHP_CHIP==3548 || BCHP_CHIP==3556 || BCHP_CHIP==7408 || BCHP_CHIP==35230 || BCHP_CHIP==7106 || BCHP_CHIP==7125 || BCHP_CHIP==7231 || BCHP_CHIP==7335 || BCHP_CHIP==7336 || BCHP_CHIP==7340 || BCHP_CHIP==7342 || BCHP_CHIP==7413
#define B_PERF_BMIPS4380 1
#elif BCHP_CHIP==7038
#define B_PERF_MIPSR5K	1
#elif BCHP_CHIP==7325
#define B_PERF_MIPS34K	1
#elif BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7420 || BCHP_CHIP==7422 || BCHP_CHIP==7425 || BCHP_CHIP==7428 || BCHP_CHIP==7435
#define B_PERF_BMIPS5000   1 
#else
#define B_PERF_NOT_SUPPORTED 1
#endif

#if (defined(LINUX) || defined(__linux__)) && !defined(__KERNEL__)
#define B_PERF_LINUX    1
#undef B_PERF_MIPSR5K
#undef B_PERF_BMIPS3300
#undef B_PERF_MIPS34K
#endif


#if B_PERF_BMIPS3300
#define BPERF_N_COUNTERS	4
#define BPERF_SAMPLE_INITIALIZER {{0,0,0,0}}


#define b_perf_read_one(sel)	__extension__			\
({ unsigned int b_perf_read_res;						\
		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mfc0\t%0, $25, " #sel "\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (b_perf_read_res));					\
	b_perf_read_res;									\
})

BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return start-stop; /* performance timers are count down */
}

#elif B_PERF_BMIPS4380
#define BPERF_N_COUNTERS	4
#define BPERF_SAMPLE_INITIALIZER {{0,0,0,0}}

#define B_PERF_BASE_ADDR    0x11F20000ul
#define B_PERF_BASE_LEN     256

#ifndef b_perf_4380_base
/* MIPS kernel mode mapping */
#define b_perf_4380_base  ((volatile uint32_t *)(B_PERF_BASE_ADDR|0xA0000000ul))
#endif
#define b_perf_read_one(sel)	b_perf_4380_base[4+sel]

BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return start-stop; /* performance timers are count down */
}

#elif defined(B_PERF_MIPSR5K) || defined(B_PERF_MIPS34K)
#define BPERF_N_COUNTERS	2
#define BPERF_SAMPLE_INITIALIZER {{0,0}}
#define b_perf_read_one(sel)	__extension__			\
({ unsigned int b_perf_read_res;						\
		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mfc0\t%0, $25, " #sel "\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (b_perf_read_res));					\
	b_perf_read_res;									\
})

BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return stop-start;
}
#elif defined(B_PERF_BMIPS5000)
#define BPERF_N_COUNTERS	5
#define BPERF_SAMPLE_INITIALIZER {{0,0,0,0,0}}

#if (defined(LINUX) || defined(__linux__)) && !defined(__KERNEL__)
#define b_perf_write_cfg b_perf_write_cfg
extern void b_perf_write_cfg(unsigned select, unsigned data);
/* using RDHWR for the Linux user mode */
#define b_perf_read_one_rdhwr(sel)	__extension__			\
({ unsigned int b_perf_read_res;						\
		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32r2\n\t"							\
			"rdhwr \t%0, $" #sel "\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (b_perf_read_res));					\
	b_perf_read_res;									\
})
#define b_perf_read_one(sel) ((sel==0)?b_perf_read_one_rdhwr(24):((sel==1)?b_perf_read_one_rdhwr(25):((sel==2)?b_perf_read_one_rdhwr(26):((sel==3)?b_perf_read_one_rdhwr(27):b_perf_read_one_rdhwr(2)))))
#else 
/* otherwise use Coprocessor 0 instructions */
#define b_perf_read_one_cp0(sel)	__extension__			\
({ unsigned int b_perf_read_res;						\
		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mfc0\t%0, $25, " #sel "\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (b_perf_read_res));					\
	b_perf_read_res;									\
})

#define b_perf_read_tick()	__extension__			\
({ unsigned int b_perf_read_res;						\
		__asm__ __volatile__(".set\tpush\n\t"			\
			".set\tmips32\n\t"							\
			"mfc0\t%0, $9, 0\n\t"			\
			".set\tpop\n\t"							\
			: "=r" (b_perf_read_res));					\
	b_perf_read_res;									\
})

#define b_perf_read_one(sel) ((sel==0)?b_perf_read_one_cp0(1):((sel==1)?b_perf_read_one_cp0(3):((sel==2)?b_perf_read_one_cp0(5):((sel==3)?b_perf_read_one_cp0(7):b_perf_read_tick()))))
#endif

BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return ((stop<<1)-(start<<1))>>1; /* HW doesn't update most significant bit */
}

#elif B_PERF_LINUX
#define BPERF_N_COUNTERS	1
#define BPERF_SAMPLE_INITIALIZER {{0}}

BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return stop-start;
}
extern unsigned b_perf_read_time(void);
#define b_perf_read_one(sel)	b_perf_read_time()

#else /* BPEFF_ */
BSTD_INLINE unsigned bperf_sample_diff(unsigned stop, unsigned start)
{
	return stop-start;
}

#define BPERF_N_COUNTERS	1
#define BPERF_SAMPLE_INITIALIZER {{0}}
#endif /* BPERF_ */

typedef struct bperf_counter_mode {
	const char *counter_names[BPERF_N_COUNTERS];
	unsigned config[4]; /* opaque HW configuration */
} bperf_counter_mode;

typedef struct bperf_sample {
	unsigned data[BPERF_N_COUNTERS];
} bperf_sample;


extern const  bperf_counter_mode bperf_counter_dcache;
extern const  bperf_counter_mode bperf_counter_icache;
extern const  bperf_counter_mode bperf_counter_instructions;
#if  B_PERF_BMIPS3300 || B_PERF_BMIPS4380
#define BPERF_COUNTER_HAS_RAC 1
extern const  bperf_counter_mode bperf_counter_rac_access;
extern const  bperf_counter_mode bperf_counter_rac_prefetch;
extern const  bperf_counter_mode bperf_counter_rac_hit;
#endif
#if B_PERF_BMIPS5000
#define bperf_counter_idle bperf_counter_idle
extern const  bperf_counter_mode bperf_counter_idle;
#define bperf_counter_memory bperf_counter_memory
extern const  bperf_counter_mode bperf_counter_memory;
#define bperf_counter_memory_cycles bperf_counter_memory_cycles
extern const  bperf_counter_mode bperf_counter_memory_cycles;
#endif

int b_perf_init(const bperf_counter_mode *mode);


const bperf_counter_mode *bperf_get_mode(void);

void bperf_print(const bperf_counter_mode *mode, const bperf_sample *stop, const bperf_sample *start);

BSTD_INLINE void __attribute__((no_instrument_function))
b_perf_read(bperf_sample *sample)
{
#if  BPERF_N_COUNTERS >= 4
    unsigned data0, data1, data2, data3;
#if  BPERF_N_COUNTERS >= 5
    unsigned data4;
#endif
    data0 = b_perf_read_one(0);
	data1 = b_perf_read_one(1);
	data2 = b_perf_read_one(2);
	data3 = b_perf_read_one(3);
#if  BPERF_N_COUNTERS >= 5
	data4 = b_perf_read_one(4);
#endif
	sample->data[0] = data0;
	sample->data[1] = data1;
	sample->data[2] = data2;
	sample->data[3] = data3;
#if  BPERF_N_COUNTERS >= 5
	sample->data[4] = data4;
#endif
#elif defined(B_PERF_MIPSR5K) || defined(B_PERF_MIPS34K)
    unsigned data0, data1;
    data0 = b_perf_read_one(1);
	data1 = b_perf_read_one(3);
	sample->data[0] = data0;
	sample->data[1] = data1;
#elif B_PERF_LINUX
	sample->data[0] = b_perf_read_one();
#else
	BSTD_UNUSED(sample);
#endif
	return;
}

#ifndef bperf_read
#define bperf_read b_perf_read
#endif

#ifndef bperf_init
#define bperf_init b_perf_init
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BPERF_COUNTER_H__ */

