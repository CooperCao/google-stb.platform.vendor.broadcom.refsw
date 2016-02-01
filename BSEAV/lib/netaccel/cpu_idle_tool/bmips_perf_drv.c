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
#include <linux/sysfs.h>
#include <asm/processor.h>
#include <linux/ethtool.h>
#include <net/net_namespace.h>

#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>

#include <linux/proc_fs.h>



#if defined(B_PERF_ARM_V7)	
#include "barm_idle_stats.h"
#else
#include "bmips_idle_stats.h"
#endif

struct proc_dir_entry *bcm_perf_dir_entry;	/** root bcm_perf_dr entry **/
struct proc_dir_entry *bcm_perf_info_entry; /** bcm_perf_file entry **/



extern unsigned long loops_per_jiffy;
extern int bmips_perf_stats_init(void);
extern void bmips_perf_stats_exit(void);
extern void setup_perfc_measurements(int);
extern void bperfc_measure_hook(int type, unsigned int off_usec);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
extern struct net init_net;
#endif

#define BCM_PERF_MAX_ETH_INTERFACE	4


int g_log_type = 1; /**** type of log : 1 for console and 2 for /proc/driver/bcm_perf_drv	****/
module_param(g_log_type, int, S_IRUGO);


int g_cpu_idle_stats = 3; //prints stats every 3sec
module_param(g_cpu_idle_stats, int, S_IRUGO);

char *g_eth_if_name0= NULL; //ethernet i/f to print stats on
module_param(g_eth_if_name0, charp, 0);

char *g_eth_if_name1= NULL; //ethernet i/f to print stats on
module_param(g_eth_if_name1, charp, 0);

char *g_eth_if_name2= NULL; //ethernet i/f to print stats on
module_param(g_eth_if_name2, charp, 0);

char *g_eth_if_name3= NULL; //ethernet i/f to print stats on
module_param(g_eth_if_name3, charp, 0);


int g_perfc_type = 1; //type of performance counter
module_param(g_perfc_type, int, S_IRUGO);

struct net_device *g_dev0 = NULL;
struct net_device *g_dev1 = NULL;
struct net_device *g_dev2 = NULL;
struct net_device *g_dev3 = NULL;


char bcm_perf_log[2048];
int bcm_log_buf_offset = 0; 

typedef struct idle_info_t {
    int initialized;
    struct timer_list timer;
    unsigned long begin_jiffies;
    unsigned long end_jiffies;
    int  count;
    long long strm_bytes;
    int     avg_load;
    atomic_t in_bytes_total[BCM_PERF_MAX_ETH_INTERFACE];
    atomic_t in_bytes_total_prev[BCM_PERF_MAX_ETH_INTERFACE];
    atomic_t in_filtered_bytes;
    atomic_t out_bytes;
    atomic_t out_bytes_total[BCM_PERF_MAX_ETH_INTERFACE];
    atomic_t out_bytes_total_prev[BCM_PERF_MAX_ETH_INTERFACE];
    atomic_t retrans;
    atomic_t xmit_timer_count;
} idle_info_t;
idle_info_t g_idle;
idle_info_t *idle = &g_idle;

#if 0
/* we can use this clock to accurately measure if timer runs accurately on the tickless kernel */
#include "asm/brcmstb/brcmstb.h"
struct wktmr_time start;
#endif
static void perf_timer_routine( unsigned long data)
{
    idle_info_t *idle = (idle_info_t *) data;
    struct net_device_stats *stats0 = NULL;
    struct net_device_stats *stats1 = NULL;
    struct net_device_stats *stats2 = NULL;
    struct net_device_stats *stats3 = NULL;
    int i;
    //struct net_device_stats *stats1 = NULL;

    

    idle->count++;
    if (g_dev0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        stats0 = &g_dev0->stats;
       
#else
        stats0 = g_dev0->get_stats(g_dev0);
#endif
        atomic_set(&idle->out_bytes_total[0], stats0->tx_bytes/g_cpu_idle_stats);
        
        atomic_set(&idle->in_bytes_total[0], stats0->rx_bytes/g_cpu_idle_stats);
        
    }

    if (g_dev1) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        stats1 = &g_dev1->stats;
       
#else
        stats1 = g_dev1->get_stats(g_dev1);
#endif
        atomic_set(&idle->out_bytes_total[1], stats1->tx_bytes/g_cpu_idle_stats);
        
        atomic_set(&idle->in_bytes_total[1], stats1->rx_bytes/g_cpu_idle_stats);
        
    }

    if (g_dev2) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        stats2 = &g_dev2->stats;
       
