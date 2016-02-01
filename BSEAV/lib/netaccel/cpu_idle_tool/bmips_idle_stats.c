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
 * $brcm_Log: $
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
#include <linux/timer.h>
#include <linux/cpumask.h>

extern unsigned long loops_per_jiffy;
extern int g_cpu_idle_stats;

static int loop[NR_CPUS];
static void (*cpu_wait_orig)(void);
static volatile unsigned long long idle_cycles[NR_CPUS];   /* Per CPU IDLE Count */
static int max_online_cpus = 1;
static unsigned long max_cycles_per_msec[NR_CPUS];

typedef struct bmips_idle_stats_info_t {
    int initialized;
    volatile unsigned long long begin_cycles[NR_CPUS];
    volatile unsigned long long end_cycles[NR_CPUS];
    volatile unsigned long begin_jiffies[NR_CPUS];
    volatile unsigned long end_jiffies[NR_CPUS];
    int  count;
    int  percent[NR_CPUS];
	int 	avg_load;
} bmips_idle_stats_info_t;

static bmips_idle_stats_info_t bmips_idle_stats_info;
static bmips_idle_stats_info_t *idle = &bmips_idle_stats_info;

// USE software based counting (now it is obsolete)
//#define USE_SW_CNT_MECHANISM 

/* measures the cpu ticks spent until next interrupt comes in */
static unsigned long do_wait(void)
{
    unsigned long ticks, tmp;

#ifdef USE_SW_CNT_MECHANISM
	int i;
	for(i=0;i<(1<<14);i++){
		cpu_relax();
	}
	ticks = tmp = (1<<14);
	ticks <<= 1;
#elif defined(B_PERF_BMIPS_5000)
    /* S5000 processor maintains the count of cycles spent during wait instruction (i.e. idle time) in CP0 reg 9, select 7 for each TP */
    /* we note it before entering & after exiting the idle state */
    __asm__ __volatile__(
    "   .set    push\n"
    "   .set    noreorder\n"
#define DELTA_METHOD
#ifdef DELTA_METHOD
//read current value of idle cycles ($9, select 7) in reg1
    "   mfc0    %1, $9, 7\n"
#else
//clear the current idle cycle count
    "   li      %1, 0\n"
    "   mtc0    %1, $9, 7\n"
#endif
//enable interrupts (incase they were disabled, is that possible in Linux), this way we know for sure to wakeup a/f an interrupt
    "   mfc0    %0, $12\n"
    "   ori %0, 1\n" 
    "   mtc0    %0, $12\n"
//wait until next interrupt arrives (either cpu 1msec tick or n/w pkt)
    "   wait\n"
//interrupt must have come & was serviced, now again note the new value of idle cycles in reg0
    "   mfc0    %0, $9, 7\n"
//this is the current idle cycles
#ifdef DELTA_METHOD
    "   subu    %0, %1\n"
#endif
    "   .set    pop\n"
    : "=&r" (ticks), "=&r" (tmp));
#else //defined(B_PERF_BMIPS_4380)
    /* CPU doesn't provide any specific instruction to measure the idle time */
    /* so we mask off interrupts, use the CP0 Count register (reg 9, select 0) to note the current cycles, */
    /* poll for any interrupt, when it comes note the current cycles again to know the actual idle cycles */
    /* enable interrupts, and then call wait which will immediately return and excute the interrupt handler */
    unsigned long tmp2;
    __asm__ __volatile__(
    "   .set    push\n"
    "   .set    noreorder\n"
//disable interrupts ($12 is the status reg, bit 0 is IE)
    "   mfc0    %0, $12\n"
    "   li      %2, ~0x1\n"
//clears interrupt bit
    "   and %0, %2\n"
    "   mtc0    %0, $12\n"
//read current value of perf counter($9) in reg1
    "   mfc0    %1, $9\n"
//poll the cause register ($13) to wait until next interrupt arrives (either cpu 1msec tick or n/w pkt)
    "   li  %2, 0xff00\n"
    "1: mfc0    %0, $13\n"
    "   and %0, %2\n"
    "   beqz    %0, 1b\n"
    "   nop\n"
//perf counter has been updated, get the new value in reg2
    "   mfc0    %2, $9\n"
//re-enable the interrupts
    "   mfc0    %0, $12\n"
    "   ori %0, 1\n"
    "   mtc0    %0, $12\n"
//calculate the change in perf counter value
    "   subu    %0, %2, %1\n"
    "   .set    pop\n"
    : "=&r" (ticks), "=&r" (tmp), "=&r" (tmp2));
#endif

    return (ticks);
}

