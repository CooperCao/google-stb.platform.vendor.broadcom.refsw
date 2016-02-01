/* 
* Function to set the callback routine to snapshot the counters.
* Add this function to either the kernel driver or base kernel code.
* cpu_idle_stats.c driver expects this hook to be either in the base kernel or a preloaded kernel driver.
*/
void (*bperfc_hook)(int, unsigned int) = NULL;
void bperfc_set_hook(void (*pfun)(int, unsigned int))

{
    bperfc_hook = pfun;
}
EXPORT_SYMBOL(bperfc_set_hook);

/* 
* In order to snapshot the performance counter, use following function in the base kernel or kernel driver 
    bperfc_hook(0, skb->tstamp.off_usec);   // 0 for begining snapshot 
    bperfc_hook(1, skb->tstamp.off_usec);   // 1 for ending snapshot
*/

/* 
 * bcm_perf_drv will calculate the delta between the ending & begining snapshots 
 * and print them on serial console.
 */
