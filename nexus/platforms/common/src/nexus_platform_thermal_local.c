/***************************************************************************
*     Copyright (c) 2006-2015, Broadcom Corporation*
*     All Rights Reserved*
*     Confidential Property of Broadcom Corporation*
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
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_platform.h"
#include "nexus_base.h"
#include "bstd.h"
#include "bkni.h"

#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM && !B_REFSW_SYSTEM_MODE_CLIENT
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <pthread.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <linux/netlink.h>

BDBG_MODULE(nexus_platform_thermal_local);

#define DEV_PATH         "/devices/virtual/thermal/thermal_zone"
#define THERMAL_SYSFS    "/sys/class/thermal/thermal_zone0/"
#define MAX_TRIP_POINTS  8

typedef struct NEXUS_Platform_P_ThermalEventMsg {
	unsigned temperature_change; /* temperature change event */
	unsigned zone;               /* Zone number in /sys/class/thermal/thermal_zoneX */
	unsigned tripnum;            /* Trip point number in /sys/class/thermal/thermal_zoneX/trip_point_Y */
	unsigned temperature;       /* Temperature, in millidegrees celsius */
} NEXUS_Platform_P_ThermalEventMsg;

typedef struct NEXUS_Platform_P_ThermalState {
    pthread_t thermalThread;
    struct {
        unsigned temp;
        unsigned hyst;
    } tripPoints[MAX_TRIP_POINTS];
    unsigned numTripPoints;
    unsigned initialThermalPoint;
    bool exit;
} NEXUS_Platform_P_ThermalState;

static NEXUS_Platform_P_ThermalState g_NEXUS_Platform_P_ThermalState;

/*
 * Look for ACTION, DEVPATH, SUBSYSTEM, TRIPNUM, TEMPERATURE, etc. variables
 */
static bool NEXUS_Platform_P_ParseThermalEvent_priv(NEXUS_Platform_P_ThermalEventMsg *msg, const char *line)
{
	const char *key;
	char *val;

	val = strchr(line, '=');
	/* Only care about 'KEY=value' lines */
	if (!val)
		return false;
	/* Split key/value */
	*(val++) = '\0';
	key = line;

	if (!strcmp("ACTION", key)) {
		if (!strcmp("change", val))
			msg->temperature_change = 1;
		return false;
	}
	if (!strcmp("DEVPATH", key)) {
		size_t devpath_len = strlen(DEV_PATH);
		if (!strncmp(DEV_PATH, val, devpath_len))
			msg->zone = atoi(val + devpath_len);
		return false;
	}
	if (!strcmp("SUBSYSTEM", key)) {
		return !strcmp("thermal", val);
	}
	if (!strcmp("TRIPNUM", key)) {
		msg->tripnum = atoi(val);
		return false;
	}
	if (!strcmp("TEMPERATURE", key)) {
		msg->temperature = atoi(val);
		return false;
	}

	return false;
}

/*
 * Parses a thermal kobject uevent notification
 *
 * Example string:
 *
 * change@/devices/virtual/thermal/thermal_zone0
 * ACTION=change
 * DEVPATH=/devices/virtual/thermal/thermal_zone0
 * SUBSYSTEM=thermal
 * TRIPNUM=2
 * TEMPERATURE=50634
 * SEQNUM=1491
 *
 */
static bool NEXUS_Platform_P_ParseThermalEvent(NEXUS_Platform_P_ThermalEventMsg *msg, const char *str, size_t len)
{
	size_t linelen;
	size_t i = 0;
	bool is_therm = false;

	memset(msg, 0, sizeof(*msg));

	while (i < len) {
		linelen = strlen(str + i);
		is_therm |= NEXUS_Platform_P_ParseThermalEvent_priv(msg, str + i);
		i += linelen + 1;
	}

	return is_therm;
}

static void NEXUS_Platform_P_PrintThermalMessage(char *buf, int len)
{
    int i = 0;
    BDBG_MSG(("Received Thermal Message:"));
    while (i < len) {
        BDBG_MSG(("%s", buf + i));
        i += strlen(buf + i) + 1;
    }
}

static void *NEXUS_Platform_P_ThermalMonitorThread(void *pParam)
{
    int fd, rc;
    struct sockaddr_nl nls;
	struct pollfd pfd;
    int len;
	char buf[512];

    BSTD_UNUSED(pParam);

    NEXUS_Platform_SetThermalScaling_driver(g_NEXUS_Platform_P_ThermalState.initialThermalPoint, g_NEXUS_Platform_P_ThermalState.numTripPoints);

    nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid() + (1 << 16); /* Set a unique port ID that does not clash with one that is used by Android */
	nls.nl_groups = -1;

    fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (fd < 0) {
        perror("socket");
		goto socket_error;
	}

    if (bind(fd, (struct sockaddr *)&nls, sizeof(nls))) {
		perror("bind");
		goto error;
	}

    memset(&pfd, 0, sizeof(pfd));
	pfd.events = POLLIN;
	pfd.fd = fd;

    BDBG_MSG(("Starting Thermal Monitor Thread ..."));

    while (!g_NEXUS_Platform_P_ThermalState.exit) {
		struct NEXUS_Platform_P_ThermalEventMsg msg;

        rc = poll(&pfd, 1, 1000);
        if(!rc)
            continue;
        if(rc == -1)
            break;

		len = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (len < 0) {
			perror("write");
            break;
		}

        NEXUS_Platform_P_PrintThermalMessage(buf, len);

		if (NEXUS_Platform_P_ParseThermalEvent(&msg, buf, len)) {
            unsigned thermal_point;

            BDBG_WRN(("Thermal event:"));
            BDBG_WRN(("Zone number: %d", msg.zone));
            BDBG_WRN(("Trip number: %d", msg.tripnum));
            BDBG_WRN(("Temperature: %d", msg.temperature));

            thermal_point = msg.tripnum;
            if(msg.temperature >= g_NEXUS_Platform_P_ThermalState.tripPoints[msg.tripnum].temp)
                thermal_point++;
            NEXUS_Platform_SetThermalScaling_driver(thermal_point, g_NEXUS_Platform_P_ThermalState.numTripPoints);
        }
	}

error:
    close(fd);
socket_error:
	return NULL;
}

