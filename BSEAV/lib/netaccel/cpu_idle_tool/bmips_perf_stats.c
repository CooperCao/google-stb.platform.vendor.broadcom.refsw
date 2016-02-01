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
 * 
 *************************************************************/ 
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <generated/autoconf.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/vmalloc.h>

#include "bmips_perf_stats.h"

#ifdef BMIPS_PERFC_ENABLE
/* 
 *  By default, system wide performance stats are captured. This should provide good high level view of the system.
 *  For monitoring specific parts of kernel or kernel drivers, you will need to instrument the kernel/driver 
 *  Please follow the instructions in the README file to do so
*/
extern int g_perfc_type; //type of performance counter
extern int g_cpu_idle_stats; //prints stats every nsec
#ifdef PERFC_INSTRUMENT_KERNEL
extern void (*bperfc_hook)(int, unsigned int);
extern void bperfc_set_hook(void (*bperfc_hook)(int, unsigned int));
#endif

#define MAX_PERFC_COUNTERS 40000 /* For 100Mbps, need ~10K counters/sec */
bmips_perf_counters *perfc_counters;
bmips_perf_counters perfc_end;
bmips_perf_counters perfc_begin;
int begin_idx = 0;
int end_idx = 0;

void get_perfc_delta(bmips_perf_counters *perfc_begin, bmips_perf_counters *perfc_end, bmips_perf_counters *perfc_counter)
{
    int interval;
#ifndef PERFC_INSTRUMENT_KERNEL
    /* if we are not measuring a kernel or driver specific stats, then normalize the counters for second interval */
    interval = g_cpu_idle_stats;
#else
    interval = 1;
#endif
    perfc_counter->num_retired_instr = (perfc_end->num_retired_instr - perfc_begin->num_retired_instr)/interval;
	perfc_counter->num_executed_cycles = (perfc_end->num_executed_cycles - perfc_begin->num_executed_cycles)/interval;
	perfc_counter->num_icache_ref = (perfc_end->num_icache_ref - perfc_begin->num_icache_ref)/interval;
	perfc_counter->num_icache_miss = (perfc_end->num_icache_miss - perfc_begin->num_icache_miss)/interval;
	perfc_counter->num_dcache_ref = (perfc_end->num_dcache_ref - perfc_begin->num_dcache_ref)/interval;
	perfc_counter->num_dcache_miss = (perfc_end->num_dcache_miss - perfc_begin->num_dcache_miss)/interval;
	perfc_counter->num_dcache_load_miss = (perfc_end->num_dcache_load_miss - perfc_begin->num_dcache_load_miss)/interval;
	perfc_counter->num_dcache_load = (perfc_end->num_dcache_load - perfc_begin->num_dcache_load)/interval;
	perfc_counter->num_dcache_store_miss = (perfc_end->num_dcache_store_miss - perfc_begin->num_dcache_store_miss)/interval;
	perfc_counter->num_dcache_store = (perfc_end->num_dcache_store- perfc_begin->num_dcache_store)/interval;
	perfc_counter->num_retired_load_instr = (perfc_end->num_retired_load_instr- perfc_begin->num_retired_load_instr)/interval;
	perfc_counter->num_retired_store_instr = (perfc_end->num_retired_store_instr- perfc_begin->num_retired_store_instr)/interval;
	perfc_counter->num_exceptions = (perfc_end->num_exceptions - perfc_begin->num_exceptions)/interval;
	perfc_counter->num_l2_cache_ref = (perfc_end->num_l2_cache_ref - perfc_begin->num_l2_cache_ref)/interval;
	perfc_counter->num_l2_cache_miss = (perfc_end->num_l2_cache_miss - perfc_begin->num_l2_cache_miss)/interval;
	perfc_counter->num_l2_dirty_evictions = (perfc_end->num_l2_dirty_evictions  - perfc_begin->num_l2_dirty_evictions)/interval;
	perfc_counter->num_rac_access = (perfc_end->num_rac_access  - perfc_begin->num_rac_access)/interval;
	perfc_counter->num_rac_hits = (perfc_end->num_rac_hits  - perfc_begin->num_rac_hits)/interval;
	perfc_counter->num_rac_prefetch = (perfc_end->num_rac_prefetch  - perfc_begin->num_rac_prefetch)/interval;
	perfc_counter->num_cpu_kernel_cycles = (perfc_end->num_cpu_kernel_cycles  - perfc_begin->num_cpu_kernel_cycles)/interval;
	perfc_counter->num_cpu_user_cycles = (perfc_end->num_cpu_user_cycles  - perfc_begin->num_cpu_user_cycles)/interval;
	perfc_counter->num_cpu_stall_cycles = (perfc_end->num_cpu_stall_cycles  - perfc_begin->num_cpu_stall_cycles)/interval;
}

