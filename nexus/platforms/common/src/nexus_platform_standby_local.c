/******************************************************************************
 *  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#include "nexus_base.h"
#include "nexus_platform_standby.h"
#include "nexus_platform_local_priv.h"
#include "bkni.h"

BDBG_MODULE(nexus_platform_standby_local);

#if !defined(NEXUS_BASE_OS_linuxkernel)

#if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT
#include <sys/ioctl.h>
#include "wakeup_driver.h"
#include "nexus_base_os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/timerfd.h>
#include <sys/stat.h>



#define RTC_WAKE_SYSFS "/sys/class/rtc/rtc0/wakealarm"

#define BUF_SIZE 256

typedef enum NEXUS_Platform_P_SysWake {
    NEXUS_Platform_P_SysWake_eTimer,
    NEXUS_Platform_P_SysWake_eGpio,
    NEXUS_Platform_P_SysWake_eMax
} NEXUS_Platform_P_SysWake;

static struct NEXUS_Platform_Standby_State {
    int wakeFd;
    int rtcFd;
    NEXUS_StandbyMode mode;
    NEXUS_PlatformStandbyStatus standbyStatus;
    bool wakeupStatusCached;
    struct sys_wake_device {
        char enable[BUF_SIZE];
        char count[BUF_SIZE];
        char timeout[BUF_SIZE];
        bool found;
    } wakeup[NEXUS_Platform_P_SysWake_eMax];
} g_Standby_State;

static struct sysWakePath {
    char *dir;
    char *file;
} g_sysWakePath[NEXUS_Platform_P_SysWake_eMax] = {{"/sys/bus/platform/drivers/brcm-waketimer","waketimer"},
                                                  {"/sys/bus/platform/drivers/brcmstb-gpio","gpio"}};

static int sysfs_get(char *path, unsigned int *out)
{
    FILE *f;
    char buf[BUF_SIZE];

    f = fopen(path, "r");
    if(! f)
        return(-1);
    if(fgets(buf, BUF_SIZE, f) != buf)
    {
        fclose(f);
        return(-1);
    }
    fclose(f);
    *out = strtoul(buf, NULL, 0);

    return(0);
}

static int sysfs_set(char *path, int in)
{
    FILE *f;
    char buf[BUF_SIZE];

    f = fopen(path, "w");
    if(! f)
        return(-1);
    snprintf(buf, BUF_SIZE, "%u", in);
    if((fputs(buf, f) < 0) || (fflush(f) < 0))
    {
        fclose(f);
        return(-1);
    }
    fclose(f);
    return(0);
}

static int sysfs_set_string(char *path, const char *in)
{
    FILE *f;

    f = fopen(path, "w");
    if(! f)
        return(-1);
    if((fputs(in, f) < 0) || (fflush(f) < 0))
    {
        fclose(f);
        return(-1);
    }
    fclose(f);
    return(0);
}

static int sysfs_get_string(char *path, char *in, int size)
{
	FILE *f;

	f = fopen(path, "r");
	if (!f)
		return(-1);
	if (fgets(in, size, f) == NULL)
	{
		fclose(f);
		return(-1);
	}
	fclose(f);
	return(0);
}

static int set_rtc_wake(unsigned timeout)
{
   struct itimerspec new_value;
   struct timespec now;

   if (clock_gettime(CLOCK_BOOTTIME, &now) == -1) {
       BDBG_ERR(("Failed to get current time"));
       return -1;
   }

   new_value.it_value.tv_sec = now.tv_sec + timeout;
   new_value.it_value.tv_nsec = now.tv_nsec;
   new_value.it_interval.tv_sec = 0;
   new_value.it_interval.tv_nsec = 0;

   if (timerfd_settime(g_Standby_State.rtcFd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
       BDBG_ERR(("Failed to set timer"));
       return -1;
   }

   BDBG_MSG(("set_rtc_wake : fd %d, timeout %u", g_Standby_State.rtcFd, timeout));

   return 0;
}

static void NEXUS_Platform_P_FindSysWake(NEXUS_Platform_P_SysWake type)
{
    DIR * dir = opendir(g_sysWakePath[type].dir);
    struct dirent *ent;

    BDBG_ASSERT(type < NEXUS_Platform_P_SysWake_eMax);

    if(dir) {
        while ((ent = readdir(dir)) != NULL) {
            if(strstr(ent->d_name, g_sysWakePath[type].file)) {
                snprintf(g_Standby_State.wakeup[type].enable, BUF_SIZE, "%s/%s/%s", g_sysWakePath[type].dir, ent->d_name, "power/wakeup");
                snprintf(g_Standby_State.wakeup[type].count, BUF_SIZE, "%s/%s/%s", g_sysWakePath[type].dir, ent->d_name, "power/wakeup_count");
                if(type == NEXUS_Platform_P_SysWake_eTimer)
                    snprintf(g_Standby_State.wakeup[type].timeout, BUF_SIZE, "%s/%s/%s", g_sysWakePath[type].dir, ent->d_name, "timeout");
                g_Standby_State.wakeup[type].found = true;
                break;
            }
        }
        closedir(dir);
    }

    return;
}

static NEXUS_Error NEXUS_Platform_P_GetSysWake(NEXUS_Platform_P_SysWake type, unsigned int *count)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    char enabled[32];

    BDBG_ASSERT(type < NEXUS_Platform_P_SysWake_eMax);

    if(!g_Standby_State.wakeup[type].found) {
        NEXUS_Platform_P_FindSysWake(type);
        if(!g_Standby_State.wakeup[type].found) {
            BDBG_WRN(("Unable to find %s wakeup", g_sysWakePath[type].file));
            rc = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto err;
        }
    }

    if(sysfs_get_string(g_Standby_State.wakeup[type].enable, enabled, sizeof(enabled))) {
        BDBG_ERR(("Unable to get %s wakeup enable status", g_sysWakePath[type].file));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }

    if(strncmp(enabled, "enabled", strlen(enabled)-1)) {
        BDBG_MSG(("%s wakeup  %s", g_sysWakePath[type].file, enabled));
        return NEXUS_SUCCESS;
    }

    if(sysfs_get(g_Standby_State.wakeup[type].count, count)) {
        BDBG_ERR(("Unable to get %s wakeup  count", g_sysWakePath[type].file));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }

err:
    return rc;
}

static NEXUS_Error NEXUS_Platform_P_SetSysWake(NEXUS_Platform_P_SysWake type, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned timeout;

    BDBG_ASSERT(type < NEXUS_Platform_P_SysWake_eMax);

    if(!g_Standby_State.wakeup[type].found) {
        NEXUS_Platform_P_FindSysWake(type);
        if(!g_Standby_State.wakeup[type].found) {
            BDBG_WRN(("Unable to find %s wakeup", g_sysWakePath[type].file));
            rc = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto err;
        }
    }

    BDBG_MSG(("Disable %s wakeup : %s", g_sysWakePath[type].file, g_Standby_State.wakeup[type].enable));
    if(sysfs_set_string(g_Standby_State.wakeup[type].enable, "disabled")) {
        BDBG_ERR(("Failed to disable %s wakeup", g_sysWakePath[type].file));
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err;
    }

    switch(type) {
        case NEXUS_Platform_P_SysWake_eTimer:
            timeout = pSettings->wakeupSettings.timeout;
            if(g_Standby_State.rtcFd > 0) {
                if(timeout) {
                    if(set_rtc_wake(timeout)) {
                        BDBG_ERR(("Failed to set RTC wakeup"));
                    }
                }
            } else {
                if(sysfs_set(g_Standby_State.wakeup[type].timeout, timeout)) {
                    BDBG_ERR(("Unable to set %s wakeup", g_sysWakePath[type].file));
                    rc = BERR_TRACE(BERR_OS_ERROR);
                    goto err;
                }
            }
            if(g_Standby_State.rtcFd <= 0 && !timeout) goto err;
            break;
        case NEXUS_Platform_P_SysWake_eGpio:
            if(!pSettings->wakeupSettings.gpio) goto err;
            break;
        default:
            break;
    }

    BDBG_MSG(("Enable %s wakeup : %s", g_sysWakePath[type].file, g_Standby_State.wakeup[type].enable));
    if(sysfs_set_string(g_Standby_State.wakeup[type].enable, "enabled")) {
        BDBG_ERR(("Failed to enable %s wakeup", g_sysWakePath[type].file));
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err;
    }

err:
    return rc;
}
#endif /* #if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT */

NEXUS_Error NEXUS_Platform_SetStandbySettings( const NEXUS_StandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Platform_P_SchedulersSet activate;
    NEXUS_Platform_P_SchedulersSet deactivated;


    /* stop schedulers that should not run at this standby level */
    rc = NEXUS_Platform_P_DeactivateSchedulers(pSettings->mode, pSettings->timeout, &deactivated);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);  goto error;}

    /* start schedulers that should run at this standby level */
    NEXUS_Platform_P_SchedulersSet_Init(&activate, pSettings->mode);
    NEXUS_Platform_P_ActivateSchedulers(&activate);

#if defined(NEXUS_WKTMR)

    rc = NEXUS_Platform_P_InitWakeupDriver();
    if (!rc) {
        wakeup_devices wakeups;
        /*Disable all wakeups first */
        wakeups.ir = wakeups.uhf = wakeups.keypad = wakeups.gpio = wakeups.cec = wakeups.transport = 1;
        if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_DISABLE, &wakeups)) {
            BDBG_ERR(("Unable to clear wakeup devices"));
            rc = BERR_TRACE(BERR_OS_ERROR);
        }

        /* Set new wakeup settings */
        wakeups.ir = pSettings->wakeupSettings.ir;
        wakeups.uhf = pSettings->wakeupSettings.uhf;
        wakeups.keypad = pSettings->wakeupSettings.keypad;
        wakeups.gpio = pSettings->wakeupSettings.gpio;
        wakeups.cec = pSettings->wakeupSettings.cec;
        wakeups.transport = pSettings->wakeupSettings.transport;
        if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_ENABLE, &wakeups)) {
            BDBG_ERR(("Unable to set wakeup devices"));
            rc = BERR_TRACE(BERR_OS_ERROR);
        }
    }

    rc = NEXUS_Platform_P_SetSysWake(NEXUS_Platform_P_SysWake_eTimer, pSettings);
    if (rc) { rc = BERR_TRACE(rc); }
    rc = NEXUS_Platform_P_SetSysWake(NEXUS_Platform_P_SysWake_eGpio, pSettings);
    if (rc) { rc = BERR_TRACE(rc); }


    if(pSettings->mode == NEXUS_StandbyMode_ePassive || pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BKNI_Memset(&g_Standby_State.standbyStatus, 0, sizeof(NEXUS_PlatformStandbyStatus));
        g_Standby_State.wakeupStatusCached = false;
    }