static NEXUS_Error b_read_thermal_info(char *filename, unsigned *val)
{
    char buf[256];
    FILE *fp = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;

    fp = fopen(filename, "r");
    if(!fp) {
        rc = NEXUS_OS_ERROR;
        goto error;
    }
    if (!fgets(buf, 256, fp)) {
        rc = NEXUS_OS_ERROR;
        goto read_error;
    }
    *val = atoi(buf);

read_error:
    fclose(fp);
error:
    return rc;
}

NEXUS_Error NEXUS_Platform_P_InitThermalMonitor(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    DIR * dir;
    struct dirent *ent;
    unsigned i, temp=0;
    size_t syslen = strlen(THERMAL_SYSFS);

    memset(&g_NEXUS_Platform_P_ThermalState, 0, sizeof(g_NEXUS_Platform_P_ThermalState));

    if (!NEXUS_StrCmp(NEXUS_GetEnv("disable_thermal_monitor"), "y"))
        return NEXUS_SUCCESS;

    dir = opendir(THERMAL_SYSFS);
    if(!dir) {
        return NEXUS_NOT_AVAILABLE;
    }

    while ((ent = readdir(dir)) != NULL) {
        unsigned idx;
        char buf[256];
        char *str = strstr(ent->d_name, "trip_point_");
        if(str) {
            str += strlen("trip_point_");
            idx = atoi(str);
            if(idx >= MAX_TRIP_POINTS) {
                BDBG_ERR(("Exceeded Max Trip Points"));
                rc = NEXUS_NOT_SUPPORTED;
                break;
            }
            strncpy(buf, THERMAL_SYSFS, syslen);
            buf[syslen] = '\0';
            strncat(buf, ent->d_name, strlen(ent->d_name));

            if(strstr(ent->d_name, "temp")) {
                rc = b_read_thermal_info(buf, &g_NEXUS_Platform_P_ThermalState.tripPoints[idx].temp);
                if(rc) { break; }
                g_NEXUS_Platform_P_ThermalState.numTripPoints++;
            }
            if(strstr(ent->d_name, "hyst")) {
                rc = b_read_thermal_info(buf, &g_NEXUS_Platform_P_ThermalState.tripPoints[idx].hyst);
                if(rc) { break; }
            }
        } else if (strstr(ent->d_name, "temp")) {
            strncpy(buf, THERMAL_SYSFS, syslen);
            buf[syslen] = '\0';
            strncat(buf, "temp", 4);
            rc = b_read_thermal_info(buf, &temp);
            if(rc) { break; }
        }
    }
    closedir(dir);
    if(rc) {
        BDBG_WRN(("Init Thermal Monitor Failed"));
        return rc;
    }


    if(g_NEXUS_Platform_P_ThermalState.numTripPoints == 0) {
        BDBG_WRN(("No thermal trip points defined"));
        return NEXUS_NOT_SUPPORTED;
    }

    for(i=0; i<g_NEXUS_Platform_P_ThermalState.numTripPoints; i++) {
        BDBG_MSG(("Trip Point %u : %u %u", i, g_NEXUS_Platform_P_ThermalState.tripPoints[i].temp, g_NEXUS_Platform_P_ThermalState.tripPoints[i].hyst));
        if (temp >= g_NEXUS_Platform_P_ThermalState.tripPoints[i].temp)
            g_NEXUS_Platform_P_ThermalState.initialThermalPoint++;
    }
    BDBG_MSG(("Current temperature %u, Thermal Point %u", temp, g_NEXUS_Platform_P_ThermalState.initialThermalPoint));

    if(pthread_create(&g_NEXUS_Platform_P_ThermalState.thermalThread, NULL, NEXUS_Platform_P_ThermalMonitorThread, NULL)) {
        perror("pthread");
        return NEXUS_OS_ERROR;
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitThermalMonitor(void)
{
    g_NEXUS_Platform_P_ThermalState.exit = true;
    if(g_NEXUS_Platform_P_ThermalState.thermalThread)
        pthread_join(g_NEXUS_Platform_P_ThermalState.thermalThread, NULL);
}
#endif