void print_perfc_stats(bmips_perf_counters *avg, bmips_perf_counters *min, bmips_perf_counters *max)
{
    int interval;
#ifndef PERFC_INSTRUMENT_KERNEL
    /* if we are not measuring a kernel or driver specific stats, then counters have already been normalized in the get_perfc_delta() */
    interval = 1;
#else
    interval = g_cpu_idle_stats;
#endif
    printk("Performace Counters\n");
    if (avg->num_retired_instr !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of Executed Instructions\n", 
                avg->num_retired_instr/interval, min->num_retired_instr, avg->num_retired_instr/(end_idx), max->num_retired_instr);
    if (avg->num_executed_cycles !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of Executed Cycles\n", 
                avg->num_executed_cycles/(interval), min->num_executed_cycles, avg->num_executed_cycles/end_idx, max->num_executed_cycles);
    if (avg->num_icache_ref !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of I-cache references\n", 
                avg->num_icache_ref/(interval), min->num_icache_ref, avg->num_icache_ref/(end_idx), max->num_icache_ref);
    if (avg->num_icache_miss !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of I-cache misses\n", 
                avg->num_icache_miss/(interval), min->num_icache_miss, avg->num_icache_miss/(end_idx), max->num_icache_miss);
    if (avg->num_dcache_ref !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache references\n", 
                avg->num_dcache_ref/(interval), min->num_dcache_ref, avg->num_dcache_ref/(end_idx), max->num_dcache_ref);
    if (avg->num_dcache_miss !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache misses \n", 
                avg->num_dcache_miss/(interval), min->num_dcache_miss, avg->num_dcache_miss/(end_idx), max->num_dcache_miss);
    if (avg->num_dcache_load_miss !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache load misses \n", 
                avg->num_dcache_load_miss/(interval), min->num_dcache_load_miss, avg->num_dcache_load_miss/(end_idx), max->num_dcache_load_miss);
    if (avg->num_dcache_load!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache load\n", 
                avg->num_dcache_load/(interval), min->num_dcache_load, avg->num_dcache_load/(end_idx), max->num_dcache_load);
    if (avg->num_dcache_store_miss !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache store misses \n", 
                avg->num_dcache_store_miss/(interval), min->num_dcache_store_miss, avg->num_dcache_store_miss/(end_idx), max->num_dcache_store_miss);
    if (avg->num_dcache_store!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of D-cache store\n", 
                avg->num_dcache_store/(interval), min->num_dcache_store, avg->num_dcache_store/(end_idx), max->num_dcache_store);
    if (avg->num_retired_load_instr!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of retired Load Instructions\n", 
                avg->num_retired_load_instr/(interval), min->num_retired_load_instr, avg->num_retired_load_instr/(end_idx), max->num_retired_load_instr);
    if (avg->num_retired_store_instr!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of retired Store Instructions\n", 
                avg->num_retired_store_instr/(interval), min->num_retired_store_instr, avg->num_retired_store_instr/(end_idx), max->num_retired_store_instr);
    if (avg->num_exceptions!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # of Exceptions \n", 
                avg->num_exceptions/(interval), min->num_exceptions, avg->num_exceptions/(end_idx), max->num_exceptions);
    if (avg->num_l2_cache_ref!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # L2 Cache References \n", 
                avg->num_l2_cache_ref/(interval), min->num_l2_cache_ref, avg->num_l2_cache_ref/(end_idx), max->num_l2_cache_ref);
    if (avg->num_l2_cache_miss!=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # L2 Cache Misses \n", 
                avg->num_l2_cache_miss/(interval), min->num_l2_cache_miss, avg->num_l2_cache_miss/(end_idx), max->num_l2_cache_miss);
    if (avg->num_l2_dirty_evictions !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # L2 Dirty Evictions \n", 
                avg->num_l2_dirty_evictions/(interval), min->num_l2_dirty_evictions, avg->num_l2_dirty_evictions/(end_idx), max->num_l2_dirty_evictions);
    if (avg->num_rac_access !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # RAC Access\n", 
                avg->num_rac_access/(interval), min->num_rac_access, avg->num_rac_access/(end_idx), max->num_rac_access);
    if (avg->num_rac_hits !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # RAC Hits\n", 
                avg->num_rac_hits/(interval), min->num_rac_hits, avg->num_rac_hits/(end_idx), max->num_rac_hits);
    if (avg->num_rac_prefetch !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # RAC Prefetch\n", 
                avg->num_rac_prefetch/(interval), min->num_rac_prefetch, avg->num_rac_prefetch/(end_idx), max->num_rac_prefetch);
    if (avg->num_cpu_kernel_cycles !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # Kernel Cycles\n", 
                avg->num_cpu_kernel_cycles/(interval), min->num_cpu_kernel_cycles, avg->num_cpu_kernel_cycles/(end_idx), max->num_cpu_kernel_cycles);
    if (avg->num_cpu_user_cycles !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # User Cycles Hits\n", 
                avg->num_cpu_user_cycles/(interval), min->num_cpu_user_cycles, avg->num_cpu_user_cycles/(end_idx), max->num_cpu_user_cycles);
    if (avg->num_cpu_stall_cycles !=0)
        printk("%10d, %10d, %10d, %10d (tot,min,avg,max) # CPU Stalls\n", 
                avg->num_cpu_stall_cycles/(interval), min->num_cpu_stall_cycles, avg->num_cpu_stall_cycles/(end_idx), max->num_cpu_stall_cycles);
}