#endif /* #if defined(NEXUS_WKTMR) */

    rc = NEXUS_Platform_SetStandbySettings_driver(pSettings);
    if (rc!=NEXUS_SUCCESS) {
        /* NEXUS_Platform_SetStandbySettings_driver failed */
        rc = BERR_TRACE(rc);
        NEXUS_Platform_P_ActivateSchedulers(&deactivated); /* on failure restart schedulers that were stopped */
    }


#if defined(NEXUS_WKTMR)
    g_Standby_State.mode = pSettings->mode;
#endif
error:
    return rc;
#else
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif /* #if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT */
}

NEXUS_Error NEXUS_Platform_GetStandbyStatus(NEXUS_PlatformStandbyStatus *pStatus)
{
#if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT
#if defined(NEXUS_WKTMR)
    NEXUS_Error rc = NEXUS_SUCCESS;
    wakeup_devices wakeups;
    unsigned int wktmr_count=0, wkgpio_count=0;

    BKNI_Memset(&wakeups, 0, sizeof(wakeup_devices));

    if(!g_Standby_State.wakeupStatusCached) {
        rc |= NEXUS_Platform_P_InitWakeupDriver();
        if (rc) {
            if(g_Standby_State.mode == NEXUS_StandbyMode_eOn || g_Standby_State.mode == NEXUS_StandbyMode_eActive) {
                g_Standby_State.wakeupStatusCached = true;
            }
        } else {
            BKNI_Memset(&wakeups, 0, sizeof(wakeups));

            if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_ACK_STATUS, &wakeups)) {
                BDBG_ERR(("Unable to get wakeup status"));
                rc |= BERR_TRACE(BERR_OS_ERROR);
}
        }

        if(NEXUS_Platform_P_GetSysWake(NEXUS_Platform_P_SysWake_eTimer, &wktmr_count)) { rc |= BERR_TRACE(rc); }
        if(NEXUS_Platform_P_GetSysWake(NEXUS_Platform_P_SysWake_eGpio, &wkgpio_count)) { rc |= BERR_TRACE(rc); }

        if(wakeups.ir || wakeups.uhf || wakeups.keypad || wakeups.gpio || wakeups.cec || wakeups.transport || wktmr_count || wkgpio_count) {
            g_Standby_State.standbyStatus.wakeupStatus.ir = wakeups.ir;
            g_Standby_State.standbyStatus.wakeupStatus.uhf = wakeups.uhf;
            g_Standby_State.standbyStatus.wakeupStatus.keypad = wakeups.keypad;
            g_Standby_State.standbyStatus.wakeupStatus.cec = wakeups.cec;
            g_Standby_State.standbyStatus.wakeupStatus.transport = wakeups.transport;
            g_Standby_State.standbyStatus.wakeupStatus.gpio = wkgpio_count || wakeups.gpio;
            g_Standby_State.standbyStatus.wakeupStatus.timeout = wktmr_count;

            g_Standby_State.wakeupStatusCached = true;
        } else if(g_Standby_State.mode == NEXUS_StandbyMode_eOn || g_Standby_State.mode == NEXUS_StandbyMode_eActive) {
            g_Standby_State.wakeupStatusCached = true;
        }
    }

    pStatus->wakeupStatus.ir = g_Standby_State.standbyStatus.wakeupStatus.ir;
    pStatus->wakeupStatus.uhf = g_Standby_State.standbyStatus.wakeupStatus.uhf;
    pStatus->wakeupStatus.keypad = g_Standby_State.standbyStatus.wakeupStatus.keypad;
    pStatus->wakeupStatus.gpio = g_Standby_State.standbyStatus.wakeupStatus.gpio;
    pStatus->wakeupStatus.cec = g_Standby_State.standbyStatus.wakeupStatus.cec;
    pStatus->wakeupStatus.transport = g_Standby_State.standbyStatus.wakeupStatus.transport;
    pStatus->wakeupStatus.timeout = g_Standby_State.standbyStatus.wakeupStatus.timeout;

    return rc;