#else
        stats2 = g_dev2->get_stats(g_dev2);
#endif
        atomic_set(&idle->out_bytes_total[2], stats2->tx_bytes/g_cpu_idle_stats);
        
        atomic_set(&idle->in_bytes_total[2], stats2->rx_bytes/g_cpu_idle_stats);
        
    }

	if (g_dev3) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        stats3 = &g_dev3->stats;
       
#else
        stats3 = g_dev3->get_stats(g_dev3);
#endif
        atomic_set(&idle->out_bytes_total[3], stats3->tx_bytes/g_cpu_idle_stats);
        
        atomic_set(&idle->in_bytes_total[3], stats3->rx_bytes/g_cpu_idle_stats);
        
    }

    

    if (idle->initialized) 
    {
        idle->end_jiffies = jiffies;
        if ((idle->end_jiffies - idle->begin_jiffies) < (g_cpu_idle_stats*HZ)) 
        {
            goto out;
        }
#if 0
        printk("elapsed time in 27Mhz %ld\n", wktmr_elapsed(&start));
#endif
#ifndef PERFC_INSTRUMENT_KERNEL
        /* snapshot the end of perf counters */
        bperfc_measure_hook(1, 0);
#endif

       
        bperfc_measure_hook(2, 0);

#if defined(B_PERF_ARM_V7)	
		barm_idle_stats_print();
#else
        bmips_idle_stats_print();
#endif
        
#ifdef B_REFSW_DEBUG
        printk(" delta jiffies %lu ", idle->end_jiffies - idle->begin_jiffies);
#endif
        if (stats0)
        {

			if(g_log_type == 1)
			{
                printk(" ETH0[ Rx %u.%dMbps, Tx %u.%1uMpbs] ",
                    8*((atomic_read(&idle->in_bytes_total[0]) - atomic_read(&idle->in_bytes_total_prev[0]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[0]) - atomic_read(&idle->in_bytes_total_prev[0]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[0]) - atomic_read(&idle->out_bytes_total_prev[0]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[0]) - atomic_read(&idle->out_bytes_total_prev[0]))) % 10);

            }
            else
            {
				bcm_log_buf_offset +=sprintf(bcm_perf_log + bcm_log_buf_offset," ETH0[ Rx %u.%dMbps, Tx %u.%1uMpbs]",
                    8*((atomic_read(&idle->in_bytes_total[0]) - atomic_read(&idle->in_bytes_total_prev[0]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[0]) - atomic_read(&idle->in_bytes_total_prev[0]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[0]) - atomic_read(&idle->out_bytes_total_prev[0]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[0]) - atomic_read(&idle->out_bytes_total_prev[0]))) % 10);
            }

		}

		if (stats1)
        {

			if(g_log_type == 1)
			{
                printk(" ETH1[ Rx %u.%dMbps, Tx %u.%1uMpbs] ",
                    8*((atomic_read(&idle->in_bytes_total[1]) - atomic_read(&idle->in_bytes_total_prev[1]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[1]) - atomic_read(&idle->in_bytes_total_prev[1]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[1]) - atomic_read(&idle->out_bytes_total_prev[1]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[1]) - atomic_read(&idle->out_bytes_total_prev[1]))) % 10);

                   
            }
            else
            {
				bcm_log_buf_offset +=sprintf(bcm_perf_log + bcm_log_buf_offset," ETH1[ Rx %u.%dMbps, Tx %u.%1uMpbs]",
                    8*((atomic_read(&idle->in_bytes_total[1]) - atomic_read(&idle->in_bytes_total_prev[1]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[1]) - atomic_read(&idle->in_bytes_total_prev[1]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[1]) - atomic_read(&idle->out_bytes_total_prev[1]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[1]) - atomic_read(&idle->out_bytes_total_prev[1]))) % 10);
            }

		}

		
		if (stats2)
        {

			if(g_log_type == 1)
			{
                printk(" ETH2[ Rx %u.%dMbps, Tx %u.%1uMpbs] ",
                    8*((atomic_read(&idle->in_bytes_total[2]) - atomic_read(&idle->in_bytes_total_prev[2]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[2]) - atomic_read(&idle->in_bytes_total_prev[2]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[2]) - atomic_read(&idle->out_bytes_total_prev[2]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[2]) - atomic_read(&idle->out_bytes_total_prev[2]))) % 10);

                   
            }
            else
            {
				bcm_log_buf_offset +=sprintf(bcm_perf_log + bcm_log_buf_offset," ETH2[ Rx %u.%dMbps, Tx %u.%1uMpbs]",
                    8*((atomic_read(&idle->in_bytes_total[2]) - atomic_read(&idle->in_bytes_total_prev[2]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[2]) - atomic_read(&idle->in_bytes_total_prev[2]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[2]) - atomic_read(&idle->out_bytes_total_prev[2]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[2]) - atomic_read(&idle->out_bytes_total_prev[2]))) % 10);
            }

		}

		
		if (stats3)
        {

			if(g_log_type == 1)
			{
                printk(" ETH3[ Rx %u.%dMbps, Tx %u.%1uMpbs] ",
                    8*((atomic_read(&idle->in_bytes_total[3]) - atomic_read(&idle->in_bytes_total_prev[3]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[3]) - atomic_read(&idle->in_bytes_total_prev[3]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[3]) - atomic_read(&idle->out_bytes_total_prev[3]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[3]) - atomic_read(&idle->out_bytes_total_prev[3]))) % 10);

                    
            }
            else
            {
				bcm_log_buf_offset +=sprintf(bcm_perf_log + bcm_log_buf_offset," ETH3[ Rx %u.%dMbps, Tx %u.%1uMpbs]",
                    8*((atomic_read(&idle->in_bytes_total[3]) - atomic_read(&idle->in_bytes_total_prev[3]))) / 1000000, 
                    8*((atomic_read(&idle->in_bytes_total[3]) - atomic_read(&idle->in_bytes_total_prev[3]))) % 10, 
                    8*((atomic_read(&idle->out_bytes_total[3]) - atomic_read(&idle->out_bytes_total_prev[3]))) / 1000000,
                    8*((atomic_read(&idle->out_bytes_total[3]) - atomic_read(&idle->out_bytes_total_prev[3]))) % 10);
            }
		
    	}


    	if(g_log_type == 1)
    	{
			 printk("\n");
    	}
    	else
    	{
			bcm_log_buf_offset +=sprintf(bcm_perf_log + bcm_log_buf_offset," \n");
    	}
    } 
    else
    {
        /* (!idle->initialized) */
#if defined(B_PERF_ARM_V7)	
		barm_idle_stats_print();
#else
        bmips_idle_stats_print();
#endif

        idle->begin_jiffies = jiffies;
        idle->initialized = 1;
        atomic_set(&idle->out_bytes, 0);
    }
    
    atomic_set(&idle->xmit_timer_count,0);
    idle->begin_jiffies = jiffies;

    idle->strm_bytes += (atomic_read(&idle->out_bytes));
    atomic_set(&idle->out_bytes, 0);
    atomic_set(&idle->in_filtered_bytes, 0);

    for(i=0; i < BCM_PERF_MAX_ETH_INTERFACE; i++)
    {
	    atomic_set(&idle->out_bytes_total_prev[0], atomic_read(&idle->out_bytes_total[0]));
	    atomic_set(&idle->in_bytes_total_prev[0], atomic_read(&idle->in_bytes_total[0]));
	}