#if defined(B_PERF_BMIPS_5000)
/* setup the performance counter measurements */
void setup_perfc_measurements(int type)
{
    printk("**************************\n");
    printk("Setting up Performance Counter Measurements of type %d for BMIPS%s Family\n", type, "5000");
    printk("**************************\n");
    switch (type) {
    case 1:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_ICACHE_REF<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_ICACHE_MISS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 2:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_REF<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_MISS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 3:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_LOAD_MISS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_LOAD<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 4:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_STORE_MISS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_DCACHE_STORE<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 5:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_LOAD_INSTR<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_STORE_INSTR<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 6:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_L2_CACHE_REF<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(2, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_L2_CACHE_MISS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 7:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_L2_DIRTY_EVICTIONS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    case 8:
	    bcm_write_perf_control(0, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXCEPTIONS<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(4, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_EXECUTED_CYCLES<< PERF_EVENT_BIT_SHIFT)));
	    bcm_write_perf_control(6, (PERF_COUNTER_CONTROL_BASE | (PERF_COUNTER_NUM_RETIRED_INSTR<< PERF_EVENT_BIT_SHIFT)));
        break;
    default:
        printk("Invalid type, use 1 to 8 \n");
        break;
    }
}
	
static void get_perfc_snapshot(int type, bmips_perf_counters *perfc_ptr)
{
    switch (type) {
    case 1:
	    bcm_read_perf(1, perfc_ptr->num_icache_ref);
	    bcm_read_perf(3, perfc_ptr->num_icache_miss);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 2:
	    bcm_read_perf(1, perfc_ptr->num_dcache_ref);
	    bcm_read_perf(3, perfc_ptr->num_dcache_miss);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 3:
	    bcm_read_perf(1, perfc_ptr->num_dcache_load_miss);
	    bcm_read_perf(3, perfc_ptr->num_dcache_load);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 4:
	    bcm_read_perf(1, perfc_ptr->num_dcache_store_miss);
	    bcm_read_perf(3, perfc_ptr->num_dcache_store);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 5:
	    bcm_read_perf(1, perfc_ptr->num_retired_load_instr);
	    bcm_read_perf(3, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(5, perfc_ptr->num_retired_store_instr);   /* counter needs to be in 0/2 control */
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 6:
	    bcm_read_perf(1, perfc_ptr->num_l2_cache_ref);
	    bcm_read_perf(3, perfc_ptr->num_l2_cache_miss);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 7:
	    bcm_read_perf(1, perfc_ptr->num_l2_dirty_evictions);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    case 8:
	    bcm_read_perf(1, perfc_ptr->num_exceptions);
	    bcm_read_perf(5, perfc_ptr->num_executed_cycles);
	    bcm_read_perf(7, perfc_ptr->num_retired_instr);
        break;
    default:
        break;
    }
}
	
#elif defined(B_PERF_BMIPS_4380)
/* setup the performance counter measurements */
void setup_perfc_measurements(int type)
{
    printk("**************************\n");
    printk("Setting up Performance Counter Measurements of type %d for BMIPS%s Family\n", type, "4380");
    printk("**************************\n");
    switch (type) {
    case 1:
        /* i-cache bit & miss */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_ICACHE_MOD_ID<<2) | /* Set ModID */
            (B_ICACHE_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_ICACHE_MISS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_ICACHE_HIT<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 2:
        /* d-cache bit & miss */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_DCACHE_MOD_ID<<2) | /* Set ModID */
            (B_DCACHE_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_DCACHE_MISS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_DCACHE_HIT<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 3:
        /* L2 stats */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_L2_MOD_ID<<2) | /* Set ModID */
            (B_L2_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_L2_MISS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_L2_HIT<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 4:
        /* rac access */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_RAC_MOD_ID<<2) | /* Set ModID */
            (B_RAC_ACCESS_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_RAC_ACCESS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_RAC_HIT<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 5:
        /* rac prefetch */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_RAC_MOD_ID<<2) | /* Set ModID */
            (B_RAC_PREFETCH_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_RAC_PREFETCH<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 6:
        /* CORE CPU stats */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_CORE_MOD_ID<<2) | /* Set ModID */
            (B_CORE_CPU_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_CPU_STALLS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CPU_EXCEPTIONS<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    case 7:
        /* OS related stats */
        b_perf_4380_base[0] = /* global control register */
            (1<<31) |/* Enable PCE*/ 
            (0<<8) | /* Enable PCSD */
            (B_CORE_MOD_ID<<2) | /* Set ModID */
            (B_CORE_OS_ID<<0);
        b_perf_4380_base[1] = /* control register 0 for counter 0 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_KERNEL_CYCLES<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[1] |= /* control register 0 for counter 1 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_USER_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] = /* control register 1 for counter 2 */
            (1<<15) |/* Enable Counter */ 
            (0<<13) |/* TPID: cpu 0 */
            (B_INSTRUCTIONS<<2) | /* Event */
            (0<<0); /* Disable Interrupts */
        b_perf_4380_base[2] |= /* control register 1 for counter 3 */
            (1<<31) |/* Enable Counter */ 
            (0<<29) |/* TPID: cpu 0 */
            (B_CYCLES<<18) | /* Event */
            (0<<0); /* Disable Interrupts */
        break;
    default:
        printk("Invalid type, try 0 to 5 \n");
        break;
    }
    printk("control reg: global %x, ctl0 | ctl 1 %x, ctl2 | ctl3 %x\n", 
        b_perf_4380_base[0],
        b_perf_4380_base[1],
        b_perf_4380_base[2]
        );
    b_perf_read_one(0) = 0;
    b_perf_read_one(1) = 0;
    b_perf_read_one(2) = 0;
    b_perf_read_one(3) = 0;
}
	
void get_perfc_snapshot(int type, bmips_perf_counters *perfc_ptr)
{
    switch (type) {
    case 1:
        perfc_ptr->num_icache_miss = b_perf_read_one(0);
        perfc_ptr->num_icache_ref = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 2:
        perfc_ptr->num_dcache_miss = b_perf_read_one(0);
        perfc_ptr->num_dcache_ref = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 3:
        perfc_ptr->num_l2_cache_miss = b_perf_read_one(0);
        perfc_ptr->num_l2_cache_ref = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 4:
        perfc_ptr->num_rac_access = b_perf_read_one(0);
        perfc_ptr->num_rac_hits = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 5:
        perfc_ptr->num_rac_prefetch = b_perf_read_one(0);
        perfc_ptr->num_retired_instr = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 6:
        perfc_ptr->num_cpu_stall_cycles = b_perf_read_one(0);
        perfc_ptr->num_exceptions_interrupts = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    case 7:
        perfc_ptr->num_cpu_kernel_cycles = b_perf_read_one(0);
        perfc_ptr->num_cpu_user_cycles = b_perf_read_one(1);
        perfc_ptr->num_retired_instr = b_perf_read_one(2);
        perfc_ptr->num_executed_cycles = b_perf_read_one(3);
        break;
    default:
        break;
    }
}
	
#else
void setup_perfc_measurements(int type)
{
    printk("ERROR: BCHP_CHIP not correctly defined, set it to 7405 (for BMIPS_4380) or 7420 (for BMIPS_5000)\n");
}

void get_perfc_snapshot(int type, bmips_perf_counters *perfc_ptr)
{
    printk("ERROR: BCHP_CHIP not correctly defined, set it to 7405 (for BMIPS_4380) or 7420 (for BMIPS_5000)\n");
}
#endif

void calc_perfc_stats(bmips_perf_counters *perfc_cur, bmips_perf_counters *avg, bmips_perf_counters *min, bmips_perf_counters *max)
{
    int i;
    memset(avg, 0, sizeof(bmips_perf_counters));
    memset(min, ~0, sizeof(bmips_perf_counters));
    memset(max, 0, sizeof(bmips_perf_counters));
    printk("# of snapshots taken %d in %d interval\n", end_idx, g_cpu_idle_stats);
    for (i=0; i<end_idx; i++) {
        avg->num_retired_instr += perfc_cur[i].num_retired_instr;
		if (min->num_retired_instr > perfc_cur[i].num_retired_instr)
		    min->num_retired_instr = perfc_cur[i].num_retired_instr;
		if (max->num_retired_instr < perfc_cur[i].num_retired_instr)
		    max->num_retired_instr = perfc_cur[i].num_retired_instr;

		avg->num_executed_cycles += perfc_cur[i].num_executed_cycles;
		if (min->num_executed_cycles > perfc_cur[i].num_executed_cycles)
		    min->num_executed_cycles = perfc_cur[i].num_executed_cycles;
		if (max->num_executed_cycles < perfc_cur[i].num_executed_cycles)
		    max->num_executed_cycles = perfc_cur[i].num_executed_cycles;

		avg->num_icache_ref += perfc_cur[i].num_icache_ref;
		if (min->num_icache_ref > perfc_cur[i].num_icache_ref)
		    min->num_icache_ref = perfc_cur[i].num_icache_ref;
		if (max->num_icache_ref < perfc_cur[i].num_icache_ref)
		    max->num_icache_ref = perfc_cur[i].num_icache_ref;

		avg->num_icache_miss += perfc_cur[i].num_icache_miss;
		if (min->num_icache_miss > perfc_cur[i].num_icache_miss)
		    min->num_icache_miss = perfc_cur[i].num_icache_miss;
		if (max->num_icache_miss < perfc_cur[i].num_icache_miss)
		    max->num_icache_miss = perfc_cur[i].num_icache_miss;

		avg->num_dcache_ref += perfc_cur[i].num_dcache_ref;
		if (min->num_dcache_ref > perfc_cur[i].num_dcache_ref)
		    min->num_dcache_ref = perfc_cur[i].num_dcache_ref;
		if (max->num_dcache_ref < perfc_cur[i].num_dcache_ref)
		    max->num_dcache_ref = perfc_cur[i].num_dcache_ref;

		avg->num_dcache_miss += perfc_cur[i].num_dcache_miss;
		if (min->num_dcache_miss > perfc_cur[i].num_dcache_miss)
		    min->num_dcache_miss = perfc_cur[i].num_dcache_miss;
		if (max->num_dcache_miss < perfc_cur[i].num_dcache_miss)
		    max->num_dcache_miss = perfc_cur[i].num_dcache_miss;

		avg->num_dcache_load_miss += perfc_cur[i].num_dcache_load_miss;
		if (min->num_dcache_load_miss > perfc_cur[i].num_dcache_load_miss)
		    min->num_dcache_load_miss = perfc_cur[i].num_dcache_load_miss;
		if (max->num_dcache_load_miss < perfc_cur[i].num_dcache_load_miss)
		    max->num_dcache_load_miss = perfc_cur[i].num_dcache_load_miss;

		avg->num_dcache_load += perfc_cur[i].num_dcache_load;
		if (min->num_dcache_load > perfc_cur[i].num_dcache_load)
		    min->num_dcache_load = perfc_cur[i].num_dcache_load;
		if (max->num_dcache_load < perfc_cur[i].num_dcache_load)
		    max->num_dcache_load = perfc_cur[i].num_dcache_load;

		avg->num_dcache_store_miss += perfc_cur[i].num_dcache_store_miss;
		if (min->num_dcache_store_miss > perfc_cur[i].num_dcache_store_miss)
		    min->num_dcache_store_miss = perfc_cur[i].num_dcache_store_miss;
		if (max->num_dcache_store_miss < perfc_cur[i].num_dcache_store_miss)
		    max->num_dcache_store_miss = perfc_cur[i].num_dcache_store_miss;

		avg->num_dcache_store += perfc_cur[i].num_dcache_store;
		if (min->num_dcache_store > perfc_cur[i].num_dcache_store)
		    min->num_dcache_store = perfc_cur[i].num_dcache_store;
		if (max->num_dcache_store < perfc_cur[i].num_dcache_store)
		    max->num_dcache_store = perfc_cur[i].num_dcache_store;

		avg->num_retired_load_instr += perfc_cur[i].num_retired_load_instr;
		if (min->num_retired_load_instr > perfc_cur[i].num_retired_load_instr)
		    min->num_retired_load_instr = perfc_cur[i].num_retired_load_instr;
		if (max->num_retired_load_instr < perfc_cur[i].num_retired_load_instr)
		    max->num_retired_load_instr = perfc_cur[i].num_retired_load_instr;

		avg->num_retired_store_instr += perfc_cur[i].num_retired_store_instr;
		if (min->num_retired_store_instr > perfc_cur[i].num_retired_store_instr)
		    min->num_retired_store_instr = perfc_cur[i].num_retired_store_instr;
		if (max->num_retired_store_instr < perfc_cur[i].num_retired_store_instr)
		    max->num_retired_store_instr = perfc_cur[i].num_retired_store_instr;

		avg->num_exceptions += perfc_cur[i].num_exceptions;
		if (min->num_exceptions > perfc_cur[i].num_exceptions)
		    min->num_exceptions = perfc_cur[i].num_exceptions;
		if (max->num_exceptions < perfc_cur[i].num_exceptions)
		    max->num_exceptions = perfc_cur[i].num_exceptions;

		avg->num_l2_cache_ref += perfc_cur[i].num_l2_cache_ref;
		if (min->num_l2_cache_ref > perfc_cur[i].num_l2_cache_ref)
		    min->num_l2_cache_ref = perfc_cur[i].num_l2_cache_ref;
		if (max->num_l2_cache_ref < perfc_cur[i].num_l2_cache_ref)
		    max->num_l2_cache_ref = perfc_cur[i].num_l2_cache_ref;

		avg->num_l2_cache_miss += perfc_cur[i].num_l2_cache_miss;
		if (min->num_l2_cache_miss > perfc_cur[i].num_l2_cache_miss)
		    min->num_l2_cache_miss = perfc_cur[i].num_l2_cache_miss;
		if (max->num_l2_cache_miss < perfc_cur[i].num_l2_cache_miss)
		    max->num_l2_cache_miss = perfc_cur[i].num_l2_cache_miss;

		avg->num_l2_dirty_evictions += perfc_cur[i].num_l2_dirty_evictions;
		if (min->num_l2_dirty_evictions > perfc_cur[i].num_l2_dirty_evictions)
		    min->num_l2_dirty_evictions = perfc_cur[i].num_l2_dirty_evictions;
		if (max->num_l2_dirty_evictions < perfc_cur[i].num_l2_dirty_evictions)
		    max->num_l2_dirty_evictions = perfc_cur[i].num_l2_dirty_evictions;

		avg->num_rac_access += perfc_cur[i].num_rac_access;
		if (min->num_rac_access > perfc_cur[i].num_rac_access)
		    min->num_rac_access = perfc_cur[i].num_rac_access;
		if (max->num_rac_access < perfc_cur[i].num_rac_access)
		    max->num_rac_access = perfc_cur[i].num_rac_access;

		avg->num_rac_hits += perfc_cur[i].num_rac_hits;
		if (min->num_rac_hits > perfc_cur[i].num_rac_hits)
		    min->num_rac_hits = perfc_cur[i].num_rac_hits;
		if (max->num_rac_hits < perfc_cur[i].num_rac_hits)
		    max->num_rac_hits = perfc_cur[i].num_rac_hits;

		avg->num_rac_prefetch += perfc_cur[i].num_rac_prefetch;
		if (min->num_rac_prefetch > perfc_cur[i].num_rac_prefetch)
		    min->num_rac_prefetch = perfc_cur[i].num_rac_prefetch;
		if (max->num_rac_prefetch < perfc_cur[i].num_rac_prefetch)
		    max->num_rac_prefetch = perfc_cur[i].num_rac_prefetch;

		avg->num_cpu_kernel_cycles += perfc_cur[i].num_cpu_kernel_cycles;
		if (min->num_cpu_kernel_cycles > perfc_cur[i].num_cpu_kernel_cycles)
		    min->num_cpu_kernel_cycles = perfc_cur[i].num_cpu_kernel_cycles;
		if (max->num_cpu_kernel_cycles < perfc_cur[i].num_cpu_kernel_cycles)
		    max->num_cpu_kernel_cycles = perfc_cur[i].num_cpu_kernel_cycles;

		avg->num_cpu_user_cycles += perfc_cur[i].num_cpu_user_cycles;
		if (min->num_cpu_user_cycles > perfc_cur[i].num_cpu_user_cycles)
		    min->num_cpu_user_cycles = perfc_cur[i].num_cpu_user_cycles;
		if (max->num_cpu_user_cycles < perfc_cur[i].num_cpu_user_cycles)
		    max->num_cpu_user_cycles = perfc_cur[i].num_cpu_user_cycles;

		avg->num_cpu_stall_cycles += perfc_cur[i].num_cpu_stall_cycles;
		if (min->num_cpu_stall_cycles > perfc_cur[i].num_cpu_stall_cycles)
		    min->num_cpu_stall_cycles = perfc_cur[i].num_cpu_stall_cycles;
		if (max->num_cpu_stall_cycles < perfc_cur[i].num_cpu_stall_cycles)
		    max->num_cpu_stall_cycles = perfc_cur[i].num_cpu_stall_cycles;
    }
}

void bperfc_measure_hook(int type, unsigned int off_usec)
{
    bmips_perf_counters end_perfc_counter;
    bmips_perf_counters avg, min, max;
    switch(type) {
    case 0:
        /* begin performance measurement */
        if (begin_idx >= MAX_PERFC_COUNTERS) {
            if (printk_ratelimit())
                printk("Warning: Counter overflow, may need to allocate > than %d counters\n", MAX_PERFC_COUNTERS);
            return;
        }
        memset(&perfc_counters[begin_idx], 0, sizeof(bmips_perf_counters));
        get_perfc_snapshot(g_perfc_type, &perfc_counters[begin_idx]);
#ifdef PERFC_INSTRUMENT_KERNEL
        /* off_usec is used as a cookie to match a starting & ending snapshots */
        if (off_usec) perfc_counters[begin_idx].off_usec = off_usec;
#endif
        begin_idx++;
        break;
    case 1:
        /* end performance measurement */
        if (end_idx >= MAX_PERFC_COUNTERS) {
            if (printk_ratelimit())
                printk("Warning: Counter overflow, may need to allocate > than %d counters\n", MAX_PERFC_COUNTERS);
            return;
        }
#ifdef PERFC_INSTRUMENT_KERNEL
        //printk("end tv %u, end idx %d\n", off_usec, end_idx);
        if (off_usec != perfc_counters[end_idx].off_usec) {
            if (printk_ratelimit())
                printk("***** skb didn't match, off begin %u, end %u\n", perfc_counters[end_idx].off_usec, off_usec);
        }
#endif
        memset(&end_perfc_counter, 0, sizeof(bmips_perf_counters));
        get_perfc_snapshot(g_perfc_type, &end_perfc_counter);

        /* calculate delta of performance counters */
#if defined(B_PERF_BMIPS_5000)
        get_perfc_delta(&perfc_counters[end_idx], &end_perfc_counter, &perfc_counters[end_idx]);
#elif defined(B_PERF_BMIPS_4380)
        get_perfc_delta(&end_perfc_counter, &perfc_counters[end_idx], &perfc_counters[end_idx]);
#else
#error
#endif
        end_idx++;
        break;
    default:
        /* print performance counters */
        calc_perfc_stats(&perfc_counters[0], &avg, &min, &max);
        print_perfc_stats(&avg, &min, &max);
        end_idx = begin_idx = 0;
        break;
    }
}

int bmips_perf_stats_init(void)
{
    if ( (perfc_counters = vmalloc(sizeof(bmips_perf_counters) * MAX_PERFC_COUNTERS)) == NULL) {
        printk("ERROR: Failed to allocate memory for perfc %d counters\n", MAX_PERFC_COUNTERS);
        return (-ENOMEM);
    }
#ifdef PERFC_INSTRUMENT_KERNEL
    bperfc_set_hook(bperfc_measure_hook);
#endif
    return 0;
}

void bmips_perf_stats_exit(void)
{
    if (perfc_counters) 
        vfree(perfc_counters);
#ifdef PERFC_INSTRUMENT_KERNEL
    bperfc_set_hook(NULL);
#endif
	mdelay(1);
}
#else /* BMIPS_PERFC_ENABLE */
/* dummy functions when BMIPS_PERFC_ENABLE is not defined */
void setup_perfc_measurements(int type)
{
    return;
}
void bperfc_measure_hook(int type, unsigned int off_usec)
{
    return;
}
int bmips_perf_stats_init(void)
{
    return 0;
}
void bmips_perf_stats_exit(void)
{
    return;
}
#endif