#else
    return NEXUS_Platform_GetStandbyStatus_driver(pStatus);
#endif
#else
    BSTD_UNUSED(pStatus);
    return NEXUS_SUCCESS;
#endif

}

#if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT
NEXUS_Error NEXUS_Platform_P_InitWakeupDriver(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char *devname;

    if (g_Standby_State.wakeFd > 0) {
        /* already open */
        return NEXUS_SUCCESS;
    }

    BKNI_Memset(&g_Standby_State, 0, sizeof(struct NEXUS_Platform_Standby_State));
    g_Standby_State.wakeFd = -1;
    g_Standby_State.rtcFd = -1;

    /* Open wakeup device driver */
    devname = NEXUS_GetEnv("NEXUS_WAKE_DEVICE_NODE");
    if (!devname) devname = "/dev/wake0";
    g_Standby_State.wakeFd = open(devname, O_RDWR);
    if ( g_Standby_State.wakeFd < 0 )
    {
        BDBG_ERR(("Unable to open wakeup driver. Wakeup devices may not work."));
        /* give message which points to solution */
        switch (errno)
        {
        case ENXIO:
            BDBG_ERR(("wakeupdriver has not been installed or wakeup nodes missing in DT. Are you running the nexus script? Are you running latest BOLT?"));
            break;
        case ENOENT:
            BDBG_ERR(("%s does not exist. Are you running the nexus script?", devname));
            break;
        default:
            BDBG_ERR(("%s error: %d", devname, errno));
            break;
        }
        rc = BERR_TRACE(BERR_OS_ERROR);
    }

#ifdef B_REFSW_ANDROID
    {
        struct stat buffer;
        if(!stat(RTC_WAKE_SYSFS, &buffer)) {
            BDBG_MSG(("using rtc wake timer"));
           g_Standby_State.rtcFd = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
           if (g_Standby_State.rtcFd < 0) {
               BDBG_ERR(("Failed to create rtc timer. Timer wakeup may not work."));
               rc = BERR_TRACE(BERR_OS_ERROR);
           }
        }
    }
#endif

    return rc;
}

