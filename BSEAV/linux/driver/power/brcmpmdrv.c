/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
 *  Test driver for Linux powerdown mode
 *
 * Design:
 * Insmod-able driver with a single SLEEP IOCTL.
 * When invoked it will substitute a body of code for the main MIPS interrupt vector.
 * The body of code is designed to run entirely in cache.
 * The body of code will manage a timer in the UPG block to maintain time of day.
 * The SLEEP IOCTL can accept a parameter specifying a time of day/date when the system is to be woken up.
 * The body of code will manage 2 other interrupts: IR receive and GPIO
 * If the code detects a valid IR code it will wake up the system
 * The SLEEP IOCTL can accept a parameter specifying which GPIO's (front panel buttons) will cause the system to wake up.
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#define _BRCMPMDRV_INTERNAL_        1

/* include statement below normally placed on command line now */
#include <linux/autoconf.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/signal.h>
#include <linux/pm.h>
#include <linux/cpumask.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
#include <linux/semaphore.h>
#endif

#include <asm/div64.h>
#include <asm/mipsregs.h>
#include <asm/hazards.h>
#include <asm/r4kcache.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <asm/cpu-info.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
#include <asm/brcmstb/brcmapi.h>
#else
#include <asm/brcmstb/common/brcmstb.h>
#endif

#include "bstd.h"
#include "bdbg.h"
#include "bchp.h"
#include "bchp_common.h"
#if (BCHP_CHIP == 7125)
#include "bchp_memc_gen_0.h"
#else
#include "bchp_memc_0.h"
#endif
#include "bchp_hif_cpu_intr1.h"
#include "bchp_irq0.h"
#include "bchp_ldk.h"
#include "bchp_kbd1.h"
#include "bchp_timer.h"
#include "bint.h"
#include "btmr.h"
#include "bkpd.h"
#include "bled.h"
#include "bkir.h"
#include "bgio.h"
#include "bkni.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_gio.h"
#include "brcmpmdrv.h"

#ifdef HDMI_CEC_FEATURE 
#include "bchp_dvp_hr.h"
#include "bchp_hdmi_rx_fe_0.h"
#include "bchp_dvp_hr_intr2.h"
#endif

#if SPI_FP
#include "bchp_hif_top_ctrl.h"
#endif

static char *msg_modules = "";
#include <linux/moduleparam.h>
module_param(msg_modules, charp, 0000);
MODULE_PARM_DESC(msg_modules, "Enable BDBG_MSG level per module");

BDBG_MODULE(ppmd);

#define DRIVER_ERROR(error) BDBG_ERR(("pmdrv: Driver error, code=%d",error))

#define PPMD_MAJOR 30
#define PPMD_MINOR 1
#define WAKEUP 1
#define PPMD_HEAP_ORDER 2   /* 8K for starts */
#define MHZ 1000000
#define IR_REMOTE_A_POWER_BUTTON 0xa
#define KEYPAD_POWER_BUTTON 0xfffe
#define WKTMR_FREQ 27000000

#ifndef DEV_RD
#define DEV_RD(x) (*((volatile unsigned long *)(x)))
#define DEV_WR(x, y) do { *((volatile unsigned long *)(x)) = (y); } while(0)
#endif

extern void passiveIRQ(void);
extern void passiveIRQ_end(void);

/* prototypes */
static int my_init (void);
static void my_exit (void);

static void dummy_tmr_callback(void);
static int test_for_memory_self_refresh(void);
static int test_for_gpio_wakeup(void);
static int test_for_keypad_wakeup(void);
static int test_for_ir_remote_wakeup(void);
static int alarm_time_ok(void);
static int upg_timer_duration_ok(void);
static int alarm_time_reached(void);
static void ppmd_drv_set_module_debuglevel(const char *, BDBG_Level);
static void ppmd_drv_set_curr_time(void);
static void ppmd_drv_adjust_curr_time(void);
static void ppmd_drv_adjust_kernel_time(void);
static void ppmd_drv_rearm_timer(void);
static void ppmd_drv_stop_timer(void);
static void ppmd_drv_disable_interrupts(void);
static void ppmd_drv_enable_interrupts(void);
static int ppmd_drv_replace_exception_vectors(void);
static int ppmd_drv_restore_exception_vectors(void);
static int ppmd_drv_replace_level_1_interrupts(void);
static int ppmd_drv_restore_level_1_interrupts(void);
static int ppmd_drv_replace_level_2_interrupts(void);
static int ppmd_drv_restore_level_2_interrupts(void);
static int ppmd_drv_replace_client_settings(void);
static int ppmd_drv_restore_client_settings(void);
static int ppmd_drv_replace_leds(void);
static int ppmd_drv_restore_leds(void);
static int ppmd_drv_program_upg_timer(void);
static void enable_user_inputs(void);
static void disable_user_inputs(void);
static int ppmd_drv_standby(void);
static int ppmd_drv_relocate_handler(void);
static int ppmd_drv_lock_handler(void);
static int ppmd_drv_unlock_handler(void);
static void ppmd_drv_setup_pinmux(void);
static void ppmd_drv_standby_gpio(void);
static void ppmd_drv_power_on_gpio(void);
static void ppmd_drv_standby_i2c(void);
static void ppmd_drv_power_on_i2c(void);

static int ppmd_drv_open (struct inode *inode, struct file *file);
static int ppmd_drv_release (struct inode *inode, struct file *file);
static int ppmd_drv_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

/* external routines */
extern unsigned long asm_wait_routine(int memory_shutdown);

/* structure declaration */

static struct file_operations ppmd_drv_fops = {
    .owner = THIS_MODULE,
    .ioctl = ppmd_drv_ioctl,
    .open = ppmd_drv_open,
    .release = ppmd_drv_release,
};

/* global variables */
/* TODO - Change structure for these */
BREG_Handle     g_ppmd_reg_handle;
BCHP_Handle     g_ppmd_chip_handle;
BINT_Handle     g_ppmd_int_handle;
BTMR_Handle         g_ppmd_timer_handle;
BTMR_TimerHandle    g_ppmd_timer;
BKPD_Handle     g_ppmd_keypad_handle;
BKIR_Handle     g_ppmd_ir_remote_handle;
BKIR_ChannelHandle  g_ppmd_ir_channel_handle;
BGIO_Handle     g_ppmd_gio_handle;
BLED_Handle     g_ppmd_led_handle;
int             g_ppmd_device;
void *          g_ppmd_low_alloc;
int             g_ppmd_low_alloc_size;
PPMD_IOCTL      g_ppmd_active;
DECLARE_MUTEX(      g_ppmd_standby_lock);
unsigned long       g_ppmd_flags;
int             g_ppmd_irq;
unsigned long       g_ppmd_saved_status;
unsigned long       g_ppmd_interrupt_vector;
unsigned long       g_ppmd_saved_exception_area[0x100];
unsigned long       g_ppmd_saved_l1_mask[4];
unsigned long       g_ppmd_saved_l2_mask[4];
unsigned long       g_ppmd_saved_clkgen[4];
unsigned long       g_ppmd_saved_rac_config;
#define L2_LDK_CONTROL 0
#define L2_KBD1_CMD 1
#define L2_UPG_CPU_INTR 2

uint8_t         g_ppmd_saved_leds[4];
struct timeval      g_ppmd_upg_timer_duration;
unsigned long       g_ppmd_client[REFRESH_CLIENT];
int             g_ppmd_icache_line_size;
struct timespec     g_ppmd_curr_time;
unsigned long       g_ppmd_wktmr_sec;
unsigned long       g_ppmd_wktmr_pre;

#if defined(BCHP_HIF_CPU_INTR1_INTR_W2_STATUS)
#define L1_IRQS     96
#elif defined(BCHP_HIF_CPU_INTR1_INTR_W1_STATUS)
#define L1_IRQS     64
#endif

#define WKTMR_1S        (27000000)
#define WKTMR_1MS       (WKTMR_1S / 1000)
#define VGA_SAMPLE_PERIOD   (250 * WKTMR_1MS)
#define FREQ(x)         ((x) * WKTMR_1S / VGA_SAMPLE_PERIOD)

#ifdef HDMI_CEC_FEATURE 
unsigned long org_reg1,org_reg2,org_reg3;
#define BCHP_MASK_DVP(Register,Field) \
	BCHP_HDMI_RX_FE_0_##Register##_##Field##_MASK

#define HDMI_CEC_IMAGE_VIEW_ON                  0x04
#define HDMI_CEC_TEXT_VIEW_ON                   0x0D
#define HDMI_CEC_GIVE_DEV_PWR_STATUS            0x8F
#define HDMI_CEC_REPORT_DEV_PWR_STATUS          0x90
#define CEC_STATUS_STANDBY                      (1L)		
#define CEC_TX_WAIT_COUNT                       200 
#endif
struct wktmr_time {
    uint32_t        sec;
    uint32_t        pre;
};

/****************************************************************
* asm_sync
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     MIPS sync command
*
****************************************************************/
static inline void asm_sync()
{
        __asm__(".set\tmips3\n\t"
                "sync\n\t"
                ".set\tmips0" : : : "memory");
}

/****************************************************************
* dummy_tmr_callback
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     dummy rouutine for TMR callback
*
****************************************************************/
static void dummy_tmr_callback(void)
{

}

/****************************************************************
* alarm_time_ok
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      1 if time is ok or not used; 0 if time is not good
* FUNCTION:     Tests for valid alarm in the future must also be
* equal to or greater than our UPG Timer rate (2 seconds)
*
****************************************************************/
static int alarm_time_ok(void)
{
    /* Don't care if this is not a timed request */
    if((g_ppmd_active.events & PPMD_IOCTL_EVENT_TIMEOUT)==0) return 1;

    /* Must have a positive number of secs */
    if(g_ppmd_active.end_time.tv_sec>=g_ppmd_upg_timer_duration.tv_sec) return 1;

    /* Time is bad :(*/
    return 0;
}

