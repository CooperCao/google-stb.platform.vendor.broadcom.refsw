/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

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

#include "sysfs.h"

BDBG_MODULE(nxserverlib_thermal);

enum callback_type {
    callback_type_agent_changed,
    callback_type_config_changed,
    callback_type_max
};

struct thermal_state {
    nxserver_t server;
    thermal_info thermal;
    cpufreq_info cpufreq;
    thermal_data data;
    BLST_Q_HEAD(config_list, thermal_config) configs;
    thermal_config *active_config;
    struct {
        unsigned park_high;
        unsigned park_low;
    } params;
    pthread_t thread;
    bool enabled;
    bool has_pmu;
    NEXUS_VideoFormat display_format;
} g_thermal_state;

extern const namevalue_t agents[];

static NEXUS_Error init_p_thermal_config(thermal_config *config);
static void uninit_p_thermal_config(thermal_config *config);

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
        sysfs_get_string(tz_name, "type", tzi->tp[tp_id].type, sizeof(tzi->tp[tp_id].type));
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

    sysfs_get_string(filename, "type", tzi->type, sizeof(tzi->type));

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

    sysfs_get_string(filename, "type", cdi->type, sizeof(cdi->type));
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
    unsigned i=0, j, tmp;

    if (sysfs_get_string(CPUFREQ_SYSFS, "scaling_available_frequencies", buf, sizeof(buf))) {
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

    /* Sort to make sure frequencies are in descending order */
    for (i=0 ; i<( g_thermal_state.cpufreq.num_p_states-1); i++) {
        for (j=0; j<g_thermal_state.cpufreq.num_p_states-i-1; j++) {
            if (g_thermal_state.cpufreq.avail_freqs[j] < g_thermal_state.cpufreq.avail_freqs[j+1]) {
                tmp = g_thermal_state.cpufreq.avail_freqs[j];
                g_thermal_state.cpufreq.avail_freqs[j] = g_thermal_state.cpufreq.avail_freqs[j+1];
                g_thermal_state.cpufreq.avail_freqs[j+1] = tmp;
            }
        }
    }
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

static void notify_app_callback(enum callback_type cb_type)
{
    unsigned i;

    if (!g_thermal_state.enabled) return;

    for(i=0; i<NXCLIENT_MAX_SESSIONS; i++) {
        if(g_thermal_state.server->session[i]) {
            switch(cb_type) {
                case callback_type_agent_changed:
                    g_thermal_state.server->session[i]->callbackStatus.coolingAgentChanged++;
                    break;
                case callback_type_config_changed:
                    g_thermal_state.server->session[i]->callbackStatus.thermalConfigChanged++;
                    break;
                default:
                    break;
            }
        }
    }
    return;
}

static NEXUS_Error throttle_cpu_frequency(cooling_agent *agent, unsigned level)
{
    if (level >= g_thermal_state.cpufreq.num_p_states) {
        BDBG_WRN(("Pstate %u not supported", level));
        return NEXUS_NOT_SUPPORTED;
    }

    BDBG_MSG(("Limit cpufreq to Pstate %u, Frequency %u", level, g_thermal_state.cpufreq.avail_freqs[level]));
    if (sysfs_set(CPUFREQ_SYSFS, "scaling_max_freq", g_thermal_state.cpufreq.avail_freqs[level])) {
        BDBG_ERR(("Could not set cpufreq sysfs"));
        return BERR_TRACE(NEXUS_OS_ERROR);
    }

    agent->cur_level = level;
    notify_app_callback(callback_type_agent_changed);

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
    notify_app_callback(callback_type_agent_changed);

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
    notify_app_callback(callback_type_agent_changed);

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
    notify_app_callback(callback_type_agent_changed);

err:
    return rc;
}

#if NEXUS_HAS_HDMI_OUTPUT
static NEXUS_Error set_display_format(cooling_agent *agent, unsigned level)
{
    NEXUS_Error rc;
    struct b_session *session = NULL;
    unsigned i;

    BSTD_UNUSED(agent);

    if (g_thermal_state.server->settings.externalApp.enabled) {
        return NEXUS_SUCCESS;
    }

    for(i=0; i<NXCLIENT_MAX_SESSIONS; i++) {
        if(g_thermal_state.server->session[i] && g_thermal_state.server->session[i]->hdmiOutput) {
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
            settings.format = g_thermal_state.display_format;
        } else {
            /* Switch format to lower supported format */
            g_thermal_state.display_format = settings.format;
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
        if (settings.format && settings.format != session->nxclient.displaySettings.format) {
            BDBG_MSG(("Setting display format to  %s", lookup_name(g_videoFormatStrs, settings.format)));
            rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &settings);
            if (rc) {rc = BERR_TRACE(rc);}
        }
    } else {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    notify_app_callback(callback_type_agent_changed);

err:
    return rc;
}
#endif

static NEXUS_Error stop_decode(unsigned level, bool pip)
{
    nxclient_t client;
    NxClient_VideoWindowType window_type = pip?NxClient_VideoWindowType_ePip:NxClient_VideoWindowType_eMain;

    for (client = BLST_D_FIRST(&g_thermal_state.server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
            if (connect->settings.simpleVideoDecoder[0].windowCapabilities.type == window_type) {
                BDBG_MSG(("%s %s Video Decode", level?"Release":"Acquire", pip?"Pip":"Main"));
                if (level) {
                    /* Dont use b_connect_release because we don't want to assign connect to another client */
                    if (is_video_request(connect)) {
                        release_video_decoders(connect);
                    }
                    release_audio_decoders(connect);
                    release_audio_playbacks(connect);
                    release_video_encoders(connect);
                } else {
                    b_connect_acquire(client, connect);
                }
            }
        }
    }
    notify_app_callback(callback_type_agent_changed);

    return NEXUS_SUCCESS;
}

static NEXUS_Error stop_pip_decode(cooling_agent *agent, unsigned level)
{
    BSTD_UNUSED(agent);
    return stop_decode(level, true);
}

static NEXUS_Error stop_main_decode(cooling_agent *agent, unsigned level)
{
    BSTD_UNUSED(agent);
    return stop_decode(level, false);
}

static NEXUS_Error notify_user(cooling_agent *agent, unsigned level)
{
    agent->cur_level = level;
    notify_app_callback(callback_type_agent_changed);
    return NEXUS_SUCCESS;
}

#define COOLING_AGENT(name, type, func, max) {#name, NxClient_CoolingAgent_e##type, func, 0, 0, max}

cooling_agent g_cooling_agents[] = {
    COOLING_AGENT(cpu_pstate, CpuPstate, throttle_cpu_frequency, 4),
    COOLING_AGENT(cpu_idle, CpuIdle, cpu_idle_injection, 4),
#if NEXUS_HAS_GRAPHICSV3D
    COOLING_AGENT(v3d, Graphics3D, scale_graphics3d_frequency, 4),
#endif
    COOLING_AGENT(m2mc, Graphics2D, scale_graphics2d_frequency, 4),
#if NEXUS_HAS_HDMI_OUTPUT
    COOLING_AGENT(display, Display, set_display_format, 1),
#endif
    COOLING_AGENT(stop_pip, StopPip, stop_pip_decode, 1),
    COOLING_AGENT(stop_main, StopMain, stop_main_decode, 1),
    COOLING_AGENT(user, User, notify_user, (unsigned)-1),
};

static void apply_cooling_agent(priority_list *priority)
{
    if (priority) {
        cooling_agent *agent = priority->agent;
        BDBG_WRN(("Applying Cooling agent %s, Level %u, Temp %u", agent->name, priority->level, g_thermal_state.data.temp[0]));
        agent->func(agent, priority->level);
        BLST_Q_REMOVE(&g_thermal_state.active_config->available, priority, link);
        BLST_Q_INSERT_HEAD(&g_thermal_state.active_config->active, priority, link);
        NEXUS_GetTimestamp(&priority->last_applied_time);
    }
}

static void remove_cooling_agent(priority_list *priority)
{
    if (priority) {
        cooling_agent *agent = priority->agent;
        BDBG_WRN(("Removing Cooling agent %s, Level %u, Temp %u", agent->name, priority->level, g_thermal_state.data.temp[0]));
        agent->func(agent, priority->level-1);
        BLST_Q_REMOVE(&g_thermal_state.active_config->active, priority, link);
        BLST_Q_INSERT_HEAD(&g_thermal_state.active_config->available, priority, link);
        NEXUS_GetTimestamp(&priority->last_removed_time);
    }
}

static void set_active_config(void)
{
    unsigned temp = g_thermal_state.data.temp[0];
    thermal_config *config, *last_enabled=NULL;
    for (config=BLST_Q_FIRST(&g_thermal_state.configs); config; config=BLST_Q_NEXT(config, link)) {
        if (!BLST_Q_EMPTY(&config->available)) {
            break;
        }
        if (config->enabled) {
            last_enabled=config;
        }
    }
    if (config) {
        if (config != g_thermal_state.active_config) {
            if (temp >= config->config.overTempThreshold || !g_thermal_state.active_config->enabled) {
                g_thermal_state.active_config = config;
                notify_app_callback(callback_type_config_changed);
                BDBG_WRN(("Active Configuration set to %u", g_thermal_state.active_config->config.overTempThreshold));
            }
        } else {
            if (temp < (config->config.overTempThreshold - config->config.hysteresis) && BLST_Q_EMPTY(&config->active)) {
                if (last_enabled) {
                    g_thermal_state.active_config = last_enabled;
                    notify_app_callback(callback_type_config_changed);
                    BDBG_WRN(("Active Configuration set to %u", g_thermal_state.active_config->config.overTempThreshold));
                }
            }
        }
    } else {
        g_thermal_state.active_config = last_enabled?last_enabled:BLST_Q_FIRST(&g_thermal_state.configs);
    }
}

#define ABOVE_THRESHOLD \
    ((g_thermal_state.data.temp[0] > g_thermal_state.active_config->config.overTempThreshold) || \
     (g_thermal_state.data.power.average >= g_thermal_state.active_config->config.overPowerThreshold))
#define BELOW_THRESHOLD \
    ((g_thermal_state.data.temp[0] < temp_target) && \
     (g_thermal_state.data.power.average < g_thermal_state.active_config->config.overPowerThreshold))
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

    while(g_thermal_state.enabled) {
        priority_list *priority;
        unsigned temp_target;

        BKNI_AcquireMutex(server->settings.lock);
        if (server->settings.watchdog) {
            unsigned timeout = g_thermal_state.active_config->config.tempDelay / 1000 * 2;
            /* default is twice the sleep time, but we want to be very conservative with nxserver watchdogs,
            so use a 15 second min. */
            if (timeout < 15) timeout = 15;
            nxserver_p_pet_watchdog(server, &server->watchdog.state, timeout);
        }
        get_temp();

        /* Set active config */
        set_active_config();
        BKNI_ReleaseMutex(server->settings.lock);

        BDBG_MSG(("Temp %u", g_thermal_state.data.temp[0]));
        if (g_thermal_state.has_pmu)
            BDBG_MSG(("Instant Power %u, Average Power %u", g_thermal_state.data.power.instant, g_thermal_state.data.power.average));
        BDBG_MSG(("Current State : %u", state));

        temp_target = g_thermal_state.active_config->config.overTempThreshold - g_thermal_state.active_config->config.hysteresis;

        if (ABOVE_THRESHOLD) {
            switch(state) {
                case cooling_state_normal:
                    if (g_thermal_state.has_pmu) {
                        unsigned power_delta = ((g_thermal_state.data.temp[0] - temp_target)*1000)/g_thermal_state.active_config->config.thetaJC;
                        if (g_thermal_state.data.power.average > power_delta) {
                            power_target = g_thermal_state.data.power.average - power_delta;
                        }
                        if (power_target > g_thermal_state.active_config->config.overPowerThreshold) {
                            power_target = g_thermal_state.active_config->config.overPowerThreshold;
                        }
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
                    /* All agents have been applied. Keep sending notification to client */
                    if (BLST_Q_EMPTY(&g_thermal_state.active_config->available) &&
                        !BLST_Q_EMPTY(&g_thermal_state.active_config->active)) {
                        notify_app_callback(callback_type_agent_changed);
                        break;
                    }
                    state = cooling_state_apply_agent;
                    break;
            }
        } else if (BELOW_THRESHOLD) {
            switch(state) {
                case cooling_state_wait:
                    state = cooling_state_remove_agent;
                    break;
                case cooling_state_remove_agent:
                    if (BLST_Q_EMPTY(&g_thermal_state.active_config->active)) {
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
            priority = BLST_Q_FIRST(&g_thermal_state.active_config->available);
            apply_cooling_agent(priority);
            BKNI_ReleaseMutex(server->settings.lock);
        } else if (state == cooling_state_remove_agent){
            /* Remove Policy */
            BKNI_AcquireMutex(server->settings.lock);
            priority = BLST_Q_FIRST(&g_thermal_state.active_config->active);
            remove_cooling_agent(priority);
            BKNI_ReleaseMutex(server->settings.lock);
        } else {
            /* Wait */
            unsigned delay = state==cooling_state_normal?g_thermal_state.active_config->config.pollInterval:g_thermal_state.active_config->config.tempDelay;
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

    return NULL;
}

static NEXUS_Error init_p_thermal_config(thermal_config *config)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NxClient_ThermalConfiguration *nxConfig = &config->config;
    unsigned i, j;

    BLST_Q_INIT(&config->available);
    BLST_Q_INIT(&config->active);

    for (i=0; i<sizeof(nxConfig->priorityTable)/sizeof(nxConfig->priorityTable[0]); i++) {
        cooling_agent *agent=NULL;
        priority_list *item=NULL;

        if (!nxConfig->priorityTable[i].agent) break;

        for (j=0; j<sizeof(g_cooling_agents)/sizeof(g_cooling_agents[0]); j++) {
            if(nxConfig->priorityTable[i].agent == g_cooling_agents[j].type) {
                agent = &g_cooling_agents[j];
                if (agent->levels >= agent->max_level) {
                    BDBG_WRN(("Exceeded max levels for Agent %s(%u:%u)", lookup_name(agents, nxConfig->priorityTable[i].agent), agent->levels, agent->max_level));
                    agent = NULL;
                    rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    break;
                }
                agent->levels++;
                break;
            }
        }
        if (rc) { break; }
        if (!agent) {
            BDBG_WRN(("Agent %s not found. Check config file", lookup_name(agents, nxConfig->priorityTable[i].agent)));
            continue;
        }
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            break;
        }
        BKNI_Memset(item, 0, sizeof(*item));
        item->agent = agent;
        item->level = agent->levels;
        BLST_Q_INSERT_TAIL(&config->available, item, link);
    }

    if (rc) {
        uninit_p_thermal_config(config);
    }
    return rc;
}

static NEXUS_Error init_thermal_config(const char *filename)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    thermal_config *config;

    config = BKNI_Malloc(sizeof(*config));
    if(!config) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(config, 0, sizeof(*config));

    rc = parse_thermal_config_file(filename, &config->config);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err;
    }

    rc = init_p_thermal_config(config);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err;
    }

    config->enabled = true;
    BLST_Q_INSERT_TAIL(&g_thermal_state.configs, config, link);
    g_thermal_state.active_config = config;

err:
    if (rc) {
        BKNI_Free(config);
    }
    return rc;
}

static void uninit_p_thermal_config(thermal_config *config)
{
    priority_list *item;
    while ((item=BLST_Q_FIRST(&config->available))) {
        BLST_Q_REMOVE_HEAD(&config->available, link);
        item->agent->levels--;
        BKNI_Free(item);
    }
    while ((item=BLST_Q_FIRST(&config->active))) {
        BLST_Q_REMOVE_HEAD(&config->active, link);
        item->agent->levels--;
        BKNI_Free(item);
    }
}

static void uninit_thermal_config(void)
{
    thermal_config *config;
    while ((config=BLST_Q_FIRST(&g_thermal_state.configs))) {
        uninit_p_thermal_config(config);
        BLST_Q_REMOVE_HEAD(&g_thermal_state.configs, link);
        BKNI_Free(config);
    }
    g_thermal_state.active_config = NULL;
}

static void print_thermal_config(void)
{
    unsigned i;
    NxClient_ThermalConfiguration *config = &g_thermal_state.active_config->config;
    BDBG_MSG(("Starting Thermal Monitor with following configuration"));
    BDBG_MSG(("over_temp_threshold = %u", config->overTempThreshold));
    BDBG_MSG(("over_temp_reset = %u", config->overTempReset));
    BDBG_MSG(("over_power_threshold = %u", config->overPowerThreshold));
    BDBG_MSG(("temp_hysteresis = %u", config->hysteresis));
    BDBG_MSG(("poll_interval = %u", config->pollInterval));
    BDBG_MSG(("temp_delay = %u", config->tempDelay));
    BDBG_MSG(("theta_jc = %u", config->thetaJC));
    BDBG_MSG(("park_high = %u", g_thermal_state.params.park_high));
    BDBG_MSG(("park_low = %u", g_thermal_state.params.park_low));
    BDBG_MSG(("Priority Table :"));
    for (i=0; i<sizeof(config->priorityTable)/sizeof(config->priorityTable[0]); i++) {
        if (!config->priorityTable[i].agent) break;
        BDBG_MSG(("\tAgent : %s", lookup_name(agents, config->priorityTable[i].agent)));
    }
}

static void set_avs_reset_threshold(void)
{
    unsigned threshold = g_thermal_state.active_config->config.overTempReset;
    unsigned park_high = threshold - 10000; /* TODO : Should it be configurable? */
    unsigned park_low  = park_high - 10000;

    NEXUS_Platform_SetOverTempResetThreshold(threshold, park_high, park_low);

    g_thermal_state.params.park_high = park_high;
    g_thermal_state.params.park_low = park_low;
}

static void reset_cooling_agents(void)
{
    unsigned i;
    for(i=0; i<sizeof(g_cooling_agents)/sizeof(g_cooling_agents[0]); i++) {
        g_cooling_agents[i].func(&g_cooling_agents[i], 0);
    }
    return;
}

NEXUS_Error nxserver_p_thermal_init(nxserver_t server)
{
    NEXUS_Error rc;

    if(getenv("disable_thermal")) {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(&g_thermal_state, 0, sizeof(g_thermal_state));
    BLST_Q_INIT(&g_thermal_state.configs);
    g_thermal_state.has_pmu = true;
    g_thermal_state.server = server;

    rc = init_thermal_config(server->settings.thermal.thermal_config_file);
    if (rc && rc != NEXUS_NOT_AVAILABLE) return BERR_TRACE(rc);

    rc = probe_thermal_sysfs();
    if (rc) return rc; /* no BERR_TRACE. this is normal. */

    rc = probe_cpufreq_sysfs();
    if (rc) return BERR_TRACE(rc);

    reset_cooling_agents();

    set_avs_reset_threshold();

    print_thermal_config();

    g_thermal_state.enabled = true;

    rc = pthread_create(&g_thermal_state.thread, NULL, thermal_monitor_thread, NULL);
    if (rc) return BERR_TRACE(rc);

    BDBG_WRN(("Started Thermal Monitor"));

    return rc;
}

void nxserver_p_thermal_uninit(nxserver_t server)
{
    BSTD_UNUSED(server);
    g_thermal_state.enabled = false;
    if (g_thermal_state.thread) {
        pthread_join(g_thermal_state.thread, NULL);
    }
    reset_cooling_agents();
    uninit_thermal_config();
}

static void nxserver_p_get_thermal_status(NxClient_ThermalStatus *pStatus, priority_list *priority, unsigned index, bool in_use)
{
    BDBG_ASSERT(index<(sizeof(pStatus->priorityTable)/sizeof(pStatus->priorityTable[0])));
    pStatus->priorityTable[index].inUse = in_use;
    pStatus->priorityTable[index].lastAppliedTime = priority->last_applied_time;
    pStatus->priorityTable[index].lastRemovedTime = priority->last_removed_time;
}

NEXUS_Error nxserver_get_thermal_status(nxclient_t client, NxClient_ThermalStatus *pStatus)
{
    priority_list *priority;
    unsigned i=0;

    BSTD_UNUSED(client);

    if (!g_thermal_state.enabled) {
        return NEXUS_NOT_AVAILABLE;
    }

    if (!pStatus) {
        return NEXUS_NOT_INITIALIZED;
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->temperature = g_thermal_state.data.temp[0];
    if (g_thermal_state.active_config->enabled) {
        pStatus->activeTempThreshold = g_thermal_state.active_config->config.overTempThreshold;
    }
    for (priority=BLST_Q_LAST(&g_thermal_state.active_config->active); priority; priority=BLST_Q_PREV(priority, link)) {
        nxserver_p_get_thermal_status(pStatus, priority, i++, true);
    }
    for (priority=BLST_Q_FIRST(&g_thermal_state.active_config->available); priority; priority=BLST_Q_NEXT(priority, link)) {
        nxserver_p_get_thermal_status(pStatus, priority, i++, false);
    }

    return NEXUS_SUCCESS;
}

void nxserver_get_thermal_configuration_list(nxclient_t client, NxClient_ThermalConfigurationList *pConfigList)
{
    thermal_config *config;
    unsigned i=0;

    BSTD_UNUSED(client);

    if (!g_thermal_state.enabled) {
        return;
    }

    BKNI_Memset(pConfigList, 0, sizeof(*pConfigList));
    for (config=BLST_Q_FIRST(&g_thermal_state.configs); config; config=BLST_Q_NEXT(config, link)) {
        if (i > NXCLIENT_MAX_THERMAL_CONFIGS) {
            BDBG_WRN(("Exceed max configs!!"));
            break;
        }
        if (config->enabled) {
            pConfigList->tempThreshold[i++] = config->config.overTempThreshold;
        }
    }
}

NEXUS_Error nxserver_get_thermal_configuration(nxclient_t client, unsigned tempThreshold, NxClient_ThermalConfiguration *pConfig)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    thermal_config *config;

    BSTD_UNUSED(client);

    if (!g_thermal_state.enabled) {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(pConfig, 0, sizeof(*pConfig));
    for (config=BLST_Q_FIRST(&g_thermal_state.configs); config; config=BLST_Q_NEXT(config, link)) {
        if (tempThreshold == config->config.overTempThreshold) {
            *pConfig = config->config;
            break;
        }
    }
    if (!config) {
        return NEXUS_NOT_AVAILABLE;
    }

    return rc;
}

static bool is_last_enabled(thermal_config *config)
{
    thermal_config *cfg;

    for (cfg=BLST_Q_NEXT(config, link); cfg; cfg=BLST_Q_NEXT(cfg, link)) {
        if (cfg->enabled) {return false;}
    }

    return true;
}

static void disable_config(thermal_config *config)
{
    priority_list *priority;
    while((priority=BLST_Q_FIRST(&config->active))) {
        remove_cooling_agent(priority);
    }
    uninit_p_thermal_config(config);
    config->enabled = false;
}

static NEXUS_Error enable_config(thermal_config *config)
{
    NEXUS_Error rc = init_p_thermal_config(config);
    if (!rc) {
        config->enabled = true;
    }
    return rc;
}

static NEXUS_Error set_new_config(const NxClient_ThermalConfiguration *pConfig, thermal_config **cfg)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    thermal_config *config;

    config = BKNI_Malloc(sizeof(*config));
    if(!config) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(config, 0, sizeof(*config));

    config->config = *pConfig;

    rc = init_p_thermal_config(config);
    if (rc) {
        BKNI_Free(config);
        return BERR_TRACE(rc);
    }
    config->enabled = true;
    *cfg = config;

    return rc;
}

NEXUS_Error nxserver_set_thermal_configuration(nxclient_t client, unsigned tempThreshold, const NxClient_ThermalConfiguration *pConfig)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    thermal_config *config;

    BSTD_UNUSED(client);

    if (!g_thermal_state.enabled) {
        return NEXUS_NOT_AVAILABLE;
    }
    if (pConfig && pConfig->overTempThreshold != tempThreshold) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    for (config=BLST_Q_FIRST(&g_thermal_state.configs); config; config=BLST_Q_NEXT(config, link)) {
        if (tempThreshold <= config->config.overTempThreshold) {
            break;
        }
    }

    if (config && !is_last_enabled(config)) {
        BDBG_ERR(("Cannot set config %u. Disable active configs with higher threshold first", tempThreshold));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if(!pConfig) {
        if (config && config->enabled) {
            /* Disable config */
            if (tempThreshold == config->config.overTempThreshold) {
                disable_config(config);
            } else {
                BDBG_ERR(("Could not find config %u to disable", tempThreshold));
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        } else {
            /* Invalid */
            BDBG_ERR(("Cannot disable invalid config %u", tempThreshold));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    } else {
        if (config) {
            if (tempThreshold == config->config.overTempThreshold) {
                /* Update/Enable config */
                if (config->enabled) {
                    /* Return if no change */
                    if (!BKNI_Memcmp(&config->config, pConfig, sizeof(*pConfig))) {
                        BDBG_MSG(("No change in config"));
                        return rc;
                    }
                    disable_config(config);
                }
                config->config = *pConfig;
                rc = enable_config(config);
                if (rc) { rc = BERR_TRACE(rc); }
            } else {
                /* Insert new config before */
                if (!config->enabled) {
                    thermal_config *new;
                    rc = set_new_config(pConfig, &new);
                    if (rc) {
                        BERR_TRACE(rc);
                    } else {
                        BLST_Q_INSERT_BEFORE(&g_thermal_state.configs, config, new, link);
                    }
                } else {
                    BDBG_ERR(("Config %u is still enabled. Cannot insert new config %u", config->config.overTempThreshold, tempThreshold));
                    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
            }
        } else {
            /* Insert new config to tail */
            thermal_config *new;
            rc = set_new_config(pConfig, &new);
            if (rc) {
                BERR_TRACE(rc);
            } else {
                BLST_Q_INSERT_TAIL(&g_thermal_state.configs, new, link);
            }
        }
    }

    return rc;
}
