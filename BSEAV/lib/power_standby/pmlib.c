/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 *  Power management API for Broadcom STB/DTV peripherals
 *
 *   when        who         what
 *   -----       ---         ----
 *   20071030    cernekee    initial version
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pmlib.h"

struct brcm_pm_priv
{
    struct brcm_pm_state    last_state;
};

#define BUF_SIZE    64
#define MAX_ARGS    16

#define SYS_USB_STAT    "/sys/devices/platform/brcm-pm/usb"
#define SYS_ENET_STAT   "/sys/devices/platform/brcm-pm/enet"
#define SYS_SATA_STAT   "/sys/devices/platform/brcm-pm/sata"
#define SYS_DDR_STAT    "/sys/devices/platform/brcm-pm/ddr"
#define SYS_TP1_STAT    "/sys/devices/system/cpu/cpu1/online"
#define SYS_BASE_FREQ   "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
#define SYS_CUR_FREQ    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define DHCPCD_PIDFILE  "/etc/config/dhcpc/dhcpcd-eth0.pid"
#define DHCPCD_PATH "/bin/dhcpcd"
#define IFCONFIG_PATH   "/bin/ifconfig"
#define HDPARM_PATH "/sbin/hdparm"
#define SATA_RESCAN_GLOB "/sys/class/scsi_host/host*/scan"
#define SATA_DEVICE_GLOB "/sys/class/scsi_device/*/device/block:*"
#define SATA_DELETE_GLOB "/sys/class/scsi_device/*/device/delete"

#if BDBG_DEBUG_BUILD
static int berr_trace(int rc, int line) {
    if (rc) {
        fprintf(stderr, "### pmlib error: %d, line %d\n", rc, line);
    }
    return rc;
}
#define BERR_TRACE(RC) berr_trace(RC, __LINE__)
#else
#define BERR_TRACE(RC) (RC)
#endif

static int sysfs_get(char *path, unsigned int *out)
{
    FILE *f;
    unsigned int tmp;

    f = fopen(path, "r");
    if(! f)
        return -1; /* this failure appears to be normal */
    if(fscanf(f, "%15u", &tmp) != 1)
    {
        fclose(f);
        return BERR_TRACE(-1);
    }
    *out = tmp;
    fclose(f);
    return(0);
}

static int sysfs_set(char *path, int in)
{
    FILE *f;
    char buf[BUF_SIZE];

    f = fopen(path, "w");
    if(! f)
        return BERR_TRACE(-1);
    sprintf(buf, "%u", in);
    if(! fputs(buf, f) < 0)
    {
        fclose(f);
        return BERR_TRACE(-1);
    }
    fclose(f);
    return(0);
}

static int sysfs_set_string(char *path, const char *in)
{
    FILE *f;

    f = fopen(path, "w");
    if(! f)
        return BERR_TRACE(-1);
    if(! fputs(in, f) < 0)
    {
        fclose(f);
        return BERR_TRACE(-1);
    }
    fclose(f);
    return(0);
}

static int run(char *prog, ...)
{
    va_list ap;
    int status, i = 1;
    pid_t pid;
    char *args[MAX_ARGS], *a;

    va_start(ap, prog);

    pid = fork();
    if(pid < 0)
    {
        va_end(ap);
        return BERR_TRACE(-1);
    }

    if(pid != 0)
    {
        wait(&status);
        va_end(ap);
        return WEXITSTATUS(status) ? BERR_TRACE(-1) : 0;
    }

    /* child */
    args[0] = prog;
    do
    {
        a = va_arg(ap, char *);
        args[i++] = a;
    } while(a);

    execv(prog, args);
    _exit(1);

    va_end(ap); /* never reached */
    return(0);
}

void *brcm_pm_init(void)
{
    struct brcm_pm_priv *ctx;

    ctx = (void *)malloc(sizeof(*ctx));
    if(! ctx)
        goto bad;

    if(sysfs_get(SYS_BASE_FREQ,
        (unsigned int *)&ctx->last_state.cpu_base) != 0)
    {
        /* cpufreq not supported on this platform */
        ctx->last_state.cpu_base = 0;
    }

    if(brcm_pm_get_status(ctx, &ctx->last_state) != 0)
        goto bad_free;

    return(ctx);

bad_free:
    free(ctx);
bad:
    return(NULL);
}

void brcm_pm_close(void *vctx)
{
    free(vctx);
}

