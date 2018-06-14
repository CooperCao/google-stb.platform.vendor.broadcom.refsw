/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "nxclient.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_surface_client.h"
#include "bkni.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "namevalue.h"
#include "thermal_config.h"
#include "sysfs.h"

BDBG_MODULE(thermal);

struct client_state{
    unsigned userLevel;
    NEXUS_VideoFormat format;
    unsigned surfaceClientId;
    NEXUS_SurfaceClientHandle surfaceClient;
    struct {
        NxClient_AllocResults allocResults;
        NEXUS_SimpleVideoDecoderHandle videoDecoder;
        NEXUS_SimpleAudioDecoderHandle audioDecoder;
        unsigned connectId;
    } decode[2];
    thermal_config config;
    pthread_t log_thread_id, cmd_thread_id;
    bool done;
    struct {
        int total;
        int idle;
    } cpu_stats;
    int priority;
    unsigned num_user_agents;
};

#define PMU_SYSFS "/sys/class/i2c-adapter/i2c-0/0-0040/iio:device0"
#define POWER_RAW "in_power2_raw"

static void print_thermal_config(thermal_config *config)
{
    NxClient_ThermalStatus status;
    unsigned i;

    NxClient_GetThermalStatus(&status);
    printf("\n");
    printf("Current Temperature : %u\n", status.temperature);
    printf("Over Temp Thershold : %u\n", config->over_temp_threshold);
    printf("Target Temperature  : %u\n", config->over_temp_threshold - config->hysteresis);
    printf("Over Temp Reset     : %u\n", config->over_temp_reset);
    printf("Poll Interval       : %u\n", config->polling_interval);
    printf("Temp Delay          : %u\n", config->temp_delay);
    printf("ThetaJc             : %u\n", config->theta_jc);
    printf("\n");
    printf("   Agent   | Level | In Use |\n");
    for (i=0; i<sizeof(config->priority_table)/sizeof(config->priority_table[0]); i++) {
        if (!config->priority_table[i].level) break;
        printf("%-11s|   %u   |   %u    |\n", config->priority_table[i].name, config->priority_table[i].level, status.priorityTable[i].inUse);
    }
}

static NEXUS_Error set_display_format(struct client_state *state, bool enable)
{
    NEXUS_Error rc;
    NxClient_DisplaySettings displaySettings;
    NxClient_DisplayStatus displayStatus;

    rc = NxClient_GetDisplayStatus(&displayStatus);
    if (rc) {return BERR_TRACE(rc);}
    NxClient_GetDisplaySettings(&displaySettings);
    if (enable) {
        state->format = displaySettings.format;
        if (displayStatus.hdmi.status.connected) {
            NEXUS_VideoFormat format;
            for (format = NEXUS_VideoFormat_eNtsc; format < NEXUS_VideoFormat_eMax; format++) {
                if (displayStatus.hdmi.status.videoFormatSupported[format]) {
                    displaySettings.format = format;
                    break;
                }
            }
        } else {
            displaySettings.format = NEXUS_VideoFormat_eNtsc;
        }
    } else {
        displaySettings.format = state->format;
    }
    BDBG_WRN(("Setting display format to  %s", lookup_name(g_videoFormatStrs, displaySettings.format)));
    rc = NxClient_SetDisplaySettings(&displaySettings);
    if (rc) { rc = BERR_TRACE(rc); }

    return rc;
}

static void disconnect_decode(struct client_state *state, unsigned idx)
{
    if (state->decode[idx].connectId) {
        NxClient_Disconnect(state->decode[idx].connectId);
    }
    if (state->decode[idx].videoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(state->decode[idx].videoDecoder);
        state->decode[idx].videoDecoder = NULL;
    }
    if (state->decode[idx].audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(state->decode[idx].audioDecoder);
        state->decode[idx].audioDecoder = NULL;
    }
    NxClient_Free(&state->decode[idx].allocResults);
}

static NEXUS_Error connect_decode(struct client_state *state, unsigned idx)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &state->decode[idx].allocResults);
    if (rc) return BERR_TRACE(rc);

    if (state->decode[idx].allocResults.simpleVideoDecoder[0].id) {
        state->decode[idx].videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(state->decode[idx].allocResults.simpleVideoDecoder[0].id);
    }
    if (state->decode[idx].allocResults.simpleAudioDecoder.id) {
        state->decode[idx].audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(state->decode[idx].allocResults.simpleAudioDecoder.id);
    }
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = state->decode[idx].allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = state->surfaceClientId;
    connectSettings.simpleVideoDecoder[0].windowId = 0;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = idx?NxClient_VideoWindowType_ePip:NxClient_VideoWindowType_eMain;
    connectSettings.simpleAudioDecoder.id = state->decode[idx].allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &state->decode[idx].connectId);
    if (rc) {
        disconnect_decode(state, idx);
        rc = BERR_TRACE(rc);
    }

    return rc;
}