/****************************************************************
* upg_timer_duration_ok
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      1 if duration is ok or not set; 0 if duration is not good
* FUNCTION:     Tests for valid UPG timer duration, must be in seconds
* and greater than 0 and less than 40. Once set, it will remain in affect
* on all subsequent calls. If never set, default is 2 seconds.
*
****************************************************************/
static int upg_timer_duration_ok(void)
{
    /* Don't care if we are not setting timer duration */
    if((g_ppmd_active.flags&PPMD_IOCTL_FLAGS_SET_UPG_TIMER_DURATION)==0) return 1;

    /* Must have a positive number of secs */
    if(g_ppmd_active.upg_timer_duration>0 && g_ppmd_active.upg_timer_duration<=PPMD_IOCTL_MAX_UPG_DURATION)
    {
        g_ppmd_upg_timer_duration.tv_sec = g_ppmd_active.upg_timer_duration;
        g_ppmd_upg_timer_duration.tv_usec = 0;
        return 1;
    }

    /* Duration is bad :(*/
    return 0;

}

#if VGA_INPUT

#if BUGGY_WKTMR

/* use UPG TIMER1 instead of WKTMR */

static inline void wktmr_init(void)
{
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0);
    BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL);

    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0xbfffffff);
    BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL);
}

static inline void wktmr_read(struct wktmr_time *t)
{
    t->sec = 0;
    t->pre = BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_STAT) &
        0x3fffffff;
}
#else
static inline void wktmr_init(void)
{
}

static inline void wktmr_read(struct wktmr_time *t)
{
    t->sec = BREG_Read32(g_ppmd_reg_handle, BCHP_WKTMR_COUNTER);
    t->pre = 26999999 - BREG_Read32(g_ppmd_reg_handle,
        BCHP_WKTMR_PRESCALER_VAL);
}
#endif /* BUGGY_WKTMR */

static inline unsigned long wktmr_diff(struct wktmr_time *t0,
    struct wktmr_time *t1)
{
    struct wktmr_time tmp = *t1;

    if(tmp.pre < t0->pre) {
        tmp.pre += 27000000;
        tmp.sec--;
    }
    return(((tmp.sec - t0->sec) * 27000000UL) + (tmp.pre - t0->pre));
}

/****************************************************************
* vga_clear_irq
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Set CG_INTR2 IRQ polarities so that the current levels
*               do not trigger an interrupt.  Clear any pending VGA
*               interrupts.
*
****************************************************************/
static void vga_clear_irq(void)
{
    unsigned long reg;

    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR,
        VGA_HSYNC_IRQ_MASK | VGA_VSYNC_IRQ_MASK);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR);

    reg = BREG_Read32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV);

    if(BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_STATUS) &
        VGA_HSYNC_IRQ_MASK)
        reg ^= VGA_HSYNC_IRQ_MASK;

    if(BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_STATUS) &
        VGA_VSYNC_IRQ_MASK)
        reg ^= VGA_VSYNC_IRQ_MASK;
    
    BREG_Write32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV, reg);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV);

    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR,
        VGA_HSYNC_IRQ_MASK | VGA_VSYNC_IRQ_MASK);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR);
}

/****************************************************************
* vga_set_irqmask
*
* INPUTS:       dpms_mode - current detected DPMS mode
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Set up VGA H/V interrupt masks to wake on DPMS mode
*               changes:
*                 SUSPEND -> ON
*                 SUSPEND -> STANDBY
*                 STANDBY -> ON
*                 STANDBY -> SUSPEND
*
****************************************************************/
static void vga_set_irqmask(int dpms_mode)
{
    unsigned long set = 0, clr = 0;

    if((dpms_mode & 1) == 0) {
        /* HSYNC not wiggling, so enable H IRQ */
        clr |= VGA_HSYNC_IRQ_MASK;
    } else {
        set |= VGA_HSYNC_IRQ_MASK;
    }
    if((dpms_mode & 2) == 0) {
        /* VSYNC not wiggling, so enable V IRQ */
        clr |= VGA_VSYNC_IRQ_MASK;
    } else {
        set |= VGA_VSYNC_IRQ_MASK;
    }

    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_CLEAR, clr);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_CLEAR);
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_SET, set);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_SET);
}

/****************************************************************
* vga_dpms_detect
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      Detected DPMS mode (3=ON, 2=STANDBY, 1=SUSPEND, 0=OFF)
* FUNCTION:     Measure frequency/jitter on VGA HSYNC/VSYNC signals, and
*               determine the current DPMS mode based on which of the two
*               signals fall into the valid range.
*
****************************************************************/
static int vga_dpms_detect(void)
{
    struct wktmr_time start, now;
    struct wktmr_time h_last_edge, v_last_edge;
    unsigned long h_last_edge_ticks = 0, v_last_edge_ticks = 0;
    int h_hits = 0, h_misses = 0;
    int v_hits = 0, v_misses = 0;
    int h_freq, v_freq, h_lock = 0, v_lock = 0;
    int new_dpms_mode;
    unsigned long ticks;
    int delta;
    int hpol = 0, vpol = 0;
    unsigned long inv;

    vga_clear_irq();

    wktmr_init();
    wktmr_read(&start);
    h_last_edge = v_last_edge = start;

    inv = BREG_Read32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV);

    /* initial setting: active low */
    inv |= VGA_HSYNC_IRQ_MASK | VGA_VSYNC_IRQ_MASK;

    BREG_Write32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV, inv);
    BREG_Read32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV);

    do {
        uint32_t hsync = 0, vsync = 0, stat, clr = 0;

        wktmr_read(&now);

        stat = BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_STATUS);

        if(stat & VGA_HSYNC_IRQ_MASK) {
            hsync = hpol;
            hpol ^= 1;
            inv ^= VGA_HSYNC_IRQ_MASK;
            clr |= VGA_HSYNC_IRQ_MASK;
        }
        if(stat & VGA_VSYNC_IRQ_MASK) {
            vsync = vpol;
            vpol ^= 1;
            inv ^= VGA_VSYNC_IRQ_MASK;
            clr |= VGA_VSYNC_IRQ_MASK;
        }

        if(clr) {
            BREG_Write32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV,
                inv);
            BREG_Read32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV);

            BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR,
                clr);
            BREG_Read32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR);
        }

        if(hsync) {
            /* found HSYNC falling edge */

            ticks = wktmr_diff(&h_last_edge, &now);
            h_last_edge = now;

            delta = h_last_edge_ticks - ticks;
            if(delta < 0)
                delta = -delta;
            if((unsigned int)delta * 3 < h_last_edge_ticks)
                h_hits++;
            else
                h_misses++;
            h_last_edge_ticks = ticks;
        }

        if(vsync) {
            /* found VSYNC falling edge */

            ticks = wktmr_diff(&v_last_edge, &now);
            v_last_edge = now;

            delta = v_last_edge_ticks - ticks;
            if(delta < 0)
                delta = -delta;
            if((unsigned int)delta * 10 < v_last_edge_ticks)
                v_hits++;
            else
                v_misses++;
            v_last_edge_ticks = ticks;
        }
    } while(wktmr_diff(&start, &now) < VGA_SAMPLE_PERIOD);

    h_freq = FREQ(h_hits + h_misses);
    v_freq = FREQ(v_hits + v_misses);

    /* 10kHz < HFREQ <= 91100, and hits/total >= 20/32 */
    if((h_freq > 10000) && (h_freq <= 91100) &&
       (h_hits * 32 / 20 >= (h_hits + h_misses)))
        h_lock = 1;
    /* 45Hz < VFREQ <= 85Hz, and hits/total >= 6/8 */
    if((v_freq > 45) && (v_freq <= 85) &&
       (v_hits * 8 / 6 >= (v_hits + v_misses)))
        v_lock = 1;
    
    new_dpms_mode = h_lock | (v_lock << 1);

    BDBG_MSG(("%s: new DPMS mode %d\n", __FUNCTION__, new_dpms_mode));
    BDBG_MSG(("h_hits: %d, h_misses: %d - %d Hz %s\n", h_hits, h_misses,
        h_freq, h_lock ? "LOCKED" : "not locked"));
    BDBG_MSG(("v_hits: %d, v_misses: %d - %d Hz %s\n", v_hits, v_misses,
        v_freq, v_lock ? "LOCKED" : "not locked"));

    return(new_dpms_mode);
}

#endif

/****************************************************************
* test_for_gpio_wakeup
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      WAKEUP (1) if gpio event hit; 0 if not the right gpio event
* or we are not waiting for an gpio event.
* FUNCTION:     Tests if we are waiting for an gpio event and
* if we are, tests if the event is the correct event, return WAKEUP
* if both conditions are met.
*
****************************************************************/
static int test_for_gpio_wakeup(void)
{
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_GPIO) {
        PPMD_L2_CLEAR(g_ppmd_reg_handle, PPMD_L2_MASK_GPIO);
        return WAKEUP;
    }

    return 0;
}

#if LDK_FP
/****************************************************************
* test_for_keypad_wakeup
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      WAKEUP (1) if front panel event hit; 0 if not the right front panel event
* or we are not waiting for an front panel event.
* FUNCTION:     Tests if we are waiting for an front panel event and
* if we are, tests if the event is the correct event, return WAKEUP
* if both conditions are met.
*
****************************************************************/
static int test_for_keypad_wakeup(void)
{
    bool keyReady;
    uint16_t key;

    if(!g_ppmd_active.events & PPMD_IOCTL_EVENT_KEYPAD) return 0;

    BKPD_Read (g_ppmd_keypad_handle, &key);
    PPMD_L2_CLEAR(g_ppmd_reg_handle, PPMD_L2_MASK_KPD);

    BDBG_MSG(("\n%s: LDK keypad data read (interrupt cleared) = %x",
            __FUNCTION__,key));

    if((g_ppmd_active.flags & PPMD_IOCTL_FLAGS_POWER_BUTTON_ONLY) &&
       (key != KEYPAD_POWER_BUTTON))
        return(0);
    
    return(WAKEUP);
}
#endif