int brcm_pm_get_status(void *vctx, struct brcm_pm_state *st)
{
    struct brcm_pm_priv *ctx = vctx;
    int tmp;

    /* read status from /proc */

    if(sysfs_get(SYS_USB_STAT, (unsigned int *)&st->usb_status) != 0) {
        st->usb_status = BRCM_PM_UNDEF;
    }
    if(sysfs_get(SYS_ENET_STAT, (unsigned int *)&st->enet_status) != 0) {
        st->enet_status = BRCM_PM_UNDEF;
    }
    if(sysfs_get(SYS_SATA_STAT, (unsigned int *)&st->sata_status) != 0) {
        st->sata_status = BRCM_PM_UNDEF;
    }
    if(sysfs_get(SYS_DDR_STAT, (unsigned int *)&st->ddr_timeout) != 0) {
        st->ddr_timeout = BRCM_PM_UNDEF;
    }
    if(sysfs_get(SYS_TP1_STAT, (unsigned int *)&st->tp1_status) != 0) {
        st->tp1_status = BRCM_PM_UNDEF;
    }

    if(ctx->last_state.cpu_base)
    {
        if((sysfs_get(SYS_CUR_FREQ, (unsigned int *)&tmp) != 0) ||
           (tmp == 0))
        {
            return BERR_TRACE(-1);
        }

        st->cpu_base = ctx->last_state.cpu_base;
        st->cpu_divisor = st->cpu_base / tmp;
    } else {
        st->cpu_base = 0;
        st->cpu_divisor = -1;
    }

    if(st != &ctx->last_state)
        memcpy(&ctx->last_state, st, sizeof(*st));

    return(0);
}

static int sata_rescan_hosts(void)
{
    glob_t g;
    int i, ret = 0;
    int rc;

    if(glob(SATA_RESCAN_GLOB, GLOB_NOSORT, NULL, &g) != 0)
        return BERR_TRACE(-1);

    for(i = 0; i < (int)g.gl_pathc; i++) {
        rc = sysfs_set_string(g.gl_pathv[i], "0 - 0");
        if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
    }
    globfree(&g);

    return(ret);
}

static int sata_spindown_devices(void)
{
    glob_t g;
    int i, ret = 0;

    /* NOTE: if there are no devices present, it is not an error */
    if(glob(SATA_DEVICE_GLOB, GLOB_NOSORT, NULL, &g) != 0)
        return(0);

    for(i = 0; i < (int)g.gl_pathc; i++)
    {
        char *devname = rindex(g.gl_pathv[i], ':');
        char buf[BUF_SIZE];

        if(! devname)
            return BERR_TRACE(-1);
        snprintf(buf, BUF_SIZE, "/dev/%s", devname + 1);

        /* ignore return value as some devices will fail */
        run(HDPARM_PATH, "-y", buf, NULL);
    }
    globfree(&g);

    return(ret);
}

static int sata_delete_devices(void)
{
    glob_t g;
    int i, ret = 0;

    if(glob(SATA_DELETE_GLOB, GLOB_NOSORT, NULL, &g) != 0)
        return(0);

    for(i = 0; i < (int)g.gl_pathc; i++) {
        int rc;
        rc = sysfs_set(g.gl_pathv[i], 1);
        if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
    }
    globfree(&g);

    return(ret);
}

int brcm_pm_set_status(void *vctx, struct brcm_pm_state *st)
{
    struct brcm_pm_priv *ctx = vctx;
    int ret = 0;
    int rc;

#define CHANGED(element) \
    ((st->element != BRCM_PM_UNDEF) && \
     (st->element != ctx->last_state.element))


    if(CHANGED(usb_status))
    {
        rc = sysfs_set(SYS_USB_STAT, st->usb_status ? 1 : 0);
        if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        ctx->last_state.usb_status = st->usb_status;
    }

    if(CHANGED(enet_status))
    {
        if(st->enet_status)
        {
            rc = run(DHCPCD_PATH, "-H", "eth0", NULL);
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        } else {
            unsigned int pid;

            if(sysfs_get(DHCPCD_PIDFILE, &pid) == 0)
                kill(pid, SIGTERM);
            rc = run(IFCONFIG_PATH, "eth0", "down", NULL);
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
            rc = sysfs_set(SYS_ENET_STAT, 0);
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        }
        ctx->last_state.enet_status = st->enet_status;
    }

    if(CHANGED(sata_status))
    {
        if(st->sata_status)
        {
            rc = sata_rescan_hosts();
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        } else {
            rc = sata_spindown_devices();
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
            rc = sata_delete_devices();
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
            rc = sysfs_set(SYS_SATA_STAT, 0);
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        }
        ctx->last_state.sata_status = st->sata_status;
    }

    if(CHANGED(tp1_status))
    {
        rc = sysfs_set(SYS_TP1_STAT, st->tp1_status);
        if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
    }

    if(CHANGED(cpu_divisor))
    {
        if(! st->cpu_divisor || ! ctx->last_state.cpu_base)
        {
            rc = -1;
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        }
        else {
            unsigned int freq;

            freq = ctx->last_state.cpu_base / st->cpu_divisor;
            rc = sysfs_set(SYS_CUR_FREQ, freq);
            if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
        }
        ctx->last_state.cpu_divisor = st->cpu_divisor;
    }

    if(CHANGED(ddr_timeout))
    {
        rc = sysfs_set(SYS_DDR_STAT, st->ddr_timeout);
        if (rc) {ret |= BERR_TRACE(rc);} /* keep going */
    }

#undef CHANGED

    return(ret);
}