out:
    /* restart this timer */
    mod_timer(&idle->timer, jiffies + g_cpu_idle_stats * (HZ/100));
#ifndef PERFC_INSTRUMENT_KERNEL
    /* snaptop the start of perf counters */
    bperfc_measure_hook(0, 0);
#endif

#if 0
    wktmr_read(&start);
#endif

}


static int bcm_proc_read_info(char *buf, char **start, off_t offset,int count, int *eof, void *data)
{
	memcpy(buf, bcm_perf_log , sizeof(bcm_perf_log));
	
	return sizeof(bcm_perf_log);

}

static void bmips_perf_drv_proc_init(void)
{
	bcm_perf_dir_entry = proc_mkdir("bcm_perf_drv", NULL);

	bcm_perf_info_entry = create_proc_entry("info", S_IFREG|S_IRUGO, bcm_perf_dir_entry);

	bcm_perf_info_entry->read_proc = bcm_proc_read_info;

}

static void bmips_perf_drv_proc_exit(void)
{

	if(bcm_perf_info_entry){
		remove_proc_entry("info", bcm_perf_dir_entry);
	}

	if(bcm_perf_dir_entry)
	{
		remove_proc_entry("bcm_perf_drv", NULL);
	}

}


int bmips_perf_drv_init(void)
{
    if (g_cpu_idle_stats <= 0 || g_cpu_idle_stats > 10)
        g_cpu_idle_stats = 1;
    printk("%s: Printing CPU IDLE Stats Every %d sec for interfaces: ", __FUNCTION__, g_cpu_idle_stats);
    if (g_eth_if_name0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        if (strstr(g_eth_if_name0, "eth0")) {
            g_dev0= dev_get_by_name(&init_net, "eth0");
            if (g_dev0)
                printk("eth0, ");
        }
        printk("\n");
#else
        g_dev0= dev_get_by_name(g_eth_if_name0);
#endif
    }
    else 
        printk("\n");

    if (g_eth_if_name1) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        if (strstr(g_eth_if_name1, "eth1")) {
            g_dev1= dev_get_by_name(&init_net, "eth1");
            if (g_dev1)
                printk("eth1, ");
        }
        printk("\n");
#else
        g_dev1= dev_get_by_name(g_eth_if_name1);
#endif
    }
    else 
        printk("\n");

	if (g_eth_if_name2) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        if (strstr(g_eth_if_name2, "eth2")) {
            g_dev2= dev_get_by_name(&init_net, "eth2");
            if (g_dev2)
                printk("eth2, ");
        }
        printk("\n");
