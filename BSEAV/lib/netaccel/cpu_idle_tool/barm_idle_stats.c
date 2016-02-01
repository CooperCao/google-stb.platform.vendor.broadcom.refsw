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
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/timer.h>
#include <linux/cpumask.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/hardware/gic.h>
#include <linux/kconfig.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/brcmstb/cma_driver.h>
#include <linux/mm_types.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>



extern void (*pm_idle) (void);
static void (*pm_idle_orig)(void);
extern int g_cpu_idle_stats;
extern int g_log_type;

extern char bcm_perf_log[];
extern int bcm_log_buf_offset;


static int loop[NR_CPUS];
static volatile unsigned int idle_cycles[NR_CPUS];   /* Per CPU IDLE Count */
static int max_online_cpus = NR_CPUS;


typedef struct barm_idle_stats_info_t {
	int armPmuInit[NR_CPUS];
	int percent[NR_CPUS];
	int avg_load;
} barm_idle_stats_info_t;

static barm_idle_stats_info_t barm_idle_stats_info;
static barm_idle_stats_info_t *idle = &barm_idle_stats_info;

/* measures the cpu ticks spent until next interrupt comes in */


static unsigned int gic_interrupt_count[8][256];
void __iomem *gic_dist_base;
int print_once = 1;
void arm_gic_busywait(void)
{
    if (!gic_dist_base) {
        __asm__("dsb");
        __asm__("wfi");
    } 
    else
    {
        int has_irq = 0;
        int i;

        if (print_once) {
            print_once = 0;
            pr_info("%s: now using poller!\n", __FUNCTION__);
        }

        /** 
         	* Reading GICC_IAR acknowledges the interrupt, which is bad
         	* because the interrupt should really be processed by the
         	* ISR. Instead, we loop through all of the interrupt status
         	* bits (& mask) and determine if it's okay to break the loop.
         	**/
        do 
        {
            for (i = 0; !has_irq && i < 8; i++) 
            {
                u32 GICD_ISENABLE_i;
                u32 GICD_ISPEND_i;

                GICD_ISENABLE_i = readl_relaxed(gic_dist_base + GIC_DIST_ENABLE_SET + i * 4);
                GICD_ISPEND_i = readl_relaxed(gic_dist_base + GIC_DIST_PENDING_SET + i * 4);
                if ((GICD_ISENABLE_i & GICD_ISPEND_i) != 0) 
                {
                    /* interrupt is pending, lets find out if it is pending for this cpu */
                    unsigned int interrupts;
                    int firstSetBit;
                    uint8_t interruptCpuTargets; 
                    u32 interruptCpuTargetsWord; 
                    int targetRegBytePosition;
                    int targetRegOffset;
                    int interruptId;
                    int targetRegByteOffset;
                    int shift;

                    interrupts = GICD_ISENABLE_i & GICD_ISPEND_i;
                    /* printk("all interrupts 0x%x at i %d, ffs %d, cpu id %d", interrupts, i, ffs(interrupts), smp_processor_id());*/
                    /* find the first set bit and map it to the interrupt number */
                    while ((firstSetBit = ffs(interrupts)) != 0) 
                    {
                        /* interrupt at position firstSetBit is set, but we dont know whether our cpu has requested this interrupt or not */
                        /* To do so, we find if this interrupt is enabled in the target processor register for this cpu */
                        int cpuId = smp_processor_id();
                        /* one bit per interrupt, upto 256 interrupts, so 8 registers are used for interrupt enable/status/pending bit configuration/indication */
                        /* also interrupt numbers begin from ffs-1 as ffs returns from bit position 1 & not 0 */
                        interruptId = (firstSetBit - 1) + i*32;

                        /* now we use this interruptId to find the byte corresponding to this interrupt where per processor target configuration is done */
                        /* one byte is used per interrupt and this byte has the bitmask for the cpus which are interested in this interrupt */
                        targetRegOffset = interruptId / 4; /* 4 interrupts can be configured in each GISD_ITARGETSR, so we find the correct register offset */
                        targetRegBytePosition = interruptId % 4; /* this gives the correct byte position within this register */
                        targetRegByteOffset = GIC_DIST_TARGET + (targetRegOffset*4) + targetRegBytePosition; /* complete byte offset of the GISD_ITARGETSR for this interrupt */
#if 0
                        /* now read the actual cpu target */
                        /* interruptCpuTargets = readb_relaxed(gic_dist_base + targetRegByteOffset);*/
#else
                        /* try the word access instead */
                        targetRegByteOffset = GIC_DIST_TARGET + targetRegOffset*4; /* complete word offset of the GISD_ITARGETSR for this interrupt */
                        interruptCpuTargetsWord = readl_relaxed(gic_dist_base + targetRegByteOffset);
                        shift = targetRegBytePosition * 8;
                        interruptCpuTargets = interruptCpuTargetsWord >> shift;
#if 0
                        if (cpuId != 0)
                        {
                            printk("ssood: #### %s: i %d, intr 0x%x, intr clr 0x%x, intr reg addr 0x%x, int id %x, intr word 0x%x, shift %d, intr byte 0x%x\n", 
                                    __FUNCTION__, i, interrupts, (interrupts & ~firstSetBit), (gic_dist_base + targetRegByteOffset), interruptId, interruptCpuTargetsWord, shift, interruptCpuTargets);
                        }
#endif
#endif
                        if ((interruptCpuTargets & (1 << cpuId)) != 0) {
                            /* interruptId is set for this cpuId, so we dont need to look for any other additional interrupts and thus break out of this cpu's interrupt polling loop */
                            has_irq = 1;

                            gic_interrupt_count[cpuId][interruptId]++;

                            break;
                        }
                        else 
                        {
                            /* this interrupt is not enabled for this cpu, so clear out its position and continue above */
                            interrupts &= (~(1<<(firstSetBit-1)));
                        }

                    } /* while */
                } /* if */
            } /* for */
        } while(!has_irq);
    }

    /* 
     * Interrupt was detected. Return so caller can re-enable IRQs
     * and vector to the ISR.
     */
}