/****************************************************************
* test_for_ir_remote_wakeup
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      WAKEUP (1) if IR event hit; 0 if not the right IR event
* or we are not waiting for an IR event.
* FUNCTION:     Tests if we are waiting for an IR remote event and
* if we are, tests if the event is the correct event, return WAKEUP
* if both conditions are met.
*
****************************************************************/
static int test_for_ir_remote_wakeup(void)
{
    bool readyFlag = false;
    unsigned char data;
    BKIR_KirInterruptDevice interruptDevice;

    if(!g_ppmd_active.events & PPMD_IOCTL_EVENT_IR) return 0;

    BKIR_Read (g_ppmd_ir_channel_handle, &interruptDevice, &data);
    PPMD_L2_CLEAR(g_ppmd_reg_handle, PPMD_L2_MASK_IR);

    BDBG_MSG(("\n%s: IR data read (interrupt cleared) = %x",
            __FUNCTION__,data));

    if((g_ppmd_active.flags & PPMD_IOCTL_FLAGS_POWER_BUTTON_ONLY) &&
       (data != IR_REMOTE_A_POWER_BUTTON))
        return(0);

    BDBG_MSG(("\nwakeup\n"));
    return WAKEUP;
}

#if VGA_INPUT
static int test_for_vga_wakeup(void)
{
    int dpms_mode;

    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_VGA) {
        dpms_mode = vga_dpms_detect();
        vga_set_irqmask(dpms_mode);

        if(dpms_mode >= VGA_WAKEUP_MODE)
            return(WAKEUP);
    }
    vga_clear_irq();

    return 0;
}
#endif

#ifdef HDMI_CEC_FEATURE 
static int test_for_cec_wakeup(void)
{
	 long reg;
	 int  retvalue = 0;
	 
	 reg = BREG_Read32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_STATUS);
	 if( reg & BCHP_DVP_HR_INTR2_CPU_STATUS_CEC_0_MASK )
	 {
	 	   	     // Clear interrupt 
	     reg = BCHP_DVP_HR_INTR2_CPU_STATUS_CEC_0_MASK;
	 	   BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_CLEAR, reg) ;	  
	     reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_5);

	     if( reg & BCHP_MASK_DVP(CEC_CNTRL_5, RX_CEC_INT) )
	     {
		      uint8_t i, j ;
		      uint8_t RxCecWordCount ;
		      uint32_t RegisterOffset ;
		      uint8_t CecBufferedMsg[24];
		      uint8_t CecBufferedMsgLength = 0;
		      
	     		reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
		      RxCecWordCount = BCHP_GET_FIELD_DATA(reg, HDMI_RX_FE_0_CEC_CNTRL_1, REC_WRD_CNT) ;
		      RxCecWordCount++ ;
		      BDBG_MSG(("\nKKKK BHDR_FE_P_CEC_isr receive cnt: %d\n",RxCecWordCount));
		      CecBufferedMsgLength = RxCecWordCount;
		      /* get the received words and place into the buffer */
		      RegisterOffset = 0 ;
		      for (i = 0 ; i < RxCecWordCount ; i = i + 4)
		      {
			        reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_RX_DATA_1 + RegisterOffset) ;
			        for (j = 0 ; j + i < RxCecWordCount; j++)
				        CecBufferedMsg[i+j] = reg  >> (8 * j) & 0xFF ;
			        RegisterOffset = RegisterOffset + 4 ;
		      }
		      for (i = 0 ; i <= CecBufferedMsgLength; i++)
			      BDBG_MSG(("CEC RxByte[%02d] = %02X ", i, CecBufferedMsg[i])) ;
			
	        BDBG_MSG(("\n"));
	        reg = BREG_Read32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL ) ;
	        reg |= BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	        BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ;
	        reg &= ~BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	        BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ;  
	 	      
	 	      // wakeup
	 	      if( (CecBufferedMsgLength>=2) &&
	 	      	 (CecBufferedMsg[1] == HDMI_CEC_IMAGE_VIEW_ON || CecBufferedMsg[1] == HDMI_CEC_TEXT_VIEW_ON ))  // view on recieved
	 	      {
	            g_ppmd_active.cec_addr = CecBufferedMsg[0] >>4;
	            g_ppmd_active.cec_cmd = CecBufferedMsg[1];
	            retvalue = WAKEUP;
	        }     
	        else if( (CecBufferedMsgLength>=2) &&
	 	      	 (CecBufferedMsg[1] == HDMI_CEC_GIVE_DEV_PWR_STATUS))
	        {
	        	  uint32_t max_try;
	        	  
	        	  // Construct send data
              reg = CecBufferedMsg[0] >>4;
              reg |= (((uint32_t)HDMI_CEC_REPORT_DEV_PWR_STATUS) <<8);
              reg |= (CEC_STATUS_STANDBY <<16);
	            BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_TX_DATA_1, reg) ;
	         
	            // Set response message length
	            reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
	            reg &= ~BCHP_MASK_DVP(CEC_CNTRL_1, MESSAGE_LENGTH) ;
	            reg |= BCHP_FIELD_DATA(HDMI_RX_FE_0_CEC_CNTRL_1, MESSAGE_LENGTH, 2) ;
	            BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;
	           
	            // Write a pulse to start transmit
	            reg &=  ~ BCHP_MASK_DVP(CEC_CNTRL_1, START_XMIT_BEGIN) ;
	            BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;   
	            reg |= BCHP_FIELD_DATA(HDMI_RX_FE_0_CEC_CNTRL_1, START_XMIT_BEGIN, 1) ;
	            BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;  
	             
	            // wait for maximum 2 seconds till transmit finish 
	            max_try = CEC_TX_WAIT_COUNT;
	            while(max_try)
	            {
	            	max_try--;
	              reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
	              if( reg & BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_TX_EOM_MASK )
	              	break;
	              BKNI_Delay(10);
	            }

              //clear previous tx pending interrupt
	            reg = BREG_Read32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL ) ;
	            reg |= BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	            BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ;
	            reg &= ~BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, CEC_CLR_INTERRUPT_DET) ;
	            BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ;  
	        } 
      		// Enable reciever
	        reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
	        reg |= BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_CLEAR_RECEIVE_OFF_MASK ;
	        BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;  
	        reg &= ~BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_CLEAR_RECEIVE_OFF_MASK ;
	        BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;  

	     }
   
	 }
	 return retvalue;
}
#endif

/****************************************************************
* test_for_memory_self_refresh
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      1 if memory self refresh was requested; 0 if not
* or we are not waiting for an IR event.
* FUNCTION:     Tests if memory self refresh was requested
*
****************************************************************/
static int test_for_memory_self_refresh(void)
{
    if(g_ppmd_active.flags&PPMD_IOCTL_FLAGS_SELF_REFRESH_W_PADS)
        return(PPMD_KILL_PADS);
    if(g_ppmd_active.flags&PPMD_IOCTL_FLAGS_MEMORY_SELF_REFRESH)
        return(PPMD_SELF_REFRESH_ONLY);
    return(PPMD_NO_SHUTDOWN);
}

/****************************************************************
* ppmd_drv_set_module_debuglevel
*
* INPUTS:       addr - cache line address
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Tells the debug module which modules to debug.
*
****************************************************************/
static void ppmd_drv_set_module_debuglevel(const char *modulelist, BDBG_Level level)
{

        while (modulelist && *modulelist) {
                const char *end = strchr(modulelist, ',');
                if (!end) {
                        BDBG_SetModuleLevel(modulelist, level);
                        break;
                }
                else {
                        int n = end-modulelist;
                        char *buf = (char *)BKNI_Malloc(n+1);
                        /* NOTE: memory leak, but this is debug */
                        strncpy(buf, modulelist, n);
                        buf[n] = 0;
                        BDBG_SetModuleLevel(buf, level);
			BKNI_Free(buf);
                }
                modulelist = ++end;
        }
}

#if ! HW_STANDBY

/****************************************************************
* ppmd_drv_adjust_curr_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Add UPG timer1 duration to the system time
****************************************************************/
static void ppmd_drv_adjust_curr_time(void)
{
    unsigned long long ticks, new_nsec, nsec, sec;

    ticks = BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_STAT) & 0x3fffffff;

    /* reset TIMER1 */
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0);
    BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL);
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0xbfffffff);

    /* figure out how many nsecs have passed, then add to system time */
    nsec = ticks * 1000000000ULL;
    do_div(nsec, XTALFREQ);
    sec = new_nsec = (unsigned long long)g_ppmd_curr_time.tv_nsec + nsec;
    do_div(sec, 1000000000);
    g_ppmd_curr_time.tv_sec += sec;
    g_ppmd_curr_time.tv_nsec = new_nsec - (sec * 1000000000);
}

/****************************************************************
* ppmd_drv_set_curr_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Sets our global variable with the current system time
****************************************************************/
static void ppmd_drv_set_curr_time()
{
    g_ppmd_curr_time = current_kernel_time();
}

/****************************************************************
* ppmd_drv_adjust_kernel_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Set the kernel time based on our version of the current
* time which is maintained via the UPG timer above.
****************************************************************/
static void ppmd_drv_adjust_kernel_time()
{
    struct timespec tv;

    tv = current_kernel_time();
    tv.tv_sec = g_ppmd_curr_time.tv_sec;
    tv.tv_nsec = g_ppmd_curr_time.tv_nsec;
    do_settimeofday(&tv);
}