static NEXUS_Error stop_pip_decode(struct client_state *state, bool enable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (enable)
        rc = connect_decode(state, 1); /*Connect decode takes away resources from other client and stops it */
    else
        disconnect_decode(state, 1);
    return rc;
}

static NEXUS_Error stop_main_decode(struct client_state *state, bool enable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (enable)
        rc = connect_decode(state, 0); /*Connect decode takes away resources from other client and stops it */
    else
        disconnect_decode(state, 0);
    return rc;
}

static NEXUS_Error standby(struct client_state *state, bool enable)
{
    NEXUS_Error rc;
    NxClient_StandbySettings standbySettings;
    BSTD_UNUSED(state);
    NxClient_GetDefaultStandbySettings(&standbySettings);
    standbySettings.settings.mode = enable?NEXUS_PlatformStandbyMode_eActive:NEXUS_PlatformStandbyMode_eOn;
    rc = NxClient_SetStandbySettings(&standbySettings);
    if (rc) {BERR_TRACE(rc); }
    return rc;
}

struct user_cooling_agents {
    char name[32];
    NEXUS_Error (*func) (struct client_state *state, bool enable);
} g_agents[] = {
    {"display", set_display_format},
    {"stop pip", stop_pip_decode},
    {"stop main", stop_main_decode},
    {"standby", standby}
};

static int get_current_priority(void)
{
    NxClient_ThermalStatus status;
    unsigned i;

    NxClient_GetThermalStatus(&status);
    for (i=0; i<sizeof(status.priorityTable)/sizeof(status.priorityTable[0]); i++) {
            if (!status.priorityTable[i].inUse)
                break;
    }
    return (int)--i;
}

static void thermal_callback(void *context, int param)
{
    struct client_state *state = context;
    int priority = get_current_priority();
    bool enable = priority>state->priority;
    int index = enable?priority:state->priority;

    BSTD_UNUSED(param);

    if (priority == state->priority) {
        /* We have probably reached the last agent and still in over temp. Nxserver keeps notifying application */
        BDBG_WRN(("Exhausted all cooling agents. Still in overtemp!!"));
        return;
    }

    BDBG_WRN(("%s Cooling Agent %s, priority %u", enable?"Applied":"Removed", state->config.priority_table[index].name, index));
    if (!strncmp(state->config.priority_table[index].name, "user", 4)) {
        unsigned level = state->config.priority_table[index].level-1;
        if (level < state->num_user_agents) {
            BDBG_WRN(("%s user agent %s", enable?"Applying":"Removing", g_agents[level].name));
            g_agents[level].func(state, enable?true:false);
        }
    }
    state->priority = priority;
}