static unsigned int do_wait(void)
{
	volatile unsigned int ticks = 0;
    volatile unsigned int value1, value2;

    /* Read CCNT Register*/
    asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value1));
  
#if	0
	asm volatile ("dsb");					
	asm volatile ("wfi");
#else	
	arm_gic_busywait();
#endif	

	/* Read CCNT Register	*/
    asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value2));
   
	ticks = (value2 - value1);
    return (ticks);
}

	
void initArmPmu(void)
{
    /* #warning "needs porting for armv7"	*/
    /** Arnab adding assembly for ARM **/
    
    unsigned int value ;

    value = 1;     /* enable all counters */
#if	0    
    value |= 2;	   /* reset event counters */
#endif
    
    value |= 4;	   /* reset cycle counter */
    value |= 8;	   /* enables count increment every 64 clock cycles */

    asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

    /* enable CCNT counters:  */
    asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x80000000));

    /* clear CCNT overflows:	*/
    asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x80000000));

    /*  asm volatile ("MCR p15, 0, %0, c9, c13, 0\t\n" :: "r"(0x1));	*/


    /* disable counter overflow interrupts (just in case)*/
    asm volatile ("MCR p15, 0, %0, C9, C14, 2\n\t" :: "r"(0x80000000));
    
}


/* Each time CPU is idle, kernel's idle thread calls this function. It keeps track of the total IDLE cycles */
/* by counting either using a SW counter or using HW count register. This value can then be compared */
/* with the max value CPU can count in a sec to determine the CPU IDLE & Load values */
static void cpu_wait_new(void)
{
	unsigned int cycleCount = 0;
	int cpu = smp_processor_id();

	if (idle->armPmuInit[cpu] == 0)
    {
        initArmPmu();
        idle->armPmuInit[cpu] = 1;
        printk("initialized PMU for %d cpu of max cpu %d\n", cpu, max_online_cpus);
    }
    else
    {
		/* For MIPS, we use MIPs COUNT register for counting */
	    /* For ARM based platforms, we use the cycle count register in the PMU */	
#if	1
		cycleCount = do_wait();
		idle_cycles[cpu] += cycleCount;
		loop[cpu]++;
#else
		idle_cycles[cpu] += do_wait();
		loop[cpu]++;
#endif
	}
	
	local_irq_enable();
	
}

static unsigned int barm_cycles_count(int cpu)
{
	return idle_cycles[cpu];
}

static void reset_barm_cycles_count(int cpu)
{
    idle_cycles[cpu] = 0;
}

