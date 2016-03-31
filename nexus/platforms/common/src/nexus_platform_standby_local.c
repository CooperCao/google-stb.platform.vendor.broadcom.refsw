/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "nexus_platform_standby.h"
#include "nexus_platform_local_priv.h"
#include "bstd.h"
#include "bkni.h"

#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM && !B_REFSW_SYSTEM_MODE_CLIENT
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

BDBG_MODULE(nexus_platform_standby_local);


#define WKTMR_SYSFS "/sys/bus/platform/drivers/brcm-waketimer"
#define RTC_WAKE_SYSFS "/sys/class/rtc/rtc0/wakealarm"

#define BUF_SIZE 256

static struct NEXUS_Platform_Standby_State {
    int wakeFd;
    int rtcFd;
    NEXUS_PlatformStandbyMode mode;
    NEXUS_PlatformStandbyStatus standbyStatus;
    bool wakeupStatusCached;
    struct NEXUS_Platform_WakeupTimer_State {
        char wktmr_enable[BUF_SIZE];
        char wktmr_count[BUF_SIZE];
        char wktmr_timeout[BUF_SIZE];
        bool wktmr_found;
        bool use_rtc;
    } wkTimer;
} g_Standby_State;

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

int set_rtc_wake(unsigned timeout)
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

void NEXUS_Platform_P_FindWakeTimer(void)
{
    DIR * dir = opendir(WKTMR_SYSFS);
    struct dirent *ent;

    if(dir) {
        while ((ent = readdir(dir)) != NULL) {
            if(strstr(ent->d_name, "waketimer")) {
                snprintf(g_Standby_State.wkTimer.wktmr_enable, BUF_SIZE, "%s/%s/%s", WKTMR_SYSFS, ent->d_name, "power/wakeup");
                snprintf(g_Standby_State.wkTimer.wktmr_count, BUF_SIZE, "%s/%s/%s", WKTMR_SYSFS, ent->d_name, "power/wakeup_count");
                snprintf(g_Standby_State.wkTimer.wktmr_timeout, BUF_SIZE, "%s/%s/%s", WKTMR_SYSFS, ent->d_name, "timeout");
                g_Standby_State.wkTimer.wktmr_found = true;
                break;
            }
        }
        closedir(dir);
    }

    return;
}

NEXUS_Error NEXUS_Platform_P_GetWakeTimer(unsigned int *count)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    char enabled[32];

    if(!g_Standby_State.wkTimer.wktmr_found) {
        NEXUS_Platform_P_FindWakeTimer();
        if(!g_Standby_State.wkTimer.wktmr_found) {
            BDBG_WRN(("Unable to find wakeup timer"));
            rc = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto err;
        }
    }

    if(sysfs_get_string(g_Standby_State.wkTimer.wktmr_enable, enabled, sizeof(enabled))) {
        BDBG_ERR(("Unable to get wake timer enable status"));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }

    if(strncmp(enabled, "enabled", strlen(enabled)-1)) {
        BDBG_MSG(("Wake timer %s", enabled));
        return NEXUS_SUCCESS;
    }

    if(sysfs_get(g_Standby_State.wkTimer.wktmr_count, count)) {
        BDBG_ERR(("Unable to get wake timer count"));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }

err:
    return rc;
}

NEXUS_Error NEXUS_Platform_P_SetWakeTimer(unsigned timeout)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(!g_Standby_State.wkTimer.wktmr_found) {
        NEXUS_Platform_P_FindWakeTimer();
        if(!g_Standby_State.wkTimer.wktmr_found) {
            BDBG_WRN(("Unable to find wakeup timer"));
            rc = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto err;
        }
    }

    if(sysfs_set_string(g_Standby_State.wkTimer.wktmr_enable, "disabled")) {
        BDBG_ERR(("Failed to disable wake timer"));
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err;
    }

    if(g_Standby_State.rtcFd > 0) {
        if(timeout) {
            if(set_rtc_wake(timeout)) {
                BDBG_ERR(("Failed to set RTC wakeup"));
            }
        }
    } else {
        if(sysfs_set(g_Standby_State.wkTimer.wktmr_timeout, timeout)) {
            BDBG_ERR(("Unable to set wakeup timer"));
            rc = BERR_TRACE(BERR_OS_ERROR);
            goto err;
        }
    }

    if(g_Standby_State.rtcFd > 0 || timeout) {
        if(sysfs_set_string(g_Standby_State.wkTimer.wktmr_enable, "enabled")) {
            BDBG_ERR(("Failed to enable wake timer"));
            rc = BERR_TRACE(BERR_OS_ERROR);
            goto err;
        }
    }