/****************************************************************
* ppmd_drv_open_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      BERR_SUCCESS on success, anything else on failure
* FUNCTION: Initialize the UPG timer
*
****************************************************************/
static int ppmd_drv_open_timer(void)
{
    BTMR_DefaultSettings tmr_settings;
    BTMR_Settings timer_settings = { BTMR_Type_eCountDown, (BTMR_CallbackFunc)dummy_tmr_callback, NULL, 0, false };
    int rc;

    /* Setup the BTMR */
    BTMR_GetDefaultSettings(&tmr_settings);

    /* Make any changes required from the default values */
    tmr_settings.timerMask = 0; /* defines the timers to reserve for external use (none) */

    rc = BTMR_Open(&g_ppmd_timer_handle, g_ppmd_chip_handle,
        g_ppmd_reg_handle, g_ppmd_int_handle, &tmr_settings);
    if(rc != BERR_SUCCESS)
        return(rc);

    // Create a timer.
    rc = BTMR_CreateTimer(g_ppmd_timer_handle, &g_ppmd_timer, &timer_settings);
    if(rc != BERR_SUCCESS)
        return(rc);

    /* Default UPG timer duration */
    g_ppmd_upg_timer_duration.tv_sec = PPMD_IOCTL_DEFAULT_UPG_DURATION;
    g_ppmd_upg_timer_duration.tv_usec = 0;

    return(BERR_SUCCESS);
}

/****************************************************************
* ppmd_drv_close_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Deinitialize the UPG timer
*
****************************************************************/
static void ppmd_drv_close_timer(void)
{
    BTMR_DestroyTimer(g_ppmd_timer);
    BTMR_Close(g_ppmd_timer_handle);
}

/****************************************************************
* ppmd_drv_rearm_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Rearms our UPG timer for the next interrupt
*
****************************************************************/
static void ppmd_drv_rearm_timer(void)
{
    unsigned int usec = g_ppmd_upg_timer_duration.tv_sec * 1000000 + g_ppmd_upg_timer_duration.tv_usec;
    BTMR_StopTimer_isr(g_ppmd_timer);
    BTMR_StartTimer_isr(g_ppmd_timer,usec);
}

/****************************************************************
* ppmd_drv_stop_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Rearms our UPG timer for the next interrupt
*
****************************************************************/
static void ppmd_drv_stop_timer(void)
{
    BTMR_StopTimer_isr(g_ppmd_timer);
}

/****************************************************************
* ppmd_drv_reprogram_upg_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION: Reprograms the UPG timer after it has expired
*
****************************************************************/
static int ppmd_drv_reprogram_upg_timer(void)
{

    unsigned int usec = g_ppmd_upg_timer_duration.tv_sec * 1000000 +
        g_ppmd_upg_timer_duration.tv_usec;

    // wake up every TV_SEC seconds
    BTMR_StartTimer_isr(g_ppmd_timer, usec);
    return(0);
}

/****************************************************************
* ppmd_drv_program_upg_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION: Programs upg timer to fire at a fixed interval
*
****************************************************************/
static int ppmd_drv_program_upg_timer(void)
{
    struct timeval tv;

    /* Offset time of day here for more accuracy */
    do_gettimeofday(&tv);
    g_ppmd_active.end_time.tv_sec += tv.tv_sec;
    g_ppmd_active.end_time.tv_usec += tv.tv_usec;
    BDBG_MSG(("\n%s: Current time %d (seconds) End time %d (seconds)",
        __FUNCTION__,
        tv.tv_sec,
        g_ppmd_active.end_time.tv_sec));

    return(ppmd_drv_reprogram_upg_timer());
}

/****************************************************************
* ppmd_drv_timer_expired
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - expired, 1 - did not expire
* FUNCTION: Checks to see if the timer expired, and clears the irq
*
****************************************************************/
static int ppmd_drv_timer_expired(void)
{
    if(BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER_IS) &
        BCHP_TIMER_TIMER_IE0_TMR0TO_MASK)
    {
        BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER_IS,
            BCHP_TIMER_TIMER_IE0_TMR0TO_MASK);
        return(1);
    }
    return(0);
}

/****************************************************************
* ppmd_drv_walltimer_start
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Starts the wall-clock timer
*
****************************************************************/
static void ppmd_drv_walltimer_start(void)
{
    /* start timer 1 in free-running mode to track wall-clock time */
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER_IE0,
        BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER_IE0) &
        ~BCHP_TIMER_TIMER_IE0_TMR1TO_MASK);

    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0);
    BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL);
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0xbfffffff);
}

/****************************************************************
* ppmd_drv_walltimer_stop
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Stops the wall-clock timer
*
****************************************************************/
static void ppmd_drv_walltimer_stop(void)
{
    /* stop timer 1 */
    BREG_Write32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL, 0);
    BREG_Read32(g_ppmd_reg_handle, BCHP_TIMER_TIMER1_CTRL);
}

/****************************************************************
* alarm_time_reached
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      1 if alarm has fired; 0 if not yet or we are not waiting
* for an alarm.
* FUNCTION:     Tests if we are wating for an alarm and then if we
* are, tests if it has been reached.
*
****************************************************************/
static int alarm_time_reached(void)
{
    struct timeval tv;
    int time_remaining;

    if((g_ppmd_active.events & PPMD_IOCTL_EVENT_TIMEOUT)==0) return 0;

    BDBG_MSG(("\n%s: Compare current time %d (seconds) to end time %d (seconds)",
        __FUNCTION__,
        g_ppmd_curr_time.tv_sec,
        g_ppmd_active.end_time.tv_sec
              ));
    time_remaining = g_ppmd_active.end_time.tv_sec-g_ppmd_curr_time.tv_sec;
    if(time_remaining>0) 
    {
        /* Adjust timer duration if remaining time is smaller */
        if(time_remaining<g_ppmd_upg_timer_duration.tv_sec)
            g_ppmd_upg_timer_duration.tv_sec = time_remaining;
        return 0;
    }
    return 1;
}

#else /* ! HW_STANDBY */

/****************************************************************
* ppmd_drv_adjust_curr_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.  WKTMR keeps track of the system time.
****************************************************************/
static void ppmd_drv_adjust_curr_time(void)
{
}

/****************************************************************
* ppmd_drv_set_curr_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
****************************************************************/
static void ppmd_drv_set_curr_time()
{
}

/****************************************************************
* ppmd_drv_adjust_kernel_time
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
****************************************************************/
static void ppmd_drv_adjust_kernel_time()
{
#ifdef BCHP_WKTMR_PRESCALER_VAL     // missing on 3548a0
    struct timespec tv = g_ppmd_curr_time;
    unsigned long sec, pre;

    sec = BREG_Read32(g_ppmd_reg_handle, BCHP_WKTMR_COUNTER);
    pre = WKTMR_FREQ - (BREG_Read32(g_ppmd_reg_handle,
        BCHP_WKTMR_PRESCALER_VAL) + 1);

    sec -= g_ppmd_wktmr_sec;
    if(g_ppmd_wktmr_pre > pre) {
        sec--;
        pre += WKTMR_FREQ - g_ppmd_wktmr_pre;
    } else {
        pre -= g_ppmd_wktmr_pre;
    }

    tv.tv_nsec += pre * (1000000000 / WKTMR_FREQ);
    tv.tv_sec += sec;
    while(tv.tv_nsec > 1000000000) {
        tv.tv_sec++;
        tv.tv_nsec -= 1000000000;
    }

    do_settimeofday(&tv);
#endif
}

/****************************************************************
* ppmd_drv_open_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      BERR_SUCCESS on success, anything else on failure
* FUNCTION: No-op.
*
****************************************************************/
static int ppmd_drv_open_timer(void)
{
    return(BERR_SUCCESS);
}

/****************************************************************
* ppmd_drv_close_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
*
****************************************************************/
static void ppmd_drv_close_timer(void)
{
}

/****************************************************************
* ppmd_drv_rearm_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
*
****************************************************************/
static void ppmd_drv_rearm_timer(void)
{
}

/****************************************************************
* ppmd_drv_stop_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
*
****************************************************************/
static void ppmd_drv_stop_timer(void)
{
}

/****************************************************************
* ppmd_drv_reprogram_upg_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION: No-op.
*
****************************************************************/
static int ppmd_drv_reprogram_upg_timer(void)
{
    return(0);
}

/****************************************************************
* ppmd_drv_program_upg_timer
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION: No-op.
*
****************************************************************/
static int ppmd_drv_program_upg_timer(void)
{
    if((g_ppmd_active.events & PPMD_IOCTL_EVENT_TIMEOUT)==0)
        return(0);

    /* set wktmr alarm to current time + TV_SEC seconds */
    BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_ALARM,
        BREG_Read32(g_ppmd_reg_handle, BCHP_WKTMR_COUNTER) +
        g_ppmd_active.end_time.tv_sec);

    /* enable WKTMR in PM L2 */
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_CLEAR,
        BCHP_CG_INTR2_CPU_STATUS_RTC_INTR_MASK);
    return(0);
}

/****************************************************************
* ppmd_drv_timer_expired
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - expired, 1 - did not expire
* FUNCTION: Checks to see if the timer expired, and clears the irq
*
****************************************************************/
static int ppmd_drv_timer_expired(void)
{
    if(BREG_Read32(g_ppmd_reg_handle, BCHP_WKTMR_EVENT) &
        BCHP_WKTMR_EVENT_wktmr_alarm_event_MASK)
    {
        BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_EVENT,
            BCHP_WKTMR_EVENT_wktmr_alarm_event_MASK);
        return(1);
    }
    return(0);
}

/****************************************************************
* ppmd_drv_walltimer_start
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
*
****************************************************************/
static void ppmd_drv_walltimer_start(void)
{
#ifdef BCHP_WKTMR_PRESCALER_VAL     // missing on 3548a0
    g_ppmd_wktmr_sec = BREG_Read32(g_ppmd_reg_handle, BCHP_WKTMR_COUNTER);
    g_ppmd_wktmr_pre = WKTMR_FREQ - (BREG_Read32(g_ppmd_reg_handle,
        BCHP_WKTMR_PRESCALER_VAL) + 1);
    g_ppmd_curr_time = current_kernel_time();
#endif
}

/****************************************************************
* ppmd_drv_walltimer_stop
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: No-op.
*
****************************************************************/
static void ppmd_drv_walltimer_stop(void)
{
}

/****************************************************************
* alarm_time_reached
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      1 - Timer has fired
* for an alarm.
* FUNCTION: No-op.  If the WKTMR alarm fired, this is always true.
****************************************************************/
static int alarm_time_reached(void)
{
    return(1);
}

/****************************************************************
* ppmd_drv_init_hw_standby
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Disables clocks, sets standby parameters, etc.
*
****************************************************************/
static void ppmd_drv_init_hw_standby(void)
{
    /* clear any pending alarm */
    BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_ALARM, 0);
    BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_EVENT, 1);

    /* set up PM L2 interrupts */
    BREG_Write32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_IRQ_INV, 0);
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_SET, 0xffffffff);
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR, 0xffffffff);

    /*
     * power-down settings:
     * 0x1e0 = power down everything except wordslice DLL
     * 0x1950 = 216 ticks/us * 30 us
     * WAIT_CNT = wait N ticks before putting the MIPS to sleep
     */
    BREG_Write32(g_ppmd_reg_handle, BCHP_DDR_IOBUF_SPARE0_RW,
        (BREG_Read32(g_ppmd_reg_handle, BCHP_DDR_IOBUF_SPARE0_RW)
            & 0xc1f) | (0x1950 << 12) | 0x1e0);
    BREG_Write32(g_ppmd_reg_handle, BCHP_CLKGEN_PM_WAIT_CNT, 0x0);

    /* keep IRR going even after we cut the UPG 27Mhz clock */
    BREG_Write32(g_ppmd_reg_handle, BCHP_PM_CONFIG,
        BREG_Read32(g_ppmd_reg_handle, BCHP_PM_CONFIG) |
        BCHP_PM_CONFIG_disable_irr_clk_shutoff_MASK);

    g_ppmd_saved_clkgen[3] = BREG_Read32(g_ppmd_reg_handle,
        BCHP_CLKGEN_PWRDN_CTRL_3);
    g_ppmd_saved_clkgen[0] = BREG_Read32(g_ppmd_reg_handle,
        BCHP_CLKGEN_PWRDN_CTRL_0);

    /* power down AVD, RPT */
    BREG_AtomicUpdate32_isr(g_ppmd_reg_handle, BCHP_CLKGEN_PWRDN_CTRL_3,
        0, 0x800001);
    BREG_AtomicUpdate32_isr(g_ppmd_reg_handle, BCHP_CLKGEN_PWRDN_CTRL_0,
        0, 0x4002);
}

/****************************************************************
* ppmd_drv_cleanup_hw_standby
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Re-enables clocks and cleans up after HW standby
*
****************************************************************/
static void ppmd_drv_cleanup_hw_standby(void)
{
    /* clear any pending alarm */
    BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_ALARM, 0);
    BREG_Write32(g_ppmd_reg_handle, BCHP_WKTMR_EVENT, 1);

    /* disable PM L2 interrupts */
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_MASK_SET, 0xffffffff);
    BREG_Write32(g_ppmd_reg_handle, BCHP_CG_INTR2_CPU_CLEAR, 0xffffffff);

    /* restore CLKGEN settings */
    BREG_AtomicUpdate32_isr(g_ppmd_reg_handle, BCHP_CLKGEN_PWRDN_CTRL_0,
        0x4002, g_ppmd_saved_clkgen[0] & 0x4002);
    BREG_AtomicUpdate32_isr(g_ppmd_reg_handle, BCHP_CLKGEN_PWRDN_CTRL_3,
        0x800001, g_ppmd_saved_clkgen[3] & 0x800001);
}

#endif /* ! HW_STANDBY */

/****************************************************************
* ppmd_drv_disable_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Disables local interrupts
*
****************************************************************/
static void ppmd_drv_disable_interrupts(void)
{
    /* Save interrupt mask */
    g_ppmd_saved_status = read_c0_status();

        local_save_flags(g_ppmd_flags);
        local_irq_disable();

    ppmd_drv_walltimer_start();
}

/****************************************************************
* ppmd_drv_enable_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION:     Enables local interrupts
*
****************************************************************/
static void ppmd_drv_enable_interrupts()
{
    ppmd_drv_walltimer_stop();

    /* Restore interrupt mask */
    clear_c0_status(ST0_IM);
    irq_disable_hazard(); 
    set_c0_status(g_ppmd_saved_status&ST0_IM);
    irq_disable_hazard(); 

        local_irq_restore(g_ppmd_flags);
}

/****************************************************************
* ppmd_drv_replace_exception_vectors
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Saves the MIPS special exception handler
* and replaces it with our version.
*
****************************************************************/
static int ppmd_drv_replace_exception_vectors()
{
    int i;
    unsigned long *base;
    unsigned long relocated_passiveIRQ = (unsigned long)g_ppmd_low_alloc +
        ((unsigned long)passiveIRQ - (unsigned long)asm_wait_routine);

#if defined(__HAS_EBASE) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30))
    /* newer kernels relocate the exception base for SMP boot (PR44747) */
    g_ppmd_interrupt_vector = ebase;
#else
    /* old kernels are always fixed at 8000_0000 */
    g_ppmd_interrupt_vector = 0x80000000;
#endif
    if((read_c0_config() & (1 << 31)) &&
       (read_c0_config1() & (1 << 31)) &&
       (read_c0_config2() & (1 << 31)) &&
       (read_c0_config3() & (1 << 5))) {
        /* vectored interrupts (MIPS 24k/34k VInt bit is set) */

        /* IntCtl 9:5 = vector spacing */
        g_ppmd_interrupt_vector += 0x200 + 2 * (read_c0_intctl() & 0x3e0);
    } else {
        /* interrupts use the general exception vector on this CPU */
        g_ppmd_interrupt_vector += 0x200;
    }

    base = (void *)g_ppmd_interrupt_vector;

    memcpy(g_ppmd_saved_exception_area,base,0x80);

    /* Put a jump to our routine in the exception vector */
    base[0] = 0x3c1a0000 | (relocated_passiveIRQ >> 16);    // lui k0, nnnn
    base[1] = 0x375a0000 | (relocated_passiveIRQ & 0xffff); // ori k0, mmmm
    base[2] = 0x03400008;                   // jr  k0
    base[3] = 0x00000000;                   // nop

    flush_dcache_line((uint32_t)base);
    flush_icache_line((uint32_t)base);

    return 0;
}

/****************************************************************
* ppmd_drv_restore_exception_vectors
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Restores MIPS special exception handler
*
****************************************************************/
static int ppmd_drv_restore_exception_vectors()
{
    unsigned long *base = (void *)g_ppmd_interrupt_vector;
    unsigned long *area_end = base + 0x20;
    memcpy(base,g_ppmd_saved_exception_area,0x80);
    while (base<area_end) {
        flush_dcache_line((uint32_t)base);
        flush_icache_line((uint32_t)base);
        base += g_ppmd_icache_line_size;
    }
    return 0;
}

/****************************************************************
* ppmd_drv_replace_level_1_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Disables all L1 interrupts except UPG CPU and UPG Timer.
*
****************************************************************/
static int ppmd_drv_replace_level_1_interrupts(void)
{
    /* Save interrupt mask state */

    g_ppmd_saved_l1_mask[0] = BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS);
    g_ppmd_saved_l1_mask[1] = BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS);
#if L1_IRQS > 64
    g_ppmd_saved_l1_mask[2] = BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W2_MASK_STATUS);
#endif

    /* Mask off all interrupts */
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_SET, 0xffffffff);
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET, 0xffffffff);
#if L1_IRQS > 64
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W2_MASK_SET, 0xffffffff);
#endif
    asm_sync();

    /* Enable interrupts we are looking for*/
#if ! HW_STANDBY
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_CLEAR,
        BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_TMR_CPU_INTR_MASK |
        BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_CPU_INTR_MASK);
#else
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR,
        BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_PM_CPU_INTR_MASK);
#endif
    asm_sync();

    BDBG_MSG(("%s: W0 Mask status=%x",__FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS)));
    BDBG_MSG(("%s: W1 Mask status=%x",__FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS)));
    return 0;
}

/****************************************************************
* ppmd_drv_restore_level_1_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Restores all L1 interrupts.
*
****************************************************************/
static int ppmd_drv_restore_level_1_interrupts(void)
{
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_SET,
	g_ppmd_saved_l1_mask[0]);
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_CLEAR,
	~g_ppmd_saved_l1_mask[0]);
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET,
	g_ppmd_saved_l1_mask[1]);
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR,
	~g_ppmd_saved_l1_mask[1]);
#if L1_IRQS > 64
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W2_MASK_SET,
	g_ppmd_saved_l1_mask[2]);
    BREG_Write32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W2_MASK_CLEAR,
	~g_ppmd_saved_l1_mask[2]);
#endif
    asm_sync();
    return 0;
}

/****************************************************************
* ppmd_drv_replace_level_2_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Disables all L2 interrupts related to UPG CPU that
* we are not interested in (i.e. UART).
*
****************************************************************/
static int ppmd_drv_replace_level_2_interrupts(void)
{
    uint32_t val;
    uint16_t hval;

    /* Save L2 control registers that we may need to program */
    g_ppmd_saved_l2_mask[L2_LDK_CONTROL] =
        BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_CONTROL);
    g_ppmd_saved_l2_mask[L2_KBD1_CMD] =
        BREG_Read32(g_ppmd_reg_handle, BCHP_KBD1_CMD);
    g_ppmd_saved_l2_mask[L2_UPG_CPU_INTR] =
        BREG_Read32(g_ppmd_reg_handle, BCHP_IRQ0_IRQEN);

    /* Unset all UPG_CPU interrupts */
    BREG_Write32(g_ppmd_reg_handle, BCHP_IRQ0_IRQEN, 0);

    /* Enable Keypad */
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_KEYPAD)
    {
#if LDK_FP
        BREG_Write32(g_ppmd_reg_handle, 
             BCHP_LDK_CONTROL,
             g_ppmd_saved_l2_mask[L2_LDK_CONTROL] |
             BCHP_LDK_CONTROL_irqen_MASK |
             BCHP_LDK_CONTROL_ver_MASK);
        PPMD_L2_ENABLE(g_ppmd_reg_handle, PPMD_L2_MASK_KPD);
        BDBG_MSG(("%s: Keypad interrupt enabled, irq0_irqen=%x ldk_cntl=%x",
                  __FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_IRQ0_IRQEN),
                  BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_CONTROL)));
#endif
#if SPI_FP
        BREG_Write32(g_ppmd_reg_handle, SPI_IRQ_LEVEL_REG,
            (BREG_Read32(g_ppmd_reg_handle, SPI_IRQ_LEVEL_REG) &
             SPI_IRQ_LEVEL_MASK) | SPI_IRQ_LEVEL_VAL);
        asm_sync();

        /* enable L1 interrupt for ext_irq pin */
        BREG_Write32(g_ppmd_reg_handle, SPI_IRQ_REG_CLEAR,
            SPI_IRQ_MASK);
        asm_sync();
#endif
    }

    /* Enable IR remote */
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_IR)
    {
        // Enable Remote A device
        BKIR_EnableIrDevice (g_ppmd_ir_channel_handle, BKIR_KirDevice_eRemoteA);

        // Clear any pending commands
        BREG_Write32(g_ppmd_reg_handle, BCHP_KBD1_STATUS, 0);

        BREG_Write32(g_ppmd_reg_handle, 
                     BCHP_KBD1_CMD,
                     g_ppmd_saved_l2_mask[L2_KBD1_CMD]|BCHP_KBD1_CMD_kbd_irqen_MASK);
        PPMD_L2_ENABLE(g_ppmd_reg_handle, PPMD_L2_MASK_IR);
        BDBG_MSG(("%s: IR remote interrupt enabled, irq0_irqen=%x",
                  __FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_IRQ0_IRQEN)));
    }

    /* Enable GPIO */
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_GPIO)
    {
        PPMD_L2_ENABLE(g_ppmd_reg_handle, PPMD_L2_MASK_GPIO);
        BDBG_MSG(("%s: GPIO interrupt enabled, irq0_irqen=%x",
                  __FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_IRQ0_IRQEN)));
    }

#if VGA_INPUT
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_VGA)
    {
        vga_clear_irq();
        PPMD_L2_ENABLE(g_ppmd_reg_handle, PPMD_L2_MASK_VGA);
        BDBG_MSG(("%s: VGA H/V interrupts enabled\n", __FUNCTION__));
    }
#endif

#ifdef HDMI_CEC_FEATURE 
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_CEC)
    {
    	  unsigned long reg;

 		    reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 ) ;
 		    org_reg1 = reg;
		    reg &= ~BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, BIAS_PD) ;
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 , reg) ;	
		    /* power up the analog core */
		    reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0) ;
		    org_reg2 = reg;
		    reg &= ~BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN) ;
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0, reg) ;
        
		    reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_RESET_CONTROL ) ;
		    org_reg3 = reg;
		    reg &= ~BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, CEC_POWER_DOWN) ;
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_RESET_CONTROL , reg) ;
        
	      reg = BREG_Read32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL ) ;
	      reg |= BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET) ;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ;
	      
	      reg &= ~BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET) ;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, reg) ; 
	         
	      /* assign the logical Cec Addr (TV default should always be 0)  */
	      reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
	      reg &= ~BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_CEC_ADDR_MASK ;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;	          
        
        /* enable reciever */
	      reg = BREG_Read32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 ) ;
	      reg |= BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_CLEAR_RECEIVE_OFF_MASK ;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;  
	
	      reg &= ~BCHP_HDMI_RX_FE_0_CEC_CNTRL_1_CLEAR_RECEIVE_OFF_MASK ;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_CEC_CNTRL_1 , reg) ;  
	      
	      /* enable interrupt */
	      reg = BREG_Read32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_SET);
	      reg |= BCHP_DVP_HR_INTR2_CPU_STATUS_CEC_0_MASK;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_SET , reg) ;  

	      reg = BCHP_DVP_HR_INTR2_CPU_STATUS_CEC_0_MASK;
	      BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_MASK_CLEAR , reg) ;  
	      BREG_Write32(g_ppmd_reg_handle, BCHP_DVP_HR_INTR2_CPU_CLEAR , reg) ;  	      
	      
        BDBG_MSG(("\n##### enable cec standby #####\n"));
    }
#endif

    return 0;
}

/****************************************************************
* ppmd_drv_restore_level_2_interrupts
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Restores all L2 interrupts.
*
****************************************************************/
static int ppmd_drv_restore_level_2_interrupts(void)
{
    BDBG_MSG(("%s: Restoring L2, irq0_irqstat=%x kdb1_status=%x",
               __FUNCTION__,BREG_Read32(g_ppmd_reg_handle, BCHP_IRQ0_IRQSTAT),
              BREG_Read32(g_ppmd_reg_handle, BCHP_KBD1_STATUS)));
    BREG_Write32(g_ppmd_reg_handle,
        BCHP_LDK_CONTROL,g_ppmd_saved_l2_mask[L2_LDK_CONTROL]);
    BREG_Write32(g_ppmd_reg_handle,
        BCHP_KBD1_CMD,g_ppmd_saved_l2_mask[L2_KBD1_CMD]);
    BREG_Write32(g_ppmd_reg_handle,
        BCHP_IRQ0_IRQEN,g_ppmd_saved_l2_mask[L2_UPG_CPU_INTR]);
#ifdef HDMI_CEC_FEATURE     
    if(g_ppmd_active.events & PPMD_IOCTL_EVENT_CEC)    
    {
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 ,org_reg1) ;	
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0, org_reg2) ;       
		    BREG_Write32(g_ppmd_reg_handle, BCHP_HDMI_RX_FE_0_RESET_CONTROL,org_reg3) ;
		}
#endif                
        
    return 0;
}

/****************************************************************
* ppmd_drv_replace_client_settings
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Saves and replace the client memory settings
*
****************************************************************/

static int ppmd_drv_replace_client_settings(void)
{
    int i, j = 0;
    unsigned long count;
    int excl[] = CLIENT_EXCLUSION_LIST;

    /* block out SCB clients */
    for (i = 0; i < REFRESH_CLIENT; i++) {
        /* Save all client values for restore in next routine */
        g_ppmd_client[i] = BREG_Read32(g_ppmd_reg_handle, SCB_CLIENT_BASE+4*i);

        /* skip any clients in CLIENT_EXCLUSION_LIST (usually MIPS related) */
        if(i == excl[j]) {
            j++;
            continue;
        }
        BREG_Write32(g_ppmd_reg_handle, SCB_CLIENT_BASE+i*4,CLIENT_BLOCK_VALUE);
    }
    /* SCB refresh client is handled in the asm code */
    return(0);
}
/****************************************************************
* ppmd_drv_restore_client_settings
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Restores memory client settings to original settings.
*
****************************************************************/
static int ppmd_drv_restore_client_settings(void)
{
    int i;

    for (i = 0; i < REFRESH_CLIENT; i++) {
        BREG_Write32(g_ppmd_reg_handle, SCB_CLIENT_BASE+i*4,g_ppmd_client[i]);
    }
    return(0);
}

/****************************************************************
* ppmd_drv_setup_pinmux
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Sets up the pinmux registers for the front panel
*
****************************************************************/

static void ppmd_drv_setup_pinmux(void)
{
    int i;
    struct {
        uint32_t reg;
        uint32_t mask;
        uint32_t val;
    } mux[] = PINMUX_SETUP;

    for(i = 0; i < (sizeof(mux) / sizeof(mux[0])); i++) {
        BREG_Write32(g_ppmd_reg_handle, mux[i].reg,
            (BREG_Read32(g_ppmd_reg_handle, mux[i].reg) &
                mux[i].mask) | mux[i].val);
    }
}


/****************************************************************
* ppmd_drv_replace_leds
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Saves led registers and puts a sleep message out
*
****************************************************************/

static int ppmd_drv_replace_leds(void)
{
#if LDK_FP
    g_ppmd_saved_leds[0] = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_DIGIT1);
    g_ppmd_saved_leds[1] = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_DIGIT2);
    g_ppmd_saved_leds[2] = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_DIGIT3);
    g_ppmd_saved_leds[3] = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_DIGIT4);
    BLED_Write(g_ppmd_led_handle, 1, LED_O);
    BLED_Write(g_ppmd_led_handle, 2, LED_F);
    BLED_Write(g_ppmd_led_handle, 3, LED_F);
    BLED_Write(g_ppmd_led_handle, 4, 0xff);
#endif
    return(0);
}

/****************************************************************
* ppmd_drv_restore_leds
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Restores leds to original settings.
*
****************************************************************/
static int ppmd_drv_restore_leds(void)
{
#if LDK_FP
    int i;
    for(i=0;i<4;i++) BLED_Write(g_ppmd_led_handle, i+1, g_ppmd_saved_leds[i]);
#endif
    return(0);
}