static int get_cpu_load(struct client_state *context)
{
    char buf[256];
    char *str;
    int total=0, idle=0, i=0;
    int cpu_load = 0;

    if (sysfs_get_string("/proc", "stat", buf, sizeof(buf))) {
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

    if (context->cpu_stats.total && context->cpu_stats.idle) {
        cpu_load = 100 - (((idle - context->cpu_stats.idle)*100)/(total - context->cpu_stats.total));
    }
    context->cpu_stats.total = total;
    context->cpu_stats.idle = idle;

    return cpu_load;
}

static void *log_thread(void *context)
{
    struct client_state *state = context;
    FILE *fp;
    unsigned time, start_time, power;
    struct timeval tv;
    bool has_pmu = false;
    double over_temp_threshold = (double)state->config.over_temp_threshold/1000;
    double temp_target = (double)(state->config.over_temp_threshold - state->config.hysteresis)/1000;


    fp = fopen("thermal_log.csv", "w");
    BDBG_ASSERT(fp);

    gettimeofday(&tv, NULL);
    start_time = tv.tv_sec;
    fprintf(fp, "Time, Temp, Over Temp, Temp Target");
    if (!sysfs_get(PMU_SYSFS, POWER_RAW, &power)) {
        has_pmu = true;
        fprintf(fp,", Power");
    }
    fprintf(fp,", CPU Load\n");

    while (!state->done) {
        NxClient_ThermalStatus status;

        gettimeofday(&tv, NULL);
        time = tv.tv_sec - start_time;
        NxClient_GetThermalStatus(&status);

        fprintf(fp, "%u, %f, %f, %f", time, (double)status.temperature/1000, over_temp_threshold, temp_target);
        if (has_pmu) {
            sysfs_get(PMU_SYSFS, POWER_RAW, &power);
            power *= 2;
            fprintf(fp,", %u", power);
        }
        fprintf(fp,", %d\n", get_cpu_load(state));
        fflush(fp);
        BKNI_Sleep(1000);
    }

    if (fp)
        fclose(fp);

    return NULL;
}

static void *cmd_thread(void *context)
{
    struct client_state *state = context;
    int retval = 1;

    while(!state->done) {
        fd_set rfds;
        struct timeval timeout;
        char buffer[256];
        char *buf;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 250000;

        if(retval) {
            fflush(stdout); printf("thermal>"); fflush(stdout);
        }

        retval = select(1, &rfds, NULL, NULL, &timeout);
        if(retval == -1) {
            printf("Stdin Error\n");
            return NULL;
        } else if (!retval) {
            continue;
        }

        fgets(buffer, 256, stdin);
        if (feof(stdin)) break;
        buffer[strlen(buffer)-1] = 0; /* chop off \n */

        buf = strtok(buffer, ";");
        if (!buf) continue;

        do {
            if (!strncmp(buf, "info", 1)) {
                print_thermal_config(&state->config);
            } else if (!strncmp(buf, "q", 1)) {
                state->done = true;
            } else {
                printf("Unknown command\n");
            }
        } while ((buf = strtok(NULL, ";")));
    }

    return NULL;
}

int main(int argc, const char **argv)
{
    NEXUS_Error rc;
    struct client_state context;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_CallbackThreadSettings callbackThreadSettings;
    bool prompt = true;
    bool log = false;
    int curarg = 1;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-prompt") && argc>curarg+1) {
            if (!strcmp(argv[++curarg], "off")){
                prompt = false;
            }
        } else if (!strcmp(argv[curarg], "-log")) {
            log = true;
        } else {
            /*print_usage();*/
            return -1;
        }
        curarg++;
    }

    BKNI_Memset(&context, 0, sizeof(context));

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_UnregisterAcknowledgeStandby(NxClient_RegisterAcknowledgeStandby());

    rc = parse_thermal_config_file("nxclient/thermal.cfg", &context.config);
    if (rc) {
        BDBG_WRN(("Failed to get thermal config"));
        goto err;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video window */
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) BERR_TRACE(rc);
    context.surfaceClientId = allocResults.surfaceClient[0].id;
    context.surfaceClient = NEXUS_SurfaceClient_Acquire(context.surfaceClientId);

    context.priority = get_current_priority();
    context.num_user_agents = sizeof(g_agents)/sizeof(g_agents[0]);

    NxClient_GetDefaultCallbackThreadSettings(&callbackThreadSettings);
    callbackThreadSettings.coolingAgentChanged.callback = thermal_callback;
    callbackThreadSettings.coolingAgentChanged.context = &context;
    rc = NxClient_StartCallbackThread(&callbackThreadSettings);
    if (rc) {
        BDBG_WRN(("Failed to start callback thread"));
        goto err_callback;
    }

    if (log) {
        rc = pthread_create(&context.log_thread_id, NULL, log_thread, &context);
        if (rc) {
            BDBG_WRN(("Failed to start log thread"));
            goto err_log;
        }
    }

    if (prompt) {
        rc = pthread_create(&context.cmd_thread_id, NULL, cmd_thread, &context);
        if (rc) {
            BDBG_WRN(("Failed to start cli thread"));
            context.done = true;
            goto err_cmd;
        }
    }
    else {
        while (1) BKNI_Sleep(1000);
    }


    if (context.cmd_thread_id)
        pthread_join(context.cmd_thread_id, NULL);
err_cmd:
    if (context.log_thread_id)
        pthread_join(context.log_thread_id, NULL);
err_log:
    NxClient_StopCallbackThread();
err_callback:
    NEXUS_SurfaceClient_Release(context.surfaceClient);
    NxClient_Free(&allocResults);
err:
    NxClient_Uninit();

    return rc;
}
