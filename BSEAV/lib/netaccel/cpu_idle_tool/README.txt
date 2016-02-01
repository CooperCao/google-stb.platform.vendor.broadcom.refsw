--------------------------------------------------------------------------------
 CPU IDLE Stats 
--------------------------------------------------------------------------------

1. OVERVIEW

 This driver provides mechanism to accurately determine the CPU load. Existing
 utilities like top dont account for the time spent in the interrupt contexts
 and bottom halves. This driver gathers the CPU idle stats and 
 prints them out at the user configurable interval on the serial console.

 In addition to cpu idle stats, this driver also prints the incoming &
 outgoing N/W thruput numbers.

 Further more, it also allows to further evaluate MIPS performance by using 
 the MIPS performance counters. 

 When there is no useful to be done in the kernel, arch/mips/kernel/process.c:
 cpu_idle() function is invoked. This function runs the idle loop where if
 there is nothing to schedule, it goes into the chip specific low power state.
 This typically invokes wait MIPS instruction (via cpu_wait() )which essentially 
 suspends execution until an interrupt is detected.

 Our kernel has been modified to allow this cpu_wait() to be overwritten by 
 kernel drivers. Our driver overrides this hook and then keep tracks of the 
 idle cycels by using the MIPS count register on BMIPS 4380 & by using
 wait count register on S5000.

 For details, please see the bmips_idle_stats.c: do_wait() function.
--------------------------------------------------------------------------------

2. BUILD

bmips_perf_drv.c contains the source code for driver. 
bmips_idle_stats.c contains the source code for capturing idle stats.
bmips_perf_stats.c contains the source code for capturing performance stats. 
kernel_performance_stats_changes.c contains the sample code for instrumenting
base kernel or kernel driver for measuring its performance counters.
Makefile contains the module makefile.

To rebuild this driver for your kernel, please set following variables:
    PLATFORM, LINUX
To build, use 
    make PLATFORM=97405 LINUX=/opt/brcm/linux-2.6.18-6.2
You can also optionally set LINUXDIR to your kernel source directory. It takes
precedence over LINUX variable.
    
Following defines are being used in this module, turn them on in the Makefile
    BMIPS_PERFC_ENABLE: to enabled system wide cpu performance stats

    PERFC_INSTRUMENT_KERNEL: if you want to get the performance stats of some
    kernel code (base kernel or driver), then define this constant. Please see
    the section below for further details.

To build this driver for Arm kernel, kernel has to be build with 
	EXPORT_SYMBOL(cpu_data); in \linux_3.8-0.5\linux\arch\arm\kernel\setup.c

3. RUN
 # insmod bcm_perf_drv.ko g_cpu_idle_stats=5 g_eth_if_name=eth0 g_perfc_type=2

    g_cpu_idle_stats - Print CPU idle stats every 5sec (default 3sec)
    g_eth_if_num - Print N/W stats for Ethernet i/f eth0 (default: no network
    starts are printed)
    g_perfc_type - Print MIPS performance stats of particular type (default
    none)
 
 Here is an example of the stats that will be output on the console:
   CPU [ 551 +  913 ] IDLE=732 LOAD=26.8% In-Filt Mbps 42.0, In-Tot Mbps 43.2, Out-Filt Mbps 0.0 Out-Tot Mpbs 56.0

 The Idle stats in the above example can be interpreted as follows:
    CPU [ 551 +  913 ]: shows the IDLE time for each CPU (to avoid floating point 
                        calculations in kernel, we show 3rd decimal digital)
                        CPU 0 is 55.1% IDLE & CPU 1 is 91.3% IDLE.

    IDLE=732: average IDLE % of the two CPUs.
              Combined IDLE % of two CPUs is 73.2%

    LOAD=26.8%: average CPU load (1 IDLE) 
                1000 - 732 = 268, so avg CPU load is 26.8%

    In-Filt Mbps 42.0: shows the incoming throughput in Megabits/sec that is 
                       being processed via the NetAccel module.

    In-Tot Mbps 43.2: shows the total incoming throughput processed by both 
                      standard Linux stack & NetAccel modules.

    Out-Filt Mbps 56.0: shows the total throughput being sent out via NetAccel 
                        module 

    Strm-Out Mbps 56.0: shows the total throughput being sent out via standard 
                        Linux stack (doesn't include any packets sent out
                        using NetAccel stack)

--------------------------------------------------------------------------------

