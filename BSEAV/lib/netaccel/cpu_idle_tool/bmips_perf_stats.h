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
 * 
 *************************************************************/ 
#ifndef __BMIPS_PERF_STATS_H__
#define __BMIPS_PERF_STATS_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmips_perf_counters {
    unsigned int num_executed_cycles;
    unsigned int num_retired_instr;
    unsigned int num_cpu_stall_cycles;
    unsigned int num_cpu_kernel_cycles;
    unsigned int num_cpu_user_cycles;
    unsigned int num_exceptions_interrupts;
    unsigned int num_icache_ref;
    unsigned int num_icache_miss;
    unsigned int num_dcache_ref;
    unsigned int num_dcache_miss;
    unsigned int num_dcache_load_miss;
    unsigned int num_dcache_load;
    unsigned int num_dcache_store_miss;
    unsigned int num_dcache_store;
    unsigned int num_retired_load_instr;
    unsigned int num_retired_store_instr;
    unsigned int num_exceptions;
    unsigned int num_l2_cache_ref;
    unsigned int num_l2_cache_miss;
    unsigned int num_l2_dirty_evictions;
    unsigned int num_rac_access;
    unsigned int num_rac_prefetch;
    unsigned int num_rac_hits;
    unsigned int off_usec;
} bmips_perf_counters;

#if defined(B_PERF_ARM_V7)

#elif defined(B_PERF_BMIPS_5000)
#define PERF_EVENT_BIT_SHIFT                         5
#define PERF_COUNTER_NUM_EXECUTED_CYCLES             0      //can be in counter 0/1/2/3
#define PERF_COUNTER_NUM_RETIRED_INSTR               1      //can be in counter 0/1/2/3
#define PERF_COUNTER_NUM_ICACHE_REF                  10     //can be in counter 0/2
#define PERF_COUNTER_NUM_ICACHE_MISS                 10     //can be in counter 1/3
#define PERF_COUNTER_NUM_DCACHE_REF                  12     //can be in counter 0/2
#define PERF_COUNTER_NUM_DCACHE_MISS                 12     //can be in counter 1/3
#define PERF_COUNTER_NUM_DCACHE_LOAD_MISS            14     //can be in counter 0/2
#define PERF_COUNTER_NUM_DCACHE_LOAD                 14     //can be in counter 1/3
#define PERF_COUNTER_NUM_DCACHE_STORE_MISS           15     //can be in counter 0/2
#define PERF_COUNTER_NUM_DCACHE_STORE                15     //can be in counter 1/3
#define PERF_COUNTER_NUM_RETIRED_LOAD_INSTR          21     //can be in counter 0/2
#define PERF_COUNTER_NUM_RETIRED_STORE_INSTR         22     //can be in counter 0/2
#define PERF_COUNTER_NUM_EXCEPTIONS                  26     //can be in counter 0/2
#define PERF_COUNTER_NUM_L2_CACHE_REF                28     //can be in counter 0/2
#define PERF_COUNTER_NUM_L2_CACHE_MISS               28     //can be in counter 1/3
#define PERF_COUNTER_NUM_L2_DIRTY_EVICTIONS          29     //can be in counter 0/2

//count for TC defined in bit 12(TC0) or 13 (TC1): bit 31
//enable count for TC0: bit 12
//enable count during exceptions: bit 0
//enable counting in kernel mode: bit 1
#define PERF_COUNTER_CONTROL_BASE \
        (1<<30) | \
        (1<<12) | \
        (1<<1)

#define bcm_write_perf_control(sel, value)  \
{   __asm__ __volatile__(".set  push\n\t"           \
                         ".set  mips32\n\t"         \
                         ".set  noreorder\n"      \
                         "mtc0  %0, $25," #sel "\n\t" \
                         ".set  pop\n\t"            \
                         : /* none */               \
                         : "r" ((unsigned int)value));  \
}

#define bcm_read_perf(sel, value)  \
{  \
    __asm__ __volatile__(".set  push\n\t"           \
                         ".set  mips32\n\t"         \
                         ".set  noreorder\n"      \
                         "mfc0  %0, $25," #sel "\n\t" \
                         ".set  pop\n\t"            \
                         : "=r" ((unsigned int)value));  \
}
#elif defined(B_PERF_BMIPS_4380)
#define BPERF_N_COUNTERS    4
#define BPERF_SAMPLE_INITIALIZER {{0,0,0,0}}

#define B_PERF_BASE_ADDR    0x11F20000ul
#define B_PERF_BASE_LEN     256

#ifndef b_perf_4380_base
/* MIPS kernel mode mapping */
#define b_perf_4380_base  ((volatile uint32_t *)(B_PERF_BASE_ADDR|0xA0000000ul))
#endif
#define b_perf_read_one(sel)    b_perf_4380_base[4+sel]

#define B_CORE_MOD_ID   0x02
#define B_CORE_CPU_ID   0x01
#define B_CORE_OS_ID    0x02
#define B_CPU_STALLS    0x01
#define B_CPU_EXCEPTIONS    0x0A
#define B_KERNEL_CYCLES 0x03
#define B_USER_CYCLES   0x01

#define B_L2_MOD_ID 0x0d
#define B_L2_ID     0x00
#define B_L2_HIT    0x01
#define B_L2_MISS   0x00

#define B_ICACHE_MOD_ID 0x06
#define B_ICACHE_ID     0x00
#define B_ICACHE_HIT    0x06
#define B_ICACHE_MISS   0x05

#define B_DCACHE_MOD_ID 0x04
#define B_DCACHE_ID     0x01
#define B_DCACHE_HIT    0x0A
#define B_DCACHE_MISS   0x09

#define B_RAC_MOD_ID    0x0B
#define B_RAC_ACCESS_ID 0x01
#define B_RAC_ACCESS    0x43
#define B_RAC_HIT_ID    0x01
#define B_RAC_HIT       0x40
#define B_DRAC_HIT      0x23

#define B_RAC_PREFETCH_ID 0x00
#define B_RAC_PREFETCH  0x47

#define B_BIU_MOD_ID    0x01

#define B_CYCLES        0x12
#define B_INSTRUCTIONS  0x11

#else
/* Define other BIMPS platforms here */
#error
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BMIPS_PERF_STATS_H__ */