#else
        g_dev2= dev_get_by_name(g_eth_if_name2);
#endif
    }
    else 
        printk("\n");

	if (g_eth_if_name3) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
        if (strstr(g_eth_if_name3, "eth3")) {
            g_dev3= dev_get_by_name(&init_net, "eth3");
            if (g_dev3)
                printk("eth3, ");
        }
        printk("\n");
#else
        g_dev3= dev_get_by_name(g_eth_if_name3);
#endif
    }
    else 
        printk("\n");
        

#if defined(B_PERF_ARM_V7)	
	if (barm_idle_stats_init())
        return (-1);    
#else		
    if (bmips_idle_stats_init())
        return (-1);
#endif

        
    if (bmips_perf_stats_init())
        return (-1);
    /* what type of MIPS performance stats to measure */
    setup_perfc_measurements(g_perfc_type);

    /* Setup the timer to periodically print the stats */
    init_timer(&idle->timer);
    idle->timer.data = (unsigned long)idle;
    idle->timer.function = perf_timer_routine;
    idle->timer.expires = jiffies + HZ*g_cpu_idle_stats;
    add_timer(&idle->timer);

    if(g_log_type == 2)
    {
    	bmips_perf_drv_proc_init();

    }

    return (0);
}


void bmips_perf_drv_exit(void)
{
    printk("%s: Exit\n", __FUNCTION__);
    del_timer_sync(&idle->timer);

#if defined(B_PERF_ARM_V7)	
	barm_idle_stats_exit();
#else
    bmips_idle_stats_exit();
#endif  

    bmips_perf_stats_exit();

    bmips_perf_drv_proc_exit();
    
	if(g_dev0)
	{
		dev_put(g_dev0);
		g_dev0=NULL;
	}
	if(g_dev1)
	{
		dev_put(g_dev1);
		g_dev1=NULL;
	}
	if(g_dev2)
	{
		dev_put(g_dev2);
		g_dev2=NULL;
	}
	if(g_dev3)
	{
		dev_put(g_dev3);
		g_dev3=NULL;
	}
	
	
}




module_init(bmips_perf_drv_init);
module_exit(bmips_perf_drv_exit);

MODULE_LICENSE("Dual BSD/GPL");