/*#define B_REFSW_DEBUG*/
void barm_idle_stats_print(void)
{
    int i;
    int avg_idle = 0;

    /*	char buf[256];	*/
    char *buf_temp = bcm_perf_log;

    for(i=0;i<max_online_cpus; i++) 
	{
		unsigned int del_temp_cycle = barm_cycles_count(i);
		unsigned int max_temp_cycle_per_msec = 234375;/*max_cycles_per_msec[i];*/ /** (1500000 / 64) = 23437.5  and the arm divider is set for 64,
														so we maintain in 10000ms to maintain precision of 0.5**/
#ifdef	B_REFSW_DEBUG
        printk("cpu %d, cycles %u per %d sec, # of cpu_idle calls %d, Max Cycles/sec %u\n", 
                i, del_temp_cycle*64, g_cpu_idle_stats, loop[i], max_temp_cycle_per_msec*64*100);
#endif
        
		idle->percent[i] = ((del_temp_cycle*10)/(g_cpu_idle_stats*max_temp_cycle_per_msec)) ;

        /* TODO: make it more accurate */
		if (idle->percent[i] > 1000)
			idle->percent[i] = 1000;
        else if (idle->percent[i] < 0)
			idle->percent[i] = 0;

		avg_idle += idle->percent[i];			
	}	

	avg_idle /= max_online_cpus; 
    idle->avg_load = 1000 - avg_idle;
	if(g_log_type == 1)
	{
	    printk("CPU [");
	    for(i=0;i<max_online_cpus; i++) {
	        if (i>0) printk(" + ");
	        printk("idle %4d",idle->percent[i]);
	    }
		printk("] IDLE=%4u LOAD=%2d.%1d%%",avg_idle,idle->avg_load/10,idle->avg_load %10);
	}
	else
	{
		int temp_len1 = 0;
		int temp_len2 = 0;

#if	0		
		temp_len1 = snprintf(buf+temp_len2, 0, "CPU ["); /* returns the number of characters needed, including \n but excluding \0 */
		temp_len2 += snprintf(buf+temp_len2, temp_len1, "CPU [");
		for(i=0;i<max_online_cpus; i++) {
	        if (i>0){
	        	temp_len1 = snprintf(buf+temp_len2, 0," + ");/* returns the number of characters needed, including \n but excluding \0 */
	        	temp_len2 += snprintf(buf+temp_len2, temp_len1," + "); 
	        }
	    }

	    printk("------------ Here is the temp Bufferr           %s \n", buf);
#endif
	    temp_len1 = 0;
	    temp_len2 = 0;

		temp_len1 = sprintf(buf_temp,"CPU IDLE [");
		for(i=0;i<max_online_cpus; i++) {
	        if (i>0){
	        	temp_len1 +=sprintf(buf_temp + temp_len1," + ");
	    	}
	    	temp_len1 +=sprintf(buf_temp + temp_len1,"%3d.%1d",idle->percent[i]/10,idle->percent[i]%10);

	    }

	    temp_len1 +=sprintf(buf_temp + temp_len1,"] IDLE=%2d.%1d%% LOAD=%2d.%1d%%",avg_idle/10,avg_idle%10,idle->avg_load/10,idle->avg_load %10);

	    bcm_log_buf_offset = temp_len1;
#if	0
	    
	     printk("----- %s ", buf_temp);
		
		//temp_len2 += snprintf(buf+len , 0, "CPU [");
#endif		

	}
    
    for (i=0;i<max_online_cpus; i++)
    {
        loop[i] = 0;
        reset_barm_cycles_count(i);
    }
}

int barm_idle_stats_init(void)
{
    int i;
    uint32_t gic_controller_type;
    struct device_node *dev_node = NULL;
	const __be32 *prop_be;
	u32  base, length;
	int len;
    
    if (cpu_online(3))
        max_online_cpus = 4;
    else if (cpu_online(2))
        max_online_cpus = 3;
    else if (cpu_online(1))
        max_online_cpus = 2;
    else
        max_online_cpus = 1;

	for (i=0;i<max_online_cpus; i++) 
	{
		idle_cycles[i] = 0;

		idle->armPmuInit[i] = 0;
        /* kernel stores max (scaled) cycle count per millisec in the udelay_val variable */
        /* times that w/ 1000 gives you the max count that a completely idle CPU can count upto, */
        /* to avoid FP math in kernel, we keep 1000 as the unit. It also helps with % calulations. */
    }

	/*	dev_node = find_devices("/intc");	*/
	dev_node = of_find_node_by_name(NULL,"interrupt-controller");
	
	if (!dev_node) {
		printk("'intc' node not in DT");
		return(-1);

	}
		/* Get "/intc/reg" property */
	prop_be = of_get_property(dev_node, "reg", &len);

	if (!prop_be) {
		printk("'/intc' node missing 'reg`'");
		return(-1);
	}

	base = be32_to_cpup(prop_be++);
	length = be32_to_cpup(prop_be++);
		

	printk("ARM GIC contoller's address: gic_dist_base 0x%x , and len is 0x%x\n", base, length);

	
	gic_dist_base = ioremap_nocache(base, length);
    
    gic_controller_type = readl_relaxed(gic_dist_base + GIC_DIST_CTR);
    printk("ARM GIC contoller's address: gic_dist_base %p, type 0x%x\n", gic_dist_base, gic_controller_type&0xff);


    /* Override the kernel cpu_wait function */
	pm_idle_orig = pm_idle;
	pm_idle = &cpu_wait_new;

    return 0;
}

void barm_idle_stats_exit(void)
{
    int cpuId, interruptId;
#if 1
    printk("%s: gic_interrupt_count %p\n", __FUNCTION__, gic_interrupt_count);
    if (gic_interrupt_count == NULL)
        return;
    for (cpuId = 0; cpuId < 8; cpuId++) {
        for (interruptId = 0; interruptId < 160; interruptId++) {
            if (gic_interrupt_count[cpuId][interruptId] != 0) 
                printk("%s: cpuId %d, interruptId %d, intr count %u\n", __FUNCTION__, cpuId, interruptId, gic_interrupt_count[cpuId][interruptId]);
        }
    }
#endif
    iounmap(gic_dist_base);
	pm_idle =	pm_idle_orig;
	mdelay(1);
}