/* Each time CPU is idle, kernel's idle thread calls this function. It keeps track of the total IDLE cycles */
/* by counting either using a SW counter or using HW count register. This value can then be compared */
/* with the max value CPU can count in a sec to determine the CPU IDLE & Load values */
static void cpu_wait_new(void)
{
	int cpu = smp_processor_id();
	/* uses MIPs COUNT register for counting */
#if defined(B_PERF_BMIPS_5000)
	idle_cycles[cpu] += (do_wait() << 3);
#else
	idle_cycles[cpu] += (do_wait() << 1);
#endif
	loop[cpu]++;
}

static unsigned long long bmips_cycles_count(int cpu)
{
	return idle_cycles[cpu];
}

static void reset_bmips_cycles_count(int cpu)
{
    idle_cycles[cpu] = 0LL;
}

/*#define B_REFSW_DEBUG*/
void bmips_idle_stats_print(void)
{
    volatile unsigned long long delta_cycles[NR_CPUS];
    volatile unsigned long delta_jiffies[NR_CPUS];
    unsigned int avg_idle = 0;
    int i;

    if (idle->initialized) {
        for (i=0;i<max_online_cpus; i++) {
            idle->end_jiffies[i] = jiffies;
            idle->end_cycles[i] = bmips_cycles_count(i);
#ifdef B_REFSW_DEBUG
            if (idle->end_cycles[i] <= idle->begin_cycles[i]) {
                printk("cpu %d cycle wrap, end %llu, begin %llu\n", i, idle->end_cycles[i], idle->begin_cycles[i]);
                goto out;
            }
#endif
            delta_cycles[i] = idle->end_cycles[i] - idle->begin_cycles[i];
            if (delta_cycles[i] < 0)
                delta_cycles[i] = 0;
            delta_jiffies[i] = idle->end_jiffies[i] - idle->begin_jiffies[i];
            //idle->percent[i] = (HZ * delta_cycles[i]) / (max_cycles_per_msec[i]*delta_jiffies[i]); // still in msec to use 1000 as base 
            idle->percent[i] = delta_cycles[i] / (max_cycles_per_msec[i]*g_cpu_idle_stats); // still in msec to use 1000 as base 
            if (idle->percent[i] > 1000)
				idle->percent[i] = 1000;
            else if (idle->percent[i] < 0)
				idle->percent[i] = 0;
            if (idle->end_cycles[i] == idle->begin_cycles[i]) {
				idle->percent[i] = 1000;
            }
            avg_idle += idle->percent[i];
        }
        avg_idle /= max_online_cpus; 
        idle->avg_load = 1000 - avg_idle;

        printk("CPU [");
        for(i=0;i<max_online_cpus; i++) {
            if (i>0) printk(" + ");
            printk("idle %4d",idle->percent[i]);
#ifdef B_REFSW_DEBUG
            printk(" cnt %4d, cy %10lld, jif %lu", loop[i], delta_cycles[i], delta_jiffies[i]);
#endif
        }
        printk("] IDLE=%4u LOAD=%2d.%1d%%",avg_idle,idle->avg_load/10,idle->avg_load %10);
    }
    else
    {
        idle->initialized = 1;
    }
#ifdef B_REFSW_DEBUG
out:
#endif
    for (i=0;i<max_online_cpus; i++) {
        idle->begin_jiffies[i] = jiffies;
        idle->begin_cycles[i] = bmips_cycles_count(i);
        loop[i] = 0;
        //reset_bmips_cycles_count(i);
    }
}

int bmips_idle_stats_init(void)
{
    int i;
    if (cpu_online(3))
        max_online_cpus = 4;
    else if (cpu_online(1))
        max_online_cpus = 2;
    else
        max_online_cpus = 1;

	for (i=0;i<max_online_cpus; i++) {
		idle_cycles[i] = 0LL;

        /* kernel stores max (scaled) cycle count per millisec in the udelay_val variable */
        /* times that w/ 1000 gives you the max count that a completely idle CPU can count upto, */
        /* to avoid FP math in kernel, we keep 1000 as the unit. It also helps with % calulations. */
#if defined(B_PERF_BMIPS_5000)
        /* hard coding to use TP0 value as udelay value for both threads as kernel reports lower value for TP1 due to shared pipeline */
        max_cycles_per_msec[i] = cpu_data[0].udelay_val*3;   // scale factor is 3, as count is incremented every 8th cycle
#else
        max_cycles_per_msec[i] = cpu_data[i].udelay_val*2;   // scale factor is 2, as count is incremented every other cycle
#endif
        printk("cpu[%d]: max cycle count %lu, udelay %u, lpj %lu\n", i, max_cycles_per_msec[i]*1000, cpu_data[0].udelay_val, loops_per_jiffy);
    }

    /* Override the kernel cpu_wait function */
	cpu_wait_orig = cpu_wait;
	cpu_wait = &cpu_wait_new;
    return 0;
}

void bmips_idle_stats_exit(void)
{
	cpu_wait = cpu_wait_orig;
	mdelay(1);
}