err:
    return rc;
}
#endif

NEXUS_Error NEXUS_Platform_SetStandbySettings( const NEXUS_PlatformStandbySettings *pSettings )
{
#if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT
    NEXUS_Error rc = NEXUS_SUCCESS;

#if NEXUS_CPU_ARM
    wakeup_devices wakeups;

    rc = NEXUS_Platform_P_InitWakeupDriver();
    if (!rc) {
        /*Disable all wakeups first */
        wakeups.ir = wakeups.uhf = wakeups.keypad = wakeups.gpio = wakeups.cec = wakeups.transport = 1;
        if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_DISABLE, &wakeups) ||
                NEXUS_Platform_P_SetWakeTimer(0)) {
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
        if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_ENABLE, &wakeups) ||
                NEXUS_Platform_P_SetWakeTimer(pSettings->wakeupSettings.timeout)) {
            BDBG_ERR(("Unable to set wakeup devices"));
            rc = BERR_TRACE(BERR_OS_ERROR);
            return rc;
        }
    }

    if(pSettings->mode == NEXUS_PlatformStandbyMode_ePassive || pSettings->mode == NEXUS_PlatformStandbyMode_eDeepSleep) {
        BKNI_Memset(&g_Standby_State.standbyStatus, 0, sizeof(NEXUS_PlatformStandbyStatus));
        g_Standby_State.wakeupStatusCached = false;
    }
#endif

    rc = NEXUS_Platform_SetStandbySettings_driver(pSettings);
    if (rc) { rc = BERR_TRACE(rc); }

#if NEXUS_CPU_ARM
    g_Standby_State.mode = pSettings->mode;
#endif

    return rc;
#else
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_Platform_GetStandbyStatus(NEXUS_PlatformStandbyStatus *pStatus)
{
#if NEXUS_POWER_MANAGEMENT && !B_REFSW_SYSTEM_MODE_CLIENT
#if NEXUS_CPU_ARM
    NEXUS_Error rc = NEXUS_SUCCESS;
    wakeup_devices wakeups;
    unsigned int wktmr_count=0;

    if(!g_Standby_State.wakeupStatusCached) {
        rc = NEXUS_Platform_P_InitWakeupDriver();
        if (rc) {
            if(g_Standby_State.mode == NEXUS_PlatformStandbyMode_eOn || g_Standby_State.mode == NEXUS_PlatformStandbyMode_eActive) {
                g_Standby_State.wakeupStatusCached = true;
            }
            return BERR_TRACE(rc);
        }

        BKNI_Memset(&wakeups, 0, sizeof(wakeups));

        if(ioctl(g_Standby_State.wakeFd, BRCM_IOCTL_WAKEUP_ACK_STATUS, &wakeups) ||
               NEXUS_Platform_P_GetWakeTimer(&wktmr_count)) {
            BDBG_ERR(("Unable to get wakeup status"));
            rc = BERR_TRACE(BERR_OS_ERROR);
            goto err;
        }

        if(wakeups.ir || wakeups.uhf || wakeups.keypad || wakeups.gpio || wakeups.cec || wakeups.transport || wktmr_count) {
            g_Standby_State.standbyStatus.wakeupStatus.ir = wakeups.ir;
            g_Standby_State.standbyStatus.wakeupStatus.uhf = wakeups.uhf;
            g_Standby_State.standbyStatus.wakeupStatus.keypad = wakeups.keypad;
            g_Standby_State.standbyStatus.wakeupStatus.gpio = wakeups.gpio;
            g_Standby_State.standbyStatus.wakeupStatus.cec = wakeups.cec;
            g_Standby_State.standbyStatus.wakeupStatus.transport = wakeups.transport;
            g_Standby_State.standbyStatus.wakeupStatus.timeout = wktmr_count;

            g_Standby_State.wakeupStatusCached = true;
        } else if(g_Standby_State.mode == NEXUS_PlatformStandbyMode_eOn || g_Standby_State.mode == NEXUS_PlatformStandbyMode_eActive) {
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

err:
    return rc;
#else
    return NEXUS_Platform_GetStandbyStatus_driver(pStatus);
#endif
#else
    BSTD_UNUSED(pStatus);
    return NEXUS_SUCCESS;
#endif

}

#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM && !B_REFSW_SYSTEM_MODE_CLIENT
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
#endif