void NEXUS_Platform_P_UninitWakeupDriver(void)
{
    if (g_Standby_State.rtcFd != -1) {
        close(g_Standby_State.rtcFd);
        g_Standby_State.rtcFd = -1;
    }
    if (g_Standby_State.wakeFd != -1) {
        close(g_Standby_State.wakeFd);
        g_Standby_State.wakeFd = -1;
    }
}
#endif  /* #if NEXUS_POWER_MANAGEMENT && defined(NEXUS_WKTMR) && !B_REFSW_SYSTEM_MODE_CLIENT */

#endif /* #if !defined(NEXUS_BASE_OS_linuxkernel) */

#if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT
static bool NEXUS_Platform_P_IsActiveStandbyScheduler(NEXUS_ModulePriority priority)
{
    bool activeStandby;
    switch (priority) {
    case NEXUS_ModulePriority_eIdle: activeStandby = false; break;
    case NEXUS_ModulePriority_eLow: activeStandby = false; break;
    case NEXUS_ModulePriority_eDefault: activeStandby = false; break;
    case NEXUS_ModulePriority_eHigh: activeStandby = false; break;
    default: activeStandby = true; break;
    }
    return activeStandby;
}

static void NEXUS_Platform_P_SchedulersSet_Clear(NEXUS_Platform_P_SchedulersSet *schedulers)
{
    BKNI_Memset(schedulers, 0, sizeof(*schedulers));
    return;
}

void NEXUS_Platform_P_SchedulersSet_Init(NEXUS_Platform_P_SchedulersSet *schedulers, NEXUS_StandbyMode mode)
{
    unsigned i;

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        bool set;
        switch(mode) {
        case NEXUS_StandbyMode_eOn:
            set = true;
            break;
        case NEXUS_StandbyMode_eActive:
            set = NEXUS_Platform_P_IsActiveStandbyScheduler(i);
            break;
        default:
            set = false;
            break;
        }
        schedulers->set[i] = set;
    }
    return;
}

void NEXUS_Platform_P_ActivateSchedulers(NEXUS_Platform_P_SchedulersSet *set)
{
    unsigned i;
    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if (set->set[i]) {
            NEXUS_Scheduler_SetState(i, NEXUS_Scheduler_State_eStarting);
        }
    }
    return;
}

NEXUS_Error NEXUS_Platform_P_DeactivateSchedulers(NEXUS_StandbyMode standbyMode, unsigned timeout, NEXUS_Platform_P_SchedulersSet *deactivate)
{
    NEXUS_Platform_P_SchedulersSet active;
    unsigned i;
    NEXUS_Time startTime;

    NEXUS_Time_Get(&startTime);

    NEXUS_Platform_P_SchedulersSet_Init(&active, standbyMode);
    NEXUS_Platform_P_SchedulersSet_Clear(deactivate);

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        if(!active.set[i]) { /* this scheduler should not run in this standbyMode */
            NEXUS_Scheduler_Status status;
            NEXUS_Scheduler_GetStatus(i, &status);
            if(status.state == NEXUS_Scheduler_State_eRunning) {
                deactivate->set[i] = true; /* it was running so deactivate it */
                NEXUS_Scheduler_SetState(i, NEXUS_Scheduler_State_eStopping);
            }
        }
    }

    for(;;) {
        NEXUS_Time now;
        unsigned active_count;
        long timeDiff;

        /* wait for schedulers to go idle */
        for(active_count=0,i=0;i<NEXUS_ModulePriority_eMax;i++) {
            if(deactivate->set[i]) {
                NEXUS_Scheduler_Status status;

                NEXUS_Scheduler_GetStatus(i, &status);
                if(status.state != NEXUS_Scheduler_State_eIdle) {
                    active_count++;
                }
            }
        }
        NEXUS_Time_Get(&now);
        timeDiff = NEXUS_Time_Diff(&now, &startTime);
        BDBG_MSG(("active_count %u time:%ld", active_count, timeDiff));
        if(active_count==0) {
            return NEXUS_SUCCESS;
        }
        if((unsigned)timeDiff > timeout) {
            /* recover state of schedulers we have tried to stop */
            NEXUS_Platform_P_ActivateSchedulers(deactivate);
            return BERR_TRACE(NEXUS_TIMEOUT);
        }
        BKNI_Sleep(10);
    }
}
#endif /* #if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT */
