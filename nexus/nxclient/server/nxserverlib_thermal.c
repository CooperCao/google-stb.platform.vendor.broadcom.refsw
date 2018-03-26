/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>

#include "nexus_graphics2d.h"
#if NEXUS_HAS_GRAPHICSV3D
#include "nexus_graphicsv3d.h"
#endif
#include "nxserverlib_impl.h"
#include "nxserverlib_thermal.h"

BDBG_MODULE(nxserverlib_thermal);

struct thermal_state {
    nxserver_t server;
    thermal_info thermal;
    cpufreq_info cpufreq;
    thermal_data data;
    BLST_Q_HEAD(priority_avail, priority_list) available;
    BLST_Q_HEAD(priority_active, priority_list) active;
    struct {
        unsigned over_temp_threshold;
        unsigned over_temp_reset;
        unsigned hysteresis;
        unsigned temp_target;
        unsigned theta_jc;
        unsigned polling_interval;
        unsigned temp_delay;
    } params;
    pthread_t thread;
    bool exit;
    bool has_pmu;
    NEXUS_VideoFormat orig_format;
    struct {
        FILE *fp;
        unsigned start_time;
        bool enabled;
    } log;
} g_thermal_state;


static int sysfs_set(const char *path, const char *filename, int val)
{
    FILE *fd;
    int ret = -1;
    char filepath[256];

    snprintf(filepath, 256, "%s/%s", path, filename);

    fd = fopen(filepath, "w");
    if (!fd) {
        BDBG_ERR(("Open failed %s", filepath));
        return ret;
    }
    ret = fprintf(fd, "%d", val);
    fclose(fd);

    return 0;
}

static int sysfs_get(const char *path, const char *filename, unsigned *val)
{
    FILE *fd;
    int ret = -1;
    char filepath[256];

    snprintf(filepath, 256, "%s/%s", path, filename);

    fd = fopen(filepath, "r");
    if (!fd) {
        BDBG_ERR(("Open failed %s", filepath));
        return ret;
    }
    ret = fscanf(fd, "%u", val);
    fclose(fd);

    return 0;
}

static int sysfs_get_string(const char *path, const char *filename, char *str)
{
    FILE *fd;
    int ret = -1;
    char filepath[256];

    snprintf(filepath, 256, "%s/%s", path, filename);

    fd = fopen(filepath, "r");
    if (!fd) {
        BDBG_ERR(("Open failed %s", filepath));
        return ret;
    }
    ret = fscanf(fd, "%256s", str);
    fclose(fd);

    return ret;
}

static char* sysfs_read_file(const char *path, const char *filename, char *buf, size_t len)
{
    FILE *fd;
    char *str = NULL;
    char filepath[256];

    snprintf(filepath, 256, "%s/%s", path, filename);

    fd = fopen(filepath, "r");
    if (!fd) {
        BDBG_ERR(("Open failed %s", filepath));
        return str;
    }

    str = fgets(buf, len, fd);
    fclose(fd);

    return str;
}

/* return instance id for file format such as trip_point_4_temp */
static int get_instance_id(const char *name, const char *match)
{
    char *str = strstr(name, match);
    if (str) {
        str += strlen(match);
        return atoi(str);
    }

    return -1;
}

/* Find trip point info of a thermal zone */
static NEXUS_Error scan_trip_point(const char *tz_name, const char *d_name, thermal_zone_info *tzi)
{
    int tp_id;

    tp_id = get_instance_id(d_name, "trip_point_");
    if (tp_id < 0) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (strstr(d_name, "temp")) {
        sysfs_get(tz_name, d_name, &tzi->tp[tp_id].temp);
        BDBG_MSG(("Thermal Zone %s node %s, Trip %d, Temp %u", tz_name, d_name, tp_id, tzi->tp[tp_id].temp));
        tzi->num_trip_points++;
    } else if (strstr(d_name, "hyst")) {
        sysfs_get(tz_name, d_name, &tzi->tp[tp_id].hyst);
        BDBG_MSG(("Thermal Zone %s node %s, Trip %d, Hyst %u", tz_name, d_name, tp_id, tzi->tp[tp_id].hyst));
    } else if (strstr(d_name, "type")) {
        sysfs_get_string(tz_name, "type", tzi->tp[tp_id].type);
        BDBG_MSG(("Thermal Zone %s node %s, Trip %d, Type %s", tz_name, d_name, tp_id, tzi->tp[tp_id].type));
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error scan_thermal_zones(const char *tz_name, thermal_zone_info *tzi)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    DIR *dir;
    struct dirent *ent;
    char filename[256];

    memset(filename, 0, sizeof(filename));
    snprintf(filename, 256, "%s/%s", THERMAL_SYSFS, tz_name);

    dir = opendir(filename);
    if (!dir) {
        BDBG_ERR(("Could not open Thermal Zone %s", filename));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    sysfs_get_string(filename, "type", tzi->type);

    /* detect trip points and cdev attached to this tzone */
    tzi->num_cooling_devices = 0;
    tzi->num_trip_points = 0;

    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, "trip_point")) {
            rc = scan_trip_point(filename, ent->d_name, tzi);
            if (rc) {BERR_TRACE(rc); goto err;}
        } else if (strstr(ent->d_name, "cdev")) {
            /*if (!find_tzone_cdev(ent, filename, tzi);*/ /* TODO : Do we need to scan the cdev bindings? */
        }
    }

    closedir(dir);

    BDBG_MSG(("TZ %d has %d trip points", tzi->instance, tzi->num_trip_points));

err:
    return rc;
}

static NEXUS_Error scan_cooling_devices(const char *cdev_name, cooling_device_info *cdi)
{
    char filename[256];

    memset(filename, 0, sizeof(filename));
    snprintf(filename, 256, "%s/%s", THERMAL_SYSFS, cdev_name);

    sysfs_get_string(filename, "type", cdi->type);
    sysfs_get(filename, "max_state", (unsigned*)&cdi->max_state);
    sysfs_get(filename, "cur_state", (unsigned*)&cdi->cur_state);
    BDBG_MSG(("Cooling Device %s type %s, Max State %d, Cur State %d", cdev_name, cdi->type, cdi->max_state, cdi->cur_state));

    return NEXUS_SUCCESS;
}

static NEXUS_Error probe_thermal_sysfs(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    thermal_info *thermal = &g_thermal_state.thermal;
    DIR *dir;
    struct dirent *ent;
    int i;

    dir = opendir(THERMAL_SYSFS);
    if (!dir) {
        BDBG_WRN(("No thermal sysfs"));
        return NEXUS_NOT_AVAILABLE;
    }

    /* read thermal zones and cooling devices info */
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, CDEV)) {
            i = thermal->num_cooling_devices++;
            if (i >= MAX_NUM_CDEV) {
                BDBG_WRN(("Max Number of Cooling Devices exceeded %s:%d", ent->d_name, i));
                continue;
            }
            BDBG_MSG(("Found Cooling Device: %s (%d)", ent->d_name, i));
            thermal->cdi[i].instance = get_instance_id(ent->d_name, "device");
            if (thermal->cdi[i].instance < 0) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err;
            }
            rc = scan_cooling_devices(ent->d_name, &thermal->cdi[i]);
            if (rc) {BERR_TRACE(rc); goto err;}
        } else if (strstr(ent->d_name, TZONE)) {
            i = thermal->num_thermal_zones++;
            if (i >= MAX_NUM_TZONE) {
                BDBG_WRN(("Max Number of Thermal Zones exceeded %s:%d", ent->d_name, i));
                continue;
            }
            BDBG_MSG(("Found Thermal Zone: %s (%d)", ent->d_name, i));
            thermal->tzi[i].instance = get_instance_id(ent->d_name, "zone");
            if (thermal->tzi[i].instance < 0) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err;
            }
            rc = scan_thermal_zones(ent->d_name, &thermal->tzi[i]);
            if (rc) {BERR_TRACE(rc); goto err;}
        }
    }