/*************************************************

The following brcm_pm_standby and related functions only apply to 2.6.18.
A new technique will be supported in future 2.6.31 releases.

For 2.6.31, the PM_L2 register will be configured outside of pmlib.
The app (likely Nexus) will configure the ir/gpio/keypad devices.
pmlib will be used to put linux into standby waiting for any of those L2's to fire.
This way linux code doesn't have to include what is normally magnum-logic.
*************************************************/
#include "brcmpmdrv.h"
#include <sys/time.h>
#include <sys/ioctl.h>

void brcm_pm_get_default_standby_settings( struct brcm_pm_standby_settings *p_settings )
{
    memset(p_settings, 0, sizeof(*p_settings));
    /* no timeout */
    p_settings->ir = true;
    p_settings->keypad = true;
    p_settings->gpio = true;
}

int brcm_pm_standby( void *ctx, const struct brcm_pm_standby_settings *p_settings, struct brcm_pm_standby_result *p_result )
{
    int fd = -1;
    int rc;
    struct timeval now;
    PPMD_IOCTL ppmd_ioctl_arg;

    ctx = ctx; /* unused */

    memset(p_result, 0, sizeof(*p_result));

    fd = open("/dev/brcmpmdrv", O_RDWR);
    if (fd == -1) return BERR_TRACE(errno);

    rc = gettimeofday(&now, NULL);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    /* Setup ioctl structure */
    memset(&ppmd_ioctl_arg, 0, sizeof(ppmd_ioctl_arg));
    if (p_settings->timeout) {
        ppmd_ioctl_arg.events |= PPMD_IOCTL_EVENT_TIMEOUT;
    }
    if (p_settings->keypad) {
        ppmd_ioctl_arg.events |= PPMD_IOCTL_EVENT_KEYPAD;
    }
    if (p_settings->gpio) {
        ppmd_ioctl_arg.events |= PPMD_IOCTL_EVENT_GPIO;
    }
    if (p_settings->ir) {
        ppmd_ioctl_arg.events |= PPMD_IOCTL_EVENT_IR;
    }
    ppmd_ioctl_arg.flags =
        PPMD_IOCTL_FLAGS_SET_UPG_TIMER_DURATION |
        PPMD_IOCTL_FLAGS_MEMORY_SELF_REFRESH |
        PPMD_IOCTL_FLAGS_SELF_REFRESH_W_PADS |
        PPMD_IOCTL_FLAGS_POWER_BUTTON_ONLY;
    ppmd_ioctl_arg.end_time.tv_sec = p_settings->timeout;
    ppmd_ioctl_arg.upg_timer_duration = PPMD_IOCTL_DEFAULT_UPG_DURATION;

    rc = ioctl(fd, PPMD_IOCTL_ACTIVATE_SLEEP, &ppmd_ioctl_arg);
    if (rc) {
        fprintf(stderr, "PPMD_IOCTL_ACTIVATE_SLEEP failed with errno %d\n", errno);
        rc = BERR_TRACE(rc);
        goto done;
    }

    switch (ppmd_ioctl_arg.wake_event) {
    case PPMD_IOCTL_EVENT_IR:       p_result->ir = true; break;
    case PPMD_IOCTL_EVENT_KEYPAD:   p_result->keypad = true; break;
    case PPMD_IOCTL_EVENT_GPIO:     p_result->gpio = true; break;
    case PPMD_IOCTL_EVENT_TIMEOUT:  p_result->timeout = true; break;
    default: break;
    }

done:
    if (fd != -1) close(fd);
    return rc;
}