/****************************************************************
* enable_user_inputs
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Enables IR remote and keypad interrupts
*
****************************************************************/
static void enable_user_inputs()
{
    uint32_t lval;

    /* Enable IR device */
    BKIR_EnableIrDevice (g_ppmd_ir_channel_handle, BKIR_KirDevice_eRemoteA);
    lval = BREG_Read32(g_ppmd_reg_handle, BCHP_KBD1_CMD);
    lval |= BCHP_KBD1_CMD_kbd_irqen_MASK;
    BREG_Write32(g_ppmd_reg_handle,BCHP_KBD1_CMD,lval);

#if LDK_FP
    /* Enable keypad  */
    lval = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_CONTROL);
    lval |= BCHP_LDK_CONTROL_irqen_MASK;
    BREG_Write32(g_ppmd_reg_handle,BCHP_LDK_CONTROL,lval);
#endif
}

/****************************************************************
* disable_user_inputs
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Disables IR remote and keypad interrupts
*
****************************************************************/
static void disable_user_inputs(void)
{
    uint32_t lval;

    /* Disable IR device */
    lval = BREG_Read32(g_ppmd_reg_handle, BCHP_KBD1_CMD);
    lval &= ~BCHP_KBD1_CMD_kbd_irqen_MASK;
    BREG_Write32(g_ppmd_reg_handle,BCHP_KBD1_CMD,lval);

#if LDK_FP
    /* Disable keypad  */
    lval = BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_CONTROL);
    lval &= ~BCHP_LDK_CONTROL_irqen_MASK;
    BREG_Write32(g_ppmd_reg_handle,BCHP_LDK_CONTROL,lval);
#endif
}

/****************************************************************
* ppmd_drv_standby
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      0 - success
* FUNCTION:     Main routine to initiate passive standby mode.
*
****************************************************************/
static int ppmd_drv_standby(void)
{
    unsigned long c;
    unsigned long (*cached_asm_wait_routine) (int, int, unsigned long) =
        g_ppmd_low_alloc;
    if(down_trylock(&g_ppmd_standby_lock)) return -EAGAIN;

    g_ppmd_active.wake_event = 0;
    g_ppmd_active.ext_irq_mask = 0;

    ppmd_drv_disable_interrupts();
    BDBG_MSG(("\n%s: interrupts disabled",__FUNCTION__));

    ppmd_drv_lock_handler();
    BDBG_MSG(("%s: lock handler ok",__FUNCTION__));

    ppmd_drv_replace_exception_vectors();
    BDBG_MSG(("%s: replaced vectors",__FUNCTION__));

#if HW_STANDBY
    ppmd_drv_init_hw_standby();
    BDBG_MSG(("%s: initialized HW standby",__FUNCTION__));
#endif

    ppmd_drv_replace_level_1_interrupts();
    BDBG_MSG(("%s: replaced L1",__FUNCTION__));

    ppmd_drv_replace_level_2_interrupts();
    BDBG_MSG(("%s: replaced L2",__FUNCTION__));

    if(test_for_memory_self_refresh() != PPMD_NO_SHUTDOWN) {
        ppmd_drv_replace_client_settings();
        BDBG_MSG(("%s: replaced client settings",__FUNCTION__));
    }

    ppmd_drv_replace_leds();
    BDBG_MSG(("%s: replaced LEDs",__FUNCTION__));

    ppmd_drv_program_upg_timer();

    ppmd_drv_set_curr_time();

    BDBG_MSG(("%s: going into standby mode",__FUNCTION__));

    /* Test with a wait instruction */
    while(g_ppmd_active.wake_event == 0) {
        enable_user_inputs();
        cached_asm_wait_routine(test_for_memory_self_refresh(),
            g_ppmd_icache_line_size, g_ppmd_interrupt_vector);

        if(ppmd_drv_timer_expired()) {
            ppmd_drv_adjust_curr_time();
            ppmd_drv_stop_timer();
            ppmd_drv_reprogram_upg_timer();

            BDBG_MSG(("%s: UPG Tmr status=%x cause=%x L1_w0_maskstatus=%x",
                __FUNCTION__,
                read_c0_status(),
                read_c0_cause(),
                BREG_Read32(g_ppmd_reg_handle, BCHP_HIF_CPU_INTR1_INTR_W0_MASK_STATUS)));
            if(alarm_time_reached()) {
                if (!g_ppmd_active.wake_event)
                    g_ppmd_active.wake_event = PPMD_IOCTL_EVENT_TIMEOUT;
            } else {
                ppmd_drv_rearm_timer();
            }
        }
        if(PPMD_L2_STATUS(g_ppmd_reg_handle, PPMD_L2_MASK_GPIO)) {
            if(!g_ppmd_active.wake_event) {
                unsigned long mask = 
                    PPMD_L2_STATUS(g_ppmd_reg_handle,
                        PPMD_L2_MASK_GPIO);

                if(test_for_gpio_wakeup() == WAKEUP) {
                    g_ppmd_active.wake_event =
                        PPMD_IOCTL_EVENT_GPIO;
                    g_ppmd_active.ext_irq_mask = mask;
                }
            }
        }
        if(PPMD_L2_STATUS(g_ppmd_reg_handle, PPMD_L2_MASK_IR)) {
            /* disable IR */
            BREG_Write32(g_ppmd_reg_handle, BCHP_KBD1_CMD, 0);
            /* clear interrupt */
            BREG_Write32(g_ppmd_reg_handle, BCHP_KBD1_STATUS, 0);

            if(!g_ppmd_active.wake_event && test_for_ir_remote_wakeup()==WAKEUP) {
                g_ppmd_active.wake_event = PPMD_IOCTL_EVENT_IR;
            }
        }
#if LDK_FP
        if(PPMD_L2_STATUS(g_ppmd_reg_handle, PPMD_L2_MASK_KPD)) {
            /* disable keypad */
            BREG_Write32(g_ppmd_reg_handle, BCHP_LDK_CONTROL, 2);
            /* clear keypad interrupt */
            BREG_Write32(g_ppmd_reg_handle, BCHP_LDK_KEYROW10,
                BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_KEYROW10));
            BREG_Write32(g_ppmd_reg_handle, BCHP_LDK_KEYROW32,
                BREG_Read32(g_ppmd_reg_handle, BCHP_LDK_KEYROW32));
            if(!g_ppmd_active.wake_event && test_for_keypad_wakeup()==WAKEUP)
            {
                g_ppmd_active.wake_event = PPMD_IOCTL_EVENT_KEYPAD;
            }
        }
#endif
#if SPI_FP
        if(BREG_Read32(g_ppmd_reg_handle, SPI_IRQ_REG_STATUS) &
            SPI_IRQ_MASK) {
            if(g_ppmd_active.events & PPMD_IOCTL_EVENT_KEYPAD)
            {
                g_ppmd_active.wake_event = PPMD_IOCTL_EVENT_KEYPAD;
            }
        }
#endif
#if VGA_INPUT
        if(PPMD_L2_STATUS(g_ppmd_reg_handle, PPMD_L2_MASK_VGA)) {

            /* NOTE: test_for_vga_wakeup() takes a long time */

            if(!g_ppmd_active.wake_event &&
                test_for_vga_wakeup() == WAKEUP) {
                    g_ppmd_active.wake_event =
                        PPMD_IOCTL_EVENT_VGA;
            }
        }
#endif
#ifdef HDMI_CEC_FEATURE 
   if(g_ppmd_active.events & PPMD_IOCTL_EVENT_CEC)
   {
			if(!g_ppmd_active.wake_event &&
				test_for_cec_wakeup() == WAKEUP) {
					g_ppmd_active.wake_event =
						PPMD_IOCTL_EVENT_CEC;
			}
	 }
#endif
        disable_user_inputs();
    }

    BDBG_MSG(("%s: coming out of standby mode",__FUNCTION__));

    BDBG_MSG(("%s: restoring LEDs",__FUNCTION__));
    ppmd_drv_restore_leds();

    if(test_for_memory_self_refresh() != PPMD_NO_SHUTDOWN) {
        BDBG_MSG(("%s: restoring client settings",__FUNCTION__));
        ppmd_drv_restore_client_settings();
    }

    BDBG_MSG(("%s: restoring vectors",__FUNCTION__));
    ppmd_drv_restore_exception_vectors();
     
    ppmd_drv_restore_level_2_interrupts();
    BDBG_MSG(("%s: restoring L2",__FUNCTION__));

    ppmd_drv_restore_level_1_interrupts();
    BDBG_MSG(("%s: restoring L1",__FUNCTION__));

#if HW_STANDBY
    ppmd_drv_cleanup_hw_standby();
    BDBG_MSG(("%s: cleaned up HW standby",__FUNCTION__));
#endif

    ppmd_drv_unlock_handler();
    BDBG_MSG(("%s: unlock handler ok",__FUNCTION__));

    ppmd_drv_stop_timer();

    /* Force a timer interrupt as soon as possible after we enable interrupts */
    c = read_c0_compare();
    irq_disable_hazard(); 
    c -= 256;
    write_c0_count(c);
    irq_disable_hazard(); 

    ppmd_drv_adjust_curr_time();
    ppmd_drv_adjust_kernel_time();
    ppmd_drv_enable_interrupts();

    BDBG_MSG(("%s: interrupts re-enabled",__FUNCTION__));
    up(&g_ppmd_standby_lock);
    BDBG_MSG(("%s: semaphore released",__FUNCTION__));
    return 0;
}

/****************************************************************
* ppmd_drv_open
*
* INPUTS:       inode, file - see linux kernel api
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Standard open driver entry point
*
****************************************************************/
static int ppmd_drv_open (struct inode *inode, struct file *file)
{
    return 0;
}