err:
    closedir(dir);

    if (!thermal->num_thermal_zones) {
        BDBG_WRN(("No Thermal Zones found"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    } else {
        BDBG_MSG(("Found %d Thermal Zone(s), %d Cooling Devices(s)", thermal->num_thermal_zones, thermal->num_cooling_devices));
    }

    return rc;
}

static NEXUS_Error cpufreq_get_available_frequencies(void)
{
    char buf[256];
    char *str;
    unsigned i=0;

    if (!sysfs_read_file(CPUFREQ_SYSFS, "scaling_available_frequencies", buf, sizeof(buf))) {
        BDBG_ERR(("Could not read cpufreq sysfs"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    str = strtok (buf," ");
    while (str != NULL) {
        unsigned freq = atoi(str);
        if (freq) {
            if (i < MAX_NUM_FREQ) {
                g_thermal_state.cpufreq.avail_freqs[i++] = freq;
                BDBG_MSG(("Freq %d", freq));
            } else {
                BDBG_WRN(("Max num frequencies exceeded"));
            }
        }
        str = strtok (NULL, " ");
    }
    g_thermal_state.cpufreq.num_p_states = i;

    return NEXUS_SUCCESS;
}

static NEXUS_Error probe_cpufreq_sysfs(void)
{
    NEXUS_Error rc;

    rc = cpufreq_get_available_frequencies();
    if (rc) {
        return BERR_TRACE(rc);
    }

    return rc;
}

int prev_total=0, prev_idle=0;
static int get_cpu_load(void)
{
    char buf[256];
    char *str;
    int total=0, idle=0, i=0;
    int cpu_load = 0;

    if (!sysfs_read_file("/proc", "stat", buf, sizeof(buf))) {
        BDBG_ERR(("Could not read /proc/stat"));
        return -1;
    }
    str = strtok (buf," ");
    while (str != NULL) {
        str = strtok (NULL, " ");
        if(str != NULL){
            total += atoi(str);
            if(i == 3)
                idle = atoi(str);
            i++;
        }
    }

    if (prev_total && prev_idle) {
        cpu_load = 100 - (((idle - prev_idle)*100)/(total - prev_total));
    }
    prev_total = total;
    prev_idle = idle;

    return cpu_load;
}

static void get_temp(void)
{
    int i;
    unsigned *temp = g_thermal_state.data.temp;
    char path[256];

    for(i=0; i<g_thermal_state.thermal.num_thermal_zones; i++) {
        BDBG_ASSERT(i < MAX_NUM_TZONE);
        memset(path, 0, sizeof(path));
        snprintf(path, 256, "%s/%s%d", THERMAL_SYSFS, TZONE, g_thermal_state.thermal.tzi[i].instance);
        sysfs_get(path, "temp", &temp[i]);
    }
}

static void get_power(void)
{
    unsigned power, div;

    if (!g_thermal_state.has_pmu) {
        return;
    }

    if (sysfs_get(PMU_SYSFS, POWER_RAW, &power)) {
        g_thermal_state.has_pmu = false;
        return;
    }
    power *= 2; /* TODO : Can we parse the scale? */

    g_thermal_state.data.power.instant = power;
    g_thermal_state.data.power.samples[g_thermal_state.data.power.idx++] = power;
    if (g_thermal_state.data.power.idx >= MAX_NUM_POWER_SAMPLES) g_thermal_state.data.power.idx = 0;
    g_thermal_state.data.power.total += power;
    div = g_thermal_state.data.power.samples[g_thermal_state.data.power.idx]?MAX_NUM_POWER_SAMPLES:g_thermal_state.data.power.idx;
    g_thermal_state.data.power.average = g_thermal_state.data.power.total/div;
    g_thermal_state.data.power.total -= g_thermal_state.data.power.samples[g_thermal_state.data.power.idx];

    return;
}

static NEXUS_Error throttle_cpu_frequency(cooling_agent *agent, unsigned level)
{
    if (level >= g_thermal_state.cpufreq.num_p_states) {
        BDBG_WRN(("Pstate %u not supported", level));
        return NEXUS_NOT_SUPPORTED;
    }

    BDBG_MSG(("Limit cpufreq to Pstate %u", level));
    if (sysfs_set(CPUFREQ_SYSFS, "scaling_max_freq", g_thermal_state.cpufreq.avail_freqs[level])) {
        BDBG_ERR(("Could not set cpufreq sysfs"));
        return BERR_TRACE(NEXUS_OS_ERROR);
    }

    agent->cur_level = level;

    return NEXUS_SUCCESS;
}

static NEXUS_Error cpu_idle_injection(cooling_agent *agent, unsigned level)
{
    int i, state;
    char filename[256];


    for(i=0; i<g_thermal_state.thermal.num_cooling_devices; i++) {
        if (!strncmp(g_thermal_state.thermal.cdi[i].type, "intel_powerclamp", 16))
            break;
    }
    if(i == g_thermal_state.thermal.num_cooling_devices) {
        BDBG_WRN(("Cpu Idle Injection is not supported"));
        return NEXUS_NOT_SUPPORTED;
    }

    state = (level * g_thermal_state.thermal.cdi[i].max_state)/agent->levels;
    if (state < 0 || state > g_thermal_state.thermal.cdi[i].max_state) {
        BDBG_WRN(("CPU Idle state %d not supported", state));
        return NEXUS_NOT_SUPPORTED;
    }

    /* TODO : Store this at init time */
    memset(filename, 0, sizeof(filename));
    snprintf(filename, 256, "%s/%s%d", THERMAL_SYSFS, "cooling_device", g_thermal_state.thermal.cdi[i].instance);
    BDBG_MSG(("Set idle injection %d", state));
    if (sysfs_set(filename, "cur_state", state)) {
        BDBG_ERR(("Could not set cpufreq sysfs"));
        return BERR_TRACE(NEXUS_OS_ERROR);
    }
    agent->cur_level = level;

    return NEXUS_SUCCESS;
}

#if NEXUS_HAS_GRAPHICSV3D
static NEXUS_Error scale_graphics3d_frequency(cooling_agent *agent, unsigned level)
{
    NEXUS_Error rc;
    unsigned step = 100/(agent->levels+1);
    unsigned scale = (100 - step*level);

    BDBG_MSG(("Setting Graphics3D Scale to %d", scale));
    rc = NEXUS_Graphicsv3d_SetFrequencyScaling(scale);
    if (rc) {rc = BERR_TRACE(rc); goto err;}
    agent->cur_level = level;

err:
    return rc;
}
#endif

static NEXUS_Error scale_graphics2d_frequency(cooling_agent *agent, unsigned level)
{
    NEXUS_Error rc;
    unsigned step = 100/(agent->levels+1);
    unsigned scale = (100 - step*level);

    BDBG_MSG(("Graphics2D Scale set to %d", scale));
    rc = NEXUS_Graphics2D_SetFrequencyScaling(scale);
    if (rc) {rc = BERR_TRACE(rc); goto err;}
    agent->cur_level = level;

err:
    return rc;
}

static NEXUS_Error set_display_format(cooling_agent *agent, unsigned level)
{
    NEXUS_Error rc;
    struct b_session *session = NULL;
    unsigned i;

    BSTD_UNUSED(agent);

    for(i=0; i<NXCLIENT_MAX_SESSIONS; i++) {
        if(g_thermal_state.server->session[i]->hdmiOutput) {
            session = g_thermal_state.server->session[i];
            break;
        }
    }

    if (session) {
        NxClient_DisplayStatus status;
        NxClient_DisplaySettings settings = session->nxclient.displaySettings;
        rc = NxClient_P_GetDisplayStatus(session, &status);
        if (rc) {rc = BERR_TRACE(rc); goto err;}
        if (!level) {
            /* Restore video format */
            settings.format = g_thermal_state.orig_format;
        } else {
            /* Switch format to lower supported format */
            g_thermal_state.orig_format = settings.format;
            if (status.hdmi.status.connected) {
                NEXUS_VideoFormat format;
                for (format = NEXUS_VideoFormat_eNtsc; format < NEXUS_VideoFormat_eMax; format++) {
                    if (status.hdmi.status.videoFormatSupported[format]) {
                        settings.format = format;
                        break;
                    }
                }
            } else {
                settings.format = NEXUS_VideoFormat_eNtsc;
            }
        }
        if (settings.format != session->nxclient.displaySettings.format) {
            BDBG_MSG(("Setting display format to  %s", lookup_name(g_videoFormatStrs, settings.format)));
            rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &settings);
            if (rc) {rc = BERR_TRACE(rc);}
        }
    } else {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

err:
    return rc;
}

static NEXUS_Error stop_pip_decode(cooling_agent *agent, unsigned level)
{
    nxclient_t client;

    BSTD_UNUSED(agent);
    BSTD_UNUSED(level);

    for (client = BLST_D_FIRST(&g_thermal_state.server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
            if (connect->settings.simpleVideoDecoder[0].windowCapabilities.type == NxClient_VideoWindowType_ePip) {
                BDBG_MSG(("%s Pip Video Decode", level?"Release":"Acquire"));
                if (level) {
                    b_connect_release(client, connect);
                } else {
                    b_connect_acquire(client, connect);
                }
            }
        }
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error notify_app_callback(cooling_agent *agent, unsigned level)
{
    unsigned i;

    for(i=0; i<NXCLIENT_MAX_SESSIONS; i++) {
        if(g_thermal_state.server->session[i]) {
            g_thermal_state.server->session[i]->callbackStatus.coolingAgentChanged++;
        }
    }
    agent->cur_level = level;

    return NEXUS_SUCCESS;
}

#define COOLING_AGENT(name, func) {#name, func, 0, 0}

cooling_agent g_cooling_agents[] = {
    COOLING_AGENT(cpu_pstate, throttle_cpu_frequency),
    COOLING_AGENT(cpu_idle, cpu_idle_injection),
#if NEXUS_HAS_GRAPHICSV3D
    COOLING_AGENT(v3d, scale_graphics3d_frequency),
#endif
    COOLING_AGENT(m2mc, scale_graphics2d_frequency),
    COOLING_AGENT(user, notify_app_callback),
};

cooling_agent g_forced_agents[] = {
    COOLING_AGENT(display, set_display_format),
    COOLING_AGENT(pip, stop_pip_decode),
};

static void init_cooling_agents(void)
{
    unsigned i, j;
    unsigned num_cooling_agents = sizeof(g_cooling_agents)/sizeof(g_cooling_agents[0]);
    unsigned default_levels=4;
    priority_list *item;

    if(!BLST_Q_FIRST(&g_thermal_state.available)) {
        /* Init default Cooling Agents */
        for(i=0; i<num_cooling_agents; i++) {
            g_cooling_agents[i].levels = default_levels;
            BDBG_MSG(("Init Cooling Agent %s (%d)", g_cooling_agents[i].name, g_cooling_agents[i].levels));
        }

        /* Init default priority list */
        for(i=1; i<=default_levels; i++) {
            for(j=0; j<num_cooling_agents; j++) {
                item=BKNI_Malloc(sizeof(*item));
                item->agent = &g_cooling_agents[j];
                item->level = i;
                BLST_Q_INSERT_TAIL(&g_thermal_state.available, item, link);
                BDBG_MSG(("Init Priority Table %s (%u:%u)", item->agent->name, item->level, item->agent->levels));
            }
        }
    } else {
        BDBG_MSG(("Priority List already initialized from config file"));
    }

    /* Initialize cooling agents */
    for(i=0; i<num_cooling_agents; i++) {
        g_cooling_agents[i].func(&g_cooling_agents[i], 0);
    }

    /* Add agent for forced display format change and pip decode*/
    for (i=0; i<sizeof(g_forced_agents)/sizeof(g_forced_agents[0]); i++) {
        item=BKNI_Malloc(sizeof(*item));
        item->agent = &g_forced_agents[i];
        item->level = 1;
        item->agent->levels = 1;
        BLST_Q_INSERT_TAIL(&g_thermal_state.available, item, link);
        BDBG_MSG(("Init Priority Table %s (%u:%u)", item->agent->name, item->level, item->agent->levels));
    }


    return;
}

static void uninit_cooling_agents(void)
{
    priority_list *item;
    while ((item=BLST_Q_FIRST(&g_thermal_state.available))) {
        BLST_Q_REMOVE_HEAD(&g_thermal_state.available, link);
        BKNI_Free(item);
    }
    while ((item=BLST_Q_FIRST(&g_thermal_state.active))) {
        BLST_Q_REMOVE_HEAD(&g_thermal_state.active, link);
        BKNI_Free(item);
    }
}

static void log_data(void)
{
    struct timeval tv;
    unsigned time;

    if (!g_thermal_state.log.enabled)
        return;

    if (!g_thermal_state.log.fp) {
        g_thermal_state.log.fp = fopen("thermal_log.csv", "w");
        BDBG_ASSERT(g_thermal_state.log.fp);

        gettimeofday(&tv, NULL);
        g_thermal_state.log.start_time = tv.tv_sec;
        fprintf(g_thermal_state.log.fp, "Time, Temp, Over Temp, Hyst");
        if (g_thermal_state.has_pmu)
            fprintf(g_thermal_state.log.fp,", Power, Power Avg");
        fprintf(g_thermal_state.log.fp,", CPU Load\n");
    }

    gettimeofday(&tv, NULL);
    time = tv.tv_sec - g_thermal_state.log.start_time;
    fprintf(g_thermal_state.log.fp, "%u, %f, %f, %f", time, (double)g_thermal_state.data.temp[0]/1000, (double)g_thermal_state.params.over_temp_threshold/1000, (double)g_thermal_state.params.temp_target/1000);
    if (g_thermal_state.has_pmu)
        fprintf(g_thermal_state.log.fp,", %u, %u", g_thermal_state.data.power.instant, g_thermal_state.data.power.average);
    fprintf(g_thermal_state.log.fp,", %d\n", get_cpu_load());
    fflush(g_thermal_state.log.fp);
}

static void log_close(void)
{
    if (g_thermal_state.log.fp)
        fclose(g_thermal_state.log.fp);
}

static void *thermal_monitor_thread(void *context)
{
    nxserver_t server = g_thermal_state.server;
    unsigned power_target = 0;
    enum cooling_state {
        cooling_state_normal,
        cooling_state_apply_agent,
        cooling_state_remove_agent,
        cooling_state_wait
    } state = 0;

    BSTD_UNUSED(context);

    BDBG_MSG(("Starting Thermal Monitor Thread ..."));

    get_power();
    g_thermal_state.params.temp_target = g_thermal_state.params.over_temp_threshold - g_thermal_state.params.hysteresis;

    while(!g_thermal_state.exit) {
        priority_list *priority;
        cooling_agent *agent;

        BKNI_AcquireMutex(server->settings.lock);
        if (server->settings.watchdog) {
            unsigned timeout = g_thermal_state.params.temp_delay / 1000 * 2;
            /* default is twice the sleep time, but we want to be very conservative with nxserver watchdogs,
            so use a 15 second min. */
            if (timeout < 15) timeout = 15;
            nxserver_p_pet_watchdog(server, &server->watchdog.state, timeout);
        }
        get_temp();
        BKNI_ReleaseMutex(server->settings.lock);

        BDBG_MSG(("Temp %u", g_thermal_state.data.temp[0]));
        if (g_thermal_state.has_pmu)
            BDBG_MSG(("Instant Power %u, Average Power %u", g_thermal_state.data.power.instant, g_thermal_state.data.power.average));
        BDBG_MSG(("Current State : %u", state));

        log_data();

        if (g_thermal_state.data.temp[0] > g_thermal_state.params.over_temp_threshold) {
            switch(state) {
                case cooling_state_normal:
                    if (g_thermal_state.has_pmu) {
                        unsigned power_delta = ((g_thermal_state.data.temp[0] - g_thermal_state.params.temp_target)*1000)/g_thermal_state.params.theta_jc;
                        power_target = g_thermal_state.data.power.average - power_delta;
                        BDBG_MSG(("Calculated Power Target %u", power_target));
                    }
                    state = cooling_state_apply_agent;
                    break;
                case cooling_state_apply_agent:
                case cooling_state_remove_agent:
                    state = cooling_state_wait;
                    break;
                case cooling_state_wait:
                    if (g_thermal_state.has_pmu) {
                        BDBG_MSG(("Average Power %u, Power Target %u", g_thermal_state.data.power.average, power_target));
                        if (g_thermal_state.data.power.instant < power_target) break;
                    }
                    if (BLST_Q_EMPTY(&g_thermal_state.available)) break;
                    state = cooling_state_apply_agent;
                    break;
            }
        } else if (g_thermal_state.data.temp[0] < g_thermal_state.params.temp_target) {
            switch(state) {
                case cooling_state_wait:
                    state = cooling_state_remove_agent;
                    break;
                case cooling_state_remove_agent:
                    if (BLST_Q_EMPTY(&g_thermal_state.active)) {
                        state = cooling_state_normal;
                        break;
                    }
                case cooling_state_apply_agent:
                    state = cooling_state_wait;
                    break;
                default:
                    break;
            }
        } else {
            switch(state) {
                case cooling_state_apply_agent:
                case cooling_state_remove_agent:
                    state = cooling_state_wait;
                    break;
                default:
                    break;
            }
        }

        BDBG_MSG(("Next State : %u", state));

        if (state == cooling_state_apply_agent) {
            /* Apply Policy */
            BKNI_AcquireMutex(server->settings.lock);
            priority = BLST_Q_FIRST(&g_thermal_state.available);
            if (priority) {
                agent = priority->agent;
                BDBG_WRN(("Applying Cooling agent %s, Temp %u", agent->name, g_thermal_state.data.temp[0]));
                agent->func(agent, priority->level);
                BLST_Q_REMOVE(&g_thermal_state.available, priority, link);
                BLST_Q_INSERT_HEAD(&g_thermal_state.active, priority, link);
            }
            BKNI_ReleaseMutex(server->settings.lock);
        } else if (state == cooling_state_remove_agent){
            /* Remove Policy */
            BKNI_AcquireMutex(server->settings.lock);
            priority = BLST_Q_FIRST(&g_thermal_state.active);
            if (priority) {
                agent = priority->agent;
                BDBG_WRN(("Removing Cooling agent %s, Temp %u", agent->name, g_thermal_state.data.temp[0]));
                agent->func(agent, priority->level-1);
                BLST_Q_REMOVE(&g_thermal_state.active, priority, link);
                BLST_Q_INSERT_HEAD(&g_thermal_state.available, priority, link);
            }
            BKNI_ReleaseMutex(server->settings.lock);
        } else {
            /* Wait */
            unsigned delay = state==cooling_state_normal?g_thermal_state.params.polling_interval:g_thermal_state.params.temp_delay;
            if (g_thermal_state.has_pmu) {
                unsigned duration, sleep_ms;
                for(duration=0, sleep_ms=100; duration < delay; duration += sleep_ms) {
                    get_power();
                    BKNI_Sleep(sleep_ms);
                }
            } else {
                BKNI_Sleep(delay);
            }
        }
    }
    if (server->settings.watchdog) {
        nxserver_p_pet_watchdog(server, &server->watchdog.state, 0);
    }

    log_close();

    return NULL;
}

static NEXUS_Error parse_config_file(char *filename)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    FILE *fp;

    BDBG_MSG(("Reading thermal configuration from %s", filename));

    fp = fopen(filename, "r");
    if(!fp) {
        BDBG_WRN(("Thermal config file %s not found. Using defaults", filename));
        return rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    while (!feof(fp)) {
        char buf[256];
        char *s;

        BKNI_Memset(buf, 0, 256);
        fgets(buf, 256, fp);

        s = buf;
        s += strspn(s, " \t");
        if (*s && *s != '#') {
            char *str;
            str = strchr(s, '\n');
            if (str) *str = 0;
            str = strchr(s, '#');
            if (str) *str = 0;
            str = strchr(s, '=');
            if (str) {
                if(strstr(s, "over_temp_threshold")) {
                    g_thermal_state.params.over_temp_threshold = 1000*atoi(++str);
                } else if (strstr(s, "over_temp_reset")) {
                    g_thermal_state.params.over_temp_reset = 1000*atoi(++str);
                } else if (strstr(s, "temp_hysteresis")) {
                    g_thermal_state.params.hysteresis = 1000*atof(++str);
                } else if (strstr(s, "poll_interval")) {
                    g_thermal_state.params.polling_interval = 1000*atoi(++str);
                } else if (strstr(s, "temp_delay")) {
                    g_thermal_state.params.temp_delay = 1000*atoi(++str);
                } else if (strstr(s, "theta_jc")) {
                    g_thermal_state.params.theta_jc = 1000*atoi(++str);
                } else if (strstr(s, "enable_log")) {
                    g_thermal_state.log.enabled = atoi(++str);
                } else if (strstr(s, "agent")) {
                    cooling_agent *agent=NULL;
                    priority_list *item=NULL;
                    unsigned i;
                    str++;
                    for(i=0; i<sizeof(g_cooling_agents)/sizeof(g_cooling_agents[0]); i++) {
                        if(strstr(str, g_cooling_agents[i].name)) {
                            agent = &g_cooling_agents[i];
                            agent->levels++;
                            break;
                        }
                    }
                    if(!agent) {
                        BDBG_WRN(("Agent %s not found. Check config file", str));
                        continue;
                    }
                    item = BKNI_Malloc(sizeof(*item));
                    if(!item) {
                        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                        goto err;
                    }
                    item->agent = agent;
                    item->level = agent->levels;
                    BLST_Q_INSERT_TAIL(&g_thermal_state.available, item, link);
                }
            }
        }
    }

err:
    return rc;
}

static void print_thermal_config(void)
{
    priority_list *item;
    BDBG_MSG(("Starting Thermal Monitor with following configuration"));
    BDBG_MSG(("over_temp_threshold = %u", g_thermal_state.params.over_temp_threshold));
    BDBG_MSG(("over_temp_reset = %u", g_thermal_state.params.over_temp_reset));
    BDBG_MSG(("temp_hysteresis = %u", g_thermal_state.params.hysteresis));
    BDBG_MSG(("poll_interval = %u", g_thermal_state.params.polling_interval));
    BDBG_MSG(("temp_delay = %u", g_thermal_state.params.temp_delay));
    BDBG_MSG(("Priority Table :"));
    for(item=BLST_Q_FIRST(&g_thermal_state.available); item; item=BLST_Q_NEXT(item, link)) {
        BDBG_MSG(("Agent : %s, Level %u", item->agent->name, item->level));
    }
}

static void set_avs_reset_threshold(void)
{
    unsigned threshold = g_thermal_state.params.over_temp_reset;
    unsigned park_high = threshold - 10000; /* TODO : Should it be configurable? */
    unsigned park_low  = park_high - 10000;

    NEXUS_Platform_SetOverTempResetThreshold(threshold, park_high, park_low);
}

NEXUS_Error nxserver_p_thermal_init(nxserver_t server)
{
    NEXUS_Error rc;

    BKNI_Memset(&g_thermal_state, 0, sizeof(g_thermal_state));
    BLST_Q_INIT(&g_thermal_state.available);
    BLST_Q_INIT(&g_thermal_state.active);
    g_thermal_state.params.over_temp_threshold = 75000;
    g_thermal_state.params.over_temp_reset = 110000;
    g_thermal_state.params.hysteresis = 2500;
    g_thermal_state.params.theta_jc = 5500;
    g_thermal_state.params.polling_interval = 1000;
    g_thermal_state.params.temp_delay = 2000;
    g_thermal_state.has_pmu = true;
    g_thermal_state.server = server;

    rc = parse_config_file(server->settings.thermal.thermal_config_file);
    if (rc) return BERR_TRACE(rc);

    rc = probe_thermal_sysfs();
    if (rc) return rc; /* no BERR_TRACE. this is normal. */

    rc = probe_cpufreq_sysfs();
    if (rc) return BERR_TRACE(rc);

    init_cooling_agents();

    set_avs_reset_threshold();

    print_thermal_config();

    rc = pthread_create(&g_thermal_state.thread, NULL, thermal_monitor_thread, NULL);
    if (rc) return BERR_TRACE(rc);

    BDBG_WRN(("Started Thermal Monitor"));

    return rc;
}

void nxserver_p_thermal_uninit(nxserver_t server)
{
    BSTD_UNUSED(server);
    if (g_thermal_state.thread) {
        g_thermal_state.exit = true;
        pthread_join(g_thermal_state.thread, NULL);
    }

    uninit_cooling_agents();
}

NEXUS_Error nxserver_get_thermal_status(nxclient_t client, NxClient_ThermalStatus *pStatus)
{
    priority_list *priority = BLST_Q_FIRST(&g_thermal_state.active);
    BSTD_UNUSED(client);
    pStatus->temperature = g_thermal_state.data.temp[0];
    if(priority) {
        pStatus->userDefined = !strncmp(priority->agent->name, "user", 4);
        pStatus->level = priority->level;
    }
    return NEXUS_SUCCESS;
}