/****************************************************************
* ppmd_drv_release
*
* INPUTS:       inode, file - see linux kernel api
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Standard release driver entry point
*
****************************************************************/
static int ppmd_drv_release (struct inode *inode, struct file *file)
{
    return 0;
}

/****************************************************************
* ppmd_drv_ioctl
*
* INPUTS:       inode, filp, cmd, arg - see linux kernel api
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Standard ioctl driver entry point
*
****************************************************************/
static int ppmd_drv_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp;  
    int status = 0;

    if(cmd==PPMD_IOCTL_ACTIVATE_SLEEP) {
        if(num_online_cpus() > 1) return -EIO;
        argp = (void __user *)arg;
        copy_from_user(&g_ppmd_active,argp,sizeof(struct _PPMD_IOCTL));
        if(!upg_timer_duration_ok()) return -EPROTO;
        if(!alarm_time_ok()) return -EPROTO;

        status = ppmd_drv_standby();
        copy_to_user(argp,&g_ppmd_active,sizeof(struct _PPMD_IOCTL));
    }
    else return -EIO;
    return status;
}

/****************************************************************
* ppmd_drv_relocate_handler
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Copies our asm handler code to KSEG0
*
****************************************************************/
static int ppmd_drv_relocate_handler(void)
{
    unsigned long addr,limit;
    int icache_line_size;
    struct cache_desc *ic = &current_cpu_data.icache;

    /* Get icache line size */
    g_ppmd_icache_line_size = ic->linesz;
    if(g_ppmd_icache_line_size==0) {
        BDBG_ERR(("Bad icache line size value")); 
        return -1;
    }
    BDBG_MSG(("%s: i-cache line size=%d",__FUNCTION__,g_ppmd_icache_line_size));

    g_ppmd_low_alloc_size = g_ppmd_icache_line_size +
        (unsigned long)passiveIRQ_end - (unsigned long)asm_wait_routine;
    g_ppmd_low_alloc = kmalloc(g_ppmd_low_alloc_size,GFP_DMA);
    if(g_ppmd_low_alloc==NULL) goto error;
    BDBG_MSG(("%s: Allocated %d bytes at 0x%8.8x",__FUNCTION__,g_ppmd_low_alloc_size,g_ppmd_low_alloc));

    /* Copy our asm handler to KSEG0 */
    memcpy((void *) g_ppmd_low_alloc,(void *) asm_wait_routine,g_ppmd_low_alloc_size);

    return 0;
error:
    if(g_ppmd_low_alloc) kfree(g_ppmd_low_alloc);
    return -1;
}

/****************************************************************
* ppmd_drv_lock_handler
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Locks our handler into i-cache
*
****************************************************************/
static int ppmd_drv_lock_handler(void)
{
    unsigned long addr,limit;

    /* actual locking happens in the asm section */

#ifdef CONFIG_BMIPS4380
    /* Disable RAC prefetching */
    unsigned int cba = KSEG1 | __read_32bit_c0_register($22, 6) & 0xfffc0000;
    g_ppmd_saved_rac_config = DEV_RD(cba);
    DEV_WR(cba, g_ppmd_saved_rac_config & ~0xc);
#endif

    return 0;
}

/****************************************************************
* ppmd_drv_unlock_handler
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Unlock I-cache lines
*
****************************************************************/
static int ppmd_drv_unlock_handler(void)
{
    struct cache_desc *ic = &current_cpu_data.icache;
    uintptr_t addr = 0x80000000;
    int i;

#ifdef CONFIG_BMIPS4380
    /* Re-enable RAC prefetching */
    unsigned int cba = KSEG1 | __read_32bit_c0_register($22, 6) & 0xfffc0000;
    DEV_WR(cba, g_ppmd_saved_rac_config);
#endif

    /* unlock/invalidate I$ */
    for(i = 0; i < ic->ways * ic->sets; i++) {
        flush_icache_line_indexed(addr);
        addr += ic->linesz;
    }

    return 0;
}

/****************************************************************/
module_init (my_init);
module_exit (my_exit);

#if 0//def CONFIG_PM
static int brcm_pm_prepare(suspend_state_t state)
{
    return 0;
}

static int brcm_pm_enter(suspend_state_t state)
{
    g_ppmd_active.flags = PPMD_IOCTL_FLAGS_MEMORY_SELF_REFRESH |
         PPMD_IOCTL_FLAGS_SELF_REFRESH_W_PADS |
         PPMD_IOCTL_FLAGS_SET_UPG_TIMER_DURATION;
    g_ppmd_active.events = PPMD_IOCTL_EVENT_IR |
         PPMD_IOCTL_EVENT_KEYPAD |
         PPMD_IOCTL_EVENT_GPIO;
    g_ppmd_active.upg_timer_duration = 2;
    return ppmd_drv_standby();
}

static int brcm_pm_finish(suspend_state_t state)
{
    return 1;
}

static int brcm_pm_valid(suspend_state_t state)
{
    return 1;
}


static struct pm_ops brcm_pm_ops = {
        .prepare        = brcm_pm_prepare,
        .enter          = brcm_pm_enter,
        .finish         = brcm_pm_finish,
        .valid          = brcm_pm_valid,
};

#endif

/****************************************************************
* my_init
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Standard driver module entry point
*
****************************************************************/
static int my_init (void)
{
    int i;

    BERR_Code rc;
    const BINT_Settings *intr_settings;
#if LDK_FP
    BKPD_Settings                       kpd_settings;
    BLED_Settings                       led_settings;
#endif
    BKIR_ChannelSettings                ir_remote_settings;
    void *uncached_sys_mem;
    void *reg_addr_offset = (void *) (0xa0000000+BCHP_PHYSICAL_OFFSET);
    size_t reg_bank_size = B_MAX_REG_ADDRESS + 4;   /* Taken from last address in RDB */
    int chanNo;

    if (BDBG_Init()!=BERR_SUCCESS) {
        DRIVER_ERROR(1);
        return -1;
    }

    if(*msg_modules!=0) ppmd_drv_set_module_debuglevel(msg_modules, BDBG_eMsg);

    BREG_Open(&g_ppmd_reg_handle, reg_addr_offset, reg_bank_size, NULL);

    rc = BCHP_OPEN(&g_ppmd_chip_handle, g_ppmd_reg_handle);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(2);
        return -1;
    }

    intr_settings = BINT_GETSETTINGS( );
    rc = BINT_Open(&g_ppmd_int_handle, g_ppmd_reg_handle, intr_settings, NULL);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(3);
        return -1;
    }

    rc = ppmd_drv_open_timer();
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(4);
        return -1;
    }

    ppmd_drv_setup_pinmux();

#if LDK_FP
    /* Setup the BLED */
    BLED_GetDefaultSettings( &led_settings, g_ppmd_chip_handle );
    rc = BLED_Open (&g_ppmd_led_handle, g_ppmd_chip_handle, g_ppmd_reg_handle, &led_settings);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(6);
        return -1;
    }

    /* Setup the BKPD */
    BKPD_GetDefaultSettings( &kpd_settings, g_ppmd_chip_handle );
    rc = BKPD_Open (&g_ppmd_keypad_handle, g_ppmd_chip_handle,
        g_ppmd_reg_handle, g_ppmd_int_handle, &kpd_settings);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(7);
        return -1;
    }
#endif

    /* Setup the BKIR */
    rc = BKIR_Open (&g_ppmd_ir_remote_handle, g_ppmd_chip_handle,
        g_ppmd_reg_handle, g_ppmd_int_handle, (BKIR_Settings *)NULL);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(8);
        return -1;
    }

    chanNo = 0;
    BKIR_GetChannelDefaultSettings( g_ppmd_ir_remote_handle, chanNo, &ir_remote_settings );

    ir_remote_settings.irPort = BKIR_KirPort1; 

    rc = BKIR_OpenChannel( g_ppmd_ir_remote_handle, &g_ppmd_ir_channel_handle,
        chanNo, &ir_remote_settings );
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(9);
        return -1;
    }

    /* Setup the BGIO */
    rc = BGIO_Open(&g_ppmd_gio_handle, g_ppmd_chip_handle, g_ppmd_reg_handle);
    if ( rc!=BERR_SUCCESS )
    {
        DRIVER_ERROR(10);
        return -1;
    }

    /* Relocate our handler to KSEG0 */
    ppmd_drv_relocate_handler();

    if (register_chrdev(PPMD_MAJOR,"ppmd0", &ppmd_drv_fops))
    {
        BDBG_ERR(("ppmd_dev: Unable to get major %d",PPMD_MAJOR));
        goto error;
    }

#if 0//def CONFIG_PM
    /* Set power manager hooks */
    pm_set_ops(&brcm_pm_ops);
#endif

    return 0;

error:
    if(g_ppmd_low_alloc) kfree(g_ppmd_low_alloc);
    return -EBUSY;
}

/****************************************************************
* my_exit
*
* INPUTS:       None
* OUTPUTS:      None
* RETURNS:      None
* FUNCTION: Standard driver module exit point called on rmmod
*
****************************************************************/
static void my_exit (void)
{
    BGIO_Close(g_ppmd_gio_handle);
    BKIR_CloseChannel(g_ppmd_ir_channel_handle);
    BKIR_Close(g_ppmd_ir_remote_handle);
#if LDK_FP
    BKPD_Close(g_ppmd_keypad_handle);
    BLED_Close(g_ppmd_led_handle);
#endif
    ppmd_drv_close_timer();
    BINT_Close(g_ppmd_int_handle);
    BCHP_Close(g_ppmd_chip_handle);
    BREG_Close(g_ppmd_reg_handle);
    BDBG_Uninit();

    kfree(g_ppmd_low_alloc);
    unregister_chrdev(PPMD_MAJOR,"ppmd0");
}
